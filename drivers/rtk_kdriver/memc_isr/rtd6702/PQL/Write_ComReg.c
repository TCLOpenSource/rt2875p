#include "memc_isr/PQL/PQLPlatformDefs.h"
#include "memc_isr/PQL/PQLContext.h"
#include "memc_isr/PQL/PQLGlobalDefs.h"
#include "memc_isr/PQL/IPR.h"
#include "memc_isr/Common/memc_type.h"
#include "memc_isr/HAL/hal_ip.h"
#include "memc_isr/HAL/hal_post.h"
#include "memc_isr/include/memc_lib.h"
//#include <tvscalercontrol/scaler/scalerstruct.h>
#include "memc_reg_def.h"
#include "rbus/kme_dehalo4_reg.h"
#include <rbus/od_reg.h>
#include "memc_isr/include/memc_identification.h"

#define TAG_NAME "MEMC"
extern int LowDelay_mode;
extern unsigned char u8_MEMCMode;
extern unsigned int dejudder;
extern unsigned int g_cur_cadence_id;
extern unsigned int g_cur_cadence_out;
extern unsigned int vpq_project_id;
extern unsigned int vpqex_project_id;
#if 0 //merlin6
unsigned int mc_fb_lvl_force_en_tmp = 0;
unsigned int mc_logo_phase_fb_phase_en = 0;
unsigned int mc_pre_phase_fb_phase_en = 0;
unsigned int mc_pt_phase_fb_phase_en = 0;
unsigned int mc_fb_lvl_force_value = 0;
#else // to sync tv006 patch
//extern webos_strInfo_t webos_strToolOption;

unsigned char g_mc_fb_lvl_force_en = 0;
unsigned char g_mc_logo_phase_fb_phase_en = 0;
unsigned char g_mc_pre_phase_fb_phase_en = 0;
unsigned char g_mc_pt_phase_fb_phase_en = 0;
unsigned char g_mc_fb_lvl_force_value = 0;
#endif

extern unsigned char scalerVIP_Get_MEMCPatternFlag_Identification(unsigned short prj,unsigned short nNumber);
extern unsigned char scalerVIP_Get_MEMCRimFlag(void);
extern unsigned int g_case_num;
extern unsigned int g_frm_cnt;
extern unsigned char get_MEMC_bypass_status_refer_platform_model(void);

#define	COLOR_ICM_D_ICM_CDS_SKIN_0_reg		0xB802A47C
#define	COLOR_ICM_D_ICM_AI_0_reg				0xB802A4BC
#define	COLOR_DCC_D_DCC_CDS_SKIN_0_reg		0xB802E500
#define	COLOR_SHARP_DM_CDS_CM0_U_BOUND_0_reg		0xB802B740

unsigned char MEMC_ID_flag = 0;
extern unsigned char MEMC_ID_GetFlag(unsigned short prj,unsigned short nNumber);
//unsigned char logo_check_K24014_flag = 0;
VOID Wrt_7segment_show(const _PARAM_WRT_COM_REG *pParam)
{
	// dehalo switch
	od_od_ctrl_RBUS od_ctrl_reg;	
	od_ctrl_reg.regValue = rtd_inl(OD_OD_CTRL_reg);	
#ifdef Mark2_Compile_Error_def	
	if(od_ctrl_reg.dummy1802ca00_31_12>>11 & 1) // bit23 to show AI info
#else
	if(od_ctrl_reg.dummy1802ca00_31_10>>13 & 1) // bit23 to show AI info
#endif
	{
#if IS_ENABLED(CONFIG_RTK_AI_DRV)
	extern int show_ai_sc;
	//extern int scene_nn;
	extern int scene_pq;
	extern int sqm_pq;
	extern int ai_genre;
	extern int ai_content;
	extern int ai_resolution;
	unsigned int u32_wrt_data;//, u32_wrt_data2;
	extern unsigned int debug_draw_cnt;
	// =======================================================================================================================
	// 7seg_0 [0~15]
	u32_wrt_data = ((rtd_inl(COLOR_ICM_D_ICM_CDS_SKIN_0_reg)&0x00000001)||((rtd_inl(COLOR_ICM_D_ICM_AI_0_reg)&0x80000000)>>31))||(rtd_inl(COLOR_DCC_D_DCC_CDS_SKIN_0_reg)&0x00000001)||((rtd_inl(COLOR_SHARP_DM_CDS_CM0_U_BOUND_0_reg)&0x10000000)>>28);//shp
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 0, 3, u32_wrt_data);
	u32_wrt_data = show_ai_sc;
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 4, 7, u32_wrt_data);
	u32_wrt_data = scene_pq;
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 8, 11, u32_wrt_data);
	u32_wrt_data = sqm_pq;
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 12, 15, u32_wrt_data);

	/*u32_wrt_data = (rtd_inl(0xB802b740)&0x10000000)>>28;//shp
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 0, 3, u32_wrt_data);
	u32_wrt_data = (rtd_inl(0xb802e500)&0x00000001);//dcc
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 4, 7, u32_wrt_data);
	u32_wrt_data = (rtd_inl(0xb802a47c)&0x00000001)&&((rtd_inl(0xb802a4bc)&0x80000000)>>31);//icm
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 8, 11, u32_wrt_data);
	u32_wrt_data = 0;
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 12, 15, u32_wrt_data);*/
	// =======================================================================================================================

	// =======================================================================================================================
	// 7seg_1 [16~31]
	u32_wrt_data = ai_genre;//scene_nn;
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 16, 31, u32_wrt_data);
	rtd_outl(KPOST_TOP_KPOST_TOP_B0_reg, 0x041f4000);// change position

	// =======================================================================================================================

	// =======================================================================================================================
	// 7seg_2 [0~15]

	u32_wrt_data = (0xf&debug_draw_cnt)|((0xf&ai_resolution)<<12);
	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 0, 15, u32_wrt_data);

	// =======================================================================================================================

	// =======================================================================================================================
	// 7seg_3 [16~31]
	u32_wrt_data = ai_content;//scene_pq;
	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 16, 31, u32_wrt_data);
	rtd_outl(KPOST_TOP_KPOST_TOP_B8_reg, 0x00258000);// change position

	// =======================================================================================================================

	// for scene_detection
#endif

	}
	else
	{
	
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	_PQLPARAMETER *s_pParam       = GetPQLParameter();

	unsigned int  u32_wrt_data = 0;//, u32_wrt_data2;
	unsigned int  u1_show_special = 0;
#ifdef BG_MV_GET
	signed short s11_gmv_mvx = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
	signed short s11_2nd_gmv_mvx = (s_pContext->_output_read_comreg.s11_me_GMV_2nd_vx_rb);
	int gmv_grp0_cnt = 0;
	int gmv_grp0_ucfa = 0;
	int gmv_grp1_cnt = 0;
	int gmv_grp1_ucfa = 0;
	int bg_transfer_mvx_u =0;
	int bg_transfer_mvy_v = 0;
	int print_cursor_mv_en = 0;
#endif

	// =======================================================================================================================
	// 7seg_0 [0~15]
	unsigned int u32_cad_data = 0;
	static unsigned int ES_Test_Show;
	ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 27, 27, &ES_Test_Show);		// YE Test 


	u32_cad_data = ((s_pContext->_output_filmDetectctrl.u1_mixMode == 1 || s_pContext->_output_filmDetectctrl.u8_BadEdit_flag == 1) ? 0xF : s_pContext->_output_filmDetectctrl.u8_cur_cadence_Id[_FILM_ALL])&0xF;
	u32_wrt_data = ((u32_cad_data<<4) | (s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL]&0xF)) & 0xFF;
	if(s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL]>0xf){
		u32_wrt_data = (0x90 + ((s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL]+1)&0xF)) & 0xFF;
	}
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 0, 7, u32_wrt_data);

	//u32_wrt_data = _MIN_(s_pContext->_output_frc_phtable.u8_deblur_lvl_pre >> 3, 0x0F);
	u32_wrt_data = ((rtd_inl(SYS_REG_SYS_CLKEN3_reg)>>30)&0x1)|
				   (((rtd_inl(LBMC_LBMC_24_reg)>>4)&0x1)<<1)|
				   (((rtd_inl(LBMC_LBMC_24_reg)>>13)&0x1)<<2)|
				   (LowDelay_mode<<3);
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 8, 11, u32_wrt_data);

	u32_wrt_data = _MIN_(s_pContext->_output_frc_phtable.u8_dejudder_lvl_pre >> 3, 0x0F);
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 12, 15, u32_wrt_data);
	// =======================================================================================================================

	// =======================================================================================================================
	// 7seg_1 [16~31]


	u32_wrt_data = s_pContext->_output_wrt_comreg.u8_FB_lvl;

	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 16, 23, u32_wrt_data);
//	u32_wrt_data = g_frm_cnt;
//	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 16, 23, u32_wrt_data);

	u32_wrt_data = s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true|
	  			(s_pContext->_output_wrt_comreg.u1_casino_RP_detect_true<< 3);


	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, u32_wrt_data);

	u32_wrt_data = s_pContext->_output_dh_close.u1_closeDh_act|
				  ((s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVx_Step!=0)?0x2:0)|
				  ((s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVy_Step!=0)?0x4:0);

//	u32_wrt_data = s_pContext->_output_frc_sceneAnalysis.u1_RgRotat_true;
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, u32_wrt_data);
//	u32_wrt_data = g_case_num;
//	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 31, u32_wrt_data);
	// =======================================================================================================================



if(ES_Test_Show==1)
{
		u32_wrt_data=0;

				if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag==1){

					u32_wrt_data=0xA;
					WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 28, 31, u32_wrt_data);

			 		u32_wrt_data =s_pContext->_output_bRMV_rFB_ctrl.u32_ES_RFB_ID_show;	
					WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 16, 27, u32_wrt_data);

					}
				if (s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag2==1){
					u32_wrt_data=0xB;
					WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 28, 31, u32_wrt_data);

			 		u32_wrt_data =s_pContext->_output_bRMV_rFB_ctrl.u32_ES_RFB_ID_show;	
					WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 16, 27, u32_wrt_data);
			 		//u32_wrt_data =((0xB<<12))|(s_pContext->_output_bRMV_rFB_ctrl.u32_ES_RFB_ID_show);

					}
				if((s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag==0)&&(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag2==0))
					{
					u32_wrt_data=0;
					WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 16, 31, u32_wrt_data);
					}
				

				
				//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 0, 31, u32_wrt_data);

				//if((s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag==1)||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag2==1))
				//	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 0, 27, u32_wrt_data)
			
			      if(s_pContext->_output_me_sceneAnalysis.u1_RP_ID_flag==1){

					u32_wrt_data=0xC;
					WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 12, 15, u32_wrt_data);

					
			 		u32_wrt_data =s_pContext->_output_me_sceneAnalysis.u32_RP_ID_Show;	
					WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 0, 11, u32_wrt_data);
			      		}
				else
					{
					u32_wrt_data=0;
					WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 0, 15, u32_wrt_data);
					}
		

}
else if(pParam->u1_rFB_show_en == 0)
{
	// =======================================================================================================================
	// 7seg_2 [0~15]
	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 0, 3, s_pContext->_output_dh_close.u5_dh_bad_rgn_num);
	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 4, 7, s_pContext->_output_dh_close.u5_dh_good_rgn_num);

	//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 8, 15, s_pContext->_output_FRC_LgDet.u8_logo_sc_FastDet_cntholdfrm);


	u32_wrt_data = s_pContext->_output_dh_close.u1_meAllDtl_close |
				   (s_pContext->_output_dh_close.u1_lbmc_switch_close << 1) |
				   (s_pContext->_output_dh_close.u1_fblvl_close << 2) |
				   (s_pContext->_output_dh_close.u1_pfvconf_close << 3) |
				   (frc_cadTable[s_pContext->_output_filmDetectctrl.u8_cur_cadence_Id[_FILM_ALL]].u1_isLowFrameRate << 4) |
				   (s_pContext->_output_dh_close.u1_dh_fadeInOut_close << 5) |
				   (s_pContext->_output_dh_close.u1_dh_sml_rgnAction_close << 6)|
				   (s_pContext->_output_dh_close.u1_panning_close<<7);


	
	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 8, 15, u32_wrt_data);

	// debug for special mode
	ReadRegister(SOFTWARE3_SOFTWARE3_10_reg, 0,0,&u1_show_special);
	if(u1_show_special) {
		u32_wrt_data = s_pContext->_external_data.u8_repeatMode;
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 8, 11, u32_wrt_data);
		u32_wrt_data = s_pContext->_external_data.u8_repeatCase;
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 12, 15, u32_wrt_data);
		u32_wrt_data = s_pContext->_output_me_sceneAnalysis.u1_SportScene;
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 4, 7, u32_wrt_data);
	}

		//gmv SR
		//u32_wrt_data = _CLIP_(s_pContext->_output_frc_sceneAnalysis.u8_ME1_searchRange_x,0,255);  //x
		//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 0, 7, u32_wrt_data);
		//u32_wrt_data = _CLIP_(s_pContext->_output_frc_sceneAnalysis.u8_ME1_searchRange_y,0,255);  //y
		//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 8, 15, u32_wrt_data);

		// 7seg_3 [16~31]
		//u32_wrt_data = _CLIP_(s_pContext->_output_frc_sceneAnalysis.u4_fadeInOut_apl_diff_alpha,0,15);
		//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 28, 31, u32_wrt_data);

		// 7seg_3, //// logo clear flag.
		//u32_wrt_data = s_pContext->_output_FRC_LgDet.u1_blkclr_glbstatus;
		//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 16, 16, u32_wrt_data);
		RTKReadRegister(PPOVERLAY_uzudtg_DVS_cnt_reg, &u32_wrt_data);
		u32_wrt_data = (270000000/(u32_wrt_data+1))+5;

		//output timing
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 24, 27, (u32_wrt_data/10)%10);
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 28, 31, (u32_wrt_data/10)/10);

		// motion dir and dtl dir
		//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 24, 27, s_pContext->_output_me_sceneAnalysis.u2_GMV_single_move);
		//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 28, 31, s_pContext->_output_me_sceneAnalysis.u2_mcDtl_HV);

		//zoom[0],swing[1],RgRotat[2],EPG[3]

 {
		u32_wrt_data = s_pContext->_output_frc_sceneAnalysis.u1_Zoom_true  |
				   ((s_pContext->_output_wrt_comreg.u1_IP_preProcess_true|s_pContext->_output_wrt_comreg.u1_IP_wrtAction_true )<< 1) |
				   (s_pContext->_output_frc_sceneAnalysis.u1_RgRotat_true<<2) |
				   ((s_pContext->_output_me_vst_ctrl.u1_detect_blackBG_VST_trure)<<3);
}		
	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 20, 23, u32_wrt_data);

	// panning[0-1], logo clear flag[2], logo sc status[3]
	#if 1
	u32_wrt_data = (s_pContext->_output_me_sceneAnalysis.u2_panning_flag) |
				   (s_pContext->_output_FRC_LgDet.u1_blkclr_glbstatus << 2) |
	               (s_pContext->_output_FRC_LgDet.u1_logo_sc_status << 3);
	//if(pParam->u1_rFB_show_en)
	//	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 16, 19, (s_pContext->_output_bRMV_rFB_ctrl.u1_rFB_panning));
	//else
	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 16, 19, u32_wrt_data);

	#endif
}
else
{
	switch(s_pParam->_param_me_sceneAnalysis.u3_show_mode2)
	{
		case 0:
			// 7seg_2 [0~15]
			WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg,  0, 15, (s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb));
			// 7seg_3 [16~31]
			WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 16, 31, (s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb >> 1));
			break;
		case 1:
			// 7seg_2 [0~15]
			WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg,  0,  7, (s_pContext->_output_me_sceneAnalysis.u8_rTCss_cnt));
			WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg,  8, 15, (s_pContext->_output_me_sceneAnalysis.u8_rPrd_cnt));
			// 7seg_3 [16~31]
			WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 16, 23, (s_pContext->_output_me_sceneAnalysis.u8_rChaos_cnt));
			WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 24, 31, (s_pContext->_output_me_sceneAnalysis.u8_big_relMV_cnt));
			break;
		case 2:
			// 7seg_2 [0~15]
			WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 12, 15, (s_pContext->_output_me_sceneAnalysis.b_2grp_GMV)); 	
			WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg,  8, 11, (s_pContext->_output_me_sceneAnalysis.b_Chaos)); 
			WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg,  4,  7, (s_pContext->_output_me_sceneAnalysis.b_big_relMV)); 
			//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg,  0,  3, (s_pContext->_output_me_sceneAnalysis.b_TCss)); 
			WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg,  0,  3, (s_pContext->_output_me_sceneAnalysis.b_prd_scene)); 
			// 7seg_3 [16~31]
			WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 28, 31, (s_pContext->_output_me_sceneAnalysis.b_logo_scene)); 
			WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 24, 27, (s_pContext->_output_me_sceneAnalysis.u2_panning_flag)); 
			WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 20, 23, (s_pContext->_output_me_sceneAnalysis.b_g_panning)); 
			WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 16, 19, (s_pContext->_output_me_sceneAnalysis.b_do_ME_medFLT)); 				
		default:
			break;
	}
}

	// =======================================================================================================================
	#ifdef BG_MV_GET   //ed
	
	bg_transfer_mvx_u = ((rtd_inl(MC_MC_F0_reg)>>10) & 0x3ff) - 512;
	bg_transfer_mvy_v = ((rtd_inl(MC_MC_F0_reg)>>20) & 0x3ff) - 512;
	gmv_grp0_cnt = ((rtd_inl(KME_ME1_TOP2_KME_ME1_TOP2_C0_reg)) & 0x1ffff);
	gmv_grp0_ucfa = ((rtd_inl(KME_ME1_TOP2_KME_ME1_TOP2_BC_reg)) & 0xfff);
	gmv_grp1_cnt = ((rtd_inl(KME_ME1_TOP2_KME_ME1_TOP2_C4_reg)) & 0x1ffff);
	gmv_grp1_ucfa = ((rtd_inl(KME_ME1_TOP2_KME_ME1_TOP2_BC_reg)>>12) & 0xfff);
	print_cursor_mv_en =  ((rtd_inl(HARDWARE_HARDWARE_26_reg)>>20) & 0x1);
	
	if(print_cursor_mv_en)
	{
		rtd_pr_memc_emerg(",%d,%d,%d,%d,%d,%d,%d,%d\n",
		s11_gmv_mvx,s11_2nd_gmv_mvx,bg_transfer_mvx_u,bg_transfer_mvy_v,gmv_grp0_cnt,gmv_grp1_cnt,gmv_grp0_ucfa,gmv_grp1_ucfa); 	
	}
	#endif

}

#if 0
	// arrange from left to right
	// =======================================================================================================================
	// 7seg_0 [0~15]
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 12, 15, s_pContext->_output_me_sceneAnalysis.u2_panning_flag);
	u32_wrt_data = ( s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true + (s_pContext->_output_wrt_comreg.u1_casino_RP_detect_true << 3) );
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 8, 11, u32_wrt_data);
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 4, 7, s_pContext->_output_dh_close.u1_closeDh_act);
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 0, 3, 0x0);
	// =======================================================================================================================
	// 7seg_1 [16~31]
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, 0x0);
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x0);
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 20, 23, 0x0);
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 16, 19, 0x0);
	// =======================================================================================================================

	// =======================================================================================================================
	// 7seg_2 [0~15]
	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 12, 15, 0x0);
	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg,  8, 11, 0x0);
	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg,  4,  7, 0x0);
	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg,  0,  3, 0x0);
	// =======================================================================================================================

	// =======================================================================================================================
	// 7seg_3 [16~31]
	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 28, 31, 0x0);
	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 24, 27, s_pContext->_output_me_sceneAnalysis.u1_MainObject_Move);
	u32_wrt_data = (scalerVIP_Get_MEMCPatternFlag_Identification(100) || scalerVIP_Get_MEMCPatternFlag_Identification(101))? 3 : s_pContext->_output_me_sceneAnalysis.u3_occl_ext_level;
	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 20, 23, u32_wrt_data);
	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 16, 19, s_pContext->_output_me_sceneAnalysis.u3_occl_post_corr_level);
	// =======================================================================================================================
#endif

#if 0
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned int u32_wrt_data = 0;
	//unsigned int u32_RB_val;

	// =======================================================================================================================
	// 7seg_0 [0~15]
	u32_wrt_data = (s_pContext->_output_dh_close.u1_closeDh_act) + (s_pContext->_output_frc_sceneAnalysis.u6_mv_accord_hold_cnt  << 4);
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 0, 15, u32_wrt_data);
	u32_wrt_data = 0;
	// =======================================================================================================================

	// =======================================================================================================================
	// 7seg_1 [16~31]
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 16, 31, 0);
	// =======================================================================================================================

	// =======================================================================================================================
	// 7seg_2 [0~15]
	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 0, 3,
		(s_pContext->_output_me_sceneAnalysis.u1_pure_TopDownPan_flag ) &&
		(s_pContext->_output_me_sceneAnalysis.u2_panning_flag) );
	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 4, 7, s_pContext->_output_me_sceneAnalysis.u1_pure_TopDownPan_flag);
	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 8, 15, s_pContext->_output_me_sceneAnalysis.u8_pure_TopDownPan_cnt);
	// =======================================================================================================================

	// =======================================================================================================================
	// 7seg_3 [16~31]
	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 16, 19, (s_pContext->_output_me_sceneAnalysis.u2_panning_flag) );
	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 20, 23, (s_pContext->_output_me_sceneAnalysis.u1_Natural_HighFreq_detect_true) );
	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 24, 27, (s_pContext->_output_me_sceneAnalysis.s10_me_GMV_1st_vy_rb_iir > 5) );
	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 28, 31,
		( (s_pContext->_output_me_sceneAnalysis.u2_panning_flag != 0) &&
		  (s_pContext->_output_me_sceneAnalysis.u1_Natural_HighFreq_detect_true != 0) &&
		  (s_pContext->_output_me_sceneAnalysis.s10_me_GMV_1st_vy_rb_iir > 5)
		 )
	);
	// =======================================================================================================================
#endif

	//SE_Film
	//u32_wrt_data = g_cur_cadence_id;
	//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 16, 19, u32_wrt_data);

	//u32_wrt_data = g_cur_cadence_out;
	//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 20, 23, u32_wrt_data);
#if 0

	if (pParam->u8_show7seg_mode == 0)  //common show
	{
		// 7seg_0

	//	u32_wrt_data = (s_pContext->_output_read_comreg.u3_ipme_filmMode_rb >> 1);
		unsigned int u32_cad_data = 0;
		u32_cad_data = ((s_pContext->_output_filmDetectctrl.u1_mixMode == 1) ? 0xF : s_pContext->_output_filmDetectctrl.u8_cur_cadence_Id[_FILM_ALL])&0xF;
	    	u32_wrt_data = ((u32_cad_data<<4) | (s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL]&0xF)) & 0xFF;
		WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 0, 7, u32_wrt_data);

		u32_wrt_data = _MIN_(s_pContext->_output_frc_phtable.u8_deblur_lvl_pre >> 3, 0x0F);
		WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 8, 11, u32_wrt_data);

		u32_wrt_data = _MIN_(s_pContext->_output_frc_phtable.u8_dejudder_lvl_pre >> 3, 0x0F);
		WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 12, 15, u32_wrt_data);

		// 7seg_1
			u32_wrt_data = s_pContext->_output_wrt_comreg.u8_FB_lvl;
		WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 16, 23, u32_wrt_data);

		u32_wrt_data = s_pContext->_output_mc_lbmcswitch.u8_lf_lbmcMode;
		WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, u32_wrt_data);

		u32_wrt_data = s_pContext->_output_dh_close.u1_closeDh_act;
		WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, u32_wrt_data);

		// 7seg_2 SceneAnalysis
		//  fadeIn/out
		u32_wrt_data = _CLIP_(s_pContext->_output_frc_sceneAnalysis.u4_fadeInOut_cnt_alpha,0,15);
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 0, 3, u32_wrt_data);
		u32_wrt_data = _CLIP_(s_pContext->_output_frc_sceneAnalysis.u4_fadeInOut_acdc_sad_alpha,0,15);
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 4, 7, u32_wrt_data);
		u32_wrt_data = _CLIP_(s_pContext->_output_frc_sceneAnalysis.u4_fadeInOut_bv_sad_alpha,0,15);
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 8, 11, u32_wrt_data);
		u32_wrt_data = _CLIP_(s_pContext->_output_frc_sceneAnalysis.u4_fadeInOut_conf,0,15);
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 12, 15, u32_wrt_data);

		//gmv SR
		//u32_wrt_data = _CLIP_(s_pContext->_output_frc_sceneAnalysis.u8_ME1_searchRange_x,0,255);  //x
		//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 0, 7, u32_wrt_data);
		//u32_wrt_data = _CLIP_(s_pContext->_output_frc_sceneAnalysis.u8_ME1_searchRange_y,0,255);  //y
		//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 8, 15, u32_wrt_data);

		// 7seg_3, //// logo clear flag.
		u32_wrt_data = s_pContext->_output_FRC_LgDet.u1_blkclr_glbstatus;
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 16, 16, u32_wrt_data);

		//u32_wrt_data = _CLIP_(s_pContext->_output_frc_sceneAnalysis.u4_fadeInOut_apl_diff_alpha,0,15);
		//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 28, 31, u32_wrt_data);

		RTKReadRegister(0xB80282E4, &u32_wrt_data);
		u32_wrt_data = (270000000/(u32_wrt_data+1))+5;

		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 24, 27, (u32_wrt_data/10)%10);
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 28, 31, (u32_wrt_data/10)/10);

		//u32_wrt_data = s_pContext->_output_FRC_LgDet.u1_blkclr_glbstatus;
		//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 16, 16, u32_wrt_data);   //x
		//u32_wrt_data = _CLIP_UBOUND_(s_pContext->_output_frc_sceneAnalysis.u2_searchRange_x_status,15);
		//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 24, 27, u32_wrt_data);
		//u32_wrt_data = _CLIP_UBOUND_(s_pContext->_output_frc_sceneAnalysis.u2_searchRange_y_status,15);  //y
		//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 28, 31, u32_wrt_data);

		//dehalo close
		u32_wrt_data = s_pContext->_output_dh_close.u1_panning_close;
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 20, 23, u32_wrt_data);

		//logo
		u32_wrt_data = s_pContext->_output_FRC_LgDet.u1_blkclr_glbstatus;
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 16, 19, u32_wrt_data);

	}
	else if (pParam->u8_show7seg_mode == 1) // for boundary
	{
		// 7seg_0
		//avgSC
		u32_wrt_data = _CLIP_(s_pContext->_output_frc_sceneAnalysis.u32_insidePan_avgSc/32, 0, 255)  & 0xFF;
		WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 0, 7, u32_wrt_data);
		//avgTc
		u32_wrt_data = _CLIP_(s_pContext->_output_frc_sceneAnalysis.u32_insidePan_avgTc/32,0,255) & 0xFF;
		WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 8, 15, u32_wrt_data);

		//boundary small
		u32_wrt_data = _CLIP_(s_pContext->_output_frc_sceneAnalysis.u8_BdySmall_lft_cnt,0,15) & 0x0F;
		WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 16, 19, u32_wrt_data);
		u32_wrt_data = _CLIP_(s_pContext->_output_frc_sceneAnalysis.u8_BdySmall_rht_cnt,0,15) & 0x0F;
		WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 20, 23, u32_wrt_data);
		u32_wrt_data = _CLIP_(s_pContext->_output_frc_sceneAnalysis.u8_dynSAD_Diff_Gain,0,255) & 0xFF;
		WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 31, u32_wrt_data);

		u32_wrt_data = _CLIP_(s_pContext->_output_frc_sceneAnalysis.u8_is_insidePan,0,1);
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 0, 3, u32_wrt_data);

		u32_wrt_data = _CLIP_(s_pContext->_output_frc_sceneAnalysis.u1_is_saddiff_large,0,1);
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 4, 7, u32_wrt_data);

		u32_wrt_data = _CLIP_(s_pContext->_output_frc_sceneAnalysis.u32_insidePan_avgAbsMVY/8,0,15);
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 8, 11,  u32_wrt_data);

		u32_wrt_data = _CLIP_(s_pContext->_output_frc_sceneAnalysis.u32_insidePan_avgAbsMVX/8,0,15);
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 12, 15, u32_wrt_data);


		u32_wrt_data =  s_pContext->_output_dh_close.u1_closeDh_act;
		//u32_wrt_data = u32_wrt_data == 1 ? 0 : 1;
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 28, 31,u32_wrt_data);

		u32_wrt_data = _CLIP_(s_pContext->_output_frc_sceneAnalysis.u1_is_BdySmall_MV,0,1);
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 24, 27,u32_wrt_data);

		u32_wrt_data = s_pContext->_output_dh_close.u1_panning_close;
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 20, 23, u32_wrt_data);

		//logo
		u32_wrt_data = s_pContext->_output_FRC_LgDet.u1_blkclr_glbstatus;
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 16, 19,u32_wrt_data);

	}
	else if(pParam->u8_show7seg_mode == 2)  //fade in out
	{
		// 7seg_0 SceneAnalysis
		u32_wrt_data = s_pContext->_output_me_sceneAnalysis.u1_casino_RP_detect_true;
		WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 0, 3, u32_wrt_data);

		u32_wrt_data = s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true;
		WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 4, 7, u32_wrt_data);

		u32_wrt_data = s_pContext->_output_me_sceneAnalysis.u1_ToL10_RP_true;
		WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 8, 11, u32_wrt_data);

		u32_wrt_data = s_pContext->_external_info.u1_PictureMode_Chg;
		WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 12, 15, u32_wrt_data);

		// 7seg_1 SceneAnalysis

		// 7seg_2 SceneAnalysis
		//  fadeIn/out
		u32_wrt_data = _CLIP_(s_pContext->_output_frc_sceneAnalysis.u4_fadeInOut_cnt_alpha,0,15);
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 0, 3, u32_wrt_data);
		u32_wrt_data = _CLIP_(s_pContext->_output_frc_sceneAnalysis.u4_fadeInOut_cnt_Thr_alpha,0,15);
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 4, 7, u32_wrt_data);
		u32_wrt_data = _CLIP_(s_pContext->_output_frc_sceneAnalysis.u4_fadeInOut_apl_diff_alpha,0,15);
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 8, 11, u32_wrt_data);
		u32_wrt_data = _CLIP_(s_pContext->_output_frc_sceneAnalysis.u4_fadeInOut_conf_iir,0,15);
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 12, 15, u32_wrt_data);

		// 7seg_3, //// logo clear flag.
		u32_wrt_data = s_pContext->_output_FRC_LgDet.u1_blkclr_glbstatus;
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 16, 16, u32_wrt_data);     //x

		u32_wrt_data = _CLIP_(s_pContext->_output_frc_sceneAnalysis.u2_searchRange_y_status,0,15);  //y
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 28, 31, u32_wrt_data);

		//u32_wrt_data = _CLIP_(s_pContext->_output_frc_sceneAnalysis.u2_searchRange_x_status,0,15);
		//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 24, 27, u32_wrt_data);

		u32_wrt_data = _CLIP_(s_pContext->_output_frc_sceneAnalysis.u5_dc_ac_bld_alpha,0,15);
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 24, 27, u32_wrt_data);

		u32_wrt_data = s_pContext->_output_dh_close.u1_closeDh_act;
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 20, 23, u32_wrt_data);
	}
	else if (pParam->u8_show7seg_mode == 3)
	{
		u32_wrt_data = s_pContext->_output_frc_sceneAnalysis.s2m_12_cnt_iir  & 0xFF;
		WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 0, 7, u32_wrt_data);

		u32_wrt_data = _MIN_(_ABS_(s_pContext->_output_frc_sceneAnalysis.s2m_1st_mvx)/4, 255) & 0xFF;
		WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 8, 15, u32_wrt_data);

		u32_wrt_data = _MIN_(_ABS_(s_pContext->_output_frc_sceneAnalysis.s2m_1st_mvy)/4, 255) & 0xFF;
		WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 16, 23, u32_wrt_data);

		u32_wrt_data = _MIN_(_ABS_(s_pContext->_output_frc_sceneAnalysis.s2m_2nd_mvx)/8, 15) & 0xF;
		WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, u32_wrt_data);

		u32_wrt_data = _MIN_(_ABS_(s_pContext->_output_frc_sceneAnalysis.s2m_2nd_mvy)/8, 15) & 0xF;
		WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, u32_wrt_data);

		//goodGMVcnt
		u32_wrt_data = _CLIP_UBOUND_(s_pContext->_output_frc_sceneAnalysis.u8_goodGmv_cnt, 255)  & 0xFF;
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 0, 7, u32_wrt_data);
		//diffGMVcnt
		u32_wrt_data = _CLIP_UBOUND_(s_pContext->_output_frc_sceneAnalysis.u8_diffGmv_cnt,255) & 0xFF;
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 8, 15, u32_wrt_data);

		//
		u32_wrt_data = _CLIP_UBOUND_(s_pContext->_output_frc_sceneAnalysis.s2m_1st_cnt, 255)  & 0xFF;
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 16, 23, u32_wrt_data);

		u32_wrt_data = _CLIP_UBOUND_(s_pContext->_output_frc_sceneAnalysis.s2m_2nd_cnt, 255)  & 0xFF;
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 24, 31, u32_wrt_data);
	}
	else if(pParam->u8_show7seg_mode == 4)
	{

		u32_wrt_data = s_pContext->_output_me_sceneAnalysis.u8_EPG_apply>0;
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 28, 31, u32_wrt_data);

		u32_wrt_data = s_pContext->_output_Patch_Manage.u1_patch_highFreq_rp_true;
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 24, 27,u32_wrt_data);

		u32_wrt_data = s_pContext->_output_Patch_Manage.u1_patch_highFreq_zmv_true;
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 20, 23, u32_wrt_data);

		u32_wrt_data = s_pContext->_output_FRC_LgDet.u1_lg_UXN_patch_det;
		WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 16, 19, u32_wrt_data);
	}
#endif
}

#if  0
//extern BOOL MEMC_LibSetMEMCFrameRepeatEnable(BOOL bEnable, BOOL bForce);
VOID FB_PureVideo_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	PQL_INPUT_FRAME_RATE in_fmRate = s_pContext->_external_data._input_frameRate;
	unsigned int Repeat_PureVideo_wrtAction_en = 0;
	unsigned char  u8_bPCMode_flag =  (s_pContext->_external_data._output_mode == _PQL_OUT_PC_MODE) ? 1 : 0;
	ReadRegister(SOFTWARE1_SOFTWARE1_00_reg, 24, 24, &Repeat_PureVideo_wrtAction_en);

	//rtd_pr_memc_notice("[FB_PureVideo_wrtAction][%d, %d]\n", in_fmRate, s_pContext->_output_frc_phtable.u1_is_inFormat_PureVideo);

	if(Repeat_PureVideo_wrtAction_en) // cadence / tru-motion / in frame rate
	{
		if(in_fmRate >= _PQL_IN_48HZ){
			if(s_pContext->_output_frc_phtable.u1_is_inFormat_PureVideo){

				WriteRegister(MC_MC_28_reg, 14, 14,0x1);
				WriteRegister(MC_MC_28_reg, 15, 22,0x0);
				WriteRegister(MC2_MC2_50_reg, 0, 1, 0x0);
				WriteRegister(MC_MC_B0_reg, 6, 7,0x2);
				WriteRegister(MC_MC_B0_reg, 12, 14,0x1);
			}else{

				if(!(( u8_MEMCMode == 0) || (u8_MEMCMode == 4 && dejudder == 0)|| u8_bPCMode_flag )){

					 WriteRegister(MC_MC_28_reg, 14, 14,0x0);
					 WriteRegister(MC_MC_28_reg, 15, 22,0x0);
					 WriteRegister(MC2_MC2_50_reg, 0, 1, 0x3);
				}

				WriteRegister(MC_MC_B0_reg, 6, 7,0x0);
				WriteRegister(MC_MC_B0_reg, 12, 14,0x3);
			}
		}else{

			WriteRegister(MC_MC_B0_reg, 6, 7,0x0);
			WriteRegister(MC_MC_B0_reg, 12, 14,0x3);

		}
	}
}
#endif

static _str_FRC_PureOffON_table FRC_PureOffON_table[2] = {
	{0x1,0x0,0x0,{CTRL_Replace,0x0}},/*OFF*/
	{0x0,0x0,0x1,{CTRL_N,0x0}},/*ON*/
};

_str_FRC_PureOffON_table *fwif_MEMC_get_FRC_PureOffON_table(unsigned char level)
{
//	if (FRC_PureOffON_table == NULL) {
//		return NULL;
//	}
	if(level > 1)
		return (_str_FRC_PureOffON_table *)&FRC_PureOffON_table[0];
	else
		return (_str_FRC_PureOffON_table *)&FRC_PureOffON_table[level];
}

VOID FB_NoNeedToDoFRC_wrtRegister(_str_FRC_PureOffON_table *FRC_PureOffON_table)
{
	WriteRegister(MC_MC_28_reg, 14, 14,FRC_PureOffON_table->mc_fb_lvl_force_en);
	WriteRegister(MC_MC_28_reg, 15, 22,FRC_PureOffON_table->mc_fb_lvl_force_value);
	WriteRegister(HARDWARE_HARDWARE_57_reg, 17, 17, FRC_PureOffON_table->u1_mc2_var_lpf_wrt_en);//u1_mc2_var_lpf_wrt_en
	if(FRC_PureOffON_table->mc_var_lpf_en_ctrl.CTRL_TYPE != CTRL_N){
		WriteRegister(MC2_MC2_50_reg, 0, 1, FRC_PureOffON_table->mc_var_lpf_en_ctrl.mc_var_lpf_en);
	}
}

VOID FB_NoNeedToDoFRC_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	unsigned int NoNeedToDoFRC_wrtAction_en = 0;
	unsigned char  u8_NoNeedToDoFRC = (u8_MEMCMode == 4 && dejudder == 0) ? 1 : 0;
	#if 1
	unsigned char table_level;
	_str_FRC_PureOffON_table *tmp_FRC_PureOffON_table = NULL;
	#endif
	static unsigned char  NoNeedToDoFRC_wrtAction_status = 0;
	ReadRegister(SOFTWARE1_SOFTWARE1_00_reg, 24, 24, &NoNeedToDoFRC_wrtAction_en);

	//rtd_pr_memc_notice("[FB_NoNeedToDoFRC_wrtAction][%d, %d]\n", in_fmRate, s_pContext->_output_frc_phtable.u1_is_inFormat_PureVideo);

	if(NoNeedToDoFRC_wrtAction_en && u8_NoNeedToDoFRC) // cadence / tru-motion / in frame rate
	{
		#if 1
		table_level = 0;
		tmp_FRC_PureOffON_table = fwif_MEMC_get_FRC_PureOffON_table(table_level);
		FB_NoNeedToDoFRC_wrtRegister(tmp_FRC_PureOffON_table);
		#else
		WriteRegister(MC_MC_28_reg, 14, 14,0x1);
		WriteRegister(MC_MC_28_reg, 15, 22,0x0);
		WriteRegister(HARDWARE_HARDWARE_57_reg, 17, 17, 0x0);//u1_mc2_var_lpf_wrt_en
		WriteRegister(MC2_MC2_50_reg, 0, 1, 0x0);
		#endif
		//WriteRegister(FRC_TOP__MC__mc_bld_flbk_bypass_ADDR, 6, 7,0x2);
		//WriteRegister(FRC_TOP__MC__mc_bld_z_type_flbk_ADDR, 12, 14,0x1);
		NoNeedToDoFRC_wrtAction_status = 1;
	}else{

		if(NoNeedToDoFRC_wrtAction_status == 1){
			#if 1
			table_level = 1;
			tmp_FRC_PureOffON_table = fwif_MEMC_get_FRC_PureOffON_table(table_level);
			FB_NoNeedToDoFRC_wrtRegister(tmp_FRC_PureOffON_table);
			#else
			WriteRegister(MC_MC_28_reg, 14, 14,0x0);
			WriteRegister(MC_MC_28_reg, 15, 22,0x0);
			WriteRegister(HARDWARE_HARDWARE_57_reg, 17, 17, 0x1);//u1_mc2_var_lpf_wrt_en
			//WriteRegister(MC_MC_B0_reg, 6, 7,0x0);
			//WriteRegister(MC_MC_B0_reg, 12, 14,0x3);
			#endif
		}
		NoNeedToDoFRC_wrtAction_status = 0;
	}
}

VOID mc_lbmcMode_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	unsigned char u8_lf_lbmcMode = s_pContext->_output_mc_lbmcswitch.u8_lf_lbmcMode;
	unsigned char u8_hf_lbmcMode = s_pContext->_output_mc_lbmcswitch.u8_hf_lbmcMode;
	unsigned char lbmc_mode_from_offset = 8;//enable_lbmc_mode_from

	if (pParam->u1_mc_lbmcMode_en == 1)
	{
		if (u8_lf_lbmcMode != pOutput->u8_lf_lbmcMode_pre)
		{
			//WriteRegister(KPHASE_kphase_00_reg,16,18, u8_lf_lbmcMode);
			WriteRegister(LBMC_LBMC_24_reg,24,27, u8_lf_lbmcMode+lbmc_mode_from_offset);
			pOutput->u8_lf_lbmcMode_pre = u8_lf_lbmcMode;
		}

		if (u8_hf_lbmcMode != pOutput->u8_hf_lbmcMode_pre)
		{
			//WriteRegister(KPHASE_kphase_54_reg,16,18, u8_hf_lbmcMode);
			WriteRegister(LBMC_LBMC_24_reg,28,31,  u8_hf_lbmcMode+lbmc_mode_from_offset);
			WriteRegister(LBMC_LBMC_24_reg,20,23, u8_hf_lbmcMode+lbmc_mode_from_offset);
			pOutput->u8_hf_lbmcMode_pre = u8_hf_lbmcMode;
		}

	}
}

unsigned int wrtRegister_ctrl(_str_wrtRegister_ctrl *wrtRegister_ctrl)
{
	unsigned int wrtRegister_val = 0;
	unsigned int default_val =0, ctrl_val =0;

	default_val = wrtRegister_ctrl->default_val;
	ctrl_val = wrtRegister_ctrl->ctrl_val;

	if( wrtRegister_ctrl->type == CTRL_Pluse ){
		wrtRegister_val = default_val + ctrl_val;
	}else if( wrtRegister_ctrl->type  == CTRL_Minus ){
		wrtRegister_val = default_val - ctrl_val;
	}else if( wrtRegister_ctrl->type  == CTRL_Replace ){
		wrtRegister_val = ctrl_val;
	}else if(wrtRegister_ctrl->type  == CTRL_Default){
		wrtRegister_val = default_val;
	}

	return  wrtRegister_val;
}

VOID scCtrl_wrtRegister(_str_scCtrl_table *scCtrl_table)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	_str_wrtRegister_ctrl _temp_wrtRegister_ctrl;
	unsigned int _temp_mc_scene_change_fb=0;
	unsigned int _temp_me1_sc_saddiff_th = 0;
	unsigned int _temp_big_apldiff_sc_hold_cnt = 0;
	unsigned int _temp_val=0;
	ReadRegister(SOFTWARE3_SOFTWARE3_28_reg, 0, 30, &_temp_val);

	_temp_wrtRegister_ctrl.default_val = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_mc_scene_change_fb;
	_temp_wrtRegister_ctrl.type = (VAL_CTRL_TYPE)scCtrl_table->mc_scene_change_fb_ctrl.CTRL_TYPE;
	_temp_wrtRegister_ctrl.ctrl_val = scCtrl_table->mc_scene_change_fb_ctrl.mc_scene_change_fb;
	_temp_mc_scene_change_fb = (_temp_wrtRegister_ctrl.default_val>0x0 && _temp_wrtRegister_ctrl.default_val<=0xFF)?_temp_mc_scene_change_fb:_temp_wrtRegister_ctrl.default_val;
	if(_temp_wrtRegister_ctrl.type !=CTRL_N ){
		_temp_mc_scene_change_fb = wrtRegister_ctrl(&_temp_wrtRegister_ctrl);
		WriteRegister(MC_MC_30_reg, 10, 10, _temp_mc_scene_change_fb);
	}

	_temp_wrtRegister_ctrl.default_val = s_pContext->_output_frc_sceneAnalysis.u32_new_saddiff_th;
	_temp_wrtRegister_ctrl.type = (VAL_CTRL_TYPE)scCtrl_table->me1_sc_saddiff_ctrl.CTRL_TYPE;
	_temp_wrtRegister_ctrl.ctrl_val = scCtrl_table->me1_sc_saddiff_ctrl.me1_sc_saddiff_th;
	_temp_me1_sc_saddiff_th=(_temp_wrtRegister_ctrl.default_val>0x0 && _temp_wrtRegister_ctrl.default_val<=0x3FFFFFFF)?_temp_me1_sc_saddiff_th:_temp_wrtRegister_ctrl.default_val;
	if(_temp_wrtRegister_ctrl.type !=CTRL_N ){
		_temp_me1_sc_saddiff_th = wrtRegister_ctrl(&_temp_wrtRegister_ctrl);
		if((0x7fffffff-_temp_me1_sc_saddiff_th)<=_temp_val){
			_temp_me1_sc_saddiff_th = 0x7fffffff;
		}
		else {
			_temp_me1_sc_saddiff_th += _temp_val;//d970
		}
		WriteRegister(KME_ME1_TOP1_ME1_SCENE1_07_reg, 0, 30, _temp_me1_sc_saddiff_th);
	}

	_temp_wrtRegister_ctrl.default_val =(unsigned int) s_pContext->_output_me_sceneAnalysis.u8_big_apldiff_sc_hold_cnt;
	_temp_wrtRegister_ctrl.type = (VAL_CTRL_TYPE)scCtrl_table->me1_sc_me1_hold_ctrl.CTRL_TYPE;
	_temp_wrtRegister_ctrl.ctrl_val = (unsigned int)scCtrl_table->me1_sc_me1_hold_ctrl.me1_sc_me1_hold_cnt;
	_temp_big_apldiff_sc_hold_cnt=(_temp_wrtRegister_ctrl.default_val>0x0 && _temp_wrtRegister_ctrl.default_val<=0xFF)?_temp_big_apldiff_sc_hold_cnt:_temp_wrtRegister_ctrl.default_val;

	if(_temp_wrtRegister_ctrl.type !=CTRL_N ){
		_temp_big_apldiff_sc_hold_cnt =wrtRegister_ctrl(&_temp_wrtRegister_ctrl);
		WriteRegister(KME_ME1_TOP1_ME1_SCENE1_02_reg, 0, 7,  _temp_big_apldiff_sc_hold_cnt);
	}

}

static _str_scCtrl_table scCtrl_table[8] = {
		{{CTRL_N,0}, 		{CTRL_N, 0}, 				{CTRL_N,0}			},/* 0.load default */
		{{CTRL_Replace,0}, 	{CTRL_N, 0}, 				{CTRL_N,0}			},/* 1.seek control */
		{{CTRL_Replace,0}, 	{CTRL_Replace, 0xD59F80}, 	{CTRL_Replace,2}	},/* 2.video cadence case use large threshold to prevent easy scene change for moving video text */
		{{CTRL_Replace,0}, 	{CTRL_Pluse, 0xA4CB80},		{CTRL_N,0}			},/*3. */
		{{CTRL_Replace,0}, 	{CTRL_Default, 0},			{CTRL_N,0}			},/*4. */
		{{CTRL_Default,0}, 	{CTRL_Default, 0}, 			{CTRL_Default,0}	},/*5. */
		{{CTRL_Default,0}, 	{CTRL_Default, 0}, 			{CTRL_Replace,1}	},/*6. (u8_cadence_Id == _CAD_22 || (in_fmRate == _PQL_IN_25HZ && u8_cadence_Id == _CAD_VIDEO)) */
		{{CTRL_Default,0}, 	{CTRL_Default, 0}, 			{CTRL_Replace,3}	},/*7. */
};

extern unsigned int cadence_transient_stream_flag;
_str_scCtrl_table *fwif_MEMC_get_scCtrl_table(unsigned char level)
{
//	if (scCtrl_table == NULL) {
//		return NULL;
//	}
//	if(level==255 || level==0)
	if(level > 7)
		return (_str_scCtrl_table *)&scCtrl_table[0];/*load default*/
	else
		return (_str_scCtrl_table *)&scCtrl_table[level];
}

VOID scCtrl_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	static unsigned char u1_ori_sc_me1_en = 0;
	static BOOL u1_sc_me1_en_chg = 0;
	static unsigned char scCtrl_wrt_cnt=0, pre_case_apply = 0;
	//static int SC_hold_panning_cnt = 0;
	unsigned int u32_RB_val;

	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned char  u8_cadence_Id = s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL];
	PQL_INPUT_FRAME_RATE in_fmRate = s_pContext->_external_data._input_frameRate;
	_str_scCtrl_table *tmp_str_scCtrl_table = NULL;
	unsigned char case_apply = 0;
	unsigned int u32_RB_debug, u1_force_case, u3_case_id=0;
	static unsigned char u8_pan_cnt = 0;
	static unsigned char u8_uniform_pan_cnt = 0;
	ReadRegister(SOFTWARE1_SOFTWARE1_25_reg, 31, 31, &u32_RB_debug);
	ReadRegister(SOFTWARE1_SOFTWARE1_25_reg, 24, 24, &u1_force_case);
	ReadRegister(SOFTWARE1_SOFTWARE1_25_reg, 25, 27, &u3_case_id);

	u8_pan_cnt = (u8_pan_cnt>0) ? u8_pan_cnt-1 : 0;
	u8_uniform_pan_cnt = (u8_uniform_pan_cnt>0) ? u8_uniform_pan_cnt-1 : 0;

	if(pParam->u1_scCtrl_wrt_en == 0)
		return;

	//==== for voseek application, need to disable me1_sc when seek ====
	if(s_pContext->_external_info.s16_speed == 256 || s_pContext->_external_info.s16_speed == -256)  // normal speed
	{
		if(u1_sc_me1_en_chg  == 1) //need to recovery
		{
			WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg,9,9,  u1_ori_sc_me1_en);
			u1_sc_me1_en_chg = 0;
		}
	}
	else
	{
		if(u1_sc_me1_en_chg  == 0)
		{
			ReadRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg,9,9, &u32_RB_val);
			u1_ori_sc_me1_en = u32_RB_val;
			u1_sc_me1_en_chg = 1;
		}
		WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg,9,9, 0);
	}

	//==========================================================================

	//if(SC_hold_panning_cnt>0) SC_hold_panning_cnt--;

	if(s_pContext->_external_data._output_mode != _PQL_OUT_PC_MODE)
	{
		#if 1
		if(s_pContext->_output_frc_sceneAnalysis.u8_VOSeek_prt_apply>0)// add by LRL for VO_seek SC blending ghost @20161124
		{
	        case_apply =1;
		}
		else if ((s_pContext->_output_frc_phtable.u1_is_inFormat_PureVideo) ||(s_pContext->_output_filmDetectctrl.u1_mixMode == 1)
			|| (s_pContext->_output_me_sceneAnalysis.u2_panning_flag == 1 && cadence_transient_stream_flag))// video cadence case use large threshold to prevent easy scene change for moving video text
		{
			case_apply =2;
		}
		//else if (( (s_pContext->_output_me_sceneAnalysis.u2_panning_flag == 1) || (s_pContext->_output_me_sceneAnalysis.u1_SC_sim_scenes_hold_panning_flag == 1)) && s_pContext->_output_frc_sceneAnalysis.u1_specialscfb_true)
		else if (( s_pContext->_output_me_sceneAnalysis.u2_panning_flag == 1) && s_pContext->_output_frc_sceneAnalysis.u1_specialscfb_true)
		{
			case_apply = 3;
			u8_pan_cnt = 8;
		}
		else if (s_pContext->_output_me_sceneAnalysis.u1_uniform_panning==1)
		{
			case_apply = 3;
			u8_uniform_pan_cnt = 12;
		}
		else if ((s_pContext->_output_frc_sceneAnalysis.u1_specialscfb_true && u8_pan_cnt>0)||u8_uniform_pan_cnt>0)
		{
			case_apply = 3;
		}
		else if(s_pContext->_output_frc_sceneAnalysis.u1_specialscfb_true)
		{
			case_apply =4;
		}
		else if(s_pContext->_output_me_sceneAnalysis.u8_big_apldiff_hold_cnt != 0)
		{
			case_apply = 5;
		}
		else
		{
			if (u8_cadence_Id == _CAD_22 || (in_fmRate == _PQL_IN_25HZ && u8_cadence_Id == _CAD_VIDEO)){
				case_apply =6;
			}else{
				case_apply = 7;
			}
		}

		if(u1_force_case){//d764[24]
			u3_case_id = _CLIP_(u3_case_id,0,7);//d764[25:27]
			case_apply = u3_case_id;
		}

		tmp_str_scCtrl_table = fwif_MEMC_get_scCtrl_table(case_apply);

		if(case_apply==2 && s_pContext->_output_me_sceneAnalysis.u2_panning_flag == 1 && cadence_transient_stream_flag){
			if(scCtrl_wrt_cnt>0){
				scCtrl_wrt_cnt--;
			}else{
				scCtrl_wrt_cnt =0;
			}
			
			if((case_apply!=pre_case_apply) ){
				scCtrl_wrt_cnt = tmp_str_scCtrl_table->me1_sc_me1_hold_ctrl.me1_sc_me1_hold_cnt;			
			}

			if((case_apply!=pre_case_apply) || (scCtrl_wrt_cnt!=0)){		
				scCtrl_wrtRegister(tmp_str_scCtrl_table);
				pOutput->u4_sc_table_lvl = case_apply;
			}
		}
		else {
			scCtrl_wrtRegister(tmp_str_scCtrl_table);
			pOutput->u4_sc_table_lvl = case_apply;
		}

		pre_case_apply = case_apply;

		if(u32_RB_debug == 1){//d764[31]
			rtd_printk(KERN_EMERG, TAG_NAME,"[%s][%d] case>>%d pan:%d uni_pan:%d fb:%d purevideo:%d mixmode:%d Voseek:%d apldiff:%d cnt:%d sadth:%d\n",
				__FUNCTION__, __LINE__, case_apply,
				s_pContext->_output_me_sceneAnalysis.u2_panning_flag, s_pContext->_output_me_sceneAnalysis.u1_uniform_panning, s_pContext->_output_frc_sceneAnalysis.u1_specialscfb_true,
				s_pContext->_output_frc_phtable.u1_is_inFormat_PureVideo, s_pContext->_output_filmDetectctrl.u1_mixMode,
				s_pContext->_output_frc_sceneAnalysis.u8_VOSeek_prt_apply, s_pContext->_output_me_sceneAnalysis.u8_big_apldiff_hold_cnt,
				scCtrl_wrt_cnt, rtd_inl(KME_ME1_TOP1_ME1_SCENE1_07_reg)&0x7fffffff );
		}

		#else
	//	if(u32_RB_debug == 1){
	//		rtd_pr_memc_emerg("(%d, %d),%d,%d, %d, %d\n", s_pContext->_output_me_sceneAnalysis.u2_panning_flag, s_pContext->_output_frc_sceneAnalysis.u1_specialscfb_true,SC_hold_panning_cnt
	//			, s_pContext->_output_me_sceneAnalysis.u8_big_apldiff_hold_cnt, s_pContext->_output_frc_sceneAnalysis.u32_new_saddiff_th, s_pContext->_output_me_sceneAnalysis.u32_big_apldiff_saddiff_th);
	//	}
		if(s_pContext->_output_frc_sceneAnalysis.u8_VOSeek_prt_apply>0)// add by LRL for VO_seek SC blending ghost @20161124
		{
	                WriteRegister(MC_MC_30_reg, 10, 10, 0x0);
		}
		else if ((s_pContext->_output_frc_phtable.u1_is_inFormat_PureVideo) ||(s_pContext->_output_filmDetectctrl.u1_mixMode == 1) )// video cadence case use large threshold to prevent easy scene change for moving video text
		{
			WriteRegister(MC_MC_30_reg, 10, 10, 0x0);
			WriteRegister(KME_ME1_TOP1_ME1_SCENE1_07_reg, 0, 30, 0x980000);
			WriteRegister(KME_ME1_TOP1_ME1_SCENE1_02_reg, 0, 7, 0x2);
		}
		//else if (( (s_pContext->_output_me_sceneAnalysis.u2_panning_flag == 1) || (s_pContext->_output_me_sceneAnalysis.u1_SC_sim_scenes_hold_panning_flag == 1)) && s_pContext->_output_frc_sceneAnalysis.u1_specialscfb_true)
		else if (( s_pContext->_output_me_sceneAnalysis.u2_panning_flag == 1) && s_pContext->_output_frc_sceneAnalysis.u1_specialscfb_true)
		{
			//SC_hold_panning_cnt = 18;
			WriteRegister(MC_MC_30_reg, 10, 10, 0x0);
			WriteRegister(KME_ME1_TOP1_ME1_SCENE1_07_reg, 0, 30, s_pContext->_output_frc_sceneAnalysis.u32_new_saddiff_th + 0x400000);
		}
		else if(s_pContext->_output_frc_sceneAnalysis.u1_specialscfb_true)
		{
				WriteRegister(MC_MC_30_reg, 10, 10, 0x0);
				WriteRegister(KME_ME1_TOP1_ME1_SCENE1_07_reg, 0, 30, s_pContext->_output_frc_sceneAnalysis.u32_new_saddiff_th);
		}
		else if(s_pContext->_output_me_sceneAnalysis.u8_big_apldiff_hold_cnt != 0)
		{
			WriteRegister(MC_MC_30_reg, 10, 10, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_mc_scene_change_fb);
			WriteRegister(KME_ME1_TOP1_ME1_SCENE1_07_reg, 0, 30, s_pContext->_output_me_sceneAnalysis.u32_big_apldiff_saddiff_th);
			WriteRegister(KME_ME1_TOP1_ME1_SCENE1_02_reg, 0, 7, s_pContext->_output_me_sceneAnalysis.u8_big_apldiff_sc_hold_cnt);
		}
		else
		{

			//if( SC_hold_panning_cnt>3){
		//		WriteRegister(MC_MC_30_reg, 10, 10, 0x0);
		//		WriteRegister(KME_ME1_TOP1_ME1_SCENE1_07_reg, 0, 30, s_pContext->_output_frc_sceneAnalysis.u32_new_saddiff_th + 0x500000);
		//	}else{
				WriteRegister(MC_MC_30_reg, 10, 10, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_mc_scene_change_fb);
				WriteRegister(KME_ME1_TOP1_ME1_SCENE1_07_reg, 0, 30, s_pContext->_output_frc_sceneAnalysis.u32_new_saddiff_th);
		//	}

			if (u8_cadence_Id == _CAD_22 || (in_fmRate == _PQL_IN_25HZ && u8_cadence_Id == _CAD_VIDEO))
			{
				WriteRegister(KME_ME1_TOP1_ME1_SCENE1_02_reg, 0, 7, 0x1);
			}
			else
			{
				WriteRegister(KME_ME1_TOP1_ME1_SCENE1_02_reg, 0, 7, 0x3);
			}
		}
		#endif


	}
	else
	{
		ReadRegister(MC_MC_30_reg, 10, 10, &u32_RB_val);
		if(u32_RB_val)
		{
			rtd_pr_memc_notice("err: PC mode but sc fb not disable.\n");
			WriteRegister(MC_MC_30_reg, 10, 10, 0x0);
		}
	}

}

VOID sc_enCtrl_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	static unsigned char  cur_cadence;
	static unsigned char  pre_cadence;
	static unsigned char  sc_cad_chg_frame_hold_cnt;


	const _PQLCONTEXT *s_pContext = GetPQLContext();

	if(pParam->u1_scCtrl_wrt_en == 0)
		return;

	cur_cadence  = s_pContext ->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL];
	if((cur_cadence == _CAD_2224)&&(pre_cadence ==_CAD_VIDEO))
	{
			sc_cad_chg_frame_hold_cnt = 16;
	}

	if(sc_cad_chg_frame_hold_cnt>0)
	{
			WriteRegister(KME_ME1_TOP1_ME1_SCENE1_02_reg, 0, 7, 0xf);
			sc_cad_chg_frame_hold_cnt--;
	}
	else
	{
			WriteRegister(KME_ME1_TOP1_ME1_SCENE1_02_reg, 0, 7, 0x8);

	}
	pre_cadence = cur_cadence;
}

VOID fbCtrl_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	_PQLPARAMETER *s_pParam       = GetPQLParameter();

	unsigned int u8_flbk_lvl_wrt = (unsigned int)(s_pContext->_output_fblevelctrl.u8_FBLevel);
	unsigned char  u8_flbk_system = s_pContext->_output_fblevelctrl.u8_FBSystem;

	unsigned char  u8_flbk_lvl_FB_FF_Speed = (s_pContext->_external_info.s16_speed != 256 && s_pContext->_external_info.s16_speed != -256) ? _FB_LEVEL_MAX_ : 0;
	unsigned char  u8_flbk_lvl_zoom = (s_pContext->_output_frc_sceneAnalysis.u8_zoom_force_fb_cnt != 0) ? _FB_LEVEL_MAX_ : 0;
	#if CONFIG_MC_8_BUFFER
	unsigned char  u8_inFormat_PureVideo =  0;
	#else
	unsigned char  u8_inFormat_PureVideo =  (s_pContext->_external_data._output_frameRate > _PQL_OUT_60HZ) ? 0 :
									(s_pContext->_output_frc_phtable.u1_is_inFormat_PureVideo != 0) ? _FB_LEVEL_MAX_ : 0;
	#endif
	unsigned char  u8_NoNeedToDoFRC = (s_pContext->_external_data._output_frameRate > _PQL_OUT_60HZ) ? 0 :
								((u8_MEMCMode == 4 && dejudder == 0)||u8_MEMCMode ==5 ) ? _FB_LEVEL_MAX_ : 0; // FallBack to P_frame to avoid judder
	unsigned int NewLowDelay_en = 0;
	ReadRegister(SOFTWARE1_SOFTWARE1_62_reg, 2, 2, &NewLowDelay_en);
	u8_flbk_lvl_wrt = _MAX_(_MAX_(_MAX_(_MAX_(_MAX_(u8_flbk_lvl_FB_FF_Speed
						, u8_flbk_lvl_zoom)
						, u8_flbk_system)
						, u8_flbk_lvl_wrt)
						, u8_inFormat_PureVideo)
						, u8_NoNeedToDoFRC);

	if(s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type==1)
		u8_flbk_lvl_wrt=0x40;
	else if(s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type==2)
		u8_flbk_lvl_wrt=0x60;
	else if(s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type==3)
		u8_flbk_lvl_wrt=0x50;
	else if (s_pContext->_output_wrt_comreg.u1_GFB_force_Custom_true==1)
		u8_flbk_lvl_wrt=0xC0;
	else if(s_pContext->_output_frc_sceneAnalysis.u1_Swing_true)
		u8_flbk_lvl_wrt=0x00;
	else if(s_pParam->_param_me_sceneAnalysis.b_pan_noFB_en && s_pContext->_output_me_sceneAnalysis.b_g_panning)
		u8_flbk_lvl_wrt=0x00;
	else if(s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true==1)
		u8_flbk_lvl_wrt=0x00;
	else if(s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true ==1)
		u8_flbk_lvl_wrt=0xff;

	if(NewLowDelay_en)
		u8_flbk_lvl_wrt=0x55;

	if(s_pContext->_output_wrt_comreg.u1_GFB_modify_true==0)
		pOutput->u8_FB_lvl = u8_flbk_lvl_wrt;

	
	if (pParam->u1_FBLevelWr_en == 1)
	{
		WriteRegister(MC_MC_B0_reg,20,27,  pOutput->u8_FB_lvl);
	}

	if(pParam->u1_bigFBlvl_dis_SCdet_en == 1)
	{
		unsigned char u1_disable_saddiff_sc = s_pContext->_output_fblevelctrl.u3_FbLvl_dSCclr_wrt;
		WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg,2,2, u1_disable_saddiff_sc);
	}
}

extern unsigned char SLD_SW_En;
extern unsigned char MEMC_Pixel_LOGO_For_SW_SLD;
VOID RimCtrl_wrtAction(const _PARAM_WRT_COM_REG *pParam)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	mc_mc_50_RBUS mc_mc_50;
	mc_mc_54_RBUS mc_mc_54;
	mc_mc_58_RBUS mc_mc_58;
	mc_mc_5c_RBUS mc_mc_5c;
	
	if ((pParam->u1_RimCtrl_wrt_en == 1) && (s_pContext->_output_rimctrl.u1_RimChange == 1))
	{
		// MC
		mc_mc_50.regValue = rtd_inl(MC_MC_50_reg);
		mc_mc_54.regValue = rtd_inl(MC_MC_54_reg);
		mc_mc_58.regValue = rtd_inl(MC_MC_58_reg);
		mc_mc_5c.regValue = rtd_inl(MC_MC_5C_reg);

		// Rim1
		mc_mc_58.mc_top_rim1 = s_pContext->_output_rimctrl.u12_mc_rim1[_RIM_TOP];
		mc_mc_58.mc_bot_rim1 = s_pContext->_output_rimctrl.u12_mc_rim1[_RIM_BOT];
		mc_mc_5c.mc_lft_rim1 = s_pContext->_output_rimctrl.u12_mc_rim1[_RIM_LFT];
		mc_mc_5c.mc_rht_rim1 = s_pContext->_output_rimctrl.u12_mc_rim1[_RIM_RHT];

		// Rim0
		mc_mc_50.mc_top_rim0 = s_pContext->_output_rimctrl.u12_mc_rim0[_RIM_TOP];
		mc_mc_50.mc_bot_rim0 = s_pContext->_output_rimctrl.u12_mc_rim0[_RIM_BOT];
		mc_mc_54.mc_lft_rim0 = s_pContext->_output_rimctrl.u12_mc_rim0[_RIM_LFT];
		mc_mc_54.mc_rht_rim0 = s_pContext->_output_rimctrl.u12_mc_rim0[_RIM_RHT];

		rtd_outl(MC_MC_50_reg, mc_mc_50.regValue);
		rtd_outl(MC_MC_54_reg, mc_mc_54.regValue);
		rtd_outl(MC_MC_58_reg, mc_mc_58.regValue);
		rtd_outl(MC_MC_5C_reg, mc_mc_5c.regValue);

#if 0
{
		unsigned char u8_LogoRim[_RIM_NUM];
		// LOGO 
		if(s_pContext->_output_rimctrl.u8_LogoRim_Blk[_RIM_LFT]>2){
			u8_LogoRim[_RIM_LFT]=s_pContext->_output_rimctrl.u8_LogoRim_Blk[_RIM_LFT]-2;
		}
		else {
			u8_LogoRim[_RIM_LFT]=0;
		}
		if(s_pContext->_output_rimctrl.u8_LogoRim_Blk[_RIM_TOP]>2){
			u8_LogoRim[_RIM_TOP]=s_pContext->_output_rimctrl.u8_LogoRim_Blk[_RIM_TOP]-2;
		}
		else {
			u8_LogoRim[_RIM_TOP]=0;
		}
		if(s_pContext->_output_rimctrl.u8_LogoRim_Blk[_RIM_RHT]<237){
			u8_LogoRim[_RIM_RHT]=s_pContext->_output_rimctrl.u8_LogoRim_Blk[_RIM_RHT]+2;
		}
		else {
			u8_LogoRim[_RIM_RHT]=239;
		}

		WriteRegister(KME_LOGO1_KME_LOGO1_04_reg,0,7, u8_LogoRim[_RIM_LFT]); // BLOCK
		WriteRegister(KME_LOGO1_KME_LOGO1_04_reg,8,15, u8_LogoRim[_RIM_RHT]);
		WriteRegister(KME_LOGO1_KME_LOGO1_04_reg,16,23, u8_LogoRim[_RIM_TOP]);
		WriteRegister(KME_LOGO1_KME_LOGO1_04_reg,24,31, s_pContext->_output_rimctrl.u8_LogoRim_Blk[_RIM_BOT]);
}
#else		
		// LOGO
		WriteRegister(KME_LOGO1_KME_LOGO1_04_reg,0,7, s_pContext->_output_rimctrl.u8_LogoRim_Blk[_RIM_LFT]); // BLOCK
		WriteRegister(KME_LOGO1_KME_LOGO1_04_reg,8,15, s_pContext->_output_rimctrl.u8_LogoRim_Blk[_RIM_RHT]);
		WriteRegister(KME_LOGO1_KME_LOGO1_04_reg,16,23, s_pContext->_output_rimctrl.u8_LogoRim_Blk[_RIM_TOP]);
		WriteRegister(KME_LOGO1_KME_LOGO1_04_reg,24,31, s_pContext->_output_rimctrl.u8_LogoRim_Blk[_RIM_BOT]);
#endif

#ifdef CONFIG_SUPPORT_SLD_BY_MEMC_DEMURA
		if (/*strcmp(webos_strToolOption.eBackLight, "oled") == 0*//*SLD_SW_En == 1*/MEMC_Pixel_LOGO_For_SW_SLD == 1)
		{
			WriteRegister(KME_LOGO1_KME_LOGO1_D8_reg,0,9, 0); // PIXEL
			WriteRegister(KME_LOGO1_KME_LOGO1_D8_reg,10,19, 959);
			WriteRegister(KME_LOGO1_KME_LOGO1_DC_reg,0,9, 0);
			WriteRegister(KME_LOGO1_KME_LOGO1_DC_reg,10,19, 539);
		}
#else
		WriteRegister(KME_LOGO1_KME_LOGO1_D8_reg,0,9, s_pContext->_output_rimctrl.u12_LogoRim_Pxl[_RIM_LFT]); // PIXEL
		WriteRegister(KME_LOGO1_KME_LOGO1_D8_reg,10,19, s_pContext->_output_rimctrl.u12_LogoRim_Pxl[_RIM_RHT]);
		WriteRegister(KME_LOGO1_KME_LOGO1_DC_reg,0,9, s_pContext->_output_rimctrl.u12_LogoRim_Pxl[_RIM_TOP]);
		WriteRegister(KME_LOGO1_KME_LOGO1_DC_reg,10,19, s_pContext->_output_rimctrl.u12_LogoRim_Pxl[_RIM_BOT]);
#endif

		WriteRegister(MC2_MC2_9C_reg,0,9, s_pContext->_output_rimctrl.u8_LogoRim_Blk[_RIM_LFT]); // BLOCK
		WriteRegister(MC2_MC2_9C_reg,10,19, s_pContext->_output_rimctrl.u8_LogoRim_Blk[_RIM_RHT]);
		WriteRegister(MC2_MC2_9C_reg,20,29, s_pContext->_output_rimctrl.u8_LogoRim_Blk[_RIM_TOP]);
		WriteRegister(MC2_MC2_A0_reg,0,9, s_pContext->_output_rimctrl.u8_LogoRim_Blk[_RIM_BOT]);

#ifdef CONFIG_SUPPORT_SLD_BY_MEMC_DEMURA
		if (/*strcmp(webos_strToolOption.eBackLight, "oled") == 0*//*SLD_SW_En == 1*/MEMC_Pixel_LOGO_For_SW_SLD == 1)
		{
			WriteRegister(MC2_MC2_A4_reg,0,9, 0); // PIXEL
			WriteRegister(MC2_MC2_A4_reg,10,19, 959);
			WriteRegister(MC2_MC2_A4_reg,20,29, 0);
			WriteRegister(MC2_MC2_A8_reg,0,9, 539);
		}
#else
		WriteRegister(MC2_MC2_A4_reg,0,9, s_pContext->_output_rimctrl.u12_LogoRim_Pxl[_RIM_LFT]); // PIXEL
		WriteRegister(MC2_MC2_A4_reg,10,19, s_pContext->_output_rimctrl.u12_LogoRim_Pxl[_RIM_RHT]);
		WriteRegister(MC2_MC2_A4_reg,20,29, s_pContext->_output_rimctrl.u12_LogoRim_Pxl[_RIM_TOP]);
		WriteRegister(MC2_MC2_A8_reg,0,9, s_pContext->_output_rimctrl.u12_LogoRim_Pxl[_RIM_BOT]);
#endif

		// Dehalo
		WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,0,8,  s_pContext->_output_rimctrl.u9_dh_blk_rim[_RIM_LFT]); // Dehalo rim.
		WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,9,17,  s_pContext->_output_rimctrl.u9_dh_blk_rim[_RIM_RHT]);
		WriteRegister(KME_DEHALO_KME_DEHALO_E4_reg,0,8,   s_pContext->_output_rimctrl.u9_dh_blk_rim[_RIM_TOP]);
		WriteRegister(KME_DEHALO_KME_DEHALO_E4_reg,9,17,   s_pContext->_output_rimctrl.u9_dh_blk_rim[_RIM_BOT]);

		WriteRegister(KME_DEHALO_KME_DEHALO_18_reg,0,8,  s_pContext->_output_rimctrl.u9_dh_blk_oftrim[_RIM_LFT]); // Dehalo retimer rim.
		WriteRegister(KME_DEHALO_KME_DEHALO_18_reg,9,17, s_pContext->_output_rimctrl.u9_dh_blk_oftrim[_RIM_RHT]);
		WriteRegister(KME_DEHALO_KME_DEHALO_18_reg,18,26,   s_pContext->_output_rimctrl.u9_dh_blk_oftrim[_RIM_TOP]);
		WriteRegister(KME_DEHALO_KME_DEHALO_1C_reg,0,8,   s_pContext->_output_rimctrl.u9_dh_blk_oftrim[_RIM_BOT]);

		WriteRegister(KME_DEHALO_KME_DEHALO_1C_reg,9,17,  s_pContext->_output_rimctrl.u9_dh_blk_oftrim[_RIM_LFT]); // Dehalo retimer1 rim.
		WriteRegister(KME_DEHALO_KME_DEHALO_1C_reg,18,26, s_pContext->_output_rimctrl.u9_dh_blk_oftrim[_RIM_RHT]);
		WriteRegister(KME_DEHALO_KME_DEHALO_20_reg,0,8,   s_pContext->_output_rimctrl.u9_dh_blk_oftrim[_RIM_TOP]);
		WriteRegister(KME_DEHALO_KME_DEHALO_20_reg,9,17,   s_pContext->_output_rimctrl.u9_dh_blk_oftrim[_RIM_BOT]);

		WriteRegister(KME_DEHALO_KME_DEHALO_20_reg,18,26,  s_pContext->_output_rimctrl.u9_dh_blk_oftrim[_RIM_LFT]); // Dehalo retimer2 rim.
		WriteRegister(KME_DEHALO_KME_DEHALO_24_reg,0,8, s_pContext->_output_rimctrl.u9_dh_blk_oftrim[_RIM_RHT]);
		WriteRegister(KME_DEHALO_KME_DEHALO_24_reg,9,17,   s_pContext->_output_rimctrl.u9_dh_blk_oftrim[_RIM_TOP]);
		WriteRegister(KME_DEHALO_KME_DEHALO_24_reg,18,26,   s_pContext->_output_rimctrl.u9_dh_blk_oftrim[_RIM_BOT]);

		// IPME
		WriteRegister(KME_IPME_KME_IPME_A0_reg,0,9, s_pContext->_output_rimctrl.u12_ipme_rim[_RIM_TOP]); // IPME
		WriteRegister(KME_IPME_KME_IPME_A0_reg,10,19, s_pContext->_output_rimctrl.u12_ipme_rim[_RIM_BOT]);
		WriteRegister(KME_IPME_KME_IPME_9C_reg,0,9, s_pContext->_output_rimctrl.u12_ipme_rim[_RIM_LFT]);
		WriteRegister(KME_IPME_KME_IPME_9C_reg,10,19, s_pContext->_output_rimctrl.u12_ipme_rim[_RIM_RHT]);

		// ME1
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_3C_reg,16,24,   s_pContext->_output_rimctrl.u12_me1_vbuf_blk_rim[_RIM_TOP]); //blk rim
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_40_reg,18,26,   s_pContext->_output_rimctrl.u12_me1_vbuf_blk_rim[_RIM_BOT]);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_40_reg,0,8,  s_pContext->_output_rimctrl.u12_me1_vbuf_blk_rim[_RIM_LFT]);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_40_reg,9,17, s_pContext->_output_rimctrl.u12_me1_vbuf_blk_rim[_RIM_RHT]);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_44_reg,0,9,  s_pContext->_output_rimctrl.u12_me1_vbuf_pix_rim[_RIM_TOP]); //pixel rim
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_44_reg,10,19,  s_pContext->_output_rimctrl.u12_me1_vbuf_pix_rim[_RIM_BOT]);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_48_reg,0,9, s_pContext->_output_rimctrl.u12_me1_vbuf_pix_rim[_RIM_LFT]);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_48_reg,10,19,s_pContext->_output_rimctrl.u12_me1_vbuf_pix_rim[_RIM_RHT]);

		WriteRegister(KME_LBME_TOP_KME_LBME_TOP_00_reg,0,10,  s_pContext->_output_rimctrl.u12_me1_lbme_rim[_RIM_LFT]);
		WriteRegister(KME_LBME_TOP_KME_LBME_TOP_00_reg,11,21,    s_pContext->_output_rimctrl.u12_me1_lbme_rim[_RIM_RHT]);
		WriteRegister(KME_LBME_TOP_KME_LBME_TOP_04_reg,0,10,  s_pContext->_output_rimctrl.u12_me1_lbme_rim[_RIM_TOP]);
		WriteRegister(KME_LBME_TOP_KME_LBME_TOP_04_reg,11,21,    s_pContext->_output_rimctrl.u12_me1_lbme_rim[_RIM_BOT]);

		WriteRegister(KME_LBME_TOP_KME_LBME_TOP_1C_reg,0,10,  s_pContext->_output_rimctrl.u12_me1_lbme_rim[_RIM_LFT]);
		WriteRegister(KME_LBME_TOP_KME_LBME_TOP_1C_reg,11,21,    s_pContext->_output_rimctrl.u12_me1_lbme_rim[_RIM_RHT]);
		WriteRegister(KME_LBME_TOP_KME_LBME_TOP_20_reg,0,10,  s_pContext->_output_rimctrl.u12_me1_vbuf_meander_pix_rim[_RIM_TOP]);
		WriteRegister(KME_LBME_TOP_KME_LBME_TOP_20_reg,11,21,    s_pContext->_output_rimctrl.u12_me1_vbuf_meander_pix_rim[_RIM_BOT]);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_84_reg,0,8, s_pContext->_output_rimctrl.u12_me1_vbuf_meander_blk_rim[_RIM_TOP]); //meander rim
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_84_reg,9,17, s_pContext->_output_rimctrl.u12_me1_vbuf_meander_blk_rim[_RIM_BOT]);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_84_reg,18,27, s_pContext->_output_rimctrl.u12_me1_vbuf_meander_pix_rim[_RIM_TOP]);
// MEMC_K6L_BRING_UP //		WriteRegister(FRC_TOP__KME_TOP__vbuf_meander_pix_rim_bot_ADDR no mat,FRC_TOP__KME_TOP__vbuf_meander_pix_rim_bot_BITSTART no mat,FRC_TOP__KME_TOP__vbuf_meander_pix_rim_bot_BITEND no mat, s_pContext->_output_rimctrl.u12_me1_vbuf_meander_pix_rim[_RIM_BOT]);

		// ME2
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_20_reg,9,17,  s_pContext->_output_rimctrl.u12_me2_vbuf_pfv_blk_rim[_RIM_LFT]); // pfv blk rim
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_20_reg,18,26, s_pContext->_output_rimctrl.u12_me2_vbuf_pfv_blk_rim[_RIM_RHT]);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_24_reg,0,8,   s_pContext->_output_rimctrl.u12_me2_vbuf_pfv_blk_rim[_RIM_TOP]);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_24_reg,9,17,   s_pContext->_output_rimctrl.u12_me2_vbuf_pfv_blk_rim[_RIM_BOT]);

		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_24_reg,18,26,  s_pContext->_output_rimctrl.u12_me2_vbuf_ph_blk_rim[_RIM_LFT]); // ph blk rim
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_28_reg,0,8, s_pContext->_output_rimctrl.u12_me2_vbuf_ph_blk_rim[_RIM_RHT]);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_28_reg,9,17,   s_pContext->_output_rimctrl.u12_me2_vbuf_ph_blk_rim[_RIM_TOP]);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_28_reg,18,26,   s_pContext->_output_rimctrl.u12_me2_vbuf_ph_blk_rim[_RIM_BOT]);

		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D4_reg,12,21,  s_pContext->_output_rimctrl.u12_me2_vbuf_pix_rim[_RIM_LFT]); // pixel
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D4_reg,22,31, s_pContext->_output_rimctrl.u12_me2_vbuf_pix_rim[_RIM_RHT]);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg,0,9,   s_pContext->_output_rimctrl.u12_me2_vbuf_pix_rim[_RIM_TOP]);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg,10,19,   s_pContext->_output_rimctrl.u12_me2_vbuf_pix_rim[_RIM_BOT]);

		WriteRegister(KME_LBME2_TOP_KME_LBME2_TOP_00_reg,0,10, s_pContext->_output_rimctrl.u12_me2_lbme_rim[_RIM_LFT]); // lbme2
		WriteRegister(KME_LBME2_TOP_KME_LBME2_TOP_00_reg,11,21,   s_pContext->_output_rimctrl.u12_me2_lbme_rim[_RIM_RHT]);
		WriteRegister(KME_LBME2_TOP_KME_LBME2_TOP_04_reg,0,10, s_pContext->_output_rimctrl.u12_me2_lbme_rim[_RIM_TOP]);
		WriteRegister(KME_LBME2_TOP_KME_LBME2_TOP_04_reg,11,21,   s_pContext->_output_rimctrl.u12_me2_lbme_rim[_RIM_BOT]);
		#if PQL_DYNAMIC_ADJUST_FIX_ISSUE
		if (s_pContext->_output_rimctrl.u1_RimTwoSide_balancing)
		{
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_38_reg, 0, 0, 0x1);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_78_reg, 0, 0, 0x1);
		}
		else
		{
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_38_reg, 0, 0, 0x0);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_78_reg, 0, 0, 0x0);
		}
		#endif
	}
}
#if 0
VOID dhLocalFbLvl_wrtAction(const _PARAM_WRT_COM_REG *pParam)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	_PQLPARAMETER *s_pParam       = GetPQLParameter();

	unsigned char u6_localfallback_lvl = s_pParam->_param_frc_sceneAnalysis.u6_localfallback_lvl;//((U32_DATA>>24) & 0x3f);
	unsigned char u1_fastMotion_lfb_en = s_pParam->_param_frc_sceneAnalysis.u1_fastMotion_lfb_en; //((U32_DATA>>31) & 0x1);

	if (pParam->u1_localfb_wrt_en == 0)
		return;

	if(s_pContext->_output_frc_sceneAnalysis.u1_fastMotion_det == 1)
	{
		if(u1_fastMotion_lfb_en == 1)
		{
			WriteRegister(KME_DEHALO_KME_DEHALO_D4_reg,18,23, u6_localfallback_lvl);
		}
	}
	else
	{
		WriteRegister(KME_DEHALO_KME_DEHALO_D4_reg,18,23, 0x20);


	}


}
#endif

VOID dhClose_wrtAction(const _PARAM_WRT_COM_REG *pParam)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	//_PQLPARAMETER *s_pParam       = GetPQLParameter();

	unsigned int u1_dehalo_en = (s_pContext->_output_dh_close.u1_closeDh_act == 1)? 0 : 1;
	// dehalo switch	
	if (pParam->u1_dhClose_wrt_en == 1)
	{
		if (u1_dehalo_en == 0)
		{
			//WriteRegister(KME_DEHALO2_KME_DEHALO2_30_reg,17,17, 1);	//regional gain
			WriteRegister(KME_DEHALO_KME_DEHALO_BC_reg,17,22, 0x3F);
		}
		else
		{
			//WriteRegister(KME_DEHALO2_KME_DEHALO2_30_reg,17,17, 0);	//global gain
			WriteRegister(KME_DEHALO_KME_DEHALO_BC_reg,17,22, 0x10);
		}
	}


}

#ifdef BG_MV_GET
VOID gmvCtrl_wrtAction(const _PARAM_WRT_COM_REG *pParam)
{
	
	
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	//_PQLPARAMETER *s_pParam       = GetPQLParameter();

	int s1_gmv_calc_mvx = s_pContext->_output_me_sceneAnalysis.s1_gmv_calc_mvx;
	int s1_gmv_calc_mvy = s_pContext->_output_me_sceneAnalysis.s1_gmv_calc_mvy;
	unsigned int swap_gmv_en = 0;

	rtd_pr_memc_emerg("gmvCtrl_wrtAction (in)\n");
	
	ReadRegister(HARDWARE_HARDWARE_26_reg, 21, 21, &swap_gmv_en);

	//write value to 0xb809ee38 bg_mv.
	if(swap_gmv_en)
	{
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 1);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, s1_gmv_calc_mvx);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, s1_gmv_calc_mvy);
	}
	else
	{
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0);
	}	
}
#endif

static _str_ME_obmeSelection_table ME_obmeSelection_table[4]={
	{CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N},/*0 load default*/
	{1,0,2,	1,0,2,	1,0,2,	1,0,2	},			/*1*/
	{0,CTRL_N,CTRL_N,0,CTRL_N,CTRL_N,1,3,0,1,3,0},	/*2*/
	{0,CTRL_N,CTRL_N,0,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N},/*3*/
};

_str_ME_obmeSelection_table *fwif_MEMC_get_ME_obmeSelection_table(unsigned char level)
{
	if(level > 3)
		return (_str_ME_obmeSelection_table *)&ME_obmeSelection_table[0];/*load default*/
	else
		return (_str_ME_obmeSelection_table *)&ME_obmeSelection_table[level];

}

VOID ME_obmeSelection_wrtRegister(const _str_ME_obmeSelection_table *ME_obmeSelection_table)
{
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,22,22, ME_obmeSelection_table->vbuf_ip_dc_obme_mode_sel);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,18,19, ME_obmeSelection_table->vbuf_ip_dc_obme_mode); // 1
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,24,25, ME_obmeSelection_table->vbuf_ip_ac_obme_mode); // 1

	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,23,23, ME_obmeSelection_table->vbuf_pi_dc_obme_mode_sel);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,20,21, ME_obmeSelection_table->vbuf_pi_dc_obme_mode); // 1
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,26,27, ME_obmeSelection_table->vbuf_pi_ac_obme_mode); // 1

	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_DC_reg,2,2, 		ME_obmeSelection_table->kme_me2_vbuf_1st_dc_obme_mode_sel);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg,28,29, 	ME_obmeSelection_table->kme_me2_vbuf_1st_dc_obme_mode);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg,21,22, 	ME_obmeSelection_table->kme_me2_vbuf_1st_ac_obme_mode);

	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_DC_reg,3,3, 			ME_obmeSelection_table->kme_me2_vbuf_2nd_dc_obme_mode_sel);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg,30,31, 	ME_obmeSelection_table->kme_me2_vbuf_2nd_dc_obme_mode);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg,23,24, 	ME_obmeSelection_table->kme_me2_vbuf_2nd_ac_obme_mode);
}


VOID ME_obmeSelection_wrtAction(const _PARAM_WRT_COM_REG *pParam)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	_PQLPARAMETER *s_pParam       = GetPQLParameter();
	unsigned char table_level = 255;
	_str_ME_obmeSelection_table *tmp_ME_obmeSelection_table = fwif_MEMC_get_ME_obmeSelection_table(0);
	unsigned int u1_dehalo_en = (s_pContext->_output_dh_close.u1_closeDh_act == 1)? 0 : 1;

	#if 1
	if (pParam->u1_dynamic_obme_sel_wrt_en == 1)
	{
		if (u1_dehalo_en == 0)
		{
			if( s_pContext->_output_me_sceneAnalysis.u2_panning_flag == 1 && s_pContext->_output_FRC_LgDet.u1_logo_rgn_distribute[LOGO_RGN_BOT] == 1 )
			{
				table_level = 1;
				tmp_ME_obmeSelection_table = fwif_MEMC_get_ME_obmeSelection_table(table_level);
			}
			else
			{
				table_level = 2;
				tmp_ME_obmeSelection_table = fwif_MEMC_get_ME_obmeSelection_table(table_level);

				tmp_ME_obmeSelection_table->vbuf_ip_dc_obme_mode = s_pParam->_param_dh_close.u2_dh_me1_ip_dc_obme_mode_off;
				tmp_ME_obmeSelection_table->vbuf_ip_ac_obme_mode = s_pParam->_param_dh_close.u2_dh_me1_ip_ac_obme_mode_off;

				tmp_ME_obmeSelection_table->vbuf_pi_dc_obme_mode = s_pParam->_param_dh_close.u2_dh_me1_pi_dc_obme_mode_off;
				tmp_ME_obmeSelection_table->vbuf_pi_ac_obme_mode = s_pParam->_param_dh_close.u2_dh_me1_pi_ac_obme_mode_off;
			}
		}
		else
		{
			table_level = 3;
			tmp_ME_obmeSelection_table = fwif_MEMC_get_ME_obmeSelection_table(table_level);

			tmp_ME_obmeSelection_table->vbuf_ip_dc_obme_mode = s_pParam->_param_dh_close.u2_dh_me1_ip_dc_obme_mode_on;
			tmp_ME_obmeSelection_table->vbuf_ip_ac_obme_mode = s_pParam->_param_dh_close.u2_dh_me1_ip_ac_obme_mode_on;

			tmp_ME_obmeSelection_table->vbuf_pi_dc_obme_mode = s_pParam->_param_dh_close.u2_dh_me1_pi_dc_obme_mode_on;
			tmp_ME_obmeSelection_table->vbuf_pi_ac_obme_mode = s_pParam->_param_dh_close.u2_dh_me1_pi_ac_obme_mode_on;

			tmp_ME_obmeSelection_table->kme_me2_vbuf_1st_dc_obme_mode_sel 		=	s_pContext->_output_FRC_LgDet.DefaultVal.me2_1st_dc_obme_mode_sel;
			tmp_ME_obmeSelection_table->kme_me2_vbuf_1st_dc_obme_mode		=	s_pContext->_output_FRC_LgDet.DefaultVal.me2_1st_dc_obme_mode;
			tmp_ME_obmeSelection_table->kme_me2_vbuf_1st_ac_obme_mode		=	s_pContext->_output_FRC_LgDet.DefaultVal.me2_1st_ac_obme_mode;

			tmp_ME_obmeSelection_table->kme_me2_vbuf_2nd_dc_obme_mode_sel 	=	s_pContext->_output_FRC_LgDet.DefaultVal.me2_2nd_dc_obme_mode_sel;
			tmp_ME_obmeSelection_table->kme_me2_vbuf_2nd_dc_obme_mode		=	s_pContext->_output_FRC_LgDet.DefaultVal.me2_2nd_dc_obme_mode;
			tmp_ME_obmeSelection_table->kme_me2_vbuf_2nd_ac_obme_mode		=	s_pContext->_output_FRC_LgDet.DefaultVal.me2_2nd_ac_obme_mode;
		}
	}

	if(tmp_ME_obmeSelection_table != NULL){
		ME_obmeSelection_wrtRegister(tmp_ME_obmeSelection_table);
	}

	#else
	if (pParam->u1_dynamic_obme_sel_wrt_en == 1)
	{
		if (u1_dehalo_en == 0)
		{
			if( s_pContext->_output_me_sceneAnalysis.u2_panning_flag == 1 && s_pContext->_output_FRC_LgDet.u1_logo_rgn_distribute[LOGO_RGN_BOT] == 1 )
			{
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,22,22, 1);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,18,19, 0);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,24,25, 2);

				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,23,23, 1);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,20,21, 0);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,26,27, 2);

				WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_DC_reg,2,2, 1);
				WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg,28,29, 0);
				WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg,21,22, 2);

				WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_DC_reg,3,3, 1);
				WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg,30,31, 0);
				WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg,23,24, 2);
			}
			else
			{
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,22,22, 0);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,18,19, s_pParam->_param_dh_close.u2_dh_me1_ip_dc_obme_mode_off); // 3
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,24,25, s_pParam->_param_dh_close.u2_dh_me1_ip_ac_obme_mode_off); // 0

				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,23,23, 0);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,20,21, s_pParam->_param_dh_close.u2_dh_me1_pi_dc_obme_mode_off); // 3
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,26,27, s_pParam->_param_dh_close.u2_dh_me1_pi_ac_obme_mode_off); // 0

				WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_DC_reg,2,2, 1);
				WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg,28,29, 3);
				WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg,21,22, 0);

				WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_DC_reg,3,3, 1);
				WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg,30,31, 3);
				WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg,23,24, 0);
			}
		}
		else
		{
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,22,22, 0);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,18,19, s_pParam->_param_dh_close.u2_dh_me1_ip_dc_obme_mode_on); // 1
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,24,25, s_pParam->_param_dh_close.u2_dh_me1_ip_ac_obme_mode_on); // 1

			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,23,23, 0);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,20,21, s_pParam->_param_dh_close.u2_dh_me1_pi_dc_obme_mode_on); // 1
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,26,27, s_pParam->_param_dh_close.u2_dh_me1_pi_ac_obme_mode_on); // 1

			WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_DC_reg,2,2, s_pContext->_output_FRC_LgDet.DefaultVal.me2_1st_dc_obme_mode_sel);
			WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg,28,29, s_pContext->_output_FRC_LgDet.DefaultVal.me2_1st_dc_obme_mode);
			WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg,21,22, s_pContext->_output_FRC_LgDet.DefaultVal.me2_1st_ac_obme_mode);

			WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_DC_reg,3,3, s_pContext->_output_FRC_LgDet.DefaultVal.me2_2nd_dc_obme_mode_sel);
			WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg,30,31, s_pContext->_output_FRC_LgDet.DefaultVal.me2_2nd_dc_obme_mode);
			WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg,23,24, s_pContext->_output_FRC_LgDet.DefaultVal.me2_2nd_ac_obme_mode);
		}
	}
	#endif
}

VOID mv_accord_wrtAction(const _PARAM_WRT_COM_REG *pParam)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned int u1_dehalo_en = (s_pContext->_output_dh_close.u1_closeDh_act == 1)? 0 : 1;

	if (pParam->u1_dh_accord_ctrl_en == 1)
	{
		if(s_pContext->_output_frc_sceneAnalysis.u6_mv_accord_hold_cnt > 0)
		{
			WriteRegister(KME_DEHALO_KME_DEHALO_B4_reg,29,29, 1);
		}
		else if(u1_dehalo_en == 0)
		{
			WriteRegister(KME_DEHALO_KME_DEHALO_B4_reg,29,29, 1);
		}
		else
		{
			WriteRegister(KME_DEHALO_KME_DEHALO_B4_reg,29,29, 0);
		}
	}
}


VOID ME_acdcBld_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	if(pParam->u1_MEacdcBld_wrt_en== 0)
		return;
	if (pParam->u1_dynME_acdcBld_wrt_en == 1  )
	{
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_00_reg,0,7, s_pContext->_output_frc_sceneAnalysis.u8_dynME_acdc_bld_x_act[0]);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_00_reg,8,15, s_pContext->_output_frc_sceneAnalysis.u8_dynME_acdc_bld_x_act[1]);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_00_reg,16,23, s_pContext->_output_frc_sceneAnalysis.u8_dynME_acdc_bld_x_act[2]);

		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_00_reg,24,29, s_pContext->_output_frc_sceneAnalysis.u6_dynME_acdc_bld_y_act[0]);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_01_reg,0,5, s_pContext->_output_frc_sceneAnalysis.u6_dynME_acdc_bld_y_act[1]);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_01_reg,6,11, s_pContext->_output_frc_sceneAnalysis.u6_dynME_acdc_bld_y_act[2]);

		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_01_reg,12,20, s_pContext->_output_frc_sceneAnalysis.s9_dynME_acdc_bld_s_act[0]);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_01_reg,21,29, s_pContext->_output_frc_sceneAnalysis.s9_dynME_acdc_bld_s_act[1]);

		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_04_reg,24,31, s_pContext->_output_frc_sceneAnalysis.u8_dynME_ac_pix_sad_limt_act);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_44_reg,24,31, s_pContext->_output_frc_sceneAnalysis.u8_dynME_ac_pix_sad_limt_act);

		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_88_reg,0,3, s_pContext->_output_frc_sceneAnalysis.u4_acdc_bld_psad_shift_act);

		#if PQL_DYNAMIC_ADJUST_FIX_ISSUE
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_50_reg,0,7, s_pContext->_output_frc_sceneAnalysis.u8_dynME2_acdc_bld_x_act[0]);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_50_reg,8,15, s_pContext->_output_frc_sceneAnalysis.u8_dynME2_acdc_bld_x_act[1]);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_50_reg,16,23, s_pContext->_output_frc_sceneAnalysis.u8_dynME2_acdc_bld_x_act[2]);

		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_50_reg,24,29, s_pContext->_output_frc_sceneAnalysis.u6_dynME2_acdc_bld_y_act[0]);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_54_reg,0,5, s_pContext->_output_frc_sceneAnalysis.u6_dynME2_acdc_bld_y_act[1]);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_54_reg,6,11, s_pContext->_output_frc_sceneAnalysis.u6_dynME2_acdc_bld_y_act[2]);

		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_54_reg,12,20, s_pContext->_output_frc_sceneAnalysis.s9_dynME2_acdc_bld_s_act[0]);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_54_reg,21,29, s_pContext->_output_frc_sceneAnalysis.s9_dynME2_acdc_bld_s_act[1]);
		#endif
	}
	else
	{
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_00_reg,0,7, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS .u8_dynME_acdc_bld_x_act[0]);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_00_reg,8,15, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS .u8_dynME_acdc_bld_x_act[1]);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_00_reg,16,23, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS .u8_dynME_acdc_bld_x_act[2]);

		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_00_reg,24,29, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS .u6_dynME_acdc_bld_y_act[0]);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_01_reg,0,5, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS .u6_dynME_acdc_bld_y_act[1]);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_01_reg,6,11, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS .u6_dynME_acdc_bld_y_act[2]);

		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_01_reg,12,20, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS .s9_dynME_acdc_bld_s_act[0]);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_01_reg,21,29, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS .s9_dynME_acdc_bld_s_act[1]);

		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_04_reg,24,31, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS .u8_dynME_ac_pix_sad_limt_act);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_44_reg,24,31, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS .u8_dynME_ac_pix_sad_limt_act);

		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_88_reg,0,3, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS .u4_acdc_bld_psad_shift_act);
		#if PQL_DYNAMIC_ADJUST_FIX_ISSUE
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_50_reg,0,7,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.u8_dynME2_acdc_bld_x_act[0]);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_50_reg,8,15,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.u8_dynME2_acdc_bld_x_act[1]);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_50_reg,16,23,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.u8_dynME2_acdc_bld_x_act[2]);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_50_reg,24,29,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.u6_dynME2_acdc_bld_y_act[0]);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_54_reg,0,5,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.u6_dynME2_acdc_bld_y_act[1]);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_54_reg,6,11,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.u6_dynME2_acdc_bld_y_act[2]);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_54_reg,12,20,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.s9_dynME2_acdc_bld_s_act[0]);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_54_reg,21,29,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.s9_dynME2_acdc_bld_s_act[1]);
		#endif
	}
}

static _str_ME_dynSR_table ME_dynSR_table[4]={
	{CTRL_N,CTRL_N,CTRL_N,CTRL_N},	/*0 load default*/
	{0,0,0,0},						/*1*/
	{60,60,CTRL_N,CTRL_N},			/*2*/
	{CTRL_N,CTRL_N,70,70},			/*3*/
};

_str_ME_dynSR_table *fwif_MEMC_get_ME_dynSR_table(unsigned char level)
{
//	if (ME_dynSR_table == NULL) {
//		return NULL;
//	}
//	if(level==255 || level==0)
	if(level > 3)
		return (_str_ME_dynSR_table *)&ME_dynSR_table[0];/*load default*/
	else
		return (_str_ME_dynSR_table *)&ME_dynSR_table[level];

}

VOID ME_dynSR_wrtRegister(_str_ME_dynSR_table *ME_dynSR_table)
{
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_3C_reg, 0,	8,	ME_dynSR_table->vbuf_mvx_range);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_3C_reg, 9,	15,	ME_dynSR_table->vbuf_mvy_range);
	#if (IC_K5LP || IC_K6LP || IC_K8LP)
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_E0_reg,0,7,		ME_dynSR_table->kme_me2_vbuf_mvx_clip_range);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_E0_reg,8,14,	ME_dynSR_table->kme_me2_vbuf_mvy_clip_range);
	#endif
}

VOID ME_dynSR_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	_PQLPARAMETER *s_pParam       = GetPQLParameter();
	unsigned char table_level = 255;/* 255->load defaule */
	_str_ME_dynSR_table *tmp_ME_dynSR_table = NULL;

	if (s_pParam->_param_frc_sceneAnalysis.u1_dynSR_en == 0)
		return;
#if 1
	if(s_pContext->_output_frc_sceneAnalysis.u1_still_frame || s_pContext->_output_Patch_Manage.u1_patch_highFreq_zmv_true)
	{
		table_level = 1;
		tmp_ME_dynSR_table = fwif_MEMC_get_ME_dynSR_table(table_level);
	}
	else if(s_pContext->_output_wrt_comreg.u1_KME_SR_increaseX_true ==1)
	{
		table_level = 2;
		tmp_ME_dynSR_table = fwif_MEMC_get_ME_dynSR_table(table_level);
		tmp_ME_dynSR_table->vbuf_mvy_range = s_pContext->_output_frc_sceneAnalysis.u8_ME1_searchRange_y;
		tmp_ME_dynSR_table->kme_me2_vbuf_mvy_clip_range = s_pContext->_output_frc_sceneAnalysis.u8_ME1_searchRange_y;

	}
	else if(s_pContext->_output_wrt_comreg.u1_KME_SR_increaseY_true ==1)
	{
		table_level = 3;
		tmp_ME_dynSR_table = fwif_MEMC_get_ME_dynSR_table(table_level);
		tmp_ME_dynSR_table->vbuf_mvx_range = s_pContext->_output_frc_sceneAnalysis.u8_ME1_searchRange_x;
		tmp_ME_dynSR_table->kme_me2_vbuf_mvx_clip_range = s_pContext->_output_frc_sceneAnalysis.u8_ME1_searchRange_x;
	}
	else
	{
		table_level = 255;
		tmp_ME_dynSR_table = fwif_MEMC_get_ME_dynSR_table(table_level);
		tmp_ME_dynSR_table->vbuf_mvx_range = s_pContext->_output_frc_sceneAnalysis.u8_ME1_searchRange_x;
		tmp_ME_dynSR_table->vbuf_mvy_range = s_pContext->_output_frc_sceneAnalysis.u8_ME1_searchRange_y;
		tmp_ME_dynSR_table->kme_me2_vbuf_mvx_clip_range = s_pContext->_output_frc_sceneAnalysis.u8_ME1_searchRange_x;
		tmp_ME_dynSR_table->kme_me2_vbuf_mvy_clip_range = s_pContext->_output_frc_sceneAnalysis.u8_ME1_searchRange_y;
	}

	ME_dynSR_wrtRegister(tmp_ME_dynSR_table);
#else
	if(s_pContext->_output_frc_sceneAnalysis.u1_still_frame || s_pContext->_output_Patch_Manage.u1_patch_highFreq_zmv_true)
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_3C_reg, 0,8,0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_3C_reg,9,15, 0);
		#if (IC_K5LP || IC_K6LP)
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_E0_reg,0,7,0);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_E0_reg,8,14,0);
		#endif
	}
	else if(s_pContext->_output_wrt_comreg.u1_KME_SR_increaseX_true ==1)
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_3C_reg, 0,8,0x60);
		#if (IC_K5LP || IC_K6LP)
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_E0_reg,0,7,0x60);
		#endif
	}
	else if(s_pContext->_output_wrt_comreg.u1_KME_SR_increaseY_true ==1)
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_3C_reg,9,15, 0x70);
		#if (IC_K5LP || IC_K6LP)
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_E0_reg,8,14,0x70);
		#endif
	}
	else
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_3C_reg,0,8,
								s_pContext->_output_frc_sceneAnalysis.u8_ME1_searchRange_x);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_3C_reg,9,15,
								s_pContext->_output_frc_sceneAnalysis.u8_ME1_searchRange_y);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_E0_reg,0,7,
								s_pContext->_output_frc_sceneAnalysis.u8_ME1_searchRange_x);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_E0_reg,8,14,
								s_pContext->_output_frc_sceneAnalysis.u8_ME1_searchRange_y);
	}
#endif
}


VOID ME_dynSAD_Diff_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	_PQLPARAMETER *s_pParam       = GetPQLParameter();
	if (s_pParam->_param_frc_sceneAnalysis.u1_dyn_SAD_Diff_en == 1)
	{
		WriteRegister(KME_ME1_TOP1_ME1_SCENE1_07_reg,0,30,
			          (0xE0000 * s_pContext->_output_frc_sceneAnalysis.u8_dynSAD_Diff_Gain) / 255);
	}
	else
	{
		WriteRegister(KME_ME1_TOP1_ME1_SCENE1_07_reg,0,30, 0xE0000);
	}
}

static _str_ME_cand_table ME_cand_table[]={
//
	/*0:load default*/
	{CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,
	 CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,
	 CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,
	 CTRL_NONE,CTRL_NONE},
	/*1*/
	{CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,
	 CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,
	 0,0,0,CTRL_NONE,0,0,0,CTRL_NONE,
	 CTRL_NONE,CTRL_NONE},	
	/*2*/
	{CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,
	 CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,
	 0,CTRL_NONE,0,CTRL_NONE,0,CTRL_NONE,0,CTRL_NONE,
	 CTRL_NONE,CTRL_NONE},
	/*3*/
	{CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,
	 CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,
	 CTRL_NONE,0,0,CTRL_NONE,CTRL_NONE,0,0,CTRL_NONE,
	 CTRL_NONE,CTRL_NONE},
	/*4*/
	{CTRL_NONE,CTRL_NONE,CTRL_NONE,0,0,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,
	 CTRL_NONE,CTRL_NONE,CTRL_NONE,0,0,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,
	 CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,
	 CTRL_NONE,CTRL_NONE},
	/*5*/
	{CTRL_NONE,CTRL_NONE,CTRL_NONE,0,0,0,0,0,0,
	 CTRL_NONE,CTRL_NONE,CTRL_NONE,0,0,0,0,0,0,
	 CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,
	 CTRL_NONE,CTRL_NONE},
	/*6*/
	{CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,
	 CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,
	 CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,
	 1,CTRL_NONE},
	/*7*/
	{CTRL_NONE,CTRL_NONE,CTRL_NONE,0,0,0,0,0,CTRL_NONE,
	 CTRL_NONE,CTRL_NONE,CTRL_NONE,0,0,0,0,0,CTRL_NONE,
	 CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,
	 CTRL_NONE,CTRL_NONE},
	/*8 repeat broken*/
	{CTRL_NONE,0,CTRL_NONE,CTRL_NONE,CTRL_NONE,0,0,0,0,
	 CTRL_NONE,0,CTRL_NONE,CTRL_NONE,CTRL_NONE,0,0,0,0,
	 CTRL_NONE,CTRL_NONE,CTRL_NONE,0,CTRL_NONE,CTRL_NONE,CTRL_NONE,0,
	 0,0},
	/*9 small object*/
	{CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,0,
	 CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,0,
	 CTRL_NONE,CTRL_NONE,CTRL_NONE,1,CTRL_NONE,CTRL_NONE,CTRL_NONE,1,
	 0,0},
};

_str_ME_cand_table *fwif_MEMC_get_ME_cand_table(unsigned char level)
{
//	if (ME_cand_table == NULL) {
//		return NULL;
//	}
//	if(level==255 || level==0)
	if(level > sizeof(ME_cand_table)/sizeof(_str_ME_cand_table))
		return (_str_ME_cand_table *)&ME_cand_table[0];/*load default*/
	else
		return (_str_ME_cand_table *)&ME_cand_table[level];


}

VOID ME_cand_wrtRegister(_str_ME_cand_table *ME_cand_table)
{
 	const _PQLCONTEXT *s_pContext = GetPQLContext();

	if (ME_cand_table->vbuf_ip_1st_cand0_en == CTRL_NONE) ME_cand_table->vbuf_ip_1st_cand0_en = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand0_en;
	if (ME_cand_table->vbuf_ip_1st_cand1_en == CTRL_NONE) ME_cand_table->vbuf_ip_1st_cand1_en = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand1_en;
	if (ME_cand_table->vbuf_ip_1st_cand2_en == CTRL_NONE) ME_cand_table->vbuf_ip_1st_cand2_en = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand2_en;
	if (ME_cand_table->vbuf_ip_1st_cand3_en == CTRL_NONE) ME_cand_table->vbuf_ip_1st_cand3_en = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand3_en;
	if (ME_cand_table->vbuf_ip_1st_cand4_en == CTRL_NONE) ME_cand_table->vbuf_ip_1st_cand4_en = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand4_en;
	if (ME_cand_table->vbuf_ip_1st_cand5_en == CTRL_NONE) ME_cand_table->vbuf_ip_1st_cand5_en = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand5_en;
	if (ME_cand_table->vbuf_ip_1st_cand6_en == CTRL_NONE) ME_cand_table->vbuf_ip_1st_cand6_en = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand6_en;
	if (ME_cand_table->vbuf_ip_1st_cand7_en == CTRL_NONE) ME_cand_table->vbuf_ip_1st_cand7_en = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand7_en;
	if (ME_cand_table->vbuf_ip_1st_cand8_en == CTRL_NONE) ME_cand_table->vbuf_ip_1st_cand8_en = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand8_en;
	if (ME_cand_table->vbuf_pi_1st_cand0_en == CTRL_NONE) ME_cand_table->vbuf_pi_1st_cand0_en = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand1_en;
	if (ME_cand_table->vbuf_pi_1st_cand1_en == CTRL_NONE) ME_cand_table->vbuf_pi_1st_cand1_en = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand0_en;
	if (ME_cand_table->vbuf_pi_1st_cand2_en == CTRL_NONE) ME_cand_table->vbuf_pi_1st_cand2_en = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand2_en;
	if (ME_cand_table->vbuf_pi_1st_cand3_en == CTRL_NONE) ME_cand_table->vbuf_pi_1st_cand3_en = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand3_en;
	if (ME_cand_table->vbuf_pi_1st_cand4_en == CTRL_NONE) ME_cand_table->vbuf_pi_1st_cand4_en = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand4_en;
	if (ME_cand_table->vbuf_pi_1st_cand5_en == CTRL_NONE) ME_cand_table->vbuf_pi_1st_cand5_en = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand5_en;
	if (ME_cand_table->vbuf_pi_1st_cand6_en == CTRL_NONE) ME_cand_table->vbuf_pi_1st_cand6_en = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand6_en;
	if (ME_cand_table->vbuf_pi_1st_cand7_en == CTRL_NONE) ME_cand_table->vbuf_pi_1st_cand7_en = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand7_en;
	if (ME_cand_table->vbuf_pi_1st_cand8_en == CTRL_NONE) ME_cand_table->vbuf_pi_1st_cand8_en = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand8_en;

	if (ME_cand_table->vbuf_ip_1st_update0_en == CTRL_NONE) ME_cand_table->vbuf_ip_1st_update0_en = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update0_en;
	if (ME_cand_table->vbuf_ip_1st_update1_en == CTRL_NONE) ME_cand_table->vbuf_ip_1st_update1_en = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update1_en;
	if (ME_cand_table->vbuf_ip_1st_update2_en == CTRL_NONE) ME_cand_table->vbuf_ip_1st_update2_en = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update2_en;
	if (ME_cand_table->vbuf_ip_1st_update3_en == CTRL_NONE) ME_cand_table->vbuf_ip_1st_update3_en = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update3_en;
	if (ME_cand_table->vbuf_pi_1st_update0_en == CTRL_NONE) ME_cand_table->vbuf_pi_1st_update0_en = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update0_en;
	if (ME_cand_table->vbuf_pi_1st_update1_en == CTRL_NONE) ME_cand_table->vbuf_pi_1st_update1_en = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update1_en;
	if (ME_cand_table->vbuf_pi_1st_update2_en == CTRL_NONE) ME_cand_table->vbuf_pi_1st_update2_en = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update2_en;
	if (ME_cand_table->vbuf_pi_1st_update3_en == CTRL_NONE) ME_cand_table->vbuf_pi_1st_update3_en = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update3_en;

	if (ME_cand_table->vbuf_ip_1st_zmv_en == CTRL_NONE) ME_cand_table->vbuf_ip_1st_zmv_en = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_zmv_en;
	if (ME_cand_table->vbuf_pi_1st_zmv_en == CTRL_NONE) ME_cand_table->vbuf_pi_1st_zmv_en = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_zmv_en;

	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 	1, 1,	ME_cand_table->vbuf_ip_1st_cand0_en);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 	2, 2,	ME_cand_table->vbuf_ip_1st_cand1_en);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 	3, 3,	ME_cand_table->vbuf_ip_1st_cand2_en);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 	4, 4,	ME_cand_table->vbuf_ip_1st_cand3_en);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 	5, 5,	ME_cand_table->vbuf_ip_1st_cand4_en);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 	6, 6,	ME_cand_table->vbuf_ip_1st_cand5_en);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 	7, 7,	ME_cand_table->vbuf_ip_1st_cand6_en);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 	8, 8,	ME_cand_table->vbuf_ip_1st_cand7_en);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 	9, 9,	ME_cand_table->vbuf_ip_1st_cand8_en);

	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 	0, 0,	ME_cand_table->vbuf_pi_1st_cand0_en);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 	1, 1,	ME_cand_table->vbuf_pi_1st_cand1_en);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 	2, 2,	ME_cand_table->vbuf_pi_1st_cand2_en);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 	3, 3,	ME_cand_table->vbuf_pi_1st_cand3_en);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 	4, 4,	ME_cand_table->vbuf_pi_1st_cand4_en);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 	5, 5,	ME_cand_table->vbuf_pi_1st_cand5_en);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 	6, 6,	ME_cand_table->vbuf_pi_1st_cand6_en);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 	7, 7,	ME_cand_table->vbuf_pi_1st_cand7_en);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 	8, 8,	ME_cand_table->vbuf_pi_1st_cand8_en);

	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,  11, 11,  ME_cand_table->vbuf_ip_1st_update0_en);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,  12, 12,  ME_cand_table->vbuf_ip_1st_update1_en);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,  13, 13,  ME_cand_table->vbuf_ip_1st_update2_en);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,  14, 14,  ME_cand_table->vbuf_ip_1st_update3_en);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,  10, 10,  ME_cand_table->vbuf_pi_1st_update0_en);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,  11, 11,  ME_cand_table->vbuf_pi_1st_update1_en);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,  12, 12,  ME_cand_table->vbuf_pi_1st_update2_en);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,  13, 13,  ME_cand_table->vbuf_pi_1st_update3_en);

	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,  16, 16,  ME_cand_table->vbuf_ip_1st_zmv_en);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,  15, 15,  ME_cand_table->vbuf_pi_1st_zmv_en);
}

VOID ME_cand_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
 	const _PQLCONTEXT *s_pContext = GetPQLContext();
	_PQLPARAMETER *s_pParam       = GetPQLParameter();
	unsigned char table_level = 255, u8_i;/* 255->load defaule */
	_str_ME_cand_table *tmp_ME_cand_table = NULL;
	unsigned int u32_ID_Log_en=0;//,ES_Test_Value=0;
	unsigned int u1_ForceCand = 0, u1_Cand_en[26];
	ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 28,  28, &u32_ID_Log_en);//0xB809D758
	ReadRegister(SOFTWARE3_SOFTWARE3_58_reg, 31,  31, &u1_ForceCand);//0xB809D9E8

	if(pParam->u1_MECand_wrt_en == 0)//d5e4[15]
		return;

#if 1
	if(u1_ForceCand) {//D9E8[31]
		for(u8_i=0;u8_i<26;u8_i++){
			ReadRegister(SOFTWARE3_SOFTWARE3_58_reg, u8_i, u8_i, &u1_Cand_en[u8_i]);
		}
		
		tmp_ME_cand_table = fwif_MEMC_get_ME_cand_table(0);
		tmp_ME_cand_table->vbuf_ip_1st_cand0_en = u1_Cand_en[0]; //0xB809D9E8[0]
		tmp_ME_cand_table->vbuf_ip_1st_cand1_en = u1_Cand_en[1];
		tmp_ME_cand_table->vbuf_ip_1st_cand2_en = u1_Cand_en[2];
		tmp_ME_cand_table->vbuf_ip_1st_cand3_en = u1_Cand_en[3];
		tmp_ME_cand_table->vbuf_ip_1st_cand4_en = u1_Cand_en[4];
		tmp_ME_cand_table->vbuf_ip_1st_cand5_en = u1_Cand_en[5];
		tmp_ME_cand_table->vbuf_ip_1st_cand6_en = u1_Cand_en[6];
		tmp_ME_cand_table->vbuf_ip_1st_cand7_en = u1_Cand_en[7];
		tmp_ME_cand_table->vbuf_ip_1st_cand8_en = u1_Cand_en[8];

		tmp_ME_cand_table->vbuf_pi_1st_cand0_en = u1_Cand_en[9]; //0xB809D9E8[9]
		tmp_ME_cand_table->vbuf_pi_1st_cand1_en = u1_Cand_en[10];
		tmp_ME_cand_table->vbuf_pi_1st_cand2_en = u1_Cand_en[11];
		tmp_ME_cand_table->vbuf_pi_1st_cand3_en = u1_Cand_en[12];
		tmp_ME_cand_table->vbuf_pi_1st_cand4_en = u1_Cand_en[13];
		tmp_ME_cand_table->vbuf_pi_1st_cand5_en = u1_Cand_en[14];
		tmp_ME_cand_table->vbuf_pi_1st_cand6_en = u1_Cand_en[15];
		tmp_ME_cand_table->vbuf_pi_1st_cand7_en = u1_Cand_en[16];
		tmp_ME_cand_table->vbuf_pi_1st_cand8_en = u1_Cand_en[17];

		tmp_ME_cand_table->vbuf_ip_1st_update1_en = u1_Cand_en[18]; //0xB809D9E8[18]
		tmp_ME_cand_table->vbuf_ip_1st_update2_en = u1_Cand_en[19];
		tmp_ME_cand_table->vbuf_ip_1st_update0_en = u1_Cand_en[20];
		tmp_ME_cand_table->vbuf_pi_1st_update1_en = u1_Cand_en[21];
		tmp_ME_cand_table->vbuf_pi_1st_update2_en = u1_Cand_en[22];
		tmp_ME_cand_table->vbuf_pi_1st_update0_en = u1_Cand_en[23];

		tmp_ME_cand_table->vbuf_ip_1st_zmv_en = u1_Cand_en[24]; //0xB809D9E8[24]
		tmp_ME_cand_table->vbuf_pi_1st_zmv_en = u1_Cand_en[25];
	}
	else if( (s_pParam->_param_read_comreg.u1_powerSaving_en && s_pContext->_external_data.u1_powerSaving == 1) ||( s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type==3) )
	{
		table_level = 1;
		tmp_ME_cand_table = fwif_MEMC_get_ME_cand_table(table_level); // disable ip/pi update0/1/2 by table, others the same
	}
	else if(s_pContext->_output_me_sceneAnalysis.u1_SportScene) {
		table_level = 9;
		tmp_ME_cand_table = fwif_MEMC_get_ME_cand_table(table_level);
	}
	else if(s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type==1)
	{
	 	table_level = 2;
		tmp_ME_cand_table = fwif_MEMC_get_ME_cand_table(table_level); // disable ip/pi update0/2 by table, others the same
	}
	else if(s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type==2)
	{
	 	table_level = 3;
		tmp_ME_cand_table = fwif_MEMC_get_ME_cand_table(table_level); // disable ip/pi update1/2 by table, others the same
	}
#if 0
	else if( (s_pContext->_output_wrt_comreg.u1_casino_RP_detect_true == 1) && (s_pContext->_output_me_sceneAnalysis.b_do_ME_medFLT != 1) )
	{
		table_level = 5;
		tmp_ME_cand_table = fwif_MEMC_get_ME_cand_table(table_level);
	}
#endif	
	else if(s_pContext->_output_me_sceneAnalysis.u1_zmv_on_flag == 1 )
	{
		table_level = 6;
		tmp_ME_cand_table = fwif_MEMC_get_ME_cand_table(table_level); // turn on ip ZMV_en
	}
	//else if(s_pContext->_output_me_sceneAnalysis.u2_strong_pure_panning_flag == 1){
	//	table_level = 7;
	//	tmp_ME_cand_table = fwif_MEMC_get_ME_cand_table(table_level);
	//}
	else if(s_pParam->_param_me_sceneAnalysis.b_FixRepeat_en && s_pContext->_output_me_sceneAnalysis.u8_RepeatMode!=RepeatMode_OFF)
	{
		if(s_pContext->_output_me_sceneAnalysis.u1_casino_RP_detect_true) {
			table_level = 8;
		}
		else if(s_pContext->_output_me_sceneAnalysis.u8_RepeatMode==RepeatMode_HIGH) {
			table_level = 8;
		}
		else {//LOW/MID
			table_level = 0;
		}
		tmp_ME_cand_table = fwif_MEMC_get_ME_cand_table(table_level);
	}
#if IC_K4LP
	else if(s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true == 1)
	{
		table_level = 4;
		tmp_ME_cand_table = fwif_MEMC_get_ME_cand_table(table_level); // disable ip/pi cand3/4 by table, others the same
	}
#endif
	else
	{
		table_level = 0;
		tmp_ME_cand_table = fwif_MEMC_get_ME_cand_table(table_level);
	}

	if(u32_ID_Log_en==1)//0xB809D758 [28]
	{
		static unsigned char counter = 0;
		if(counter % 10 == 0)
			rtd_printk(KERN_EMERG, TAG_NAME, "[%s][%d][Cand En] table_level=%d\n",__FUNCTION__, __LINE__,table_level);
		counter++;
	}

	ME_cand_wrtRegister(tmp_ME_cand_table);

#else
	 if(s_pParam->_param_read_comreg.u1_powerSaving_en && s_pContext->_external_data.u1_powerSaving == 1 )
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 12, 12, 0x00);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 13, 13,0x00);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 11, 11,0x00);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 11, 11, 0x00);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 12, 12,0x00);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 10, 10, 0x00);
	}
	 else if(s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type==1)
	 {
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 13, 13,0x00);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 11, 11,0x00);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 12, 12,0x00);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 10, 10, 0x00);
	}
	 else if(s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type==2)
	 {
	 	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 12, 12, 0x00);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 13, 13,0x00);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 11, 11, 0x00);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 12, 12,0x00);
	}
	else if(s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type==3)
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 12, 12, 0x00);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 13, 13,0x00);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 11, 11,0x00);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 11, 11, 0x00);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 12, 12,0x00);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 10, 10, 0x00);
	}
#if IC_K4LP
	else if(s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true == 1 )
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4,0x00);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5,0x00);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3,0x00);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4,0x00);
	}
#endif
	else
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4,s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand3_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5,s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand4_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3,s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand3_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4,s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand4_en);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 12, 12, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update1_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 13, 13, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update2_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 11, 11, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update0_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 11, 11, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update1_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 12, 12, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update2_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 10, 10, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update0_en);
	}
#endif
}


static _str_ME_candOfst_table  ME_candOfst_table[3]={
	/*0.load default*/
	{CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,
	 CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,
 	 CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,
 	 CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE},
	/*1*/
	{CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,0x6,CTRL_NONE,0x6,CTRL_NONE,0x6,CTRL_NONE,0x6,CTRL_NONE,0x6,
	 CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,0x6,CTRL_NONE,0x6,CTRL_NONE,0x6,CTRL_NONE,0x6,CTRL_NONE,0x6,
	 CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,
	 CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE},
	/*2 small object */
	{CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,
	 CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,
	 7,7, 7,1, 1,7, 1,1,
	 7,7, 7,1, 1,7, 1,1},
};

_str_ME_candOfst_table *fwif_MEMC_get_ME_candOfst_table(unsigned char level)
{
//	if (ME_candOfst_table == NULL) {
//		return NULL;
//	}
//	if( level==255 || level==0)
	if(level > 2)
		return (_str_ME_candOfst_table *)&ME_candOfst_table[0];
	else
		return (_str_ME_candOfst_table *)&ME_candOfst_table[level];
}

VOID  ME_candOfst_wrtRegister(_str_ME_candOfst_table *ME_candOfst_table)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	if (ME_candOfst_table->vbuf_ip_1st_cand0_offsetx == CTRL_NONE) ME_candOfst_table->vbuf_ip_1st_cand0_offsetx = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand0_offsetx;
	if (ME_candOfst_table->vbuf_ip_1st_cand0_offsety == CTRL_NONE) ME_candOfst_table->vbuf_ip_1st_cand0_offsety = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand0_offsety;
	if (ME_candOfst_table->vbuf_ip_1st_cand1_offsetx == CTRL_NONE) ME_candOfst_table->vbuf_ip_1st_cand1_offsetx = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand1_offsetx;
	if (ME_candOfst_table->vbuf_ip_1st_cand1_offsety == CTRL_NONE) ME_candOfst_table->vbuf_ip_1st_cand1_offsety = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand1_offsety;
	if (ME_candOfst_table->vbuf_ip_1st_cand2_offsetx == CTRL_NONE) ME_candOfst_table->vbuf_ip_1st_cand2_offsetx = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand2_offsetx;
	if (ME_candOfst_table->vbuf_ip_1st_cand2_offsety == CTRL_NONE) ME_candOfst_table->vbuf_ip_1st_cand2_offsety = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand2_offsety;
	if (ME_candOfst_table->vbuf_ip_1st_cand3_offsetx == CTRL_NONE) ME_candOfst_table->vbuf_ip_1st_cand3_offsetx = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand3_offsetx;
	if (ME_candOfst_table->vbuf_ip_1st_cand3_offsety == CTRL_NONE) ME_candOfst_table->vbuf_ip_1st_cand3_offsety = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand3_offsety;
	if (ME_candOfst_table->vbuf_ip_1st_cand4_offsetx == CTRL_NONE) ME_candOfst_table->vbuf_ip_1st_cand4_offsetx = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand4_offsetx;
	if (ME_candOfst_table->vbuf_ip_1st_cand4_offsety == CTRL_NONE) ME_candOfst_table->vbuf_ip_1st_cand4_offsety = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand4_offsety;
	if (ME_candOfst_table->vbuf_ip_1st_cand5_offsetx == CTRL_NONE) ME_candOfst_table->vbuf_ip_1st_cand5_offsetx = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand5_offsetx;
	if (ME_candOfst_table->vbuf_ip_1st_cand5_offsety == CTRL_NONE) ME_candOfst_table->vbuf_ip_1st_cand5_offsety = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand5_offsety;
	if (ME_candOfst_table->vbuf_ip_1st_cand6_offsetx == CTRL_NONE) ME_candOfst_table->vbuf_ip_1st_cand6_offsetx = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand6_offsetx;
	if (ME_candOfst_table->vbuf_ip_1st_cand6_offsety == CTRL_NONE) ME_candOfst_table->vbuf_ip_1st_cand6_offsety = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand6_offsety;
	if (ME_candOfst_table->vbuf_ip_1st_cand7_offsetx == CTRL_NONE) ME_candOfst_table->vbuf_ip_1st_cand7_offsetx = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand7_offsetx;
	if (ME_candOfst_table->vbuf_ip_1st_cand7_offsety == CTRL_NONE) ME_candOfst_table->vbuf_ip_1st_cand7_offsety = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand7_offsety;

	if (ME_candOfst_table->vbuf_ip_1st_update0_offsetx == CTRL_NONE) ME_candOfst_table->vbuf_ip_1st_update0_offsetx = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update0_offsetx;
	if (ME_candOfst_table->vbuf_ip_1st_update0_offsety == CTRL_NONE) ME_candOfst_table->vbuf_ip_1st_update0_offsety = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update0_offsety;
	if (ME_candOfst_table->vbuf_ip_1st_update1_offsetx == CTRL_NONE) ME_candOfst_table->vbuf_ip_1st_update1_offsetx = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update1_offsetx;
	if (ME_candOfst_table->vbuf_ip_1st_update1_offsety == CTRL_NONE) ME_candOfst_table->vbuf_ip_1st_update1_offsety = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update1_offsety;
	if (ME_candOfst_table->vbuf_ip_1st_update2_offsetx == CTRL_NONE) ME_candOfst_table->vbuf_ip_1st_update2_offsetx = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update2_offsetx;
	if (ME_candOfst_table->vbuf_ip_1st_update2_offsety == CTRL_NONE) ME_candOfst_table->vbuf_ip_1st_update2_offsety = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update2_offsety;
	if (ME_candOfst_table->vbuf_ip_1st_update3_offsetx == CTRL_NONE) ME_candOfst_table->vbuf_ip_1st_update3_offsetx = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update3_offsetx;
	if (ME_candOfst_table->vbuf_ip_1st_update3_offsety == CTRL_NONE) ME_candOfst_table->vbuf_ip_1st_update3_offsety = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update3_offsety;

	if (ME_candOfst_table->vbuf_pi_1st_cand0_offsetx == CTRL_NONE) ME_candOfst_table->vbuf_pi_1st_cand0_offsetx = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand0_offsetx;
	if (ME_candOfst_table->vbuf_pi_1st_cand0_offsety == CTRL_NONE) ME_candOfst_table->vbuf_pi_1st_cand0_offsety = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand0_offsety;
	if (ME_candOfst_table->vbuf_pi_1st_cand1_offsetx == CTRL_NONE) ME_candOfst_table->vbuf_pi_1st_cand1_offsetx = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand1_offsetx;
	if (ME_candOfst_table->vbuf_pi_1st_cand1_offsety == CTRL_NONE) ME_candOfst_table->vbuf_pi_1st_cand1_offsety = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand1_offsety;
	if (ME_candOfst_table->vbuf_pi_1st_cand2_offsetx == CTRL_NONE) ME_candOfst_table->vbuf_pi_1st_cand2_offsetx = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand2_offsetx;
	if (ME_candOfst_table->vbuf_pi_1st_cand2_offsety == CTRL_NONE) ME_candOfst_table->vbuf_pi_1st_cand2_offsety = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand2_offsety;
	if (ME_candOfst_table->vbuf_pi_1st_cand3_offsetx == CTRL_NONE) ME_candOfst_table->vbuf_pi_1st_cand3_offsetx = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand3_offsetx;
	if (ME_candOfst_table->vbuf_pi_1st_cand3_offsety == CTRL_NONE) ME_candOfst_table->vbuf_pi_1st_cand3_offsety = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand3_offsety;
	if (ME_candOfst_table->vbuf_pi_1st_cand4_offsetx == CTRL_NONE) ME_candOfst_table->vbuf_pi_1st_cand4_offsetx = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand4_offsetx;
	if (ME_candOfst_table->vbuf_pi_1st_cand4_offsety == CTRL_NONE) ME_candOfst_table->vbuf_pi_1st_cand4_offsety = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand4_offsety;
	if (ME_candOfst_table->vbuf_pi_1st_cand5_offsetx == CTRL_NONE) ME_candOfst_table->vbuf_pi_1st_cand5_offsetx = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand5_offsetx;
	if (ME_candOfst_table->vbuf_pi_1st_cand5_offsety == CTRL_NONE) ME_candOfst_table->vbuf_pi_1st_cand5_offsety = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand5_offsety;
	if (ME_candOfst_table->vbuf_pi_1st_cand6_offsetx == CTRL_NONE) ME_candOfst_table->vbuf_pi_1st_cand6_offsetx = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand6_offsetx;
	if (ME_candOfst_table->vbuf_pi_1st_cand6_offsety == CTRL_NONE) ME_candOfst_table->vbuf_pi_1st_cand6_offsety = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand6_offsety;
	if (ME_candOfst_table->vbuf_pi_1st_cand7_offsetx == CTRL_NONE) ME_candOfst_table->vbuf_pi_1st_cand7_offsetx = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand7_offsetx;
	if (ME_candOfst_table->vbuf_pi_1st_cand7_offsety == CTRL_NONE) ME_candOfst_table->vbuf_pi_1st_cand7_offsety = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand7_offsety;

	if (ME_candOfst_table->vbuf_pi_1st_update0_offsetx == CTRL_NONE) ME_candOfst_table->vbuf_pi_1st_update0_offsetx = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update0_offsetx;
	if (ME_candOfst_table->vbuf_pi_1st_update0_offsety == CTRL_NONE) ME_candOfst_table->vbuf_pi_1st_update0_offsety = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update0_offsety;
	if (ME_candOfst_table->vbuf_pi_1st_update1_offsetx == CTRL_NONE) ME_candOfst_table->vbuf_pi_1st_update1_offsetx = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update1_offsetx;
	if (ME_candOfst_table->vbuf_pi_1st_update1_offsety == CTRL_NONE) ME_candOfst_table->vbuf_pi_1st_update1_offsety = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update1_offsety;
	if (ME_candOfst_table->vbuf_pi_1st_update2_offsetx == CTRL_NONE) ME_candOfst_table->vbuf_pi_1st_update2_offsetx = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update2_offsetx;
	if (ME_candOfst_table->vbuf_pi_1st_update2_offsety == CTRL_NONE) ME_candOfst_table->vbuf_pi_1st_update2_offsety = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update2_offsety;
	if (ME_candOfst_table->vbuf_pi_1st_update3_offsetx == CTRL_NONE) ME_candOfst_table->vbuf_pi_1st_update3_offsetx = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update3_offsetx;
	if (ME_candOfst_table->vbuf_pi_1st_update3_offsety == CTRL_NONE) ME_candOfst_table->vbuf_pi_1st_update3_offsety = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update3_offsety;

	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg,  0,  2, 	ME_candOfst_table->vbuf_ip_1st_cand0_offsetx);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg,  3,  5, 	ME_candOfst_table->vbuf_ip_1st_cand0_offsety);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg,  6,  8, 	ME_candOfst_table->vbuf_ip_1st_cand1_offsetx);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg,  9, 11, 	ME_candOfst_table->vbuf_ip_1st_cand1_offsety);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg, 12, 14, 	ME_candOfst_table->vbuf_ip_1st_cand2_offsetx);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg, 15, 17, 	ME_candOfst_table->vbuf_ip_1st_cand2_offsety);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg, 18, 20, 	ME_candOfst_table->vbuf_ip_1st_cand3_offsetx);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg, 21, 23, 	ME_candOfst_table->vbuf_ip_1st_cand3_offsety);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg, 24, 26, 	ME_candOfst_table->vbuf_ip_1st_cand4_offsetx);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg, 27, 29, 	ME_candOfst_table->vbuf_ip_1st_cand4_offsety);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg,  0,  2, 	ME_candOfst_table->vbuf_ip_1st_cand5_offsetx);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg,  3,  5, 	ME_candOfst_table->vbuf_ip_1st_cand5_offsety);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg,  6,  8, 	ME_candOfst_table->vbuf_ip_1st_cand6_offsetx);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg,  9, 11, 	ME_candOfst_table->vbuf_ip_1st_cand6_offsety);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg, 12, 14, 	ME_candOfst_table->vbuf_ip_1st_cand7_offsetx);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg, 15, 17, 	ME_candOfst_table->vbuf_ip_1st_cand7_offsety);

	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg,  0,  2, 	ME_candOfst_table->vbuf_ip_1st_update0_offsetx);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg,  3,  5, 	ME_candOfst_table->vbuf_ip_1st_update0_offsety);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg,  6,  8, 	ME_candOfst_table->vbuf_ip_1st_update1_offsetx);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg,  9, 11, 	ME_candOfst_table->vbuf_ip_1st_update1_offsety);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg, 12, 14, 	ME_candOfst_table->vbuf_ip_1st_update2_offsetx);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg, 15, 17, 	ME_candOfst_table->vbuf_ip_1st_update2_offsety);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg, 18, 20, 	ME_candOfst_table->vbuf_ip_1st_update3_offsetx);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg, 21, 23, 	ME_candOfst_table->vbuf_ip_1st_update3_offsety);

	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg,  0,  2, 	ME_candOfst_table->vbuf_pi_1st_cand0_offsetx);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg,  3,  5, 	ME_candOfst_table->vbuf_pi_1st_cand0_offsety);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg,  6,  8, 	ME_candOfst_table->vbuf_pi_1st_cand1_offsetx);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg,  9, 11, 	ME_candOfst_table->vbuf_pi_1st_cand1_offsety);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg, 12, 14, 	ME_candOfst_table->vbuf_pi_1st_cand2_offsetx);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg, 15, 17, 	ME_candOfst_table->vbuf_pi_1st_cand2_offsety);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg, 18, 20, 	ME_candOfst_table->vbuf_pi_1st_cand3_offsetx);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg, 21, 23, 	ME_candOfst_table->vbuf_pi_1st_cand3_offsety);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg, 24, 26, 	ME_candOfst_table->vbuf_pi_1st_cand4_offsetx);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg, 27, 29, 	ME_candOfst_table->vbuf_pi_1st_cand4_offsety);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg,  0,  2, 	ME_candOfst_table->vbuf_pi_1st_cand5_offsetx);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg,  3,  5, 	ME_candOfst_table->vbuf_pi_1st_cand5_offsety);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg,  6,  8, 	ME_candOfst_table->vbuf_pi_1st_cand6_offsetx);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg,  9, 11, 	ME_candOfst_table->vbuf_pi_1st_cand6_offsety);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg, 12, 14, 	ME_candOfst_table->vbuf_pi_1st_cand7_offsetx);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg, 15, 17, 	ME_candOfst_table->vbuf_pi_1st_cand7_offsety);

	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg,  0,  2, 	ME_candOfst_table->vbuf_pi_1st_update0_offsetx);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg,  3,  5, 	ME_candOfst_table->vbuf_pi_1st_update0_offsety);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg,  6,  8, 	ME_candOfst_table->vbuf_pi_1st_update1_offsetx);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg,  9, 11, 	ME_candOfst_table->vbuf_pi_1st_update1_offsety);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg, 12, 14, 	ME_candOfst_table->vbuf_pi_1st_update2_offsetx);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg, 15, 17, 	ME_candOfst_table->vbuf_pi_1st_update2_offsety);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg, 18, 20, 	ME_candOfst_table->vbuf_pi_1st_update3_offsetx);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg, 21, 23, 	ME_candOfst_table->vbuf_pi_1st_update3_offsety);
}

VOID ME_candOfst_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned int u25_me_aDTL = s_pContext->_output_read_comreg.u25_me_aDTL_rb;
	signed short u11_gmv_mvx = (s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb)>>1;
	signed short u10_gmv_mvy = (s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb)>>1;
	signed short u11_2nd_gmv_mvx = (s_pContext->_output_read_comreg.s11_me_GMV_2nd_vx_rb);
	//signed short u10_2nd_gmv_mvy = (s_pContext->_output_read_comreg.s10_me_GMV_2nd_vy_rb);
	unsigned int u32_gmv_cnt = s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb;
	unsigned int u32_gmv_unconf = s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb;
	unsigned int u26_aAPLi_rb = s_pContext->_output_read_comreg.u26_me_aAPLi_rb;
	static unsigned int  u25_me_aDTL_pre;
	unsigned int log_en;
	unsigned char table_level = 255;/* 255->load defaule */
	_str_ME_candOfst_table *tmp_ME_candOfst_table = NULL;

	ReadRegister(SOFTWARE3_SOFTWARE3_59_reg, 24,  24, &log_en);//0xB809D9EC

	if(pParam->u1_MECandOfst_wrt_en == 0)//d5e4[13]
		return;
	u25_me_aDTL = (u25_me_aDTL_pre*7 + u25_me_aDTL)>>3;

	if(s_pContext->_output_me_sceneAnalysis.u1_SportScene == true) {
		table_level = 2;
		tmp_ME_candOfst_table = fwif_MEMC_get_ME_candOfst_table(table_level);	
	}
	else if( (s_pContext->_output_me_sceneAnalysis.u1_pure_TopDownPan_flag != 0) && (s_pContext->_output_me_sceneAnalysis.u2_panning_flag != 0)&&(u25_me_aDTL > 2800000))
	{
		table_level = 255;
		tmp_ME_candOfst_table = fwif_MEMC_get_ME_candOfst_table(table_level);
	}
	else if((s_pContext->_output_FRC_LgDet.u1_logo_KeyRgn_status!=1)&&(u32_gmv_cnt>=25000)&&(s_pContext->_output_me_sceneAnalysis.u2_panning_flag == 1)&&( (u32_gmv_unconf<=3) && (_ABS_(u10_gmv_mvy)<=2) )
			&&( (u25_me_aDTL > 770000) && (u25_me_aDTL < 1500000))&&( (u26_aAPLi_rb > 3500000) && (u26_aAPLi_rb < 5200000))
			&&(s_pContext->_output_FRC_LgDet.u1_logo_PurePanning_status == 2 ))
	{
		table_level = 1;
		tmp_ME_candOfst_table = fwif_MEMC_get_ME_candOfst_table(table_level);		
	}
	else if((s_pContext->_output_me_sceneAnalysis.u2_panning_flag == 1)&&(u32_gmv_cnt>=20000)
			&&(_ABS_(u11_gmv_mvx)<=2) && (u10_gmv_mvy<=55 && u10_gmv_mvy>=5)&& (_ABS_(u11_2nd_gmv_mvx)<=5)&&(u32_gmv_unconf<=10)
			&&( (u25_me_aDTL > 1000000) && (u25_me_aDTL < 1600000)) &&( (u26_aAPLi_rb > 4000000) && (u26_aAPLi_rb < 5200000)))
	{
		table_level = 1;
		tmp_ME_candOfst_table = fwif_MEMC_get_ME_candOfst_table(table_level);		
	}
	else
	{
		table_level = 255;
		tmp_ME_candOfst_table = fwif_MEMC_get_ME_candOfst_table(table_level);
	}
	u25_me_aDTL_pre = u25_me_aDTL;

	ME_candOfst_wrtRegister(tmp_ME_candOfst_table);

	if(log_en){//D9EC[24]
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s][%d][cand offset] table_level=%d\n",__FUNCTION__, __LINE__,table_level);
	}
}

static _str_ME_caddpnt_table  ME_caddpnt_table[]={
	/*0.load default*/
	{CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,
	 CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,
	 CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,
	 CTRL_N,CTRL_N,CTRL_N,CTRL_N},	
	/*1*/
	{0xFF,0xFF,0xFF,CTRL_N,0xFF,0xFF,0xFF,CTRL_N,
	 CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,
	 CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,
	 CTRL_N,CTRL_N,CTRL_N,CTRL_N},	
	/*2*/
	{0x23,0x23,0x23,CTRL_N,0x23,0x23,0x23,CTRL_N,
	 CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,
	 CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,
 	 CTRL_N,CTRL_N,CTRL_N,CTRL_N},	
	/*3*/
	{0x0,0x0,0x0,CTRL_N,0x0,0x0,0x0,CTRL_N,
	 CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,
	 CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,
	 CTRL_N,CTRL_N,CTRL_N,CTRL_N},	
	/*4*/
	{0xFF,0xFF,0xFF,CTRL_N,0xFF,0xFF,0xFF,CTRL_N,
	 CTRL_N,CTRL_N,CTRL_N,0x90,0x90,0x60,0x60,CTRL_N,
	 CTRL_N,CTRL_N,CTRL_N,0x90,0x90,0x60,0x60,CTRL_N,
	 CTRL_N,CTRL_N,CTRL_N,CTRL_N},	
	/*5*/
	{0xC0,0xC0,0xC0,CTRL_N,0xC0,0xC0,0xC0,CTRL_N,
	 CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,
	 CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,
	 CTRL_N,CTRL_N,CTRL_N,CTRL_N},	
	/*6*/
	{CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,
	 CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,
	 CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,
	 32,CTRL_N,CTRL_N,CTRL_N},
	/*7*/ //repeat LOW
	{0xA0,0xA0,0xA0,0xA0,0xA0,0xA0,0xA0,0xA0,
	 28,36,29,40,40,40,40,48,
	 28,36,29,40,40,48,48,48,
	 CTRL_N,CTRL_N,200,200},
	/*8*/ //repeat MID
	{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	 0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	 0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	 CTRL_N,CTRL_N,0,0},
	/*9*/ //repeat HIGH, casino
	{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	 CTRL_N,CTRL_N,0,0},	
	/*10*/ //case:small object
	{64,64,64,64,64,64,64,64,
	 28,36,29,40,40,40,40,48,
	 28,36,29,40,40,48,48,48,
	 CTRL_N,CTRL_N,1000,1000},
};

_str_ME_caddpnt_table *fwif_MEMC_get_ME_caddpnt_table(unsigned char level)
{
//	if (ME_caddpnt_table == NULL) {
//		return NULL;
//	}
//	if( level==255 || level==0)
	if(level > sizeof(ME_caddpnt_table)/sizeof(_str_ME_caddpnt_table))
		return (_str_ME_caddpnt_table *)&ME_caddpnt_table[0];
	else
		return (_str_ME_caddpnt_table *)&ME_caddpnt_table[level];

}

VOID  ME_caddpnt_wrtRegister(_str_ME_caddpnt_table *ME_caddpnt_table)
{
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_24_reg, 16, 23, 	ME_caddpnt_table->me1_ip_cddpnt_rnd0);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_24_reg, 24, 31, 	ME_caddpnt_table->me1_ip_cddpnt_rnd1);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_28_reg, 0, 7, 		ME_caddpnt_table->me1_ip_cddpnt_rnd2);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_28_reg, 8, 15, 		ME_caddpnt_table->me1_ip_cddpnt_rnd3);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_64_reg, 16, 23, 	ME_caddpnt_table->me1_pi_cddpnt_rnd0);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_64_reg, 24, 31, 	ME_caddpnt_table->me1_pi_cddpnt_rnd1);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 0, 7, 		ME_caddpnt_table->me1_pi_cddpnt_rnd2);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 8, 15, 		ME_caddpnt_table->me1_pi_cddpnt_rnd3);

	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg, 0, 7, 		ME_caddpnt_table->me1_ip_cddpnt_st0);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg, 8, 15, 		ME_caddpnt_table->me1_ip_cddpnt_st1);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg, 16, 23, 	ME_caddpnt_table->me1_ip_cddpnt_st2);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg, 24, 31, 	ME_caddpnt_table->me1_ip_cddpnt_st3);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg, 0, 7, 		ME_caddpnt_table->me1_ip_cddpnt_st4);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg, 8, 15, 		ME_caddpnt_table->me1_ip_cddpnt_st5);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg, 16, 23, 	ME_caddpnt_table->me1_ip_cddpnt_st6);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg, 24, 31, 	ME_caddpnt_table->me1_ip_cddpnt_st7);

	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg, 0, 7, 		ME_caddpnt_table->me1_pi_cddpnt_st0);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg, 8, 15, 		ME_caddpnt_table->me1_pi_cddpnt_st1);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg, 16, 23, 	ME_caddpnt_table->me1_pi_cddpnt_st2);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg, 24, 31, 	ME_caddpnt_table->me1_pi_cddpnt_st3);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg, 0, 7, 		ME_caddpnt_table->me1_pi_cddpnt_st4);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg, 8, 15, 		ME_caddpnt_table->me1_pi_cddpnt_st5);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg, 16, 23, 	ME_caddpnt_table->me1_pi_cddpnt_st6);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg, 24, 31, 	ME_caddpnt_table->me1_pi_cddpnt_st7);
		
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_28_reg, 16, 28, 	ME_caddpnt_table->me1_ip_cddpnt_zmv);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 16, 28, 	ME_caddpnt_table->me1_pi_cddpnt_zmv);

	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_2C_reg, 0, 12, 		ME_caddpnt_table->me1_ip_cddpnt_gmv);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_6C_reg, 0, 12, 		ME_caddpnt_table->me1_pi_cddpnt_gmv);
}

VOID ME_caddpnt_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
 	const _PQLCONTEXT *s_pContext = GetPQLContext();
	_PQLPARAMETER *s_pParam       = GetPQLParameter();
	unsigned char table_level = 255;/* 255->load defaule */
	_str_ME_caddpnt_table *tmp_ME_caddpnt_table = NULL;
	static unsigned int u32_ID_Log_en=0;//,ES_Test_Value=0;
	unsigned int u1_ForceCaddpnt, u8_ForceSpePntVal, u8_ForceTmpPntVal, u8_ForceRndPntVal, u8_ForceGmvPntVal, u8_ForceZmvPntVal;
	ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 28,  28, &u32_ID_Log_en);//0xB809D758 [28]
	ReadRegister(SOFTWARE3_SOFTWARE3_60_reg, 31,  31, &u1_ForceCaddpnt);//0xB809D9F0[31]

	if(pParam->u1_MECandPnt_wrt_en == 0)//d5e4[16]
		return;

	if(u1_ForceCaddpnt) {
		ReadRegister(SOFTWARE3_SOFTWARE3_60_reg,  0,   7, &u8_ForceSpePntVal);//0xB809D9F0
		ReadRegister(SOFTWARE3_SOFTWARE3_60_reg,  8,  15, &u8_ForceTmpPntVal);
		ReadRegister(SOFTWARE3_SOFTWARE3_60_reg, 16,  23, &u8_ForceRndPntVal);
		ReadRegister(SOFTWARE3_SOFTWARE3_61_reg,  0,  12, &u8_ForceGmvPntVal);//0xB809D9F4
		ReadRegister(SOFTWARE3_SOFTWARE3_61_reg, 16,  28, &u8_ForceZmvPntVal);
		
		tmp_ME_caddpnt_table = fwif_MEMC_get_ME_caddpnt_table(0);

		tmp_ME_caddpnt_table->me1_ip_cddpnt_rnd0 = u8_ForceRndPntVal;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_rnd1 = u8_ForceRndPntVal;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_rnd2 = u8_ForceRndPntVal;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_rnd3 = u8_ForceRndPntVal;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_rnd0 = u8_ForceRndPntVal;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_rnd1 = u8_ForceRndPntVal;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_rnd2 = u8_ForceRndPntVal;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_rnd3 = u8_ForceRndPntVal;

		tmp_ME_caddpnt_table->me1_ip_cddpnt_st0 = u8_ForceSpePntVal;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st1 = u8_ForceSpePntVal;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st2 = u8_ForceSpePntVal;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st3 = u8_ForceSpePntVal;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st4 = u8_ForceTmpPntVal;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st5 = u8_ForceTmpPntVal;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st6 = u8_ForceTmpPntVal;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st7 = u8_ForceTmpPntVal;

		tmp_ME_caddpnt_table->me1_pi_cddpnt_st0 = u8_ForceSpePntVal;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st1 = u8_ForceSpePntVal;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st2 = u8_ForceSpePntVal;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st3 = u8_ForceSpePntVal;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st4 = u8_ForceTmpPntVal;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st5 = u8_ForceTmpPntVal;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st6 = u8_ForceTmpPntVal;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st7 = u8_ForceTmpPntVal;

		tmp_ME_caddpnt_table->me1_ip_cddpnt_zmv = u8_ForceZmvPntVal;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_zmv = u8_ForceZmvPntVal;

		tmp_ME_caddpnt_table->me1_ip_cddpnt_gmv = u8_ForceGmvPntVal;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_gmv = u8_ForceGmvPntVal;
	}
	else if(s_pContext->_output_me_sceneAnalysis.u1_SportScene)
	{
		table_level = 10;
		tmp_ME_caddpnt_table = fwif_MEMC_get_ME_caddpnt_table(table_level);
	}
	else if(s_pContext->_output_me_sceneAnalysis.u1_Avenger_RP_detect_true == 1 )
	{
		table_level = 1;
		tmp_ME_caddpnt_table = fwif_MEMC_get_ME_caddpnt_table(table_level);

		tmp_ME_caddpnt_table->me1_ip_cddpnt_st0 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st0;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st1 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st1;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st2 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st2;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st3;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st4 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st4;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st5 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st5;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st6 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st6;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st7 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st7;
		
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st0 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st0;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st1 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st1;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st2 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st2;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st3;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st4 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st4;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st5 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st5;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st6 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st6;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st7 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st7;

		tmp_ME_caddpnt_table->me1_ip_cddpnt_rnd3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_rnd3;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_rnd3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_rnd3;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_gmv = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_gmv;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_gmv = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_gmv;
		
		tmp_ME_caddpnt_table->me1_ip_cddpnt_zmv = 255;
	}
	else if(s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Fast_Convergence_type ==1)
	{
		table_level = 2;
		tmp_ME_caddpnt_table = fwif_MEMC_get_ME_caddpnt_table(table_level);

		tmp_ME_caddpnt_table->me1_ip_cddpnt_st0 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st0;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st1 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st1;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st2 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st2;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st3;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st4 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st4;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st5 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st5;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st6 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st6;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st7 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st7;
		
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st0 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st0;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st1 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st1;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st2 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st2;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st3;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st4 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st4;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st5 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st5;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st6 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st6;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st7 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st7;

		tmp_ME_caddpnt_table->me1_ip_cddpnt_rnd3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_rnd3;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_rnd3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_rnd3;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_gmv = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_gmv;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_gmv = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_gmv;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_zmv = 255;
	}
	else if(s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Fast_Convergence_type ==2)
	{
		table_level = 3;
		tmp_ME_caddpnt_table = fwif_MEMC_get_ME_caddpnt_table(table_level);

		tmp_ME_caddpnt_table->me1_ip_cddpnt_st0 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st0;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st1 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st1;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st2 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st2;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st3;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st4 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st4;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st5 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st5;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st6 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st6;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st7 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st7;
		
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st0 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st0;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st1 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st1;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st2 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st2;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st3;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st4 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st4;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st5 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st5;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st6 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st6;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st7 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st7;

		tmp_ME_caddpnt_table->me1_ip_cddpnt_rnd3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_rnd3;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_rnd3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_rnd3;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_gmv = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_gmv;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_gmv = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_gmv;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_zmv = 255;
	}
	else if(s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true|| s_pContext->_output_me_sceneAnalysis.u2_panning_flag ==2) // pure slow pan | Slow Convergence
	{
		table_level = 4;
		tmp_ME_caddpnt_table = fwif_MEMC_get_ME_caddpnt_table(table_level);

		tmp_ME_caddpnt_table->me1_ip_cddpnt_st0 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st0;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st1 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st1;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st2 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st2;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st7 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st7;
		
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st0 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st0;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st1 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st1;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st2 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st2;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st7 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st7;

		tmp_ME_caddpnt_table->me1_ip_cddpnt_rnd3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_rnd3;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_rnd3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_rnd3;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_gmv = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_gmv;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_gmv = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_gmv;
	}
	else if(s_pContext->_output_me_sceneAnalysis.u1_zmv_on_flag == 1 )
	{
		table_level = 6;
		tmp_ME_caddpnt_table = fwif_MEMC_get_ME_caddpnt_table(table_level);

		tmp_ME_caddpnt_table->me1_ip_cddpnt_st0 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st0;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st1 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st1;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st2 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st2;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st3;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st4 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st4;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st5 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st5;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st6 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st6;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st7 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st7;
		
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st0 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st0;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st1 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st1;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st2 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st2;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st3;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st4 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st4;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st5 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st5;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st6 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st6;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st7 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st7;
		
		tmp_ME_caddpnt_table->me1_ip_cddpnt_rnd0 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_rnd0;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_rnd1 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_rnd1;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_rnd2 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_rnd2;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_rnd3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_rnd3;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_rnd0 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_rnd0;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_rnd1 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_rnd1;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_rnd2 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_rnd2;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_rnd3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_rnd3;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_gmv = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_gmv;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_gmv = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_gmv;
	}
	else if(s_pParam->_param_me_sceneAnalysis.b_FixRepeat_en && 
		s_pContext->_output_me_sceneAnalysis.u8_RepeatMode!=RepeatMode_OFF)
	{
		if(s_pContext->_output_me_sceneAnalysis.u1_casino_RP_detect_true || 
		   s_pContext->_output_me_sceneAnalysis.u8_RepeatMode==RepeatMode_HIGH){
			table_level = 9;
		}
		else if(s_pContext->_output_me_sceneAnalysis.u8_RepeatMode==RepeatMode_MID){
			table_level = 8;
		}
		else {//LOW
			table_level = 7;
		}
		tmp_ME_caddpnt_table = fwif_MEMC_get_ME_caddpnt_table(table_level);
	}
	else if(s_pContext->_output_me_sceneAnalysis.u1_Region_Periodic_detect_true == 3 &&(s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVx_Step!=0)&& (s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVy_Step!=0) )
	{
		table_level = 5;
		tmp_ME_caddpnt_table = fwif_MEMC_get_ME_caddpnt_table(table_level);

		tmp_ME_caddpnt_table->me1_ip_cddpnt_st0 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st0;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st1 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st1;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st2 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st2;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st3;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st4 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st4;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st5 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st5;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st6 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st6;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st7 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st7;
		
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st0 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st0;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st1 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st1;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st2 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st2;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st3;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st4 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st4;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st5 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st5;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st6 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st6;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st7 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st7;

		tmp_ME_caddpnt_table->me1_ip_cddpnt_rnd3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_rnd3;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_rnd3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_rnd3;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_gmv = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_gmv;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_gmv = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_gmv;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_zmv = 255;
	}
	else if(s_pParam->_param_me_sceneAnalysis.b_FixRepeat_en &&
		s_pContext->_output_me_sceneAnalysis.u8_RepeatMode==RepeatMode_OFF)
	{
		table_level = 0;
		tmp_ME_caddpnt_table = fwif_MEMC_get_ME_caddpnt_table(table_level);
		
		tmp_ME_caddpnt_table->me1_ip_cddpnt_rnd0 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_rnd0;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_rnd1 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_rnd1;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_rnd2 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_rnd2;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_rnd3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_rnd3;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_rnd0 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_rnd0;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_rnd1 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_rnd1;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_rnd2 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_rnd2;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_rnd3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_rnd3;
		
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st0 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st0;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st1 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st1;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st2 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st2;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st3;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st4 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st4;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st5 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st5;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st6 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st6;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st7 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st7;
		
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st0 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st0;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st1 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st1;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st2 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st2;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st3;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st4 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st4;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st5 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st5;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st6 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st6;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st7 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st7;
		
		tmp_ME_caddpnt_table->me1_ip_cddpnt_gmv = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_gmv;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_gmv = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_gmv;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_zmv = 255;
	}
	else
	{
		table_level = 0;
		tmp_ME_caddpnt_table = fwif_MEMC_get_ME_caddpnt_table(table_level);

		tmp_ME_caddpnt_table->me1_ip_cddpnt_rnd0 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_rnd0;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_rnd1 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_rnd1;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_rnd2 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_rnd2;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_rnd3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_rnd3;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_rnd0 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_rnd0;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_rnd1 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_rnd1;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_rnd2 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_rnd2;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_rnd3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_rnd3;

		tmp_ME_caddpnt_table->me1_ip_cddpnt_st0 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st0;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st1 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st1;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st2 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st2;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st3;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st4 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st4;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st5 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st5;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st6 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st6;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st7 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st7;

		tmp_ME_caddpnt_table->me1_pi_cddpnt_st0 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st0;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st1 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st1;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st2 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st2;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st3;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st4 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st4;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st5 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st5;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st6 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st6;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st7 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st7;

		tmp_ME_caddpnt_table->me1_ip_cddpnt_gmv = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_gmv;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_gmv = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_gmv;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_zmv = 255;
	}
	if(u32_ID_Log_en==1)//0xB809D758 [28]
	{
		static unsigned char counter = 0;
		if(counter % 10 == 0)
			rtd_printk(KERN_EMERG, TAG_NAME,"[%s][%d][cand pnt] table_level=%d\n",__FUNCTION__, __LINE__,table_level);
		counter++;
	}

	ME_caddpnt_wrtRegister(tmp_ME_caddpnt_table);
}


static _str_ME_Vbuf_LFSR_updStgh_table  ME_Vbuf_LFSR_updStgh_table[11]={
	{CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N},	/*0.load default*/
	{0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3},	/*1*/
	{0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf},	/*2*/ //case: small object/fast convergency
	{0x3,CTRL_N,0x3,CTRL_N,0x3,CTRL_N,0x3,CTRL_N,0x3,CTRL_N,0x3,CTRL_N,0x3,CTRL_N,0x3,CTRL_N},	/*3*/
	{CTRL_N,0x3,CTRL_N,0x3,CTRL_N,0x3,CTRL_N,0x3,CTRL_N,0x3,CTRL_N,0x3,CTRL_N,0x3,CTRL_N,0x3},	/*4*/
	{0x1,CTRL_N,0x1,CTRL_N,0x1,CTRL_N,0x1,CTRL_N,0x1,CTRL_N,0x1,CTRL_N,0x1,CTRL_N,0x1,CTRL_N},	/*5*/
	{0x1,0x3,0x1,0x3,0x1,0x3,0x1,0x3,0x1,0x3,0x1,0x3,0x1,0x3,0x1,0x3},	/*6*/
	{CTRL_N,0x1,CTRL_N,0x1,CTRL_N,0x1,CTRL_N,0x1,CTRL_N,0x1,CTRL_N,0x1,CTRL_N,0x1,CTRL_N,0x1},	/*7*/
	{0x3,0x1,0x3,0x1,0x3,0x1,0x3,0x1,0x3,0x1,0x3,0x1,0x3,0x1,0x3,0x1},	/*8*/
	{0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},	/*9*/ //repeat MID/HIGH
	{0x3,0x7,0x3,0x7,0x3,0x7,0x3,0x7,0x7,0x3,0x7,0x3,0x7,0x3,0x7,0x3},	/*10*/ //repeat LOW
};

_str_ME_Vbuf_LFSR_updStgh_table *fwif_MEMC_get_ME_Vbuf_LFSR_updStgh_table(unsigned char level)
{
//	if (ME_Vbuf_LFSR_updStgh_table == NULL) {
//		return NULL;
//	}
//	if( level==255 || level==0)
	if(level > 10)
		return (_str_ME_Vbuf_LFSR_updStgh_table *)&ME_Vbuf_LFSR_updStgh_table[0];
	else
		return (_str_ME_Vbuf_LFSR_updStgh_table *)&ME_Vbuf_LFSR_updStgh_table[level];

}

VOID  ME_Vbuf_LFSR_updStgh_wrtRegister(_str_ME_Vbuf_LFSR_updStgh_table *ME_Vbuf_LFSR_updStgh_table)
{
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3, 		ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask0_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7, 		ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask0_y);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11, 		ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask1_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 	ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask1_y);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 19, 	ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask2_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 	ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask2_y);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 	ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask3_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 	ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask3_y);

	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3, 		ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask0_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7, 		ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask0_y);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11, 		ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask1_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 	ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask1_y);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 19, 	ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask2_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 	ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask2_y);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 	ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask3_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, 	ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask3_y);
}

VOID ME_Vbuf_LFSR_updStgh_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	_PQLPARAMETER *s_pParam       = GetPQLParameter();

	static unsigned int print_cnt = 0 ;
	unsigned char table_level = 255;/* 255->load defaule */
	_str_ME_Vbuf_LFSR_updStgh_table *tmp_ME_Vbuf_LFSR_updStgh_table = NULL;
	unsigned int u1_ForceMask, u4_MaskVal, log_en;
	ReadRegister(SOFTWARE3_SOFTWARE3_59_reg, 15, 15, &u1_ForceMask);//0xB890D9EC[15]
	ReadRegister(SOFTWARE3_SOFTWARE3_59_reg, 14, 14, &log_en);//0xB890D9EC[14]

	print_cnt++;

	if(pParam->u1_MEUpdStgh_wrt_en== 0)//d5e4[9]
		return;

	if(u1_ForceMask) {
		ReadRegister(SOFTWARE3_SOFTWARE3_59_reg, 0, 3, &u4_MaskVal);//0xB890D9EC
		
		tmp_ME_Vbuf_LFSR_updStgh_table = fwif_MEMC_get_ME_Vbuf_LFSR_updStgh_table(0);
		tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask0_x = u4_MaskVal;
		tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask0_y = u4_MaskVal;
		tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask1_x = u4_MaskVal;
		tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask1_y = u4_MaskVal;
		tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask2_x = u4_MaskVal;
		tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask2_y = u4_MaskVal;
		tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask3_x = u4_MaskVal;
		tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask3_y = u4_MaskVal;

		tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask0_x = u4_MaskVal;
		tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask0_y = u4_MaskVal;
		tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask1_x = u4_MaskVal;
		tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask1_y = u4_MaskVal;
		tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask2_x = u4_MaskVal;
		tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask2_y = u4_MaskVal;
		tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask3_x = u4_MaskVal;
		tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask3_y = u4_MaskVal;
	}
	else if(s_pContext->_output_me_sceneAnalysis.u1_SportScene){
		table_level = 2;
		tmp_ME_Vbuf_LFSR_updStgh_table = fwif_MEMC_get_ME_Vbuf_LFSR_updStgh_table(table_level);
	}
	else if(s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true || s_pContext->_output_me_sceneAnalysis.u2_panning_flag ==2 || s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true ) // pure slow pan | Slow Convergence
	{
		//  lfsr_mask
		table_level = 1;
		tmp_ME_Vbuf_LFSR_updStgh_table = fwif_MEMC_get_ME_Vbuf_LFSR_updStgh_table(table_level);
	}
	else if(s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Fast_Convergence_true ==1)
	{
		//  lfsr_mask
		table_level = 2;
		tmp_ME_Vbuf_LFSR_updStgh_table = fwif_MEMC_get_ME_Vbuf_LFSR_updStgh_table(table_level);
	}
	else if(s_pParam->_param_me_sceneAnalysis.b_FixRepeat_en &&
		s_pContext->_output_me_sceneAnalysis.u8_RepeatMode!=RepeatMode_OFF) {
		if(s_pContext->_external_data.u8_repeatCase==RepeatCase_HoldOn||s_pContext->_external_data.u8_repeatCase==RepeatCase_SlowMV){
			table_level = 10;
		}
		else if(s_pContext->_output_me_sceneAnalysis.u8_RepeatMode==RepeatMode_LOW){
			table_level = 10;
		}
		else {//MID/HIGH
			table_level = 9;
		}
		tmp_ME_Vbuf_LFSR_updStgh_table = fwif_MEMC_get_ME_Vbuf_LFSR_updStgh_table(table_level);
	}
	else if(((s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true ==1)||(s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true ==4))&&
			((s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVx_Step != 0)&&(s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVy_Step == 0)))
	{
		//  lfsr_mask
		if(s_pContext->_output_read_comreg.s11_rgn_mvx_max < 30)
		{
			table_level = 6;
			tmp_ME_Vbuf_LFSR_updStgh_table = fwif_MEMC_get_ME_Vbuf_LFSR_updStgh_table(table_level);
		}
		else
		{
			table_level = 5;
			tmp_ME_Vbuf_LFSR_updStgh_table = fwif_MEMC_get_ME_Vbuf_LFSR_updStgh_table(table_level);
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask0_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask0_y;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask1_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask1_y;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask2_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask2_y;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask3_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask3_y;

			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask0_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask0_y;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask1_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask1_y;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask2_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask2_y;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask3_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask3_y;
		}
	}
	else if(((s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true ==1)||(s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true ==4))&&
			((s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVx_Step == 0)&&(s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVy_Step != 0)))
	{
		//  lfsr_mask
		if(s_pContext->_output_read_comreg.s11_rgn_mvy_max < 30)
		{
			table_level = 8;
			tmp_ME_Vbuf_LFSR_updStgh_table = fwif_MEMC_get_ME_Vbuf_LFSR_updStgh_table(table_level);	
		}
		else
		{
			table_level = 7;
			tmp_ME_Vbuf_LFSR_updStgh_table = fwif_MEMC_get_ME_Vbuf_LFSR_updStgh_table(table_level);		

			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask0_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask0_x;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask1_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask1_x;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask2_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask2_x;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask3_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask3_x;

			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask0_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask0_x;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask1_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask1_x;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask2_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask2_x;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask3_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask3_x;		
		}		
	}
	else
	{

		//  lfsr_mask
		if( (s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVx_Step!= 0) && (s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVy_Step != 0))
		{
			table_level = 1;
			tmp_ME_Vbuf_LFSR_updStgh_table = fwif_MEMC_get_ME_Vbuf_LFSR_updStgh_table(table_level);
		}
		else if( (s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVx_Step!= 0) && (s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVy_Step == 0))
		{				
			table_level =3;
			tmp_ME_Vbuf_LFSR_updStgh_table = fwif_MEMC_get_ME_Vbuf_LFSR_updStgh_table(table_level);

			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask0_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask0_y;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask1_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask1_y;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask2_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask2_y;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask3_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask3_y;

			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask0_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask0_y;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask1_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask1_y;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask2_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask2_y;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask3_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask3_y;
			
		}
		else if ( (s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVx_Step == 0) && (s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVy_Step != 0))
		{
			table_level =4;
			tmp_ME_Vbuf_LFSR_updStgh_table = fwif_MEMC_get_ME_Vbuf_LFSR_updStgh_table(table_level);

			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask0_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask0_x;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask1_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask1_x;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask2_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask2_x;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask3_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask3_x;

			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask0_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask0_x;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask1_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask1_x;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask2_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask2_x;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask3_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask3_x;
			
		}
		else if(s_pParam->_param_me_sceneAnalysis.b_FixRepeat_en && 
			s_pContext->_output_me_sceneAnalysis.u8_RepeatMode==RepeatMode_OFF) {
			table_level = 0;
			tmp_ME_Vbuf_LFSR_updStgh_table = fwif_MEMC_get_ME_Vbuf_LFSR_updStgh_table(table_level);

			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask0_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask0_x;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask1_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask1_x;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask2_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask2_x;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask3_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask3_x;

			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask0_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask0_x;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask1_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask1_x;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask2_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask2_x;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask3_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask3_x;

			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask0_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask0_y;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask1_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask1_y;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask2_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask2_y;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask3_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask3_y;

			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask0_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask0_y;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask1_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask1_y;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask2_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask2_y;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask3_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask3_y;
		}
		else
		{
			table_level = 0;//load default
			tmp_ME_Vbuf_LFSR_updStgh_table = fwif_MEMC_get_ME_Vbuf_LFSR_updStgh_table(table_level);//load default structure address

			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask0_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask0_x;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask1_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask1_x;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask2_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask2_x;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask3_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask3_x;

			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask0_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask0_x;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask1_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask1_x;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask2_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask2_x;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask3_x = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask3_x;

			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask0_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask0_y;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask1_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask1_y;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask2_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask2_y;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_ip_1st_lfsr_mask3_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask3_y;

			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask0_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask0_y;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask1_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask1_y;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask2_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask2_y;
			tmp_ME_Vbuf_LFSR_updStgh_table->vbuf_pi_1st_lfsr_mask3_y = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask3_y;
		}		

	}

	ME_Vbuf_LFSR_updStgh_wrtRegister(tmp_ME_Vbuf_LFSR_updStgh_table);
	if(log_en){//D9EC[14]
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s][%d][Random mask]table_level=%d\n",__FUNCTION__, __LINE__, table_level);
	}
}


static _str_ME_cost_table  ME_cost_table[5]={
	{CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N},	/*0.load default*/
	{0xffffffff,0xa,0xffffffff,0xa,0x1,0x100,0x100,0x100,0x0,0x4,0x41,0xc,0x3,0x0,0x7fb,0x7fe,0x0,0x1e4,0x39c,0x0,0x5,0x33,0x0,0x0},	/*1*/
	{0x3e,0x3,0x0,0x1,0x1,0x88,0x88,0x24,0x0,0x4,0x41,0xc,0x3,0x0,0x7fb,0x7fe,0x0,0x1e4,0x39c,0x0,0x5,0x33,0x0,0x0},	/*2*/
	{0x0,0xf,0x0,0x1,0x1,0xa,0xa,0x24,0x0,0x4,0x41,0xc,0x3,0x0,0x7fb,0x7fe,0x0,0x1e4,0x39c,0x0,0x5,0x33,0x0,0x0},	/*3*/
	{0x0,0x1,0x0,0x1,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N},	/*4*/
};

_str_ME_cost_table *fwif_MEMC_get_ME_cost_table(unsigned char level)
{
//	if (ME_cost_table == NULL) {
//		return NULL;
//	}
//	if( level==255 || level==0)
	if(level > 4)
		return (_str_ME_cost_table *)&ME_cost_table[0];
	else
		return (_str_ME_cost_table *)&ME_cost_table[level];

}

VOID  ME_cost_wrtRegister(_str_ME_cost_table *ME_cost_table)
{
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP1_GMVCOST_0_reg, 0, 31, 	ME_cost_table->me1_gmvcost_sel	);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP1_GMVCOST_2_reg, 0, 5,		ME_cost_table->me1_gmvd_gain	);
	WriteRegister(KME_ME2_CALC1_KME_ME2_GMVCOST_0_reg, 0, 31, 		ME_cost_table->me2_gmvcost_sel	);
	WriteRegister(KME_ME2_CALC1_KME_ME2_GMVCOST_1_reg, 0, 5, 			ME_cost_table->me2_gmvd_gain	);

	WriteRegister(HARDWARE_HARDWARE_18_reg, 2, 2, 				ME_cost_table->pql_me1_gmvd_sel_en	);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_4C_reg, 0, 12, 			ME_cost_table->me1_pi_gmvd_cost_limt	);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_0C_reg, 0, 12, 			ME_cost_table->me1_ip_gmvd_cost_limt	);
	WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_78_reg, 15, 27, 			ME_cost_table->me2_ph_gmvd_cost_limt);

	WriteRegister(KME_ME1_TOP1_ME1_COMMON1_16_reg, 12, 21, 			ME_cost_table->me1_gmvd_ucf_x1	);
	WriteRegister(KME_ME1_TOP1_ME1_COMMON1_16_reg, 22, 31, 			ME_cost_table->me1_gmvd_ucf_x2	);
	WriteRegister(KME_ME1_TOP1_ME1_COMMON1_17_reg, 0, 9, 			ME_cost_table->me1_gmvd_ucf_x3	);
	WriteRegister(KME_ME1_TOP1_ME1_COMMON1_17_reg, 16, 23, 			ME_cost_table->me1_gmvd_ucf_y1	);
	WriteRegister(KME_ME1_TOP1_ME1_COMMON1_17_reg, 24, 31, 			ME_cost_table->me1_gmvd_ucf_y2	);
	WriteRegister(KME_ME1_TOP1_ME1_COMMON1_18_reg, 0, 7, 			ME_cost_table->me1_gmvd_ucf_y3	);
	WriteRegister(KME_ME1_TOP1_ME1_COMMON1_18_reg, 8, 18, 			ME_cost_table->me1_gmvd_ucf_slope1	);
	WriteRegister(KME_ME1_TOP1_ME1_COMMON1_18_reg, 20, 30, 			ME_cost_table->me1_gmvd_ucf_slope2	);

	WriteRegister(KME_ME1_TOP1_ME1_COMMON1_19_reg, 0, 11, 		ME_cost_table->me1_gmvd_cnt_x1	);
	WriteRegister(KME_ME1_TOP1_ME1_COMMON1_19_reg, 12, 23, 		ME_cost_table->me1_gmvd_cnt_x2	);
	WriteRegister(KME_ME1_TOP1_ME1_COMMON1_20_reg, 0, 11, 		ME_cost_table->me1_gmvd_cnt_x3	);
	WriteRegister(KME_ME1_TOP1_ME1_COMMON1_20_reg, 16, 23, 		ME_cost_table->me1_gmvd_cnt_y1	);
	WriteRegister(KME_ME1_TOP1_ME1_COMMON1_20_reg, 24, 31, 		ME_cost_table->me1_gmvd_cnt_y2	);
	WriteRegister(KME_ME1_TOP1_ME1_COMMON1_21_reg, 0, 7, 		ME_cost_table->me1_gmvd_cnt_y3	);
	WriteRegister(KME_ME1_TOP1_ME1_COMMON1_21_reg, 8, 18, 		ME_cost_table->me1_gmvd_cnt_slope1		);
	WriteRegister(KME_ME1_TOP1_ME1_COMMON1_21_reg, 20, 30, 		ME_cost_table->me1_gmvd_cnt_slope2		);
}

VOID ME_cost_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
 	const _PQLCONTEXT *s_pContext = GetPQLContext();
	//_PQLPARAMETER *s_pParam       = GetPQLParameter();
	unsigned char table_level = 255;/* 255->load defaule */
	_str_ME_cost_table *tmp_ME_cost_table = NULL;

	if(pParam->u1_ME_Cost_Ctrl== 0)
		return;

	if( s_pContext->_output_wrt_comreg.u1_casino_RP_detect_true == 1 )
	{
		table_level = 1;
		tmp_ME_cost_table = fwif_MEMC_get_ME_cost_table(table_level);
	}
	else if(s_pContext->_output_wrt_comreg.u1_KME_gmvd_cost_type == 1 )
	{
		table_level = 2;
		tmp_ME_cost_table = fwif_MEMC_get_ME_cost_table(table_level);
	}
	else if(s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true >= 1 )
	{
		table_level = 3;
		tmp_ME_cost_table = fwif_MEMC_get_ME_cost_table(table_level);
	}
	else
	{
		table_level = 4;
		tmp_ME_cost_table = fwif_MEMC_get_ME_cost_table(table_level);

		tmp_ME_cost_table->pql_me1_gmvd_sel_en		=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_sel;
		tmp_ME_cost_table->me1_pi_gmvd_cost_limt		=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_gmvd_cost_limt;
		tmp_ME_cost_table->me1_ip_gmvd_cost_limt		=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_gmvd_cost_limt;
		tmp_ME_cost_table->me2_ph_gmvd_cost_limt		=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me2_ph_gmvd_cost_limt;

		tmp_ME_cost_table->me1_gmvd_ucf_x1	=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_ucf_x1;
		tmp_ME_cost_table->me1_gmvd_ucf_x2	=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_ucf_x2;
		tmp_ME_cost_table->me1_gmvd_ucf_x3	=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_ucf_x3;
		tmp_ME_cost_table->me1_gmvd_ucf_y1	=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_ucf_y1;
		tmp_ME_cost_table->me1_gmvd_ucf_y2	=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_ucf_y2;
		tmp_ME_cost_table->me1_gmvd_ucf_y3	=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_ucf_y3;
		tmp_ME_cost_table->me1_gmvd_ucf_slope1	=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_ucf_slope1;
		tmp_ME_cost_table->me1_gmvd_ucf_slope2	=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_ucf_slope2;

		tmp_ME_cost_table->me1_gmvd_cnt_x1	=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_cnt_x1;
		tmp_ME_cost_table->me1_gmvd_cnt_x2	=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_cnt_x2;
		tmp_ME_cost_table->me1_gmvd_cnt_x3	=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_cnt_x3;
		tmp_ME_cost_table->me1_gmvd_cnt_y1	=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_cnt_y1;
		tmp_ME_cost_table->me1_gmvd_cnt_y2	=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_cnt_y2;
		tmp_ME_cost_table->me1_gmvd_cnt_y3	=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_cnt_y3;
		 tmp_ME_cost_table->me1_gmvd_cnt_slope1 	=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_cnt_slope1;
		 tmp_ME_cost_table->me1_gmvd_cnt_slope2	=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_cnt_slope2;
	}

	ME_cost_wrtRegister(tmp_ME_cost_table);
}

VOID ME_gmv_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
 	const _PQLCONTEXT *s_pContext = GetPQLContext();
	//_PQLPARAMETER *s_pParam       = GetPQLParameter();

	if(pParam->u1_GMV_wrt_en== 0)
		return;

	if(s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true == 1 || s_pContext->_output_Patch_Manage.u1_patch_highFreq_rp_true)
	{
		//k4lp RP_detect no need
		//WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 0x0);

	}
	else
	{
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_gmv_sel);
	}
}

static _str_ME_ipme_flp_alp_table   ME_ipme_flp_alp_table[4]={
	{CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N},	/*0.load default*/
	{0x16,0x16,0x14,0x14,0x12,0x12,0x10,0x16,0x16,0x14,0x14,0x12,0x12,0x10},	/*1*/
	{0x20,0x1c,0x18,0x14,0x10,0xe,0xa,0x20,0x1c,0x18,0x14,0x10,0xe,0xa},	/*2*/ //repeat MID/HIGH
	{0x30,0x26,0x1C,0x12,0x8,0x7,0x5,0x30,0x26,0x1C,0x12,0x8,0x7,0x5},		/*3*/ //repeat LOW
};

_str_ME_ipme_flp_alp_table *fwif_MEMC_get_ME_ipme_flp_alp_table(unsigned char level)
{
//	if (ME_ipme_flp_alp_table == NULL) {
//		return NULL;
//	}
//	if( level==255 || level==0)
	if(level > 3)
		return (_str_ME_ipme_flp_alp_table *)&ME_ipme_flp_alp_table[0];
	else
		return (_str_ME_ipme_flp_alp_table *)&ME_ipme_flp_alp_table[level];

}

VOID   ME_ipme_flp_alp_wrtRegister(_str_ME_ipme_flp_alp_table *ME_ipme_flp_alp_table)
{
	WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 		ME_ipme_flp_alp_table->ipme_h_flp_alp0);
	WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 		ME_ipme_flp_alp_table->ipme_h_flp_alp1);
	WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 	ME_ipme_flp_alp_table->ipme_h_flp_alp2);
	WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 		ME_ipme_flp_alp_table->ipme_h_flp_alp3);
	WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 		ME_ipme_flp_alp_table->ipme_h_flp_alp4);
	WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 	ME_ipme_flp_alp_table->ipme_h_flp_alp5);
	WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 		ME_ipme_flp_alp_table->ipme_h_flp_alp6);

	WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 		ME_ipme_flp_alp_table->ipme_v_flp_alp0);
	WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 	ME_ipme_flp_alp_table->ipme_v_flp_alp1);
	WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 		ME_ipme_flp_alp_table->ipme_v_flp_alp2);
	WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 		ME_ipme_flp_alp_table->ipme_v_flp_alp3);
	WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 	ME_ipme_flp_alp_table->ipme_v_flp_alp4);
	WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 		ME_ipme_flp_alp_table->ipme_v_flp_alp5);
	WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 		ME_ipme_flp_alp_table->ipme_v_flp_alp6);
}

VOID ME_ipme_flp_alp_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
 	const _PQLCONTEXT *s_pContext = GetPQLContext();
	_PQLPARAMETER *s_pParam       = GetPQLParameter();
	unsigned char table_level = 255;/* 255->load defaule */
	_str_ME_ipme_flp_alp_table *tmp_ME_ipme_flp_alp_table = NULL;
	unsigned int u1_ForceFlpAlp, FlpAlp[7]={0}, log_en;
	ReadRegister(SOFTWARE3_SOFTWARE3_57_reg, 31, 31, &u1_ForceFlpAlp);//0xb809d9e4[31]
	ReadRegister(SOFTWARE3_SOFTWARE3_57_reg, 30, 30, &log_en);//0xb809d9e4[30]

	if(pParam->u1_ME_ipme_flp_alp_wrt_en== 0)//d5e4[18]
		return;

	if(u1_ForceFlpAlp) { //0xb809d9e4[31]
		tmp_ME_ipme_flp_alp_table = fwif_MEMC_get_ME_ipme_flp_alp_table(0);

		ReadRegister(SOFTWARE3_SOFTWARE3_55_reg,  0,  8, &FlpAlp[0]);//0xb809d9dc
		ReadRegister(SOFTWARE3_SOFTWARE3_55_reg,  9, 17, &FlpAlp[1]);
		ReadRegister(SOFTWARE3_SOFTWARE3_55_reg, 18, 26, &FlpAlp[2]);
		ReadRegister(SOFTWARE3_SOFTWARE3_56_reg,  0,  8, &FlpAlp[3]);//0xb809d9e0
		ReadRegister(SOFTWARE3_SOFTWARE3_56_reg,  9, 17, &FlpAlp[4]);
		ReadRegister(SOFTWARE3_SOFTWARE3_56_reg, 18, 26, &FlpAlp[5]);
		ReadRegister(SOFTWARE3_SOFTWARE3_57_reg,  0,  8, &FlpAlp[6]);//0xb809d9e4
		tmp_ME_ipme_flp_alp_table->ipme_h_flp_alp0 = FlpAlp[0];
		tmp_ME_ipme_flp_alp_table->ipme_h_flp_alp1 = FlpAlp[1];
		tmp_ME_ipme_flp_alp_table->ipme_h_flp_alp2 = FlpAlp[2];
		tmp_ME_ipme_flp_alp_table->ipme_h_flp_alp3 = FlpAlp[3];
		tmp_ME_ipme_flp_alp_table->ipme_h_flp_alp4 = FlpAlp[4];
		tmp_ME_ipme_flp_alp_table->ipme_h_flp_alp5 = FlpAlp[5];
		tmp_ME_ipme_flp_alp_table->ipme_h_flp_alp6 = FlpAlp[6];

		tmp_ME_ipme_flp_alp_table->ipme_v_flp_alp0 = FlpAlp[0];
		tmp_ME_ipme_flp_alp_table->ipme_v_flp_alp1 = FlpAlp[1];
		tmp_ME_ipme_flp_alp_table->ipme_v_flp_alp2 = FlpAlp[2];
		tmp_ME_ipme_flp_alp_table->ipme_v_flp_alp3 = FlpAlp[3];
		tmp_ME_ipme_flp_alp_table->ipme_v_flp_alp4 = FlpAlp[4];
		tmp_ME_ipme_flp_alp_table->ipme_v_flp_alp5 = FlpAlp[5];
		tmp_ME_ipme_flp_alp_table->ipme_v_flp_alp6 = FlpAlp[6];
	}
	else if(s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type==2)
	{
		table_level = 1;
		tmp_ME_ipme_flp_alp_table = fwif_MEMC_get_ME_ipme_flp_alp_table(table_level);
	}
	else if(s_pParam->_param_me_sceneAnalysis.b_FixRepeat_en && 
		s_pContext->_output_me_sceneAnalysis.u8_RepeatMode!=RepeatMode_OFF) {
		//if(s_pContext->_output_me_sceneAnalysis.u1_casino_RP_detect_true){
		//	table_level = 3; {0x14,0x14,0x14,0x14,0x14,0x13,0x13,0x14,0x14,0x14,0x14,0x14,0x13,0x13}
		//}
		//else{
		//	table_level = 4; {60,50,16,16,8,4,2,60,50,16,16,8,4,2}
		//}
		if(s_pContext->_output_me_sceneAnalysis.u8_RepeatMode==RepeatMode_LOW) {
			table_level = 3;
		}
		else {//MID/HIGH
			table_level = 2;
		}
		tmp_ME_ipme_flp_alp_table = fwif_MEMC_get_ME_ipme_flp_alp_table(table_level);
	}
	else if((s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true >= 1 && s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true !=3)|| s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type==1 )
	{
		table_level = 2;
		tmp_ME_ipme_flp_alp_table = fwif_MEMC_get_ME_ipme_flp_alp_table(table_level);
	}
	else if(s_pParam->_param_me_sceneAnalysis.b_FixRepeat_en &&
		s_pContext->_output_me_sceneAnalysis.u8_RepeatMode==RepeatMode_OFF) {
		table_level = 0;
		tmp_ME_ipme_flp_alp_table = fwif_MEMC_get_ME_ipme_flp_alp_table(table_level);

		tmp_ME_ipme_flp_alp_table->ipme_h_flp_alp0=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp0;
		tmp_ME_ipme_flp_alp_table->ipme_h_flp_alp1=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp1;
		tmp_ME_ipme_flp_alp_table->ipme_h_flp_alp2=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp2;
		tmp_ME_ipme_flp_alp_table->ipme_h_flp_alp3=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp3;
		tmp_ME_ipme_flp_alp_table->ipme_h_flp_alp4=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp4;
		tmp_ME_ipme_flp_alp_table->ipme_h_flp_alp5=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp5;
		tmp_ME_ipme_flp_alp_table->ipme_h_flp_alp6=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp6;

		tmp_ME_ipme_flp_alp_table->ipme_v_flp_alp0=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp0;
		tmp_ME_ipme_flp_alp_table->ipme_v_flp_alp1=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp1;
		tmp_ME_ipme_flp_alp_table->ipme_v_flp_alp2=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp2;
		tmp_ME_ipme_flp_alp_table->ipme_v_flp_alp3=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp3;
		tmp_ME_ipme_flp_alp_table->ipme_v_flp_alp4=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp4;
		tmp_ME_ipme_flp_alp_table->ipme_v_flp_alp5=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp5;
		tmp_ME_ipme_flp_alp_table->ipme_v_flp_alp6=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp6;
	}
	else
	{
		table_level = 0;
		tmp_ME_ipme_flp_alp_table = fwif_MEMC_get_ME_ipme_flp_alp_table(table_level);

		tmp_ME_ipme_flp_alp_table->ipme_h_flp_alp0=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp0;
		tmp_ME_ipme_flp_alp_table->ipme_h_flp_alp1=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp1;
		tmp_ME_ipme_flp_alp_table->ipme_h_flp_alp2=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp2;
		tmp_ME_ipme_flp_alp_table->ipme_h_flp_alp3=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp3;
		tmp_ME_ipme_flp_alp_table->ipme_h_flp_alp4=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp4;
		tmp_ME_ipme_flp_alp_table->ipme_h_flp_alp5=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp5;
		tmp_ME_ipme_flp_alp_table->ipme_h_flp_alp6=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp6;

		tmp_ME_ipme_flp_alp_table->ipme_v_flp_alp0=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp0;
		tmp_ME_ipme_flp_alp_table->ipme_v_flp_alp1=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp1;
		tmp_ME_ipme_flp_alp_table->ipme_v_flp_alp2=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp2;
		tmp_ME_ipme_flp_alp_table->ipme_v_flp_alp3=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp3;
		tmp_ME_ipme_flp_alp_table->ipme_v_flp_alp4=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp4;
		tmp_ME_ipme_flp_alp_table->ipme_v_flp_alp5=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp5;
		tmp_ME_ipme_flp_alp_table->ipme_v_flp_alp6=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp6;
	}

	ME_ipme_flp_alp_wrtRegister(tmp_ME_ipme_flp_alp_table);

	if(log_en) { //d9e4[30]
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s][%d][FLP ALP] table_level=%d RM:%d\n",__FUNCTION__, __LINE__, table_level, s_pContext->_output_me_sceneAnalysis.u8_RepeatMode);
	}
}

static _str_ME_mvd_cuv_table   ME_mvd_cuv_table[2]={
	{CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N},	/*0.load default*/
	{0x0,0x1e5,0x321,0x0,0x26,0x4e,0x5,0x8},	/*1*/
};

_str_ME_mvd_cuv_table *fwif_MEMC_get_ME_mvd_cuv_table(unsigned char level)
{
//	if (ME_mvd_cuv_table == NULL) {
//		return NULL;
//	}
//	if( level==255 || level==0)
	if(level > 1)
		return (_str_ME_mvd_cuv_table *)&ME_mvd_cuv_table[0];
	else
		return (_str_ME_mvd_cuv_table *)&ME_mvd_cuv_table[level];

}

VOID   ME_mvd_cuv_wrtRegister(_str_ME_mvd_cuv_table *ME_mvd_cuv_table)
{
	WriteRegister(KME_ME1_TOP1_ME1_COMMON1_06_reg, 0, 9, 		ME_mvd_cuv_table->me1_mvd_cuv_x1);
	WriteRegister(KME_ME1_TOP1_ME1_COMMON1_06_reg, 10, 19, 		ME_mvd_cuv_table->me1_mvd_cuv_x2);
	WriteRegister(KME_ME1_TOP1_ME1_COMMON1_06_reg, 20, 29, 		ME_mvd_cuv_table->me1_mvd_cuv_x3);
	WriteRegister(KME_ME1_TOP1_ME1_COMMON1_07_reg, 0, 9, 		ME_mvd_cuv_table->me1_mvd_cuv_y1);
	WriteRegister(KME_ME1_TOP1_ME1_COMMON1_07_reg, 10, 19, 		ME_mvd_cuv_table->me1_mvd_cuv_y2);
	WriteRegister(KME_ME1_TOP1_ME1_COMMON1_07_reg, 20, 29,		ME_mvd_cuv_table->me1_mvd_cuv_y3);
	WriteRegister(KME_ME1_TOP1_ME1_COMMON1_08_reg, 0, 12, 		ME_mvd_cuv_table->me1_mvd_cuv_slope1);
	WriteRegister(KME_ME1_TOP1_ME1_COMMON1_08_reg, 13, 25, 		ME_mvd_cuv_table->me1_mvd_cuv_slope2);
}

VOID ME_mvd_cuv_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
 	const _PQLCONTEXT *s_pContext = GetPQLContext();
	//_PQLPARAMETER *s_pParam       = GetPQLParameter();
	unsigned char table_level = 255;/* 255->load defaule */
	_str_ME_mvd_cuv_table *tmp_ME_mvd_cuv_table = NULL;

	if(pParam->u1_ME_mvd_cuv_wrt_en== 0)
		return;

	if(s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true >= 1 || s_pContext->_output_Patch_Manage.u1_patch_highFreq_rp_true)
	{
		table_level = 1;
		tmp_ME_mvd_cuv_table = fwif_MEMC_get_ME_mvd_cuv_table(table_level);
	}
	else
	{
		table_level = 0;
		tmp_ME_mvd_cuv_table = fwif_MEMC_get_ME_mvd_cuv_table(table_level);

		tmp_ME_mvd_cuv_table->me1_mvd_cuv_x1=		s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_mvd_cuv_x1;
		tmp_ME_mvd_cuv_table->me1_mvd_cuv_x2=		s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_mvd_cuv_x2;
		tmp_ME_mvd_cuv_table->me1_mvd_cuv_x3=		s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_mvd_cuv_x3;
		tmp_ME_mvd_cuv_table->me1_mvd_cuv_y1=		s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_mvd_cuv_y1;
		tmp_ME_mvd_cuv_table->me1_mvd_cuv_y2=		s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_mvd_cuv_y2;
		tmp_ME_mvd_cuv_table->me1_mvd_cuv_y3=		s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_mvd_cuv_y3;
		tmp_ME_mvd_cuv_table->me1_mvd_cuv_slope1=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_mvd_cuv_slope1;
		tmp_ME_mvd_cuv_table->me1_mvd_cuv_slope2=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_mvd_cuv_slope2;
	}

	ME_mvd_cuv_wrtRegister(tmp_ME_mvd_cuv_table);

}

static _str_ME_DRP_table   ME_DRP_table[3]={
	{0x0,CTRL_N,CTRL_N,CTRL_N},	/*0.load default*/
	{0x1,CTRL_N,CTRL_N,CTRL_N},	/*1*/
	{0x0,0x384,0x110,0xF8},	/*2*/
};

_str_ME_DRP_table *fwif_MEMC_get_ME_DRP_table(unsigned char level)
{
//	if (ME_DRP_table == NULL) {
//		return NULL;
//	}
//	if( level==255 || level==0)
	if(level > 2)
		return (_str_ME_DRP_table *)&ME_DRP_table[0];
	else
		return (_str_ME_DRP_table *)&ME_DRP_table[level];

}

VOID   ME_DRP_wrtRegister(_str_ME_DRP_table *ME_DRP_table)
{
	WriteRegister(KME_ME1_TOP0_KME_ME1_DRP_reg, 31, 31, 		ME_DRP_table->me1_drp_en);
	WriteRegister(KME_ME1_TOP0_KME_ME1_DRP_2_reg, 20, 29, 		ME_DRP_table->me1_drp_cuv_y2);
	WriteRegister(KME_ME1_TOP0_KME_ME1_DRP_2_reg, 10, 19, 		ME_DRP_table->me1_drp_cuv_y1);
	WriteRegister(KME_ME1_TOP0_KME_ME1_DRP_2_reg, 0, 9, 		ME_DRP_table->me1_drp_cuv_y0);
}

VOID ME_DRP_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
 	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned char table_level = 255;/* 255->load defaule */
	_str_ME_DRP_table *tmp_ME_DRP_table = NULL;

	if(pParam->u1_ME1_DRP_en == 0)
		return;
	#if (IC_K5LP || IC_K6LP || IC_K8LP)
	if(s_pContext->_output_wrt_comreg.u1_ME1_DRP_type == 1)
	{
		table_level = 1;
		tmp_ME_DRP_table = fwif_MEMC_get_ME_DRP_table(table_level);

		tmp_ME_DRP_table->me1_drp_cuv_y2=s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_drp_cuv_y2;
		tmp_ME_DRP_table->me1_drp_cuv_y1=s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_drp_cuv_y1;
		tmp_ME_DRP_table->me1_drp_cuv_y0=s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_drp_cuv_y0;
	}
	else if(s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true >= 1 ) {

		 if(s_pContext->_output_me_sceneAnalysis.u1_RP_DTLPRD_move_check >= 3){
		 	table_level = 1;
			tmp_ME_DRP_table = fwif_MEMC_get_ME_DRP_table(table_level);

			tmp_ME_DRP_table->me1_drp_cuv_y2=s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_drp_cuv_y2;
			tmp_ME_DRP_table->me1_drp_cuv_y1=s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_drp_cuv_y1;
			tmp_ME_DRP_table->me1_drp_cuv_y0=s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_drp_cuv_y0;
		 }else{
		 	table_level = 2;
			tmp_ME_DRP_table = fwif_MEMC_get_ME_DRP_table(table_level);
		 }
	}
	else if(s_pContext->_output_me_sceneAnalysis.u1_Region_Periodic_detect_true == 3 &&(s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVx_Step!=0)&& (s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVy_Step!=0) )
	{
		table_level = 2;
		tmp_ME_DRP_table = fwif_MEMC_get_ME_DRP_table(table_level);
	}
	else {
		table_level = 0;
		tmp_ME_DRP_table = fwif_MEMC_get_ME_DRP_table(table_level);

		tmp_ME_DRP_table->me1_drp_cuv_y2=s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_drp_cuv_y2;
		tmp_ME_DRP_table->me1_drp_cuv_y1=s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_drp_cuv_y1;
		tmp_ME_DRP_table->me1_drp_cuv_y0=s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_drp_cuv_y0;
	}
	#endif

	ME_DRP_wrtRegister(tmp_ME_DRP_table);

}

static _str_ME_adptpnt_rnd_cuv_table  ME_adptpnt_rnd_cuv_table[5]={
	{CTRL_N,CTRL_N,CTRL_N,CTRL_N},	/*0.load default*/
	{0x30,0x28,0x24,0x22,CTRL_N},	/*1*/
	{0x3f,0x3f,0x3f,0x3f,CTRL_N},	/*2*/
	{0x24,0x1e,0x10,0xe,CTRL_N},		/*3*/
	{0x24,0x1e,0x0a,0x10,0x1fc},		/*4*/
};

_str_ME_adptpnt_rnd_cuv_table *fwif_MEMC_get_ME_adptpnt_rnd_cuv_table(unsigned char level)
{
//	if (ME_adptpnt_rnd_cuv_table == NULL) {
//		return NULL;
//	}
//	if( level==255 || level==0)
	if(level > 4)
		return (_str_ME_adptpnt_rnd_cuv_table *)&ME_adptpnt_rnd_cuv_table[0];
	else
		return (_str_ME_adptpnt_rnd_cuv_table *)&ME_adptpnt_rnd_cuv_table[level];

}

VOID   ME_adptpnt_rnd_cuv_wrtRegister(_str_ME_adptpnt_rnd_cuv_table *ME_adptpnt_rnd_cuv_table)
{
	WriteRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 0, 5, 	ME_adptpnt_rnd_cuv_table->me1_adptpnt_rnd_y1);
	WriteRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 8, 13, 	ME_adptpnt_rnd_cuv_table->me1_adptpnt_rnd_y2);
	WriteRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 14, 19,	ME_adptpnt_rnd_cuv_table->me1_adptpnt_rnd_y3);
	WriteRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 20, 25, 	ME_adptpnt_rnd_cuv_table->me1_adptpnt_rnd_y4);
}

VOID ME_adptpnt_rnd_cuv_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
 	const _PQLCONTEXT *s_pContext = GetPQLContext();
	//_PQLPARAMETER *s_pParam       = GetPQLParameter();
	unsigned char table_level = 255;/* 255->load defaule */
	_str_ME_adptpnt_rnd_cuv_table *tmp_ME_adptpnt_rnd_cuv_table = NULL;

	if(pParam->u1_ME_AdptPnt_rnd_wrt_en == 0)
		return;

	if(s_pContext->_output_me_sceneAnalysis.u2_panning_flag == 2){   // pure slow pan
		table_level = 1;
		tmp_ME_adptpnt_rnd_cuv_table = fwif_MEMC_get_ME_adptpnt_rnd_cuv_table(table_level);
		tmp_ME_adptpnt_rnd_cuv_table->me1_adptpnt_rnd_slope2= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_adptpnt_rnd_slope2;
	}
	else if(s_pContext->_output_wrt_comreg.u1_ME1_adptpnt_rnd_Slow_Convergence_true==1)
	{
		table_level = 2;
		tmp_ME_adptpnt_rnd_cuv_table = fwif_MEMC_get_ME_adptpnt_rnd_cuv_table(table_level);
		tmp_ME_adptpnt_rnd_cuv_table->me1_adptpnt_rnd_slope2 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_adptpnt_rnd_slope2;
	}
	else if(s_pContext->_output_wrt_comreg.u1_ME1_adptpnt_rnd_gain_true==1)
	{
		table_level = 3;
		tmp_ME_adptpnt_rnd_cuv_table = fwif_MEMC_get_ME_adptpnt_rnd_cuv_table(table_level);
		tmp_ME_adptpnt_rnd_cuv_table->me1_adptpnt_rnd_slope2 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_adptpnt_rnd_slope2;
	}
	else if(s_pContext->_output_me_sceneAnalysis.u1_Avenger_RP_detect_true == 1 )
	{
		table_level = 4;
		tmp_ME_adptpnt_rnd_cuv_table = fwif_MEMC_get_ME_adptpnt_rnd_cuv_table(table_level);
	}
	else
	{
		table_level = 1;
		tmp_ME_adptpnt_rnd_cuv_table = fwif_MEMC_get_ME_adptpnt_rnd_cuv_table(table_level);
		tmp_ME_adptpnt_rnd_cuv_table->me1_adptpnt_rnd_y1 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_adptpnt_rnd_y1;
		tmp_ME_adptpnt_rnd_cuv_table->me1_adptpnt_rnd_y2 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_adptpnt_rnd_y2;
		tmp_ME_adptpnt_rnd_cuv_table->me1_adptpnt_rnd_y3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_adptpnt_rnd_y3;
		tmp_ME_adptpnt_rnd_cuv_table->me1_adptpnt_rnd_y4 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_adptpnt_rnd_y4;
		tmp_ME_adptpnt_rnd_cuv_table->me1_adptpnt_rnd_slope2 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_adptpnt_rnd_slope2;
	}

	ME_adptpnt_rnd_cuv_wrtRegister(tmp_ME_adptpnt_rnd_cuv_table);

}

static _str_Dh_Local_FB_table  Dh_Local_FB_table[22]={
	{CTRL_N,CTRL_N,CTRL_N,CTRL_N},	/*0.load default*/
	{0x30,CTRL_N,CTRL_N,CTRL_N},		/*1*/
	{0x0,0x0,CTRL_N,CTRL_N},			/*2*/
	{0x60,0xff,CTRL_N,CTRL_N},		/*3*/
	{0x20,CTRL_N,CTRL_N,CTRL_N},		/*4*/
	{0x40,CTRL_N,CTRL_N,CTRL_N},		/*5*/
	{0x60,CTRL_N,CTRL_N,CTRL_N},		/*6*/
	{0x0,0x40,CTRL_N,CTRL_N},		/*7*/
	{0x0,CTRL_N,CTRL_N,CTRL_N},		/*8*/
	{0x30,CTRL_N,CTRL_N,CTRL_N},		/*9*/
	{0x0,0x12,CTRL_N,CTRL_N},		/*10*/
	{0x0,0x0,0x8,0x80},		/*11*/  // level 0  LFB+RFB level ctrl  //y1/y2/x1/x2
	{0x4,0x10,0x8,0x80},		/*12*/  
	{0x4,0x20,0x8,0x80},		/*13*/
	{0x4,0x30,0x8,0x80},		/*14*/
	{0x4,0x40,0x8,0x80},		/*15*/  
	{0x4,0x50,0x8,0x80},		/*16*/  //3
	{0x4,0x60,0x8,0x80},		/*17*/
	{0x4,0x70,0x8,0x80},		/*18*/
	{0xA,0x80,0x8,0x60},		/*19*/	// it's ori default value
	{0xA,0x90,0x8,0x60},		/*20*/
	{0xF,0xA0,0x8,0x60},		/*21*/  // level 10
};

_str_Dh_Local_FB_table *fwif_MEMC_get_Dh_Local_FB_table(unsigned char level)
{
//	if (Dh_Local_FB_table == NULL) {
//		return NULL;
//	}
//	if( level==255 || level==0)
	if(level > 21)
		return (_str_Dh_Local_FB_table *)&Dh_Local_FB_table[0];
	else
		return (_str_Dh_Local_FB_table *)&Dh_Local_FB_table[level];
}

VOID   Dh_Local_FB_wrtRegister(_str_Dh_Local_FB_table *Dh_Local_FB_table)
{
	//const _PQLCONTEXT *s_pContext = GetPQLContext();

	//if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_Test_0_flag ==0)//YE Test
	//	return;


	WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg,24, 31, Dh_Local_FB_table->dh_dtl_curve_y1);
	WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg,0, 7, 	Dh_Local_FB_table->dh_dtl_curve_y2);
	WriteRegister(KME_DEHALO_KME_DEHALO_F0_reg,24, 31, Dh_Local_FB_table->dh_dtl_curve_x1);
	WriteRegister(KME_DEHALO_KME_DEHALO_F4_reg,24, 31, 	Dh_Local_FB_table->dh_dtl_curve_x2);	
}

VOID Dh_Local_FB_wrtAction(const _PARAM_WRT_COM_REG * pParam, _OUTPUT_WRT_COM_REG * pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	//unsigned int u32_rb_val = 0;
	//static unsigned char u1_panning_state;
	unsigned char table_level = 255;/* 255->load defaule */

	#if 1 //for YE Test temp test
	static unsigned int u32_ID_Log_en=0;//,ES_Test_Value=0;

	#endif

	int special_pattern_flag = 0; // djntest rotterdam
	int special_pattern_flag_transporter = 0; // djntest transporter
	int special_pattern_flag_insect = 0; // djntest transporter
	int special_pattern_flag_eagle = 0; // djntest eagle
	
	_str_Dh_Local_FB_table *tmp_Dh_Local_FB_table = NULL;
	ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 19, 19, &special_pattern_flag);
	ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 18, 18, &special_pattern_flag_transporter);
	ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 20, 20, &special_pattern_flag_insect);
	ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 17, 17, &special_pattern_flag_eagle);
	
	ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 28,  28, &u32_ID_Log_en);//YE Test debug dummy register
	//if((pParam->u1_localfb_wrt_en == 0)||((s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag==0))) //YE Test 20200728 to enable dh_dtl_curve x1/x2/y1/y2 AutoGUI
	//if((pParam->u1_localfb_wrt_en == 0)||((s_pContext->_output_bRMV_rFB_ctrl.u8_ES_Test_0_flag==0))) //YE Test 20200728 to enable dh_dtl_curve x1/x2/y1/y2 AutoGUI
	if(pParam->u1_localfb_wrt_en == 0) //YE Test 20200728 to enable dh_dtl_curve x1/x2/y1/y2 AutoGUI
		return;

	if(s_pContext->_output_me_vst_ctrl.u1_detect_blackBG_VST_trure)
	{
		table_level = 1;
		tmp_Dh_Local_FB_table = fwif_MEMC_get_Dh_Local_FB_table(table_level);
		tmp_Dh_Local_FB_table->dh_dtl_curve_y2= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_y2;
		tmp_Dh_Local_FB_table->dh_dtl_curve_x1= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x1;
		tmp_Dh_Local_FB_table->dh_dtl_curve_x2= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x2;
		
	}
	else if((s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true== 1) || special_pattern_flag || special_pattern_flag_transporter || special_pattern_flag_insect || special_pattern_flag_eagle)

	{
		table_level = 2;
		tmp_Dh_Local_FB_table = fwif_MEMC_get_Dh_Local_FB_table(table_level);
		tmp_Dh_Local_FB_table->dh_dtl_curve_x1= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x1;
		tmp_Dh_Local_FB_table->dh_dtl_curve_x2= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x2;		
	}
	else if((s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL]&0xF) == _CAD_55)
	{
		table_level = 3;
		tmp_Dh_Local_FB_table = fwif_MEMC_get_Dh_Local_FB_table(table_level);
		tmp_Dh_Local_FB_table->dh_dtl_curve_x1= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x1;
		tmp_Dh_Local_FB_table->dh_dtl_curve_x2= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x2;
		
	}
	else if(s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type ==1)
	{
		table_level = 4;
		tmp_Dh_Local_FB_table = fwif_MEMC_get_Dh_Local_FB_table(table_level);
		tmp_Dh_Local_FB_table->dh_dtl_curve_y2= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_y2;
		tmp_Dh_Local_FB_table->dh_dtl_curve_x1= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x1;
		tmp_Dh_Local_FB_table->dh_dtl_curve_x2= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x2;
		
	}
	else if(s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type ==2)
	{
		table_level = 5;
		tmp_Dh_Local_FB_table = fwif_MEMC_get_Dh_Local_FB_table(table_level);
		tmp_Dh_Local_FB_table->dh_dtl_curve_y2= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_y2;
		tmp_Dh_Local_FB_table->dh_dtl_curve_x1= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x1;
		tmp_Dh_Local_FB_table->dh_dtl_curve_x2= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x2;		
	}
	else if(s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type ==3)
	{
		table_level = 6;
		tmp_Dh_Local_FB_table = fwif_MEMC_get_Dh_Local_FB_table(table_level);
		tmp_Dh_Local_FB_table->dh_dtl_curve_y2= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_y2;
		tmp_Dh_Local_FB_table->dh_dtl_curve_x1= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x1;
		tmp_Dh_Local_FB_table->dh_dtl_curve_x2= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x2;
	}
	else if(s_pContext->_output_wrt_comreg.u1_LFB_Dtl_increase_true==1)
	{
		table_level = 7;
		tmp_Dh_Local_FB_table = fwif_MEMC_get_Dh_Local_FB_table(table_level);
		tmp_Dh_Local_FB_table->dh_dtl_curve_x1= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x1;
		tmp_Dh_Local_FB_table->dh_dtl_curve_x2= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x2;
	}
	else if(s_pContext->_output_wrt_comreg.u8_FB_lvl >0x60)
	{
		table_level = 8;
		tmp_Dh_Local_FB_table = fwif_MEMC_get_Dh_Local_FB_table(table_level);
		tmp_Dh_Local_FB_table->dh_dtl_curve_y2= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_y2;
		tmp_Dh_Local_FB_table->dh_dtl_curve_x1= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x1;
		tmp_Dh_Local_FB_table->dh_dtl_curve_x2= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x2;
	}
	else if(s_pContext->_output_frc_sceneAnalysis.u1_Swing_true )
	{
		table_level = 2;
		tmp_Dh_Local_FB_table = fwif_MEMC_get_Dh_Local_FB_table(table_level);
		tmp_Dh_Local_FB_table->dh_dtl_curve_x1= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x1;
		tmp_Dh_Local_FB_table->dh_dtl_curve_x2= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x2;
	}
	else if(s_pContext->_output_dh_close.u1_dh_sml_rgnAction_close && s_pContext->_output_wrt_comreg.u8_FB_lvl ==0)
	{
		table_level = 9;
		tmp_Dh_Local_FB_table = fwif_MEMC_get_Dh_Local_FB_table(table_level);
		tmp_Dh_Local_FB_table->dh_dtl_curve_y2= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_y2;
		tmp_Dh_Local_FB_table->dh_dtl_curve_x1= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x1;
		tmp_Dh_Local_FB_table->dh_dtl_curve_x2= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x2;
	}
	else if(s_pContext->_output_me_sceneAnalysis.u2_panning_flag == 1)
	{
		table_level = 2;
		tmp_Dh_Local_FB_table = fwif_MEMC_get_Dh_Local_FB_table(table_level);
		tmp_Dh_Local_FB_table->dh_dtl_curve_x1= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x1;
		tmp_Dh_Local_FB_table->dh_dtl_curve_x2= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x2;
	}
	else if(s_pContext->_output_me_sceneAnalysis.u2_panning_flag == 3)
	{
		table_level = 10;
		tmp_Dh_Local_FB_table = fwif_MEMC_get_Dh_Local_FB_table(table_level);
		tmp_Dh_Local_FB_table->dh_dtl_curve_x1= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x1;
		tmp_Dh_Local_FB_table->dh_dtl_curve_x2= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x2;
	}
	else
	{
		table_level = 0;
		tmp_Dh_Local_FB_table = fwif_MEMC_get_Dh_Local_FB_table(table_level);
		tmp_Dh_Local_FB_table->dh_dtl_curve_y1= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_y1;
		tmp_Dh_Local_FB_table->dh_dtl_curve_y2= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_y2;
		tmp_Dh_Local_FB_table->dh_dtl_curve_x1= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x1;
		tmp_Dh_Local_FB_table->dh_dtl_curve_x2= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x2;
	}


#if 1
//++YE Test for LFB/RFB level ctrl


if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_Test_0_flag ==0)
{

  if(s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_LFB_ctrl_level ==0)
  	{
			table_level =  11;
			tmp_Dh_Local_FB_table = fwif_MEMC_get_Dh_Local_FB_table(table_level);
			//if(u32_ID_Log_en==1)
			//rtd_pr_memc_info("table_level =  11");
		
  	}
 else  if(s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_LFB_ctrl_level ==1)
  	{
			table_level =  12;
			tmp_Dh_Local_FB_table = fwif_MEMC_get_Dh_Local_FB_table(table_level);
			//if(u32_ID_Log_en==1)
			//rtd_pr_memc_info("table_level =  12");
			
  	}
 else  if(s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_LFB_ctrl_level ==2)
  	{
			table_level =  13;
			tmp_Dh_Local_FB_table = fwif_MEMC_get_Dh_Local_FB_table(table_level);
			//if(u32_ID_Log_en==1)
			//rtd_pr_memc_info("table_level =  13");

  	}
 else  if(s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_LFB_ctrl_level ==3)
  	{
			table_level =  14;
			tmp_Dh_Local_FB_table = fwif_MEMC_get_Dh_Local_FB_table(table_level);
			//if(u32_ID_Log_en==1)			
			//rtd_pr_memc_info("table_level =  14");
			
  	}
 else  if(s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_LFB_ctrl_level ==4)
  	{
			table_level =  15;
			tmp_Dh_Local_FB_table = fwif_MEMC_get_Dh_Local_FB_table(table_level);
			//if(u32_ID_Log_en==1)
			//rtd_pr_memc_info("table_level =  15");
			
  	}
 else  if(s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_LFB_ctrl_level ==5) // it's ori default value
  	{
			table_level =  16;
			tmp_Dh_Local_FB_table = fwif_MEMC_get_Dh_Local_FB_table(table_level);
			//if(u32_ID_Log_en==1)
			//rtd_pr_memc_info("table_level =  16");
			
  	}
  else  if(s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_LFB_ctrl_level ==6) // it's ori default value
  	{
			table_level =  17;
			tmp_Dh_Local_FB_table = fwif_MEMC_get_Dh_Local_FB_table(table_level);
			//if(u32_ID_Log_en==1)
			//rtd_pr_memc_info("table_level =  16");
			
  	}
   else  if(s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_LFB_ctrl_level ==7) // it's ori default value
  	{
			table_level =  18;
			tmp_Dh_Local_FB_table = fwif_MEMC_get_Dh_Local_FB_table(table_level);
			//if(u32_ID_Log_en==1)
			//rtd_pr_memc_info("table_level =  16");
			
  	}
    else  if(s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_LFB_ctrl_level ==8) // it's ori default value
  	{
			table_level =  19;
			tmp_Dh_Local_FB_table = fwif_MEMC_get_Dh_Local_FB_table(table_level);
			//if(u32_ID_Log_en==1)
			//rtd_pr_memc_info("table_level =  16");
			
  	}
 else  if(s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_LFB_ctrl_level ==9) // it's ori default value
  	{
			table_level =  20;
			tmp_Dh_Local_FB_table = fwif_MEMC_get_Dh_Local_FB_table(table_level);
			//if(u32_ID_Log_en==1)
			//rtd_pr_memc_info("table_level =  16");
			
  	}
 else  if(s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_LFB_ctrl_level ==10) // it's ori default value
  	{
			table_level =  21;
			tmp_Dh_Local_FB_table = fwif_MEMC_get_Dh_Local_FB_table(table_level);
			//if(u32_ID_Log_en==1)
			//rtd_pr_memc_info("table_level =  16");
			
  	}
	
	if(u32_ID_Log_en==1)
		rtd_pr_memc_info("[MEMC_YE_LEVEL] ,u8_ES_Test_0_flag =%d,rFB_LFB_ctrl_level=%d, table_level=%d,\n"
		,s_pContext->_output_bRMV_rFB_ctrl.u8_ES_Test_0_flag ,s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_LFB_ctrl_level,table_level);


}



//--YE Test for LFB/RFB level ctrl
#endif



      //if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag !=1) //YE Test Edge Shaking
      if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_DEHALO_flag!=1) //YE Test 20200521
      	{
		Dh_Local_FB_wrtRegister(tmp_Dh_Local_FB_table);
      	}
}
VOID dynamicME_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	if(pParam->u1_dynamicME_en==0)
		return;

	//const _PQLCONTEXT *s_pContext = GetPQLContext();
	ME_dynSR_wrtAction(pParam,pOutput);

	//ME_dynSAD_Diff_wrtAction(pParam,pOutput);
	#if PQL_DYNAMIC_ADJUST_FIX_ISSUE
	ME_acdcBld_wrtAction(pParam,pOutput);
	ME_cand_wrtAction(pParam,pOutput);
	ME_caddpnt_wrtAction(pParam,pOutput);
	ME_candOfst_wrtAction(pParam,pOutput);
	//ME_gmv_wrtAction(pParam,pOutput);
	ME_ipme_flp_alp_wrtAction(pParam,pOutput);
	ME_mvd_cuv_wrtAction(pParam,pOutput);
	ME_adptpnt_rnd_cuv_wrtAction(pParam,pOutput);
	ME_cost_wrtAction(pParam,pOutput);
	ME_Vbuf_LFSR_updStgh_wrtAction(pParam,pOutput);
	#if (IC_K5LP || IC_K6LP || IC_K8LP)
	ME_DRP_wrtAction(pParam,pOutput);
	#endif
	#endif
}

//#endif

VOID dhLogoProc_BypassLvlCtrl(unsigned char case_level)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	unsigned int dh_logo_bypass[15][5] =
	{
		{0x7,0x7,0x7,0x7,0x1},
		{0x7,0x7,0x7,0x7,0x1},
		{0x7,0x7,0x7,0x7,0x1},
		{0x7,0x7,0x7,0x7,0x1},
		{0x7,0x7,0x7,0x7,0x1},
		{0x7,0x7,0x7,0x7,0x1},
		{0x7,0x7,0x7,0x7,0x1},
		{0x7,0x7,0x7,0x7,0x1},
		{0x7,0x7,0x7,0x7,0x1},
		{0x7,0x7,0x7,0x7,0x1},
		{0x7,0x7,0x7,0x7,0x1},
		{0x7,0x7,0x7,0x7,0x1},
		{0x7,0x7,0x7,0x7,0x1},
		{0x7,0x7,0x7,0x7,0x1},
		{0x7,0x7,0x7,0x7,0x1},
	};

	if(case_level ==0 || case_level ==255)
	{
		dh_logo_bypass[0][0] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[0][0];
		dh_logo_bypass[0][1] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[0][1];
		dh_logo_bypass[0][2] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[0][2];
		dh_logo_bypass[0][3] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[0][3];
		dh_logo_bypass[0][4] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[0][4];

		dh_logo_bypass[1][0] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[1][0];
		dh_logo_bypass[1][1] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[1][1];
		dh_logo_bypass[1][2] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[1][2];
		dh_logo_bypass[1][3] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[1][3];
		dh_logo_bypass[1][4] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[1][4];

		dh_logo_bypass[2][0] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[2][0];
		dh_logo_bypass[2][1] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[2][1];
		dh_logo_bypass[2][2] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[2][2];
		dh_logo_bypass[2][3] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[2][3];
		dh_logo_bypass[2][4] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[2][4];

		dh_logo_bypass[3][0] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[3][0];
		dh_logo_bypass[3][1] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[3][1];
		dh_logo_bypass[3][2] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[3][2];
		dh_logo_bypass[3][3] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[3][3];
		dh_logo_bypass[3][4] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[3][4];

		dh_logo_bypass[4][0] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[4][0];
		dh_logo_bypass[4][1] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[4][1];
		dh_logo_bypass[4][2] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[4][2];
		dh_logo_bypass[4][3] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[4][3];
		dh_logo_bypass[4][4] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[4][4];

		dh_logo_bypass[5][0] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[5][0];
		dh_logo_bypass[5][1] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[5][1];
		dh_logo_bypass[5][2] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[5][2];
		dh_logo_bypass[5][3] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[5][3];
		dh_logo_bypass[5][4] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[5][4];

		dh_logo_bypass[6][0] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[6][0];
		dh_logo_bypass[6][1] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[6][1];
		dh_logo_bypass[6][2] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[6][2];
		dh_logo_bypass[6][3] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[6][3];
		dh_logo_bypass[6][4] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[6][4];

		dh_logo_bypass[7][0] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[7][0];
		dh_logo_bypass[7][1] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[7][1];
		dh_logo_bypass[7][2] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[7][2];
		dh_logo_bypass[7][3] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[7][3];
		dh_logo_bypass[7][4] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[7][4];

		dh_logo_bypass[8][0] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[8][0];
		dh_logo_bypass[8][1] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[8][1];
		dh_logo_bypass[8][2] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[8][2];
		dh_logo_bypass[8][3] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[8][3];
		dh_logo_bypass[8][4] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[8][4];

		dh_logo_bypass[9][0] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[9][0];
		dh_logo_bypass[9][1] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[9][1];
		dh_logo_bypass[9][2] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[9][2];
		dh_logo_bypass[9][3] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[9][3];
		dh_logo_bypass[9][4] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[9][4];

		dh_logo_bypass[10][0] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[10][0];
		dh_logo_bypass[10][1] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[10][1];
		dh_logo_bypass[10][2] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[10][2];
		dh_logo_bypass[10][3] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[10][3];
		dh_logo_bypass[10][4] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[10][4];

		dh_logo_bypass[11][0] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[11][0];
		dh_logo_bypass[11][1] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[11][1];
		dh_logo_bypass[11][2] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[11][2];
		dh_logo_bypass[11][3] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[11][3];
		dh_logo_bypass[11][4] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[11][4];

		dh_logo_bypass[12][0] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[12][0];
		dh_logo_bypass[12][1] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[12][1];
		dh_logo_bypass[12][2] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[12][2];
		dh_logo_bypass[12][3] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[12][3];
		dh_logo_bypass[12][4] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[12][4];

		dh_logo_bypass[13][0] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[13][0];
		dh_logo_bypass[13][1] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[13][1];
		dh_logo_bypass[13][2] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[13][2];
		dh_logo_bypass[13][3] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[13][3];
		dh_logo_bypass[13][4] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[13][4];

		dh_logo_bypass[14][0] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[14][0];
		dh_logo_bypass[14][1] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[14][1];
		dh_logo_bypass[14][2] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[14][2];
		dh_logo_bypass[14][3] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[14][3];
		dh_logo_bypass[14][4] = s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[1][4];
	}

	WriteRegister(KME_DEHALO2_KME_DEHALO2_90_reg, 0, 2, dh_logo_bypass[0][0]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_90_reg, 4, 6, dh_logo_bypass[0][1]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_90_reg, 12, 14, dh_logo_bypass[0][2]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_90_reg, 8, 10, dh_logo_bypass[0][3]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_90_reg, 15, 15, dh_logo_bypass[0][4]);

	WriteRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 1, 3, dh_logo_bypass[0][0]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 4, 6, dh_logo_bypass[0][1]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 10, 12, dh_logo_bypass[0][2]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 7, 9, dh_logo_bypass[0][3]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 0, 0, dh_logo_bypass[0][4]);

	WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 0, 2, dh_logo_bypass[2][0]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 3, 5, dh_logo_bypass[2][1]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 9, 11, dh_logo_bypass[2][2]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 6, 8, dh_logo_bypass[2][3]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 12, 12, dh_logo_bypass[2][4]);

	WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 0, 2, dh_logo_bypass[3][0]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 3, 5, dh_logo_bypass[3][1]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 9, 11, dh_logo_bypass[3][2]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 6, 8, dh_logo_bypass[3][3]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 12, 12, dh_logo_bypass[3][4]);

	WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 16, 18, dh_logo_bypass[4][0]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 19, 21, dh_logo_bypass[4][1]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 25, 27, dh_logo_bypass[4][2]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 22, 24, dh_logo_bypass[4][3]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 28, 28, dh_logo_bypass[4][4]);

	WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 0, 2, dh_logo_bypass[5][0]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 3, 5, dh_logo_bypass[5][1]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 9, 11, dh_logo_bypass[5][2]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 6, 8, dh_logo_bypass[5][3]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 12, 12, dh_logo_bypass[5][4]);

	WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 16, 18, dh_logo_bypass[6][0]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 19, 21, dh_logo_bypass[6][1]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 25, 27, dh_logo_bypass[6][2]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 22, 24, dh_logo_bypass[6][3]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 28, 28, dh_logo_bypass[6][4]);

	WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 0, 2, dh_logo_bypass[7][0]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 3, 5, dh_logo_bypass[7][1]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 9, 11, dh_logo_bypass[7][2]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 6, 8, dh_logo_bypass[7][3]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 12, 12, dh_logo_bypass[7][4]);

	WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 16, 18, dh_logo_bypass[8][0]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 19, 21, dh_logo_bypass[8][1]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 25, 27, dh_logo_bypass[8][2]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 22, 24, dh_logo_bypass[8][3]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 28, 28, dh_logo_bypass[8][4]);

	WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 0, 2, dh_logo_bypass[9][0]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 3, 5, dh_logo_bypass[9][1]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 9, 11, dh_logo_bypass[9][2]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 6, 8, dh_logo_bypass[9][3]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 12, 12, dh_logo_bypass[9][4]);

	WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 16, 18, dh_logo_bypass[10][0]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 19, 21, dh_logo_bypass[10][1]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 25, 27, dh_logo_bypass[10][2]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 22, 24, dh_logo_bypass[10][3]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 28, 28, dh_logo_bypass[10][4]);

	WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 0, 2, dh_logo_bypass[11][0]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 3, 5, dh_logo_bypass[11][1]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 9, 11, dh_logo_bypass[11][2]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 6, 8, dh_logo_bypass[11][3]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 12, 12, dh_logo_bypass[11][4]);

	WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 16, 18, dh_logo_bypass[12][0]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 19, 21, dh_logo_bypass[12][1]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 25, 27, dh_logo_bypass[12][2]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 22, 24, dh_logo_bypass[12][3]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 28, 28, dh_logo_bypass[12][4]);

	WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 0, 2, dh_logo_bypass[13][0]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 3, 5, dh_logo_bypass[13][1]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 9, 11, dh_logo_bypass[13][2]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 6, 8, dh_logo_bypass[13][3]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 12, 12, dh_logo_bypass[13][4]);

	WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 16, 18, dh_logo_bypass[14][0]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 19, 21, dh_logo_bypass[14][1]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 25, 27, dh_logo_bypass[14][2]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 22, 24, dh_logo_bypass[14][3]);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 28, 28, dh_logo_bypass[14][4]);

}


VOID logoDetect_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	static unsigned char u1_pre_dh_logo_bypass_en = 0;
	static unsigned char u1_pre_logo_HaloEnahce_en = 0;
	static unsigned char u1_pre_logo_DynOBME_en = 0;
	unsigned char case_level = 255; /* 255->load defaule */

	pOutput->u1_LG_mb_wrt_en = 0;

	if(pParam->u1_logoCtrl_wrt_en == 0)
		return;

	// dehal logo process related
	if(u1_pre_dh_logo_bypass_en == 0 && pParam->u1_dh_logo_bypass_en == 1)
	{
		case_level = 1;
		dhLogoProc_BypassLvlCtrl(case_level);
		u1_pre_dh_logo_bypass_en = 1;
	}

	if(u1_pre_dh_logo_bypass_en == 1 && pParam->u1_dh_logo_bypass_en == 0)
	{
		case_level = 0;
		dhLogoProc_BypassLvlCtrl(case_level);
		u1_pre_dh_logo_bypass_en = 0;
	}
#if 1

	if(pParam->u1_logo_dhRgnProc_en){

		// region 0 control
		if( s_pContext->_output_FRC_LgDet.u1_logo_PurePanning_status == 1 )
		{
			WriteRegister(KME_DEHALO2_KME_DEHALO2_84_reg, 0, 5, 10);
		}
		else if( s_pContext->_output_FRC_LgDet.u3_logo_dhRgnProc_lvl[0] == 0 )
		{
			//WriteRegister(KME_DEHALO2_KME_DEHALO2_84_reg, 12, 21, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr[0][0]); // rgn0_mvd_sad
			WriteRegister(KME_DEHALO2_KME_DEHALO2_84_reg, 0, 5, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr[2][0]); // rgn0_mvd_mv_d_diff_th
			WriteRegister(KME_DEHALO2_KME_DEHALO2_A0_reg, 0, 7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr[4][0]); // rgn0_tmv_mv_t_th
		}
		else if( s_pContext->_output_FRC_LgDet.u3_logo_dhRgnProc_lvl[0] == 1 )
		{
			//WriteRegister(KME_DEHALO2_KME_DEHALO2_84_reg, 12, 21, 128); // rgn0_mvd_diff_th
			WriteRegister(KME_DEHALO2_KME_DEHALO2_84_reg, 0, 5, 35); // rgn0_mvd_mv_d_diff_th
			WriteRegister(KME_DEHALO2_KME_DEHALO2_A0_reg, 0, 7,  8); // rgn0_tmv_mv_t_th
		}
		else if( s_pContext->_output_FRC_LgDet.u3_logo_dhRgnProc_lvl[0] == 2 )
		{
			//WriteRegister(KME_DEHALO2_KME_DEHALO2_84_reg, 12, 21, 160); // rgn0_mvd_diff_th
			WriteRegister(KME_DEHALO2_KME_DEHALO2_84_reg, 0, 5, 50); // rgn0_mvd_mv_d_diff_th
			WriteRegister(KME_DEHALO2_KME_DEHALO2_A0_reg, 0, 7,  4); // rgn0_tmv_mv_t_th
		}
		else
		{
			//WriteRegister(KME_DEHALO2_KME_DEHALO2_84_reg, 12, 21, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr[0][0]); // rgn0_mvd_sad
			WriteRegister(KME_DEHALO2_KME_DEHALO2_84_reg, 0, 5, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr[2][0]); // rgn0_mvd_mv_d_diff_th
			WriteRegister(KME_DEHALO2_KME_DEHALO2_A0_reg, 0, 7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr[4][0]); // rgn0_tmv_mv_t_th
		}

		// region 1 control
		if( s_pContext->_output_FRC_LgDet.u1_logo_BG_still_status == 1 )
		{
			WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg, 8, 15, 32); // rgn1_tmv_num
			WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg, 0, 7, 4); // rgn1_tmv_mv_t_th					
		}	
		else if( s_pContext->_output_FRC_LgDet.u3_logo_dhRgnProc_lvl[1] == 0 )
		{
			//WriteRegister(KME_DEHALO2_KME_DEHALO2_AC_reg, 12, 21, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr[0][1]); // rgn1_mvd_sad
			//WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg, 16, 25, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr[3][1]); // rgn1_tmv_sad
			WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg, 8, 15, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr[5][1]); // rgn1_tmv_num
			WriteRegister(KME_DEHALO2_KME_DEHALO2_AC_reg, 0, 5, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr[2][1]); // rgn1_mvd_mv_d_diff_th
			WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg, 0, 7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr[4][1]); // rgn1_tmv_mv_t_th
		}
		else if( s_pContext->_output_FRC_LgDet.u3_logo_dhRgnProc_lvl[1] == 1 )
		{
			//WriteRegister(KME_DEHALO2_KME_DEHALO2_AC_reg, 12, 21, 128); // rgn1_mvd_sad
			//WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg, 16, 25, 128); // rgn1_tmv_sad
			//WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg, 8, 15, 51); // rgn1_tmv_num
			WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg, 8, 15, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr[5][1]); // rgn1_tmv_num
			WriteRegister(KME_DEHALO2_KME_DEHALO2_AC_reg, 0, 5, 40); // rgn1_mvd_mv_d_diff_th
			WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg, 0, 7, 8); // rgn1_tmv_mv_t_th
		}
		else if( s_pContext->_output_FRC_LgDet.u3_logo_dhRgnProc_lvl[1] == 2 )
		{
			//WriteRegister(KME_DEHALO2_KME_DEHALO2_AC_reg, 12, 21, 255); // rgn1_mvd_sad
			//WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg, 16, 25, 192); // rgn1_tmv_sad
			//WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg, 8, 15, 17); // rgn1_tmv_num
			WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg, 8, 15, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr[5][1]); // rgn1_tmv_num
			WriteRegister(KME_DEHALO2_KME_DEHALO2_AC_reg, 0, 5, 52); // rgn1_mvd_mv_d_diff_th
			WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg, 0, 7, 4); // rgn1_tmv_mv_t_th
		}
		else
		{
			//WriteRegister(KME_DEHALO2_KME_DEHALO2_AC_reg, 12, 21, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr[0][1]); // rgn1_mvd_sad
			//WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg, 16, 25, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr[3][1]); // rgn1_tmv_sad
			WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg, 8, 15, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr[5][1]); // rgn1_tmv_num
			WriteRegister(KME_DEHALO2_KME_DEHALO2_AC_reg, 0, 5, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr[2][1]); // rgn1_mvd_mv_d_diff_th
			WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg, 0, 7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr[4][1]); // rgn1_tmv_mv_t_th
		}		


		// region 2 control
		if( s_pContext->_output_FRC_LgDet.u1_logo_PurePanning_status == 1 )
		{
			WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 13, 18, 12); // rgn2_mvd_mv_d_diff_th
		}else{
			WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 13, 18, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr[2][2]); // rgn2_mvd_mv_d_diff_th
		}					
		
	}else{	

		// region 0 control
		if( s_pContext->_output_FRC_LgDet.u1_logo_PurePanning_status == 1 )
		{
			WriteRegister(KME_DEHALO2_KME_DEHALO2_84_reg, 0, 5, 10);
		}		
		else
		{
			//WriteRegister(KME_DEHALO2_KME_DEHALO2_84_reg, 12, 21, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr[0][0]); // rgn0_mvd_sad
			WriteRegister(KME_DEHALO2_KME_DEHALO2_84_reg, 0, 5, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr[2][0]); // rgn0_mvd_mv_d_diff_th			
		}

		// region 1 control
		if( s_pContext->_output_FRC_LgDet.u1_logo_BG_still_status == 1 )
		{
			WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg, 8, 15, 32); // rgn1_tmv_num
			WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg, 0, 7, 4); // rgn1_tmv_mv_t_th					
		}	
		
		else
		{
			//WriteRegister(KME_DEHALO2_KME_DEHALO2_AC_reg, 12, 21, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr[0][1]); // rgn1_mvd_sad
			//WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg, 16, 25, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr[3][1]); // rgn1_tmv_sad
			WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg, 8, 15, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr[5][1]); // rgn1_tmv_num			
			WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg, 0, 7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr[4][1]); // rgn1_tmv_mv_t_th
		}		

		// region 2 control
		if( s_pContext->_output_FRC_LgDet.u1_logo_PurePanning_status == 1 )
		{
			WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 13, 18, 12); // rgn2_mvd_mv_d_diff_th
		}else{
			WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 13, 18, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr[2][2]); // rgn2_mvd_mv_d_diff_th
		}		
		
	}
#endif

#ifdef CONFIG_SUPPORT_SLD_BY_MEMC_DEMURA
	if (/*strcmp(webos_strToolOption.eBackLight, "oled") == 0*/ /*SLD_SW_En == 1*/MEMC_Pixel_LOGO_For_SW_SLD == 1)
	{
		if( pParam->u1_logo_pxldfth_dyn_en == 1)
		{
			WriteRegister(KME_LOGO2_KME_LOGO2_00_reg, 0, 7, s_pContext->_output_FRC_LgDet.u8_PxlRgnDfy[0]);
			WriteRegister(KME_LOGO2_KME_LOGO2_00_reg, 8, 15, s_pContext->_output_FRC_LgDet.u8_PxlRgnDfy[1]);
			WriteRegister(KME_LOGO2_KME_LOGO2_00_reg, 16, 23, s_pContext->_output_FRC_LgDet.u8_PxlRgnDfy[2]);
		}
		if (pParam->u1_logo_pxlrgclr_en == 1)
		{
			WriteRegister(KME_LOGO1_KME_LOGO1_E4_reg, 0, 31, s_pContext->_output_FRC_LgDet.u32_PxlRgClrSig);
		}
	}
#endif
	if (s_pContext->_output_FRC_LgDet.u32_logo_closeVar_hold_cnt > 0)
	{
	    WriteRegister(MC2_MC2_98_reg, 8, 8, 0x0);
	}
	else
	{
	    WriteRegister(MC2_MC2_98_reg, 8, 8, s_pContext->_output_FRC_LgDet.DefaultVal.reg_mc_logo_vlpf_en);
	}

	//clear write action
	if (pParam->u1_LGClr_wrt_en == 1)
	{
		//sw ctrl
		//unsigned int logo_clr_idx = s_pContext->_output_FRC_LgDet.u32_blklogo_clr_idx | s_pContext->_output_frc_sceneAnalysis.u32_RgPan_idx | s_pContext->_output_FRC_LgDet.u32_Rg_unstatic_idx;
		unsigned int logo_clr_idx = s_pContext->_output_FRC_LgDet.u32_blklogo_clr_idx;

		// logo hsty clear clear logo
		if ((pParam->u1_LGPixHstyClr_wrt_en == 1 || pParam->u1_LGBlkHstyClr_wrt_en == 1) && pParam->u1_LGHstyClr_ClrLG_wrt_en == 1)
		{
			logo_clr_idx = logo_clr_idx | s_pContext->_output_FRC_LgDet.u32_logo_hsty_clr_lgclr_idx;
		}
#ifdef CONFIG_SUPPORT_SLD_BY_MEMC_DEMURA

		if (/*strcmp(webos_strToolOption.eBackLight, "oled") == 0*//*SLD_SW_En == 1*/MEMC_Pixel_LOGO_For_SW_SLD == 1)
		{
			// Just clear the block logo result by dilaiton threshold setting
			if( logo_clr_idx == 0xffffffff )
			{
				WriteRegister(KME_DEHALO2_KME_DEHALO2_90_reg, 12, 	14, 0);
				WriteRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 10, 	12, 0);
				WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 9, 	11, 0);
				WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 9, 	11, 0);
				WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 25, 	27, 0);
				WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 9, 	11, 0);
				WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 25, 	27, 0);
				WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 9, 	11, 0);
				WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 25, 	27, 0);
				WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 9, 	11, 0);
				WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 25, 	27, 0);
				WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 9, 	11, 0);
				WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 25, 	27, 0);
				WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 9, 	11, 0);
				WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 25, 	27, 0);
			}
			else
			{
				WriteRegister(KME_DEHALO2_KME_DEHALO2_90_reg, 12, 	14, 7);
				WriteRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 10, 	12, 7);
				WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 9, 	11, 7);
				WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 9, 	11, 7);
				WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 25, 	27, 7);
				WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 9, 	11, 7);
				WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 25, 	27, 7);
				WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 9, 	11, 7);
				WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 25, 	27, 7);
				WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 9, 	11, 7);
				WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 25, 	27, 7);
				WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 9, 	11, 7);
				WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 25, 	27, 7);
				WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 9, 	11, 7);
				WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 25, 	27, 7);
			}
		}
		else
		{
			WriteRegister(KME_LOGO1_KME_LOGO1_E0_reg, 30, 30, (1-s_pContext->_output_FRC_LgDet.u1_sw_clr_en_out)); // awlays 0
			WriteRegister(KME_LOGO1_KME_LOGO1_E4_reg, 0, 31, logo_clr_idx);
		}
#else
		WriteRegister(KME_LOGO1_KME_LOGO1_E0_reg, 30, 30, (1-s_pContext->_output_FRC_LgDet.u1_sw_clr_en_out)); // awlays 0
		WriteRegister(KME_LOGO1_KME_LOGO1_E4_reg, 0, 31, logo_clr_idx);
#endif
	}

	if (pParam->u1_LGSC_ctrl_wrt_en == 1)
	{
		if(s_pContext->_output_me_sceneAnalysis.u2_panning_flag ==2)
			WriteRegister(KME_LOGO0_KME_LOGO0_10_reg, 0, 4, 0x1f);
		else
			WriteRegister(KME_LOGO0_KME_LOGO0_10_reg, 0, 4, s_pContext->_output_FRC_LgDet.DefaultVal.reg_km_logo_iir_alpha);
	}
	else
	{
		// pql adjust
		WriteRegister(KME_LOGO0_KME_LOGO0_10_reg, 0, 4, s_pContext->_output_FRC_LgDet.u5_iir_alpha_out);
	}


	if(s_pContext->_output_frc_sceneAnalysis.u1_fadeInOut_logo_clr)
	{
		WriteRegister(MC2_MC2_20_reg, 0, 1, 0x0);
	}
	else if(s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true)  // use force disable mc logo
	{
		WriteRegister(MC2_MC2_20_reg, 0, 1, 0x0);
	}
	else if(s_pContext->_output_frc_sceneAnalysis.u1_Swing_true )  // need refine
	{
		WriteRegister(MC2_MC2_20_reg, 0, 1, 0x0);
	}
	else if (pParam->u1_LG_SC_PtClose_wrt_en)
	{
		if (s_pContext->_output_FRC_LgDet.u1_logo_sc_status == 1)
		{
			WriteRegister(MC2_MC2_20_reg, 0, 1, 0x1);
		}
		else
		{
			WriteRegister(MC2_MC2_20_reg, 0, 1, s_pContext->_output_FRC_LgDet.DefaultVal.reg_mc_logo_en);
		}
	}
	else
	{
		WriteRegister(MC2_MC2_20_reg, 0, 1, s_pContext->_output_FRC_LgDet.DefaultVal.reg_mc_logo_en);
	}

	if(pParam->u1_logo_sc_FastDet_wrt_en == 1)
	{
		if(s_pContext->_output_FRC_LgDet.u1_logo_sc_FastDet_status == 1)
		{
			// right top corner
			WriteRegister(KME_LOGO0_KME_LOGO0_C8_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.blksamethr_a[4]/2);
			WriteRegister(KME_LOGO0_KME_LOGO0_B8_reg,  0,  7, (s_pContext->_output_FRC_LgDet.DefaultVal.blksamethr_h[4]*7)>>4);
			// left bot corner
			//WriteRegister(KME_LOGO0_KME_LOGO0_CC_reg,  16,  23, s_pContext->_output_FRC_LgDet.DefaultVal.blksamethr_a[10]/2);
			//WriteRegister(KME_LOGO0_KME_LOGO0_BC_reg,  16,  23, (s_pContext->_output_FRC_LgDet.DefaultVal.blksamethr_h[10]*7)>>4);
			// mid bot
			//WriteRegister(KME_LOGO0_KME_LOGO0_D0_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.blksamethr_a[12]/2);
			//WriteRegister(KME_LOGO0_KME_LOGO0_C0_reg,  0,  7, (s_pContext->_output_FRC_LgDet.DefaultVal.blksamethr_h[12]*7)>>4);
		}
		else
		{
			// right top corner
			WriteRegister(KME_LOGO0_KME_LOGO0_C8_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.blksamethr_a[4]);
			WriteRegister(KME_LOGO0_KME_LOGO0_B8_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.blksamethr_h[4]);
			// left bot corner
			//WriteRegister(KME_LOGO0_KME_LOGO0_CC_reg,  16,  23, s_pContext->_output_FRC_LgDet.DefaultVal.blksamethr_a[10]);
			//WriteRegister(KME_LOGO0_KME_LOGO0_BC_reg,  16,  23, s_pContext->_output_FRC_LgDet.DefaultVal.blksamethr_h[10]);
			// mid bot
			//WriteRegister(KME_LOGO0_KME_LOGO0_D0_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.blksamethr_a[12]);
			//WriteRegister(KME_LOGO0_KME_LOGO0_C0_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.blksamethr_h[12]);
		}
	}

	// adjust logo threshold to decrease the hsty energy when seek
	if(s_pContext->_output_frc_sceneAnalysis.u8_VOSeek_prt_logo_apply > 0)
	{
		WriteRegister(KME_LOGO0_KME_LOGO0_68_reg, 0, 10, 0x800);
		WriteRegister(KME_LOGO0_KME_LOGO0_EC_reg, 8, 8, 0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_EC_reg, 0, 3, 0xf);
		WriteRegister(KME_LOGO0_KME_LOGO0_EC_reg, 4, 7, 0x0);
	}
	else
	{
		WriteRegister(KME_LOGO0_KME_LOGO0_68_reg, 0, 10, s_pContext->_output_FRC_LgDet.DefaultVal.reg_km_logo_blkgrdsum2_th);
		WriteRegister(KME_LOGO0_KME_LOGO0_EC_reg, 8, 8, s_pContext->_output_FRC_LgDet.DefaultVal.reg_km_logo_blkadphstystep_en);
		WriteRegister(KME_LOGO0_KME_LOGO0_EC_reg, 0, 3, s_pContext->_output_FRC_LgDet.DefaultVal.reg_km_logo_blkhsty_nstep);
		WriteRegister(KME_LOGO0_KME_LOGO0_EC_reg, 4, 7, s_pContext->_output_FRC_LgDet.DefaultVal.reg_km_logo_blkhsty_pstep);
	}

	// merlin6 new algo control	
#if	1
	if(pParam->u1_logo_HaloEnahce_en == 0){
		WriteRegister(KME_DEHALO3_KME_DEHALO3_0C_reg, 12, 12, 0);
		u1_pre_logo_HaloEnahce_en = pParam->u1_logo_HaloEnahce_en;
	}else{

		if( (s_pContext->_output_FRC_LgDet.u1_logo_PurePanning_status==3 || s_pContext->_output_FRC_LgDet.u1_logo_PurePanning_status== 4)&&(s_pContext->_output_FRC_LgDet.u1_logo_NearRim_logo_status==1)){
			WriteRegister(KME_DEHALO3_KME_DEHALO3_0C_reg, 12, 12, 0);
			WriteRegister(MC_MC_18_reg, 17, 20, 3);
			WriteRegister(KME_LOGO2_KME_LOGO2_28_reg, 9, 14, 2);
		}else{
			WriteRegister(KME_DEHALO3_KME_DEHALO3_0C_reg, 12, 12, 1);
			WriteRegister(MC_MC_18_reg, 17, 20, 6);
			WriteRegister(KME_LOGO2_KME_LOGO2_28_reg, 9, 14, s_pContext->_output_FRC_LgDet.DefaultVal.u6_logo_blklogopostdlt_th);
		}
		
	
	}
#else	
	if(u1_pre_logo_HaloEnahce_en == 1 && pParam->u1_logo_HaloEnahce_en == 0)
	{
		WriteRegister(KME_DEHALO3_KME_DEHALO3_0C_reg, 12, 12, 0);
		u1_pre_logo_HaloEnahce_en = 0;
	}
	if(u1_pre_logo_HaloEnahce_en == 0 && pParam->u1_logo_HaloEnahce_en == 1)
	{
		WriteRegister(KME_DEHALO3_KME_DEHALO3_0C_reg, 12, 12, 1);
		u1_pre_logo_HaloEnahce_en = 1;
	}
#endif	

	if(u1_pre_logo_DynOBME_en == 1 && pParam->u1_logo_DynOBME_en == 0)
	{
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_48_reg, 31, 31, 0);
		WriteRegister(KME_ME1_TOP10_LOGO_HALO_reg, 0, 0, 0);
		u1_pre_logo_DynOBME_en = 0;
	}
	if(u1_pre_logo_DynOBME_en == 0 && pParam->u1_logo_DynOBME_en == 1)
	{
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_48_reg, 31, 31, 1);
		WriteRegister(KME_ME1_TOP10_LOGO_HALO_reg, 0, 0, 1);
		u1_pre_logo_DynOBME_en = 1;
	}

}

VOID NearRimDetect_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	unsigned int  logo_cnt_temp = 0, logo_cnt_top = 0;
	signed short u10_gmv_mvy = (s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb);
	signed short u11_gmv_mvx = (s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb);

	mc3_mc_nrea_rim_blend_RBUS mc3_mc_nrea_rim_blend_reg;
	mc3_mc_nrea_rim_blend_h_RBUS mc3_mc_nrea_rim_blend_h_reg;
	mc3_mc_nrea_rim_blend_reg.regValue = rtd_inl(MC3_MC_NREA_RIM_BLEND_reg);
	mc3_mc_nrea_rim_blend_h_reg.regValue = rtd_inl(MC3_MC_NREA_RIM_BLEND_H_reg);


#if 1//logo info: if top region has to many logo detect, top region doesnt count --> this is to fix little shaking of logo on top region
		ReadRegister(KME_LOGO2_KME_LOGO2_80_reg,20,29,&logo_cnt_temp);
		logo_cnt_top += logo_cnt_temp;
		ReadRegister(KME_LOGO2_KME_LOGO2_84_reg,20,29,&logo_cnt_temp);
		logo_cnt_top += logo_cnt_temp;
		ReadRegister(KME_LOGO2_KME_LOGO2_88_reg,20,29,&logo_cnt_temp);
		logo_cnt_top += logo_cnt_temp;
		ReadRegister(KME_LOGO2_KME_LOGO2_8C_reg,20,29,&logo_cnt_temp);
		logo_cnt_top += logo_cnt_temp;
		ReadRegister(KME_LOGO2_KME_LOGO2_90_reg,20,29,&logo_cnt_temp);
		logo_cnt_top += logo_cnt_temp;
		ReadRegister(KME_LOGO2_KME_LOGO2_94_reg,20,29,&logo_cnt_temp);
		logo_cnt_top += logo_cnt_temp;
		ReadRegister(KME_LOGO2_KME_LOGO2_98_reg,20,29,&logo_cnt_temp);
		logo_cnt_top += logo_cnt_temp;
		ReadRegister(KME_LOGO2_KME_LOGO2_9C_reg,20,29,&logo_cnt_temp);
		logo_cnt_top += logo_cnt_temp;
#endif
	if (pParam->u1_mc_near_rim_blending_en == 1)
	{
		if (u11_gmv_mvx > 2*u10_gmv_mvy && _ABS_(u10_gmv_mvy) < 10)
		{
			mc3_mc_nrea_rim_blend_reg.mc_near_rim_blending_det_top_en = 0;
			mc3_mc_nrea_rim_blend_reg.mc_near_rim_blending_det_bot_en = 0;
			mc3_mc_nrea_rim_blend_reg.mc_near_rim_blending_det_lft_en = 1;
			mc3_mc_nrea_rim_blend_reg.mc_near_rim_blending_det_rht_en = 1;
		}
		else if (u10_gmv_mvy > 2*u11_gmv_mvx && _ABS_(u11_gmv_mvx) < 10)
		{
			mc3_mc_nrea_rim_blend_reg.mc_near_rim_blending_det_top_en = 1;
			mc3_mc_nrea_rim_blend_reg.mc_near_rim_blending_det_bot_en = 1;
			mc3_mc_nrea_rim_blend_reg.mc_near_rim_blending_det_lft_en = 0;
			mc3_mc_nrea_rim_blend_reg.mc_near_rim_blending_det_rht_en = 0;
		}
		else
		{
			mc3_mc_nrea_rim_blend_reg.mc_near_rim_blending_det_top_en = 1;
			mc3_mc_nrea_rim_blend_reg.mc_near_rim_blending_det_bot_en = 1;
			mc3_mc_nrea_rim_blend_reg.mc_near_rim_blending_det_lft_en = 1;
			mc3_mc_nrea_rim_blend_reg.mc_near_rim_blending_det_rht_en = 1;
		}

		if( (s_pContext->_output_FRC_LgDet.u1_logo_PurePanning_status==3 || s_pContext->_output_FRC_LgDet.u1_logo_PurePanning_status== 4)&&(s_pContext->_output_FRC_LgDet.u1_logo_NearRim_logo_status==1)){
			mc3_mc_nrea_rim_blend_reg.mc_near_rim_blending_en = 0;
			//rtd_pr_memc_info("[YE_MEMC_Logo]  =Case 1= logo_cnt_top=%d ",logo_cnt_top );
			if(logo_cnt_top > 150)
			WriteRegister(MC_MC_18_reg, 25, 25, 1);//YE Test Logo
			
		}
		else if(s_pContext->_output_me_sceneAnalysis.u1_boundary_v_panning_flag == 1){
			// WriteRegister(MC3_MC_NREA_RIM_BLEND_reg_addr, 0, 0, 1);
			mc3_mc_nrea_rim_blend_reg.mc_near_rim_blending_en = 1;
			//rtd_pr_memc_info("[YE_MEMC_Logo] ==Case 2== logo_cnt_top=%d ",logo_cnt_top );
			if(logo_cnt_top > 150)
			WriteRegister(MC_MC_18_reg, 25, 25, 1);//YE Test Logo
			
			
		}
		else
		{
			mc3_mc_nrea_rim_blend_reg.mc_near_rim_blending_en = 1;
			WriteRegister(MC_MC_18_reg, 25, 25, 0);//YE Test Logo
			//rtd_pr_memc_info("[YE_MEMC_Logo] ===Case 3=== logo_cnt_top=%d ",logo_cnt_top );
		}

		if((s_pContext->_output_me_sceneAnalysis.u1_over_searchrange == 1) || (s_pContext->_output_me_sceneAnalysis.u1_vpan_for_nearrim ==1)){
			mc3_mc_nrea_rim_blend_h_reg.mc_near_rim_blending_invld_bypass = 0;
			mc3_mc_nrea_rim_blend_h_reg.mc_near_rim_blending_duplicate_sel = 0;
		}else{
			mc3_mc_nrea_rim_blend_h_reg.mc_near_rim_blending_invld_bypass = 1;
			mc3_mc_nrea_rim_blend_h_reg.mc_near_rim_blending_duplicate_sel = 2;
		}

		// rtd_pr_memc_notice("[MEMC] VPanning: %d\n", s_pContext->_output_me_sceneAnalysis.u1_boundary_h_panning_flag);
		rtd_outl(MC3_MC_NREA_RIM_BLEND_reg, mc3_mc_nrea_rim_blend_reg.regValue);
		rtd_outl(MC3_MC_NREA_RIM_BLEND_H_reg, mc3_mc_nrea_rim_blend_h_reg.regValue);
	}

}

VOID Mc2_var_lpf_en_wrtRegister(unsigned char value)
{
	WriteRegister(MC2_MC2_50_reg,0,1, value);
}


VOID Mc2_var_lpf_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	_PQLPARAMETER *s_pParam       = GetPQLParameter();
	PQL_INPUT_FRAME_RATE in_fmRate = s_pContext->_external_data._input_frameRate;
	unsigned char  u8_cadence_Id = s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL];
	unsigned char u1_ori_reg_mc_var_lpf_en = 0x3;

	if(s_pParam->_param_frc_sceneAnalysis.u1_mc2_var_lpf_wrt_en == 0) // 0906,lisa,temporal use, can be change if sync all dynamicPQ from K2
		return;


	if(s_pContext->_external_data.u8_InputVSXral_FrameRepeatMode > 0)
	{
		u1_ori_reg_mc_var_lpf_en= 0x0;
	}
	else if(s_pContext->_output_frc_sceneAnalysis.u8_HDR_PicChg_prt_apply > 0)
	{
		u1_ori_reg_mc_var_lpf_en= 0x0;
	}
	else if(s_pContext->_output_frc_sceneAnalysis.u8_OSDBrightnessBigChg_prt_apply > 0)
	{
		u1_ori_reg_mc_var_lpf_en= 0x0;
	}
	else if(s_pContext->_output_frc_sceneAnalysis.u8_OSDContrastBigChg_prt_apply> 0)
	{
		u1_ori_reg_mc_var_lpf_en= 0x0;
	}
	else 	if(s_pContext->_output_filmDetectctrl.u8_flbk_lvl_cad_cnt > 0)
	{
		u1_ori_reg_mc_var_lpf_en= 0x0;
	}
	else if(s_pContext->_external_info.u16_MEMCMode == 3)  // MEMC OFF
	{
		u1_ori_reg_mc_var_lpf_en= 0x0;
	}
	else if((in_fmRate == _PQL_IN_60HZ || in_fmRate == _PQL_IN_50HZ ) && u8_cadence_Id == _CAD_VIDEO)
	{
		u1_ori_reg_mc_var_lpf_en= 0x0;
	}
	else if(s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true >= 1  && s_pContext->_output_frc_sceneAnalysis.u1_still_frame == 0)
	{
		u1_ori_reg_mc_var_lpf_en= 0x0;
	}
	else if(s_pContext->_output_me_sceneAnalysis.u2_panning_flag ==2)  // pure slow pan
	{
		u1_ori_reg_mc_var_lpf_en= 0x0;
	}
	else if(s_pContext->_output_frc_sceneAnalysis.u1_Swing_true )
	{
		u1_ori_reg_mc_var_lpf_en= 0x0;
	}
	else
	{
		if(s_pContext->_external_data._output_mode == _PQL_OUT_VIDEO_MODE)//sync from Mid_Mode_SetInOutMode() setting
			u1_ori_reg_mc_var_lpf_en = 0x3;
		if(s_pContext->_external_data._output_mode == _PQL_OUT_PC_MODE)
			u1_ori_reg_mc_var_lpf_en = 0x0;
	}

	Mc2_var_lpf_en_wrtRegister(u1_ori_reg_mc_var_lpf_en);

}

VOID MC_Unsharp_Mask_wrtRegister(unsigned char mc_poly_intp_en, unsigned char mc_mvd_intp_en)
{
	WriteRegister(MC_MC_40_reg,0,1, mc_poly_intp_en);
	WriteRegister(MC_MC_40_reg,6,7, mc_mvd_intp_en);
}


VOID MC_Unsharp_Mask_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	//_PQLPARAMETER *s_pParam       = GetPQLParameter();
	PQL_INPUT_FRAME_RATE in_fmRate = s_pContext->_external_data._input_frameRate;
	unsigned char  u8_cadence_Id = s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL];
	static  unsigned int u5_Unsharp_Mask_holdfrm =0x0;
	unsigned char mc_poly_intp_en=0x3;
	unsigned char mc_mvd_intp_en=0x3;

	if(pParam->u1_Unsharp_Mask_wrt_en== 0)//d5e4[10]
		return;

	if(s_pContext->_external_data._output_mode == _PQL_OUT_VIDEO_MODE)
	{
		if(s_pContext->_output_me_sceneAnalysis.u2_panning_flag || s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true)
		{
			mc_poly_intp_en = 0x0;
			mc_mvd_intp_en =0x0;
			u5_Unsharp_Mask_holdfrm=0X10;
		}
		else if((in_fmRate == _PQL_IN_60HZ || in_fmRate == _PQL_IN_50HZ ) && u8_cadence_Id == _CAD_VIDEO)
		{
			mc_poly_intp_en = 0x0;
			mc_mvd_intp_en =0x0;
			u5_Unsharp_Mask_holdfrm=0X10;
		}
		else if(s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true >= 1  && s_pContext->_output_frc_sceneAnalysis.u1_still_frame == 0)
		{
			mc_poly_intp_en = 0x0;
			mc_mvd_intp_en =0x0;
			u5_Unsharp_Mask_holdfrm=0X10;
		}
		else if(u5_Unsharp_Mask_holdfrm >0)
		{
			u5_Unsharp_Mask_holdfrm--;
		}
		else
		{
			mc_poly_intp_en = 0x3;
			mc_mvd_intp_en =0x3;
		}

		MC_Unsharp_Mask_wrtRegister( mc_poly_intp_en,  mc_mvd_intp_en);

	}
}


static _str_MC_SObj_table  MC_SObj_table[3]={
	{CTRL_N,CTRL_N},	/*0.load default*/
	{0x8,CTRL_N,CTRL_N},		/*1*/
	{0xa,0xa,CTRL_N},		/*2*/
};

_str_MC_SObj_table *fwif_MEMC_get_MC_SObj_table(unsigned char level)
{
//	if (MC_SObj_table == NULL) {
//		return NULL;
//	}
//	if( level==255 || level==0)
	if(level > 2)
		return (_str_MC_SObj_table *)&MC_SObj_table[0];
	else
		return (_str_MC_SObj_table *)&MC_SObj_table[level];
}

VOID   MC_SObj_wrtRegister(_str_MC_SObj_table *MC_SObj_table)
{
	WriteRegister(MC_MC_B8_reg, 12, 15, 	MC_SObj_table->mc_sobj_slop1);
	WriteRegister(MC_MC_BC_reg, 16, 19,	MC_SObj_table->mc_sobj_ymin0);
	WriteRegister(MC_MC_BC_reg, 8, 13, 	MC_SObj_table->mc_sobj_ymin1);
}


VOID MC_SObj_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	//_PQLPARAMETER *s_pParam       = GetPQLParameter();
	unsigned char table_level = 255;/* 255->load defaule */
	_str_MC_SObj_table *tmp_MC_SObj_table = NULL;

	if(pParam->u1_MCSobj_wrt_en== 0)
		return;

	if(s_pContext->_output_me_sceneAnalysis.u2_panning_flag == 2){   // pure slow pan
		table_level = 1;
		tmp_MC_SObj_table = fwif_MEMC_get_MC_SObj_table(table_level);
		tmp_MC_SObj_table->mc_sobj_ymin0= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_mc_sobj_ymin1;
		tmp_MC_SObj_table->mc_sobj_ymin1= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_mc_sobj_slop1;
	}
	#if Pro_tv030
	else if(s_pContext->_output_me_sceneAnalysis.u2_panning_flag == 1){   //  normal pan
		table_level = 2;
		tmp_MC_SObj_table = fwif_MEMC_get_MC_SObj_table(table_level);
		tmp_MC_SObj_table->mc_sobj_ymin1= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_mc_sobj_slop1;
	}
	#endif
	else if(s_pContext->_output_wrt_comreg.u1_MC_SmallObject_type==1)
	{
		table_level = 2;
		tmp_MC_SObj_table = fwif_MEMC_get_MC_SObj_table(table_level);
		tmp_MC_SObj_table->mc_sobj_ymin1= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_mc_sobj_slop1;
	}
	else{
		table_level = 0;
		tmp_MC_SObj_table = fwif_MEMC_get_MC_SObj_table(table_level);

		tmp_MC_SObj_table->mc_sobj_slop1= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_mc_sobj_ymin0;
		tmp_MC_SObj_table->mc_sobj_ymin0= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_mc_sobj_ymin1;
		tmp_MC_SObj_table->mc_sobj_ymin1= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_mc_sobj_slop1;
	}
	MC_SObj_wrtRegister(tmp_MC_SObj_table);
}


static _str_ME2_SObj_table  ME2_SObj_table[5]={
	{CTRL_N,CTRL_N,CTRL_N,CTRL_N},		/*0.load default*/
	{CTRL_N,0x80,CTRL_N,CTRL_N},		/*1*/
	{CTRL_N,CTRL_N,CTRL_N,0x20},			/*2*/
	{CTRL_N,0x10,CTRL_N,0x20},			/*3*/
	{0x1c00,0x380,0x38,0x0},	/*4*/
};

_str_ME2_SObj_table *fwif_MEMC_get_ME2_SObj_table(unsigned char level)
{
//	if (ME2_SObj_table == NULL) {
//		return NULL;
//	}
//	if( level==255 || level==0)
	if(level > 4)
		return (_str_ME2_SObj_table *)&ME2_SObj_table[0];
	else
		return (_str_ME2_SObj_table *)&ME2_SObj_table[level];
}

VOID   ME2_SObj_wrtRegister(_str_ME2_SObj_table *ME2_SObj_table)
{
	WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_D4_reg, 17, 29, 		ME2_SObj_table->me2_sec_small_object_sad_th);
	WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_D8_reg, 0, 9, 		ME2_SObj_table->me2_sec_small_object_sm_mv_th);
	WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_D8_reg, 10, 15,		ME2_SObj_table->me2_sec_small_object_ip_pi_mvdiff_gain);
	WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_D8_reg, 16, 21, 		ME2_SObj_table->me2_sec_small_object_ph_mvdiff_gain);
}

VOID ME2_SObj_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	//_PQLPARAMETER *s_pParam       = GetPQLParameter();
	unsigned char table_level = 255;/* 255->load defaule */
	_str_ME2_SObj_table *tmp_ME2_SObj_table = NULL;

	if(pParam->u1_ME2Sobj_wrt_en== 0)
		return;

	if(s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type==1)  // more me1
	{
		table_level = 1;
		tmp_ME2_SObj_table =fwif_MEMC_get_ME2_SObj_table(table_level);
		tmp_ME2_SObj_table->me2_sec_small_object_sad_th			=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me2_sec_small_object_sad_th;
		tmp_ME2_SObj_table->me2_sec_small_object_ip_pi_mvdiff_gain	=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me2_sec_small_object_ip_pi_mvdiff_gain;
		tmp_ME2_SObj_table->me2_sec_small_object_ph_mvdiff_gain		=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me2_sec_small_object_ph_mvdiff_gain;
	}
	else if(s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type==2)  // more me2
	{
		table_level = 1;
		tmp_ME2_SObj_table =fwif_MEMC_get_ME2_SObj_table(table_level);
		tmp_ME2_SObj_table->me2_sec_small_object_sad_th			=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me2_sec_small_object_sad_th;
		tmp_ME2_SObj_table->me2_sec_small_object_sm_mv_th		=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me2_sec_small_object_sm_mv_th;
		tmp_ME2_SObj_table->me2_sec_small_object_ip_pi_mvdiff_gain	=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me2_sec_small_object_ip_pi_mvdiff_gain;
	}
	else if(s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type==3)  // more me2
	{
		table_level = 1;
		tmp_ME2_SObj_table =fwif_MEMC_get_ME2_SObj_table(table_level);
		tmp_ME2_SObj_table->me2_sec_small_object_sad_th			=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me2_sec_small_object_sad_th;
		tmp_ME2_SObj_table->me2_sec_small_object_ip_pi_mvdiff_gain	=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me2_sec_small_object_ip_pi_mvdiff_gain;
	}
	else if(s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type==4)  // force me1
	{
		table_level = 1;
		tmp_ME2_SObj_table =fwif_MEMC_get_ME2_SObj_table(table_level);
	}
	else{
		table_level = 1;
		tmp_ME2_SObj_table =fwif_MEMC_get_ME2_SObj_table(table_level);
		tmp_ME2_SObj_table->me2_sec_small_object_sad_th			=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me2_sec_small_object_sad_th;
		tmp_ME2_SObj_table->me2_sec_small_object_sm_mv_th			=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me2_sec_small_object_sm_mv_th;
		tmp_ME2_SObj_table->me2_sec_small_object_ip_pi_mvdiff_gain	=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me2_sec_small_object_ip_pi_mvdiff_gain;
		tmp_ME2_SObj_table->me2_sec_small_object_ph_mvdiff_gain		=	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me2_sec_small_object_ph_mvdiff_gain;
	}

	ME2_SObj_wrtRegister(tmp_ME2_SObj_table);

}


VOID MC_blend_alpha_wrtRegister(unsigned int kmc_blend_y_alpha,unsigned int blend_uv_alpha)
{
	WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, kmc_blend_y_alpha);
	WriteRegister(IPPRE_IPPRE_04_reg, 16, 23, blend_uv_alpha);
}

VOID MC_blend_alpha_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	static  unsigned int u32_debounce =0x0,u5_blend_alpha_holdfrm =0x0;
	unsigned int u26_aAPL_th,u26_aDTL_th;
	unsigned int u26_aAPLi_rb=s_pContext->_output_read_comreg.u26_me_aAPLi_rb;
	unsigned int u26_aAPLp_rb=s_pContext->_output_read_comreg.u26_me_aAPLp_rb;
	unsigned int u26_aDTL_rb=s_pContext->_output_read_comreg.u25_me_aDTL_rb;
	unsigned int u32_rimRatio = (s_pContext->_output_rimctrl.u32_rimRatio == 0) ? 128 : s_pContext->_output_rimctrl.u32_rimRatio;
	unsigned int tmp_kmc_blend_y_alpha=0;
	unsigned int tmp_blend_uv_alpha =0;

	if(u32_debounce ==0x0)
	{
		u26_aAPL_th = 0x140000;
		u26_aDTL_th = 0x60000;
	}
	else
	{
		u26_aAPL_th = 0x1e0000;  // 0x140000 *3/2
		u26_aDTL_th = 0x40000;  // 0x60000 *2/3
	}
	u26_aAPL_th = (u26_aAPL_th * u32_rimRatio) >> 7;

	if(((u26_aAPLi_rb+ u26_aAPLp_rb)/2  < (u26_aAPL_th+u32_debounce)) && (u26_aDTL_rb >u26_aDTL_th))
	{
		tmp_kmc_blend_y_alpha = 0xf0;
		tmp_blend_uv_alpha	= 0x30;

		u32_debounce = 0x30000;
		u5_blend_alpha_holdfrm =0x10;
	}
	else if(u5_blend_alpha_holdfrm >0)
	{
		u5_blend_alpha_holdfrm = u5_blend_alpha_holdfrm -1;
	}
	else
	{
		u32_debounce = 0x0;
		u5_blend_alpha_holdfrm = 0x0;

		tmp_kmc_blend_y_alpha = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha;
		tmp_blend_uv_alpha	= s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_uv_alpha;
	}

	MC_blend_alpha_wrtRegister(tmp_kmc_blend_y_alpha,tmp_blend_uv_alpha);

}

#if 1
#define MEMC_PERFORMANCE_SETTING_TABLE_MAX 256
#define MEMC_PERFORMANCE_SETTING_ITEM_MAX 45
const unsigned int MEMC_setting_Table[MEMC_PERFORMANCE_SETTING_TABLE_MAX][MEMC_PERFORMANCE_SETTING_ITEM_MAX][MEMC_PERFORMANCE_SETTING_MAX] = 
{
	//#0
	//do nothing -> return all values
	{},

	//#1
	/*casino_royale_halo_1_09_00.mp4*/
	/*THE MATRIX RELOADED.4khdr_5_short.mp4*/
	{
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,   15},//reg_vbuf_pi_1st_lfsr_mask0_x
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,   15},//reg_vbuf_pi_1st_lfsr_mask0_y
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8, 11,  15},//reg_vbuf_pi_1st_lfsr_mask1_x
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 15},//reg_vbuf_pi_1st_lfsr_mask1_y
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 21, 15},//reg_vbuf_pi_1st_lfsr_mask2_x
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 15},//reg_vbuf_pi_1st_lfsr_mask2_y
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 15},//reg_vbuf_pi_1st_lfsr_mask3_x
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, 15},//reg_vbuf_pi_1st_lfsr_mask3_y
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,   15},//reg_vbuf_ip_1st_lfsr_mask0_x
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,   15},//reg_vbuf_ip_1st_lfsr_mask0_y
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8, 11,  15},//reg_vbuf_ip_1st_lfsr_mask1_x
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 15},//reg_vbuf_ip_1st_lfsr_mask1_y
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 21, 15},//reg_vbuf_ip_1st_lfsr_mask2_x
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 15},//reg_vbuf_ip_1st_lfsr_mask2_y
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 15},//reg_vbuf_ip_1st_lfsr_mask3_x
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 15},//reg_vbuf_ip_1st_lfsr_mask3_y
	},

	//#2
	/*fusion_PW_120Hz_l_Robot_(SD_NTSC)_02.mpg*/
	{
		{KME_DEHALO_KME_DEHALO_10_reg, 0, 0, 0},//FRC_TOP__KME_DEHALO__reg_dh_en
		{KME_DEHALO_KME_DEHALO_FC_reg, 23, 23, 1},//FRC_TOP__KME_DEHALO__dh_dtl_gain_en
		{KME_DEHALO_KME_DEHALO_FC_reg, 20, 22, 5},//FRC_TOP__KME_DEHALO__dh_fb_dtl_shift
		{KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 128},//FRC_TOP__KME_DEHALO__reg_dh_dtl_curve_y1
	},

	//#3
	/*DNM_cityscapes 720p60*/
	{
		{MC_MC_28_reg, 14, 14, 1},
		{MC_MC_28_reg, 15, 22, 0},
		{KME_ME1_TOP1_ME1_COMMON1_10_reg, 8, 17, 32},
		{KME_ME1_TOP1_ME1_COMMON1_10_reg,18, 27, 32},
		{KME_ME1_TOP1_ME1_COMMON1_12_reg, 16, 31, 65532},
		{KME_TOP_KME_TOP_04_reg, 29, 29, 0},
	},

	//#4
	/*4K.znds.com]ShutUpAndPlay.Berlin.2014.2160p.HDTV.HEVC-jTV-50p-3840x2160*/
	//0:40-0:43
	{
		{KME_DEHALO_KME_DEHALO_FC_reg, 23, 23, 0},
	},

	//#5
	/*3.QTEK_6.5PPF_00081*/
	/*monosco4K_Hscroll_10pix_50Hz_long_50p-3840*2160*/
	{
		{KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 3, 3, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_cand2_en
		{KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_cand3_en
		{KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_cand4_en
		{KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 6, 6, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_cand5_en
		{KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 7, 7, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_cand6_en
		{KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 8, 8, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_cand7_en

		{KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 2, 2, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_cand2_en
		{KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_cand3_en
		{KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_cand4_en
		{KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 5, 5, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_cand5_en
		{KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 6, 6, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_cand6_en
		{KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 7, 7, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_cand7_en
		{0xb802b500, 0, 0, 0},//  sharpness peaking
		{0xb802b950, 0, 0, 0},//  sharpness dlti
		{0xb8023c00, 0, 0, 0},//  Multiband peaking
		{0xb8023c30, 0, 0, 1},//  Multiband peaking DB apply
	},

	//#6
	/*Valley of fire_breakup.mp4*/
	{
		{KME_VBUF_TOP_KME_VBUF_TOP_10_reg,15,15,1},//reg_vbuf_ip_1st_gmv_en
		{KME_VBUF_TOP_KME_VBUF_TOP_10_reg,16,16,1},//reg_vbuf_ip_1st_zmv_en
		{KME_VBUF_TOP_KME_VBUF_TOP_14_reg,15,15,1},//reg_vbuf_pi_1st_gmv_en
		{KME_VBUF_TOP_KME_VBUF_TOP_14_reg,16,16,1},//reg_vbuf_pi_1st_zmv_en
		{KME_ME1_TOP2_KME_ME1_TOP2_34_reg,0,0,0},//FRC_TOP__KME_ME1_TOP2__me1_acdc_pk_en
		{KME_ME2_CALC0_KME_ME2_CALC0_A4_reg,9,9,0},//FRC_TOP__KME_ME2_CALC0__me2_acdc_pk_en
	},

	//#7
	/*THE_ECLIPSE.fhd24_01.mkv*/
	{
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,   1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask0_x
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,   1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask0_y
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8, 11,  1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask1_x
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask1_y
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 21, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask2_x
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask2_y
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask3_x
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask3_y
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,   1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask0_x
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,   1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask0_y
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8, 11,  1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask1_x
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask1_y
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 21, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask2_x
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask2_y
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask3_x
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask3_y
	},

	//#8
	/*Midway.4khdr_13_Propeller*/
	{
		{MC2_MC2_20_reg, 0, 1, 0},//FRC_TOP__MC2__reg_mc_logo_en
		{KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x32},//FRC_TOP__KME_DEHALO__reg_dh_dtl_curve_y1
		{KME_DEHALO_KME_DEHALO_FC_reg, 20, 22, 6},//FRC_TOP__KME_DEHALO__dh_fb_dtl_shift
	},

	//#9
	////Art of Flight 00:02:28
	//Art of Flight  2:23
	//Art of Flight 00:34 /2:23
	{
		{KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_cand3_en
		{KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_cand4_en
		{KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 6, 6, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_cand5_en
		{KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 7, 7, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_cand6_en
		{KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 12, 12, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_update1_en
		{KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 13, 13, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_update2_en
		{KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_40_reg, 13, 16, 1},//FRC_TOP__KME_ME2_VBUF_TOP__reg_kme_me2_vbuf_lfsr_y_mask
		{KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_40_reg, 9, 12, 1},//FRC_TOP__KME_ME2_VBUF_TOP__reg_kme_me2_vbuf_lfsr_x_mask
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,   1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask0_x
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,   1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask0_y
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8, 11,  1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask1_x
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask1_y
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 21, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask2_x
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask2_y
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask3_x
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask3_y
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,   1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask0_x
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,   1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask0_y
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8, 11,  1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask1_x
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask1_y
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 21, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask2_x
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask2_y
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask3_x
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask3_y
	},

	//#10
	//Montage_M_ROT-720p50.mpg
	{
		//{FRC_TOP__PQL_0__pql_patch_dummy_e4_ADDR,7,7, 0},//Merlin7 does not have it
		{KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0},//FRC_TOP__KME_DEHALO__reg_dh_dtl_curve_y1
		{KME_DEHALO_KME_DEHALO_FC_reg,7,0, 0x00},//FRC_TOP__KME_DEHALO__reg_dh_dtl_curve_y2_ADDR
		{KMC_BI_bi_top_01_reg,2,3, 0},//FRC_TOP__KMC_BI__bi_fb_mode_ADDR
		//{FRC_TOP__PQL_1__pql_wr_dhclose_wrt_en_ADDR,4,4, 0},//Merlin7 does not have it
		{KME_DEHALO_KME_DEHALO_BC_reg,22,17, 0x3f},//FRC_TOP__KME_DEHALO__reg_dh_pst_occl_type_thr_ADDR
		{MC_MC_B8_reg,11,11, 0},//FRC_TOP__MC__reg_mc_sobj_en_ADDR
		{MC_MC_B4_reg,23,16, 0x00},//FRC_TOP__MC__reg_mc_bld_mvd_y_min_ADDR
		{MC_MC_B4_reg,31,24, 0x00},//FRC_TOP__MC__reg_mc_bld_mvd_y_min_ADDR
		//{KIWI_REG(FRC_TOP__PQL_1__pql_fb_ctrl_en_ADDR), 0},//Merlin7 do not have it
		{MC_MC_B0_reg, 20 ,27, 0},//FRC_TOP__MC__reg_mc_bld_flbk_lvl
		{MC_MC_28_reg, 14, 14, 1},//FRC_TOP__MC__reg_mc_fb_lvl_force_en
		{MC_MC_28_reg, 15, 15, 0},//FRC_TOP__MC__reg_mc_fb_lvl_force_value
	},

	//#11
	/*Passion Cocoa Milk Buns*/ // standing at the middle
	{
		{KME_DEHALO_KME_DEHALO_FC_reg, 23, 23, 1},//FRC_TOP__KME_DEHALO__dh_dtl_gain_en
		{KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_cand3_en
		{KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_cand4_en
		{KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 6, 6, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_cand5_en
		{KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 7, 7, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_cand6_en
		{KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 12, 12, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_update1_en
		{KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 13, 13, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_update2_en
		{KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_40_reg, 13, 16, 1},//FRC_TOP__KME_ME2_VBUF_TOP__reg_kme_me2_vbuf_lfsr_y_mask
		{KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_40_reg, 9, 12, 1},//FRC_TOP__KME_ME2_VBUF_TOP__reg_kme_me2_vbuf_lfsr_x_mask
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,   1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask0_x
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,   1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask0_y
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8, 11,  1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask1_x
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask1_y
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 21, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask2_x
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask2_y
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask3_x
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask3_y
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,   1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask0_x
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,   1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask0_y
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8, 11,  1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask1_x
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask1_y
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 21, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask2_x
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask2_y
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask3_x
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask3_y
	},

	//#12
	//S&M saech on a hammock
	{
		{KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_cand3_en
		{KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_cand4_en
		{KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 6, 6, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_cand5_en
		{KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 7, 7, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_cand6_en
		{KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 12, 12, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_update1_en
		{KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 13, 13, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_update2_en
		{KME_ME1_TOP0_KME_ME1_TOP0_2C_reg, 0, 12, 0},//FRC_TOP__KME_ME1_TOP0__reg_me1_ip_cddpnt_gmv
		{KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 15, 15, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_zmv_en
		{KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg, 28, 29, 3},//FRC_TOP__KME_ME2_VBUF_TOP__kme_me2_vbuf_1st_dc_obme_mode
		{KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg, 30, 31, 3},//FRC_TOP__KME_ME2_VBUF_TOP__kme_me2_vbuf_2nd_dc_obme_mode
		{KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg, 21, 22, 0},//FRC_TOP__KME_ME2_VBUF_TOP__kme_me2_vbuf_1st_ac_obme_mode
		{KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg, 23, 24, 0},//FRC_TOP__KME_ME2_VBUF_TOP__kme_me2_vbuf_2nd_ac_obme_mode
		{KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_DC_reg, 2, 2, 0},//FRC_TOP__KME_ME2_VBUF_TOP__kme_me2_vbuf_1st_dc_obme_mode_sel
		{KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_DC_reg, 3, 3, 0},//FRC_TOP__KME_ME2_VBUF_TOP__kme_me2_vbuf_2nd_dc_obme_mode_sel
		{KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 18, 19, 3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_dc_obme_mode
		{KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 20, 21, 3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_dc_obme_mode
		{KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 22, 22, 0},//FRC_TOP__KME_VBUF_TOP__vbuf_ip_dc_obme_mode_sel
		{KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 23, 23, 0},//FRC_TOP__KME_VBUF_TOP__vbuf_pi_dc_obme_mode_sel
		{KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 24, 25, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_ac_obme_mode
		{KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 26, 27, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_ac_obme_mode
	},

	//#13
	/*7.9.09.IDT_HD_H_R_MPG1080P24.mp4*/
	{
		{0xb802b500, 0, 0, 0},//  sharpness peaking
		{0xb802b950, 0, 0, 0},//  sharpness dlti
		{0xb8023c00, 0, 0, 0},//  Multiband peaking
		{0xb8023c30, 0, 0, 1},//  Multiband peaking DB apply
		{KME_DEHALO_KME_DEHALO_70_reg, 4, 4,1},//FRC_TOP__KME_DEHALO__dh_pfv_conf_gen_andor_fb_sel
		{KME_DEHALO_KME_DEHALO_B0_reg, 12, 19,0},//FRC_TOP__KME_DEHALO__reg_dh_occl_gen_sim_thr
		{KME_DEHALO_KME_DEHALO_D4_reg, 6, 11,15},//FRC_TOP__KME_DEHALO__dh_pfv_gen_force_dehalo_level
		{MC2_MC2_50_reg,0,1, 0},//FRC_TOP__MC2__reg_mc_var_lpf_en
		//{FRC_TOP__PQL_0__pql_patch_dummy_e4_ADDR,7,7, 0},//Merlin7 does not have it
		{MC2_MC2_20_reg, 0 ,1, 0},	//FRC_TOP__MC2__reg_mc_logo_en
		//{FRC_TOP__PQL_0__pql_patch_dummy_e4_ADDR,5,5, 0},//Merlin7 does not have it
	},

	//#14
	/*00027.m2ts*/
	{
		{MC2_MC2_50_reg,0,1, 0},//FRC_TOP__MC2__reg_mc_var_lpf_en
	},

	//#15
	/*7.12.02.movZonePlate_AVC1080P60.mp4*/
	/*01.E_CMO-23_98psf.mp4*/
	{
		{MC_MC_28_reg, 14, 14, 1},//FRC_TOP__MC__reg_mc_fb_lvl_force_en
		{MC_MC_28_reg, 15, 15, 0},//FRC_TOP__MC__reg_mc_fb_lvl_force_value
		//{KIWI_REG(FRC_TOP__PQL_1__pql_fb_ctrl_en_ADDR), 0},//Merlin7 do not have it
		{MC_MC_B0_reg, 20, 27, 0},//FRC_TOP__MC__reg_mc_bld_flbk_lvl_ADDR
	},

	//#16
	//youtube: Above NYC-Filmed in 12K
	//playback TCL-8K-Switzerland-24p-20180713.mp4
	{
		{KME_ME1_TOP0_KME_ME1_DRP_reg, 31, 31, 0x1},//FRC_TOP__KME_ME1_TOP0__me1_drp_en_ADDR
	},

	//#17
	//Emperor_offical_trailer  33s
	{
		//	BG repeat detect
		//{FRC_TOP__PQL_0__pql_patch_dummy_e8_ADDR, 11, 11, 1},//Merlin7 does not have it
		//gmvd
		//pOutput->u1_casino_RP_detect_true =1;

		//	lfsr_mask
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,   3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask0_x
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,   3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask0_y
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8, 11,  3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask1_x
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask1_y
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 21, 3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask2_x
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask2_y
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask3_x
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, 3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask3_y
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,   3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask0_x
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,   3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask0_y
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8, 11,  3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask1_x
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask1_y
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 21, 3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask2_x
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask2_y
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask3_x
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask3_y
		// ME1 cddpnt rnd
		{KME_ME1_TOP0_KME_ME1_TOP0_24_reg, 16, 23, 0xff},//FRC_TOP__KME_ME1_TOP0__reg_me1_ip_cddpnt_rnd0_ADDR
		{KME_ME1_TOP0_KME_ME1_TOP0_24_reg, 24, 31, 0xff},//FRC_TOP__KME_ME1_TOP0__reg_me1_ip_cddpnt_rnd1_ADDR
		{KME_ME1_TOP0_KME_ME1_TOP0_28_reg, 0, 7,   0xff},//FRC_TOP__KME_ME1_TOP0__reg_me1_ip_cddpnt_rnd2_ADDR
		{KME_ME1_TOP0_KME_ME1_TOP0_64_reg, 16, 23, 0xff},//FRC_TOP__KME_ME1_TOP0__reg_me1_pi_cddpnt_rnd0_ADDR
		{KME_ME1_TOP0_KME_ME1_TOP0_64_reg, 24, 31, 0xff},//FRC_TOP__KME_ME1_TOP0__reg_me1_pi_cddpnt_rnd1_ADDR
		{KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 0, 7,   0xff},//FRC_TOP__KME_ME1_TOP0__reg_me1_pi_cddpnt_rnd2_ADDR
		// ME1 adptpnt rnd curve
		{KME_ME1_TOP1_ME1_COMMON1_23_reg, 0, 5,  0x3f},//  FRC_TOP__KME_ME1_TOP1__reg_me1_adptpnt_rnd_y1_ADDR
		{KME_ME1_TOP1_ME1_COMMON1_23_reg, 8, 13, 0x3f},// FRC_TOP__KME_ME1_TOP1__reg_me1_adptpnt_rnd_y2_ADDR
		{KME_ME1_TOP1_ME1_COMMON1_23_reg, 14, 19,0x3f},// FRC_TOP__KME_ME1_TOP1__reg_me1_adptpnt_rnd_y3_ADDR
		{KME_ME1_TOP1_ME1_COMMON1_23_reg, 20, 25,0x3f},//FRC_TOP__KME_ME1_TOP1__reg_me1_adptpnt_rnd_y4_ADDR
	},

	//#18
	/*Happy_Gilmore_Longdrive_1080I60*/
	{
		{KME_DEHALO_KME_DEHALO_10_reg, 0, 0, 0},//FRC_TOP__KME_DEHALO__reg_dh_en
		{MC_MC_B4_reg, 16, 23, 255},//FRC_TOP__MC__reg_mc_bld_mvd_y_min
		{MC_MC_B4_reg, 24 ,31, 255},//FRC_TOP__MC__reg_mc_bld_mvd_y_max
		{MC_MC_B8_reg,11,11, 0},//FRC_TOP__MC__reg_mc_sobj_en_ADDR
	},

	//#19
    /*FRC_Spideman_hd_demo_04.mkv*/
	{
		{KME_DEHALO_KME_DEHALO_10_reg, 0, 0, 1},//FRC_TOP__KME_DEHALO__reg_dh_en
		{KME_DEHALO_KME_DEHALO_70_reg, 4, 4,1},//FRC_TOP__KME_DEHALO__dh_pfv_conf_gen_andor_fb_sel
		{KME_DEHALO_KME_DEHALO_B0_reg, 12, 19,0},//FRC_TOP__KME_DEHALO__reg_dh_occl_gen_sim_thr
		{KME_DEHALO_KME_DEHALO_D4_reg, 6, 11,63},//FRC_TOP__KME_DEHALO__dh_pfv_gen_force_dehalo_level
		{KME_DEHALO_KME_DEHALO_D4_reg, 18, 23,63},//FRC_TOP__KME_DEHALO__dh_pfv_gen_force_dehalo_level2
		{MC_MC_28_reg, 14, 14, 1},//FRC_TOP__MC__reg_mc_fb_lvl_force_en
		{MC_MC_28_reg, 15, 15, 95},//FRC_TOP__MC__reg_mc_fb_lvl_force_value
	},

	//#20
	/*14-redbull-santorini-action-25-x265-15-25p-3840x2160*/
	{
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,   15},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask0_x
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,   15},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask0_y
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8, 11,  15},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask1_x
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 15},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask1_y
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 21, 15},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask2_x
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 15},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask2_y
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 15},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask3_x
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, 15},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask3_y
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,   15},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask0_x
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,   15},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask0_y
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8, 11,  15},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask1_x
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 15},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask1_y
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 21, 15},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask2_x
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 15},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask2_y
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 15},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask3_x
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 15},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask3_y
	},

	//#21
	/*7_Harry Potter and The Sorerer's Stone*/
	/*DNM_hrnm 720p60*/ //0:47 rolling-blade girl
	/*07_Montage_P*/ //0:48-0:51 rolling-blade girl
	{
		{0xb802b500, 0, 0, 0},//  sharpness peaking
		{0xb802b950, 0, 0, 0},//  sharpness dlti
		{0xb8023c00, 0, 0, 0},//  Multiband peaking
		{0xb8023c30, 0, 0, 1},//  Multiband peaking DB apply
		{KME_DEHALO_KME_DEHALO_70_reg, 4, 4,1},//FRC_TOP__KME_DEHALO__dh_pfv_conf_gen_andor_fb_sel
		{KME_DEHALO_KME_DEHALO_B0_reg, 12, 19,0},//FRC_TOP__KME_DEHALO__reg_dh_occl_gen_sim_thr
		{KME_DEHALO_KME_DEHALO_D4_reg, 6, 11,63},//FRC_TOP__KME_DEHALO__dh_pfv_gen_force_dehalo_level
		{KME_DEHALO_KME_DEHALO_D4_reg, 18, 23,63},//FRC_TOP__KME_DEHALO__dh_pfv_gen_force_dehalo_level2
	},

	//#22
	/*DNM_hrnm 720p60*/ //walking stick
	{
		{MC_MC_28_reg, 14, 14, 1},//FRC_TOP__MC__reg_mc_fb_lvl_force_en
		{MC_MC_28_reg, 15, 15, 0},//FRC_TOP__MC__reg_mc_fb_lvl_force_value
		{KME_ME1_TOP1_ME1_COMMON1_10_reg, 8, 17, 32},//FRC_TOP__KME_ME1_TOP1__reg_me1_mvd_dgain_y1
		{KME_ME1_TOP1_ME1_COMMON1_10_reg, 18, 27, 32},//FRC_TOP__KME_ME1_TOP1__reg_me1_mvd_dgain_y2
		{KME_ME1_TOP1_ME1_COMMON1_12_reg, 16, 31, 65530},//FRC_TOP__KME_ME1_TOP1__reg_me1_mvd_dgain_slope2
		{KME_TOP_KME_TOP_04_reg, 29, 29, 0},//FRC_TOP__KME_TOP__reg_frame_meander_enable
		{KME_ME1_TOP1_ME1_COMMON1_06_reg, 0, 9, 0},//FRC_TOP__KME_ME1_TOP1__reg_me1_mvd_cuv_x1
		{KME_ME1_TOP1_ME1_COMMON1_06_reg, 10, 19, 40},//FRC_TOP__KME_ME1_TOP1__reg_me1_mvd_cuv_x2
		{KME_ME1_TOP1_ME1_COMMON1_06_reg, 20 ,29, 120},//FRC_TOP__KME_ME1_TOP1__reg_me1_mvd_cuv_x3
		{KME_ME1_TOP1_ME1_COMMON1_07_reg, 0, 9, 0},//FRC_TOP__KME_ME1_TOP1__reg_me1_mvd_cuv_y1
		{KME_ME1_TOP1_ME1_COMMON1_07_reg, 10, 19, 21},//FRC_TOP__KME_ME1_TOP1__reg_me1_mvd_cuv_y2
		{KME_ME1_TOP1_ME1_COMMON1_07_reg, 20 ,29, 32},//FRC_TOP__KME_ME1_TOP1__reg_me1_mvd_cuv_y3
		{KME_ME1_TOP1_ME1_COMMON1_08_reg, 0 ,12, 34},//FRC_TOP__KME_ME1_TOP1__reg_me1_mvd_cuv_slope1
		{KME_ME1_TOP1_ME1_COMMON1_08_reg, 13, 25, 9},//FRC_TOP__KME_ME1_TOP1__reg_me1_mvd_cuv_slope2
		{KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 0, 0, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_cand0_en
		{KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 1, 1, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_cand1_en
		{KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 2, 2, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_cand2_en
		{KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_cand3_en
		{KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_cand4_en
		{KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 5, 5, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_cand5_en
		{KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 6, 6, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_cand6_en
		{KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 7, 7, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_cand7_en
		{KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 8, 8, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_cand8_en
		{KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 9, 9, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_cand9_en
		{KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 10, 10, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_update0_en
		{KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 11, 11, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_update1_en
		{KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 12, 12, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_update2_en
		{KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 13, 13, 0},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_update3_en
		{KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 14, 14, 1},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_gmv_en
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,   3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask0_x
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,   3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask0_y
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8, 11,  3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask1_x
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask1_y
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 21, 3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask2_x
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask2_y
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask3_x
		{KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, 3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_lfsr_mask3_y
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,   3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask0_x
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,   3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask0_y
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8, 11,  3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask1_x
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask1_y
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 21, 3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask2_x
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask2_y
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask3_x
		{KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 3},//FRC_TOP__KME_VBUF_TOP__reg_vbuf_ip_1st_lfsr_mask3_y
	},

	//#23
	/*taxi3_ntsc_n1.m2t*/
	/*The Art of Flight disk: 46"26 antenna broken when scene pans*/
	{
		{KME_DEHALO_KME_DEHALO_10_reg, 0, 0, 0},//FRC_TOP__KME_DEHALO__reg_dh_en
		{MC_MC_28_reg, 14, 14, 1},//FRC_TOP__MC__reg_mc_fb_lvl_force_en
		{MC_MC_28_reg, 15, 15, 0},//FRC_TOP__MC__reg_mc_fb_lvl_force_value
		{KME_ME2_CALC0_KME_ME2_CALC0_D4_reg, 16, 16, 0},//FRC_TOP__KME_ME2_CALC0__reg_kme_me2_sec_small_object_en
		{KME_ME2_CALC0_KME_ME2_CALC0_DC_reg, 0, 0, 0},//FRC_TOP__KME_ME2_CALC0__reg_kme_me2_fst_small_object_en
	},

	//#24
	/*DMN Cityscapes 720p60-breakup.mp4*/
	/*Samsara 54"42:fire black hole*/
	/*Samsara 1"29"14:sun light flash when passing through high frequency*/
	/*Valley of fire_breakup.mp4*/
	/*HQV_text_rot35_MPG1080P24.m2ts*/
	{
		{KME_DEHALO_KME_DEHALO_10_reg, 0, 0, 0},//FRC_TOP__KME_DEHALO__reg_dh_en
		{MC_MC_28_reg, 14, 14, 1},//FRC_TOP__MC__reg_mc_fb_lvl_force_en
		{MC_MC_28_reg, 15, 15, 0},//FRC_TOP__MC__reg_mc_fb_lvl_force_value
		//{FRC_TOP__PQL_0__pql_patch_dummy_e4_ADDR, 29, 28, 0},//Merlin7 does not have it
		{MC2_MC2_20_reg, 0, 2, 0},//logo off
	},

	//#25
	/*Satelite_1920x1080_24_362.avi*/
	/*4K.znds.com]ShutUpAndPlay.Berlin.2014.2160p.HDTV.HEVC-jTV-50p-3840x2160*/
	//0:28-0:29   0:58-1:00   01:46-01:48	 02:33-02:34
	{
		{MC_MC_28_reg, 14, 14, 1},//FRC_TOP__MC__reg_mc_fb_lvl_force_en
		{MC_MC_28_reg, 15, 15, 0},//FRC_TOP__MC__reg_mc_fb_lvl_force_value
	},

	//#26
	/*MI3_17_M2TS*/
	/*TobyFree.com - 8K UHD Test Pattern H.264 MP4 [4320p].mp4*/
	/*1080P-motion-res-test-pattern*/
	/*7.12.02.movZonePlate_AVC1080P60.mp4*/
	/*Transformer.3.fly_men_short*/
	/*avsync_cont-4000*/
	{
		{MC_MC_28_reg, 14, 14, 1},//FRC_TOP__MC__reg_mc_fb_lvl_force_en
		{MC_MC_28_reg, 15, 15, 255},//FRC_TOP__MC__reg_mc_fb_lvl_force_value
	},

	//#27
	/*Martian.4KHDR_4_contrast.mp4*/
	/*Blade Runner.4khdr_06.mp4*/
	/*CAPTAIN_AMERICA__THE_FIRST_AVENGER_4KHDR15.mp4*/
	/*Transformers.4khdr_1.mp4*/
	/*MI_III.c03.short.mp4*/
	{
		{KME_DEHALO_KME_DEHALO_10_reg, 0, 0, 0},//FRC_TOP__KME_DEHALO__reg_dh_en
	},

	//#28
	/*06.Sharks.mp4*/
	/*H_Move_fast(1080_24p)*/
	{
		{KME_DEHALO_KME_DEHALO_10_reg, 0, 0, 0},//FRC_TOP__KME_DEHALO__reg_dh_en
		{MC_MC_28_reg, 14, 14, 1},//FRC_TOP__MC__reg_mc_fb_lvl_force_en
		{MC_MC_28_reg, 15, 15, 0},//FRC_TOP__MC__reg_mc_fb_lvl_force_value
		{IPPRE_IPPRE_04_reg, 4, 11, 100},//FRC_TOP__IPPRE__reg_kmc_blend_y_alpha
		{IPPRE_IPPRE_04_reg, 16, 23, 255},//FRC_TOP__IPPRE__reg_kmc_blend_uv_alpha
	},
};

extern unsigned char Scaler_MEMC_Get_Performance_Checking_Database_index(void);

void MEMC_Performance_Checking_Database(void)
{
	static unsigned int register_return[MEMC_PERFORMANCE_SETTING_ITEM_MAX] = {};
	int i = 0;
	unsigned int check_en = 0;
	unsigned int check_index = Scaler_MEMC_Get_Performance_Checking_Database_index();
	static unsigned int pre_check_index = 0;
	ReadRegister(SOFTWARE1_SOFTWARE1_42_reg, 19, 19, &check_en);	//to clear any other MEMC isuue is not related to the desgin

	if(check_en == 0 || (pre_check_index == 0 && check_index == 0) || (pre_check_index >= MEMC_PERFORMANCE_SETTING_TABLE_MAX) || (check_index >= MEMC_PERFORMANCE_SETTING_TABLE_MAX))
		return;

	//Return value when switching(first time no need to return value)
	if(pre_check_index != check_index && pre_check_index != 0)
	{
		for(i = 0; i < sizeof(MEMC_setting_Table[pre_check_index])>>4; i++) // 4 (MEMC_PERFORMANCE_SETTING_MAX) * 4(unsigned int) = 16
		{
			WriteRegister(MEMC_setting_Table[pre_check_index][i][0], MEMC_setting_Table[pre_check_index][i][1], MEMC_setting_Table[pre_check_index][i][2], register_return[i]);
		}
	}

	for(i = 0; i < sizeof(MEMC_setting_Table[check_index])>>4; i++) // 4 (MEMC_PERFORMANCE_SETTING_MAX) * 4(unsigned int) = 16
	{
		//store the value when switching
		if((pre_check_index != check_index) && (check_index != 0))
		{
			ReadRegister(MEMC_setting_Table[check_index][i][0], MEMC_setting_Table[check_index][i][1], MEMC_setting_Table[check_index][i][2],&register_return[i]);
		}
		//write table value
		if(check_index != 0)
		{
			WriteRegister(MEMC_setting_Table[check_index][i][0], MEMC_setting_Table[check_index][i][1], MEMC_setting_Table[check_index][i][2], MEMC_setting_Table[check_index][i][3]);
		}
	}
	pre_check_index = check_index;
}

#endif
VOID Identification_Pattern_preProcess(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
#if IDENTIFICATION_EN
	Identification_Pattern_preProcess_TV001(pParam, pOutput);
	if(Pro_tv002)
		Identification_Pattern_preProcess_TV002(pParam, pOutput);
	if( vpq_project_id == 0x00060000 || vpqex_project_id == 0x00060000 )
		Identification_Pattern_preProcess_TV006(pParam, pOutput);
	if(Pro_tv010)
		Identification_Pattern_preProcess_TV010(pParam, pOutput);
	if(Pro_tv011)
		Identification_Pattern_preProcess_TV011(pParam, pOutput);
	if(Pro_tv030)
		Identification_Pattern_preProcess_TV030(pParam, pOutput);
	#if 0
	Identification_Pattern_preProcess_TV043(pParam, pOutput);
	#endif
#endif
}

VOID Identification_Pattern_Update(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	static unsigned char u5_blend_holdfrm_preProcess_a[255] = {0};
	static unsigned char nIdentifiedNum_preProcess_a[255] = {0};
	unsigned char u8_Index;

	#if 1 //for YE Test temp test
	static unsigned int u32_ID_Log_en=0;//,ES_Test_Value=0;
	ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 24,  24, &u32_ID_Log_en);//YE Test debug dummy register
	#endif

	if(pParam->u1_Identification_Pattern_en== 0)
		return;
	
	//reset
	pOutput->u1_Pattern_Update_true=0;

     	//TOP_ratio = ((_ABS_( s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_TOP] - s_pContext->_output_rimctrl.u12_out_resolution[_RIM_TOP]))<<10)>>11;
	//BOT_ratio = ((_ABS_DIFF_(s_pContext->_output_rimctrl.u12_out_resolution[_RIM_BOT], s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_BOT]))<<10)>>(9+s_pRimParam->u8_V_Divide);
 //#128
#if 0
    if((scalerVIP_Get_MEMCPatternFlag_Identification(0,124) == TRUE)  || (scalerVIP_Get_MEMCPatternFlag_Identification(0,126) == TRUE) ) 
    {
        
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 19, 23, 0x2);
        //rtd_pr_memc_notice("128in");
        //WriteRegister(MC_MC_B0_reg,20,27,  0);
        //WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,23,23,0);
        //WriteRegister(HARDWARE_HARDWARE_25_reg,0,0,0x1);

        u5_blend_holdfrm_preProcess_a[100] = 15;
        nIdentifiedNum_preProcess_a[100]=124;

//+apply new code
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg,0,7,240);//ip0
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg,8,15,240);	 
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg,16,23,240);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg,24,31,240);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,0,7,240);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,8,15,240);	 
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,16,23,240);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,24,31,240);
		
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg,0,7,240);//pi0
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg,8,15,240);	 
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg,16,23,240);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg,24,31,240);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,0,7,240);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,8,15,240);	 
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,16,23,240);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,24,31,240);
		
		
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_18_reg,1,1,0);	  //ip_adptpnt_gmv_en
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_58_reg,1,1,0);	  //pi_adptpnt_gmv_en
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_2C_reg,0,12,0);    //me1_ip_cddpnt_gmv
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_6C_reg,0,12,0);   //me1_pi_cddpnt_gmv
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0,1);//gmv_sel
//end apply new code

    }
    else if(u5_blend_holdfrm_preProcess_a[100] >0)
    {
        //rtd_pr_memc_notice("128in");

        //WriteRegister(MC_MC_B0_reg,20,27,  0);
        //WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,23,23,0);
        //WriteRegister(HARDWARE_HARDWARE_25_reg,0,0,0x1);
        
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 19, 23, 0x2);
      //  WriteRegister(HARDWARE_HARDWARE_25_reg,0,0,0x1);         
        u5_blend_holdfrm_preProcess_a[100]--;

		//+apply new code
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg,0,7,240);//ip0
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg,8,15,240);	 
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg,16,23,240);
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg,24,31,240);
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,0,7,240);
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,8,15,240);	 
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,16,23,240);
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,24,31,240);
				
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg,0,7,240);//pi0
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg,8,15,240);	 
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg,16,23,240);
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg,24,31,240);
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,0,7,240);
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,8,15,240);	 
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,16,23,240);
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,24,31,240);
				
				
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_18_reg,1,1,0);	  //ip_adptpnt_gmv_en
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_58_reg,1,1,0);	  //pi_adptpnt_gmv_en
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_2C_reg,0,12,0);    //me1_ip_cddpnt_gmv
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_6C_reg,0,12,0);   //me1_pi_cddpnt_gmv
				WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0,1);//gmv_sel
		//end apply new code

    }
    else if(nIdentifiedNum_preProcess_a[100] == 124 )
    {
		//rtd_pr_memc_notice("128out");
		
		//+apply new code
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg,0,7,28);//ip0
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg,8,15,36);	
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg,16,23,29);
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg,24,31,40);
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,0,7,40);
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,8,15,40);	
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,16,23,40);
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,24,31,48);
				
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg,0,7,28);//pi0
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg,8,15,36);	
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg,16,23,29);
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg,24,31,40);
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,0,7,40);
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,8,15,48);	
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,16,23,48);
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,24,31,48);
				
				
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_18_reg,1,1,1);	  //ip_adptpnt_gmv_en
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_58_reg,1,1,1);	  //pi_adptpnt_gmv_en
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_2C_reg,0,12,1);    //me1_ip_cddpnt_gmv
				WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_6C_reg,0,12,1);   //me1_pi_cddpnt_gmv
				WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0,0);//gmv_sel
		//end apply new code
        

    if (pParam->u1_FBLevelWr_en == 1)
	{
		WriteRegister(MC_MC_B0_reg,20,27,  pOutput->u8_FB_lvl);
	}
    else
    {
        WriteRegister(MC_MC_B0_reg,20,27,  0);        
    }        
        dhClose_wrtAction(pParam);        
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 19, 23, 0x1);
        //WriteRegister(HARDWARE_HARDWARE_25_reg,0,0,0x0);
        //  WriteRegister(HARDWARE_HARDWARE_25_reg,0,0,0x0);        
        nIdentifiedNum_preProcess_a[100] = 0;
    }

#endif
//#128 end


//#298  

    if((scalerVIP_Get_MEMCPatternFlag_Identification(0,125) == TRUE)||(scalerVIP_Get_MEMCPatternFlag_Identification(0,108) == TRUE)) 
    {
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x4);
        WriteRegister(HARDWARE_HARDWARE_25_reg,2,2,0x1);  

        u5_blend_holdfrm_preProcess_a[0] = 15;
        nIdentifiedNum_preProcess_a[0]=125;

    }
    else if(u5_blend_holdfrm_preProcess_a[0] >0)
    {
       //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x4);
       WriteRegister(HARDWARE_HARDWARE_25_reg,2,2,0x1);  

      //  WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0xf);
      //  WriteRegister(HARDWARE_HARDWARE_25_reg,0,0,0x1);     
        u5_blend_holdfrm_preProcess_a[0]--;
    }
    else if(nIdentifiedNum_preProcess_a[0] == 125 )
    {

        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x3);
        WriteRegister(HARDWARE_HARDWARE_25_reg,2,2,0x0);  

      //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0xe);
      //  WriteRegister(HARDWARE_HARDWARE_25_reg,0,0,0x0);        
        nIdentifiedNum_preProcess_a[0] = 0;
    }
//#298 end

	//#13m2v 
		if((scalerVIP_Get_MEMCPatternFlag_Identification(0,127) == TRUE)) 
		{
			u5_blend_holdfrm_preProcess_a[1] = 15;
			nIdentifiedNum_preProcess_a[1]=127;
		}
		else if(u5_blend_holdfrm_preProcess_a[1] >0)
		{
			u5_blend_holdfrm_preProcess_a[1]--;
		}
		else if(nIdentifiedNum_preProcess_a[1] == 127 )
		{	 
			nIdentifiedNum_preProcess_a[1] = 0;
		}
	
	//#13m2v end
	
	
	//#339
		if((scalerVIP_Get_MEMCPatternFlag_Identification(0,128) == TRUE)) 
		{
		   // WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x4);
			WriteRegister(HARDWARE_HARDWARE_25_reg,7,7,0x1);  
	
			u5_blend_holdfrm_preProcess_a[2] = 15;
			nIdentifiedNum_preProcess_a[2]=128;
		   // rtd_pr_memc_notice("tulia 339 test\n");
	
		}
		else if(u5_blend_holdfrm_preProcess_a[2] >0)
		{
		 //  WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x4);
		   WriteRegister(HARDWARE_HARDWARE_25_reg,7,7,0x1);  
			u5_blend_holdfrm_preProcess_a[2]--;
		}
		else if(nIdentifiedNum_preProcess_a[2] == 128 )
		{
	
		  //  WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x3);
			WriteRegister(HARDWARE_HARDWARE_25_reg,7,7,0x0);	   
			nIdentifiedNum_preProcess_a[2] = 0;
		}
	
	//#339 end


	//=======================
	//===debug show
	for(u8_Index =0;u8_Index<3;u8_Index++)
	{
		if(nIdentifiedNum_preProcess_a[u8_Index] !=0){			
			
			if(u32_ID_Log_en==1)
				printk(KERN_EMERG "==Pattern_Update===nIdentifiedNum_a>>[%d][%d]   ,\n", u8_Index, nIdentifiedNum_preProcess_a[u8_Index]);
	
			pOutput->u1_Pattern_Update_true=1;
		}
	}
	//===

}

VOID Identification_Pattern_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
#if IDENTIFICATION_EN
	Identification_Pattern_wrtAction_TV001(pParam, pOutput);
	if(Pro_tv002)
		Identification_Pattern_wrtAction_TV002(pParam, pOutput);
	if( vpq_project_id == 0x00060000 || vpqex_project_id == 0x00060000 )
		Identification_Pattern_wrtAction_TV006(pParam, pOutput);
	if(Pro_tv010)
		Identification_Pattern_wrtAction_TV010(pParam, pOutput);	
	if(Pro_tv011)
		Identification_Pattern_wrtAction_TV011(pParam, pOutput);	
	if(Pro_tv030)
		Identification_Pattern_wrtAction_TV030(pParam, pOutput);
	#if 0
	Identification_Pattern_wrtAction_TV043(pParam, pOutput);
	#endif
#endif
}

VOID Dh_MV_Corr_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
    unsigned int u32_RB_val;
	unsigned int u32_en_val;
	unsigned int  u32_oft_val;
	bool rgn_en[32]={0} ;
	int rmvx_sum =0 ;
	int rmvy_sum =0 ;
	int cnt =0 ;
	int rmvx_temp =0 ;
	int rmvy_temp =0 ;
	unsigned int u32_i = 0 ;
	int  sw_mvx  = 0 ;
	int  sw_mvy =0 ;
	int oft =0 ;
	//dehalo new  algo fw auto set
	// 32 regional mv use
	
	// dehalo switch
	unsigned int DehaloFlag = 0; // 0:old 1:new dehalo
	ReadRegister(HARDWARE_HARDWARE_43_reg, 31, 31, &DehaloFlag);
	if(1)
	{
		//ReadRegister(HARDWARE_HARDWARE_25_reg, 0, 31, &u32_en_val);
		u32_en_val = 0x8080 ; 
		ReadRegister(HARDWARE_HARDWARE_24_reg, 0, 7, &u32_oft_val);
		//7seg_0 [0~31]
		rgn_en[0] = u32_en_val & 0x1;
		rgn_en[1] = (u32_en_val >> 1)  & 0x1;
		rgn_en[2] = (u32_en_val >> 2)  & 0x1;
		rgn_en[3] = (u32_en_val >> 3)  & 0x1;
		rgn_en[4] = (u32_en_val >> 4)  & 0x1;
		rgn_en[5] = (u32_en_val >> 5)  & 0x1;
		rgn_en[6] = (u32_en_val >> 6)  & 0x1;
		rgn_en[7] = (u32_en_val >> 7)  & 0x1;
		rgn_en[8] = (u32_en_val >> 8)  & 0x1;
		rgn_en[9] = (u32_en_val >> 9)  & 0x1;
		rgn_en[10] = (u32_en_val >> 10)  & 0x1;
		rgn_en[11] = (u32_en_val >> 11)  & 0x1;
		rgn_en[12] = (u32_en_val >> 12)  & 0x1;
		rgn_en[13] = (u32_en_val >> 13)  & 0x1;
		rgn_en[14] = (u32_en_val >> 14)  & 0x1;
		rgn_en[15] = (u32_en_val >> 15)  & 0x1;
		rgn_en[16] = (u32_en_val >> 16)  & 0x1;
		rgn_en[17] = (u32_en_val >> 17)  & 0x1;
		rgn_en[18] = (u32_en_val >> 18)  & 0x1;
		rgn_en[19] = (u32_en_val >> 19)  & 0x1;
		rgn_en[20] = (u32_en_val >> 20)  & 0x1;
		rgn_en[21] = (u32_en_val >> 21)  & 0x1;
		rgn_en[22] = (u32_en_val >> 22)  & 0x1;
		rgn_en[23] = (u32_en_val >> 23)  & 0x1;
		rgn_en[24] = (u32_en_val >> 24)  & 0x1;
		rgn_en[25] = (u32_en_val >> 25)  & 0x1;
		rgn_en[26] = (u32_en_val >> 26)  & 0x1;
		rgn_en[27] = (u32_en_val >> 27)  & 0x1;
		rgn_en[28] = (u32_en_val >> 28)  & 0x1;
		rgn_en[29] = (u32_en_val >> 29)  & 0x1;
		rgn_en[30] = (u32_en_val >> 20)  & 0x1;
		rgn_en[31] = (u32_en_val >> 31)  & 0x1;

		for (u32_i = 0; u32_i < 32; u32_i ++)
		{
		       if(rgn_en[u32_i] )
		       {
				ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_00_reg + 8 * u32_i, 0, 20, &u32_RB_val);
				rmvx_temp = ((u32_RB_val >> 10) &1) == 0? (u32_RB_val         & 0x7ff) : ((u32_RB_val & 0x7ff) - (1<<11));
				rmvy_temp = ((u32_RB_val >> 20) &1) == 0? ((u32_RB_val >> 11) & 0x3ff) : (((u32_RB_val >> 11) & 0x3ff) - (1<<10));
				cnt++;
				rmvx_sum+=rmvx_temp ;
				rmvy_sum+=rmvy_temp ;
			}
		}
		oft =  ((u32_oft_val >> 7) &1) == 0? (u32_oft_val & 0xff) : ((u32_oft_val & 0xff) - (1<<8));
		if(cnt!=0)
		{
			sw_mvx =( rmvx_sum+(cnt>>1))/cnt ;
		       sw_mvy =( rmvy_sum+(cnt>>1))/cnt ;
			sw_mvx =    sw_mvx - oft ;


			WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg , 8 , 18 , sw_mvx );
			WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg , 19 , 28 , sw_mvy);

		}
		// 7seg_1 [0~]
		//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 0, 10, sw_mvx);
		//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 16, 25, sw_mvy);

	}

}

VOID MEMC_SmallObject_wrtAction(const _PARAM_WRT_COM_REG *pParam)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	kme_me1_top10_pred_mv_cand_1_RBUS kme_me1_top10_pred_mv_cand_1;
	kme_me1_top10_pred_mv_cand_2_RBUS kme_me1_top10_pred_mv_cand_2;
	kme_me1_top10_pred_mv_cand_3_RBUS kme_me1_top10_pred_mv_cand_3;
	static bool u1_SmallObject_en = false;

	if(!pParam->u1_SmallObject_en){//d910[20]
		return;
	}

	if(s_pContext->_output_me_sceneAnalysis.u1_SportScene == true) {
		kme_me1_top10_pred_mv_cand_1.regValue = rtd_inl(KME_ME1_TOP10_PRED_MV_CAND_1_reg);
		kme_me1_top10_pred_mv_cand_2.regValue = rtd_inl(KME_ME1_TOP10_PRED_MV_CAND_2_reg);
		kme_me1_top10_pred_mv_cand_3.regValue = rtd_inl(KME_ME1_TOP10_PRED_MV_CAND_3_reg);
		
		kme_me1_top10_pred_mv_cand_1.me1_apl_maskpred_cand_en = 1;
		kme_me1_top10_pred_mv_cand_1.me1_apl_blk_mode = 0;
		kme_me1_top10_pred_mv_cand_1.me1_apl_mask_up_th = 255;
		kme_me1_top10_pred_mv_cand_1.me1_apl_mask_low_th = 130;
		kme_me1_top10_pred_mv_cand_2.me1_cddpnt_aplpred = 10;
		kme_me1_top10_pred_mv_cand_2.me1_apl_conti_b_th = 7;
		kme_me1_top10_pred_mv_cand_2.me1_apl_conti_s_th = 3;
		kme_me1_top10_pred_mv_cand_3.me1_small_obj_cand2_en = 0;
		kme_me1_top10_pred_mv_cand_3.me1_small_obj_cand3_en = 0;
		
		rtd_outl(KME_ME1_TOP10_PRED_MV_CAND_1_reg, kme_me1_top10_pred_mv_cand_1.regValue);
		rtd_outl(KME_ME1_TOP10_PRED_MV_CAND_2_reg, kme_me1_top10_pred_mv_cand_2.regValue);
		rtd_outl(KME_ME1_TOP10_PRED_MV_CAND_3_reg, kme_me1_top10_pred_mv_cand_3.regValue);

		u1_SmallObject_en = true;
	}
	else if(u1_SmallObject_en) {
		kme_me1_top10_pred_mv_cand_1.regValue = rtd_inl(KME_ME1_TOP10_PRED_MV_CAND_1_reg);
		kme_me1_top10_pred_mv_cand_2.regValue = rtd_inl(KME_ME1_TOP10_PRED_MV_CAND_2_reg);
		kme_me1_top10_pred_mv_cand_3.regValue = rtd_inl(KME_ME1_TOP10_PRED_MV_CAND_3_reg);
		
		kme_me1_top10_pred_mv_cand_1.me1_apl_maskpred_cand_en = 0;
		kme_me1_top10_pred_mv_cand_1.me1_apl_blk_mode = 0;
		kme_me1_top10_pred_mv_cand_1.me1_apl_mask_up_th = 255;
		kme_me1_top10_pred_mv_cand_1.me1_apl_mask_low_th = 140;
		kme_me1_top10_pred_mv_cand_2.me1_cddpnt_aplpred = 256;
		kme_me1_top10_pred_mv_cand_2.me1_apl_conti_b_th = 13;
		kme_me1_top10_pred_mv_cand_2.me1_apl_conti_s_th = 7;
		kme_me1_top10_pred_mv_cand_3.me1_small_obj_cand2_en = 0;
		kme_me1_top10_pred_mv_cand_3.me1_small_obj_cand3_en = 0;
		
		rtd_outl(KME_ME1_TOP10_PRED_MV_CAND_1_reg, kme_me1_top10_pred_mv_cand_1.regValue);
		rtd_outl(KME_ME1_TOP10_PRED_MV_CAND_2_reg, kme_me1_top10_pred_mv_cand_2.regValue);
		rtd_outl(KME_ME1_TOP10_PRED_MV_CAND_3_reg, kme_me1_top10_pred_mv_cand_3.regValue);

		u1_SmallObject_en = false;
	}
}

VOID Pred_MV_Cand_dis(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{ 

	unsigned int u32_RB_val =0 ;
	unsigned int u32_predmv_en =0 ;
	signed short  u11_gmv_mvx =0 ;
	signed short u10_gmv_mvy=0 ;

	ReadRegister(KME_ME1_TOP2_KME_ME1_TOP2_B0_reg, 0, 31, &u32_RB_val);
	ReadRegister(HARDWARE_HARDWARE_24_reg, 9, 9, &u32_predmv_en);  //small obj  pql
	u11_gmv_mvx = ((u32_RB_val >> 10) &1) == 0? (u32_RB_val         & 0x7ff) : ((u32_RB_val         & 0x7ff)  - (1<<11));
	u10_gmv_mvy = ((u32_RB_val >> 20) &1) == 0? ((u32_RB_val >> 11) & 0x3ff) : (((u32_RB_val >> 11) & 0x3ff)  - (1<<10));
       if(u32_predmv_en==1)
       {
		if((( u10_gmv_mvy<-65)||(u10_gmv_mvy>35))/*&&( u11_gmv_mvx<10|| u11_gmv_mvx>-10)*/)
		{    
			WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_1_reg, 28, 28,  0  );  // new algo switch  disable
			//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 0, 3, 1);
		}
		else
		{
			WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_1_reg, 28, 28,  1 );  
			//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 0, 3, 0);
		}
       }

}
VOID Dh_OCCL_Ext_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	unsigned char occl_ext_level = 0;
	unsigned char post_corr_level = 0;
	
	unsigned char occl_ext_level_sel = 0;
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	const _PQLPARAMETER *s_pParam = GetPQLParameter();

	static unsigned char u1_force_prot_off_pre = 0;
	static unsigned int u1_occl_ext_prot_pre[7] = {0};

	const unsigned char ExtLevelSetting[6][20] =
	{
		// post corr : [0] occl cnt [1] mvAB ratio [2] small dtl [3] small apl
		// re-judge  : [4] ucov cnt [5] cov cnt
		// ext range : [6] cov x bg  [7] cov y bg  [8] ucov x bg [9] ucov y bg [10] cov x fg [11] cov y fg [12] ucov x fg [13] ucov y fg
		// err ext   : [14] mvAB diff [15] mvAB GMV [16] curBGMV
		// ext stop  : [17] SAD [18] APL
		// other     : [20] mv sync en
		//			 0  1   2   3        4  5      6  7  8  9 10 11 12 13     14 15 16      17  18 19
		/*0*/ { /**/ 6, 1,  0,  0, /**/ 12,12, /**/0, 0, 0, 0, 0, 0, 0, 0, /**/1, 1, 1, /**/32, 40, 1 },
		/*1*/ { /**/ 7, 1,  5,  1, /**/ 12,12, /**/1, 0, 1, 0, 1, 0, 1, 0, /**/1, 1, 1, /**/32, 40, 1 },
		/*2*/ { /**/ 8, 1, 10,  3, /**/ 12,12, /**/1, 0, 1, 0, 2, 0, 2, 0, /**/1, 1, 1, /**/32, 40, 1 },
		/*3*/ { /**/10, 1, 16,  4, /**/ 12,12, /**/1, 0, 1, 0, 3, 0, 3, 0, /**/1, 1, 1, /**/32, 40, 1 }, // current setting
		/*4*/ { /**/10, 1, 16,  8, /**/ 12,12, /**/1, 0, 1, 0, 4, 0, 4, 0, /**/1, 1, 1, /**/32, 40, 0 },
		/*5*/ { /**/10, 1, 16, 16, /**/ 12,12, /**/1, 0, 1, 0, 5, 0, 5, 0, /**/1, 1, 1, /**/32, 40, 0 }
	};

	if( pParam->u1_OcclExt_wrt_en == 0 )
		return;

	occl_ext_level_sel = s_pContext->_output_me_sceneAnalysis.u3_occl_ext_level;

	// force level
	if( s_pParam->_param_me_sceneAnalysis.u1_occl_ext_force_lvl_en == 1 )
	{
		occl_ext_level = _MIN_(s_pParam->_param_me_sceneAnalysis.u3_occl_ext_force_lvl, 5);
	}
	if( s_pParam->_param_me_sceneAnalysis.u1_dh_postcorr_force_lvl_en == 1 )
	{
		post_corr_level = _MIN_(s_pParam->_param_me_sceneAnalysis.u3_dh_postcorr_force_lvl, 5);
	}

	WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,4,8,  ExtLevelSetting[post_corr_level][0]);
	WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,13,13,  ExtLevelSetting[post_corr_level][1]);
	WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,16,23, ExtLevelSetting[post_corr_level][2]);
	WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,24,31, ExtLevelSetting[post_corr_level][3]);

	WriteRegister(KME_DEHALO2_KME_DEHALO_POSTCORR_reg,0,3, ExtLevelSetting[occl_ext_level][4]);
	WriteRegister(KME_DEHALO2_KME_DEHALO_POSTCORR_reg,4,7,  ExtLevelSetting[occl_ext_level][5]);

	WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,  ExtLevelSetting[occl_ext_level][6]);
	WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,  ExtLevelSetting[occl_ext_level][7]);
	WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11, ExtLevelSetting[occl_ext_level][8]);
	WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14, ExtLevelSetting[occl_ext_level][9]);

	WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,  ExtLevelSetting[occl_ext_level][10]);
	WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,  ExtLevelSetting[occl_ext_level][11]);
	WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26, ExtLevelSetting[occl_ext_level][12]);
	WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29, ExtLevelSetting[occl_ext_level][13]);

	WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,4,5,  ExtLevelSetting[occl_ext_level][14]);
	WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,8,9, ExtLevelSetting[occl_ext_level][15]);
	WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,12,14,   ExtLevelSetting[occl_ext_level][16]);

	WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,8,15, ExtLevelSetting[occl_ext_level][17]);
	WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,24,31, ExtLevelSetting[occl_ext_level][18]);

	WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8, ExtLevelSetting[occl_ext_level][19]);

	if( s_pParam->_param_me_sceneAnalysis.u1_occl_ext_force_protection_off != u1_force_prot_off_pre )
	{
		if( s_pParam->_param_me_sceneAnalysis.u1_occl_ext_force_protection_off != 0 )
		{
			ReadRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,0,0, &u1_occl_ext_prot_pre[0]);
			ReadRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,1,1, &u1_occl_ext_prot_pre[1]);
			ReadRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,2,2, &u1_occl_ext_prot_pre[2]);
			//ReadRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,3,3, &u1_occl_ext_prot_pre[3]);
			//ReadRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,2,2, &u1_occl_ext_prot_pre[4]);
			//ReadRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,1,1, &u1_occl_ext_prot_pre[5]);
			//ReadRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,0,0, &u1_occl_ext_prot_pre[6]);

			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,0,0, 0);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,1,1, 0);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,2,2, 0);
			//WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,3,3, 0);
			//WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,2,2, 0);
			//WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,1,1, 0);
			//WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,0,0, 0);
		}
		else
		{
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,0,0, u1_occl_ext_prot_pre[0]);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,1,1, u1_occl_ext_prot_pre[1]);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,2,2, u1_occl_ext_prot_pre[2]);
			//WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,3,3, u1_occl_ext_prot_pre[3]);
			//WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,2,2, u1_occl_ext_prot_pre[4]);
			//WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,1,1, u1_occl_ext_prot_pre[5]);
			//WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,0,0, u1_occl_ext_prot_pre[6]);
		}

		u1_force_prot_off_pre = s_pParam->_param_me_sceneAnalysis.u1_occl_ext_force_protection_off;
	}
}

VOID ZMV_AdaptPNT_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	_PQLPARAMETER *s_pParam       = GetPQLParameter();
	unsigned int u32_y2 = 0, u32_y1 = 0, u32_x2 = 0, u32_x1 = 0, u32_slope = 0,u32_div=0;

	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_39_reg,0,12, &u32_y2);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_37_reg,16,28, &u32_x2);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_37_reg,0,12, &u32_x1);

	u32_div = _ABS_DIFF_(u32_x2 , u32_x1)==0 ? 1:_ABS_DIFF_(u32_x2 , u32_x1);

	if(s_pParam->_param_frc_sceneAnalysis.u1_zmv_adapt_pnt_en)
	{
		if(s_pContext->_output_frc_sceneAnalysis.u1_zmv_adapt_pnt_GMV_Small)
		{
			u32_y1 = s_pParam->_param_frc_sceneAnalysis.u13_zmv_adapt_pnt_GMV_Small_Y1;
			u32_slope = (_ABS_DIFF_(u32_y2 , u32_y1) << 3) / u32_div;

			WriteRegister(KME_ME1_TOP1_ME1_COMMON1_38_reg,16,28, u32_y1);
			WriteRegister(KME_ME1_TOP1_ME1_COMMON1_40_reg,0,15, u32_slope);
		}
		else
		{
			u32_y1 = s_pParam->_param_frc_sceneAnalysis.u13_zmv_adapt_pnt_GMV_Big_Y1;
			u32_slope = (_ABS_DIFF_(u32_y2 , u32_y1) << 3) / u32_div;

			WriteRegister(KME_ME1_TOP1_ME1_COMMON1_38_reg,16,28, u32_y1);
			WriteRegister(KME_ME1_TOP1_ME1_COMMON1_40_reg,0,15, u32_slope);
		}
	}
}

VOID PFV_CDD_Number_wrtRegister(unsigned char  u5_phfv0_candi_num, unsigned char  u5_phfv1_candi_num)
{
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,0,4, u5_phfv0_candi_num);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,8,12, u5_phfv1_candi_num);
}


VOID PFV_CDD_Number_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	_PQLPARAMETER *s_pParam       = GetPQLParameter();
	unsigned char  u5_phfv0_candi_num = s_pParam->_param_dh_close.u5_phfv0_candi_num;
	unsigned char  u5_phfv1_candi_num = s_pParam->_param_dh_close.u5_phfv1_candi_num;
//	unsigned int u25me_Detail_rb    = s_pContext->_output_read_comreg.u25_me_aDTL_rb;

	if(s_pParam->_param_dh_close.u1_pfv_cdd_num_en)
	{
		if(s_pContext->_output_frc_sceneAnalysis.u8_SC_PFV_CDD_hold_cnt > 0)
		{
			PFV_CDD_Number_wrtRegister(u5_phfv0_candi_num, u5_phfv1_candi_num);
		}
		else
		{
			if((s_pContext->_output_dh_close.u5_dh_sml_rgnAction_holdfrm > 0) &&
			   (s_pContext->_output_frc_sceneAnalysis.u1_fastMotion_det == 0))
			{
				u5_phfv0_candi_num =0;
				u5_phfv1_candi_num =0;
			}
			else
			{
				PFV_CDD_Number_wrtRegister(u5_phfv0_candi_num, u5_phfv1_candi_num);
			}
		}
	}
}

VOID ME_Vst_en_wrtRegister(unsigned char me2_vst_enable)
{
	WriteRegister(KME_ME2_CALC1_KME_ME2_CALC1_VST_CTRL_0_reg,31,31, me2_vst_enable);
}

VOID ME_Vst_boundary_wrtRegister(unsigned char me2_vst_apply_boundary_xsta, unsigned char me2_vst_apply_boundary_xend)
{
	WriteRegister(KME_ME2_CALC1_KME_ME2_CALC1_VST_CTRL_2_reg,16,23, me2_vst_apply_boundary_xsta);
	WriteRegister(KME_ME2_CALC1_KME_ME2_CALC1_VST_CTRL_2_reg,24,31, me2_vst_apply_boundary_xend);
}



VOID ME_Vst_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	static unsigned int u32_print_counter = 0;
	unsigned char me2_vst_enable =0;
	unsigned char me2_vst_apply_boundary_xsta =0;
	unsigned char me2_vst_apply_boundary_xend =0;

	if(pParam->u1_ME_vst_en == 0)
		return;

	me2_vst_apply_boundary_xsta =  s_pContext->_output_me_vst_ctrl.u8_boundarySta;
	me2_vst_apply_boundary_xend	= s_pContext->_output_me_vst_ctrl.u8_boundaryEnd;

	#if (IC_K5LP || IC_K6LP || IC_K8LP)

	u32_print_counter++;
	if(u32_print_counter >= 10)
		u32_print_counter = 0;

	if(s_pContext->_output_me_vst_ctrl.u1_detect == 1)
	{
		me2_vst_enable = 0x1;
		ME_Vst_boundary_wrtRegister(me2_vst_apply_boundary_xsta,  me2_vst_apply_boundary_xend);
	}
	else
	{
		me2_vst_enable = 0x0;
	}

	ME_Vst_en_wrtRegister( me2_vst_enable);

#if 0
	if(u32_print_counter == 0)
		rtd_pr_memc_info("[%s] en = %d, %d to %d\r\n",
			__func__, s_pContext->_output_me_vst_ctrl.u1_detect,
			s_pContext->_output_me_vst_ctrl.u8_boundarySta,
			s_pContext->_output_me_vst_ctrl.u8_boundaryEnd);
#endif

	#endif
}

VOID MC_RMV_blending_ReginaolFB_wrtAction(VOID)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	_PQLPARAMETER *s_pParam       = GetPQLParameter();

	unsigned int u32_wr_val = 0;
	int i=0;

#if (IC_K6LP || IC_K8LP)
		//if(1) //me2 block =2x2
		//{
		//	WriteRegister(KMC_BI_bi_size_ctrl_reg,
		//		0,
		//		0, 0x0); // 0:480x270, 1:240x135
		//}

		if(s_pContext->_output_read_comreg.u8_gmv_blend_en == 1)
		{
			//blending RMV
			for(i=0; i<32; i++)
			{
				u32_wr_val = (s_pContext->_output_bRMV_rFB_ctrl.u10_fb_rgn00_mvy[i] << 11) | s_pContext->_output_bRMV_rFB_ctrl.u11_fb_rgn00_mvx[i];

				WriteRegister(KMC_BI_bi_rmv11_reg + i*4, 0, 31, u32_wr_val);
			}

			//blending alpha
			for(i=0; i<8; i++)
			{
				u32_wr_val = (s_pContext->_output_bRMV_rFB_ctrl.u8_fb_rmv_alpha[i*4+3] << 24) | (s_pContext->_output_bRMV_rFB_ctrl.u8_fb_rmv_alpha[i*4+2] << 16) |
							 (s_pContext->_output_bRMV_rFB_ctrl.u8_fb_rmv_alpha[i*4+1] <<  8) |  s_pContext->_output_bRMV_rFB_ctrl.u8_fb_rmv_alpha[i*4];

				WriteRegister(KMC_BI_bi_beta_00_reg + i*4, 0, 31, u32_wr_val);
			}
		}

		if(s_pContext->_output_read_comreg.u8_regional_fb_en == 1)
		{
			for(i=0; i<8; i++)
			{
				//blending FB_lvl
				u32_wr_val = (s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[i*4+3] << 24) | (s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[i*4+2] << 16) |
							 (s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[i*4+1] <<  8) |  s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[i*4];

				WriteRegister(KMC_BI_bi_fb_00_reg + i*4, 0, 31, u32_wr_val);

				//blending alpha
				u32_wr_val = (s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[i*4+3] << 24) | (s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[i*4+2] << 16) |
							 (s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[i*4+1] <<  8) |  s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[i*4];

				WriteRegister(KMC_BI_bi_gama_00_reg + i*4, 0, 31, u32_wr_val);
			}
			// panning scene
			#if 1  //YE Test it will hold  mc_fb_lvl_force_en and can't cotrol AutoGUI temp mark 
			if ((s_pParam->_param_wrt_comreg.u1_mc_localfb_en == 1)&&(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_Test_0_flag ==1)) //from LearnRPG
			{
				if(s_pContext->_output_bRMV_rFB_ctrl.u1_rFB_panning)
				{
					WriteRegister(MC_MC_28_reg, 14, 14, 1);
					WriteRegister(MC_MC_28_reg, 15, 22, 0);
				}
				else
				{
					WriteRegister(MC_MC_28_reg, 14, 14, 0);
				}
			}
			#endif
		}

#endif

}

VOID Over_SearchRange_wrtAction(VOID)
{
	extern MEMC_APP_DATA_ST gstAppData;

	const _PQLCONTEXT *s_pContext = GetPQLContext();
	_PQLPARAMETER *s_pParam       = GetPQLParameter();

	if((s_pParam->_param_me_sceneAnalysis.u1_over_searchrange_en == 1 || (gstAppData.stMEMCData.enMemcMode == MEMC_USER && gstAppData.stMEMCData.uiDejudderLevel >= 70 && gstAppData.stMEMCData.uiDejudderLevel <= 96)) 
		&& s_pContext->_output_me_sceneAnalysis.u1_over_searchrange == 1)
	{
		//extend limitation of ME1 search range
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_3C_reg,
					  9,
					  15, 0x2d);

		//extend limitation of ME2 search range
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_E0_reg,
		              8,
		              14, 0x2d);

		// Merlin5 LBMC mapping_index enable
		WriteRegister(KPHASE_kphase_7C_reg,
					  4,
					  7, 4);
		WriteRegister(KPHASE_kphase_7C_reg,
					  0,
					  3, 8);
		WriteRegister(KPHASE_kphase_7C_reg,
					  8,
					  8, 1);
	}
	else
	{
		//reset limitation of ME1 search range
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_3C_reg,
					  9,
					  15, 0x2a);

		//reset limitation of ME2 search range
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_E0_reg,
		              8,
		              14, 0x2a);


		// Merlin5 LBMC mapping_index disable
		WriteRegister(KPHASE_kphase_7C_reg,
					  8,
					  8, 0);
	}


}

VOID ME_Median_Filter_wrtAction(VOID)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	_PQLPARAMETER *s_pParam       = GetPQLParameter();
	unsigned int u1_ForceUndoMask, u1_UndoMaskVal, log_en, table_level=0;
	ReadRegister(SOFTWARE3_SOFTWARE3_59_reg, 31, 31, &u1_ForceUndoMask);//0xB809D9EC[31]
	ReadRegister(SOFTWARE3_SOFTWARE3_59_reg, 29, 29, &log_en);//0xB809D9EC[29]

	if(s_pParam->_param_me_sceneAnalysis.u2_medFLT_sel != 0 && s_pContext->_output_me_sceneAnalysis.b_do_ME_medFLT == 1 && s_pContext->_output_wrt_comreg.u1_casino_RP_detect_true!=1)
	{
		//enable ME1 median filter
		WriteRegister(KME_ME1_TOP10_MV_MED_reg,
					  0,
					  0, 0x1);

		//enable ME2 median filter
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_58_reg,
		              0,
		              0, 0x1);
	}
	else
	{
		WriteRegister(KME_ME1_TOP10_MV_MED_reg,
					  0,
					  0, 0x0);

		//enable ME2 median filter
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_58_reg,
		              0,
					  0, 0x0);

	}

	//o------ adaptive med9FLT ------o
	if(s_pParam->_param_me_sceneAnalysis.b_medFLT_adpt_en == 1)
	{
		WriteRegister(KME_ALGO_ME2_PFVD_PENALTY_CTRL_00_reg, 31, 31, s_pContext->_output_me_sceneAnalysis.b_do_adpt_med9flt);
	}

	//o------ pfvd for ME2 candidate penalty ------o
	if(u1_ForceUndoMask) //0xB809D9EC[31]
	{
		table_level=1;
		ReadRegister(SOFTWARE3_SOFTWARE3_59_reg, 30, 30, &u1_UndoMaskVal);//0xB809D9EC[30]
		WriteRegister(KME_ALGO_ME1_MEDFLT_CTRL_00_reg, 18, 18, u1_UndoMaskVal);
	}
	else if(s_pParam->_param_me_sceneAnalysis.b_FixRepeat_en && 
		s_pContext->_output_me_sceneAnalysis.u8_RepeatMode!=RepeatMode_OFF)
	{
		table_level=2;
		if(s_pContext->_output_me_sceneAnalysis.u1_casino_RP_detect_true) {
			WriteRegister(KME_ALGO_ME1_MEDFLT_CTRL_00_reg, 18, 18, 0);
		}
		//else if(s_pContext->_output_me_sceneAnalysis.u2_panning_flag) {
		//	WriteRegister(KME_ALGO_ME1_MEDFLT_CTRL_00_reg, 18, 18, 1);
		//}
		//else {
		//	WriteRegister(KME_ALGO_ME1_MEDFLT_CTRL_00_reg, 18, 18, 0);
		//}			
	}
	else if(s_pParam->_param_me_sceneAnalysis.b_pfvd_en == 1)
	{
		table_level=3;
		WriteRegister(KME_ALGO_ME1_MEDFLT_CTRL_00_reg, 18, 18, s_pContext->_output_me_sceneAnalysis.b_do_pfvd);
	}
	else
	{
		table_level=4;
		WriteRegister(KME_ALGO_ME1_MEDFLT_CTRL_00_reg, 18, 18, 1);
	}

	if(log_en) { //D9EC[29]
		rtd_printk(KERN_EMERG, TAG_NAME, "[Undo mask][table_level][%d]\n", table_level);
	}
}

#if 0 //to sync tv006


VOID PanningDetect_wrtRegister(_str_PanningDetect_table *PanningDetect_table)
{
	WriteRegister(MC_MC_28_reg,14,14, PanningDetect_table->mc_fb_lvl_force_en);
	WriteRegister(MC_MC_0C_reg,29,29, PanningDetect_table->mc_logo_phase_fb_phase_en);
	WriteRegister(MC_MC_0C_reg,30,30, PanningDetect_table->mc_pre_phase_fb_phase_en);
	WriteRegister(MC_MC_0C_reg,31,31, PanningDetect_table->mc_pt_phase_fb_phase_en);
	WriteRegister(MC_MC_28_reg,15,22, PanningDetect_table->mc_fb_lvl_force_value);
}

VOID PanningDetect_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	_PQLPARAMETER *s_pParam       = GetPQLParameter();
	unsigned int u32_rb_val = 0;
	static unsigned char u1_panning_state;
	_str_PanningDetect_table *tmp_PanningDetect_table = NULL;


	if(s_pParam->_param_read_comreg.u1_jerry_PanAct_en == 1){
		if((s_pContext->_output_me_sceneAnalysis.u2_panning_flag >= 1)){
			if(u1_panning_state == 0){
				ReadRegister(MC_MC_28_reg,14,14, &u32_rb_val);
				mc_fb_lvl_force_en_tmp = u32_rb_val;
				ReadRegister(MC_MC_0C_reg,29,29, &u32_rb_val);
				mc_logo_phase_fb_phase_en = u32_rb_val;
				ReadRegister(MC_MC_0C_reg,30,30, &u32_rb_val);
				mc_pre_phase_fb_phase_en = u32_rb_val;
				ReadRegister(MC_MC_0C_reg,31,31, &u32_rb_val);
				mc_pt_phase_fb_phase_en = u32_rb_val;
				ReadRegister(MC_MC_28_reg,15,22, &u32_rb_val);
				mc_fb_lvl_force_value = u32_rb_val;

				tmp_PanningDetect_table->mc_fb_lvl_force_en = 1;
				tmp_PanningDetect_table->mc_logo_phase_fb_phase_en =0;
				tmp_PanningDetect_table->mc_pre_phase_fb_phase_en =0;
				tmp_PanningDetect_table->mc_pt_phase_fb_phase_en =0;
				tmp_PanningDetect_table->mc_fb_lvl_force_value = 0;

				u1_panning_state = 1;
			}
		}else{
			if(u1_panning_state == 1){

				tmp_PanningDetect_table->mc_fb_lvl_force_en 			= mc_fb_lvl_force_en_tmp;
				tmp_PanningDetect_table->mc_logo_phase_fb_phase_en 	=mc_logo_phase_fb_phase_en;
				tmp_PanningDetect_table->mc_pre_phase_fb_phase_en 	=mc_pre_phase_fb_phase_en;
				tmp_PanningDetect_table->mc_pt_phase_fb_phase_en 	=mc_pt_phase_fb_phase_en;
				tmp_PanningDetect_table->mc_fb_lvl_force_value 		= mc_fb_lvl_force_value;

				u1_panning_state = 0;
			}
		}
	}

	if(tmp_PanningDetect_table != NULL){
		PanningDetect_wrtRegister(tmp_PanningDetect_table);
	}
}
#endif

VOID IPPRE_POST_CSCMode_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	//_PQLPARAMETER *s_pParam       = GetPQLParameter();
	PQL_INPUT_FRAME_RATE in_fmRate = s_pContext->_external_data._input_frameRate;
	unsigned char  u8_cadence_Id = s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL];

       if(s_pContext->_external_info.u16_MEMCMode == 3)  // MEMC OFF
	{
		if(s_pContext->_output_me_sceneAnalysis.u1_SportScene==false){
		HAL_IP_SetIPPRECSCMode(CSC_MODE_RGB_YUV_709_FULL);
		}
		HAL_POST_SetCSCMode(CSC_MODE_YUV_RGB_709_FULL);
	}
	else if((in_fmRate == _PQL_IN_60HZ || in_fmRate == _PQL_IN_50HZ ) && u8_cadence_Id == _CAD_VIDEO)    //  panel 60 Hz
	{
		if(s_pContext->_output_me_sceneAnalysis.u1_SportScene==false){
		HAL_IP_SetIPPRECSCMode(CSC_MODE_RGB_YUV_709_FULL);
		}
		HAL_POST_SetCSCMode(CSC_MODE_YUV_RGB_709_FULL);
	}
	else   //  only RGB input
	{
		if(s_pContext->_external_data._output_mode == _PQL_OUT_VIDEO_MODE)
		{
			if(s_pContext->_output_me_sceneAnalysis.u1_SportScene==false){
			HAL_IP_SetIPPRECSCMode(CSC_MODE_RGB_YUV_709_LIMIT);
			}
			HAL_POST_SetCSCMode(CSC_MODE_YUV_RGB_709_LIMIT);
		}
		else if(s_pContext->_external_data._output_mode == _PQL_OUT_PC_MODE)
		{
			if(s_pContext->_output_me_sceneAnalysis.u1_SportScene==false){
			HAL_IP_SetIPPRECSCMode(CSC_MODE_RGB_YUV_709_LIMIT);
			}
			HAL_IP_SetIPPRECSCEnable(FALSE);
			HAL_POST_SetCSCMode(CSC_MODE_YUV_RGB_709_LIMIT);
			HAL_POST_SetCSCEnable(FALSE);
		}
	}

}

VOID debug_info_show_writeRegister(unsigned char enable)
{
	WriteRegister(MC_MC_DC_reg,28,28, enable);
	WriteRegister(KPOST_TOP_KPOST_TOP_A8_reg,24,24, enable);
	WriteRegister(KPOST_TOP_KPOST_TOP_A8_reg,25,25, enable);
	WriteRegister(KPOST_TOP_KPOST_TOP_A8_reg,26,26, enable);
	WriteRegister(KPOST_TOP_KPOST_TOP_A8_reg,27,27, enable);
}

VOID debug_info_show(const _PARAM_WRT_COM_REG *pParam)
{
	unsigned char u1_show_debug_info_en;

	if(pParam->u1_pql_debug_info_Show_en == 1)
	{
		u1_show_debug_info_en = 1;
	}
	else
	{
		u1_show_debug_info_en = 0;
	}
	debug_info_show_writeRegister(u1_show_debug_info_en);
}

extern VOID MEMC_LibPowerSaveing(VOID);
VOID MEMC_HDMI_PowerSaving(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	MEMC_LibPowerSaveing();
}

unsigned int MEMC_OutputFrame_cnt = 0;
unsigned int MEMC_OutputFrame_pre_cnt = 0;
extern VOID MEMC_LibBlueScreen(unsigned char is_BlueScreen);
extern VOID MEMC_Lib_set_BlueScreen_Recall_flag(unsigned char u1_enable);
extern unsigned char MEMC_Lib_get_BlueScreen_Recall_flag(VOID);
extern unsigned short Scaler_DispGetInputInfoByDisp(unsigned char channel, SLR_INPUT_INFO infoList);
extern unsigned int Scaler_get_MEMC_unmute_ready_flag(void);
VOID MEMC_DisableMute(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
#ifdef CONFIG_MEMC_NOTSUPPORT // for code size reduce, when MEMC_BYPASS
			return;
#else

	unsigned int memc_mute_state = 0;
	unsigned int memc_mute_ctrl_en = 0;
	unsigned char memc_mute_en_chg = 0;
	static unsigned char memc_mute_cnt = 0;
	unsigned int test_val = 0;
	unsigned char scaler_force_bg_status = 1;

	unsigned char scaler_main_bg = 0;
	unsigned char scaler_sub_active = 0;
	unsigned char scaler_sub_enable = 0;
	unsigned int log_en = 0;

	const _PQLCONTEXT *s_pContext = GetPQLContext();
	PQL_INPUT_FRAME_RATE in_fmRate = 0;
	PQL_OUTPUT_FRAME_RATE  out_fmRate = 0;
	unsigned int test_en = 0;

#if 0

#endif
	if(get_MEMC_bypass_status_refer_platform_model() == TRUE) {
		return;
	}

#if 1
	in_fmRate = s_pContext->_external_data._input_frameRate;
	out_fmRate = s_pContext->_external_data._output_frameRate;
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 24, 24, &test_en);

	if(test_en){
		ReadRegister(SOFTWARE1_SOFTWARE1_61_reg, 24, 31, &test_val);
	}
	else if(in_fmRate == _PQL_IN_24HZ){
		if(out_fmRate == _PQL_OUT_48HZ){
			test_val = (8*48/24);
		}
		else if(out_fmRate == _PQL_OUT_60HZ){
			test_val = (8*60/24);
		}
		else if(out_fmRate == _PQL_OUT_120HZ){
			test_val = (8*120/24);
		}
		else{
			test_val = (8*60/24);
		}
	}
	else if(in_fmRate == _PQL_IN_25HZ){
		if(out_fmRate == _PQL_OUT_50HZ){
			test_val = (8*50/25);
		}
		else if(out_fmRate == _PQL_OUT_100HZ){
			test_val = (8*100/25);
		}
		else{
			test_val = (8*50/25);
		}
	}
	else if(in_fmRate == _PQL_IN_30HZ){
		if(out_fmRate == _PQL_OUT_60HZ){
			test_val = (8*60/30);
		}
		else if(out_fmRate == _PQL_OUT_120HZ){
			test_val = (8*120/30);
		}
		else{
			test_val = (8*60/30);
		}
	}
	else if(in_fmRate == _PQL_IN_48HZ){
		if(out_fmRate == _PQL_OUT_48HZ){
			test_val = (8*48/48);
		}
		else if(out_fmRate == _PQL_OUT_96HZ){
			test_val = (8*96/48);
		}
		else{
			test_val = (8*60/48);
		}
	}
	else if(in_fmRate == _PQL_IN_50HZ){
		if(out_fmRate == _PQL_OUT_50HZ){
			test_val = (8*50/50);
		}
		else if(out_fmRate == _PQL_OUT_100HZ){
			test_val = (8*100/50);
		}
		else{
			test_val = (8*50/50);
		}
	}
	else if(in_fmRate == _PQL_IN_60HZ){
		if(out_fmRate == _PQL_OUT_60HZ){
			test_val = (8*60/60);
		}
		else if(out_fmRate == _PQL_OUT_120HZ){
			test_val = (8*120/60);
		}
		else{
			test_val = (8*60/60);
		}
	}
	else if(in_fmRate == _PQL_IN_96HZ){
		if(out_fmRate == _PQL_OUT_96HZ){
			test_val = (8*96/96);
		}
		else{
			test_val = (8*120/96);
		}
	}
	else if(in_fmRate == _PQL_IN_100HZ){
		test_val = (8*100/100);
	}
	else if(in_fmRate == _PQL_IN_120HZ){
		test_val = (8*120/120);
	}
	else{
		ReadRegister(SOFTWARE1_SOFTWARE1_61_reg, 24, 31, &test_val);
	}

	if(test_val < 31){
		test_val = 31;
	}
		
	#else
	ReadRegister(SOFTWARE1_SOFTWARE1_61_reg, 24, 31, &test_val);
	#endif

	ReadRegister(SOFTWARE1_SOFTWARE1_61_reg, 24, 31, &test_val);
	ReadRegister(SOFTWARE1_SOFTWARE1_63_reg, 28, 28, &memc_mute_ctrl_en);
	//memc_mute_state = (rtd_inl(PPOVERLAY_MEMC_MUX_CTRL_reg)>>12)&0x1;
	ReadRegister(KPOST_TOP_KPOST_TOP_60_reg, 30, 30, &memc_mute_state);
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 25, 25, &log_en);
	memc_mute_en_chg = (memc_mute_state == pOutput->u1_memc_mute_state_pre) ? 0 : 1;
#if 0
	//scaler_force_bg_status = ((rtd_inl(0xB8028300)&0x00000002)>>1);//PPOVERLAY_Main_Display_Control_RSV_get_m_force_bg
#else
	scaler_main_bg = ((rtd_inl(PPOVERLAY_Main_Display_Control_RSV_reg)&0x00000002)>>1);
	scaler_sub_enable = (rtd_inl(PPOVERLAY_MP_Layout_Force_to_Background_reg)&0x1);
	//scaler_sub_active = (Scaler_DispGetInputInfoByDisp(SLR_SUB_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE);
	scaler_sub_active = (Scaler_DispGetInputInfoByDisp(1/*SLR_SUB_DISPLAY*/, 41/*SLR_INPUT_STATE*/) == 2/*_MODE_STATE_ACTIVE*/);

	if( scaler_main_bg == 0 )
	{
		scaler_force_bg_status = 0;
	}
	else if( scaler_sub_active != 0 && scaler_sub_enable != 0 )
	{
		scaler_force_bg_status = 0;
	}
	if(log_en){
		rtd_printk(KERN_NOTICE, TAG_NAME, "[MEMC_DisableMute_02][%d,]\n", scaler_sub_active);
	}
#endif

	if(memc_mute_en_chg == 1 && memc_mute_state == 1 && memc_mute_ctrl_en == 1){
		memc_mute_cnt = test_val;//10;//
		//rtd_pr_memc_debug("[MEMC_DisableMute_01][%d, %d](90K = %d)\n", memc_mute_cnt, memc_mute_state,rtd_inl(TIMER_SCPU_CLK90K_LO_reg));
		rtd_pr_memc_notice("[MEMC_DisableMute_01][%d, %d][FRM,%d,%d,](90K = %d)\n", memc_mute_cnt, memc_mute_state, in_fmRate, out_fmRate, rtd_inl(TIMER_SCPU_CLK90K_LO_reg));
	}

	if((MEMC_Lib_get_BlueScreen_Recall_flag() == 1) && (memc_mute_cnt > 0)){
		memc_mute_cnt = test_val;
		MEMC_Lib_set_BlueScreen_Recall_flag(0);
	}

	if( (memc_mute_cnt <=5)  && (memc_mute_state == 1)&& (memc_mute_ctrl_en == 1) && ((scaler_force_bg_status !=0) || Scaler_get_MEMC_unmute_ready_flag() == 0) ){
		memc_mute_cnt+=3;
	}

	if((memc_mute_cnt <= 3) && (memc_mute_state == 1) && (memc_mute_ctrl_en == 1)){
		MEMC_LibBlueScreen(0);
		//WriteRegister(KPOST_TOP_KPOST_TOP_60_reg, 30, 30, 0x0);
		rtd_pr_memc_notice("[MEMC_DisableMute_03][%d, %d](90K = %d)\n", memc_mute_cnt, memc_mute_state,rtd_inl(TIMER_SCPU_CLK90K_LO_reg));
	}

	if(memc_mute_cnt > 0){
		memc_mute_cnt--;
		if(log_en){
			rtd_pr_memc_notice("[MEMC_DisableMute_02][%d, %d](90K = %d)\n", memc_mute_cnt, memc_mute_state,rtd_inl(TIMER_SCPU_CLK90K_LO_reg));
		}
	}
	else{
		memc_mute_cnt = 0;
	}

	pOutput->u1_memc_mute_state_pre = memc_mute_state;

	MEMC_OutputFrame_cnt++;
	if(MEMC_OutputFrame_cnt > 80000000){
		MEMC_OutputFrame_cnt = 1;
	}
#endif
}

VOID DW_fmdet_4region_En_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	unsigned short u16_vip_source_type = 0;
	u16_vip_source_type = s_pContext->_external_info.u16_source_type;

	if (pParam->u1_DW_fmdet_4region_En == 1)
	{
		if ( (u16_vip_source_type >= 24) && (u16_vip_source_type<=33))//only apply in DTV
		{
			WriteRegister(KME_IPME_KME_IPME_7C_reg, 0, 0, 0x1);
		}
		else
		{
			WriteRegister(KME_IPME_KME_IPME_7C_reg, 0, 0, 0x0);
		}
	}

}

VOID MEMC_LBMC_Apply(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	WriteRegister(LBMC_LBMC_EC_reg, 2, 2, 0x1); // lbmc_db_apply
}
//////////////////////////////////////////////////////////////////////////

VOID Wrt_ComReg_Init(_OUTPUT_WRT_COM_REG *pOutput)
{
	pOutput->u8_lf_lbmcMode_pre         = _MC_NORMAL_;
	pOutput->u8_hf_lbmcMode_pre         = _MC_NORMAL_;
}

VOID Wrt_ComReg_Proc_intputInterrupt(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	mc_lbmcMode_wrtAction(pParam, pOutput);
	logoDetect_wrtAction(pParam, pOutput);
	ZMV_AdaptPNT_wrtAction(pParam, pOutput);
	PFV_CDD_Number_wrtAction(pParam, pOutput);

	// Willy 20180528
	ME_Vst_wrtAction(pParam, pOutput);
	FB_NoNeedToDoFRC_wrtAction(pParam, pOutput);

	NearRimDetect_wrtAction(pParam, pOutput);
}

MEMC_ColorTable_Pair Ori_ColorTable = {
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

MEMC_ColorTable_Pair ColorTable_startMtx = {
	{980, 2783, 333, 32299, 31189, 2048, 2048, 30908, 32580, 0, 512, 512},
	 {4096, 32672, 6210, 4096, 31904, 30610, 4096, 7504, 32526, 3332, 378, 3188},
};
MEMC_ColorTable_Pair ColorTable_endMtx = {
	{1964, 1465, 667, 32299, 31189, 2048, 2048, 30908, 32580, 0, 512, 512},
	{4096, 31805, 4044, 4096, 31037, 28444, 4096, 6637, 30360, 3711, 757, 3567},
};

#define Negative_Shift(Val,Bits) (Val<0) ? Val+(1<<Bits) : Val;

MEMC_ColorTable_Pair GetColorTable(unsigned char u8_scale, unsigned char u8_id)
{
	MEMC_ColorTable_Pair Result;
	short u16_scale;
	u8_scale = (u8_scale<=1) ? 1 : u8_scale-1;
	u8_id = (u8_id>=u8_scale) ? u8_scale : u8_id;
	u16_scale = (1<<15)/u8_scale*u8_id;
	Result.YUV2RGB.R00 = Negative_Shift(((((short)ColorTable_endMtx.YUV2RGB.R00-ColorTable_startMtx.YUV2RGB.R00)*u16_scale)>>15)+ColorTable_startMtx.YUV2RGB.R00, 15);
	Result.YUV2RGB.R01 = Negative_Shift(((((short)ColorTable_endMtx.YUV2RGB.R01-ColorTable_startMtx.YUV2RGB.R01)*u16_scale)>>15)+ColorTable_startMtx.YUV2RGB.R01, 15);
	Result.YUV2RGB.R02 = Negative_Shift(((((short)ColorTable_endMtx.YUV2RGB.R02-ColorTable_startMtx.YUV2RGB.R02)*u16_scale)>>15)+ColorTable_startMtx.YUV2RGB.R02, 15);
	Result.YUV2RGB.R10 = Negative_Shift(((((short)ColorTable_endMtx.YUV2RGB.R10-ColorTable_startMtx.YUV2RGB.R10)*u16_scale)>>15)+ColorTable_startMtx.YUV2RGB.R10, 15);
	Result.YUV2RGB.R11 = Negative_Shift(((((short)ColorTable_endMtx.YUV2RGB.R11-ColorTable_startMtx.YUV2RGB.R11)*u16_scale)>>15)+ColorTable_startMtx.YUV2RGB.R11, 15);
	Result.YUV2RGB.R12 = Negative_Shift(((((short)ColorTable_endMtx.YUV2RGB.R12-ColorTable_startMtx.YUV2RGB.R12)*u16_scale)>>15)+ColorTable_startMtx.YUV2RGB.R12, 15);
	Result.YUV2RGB.R20 = Negative_Shift(((((short)ColorTable_endMtx.YUV2RGB.R20-ColorTable_startMtx.YUV2RGB.R20)*u16_scale)>>15)+ColorTable_startMtx.YUV2RGB.R20, 15);
	Result.YUV2RGB.R21 = Negative_Shift(((((short)ColorTable_endMtx.YUV2RGB.R21-ColorTable_startMtx.YUV2RGB.R21)*u16_scale)>>15)+ColorTable_startMtx.YUV2RGB.R21, 15);
	Result.YUV2RGB.R22 = Negative_Shift(((((short)ColorTable_endMtx.YUV2RGB.R22-ColorTable_startMtx.YUV2RGB.R22)*u16_scale)>>15)+ColorTable_startMtx.YUV2RGB.R22, 15);
	Result.YUV2RGB.T0 = Negative_Shift(((((short)ColorTable_endMtx.YUV2RGB.T0-ColorTable_startMtx.YUV2RGB.T0)*u16_scale)>>15)+ColorTable_startMtx.YUV2RGB.T0, 12);
	Result.YUV2RGB.T1 = Negative_Shift(((((short)ColorTable_endMtx.YUV2RGB.T1-ColorTable_startMtx.YUV2RGB.T1)*u16_scale)>>15)+ColorTable_startMtx.YUV2RGB.T1, 12);
	Result.YUV2RGB.T2 = Negative_Shift(((((short)ColorTable_endMtx.YUV2RGB.T2-ColorTable_startMtx.YUV2RGB.T2)*u16_scale)>>15)+ColorTable_startMtx.YUV2RGB.T2, 12);
  
  	Result.RGB2YUV.R00 = Negative_Shift(((((short)ColorTable_endMtx.RGB2YUV.R00-ColorTable_startMtx.RGB2YUV.R00)*u16_scale)>>15)+ColorTable_startMtx.RGB2YUV.R00, 15);
	Result.RGB2YUV.R01 = Negative_Shift(((((short)ColorTable_endMtx.RGB2YUV.R01-ColorTable_startMtx.RGB2YUV.R01)*u16_scale)>>15)+ColorTable_startMtx.RGB2YUV.R01, 15);
	Result.RGB2YUV.R02 = Negative_Shift(((((short)ColorTable_endMtx.RGB2YUV.R02-ColorTable_startMtx.RGB2YUV.R02)*u16_scale)>>15)+ColorTable_startMtx.RGB2YUV.R02, 15);
	Result.RGB2YUV.R10 = Negative_Shift(((((short)ColorTable_endMtx.RGB2YUV.R10-ColorTable_startMtx.RGB2YUV.R10)*u16_scale)>>15)+ColorTable_startMtx.RGB2YUV.R10, 15);
	Result.RGB2YUV.R11 = Negative_Shift(((((short)ColorTable_endMtx.RGB2YUV.R11-ColorTable_startMtx.RGB2YUV.R11)*u16_scale)>>15)+ColorTable_startMtx.RGB2YUV.R11, 15);
	Result.RGB2YUV.R12 = Negative_Shift(((((short)ColorTable_endMtx.RGB2YUV.R12-ColorTable_startMtx.RGB2YUV.R12)*u16_scale)>>15)+ColorTable_startMtx.RGB2YUV.R12, 15);
	Result.RGB2YUV.R20 = Negative_Shift(((((short)ColorTable_endMtx.RGB2YUV.R20-ColorTable_startMtx.RGB2YUV.R20)*u16_scale)>>15)+ColorTable_startMtx.RGB2YUV.R20, 15);
	Result.RGB2YUV.R21 = Negative_Shift(((((short)ColorTable_endMtx.RGB2YUV.R21-ColorTable_startMtx.RGB2YUV.R21)*u16_scale)>>15)+ColorTable_startMtx.RGB2YUV.R21, 15);
	Result.RGB2YUV.R22 = Negative_Shift(((((short)ColorTable_endMtx.RGB2YUV.R22-ColorTable_startMtx.RGB2YUV.R22)*u16_scale)>>15)+ColorTable_startMtx.RGB2YUV.R22, 15);
	Result.RGB2YUV.T0 = Negative_Shift(((((short)ColorTable_endMtx.RGB2YUV.T0-ColorTable_startMtx.RGB2YUV.T0)*u16_scale)>>15)+ColorTable_startMtx.RGB2YUV.T0, 12);
	Result.RGB2YUV.T1 = Negative_Shift(((((short)ColorTable_endMtx.RGB2YUV.T1-ColorTable_startMtx.RGB2YUV.T1)*u16_scale)>>15)+ColorTable_startMtx.RGB2YUV.T1, 12);
	Result.RGB2YUV.T2 = Negative_Shift(((((short)ColorTable_endMtx.RGB2YUV.T2-ColorTable_startMtx.RGB2YUV.T2)*u16_scale)>>15)+ColorTable_startMtx.RGB2YUV.T2, 12);

	return Result;
}

VOID Wrt_ComReg_Set_colormatrix(MEMC_ColorTable_Pair *ColorTable)
{
	WriteRegister(IPPRE_IPPRE_80_reg,  0, 14,ColorTable->RGB2YUV.R00);
	WriteRegister(IPPRE_IPPRE_80_reg, 15, 29,ColorTable->RGB2YUV.R01);
	WriteRegister(IPPRE_IPPRE_84_reg,  0, 14,ColorTable->RGB2YUV.R02);
	WriteRegister(IPPRE_IPPRE_84_reg, 15, 29,ColorTable->RGB2YUV.R10);
	WriteRegister(IPPRE_IPPRE_88_reg,  0, 14,ColorTable->RGB2YUV.R11);
	WriteRegister(IPPRE_IPPRE_88_reg, 15, 29,ColorTable->RGB2YUV.R12);
	WriteRegister(IPPRE_IPPRE_8C_reg,  0, 14,ColorTable->RGB2YUV.R20);
	WriteRegister(IPPRE_IPPRE_8C_reg, 15, 29,ColorTable->RGB2YUV.R21);
	WriteRegister(IPPRE_IPPRE_90_reg,  0, 14,ColorTable->RGB2YUV.R22);
	WriteRegister(IPPRE_IPPRE_90_reg, 15, 26,ColorTable->RGB2YUV.T0);
	WriteRegister(IPPRE_IPPRE_94_reg,  0, 11,ColorTable->RGB2YUV.T1);
	WriteRegister(IPPRE_IPPRE_94_reg, 12, 23,ColorTable->RGB2YUV.T2);

	WriteRegister(IPPRE_IPPRE_9C_reg,  0, 14,ColorTable->RGB2YUV.R00);
	WriteRegister(IPPRE_IPPRE_9C_reg, 15, 29,ColorTable->RGB2YUV.R01);
	WriteRegister(IPPRE_IPPRE_A0_reg,  0, 14,ColorTable->RGB2YUV.R02);
	WriteRegister(IPPRE_IPPRE_A0_reg, 15, 29,ColorTable->RGB2YUV.R10);
	WriteRegister(IPPRE_IPPRE_A4_reg,  0, 14,ColorTable->RGB2YUV.R11);
	WriteRegister(IPPRE_IPPRE_A4_reg, 15, 29,ColorTable->RGB2YUV.R12);
	WriteRegister(IPPRE_IPPRE_A8_reg,  0, 14,ColorTable->RGB2YUV.R20);
	WriteRegister(IPPRE_IPPRE_A8_reg, 15, 29,ColorTable->RGB2YUV.R21);
	WriteRegister(IPPRE_IPPRE_AC_reg,  0, 14,ColorTable->RGB2YUV.R22);
	WriteRegister(IPPRE_IPPRE_AC_reg, 15, 26,ColorTable->RGB2YUV.T0);
	WriteRegister(IPPRE_IPPRE_B0_reg,  0, 11,ColorTable->RGB2YUV.T1);
	WriteRegister(IPPRE_IPPRE_B0_reg, 12, 23,ColorTable->RGB2YUV.T2);

	WriteRegister(KPOST_TOP_KPOST_TOP_80_reg,  0, 14,ColorTable->YUV2RGB.R00);
	WriteRegister(KPOST_TOP_KPOST_TOP_80_reg, 15, 29,ColorTable->YUV2RGB.R01);
	WriteRegister(KPOST_TOP_KPOST_TOP_84_reg,  0, 14,ColorTable->YUV2RGB.R02);
	WriteRegister(KPOST_TOP_KPOST_TOP_84_reg, 15, 29,ColorTable->YUV2RGB.R10);
	WriteRegister(KPOST_TOP_KPOST_TOP_88_reg,  0, 14,ColorTable->YUV2RGB.R11);
	WriteRegister(KPOST_TOP_KPOST_TOP_88_reg, 15, 29,ColorTable->YUV2RGB.R12);
	WriteRegister(KPOST_TOP_KPOST_TOP_8C_reg,  0, 14,ColorTable->YUV2RGB.R20);
	WriteRegister(KPOST_TOP_KPOST_TOP_8C_reg, 15, 29,ColorTable->YUV2RGB.R21);
	WriteRegister(KPOST_TOP_KPOST_TOP_90_reg,  0, 14,ColorTable->YUV2RGB.R22);
	WriteRegister(KPOST_TOP_KPOST_TOP_90_reg, 15, 26,ColorTable->YUV2RGB.T0);
	WriteRegister(KPOST_TOP_KPOST_TOP_94_reg,  0, 11,ColorTable->YUV2RGB.T1);
	WriteRegister(KPOST_TOP_KPOST_TOP_94_reg, 12, 23,ColorTable->YUV2RGB.T2);
}

VOID MEMC_Lib_Manual_colormatrix(_OUTPUT_ME_SCENE_ANALYSIS *pSceneInfo)
{
	bool u1_SportScene = pSceneInfo->u1_SportScene;
	unsigned char u8_scale = 40, u8_keepLen = 4;
	unsigned int u32_post_out_vtotal_int, Uzudtg_line_count_read_out, Memcdtg_line_count_read_out;
	unsigned int Algo_en, log_en, log_en2, force_Trans, dis_force, forcetable, forcetableid;
	static unsigned char u8_table_cnt = 0;
	unsigned char u8_table_id=0;
	MEMC_ColorTable_Pair ColorTable;
	
	ReadRegister(SOFTWARE3_SOFTWARE3_05_reg, 0, 0, &Algo_en); //d914[0]
	ReadRegister(SOFTWARE3_SOFTWARE3_05_reg, 1, 1, &log_en);
	ReadRegister(SOFTWARE3_SOFTWARE3_05_reg, 2, 2, &log_en2);
	ReadRegister(SOFTWARE3_SOFTWARE3_05_reg,31,31, &force_Trans);
	ReadRegister(SOFTWARE3_SOFTWARE3_05_reg,30,30, &dis_force);
	ReadRegister(SOFTWARE3_SOFTWARE3_05_reg,15,15, &forcetable);
	ReadRegister(SOFTWARE3_SOFTWARE3_05_reg, 8,14, &forcetableid);
	
	if(Algo_en==false) {
		return;
	}

	if(forcetable){//d914[15]
		u8_table_id = (forcetableid>=u8_scale) ? u8_scale-1 : forcetableid;
		ColorTable = GetColorTable(u8_scale,u8_table_id);

		Wrt_ComReg_Set_colormatrix(&ColorTable);

		WriteRegister(IPPRE_IPPRE_94_reg,25,28,0x8);
		WriteRegister(IPPRE_IPPRE_B0_reg,25,28,0x8);
		WriteRegister(KPOST_TOP_KPOST_TOP_94_reg,25,28,0x8);
		if(log_en) {
			rtd_pr_memc_emerg("[%s][%d] [%d][%d]\n", __FUNCTION__, __LINE__, u8_table_cnt, u8_table_id );
		}
	}
	else if(dis_force || u1_SportScene==false){
		//resume
		u8_table_id = u8_table_cnt/u8_keepLen;
		u8_table_cnt = (u8_table_cnt>=1) ? u8_table_cnt-1 : 0;
		
		if(u8_table_cnt==0) {
			WriteRegister(IPPRE_IPPRE_94_reg,25,28,0x2);
			WriteRegister(IPPRE_IPPRE_B0_reg,25,28,0x2);
			WriteRegister(KPOST_TOP_KPOST_TOP_94_reg,25,28,0x3);
			
			Wrt_ComReg_Set_colormatrix(&Ori_ColorTable);
		}
		else {
			ColorTable = GetColorTable(u8_scale,u8_table_id);
			
			Wrt_ComReg_Set_colormatrix(&ColorTable);
			
			WriteRegister(IPPRE_IPPRE_94_reg,25,28,0x8);
			WriteRegister(IPPRE_IPPRE_B0_reg,25,28,0x8);
			WriteRegister(KPOST_TOP_KPOST_TOP_94_reg,25,28,0x8);

		}
		if(log_en) {
			rtd_pr_memc_emerg("[%s][%d] [%d][%d]\n", __FUNCTION__, __LINE__, u8_table_cnt, u8_table_id );
		}
	}
	else if(force_Trans || u1_SportScene==true) {
		u8_table_id = u8_table_cnt/u8_keepLen;
		u8_table_cnt = (u8_table_cnt>=(u8_keepLen*u8_scale)-1) ? (u8_keepLen*u8_scale)-1 : u8_table_cnt+1;
		ColorTable = GetColorTable(u8_scale,u8_table_id);

		Wrt_ComReg_Set_colormatrix(&ColorTable);

		WriteRegister(IPPRE_IPPRE_94_reg,25,28,0x8);
		WriteRegister(IPPRE_IPPRE_B0_reg,25,28,0x8);
		WriteRegister(KPOST_TOP_KPOST_TOP_94_reg,25,28,0x8);

		if(log_en) {
			rtd_pr_memc_emerg("[%s][%d] [%d][%d]\n", __FUNCTION__, __LINE__, u8_table_cnt, u8_table_id );
		}
	}

#if 1
	if(log_en2) {//d914[2]
		ReadRegister(KPOST_TOP_KPOST_TOP_04_reg,20,31,&u32_post_out_vtotal_int);
		ReadRegister(PPOVERLAY_new_meas1_linecnt_real_reg,16,28,&Uzudtg_line_count_read_out);
		ReadRegister(PPOVERLAY_new_meas1_linecnt_real_reg,0,12,&Memcdtg_line_count_read_out);	
		
		rtd_pr_memc_emerg("[%s][%d][%d,%d][d004,%x,][Uzudtg_line_cnt,%d,][MEMC_line_cnt,%d,][90K,%d,]\n", 
			__FUNCTION__, u1_SportScene, u8_table_id, u8_table_cnt,u32_post_out_vtotal_int, Uzudtg_line_count_read_out, Memcdtg_line_count_read_out, rtd_inl(TIMER_SCPU_CLK90K_LO_reg));
	}
#endif
}


extern VOID MEMC_Lib_MC_IndexCheck(VOID);
extern VOID MEMC_LibResetFlow(VOID);
extern VOID MEMC_Lib_VR360_Flow(VOID);
extern VOID MEMC_Lib_CadenceChangeCtrl(VOID);
extern VOID MEMC_LibSet_PQC_DB_apply(VOID);
extern VOID MEMC_LibSet_MCDMA_DB_apply(VOID);
extern VOID Scaler_MEMC_Set_Lib_Freeze(unsigned char enable);
extern VOID MEMC_Lib_SetLineMode(unsigned char u1_enable);
extern unsigned char MEMC_Lib_get_LineMode_flag(VOID);
extern VOID MEMC_LibSet_PQC_DB_en(unsigned int u1_enable);
extern VOID MEMC_LibSet_MCDMA_DB_en(unsigned int u1_enable);
extern VOID MEMC_Lib_DelayTime_Calc(VOID);
extern VOID Mid_Mode_MEDEN_Checking(VOID);
extern VOID MEMC_Lib_SetLbmcVtrigDelayMode(unsigned char u1_enable);
extern unsigned char MEMC_Lib_get_LbmcTrig_flag(VOID);
//extern VOID Mid_Mode_MEDEN_Checking(VOID);
//extern unsigned char MEMC_Lib_get_LbmcTrig_flag(VOID);
extern unsigned char lowdelay_state_chg_cnt;

VOID Wrt_ComReg_Proc_outputInterrupt(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	unsigned int Freeze_en = 0, test_en = 0, log_en = 0;
	unsigned char LineMode_flag_chg = 0, LbmcTrig_flag_chg = 0;
	static unsigned char LineMode_flag_pre = 0, LbmcTrig_stage_pre = 0, line_mode_states = 0;
#if IDENTIFICATION_EN
	static unsigned char MEMC_ID_cnt = 0;
#endif

	LineMode_flag_chg = (LineMode_flag_pre == MEMC_Lib_get_LineMode_flag()) ? 0 : 1;
	LbmcTrig_flag_chg = (LbmcTrig_stage_pre == MEMC_Lib_get_LbmcTrig_flag()) ? 0 : 1;
	ReadRegister(SOFTWARE1_SOFTWARE1_00_reg, 25, 25, &Freeze_en);
	ReadRegister(SOFTWARE2_SOFTWARE2_63_reg, 30, 30, &test_en);
	ReadRegister(SOFTWARE2_SOFTWARE2_63_reg, 29, 29, &log_en);

	if(pParam->u1_Wrt_ComReg_Proc_en)
	{
		#if PQL_DYNAMIC_ADJUST_FIX_ISSUE
		Identification_Pattern_preProcess(pParam,pOutput);
		#endif
		fbCtrl_wrtAction(pParam, pOutput);
		dhClose_wrtAction(pParam);
		ME_obmeSelection_wrtAction(pParam);
		mv_accord_wrtAction(pParam);
		//dhLocalFbLvl_wrtAction(pParam);
		dynamicME_wrtAction(pParam,pOutput);
		Identification_Pattern_Update(pParam, pOutput);

#ifdef BG_MV_GET
		gmvCtrl_wrtAction(pParam);//ed
#endif

#if PQL_DYNAMIC_ADJUST_FIX_ISSUE
		scCtrl_wrtAction(pParam, pOutput);
		sc_enCtrl_wrtAction(pParam, pOutput);
		Mc2_var_lpf_wrtAction(pParam,pOutput);
		MC_Unsharp_Mask_wrtAction(pParam,pOutput);
		MC_SObj_wrtAction(pParam,pOutput);
		ME2_SObj_wrtAction(pParam,pOutput);
		//IPPRE_POST_CSCMode_wrtAction(pParam,pOutput);
		//MC_blend_alpha_wrtAction(pParam,pOutput);

		Dh_Local_FB_wrtAction(pParam,pOutput);
		Dh_OCCL_Ext_wrtAction(pParam,pOutput);
		DW_fmdet_4region_En_wrtAction(pParam,pOutput);
		Identification_Pattern_wrtAction(pParam,pOutput);
		Dh_MV_Corr_wrtAction(pParam,pOutput);
		Pred_MV_Cand_dis(pParam,pOutput) ;          //pred mv cand disable
		MEMC_Performance_Checking_Database();
#endif
	}

	Wrt_7segment_show(pParam);
	debug_info_show(pParam);
	MEMC_HDMI_PowerSaving(pParam, pOutput);

	//-----line mode state machine
	if(LineMode_flag_chg){
		if(MEMC_Lib_get_LineMode_flag() == 1){
			line_mode_states = 1;
		}else{
			line_mode_states = 5;
		}
	}
	if(line_mode_states > 0 && lowdelay_state_chg_cnt > 0){
		line_mode_states = 5;
	}
	
	if(line_mode_states == 1){
		if(MEMC_Lib_get_LineMode_flag() == 1)
			MEMC_Lib_SetLineMode(1);
		else
			MEMC_Lib_SetLineMode(0);
	}
	if(line_mode_states > 0){
		line_mode_states--;
	}
	if(Freeze_en){
		rtd_pr_memc_notice("[%s][,%d,][,%d,%d,%d,]\n\r", __FUNCTION__, __LINE__, LineMode_flag_chg, lowdelay_state_chg_cnt, line_mode_states);
	}
	//-----

	//-----lbmc trig state machine
	if(LbmcTrig_flag_chg){
		if(MEMC_Lib_get_LbmcTrig_flag() == 1)
			MEMC_Lib_SetLbmcVtrigDelayMode(1);
		else
			MEMC_Lib_SetLbmcVtrigDelayMode(0);
	}
	//-----

	MEMC_DisableMute(pParam, pOutput);
	MEMC_Lib_MC_IndexCheck();
	MEMC_Lib_DelayTime_Calc();
	MEMC_LibResetFlow();
	MEMC_Lib_CadenceChangeCtrl();
	/*
	if(Freeze_en)
		Scaler_MEMC_Set_Lib_Freeze(1);
	else
		Scaler_MEMC_Set_Lib_Freeze(0);
	*/
	MEMC_Lib_VR360_Flow();
	MC_RMV_blending_ReginaolFB_wrtAction();
	Over_SearchRange_wrtAction();
	ME_Median_Filter_wrtAction();
	MEMC_LBMC_Apply(pParam, pOutput);
	MEMC_LibSet_PQC_DB_apply();
	MEMC_LibSet_MCDMA_DB_apply();

	//update status
	LineMode_flag_pre = MEMC_Lib_get_LineMode_flag();
	LbmcTrig_stage_pre = MEMC_Lib_get_LbmcTrig_flag();
	MEMC_SmallObject_wrtAction(pParam);
	Mid_Mode_MEDEN_Checking();

#if IDENTIFICATION_EN
	// MEMC_ID_test
	if(test_en){
		if( MEMC_ID_GetFlag(0, 1) == TRUE ){
			MEMC_ID_cnt = 60;
		}

		if(MEMC_ID_cnt > 0){
			MEMC_ID_cnt--;
			MEMC_ID_flag = 1;
		}
		else{
			MEMC_ID_flag = 0;
		}

#if Pro_tv010
{
		unsigned char u8_i = 0;
		if(MEMC_ID_flag == 0) {
			for(u8_i=0;u8_i<255;u8_i++){
				if( MEMC_ID_GetFlag(4, u8_i) == TRUE ){
					MEMC_ID_flag = 1;
					break;
				}
			}
		}
}		
#endif
	}
	else{
		MEMC_ID_cnt = 0;
		MEMC_ID_flag = 0;
	}
#endif
}
