#ifndef  __DEMOD_REALTEK_H_DTMB_PRIV_H__
#define  __DEMOD_REALTEK_H_DTMB_PRIV_H__

#include "dtmb_demod_base.h"
#include "dtmb_demod_rtk.h"
#include "dtmb_demod_rtk_user.h"
#include "dtmb_demod_rtk_type.h"

#define DTMB_H_AAGC_TARGET_reg  0xB8170090

typedef struct {
	DTMB_DEMOD_MODULE*      pDemod;
	BASE_INTERFACE_MODULE*  pBaseInterface;
	I2C_BRIDGE_MODULE*      pI2CBridge;
	unsigned char           DeviceAddr;
	U32BITS           CrystalFreqHz;
} REALTEK_H_DTMB_DRIVER_DATA;


#define DECODE_LOCK(x)      ((x==YES) ? DTV_SIGNAL_LOCK : DTV_SIGNAL_NOT_LOCK)


extern REALTEK_H_DTMB_DRIVER_DATA* AllocRealtekHDtmbDriver(
	COMM*               pComm,
	unsigned char       Addr,
	U32BITS       CrystalFreq
);

extern void ReleaseRealtekHDtmbDriver(REALTEK_H_DTMB_DRIVER_DATA *pDriver);
//--------------------------------------------------------------------------
// Optimization Setting for Tuner
//--------------------------------------------------------------------------
static const REG_VALUE_ENTRY_RBUS MXL661_OPT_VALUE_DTMB[] = {
    // RegBitName,              Value
    {DTMB_H_AAGC_TARGET_reg,             0xB4    },     // Default 0xA4, change to 0xB4
};
static const REG_VALUE_ENTRY_RBUS R840_OPT_VALUE_DTMB[] = {
    // RegBitName,              Value
    {DTMB_H_AAGC_TARGET_reg,             0xB4    },     // Default 0xA4, change to 0xB4
};
static const REG_VALUE_ENTRY_RBUS TDA18275A_OPT_VALUE_DTMB[] = {
    // RegBitName,              Value
    {DTMB_H_AAGC_TARGET_reg,             0xB4    },     // Default 0xA4, change to 0xB4
};
static const REG_VALUE_ENTRY_RBUS SI2151_OPT_VALUE_DTMB[] = {
    // RegBitName,              Value
    {DTMB_H_AAGC_TARGET_reg,             0xB4    },     // Default 0xA4, change to 0xB4
};
static const REG_VALUE_ENTRY_RBUS SUTRX243_OPT_VALUE_DTMB[] = {
    // RegBitName,              Value
    {DTMB_H_AAGC_TARGET_reg,             0xA8    },     // Default 0xA4, change to 0xA8
};
static const REG_VALUE_ENTRY_RBUS ATBM253_OPT_VALUE_DTMB[] = {
    // RegBitName,              Value
    {DTMB_H_AAGC_TARGET_reg,             0xB4    },     // Default 0xA4, change to 0xB4
};

#endif

