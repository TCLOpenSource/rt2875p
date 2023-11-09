#include "memc_isr/PQL/PQLPlatformDefs.h"
#include "memc_isr/PQL/PQLContext.h"
#include "memc_isr/PQL/PQLGlobalDefs.h"
#include "memc_isr/PQL/ME_VstCtrl.h"
#include "memc_reg_def.h"

#define ME_VST_DEBUG_PRINT 0

#if RTK_MEMC_Performance_tunging_from_tv001
#ifndef Min
#define Min(a, b)   ( ((a) < (b)) ? (a) : (b) )
#endif
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif
#endif

// interal histogram info
static unsigned short u12_me1_statis_bv_x[SEC_NUM][X_NUM]; // neg, zero, pos
static unsigned short u12_me1_statis_bv_y[SEC_NUM][Y_NUM]; // -8~+8
static unsigned int u32_rgnTotalBlks[4] ;   //  4 section  default 60 *135
static unsigned int u32_rgnTotalStatBlks_x[4] ; // Statistic one section total mvx
static unsigned int u32_rgnTotalStatBlks_y[4] ; // Statistic one section total mvy

VOID ME_Vst_ReadHistogram(VOID);
VOID ME_Vst_Statistic(VOID);
VOID ME_Vst_Detection(_OUTPUT_ME_VST_CTRL *pOutput);
VOID ME_Vst_blackBG_Detection(_OUTPUT_ME_VST_CTRL *pOutput);

VOID ME_Vst_Init(_OUTPUT_ME_VST_CTRL *pOutput)
{
	pOutput->u1_detect = 0;
	pOutput->u1_detect_blackBG_VST_trure= 0;
	pOutput->u8_boundarySta = 0;
	pOutput->u8_boundaryEnd = 0;
}
VOID ME_Vst_Proc(const _PARAM_ME_VST_CTRL *pParam, _OUTPUT_ME_VST_CTRL *pOutput)
{
	ME_Vst_ReadHistogram();
	ME_Vst_Statistic();
	ME_Vst_blackBG_Detection(pOutput);
	ME_Vst_Detection(pOutput);
}

