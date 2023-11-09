/******************************************************************************
 *
 *   Copyright(c) 2014 Realtek Semiconductor Corp. All rights reserved.
 *
 *   @author yungjui.lee@realtek.com
 *
 *****************************************************************************/
#include "audio_base.h"
#include "audio_rpc.h"
#include <linux/slab.h>
#include "hal_common.h"
#include "audio_inc.h"


#define MAX_INST_NUM    2
#define INST_IDX_MAIN   0
#define INST_IDX_SUB    1
#define INST_MASK       0xFFFFF000
static SINT32 g_instance_idx = 0;
static SINT32 g_instance_list[MAX_INST_NUM] = {-1,-1};

static UINT32 ConfigBBADC(Base* pBaseObj)
{
    AUDIO_BBADC_CONFIG cfg_bbadc;
    AIN* pAINObj = (AIN*)pBaseObj->pDerivedObj;

    memset(&cfg_bbadc, 0, sizeof(AUDIO_BBADC_CONFIG));
    cfg_bbadc.audioGeneralConfig.channel_in = AUDIO_CH_IDX_0 | AUDIO_CH_IDX_1;
    cfg_bbadc.sampleInfo.sampling_rate = 48000;
    cfg_bbadc.sampleInfo.PCM_bitnum    = 24;
    cfg_bbadc.clk_sel = AUDIO_BBADC_CLK_49M;
    cfg_bbadc.src_sel = AUDIO_BBADC_SRC_AIN1;
    memcpy(&(pAINObj->m_pBBADC_cfg), &cfg_bbadc, sizeof(AUDIO_BBADC_CONFIG));

    return S_OK;
}

static UINT32 ConfigI2S(Base* pBaseObj)
{
    AUDIO_I2SI_CONFIG cfg_i2s;
    AIN* pAINObj = (AIN*)pBaseObj->pDerivedObj;

    memset(&cfg_i2s, 0, sizeof(AUDIO_I2SI_CONFIG));
    cfg_i2s.audioGeneralConfig.channel_in = AUDIO_CH_IDX_0 | AUDIO_CH_IDX_1;
    cfg_i2s.id        = AUDIO_I2SI_ID_PRIMARY;
    cfg_i2s.src_sel   = AUDIO_I2SI_SRC_IN;
    cfg_i2s.mode_sel  = AUDIO_I2SI_MODE_SLAVE;
    cfg_i2s.sync_type = AUDIO_I2SI_SYNC_NORMAL;
    memcpy(&(pAINObj->m_pI2S_cfg), &cfg_i2s, sizeof(AUDIO_I2SI_CONFIG));

    return S_OK;
}

static UINT32 ConfigSPDIF(Base* pBaseObj)
{
    AUDIO_SPDIFI_CONFIG cfg_spdif;
    AIN* pAINObj = (AIN*)pBaseObj->pDerivedObj;

    memset(&cfg_spdif, 0, sizeof(AUDIO_SPDIFI_CONFIG));
    cfg_spdif.audioGeneralConfig.channel_in = AUDIO_CH_IDX_0 | AUDIO_CH_IDX_1;
    cfg_spdif.src_sel        = AUDIO_SPDIFI_SRC_HDMI;
    cfg_spdif.req_after_lock = true;
    memcpy(&(pAINObj->m_pSPDIF_cfg), &cfg_spdif, sizeof(AUDIO_SPDIFI_CONFIG));

    return S_OK;
}

static UINT32 SetInterleaveOut(Base* pObj)
{
    UINT32 res;
    AUDIO_RPC_SET_INTERLEAVE_OUT pInfo;
    pInfo.instanceID = pObj->GetAgentID(pObj);
    pInfo.enable_interleave_out_pb = TRUE;
    pInfo.enable_interleave_out_ts = TRUE;
    res = RTKAUDIO_RPC_TOAGENT_SET_INTERLEAVE_OUT_SVC(&pInfo);
    if(res != S_OK) ERROR("[%s] RPC return != S_OK\n",__FUNCTION__);

    return res;
}

