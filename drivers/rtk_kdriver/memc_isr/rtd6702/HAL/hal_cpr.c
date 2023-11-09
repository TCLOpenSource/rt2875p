/**
 * @file hal_cpr.c
 * @brief This file is for cpr register setting
 * @date Nov.18.2014
 * @version V0.0.1
 * @author Martin Yan
 * @par Copyright (C) 2014 Real-Image Co. LTD
 * @par History:
 *			Version: 0.0.1
*/
#ifdef CONFIG_ARM64 //ARM32 compatible
	// for register dump
#include <tvscalercontrol/io/ioregdrv.h>
#undef rtd_outl
#define rtd_outl(x, y)     								IoReg_Write32(x,y)
#undef rtd_inl
#define rtd_inl(x)     									IoReg_Read32(x)
#undef rtd_maskl
#define rtd_maskl(x, y, z)     							IoReg_Mask32(x,y,z)
#undef rtd_setbits
#define rtd_setbits(offset, Mask) rtd_outl(offset, (rtd_inl(offset) | Mask))
#undef rtd_clearbits
#define rtd_clearbits(offset, Mask) rtd_outl(offset, ((rtd_inl(offset) & ~(Mask))))
#endif

#include "memc_isr/Common/kw_debug.h"
#include "memc_isr/Common/kw_common.h"
#include "memc_isr/Driver/regio.h"
#include "memc_isr/PQL/PQLPlatformDefs.h"

extern unsigned char MEMC_Lib_get_INPUT_COLOR_SPACE_Mode(VOID);
#define msleep(x)							((void)0)
#include "memc_reg_def.h"


/**
 * @brief This function set cpr h/v active
 * @param [in] unsigned int u32Hactive
 * @param [in] unsigned int u32Vactive
 * @retval VOID
*/
VOID HAL_CPR_SetActive(unsigned int u32Hactive, unsigned int u32Vactive)
{
	//reg_hact
	//WriteRegister(PQC_PQDC_MC_LFH_PQ_CMP_PAIR_reg,0,11,u32Hactive);	// 2 port
	//reg_vact
	//WriteRegister(PQC_PQDC_MC_LFH_PQ_CMP_PAIR_reg,16,27,u32Vactive);	// hf/lf row count
}

/**
 * @brief This function set encode 3d enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_CPR_SetEncode3DEnable(BOOL bEnable)
{
	//reg_enc_3d_mode
	//WriteRegister(PQC_PQDC_MC_PQ_CMP_reg,16,16,(unsigned int)bEnable); 
}

/**
 * @brief This function set encode 3d mode
 * @param [in] unsigned int u32Mode
 * @retval VOID
*/
VOID HAL_CPR_SetEncode3DMode(unsigned int u32Mode)
{
	//reg_enc_3d_format
 	//WriteRegister(PQC_PQDC_MC_PQ_CMP_reg,17,18,u32Mode);
}

/**
 * @brief This function set decode 3d enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_CPR_SetDecode3DEnable(BOOL bEnable)
{
	//reg_dec_3d_mode
	//WriteRegister(PQC_PQDC_MC_PQ_CMP_reg,19,19,(unsigned int)bEnable); 
}

/**
 * @brief This function set decode 3d mode
 * @param [in] unsigned int u32Mode
 * @retval VOID
*/
VOID HAL_CPR_SetDecode3DMode(unsigned int u32Mode)
{
	//dec_3d_format
 	//WriteRegister(PQC_PQDC_MC_PQ_CMP_reg,20,20,u32Mode);
}

