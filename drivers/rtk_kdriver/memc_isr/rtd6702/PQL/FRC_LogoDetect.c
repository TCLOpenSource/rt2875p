#include "memc_isr/PQL/PQLPlatformDefs.h"
#include "memc_isr/PQL/PQLContext.h"
#include "memc_isr/PQL/FRC_LogoDetect.h"
#include "memc_isr/PQL/PQLGlobalDefs.h"
#include "memc_isr/PQL/Read_ComReg.h"
#include "memc_reg_def.h"
///////////////////////////////////////////////////////////////////////////////////////////
/*
PQL Improvement:
1. PQL run at PC use _WIN32. (need change to more clear define)
2. Param must be init in order, gen PQL_Glb_Param.c (it's not convenient, can same as output?)


*/
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
VOID FRC_LogoDet_Init(_OUTPUT_FRC_LGDet *pOutput)
{
	int i;
	pOutput->s16_blklogo_glbclr_cnt = 0;

	for (i=0; i<LOGO_RG_32; i++)
	{
		*(pOutput->s16_blklogo_rgclr_cnt + i) = 0;
	}

	pOutput->u1_rg_dh_clr_rg0 = 0;
	pOutput->u1_rg_dh_clr_rg1 = 0;
	pOutput->u1_rg_dh_clr_rg2 = 0;
	pOutput->u1_rg_dh_clr_rg3 = 0;

	pOutput->u8_rg_dh_clr_lgclr_hold_frm = 0;
	pOutput->u8_rg_dh_clr_static_hold_frm = 0;

	pOutput->u32_logo_hsty_clr_idx = 0;
	pOutput->u32_logo_hsty_clr_lgclr_idx = 0;
	pOutput->u1_logo_cnt_glb_clr_status = 0;
	pOutput->u1_logo_sc_glb_clr_status  = 0;

	pOutput->u1_logo_static_status = 0;

	pOutput->u31_logo_sad_pre = 0;
	pOutput->u1_logo_sc_status = 0;
	pOutput->u8_logo_sc_hold_frm = 0;

	pOutput->u1_logo_sc_dtldif_status = 0;
	pOutput->u1_logo_sc_Gmv_status = 0;
	pOutput->u1_logo_sc_saddif_status = 0;
	pOutput->u8_Rg_static_num = 0;
	pOutput->u32_Rg_unstatic_idx = 0;

	pOutput->u1_logo_netflix_status = 0;
	pOutput->u8_logo_netflix_hold_frm = 0;

	pOutput->u1_logo_lg16s_patch_status   = 0;
	pOutput->u8_logo_lg16s_patch_hold_frm = 0;

	pOutput->u1_logo_lg16s_patch_Gapli_status     = 0;
	pOutput->u1_logo_lg16s_patch_Gaplp_status     = 0;
	pOutput->u1_logo_lg16s_patch_Gdtl_status     = 0;
	pOutput->u1_logo_lg16s_patch_Gsad_status     = 0;
	pOutput->u1_logo_lg16s_patch_Gmv_status      = 0;
	pOutput->u1_logo_lg16s_patch_Rdtl_num_status = 0;

	// UXN patch
	pOutput->u1_lg_UXN_patch_det = 0;

	// Defualt value storage
	ReadRegister(KME_LOGO0_KME_LOGO0_68_reg, 0, 10, &pOutput->DefaultVal.reg_km_logo_blkgrdsum2_th);
	ReadRegister(KME_LOGO0_KME_LOGO0_EC_reg, 8, 8, &pOutput->DefaultVal.reg_km_logo_blkadphstystep_en);
	ReadRegister(KME_LOGO0_KME_LOGO0_EC_reg, 0, 3, &pOutput->DefaultVal.reg_km_logo_blkhsty_nstep);
	ReadRegister(KME_LOGO0_KME_LOGO0_EC_reg, 4, 7, &pOutput->DefaultVal.reg_km_logo_blkhsty_pstep);

	ReadRegister(MC2_MC2_98_reg,8, 8, &pOutput->DefaultVal.reg_mc_logo_vlpf_en);
	ReadRegister(KME_LOGO0_KME_LOGO0_10_reg, 0, 4, &pOutput->DefaultVal.reg_km_logo_iir_alpha);
	ReadRegister(MC2_MC2_20_reg, 0, 1, &pOutput->DefaultVal.reg_mc_logo_en);

	ReadRegister(KME_LOGO1_KME_LOGO1_10_reg,  0,  3,  &pOutput->DefaultVal.blkhsty_pth[0]);
	ReadRegister(KME_LOGO1_KME_LOGO1_10_reg,  4,  7,  &pOutput->DefaultVal.blkhsty_pth[1]);
	ReadRegister(KME_LOGO1_KME_LOGO1_10_reg,  8,  11,  &pOutput->DefaultVal.blkhsty_pth[2]);
	ReadRegister(KME_LOGO1_KME_LOGO1_10_reg,  12,  15,  &pOutput->DefaultVal.blkhsty_pth[3]);
	ReadRegister(KME_LOGO1_KME_LOGO1_10_reg,  16,  19,  &pOutput->DefaultVal.blkhsty_pth[4]);
	ReadRegister(KME_LOGO1_KME_LOGO1_10_reg,  20,  23,  &pOutput->DefaultVal.blkhsty_pth[5]);
	ReadRegister(KME_LOGO1_KME_LOGO1_10_reg,  24,  27,  &pOutput->DefaultVal.blkhsty_pth[6]);
	ReadRegister(KME_LOGO1_KME_LOGO1_10_reg,  28,  31,  &pOutput->DefaultVal.blkhsty_pth[7]);
	ReadRegister(KME_LOGO1_KME_LOGO1_14_reg,  0,  3,  &pOutput->DefaultVal.blkhsty_pth[8]);
	ReadRegister(KME_LOGO1_KME_LOGO1_14_reg,  4,  7,  &pOutput->DefaultVal.blkhsty_pth[9]);
	ReadRegister(KME_LOGO1_KME_LOGO1_14_reg, 8, 11, &pOutput->DefaultVal.blkhsty_pth[10]);
	ReadRegister(KME_LOGO1_KME_LOGO1_14_reg, 12, 15, &pOutput->DefaultVal.blkhsty_pth[11]);
	ReadRegister(KME_LOGO1_KME_LOGO1_14_reg, 16, 19, &pOutput->DefaultVal.blkhsty_pth[12]);
	ReadRegister(KME_LOGO1_KME_LOGO1_14_reg, 20, 23, &pOutput->DefaultVal.blkhsty_pth[13]);
	ReadRegister(KME_LOGO1_KME_LOGO1_14_reg, 24, 27, &pOutput->DefaultVal.blkhsty_pth[14]);

	ReadRegister(KME_LOGO1_KME_LOGO1_18_reg,  0,  3,  &pOutput->DefaultVal.blkhsty_nth[0]);
	ReadRegister(KME_LOGO1_KME_LOGO1_18_reg,  4,  7,  &pOutput->DefaultVal.blkhsty_nth[1]);
	ReadRegister(KME_LOGO1_KME_LOGO1_18_reg,  8,  11,  &pOutput->DefaultVal.blkhsty_nth[2]);
	ReadRegister(KME_LOGO1_KME_LOGO1_18_reg,  12,  15,  &pOutput->DefaultVal.blkhsty_nth[3]);
	ReadRegister(KME_LOGO1_KME_LOGO1_18_reg,  16,  19,  &pOutput->DefaultVal.blkhsty_nth[4]);
	ReadRegister(KME_LOGO1_KME_LOGO1_18_reg,  20,  23,  &pOutput->DefaultVal.blkhsty_nth[5]);
	ReadRegister(KME_LOGO1_KME_LOGO1_18_reg,  24,  27,  &pOutput->DefaultVal.blkhsty_nth[6]);
	ReadRegister(KME_LOGO1_KME_LOGO1_18_reg,  28,  31,  &pOutput->DefaultVal.blkhsty_nth[7]);
	ReadRegister(KME_LOGO1_KME_LOGO1_1C_reg,  0,  3,  &pOutput->DefaultVal.blkhsty_nth[8]);
	ReadRegister(KME_LOGO1_KME_LOGO1_1C_reg,  4,  7,  &pOutput->DefaultVal.blkhsty_nth[9]);
	ReadRegister(KME_LOGO1_KME_LOGO1_1C_reg, 8, 11, &pOutput->DefaultVal.blkhsty_nth[10]);
	ReadRegister(KME_LOGO1_KME_LOGO1_1C_reg, 12, 15, &pOutput->DefaultVal.blkhsty_nth[11]);
	ReadRegister(KME_LOGO1_KME_LOGO1_1C_reg, 16, 19, &pOutput->DefaultVal.blkhsty_nth[12]);
	ReadRegister(KME_LOGO1_KME_LOGO1_1C_reg, 20, 23, &pOutput->DefaultVal.blkhsty_nth[13]);
	ReadRegister(KME_LOGO1_KME_LOGO1_1C_reg, 24, 27, &pOutput->DefaultVal.blkhsty_nth[14]);

	ReadRegister(KME_LOGO0_KME_LOGO0_A4_reg,  0,  7,  &pOutput->DefaultVal.blksamethr_l[0]);
	ReadRegister(KME_LOGO0_KME_LOGO0_A4_reg,  8,  15,  &pOutput->DefaultVal.blksamethr_l[1]);
	ReadRegister(KME_LOGO0_KME_LOGO0_A4_reg,  16,  23,  &pOutput->DefaultVal.blksamethr_l[2]);
	ReadRegister(KME_LOGO0_KME_LOGO0_A4_reg,  24,  31,  &pOutput->DefaultVal.blksamethr_l[3]);
	ReadRegister(KME_LOGO0_KME_LOGO0_A8_reg,  0,  7,  &pOutput->DefaultVal.blksamethr_l[4]);
	ReadRegister(KME_LOGO0_KME_LOGO0_A8_reg,  8,  15,  &pOutput->DefaultVal.blksamethr_l[5]);
	ReadRegister(KME_LOGO0_KME_LOGO0_A8_reg,  16,  23,  &pOutput->DefaultVal.blksamethr_l[6]);
	ReadRegister(KME_LOGO0_KME_LOGO0_A8_reg,  24,  31,  &pOutput->DefaultVal.blksamethr_l[7]);
	ReadRegister(KME_LOGO0_KME_LOGO0_AC_reg,  0,  7,  &pOutput->DefaultVal.blksamethr_l[8]);
	ReadRegister(KME_LOGO0_KME_LOGO0_AC_reg,  8,  15,  &pOutput->DefaultVal.blksamethr_l[9]);
	ReadRegister(KME_LOGO0_KME_LOGO0_AC_reg, 16, 23, &pOutput->DefaultVal.blksamethr_l[10]);
	ReadRegister(KME_LOGO0_KME_LOGO0_AC_reg, 24, 31, &pOutput->DefaultVal.blksamethr_l[11]);
	ReadRegister(KME_LOGO0_KME_LOGO0_B0_reg, 0, 7, &pOutput->DefaultVal.blksamethr_l[12]);
	ReadRegister(KME_LOGO0_KME_LOGO0_B0_reg, 8, 15, &pOutput->DefaultVal.blksamethr_l[13]);
	ReadRegister(KME_LOGO0_KME_LOGO0_B0_reg, 16, 23, &pOutput->DefaultVal.blksamethr_l[14]);

	ReadRegister(KME_LOGO0_KME_LOGO0_C4_reg,  0,  7,  &pOutput->DefaultVal.blksamethr_a[0]);
	ReadRegister(KME_LOGO0_KME_LOGO0_C4_reg,  8,  15,  &pOutput->DefaultVal.blksamethr_a[1]);
	ReadRegister(KME_LOGO0_KME_LOGO0_C4_reg,  16,  23,  &pOutput->DefaultVal.blksamethr_a[2]);
	ReadRegister(KME_LOGO0_KME_LOGO0_C4_reg,  24,  31,  &pOutput->DefaultVal.blksamethr_a[3]);
	ReadRegister(KME_LOGO0_KME_LOGO0_C8_reg,  0,  7,  &pOutput->DefaultVal.blksamethr_a[4]);
	ReadRegister(KME_LOGO0_KME_LOGO0_C8_reg,  8,  15,  &pOutput->DefaultVal.blksamethr_a[5]);
	ReadRegister(KME_LOGO0_KME_LOGO0_C8_reg,  16,  23,  &pOutput->DefaultVal.blksamethr_a[6]);
	ReadRegister(KME_LOGO0_KME_LOGO0_C8_reg,  24,  31,  &pOutput->DefaultVal.blksamethr_a[7]);
	ReadRegister(KME_LOGO0_KME_LOGO0_CC_reg,  0,  7,  &pOutput->DefaultVal.blksamethr_a[8]);
	ReadRegister(KME_LOGO0_KME_LOGO0_CC_reg,  8,  15,  &pOutput->DefaultVal.blksamethr_a[9]);
	ReadRegister(KME_LOGO0_KME_LOGO0_CC_reg, 16, 23, &pOutput->DefaultVal.blksamethr_a[10]);
	ReadRegister(KME_LOGO0_KME_LOGO0_CC_reg, 24, 31, &pOutput->DefaultVal.blksamethr_a[11]);
	ReadRegister(KME_LOGO0_KME_LOGO0_D0_reg, 0, 7, &pOutput->DefaultVal.blksamethr_a[12]);
	ReadRegister(KME_LOGO0_KME_LOGO0_D0_reg, 8, 15, &pOutput->DefaultVal.blksamethr_a[13]);
	ReadRegister(KME_LOGO0_KME_LOGO0_D0_reg, 16, 23, &pOutput->DefaultVal.blksamethr_a[14]);

	ReadRegister(KME_LOGO0_KME_LOGO0_B4_reg,  0,  7,  &pOutput->DefaultVal.blksamethr_h[0]);
	ReadRegister(KME_LOGO0_KME_LOGO0_B4_reg,  8,  15,  &pOutput->DefaultVal.blksamethr_h[1]);
	ReadRegister(KME_LOGO0_KME_LOGO0_B4_reg,  16,  23,  &pOutput->DefaultVal.blksamethr_h[2]);
	ReadRegister(KME_LOGO0_KME_LOGO0_B4_reg,  24,  31,  &pOutput->DefaultVal.blksamethr_h[3]);
	ReadRegister(KME_LOGO0_KME_LOGO0_B8_reg,  0,  7,  &pOutput->DefaultVal.blksamethr_h[4]);
	ReadRegister(KME_LOGO0_KME_LOGO0_B8_reg,  8,  15,  &pOutput->DefaultVal.blksamethr_h[5]);
	ReadRegister(KME_LOGO0_KME_LOGO0_B8_reg,  16,  23,  &pOutput->DefaultVal.blksamethr_h[6]);
	ReadRegister(KME_LOGO0_KME_LOGO0_B8_reg,  24,  31,  &pOutput->DefaultVal.blksamethr_h[7]);
	ReadRegister(KME_LOGO0_KME_LOGO0_BC_reg,  0,  7,  &pOutput->DefaultVal.blksamethr_h[8]);
	ReadRegister(KME_LOGO0_KME_LOGO0_BC_reg,  8,  15,  &pOutput->DefaultVal.blksamethr_h[9]);
	ReadRegister(KME_LOGO0_KME_LOGO0_BC_reg, 16, 23, &pOutput->DefaultVal.blksamethr_h[10]);
	ReadRegister(KME_LOGO0_KME_LOGO0_BC_reg, 24, 31, &pOutput->DefaultVal.blksamethr_h[11]);
	ReadRegister(KME_LOGO0_KME_LOGO0_C0_reg, 0, 7, &pOutput->DefaultVal.blksamethr_h[12]);
	ReadRegister(KME_LOGO0_KME_LOGO0_C0_reg, 8, 15, &pOutput->DefaultVal.blksamethr_h[13]);
	ReadRegister(KME_LOGO0_KME_LOGO0_C0_reg, 16, 23, &pOutput->DefaultVal.blksamethr_h[14]);

	// Dehalo logo process default value
	ReadRegister(KME_DEHALO2_KME_DEHALO2_90_reg, 0, 2, &pOutput->DefaultVal.dh_logo_bypass[0][0]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_90_reg, 4, 6, &pOutput->DefaultVal.dh_logo_bypass[0][1]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_90_reg, 12, 14, &pOutput->DefaultVal.dh_logo_bypass[0][2]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_90_reg, 8, 10, &pOutput->DefaultVal.dh_logo_bypass[0][3]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_90_reg, 15, 15, &pOutput->DefaultVal.dh_logo_bypass[0][4]);

	ReadRegister(KME_DEHALO2_KME_DEHALO2_90_reg, 0, 2, &pOutput->DefaultVal.dh_logo_bypass[0][0]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_90_reg, 4, 6, &pOutput->DefaultVal.dh_logo_bypass[0][1]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_90_reg, 12, 14, &pOutput->DefaultVal.dh_logo_bypass[0][2]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_90_reg, 8, 10, &pOutput->DefaultVal.dh_logo_bypass[0][3]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_90_reg, 15, 15, &pOutput->DefaultVal.dh_logo_bypass[0][4]);

	ReadRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 1, 3, &pOutput->DefaultVal.dh_logo_bypass[1][0]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 4, 6, &pOutput->DefaultVal.dh_logo_bypass[1][1]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 10, 12, &pOutput->DefaultVal.dh_logo_bypass[1][2]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 7, 9, &pOutput->DefaultVal.dh_logo_bypass[1][3]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 0, 0, &pOutput->DefaultVal.dh_logo_bypass[1][4]);

	ReadRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 0, 2, &pOutput->DefaultVal.dh_logo_bypass[2][0]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 3, 5, &pOutput->DefaultVal.dh_logo_bypass[2][1]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 9, 11, &pOutput->DefaultVal.dh_logo_bypass[2][2]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 6, 8, &pOutput->DefaultVal.dh_logo_bypass[2][3]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 12, 12, &pOutput->DefaultVal.dh_logo_bypass[2][4]);

	ReadRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 0, 2, &pOutput->DefaultVal.dh_logo_bypass[3][0]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 3, 5, &pOutput->DefaultVal.dh_logo_bypass[3][1]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 9, 11, &pOutput->DefaultVal.dh_logo_bypass[3][2]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 6, 8, &pOutput->DefaultVal.dh_logo_bypass[3][3]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 12, 12, &pOutput->DefaultVal.dh_logo_bypass[3][4]);

	ReadRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 16, 18, &pOutput->DefaultVal.dh_logo_bypass[4][0]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 19, 21, &pOutput->DefaultVal.dh_logo_bypass[4][1]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 25, 27, &pOutput->DefaultVal.dh_logo_bypass[4][2]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 22, 24, &pOutput->DefaultVal.dh_logo_bypass[4][3]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 28, 28, &pOutput->DefaultVal.dh_logo_bypass[4][4]);

	ReadRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 0, 2, &pOutput->DefaultVal.dh_logo_bypass[5][0]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 3, 5, &pOutput->DefaultVal.dh_logo_bypass[5][1]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 9, 11, &pOutput->DefaultVal.dh_logo_bypass[5][2]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 6, 8, &pOutput->DefaultVal.dh_logo_bypass[5][3]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 12, 12, &pOutput->DefaultVal.dh_logo_bypass[5][4]);

	ReadRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 16, 18, &pOutput->DefaultVal.dh_logo_bypass[6][0]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 19, 21, &pOutput->DefaultVal.dh_logo_bypass[6][1]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 25, 27, &pOutput->DefaultVal.dh_logo_bypass[6][2]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 22, 24, &pOutput->DefaultVal.dh_logo_bypass[6][3]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 28, 28, &pOutput->DefaultVal.dh_logo_bypass[6][4]);

	ReadRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 0, 2, &pOutput->DefaultVal.dh_logo_bypass[7][0]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 3, 5, &pOutput->DefaultVal.dh_logo_bypass[7][1]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 9, 11, &pOutput->DefaultVal.dh_logo_bypass[7][2]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 6, 8, &pOutput->DefaultVal.dh_logo_bypass[7][3]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 12, 12, &pOutput->DefaultVal.dh_logo_bypass[7][4]);

	ReadRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 16, 18, &pOutput->DefaultVal.dh_logo_bypass[8][0]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 19, 21, &pOutput->DefaultVal.dh_logo_bypass[8][1]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 25, 27, &pOutput->DefaultVal.dh_logo_bypass[8][2]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 22, 24, &pOutput->DefaultVal.dh_logo_bypass[8][3]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 28, 28, &pOutput->DefaultVal.dh_logo_bypass[8][4]);

	ReadRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 0, 2, &pOutput->DefaultVal.dh_logo_bypass[9][0]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 3, 5, &pOutput->DefaultVal.dh_logo_bypass[9][1]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 9, 11, &pOutput->DefaultVal.dh_logo_bypass[9][2]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 6, 8, &pOutput->DefaultVal.dh_logo_bypass[9][3]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 12, 12, &pOutput->DefaultVal.dh_logo_bypass[9][4]);

	ReadRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 16, 18, &pOutput->DefaultVal.dh_logo_bypass[10][0]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 19, 21, &pOutput->DefaultVal.dh_logo_bypass[10][1]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 25, 27, &pOutput->DefaultVal.dh_logo_bypass[10][2]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 22, 24, &pOutput->DefaultVal.dh_logo_bypass[10][3]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 28, 28, &pOutput->DefaultVal.dh_logo_bypass[10][4]);

	ReadRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 0, 2, &pOutput->DefaultVal.dh_logo_bypass[11][0]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 3, 5, &pOutput->DefaultVal.dh_logo_bypass[11][1]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 9, 11, &pOutput->DefaultVal.dh_logo_bypass[11][2]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 6, 8, &pOutput->DefaultVal.dh_logo_bypass[11][3]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 12, 12, &pOutput->DefaultVal.dh_logo_bypass[11][4]);

	ReadRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 16, 18, &pOutput->DefaultVal.dh_logo_bypass[12][0]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 19, 21, &pOutput->DefaultVal.dh_logo_bypass[12][1]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 25, 27, &pOutput->DefaultVal.dh_logo_bypass[12][2]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 22, 24, &pOutput->DefaultVal.dh_logo_bypass[12][3]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 28, 28, &pOutput->DefaultVal.dh_logo_bypass[12][4]);

	ReadRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 0, 2, &pOutput->DefaultVal.dh_logo_bypass[13][0]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 3, 5, &pOutput->DefaultVal.dh_logo_bypass[13][1]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 9, 11, &pOutput->DefaultVal.dh_logo_bypass[13][2]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 6, 8, &pOutput->DefaultVal.dh_logo_bypass[13][3]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 12, 12, &pOutput->DefaultVal.dh_logo_bypass[13][4]);

	ReadRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 16, 18, &pOutput->DefaultVal.dh_logo_bypass[14][0]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 19, 21, &pOutput->DefaultVal.dh_logo_bypass[14][1]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 25, 27, &pOutput->DefaultVal.dh_logo_bypass[14][2]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 22, 24, &pOutput->DefaultVal.dh_logo_bypass[14][3]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 28, 28, &pOutput->DefaultVal.dh_logo_bypass[14][4]);

	// Clear threshold
	ReadRegister(KME_DEHALO2_KME_DEHALO2_84_reg,12,21, &pOutput->DefaultVal.dh_logo_RgnThr[0][0]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_84_reg,6,11, &pOutput->DefaultVal.dh_logo_RgnThr[1][0]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_84_reg,0,5, &pOutput->DefaultVal.dh_logo_RgnThr[2][0]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_A0_reg,16,25, &pOutput->DefaultVal.dh_logo_RgnThr[3][0]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_A0_reg,0,7, &pOutput->DefaultVal.dh_logo_RgnThr[4][0]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_A0_reg,8,15, &pOutput->DefaultVal.dh_logo_RgnThr[5][0]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_A0_reg,26,27, &pOutput->DefaultVal.dh_logo_RgnThr[6][0]);

	ReadRegister(KME_DEHALO2_KME_DEHALO2_AC_reg,12,21, &pOutput->DefaultVal.dh_logo_RgnThr[0][1]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_AC_reg,6,11, &pOutput->DefaultVal.dh_logo_RgnThr[1][1]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_AC_reg,0,5, &pOutput->DefaultVal.dh_logo_RgnThr[2][1]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_A4_reg,16,25, &pOutput->DefaultVal.dh_logo_RgnThr[3][1]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_A4_reg,0,7, &pOutput->DefaultVal.dh_logo_RgnThr[4][1]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_A4_reg,8,15, &pOutput->DefaultVal.dh_logo_RgnThr[5][1]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_A4_reg,26,27, &pOutput->DefaultVal.dh_logo_RgnThr[6][1]);

	ReadRegister(KME_DEHALO2_KME_DEHALO2_08_reg,0,9, &pOutput->DefaultVal.dh_logo_RgnThr[0][2]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_E8_reg,19,24, &pOutput->DefaultVal.dh_logo_RgnThr[1][2]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_E8_reg,13,18, &pOutput->DefaultVal.dh_logo_RgnThr[2][2]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_10_reg,8,17, &pOutput->DefaultVal.dh_logo_RgnThr[3][2]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_08_reg,10,17, &pOutput->DefaultVal.dh_logo_RgnThr[4][2]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_10_reg,0,7, &pOutput->DefaultVal.dh_logo_RgnThr[5][2]);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_E8_reg,25,26, &pOutput->DefaultVal.dh_logo_RgnThr[6][2]);

	// OBME pattern selection
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,22,22, &pOutput->DefaultVal.me1_ip_dc_obme_mode_sel);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,18,19, &pOutput->DefaultVal.me1_ip_dc_obme_mode);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,24,25, &pOutput->DefaultVal.me1_ip_ac_obme_mode);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,20,21, &pOutput->DefaultVal.me1_pi_dc_obme_mode_sel);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,20,21, &pOutput->DefaultVal.me1_pi_dc_obme_mode);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,26,27, &pOutput->DefaultVal.me1_pi_ac_obme_mode);
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_DC_reg,2,2, &pOutput->DefaultVal.me2_1st_dc_obme_mode_sel);
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg,28,29, &pOutput->DefaultVal.me2_1st_dc_obme_mode);
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg,21,22, &pOutput->DefaultVal.me2_1st_ac_obme_mode);
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_DC_reg,3,3, &pOutput->DefaultVal.me2_2nd_dc_obme_mode_sel);
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg,30,31, &pOutput->DefaultVal.me2_2nd_dc_obme_mode);
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg,23,24, &pOutput->DefaultVal.me2_2nd_ac_obme_mode);

	// sld need only clear block logo result when necessary, use erosion threshold to do it
	ReadRegister(KME_LOGO2_KME_LOGO2_28_reg, 9, 14, &pOutput->DefaultVal.u6_logo_blklogopostdlt_th);
	ReadRegister(KME_LOGO2_KME_LOGO2_28_reg, 16, 21, &pOutput->DefaultVal.u6_logo_blklogopost_ero_th);
	ReadRegister(KME_LOGO2_KME_LOGO2_2C_reg, 9, 16, &pOutput->DefaultVal.u6_logo_pxllogopostdlt_th);
}

