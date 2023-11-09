/*==========================================================================
    * Copyright (c)      Realtek Semiconductor Corporation, 2006
  * All rights reserved.
  * ========================================================================*/

/*================= File Description =======================================*/

/**
 * @file
 * 	This file is for vip ring-buffer flow control related functions.
 *
 * @author 	Wei Yuan Hsu
 * @date 	20210715
 * @version 1
 */

/****************************** Header files ******************************/
// platform
#include <rtd_log/rtd_module_log.h>
#include <mach/rtk_platform.h>
#include <linux/string.h>
#include <linux/kernel.h>
// library
#include <tvscalercontrol/vip/vip_ring_buffer.h>

/****************************** Configurations ******************************/
#define TAG_NAME "VIP_RINGBUF"

/****************************** Functions ******************************/

#if 0 // SAMPLE TO USE THIS RING BUFFER
VIP_EASY_RING_BUF_INIT_T RingBufInitParam = {
	.Length = myRingBufferLength,
	.Name = "myRingBufferName"
};

// in your cdoe
{
	// declare and initialize
	VIP_EASY_RING_BUF_T myRingBuf;
	myRingBuf.FuncInit = &VipEasyRingBuf_Init;
	myRingBuf.FuncInit((void *)&myRingBuf, (void *)&RingBufInitParam);

	// ready to use
	myRingBuf.FuncGetIdx((void *)&myRingBuf, VIP_RINGBUF_PTR_READ);
}
#endif

/**
 * @brief 
 * ring buffer function, initialization
 * @param pSelf 
 * structure pointer
 * @param ExParams 
 * pointer to the extra parameter
 * @return char 
 * status
 */
char VipEasyRingBuf_Init(void *pSelf, void *ExParams) {
	char Ret = 0;
	VIP_EASY_RING_BUF_T *pThis = (VIP_EASY_RING_BUF_T *)pSelf;
	VIP_EASY_RING_BUF_INIT_T *pInitParam = (VIP_EASY_RING_BUF_INIT_T *)ExParams;

	if( pInitParam->Length <= 2 ) {
		rtd_printk(KERN_EMERG, TAG_NAME, "RingBuf init fail, length = %d\n", pInitParam->Length);
		return -1;
	}

	// member
	spin_lock_init(&pThis->Lock);
	pThis->BufLength = pInitParam->Length;
	pThis->ReadPtr = 0;
	pThis->WritePtr = 1;
	memset(pThis->Name, '\0', VIP_RINGBUF_NAME_STRLEN*sizeof(char));
	strncpy(pThis->Name, pInitParam->Name, VIP_RINGBUF_NAME_STRLEN);
	pThis->Name[VIP_RINGBUF_NAME_STRLEN-1] = '\0';

	// method
	pThis->FuncReset = &VipEasyRingBuf_Reset;
	pThis->FuncGetIdx = &VipEasyRingBuf_GetIdx;
	pThis->FuncSetIdx = &VipEasyRingBuf_SetIdx;
	pThis->FuncPeekIdx = &VipEasyRingBuf_PeekIdx;
	pThis->FuncSetIdx_Force = &VipEasyRingBuf_SetIdx_Force;

	rtd_printk(KERN_INFO, TAG_NAME, "init buffer %s with length %d done\n", pThis->Name, pThis->BufLength);
	return Ret;
}

/**
 * @brief 
 * ring buffer function, reset all status
 * @param pSelf 
 * structure pointer
 * @param ExParams 
 * pointer to the extra parameter
 * @return char 
 * status
 */
char VipEasyRingBuf_Reset(void *pSelf, void *ExParams) {
	char Ret = 0;
	VIP_EASY_RING_BUF_T *pThis = (VIP_EASY_RING_BUF_T *)pSelf;
	spin_lock_irqsave(&(pThis->Lock), pThis->_RINGBUFFlags);
	pThis->ReadPtr = 0;
	pThis->WritePtr = 1;
	spin_unlock_irqrestore(&(pThis->Lock), pThis->_RINGBUFFlags);
	return Ret;
}

/**
 * @brief 
 * ring buffer function, get the index of specific type
 * @param pSelf 
 * structure pointer
 * @param Type 
 * read or write pointer
 * @return unsigned char 
 * buffer index
 */
