#ifndef  __DEMOD_REALTEK_R_DTMB_PRIV_H__
#define  __DEMOD_REALTEK_R_DTMB_PRIV_H__

#include "dtmb_demod_rtk_r_base.h"
#include "dtmb_demod_rtk_r.h"


#define RTK_R_DTMB_DDR_SIZE  (4 * 1024 * 1024)    // 4MBs

typedef struct {
	DTMB_DEMOD_MODULE*      pDemod;
	BASE_INTERFACE_MODULE*  pBaseInterface;
	I2C_BRIDGE_MODULE*      pI2CBridge;
	unsigned char           DeviceAddr;
	U32BITS           CrystalFreqHz;
	unsigned long*          pShareMemory;
	unsigned long           ShareMemoryPhysicalAddress;

} REALTEK_R_DTMB_DRIVER_DATA;


#define DECODE_RET(x)       ((x==FUNCTION_SUCCESS) ? TUNER_CTRL_OK : TUNER_CTRL_FAIL)
#define ENCODE_RET(x)       ((x==TUNER_CTRL_OK) ?  FUNCTION_SUCCESS : FUNCTION_ERROR)
#define DECODE_LOCK(x)      ((x==YES) ? DTV_SIGNAL_LOCK : DTV_SIGNAL_NOT_LOCK)


extern REALTEK_R_DTMB_DRIVER_DATA* AllocRealtekRDtmbDriver(
	COMM*               pComm,
	unsigned char       Addr,
	U32BITS       CrystalFreq
);

extern void ReleaseRealtekRDtmbDriver(REALTEK_R_DTMB_DRIVER_DATA *pDriver);


//--------------------------------------------------------------------------
// Optimization Setting for Tuner
//--------------------------------------------------------------------------

static const REG_VALUE_ENTRY TDA18273_OPT_VALUE_DTMB[] = {
	// RegBitName,				Value
	{DTMB_Z_AGC,					0x2		},
	{DTMB_EN_PGA_MODE,				0x0		},
	{DTMB_TARGET_VAL,				0x28	},
	{DTMB_IFAGC_MAX,				0xc7       },
	{DTMB_IFAGC_MIN,				0x0       },
	{DTMB_AAGC_LOOPGAIN1,			0x15	},
	{DTMB_INTEGRAL_CNT_LEN,			0xc		},
	{DTMB_AAGC_LOCK_PGA_HIT_LEN,	0x1		},
	{DTMB_THD_LOCK_UP,				0x205	},
	{DTMB_THD_LOCK_DW,				0x1fb	},
	{DTMB_THD_UP1,					0x205	},
	{DTMB_THD_DW1,					0x1fb	},
	{DTMB_THD_UP2,					0x86	},
	{DTMB_THD_DW2,					0x7a	},
	{DTMB_GAIN_PULSE_SPACE_LEN,		0x1		},
	{DTMB_GAIN_PULSE_HOLD_LEN,		0x1		},
	{DTMB_GAIN_STEP_SUM_UP_THD,		0xb		},
	{DTMB_GAIN_STEP_SUM_DW_THD,		0x5		},
};

static const REG_VALUE_ENTRY TDA18273_SIRIUS_OPT_VALUE_DTMB[] = {
	// RegBitName,				Value
	{DTMB_Z_AGC,					0x2		},
	{DTMB_EN_PGA_MODE,				0x0		},
	{DTMB_TARGET_VAL,				0x50	},
	{DTMB_IFAGC_MAX,				0xc7       },
	{DTMB_IFAGC_MIN,				0x0       },
	{DTMB_AAGC_LOOPGAIN1,			0x15	},
	{DTMB_INTEGRAL_CNT_LEN,			0xc		},
	{DTMB_AAGC_LOCK_PGA_HIT_LEN,	0x1		},
	{DTMB_THD_LOCK_UP,				0x205	},
	{DTMB_THD_LOCK_DW,				0x1fb	},
	{DTMB_THD_UP1,					0x205	},
	{DTMB_THD_DW1,					0x1fb	},
	{DTMB_THD_UP2,					0x86	},
	{DTMB_THD_DW2,					0x7a	},
	{DTMB_GAIN_PULSE_SPACE_LEN,		0x1		},
	{DTMB_GAIN_PULSE_HOLD_LEN,		0x1		},
	{DTMB_GAIN_STEP_SUM_UP_THD,		0xb		},
	{DTMB_GAIN_STEP_SUM_DW_THD,		0x5		},
};