VOID FRC_LgDet_BlkClrCtrl(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	unsigned int idx;
	unsigned int acc_blk_num;
	unsigned int acc_left_blk_num = 0;
	unsigned int acc_right_blk_num = 0;
	unsigned int acc_top_blk_num = 0;
	unsigned int acc_bottom_blk_num = 0;
	signed short*  hold_cnt_rg  = pOutput->s16_blklogo_rgclr_cnt;

	acc_blk_num = 0;
	for (idx = 0; idx < LOGO_RG_32; idx++)
	{
		unsigned int blklogo_rg_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[idx];
		int u8_clr_rg_thr;
		u8_clr_rg_thr = ((idx/8 )>= 1 && idx/8 <= 2)? pParam->u8_clr_rg_thr_l : pParam->u8_clr_rg_thr  ;
		if ((((blklogo_rg_cnt * 256) / (BLK_LOGO_RG_BLKCNT)) >= u8_clr_rg_thr) && pParam->u1_logo_rg_clr_en == 1)
		{
			pOutput->u32_blklogo_clr_idx = pOutput->u32_blklogo_clr_idx | (1 << idx);
			hold_cnt_rg[idx] = pParam->u6_rgClr_holdtime;
		}
		else if ((hold_cnt_rg[idx] > 1)  && pParam->u1_logo_rg_clr_en == 1)
		{
			pOutput->u32_blklogo_clr_idx = pOutput->u32_blklogo_clr_idx | (1 << idx);
			hold_cnt_rg[idx] = _MAX_(hold_cnt_rg[idx] - 1,0);
		}

		acc_blk_num += blklogo_rg_cnt;

		if (idx < 16)
		{
			acc_top_blk_num    += blklogo_rg_cnt;
		}
		else
		{
			acc_bottom_blk_num += blklogo_rg_cnt;
		}
		if (idx%8 <4)
		{
			acc_left_blk_num  += blklogo_rg_cnt;
		}
		else
		{
			acc_right_blk_num += blklogo_rg_cnt;
		}
	}

	//rtd_pr_memc_notice("[%s][%d][%d,%d,%d,%d,%d]\n",__FUNCTION__,pOutput->u1_logo_static_status, acc_blk_num,acc_left_blk_num,acc_right_blk_num,acc_top_blk_num,acc_bottom_blk_num);
	
	if ((((((acc_blk_num * 256) / (BLK_LOGO_BLKCNT)) >= pParam->u8_clr_glb_thr) && pParam->u1_logo_glb_clr_en == 1)
		|| ((((acc_left_blk_num * 256) / (BLK_LOGO_BLKCNT/2)) >= pParam->u8_clr_half_thr) && (pParam->u1_logo_left_half_clr_en  == 1))
		|| ((((acc_right_blk_num * 256) / (BLK_LOGO_BLKCNT/2)) >= pParam->u8_clr_half_thr) && (pParam->u1_logo_right_half_clr_en  == 1))
		|| ((((acc_top_blk_num * 256) / (BLK_LOGO_BLKCNT/2)) >= pParam->u8_clr_half_thr) && (pParam->u1_logo_top_half_clr_en  == 1))
		|| ((((acc_bottom_blk_num * 256) / (BLK_LOGO_BLKCNT/2)) >= pParam->u8_clr_half_thr) && (pParam->u1_logo_bot_half_clr_en  == 1)))
		&& ( pOutput->u1_logo_static_status != 0))
	{
		pOutput->u32_blklogo_clr_idx    = /*pOutput->u32_blklogo_clr_idx |*/ 0xFFFFFFFF;
		pOutput->s16_blklogo_glbclr_cnt = pParam->u6_glbClr_holdtime;
		pOutput->u1_blkclr_glbstatus = 1;
		pOutput->u1_logo_cnt_glb_clr_status = 1;
	}
	else if (pOutput->s16_blklogo_glbclr_cnt > 1 &&  pParam->u1_logo_glb_clr_en == 1)
	{

		pOutput->u32_blklogo_clr_idx    = /*pOutput->u32_blklogo_clr_idx |*/ 0xFFFFFFFF;
		pOutput->s16_blklogo_glbclr_cnt = _MAX_(pOutput->s16_blklogo_glbclr_cnt - 1,0);
		pOutput->u1_blkclr_glbstatus = 1;
		pOutput->u1_logo_cnt_glb_clr_status = 1;
	}
	else
	{
		pOutput->u32_blklogo_clr_idx = 0;
		pOutput->s16_blklogo_glbclr_cnt = 0;
		pOutput->u1_blkclr_glbstatus = 0;
		pOutput->u1_logo_cnt_glb_clr_status = 0;
	}
}

//current, use blk_logo idx to clear SC logo (global all)
//default is logoClr=1, holdHsty=1, clrAlpha=1
VOID FRC_LgDet_SCCtrl(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	pOutput->u5_iir_alpha_out = pParam->u5_iir_alpha;
	if (s_pContext->_output_read_comreg.u1_sc_status_logo_rb == 1)
	{
		if (pOutput->u1_logo_netflix_status == 0 && pParam->u1_logo_sc_logo_clr_en == 1) //sc signal is consider holdtime.
		{
			pOutput->u32_blklogo_clr_idx = /*pOutput->u32_blklogo_clr_idx |*/ 0xFFFFFFFF;
			pOutput->u1_blkclr_glbstatus = 1;
			pOutput->u1_logo_sc_glb_clr_status = 1;
		}
		if (pParam->u1_logo_sc_alpha_ctrl_en == 1)
		{
			pOutput->u5_iir_alpha_out = 0;
		}
	}
	else
	{	if (pParam->u1_logo_sc_alpha_ctrl_en == 1)
		{
			pOutput->u5_iir_alpha_out = pParam->u5_iir_alpha;
		}
	}
}

VOID FRC_LgDet_SC_FastDetectionCtrl(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	signed short u11_gmv_mvx = (s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb)>>1;
	signed short u10_gmv_mvy = (s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb)>>1;

	if(pParam->u1_logo_sc_FastDet_en == 1)
	{
		if (s_pContext->_output_read_comreg.u1_sc_status_logo_rb == 1)
		{			
			pOutput->u8_logo_sc_FastDet_cntholdfrm = pParam->u8_logo_sc_FastDet_rstholdfrm ;
		}
		else			
		{
			if(s_pContext->_output_me_sceneAnalysis.u2_panning_flag == 1 && ( _ABS_(u11_gmv_mvx)>10 || _ABS_(u10_gmv_mvy)>10 )){
				if(pOutput->u8_logo_sc_FastDet_cntholdfrm>2){
					pOutput->u8_logo_sc_FastDet_cntholdfrm = 5;
				}
				pOutput->u8_logo_sc_FastDet_cntholdfrm = (pOutput->u8_logo_sc_FastDet_cntholdfrm > 0)? pOutput->u8_logo_sc_FastDet_cntholdfrm - 1 : 0;
			}else{
				if(( _ABS_(u11_gmv_mvx)>10 || _ABS_(u10_gmv_mvy)>10 )){
				pOutput->u8_logo_sc_FastDet_cntholdfrm = (pOutput->u8_logo_sc_FastDet_cntholdfrm > 0)? pOutput->u8_logo_sc_FastDet_cntholdfrm - 1 : 0;
				}else{
					if(pOutput->u8_logo_sc_FastDet_cntholdfrm>7){
						pOutput->u8_logo_sc_FastDet_cntholdfrm = (pOutput->u8_logo_sc_FastDet_cntholdfrm > 0)? pOutput->u8_logo_sc_FastDet_cntholdfrm - 7 : 0;
					}else{
						pOutput->u8_logo_sc_FastDet_cntholdfrm = (pOutput->u8_logo_sc_FastDet_cntholdfrm > 0)? pOutput->u8_logo_sc_FastDet_cntholdfrm - 1 : 0;	
					}
				}
			}
		}

		if( pOutput->u8_logo_sc_FastDet_cntholdfrm > 0 )
			pOutput->u1_logo_sc_FastDet_status = 1;
		else
			pOutput->u1_logo_sc_FastDet_status = 0;
	}
	else
	{
		pOutput->u1_logo_sc_FastDet_status = 0;
		pOutput->u8_logo_sc_FastDet_cntholdfrm = 0;
	}
}

/// bai change
//VOID FRC_LgDet_RgDhClr(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
//{
//	const _PQLCONTEXT *s_pContext = GetPQLContext();
//	unsigned int rg_tl_blk_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[0] + s_pContext->_output_read_comreg.u10_blklogo_rgcnt[1];
//	unsigned int rg_tr_blk_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[6] + s_pContext->_output_read_comreg.u10_blklogo_rgcnt[7];
//	unsigned int rg_bl_blk_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[24] + s_pContext->_output_read_comreg.u10_blklogo_rgcnt[25];
//	unsigned int rg_br_blk_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[30] + s_pContext->_output_read_comreg.u10_blklogo_rgcnt[31];
//
//	unsigned int rg_tl_blk_logo_dh_clr_cnt = (rg_tl_blk_logo_cnt >= s_pContext->_output_read_comreg.u16_blklogo_dh_clrcnt[0])?(rg_tl_blk_logo_cnt - s_pContext->_output_read_comreg.u16_blklogo_dh_clrcnt[0]) : 0;
//	unsigned int rg_tr_blk_logo_dh_clr_cnt = (rg_tr_blk_logo_cnt >= s_pContext->_output_read_comreg.u16_blklogo_dh_clrcnt[1])?(rg_tr_blk_logo_cnt - s_pContext->_output_read_comreg.u16_blklogo_dh_clrcnt[1]) : 0;
//	unsigned int rg_bl_blk_logo_dh_clr_cnt = (rg_bl_blk_logo_cnt >= s_pContext->_output_read_comreg.u16_blklogo_dh_clrcnt[2])?(rg_bl_blk_logo_cnt - s_pContext->_output_read_comreg.u16_blklogo_dh_clrcnt[2]) : 0;
//	unsigned int rg_br_blk_logo_dh_clr_cnt = (rg_br_blk_logo_cnt >= s_pContext->_output_read_comreg.u16_blklogo_dh_clrcnt[3])?(rg_br_blk_logo_cnt - s_pContext->_output_read_comreg.u16_blklogo_dh_clrcnt[3]) : 0;
//
//	unsigned int rg_logo_diff_prm  = pParam->u16_RgDhClr_thr_h - pParam->u16_RgDhClr_thr_l;
//	unsigned int rg_logo_slope      = rg_logo_diff_prm <= 0 ? 0 : 16 * 255 / rg_logo_diff_prm;
//
//	unsigned char  rg_tl_blk_logo_alp = _IncreaseCurveMapping(rg_tl_blk_logo_dh_clr_cnt, pParam->u16_RgDhClr_thr_l, 0, 16, rg_logo_slope, 8);
//	unsigned char  rg_tr_blk_logo_alp = _IncreaseCurveMapping(rg_tr_blk_logo_dh_clr_cnt, pParam->u16_RgDhClr_thr_l, 0, 16, rg_logo_slope, 8);
//	unsigned char  rg_bl_blk_logo_alp = _IncreaseCurveMapping(rg_bl_blk_logo_dh_clr_cnt, pParam->u16_RgDhClr_thr_l, 0, 16, rg_logo_slope, 8);
//	unsigned char  rg_br_blk_logo_alp = _IncreaseCurveMapping(rg_br_blk_logo_dh_clr_cnt, pParam->u16_RgDhClr_thr_l, 0, 16, rg_logo_slope, 8);
//
//
//	if( pParam->u1_RgDhClr_thr_en == 1)
//	{
//		pOutput->u1_rg_dh_clr_rg0 = (rg_tl_blk_logo_alp>=pParam->u5_RgDhClr_cnt_th) ? 1:0;
//		pOutput->u1_rg_dh_clr_rg1 = (rg_tr_blk_logo_alp>=pParam->u5_RgDhClr_cnt_th) ? 1:0;
//		pOutput->u1_rg_dh_clr_rg2 = (rg_bl_blk_logo_alp>=pParam->u5_RgDhClr_cnt_th) ? 1:0;
//		pOutput->u1_rg_dh_clr_rg3 = (rg_br_blk_logo_alp>=pParam->u5_RgDhClr_cnt_th) ? 1:0;
//	}
//	else
//	{
//		pOutput->u1_rg_dh_clr_rg0 =  0;
//		pOutput->u1_rg_dh_clr_rg1 =  0;
//		pOutput->u1_rg_dh_clr_rg2 =  0;
//		pOutput->u1_rg_dh_clr_rg3 =  0;
//	}
//
//	if (pOutput->u32_blklogo_clr_idx == 0xFFFFFFFF)
//	{
//		pOutput->u1_rg_dh_clr_rg0 =  0;
//		pOutput->u1_rg_dh_clr_rg1 =  0;
//		pOutput->u1_rg_dh_clr_rg2 =  0;
//		pOutput->u1_rg_dh_clr_rg3 =  0;
//
//		pOutput->u8_rg_dh_clr_lgclr_hold_frm = pParam->u8_rg_dh_clr_lgclr_hold_frm;
//	}
//	else if (pOutput->u8_rg_dh_clr_lgclr_hold_frm > 0)
//	{
//		pOutput->u1_rg_dh_clr_rg0 =  0;
//		pOutput->u1_rg_dh_clr_rg1 =  0;
//		pOutput->u1_rg_dh_clr_rg2 =  0;
//		pOutput->u1_rg_dh_clr_rg3 =  0;
//		pOutput->u8_rg_dh_clr_lgclr_hold_frm = pOutput->u8_rg_dh_clr_lgclr_hold_frm - 1;
//	}
//	else
//	{
//		pOutput->u8_rg_dh_clr_lgclr_hold_frm = 0;
//	}
//
//    if (pParam->u1_RgDhClr_static_en == 1)
//    {
//		if (pOutput->u1_logo_static_status == 1)
//		{
//			pOutput->u1_rg_dh_clr_rg0 =  0;
//			pOutput->u1_rg_dh_clr_rg1 =  0;
//			pOutput->u1_rg_dh_clr_rg2 =  0;
//			pOutput->u1_rg_dh_clr_rg3 =  0;
//			pOutput->u8_rg_dh_clr_static_hold_frm = pParam->u8_rg_dh_clr_static_hold_frm;
//		}
//		else if (pOutput->u8_rg_dh_clr_static_hold_frm > 0)
//		{
//			pOutput->u1_rg_dh_clr_rg0 =  0;
//			pOutput->u1_rg_dh_clr_rg1 =  0;
//			pOutput->u1_rg_dh_clr_rg2 =  0;
//			pOutput->u1_rg_dh_clr_rg3 =  0;
//			pOutput->u8_rg_dh_clr_static_hold_frm = pOutput->u8_rg_dh_clr_static_hold_frm - 1;
//		}
//		else
//		{
//			pOutput->u8_rg_dh_clr_static_hold_frm = 0;
//		}
//    }
//}

VOID FRC_LgDet_RgDhClr(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned int u16_tl_dh_clrcnt;
	unsigned int u16_tr_dh_clrcnt;
	unsigned int u16_bl_dh_clrcnt;
	unsigned int u16_br_dh_clrcnt;

	unsigned int rg_tl_blk_logo_dh_clr_cnt;
	unsigned int rg_tr_blk_logo_dh_clr_cnt;
	unsigned int rg_bl_blk_logo_dh_clr_cnt;
	unsigned int rg_br_blk_logo_dh_clr_cnt;

	unsigned int rg_tl_blk_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[0] + s_pContext->_output_read_comreg.u10_blklogo_rgcnt[1];
	unsigned int rg_tr_blk_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[6] + s_pContext->_output_read_comreg.u10_blklogo_rgcnt[7];
	unsigned int rg_bl_blk_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[24] + s_pContext->_output_read_comreg.u10_blklogo_rgcnt[25];
	unsigned int rg_br_blk_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[30] + s_pContext->_output_read_comreg.u10_blklogo_rgcnt[31];

	if (s_pContext->_output_wrt_comreg.u1_LG_mb_wrt_en == 1)
	{
		u16_tl_dh_clrcnt = s_pContext->_output_read_comreg.u16_blklogo_dh_clrcnt[0] - s_pContext->_output_read_comreg.u16_blklogo_dh_clrcnt_top[2];
		u16_tr_dh_clrcnt = s_pContext->_output_read_comreg.u16_blklogo_dh_clrcnt[1] - s_pContext->_output_read_comreg.u16_blklogo_dh_clrcnt_top[2];
		u16_bl_dh_clrcnt = s_pContext->_output_read_comreg.u16_blklogo_dh_clrcnt[2] - s_pContext->_output_read_comreg.u16_blklogo_dh_clrcnt_bot[2];
		u16_br_dh_clrcnt = s_pContext->_output_read_comreg.u16_blklogo_dh_clrcnt[3] - s_pContext->_output_read_comreg.u16_blklogo_dh_clrcnt_bot[2];
	}
	else
	{
		u16_tl_dh_clrcnt = 	s_pContext->_output_read_comreg.u16_blklogo_dh_clrcnt[0];
		u16_tr_dh_clrcnt = 	s_pContext->_output_read_comreg.u16_blklogo_dh_clrcnt[1];
		u16_bl_dh_clrcnt = 	s_pContext->_output_read_comreg.u16_blklogo_dh_clrcnt[2];
		u16_br_dh_clrcnt = 	s_pContext->_output_read_comreg.u16_blklogo_dh_clrcnt[3];
	}

	rg_tl_blk_logo_dh_clr_cnt = (rg_tl_blk_logo_cnt >= u16_tl_dh_clrcnt)?(rg_tl_blk_logo_cnt - u16_tl_dh_clrcnt) : 0;
	rg_tr_blk_logo_dh_clr_cnt = (rg_tr_blk_logo_cnt >= u16_tr_dh_clrcnt)?(rg_tr_blk_logo_cnt - u16_tr_dh_clrcnt) : 0;
	rg_bl_blk_logo_dh_clr_cnt = (rg_bl_blk_logo_cnt >= u16_bl_dh_clrcnt)?(rg_bl_blk_logo_cnt - u16_bl_dh_clrcnt) : 0;
	rg_br_blk_logo_dh_clr_cnt = (rg_br_blk_logo_cnt >= u16_br_dh_clrcnt)?(rg_br_blk_logo_cnt - u16_br_dh_clrcnt) : 0;


	if( pParam->u1_RgDhClr_thr_en == 1)
	{
		pOutput->u1_rg_dh_clr_rg0 = (rg_tl_blk_logo_dh_clr_cnt>=pParam->u16_RgDhClr_thr_l && rg_tl_blk_logo_dh_clr_cnt<=pParam->u16_RgDhClr_thr_h) ? 1:0;
		pOutput->u1_rg_dh_clr_rg1 = (rg_tr_blk_logo_dh_clr_cnt>=pParam->u16_RgDhClr_thr_l && rg_tl_blk_logo_dh_clr_cnt<=pParam->u16_RgDhClr_thr_h) ? 1:0;
		pOutput->u1_rg_dh_clr_rg2 = (rg_bl_blk_logo_dh_clr_cnt>=pParam->u16_RgDhClr_thr_l && rg_tl_blk_logo_dh_clr_cnt<=pParam->u16_RgDhClr_thr_h) ? 1:0;
		pOutput->u1_rg_dh_clr_rg3 = (rg_br_blk_logo_dh_clr_cnt>=pParam->u16_RgDhClr_thr_l && rg_tl_blk_logo_dh_clr_cnt<=pParam->u16_RgDhClr_thr_h) ? 1:0;
	}
	else
	{
		pOutput->u1_rg_dh_clr_rg0 =  0;
		pOutput->u1_rg_dh_clr_rg1 =  0;
		pOutput->u1_rg_dh_clr_rg2 =  0;
		pOutput->u1_rg_dh_clr_rg3 =  0;
	}

	if (pOutput->u32_blklogo_clr_idx == 0xFFFFFFFF)
	{
		pOutput->u1_rg_dh_clr_rg0 =  0;
		pOutput->u1_rg_dh_clr_rg1 =  0;
		pOutput->u1_rg_dh_clr_rg2 =  0;
		pOutput->u1_rg_dh_clr_rg3 =  0;

		pOutput->u8_rg_dh_clr_lgclr_hold_frm = pParam->u8_rg_dh_clr_lgclr_hold_frm;
	}
	else if (pOutput->u8_rg_dh_clr_lgclr_hold_frm > 0)
	{
		pOutput->u1_rg_dh_clr_rg0 =  0;
		pOutput->u1_rg_dh_clr_rg1 =  0;
		pOutput->u1_rg_dh_clr_rg2 =  0;
		pOutput->u1_rg_dh_clr_rg3 =  0;
		pOutput->u8_rg_dh_clr_lgclr_hold_frm = pOutput->u8_rg_dh_clr_lgclr_hold_frm - 1;
	}
	else
	{
		pOutput->u8_rg_dh_clr_lgclr_hold_frm = 0;
	}

    if (pParam->u1_RgDhClr_static_en == 1)
    {
		if (pOutput->u1_logo_static_status == 1)
		{
			pOutput->u1_rg_dh_clr_rg0 =  0;
			pOutput->u1_rg_dh_clr_rg1 =  0;
			pOutput->u1_rg_dh_clr_rg2 =  0;
			pOutput->u1_rg_dh_clr_rg3 =  0;
			pOutput->u8_rg_dh_clr_static_hold_frm = pParam->u8_rg_dh_clr_static_hold_frm;
		}
		else if (pOutput->u8_rg_dh_clr_static_hold_frm > 0)
		{
			pOutput->u1_rg_dh_clr_rg0 =  0;
			pOutput->u1_rg_dh_clr_rg1 =  0;
			pOutput->u1_rg_dh_clr_rg2 =  0;
			pOutput->u1_rg_dh_clr_rg3 =  0;
			pOutput->u8_rg_dh_clr_static_hold_frm = pOutput->u8_rg_dh_clr_static_hold_frm - 1;
		}
		else
		{
			pOutput->u8_rg_dh_clr_static_hold_frm = 0;
		}
    }
}
VOID FRC_LgDet_RgHstyClr(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
#if 1
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned int u16_cnt_totalNum1 = (s_pContext->_output_read_comreg.u8_me1_Vbuf_Hsize_rb * s_pContext->_output_read_comreg.u8_me1_Vbuf_Vsize_rb);
	unsigned char u1_sc_stat = s_pContext->_output_read_comreg.u1_sc_status_logo_rb;
	unsigned int u11_Gmv_mvx = _ABS_(s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb);
	unsigned int u10_Gmv_mvy = _ABS_(s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb);

	//horizatal_speed = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
	//vertical_speed  = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;
	#if 0
	rtd_pr_memc_notice("[logo][GMV]->%d,%d,%d,%d\r\n",u11_Gmv_mvx,u10_Gmv_mvy,s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb,s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb);
	rtd_pr_memc_notice("[logo][GMV]sad->%d,%d\r\n",s_pContext->_output_read_comreg.u30_me_aSAD_rb,u16_cnt_totalNum1);
	#endif	

	//rtd_pr_memc_notice("[%s][%d,%d,%d,%d]\n",__FUNCTION__,pParam->u1_GlbHstyClr_en, pParam->u1_RgHstyClr_Sc_en,u1_sc_stat,pParam->u1_RgHstyClr_Sc_en);

	if (pParam->u1_GlbHstyClr_en == 1 && ((pParam->u1_RgHstyClr_Sc_en == 1 && u1_sc_stat == 1) || (pParam->u1_RgHstyClr_Sc_en == 0)))
	{
		//rtd_pr_memc_notice("[%s][%d,%d,%d,%d]\n",__FUNCTION__,(u11_Gmv_mvx + u10_Gmv_mvy), s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb,s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb,s_pContext->_output_read_comreg.u30_me_aSAD_rb);
		
		if ((u11_Gmv_mvx + u10_Gmv_mvy) < pParam->u16_RgHstyClr_Gmv_thr &&
			s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb > pParam->u8_RgHstyClr_Gmv_cnt_thr &&
			s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb < pParam->u12_RgHstyClr_Gmv_unconf_thr &&
			s_pContext->_output_read_comreg.u30_me_aSAD_rb/u16_cnt_totalNum1 < pParam->u10_RgHstyClr_Gsad_thr)
		{
			pOutput->u32_logo_hsty_clr_idx = /*pOutput->u32_logo_hsty_clr_idx |*/ 0xFFFFFFFF;
			pOutput->u1_logo_static_status = 1;
		}
		else
		{
			pOutput->u1_logo_static_status = 0;
		}

	}
	#if 0
	rtd_pr_memc_notice("[logo][GMV]static_status->%d\r\n",pOutput->u1_logo_static_status);
	#endif
#else
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned int idx;
	unsigned int u16_cnt_totalNum1 = (s_pContext->_output_read_comreg.u8_me1_Vbuf_Hsize_rb * s_pContext->_output_read_comreg.u8_me1_Vbuf_Vsize_rb);
	unsigned int u16_cnt_totalNum = u16_cnt_totalNum1 == 0 ? 1 : u16_cnt_totalNum1/32;
	unsigned char u1_sc_stat = s_pContext->_output_read_comreg.u1_sc_status_logo_rb;
	unsigned int u11_Gmv_mvx = _ABS_(s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb);
	unsigned int u10_Gmv_mvy = _ABS_(s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb);
	unsigned int u11_Rgmv_mvx;
	unsigned int u10_Rgmv_mvy;
	unsigned int u11_Rgmv2_mvx;
	unsigned int u10_Rgmv2_mvy;
	unsigned int u32_subtitle_rgn = 0;
	unsigned int u32_Rg_static_idx = 0;
	unsigned int u32_Rg_unstatic_idx = 0;
	unsigned char  u8_Rg_static_num = 0;
	if (pParam->u1_GlbHstyClr_en == 1 && ((pParam->u1_RgHstyClr_Sc_en == 1 && u1_sc_stat == 1) || (pParam->u1_RgHstyClr_Sc_en == 0)))
	{
		if ((u11_Gmv_mvx + u10_Gmv_mvy) < pParam->u16_RgHstyClr_Gmv_thr &&
			s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb > pParam->u8_RgHstyClr_Gmv_cnt_thr &&
			s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb < pParam->u12_RgHstyClr_Gmv_unconf_thr &&
			s_pContext->_output_read_comreg.u30_me_aSAD_rb/u16_cnt_totalNum1 < pParam->u10_RgHstyClr_Gsad_thr)
		{
			pOutput->u32_logo_hsty_clr_idx = /*pOutput->u32_logo_hsty_clr_idx |*/ 0xFFFFFFFF;
			pOutput->u1_logo_static_status = 1;
		}
		else
		{
			pOutput->u1_logo_static_status = 0;
		}

	}

	for (idx = 0; idx < LOGO_RG_32; idx++)
	{
		u11_Rgmv_mvx = _ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[idx]);
		u10_Rgmv_mvy = _ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[idx]);
		if ((u11_Rgmv_mvx + u10_Rgmv_mvy) < pParam->u16_RgHstyClr_mv_thr &&
			s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[idx] > pParam->u12_RgHstyClr_mv_cnt_thr &&
			s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[idx] < pParam->u12_RgHstyClr_mv_unconf_thr &&
			s_pContext->_output_read_comreg.u25_me_rSAD_rb[idx]/u16_cnt_totalNum < pParam->u10_RgHstyClr_Rsad_thr)
		{
			u32_Rg_static_idx = u32_Rg_static_idx | (1 << idx);
			u8_Rg_static_num = u8_Rg_static_num + 1;
		}
		else
		{
			u32_Rg_unstatic_idx = u32_Rg_unstatic_idx| (1 << idx);
		}

		//special for rgn26/27/28/29 - maybe subtitle.
		{
			unsigned char u1_subtitle_bmv_noclr_en = 1;
			u11_Rgmv2_mvx = _ABS_(s_pContext->_output_read_comreg.s11_me_rMV_2nd_vx_rb[idx]);
			u10_Rgmv2_mvy = _ABS_(s_pContext->_output_read_comreg.s10_me_rMV_2nd_vy_rb[idx]);
			if(idx >= 26 && idx <=29 && u1_subtitle_bmv_noclr_en)
			{
				if((u11_Rgmv_mvx + u10_Rgmv_mvy) > 30 ||(u11_Rgmv2_mvx + u10_Rgmv2_mvy) > 30
				)
				{
					u32_subtitle_rgn = u32_subtitle_rgn | (1 << idx);
				}

			}
		}
	}


	if (pParam->u1_logo_static_bg_en == 1)
	{
		u32_Rg_unstatic_idx = u32_Rg_unstatic_idx & 0x3CFFFF3C & (~u32_subtitle_rgn);
		pOutput->u8_Rg_static_num  = u8_Rg_static_num;
		pOutput->u32_Rg_unstatic_idx = ((u8_Rg_static_num > pParam->u6_logo_static_bg_th_l) && (u8_Rg_static_num < pParam->u6_logo_static_bg_th_h)) ? u32_Rg_unstatic_idx : 0;

	}else
	{
		pOutput->u8_Rg_static_num  = 0;
		pOutput->u32_Rg_unstatic_idx = 0;
	}

	if (pParam->u1_RgHstyClr_en == 1 && ((pParam->u1_RgHstyClr_Sc_en == 1 && u1_sc_stat == 1) || (pParam->u1_RgHstyClr_Sc_en == 0)))
	{
		for (idx = 0; idx < LOGO_RG_32; idx++)
		{
			u11_Rgmv_mvx = _ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[idx]);
			u10_Rgmv_mvy = _ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[idx]);
			if ((u11_Rgmv_mvx + u10_Rgmv_mvy) < pParam->u16_RgHstyClr_mv_thr &&
				s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[idx] > pParam->u12_RgHstyClr_mv_cnt_thr &&
				s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[idx] < pParam->u12_RgHstyClr_mv_unconf_thr &&
				s_pContext->_output_read_comreg.u25_me_rSAD_rb[idx]/u16_cnt_totalNum < pParam->u10_RgHstyClr_Rsad_thr)
			{
				pOutput->u32_logo_hsty_clr_idx = pOutput->u32_logo_hsty_clr_idx | (1 << idx);
			}
		}
	}
	if (pParam->u1_RgHstyClr_glbclr_close_en == 1 && pOutput->u1_logo_sc_glb_clr_status == 0 && pOutput->u1_logo_cnt_glb_clr_status == 1)
	{
		pOutput->u32_logo_hsty_clr_idx = 0;
	}