static UINT32 ConfigFirmware(Base* pObj)
{
    UINT32 res;
    AIN* pAINObj = (AIN*)pObj->pDerivedObj;
    AUDIO_RPC_BBADC_CONFIG  pRPC_BBADC_cfg;
    AUDIO_RPC_I2SI_CONFIG   pRPC_I2S_cfg;
    AUDIO_RPC_SPDIFI_CONFIG pRPC_SPDIF_cfg;

    DEBUG("[%s][%s]\n", pObj->name, __FUNCTION__);

    if(pObj->GetAgentID(pObj) != (UINT32)g_instance_list[INST_IDX_MAIN])
    {
        return S_OK;
    }

    pRPC_BBADC_cfg.instanceID = pObj->GetAgentID(pObj);
    pRPC_BBADC_cfg.config = pAINObj->m_pBBADC_cfg;
    res = RTKAUDIO_RPC_TOAGENT_BBADC_CONFIG_SVC(&pRPC_BBADC_cfg);

    if(res != S_OK) ERROR("[%s:%d] RPC return != S_OK\n",__FUNCTION__,__LINE__);

    pRPC_I2S_cfg.instanceID = pObj->GetAgentID(pObj);
    pRPC_I2S_cfg.config = pAINObj->m_pI2S_cfg;
    res = RTKAUDIO_RPC_TOAGENT_I2SI_CONFIG_SVC(&pRPC_I2S_cfg);

    if(res != S_OK) ERROR("[%s:%d] RPC return != S_OK\n",__FUNCTION__,__LINE__);

    pRPC_SPDIF_cfg.instanceID = pObj->GetAgentID(pObj);
    pRPC_SPDIF_cfg.config = pAINObj->m_pSPDIF_cfg;
    res = RTKAUDIO_RPC_TOAGENT_SPDIFI_CONFIG_SVC(&pRPC_SPDIF_cfg);

    if(res != S_OK) ERROR("[%s:%d] RPC return != S_OK\n",__FUNCTION__,__LINE__);

    return res;
}

UINT32 AIN_SwitchFocus(Base* pBaseObj, void* data)
{
    UINT32 res;
    AIN* pAINObj = (AIN*)pBaseObj->pDerivedObj;
    AUDIO_RPC_IPT_SRC pSource;
    AUDIO_IPT_SRC* pIptSrc = (AUDIO_IPT_SRC*)data;
    AUDIO_RPC_IPT_SRC* m_pIptSrc = &(pAINObj->m_pIptSrc);
    DEBUG("[%s][%s]\n", __FUNCTION__, pBaseObj->name);

    switch((pIptSrc->focus[0] & 0xFFFF)) {
        case AUDIO_IPT_SRC_BBADC:
            pAINObj->m_pBBADC_cfg.src_sel = (pIptSrc->mux_in & 0xFFFF);
            break;
        case AUDIO_IPT_SRC_I2S_PRI_CH12:
        case AUDIO_IPT_SRC_I2S_PRI_CH34:
        case AUDIO_IPT_SRC_I2S_PRI_CH56:
        case AUDIO_IPT_SRC_I2S_PRI_CH78:
            pAINObj->m_pI2S_cfg.src_sel = (pIptSrc->mux_in & 0xFFFF);
            break;
        case AUDIO_IPT_SRC_SPDIF:
            pAINObj->m_pSPDIF_cfg.src_sel = (pIptSrc->mux_in & 0xFFFF);
            break;
        case AUDIO_IPT_SRC_ATV:
            break;
        case AUDIO_IPT_SRC_UNKNOWN:
            break;
        case AUDIO_IPT_SRC_DTV:
        case AUDIO_IPT_SRC_FILE:
        default:
            ERROR("[AIN] unknown PB source(%x), can't switch focus ! \n", (pIptSrc->focus[0] & 0xFFFF));
            return S_FALSE;
    }

    if ((UINT32)m_pIptSrc->instanceID == pBaseObj->GetAgentID(pBaseObj) && m_pIptSrc->mux_in == pIptSrc->mux_in &&
        m_pIptSrc->focus_in[0] == pIptSrc->focus[0] && m_pIptSrc->focus_in[1] == pIptSrc->focus[1] &&
        m_pIptSrc->focus_in[2] == pIptSrc->focus[2] && m_pIptSrc->focus_in[3] == pIptSrc->focus[3]) {
        return S_OK;
    }

    pSource.instanceID  = pBaseObj->GetAgentID(pBaseObj);
    pSource.focus_in[0] = pIptSrc->focus[0];    // CH1&CH2
    pSource.focus_in[1] = pIptSrc->focus[1];    // CH3&CH4
    pSource.focus_in[2] = pIptSrc->focus[2];    // CH5&CH6
    pSource.focus_in[3] = pIptSrc->focus[3];    // CH7&CH8
    pSource.mux_in      = pIptSrc->mux_in;
    res = RTKAUDIO_RPC_TOAGENT_CHG_IPT_SRC_SVC(&pSource);

    if(res == S_OK)
    {
        m_pIptSrc->instanceID  = pBaseObj->GetAgentID(pBaseObj);
        m_pIptSrc->focus_in[0] = pIptSrc->focus[0];  // CH1&CH2
        m_pIptSrc->focus_in[1] = pIptSrc->focus[1];  // CH3&CH4
        m_pIptSrc->focus_in[2] = pIptSrc->focus[2];  // CH5&CH6
        m_pIptSrc->focus_in[3] = pIptSrc->focus[3];  // CH7&CH8
        m_pIptSrc->mux_in      = pIptSrc->mux_in;
    }
    else
        ERROR("[%s:%d] RPC return != S_OK\n",__FUNCTION__,__LINE__);

    return res;
}

