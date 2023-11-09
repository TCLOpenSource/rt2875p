/******************************************************************************
 *
 *   Realtek Delivery driver
 *
 *   Copyright(c) 2019 Realtek Semiconductor Corp. All rights reserved.
 *
 *   @author 
 *
 *****************************************************************************/


#ifndef _DELIVERY_DRIVER_H_
#define _DELIVERY_DRIVER_H_


#include <generated/autoconf.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include "rtkdelivery_common.h"
#include "rtkdelivery_export.h"
#include <tp/tp_drv_api.h>

#define RTKDELIVERY_MAJOR  0

typedef enum {
	DELIVERY_STATE_STOP = 0,
	DELIVERY_STATE_PAUSE,
	DELIVERY_STATE_RUNNING,

} DELIVERY_STATE_T;

typedef struct {
	dma_addr_t     phyAddr ;
	unsigned char *virAddr ;
	unsigned char *cachedaddr ;     /* cached addresss for release*/
	unsigned char *nonCachedaddr ;
	unsigned int   size ;
	unsigned int   allocSize ;
	unsigned char  isCache ;
	unsigned char  fromPoll ;
} DELIVERY_BUF_T ;

typedef struct {
	unsigned char       isInit ;
	unsigned char       startStreaming ;
	struct task_struct *thread_task ;
	struct mutex        mutex ;
	DELIVERY_STATE_T    state ;
	DELIVERY_BUF_T      tpBuffer ;
	unsigned char       tunerHandle ;
	unsigned int        tsPacketSize;
} delivery_channel ;

typedef struct {
	struct semaphore    sem ;          /* mutual exclusion semaphore     */
	struct cdev         cdev ;         /* Char device structure          */
	struct mutex        mutex ;      /* make secFilterBuffer, secMap, secBufCnt and cwInfo thread-safe*/
	struct file        *fpInit ;       /* which fp to init sdec module can un-init sdec while init-process is killed*/
	unsigned char	    isInit;		/* Is init members in DELIVERY_InitModule */
	unsigned char       chNum ;        /* total channels*/
	delivery_channel   *pChArray ;     /* pointer to channel structure*/
	TPK_HARDWARE_INFO_T tpInfo;

} delivery_dev ;

extern delivery_dev *delivery_device ;

long DELIVERY_ioctl          (struct file *filp, unsigned int cmd, unsigned long arg) ;
int DELIVERY_open            (struct inode *inode, struct file *filp) ;
int DELIVERY_release         (struct inode *inode, struct file *filp) ;

int IOCTL_Delivery_InitModule            (void);
int IOCTL_Delivery_UnInitModule          (void);
int IOCTL_Delivery_STDResetInfoPoll      (void);
int IOCTL_Delivery_INIT(void);
int IOCTL_Delivery_UNINIT(void);
int IOCTL_Delivery_Open(DELIVERY_DELIVERY_CHANNEL_T *pChannel);
int IOCTL_Delivery_Close(DELIVERY_DELIVERY_CHANNEL_T *pChannel);
int IOCTL_Delivery_SetTPInputConfig(DELIVERY_TP_SOURCE_CONFIG_T *pInfo);
int IOCTL_Delivery_RequestBBFrame(DELIVERY_DELIVERY_CHANNEL_T *pChannel);
int IOCTL_Delivery_CancelBBFrame(DELIVERY_DELIVERY_CHANNEL_T *pChannel);
int IOCTL_Delivery_GetBBFrameBufferInfo(DELIVERY_DELIVERY_BBF_BUFFER_INFO_T *pBBFBuffInfo);
int IOCTL_Delivery_GetBBFrameBuffer(DELIVERY_DELIVERY_GET_BBF_BUFFER_T *pBBFBuff);
int IOCTL_Delivery_ReturnFrameBuffer(DELIVERY_DELIVERY_GET_BBF_BUFFER_T *pBBFBuff);
int IOCTL_Delivery_GetTimeInfo(DELIVERY_DELIVERY_TIME_INFO_T *pTimeInfo);
int IOCTL_Delivery_SetTimeInfo(DELIVERY_DELIVERY_TIME_INFO_T *pTimeInfo);
int IOCTL_Delivery_SetSystemTimeInfo(DELIVERY_DELIVERY_SYSTEM_TIME_INFO_T *pSystemTimeInfo);
int IOCTL_Delivery_SetClockRecovery(DELIVERY_DELIVERY_CLOCK_RECOVERY_T *pClockRecovery);
int IOCTL_Delivery_SetMode(DELIVERY_DELIVERY_SET_MODE_T *pClockRecovery);
int IOCTL_Delivery_RequestData(DELIVERY_DELIVERY_CHANNEL_T *pClockRecovery);
int IOCTL_Delivery_CancelData(DELIVERY_DELIVERY_CHANNEL_T *pClockRecovery);
int IOCTL_Delivery_GetData(DELIVERY_DELIVERY_READ_DATA_T *pClockRecovery);
int IOCTL_Delivery_EnableTLVFilter(DELIVERY_DELIVERY_FILTER_ENABLE_T *ptlv_param);
int IOCTL_Delivery_AddTLVFilters(DELIVERY_DELIVERY_TLV_FILTER_INFO_T *ptlv_param);
int IOCTL_Delivery_RemoveTLVFilters(DELIVERY_DELIVERY_TLV_FILTER_INFO_T *ptlv_param);
int IOCTL_Delivery_GetNTP(DELIVERY_DELIVERY_TLV_NTP_T *channel);
int IOCTL_Delivery_GetBootStatus(DELIVERY_DELIVERY_GET_BOOT_STATUS_T *pData);
int IOCTL_Delivery_SaveBootStatus(void);
int IOCTL_Delivery_AdjustStc(DELIVERY_ADJUST_STC_T *pInfo);

#endif
