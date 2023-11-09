/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2014
  * All rights reserved.
  * ========================================================================*/
  /*================= File Description =======================================*/
  /**
   * @file
   *  This file is for HDR 10+ related functions.
   *
   * @author  $Author$
   * @date	  $Date$
   * @version	  $Revision$
   */


  /*============================ Module dependency	===========================*/
#ifndef BUILD_QUICK_SHOW
#include <linux/string.h>
#include <linux/kernel.h>
#else
#include <include/string.h>
#endif
#include <tvscalercontrol/io/ioregdrv.h>

#include <tvscalercontrol/vip/tc_hdr.h>
#include <tvscalercontrol/vip/tc_math.h>//juwen, merlin3, tc//juwen, 170216, TCv1.3.0_computeMetaData
#include <tvscalercontrol/scaler/vipinclude.h> // for typedef that used in VIP_reg_def.h
#include <tvscalercontrol/vip/vip_reg_def.h>
//#include <rbus/rbusTCReg.h>
//#include <rbus/tc_reg.h>
#include <tvscalercontrol/vip/scalerColor.h>
#include <rbus/dmato3dtable_reg.h>
#include <rbus/dmato3dlut_reg.h>
#include <rtd_log/rtd_module_log.h>
#include "rtk_vip_logger.h"
#include <tvscalercontrol/vip/ST2094.h>
#include <rbus/efuse_reg.h>
#include <tvscalercontrol/hdmirx/hdmifun.h>
#include <rtk_otp_region_api.h>

static VIP_ST2094_TBL ST2094_usingTBL = {0};

#if 1	/*========================== ST2094==========================================*/
//====================20171127 ST2094, jimmy==================================================
unsigned char drvif_Get_ST2094_EFuse_Status(void)
{
	int ret = rtk_otp_field_read_int_by_name("sw_func_hdr_10_plus");

	rtd_pr_vpq_info("ST2094_opt_field ret=%d\n", ret);
	if (ret == 0)
		return ret;
	else
		return 1;
}

// Curve calculations, move to separate files when ready
long_int long_multiply( unsigned int a, unsigned int b )
{
	long_int out;
	int a_msb, a_lsb, b_msb, b_lsb;
	unsigned int mul_32, mul_16, mul_0;
	unsigned int mul_16_high, mul_16_low;
	unsigned int mul_16_1, mul_16_2;
	a_msb = a >> 16;
	a_lsb = a & 0xFFFF;
	b_msb = b >> 16;
	b_lsb = b & 0xFFFF;

	mul_0 = a_lsb * b_lsb;

	mul_16_1 = a_lsb * b_msb;
	mul_16_2 = a_msb * b_lsb;

	mul_32 = a_msb * b_msb;

	if (a == 0 || b == 0) {
		out.high = 0;
		out.low = 0;
		return out;
	}

	if( mul_16_1 > 0xFFFFFFFF - mul_16_2 )
	{
		mul_16 = mul_16_1 - (0xFFFFFFFF - mul_16_2) + 1;
		mul_32++;
	}
	else
	{
		mul_16 = mul_16_1 + mul_16_2;
	}

	mul_16_high = mul_16 >> 16;
	mul_16_low = (mul_16 & 0xFFFF) << 16;

	out.high = mul_32 + mul_16_high;
	if( mul_0 > 0xFFFFFFFF - mul_16_low )
	{
		out.low = mul_0 - (0xFFFFFFFF - mul_16_low) + 1;
		out.high++;
	}
	else
	{
		out.low = ((mul_16 & 0xFFFF) << 16) + mul_0;
	}

	return out;
}

long_int long_divide( long_int a, unsigned int b )
{
	long_int out;
	unsigned int div_high, remain_high;
	unsigned int div_32b, remain_32b;
	unsigned int remain_low;

	if (a.high == 0 && a.low == 0) {
		out.high = 0;
		out.low = 0;
		return out;
	}

	div_high = a.high / b;
	remain_high = a.high % b;

	div_32b = 0xFFFFFFFF / b;
	remain_32b = (0xFFFFFFFF % b) + 1;
	if( remain_32b == b )
	{
		remain_32b = 0;
		div_32b++;
	}

	out.high = div_high;
	out.low = remain_high * div_32b + (remain_high * remain_32b) / b + a.low / b;
	remain_low = (remain_high * remain_32b) % b + a.low % b;
	if( remain_low >= b*3/2 )
		out.low += 2;
	else if( remain_low >= b/2 )
		out.low++;

	return out;
}

long_int long_add( long_int a, long_int b )
{
	long_int out;
	if( a.low > 0xffffffff - b.low )
	{
		out.low = a.low - (0xffffffff - b.low) + 1;
		out.high = a.high + b.high + 1;
	}
	else
	{
		out.low = a.low + b.low;
		out.high = a.high + b.high;
	}

	return out;
}

unsigned int long_shift( long_int a, unsigned int shift_bit )
{
	unsigned int out;
	unsigned int mask = (1<<shift_bit) - 1;
	unsigned int round = 1<<(shift_bit-1);

	out = ( a.low + round ) >> shift_bit;
	out += ( a.high & mask ) << (32-shift_bit);

	return out;
}

unsigned int rampWeight(const unsigned int v1, const unsigned int v2, const unsigned int t1, const unsigned int t2, const unsigned int t)
{
	// fixed point 16 bits
	unsigned int retVal = v1;
	if (t1 == t2) {
		retVal = (t < t1) ? (v1) : (v2);
	}
	else {
		if (t <= t1) {
		retVal = v1;
		}
		else if (t >= t2) {
			retVal = v2;
		}
		else {
			if( v2 >= v1 )
				retVal = v1 + (v2-v1) * (t-t1) / (t2-t1);
			else
				retVal = v2 + (v1-v2) * (t2-t) / (t2-t1);
		}
	}

	return retVal;

}

extern UINT32 hdr_YUV2RGB[VIP_HDR_DM_CSC1_YUV2RGB_TABLE_Max][12];
extern UINT32 hdr_RGB2OPT[HDR_RGB2OPT_TABLE_NUM][9];
extern unsigned int HDR_Bezier_instance[bezier_curve_anchors_num][OETF_size];
extern unsigned int EOTF_LUT_2084[OETF_size];
extern unsigned int EOTF_LUT_2084_nits[OETF_size];
unsigned int EOTF_curve_ST2094[OETF_size];
unsigned short OETF_curve_ST2094[OETF_size];
unsigned int bezier_curve[OETF_size];
unsigned int knee_curve_ST2094[OETF_size];

void BasisOOTF_Params_setDefault( BasisOOTF_Params* params )
{
	int i;
	unsigned int P2ToP9_MAX1_init[10-2] = {36582, 44204, 50482, 53943, 57206, 59834, 62908, 64514};
		//{ 0.5582, 0.6745, 0.7703, 0.8231, 0.8729, 0.9130, 0.9599, 0.9844 };
	unsigned int P2ToP9_MAX2_init[10-2] = {31713, 41452, 47533, 50607, 53746, 57914, 60346 ,62783};
		//{ 0.4839, 0.6325, 0.7253, 0.7722, 0.8201, 0.8837, 0.9208, 0.9580 };

	// KP ramp base thresholds (two bounds KP # 1 and KP # 2 are computed)
	params->SY1_V1  = 0;
	params->SY1_V2  = 19661;	// 0.3, 16 bits fxp
	params->SY1_T1  = 14418;
	params->SY1_T2  = 65536; // 1
	params->SY2_V1  = 0;
	params->SY2_V2  = 13107; // 0.2
	params->SY2_T1  = 16384; // 0.25
	params->SY2_T2  = 62259; // 0.95

	// KP mixing gain (final KP from bounds KP # 1 and KP # 2 as a function of scene percentile)
	params->KP_G_V1 = 65536; // 1
	params->KP_G_V2 = 3277; // 0.05
	params->KP_G_T1 = 3277; // 0.05
	params->KP_G_T2 = 32768; // 0.5

	// Thresholds of minimum bound of P1 coefficient
	params->P1_LIMIT_V1 = 60293; // 0.92
	params->P1_LIMIT_V2 = 64225; // 0.98
	params->P1_LIMIT_T1 = 655; // 0.01
	params->P1_LIMIT_T2 = 6554; // 0.1

	// Thresholds to compute relative shape of curve (P2~P9 coefficient) by pre-defined bounds - as a function of scene percentile
	params->P2To9_T1 = 3277; // 0.05
	params->P2To9_T2 = 36045; // 0.55

	for (i=0; i<(10-2); i++) {
	    params->P2ToP9_MAX1[i] = P2ToP9_MAX1_init[i];
	    params->P2ToP9_MAX2[i] = P2ToP9_MAX2_init[i];
	}

	// Ps mixing gain (obtain all Ps coefficients) - as a function of TM dynamic compression ratio
	params->PS_G_T1 = 8192; //0.125
	params->PS_G_T2 = 62259; // 0.95

	// Post-processing : Reduce P1/P2 (to enhance mid tone) for high TM dynamic range compression cases
	params->LOW_SY_T1 = 328; //0.005
	params->LOW_SY_T2 = 2621; //0.04
	params->LOW_K_T1  = 7864; //0.12
	params->LOW_K_T2  = 26214; //0.4
	params->RED_P1_V1 = 42598; //0.65
	params->RED_P1_T1 = 6554; //0.1
	params->RED_P1_T2 = 49152; //0.75
	params->RED_P2_V1 = 52429; //0.8
	params->RED_P2_T1 = 6554; //0.1
	params->RED_P2_T2 = 49152; //0.75
}

