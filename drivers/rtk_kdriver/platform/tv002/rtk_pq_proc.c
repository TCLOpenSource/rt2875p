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
#include <linux/compiler.h>

#include "rtk_pq_tv002_api.h"

#include <rtk_pq_proc.h>
#include <tvscalercontrol/vip/frequency_detection.h>

#include <rtd_log/rtd_module_log.h>
#define TAG_NAME "DPQ"

RTK_TV002_CALLBACK_RunCalc  cb_RTK_TV002_PQLib_RunDConCalc = NULL;
RTK_TV002_CALLBACK_RunCalc  cb_RTK_TV002_PQLib_RunDColCalc = NULL;
RTK_TV002_CALLBACK_RunCalc  cb_RTK_TV002_PQLib_RunDBCCalc = NULL;
RTK_TV002_CALLBACK_RunCalc  cb_RTK_TV002_PQLib_RunLDCalc = NULL;
RTK_TV002_CALLBACK_RunDSHPCalc cb_RTK_TV002_PQLib_RunDSHPCalc = NULL;

RTK_TV002_CALLBACK_SetScalerMeter cb_RTK_TV002_PQLib_SetScalerMeter = NULL;
RTK_TV002_CALLBACK_SetLdMeter  cb_RTK_TV002_PQLib_SetLdMeter = NULL;
RTK_TV002_CALLBACK_SetGeneralFunc  cb_RTK_TV002_PQLib_SetGeneralFunc = NULL;
RTK_TV002_CALLBACK_SetShareMemoryArea  cb_RTK_TV002_PQLib_SetShareMemoryArea = NULL;
RTK_TV002_CALLBACK_SetVgipIsrStatus cb_RTK_TV002_PQLib_SetVgipIsrStatus = NULL;
RTK_TV002_CALLBACK_SetOsdInfo cb_RTK_TV002_PQLib_SetOsdInfo = NULL;
RTK_TV002_CALLBACK_SetNoiseLevel cb_RTK_TV002_PQLib_SetNoiseLevel = NULL;
RTK_TV002_CALLBACK_SetFreqDet cb_RTK_TV002_PQLib_SetFreqDet = NULL;

RTK_TV002_CALLBACK_GetDConOut  cb_RTK_TV002_PQLib_GetDConOut = NULL;
RTK_TV002_CALLBACK_GetDColOut  cb_RTK_TV002_PQLib_GetDColOut = NULL;
RTK_TV002_CALLBACK_GetDBCOut  cb_RTK_TV002_PQLib_GetDBCOut = NULL;
RTK_TV002_CALLBACK_GetLDOut  cb_RTK_TV002_PQLib_GetLDOut = NULL;
RTK_TV002_CALLBACK_GetGeneralFunc  cb_RTK_TV002_PQLib_GetGeneralFunc = NULL;
RTK_TV002_CALLBACK_GetDShpOut cb_RTK_TV002_PQLib_GetDShpOut = NULL;

extern void vpqex_set_sopq_ShareMem(void);

void pq_ctrl_tv002_init(void)
{
	vpqex_set_sopq_ShareMem();
}