VOID ME_Vst_ReadHistogram()
{
	unsigned int u32_i = 0, u32_j = 0;
	unsigned int u32_RB_val1 = 0, u32_RB_val2 = 0, u32_RB_val3 = 0;
	static unsigned int u32_print_counter = 0;

	u32_print_counter++;
	if(u32_print_counter >= 60)
		u32_print_counter = 0;

	ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_CTRL_0_reg, 0, 31, &u32_RB_val1);

	// update done
	if( ((u32_RB_val1 >> 24) & 0x1) !=0 )
	{
		// reset histogram
		for(u32_i=0; u32_i<4; u32_i++)
		{
			for(u32_j=0; u32_j<3; u32_j++)
			{
				u12_me1_statis_bv_x[u32_i][u32_j] = 0;
			}
			for(u32_j=0; u32_j<17; u32_j++)
			{
				u12_me1_statis_bv_y[u32_i][u32_j] = 0;
			}
		}

		{
			// section 0 x negative
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_0_reg,
						 0,
						 3, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_0_reg,
						 4,
						 7, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_0_reg,
						 8,
						 11, &u32_RB_val3);
			u12_me1_statis_bv_x[SEC0][X_NEG] =  (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 0 x zero
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_0_reg,
						 12,
						 15, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_0_reg,
						 16,
						 19, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_0_reg,
						 20,
						 23, &u32_RB_val3);
			u12_me1_statis_bv_x[SEC0][X_ZER] =  (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 0 x positive
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_0_reg,
						 24,
						 27, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_0_reg,
						 28,
						 31, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_1_reg,
						 0,
						 3, &u32_RB_val3);
			u12_me1_statis_bv_x[SEC0][X_POS] =  (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 1 x negative
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_8_reg,
						 0,
						 3, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_8_reg,
						 4,
						 7, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_8_reg,
						 8,
						 11, &u32_RB_val3);
			u12_me1_statis_bv_x[SEC1][X_NEG] =  (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 1 x zero
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_8_reg,
						 12,
						 15, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_8_reg,
						 16,
						 19, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_8_reg,
						 20,
						 23, &u32_RB_val3);
			u12_me1_statis_bv_x[SEC1][X_ZER] =  (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 1 x positive
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_8_reg,
						 24,
						 27, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_8_reg,
						 28,
						 31, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_9_reg,
						 0,
						 3, &u32_RB_val3);
			u12_me1_statis_bv_x[SEC1][X_POS] =  (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 2 x negative
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_16_reg,
						 0,
						 3, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_16_reg,
						 4,
						 7, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_16_reg,
						 8,
						 11, &u32_RB_val3);
			u12_me1_statis_bv_x[SEC2][X_NEG] =  (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 2 x zero
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_16_reg,
						 12,
						 15, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_16_reg,
						 16,
						 19, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_16_reg,
						 20,
						 23, &u32_RB_val3);
			u12_me1_statis_bv_x[SEC2][X_ZER] =  (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 2 x positive
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_16_reg,
						 24,
						 27, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_16_reg,
						 28,
						 31, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_17_reg,
						 0,
						 3, &u32_RB_val3);
			u12_me1_statis_bv_x[SEC2][X_POS] =  (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 3 x negative
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_24_reg,
						 0,
						 3, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_24_reg,
						 4,
						 7, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_24_reg,
						 8,
						 11, &u32_RB_val3);
			u12_me1_statis_bv_x[SEC3][X_NEG] =  (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 3 x zero
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_24_reg,
						 12,
						 15, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_24_reg,
						 16,
						 19, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_24_reg,
						 20,
						 23, &u32_RB_val3);
			u12_me1_statis_bv_x[SEC3][X_ZER] =  (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 3 x positive
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_24_reg,
						 24,
						 27, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_24_reg,
						 28,
						 31, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_25_reg,
						 0,
						 3, &u32_RB_val3);
			u12_me1_statis_bv_x[SEC3][X_POS] =  (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 0 y negative 8
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_1_reg,
						 4,
						 7, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_1_reg,
						 8,
						 11, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_1_reg,
						 12,
						 15, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC0][Y_NEG_8] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 0 y negative 7
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_1_reg,
						 16,
						 19, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_1_reg,
						 20,
						 23, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_1_reg,
						 24,
						 27, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC0][Y_NEG_7] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 0 y negative 6
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_1_reg,
						 28,
						 31, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_2_reg,
						 0,
						 3, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_2_reg,
						 4,
						 7, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC0][Y_NEG_6] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 0 y negative 5
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_2_reg,
						 8,
						 11, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_2_reg,
						 12,
						 15, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_2_reg,
						 16,
						 19, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC0][Y_NEG_5] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 0 y negative 4
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_2_reg,
						 20,
						 23, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_2_reg,
						 24,
						 27, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_2_reg,
						 28,
						 31, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC0][Y_NEG_4] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 0 y negative 3
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_3_reg,
						 0,
						 3, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_3_reg,
						 4,
						 7, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_3_reg,
						 8,
						 11, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC0][Y_NEG_3] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 0 y negative 2
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_3_reg,
						 12,
						 15, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_3_reg,
						 16,
						 19, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_3_reg,
						 20,
						 23, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC0][Y_NEG_2] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 0 y negative 1
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_3_reg,
						 24,
						 27, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_3_reg,
						 28,
						 31, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_4_reg,
						 0,
						 3, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC0][Y_NEG_1] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 0 y zero
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_4_reg,
						 4,
						 7, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_4_reg,
						 8,
						 11, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_4_reg,
						 12,
						 15, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC0][Y_ZER_0] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 0 y positive 1
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_4_reg,
						 16,
						 19, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_4_reg,
						 20,
						 23, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_4_reg,
						 24,
						 27, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC0][Y_POS_1] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 0 y positive 2
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_4_reg,
						 28,
						 31, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_5_reg,
						 0,
						 3, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_5_reg,
						 4,
						 7, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC0][Y_POS_2] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 0 y positive 3
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_5_reg,
						 8,
						 11, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_5_reg,
						 12,
						 15, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_5_reg,
						 16,
						 19, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC0][Y_POS_3] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 0 y positive 4
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_5_reg,
						 20,
						 23, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_5_reg,
						 24,
						 27, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_5_reg,
						 28,
						 31, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC0][Y_POS_4] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 0 y positive 5
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_6_reg,
						 0,
						 3, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_6_reg,
						 4,
						 7, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_6_reg,
						 8,
						 11, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC0][Y_POS_5] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 0 y positive 6
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_6_reg,
						 12,
						 15, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_6_reg,
						 16,
						 19, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_6_reg,
						 20,
						 23, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC0][Y_POS_6] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 0 y positive 7
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_6_reg,
						 24,
						 27, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_6_reg,
						 28,
						 31, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_7_reg,
						 0,
						 3, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC0][Y_POS_7] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 0 y positive 8
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_7_reg,
						 4,
						 7, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_7_reg,
						 8,
						 11, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_7_reg,
						 12,
						 15, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC0][Y_POS_8] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 1 y negative 8
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_9_reg,
						 4,
						 7, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_9_reg,
						 8,
						 11, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_9_reg,
						 12,
						 15, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC1][Y_NEG_8] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 1 y negative 7
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_9_reg,
						 16,
						 19, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_9_reg,
						 20,
						 23, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_9_reg,
						 24,
						 27, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC1][Y_NEG_7] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 1 y negative 6
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_9_reg,
						 28,
						 31, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_10_reg,
						 0,
						 3, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_10_reg,
						 4,
						 7, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC1][Y_NEG_6] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 1 y negative 5
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_10_reg,
						 8,
						 11, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_10_reg,
						 12,
						 15, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_10_reg,
						 16,
						 19, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC1][Y_NEG_5] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 1 y negative 4
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_10_reg,
						 20,
						 23, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_10_reg,
						 24,
						 27, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_10_reg,
						 28,
						 31, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC1][Y_NEG_4] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 1 y negative 3
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_11_reg,
						 0,
						 3, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_11_reg,
						 4,
						 7, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_11_reg,
						 8,
						 11, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC1][Y_NEG_3] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 1 y negative 2
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_11_reg,
						 12,
						 15, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_11_reg,
						 16,
						 19, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_11_reg,
						 20,
						 23, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC1][Y_NEG_2] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 1 y negative 1
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_11_reg,
						 24,
						 27, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_11_reg,
						 28,
						 31, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_12_reg,
						 0,
						 3, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC1][Y_NEG_1] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 1 y zero
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_12_reg,
						 4,
						 7, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_12_reg,
						 8,
						 11, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_12_reg,
						 12,
						 15, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC1][Y_ZER_0] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 1 y positive 1
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_12_reg,
						 16,
						 19, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_12_reg,
						 20,
						 23, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_12_reg,
						 24,
						 27, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC1][Y_POS_1] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 1 y positive 2
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_12_reg,
						 28,
						 31, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_13_reg,
						 0,
						 3, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_13_reg,
						 4,
						 7, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC1][Y_POS_2] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 1 y positive 3
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_13_reg,
						 8,
						 11, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_13_reg,
						 12,
						 15, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_13_reg,
						 16,
						 19, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC1][Y_POS_3] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 1 y positive 4
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_13_reg,
						 20,
						 23, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_13_reg,
						 24,
						 27, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_13_reg,
						 28,
						 31, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC1][Y_POS_4] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 1 y positive 5
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_14_reg,
						 0,
						 3, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_14_reg,
						 4,
						 7, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_14_reg,
						 8,
						 11, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC1][Y_POS_5] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 1 y positive 6
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_14_reg,
						 12,
						 15, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_14_reg,
						 16,
						 19, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_14_reg,
						 20,
						 23, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC1][Y_POS_6] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 1 y positive 7
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_14_reg,
						 24,
						 27, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_14_reg,
						 28,
						 31, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_15_reg,
						 0,
						 3, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC1][Y_POS_7] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 1 y positive 8
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_15_reg,
						 4,
						 7, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_15_reg,
						 8,
						 11, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_15_reg,
						 12,
						 15, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC1][Y_POS_8] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 2 y negative 8
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_17_reg,
						 4,
						 7, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_17_reg,
						 8,
						 11, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_17_reg,
						 12,
						 15, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC2][Y_NEG_8] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 2 y negative 7
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_17_reg,
						 16,
						 19, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_17_reg,
						 20,
						 23, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_17_reg,
						 24,
						 27, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC2][Y_NEG_7] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 2 y negative 6
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_17_reg,
						 28,
						 31, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_18_reg,
						 0,
						 3, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_18_reg,
						 4,
						 7, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC2][Y_NEG_6] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 2 y negative 5
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_18_reg,
						 8,
						 11, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_18_reg,
						 12,
						 15, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_18_reg,
						 16,
						 19, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC2][Y_NEG_5] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 2 y negative 4
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_18_reg,
						 20,
						 23, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_18_reg,
						 24,
						 27, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_18_reg,
						 28,
						 31, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC2][Y_NEG_4] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 2 y negative 3
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_19_reg,
						 0,
						 3, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_19_reg,
						 4,
						 7, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_19_reg,
						 8,
						 11, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC2][Y_NEG_3] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 2 y negative 2
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_19_reg,
						 12,
						 15, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_19_reg,
						 16,
						 19, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_19_reg,
						 20,
						 23, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC2][Y_NEG_2] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 2 y negative 1
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_19_reg,
						 24,
						 27, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_19_reg,
						 28,
						 31, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_20_reg,
						 0,
						 3, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC2][Y_NEG_1] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 2 y zero
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_20_reg,
						 4,
						 7, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_20_reg,
						 8,
						 11, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_20_reg,
						 12,
						 15, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC2][Y_ZER_0] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 2 y positive 1
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_20_reg,
						 16,
						 19, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_20_reg,
						 20,
						 23, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_20_reg,
						 24,
						 27, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC2][Y_POS_1] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 2 y positive 2
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_20_reg,
						 28,
						 31, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_21_reg,
						 0,
						 3, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_21_reg,
						 4,
						 7, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC2][Y_POS_2] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 2 y positive 3
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_21_reg,
						 8,
						 11, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_21_reg,
						 12,
						 15, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_21_reg,
						 16,
						 19, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC2][Y_POS_3] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 2 y positive 4
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_21_reg,
						 20,
						 23, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_21_reg,
						 24,
						 27, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_21_reg,
						 28,
						 31, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC2][Y_POS_4] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 2 y positive 5
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_22_reg,
						 0,
						 3, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_22_reg,
						 4,
						 7, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_22_reg,
						 8,
						 11, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC2][Y_POS_5] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 2 y positive 6
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_22_reg,
						 12,
						 15, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_22_reg,
						 16,
						 19, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_22_reg,
						 20,
						 23, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC2][Y_POS_6] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 2 y positive 7
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_22_reg,
						 24,
						 27, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_22_reg,
						 28,
						 31, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_23_reg,
						 0,
						 3, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC2][Y_POS_7] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 2 y positive 8
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_23_reg,
						 4,
						 7, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_23_reg,
						 8,
						 11, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_23_reg,
						 12,
						 15, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC2][Y_POS_8] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 3 y negative 8
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_25_reg,
						 4,
						 7, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_25_reg,
						 8,
						 11, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_25_reg,
						 12,
						 15, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC3][Y_NEG_8] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 3 y negative 7
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_25_reg,
						 16,
						 19, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_25_reg,
						 20,
						 23, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_25_reg,
						 24,
						 27, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC3][Y_NEG_7] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 3 y negative 6
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_25_reg,
						 28,
						 31, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_26_reg,
						 0,
						 3, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_26_reg,
						 4,
						 7, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC3][Y_NEG_6] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 3 y negative 5
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_26_reg,
						 8,
						 11, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_26_reg,
						 12,
						 15, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_26_reg,
						 16,
						 19, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC3][Y_NEG_5] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 3 y negative 4
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_26_reg,
						 20,
						 23, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_26_reg,
						 24,
						 27, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_26_reg,
						 28,
						 31, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC3][Y_NEG_4] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 3 y negative 3
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_27_reg,
						 0,
						 3, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_27_reg,
						 4,
						 7, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_27_reg,
						 8,
						 11, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC3][Y_NEG_3] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 3 y negative 2
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_27_reg,
						 12,
						 15, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_27_reg,
						 16,
						 19, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_27_reg,
						 20,
						 23, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC3][Y_NEG_2] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 3 y negative 1
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_27_reg,
						 24,
						 27, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_27_reg,
						 28,
						 31, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_28_reg,
						 0,
						 3, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC3][Y_NEG_1] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 3 y zero
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_28_reg,
						 4,
						 7, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_28_reg,
						 8,
						 11, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_28_reg,
						 12,
						 15, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC3][Y_ZER_0] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 3 y positive 1
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_28_reg,
						 16,
						 19, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_28_reg,
						 20,
						 23, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_28_reg,
						 24,
						 27, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC3][Y_POS_1] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 3 y positive 2
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_28_reg,
						 28,
						 31, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_29_reg,
						 0,
						 3, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_29_reg,
						 4,
						 7, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC3][Y_POS_2] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 3 y positive 3
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_29_reg,
						 8,
						 11, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_29_reg,
						 12,
						 15, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_29_reg,
						 16,
						 19, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC3][Y_POS_3] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 3 y positive 4
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_29_reg,
						 20,
						 23, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_29_reg,
						 24,
						 27, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_29_reg,
						 28,
						 31, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC3][Y_POS_4] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 3 y positive 5
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_30_reg,
						 0,
						 3, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_30_reg,
						 4,
						 7, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_30_reg,
						 8,
						 11, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC3][Y_POS_5] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 3 y positive 6
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_30_reg,
						 12,
						 15, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_30_reg,
						 16,
						 19, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_30_reg,
						 20,
						 23, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC3][Y_POS_6] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 3 y positive 7
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_30_reg,
						 24,
						 27, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_30_reg,
						 28,
						 31, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_31_reg,
						 0,
						 3, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC3][Y_POS_7] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}

		{
			// section 3 y positive 8
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_31_reg,
						 4,
						 7, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_31_reg,
						 8,
						 11, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_STAT_31_reg,
						 12,
						 15, &u32_RB_val3);
			u12_me1_statis_bv_y[SEC3][Y_POS_8] = (u32_RB_val3 << 8) + (u32_RB_val2 << 4) + u32_RB_val1;
		}
		//  RTL x_zer issue
		for(u32_i=0; u32_i<SEC_NUM; u32_i++)
		{
			for(u32_j=0; u32_j<X_NUM; u32_j++)
			{
				if(u12_me1_statis_bv_x[u32_i][u32_j] >=1000)
					u12_me1_statis_bv_x[u32_i][u32_j] =1000;
			}
			for(u32_j=0; u32_j<Y_NUM; u32_j++)
			{
				if(u12_me1_statis_bv_y[u32_i][u32_j] >= 1000)
					u12_me1_statis_bv_y[u32_i][u32_j] =1000;
			}
		}
