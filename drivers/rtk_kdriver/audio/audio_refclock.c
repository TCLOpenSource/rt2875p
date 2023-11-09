/******************************************************************************
 *
 *   Copyright(c) 2014 Realtek Semiconductor Corp. All rights reserved.
 *
 *   @author yungjui.lee@realtek.com
 *
 *****************************************************************************/
#include "audio_inc.h"
#include <linux/slab.h>
#include <linux/pageremap.h>


#define CLOCK_MODE_MISC 0
#ifdef ENABLE_HW_PCR_TRACKING
#define CLOCK_MODE_TP 1
#endif

extern uint32_t rtkaudio_alloc_uncached(int size, unsigned char** uncached_addr_info);
extern int rtkaudio_free(unsigned int phy_addr);

void IPCWrite_U32(uint32_t *src, uint32_t x)
{
  return IPC_WriteU32((BYTE*)src, x);
}
void IPCWrite_S32(int32_t *src, int32_t x)
{
  return IPC_WriteU32((BYTE*)src, x);
}

void IPCWrite_U64(unsigned long long *src, unsigned long long x)
{
  return IPC_WriteU64((BYTE*)src, x);
}
void IPCWrite_S64(long long *src, long long x)
{
  return IPC_WriteU64((BYTE*)src, x);
}

/** ReferenceClock **/

/* get the core structure that shared between CPUs */
UINT32 GetCoreAddress(ReferenceClock *pRefObj, UINT32 *phyAddr, UINT8 **virtualAddr, UINT8 **nonCachedAddr)
{
  if (phyAddr!= 0)
    *phyAddr = pRefObj->m_phyAddr;
  if (virtualAddr != 0)
    *virtualAddr = pRefObj->m_virtualAddr;
  if (nonCachedAddr != 0)
    *nonCachedAddr = (UINT8*)pRefObj->m_core;
  return S_OK;
}
void delete_ReferenceClock(ReferenceClock* pObj)
{
    if(pObj == NULL) return;
    if(pObj->m_virtualAddr == NULL) return;

    rtkaudio_free(pObj->m_phyAddr);
    kfree(pObj);
    return;
}
ReferenceClock* new_ReferenceClock(void)
{
    ReferenceClock* pObj = NULL;
    BYTE *nonCachedAddr;

    pObj = (ReferenceClock*)kmalloc(sizeof(ReferenceClock), GFP_KERNEL);
    if(pObj == NULL)
    {
        return NULL;
    }
    memset(pObj, 0, sizeof(ReferenceClock));

    DEBUG("[%s] reference clock\n", __FUNCTION__);
    pObj->m_phyAddr = rtkaudio_alloc_uncached(sizeof(REFCLOCK), &pObj->m_virtualAddr);
    nonCachedAddr = pObj->m_virtualAddr;
    if(pObj->m_virtualAddr == NULL)
    {
        kfree(pObj);
        return NULL;
    }
    pObj->m_core = (REFCLOCK*)nonCachedAddr;
    pObj->Delete = delete_ReferenceClock;
    pObj->GetCoreAddress = GetCoreAddress;

    IPCWrite_S64(&pObj->m_core->RCD, -1);
    IPCWrite_S64(&pObj->m_core->masterGPTS, -1);

    IPCWrite_S64(&pObj->m_core->GPTSTimeout,     0);
    IPCWrite_S64(&pObj->m_core->videoSystemPTS, -1);
    IPCWrite_S64(&pObj->m_core->audioSystemPTS, -1);
    IPCWrite_S64(&pObj->m_core->videoRPTS,      -1);
    IPCWrite_S64(&pObj->m_core->audioRPTS,      -1);
    IPCWrite_U32(&pObj->m_core->videoContext,   (uint32_t)-1);
    IPCWrite_U32(&pObj->m_core->audioContext,   (uint32_t)-1);
    IPCWrite_S32(&pObj->m_core->videoEndOfSegment, (int32_t)-1);

    IPCWrite_U32(&pObj->m_core->videoFreeRunThreshold, 0x7FFFFFFF);
    IPCWrite_U32(&pObj->m_core->audioFreeRunThreshold, 0x7FFFFFFF);
    IPCWrite_U32(&pObj->m_core->audioFreeRunThresholdToWait, 0x7FFFFFFF);

    IPCWrite_S32(&pObj->m_core->VO_Underflow, 0);
    IPCWrite_S32(&pObj->m_core->AO_Underflow, 0);

    IPCWrite_U32(&pObj->m_core->clockMode, CLOCK_MODE_MISC);

    pObj->m_core->mastership.systemMode  = (unsigned char)AVSYNC_FORCED_SLAVE;
    pObj->m_core->mastership.videoMode   = (unsigned char)AVSYNC_FORCED_MASTER;
    pObj->m_core->mastership.audioMode   = (unsigned char)AVSYNC_FORCED_MASTER;
    pObj->m_core->mastership.masterState = (unsigned char)AUTOMASTER_NOT_MASTER;

    return pObj;
}
/** End of Referenceclock **/