/**
 * @brief This function set 422 enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_CPR_Set422Enable(BOOL bEnable)
{
	//reg_422_mode
	//WriteRegister(PQC_PQDC_MC_PQ_CMP_reg,1,1,(unsigned int)bEnable); 
}

/**
 * @brief This function set 444 enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
extern int RHAL_TP_IsJapan4K(void);
extern int RHAL_TP_IsATSC30(void);
extern unsigned char MEMC_Lib_Get_JP_DTV_4K_flag(void);
extern VOID MEMC_LibSet_PQC_DB_apply(void);
extern unsigned int GS_MC_8_buffer;
extern unsigned int MEMC_Lib_get_DisplayRefreshRate(void);

VOID HAL_CPR_Set444Enable(BOOL bEnable)//modify from merlin4, using RTK pqc
{
	#if 1 // it's follow tv006
	//pqc_pqdc_mc_lf_pq_cmp_bit_RBUS	pqc_pqdc_mc_lf_pq_cmp_bit_reg;
	pqc_pqdc_mc_lfh_pq_cmp_balance_RBUS pqc_pqdc_mc_lf_pq_cmp_balance_reg;
	pqc_pqdc_mc_hf_pq_cmp_balance_RBUS pqc_pqdc_mc_hf_pq_cmp_balance_reg;
	pqc_pqdc_mc_hf_pq_cmp_poor_RBUS pqc_pqdc_mc_hf_pq_cmp_poor_reg;
	pqc_pqdc_mc_lfh_pq_cmp_pair_RBUS pqc_pqdc_mc_lf_pq_cmp_pair_reg;
	pqc_pqdc_mc_hf_pq_cmp_pair_RBUS pqc_pqdc_mc_hf_pq_cmp_pair_reg;
	pqc_pqdc_mc_lfh_pq_decmp_pair_RBUS pqc_pqdc_mc_lf_pq_decmp_pair_reg;
	pqc_pqdc_mc_hf_pq_decmp_pair_RBUS pqc_pqdc_mc_hf_pq_decmp_pair_reg;
	pqc_pqdc_mc_lfh_pq_cmp_enable_RBUS pqc_pqdc_mc_lf_pq_cmp_enable_reg;
	pqc_pqdc_mc_hf_pq_cmp_enable_RBUS pqc_pqdc_mc_hf_pq_cmp_enable_reg;
	pqc_pqdc_mc_lfh_pq_decmp_sat_en_RBUS pqc_pqdc_mc_lf_pq_decmp_sat_en_reg;
	pqc_pqdc_mc_hf_pq_decmp_sat_en_RBUS pqc_pqdc_mc_hf_pq_decmp_sat_en_reg;
	kme_pqc_pqdc_kme_pq_cmp_pair_RBUS kme_pqc_pqdc_kme_pq_cmp_pair_reg;
	kme_pqc_pqdc_kme_pq_decmp_pair_RBUS kme_pqc_pqdc_kme_pq_decmp_pair_reg;
	kme_pqc_pqdc_kme_pq_cmp_enable_RBUS kme_pqc_pqdc_kme_pq_cmp_enable_reg;
	kme_pqc_pqdc_kme_pq_decmp_sat_en_RBUS kme_pqc_pqdc_kme_pq_decmp_sat_en_reg;
	
	pqc_pqdc_mc_lf_pq_cmp_balance_reg.regValue = rtd_inl(PQC_PQDC_MC_LFH_PQ_CMP_BALANCE_reg);
	pqc_pqdc_mc_lf_pq_cmp_balance_reg.balance_g_give =3;
	pqc_pqdc_mc_lf_pq_cmp_balance_reg.balance_g_ov_th =0; // 3;
	pqc_pqdc_mc_lf_pq_cmp_balance_reg.balance_g_ud_th=3; // 0;
	pqc_pqdc_mc_lf_pq_cmp_balance_reg.balance_rb_ov_th =0; // 3;
	pqc_pqdc_mc_lf_pq_cmp_balance_reg.balance_rb_ud_th =3; // 0;
	pqc_pqdc_mc_lf_pq_cmp_balance_reg.balance_rb_give =3;
	rtd_outl(PQC_PQDC_MC_LFH_PQ_CMP_BALANCE_reg,pqc_pqdc_mc_lf_pq_cmp_balance_reg.regValue);

	pqc_pqdc_mc_hf_pq_cmp_balance_reg.regValue = rtd_inl(PQC_PQDC_MC_HF_PQ_CMP_BALANCE_reg);
	pqc_pqdc_mc_hf_pq_cmp_balance_reg.balance_g_give =3;
	pqc_pqdc_mc_hf_pq_cmp_balance_reg.balance_g_ov_th =0; // 3;
	pqc_pqdc_mc_hf_pq_cmp_balance_reg.balance_g_ud_th=3; // 0;
  	pqc_pqdc_mc_hf_pq_cmp_balance_reg.balance_rb_ov_th =0; // 3;
	pqc_pqdc_mc_hf_pq_cmp_balance_reg.balance_rb_ud_th =3; // 0;
	pqc_pqdc_mc_hf_pq_cmp_balance_reg.balance_rb_give =3;
	rtd_outl(PQC_PQDC_MC_HF_PQ_CMP_BALANCE_reg,pqc_pqdc_mc_hf_pq_cmp_balance_reg.regValue);

	pqc_pqdc_mc_lf_pq_cmp_pair_reg.regValue = rtd_inl(PQC_PQDC_MC_LFH_PQ_CMP_PAIR_reg);
	pqc_pqdc_mc_lf_pq_cmp_pair_reg.cmp_pair_para = 1;
	rtd_outl(PQC_PQDC_MC_LFH_PQ_CMP_PAIR_reg,pqc_pqdc_mc_lf_pq_cmp_pair_reg.regValue);

	pqc_pqdc_mc_hf_pq_cmp_pair_reg.regValue = rtd_inl(PQC_PQDC_MC_HF_PQ_CMP_PAIR_reg);
	pqc_pqdc_mc_hf_pq_cmp_pair_reg.cmp_pair_para = 1;
	rtd_outl(PQC_PQDC_MC_HF_PQ_CMP_PAIR_reg,pqc_pqdc_mc_hf_pq_cmp_pair_reg.regValue);

	pqc_pqdc_mc_lf_pq_decmp_pair_reg.regValue = rtd_inl(PQC_PQDC_MC_LFH_PQ_DECMP_PAIR_reg);
	pqc_pqdc_mc_lf_pq_decmp_pair_reg.decmp_pair_para = 1;
	rtd_outl(PQC_PQDC_MC_LFH_PQ_DECMP_PAIR_reg,pqc_pqdc_mc_lf_pq_decmp_pair_reg.regValue);

	pqc_pqdc_mc_hf_pq_decmp_pair_reg.regValue = rtd_inl(PQC_PQDC_MC_HF_PQ_DECMP_PAIR_reg);
	pqc_pqdc_mc_hf_pq_decmp_pair_reg.decmp_pair_para = 1;
	rtd_outl(PQC_PQDC_MC_HF_PQ_DECMP_PAIR_reg,pqc_pqdc_mc_hf_pq_decmp_pair_reg.regValue);

	pqc_pqdc_mc_lf_pq_cmp_enable_reg.regValue = rtd_inl(PQC_PQDC_MC_LFH_PQ_CMP_ENABLE_reg);
	pqc_pqdc_mc_lf_pq_cmp_enable_reg.cmp_ctrl_para0 = 5;
	pqc_pqdc_mc_lf_pq_cmp_enable_reg.variation_maxmin_en = 0;
	rtd_outl(PQC_PQDC_MC_LFH_PQ_CMP_ENABLE_reg,pqc_pqdc_mc_lf_pq_cmp_enable_reg.regValue);

	pqc_pqdc_mc_hf_pq_cmp_enable_reg.regValue = rtd_inl(PQC_PQDC_MC_HF_PQ_CMP_ENABLE_reg);
	pqc_pqdc_mc_hf_pq_cmp_enable_reg.cmp_ctrl_para0 = 5;
	pqc_pqdc_mc_hf_pq_cmp_enable_reg.variation_maxmin_en = 0;
	rtd_outl(PQC_PQDC_MC_HF_PQ_CMP_ENABLE_reg,pqc_pqdc_mc_hf_pq_cmp_enable_reg.regValue);

	pqc_pqdc_mc_lf_pq_decmp_sat_en_reg.regValue = rtd_inl(PQC_PQDC_MC_LFH_PQ_DECMP_SAT_EN_reg);
	pqc_pqdc_mc_lf_pq_decmp_sat_en_reg.decmp_ctrl_para = 1;
#if IC_K7LP	
	pqc_pqdc_mc_lf_pq_decmp_sat_en_reg.saturation_en = 1;
	pqc_pqdc_mc_lf_pq_decmp_sat_en_reg.saturation_type = 0;
	pqc_pqdc_mc_lf_pq_decmp_sat_en_reg.saturation_type_delta = 0;
#endif	
	rtd_outl(PQC_PQDC_MC_LFH_PQ_DECMP_SAT_EN_reg,pqc_pqdc_mc_lf_pq_decmp_sat_en_reg.regValue);

	pqc_pqdc_mc_hf_pq_decmp_sat_en_reg.regValue = rtd_inl(PQC_PQDC_MC_HF_PQ_DECMP_SAT_EN_reg);
	pqc_pqdc_mc_hf_pq_decmp_sat_en_reg.decmp_ctrl_para = 1;
#if IC_K7LP	
	pqc_pqdc_mc_hf_pq_decmp_sat_en_reg.saturation_en =1;
	pqc_pqdc_mc_hf_pq_decmp_sat_en_reg.saturation_type = 0;
	pqc_pqdc_mc_hf_pq_decmp_sat_en_reg.saturation_type_delta =1;
#endif	
	rtd_outl(PQC_PQDC_MC_HF_PQ_DECMP_SAT_EN_reg,pqc_pqdc_mc_hf_pq_decmp_sat_en_reg.regValue);

	kme_pqc_pqdc_kme_pq_cmp_pair_reg.regValue = rtd_inl(KME_PQC_PQDC_KME_PQ_CMP_PAIR_reg);
	kme_pqc_pqdc_kme_pq_cmp_pair_reg.cmp_pair_para = 1;
	rtd_outl(KME_PQC_PQDC_KME_PQ_CMP_PAIR_reg,kme_pqc_pqdc_kme_pq_cmp_pair_reg.regValue);

	kme_pqc_pqdc_kme_pq_decmp_pair_reg.regValue = rtd_inl(KME_PQC_PQDC_KME_PQ_CMP_PAIR_reg);
	kme_pqc_pqdc_kme_pq_decmp_pair_reg.decmp_pair_para = 1;
	rtd_outl(KME_PQC_PQDC_KME_PQ_CMP_PAIR_reg,kme_pqc_pqdc_kme_pq_decmp_pair_reg.regValue);

	kme_pqc_pqdc_kme_pq_cmp_enable_reg.regValue = rtd_inl(KME_PQC_PQDC_KME_PQ_CMP_ENABLE_reg);
	kme_pqc_pqdc_kme_pq_cmp_enable_reg.cmp_ctrl_para0 = 5;
	rtd_outl(KME_PQC_PQDC_KME_PQ_CMP_ENABLE_reg,kme_pqc_pqdc_kme_pq_cmp_enable_reg.regValue);

	kme_pqc_pqdc_kme_pq_decmp_sat_en_reg.regValue = rtd_inl(KME_PQC_PQDC_KME_PQ_DECMP_SAT_EN_reg);
	kme_pqc_pqdc_kme_pq_decmp_sat_en_reg.decmp_ctrl_para = 1;
	rtd_outl(KME_PQC_PQDC_KME_PQ_DECMP_SAT_EN_reg,kme_pqc_pqdc_kme_pq_decmp_sat_en_reg.regValue);
	#endif
	
	if(bEnable){
		//rtd_maskl(PQC_PQDC_MC_LFH_PQ_CMP_PAIR_reg,0xfffffffc,0x00000000); //444(bit:0)
		//rtd_maskl(PQC_PQDC_MC_HF_PQ_CMP_PAIR_reg,0xfffffffc,0x00000000); //444(bit:0)
		//rtd_maskl(PQC_PQDC_MC_LFH_PQ_DECMP_PAIR_reg,0xfffffffc,0x00000000); //444(bit:0)
		//rtd_maskl(PQC_PQDC_MC_HF_PQ_DECMP_PAIR_reg,0xfffffffc,0x00000000); //444(bit:0)

		if(MEMC_Lib_get_INPUT_COLOR_SPACE_Mode()== 1){ //I3DDMA_COLOR_SPACE_T
			rtd_clearbits(PQC_PQDC_MC_LFH_PQ_CMP_PAIR_reg, 0x11); //cmp_data_color(bit:0 & 4)
			rtd_clearbits(PQC_PQDC_MC_HF_PQ_CMP_PAIR_reg, 0x13); //cmp_data_color(bit:0 & 4)
			rtd_clearbits(PQC_PQDC_MC_LFH_PQ_DECMP_PAIR_reg, 0x11); //cmp_data_color(bit:0 & 4)
			rtd_clearbits(PQC_PQDC_MC_HF_PQ_DECMP_PAIR_reg, 0x13); //cmp_data_color(bit:0 & 4)

			rtd_maskl(PQC_PQDC_MC_LFH_PQ_CMP_ENABLE_reg,0xffffff7f,0x00000000); //first_line(bit:7)
			rtd_maskl(PQC_PQDC_MC_HF_PQ_CMP_ENABLE_reg,0xffffff7f,0x00000000); //first_line(bit:7)

			rtd_maskl(PQC_PQDC_MC_LFH_PQ_CMP_ENABLE_reg,0xffffffcf,0x00000010); //444(bit:4~5)
			rtd_maskl(PQC_PQDC_MC_HF_PQ_CMP_ENABLE_reg,0xffffffcf,0x00000010); //444(bit:4~5)

			rtd_maskl(PQC_PQDC_MC_LFH_PQ_CMP_ENABLE_reg,0xffe0ffff,0x000e0000); //gratio(bit:16~20)
			rtd_maskl(PQC_PQDC_MC_HF_PQ_CMP_ENABLE_reg,0xffe0ffff,0x000e0000); //gratio(bit:16~20)

			rtd_maskl(PQC_PQDC_MC_LFH_PQ_CMP_ALLOCATE_reg,0x0000ffff,0x100d0000); //gratiomaxmin (bit:16~20, 24~28)
			rtd_maskl(PQC_PQDC_MC_HF_PQ_CMP_ALLOCATE_reg,0x0000ffff,0x100d0000); //gratiomaxmin (bit:16~20, 24~28)

			
		}else{	

			rtd_clearbits(PQC_PQDC_MC_LFH_PQ_CMP_PAIR_reg, 0x11); //cmp_data_color(bit:0 & 4)
			rtd_clearbits(PQC_PQDC_MC_HF_PQ_CMP_PAIR_reg, 0x13); //cmp_data_color(bit:0 & 4)
			rtd_clearbits(PQC_PQDC_MC_LFH_PQ_DECMP_PAIR_reg, 0x11); //cmp_data_color(bit:0 & 4)
			rtd_clearbits(PQC_PQDC_MC_HF_PQ_DECMP_PAIR_reg, 0x13); //cmp_data_color(bit:0 & 4)

			rtd_maskl(PQC_PQDC_MC_LFH_PQ_CMP_ENABLE_reg,0xffffff7f,0x00000000); //first_line(bit:7)
			rtd_maskl(PQC_PQDC_MC_HF_PQ_CMP_ENABLE_reg,0xffffff7f,0x00000000); //first_line(bit:7)

			rtd_maskl(PQC_PQDC_MC_LFH_PQ_CMP_ENABLE_reg,0xffffffcf,0x00000020); //444(bit:4~5)
			rtd_maskl(PQC_PQDC_MC_HF_PQ_CMP_ENABLE_reg,0xffffffcf,0x00000020); //444(bit:4~5)

			rtd_maskl(PQC_PQDC_MC_LFH_PQ_CMP_ENABLE_reg,0xffe0ffff,0x000c0000); //gratio(bit:16~20)
			rtd_maskl(PQC_PQDC_MC_HF_PQ_CMP_ENABLE_reg,0xffe0ffff,0x000c0000); //gratio(bit:16~20)

			rtd_maskl(PQC_PQDC_MC_LFH_PQ_CMP_ALLOCATE_reg,0x0000ffff,0x0d090000); //gratiomaxmin (bit:16~20, 24~28)
			rtd_maskl(PQC_PQDC_MC_HF_PQ_CMP_ALLOCATE_reg,0x0000ffff,0x0d090000); //gratiomaxmin (bit:16~20, 24~28)
		}

		rtd_maskl(PQC_PQDC_MC_LFH_PQ_CMP_POOR_reg,0xffffff00,0x00000003); //LF_PQ_CMP_POOR
		rtd_maskl(PQC_PQDC_MC_LFH_PQ_CMP_POOR_reg,0xffff00ff,0x00000600); //LF_PQ_CMP_POOR
		rtd_maskl(PQC_PQDC_MC_LFH_PQ_CMP_POOR_reg,0xff00ffff,0x000c0000); //LF_PQ_CMP_POOR
		rtd_maskl(PQC_PQDC_MC_LFH_PQ_CMP_POOR_reg,0x00ffffff,0x14000000); //LF_PQ_CMP_POOR
		pqc_pqdc_mc_hf_pq_cmp_poor_reg.regValue = rtd_inl(PQC_PQDC_MC_HF_PQ_CMP_POOR_reg);
		pqc_pqdc_mc_hf_pq_cmp_poor_reg.poor_limit_th_qp1 = 3;
		pqc_pqdc_mc_hf_pq_cmp_poor_reg.poor_limit_th_qp2 = 6;
		pqc_pqdc_mc_hf_pq_cmp_poor_reg.poor_limit_th_qp3 = 12;
		pqc_pqdc_mc_hf_pq_cmp_poor_reg.poor_limit_th_qp4 = 20;
		rtd_outl(PQC_PQDC_MC_HF_PQ_CMP_POOR_reg,pqc_pqdc_mc_hf_pq_cmp_poor_reg.regValue);

		//== org setting===
		//rtd_maskl(PQC_PQDC_MC_LFH_PQ_CMP_BIT_reg,0xffff0000,0x00001412); //flb (bit:0~5, 8~13)
		//rtd_maskl(PQC_PQDC_MC_HF_PQ_CMP_BIT_reg,0xffff0000,0x00001614); //flb (bit:0~5, 8~13)
		//== new setting  for BW saving==

		//rtd_maskl(PQC_PQDC_MC_LFH_PQ_CMP_BIT_reg,0xffff0000,0x0000100e); //flb (bit:0~5, 8~13)
		//rtd_maskl(PQC_PQDC_MC_HF_PQ_CMP_BIT_reg,0xffff0000,0x0000110f); //flb (bit:0~5, 8~13)
		rtd_maskl(PQC_PQDC_MC_LFH_PQ_CMP_BIT_reg,0xc0ff0000,0x08001a14); //flb (bit:0~5, 8~13)
		rtd_maskl(PQC_PQDC_MC_HF_PQ_CMP_BIT_reg,0xc0ff0000,0x08001610); //flb (bit:0~5, 8~13)

		//DB apply
		//WriteRegister(PQC_PQDC_MC_PQ_CMP_DB_CTRL_reg, 2, 2, 1);
		
	}
	else// if(GS_MC_8_buffer == 0)//Not 444, but 420 or 422
	{
		WriteRegister(PQC_PQDC_MC_HF_PQ_DECMP_PAIR_reg, 13, 13, 1);
		WriteRegister(PQC_PQDC_MC_HF_PQ_DECMP_PAIR_reg, 12, 12, 1);
		WriteRegister(PQC_PQDC_MC_HF_PQ_DECMP_PAIR_reg, 11, 11, 1);

		//DB enable
		//WriteRegister(PQC_PQDC_MC_PQ_CMP_DB_CTRL_reg, 0, 0, 1);


		rtd_setbits(PQC_PQDC_MC_LFH_PQ_CMP_PAIR_reg, 0x11); //cmp_data_color(bit:0 & 4)

		if(MEMC_Lib_Get_JP_DTV_4K_flag())// PQC 420 mode
		{
			WriteRegister(PQC_PQDC_MC_HF_PQ_CMP_PAIR_reg, 0, 1, 3);
			WriteRegister(PQC_PQDC_MC_HF_PQ_CMP_PAIR_reg, 4, 4, 1); 	
		}
		else// PQC 422 mode
		{
			WriteRegister(PQC_PQDC_MC_HF_PQ_CMP_PAIR_reg, 0, 1, 1);
			WriteRegister(PQC_PQDC_MC_HF_PQ_CMP_PAIR_reg, 4, 4, 1);
		}

		WriteRegister(PQC_PQDC_MC_LFH_PQ_DECMP_PAIR_reg, 0, 0, 1);
		WriteRegister(PQC_PQDC_MC_LFH_PQ_DECMP_PAIR_reg, 4, 4, 1);
	#if IC_K6LP
		if(MEMC_Lib_Get_JP_DTV_4K_flag())// PQC 420 mode
		{
			WriteRegister(PQC_PQDC_MC_HF_PQ_DECMP_PAIR_reg, 0, 1, 3);
			WriteRegister(PQC_PQDC_MC_HF_PQ_DECMP_PAIR_reg, 4, 4, 1); 	
		}
		else// PQC 422 mode
		{
			WriteRegister(PQC_PQDC_MC_HF_PQ_DECMP_PAIR_reg, 0, 1, 1);
			WriteRegister(PQC_PQDC_MC_HF_PQ_DECMP_PAIR_reg, 4, 4, 1);
		}

		WriteRegister(PQC_PQDC_MC_HF_PQ_DECMP_PAIR_reg, 7, 7, 1);
		WriteRegister(PQC_PQDC_MC_HF_PQ_DECMP_PAIR_reg, 10, 10, 1);
	#else
		rtd_setbits(PQC_PQDC_MC_HF_PQ_DECMP_PAIR_reg, 0x11); //cmp_data_color(bit:0 & 4)
	#endif

		rtd_maskl(PQC_PQDC_MC_LFH_PQ_CMP_ENABLE_reg,0xffffff7f,0x00000080); //first_line(bit:7)
		rtd_maskl(PQC_PQDC_MC_HF_PQ_CMP_ENABLE_reg,0xffffff7f,0x00000080); //first_line(bit:7)

		rtd_maskl(PQC_PQDC_MC_LFH_PQ_CMP_ENABLE_reg,0xffffffcf,0x00000000); //444(bit:4~5)
		rtd_maskl(PQC_PQDC_MC_HF_PQ_CMP_ENABLE_reg,0xffffffcf,0x00000000); //444(bit:4~5)

		rtd_maskl(PQC_PQDC_MC_LFH_PQ_CMP_ENABLE_reg,0xffe0ffff,0x00100000); //gratio(bit:16~20)
		rtd_maskl(PQC_PQDC_MC_HF_PQ_CMP_ENABLE_reg,0xffe0ffff,0x00100000); //gratio(bit:16~20)

	#if IC_K7LP
		WriteRegister(PQC_PQDC_MC_LFH_PQ_CMP_POOR_reg, 0, 4, 3);
		WriteRegister(PQC_PQDC_MC_LFH_PQ_CMP_POOR_reg, 8, 12, 6);
		WriteRegister(PQC_PQDC_MC_LFH_PQ_CMP_POOR_reg, 16, 20, 12);
		WriteRegister(PQC_PQDC_MC_LFH_PQ_CMP_POOR_reg, 24, 28, 20);
		pqc_pqdc_mc_hf_pq_cmp_poor_reg.regValue = rtd_inl(PQC_PQDC_MC_HF_PQ_CMP_POOR_reg);
		pqc_pqdc_mc_hf_pq_cmp_poor_reg.poor_limit_th_qp1 = 3;
		pqc_pqdc_mc_hf_pq_cmp_poor_reg.poor_limit_th_qp2 = 6;
		pqc_pqdc_mc_hf_pq_cmp_poor_reg.poor_limit_th_qp3 = 12;
		pqc_pqdc_mc_hf_pq_cmp_poor_reg.poor_limit_th_qp4 = 20;
		rtd_outl(PQC_PQDC_MC_HF_PQ_CMP_POOR_reg,pqc_pqdc_mc_hf_pq_cmp_poor_reg.regValue);
	
	#else //K6LP
		WriteRegister(PQC_PQDC_MC_LFH_PQ_CMP_POOR_reg, 0, 4, 1);
		WriteRegister(PQC_PQDC_MC_LFH_PQ_CMP_POOR_reg, 8, 12, 3);
		WriteRegister(PQC_PQDC_MC_LFH_PQ_CMP_POOR_reg, 16, 20, 6);
		WriteRegister(PQC_PQDC_MC_LFH_PQ_CMP_POOR_reg, 24, 28, 12);
		pqc_pqdc_mc_hf_pq_cmp_poor_reg.regValue = rtd_inl(PQC_PQDC_MC_HF_PQ_CMP_POOR_reg);
		pqc_pqdc_mc_hf_pq_cmp_poor_reg.poor_limit_th_qp1 = 1;
		pqc_pqdc_mc_hf_pq_cmp_poor_reg.poor_limit_th_qp2 = 3;
		pqc_pqdc_mc_hf_pq_cmp_poor_reg.poor_limit_th_qp3 = 6;
		pqc_pqdc_mc_hf_pq_cmp_poor_reg.poor_limit_th_qp4 = 12;
		rtd_outl(PQC_PQDC_MC_HF_PQ_CMP_POOR_reg,pqc_pqdc_mc_hf_pq_cmp_poor_reg.regValue);
	
	
	#endif
		rtd_maskl(PQC_PQDC_MC_LFH_PQ_CMP_ALLOCATE_reg,0x0000ffff,0x100d0000); //gratiomaxmin (bit:16~20, 24~28)
		rtd_maskl(PQC_PQDC_MC_HF_PQ_CMP_ALLOCATE_reg,0x0000ffff,0x100d0000); //gratiomaxmin (bit:16~20, 24~28)

		if(MEMC_Lib_Get_JP_DTV_4K_flag())// PQC 8 bit
		{
			if((GS_MC_8_buffer == 0) || (GS_MC_8_buffer == 1 && MEMC_Lib_get_DisplayRefreshRate() > 60)){
				WriteRegister(PQC_PQDC_MC_HF_PQ_CMP_PAIR_reg, 0, 1, 3);
				WriteRegister(PQC_PQDC_MC_HF_PQ_CMP_PAIR_reg, 4, 4, 1);
				WriteRegister(PQC_PQDC_MC_HF_PQ_DECMP_PAIR_reg, 0, 1, 3);
				WriteRegister(PQC_PQDC_MC_HF_PQ_DECMP_PAIR_reg, 4, 4, 1); 	
				rtd_maskl(PQC_PQDC_MC_LFH_PQ_CMP_BIT_reg,0xc0ff0000,0x08000c09); //flb (bit:0~5, 8~13)
				rtd_maskl(PQC_PQDC_MC_HF_PQ_CMP_BIT_reg,0xc0ff0000,0x08000906); //flb (bit:0~5, 8~13)		
			}
		}
		else// PQC 10 bit
		{
			if((GS_MC_8_buffer == 0) || (GS_MC_8_buffer == 1 && MEMC_Lib_get_DisplayRefreshRate() > 60)){
				WriteRegister(PQC_PQDC_MC_HF_PQ_CMP_PAIR_reg, 0, 1, 1);
				WriteRegister(PQC_PQDC_MC_HF_PQ_CMP_PAIR_reg, 4, 4, 1);
				WriteRegister(PQC_PQDC_MC_HF_PQ_DECMP_PAIR_reg, 0, 1, 1);
				WriteRegister(PQC_PQDC_MC_HF_PQ_DECMP_PAIR_reg, 4, 4, 1);
				rtd_maskl(PQC_PQDC_MC_LFH_PQ_CMP_BIT_reg,0xc0ff0000,0x20000e09); //flb (bit:0~5, 8~13)
				rtd_maskl(PQC_PQDC_MC_HF_PQ_CMP_BIT_reg,0xc0ff0000,0x20000e0a); //flb (bit:0~5, 8~13)
			}
		}
		//DB apply
		//WriteRegister(PQC_PQDC_MC_PQ_CMP_DB_CTRL_reg, 2, 2, 1);
		//rtd_pr_memc_notice("HAL_CPR_Set444Enable, rtd_inl(0xb801a618) = %x, RHAL_TP_IsJapan4K() = %d, RHAL_TP_IsATSC30() = %d\n",rtd_inl(0xb801a618),RHAL_TP_IsJapan4K(), RHAL_TP_IsATSC30());
	}	

	
	//rtd_inl(PQC_PQDC_MC_LFH_PQ_CMP_BIT_reg);
	MEMC_LibSet_PQC_DB_apply();
	
	//rtd_pr_memc_notice("[%s]lf_cmp_data_format=%d, lf_decmp_data_format=%d\n", __FUNCTION__, (rtd_inl(PQC_PQDC_MC_LFH_PQ_CMP_PAIR_reg)&0x3), (rtd_inl(PQC_PQDC_MC_LFH_PQ_DECMP_PAIR_reg)&0x3));
	//rtd_pr_memc_notice("[%s]hf_cmp_data_format=%d, hf_decmp_data_format=%d\n", __FUNCTION__, (rtd_inl(PQC_PQDC_MC_HF_PQ_CMP_PAIR_reg)&0x3), (rtd_inl(PQC_PQDC_MC_HF_PQ_DECMP_PAIR_reg)&0x3));
}


/**
 * @brief This function set budget packet
 * @param [in] unsigned int u32Budget_Packet
 * @retval VOID
*/
VOID HAL_CPR_SetBudgetPacket(unsigned int u32Budget_Packet_Lf, unsigned int u32Budget_Packet_Hf)
{
	//reg_lf_budget_packet
	//WriteRegister(PQC_PQDC_MC_LFH_PQ_CMP_ALLOCATE_reg,0,7,u32Budget_Packet_Lf);	
	//reg_hf_budget_packet
	//WriteRegister(PQC_PQDC_MC_LFH_PQ_CMP_ALLOCATE_reg,16,23,u32Budget_Packet_Hf);		
}


/**
 * @brief This function set last line enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_CPR_SetLastLineEnable(BOOL bEnable)
{
	//reg_lastline_en
	//WriteRegister(PQC_PQDC_MC_PQ_CMP_reg,8,8,(unsigned int)bEnable);	
}

/**
 * @brief This function set LR inverse
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_CPR_SetLRInverse(BOOL bEnable)
{
	//reg_lr_inv
	//WriteRegister(PQC_PQDC_MC_LFH_PQ_CMP_BIT_reg,12,12,(unsigned int)bEnable);	
}