#if ME_VST_DEBUG_PRINT
		if( u32_print_counter == 0 )
		{
			rtd_pr_memc_debug("%s %d\r\n", __func__, u32_print_counter);
			for(u32_i=0; u32_i<SEC_NUM; u32_i++)
			{
				for(u32_j=0; u32_j<X_NUM; u32_j++)
				{
					rtd_pr_memc_debug("%d ", u12_me1_statis_bv_x[u32_i][u32_j]);
				}
				rtd_pr_memc_debug("\r\n");
				for(u32_j=0; u32_j<Y_NUM; u32_j++)
				{
					rtd_pr_memc_debug("%d ", u12_me1_statis_bv_y[u32_i][u32_j]);
				}
				rtd_pr_memc_debug("\r\n\r\n");
			}
		}
#endif
	}
}
VOID ME_Vst_Statistic()
{
	unsigned int u8_boundary[5] = {0};
	unsigned char u8_i = 0, u8_j = 0;

	// Read boundary information
	ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_CTRL_2_reg,
				 0,
				 7, &u8_boundary[0]);
	ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_CTRL_2_reg,
				 8,
				 15, &u8_boundary[1]);
	ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_CTRL_2_reg,
				 16,
				 23, &u8_boundary[2]);
	ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_CTRL_2_reg,
				 24,
				 31, &u8_boundary[3]);
	ReadRegister(KME_ME1_TOP10_KME_ME1_TOP10_VST_CTRL_1_reg,
				 0,
				 7, &u8_boundary[4]);

	u32_rgnTotalBlks[0] = (u8_boundary[1] > u8_boundary[0])? (u8_boundary[1]-u8_boundary[0])*135 : 0;
	u32_rgnTotalBlks[1] = (u8_boundary[2] > u8_boundary[1])? (u8_boundary[2]-u8_boundary[1])*135 : 0;
	u32_rgnTotalBlks[2] = (u8_boundary[3] > u8_boundary[2])? (u8_boundary[3]-u8_boundary[2])*135 : 0;
	u32_rgnTotalBlks[3] = (u8_boundary[4] > u8_boundary[3])? (u8_boundary[4]-u8_boundary[3])*135 : 0;

	// Get the region statistic total region
	for(u8_i=0; u8_i<SEC_NUM; u8_i++)
	{
		u32_rgnTotalStatBlks_x[u8_i] = 0;
		for(u8_j=0; u8_j<X_NUM; u8_j++)
		{
			u32_rgnTotalStatBlks_x[u8_i] += u12_me1_statis_bv_x[u8_i][u8_j];
		}

		u32_rgnTotalStatBlks_y[u8_i] = 0;
		for(u8_j=0; u8_j<Y_NUM; u8_j++)
		{
			u32_rgnTotalStatBlks_y[u8_i] += u12_me1_statis_bv_y[u8_i][u8_j];
		}
	}
}
VOID ME_Vst_Detection(_OUTPUT_ME_VST_CTRL *pOutput)
{
	const unsigned short u8_rgnDetectTh = 16;
	const unsigned char u8_iirSftDivisor = 2, u8_iirCoef = 3; // (score*coef >> sft)
	
	unsigned int  u32_HistBinsSum = 0;
	static unsigned short u16_rgnDetectScroe[4] = {0};
	unsigned char u8_i = 0, u8_j = 0;
	unsigned char u1_cond1 = 0, u1_cond2 = 0, u1_cond3 = 0;

	static unsigned int u32_print_counter = 0;

	u32_print_counter++;
	if(u32_print_counter >= 60)
		u32_print_counter = 0;

	// condition judge
	for(u8_i=0; u8_i<SEC_NUM; u8_i++)
	{
		u32_HistBinsSum = 0;
		for(u8_j=Y_POS_5; u8_j<=Y_POS_6; u8_j++)
		{
			u32_HistBinsSum += u12_me1_statis_bv_y[u8_i][u8_j];
		}

		u1_cond1 = (MAX( u32_rgnTotalStatBlks_x[u8_i], u32_rgnTotalStatBlks_y[u8_i] ) > (u32_rgnTotalBlks[u8_i]>>4)); // total stat blks amount is enough
		u1_cond2 = (u32_HistBinsSum > ( ((u32_rgnTotalStatBlks_y[u8_i]-(u12_me1_statis_bv_y[u8_i][Y_ZER_0]/2)) * 5) >> 3)); // target mv is larget enough
		u1_cond3 = ((u12_me1_statis_bv_x[u8_i][X_ZER]) > (u32_rgnTotalStatBlks_x[u8_i]>>1)); // target mv is larget enough

		if( u1_cond1 && u1_cond2 && u1_cond3 )
		{
#if RTK_MEMC_Performance_tunging_from_tv001
			u16_rgnDetectScroe[u8_i] = Min(u16_rgnDetectScroe[u8_i]+1, 512);
#else
			u16_rgnDetectScroe[u8_i] = MIN(u16_rgnDetectScroe[u8_i]+1, 512);
#endif	
		}
		else
		{
			u16_rgnDetectScroe[u8_i] = (u16_rgnDetectScroe[u8_i]*u8_iirCoef)>>u8_iirSftDivisor;
		}

#if ME_VST_DEBUG_PRINT
		if(u32_print_counter == 0)
		{
			rtd_pr_memc_debug("cond = %d %d %d\r\n", u1_cond1, u1_cond2, u1_cond3);
		}
		if(u32_print_counter == 0 )
		{
			rtd_pr_memc_debug("cond = %d %d %d %d %d\r\n", u32_rgnTotalStatBlks_x[u8_i], u32_rgnTotalStatBlks_y[u8_i], u32_HistBinsSum,(u12_me1_statis_bv_y[u8_i][Y_ZER_0]/2),u12_me1_statis_bv_x[u8_i][X_ZER]);
		}
#endif

	}

	// case analysis
	pOutput->u8_boundarySta = 0;
	pOutput->u8_boundaryEnd = 0;
	pOutput->u1_detect = 0;

	if( u16_rgnDetectScroe[0] > u8_rgnDetectTh && u16_rgnDetectScroe[1] <= u8_rgnDetectTh && u16_rgnDetectScroe[2] <= u8_rgnDetectTh && u16_rgnDetectScroe[3] <= u8_rgnDetectTh )
	{
		pOutput->u8_boundarySta = 0;
		pOutput->u8_boundaryEnd = 59;

		pOutput->u1_detect = 1;
	}
	else if( u16_rgnDetectScroe[0] <= u8_rgnDetectTh && u16_rgnDetectScroe[1] > u8_rgnDetectTh && u16_rgnDetectScroe[2] <= u8_rgnDetectTh && u16_rgnDetectScroe[3] <= u8_rgnDetectTh )
	{
		pOutput->u8_boundarySta = 60;
		pOutput->u8_boundaryEnd = 119;

		pOutput->u1_detect = 1;
	}
	else if( u16_rgnDetectScroe[0] <= u8_rgnDetectTh && u16_rgnDetectScroe[1] <= u8_rgnDetectTh && u16_rgnDetectScroe[2] > u8_rgnDetectTh && u16_rgnDetectScroe[3] <= u8_rgnDetectTh )
	{
		pOutput->u8_boundarySta = 120;
		pOutput->u8_boundaryEnd = 179;

		pOutput->u1_detect = 1;
	}
	else if( u16_rgnDetectScroe[0] <= u8_rgnDetectTh && u16_rgnDetectScroe[1] <= u8_rgnDetectTh && u16_rgnDetectScroe[2] <= u8_rgnDetectTh && u16_rgnDetectScroe[3] > u8_rgnDetectTh )
	{
		pOutput->u8_boundarySta = 180;
		pOutput->u8_boundaryEnd = 239;

		pOutput->u1_detect = 1;
	}
	else if( u16_rgnDetectScroe[0] > u8_rgnDetectTh && u16_rgnDetectScroe[1] > u8_rgnDetectTh && u16_rgnDetectScroe[2] <= u8_rgnDetectTh && u16_rgnDetectScroe[3] <= u8_rgnDetectTh )
	{
		pOutput->u8_boundarySta = 0;
		pOutput->u8_boundaryEnd = 119;

		pOutput->u1_detect = 1;
	}
	else if( u16_rgnDetectScroe[0] > u8_rgnDetectTh && u16_rgnDetectScroe[1] <= u8_rgnDetectTh && u16_rgnDetectScroe[2] > u8_rgnDetectTh && u16_rgnDetectScroe[3] <= u8_rgnDetectTh )
	{
		// need futer judge, not complete finish yet
		pOutput->u8_boundarySta = 0;
		pOutput->u8_boundaryEnd = 0;

		pOutput->u1_detect = 0;
	}
	else if( u16_rgnDetectScroe[0] > u8_rgnDetectTh && u16_rgnDetectScroe[1] <= u8_rgnDetectTh && u16_rgnDetectScroe[2] <= u8_rgnDetectTh && u16_rgnDetectScroe[3] > u8_rgnDetectTh )
	{
		// need futer judge, not complete finish yet
		pOutput->u8_boundarySta = 0;
		pOutput->u8_boundaryEnd = 0;

		pOutput->u1_detect = 0;
	}
	else if( u16_rgnDetectScroe[0] <= u8_rgnDetectTh && u16_rgnDetectScroe[1] > u8_rgnDetectTh && u16_rgnDetectScroe[2] > u8_rgnDetectTh && u16_rgnDetectScroe[3] <= u8_rgnDetectTh )
	{
		pOutput->u8_boundarySta = 60;
		pOutput->u8_boundaryEnd = 179;

		pOutput->u1_detect = 1;
	}
	else if( u16_rgnDetectScroe[0] <= u8_rgnDetectTh && u16_rgnDetectScroe[1] > u8_rgnDetectTh && u16_rgnDetectScroe[2] <= u8_rgnDetectTh && u16_rgnDetectScroe[3] > u8_rgnDetectTh )
	{
		// need futer judge, not complete finish yet
		pOutput->u8_boundarySta = 0;
		pOutput->u8_boundaryEnd = 0;

		pOutput->u1_detect = 0;
	}
	else if( u16_rgnDetectScroe[0] <= u8_rgnDetectTh && u16_rgnDetectScroe[1] <= u8_rgnDetectTh && u16_rgnDetectScroe[2] > u8_rgnDetectTh && u16_rgnDetectScroe[3] > u8_rgnDetectTh )
	{
		pOutput->u8_boundarySta = 120;
		pOutput->u8_boundaryEnd = 239;

		pOutput->u1_detect = 1;
	}
	else if( u16_rgnDetectScroe[0] > u8_rgnDetectTh && u16_rgnDetectScroe[1] > u8_rgnDetectTh && u16_rgnDetectScroe[2] > u8_rgnDetectTh && u16_rgnDetectScroe[3] <= u8_rgnDetectTh )
	{
		pOutput->u8_boundarySta = 0;
		pOutput->u8_boundaryEnd = 179;

		pOutput->u1_detect = 1;
	}
	else if( u16_rgnDetectScroe[0] > u8_rgnDetectTh && u16_rgnDetectScroe[1] > u8_rgnDetectTh && u16_rgnDetectScroe[2] <= u8_rgnDetectTh && u16_rgnDetectScroe[3] > u8_rgnDetectTh )
	{
		// need futer judge, not complete finish yet
		pOutput->u8_boundarySta = 0;
		pOutput->u8_boundaryEnd = 0;

		pOutput->u1_detect = 0;
	}
	else if( u16_rgnDetectScroe[0] <= u8_rgnDetectTh && u16_rgnDetectScroe[1] > u8_rgnDetectTh && u16_rgnDetectScroe[2] > u8_rgnDetectTh && u16_rgnDetectScroe[3] > u8_rgnDetectTh )
	{
		pOutput->u8_boundarySta = 60;
		pOutput->u8_boundaryEnd = 239;

		pOutput->u1_detect = 1;
	}
	else if( u16_rgnDetectScroe[0] > u8_rgnDetectTh && u16_rgnDetectScroe[1] > u8_rgnDetectTh && u16_rgnDetectScroe[2] > u8_rgnDetectTh && u16_rgnDetectScroe[3] > u8_rgnDetectTh )
	{
		pOutput->u8_boundarySta = 0;
		pOutput->u8_boundaryEnd = 239;

		pOutput->u1_detect = 1;
	}
#if ME_VST_DEBUG_PRINT
	if(u32_print_counter == 0)
	{
		rtd_pr_memc_debug("detect result = %d %d %d %d, from %d to %d\r\n",
			u16_rgnDetectScroe[0], u16_rgnDetectScroe[1], u16_rgnDetectScroe[2], u16_rgnDetectScroe[3], pOutput->u8_boundarySta, pOutput->u8_boundaryEnd);
	}
#endif
}
VOID ME_Vst_blackBG_Detection(_OUTPUT_ME_VST_CTRL *pOutput)
{
	const unsigned short u8_rgnDetectTh = 16;
	const unsigned char u8_iirSftDivisor = 2, u8_iirCoef = 3; // (score*coef >> sft)
	unsigned int  u32_HistBinsSum = 0;
	static unsigned short u16_rgnDetectScroe[4] = {0};
	unsigned char u8_i = 0, u8_j = 0;
	unsigned char u1_cond1 = 0, u1_cond2 = 0, u1_cond3 = 0;

	static unsigned int u32_print_counter = 0;

	u32_print_counter++;
	if(u32_print_counter >= 60)
		u32_print_counter = 0;

	
	// condition judge
	for(u8_i=0; u8_i<SEC_NUM; u8_i++)
	{
		u32_HistBinsSum = 0;
		for(u8_j=Y_POS_7; u8_j<=Y_POS_8; u8_j++)
		{
			u32_HistBinsSum += u12_me1_statis_bv_y[u8_i][u8_j];
		}

		u1_cond1 = (MAX( u32_rgnTotalStatBlks_x[u8_i], u32_rgnTotalStatBlks_y[u8_i] ) > (u32_rgnTotalBlks[u8_i]>>4)); // total stat blks amount is enough
		u1_cond2 = (u32_HistBinsSum > ( ((u32_rgnTotalStatBlks_y[u8_i]-(u12_me1_statis_bv_y[u8_i][Y_ZER_0]/2)) * 5) >> 3) && (u32_HistBinsSum >300)); // target mv is larget enough
		u1_cond3 = ((u12_me1_statis_bv_x[u8_i][X_ZER]) > (u32_rgnTotalStatBlks_x[u8_i]>>1)); // target mv is larget enough

		if( u1_cond1 && u1_cond2 && u1_cond3 )
		{

#if RTK_MEMC_Performance_tunging_from_tv001
			u16_rgnDetectScroe[u8_i] = Min(u16_rgnDetectScroe[u8_i]+1, 512);	
#else	
			u16_rgnDetectScroe[u8_i] = MIN(u16_rgnDetectScroe[u8_i]+1, 512);
#endif	
		}
		else
		{
			u16_rgnDetectScroe[u8_i] = (u16_rgnDetectScroe[u8_i]*u8_iirCoef)>>u8_iirSftDivisor;
		}

#if ME_VST_DEBUG_PRINT
		if(u32_print_counter == 0)
		{
			rtd_pr_memc_debug("blackBG cond = %d %d %d\r\n", u1_cond1, u1_cond2, u1_cond3);
		}
#endif

	}

	// case analysis
	pOutput->u1_detect_blackBG_VST_trure= 0;

	if( u16_rgnDetectScroe[0] > u8_rgnDetectTh && u16_rgnDetectScroe[1] <= u8_rgnDetectTh && u16_rgnDetectScroe[2] <= u8_rgnDetectTh && u16_rgnDetectScroe[3] <= u8_rgnDetectTh )
	{
		pOutput->u8_boundarySta = 0;
		pOutput->u8_boundaryEnd = 59;

		pOutput->u1_detect_blackBG_VST_trure = 1;
	}
	else if( u16_rgnDetectScroe[0] <= u8_rgnDetectTh && u16_rgnDetectScroe[1] > u8_rgnDetectTh && u16_rgnDetectScroe[2] <= u8_rgnDetectTh && u16_rgnDetectScroe[3] <= u8_rgnDetectTh )
	{
		pOutput->u8_boundarySta = 60;
		pOutput->u8_boundaryEnd = 119;

		pOutput->u1_detect_blackBG_VST_trure = 1;
	}
	else if( u16_rgnDetectScroe[0] <= u8_rgnDetectTh && u16_rgnDetectScroe[1] <= u8_rgnDetectTh && u16_rgnDetectScroe[2] > u8_rgnDetectTh && u16_rgnDetectScroe[3] <= u8_rgnDetectTh )
	{
		pOutput->u8_boundarySta = 120;
		pOutput->u8_boundaryEnd = 179;

		pOutput->u1_detect_blackBG_VST_trure = 1;
	}
	else if( u16_rgnDetectScroe[0] <= u8_rgnDetectTh && u16_rgnDetectScroe[1] <= u8_rgnDetectTh && u16_rgnDetectScroe[2] <= u8_rgnDetectTh && u16_rgnDetectScroe[3] > u8_rgnDetectTh )
	{
		pOutput->u8_boundarySta = 180;
		pOutput->u8_boundaryEnd = 239;

		pOutput->u1_detect_blackBG_VST_trure = 1;
	}
	else if( u16_rgnDetectScroe[0] > u8_rgnDetectTh && u16_rgnDetectScroe[1] > u8_rgnDetectTh && u16_rgnDetectScroe[2] <= u8_rgnDetectTh && u16_rgnDetectScroe[3] <= u8_rgnDetectTh )
	{
		pOutput->u8_boundarySta = 0;
		pOutput->u8_boundaryEnd = 119;

		pOutput->u1_detect_blackBG_VST_trure = 1;
	}
	else if( u16_rgnDetectScroe[0] > u8_rgnDetectTh && u16_rgnDetectScroe[1] <= u8_rgnDetectTh && u16_rgnDetectScroe[2] > u8_rgnDetectTh && u16_rgnDetectScroe[3] <= u8_rgnDetectTh )
	{
		// need futer judge, not complete finish yet
		pOutput->u8_boundarySta = 0;
		pOutput->u8_boundaryEnd = 0;

		pOutput->u1_detect_blackBG_VST_trure = 0;
	}
	else if( u16_rgnDetectScroe[0] > u8_rgnDetectTh && u16_rgnDetectScroe[1] <= u8_rgnDetectTh && u16_rgnDetectScroe[2] <= u8_rgnDetectTh && u16_rgnDetectScroe[3] > u8_rgnDetectTh )
	{
		// need futer judge, not complete finish yet
		pOutput->u8_boundarySta = 0;
		pOutput->u8_boundaryEnd = 0;

		pOutput->u1_detect_blackBG_VST_trure = 0;
	}
	else if( u16_rgnDetectScroe[0] <= u8_rgnDetectTh && u16_rgnDetectScroe[1] > u8_rgnDetectTh && u16_rgnDetectScroe[2] > u8_rgnDetectTh && u16_rgnDetectScroe[3] <= u8_rgnDetectTh )
	{
		pOutput->u8_boundarySta = 60;
		pOutput->u8_boundaryEnd = 179;

		pOutput->u1_detect_blackBG_VST_trure = 1;
	}
	else if( u16_rgnDetectScroe[0] <= u8_rgnDetectTh && u16_rgnDetectScroe[1] > u8_rgnDetectTh && u16_rgnDetectScroe[2] <= u8_rgnDetectTh && u16_rgnDetectScroe[3] > u8_rgnDetectTh )
	{
		// need futer judge, not complete finish yet
		pOutput->u8_boundarySta = 0;
		pOutput->u8_boundaryEnd = 0;

		pOutput->u1_detect_blackBG_VST_trure = 0;
	}
	else if( u16_rgnDetectScroe[0] <= u8_rgnDetectTh && u16_rgnDetectScroe[1] <= u8_rgnDetectTh && u16_rgnDetectScroe[2] > u8_rgnDetectTh && u16_rgnDetectScroe[3] > u8_rgnDetectTh )
	{
		pOutput->u8_boundarySta = 120;
		pOutput->u8_boundaryEnd = 239;

		pOutput->u1_detect_blackBG_VST_trure = 1;
	}
	else if( u16_rgnDetectScroe[0] > u8_rgnDetectTh && u16_rgnDetectScroe[1] > u8_rgnDetectTh && u16_rgnDetectScroe[2] > u8_rgnDetectTh && u16_rgnDetectScroe[3] <= u8_rgnDetectTh )
	{
		pOutput->u8_boundarySta = 0;
		pOutput->u8_boundaryEnd = 179;

		pOutput->u1_detect_blackBG_VST_trure = 1;
	}
	else if( u16_rgnDetectScroe[0] > u8_rgnDetectTh && u16_rgnDetectScroe[1] > u8_rgnDetectTh && u16_rgnDetectScroe[2] <= u8_rgnDetectTh && u16_rgnDetectScroe[3] > u8_rgnDetectTh )
	{
		// need futer judge, not complete finish yet
		pOutput->u8_boundarySta = 0;
		pOutput->u8_boundaryEnd = 0;

		pOutput->u1_detect_blackBG_VST_trure = 0;
	}
	else if( u16_rgnDetectScroe[0] <= u8_rgnDetectTh && u16_rgnDetectScroe[1] > u8_rgnDetectTh && u16_rgnDetectScroe[2] > u8_rgnDetectTh && u16_rgnDetectScroe[3] > u8_rgnDetectTh )
	{
		pOutput->u8_boundarySta = 60;
		pOutput->u8_boundaryEnd = 239;

		pOutput->u1_detect_blackBG_VST_trure = 1;
	}
	else if( u16_rgnDetectScroe[0] > u8_rgnDetectTh && u16_rgnDetectScroe[1] > u8_rgnDetectTh && u16_rgnDetectScroe[2] > u8_rgnDetectTh && u16_rgnDetectScroe[3] > u8_rgnDetectTh )
	{
		pOutput->u8_boundarySta = 0;
		pOutput->u8_boundaryEnd = 239;

		pOutput->u1_detect_blackBG_VST_trure = 1;
	}
#if ME_VST_DEBUG_PRINT
	if(u32_print_counter == 0)
	{
		rtd_pr_memc_debug("detect result = %d %d %d %d, from %d to %d\r\n",
			u16_rgnDetectScroe[0], u16_rgnDetectScroe[1], u16_rgnDetectScroe[2], u16_rgnDetectScroe[3], pOutput->u8_boundarySta, pOutput->u8_boundaryEnd);
	}
#endif
}

