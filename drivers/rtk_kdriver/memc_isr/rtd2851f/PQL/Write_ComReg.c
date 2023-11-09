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
//dehalo me1 normal
#include "common/include/rbus/kme_dehalo5_reg.h"
#include "common/include/rbus/kme_me1_bg0_reg.h"
#include "common/include/rbus/kme_me1_bg1_reg.h"
#include "memc_isr/include/memc_identification.h"

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
	od_od_ctrl_RBUS od_ctrl_reg;			
	od_ctrl_reg.regValue = rtd_inl(OD_OD_CTRL_reg);	
	
	if(od_ctrl_reg.dummy1802ca00_31_12>>11 & 1) // bit23 to show AI info
	{
	#if IS_ENABLED(CONFIG_RTK_AI_DRV)
	extern int show_ai_sc;
	//extern int scene_nn;
	extern int scene_pq;
	extern int sqm_pq;
	extern int ai_genre;
	extern int ai_content;
	extern int ai_resolution;
	unsigned int  u32_wrt_data;//, u32_wrt_data2;
	extern unsigned int debug_draw_cnt;
	// =======================================================================================================================
	// 7seg_0 [0~15]
	u32_wrt_data = ((rtd_inl(0xb802a47c)&0x00000001)||((rtd_inl(0xb802a4bc)&0x80000000)>>31))||(rtd_inl(0xb802e500)&0x00000001)||((rtd_inl(0xB802b740)&0x10000000)>>28);//shp
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
	unsigned int  show_repeat = 0;
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
	// u32_wrt_data=s_pContext->_output_read_comreg.u1_sc_status_rb;
	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 16, 23, u32_wrt_data);
//	u32_wrt_data = g_frm_cnt;
//	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 16, 23, u32_wrt_data);
#if 1
//	u32_wrt_data = s_pContext->_output_mc_lbmcswitch.u8_lf_lbmcMode;
	u32_wrt_data = s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true|
	  			(s_pContext->_output_wrt_comreg.u1_casino_RP_detect_true<< 3);
	//u32_wrt_data = s_pContext->_output_fblevelctrl.u8_pan_scene_hold_cnt;
	// if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_dhbypass==1)
		
	//  u32_wrt_data=s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_dhbypass;
#else
	u32_wrt_data=s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_lowfgapl;
#endif

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
	
	// debug for repeat mode
	ReadRegister(SOFTWARE3_SOFTWARE3_10_reg, 0,0,&show_repeat);
	if(show_repeat) {
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
	RTKReadRegister(0xB80282E4, &u32_wrt_data);
	u32_wrt_data = (270000000/(u32_wrt_data+1))+5;

	//output timing
	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 24, 27, (u32_wrt_data/10)%10);
	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 28, 31, (u32_wrt_data/10)/10);

	// motion dir and dtl dir
	//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 24, 27, s_pContext->_output_me_sceneAnalysis.u2_GMV_single_move);
	//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 28, 31, s_pContext->_output_me_sceneAnalysis.u2_mcDtl_HV);

	//zoom[0],swing[1],RgRotat[2],EPG[3]

	u32_wrt_data = s_pContext->_output_frc_sceneAnalysis.u1_Zoom_true  |
			   (( MEMC_ID_flag | s_pContext->_output_wrt_comreg.u1_IP_preProcess_true | s_pContext->_output_wrt_comreg.u1_IP_wrtAction_true | s_pContext->_output_wrt_comreg.u1_Pattern_Update_true )<< 1) |
//				((MEMC_ID_flag)<< 1) |
			   (s_pContext->_output_frc_sceneAnalysis.u1_RgRotat_true<<2) |
			   ((s_pContext->_output_me_vst_ctrl.u1_detect_blackBG_VST_trure)<<3);
		
	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 20, 23, u32_wrt_data);

	// panning[0-1], logo clear flag[2], logo sc status[3]
#if 1  //ori
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
		{{CTRL_N,0}, 		{CTRL_N, 0}, 				{CTRL_N,0}		},/* 0.load default */
		{{CTRL_Replace,0}, 	{CTRL_N, 0}, 				{CTRL_N,0}		},/* 1.seek control */
		{{CTRL_Replace,0}, 	{CTRL_Replace, 0x980000}, 	{CTRL_Replace,2}	},/* 2.video cadence case use large threshold to prevent easy scene change for moving video text */
		{{CTRL_Replace,0}, 	{CTRL_Pluse, 0x660000},		{CTRL_N,0}		},/*3. */
		{{CTRL_Replace,0}, 	{CTRL_Default, 0},			{CTRL_N,0}		},/*4. */
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
	unsigned int u32_RB_debug;
	ReadRegister(SOFTWARE1_SOFTWARE1_25_reg, 31, 31, &u32_RB_debug);

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

		tmp_str_scCtrl_table = fwif_MEMC_get_scCtrl_table(case_apply);

		if(case_apply==2 && s_pContext->_output_me_sceneAnalysis.u2_panning_flag == 1 && cadence_transient_stream_flag){
			if(scCtrl_wrt_cnt>0){
				scCtrl_wrt_cnt--;
			}else{
				scCtrl_wrt_cnt =0;;
			}
			
			if((case_apply!=pre_case_apply) ){
				scCtrl_wrt_cnt = tmp_str_scCtrl_table->me1_sc_me1_hold_ctrl.me1_sc_me1_hold_cnt;			
			}

			if((case_apply!=pre_case_apply) || (scCtrl_wrt_cnt!=0)){		
				scCtrl_wrtRegister(tmp_str_scCtrl_table);

			}
		}
		else {
			scCtrl_wrtRegister(tmp_str_scCtrl_table);
		}

		pre_case_apply = case_apply;

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
	if ((pParam->u1_RimCtrl_wrt_en == 1) && (s_pContext->_output_rimctrl.u1_RimChange == 1))
	{
		// MC
		WriteRegister(MC_MC_58_reg,0,11, s_pContext->_output_rimctrl.u12_mc_rim1[_RIM_TOP]); // Rim1
		WriteRegister(MC_MC_58_reg,16,27, s_pContext->_output_rimctrl.u12_mc_rim1[_RIM_BOT]);
		WriteRegister(MC_MC_5C_reg,0,11, s_pContext->_output_rimctrl.u12_mc_rim1[_RIM_LFT]);
		WriteRegister(MC_MC_5C_reg,16,27, s_pContext->_output_rimctrl.u12_mc_rim1[_RIM_RHT]);

		WriteRegister(MC_MC_50_reg,0,11, s_pContext->_output_rimctrl.u12_mc_rim0[_RIM_TOP]); // Rim0
		WriteRegister(MC_MC_50_reg,16,27, s_pContext->_output_rimctrl.u12_mc_rim0[_RIM_BOT]);
		WriteRegister(MC_MC_54_reg,0,11, s_pContext->_output_rimctrl.u12_mc_rim0[_RIM_LFT]);
		WriteRegister(MC_MC_54_reg,16,27, s_pContext->_output_rimctrl.u12_mc_rim0[_RIM_RHT]);

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

	if (pParam->u1_dhClose_wrt_en == 1)
	{
		if (u1_dehalo_en == 0)
		{
			//WriteRegister(KME_DEHALO2_KME_DEHALO2_30_reg,17,17, 1);	//regional gain
			//WriteRegister(KME_DEHALO_KME_DEHALO_BC_reg,17,22, 0x3F);//old alogo
            WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,23,23,0x1); // merlin7 new algo
		}
		else
		{
			//WriteRegister(KME_DEHALO2_KME_DEHALO2_30_reg,17,17, 0);	//global gain
			//WriteRegister(KME_DEHALO_KME_DEHALO_BC_reg,17,22, 0x10); //old alogo
            WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,23,23,0x0); // merlin7 new algo
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
	if(level > 4)
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
	{CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE},	/*0:load default*/
	{CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,0,0,0,CTRL_NONE,0,0,0,CTRL_NONE,CTRL_NONE,CTRL_NONE},				/*1*/
	{CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,0,CTRL_NONE,0,CTRL_NONE,0,CTRL_NONE,0,CTRL_NONE,CTRL_NONE,CTRL_NONE},	/*2*/
	{CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,0,0,CTRL_NONE,CTRL_NONE,0,0,CTRL_NONE,CTRL_NONE,CTRL_NONE},	/*3*/
	{CTRL_NONE,CTRL_NONE,CTRL_NONE,0,0,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,0,0,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE},	/*4*/
	{CTRL_NONE,CTRL_NONE,CTRL_NONE,0,0,0,0,0,0,CTRL_NONE,CTRL_NONE,CTRL_NONE,0,0,0,0,0,0,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE},	/*5*/
	{CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,1,CTRL_NONE},	/*6*/
	{CTRL_NONE,CTRL_NONE,CTRL_NONE,0,0,0,0,0,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,0,0,0,0,0,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE,CTRL_NONE},	/*7*/
	{1,0,1,1,1,0,0,0,0,1,0,1,1,1,0,0,0,0,1,1,1,0,1,1,1,0,0,0},	/*8 repeat broken*/
	{1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,0},	/*9 small object*/
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
	static unsigned int u32_ID_Log_en=0;//,ES_Test_Value=0;
	unsigned int u1_ForceCand = 0, u1_Cand_en[26];
	ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 28,  28, &u32_ID_Log_en);//0xB809D758
	ReadRegister(SOFTWARE3_SOFTWARE3_58_reg, 31,  31, &u1_ForceCand);//0xB809D9E8

	if(pParam->u1_MECand_wrt_en == 0)//d5e4[15]
		return;

	#if 1
	if(u1_ForceCand) {
		for(u8_i=0;u8_i<26;u8_i++){
			ReadRegister(SOFTWARE3_SOFTWARE3_58_reg, u8_i,  u8_i, &u1_Cand_en[u8_i]);
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
	else if(s_pContext->_output_me_sceneAnalysis.u1_SportScene) {
		table_level = 9;
		tmp_ME_cand_table = fwif_MEMC_get_ME_cand_table(table_level);
	}
	else if(s_pParam->_param_me_sceneAnalysis.b_FixRepeat_en && 
		s_pContext->_output_me_sceneAnalysis.u8_RepeatMode!=RepeatMode_OFF)
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
	else if( (s_pParam->_param_read_comreg.u1_powerSaving_en && s_pContext->_external_data.u1_powerSaving == 1) ||( s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type==3) )
	{
		table_level = 1;
		tmp_ME_cand_table = fwif_MEMC_get_ME_cand_table(table_level); // disable ip/pi update0/1/2 by table, others the same

		//tmp_ME_cand_table->vbuf_ip_1st_cand3_en = 	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand3_en;
		//tmp_ME_cand_table->vbuf_ip_1st_cand4_en = 	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand4_en;
		//tmp_ME_cand_table->vbuf_pi_1st_cand3_en = 	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand3_en;
		//tmp_ME_cand_table->vbuf_pi_1st_cand4_en = 	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand4_en;
	}
	 else if(s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type==1)
	 {
	 	table_level = 2;
		tmp_ME_cand_table = fwif_MEMC_get_ME_cand_table(table_level); // disable ip/pi update0/2 by table, others the same

		//tmp_ME_cand_table->vbuf_ip_1st_cand3_en = 	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand3_en;
		//tmp_ME_cand_table->vbuf_ip_1st_cand4_en = 	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand4_en;
		//tmp_ME_cand_table->vbuf_pi_1st_cand3_en = 	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand3_en;
		//tmp_ME_cand_table->vbuf_pi_1st_cand4_en = 	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand4_en;

		//tmp_ME_cand_table->vbuf_ip_1st_update1_en =	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update1_en;
		//tmp_ME_cand_table->vbuf_pi_1st_update1_en =	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update1_en;
	}
	 else if(s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type==2)
	 {
	 	table_level = 3;
		tmp_ME_cand_table = fwif_MEMC_get_ME_cand_table(table_level); // disable ip/pi update1/2 by table, others the same

		//tmp_ME_cand_table->vbuf_ip_1st_cand3_en = 	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand3_en;
		//tmp_ME_cand_table->vbuf_ip_1st_cand4_en = 	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand4_en;
		//tmp_ME_cand_table->vbuf_pi_1st_cand3_en = 	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand3_en;
		//tmp_ME_cand_table->vbuf_pi_1st_cand4_en = 	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand4_en;

		//tmp_ME_cand_table->vbuf_ip_1st_update0_en =	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update0_en;
		//tmp_ME_cand_table->vbuf_pi_1st_update0_en	 =	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update0_en;

	}
#if IC_K4LP
	else if(s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true == 1 )
	{
		table_level = 4;
		tmp_ME_cand_table = fwif_MEMC_get_ME_cand_table(table_level); // disable ip/pi cand3/4 by table, others the same

		//tmp_ME_cand_table->vbuf_ip_1st_update1_en =	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update1_en;
		//tmp_ME_cand_table->vbuf_ip_1st_update2_en	 =	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update2_en;
		//tmp_ME_cand_table->vbuf_ip_1st_update0_en =	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update0_en;
		//tmp_ME_cand_table->vbuf_pi_1st_update1_en =	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update1_en;
		//tmp_ME_cand_table->vbuf_pi_1st_update2_en =	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update2_en;
		//tmp_ME_cand_table->vbuf_pi_1st_update0_en	 =	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update0_en;
	}
#endif
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
	}else if(s_pContext->_output_me_sceneAnalysis.u2_strong_pure_panning_flag == 1){
		table_level = 7;
		tmp_ME_cand_table = fwif_MEMC_get_ME_cand_table(table_level);
	}
	else if(s_pParam->_param_me_sceneAnalysis.b_FixRepeat_en &&
		s_pContext->_output_me_sceneAnalysis.u8_RepeatMode==RepeatMode_OFF)
	{
		table_level = 0;
		tmp_ME_cand_table = fwif_MEMC_get_ME_cand_table(table_level);
	}
	else
	{
		table_level = 0;
		tmp_ME_cand_table = fwif_MEMC_get_ME_cand_table(table_level);

		//tmp_ME_cand_table->vbuf_ip_1st_cand3_en = 	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand3_en;
		//tmp_ME_cand_table->vbuf_ip_1st_cand4_en = 	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand4_en;
		//tmp_ME_cand_table->vbuf_pi_1st_cand3_en = 	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand3_en;
		//tmp_ME_cand_table->vbuf_pi_1st_cand4_en = 	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand4_en;

		//tmp_ME_cand_table->vbuf_ip_1st_update1_en =	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update1_en;
		//tmp_ME_cand_table->vbuf_ip_1st_update2_en	 =	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update2_en;
		//tmp_ME_cand_table->vbuf_ip_1st_update0_en =	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update0_en;
		//tmp_ME_cand_table->vbuf_pi_1st_update1_en =	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update1_en;
		//tmp_ME_cand_table->vbuf_pi_1st_update2_en =	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update2_en;
		//tmp_ME_cand_table->vbuf_pi_1st_update0_en	 =	s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update0_en;

	}
	if(u32_ID_Log_en==1)//0xB809D758 [28]
	{
		static unsigned char counter = 0;
		if(counter % 10 == 0)
			rtd_pr_memc_info("[%s][%d]table_level =  %d",__FUNCTION__, __LINE__,table_level);
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
	{7,7, 0,7, 1,7, 7,0, 1,0, 6,2, 0,2, 2,2,
	 7,7, 0,7, 1,7, 7,0, 1,0, 6,2, 0,2, 2,2,
 	 7,7, 0,7, 1,7, 1,7,
 	 7,7, 0,7, 1,7, 0,1},
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
	if (ME_candOfst_table->vbuf_ip_1st_cand0_offsetx != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg,  0,  2, 	ME_candOfst_table->vbuf_ip_1st_cand0_offsetx);
	if (ME_candOfst_table->vbuf_ip_1st_cand0_offsety != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg,  3,  5, 	ME_candOfst_table->vbuf_ip_1st_cand0_offsety);
	if (ME_candOfst_table->vbuf_ip_1st_cand1_offsetx != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg,  6,  8, 	ME_candOfst_table->vbuf_ip_1st_cand1_offsetx);
	if (ME_candOfst_table->vbuf_ip_1st_cand1_offsety != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg,  9, 11, 	ME_candOfst_table->vbuf_ip_1st_cand1_offsety);
	if (ME_candOfst_table->vbuf_ip_1st_cand2_offsetx != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg, 12, 14, 	ME_candOfst_table->vbuf_ip_1st_cand2_offsetx);
	if (ME_candOfst_table->vbuf_ip_1st_cand2_offsety != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg, 15, 17, 	ME_candOfst_table->vbuf_ip_1st_cand2_offsety);
	if (ME_candOfst_table->vbuf_ip_1st_cand3_offsetx != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg, 18, 20, 	ME_candOfst_table->vbuf_ip_1st_cand3_offsetx);
	if (ME_candOfst_table->vbuf_ip_1st_cand3_offsety != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg, 21, 23, 	ME_candOfst_table->vbuf_ip_1st_cand3_offsety);
	if (ME_candOfst_table->vbuf_ip_1st_cand4_offsetx != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg, 24, 26, 	ME_candOfst_table->vbuf_ip_1st_cand4_offsetx);
	if (ME_candOfst_table->vbuf_ip_1st_cand4_offsety != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg, 27, 29, 	ME_candOfst_table->vbuf_ip_1st_cand4_offsety);
	if (ME_candOfst_table->vbuf_ip_1st_cand5_offsetx != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg,  0,  2, 	ME_candOfst_table->vbuf_ip_1st_cand5_offsetx);
	if (ME_candOfst_table->vbuf_ip_1st_cand5_offsety != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg,  3,  5, 	ME_candOfst_table->vbuf_ip_1st_cand5_offsety);
	if (ME_candOfst_table->vbuf_ip_1st_cand6_offsetx != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg,  6,  8, 	ME_candOfst_table->vbuf_ip_1st_cand6_offsetx);
	if (ME_candOfst_table->vbuf_ip_1st_cand6_offsety != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg,  9, 11, 	ME_candOfst_table->vbuf_ip_1st_cand6_offsety);
	if (ME_candOfst_table->vbuf_ip_1st_cand7_offsetx != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg, 12, 14, 	ME_candOfst_table->vbuf_ip_1st_cand7_offsetx);
	if (ME_candOfst_table->vbuf_ip_1st_cand7_offsety != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg, 15, 17, 	ME_candOfst_table->vbuf_ip_1st_cand7_offsety);

	if (ME_candOfst_table->vbuf_ip_1st_update0_offsetx != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg,  0,  2, 	ME_candOfst_table->vbuf_ip_1st_update0_offsetx);
	if (ME_candOfst_table->vbuf_ip_1st_update0_offsety != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg,  3,  5, 	ME_candOfst_table->vbuf_ip_1st_update0_offsety);
	if (ME_candOfst_table->vbuf_ip_1st_update1_offsetx != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg,  6,  8, 	ME_candOfst_table->vbuf_ip_1st_update1_offsetx);
	if (ME_candOfst_table->vbuf_ip_1st_update1_offsety != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg,  9, 11, 	ME_candOfst_table->vbuf_ip_1st_update1_offsety);
	if (ME_candOfst_table->vbuf_ip_1st_update2_offsetx != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg, 12, 14, 	ME_candOfst_table->vbuf_ip_1st_update2_offsetx);
	if (ME_candOfst_table->vbuf_ip_1st_update2_offsety != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg, 15, 17, 	ME_candOfst_table->vbuf_ip_1st_update2_offsety);
	if (ME_candOfst_table->vbuf_ip_1st_update3_offsetx != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg, 18, 20, 	ME_candOfst_table->vbuf_ip_1st_update3_offsetx);
	if (ME_candOfst_table->vbuf_ip_1st_update3_offsety != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg, 21, 23, 	ME_candOfst_table->vbuf_ip_1st_update3_offsety);

	if (ME_candOfst_table->vbuf_pi_1st_cand0_offsetx != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg,  0,  2, 	ME_candOfst_table->vbuf_pi_1st_cand0_offsetx);
	if (ME_candOfst_table->vbuf_pi_1st_cand0_offsety != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg,  3,  5, 	ME_candOfst_table->vbuf_pi_1st_cand0_offsety);
	if (ME_candOfst_table->vbuf_pi_1st_cand1_offsetx != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg,  6,  8, 	ME_candOfst_table->vbuf_pi_1st_cand1_offsetx);
	if (ME_candOfst_table->vbuf_pi_1st_cand1_offsety != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg,  9, 11, 	ME_candOfst_table->vbuf_pi_1st_cand1_offsety);
	if (ME_candOfst_table->vbuf_pi_1st_cand2_offsetx != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg, 12, 14, 	ME_candOfst_table->vbuf_pi_1st_cand2_offsetx);
	if (ME_candOfst_table->vbuf_pi_1st_cand2_offsety != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg, 15, 17, 	ME_candOfst_table->vbuf_pi_1st_cand2_offsety);
	if (ME_candOfst_table->vbuf_pi_1st_cand3_offsetx != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg, 18, 20, 	ME_candOfst_table->vbuf_pi_1st_cand3_offsetx);
	if (ME_candOfst_table->vbuf_pi_1st_cand3_offsety != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg, 21, 23, 	ME_candOfst_table->vbuf_pi_1st_cand3_offsety);
	if (ME_candOfst_table->vbuf_pi_1st_cand4_offsetx != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg, 24, 26, 	ME_candOfst_table->vbuf_pi_1st_cand4_offsetx);
	if (ME_candOfst_table->vbuf_pi_1st_cand4_offsety != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg, 27, 29, 	ME_candOfst_table->vbuf_pi_1st_cand4_offsety);
	if (ME_candOfst_table->vbuf_pi_1st_cand5_offsetx != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg,  0,  2, 	ME_candOfst_table->vbuf_pi_1st_cand5_offsetx);
	if (ME_candOfst_table->vbuf_pi_1st_cand5_offsety != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg,  3,  5, 	ME_candOfst_table->vbuf_pi_1st_cand5_offsety);
	if (ME_candOfst_table->vbuf_pi_1st_cand6_offsetx != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg,  6,  8, 	ME_candOfst_table->vbuf_pi_1st_cand6_offsetx);
	if (ME_candOfst_table->vbuf_pi_1st_cand6_offsety != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg,  9, 11, 	ME_candOfst_table->vbuf_pi_1st_cand6_offsety);
	if (ME_candOfst_table->vbuf_pi_1st_cand7_offsetx != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg, 12, 14, 	ME_candOfst_table->vbuf_pi_1st_cand7_offsetx);
	if (ME_candOfst_table->vbuf_pi_1st_cand7_offsety != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg, 15, 17, 	ME_candOfst_table->vbuf_pi_1st_cand7_offsety);

	if (ME_candOfst_table->vbuf_pi_1st_update0_offsetx != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg,  0,  2, 	ME_candOfst_table->vbuf_pi_1st_update0_offsetx);
	if (ME_candOfst_table->vbuf_pi_1st_update0_offsety != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg,  3,  5, 	ME_candOfst_table->vbuf_pi_1st_update0_offsety);
	if (ME_candOfst_table->vbuf_pi_1st_update1_offsetx != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg,  6,  8, 	ME_candOfst_table->vbuf_pi_1st_update1_offsetx);
	if (ME_candOfst_table->vbuf_pi_1st_update1_offsety != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg,  9, 11, 	ME_candOfst_table->vbuf_pi_1st_update1_offsety);
	if (ME_candOfst_table->vbuf_pi_1st_update2_offsetx != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg, 12, 14, 	ME_candOfst_table->vbuf_pi_1st_update2_offsetx);
	if (ME_candOfst_table->vbuf_pi_1st_update2_offsety != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg, 15, 17, 	ME_candOfst_table->vbuf_pi_1st_update2_offsety);
	if (ME_candOfst_table->vbuf_pi_1st_update3_offsetx != CTRL_NONE)
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg, 18, 20, 	ME_candOfst_table->vbuf_pi_1st_update3_offsetx);
	if (ME_candOfst_table->vbuf_pi_1st_update3_offsety != CTRL_NONE)
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
	unsigned char table_level = 255;/* 255->load defaule */
	_str_ME_candOfst_table *tmp_ME_candOfst_table = NULL;

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

		tmp_ME_candOfst_table->vbuf_ip_1st_cand3_offsety =s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand3_offsety;
		tmp_ME_candOfst_table->vbuf_ip_1st_cand4_offsety =s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand4_offsety;
		tmp_ME_candOfst_table->vbuf_ip_1st_cand5_offsety =s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand5_offsety;
		tmp_ME_candOfst_table->vbuf_ip_1st_cand6_offsety =s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand6_offsety;
		tmp_ME_candOfst_table->vbuf_ip_1st_cand7_offsety =s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand7_offsety;

		tmp_ME_candOfst_table->vbuf_pi_1st_cand3_offsety =s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand3_offsety;
		tmp_ME_candOfst_table->vbuf_pi_1st_cand4_offsety =s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand4_offsety;
		tmp_ME_candOfst_table->vbuf_pi_1st_cand5_offsety =s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand5_offsety;
		tmp_ME_candOfst_table->vbuf_pi_1st_cand6_offsety =s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand6_offsety;
		tmp_ME_candOfst_table->vbuf_pi_1st_cand7_offsety =s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand7_offsety;
	}
	u25_me_aDTL_pre = u25_me_aDTL;

	ME_candOfst_wrtRegister(tmp_ME_candOfst_table);

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
	/*10*/ //repeat OFF
	{128,128,128,128,128,128,128,128,
	 28,36,29,40,40,40,40,48,
	 28,36,29,40,40,48,48,48,
	 CTRL_N,CTRL_N,300,300},
	 /*11*/ //case:small object
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
		table_level = 11;
		tmp_ME_caddpnt_table = fwif_MEMC_get_ME_caddpnt_table(table_level);
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
	else if(s_pContext->_output_me_sceneAnalysis.u1_Avenger_RP_detect_true == 1 )
	{
		table_level = 1;
		tmp_ME_caddpnt_table = fwif_MEMC_get_ME_caddpnt_table(table_level);

		tmp_ME_caddpnt_table->me1_ip_cddpnt_st3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st3;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st4 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st4;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st5 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st5;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st6 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st6;

		tmp_ME_caddpnt_table->me1_pi_cddpnt_st3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st3;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st4 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st4;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st5 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st5;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st6 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st6;

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

		tmp_ME_caddpnt_table->me1_ip_cddpnt_st3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st3;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st4 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st4;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st5 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st5;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st6 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st6;

		tmp_ME_caddpnt_table->me1_pi_cddpnt_st3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st3;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st4 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st4;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st5 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st5;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st6 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st6;

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

		tmp_ME_caddpnt_table->me1_ip_cddpnt_st3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st3;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st4 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st4;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st5 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st5;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st6 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st6;

		tmp_ME_caddpnt_table->me1_pi_cddpnt_st3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st3;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st4 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st4;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st5 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st5;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st6 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st6;

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

		tmp_ME_caddpnt_table->me1_ip_cddpnt_rnd3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_rnd3;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_rnd3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_rnd3;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_gmv = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_gmv;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_gmv = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_gmv;
	}
	else if(s_pContext->_output_me_sceneAnalysis.u1_Region_Periodic_detect_true == 3 &&(s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVx_Step!=0)&& (s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVy_Step!=0) )
	{
		table_level = 5;
		tmp_ME_caddpnt_table = fwif_MEMC_get_ME_caddpnt_table(table_level);

		tmp_ME_caddpnt_table->me1_ip_cddpnt_st3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st3;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st4 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st4;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st5 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st5;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st6 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st6;

		tmp_ME_caddpnt_table->me1_pi_cddpnt_st3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st3;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st4 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st4;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st5 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st5;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st6 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st6;

		tmp_ME_caddpnt_table->me1_ip_cddpnt_rnd3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_rnd3;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_rnd3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_rnd3;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_gmv = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_gmv;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_gmv = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_gmv;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_zmv = 255;
	}
	else if(s_pContext->_output_me_sceneAnalysis.u1_zmv_on_flag == 1 )
	{
		table_level = 6;
		tmp_ME_caddpnt_table = fwif_MEMC_get_ME_caddpnt_table(table_level);
		
		tmp_ME_caddpnt_table->me1_ip_cddpnt_rnd3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_rnd3;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_rnd3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_rnd3;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_gmv = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_gmv;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_gmv = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_gmv;
	}
	else if(s_pParam->_param_me_sceneAnalysis.b_FixRepeat_en &&
		s_pContext->_output_me_sceneAnalysis.u8_RepeatMode==RepeatMode_OFF)
	{
		table_level = 10;
		tmp_ME_caddpnt_table = fwif_MEMC_get_ME_caddpnt_table(table_level);
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

		tmp_ME_caddpnt_table->me1_ip_cddpnt_st3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st3;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st4 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st4;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st5 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st5;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_st6 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st6;

		tmp_ME_caddpnt_table->me1_pi_cddpnt_st3 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st3;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st4 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st4;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st5 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st5;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_st6 = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st6;

		tmp_ME_caddpnt_table->me1_ip_cddpnt_gmv = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_gmv;
		tmp_ME_caddpnt_table->me1_pi_cddpnt_gmv = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_gmv;
		tmp_ME_caddpnt_table->me1_ip_cddpnt_zmv = 255;
	}
	if(u32_ID_Log_en==1)//0xB809D758 [28]
	{
		static unsigned char counter = 0;
		if(counter % 10 == 0)
			rtd_pr_memc_info("[%s][%d]table_level =  %d",__FUNCTION__, __LINE__,table_level);
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
		rtd_pr_memc_emerg("[Random mask][table_level][%d]\n", table_level);
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

static _str_ME_ipme_flp_alp_table   ME_ipme_flp_alp_table[5]={
	{CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N,CTRL_N},	/*0.load default*/
	{0x16,0x16,0x14,0x14,0x12,0x12,0x10,0x16,0x16,0x14,0x14,0x12,0x12,0x10},	/*1*/
	{0x20,0x1c,0x18,0x14,0x10,0xe,0xa,0x20,0x1c,0x18,0x14,0x10,0xe,0xa},	/*2*/ //repeat MID/HIGH
	{0x30,0x26,0x1C,0x12,0x8,0x7,0x5,0x30,0x26,0x1C,0x12,0x8,0x7,0x5},		/*3*/ //repeat LOW
	{0x40,0x30,0x20,0x10,0x0,0x0,0x0,0x40,0x30,0x20,0x10,0x0,0x0,0x0},		/*4*/ //repeat OFF
};

_str_ME_ipme_flp_alp_table *fwif_MEMC_get_ME_ipme_flp_alp_table(unsigned char level)
{
//	if (ME_ipme_flp_alp_table == NULL) {
//		return NULL;
//	}
//	if( level==255 || level==0)
	if(level > 4)
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
	else if(s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type==2)
	{
		table_level = 1;
		tmp_ME_ipme_flp_alp_table = fwif_MEMC_get_ME_ipme_flp_alp_table(table_level);
	}
	else if((s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true >= 1 && s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true !=3)|| s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type==1 )
	{
		table_level = 2;
		tmp_ME_ipme_flp_alp_table = fwif_MEMC_get_ME_ipme_flp_alp_table(table_level);
	}
	else if(s_pParam->_param_me_sceneAnalysis.b_FixRepeat_en &&
		s_pContext->_output_me_sceneAnalysis.u8_RepeatMode==RepeatMode_OFF) {
		table_level = 4;
		tmp_ME_ipme_flp_alp_table = fwif_MEMC_get_ME_ipme_flp_alp_table(table_level);
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
		rtd_pr_memc_emerg("[FLP ALP][table_level][%d] RM:%d\n", table_level, s_pContext->_output_me_sceneAnalysis.u8_RepeatMode);
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

void MEMC_me1_bgmask_special_setting(const _PARAM_WRT_COM_REG *pParam , _OUTPUT_WRT_COM_REG *pOutput ,int th0 ,int gmv_mvx_final)
{ 
    const _PQLCONTEXT *s_pContext = GetPQLContext();
    signed short u11_gmv_mvx;
    signed short u10_gmv_mvy;
    
    static int holdframe128 , holdframe13,  holdframe339   ,holdframe299 ,holdframe189 , holdframe185 , holdframe317,holdframe226 ,holdframe192 ,holdframe72,holdframe200;
    static int holdframe306_2  , holdgmv399frame,holdframe183 , holdframe151 , holdframe432,holdframe195 ,holdframe214 ,holdframe317_2 ,holdframe317_3 ,holdframe315;
    static int holdframe_96,holdframe_96_2 ,holdframe_421 ,holdframe431 ,holdframe163 ,holdframe49,holdframe343 ,FrameHold_PQ_Issue ;
    static bool video96,video96_2,video421,video219,holdframe432final,video343 ,FrameHoldflag_PQ_Issue;
    static bool video2271,video2272,video2273,video2274,video2275  ,video432final;

    static bool video128 , video339   ,video299 ,video189  , video185, video13 ,video317,video226 ,video192 ,video72 ,video183,video200 ;	
    static bool video214 ,video317_2 ,video317_3,video315 ,video431 ,video163,video49 ;	
    static bool video306_2 ,video151 ,video432,video195,video_transporter,video_dance,video_smallplane,video_soccer,video_dance2,video_skating,video_C001soccer,video30006,video_rotterdam,video_ridebike,video_toleftbike;	
	static int video_0026,video_insect,video_eagle,video_huapi2;

    int fwcontrol_128 ,fwcontrol3   ,fwcontrol_video339  ,fwcontrol_video299, fwcontrol_video189, fwcontrol_185,fwcontrol_13,fwcontrol_1282; //fwcontrol1 ,fwcontrol2 ,fwcontrol3 ;
    int fwcontrol_306, fwcontrol_214; 
    static int holdframe182,holdframe393,holdframe416,holdframe412,holdframe410,holdframe2144,holdframe419,holdframe1922,holdframe1511,holdframe1288;
    static bool video182,video393,video416,video412,video410,video2144,video419,video1922,video1511,video1288;
	static int video_IDT;
    int rgnIdx;
    //int path2 =0;
    int path3 = 0;
    int path4 = 0;
    int rmvx[32]={0},rmvy[32]={0};
    

    int bg_s3_dubt_cov_num_yth1_init=0xa;
    int bg_s3_dubt_cov_num_yth2_init=0x22;
    int bg_s3_dubt_cov_shift_init=0x1;
    int bg_s3_dubt_cov_slope_init=0x2;
    int bg_s3_dubt_ucov_yth1_init=0x8;
    int bg_s3_dubt_ucov_yth2_init=0x1e;
    int bg_s3_dubt_ucov_shift_init=0x1;
    int bg_s3_dubt_ucov_slope_init=0x3;
    int bg_s3_dubt_cov_sad_th_init = 0x1c2 ;  
    int fg_apl_s2_protect_th0_init=0x0;
    int fg_apl_s2_protect_th1_init=0x1e;
    int fg_apl_s2_protect_bgnum_th_init=0x32;  
    int fg_apl_s3_protect_th0_init=0x0;
    int fg_apl_s3_protect_th1_init=0x28;
    int fg_apl_s3_protect_bgnum_th_init=0x2d; 

    int bg_s3_cov_largesad_th_init=0x12c;
    int bg_s3_cov_smallsad_th_init=0x78;
    int bg_s3_cov_setbg_th_init=0x190;
    int bg_s3_dubt_ucov_sad_th_init = 0x1c2 ;  
    int bg_s3_ucov_smallsad_th_init=0x78;
    int bg_s3_ucov_largesad_th_init=0x12c;
    int bg_s3_ucov_setbg_sad_th_init=0x190;   
    int fg_apl_s2_protect_th0_299=0x96;
    int fg_apl_s2_protect_th1_299=0xfa;
    int fg_apl_s2_protect_bgnum_th_299=0x2d;   
    int fg_apl_s3_protect_th0=0x96;
    int fg_apl_s3_protect_th1=0xfa;
    int fg_apl_s3_protect_bgnum_th=0x2d;        
    int fg_mask_pfv_blend_dis=0x0; 
    //int pfv_corr_en=0x1;
    int fg_mask_pfv_blend_en_init=0x1;
    // int pfv_corr_en_init=0x0;
    int bg_s3_ucov_data_en_init=0x1;
    int bg_s3_cov_data_en_init=0x1;
    int fg_hold_en_init=0x0;
    int set2_ycenter_init = 536 ;
    int set2_ucenter_init = 432 ;
    int set2_vcenter_init =  590 ;
    int set2_yrange_init = 400 ; //300
    int set2_urange_init = 100 ; //60
    int set2_vrange_init =  100 ;   //72    
    int bg_color_protect_en_off = 0 ;  // init =1 
    int bg_color_protect_en_init =1 ;
    int set1_ycenter_init = 0x190 ;
    int set1_ucenter_init = 0x1b8 ;
    int set1_vcenter_init =  0x262 ;
    int set1_yrange_init = 0x12c ; 
    int set1_urange_init = 0x30 ; 
	int set1_vrange_init =  0x30 ;  
    signed short fg_apl_seg0_cnt=s_pContext->_output_read_comreg.dh_fg_apl_seg0_cnt;


    u11_gmv_mvx = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
    u10_gmv_mvy = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;
    for(rgnIdx =0 ; rgnIdx< 32; rgnIdx++)
    {
        
        rmvx[rgnIdx] =    s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[rgnIdx]  ;
        rmvy[rgnIdx] =    s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[rgnIdx]  ;        
    }
	
    
    //+++++++++++video 128  code is disable ++++++++++++++++//
    ReadRegister(HARDWARE_HARDWARE_25_reg, 0, 0 , &fwcontrol_128);
    ReadRegister(HARDWARE_HARDWARE_25_reg, 3,3 , &fwcontrol3);  
    ReadRegister(HARDWARE_HARDWARE_25_reg, 7, 7 , &fwcontrol_video339); 
    ReadRegister(HARDWARE_HARDWARE_25_reg , 9, 9, &fwcontrol_video189);
    ReadRegister(HARDWARE_HARDWARE_25_reg , 8, 8, &fwcontrol_video299);
    ReadRegister(HARDWARE_HARDWARE_25_reg, 10,10 , &fwcontrol_185); 
    ReadRegister(HARDWARE_HARDWARE_25_reg, 11,11 , &fwcontrol_13);
    ReadRegister(HARDWARE_HARDWARE_25_reg, 17,17 , &fwcontrol_306); 
    ReadRegister(HARDWARE_HARDWARE_25_reg, 6,6 , &fwcontrol_214);

    if((s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_black_apl==1) && (!fwcontrol_214))
    {

        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,0,0,0x1); //s2_protect_en
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,fg_apl_s2_protect_th0_init);	    	    
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,  5); 	     
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,fg_apl_s2_protect_bgnum_th_init);

        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,fg_apl_s3_protect_th0_init);   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,  5);   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,fg_apl_s3_protect_bgnum_th_init);  
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,0x0);
        holdframe226 = 60;  
        video226=1;
        WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15, 10);     
        WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,0,7,0x0);
        path3 = 0x3 ;
    } 
    else if(holdframe226>0)
    {

        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,0,0,0x1); //s2_protect_en
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,fg_apl_s2_protect_th0_init);	    	    
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,  5); 	     
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,fg_apl_s2_protect_bgnum_th_init);

        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,fg_apl_s3_protect_th0_init);   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,  5);   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,fg_apl_s3_protect_bgnum_th_init);  
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,0x0);

        WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15, 10);     
        WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,0,7,0x0);
         path3 = 0x3 ;
         holdframe226 -- ; 
    }
    else if(video226)
    {
        video226 = 0 ;

        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,0,0,0x1); //s2_protect_en
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,fg_apl_s2_protect_th0_init);	    	    
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,fg_apl_s2_protect_th1_init); 	     
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,fg_apl_s2_protect_bgnum_th_init);

        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,fg_apl_s3_protect_th0_init);   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,fg_apl_s3_protect_th1_init);   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,fg_apl_s3_protect_bgnum_th_init);  
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,0x1);
        WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x22);     
        WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,0,7,0x0);
         path3 = 0  ;
    } 
    //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg,  0,  3, (holdframe226>0)); 

   //+++++++++++++++++192 horn close th skin protect ++++++++++++++//

    if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_horn_skin_off==1)
    {
        holdframe192 = 90;  
        video192=1;
        path3 = 0x3 ;

        WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,26,26,bg_color_protect_en_off);  // init =1 
        WriteRegister(KME_ME1_BG1_ME_BG_S4_1_reg,7,7,0x1);  //mv_diff_only_hor_en
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,9);  //bg_s1_num_yth2
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,1);  //bg_s1_num_yth1
       // WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,12);//bg_s2_num_yth2
      //  WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,1); //bg_s2_num_yth1   
        WriteRegister(HARDWARE_HARDWARE_25_reg, 18,18 , 1);
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31, 0x0);  //bg_s3_cov_data_en     
    }
    else if(holdframe192>0)
    {
         path3 = 0x3 ;
         holdframe192 -- ; 
        WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,26,26,bg_color_protect_en_off);  // init =1 
        WriteRegister(KME_ME1_BG1_ME_BG_S4_1_reg,7,7,0x1);  //mv_diff_only_hor_en
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,9);  //bg_s1_num_yth2
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,1);  //bg_s1_num_yth1
      //  WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,12);//bg_s2_num_yth2
      //  WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,1); //bg_s2_num_yth1      
        WriteRegister(HARDWARE_HARDWARE_25_reg, 18,18 , 1);
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31, 0x0);  //bg_s3_cov_data_en     
    }
    else if(video192)
    {
         video192 = 0 ;
       //  path2 = 0 ;
        WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,26,26,bg_color_protect_en_init);  // init =1 
        WriteRegister(KME_ME1_BG1_ME_BG_S4_1_reg,7,7,0x0);  //mv_diff_only_hor_en
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x10);  //bg_s1_num_yth2
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0x4);  //bg_s1_num_yth1
      //  WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x14);//bg_s2_num_yth2
      //  WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0xa); //bg_s2_num_yth1    
        WriteRegister(HARDWARE_HARDWARE_25_reg, 18,18 , 0);
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,bg_s3_cov_data_en_init);  //bg_s3_cov_data_en   

    } 


		if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_128_flag )
		{
			WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,0); 		 //s2_low	 
			WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,15);	//		 
			WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,0);	 //s3_low	 
			WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,15); 

			WriteRegister(KME_ME1_BG0_ME1_BG_3_reg, 1,7,0x28);	//s2_protect_ num_th
			WriteRegister(KME_ME1_BG0_ME1_BG_11_reg, 1,7, 25);	//s3_protect_ num_th
			//	WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,26,26,bg_color_protect_en_off);  // init =1 
			holdframe128 =	(s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 120 : 60 ;
			video128=1;
            WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,100); 
            WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,100); 
            WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,100); 
            WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,105); 
            WriteRegister(HARDWARE_HARDWARE_25_reg, 21,21 , 0x1);
             path3=4 ;
		}
		else if(holdframe128>0)
		{
			holdframe128 -- ;
			WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,0); 		 //s2_low	 
			WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,15);	//		 
			WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,0);	 //s3_low	 
			WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,15); 

			WriteRegister(KME_ME1_BG0_ME1_BG_3_reg, 1,7,0x28);	//s2_protect_ num_th
			WriteRegister(KME_ME1_BG0_ME1_BG_11_reg, 1,7, 25);	//s3_protect_ num_th
			// WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,26,26,bg_color_protect_en_off);	// init =1 
			WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,100); 
			WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,100); 
			WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,100); 
			WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,105); 
			WriteRegister(HARDWARE_HARDWARE_25_reg, 21,21 , 0x1);
			  path3=4 ;
		}
		else if(video128)
		{
			video128=0;
			WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,fg_apl_s2_protect_th0_init);			 //s2_low	 
			WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,fg_apl_s2_protect_th1_init);	//		 
			WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,fg_apl_s3_protect_th0_init);	  //s3_low	  
			WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,fg_apl_s3_protect_th1_init); 

			WriteRegister(KME_ME1_BG0_ME1_BG_3_reg, 1,7,fg_apl_s2_protect_bgnum_th_init);  //s2_protect_ num_th
			WriteRegister(KME_ME1_BG0_ME1_BG_11_reg, 1,7, fg_apl_s3_protect_bgnum_th_init);  //s3_protect_ num_th
			WriteRegister(HARDWARE_HARDWARE_25_reg, 21,21 , 0x0);
            // WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,26,26,bg_color_protect_en_init);  // init =1 
               path3=0 ;

		} 
		
        
		ReadRegister(HARDWARE_HARDWARE_25_reg, 21,21 , &fwcontrol_1282);
			
	 //+++++++++++video 339 code is enable	++++++++++++++++//

    if(fwcontrol_video339)
    {
        holdframe339 = 5 ;
        video339 = 1 ;    
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,  0);	    	    
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,70); 	  //s2_protect_th1     
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7, 66);  //s2_protect_bgnum_th

        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23, 0 );   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,70);    // s3_protect_th1 
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7, 65);  //s3_protect_bgnum_th
        WriteRegister(KME_ME1_BG0_ME1_BG_4_reg,24,24,fg_mask_pfv_blend_dis);   
     //   WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,5,5,fg_hold_en_init); 
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,0,7,0x0);  //range_min_x_2
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,8,15,0xef);  //range_max_x_2
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,16,23,0x0);  //range_min_y_2
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,24,31,0x86);   //range_max_y_2
        WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,22,31,bg_s3_cov_largesad_th_init+50);  //300
        WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,12,21,bg_s3_cov_smallsad_th_init);  //120
        WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,2,11,bg_s3_cov_setbg_th_init);    //400
        WriteRegister(KME_ME1_BG0_ME1_BG_13_reg,1,10,0x1f4);   //s3_dubt_cov_sad   500         

        if( _ABS_(u11_gmv_mvx)>12)
        {
            WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,20); 
            WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,20); 
            WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,20); 
            WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,22);  
            WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, -30);
         //   rtd_pr_memc_info("339 pattern \n");   
            holdgmv399frame = 8 ; 
        }
        else if(holdgmv399frame>0)
        {
            WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,20); 
            WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,20); 
            WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,20); 
            WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,22);  
            WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, -30);
            holdgmv399frame --;
    }
     else
    {

        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,th0+8); 
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,th0+8); 
        WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,th0+8); 
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,th0+10);  
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, gmv_mvx_final);

    }
     
		 WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,0);	//bg_s3_cov_data_en   
		path3=4 ;
    }
    else if(holdframe339>0)
    { 

        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,  0);	    	    
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,70); 	  //s2_protect_th1     
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7, 66);  //s2_protect_bgnum_th

        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23, 0 );   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,70);    // s3_protect_th1 
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7, 65);  //s3_protect_bgnum_th
        WriteRegister(KME_ME1_BG0_ME1_BG_4_reg,24,24,fg_mask_pfv_blend_dis); 
      //  WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,5,5,fg_hold_en_init); 
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,0,7,0x0);  //range_min_x_2
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,8,15,0xef);  //range_max_x_2
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,16,23,0x0);  //range_min_y_2
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,24,31,0x86);   //range_max_y_2
        WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,22,31,bg_s3_cov_largesad_th_init+50);  //300
        WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,12,21,bg_s3_cov_smallsad_th_init);  //120
        WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,2,11,bg_s3_cov_setbg_th_init+50);    //400
        WriteRegister(KME_ME1_BG0_ME1_BG_13_reg,1,10,0x1f4);   //s3_dubt_cov_sad   500 

        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,th0+8); 
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,th0+8); 
        WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,th0+8); 
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,th0+10);  
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, gmv_mvx_final);

        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,0);  //bg_s3_cov_data_en   
        holdframe339 --;
        path3=4 ;
     }
    else if(video339)
    {
        video339 =0 ;
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,th0); 
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,th0); 
        WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,th0); 
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,th0+5); 

        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,fg_apl_s2_protect_th0_init);	    	    
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,fg_apl_s2_protect_th1_init); 	     
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,fg_apl_s2_protect_bgnum_th_init);

        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,fg_apl_s3_protect_th0_init);   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,fg_apl_s3_protect_th1_init);   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,fg_apl_s3_protect_bgnum_th_init); 
        WriteRegister(KME_ME1_BG0_ME1_BG_4_reg,24,24,fg_mask_pfv_blend_en_init);   
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,0,7,0x0);  //range_min_x_2
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,8,15,0);  //range_max_x_2
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,16,23,0x0);  //range_min_y_2
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,24,31,0);   //range_max_y_2
        WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,22,31,bg_s3_cov_largesad_th_init);  //300
        WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,12,21,bg_s3_cov_smallsad_th_init);  //120
        WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,2,11,bg_s3_cov_setbg_th_init);    //400
        WriteRegister(KME_ME1_BG0_ME1_BG_13_reg,1,10,bg_s3_dubt_cov_sad_th_init);   //s3_dubt_cov_sad
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, gmv_mvx_final);

        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,bg_s3_cov_data_en_init);  //bg_s3_cov_data_en   
        path3=0 ;
    }


    if(fwcontrol_video299)
    {
        holdframe299 = 30 ;
        video299 = 1 ; 
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,fg_apl_s2_protect_th0_299);	    	    
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,fg_apl_s2_protect_th1_299); 	     
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,fg_apl_s2_protect_bgnum_th_299);

        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,fg_apl_s3_protect_th0);   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,fg_apl_s3_protect_th1);   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,fg_apl_s3_protect_bgnum_th);

        if(pOutput->u1_me1_bg_pfv_corr_cond)
        {
            WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,25);   //full range 
            WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,240);
            WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23, 41);
            WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,135);
        }
        WriteRegister(KME_ME1_BG0_ME1_BG_4_reg,24,24,fg_mask_pfv_blend_dis);   

        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,bg_s3_ucov_data_en_init);
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,bg_s3_cov_data_en_init);


        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,33); 
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,33); 
        WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,33); 
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,35); 
		path3=5 ;

    }
    else if(holdframe299>0)
    { 

        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,fg_apl_s2_protect_th0_299);	    	    
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,fg_apl_s2_protect_th1_299); 	     
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,fg_apl_s2_protect_bgnum_th_299);

        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,fg_apl_s3_protect_th0);   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,fg_apl_s3_protect_th1);   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,fg_apl_s3_protect_bgnum_th);
        if(pOutput->u1_me1_bg_pfv_corr_cond)
        {
            WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,25);   //full range 
            WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,240);
            WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23, 41);
            WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,135);
        }
        //WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,corrth); 
        WriteRegister(KME_ME1_BG0_ME1_BG_4_reg,24,24,fg_mask_pfv_blend_dis);   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,bg_s3_ucov_data_en_init);
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,bg_s3_cov_data_en_init);

        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,33); 
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,33); 
        WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,33); 
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,35);  
  
        holdframe299 --;
		path3=5 ;
    }
    else if(video299)
    {
        video299 =0 ;

        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,fg_apl_s2_protect_th0_init);	    	    
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,fg_apl_s2_protect_th1_init); 	     
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,fg_apl_s2_protect_bgnum_th_init);

        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,fg_apl_s3_protect_th0_init);   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,fg_apl_s3_protect_th1_init);   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,fg_apl_s3_protect_bgnum_th_init);   

        WriteRegister(KME_ME1_BG0_ME1_BG_4_reg,24,24,fg_mask_pfv_blend_en_init);  

        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,th0); 
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,th0); 
        WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,th0); 
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,th0+5); 
		path3=0 ;
              
    }


    if(fwcontrol_video189)
    {
        holdframe189= 15 ;
        video189 = 1 ; 
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15, 160);	    	    
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,fg_apl_s2_protect_th1_299); 	     
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,fg_apl_s2_protect_bgnum_th_299);

        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,160);   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,fg_apl_s3_protect_th1);   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,fg_apl_s3_protect_bgnum_th);
		path3=6 ;

        if(pOutput->u1_me1_bg_pfv_corr_cond)
        {

            WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7, 89);   //full range 
            WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,240);
            WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,28);
            WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,134);

        }
        WriteRegister(KME_ME1_BG0_ME1_BG_4_reg,24,24,fg_mask_pfv_blend_dis);   

        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,bg_s3_ucov_data_en_init);
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,bg_s3_cov_data_en_init);

        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,33); 
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,33); 
        WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,33); 
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,35); 

        // WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 0, 3, 0x5); 

        //    path2 = 5 ;

    }
    else if(holdframe189>0)
    {
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15, 160);	    	    
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,fg_apl_s2_protect_th1_299); 	     
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,fg_apl_s2_protect_bgnum_th_299);

        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,160);   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,fg_apl_s3_protect_th1);   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,fg_apl_s3_protect_bgnum_th);


        if(pOutput->u1_me1_bg_pfv_corr_cond)
        {

            WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7, 89);   //full range 
            WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,240);
            WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,28);
            WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,134);

        }
            WriteRegister(KME_ME1_BG0_ME1_BG_4_reg,24,24,fg_mask_pfv_blend_dis);   

            WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,bg_s3_ucov_data_en_init);
            WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,bg_s3_cov_data_en_init);

            WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,33); 
            WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,33); 
            WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,33); 
            WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,35); 
        //   path2=6 ;

        //   WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 0, 3, 0x6); 
        holdframe189--;
		path3=6;
    }
    else if(video189)
    {
        video189 =0 ;

        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,fg_apl_s2_protect_th0_init);	    	    
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,fg_apl_s2_protect_th1_init); 	     
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,fg_apl_s2_protect_bgnum_th_init);

        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,fg_apl_s3_protect_th0_init);   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,fg_apl_s3_protect_th1_init);   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,fg_apl_s3_protect_bgnum_th_init);   

        WriteRegister(KME_ME1_BG0_ME1_BG_4_reg,24,24,fg_mask_pfv_blend_en_init);  

        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,th0); 
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,th0); 
        WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,th0); 
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,th0+5); 

    //  WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 0, 3, 0x7);       
		path3= 0 ;
    }

  
       //+++++++++++video 185 s1 s2 curve adjust ++++++++++++++++//

    if(fwcontrol_185)
    {
        holdframe185 = 50 ;
        video185 = 1 ;    
        //  WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12, 20);  //bg_s1_num_yth2
        //   WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17, 12);  //bg_s1_num_yth1
        //  WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,20);//bg_s2_num_yth2
        //  WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,13); //bg_s2_num_yth1   
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12, 8);  //bg_s1_num_yth2
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17, 1);  //bg_s1_num_yth1
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12, 10);//bg_s2_num_yth2
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,1); //bg_s2_num_yth1  


      //  WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,0,0,0x1);  //s1_clr_prot_en
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,28,31,0x1);
     //   WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,0,0,0x1); //s2_clr_prot_en
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,28,31,0x1);

        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_1_reg,31,31,0x1);  //range1_en
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_1_reg,20,29,640);  //v center 
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_1_reg,10,19,470); // u center
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_1_reg,0,9,90);  //y center
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_2_reg,18,26,60);  //v range
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_2_reg,9,17,60);  //u range
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_2_reg,0,8,70);  //y range
        WriteRegister(KME_ME1_BG0_ME1_BG_4_reg,24,24,fg_mask_pfv_blend_dis);   
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,0,7,0x0);  //range_min_x_2
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,8,15,0xef);  //range_max_x_2
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,16,23,24);  //range_min_y_2
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,24,31,0x86);   //range_max_y_2
   		path3 = 7 ;
  
    }
    else if(holdframe185>0)
    { 

        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12, 20);  //bg_s1_num_yth2
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17, 11);  //bg_s1_num_yth1
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,20);//bg_s2_num_yth2
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,11); //bg_s2_num_yth1  

      //  WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,0,0,0x1);  //s1_clr_prot_en
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,28,31,0x1);
     //   WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,0,0,0x1); //s2_clr_prot_en
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,28,31,0x1);

        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_1_reg,31,31,0x1);  //range1_en
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_1_reg,20,29,640);  //v center 
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_1_reg,10,19,470); // u center
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_1_reg,0,9,90);  //y center
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_2_reg,18,26,60);  //v range
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_2_reg,9,17,60);  //u range
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_2_reg,0,8,70);  //y range
        WriteRegister(KME_ME1_BG0_ME1_BG_4_reg,24,24,fg_mask_pfv_blend_dis);   
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,0,7,0x0);  //range_min_x_2
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,8,15,0xef);  //range_max_x_2
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,16,23,24);  //range_min_y_2
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,24,31,0x86);   //range_max_y_2
       	path3 = 7 ;   
        holdframe185 --;

    }
    else if(video185)
    {
        video185 =0 ;
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x10);  //bg_s1_num_yth2
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0x4);  //bg_s1_num_yth1
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x14);//bg_s2_num_yth2
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0xa); //bg_s2_num_yth1         
       // WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,0,0,0x0);  //s1_clr_prot_en

        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,28,31,0x3);
     //   WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,0,0,0x0); //s2_clr_prot_en

        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,28,31,0x3);

        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_1_reg,31,31,0x0);  //range1_en
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_1_reg,20,29,set2_vcenter_init);  //v center 
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_1_reg,10,19,set2_ucenter_init); // u center
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_1_reg,0,9,set2_ycenter_init);  //y center
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_2_reg,18,26,set2_vrange_init);  //v range
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_2_reg,9,17,set2_urange_init);  //u range
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_2_reg,0,8,set2_yrange_init);  //y range
        WriteRegister(KME_ME1_BG0_ME1_BG_4_reg,24,24,fg_mask_pfv_blend_en_init);  

        //      WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,0x19); //gmv y0
        path3 = 0 ;
    }     

    
    if(fwcontrol_13)
    {
        holdframe13 = 20 ;
        video13 = 1 ;    

          path3 =7 ;
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,1); //bg_s1_num_yth2
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0x1); //bg_s1_num_yth1
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,18,22,0xa);
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,23,27,0x2);
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,28,31,0x1);  //bg_s1_slope

        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x1);  //bg_s2_num_yth2
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0x1); //bg_s2_num_yth1
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,18,22,0xc);
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,23,27,0x2);
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,28,31,0x1); //bg_s2_slope

        WriteRegister(KME_ME1_BG0_ME1_BG_13_reg,1,10,650);   //s3_dubt_cov_sad    
        WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,22,31,600);
        WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,12,21,250);
        WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,2,11,600); 

        WriteRegister(KME_ME1_BG1_ME_BG_2x2_3_reg,0,9,650);  //cov_dubt_ucov_sad_th
        WriteRegister(KME_ME1_BG1_ME_BG_S3_0_reg,20,29,600);  
        WriteRegister(KME_ME1_BG1_ME_BG_S3_0_reg,10,19,600);        
        WriteRegister(KME_ME1_BG1_ME_BG_S3_0_reg,0,9,250);    

        // WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,0,0,0x1);  //s1_clr_prot_en
        //   WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,0,0,0x1); //s2_clr_prot_en
        WriteRegister(KME_ME1_BG0_ME1_BG_13_reg,26,26,0x0); //bg_s3_sad_sum_en
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,5,5, fg_hold_en_init); 

        WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,8,14,20); //bg_s3_dubt_cov_num_yth1
        WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,15,21,4);  //bg_s3_dubt_cov_num_yth1
        WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,4,7,0); //bg_s3_dubt_cov_shift
        WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,0,3,1); //bg_s3_dubt_cov_slope

        WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,15,21,4);  //bg_s3_dubt_ucov_yth1
        WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,8,14,20);  //bg_s3_dubt_ucov_yth2
        WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,4,7,0);  //bg_s3_dubt_ucov_shift
        WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,0,3,1);  //bg_s3_dubt_ucov_slope
        
    } 
    else if(holdframe13)
    {
        holdframe13--;
        path3 =7 ;
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,1); //bg_s1_num_yth2
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0x1); //bg_s1_num_yth1
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,18,22,0xa);
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,23,27,0x2);
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,28,31,0x1);  //bg_s1_slope

        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x1);  //bg_s2_num_yth2
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0x1); //bg_s2_num_yth1
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,18,22,0xc);
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,23,27,0x2);
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,28,31,0x1); //bg_s2_slope

        WriteRegister(KME_ME1_BG0_ME1_BG_13_reg,1,10,650);   //s3_dubt_cov_sad    
        WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,22,31,600);
        WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,12,21,250);
        WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,2,11,600); 

        WriteRegister(KME_ME1_BG1_ME_BG_2x2_3_reg,0,9,650);  //cov_dubt_ucov_sad_th
        WriteRegister(KME_ME1_BG1_ME_BG_S3_0_reg,20,29,600);  
        WriteRegister(KME_ME1_BG1_ME_BG_S3_0_reg,10,19,600);        
        WriteRegister(KME_ME1_BG1_ME_BG_S3_0_reg,0,9,250);    

     //   WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,0,0,0x1);  //s1_clr_prot_en
     //   WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,0,0,0x1); //s2_clr_prot_en
        WriteRegister(KME_ME1_BG0_ME1_BG_13_reg,26,26,0x0); //bg_s3_sad_sum_en
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,5,5, fg_hold_en_init);
        
        WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,8,14,20); //bg_s3_dubt_cov_num_yth1
        WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,15,21,4);  //bg_s3_dubt_cov_num_yth1
        WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,4,7,0); //bg_s3_dubt_cov_shift
        WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,0,3,1); //bg_s3_dubt_cov_slope
        
        WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,15,21,4);  //bg_s3_dubt_ucov_yth1
        WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,8,14,20);  //bg_s3_dubt_ucov_yth2
        WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,4,7,0);  //bg_s3_dubt_ucov_shift
        WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,0,3,1);  //bg_s3_dubt_ucov_slope
    }

    else if(video13)
    {
        video13 =0 ;
        path3 =0 ;

        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x10);
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0x4);
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,18,22,0xa);
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,23,27,0x2);
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,28,31,0x3);

        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x14);
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0xa);
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,18,22,0xc);
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,23,27,0x2);
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,28,31,0x3);

        WriteRegister(KME_ME1_BG0_ME1_BG_13_reg,1,10,bg_s3_dubt_cov_sad_th_init);   //s3_dubt_cov_sad    
        WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,22,31,bg_s3_cov_largesad_th_init);
        WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,12,21,bg_s3_cov_smallsad_th_init);
        WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,2,11,bg_s3_cov_setbg_th_init); 

        WriteRegister(KME_ME1_BG1_ME_BG_2x2_3_reg,0,9,bg_s3_dubt_ucov_sad_th_init);  //cov_dubt_ucov_sad_th
        WriteRegister(KME_ME1_BG1_ME_BG_S3_0_reg,20,29,bg_s3_ucov_smallsad_th_init);  
        WriteRegister(KME_ME1_BG1_ME_BG_S3_0_reg,10,19,bg_s3_ucov_largesad_th_init);        
        WriteRegister(KME_ME1_BG1_ME_BG_S3_0_reg,0,9,bg_s3_ucov_setbg_sad_th_init);    

      //  WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,0,0,0x0);  //s1_clr_prot_en
     //   WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,0,0,0x0); //s2_clr_prot_en
        WriteRegister(KME_ME1_BG0_ME1_BG_13_reg,26,26,0x1); //bg_s3_sad_sum_en
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,5,5,0x1);  //fg_hold_en


        WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,8,14,bg_s3_dubt_cov_num_yth2_init);
        WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,15,21,bg_s3_dubt_cov_num_yth1_init);
        WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,4,7,bg_s3_dubt_cov_shift_init);
        WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,0,3,bg_s3_dubt_cov_slope_init);
        
        WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,15,21,bg_s3_dubt_ucov_yth1_init);
        WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,8,14,bg_s3_dubt_ucov_yth2_init);
        WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,4,7,bg_s3_dubt_ucov_shift_init);
        WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,0,3,bg_s3_dubt_ucov_slope_init);

    }

    if(fwcontrol_306&&fg_apl_seg0_cnt<2100)
    {
        video306_2 =1 ;
        holdframe306_2 = 180 ;     
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,fg_apl_s2_protect_th0_init);	    	    
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,40); 	     
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,66);

        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,fg_apl_s3_protect_th0_init);   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,40);   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,66);  


     //   WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg,24,24,0x1);  //s4_apl_force_bg_en
    //   WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg,8,15,180);  //s4_apl_force_bg_th0
    //   WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg,0,7,0xff);  //s4_apl_force_bg_th1

        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg, 0, 0, 0x1); //me1 black protect
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,0,0,0x1); //me1 black protect en

   		path3=8 ;
    }
    else if(holdframe306_2>0)
    {
        holdframe306_2--;
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,fg_apl_s2_protect_th0_init);	    	    
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,35); 	     
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,66);

        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,fg_apl_s3_protect_th0_init);   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,35);   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,66);     

     //  WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg,24,24,0x1);  //s4_apl_force_bg_en
     //  WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg,8,15,180);  //s4_apl_force_bg_th0
       // WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg,0,7,0xff);  //s4_apl_force_bg_th1

        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg, 0, 0, 0x1); //me1 black protect
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,0,0,0x1); //me1 black protect en
       	path3=8 ;
    }
    else if(video306_2)
    {
        video306_2 = 0 ;
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,fg_apl_s2_protect_th0_init);	    	    
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,fg_apl_s2_protect_th1_init); 	     
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,fg_apl_s2_protect_bgnum_th_init);

        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,fg_apl_s3_protect_th0_init);   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,fg_apl_s3_protect_th1_init);   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,fg_apl_s3_protect_bgnum_th_init);   
       // WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg,24,24,0x0);                 
       // WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg,8,15,0xff);
       // WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg,0,7,0xff);
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg, 0, 0, 0x1); //me1 black protect
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,0,0,0x1); //me1 black protect en
       	path3=0 ;
    }

  //  rtd_pr_memc_info("fgapl=%d %d %d %d %d %d%d %d\n" ,fg_apl_seg0_cnt,fg_apl_seg1_cnt,fg_apl_seg2_cnt,fg_apl_seg3_cnt,fg_apl_seg4_cnt,fg_apl_seg5_cnt,fg_apl_seg6_cnt,fg_apl_seg7_cnt ) ;
    if((s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_317_face_flag == 1)&&(!fwcontrol_1282))
    {
        path3=9  ;
        video317 =1 ;
        holdframe317 = 120 ; 
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0);
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,0);
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,0);

        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x16);
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0xc);
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,18,22,0xa);
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,23,27,0x2);
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,28,31,0x3);      

        
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x9);
        
    }
    else if(holdframe317>0)
    {
        path3=9  ;
        holdframe317--;
        pOutput->u1_me1_bg_pfv_corr_cond = 0; 
        pOutput->u1_me1_sad_bg_cond = 0 ;
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0);
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,0);
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,0);

        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x16);
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0xc);
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,18,22,0xa);
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,23,27,0x2);
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,28,31,0x3);

        
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x9);
    }
    else if(video317)
    {
        path3= 0x0  ;
        video317 =0 ;
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0);
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,1);
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,1);
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x10);
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0x4);
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,18,22,0xa);
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,23,27,0x2);
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,28,31,0x3);

        
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x0);

    }


    if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_72_face_flag == 1)
    {
        video72 =1 ;
        holdframe72 =50 ;
		if( s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ)
        {
            holdframe72 =  90 ;
        } 
        path3 = 0xa ; 

        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,4); //djntest
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,4); 
        WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,4); 
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,4);

        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0);

        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,0);
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,0);
    }
    else if(holdframe72>0)
    {
        holdframe72--;
        path3 = 0xa; 
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0);

        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,0);
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,0);

        
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,4); 
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,4); 
        WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,4); 
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,4);

        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0);

        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,0);
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,0);


    }
    else if(video72)
    {
        video72 =0 ;
        path3 = 0  ; 
    }


   if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_183_face_broken == 1)
   {
	   video183 =1 ;
	   holdframe183 = s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ? 150:	80 ;
	   WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0xa);  //bg_s1_num_yth2
	   WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0x3);  //bg_s1_num_yth1
	   WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x10);//bg_s2_num_yth2
	   WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0x6); //bg_s2_num_yth1	
	   WriteRegister(KME_ME1_BG0_ME1_BG_16_reg,24,28,0x19);  //BG_16_bg_s3_clr_prot_th	0x12
	   WriteRegister(KME_ME1_BG0_ME1_BG_16_reg,18,22,0x19);  //BG_16_bg_s1_clr_prot_th
	//	 WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,1); //sad_cov_en  
	//	 WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,1); //sad_ucov_en  
   
	   WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,8,14, 38);  //bg_s3_dubt_cov_num_yth2_init
	   WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,15,21, 14); //bg_s3_dubt_cov_num_yth1_init 
	   
	   WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,15,21, 10);//bg_s3_dubt_ucov_yth1_init
	   WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,8,14, 36);  //bg_s3_dubt_ucov_yth2_init
	   WriteRegister(KME_ME1_BG0_ME1_BG_10_reg,2,2,0x1); //BG_10_bg_s3_occl_match_bypass
	   WriteRegister(KME_ME1_BG1_ME_BG_S4_1_reg,21,21,0x1); //bg_s3_occl_rgn_match_en
	   WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0);
	   WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,240);
	   WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,65);
	   WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,135);
	   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30, 0x1); //bg_s3_ucov_data_en			   
	   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31, 0x0); //bg_s3_cov_data_en
   
	   WriteRegister(HARDWARE_HARDWARE_25_reg, 23,23 , 1);
	   path3  = 0x5 ;
   }
   else if(holdframe183>0)
   {
	   holdframe183--;
	   WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0xa);  //bg_s1_num_yth2
	   WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0x3);  //bg_s1_num_yth1
	   WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x10);//bg_s2_num_yth2
	   WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0x6); //bg_s2_num_yth1	
	   WriteRegister(KME_ME1_BG0_ME1_BG_16_reg,24,28,0x19);  //BG_16_bg_s3_clr_prot_th	0x12
	   WriteRegister(KME_ME1_BG0_ME1_BG_16_reg,18,22,0x19);  //BG_16_bg_s1_clr_prot_th
	//	 WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,1); //sad_cov_en  
	//	 WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,1); //sad_ucov_en  
   
	   WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,8,14, 38);  //bg_s3_dubt_cov_num_yth2_init
	   WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,15,21,14); //bg_s3_dubt_cov_num_yth1_init 
	   
	   WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,15,21, 10);//bg_s3_dubt_ucov_yth1_init
	   WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,8,14, 36);  //bg_s3_dubt_ucov_yth2_init
	   WriteRegister(KME_ME1_BG0_ME1_BG_10_reg,2,2,0x1); //BG_10_bg_s3_occl_match_bypass
	   WriteRegister(KME_ME1_BG1_ME_BG_S4_1_reg,21,21,0x1); //bg_s3_occl_rgn_match_en
	   WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0);
	   WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,240);
	   WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,65);
	   WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,135);
	   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30, 0x1); //bg_s3_ucov_data_en			   
	   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31, 0x0); //bg_s3_cov_data_en
   
	   WriteRegister(HARDWARE_HARDWARE_25_reg, 23,23 , 1);
	   
	   path3  = 0x5 ;
   }
   else if(video183)
   {
	   video183 =0 ;
	   WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x10);  //bg_s1_num_yth2
	   WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0x4);  //bg_s1_num_yth1
	   WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x14);//bg_s2_num_yth2
	   WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0xa); //bg_s2_num_yth1	 
	   WriteRegister(KME_ME1_BG0_ME1_BG_16_reg,24,28,0x14);  //BG_16_bg_s3_clr_prot_th	0x12
	   WriteRegister(KME_ME1_BG0_ME1_BG_16_reg,18,22,0x14);  //BG_16_bg_s1_clr_prot_th
   
   
	   WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,8,14,bg_s3_dubt_cov_num_yth2_init); 
	   WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,15,21,bg_s3_dubt_cov_num_yth1_init);
	   
	   WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,15,21,bg_s3_dubt_ucov_yth1_init);
	   WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,8,14,bg_s3_dubt_ucov_yth2_init);
	   WriteRegister(KME_ME1_BG0_ME1_BG_10_reg,2,2,0x0); //BG_10_bg_s3_occl_match_bypass
	   WriteRegister(KME_ME1_BG1_ME_BG_S4_1_reg,21,21,0x0); //bg_s3_occl_rgn_match_en
   
	   WriteRegister(HARDWARE_HARDWARE_25_reg, 23,23 , 0);
	   path3 =0 ;
   }	   


 //   ReadRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,&mvth); 
   if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_151_broken == 1)
   {
	   video151 =1 ;
	   holdframe151 = s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ ? 360: 200; 

	   WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x18);  //bg_s1_num_yth2
	   WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0xa);  //bg_s1_num_yth1
	   WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x1a);//bg_s2_num_yth2
	   WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0x1a); //bg_s2_num_yth1  
	   WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0);
	   WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0);
	   WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0);
	   WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0);

	   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,0);
	   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,0);
	   path3 = 0xa ; 
   }
   else if(holdframe151>0)
   { 
	   holdframe151--;
	   WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x18);  //bg_s1_num_yth2
	   WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0xa);  //bg_s1_num_yth1
	   WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x1a);//bg_s2_num_yth2
	   WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0x1a); //bg_s2_num_yth1  
	   WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0);
	   WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0);
	   WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0);
	   WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0);

	   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,0);
	   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,0);
        path3 = 0xa ; 
   }
   else if(video151)
   {
	   video151 =0 ;
	   WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x10);  //bg_s1_num_yth2
	   WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0x4);  //bg_s1_num_yth1
	   WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x14);//bg_s2_num_yth2
	   WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0xa); //bg_s2_num_yth1	 
	   path3 = 0x0 ; 
   }


    
   if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_200_face_flag == 1)
   {
	   video200 =1 ;
	   holdframe200 = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 150 : 80 ;
	   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30, 0x0); //bg_s3_ucov_data_dis				 
	   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31, 0x0); //bg_s3_cov_data_dis
	   path3 = 0xb ; 
   }
   else if(holdframe200>0)
   {
	   holdframe200--;
	   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30, 0x0); //bg_s3_ucov_data_dis				 
	   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31, 0x0); //bg_s3_cov_data_dis
	   path3 = 0xb ; 
   }
   else if(video200)
   {
	   video200 =0 ;
	   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30, 0x1); //bg_s3_ucov_data_en			   
	   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31, 0x1); //bg_s3_cov_data_en
	   path3 = 0xb ; 
   }

  if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_432_flag > 0)
  {
	  video432 =1 ;
	  holdframe432 = ( s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 360 : 180 ;
  
	  WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x18);  //bg_s1_num_yth2
	  WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0xa);  //bg_s1_num_yth1
	  WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x1a);//bg_s2_num_yth2
	  WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0x1a); //bg_s2_num_yth1	
	  WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0);
	  WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0);
	  WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0);
	  WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0);
  
	  WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,0);
	  WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,0);
	  path3= 0xc ;
  
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_1_reg,20,29,400);	//v center 
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_1_reg,10,19,655); // u center
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_1_reg,0,9,620);  //y center
  
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_2_reg,18,26,48);  //v range
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_2_reg,9,17,48);  //u range
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_2_reg,0,8,200);  //y range 	 
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_1_reg,31,31,0x1);	//range1_en
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_7_reg,10,15,0x4);//colorprotect_cnt_th1  set to max value	 for hw bug 
	  WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,29,29,0x0); //colorprotect_set1_fg
  
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_5_reg,31,31,0x1); //range3_en
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_5_reg,20,29,490);	//v center 
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_5_reg,10,19,475);//u center 
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_5_reg,0,9,250); //y center 
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_6_reg,18,26,40); //v range 
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_6_reg,9,17,40); //u range
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_6_reg,0,8,100);   // y range
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_7_reg,22,27,4);//colorprotect_cnt_th3	set to max value   for hw bug 
	  WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,16,25,150); //bg_s4_color_protect_sad_th
	  WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,15,15,0x1);//bg_s4_color_protect_set_en
  
  }
  else if(holdframe432>0)
  {
	  holdframe432--;
	  WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x18);  //bg_s1_num_yth2
	  WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0xa);  //bg_s1_num_yth1
	  WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x1a);//bg_s2_num_yth2
	  WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0x1a); //bg_s2_num_yth1	
	  WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0);
	  WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0);
	  WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0);
	  WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0);
  
	  WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,0);
	  WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,0);
	  path3 = 0xc ; 
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_1_reg,20,29,400);	//v center 
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_1_reg,10,19,655); // u center
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_1_reg,0,9,620);  //y center
  
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_2_reg,18,26,48);  //v range
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_2_reg,9,17,48);  //u range
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_2_reg,0,8,200);  //y range 	 
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_1_reg,31,31,0x1);	//range1_en
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_7_reg,10,15,0x4);//colorprotect_cnt_th1  set to max value	 for hw bug 
	  WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,29,29,0x0); //colorprotect_set1_fg
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_5_reg,31,31,0x1); //range3_en
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_5_reg,20,29,490);	//v center 
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_5_reg,10,19,475);//u center 
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_5_reg,0,9,250); //y center 
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_6_reg,18,26,40); //v range 
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_6_reg,9,17,40); //u range
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_6_reg,0,8,100);   // y range
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_7_reg,22,27,4);//colorprotect_cnt_th3	set to max value   for hw bug 
	  WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,16,25,150); //bg_s4_color_protect_sad_th
	  WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,15,15,0x1);//bg_s4_color_protect_set_en
  
  }
  else if(video432)
  {
	  video432 =0 ;
	  WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x10);  //bg_s1_num_yth2
	  WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0x4);  //bg_s1_num_yth1
	  WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x14);//bg_s2_num_yth2
	  WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0xa); //bg_s2_num_yth1	
	  path3 = 0x0 ; 
  
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_1_reg,20,29,set2_vcenter_init);  //v center 
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_1_reg,10,19,set2_ucenter_init); // u center
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_1_reg,0,9,set2_ycenter_init);	//y center
  
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_2_reg,18,26,set2_vrange_init);  //v range
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_2_reg,9,17,set2_urange_init);	//u range
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_2_reg,0,8,set2_yrange_init);  //y range	  
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_1_reg,31,31,0x0);	//range1_en
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_7_reg,10,15,0x3f);//colorprotect_cnt_th1  set to max value   for hw bug 
	  WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,29,29,0x1); //colorprotect_set1_fg
  
  
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_5_reg,31,31,0x0); //range3_en
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_5_reg,20,29,0x0);	//v center 
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_5_reg,10,19,0x0);//u center 
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_5_reg,0,9,0x0); //y center 
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_6_reg,18,26,0x0); //v range 
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_6_reg,9,17,0x0); //u range
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_6_reg,0,8,0x0);   // y range
	  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_7_reg,22,27,0x3f);//colorprotect_cnt_th3  set to max value   for hw bug 
	  WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,16,25,0x12c); //bg_s4_color_protect_sad_th
	  WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,15,15,0x0);//bg_s4_color_protect_set_en
  
  }

 if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_195_face_broken == 1&&(!video192))
   {
	   video195 =1 ;
	   holdframe195 =  (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 220 : 110 ;

	   WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x18);  //bg_s1_num_yth2
	   WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0x10);	//bg_s1_num_yth1
	   WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x1a);//bg_s2_num_yth2
	   WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0x1a); //bg_s2_num_yth1  
	   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,0);
	   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,0);
	   WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,25);	//fg_apl_s2_protect_bgnum_th_init
	   path3 = 0xc ; 
   }
   else if(holdframe195>0)
   {
	   holdframe195--;

	   WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x18);  //bg_s1_num_yth2
	   WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0x10);	//bg_s1_num_yth1
	   WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x1a);//bg_s2_num_yth2
	   WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0x1a); //bg_s2_num_yth1  
	   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,0);
	   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,0);
		WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,25);  //fg_apl_s2_protect_bgnum_th_init
		path3 = 0xc ; 
   }
   else if(video195)
   {

	   video195 = 0 ;

	   WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x10);  //bg_s1_num_yth2
	   WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0x4);  //bg_s1_num_yth1
	   WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x14);//bg_s2_num_yth2
	   WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0xa); //bg_s2_num_yth1	
	   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,0);
	   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,0);
	   WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,fg_apl_s2_protect_bgnum_th_init);
	   WriteRegister(KME_ME1_BG0_ME1_BG_4_reg,24,24,fg_mask_pfv_blend_dis);   
	   WriteRegister(KME_ME1_BG0_ME1_BG_4_reg,16,16,0);    //bg_mask_pfv_blend_dis
	   path3 = 0x0 ; 
   }

    if(fwcontrol_214 || (s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_214 == 1))
    {
        holdframe214 = 5 ;
        video214 =2  ;
        
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,0x20); 
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,0x20); 
        WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,0x20); 
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,0x25); 
        path3 =0xc;
        WriteRegister(KME_ME1_BG0_ME1_BG_4_reg,24,24,fg_mask_pfv_blend_dis); 
        WriteRegister(KME_ME1_BG0_ME1_BG_4_reg,16,16,0);    //bg_mask_pfv_blend_dis
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0x0);   //full range 
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0xef);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0x86);
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,31,31,0x0); //apl_s2_protect_extend_en
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);   // gmv candidtae choose gmv  not  rmv 
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0);


    }
    else if(holdframe214>0)
    {
        holdframe214--;
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,0x20); 
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,0x20); 
        WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,0x20); 
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,0x25); 
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0x0);   //full range 
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0xef);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0);
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0x86);
		path3 =0xc;
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,31,31,0x0); //apl_s2_protect_extend_en
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);   // gmv candidtae choose gmv  not  rmv 
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0);
    }
    else if(video214)
    {
        video214 = 0 ;
        path3 =0;
        WriteRegister(KME_ME1_BG0_ME1_BG_4_reg,24,24,fg_mask_pfv_blend_en_init); 
        WriteRegister(KME_ME1_BG0_ME1_BG_4_reg,16,16,1);    //bg_mask_pfv_blend_dis
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,31,31,0x1); //apl_s2_protect_extend_en
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 0);   // gmv candidtae choose gmv  not  rmv 
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 1);

    }

    if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_317_face_flag3&&(!video192))
    {
        holdframe317_3 = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 200 : 100 ;
        video317_3 =1 ;
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0);
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,0);
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,0);
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x18);  //bg_s1_num_yth2
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0xa);  //bg_s1_num_yth1
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x1a);//bg_s2_num_yth2
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0x1a); //bg_s2_num_yth1  
        path3 =2;
    }
    else if(holdframe317_3)
    {
        holdframe317_3--;
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0);

        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,0);
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,0);
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x18);  //bg_s1_num_yth2
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0xa);  //bg_s1_num_yth1
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x1a);//bg_s2_num_yth2
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0x1a); //bg_s2_num_yth1  
         path3 =2;
    }
    else if(video317_3)
    {
        video317_3 = 0 ;
         path3 =0 ;
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x10);  //bg_s1_num_yth2
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0x4);  //bg_s1_num_yth1
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x14);//bg_s2_num_yth2
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0xa); //bg_s2_num_yth1   
    }

    if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_317_face_flag2&&(!video306_2)&&(!video192))
    {
        video317_2 =1 ;
        holdframe317_2 = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 500 : 250 ;
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0);

        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,0);
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,0);
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x18);  //bg_s1_num_yth2
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0xa);  //bg_s1_num_yth1
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x1a);//bg_s2_num_yth2
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0x1a); //bg_s2_num_yth1  
         path3 =9;
    }
    else if(holdframe317_2)
    {
        holdframe317_2--;
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0);

        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,0);
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,0);
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x18);  //bg_s1_num_yth2
        WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0xa);  //bg_s1_num_yth1
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x1a);//bg_s2_num_yth2
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0x1a); //bg_s2_num_yth1  
         path3 =9 ;
    }
	else if(video317_2)
	{
		video317_2 = 0 ;
		 path3 =0;
		WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x10);	//bg_s1_num_yth2
		WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0x4);	//bg_s1_num_yth1
		WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x14);//bg_s2_num_yth2
		WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0xa); //bg_s2_num_yth1  
	}
	
	
	
	//---------------------------	96	-----------------------------//
	//rtd_pr_memc_info("flag:%d\n",s_pContext->_output_frc_sceneAnalysis.u8_dh_gmv_valchange_96);
	//rtd_pr_memc_info("gmv:%d %d rmv:%d %d\n",u11_gmv_mvx,u10_gmv_mvy,rmvx[15],rmvy[15]);
	if(s_pContext->_output_frc_sceneAnalysis.u8_dh_gmv_valchange_96 == 1) 
	{
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31,1);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,rmvx[15]);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);
		holdframe_96 = 0;
		video96 = 1;
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, rmvx[15]);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, rmvy[15]);
		
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31, 0);	//gmv_bg_en
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);//gmv_sel_en
		path3 = 0xd ; 
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0x0);	//full range 
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0xef);   //range max  x 
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0x0);
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0x86); //range max y
		WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,1);
		WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,1);  
		WriteRegister(KME_ME1_BG1_ME_BG_S4_1_reg,16,20,0x0); //BG_S4_1_bg_s3_occl_rgn25_th
		WriteRegister(KME_ME1_BG1_ME_BG_S4_1_reg,9,15,0x0);  //BG_S4_1_bg_s3_occl_rgn65_th
		WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,14,14,0x1);  //bg_occl_match_ver_en
		WriteRegister(KME_ME1_BG0_ME1_BG_10_reg,2,2,0x1); //BG_10_bg_s3_occl_match_bypass  
	}
	else if(holdframe_96>0)
	{
		holdframe_96--;
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31,1);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,rmvx[15]);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);
		
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, rmvx[15]);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, rmvy[15]);

		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31, 0);	//gmv_bg_en
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);//gmv_sel_en
		path3 = 0xd ;
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0x0);	//full range 
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0xef);   //range max  x 
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0x0);
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0x86); //range max y
		WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,1);
		WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,1);	  
		WriteRegister(KME_ME1_BG1_ME_BG_S4_1_reg,16,20,0x0); //BG_S4_1_bg_s3_occl_rgn25_th
		WriteRegister(KME_ME1_BG1_ME_BG_S4_1_reg,9,15,0x0);  //BG_S4_1_bg_s3_occl_rgn65_th
		WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,14,14,0x1);  //bg_occl_match_ver_en
		WriteRegister(KME_ME1_BG0_ME1_BG_10_reg,2,2,0x1); //BG_10_bg_s3_occl_match_bypass  
	}
	else if(video96)
	{
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31,1);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,u11_gmv_mvx);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);
		video96 = 0;
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, u11_gmv_mvx);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, u10_gmv_mvy);

		
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31, 1);	//gmv_bg_en
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 0);//gmv_sel_en
		WriteRegister(KME_ME1_BG1_ME_BG_S4_1_reg,16,20,0x5); //BG_S4_1_bg_s3_occl_rgn25_th
		WriteRegister(KME_ME1_BG1_ME_BG_S4_1_reg,9,15,0xa);  //BG_S4_1_bg_s3_occl_rgn65_th
		WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,14,14,0x0);  //bg_occl_match_ver_en
		WriteRegister(KME_ME1_BG0_ME1_BG_10_reg,2,2,0x0); //BG_10_bg_s3_occl_match_bypass


	   WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0x0);   //full range 
	   WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0);   //range max  x 
	   WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0x0);
	   WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0); //range max y
		//path3 = 0x0 ;
	}

	if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_96_2_flag == 1) 
	{
		holdframe_96_2 = 0;
		video96_2 = 1;
		path3 = 0xd ;
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0x0);	//full range 
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0xef);   //range max  x 
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0x0);
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0x86); //range max y
		// WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,1);
		// WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,1);  
		WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x14);//bg_s2_num_yth2
		WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0xa); //bg_s2_num_yth1  
	}
	else if(holdframe_96_2>0)
	{
		holdframe_96_2--;

		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0x0);	//full range 
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0xef);   //range max  x 
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0x0);
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0x86); //range max y
		// WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,1);
		// WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,1); 
		WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x14);//bg_s2_num_yth2
		WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0xa); //bg_s2_num_yth1  
	   path3 = 0xd ;
	}
	else if(video96_2)
	{

		path3 = 0x0 ;
		WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x14);//bg_s2_num_yth2
		WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0xa); //bg_s2_num_yth1  
	}


	//---------------------------	421  -----------------------------//
	if(s_pContext->_output_frc_sceneAnalysis.u8_dh_gmv_valchange_421 == 1&&(!s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_128_flag))
	{
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31,1);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,rmvx[8]);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);
		holdframe_421 = 0;
		video421 = 1;
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, rmvx[8]);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, rmvy[8]);
		
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31, 0);	//gmv_bg_en
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);//gmv_sel_en

		WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,0x3f);	//th0
		WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,0x3f);
		WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,0x3f);
		WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,0x44);
		path3 = 0xd ;

		
	}
	else if(holdframe_421>0)
	{
		holdframe_421--;
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31,1);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,rmvx[8]);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);
		
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, rmvx[8]);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, rmvy[8]);

		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31, 0);	//gmv_bg_en
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);//gmv_sel_en

		WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,0x3f);	//th0
		WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,0x3f);
		WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,0x3f);
		WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,0x44);
		path3 = 0xd ;		
        
	}
	else if(video421)
	{
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31,1);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,u11_gmv_mvx);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);
		video421 = 0;
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, u11_gmv_mvx);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, u10_gmv_mvy);

		
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31, 1);	//gmv_bg_en
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 0);//gmv_sel_en

		WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,0x14);	//th0
		WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,0x14);
		WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,0x14);
		WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,0x19);
		path3 = 0x0 ;

		
	}


		
	  video315 = 0;
	  holdframe315 = 0;


/*	  //+++++++++++++++++315 181 horse vertical moving video++++++++++++++//
	if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_ver_case1==1)
	{
		holdframe315=  (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 250 : 120 ; 
		video315=1;
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0x0);	//full range 
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0xef);
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0x0);
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0x86);
		WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x4);  //bg_s1_num_yth2
		WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0x1);	//bg_s1_num_yth1
		WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x8);//bg_s2_num_yth2
		WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0x2); //bg_s2_num_yth1   
		
		WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,8,14,22);  //dubt_cov_num_yth2
		WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,15,21,4);  //dubt_cov_num_yth1
		//WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,4,7,bg_s3_dubt_cov_shift_init);
	   // WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,0,3,bg_s3_dubt_cov_slope_init);
		
		WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,15,21,4);  //dubt_ucov_yth1
		WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,8,14,22); //s3_dubt_ucov_yth2
	   // WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,4,7,bg_s3_dubt_ucov_shift_init);
	  //  WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,0,3,bg_s3_dubt_ucov_slope_init);
		WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30, 0x1); //bg_s3_ucov_data_en				
		WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31, 0x1); //bg_s3_cov_data_en		
		WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,26,26,bg_color_protect_en_off);  // init =1 
		path3 = 0xd ;

	}
	else if(holdframe315>0)
	{
		path3 = 0xd ;
		holdframe315 -- ; 
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0x0);	//full range 
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0xef);
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0x0);
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0x86);
		WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x4);  //bg_s1_num_yth2
		WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0x1);	//bg_s1_num_yth1
		WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x8);//bg_s2_num_yth2
		WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0x2); //bg_s2_num_yth1   

				
		WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,8,14,22);  //dubt_cov_num_yth2
		WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,15,21,4);  //dubt_cov_num_yth1
		//WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,4,7,bg_s3_dubt_cov_shift_init);
	   // WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,0,3,bg_s3_dubt_cov_slope_init);
		
		WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,15,21,4);  //dubt_ucov_yth1
		WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,8,14,22); //s3_dubt_ucov_yth2
	   // WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,4,7,bg_s3_dubt_ucov_shift_init);
	  //  WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,0,3,bg_s3_dubt_ucov_slope_init);
		WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30, 0x1); //bg_s3_ucov_data_en				
		WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31, 0x1); //bg_s3_cov_data_en		
		WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,26,26,bg_color_protect_en_off);  // init =1 
	}
	else if(video315)
	{
		video315 = 0 ;
		path3 = 0 ;
		WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x10);	//bg_s1_num_yth2
		WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0x4);	//bg_s1_num_yth1
		WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x14);//bg_s2_num_yth2
		WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0xa); //bg_s2_num_yth1   
		WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,8,14,bg_s3_dubt_cov_num_yth2_init);
		WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,15,21,bg_s3_dubt_cov_num_yth1_init);
	 //   WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,4,7,bg_s3_dubt_cov_shift_init);
	  //  WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,0,3,bg_s3_dubt_cov_slope_init);
		
		WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,15,21,bg_s3_dubt_ucov_yth1_init);
		WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,8,14,bg_s3_dubt_ucov_yth2_init);
	 //   WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,4,7,bg_s3_dubt_ucov_shift_init);
	  //  WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,0,3,bg_s3_dubt_ucov_slope_init);
		WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,26,26,bg_color_protect_en_init);  // init =1 
	} 

*/

 //------------------------- 219 -------------------------------//
	
	if(s_pContext->_output_frc_sceneAnalysis.u8_dh_gmv_valchange_219 == 1) 
	{
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,rmvx[7]);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);
		video219 = 1;
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, rmvx[7]);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, rmvy[7]);
		
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31, 0);  //gmv_bg_en
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);//gmv_sel_en
        path3 =6 ;
	}
	else if(video219)
	{
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31,1);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,u11_gmv_mvx);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);
		video219 = 0;
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, u11_gmv_mvx);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, u10_gmv_mvy);

		
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31, 1);  //gmv_bg_en
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 0);//gmv_sel_en
        path3 =0 ;
	}


    //------------------------  227  -----------------------------------//
    if(s_pContext->_output_frc_sceneAnalysis.u8_dh_gmv_valchange_227_1 == 1) 
    {
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,rmvx[13]);
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);
        video2271 = 1;
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, rmvx[13]);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, rmvy[13]);
        
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31, 0);  //gmv_bg_en
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);//gmv_sel_en
        path3 =7 ;
    }
    else if(video2271)
    {
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31,1);
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,u11_gmv_mvx);
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);
        video2271 = 0;
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, u11_gmv_mvx);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, u10_gmv_mvy);

        
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31, 1);  //gmv_bg_en
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 0);//gmv_sel_en
        path3 =0 ;
    }

    if(s_pContext->_output_frc_sceneAnalysis.u8_dh_gmv_valchange_227_2 == 1) 
    {
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,rmvx[11]);
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);
        video2272 = 1;
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, rmvx[11]);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, rmvy[11]);
        
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31, 0);  //gmv_bg_en
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);//gmv_sel_en

        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,0x60);  //th0
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,0x60);
        WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,0x60);
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,0x65);
        path3 =7 ;
    }
    else if(video2272)
    {
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31,1);
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,u11_gmv_mvx);
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);
        video2272 = 0;
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, u11_gmv_mvx);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, u10_gmv_mvy);

        
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31, 1);  //gmv_bg_en
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 0);//gmv_sel_en

        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,0x14);  //th0
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,0x14);
        WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,0x14);
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,0x19);
        path3 =0 ;
    }


    if(s_pContext->_output_frc_sceneAnalysis.u8_dh_gmv_valchange_227_3 == 1) 
    {
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,rmvx[20]);
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);
        video2273 = 1;
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, rmvx[20]);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, rmvy[20]);

        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31, 0);  //gmv_bg_en
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);//gmv_sel_en

        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,0x25);  //th0
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,0x25);
        WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,0x25);
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,0x2a);

        path3 =7 ;
        
    }
    else if(video2273)
    {
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31,1);
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,u11_gmv_mvx);
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);
        video2273 = 0;
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, u11_gmv_mvx);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, u10_gmv_mvy);

        
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31, 1);  //gmv_bg_en
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 0);//gmv_sel_en

        
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,0x14);  //th0
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,0x14);
        WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,0x14);
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,0x19);

        path3 =0 ; 

        
    }


    if(s_pContext->_output_frc_sceneAnalysis.u8_dh_gmv_valchange_227_4 == 1) 
    {
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,rmvx[8]);
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);
        video2274 = 1;
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, rmvx[8]);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, rmvy[8]);
        
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31, 0);  //gmv_bg_en
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);//gmv_sel_en
        
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,0xb0);  //th0
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,0xb0);
        WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,0xb0);
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,0xb5);
        path3 =7 ;

    }
    else if(video2274)
    {
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31,1);
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,u11_gmv_mvx);
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);
        video2274 = 0;
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, u11_gmv_mvx);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, u10_gmv_mvy);

        
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31, 1);  //gmv_bg_en
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 0);//gmv_sel_en

        
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,0x14);  //th0
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,0x14);
        WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,0x14);
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,0x19);
        path3 = 0;
    }



    if(s_pContext->_output_frc_sceneAnalysis.u8_dh_gmv_valchange_227_5 == 1) 
    {
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,rmvx[11]);
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);
        video2275 = 1;
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, rmvx[11]);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, rmvy[11]);
        
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31, 0);  //gmv_bg_en
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);//gmv_sel_en
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0x0); //pfv corr max x 
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0x0);//pfv corr max y

        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,0x20);  //th0
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,0x20);
        WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,0x20);
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,0x25);
        path3 =7 ;
    }
    else if(video2275)
    {
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31,1);
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,u11_gmv_mvx);
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);
        video2275 = 0;
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, u11_gmv_mvx);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, u10_gmv_mvy);
        
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31, 1);  //gmv_bg_en
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 0);//gmv_sel_en
        path3 =0 ;
    }


    if(s_pContext->_output_frc_sceneAnalysis.u8_dh_gmv_valchange_227_2 ||
        s_pContext->_output_frc_sceneAnalysis.u8_dh_gmv_valchange_227_3 || s_pContext->_output_frc_sceneAnalysis.u8_dh_gmv_valchange_227_4 ||
        s_pContext->_output_frc_sceneAnalysis.u8_dh_gmv_valchange_227_5)
    {
        WriteRegister(HARDWARE_HARDWARE_25_reg, 22,22 , 1);
    }
    else
    {
        WriteRegister(HARDWARE_HARDWARE_25_reg, 22,22 , 0);
    }

    //+++++++++++++++++431 halo+++++++++++++++++++++++// 
    if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_431_halo_flag==1)
    {
        holdframe431=  (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 160 : 80; 
        video431=1;
        path3 = 0xd ;

        WriteRegister(KME_ME1_BG0_ME1_BG_10_reg,1,1,1); //bg_s3_mode0_cond_en 
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,0,0,0x0);   //fg_apl_s3_protect_en
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,0,0,0x0);  //s2_protect_en
        WriteRegister(KME_ME1_BG0_ME1_BG_8_reg,15,21,8); //reg_me1_bg_s3_mode0_ucov_yth1
        WriteRegister(KME_ME1_BG0_ME1_BG_8_reg,8,14, 30); //reg_me1_bg_s3_mode0_ucov_yth2
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0xa);//bg_s2_num_yth2
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0x2); //bg_s2_num_yth1 
        WriteRegister(KME_ME1_BG0_ME1_BG_16_reg,9,9,0x1);  //s2_ucov_cond_en
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,134);   //corr  range 
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,240);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,57);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,135);
    }
    else if(holdframe431)
    {
        holdframe431--;
        path3 = 0xd ;

        WriteRegister(KME_ME1_BG0_ME1_BG_10_reg,1,1,1); //bg_s3_mode0_cond_en 
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,0,0,0x0);   //fg_apl_s3_protect_en
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,0,0,0x0);  //s2_protect_en
        WriteRegister(KME_ME1_BG0_ME1_BG_8_reg,15,21,8); //reg_me1_bg_s3_mode0_ucov_yth1
        WriteRegister(KME_ME1_BG0_ME1_BG_8_reg,8,14, 30); //reg_me1_bg_s3_mode0_ucov_yth2
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0xa);//bg_s2_num_yth2
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0x2); //bg_s2_num_yth1 
        WriteRegister(KME_ME1_BG0_ME1_BG_16_reg,9,9,0x1);  //s2_ucov_cond_en     
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,134);   //corr  range 
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,240);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,57);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,135);
    }
    else if(video431)
    {
        video431=0 ;
        path3 = 0 ;

        WriteRegister(KME_ME1_BG0_ME1_BG_10_reg,1,1,0); //bg_s3_mode0_cond_en 
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,0,0,0x1);   //fg_apl_s3_protect_en
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,0,0,0x1);  //s2_protect_en
        WriteRegister(KME_ME1_BG0_ME1_BG_8_reg,15,21,0x2); //reg_me1_bg_s3_mode0_ucov_yth1
        WriteRegister(KME_ME1_BG0_ME1_BG_8_reg,8,14,0x14); //reg_me1_bg_s3_mode0_ucov_yth2

        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x14);//bg_s2_num_yth2
        WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0xa); //bg_s2_num_yth1    
        WriteRegister(KME_ME1_BG0_ME1_BG_16_reg,9,9,0x0);  //s2_ucov_cond_en
    }
   //++++++++++++++++49 boy halo+++++++++++++++++++++++++//
    if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_49_halo_flag==1)
    {
        holdframe49=  (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 160 : 80; 
        video49=1;
        path3 =0xe ;

        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,0);	    	    
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,0); 	     
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,0);

        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,0x20);  //th0
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,0x20);
        WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,0x20);
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,0x25);

        WriteRegister(HARDWARE_HARDWARE_25_reg, 24,24 , 1);
            
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_3_reg,20,29,580);  //v center 
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_3_reg,10,19,460);  //u center
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_3_reg,0,9, 400);//y center 
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_4_reg,18,26,140);  //v range 
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_4_reg,9,17,140);  //u range 
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_4_reg,0,8,350);  // y range
    }
    else if(holdframe49>0)
    {
        holdframe49--;
        path3 =0xe ;
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,0);	    	    
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,0); 	     
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,0);

        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,0x20);  //th0
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,0x20);
        WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,0x20);
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,0x25);
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_3_reg,20,29,580);  //v center 
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_3_reg,10,19,460);  //u center
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_3_reg,0,9, 400);//y center 
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_4_reg,18,26,140);  //v range 
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_4_reg,9,17,140);  //u range 
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_4_reg,0,8,350);  // y range
        
        WriteRegister(HARDWARE_HARDWARE_25_reg, 24,24 , 1);
              

    }
    else if(video49)
    {
        video49 =0 ;
        path3 =0x0 ;

        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,fg_apl_s2_protect_th0_init);	    	    
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,fg_apl_s2_protect_th1_init); 	     
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,fg_apl_s2_protect_bgnum_th_init);
        WriteRegister(HARDWARE_HARDWARE_25_reg, 24,24 , 0);

        //  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_3_reg,31,31,0x1);  //range2_en
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_3_reg,20,29,set1_vcenter_init);  //v center 
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_3_reg,10,19,set1_ucenter_init);  //u center
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_3_reg,0,9, set1_ycenter_init);//y center 
        //  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_4_reg,27,27,0x0);
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_4_reg,18,26,set1_vrange_init);  //v range 
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_4_reg,9,17,set1_urange_init);  //u range 
        WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_4_reg,0,8,set1_yrange_init);  // y range

        
    }

    
    //++++++++++++++++163 dog hortical move halo+++++++++++++++++++++++++//
    if(s_pContext->_output_frc_sceneAnalysis.u8_dh_gmv_valchange_163==1)
    {
        holdframe163=  (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 300 : 150; 
        video163=1;
        path3 =0xe ;
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,rmvx[11]);
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, rmvx[11]);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, rmvy[11]);
        
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31, 0);  //gmv_bg_en
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);//gmv_sel_en

        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,0x10);  //th0
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,0x10);
        WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,0x10);
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,0x12);
        //corr pfv full range 
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0xef);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0x86);  

        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,0);	  // fg_apl_s2_protect_th0   	    
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,80); //fg_apl_s2_protect_th1 	     
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,65); //fg_apl_s2_protect_bgnum_th 

        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,0);   //fg_apl_s3_protect_th0 
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,80);   //fg_apl_s3_protect_th1 
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,65); //fg_apl_s3_protect_bgnum_th  
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg, 0, 0, 0x1); //me1 black protect

        WriteRegister(HARDWARE_HARDWARE_25_reg, 25,25 , 1);
    }
    else if(holdframe163>0)
    {
        holdframe163--;
        path3 =0xe ;
		
	  WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,rmvx[11]);
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, rmvx[11]);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, rmvy[11]);
        
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31, 0);  //gmv_bg_en
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);//gmv_sel_en
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,0x10);  //th0
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,0x10);
        WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,0x10);
        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,0x12);
                //corr pfv full range 
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0xef);
        WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0x86);   


        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,0);	  // fg_apl_s2_protect_th0   	    
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,80); //fg_apl_s2_protect_th1 	     
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,65); //fg_apl_s2_protect_bgnum_th 

        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,0);   //fg_apl_s3_protect_th0 
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,80);   //fg_apl_s3_protect_th1 
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,65); //fg_apl_s3_protect_bgnum_th  
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg, 0, 0, 0x1); //me1 black protect
       
        WriteRegister(HARDWARE_HARDWARE_25_reg, 25,25 , 1);
    }
    else if(video163)
    {
        video163 =0 ;
        path3 =0x0 ;
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31,1);
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,u11_gmv_mvx);
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, u11_gmv_mvx);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, u10_gmv_mvy);

        
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31, 1);  //gmv_bg_en
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 0);//gmv_sel_en

        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,fg_apl_s2_protect_th0_init);	    	    
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,fg_apl_s2_protect_th1_init); 	     
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,fg_apl_s2_protect_bgnum_th_init);

        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,fg_apl_s3_protect_th0_init);   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,fg_apl_s3_protect_th1_init);   
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,fg_apl_s3_protect_bgnum_th_init);  

        WriteRegister(HARDWARE_HARDWARE_25_reg, 25,25 , 0);
    }


    //------------------------- 432 final gmv ------------------------------//

    if(s_pContext->_output_frc_sceneAnalysis.u8_dh_gmv_valchange_432final == 1) 
    {
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,rmvx[6]);
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);

        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, rmvx[6]);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, rmvy[6]);
        
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31, 0);  //gmv_bg_en
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);//gmv_sel_en

        video432final = 1;
        
        if( s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ)
        {
            holdframe432final = 240;
        }
        else
        {
            holdframe432final = 120;
        }
        path3 =1 ;
    }
    else if(holdframe432final>0)
    {
    
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,rmvx[6]);
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);

        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, rmvx[6]);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, rmvy[6]);
        
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31, 0);  //gmv_bg_en
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);//gmv_sel_en
     
        video432final = 1;
        
        holdframe432final--;
        path3 =1 ;
    }
    else if(video432final)
    {
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,u11_gmv_mvx);
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);
        
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, u11_gmv_mvx);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, u10_gmv_mvy);
        
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31, 1);  //gmv_bg_en
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 0);//gmv_sel_en
        video432final = 0;
        path3 =0 ;
    }      
   

   
   if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_343_bypass == 1) 
   {
	   video343= 1;
	   holdframe343  = 120 ;
	   path4 =1 ;
   }
   else if(holdframe343>0)
   {
	   holdframe343--;
	   path4 =1 ;
   }
   else if(video343)
   {
	   video343 =0 ;
	   path4 =0 ;
   }
   
   if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_182_bypass == 1) 
   {
	   video182= 1;
	   holdframe182  = 120 ;
	   path4 =2 ;
   }
   else if(holdframe182>0)
   {
	   holdframe182--;
	   path4 =2 ;
   }
   else if(video182)
   {
	   video182 =0 ;
	   path4 =0 ;
   }
   
   if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_393_scgarbage == 1) 
   {
	   video393= 1;
	   holdframe393  = 10 ;
	   path4 = 3;
   }
   else if(holdframe393>0)
   {
	   holdframe393--;
	   path4 = 3;
   }
   else if(video393)
   {
	   video393 =0 ;
	   path4 =0 ;
   }
   
   if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_410_scgarbage == 1) 
   {
	   video410= 1;
	   holdframe410  = 10 ;
	   path4 = 4;
   }
   else if(holdframe410>0)
   {
	   holdframe410--;
	   path4 = 4;
   }
   else if(video410)
   {
	   video410 =0 ;
	   path4 =0 ;
   }
   
   if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_416_bypass == 1) 
   {
	   video416= 1;
	   holdframe416  = 120 ;
	   path4 = 5;
   }
   else if(holdframe416>0)
   {
	   holdframe416--;
	   path4 = 5;
   }
   else if(video416)
   {
	   video416 =0 ;
	   path4 =0 ;
   }
   
   if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_412_bypass == 1) 
   {
	   video412= 1;
	   holdframe412  = 120 ;
	   path4 = 6;
   }
   else if(holdframe412>0)
   {
	   holdframe412--;
	   path4 = 6;
   }
   else if(video412)
   {
	   video412 =0 ;
	   path4 =0 ;
   }
   
   if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_214 == 1) 
   {
	   video2144= 1;
	   holdframe2144  = 120 ;
	   path4 = 7;
   }
   else if(holdframe2144>0)
   {
	   holdframe2144--;
	   path4 = 7;
   }
   else if(video2144)
   {
	   video2144 =0 ;
	   path4 =0 ;
   }
   
   
   if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_419_flag == 1) 
   {
	   video419= 1;
	   holdframe419  = 120 ;
	   path4 = 8;
   }
   else if(holdframe419>0)
   {
	   holdframe419--;
	   path4 = 8;
   }
   else if(video419)
   {
	   video419 =0 ;
	   path4 =0 ;
   }
   
   if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_horn_skin_off == 1)  //192
   {
	   video1922= 1;
	   holdframe1922  = 120 ;
	   path4 = 0x9;
   }
   else if(holdframe1922>0)
   {
	   holdframe1922--;
	   path4 = 0x9;
   }
   else if(video1922)
   {
	   video1922 =0 ;
	   path4 =0 ;
   }
   
   if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_151_broken == 1)  //151
   {
	   video1511= 1;
	   holdframe1511  = 120 ;
	   path4 = 0xa;
   }
   else if(holdframe1511>0)
   {
	   holdframe1511--;
	   path4 = 0xa;
   }
   else if(video1511)
   {
	   video1511 =0 ;
	   path4 =0 ;
   }
   
   if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_128_flag == 1)  //128
   {
	   video1288= 1;
	   holdframe1288  = 120 ;
	   path4 = 0xb;
   }
   else if(holdframe1288>0)
   {
	   holdframe1288--;
	   path4 = 0xb;
   }
   else if(video1288)
   {
	   video1288 =0 ;
	   path4 =0 ;
   }
   //rtd_pr_memc_info("%d %d %d %d %d %d %d %d %d %d %d\n",s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_343_bypass,s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_182_bypass,
   //s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_393_scgarbage,s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_410_scgarbage,s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_416_bypass,
   //s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_412_bypass,s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_214,s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_419_flag,
   //s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_horn_skin_off,s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_151_broken,s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_128_flag);
   
   
   //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, path4); //path segment 
   
	   //---------------------------pq issue pattern ----------------------------//
	   if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_PQ_Issue == 1)
	   {		
   
		   FrameHold_PQ_Issue = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 60: 30  ;  
		   FrameHoldflag_PQ_Issue=1;
		   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,bg_s3_cov_data_en_init);  //bg_s3_cov_data_en	 
		   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,bg_s3_ucov_data_en_init);	//bg_s3_ucov_data_en   
		   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,0);   // fg_apl_s3_protect_th0_init 
		   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,0);    //fg_apl_s3_protect_th1_init
		   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,0);	  // fg_apl_s3_protect_bgnum_th_init   
		   WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,0,0,0x0); //s2_protect_en
		   WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,10); 
		   WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,10); 
		   WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,10); 
		   WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,12); 
				 //path4 = 0xf;
					 // 	WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);   // gmv candidtae choose gmv  not	rmv 
		   WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 1);
		   WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, rmvx[9]+10);
		   WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, rmvy[9]);
   
   
	   }
	   else if(FrameHold_PQ_Issue>0)
	   {
		   FrameHold_PQ_Issue -- ;
		   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,bg_s3_cov_data_en_init);  //bg_s3_cov_data_en	 
		   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,bg_s3_ucov_data_en_init);	//bg_s3_ucov_data_en   
		   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,0);   // fg_apl_s3_protect_th0_init 
		   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,0);    //fg_apl_s3_protect_th1_init
		   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,0);	  // fg_apl_s3_protect_bgnum_th_init   
		   WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,0,0,0x0); //s2_protect_en
		   WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,10); 
		   WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,10); 
		   WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,10); 
		   WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,12); 
						  //path4 = 0xf;
   
		   //  WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);	 // gmv candidtae choose gmv  not  rmv 
		   WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 1);
						 WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, rmvx[9]+10);
		   WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, rmvy[9]);
   
		 
	   }
	   else if(FrameHoldflag_PQ_Issue)
	   {
		   FrameHoldflag_PQ_Issue=0;
		   WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,0,0,0x1); //s2_protect_en
		   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,fg_apl_s3_protect_th0_init);	
		   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,fg_apl_s3_protect_th1_init);   
		   WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,fg_apl_s3_protect_bgnum_th_init);   
		   WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,th0); 
		   WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,th0); 
		   WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,th0); 
		   WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,th0+5); 
							 //path4 = 0x0;
		   WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 0);	 // gmv candidtae choose gmv  not  rmv 
		   WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 1);
   
	   }
//----------------------------  transporter -----------------------------//
if(s_pContext->_external_data._output_frameRate == _PQL_OUT_100HZ)
{
	if((s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_transporter_status == 1)&&(s_pContext->_output_frc_sceneAnalysis.u8_dh_conditio_0026_1_status == 0)&&(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_skating_status==0)&&(s_pContext->_output_frc_sceneAnalysis.u8_dh_conditio_3003man_status==0))
	{
		video_transporter =1 ;
		WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,25); // bg_s1_num_yth2
		WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,14);  //  bg_s1_num_yth1 
		WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,31); // bg_s2_num_yth2
		WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,31);  //  bg_s2_num_yth1 
		WriteRegister(KME_ME1_BG1_ME_BG_S4_1_reg,6,6,0x0);//apl_set_en
		WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,0x0); //s3_ucov_data
		WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,0x0); //s3_cov_data
		
	}
	else if(video_transporter)
	{
		WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x10); // bg_s1_num_yth2
		WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0x4);  //  bg_s1_num_yth1 
		WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0xc); // bg_s2_num_yth2
		WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0xa);  //  bg_s2_num_yth1 
		WriteRegister(KME_ME1_BG1_ME_BG_S4_1_reg,6,6,0x1);//apl_set_en
		WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,0x1); //s3_ucov_data
		WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,0x1); //s3_cov_data
		video_transporter = 0 ;
	}

}
else
{
	if((s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_transporter_status == 1)&&(s_pContext->_output_frc_sceneAnalysis.u8_dh_conditio_0026_1_status == 0)&&(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_skating_status==0)&&(s_pContext->_output_frc_sceneAnalysis.u8_dh_conditio_3003man_status==0))
	{
		video_transporter =1 ;
		WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,26,26,0x0);  //bg_color_protect_en             
		WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,31,31,0x0); //apl_s2_protect_extend_en                           
		WriteRegister(KME_ME1_BG0_ME1_BG_3_reg, 0, 0, 0x0); //me1 black protect                    
		WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,0,0,0x0); //me1 black protect en 	  
	}
	else if(video_transporter)
	{
		video_transporter = 0 ;
		WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,26,26,0x1);  //bg_color_protect_en           
		WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,31,31,0x1); //apl_s2_protect_extend_en                         
		WriteRegister(KME_ME1_BG0_ME1_BG_3_reg, 0, 0, 0x1); //me1 black protect                    
		WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,0,0,0x1); //me1 black protect en	  
	}



}










	if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_dance_status == 1)
	{
			WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,26,26,0x0);  //bg_color_protect_en        
			WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,31,31,0x0); //apl_s2_protect_extend_en          

		if(u11_gmv_mvx<20)
		{
			WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,0x10);         
			WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,0x10);         
			WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,0x10);        
			WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,0x10+5);

			WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x8); //bg_s1_num_yth2
			WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0x1); //bg_s1_num_yth1
			WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x8);//bg_s2_num_yth2
			WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0x2); //bg_s2_num_yth1
			WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,15,21,4);
			WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,8,14,20);
		}
		else
		{
			WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,0x18);         
			WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,0x18);         
			WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,0x18);        
			WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,0x18+5);	
		}
		video_dance = 1 ;
	}
	else if(video_dance)
	{

		video_dance = 0 ;
		WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,26,26,0x1);  //bg_color_protect_en        
		WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,31,31,0x1); //apl_s2_protect_extend_en

		WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x10); //bg_s1_num_yth2
		WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0x4); //bg_s1_num_yth1
		WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x14);//bg_s2_num_yth2
		WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0xa); //bg_s2_num_yth1
		WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,15,21,bg_s3_dubt_ucov_yth1_init);
		WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,8,14,bg_s3_dubt_ucov_yth2_init);



		
	}




	if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_smallplane_status == 1)
	{
		video_smallplane =1 ;
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0x0); //pfv corr max x        
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0x0);//pfv corr max y	
		WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0xa);  //bg_s1_num_yth1
		WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x14);  //bg_s1_num_yth2
		WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30, 0x0); //bg_s3_ucov_data_dis
		WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31, 0x0); //bg_s3_cov_data_dis

	}
	else if(video_smallplane)
	{

		WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0x4);  //bg_s1_num_yth1
		WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x10);  //bg_s1_num_yth2
		video_smallplane = 0 ;
	}



	if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_soccer_status == 1 || 
		s_pContext->_output_me_sceneAnalysis.u1_SportScene == 1)
	{
		video_soccer =1 ;
		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_1_reg, 28, 28,  1  );  

	}
	else if(video_soccer)
	{
		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_1_reg, 28, 28,  0  );  
		video_soccer = 0 ;
	}


	
	//---------------------  dacne 2 --------------------------//
	if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_dance_2_status == 1)
	{
		
	        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,(rmvx[14]+rmvx[9])/2);
	        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);

	        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
	        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18,(rmvx[14]+rmvx[9])/2);
	        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28,(rmvy[14]+rmvy[9])/2);
	        
	        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31, 0);  //gmv_bg_en
	        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);//gmv_sel_en
		
		video_dance2 =1 ;
	}
	else if(video_dance2)
	{
	        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,u11_gmv_mvx);
	        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);
	        
	        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0);
	        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, u11_gmv_mvx);
	        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, u10_gmv_mvy);
	        
	        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31, 1);  //gmv_bg_en
	        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 0);//gmv_sel_en

		video_dance2 = 0 ;
	}

if(s_pContext->_external_data._output_frameRate != _PQL_OUT_100HZ)
{
	if((s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_transporter_status == 1)&&(s_pContext->_output_frc_sceneAnalysis.u8_dh_conditio_0026_1_status == 0)&&(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_skating_status==0)&&(s_pContext->_output_frc_sceneAnalysis.u8_dh_conditio_3003man_status==0))
	{
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0x0); //full range
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0xef);
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0);
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0x86); 
		WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30, 0x0); //bg_s3_ucov_data_dis
		WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31, 0x1); //bg_s3_cov_data_dis
	}
}

	if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_dance_status == 1)
	{
		if(u11_gmv_mvx<20)
		{		
			WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0x0); //full range
			WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0xef);
			WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0);
			WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0x86); 
		}
		
	}

	if((s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_skating_status == 1)&&(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_huapi2_status==0))
	{
		WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30, 0x1); //bg_s3_ucov_data_dis
		WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31, 0x1); //bg_s3_cov_data_dis
		WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,31,31,0x0); //apl_s2_protect_extend_en
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg,8,15,200); //bg_s4_apl_force_bg_th0
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg,0,7,220); //bg_s4_apl_force_bg_th1
		WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg,24,24,0x1);
		video_skating = 1;

		
		WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,0,0,0x0); 
		WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,0,0,0x0); 

		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,1990);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);

		//WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
		//WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, 1990);
		//WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, 0);

		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31, 0);  //gmv_bg_en
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);//gmv_sel_en


		path3 = 0xe;
		
	}
	else if(video_skating)
	{
		WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,31,31,0x1); //apl_s2_protect_extend_en
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg,8,15,0xff); //bg_s4_apl_force_bg_th0
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg,0,7,0xff); //bg_s4_apl_force_bg_th1
		WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg,24,24,0x0);
		video_skating = 0;

		WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,0,0,0x1); 
		WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,0,0,0x1); 

		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31,1);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,u11_gmv_mvx);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, u11_gmv_mvx);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, u10_gmv_mvy);


		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31, 1);  //gmv_bg_en
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 0);//gmv_sel_en
		path3 = 0xf;
		
	}


	if(s_pContext->_output_me_sceneAnalysis.u1_SportScene == 1){
		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_1_reg, 28, 28, 1 ); // new algo switch enable
		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_1_reg, 24, 25, 2 );
		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_1_reg, 8, 15, 255 );
		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_1_reg, 0, 7, 140 );
		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_2_reg, 0, 9, 10 ); //penalty
		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_2_reg, 22, 26, 20 );
		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_2_reg, 27, 31, 9 );
		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_3_reg, 2, 2, 0 ); //me1_small_obj_cand2_en
		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_3_reg, 3, 3, 0 );

	}
	else if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_C001soccer_status)
	{

		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_1_reg, 28, 28, 1 ); // new algo switch enable
		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_1_reg, 24, 25, 2 );
		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_1_reg, 0, 7, 176 );
		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_2_reg, 0, 9, 10 ); //penalty
		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_2_reg, 22, 26, 20 );
		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_2_reg, 27, 31, 9 );
		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_3_reg, 2, 2, 0 ); //me1_small_obj_cand2_en
		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_3_reg, 3, 3, 0 );

		video_C001soccer = 1;
	}
	else if(video_C001soccer)
	{

		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_1_reg, 28, 28, 0 ); // new algo switch enable
		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_1_reg, 24, 25, 0 );
		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_1_reg, 8, 15, 255 );
		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_1_reg, 0, 7, 140 );
		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_2_reg, 0, 9, 256 ); //penalty
		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_2_reg, 22, 26, 13 );
		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_2_reg, 27, 31, 7 );
		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_3_reg, 2, 2, 0 ); //me1_small_obj_cand2_en
		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_3_reg, 3, 3, 0 );

		video_C001soccer = 0;
	}



	//---------------------- 30006 ----------------------------//
	if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_30006_status == 1)
	{
		video30006 = 1 ;
		WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,0);
		WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,0);
		WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,0);

		WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,0);
		WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,0);
		WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,0);

		
	}
	else if(video30006)
	{
		video30006 = 0 ;
		WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,fg_apl_s2_protect_th0_init);
		WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,fg_apl_s2_protect_th1_init);
		WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,fg_apl_s2_protect_bgnum_th_init);

		WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,fg_apl_s3_protect_th0_init);
		WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,fg_apl_s3_protect_th1_init);
		WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,fg_apl_s3_protect_bgnum_th_init);


	}


	if(s_pContext->_output_frc_sceneAnalysis.u8_dh_conditio_3003man_status == 1)
	{
		
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0x0);
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0x0);
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0x0);
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0x0);

	}

	if(s_pContext->_output_frc_sceneAnalysis.u8_dh_conditio_3003man_status == 1)
	{
		
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0x0);
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0x0);
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0x0);
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0x0);

	}

	if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_rotterdam2_status == 1)
	{
		video_rotterdam = 1 ;
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0x0);
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0x0);
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0x0);
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0x0);

	}

	

	if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_ridebikegirl_status == 1)
	{
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,rmvx[26]+10);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);

		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, rmvx[26]+10);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, rmvy[26]);

		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31, 0);  //gmv_bg_en
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);//gmv_sel_en

		//WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,17); 
		//WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,17); 
		//WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,17); 
		//WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,22); 

		WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg,24,24,0x1); //s4_apl_force_bg_en
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg,8,15,180); //s4_apl_force_bg_th0
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg,0,7,0xff); //s4_apl_force_bg_th1
		
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0);
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0);
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0);
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0);

		
		video_ridebike = 1;
	}
	else if(video_ridebike>0)
	{
		video_ridebike = 0;
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31,1);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,u11_gmv_mvx);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);
		
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, u11_gmv_mvx);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, u10_gmv_mvy);


		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31, 1);  //gmv_bg_en
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 0);//gmv_sel_en

		//WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,th0); 
		//WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,th0); 
		//WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,th0); 
		//WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,th0+5); 

		WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg,24,24,0x0);
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg,8,15,0xff);
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg,0,7,0xff);//???????????
		

	}
	

	if((s_pContext->_output_frc_sceneAnalysis.u8_dh_conditio_toleft_bike_status == 1)&&(s_pContext->_output_frc_sceneAnalysis.u8_dh_conditio_3003man_status ==0))
	{
		WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,26,26,0x0);  //bg_color_protect_en        
		WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,31,31,0x0); //apl_s2_protect_extend_en  
		
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0x0); //full range
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0x0);
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0x0);
		WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0x0);
		
		video_toleftbike = 1;
	}
	else if(video_toleftbike>0)
	{
		WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,26,26,0x1);  //bg_color_protect_en        
		WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,31,31,0x1); //apl_s2_protect_extend_en          
		video_toleftbike = 0;
		
	}


	if((s_pContext->_output_frc_sceneAnalysis.u8_dh_conditio_0026_1_status == 1))
	{
						WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 1);
						WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, gmv_mvx_final);
						WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0);	
		video_0026 = 1;
	}
	else if(video_0026>0)
	{
						WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0);
						WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, 0);
						WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0);	

		video_0026 = 0;
		
	}
   
	   
   if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_insect_status == 1)
   {
	   video_insect = 1;
	   WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,32); 
	   WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,32); 
	   WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,32); 
	   WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,37); 
   
	   WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,8,15,240);
	   WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,24,31,135);
   
   }
   else if(video_insect>0)
   {
	   video_insect = 0;
	   WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,th0); 
	   WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,th0); 
	   WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,th0); 
	   WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,th0+5); 
	   
	   WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,8,15,0x0);
	   WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,24,31,0x0);
	   
   }

   if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_eagle_status == 1)
   {
	   video_eagle = 1;  
	   WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg,24,24,0x1);
	   WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg,8,15,90);
	   WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg,0,7,160);			   
	   
   
	   
	 }
	 else if(video_eagle)
	 {
	   video_eagle = 0;
	   WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg,24,24,0x0);
	   WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg,8,15,0x0);
	   WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg,0,7,0xff);	   
	 }

	if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_huapi2_status == 1)
	{
		video_huapi2 = 1;
		WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,31,31,0x0);
	}
	else if(video_huapi2>0)
	{
		video_huapi2 = 0;
		WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,31,31,0x1);
	}
        
	 if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_IDT_flag == 1)
	 {
			 video_IDT = 1;
			 WriteRegister(0xb8099768, 18, 19,0x1);
			 WriteRegister(MC2_MC2_50_reg,0,1,0x0);
	
	
			 WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31,0x1);// 0xb809EE38
			 WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 10, 20,0x0);
			 WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 21, 30,0x0);
	
			 WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,  8, 15,240);// 0xb809EF3C 
			 WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg, 24, 31,135);
			 WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,  0,  9,5);//	0xb809EFA8
			 WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg, 10, 19,5);
			 WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg, 20, 29,8);
			 WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,  0,  7,5);//	0xb809EE34
	
			 WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,0,0,0x0);  //s1_clr_prot_en
			 WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,1); //bg_s1_num_yth2
			 WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0x1); //bg_s1_num_yth1
			 WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,18,22,0xa); //bg_s1_xth2
			 WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,23,27,0x2); //bg_s1_xth1
			 WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,28,31,0x1);  //bg_s1_slope
	
			 WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,0,0,0x0); //s2_clr_prot_en			  
			 WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x1);	//bg_s2_num_yth2
			 WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0x1); //bg_s2_num_yth1
			 WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,18,22,0xc); //bg_s2_xth2
			 WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,23,27,0x2); //bg_s2_xth1
			 WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,28,31,0x1); //bg_s2_slope
	
			 WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,0x1);//	0xb809EE28
			 WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,0x1);//
			 WriteRegister(KME_ME1_BG0_ME1_BG_13_reg,26,26,0x0);//0xb809EE30
	
			 WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,8,14,4); //bg_s3_dubt_cov_num_yth2_init
			 WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,15,21,1); // bg_s3_dubt_cov_num_yth1_init
			 WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,4,7,3);   //bg_s3_dubt_cov_shift_init
			 WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,0,3,2); //bg_s3_dubt_cov_slope_init
	
			 WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,15,21, 1); //bg_s3_dubt_ucov_yth1_init
			 WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,8,14, 4); //bg_s3_dubt_ucov_yth2_init
			 WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,4,7,  3); //bg_s3_dubt_ucov_shift_init
			 WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,0,3,  2); // bg_s3_dubt_ucov_slope_init
			 WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,5,5,0);  //fg_hold_en_init
			 WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,0,4,25);	//fg_hold_th
	 }
	 else if(video_IDT>0)
	 {
			 video_IDT = 0;
			 WriteRegister(0xb8099768, 18, 19,0x2);
			 //WriteRegister(0xb809d564, 15, 15,0x0);
			 //WriteRegister(0xb8099750, 0, 1,0x3);
			 WriteRegister(MC2_MC2_50_reg,0,1,0x1);
	
			 WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31,0);
			 WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,0);
			 WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);
			 WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,8,15,0);
			 WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,24,31,0);
			 WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,12);
			 WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,12);
			 WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,25);
			 WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,12);
			
			 WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,0,0,0x1);  //s1_clr_prot_en
			 WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x10);
			 WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0x4);
			 WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,18,22,0xa);
			 WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,23,27,0x2);
			 WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,28,31,0x3);
			 
			 WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,0,0,0x1); //s2_clr_prot_en
			 WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x14);
			 WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0xa);
			 WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,18,22,0xc);
			 WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,23,27,0x2);
			 WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,28,31,0x3);
		   
			 WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,1);
			 WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,1);
			 WriteRegister(KME_ME1_BG0_ME1_BG_13_reg,26,26,1);
	
			 WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,15,21,0xa);
			 WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,8,14,0x22);
			 WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,4,7,0x1);
			 WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,0,3,0x2);
			 
			 WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,15,21,0x8);
			 WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,8,14,0x1e);
			 WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,4,7,0x1);
			 WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,0,3,0x3);
			 WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,5,5,0x0);	//fg_hold_en
			 WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,0,4,0x19);
	
			 
	 }
	
	
	
	
	
	
	
	//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 8, 11, path2); //path segment 

  // WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 4, 7, path2); //path segment 
}


int MEMC_me1_bgmask_mvdiffth_set(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
    unsigned int u32_RB_val;
    const _PQLCONTEXT *s_pContext = GetPQLContext();
    int th0 = 0;
    signed short gmv_fgx,gmv_fgy,gmv_bgx,gmv_bgy;
    int avr_region_fg_x,avr_region_fg_y;

    signed short u11_gmv_mvx;
    signed short u10_gmv_mvy;
    int mvdiff_bgfg  ;
    int mvdiff_gmvfg  ;  
    int mvdiff_bggmv  ; 
    int gmv_val;
    int mvdiff  ,fgmvdiff_x ,fgmvdiff_y,fgmvdiff;
    static unsigned int sum ;
    static unsigned int count,count_change ;
    int avr ;
    int path ;

    int me1_bg_pfv_corr_cond =0 ; 
    int me1_sad_bg_cond =1 ; 

//    int bg_s3_ucov_data_en_init=0x1;
//    int bg_s3_cov_data_en_init=0x1;
 //   int bg_s3_ucov_data_dis=0x0;
 //   int bg_s3_cov_data_dis=0x0;

    ReadRegister(KME_ME1_BG1_ME_BG_INFO_GMV01_reg,0,31, &u32_RB_val);
    gmv_fgx = ((u32_RB_val >> 20) &1) == 0? ((u32_RB_val>>10)& 0x7ff):(((u32_RB_val>>10)& 0x7ff)  - (1<<11));
    gmv_fgy = ((u32_RB_val >> 9) &1) == 0? (u32_RB_val& 0x3ff):((u32_RB_val& 0x3ff)  - (1<<10));
    gmv_bgy = ((u32_RB_val >> 30) &1) == 0? ((u32_RB_val>>21)& 0x3ff):(((u32_RB_val>>21)& 0x3ff)  - (1<<10));
    ReadRegister(KME_ME1_BG1_ME_BG_INFO_GMV00_reg,0,31, &u32_RB_val);
    gmv_bgx = ((u32_RB_val >> 26) &1) == 0? ((u32_RB_val>>16)& 0x7ff):(((u32_RB_val>>16)& 0x7ff)  - (1<<11)); 

    u11_gmv_mvx = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
    u10_gmv_mvy = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;


    //   ReadRegister(KME_ME1_BG1_ME_BG_INFO_GMV00_reg,0,14,&gmv_bg_cnt); 

    gmv_val=_ABS_(u11_gmv_mvx)>_ABS_(u10_gmv_mvy)?  _ABS_(u11_gmv_mvx)+_ABS_(u10_gmv_mvy)/2:  _ABS_(u10_gmv_mvy)+_ABS_(u11_gmv_mvx)/2;
    avr_region_fg_x=s_pContext->_output_frc_sceneAnalysis.region_fg_x;
    avr_region_fg_y=s_pContext->_output_frc_sceneAnalysis.region_fg_y;
    path = 0xf ;
    if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_large_fg_mvx==1)  // for 439 fast movtion 
    {
        // WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,bg_s1_mvdiff_bv_lower_th);
        // WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,bg_s2_mvdiff_ppi_lower_th);
        //  WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,bg_neargmv_diff_th);
        //   WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,bg_pfv_rep_mvdiff_th);  
        th0 =0x5f ;
        path=0xe;
    }
    else
    {

        fgmvdiff_x =  _ABS_DIFF_(gmv_fgx,avr_region_fg_x) ; 
        fgmvdiff_y =  _ABS_DIFF_(gmv_fgy,avr_region_fg_y) ; 
        fgmvdiff =  fgmvdiff_x> fgmvdiff_y  ? fgmvdiff_x+fgmvdiff_y/2 : fgmvdiff_x/2+fgmvdiff_y  ;
        
        if(fgmvdiff<30)  
        {
        mvdiff_bgfg = _ABS_DIFF_(gmv_bgx,avr_region_fg_x)> _ABS_DIFF_(gmv_bgy,avr_region_fg_y)  ? _ABS_DIFF_(gmv_bgx,avr_region_fg_x) : _ABS_DIFF_(gmv_bgy,avr_region_fg_y) ;
        mvdiff_gmvfg = _ABS_DIFF_(u11_gmv_mvx,avr_region_fg_x)> _ABS_DIFF_(u10_gmv_mvy,avr_region_fg_y)  ? _ABS_DIFF_(u11_gmv_mvx,avr_region_fg_x) : _ABS_DIFF_(u10_gmv_mvy,avr_region_fg_y) ;  
        }
        else
        {
            mvdiff_bgfg = _ABS_DIFF_(gmv_bgx,gmv_fgx)> _ABS_DIFF_(gmv_bgy,gmv_fgy)  ? _ABS_DIFF_(gmv_bgx,gmv_fgx) : _ABS_DIFF_(gmv_bgy,gmv_fgy) ;
            mvdiff_gmvfg = _ABS_DIFF_(u11_gmv_mvx,gmv_fgx)> _ABS_DIFF_(u10_gmv_mvy,gmv_fgy)  ? _ABS_DIFF_(u11_gmv_mvx,gmv_fgx) : _ABS_DIFF_(u10_gmv_mvy,gmv_fgy) ;             
        }
        if(mvdiff_bgfg> mvdiff_gmvfg)
        {
            mvdiff = mvdiff_bgfg ; 
        }
        else
        {
            mvdiff = mvdiff_gmvfg ; 
        }

		mvdiff_bggmv = _ABS_DIFF_(gmv_bgx,u11_gmv_mvx) + _ABS_DIFF_(gmv_bgy,u10_gmv_mvy) ;  
      //  rtd_pr_memc_info("gmv(%d %d %d %d)fg(%d %d) rmv(%d %d) diff(%d )\n",gmv_bgx , gmv_bgy ,u11_gmv_mvx,u10_gmv_mvy, gmv_fgx ,gmv_fgy, avr_region_fg_x,avr_region_fg_y ,mvdiff);

        if(mvdiff_bggmv<50)     
        {
            sum+=gmv_val;
            count++ ;
            avr=sum/count;
            if(_ABS_DIFF_(avr,gmv_val)>45)
            {
                count_change=count_change+1;
            }
            else
            {
                count_change=0;
            }
            if(count_change>5)
            {   
                avr=gmv_val;
                sum =0 ;
                count =0 ;
                count_change = 0 ;
            }
            else if(count>30&&count_change ==0)
            {
                sum = avr*4 ;
                count =4 ;
            }

            //  th0_pre = th0 ;
            // rtd_pr_memc_info("sum=%d count=%d avr=%d cnt=%d preth=%d\n",sum,count,avr,count_change , th0_pre);

            //	rtd_pr_memc_info("inmvdiff");				    

            if(avr>25||gmv_val>25)
            {
                if(avr>250||gmv_val>250||mvdiff>250)
                {
                    th0=85;
                    path = 0x2 ;
                }
                else if(avr>140||gmv_val>140||mvdiff>140)
                {
                    th0=68;
                    path = 0x3 ;
                }
                else if(avr>120||gmv_val>120||mvdiff>140)
                {
                    th0=58;
                    path = 0x3 ;
                }
                else if(avr>100||gmv_val>100||mvdiff>120)
                {
                    th0=50;
                    path = 0x4 ;
                }
                else if(avr>80||gmv_val>80||mvdiff>100)
                {
                    th0=45;
                    path = 0x4 ;
                }
                else if(avr>60||gmv_val>60)
                {
                    th0=25;
                    path = 0x5 ;
                }
				else if(avr>40||gmv_val>40)
                {
                    th0=18;
                    path = 0x6 ;
                }
                else
				//else if(avr>20||gmv_val>20)
                {
                    th0=15;
                    path = 0x7;
                }
				me1_bg_pfv_corr_cond =1 ; 
				
                if(path>5)
                {
                    me1_sad_bg_cond = 0; 
                }
                else
                {
                    me1_sad_bg_cond =1 ; 
                }

				if(mvdiff_bgfg>40&&path==6)
                {
                    me1_sad_bg_cond =1 ; 
                    path = 0xe ;
                }
				// rtd_pr_memc_info("avr=%d %d %d \n",avr,gmv_val , mvdiff );
            }
            else  //gmv  value is small 
            {
                if((_ABS_(gmv_fgy)<20)&&(_ABS_(gmv_fgx)<5)) //fg  move slow 
                {
                    me1_bg_pfv_corr_cond =0 ; 
                    me1_sad_bg_cond =1 ; 

                    th0= 12;
                    path=0x8;
                } 
                else if((_ABS_(gmv_fgx)<20)&&(_ABS_(gmv_fgy)<5))  
                {
                    th0=15;
                    path=0x9;
                    me1_bg_pfv_corr_cond =0 ; 
                    me1_sad_bg_cond =0 ;   
                }
                else if((_ABS_(gmv_fgx)>38)||(_ABS_(gmv_fgy)>32)||(_ABS_(avr_region_fg_y)+(_ABS_(avr_region_fg_x))>52))
                {
                    th0=20;
                    path=0xb;
                    me1_bg_pfv_corr_cond =1 ; 
                    me1_sad_bg_cond =1 ;   
					if(_ABS_(gmv_bgy)>3) // for 224 
					{
						path=0xd;
						me1_bg_pfv_corr_cond =0 ; 
						me1_sad_bg_cond =1 ;  
					}
                }
                else
                {                            
                    th0=12;
                    path =0xc ;
                    me1_bg_pfv_corr_cond =0 ; 
                    me1_sad_bg_cond =1 ;   
                }
            }
        }
        else   // if  bg mv and gmv diff >th , gmv may not  accurate £¬ need to disable the pfv corr 
        {
            path=0x1; 
            th0=20;
            me1_bg_pfv_corr_cond =0 ; 
            me1_sad_bg_cond =1 ;   
        }
    }
	//  WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 0, 3, path); //path segment 7

	pOutput->u1_me1_bg_pfv_corr_cond = me1_bg_pfv_corr_cond ;
    pOutput->u1_me1_sad_bg_cond = me1_sad_bg_cond ;

    return th0 ;
}

VOID MEMC_me1_bgmask_newalgo_WrtAction(const _PARAM_WRT_COM_REG *pParam ,_OUTPUT_WRT_COM_REG *pOutput)
{
    const _PQLCONTEXT *s_pContext = GetPQLContext();
    unsigned int fb_cnt_array[32] ;
    static int th0 ;
    static int framenum ;
    static bool BSC;
    static int gmv_bg_cnt =0 ;
    static int gmv_bg_cnt_pre = 0;    
    static int frame_cnt;
    static int th0_count;   
	static int th0_cur;

    int SCFlag , panningFlag;
    static int th0_pre;
    static int  badfgholdframe  ;
    static int  badfgframe  ; 
    
    int path2; 
    static int  largeFastFrameHold ,SCFrameHold ; 
    static bool blargefast  ,bschold;
    static int  highaplFrameHold ; 
    static bool bhighapl  ;   
    static int holdframe315 , holdframe306 ,holdframe163 ,holdframe129 ,holdframe171   ,holdframe172 ,holdframe298  ; 
    static bool  video315 , video306 ,video163 , video129 , video171 , video172 ,video298  ;
    static int holdframe_156_mv,holdframe_156_th;

	int  fwcontrol_298 ;

    static bool   vertical_slow_165_flag,  ver_hor_move_flag ,ver_hor_move_flag0,  ver_hor_move_flag1 , ver_hor_move_flag2  ;
    static int    vertical_slow_165_holdframe  ,ver_hor_move_holdframe ;
    int bg_s3_mode0_cov_yth2_init=0xe;
    int bg_s3_mode0_cov_shift_init=0x1;
    int bg_s3_mode0_cov_slope_init=0x3;
    int bg_s3_mode0_ucov_yth2_init=0x14;
    int bg_s3_mode0_ucov_shift_init=0x0;
    int bg_s3_mode0_ucov_slope_init=0x1;
    int bg_s3_mode0_cond_en_init=0x0;
    int bg_s3_dubt_cov_num_yth1_init=0xa;
    int bg_s3_dubt_cov_num_yth2_init=0x22;
    int bg_s3_dubt_cov_shift_init=0x1;
    int bg_s3_dubt_cov_slope_init=0x2;
    int bg_s3_dubt_ucov_yth1_init=0x8;
    int bg_s3_dubt_ucov_yth2_init=0x1e;
    int bg_s3_dubt_ucov_shift_init=0x1;
    int bg_s3_dubt_ucov_slope_init=0x3;
     // int bg_s3_dubt_cov_sad_th_init = 0x1c2 ;  
    int bg_s3_cov_largesad_th_init=0x12c;
    int bg_s3_cov_smallsad_th_init=0x78;
    int bg_s3_cov_setbg_th_init=0x190;
    // int bg_s3_dubt_ucov_sad_th_init = 0x1c2 ;  
    int bg_s3_ucov_smallsad_th_init=0x78;
	int bg_s3_ucov_largesad_th_init=0x12c;
    int bg_s3_ucov_setbg_sad_th_init=0x190;   
	
    int fg_apl_protect_th1_init=0x28;
    int fg_apl_protect_bgnum_th_init=0x20;
    int rmv_fg_mvd_merge_thd_init=0x32;  
    

    
    int bg_s1_mvdiff_bv_lower_th_init=0x14;
    int bg_s2_mvdiff_ppi_lower_th_init=0x14;
    int bg_neargmv_diff_th_init=0x14;
    int bg_pfv_rep_mvdiff_th_init=0x19;
    
    //color_portect 317 +299
    int fg_apl_s2_protect_th0_init=0x0;
    int fg_apl_s2_protect_th1_init=0x1e;
    int fg_apl_s2_protect_bgnum_th_init=0x32;
    
    //color_protect 299
    int fg_apl_s3_protect_th0_init=0x0;
    int fg_apl_s3_protect_th1_init=0x28;
    int fg_apl_s3_protect_bgnum_th_init=0x2d; 
    int fg_apl_s3_protect_mvcond_dis = 0x0 ;
    
    int fg_mask_pfv_blend_en_init=0x1;
    // int pfv_corr_en_init=0x0;
    int bg_s3_ucov_data_en_init=0x1;
    int bg_s3_cov_data_en_init=0x1;
    int fg_pfv_rep_mvdiff_th_init=0x14;
    int fg_hold_th_init=0x19;
    int fg_hold_en_init=0x0;
    int fg_mask_en_init = 0x0 ;

    int bg_s3_mode0_cov_yth2=0xA;
    int bg_s3_mode0_cov_shift=0x0;
    int bg_s3_mode0_cov_slope=0x1;
    int bg_s3_mode0_ucov_yth2=0x12;
    int bg_s3_mode0_ucov_shift=0x2;
    int bg_s3_mode0_ucov_slope=0x5;
    int bg_s3_mode0_cond_en=0x1;
    int bg_s3_dubt_cov_num_yth1=0x8;
    int bg_s3_dubt_cov_num_yth2=0x10;
    int bg_s3_dubt_cov_shift=0x0;
    int bg_s3_dubt_cov_slope=0x1;
    int bg_s3_dubt_ucov_yth1=0x4;
    int bg_s3_dubt_ucov_yth2=0x17;
    int bg_s3_dubt_ucov_shift=0x2;
    int bg_s3_dubt_ucov_slope=0x3;
    int bg_s3_cov_largesad_th=0x190;
    int bg_s3_cov_smallsad_th=0x9b;
	
    //int bg_s3_cov_setbg_th=0xcc;
    int bg_s3_ucov_smallsad_th=0x9b;
    int fg_apl_protect_th1=0x20;
    int fg_apl_protect_bgnum_th=0x42;  //  0x30  
    int rmv_fg_mvd_merge_thd=0x10;       
    int bg_s3_ucov_largesad_th=0xc8;
    int bg_s3_ucov_setbg_sad_th=0xfa;
	int bg_s3_ucov_data_dis=0x0;
    int bg_s3_cov_data_dis=0x0;

    //color_portect 299
    int fg_apl_s2_protect_th0_299=0x96;
    int fg_apl_s2_protect_th1_299=0xfa;
    int fg_apl_s2_protect_bgnum_th_299=0x2d;   
     //color_protect 299
    int fg_apl_s3_protect_th0=0x96;
    int fg_apl_s3_protect_th1=0xfa;
    int fg_apl_s3_protect_bgnum_th=0x2d;        
    int fg_mask_pfv_blend_dis=0x0; 
    //int pfv_corr_en=0x1;

    int fg_hold_th=0x17;
    int fg_hold_en=0x1;
	int readval15;

    signed short fg_apl_seg0_cnt=s_pContext->_output_read_comreg.dh_fg_apl_seg0_cnt;
    signed short fg_apl_seg1_cnt=s_pContext->_output_read_comreg.dh_fg_apl_seg1_cnt;
    signed short fg_apl_seg2_cnt=s_pContext->_output_read_comreg.dh_fg_apl_seg2_cnt;
    signed short fg_apl_seg3_cnt=s_pContext->_output_read_comreg.dh_fg_apl_seg3_cnt;
    signed short fg_apl_seg4_cnt=s_pContext->_output_read_comreg.dh_fg_apl_seg4_cnt;
    signed short fg_apl_seg5_cnt=s_pContext->_output_read_comreg.dh_fg_apl_seg5_cnt;
    signed short fg_apl_seg6_cnt=s_pContext->_output_read_comreg.dh_fg_apl_seg6_cnt;
    signed short fg_apl_seg7_cnt=s_pContext->_output_read_comreg.dh_fg_apl_seg7_cnt;

    static signed short gmv_fgx,gmv_fgy,gmv_bgx,gmv_bgy;
  
    int avr_region_fg_x,avr_region_fg_y;
    unsigned int u32_RB_val;
    signed short u11_gmv_mvx;
    signed short u10_gmv_mvy;
    static signed short gmv_mvx_sum =0 ; 
    static signed short gmv_mvy_sum =0 ;
    static unsigned short gmv_frame_cnt =0 ;
    static signed short gmv_mvx_avr=0 ; 
    static signed short gmv_mvy_avr =0 ;
    int gmvdiff_avr_cur ;
    signed short gmv_mvx_target;
    signed short gmv_mvy_target;
    signed short gmv_mvx_final;
    signed short gmv_mvy_final;
    signed short u11_mv_max_rx;
    signed short u10_mv_max_ry;
    signed short u11_mv_min_rx;
    signed short u10_mv_min_ry;

      
    static int change_frm_cnt  ; 
    int mvdiff_bgfg , mvdiff_gmvfg ,mvdiff_bggmv ,mvdiff_varfg ,mvdiff_curfg ;
    int fg_apl_sum=fg_apl_seg0_cnt+fg_apl_seg1_cnt+fg_apl_seg2_cnt+fg_apl_seg3_cnt+fg_apl_seg4_cnt+fg_apl_seg5_cnt+fg_apl_seg6_cnt+fg_apl_seg7_cnt;
	//   int bg_apl_sum=bg_apl_seg0_cnt+bg_apl_seg1_cnt+bg_apl_seg2_cnt+bg_apl_seg3_cnt+bg_apl_seg4_cnt+bg_apl_seg5_cnt+bg_apl_seg6_cnt+bg_apl_seg7_cnt;  // need to code refine 
    int x_range_old , y_range_old ;
    int rgnidx ,  fb_lvl_sum , fb_lvl_max  ; 

	static  int me1_bg_pfv_corr_cond  ; 
    bool pfv_corr_dis_cond1   ;// pfv_corr_dis_cond2
    // static  int me1_bg_pfv_corr_cond  ; 

	//static int holdgmv399frame ; 
    int fwgmven  , readval,realval1;
    static int reg_me1_gmv_sel_old ;
    static int sc_s2_apl_prot_off   ,  sc_s2_apl_prot_off_holdframe  , corr_dis_holdframe ;
    
    int gmvx_test  ,gmvy_test ;

    int rgnIdx ,fgleftcnt, bgrightcnt;
    int Dtl_array[32]={0};
    int rmv_mvx[32]={0};
    int rmv_mvy[32]={0};
	int rgn_bgcnt[32]={0};
    static bool video_156,video_1561;
    int s11_2nd_gmv_mvx,s10_2nd_gmv_mvy;
    //static int incnt_156=0;
    int rgn_aplp[32]={0};
    int logo_array[32]={0};
    int logo_sum=0;
	int enbale_readval = 0;

 	ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 11, 11, &enbale_readval);
	ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,15 , 15 , &readval15);
    
	framenum = 0;
    BSC = 1;
    path2 = 0x0 ;


	SCFlag = s_pContext->_output_read_comreg.u1_sc_status_rb;
	panningFlag =  s_pContext->_output_me_sceneAnalysis.u2_panning_flag ; 
    //********bg gmv force value for hw gmv bug apply start  ******//
    if(pParam->u1_me1_condition1)   // b809d560 [13]    
    {
        ReadRegister(KME_ME1_BG1_ME_BG_INFO_GMV01_reg,0,31, &u32_RB_val);
        gmv_fgx = ((u32_RB_val >> 20) &1) == 0? ((u32_RB_val>>10)& 0x7ff):(((u32_RB_val>>10)& 0x7ff)  - (1<<11));
        gmv_fgy = ((u32_RB_val >> 9) &1) == 0? (u32_RB_val& 0x3ff):((u32_RB_val& 0x3ff)  - (1<<10));
        gmv_bgy = ((u32_RB_val >> 30) &1) == 0? ((u32_RB_val>>21)& 0x3ff):(((u32_RB_val>>21)& 0x3ff)  - (1<<10));
        ReadRegister(KME_ME1_BG1_ME_BG_INFO_GMV00_reg,0,31, &u32_RB_val);
        gmv_bgx = ((u32_RB_val >> 26) &1) == 0? ((u32_RB_val>>16)& 0x7ff):(((u32_RB_val>>16)& 0x7ff)  - (1<<11)); 

        u11_gmv_mvx = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
        u10_gmv_mvy = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;

		u11_mv_max_rx  = s_pContext->_output_read_comreg.s11_me_mv_max_vx_rb; 
        u10_mv_max_ry  = s_pContext->_output_read_comreg.s10_me_mv_max_vy_rb; 
        u11_mv_min_rx   = s_pContext->_output_read_comreg.s11_me_mv_min_vx_rb; 
        u10_mv_min_ry   = s_pContext->_output_read_comreg.s10_me_mv_min_vy_rb; 

        //   ReadRegister(KME_ME1_BG1_ME_BG_INFO_GMV00_reg,0,14,&gmv_bg_cnt); 
        avr_region_fg_x=s_pContext->_output_frc_sceneAnalysis.region_fg_x;
        avr_region_fg_y=s_pContext->_output_frc_sceneAnalysis.region_fg_y;
        if(SCFlag) //SCflag
        {
            gmv_mvx_sum=u11_gmv_mvx;
            gmv_mvy_sum=u10_gmv_mvy;          
            gmv_frame_cnt=1 ;
            gmv_mvx_avr =gmv_mvx_sum/gmv_frame_cnt;
            gmv_mvy_avr =gmv_mvy_sum/gmv_frame_cnt;  
            gmv_mvx_target  = gmv_mvx_avr ; 
            gmv_mvy_target  = gmv_mvy_avr ;   
            SCFrameHold =10 ;
            bschold = 1 ;
        }
        else if(SCFrameHold>0)
        {
            SCFrameHold -- ;
            gmv_mvx_sum=u11_gmv_mvx;
            gmv_mvy_sum=u10_gmv_mvy;          
            gmv_frame_cnt=1 ;
            gmv_mvx_avr =gmv_mvx_sum/gmv_frame_cnt;
            gmv_mvy_avr =gmv_mvy_sum/gmv_frame_cnt;  
            gmv_mvx_target  = gmv_mvx_avr ; 
            gmv_mvy_target  = gmv_mvy_avr ;    
        }                 
        else
        {
        	bschold = 0 ;
            gmvdiff_avr_cur = _ABS_DIFF_(gmv_mvx_avr,u11_gmv_mvx)> _ABS_DIFF_(gmv_mvy_avr,u10_gmv_mvy)  ? _ABS_DIFF_(gmv_bgx,avr_region_fg_x)+_ABS_DIFF_(gmv_bgx,avr_region_fg_x)/2 : _ABS_DIFF_(gmv_bgx,avr_region_fg_x)+_ABS_DIFF_(gmv_bgx,avr_region_fg_x)/2 ;              
            if(gmvdiff_avr_cur>30)
            {  
                change_frm_cnt ++ ;   
                mvdiff_varfg = _ABS_DIFF_(gmv_mvx_avr,avr_region_fg_x)> _ABS_DIFF_(gmv_mvy_avr,avr_region_fg_y)  ? _ABS_DIFF_(gmv_mvx_avr,avr_region_fg_x) : _ABS_DIFF_(gmv_mvy_avr,avr_region_fg_y) ;
                mvdiff_curfg = _ABS_DIFF_(u11_gmv_mvx,avr_region_fg_x)> _ABS_DIFF_(u10_gmv_mvy,avr_region_fg_y)  ? _ABS_DIFF_(u11_gmv_mvx,avr_region_fg_x) : _ABS_DIFF_(u10_gmv_mvy,avr_region_fg_y) ;  
                if(mvdiff_varfg>mvdiff_curfg&&gmv_bg_cnt>18000)
                {
                    gmv_mvx_target  = gmv_mvx_avr ; 
                    gmv_mvy_target  = gmv_mvy_avr ;     
                }
                else
                {
                    gmv_mvx_target  = u11_gmv_mvx ; 
                    gmv_mvy_target  = u10_gmv_mvy ; 
                }                                  
            }
            else 
            {
                gmv_mvx_target =  (u11_gmv_mvx+gmv_mvx_avr)>>1 ;
                gmv_mvy_target =  (u10_gmv_mvy+gmv_mvy_avr)>>1;
                gmv_mvx_sum+=u11_gmv_mvx;
                gmv_mvy_sum+=u10_gmv_mvy;          
                gmv_frame_cnt++ ;
                gmv_mvx_avr =gmv_mvx_sum/gmv_frame_cnt;
                gmv_mvy_avr =gmv_mvy_sum/gmv_frame_cnt;  
                change_frm_cnt = 0 ;
            }

            if(change_frm_cnt>=5)
            {
                change_frm_cnt = 0 ; 
                gmv_mvx_sum= u11_gmv_mvx*2;
                gmv_mvy_sum=u10_gmv_mvy*2;          
                gmv_frame_cnt =2  ;
                gmv_mvx_target =  u11_gmv_mvx ;
                gmv_mvy_target =  u10_gmv_mvy;
				gmv_mvx_avr =gmv_mvx_sum/gmv_frame_cnt;
                gmv_mvy_avr =gmv_mvy_sum/gmv_frame_cnt;  
            }
            else if(gmv_frame_cnt>15)
            {
                gmv_mvx_sum = gmv_mvx_avr *5 ;
                gmv_mvy_sum = gmv_mvy_avr*5 ;
                gmv_frame_cnt =5 ;
               // gmv_same_cnt++ ;                
            }
        }
	   if(readval15==1)
	   {	   
		//rtd_pr_memc_info("sum(%d %d)avr(%d %d) cur(%d %d) tar(%d %d)\n" ,gmv_mvx_sum,gmv_mvy_sum,gmv_mvx_avr , gmv_mvy_avr ,u11_gmv_mvx ,u10_gmv_mvy ,gmv_mvx_target ,gmv_mvy_target) ;

		rtd_pr_memc_info("skywoth_cond:%d %d %d %d %d %d %d\n",s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_transporter_status,s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_dance_status ,s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_rotterdam_status,s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_dance_2_status,s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_skating_status ,s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_30006_status ,s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_ridebikegirl_status);


	   }
       //  rtd_pr_memc_info("cnt(%d %d)\n"  ,gmv_frame_cnt ,  change_frm_cnt) ;

        mvdiff_bgfg = _ABS_DIFF_(gmv_bgx,avr_region_fg_x)> _ABS_DIFF_(gmv_bgy,avr_region_fg_y)  ? _ABS_DIFF_(gmv_bgx,avr_region_fg_x) : _ABS_DIFF_(gmv_bgy,avr_region_fg_y) ;
        mvdiff_gmvfg = _ABS_DIFF_(gmv_mvx_target,avr_region_fg_x)> _ABS_DIFF_(gmv_mvy_target,avr_region_fg_y)  ? _ABS_DIFF_(u11_gmv_mvx,avr_region_fg_x) : _ABS_DIFF_(u10_gmv_mvy,avr_region_fg_y) ;  
        mvdiff_bggmv = _ABS_DIFF_(gmv_bgx,gmv_mvx_target) + _ABS_DIFF_(gmv_bgy,gmv_mvy_target) ;  
  		if(bschold)
        {
            gmv_mvx_final = gmv_mvx_target ;
            gmv_mvy_final = gmv_mvy_target ;
        }
        else
        {
	        if(mvdiff_bggmv<30)
	        {           
	            if(mvdiff_bgfg>mvdiff_gmvfg)
	            {
	                gmv_mvx_final = (gmv_bgx*3+gmv_mvx_target)>>2 ;
	                gmv_mvy_final = (gmv_bgy*3+gmv_mvy_target)>>2 ;
	            }
	            else
	            {
	                gmv_mvx_final = (gmv_mvx_target*3+gmv_bgx)>>2 ;
	                gmv_mvy_final = (gmv_mvy_target*3+gmv_bgy)>>2 ;
	            }   
	        }
	        else 
	        {
	        	if(gmv_bg_cnt>18000)
           		{
		            if(mvdiff_bgfg>mvdiff_gmvfg)
		            {
		                gmv_mvx_final = gmv_bgx ;
		                gmv_mvy_final = gmv_bgy ;
		            }
		            else
		            {
		                gmv_mvx_final = gmv_mvx_target ;
		                gmv_mvy_final = gmv_mvy_target ;
		            }
		        }
				else
	            {
	                gmv_mvx_final = gmv_mvx_target ;
	                gmv_mvy_final = gmv_mvy_target ;
	            }
	          //  rtd_pr_memc_info("fw refine new\n");
	        }
        }
		//rtd_pr_memc_info("final(%d %d) fg(%d %d) %d %d\n" ,gmv_mvx_final , gmv_mvy_final ,avr_region_fg_x ,avr_region_fg_y , mvdiff_bgfg , mvdiff_gmvfg) ;
		ReadRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, &fwgmven);
        if(!fwgmven) // only fw gmv not be set bg use bg fw gmv 
        {

		if(_ABS_(gmv_mvy_final)<(_ABS_(gmv_mvx_final)+4))    // 
        {
            WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 1);
            WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, gmv_mvx_final);
            WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0);  
           // WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, gmv_mvy_final);
        }
        else 
        {
            WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0);
        }
                         
			   // ++++if  bg  vertical hortical is similar and >30	 fw bg gmv need close and  gmv candidta choose	gmv start 
				pOutput->u1_me1_bg_ver_hor_candsel = 0 ;
				//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 4, 7, 0); //path segment 
				ver_hor_move_flag = ver_hor_move_flag0||ver_hor_move_flag1||ver_hor_move_flag2 ;
				if(ver_hor_move_flag==0)
				{
					 ReadRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, &reg_me1_gmv_sel_old);	   
				}
				
				if((_ABS_DIFF_(_ABS_(u11_gmv_mvx),_ABS_(u10_gmv_mvy)) <10)&&((_ABS_(u10_gmv_mvy)>30)&&(_ABS_(u11_gmv_mvx)+_ABS_(u10_gmv_mvy))>50))
				{
		
					WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);   // gmv candidtae choose gmv  not	rmv 
					WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0);
                ver_hor_move_flag0= 1 ; 
					ver_hor_move_holdframe = 90 ; 
					pOutput->u1_me1_bg_ver_hor_candsel = 1 ; 
		
					path2 = 1 ;
				}
				else if(ver_hor_move_holdframe>0)
				{
					ver_hor_move_holdframe-- ;
					WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);   // gmv candidtae choose gmv  not	rmv 
					WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0);
		
					pOutput->u1_me1_bg_ver_hor_candsel = 1 ; 
					path2 = 1 ;
				}
				else if(ver_hor_move_flag0)
				{
					ver_hor_move_flag0 =0 ;
					pOutput->u1_me1_bg_ver_hor_candsel = 0 ;  
					
					WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 0);
					WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 1);
					path2 = 0 ;
		
				}	
			   
			 // ---- bg  vertical hortical is similar and >30	fw bg gmv need close and  gmv candidta choose  gmv end 
				//+++++++++++++++++315 181 horse vertical moving video++++++++++++++//
				if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_ver_case1==1)
				{
					holdframe315=  (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 340 : 170 ; 
					video315=1;
					path2 = 0xe ;
					ver_hor_move_flag1 = 1 ; 
					WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);   // gmv candidtae choose gmv  not	rmv 
					WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0);
					 // rtd_pr_memc_info("ver case1\n") ;
			}
				else if(holdframe315>0)
				{
					 path2 =0xf ;
					 holdframe315 -- ; 
		
					WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);   // gmv candidtae choose gmv  not	rmv 
					WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0);			 
				}
				else if(video315)
				{
					ver_hor_move_flag1 = 0 ; 
					video315 = 0 ;
					path2 = 0 ;
					WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 0);
					WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 1);
				} 
		
		   //+++++++++++++++++306 bike	vertical moving video ++++++++++++++//
				if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_virtical_case2==1)
				{
					holdframe306= 260 ;  
					video306=1;
					path2 = 2 ;
					WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);   // gmv candidtae choose gmv  not	rmv 
					WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0);
					WriteRegister(HARDWARE_HARDWARE_25_reg, 17,17 , 1);
					ver_hor_move_flag2 =1 ;
					 // rtd_pr_memc_info("ver case1\n") ;
		
				}
				else if(holdframe306>0)
				{
					 path2 = 2 ;
					 holdframe306 -- ; 
				   
					WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);   // gmv candidtae choose gmv  not	rmv 
					WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0);			 
					WriteRegister(HARDWARE_HARDWARE_25_reg, 17,17 , 1);
			   
				}
				else if(video306)
				{
					video306 = 0 ;
					path2 = 0x0 ;
					WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 0);
					WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 1);
					WriteRegister(HARDWARE_HARDWARE_25_reg, 17,17 , 0);
					ver_hor_move_flag2 =0 ;
				   		
				} 
				 
				 
			}
		
			//--------------------156 gmv cor------------------//
			for(rgnIdx=0;rgnIdx<32;rgnIdx++)
			{
				rmv_mvx[rgnIdx] = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[rgnIdx]	; 
				rmv_mvy[rgnIdx] = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[rgnIdx] ;
			}
			
			for(rgnIdx=0;rgnIdx<32;rgnIdx++)
			{
				ReadRegister(KME_ME1_TOP7_ME1_STATIS_DTL_01_reg+4*rgnIdx ,0,19,&u32_RB_val);  
				Dtl_array[rgnIdx]=u32_RB_val;	 
			}
			   //rtd_pr_memc_info("------------------dtl ------------\n");
			   //rtd_pr_memc_info("%d %d %d %d %d %d %d %d\n", Dtl_array[0] ,Dtl_array[1],Dtl_array[2],Dtl_array[3],Dtl_array[4],Dtl_array[5],Dtl_array[6],Dtl_array[7]);
			   //rtd_pr_memc_info("%d %d %d %d %d %d %d %d\n", Dtl_array[8] ,Dtl_array[9],Dtl_array[10],Dtl_array[11],Dtl_array[12],Dtl_array[13],Dtl_array[14],Dtl_array[15]);
			   //rtd_pr_memc_info("%d %d %d %d %d %d %d %d\n", Dtl_array[16] ,Dtl_array[17],Dtl_array[18],Dtl_array[19],Dtl_array[20],Dtl_array[21],Dtl_array[22],Dtl_array[23]);
			   //rtd_pr_memc_info("%d %d %d %d %d %d %d %d\n", Dtl_array[24] ,Dtl_array[25],Dtl_array[26],Dtl_array[27],Dtl_array[28],Dtl_array[29],Dtl_array[30],Dtl_array[31]);
			
			s11_2nd_gmv_mvx = s_pContext->_output_read_comreg.s11_me_GMV_2nd_vx_rb;
			s10_2nd_gmv_mvy = s_pContext->_output_read_comreg.s10_me_GMV_2nd_vy_rb;
			
			  for(rgnIdx =0 ; rgnIdx< 32; rgnIdx++)
			  {
				  ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_80_reg+4*rgnIdx ,0,19,&u32_RB_val);   
				  rgn_aplp[rgnIdx]=u32_RB_val;
			  } 
		
		
			for(rgnIdx=0;rgnIdx<32;rgnIdx++)
			{
				ReadRegister(KME_LOGO2_KME_LOGO2_80_reg+4*rgnIdx ,20,29,&u32_RB_val);  
				logo_array[rgnIdx]=u32_RB_val;	  
			}
			
			for(rgnIdx=0;rgnIdx<32;rgnIdx++)
			{
				logo_sum+=logo_array[rgnIdx];
			}
			
			if((Dtl_array[12]<80000 && Dtl_array[12]>40000)&&(Dtl_array[20]<100000 && Dtl_array[20]>50000)&&(Dtl_array[21]<120000 && Dtl_array[21]>70000)&&
			(Dtl_array[29]<80000 && Dtl_array[29]>30000) &&(Dtl_array[28]<75000 && Dtl_array[28]>40000) &&(Dtl_array[7]<55000 && Dtl_array[7]>5000) &&(Dtl_array[4]<65000 && Dtl_array[4]>15000)&& Dtl_array[24]<65000 && (rgn_aplp[12]>70000 && rgn_aplp[12]<135000) && (rgn_aplp[20]<120000 &&rgn_aplp[20]>40000)
			&& (rgn_aplp[24]<15000)&&(rgn_aplp[0]<50000 && rgn_aplp[0]>20000)&&(rgn_aplp[8]<80000 &&rgn_aplp[8]>20000)&& _ABS_(rmv_mvx[15])<100)
			{
				
				WriteRegister(HARDWARE_HARDWARE_25_reg, 13,13 , 0x1);
				video_156=1;			 
				WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31,0);
				WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,rmv_mvx[23]);
				WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);			  
		
				WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,26,26,0x0);   //color_protect	
		
				WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
				WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, rmv_mvx[23]);
				WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, rmv_mvy[23]);
		
				WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);//gmv_sel_en
			  
				//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x1);  
		
				holdframe_156_mv = 80;
		
								  
			}
			else if(holdframe_156_mv>0)
			{
				WriteRegister(HARDWARE_HARDWARE_25_reg, 13,13 , 0x1);
				video_156=1;			 
				WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31,0);
				WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,rmv_mvx[15]);
				WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);			  
		
				WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,26,26,0x0);   //color_protect	
		
				WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
				WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, rmv_mvx[15]);
				WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, rmv_mvy[15]);
		
				WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);//gmv_sel_en
			  
				//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x1);  
		
				holdframe_156_mv--;
			}
		
		
			
			else if(video_156)
			{
			
				WriteRegister(HARDWARE_HARDWARE_25_reg, 13,13 , 0x0);
				video_156=0;   
				WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31,1);
				WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,u11_gmv_mvx);
				WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);
				
				WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,26,26,0x1);
		
				WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0,0);
				WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, u11_gmv_mvx);
				WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, u10_gmv_mvy);
		
				WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 0);//gmv_sel_en
		
				//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x0); 
		
			}	  
		
		}

     //********bg gmv force value for hw gmv bug apply end  ******//
    for(rgnIdx =0 ;  rgnIdx< 31; rgnIdx++)
    {
        fb_cnt_array[rgnIdx] = s_pContext->_output_read_comreg.u12_region_fb_cnt[rgnIdx] ; 
    }

    fb_lvl_sum =0 ;
    fb_lvl_max = 0 ;
    
    for (rgnidx = 0 ; rgnidx <32 ; rgnidx++)
    {
        fb_lvl_sum += fb_cnt_array[rgnidx] ; 
        if(fb_lvl_max < fb_cnt_array[rgnidx])
        {
            fb_lvl_max = fb_cnt_array[rgnidx] ; 
        }
    }  
	//  rtd_pr_memc_info("fb=(%d %d %d %d %d %d %d %d)\n" ,fb_cnt_array[16],fb_cnt_array[17],fb_cnt_array[18],fb_cnt_array[19],fb_cnt_array[20],fb_cnt_array[21],fb_cnt_array[22],fb_cnt_array[23] ) ;
	//  rtd_pr_memc_info("fb=(%d %d %d %d %d %d %d %d)\n" ,fb_cnt_array[24],fb_cnt_array[25],fb_cnt_array[26],fb_cnt_array[27],fb_cnt_array[28],fb_cnt_array[29],fb_cnt_array[30],fb_cnt_array[31] ) ;
	//   rtd_pr_memc_info("fb=(%d %d %d %d %d %d %d %d)\n" ,fb_cnt_array[0],fb_cnt_array[1],fb_cnt_array[2],fb_cnt_array[3],fb_cnt_array[4],fb_cnt_array[5],fb_cnt_array[6],fb_cnt_array[7] ) ;
	//   rtd_pr_memc_info("fb=(%d %d %d %d %d %d %d %d)\n" ,fb_cnt_array[8],fb_cnt_array[9],fb_cnt_array[10],fb_cnt_array[11],fb_cnt_array[12],fb_cnt_array[13],fb_cnt_array[14],fb_cnt_array[15] ) ;

	//  rtd_pr_memc_info("fb=(%d %d)\n" ,fb_lvl_sum ,fb_lvl_max ) ;
    if((pParam->u1_me1_Newalgo_Proc_en_all)||(enbale_readval == 1)) // b809d560 [18]      
   {

	//   WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0x0);
	//  WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0xef);
	//   WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0x0);
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,fg_apl_s3_protect_bgnum_th_init);    
        WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,fg_apl_s2_protect_bgnum_th_init); 
        WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,24,24,fg_apl_s3_protect_mvcond_dis); // disable

		 //    WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,0,0,0x1); //me1 black protect en
		//	   WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,0,0,0x1); //me1 black protect en
		gmv_bg_cnt_pre = gmv_bg_cnt ;	   
		ReadRegister(KME_ME1_BG1_ME_BG_INFO_GMV00_reg,0,14,&gmv_bg_cnt); 


        if((pParam->u1_me1_Newalgo_Proc_en)||(enbale_readval == 1))   // b809d560 [30] 
        {
    
            //***** set MV diff th start *****//
            if(pParam->u1_me1_condition2)   // b809d560 [17]  
            {	
                th0_cur = MEMC_me1_bgmask_mvdiffth_set(pParam  ,pOutput ) ;     
          
                if(_ABS_DIFF_(th0_pre,th0_cur)>30)
                {
                    th0=th0_pre;
                    th0_count=th0_count+1;
                } 
                else
                {
                    th0_count=0;	    
                    th0 = (th0_cur*1+th0_pre*3)>>2 ;
                }
                if(th0_count>5)
                {
                    th0=th0_cur;
                    th0_count =0 ;      
                }
                
                if(fg_apl_sum>30000||fg_apl_sum<300)   // fg cnt too small or too large disable the pfv corr 
                {
                    //  th0=25 ;
                    //  path2 = 4 ;
                }

                th0_pre =th0 ;
             //   rtd_pr_memc_info("avr2 (%d %d  %d)\n",th0_pre ,th0 , th0_count );
                WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,th0); 
                WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,th0); 
                WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,th0); 
                if(fg_apl_sum>17800&&(th0<25))   //32400*0.55
                {
                    WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,th0+10); 
                    WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,th0+10); 
                    WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,th0+10);    
                  //  path2 = 0xa ;
                }
                
                if(th0>25)
                {
                    WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,th0+5); 
                }
                else
                {
                    WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29, 25); 
                }
				// for slow hortical   but	vertical moving  video 
				
				if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_165_ver_th ) 
				{ 
				  
					 vertical_slow_165_flag = 1 ;
					 vertical_slow_165_holdframe = 60 ; 
				
					 WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,0x19); 
					 WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,0x19); 
					 WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,0x19); 
					 WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,0x19); 
					 path2 =3 ;
				}
				else if(vertical_slow_165_holdframe>0)				 
				{
					 vertical_slow_165_holdframe-- ;
				
					 WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,0x19); 
					 WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,0x19); 
					 WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,0x19); 
					 WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,0x19); 
					 path2 =3 ;	
				}
				else if(vertical_slow_165_flag)
				 {
					 vertical_slow_165_flag=0 ;
					 
					 WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,th0); 
					 WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,th0); 
					 WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,th0); 
					 WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,th0+5); 
					 path2 =0 ;
				}
				//+++++ver hor all	moving +++++++++++//
				
				if(pOutput->u1_me1_bg_ver_hor_candsel )  //???????? 315??	can bot remember which pattern 
				{
					 WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,th0+8); 
					 WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,th0+8); 
					 WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,th0+8); 
					 WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,th0+10);
					 
				}
				
				//for 298 more than one fg case	 face  broken issue   ++++++++298++++++
                { 
                for(rgnIdx =0 ; rgnIdx< 32; rgnIdx++)
                {
                ReadRegister(KME_ME1_BG0_ME_BG_INFO_RGN00_BG_reg+4*rgnIdx ,0,9,&u32_RB_val);   
                rgn_bgcnt[rgnIdx] = u32_RB_val ;
                }
                 fgleftcnt = 8100- rgn_bgcnt[6]-rgn_bgcnt[7]- rgn_bgcnt[14]-rgn_bgcnt[15]- rgn_bgcnt[22]-rgn_bgcnt[23]- rgn_bgcnt[30]-rgn_bgcnt[31] ;
                  bgrightcnt =  rgn_bgcnt[0]+rgn_bgcnt[1]+ rgn_bgcnt[8]+rgn_bgcnt[9]+ rgn_bgcnt[16]+rgn_bgcnt[17]+ rgn_bgcnt[24]+rgn_bgcnt[25] ;

                    ReadRegister(HARDWARE_HARDWARE_25_reg, 2, 2 , &fwcontrol_298); 
                   if(fwcontrol_298)  //&&u11_gmv_mvx>-100&&fgleftcnt>600
                   {
                        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,0x1e); 
                        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,0x1e); 
                        WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,0x1e); 
                        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,0x1e); 
                        badfgholdframe = 15 ;
                        badfgframe = 1 ;    
                        path2 =4 ;
                     //   rtd_pr_memc_info("298 pattern \n"); 
                    }
                    else if(badfgholdframe>0)
                    { 
                        if(th0>35)
                        {
                            WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,0x1e); 
                            WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,0x1e); 
                            WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,0x1e); 
                            WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,0x1e); 
                        }
                        badfgholdframe --;
                        path2 =4 ;                  
                    }
                    else if(badfgframe)
                    {
                        badfgframe =0 ;
                        path2 =0 ;
                        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,th0); 
                        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,th0); 
                        WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,th0); 
                        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,th0+5); 
                    }
                                         if(fwcontrol_298&&fgleftcnt<10&&bgrightcnt<300)
                    {
                        video298 =1 ;
                        holdframe298 =  s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ ? 240 :120 ;
                        path2 =5 ;
                        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,0x45); 
                        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,0x45); 
                        WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,0x45); 
                        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,0x45); 

                    }
                    else if(holdframe298>0)
                    {
                        holdframe298--;
                        path2 =5 ;
                        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,0x45); 
                        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,0x45); 
                        WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,0x45); 
                        WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,0x45); 

                    }
                    else if(video298)
                    {
                        video298 = 0 ;
                    }                        
                }
                //for 298 more than one fg case   end 



			  //+++++++++++++++++163 dog  vertical moving fast	video	 GMV_SEL=1	has issue and  only clear bg and no occl ++++++++++++++//
				   if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_virtical_case3==1)
				   {
		   
					   holdframe163 =	 s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ ? 240 :120 ;
		   
					   video163=1;
					   path2 = 5 ; 
					   //  WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);	 // gmv candidtae choose gmv  not  rmv 
					   // WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0);
					   //	WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,0x40);
					   //  WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,0x40);
					   //	WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,0x40);
					   //	WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,0x64);
					   //	  WriteRegister(KME_ME1_BG0_ME1_BG_18_reg,31,31,0x1);  //BG force_clear_en
		   
					   //WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,22,22,0x1);
				   }
				   else if(holdframe163>0)
				   {
						path2 = 5 ;
						holdframe163 -- ; 
						   
					   //	WriteRegister(KME_ME1_BG0_ME1_BG_18_reg,31,31,0x1);  //BG force_clear_en
					   //WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,22,22,0x1);
				   }
				   else if(video163)
				   {
					   video163 = 0 ;
					   path2 = 0x0 ;
					   //  WriteRegister(KME_ME1_BG0_ME1_BG_18_reg,31,31,0x0);	//BG force_clear_en
					   //WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,22,22,0x0);
				   } 
				  //++++++++++++++++129 vertical normal mv	++++++++++++++//
				   if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_virtical_case4==1)
				   {
					   holdframe129=100 ;  
					   video129=1;
					 //  path2 = 6 ; ;
		   
					   }
				   else if(holdframe129>0)
				   {
					  //  path2 = 7 ;
						holdframe129 -- ; 
					 
					//	 WriteRegister(KME_ME1_BG0_ME1_BG_18_reg,31,31,0x1);  //BG force_clear_en
				   }
				   else if(video129)
				   {
					   video129 = 0 ;
					 //  path2 = 0 ;
		   
				   } 
		   
				   
		   //////////////////////////------------------------156----------------------///////////////
		   
					if((Dtl_array[12]<80000 && Dtl_array[12]>40000)&&(Dtl_array[20]<100000 && Dtl_array[20]>50000)&&(Dtl_array[21]<120000 && Dtl_array[21]>70000)&&
		           (Dtl_array[29]<80000 && Dtl_array[29]>30000) &&(Dtl_array[28]<75000 && Dtl_array[28]>40000) &&(Dtl_array[7]<55000 && Dtl_array[7]>5000) &&(Dtl_array[4]<65000 && Dtl_array[4]>15000)&& Dtl_array[24]<65000 && (rgn_aplp[12]>70000 && rgn_aplp[12]<135000) && (rgn_aplp[20]<120000 &&rgn_aplp[20]>40000)
		           && (rgn_aplp[24]<15000)&&(rgn_aplp[0]<50000 && rgn_aplp[0]>20000)&&(rgn_aplp[8]<80000 &&rgn_aplp[8]>20000)&& _ABS_(rmv_mvx[15])<100)
		           {       
		           
		               holdframe_156_th = 80;
		               WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,0x9);
		               WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,0x9);
		               WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,0x9);
		               WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,0xe);
		               video_1561=1;
		           }
		           else if(holdframe_156_th>0)
		           {
		               holdframe_156_th--;
		               WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,0x9);
		               WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,0x9);
		               WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,0x9);
		               WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,0xe);
		               video_1561=1;
		           }
				   else if(video_1561)
				   {
					   WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,th0); 
					   WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,th0); 
					   WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,th0); 
					   WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,th0+5);
					   video_1561=0;
				   }


            }
       /*     else
            {
                WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,bg_s1_mvdiff_bv_lower_th_init);
                WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,bg_s2_mvdiff_ppi_lower_th_init);
                WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,bg_neargmv_diff_th_init);
                WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,bg_pfv_rep_mvdiff_th_init);  
                WriteRegister(KME_ME1_BG0_ME1_BG_4_reg,9,15,fg_pfv_rep_mvdiff_th_init);

                WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,bg_s3_ucov_data_en_init);  
                WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,bg_s3_ucov_data_en_init); 

                WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0x0);
                WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0xef);
                WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0x0);
                WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0x86);

            } 

            path2 = 0xf ;
            if(fb_lvl_sum>4000)
            {
                WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0x0);
                WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0x0);
                WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0x0);
                WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0x0);
                path2 =0xf; 
            }
            else if(mvdiff_bgfg<65)
            {
                WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0);
                WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0);
                WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0);
                WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0);
                path2 =2 ; 
            }
            else
            {
                WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0x0);
                WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0xef);
                WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0x0);
                WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0x86);
                path2 =3 ; 
                    
            }
        */

            //***** set MV diff th end *****//       

            //***** frames need close the hold and fg mask curve after scene change *****//								

            if(gmv_bg_cnt<27000)   //for 13m2v small fg   global fg cnt need set small  only  affect global fg mv value
            {
                  WriteRegister(KME_ME1_BG1_ME_BG_TH2_reg,0,9,0xff);   //gmv_fg_cnt_th   
            }
            else 
            {
                  WriteRegister(KME_ME1_BG1_ME_BG_TH2_reg,0,9,0x50);   //gmv_fg_cnt_th   
            }          
        

			// rtd_pr_memc_info("bgcnt =(%d %d)\n" ,gmv_bg_cnt_pre ,	  gmv_bg_cnt  ) ; 
			

            ReadRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15, &x_range_old);
            ReadRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,&y_range_old); 
            pfv_corr_dis_cond1 = (_ABS_DIFF_(gmv_bg_cnt_pre ,gmv_bg_cnt ) > 4000) ;  //bg cnt change 

			me1_bg_pfv_corr_cond = pOutput->u1_me1_bg_pfv_corr_cond ; 
            if((SCFlag==1) ||pfv_corr_dis_cond1)
			{
			   // frame_cnt = 0; 
				framenum=0;
				BSC=1;
				WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,5,5,fg_hold_en_init); 
				WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,0,4,fg_hold_th);  
				WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,27,27,fg_mask_en_init); 
				pOutput->u1_me1_bg_pfv_corr_cond = 0 ;
                //path2+=1;
                frame_cnt = 32 ;
                //  path2 =1 ;
                
			}
			else  if(frame_cnt>0)  //((u11_gmv_mvx!=0)||(u10_gmv_mvy!=0))&&
			{
				frame_cnt--;
				WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,5,5,fg_hold_en_init);  
				WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,27,27,fg_mask_en_init); 

				pOutput->u1_me1_bg_pfv_corr_cond = 0 ;
               //  path2 =2;
			} 
			else if(BSC)
			{
				BSC=0;
				WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,5,5,fg_hold_en); 
				WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,0,4,fg_hold_th_init); 
				WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,27,27, 0x1 ); //fg mask enable
              //  path2 =3;
                pOutput->u1_me1_bg_pfv_corr_cond  = me1_bg_pfv_corr_cond ;
			}
			framenum++ ;
			
		/*	  if(gmv_bg_cnt<16200)
			{
				framenum=frame_cnt;
				BSC=1;
				WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,5,5,fg_hold_en); 
				WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,0,4,fg_hold_th); 
				path+=0x20;
			}
		*/
                  
			//**************fast and large fg case 144**********************//
            if(pParam->u1_me1_condition3)  // b809d560 [11]  
            {
                if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_large_fg_cnt==1)
                {
                    WriteRegister(KME_ME1_BG0_ME1_BG_7_reg,8,14,bg_s3_mode0_cov_yth2);
                    WriteRegister(KME_ME1_BG0_ME1_BG_7_reg,4,7,bg_s3_mode0_cov_shift);
                    WriteRegister(KME_ME1_BG0_ME1_BG_7_reg,0,3,bg_s3_mode0_cov_slope);
                    WriteRegister(KME_ME1_BG0_ME1_BG_8_reg,8,14,bg_s3_mode0_ucov_yth2);
                    WriteRegister(KME_ME1_BG0_ME1_BG_8_reg,0,3,bg_s3_mode0_ucov_shift);
                    WriteRegister(KME_ME1_BG0_ME1_BG_8_reg,4,7,bg_s3_mode0_ucov_slope);
                    WriteRegister(KME_ME1_BG0_ME1_BG_10_reg,1,1,bg_s3_mode0_cond_en);
                    WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,15,21,bg_s3_dubt_cov_num_yth1);
                    WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,8,14,bg_s3_dubt_cov_num_yth2);
                    WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,4,7,bg_s3_dubt_cov_shift);
                    WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,0,3,bg_s3_dubt_cov_slope);
                    WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,15,21,bg_s3_dubt_ucov_yth1);
                    WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,8,14,bg_s3_dubt_ucov_yth2);
                    WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,4,7,bg_s3_dubt_ucov_shift);
                    WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,0,3,bg_s3_dubt_ucov_slope);
                    WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,22,31,bg_s3_cov_largesad_th);
                    WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,12,21,bg_s3_cov_smallsad_th);
                    //  WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,2,11,bg_s3_cov_setbg_th);   
                    WriteRegister(KME_ME1_BG1_ME_BG_S3_0_reg,20,29,bg_s3_ucov_smallsad_th);
                    WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,5,18,fg_apl_protect_th1);
                    WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,fg_apl_protect_bgnum_th);
                    WriteRegister(KME_ME1_BG0_ME1_BG_18_reg,20,29,rmv_fg_mvd_merge_thd);         
                    
                    //   WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,0,4,0x14);   //fg_hold_th
                    //  WriteRegister(KME_ME1_BG0_ME1_BG_18_reg,20,29,0x32);  //rmv_fg_mvd_merge_thd
                    WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,12,21,0x12C);//bg_s3_cov_smallsad_th
                    WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,2,11,0x190);  //bg_s3_cov_setbg_sad_th
                    WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_1_reg,31,31,0x1);   //color range2 en
                    WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23, 0x14);   //fg_apl_s2_protect_th1	     
                    WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15, 0x14);   //fg_apl_s3_protect_th1
					//	WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_7_reg,16,21,0x4); //colorprotect_cnt_th2  set to max value	for hw bug 
					  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_7_reg,10,15,0x4);//colorprotect_cnt_th1  set to max value	 for hw bug 
                  
                    //  path2+=0x2;
                    
					//  rtd_pr_memc_info("144 video cond\n");
                    largeFastFrameHold  = 12 ;
                    blargefast = 1 ;

					WriteRegister(KME_ME1_BG1_ME_BG_S3_0_reg,30,30,0x1);
                    WriteRegister(KME_ME1_BG1_ME_BG_TH1_reg,24,31,0xff);  // apl set2 large th
                    WriteRegister(KME_ME1_BG1_ME_BG_TH1_reg,16,23,0xf0);   // apl set2 low th

                    //   path2 =0x7;

                }
                else if(largeFastFrameHold>0)
                {
                    largeFastFrameHold -- ;
                }
                else if(blargefast)
                {
                    blargefast =0 ;
                    WriteRegister(KME_ME1_BG0_ME1_BG_7_reg,8,14,bg_s3_mode0_cov_yth2_init);
                    WriteRegister(KME_ME1_BG0_ME1_BG_7_reg,4,7,bg_s3_mode0_cov_shift_init);
                    WriteRegister(KME_ME1_BG0_ME1_BG_7_reg,0,3,bg_s3_mode0_cov_slope_init);
                    WriteRegister(KME_ME1_BG0_ME1_BG_8_reg,8,14,bg_s3_mode0_ucov_yth2_init);
                    WriteRegister(KME_ME1_BG0_ME1_BG_8_reg,0,3,bg_s3_mode0_ucov_shift_init);
                    WriteRegister(KME_ME1_BG0_ME1_BG_8_reg,4,7,bg_s3_mode0_ucov_slope_init);
                    WriteRegister(KME_ME1_BG0_ME1_BG_10_reg,1,1,bg_s3_mode0_cond_en_init);
                    WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,8,14,bg_s3_dubt_cov_num_yth2_init);
                    WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,15,21,bg_s3_dubt_cov_num_yth1_init);
                    WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,4,7,bg_s3_dubt_cov_shift_init);
                    WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,0,3,bg_s3_dubt_cov_slope_init);
                    WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,15,21,bg_s3_dubt_ucov_yth1_init);
                    WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,8,14,bg_s3_dubt_ucov_yth2_init);
                    WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,4,7,bg_s3_dubt_ucov_shift_init);
                    WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,0,3,bg_s3_dubt_ucov_slope_init);
                    WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,22,31,bg_s3_cov_largesad_th_init);
                    WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,12,21,bg_s3_cov_smallsad_th_init);
                    WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,2,11,bg_s3_cov_setbg_th_init);   
                    WriteRegister(KME_ME1_BG1_ME_BG_S3_0_reg,20,29,bg_s3_ucov_smallsad_th_init);
                    WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,5,18,fg_apl_protect_th1_init);
                    WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,fg_apl_protect_bgnum_th_init);
                    WriteRegister(KME_ME1_BG0_ME1_BG_18_reg,20,29,rmv_fg_mvd_merge_thd_init); 
					WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_1_reg,31,31,0x0);   //color range2 en
                    WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,fg_apl_s2_protect_th1_init); 	     
                    WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,fg_apl_s3_protect_th1_init);   
					WriteRegister(KME_ME1_BG1_ME_BG_S3_0_reg,30,30,0x0);
                    WriteRegister(KME_ME1_BG1_ME_BG_TH1_reg,24,31,0x0);  // apl set2 large th
                    WriteRegister(KME_ME1_BG1_ME_BG_TH1_reg,16,23,0x0);   // apl set2 low th
                  //  WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_7_reg,16,21,0x39); //colorprotect_cnt_th2  set to max value   for hw bug 

                    WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_7_reg,10,15, 63);//colorprotect_cnt_th1  set to max value   for hw bug 
                   }
            }
       /*     else
            {
                WriteRegister(KME_ME1_BG0_ME1_BG_7_reg,8,14,bg_s3_mode0_cov_yth2_init);
                WriteRegister(KME_ME1_BG0_ME1_BG_7_reg,4,7,bg_s3_mode0_cov_shift_init);
                WriteRegister(KME_ME1_BG0_ME1_BG_7_reg,0,3,bg_s3_mode0_cov_slope_init);
                WriteRegister(KME_ME1_BG0_ME1_BG_8_reg,8,14,bg_s3_mode0_ucov_yth2_init);
                WriteRegister(KME_ME1_BG0_ME1_BG_8_reg,0,3,bg_s3_mode0_ucov_shift_init);
                WriteRegister(KME_ME1_BG0_ME1_BG_8_reg,4,7,bg_s3_mode0_ucov_slope_init);
                WriteRegister(KME_ME1_BG0_ME1_BG_10_reg,1,1,bg_s3_mode0_cond_en_init);
                WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,8,14,bg_s3_dubt_cov_num_yth2_init);
                WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,15,21,bg_s3_dubt_cov_num_yth1_init);
                WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,4,7,bg_s3_dubt_cov_shift_init);
                WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,0,3,bg_s3_dubt_cov_slope_init);
                WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,15,21,bg_s3_dubt_ucov_yth1_init);
                WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,8,14,bg_s3_dubt_ucov_yth2_init);
                WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,4,7,bg_s3_dubt_ucov_shift_init);
                WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,0,3,bg_s3_dubt_ucov_slope_init);
                WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,22,31,bg_s3_cov_largesad_th_init);
                WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,12,21,bg_s3_cov_smallsad_th_init);
                WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,2,11,bg_s3_cov_setbg_th_init);   
                WriteRegister(KME_ME1_BG1_ME_BG_S3_0_reg,20,29,bg_s3_ucov_smallsad_th_init);
                WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,5,18,fg_apl_protect_th1_init);
                WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,fg_apl_protect_bgnum_th_init);
                WriteRegister(KME_ME1_BG0_ME1_BG_18_reg,20,29,rmv_fg_mvd_merge_thd_init); 
				WriteRegister(IPPRE1_IPPRE1_COLOR_PROT_3_reg,31,31,0x0);   //color range2 en
    
                WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,fg_apl_s2_protect_th1_init); 	     
                WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,fg_apl_s3_protect_th1_init);
            }*/

            if(pParam->u1_me1_condition4)
            {
                if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_small_fg_cnt==1)
                {
                    WriteRegister(KME_ME1_BG1_ME_BG_S3_0_reg,20,29,bg_s3_ucov_smallsad_th);	    
                    WriteRegister(KME_ME1_BG1_ME_BG_S3_0_reg,10,19,bg_s3_ucov_largesad_th); 
                    WriteRegister(KME_ME1_BG1_ME_BG_S3_0_reg,0,9,bg_s3_ucov_setbg_sad_th);  
                    WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,0,3,bg_s3_dubt_ucov_slope);	    
                    WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,15,21,bg_s3_dubt_ucov_yth1);
                    WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,8,14,bg_s3_dubt_ucov_yth2);	    
                    WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,0,3,bg_s3_dubt_cov_slope);   
                    WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,15,21,bg_s3_dubt_cov_num_yth1);
				//	path+=0x40;
                }
                else
                {
                    WriteRegister(KME_ME1_BG1_ME_BG_S3_0_reg,20,29,bg_s3_ucov_smallsad_th_init);  
                    WriteRegister(KME_ME1_BG1_ME_BG_S3_0_reg,10,19,bg_s3_ucov_largesad_th_init);        
                    WriteRegister(KME_ME1_BG1_ME_BG_S3_0_reg,0,9,bg_s3_ucov_setbg_sad_th_init);     
                    WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,0,3,bg_s3_dubt_ucov_slope_init);   
                    WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,15,21,bg_s3_dubt_ucov_yth1_init);
                    WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,8,14,bg_s3_dubt_ucov_yth2_init);   
                    WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,0,3,bg_s3_dubt_cov_slope_init);   
                    WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,15,21,bg_s3_dubt_cov_num_yth1_init);  
                }
            }
  

            if(pParam->u1_me1_condition5)   // 299 high apl set 
            {
                /*if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_s2_apl_protect_en==1)
                {  	    
                    WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,fg_apl_s2_protect_th0_317);	     
                    WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,fg_apl_s2_protect_th1_317);	      
                    WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,fg_apl_s2_protect_bgnum_th_317); 
                  //  path2+=0x20;
                   rtd_pr_memc_info("high apl protect 1\n");
                }
                else */
                if(s_pContext->_output_frc_sceneAnalysis.u8_me1_conditions2_s3_apl_protect_en==1)
                {
                    WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,fg_apl_s2_protect_th0_299);	    	    
                    WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,fg_apl_s2_protect_th1_299); 	     
                    WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,fg_apl_s2_protect_bgnum_th_299);

                    WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,fg_apl_s3_protect_th0);   
                    WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,fg_apl_s3_protect_th1);   
                    WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,fg_apl_s3_protect_bgnum_th);
                  //  path2+=0x4;
                  //  rtd_pr_memc_info("high apl protect 2\n");
                    highaplFrameHold =10  ; 
                    bhighapl =1   ;  
                    path2 = 6 ; 
                    
                }
                else if(highaplFrameHold>0)
                {
                    highaplFrameHold -- ;
					path2 = 6 ;
                }
                else if(bhighapl&&(!blargefast))
                {
                    bhighapl =0 ;
                    WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,fg_apl_s2_protect_th0_init);	    	    
                    WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,fg_apl_s2_protect_th1_init); 	     
                    WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,fg_apl_s2_protect_bgnum_th_init);

                    WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,fg_apl_s3_protect_th0_init);   
                    WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,fg_apl_s3_protect_th1_init);   
                    WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,fg_apl_s3_protect_bgnum_th_init);    
                    path2 = 0 ;
                }

            }
         /*   else
            {
                WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,fg_apl_s2_protect_th0_init);	    	    
                WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,fg_apl_s2_protect_th1_init); 	     
                WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,fg_apl_s2_protect_bgnum_th_init);

                WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,fg_apl_s3_protect_th0_init);   
                WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,fg_apl_s3_protect_th1_init);   
                WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,fg_apl_s3_protect_bgnum_th_init); 
            }*/

            if(pParam->u1_me1_condition6)
            {
                if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_virtical==1)
                {
                    WriteRegister(KME_ME1_BG0_ME1_BG_4_reg,24,24,fg_mask_pfv_blend_dis);   
                  //  path2+=0x8;
                }
				else if(_ABS_(gmv_fgy)>5|| _ABS_( avr_region_fg_y)>5)
                {
                    WriteRegister(KME_ME1_BG0_ME1_BG_4_reg,24,24,fg_mask_pfv_blend_dis);   
                }
                else
                {
                    WriteRegister(KME_ME1_BG0_ME1_BG_4_reg,24,24,fg_mask_pfv_blend_en_init);   
                } 

				if(_ABS_(gmv_bgy)<3&&_ABS_(u10_gmv_mvy)<3)
                {
                    WriteRegister(KME_ME1_BG0_ME1_BG_4_reg,16,16,0x1);  //bg_mask_pfv_blend_en
                }
                else
                {
                    WriteRegister(KME_ME1_BG0_ME1_BG_4_reg,16,16,0x0);  //bg_mask_pfv_blend_en
                }
            }
            /*else
            {
                WriteRegister(KME_ME1_BG0_ME1_BG_4_reg,24,24,fg_mask_pfv_blend_en_init);   
            }*/
            

			if(pParam->u1_me1_condition6)
			{

			    //++++++SC  15 frame  s2 apl protect off  
                if(SCFlag==1)
                {

                      sc_s2_apl_prot_off  = 1 ;
                      sc_s2_apl_prot_off_holdframe = 20 ;
                      WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,0,0,0x0); //s2_protect_en
                }
                else  if(sc_s2_apl_prot_off_holdframe>0)  //((u11_gmv_mvx!=0)||(u10_gmv_mvy!=0))&&
                {
                    sc_s2_apl_prot_off_holdframe-- ;
                    WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,0,0,0x0); //s2_protect_en

                } 
                else if(sc_s2_apl_prot_off)
                {
                    sc_s2_apl_prot_off = 0 ;
                    WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,0,0,0x1); //s2_protect_en
                }
                
               //++++171  black protect off++++++++//   
                if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_black_off_case1==1||s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_black_off_case2==1)
                {

                    holdframe171= 110;  
                    video171=1;
                 
                    //WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,0,0,0x0); //s3_protect_en
                    WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,0,0,0x0); //s2_protect_en

                    path2 = 6 ;
                }
                else if(holdframe171>0)
                {

                    //WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,0,0,0x0); //s3_protect_en
                    WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,0,0,0x0); //s2_protect_en
                     path2 =6 ;
                     holdframe171 -- ; 
                }
                else if(video171)
                {
                    video171 = 0 ;

                   //WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,0,0,0x0); //s3_protect_en
                    WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,0,0,0x1); //s2_protect_en
                     path2 = 0 ;
                } 
				 
			/*	 else
			{
				WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,fg_apl_s2_protect_th0_init);				
				WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,fg_apl_s2_protect_th1_init);		 
				WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,fg_apl_s2_protect_bgnum_th_init);

				WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,fg_apl_s3_protect_th0_init);	 
				WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,fg_apl_s3_protect_th1_init);	
				WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,fg_apl_s3_protect_bgnum_th_init);	
				WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x22);	  
				WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,0,7,0x0);
			}*/
			

			}

			if (_ABS_DIFF_(gmv_bg_cnt_pre ,gmv_bg_cnt ) > 2500) 
            {
                corr_dis_holdframe = 4;

                pOutput->u1_me1_bg_pfv_corr_cond =0 ;
                path2 =7 ;

            }
            else if(corr_dis_holdframe>0)
            {
                corr_dis_holdframe-- ;
                pOutput->u1_me1_bg_pfv_corr_cond =0 ;           
               // path2 =7 ;
            }
			
            if(pParam->u1_me1_condition7)
            {
                  //**********pfv corr enable and  sad bg enable start ********//
                 if(pOutput->u1_me1_bg_pfv_corr_cond)
            {
                    WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0x0);   //full range 
                    WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0xef);
                    WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0x0);
                    WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0x86);
                  //  path2 =1 ; 
            }
                else
            {
            WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0);
            WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0);
            WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0);
            WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0);
                  //  path2 =0 ; 
            }
                if(pOutput->u1_me1_sad_bg_cond)
            {
                    WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,bg_s3_ucov_data_en_init);
                    WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,bg_s3_cov_data_en_init);
                  //  path2 =3 ; 
            }
            else
            {
                    WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30, bg_s3_ucov_data_dis); 
                    WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31, bg_s3_cov_data_dis);
                    //path2 =4 ; 
            }
                //**********pfv corr enable and  sad bg enable end********//
            }
			


			if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_172_black_th_flag&&(!s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_128_flag))
            {
                video172 =1 ;
                holdframe172 = s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ ? 220 :110 ;

                WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,fg_apl_s2_protect_th0_init);	    	    
                WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,60); 	     
                WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,fg_apl_s2_protect_bgnum_th_init);

                WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,fg_apl_s3_protect_th0_init);   
                WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,60);   
                WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,fg_apl_s3_protect_bgnum_th_init);    
                WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30, 0); 
                WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31, 0);

                WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0);
                WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0);
                WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0);
                WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0); 
                WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x18);//bg_s2_num_yth2
                WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0x12); //bg_s2_num_yth1     
                WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x14);  //bg_s1_num_yth2
                WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0xa);  //bg_s1_num_yth1      
                
                path2=0x8;
            }
            else if(holdframe172>0&&_ABS_(u11_gmv_mvx)<70)
            {
                holdframe172--;
                WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,fg_apl_s2_protect_th0_init);	    	    
                WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,60); 	     
                WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,fg_apl_s2_protect_bgnum_th_init);

                WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,fg_apl_s3_protect_th0_init);   
                WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,60);   
                WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,fg_apl_s3_protect_bgnum_th_init);    
                WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30, 0); 
                WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31, 0);

                WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30, 0); 
                WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31, 0);
                WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0);
                WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0);
                WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0);
                WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0);
                WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x16);//bg_s2_num_yth2
                WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0x10); //bg_s2_num_yth1    
                WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x12);  //bg_s1_num_yth2
                WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0x8);  //bg_s1_num_yth1
                path2=0x8;
            }
            else if(video172)
            {
                video172 =0 ;
                WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,fg_apl_s2_protect_th0_init);	    	    
                WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,fg_apl_s2_protect_th1_init); 	     
                WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,fg_apl_s2_protect_bgnum_th_init);

                WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,fg_apl_s3_protect_th0_init);   
                WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,fg_apl_s3_protect_th1_init);   
                WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,fg_apl_s3_protect_bgnum_th_init);    

                WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x14);//bg_s2_num_yth2
                WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0xa); //bg_s2_num_yth1       
                WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,0x10);  //bg_s1_num_yth2
                WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0x4);  //bg_s1_num_yth1
                path2=0x0;
                
                
            }


            
            //+++++++for special video setting code+++++++++++++++++++++++//
            // ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_2_reg,17 , 17 , &readval);
           // if(readval==0)
            {
                MEMC_me1_bgmask_special_setting(pParam  ,pOutput  ,th0 ,gmv_mvx_final)  ;
            }
            //----------for special video setting code------------------------//
			   //+++++bg adjust code useless need to delete++++++//
				 ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_1_reg,31 , 31 , &readval);
				if(readval==1)
				{
					ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_1_reg,0 , 10 , &readval);
					gmvx_test = ((readval >> 20) &1) == 0? ((readval>>10)& 0x7ff):(((readval>>10)& 0x7ff)  - (1<<11));
		
					WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 10, 20, gmvx_test);  //fw mvx 
		
					ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_1_reg, 16 , 25 , &readval);
					gmvy_test = ((readval >> 9) &1) == 0? (readval& 0x3ff):((readval& 0x3ff)  - (1<<10));
		
					WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 21, 30, gmvy_test);   //fw mvy
		
					ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_1_reg ,30, 30, &readval );  
					WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, readval);	//enable 
		
					ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_1_reg ,29, 29, &readval );  
				   if(readval)
				   {
						WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 1);
						WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, gmv_mvx_final);
						WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0);	
				   }
				}
		
       
				//+++++++ fw adjust test  code ++++++++//
				ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,17 , 17 , &readval);
				if(readval==1)
				{
					ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,18 , 18 , &readval);
					WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_1_reg, 28, 28, readval ); // new algo switch enable 
				}

				ReadRegister(SOFTWARE3_SOFTWARE3_20_reg,1, 1 , &readval);
				if(readval==1)
				{
					ReadRegister(SOFTWARE3_SOFTWARE3_20_reg,2 , 2, &readval);
					WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,0,0,readval); 
					ReadRegister(SOFTWARE3_SOFTWARE3_20_reg,3 , 3, &realval1);
					WriteRegister(KME_ME1_BG1_ME_BG_S4_1_reg,22,22,realval1); 	   
				}
				 ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,31 , 31 , &readval);
				if(readval==1)
				{
					ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,0 , 7 , &readval);	 
					
					WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,readval); 
					WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,readval); 
					WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,readval); 
					WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,readval+5); 
				}


				ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_2_reg,15 , 15 , &readval);
				if(readval==1)
				{
					ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_2_reg, 0 , 7, &readval);	 
					WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,readval); 
				}

				ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_2_reg,8 , 8 , &readval);
				if(readval==1)
				{
					ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_2_reg, 9 , 9, &readval);	 
					WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,5,5,readval); 
				}

				ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_2_reg,8 , 8 , &readval);
				if(readval==1)
				{
					ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_2_reg, 9 , 9, &readval);	 
					WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,5,5,readval); 
				}
				ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_2_reg,10 , 10 , &readval);
				if(readval==1)
				{
					WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,1); //bg_s1_num_yth2
					WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,0x1); //bg_s1_num_yth1
					WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,18,22,0xa);
					WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,23,27,0x2);
					WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,28,31,0x1);	//bg_s1_slope
					
					WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,0x1);  //bg_s2_num_yth2
					WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,0x1); //bg_s2_num_yth1
					WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,18,22,0xc);
					WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,23,27,0x2);
					WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,28,31,0x1); //bg_s2_slope
				}


				ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_2_reg,11 , 11 , &readval);
				if(readval==1)
				{

					WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,8,14,4); //bg_s3_dubt_cov_num_yth2_init
					WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,15,21,1); // bg_s3_dubt_cov_num_yth1_init 
					WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,4,7,3);	 //bg_s3_dubt_cov_shift_init
					WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,0,3,2); //bg_s3_dubt_cov_slope_init
					
					WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,15,21, 1); //bg_s3_dubt_ucov_yth1_init
					WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,8,14, 4); //bg_s3_dubt_ucov_yth2_init
					WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,4,7,  3); //bg_s3_dubt_ucov_shift_init
					WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,0,3,  2); // bg_s3_dubt_ucov_slope_init

				}

				ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_2_reg,12 , 12 , &readval);
				if(readval==1)
				{

			//	  WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,  th0);
				WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,5,5,0);  //fg_hold_en_init
				WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,0,4,25);   //fg_hold_th

				}
				 ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_2_reg,13 , 13 , &readval);
				if(readval==1)
				{

				  WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg,24,24,0x0);	//s4_apl_force_bg_en
				
				}

				ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_2_reg,16 ,16 , &readval);
				if(readval==1)
				{
					 WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0);			 
				}
				ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_2_reg,17 ,17 , &readval);
				if(readval==1)
				{
					WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,0,0, 1);   
				}
        }
        else  //no fw   the register value reset to init value      b809d560 [30] 
        {
            WriteRegister(KME_ME1_BG0_ME1_BG_7_reg,8,14,bg_s3_mode0_cov_yth2_init);
            WriteRegister(KME_ME1_BG0_ME1_BG_7_reg,4,7,bg_s3_mode0_cov_shift_init);
            WriteRegister(KME_ME1_BG0_ME1_BG_7_reg,0,3,bg_s3_mode0_cov_slope_init);
            WriteRegister(KME_ME1_BG0_ME1_BG_8_reg,8,14,bg_s3_mode0_ucov_yth2_init);
            WriteRegister(KME_ME1_BG0_ME1_BG_8_reg,0,3,bg_s3_mode0_ucov_shift_init);
            WriteRegister(KME_ME1_BG0_ME1_BG_8_reg,4,7,bg_s3_mode0_ucov_slope_init);
            WriteRegister(KME_ME1_BG0_ME1_BG_10_reg,1,1,bg_s3_mode0_cond_en_init);
            WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,8,14,bg_s3_dubt_cov_num_yth2_init);
            WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,15,21,bg_s3_dubt_cov_num_yth1_init);
            WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,4,7,bg_s3_dubt_cov_shift_init);
            WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,0,3,bg_s3_dubt_cov_slope_init);
            WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,15,21,bg_s3_dubt_ucov_yth1_init);
            WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,8,14,bg_s3_dubt_ucov_yth2_init);
            WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,4,7,bg_s3_dubt_ucov_shift_init);
            WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,0,3,bg_s3_dubt_ucov_slope_init);
            WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,22,31,bg_s3_cov_largesad_th_init);
            WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,12,21,bg_s3_cov_smallsad_th_init);
            WriteRegister(KME_ME1_BG0_ME1_BG_12_reg,2,11,bg_s3_cov_setbg_th_init);   
            WriteRegister(KME_ME1_BG1_ME_BG_S3_0_reg,20,29,bg_s3_ucov_smallsad_th_init);
            WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,5,18,fg_apl_protect_th1_init);
            WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,fg_apl_protect_bgnum_th_init);
            WriteRegister(KME_ME1_BG0_ME1_BG_18_reg,20,29,rmv_fg_mvd_merge_thd_init); 
            WriteRegister(KME_ME1_BG1_ME_BG_S3_0_reg,20,29,bg_s3_ucov_smallsad_th_init);  
            WriteRegister(KME_ME1_BG1_ME_BG_S3_0_reg,10,19,bg_s3_ucov_largesad_th_init);	    
            WriteRegister(KME_ME1_BG1_ME_BG_S3_0_reg,0,9,bg_s3_ucov_setbg_sad_th_init);     
            WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,0,3,bg_s3_dubt_ucov_slope_init);   
            WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,15,21,bg_s3_dubt_ucov_yth1_init);
            WriteRegister(KME_ME1_BG0_ME1_BG_6_reg,8,14,bg_s3_dubt_ucov_yth2_init);   
            WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,0,3,bg_s3_dubt_cov_slope_init);   
            WriteRegister(KME_ME1_BG0_ME1_BG_5_reg,15,21,bg_s3_dubt_cov_num_yth1_init);  
            WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,bg_s1_mvdiff_bv_lower_th_init);
            WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,bg_s2_mvdiff_ppi_lower_th_init);
            WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,bg_neargmv_diff_th_init);
            WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,bg_pfv_rep_mvdiff_th_init);  
            WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,fg_apl_s2_protect_th0_init);         
            WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,fg_apl_s2_protect_th1_init);         
            WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,fg_apl_s2_protect_bgnum_th_init);    
           	// WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,8,15,fg_apl_s2_protect_th0_init);	    	    
          	// WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,16,23,fg_apl_s2_protect_th1_init); 	     
          	// WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,1,7,fg_apl_s2_protect_bgnum_th_init);
          
            WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,fg_apl_s3_protect_th0_init);   
            WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,fg_apl_s3_protect_th1_init);   
            WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,1,7,fg_apl_s3_protect_bgnum_th_init); 
            WriteRegister(KME_ME1_BG0_ME1_BG_4_reg,24,24,fg_mask_pfv_blend_en_init);   
            WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,5,5,fg_hold_en);  //fg_hold_en
            WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,0,4,fg_hold_th_init);     //fg_hold_th
            
            //  WriteRegister(KME_ME1_BG0_ME1_BG_17_reg,31,31,pfv_corr_en_init);   //pfv_corr_en
            
            //conditon7
            WriteRegister(KME_ME1_BG0_ME1_BG_4_reg,9,15,fg_pfv_rep_mvdiff_th_init);  //fg_pfv_rep_mvdiff_th
                
            WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,bg_s3_ucov_data_en_init);  //bg_s3_ucov_data_en
            WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,bg_s3_cov_data_en_init);  //bg_s3_cov_data_en
               
            WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,0,7,0);
            WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0);
            WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,16,23,0);
            WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0);

        }
    }
}
VOID MEMC_dehalo_newalgo_bgmaskBase_WrtAction(const _PARAM_WRT_COM_REG *pParam)
{
	int enbale_readval;
	ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 11, 11, &enbale_readval);
	
	if((pParam->u1_dehalo_Newalgo_Proc_en_all)||(enbale_readval == 1))
    {
    	//	int unconf_val1,unconf_val2,u32_RB_val,rgnIdx,avr_sad,sum_sad;
		//  int SAD_rgn[32]={0};
	    const _PQLCONTEXT *s_pContext = GetPQLContext();
		static signed short gmv_fgx = 0;
	    static signed short gmv_fgy = 0;
		static signed short gmv_bgx = 0;
	    static signed short gmv_bgy = 0;
		int left_rim_pre;
		    signed short u11_gmv_mvx;
    	    signed short u10_gmv_mvy;
		
        //dehalo init
        int bg_useless_fix_bg_th_init=0x54;
        //int posdiff_judge_divide_en_init=0x0;
        //int posdiff_judge_penalty_en_init=0x0;
        int posdiff_judge_th1_init=0x384;
        int pred_en_init=0x2;
        int pred_th0_init=0x16;//0xf;
        int pred_th1_init=0x3;
        int occl_correct_en_init=0x1;
        int bg_extend_cov_th1_init=0x5;
        int bg_extend_ucov_th1_init=0x5;
        int pred_ver_en_init=0x0;
        int bg_extend_ucov_th0_init= 0xc;
        int bg_extend_cov_th0_init= 0xc;
        int pred_me1_diff_cut_en_init= 0x0;
        int gmv_th_y2_init=0x4b;
        //int bg_extend_cond3_en_init=0x1;
        int occl_filter_ucov_cut_init = 0x4;
        int occl_filter_cov_cut_init= 0x4;
        int bg_extend_bg_sum_max_th_init= 0x57;
        int color_protect_th0_en_init=0x1;
        int color_protect_th1_en_init=0x0;
        int color_protect_th1_min_init=0x96;
        int fg_protect_by_apl_en_init=0x0;    
		int bg_extend_cond4_en_init=0x1;
	    int useless_max_th_init=0x54;
	    int extend_posdiff_th_init = 0x12c;
	    int extend_flag_num_init = 0x57;
	    int extend_mvdiff_th_init = 0x0;
	    int clear_y0_init = 0x12;
	    int clear_y1_init = 0x23;
	    int clear_y2_init = 0x3c;
	    int clear_posdiff_th_init = 0x0;
		int pfv_bg_change_th_init = 0x8;
	    int ppfv_bg_change_th_init = 0x8;
	    int judge_max_th0_init = 0x50;
	    int judge_max_th1_init = 0x50;
	    int old_pred_en_init = 0x1;
	    int pred_old_diff_cut_init = 0x1;
	    int bad_pred_cut_init = 0x1;
	    int bad_old_cut_init = 0x1;
	    int bg_change_cut_init = 0x1;
	    int pred_bad_cut_init = 0x1;
		int gmv_th_y0_init = 0x15;
		int pred_y0_init = 0x14;
	    int extend4_posdiff_th_init = 0x2bc;
		
            
	    //dehalo change
	    int pred_en_0=0x0;
	    int bg_useless_fix_bg_th=0x57;
	    //int posdiff_judge_divide_en=0x1;
	    //int posdiff_judge_penalty_en=0x1;
	    //int pred_en=0x1;
	    //int pred_th0=0xb;
	    int pred_th1=0x9;
	    //int occl_correct_en=0x0;
	    int bg_extend_cov_th1=0xa;
	    int bg_extend_ucov_th0=0xa;
	    int bg_extend_ucov_th1=0xa;
	    int pred_ver_en=0x1; 
	    int gmv_th_y2=0x65;
	    //int bg_extend_cond3_en=0x0;
	    int posdiff_judge_th1=0x33e;
	    int occl_filter_ucov_cut= 0xC;
	    int occl_filter_cov_cut = 0xC;
	    //int bg_extend_ucov_th0_y0= 0x14;
	    int bg_extend_cov_th0= 0x14;
	    //int bg_extend_bg_sum_max_th = 0x54;
	    //int pred_me1_diff_cut_en= 0x1;  
	    int color_protect_th0_en=0x0;
	    int color_protect_th1_en=0x1;
	    int color_protect_th1_min=0x82; 
	    int fg_protect_by_apl_en=0x1;
		int bg_extend_cond4_en=0x0;
	    int useless_max_th=0x57;
	    int extend_posdiff_th = 0x3ff;
	    int extend_flag_num = 0x64;
	    int extend_mvdiff_th = 0x14;
	    int clear_y0 = 0x5;
	    int clear_y1 = 0x19;
	    int clear_y2 = 0x32;
	    int clear_posdiff_th = 0x3f;
		int pfv_bg_change_th = 0x6;
	    int ppfv_bg_change_th = 0x6;
	    int judge_max_th0 = 0x96;
	    int judge_max_th1 = 0x96;
	    int old_pred_en = 0x0;
	    int pred_old_diff_cut = 0x0;
	    int bad_pred_cut = 0x0;
	    int bad_old_cut = 0x0;
	    int bg_change_cut = 0x0;
	    int pred_bad_cut = 0x0;
		int gmv_th_y0 = 0x0;
		int extend4_posdiff_th = 0x12c;
	    int pred_en_pre;
		int auto_extend_mvdiff_th = 0x14;
		int auto_extend_mvdiff_thfor128 = 0x14;
        int occl_more_1, occl_more_2, occl_more_3, occl_more_flag1=0, occl_more_flag2=0, occl_more_flag3=0, occl_more_hold1=0,  occl_more_hold2=0, occl_more_hold3=0;
		
        static int FrameHold;
        static int FrameHoldflag;
        static int  FrameHold_13, FrameHold_299, FrameHold_189, FrameHold_339,FrameHold_299_begin ,FrameHold_214,FrameHold_185, FrameHold_306, FrameHold_192, FrameHold_301, FrameHold_226,FrameHold_153, FrameHold_317_broken, FrameHold_172_broken, FrameHold_200_broken, FrameHold_432_broken, FrameHold_43_broken, FrameHold_128, FrameHold_366, FrameHold_317_broken2, FrameHold_317_broken3, FrameHold_173_begin, FrameHold_96_begin, FrameHold_96_end, FrameHold_130, FrameHold_227_2, FrameHold_227_4, FrameHold_227_5, FrameHold_156, FrameHold_156_final, FrameHold_431, FrameHold_49, FrameHold_163, FrameHold_183, FrameHold_432_final;
        static int  FrameHoldflag_13, FrameHoldflag_299, FrameHoldflag_189, FrameHoldflag_339,FrameHoldflag_299_begin,FrameHoldflag_214, FrameHoldflag_185, FrameHoldflag_306, FrameHoldflag_192, FrameHoldflag_301, FrameHoldflag_226,FrameHoldflag_153,FrameHoldflag_317_broken,FrameHoldflag_172_broken,FrameHoldflag_200_broken,FrameHoldflag_432_broken,FrameHoldflag_43_broken,FrameHoldflag_128, FrameHoldflag_366, FrameHoldflag_317_broken2, FrameHoldflag_317_broken3, FrameHoldflag_173_begin, FrameHoldflag_96_begin, FrameHoldflag_96_end, FrameHoldflag_130, FrameHoldflag_227_2, FrameHoldflag_227_4, FrameHoldflag_227_5, FrameHoldflag_156, FrameHoldflag_156_final, FrameHoldflag_431, FrameHoldflag_49, FrameHoldflag_163, FrameHoldflag_183, FrameHoldflag_432_final;
        int fwcontrol0 , fwcontrol4, fwcontrol_13, fwcontrol_299, fwcontrol_189, fwcontrol_339, fwcontrol_299_begin, fwcontrol_214, fwcontrol_185, fwcontrol_306, fwcontrol_192, fwcontrol_301,fwcontrol_153,fwcontrol_128,fwcontrol_156=0;
        static int me_blackoff_flag , me_blackoff_frame_hold ,  color_pro_299_flag, color_pro_299_hold;
        static int  FrameHold_151_broken, FrameHold_fast_hor, FrameHold_419, FrameHold_PQ_Issue ;          
        static int  FrameHoldflag_151_broken, FrameHoldflag_fast_hor, FrameHoldflag_419, FrameHoldflag_PQ_Issue,FrameHoldflag_equallbgcnt; 
		static bool check_biglogo=0;
		static int video_eagle,video_plane,video_smallplane,video_transporter,video_dance,video_skating,video30006,video_rotterdam,video_ridebike,video_30003man,vdieo_toleftbike,video_insect,video_huapi2,video_transporter_50hz;
		static int video_IDT;
		
		gmv_fgx=s_pContext->_output_read_comreg.dh_gmv_fgx;
        gmv_fgy=s_pContext->_output_read_comreg.dh_gmv_fgy;
        gmv_bgx=s_pContext->_output_read_comreg.dh_gmv_bgx;
        gmv_bgy=s_pContext->_output_read_comreg.dh_gmv_bgy;   

	    u11_gmv_mvx = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
	    u10_gmv_mvy = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;
        ReadRegister(HARDWARE_HARDWARE_25_reg, 0, 0 , &fwcontrol0);
        ReadRegister(HARDWARE_HARDWARE_25_reg, 4,4 , &fwcontrol4);  
        ReadRegister(HARDWARE_HARDWARE_25_reg, 10,10 , &fwcontrol_185);  
        ReadRegister(HARDWARE_HARDWARE_25_reg, 11,11 , &fwcontrol_13);  
        ReadRegister(HARDWARE_HARDWARE_25_reg, 6,6 , &fwcontrol_214);
        ReadRegister(HARDWARE_HARDWARE_25_reg, 8,8 , &fwcontrol_299); 
        ReadRegister(HARDWARE_HARDWARE_25_reg, 9,9 , &fwcontrol_189);
        ReadRegister(HARDWARE_HARDWARE_25_reg, 7,7 , &fwcontrol_339);
        ReadRegister(HARDWARE_HARDWARE_25_reg, 12,12 , &fwcontrol_299_begin);  
        ReadRegister(HARDWARE_HARDWARE_25_reg, 17,17 , &fwcontrol_306);
        ReadRegister(HARDWARE_HARDWARE_25_reg, 18,18 , &fwcontrol_192);
        ReadRegister(HARDWARE_HARDWARE_25_reg, 19,19 , &fwcontrol_301); 
        ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_3_reg, 0,0 , &occl_more_1);
        ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_3_reg, 1,1 , &occl_more_2);
        ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_3_reg, 2,2 , &occl_more_3);
        ReadRegister(HARDWARE_HARDWARE_25_reg, 20,20 , &fwcontrol_153);
        ReadRegister(HARDWARE_HARDWARE_25_reg, 21,21 , &fwcontrol_128);

        if((pParam->u1_dehalo_Newalgo_Proc_en)||(enbale_readval == 1))
        {
            if(pParam->u1_dehalo_condition9)
            {
                if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_pred_en0==1)
                {
                    WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,10,11,pred_en_0);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,0,6,bg_useless_fix_bg_th);
                }  
                else
                {
                    WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,10,11,pred_en_init);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,0,6,bg_useless_fix_bg_th_init);
                }
            }
            else
            {
                WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,10,11,pred_en_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,0,6,bg_useless_fix_bg_th_init);
            }

            ReadRegister(KME_DEHALO5_PHMV_FIX_A_reg,10,11,&pred_en_pre);
        
            if(pParam->u1_dehalo_condition1)
            {
                if((s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_divide_en==1) || (s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_virtical==1))
                {
                    //WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,posdiff_judge_divide_en);
                    //WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,posdiff_judge_penalty_en);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,posdiff_judge_th1);
					WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,0,7,judge_max_th0);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,8,15,judge_max_th1);
                }
                else
                {
                    //WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,posdiff_judge_divide_en_init);
                    //WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,posdiff_judge_penalty_en_init);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,posdiff_judge_th1_init);
					WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,0,7,judge_max_th0_init);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,8,15,judge_max_th1_init);
                }
            }
            else
            {
                //WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,posdiff_judge_divide_en_init);
                //WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,posdiff_judge_penalty_en_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,posdiff_judge_th1_init);
				WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,0,7,judge_max_th0_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,8,15,judge_max_th1_init);
            }
        
            if(pParam->u1_dehalo_condition2)
            {
                if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_virtical==1)
                {
                    WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,26,30,pred_th1);
                    //WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,20,20,occl_correct_en);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,8,14,bg_extend_cov_th1);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,bg_extend_ucov_th0);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,24,30,bg_extend_ucov_th1);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,pred_ver_en);
                    //WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,16,16,pred_me1_diff_cut_en_init);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,old_pred_en);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,pred_old_diff_cut);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,7,7,bad_pred_cut);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,bad_old_cut);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,bg_change_cut);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,pred_bad_cut);         
                }
                else
                {
                    WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,26,30,pred_th1_init);
                    //WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,20,20,occl_correct_en_init);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0_init);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,8,14,bg_extend_cov_th1_init);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,bg_extend_ucov_th0_init);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,24,30,bg_extend_ucov_th1_init);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,pred_ver_en_init);
                    //WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,16,16,pred_me1_diff_cut_en);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,old_pred_en_init);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,pred_old_diff_cut_init);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,7,7,bad_pred_cut_init);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,bad_old_cut_init);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,bg_change_cut_init);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,pred_bad_cut_init);
                }
				if((s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_small_hor_slow == 1))
                {
                    //WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,pred_th0);
                }
                else
                {
                    //WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,pred_th0_init);
                }
            }
            else
            {
                WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,pred_th0_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,26,30,pred_th1_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,20,20,occl_correct_en_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,8,14,bg_extend_cov_th1_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,bg_extend_ucov_th0_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,24,30,bg_extend_ucov_th1_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,pred_ver_en_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,16,16,pred_me1_diff_cut_en_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,old_pred_en_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,pred_old_diff_cut_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,7,7,bad_pred_cut_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,bad_old_cut_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,bg_change_cut_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,pred_bad_cut_init);
            }
        
            if(pParam->u1_dehalo_condition3)
            {
                if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_large_mv_th_y2==1)
                {
                    WriteRegister(KME_DEHALO5_PHMV_FIX_9_reg,16,25,gmv_th_y2);
                }
                else
                {
                    WriteRegister(KME_DEHALO5_PHMV_FIX_9_reg,16,25,gmv_th_y2_init);
                }
            }
            else
            {
                WriteRegister(KME_DEHALO5_PHMV_FIX_9_reg,16,25,gmv_th_y2_init);
            }

			if(pParam->u1_dehalo_condition3)     //fast extend
            {
                if((s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_extend_more==1) || (s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_virtical))
                {
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,0,6, useless_max_th);
                }
                else
                {
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,0,6, useless_max_th_init);
                }
            }
            else
            {
                WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,0,6, useless_max_th_init);
            }

             if(pParam->u1_dehalo_condition3)
            {
                if(pParam->u1_dehalo_condition7)
                {
                    auto_extend_mvdiff_th = (_ABS_DIFF_(gmv_fgx,gmv_bgx) > 200) ? 40 : _ABS_DIFF_(gmv_fgx,gmv_bgx)/5;
                }
                else
                {
                    auto_extend_mvdiff_th = (_ABS_DIFF_(gmv_fgx,gmv_bgx) > 100) ? 20 : _ABS_DIFF_(gmv_fgx,gmv_bgx)/5;
                }
				
				if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_noslow_easy_extend==1 && (s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_317_broken == 0))
                {
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23, auto_extend_mvdiff_th);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,24,30, extend_flag_num);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_F_reg,16,25, clear_y0);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_11_reg,0,9, clear_y1);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_11_reg,16,25, clear_y2); 
                    WriteRegister(KME_DEHALO5_PHMV_FIX_10_reg,24,31, clear_posdiff_th);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,0,6,bg_useless_fix_bg_th);
                }
                else
                {
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23, extend_mvdiff_th_init);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,24,30, extend_flag_num_init);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_F_reg,16,25, clear_y0_init);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_11_reg,0,9, clear_y1_init);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_11_reg,16,25, clear_y2_init); 
                    WriteRegister(KME_DEHALO5_PHMV_FIX_10_reg,24,31, clear_posdiff_th_init);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,0,6,bg_useless_fix_bg_th_init);
                }
				if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_noslow_easy_extend==2)
                {
                    WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,extend4_posdiff_th); 
                }
                else
                {
                    WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,extend4_posdiff_th_init); 
                }

                if((s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_noslow_easy_extend==1) || (s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_noslow_easy_extend==2))
                {
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25, extend_posdiff_th);
                }
                else
                {
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25, extend_posdiff_th_init);
                }
            }
            else
            {
                WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23, extend_mvdiff_th_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25, extend_posdiff_th_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,24,30, extend_flag_num_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_F_reg,16,25, clear_y0_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_11_reg,0,9, clear_y1_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_11_reg,16,25, clear_y2_init); 
                WriteRegister(KME_DEHALO5_PHMV_FIX_10_reg,24,31, clear_posdiff_th_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,0,6,bg_useless_fix_bg_th_init);
            }
        
            if(pParam->u1_dehalo_condition7)
            {
            
                if(s_pContext->_output_frc_sceneAnalysis.u8_dh_fast_hor_in_cnt==1)
                {
                    WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,1,1,0x1); // hor en
                    WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,3,0x0); // pos en
                    WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,4,4,0x0); // neg en
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0_init);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,bg_extend_ucov_th0_init);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xa); //pred sun cut th
                    WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x1); //old pred en
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
                    FrameHold_fast_hor = 2;
                    FrameHoldflag_fast_hor = 1; 
                }
                else if(FrameHold_fast_hor > 0)
                {
                    WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,1,1,0x1); // hor en
                    WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,3,0x0); // pos en
                    WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,4,4,0x0); // neg en
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0_init);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,bg_extend_ucov_th0_init);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xa); //pred sun cut th
                    WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x1); //old pred en
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
                    FrameHold_fast_hor--;
                }
                else if(FrameHoldflag_fast_hor)
                {
                    WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,1,1,0x1); // hor en
                    WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,3,0x1); // pos en
                    WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,4,4,0x1); // neg en
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0_init);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,bg_extend_ucov_th0_init);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x14); //pred sun cut th
                    WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x1); //old pred en
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x12c); //extend posdiff th
                    FrameHoldflag_fast_hor = 0;
            
                }
            }
            
			if(pParam->u1_dehalo_condition4)
            {
            
                if((s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_slow_motion==1) && (s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_logo_off == 0))
                {
                    WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,gmv_th_y0);
                }
                else
                {
                    WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,gmv_th_y0_init);        
                }
            }
            else
            {
                WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,gmv_th_y0_init);
            }
			
			 if(pParam->u1_dehalo_condition4)
			 {
				 if((s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_small_mv_th_y0==1) && (s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_slow_motion==0))
				 {
					 WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,0xa); // gmv y0
					 WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,0xa); // pred y0
				 }
				 else if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_slow_motion==1)
				 {
					 WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,0x0);
				 }
				 else
				 {
					 WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,gmv_th_y0_init);
					 WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,pred_y0_init); 
				 }
			 }
			//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg,  0,  3, (s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_229)); 
			 if(pParam->u1_dehalo_condition5) 
			 {
				 if((s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_black_protect_off==1 || s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_301 ||
					 s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_229 || s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_171) && 
					 (s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_black_apl == 0))
				 {
					 WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,color_protect_th0_en);
				 }
				 else
				 {
					 WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,color_protect_th0_en_init);
				 }
				 if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_black_protect_off==1)
				 {
					 WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,25,30, pfv_bg_change_th); //for 176 ghost
					 WriteRegister(KME_DEHALO5_PHMV_FIX_36_reg,10,15, ppfv_bg_change_th);
				 }
				 else
				 {
					 WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,25,30, pfv_bg_change_th_init);
					 WriteRegister(KME_DEHALO5_PHMV_FIX_36_reg,10,15, ppfv_bg_change_th_init);
				 }
			 }
			 else
			 {
				 WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,color_protect_th0_en_init);
				 WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,25,30, pfv_bg_change_th_init);
				 WriteRegister(KME_DEHALO5_PHMV_FIX_36_reg,10,15, ppfv_bg_change_th_init);
			 }
			 
			 //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg,  0,  3, s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_301); 
			 if(pParam->u1_dehalo_condition5) //for 301, me1 black protect off
			 {
				 if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_301 || s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_229)
				 {
					 WriteRegister(KME_ME1_BG0_ME1_BG_3_reg, 0, 0, 0x0); //me1 black protect
					 WriteRegister(KME_ME1_BG0_ME1_BG_11_reg, 0, 0, 0x0);  //me1 black protect
					 me_blackoff_flag =1 ;
					 me_blackoff_frame_hold = 2 ; 
				 }
				 else if(me_blackoff_frame_hold>0)
				 {
					 WriteRegister(KME_ME1_BG0_ME1_BG_3_reg, 0, 0, 0x0); //me1 black protect
					 WriteRegister(KME_ME1_BG0_ME1_BG_11_reg, 0, 0, 0x0);  //me1 black protect
					 me_blackoff_frame_hold-- ;
				 }
				 else if(me_blackoff_flag)
				 {
					 me_blackoff_flag =0 ; 
					 WriteRegister(KME_ME1_BG0_ME1_BG_3_reg, 0, 0, 0x1); //me1 black protect
					 WriteRegister(KME_ME1_BG0_ME1_BG_11_reg, 0, 0, 0x1);  //me1 black protect
				 }
			 }
			 else
			 {
				 WriteRegister(KME_ME1_BG0_ME1_BG_3_reg, 0, 0, 0x1); //me1 black protect
				 WriteRegister(KME_ME1_BG0_ME1_BG_11_reg, 0, 0, 0x1);  //me1 black protect
			 }
            if(pParam->u1_dehalo_condition5) 
            {
                if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_229)
                {
                    WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg, 20, 20, 0x1); //color en4
                    WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg, 0, 7, 0x78);  //color th min
                    WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg, 8,15, 0x8c);  //color th max
                    color_pro_299_flag =1 ;
                    color_pro_299_hold = 2 ; 
                }
                else if(color_pro_299_hold>0)
                {
                    WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg, 20, 20, 0x0); //color en4
                    WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg, 0, 7, 0x0);  //color th min
                    WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg, 8,15, 0xff);  //color th max
                    color_pro_299_hold-- ;
                }
                else if(color_pro_299_flag)
                {
                    color_pro_299_flag =0 ; 
                    WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg, 20, 20, 0x0); //color en4
                    WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg, 0, 7, 0x0);  //color th min
                    WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg, 8,15, 0xff);  //color th max
                }
            }
            else
            {
                WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg, 20, 20, 0x0); //color en4
                WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg, 0, 7, 0x0);  //color th min
                WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg, 8,15, 0xff);  //color th max
            }

            if(pParam->u1_dehalo_condition5)
            {
                if((s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_white_protect_on==1) || (s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_predflag_dis))
                {
                WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,color_protect_th1_en);
                WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,16,23,color_protect_th1_min);
                WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut);
                WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,occl_filter_cov_cut);
                //rtd_pr_memc_info("white_protect_on\n");
                }
                else
                {
                WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,color_protect_th1_en_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,16,23,color_protect_th1_min_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,occl_filter_cov_cut_init);
                }
            }
            else
            {
                WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,color_protect_th1_en_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,16,23,color_protect_th1_min_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,occl_filter_cov_cut_init);
            }

	//for 144 face broken
		   //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg,	0,	3, (s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_144_face)); 
		   if(pParam->u1_dehalo_condition6)
		   {
			   if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_144_face==1)
			   {
				   WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,bg_extend_cond4_en);	  
				   WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x0);   
				   WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,2,2,0x0);  //mv filter en
				   WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,1,3,0x0);  //bg cond th
			   }
			   else
			   {
				   WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,bg_extend_cond4_en_init); 
				   WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x1);   
				   WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,2,2,0x1);  //mv filter en
				   WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,1,3,0x3);  //bg cond th
			   }
		   }
		   else
		   {
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,bg_extend_cond4_en_init); 
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x1);   
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,2,2,0x1);  //mv filter en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,1,3,0x3);  //bg cond th
		   }
		   
			if(pParam->u1_dehalo_condition6)
            {
                if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_logo_off==1)
                {
                    WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x0); 
                    ///////////////////////////
                }
                else
                {
                    WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x1);  
                }
            }
            else
            {
                WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x1);   
            }

            /*if(pParam->u1_dehalo_condition7)
            {
                if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_correct_cut_th==1)
                {
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,10,13,extend_correct_cut_th);
                }
                else
                {
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,10,13,extend_correct_cut_th_init);
                }
            }
            else
            {
                WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,10,13,extend_correct_cut_th_init); 
            }*/
        
            if(pParam->u1_dehalo_condition8)
            {
                if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_aplfix_en==1)
                {
                    WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,6,6,fg_protect_by_apl_en);
                }
                else
                {
                    WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,6,6,fg_protect_by_apl_en_init);
                }
            }
            else
            {
                WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,6,6,fg_protect_by_apl_en_init);
            }



			//----------------------- logo rim ----------------------------//
			if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_logo_rimoff==1) 
			{
				WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,12,12,0x0);
				check_biglogo = 1;
			}
			else if(check_biglogo>0)
			{
				WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,12,12,0x1);
				check_biglogo  = 0;
			} 


            //---------------------------for 189 ----------------------------------//
            if(fwcontrol_189 == 1)
            {
                FrameHold_189 = 30;  
                FrameHoldflag_189=1;
                WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x0);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x3);        
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x0);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x3);  
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x0); 
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x3);        
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x0);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x5);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x0);

                WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x2bc); //judge th1
                WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x1); //extend cond4 en
                WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0xf); //extend mvdiff th
                WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
                WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x12c); //extend4 posdiff
                WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,0xa);  //pred diff y0
                WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,25,29,0x12); //mv filter th
                WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,0x23); //gmv y0
                WriteRegister(KME_DEHALO5_PHMV_FIX_9_reg,0,9,0x3c); //gmv y1
                WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,0xc); //me1 pred th0
                WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,8,14,0x18); //useless fg th
                WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x0); //logo protect en
                WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,20,20,0x1); //color en4
                WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,0,7,0x4b); //color min4
                WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,8,15,0x61); //color max4
                WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,bg_change_cut);
                WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,pred_bad_cut);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x1);
                WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,bg_extend_ucov_th0_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1F_reg,8,14,bg_extend_bg_sum_max_th_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,16,16,pred_me1_diff_cut_en_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,color_protect_th0_en);
                WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,color_protect_th1_en);
                WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,16,23,color_protect_th1_min);
                WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,occl_filter_cov_cut_init);				
				
				//add 20210719
				
				WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,0x1);//reg_dh_color_protect_th1_en
				WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,20,20,0x0);//reg_dh_color_protect_th4_en
				WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,24,31,0xff);//color protect1 max
				WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,16,23,0x82);//color protect1 min
				WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,19,19,0x1);//extend occl cond2
				WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,18,18,0x0);//extend occl cond1
				WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,17,17,0x0);//extend occl cond0
				WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x1); // extend cond4
				WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x1);//reg_dh_bg_extend_cond3_en 
				WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0x8);  //mvdiff_th
				WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x154); //  //cond_posdiff_th
				


                
            }
            else if(FrameHold_189>0)
            {
                FrameHold_189 -- ;
                WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x0);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x3);        
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x0);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x3);  
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x0); 
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x3);        
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x0);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x5);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x0);

                WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x2bc); //judge th1
                WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x1); //extend cond4 en
                WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0xf); //extend mvdiff th
                WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
                WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x12c); //extend4 posdiff
                WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,0xa);  //pred diff y0
                WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,25,29,0x12); //mv filter th
                WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,0x23); //gmv y0
                WriteRegister(KME_DEHALO5_PHMV_FIX_9_reg,0,9,0x3c); //gmv y1
                WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,0xc); //me1 pred th0
                WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,8,14,0x18); //useless fg th
                WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x0); //logo protect en
                WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,20,20,0x1); //color en4
                WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,0,7,0x4b); //color min4
                WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,8,15,0x61); //color max4
                WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,bg_change_cut);
                WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,pred_bad_cut);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x1);
                WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,bg_extend_ucov_th0_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1F_reg,8,14,bg_extend_bg_sum_max_th_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,16,16,pred_me1_diff_cut_en_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,color_protect_th0_en);
                WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,color_protect_th1_en);
                WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,16,23,color_protect_th1_min);
                WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,occl_filter_cov_cut_init);

			
				//add 20210719
				WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,0x1);//reg_dh_color_protect_th1_en
				WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,20,20,0x0);//reg_dh_color_protect_th4_en
				WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,24,31,0xff);//color protect1 max
				WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,16,23,0x82);//color protect1 min
				WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,19,19,0x1);//extend occl cond2
				WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,18,18,0x0);//extend occl cond1
				WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,17,17,0x0);//extend occl cond0
				WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x1); // extend cond4
				WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x1);//reg_dh_bg_extend_cond3_en 
				WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0x8);  //mvdiff_th
				WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x154); //  //cond_posdiff_th
            }
            else if(FrameHoldflag_189)
            {
                FrameHoldflag_189=0;
                WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x1);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x0);        
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x0);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x0);  
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x0); 
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x0);        
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x0);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x0);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x0);
                
                WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x384); //judge th1
                WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x0); //extend cond4 en
                WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0x0); //extend mvdiff th
                WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x12c); //extend posdiff th
                WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x2bc); //extend4 posdiff
                WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,0x14);  //pred diff y0
               WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,25,29,0x14); //mv filter th
                WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,0x15); //gmv y0
                WriteRegister(KME_DEHALO5_PHMV_FIX_9_reg,0,9,0x30); //gmv y1
                WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,0x16); //me1 pred th0
                WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,8,14,0x25); //useless fg th
                WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x1); //logo protect en
                WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,20,20,0x0); //color en4
                WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,0,7,0x0); //color min4
                WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,8,15,0xff); //color max4
                WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,bg_change_cut_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,pred_bad_cut_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x1);
                WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,bg_extend_ucov_th0_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1F_reg,8,14,bg_extend_bg_sum_max_th_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,16,16,pred_me1_diff_cut_en_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,color_protect_th0_en_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,color_protect_th1_en_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,16,23,color_protect_th1_min_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,occl_filter_cov_cut_init);


                //add 20210719
                WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,0x0);//reg_dh_color_protect_th1_en
                WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,20,20,0x0);//reg_dh_color_protect_th4_en
                WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,24,31,0xff);//color protect1 max
                WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,16,23,0x96);//color protect1 min
                WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,19,19,0x1);//extend occl cond2
                WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,18,18,0x1);//extend occl cond1
                WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,17,17,0x1);//extend occl cond0
                WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x1); // extend cond4
                WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x1);//reg_dh_bg_extend_cond3_en 
                WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0x0);  //mvdiff_th
                WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x2bc); //  //cond_posdiff_th
               
                
                
            }
            //rtd_pr_memc_info("299 : %d    hold(%d)", ((s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_white_protect_on==1) && (s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_black_protect_off==1)), FrameHold_299);
            //-------------------------------------------------------------------//
            //---------------------------for 299 ----------------------------------//
            if(fwcontrol_299 == 1)
            {
                FrameHold_299 = 2;  
                FrameHoldflag_299=1;
                WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x0);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x1);   //extend       
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x0);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x3);  
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x0); 
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x1);   //extend       
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x0);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x3);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x0);

                WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x384); //judge th1
                WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0xf); //extend mvdiff th
                WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
                WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x12c); //extend4 posdiff
                WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,0xa);  //pred diff y0
                //WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,25,29,0x12); //mv filter th
                WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,0x19); //gmv y0
                WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,0xc); //me1 pred th0
                WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x0); //logo protect en
                WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,25,30, 0x9); //bg change pfv th
                WriteRegister(KME_DEHALO5_PHMV_FIX_36_reg,10,15, 0x9); //bg change ppfv th
                WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,0x1); //bg change en
                WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,pred_bad_cut);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x1);
                WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0x14);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0x14);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1F_reg,8,14,bg_extend_bg_sum_max_th_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,16,16,pred_me1_diff_cut_en_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,color_protect_th0_en);
                WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,color_protect_th1_en);
                WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,16,23,0x6e); //color_protect_th1_min
                WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xf); //cov cut
                WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0x9); //ucov cut
                WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,1,3,0x1);  //bg cond th
                WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,10,11,0x0);
                //WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0x1e); //extend cov th0
            }
            else if(FrameHold_299>0)
            {
                FrameHold_299 -- ;
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x1);   //extend       
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x0);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x3);  
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x0); 
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x1);   //extend       
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x0);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x3);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x0);

                WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x384); //judge th1
                WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0xf); //extend mvdiff th
                WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
                WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x12c); //extend4 posdiff
                WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,0xa);  //pred diff y0
                //WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,25,29,0x12); //mv filter th
                WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,0x19); //gmv y0
                WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,0xc); //me1 pred th0
                WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x0); //logo protect en
                WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,25,30, 0x9); //bg change pfv th
                WriteRegister(KME_DEHALO5_PHMV_FIX_36_reg,10,15, 0x9); //bg change ppfv th
                WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,0x1); //bg change en
                WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,pred_bad_cut);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x1);
                WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0x14);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0x14);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1F_reg,8,14,bg_extend_bg_sum_max_th_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,16,16,pred_me1_diff_cut_en_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,color_protect_th0_en);
                WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,color_protect_th1_en);
                WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,16,23,0x6e); //color_protect_th1_min
                WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xf); //cov cut
                WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0x9); //ucov cut
                WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,1,3,0x1);  //bg cond th
                WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,10,11,0x0);
                //WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0x1e); //extend cov th0
            }
            else if(FrameHoldflag_299)
            {
                FrameHoldflag_299=0;
                WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x1);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x0);        
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x0);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x0);  
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x0); 
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x0);        
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x0);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x0);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x0);
                
                WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x384); //judge th1
                WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0x0); //extend mvdiff th
                WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x12c); //extend posdiff th
                WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x2bc); //extend4 posdiff
                WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,0x14);  //pred diff y0
                //WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,25,29,0x14); //mv filter th
                WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,0x15); //gmv y0
                WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,0x16); //me1 pred th0
                WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x1); //logo protect en
                WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,bg_change_cut_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,25,30, pfv_bg_change_th_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_36_reg,10,15, ppfv_bg_change_th_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,pred_bad_cut_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x1);
                WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,bg_extend_ucov_th0_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_1F_reg,8,14,bg_extend_bg_sum_max_th_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,16,16,pred_me1_diff_cut_en_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,color_protect_th0_en_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,color_protect_th1_en_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,16,23,color_protect_th1_min_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,occl_filter_cov_cut_init);
                WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,1,3,0x3);  //bg cond th
                WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,10,11,pred_en_init);
                
            }
            if(fwcontrol_299_begin == 1)
            {
                FrameHold_299_begin = 2;  
                FrameHoldflag_299_begin=1;
                WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x0);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0);    
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0);    
            }
            else if(FrameHold_299_begin>0)
            {
                FrameHold_299_begin-- ;
                WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x0);
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0);    
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0);    
            }
            else if(FrameHoldflag_299_begin)
            {
                FrameHoldflag_299_begin =0 ;
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0);    
                WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0);    
                WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x1);
            }
            //rtd_pr_memc_info("299 : %d    hold(%d)", ((s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_white_protect_on==1) && (s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_black_protect_off==1)), FrameHold_299);
            //-------------------------------------------------------------------//

            //---------------------------for 128 ----------------------------------//   
            if(fwcontrol4)
            {
                if(fwcontrol0 )
                {
                    WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x5);
                    WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x5);
                    WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x5);
                    WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x5);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x0);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,25,29,0x11);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,16,20,0xb);
                    
                    WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x0);
                    WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x5);
                    WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x1);
                    WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x5);           
                    WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x1);
                    FrameHold = 5  ;  
                    FrameHoldflag=1;
                    
                }
                else if(FrameHold>0)
                {
                    FrameHold -- ;
                    WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x5);
                    WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x5);
                    WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x5);
                    WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x5);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x0);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,25,29,0x11);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,16,20,0xb);
                    WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x0);


                    WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x5);
                    WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x1);
                    WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x5);           
                    WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x1);
                }
                else if(FrameHoldflag)
                {
                    FrameHoldflag=0;
                    WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x0);
                    WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x0);
                    WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x0);
                    WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x0);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x22);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,25,29,0x14);
                    WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,16,20,0xf);
                    WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x1);

                    WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x0);
                    WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x0);
                    WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x0);           
                    WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x0);


                }
            }
        //--------------------------------- ----------------------------------//

        //---------------------------for 172 broken ----------------------------------// 
        if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_172_black_th_flag)
        {        
	WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x0);  //extend cond4 en
	WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x0);  //extend cond3 en
	WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23, extend_mvdiff_th_init);
	WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25, extend_posdiff_th_init);
	WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xe);
	WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0xe);

	FrameHold_172_broken = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 200 : 90	;  
	FrameHoldflag_172_broken=1;
			
        }
        else if(FrameHold_172_broken>0)
        {
	WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x0);  //extend cond4 en
	WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x0);  //extend cond3 en
	WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23, extend_mvdiff_th_init);
	WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25, extend_posdiff_th_init);
	WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xe);
	WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0xe);
	FrameHold_172_broken -- ;
			
        }
        else if(FrameHoldflag_172_broken)
        {
	WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x1);  //extend cond4 en
	WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x1);  //extend cond3 en
	WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23, extend_mvdiff_th_init);
	WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25, extend_posdiff_th_init);
	WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
	WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,occl_filter_cov_cut_init);
	FrameHoldflag_172_broken=0;

        }
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg,  0,  3, s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_200_face_flag); 
        //---------------------------for 339 ----------------------------------//
			if(fwcontrol_339)
			{
				WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x258); //judge th1
				WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x6e); //color th0 max
				WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0xf); //extend mvdiff th
				WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
				WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x12c); //extend4 posdiff
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xb);
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0xb);

				FrameHold_339 = 5  ;  
				FrameHoldflag_339=1;
				
			}
			else if(FrameHold_339>0)
			{
				FrameHold_339 -- ;
				WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x258); //judge th1
				WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x6e); //color th0 max
				WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0xf); //extend mvdiff th
				WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
				WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x12c); //extend4 posdiff
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xb);
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0xb);
			}
			else if(FrameHoldflag_339)
			{
				FrameHoldflag_339=0;
				WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x384); //judge th1
				WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x22); //color th0 max
				WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0x0); //extend mvdiff th
				WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x12c); //extend posdiff th
				WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x2bc); //extend4 posdiff
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,occl_filter_cov_cut_init);
			}
        //--------------------------------- ----------------------------------//

        //---------------------------for 185 ----------------------------------//
			if(fwcontrol_185)
			{
				WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x0);
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x0);		
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x0);
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x0);
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x0);  
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x0); 
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x0);		  
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x0);
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x0);
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x0);
				
				//WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,28,29,0x0); //pick phmv mv
				//WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,25,29,0x12); //mv filter th
				WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x190); //judge th1
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xa); //pred sum th
				WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,0xc); //me1 pred th0
				//WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0x0); //extend mvdiff th
				WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
				WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x18); //color th0 max
				WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x12c); //extend4 posdiff
				WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x0);  //logo protect
				WriteRegister(KME_DEHALO5_PHMV_FIX_19_reg,16,22,0xb);  //only ucov th0
				WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,0xa);  //pred diff y0
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x0);  //old occl en
				WriteRegister(KME_ME1_BG0_ME1_BG_3_reg, 0, 0, 0x1); //me1 black protect
				WriteRegister(KME_ME1_BG0_ME1_BG_11_reg, 0, 0, 0x1);  //me1 black protect
				
				WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,0,7,judge_max_th0);
				WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,8,15,judge_max_th1);
				WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,bg_change_cut);
				WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,pred_bad_cut);
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,bg_extend_ucov_th0_init);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0_init);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1F_reg,8,14,bg_extend_bg_sum_max_th_init);
				WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,16,16,pred_me1_diff_cut_en_init);
				
				FrameHold_185 = 60	;  
				FrameHoldflag_185=1;

				 if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_virtical_case5)
				{	 
				
					 WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,0); //gmv y0
					 WriteRegister(KME_DEHALO5_PHMV_FIX_9_reg,0,9,0x0); //gmv y1
				 }
			}
			else if(FrameHold_185>0)
			{
				FrameHold_185 -- ;
				WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x0);
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x0);		
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x0);
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x0);
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x0);  
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x0); 
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x0);		  
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x0);
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x0);
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x0);
				
				//WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,28,29,0x0); //pick phmv mv
				//WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,25,29,0x12); //mv filter th
				WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x190); //judge th1
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xa); //pred sum th
				WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,0xc); //me1 pred th0
				//WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0x0); //extend mvdiff th
				WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
				WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x18); //color th0 max
				WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x12c); //extend4 posdiff
				WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x0);  //logo protect
				WriteRegister(KME_DEHALO5_PHMV_FIX_19_reg,16,22,0xb);  //only ucov th0
				WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,0xa);  //pred diff y0
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x0);  //old occl en
				WriteRegister(KME_ME1_BG0_ME1_BG_3_reg, 0, 0, 0x1); //me1 black protect
				WriteRegister(KME_ME1_BG0_ME1_BG_11_reg, 0, 0, 0x1);  //me1 black protect
				
				WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,0,7,judge_max_th0);
				WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,8,15,judge_max_th1);
				WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,bg_change_cut);
				WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,pred_bad_cut);
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,bg_extend_ucov_th0_init);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0_init);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1F_reg,8,14,bg_extend_bg_sum_max_th_init);
				WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,16,16,pred_me1_diff_cut_en_init);
				if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_virtical_case5)
				 { 
						WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,0); //gmv y0
						WriteRegister(KME_DEHALO5_PHMV_FIX_9_reg,0,9,0x0); //gmv y1
				}
			}
			else if(FrameHoldflag_185)
			{
				FrameHoldflag_185=0;
				WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x1);
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x0);		
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x0);
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x0);  
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x0); 
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x0);		  
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x0);
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x0);
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x0);
				
				//WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,28,29,0x2); //pick phmv mv
				//WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,25,29,0x14); //mv filter th
				WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x384); //judge th1
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x14); //pred sum th
				WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,0x16); //me1 pred th0
				//WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0x0); //extend mvdiff th
				WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x12c); //extend posdiff th
				WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x22); //color th0 max
				WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x2bc); //extend4 posdiff
				WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x1);  //logo protect
				WriteRegister(KME_DEHALO5_PHMV_FIX_19_reg,16,22,0xf);  //only ucov th0
				WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,0x14);	//pred diff y0
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x1);  //old occl en
				WriteRegister(KME_ME1_BG0_ME1_BG_3_reg, 0, 0, 0x1); //me1 black protect
				WriteRegister(KME_ME1_BG0_ME1_BG_11_reg, 0, 0, 0x1);  //me1 black protect
				
				WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,0,7,judge_max_th0_init);
				WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,8,15,judge_max_th1_init);
				WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,bg_change_cut_init);
				WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,pred_bad_cut_init);
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,bg_extend_ucov_th0_init);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0_init);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1F_reg,8,14,bg_extend_bg_sum_max_th_init);
				WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,16,16,pred_me1_diff_cut_en_init);
				WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,0x15); //gmv y0
				WriteRegister(KME_DEHALO5_PHMV_FIX_9_reg,0,9,0x30); //gmv y1

			}
		 //--------------------------------- ----------------------------------//
		 
		 //---------------------------for 13m2v ----------------------------------//
			 gmv_fgx=s_pContext->_output_read_comreg.dh_gmv_fgx;
			 gmv_fgy=s_pContext->_output_read_comreg.dh_gmv_fgy;
			 //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg,  0,  3, (_ABS_(gmv_fgy) < 55)	&& (_ABS_(gmv_fgx) <10)); 
			if(fwcontrol_13/* && (!(_ABS_(gmv_fgy) < 55)  && (_ABS_(gmv_fgx) <10))*/)
			{
				WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,2,2,0x0); //dh black protect en
				WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,0,0,0x0); //me1 black protect en
				WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,0,0,0x0); //me1 black protect en
				WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x1); //ver_en
				WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,0x0); //bad pred
				WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x12c); //extend4 posdiff
				WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,0xf);  //pred diff y0
				WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
				WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0xa); //extend mvdiff th
				WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,0x14); //me1 pred th0
				WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,25,29,0x18); //mv filter th
				WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x1); //extend cond4 en
				WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x258); //judge th1
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xa); //pred sum th
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,16,20,0xd); //filter cov th0
				WriteRegister(KME_DEHALO5_PHMV_FIX_7_reg,8,12,0xd); //filter ucov th0
				WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,0x0); //pred old diff cut
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x0); //old occl en
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,0x0); //bad old cut
				WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,1,3,0x1);	//bg cond th
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0x9);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0xa); //cov th0
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0xa); //ucov th0
				WriteRegister(KME_DEHALO5_PHMV_FIX_34_reg,16,23,0x14); //pred y th0
				WriteRegister(KME_DEHALO5_PHMV_FIX_34_reg,24,31,0x14); //pred y th1

				FrameHold_13 = 5  ;  
				FrameHoldflag_13=1;
				
			}
			else if(FrameHold_13>0)
			{
				WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,2,2,0x0); //dh black protect en
				WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,0,0,0x0); //me1 black protect en
				WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,0,0,0x0); //me1 black protect en
				WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x1); //ver_en
				WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,0x0); //bad pred
				WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x12c); //extend4 posdiff
				WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,0xf);  //pred diff y0
				WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
				WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0xa); //extend mvdiff th
				WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,0x14); //me1 pred th0
				WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,25,29,0x18); //mv filter th
				WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x1); //extend cond4 en
				WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x258); //judge th1
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xa); //pred sum th
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,16,20,0xd); //filter cov th0
				WriteRegister(KME_DEHALO5_PHMV_FIX_7_reg,8,12,0xd); //filter ucov th0
				WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,0x0); //pred old diff cut
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x0); //old occl en
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,0x0); //bad old cut
				WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,1,3,0x1);	//bg cond th
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0x9);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0xa); //cov th0
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0xa); //ucov th0
				WriteRegister(KME_DEHALO5_PHMV_FIX_34_reg,16,23,0x14); //pred y th0
				WriteRegister(KME_DEHALO5_PHMV_FIX_34_reg,24,31,0x14); //pred y th1
				FrameHold_13 -- ;
			}
			else if(FrameHoldflag_13)
			{
				WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,2,2,0x1); //dh black protect en
				WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,0,0,0x1); //me1 black protect en
				WriteRegister(KME_ME1_BG0_ME1_BG_3_reg,0,0,0x1); //me1 black protect en
				WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x0); //ver_en
				WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,0x1); //bad pred
				WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x2bc); //extend4 posdiff
				WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,0x14);	//pred diff y0
				WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x12c); //extend posdiff th
				WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0x0); //extend mvdiff th
				WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,0x16); //me1 pred th0
				WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,25,29,0x14); //mv filter th
				WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x0); //extend cond4 en
				WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x384); //judge th1
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x14); //pred sum th
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,16,20,0xf); //filter cov th0
				WriteRegister(KME_DEHALO5_PHMV_FIX_7_reg,8,12,0xf); //filter ucov th0
				WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,0x1); //pred old diff cut
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x1); //old occl en
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,0x1); //bad old cut
				WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,1,3,0x3);  //bg cond th
           		WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,occl_filter_cov_cut_init);
	            WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0xc); //cov th0
	            WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0xc); //ucov th0
	            WriteRegister(KME_DEHALO5_PHMV_FIX_34_reg,16,23,0x5); //pred y th0
	            WriteRegister(KME_DEHALO5_PHMV_FIX_34_reg,24,31,0xf); //pred y th1
				FrameHoldflag_13=0; 
			}
		 //--------------------------------- ----------------------------------// 
		
		//---------------------------for 301 ----------------------------------//
		   if(fwcontrol_301)
		   {
			   WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x0);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x0);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x1); 	   
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x1);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x1);  
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x1); 
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x1);		 
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x1);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x1);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x1);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
			   //WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x12c); //extend4 posdiff
			   //WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x1);  //penalty_en
			   //WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x1); //divide en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xa); //pred sum cut
			   WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x190); //judge th1
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0x8); //cov th0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,8,14,0x8); //cov th1
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0x8); //ucov th0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,24,30,0x8); //ucov th1
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x1); //ver_en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,0x0); //bad pred
			   //WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,1,3,0x4);  //bg cond th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x0);  //logo protect
			   WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,0xa);  //pred diff y0
			   //WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,5,5,0x1);  //small en
			   //WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,6,7,0x1);  //small min th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0x14); //extend mvdiff th
			   //WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,16,20,0xa); //filter cov th0
			   //WriteRegister(KME_DEHALO5_PHMV_FIX_7_reg,8,12,0xc); //filter ucov th0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,0x15); //gmv y0
			   //WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,28,29,0x1); //pick phmv mv
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,20,20,0x1); //color en4
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,0,7,0x8a); //color min4
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,8,15,0xff); //color max4
			   WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,pred_old_diff_cut);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,7,7,bad_pred_cut);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,bad_old_cut);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,bg_change_cut);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xa);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0xa);
			   
			   FrameHold_301 = 20  ;  
			   FrameHoldflag_301=1;
			   
		   }
		   else if(FrameHold_301>0)
		   {
				  FrameHold_301 -- ;
			   WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x0);
				  WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x0);
				  WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x1);		  
				  WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x1);
				  WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
				  WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x1);  
				  WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x1); 
				  WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x1);		
				  WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x1);
				  WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x1);
				  WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x1);
				  WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
				  //WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x12c); //extend4 posdiff
				  //WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x1);	//penalty_en
				  //WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x1); //divide en
				  WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xa); //pred sum cut
				  WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x190); //judge th1
				  WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0x8); //cov th0
				  WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,8,14,0x8); //cov th1
				  WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0x8); //ucov th0
				  WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,24,30,0x8); //ucov th1
				  WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x1); //ver_en
				  WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,0x0); //bad pred
				  //WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,1,3,0x4);	//bg cond th
				  WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x0);	//logo protect
				  WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,0xa);  //pred diff y0
				  //WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,5,5,0x1);	//small en
				  //WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,6,7,0x1);	//small min th
				  WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0x14); //extend mvdiff th
				  //WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,16,20,0xa); //filter cov th0
				  //WriteRegister(KME_DEHALO5_PHMV_FIX_7_reg,8,12,0xc); //filter ucov th0
				  WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,0x15); //gmv y0
				  //WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,28,29,0x1); //pick phmv mv
				  WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,20,20,0x1); //color en4
				  WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,0,7,0x8a); //color min4
				  WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,8,15,0xff); //color max4
				  WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,pred_old_diff_cut);
				  WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,7,7,bad_pred_cut);
				  WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,bad_old_cut);
				  WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,bg_change_cut);
				  WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xa);
				  WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0xa);
		   }
		   else if(FrameHoldflag_301)
		   {
				  FrameHoldflag_301=0;
			   WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x1);
				  WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
				  WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x0);		  
				  WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x0);
				  WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
				  WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x0);  
				  WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x0); 
				  WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x0);		
				  WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x0);
				  WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x0);
				  WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x0);
				  WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x12c); //extend posdiff th
				  //WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x2bc); //extend4 posdiff
				  //WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x0);	//penalty_en
				  WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x0); //divide en
				  WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x14); //pred sum cut
				  WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x384); //judge th1
				  WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0xc); //cov th0
				  WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,8,14,0x5); //cov th1
				  WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0xc); //ucov th0
				  WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,24,30,0x5); //ucov th1
				  WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x0); //ver_en
				  WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,0x1); //bad pred
				  //WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,1,3,0x3);	//bg cond th
				  WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x1);	//logo protect
				  WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,0x14);  //pred diff y0
				  WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,5,5,0x0);  //small en
				  WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,6,7,0x3);  //small min th
				  WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0x0); //extend mvdiff th
				  WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,16,20,0xf); //filter cov th0
				  WriteRegister(KME_DEHALO5_PHMV_FIX_7_reg,8,12,0xf); //filter ucov th0
				  WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,0x15); //gmv y0
				  //WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,28,29,0x2); //pick phmv mv
				  WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,20,20,0x0); //color en4
				  WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,0,7,0x0); //color min4
				  WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,8,15,0xff); //color max4
				  WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,pred_old_diff_cut_init);
				  WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,7,7,bad_pred_cut_init);
				  WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,bad_old_cut_init);
				  WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,bg_change_cut_init);
				  WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
				  WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,occl_filter_cov_cut_init);
		   }
		   //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg,	0,	3, (FrameHold_301>0)); 
		   //--------------------------------- ----------------------------------//
			   
			//---------------------------for 306 ----------------------------------//
		   if(fwcontrol_306)
		   {
			   WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x0);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x2); 	   
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x0);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x2);  
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x0); 
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x2);		 
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x0);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x2);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x0);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x12c); //extend4 posdiff
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x1);  //penalty_en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x1); //divide en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,0xa); //me1 pred th0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xa); //pred sum cut
		
			   FrameHold_306 = 160	;  
			   FrameHoldflag_306=1;
			   
		   }
		   else if(FrameHold_306>0)
		   {
			   FrameHold_306 -- ;
			   WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x0);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x2); 	   
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x0);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x2);  
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x0); 
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x2);		 
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x0);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x2);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x0);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x12c); //extend4 posdiff
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x1);  //penalty_en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x1); //divide en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,0xa); //me1 pred th0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xa); //pred sum cut

		   }
		   else if(FrameHoldflag_306)
		   {
			   FrameHoldflag_306=0;
			   WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x1);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x0); 	   
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x0);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x0);  
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x0); 
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x0);		 
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x0);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x0);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x0);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x12c); //extend posdiff th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x2bc); //extend4 posdiff
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x0);  //penalty_en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x0); //divide en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,0x16); //me1 pred th0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x14); //pred sum cut

		   }
		   //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg,	0,	3, (FrameHold_306>0)); 
		   //--------------------------------- ----------------------------------//
		
		   //---------------------------for 192 ----------------------------------//
		   left_rim_pre=s_pContext->_output_rimctrl.u9_dh_blk_rim[_RIM_LFT];
		   if(fwcontrol_192)
		   {
			   WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x0);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x0); 	   
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x0);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x2);  
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x0); 
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x0);		 
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x0);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x2);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x0);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x12c); //extend4 posdiff
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xa); //pred sum cut
			   WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x190); //judge th1
			   WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x0); 
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0x1e); //extend cov th0
	           WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0x11); //filter cov cut
	           WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,0x28); //pred y0
	           WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x0);//old pred en
	           WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23, extend_mvdiff_th);
	           //WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,0,8,  0xa3);//left rim
			   //WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x1);  //penalty_en
			   //WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x1); //divide en
		
			   FrameHold_192 = 60  ;  
			   FrameHoldflag_192=1;
			   
		   }
		   else if(FrameHold_192>0)
		   {
			   FrameHold_192 -- ;
			   WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x0);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x0); 	   
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x0);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x2);  
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x0); 
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x0);		 
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x0);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x2);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x0);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x12c); //extend4 posdiff
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xa); //pred sum cut
			   WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x190); //judge th1
			   WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x0); 
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0x1e); //extend cov th0
	           WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0x11); //filter cov cut
	           WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,0x28); //pred y0
	           WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x0);//old pred en
	           WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23, extend_mvdiff_th);
	           //WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,0,8,  0xa3);//left rim
			   //WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x1);  //penalty_en
			   //WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x1); //divide en
		   }
		   else if(FrameHoldflag_192)
		   {
			   FrameHoldflag_192=0;
			   WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x1);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x0); 	   
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x0);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x0);  
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x0); 
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x0);		 
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x0);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x0);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x0);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x12c); //extend posdiff th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x2bc); //extend4 posdiff
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x14); //pred sum cut
			   WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x384); //judge th1
			   WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x1); 
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0xc); //extend cov th0
	           WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0x4); //filter cov cut
	           WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,old_pred_en_init);
	           WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,pred_y0_init); 
	           WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23, extend_mvdiff_th_init);
	           //WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,0,8,  left_rim_pre);//left rim
			   //WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x0);  //penalty_en
			   //WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x0); //divide en
		   }
		   //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg,	0,	3, (FrameHold_192>0)); 
        	/*rtd_pr_memc_info("l(%d) r(%d) t(%d) b(%d)", s_pContext->_output_rimctrl.u9_dh_blk_rim[_RIM_LFT],
                                                            s_pContext->_output_rimctrl.u9_dh_blk_rim[_RIM_RHT],
                                                            s_pContext->_output_rimctrl.u9_dh_blk_rim[_RIM_TOP],
                                                            s_pContext->_output_rimctrl.u9_dh_blk_rim[_RIM_BOT]);*/
 		   //--------------------------------- ----------------------------------//
		
		   //---------------------------------for 226 dark patten ----------------------------------//
		   if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_black_apl==1)
		   {
			   FrameHold_226 = 60;	
			   FrameHoldflag_226=1;
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xc); //filter ucov cut
			   WriteRegister(KME_DEHALO5_PHMV_FIX_7_reg,8,12,0xc); //filter cov th0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0x1e); //extend ucov th0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0x14); //cov th0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,3,0x0); //pos en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,4,4,0x0); //neg en
		   }
		   else if(FrameHold_226>0)
		   {
			   FrameHold_226 -- ;
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xc); //filter ucov cut
			   WriteRegister(KME_DEHALO5_PHMV_FIX_7_reg,8,12,0xc); //filter cov th0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0x1e); //extend ucov th0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0xc); //cov th0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0x14); //cov th0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,3,0x0); //pos en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,4,4,0x0); //neg en
		   }
		   else if(FrameHoldflag_226)
		   {
			   FrameHoldflag_226=0;
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0x4);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_7_reg,8,12,0xf); //filter cov th0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0xc); //extend ucov th0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0xc); //cov th0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0xc); //cov th0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,3,0x1); //pos en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,4,4,0x1); //neg en
		   }
		   //---------------------------for 153 ----------------------------------//
		   if(fwcontrol_153)
		   {		
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x1);  //penalty_en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x1); //divide en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,1,3,0x1); //bg cond th
			   FrameHold_153 = 30  ;  
			   FrameHoldflag_153=1;
					   
		   }
		   else if(FrameHold_153>0)
		   {
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x1);  //penalty_en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x1); //divide en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,1,3,0x1); //bg cond th
			   FrameHold_153 -- ;
					   
		   }
		   else if(FrameHoldflag_153)
		   {
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x0);  //penalty_en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x0); //divide en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,1,3,0x3); //bg cond th
			   FrameHoldflag_153=0;
		
		   }
			//---------------------------for 317 broken ----------------------------------// 
			if((s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_317_broken || s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_317_face_flag) && (!fwcontrol_153) && (!fwcontrol_128))
			{		 
				WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x0);  //extend cond3 en
				WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x0);  //extend cond4 en
				WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23, extend_mvdiff_th_init);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25, extend_posdiff_th_init);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,24,30, extend_flag_num_init);
				WriteRegister(KME_DEHALO5_PHMV_FIX_F_reg,16,25, clear_y0_init);
				WriteRegister(KME_DEHALO5_PHMV_FIX_11_reg,0,9, clear_y1_init);
				WriteRegister(KME_DEHALO5_PHMV_FIX_11_reg,16,25, clear_y2_init); 
				WriteRegister(KME_DEHALO5_PHMV_FIX_10_reg,24,31, clear_posdiff_th_init);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,0,6,bg_useless_fix_bg_th_init);
				WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,1,3,0x1); //bg cond th
				WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,20,20,0x1); //color en4
				WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,0,7,0x50); //color min4
				WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,8,15,0x8c); //color max4
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xe);
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0xe);
				
				FrameHold_317_broken = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 80 : 50  ;  
				FrameHoldflag_317_broken=1;
	                    
	        }
	        else if(FrameHold_317_broken>0)
	        {
	            WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x0);  //extend cond3 en
	            WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x0);  //extend cond4 en
	            WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23, extend_mvdiff_th_init);
	            WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25, extend_posdiff_th_init);
	            WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,24,30, extend_flag_num_init);
	            WriteRegister(KME_DEHALO5_PHMV_FIX_F_reg,16,25, clear_y0_init);
	            WriteRegister(KME_DEHALO5_PHMV_FIX_11_reg,0,9, clear_y1_init);
	            WriteRegister(KME_DEHALO5_PHMV_FIX_11_reg,16,25, clear_y2_init); 
	            WriteRegister(KME_DEHALO5_PHMV_FIX_10_reg,24,31, clear_posdiff_th_init);
	            WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,0,6,bg_useless_fix_bg_th_init);
	            WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,1,3,0x1); //bg cond th
                WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,20,20,0x1); //color en4
	            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,0,7,0x50); //color min4
	            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,8,15,0x8c); //color max4
	            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xe);
	            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0xe);
	            FrameHold_317_broken -- ;
	                    
	        }
	        else if(FrameHoldflag_317_broken)
	        {
	            WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x1);  //extend cond3 en
	            WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x1);  //extend cond4 en
	            WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23, extend_mvdiff_th_init);
	            WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25, extend_posdiff_th_init);
	            WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,24,30, extend_flag_num_init);
	            WriteRegister(KME_DEHALO5_PHMV_FIX_F_reg,16,25, clear_y0_init);
	            WriteRegister(KME_DEHALO5_PHMV_FIX_11_reg,0,9, clear_y1_init);
	            WriteRegister(KME_DEHALO5_PHMV_FIX_11_reg,16,25, clear_y2_init); 
	            WriteRegister(KME_DEHALO5_PHMV_FIX_10_reg,24,31, clear_posdiff_th_init);
	            WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,0,6,bg_useless_fix_bg_th_init);
	            WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,1,3,0x3); //bg cond th
	            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,20,20,0x0); //color en4
	            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,0,7,0x0); //color min4
	            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,8,15,0xff); //color max4
	            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
	            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,occl_filter_cov_cut_init);
	            FrameHoldflag_317_broken=0;

	        }	
		   
	        //---------------------------for 317 broken2 ----------------------------------//
	        if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_317_face_flag2 == 1)     
	        {
	            //WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x0);  //extend cond3 en          
	            WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x0);  //extend cond4 en            
	            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xe);     
	            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0xe);        
	            FrameHold_317_broken2 = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 800 : 400 ;          
	            FrameHoldflag_317_broken2=1;                            
	        }
	        else if(FrameHold_317_broken2>0)       
	        {
	            //WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x0);  //extend cond3 en     
	            WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x0);  //extend cond4 en         
	            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xe);          
	            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0xe);    
	            FrameHold_317_broken2 -- ;                           
	        }
	        else if(FrameHoldflag_317_broken2)        
	        {
	            //WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x1);  //extend cond3 en     
	            WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x1);  //extend cond4 en   
	            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);          
	            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,occl_filter_cov_cut_init);         
	            FrameHoldflag_317_broken2=0;       
	        }
	        
	        //---------------------------for 317 broken3 ----------------------------------//
			if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_317_face_flag3 == 1) 
	        {
	            WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x0);  //extend cond3 en          
	            WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x0);  //extend cond4 en            
	            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xe);     
	            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0xe);        
	            FrameHold_317_broken3 = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 200 : 100 ;          
	            FrameHoldflag_317_broken3=1;                            
	        }
	        else if(FrameHold_317_broken3>0)       
	        {
	            WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x0);  //extend cond3 en     
	            WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x0);  //extend cond4 en         
	            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xe);          
	            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0xe);    
	            FrameHold_317_broken3 -- ;                           
	        }
	        else if(FrameHoldflag_317_broken3)        
	        {
	            WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x1);  //extend cond3 en     
	            WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x1);  //extend cond4 en   
	            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);          
	            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,occl_filter_cov_cut_init);         
	            FrameHoldflag_317_broken3=0;       
	        }
	        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg,  0,  3, FrameHold_317_broken2>0 || FrameHold_317_broken3>0); 
	        
	        //---------------------------for 151 broken ----------------------------------//		   
		   if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_151_broken == 1)	  
		   {
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x0);  //extend cond3 en			
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x0);  //extend cond4 en			  
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xe);	   
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0xe);		 
			   FrameHold_151_broken = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 360 : 200 ;		  
			   FrameHoldflag_151_broken=1;							  
		   }
		   else if(FrameHold_151_broken>0)		 
		   {
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x0);  //extend cond3 en	   
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x0);  //extend cond4 en		   
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xe);			
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0xe);	 
			   FrameHold_151_broken -- ;						   
		   }
		   else if(FrameHoldflag_151_broken)		
		   {
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x1);  //extend cond3 en	   
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x1);  //extend cond4 en	 
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);		  
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,occl_filter_cov_cut_init);		   
			   FrameHoldflag_151_broken=0;		 
		   }
		   //---------------------------for 200 broken ----------------------------//
		   if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_200_face_flag == 1)
		   {		
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x0);  //extend cond3 en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x0);  //extend cond4 en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xe);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0xe);
		   
			   FrameHold_200_broken = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 150 : 80  ;  
			   FrameHoldflag_200_broken=1;
					   
		   }
		   else if(FrameHold_200_broken>0)
		   {
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x0);  //extend cond3 en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x0);  //extend cond4 en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xe);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0xe);
			   FrameHold_200_broken -- ;
					   
		   }
		   else if(FrameHoldflag_200_broken)
		   {
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x1);  //extend cond3 en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x1);  //extend cond4 en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,occl_filter_cov_cut_init);
			   FrameHoldflag_200_broken=0;
		   
		   }
		 //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg,  0,  3, FrameHold_200_broken>0); 
		 
		 //---------------------------for 366 broken ----------------------------//
		 if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_366_flag == 1)
		 {		  
			 WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x0);  //extend cond3 en
			 WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x0);  //extend cond4 en
			 WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xe);
			 WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0xe);
		 
			 FrameHold_366 = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 150 : 80  ;  
			 FrameHoldflag_366=1;
					 
		 }
		 else if(FrameHold_366>0)
		 {
			 WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x0);  //extend cond3 en
			 WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x0);  //extend cond4 en
			 WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xe);
			 WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0xe);
			 FrameHold_366 -- ;
					 
		 }
		 else if(FrameHoldflag_366)
		 {
			 WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x1);  //extend cond3 en
			 WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x1);  //extend cond4 en
			 WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
			 WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,occl_filter_cov_cut_init);
			 FrameHoldflag_366=0;
		 
		 }
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg,  0,  3, FrameHold_366>0); 
        
        //---------------------------for 432 bad gmv issue ----------------------------//
        if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_432_flag > 0)
        {        
            FrameHold_432_broken = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 400 : 260  ;  
            FrameHoldflag_432_broken=1;

            if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_432_flag == 1)
            {
                WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0x1);
                WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, 0x77e);
                //WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0x0);
                WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0x1);
                WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, 0x77e);
                //WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, 0x0);
            }
			else if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_432_flag == 2)
			{
				WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0x1);
				WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, 0x79c);
				//WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0x0);
				WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0x1);
				WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, 0x79c);
				//WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, 0x0);
			}
			//WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x0);	//extend cond3 en
			WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x0);  //extend cond4 en
			WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0x12);
			WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0x12);
			WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,10,11,pred_en_0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,0x0);//black protect en
			WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,0x0);//write protect en
			WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,0x0); //bg change en
		
			WriteRegister(HARDWARE_HARDWARE_25_reg, 26,26 , 0x1);
					
		}
        else if(FrameHold_432_broken>0)
        {
            FrameHold_432_broken -- ;
            if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_432_flag == 1)
            {
                WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0x1);
                WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, 0x77e);
                //WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0x0);
                WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0x1);
                WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, 0x77e);
                //WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, 0x0);
            }
            else if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_432_flag == 2)
            {
                WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0x1);
                WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, 0x79c);
                //WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0x0);
                WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0x1);
                WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, 0x79c);
                //WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, 0x0);
            }
            //WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x0);  //extend cond3 en
            WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x0);  //extend cond4 en
            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0x12);
            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0x12);
            WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,10,11,pred_en_0);
            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,0x0);//black protect en
            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,0x0);//write protect en
            WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,0x0); //bg change en

            WriteRegister(HARDWARE_HARDWARE_25_reg, 26,26 , 0x1);
                    
        }
        else if(FrameHoldflag_432_broken)
        {
            FrameHoldflag_432_broken=0;
            WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0x0);
            WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, 0x0);
            //WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0x0);
            WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0x0);
            WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, 0x0);
            //WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, 0x0);
            //WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x1);  //extend cond3 en
            WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x1);  //extend cond4 en
            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,occl_filter_cov_cut_init);
            WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,10,11,pred_en_init);
            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,0x1);//black protect en
            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,0x0);//write protect en
            WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,0x1); //bg change en

            WriteRegister(HARDWARE_HARDWARE_25_reg, 26,26 , 0x0);

        }
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg,  0,  3, FrameHold_432_broken>0); 

        //---------------------------for 432 final ----------------------------//
        if(s_pContext->_output_frc_sceneAnalysis.u8_dh_gmv_valchange_432final == 1)
        {        

            FrameHold_432_final = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 80: 40  ;  
            FrameHoldflag_432_final=1;
            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,0x0);//black protect en
            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,0x0);//write protect en
                    
        }
        else if(FrameHold_432_final>0)
        {
            FrameHold_432_final -- ;
            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,0x0);//black protect en
            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,0x0);//write protect en
                    
        }
        else if(FrameHoldflag_432_final)
        {
            FrameHoldflag_432_final=0;
            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,0x1);//black protect en
            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,0x0);//write protect en
        }

        //---------------------------for 43 broken and ghost ----------------------------//
        if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_43_flag == 1)
        {        

            FrameHold_43_broken = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 80: 40  ;  
            FrameHoldflag_43_broken=1;
            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xc);
            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0xc);
            WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,12,12,0x0); //only mv en
            WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,2,2,0x0); //mv filter
                    
        }
        else if(FrameHold_43_broken>0)
        {
            FrameHold_43_broken -- ;
            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xc);
            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0xc);
            WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,12,12,0x0); //only mv en
            WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,2,2,0x0); //mv filter
                    
        }
        else if(FrameHoldflag_43_broken)
        {
            FrameHoldflag_43_broken=0;
            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,occl_filter_cov_cut_init);
            WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,12,12,0x1); //only mv en
            WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,2,2,0x1); //mv filter

        }
         //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg,  0,  3, FrameHold_43_broken>0); 

        //---------------------------for 130 wrong gmv ----------------------------//
        if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_130_flag == 1)
        {        

            FrameHold_130 = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 150: 90  ;  
            FrameHoldflag_130=1;
            //s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[rgnIdx]
                    
        }
        else if(FrameHold_130>0)
        {
            FrameHold_130 -- ;
                    
        }
        else if(FrameHoldflag_130)
        {
            FrameHoldflag_130=0;

        }
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg,  0,  3, FrameHold_130>0); 

        //---------------------------for 173 begin small and black fg ----------------------------//
        if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_173_begin_flag == 1)
        {        

            FrameHold_173_begin = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 150: 90  ;  
            FrameHoldflag_173_begin=1;
            WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,pred_old_diff_cut);
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,bad_old_cut);
            //WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,0x0); //bg change en
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xb); //pred sun cut th
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,10,13,0x5);//reg_dh_pred_x_range 
            WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x14); //color th0 max
            WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,31,31,0x0); //apl_s2_protect_extend_en  
                    
        }
        else if(FrameHold_173_begin>0)
        {
            FrameHold_173_begin -- ;
            WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,pred_old_diff_cut);
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,bad_old_cut);
            //WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,0x0); //bg change en
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xb); //pred sun cut th
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,10,13,0x5);//reg_dh_pred_x_range 
            WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x14); //color th0 max
            WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,31,31,0x0); //apl_s2_protect_extend_en  
                    
        }
        else if(FrameHoldflag_173_begin)
        {
            FrameHoldflag_173_begin=0;
            WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,pred_old_diff_cut_init);
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,bad_old_cut_init);
            //WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,0x1); //bg change en
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x14); //pred sun cut th
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,10,13,0x9);//reg_dh_pred_x_range 
            WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x22); //color th0 max
            WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,31,31,0x1); //apl_s2_protect_extend_en  

        }
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg,  0,  3, FrameHold_173_begin>0);

        //---------------------------for 227 2 scene ----------------------------//
        if(s_pContext->_output_frc_sceneAnalysis.u8_dh_gmv_valchange_227_2 == 1)
        {        

            FrameHold_227_2 = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 10: 5  ;  
            FrameHoldflag_227_2=1;
            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xc); //filter ucov cut
            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0xc); //filter cov cut
            WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,10,11,pred_en_0);
            WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,16,16,0x1); //pred me1 diff cut en
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,1,1,0x0); // hor en
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x1); // ver en
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x1e); //pred sun cut th
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x0); //old pred en
            WriteRegister(KME_DEHALO5_PHMV_FIX_34_reg,16,23,0x19); //ver pred y0
            WriteRegister(KME_DEHALO5_PHMV_FIX_34_reg,24,31,0x19); //ver pred y1
                    
        }
        else if(FrameHold_227_2>0)
        {
            FrameHold_227_2 -- ;
            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xc);//filter ucov cut
            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0xc);//filter cov cut
            WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,10,11,pred_en_0);
            WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,16,16,0x1); //pred me1 diff cut en
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,1,1,0x0); // hor en
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x1); // ver en
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x1e); //pred sun cut th
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x0); //old pred en
            WriteRegister(KME_DEHALO5_PHMV_FIX_34_reg,16,23,0x19); //ver pred y0
            WriteRegister(KME_DEHALO5_PHMV_FIX_34_reg,24,31,0x19); //ver pred y1
                    
        }
        else if(FrameHoldflag_227_2)
        {
            FrameHoldflag_227_2=0;
            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,occl_filter_cov_cut_init);
            WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,10,11,pred_en_init);
            WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,16,16,0x0); //pred me1 diff cut en
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,1,1,0x1); // hor en
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x0); // ver en
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x14); //pred sun cut th
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x1); //old pred en
            WriteRegister(KME_DEHALO5_PHMV_FIX_34_reg,16,23,0x5); //ver pred y0
            WriteRegister(KME_DEHALO5_PHMV_FIX_34_reg,24,31,0xf); //ver pred y1

        }
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg,  0,  3, FrameHold_227_2>0);

        //---------------------------for 227 4 scene ----------------------------//
        if(s_pContext->_output_frc_sceneAnalysis.u8_dh_gmv_valchange_227_4 == 1)
        {        

            FrameHold_227_4 = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 10: 5  ;  
            FrameHoldflag_227_4=1;
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xb); //pred sun cut th
            WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x10); //color th0 max
            WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0x50); //extend mvdiff th
                    
        }
        else if(FrameHold_227_4>0)
        {
            FrameHold_227_4 -- ;
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xb); //pred sun cut th
            WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x10); //color th0 max
            WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0x50); //extend mvdiff th
                    
        }
        else if(FrameHoldflag_227_4)
        {
            FrameHoldflag_227_4=0;
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x14); //pred sun cut th
            WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x22); //color th0 max
            WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0x0); //extend mvdiff th

        }
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg,  0,  3, FrameHold_227_4>0);

        //---------------------------for 227 5 scene ----------------------------//
        if(s_pContext->_output_frc_sceneAnalysis.u8_dh_gmv_valchange_227_5 == 1)
        {        

            FrameHold_227_5 = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 10: 5  ;  
            FrameHoldflag_227_5=1;
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,1,1,0x1); // hor en
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x0); // ver en
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,3,0x0); // pos en
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,4,4,0x0); // neg en
            WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0xa);//extend cov th0
            WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x1);  //extend cond4 en
            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,0x0); //black protect en
            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,color_protect_th1_en_init);
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xa); //pred sun cut th
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x1); //old pred en
            WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
            WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x12c); //extend4 posdiff
            WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0x20); //extend mvdiff th
            WriteRegister(KME_ME1_BG0_ME1_BG_3_reg, 0, 0, 0x0); //me1 black protect        
            WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,0,0,0x0); //me1 black protect en
                    
        }
        else if(FrameHold_227_5>0)
        {
            FrameHold_227_5 -- ;
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,1,1,0x1); // hor en
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x0); // ver en
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,3,0x0); // pos en
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,4,4,0x0); // neg en
            WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0xa);//extend cov th0
            WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x1);  //extend cond4 en
            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,0x0); //black protect en
            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,color_protect_th1_en_init);
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xa); //pred sun cut th
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x1); //old pred en
            WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
            WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x12c); //extend4 posdiff
            WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0x20); //extend mvdiff th
            WriteRegister(KME_ME1_BG0_ME1_BG_3_reg, 0, 0, 0x0); //me1 black protect        
            WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,0,0,0x0); //me1 black protect en
                    
        }
        else if(FrameHoldflag_227_5)
        {
            FrameHoldflag_227_5=0;
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,1,1,0x1); // hor en
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x0); // ver en
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,3,0x1); // pos en
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,4,4,0x1); // neg en
            WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0_init);
            WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x1);  //extend cond4 en
            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,color_protect_th0_en_init);
            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,color_protect_th1_en_init);
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x14); //pred sun cut th
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x1); //old pred en
            WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x12c); //extend posdiff th
            WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x2bc); //extend4 posdiff
            WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0x0); //extend mvdiff th
            WriteRegister(KME_ME1_BG0_ME1_BG_3_reg, 0, 0, 0x1); //me1 black protect        
            WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,0,0,0x1); //me1 black protect en

        }
		   //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg,	0,	3, FrameHold_227_5>0);
		
		//---------------------------for 156 ----------------------------//
		if(fwcontrol_156)
		   {		
			   FrameHold_156 = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 10: 5  ;	
			   FrameHoldflag_156=1;    
			   WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x0); //logo protect
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x0); // ver en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xb); //pred sun cut th
			   //WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x1);  //penalty_en
			   //WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x1); //divide en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0xa); //extend mvdiff th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0_init);
		   }
		   else if(FrameHold_156>0)
		   {
			   FrameHold_156 -- ;
			   WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x0); //logo protect
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x0); // ver en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xb); //pred sun cut th
			   //WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x1);  //penalty_en
			   //WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x1); //divide en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0x0); //extend mvdiff th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0_init);
					   
		   }
		   else if(FrameHoldflag_156)
		   {
			   FrameHoldflag_156=0;
			   WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x1); //logo protect
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x0); // ver en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x14); //pred sun cut th
			   //WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x0);  //penalty_en
			   //WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x0); //divide en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0x0); //extend mvdiff th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0_init);
		   }
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg,  0,  3, FrameHold_156>0); 
		
		//---------------------------for 156 final ----------------------------//
		if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_156_final_flag)
		   {		
			   FrameHold_156_final = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 80: 40	;  
			   FrameHoldflag_156_final=1;	 
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,4,4,0x0); // neg en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x1); //old pred en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,bg_change_cut_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x1f4); //judge th1
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x12c); //extend4 posdiff
			   WriteRegister(KME_DEHALO5_PHMV_FIX_7_reg,8,12,0xc); //filter ucov th0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,5,5,0x1); //small judge en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x1);  //penalty_en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x1); //divide en
		   }
		   else if(FrameHold_156_final>0)
		   {
			   FrameHold_156_final -- ;
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,4,4,0x0); // neg en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x1); //old pred en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,bg_change_cut_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x1f4);  //judge th1
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x12c); //extend4 posdiff
			   WriteRegister(KME_DEHALO5_PHMV_FIX_7_reg,8,12,0xc); //filter ucov th0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,5,5,0x1); //small judge en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x1);  //penalty_en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x1); //divide en
					   
		   }
		   else if(FrameHoldflag_156_final)
		   {
			   FrameHoldflag_156_final=0;
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,4,4,0x1); // neg en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x1); //old pred en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,bg_change_cut_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x1f4);  //judge th1
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x12c); //extend posdiff th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x2bc); //extend4 posdiff
			   WriteRegister(KME_DEHALO5_PHMV_FIX_7_reg,8,12,0xf); //filter ucov th0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,5,5,0x0); //small judge en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x0);  //penalty_en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x0); //divide en
		   }
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg,  0,  3, FrameHold_156_final>0); 
		
		//---------------------------for 431 ----------------------------//
		if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_431_halo_flag)
		   {		
			   FrameHold_431 = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 160: 80  ;  
			   FrameHoldflag_431=1;   
			   WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x0); //logo protect
			   WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,pred_th0_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,26,30,pred_th1_init);
		   }
		   else if(FrameHold_431>0)
		   {
			   FrameHold_431 -- ;
			   WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x0); //logo protect
			   WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,pred_th0_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,26,30,pred_th1_init);
					   
		   }
		   else if(FrameHoldflag_431)
		   {
			   FrameHoldflag_431=0;
			   WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x1); //logo protect
			   WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,pred_th0_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,26,30,pred_th1_init);
		   }
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg,  0,  3, FrameHold_431>0);
		
		
		   //---------------------------for 96 begin ----------------------------//
		   if(s_pContext->_output_frc_sceneAnalysis.u8_dh_gmv_valchange_96 == 1)
		   {		
		
			   FrameHold_96_begin = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 10: 5  ;  
			   FrameHoldflag_96_begin=1;
		
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xb); //pred sun cut th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,1,1,0x0); // hor en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x1); // ver en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,3,0x0); // pos en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,4,4,0x0); // neg en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_34_reg,16,23,0xa); // pred ver y0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x12c); //extend4 posdiff
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,8,14,bg_extend_cov_th1_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,bg_extend_ucov_th0_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,24,30,bg_extend_ucov_th1_init);
					   
		   }
		   else if(FrameHold_96_begin>0)
		   {
			   FrameHold_96_begin -- ;
		
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xb); //pred sun cut th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,1,1,0x0); // hor en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x1); // ver en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,3,0x0); // pos en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,4,4,0x0); // neg en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_34_reg,16,23,0xa); // pred ver y0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x12c); //extend4 posdiff
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,8,14,bg_extend_cov_th1_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,bg_extend_ucov_th0_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,24,30,bg_extend_ucov_th1_init);
					   
		   }
		   else if(FrameHoldflag_96_begin)
		   {
			   FrameHoldflag_96_begin=0;
		
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x14); //pred sun cut th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,1,1,0x1); // hor en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x0); // ver en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,3,0x1); // pos en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,4,4,0x1); // neg en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_34_reg,16,23,0x5); // pred ver y0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x12c); //extend posdiff th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x2bc); //extend4 posdiff
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,8,14,bg_extend_cov_th1_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,bg_extend_ucov_th0_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,24,30,bg_extend_ucov_th1_init);
		
		   }
		   //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg,	0,	3, FrameHold_96_begin>0); 
		
		   //---------------------------for 96 end ----------------------------//
		   if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_96_2_flag == 1)
		   {		
		
			   FrameHold_96_end = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 10: 5	;  
			   FrameHoldflag_96_end=1;
		
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xb); //pred sun cut th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,1,1,0x1); // hor en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x0); // ver en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,3,0x0); // pos en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,4,4,0x1); // neg en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,8,14,bg_extend_cov_th1_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,bg_extend_ucov_th0_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,24,30,bg_extend_ucov_th1_init);
					   
		   }
		   else if(FrameHold_96_end>0)
		   {
			   FrameHold_96_end -- ;
		
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xb); //pred sun cut th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,1,1,0x0); // hor en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x1); // ver en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,3,0x0); // pos en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,4,4,0x0); // neg en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_34_reg,16,23,0x8); // pred ver y0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,8,14,bg_extend_cov_th1_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,bg_extend_ucov_th0_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,24,30,bg_extend_ucov_th1_init);
					   
		   }
		   else if(FrameHoldflag_96_end)
		   {
			   FrameHoldflag_96_end=0;
		
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x14); //pred sun cut th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,1,1,0x1); // hor en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x0); // ver en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,3,0x1); // pos en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,4,4,0x1); // neg en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,8,14,bg_extend_cov_th1_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,bg_extend_ucov_th0_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,24,30,bg_extend_ucov_th1_init);
		
		   }
		   //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg,	0,	3, FrameHold_96_end>0);
		
		   //---------------------------49 halo ----------------------------//
		   if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_49_halo_flag == 1)
		   {		
		
			   FrameHold_49 = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 160: 80  ;  
			   FrameHoldflag_49=1;
		
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,color_protect_th1_en_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xa); //pred sun cut th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x1f4); //extend4 posdiff
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,occl_filter_cov_cut_init);
					   
		   }
		   else if(FrameHold_49>0)
		   {
			   FrameHold_49 -- ;
		
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,color_protect_th1_en_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xa); //pred sun cut th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x1f4); //extend4 posdiff
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,occl_filter_cov_cut_init);
					   
		   }
		   else if(FrameHoldflag_49)
		   {
			   FrameHoldflag_49=0;
		
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,color_protect_th0_en_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,color_protect_th1_en_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x14); //pred sun cut th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x12c); //extend posdiff th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x2bc); //extend4 posdiff
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,occl_filter_cov_cut_init);
		
		   }
		   //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg,	0,	3, FrameHold_49>0);  
		
		   //---------------------------163 halo ----------------------------//
		   if(s_pContext->_output_frc_sceneAnalysis.u8_dh_gmv_valchange_163 == 1)
		   {		
		
			   FrameHold_163 = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 300: 150	;  
			   FrameHoldflag_163=1;
		
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,color_protect_th1_en_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xa); //pred sun cut th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x1f4); //extend4 posdiff
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0x8);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0x8);
			   //WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,0x0);	//bg change en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,20,20,0x1); //color en4
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,0,7,0x0); //color min4
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,8,15,0x50); //color max4
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,10,13,0x7); //pred range x
					   
		   }
		   else if(FrameHold_163>0)
		   {
			   FrameHold_163 -- ;
		
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,color_protect_th1_en_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xa); //pred sun cut th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x1f4); //extend4 posdiff
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0x8);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0x8);
			   //WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,0x0);	//bg change en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,20,20,0x1); //color en4
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,0,7,0x0); //color min4
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,8,15,0x50); //color max4
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,10,13,0x7); //pred range x
					   
		   }
		   else if(FrameHoldflag_163)
		   {
			   FrameHoldflag_163=0;
		
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,color_protect_th1_en_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x14); //pred sun cut th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x12c); //extend posdiff th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x2bc); //extend4 posdiff
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,occl_filter_cov_cut_init);
			   //WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,0x1);	//bg change en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,20,20,0x0); //color en4
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,0,7,0x0); //color min4
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,8,15,0xff); //color max4
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,10,13,0x9); //pred range x
		
		   }
		   //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg,	0,	3, FrameHold_163>0);
		
		   //---------------------------183 halo ----------------------------//
		   if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_183_face_broken == 1)
		   {		
		
			   FrameHold_183 = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 300: 150	;  
			   FrameHoldflag_183=1;
		
			   WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x0); //logo protect
					   
		   }
		   else if(FrameHold_183>0)
		   {
			   FrameHold_183 -- ;
		
			   WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x0); //logo protect
					   
		   }
		   else if(FrameHoldflag_183)
		   {
			   FrameHoldflag_183=0;
		
			   WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x1); //logo protect
		
		   }
		   //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg,	0,	3, FrameHold_183>0);
		   
		   //---------------------------419 ----------------------------//
		   if(s_pContext->_output_frc_sceneAnalysis.u8_me1_condition_419_flag == 1)
		   {		
		   
			   FrameHold_419 = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 200: 100	;  
			   FrameHoldflag_419=1;
		   
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xf);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0x14);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0x28); //extend cov th0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,8,14,0x2); //extend cov th1
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0x14); //extend ucov th0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,24,30,0x2); //extend ucov th1
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,3,0x0); // pos en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,4,4,0x0); // neg en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,10,13,0x7); //pred range x
			   WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,12,12,0x0); //only mv en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,2,2,0x0); //mv filter
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x0);  //extend cond4 en
					   
		   }
		   else if(FrameHold_419>0)
		   {
			   FrameHold_419 -- ;
		   
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xf);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0xf);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0x28); //extend cov th0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,8,14,0x2); //extend cov th1
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0x14); //extend ucov th0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,24,30,0x2); //extend ucov th1
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,3,0x0); // pos en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,4,4,0x0); // neg en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,10,13,0x7); //pred range x  
			   WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,12,12,0x0); //only mv en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,2,2,0x0); //mv filter
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x0);  //extend cond4 en
		   }
		   else if(FrameHoldflag_419)
		   {
			   FrameHoldflag_419=0;
		   
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,occl_filter_cov_cut_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,8,14,bg_extend_cov_th1_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,bg_extend_ucov_th0_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,24,30,bg_extend_ucov_th1_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,3,0x1); // pos en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,4,4,0x1); // neg en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,10,13,0x9); //pred range x
			   WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,12,12,0x1); //only mv en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,2,2,0x1); //mv filter
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x1);  //extend cond4 en
		   }
		   
		   //---------------------------pq issue pattern ----------------------------//
		   if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_PQ_Issue == 1)
		   {		
		   
			   FrameHold_PQ_Issue = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 60: 30  ;  
			   FrameHoldflag_PQ_Issue=1;
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,0x0); //black protect
			   WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x258);//judge th1
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,10,13,0x7); //pred range x
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xa); //pred sun cut th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x12c); //extend4 posdiff
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0x0); //filter ucov cut
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0x0); //filter cov cut
			   WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,0x0);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,7,7,0x0);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,0x0);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,0x0);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,0x0);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,0,0,0x0); //bad_pred_diff_cut_en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,0,6,0x57);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1F_reg,0,6,0x57); //extend min th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1A_reg,26,31,0x0); //correct cut th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,0x7);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,0x7);
			   //WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,25,29,0x12); //mv filter th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0x8);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,8,14,bg_extend_cov_th1_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0xa);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,24,30,bg_extend_ucov_th1_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,pred_ver_en_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,old_pred_en_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,20,20,0x1); //color en4
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,0,7,0x2d); //color min4
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,8,15,0xaa); //color max4
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,16,20,0xd); //filter cov th0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x8); //pred sun cut th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,24,29,0x8); //old sun cut th
					   
		   }
		   else if(FrameHold_PQ_Issue>0)
		   {
			   FrameHold_PQ_Issue -- ;
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,0x0);  //black protect
			   WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x258); //judge th1
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,10,13,0x7); //pred range x
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xa); //pred sun cut th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x12c); //extend4 posdiff
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0x0); //filter ucov cut
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0x0); //filter cov cut
			   WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,0x0);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,7,7,0x0);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,0x0);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,0x0);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,0x0);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,0,0,0x0); //bad_pred_diff_cut_en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,0,6,0x57);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1F_reg,0,6,0x57); //extend min th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1A_reg,26,31,0x0); //correct cut th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,0x7);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,0x7);
			   //WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,25,29,0x12); //mv filter th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0x8);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,8,14,bg_extend_cov_th1_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0xa);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,24,30,bg_extend_ucov_th1_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,pred_ver_en_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,old_pred_en_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,20,20,0x1); //color en4
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,0,7,0x2d);//color min4
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,8,15,0xaa); //color max4
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,16,20,0xd); //filter cov th0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x8); //pred sun cut th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,24,29,0x8); //old sun cut th
		   
		   }
		   else if(FrameHoldflag_PQ_Issue)
		   {
			   FrameHoldflag_PQ_Issue=0;
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,0x1);  //black protect
			   WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x384); //judge th1
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,10,13,0x9); //pred range x
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x14); //pred sun cut th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x12c); //extend posdiff th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x2bc); //extend4 posdiff
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,occl_filter_cov_cut_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,pred_old_diff_cut_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,7,7,bad_pred_cut_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,bad_old_cut_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,bg_change_cut_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,pred_bad_cut_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,0,0,0x1); //bad_pred_diff_cut_en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,0,6,bg_useless_fix_bg_th_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1F_reg,0,6,0x52); //extend min th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1A_reg,26,31,0x4); //correct cut th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,gmv_th_y0_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,pred_y0_init); 
			   //WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,25,29,0x14); //mv filter th 
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,8,14,bg_extend_cov_th1_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,bg_extend_ucov_th0_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,24,30,bg_extend_ucov_th1_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,pred_ver_en_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,old_pred_en_init);
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,20,20,0x0); //color en4
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,0,7,0x0); //color min4
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,8,15,0xff); //color max4
			   WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,16,20,0xf); //filter cov th0
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x14); //pred sun cut th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,24,29,0x28); //old sun cut th
		   
		   }

	if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_equalbgcnt_bigmvx == 1)
	{
		FrameHoldflag_equallbgcnt = 1;
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,0x0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,0x0);
	
		
	}
	else if(FrameHoldflag_equallbgcnt>0)
	{
		FrameHoldflag_equallbgcnt = 0;
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,0x1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,0x1);

		
	}





		
        //-----------------------------128 --------------------------------//
        auto_extend_mvdiff_thfor128 = (_ABS_DIFF_(gmv_fgx,gmv_bgx) > 100) ? 20 : _ABS_DIFF_(gmv_fgx,gmv_bgx)/5;
        if(fwcontrol_128)
        {        

            FrameHold_128 = 5  ;  
            FrameHoldflag_128=1;
            WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x0);
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x0);        
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x0);
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x0);  
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x0); 
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x0);        
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x0);
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x0);
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x0);
            WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x1);  //extend cond3 en
            WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x1);  //extend cond4 en
            WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0xf); //color th0 max
            WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xe); //ucov cut
            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0xc); //cov cut
            WriteRegister(KME_DEHALO5_PHMV_FIX_1A_reg,26,31,0x0); //correct cut
            WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0x19); //extend ucov th0
            WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,0x1e); //pred y0
            WriteRegister(KME_DEHALO5_PHMV_FIX_28_reg,0,9,0x50); //pred y1
            WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,0x0);
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,7,7,0x0);
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,0x0);
            WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,10,11,pred_en_0);
            WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,0,0,0x0);
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,1,1,0x1); // hor en
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x0); // ver en
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,3,0x1); // pos en
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,4,4,0x1); // neg en
            WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0_init);
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x14); //pred sun cut th
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x1); //old pred en
            WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23, auto_extend_mvdiff_thfor128);
                    
        }
        else if(FrameHold_128>0)
        {
            FrameHold_128 -- ;
            WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x0);
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x0);        
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x0);
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x0);  
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x0); 
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x0);        
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x0);
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x0);
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x0);
            WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x1);  //extend cond3 en
            WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x1);  //extend cond4 en
            WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0xf); //color th0 max
            WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xe);//ucov cut
            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0xc); //cov cut  
            WriteRegister(KME_DEHALO5_PHMV_FIX_1A_reg,26,31,0x0); //correct cut
            WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0x19);  //extend ucov th0
            WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,0x1e); //pred y0
            WriteRegister(KME_DEHALO5_PHMV_FIX_28_reg,0,9,0x50); //pred y1
            WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,0x0);
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,7,7,0x0);
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,0x0);
            WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,10,11,pred_en_0);
            WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,0,0,0x0);
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,1,1,0x1); // hor en
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x0); // ver en
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,3,0x1); // pos en
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,4,4,0x1); // neg en
            WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0_init);
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x14); //pred sun cut th
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x1); //old pred en
            WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23, auto_extend_mvdiff_thfor128);
        }
        else if(FrameHoldflag_128)
        {
            FrameHoldflag_128=0;
            WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x1);
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x0);        
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x0);
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x0);  
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x0); 
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x0);        
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x0);
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x0);
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x0);
            WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x1);  //extend cond3 en
            WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x1);  //extend cond4 en
            WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x22); //color th0 max
            WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x12c); //extend posdiff th
            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
            WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,occl_filter_cov_cut_init);
            WriteRegister(KME_DEHALO5_PHMV_FIX_1A_reg,26,31,0x4); //correct cut
            WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,bg_extend_ucov_th0_init);
            WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,0x14); //pred y0
            WriteRegister(KME_DEHALO5_PHMV_FIX_28_reg,0,9,0x3c); //pred y1
            WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,pred_old_diff_cut_init);
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,7,7,bad_pred_cut_init);
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,bad_old_cut_init);
            WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,10,11,pred_en_init);
            WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,0,0,0x1);
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,1,1,0x1); // hor en
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x0); // ver en
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,3,0x1); // pos en
            WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,4,4,0x1); // neg en
            WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0_init);
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x14); //pred sun cut th
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x1); //old pred en
            WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23, 0x0);

        }
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg,  0,  3, FrameHold_128>0); 

		//--------------------  eagle  ---------------------------------//

	if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_eagle_status == 1)
	{
	  	video_eagle = 1;
		WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x0); //logo protect
		WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,0,6, 0x57); //useless max th
		WriteRegister(KME_DEHALO5_PHMV_FIX_1F_reg,0,6,0x56);  //bg min
		WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0xf); //extend ucov th0
		WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0x5); //extend cov th0
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x5); //pred sun cut th
		WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,10,11,0x0); //pred en
		WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
		WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,1,1,0x0); // hor en
		WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x1); // ver en
		WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,3,0x0); // pos en
		WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,4,4,0x0); // neg en
		WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0x8); //extend mvdiff th
		WriteRegister(KME_DEHALO5_PHMV_FIX_34_reg,16,23,0xf); //ver pred y0
		WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0x9);
		WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0x9); //filter cov cut

		WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,18);
		WriteRegister(KME_DEHALO5_PHMV_FIX_7_reg,8,12,12);
	  }
	  else if(video_eagle)
	  {
	  	video_eagle = 0;
		WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x1); //logo protect
		WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,0,6, 0x54); //useless max th
		WriteRegister(KME_DEHALO5_PHMV_FIX_1F_reg,0,6,0x54);  //bg min
		WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0xc); //extend ucov th0
		WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0xc); //extend cov th0
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x14); //pred sun cut th
		WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,10,11,0x2); //pred en
		WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x12c); //extend posdiff th
		WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,1,1,0x1); // hor en
		WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x0); // ver en
		WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,3,0x1); // pos en
		WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,4,4,0x1); // neg en
		WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0x0); //extend mvdiff th
		WriteRegister(KME_DEHALO5_PHMV_FIX_34_reg,16,23,0x5); //ver pred y0
		WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0x4); //filter cov cut

		WriteRegister(KME_DEHALO5_PHMV_FIX_7_reg,8,12,0xf);
	  }


	//---------------------------------  C005 plane -------------------------------//
	if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_C005plane_status == 1)
	{
	  	video_plane = 1;
		WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0x1e); //extend ucov th0
		WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0x1e); //extend cov th0
		WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,10,11,0x0); //pred en
		WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
		WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,1,1,0x0); // hor en
		WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x1); // ver en
		WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,3,0x0); // pos en
		WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,4,4,0x0); // neg en
		WriteRegister(KME_DEHALO5_PHMV_FIX_34_reg,16,23,0xf); //ver pred y0
		WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0xa);
		WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0xf);
		WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,12,12,0x0); //only mv
		WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,15,15,0x0); //only occl
	  }
	  else if(video_plane)
	  {
	  	video_plane = 0;
		WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0xc); //extend ucov th0
		WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0xc); //extend cov th0
		WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,10,11,0x2); //pred en
		WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x12c); //extend posdiff th
		WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,1,1,0x1); // hor en
		WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x0); // ver en
		WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,3,0x1); // pos en
		WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,4,4,0x1); // neg en
		WriteRegister(KME_DEHALO5_PHMV_FIX_34_reg,16,23,0x5); //ver pred y0
		WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,occl_filter_cov_cut_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,12,12,0x1); //only mv
		WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,15,15,0x1); //only occl

	  }


	if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_smallplane_status == 1)
	{
		video_smallplane =1 ;
		WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0x28); 
		WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0xf); 

	}
	else if(video_smallplane)
	{
		video_smallplane = 0 ;
		WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0_init); 
		WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0x4); 
	}



	if((s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_transporter_status == 1)&&(s_pContext->_output_frc_sceneAnalysis.u8_dh_conditio_0026_1_status == 0)&&(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_skating_status==0)&&(s_pContext->_output_frc_sceneAnalysis.u8_dh_conditio_3003man_status==0))
	{
		video_transporter =1 ;
		WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x0); //logo protect
		WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
		WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x12c); //extend4 posdiff
		WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,16,20,0xc); //filter cov th0
		WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,0); //white protect
		

	}
	else if(video_transporter)
	{

		video_transporter = 0 ;
		WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x1); //logo protect
		WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x12c); //extend posdiff th
		WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x2bc); //extend4 posdiff
		WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,16,20,0xf); //filter cov th0
		WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,1);

	}

if(s_pContext->_external_data._output_frameRate == _PQL_OUT_100HZ)
{
	if((s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_transporter_status == 1)&&(s_pContext->_output_frc_sceneAnalysis.u8_dh_conditio_0026_1_status == 0)&&(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_skating_status==0)&&(s_pContext->_output_frc_sceneAnalysis.u8_dh_conditio_3003man_status==0))
	{
		video_transporter_50hz = 1;
		WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x1); // extend cond4
		WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x1);//reg_dh_bg_extend_cond3_en 
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x1);//old_occl_en
		WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,0x1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,7,7,0x1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,0x1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,16,16,0x1);

		WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,0x1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,65);  //reg_dh_color_protect_th0_max 


	}
	else if(video_transporter_50hz)
	{
		video_transporter_50hz = 0;
		WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x1); // extend cond4
		WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x1);//reg_dh_bg_extend_cond3_en
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,0x1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,7,7,0x1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,0x1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,16,16,0x0);

		WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,0x1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x22);  //reg_dh_color_protect_th0_max 
	}


}


	if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_dance_status == 1)
	{
		if(u11_gmv_mvx<20)
		{
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x5); //pred sun cut th
			WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
			WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x12c); //extend4 posdiff
			WriteRegister(KME_DEHALO5_PHMV_FIX_7_reg,8,12,0xb); //filter ucov th0
			WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x258); //judge th1
		}
		video_dance = 1 ;
	}
	else if(video_dance)
	{

		video_dance = 0 ;
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x14); //pred sun cut th
		WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x12c); //extend posdiff th
		WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x2bc); //extend4 posdiff
		WriteRegister(KME_DEHALO5_PHMV_FIX_7_reg,8,12,0xf); //filter ucov th0
		WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x384); //judge th1	
	}

	if((s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_skating_status == 1)&&(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_huapi2_status==0))
	{
		WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,0x0); //black protect en
		//WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
		//WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x12c); //extend4 posdiff
		WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x0); //logo protect en
		WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,0x0); //bg change en
		WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,0x0);//bad pred cut
		WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,10,13,0x7);//reg_dh_pred_x_range
	
		video_skating = 1 ;
	}
	else if(video_skating)
	{
		video_skating = 0 ;
		WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,0x1); //black protect en
		//WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x12c); //extend posdiff th
		//WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x2bc); //extend4 posdiff
		WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x1); //logo protect en
		WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,0x1); //bg change en
		WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,0x1);//bad pred cut
		WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,10,13,0x9);//reg_dh_pred_x_range	
	}



	//---------------------- 30006 ----------------------------//
	if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_30006_status == 1)
	{
		WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,0x0);
		video30006 = 1 ;
	}
	else if(video30006)
	{
		video30006 = 0 ;
		WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,0x1);

	}

	   
	   //--------------------- rotterdam 1---------------------------//
	   if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_rotterdam_status == 1)
	   {
		   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xa); //pred sun cut th
		   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,24,29,0xa); //old sun cut th
		   WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x190); //extend4 posdiff
		   WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x258); //judge th1
		   WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,0x0); //bg change en
		   WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,0x0); //bad pred cut en
		   video_rotterdam = 1 ;
	   }
	   else if(video_rotterdam)
	   {
		   video_rotterdam = 0 ;
		   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x14); //pred sun cut th
		   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,24,29,0x28); //old sun cut th
		   WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x2bc); //extend4 posdiff
		   WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x384); //judge th1
		   WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,0x1); //bg change en
		   WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,0x1); //bad pred cut en
	   }
	   
	   
	   //------------------------------- ride bike girl --------------------------------//
	   
	   if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_ridebikegirl_status == 1)
	   {
		   video_ridebike = 1;
		   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xa); //pred sun cut th
		   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,24,29,0xa); //old sun cut th
		   WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x12c); //extend4 posdiff
		   WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x258); //judge th1
		   WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x0); //logo protect
		   WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x1); //penalty_en
		   WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x1); //divide en
		   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x1); //old pred en
		   WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0xa); //extend mvdiff th
		   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0xa);
		   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0xa);
		   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,10,13,0x7); //pred range x
		   WriteRegister(KME_DEHALO5_PHMV_FIX_7_reg,8,12,0xc); //filter ucov th0
	   }
	   else if(video_ridebike>0)
	   {
		   video_ridebike = 0;
		   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x14); //pred sun cut th
		   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,24,29,0x28); //old sun cut th
		   WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x2bc); //extend4 posdiff
		   WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x384); //judge th1
		   WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x1); //logo protect
		   WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x0); //penalty_en
		   WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x0); //divide en
		   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x1); //old pred en
		   WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23,0x0); //extend mvdiff th
		   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0_init);
		   WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,bg_extend_ucov_th0_init);
		   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,10,13,0x9); //pred range x
		   WriteRegister(KME_DEHALO5_PHMV_FIX_7_reg,8,12,0xf); //filter ucov th0
	   
	   }
	   
	   
	   
		   //--------------------------- man ride bike 30003 -------------------------//
		   if(s_pContext->_output_frc_sceneAnalysis.u8_dh_conditio_3003man_status == 1)
		   {
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,0x0); //th0_en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,0x0); 
			   WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,0x0); //bad_cut_en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,12);//th_y0 
			   WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,0x0); 
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,7,7,0x0); 
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,0x0); 
			   WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,0,0,0x0); 
	   
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,10); 
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,24,29,10); 
	   
			   video_30003man = 1;
	   
			   
	   
		   }
		   else if(video_30003man>0)
		   {
			   WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,pred_y0_init); 
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x14); //pred sun cut th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,24,29,0x28); //old sun cut th
	   
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,0x1); //th0_en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,0x1); 
			   WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,0x1); //bad_cut_en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,0x1); 
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,7,7,0x1); 
			   WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,0x1); 
			   WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,0,0,0x1); 
			   video_30003man = 0;
		   }
	   
	   
	   
	   
		   //---------------  30003 2---------------------//
	   
		   if(s_pContext->_output_frc_sceneAnalysis.u8_dh_conditio_toleft_bike_status == 1)
		   {
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,0x0); //th0_en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,0x0); //bg_change_cond_en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,0x0); //bad_cut_en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x0); 
	   
			   
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,10,13,0x5); //predflag_x_range
			   
	   
			   vdieo_toleftbike = 1;
	   
		   }
		   else if(vdieo_toleftbike>0)
		   {
			   WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,0x1); //th0_en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,0x1); 
			   WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,0x1); //bad_cut_en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,13,13,0x1);
	   
			   WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,10,13,0x9); //predflag_x_range
	   
			   vdieo_toleftbike = 0;
		   }


	if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_insect_status == 1)
	{
		video_insect = 1;
		WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,0x0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,7,7,0x0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,0x0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,16,16,0x0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,0,0,0x0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,10,13,15);

		WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,30);
		WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,8,14,18);
		WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,8);
		WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,24,30,19);

		WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,300);

		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,10);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,24,29,10);

		WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,3,0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,4,4,0);

		WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,30);
		WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,30);
		
		WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,8);
		WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,4);


		
	}
	else if(video_insect>0)
	{
		video_insect = 0;

		WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,0x1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,7,7,0x1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,0x1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,16,16,0x0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,0,0,0x1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,10,13,0);

		WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0xc);
		WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,8,14,5);
		WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0xc);
		WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,24,30,5);

		WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x2bc);

		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x14);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,24,29,0x28);

		WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,3,1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,4,4,1);

		WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,0x15);
		WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,0x14);
		
		WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,4);
		WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,4);


		
	}
	   
	   
	if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_huapi2_status == 1)
	{
		video_huapi2 = 1;
		WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,10);
	}
	else if(video_huapi2>0)
	{
		video_huapi2 = 0;
		WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x22);//reg_dh_color_protect_th0_max 
	}   
		if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_IDT_flag == 1)
        {

            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,0x1);

            WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg, 16, 25,   0x2  );//0xb809EB94
            WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg, 0, 11,   0x32 );//0xb809EB0C
            WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg, 24, 24,    0x1);
            WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg, 4,  4,   0x1);
            WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg, 16, 25,   0x3ff);
            WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 0, 6,   0x5   );
            WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 8, 14,   0xc    );
            WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 16, 22,   0x5   );
            WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 24, 30,   0xc   );
            WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg, 4, 13,   0x32  );

            WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15,   0x0      );
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7,  7,   0x0     );
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6,  6,   0x0      );
            WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0,  0,   0x0     );

            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 16, 21,   0xa);
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 24, 29,    0xa);

            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg, 16, 16,  0x1);
            WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg, 0, 7,   0x0);
            WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg, 8, 15,  0x50);
            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg, 17, 17,  0x1);
            WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg, 16, 23,  0x82);
            WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg, 24, 31,  0xff);

            //	WriteRegister(0xb809d7c4, 31, 31, 0);
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg, 24, 26,  0x2);
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg, 16, 18,    0x2);
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg, 9, 11,    0x2);
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg, 2, 4,    0x2);


	      WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,0x15);
            //WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0);
            //WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0);
            //WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 0);
            //WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 0);
            video_IDT = 1;
        }
        else if(video_IDT>0)
        {
            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,0x0);//white protect
            
            WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,0x14);
            WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x384);
            WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0);
            WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0);
            WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x12c);
            
            WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,12);
            WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,8,14,5);
            WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,12);
            WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,24,30,5);
            
            WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x2bc);
            
            WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,1);
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,7,7,1);
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,1);
            
            WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,0,0,1);
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,24,29,0x28);
            WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x14);
            
            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,1);
            WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,0,7,0);
            WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x22);
            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,0);
            WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,16,23,150);
            WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,24,31,255);
            //WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,1);
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0);
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0);
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0);
            WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0);

	      WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,0x15);

            //WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 1);
            //WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 1);
            //WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 1);
            //WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 1);
            video_IDT = 0;
        }
        //---------------------------for 214 ----------------------------------//
        	if(fwcontrol_214 ||(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_214 == 1))
			{
				//WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,28,29,0x1); //pick bg mv
				WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x1);	//penalty_en
				WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x1); //divide en
				WriteRegister(KME_DEHALO5_PHMV_FIX_2B_reg,8,15,0x4F); //phase_th1
				WriteRegister(KME_DEHALO5_PHMV_FIX_2C_reg,16,23,0x32); //phase_th6
				WriteRegister(KME_DEHALO5_PHMV_FIX_2E_reg,0,7,0x23); //phase th12
				WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,0x0);//pred_old_diff_cut_en
				WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,0,0,0x0); //bad pred diff cut en
				WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,0x0); //bad pred cut en
				WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,24,29,0x29); //bad old cov
				WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,16,21,0x28); //bad pred cov
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xe); //pred sun cut th
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,8,13,0x29); //bad old ucov
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,0,5,0x28); //bad pred ucov
				//WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,0x9); //me1 pred th0
				WriteRegister(KME_DEHALO5_PHMV_FIX_1A_reg,26,31,0x0); //correct cut
				WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,10,13,0x0); //extend cut
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0xa); //extend ucov th0
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0xa);  //extend cov th0
				WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x19); // color protect th max0
				WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,2,2,0x1); // mv filter en
				WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x258); //posdiff th1
				WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,18,26,0xdf); //posdiff th2
				WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,16,24,0x1a4); //posdiff th3
				WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,0,6,0x55); //useless bg th
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,16,20,0xe); //filter cov th0
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0x2); //filter ucov cut
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0x2); //filter cov cut
				WriteRegister(KME_DEHALO5_PHMV_FIX_7_reg,8,12,0xe); //filter ucov th0
				WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x1); // ver en
				WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,8,9,0x1); //ver y range
				WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,0,7,0x14);  //pred y2
				WriteRegister(KME_DEHALO5_PHMV_FIX_43_reg,6,6,0x0); // blend
				WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,0x0);  //bg change en
				WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,18,18,0x0);  //bg sum en
				WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x1); // extend cond4
				WriteRegister(KME_DEHALO5_PHMV_FIX_1F_reg,8,14,0x57);  //bg max
				WriteRegister(KME_DEHALO5_PHMV_FIX_1F_reg,0,6,0x54);  //bg min
				WriteRegister(KME_DEHALO5_PHMV_FIX_34_reg,16,23,0x14);	//ver pre y0
				WriteRegister(KME_DEHALO5_PHMV_FIX_34_reg,24,31,0x19);	//ver pre y1
				WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23, 0x14);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25, 0x3ff);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,24,30, 0x64);
				WriteRegister(KME_DEHALO5_PHMV_FIX_10_reg,24,31, 0x3f);
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x0); //old pred en
				WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,12,12,0x0); //only mv
				WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,15,15,0x0); //only occl
		   
				if(s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ)
				{
					WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x0);
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x2);		
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x0);
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x2);  
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x0); 
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x3);		  
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x0);
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x2);
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x0);
					WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0x0); //filter cov cut
					WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,16,20,0xc); //filter cov th0
				}
				else
				{
					WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x0);
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x1);		
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x0);
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x1);  
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x0); 
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x1);		  
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x0);
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x1);
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x0);
				}
		   
						//fw init
				WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,10,11,0x2);
				WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,26,30,0x3); 
				WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,20,20,0x1); 
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,8,14,0x5);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,24,30,0x5);
				WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,16,16,0x0);
				WriteRegister(KME_DEHALO5_PHMV_FIX_9_reg,16,25,0x4b);  
				WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x1); 
				WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,0x1);
				WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,0x0);
				WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,6,6,0x0); 
				WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,0x15);
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x1);
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,7,7,0x1);
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,0x1);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23, 0x0);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,24,30, 0x57);
				WriteRegister(KME_DEHALO5_PHMV_FIX_F_reg,16,25, 0x12);
				WriteRegister(KME_DEHALO5_PHMV_FIX_11_reg,0,9, 0x23);
				WriteRegister(KME_DEHALO5_PHMV_FIX_11_reg,16,25, 0x3c); 
				WriteRegister(KME_DEHALO5_PHMV_FIX_10_reg,24,31, 0x0);
				WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,25,30, 0x8);
				WriteRegister(KME_DEHALO5_PHMV_FIX_36_reg,10,15, 0x8);
		   
				FrameHold_214 = 5  ;  
				FrameHoldflag_214=1;
				
			}
			else if(FrameHold_214>0)
			{
				//WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,28,29,0x1); //pick bg mv
				WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x1);	//penalty_en
				WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x1); //divide en
				WriteRegister(KME_DEHALO5_PHMV_FIX_2B_reg,8,15,0x4F); //phase_th1
				WriteRegister(KME_DEHALO5_PHMV_FIX_2C_reg,16,23,0x32); //phase_th6
				WriteRegister(KME_DEHALO5_PHMV_FIX_2E_reg,0,7,0x23); //phase th12
				WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,0x0);//pred_old_diff_cut_en
				WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,0,0,0x0); //bad pred diff cut en
				WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,0x0); //bad pred cut en
				WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,24,29,0x29); //bad old cov
				WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,16,21,0x28); //bad pred cov
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xe); //pred sun cut th
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,8,13,0x29); //bad old ucov
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,0,5,0x28); //bad pred ucov
				//WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,0x9); //me1 pred th0
				WriteRegister(KME_DEHALO5_PHMV_FIX_1A_reg,26,31,0x0); //correct cut
				WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,10,13,0x0); //extend cut
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0xa); //extend ucov th0
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0xa);  //extend cov th0
				WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x19); // color protect th max0
				WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,2,2,0x1); // mv filter en
				WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x258); //posdiff th1
				WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,18,26,0xdf); //posdiff th2
				WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,16,24,0x1a4); //posdiff th3
				WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,0,6,0x55); //useless bg th
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,16,20,0xe); //filter cov th0
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0x2); //filter ucov cut
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0x2); //filter cov cut
				WriteRegister(KME_DEHALO5_PHMV_FIX_7_reg,8,12,0xe); //filter ucov th0
				WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x1); // ver en
				WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,8,9,0x1); //ver y range
				WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,0,7,0x14);  //pred y2
				WriteRegister(KME_DEHALO5_PHMV_FIX_43_reg,6,6,0x0); // blend
				WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,0x0);  //bg change en
				WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,18,18,0x0);  //bg sum en
				WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x1); // extend cond4
				WriteRegister(KME_DEHALO5_PHMV_FIX_1F_reg,8,14,0x57);  //bg max
				WriteRegister(KME_DEHALO5_PHMV_FIX_1F_reg,0,6,0x54);  //bg min
				WriteRegister(KME_DEHALO5_PHMV_FIX_34_reg,16,23,0x14);	//ver pre y0
				WriteRegister(KME_DEHALO5_PHMV_FIX_34_reg,24,31,0x19);	//ver pre y1
				WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23, 0x14);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25, 0x3ff);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,24,30, 0x64);
				WriteRegister(KME_DEHALO5_PHMV_FIX_10_reg,24,31, 0x3f);
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x0);
				WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,12,12,0x0); //only mv
				WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,15,15,0x0); //only occl
		   
				if(s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ)
				{
					WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x0);
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x2);		
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x0);
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x2);  
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x0); 
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x3);		  
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x0);
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x2);
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x0);
					WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0x0); //filter cov cut
					WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,16,20,0xc); //filter cov th0
				}
				else
				{
					WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x0);
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x1);		
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x0);
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x1);  
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x0); 
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x1);		  
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x0);
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x1);
					WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x0);
				}
		   
						//fw init
				WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,10,11,0x2);
				WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,26,30,0x3); 
				WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,20,20,0x1); 
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,8,14,0x5);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,24,30,0x5);
				WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,16,16,0x0);
				WriteRegister(KME_DEHALO5_PHMV_FIX_9_reg,16,25,0x4b);  
				WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x1); 
				WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,0x1);
				WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,0x0);
				WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,6,6,0x0); 
				WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,0x15);
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,0x1);
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,7,7,0x1);
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,0x1);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23, 0x0);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,24,30, 0x57);
				WriteRegister(KME_DEHALO5_PHMV_FIX_F_reg,16,25, 0x12);
				WriteRegister(KME_DEHALO5_PHMV_FIX_11_reg,0,9, 0x23);
				WriteRegister(KME_DEHALO5_PHMV_FIX_11_reg,16,25, 0x3c); 
				WriteRegister(KME_DEHALO5_PHMV_FIX_10_reg,24,31, 0x0);
				WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,25,30, 0x8);
				WriteRegister(KME_DEHALO5_PHMV_FIX_36_reg,10,15, 0x8);
				FrameHold_214 -- ;
		   
			}
			else if(FrameHoldflag_214)
			{
				WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,28,29,0x2);  // init
				WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x0);
				WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,27,27,0x0);
				WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x0);
				WriteRegister(KME_DEHALO5_PHMV_FIX_2B_reg,8,15,0x62);
				WriteRegister(KME_DEHALO5_PHMV_FIX_2C_reg,16,23,0x46);
				WriteRegister(KME_DEHALO5_PHMV_FIX_2E_reg,0,7,0x28);
				WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,0x1);
				WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,0,0,0x1);
				WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,0x1);
				WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,24,29,0xc);
				WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,16,21,0xf);
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0x14);
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,8,13,0xc);
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,0,5,0xf);
				//WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,0x16);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1A_reg,26,31,0x4);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0xc);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0xc);
				WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x22);
				WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x384);
				WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,18,26,0x0);
				WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,16,24,0x15e);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,0,6,0x54);
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,16,20,0xf);
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,0x4);
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,0x4); 
				WriteRegister(KME_DEHALO5_PHMV_FIX_7_reg,8,12,0xf);
				WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,0x0);
				WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,8,9,0x2);
				WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,0,7,0x19);
				WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,0x1);
				WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,18,18,0x1);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1F_reg,8,14,0x57);  //bg max
				WriteRegister(KME_DEHALO5_PHMV_FIX_1F_reg,0,6,0x52);  //bg min
				WriteRegister(KME_DEHALO5_PHMV_FIX_34_reg,16,23,0x5);  //ver pre y0
				WriteRegister(KME_DEHALO5_PHMV_FIX_34_reg,24,31,0xf);  //ver pre y1
				WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23, 0x0);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25, 0x12c);
				WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,24,30, 0x57);
				WriteRegister(KME_DEHALO5_PHMV_FIX_10_reg,24,31, 0x0);
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,old_pred_en_init);
				WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,12,12,0x1); //only mv
				WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,15,15,0x1); //only occl
		   
				WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x1);
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x1);		
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x0);
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x1);  
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x0); 
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x1);		  
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x0);
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x1);
				WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x0);
				FrameHoldflag_214=0;
		   
			}
		   //--------------------------------- ----------------------------------//
		
		   if(occl_more_1)
		   {
			   WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x0);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x2); 	   
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x0);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x2);  
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x0); 
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x2);		 
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x0);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x2);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x0);
		
			   occl_more_hold1 = 5;
			   occl_more_flag1 = 1;
		   }
		   else if(occl_more_hold1 > 0)
		   {
			   WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x0);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x2); 	   
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x0);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x2);  
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x0); 
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x2);		 
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x0);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x2);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x0);
			   occl_more_hold1--;
		   }
		   else if(occl_more_flag1)
		   {
			   occl_more_flag1 = 0;
			   WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x1);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x0); 	   
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x0);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x0);  
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x0); 
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x0);		 
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x0);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x0);
			   WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x0);
		   }
		
		   if(occl_more_2)
		   {
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x1);  //penalty_en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x1); //divide en
			   occl_more_hold2 = 5;
			   occl_more_flag2 = 1;
		   }
		   else if(occl_more_hold2 > 0)
		   {
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x1);  //penalty_en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x1); //divide en
			   occl_more_hold2 --;
		   }
		   else if(occl_more_flag2)
		   {
			   occl_more_flag2 = 0;
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x0);  //penalty_en
			   WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x0); //divide en
		   }
		
		   if(occl_more_3)
		   {
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x12c); //extend4 posdiff
			   occl_more_hold3 = 5;
			   occl_more_flag3 = 1;
		   }
		   else if(occl_more_hold3 > 0)
		   {
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff); //extend posdiff th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x12c); //extend4 posdiff
			   occl_more_hold3--;
		   }
		   else if(occl_more_flag3)
		   {
			   WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x12c); //extend posdiff th
			   WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x2bc); //extend4 posdiff
			   occl_more_flag3 = 0;
		   }

        }
        else
        {
		/*WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,0,6,bg_useless_fix_bg_th_init);//198
		WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,posdiff_judge_divide_en_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,posdiff_judge_penalty_en_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,posdiff_judge_th1_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,10,11,pred_en_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,pred_th0_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_A_reg,26,30,pred_th1_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,20,20,occl_correct_en_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,bg_extend_cov_th0_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,8,14,bg_extend_cov_th1_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,bg_extend_ucov_th0_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,24,30,bg_extend_ucov_th1_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,2,2,pred_ver_en_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,16,16,pred_me1_diff_cut_en_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_9_reg,16,25,gmv_th_y2_init);          
		WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,bg_extend_cond3_en_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,8,12,occl_filter_ucov_cut_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_1F_reg,8,14,bg_extend_bg_sum_max_th_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,color_protect_th0_en_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,color_protect_th1_en_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,16,23,color_protect_th1_min_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,0,4,occl_filter_cov_cut_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,10,13,extend_correct_cut_th_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,0,7,judge_max_th0_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,8,15,judge_max_th1_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,6,6,fg_protect_by_apl_en_init); 
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,14,14,old_pred_en_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,pred_old_diff_cut_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,7,7,bad_pred_cut_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,bad_old_cut_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,24,24,bg_change_cut_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg,6,6,pred_bad_cut_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,gmv_th_y0_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15, bg_extend_cond4_en_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,0,6, useless_max_th_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23, extend_mvdiff_th_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25, extend_posdiff_th_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,24,30, extend_flag_num_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_F_reg,16,25, clear_y0_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_11_reg,0,9, clear_y1_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_11_reg,16,25, clear_y2_init); 
		WriteRegister(KME_DEHALO5_PHMV_FIX_10_reg,24,31, clear_posdiff_th_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,25,30, pfv_bg_change_th_init);
		WriteRegister(KME_DEHALO5_PHMV_FIX_36_reg,10,15, ppfv_bg_change_th_init);*/
		//  WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,23,23,0x0); // reg_dh_postflt_bypass
        }

	/*
        ReadRegister(KME_ME1_TOP2_KME_ME1_TOP2_BC_reg,12,23,&unconf_val1);
        ReadRegister(KME_ME1_TOP2_KME_ME1_TOP2_BC_reg,0,11,&unconf_val2);
        rtd_pr_memc_info("grp1=%d grp0=%d\n",unconf_val1,unconf_val2);

        for(rgnIdx =0 ; rgnIdx< 32; rgnIdx++)
           {
               ReadRegister(KME_ME1_TOP7_ME1_STATIS_SAD_01_reg+4*rgnIdx ,0,24,&u32_RB_val);   
               SAD_rgn[rgnIdx] = u32_RB_val ;

           }
       for(rgnIdx =0 ; rgnIdx< 32; rgnIdx++)
           {  
               sum_sad+=SAD_rgn[rgnIdx];
           } 
       avr_sad=sum_sad/32;

        
	rtd_pr_memc_info("sad=%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d avr=%d\n",SAD_rgn[0],SAD_rgn[1],SAD_rgn[2],SAD_rgn[3],SAD_rgn[4],SAD_rgn[5],SAD_rgn[6],SAD_rgn[7],SAD_rgn[8],SAD_rgn[9],SAD_rgn[10],SAD_rgn[11],SAD_rgn[12],SAD_rgn[13],SAD_rgn[14],SAD_rgn[15],SAD_rgn[16],SAD_rgn[17],SAD_rgn[18],SAD_rgn[19],SAD_rgn[20],SAD_rgn[21],SAD_rgn[22],SAD_rgn[23],SAD_rgn[24],SAD_rgn[25],SAD_rgn[26],SAD_rgn[27],SAD_rgn[28],SAD_rgn[29],SAD_rgn[30],SAD_rgn[31],avr_sad);

	*/


	/*
		if(pParam->u1_me1_dehalo_Newalgo_print==1)
		{
			if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_divide_en==1)
			{
			    //rtd_pr_memc_info("dh_condition_divide_en");
			    rtd_pr_memc_info("dh_1");
			}
			if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_virtical==1)
			{
			   // rtd_pr_memc_info("dh_condition_virtical");
			   rtd_pr_memc_info("dh_2");
			} 			
			if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_large_mv_th_y2==1)
			{
			//	rtd_pr_memc_info("dh_condition_large_mv_th_y2");
			rtd_pr_memc_info("dh_3");
		    }
			if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_small_mv_th_y0==1)
			{
			//	rtd_pr_memc_info("dh_condition_small_mv_th_y0");
			rtd_pr_memc_info("dh_4");
			}
			if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_black_protect_off==1)
			{
			//	rtd_pr_memc_info("dh_condition_black_protect_off");
			rtd_pr_memc_info("dh_5");
			}
			if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_correct_cut_th==1)
			{
				//rtd_pr_memc_info("dh_condition_correct_cut_th");
				rtd_pr_memc_info("dh_6");
			}
			if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_aplfix_en==1)
			{
			  //  rtd_pr_memc_info("dh_condition_aplfix_en");
			  rtd_pr_memc_info("dh_7");
			}
		    if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_white_protect_on==1)
			{
			//	rtd_pr_memc_info("dh_condition_white_protect_on\n");
			rtd_pr_memc_info("dh_8\n");
			}
	   }
	   */
	}
} 	

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
// HS merlin7
	unsigned int u32_RB_val;
// HS merlin7

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
	ReadRegister(KME_DEHALO6_dh_logo_ctrl_func_en_tmv_mode_reg, 30, 30, &u32_RB_val); // bool_dh_logo_15rgn_ctrl_en
	if( u32_RB_val == 0 )	// 3 regions
	{
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
		
	}
	else
	{	

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
	}
	else if( u32_RB_val == 1 )	// 15 regions
	{
		if(pParam->u1_logo_dhRgnProc_en){

			// region 0, 4, 10, 14 control (mapping to region 0 control in 3 regions)
			if( s_pContext->_output_FRC_LgDet.u1_logo_PurePanning_status == 1)
			{
				WriteRegister(KME_DEHALO2_KME_DEHALO2_84_reg,      0,  5, 10);  // rgn0_mvd_mv_d_diff_th
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg4_reg, 26, 31, 10);	// dh_logo_mv_d_diff_th_rg4
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rga_reg, 26, 31, 10);	// dh_logo_mv_d_diff_th_rga
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rge_reg, 26, 31, 10);	// dh_logo_mv_d_diff_th_rge
			}
			else if( s_pContext->_output_FRC_LgDet.u3_logo_dhRgnProc_lvl[0] == 0 )
			{
				WriteRegister(KME_DEHALO2_KME_DEHALO2_84_reg,      0,  5, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][0]); 	// rgn0_mvd_mv_d_diff_th
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg4_reg, 26, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][4]);		// dh_logo_mv_d_diff_th_rg4
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rga_reg, 26, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][10]);	// dh_logo_mv_d_diff_th_rga
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rge_reg, 26, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][14]);	// dh_logo_mv_d_diff_th_rge
				
				WriteRegister(KME_DEHALO2_KME_DEHALO2_A0_reg,          0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][0]); 	// rgn0_tmv_mv_t_th
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg3_rg4_reg, 24, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][4]);		// dh_logo_mv_t_th_rg4
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg9_rga_reg, 24, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][10]);	// dh_logo_mv_t_th_rga
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgd_rge_reg, 24, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][14]);	// dh_logo_mv_t_th_rge
			}
			else if( s_pContext->_output_FRC_LgDet.u3_logo_dhRgnProc_lvl[0] == 1 )
			{
				WriteRegister(KME_DEHALO2_KME_DEHALO2_84_reg,      0,  5, 35);	// rgn0_mvd_mv_d_diff_th
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg4_reg, 26, 31, 35);	// dh_logo_mv_d_diff_th_rg4
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rga_reg, 26, 31, 35);	// dh_logo_mv_d_diff_th_rga
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rge_reg, 26, 31, 35);	// dh_logo_mv_d_diff_th_rge
				
				WriteRegister(KME_DEHALO2_KME_DEHALO2_A0_reg,          0,  7, 8);	// rgn0_tmv_mv_t_th
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg3_rg4_reg, 24, 31, 8);	// dh_logo_mv_t_th_rg4
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg9_rga_reg, 24, 31, 8);	// dh_logo_mv_t_th_rga
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgd_rge_reg, 24, 31, 8);	// dh_logo_mv_t_th_rge
			}
			else if( s_pContext->_output_FRC_LgDet.u3_logo_dhRgnProc_lvl[0] == 2 )
			{
				WriteRegister(KME_DEHALO2_KME_DEHALO2_84_reg,      0,  5, 50);	// rgn0_mvd_mv_d_diff_th
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg4_reg, 26, 31, 50);	// dh_logo_mv_d_diff_th_rg4
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rga_reg, 26, 31, 50);	// dh_logo_mv_d_diff_th_rga
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rge_reg, 26, 31, 50);	// dh_logo_mv_d_diff_th_rge
				
				WriteRegister(KME_DEHALO2_KME_DEHALO2_A0_reg,          0,  7, 4);	// rgn0_tmv_mv_t_th
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg3_rg4_reg, 24, 31, 4);	// dh_logo_mv_t_th_rg4
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg9_rga_reg, 24, 31, 4);	// dh_logo_mv_t_th_rga
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgd_rge_reg, 24, 31, 4);	// dh_logo_mv_t_th_rge				
			}
			else
			{
				WriteRegister(KME_DEHALO2_KME_DEHALO2_84_reg,      0,  5, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][0]);		// rgn0_mvd_mv_d_diff_th
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg4_reg, 26, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][4]);		// dh_logo_mv_d_diff_th_rg4
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rga_reg, 26, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][10]);	// dh_logo_mv_d_diff_th_rga
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rge_reg, 26, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][14]);	// dh_logo_mv_d_diff_th_rge
				
				WriteRegister(KME_DEHALO2_KME_DEHALO2_A0_reg,          0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][0]);		// rgn0_tmv_mv_t_th
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg3_rg4_reg, 24, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][4]);		// dh_logo_mv_t_th_rg4
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg9_rga_reg, 24, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][10]);	// dh_logo_mv_t_th_rga
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgd_rge_reg, 24, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][14]);	// dh_logo_mv_t_th_rge	
			}

			// region 1, 2, 3, 5, 6, 7, 8, 9, 11, 13 control (mapping to region 1 control in 3 regions)
			if( s_pContext->_output_FRC_LgDet.u1_logo_BG_still_status == 1 )
			{
				WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg,          8, 15, 32);	// rgn1_tmv_num
				WriteRegister(KME_DEHALO2_KME_DEHALO2_10_reg,          0,  7, 32);	// tmv_num_rg2
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg3_rg4_reg,  0,  7, 32);	// dh_logo_tmv_num_rg3
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg5_rg6_reg,  0,  7, 32);	// dh_logo_tmv_num_rg5
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg5_rg6_reg, 16, 23, 32);	// dh_logo_tmv_num_rg6				
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg7_rg8_reg,  0,  7, 32);	// dh_logo_tmv_num_rg7
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg7_rg8_reg, 16, 23, 32);	// dh_logo_tmv_num_rg8
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg9_rga_reg,  0,  7, 32);	// dh_logo_tmv_num_rg9
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgb_rgc_reg,  0,  7, 32);	// dh_logo_tmv_num_rgb
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgd_rge_reg,  0,  7, 32);	// dh_logo_tmv_num_rgd
				
				WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg,          0,  7, 4);	// rgn1_tmv_mv_t_th					
				WriteRegister(KME_DEHALO2_KME_DEHALO2_08_reg,         10, 17, 4);	// mv_t_th_rg2
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg3_rg4_reg,  8, 15, 4);	// dh_logo_mv_t_th_rg3
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg5_rg6_reg,  8, 15, 4);	// dh_logo_mv_t_th_rg5
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg5_rg6_reg, 24, 31, 4);	// dh_logo_mv_t_th_rg6
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg7_rg8_reg,  8, 15, 4);	// dh_logo_mv_t_th_rg7
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg7_rg8_reg, 24, 31, 4);	// dh_logo_mv_t_th_rg8
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg9_rga_reg,  8, 15, 4);	// dh_logo_mv_t_th_rg9
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgb_rgc_reg,  8, 15, 4);	// dh_logo_mv_t_th_rgb
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgd_rge_reg,  8, 15, 4);	// dh_logo_mv_t_th_rgd
			}	
			else if( s_pContext->_output_FRC_LgDet.u3_logo_dhRgnProc_lvl[1] == 0 )
			{
				WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg,          8, 15, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][1]); 	// rgn1_tmv_num
				WriteRegister(KME_DEHALO2_KME_DEHALO2_10_reg,          0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][2]);		// tmv_num_rg2
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg3_rg4_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][3]);		// dh_logo_tmv_num_rg3
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg5_rg6_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][5]);		// dh_logo_tmv_num_rg5
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg5_rg6_reg, 16, 23, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][6]);		// dh_logo_tmv_num_rg6				
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg7_rg8_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][7]);		// dh_logo_tmv_num_rg7
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg7_rg8_reg, 16, 23, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][8]);		// dh_logo_tmv_num_rg8
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg9_rga_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][9]);		// dh_logo_tmv_num_rg9
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgb_rgc_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][11]);	// dh_logo_tmv_num_rgb
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgd_rge_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][13]);	// dh_logo_tmv_num_rgd
				
				WriteRegister(KME_DEHALO2_KME_DEHALO2_AC_reg,          0,  5, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][1]); 	// rgn1_mvd_mv_d_diff_th
				WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg,         13, 18, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][2]);		// mv_d_diff_th_rg2
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg3_reg,     26, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][3]);		// dh_logo_mv_d_diff_th_rg3
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg5_reg,     26, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][5]);		// dh_logo_mv_d_diff_th_rg5
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg6_reg,     26, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][6]);		// dh_logo_mv_d_diff_th_rg6
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg7_reg,     26, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][7]);		// dh_logo_mv_d_diff_th_rg7
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg8_reg,     26, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][8]);		// dh_logo_mv_d_diff_th_rg8
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg9_reg,     26, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][9]);		// dh_logo_mv_d_diff_th_rg9
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rgb_reg,     26, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][11]);	// dh_logo_mv_d_diff_th_rgb
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rgd_reg,     26, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][13]);	// dh_logo_mv_d_diff_th_rgd
				
				WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg,          0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][1]); 	// rgn1_tmv_mv_t_th
				WriteRegister(KME_DEHALO2_KME_DEHALO2_08_reg,         10, 17, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][2]);		// mv_t_th_rg2
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg3_rg4_reg,  8, 15, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][3]);		// dh_logo_mv_t_th_rg3
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg5_rg6_reg,  8, 15, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][5]);		// dh_logo_mv_t_th_rg5
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg5_rg6_reg, 24, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][6]);		// dh_logo_mv_t_th_rg6
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg7_rg8_reg,  8, 15, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][7]);		// dh_logo_mv_t_th_rg7
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg7_rg8_reg, 24, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][8]);		// dh_logo_mv_t_th_rg8
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg9_rga_reg,  8, 15, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][9]);		// dh_logo_mv_t_th_rg9
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgb_rgc_reg,  8, 15, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][11]);	// dh_logo_mv_t_th_rgb
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgd_rge_reg,  8, 15, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][13]);	// dh_logo_mv_t_th_rgd				
			}
			else if( s_pContext->_output_FRC_LgDet.u3_logo_dhRgnProc_lvl[1] == 1 )
			{
				WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg,          8, 15, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][1]); 	// rgn1_tmv_num
				WriteRegister(KME_DEHALO2_KME_DEHALO2_10_reg,          0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][2]);		// tmv_num_rg2
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg3_rg4_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][3]);		// dh_logo_tmv_num_rg3
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg5_rg6_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][5]);		// dh_logo_tmv_num_rg5
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg5_rg6_reg, 16, 23, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][6]);		// dh_logo_tmv_num_rg6				
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg7_rg8_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][7]);		// dh_logo_tmv_num_rg7
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg7_rg8_reg, 16, 23, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][8]);		// dh_logo_tmv_num_rg8
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg9_rga_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][9]);		// dh_logo_tmv_num_rg9
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgb_rgc_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][11]);	// dh_logo_tmv_num_rgb
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgd_rge_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][13]);	// dh_logo_tmv_num_rgd
				
				WriteRegister(KME_DEHALO2_KME_DEHALO2_AC_reg,          0,  5, 40); 	// rgn1_mvd_mv_d_diff_th
				WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg,         13, 18, 40);	// mv_d_diff_th_rg2
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg3_reg,     26, 31, 40);	// dh_logo_mv_d_diff_th_rg3
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg5_reg,     26, 31, 40);	// dh_logo_mv_d_diff_th_rg5
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg6_reg,     26, 31, 40);	// dh_logo_mv_d_diff_th_rg6
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg7_reg,     26, 31, 40);	// dh_logo_mv_d_diff_th_rg7
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg8_reg,     26, 31, 40);	// dh_logo_mv_d_diff_th_rg8
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg9_reg,     26, 31, 40);	// dh_logo_mv_d_diff_th_rg9
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rgb_reg,     26, 31, 40);	// dh_logo_mv_d_diff_th_rgb
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rgd_reg,     26, 31, 40);	// dh_logo_mv_d_diff_th_rgd
				
				WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg,          0,  7, 8); 	// rgn1_tmv_mv_t_th
				WriteRegister(KME_DEHALO2_KME_DEHALO2_08_reg,         10, 17, 8);	// mv_t_th_rg2
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg3_rg4_reg,  8, 15, 8);	// dh_logo_mv_t_th_rg3
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg5_rg6_reg,  8, 15, 8);	// dh_logo_mv_t_th_rg5
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg5_rg6_reg, 24, 31, 8);	// dh_logo_mv_t_th_rg6
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg7_rg8_reg,  8, 15, 8);	// dh_logo_mv_t_th_rg7
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg7_rg8_reg, 24, 31, 8);	// dh_logo_mv_t_th_rg8
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg9_rga_reg,  8, 15, 8);	// dh_logo_mv_t_th_rg9
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgb_rgc_reg,  8, 15, 8);	// dh_logo_mv_t_th_rgb
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgd_rge_reg,  8, 15, 8);	// dh_logo_mv_t_th_rgd	
				
			}
			else if( s_pContext->_output_FRC_LgDet.u3_logo_dhRgnProc_lvl[1] == 2 )
			{
				WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg,          8, 15, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][1]); 	// rgn1_tmv_num
				WriteRegister(KME_DEHALO2_KME_DEHALO2_10_reg,          0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][2]);		// tmv_num_rg2
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg3_rg4_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][3]);		// dh_logo_tmv_num_rg3
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg5_rg6_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][5]);		// dh_logo_tmv_num_rg5
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg5_rg6_reg, 16, 23, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][6]);		// dh_logo_tmv_num_rg6				
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg7_rg8_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][7]);		// dh_logo_tmv_num_rg7
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg7_rg8_reg, 16, 23, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][8]);		// dh_logo_tmv_num_rg8
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg9_rga_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][9]);		// dh_logo_tmv_num_rg9
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgb_rgc_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][11]);	// dh_logo_tmv_num_rgb
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgd_rge_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][13]);	// dh_logo_tmv_num_rgd
				
				WriteRegister(KME_DEHALO2_KME_DEHALO2_AC_reg,          0,  5, 52); 	// rgn1_mvd_mv_d_diff_th
				WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg,         13, 18, 52);	// mv_d_diff_th_rg2
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg3_reg,     26, 31, 52);	// dh_logo_mv_d_diff_th_rg3
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg5_reg,     26, 31, 52);	// dh_logo_mv_d_diff_th_rg5
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg6_reg,     26, 31, 52);	// dh_logo_mv_d_diff_th_rg6
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg7_reg,     26, 31, 52);	// dh_logo_mv_d_diff_th_rg7
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg8_reg,     26, 31, 52);	// dh_logo_mv_d_diff_th_rg8
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg9_reg,     26, 31, 52);	// dh_logo_mv_d_diff_th_rg9
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rgb_reg,     26, 31, 52);	// dh_logo_mv_d_diff_th_rgb
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rgd_reg,     26, 31, 52);	// dh_logo_mv_d_diff_th_rgd
				
				WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg,          0,  7, 4); 	// rgn1_tmv_mv_t_th
				WriteRegister(KME_DEHALO2_KME_DEHALO2_08_reg,         10, 17, 4);	// mv_t_th_rg2
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg3_rg4_reg,  8, 15, 4);	// dh_logo_mv_t_th_rg3
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg5_rg6_reg,  8, 15, 4);	// dh_logo_mv_t_th_rg5
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg5_rg6_reg, 24, 31, 4);	// dh_logo_mv_t_th_rg6
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg7_rg8_reg,  8, 15, 4);	// dh_logo_mv_t_th_rg7
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg7_rg8_reg, 24, 31, 4);	// dh_logo_mv_t_th_rg8
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg9_rga_reg,  8, 15, 4);	// dh_logo_mv_t_th_rg9
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgb_rgc_reg,  8, 15, 4);	// dh_logo_mv_t_th_rgb
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgd_rge_reg,  8, 15, 4);	// dh_logo_mv_t_th_rgd					
			}
			else
			{
				WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg,          8, 15, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][1]); 	// rgn1_tmv_num
				WriteRegister(KME_DEHALO2_KME_DEHALO2_10_reg,          0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][2]);		// tmv_num_rg2
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg3_rg4_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][3]);		// dh_logo_tmv_num_rg3
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg5_rg6_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][5]);		// dh_logo_tmv_num_rg5
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg5_rg6_reg, 16, 23, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][6]);		// dh_logo_tmv_num_rg6				
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg7_rg8_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][7]);		// dh_logo_tmv_num_rg7
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg7_rg8_reg, 16, 23, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][8]);		// dh_logo_tmv_num_rg8
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg9_rga_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][9]);		// dh_logo_tmv_num_rg9
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgb_rgc_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][11]);	// dh_logo_tmv_num_rgb
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgd_rge_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][13]);	// dh_logo_tmv_num_rgd
				
				WriteRegister(KME_DEHALO2_KME_DEHALO2_AC_reg,          0,  5, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][1]); 	// rgn1_mvd_mv_d_diff_th
				WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg,         13, 18, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][2]);		// mv_d_diff_th_rg2
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg3_reg,     26, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][3]);		// dh_logo_mv_d_diff_th_rg3
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg5_reg,     26, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][5]);		// dh_logo_mv_d_diff_th_rg5
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg6_reg,     26, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][6]);		// dh_logo_mv_d_diff_th_rg6
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg7_reg,     26, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][7]);		// dh_logo_mv_d_diff_th_rg7
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg8_reg,     26, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][8]);		// dh_logo_mv_d_diff_th_rg8
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg9_reg,     26, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][9]);		// dh_logo_mv_d_diff_th_rg9
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rgb_reg,     26, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][11]);	// dh_logo_mv_d_diff_th_rgb
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rgd_reg,     26, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][13]);	// dh_logo_mv_d_diff_th_rgd
				
				WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg,          0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][1]); 	// rgn1_tmv_mv_t_th
				WriteRegister(KME_DEHALO2_KME_DEHALO2_08_reg,         10, 17, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][2]);		// mv_t_th_rg2
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg3_rg4_reg,  8, 15, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][3]);		// dh_logo_mv_t_th_rg3
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg5_rg6_reg,  8, 15, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][5]);		// dh_logo_mv_t_th_rg5
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg5_rg6_reg, 24, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][6]);		// dh_logo_mv_t_th_rg6
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg7_rg8_reg,  8, 15, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][7]);		// dh_logo_mv_t_th_rg7
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg7_rg8_reg, 24, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][8]);		// dh_logo_mv_t_th_rg8
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg9_rga_reg,  8, 15, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][9]);		// dh_logo_mv_t_th_rg9
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgb_rgc_reg,  8, 15, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][11]);	// dh_logo_mv_t_th_rgb
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgd_rge_reg,  8, 15, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][13]);	// dh_logo_mv_t_th_rgd	
				
			}		

			// region 12 control (mapping to region 2 control in 3 regions)
			if( s_pContext->_output_FRC_LgDet.u1_logo_PurePanning_status == 1 )
			{
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rgc_reg, 26, 31, 12);	// dh_logo_mv_d_diff_th_rgc				
			}else{
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rgc_reg, 26, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][12]);	// dh_logo_mv_d_diff_th_rgc		
			}			
			
			if(s_pContext->_output_FRC_LgDet.u3_logo_dhRgnProc_case[2] ==1){
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg3_reg, 20, 25, 31);	
			}else{
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg3_reg, 20, 25, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[1][3]);	
			}

			if( (s_pContext->_output_FRC_LgDet.u3_logo_dhRgnProc_H_Moving_case==1) && (s_pContext->_output_FRC_LgDet.u3_logo_dhRgnProc_case[3] ==1)){
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rga_reg, 0, 9, 56);	
			}else{
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rga_reg, 0, 9, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[0][10]);	
			}
		}
		else
		{	
			// region 0, 4, 10, 14 control (mapping to region 0 control in 3 regions)
			if( s_pContext->_output_FRC_LgDet.u1_logo_PurePanning_status == 1 )
			{
				WriteRegister(KME_DEHALO2_KME_DEHALO2_84_reg,      0,  5, 10);  // rgn0_mvd_mv_d_diff_th
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg4_reg, 26, 31, 10);	// dh_logo_mv_d_diff_th_rg4
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rga_reg, 26, 31, 10);	// dh_logo_mv_d_diff_th_rga
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rge_reg, 26, 31, 10);	// dh_logo_mv_d_diff_th_rge
			}		
			else
			{
				WriteRegister(KME_DEHALO2_KME_DEHALO2_84_reg,      0,  5, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][0]); 	// rgn0_mvd_mv_d_diff_th
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rg4_reg, 26, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][4]);		// dh_logo_mv_d_diff_th_rg4
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rga_reg, 26, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][10]);	// dh_logo_mv_d_diff_th_rga
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rge_reg, 26, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][14]);	// dh_logo_mv_d_diff_th_rge				
			}

			// region 1, 2, 3, 5, 6, 7, 8, 9, 11, 13 control (mapping to region 1 control in 3 regions)
			if( s_pContext->_output_FRC_LgDet.u1_logo_BG_still_status == 1 )
			{
				WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg,          8, 15, 32);	// rgn1_tmv_num
				WriteRegister(KME_DEHALO2_KME_DEHALO2_10_reg,          0,  7, 32);	// tmv_num_rg2
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg3_rg4_reg,  0,  7, 32);	// dh_logo_tmv_num_rg3
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg5_rg6_reg,  0,  7, 32);	// dh_logo_tmv_num_rg5
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg5_rg6_reg, 16, 23, 32);	// dh_logo_tmv_num_rg6				
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg7_rg8_reg,  0,  7, 32);	// dh_logo_tmv_num_rg7
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg7_rg8_reg, 16, 23, 32);	// dh_logo_tmv_num_rg8
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg9_rga_reg,  0,  7, 32);	// dh_logo_tmv_num_rg9
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgb_rgc_reg,  0,  7, 32);	// dh_logo_tmv_num_rgb
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgd_rge_reg,  0,  7, 32);	// dh_logo_tmv_num_rgd
				
				WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg,          0,  7, 4);	// rgn1_tmv_mv_t_th					
				WriteRegister(KME_DEHALO2_KME_DEHALO2_08_reg,         10, 17, 4);	// mv_t_th_rg2
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg3_rg4_reg,  8, 15, 4);	// dh_logo_mv_t_th_rg3
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg5_rg6_reg,  8, 15, 4);	// dh_logo_mv_t_th_rg5
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg5_rg6_reg, 24, 31, 4);	// dh_logo_mv_t_th_rg6
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg7_rg8_reg,  8, 15, 4);	// dh_logo_mv_t_th_rg7
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg7_rg8_reg, 24, 31, 4);	// dh_logo_mv_t_th_rg8
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg9_rga_reg,  8, 15, 4);	// dh_logo_mv_t_th_rg9
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgb_rgc_reg,  8, 15, 4);	// dh_logo_mv_t_th_rgb
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgd_rge_reg,  8, 15, 4);	// dh_logo_mv_t_th_rgd				
			}				
			else
			{
				WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg,          8, 15, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][1]); 	// rgn1_tmv_num
				WriteRegister(KME_DEHALO2_KME_DEHALO2_10_reg,          0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][2]);		// tmv_num_rg2
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg3_rg4_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][3]);		// dh_logo_tmv_num_rg3
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg5_rg6_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][5]);		// dh_logo_tmv_num_rg5
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg5_rg6_reg, 16, 23, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][6]);		// dh_logo_tmv_num_rg6				
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg7_rg8_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][7]);		// dh_logo_tmv_num_rg7
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg7_rg8_reg, 16, 23, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][8]);		// dh_logo_tmv_num_rg8
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg9_rga_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][9]);		// dh_logo_tmv_num_rg9
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgb_rgc_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][11]);	// dh_logo_tmv_num_rgb
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgd_rge_reg,  0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[5][13]);	// dh_logo_tmv_num_rgd
				
				WriteRegister(KME_DEHALO2_KME_DEHALO2_A4_reg,          0,  7, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][1]); 	// rgn1_tmv_mv_t_th
				WriteRegister(KME_DEHALO2_KME_DEHALO2_08_reg,         10, 17, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][2]);		// mv_t_th_rg2
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg3_rg4_reg,  8, 15, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][3]);		// dh_logo_mv_t_th_rg3
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg5_rg6_reg,  8, 15, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][5]);		// dh_logo_mv_t_th_rg5
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg5_rg6_reg, 24, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][6]);		// dh_logo_mv_t_th_rg6
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg7_rg8_reg,  8, 15, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][7]);		// dh_logo_mv_t_th_rg7
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg7_rg8_reg, 24, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][8]);		// dh_logo_mv_t_th_rg8
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rg9_rga_reg,  8, 15, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][9]);		// dh_logo_mv_t_th_rg9
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgb_rgc_reg,  8, 15, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][11]);	// dh_logo_mv_t_th_rgb
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_2_rgd_rge_reg,  8, 15, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[4][13]);	// dh_logo_mv_t_th_rgd				
			}		

			// region 12 control (mapping to region 2 control in 3 regions)
			if( s_pContext->_output_FRC_LgDet.u1_logo_PurePanning_status == 1 )
			{
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rgc_reg, 26, 31, 12);	// dh_logo_mv_d_diff_th_rgc	
			}else{
				WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rgc_reg, 26, 31, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_RgnThr_15rgn[2][12]);	// dh_logo_mv_d_diff_th_rgc
			}		
			
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

		if (/*strcmp(webos_strToolOption.eBackLight, "oled") == 0*//*SLD_SW_En == 1*/MEMC_Pixel_LOGO_For_SW_SLD == 1 || MEMC_Pixel_LOGO_For_SW_SLD == 2)
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
//	ReadRegister(HARDWARE_HARDWARE_58_reg, 0, 31, &u32_RB_val);//bit 8
//	if ( ((u32_RB_val >>  8) & 0x01) ==1 ){
//			rtd_pr_memc_emerg("[%s][%d][,%d,%d]\n",__FUNCTION__, __LINE__,s_pContext->_output_me_sceneAnalysis.u2_panning_flag, ,s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true);
//	}	
#if	1
	if(pParam->u1_logo_HaloEnahce_en == 0){
		WriteRegister(KME_DEHALO3_KME_DEHALO3_0C_reg, 12, 12, 0);
		u1_pre_logo_HaloEnahce_en = pParam->u1_logo_HaloEnahce_en;
	}else{

		if( (s_pContext->_output_FRC_LgDet.u1_logo_PurePanning_status==3 || s_pContext->_output_FRC_LgDet.u1_logo_PurePanning_status== 4)&&(s_pContext->_output_FRC_LgDet.u1_logo_NearRim_logo_status==1)){
			WriteRegister(KME_DEHALO3_KME_DEHALO3_0C_reg, 12, 12, 0);
			WriteRegister(MC_MC_18_reg, 17, 20, 3);
			WriteRegister(KME_LOGO2_KME_LOGO2_28_reg, 9, 14, 2);
		}else if((s_pContext->_output_FRC_LgDet.u3_logo_dhRgnProc_H_Moving_case == 1) && (s_pContext->_output_FRC_LgDet.u3_logo_dhRgnProc_case[3] ==1)){
			WriteRegister(KME_LOGO2_KME_LOGO2_28_reg, 9, 14, 2);
		}else if( (s_pContext->_output_FRC_LgDet.u1_logo_PurePanning_status == 2 || s_pContext->_output_wrt_comreg.u1_SquidGame_subTitle_flag ==1 )){
			WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 6, 	8, 7);
			WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 22, 	24, 7);
			WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 6, 	8, 7);
			WriteRegister(KME_LOGO2_KME_LOGO2_28_reg, 9, 14, 3);
			WriteRegister(KME_LOGO2_KME_LOGO2_2C_reg, 9, 16, 4);
			//logo_check_K24014_flag =1;
		}else{
			WriteRegister(KME_DEHALO3_KME_DEHALO3_0C_reg, 12, 12, 1);
			WriteRegister(MC_MC_18_reg, 17, 20, 6);
			WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 6, 8, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[11][3]);
			WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 22, 24, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[12][3]);
			WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 6, 8, s_pContext->_output_FRC_LgDet.DefaultVal.dh_logo_bypass[13][3]);
			WriteRegister(KME_LOGO2_KME_LOGO2_28_reg, 9, 14, s_pContext->_output_FRC_LgDet.DefaultVal.u6_logo_blklogopostdlt_th);
			WriteRegister(KME_LOGO2_KME_LOGO2_2C_reg, 9, 16, s_pContext->_output_FRC_LgDet.DefaultVal.u6_logo_pxllogopostdlt_th);
			//logo_check_K24014_flag =0;
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

	if(pParam->u1_Unsharp_Mask_wrt_en== 0)
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
#define MEMC_PERFORMANCE_SETTING_TABLE_MAX 32 //256
#define MEMC_PERFORMANCE_SETTING_ITEM_MAX 45
const unsigned int MEMC_setting_Table[/*MEMC_PERFORMANCE_SETTING_TABLE_MAX*/][MEMC_PERFORMANCE_SETTING_ITEM_MAX][MEMC_PERFORMANCE_SETTING_MAX] = 
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
		{MC_MC_28_reg, 15, 22, 0},//FRC_TOP__MC__reg_mc_fb_lvl_force_value
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
		{MC_MC_28_reg, 15, 22, 0},//FRC_TOP__MC__reg_mc_fb_lvl_force_value
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
		{MC_MC_28_reg, 15, 22, 95},//FRC_TOP__MC__reg_mc_fb_lvl_force_value
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
		{MC_MC_28_reg, 15, 22, 0},//FRC_TOP__MC__reg_mc_fb_lvl_force_value
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
		{MC_MC_28_reg, 15, 22, 0},//FRC_TOP__MC__reg_mc_fb_lvl_force_value
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
		{MC_MC_28_reg, 15, 22, 0},//FRC_TOP__MC__reg_mc_fb_lvl_force_value
		//{FRC_TOP__PQL_0__pql_patch_dummy_e4_ADDR, 29, 28, 0},//Merlin7 does not have it
		{MC2_MC2_20_reg, 0, 2, 0},//logo off
	},

	//#25	
	/*Satelite_1920x1080_24_362.avi*/
	/*4K.znds.com]ShutUpAndPlay.Berlin.2014.2160p.HDTV.HEVC-jTV-50p-3840x2160*/
	//0:28-0:29   0:58-1:00   01:46-01:48	 02:33-02:34
	{
		{MC_MC_28_reg, 14, 14, 1},//FRC_TOP__MC__reg_mc_fb_lvl_force_en
		{MC_MC_28_reg, 15, 22, 0},//FRC_TOP__MC__reg_mc_fb_lvl_force_value
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
		{MC_MC_28_reg, 15, 22, 255},//FRC_TOP__MC__reg_mc_fb_lvl_force_value
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
		{MC_MC_28_reg, 15, 22, 0},//FRC_TOP__MC__reg_mc_fb_lvl_force_value
		{IPPRE_IPPRE_04_reg, 4, 11, 100},//FRC_TOP__IPPRE__reg_kmc_blend_y_alpha
		{IPPRE_IPPRE_04_reg, 16, 23, 255},//FRC_TOP__IPPRE__reg_kmc_blend_uv_alpha
	},
};

unsigned char MEMC_Performance_Checking_Database_index = 0;
void MEMC_Performance_Checking_Database(void)
{
	static unsigned int register_return[MEMC_PERFORMANCE_SETTING_ITEM_MAX] = {};
	int i = 0;
	unsigned int check_en = 0;
	unsigned int check_index = MEMC_Performance_Checking_Database_index;
	static unsigned int pre_check_index = 0;
	ReadRegister(SOFTWARE1_SOFTWARE1_42_reg, 19, 19, &check_en);	//to clear any other MEMC isuue is not related to the desgin

	if(check_en == 0 || (pre_check_index == 0 && check_index == 0) || (pre_check_index >= MEMC_PERFORMANCE_SETTING_TABLE_MAX) || (check_index >= MEMC_PERFORMANCE_SETTING_TABLE_MAX))
		return;

	//Return value when switching(first time no need to return value)
	if(pre_check_index != check_index && pre_check_index != 0)
	{
		for(i = 0; i < sizeof(MEMC_setting_Table[pre_check_index])>>4; i++) // 4 (MEMC_PERFORMANCE_SETTING_MAX) * 4(unsigned int) = 16
		{
			if(MEMC_setting_Table[pre_check_index][i][0] != 0)
				WriteRegister(MEMC_setting_Table[pre_check_index][i][0], MEMC_setting_Table[pre_check_index][i][1], MEMC_setting_Table[pre_check_index][i][2], register_return[i]);
		}
	}
	
	for(i = 0; i < sizeof(MEMC_setting_Table[check_index])>>4; i++) // 4 (MEMC_PERFORMANCE_SETTING_MAX) * 4(unsigned int) = 16
	{
		//store the value when switching
		if((pre_check_index != check_index) && (check_index != 0) && MEMC_setting_Table[check_index][i][0] != 0)
		{
			ReadRegister(MEMC_setting_Table[check_index][i][0], MEMC_setting_Table[check_index][i][1], MEMC_setting_Table[check_index][i][2],&register_return[i]);
		}
		//write table value
		if(check_index != 0 && MEMC_setting_Table[check_index][i][0] != 0)
		{
			WriteRegister(MEMC_setting_Table[check_index][i][0], MEMC_setting_Table[check_index][i][1], MEMC_setting_Table[check_index][i][2], MEMC_setting_Table[check_index][i][3]);
		}
	}
	pre_check_index = check_index;
}

#endif
VOID Identification_Pattern_preProcess(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{	
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

 //#128
#if 0
    if((scalerVIP_Get_MEMCPatternFlag_Identification(0,124) == TRUE)  || (scalerVIP_Get_MEMCPatternFlag_Identification(0,126) == TRUE) ) 
    {
        
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 19, 23, 0x2);
        //rtd_pr_memc_info("128in");
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
        //rtd_pr_memc_info("128in");

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
		//rtd_pr_memc_info("128out");
		
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
		   // rtd_pr_memc_info("tulia 339 test\n");
	
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
				rtd_pr_memc_emerg("==Pattern_Update===nIdentifiedNum_a>>[%d][%d]   ,\n", u8_Index, nIdentifiedNum_preProcess_a[u8_Index]);
	
			pOutput->u1_Pattern_Update_true=1;
		}
	}
	//===

}
VOID Identification_Pattern_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
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

	if(pParam->u1_Dh_MV_Corr_en==0)
		return;
	
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

			//WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg , 8 , 18 , sw_mvx );
			//WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg , 19 , 28 , sw_mvy);
		}
		// 7seg_1 [0~]
		//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 0, 10, sw_mvx);
		//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 16, 25, sw_mvy);

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
		rtd_pr_memc_emerg("[Undo mask][table_level][%d]\n", table_level);
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
#ifdef CONFIG_MEMC_BYPASS
	return;
#endif
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
		rtd_pr_memc_notice("[MEMC_DisableMute_02][%d,]\n", scaler_sub_active);
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

		WriteRegister(IPPRE_IPPRE_80_reg,  0, 14,ColorTable.RGB2YUV.R00);
		WriteRegister(IPPRE_IPPRE_80_reg, 15, 29,ColorTable.RGB2YUV.R01);
		WriteRegister(IPPRE_IPPRE_84_reg,  0, 14,ColorTable.RGB2YUV.R02);
		WriteRegister(IPPRE_IPPRE_84_reg, 15, 29,ColorTable.RGB2YUV.R10);
		WriteRegister(IPPRE_IPPRE_88_reg,  0, 14,ColorTable.RGB2YUV.R11);
		WriteRegister(IPPRE_IPPRE_88_reg, 15, 29,ColorTable.RGB2YUV.R12);
		WriteRegister(IPPRE_IPPRE_8C_reg,  0, 14,ColorTable.RGB2YUV.R20);
		WriteRegister(IPPRE_IPPRE_8C_reg, 15, 29,ColorTable.RGB2YUV.R21);
		WriteRegister(IPPRE_IPPRE_90_reg,  0, 14,ColorTable.RGB2YUV.R22);
		WriteRegister(IPPRE_IPPRE_90_reg, 15, 26,ColorTable.RGB2YUV.T0);
		WriteRegister(IPPRE_IPPRE_94_reg,  0, 11,ColorTable.RGB2YUV.T1);
		WriteRegister(IPPRE_IPPRE_94_reg, 12, 23,ColorTable.RGB2YUV.T2);

		WriteRegister(IPPRE_IPPRE_9C_reg,  0, 14,ColorTable.RGB2YUV.R00);
		WriteRegister(IPPRE_IPPRE_9C_reg, 15, 29,ColorTable.RGB2YUV.R01);
		WriteRegister(IPPRE_IPPRE_A0_reg,  0, 14,ColorTable.RGB2YUV.R02);
		WriteRegister(IPPRE_IPPRE_A0_reg, 15, 29,ColorTable.RGB2YUV.R10);
		WriteRegister(IPPRE_IPPRE_A4_reg,  0, 14,ColorTable.RGB2YUV.R11);
		WriteRegister(IPPRE_IPPRE_A4_reg, 15, 29,ColorTable.RGB2YUV.R12);
		WriteRegister(IPPRE_IPPRE_A8_reg,  0, 14,ColorTable.RGB2YUV.R20);
		WriteRegister(IPPRE_IPPRE_A8_reg, 15, 29,ColorTable.RGB2YUV.R21);
		WriteRegister(IPPRE_IPPRE_AC_reg,  0, 14,ColorTable.RGB2YUV.R22);
		WriteRegister(IPPRE_IPPRE_AC_reg, 15, 26,ColorTable.RGB2YUV.T0);
		WriteRegister(IPPRE_IPPRE_B0_reg,  0, 11,ColorTable.RGB2YUV.T1);
		WriteRegister(IPPRE_IPPRE_B0_reg, 12, 23,ColorTable.RGB2YUV.T2);

		WriteRegister(KPOST_TOP_KPOST_TOP_80_reg,  0, 14,ColorTable.YUV2RGB.R00);
		WriteRegister(KPOST_TOP_KPOST_TOP_80_reg, 15, 29,ColorTable.YUV2RGB.R01);
		WriteRegister(KPOST_TOP_KPOST_TOP_84_reg,  0, 14,ColorTable.YUV2RGB.R02);
		WriteRegister(KPOST_TOP_KPOST_TOP_84_reg, 15, 29,ColorTable.YUV2RGB.R10);
		WriteRegister(KPOST_TOP_KPOST_TOP_88_reg,  0, 14,ColorTable.YUV2RGB.R11);
		WriteRegister(KPOST_TOP_KPOST_TOP_88_reg, 15, 29,ColorTable.YUV2RGB.R12);
		WriteRegister(KPOST_TOP_KPOST_TOP_8C_reg,  0, 14,ColorTable.YUV2RGB.R20);
		WriteRegister(KPOST_TOP_KPOST_TOP_8C_reg, 15, 29,ColorTable.YUV2RGB.R21);
		WriteRegister(KPOST_TOP_KPOST_TOP_90_reg,  0, 14,ColorTable.YUV2RGB.R22);
		WriteRegister(KPOST_TOP_KPOST_TOP_90_reg, 15, 26,ColorTable.YUV2RGB.T0);
		WriteRegister(KPOST_TOP_KPOST_TOP_94_reg,  0, 11,ColorTable.YUV2RGB.T1);
		WriteRegister(KPOST_TOP_KPOST_TOP_94_reg, 12, 23,ColorTable.YUV2RGB.T2);

		WriteRegister(IPPRE_IPPRE_94_reg,25,28,0x8);
		WriteRegister(IPPRE_IPPRE_B0_reg,25,28,0x8);
		WriteRegister(KPOST_TOP_KPOST_TOP_94_reg,25,28,0x8);
		if(log_en) {
			rtd_pr_memc_emerg("[%d][%d]\n",u8_table_cnt, u8_table_id );
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
			
			rtd_outl(IPPRE_IPPRE_80_reg,0x00000000);
			rtd_outl(IPPRE_IPPRE_84_reg,0x00000000);
			rtd_outl(IPPRE_IPPRE_88_reg,0x00000000);
			rtd_outl(IPPRE_IPPRE_8C_reg,0x00000000);
			rtd_outl(IPPRE_IPPRE_90_reg,0x00000000);
			rtd_outl(IPPRE_IPPRE_94_reg,0x05000000);
			rtd_outl(IPPRE_IPPRE_9C_reg,0x00000000);
			rtd_outl(IPPRE_IPPRE_A0_reg,0x00000000);
			rtd_outl(IPPRE_IPPRE_A4_reg,0x00000000);
			rtd_outl(IPPRE_IPPRE_A8_reg,0x00000000);
			rtd_outl(IPPRE_IPPRE_AC_reg,0x00000000);
			rtd_outl(IPPRE_IPPRE_B0_reg,0x05000000);
			rtd_outl(KPOST_TOP_KPOST_TOP_80_reg,0x00000000);
			rtd_outl(KPOST_TOP_KPOST_TOP_84_reg,0x00000000);
			rtd_outl(KPOST_TOP_KPOST_TOP_88_reg,0x00000000);
			rtd_outl(KPOST_TOP_KPOST_TOP_8C_reg,0x00000000);
			rtd_outl(KPOST_TOP_KPOST_TOP_90_reg,0x00000000);
			rtd_outl(KPOST_TOP_KPOST_TOP_94_reg,0x07000000);
		}
		else {
			ColorTable = GetColorTable(u8_scale,u8_table_id);
			
			WriteRegister(IPPRE_IPPRE_80_reg,  0, 14,ColorTable.RGB2YUV.R00);
			WriteRegister(IPPRE_IPPRE_80_reg, 15, 29,ColorTable.RGB2YUV.R01);
			WriteRegister(IPPRE_IPPRE_84_reg,  0, 14,ColorTable.RGB2YUV.R02);
			WriteRegister(IPPRE_IPPRE_84_reg, 15, 29,ColorTable.RGB2YUV.R10);
			WriteRegister(IPPRE_IPPRE_88_reg,  0, 14,ColorTable.RGB2YUV.R11);
			WriteRegister(IPPRE_IPPRE_88_reg, 15, 29,ColorTable.RGB2YUV.R12);
			WriteRegister(IPPRE_IPPRE_8C_reg,  0, 14,ColorTable.RGB2YUV.R20);
			WriteRegister(IPPRE_IPPRE_8C_reg, 15, 29,ColorTable.RGB2YUV.R21);
			WriteRegister(IPPRE_IPPRE_90_reg,  0, 14,ColorTable.RGB2YUV.R22);
			WriteRegister(IPPRE_IPPRE_90_reg, 15, 26,ColorTable.RGB2YUV.T0);
			WriteRegister(IPPRE_IPPRE_94_reg,  0, 11,ColorTable.RGB2YUV.T1);
			WriteRegister(IPPRE_IPPRE_94_reg, 12, 23,ColorTable.RGB2YUV.T2);
			
			WriteRegister(IPPRE_IPPRE_9C_reg,  0, 14,ColorTable.RGB2YUV.R00);
			WriteRegister(IPPRE_IPPRE_9C_reg, 15, 29,ColorTable.RGB2YUV.R01);
			WriteRegister(IPPRE_IPPRE_A0_reg,  0, 14,ColorTable.RGB2YUV.R02);
			WriteRegister(IPPRE_IPPRE_A0_reg, 15, 29,ColorTable.RGB2YUV.R10);
			WriteRegister(IPPRE_IPPRE_A4_reg,  0, 14,ColorTable.RGB2YUV.R11);
			WriteRegister(IPPRE_IPPRE_A4_reg, 15, 29,ColorTable.RGB2YUV.R12);
			WriteRegister(IPPRE_IPPRE_A8_reg,  0, 14,ColorTable.RGB2YUV.R20);
			WriteRegister(IPPRE_IPPRE_A8_reg, 15, 29,ColorTable.RGB2YUV.R21);
			WriteRegister(IPPRE_IPPRE_AC_reg,  0, 14,ColorTable.RGB2YUV.R22);
			WriteRegister(IPPRE_IPPRE_AC_reg, 15, 26,ColorTable.RGB2YUV.T0);
			WriteRegister(IPPRE_IPPRE_B0_reg,  0, 11,ColorTable.RGB2YUV.T1);
			WriteRegister(IPPRE_IPPRE_B0_reg, 12, 23,ColorTable.RGB2YUV.T2);
			
			WriteRegister(KPOST_TOP_KPOST_TOP_80_reg,  0, 14,ColorTable.YUV2RGB.R00);
			WriteRegister(KPOST_TOP_KPOST_TOP_80_reg, 15, 29,ColorTable.YUV2RGB.R01);
			WriteRegister(KPOST_TOP_KPOST_TOP_84_reg,  0, 14,ColorTable.YUV2RGB.R02);
			WriteRegister(KPOST_TOP_KPOST_TOP_84_reg, 15, 29,ColorTable.YUV2RGB.R10);
			WriteRegister(KPOST_TOP_KPOST_TOP_88_reg,  0, 14,ColorTable.YUV2RGB.R11);
			WriteRegister(KPOST_TOP_KPOST_TOP_88_reg, 15, 29,ColorTable.YUV2RGB.R12);
			WriteRegister(KPOST_TOP_KPOST_TOP_8C_reg,  0, 14,ColorTable.YUV2RGB.R20);
			WriteRegister(KPOST_TOP_KPOST_TOP_8C_reg, 15, 29,ColorTable.YUV2RGB.R21);
			WriteRegister(KPOST_TOP_KPOST_TOP_90_reg,  0, 14,ColorTable.YUV2RGB.R22);
			WriteRegister(KPOST_TOP_KPOST_TOP_90_reg, 15, 26,ColorTable.YUV2RGB.T0);
			WriteRegister(KPOST_TOP_KPOST_TOP_94_reg,  0, 11,ColorTable.YUV2RGB.T1);
			WriteRegister(KPOST_TOP_KPOST_TOP_94_reg, 12, 23,ColorTable.YUV2RGB.T2);
			
			WriteRegister(IPPRE_IPPRE_94_reg,25,28,0x8);
			WriteRegister(IPPRE_IPPRE_B0_reg,25,28,0x8);
			WriteRegister(KPOST_TOP_KPOST_TOP_94_reg,25,28,0x8);

		}
		if(log_en) {
			rtd_pr_memc_emerg("[%d][%d]\n",u8_table_cnt, u8_table_id );
		}
	}
	else if(force_Trans || u1_SportScene==true) {
		u8_table_id = u8_table_cnt/u8_keepLen;
		u8_table_cnt = (u8_table_cnt>=(u8_keepLen*u8_scale)-1) ? (u8_keepLen*u8_scale)-1 : u8_table_cnt+1;
		ColorTable = GetColorTable(u8_scale,u8_table_id);

		WriteRegister(IPPRE_IPPRE_80_reg,  0, 14,ColorTable.RGB2YUV.R00);
		WriteRegister(IPPRE_IPPRE_80_reg, 15, 29,ColorTable.RGB2YUV.R01);
		WriteRegister(IPPRE_IPPRE_84_reg,  0, 14,ColorTable.RGB2YUV.R02);
		WriteRegister(IPPRE_IPPRE_84_reg, 15, 29,ColorTable.RGB2YUV.R10);
		WriteRegister(IPPRE_IPPRE_88_reg,  0, 14,ColorTable.RGB2YUV.R11);
		WriteRegister(IPPRE_IPPRE_88_reg, 15, 29,ColorTable.RGB2YUV.R12);
		WriteRegister(IPPRE_IPPRE_8C_reg,  0, 14,ColorTable.RGB2YUV.R20);
		WriteRegister(IPPRE_IPPRE_8C_reg, 15, 29,ColorTable.RGB2YUV.R21);
		WriteRegister(IPPRE_IPPRE_90_reg,  0, 14,ColorTable.RGB2YUV.R22);
		WriteRegister(IPPRE_IPPRE_90_reg, 15, 26,ColorTable.RGB2YUV.T0);
		WriteRegister(IPPRE_IPPRE_94_reg,  0, 11,ColorTable.RGB2YUV.T1);
		WriteRegister(IPPRE_IPPRE_94_reg, 12, 23,ColorTable.RGB2YUV.T2);

		WriteRegister(IPPRE_IPPRE_9C_reg,  0, 14,ColorTable.RGB2YUV.R00);
		WriteRegister(IPPRE_IPPRE_9C_reg, 15, 29,ColorTable.RGB2YUV.R01);
		WriteRegister(IPPRE_IPPRE_A0_reg,  0, 14,ColorTable.RGB2YUV.R02);
		WriteRegister(IPPRE_IPPRE_A0_reg, 15, 29,ColorTable.RGB2YUV.R10);
		WriteRegister(IPPRE_IPPRE_A4_reg,  0, 14,ColorTable.RGB2YUV.R11);
		WriteRegister(IPPRE_IPPRE_A4_reg, 15, 29,ColorTable.RGB2YUV.R12);
		WriteRegister(IPPRE_IPPRE_A8_reg,  0, 14,ColorTable.RGB2YUV.R20);
		WriteRegister(IPPRE_IPPRE_A8_reg, 15, 29,ColorTable.RGB2YUV.R21);
		WriteRegister(IPPRE_IPPRE_AC_reg,  0, 14,ColorTable.RGB2YUV.R22);
		WriteRegister(IPPRE_IPPRE_AC_reg, 15, 26,ColorTable.RGB2YUV.T0);
		WriteRegister(IPPRE_IPPRE_B0_reg,  0, 11,ColorTable.RGB2YUV.T1);
		WriteRegister(IPPRE_IPPRE_B0_reg, 12, 23,ColorTable.RGB2YUV.T2);

		WriteRegister(KPOST_TOP_KPOST_TOP_80_reg,  0, 14,ColorTable.YUV2RGB.R00);
		WriteRegister(KPOST_TOP_KPOST_TOP_80_reg, 15, 29,ColorTable.YUV2RGB.R01);
		WriteRegister(KPOST_TOP_KPOST_TOP_84_reg,  0, 14,ColorTable.YUV2RGB.R02);
		WriteRegister(KPOST_TOP_KPOST_TOP_84_reg, 15, 29,ColorTable.YUV2RGB.R10);
		WriteRegister(KPOST_TOP_KPOST_TOP_88_reg,  0, 14,ColorTable.YUV2RGB.R11);
		WriteRegister(KPOST_TOP_KPOST_TOP_88_reg, 15, 29,ColorTable.YUV2RGB.R12);
		WriteRegister(KPOST_TOP_KPOST_TOP_8C_reg,  0, 14,ColorTable.YUV2RGB.R20);
		WriteRegister(KPOST_TOP_KPOST_TOP_8C_reg, 15, 29,ColorTable.YUV2RGB.R21);
		WriteRegister(KPOST_TOP_KPOST_TOP_90_reg,  0, 14,ColorTable.YUV2RGB.R22);
		WriteRegister(KPOST_TOP_KPOST_TOP_90_reg, 15, 26,ColorTable.YUV2RGB.T0);
		WriteRegister(KPOST_TOP_KPOST_TOP_94_reg,  0, 11,ColorTable.YUV2RGB.T1);
		WriteRegister(KPOST_TOP_KPOST_TOP_94_reg, 12, 23,ColorTable.YUV2RGB.T2);

		WriteRegister(IPPRE_IPPRE_94_reg,25,28,0x8);
		WriteRegister(IPPRE_IPPRE_B0_reg,25,28,0x8);
		WriteRegister(KPOST_TOP_KPOST_TOP_94_reg,25,28,0x8);

		if(log_en) {
			rtd_pr_memc_emerg("[%d][%d]\n",u8_table_cnt, u8_table_id );
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
extern VOID MEMC_Lib_Freeze(unsigned char enable);
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
	static unsigned char MEMC_ID_cnt = 0;

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
		MEMC_Lib_Freeze(1);
	else
		MEMC_Lib_Freeze(0);
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
	//normal check
	MEMC_me1_bgmask_newalgo_WrtAction(pParam,pOutput);	
	MEMC_dehalo_newalgo_bgmaskBase_WrtAction (pParam);
	//rtd_pr_memc_info("MEMC_me1_bgmask_newalgo_write");
	Mid_Mode_MEDEN_Checking();

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
}