void drvif_fwif_color_ST2094_bezier_coef_elevation ( unsigned short N, unsigned int* P, unsigned int* P_elev )
{
	int i, k;
	unsigned int tmp[bezier_curve_anchors_num] = {0};

	if (N >= bezier_curve_anchors_num) {
		rtd_pr_vpq_info("bezier coef elevation, Order error, N=%d, \n", N);
		N = 5;
	}

	// first shift P to 20-bit
	for( i=0; i<N; i++ )
		P_elev[i] = P[i] << 10;
	// last coefficient default to 1023
	P_elev[N] = 1023 << 10;

	for( k=N+2; k<=bezier_curve_anchors_num; k++ )
	{
		tmp[0] = P_elev[0]*(k-1) / k;
		for( i=1; i<k; i++ )
		{
			tmp[i] = ( P_elev[i-1]*(i+1) + P_elev[i]*(k-i-1) ) / k;
		}

		memcpy( P_elev, tmp, sizeof(int)*k );
	}
#if 0
	printk( "[bezier_coef_elevation] P after elevation:\n" );
	for( i=0; i<15; i++ )
	{
		printk( "P_elev[%d] = %8d\n", i, P_elev[i] );
	}
#endif
}

void getPercentile_50_99( ST2094_metadata *metadata, unsigned int* psll50, unsigned int* psll99 )
{
	int i;
	int curPercent = 0;
	int prevPercent = 0;
	int per50_1 = -1, per50_2 = -1, per99_1 = -1, per99_2 = -1;
	int psll50_1 = -1, psll50_2 = -1, psll99_1 = -1, psll99_2 = -1;

	(*psll50) = -1;
	(*psll99) = -1;

	for( i=0; i<metadata->num_distribution_maxrgb_percentiles; i++ )
	{
		curPercent = metadata->distribution_maxrgb_percentages[i];
		if( curPercent == 50 )
			(*psll50) = metadata->distribution_maxrgb_percentiles[i];
		else if( (*psll50) == -1 && curPercent > 50 && prevPercent < 50 )
		{
			per50_1 = prevPercent;
			per50_2 = curPercent;
			psll50_1 = (i>0) ? metadata->distribution_maxrgb_percentiles[i-1] : 0;
			psll50_2 = metadata->distribution_maxrgb_percentiles[i];
		}

		if( curPercent == 99 )
			(*psll99) = metadata->distribution_maxrgb_percentiles[i];
		else if( *psll99 == -1 && curPercent > 99 && prevPercent < 99 )
		{
			per99_1 = prevPercent;
			per99_2 = curPercent;
			psll99_1 = (i>0) ? metadata->distribution_maxrgb_percentiles[i-1] : 0;
			psll99_2 = metadata->distribution_maxrgb_percentiles[i];
		}

		prevPercent = curPercent;
	}

	// interpolate percentile if no exact one exists
	if( (*psll50) == -1 )
	{
		if(per50_2 == per50_1)
			rtd_pr_vpq_emerg("[error]getPercentile_50_99 division by zero\n");
		else
			(*psll50) = psll50_1 + (psll50_2 - psll50_1) * (50 - per50_1) / (per50_2 - per50_1);
	}

	if( (*psll99) == -1 )
	{
		if(per99_2 == per99_1)
			rtd_pr_vpq_emerg("[error]getPercentile_50_99 division by zero\n");
		else
			(*psll99) = psll99_1 + (psll99_2 - psll99_1) * (99 - per99_1) / (per99_2 - per99_1);
	}
}

unsigned int calcP1( unsigned int sx, unsigned int sy, unsigned int tgtL, unsigned int srcL,
			unsigned int p1_limit_t1, unsigned int p1_limit_t2, unsigned int p1_limit_v1, unsigned int p1_limit_v2,
			unsigned int low_sy_t1, unsigned int low_sy_t2, unsigned int low_k_t1, unsigned int low_k_t2,
			unsigned int red_p1_t1, unsigned int red_p1_t2, unsigned int red_p1_v1,
			unsigned int *p1_red_gain)
{
	const unsigned int ORDER = 10;

	unsigned int ax	= min(sx, 65535u);
	unsigned int ay	= min(sy, 65535u);

	unsigned int k		= (tgtL<<16) / srcL;
	unsigned int p1_limit = rampWeight(p1_limit_v2, p1_limit_v1, p1_limit_t1, p1_limit_t2, sy);
	unsigned int p1	= ((65535-ax) << 16) / (ORDER*(65536-ay));

	// Reduce p1 to make more mid tone details for high compression TM case.
	unsigned int low_sy_g  = rampWeight(65535, 0, low_sy_t1, low_sy_t2, sy);
	unsigned int high_k_g  = rampWeight(65535, 0, low_k_t1, low_k_t2, k);
	unsigned int high_tm_g = (low_sy_g * high_k_g) >> 16;
	unsigned int red_p1    = rampWeight(65536, red_p1_v1, red_p1_t1, red_p1_t2, high_tm_g);
	const unsigned int P1MIN  = 65536/ORDER; // 16bits fxp

	p1 	= max( min( (p1<<16) / k, p1_limit), 0u );
	p1 = min( max( (p1*red_p1)>>16, P1MIN ), p1_limit );

	if (p1_red_gain != NULL) {
		*p1_red_gain = high_tm_g;
	}

	return p1;

}

void drvif_fwif_color_ST2094_basisOOTF_cal( ST2094_metadata *metadata, unsigned int productPeak, unsigned int sourceMaxL, unsigned short* bas_Kx, unsigned short* bas_Ky, unsigned int* bas_P, VIP_ST2094_CTRL* ST2094_CTRL )
{
	int i;

	// Algorithm  Parameters
	BasisOOTF_Params params;
	// /Algorithm  Parameters

	unsigned int tgtL = productPeak; // int, in nits
	unsigned int srcL = max(tgtL, sourceMaxL); // int, in nits
	unsigned int psll50, psll99;

	unsigned int ctrL;
	unsigned int k;
	unsigned int sy1, sy2;
	unsigned int kp_g;
	unsigned int sx, sy;

	//const unsigned int NPCOEFF = 9;
#define NPCOEFF 9
	const unsigned int ORDER = 10;

	// P Coefficient
	unsigned int high_tm_g = 0;
	unsigned int p1;
	unsigned int ps2To9[NPCOEFF-1];
	unsigned int pcoeff[NPCOEFF];

	unsigned int g_p29;
	unsigned int ps_g;
	unsigned int plin;
	unsigned int red_p2;

	unsigned char Debug_Log = ST2094_CTRL->Debug_Log;
	unsigned char Debug_log_delay = ST2094_CTRL->Debug_Delay;
	static unsigned char CNT = 0;

	BasisOOTF_Params_setDefault(&params); // sets to default

	CNT++;

	getPercentile_50_99(metadata, &psll50, &psll99);

       ctrL	= (psll50<<16) / max(psll99, 1u); // 16bits fxp

	// KP - Knee Point
       k	= (tgtL<<16) / srcL;
	sy1	= rampWeight(params.SY1_V1, params.SY1_V2, params.SY1_T1, params.SY1_T2, k);
	sy2	= rampWeight(params.SY2_V1, params.SY2_V2, params.SY2_T1, params.SY2_T2, k);
	kp_g= rampWeight(params.KP_G_V1,  params.KP_G_V2,  params.KP_G_T1,  params.KP_G_T2, ctrL);
	sy	= ( kp_g*sy1 + (65536-kp_g)*sy2 + 32768 ) >> 16;
	sx	= (sy*k + 32768) >> 16;

	// P Coefficient
	p1 = calcP1(sx, sy, tgtL, srcL,
            params.P1_LIMIT_T1, params.P1_LIMIT_T2, params.P1_LIMIT_V1, params.P1_LIMIT_V2,
            params.LOW_SY_T1, params.LOW_SY_T2, params.LOW_K_T1, params.LOW_K_T2,
            params.RED_P1_T1, params.RED_P1_T2, params.RED_P1_V1,
            &high_tm_g);

	for( i=0; i<NPCOEFF-1; i++ )
	{
		g_p29 = rampWeight(params.P2ToP9_MAX2[i], params.P2ToP9_MAX1[i], params.P2To9_T1, params.P2To9_T2, ctrL);
		ps2To9[i] =  ( g_p29 * params.P2ToP9_MAX1[i] + (65536-g_p29)*params.P2ToP9_MAX2[i] ) >> 16;
	}

	ps_g = rampWeight(65536, 0, params.PS_G_T1, params.PS_G_T2, k);

       pcoeff[0] = p1;
	for( i=1; i<NPCOEFF; i++ )
	{
		plin = ((i+1)<<16)/ORDER;
		pcoeff[i] = max(min( (ps_g*ps2To9[i-1] + (65536-ps_g)*plin) >> 16, (i+1)*p1), plin);
	}

	red_p2 = rampWeight(65536, params.RED_P2_V1, params.RED_P2_T1, params.RED_P2_T2, high_tm_g);
	pcoeff[1] = max( min( (pcoeff[1]*red_p2)>>16, 2*p1 ), (2<<16)/ORDER );

	// knee point, 16bits fxp to 12bits
	(*bas_Kx) = (unsigned short)( (sx + (1<<3)) >> 4 );
	(*bas_Ky) = (unsigned short)( (sy + (1<<3)) >> 4 );

	// anchors, 16bits fxp to 10bits
	for( i=0; i<NPCOEFF; i++ )
	{
		bas_P[i] = (pcoeff[i] + (1<<5)) >> 6;
	}

	if((Debug_Log&_BIT4)!=0) {
		if (CNT >= Debug_log_delay) {
			CNT = 0;
			rtd_pr_vpq_info("[BasisOOTF] ctrL=%d, k=%d, g=%d sy1=%d, sy2=%d, sy=%d, sx=%d, p1=%d \n", ctrL, k, kp_g, sy1, sy2, sy, sx, p1 );

			rtd_pr_vpq_info("[BasisOOTF] Knee point x=%d, y=%d \n", (*bas_Kx), (*bas_Ky) );
			rtd_pr_vpq_info("[BasisOOTF] P[0~4] = %d, %d, %d, %d, %d\n", bas_P[0], bas_P[1], bas_P[2], bas_P[3], bas_P[4] );
			rtd_pr_vpq_info("[BasisOOTF] P[5~9] = %d, %d, %d, %d, %d\n", bas_P[5], bas_P[6], bas_P[7], bas_P[8], bas_P[9] );
			//rtd_pr_vpq_info("[BasisOOTF] P[10~13] = %d, %d, %d, %d\n", bas_P[10], bas_P[11], bas_P[12], bas_P[13] );
		}
	}

}

