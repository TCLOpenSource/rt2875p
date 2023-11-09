/*****************************************************************************
 *
 *   Realtek Demux driver
 *
 *   Copyright(c) 2014 Realtek Semiconductor Corp. All rights reserved.
 *
 *****************************************************************************/
#include "rtkdemux.h"
#include "rtkdemux_debug.h"
#include "tvscalercontrol/vip/scalerColor.h"

#if 0
#define DEMUX_CHECK_CH(x)  \
{\
        if (!demux_device->pChArray || x >= demux_device->chNum || !demux_device->pChArray[x].isInit) { \
                dmx_warn(x,"func %s, line %d, Error(isInit %d, ch %d, chNum %d)!!\n", __func__, __LINE__, (demux_device->pChArray != 0), x, demux_device->chNum); \
                return -ENOTTY; \
        } \
}
#endif

#define DEMUX_GET_DELIVERY_ID(_ch) (((_ch) == DEMUX_CH_A) ? TP_TP0 : (((_ch) == DEMUX_CH_B) ? TP_TP1 : TP_TP2))

TPK_TP_ENGINE_T ATSC3_ALP_Channel_Mapping(TPK_TP_ENGINE_T input)
{
#if 1
        switch(input) {
                case TP_TP0:
                        return TP_TP2;
                case TP_TP1:
                        return TP_TP3;
                default:
                        return TP_TP2;
        }
#else
        switch(input) {
                case TP_TP0:
                        return TP_TP0;
                case TP_TP1:
                        return TP_TP1;
                default:
                        return TP_TP2;
        }
#endif
}


#ifdef CONFIG_RTK_KDRV_ATSC3_DMX
int IOCTL_ReConfigTPBuffer(DEMUX_CHANNEL_T ch);

typedef enum {
        DELIVERY_MODE_ATSC30 = 0,
        DELIVERY_MODE_ATSC30_ALP,
        DELIVERY_MODE_JAPAN4K,
        DELIVERY_MODE_MAX
} DELIVERY_MODE_T;


UINT8 delivery_mode = DELIVERY_MODE_ATSC30;