UINT32 AIN_Pause(Base* pBaseObj)
{
    UINT32 res = S_OK;

    if(pBaseObj->state == STATE_RUN)
    {
        res = RTKAUDIO_RPC_TOAGENT_PAUSE_SVC(&(pBaseObj->instanceID));

        if(res != S_OK) ERROR("[%s][%s] RPC return != S_OK\n",__FUNCTION__,pBaseObj->name);
        else DEBUG("[%s][%s] RPC return = S_OK\n",__FUNCTION__,pBaseObj->name);
    }
    else if(pBaseObj->state == STATE_STOP)
    {
        ConfigFirmware(pBaseObj);
        res = S_OK;
    }

    pBaseObj->state = STATE_PAUSE;
    return res;
}

UINT32 AIN_Flush(Base* pBaseObj)
{
    AUDIO_RPC_SENDIO sendIO;
    UINT32 res;
    if(pBaseObj->GetAgentID(pBaseObj) == UNDEFINED_AGENT_ID) return S_OK;

    sendIO.pinID      = pBaseObj->GetOutPinID(pBaseObj);// ain use out pin to do flush
    sendIO.instanceID = pBaseObj->GetAgentID(pBaseObj);

    res = RTKAUDIO_RPC_TOAGENT_FLUSH_SVC(&sendIO);
    if(res != S_OK) ERROR("[%s][%s] RPC return != S_OK\n",pBaseObj->name,__FUNCTION__);
    else DEBUG("[%s][%s] RPC return = S_OK\n",pBaseObj->name,__FUNCTION__);
    return res;
}

UINT32 AIN_GetAudioFormatInfo(Base* pBaseObj, AUDIO_RPC_AIN_FORMAT_INFO* formatInfo)
{
    UINT32 res;
    AUDIO_RPC_AIN_FORMAT_INFO Info;
    int instanceID = pBaseObj->GetAgentID(pBaseObj);

    res = RTKAUDIO_RPC_TOAGENT_GET_FORMAT_SVC((UINT32*)&instanceID, &Info);
    if(res != S_OK) ERROR("[%s][%s] RPC return != S_OK\n",pBaseObj->name,__FUNCTION__);
    else
    {
        formatInfo->type = Info.type;
        formatInfo->nSamplesPerSec = Info.nSamplesPerSec;
        DEBUG("[%s][%s] RPC return = S_OK\n",pBaseObj->name,__FUNCTION__);
    }
    return res;
}

UINT32 SetATVClock(Base* pBaseObj, SINT32 atv_type)
{
    UINT32 res;
    AUDIO_RPC_SET_ATV_CLOCK pInfo;

    pInfo.instanceID = pBaseObj->GetAgentID(pBaseObj);
    pInfo.atv_type = atv_type;

    res = RTKAUDIO_RPC_TOAGENT_SET_ATV_CLOCK_SVC(&pInfo);
    if(res != S_OK) ERROR("[%s][%s] RPC return != S_OK\n",pBaseObj->name,__FUNCTION__);
    else DEBUG("[%s][%s] RPC return = S_OK\n",pBaseObj->name,__FUNCTION__);
    return res;
}