void drvif_fwif_color_ST2094_bezier_curve_cal( unsigned int* P, unsigned int* bezier_curve )
{
	int i, j;
	//unsigned long long int tmp;
	unsigned int tmp;
	long_int tmp_mul, tmp_sum;

	for( i=0; i<=1024; i++ )
	{
		//tmp = 0;
		tmp_sum.low = 0;
		tmp_sum.high = 0;

		for( j=0; j<bezier_curve_anchors_num; j++ )
		{
			//tmp += (unsigned long long int)P[j] * (unsigned long long int)HDR_Bezier_instance[j][i]; // 20-bit * 20-bit
			tmp_mul = long_multiply( P[j], HDR_Bezier_instance[j][i] );
			tmp_sum = long_add( tmp_sum, tmp_mul );
		}
		//tmp = ( tmp + (1<<19) ) >> 20; // shift to 20-bit
		tmp = long_shift( tmp_sum, 20 );

		if( tmp > 0xfffff )
			tmp = 0xfffff;

		bezier_curve[i] = tmp;
		if( i>0 && bezier_curve[i] < bezier_curve[i-1] )
			bezier_curve[i] = bezier_curve[i-1];

		//curve[i] = bezier_curve[i] >> 16;
	}

}

void drvif_fwif_color_ST2094_bezier_knee_cal( unsigned int* bezier_curve, unsigned int Kx, unsigned int Ky, unsigned int* bezier_knee_curve )
{
	int i;
	unsigned int interp;
	unsigned int sample;
	unsigned int idx, sub;

	// assuming Kx & Ky are 12-bits

	for( i=0; i<=1024; i++ )
	{
		if( Kx > 0 )
		{
			if( i*4 <= Kx )
			{
				bezier_knee_curve[i] = (Ky << 8) * (i<<2) / Kx; // 20-bit * 12-bit / 12-bit
			}
			else
			{
				// resample
				sample = ( (i*4 -Kx) << 20 ) / (4096-Kx); // 20 bits: 10-bit int, 10-bit decimal
				idx = sample >> 10;
				sub = sample & ((1<<10)-1);

				if( idx < 1024 )
					interp = bezier_curve[idx] + ( ( (bezier_curve[idx+1]-bezier_curve[idx]) * sub + (1<<9) ) >> 10 );
				else
					interp = bezier_curve[1024];

				bezier_knee_curve[i] = (Ky << 8) + ( ( interp * (4096-Ky) ) >> 12 );
			}
		}
		else
			bezier_knee_curve[i] = (Ky << 8) + ( ( bezier_curve[i] * (4096-Ky) ) >> 12 );

	}
}

void drvif_fwif_color_ST2094_ratio_curve_cal( unsigned int* bezier_curve, unsigned int Kx, unsigned int Ky, unsigned short* ratio_curve, unsigned char* shiftBit )
{
	int i;
#if 1
	unsigned int tmp;
	unsigned int curve_max = 0;

	for( i=0; i<=1024; i++ )
	{
		// calculate gain (20-bit curve, (20 << 6) / 10bit index for 16-bit result)
		if( Kx == 0 )
		{
			if( i==0 )
				bezier_curve[i] = bezier_curve[1] << 6;
			else
				bezier_curve[i] = (bezier_curve[i] << 6) / i;
		}
		else
		{
			if( i < Kx/4)
				bezier_curve[i] = ((Ky<<16) + (Kx>>1)) / Kx;
			else if (i != 0)
				bezier_curve[i] = (bezier_curve[i] << 6) / i;
		}

		if( bezier_curve[i] > curve_max )
			curve_max = bezier_curve[i];
	}
#if 1
	// determine shift bit
	if( curve_max < (1<<16) )
		*shiftBit = 0;
	else if( curve_max < (1<<17) )
		*shiftBit = 1;
	else if( curve_max < (1<<18) )
		*shiftBit = 2;
	else if( curve_max < (1<<19) )
		*shiftBit = 3;
	else
		*shiftBit = 4;

	//VIPprintf( "gain max = 0x%x, shift_bit = %d\n", curve_max, *shift_bit );

	for( i=0; i<=1024; i++ )
	{
		tmp = bezier_curve[i] >> (*shiftBit);
		if( tmp > 0xffff )
			tmp = 0xffff;
		ratio_curve[i] = (unsigned short)tmp;

		//if( i>0 &&  ratio_curve[i-1] < ratio_curve[i] )
		//	ratio_curve[i] = ratio_curve[i-1];
	}
#else
	*gain = (curve_max << 10) / 0xFFFF;	/* 1024 == 1*/

	for( i=0; i<=1024; i++ )
	{
		tmp = (bezier_curve[i] << 10) /(*gain);
		if( tmp > 0xffff )
			tmp = 0xffff;
		ratio_curve[i] = (unsigned short)tmp;

		//if( i>0 &&  ratio_curve[i-1] < ratio_curve[i] )
		//	ratio_curve[i] = ratio_curve[i-1];
	}

#endif
#endif
	/*
	// Use direct mapping solution
	for( i=0; i<=1024; i++ )
	{
		ratio_curve[i] = (unsigned short)(bezier_curve[i] >> 4);
	}

	*gain = 1024;

	*/
}

void drvif_fwif_color_ST2094_OETF_Gamma22(unsigned int* bezier_curve, unsigned short* ratio_curve)
{
	extern unsigned short OETF_gamma22_lo_ST2094[OETF_gamma22_Lo_ST2094_size];
	extern unsigned short OETF_gamma22_hi_ST2094[OETF_gamma22_Hi_ST2094_size];
	long_int tmp_long, tmp_long2;

	unsigned short i, index;
	unsigned int ratio_curve_10, scale_lo, scale_hi;

	scale_lo = (OETF_gamma22_Lo_Hi_th_ST2094 << 10) / (OETF_gamma22_Lo_ST2094_size - 1) + 1;
	scale_hi = ((OETF_max - OETF_gamma22_Lo_Hi_th_ST2094) << 10) / (OETF_gamma22_Hi_ST2094_size - 1) + 1;

	for( i=0; i<OETF_size; i++ ) {
		if (bezier_curve[i] == OETF_gamma22_Lo_Hi_th_ST2094)	{
			ratio_curve[i] = OETF_gamma22_lo_ST2094[OETF_gamma22_Lo_ST2094_size-1];
		} else if (bezier_curve[i] == OETF_max)	{
			ratio_curve[i] = OETF_gamma22_hi_ST2094[OETF_gamma22_Hi_ST2094_size-1];
		} else if (bezier_curve[i] < OETF_gamma22_Lo_Hi_th_ST2094)	{	// for dark scene
			ratio_curve_10 = bezier_curve[i] << 10;
			index =  ratio_curve_10 / scale_lo;
			ratio_curve_10 = ratio_curve_10 - (index * scale_lo);
			tmp_long = long_multiply(ratio_curve_10, (OETF_gamma22_lo_ST2094[index+1] - OETF_gamma22_lo_ST2094[index]));
			tmp_long2 = long_divide(tmp_long, scale_lo);
			ratio_curve[i]  = tmp_long2.low + OETF_gamma22_lo_ST2094[index];
		} else {	// bright scene
			ratio_curve_10 = (bezier_curve[i] - OETF_gamma22_Lo_Hi_th_ST2094)<< 10;
			index =  ratio_curve_10 / scale_hi;
			ratio_curve_10 = ratio_curve_10 - (index * scale_hi);
			tmp_long = long_multiply(ratio_curve_10, (OETF_gamma22_hi_ST2094[index+1] - OETF_gamma22_hi_ST2094[index]));
			tmp_long2 = long_divide(tmp_long, scale_hi);
			ratio_curve[i]  = tmp_long2.low + OETF_gamma22_hi_ST2094[index];
		}
	}

}

