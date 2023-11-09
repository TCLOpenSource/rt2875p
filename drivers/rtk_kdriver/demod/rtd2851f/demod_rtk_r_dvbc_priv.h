#ifndef  __DEMOD_REALTEK_I_DVBC_PRIV_H__
#define  __DEMOD_REALTEK_I_DVBC_PRIV_H__

#include "qam_demod_rtk_r.h"
#include "demod_rtk_common.h"
#include "dvbc_reg.h"

#define GET_SIGNAL_STRENGTH_FROM_SNR

typedef struct {
	QAM_DEMOD_MODULE*      pDemod;
	BASE_INTERFACE_MODULE*  pBaseInterface;
	I2C_BRIDGE_MODULE*      pI2CBridge;
	unsigned char           DeviceAddr;
	U32BITS           CrystalFreqHz;
} REALTEK_R_DVBC_DRIVER_DATA;


#define DECODE_LOCK(x)      ((x==YES) ? DTV_SIGNAL_LOCK : DTV_SIGNAL_NOT_LOCK)


extern REALTEK_R_DVBC_DRIVER_DATA* AllocRealtekRDvbcDriver(
	COMM*               pComm,
	unsigned char       Addr,
	U32BITS       CrystalFreq
);

extern void ReleaseRealtekRDvbcDriver(REALTEK_R_DVBC_DRIVER_DATA *pDriver);

//--------------------------------------------------------------------------
// Optimization Setting for Tuner
// Set AAGC_TARGET_AMP
//--------------------------------------------------------------------------
static const REG_VALUE_ENTRY_RBUS SI2151_OPT_VALUE_DVBC[] = {
	// RegBitName,				Value
	{DVBC_OPT_AAGC_TARGET_AMP_reg,			0x64	},// default 0x64
};
static const REG_VALUE_ENTRY_RBUS MXL661_OPT_VALUE_DVBC[] = {
	// RegBitName,				Value
	{DVBC_OPT_AAGC_TARGET_AMP_reg,			0x64	},// default 0x64
};
static const REG_VALUE_ENTRY_RBUS TDA18273_OPT_VALUE_DVBC[] = {
	// RegBitName,				Value
	{DVBC_OPT_AAGC_TARGET_AMP_reg,			0x64	},// default 0x64
};
static const REG_VALUE_ENTRY_RBUS R840_OPT_VALUE_DVBC[] = {
	// RegBitName,				Value
	{DVBC_OPT_AAGC_TARGET_AMP_reg,			0x64	},// default 0x64
};
static const REG_VALUE_ENTRY_RBUS SUTRX243_OPT_VALUE_DVBC[] = {
	// RegBitName,				Value
	{DVBC_OPT_AAGC_TARGET_AMP_reg,			0x52	},// default 0x64 change to 0x52 for Sony243 tuner
};

#endif // __DEMOD_REALTEK_R_DVBC_PRIV_H__