/*------------------------------------------------------------------
 * Func : IOCTL_DeliveryINIT
 *
 * Desc :
 *
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_DeliveryINIT(void)
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
int IOCTL_DeliveryUNINIT(void)
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
int IOCTL_DeliveryOpen(DEMUX_DELIVERY_CHANNEL_T *pChannel)
{
        TPK_TP_ENGINE_T tp_id;

        DEMUX_CHECK_CH(pChannel->ch);

        tp_id = DEMUX_GET_DELIVERY_ID(pChannel->ch);

        if( delivery_mode == DELIVERY_MODE_ATSC30) {
                if(RHAL_ATSC30_Open(tp_id) != TPK_SUCCESS)
                        return -1;
        } else if( delivery_mode == DELIVERY_MODE_ATSC30_ALP ) {
                if(RHAL_ATSC30_ALP_Open(tp_id) != TPK_SUCCESS)
                        return -1;
        }
        return 0;
}

/*------------------------------------------------------------------
 * Func : IOCTL_DeliveryClose
 *
 * Desc :
 *
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_DeliveryClose(DEMUX_DELIVERY_CHANNEL_T *pChannel)
{
        TPK_TP_ENGINE_T tp_id;

        DEMUX_CHECK_CH(pChannel->ch);

        tp_id = DEMUX_GET_DELIVERY_ID(pChannel->ch);

        if( delivery_mode == DELIVERY_MODE_ATSC30) {
                if(RHAL_ATSC30_Close(tp_id) != TPK_SUCCESS)
                        return -1;
        } else if( delivery_mode == DELIVERY_MODE_ATSC30_ALP ) {
                if(RHAL_ATSC30_ALP_Close(tp_id) != TPK_SUCCESS)
                        return -1;
        }
        return 0;
}

/*------------------------------------------------------------------
 * Func : IOCTL_DeliverySetTPInputConfig
 *
 * Desc :
 *
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_DeliverySetTPInputConfig(DEMUX_TP_SOURCE_CONFIG_T *pInfo)
{
        TPK_TP_SOURCE_T tp_src;
        TPK_TP_STATUS_T tpStatus;
        TPK_TP_ENGINE_T tp_id = DEMUX_GET_DELIVERY_ID(pInfo->ch);

        tp_src = RHAL_GetTPSource(pInfo->portType, pInfo->casType);

        if( delivery_mode == DELIVERY_MODE_ATSC30) {
                if (RHAL_SetTPSource(tp_id, tp_src, TPK_WITHOUT_CAS) != TPK_SUCCESS)
                        return TPK_FAIL;
        } else if(delivery_mode == DELIVERY_MODE_ATSC30_ALP) {
                if (RHAL_GetTpStatus(TP_TP2, &tpStatus) != TPK_SUCCESS)
                        return TPK_FAIL;

                if(pInfo->inputType == TPK_INPUT_SERIAL)
                        tpStatus.tp_param.serial = 1;
                else
                        tpStatus.tp_param.serial = 0;

                if (RHAL_SetTPMode(TP_TP2, tpStatus.tp_param) != TPK_SUCCESS)
                        return -1;

                if(RHAL_SetTPSource(TP_TP2, TS_SRC_2, TPK_WITHOUT_CAS) != TPK_SUCCESS)
                        return TPK_FAIL;
        }
        return 0;
}

/*------------------------------------------------------------------
 * Func : IOCTL_DeliveryRequestBBFrame
 *
 * Desc :
 *
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_DeliveryRequestBBFrame(DEMUX_DELIVERY_CHANNEL_T *pChannel)
{
        //demux_channel *pCh;
        TPK_TP_ENGINE_T tp_id;

        DEMUX_CHECK_CH(pChannel->ch);

        tp_id = DEMUX_GET_DELIVERY_ID(pChannel->ch);

        if( delivery_mode == DELIVERY_MODE_ATSC30) {
#if 0
                // only run on raw mode
                RHAL_SetTPSource(TP_TP0, TS_SRC_0, TPK_WITHOUT_CAS);

                if(RHAL_TPRawModeEnable(TP_TP0, TP_RAW_ENABLE) != TPK_SUCCESS)
                        return -1;

                if(RHAL_EnablePIDFilter(TP_TP0, 0) != TPK_SUCCESS)
                        return -1;

                if(RHAL_TPStreamControl(TP_TP0, TP_STREAM_START) != TPK_SUCCESS)
                        return -1;

                pCh = &demux_device->pChArray[pChannel->ch];
                pCh->startStreaming = 1;
#endif

                RHAL_ATSC30_RequestBBFrame(tp_id);
        } else if(delivery_mode == DELIVERY_MODE_ATSC30_ALP) {
                if(RHAL_TPRawModeEnable(TP_TP2, TP_RAW_ENABLE) != TPK_SUCCESS) {
                        return -1;
                }

                if(RHAL_EnablePIDFilter(TP_TP2, 0) != TPK_SUCCESS) {
                        return -1;
                }

                RHAL_ATSC30_ALP_RequestBBFrame(tp_id);

                if(RHAL_TPStreamControl(TP_TP2, TP_STREAM_START) != TPK_SUCCESS) {
                        return -1;
                }
        }
        //fwif_color_set_force_I_De_XC_Disable(1);
        return 0;
}

/*------------------------------------------------------------------
 * Func : IOCTL_DeliveryCancelBBFrame
 *
 * Desc :
 *
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_DeliveryCancelBBFrame(DEMUX_DELIVERY_CHANNEL_T *pChannel)
{
        TPK_TP_ENGINE_T tp_id;
        //demux_channel *pCh;

        DEMUX_CHECK_CH(pChannel->ch);

        tp_id = DEMUX_GET_DELIVERY_ID(pChannel->ch);

        if( delivery_mode == DELIVERY_MODE_ATSC30) {
                // only run in raw mode
                //pCh = &demux_device->pChArray[pChannel->ch];
                //pCh->startStreaming = 0;

                RHAL_ATSC30_CancelBBFrame(tp_id);
        } else if(delivery_mode == DELIVERY_MODE_ATSC30_ALP) {
                if (RHAL_TPStreamControl(TP_TP2, TP_STREAM_STOP) != TPK_SUCCESS) {
                        return -1;
                }
                RHAL_ATSC30_ALP_CancelBBFrame(tp_id);
        }

        //fwif_color_set_force_I_De_XC_Disable(0);
        return 0;
}

/*------------------------------------------------------------------
 * Func : IOCTL_DeliveryGetBBFrameBuffer
 *
 * Desc :
 *
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_DeliveryGetBBFrameBuffer(DEMUX_DELIVERY_GET_BBF_BUFFER_T *pBBFBuff)
{
        TPK_TP_ENGINE_T tp_id;

        DEMUX_CHECK_CH(pBBFBuff->ch);

        tp_id = DEMUX_GET_DELIVERY_ID(pBBFBuff->ch);

        if( delivery_mode == DELIVERY_MODE_ATSC30 || delivery_mode == DELIVERY_MODE_ATSC30_ALP) {
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
int IOCTL_DeliveryReturnFrameBuffer(DEMUX_DELIVERY_GET_BBF_BUFFER_T *pBBFBuff)
{
        TPK_TP_ENGINE_T tp_id;

        DEMUX_CHECK_CH(pBBFBuff->ch);

        tp_id = DEMUX_GET_DELIVERY_ID(pBBFBuff->ch);

        if( delivery_mode == DELIVERY_MODE_ATSC30 || delivery_mode == DELIVERY_MODE_ATSC30_ALP) {
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
int IOCTL_DeliveryGetBBFrameBufferInfo(DEMUX_DELIVERY_BBF_BUFFER_INFO_T *pBBFBuffInfo)
{
        TPK_TP_ENGINE_T tp_id;

        DEMUX_CHECK_CH(pBBFBuffInfo->ch);

        tp_id = DEMUX_GET_DELIVERY_ID(pBBFBuffInfo->ch);

        if( delivery_mode == DELIVERY_MODE_ATSC30  || delivery_mode == DELIVERY_MODE_ATSC30_ALP) {
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
int IOCTL_DeliveryGetTimeInfo(DEMUX_DELIVERY_TIME_INFO_T *pTimeInfo)
{
        TPK_TP_ENGINE_T tp_id;

        DEMUX_CHECK_CH(pTimeInfo->ch);

        tp_id = DEMUX_GET_DELIVERY_ID(pTimeInfo->ch);

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
int IOCTL_DeliverySetTimeInfo(DEMUX_DELIVERY_TIME_INFO_T *pTimeInfo)
{
        TPK_TP_ENGINE_T tp_id;

        DEMUX_CHECK_CH(pTimeInfo->ch);

        tp_id = DEMUX_GET_DELIVERY_ID(pTimeInfo->ch);

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
int IOCTL_DeliverySetSystemTimeInfo(DEMUX_DELIVERY_SYSTEM_TIME_INFO_T *pSystemTimeInfo)
{
        TPK_TP_ENGINE_T tp_id;

        DEMUX_CHECK_CH(pSystemTimeInfo->ch);

        tp_id = DEMUX_GET_DELIVERY_ID(pSystemTimeInfo->ch);

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
int IOCTL_DeliverySetClockRecovery(DEMUX_DELIVERY_CLOCK_RECOVERY_T *pClockRecovery)
{
        TPK_TP_ENGINE_T tp_id;

        DEMUX_CHECK_CH(pClockRecovery->ch);

        tp_id = DEMUX_GET_DELIVERY_ID(pClockRecovery->ch);

        if (RHAL_ATSC30_SetClockRecovery(tp_id, pClockRecovery->isNeedRecovery) != TPK_SUCCESS)
                return -1;

        return 0;
}

/*------------------------------------------------------------------
 * Func : IOCTL_DeliverySetMode
 *
 * Desc :
 *
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_DeliverySetMode(DEMUX_DELIVERY_SET_MODE_T *pMode)
{
        delivery_mode = pMode->mode;

        if ( RHAL_Delivery_SetMode(pMode->mode) != TPK_SUCCESS)
                return -1;

        return 0;
}

/*------------------------------------------------------------------
 * Func : IOCTL_DeliveryRequestData
 *
 * Desc :
 *
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_DeliveryRequestData(DEMUX_DELIVERY_CHANNEL_T *channel)
{
        return 0;
}

/*------------------------------------------------------------------
 * Func : IOCTL_DeliveryCancelData
 *
 * Desc :
 *
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_DeliveryCancelData(DEMUX_DELIVERY_CHANNEL_T *channel)
{
        return 0;
}

/*------------------------------------------------------------------
 * Func : IOCTL_DeliveryGetData
 *
 * Desc :
 *
 * Retn :
 *------------------------------------------------------------------*/
int IOCTL_DeliveryGetData(DEMUX_DELIVERY_READ_DATA_T *pData)
{
        return 0;
}
#endif