extern unsigned char g_bFreqDet_Setting_by_ISR;
extern FREQ_DET_RTICE g_Freq_Det_TV002_Save;
void Scaler_Set4BandHFilter(RTK_TV002_PQLIB_4BAND_HFILTER_T *tDShpHFilter)
{
	DRV_FreqDet_Filter_t flt;

	if (tDShpHFilter == NULL)
		return;

	g_bFreqDet_Setting_by_ISR = true;

	flt.BitSft_A = tDShpHFilter->u1bitshift_a0;
	flt.BitSft_B = tDShpHFilter->u1bitshift_b0;
	flt.Switch = tDShpHFilter->u1h_filter_switch_0;
	flt.Div = tDShpHFilter->u1h_filter_band0_div;
	flt.Coef[0] = tDShpHFilter->i2h_filter_band0_coef[0];
	flt.Coef[1] = tDShpHFilter->i2h_filter_band0_coef[1];
	flt.Coef[2] = tDShpHFilter->i2h_filter_band0_coef[2];
	flt.Coef[3] = tDShpHFilter->i2h_filter_band0_coef[3];
	flt.Coef[4] = tDShpHFilter->i2h_filter_band0_coef[4];
	flt.Coef[5] = tDShpHFilter->i2h_filter_band0_coef[5];
	flt.Coef[6] = tDShpHFilter->i2h_filter_band0_coef[6];
	flt.Coef[7] = tDShpHFilter->i2h_filter_band0_coef[7];
	flt.Coef[8] = tDShpHFilter->i2h_filter_band0_coef[8];
	flt.Coef[9] = tDShpHFilter->i2h_filter_band0_coef[9];
	flt.Coef[10] = tDShpHFilter->i2h_filter_band0_coef[10];
	drvif_color_set_set_frequency_detection_filter(&flt, _FD_FLT_TAP21_);

	flt.BitSft_A = tDShpHFilter->u1bitshift_a1;
	flt.BitSft_B = tDShpHFilter->u1bitshift_b1;
	flt.Switch = tDShpHFilter->u1h_filter_switch_1;
	flt.Div = tDShpHFilter->u1h_filter_band1_div;
	flt.Coef[0] = tDShpHFilter->i1h_filter_band1_coef[0];
	flt.Coef[1] = tDShpHFilter->i1h_filter_band1_coef[1];
	flt.Coef[2] = tDShpHFilter->i1h_filter_band1_coef[2];
	flt.Coef[3] = tDShpHFilter->i1h_filter_band1_coef[3];
	flt.Coef[4] = tDShpHFilter->i1h_filter_band1_coef[4];
	flt.Coef[5] = tDShpHFilter->i1h_filter_band1_coef[5];
	flt.Coef[6] = tDShpHFilter->i1h_filter_band1_coef[6];
	flt.Coef[7] = tDShpHFilter->i1h_filter_band1_coef[7];
	drvif_color_set_set_frequency_detection_filter(&flt, _FD_FLT_TAP15_1);

	flt.BitSft_A = tDShpHFilter->u1bitshift_a2;
	flt.BitSft_B = tDShpHFilter->u1bitshift_b2;
	flt.Switch = tDShpHFilter->u1h_filter_switch_2;
	flt.Div = tDShpHFilter->u1h_filter_band2_div;
	flt.Coef[0] = tDShpHFilter->i1h_filter_band2_coef[0];
	flt.Coef[1] = tDShpHFilter->i1h_filter_band2_coef[1];
	flt.Coef[2] = tDShpHFilter->i1h_filter_band2_coef[2];
	flt.Coef[3] = tDShpHFilter->i1h_filter_band2_coef[3];
	flt.Coef[4] = tDShpHFilter->i1h_filter_band2_coef[4];
	flt.Coef[5] = tDShpHFilter->i1h_filter_band2_coef[5];
	flt.Coef[6] = tDShpHFilter->i1h_filter_band2_coef[6];
	flt.Coef[7] = tDShpHFilter->i1h_filter_band2_coef[7];
	drvif_color_set_set_frequency_detection_filter(&flt, _FD_FLT_TAP15_2);

	flt.BitSft_A = tDShpHFilter->u1bitshift_a3;
	flt.BitSft_B = tDShpHFilter->u1bitshift_b3;
	flt.Switch = tDShpHFilter->u1h_filter_switch_3;
	flt.Div = tDShpHFilter->u1h_filter_band3_div;
	flt.Coef[0] = tDShpHFilter->i1h_filter_band3_coef[0];
	flt.Coef[1] = tDShpHFilter->i1h_filter_band3_coef[1];
	flt.Coef[2] = tDShpHFilter->i1h_filter_band3_coef[2];
	flt.Coef[3] = tDShpHFilter->i1h_filter_band3_coef[3];
	flt.Coef[4] = tDShpHFilter->i1h_filter_band3_coef[4];
	flt.Coef[5] = tDShpHFilter->i1h_filter_band3_coef[5];
	flt.Coef[6] = tDShpHFilter->i1h_filter_band3_coef[6];
	flt.Coef[7] = tDShpHFilter->i1h_filter_band3_coef[7];
	drvif_color_set_set_frequency_detection_filter(&flt, _FD_FLT_TAP15_3);

	//for rtice start
	{
		int i;
		FREQ_DET_RTICE *p = &g_Freq_Det_TV002_Save;

		p->filter.bitshift_a0 = tDShpHFilter->u1bitshift_a0;
		p->filter.bitshift_b0 = tDShpHFilter->u1bitshift_b0;
		p->filter.h_filter_switch_0 = tDShpHFilter->u1h_filter_switch_0;
		p->filter.h_filter_band0_div = tDShpHFilter->u1h_filter_band0_div;
		for (i = 0; i < 21; i++)
			p->filter.h_filter_band0_coef[i] = tDShpHFilter->i2h_filter_band0_coef[i];

		p->filter.bitshift_a1 = tDShpHFilter->u1bitshift_a1;
		p->filter.bitshift_b1 = tDShpHFilter->u1bitshift_b1;
		p->filter.h_filter_switch_1 = tDShpHFilter->u1h_filter_switch_1;
		p->filter.h_filter_band1_div = tDShpHFilter->u1h_filter_band1_div;
		for (i = 0; i < 15; i++)
			p->filter.h_filter_band1_coef[i] = tDShpHFilter->i1h_filter_band1_coef[i];

		p->filter.bitshift_a2 = tDShpHFilter->u1bitshift_a2;
		p->filter.bitshift_b2 = tDShpHFilter->u1bitshift_b2;
		p->filter.h_filter_switch_2 = tDShpHFilter->u1h_filter_switch_2;
		p->filter.h_filter_band2_div = tDShpHFilter->u1h_filter_band2_div;
		for (i = 0; i < 15; i++)
			p->filter.h_filter_band1_coef[i] = tDShpHFilter->i1h_filter_band1_coef[i];

		p->filter.bitshift_a3 = tDShpHFilter->u1bitshift_a3;
		p->filter.bitshift_b3 = tDShpHFilter->u1bitshift_b3;
		p->filter.h_filter_switch_3 = tDShpHFilter->u1h_filter_switch_3;
		p->filter.h_filter_band3_div = tDShpHFilter->u1h_filter_band3_div;
		for (i = 0; i < 15; i++)
			p->filter.h_filter_band3_coef[i] = tDShpHFilter->i1h_filter_band3_coef[i];
	}
	//for rtice end
}