UINT32 SetSIFADCInit(Base* pBaseObj, SINT32 enable)
{
    UINT32 res;
    AUDIO_RPC_SET_SIF_ADC_INIT pInfo;

    pInfo.instanceID = pBaseObj->GetAgentID(pBaseObj);
    pInfo.sif_adc_init_enable = enable;

    res = RTKAUDIO_RPC_TOAGENT_SET_SIF_ADC_INIT_SVC(&pInfo);
    if(res != S_OK) ERROR("[%s][%s] RPC return != S_OK\n",pBaseObj->name,__FUNCTION__);
    else DEBUG("[%s][%s] RPC return = S_OK\n",pBaseObj->name,__FUNCTION__);
    return res;
}


void delete_AIN(Base* pBaseObj)
{
    AIN* pAINObj = NULL;
    int info_fw = true;
    if(pBaseObj == NULL) return;
    pAINObj = (AIN*)pBaseObj->pDerivedObj;

    del_id_from_map(pBaseObj->GetAgentID(pBaseObj));

    g_instance_idx--;
    if(g_instance_idx >= 0) {
        DEBUG("[AIN] delete instance(0x%08x), cntr=%d\n",pBaseObj->GetAgentID(pBaseObj),g_instance_idx);
        if((int)pBaseObj->instanceID != g_instance_list[g_instance_idx]) {
            SINT32 i;
            ERROR("[AIN] ERROR: delete instance mismatch!!!\n");
            for(i=0; i<MAX_INST_NUM; i++) {
                ERROR("[AIN] g_instance_list[%d] = 0x%08x\n",i,(UINT32)g_instance_list[i]);
            }
            /*BUG_ON(0);*/
        }

        g_instance_list[g_instance_idx] = -1;   //reset

        if((g_instance_idx == INST_IDX_MAIN) && (g_instance_list[INST_IDX_SUB] != -1)) {
            ERROR("[AIN] ERROR: can't delete main before sub flow!!!\n");
            /*BUG_ON(0);*/
        }
    }else {
        ERROR("[AIN] ERROR: delete instance(0x%08x), cntr=%d\n",(UINT32)pBaseObj->GetAgentID(pBaseObj),g_instance_idx);
        /*BUG_ON(0);*/
    }
    info_fw = (g_instance_idx == INST_IDX_MAIN) ? true : false;

    pBaseObj->ResetOutRingBuf(pBaseObj);

    if(info_fw)
    {
        UINT32 instanceID = pBaseObj->instanceID & INST_MASK;
        pBaseObj->Remove(pBaseObj);
        DestroyAgent(&instanceID);
    }

    if(pBaseObj->outPin != NULL)
    {
        pBaseObj->outPin->Delete(pBaseObj->outPin);
        pBaseObj->outPin = NULL;
    }

    kfree(pAINObj);
    delete_base(pBaseObj);
    return;
}

