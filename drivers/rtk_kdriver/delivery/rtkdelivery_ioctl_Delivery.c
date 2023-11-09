/*****************************************************************************
 *
 *   Realtek Delivery driver
 *
 *   Copyright(c) 2019 Realtek Semiconductor Corp. All rights reserved.
 *
 *****************************************************************************/
#include "rtkdelivery.h"
#include "rtkdelivery_debug.h"
#include "tvscalercontrol/vip/scalerColor.h"
#include "demux_tv001/rtkdemux.h"

#include <linux/sched.h>
#include <asm/current.h>
#include <tp/tp_drv_global.h>

extern INT32 RHAL_TPRawModeEnable(TPK_TP_ENGINE_T tp_id, TPK_TP_RAW_CTRL_T enable);

#define DELIVERY_CHECK_CH(x)  \
{\
    if (!delivery_device->pChArray || x >= delivery_device->chNum || !delivery_device->pChArray[x].isInit) { \
        delivery_warn(x,"func %s, line %d, Error(isInit %d, ch %d, chNum %d)!!\n", __func__, __LINE__, (delivery_device->pChArray != 0), x, delivery_device->chNum); \
        return -ENOTTY; \
    } \
}
#define DELIVERY_GET_DELIVERY_ID(_ch) (((_ch) == DELIVERY_CH_A) ? TP_TP0 : (((_ch) == DELIVERY_CH_B) ? TP_TP1 : TP_TP2))

UINT8 boot_status_isACOn = 1;
int IOCTL_ReConfigTPBuffer(DELIVERY_CHANNEL_T ch);
UINT8 deliveryMode[DELIVERY_CH_NUM] = {TP_DELIVERY_MODE_MAX};

