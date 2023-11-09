/******************************************************************************
 *
 *   Copyright(c) 2014 Realtek Semiconductor Corp. All rights reserved.
 *
 *   @author yungjui.lee@realtek.com
 *
 *****************************************************************************/
#include "audio_inc.h"
#include "audio_pin.h"
#include "AudioInbandAPI.h"
#include <linux/slab.h>
#include "hal_common.h"
#include <linux/pageremap.h>


#define CH_INFO               0
#define CH_DATA0              1

extern uint32_t rtkaudio_alloc_uncached(int size, unsigned char** uncached_addr_info);
extern int rtkaudio_free(unsigned int phy_addr);

typedef struct {
    UINT8               m_flag;
    UINT8               m_bNeedSizeNotifier;
    UINT32              m_requestSize;

    // header addresses
    UINT8               *m_pBufferHeaderAddr;    // for free (free must use cache-able address)
    RINGBUFFER_HEADER   *m_pBufferHeader;        // point to ring buffer property structure
    RINGBUFFER_HEADER   *m_pBufferHeader_ext;    // point to ring buffer property structure
    UINT32              m_bufferHeaderSize_ext;
    REFCLOCK            m_pRefClock;
    UINT32              m_refClockSize;
    UINT32              m_bufferHeaderPhyAddr;
    SINT32              m_headerPhyAddrDiff;     // value = PhysicalAddress - VirtualAddress
    UINT32              m_headerPhyAddrDiff_ext; // value = Physicaladdress - Virtualaddress

    // ring buffer addresses and size
    SINT32              m_bufferSize;
    UINT8               *m_pBufferLower;
    UINT8               *m_pBufferUpper;
    UINT8               *m_pBufferNonCachedLower;
    UINT32              m_bufferPhyAddr;
    SINT32              m_bufferPhyAddrDiff;
} ALLOCATOR;

#define RING_CONSTRUCT  (0x2) /* BufferHeader created*/
#define RING_SET_BUFFER (0x4) /* BufferSize setted*/
#define RING_COMMIT     (0x1) /* Buffer created*/

#define PIN_HAS_FLAG(flag, bit)     (((flag) & (bit))!=0)
#define PIN_SET_FLAG(flag, bit)     ((flag)|= (bit))
#define PIN_RESET_FLAG(flag, bit)   ((flag)&= (~bit))


char libk_printBuffer[256];
char libk_printBuffer2[256];

void Destroy(ALLOCATOR* alloc)
{
    if(PIN_HAS_FLAG(alloc->m_flag, RING_COMMIT) && alloc->m_pBufferLower)
    {
        rtkaudio_free(alloc->m_bufferPhyAddr);
        alloc->m_pBufferLower = 0;
        PIN_RESET_FLAG(alloc->m_flag, RING_COMMIT);
    }
    if(PIN_HAS_FLAG(alloc->m_flag, RING_CONSTRUCT) && alloc->m_pBufferHeaderAddr)
    {
        rtkaudio_free(alloc->m_bufferHeaderPhyAddr);
        alloc->m_pBufferHeaderAddr = 0;
        PIN_RESET_FLAG(alloc->m_flag, RING_CONSTRUCT);
    }
    alloc->m_bufferSize = 0;
    PIN_RESET_FLAG(alloc->m_flag, RING_SET_BUFFER);
}

static UINT32 FreeAllocator(ALLOCATOR* alloc)
{
    if(alloc == NULL) return S_FALSE;

    Destroy(alloc);
    kfree(alloc);
    alloc = NULL;
    return S_OK;
}

static UINT32 ConstructorQuickShow(ALLOCATOR* alloc, UINT32 *nonCachedAddr, UINT32 *phyAddr)
{
    alloc->m_flag = 0;
    alloc->m_bNeedSizeNotifier = FALSE;
    alloc->m_pBufferHeader = 0;
    alloc->m_bufferSize = 0;
    alloc->m_pBufferLower = 0;
    alloc->m_pBufferUpper = 0;

    alloc->m_pBufferHeaderAddr = (UINT8*)nonCachedAddr;
    alloc->m_pBufferHeader = (RINGBUFFER_HEADER*)nonCachedAddr;
    alloc->m_bufferHeaderPhyAddr = *phyAddr;

    *nonCachedAddr += sizeof(RINGBUFFER_HEADER);
    *phyAddr += sizeof(RINGBUFFER_HEADER);

    if(alloc->m_pBufferHeader)
        memset(alloc->m_pBufferHeader, 0, sizeof(RINGBUFFER_HEADER));

    PIN_SET_FLAG(alloc->m_flag, RING_CONSTRUCT);
    return S_OK;
}

static UINT32 SetBufferQuickShow(ALLOCATOR* alloc, UINT32 size)
{
    if(!PIN_HAS_FLAG(alloc->m_flag, RING_CONSTRUCT)) return S_FALSE;

    alloc->m_bufferSize = size;

    PIN_SET_FLAG(alloc->m_flag, RING_SET_BUFFER);
    return S_OK;
}

static UINT32 CommitQuickShow(ALLOCATOR* alloc, UINT32 *nonCachedAddr, UINT32 *phyAddr)
{
    if(!PIN_HAS_FLAG(alloc->m_flag, RING_SET_BUFFER)) return S_FALSE;
    if(PIN_HAS_FLAG(alloc->m_flag, RING_COMMIT)) return S_FALSE;
    if(alloc->m_bufferSize == 0)
    {
        ERROR("SetBuffer has not been called\n");
        return S_FALSE;
    }

    alloc->m_pBufferLower = (UINT8*)nonCachedAddr;
    alloc->m_pBufferNonCachedLower = (UINT8*)nonCachedAddr;
    alloc->m_bufferPhyAddr = *phyAddr;

    *nonCachedAddr += alloc->m_bufferSize;
    *phyAddr += alloc->m_bufferSize;

    alloc->m_pBufferUpper = alloc->m_pBufferLower + alloc->m_bufferSize;

    PIN_SET_FLAG(alloc->m_flag, RING_COMMIT);
    return S_OK;
}