#endif
	pOutput->u32_logo_hsty_clr_lgclr_idx = pOutput->u32_logo_hsty_clr_idx;
}

VOID FRC_LgDet_SC_Detect(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
	unsigned int idx;
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	if (pParam->u1_logo_sc_detect_en)
	{

		unsigned char  u1_logo_sc_status;
		unsigned char  u1_logo_sc_dtldiff_status;
		unsigned int sad_diff = _MAX_(pOutput->u31_logo_sad_pre,s_pContext->_output_read_comreg.u30_me_aSAD_rb)-_MIN_(pOutput->u31_logo_sad_pre , s_pContext->_output_read_comreg.u30_me_aSAD_rb);
		unsigned char  u1_logo_sc_saddif_status = sad_diff > pParam->u31_logo_sc_saddiff_th ? 1 : 0;
		unsigned char  u1_logo_sc_Gmv_status;
		unsigned int reg_dtl_diff_sum = 0;

		if ((s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb < pParam->u6_logo_sc_Gmv_cnt_th) && (s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb >= pParam->u12_logo_sc_Gmv_unconf_th))
		{
			u1_logo_sc_Gmv_status = 1;
		}
		else
		{
			u1_logo_sc_Gmv_status = 0;
		}
		for (idx = 0; idx < LOGO_RG_32; idx++)
		{
			reg_dtl_diff_sum = reg_dtl_diff_sum + (_MAX_(s_pContext->_output_read_comreg.u20_me_rDTL_rb[idx],pOutput->u20_logo_dtl_pre[idx])- _MIN_(s_pContext->_output_read_comreg.u20_me_rDTL_rb[idx],pOutput->u20_logo_dtl_pre[idx]));
		}
		reg_dtl_diff_sum = reg_dtl_diff_sum >> 5;
		u1_logo_sc_dtldiff_status = reg_dtl_diff_sum > pParam->u20_logo_sc_dtldif_th ? 1 : 0;

		pOutput->u1_logo_sc_saddif_status = u1_logo_sc_saddif_status;
		pOutput->u1_logo_sc_Gmv_status    = u1_logo_sc_Gmv_status;
		pOutput->u1_logo_sc_dtldif_status = u1_logo_sc_dtldiff_status;

		if (pParam->u1_logo_sc_Gmv_en == 1)
		{
			u1_logo_sc_status = ((u1_logo_sc_saddif_status && pParam->u1_logo_sc_saddif_en) ||
				                 (u1_logo_sc_dtldiff_status && pParam->u1_logo_sc_dtldif_en)) && u1_logo_sc_Gmv_status;
		}
		else if (pParam->u1_logo_sc_saddif_en==1 && pParam->u1_logo_sc_dtldif_en ==1)
		{
			u1_logo_sc_status = u1_logo_sc_saddif_status && u1_logo_sc_dtldiff_status;
		}
		else
		{
			u1_logo_sc_status = (u1_logo_sc_saddif_status && pParam->u1_logo_sc_saddif_en) ||
				                (u1_logo_sc_dtldiff_status && pParam->u1_logo_sc_dtldif_en) ;
		}

		if (u1_logo_sc_status == 1)
		{
			pOutput->u1_logo_sc_status = 1;
			pOutput->u8_logo_sc_hold_frm = pParam->u8_logo_sc_hold_frm;
		}
		else if (pOutput->u8_logo_sc_hold_frm > 0)
		{
			pOutput->u1_logo_sc_status = 1;
			pOutput->u8_logo_sc_hold_frm = pOutput->u8_logo_sc_hold_frm - 1;
		}
		else
		{
			pOutput->u1_logo_sc_status = 0;
		}
	}
	else
	{
		pOutput->u1_logo_sc_status = 0;

	}
	// update  pre
	pOutput->u31_logo_sad_pre = s_pContext->_output_read_comreg.u30_me_aSAD_rb;
	for (idx = 0; idx < LOGO_RG_32; idx++)
	{
		pOutput->u20_logo_dtl_pre[idx] = s_pContext->_output_read_comreg.u20_me_rDTL_rb[idx];
	}
}

VOID FRC_LgDet_pure_panning_detect(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	short u11_gmv_mvx = (s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb);
	short u10_gmv_mvy = (s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb);
	unsigned int u32_gmv_cnt = s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb;
	short u11_2nd_gmv_mvx = (s_pContext->_output_read_comreg.s11_me_GMV_2nd_vx_rb);
	short u10_2nd_gmv_mvy = (s_pContext->_output_read_comreg.s10_me_GMV_2nd_vy_rb);
	unsigned int u32_2nd_gmv_cnt = s_pContext->_output_read_comreg.u17_me_GMV_2nd_cnt_rb;
	unsigned int i=0, cnt = 0, cnt1 = 0, cnt2 = 0;
	unsigned int u32_RB_val;
	short  u10_rmv_mvy[32];
	short  u11_rmv_mvx[32];
	unsigned short   rmv_rcnt_rb[32];
	unsigned char   Pure_panning_status = 0;
	static unsigned int fcnt =0;
	unsigned short Boundary_check[2];//0:Top, 1:Bot
	unsigned char  Full_Boundary_check= 1;
	unsigned short	 rmv_check_status[3][32]={0};
	static short u11_gmv_mvx_pre = 0;
	static short u10_gmv_mvy_pre = 0;

	ReadRegister(HARDWARE_HARDWARE_58_reg, 0, 31, &u32_RB_val);//bit 8
	Boundary_check[0] = s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_TOP];
	Boundary_check[1] = s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_BOT];

	if(Boundary_check[0]>220 && Boundary_check[1]<=1930){
		Full_Boundary_check = 0;
	}else{
		Full_Boundary_check = 1;
	}

	for(i=0;i<32;i++)
	{
		u11_rmv_mvx[i]=s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i];
		u10_rmv_mvy[i]=s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i];
		rmv_rcnt_rb[i]=s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[i];
	
		if(_ABS_DIFF_(u11_rmv_mvx[i],u11_gmv_mvx)<=1 && _ABS_DIFF_(u10_rmv_mvy[i],u10_gmv_mvy)<=1 && rmv_rcnt_rb[i]>=800){
			rmv_check_status[0][i]=1;
			cnt++;
		}else{
			rmv_check_status[0][i]=0;
		}

		if(_ABS_DIFF_(u11_rmv_mvx[i],u11_gmv_mvx)<10 && _ABS_DIFF_(u10_rmv_mvy[i],u10_gmv_mvy)<=3 && rmv_rcnt_rb[i]>=500){
			rmv_check_status[1][i]=1;
			cnt1++;//rmv_mvy_diff small 
		}else{
			rmv_check_status[1][i]=0;
		}
		
		if(_ABS_DIFF_(u11_rmv_mvx[i],u11_gmv_mvx)<3 && _ABS_DIFF_(u10_rmv_mvy[i],u10_gmv_mvy)<=10 && rmv_rcnt_rb[i]>=500){
			rmv_check_status[2][i]=1;
			cnt2++;//rmv_mvx_diff small
		}else{
			rmv_check_status[2][i]=0;
		}
	 }

	if ( ((u32_RB_val >>  8) & 0x01) ==1 ){
		for(i=0;i<32;i++)
		{
			//rtd_pr_memc_emerg("[%s][%d][%d][,%d,%d,%d,]\n",__FUNCTION__, __LINE__,i,rmv_check_status[0][i],rmv_check_status[1][i],rmv_check_status[2][i]);
		}
	 }

	if((s_pContext->_output_me_sceneAnalysis.u2_panning_flag == 1)&&(u32_gmv_cnt>30000)&&(u11_gmv_mvx>=5 && _ABS_(u10_gmv_mvy)<=1) && (cnt>=30)){
		Pure_panning_status = 1;// panning mode 1 (slow V) 	
	}else if((s_pContext->_output_me_sceneAnalysis.u2_panning_flag == 1)&&(u32_gmv_cnt>20000)&&(u11_gmv_mvx>=5 && _ABS_(u10_gmv_mvy)<=5) && (_ABS_(u11_2nd_gmv_mvx)<=5 && _ABS_(u10_2nd_gmv_mvy)<=10 && u32_2nd_gmv_cnt<=400) && (cnt1>=25)){
			if(((rmv_check_status[0][26]==0 && rmv_check_status[0][27]==0 && rmv_check_status[0][28]==0 && rmv_check_status[0][29]==0) && (cnt>=28))
				&& ((rmv_check_status[0][26]==0 && rmv_check_status[0][27]==0 && rmv_check_status[0][28]==0) && (cnt>=29))){
				Pure_panning_status = 9;//panning mode 4 (with logo)	
			}else{	
				Pure_panning_status = 2;//panning mode 2(slow V )	
	}	
	}else if((u32_gmv_cnt>25000)&&(u10_gmv_mvy>=70 || u10_gmv_mvy<=-50)&& (u32_2nd_gmv_cnt<=500) && (cnt1>=26)){
		Pure_panning_status = 3;//fast V panning 
	}else if((u32_gmv_cnt>25000)&&(u11_gmv_mvx>=70 || u11_gmv_mvx<=-50)&& (u32_2nd_gmv_cnt<=500) && (cnt2>=26)){
		Pure_panning_status = 4;//fast H panning
	}else if((s_pContext->_output_me_sceneAnalysis.u2_panning_flag == 1)&&(u32_gmv_cnt>22000)&&(_ABS_(u11_gmv_mvx)<=35 && _ABS_(u10_gmv_mvy)<=5) && (_ABS_(u11_2nd_gmv_mvx)<=5 && _ABS_(u10_2nd_gmv_mvy)<=10 && u32_2nd_gmv_cnt<=100) && (cnt2>=25)){
		Pure_panning_status = 5;//slow H panning mode
	}else if((u32_gmv_cnt>20000)&&(_ABS_(u11_gmv_mvx)>=10 && _ABS_(u11_gmv_mvx)<=60 && _ABS_(u10_gmv_mvy)<=80) &&(Full_Boundary_check==0)){
		Pure_panning_status = 6;//slow panning mode mix 	
	}else if((u32_gmv_cnt>22000)&&(_ABS_(u11_gmv_mvx)<=10 && _ABS_(u10_gmv_mvy)<=10) 
				&& ((_ABS_(u11_gmv_mvx)<=15 && _ABS_(u10_2nd_gmv_mvy)>=40 && _ABS_(u10_2nd_gmv_mvy)<=80 && u32_2nd_gmv_cnt<=450) || (_ABS_(u10_2nd_gmv_mvy)<=20 && u32_2nd_gmv_cnt<=400))
				&& (cnt1>=22 || cnt2>=22)){
				Pure_panning_status = 7;//slow panning mode mix 	
	}else if((s_pContext->_output_me_sceneAnalysis.u2_panning_flag == 1)&&(u32_gmv_cnt>20000)&& (_ABS_(u11_gmv_mvx)>=5 || _ABS_(u10_gmv_mvy)>=5) 
			&& (_ABS_(u11_2nd_gmv_mvx)<=5 && _ABS_(u10_2nd_gmv_mvy)<=10 && u32_2nd_gmv_cnt<=1600) && (cnt>=25)){
			if((cnt>=30)){
				Pure_panning_status = 8;//panning mode 3
			}else if((rmv_check_status[0][26]==0 && rmv_check_status[0][27]==0 && rmv_check_status[0][28]==0 && rmv_check_status[0][29]==0) && (cnt>=28)){
				Pure_panning_status = 9;//panning mode 4 (with logo)
			}else{
				Pure_panning_status = 0;
			}	
	}else if((s_pContext->_output_me_sceneAnalysis.u2_panning_flag == 1)&&(u32_gmv_cnt>18000)&&(_ABS_(u11_gmv_mvx)>=40 && _ABS_(u11_gmv_mvx)<=60 && _ABS_(u10_gmv_mvy)<=15) 
				&& ((_ABS_(u11_2nd_gmv_mvx)<=8 && _ABS_(u10_2nd_gmv_mvy)<=8 && u32_2nd_gmv_cnt<=400)||(_ABS_(u11_2nd_gmv_mvx)<=2 && _ABS_(u10_2nd_gmv_mvy)<=2 && u32_2nd_gmv_cnt<=1200)) && (cnt1>=20 && cnt2>=18 )){
			Pure_panning_status = 10;
	}else{		
		Pure_panning_status = 0;
	}

	if (Pure_panning_status != 0)
	{
		pOutput->u1_logo_PurePanning_status = Pure_panning_status;
		pOutput->u8_logo_PurePanning_hold_frm = 20;//same as netflix_hold_frm
	}
	else if (pOutput->u8_logo_PurePanning_hold_frm > 0)
	{
		pOutput->u8_logo_PurePanning_hold_frm = pOutput->u8_logo_PurePanning_hold_frm - 1;
	}
	else
	{
		pOutput->u1_logo_PurePanning_status = 0;
		pOutput->u8_logo_PurePanning_hold_frm = 0;
	}

	if ( ((u32_RB_val >>  8) & 0x01) ==1 ){
		if( fcnt % 10 == 0 || pOutput->u1_logo_PurePanning_status !=10){
			rtd_pr_memc_emerg("[%s][%d][,%d,%d][,%d,%d,%d,][,%d,%d,%d,][,%d,%d,%d,][%d][%d]\n",__FUNCTION__, __LINE__,pOutput->u1_logo_PurePanning_status,s_pContext->_output_me_sceneAnalysis.u2_panning_flag, 
				cnt,cnt1,cnt2,u11_gmv_mvx,u10_gmv_mvy,u32_gmv_cnt,u11_2nd_gmv_mvx,u10_2nd_gmv_mvy,u32_2nd_gmv_cnt, pOutput->u8_logo_PurePanning_hold_frm
				,s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true);
		}
	}

	if(fcnt<=0xffff){
		fcnt++;
	}
else{
		fcnt=0;
	}
	u11_gmv_mvx_pre = u11_gmv_mvx;
	u10_gmv_mvy_pre = u10_gmv_mvy;
}


VOID FRC_LgDet_BG_still_detect(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	short u11_gmv_mvx = (s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb)>>1;
	short u10_gmv_mvy = (s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb)>>1;
	short  u10_rmv_mvy[32];
	short  u11_rmv_mvx[32];
	unsigned int i=0, cnt = 0, cnt1 = 0;
	unsigned int u32_RB_val;
	static unsigned int fcnt =0;
	//unsigned int rPRD_rb_sum=0;
	unsigned int  lt_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[0] + s_pContext->_output_read_comreg.u10_blklogo_rgcnt[1];
	unsigned int  rt_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[6] + s_pContext->_output_read_comreg.u10_blklogo_rgcnt[7];
	unsigned int  lb_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[24];
	unsigned int  rb_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[31];
	unsigned char   BG_still_status = 0;

	ReadRegister(HARDWARE_HARDWARE_58_reg, 0, 31, &u32_RB_val);//bit 13
	
	for(i=0;i<32;i++)
	{
		u11_rmv_mvx[i]=s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i];
		u10_rmv_mvy[i]=s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i];
		//u13_me_rPRD_rb[i]= s_pContext->_output_read_comreg.u13_me_rPRD_rb[i];
		if(_ABS_(u11_rmv_mvx[i])<4 && _ABS_(u10_rmv_mvy[i])<=4){
			cnt++;
		}
		if(_ABS_(u11_rmv_mvx[i]-u11_gmv_mvx*2)<4 && _ABS_(u10_rmv_mvy[i]-u10_gmv_mvy*2)<=4){
			cnt1++;
		}
	 }
	

	if(((lt_logo_cnt<30)&&(rt_logo_cnt)<80) &&(lb_logo_cnt<100)&&(rb_logo_cnt<200)
		&& ((((cnt>=25) || (cnt1>=25)) && (u11_gmv_mvx <=6 && u10_gmv_mvy <= 2)) || ((u11_gmv_mvx <=40 && u10_gmv_mvy <= 2)&&(s_pContext->_output_me_sceneAnalysis.u2_panning_flag ==1)))){
		BG_still_status = 1;
	}else{
		BG_still_status = 0;
	}	
	
	if ( (fcnt % 10 == 0  && ((u32_RB_val >>  13) & 0x01) ==1 )){
		//rtd_pr_memc_emerg("[%s][%d][,%d,%d,%d,][,%d,%d,][,%d,%d,%d,]\n",__FUNCTION__, __LINE__,BG_still_status, cnt,cnt1,u11_gmv_mvx,u10_gmv_mvy,(lt_logo_cnt+rt_logo_cnt),lb_logo_cnt,rb_logo_cnt);
	}

	if (BG_still_status == 1)
	{
		pOutput->u1_logo_BG_still_status = 1;
		pOutput->u8_logo_BG_still_hold_frm = 15;//same as netflix_hold_frm
	}
	else if (pOutput->u8_logo_BG_still_hold_frm > 0)
	{
		pOutput->u1_logo_BG_still_status = 1;
		pOutput->u8_logo_BG_still_hold_frm = pOutput->u8_logo_BG_still_hold_frm - 1;
	}
	else
	{
		pOutput->u1_logo_BG_still_status = 0;
		pOutput->u8_logo_BG_still_hold_frm = 0;
	}

	if(fcnt<=0xffff){
		fcnt++;
	}
	else{
		fcnt=0;
	}

}

