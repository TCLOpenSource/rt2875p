#ifndef  __DEMOD_REALTEK_R_ATSC3_PRIV_H__
#define  __DEMOD_REALTEK_R_ATSC3_PRIV_H__

#include "atsc3_demod_rtk_r.h"
#include "demod_rtk_common.h"

#define GET_SIGNAL_STRENGTH_FROM_SNR
#define ATSC3_AAGC_TARGET_reg   0xB8164010   

typedef struct {
	ATSC3_DEMOD_MODULE*     pDemod;
	BASE_INTERFACE_MODULE*  pBaseInterface;
	I2C_BRIDGE_MODULE*      pI2CBridge;
	unsigned char           DeviceAddr;
	U32BITS           CrystalFreqHz;
} REALTEK_R_ATSC3_DRIVER_DATA;


#define DECODE_LOCK(x)      ((x==YES) ? DTV_SIGNAL_LOCK : DTV_SIGNAL_NOT_LOCK)


extern REALTEK_R_ATSC3_DRIVER_DATA* AllocRealtekRAtsc3Driver(
	COMM*               pComm,
	unsigned char       Addr,
	U32BITS       CrystalFreq
);

extern void ReleaseRealtekRAtsc3Driver(REALTEK_R_ATSC3_DRIVER_DATA *pDriver);

//--------------------------------------------------------------------------
// Optimization Setting for Tuner
//--------------------------------------------------------------------------
static const REG_VALUE_ENTRY_RBUS TDA18275A_OPT_VALUE_ATSC3[] = {
    // RegBitName,              Value
    {ATSC3_AAGC_TARGET_reg,             0x6B    },
};
static const REG_VALUE_ENTRY_RBUS MXL661_OPT_VALUE_ATSC3[] = {
    // RegBitName,              Value
    {ATSC3_AAGC_TARGET_reg,             0x6B    },
};
static const REG_VALUE_ENTRY_RBUS R840_OPT_VALUE_ATSC3[] = {
    // RegBitName,              Value
    {ATSC3_AAGC_TARGET_reg,             0x6B    },
};
static const REG_VALUE_ENTRY_RBUS SI2151_OPT_VALUE_ATSC3[] = {
    // RegBitName,              Value
    {ATSC3_AAGC_TARGET_reg,             0x6B    },
};
static const REG_VALUE_ENTRY_RBUS SUTRX243_OPT_VALUE_ATSC3[] = {
    // RegBitName,              Value
    {ATSC3_AAGC_TARGET_reg,             0x65    },      //change to 0x65 for Sony243 tuner
};
#endif // __DEMOD_REALTEK_R_ATSC3_PRIV_H__