static ALLOCATOR* CreateAllocatorQuickShow(UINT32 size, UINT32 *nonCachedAddr, UINT32 *phyAddr)
{
    ALLOCATOR* alloc;
    if(size == 0) return NULL;

    alloc = (ALLOCATOR*)kmalloc(sizeof(ALLOCATOR), GFP_KERNEL);
    if(alloc == NULL)
    {
        return NULL;
    }
    memset(alloc, 0, sizeof(ALLOCATOR));

    ConstructorQuickShow(alloc, nonCachedAddr, phyAddr);
    SetBufferQuickShow(alloc, size);
    CommitQuickShow(alloc, nonCachedAddr, phyAddr);

    if(!PIN_HAS_FLAG(alloc->m_flag, RING_COMMIT))
    {
        FreeAllocator(alloc);
        return NULL;
    }
    return alloc;
}

static UINT32 Constructor(ALLOCATOR* alloc)
{
    alloc->m_flag = 0;
    alloc->m_bNeedSizeNotifier = FALSE;
    alloc->m_pBufferHeader = 0;
    alloc->m_bufferSize = 0;
    alloc->m_pBufferLower = 0;
    alloc->m_pBufferUpper = 0;

    //INFO("[%s] allocate RINGBUFFER_HEADER \n", __FUNCTION__);
    alloc->m_bufferHeaderPhyAddr = rtkaudio_alloc_uncached(sizeof(RINGBUFFER_HEADER), &(alloc->m_pBufferHeaderAddr));

    if(alloc->m_pBufferHeaderAddr == NULL)
    {
        ERROR("Constructor memory alloc fail!\n");
        return S_FALSE;
    }
    alloc->m_pBufferHeader = (RINGBUFFER_HEADER*)alloc->m_pBufferHeaderAddr;

    if(alloc->m_pBufferHeader)
        memset(alloc->m_pBufferHeader, 0, sizeof(RINGBUFFER_HEADER));

    PIN_SET_FLAG(alloc->m_flag, RING_CONSTRUCT);
    return S_OK;
}

static UINT32 SetBuffer(ALLOCATOR* alloc, UINT32 size)
{
    if(!PIN_HAS_FLAG(alloc->m_flag, RING_CONSTRUCT)) return S_FALSE;

    alloc->m_bufferSize = size;
    // set write ptr
    IPC_WriteU32((BYTE*)&(alloc->m_pBufferHeader->writePtr), (UINT32)alloc->m_bufferPhyAddr);

    PIN_SET_FLAG(alloc->m_flag, RING_SET_BUFFER);
    return S_OK;
}

static UINT32 Commit(ALLOCATOR* alloc)
{
    SINT32 i;

    if(!PIN_HAS_FLAG(alloc->m_flag, RING_SET_BUFFER)) return S_FALSE;
    if(PIN_HAS_FLAG(alloc->m_flag, RING_COMMIT)) return S_FALSE;
    if(alloc->m_bufferSize == 0)
    {
        ERROR("SetBuffer has not been called\n");
        return S_FALSE;
    }
    //INFO("[%s] allocate m_bufferSize \n", __FUNCTION__);
    alloc->m_bufferPhyAddr = rtkaudio_alloc_uncached(alloc->m_bufferSize*sizeof(UINT8), &(alloc->m_pBufferNonCachedLower));
    if(alloc->m_pBufferNonCachedLower == NULL)
    {
        ERROR("Commit memory alloc fail!\n");
        return S_FALSE;
    }
    alloc->m_pBufferLower = alloc->m_pBufferNonCachedLower;
    DEBUG("Commit (p:%x,v:%px)\n", alloc->m_bufferPhyAddr, alloc->m_pBufferNonCachedLower);

    alloc->m_pBufferUpper = alloc->m_pBufferLower + alloc->m_bufferSize;

    IPC_WriteU32((BYTE*)&(alloc->m_pBufferHeader->beginAddr), alloc->m_bufferPhyAddr);
    IPC_WriteU32((BYTE*)&(alloc->m_pBufferHeader->size), alloc->m_bufferSize);
    IPC_WriteU32((BYTE*)&(alloc->m_pBufferHeader->writePtr),alloc->m_bufferPhyAddr);

    // set read ptr to read ptr
    for(i = 0; i < 4; i++) {
        IPC_WriteU32((BYTE*)&(alloc->m_pBufferHeader->readPtr[i]),
                      IPC_ReadU32((BYTE*)&alloc->m_pBufferHeader->writePtr));
    }
    PIN_SET_FLAG(alloc->m_flag, RING_COMMIT);
    return S_OK;
}

static UINT32 GetBufferHeader(ALLOCATOR* alloc, RINGBUFFER_HEADER **vAddr, UINT32 *pAddr)
{
    if(!PIN_HAS_FLAG(alloc->m_flag, RING_COMMIT))
    {
        *vAddr = NULL;
        *pAddr = 0x0;
        return S_FALSE;
    }
    *vAddr = alloc->m_pBufferHeader;
    *pAddr = alloc->m_bufferHeaderPhyAddr;
    //DEBUG("[%s %d] m_pBufferHeader= %px, m_bufferHeaderPhyAddr= %x\n", __func__, __LINE__, alloc->m_pBufferHeader, alloc->m_bufferHeaderPhyAddr);
    return S_OK;
}

static UINT32 GetBufferAddress(ALLOCATOR* alloc, UINT8 **vAddr, UINT32 *pAddr)
{
    if(!PIN_HAS_FLAG(alloc->m_flag, RING_COMMIT))
    {
        *vAddr = NULL;
        *pAddr = 0x0;
        return S_FALSE;
    }
    *vAddr = alloc->m_pBufferLower;
    *pAddr = alloc->m_bufferPhyAddr;
    return S_OK;
}