Base* new_AIN_QuickShow(QuickShowCreateParam param)
{
    /* object init */
    AIN* pAINObj = NULL;
    Base* pObj = new_base();
    if(pObj == NULL)
    {
        return NULL;
    }
    pAINObj = (AIN*)kmalloc(sizeof(AIN), GFP_KERNEL);
    if(pAINObj == NULL)
    {
        pObj->Delete(pObj);
        return NULL;
    }
    pObj->pDerivedObj = pAINObj;

    DEBUG("[%s] \n", __FUNCTION__);

    /* setup member functions */
    pObj->Delete      = delete_AIN;
    pObj->SwitchFocus = AIN_SwitchFocus;
    pObj->Pause       = AIN_Pause;
    pObj->Flush       = AIN_Flush;
    pAINObj->SetATVClock = SetATVClock;
    pAINObj->SetSIFADCInit = SetSIFADCInit;
    pAINObj->GetAudioFormatInfo = AIN_GetAudioFormatInfo;

    /* agent init */
    if(g_instance_idx == INST_IDX_MAIN)
    {
        char name[] = "Main-AIN";
        //pObj->instanceID = CreateAgent(AUDIO_IN);
        pObj->instanceID = param.ain_instanceID;
        if(pObj->instanceID == UNDEFINED_AGENT_ID)
        {
            pObj->Delete(pObj);
            return NULL;
        }
        pObj->outPinID = PCM_OUT_RTK;

        g_instance_list[INST_IDX_MAIN] = (pObj->instanceID & INST_MASK) | pObj->outPinID;
        memcpy(pObj->name, name, sizeof(name));
    }
    else if(g_instance_idx == INST_IDX_SUB)
    {
        char name[] = "Sub-AIN";
        pObj->outPinID = PCM_OUT2;
        g_instance_list[INST_IDX_SUB] = (g_instance_list[INST_IDX_MAIN] & INST_MASK) | pObj->outPinID;
        memcpy(pObj->name, name, sizeof(name));
    }
    pObj->instanceID = g_instance_list[g_instance_idx];
    g_instance_idx++;

    pAINObj->m_pBBADC_cfg.audioGeneralConfig.channel_in = AUDIO_CH_IDX_0 | AUDIO_CH_IDX_1;
    pAINObj->m_pBBADC_cfg.sampleInfo.sampling_rate = 48000;
    pAINObj->m_pBBADC_cfg.sampleInfo.PCM_bitnum = 24;
    pAINObj->m_pBBADC_cfg.clk_sel = AUDIO_BBADC_CLK_49M;
    pAINObj->m_pBBADC_cfg.src_sel = AUDIO_BBADC_SRC_AIN1;

    pAINObj->m_pI2S_cfg.audioGeneralConfig.channel_in = AUDIO_CH_IDX_0 | AUDIO_CH_IDX_1;
    pAINObj->m_pI2S_cfg.id = AUDIO_I2SI_ID_PRIMARY;
    pAINObj->m_pI2S_cfg.src_sel = AUDIO_I2SI_SRC_IN;
    pAINObj->m_pI2S_cfg.mode_sel = AUDIO_I2SI_MODE_SLAVE;
    pAINObj->m_pI2S_cfg.sync_type = AUDIO_I2SI_SYNC_NORMAL;

    pAINObj->m_pSPDIF_cfg.audioGeneralConfig.channel_in = AUDIO_CH_IDX_0 | AUDIO_CH_IDX_1;
    pAINObj->m_pSPDIF_cfg.src_sel = AUDIO_SPDIFI_SRC_HDMI;
    pAINObj->m_pSPDIF_cfg.req_after_lock = true;

    pAINObj->m_pIptSrc.instanceID  = -1;
    pAINObj->m_pIptSrc.focus_in[0] = AUDIO_IPT_SRC_UNKNOWN;  // CH1&CH2
    pAINObj->m_pIptSrc.focus_in[1] = AUDIO_IPT_SRC_UNKNOWN;  // CH3&CH4
    pAINObj->m_pIptSrc.focus_in[2] = AUDIO_IPT_SRC_UNKNOWN;  // CH5&CH6
    pAINObj->m_pIptSrc.focus_in[3] = AUDIO_IPT_SRC_UNKNOWN;  // CH7&CH8
    pAINObj->m_pIptSrc.mux_in      = 0;

    SetInterleaveOut(pObj);
    ConfigBBADC(pObj);
    ConfigI2S(pObj);
    ConfigSPDIF(pObj);
    ConfigFirmware(pObj);

    pObj->state = STATE_RUN; /*avoid init InRingBuf & rpc as connect*/

    pObj->outPin = new_pin_QuickShow(param.ain_buf_size, param.ain_nonCachedAddr, &param.ain_rh_phyAddr);
    //pObj->InitOutRingBuf(pObj);

    add_id_to_map(pObj, pObj->GetAgentID(pObj));
    DEBUG("[%s] init OK\n", pObj->name);
    return pObj;
}