VOID FRC_LgDet_KeyRgn_detect(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned int  lt_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[0] + s_pContext->_output_read_comreg.u10_blklogo_rgcnt[1];
	unsigned int  rt_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[6] + s_pContext->_output_read_comreg.u10_blklogo_rgcnt[7];
	unsigned int  cb_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[27] + s_pContext->_output_read_comreg.u10_blklogo_rgcnt[28];
	unsigned int  lb_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[24];
	unsigned int  rb_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[31];
	unsigned char   BG_KeyRgn_status = 0;


	if((lt_logo_cnt>150)||(rt_logo_cnt>150)||(cb_logo_cnt>500)||(lb_logo_cnt>100)||(rb_logo_cnt>100)){
		BG_KeyRgn_status = 1;
	}else{
		BG_KeyRgn_status = 0;
	}


	if (BG_KeyRgn_status == 1)
	{
		pOutput->u1_logo_KeyRgn_status = 1;
		pOutput->u8_logo_KeyRgn_hold_frm = 15;//same as netflix_hold_frm
	}
	else if (pOutput->u8_logo_KeyRgn_hold_frm > 0)
	{
		pOutput->u1_logo_KeyRgn_status = 1;
		pOutput->u8_logo_KeyRgn_hold_frm = pOutput->u8_logo_KeyRgn_hold_frm - 1;
	}
	else
	{
		pOutput->u1_logo_KeyRgn_status = 0;
		pOutput->u8_logo_KeyRgn_hold_frm = 0;
	}

	
}

VOID FRC_LgDet_netflix_detect(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned int lt_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[0] + s_pContext->_output_read_comreg.u10_blklogo_rgcnt[1];
	unsigned int rt_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[6] + s_pContext->_output_read_comreg.u10_blklogo_rgcnt[7];
	unsigned int lb_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[24];
	unsigned int rb_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[31];
	unsigned char  netflix_status = 0;

	if (pParam->u1_logo_netflix_detct_en == 1)
	{
		if (lt_logo_cnt >= pParam->u20_logo_netflix_detect_lt_th_l && lt_logo_cnt <= pParam->u20_logo_netflix_detect_lt_th_h &&
			rt_logo_cnt >= pParam->u20_logo_netflix_detect_rt_th_l && rt_logo_cnt <= pParam->u20_logo_netflix_detect_rt_th_h &&
			lb_logo_cnt >= pParam->u20_logo_netflix_detect_lb_th_l && lb_logo_cnt <= pParam->u20_logo_netflix_detect_lb_th_h &&
			rb_logo_cnt >= pParam->u20_logo_netflix_detect_rb_th_l && rb_logo_cnt <= pParam->u20_logo_netflix_detect_rb_th_h &&
			pOutput->u1_logo_cnt_glb_clr_status == 0)
		{
			netflix_status = 1;
		}
		else
		{
			netflix_status = 0;
		}
	}
	else
	{
		netflix_status = 0;
	}


	if (netflix_status == 1)
	{
		pOutput->u1_logo_netflix_status = 1;
		pOutput->u8_logo_netflix_hold_frm = pParam->u8_logo_netflix_hold_frm;
	}
	else if (pOutput->u8_logo_netflix_hold_frm > 0)
	{
		pOutput->u1_logo_netflix_status = 1;
		pOutput->u8_logo_netflix_hold_frm = pOutput->u8_logo_netflix_hold_frm - 1;
	}
	else
	{
		pOutput->u1_logo_netflix_status = 0;
		pOutput->u8_logo_netflix_hold_frm = 0;
	}
}

VOID FRC_LogoDet_NearRim_logo(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned int  lt_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[0];
	unsigned int  rt_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[7];
	unsigned int  lb_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[24];
	unsigned int  rb_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[31];	
	static unsigned int  tmp_lt_logo_cnt[17] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	static unsigned int  tmp_rt_logo_cnt[17] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	static unsigned int  tmp_lb_logo_cnt[17] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	static unsigned int  tmp_rb_logo_cnt[17] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	unsigned char   NearRim_logo_status = 0;
	unsigned int u32_RB_val;
	unsigned int i = 0;
	unsigned int temp_sum_lt_logo_cnt = 0,temp_sum_rt_logo_cnt = 0,temp_sum_lb_logo_cnt = 0,temp_sum_rb_logo_cnt = 0;
	static unsigned int temp_lt_logo_cnt_avg = 0,temp_rt_logo_cnt_avg = 0,temp_lb_logo_cnt_avg = 0,temp_rb_logo_cnt_avg = 0;
	static unsigned int fcnt =0;

	ReadRegister(HARDWARE_HARDWARE_58_reg, 0, 31, &u32_RB_val);//bit 13

	tmp_lt_logo_cnt[16]=lt_logo_cnt;
	tmp_rt_logo_cnt[16]=rt_logo_cnt;
	tmp_lb_logo_cnt[16]=lb_logo_cnt;
	tmp_rb_logo_cnt[16]=rb_logo_cnt;
	
	for(i=0;i<16;i++){
		//first in and first out	
		tmp_lt_logo_cnt[i]=tmp_lt_logo_cnt[i+1];		
		tmp_rt_logo_cnt[i]=tmp_rt_logo_cnt[i+1];
		tmp_lb_logo_cnt[i]=tmp_lb_logo_cnt[i+1];
		tmp_rb_logo_cnt[i]=tmp_rb_logo_cnt[i+1];	
		//sum by temp logo cnt
		temp_sum_lt_logo_cnt = temp_sum_lt_logo_cnt + tmp_lt_logo_cnt[i];	
		temp_sum_rt_logo_cnt = temp_sum_rt_logo_cnt + tmp_rt_logo_cnt[i];
		temp_sum_lb_logo_cnt = temp_sum_lb_logo_cnt + tmp_lb_logo_cnt[i];
		temp_sum_rb_logo_cnt = temp_sum_rb_logo_cnt + tmp_rb_logo_cnt[i];			
	}
	temp_lt_logo_cnt_avg = 	temp_sum_lt_logo_cnt>>4;
	temp_rt_logo_cnt_avg = 	temp_sum_rt_logo_cnt>>4;
	temp_lb_logo_cnt_avg = 	temp_sum_lb_logo_cnt>>4;
	temp_rb_logo_cnt_avg = 	temp_sum_rb_logo_cnt>>4;

	if((temp_lt_logo_cnt_avg>130 && temp_lt_logo_cnt_avg<180)&&(temp_rt_logo_cnt_avg<=20)&&(temp_lb_logo_cnt_avg<=20)&&(temp_rb_logo_cnt_avg<=20)){
		NearRim_logo_status = 1;
	}else{
		NearRim_logo_status = 0;
	}

	if ( (fcnt % 10 == 0  && ((u32_RB_val >>  13) & 0x01) ==1 )){
		//rtd_pr_memc_emerg("[%s][%d][,%d,][,%d,%d,%d,%d,]\n",__FUNCTION__, __LINE__,NearRim_logo_status,temp_lt_logo_cnt_avg,temp_rt_logo_cnt_avg,temp_lb_logo_cnt_avg,temp_rb_logo_cnt_avg);
	}

	if (NearRim_logo_status == 1)
	{
		pOutput->u1_logo_NearRim_logo_status = 1;
		pOutput->u8_logo_NearRim_logo_hold_frm = 20;//pParam->u8_logo_netflix_hold_frm;
	}
	else if (pOutput->u8_logo_NearRim_logo_hold_frm > 0)
	{
		pOutput->u1_logo_NearRim_logo_status = 1;
		pOutput->u8_logo_NearRim_logo_hold_frm = pOutput->u8_logo_NearRim_logo_hold_frm - 1;
	}
	else
	{
		pOutput->u1_logo_NearRim_logo_status = 0;
		pOutput->u8_logo_NearRim_logo_hold_frm = 0;
	}

	if(fcnt<=0xfffA){
		fcnt++;
	}
else{
		fcnt=0;
	}

}


VOID FRC_LgDet_LG_16s_Patch(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned char  u1_apli_status = 0;
	unsigned char  u1_aplp_status = 0;
	unsigned char  u1_gdtl_status = 0;
	unsigned char  u1_gsad_status = 0;
	unsigned char  u1_gmv_status  = 0;
	unsigned char  u1_rdtl_num_status;
	unsigned int idx;
	unsigned int rdtl_good_num  = 0;

	unsigned char  u1_lg16s_patch_status = 0;

	unsigned int u16_cnt_totalNum = (s_pContext->_output_read_comreg.u8_me1_Vbuf_Hsize_rb * s_pContext->_output_read_comreg.u8_me1_Vbuf_Vsize_rb);

	// apl
	unsigned int apli_avg = (u16_cnt_totalNum > 0 ) ? (s_pContext->_output_read_comreg.u26_me_aAPLi_rb / u16_cnt_totalNum) : s_pContext->_output_read_comreg.u26_me_aAPLi_rb;
	unsigned int aplp_avg = (u16_cnt_totalNum > 0 ) ? (s_pContext->_output_read_comreg.u26_me_aAPLp_rb / u16_cnt_totalNum) : s_pContext->_output_read_comreg.u26_me_aAPLp_rb;

	if (pParam->u1_logo_lg16s_patch_Gapl_en == 1)
	{
		u1_apli_status = ((apli_avg >= pParam->u10_logo_lg16s_patch_Gapl_th_l) && (apli_avg <= pParam->u10_logo_lg16s_patch_Gapl_th_h))? 1 : 0;
		u1_aplp_status = ((aplp_avg >= pParam->u10_logo_lg16s_patch_Gapl_th_l) && (aplp_avg <= pParam->u10_logo_lg16s_patch_Gapl_th_h))? 1 : 0;
	}
	else
	{
		u1_apli_status = 1;
		u1_aplp_status = 1;
	}

	// glb dtl
	if(pParam->u1_logo_lg16s_patch_Gdtl_en == 1)
	{
		u1_gdtl_status = ((s_pContext->_output_read_comreg.u25_me_aDTL_rb >= pParam->u25_logo_lg16s_patch_Gdtl_th_l) &&
			              (s_pContext->_output_read_comreg.u25_me_aDTL_rb <= pParam->u25_logo_lg16s_patch_Gdtl_th_h)) ? 1 : 0;
	}
	else
	{
		u1_gdtl_status = 1;
	}

	// glb sad
	if (pParam->u1_logo_lg16s_patch_Gsad_en == 1)
	{
		u1_gsad_status = ((s_pContext->_output_read_comreg.u30_me_aSAD_rb >= pParam->u28_logo_lg16s_patch_Gsad_th_l) &&
			              (s_pContext->_output_read_comreg.u30_me_aSAD_rb <= pParam->u28_logo_lg16s_patch_Gsad_th_h)) ? 1 : 0;
	}
	else
	{
		u1_gsad_status = 1;
	}

	// GMV

	if (pParam->u1_logo_lg16s_patch_Gmv_en == 1)
	{
		if ( (s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb >= pParam->u11_logo_lg16s_patch_Gmvx_th_l) &&
			(s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb <= pParam->u11_logo_lg16s_patch_Gmvx_th_h) &&
			(s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb <= pParam->u11_logo_lg16s_patch_Gmvy_th)   &&
			(s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb >= pParam->u6_logo_lg16s_patch_Gmv_cnt_th) &&
			(s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb <= pParam->u12_logo_lg16s_patch_Gmv_unconf_th))
		{
			u1_gmv_status = 1;
		}
		else
		{
			u1_gmv_status = 0;
		}
	}
	else
	{
		u1_gmv_status = 1;
	}

	// region dtl num
	if (pParam->u1_logo_lg16s_patch_Rdtl_num_en == 1)
	{
		for (idx = 0; idx <LOGO_RG_32; idx ++)
		{

			rdtl_good_num = ((s_pContext->_output_read_comreg.u20_me_rDTL_rb[idx] >= pParam->u20_logo_lg16s_patch_Rdtl_th_l) &&
				(s_pContext->_output_read_comreg.u20_me_rDTL_rb[idx] <= pParam->u20_logo_lg16s_patch_Rdtl_th_h)) ? (rdtl_good_num + 1) : rdtl_good_num;
		}

		u1_rdtl_num_status = rdtl_good_num >= pParam->u6_logo_lg16s_patch_Rdtl_num_th ? 1 : 0;
	}
	else
	{
		u1_rdtl_num_status = 1;
	}

	if ((pParam->u1_logo_lg16s_patch_Gapl_en == 1) ||
		(pParam->u1_logo_lg16s_patch_Gdtl_en == 1) ||
		(pParam->u1_logo_lg16s_patch_Gsad_en == 1) ||
		(pParam->u1_logo_lg16s_patch_Gmv_en == 1)  ||
		(pParam->u1_logo_lg16s_patch_Rdtl_num_en == 1))
	{
		u1_lg16s_patch_status = u1_apli_status && u1_aplp_status && u1_gdtl_status && u1_gsad_status && u1_gmv_status && u1_rdtl_num_status;
	}
	else
	{
		u1_lg16s_patch_status = 0;
	}

	pOutput->u1_logo_lg16s_patch_Gapli_status     = u1_apli_status;
	pOutput->u1_logo_lg16s_patch_Gaplp_status     = u1_aplp_status;
	pOutput->u1_logo_lg16s_patch_Gdtl_status      = u1_gdtl_status;
	pOutput->u1_logo_lg16s_patch_Gsad_status      = u1_gsad_status;
	pOutput->u1_logo_lg16s_patch_Gmv_status       = u1_gmv_status;
	pOutput->u1_logo_lg16s_patch_Rdtl_num_status  = u1_rdtl_num_status;

	if (u1_lg16s_patch_status == 1)
	{
		pOutput->u1_logo_lg16s_patch_status = 1;
		pOutput->u8_logo_lg16s_patch_hold_frm = pParam->u8_logo_lg16s_patch_hold_frm;
	}
	else if (pOutput->u8_logo_lg16s_patch_hold_frm > 0)
	{
		pOutput->u1_logo_lg16s_patch_status = 1;
		pOutput->u8_logo_lg16s_patch_hold_frm = pOutput->u8_logo_lg16s_patch_hold_frm - 1;
	}
	else
	{
		pOutput->u1_logo_lg16s_patch_status = 0;
		pOutput->u8_logo_lg16s_patch_hold_frm = 0;
	}
}

VOID FRC_LgDet_RgDhAdp_Rimunbalance(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned char u1_rg_dh_clr_rg0;
	unsigned int blk_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[26] + s_pContext->_output_read_comreg.u10_blklogo_rgcnt[27] + s_pContext->_output_read_comreg.u10_blklogo_rgcnt[28] + s_pContext->_output_read_comreg.u10_blklogo_rgcnt[29];
	unsigned int dh_clr_cnt = s_pContext->_output_read_comreg.u16_blklogo_dh_clrcnt_bot[0] + s_pContext->_output_read_comreg.u16_blklogo_dh_clrcnt_bot[1] + s_pContext->_output_read_comreg.u16_blklogo_dh_clrcnt_bot[2];

	unsigned int blk_logo_dh_clr_cnt = (blk_logo_cnt >= dh_clr_cnt)?(blk_logo_cnt - dh_clr_cnt) : 0;
	if( pParam->u1_RgDhClr_thr_en == 1 && blk_logo_dh_clr_cnt>=pParam->u16_RgDhadp_Rimunbalance_thr_l && blk_logo_dh_clr_cnt<=pParam->u16_RgDhadp_Rimunbalance_thr_h)
	{
		u1_rg_dh_clr_rg0 = 1;
	}
	else
	{
		u1_rg_dh_clr_rg0 =  0;
	}

	if (u1_rg_dh_clr_rg0 == 1)
	{
		pOutput->u1_RgDhadp_Rimunbalance_status =  1;

		pOutput->u8_rg_dh_clr_lgclr_hold_frm = pParam->u8_RgDhadp_Rimunbalance_hold_frm;
	}
	else if (pOutput->u8_RgDhadp_Rimunbalance_hold_frm > 0)
	{
		pOutput->u1_RgDhadp_Rimunbalance_status =  1;
		pOutput->u8_RgDhadp_Rimunbalance_hold_frm = pOutput->u8_RgDhadp_Rimunbalance_hold_frm - 1;
	}
	else
	{
		pOutput->u1_RgDhadp_Rimunbalance_status =  0;
	}

}


VOID FRC_LgDet_LG_UXN_Patch(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();	
	PQL_OUTPUT_RESOLUTION output_Resolution = s_pContext->_external_data._output_resolution;
	const _Param_By_Resolution_RimCtrl *s_pRimParam = GetRimParam(output_Resolution);

	unsigned int lt_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[0]
					+ s_pContext->_output_read_comreg.u10_blklogo_rgcnt[1]
					+ s_pContext->_output_read_comreg.u10_blklogo_rgcnt[2];
	unsigned int rt_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[7];
	unsigned int md_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[8]
					+ s_pContext->_output_read_comreg.u10_blklogo_rgcnt[9]
					+ s_pContext->_output_read_comreg.u10_blklogo_rgcnt[10]
					+ s_pContext->_output_read_comreg.u10_blklogo_rgcnt[11]
					+ s_pContext->_output_read_comreg.u10_blklogo_rgcnt[12]
					+ s_pContext->_output_read_comreg.u10_blklogo_rgcnt[13]
					+ s_pContext->_output_read_comreg.u10_blklogo_rgcnt[14]
					+ s_pContext->_output_read_comreg.u10_blklogo_rgcnt[15];

	unsigned char u1_lt_islogo_cond = 0;
	unsigned char u1_rt_islogo_cond = 0;
	unsigned char u1_md_nologo_cond = 0;
	unsigned char u1_bgmove_cond = 1;
	unsigned char u1_rim_cond = 1;
	unsigned char u1_rg_bright_cond = 1;

	unsigned char u1_lt_islogo_cond_en = 1;
	unsigned char u1_rt_islogo_cond_en = 1;
	unsigned char u1_md_nologo_cond_en = 1;

	unsigned char u1_lg_UXN_patch_en = pParam->u1_lg_UXN_patch_en;//(U32_DATA1 >>31); //
	unsigned char u1_lg_UXN_patch_det = 0;
	static unsigned char u8_lg_UXN_patch_cnt = 0;
	unsigned char u8_lg_UXN_patch_cnt_th = 0x8;//(U32_DATA1 & 0xff);
	unsigned char u8_lg_UXN_patch_cnt_max = 0xC;//((U32_DATA1>>8) & 0xff);

	//unsigned int U32_DATA1 = 0;
	//ReadRegister(KME_TOP_KME_TOP_34_reg, 0, 31, &U32_DATA1);
	u1_lt_islogo_cond_en = 1;//(U32_DATA1 >>30) & 0x1;
	u1_rt_islogo_cond_en = 1;//(U32_DATA1 >>29) & 0x1;
	u1_md_nologo_cond_en = 1;//(U32_DATA1 >>28) & 0x1;

	//unsigned int U32_DATA2;
	//ReadRegister(SOFTWARE_SOFTWARE_43_reg, 0, 31, &U32_DATA2);


	//unsigned int U32_DATA = 0;
	//ReadRegister(SOFTWARE_SOFTWARE_07_reg, 0, 31, &U32_DATA);

	// lt_islogo_cond
	{
		unsigned int u10_logo_cnt_th_l = 0x10<<4;//((U32_DATA & 0xff)<<4);
		unsigned int u10_logo_cnt_th_h = 0x30<<4;//((U32_DATA>>8) & 0xff)<<4;

		static unsigned int lt_logo_cnt_pre = 0;

		unsigned char u1_lt_logoCnt_cond = 0;

		u1_lt_logoCnt_cond = (lt_logo_cnt > u10_logo_cnt_th_l && lt_logo_cnt < u10_logo_cnt_th_h) ? 1 : 0;

		u1_lt_islogo_cond = (u1_lt_logoCnt_cond) ? 1 : 0;

		u1_lt_islogo_cond = (u1_lt_islogo_cond & u1_lt_islogo_cond_en) || (!u1_lt_islogo_cond_en);

		lt_logo_cnt_pre = lt_logo_cnt;
	}

	// rt_islogo_cond
	{
		unsigned int u10_logo_cnt_th2 = 0;//((U32_DATA>>24)& 0xf)<<4;
		unsigned int u10_logo_cnt_thH = ((240/8)*(135/4))*2/3;

		unsigned char u1_rt_logoCnt_cond = 0;

		u1_rt_logoCnt_cond = (rt_logo_cnt > u10_logo_cnt_th2 && rt_logo_cnt < u10_logo_cnt_thH) ? 1 : 0;

		u1_rt_islogo_cond = (u1_rt_logoCnt_cond & u1_rt_islogo_cond_en) || (!u1_rt_islogo_cond_en);
	}

	// md_islogo_cond
	{
		unsigned int u10_logo_cnt_th3 = 0xf<<4;//((U32_DATA>>28) & 0xf)<<4;

		unsigned char u1_md_logoCnt_cond = 0;

		u1_md_logoCnt_cond = (md_logo_cnt < u10_logo_cnt_th3) ? 1: 0;

		u1_md_nologo_cond = (u1_md_logoCnt_cond & u1_md_nologo_cond_en) || (!u1_md_nologo_cond_en);
	}

	// bg move
	{
		//unsigned int u17_hpan_cnt = s_pContext->_output_read_comreg.u17_me_Hpan_cnt_rb;
		//unsigned int u17_vpan_cnt = s_pContext->_output_read_comreg.u17_me_Vpan_cnt_rb;
		//unsigned int u16_hvpan_th = (U32_DATA2 & 0xFFFF);

		unsigned char u1_bgmove_mode_sel = 1;//(U32_DATA2>>31)&0x1;
		unsigned char u8_bgmove_cnt_th = 0x32;//(U32_DATA2>>16)&0xff;

		unsigned char idx = 0;
		unsigned char u8_bmvx_cnt = 0;
		unsigned char u8_bmvy_cnt = 0;
		unsigned char u8_bmvx_th = 0x10;
		unsigned char u8_bmvy_th = 0xC;
		for (idx=0; idx<32; idx++)
		{
			unsigned char u1_bmvx_cond =		(_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[idx]) > u8_bmvx_th) ||
									(_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_2nd_vx_rb[idx]) > u8_bmvx_th);
			unsigned char u1_bmvy_cond =		(_ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[idx]) > u8_bmvy_th) ||
									(_ABS_(s_pContext->_output_read_comreg.s10_me_rMV_2nd_vy_rb[idx]) > u8_bmvy_th);
			u8_bmvx_cnt = (u1_bmvx_cond || u1_bmvy_cond) ? (u8_bmvx_cnt+1) : u8_bmvx_cnt;
			u8_bmvy_cnt = u8_bmvx_cnt;//u1_bmvy_cond ? (u8_bmvy_cnt+1) : u8_bmvy_cnt;
		}

		//u1_bgmove_cond = ((u17_hpan_cnt+u17_vpan_cnt) > u16_hvpan_th) ? 1 : 0;

		if(u1_bgmove_mode_sel==1){
			u1_bgmove_cond = ((u8_bmvx_cnt+u8_bmvy_cnt) > u8_bgmove_cnt_th) ? 1 : 0;
		}
	}

	// rim cond
	{
		unsigned char u1_rim_tb_cond = 1;
		unsigned char u1_rim_lr_cond = 1;

		unsigned char u8_rim_tb_th = 8*s_pRimParam->u8_scale_V; //4k: u8_rim_tb_th=32
		unsigned char u8_rim_lr_th = 1*s_pRimParam->u8_scale_H; //4k: u8_rim_lr_th=4

		u1_rim_tb_cond = (s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_TOP] < u8_rim_tb_th)
					&& (s_pContext->_output_rimctrl.u12_out_resolution[_RIM_BOT] < (s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_BOT] + u8_rim_tb_th));
		u1_rim_lr_cond = (s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_LFT] < u8_rim_lr_th)
					&& (s_pContext->_output_rimctrl.u12_out_resolution[_RIM_RHT] < (s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_RHT] + u8_rim_lr_th));

		u1_rim_cond = (u1_rim_tb_cond && u1_rim_lr_cond);
	}

	// bright cond
	{
		unsigned int u20_me_rAPLi = s_pContext->_output_read_comreg.u20_me_rAPLi_rb[7];
		unsigned int u20_me_rAPLp = s_pContext->_output_read_comreg.u20_me_rAPLp_rb[7];
		unsigned int u20_me_rAPL = (u20_me_rAPLi + u20_me_rAPLp)/2;

		unsigned int u20_me_rAPL_th = 0x40000;

		u1_rg_bright_cond = (u20_me_rAPL > u20_me_rAPL_th) ? 1 : 0;
	}

	// UXN patch detect
	if(  u1_lt_islogo_cond && u1_rt_islogo_cond && u1_md_nologo_cond
	&& u1_bgmove_cond && u1_rim_cond
	&& u1_lg_UXN_patch_en)
	{
		u8_lg_UXN_patch_cnt ++;
	}
	else
	{
		u8_lg_UXN_patch_cnt = (u8_lg_UXN_patch_cnt > 0) ? (u8_lg_UXN_patch_cnt - 1) : 0;
	}

	u8_lg_UXN_patch_cnt = _CLIP_UBOUND_(u8_lg_UXN_patch_cnt, u8_lg_UXN_patch_cnt_max);

	u1_lg_UXN_patch_det = (u8_lg_UXN_patch_cnt > u8_lg_UXN_patch_cnt_th) ? 1 : 0;


	// judge
	pOutput->u1_lg_UXN_patch_det = u1_lg_UXN_patch_det;