static ALLOCATOR* CreateAllocator(UINT32 size)
{
    ALLOCATOR* alloc;
    if(size == 0) return NULL;

    alloc = (ALLOCATOR*)kmalloc(sizeof(ALLOCATOR), GFP_KERNEL);
    if(alloc == NULL)
    {
        return NULL;
    }
    memset(alloc, 0, sizeof(ALLOCATOR));

    //INFO("[%s] create allocate  \n", __FUNCTION__);

    Constructor(alloc);
    SetBuffer(alloc, size);
    Commit(alloc);

    if(!PIN_HAS_FLAG(alloc->m_flag, RING_COMMIT))
    {
        FreeAllocator(alloc);
        return NULL;
    }
    return alloc;
}

UINT32 GetReadSize(Pin* pPinObj, UINT32 Idx)
{
    ALLOCATOR *alloc = (ALLOCATOR*)pPinObj->allocator;
    SINT32 readSize = (SINT32)(pPinObj->GetWritePtr(pPinObj) - pPinObj->GetReadPtr(pPinObj, Idx));
    if(readSize < 0) readSize += alloc->m_bufferSize;
    return (UINT32)readSize;
}

UINT32 GetWriteSize(Pin* pPinObj)
{
    ALLOCATOR *alloc = (ALLOCATOR*)pPinObj->allocator;
    SINT32 writeSize = ((int32_t)(~0UL >> 1)); //LONG_MAX;
    SINT32 tWriteSize, i;
    RINGBUFFER_HEADER* bufHeader = alloc->m_pBufferHeader;
    UINT32 numOfReadPtr = IPC_ReadU32((BYTE*)&(bufHeader->numOfReadPtr));
    /* write_size = total_size - read_size -1 */
    if(numOfReadPtr == 1) {
        tWriteSize = alloc->m_bufferSize - pPinObj->GetReadSize(pPinObj, 0) -1;
        if(tWriteSize < writeSize) writeSize = tWriteSize;
    } else {
        for(i = 0; i < (int)numOfReadPtr; i++) {
            tWriteSize = alloc->m_bufferSize - pPinObj->GetReadSize(pPinObj, i) -1;
    if(tWriteSize < writeSize) writeSize = tWriteSize;
        }
    }

    return (UINT32)writeSize;
}

UINT8* GetReadPtr(Pin* pPinObj, uint32_t Idx)// function return virtual address
{
    ALLOCATOR *alloc = (ALLOCATOR*)pPinObj->allocator;
    RINGBUFFER_HEADER* bufHeader = alloc->m_pBufferHeader;

    return (UINT8*)(alloc->m_pBufferLower + IPC_ReadU32((BYTE*)&(bufHeader->readPtr[Idx])) - alloc->m_bufferPhyAddr);
}

UINT32 SetReadPtr(Pin* pPinObj, UINT8* readPtr, UINT32 Idx)// function accept virtual address
{
    ALLOCATOR *alloc = (ALLOCATOR*)pPinObj->allocator;
    RINGBUFFER_HEADER* bufHeader = alloc->m_pBufferHeader;

    IPC_WriteU32((BYTE*)&(bufHeader->readPtr[Idx]), alloc->m_bufferPhyAddr + (UINT32)(readPtr - alloc->m_pBufferLower));
    return S_OK;
}

UINT32 SetNumOfReadPtr(Pin* pPinObj, UINT32 nReadPtr)// function accept virtual address
{
    ALLOCATOR *alloc = (ALLOCATOR*)pPinObj->allocator;
    RINGBUFFER_HEADER* bufHeader = alloc->m_pBufferHeader;

    IPC_WriteU32((BYTE*)&(bufHeader->numOfReadPtr), (UINT32)nReadPtr);
    return S_OK;
}

UINT8* GetWritePtr(Pin* pPinObj)// function return virtual address
{
    ALLOCATOR *alloc = (ALLOCATOR*)pPinObj->allocator;
    RINGBUFFER_HEADER* bufHeader = alloc->m_pBufferHeader;

    return (UINT8*)(alloc->m_pBufferLower + IPC_ReadU32((BYTE*)&(bufHeader->writePtr)) - alloc->m_bufferPhyAddr);
}

uint64_t*  ConvertPhy2Virtual(Pin* pPinObj, int64_t phyAddress)
{
    ALLOCATOR *alloc = (ALLOCATOR*)pPinObj->allocator;

    return (uint64_t*) (phyAddress - alloc->m_bufferPhyAddr + alloc->m_pBufferLower);
}

uint64_t*  ConvertVirtual2Phy(Pin* pPinObj, int64_t VirtualAddress)
{
    ALLOCATOR *alloc = (ALLOCATOR*)pPinObj->allocator;
    SINT32  bufferPhyAddrDiff = alloc->m_bufferPhyAddr - (uint64_t)alloc->m_pBufferLower;

    return (uint64_t*) (VirtualAddress + bufferPhyAddrDiff);
}

UINT32 SetWritePtr(Pin* pPinObj, UINT8* writePtr)// function accept virtual address
{
    ALLOCATOR *alloc = (ALLOCATOR*)pPinObj->allocator;
    RINGBUFFER_HEADER* bufHeader = alloc->m_pBufferHeader;

    IPC_WriteU32((BYTE*)&(bufHeader->writePtr), alloc->m_bufferPhyAddr + (UINT32)(writePtr-alloc->m_pBufferLower));
    return S_OK;
}
UINT32 GetBufHeader(Pin* pPinObj, void** vAddr, UINT32 *pAddr)
{
    ALLOCATOR *alloc = (ALLOCATOR*)pPinObj->allocator;
    return GetBufferHeader(alloc, (RINGBUFFER_HEADER**)vAddr, pAddr);
}
UINT32 GetBufAddress(Pin* pPinObj, UINT8** vAddr, UINT32 *pAddr)
{
    ALLOCATOR *alloc = (ALLOCATOR*)pPinObj->allocator;
    return GetBufferAddress(alloc, vAddr, pAddr);
}
UINT32 GetBufferSize(Pin* pPinObj)
{
    ALLOCATOR *alloc = (ALLOCATOR*)pPinObj->allocator;
    return alloc->m_bufferSize;
}
void SetBufferSize(Pin* pPinObj, UINT32 size)
{
    ALLOCATOR *alloc = (ALLOCATOR*)pPinObj->allocator;
    alloc->m_bufferSize = size;
    alloc->m_pBufferUpper = alloc->m_pBufferLower + alloc->m_bufferSize;
    // set write ptr
    IPC_WriteU32((BYTE*)&(alloc->m_pBufferHeader->size), alloc->m_bufferSize);
}
UINT32 GetListSize(Pin* pPinObj)
{
    return pPinObj->listSize;
}
UINT32 GetBuffer(Pin* pPinObj, UINT8** ppBuffer, UINT32* pSize)
{
    ALLOCATOR *alloc = (ALLOCATOR*)pPinObj->allocator;
    if(ppBuffer != NULL)
        *ppBuffer = alloc->m_pBufferNonCachedLower;
    if(pSize != NULL)
        *pSize = alloc->m_bufferSize;
    return S_OK;
}
UINT32 GetPhysicalWriteAddress(Pin* pPinObj)
{
    UINT32 pAddr;
    RINGBUFFER_HEADER *vAddr;

    pPinObj->GetBufHeader(pPinObj, (void**)&vAddr, &pAddr);

    return IPC_ReadU32((BYTE*)(&(vAddr->writePtr)));
}