void drvif_fwif_color_Set_ST2094_EOTF_LinearBlend(unsigned int *pEOTF, VIP_ST2094_CTRL* ST2094_CTRL)
{
	unsigned short i;
	long_int Linear_curve;
	long_int tmp1, tmp2;
	//unsigned int wt = (ST2094_CTRL->Debug_EOTF_Linear_Blend_W == 0) ? 300 : (ST2094_CTRL->Debug_EOTF_Linear_Blend_W);
	unsigned int wt = ST2094_usingTBL.EOTF_Linear_Blend_W;
	const unsigned int EOTF_Linear_Scale = 4194304; /* = (0xFFFFFFFF+1) / (EOTF_size-1)*/

	for(i=0;i<EOTF_size;i++) {
		Linear_curve = long_multiply(i, EOTF_Linear_Scale);
		if (Linear_curve.high>0) {
			Linear_curve.high = 0;
			Linear_curve.low = 0xFFFFFFFF;
		}

		if (pEOTF[i] < Linear_curve.low) {
			tmp1 = long_multiply((1048576-wt), pEOTF[i]);
			tmp2 = long_multiply(wt, Linear_curve.low);
			tmp1 = long_add(tmp1, tmp2);
			pEOTF[i] = long_shift(tmp1, 20);
		}
	}

}
extern unsigned int sourceMaxL_a;
void drvif_fwif_color_Set_HDR10_EOTF_depNITS(void)
{
	int i;
	long_int tmp1, tmp2;
	//extern unsigned int sourceMaxL_a;
	//printk("[hdr10+]sourceMaxL_a=%d\n",sourceMaxL_a);
	//printk("[hdr10+]EOTF_LUT_2084[500]=0x%8x\n",EOTF_LUT_2084[500]);
	//printk("[hdr10+]EOTF_LUT_2084_nits[500]=0x%8x\n",EOTF_LUT_2084_nits[500]);

	for( i=0; i<EOTF_size; i++ )
	{
		tmp1 = long_multiply( EOTF_LUT_2084[i], 10000 );
		tmp2 = long_divide( tmp1, sourceMaxL_a );
		EOTF_LUT_2084_nits[i] = (tmp2.high > 0) ? 0xFFFFFFFF : tmp2.low;
	}
	//printk("[hdr10+]a_EOTF_LUT_2084[500]=0x%8x\n",EOTF_LUT_2084[500]);
	//printk("[hdr10+]a_EOTF_LUT_2084_nits[500]=0x%8x\n",EOTF_LUT_2084_nits[500]);

}

void drvif_fwif_color_Set_ST2094_EOTF(unsigned int sourceMaxL, unsigned int *virAddr, unsigned int phyAddr, VIP_ST2094_CTRL* ST2094_CTRL)
{
	int i;
	long_int tmp1, tmp2;
	//unsigned int norm_div = (ST2094_CTRL->Debug_EOTF_norm_div == 0) ? 100000 : (ST2094_CTRL->Debug_EOTF_norm_div*10);
	unsigned int norm_div = ST2094_usingTBL.EOTF_norm_div;
	unsigned int normalizerNit =  sourceMaxL;
	if( ST2094_usingTBL.product_peak > normalizerNit )
		normalizerNit = ST2094_usingTBL.product_peak;
	if( normalizerNit > 100000 )
		normalizerNit = 100000;

	memcpy(EOTF_curve_ST2094, EOTF_LUT_2084, sizeof(int)*EOTF_size);
	drvif_fwif_color_Set_ST2094_EOTF_LinearBlend(EOTF_curve_ST2094, ST2094_CTRL);

	for( i=0; i<EOTF_size; i++ )
	{
		tmp1 = long_multiply( EOTF_curve_ST2094[i], norm_div );
		tmp2 = long_divide( tmp1, normalizerNit );
		EOTF_curve_ST2094[i] = (tmp2.high > 0) ? 0xFFFFFFFF : tmp2.low;
	}

	//drvif_DM2_EOTF_Enable(1, 1);

	drvif_DM2_EOTF_Set_By_DMA(EOTF_curve_ST2094, EOTF_curve_ST2094, EOTF_curve_ST2094, phyAddr, virAddr);
}