#if 0
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 0, 3, u1_lt_islogo_cond);
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 4, 7, u1_rt_islogo_cond);
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 8, 11, u1_md_nologo_cond);
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 12, 15, u1_bgmove_cond);

	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 16, 19, _MIN_(u8_lg_UXN_patch_cnt, 0xF));
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 20, 23, u1_lg_UXN_patch_det);
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, u1_rg_bright_cond);
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, u1_rim_cond);
#endif

}


VOID FRC_LgDet_CloseVar(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
    const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned char u1_hw_sc_signal = s_pContext->_output_read_comreg.u1_sc_status_rb;

	if (u1_hw_sc_signal == 1)
	{
	    pOutput->u32_logo_closeVar_hold_cnt = 60;
	}
	else if (pOutput->u32_logo_closeVar_hold_cnt > 0)
	{
	    pOutput->u32_logo_closeVar_hold_cnt = pOutput->u32_logo_closeVar_hold_cnt - 1;
	}
	else
	{
	    pOutput->u32_logo_closeVar_hold_cnt = 0;
	}

}

VOID FRC_LgDet_LogoHaloEnhance(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
	//const _PQLCONTEXT *s_pContext = GetPQLContext();
}

VOID FRC_LgDet_DynamicOBME(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
	//const _PQLCONTEXT *s_pContext = GetPQLContext();
}

VOID FRC_LgDet_RgnDistribute(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	unsigned int iter = 0;

	//unsigned int lt_corner_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[0] + s_pContext->_output_read_comreg.u10_blklogo_rgcnt[1];
	//unsigned int rt_corner_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[6] + s_pContext->_output_read_comreg.u10_blklogo_rgcnt[7];
	//unsigned int lb_corner_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[24];
	//unsigned int rb_corner_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[31];

	unsigned int top_cnt = 0;
	unsigned int bot_cnt = 0;

	for(iter=0; iter<8; iter++)
	{
		top_cnt = top_cnt + s_pContext->_output_read_comreg.u10_blklogo_rgcnt[0+iter];
		bot_cnt = bot_cnt + s_pContext->_output_read_comreg.u10_blklogo_rgcnt[24+iter];
	}

	if( bot_cnt > ((BLK_LOGO_RG_BLKCNT*8) >> 1) )
	{
		pOutput->u1_logo_rgn_distribute[LOGO_RGN_BOT] = 1;
	}
	else
	{
		pOutput->u1_logo_rgn_distribute[LOGO_RGN_BOT] = 0;
	}

}


typedef struct dhProcLvlThr
{
	int Inc_Z2LRatio;
	int Dec_Z2LRatio;
	int Inc_Z2LDiff;
	int Dec_Z2LDiff;
	int Inc_DevVal;
	int Dec_DevVal;
} dhProcLvlThr;

VOID FRC_DhLgProc_H_MovingCase(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
#ifdef FUNC_LOG
#undef FUNC_LOG
#endif
#define FUNC_LOG 1

#if FUNC_LOG
	// print related code
	static unsigned char printkCounter = 0;
	unsigned int printkEnable = 0;
	unsigned int printkPeriod = 1;
#endif	

	unsigned char RgnIdx = 0, i = 0;
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	const unsigned char RgnIdxREMAP[32] = // current only separate the region0/1, but dehalo logo process has 0/1/2 regions
	{
		0,	0,	0,	0,	0,	0,	0,	0,
		1,	1,	1,	1,	1,	1,	1,	1,
		2,	2,	2,	2,	2,	2,	2,	2,
		3,	3,	3,	3,	3,	3,	3,	3
	};

	int s32_RgnRmvSumX[4] = {0}, s32_RgnRmvSumY[4] = {0}, s32_RgnDTLSum[4] = {0};
	static int s32_RgnRmvSumX_iir[4] = {0}, s32_RgnRmvSumY_iir[4] = {0}, s32_RgnDTLSum_iir[4] = {0};
	unsigned int  u32_RgnRmvDevX[4] = {0}, u32_RgnRmvDevY[4] = {0}, u32_RgnDTLDev[4] = {0};
	static unsigned int u32_RgnRmvDevX_iir[4] = {0}, u32_RgnRmvDevY_iir[4] = {0}, u32_RgnDTLDev_iir[4] = {0};
	unsigned int RmvDevX_iir_th = 400, RmvDevY_iir_th = 300;
	unsigned int RmvDevX_iir_stable_csae = 0, RmvDevY_iir_stable_csae = 0;
	unsigned int test_case1 = 0, test_case2 = 0, test_case3 = 0;


	for(RgnIdx=0; RgnIdx<32; RgnIdx++)
	{
		s32_RgnRmvSumX[RgnIdxREMAP[RgnIdx]] = s32_RgnRmvSumX[RgnIdxREMAP[RgnIdx]] + s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[RgnIdx];
		s32_RgnRmvSumY[RgnIdxREMAP[RgnIdx]] = s32_RgnRmvSumY[RgnIdxREMAP[RgnIdx]] + s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[RgnIdx];
		s32_RgnDTLSum[RgnIdxREMAP[RgnIdx]]	= s32_RgnDTLSum[RgnIdxREMAP[RgnIdx]]  +	s_pContext->_output_read_comreg.u20_me_rDTL_rb[RgnIdx];	
	}

	for(RgnIdx=0; RgnIdx<32; RgnIdx++)
	{
		// deviation
		if( RgnIdxREMAP[RgnIdx] == 0 )
		{
			u32_RgnRmvDevX[0] = u32_RgnRmvDevX[0] + _ABS_DIFF_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[RgnIdx]*8,s32_RgnRmvSumX[0]);
			u32_RgnRmvDevY[0] = u32_RgnRmvDevY[0] + _ABS_DIFF_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[RgnIdx]*8,s32_RgnRmvSumY[0]);
			u32_RgnDTLDev[0] = u32_RgnDTLDev[0] + _ABS_DIFF_(s_pContext->_output_read_comreg.u20_me_rDTL_rb[RgnIdx]*8,s32_RgnDTLSum[0]);
		}
		else if( RgnIdxREMAP[RgnIdx] == 1 )
		{
			u32_RgnRmvDevX[1] = u32_RgnRmvDevX[1] + _ABS_DIFF_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[RgnIdx]*8,s32_RgnRmvSumX[1]);
			u32_RgnRmvDevY[1] = u32_RgnRmvDevY[1] + _ABS_DIFF_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[RgnIdx]*8,s32_RgnRmvSumY[1]);
			u32_RgnDTLDev[1] = u32_RgnDTLDev[1] + _ABS_DIFF_(s_pContext->_output_read_comreg.u20_me_rDTL_rb[RgnIdx]*8,s32_RgnDTLSum[1]);		
		}
		else if( RgnIdxREMAP[RgnIdx] == 2 )
		{
			u32_RgnRmvDevX[2] = u32_RgnRmvDevX[2] + _ABS_DIFF_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[RgnIdx]*8,s32_RgnRmvSumX[2]);
			u32_RgnRmvDevY[2] = u32_RgnRmvDevY[2] + _ABS_DIFF_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[RgnIdx]*8,s32_RgnRmvSumY[2]);
			u32_RgnDTLDev[2] = u32_RgnDTLDev[2] + _ABS_DIFF_(s_pContext->_output_read_comreg.u20_me_rDTL_rb[RgnIdx]*8,s32_RgnDTLSum[2]);		
		}
		else if( RgnIdxREMAP[RgnIdx] == 3 )
		{
			u32_RgnRmvDevX[3] = u32_RgnRmvDevX[3] + _ABS_DIFF_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[RgnIdx]*8,s32_RgnRmvSumX[3]);
			u32_RgnRmvDevY[3] = u32_RgnRmvDevY[3] + _ABS_DIFF_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[RgnIdx]*8,s32_RgnRmvSumY[3]);
			u32_RgnDTLDev[3] = u32_RgnDTLDev[3] + _ABS_DIFF_(s_pContext->_output_read_comreg.u20_me_rDTL_rb[RgnIdx]*8,s32_RgnDTLSum[3]);	
		}

#if 0//FUNC_LOG
		ReadRegister(SOFTWARE1_SOFTWARE1_21_reg, 29, 29, &printkEnable);
		ReadRegister(SOFTWARE1_SOFTWARE1_21_reg, 24, 27, &printkPeriod);
		printkPeriod = (printkPeriod+1);//*8;
		
		if( printkCounter == 0 && printkEnable == 1 )
		{
			rtd_pr_memc_info("DEV_%d_u32_RgnDTLDev = ( %d  ) ( %d ) ( %d , %d) (%d)\n",RgnIdx,u32_RgnDTLDev[RgnIdxREMAP[RgnIdx]],
						RgnIdxREMAP[RgnIdx], s_pContext->_output_read_comreg.u20_me_rDTL_rb[RgnIdx], s32_RgnDTLSum[RgnIdxREMAP[RgnIdx]],
						_ABS_DIFF_(s_pContext->_output_read_comreg.u20_me_rDTL_rb[RgnIdx]*8,s32_RgnDTLSum[RgnIdxREMAP[RgnIdx]]));	
		}
		
#endif	
		
		
	}

	// iir process
	for(i=0; i<4; i++)
	{
		u32_RgnRmvDevX_iir[i] = (u32_RgnRmvDevX_iir[i] + u32_RgnRmvDevX[i]) >> 1;
		u32_RgnRmvDevY_iir[i] = (u32_RgnRmvDevY_iir[i] + u32_RgnRmvDevY[i]) >> 1;
		u32_RgnDTLDev_iir[i]  = (u32_RgnDTLDev_iir[i]  + u32_RgnDTLDev[i] ) >> 1;
		s32_RgnRmvSumX_iir[i] = (s32_RgnRmvSumX_iir[i] + s32_RgnRmvSumX[i]) >> 1;
		s32_RgnRmvSumY_iir[i] = (s32_RgnRmvSumY_iir[i] + s32_RgnRmvSumY[i]) >> 1;		
		s32_RgnDTLSum_iir[i] = (s32_RgnDTLSum_iir[i] + s32_RgnDTLSum[i]) >> 1;

		if(u32_RgnRmvDevX_iir[i] < RmvDevX_iir_th){
		 	RmvDevX_iir_stable_csae++;
		}

		if(u32_RgnRmvDevY_iir[i] < RmvDevY_iir_th){
			RmvDevY_iir_stable_csae++;
		}

	}


	if((((s32_RgnDTLSum_iir[0]>400000) && (s32_RgnDTLSum_iir[0]<=700000)&&(u32_RgnDTLDev_iir[0]>1500000) && (u32_RgnDTLDev_iir[0]<=2200000)) &&
		 ((s32_RgnDTLSum_iir[1]>650000) && (s32_RgnDTLSum_iir[1]<=860000)&&(u32_RgnDTLDev_iir[1]>400000)  && (u32_RgnDTLDev_iir[1]<=1100000)) &&
		 ((s32_RgnDTLSum_iir[2]>500000) && (s32_RgnDTLSum_iir[2]<=740000)&&(u32_RgnDTLDev_iir[2]>530000)  && (u32_RgnDTLDev_iir[2]<=1200000)) &&
		 ((s32_RgnDTLSum_iir[3]>160000) && (s32_RgnDTLSum_iir[3]<=360000)&&(u32_RgnDTLDev_iir[3]>450000)  && (u32_RgnDTLDev_iir[3]<=1200000))	)
		 &&
		 ((RmvDevX_iir_stable_csae >= 3) && (RmvDevY_iir_stable_csae >= 3) && (u32_RgnRmvDevX_iir[3] > 800))
		 &&
		 (((s32_RgnRmvSumX_iir[0] > -60 && s32_RgnRmvSumX_iir[0] <= -10)&&(s32_RgnRmvSumY_iir[0] > -20 && s32_RgnRmvSumY_iir[0] <= 50)) &&
		 ((s32_RgnRmvSumX_iir[1] > -60 && s32_RgnRmvSumX_iir[1] <= -10)&&(s32_RgnRmvSumY_iir[1] > -25 && s32_RgnRmvSumY_iir[1] <= 50)) &&
		 ((s32_RgnRmvSumX_iir[2] > -80 && s32_RgnRmvSumX_iir[2] <= -10)&&(s32_RgnRmvSumY_iir[2] > -25 && s32_RgnRmvSumY_iir[2] <= 40)) &&
		 ((s32_RgnRmvSumX_iir[3] > -800 && s32_RgnRmvSumX_iir[3] <= -300)&&(s32_RgnRmvSumY_iir[3] > -160 && s32_RgnRmvSumY_iir[3] <= 0))	))
	{
		pOutput->u3_logo_dhRgnProc_H_Moving_case = 1;
	}else{
		pOutput->u3_logo_dhRgnProc_H_Moving_case = 0;
	}

	if(((s32_RgnDTLSum_iir[0]>400000) && (s32_RgnDTLSum_iir[0]<=700000)&&(u32_RgnDTLDev_iir[0]>1500000) && (u32_RgnDTLDev_iir[0]<=2200000)) &&
		 ((s32_RgnDTLSum_iir[1]>650000) && (s32_RgnDTLSum_iir[1]<=860000)&&(u32_RgnDTLDev_iir[1]>400000)  && (u32_RgnDTLDev_iir[1]<=1100000)) &&
		 ((s32_RgnDTLSum_iir[2]>500000) && (s32_RgnDTLSum_iir[2]<=740000)&&(u32_RgnDTLDev_iir[2]>530000)  && (u32_RgnDTLDev_iir[2]<=1200000)) &&
		 ((s32_RgnDTLSum_iir[3]>160000) && (s32_RgnDTLSum_iir[3]<=360000)&&(u32_RgnDTLDev_iir[3]>450000)  && (u32_RgnDTLDev_iir[3]<=1200000))	)
	{
		test_case1 = 1;
	}

	if((RmvDevX_iir_stable_csae >= 3) && (RmvDevY_iir_stable_csae >= 3) && (u32_RgnRmvDevX_iir[3] > 800))
	{
		test_case2 = 1;
	}

	if(((s32_RgnRmvSumX_iir[0] > -60 && s32_RgnRmvSumX_iir[0] <= -10)&&(s32_RgnRmvSumY_iir[0] > -20 && s32_RgnRmvSumY_iir[0] <= 50)) &&
		 ((s32_RgnRmvSumX_iir[1] > -60 && s32_RgnRmvSumX_iir[1] <= -10)&&(s32_RgnRmvSumY_iir[1] > -25 && s32_RgnRmvSumY_iir[1] <= 50)) &&
		 ((s32_RgnRmvSumX_iir[2] > -80 && s32_RgnRmvSumX_iir[2] <= -10)&&(s32_RgnRmvSumY_iir[2] > -25 && s32_RgnRmvSumY_iir[2] <= 40)) &&
		 ((s32_RgnRmvSumX_iir[3] > -800 && s32_RgnRmvSumX_iir[3] <= -300)&&(s32_RgnRmvSumY_iir[3] > -160 && s32_RgnRmvSumY_iir[3] <= 0))	)
	{
		test_case3 = 1;
	}



#if FUNC_LOG
	ReadRegister(SOFTWARE1_SOFTWARE1_21_reg, 29, 29, &printkEnable);
	ReadRegister(SOFTWARE1_SOFTWARE1_21_reg, 24, 27, &printkPeriod);
	printkPeriod = (printkPeriod+1);//*8;

	if( printkCounter == 0 && printkEnable == 1 )
	{
		rtd_pr_memc_info("H_Moving_case = %5d ( %d, %d, %d, )\n",pOutput->u3_logo_dhRgnProc_H_Moving_case, test_case1, test_case2, test_case3);	
	}

				
	if( printkCounter == 0 && printkEnable == 1 )
	{
		rtd_pr_memc_info("[DEV]RgnDevX = ( %5d %5d %5d %5d ), RgnDevY = ( %5d %5d %5d %5d ), RgnDevDTL = ( %5d %5d %5d %5d )\n",
				u32_RgnRmvDevX[0],u32_RgnRmvDevX[1],u32_RgnRmvDevX[2],u32_RgnRmvDevX[3],
				u32_RgnRmvDevY[0],u32_RgnRmvDevY[1],u32_RgnRmvDevY[2],u32_RgnRmvDevY[3],
				u32_RgnDTLDev[0],u32_RgnDTLDev[1],u32_RgnDTLDev[2],u32_RgnDTLDev[3] );	

		rtd_pr_memc_info("[DEV_iir]RgnDevX_iir = ( %5d %5d %5d %5d ), RgnDevY_iir = ( %5d %5d %5d %5d ), RgnDevDTL_iir = ( %5d %5d %5d %5d )\n",
				u32_RgnRmvDevX_iir[0],u32_RgnRmvDevX[1],u32_RgnRmvDevX[2],u32_RgnRmvDevX[3],
				u32_RgnRmvDevY_iir[0],u32_RgnRmvDevY[1],u32_RgnRmvDevY[2],u32_RgnRmvDevY[3],
				u32_RgnDTLDev_iir[0],u32_RgnDTLDev_iir[1],u32_RgnDTLDev_iir[2],u32_RgnDTLDev_iir[3] );	
	

		rtd_pr_memc_info("[Sum]RmvSumX = ( %5d %5d %5d %5d ), RmvSumY = ( %5d %5d %5d %5d ), DTLSum = ( %5d %5d %5d %5d )\n",
				s32_RgnRmvSumX[0],s32_RgnRmvSumX[1],s32_RgnRmvSumX[2],s32_RgnRmvSumX[3],
				s32_RgnRmvSumY[0],s32_RgnRmvSumY[1],s32_RgnRmvSumY[2],s32_RgnRmvSumY[3],
				s32_RgnDTLSum[0],s32_RgnDTLSum[1],s32_RgnDTLSum[2],s32_RgnDTLSum[3]);	
		
		rtd_pr_memc_info("[Sum_iir]RmvSumX_iir = ( %5d %5d %5d %5d ), RmvSumY_iir = ( %5d %5d %5d %5d ), SumDTL_iir = ( %5d %5d %5d %5d )\n",
				s32_RgnRmvSumX_iir[0],s32_RgnRmvSumX_iir[1],s32_RgnRmvSumX_iir[2],s32_RgnRmvSumX_iir[3],
				s32_RgnRmvSumY_iir[0],s32_RgnRmvSumY_iir[1],s32_RgnRmvSumY_iir[2],s32_RgnRmvSumY_iir[3],
				s32_RgnDTLSum_iir[0],s32_RgnDTLSum_iir[1],s32_RgnDTLSum_iir[2],s32_RgnDTLSum_iir[3]);	
				
				
	}

	printkCounter = printkCounter + 1;
	if( printkCounter >= printkPeriod )
		printkCounter = 0;
	
#endif	


#ifdef FUNC_LOG
#undef FUNC_LOG
#endif

}

VOID FRC_DhLgProc_DynamicLevel(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
#ifdef FUNC_LOG
#undef FUNC_LOG
#endif
#define FUNC_LOG 1

#if FUNC_LOG
	// print related code
	static unsigned char printkCounter = 0;
	unsigned int printkEnable = 0;
	unsigned int printkPeriod = 1;
	unsigned int u1_force_dhProc_lvl_en[6] = {0};
	unsigned int u3_force_dhProc_lvl[6] = {0};
#endif


	const _PQLCONTEXT *s_pContext = GetPQLContext();
	const unsigned char RgnIdxREMAP[32] = // current only separate the region0/1, but dehalo logo process has 0/1/2 regions
	{
		0,	0,	1,	1,	1,	1,	2,	2,
		1,	1,	1,	1,	1,	1,	1,	1,
		1,	1,	1,	1,	1,	1,	1,	1,
		3,	3,	1,	4,	4,	1,	5,	5
	};

	unsigned char RgnIdx = 0, i = 0;
	static unsigned int u32_RgnZmvCnt[32] = {0};
	unsigned int u32_1stGmvZDiff = 0;
	unsigned int u32_2ndGmvZDiff = 0;
	unsigned int ZmvDiffThr = 6;

	unsigned int u16_LogoCnt[6] = {0};
	unsigned int u16_ZmvCnt[6] = {0};
	static unsigned int u16_LogoCnt_iir[6] = {0};
	static unsigned int u16_ZmvCnt_iir[6] = {0};
	unsigned int u32_SumLogoCnt = 0;
	unsigned int u32_SumZmvCnt = 0;

	int s32_RgnRmvSumX[6] = {0}, s32_RgnRmvSumY[6] = {0};
	unsigned int u32_RgnRmvDevX[6] = {0}, u32_RgnRmvDevY[6] = {0};
	static unsigned int u32_RgnRmvDevX_iir[6] = {0}, u32_RgnRmvDevY_iir[6] = {0};
	static unsigned int s32_RgnRmvSumX_iir[6] = {0}, s32_RgnRmvSumY_iir[6] = {0};

	int s32_ZmvLogoDiff = 0;
	const dhProcLvlThr LvlThr[3] =
	{
		// /*DC = don't care*/
		// level up
		// 1. ZmvCnt <= LogoCnt * Z2L_IncRatio/64
		// 2. ZmvCnt - LogoCnt <= Inc_Z2LDiff
		// 3. Mv Deviation <= Inc_DevVal
		// level down
		// 1. ZmvCnt >= LogoCnt * (1+(Z2L_IncRatio/64))
		// 2. ZmvCnt - LogoCnt >= IncAbsVal
		// 3. Mv Deviation >= Dec_DevVal
		//		Inc_Z2LRatio	Dec_Z2LRatio	Inc_Z2LDiff	Dec_Z2LDiff	Inc_DevVal	Dec_DevVal
		/*0*/ {	50,				0/*DC*/,		-600,		0/*DC*/,	6400,		0/*DC*/	}, // default level (weak)
		/*1*/ {	5,				68,				-3000,		400,		2800,		6600	},
		/*2*/ {	0/*DC*/,		12,				0/*DC*/,	-2800,		0/*DC*/,	3000	}  // strong (strictly panning)
	};


#if FUNC_LOG
	ReadRegister(SOFTWARE1_SOFTWARE1_21_reg, 14, 17, &ZmvDiffThr);
	ReadRegister(SOFTWARE1_SOFTWARE1_21_reg, 29, 29, &printkEnable);
	ReadRegister(SOFTWARE1_SOFTWARE1_21_reg, 24, 27, &printkPeriod);
	printkPeriod = (printkPeriod+1);//*8;

	for(RgnIdx=0; RgnIdx<32; RgnIdx++)
	{
		// 240*135 domain
		u32_1stGmvZDiff = _ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[RgnIdx]) + _ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[RgnIdx]);
		u32_2ndGmvZDiff = _ABS_(s_pContext->_output_read_comreg.s11_me_rMV_2nd_vx_rb[RgnIdx]) + _ABS_(s_pContext->_output_read_comreg.s10_me_rMV_2nd_vy_rb[RgnIdx]);

		if( printkCounter == 0 && printkEnable == 1 )
		{
			rtd_pr_memc_info("[%2d] z1=%5d (%5d %5d) (%5d) (%4d) z2=%5d (%5d %5d) (%5d) (%4d) p=%4d t=%4d sad=%4d %d\n",
				RgnIdx,
				s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[RgnIdx], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[RgnIdx], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[RgnIdx], u32_1stGmvZDiff, s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[RgnIdx],
				s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[RgnIdx], s_pContext->_output_read_comreg.s11_me_rMV_2nd_vx_rb[RgnIdx], s_pContext->_output_read_comreg.s10_me_rMV_2nd_vy_rb[RgnIdx], u32_2ndGmvZDiff, s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[RgnIdx],
				s_pContext->_output_read_comreg.u13_me_rPRD_rb[RgnIdx], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[RgnIdx]+s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[RgnIdx]+s_pContext->_output_read_comreg.u13_me_rPRD_rb[RgnIdx],
				s_pContext->_output_read_comreg.u25_me_rSAD_rb[RgnIdx]/1020, s_pContext->_output_read_comreg.u25_me_rSAD_rb[RgnIdx]
			);
		}
	}
