/******************************************************************************
 *
 *   Copyright(c) 2014 Realtek Semiconductor Corp. All rights reserved.
 *
 *   @author yungjui.lee@realtek.com
 *
 *****************************************************************************/

#ifndef _RTK_PIN_
#define _RTK_PIN_
#include "audio_inc.h"

//typedef struct _MultiPin MultiPin;
typedef struct _MultiPin{
    Pin* pPinObj;
    Pin** pPinObjArr;
}MultiPin;
Pin* new_multiPin(UINT32 bufSize, UINT32 listSize);
Pin* new_multiPin_DEC_QuickShow(UINT32 bufSize, UINT32 listSize, UINT32 *nonCachedAddr, UINT32 *phyAddr);

//typedef struct _PacketPin PacketPin;
typedef struct _PacketPin{
    Pin* pPinObj;
    Pin* pDataPinObj;

    void (*InitPacketPin)(Pin*, int);
}PacketPin;

Pin* new_packetPin(UINT32 bufSize, UINT32 listSize);
Pin* new_packetPin_optee(UINT32 bufSize, UINT32 listSize, UINT32* shmva, UINT32* shmpa);
Pin* new_packetPin_Cached(UINT32 bufSize, UINT32 listSize);
Pin* new_pin_QuickShow(UINT32 bufSize, UINT32 *nonCachedAddr, UINT32 *phyAddr);

uint32_t IsPtrValid(int64_t ptr, Pin* pPinObj);
void ShowPinInfo(Pin* pPinObj);

uint64_t*  ConvertPhy2Virtual(Pin* pPinObj, int64_t phyAddress);

uint64_t*  ConvertVirtual2Phy(Pin* pPinObj, int64_t VirtualAddress);
void MyMemWrite(BYTE* pCurr, BYTE* Data, long Size);
void MyMemRead(BYTE* pCurr, BYTE* Data, int32_t Size);


#endif /*_RTK_PIN_*/
