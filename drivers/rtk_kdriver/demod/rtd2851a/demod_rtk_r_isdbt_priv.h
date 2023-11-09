#ifndef  __DEMOD_RTK_R_ISDBT_PRIV_H__
#define  __DEMOD_RTK_R_ISDBT_PRIV_H__

#include "demod_rtk_r_isdbt.h"
#include "demod_rtk_common.h"

#define GET_SIGNAL_STRENGTH_FROM_SNR

//change memory size from 26Mbs(3.3MBs) to 4MBs because 3.3MBs cause memory trash on MacArthur2
#define RTK_R_ISDBTT_DEMUX_DDR_SIZE  (4 * 1024 * 1024)    // 4MBs
#define RTK_R_ISDBTT_ITER_OUTER_DDR_SIZE  (400 * 1024)    // 400KBs

typedef struct {
	ISDBT_DEMOD_MODULE*     pDemod;
	BASE_INTERFACE_MODULE*  pBaseInterface;
	I2C_BRIDGE_MODULE*      pI2CBridge;
	unsigned char           DeviceAddr;
	U32BITS           CrystalFreqHz;
	unsigned char*          pShareMemory;
	unsigned long           ShareMemoryPhysicalAddress;
} RTK_R_ISDBT_DRIVER_DATA;

#define DECODE_LOCK(x)      ((x==YES) ? DTV_SIGNAL_LOCK : DTV_SIGNAL_NOT_LOCK)


extern RTK_R_ISDBT_DRIVER_DATA* AllocRtkrDriver_ISDBT(
	COMM*               pComm,
	unsigned char       Addr,
	U32BITS       CrystalFreq,
	int                 TSMode
);

extern void ReleaseRtkrDriver(RTK_R_ISDBT_DRIVER_DATA *pDriver);



//--------------------------------------------------------------------------
// Optimization Setting for Tuner
//--------------------------------------------------------------------------

static const REG_VALUE_ENTRY TDA18273_OPT_VALUE_ISDBT[] = {
	// RegBitName,				Value
	{ISDBT_LOOPGAIN1,			24	},
};

static const REG_VALUE_ENTRY TDA18274_OPT_VALUE_ISDBT[] = {
	// RegBitName,				Value
	{ISDBT_LOOPGAIN1,			24	},
};

static const REG_VALUE_ENTRY TDA18275_OPT_VALUE_ISDBT[] = {
	// RegBitName,				Value
	{ISDBT_LOOPGAIN1,			24	},
};

static const REG_VALUE_ENTRY TDA18275A_OPT_VALUE_ISDBT[] = {
	// RegBitName,				Value
	{ISDBT_LOOPGAIN1,			24	},
};

static const REG_VALUE_ENTRY SI2158_OPT_VALUE_ISDBT[] = {
	// RegBitName,				Value
	{ISDBT_LOOPGAIN1,			24	},
	{ISDBT_AAGC_TARGET_VAL,     0x26},
};

static const REG_VALUE_ENTRY MXL601_OPT_VALUE_ISDBT[] = {
	// RegBitName,				Value
	{ISDBT_LOOPGAIN1,			24	},
};

static const REG_VALUE_ENTRY R840_OPT_VALUE_ISDBT[] = {
	// RegBitName,				Value
	{ISDBT_LOOPGAIN1,			24	},
};
//Add SONY Tuner SUTRX243 not fine tune !!
static const REG_VALUE_ENTRY SUTRX243_OPT_VALUE_ISDBT[] = {
	// RegBitName,				Value
	{ISDBT_LOOPGAIN1,			24	},
    {ISDBT_AAGC_TARGET_VAL,     0x25},
};

static const REG_VALUE_ENTRY ATBM253_OPT_VALUE_ISDBT[] = {
	// RegBitName,				Value
	{ISDBT_LOOPGAIN1,			24	},
};
#endif