void Scaler_Set4BandFilterRange(RTK_TV002_PQLIB_4BAND_FILTERRANGE_T *tDshpFilterRng)
{
	DRV_FreqDet_Region_t rgn;

	if (tDshpFilterRng == NULL)
		return;

	g_bFreqDet_Setting_by_ISR = true;

	rgn.HSta = tDshpFilterRng->u2h_filter_h_start;
	rgn.Width = tDshpFilterRng->u2h_filter_h_size;
	rgn.VSta = tDshpFilterRng->u2h_filter_v_start;
	rgn.Height = tDshpFilterRng->u2h_filter_v_size;
	
	drvif_color_set_set_frequency_detection_region(&rgn);

	//for rtice start
	{
		FREQ_DET_RTICE *p = &g_Freq_Det_TV002_Save;

		p->range.filter_h_start = tDshpFilterRng->u2h_filter_h_start;
		p->range.filter_h_size = tDshpFilterRng->u2h_filter_h_size;
		p->range.filter_v_start = tDshpFilterRng->u2h_filter_v_start;
		p->range.filter_v_size = tDshpFilterRng->u2h_filter_v_size;
	}
	//for rtice end

}

EXPORT_SYMBOL(cb_RTK_TV002_PQLib_RunDConCalc);
EXPORT_SYMBOL(cb_RTK_TV002_PQLib_RunDColCalc);
EXPORT_SYMBOL(cb_RTK_TV002_PQLib_RunDBCCalc);
EXPORT_SYMBOL(cb_RTK_TV002_PQLib_RunLDCalc);
EXPORT_SYMBOL(cb_RTK_TV002_PQLib_RunDSHPCalc);

EXPORT_SYMBOL(cb_RTK_TV002_PQLib_SetScalerMeter);
EXPORT_SYMBOL(cb_RTK_TV002_PQLib_SetLdMeter);
EXPORT_SYMBOL(cb_RTK_TV002_PQLib_SetGeneralFunc);
EXPORT_SYMBOL(cb_RTK_TV002_PQLib_SetShareMemoryArea);
EXPORT_SYMBOL(cb_RTK_TV002_PQLib_SetVgipIsrStatus);
EXPORT_SYMBOL(cb_RTK_TV002_PQLib_SetOsdInfo);
EXPORT_SYMBOL(cb_RTK_TV002_PQLib_SetNoiseLevel);
EXPORT_SYMBOL(cb_RTK_TV002_PQLib_SetFreqDet);

EXPORT_SYMBOL(cb_RTK_TV002_PQLib_GetDConOut);
EXPORT_SYMBOL(cb_RTK_TV002_PQLib_GetDColOut);
EXPORT_SYMBOL(cb_RTK_TV002_PQLib_GetDBCOut);
EXPORT_SYMBOL(cb_RTK_TV002_PQLib_GetLDOut);
EXPORT_SYMBOL(cb_RTK_TV002_PQLib_GetGeneralFunc);
EXPORT_SYMBOL(cb_RTK_TV002_PQLib_GetDShpOut);

EXPORT_SYMBOL(pq_ctrl_tv002_init);
EXPORT_SYMBOL(Scaler_Set4BandHFilter);
EXPORT_SYMBOL(Scaler_Set4BandFilterRange);
