#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/uaccess.h>
#include <asm/cacheflush.h>
#include <linux/pageremap.h>

#include "sopq_lib_ext_api.h"
#include "pq_ctrl_tv002.h"

#include <rtd_log/rtd_module_log.h>
#include "rtk_vip_logger.h"
#define TAG_NAME "PQ"

extern RTK_TV002_CALLBACK_RunCalc cb_RTK_TV002_PQLib_RunDConCalc;
extern RTK_TV002_CALLBACK_RunCalc cb_RTK_TV002_PQLib_RunDColCalc;
extern RTK_TV002_CALLBACK_RunCalc cb_RTK_TV002_PQLib_RunDBCCalc;
extern RTK_TV002_CALLBACK_RunCalc cb_RTK_TV002_PQLib_RunLDCalc;
extern RTK_TV002_CALLBACK_RunDSHPCalc cb_RTK_TV002_PQLib_RunDSHPCalc;

extern RTK_TV002_CALLBACK_SetScalerMeter cb_RTK_TV002_PQLib_SetScalerMeter;
extern RTK_TV002_CALLBACK_SetLdMeter cb_RTK_TV002_PQLib_SetLdMeter;
extern RTK_TV002_CALLBACK_SetGeneralFunc cb_RTK_TV002_PQLib_SetGeneralFunc;
extern RTK_TV002_CALLBACK_SetShareMemoryArea cb_RTK_TV002_PQLib_SetShareMemoryArea;
extern RTK_TV002_CALLBACK_SetVgipIsrStatus cb_RTK_TV002_PQLib_SetVgipIsrStatus;
extern RTK_TV002_CALLBACK_SetOsdInfo cb_RTK_TV002_PQLib_SetOsdInfo;
extern RTK_TV002_CALLBACK_SetNoiseLevel cb_RTK_TV002_PQLib_SetNoiseLevel;
extern RTK_TV002_CALLBACK_SetFreqDet cb_RTK_TV002_PQLib_SetFreqDet;

extern RTK_TV002_CALLBACK_GetDConOut cb_RTK_TV002_PQLib_GetDConOut;
extern RTK_TV002_CALLBACK_GetDColOut cb_RTK_TV002_PQLib_GetDColOut;
extern RTK_TV002_CALLBACK_GetDBCOut cb_RTK_TV002_PQLib_GetDBCOut;
extern RTK_TV002_CALLBACK_GetLDOut cb_RTK_TV002_PQLib_GetLDOut;
extern RTK_TV002_CALLBACK_GetGeneralFunc cb_RTK_TV002_PQLib_GetGeneralFunc;
extern RTK_TV002_CALLBACK_GetDShpOut cb_RTK_TV002_PQLib_GetDShpOut;

extern void pq_ctrl_tv002_init(void);

extern void IoReg_Write32 (unsigned int addr, unsigned int value);
extern unsigned int IoReg_Read32 (unsigned int addr);
extern void Scaler_Set4BandHFilter(RTK_TV002_PQLIB_4BAND_HFILTER_T *tDShpHFilter);
extern void Scaler_Set4BandFilterRange(RTK_TV002_PQLIB_4BAND_FILTERRANGE_T *tDshpFilterRng);

void RTK_TV002_PQLib_GetHardwareReg(UINT32 u4PhysicalAddress, UINT32 u4RegValue)
{
	IoReg_Write32(u4PhysicalAddress, u4RegValue);
}

UINT32 RTK_TV002_PQLib_SetHardwareReg (UINT32 u4PhysicalAddress)
{
	return IoReg_Read32(u4PhysicalAddress);
}

INT32 RTK_TV002_to_SonyPQLib_SetVgipIsrStatus( E_RTK_TV002_PQLIB_VGIP_ISR_STATUS eVgipIsrStatus )
{
	return SonyPQLib_SetVgipIsrStatus((E_SONYPQLIB_VGIP_ISR_STATUS)eVgipIsrStatus);
}

void RTK_TV002_PQLib_Set4BandHFilter(RTK_TV002_PQLIB_4BAND_HFILTER_T *tDShpHFilter)
{
	Scaler_Set4BandHFilter(tDShpHFilter);
}

void RTK_TV002_PQLib_Set4BandFilterRange(RTK_TV002_PQLIB_4BAND_FILTERRANGE_T *tDshpFilterRng)
{
	Scaler_Set4BandFilterRange(tDshpFilterRng);
}