void delete_pin(Pin* pPinObj)
{
    if(pPinObj == NULL) return;

    if(pPinObj->allocator != NULL)
        FreeAllocator((ALLOCATOR*)pPinObj->allocator);

    kfree(pPinObj);
    return;
}

void delete_multiPin(Pin* pPinObj)
{
    SINT32 i;
    MultiPin* pMObj = NULL;
    Pin **pinArr = NULL;

    if(pPinObj == NULL) return;

    pMObj = (MultiPin*)pPinObj->pDerivedObj;
    if(pMObj == NULL) return;

    pinArr = pMObj->pPinObjArr;

    if(pMObj->pPinObjArr != NULL)
    {
        for(i = 0; i < pPinObj->listSize-1; i++)
        {
            Pin *pPin = (Pin*)pMObj->pPinObjArr[i];
            if(pPin != NULL)
                pPin->Delete(pPin);
            pMObj->pPinObjArr[i] = NULL;
        }
    }

    if(pinArr)
        kfree(pinArr);
    kfree(pMObj);

    return delete_pin(pPinObj);
}

void MyMemWrite(BYTE* pCurr, BYTE* Data, long Size)
{
    long value;
    BYTE*   start = Data;
    /*BUG_ON((Size % 4) == 0);*/

    Size /= 4;
    while(Size--) {
        value = *((long*)start);
        IPC_WriteU32(pCurr, value);
        pCurr += 4; start += 4;
    }
}
static void MyMemWriteNoRevert(BYTE* pCurr, BYTE* Data, long Size)
{
    if (Size%4 != 0){
        pr_info("wrong size %d\n", Size);
        //BUG_ON((Size % 4) == 0);
    }
    memcpy(pCurr, Data, Size);
}

void MyMemRead(BYTE* pCurr, BYTE* Data, int32_t Size)
{
    int32_t value;
    BYTE*   start = Data;

    if((Size % 4) != 0)
    {
        pr_info("wrong size %d\n", Size);
        //while(1);
    }

    /*BUG_ON((Size % 4) == 0);*/

    Size /= 4;
    while(Size--) {
        value = IPC_ReadU32(pCurr);
        *((uint32_t*)start) = value;
        pCurr += 4; start += 4;
    }
}

static void MyMemReadNonInverse(BYTE* pCurr, BYTE* Data, long Size)
{
    /*long value;*/
    /*BYTE*   start = Data;*/

    //BUG_ON((Size % 4) == 0);
    memcpy(Data, pCurr, Size);

    /*
    Size /= 4;
    while(Size--) {
        value = IPC_ReadU32(pCurr);
        *((long*)start) = value;
        pCurr += 4; start += 4;
}
*/

}

/*<! BS data does not need revert */
static void MyMemReadNoRevert(BYTE* pCurr, BYTE* Data, int32_t Size)
{
    /*long value;*/
    //assert((Size % 4) == 0);

    memcpy(Data, pCurr, Size);
}

void ShowPinInfo(Pin* pPinObj)
{
    UINT8 *readPtr, *base,  *writePtr;
    UINT32 bufferSize;
    pPinObj->GetBuffer(pPinObj, &base, &bufferSize);
    readPtr  = pPinObj->GetReadPtr(pPinObj, 0);
    writePtr = pPinObj->GetWritePtr(pPinObj);
    INFO("7base = %llx sz = %d  rp %llx wp %llx \n", (uint64_t)base, bufferSize, (uint64_t)readPtr, (uint64_t)writePtr);
}

UINT32 MemCopyFromReadPtr(Pin* pPinObj, UINT8* dest, UINT32 size, UINT32 type)
{
    UINT8 *readPtr, *base, *limit;
    UINT32 bufferSize = 0;
    UINT32 size2 = 0;
    pPinObj->GetBuffer(pPinObj, &base, &bufferSize);
    limit = base + bufferSize;

    if (size%4 != 0 && type != AUDIO_BS)
    {
        pr_alert("[%s] wrong size:%d\n", __FUNCTION__, size);
        ShowPinInfo(pPinObj);
        //while(1);
        /*assert((size%4)==0);*/
    }

    readPtr = pPinObj->GetReadPtr(pPinObj, 0);
    if ((uint64_t)readPtr%4 != 0 && type != AUDIO_BS)
    {
        pr_alert("[%s] wrong ptr:%llx\n", __FUNCTION__, (uint64_t)readPtr);
        ShowPinInfo(pPinObj);
        //while(1);
        /*BUG_ON(((int)readPtr%4)==0);*/
    }
    if(readPtr + size > limit) {
        if (type == AUDIO_BS) {
            MyMemReadNoRevert(readPtr, dest, (limit - readPtr));
            MyMemReadNoRevert(base, dest + (limit - readPtr), size - (limit - readPtr));
        }
        else
        {
            size2 = (limit - readPtr);
            //MyMemRead(readPtr, dest, (limit - readPtr));
            if(size2 % 4)
            {
                pr_info("read size %d ori %d \n", size2, size);
                ShowPinInfo(pPinObj);
                //while(1);
            }
            MyMemRead(readPtr, dest, size2);
            size2 = (size - (limit - readPtr));
            if(size2 % 4)
            {
                pr_info("read size %d ori %d \n", size2, size);
                ShowPinInfo(pPinObj);
                //while(1);
            }
            //MyMemRead(base, dest + (limit - readPtr), size - (limit - readPtr));
            MyMemRead(base, dest + (limit - readPtr), size2);
        }
    } else {
        if (type == AUDIO_BS)
            MyMemReadNoRevert(readPtr, dest, size);
        else
        {
            if(size % 4)
            {
                pr_info("read size %d ori %d \n", size2, size);
                ShowPinInfo(pPinObj);
                //while(1);
            }
            MyMemRead(readPtr, dest, size);
        }
    }
    return S_OK;
}

