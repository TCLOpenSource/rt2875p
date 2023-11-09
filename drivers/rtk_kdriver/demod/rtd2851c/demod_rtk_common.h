#ifndef __DEMOD_REALTEK_COMMON_H__
#define __DEMOD_REALTEK_COMMON_H__

#include "demodcore/foundation.h"
#include "demod_rtk_def.h"
#include <mach/rtk_platform.h>

//#define DEMOD_DVBT2_METACLOCK_594MHZ
//#define DEMOD_DTMB_DTVFRONTEND_BYPASS
#define DEMOD_DTMB_DTVFRONTEND_DOWDSAMPLE

//#define DEMOD_LDR_CAHNGE_BY_FILE
#define DEMOD_MEM_START_PHYS 0x15400000
#define DEMOD_MEM_END_PHYS 0x0153fffff
#define DEMOD_MEM_LENGTH 0x00800000

#define RTK_A_DVBT2_DDR_SIZE  (2 * 1024 * 1024)    // 2MBs
#define RTK_A_DVBC2_DDR_SIZE  (2 * 1024 * 1024)    // 2MBs
#define RTK_A_DVBSX_DDR_SIZE  (0 * 1024 * 1024)    // 0MBs
#define RTK_H_DTMB_DDR_SIZE  (2 * 1024 * 1024)    // 2MBs
#define RTK_R_ATSC1_DDR_SIZE  (256 * 1024)	// 256KBs

#ifndef _TRUE
#define _TRUE 1
#endif
#ifndef _FALSE
#define _FALSE 0
#endif

#ifndef ABS
#define ABS(x,y)				((x > y) ? (x-y) : (y-x))
#endif

typedef enum {
	PGAFIX_N7DB = -7,
	PGAFIX_N6DB = -6,
	PGAFIX_N5DB = -5,
	PGAFIX_N4DB = -5,
	PGAFIX_N3DB = -5,
	PGAFIX_N2DB = -5,
	PGAFIX_N1DB = -5,
	PGAFIX_0DB = 0,
	PGAFIX_1DB = 1,
	PGAFIX_2DB = 2,
	PGAFIX_3DB = 3,
	PGAFIX_4DB = 4,
	PGAFIX_5DB = 5,
	PGAFIX_6DB = 6,
	PGAFIX_7DB = 7,
	PGAFIX_8DB = 8,
	PGAFIX_9DB = 9,
	PGAFIX_10DB = 10,
	PGAFIX_11DB = 11,
	PGAFIX_12DB = 12,
} IFADCGAIN;


	typedef struct {
		unsigned int TpOutClk;
		unsigned int pre_div;
		unsigned int post_div;
		unsigned int div16;
	}
	SETTING_ENTRY_TP_OUTPUT_CLK;

/* support diff arch(arm64 or another) */

#ifdef CONFIG_ARM64
typedef unsigned long DUADDRESS;
#define PT_UDEC_DUADDRESS "%lu"
#define PT_DEC_DUADDRESS "%ld"
#define PT_HEX_DUADDRESS "%lx"
#else
typedef unsigned int DUADDRESS;
#define PT_UDEC_DUADDRESS "%u"
#define PT_DEC_DUADDRESS "%d"
#define PT_HEX_DUADDRESS "%x"
#endif


int DtvGetChipVersion(void);
int DtvGetICVersion(void);
int DtvPGAControl(unsigned char on, IFADCGAIN val);
int DtvDemodInitialization(RTK_DEMOD_MODE mode, int force_rst);
int ExtAtsc3DemodInitialization(RTK_DEMOD_MODE mode, int force_rst);
int DtvDemodCheckPll(RTK_DEMOD_MODE mode);
int DtvDemodPllSettingCheck(RTK_DEMOD_MODE mode);
int DtvDemodCRTSettingCheck(void);
int DtvDemodClockEnable(unsigned char On);
int DtvIsDemodClockEnable(RTK_DEMOD_MODE mode);
int TvFrontendPowerControl(unsigned char On);
int TvFrontendPowerControlIsdbt(unsigned char On);
int DtvDemodGetPowerStatus(void);
int IQAdcLdoSetting(void);
int QVcmSetting(void);
int DtvDemodAdcPower(unsigned char on);
int DtvDemodAdcPowerCheck(void);
int DtvIAdcIPgaPowerSetting(unsigned char On);
int GetDemodTvSysMode(TV_SYSTEM_TYPE* mode);
int CheckDemodReset(RTK_DEMOD_MODE mode);
void ClearDemodReset(RTK_DEMOD_MODE mode);
void DtvDumpShareRegStatus(void);
int AtvDemodInitialization(RTK_DEMOD_MODE mode);
int AtvDemodPowerControl(unsigned char On);
int DtvDemodTPOutEn(unsigned char enable);
int DemodBisrCheck(void);
int DemodTsOutClkCtrl(unsigned int TsClkKHz);
int DtvDemodDMAAllocateMemory(void);
int DtvDemodDMADump(BASE_INTERFACE_MODULE* pBaseInterface, unsigned char DeviceAddr, unsigned char mode);
int DtvDemodMemRemappingInt(size_t size, unsigned long *phyAddr, unsigned int *StaticAllocateSize);
void DtvDemodFreeAllocMem(void *ptr);
//int ImgIndirectRegAccess(unsigned int RegAddr, unsigned int RegVal);
void*   pli_allocContinuousMemoryMesg(char *str, size_t size, unsigned char** nonCachedAddr, unsigned long *phyAddr);
void    pli_freeContinuousMemoryMesg(char *str, void *ptr);
unsigned int CravedoutMemoryQuery(unsigned long *pPhyAddr);

