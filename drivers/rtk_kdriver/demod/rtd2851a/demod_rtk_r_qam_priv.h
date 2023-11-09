#ifndef  __DEMOD_REALTEK_R_QAM_PRIV_H__
#define  __DEMOD_REALTEK_R_QAM_PRIV_H__

#include "qam_demod_rtk_r.h"
#include "demod_rtk_common.h"
#include "dvbc_reg.h"

//#define GET_SIGNAL_STRENGTH_FROM_SNR

typedef struct {
	QAM_DEMOD_MODULE*       pDemod;
	BASE_INTERFACE_MODULE*  pBaseInterface;
	I2C_BRIDGE_MODULE*      pI2CBridge;
	unsigned char           DeviceAddr;
	U32BITS          CrystalFreqHz;
} REALTEK_R_QAM_DRIVER_DATA;


#define DECODE_LOCK(x)      ((x==YES) ? DTV_SIGNAL_LOCK : DTV_SIGNAL_NOT_LOCK)


extern REALTEK_R_QAM_DRIVER_DATA* AllocRealtekRQamDriver(
	COMM*               pComm,
	unsigned char       Addr,
	U32BITS       CrystalFreq
);

extern void ReleaseRealtekRQamDriver(REALTEK_R_QAM_DRIVER_DATA *pDriver);

//--------------------------------------------------------------------------
// Optimization Setting for Tuner (OpenCable AAGC Target)
//--------------------------------------------------------------------------
static const REG_VALUE_ENTRY_RBUS TDA18273_OPT_VALUE_QAM[] = {
	// RegBitName,								Value
	{DVBC_OPT_AAGC_TARGET_AMP_reg,				0x38		},
};

//--------------------------------------------------------------------------
// Optimization Setting for Tuner IFAGC (Max/Min)
//--------------------------------------------------------------------------
static const REG_VALUE_ENTRY_RBUS SI2151_OPT_VALUE_QAM[] = {
	// RegBitName,				Value
	{DVBC_OPT_IF_AAGC_MAX_reg,				0xfe	},
	{DVBC_OPT_IF_AAGC_MIN_reg,				0x0		},
	{DVBC_OPT_AAGC_TARGET_AMP_reg,			0x5a	},// default 0x64
};

static const REG_VALUE_ENTRY_RBUS SI2178B_OPT_VALUE_QAM[] = {
	// RegBitName,				Value
	{DVBC_OPT_IF_AAGC_MAX_reg,				0xfe	},
	{DVBC_OPT_IF_AAGC_MIN_reg,				0x0		},
};

//IFAGC Controlled by Tuner
static const REG_VALUE_ENTRY_RBUS MXL661_OPT_VALUE_QAM[] = {
	// RegBitName,				Value
	{DVBC_OPT_IF_AAGC_MAX_reg,				0x0 	},
	{DVBC_OPT_IF_AAGC_MIN_reg,				0x0		},
};

//IFAGC Controlled by Demod
static const REG_VALUE_ENTRY_RBUS SUTRX243_OPT_VALUE_QAM[] = {
	// RegBitName,				Value
	{DVBC_OPT_IF_AAGC_MAX_reg,				0xc0	},// IFAGC Max=1.5V
	{DVBC_OPT_IF_AAGC_MIN_reg,				0x0		},// IFAGC Min=0V
	{DVBC_OPT_AAGC_TARGET_AMP_reg,			0x52	},// default 0x64
};

#endif // __DEMOD_REALTEK_R_QAM_PRIV_H__