UINT32 MemCopyFromReadPtrNonInVerse(Pin* pPinObj, UINT8* dest, UINT32 size)
{
    UINT8 *readPtr, *base, *limit;
    UINT32 bufferSize;
    pPinObj->GetBuffer(pPinObj, &base, &bufferSize);
    limit = base + bufferSize;

    readPtr = pPinObj->GetReadPtr(pPinObj, 0);
    if(readPtr + size > limit) {
        MyMemReadNonInverse(readPtr, dest, (limit - readPtr));
        MyMemReadNonInverse(base, dest + (limit - readPtr), size - (limit - readPtr));
    } else {
        MyMemReadNonInverse(readPtr, dest, size);
    }
    return S_OK;
}

UINT32 MemCopyToWritePtr(Pin* pPinObj, UINT8* src, UINT32 size)
{
    UINT8 *writePtr, *base, *limit;
    UINT32 bufferSize;
    pPinObj->GetBuffer(pPinObj, &base, &bufferSize);
    limit = base + bufferSize;
    if (size%4 != 0)
    {
        pr_alert("[%s] wrong size:%d\n", __FUNCTION__, size);
        /*BUG_ON((size%4)==0);*/
    }

    writePtr = pPinObj->GetWritePtr(pPinObj);
    if ((int64_t)writePtr%4 != 0)
    {
        pr_alert("[%s] wrong ptr:%llx\n", __FUNCTION__, (uint64_t)writePtr);
        /*BUG_ON(((int)writePtr%4)==0);*/
    }
    if(writePtr + size > limit) {
        MyMemWrite(writePtr, src, (limit - writePtr));
        MyMemWrite(base, src + (limit - writePtr), size - (limit - writePtr));
    } else {
        MyMemWrite(writePtr, src, size);
    }
    return S_OK;
}

UINT32 MemCopyToWritePtrNoInverse(Pin* pPinObj, UINT8* src, UINT32 size)
{
    UINT8 *writePtr, *base, *limit;
    UINT32 bufferSize;
    pPinObj->GetBuffer(pPinObj, &base, &bufferSize);
    limit = base + bufferSize;
    if (size%4 != 0)
    {
        pr_alert("[%s] wrong size:%d\n", __FUNCTION__, size);
        /*BUG_ON((size%4)==0);*/
    }
    writePtr = pPinObj->GetWritePtr(pPinObj);
    if ((int64_t)writePtr%4 != 0)
    {
        pr_alert("[%s] wrong ptr:%llx\n", __FUNCTION__, (uint64_t)writePtr);
        /*BUG_ON(((int)writePtr%4)==0);*/
    }
    if(writePtr + size > limit) {
        MyMemWriteNoRevert(writePtr, src, (limit - writePtr));
        MyMemWriteNoRevert(base, src + (limit - writePtr), size - (limit - writePtr));
    } else {
        MyMemWriteNoRevert(writePtr, src, size);
    }
    return S_OK;
}

uint32_t IsPtrInReage(int64_t ptr, int64_t base , int64_t limit)
{
   if( (ptr < base) ||  (ptr >=  ( limit)))
    {
        return FALSE;
    }
   return TRUE;
}

uint32_t IsPtrValid(int64_t ptr, Pin* pPinObj)
{

    UINT8 *base, *limit;
    UINT32 bufferSize;
    pPinObj->GetBuffer(pPinObj, &base, &bufferSize);
    limit = base + bufferSize;
    //FIXME: pointer to int is incorrect.
    return IsPtrInReage(ptr, (int64_t)(&(*base)), (int64_t)(&(*limit)));
}

UINT32 AddReadPtr(Pin* pPinObj, UINT32 size)
{
    UINT8 *readPtr, *base, *limit;
    UINT32 bufferSize;
    pPinObj->GetBuffer(pPinObj, &base, &bufferSize);
    limit = base + bufferSize;
    /*static int i = 0;*/

#if 0
    if (size%4 != 0)
    {
        if (i % 200 == 0){
            ShowPinInfo(pPinObj);
            MY_LIBK_ALERT("[%s] wrong size:%d\n", __FUNCTION__, size);
        }
        //while(1);
        //BUG_ON((size%4)==0);
    }
#endif

    readPtr = pPinObj->GetReadPtr(pPinObj, 0);

    //FIXME: pointer to int is incorrect.
    if(!IsPtrValid((int64_t)(&(*readPtr)), pPinObj))
    {
        pr_alert("error ptr b %llx \n", (uint64_t)readPtr);
        ShowPinInfo(pPinObj);
        /*BUG_ON(1==0);*/
    }

#if 0
    if ((int)readPtr%4 != 0)
    {
        if (i % 200 == 0){
            ShowPinInfo(pPinObj);
            MY_LIBK_ALERT("[%s] wrong ptr:%x\n", __FUNCTION__, readPtr);
        }
        i++;
        //while(1);
        //assert(((int)readPtr%4)==0);
    }
#endif
    readPtr += size;
    if(readPtr >= limit)
        readPtr = base + (readPtr - limit);

    //FIXME: pointer to int is incorrect.
    if(!IsPtrValid((int64_t)(&(*readPtr)), pPinObj))
    {
       pr_alert("af error ptr %llx \n", (uint64_t)readPtr);
       ShowPinInfo(pPinObj);
       /*BUG_ON(1==0);*/
    }

    return pPinObj->SetReadPtr(pPinObj, readPtr, 0);
}