#endif

	for(RgnIdx=0; RgnIdx<32; RgnIdx++)
	{
		s32_RgnRmvSumX[RgnIdxREMAP[RgnIdx]] = s32_RgnRmvSumX[RgnIdxREMAP[RgnIdx]] + s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[RgnIdx];
		s32_RgnRmvSumY[RgnIdxREMAP[RgnIdx]] = s32_RgnRmvSumY[RgnIdxREMAP[RgnIdx]] + s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[RgnIdx];
	}

	for(RgnIdx=0; RgnIdx<32; RgnIdx++)
	{
		// deviation
		if( RgnIdxREMAP[RgnIdx] == 0 )
		{
			u32_RgnRmvDevX[0] = u32_RgnRmvDevX[0] + _ABS_DIFF_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[RgnIdx]*2,s32_RgnRmvSumX[0]);
			u32_RgnRmvDevY[0] = u32_RgnRmvDevY[0] + _ABS_DIFF_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[RgnIdx]*2,s32_RgnRmvSumY[0]);
		}		
		else if( RgnIdxREMAP[RgnIdx] == 2 )
		{
			u32_RgnRmvDevX[2] = u32_RgnRmvDevX[2] + _ABS_DIFF_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[RgnIdx]*2,s32_RgnRmvSumX[2]);
			u32_RgnRmvDevY[2] = u32_RgnRmvDevY[2] + _ABS_DIFF_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[RgnIdx]*2,s32_RgnRmvSumY[2]);		
		}
		else if( RgnIdxREMAP[RgnIdx] == 3 )
		{
			u32_RgnRmvDevX[3] = u32_RgnRmvDevX[3] + _ABS_DIFF_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[RgnIdx]*2,s32_RgnRmvSumX[3]);
			u32_RgnRmvDevY[3] = u32_RgnRmvDevY[3] + _ABS_DIFF_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[RgnIdx]*2,s32_RgnRmvSumY[3]);	
		}
		else if( RgnIdxREMAP[RgnIdx] == 4 )
		{
			u32_RgnRmvDevX[4] = u32_RgnRmvDevX[4] + _ABS_DIFF_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[RgnIdx]*2,s32_RgnRmvSumX[4]);
			u32_RgnRmvDevY[4] = u32_RgnRmvDevY[4] + _ABS_DIFF_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[RgnIdx]*2,s32_RgnRmvSumY[4]);			
		}	
		else if( RgnIdxREMAP[RgnIdx] == 5 )	
		{
			u32_RgnRmvDevX[5] = u32_RgnRmvDevX[5] + _ABS_DIFF_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[RgnIdx]*2,s32_RgnRmvSumX[5]);
			u32_RgnRmvDevY[5] = u32_RgnRmvDevY[5] + _ABS_DIFF_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[RgnIdx]*2,s32_RgnRmvSumY[5]);			
		}
		else
		{
			u32_RgnRmvDevX[1] = u32_RgnRmvDevX[1] + _ABS_DIFF_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[RgnIdx]*22,s32_RgnRmvSumX[1]);
			u32_RgnRmvDevY[1] = u32_RgnRmvDevY[1] + _ABS_DIFF_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[RgnIdx]*22,s32_RgnRmvSumY[1]);
		}

		// 240*135 domain
		u32_1stGmvZDiff = _ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[RgnIdx]) + _ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[RgnIdx]);
		u32_2ndGmvZDiff = _ABS_(s_pContext->_output_read_comreg.s11_me_rMV_2nd_vx_rb[RgnIdx]) + _ABS_(s_pContext->_output_read_comreg.s10_me_rMV_2nd_vy_rb[RgnIdx]);

		// there are only 990 block at last statistic region
		if( RgnIdx>=24 )
			u32_RgnZmvCnt[RgnIdx] = 990;
		else
			u32_RgnZmvCnt[RgnIdx] = 1020;

		// guess the zmv amount
		if( u32_1stGmvZDiff > ZmvDiffThr && s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[RgnIdx] != 0 )
		{
			u32_RgnZmvCnt[RgnIdx] = (u32_RgnZmvCnt[RgnIdx] >= s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[RgnIdx])? u32_RgnZmvCnt[RgnIdx]-s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[RgnIdx] : 0;
		}
		if( u32_2ndGmvZDiff > ZmvDiffThr && s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[RgnIdx] != 0 )
		{
			u32_RgnZmvCnt[RgnIdx] = (u32_RgnZmvCnt[RgnIdx] >= s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[RgnIdx])? u32_RgnZmvCnt[RgnIdx]-s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[RgnIdx] : 0;
		}
		u32_RgnZmvCnt[RgnIdx] = (u32_RgnZmvCnt[RgnIdx] >= s_pContext->_output_read_comreg.u13_me_rPRD_rb[RgnIdx])? u32_RgnZmvCnt[RgnIdx]-s_pContext->_output_read_comreg.u13_me_rPRD_rb[RgnIdx] :  0;

		// add the logo count and zmv count to each region counter
		u16_LogoCnt[RgnIdxREMAP[RgnIdx]] = u16_LogoCnt[RgnIdxREMAP[RgnIdx]] + s_pContext->_output_read_comreg.u10_blklogo_rgcnt[RgnIdx];
		u16_ZmvCnt[RgnIdxREMAP[RgnIdx]] = u16_ZmvCnt[RgnIdxREMAP[RgnIdx]] + u32_RgnZmvCnt[RgnIdx];

#if FUNC_LOG
		if( printkCounter == 0 && printkEnable == 1 )
		{
			rtd_pr_memc_info("[%2d] z=%4d l=%4d ", RgnIdx, u32_RgnZmvCnt[RgnIdx], s_pContext->_output_read_comreg.u10_blklogo_rgcnt[RgnIdx]);
			if( (RgnIdx+1)%8 == 0 )
				rtd_pr_memc_info("\n");
		}
#endif

	}

	// iir process
	for(i=0; i<6; i++)
	{
		u16_LogoCnt_iir[i] = (u16_LogoCnt[i] + u16_LogoCnt_iir[i]) >> 1;
		u16_ZmvCnt_iir[i] = (u16_ZmvCnt[i] + u16_ZmvCnt_iir[i]) >> 1;
		u32_SumLogoCnt = u32_SumLogoCnt + u16_LogoCnt_iir[i];
		u32_SumZmvCnt = u32_SumZmvCnt + u16_ZmvCnt_iir[i];
		u32_RgnRmvDevX_iir[i] = (u32_RgnRmvDevX_iir[i] + u32_RgnRmvDevX[i]) >> 1;
		u32_RgnRmvDevY_iir[i] = (u32_RgnRmvDevY_iir[i] + u32_RgnRmvDevY[i]) >> 1;
		s32_RgnRmvSumX_iir[i] = (s32_RgnRmvSumX_iir[i] + s32_RgnRmvSumX[i]) >> 1;
		s32_RgnRmvSumY_iir[i] = (s32_RgnRmvSumY_iir[i] + s32_RgnRmvSumY[i]) >> 1;
	}
	s32_ZmvLogoDiff = (u32_SumZmvCnt-u32_SumLogoCnt);

#if FUNC_LOG
	if( printkCounter == 0 && printkEnable == 1 )
	{
		rtd_pr_memc_info("SumZ=%5d, SumL=%5d, Diff=%d\n", u32_SumZmvCnt, u32_SumLogoCnt, s32_ZmvLogoDiff);
			rtd_pr_memc_info("RgnZ=%5d %5d %5d %5d %5d %5d,\n",u16_ZmvCnt_iir[0], u16_ZmvCnt_iir[1],u16_ZmvCnt_iir[2],u16_ZmvCnt_iir[3],u16_ZmvCnt_iir[4],u16_ZmvCnt_iir[5]);
			rtd_pr_memc_info("RgnL=%5d %5d %5d %5d %5d %5d,\n",u16_LogoCnt_iir[0], u16_LogoCnt_iir[1],u16_LogoCnt_iir[2], u16_LogoCnt_iir[3],u16_LogoCnt_iir[4], u16_LogoCnt_iir[5] );
	#if 1
			rtd_pr_memc_info("RgnDevX = ( %5d %5d %5d %5d %5d %5d ), RgnDevY = ( %5d %5d %5d %5d %5d %5d )\n",
					u32_RgnRmvDevX[0],u32_RgnRmvDevX[1],u32_RgnRmvDevX[2],u32_RgnRmvDevX[3],u32_RgnRmvDevX[4],u32_RgnRmvDevX[5],
					u32_RgnRmvDevY[0],u32_RgnRmvDevY[1],u32_RgnRmvDevY[2],u32_RgnRmvDevY[3],u32_RgnRmvDevY[4],u32_RgnRmvDevY[5]);	
		
			rtd_pr_memc_info("RmvSumX = ( %5d %5d %5d %5d %5d %5d ), RmvSumY = ( %5d %5d %5d %5d %5d %5d )\n",
					s32_RgnRmvSumX_iir[0],s32_RgnRmvSumX_iir[1],s32_RgnRmvSumX_iir[2],s32_RgnRmvSumX_iir[3],s32_RgnRmvSumX_iir[4],s32_RgnRmvSumX_iir[5],
					s32_RgnRmvSumY_iir[0],s32_RgnRmvSumY_iir[1],s32_RgnRmvSumY_iir[2],s32_RgnRmvSumY_iir[3],s32_RgnRmvSumY_iir[4],s32_RgnRmvSumY_iir[5]);				
			
			rtd_pr_memc_info("RgnDeviir = (%5d %5d ) %5d, (%5d %5d ) %5d, (%5d %5d ) %5d, (%5d %5d ) %5d, (%5d %5d ) %5d, (%5d %5d ) %5d\n",
				u32_RgnRmvDevX_iir[0]*16, u32_RgnRmvDevY_iir[0]*16, (u32_RgnRmvDevX_iir[0]+u32_RgnRmvDevY_iir[0])*16,(u32_RgnRmvDevX_iir[1]*16)/11, (u32_RgnRmvDevY_iir[1]*16)/11, ((u32_RgnRmvDevX_iir[1]+u32_RgnRmvDevY_iir[1])*16)/11,
				u32_RgnRmvDevX_iir[2]*16, u32_RgnRmvDevY_iir[2]*16, (u32_RgnRmvDevX_iir[2]+u32_RgnRmvDevY_iir[2])*16,u32_RgnRmvDevX_iir[3]*16, u32_RgnRmvDevY_iir[3]*16, (u32_RgnRmvDevX_iir[3]+u32_RgnRmvDevY_iir[3])*16,
				u32_RgnRmvDevX_iir[4]*16, u32_RgnRmvDevY_iir[4]*16, (u32_RgnRmvDevX_iir[4]+u32_RgnRmvDevY_iir[4])*16,u32_RgnRmvDevX_iir[5]*16, u32_RgnRmvDevY_iir[5]*16, (u32_RgnRmvDevX_iir[5]+u32_RgnRmvDevY_iir[5])*16
				);
	#else
		
		rtd_pr_memc_info("RgnDev = (%5d %5d) %5d (%5d %5d) %5d\n",
			u32_RgnRmvDevX_iir[0]*4, u32_RgnRmvDevY_iir[0]*4, (u32_RgnRmvDevX_iir[0]+u32_RgnRmvDevY_iir[0])*4,
			u32_RgnRmvDevX_iir[1]*4/3, u32_RgnRmvDevY_iir[1]*4/3, (u32_RgnRmvDevX_iir[1]+u32_RgnRmvDevY_iir[1])*4/3
		);
	#endif	
			rtd_pr_memc_info("rgnclv=%1d %1d %1d %1d %1d %1d\n", pOutput->u3_logo_dhRgnProc_lvl[0], pOutput->u3_logo_dhRgnProc_lvl[1], pOutput->u3_logo_dhRgnProc_lvl[2], pOutput->u3_logo_dhRgnProc_lvl[3]
										, pOutput->u3_logo_dhRgnProc_lvl[4], pOutput->u3_logo_dhRgnProc_lvl[5]);
	}
#endif
		//RgnProc_case control
		if((u16_LogoCnt_iir[2]>200 && u16_LogoCnt_iir[1] < 100) && (u16_ZmvCnt_iir[2]<=20) && (u32_RgnRmvDevX[2]<=30 && u32_RgnRmvDevY[2]<=20))
		{
			pOutput->u3_logo_dhRgnProc_case[2] = 1;
			
		}else if((u16_LogoCnt_iir[0]<60 && u16_LogoCnt_iir[1]>=190 && u16_LogoCnt_iir[2]<110 && u16_LogoCnt_iir[4]<10) && 
				 (u16_LogoCnt_iir[3]>130 && u16_LogoCnt_iir[3]<=350) && 
				 (u16_LogoCnt_iir[5]>250 && u16_LogoCnt_iir[5]<550)){
	
			pOutput->u3_logo_dhRgnProc_case[3] = 1;
			
		}else{
			
			for(i=0; i<6; i++){
				pOutput->u3_logo_dhRgnProc_case[i] = 0;
			}		
	
		}
#if FUNC_LOG
		if( printkCounter == 0 && printkEnable == 1 )
		{
			rtd_pr_memc_info("hRgnProc_case = ( %5d %5d %5d %5d %5d %5d )\n",
					pOutput->u3_logo_dhRgnProc_case[0],pOutput->u3_logo_dhRgnProc_case[1],pOutput->u3_logo_dhRgnProc_case[2],pOutput->u3_logo_dhRgnProc_case[3],
					pOutput->u3_logo_dhRgnProc_case[4],pOutput->u3_logo_dhRgnProc_case[5]); 
		}	
#endif


	// region condition
	if( u32_SumZmvCnt >= (((240*135)*14)>>4) || s_pContext->_output_me_sceneAnalysis.u2_panning_flag !=1 || 
		(s_pContext->_output_read_comreg.u10_blklogo_rgcnt[1]<=400 && s_pContext->_output_read_comreg.u10_blklogo_rgcnt[7]<=400)) // still image and non-panning
	{
		for(i=0; i<6; i++){
			pOutput->u3_logo_dhRgnProc_lvl[i] = 0;
		}
	}
	else
	{
		// region 0 condition

		// region 1 condition
		// deviation very large
		// some region with logo and zmv appears
#if FUNC_LOG
		if( printkCounter == 0 && printkEnable == 1 )
		{
			rtd_pr_memc_info("inc %1d %1d %1d\n",
				( (u16_ZmvCnt_iir[1]*64) <= (u16_LogoCnt_iir[1]*LvlThr[pOutput->u3_logo_dhRgnProc_lvl[1]].Inc_Z2LRatio) ),
				( s32_ZmvLogoDiff <= LvlThr[pOutput->u3_logo_dhRgnProc_lvl[1]].Inc_Z2LDiff ),
				( ((u32_RgnRmvDevX_iir[1]+u32_RgnRmvDevY_iir[1])*16/11) <= LvlThr[pOutput->u3_logo_dhRgnProc_lvl[1]].Inc_DevVal )
			);
			rtd_pr_memc_info("dec %1d %1d %1d\n",
				( (u16_ZmvCnt_iir[1]*64) >= (u16_LogoCnt_iir[1]*LvlThr[pOutput->u3_logo_dhRgnProc_lvl[1]].Dec_Z2LRatio) ),
				( s32_ZmvLogoDiff >= LvlThr[pOutput->u3_logo_dhRgnProc_lvl[1]].Dec_Z2LDiff ),
				( ((u32_RgnRmvDevX_iir[1]+u32_RgnRmvDevY_iir[1])*16/11) >= LvlThr[pOutput->u3_logo_dhRgnProc_lvl[1]].Dec_DevVal )
			);
		}
#endif
		if( (u32_RgnRmvDevX_iir[1]+u32_RgnRmvDevY_iir[1])*4/3 > 8300 ) // complex scene
		{
			pOutput->u3_logo_dhRgnProc_lvl[1] = 0;
		}
		else if( ((u32_RgnRmvDevX_iir[1]+u32_RgnRmvDevY_iir[1])*4/3) <= 200 ) // bamtori
		{
			pOutput->u3_logo_dhRgnProc_lvl[1] = 2;
		}
		else if( ( (u16_ZmvCnt_iir[1]*64) <= (u16_LogoCnt_iir[1]*LvlThr[pOutput->u3_logo_dhRgnProc_lvl[1]].Inc_Z2LRatio) ) &&
			( s32_ZmvLogoDiff <= LvlThr[pOutput->u3_logo_dhRgnProc_lvl[1]].Inc_Z2LDiff ) &&
			( ((u32_RgnRmvDevX_iir[1]+u32_RgnRmvDevY_iir[1])*4/3) <= LvlThr[pOutput->u3_logo_dhRgnProc_lvl[1]].Inc_DevVal ) ) // inc
		{
			pOutput->u3_logo_dhRgnProc_lvl[1] = (pOutput->u3_logo_dhRgnProc_lvl[1]<2)? pOutput->u3_logo_dhRgnProc_lvl[1]+1 : 2;
		}
		else if( ( (u16_ZmvCnt_iir[1]*64) >= (u16_LogoCnt_iir[1]*LvlThr[pOutput->u3_logo_dhRgnProc_lvl[1]].Dec_Z2LRatio) ) &&
				 ( s32_ZmvLogoDiff >= LvlThr[pOutput->u3_logo_dhRgnProc_lvl[1]].Dec_Z2LDiff ) &&
				 ( ((u32_RgnRmvDevX_iir[1]+u32_RgnRmvDevY_iir[1])*4/3) >= LvlThr[pOutput->u3_logo_dhRgnProc_lvl[1]].Dec_DevVal ) ) // dec
		{
			pOutput->u3_logo_dhRgnProc_lvl[1] = (pOutput->u3_logo_dhRgnProc_lvl[1]>1) ? pOutput->u3_logo_dhRgnProc_lvl[1]-1 : 0;
		}

		// temporally make it the same
		for(i=0; i<6; i++){
			if( (pOutput->u3_logo_dhRgnProc_lvl[i]!=pOutput->u3_logo_dhRgnProc_lvl[1]) && pOutput->u3_logo_dhRgnProc_lvl[1]!=0 ){
				pOutput->u3_logo_dhRgnProc_lvl[i] = pOutput->u3_logo_dhRgnProc_lvl[1];
			}
		}

	}

#if FUNC_LOG
#if 0
	
	ReadRegister(SOFTWARE1_SOFTWARE1_21_reg, 23, 23, &u1_force_dhProc_lvl_en[0]);
	ReadRegister(SOFTWARE1_SOFTWARE1_21_reg, 22, 22, &u1_force_dhProc_lvl_en[1]);
	
	ReadRegister(SOFTWARE1_SOFTWARE1_21_reg, 20, 21, &u3_force_dhProc_lvl[0]);
	ReadRegister(SOFTWARE1_SOFTWARE1_21_reg, 18, 19, &u3_force_dhProc_lvl[1]);
	if( u1_force_dhProc_lvl_en[0] != 0 )
	{
		pOutput->u3_logo_dhRgnProc_lvl[0] = _MIN_(u3_force_dhProc_lvl[0], 2);
	}
	if( u1_force_dhProc_lvl_en[1] != 0 )
	{
		pOutput->u3_logo_dhRgnProc_lvl[1] = _MIN_(u3_force_dhProc_lvl[1], 2);
	}

	if( printkCounter == 0 && printkEnable == 1 )
	{
		rtd_pr_memc_info("olv=%1d %1d, force=%1d %1d\n", pOutput->u3_logo_dhRgnProc_lvl[0], pOutput->u3_logo_dhRgnProc_lvl[1], u1_force_dhProc_lvl_en[0], u1_force_dhProc_lvl_en[1]);
	}

	
#else
		ReadRegister(SOFTWARE1_SOFTWARE1_21_reg, 23, 23, &u1_force_dhProc_lvl_en[0]);
		ReadRegister(SOFTWARE1_SOFTWARE1_21_reg, 22, 22, &u1_force_dhProc_lvl_en[1]);
		ReadRegister(SOFTWARE1_SOFTWARE1_25_reg, 0, 0, &u1_force_dhProc_lvl_en[2]);
		ReadRegister(SOFTWARE1_SOFTWARE1_25_reg, 1, 1, &u1_force_dhProc_lvl_en[3]);
		ReadRegister(SOFTWARE1_SOFTWARE1_25_reg, 2, 2, &u1_force_dhProc_lvl_en[4]);
		ReadRegister(SOFTWARE1_SOFTWARE1_25_reg, 3, 3, &u1_force_dhProc_lvl_en[5]);
		
		ReadRegister(SOFTWARE1_SOFTWARE1_21_reg, 20, 21, &u3_force_dhProc_lvl[0]);
		ReadRegister(SOFTWARE1_SOFTWARE1_21_reg, 18, 19, &u3_force_dhProc_lvl[1]);	
		ReadRegister(SOFTWARE1_SOFTWARE1_25_reg, 10, 11, &u3_force_dhProc_lvl[2]);
		ReadRegister(SOFTWARE1_SOFTWARE1_25_reg, 12, 13, &u3_force_dhProc_lvl[3]);
		ReadRegister(SOFTWARE1_SOFTWARE1_25_reg, 14, 15, &u3_force_dhProc_lvl[4]);
		ReadRegister(SOFTWARE1_SOFTWARE1_25_reg, 16, 17, &u3_force_dhProc_lvl[5]);
	
		if( u1_force_dhProc_lvl_en[0] != 0 )
		{
			pOutput->u3_logo_dhRgnProc_lvl[0] = _MIN_(u3_force_dhProc_lvl[0], 2);
		}
		if( u1_force_dhProc_lvl_en[1] != 0 )
		{
			pOutput->u3_logo_dhRgnProc_lvl[1] = _MIN_(u3_force_dhProc_lvl[1], 2);
		}
		if( u1_force_dhProc_lvl_en[2] != 0 )
		{
			pOutput->u3_logo_dhRgnProc_lvl[2] = _MIN_(u3_force_dhProc_lvl[2], 2);
		}
		if( u1_force_dhProc_lvl_en[3] != 0 )
		{
			pOutput->u3_logo_dhRgnProc_lvl[3] = _MIN_(u3_force_dhProc_lvl[3], 2);
		}	
		if( u1_force_dhProc_lvl_en[4] != 0 )
		{
			pOutput->u3_logo_dhRgnProc_lvl[4] = _MIN_(u3_force_dhProc_lvl[4], 2);
		}
		if( u1_force_dhProc_lvl_en[5] != 0 )
		{
			pOutput->u3_logo_dhRgnProc_lvl[5] = _MIN_(u3_force_dhProc_lvl[5], 2);
		}	
	
		if( printkCounter == 0 && printkEnable == 1 )
		{
			rtd_pr_memc_info("olv=%1d %1d %1d %1d %1d %1d, force=%1d %1d %1d %1d %1d %1d\n", 
					pOutput->u3_logo_dhRgnProc_lvl[0], pOutput->u3_logo_dhRgnProc_lvl[1], pOutput->u3_logo_dhRgnProc_lvl[2], pOutput->u3_logo_dhRgnProc_lvl[3],
					pOutput->u3_logo_dhRgnProc_lvl[4], pOutput->u3_logo_dhRgnProc_lvl[5], u1_force_dhProc_lvl_en[0], u1_force_dhProc_lvl_en[1],
					u1_force_dhProc_lvl_en[2], u1_force_dhProc_lvl_en[3], u1_force_dhProc_lvl_en[4], u1_force_dhProc_lvl_en[5]);
		}	
#endif

	printkCounter = printkCounter + 1;
	if( printkCounter >= printkPeriod )
		printkCounter = 0;
#endif


#ifdef FUNC_LOG
#undef FUNC_LOG
#endif
}

