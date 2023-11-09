/******************************************************************************
 *
 *   Copyright(c) 2014 Realtek Semiconductor Corp. All rights reserved.
 *
 *   @author yungjui.lee@realtek.com
 *
 *****************************************************************************/

#include "audio_base.h"
#include <linux/slab.h>

void SetSDECInfo(Base* pBaseObj, UINT32 sdec_channel, UINT32 sdec_dest)
{
    DtvCom* pDtvComObj = (DtvCom*)pBaseObj->pDerivedObj;

    pDtvComObj->m_sdecChannel = sdec_channel;
    pDtvComObj->m_sdecDest    = sdec_dest;
    return;
}
void SetSDECTracking(Base* pBaseObj, bool isEnable)
{
    DtvCom* pDtvComObj = (DtvCom*)pBaseObj->pDerivedObj;

    pDtvComObj->m_isEnableTracking = isEnable;
    return;
}
UINT32 GetSDECChannel(Base* pBaseObj)
{
    DtvCom* pDtvComObj = (DtvCom*)pBaseObj->pDerivedObj;
    return pDtvComObj->m_sdecChannel;
}
UINT32 GetSDECDest(Base* pBaseObj)
{
    DtvCom* pDtvComObj = (DtvCom*)pBaseObj->pDerivedObj;
    return pDtvComObj->m_sdecDest;
}
bool GetSDECTracking(Base* pBaseObj)
{
    DtvCom* pDtvComObj = (DtvCom*)pBaseObj->pDerivedObj;
    return pDtvComObj->m_isEnableTracking;
}
bool SetAudioSyncMode(Base* pBaseObj, AVSYNC_MODE mode)
{
    DtvCom* pDtvComObj = (DtvCom*)pBaseObj->pDerivedObj;
    REFCLOCK* pRefClk = (REFCLOCK*)(pDtvComObj->RefClock_vir);

    if(pRefClk == NULL)
        return FALSE;

    pRefClk->mastership.audioMode = mode;
    return TRUE;
}
UINT32 SetBSRingBufPhyAddress(Base* pBaseObj, UINT32 phyAddr, void *virAddr, UINT32 mapAddr, UINT32 mapSize)
{
    DtvCom* pDtvComObj = (DtvCom*)pBaseObj->pDerivedObj;
    pDtvComObj->BSRing_phy = phyAddr;
    pDtvComObj->BSRing_vir = virAddr;
    pDtvComObj->BSRing_mapAddr = mapAddr;// for unmap used
    pDtvComObj->BSRing_mapSz = mapSize;

    return S_OK;
}
UINT32 SetICQRingBufPhyAddress(Base* pBaseObj, UINT32 phyAddr, void *virAddr, UINT32 mapAddr, UINT32 mapSize)
{
    DtvCom* pDtvComObj = (DtvCom*)pBaseObj->pDerivedObj;
    pDtvComObj->ICQRing_phy = phyAddr;
    pDtvComObj->ICQRing_vir = virAddr;
    pDtvComObj->ICQRing_mapAddr = mapAddr;// for unmap used
    pDtvComObj->ICQRing_mapSz = mapSize;
    return S_OK;
}
UINT32 SetRefClockPhyAddress(Base* pBaseObj, UINT32 phyAddr, void *virAddr, UINT32 mapAddr, UINT32 mapSize)
{
    DtvCom* pDtvComObj = (DtvCom*)pBaseObj->pDerivedObj;
    pDtvComObj->RefClock_phy = phyAddr;
    pDtvComObj->RefClock_vir = virAddr;
    pDtvComObj->RefClock_mapAddr = mapAddr;// for unmap used
    pDtvComObj->RefClock_mapSz = mapSize;
    return S_OK;
}
UINT32 GetBSRingBufPhyAddress(Base* pBaseObj)
{
    DtvCom* pDtvComObj = (DtvCom*)pBaseObj->pDerivedObj;
    return pDtvComObj->BSRing_phy;
}
UINT32 GetICQRingBufPhyAddress(Base* pBaseObj)
{
    DtvCom* pDtvComObj = (DtvCom*)pBaseObj->pDerivedObj;
    return pDtvComObj->ICQRing_phy;
}
UINT32 GetRefClockPhyAddress(Base* pBaseObj)
{
    DtvCom* pDtvComObj = (DtvCom*)pBaseObj->pDerivedObj;
    return pDtvComObj->RefClock_phy;
}
void * GetBSRingBufVirAddress(Base* pBaseObj)
{
    DtvCom* pDtvComObj = (DtvCom*)pBaseObj->pDerivedObj;
    return pDtvComObj->BSRing_vir;
}
void * GetICQRingBufVirAddress(Base* pBaseObj)
{
    DtvCom* pDtvComObj = (DtvCom*)pBaseObj->pDerivedObj;
    return pDtvComObj->ICQRing_vir;
}
void * GetRefClockVirAddress(Base* pBaseObj)
{
    DtvCom* pDtvComObj = (DtvCom*)pBaseObj->pDerivedObj;
    return pDtvComObj->RefClock_vir;
}
void GetBSRingBufMapInfo(Base* pBaseObj, UINT32* mapAddr, UINT32* mapSize)
{
    DtvCom* pDtvComObj = (DtvCom*)pBaseObj->pDerivedObj;

    if(mapAddr)
        *mapAddr = pDtvComObj->BSRing_mapAddr;

    if(mapSize)
        *mapSize = pDtvComObj->BSRing_mapSz;

}
void GetICQRingBufMapInfo(Base* pBaseObj, UINT32* mapAddr, UINT32* mapSize)
{
    DtvCom* pDtvComObj = (DtvCom*)pBaseObj->pDerivedObj;

     if(mapAddr)
        *mapAddr = pDtvComObj->ICQRing_mapAddr;

    if(mapSize)
        *mapSize = pDtvComObj->ICQRing_mapSz;

}
void GetRefClockMapInfo(Base* pBaseObj, UINT32* mapAddr, UINT32* mapSize)
{
    DtvCom* pDtvComObj = (DtvCom*)pBaseObj->pDerivedObj;

    if(mapAddr)
       *mapAddr = pDtvComObj->RefClock_mapAddr;

   if(mapSize)
       *mapSize = pDtvComObj->RefClock_mapSz;
}
void delete_DtvCom(Base* pBaseObj)
{
    DtvCom* pDtvComObj = NULL;
    if(pBaseObj == NULL) return;
    pDtvComObj = (DtvCom*)pBaseObj->pDerivedObj;

    kfree(pDtvComObj);
    return delete_base(pBaseObj);
}
Base* new_DtvCom(void)
{
    /* object init */
    char name[] = "DTVCOM";
    DtvCom* pDtvComObj = NULL;
    Base* pObj = new_base();
    if(pObj == NULL)
    {
        return NULL;
    }
    pDtvComObj = (DtvCom*)kmalloc(sizeof(DtvCom), GFP_KERNEL);
    if(pDtvComObj == NULL)
    {
        pObj->Delete(pObj);
        return NULL;
    }
    memset(pDtvComObj, 0, sizeof(DtvCom));
    memcpy(pObj->name, name, sizeof(name));
    pObj->pDerivedObj = pDtvComObj;

    /* setup member functions */
    pObj->Delete                        = delete_DtvCom;
    pDtvComObj->SetSDECInfo             = SetSDECInfo;
    pDtvComObj->SetAudioSyncMode        = SetAudioSyncMode;
    pDtvComObj->GetSDECChannel          = GetSDECChannel;
    pDtvComObj->GetSDECDest             = GetSDECDest;
    pDtvComObj->SetBSRingBufPhyAddress  = SetBSRingBufPhyAddress;
    pDtvComObj->SetICQRingBufPhyAddress = SetICQRingBufPhyAddress;
    pDtvComObj->SetRefClockPhyAddress   = SetRefClockPhyAddress;
    pDtvComObj->GetBSRingBufPhyAddress  = GetBSRingBufPhyAddress;
    pDtvComObj->GetICQRingBufPhyAddress = GetICQRingBufPhyAddress;
    pDtvComObj->GetRefClockPhyAddress   = GetRefClockPhyAddress;
    pDtvComObj->GetBSRingBufVirAddress  = GetBSRingBufVirAddress;
    pDtvComObj->GetICQRingBufVirAddress = GetICQRingBufVirAddress;
    pDtvComObj->GetRefClockVirAddress   = GetRefClockVirAddress;
    pDtvComObj->GetSDECTracking         = GetSDECTracking;
    pDtvComObj->SetSDECTracking         = SetSDECTracking;

    pDtvComObj->GetBSRingBufMapInfo      = GetBSRingBufMapInfo;
    pDtvComObj->GetICQRingBufMapInfo     = GetICQRingBufMapInfo;
    pDtvComObj->GetRefClockMapInfo       = GetRefClockMapInfo;


    /* agent init */
    pObj->instanceID = UNDEFINED_AGENT_ID;  // no agent
    pObj->inPinID = UNINIT_PINID;           // no in pin

    pObj->outPin = NULL;
    pObj->outPinID = BASE_BS_OUT;

    return pObj;
}