UINT32 AddWritePtr(Pin* pPinObj, UINT32 size)
{
    UINT8 *writePtr, *base, *limit;
    UINT32 bufferSize;
    pPinObj->GetBuffer(pPinObj, &base, &bufferSize);
    limit = base + bufferSize;
    if (size%4 != 0)
    {
        pr_alert("[%s] wrong size:%d\n", __FUNCTION__, size);
        /*BUG_ON((size%4)==0);*/
    }

    writePtr = pPinObj->GetWritePtr(pPinObj);
    if ((int64_t)writePtr%4 != 0)
    {
        pr_alert("[%s] wrong ptr:%llx\n", __FUNCTION__, (uint64_t)writePtr);
        /*assert(((int)writePtr%4)==0);*/
    }
    writePtr += size;
    if(writePtr >= limit)
        writePtr = base + (writePtr - limit);
    return pPinObj->SetWritePtr(pPinObj, writePtr);
}

UINT32 CheckReadableSize(Pin *pPinObj, UINT32 sizeToRead)
{
    if(pPinObj->GetReadSize(pPinObj, 0) < sizeToRead) {
        /* DEBUG("[UD] rp %x, wp %x\n", pPinObj->GetReadPtr(pPinObj, 0), pPinObj->GetWritePtr(pPinObj)); */
        return ERR_UNDERFLOW;
    }
    return NO_ERR;
}

UINT32 CheckWritableSize(Pin *pPinObj, UINT32 sizeToWrite)
{
    if(pPinObj->GetWriteSize(pPinObj) < sizeToWrite) {
        //DEBUG("[OF] rp %x, wp %x\n", pPinObj->GetReadPtr(pPinObj, 0), pPinObj->GetWritePtr(pPinObj));
        return ERR_OVERFLOW;
    }
    return NO_ERR;
}

UINT32 reset_pin_multiPin(Pin *pPinObj)
{
    UINT8 *base;
    UINT32 bufferSize;

    if (pPinObj == NULL) {
        return -1;
    }

    //AUDIO_INFO("reset_pin_multiPin\n");

    pPinObj->GetBuffer(pPinObj, &base, &bufferSize);
    pPinObj->SetReadPtr(pPinObj, base,  0);
    pPinObj->SetReadPtr(pPinObj, base,  1);
    pPinObj->SetReadPtr(pPinObj, base,  2);
    pPinObj->SetReadPtr(pPinObj, base,  3);
    pPinObj->SetWritePtr(pPinObj,  base);
    memset(base, 0, bufferSize);// clean buffer
    //AUDIO_INFO("[0]  base = %x size = %x\n", (UINT32)base, bufferSize);

    if(pPinObj->listSize > 1) {
        int i;
        MultiPin* pMObj = (MultiPin*)pPinObj->pDerivedObj;
        for(i = 1; i < pPinObj->listSize ; i++) {
            pMObj->pPinObjArr[i-1]->GetBuffer( pMObj->pPinObjArr[i-1], &base, &bufferSize);
            //AUDIO_INFO("[%d]  base = %x size = %x\n", i, (UINT32)base, bufferSize);
            pMObj->pPinObjArr[i-1]->SetReadPtr( pMObj->pPinObjArr[i-1], base, 0);
            pMObj->pPinObjArr[i-1]->SetReadPtr( pMObj->pPinObjArr[i-1], base, 1);
            pMObj->pPinObjArr[i-1]->SetReadPtr( pMObj->pPinObjArr[i-1], base, 2);
            pMObj->pPinObjArr[i-1]->SetReadPtr( pMObj->pPinObjArr[i-1], base, 3);
            pMObj->pPinObjArr[i-1]->SetWritePtr( pMObj->pPinObjArr[i-1],  base);
            memset(base, 0, bufferSize);// clean buffer
        }
    }

    return NO_ERR;
}

Pin* new_pin(UINT32 bufSize)
{
    Pin* pObj = NULL;
    if(bufSize == 0) return NULL;
    //DEBUG("[%s] \n", __FUNCTION__);

    pObj = (Pin*)kmalloc(sizeof(Pin), GFP_KERNEL);
    if(pObj == NULL)
    {
        return NULL;
    }
    pObj->pDerivedObj = pObj;
    pObj->allocator = NULL;
    pObj->allocator = CreateAllocator(bufSize);
    if(pObj->allocator == NULL)
    {
        kfree(pObj);
        return NULL;
    }
    pObj->listSize = 1;

    pObj->GetReadSize = GetReadSize;
    pObj->GetWriteSize = GetWriteSize;
    pObj->GetReadPtr = GetReadPtr;
    pObj->SetReadPtr = SetReadPtr;
    pObj->SetNumOfReadPtr         = SetNumOfReadPtr;
    pObj->GetWritePtr = GetWritePtr;
    pObj->SetWritePtr = SetWritePtr;
    pObj->GetBufHeader = GetBufHeader;
    pObj->GetBufAddress           = GetBufAddress;
    pObj->GetListSize = GetListSize;
    pObj->GetBufferSize           = GetBufferSize;
    pObj->SetBufferSize           = SetBufferSize;
    pObj->GetBuffer = GetBuffer;
    pObj->GetPhysicalWriteAddress = GetPhysicalWriteAddress;
    pObj->Delete = delete_pin;

    pObj->MemCopyFromReadPtr = MemCopyFromReadPtr;
    //pObj->MemCopyFromReadPtrNonInverse = MemCopyFromReadPtrNonInVerse;
    pObj->MemCopyToWritePtr  = MemCopyToWritePtr;
    pObj->MemCopyToWritePtrNoInverse = MemCopyToWritePtrNoInverse;
    pObj->AddReadPtr         = AddReadPtr;
    pObj->AddWritePtr        = AddWritePtr;
    pObj->CheckReadableSize  = CheckReadableSize;
    pObj->CheckWritableSize  = CheckWritableSize;

    SetNumOfReadPtr(pObj, 1);

    return pObj;
}