void drvif_fwif_color_Set_ST2094_OETF(ST2094_metadata *metadata, unsigned int sourceMaxL, unsigned int *virAddr, unsigned int phyAddr, VIP_ST2094_CTRL* ST2094_CTRL)
{
	unsigned int P_basis[bezier_curve_anchors_num] = {0};
	unsigned int P_basis_elev[bezier_curve_anchors_num] = {0};
	unsigned int P_elev[bezier_curve_anchors_num] = {0};
	unsigned int P_linear[bezier_curve_anchors_num];
	unsigned int Kx_linear, Ky_linear;
	unsigned int* P = metadata->bezier_curve_anchors;
	unsigned short N = metadata->num_bezier_curve_anchors;
	unsigned short Kx = metadata->knee_point_x;
	unsigned short Ky = metadata->knee_point_y;
	unsigned short Kx_basis;
	unsigned short Ky_basis;
	unsigned short basisOOTF_order;
	unsigned int referencePeak = metadata->targeted_system_display_maximum_luminance * 10;
	//unsigned int productPeak = (ST2094_CTRL->Debug_product_peak > 0) ? ST2094_CTRL->Debug_product_peak : referencePeak - 500;
	unsigned int productPeak;
	unsigned char shiftBit = 0;

	// Algorithm  Parameters
	BasisOOTF_Params params;
	// /Algorithm  Parameters

	long_int interp_tmp;

	unsigned int T0;
	unsigned int T1;
	unsigned int srcL0;
	unsigned int srcL;
	unsigned int kmin;
	unsigned int kmax;
	unsigned int k0;
	unsigned int k1;
	unsigned int g;
	unsigned int ps1;
	int i;

	unsigned char Debug_Log = ST2094_CTRL->Debug_Log;
	unsigned char Debug_log_delay = ST2094_CTRL->Debug_Delay;
	static unsigned char CNT = 0;

	BasisOOTF_Params_setDefault(&params); // sets to default
	CNT++;
	basisOOTF_order = ST2094_usingTBL.basisOOTF_order;

	// coefficients for linear
	for( i=0; i<bezier_curve_anchors_num; i++ )
	{
		P_linear[i] = ( (i+1)<<20 ) / bezier_curve_anchors_num; // 20-bit
	}
	Kx_linear = 1229; // 4096*0.3
	Ky_linear = 1229; // 4096*0.3

	if( N>0 && ST2094_usingTBL.curve_sel == 0 )
	{
		//productPeak = (((int)referencePeak - (int)ST2094_usingTBL.product_peak)<(int)ST2094_usingTBL.product_min)?(ST2094_usingTBL.product_min):(referencePeak - ST2094_usingTBL.product_peak);
		productPeak = (ST2094_usingTBL.product_peak < ST2094_usingTBL.product_min) ? ST2094_usingTBL.product_min : ST2094_usingTBL.product_peak;
		T0 = referencePeak;
		T1 = productPeak;
		srcL0 = max( sourceMaxL, T0 );
		srcL = max( sourceMaxL, T1 );
		kmin = (2000 << 16) / srcL;
		kmax = 65536;
		k0 = (T0<<16) / srcL0;
		k1 = (T1<<16) / srcL;

		if( T0 == T1 )
		{
			//reference peak = product peak: use metadata curve
			drvif_fwif_color_ST2094_bezier_coef_elevation( N, P, P_elev );
		}
		else if( T1 >= sourceMaxL )
		{
			//product peak >= scene max: use linear curve
			Kx = Kx_linear;
			Ky = Ky_linear;
			for( i=0; i<bezier_curve_anchors_num; i++ )
				P_elev[i] = P_linear[i];
		}
		else
		{
			drvif_fwif_color_ST2094_basisOOTF_cal( metadata, ST2094_usingTBL.product_min, sourceMaxL, &Kx_basis, &Ky_basis, P_basis, ST2094_CTRL );

			drvif_fwif_color_ST2094_bezier_coef_elevation( basisOOTF_order, P_basis, P_basis_elev );
			drvif_fwif_color_ST2094_bezier_coef_elevation( N, P, P_elev );

			if (ST2094_CTRL->Debug_basisOOTF_Fixed_KneePoint == 1) {
				Kx_basis = ST2094_CTRL->Debug_basisOOTF_kneePx;
				Ky_basis = ST2094_CTRL->Debug_basisOOTF_kneePy;
			} else {	/* recored for tool*/
				ST2094_CTRL->Debug_basisOOTF_kneePx = Kx_basis;
				ST2094_CTRL->Debug_basisOOTF_kneePy = Ky_basis;
			}
			if (ST2094_CTRL->Debug_basisOOTF_Fixed_Anchors == 1) {
				for(i=0; i<bezier_curve_anchors_num;i++)
					P_basis[i] = ST2094_CTRL->Debug_basisOOTF_P[i];
			} else {/* recored for tool*/
				for(i=0; i<bezier_curve_anchors_num;i++)
					ST2094_CTRL->Debug_basisOOTF_P[i] = P_basis[i];
			}

			if( T1 < T0 )
			{
				// product peak < reference peak: blend with BasisOOTF curve
				g = min( ((k1-kmin)<<16)/(k0-kmin), 65536u);
				Ky = ( g * Ky + (65536-g) * Ky_basis ) >> 16;
				Kx = ( Ky * k1 ) >> 16;

				for( i=0; i<bezier_curve_anchors_num; i++ )
				{
					interp_tmp = long_add( long_multiply( g, P_elev[i]), long_multiply( 65536-g, P_basis_elev[i] ) );
					P_elev[i] = long_shift( interp_tmp, 16 );
				}
			}
			else
			{
				// product peak > reference peak: blend with linear
				g = min( ((k1-k0)<<16)/(kmax-k0), 65536u);
				Ky = ( g * Ky_linear + (65536-g) * Ky ) >> 16;
            			Kx = ( Ky * k1 ) >> 16;

				for( i=0; i<bezier_curve_anchors_num; i++ )
				{
					interp_tmp = long_add( long_multiply( g, P_linear[i]), long_multiply( 65536-g, P_elev[i] ) );
					P_elev[i] = long_shift( interp_tmp, 16 );
				}
			}

			ps1 = calcP1(Kx << 4, Ky << 4, T1, sourceMaxL,
				params.P1_LIMIT_T1, params.P1_LIMIT_T2, params.P1_LIMIT_V1, params.P1_LIMIT_V2,
				params.LOW_SY_T1, params.LOW_SY_T2, params.LOW_K_T1, params.LOW_K_T2,
				params.RED_P1_T1, params.RED_P1_T2, params.RED_P1_V1,
				NULL);

			// Recalculate p1 to form smooth connection with knee-point slope
			P_elev[0] = ps1 << 4; // 20-bit
			for( i=1; i<bezier_curve_anchors_num; i++ )
			{
				P_elev[i] = min( P_elev[i], P_elev[0]*(i+1) );
			}

		}

	}
	else if( ST2094_usingTBL.curve_sel == 1 ) // bezier curve only
		drvif_fwif_color_ST2094_bezier_coef_elevation( N, P, P_elev );
	else // basis curve only
	{
		productPeak = ST2094_usingTBL.basis_peak;
		drvif_fwif_color_ST2094_basisOOTF_cal( metadata, productPeak, sourceMaxL, &Kx_basis, &Ky_basis, P_basis, ST2094_CTRL );

		if (ST2094_CTRL->Debug_basisOOTF_Fixed_KneePoint == 1) {
			Kx_basis = ST2094_CTRL->Debug_basisOOTF_kneePx;
			Ky_basis = ST2094_CTRL->Debug_basisOOTF_kneePy;
		} else {	/* recored for tool*/
			ST2094_CTRL->Debug_basisOOTF_kneePx = Kx_basis;
			ST2094_CTRL->Debug_basisOOTF_kneePy = Ky_basis;
		}
		if (ST2094_CTRL->Debug_basisOOTF_Fixed_Anchors == 1) {
			for(i=0; i<bezier_curve_anchors_num;i++)
				P_basis[i] = ST2094_CTRL->Debug_basisOOTF_P[i];
		} else {/* recored for tool*/
			for(i=0; i<bezier_curve_anchors_num;i++)
				ST2094_CTRL->Debug_basisOOTF_P[i] = P_basis[i];
		}

		Kx = Kx_basis;
		Ky = Ky_basis;
		drvif_fwif_color_ST2094_bezier_coef_elevation( basisOOTF_order, P_basis, P_elev );
	}

	// data record for GuidedOOTF info
	ST2094_CTRL->Debug_GuidedOOTF_kneePx = Kx;
	ST2094_CTRL->Debug_GuidedOOTF_kneePy = Ky;
	for(i=0; i<bezier_curve_anchors_num;i++)
		ST2094_CTRL->Debug_GuidedOOTF_P[i] = P_elev[i];

	drvif_fwif_color_ST2094_bezier_curve_cal( P_elev, bezier_curve );
	drvif_fwif_color_ST2094_bezier_knee_cal( bezier_curve, Kx, Ky, knee_curve_ST2094 );
#if 0
	if( ST2094_usingTBL.EOTF_Gamma22 == 1 )
	{
		//drvif_fwif_color_ST2094_OETF_Gamma22(knee_curve_ST2094, OETF_curve_ST2094);
		for(i=0; i<OETF_size; i++)
		{
			OETF_curve_ST2094[i] = knee_curve_ST2094[i] >>4;
			//*gain = 1024;
		}
	}
	else
#endif
		drvif_fwif_color_ST2094_ratio_curve_cal( knee_curve_ST2094, Kx, Ky, OETF_curve_ST2094, &shiftBit );

	//drvif_DM2_OETF_Enable(1, 1);

	drvif_DM2_OETF_Set_By_DMA(OETF_curve_ST2094, OETF_curve_ST2094, OETF_curve_ST2094, phyAddr, virAddr);

	/*
	if( OETF_curve_ST2094[0] > OETF_curve_ST2094[1024] ) // Assuming decreasing curve
	{
		// Merlin3 ver. B fix: B806B7E0[4] = 1
		drvif_color_Set_HDR_OETF_Decrease( 1 );
	}
	else
	{
		drvif_color_Set_HDR_OETF_Decrease( 0 );
	}
	*/

	// BBC gain on
	//if( ST2094_usingTBL.EOTF_Gamma22 == 0 )
	//	drvif_color_set_BBC_Enable(2);
	//else
	//	drvif_color_set_BBC_Enable(0);

	drvif_color_set_BBC_shift_bit(shiftBit);

	if((Debug_Log&_BIT4)!=0) {
		if (CNT >= Debug_log_delay) {
			CNT = 0;
			rtd_pr_vpq_info("[HDR10+ OETF] Ref. peak=%d, Product peak=%d \n", referencePeak, productPeak );
			rtd_pr_vpq_info("[HDR10+ OETF] New knee point x=%d, y=%d \n", Kx, Ky );
			rtd_pr_vpq_info("[HDR10+ OETF] P[0~4] = %d, %d, %d, %d, %d\n", P_elev[0], P_elev[1], P_elev[2], P_elev[3], P_elev[4] );
			rtd_pr_vpq_info("[HDR10+ OETF] P[5~9] = %d, %d, %d, %d, %d\n", P_elev[5], P_elev[6], P_elev[7], P_elev[8], P_elev[9] );
			//rtd_pr_vpq_info("[HDR10+ OETF] P[10~13] = %d, %d, %d, %d\n", P_elev[10], P_elev[11], P_elev[12], P_elev[13] );
		}
	}

}