Base* new_AIN(void)
{
    /* object init */
    AIN* pAINObj = NULL;
    Base* pObj = new_base();
    if(pObj == NULL)
    {
        return NULL;
    }
    pAINObj = (AIN*)kmalloc(sizeof(AIN), GFP_KERNEL);
    if(pAINObj == NULL)
    {
        pObj->Delete(pObj);
        return NULL;
    }
    pObj->pDerivedObj = pAINObj;

    DEBUG("[%s] \n", __FUNCTION__);

    /* setup member functions */
    pObj->Delete      = delete_AIN;
    pObj->SwitchFocus = AIN_SwitchFocus;
    pObj->Pause       = AIN_Pause;
    pObj->Flush       = AIN_Flush;
    pAINObj->SetATVClock = SetATVClock;
    pAINObj->SetSIFADCInit = SetSIFADCInit;
    pAINObj->GetAudioFormatInfo = AIN_GetAudioFormatInfo;

    /* agent init */
    if(g_instance_idx == INST_IDX_MAIN)
    {
        char name[] = "Main-AIN";
        pObj->instanceID = CreateAgent(AUDIO_IN);
        if(pObj->instanceID == UNDEFINED_AGENT_ID)
        {
            pObj->Delete(pObj);
            return NULL;
        }
        pObj->outPinID = PCM_OUT_RTK;

        g_instance_list[INST_IDX_MAIN] = (pObj->instanceID & INST_MASK) | pObj->outPinID;
        memcpy(pObj->name, name, sizeof(name));
    }
    else if(g_instance_idx == INST_IDX_SUB)
    {
        char name[] = "Sub-AIN";
        pObj->outPinID = PCM_OUT2;
        g_instance_list[INST_IDX_SUB] = (g_instance_list[INST_IDX_MAIN] & INST_MASK) | pObj->outPinID;
        memcpy(pObj->name, name, sizeof(name));
    }
    pObj->instanceID = g_instance_list[g_instance_idx];
    g_instance_idx++;

    pAINObj->m_pBBADC_cfg.audioGeneralConfig.channel_in = AUDIO_CH_IDX_0 | AUDIO_CH_IDX_1;
    pAINObj->m_pBBADC_cfg.sampleInfo.sampling_rate = 48000;
    pAINObj->m_pBBADC_cfg.sampleInfo.PCM_bitnum = 24;
    pAINObj->m_pBBADC_cfg.clk_sel = AUDIO_BBADC_CLK_49M;
    pAINObj->m_pBBADC_cfg.src_sel = AUDIO_BBADC_SRC_AIN1;

    pAINObj->m_pI2S_cfg.audioGeneralConfig.channel_in = AUDIO_CH_IDX_0 | AUDIO_CH_IDX_1;
    pAINObj->m_pI2S_cfg.id = AUDIO_I2SI_ID_PRIMARY;
    pAINObj->m_pI2S_cfg.src_sel = AUDIO_I2SI_SRC_IN;
    pAINObj->m_pI2S_cfg.mode_sel = AUDIO_I2SI_MODE_SLAVE;
    pAINObj->m_pI2S_cfg.sync_type = AUDIO_I2SI_SYNC_NORMAL;

    pAINObj->m_pSPDIF_cfg.audioGeneralConfig.channel_in = AUDIO_CH_IDX_0 | AUDIO_CH_IDX_1;
    pAINObj->m_pSPDIF_cfg.src_sel = AUDIO_SPDIFI_SRC_HDMI;
    pAINObj->m_pSPDIF_cfg.req_after_lock = true;

    pAINObj->m_pIptSrc.instanceID  = -1;
    pAINObj->m_pIptSrc.focus_in[0] = AUDIO_IPT_SRC_UNKNOWN;  // CH1&CH2
    pAINObj->m_pIptSrc.focus_in[1] = AUDIO_IPT_SRC_UNKNOWN;  // CH3&CH4
    pAINObj->m_pIptSrc.focus_in[2] = AUDIO_IPT_SRC_UNKNOWN;  // CH5&CH6
    pAINObj->m_pIptSrc.focus_in[3] = AUDIO_IPT_SRC_UNKNOWN;  // CH7&CH8
    pAINObj->m_pIptSrc.mux_in      = 0;

    SetInterleaveOut(pObj);
    ConfigBBADC(pObj);
    ConfigI2S(pObj);
    ConfigSPDIF(pObj);
    ConfigFirmware(pObj);

    pObj->outPin = new_pin(AUDIO_IN_OUTPUT_BUF_SIZE);
    pObj->InitOutRingBuf(pObj);

    add_id_to_map(pObj, pObj->GetAgentID(pObj));
    DEBUG("[%s] init OK\n", pObj->name);
    return pObj;
}