int rtk_pq_register_callback(void)
{
	cb_RTK_TV002_PQLib_RunDConCalc = SonyPQLib_RunDConCalc;
	cb_RTK_TV002_PQLib_RunDColCalc = SonyPQLib_RunDColCalc;
	cb_RTK_TV002_PQLib_RunDBCCalc = SonyPQLib_RunDBCCalc;
	cb_RTK_TV002_PQLib_RunLDCalc = SonyPQLib_RunLDCalc;
	cb_RTK_TV002_PQLib_RunDSHPCalc = SonyPQLib_RunDSHPCalc;

	cb_RTK_TV002_PQLib_SetScalerMeter = SonyPQLib_SetScalerMeter;
	cb_RTK_TV002_PQLib_SetLdMeter = SonyPQLib_SetLdMeter;
	cb_RTK_TV002_PQLib_SetGeneralFunc = SonyPQLib_SetGeneralFunc;
	cb_RTK_TV002_PQLib_SetShareMemoryArea = SonyPQLib_SetShareMemoryArea;
	cb_RTK_TV002_PQLib_SetVgipIsrStatus = RTK_TV002_to_SonyPQLib_SetVgipIsrStatus;
	cb_RTK_TV002_PQLib_SetOsdInfo = SonyPQLib_SetOsdInfo;
	cb_RTK_TV002_PQLib_SetNoiseLevel = SonyPQLib_SetNoiseLevel;
	cb_RTK_TV002_PQLib_SetFreqDet = SonyPQLib_SetFreqDet;

	cb_RTK_TV002_PQLib_GetDConOut = SonyPQLib_GetDConOut;
	cb_RTK_TV002_PQLib_GetDColOut = SonyPQLib_GetDColOut;
	cb_RTK_TV002_PQLib_GetDBCOut = SonyPQLib_GetDBCOut;
	cb_RTK_TV002_PQLib_GetLDOut = SonyPQLib_GetLDOut;
	cb_RTK_TV002_PQLib_GetGeneralFunc = SonyPQLib_GetGeneralFunc;
	cb_RTK_TV002_PQLib_GetDShpOut = SonyPQLib_GetDShpOut;
	
	return 0;
}

int rtk_pq_deregister_callback(void)
{
	cb_RTK_TV002_PQLib_RunDConCalc = NULL;
	cb_RTK_TV002_PQLib_RunDColCalc = NULL;
	cb_RTK_TV002_PQLib_RunDBCCalc = NULL;
	cb_RTK_TV002_PQLib_RunLDCalc = NULL;
	cb_RTK_TV002_PQLib_RunDSHPCalc = NULL;

	cb_RTK_TV002_PQLib_SetScalerMeter = NULL;
	cb_RTK_TV002_PQLib_SetLdMeter = NULL;
	cb_RTK_TV002_PQLib_SetGeneralFunc = NULL;
	cb_RTK_TV002_PQLib_SetShareMemoryArea = NULL;
	cb_RTK_TV002_PQLib_SetVgipIsrStatus = NULL;
	cb_RTK_TV002_PQLib_SetOsdInfo = NULL;
	cb_RTK_TV002_PQLib_SetNoiseLevel = NULL;
	cb_RTK_TV002_PQLib_SetFreqDet = NULL;

	cb_RTK_TV002_PQLib_GetDConOut = NULL;
	cb_RTK_TV002_PQLib_GetDColOut = NULL;
	cb_RTK_TV002_PQLib_GetDBCOut = NULL;
	cb_RTK_TV002_PQLib_GetLDOut = NULL;
	cb_RTK_TV002_PQLib_GetGeneralFunc = NULL;
	cb_RTK_TV002_PQLib_GetDShpOut = NULL;
	
	return 0;
}

static int pq_ctrl_tv002_init_module(void)
{
	int result;
	result = rtk_pq_register_callback();
	if (result < 0) {
		rtd_pr_platform_err("["TAG_NAME"]""pq_ctrl_tv002: can not register callback...\n");
		return result;
	}

	pq_ctrl_tv002_init();

	rtd_pr_platform_emerg("["TAG_NAME"]""pq_ctrl_tv002_mod insmod OK!\n");

	return 0;
}

static void pq_ctrl_tv002_exit_module(void)
{
	rtk_pq_deregister_callback();
	msleep(50);
}

module_init(pq_ctrl_tv002_init_module);
module_exit(pq_ctrl_tv002_exit_module);