Pin* new_multiPin(UINT32 bufSize, UINT32 listSize)
{
    SINT32 i;
    MultiPin* pMObj = NULL;
    Pin* pObj = NULL;

    Pin **pinArr = (Pin**)kmalloc(sizeof(uintptr_t)*(listSize-1), GFP_KERNEL);
    if(pinArr == NULL)
    {
        return NULL;
    }

    pObj = new_pin(bufSize);
    //DEBUG("[%s] \n", __FUNCTION__);
    if(pObj == NULL)
    {
        kfree(pinArr);
        return NULL;
    }

    pMObj = (MultiPin*)kmalloc(sizeof(MultiPin), GFP_KERNEL);
    if(pMObj == NULL)
    {
        kfree(pinArr);
        pObj->Delete(pObj);
        return NULL;
    }
    pObj->pDerivedObj = pMObj;
    pObj->listSize = listSize;
    pObj->ResetPin = reset_pin_multiPin;
    pObj->Delete = delete_multiPin;

    pMObj->pPinObjArr = pinArr;
    for(i = 0; i < (SINT32)listSize-1; i++)
    {
        pinArr[i] = new_pin(bufSize);
        if(pinArr[i] == NULL)
        {
            kfree(pinArr);
            pObj->Delete(pObj);
            return NULL;
        }
    }

    return pObj;
}

void InitPacketPin(Pin *pPinObj, int isChild)
{
    UINT32 pAddr;
    UINT8 *base;
    UINT32 bufferSize;

    RINGBUFFER_HEADER *vAddr;
    SYS_PROCESS_INIT_RING_HEADER InitHeader;
    Pin *pDataPinObj = ((PacketPin*)pPinObj->pDerivedObj)->pDataPinObj;

    pPinObj->GetBuffer(pPinObj, &base, &bufferSize);
    //INFO("base2 = %x size = %x\n", base, bufferSize);
    pPinObj->SetReadPtr(pPinObj, base,  0);
    pPinObj->SetWritePtr(pPinObj,  base);
    memset(base, 0, bufferSize);// clean buffer

    InitHeader.header.type = AUDIO_SYS_PROCESS_CMD_INIT_RING_HEADER;
    InitHeader.header.size = sizeof(SYS_PROCESS_INIT_RING_HEADER);
    InitHeader.listSize = pDataPinObj->GetListSize(pDataPinObj);

    pDataPinObj->GetBufHeader(pDataPinObj, (void**)&vAddr, &pAddr);
    InitHeader.pRingBufferHeaderList[0] = (unsigned int)pAddr;
    //INFO("[%s] ch0 pAddr:%x\n", __FUNCTION__, pAddr);

    pDataPinObj->GetBuffer(pDataPinObj, &base, &bufferSize);
    pDataPinObj->SetReadPtr(pDataPinObj, base, 0);
    pDataPinObj->SetReadPtr(pDataPinObj, base, 1);
    pDataPinObj->SetReadPtr(pDataPinObj, base, 2);
    pDataPinObj->SetReadPtr(pDataPinObj, base, 3);
    pDataPinObj->SetWritePtr(pDataPinObj,  base);
    memset(base, 0, bufferSize);// clean buffer

    if(InitHeader.listSize > 1) {
        int i;
        MultiPin* pMObj = (MultiPin*)pDataPinObj->pDerivedObj;
        for(i = 1; i < InitHeader.listSize; i++) {
            pMObj->pPinObjArr[i-1]->GetBuffer( pMObj->pPinObjArr[i-1], &base, &bufferSize);
            //INFO("%d  base = %x size = %x\n", i, base, bufferSize);
            pMObj->pPinObjArr[i-1]->SetReadPtr( pMObj->pPinObjArr[i-1], base, 0);
            pMObj->pPinObjArr[i-1]->SetReadPtr( pMObj->pPinObjArr[i-1], base, 1);
            pMObj->pPinObjArr[i-1]->SetReadPtr( pMObj->pPinObjArr[i-1], base, 2);
            pMObj->pPinObjArr[i-1]->SetReadPtr( pMObj->pPinObjArr[i-1], base, 3);
            pMObj->pPinObjArr[i-1]->SetWritePtr( pMObj->pPinObjArr[i-1],  base);
            memset(base, 0, bufferSize);// clean buffer

            pMObj->pPinObjArr[i-1]->GetBufHeader(pMObj->pPinObjArr[i-1], (void**)&vAddr, &pAddr);
            if(isChild==0) InitHeader.pRingBufferHeaderList[i] = (unsigned int)pAddr;
            //INFO("[%s] ch[%d] pAddr:%x,cmdsize:%x\n", __FUNCTION__, i, pAddr,sizeof(SYS_PROCESS_INIT_RING_HEADER));
        }
    }

    if(isChild==0){
        pPinObj->MemCopyToWritePtr(pPinObj, (UINT8*)&InitHeader, sizeof(SYS_PROCESS_INIT_RING_HEADER));
        pPinObj->AddWritePtr(pPinObj, sizeof(SYS_PROCESS_INIT_RING_HEADER));
    }
}

UINT32 Pkt_CheckReadableSize(Pin *pPinObj, UINT32 sizeToRead)
{
    Pin *pDataPinObj = ((PacketPin*)pPinObj->pDerivedObj)->pDataPinObj;

    if(pPinObj->GetReadSize(pPinObj, 0) == 0) {
        /* DEBUG("[UD] Pkt rp %x, wp %x\n", pPinObj->GetReadPtr(pPinObj, 0), pPinObj->GetWritePtr(pPinObj)); */
        return ERR_UNDERFLOW;
    }
    if(pDataPinObj->GetReadSize(pDataPinObj, 0) < sizeToRead) {
        /* DEBUG("[UD] Data rp %x, wp %x\n", pDataPinObj->GetReadPtr(pDataPinObj, 0), pDataPinObj->GetWritePtr(pDataPinObj)); */
        return ERR_UNDERFLOW;
    }
    return NO_ERR;
}

