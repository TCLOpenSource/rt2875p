#ifndef  __DEMOD_REALTEK_A_DVBTX_PRIV_H__
#define  __DEMOD_REALTEK_A_DVBTX_PRIV_H__

#include "dvbtx_demod_rtk_a.h"
#include "demod_rtk_common.h"

#define GET_SIGNAL_STRENGTH_FROM_SNR
#define DVBTX_AAGC_TARGET_reg   0xB8164010   

typedef struct {
	DVBTX_DEMOD_MODULE*     pDemod;
	BASE_INTERFACE_MODULE*  pBaseInterface;
	I2C_BRIDGE_MODULE*      pI2CBridge;
	unsigned char           DeviceAddr;
	U32BITS           CrystalFreqHz;
} REALTEK_A_DVBTX_DRIVER_DATA;


#define DECODE_LOCK(x)      ((x==YES) ? DTV_SIGNAL_LOCK : DTV_SIGNAL_NOT_LOCK)


extern REALTEK_A_DVBTX_DRIVER_DATA* AllocRealtekADvbtxDriver(
	COMM*               pComm,
	unsigned char       Addr,
	U32BITS       CrystalFreq
);

extern void ReleaseRealtekADvbtxDriver(REALTEK_A_DVBTX_DRIVER_DATA *pDriver);

//--------------------------------------------------------------------------
// Optimization Setting for Tuner
//--------------------------------------------------------------------------
static const REG_VALUE_ENTRY_RBUS TDA18275A_OPT_VALUE_DVBT2[] = {
    // RegBitName,              Value
    {DVBTX_AAGC_TARGET_reg,             0x6B    },
};
static const REG_VALUE_ENTRY_RBUS MXL661_OPT_VALUE_DVBT2[] = {
    // RegBitName,              Value
    {DVBTX_AAGC_TARGET_reg,             0x6B    },
};
static const REG_VALUE_ENTRY_RBUS R840_OPT_VALUE_DVBT2[] = {
    // RegBitName,              Value
    {DVBTX_AAGC_TARGET_reg,             0x6B    },
};
static const REG_VALUE_ENTRY_RBUS SI2151_OPT_VALUE_DVBT2[] = {
    // RegBitName,              Value
    {DVBTX_AAGC_TARGET_reg,             0x6B    },
};
static const REG_VALUE_ENTRY_RBUS SUTRX243_OPT_VALUE_DVBT2[] = {
    // RegBitName,              Value
    {DVBTX_AAGC_TARGET_reg,             0x65    },      //change to 0x65 for Sony243 tuner
};
static const REG_VALUE_ENTRY_RBUS ATBM253_OPT_VALUE_DVBT2[] = {
    // RegBitName,              Value
    {DVBTX_AAGC_TARGET_reg,             0x6B    },
};
#endif // __DEMOD_REALTEK_A_DVBTX_PRIV_H__