/*------------------------------------------------------------------
 * Func : IOCTL_DeliveryINIT
 *
 * Desc :
 *
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_Delivery_INIT(void)
{
    return 0;
}

/*------------------------------------------------------------------
 * Func : IOCTL_DeliveryUNINIT
 *
 * Desc :
 *
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_Delivery_UNINIT(void)
{
    return 0;
}

/*------------------------------------------------------------------
 * Func : IOCTL_DeliveryOpen
 *
 * Desc :
 *
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_Delivery_Open(DELIVERY_DELIVERY_CHANNEL_T *pChannel)
{
    TPK_TP_ENGINE_T tp_id;
    DELIVERY_CHECK_CH(pChannel->ch);

    tp_id = DELIVERY_GET_DELIVERY_ID(pChannel->ch);
    deliveryMode[pChannel->ch] = pChannel->mode;
    RHAL_Delivery_SetMode(tp_id, pChannel->mode);

#if defined(CONFIG_RTK_KDRV_ATSC3_DMX)
    if( deliveryMode[pChannel->ch] == TP_DELIVERY_MODE_ATSC30_BBF ){
        if(RHAL_ATSC30_Open(tp_id) != TPK_SUCCESS)
            return -1;
    }
#endif
#if defined(CONFIG_RTK_KDRV_JP4K)
    if( deliveryMode[pChannel->ch] == TP_DELIVERY_MODE_JAPAN4K ){
        if( RHAL_JAPAN4K_Open(tp_id) != TPK_SUCCESS )
            return -1;
    }
#endif

    return 0;
}

/*------------------------------------------------------------------
 * Func : IOCTL_DeliveryClose
 *
 * Desc :
 *
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_Delivery_Close(DELIVERY_DELIVERY_CHANNEL_T *pChannel)
{
    TPK_TP_ENGINE_T tp_id;
    DELIVERY_CHECK_CH(pChannel->ch);
    tp_id = DELIVERY_GET_DELIVERY_ID(pChannel->ch);

#if defined(CONFIG_RTK_KDRV_ATSC3_DMX)
    if( deliveryMode[pChannel->ch] == TP_DELIVERY_MODE_ATSC30_BBF){
        if(RHAL_ATSC30_Close(tp_id) != TPK_SUCCESS)
            return -1;
    }
#endif
#if defined(CONFIG_RTK_KDRV_JP4K)
    if( deliveryMode[pChannel->ch] == TP_DELIVERY_MODE_JAPAN4K ){
        if(RHAL_JAPAN4K_Close(tp_id) != TPK_SUCCESS)
            return -1;
    }
#endif

    RHAL_Delivery_SetMode(tp_id, TP_DELIVERY_MODE_MAX);
    deliveryMode[pChannel->ch] = TP_DELIVERY_MODE_MAX;

    return 0;
}
/*------------------------------------------------------------------
 * Func : IOCTL_DeliverySetMode
 *
 * Desc :
 *
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_Delivery_SetMode(DELIVERY_DELIVERY_SET_MODE_T *pMode)
{
    TPK_TP_ENGINE_T tp_id = DELIVERY_GET_DELIVERY_ID(pMode->ch);

    deliveryMode[pMode->ch] = pMode->mode;
    RHAL_Delivery_SetMode(tp_id, pMode->mode);

    return 0;
}

/*------------------------------------------------------------------
 * Func : IOCTL_DeliverySetTPInputConfig
 *
 * Desc :
 *
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_Delivery_SetTPInputConfig(DELIVERY_TP_SOURCE_CONFIG_T *pInfo)
{
    INT32 ret;
    TPK_TP_ENGINE_T tp_id = DELIVERY_GET_DELIVERY_ID(pInfo->ch);
#if defined(CONFIG_RTK_KDRV_ATSC3_DMX)
    if( deliveryMode[pInfo->ch] == TP_DELIVERY_MODE_ATSC30_BBF){
        ret = RHAL_Delivery_ATSC30_SetInputConfig(tp_id, pInfo->portType, pInfo->inputType);
    }
#endif
#if defined(CONFIG_RTK_KDRV_JP4K)
    if(deliveryMode[pInfo->ch] == TP_DELIVERY_MODE_JAPAN4K){
        ret = RHAL_Delivery_JAPAN4K_SetInputConfig(tp_id, pInfo->portType, pInfo->inputType, pInfo->srcType);
    }
#endif
    if( deliveryMode[pInfo->ch] == TP_DELIVERY_MODE_TS ){
        ret = RHAL_Delivery_TS_SetInputConfig(tp_id, pInfo->portType, pInfo->inputType, pInfo->srcType);
    }
    if( deliveryMode[pInfo->ch] == TP_DELIVERY_MODE_CI20 ){
        ret = RHAL_Delivery_TPP_SetInputConfig(tp_id, pInfo->portType, pInfo->inputType);
    }
    delivery_err(pInfo->ch,"[%s L:%d] exit ret:%d", __FUNCTION__,__LINE__, ret);

    return 0;
}

/*------------------------------------------------------------------
 * Func : IOCTL_DeliveryRequestData
 * Desc :
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_Delivery_RequestData(DELIVERY_DELIVERY_CHANNEL_T *channel)
{
    TPK_TP_ENGINE_T tp_id;
    UINT16 srcType;
    DELIVERY_CHECK_CH(channel->ch);
    tp_id = DELIVERY_GET_DELIVERY_ID(channel->ch);


    if( RHAL_Delivery_RequestData(tp_id) != TPK_SUCCESS )
        return -1;

    srcType = pTp_drv->tp_japan[tp_id].src_type;
    if( srcType == TPK_DELI_SRC_TYPE_TLV_SW0 || srcType == TPK_DELI_SRC_TYPE_TS ){

        if( srcType == TPK_DELI_SRC_TYPE_TLV_SW0 ){
            if( RHAL_TPRawModeEnable(tp_id, TP_RAW_ENABLE) != TPK_SUCCESS )
                return -1;
        }

        if( RHAL_EnablePIDFilter(tp_id, 0) != TPK_SUCCESS )
            return -1;
        if( RHAL_TPStreamControl(tp_id, TP_STREAM_START) != TPK_SUCCESS )
            return -1;
    }

#ifdef CONFIG_SUPPORT_SCALER
    //fwif_color_set_force_I_De_XC_Disable(1);
#endif
    return 0;
}

/*------------------------------------------------------------------
 * Func : IOCTL_DeliveryCancelData
 * Desc :
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_Delivery_CancelData(DELIVERY_DELIVERY_CHANNEL_T *channel)
{
    TPK_TP_ENGINE_T tp_id;
    UINT16 srcType;
    DELIVERY_CHECK_CH(channel->ch);
    tp_id = DELIVERY_GET_DELIVERY_ID(channel->ch);

    srcType = pTp_drv->tp_japan[tp_id].src_type;
    if( srcType == TPK_DELI_SRC_TYPE_TLV_SW0 || srcType == TPK_DELI_SRC_TYPE_TS ) {
        if (RHAL_TPStreamControl(tp_id, TP_STREAM_STOP) != TPK_SUCCESS)
            return -1;
    }

    if(RHAL_Delivery_CancelData(tp_id) != TPK_SUCCESS)
        return -1;

#ifdef CONFIG_SUPPORT_SCALER
    //fwif_color_set_force_I_De_XC_Disable(0);
#endif
    return 0;
}

/*------------------------------------------------------------------
 * Func : IOCTL_DeliveryGetData
 * Desc :
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_Delivery_GetData(DELIVERY_DELIVERY_READ_DATA_T *pData)
{
    TPK_TP_ENGINE_T tp_id;

    DELIVERY_CHECK_CH(pData->ch);
    tp_id = DELIVERY_GET_DELIVERY_ID(pData->ch);

    if( RHAL_Delivery_GetData(tp_id, pData->data, &pData->data_size) != TPK_SUCCESS)
        return -1;

    return 0;
}


int IOCTL_Delivery_SetPIDFilters(DELIVERY_PID_SETS_T *pInfo)
{
    TPK_PID_FILTER_PARAM_T  add_pidEntry;
    TPK_TP_ENGINE_T tp_id;
    UINT8 i;
    DELIVERY_CHECK_CH(pInfo->ch);

    tp_id = DELIVERY_GET_DELIVERY_ID(pInfo->ch);
    for(i = 0; i < pInfo->count; i++){
        add_pidEntry.valid = 1;
        add_pidEntry.DescrambleEn = 1;
        add_pidEntry.KeySel = tp_id;
        add_pidEntry.PID = pInfo->pidlist[i];

        RHAL_AddPIDFilter(tp_id, add_pidEntry, (void *)DEFAULT_FILE_HANDLE);
    }
    return 0;
}

int IOCTL_Delivery_RemovePIDFilters(DELIVERY_PID_SETS_T *pInfo)
{
    TPK_TP_ENGINE_T tp_id;
    UINT8 i;
    DELIVERY_CHECK_CH(pInfo->ch);

    tp_id = DELIVERY_GET_DELIVERY_ID(pInfo->ch);
    for(i = 0; i < pInfo->count; i++){
        RHAL_RemovePIDFilter(tp_id, pInfo->pidlist[i], (void *)DEFAULT_FILE_HANDLE);
    }
    return 0;
}

int IOCTL_Delivery_EnablePIDFilters(DELIVERY_PID_FILTER_EN_T *pInfo)
{
    TPK_TP_ENGINE_T tp_id;
    DELIVERY_CHECK_CH(pInfo->ch);

    tp_id = DELIVERY_GET_DELIVERY_ID(pInfo->ch);

    return RHAL_EnablePIDFilter(tp_id, pInfo->pid_filter_en);
}

int IOCTL_Delivery_InitDescrambler(DELIVERY_INIT_DESCRAMBLER_T *pInfo)
{
    TPK_TP_ENGINE_T tp_id;
    DELIVERY_CHECK_CH(pInfo->ch);

    tp_id = DELIVERY_GET_DELIVERY_ID(pInfo->ch);
	delivery_err(tp_id,"IntDescramble:ch:%d, alg:%d,round:%d\n",tp_id, pInfo->alg.algo,pInfo->alg.round)
	return (RHAL_SetDescrambleAlgorithm(tp_id, pInfo->alg) == TPK_SUCCESS ? 0 : -1);
}

int IOCTL_Delivery_ControlDescrambler(DELIVERY_DESCRAMBLER_MODE_T *pInfo)
{
    TPK_TP_ENGINE_T tp_id;
    DELIVERY_CHECK_CH(pInfo->ch);

    tp_id = DELIVERY_GET_DELIVERY_ID(pInfo->ch);
	delivery_err(tp_id,"descramber enable:ch:%d, mode:%d\n",tp_id, pInfo->mode)
	return (RHAL_DescrambleControl(tp_id, pInfo->mode) == TPK_SUCCESS ? 0 : -1);
}

int IOCTL_Delivery_DescramblerSetCW(DELIVERY_SET_CW_T *pInfo)
{
    TPK_TP_ENGINE_T tp_id;
    DELIVERY_CHECK_CH(pInfo->ch);

    tp_id = DELIVERY_GET_DELIVERY_ID(pInfo->ch);
	//delivery_err(tp_id,"set key:ch:%d, keyType:%d, keyid:%d\n",tp_id, pInfo->keyType, pInfo->key_id)
	return (RHAL_SetCW(tp_id, tp_id, pInfo->key_id, pInfo->Key, pInfo->IV) == TPK_SUCCESS ? 0 : -1);
}

int IOCTL_Delivery_AdjustStc(DELIVERY_ADJUST_STC_T *pInfo)
{
	return (RHAL_adjustStcClock(pInfo->clock_id, pInfo->f_code, pInfo->n_code) == TPK_SUCCESS ? 0 : -1);
}


#if defined(CONFIG_RTK_KDRV_ATSC3_DMX)
/*------------------------------------------------------------------
 * Func : IOCTL_DeliveryRequestBBFrame
 *
 * Desc :
 *
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_Delivery_RequestBBFrame(DELIVERY_DELIVERY_CHANNEL_T *pChannel)
{
    TPK_TP_ENGINE_T tp_id;
    DELIVERY_CHECK_CH(pChannel->ch);
    tp_id = DELIVERY_GET_DELIVERY_ID(pChannel->ch);

    if( deliveryMode[pChannel->ch] == TP_DELIVERY_MODE_ATSC30_BBF) {
        // only run in raw mode
        //if(RHAL_EnablePIDFilter(tp_id, 0) != TPK_SUCCESS)
        //        return -1;
        //pCh = &delivery_device->pChArray[pChannel->ch];
        //pCh->startStreaming = 1;

        RHAL_ATSC30_RequestBBFrame(tp_id);
    }
#ifdef CONFIG_SUPPORT_SCALER
        //fwif_color_set_force_I_De_XC_Disable(1);
#endif
    return 0;
}

/*------------------------------------------------------------------
 * Func : IOCTL_DeliveryCancelBBFrame
 *
 * Desc :
 *
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_Delivery_CancelBBFrame(DELIVERY_DELIVERY_CHANNEL_T *pChannel)
{
    TPK_TP_ENGINE_T tp_id;
    DELIVERY_CHECK_CH(pChannel->ch);
    tp_id = DELIVERY_GET_DELIVERY_ID(pChannel->ch);

    if( deliveryMode[pChannel->ch] == TP_DELIVERY_MODE_ATSC30_BBF) {
        // only run in raw mode
        //pCh = &delivery_device->pChArray[pChannel->ch];
        //pCh->startStreaming = 0;

        RHAL_ATSC30_CancelBBFrame(tp_id);
    }

#ifdef CONFIG_SUPPORT_SCALER
		//fwif_color_set_force_I_De_XC_Disable(0);
#endif
    return 0;
}

/*------------------------------------------------------------------
 * Func : IOCTL_DeliveryGetBBFrameBuffer
 *
 * Desc :
 *
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_Delivery_GetBBFrameBuffer(DELIVERY_DELIVERY_GET_BBF_BUFFER_T *pBBFBuff)
{
    TPK_TP_ENGINE_T tp_id;
    DELIVERY_CHECK_CH(pBBFBuff->ch);
    tp_id = DELIVERY_GET_DELIVERY_ID(pBBFBuff->ch);

    if( deliveryMode[pBBFBuff->ch] == TP_DELIVERY_MODE_ATSC30_BBF) {
        if (RHAL_ATSC30_GetBBFrame(tp_id, &(pBBFBuff->bbframe_address), &(pBBFBuff->count)) != TPK_SUCCESS)
            return -1;
    }

    return 0;
}

/*------------------------------------------------------------------
 * Func : IOCTL_DeliveryReturnFrameBuffer
 *
 * Desc :
 *
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_Delivery_ReturnFrameBuffer(DELIVERY_DELIVERY_GET_BBF_BUFFER_T *pBBFBuff)
{
    TPK_TP_ENGINE_T tp_id;
    DELIVERY_CHECK_CH(pBBFBuff->ch);
    tp_id = DELIVERY_GET_DELIVERY_ID(pBBFBuff->ch);

    if( deliveryMode[pBBFBuff->ch] == TP_DELIVERY_MODE_ATSC30_BBF) {
        if (RHAL_ATSC30_ReturnBBFrame(tp_id, pBBFBuff->bbframe_address) != TPK_SUCCESS)
            return -1;
    }

    return 0;
}

/*------------------------------------------------------------------
 * Func : IOCTL_DeliveryGetBBFrameBufferInfo
 *
 * Desc :
 *
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_Delivery_GetBBFrameBufferInfo(DELIVERY_DELIVERY_BBF_BUFFER_INFO_T *pBBFBuffInfo)
{
    TPK_TP_ENGINE_T tp_id;
    DELIVERY_CHECK_CH(pBBFBuffInfo->ch);
    tp_id = DELIVERY_GET_DELIVERY_ID(pBBFBuffInfo->ch);

    if( deliveryMode[pBBFBuffInfo->ch] == TP_DELIVERY_MODE_ATSC30_BBF) {
        if (RHAL_ATSC30_GetBBFrameInfo(tp_id, &(pBBFBuffInfo->bbf_buff_phy), &(pBBFBuffInfo->bbf_buff_size)) != TPK_SUCCESS)
            return -1;
    }

    return 0;
}

/*------------------------------------------------------------------
 * Func : IOCTL_DeliveryGetTimeInfo
 *
 * Desc :
 *
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_Delivery_GetTimeInfo(DELIVERY_DELIVERY_TIME_INFO_T *pTimeInfo)
{
    TPK_TP_ENGINE_T tp_id;
    DELIVERY_CHECK_CH(pTimeInfo->ch);
    tp_id = DELIVERY_GET_DELIVERY_ID(pTimeInfo->ch);

    if(RHAL_ATSC30_GetTimeInfo(tp_id, &(pTimeInfo->sec), &(pTimeInfo->nsec), &(pTimeInfo->wall_clock) ) != TPK_SUCCESS) {
        return -1;
    }

    return 0;
}

/*------------------------------------------------------------------
 * Func : IOCTL_DeliverySetTimeInfo
 *
 * Desc :
 *
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_Delivery_SetTimeInfo(DELIVERY_DELIVERY_TIME_INFO_T *pTimeInfo)
{
    TPK_TP_ENGINE_T tp_id;
    DELIVERY_CHECK_CH(pTimeInfo->ch);
    tp_id = DELIVERY_GET_DELIVERY_ID(pTimeInfo->ch);

    if (RHAL_ATSC30_SetTimeInfo(tp_id, pTimeInfo->sec, pTimeInfo->nsec, pTimeInfo->wall_clock ) != TPK_SUCCESS)
        return -1;

    return 0;
}

/*------------------------------------------------------------------
 * Func : IOCTL_DeliverySetSystemTimeInfo
 *
 * Desc :
 *
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_Delivery_SetSystemTimeInfo(DELIVERY_DELIVERY_SYSTEM_TIME_INFO_T *pSystemTimeInfo)
{
    TPK_TP_ENGINE_T tp_id;
    DELIVERY_CHECK_CH(pSystemTimeInfo->ch);
    tp_id = DELIVERY_GET_DELIVERY_ID(pSystemTimeInfo->ch);

    if (RHAL_ATSC30_SetSystemTimeInfo(tp_id, pSystemTimeInfo->current_utc_offset, pSystemTimeInfo->ptp_prepend, pSystemTimeInfo->leap59, pSystemTimeInfo->leap61) != TPK_SUCCESS)
        return -1;

    return 0;
}

/*------------------------------------------------------------------
 * Func : IOCTL_DeliverySetClockRecovery
 *
 * Desc :
 *
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_Delivery_SetClockRecovery(DELIVERY_DELIVERY_CLOCK_RECOVERY_T *pClockRecovery)
{
    TPK_TP_ENGINE_T tp_id;
    DELIVERY_CHECK_CH(pClockRecovery->ch);
    tp_id = DELIVERY_GET_DELIVERY_ID(pClockRecovery->ch);

    if (RHAL_ATSC30_SetClockRecovery(tp_id, pClockRecovery->isNeedRecovery) != TPK_SUCCESS)
        return -1;

    return 0;
}
#endif
#if defined(CONFIG_RTK_KDRV_JP4K)

/*------------------------------------------------------------------
 * Func : IOCTL_Delivery_EnableTLVFilter
 * Desc : enable or disable TLV filter
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_Delivery_EnableTLVFilter(DELIVERY_DELIVERY_FILTER_ENABLE_T *ptlv_param)
{
    TPK_TP_ENGINE_T tp_id;
    DELIVERY_CHECK_CH(ptlv_param->ch);
    tp_id = DELIVERY_GET_DELIVERY_ID(ptlv_param->ch);
    return japan4k_dmx_enable_TLV_filter(tp_id, ptlv_param->enable);
}

/*------------------------------------------------------------------
 * Func : IOCTL_Delivery_AddTLVFilters
 * Desc :
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_Delivery_AddTLVFilters(DELIVERY_DELIVERY_TLV_FILTER_INFO_T *ptlv_param)
{
    TPK_TP_ENGINE_T tp_id;
    DELIVERY_CHECK_CH(ptlv_param->ch);
    tp_id = DELIVERY_GET_DELIVERY_ID(ptlv_param->ch);
    return japan4k_dmx_add_TLV_filters(tp_id, ptlv_param->cnt, ptlv_param->pfiltet_info_list);
}

/*------------------------------------------------------------------
 * Func : IOCTL_Delivery_RemoveTLVFilters
 * Desc :
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_Delivery_RemoveTLVFilters(DELIVERY_DELIVERY_TLV_FILTER_INFO_T *ptlv_param)
{
    TPK_TP_ENGINE_T tp_id;
    DELIVERY_CHECK_CH(ptlv_param->ch);
    tp_id = DELIVERY_GET_DELIVERY_ID(ptlv_param->ch);
    return japan4k_dmx_remove_TLV_filters(tp_id, ptlv_param->cnt, ptlv_param->pfiltet_info_list);
}

int IOCTL_Delivery_GetNTP(DELIVERY_DELIVERY_TLV_NTP_T *channel)
{
    TPK_TP_ENGINE_T tp_id;
    DELIVERY_CHECK_CH(channel->ch);
    tp_id = DELIVERY_GET_DELIVERY_ID(channel->ch);

    return japan4k_dmx_get_ntp(tp_id, &channel->ntp_high, &channel->ntp_low);
}

#endif
/*------------------------------------------------------------------
 * Func : IOCTL_Delivery_GetBootStatus
 *
 * Desc :
 *
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_Delivery_GetBootStatus(DELIVERY_DELIVERY_GET_BOOT_STATUS_T *pData)
{
    pData->isACON = boot_status_isACOn;
    boot_status_isACOn = 0;
    return 0;
}

/*------------------------------------------------------------------
 * Func : IOCTL_Delivery_SaveBootStatus
 *
 * Desc :
 *
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_Delivery_SaveBootStatus(void)
{
// need to check
#if 0
        UINT32 reg_suslt = rtd_inl(STB_WDOG_DATA1_reg);
        if(reg_suslt == 0x2379beef)
                boot_status_isACOn = 0;
        else
                boot_status_isACOn = 1;
#endif
        return 0;
}