void drvif_fwif_color_ScalerVIP_set_ST2094_metadata2reg_frameSync(int display_Brightness, _RPC_system_setting_info *RPC_sys_info, VIP_ST2094_CTRL* ST2094_CTRL)
{
	vfe_hdmi_vsi_t vsif_info_frame = {0};
	vfe_hdmi_vsi_t *vsif_packet = NULL;
	unsigned char vsif_ret;
	ST2094_metadata *ST2094_metadata;
	unsigned char Debug_Log, Debug_log_delay;
	unsigned int* virAddr_EOTF = NULL;
	unsigned int phyAddr_EOTF = 0;
	unsigned int* virAddr_OETF = NULL;
	unsigned int phyAddr_OETF = 0;
	static unsigned char CNT = 0;
	unsigned char percentile99_idx = 0;
	unsigned int sourceMaxL = 0;

	CNT++;

	phyAddr_EOTF = ST2094_CTRL->EOTF_DMA.phy_addr_align;
	virAddr_EOTF = ST2094_CTRL->EOTF_DMA.pVir_addr_align;
	phyAddr_OETF = ST2094_CTRL->OETF_DMA.phy_addr_align;
	virAddr_OETF = ST2094_CTRL->OETF_DMA.pVir_addr_align;

	Debug_Log = ST2094_CTRL->Debug_Log;
	Debug_log_delay = ST2094_CTRL->Debug_Delay;

	ST2094_metadata = drvif_fwif_color_Get_ST2094_METADATA_Buff();

	if (drvif_Get_ST2094_EFuse_Status() == 1) {	// no hdr10+
		memset(ST2094_metadata, 0 , sizeof(*ST2094_metadata));
		return;
	}

	if (1) {
		if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI) {
			vsif_ret = (unsigned char)drvif_Hdmi_GetHDR10pVsInfoFrame(&vsif_info_frame);
			if (vsif_ret == 0) {
				rtd_pr_vpq_emerg("ST2094, get VSIF fail from HDMI \n");
				return;
			}
			vsif_packet = &vsif_info_frame;
			// set metadata to RPC system info from HDMI VSIF
			ST2094_metadata->application_version						= ( vsif_packet->payload[0] & 0xC0 ) >> 6;
			ST2094_metadata->targeted_system_display_maximum_luminance	= (( vsif_packet->payload[0] & 0x3E ) >> 1 ) * 32;
			ST2094_metadata->average_maxrgb							= vsif_packet->payload[1] * 160;
			ST2094_metadata->maxscl[0] 								= vsif_packet->payload[10] * 160; // use 99% percentile as maxscl
			ST2094_metadata->maxscl[1] 								= vsif_packet->payload[10] * 160;
			ST2094_metadata->maxscl[2] 								= vsif_packet->payload[10] * 160;
			ST2094_metadata->num_distribution_maxrgb_percentiles			= 9;
			ST2094_metadata->distribution_maxrgb_percentages[0]			= 1;
			ST2094_metadata->distribution_maxrgb_percentages[1]			= 5;
			ST2094_metadata->distribution_maxrgb_percentages[2]			= 10;
			ST2094_metadata->distribution_maxrgb_percentages[3]			= 25;
			ST2094_metadata->distribution_maxrgb_percentages[4]			= 50;
			ST2094_metadata->distribution_maxrgb_percentages[5]			= 75;
			ST2094_metadata->distribution_maxrgb_percentages[6]			= 90;
			ST2094_metadata->distribution_maxrgb_percentages[7]			= 95;
			ST2094_metadata->distribution_maxrgb_percentages[8]			= 99;
			ST2094_metadata->distribution_maxrgb_percentiles[0]			= vsif_packet->payload[2] * 160;
			ST2094_metadata->distribution_maxrgb_percentiles[1]			= vsif_packet->payload[3] * 160;
			ST2094_metadata->distribution_maxrgb_percentiles[2]			= vsif_packet->payload[4];
			ST2094_metadata->distribution_maxrgb_percentiles[3]			= vsif_packet->payload[5] * 160;
			ST2094_metadata->distribution_maxrgb_percentiles[4]			= vsif_packet->payload[6] * 160;
			ST2094_metadata->distribution_maxrgb_percentiles[5]			= vsif_packet->payload[7] * 160;
			ST2094_metadata->distribution_maxrgb_percentiles[6]			= vsif_packet->payload[8] * 160;
			ST2094_metadata->distribution_maxrgb_percentiles[7]			= vsif_packet->payload[9] * 160;
			ST2094_metadata->distribution_maxrgb_percentiles[8]			= vsif_packet->payload[10] * 160;
			ST2094_metadata->num_bezier_curve_anchors					= (vsif_packet->payload[11] & 0xF0) >> 4;
			ST2094_metadata->tone_mapping_flag						= ST2094_metadata->num_bezier_curve_anchors > 0 ? 1 : 0;
			ST2094_metadata->knee_point_x								= ( ( ((unsigned short)vsif_packet->payload[12] & 0x0F) << 6 ) + ( (vsif_packet->payload[13] & 0xFC) >> 2 ) ) * 4;
			ST2094_metadata->knee_point_y								= ( ( ((unsigned short)vsif_packet->payload[13] & 0x03) << 8 ) + vsif_packet->payload[14] ) * 4;
			ST2094_metadata->bezier_curve_anchors[0]					= vsif_packet->payload[15] * 4;
			ST2094_metadata->bezier_curve_anchors[1]					= vsif_packet->payload[16] * 4;
			ST2094_metadata->bezier_curve_anchors[2]					= vsif_packet->payload[17] * 4;
			ST2094_metadata->bezier_curve_anchors[3]					= vsif_packet->payload[18] * 4;
			ST2094_metadata->bezier_curve_anchors[4]					= vsif_packet->payload[19] * 4;
			ST2094_metadata->bezier_curve_anchors[5]					= vsif_packet->payload[20] * 4;
			ST2094_metadata->bezier_curve_anchors[6]					= vsif_packet->payload[21] * 4;
			ST2094_metadata->bezier_curve_anchors[7]					= vsif_packet->payload[22] * 4;
			ST2094_metadata->bezier_curve_anchors[8]					= vsif_packet->payload[23] * 4;
			ST2094_metadata->bezier_curve_anchors[9]					= 0;

		} else {
			ST2094_metadata->application_version							=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.application_version						, 1);
			ST2094_metadata->targeted_system_display_maximum_luminance 	=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.targeted_system_display_maximum_luminance , 1);
			ST2094_metadata->average_maxrgb								=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.average_maxrgb							, 1);
			ST2094_metadata->maxscl[0] 									=	fwif_color_ChangeOneUINT16Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.maxscl[0] 								, 1);
			ST2094_metadata->maxscl[1] 									=	fwif_color_ChangeOneUINT16Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.maxscl[1] 								, 1);
			ST2094_metadata->maxscl[2] 									=	fwif_color_ChangeOneUINT16Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.maxscl[2] 								, 1);
			ST2094_metadata->num_distribution_maxrgb_percentiles			=	fwif_color_ChangeOneUINT16Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.num_distribution_maxrgb_percentiles		, 1);
			ST2094_metadata->distribution_maxrgb_percentages[0]			=	fwif_color_ChangeOneUINT16Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.distribution_maxrgb_percentages[0]		, 1);
			ST2094_metadata->distribution_maxrgb_percentages[1]			=	fwif_color_ChangeOneUINT16Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.distribution_maxrgb_percentages[1]		, 1);
			ST2094_metadata->distribution_maxrgb_percentages[2]			=	fwif_color_ChangeOneUINT16Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.distribution_maxrgb_percentages[2]		, 1);
			ST2094_metadata->distribution_maxrgb_percentages[3]			=	fwif_color_ChangeOneUINT16Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.distribution_maxrgb_percentages[3]		, 1);
			ST2094_metadata->distribution_maxrgb_percentages[4]			=	fwif_color_ChangeOneUINT16Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.distribution_maxrgb_percentages[4]		, 1);
			ST2094_metadata->distribution_maxrgb_percentages[5]			=	fwif_color_ChangeOneUINT16Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.distribution_maxrgb_percentages[5]		, 1);
			ST2094_metadata->distribution_maxrgb_percentages[6]			=	fwif_color_ChangeOneUINT16Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.distribution_maxrgb_percentages[6]		, 1);
			ST2094_metadata->distribution_maxrgb_percentages[7]			=	fwif_color_ChangeOneUINT16Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.distribution_maxrgb_percentages[7]		, 1);
			ST2094_metadata->distribution_maxrgb_percentages[8]			=	fwif_color_ChangeOneUINT16Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.distribution_maxrgb_percentages[8]		, 1);
			ST2094_metadata->distribution_maxrgb_percentages[9]			=	fwif_color_ChangeOneUINT16Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.distribution_maxrgb_percentages[9]		, 1);
			ST2094_metadata->distribution_maxrgb_percentiles[0]			=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.distribution_maxrgb_percentiles[0]		, 1);
			ST2094_metadata->distribution_maxrgb_percentiles[1]			=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.distribution_maxrgb_percentiles[1]		, 1);
			ST2094_metadata->distribution_maxrgb_percentiles[2]			=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.distribution_maxrgb_percentiles[2]		, 1);
			ST2094_metadata->distribution_maxrgb_percentiles[3]			=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.distribution_maxrgb_percentiles[3]		, 1);
			ST2094_metadata->distribution_maxrgb_percentiles[4]			=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.distribution_maxrgb_percentiles[4]		, 1);
			ST2094_metadata->distribution_maxrgb_percentiles[5]			=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.distribution_maxrgb_percentiles[5]		, 1);
			ST2094_metadata->distribution_maxrgb_percentiles[6]			=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.distribution_maxrgb_percentiles[6]		, 1);
			ST2094_metadata->distribution_maxrgb_percentiles[7]			=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.distribution_maxrgb_percentiles[7]		, 1);
			ST2094_metadata->distribution_maxrgb_percentiles[8]			=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.distribution_maxrgb_percentiles[8]		, 1);
			ST2094_metadata->distribution_maxrgb_percentiles[9]			=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.distribution_maxrgb_percentiles[9]		, 1);
			ST2094_metadata->tone_mapping_flag							=	RPC_sys_info->HDR_info.hdr_ST2094_metadata.tone_mapping_flag																;
			ST2094_metadata->knee_point_x 								=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.knee_point_x 								, 1);
			ST2094_metadata->knee_point_y 								=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.knee_point_y 								, 1);
			ST2094_metadata->num_bezier_curve_anchors					=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.num_bezier_curve_anchors					, 1);
			ST2094_metadata->bezier_curve_anchors[0] 					=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.bezier_curve_anchors[0] 					, 1);
			ST2094_metadata->bezier_curve_anchors[1] 					=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.bezier_curve_anchors[1] 					, 1);
			ST2094_metadata->bezier_curve_anchors[2] 					=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.bezier_curve_anchors[2] 					, 1);
			ST2094_metadata->bezier_curve_anchors[3] 					=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.bezier_curve_anchors[3] 					, 1);
			ST2094_metadata->bezier_curve_anchors[4] 					=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.bezier_curve_anchors[4] 					, 1);
			ST2094_metadata->bezier_curve_anchors[5] 					=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.bezier_curve_anchors[5] 					, 1);
			ST2094_metadata->bezier_curve_anchors[6] 					=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.bezier_curve_anchors[6] 					, 1);
			ST2094_metadata->bezier_curve_anchors[7] 					=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.bezier_curve_anchors[7] 					, 1);
			ST2094_metadata->bezier_curve_anchors[8] 					=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.bezier_curve_anchors[8] 					, 1);
			ST2094_metadata->bezier_curve_anchors[9] 					=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.bezier_curve_anchors[9] 					, 1);
			//ST2094_metadata->bezier_curve_anchors[10]					=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.bezier_curve_anchors[10]					, 1);
			//ST2094_metadata->bezier_curve_anchors[11]					=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.bezier_curve_anchors[11]					, 1);
			//ST2094_metadata->bezier_curve_anchors[12]					=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.bezier_curve_anchors[12]					, 1);
			//ST2094_metadata->bezier_curve_anchors[13]					=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.bezier_curve_anchors[13]					, 1);
			//ST2094_metadata->bezier_curve_anchors[14]					=	fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.hdr_ST2094_metadata.bezier_curve_anchors[14]					, 1);
		}
	}
	else
	{
		// preset values for testing
		ST2094_metadata->application_version = 0;
		ST2094_metadata->targeted_system_display_maximum_luminance = 400;
		ST2094_metadata->average_maxrgb = 1052;
		ST2094_metadata->maxscl[0] = 8002;
		ST2094_metadata->maxscl[1] = 8370;
		ST2094_metadata->maxscl[2] = 7120;
		ST2094_metadata->num_distribution_maxrgb_percentiles = 10;
		ST2094_metadata->distribution_maxrgb_percentages[0] = 1;
		ST2094_metadata->distribution_maxrgb_percentages[1] = 5;
		ST2094_metadata->distribution_maxrgb_percentages[2] = 10;
		ST2094_metadata->distribution_maxrgb_percentages[3] = 25;
		ST2094_metadata->distribution_maxrgb_percentages[4] = 50;
		ST2094_metadata->distribution_maxrgb_percentages[5] = 75;
		ST2094_metadata->distribution_maxrgb_percentages[6] = 90;
		ST2094_metadata->distribution_maxrgb_percentages[7] = 95;
		ST2094_metadata->distribution_maxrgb_percentages[8] = 98;
		ST2094_metadata->distribution_maxrgb_percentages[9] = 99;
		ST2094_metadata->distribution_maxrgb_percentiles[0] = 1;
		ST2094_metadata->distribution_maxrgb_percentiles[1] = 7;
		ST2094_metadata->distribution_maxrgb_percentiles[2] = 15;
		ST2094_metadata->distribution_maxrgb_percentiles[3] = 45;
		ST2094_metadata->distribution_maxrgb_percentiles[4] = 136;
		ST2094_metadata->distribution_maxrgb_percentiles[5] = 646;
		ST2094_metadata->distribution_maxrgb_percentiles[6] = 4919;
		ST2094_metadata->distribution_maxrgb_percentiles[7] = 7120;
		ST2094_metadata->distribution_maxrgb_percentiles[8] = 7185;
		ST2094_metadata->distribution_maxrgb_percentiles[9] = 7381;
		ST2094_metadata->tone_mapping_flag = 1;
		ST2094_metadata->knee_point_x = 572;
		ST2094_metadata->knee_point_y = 1056;
		ST2094_metadata->num_bezier_curve_anchors = 3;
		ST2094_metadata->bezier_curve_anchors[0] = 506;
		ST2094_metadata->bezier_curve_anchors[1] = 694;
		ST2094_metadata->bezier_curve_anchors[2] = 701;
		ST2094_metadata->bezier_curve_anchors[3] = 0;
		ST2094_metadata->bezier_curve_anchors[4] = 0;
		ST2094_metadata->bezier_curve_anchors[5] = 0;
		ST2094_metadata->bezier_curve_anchors[6] = 0;
		ST2094_metadata->bezier_curve_anchors[7] = 0;
		ST2094_metadata->bezier_curve_anchors[8] = 0;
		ST2094_metadata->bezier_curve_anchors[9] = 0;
		//ST2094_metadata->bezier_curve_anchors[10] = 0;
		//ST2094_metadata->bezier_curve_anchors[11] = 0;
		//ST2094_metadata->bezier_curve_anchors[12] = 0;
		//ST2094_metadata->bezier_curve_anchors[13] = 0;
		//ST2094_metadata->bezier_curve_anchors[14] = 0;

	}

	//sourceMaxL = max( (unsigned short)ST2094_metadata->maxscl[0], (unsigned short)max( (unsigned short)ST2094_metadata->maxscl[1], (unsigned short)ST2094_metadata->maxscl[2] ) ); // Max of MaxSCL
	if (ST2094_metadata->num_distribution_maxrgb_percentiles <= 0 || ST2094_metadata->num_distribution_maxrgb_percentiles > distribution_maxrgb_bin_num)
		percentile99_idx = distribution_maxrgb_bin_num - 1;
	else
		percentile99_idx = ST2094_metadata->num_distribution_maxrgb_percentiles - 1;
	sourceMaxL = ST2094_metadata->distribution_maxrgb_percentiles[percentile99_idx]; // 99% percentile

	// EOTF: ST2084 with normalization
	drvif_fwif_color_Set_ST2094_EOTF(sourceMaxL, virAddr_EOTF, phyAddr_EOTF, ST2094_CTRL);

	// OETF: bezier curve with knee, translated to ratio
	drvif_fwif_color_Set_ST2094_OETF(ST2094_metadata, sourceMaxL, virAddr_OETF, phyAddr_OETF, ST2094_CTRL );

	// Shift bit compensation done on 3DLUT in Merlin3 & Mac6, will change to BBC_shift in future IC
	//if(drvif_fwif_color_Get_ST2094_TBL()->EOTF_Gamma22 == 0)
	//	drvif_color_Set_ST2094_3Dlut_CInv(gain, 0);