static const REG_VALUE_ENTRY SI2158_OPT_VALUE_DTMB[] = {
	// RegBitName,				Value
	{DTMB_Z_AGC,					0x2		},
	{DTMB_EN_PGA_MODE,				0x0		},
	{DTMB_TARGET_VAL,				0x2a	},
//        {DTMB_IFAGC_MAX,				0x0},
//        {DTMB_IFAGC_MIN,				0x0},
	{DTMB_AAGC_LOOPGAIN1,			0x15	},
	{DTMB_INTEGRAL_CNT_LEN,			0xc		},
	{DTMB_AAGC_LOCK_PGA_HIT_LEN,	0x1		},
	{DTMB_THD_LOCK_UP,				0x205	},
	{DTMB_THD_LOCK_DW,				0x1fb	},
	{DTMB_THD_UP1,					0x205	},
	{DTMB_THD_DW1,					0x1fb	},
	{DTMB_THD_UP2,					0x86	},
	{DTMB_THD_DW2,					0x7a	},
	{DTMB_GAIN_PULSE_SPACE_LEN,		0x1		},
	{DTMB_GAIN_PULSE_HOLD_LEN,		0x1		},
	{DTMB_GAIN_STEP_SUM_UP_THD,		0xb		},
	{DTMB_GAIN_STEP_SUM_DW_THD,		0x5		},
};

static const REG_VALUE_ENTRY MXL601_OPT_VALUE_DTMB[] = {
	// RegBitName,				Value
	{DTMB_TARGET_VAL,				0x38	},
	{DTMB_CCI_THD, 0x18},
	{DTMB_Z_AGC,					0x2		},
	{DTMB_EN_PGA_MODE,				0x0		},
	{DTMB_AAGC_LOOPGAIN1,			0x15	},
	{DTMB_INTEGRAL_CNT_LEN,			0xc		},
	{DTMB_AAGC_LOCK_PGA_HIT_LEN,	0x1		},
	{DTMB_THD_LOCK_UP,				0x205	},
	{DTMB_THD_LOCK_DW,				0x1fb	},
	{DTMB_THD_UP1,					0x205	},
	{DTMB_THD_DW1,					0x1fb	},
	{DTMB_THD_UP2,					0x86	},
	{DTMB_THD_DW2,					0x7a	},
	{DTMB_GAIN_PULSE_SPACE_LEN,		0x1		},
	{DTMB_GAIN_PULSE_HOLD_LEN,		0x1		},
	{DTMB_GAIN_STEP_SUM_UP_THD,		0xb		},
	{DTMB_GAIN_STEP_SUM_DW_THD,		0x5		},
};

static const REG_VALUE_ENTRY R830_OPT_VALUE_DTMB[] = {
	// RegBitName,				Value
	{DTMB_Z_AGC,					0x2		},
	{DTMB_EN_PGA_MODE,				0x0		},
	//{DTMB_TARGET_VAL,				0x38	},
	{DTMB_AAGC_LOOPGAIN1,			0x15	},
	{DTMB_INTEGRAL_CNT_LEN,			0xc		},
	{DTMB_AAGC_LOCK_PGA_HIT_LEN,	0x1		},
	{DTMB_THD_LOCK_UP,				0x205	},
	{DTMB_THD_LOCK_DW,				0x1fb	},
	{DTMB_THD_UP1,					0x205	},
	{DTMB_THD_DW1,					0x1fb	},
	{DTMB_THD_UP2,					0x86	},
	{DTMB_THD_DW2,					0x7a	},
	{DTMB_GAIN_PULSE_SPACE_LEN,		0x1		},
	{DTMB_GAIN_PULSE_HOLD_LEN,		0x1		},
	{DTMB_GAIN_STEP_SUM_UP_THD,		0xb		},
	{DTMB_GAIN_STEP_SUM_DW_THD,		0x5		},
};



#endif