#ifdef DEMOD_LDR_CAHNGE_BY_FILE
struct device* get_rtkdemod_device_ptr(void);
#endif

#define RTK_DEMOD_BIF_TX_FIFO_DEPTH       16
#define RTK_DEMOD_BIF_RX_FIFO_DEPTH       16


extern void __realtek_wait_ms(
	BASE_INTERFACE_MODULE*      pBaseInterface,
	unsigned long               ms
);

extern int __realtek_i2c_write(
	BASE_INTERFACE_MODULE*      pBaseInterface,
	unsigned char               DeviceAddr,
	const unsigned char*        pWritingBytes,
	U32BITS               ByteNum
);

extern int __realtek_i2c_read(
	BASE_INTERFACE_MODULE*      pBaseInterface,
	unsigned char               DeviceAddr,
	unsigned char* pRegisterAddr,
	unsigned char RegisterAddrSize,
	unsigned char*              pReadingBytes,
	U32BITS               ByteNum
);

extern int __realtek_i2c_write_speedF(
	BASE_INTERFACE_MODULE*      pBaseInterface,
	unsigned char               DeviceAddr,
	const unsigned char*        pWritingBytes,
	U32BITS               ByteNum
);

extern int __realtek_i2c_read_speedF(
	BASE_INTERFACE_MODULE*      pBaseInterface,
	unsigned char               DeviceAddr,
	unsigned char* pRegisterAddr,
	unsigned char RegisterAddrSize,
	unsigned char*              pReadingBytes,
	U32BITS               ByteNum
);

extern int __realtek_rbus_i2c_write(
	BASE_INTERFACE_MODULE*      pBaseInterface,
	unsigned char               DeviceAddr,
	const unsigned char*        pWritingBytes,
	U32BITS              ByteNum
);

extern int __realtek_rbus_i2c_read(
	BASE_INTERFACE_MODULE*      pBaseInterface,
	unsigned char               DeviceAddr,
	unsigned char* pRegisterAddr,
	unsigned char RegisterAddrSize,
	unsigned char*              pReadingBytes,
	U32BITS               ByteNum
);

extern int __realtek_a_rbus_i2c_write(
	BASE_INTERFACE_MODULE*      pBaseInterface,
	unsigned char               DeviceAddr,
	const unsigned char*        pWritingBytes,
	U32BITS               ByteNum
);

extern int __realtek_a_rbus_i2c_read(
	BASE_INTERFACE_MODULE*      pBaseInterface,
	unsigned char               DeviceAddr,
	unsigned char* pRegisterAddr,
	unsigned char RegisterAddrSize,
	unsigned char*              pReadingBytes,
	U32BITS              ByteNum
);


unsigned int IsAtsc3McmPackage(
RTK_DEMOD_MODE mode
);

void ExtAtsc3PowOffOn(unsigned char on);

#define DECODE_RET(x)       ((x==FUNCTION_SUCCESS) ? TUNER_CTRL_OK : TUNER_CTRL_FAIL)
#define ENCODE_RET(x)       ((x==TUNER_CTRL_OK) ?  FUNCTION_SUCCESS : FUNCTION_ERROR)

#define RTK_DEMOD_PRINT_LVL_DBG      0
#define RTK_DEMOD_PRINT_LVL_INFO     1
#define RTK_DEMOD_PRINT_LVL_WARNING  2

#ifndef RTK_DEMOD_PRINT_LVL
#define RTK_DEMOD_PRINT_LVL          RTK_DEMOD_PRINT_LVL_WARNING
#endif


#define RTK_DEMOD_CALLER(fmt, args...)     rtd_demod_print(KERN_DEBUG,"CALLER, " fmt, ##args);
#define RTK_DEMOD_DBG(fmt, args...)        rtd_demod_print(KERN_DEBUG,"Debug, " fmt, ##args);
#define RTK_DEMOD_INFO(fmt, args...)       rtd_demod_print(KERN_INFO,"Info, " fmt, ##args);
#define RTK_DEMOD_WARNING(fmt, args...)    rtd_demod_print(KERN_WARNING,"Warning, " fmt, ##args);

#endif // __DEMOD_REALTEK_COMMON_H__