#if 1	/** for debug */
	if((Debug_Log&_BIT1)!=0) {
		if (CNT >= Debug_log_delay) {
			CNT = 0;
			rtd_pr_vpq_info("ST2094_enable=%d, application_version=%d, targeted_system_display_maximum_luminance=%d, average_maxrgb=%d \n",
				ST2094_CTRL->Enable, ST2094_metadata->application_version, ST2094_metadata->targeted_system_display_maximum_luminance, ST2094_metadata->average_maxrgb);

			rtd_pr_vpq_info("maxscl[0]=%d, maxscl[1]=%d, maxscl[2]=%d, num_distribution_maxrgb_percentiles=%d \n",
				ST2094_metadata->maxscl[0], ST2094_metadata->maxscl[1], ST2094_metadata->maxscl[2], ST2094_metadata->num_distribution_maxrgb_percentiles);

			rtd_pr_vpq_info("distribution_maxrgb_percentages[0]~[4]=%d,%d,%d,%d,%d \n",
				ST2094_metadata->distribution_maxrgb_percentages[0], ST2094_metadata->distribution_maxrgb_percentages[1], ST2094_metadata->distribution_maxrgb_percentages[2],
				ST2094_metadata->distribution_maxrgb_percentages[3], ST2094_metadata->distribution_maxrgb_percentages[4]);

			rtd_pr_vpq_info("distribution_maxrgb_percentages[5]~[9]=%d,%d,%d,%d,%d \n",
				ST2094_metadata->distribution_maxrgb_percentages[5], ST2094_metadata->distribution_maxrgb_percentages[6], ST2094_metadata->distribution_maxrgb_percentages[7],
				ST2094_metadata->distribution_maxrgb_percentages[8], ST2094_metadata->distribution_maxrgb_percentages[9]);

			rtd_pr_vpq_info("distribution_maxrgb_percentiles[0]~[4]=%d,%d,%d,%d,%d \n",
				ST2094_metadata->distribution_maxrgb_percentiles[0], ST2094_metadata->distribution_maxrgb_percentiles[1], ST2094_metadata->distribution_maxrgb_percentiles[2],
				ST2094_metadata->distribution_maxrgb_percentiles[3], ST2094_metadata->distribution_maxrgb_percentiles[4]);

			rtd_pr_vpq_info("distribution_maxrgb_percentiles[5]~[9]=%d,%d,%d,%d,%d \n",
				ST2094_metadata->distribution_maxrgb_percentiles[5], ST2094_metadata->distribution_maxrgb_percentiles[6], ST2094_metadata->distribution_maxrgb_percentiles[7],
				ST2094_metadata->distribution_maxrgb_percentiles[8], ST2094_metadata->distribution_maxrgb_percentiles[9]);

			rtd_pr_vpq_info("tone_mapping_flag=%d, knee_point_x=%d, knee_point_y=%d, num_bezier_curve_anchors=%d \n",
				ST2094_metadata->tone_mapping_flag, ST2094_metadata->knee_point_x, ST2094_metadata->knee_point_y, ST2094_metadata->num_bezier_curve_anchors);

			rtd_pr_vpq_info("bezier_curve_anchors[0]~[4]=%d,%d,%d,%d,%d \n",
				ST2094_metadata->bezier_curve_anchors[0], ST2094_metadata->bezier_curve_anchors[1], ST2094_metadata->bezier_curve_anchors[2],
				ST2094_metadata->bezier_curve_anchors[3], ST2094_metadata->bezier_curve_anchors[4]);

			rtd_pr_vpq_info("bezier_curve_anchors[5]~[9]=%d,%d,%d,,%d,%d \n",
				ST2094_metadata->bezier_curve_anchors[5], ST2094_metadata->bezier_curve_anchors[6], ST2094_metadata->bezier_curve_anchors[7],
				ST2094_metadata->bezier_curve_anchors[8], ST2094_metadata->bezier_curve_anchors[9]);

			//rtd_pr_vpq_info("bezier_curve_anchors[12]~[14]=%d,%d,%d,%d,%d \n",
				//ST2094_metadata->bezier_curve_anchors[10], ST2094_metadata->bezier_curve_anchors[11], ST2094_metadata->bezier_curve_anchors[12],
				//ST2094_metadata->bezier_curve_anchors[13], ST2094_metadata->bezier_curve_anchors[14]);

		}
	}
#endif

}

static ST2094_metadata ST2094_metadata_buff;
ST2094_metadata* drvif_fwif_color_Get_ST2094_METADATA_Buff(void)
{
	return &ST2094_metadata_buff;
}

static VIP_ST2094_CTRL ST2094_CTRL = {0};
VIP_ST2094_CTRL* drvif_fwif_color_Get_ST2094_Ctrl(void)
{
	return &ST2094_CTRL;
}

VIP_ST2094_TBL* drvif_fwif_color_Get_ST2094_TBL(void)
{
	return &ST2094_usingTBL;
}

//====================20171127 END =========================================================
static int ST2094_Demo_flag[VIP_ST2094_layer_MAX] = {ST2094_Demo_OFF};
int drvif_fwif_color_Get_ST2094_demo_flag(unsigned char layer)
{
	return ST2094_Demo_flag[layer];
}

void drvif_fwif_color_Set_ST2094_demo_flag(int flag, unsigned char layer)
{
	ST2094_Demo_flag[layer] = flag;
}