#ifdef CONFIG_SUPPORT_SLD_BY_MEMC_DEMURA
VOID FRC_SLD_PxlTargetYVal(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
	// consider the regional apl and dtl to set the regional target y value
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	// for regional apl/dtl with basic value
	unsigned char RgnIdx = 0;
	unsigned int RgnAPL = 0;
	unsigned int RgnDTL = 0;
	unsigned int RgnBlkNum = (240*135)/32;

	unsigned int LT_RgnAplAvg = (s_pContext->_output_read_comreg.u20_me_rAPLi_rb[0] + s_pContext->_output_read_comreg.u20_me_rAPLp_rb[0] +
						   s_pContext->_output_read_comreg.u20_me_rAPLi_rb[1] + s_pContext->_output_read_comreg.u20_me_rAPLp_rb[1] +
						   s_pContext->_output_read_comreg.u20_me_rAPLi_rb[2] + s_pContext->_output_read_comreg.u20_me_rAPLp_rb[2])/6;
	unsigned int RgnAplAvgWithouCurBlk = 0;

	unsigned int LT_apl_l = pParam->u8_logo_rglt_pxldf_apl_thr_l; // 60
	unsigned int LT_apl_h = pParam->u8_logo_rglt_pxldf_apl_thr_h; // 220
	unsigned int LT_val_l = pParam->u8_logo_rglt_pxldf_val_thr_l; // 190
	unsigned int LT_val_h = pParam->u8_logo_rglt_pxldf_val_thr_h; // 230

	// modify by statistic
	unsigned char HistoIdx = 0;
	unsigned int u32_TotalHistNum = 1;
	unsigned int u32_HistoPartial_H = 0, u8_Ratio_H = 0; // high apl part of the histogram
	unsigned int u32_HistoPartial_M = 0, u8_Ratio_M = 0; // middle apl part of the histogram
	unsigned int u32_HistoPartial_L = 0, u8_Ratio_L = 0; // low apl part of the histogram
	unsigned char Hist2AplOft_H = 0, Hist2AplOft_M = 0, Hist2AplOft_L = 0;

#if 0
	static unsigned char printkCounter = 0;
	const unsigned char printkEnable = pParam->u1_logo_sld_debug_print_en;
	const unsigned int printkPeriod = (pParam->u1_logo_sld_debug_print_period + 1) * 16;
#endif
	if (MEMC_Pixel_LOGO_For_SW_SLD != 1)
		return;
	// -------------------- histogram --------------------
	for(HistoIdx=0; HistoIdx<32; HistoIdx++)
	{
		u32_TotalHistNum = u32_TotalHistNum + s_pContext->_output_read_comreg.u32_yuv_meter[HistoIdx];
		if(HistoIdx < 4)
			u32_HistoPartial_L = u32_HistoPartial_L + s_pContext->_output_read_comreg.u32_yuv_meter[HistoIdx];
		if(HistoIdx >= 4 && HistoIdx < 28)
			u32_HistoPartial_M = u32_HistoPartial_M + s_pContext->_output_read_comreg.u32_yuv_meter[HistoIdx];
		if(HistoIdx >= 28)
			u32_HistoPartial_H = u32_HistoPartial_H + s_pContext->_output_read_comreg.u32_yuv_meter[HistoIdx];
	}

	if( u32_TotalHistNum >= 1400*240 )
	{
		u8_Ratio_H = (u32_HistoPartial_H*100)/u32_TotalHistNum;
		u8_Ratio_M = (u32_HistoPartial_M*100)/u32_TotalHistNum;
		u8_Ratio_L = (u32_HistoPartial_L*100)/u32_TotalHistNum;

		if( u8_Ratio_H == 0 )
			Hist2AplOft_H = 99;

		Hist2AplOft_L = (u8_Ratio_L<=40)? 0 : _MIN_(u8_Ratio_L-40, 25);
	}

#if 0
	if( printkEnable == 1 && printkCounter == 0 )
	{
		for(HistoIdx=0; HistoIdx<32; HistoIdx++)
		{
			rtd_pr_memc_info("[%2d] %6d ", HistoIdx, s_pContext->_output_read_comreg.u32_yuv_meter[HistoIdx]);
			if((HistoIdx+1)%4==0)
				rtd_pr_memc_info("\n");
		}
		rtd_pr_memc_info("\n");
		rtd_pr_memc_info("TotalCnt = %d\n", u32_TotalHistNum);
		rtd_pr_memc_info("L = %8d, r = %d, o = %d\n", u32_HistoPartial_L, u8_Ratio_L, Hist2AplOft_L);
		rtd_pr_memc_info("M = %8d, r = %d, o = %d\n", u32_HistoPartial_M, u8_Ratio_M, Hist2AplOft_M);
		rtd_pr_memc_info("H = %8d, r = %d, o = %d\n", u32_HistoPartial_H, u8_Ratio_H, Hist2AplOft_H);
	}
#endif

	// -------------------- region target value --------------------
	for(RgnIdx = 0; RgnIdx<3; RgnIdx++)
	{
		RgnAPL = (s_pContext->_output_read_comreg.u20_me_rAPLi_rb[RgnIdx] + s_pContext->_output_read_comreg.u20_me_rAPLp_rb[RgnIdx]) >> 1;
		RgnDTL = s_pContext->_output_read_comreg.u20_me_rDTL_rb[RgnIdx];
		RgnAplAvgWithouCurBlk = (LT_RgnAplAvg*6-s_pContext->_output_read_comreg.u20_me_rAPLi_rb[RgnIdx]-s_pContext->_output_read_comreg.u20_me_rAPLp_rb[RgnIdx]) >> 2;

#if 0
		// current rgn apl different from others too much
		if( printkEnable == 1 && printkCounter == 0 )
		{
			rtd_pr_memc_info("[%d] RgnAplwoCur = %d thr = %d curApl = %d\n", RgnIdx, RgnAplAvgWithouCurBlk, (80*RgnBlkNum), RgnAPL);
		}
#endif

		if( RgnAplAvgWithouCurBlk > RgnAPL && (RgnAplAvgWithouCurBlk-RgnAPL) >= (80*RgnBlkNum) )
		{
			RgnAPL = RgnAplAvgWithouCurBlk;
			//RgnAPL = (RgnAPL*3+LT_RgnAplAvg*5) >> 3;
#if 0
			if( printkEnable == 1 && printkCounter == 0 )
			{
				rtd_pr_memc_info("[%d] FixAPL = %d (%d)\n", RgnIdx, RgnAPL, (RgnAPL*3+LT_RgnAplAvg*5) >> 3);
			}
#endif
		}

		// get the basic output value by apl
		if( RgnDTL >= 8000/*10000*/ )
		{
			if( RgnAPL >= LT_apl_h*RgnBlkNum)
				pOutput->u8_PxlRgnDfy[RgnIdx] = LT_val_h;
			else if(RgnAPL <= LT_apl_l*RgnBlkNum)
				pOutput->u8_PxlRgnDfy[RgnIdx] = LT_val_l;
			else
				pOutput->u8_PxlRgnDfy[RgnIdx] = ((LT_val_h-LT_val_l)*(RgnAPL-LT_apl_l*RgnBlkNum))/((LT_apl_h-LT_apl_l)*RgnBlkNum) + LT_val_l;
		}
		else
		{
			pOutput->u8_PxlRgnDfy[RgnIdx] = LT_val_h;
#if 0
			if( printkEnable == 1 && printkCounter == 0 )
			{
				rtd_pr_memc_info("[%d] dtl low, fix value\n", RgnIdx);
			}
#endif
		}
#if 0
		if( printkEnable == 1 && printkCounter == 0 )
		{
			rtd_pr_memc_info("[%d] RgnBlks = %d apl bound = %d %d\n", RgnIdx, RgnBlkNum, LT_apl_h*RgnBlkNum, LT_apl_l*RgnBlkNum);
			rtd_pr_memc_info("[%d] apl = %d, dtl = %d, value = %d\n", RgnIdx, RgnAPL, RgnDTL, pOutput->u8_PxlRgnDfy[RgnIdx]);
		}
#endif
	}

	// -------------------- decide the final target value --------------------
	for(RgnIdx = 0; RgnIdx<3; RgnIdx++)
	{
		pOutput->u8_PxlRgnDfy[RgnIdx] = pOutput->u8_PxlRgnDfy[RgnIdx] + Hist2AplOft_L + Hist2AplOft_M + Hist2AplOft_H;
		pOutput->u8_PxlRgnDfy[RgnIdx] = _CLIP_(pOutput->u8_PxlRgnDfy[RgnIdx], LT_val_l, LT_val_h);
#if 0
		if( printkEnable == 1 && printkCounter == 0 )
		{
			rtd_pr_memc_info("[%d] final val = %d\n", RgnIdx, pOutput->u8_PxlRgnDfy[RgnIdx]);
		}
#endif
	}
#if 0
	printkCounter = printkCounter + 1;
	if( printkCounter >= printkPeriod )
		printkCounter = 0;
#endif
}

VOID FRC_SLD_PxlRgnClr(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
	// cond1. backgournd and logo are the same > clear
	// cond2. flat / motion / too much logo > clear
	const _PQLCONTEXT *s_pContext = GetPQLContext();
#if 0
	static unsigned char printkCounter = 0;
	const unsigned char printkEnable = pParam->u1_logo_sld_debug_print_en;
	const unsigned int printkPeriod = (pParam->u1_logo_sld_debug_print_period + 1) * 16;
#endif
	unsigned char RgnIdx = 0;

	static unsigned int u32_RgnApl[32] = {0};
	static unsigned int u32_RgnDtl[32] = {0};
	static unsigned int u32_RgnZmvCnt[32] = {0};
	static unsigned int u32_RgnPxlCnt[32] = {0};

	unsigned int u32_1stGmvZDiff = 0;
	unsigned int u32_2ndGmvZDiff = 0;
	unsigned int u32_1st2ndGmvDiff = 0;
	//unsigned char u3_RgnLogoSmlMvStatus = 0;

	const unsigned int u32_PxlRgClrCntThr = ((960*540*pParam->u32_logo_rgclr_cnt_thr)/100)/32;
	unsigned int u32_RgnClrCmpThr = 0;

	// output
	unsigned int u32_ClrSigByApl = 0;
	unsigned int u32_ClrSigByDtl = 0;
	unsigned int u32_ClrSigByCnt = 0;
	unsigned int u32_ClrSigByCmp = 0;

	if ((MEMC_Pixel_LOGO_For_SW_SLD != 1) && (MEMC_Pixel_LOGO_For_SW_SLD != 2))
		return;

	// -------------------- Histogram --------------------
	for(RgnIdx=0; RgnIdx<32; RgnIdx++)
	{
		// 240*135 domain
		u32_RgnApl[RgnIdx] = 0;
		u32_RgnApl[RgnIdx] = (s_pContext->_output_read_comreg.u20_me_rAPLi_rb[RgnIdx] + s_pContext->_output_read_comreg.u20_me_rAPLp_rb[RgnIdx]+1012)/2024;

		// 240*135 domain
		u32_RgnDtl[RgnIdx] = 0;
		u32_RgnDtl[RgnIdx] = (s_pContext->_output_read_comreg.u20_me_rDTL_rb[RgnIdx]+506)/1024;

		// 960x540 domain
		u32_RgnPxlCnt[RgnIdx] = 0;
		u32_RgnPxlCnt[RgnIdx] = s_pContext->_output_read_comreg.u14_pxllogo_rgcnt[RgnIdx];

		// 240*135 domain
		u32_1stGmvZDiff = _ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[RgnIdx]) + _ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[RgnIdx]);
		u32_2ndGmvZDiff = _ABS_(s_pContext->_output_read_comreg.s11_me_rMV_2nd_vx_rb[RgnIdx]) + _ABS_(s_pContext->_output_read_comreg.s10_me_rMV_2nd_vy_rb[RgnIdx]);
		u32_1st2ndGmvDiff = _ABS_DIFF_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[RgnIdx] , s_pContext->_output_read_comreg.s11_me_rMV_2nd_vx_rb[RgnIdx]) +
							_ABS_DIFF_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[RgnIdx] , s_pContext->_output_read_comreg.s10_me_rMV_2nd_vy_rb[RgnIdx]);

		u32_RgnZmvCnt[RgnIdx] = 1020;
		if( u32_1stGmvZDiff > pParam->u8_logo_rgclr_logo_mv_thr )
		{
			u32_RgnZmvCnt[RgnIdx] = (u32_RgnZmvCnt[RgnIdx] >= s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[RgnIdx])? u32_RgnZmvCnt[RgnIdx]-s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[RgnIdx] : 0;
			//u3_RgnLogoSmlMvStatus = u3_RgnLogoSmlMvStatus | 0x1;
		}
		if( u32_2ndGmvZDiff > pParam->u8_logo_rgclr_logo_mv_thr )
		{
			u32_RgnZmvCnt[RgnIdx] = (u32_RgnZmvCnt[RgnIdx] >= s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[RgnIdx])? u32_RgnZmvCnt[RgnIdx]-s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[RgnIdx] : 0;
			//u3_RgnLogoSmlMvStatus = u3_RgnLogoSmlMvStatus | 0x2;
		}
#if 0
#if 1 // normal log

		if( printkEnable == 1 && printkCounter == 0 )
		{
			if((RgnIdx+1)%4==0)
				rtd_pr_memc_info("\n");
		}
#else // debugging detail log
		if( printkEnable == 1 && printkCounter == 0 && (RgnIdx<=7 || RgnIdx>=24) )
		{
			rtd_pr_memc_info("[%2d] (%5d %5d %4d) (%5d %5d %4d) final = %4d, flg = %d\n",
				RgnIdx,
				s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[RgnIdx], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[RgnIdx], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[RgnIdx],
				s_pContext->_output_read_comreg.s11_me_rMV_2nd_vx_rb[RgnIdx], s_pContext->_output_read_comreg.s10_me_rMV_2nd_vy_rb[RgnIdx], s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[RgnIdx],
				u32_RgnZmvCnt[RgnIdx], u3_RgnLogoSmlMvStatus
			);

		}
#endif
#endif

	}


	for(RgnIdx=0; RgnIdx<32; RgnIdx++)
	{
		// -------------------- clear by apl --------------------
		if( u32_RgnApl[RgnIdx] >= pParam->u32_logo_rgclr_apl_thr )
			u32_ClrSigByApl = (u32_ClrSigByApl | (1<<RgnIdx));

		// -------------------- clear by dtl --------------------
		if( u32_RgnDtl[RgnIdx] <= pParam->u32_logo_rgclr_dtl_thr && u32_RgnApl[RgnIdx] >= pParam->u32_logo_rgclr_apl2dtl_thr )
			u32_ClrSigByDtl = (u32_ClrSigByDtl | (1<<RgnIdx));

		// -------------------- clear by logo count --------------------
		if( u32_RgnPxlCnt[RgnIdx] >= u32_PxlRgClrCntThr )
			u32_ClrSigByCnt = (u32_ClrSigByCnt | (1<<RgnIdx));

		// -------------------- clear by logo and zmv count comparison --------------------
		u32_RgnClrCmpThr = ((u32_RgnZmvCnt[RgnIdx]*pParam->u32_logo_rgclr_cmp_gain_thr)/64 + pParam->u32_logo_rgclr_cmp_ofst_thr*8);
		if( (u32_RgnPxlCnt[RgnIdx]/16) >= u32_RgnClrCmpThr )
			u32_ClrSigByCmp = (u32_ClrSigByCmp | (1<<RgnIdx));

#if 0
#if 1
		if( printkEnable == 1 && printkCounter == 0 )
#else
		if( printkEnable == 1 && printkCounter == 0 && (RgnIdx<=7 || RgnIdx>=24) )
#endif
		{
			if((RgnIdx+1)%4==0)
				rtd_pr_memc_info("\n");
		}
#endif
	}

	pOutput->u32_PxlRgClrSig = 0;
	if( pParam->u1_logo_rgclr_apl_en == 1 )
		pOutput->u32_PxlRgClrSig = pOutput->u32_PxlRgClrSig | u32_ClrSigByApl;
	if( pParam->u1_logo_rgclr_dtl_en == 1 )
		pOutput->u32_PxlRgClrSig = pOutput->u32_PxlRgClrSig | u32_ClrSigByDtl;
	if( pParam->u1_logo_rgclr_cnt_en == 1 )
		pOutput->u32_PxlRgClrSig = pOutput->u32_PxlRgClrSig | u32_ClrSigByCnt;
	if( pParam->u1_logo_rgclr_cmp_en == 1 )
		pOutput->u32_PxlRgClrSig = pOutput->u32_PxlRgClrSig | u32_ClrSigByCmp;
#if 0
	if( printkEnable == 1 && printkCounter == 0 )
	{
		rtd_pr_memc_info("PxlCntThr = %d, Sig = apl[%08x] dtl[%08x] cnt[%08x] cmp[%08x] out[%08x]\n",
			u32_PxlRgClrCntThr,
			u32_ClrSigByApl, u32_ClrSigByDtl, u32_ClrSigByCnt, u32_ClrSigByCmp,
			pOutput->u32_PxlRgClrSig);
	}

	printkCounter = printkCounter + 1;
	if( printkCounter >= printkPeriod )
		printkCounter = 0;
#endif
}
#endif

