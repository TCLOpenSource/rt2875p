#ifndef  __DEMOD_REALTEK_A_DVBSX_PRIV_H__
#define  __DEMOD_REALTEK_A_DVBSX_PRIV_H__

#include "dvbsx_demod_rtk_a.h"
#include "demod_rtk_common.h"

#define GET_SIGNAL_STRENGTH_FROM_SNR

typedef struct {
    DVBSX_DEMOD_MODULE*     pDemod;
    BASE_INTERFACE_MODULE*  pBaseInterface;
    I2C_BRIDGE_MODULE*      pI2CBridge;
    unsigned char           DeviceAddr;
    U32BITS           CrystalFreqHz;
} REALTEK_A_DVBSX_DRIVER_DATA;


#define DECODE_LOCK(x)      ((x==YES) ? DTV_SIGNAL_LOCK : DTV_SIGNAL_NOT_LOCK)


extern REALTEK_A_DVBSX_DRIVER_DATA* AllocRealtekIDvbSxDriver(
    COMM*               pComm,
    unsigned char       Addr,
    U32BITS       CrystalFreq
);

extern void ReleaseRealtekIDvbs2Driver(REALTEK_A_DVBSX_DRIVER_DATA *pDriver);

#endif // __DEMOD_REALTEK_A_DVBSX_PRIV_H__