void drvif_color_Set_HDR_OETF_Decrease( unsigned char dec )
{
	// Merlin3 ver. B, set dummy bit to enable decreasing curve
	if( dec )
		IoReg_SetBits( DM_DM_DUMMY_reg, _BIT4 );
	else
		IoReg_ClearBits( DM_DM_DUMMY_reg, _BIT4 );
}

void drvif_color_Set_ST2094_3Dlut_CInv(unsigned int gain, unsigned char dim, unsigned short precision)
{
	dm_threedlut_minmaxinv1_RBUS threedlut_minmaxinv1_reg;
	dm_threedlut_minmaxinv2_RBUS threedlut_minmaxinv2_reg;
	dm_threedlut_minmaxinv3_RBUS threedlut_minmaxinv3_reg;
	dm_hdr_double_buffer_ctrl_RBUS dm_double_buffer_ctrl_reg;

	threedlut_minmaxinv1_reg.regValue = IoReg_Read32(DM_ThreeDLUT_MinMaxInv1_reg);
	threedlut_minmaxinv2_reg.regValue = IoReg_Read32(DM_ThreeDLUT_MinMaxInv2_reg);
	threedlut_minmaxinv3_reg.regValue = IoReg_Read32(DM_ThreeDLUT_MinMaxInv3_reg);
#if 0
	if( dim == 1 ) // 24x24x24 LUT
	{
		threedlut_minmaxinv1_reg.idistc1inv = 0x0000b80b << shift_bit;
		threedlut_minmaxinv2_reg.idistc2inv = 0x0000b80b << shift_bit;
		threedlut_minmaxinv3_reg.idistc3inv = 0x0000b80b << shift_bit;
	}
	else // 17x17x17 LUT
	{
		threedlut_minmaxinv1_reg.idistc1inv = 0x00008000 << shift_bit;
		threedlut_minmaxinv2_reg.idistc2inv = 0x00008000 << shift_bit;
		threedlut_minmaxinv3_reg.idistc3inv = 0x00008000 << shift_bit;
	}
#else
	if( dim == 1 ) // 24x24x24 LUT
	{
		threedlut_minmaxinv1_reg.idistc1inv = (0x00005c05 * gain) >> precision;
		threedlut_minmaxinv2_reg.idistc2inv = (0x00005c05 * gain) >> precision;
		threedlut_minmaxinv3_reg.idistc3inv = (0x00005c05 * gain) >> precision;
	}
	else // 17x17x17 LUT
	{
		threedlut_minmaxinv1_reg.idistc1inv = (0x00004000 * gain) >> precision;
		threedlut_minmaxinv2_reg.idistc2inv = (0x00004000 * gain) >> precision;
		threedlut_minmaxinv3_reg.idistc3inv = (0x00004000 * gain) >> precision;
	}

#endif
	IoReg_Write32(DM_ThreeDLUT_MinMaxInv1_reg, threedlut_minmaxinv1_reg.regValue);
	IoReg_Write32(DM_ThreeDLUT_MinMaxInv2_reg, threedlut_minmaxinv2_reg.regValue);
	IoReg_Write32(DM_ThreeDLUT_MinMaxInv3_reg, threedlut_minmaxinv3_reg.regValue);

	//apply
	dm_double_buffer_ctrl_reg.regValue = IoReg_Read32(DM_HDR_Double_Buffer_CTRL_reg);
	dm_double_buffer_ctrl_reg.dm_db_apply = 1;
	IoReg_Write32(DM_HDR_Double_Buffer_CTRL_reg, dm_double_buffer_ctrl_reg.regValue);
}


void drvif_color_set_HDR_OETF_Lmix(unsigned char enable, unsigned short* alpha_th_table)
{
 dm_hdr_oetf_lmix_alpha_th0_RBUS oetf_lmix_alpha_th0_reg;
 dm_hdr_oetf_lmix_alpha_th1_RBUS oetf_lmix_alpha_th1_reg;
 dm_hdr_oetf_lmix_alpha_th2_RBUS oetf_lmix_alpha_th2_reg;
 dm_hdr_oetf_lmix_alpha_th3_RBUS oetf_lmix_alpha_th3_reg;
 dm_hdr_oetf_lmix_alpha_th4_RBUS oetf_lmix_alpha_th4_reg;
 dm_hdr_oetf_lmix_alpha_th5_RBUS oetf_lmix_alpha_th5_reg;
 dm_hdr_oetf_lmix_alpha_th6_RBUS oetf_lmix_alpha_th6_reg;
 dm_hdr_oetf_lmix_alpha_th7_RBUS oetf_lmix_alpha_th7_reg;
 dm_hdr_double_buffer_ctrl_RBUS dm_double_buffer_ctrl_reg;
 
 oetf_lmix_alpha_th0_reg.regValue = IoReg_Read32(DM_HDR_OETF_LMIX_ALPHA_TH0_reg);
 oetf_lmix_alpha_th1_reg.regValue = IoReg_Read32(DM_HDR_OETF_LMIX_ALPHA_TH1_reg);
 oetf_lmix_alpha_th2_reg.regValue = IoReg_Read32(DM_HDR_OETF_LMIX_ALPHA_TH2_reg);
 oetf_lmix_alpha_th3_reg.regValue = IoReg_Read32(DM_HDR_OETF_LMIX_ALPHA_TH3_reg);
 oetf_lmix_alpha_th4_reg.regValue = IoReg_Read32(DM_HDR_OETF_LMIX_ALPHA_TH4_reg);
 oetf_lmix_alpha_th5_reg.regValue = IoReg_Read32(DM_HDR_OETF_LMIX_ALPHA_TH5_reg);
 oetf_lmix_alpha_th6_reg.regValue = IoReg_Read32(DM_HDR_OETF_LMIX_ALPHA_TH6_reg);
 oetf_lmix_alpha_th7_reg.regValue = IoReg_Read32(DM_HDR_OETF_LMIX_ALPHA_TH7_reg);
 
 oetf_lmix_alpha_th0_reg.hdr_oetf_lmix_en = enable;
 oetf_lmix_alpha_th0_reg.hdr_oetf_lmix_alpha_th0 = alpha_th_table[0];
 oetf_lmix_alpha_th0_reg.hdr_oetf_lmix_alpha_th1 = alpha_th_table[1];
 oetf_lmix_alpha_th1_reg.hdr_oetf_lmix_alpha_th2 = alpha_th_table[2];
 oetf_lmix_alpha_th1_reg.hdr_oetf_lmix_alpha_th3 = alpha_th_table[3];
 oetf_lmix_alpha_th2_reg.hdr_oetf_lmix_alpha_th4 = alpha_th_table[4];
 oetf_lmix_alpha_th2_reg.hdr_oetf_lmix_alpha_th5 = alpha_th_table[5];
 oetf_lmix_alpha_th3_reg.hdr_oetf_lmix_alpha_th6 = alpha_th_table[6];
 oetf_lmix_alpha_th3_reg.hdr_oetf_lmix_alpha_th7 = alpha_th_table[7];
 oetf_lmix_alpha_th4_reg.hdr_oetf_lmix_alpha_th8 = alpha_th_table[8];
 oetf_lmix_alpha_th4_reg.hdr_oetf_lmix_alpha_th9 = alpha_th_table[9];
 oetf_lmix_alpha_th5_reg.hdr_oetf_lmix_alpha_th10 = alpha_th_table[10];
 oetf_lmix_alpha_th5_reg.hdr_oetf_lmix_alpha_th11 = alpha_th_table[11];
 oetf_lmix_alpha_th6_reg.hdr_oetf_lmix_alpha_th12 = alpha_th_table[12];
 oetf_lmix_alpha_th6_reg.hdr_oetf_lmix_alpha_th13 = alpha_th_table[13];
 oetf_lmix_alpha_th7_reg.hdr_oetf_lmix_alpha_th14 = alpha_th_table[14];
 oetf_lmix_alpha_th7_reg.hdr_oetf_lmix_alpha_th15 = alpha_th_table[15];
 
 IoReg_Write32(DM_HDR_OETF_LMIX_ALPHA_TH0_reg, oetf_lmix_alpha_th0_reg.regValue);
 IoReg_Write32(DM_HDR_OETF_LMIX_ALPHA_TH1_reg, oetf_lmix_alpha_th1_reg.regValue);
 IoReg_Write32(DM_HDR_OETF_LMIX_ALPHA_TH2_reg, oetf_lmix_alpha_th2_reg.regValue);
 IoReg_Write32(DM_HDR_OETF_LMIX_ALPHA_TH3_reg, oetf_lmix_alpha_th3_reg.regValue);
 IoReg_Write32(DM_HDR_OETF_LMIX_ALPHA_TH4_reg, oetf_lmix_alpha_th4_reg.regValue);
 IoReg_Write32(DM_HDR_OETF_LMIX_ALPHA_TH5_reg, oetf_lmix_alpha_th5_reg.regValue);
 IoReg_Write32(DM_HDR_OETF_LMIX_ALPHA_TH6_reg, oetf_lmix_alpha_th6_reg.regValue);
 IoReg_Write32(DM_HDR_OETF_LMIX_ALPHA_TH7_reg, oetf_lmix_alpha_th7_reg.regValue);
 
 //apply
 dm_double_buffer_ctrl_reg.regValue = IoReg_Read32(DM_HDR_Double_Buffer_CTRL_reg);
 dm_double_buffer_ctrl_reg.dm_db_apply = 1;
 IoReg_Write32(DM_HDR_Double_Buffer_CTRL_reg, dm_double_buffer_ctrl_reg.regValue);
}

/*========================== ST2094==========================================*/
#endif