VOID FRC_LogoDet_Proc(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{

	if (pParam->u1_logo_pql_en == 1)
	{
		//const _PQLCONTEXT *s_pContext		= GetPQLContext();
		pOutput->u32_blklogo_clr_idx		= 0;
		pOutput->u1_blkclr_glbstatus		= 0;
		pOutput->u1_logo_cnt_glb_clr_status = 0;
		pOutput->u1_logo_sc_glb_clr_status  = 0;
		pOutput->u1_sw_clr_en_out           = pParam->u1_logo_sw_clr_en;
		pOutput->u4_blk_pix_merge_type_out	= pParam->u4_blk_pix_merge_type;
		pOutput->u32_logo_hsty_clr_idx      = 0;
		pOutput->u32_logo_hsty_clr_lgclr_idx = 0;

		FRC_LgDet_RgHstyClr(pParam, pOutput);
		FRC_LgDet_BlkClrCtrl(pParam, pOutput);
		FRC_LgDet_netflix_detect(pParam, pOutput);
		FRC_LgDet_SCCtrl(pParam, pOutput);
		FRC_LgDet_CloseVar(pParam, pOutput);
		//FRC_LgDet_RgDhClr(pParam, pOutput);
		//FRC_LgDet_RgHstyClr(pParam, pOutput);
		FRC_LgDet_SC_Detect(pParam, pOutput);
		FRC_LgDet_SC_FastDetectionCtrl(pParam, pOutput);
		FRC_LgDet_RgnDistribute(pParam, pOutput);
		//FRC_LgDet_LG_16s_Patch(pParam, pOutput);
		//FRC_LgDet_RgDhAdp_Rimunbalance(pParam, pOutput);
		//FRC_LgDet_LG_UXN_Patch(pParam, pOutput);
#ifdef CONFIG_SUPPORT_SLD_BY_MEMC_DEMURA
		FRC_SLD_PxlTargetYVal(pParam, pOutput);
		FRC_SLD_PxlRgnClr(pParam, pOutput);
#endif
		FRC_LgDet_LogoHaloEnhance(pParam, pOutput);
		FRC_LgDet_DynamicOBME(pParam, pOutput);
		FRC_DhLgProc_DynamicLevel(pParam, pOutput);
		FRC_DhLgProc_H_MovingCase(pParam, pOutput);
		FRC_LgDet_BG_still_detect(pParam, pOutput);
		FRC_LgDet_KeyRgn_detect(pParam, pOutput);
		FRC_LgDet_pure_panning_detect(pParam, pOutput);
		FRC_LogoDet_NearRim_logo(pParam, pOutput);

	}
	else
	{
	    pOutput->u1_sw_clr_en_out = 1;
		pOutput->u4_blk_pix_merge_type_out = 0;
		pOutput->u32_blklogo_clr_idx  = 0;
		pOutput->u5_iir_alpha_out = 0x1F;
		pOutput->u32_logo_hsty_clr_idx      = 0;
		pOutput->u32_logo_hsty_clr_lgclr_idx = 0;

		pOutput->u1_logo_netflix_status = 0;
		pOutput->u8_logo_netflix_hold_frm = 0;

		pOutput->u1_logo_lg16s_patch_status   = 0;
		pOutput->u8_logo_lg16s_patch_hold_frm = 0;

		pOutput->u1_logo_lg16s_patch_Gapli_status     = 0;
		pOutput->u1_logo_lg16s_patch_Gaplp_status     = 0;
		pOutput->u1_logo_lg16s_patch_Gdtl_status     = 0;
		pOutput->u1_logo_lg16s_patch_Gsad_status     = 0;
		pOutput->u1_logo_lg16s_patch_Gmv_status      = 0;
		pOutput->u1_logo_lg16s_patch_Rdtl_num_status = 0;

		pOutput->u1_logo_sc_FastDet_status = 0;
		pOutput->u8_logo_sc_FastDet_cntholdfrm = 0;

		pOutput->u1_logo_BG_still_status = 0;
		pOutput->u8_logo_BG_still_hold_frm = 0;
		pOutput->u1_logo_KeyRgn_status = 0;
		pOutput->u8_logo_KeyRgn_hold_frm = 0;
		pOutput->u1_logo_PurePanning_status = 0;
		pOutput->u8_logo_PurePanning_hold_frm = 0;
		pOutput->u1_logo_NearRim_logo_status = 0;
		pOutput->u8_logo_NearRim_logo_hold_frm = 0;
	}

}


#if 0 // rremove from Merlin2

VOID FRC_LogoDet_Init(_OUTPUT_FRC_LGDet *pOutput)
{
	int i;
	pOutput->s16_blklogo_glbclr_cnt = 0;
	//pOutput->s16_pixlogo_glbclr_cnt = 0;

	//pOutput->s16_SC_clrAlpha_cnt = 0;
	//pOutput->s16_SC_clrLogo_cnt  = 0;
	//pOutput->s16_SC_holdHsty_cnt = 0;

	for (i=0; i<LOGO_RG_32; i++)
	{
		*(pOutput->s16_blklogo_rgclr_cnt + i) = 0;
		//*(pOutput->s16_pixlogo_rgclr_cnt + i) = 0;
	}

	//WriteRegister(KME_LOGO1_KME_LOGO1_E0_reg,30,30, 0);
}

VOID FRC_LgDet_RB(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
	unsigned int idx;

	//////////////////////////////////////////////////////////////////////////
	for (idx=0; idx < RG_32/2; idx++)
	{
		ReadRegister(KME_LOGO2_KME_LOGO2_40_reg + idx *4,
			0,
			13, pOutput->u14_pixlogo_rgcnt + idx*2);

		ReadRegister(KME_LOGO2_KME_LOGO2_40_reg + idx *4,
			14,
			27, pOutput->u14_pixlogo_rgcnt + idx*2 + 1);
	}

	for (idx=0; idx < RG_32; idx++)
	{
		ReadRegister(KME_LOGO2_KME_LOGO2_80_reg + idx *4,
			0,
			19, pOutput->u20_pixlogo_rgaccY + idx);
	}

	for (idx=0; idx < RG_32; idx++)
	{
		ReadRegister(KME_LOGO2_KME_LOGO2_80_reg + idx *4,
			20,
			29, pOutput->u10_blklogo_rgcnt + idx);
	}


	//readback gmv
	for (idx=0; idx < RG_32; idx++)
	{
		unsigned int tmp;
		ReadRegister(FRC_TOP__KME_ME_TOP2__regr_metop_gmv_gmvratio00_rdback_ADDR + idx*4,
			0, 7, &tmp);
		*(pOutput->s16_gmv_y_max + idx) = tmp < 128 ? tmp : tmp-256;
		ReadRegister(FRC_TOP__KME_ME_TOP2__regr_metop_gmv_gmvratio00_rdback_ADDR + idx*4,
			8, 16, &tmp);
		*(pOutput->s16_gmv_x_max + idx) = tmp < 256 ? tmp : tmp-512;
		ReadRegister(FRC_TOP__KME_ME_TOP2__regr_metop_gmv_gmvratio00_rdback_ADDR + idx*4,
			24, 31,&tmp);
		*(pOutput->u8_gmv_rat_max + idx) = tmp;


		ReadRegister(FRC_TOP__KME_ME_TOP5__regr_metop_gmv_gmvsec00_rdback_ADDR + idx*4,
			0, 7, &tmp);
		*(pOutput->s16_gmv_y_sec + idx) = tmp < 128 ? tmp : tmp-256;
		ReadRegister(FRC_TOP__KME_ME_TOP5__regr_metop_gmv_gmvsec00_rdback_ADDR + idx*4,
			8, 16, &tmp);
		*(pOutput->s16_gmv_x_sec + idx) = tmp < 256 ? tmp : tmp-512;
		ReadRegister(FRC_TOP__KME_ME_TOP5__regr_metop_gmv_gmvsec00_rdback_ADDR + idx*4,
			24, 31,&tmp);
		*(pOutput->u8_gmv_rat_sec + idx) = tmp;
	}

	for (idx=0; idx < RG_32-2; idx++)
	{
		ReadRegister(FRC_TOP__KME_ME_TOP2__regr_metop_rdbk_sad_region00_ADDR + idx *4,
			FRC_TOP__KME_ME_TOP2__regr_metop_rdbk_sad_region00_BITSTART,
			FRC_TOP__KME_ME_TOP2__regr_metop_rdbk_sad_region00_BITEND, pOutput->u32_region_sad_sum + idx);
	}

	ReadRegister(FRC_TOP__KME_ME_TOP3__regr_metop_rdbk_sad_region36_ADDR + idx *4,
		FRC_TOP__KME_ME_TOP3__regr_metop_rdbk_sad_region36_BITSTART,
		FRC_TOP__KME_ME_TOP3__regr_metop_rdbk_sad_region36_BITEND, pOutput->u32_region_sad_sum + 30);

	ReadRegister(FRC_TOP__KME_ME_TOP3__regr_metop_rdbk_sad_region37_ADDR + idx *4,
		FRC_TOP__KME_ME_TOP3__regr_metop_rdbk_sad_region37_BITSTART,
		FRC_TOP__KME_ME_TOP3__regr_metop_rdbk_sad_region37_BITEND, pOutput->u32_region_sad_sum + 31);

}



VOID FRC_LgDet_DS_RgY(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	unsigned int u32_logo_pxlrgydf_bypass = 0;

	if (pParam->u1_logo_dynY_en == 1)
	{
		int j,i;
		for (j = 0; j < RG_32/4; j++)
		{
			unsigned int data_tmp = 0;
			for (i = 0; i < 4; i++)
			{
				unsigned int pixlogo_rg_Y   = s_pContext->_output_FRC_LgDet.u20_pixlogo_rgaccY[j*4+i];
				unsigned int pixlogo_rg_cnt = s_pContext->_output_FRC_LgDet.u14_pixlogo_rgcnt[j*4+i];

				if (pixlogo_rg_cnt != 0)
				{
					pixlogo_rg_Y = (pixlogo_rg_Y << 2) / pixlogo_rg_cnt;
				}
				else
				{
					pixlogo_rg_Y = 128;
					u32_logo_pxlrgydf_bypass = u32_logo_pxlrgydf_bypass | (1<< (i + j*4));
				}

				data_tmp = data_tmp | (pixlogo_rg_Y << (i*8));
			}
			WriteRegister(KME_LOGO2_KME_LOGO2_00_reg+j*4,0,31, data_tmp);
		}
	}
	else
	{
		u32_logo_pxlrgydf_bypass = 0xFFFFFFFF;
	}

	WriteRegister(KME_LOGO2_KME_LOGO2_24_reg,0,31, u32_logo_pxlrgydf_bypass);
}

VOID FRC_LgDet_DS_Delg(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
	/*	const _PQLCONTEXT *s_pContext = GetPQLContext();*/

	int* gmv_x_max = pOutput->s16_gmv_x_max;
	int* gmv_y_max = pOutput->s16_gmv_y_max;
	unsigned char*  gmv_r_max = pOutput->u8_gmv_rat_max;

	int* gmv_x_sec = pOutput->s16_gmv_x_sec;
	int* gmv_y_sec = pOutput->s16_gmv_y_sec;
	unsigned char*  gmv_r_sec = pOutput->u8_gmv_rat_sec;

	if (pParam->u1_logo_dynDelogo_en == 1)
	{
		//for giraffe's Philips logo, maybe need more detail feature.
		//current use first, sec gmv, and ratio.(ratio max is 128)
		//unsigned int idx;
		//unsigned int mvx[2], mvy[2], rat[2], mvsum[2];

		//for (idx = 0; idx<2; idx++)
		//{
		//	ReadRegister(FRC_TOP__KME_ME_TOP2__regr_metop_gmv_gmvratio00_rdback_ADDR + idx*4,
		//		0, 7, mvy + idx);
		//	ReadRegister(FRC_TOP__KME_ME_TOP2__regr_metop_gmv_gmvratio00_rdback_ADDR + idx*4,
		//		8, 16,mvx + idx);
		//	ReadRegister(FRC_TOP__KME_ME_TOP2__regr_metop_gmv_gmvratio00_rdback_ADDR + idx*4,
		//		24, 31,rat + idx);
		//	//abs
		//	mvx[idx]   = _CLIP_(mvx[idx], 0, 0xFF);
		//	mvy[idx]   = _CLIP_(mvy[idx], 0, 0x7F);
		//	mvsum[idx] = mvx[idx] + mvy[idx];
		//}

		//if (mvsum[0] >= pParam->u16_dynDelogo_mv_th && mvsum[1] >= pParam->u16_dynDelogo_mv_th &&
		//	rat[0] >= pParam->u8_dynDelogo_rat_th && rat[1] >= pParam->u8_dynDelogo_rat_th)
		//{
		//	//need test hor/ver for fast gmv motion, delogo.
		//	WriteRegister(FRC_TOP__DEHALO__reg_dh_logo_mvd_bypass_ADDR no mat,FRC_TOP__DEHALO__reg_dh_logo_mvd_bypass_BITSTART no mat,FRC_TOP__DEHALO__reg_dh_logo_mvd_bypass_BITEND no mat, 7);
		//	WriteRegister(FRC_TOP__DEHALO__reg_dh_logo_tmv_bypass_ADDR no mat,FRC_TOP__DEHALO__reg_dh_logo_tmv_bypass_BITSTART no mat,FRC_TOP__DEHALO__reg_dh_logo_tmv_bypass_BITEND no mat, 1);
		//}
		//else
		//{
		//	WriteRegister(FRC_TOP__DEHALO__reg_dh_logo_mvd_bypass_ADDR no mat,FRC_TOP__DEHALO__reg_dh_logo_mvd_bypass_BITSTART no mat,FRC_TOP__DEHALO__reg_dh_logo_mvd_bypass_BITEND no mat, 0);
		//	WriteRegister(FRC_TOP__DEHALO__reg_dh_logo_tmv_bypass_ADDR no mat,FRC_TOP__DEHALO__reg_dh_logo_tmv_bypass_BITSTART no mat,FRC_TOP__DEHALO__reg_dh_logo_tmv_bypass_BITEND no mat, 0);
		//}

		unsigned int idx;
		unsigned int gg_num = 0;
		for (idx=0; idx < RG_32; idx++)
		{
			unsigned int mv_diff  = abs(gmv_x_max[idx] - gmv_x_sec[idx]) + abs(gmv_y_max[idx] - gmv_y_sec[idx]);
			unsigned int mv_alpha = mv_diff <= pParam->u8_gg_mvd12_thr ? gmv_r_max[idx] + gmv_r_sec[idx] - gmv_r_sec[idx]*mv_diff/ pParam->u8_gg_mvd12_thr : gmv_r_max[idx];
			if ((abs(gmv_x_max[idx]) > pParam->u8_gg_mvx_thr) && (abs(gmv_y_max[idx]) > pParam->u8_gg_mvy_thr) &&
				(mv_alpha > pParam->u8_gg_alpha_thr) && (*(pOutput->u32_region_sad_sum + idx) < pParam->u32_gg_sad_thr))
			{
				gg_num++;
			}
		}

		if (gg_num > pParam->u8_gg_num_thr)
		{
			WriteRegister(FRC_TOP__DEHALO__reg_dh_logo_mvd_bypass_ADDR no mat,FRC_TOP__DEHALO__reg_dh_logo_mvd_bypass_BITSTART no mat,FRC_TOP__DEHALO__reg_dh_logo_mvd_bypass_BITEND no mat, 7);
			WriteRegister(FRC_TOP__DEHALO__reg_dh_logo_tmv_bypass_ADDR no mat,FRC_TOP__DEHALO__reg_dh_logo_tmv_bypass_BITSTART no mat,FRC_TOP__DEHALO__reg_dh_logo_tmv_bypass_BITEND no mat, 1);
		}
		else
		{
			WriteRegister(FRC_TOP__DEHALO__reg_dh_logo_mvd_bypass_ADDR no mat,FRC_TOP__DEHALO__reg_dh_logo_mvd_bypass_BITSTART no mat,FRC_TOP__DEHALO__reg_dh_logo_mvd_bypass_BITEND no mat, pParam->u8_reg_mvd);
			WriteRegister(FRC_TOP__DEHALO__reg_dh_logo_tmv_bypass_ADDR no mat,FRC_TOP__DEHALO__reg_dh_logo_tmv_bypass_BITSTART no mat,FRC_TOP__DEHALO__reg_dh_logo_tmv_bypass_BITEND no mat, pParam->u8_reg_tmv);
		}
	}
	else
	{
		//need set to default setting.
		WriteRegister(FRC_TOP__DEHALO__reg_dh_logo_mvd_bypass_ADDR no mat,FRC_TOP__DEHALO__reg_dh_logo_mvd_bypass_BITSTART no mat,FRC_TOP__DEHALO__reg_dh_logo_mvd_bypass_BITEND no mat, pParam->u8_reg_mvd);
		WriteRegister(FRC_TOP__DEHALO__reg_dh_logo_tmv_bypass_ADDR no mat,FRC_TOP__DEHALO__reg_dh_logo_tmv_bypass_BITSTART no mat,FRC_TOP__DEHALO__reg_dh_logo_tmv_bypass_BITEND no mat, pParam->u8_reg_tmv);
	}
}


VOID FRC_LgDet_DS_ThrStep(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
	/*	const _PQLCONTEXT *s_pContext = GetPQLContext();*/

	if (pParam->u1_logo_dynBlkTh_en == 1 &&  pOutput->u32_frm_cnt >= pParam->u8_dynBlkTh_cnt)
	{
		WriteRegister(KME_LOGO0_KME_LOGO0_84_reg,25,25, 1);
	}
	else
	{
		WriteRegister(KME_LOGO0_KME_LOGO0_84_reg,25,25, 0);
	}

	if (pParam->u1_logo_dynPixTh_en == 1 &&  pOutput->u32_frm_cnt >= pParam->u8_dynPixTh_cnt)
	{
		WriteRegister(KME_LOGO1_KME_LOGO1_94_reg,8,8,   1);
	}
	else
	{
		WriteRegister(KME_LOGO1_KME_LOGO1_94_reg,8,8,   0);
	}


	if (pParam->u1_logo_dynBlkStep_en == 1 &&  pOutput->u32_frm_cnt >= pParam->u8_dynBlkStep_cnt)
	{
		WriteRegister(KME_LOGO0_KME_LOGO0_EC_reg,8,8, 1);
		WriteRegister(KME_LOGO0_KME_LOGO0_EC_reg,9,9, pParam->u1_blkHsty_upmet);
	}
	else
	{
		WriteRegister(KME_LOGO0_KME_LOGO0_EC_reg,8,8, 0);
		WriteRegister(KME_LOGO0_KME_LOGO0_EC_reg,9,9, 0);
	}

	if (pParam->u1_logo_dynPixStep_en == 1 &&  pOutput->u32_frm_cnt >= pParam->u8_dynPixStep_cnt)
	{
		WriteRegister(KME_LOGO1_KME_LOGO1_98_reg,10,10, 1);
	}
	else
	{
		WriteRegister(KME_LOGO1_KME_LOGO1_98_reg,10,10, 0);
	}
}



VOID FRC_LgDet_DynSet(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
	//#1: Dyn regional Y set.
	FRC_LgDet_DS_RgY(pParam, pOutput);

	//#2: Dyn. use Adp. thr, step.
	FRC_LgDet_DS_ThrStep(pParam, pOutput);

	//#3. Fast,Good GMV and abandon dehalo de-logo (mvd,zmv)
	FRC_LgDet_DS_Delg(pParam, pOutput);

}

VOID FRC_LgDet_PixClrCtrl(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	unsigned int idx;
	unsigned int acc_pix_num;
	int*  hold_cnt_rg  = pOutput->s16_pixlogo_rgclr_cnt;

	acc_pix_num = 0;
	for (idx = 0; idx < RG_32; idx++)
	{
		unsigned int pixlogo_rg_cnt = s_pContext->_output_FRC_LgDet.u14_pixlogo_rgcnt[idx];
		if (((pixlogo_rg_cnt * 256 / PIX_LOGO_RG_PIXCNT) >= pParam->u8_pixRgClr_thr) && pParam->u1_logo_pixRgClr_en == 1)
		{
			pOutput->u32_pixlogo_clr_idx = pOutput->u32_pixlogo_clr_idx | (1 << idx);
			hold_cnt_rg[idx] = pParam->u6_rgClr_holdtime;
		}
		else if ((hold_cnt_rg[idx] > 1) && pParam->u1_logo_pixRgClr_en == 1)
		{
			pOutput->u32_pixlogo_clr_idx = pOutput->u32_pixlogo_clr_idx | (1 << idx);
			hold_cnt_rg[idx] = _MAX_(hold_cnt_rg[idx] - 1,0);
#if LOGO_DEBUG_PRINTF
			if (idx == 15 && pParam->u1_printf_en==1)  LogPrintf(DBG_MSG,"pix logo region(idx=15) clear cnt: %d  \r\n",hold_cnt_rg[idx]);
#endif
		}

		acc_pix_num += pixlogo_rg_cnt;
	}

	if ((((acc_pix_num * 256) / (PIX_LOGO_PIXCNT)) >= pParam->u8_pixGlbClr_thr) &&  pParam->u1_logo_pixGlbClr_en == 1)
	{
		pOutput->u32_pixlogo_clr_idx    = pOutput->u32_pixlogo_clr_idx | 0xFFFFFFFF;
		pOutput->s16_pixlogo_glbclr_cnt = pParam->u6_GlbClr_holdtime;
		pOutput->u1_pixclr_glbstatus    = 1;
	}
	else if (pOutput->s16_pixlogo_glbclr_cnt > 1 &&  pParam->u1_logo_pixGlbClr_en == 1)
	{
		pOutput->u32_pixlogo_clr_idx    = pOutput->u32_pixlogo_clr_idx | 0xFFFFFFFF;
		pOutput->s16_pixlogo_glbclr_cnt = _MAX_(pOutput->s16_pixlogo_glbclr_cnt - 1,0);
		pOutput->u1_pixclr_glbstatus    = 1;
	}

#if LOGO_DEBUG_PRINTF
	if ((pOutput->u32_frm_cnt%500) == 0 && pParam->u1_printf_en==1)  LogPrintf(DBG_MSG,"pix logo clear: %d  \r\n",pOutput->u1_pixclr_glbstatus);
	if ((pOutput->u32_frm_cnt%500) == 0 && pParam->u1_printf_en==1)  LogPrintf(DBG_MSG,"accum pix logo: %d  \r\n",acc_pix_num);
#endif

}

//for clear logo (glb, rg), just set the clr status.
VOID FRC_LgDet_BlkClrCtrl(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	unsigned int idx;
	unsigned int acc_blk_num;
	unsigned int acc_left_blk_num = 0;
	unsigned int acc_right_blk_num = 0;
	unsigned int acc_top_blk_num = 0;
	unsigned int acc_bottom_blk_num = 0;
	int*  hold_cnt_rg  = pOutput->s16_blklogo_rgclr_cnt;

	acc_blk_num = 0;
	for (idx = 0; idx < RG_32; idx++)
	{
		unsigned int blklogo_rg_cnt = s_pContext->_output_FRC_LgDet.u10_blklogo_rgcnt[idx];
		if ((((blklogo_rg_cnt * 256) / (BLK_LOGO_RG_BLKCNT)) >= pParam->u8_clr_rg_thr) && pParam->u1_logo_rg_clr_en == 1)
		{
			pOutput->u32_blklogo_clr_idx = pOutput->u32_blklogo_clr_idx | (1 << idx);
			hold_cnt_rg[idx] = pParam->u6_rgClr_holdtime;
		}
		else if ((hold_cnt_rg[idx] > 1)  && pParam->u1_logo_rg_clr_en == 1)
		{
			pOutput->u32_blklogo_clr_idx = pOutput->u32_blklogo_clr_idx | (1 << idx);
			hold_cnt_rg[idx] = _MAX_(hold_cnt_rg[idx] - 1,0);
#if LOGO_DEBUG_PRINTF
			if (idx == 15 && pParam->u1_printf_en==1)  LogPrintf(DBG_MSG,"blk logo region(idx=15) clear cnt: %d  \r\n",hold_cnt_rg[idx]);
#endif
		}

		//CMO
		if ((((blklogo_rg_cnt * 256) / (BLK_LOGO_RG_BLKCNT)) >= pParam->u8_cmo_clear_thr) && pParam->u1_logo_rg_clr_en == 1
			&& pParam->u1_logo_dynAlpha_en == 1 && (((pParam->u32_cmo_clear_idx >> idx) & 0x1) == 1))
		{
			pOutput->u32_blklogo_clr_idx = pOutput->u32_blklogo_clr_idx | (1 << idx);
#if LOGO_DEBUG_PRINTF
			if (pParam->u1_printf_en==1)	 LogPrintf(DBG_MSG,"CMO logo: %d region clear, cnt: %d  \r\n",idx,blklogo_rg_cnt);
#endif
		}

		acc_blk_num += blklogo_rg_cnt;

		if (idx < 16)
		{
			acc_top_blk_num += blklogo_rg_cnt;
		}
		else
		{
			acc_bottom_blk_num += blklogo_rg_cnt;
		}
		if (idx%8 <4)
		{
			acc_left_blk_num += blklogo_rg_cnt;
		}
		else
		{
			acc_right_blk_num += blklogo_rg_cnt;
		}

	}


	if (((((acc_blk_num * 256) / (BLK_LOGO_BLKCNT)) >= pParam->u8_clr_glb_thr) && pParam->u1_logo_glb_clr_en == 1)
		|| ((((acc_left_blk_num * 256) / (BLK_LOGO_BLKCNT/2)) >= pParam->u8_clr_half_thr) && ((pParam->u4_logo_half_clr_en & 0x1) == 1))
		|| ((((acc_right_blk_num * 256) / (BLK_LOGO_BLKCNT/2)) >= pParam->u8_clr_half_thr) && (((pParam->u4_logo_half_clr_en>>1) & 0x1) == 1))
		|| ((((acc_top_blk_num * 256) / (BLK_LOGO_BLKCNT/2)) >= pParam->u8_clr_half_thr) && (((pParam->u4_logo_half_clr_en>>2) & 0x1) == 1))
		|| ((((acc_bottom_blk_num * 256) / (BLK_LOGO_BLKCNT/2)) >= pParam->u8_clr_half_thr) && (((pParam->u4_logo_half_clr_en>>3) & 0x1) == 1)))
	{
		pOutput->u32_blklogo_clr_idx    = pOutput->u32_blklogo_clr_idx | 0xFFFFFFFF;
		pOutput->s16_blklogo_glbclr_cnt = pParam->u6_GlbClr_holdtime;
		pOutput->u1_blkclr_glbstatus = 1;
	}
	else if (pOutput->s16_blklogo_glbclr_cnt > 1 &&  pParam->u1_logo_glb_clr_en == 1)
	{

		pOutput->u32_blklogo_clr_idx    = pOutput->u32_blklogo_clr_idx | 0xFFFFFFFF;
		pOutput->s16_blklogo_glbclr_cnt = _MAX_(pOutput->s16_blklogo_glbclr_cnt - 1,0);
		pOutput->u1_blkclr_glbstatus = 1;
	}

	//for disp.
	//if (pOutput->u1_blkclr_glbstatus == 1 || s_pContext->_output_read_comreg.u1_sc_status_rb == 1)
	//{
	//	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg,0,15, 1);
	//}
	//else
	//{
	//	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg,0,15, 0);
	//}

#if LOGO_DEBUG_PRINTF
	if ((pOutput->u32_frm_cnt%500) == 0 && pParam->u1_printf_en==1)  LogPrintf(DBG_MSG,"blk logo clear: %d  \r\n",pOutput->u1_blkclr_glbstatus);
	if ((pOutput->u32_frm_cnt%500) == 0 && pParam->u1_printf_en==1)  LogPrintf(DBG_MSG,"accum blk logo: %d  \r\n",acc_blk_num);
#endif
}

//current, use blk_logo idx to clear SC logo (global all)
//default is logoClr=1, holdHsty=1, clrAlpha=1
VOID FRC_LgDet_SCCtrl(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	pOutput->u5_alpha_bld           = pParam->u5_reg_iir_alpha;
	pOutput->u4_blklogo_pStep_cur   = pParam->u4_blklogo_pStep;
	pOutput->u4_pixlogo_pStep_cur   = pParam->u4_pixlogo_pStep;

	if (s_pContext->_output_read_comreg.u1_sc_status_rb == 1)
	{
		pOutput->u32_frm_cnt  = 0;  //for auto En
		if (pParam->u1_logo_SCalphaClr_en == 1)
		{
			pOutput->u5_alpha_bld = 0;
			pOutput->s16_SC_clrAlpha_cnt = pParam->u6_SCalpha_holdtime;
#if LOGO_DEBUG_PRINTF
			if (pParam->u1_printf_en==1)  LogPrintf(DBG_MSG,"alpha %d \r\n",pOutput->u5_alpha_bld);
#endif
		}
		if (pParam->u1_logo_SCLogoClr_en == 1)
		{
			//pOutput->u32_pixlogo_clr_idx = pOutput->u32_pixlogo_clr_idx | 0xFFFFFFFF;
			//pOutput->u1_pixclr_glbstatus = 1;

			pOutput->u32_blklogo_clr_idx = pOutput->u32_blklogo_clr_idx | 0xFFFFFFFF;
			pOutput->u1_blkclr_glbstatus = 1;

			pOutput->s16_SC_clrLogo_cnt  = pParam->u6_SCclrlogo_holdtime;
#if LOGO_DEBUG_PRINTF
			if (pParam->u1_printf_en==1)  LogPrintf(DBG_MSG,"SC clr pix logo \r\n");
#endif
		}
		if (pParam->u1_logo_SCHstyClr_en == 1)
		{
			pOutput->u4_blklogo_pStep_cur = 0;
			pOutput->u4_pixlogo_pStep_cur = 0;
			pOutput->s16_SC_holdHsty_cnt  = pParam->u6_SCclrHsty_holdtime;
#if LOGO_DEBUG_PRINTF
			if (pParam->u1_printf_en==1)	LogPrintf(DBG_MSG,"SC hold hsty \r\n");
#endif
		}
	}
	else
	{
		pOutput->u32_frm_cnt = _CLIP_(pOutput->u32_frm_cnt = pOutput->u32_frm_cnt + 1, 0, 0xFFFFFFF);
		if (pParam->u1_logo_SCalphaClr_en == 1 && pOutput->s16_SC_clrAlpha_cnt > 1)
		{
			pOutput->u5_alpha_bld = 0;
			pOutput->s16_SC_clrAlpha_cnt = _MAX_(pOutput->s16_SC_clrAlpha_cnt-1,0);
#if LOGO_DEBUG_PRINTF
			if (pParam->u1_printf_en==1)  LogPrintf(DBG_MSG,"alpha %d \r\n",pOutput->u5_alpha_bld);
#endif
		}
		if (pParam->u1_logo_SCLogoClr_en == 1 && pOutput->s16_SC_clrLogo_cnt > 1)
		{
			//pOutput->u32_pixlogo_clr_idx = pOutput->u32_pixlogo_clr_idx | 0xFFFFFFFF;
			//pOutput->u1_pixclr_glbstatus = 1;

			pOutput->u32_blklogo_clr_idx = pOutput->u32_blklogo_clr_idx | 0xFFFFFFFF;
			pOutput->u1_blkclr_glbstatus = 1;

			pOutput->s16_SC_clrLogo_cnt  = _MAX_(pOutput->s16_SC_clrLogo_cnt-1,0);
#if LOGO_DEBUG_PRINTF
			if (pParam->u1_printf_en==1)   LogPrintf(DBG_MSG,"SC clr pix logo, hold cnt:%d \r\n", pOutput->s16_SC_clrLogo_cnt);
#endif
		}
		if (pParam->u1_logo_SCLogoClr_en == 1 && pOutput->s16_SC_clrLogo_cnt > 1)
		{
			pOutput->u4_blklogo_pStep_cur = 0;
			pOutput->u4_pixlogo_pStep_cur = 0;
			pOutput->s16_SC_holdHsty_cnt  = _MAX_(pOutput->s16_SC_holdHsty_cnt-1,0);
#if LOGO_DEBUG_PRINTF
			if (pParam->u1_printf_en==1)  LogPrintf(DBG_MSG,"SC hold hsty, hold cnt:%d \r\n", pOutput->s16_SC_holdHsty_cnt);
#endif
		}
	}

	WriteRegister(KME_LOGO0_KME_LOGO0_10_reg,0,4,     pOutput->u5_alpha_bld);
	WriteRegister(KME_LOGO0_KME_LOGO0_EC_reg,4,7, pOutput->u4_blklogo_pStep_cur);
	WriteRegister(KME_LOGO1_KME_LOGO1_BC_reg,0,3, pOutput->u4_pixlogo_pStep_cur);
}

VOID FRC_LgDet_ClrAction(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{
	if(pParam->u4_reg_merge_type > 0 && pOutput->u32_blklogo_clr_idx > 0)
	{
		WriteRegister(MC2_MC2_20_reg,6,9, 0);
		WriteRegister(KME_LOGO1_KME_LOGO1_E4_reg,0,31, pOutput->u32_blklogo_clr_idx);
	}
	else
	{
		WriteRegister(MC2_MC2_20_reg,6,9, pParam->u4_reg_merge_type);
		WriteRegister(KME_LOGO1_KME_LOGO1_E4_reg,0,31, pOutput->u32_blklogo_clr_idx);
	}

	if ((pParam->u1_logo_blkRgClrHsty_en == 1) || (pParam->u1_logo_blkGlbClrHsty_en == 1 && pOutput->u1_blkclr_glbstatus == 1))
	{
		WriteRegister(KME_LOGO1_KME_LOGO1_EC_reg,0,31, pOutput->u32_blklogo_clr_idx);
	}
	else
	{
		WriteRegister(KME_LOGO1_KME_LOGO1_EC_reg,0,31, 0);
	}
}



VOID FRC_LogoDet_Proc(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput)
{

	if (pParam->u1_logo_pql_en == 1)
	{

		FRC_LgDet_BlkClrCtrl(pParam, pOutput);
	}


	if (pParam->u1_logo_pql_en == 1)
	{
		const _PQLCONTEXT *s_pContext = GetPQLContext();
		unsigned char  u1_gmv_logoClr = s_pContext->_output_me_sceneAnalysis.u1_logoClr_gmv_true;

#if LOGO_DEBUG_PRINTF
		if ((pOutput->u32_frm_cnt%500) == 5 && pParam->u1_printf_en==1)   LogPrintf(DBG_MSG,"Logo PQL Enable \r\n");
#endif

		pOutput->u32_blklogo_clr_idx = 0;
		pOutput->u32_pixlogo_clr_idx = 0;
		pOutput->u1_blkclr_glbstatus = 0;
		pOutput->u1_pixclr_glbstatus = 0;

		//1st read-back
		FRC_LgDet_RB(pParam, pOutput);

		FRC_LgDet_SCCtrl(pParam, pOutput);

		if(u1_gmv_logoClr == 1)
		{
			pOutput->u32_blklogo_clr_idx = pOutput->u32_blklogo_clr_idx | 0xFFFFFFFF;
			pOutput->u1_blkclr_glbstatus = 1;
		}

		FRC_LgDet_BlkClrCtrl(pParam, pOutput);
		//FRC_LgDet_PixClrCtrl(pParam, pOutput);

		//use blk statics, use pixclr reg to clear.
		FRC_LgDet_ClrAction(pParam, pOutput);

		//local dynamic set, if clear/sc = manual setting.
		FRC_LgDet_DynSet(pParam, pOutput);
	}

}
#endif