unsigned char VipEasyRingBuf_GetIdx(void *pSelf, char Type) {
	unsigned char retPtr = 0;
	VIP_EASY_RING_BUF_T *pThis = (VIP_EASY_RING_BUF_T *)pSelf;
	spin_lock_irqsave(&(pThis->Lock), pThis->_RINGBUFFlags);
	if(Type==VIP_RINGBUF_PTR_READ) {
		retPtr = pThis->ReadPtr;
	} else if(Type==VIP_RINGBUF_PTR_WRITE) {
		retPtr = pThis->WritePtr;
	} else {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s] get idx type %d err\n", pThis->Name, Type);
		retPtr = 255;
	}
	spin_unlock_irqrestore(&(pThis->Lock), pThis->_RINGBUFFlags);
	return retPtr;
}

/**
 * @brief 
 * ring buffer function, move the index of specific type
 * @param pSelf 
 * structure pointer
 * @param Type 
 * read or write pointer
 * @return char 
 * status
 */
char VipEasyRingBuf_SetIdx(void *pSelf, char Type) {
	int nxtPtr = 0;
	VIP_EASY_RING_BUF_T *pThis = (VIP_EASY_RING_BUF_T *)pSelf;
	spin_lock_irqsave(&(pThis->Lock), pThis->_RINGBUFFlags);
	if(Type==VIP_RINGBUF_PTR_READ) {
		nxtPtr = ((pThis->ReadPtr+1) % (pThis->BufLength));
		if( nxtPtr != pThis->WritePtr ) {
			pThis->ReadPtr = nxtPtr;
		}
	} else if(Type==VIP_RINGBUF_PTR_WRITE) {
		nxtPtr = ((pThis->WritePtr+1) % (pThis->BufLength));
		if( nxtPtr != pThis->ReadPtr ) {
			pThis->WritePtr = nxtPtr;
		}	
	}
	spin_unlock_irqrestore(&(pThis->Lock), pThis->_RINGBUFFlags);
	return 0;
}

/**
 * @brief 
 * ring buffer function, peek the index of specific type if try to move
 * note. this one WILL NOT MOVE the pointer forward
 * @param pSelf 
 * structure pointer
 * @param Type 
 * read or write pointer
 * @return unsigned char 
 * buffer index
 */
unsigned char VipEasyRingBuf_PeekIdx(void *pSelf, char Type) {
	int nxtPtr = 0;
	unsigned char retPtr = 0;
	VIP_EASY_RING_BUF_T *pThis = (VIP_EASY_RING_BUF_T *)pSelf;
	spin_lock_irqsave(&(pThis->Lock), pThis->_RINGBUFFlags);
	if(Type==VIP_RINGBUF_PTR_READ) {
		nxtPtr = ((pThis->ReadPtr+1) % (pThis->BufLength));
		if( nxtPtr == pThis->WritePtr )
			retPtr = pThis->ReadPtr;
		else
			retPtr = nxtPtr;
	} else if(Type==VIP_RINGBUF_PTR_WRITE) {
		nxtPtr = ((pThis->WritePtr+1) % (pThis->BufLength));
		if( nxtPtr == pThis->ReadPtr )
			retPtr = pThis->WritePtr;
		else
			retPtr = nxtPtr;
	}
	spin_unlock_irqrestore(&(pThis->Lock), pThis->_RINGBUFFlags);
	return retPtr;
}

/**
 * @brief 
 * ring buffer function, force to move the specific pointer forward
 * note. be careful to use this
 * @param pSelf 
 * structure pointer
 * @param Type 
 * read or write pointer
 * @return unsigned char 
 * status
 */
unsigned char VipEasyRingBuf_SetIdx_Force(void *pSelf, char Type) {
	int nxtPtr = 0;
	VIP_EASY_RING_BUF_T *pThis = (VIP_EASY_RING_BUF_T *)pSelf;
	spin_lock_irqsave(&(pThis->Lock), pThis->_RINGBUFFlags);
	if(Type==VIP_RINGBUF_PTR_READ) {
		nxtPtr = ((pThis->ReadPtr+1) % (pThis->BufLength));
		pThis->ReadPtr = nxtPtr;
	} else if(Type==VIP_RINGBUF_PTR_WRITE) {
		nxtPtr = ((pThis->WritePtr+1) % (pThis->BufLength));
		pThis->WritePtr = nxtPtr;
	}
	spin_unlock_irqrestore(&(pThis->Lock), pThis->_RINGBUFFlags);
	return 0;
}