UINT32 Pkt_CheckWritableSize(Pin *pPinObj, UINT32 sizeToWrite)
{
    Pin *pDataPinObj = ((PacketPin*)pPinObj->pDerivedObj)->pDataPinObj;
    if(pPinObj->GetWriteSize(pPinObj) < sizeToWrite) {
        //DEBUG("[OF] Pkt rp %x, wp %x\n", pPinObj->GetReadPtr(pPinObj, 0), pPinObj->GetWritePtr(pPinObj));
        return ERR_OVERFLOW;
    }
    if(pDataPinObj->GetWriteSize(pDataPinObj) < sizeToWrite) {
        //DEBUG("[OF] Data rp %x, wp %x\n", pDataPinObj->GetReadPtr(pDataPinObj, 0), pDataPinObj->GetWritePtr(pDataPinObj));
        return ERR_OVERFLOW;
    }
    return NO_ERR;
}

void delete_packetPin(Pin *pPinObj)
{
    PacketPin* pPktObj = NULL;
    if(pPinObj == NULL) return;

    pPktObj = (PacketPin*)pPinObj->pDerivedObj;
    if(pPktObj) {
        if(pPktObj->pDataPinObj) {
            pPktObj->pDataPinObj->Delete(pPktObj->pDataPinObj);
        }
        pPktObj->pDataPinObj = NULL;
        kfree(pPktObj);
    }

    return delete_pin(pPinObj);
}

Pin* new_packetPin(UINT32 bufSize, UINT32 listSize)
{
    //DEBUG("[%s] \n", __FUNCTION__);
    PacketPin* pPktObj = NULL;
    Pin* pDataPinObj = NULL;
    Pin* pObj = new_pin(bufSize);
    if(pObj == NULL)
        return NULL;

    pPktObj = (PacketPin*)kmalloc(sizeof(PacketPin), GFP_KERNEL);
    if(pPktObj == NULL) {
        pObj->Delete(pObj);
        return NULL;
    }

    if(listSize == 1) {
        pDataPinObj = new_pin(bufSize);
    } else {
        pDataPinObj = new_multiPin(bufSize, listSize);
    }
    if(pDataPinObj == NULL) {
        kfree(pPktObj);
        pObj->Delete(pObj);
        return NULL;
    }
    pObj->pDerivedObj       = pPktObj;
    pObj->listSize          = 1;
    pObj->Delete            = delete_packetPin;
    pObj->CheckReadableSize = Pkt_CheckReadableSize;
    pObj->CheckWritableSize = Pkt_CheckWritableSize;
    pPktObj->pDataPinObj    = pDataPinObj;
    pPktObj->InitPacketPin  = InitPacketPin;
    return pObj;
}

Pin* new_multiPin_DEC_QuickShow(UINT32 bufSize, UINT32 listSize, UINT32 *nonCachedAddr, UINT32 *phyAddr)
{
    SINT32 i;
    MultiPin* pMObj = NULL;
    Pin **pinArr;
    Pin *pObj = new_pin_QuickShow(bufSize, nonCachedAddr, phyAddr);
    if(pObj == NULL)
    {
        return NULL;
    }
    pMObj = (MultiPin*)kmalloc(sizeof(MultiPin), GFP_KERNEL);
    if(pMObj == NULL)
    {
        pObj->Delete(pObj);
        return NULL;
    }
    pObj->pDerivedObj = pMObj;
    pObj->listSize = listSize;
    pObj->Delete = delete_multiPin;

    pinArr = (Pin**)kmalloc(sizeof(UINT32)*(listSize-1), GFP_KERNEL);
    if(pinArr == NULL)
    {
        pObj->Delete(pObj);
        return NULL;
    }
    pMObj->pPinObjArr = pinArr;
    for(i = 0; i < (SINT32)listSize-1; i++)
    {
        pinArr[i] = new_pin_QuickShow(bufSize, nonCachedAddr, phyAddr);
        if(pinArr[i] == NULL)
        {
            pObj->Delete(pObj);
            return NULL;
        }
    }

    return pObj;
}
Pin* new_pin_QuickShow(UINT32 bufSize, UINT32 *nonCachedAddr, UINT32 *phyAddr)
{
    Pin* pObj = NULL;
    if(bufSize == 0) return NULL;

    pObj = (Pin*)kmalloc(sizeof(Pin), GFP_KERNEL);
    if(pObj == NULL)
    {
        return NULL;
    }

    pObj->pDerivedObj = pObj;
    pObj->allocator = NULL;
    pObj->allocator = CreateAllocatorQuickShow(bufSize, nonCachedAddr, phyAddr);
    if(pObj->allocator == NULL)
    {
        kfree(pObj);
        return NULL;
    }
    pObj->listSize = 1;

    pObj->GetReadSize = GetReadSize;
    pObj->GetWriteSize = GetWriteSize;
    pObj->GetReadPtr = GetReadPtr;
    pObj->SetReadPtr = SetReadPtr;
    pObj->SetNumOfReadPtr         = SetNumOfReadPtr;
    pObj->GetWritePtr = GetWritePtr;
    pObj->SetWritePtr = SetWritePtr;
    pObj->GetBufHeader = GetBufHeader;
    pObj->GetListSize = GetListSize;
    pObj->GetBuffer = GetBuffer;
    pObj->GetPhysicalWriteAddress = GetPhysicalWriteAddress;
    pObj->Delete = delete_pin;

    pObj->MemCopyFromReadPtr = MemCopyFromReadPtr;
    //pObj->MemCopyFromReadPtrNonInverse = MemCopyFromReadPtrNonInVerse;
    pObj->MemCopyToWritePtr  = MemCopyToWritePtr;
    pObj->MemCopyToWritePtrNoInverse = MemCopyToWritePtrNoInverse;
    pObj->AddReadPtr         = AddReadPtr;
    pObj->AddWritePtr        = AddWritePtr;
    pObj->CheckReadableSize  = CheckReadableSize;
    pObj->CheckWritableSize  = CheckWritableSize;

    SetNumOfReadPtr(pObj, 1);

    return pObj;
}

