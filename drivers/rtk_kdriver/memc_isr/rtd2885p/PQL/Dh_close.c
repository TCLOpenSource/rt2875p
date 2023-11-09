#include "memc_isr/PQL/PQLPlatformDefs.h"
#include "memc_isr/PQL/PQLContext.h"
#include "memc_isr/PQL/PQLGlobalDefs.h"
#include "memc_isr/PQL/FRC_glb_Context.h"
#include "memc_reg_def.h"


extern unsigned int vpq_project_id;
extern unsigned int vpqex_project_id;
///////////////////////////////////////////////////////////////////////////////////////////
VOID Dh_close_action(const _PARAM_DH_CLOSE *pParam, _OUTPUT_DH_CLOSE *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	// dh close action
	if(pOutput->u1_closeDh_sig == 1)
	{
		pOutput->u1_closeDh_act    = 1;
        if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_fadeinout_dhbypass == 1)
        {
            pOutput->u8_dhOff_holdFrm  = 5;
        }
        else
        {
            pOutput->u8_dhOff_holdFrm  = pParam->u8_dhOff_holdFrm;
        }
		
	}
	else if(pOutput->u8_dhOff_holdFrm > 0)
	{
		pOutput->u1_closeDh_act    = 1;
		pOutput->u8_dhOff_holdFrm  = pOutput->u8_dhOff_holdFrm - 1;
	}
	else
	{
		pOutput->u1_closeDh_act    = 0;
		pOutput->u8_dhOff_holdFrm  = 0;
	}

	//need move to ME action
//	if ((pOutput->u1_panning_close == 1) && (pParam->u1_panning_en == 1))
//	{
//		WriteRegister(FRC_TOP__KME_ME2_TOP_REG__reg_me2_ph_invalid_sel_ADDR no mat,FRC_TOP__KME_ME2_TOP_REG__reg_me2_ph_invalid_sel_BITSTART no mat,FRC_TOP__KME_ME2_TOP_REG__reg_me2_ph_invalid_sel_BITEND no mat, 1);  //gmv
//		}
//		else
//		{
//		WriteRegister(FRC_TOP__KME_ME2_TOP_REG__reg_me2_ph_invalid_sel_ADDR no mat,FRC_TOP__KME_ME2_TOP_REG__reg_me2_ph_invalid_sel_BITSTART no mat,FRC_TOP__KME_ME2_TOP_REG__reg_me2_ph_invalid_sel_BITEND no mat, 3);  //pfv1
//	}
}

VOID Dh_closeSignal_Detect(const _PARAM_DH_CLOSE *pParam, _OUTPUT_DH_CLOSE *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	_PQLPARAMETER *s_pParam       = GetPQLParameter();
	unsigned char u8_cadenceId            = _CAD_VIDEO; //s_pContext->_output_filmDetectctrl.u8_CurCadence;

	unsigned char u1_fblvl_dhClose = 0, u1_meAllDtl_dhClose = 0;
	unsigned char u1_fvconf_dhClose = 0;
	unsigned char u1_lbmc_switch_dhClose=0;
	unsigned char u1_dh_fadeInOut_close = 0;
	unsigned int u25_meAllDtl    = s_pContext->_output_read_comreg.u25_me_aDTL_rb;
	static unsigned int  u25_avgDTL_pre,u32_gmv_cnt_pre,u32_gmv2_cnt_pre,u26_avgAPLi_pre;
	unsigned int u25_avgDTL =0;
	unsigned int u32_gmv_cnt = s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb;
	unsigned int u32_gmv2_cnt = s_pContext->_output_read_comreg.u17_me_GMV_2nd_cnt_rb;
	unsigned char u1_Swing_close=0;
	unsigned int u26_aAPLi_rb=s_pContext->_output_read_comreg.u26_me_aAPLi_rb;
	unsigned int reg_08_dh_off2on_rb_en = 0;
       unsigned int  special_pattern_flag = 0;
       unsigned int  special_pattern_227_flag = 0, special_pattern_49_flag = 0,special_pattern_421_flag = 0;
	#if PQL_DYNAMIC_ADJUST_FIX_ISSUE
	unsigned char u1_panning_close = 0;
	unsigned int debug_index_check = 0;
	unsigned int debug_flag_check = 0;
	unsigned int gmv_cnt_th2=20000,gmv_cnt_th3=12000,avgDTL_th2=1100000,avgDTL_th3=1600000,avgDTL_th5=1300000,gmv_cnt_th5 =29000;//for fix halo issue( set u1_closeDh_sig tobe 0 )
	unsigned int enableBit=0,printlog_en=0;
	#endif

#if RTK_MEMC_Performance_tunging_from_tv001
	unsigned int  special_pattern_eagle_flag = 0;
	unsigned int  special_pattern_transporter_flag = 0;
	unsigned int dhbypass_rotterdam_30s = 0;
	unsigned int special_flag_insect = 0;
	unsigned int  special_pattern_huapi2_flag = 0;
	int u1_is_inFormat_PureVideo_dh = 0;
#endif



	u25_avgDTL = (u25_avgDTL_pre*7 + u25_meAllDtl)>>3;
	u32_gmv_cnt = (u32_gmv_cnt_pre*7 + u32_gmv_cnt)>>3;
	u32_gmv2_cnt= (u32_gmv2_cnt_pre*7 + u32_gmv2_cnt)>>3;
	u26_aAPLi_rb=(u26_avgAPLi_pre*7 + u26_aAPLi_rb)>>3;

	if(u8_cadenceId >= _FRC_CADENCE_NUM_){
		rtd_pr_memc_notice( "[%s]Invalid Cadence:%d,Resume to '_CAD_VIDEO'!\n",__FUNCTION__,u8_cadenceId);
		u8_cadenceId = _CAD_VIDEO;
	}
	// fall back level close dehalo
	if(pParam->u1_fblvl_en == 1)
	{
		unsigned char  u8_dh_fbLvl     = s_pContext->_output_fblevelctrl.u8_Dehalo_FBLevel;
		unsigned char  u8_fbLvl     = s_pContext->_output_fblevelctrl.u8_FBLevel;
		unsigned char  u1_dh_on2off_en = 0;
		unsigned char  u1_dh_off2on_en = 0;
		u8_dh_fbLvl =( u8_dh_fbLvl +u8_fbLvl)>>1;
		u1_dh_on2off_en = u8_dh_fbLvl >= pParam->u8_fbLvl_Th0? 1 : 0; //128
		u1_dh_off2on_en = u8_dh_fbLvl <= pParam->u8_fbLvl_Th1? 1 : 0;  //64

        //   rtd_pr_memc_debug("u8_dh_fbLvl=%d u8_fbLvl=%d u8_fbLvl_Th0=%d u8_fbLvl_Th1=%d\n",u8_dh_fbLvl,u8_fbLvl, pParam->u8_fbLvl_Th0,pParam->u8_fbLvl_Th1);
              ReadRegister(HARDWARE_HARDWARE_25_reg, 21,21 , &special_pattern_flag); 
              ReadRegister(HARDWARE_HARDWARE_25_reg, 22,22 , &special_pattern_227_flag); 
              ReadRegister(HARDWARE_HARDWARE_25_reg, 24,24 , &special_pattern_49_flag); 

		// for each frame, if the flbk level drive it to dh-close.
		pOutput->u1_fblvl_1frm_close = (pOutput->u1_fblvl_1frm_close == 0 && u1_dh_on2off_en == 1)? 1 : \
			                           ((pOutput->u1_fblvl_1frm_close == 1 && u1_dh_off2on_en == 1)? 0 : pOutput->u1_fblvl_1frm_close);

		// finally, whether flbk level can do dh-close.
		#if RTK_MEMC_Performance_tunging_from_tv001
		if((pOutput->u1_fblvl_1frm_close == 1) && (!special_pattern_flag) && (!special_pattern_227_flag) && (!special_pattern_49_flag) &&(!special_pattern_transporter_flag))
		#else
		if((pOutput->u1_fblvl_1frm_close == 1) && (!special_pattern_flag) && (!special_pattern_227_flag) && (!special_pattern_49_flag))
		#endif
		{
			u1_fblvl_dhClose          = 1;
			pOutput->u8_fblvl_holdFrm = pParam->u8_fblvl_holdFrm;
		}
		else if(pOutput->u8_fblvl_holdFrm > 0)
		{
			u1_fblvl_dhClose          = 1;
			pOutput->u8_fblvl_holdFrm = pOutput->u8_fblvl_holdFrm - 1;
		}
		else
		{
			u1_fblvl_dhClose          = 0;
			pOutput->u8_fblvl_holdFrm = 0;
		}
	}


	ReadRegister(HARDWARE_HARDWARE_06_reg,31,31, &enableBit);
	//blow set register need to cut to memclib.c setRTKHDF()
	

	if(0){//if(enableBit){ //for 0819_pak3, Terrence
		WriteRegister(HARDWARE_HARDWARE_10_reg, 21, 26, 0x30); // org : 0x10  pfvconf_cnt_th   ===3f
		//WriteRegister(HARDWARE_HARDWARE_10_reg, 27, 31, 0x0); // org : 0x10  pfvconf_holdfrm  //for 0819_pak3, Terrence
		//WriteRegister(HARDWARE_HARDWARE_10_reg, 1, 20, 0xd1000); // org : 0x10  pfvconf_holdfrm  //for 0819_pak3, Terrence
		WriteRegister(HARDWARE_HARDWARE_11_reg, 9, 13, 0x1); // org : 0x8  dh_fadeInOut_holdfrm		
		
		WriteRegister(HARDWARE_HARDWARE_28_reg, 7, 11, 0xf);  //u5_fadeInOut_dh_close_thr ====1f==>f

		
	}else{
		WriteRegister(HARDWARE_HARDWARE_10_reg, 21, 26, 0x10); // org : 0x10  pfvconf_cnt_th
		//WriteRegister(HARDWARE_HARDWARE_10_reg, 27, 31, 0x3); // org : 0x10  pfvconf_holdfrm  //for 0819_pak3, Terrence
		//WriteRegister(HARDWARE_HARDWARE_10_reg, 1, 20, 0x18000); // org : 0x10  pfvconf_holdfrm  //for 0819_pak3, Terrence
		WriteRegister(HARDWARE_HARDWARE_11_reg, 9, 13, 0x8); // org : 0x8  dh_fadeInOut_holdfrm		

		WriteRegister(HARDWARE_HARDWARE_28_reg, 7, 11, 0x11);//u5_fadeInOut_dh_close_thr  //change e to 11 for 214 198 128...
		
	}
	
	

	if(pParam->u1_dh_fadeInOut_en == 1){
		
	    	if(s_pContext->_output_frc_sceneAnalysis.u1_fadeInOut_dh_close == 1){
				
			u1_dh_fadeInOut_close          = 1;
			if( ( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
				pOutput->u5_dh_fadeInOut_holdfrm = pParam->u5_dh_fadeInOut_holdfrm;
			else
				pOutput->u5_dh_fadeInOut_holdfrm = (pParam->u5_dh_fadeInOut_holdfrm) <<2;

			//rtd_pr_memc_emerg("====[jimmy_wei]=u1_dh_fadeInOut_en == 1;==== u5_dh_fadeInOut_holdfrm:%d ==\n",pOutput->u5_dh_fadeInOut_holdfrm);
			
		}else if(pOutput->u5_dh_fadeInOut_holdfrm > 0){
		
			u1_dh_fadeInOut_close          = 1;
			if(s_pParam->_param_frc_sceneAnalysis.u1_fdIO_sc_clear_en == 1)
				pOutput->u5_dh_fadeInOut_holdfrm = (s_pContext->_output_read_comreg.u1_sc_status_dh_rb==1 || s_pContext->_output_read_comreg.u1_sc_status_rb == 1 || s_pContext->_output_me_sceneAnalysis.u2_panning_flag >= 1) ? pOutput->u5_dh_fadeInOut_holdfrm>>2 : pOutput->u5_dh_fadeInOut_holdfrm - 1;
			else
				pOutput->u5_dh_fadeInOut_holdfrm = pOutput->u5_dh_fadeInOut_holdfrm - 1;
			
		}else{
		
		   	 u1_dh_fadeInOut_close          = 0;
		   	 pOutput->u5_dh_fadeInOut_holdfrm = 0;
			
	    	}
	}

	// sml rgn action
	if(pParam->u1_dh_sml_rgnAction_en == 1)
	{
	    Dh_close_sml_rgnAction(pParam, pOutput);
	}
	else
	{
		pOutput->u1_dh_sml_rgnAction_close = 0;
	}

	// me_allDtl close dehalo
	if (pParam->u1_meAllDtl_en == 1)
	{
		unsigned int u32_rimRatio = (s_pContext->_output_rimctrl.u32_rimRatio == 0) ? 128 : s_pContext->_output_rimctrl.u32_rimRatio;
		unsigned char  u1_dh_on2off_en = u25_meAllDtl <= ((pParam->u25_meAllDtl_Th0 * u32_rimRatio) >> 7)? 1 : 0;
		unsigned char  u1_dh_off2on_en = u25_meAllDtl >= ((pParam->u25_meAllDtl_Th1 * u32_rimRatio) >> 7)? 1 : 0;
		//--------------------for hisense pattern#08 insect head broken----start--------------//
#if RTK_MEMC_Performance_tunging_from_tv001
		bool condition_insect = 0;// hisense #08_insect
#else
		unsigned char condition_insect = 0;// hisense #08_insect
#endif
		unsigned char condition1 = 0;
		unsigned int rDTL[32] = {0};
		int i = 0;
		int cnt = 0;
		
		for(i=0; i<32; i++)
		{
			rDTL[i] = s_pContext->_output_read_comreg.u20_me_rDTL_rb[i];
			if(rDTL[i] < 500)
			{
				cnt++;
			}
		
		}
	
#if RTK_MEMC_Performance_tunging_from_tv001	
		ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,20, 20, &special_flag_insect);
		ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 17, 17, &special_pattern_eagle_flag);	
#endif

#if RTK_MEMC_Performance_tunging_from_tv001
	condition_insect =(( (rDTL[31] < 6000) && (rDTL[24] < 6000) && (cnt >= 13))||(special_flag_insect==1));
#else
	condition_insect = (rDTL[31] < 6000) && (rDTL[24] < 6000) && (cnt >= 13);	
#endif
	
		ReadRegister(HARDWARE_HARDWARE_26_reg,20,20, &reg_08_dh_off2on_rb_en);//0xB809D568
		condition1 = reg_08_dh_off2on_rb_en ? (!condition_insect) : 1;
		if(reg_08_dh_off2on_rb_en)
		{
			if(condition_insect == 1)
			{
				//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 0, 3, 0x1);
			}
			else
			{
				//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 0, 3, 0x0);
			}
		}
		//--------------------for hisense pattern#08 insect head broken----end--------------//

		u1_dh_on2off_en = u1_dh_on2off_en && condition1;
		u1_dh_off2on_en = (reg_08_dh_off2on_rb_en)? condition_insect : u1_dh_off2on_en;

		u1_meAllDtl_dhClose = (pOutput->u1_meAllDtl_close == 0 && u1_dh_on2off_en == 1)? 1 : \
							  ((pOutput->u1_meAllDtl_close == 1 && u1_dh_off2on_en == 1)? 0 : pOutput->u1_meAllDtl_close);

#if RTK_MEMC_Performance_tunging_from_tv001	
		u1_meAllDtl_dhClose = special_pattern_eagle_flag?0:u1_meAllDtl_dhClose;
		u1_meAllDtl_dhClose = special_flag_insect?0:u1_meAllDtl_dhClose;
#endif

	}

	//bad pfvconf
	if (pParam->u1_dh_pfvconf_en == 1)
	{
		int i=0;
		int bad_conf_num = 0;
		for (i=0; i<32; i++)
		{
			unsigned int pfvconf = s_pContext->_output_read_comreg.u20_dh_pfvconf[i];
			if (pfvconf > pParam->u20_dh_pfvconf_thrL)
			{
				bad_conf_num++;
			}
		}

		if (bad_conf_num >= pParam->u6_dh_pfvconf_cnt_thr)//u6_dh_pfvconf_cnt_thr should be change from 16 to 32 for dehalo.
		{			
			pOutput->u1_pfvconf_close = 1;
			//rtd_pr_memc_emerg("\n====[jimmy_we] bad_conf_num:%d====\n",bad_conf_num);
		}
		else
		{
			pOutput->u1_pfvconf_close = 0;
		}

		// finally, whether flbk level can do dh-close.
		if(pOutput->u1_pfvconf_close == 1)
		{
			u1_fvconf_dhClose = 1;
			pOutput->u8_pfvconf_holdFrm = pParam->u5_dh_pfvconf_holdfrm;
		}
		else if(pOutput->u8_pfvconf_holdFrm > 0)
		{
			u1_fvconf_dhClose           = 1;
			pOutput->u8_pfvconf_holdFrm = pOutput->u8_pfvconf_holdFrm - 1;
		}
		else
		{
			u1_fvconf_dhClose           = 0;
			pOutput->u8_pfvconf_holdFrm = 0;
		}
	}

	//LBMC mode switch
	if (pParam->u1_lbmc_switch_en == 1)
	{

#if RTK_MEMC_Performance_tunging_from_tv001
		ReadRegister(HARDWARE_HARDWARE_25_reg, 16,16 , &special_pattern_421_flag);
		ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 17, 17, &special_pattern_eagle_flag);
		ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 18, 18, &special_pattern_transporter_flag);
		ReadRegister(HARDWARE_HARDWARE_43_reg, 30, 30, &dhbypass_rotterdam_30s);
		ReadRegister(SOFTWARE3_SOFTWARE3_20_reg, 30, 30, &special_pattern_huapi2_flag);
		ReadRegister(HARDWARE_HARDWARE_25_reg, 22,22 , &special_pattern_227_flag);
		
		if((((pOutput->u8_lbmc_mode == _MC_NORMAL_) && (s_pContext->_output_mc_lbmcswitch.u8_lf_lbmcMode == _MC_SINGLE_UP_)) ||
		   ((pOutput->u8_lbmc_mode == _MC_NORMAL_) && (s_pContext->_output_mc_lbmcswitch.u8_lf_lbmcMode == _MC_SINGLE_DOWN_)))&& (!special_pattern_eagle_flag)&&(!special_pattern_transporter_flag)&&(!dhbypass_rotterdam_30s)&&(!special_pattern_huapi2_flag) && (!special_pattern_227_flag)&&(!special_pattern_421_flag))
		{
			u1_lbmc_switch_dhClose = 1;
			pOutput->u8_lbmc_switch_holdfrm = pParam->u5_lbmc_switch_holdfrm ;
		}
		else if(pOutput->u8_lbmc_switch_holdfrm > 0)
		{
			u1_lbmc_switch_dhClose = 1;
			pOutput->u8_lbmc_switch_holdfrm = pOutput->u8_lbmc_switch_holdfrm - 1;
		}
		else
		{
			u1_lbmc_switch_dhClose = 0;
			pOutput->u8_lbmc_switch_holdfrm = 0;
		}
#else
		ReadRegister(HARDWARE_HARDWARE_25_reg, 22,22 , &special_pattern_227_flag);
        ReadRegister(HARDWARE_HARDWARE_25_reg, 16,16 , &special_pattern_421_flag);
            
		if((((pOutput->u8_lbmc_mode == _MC_NORMAL_) && (s_pContext->_output_mc_lbmcswitch.u8_lf_lbmcMode == _MC_SINGLE_UP_)) ||
		   ((pOutput->u8_lbmc_mode == _MC_NORMAL_) && (s_pContext->_output_mc_lbmcswitch.u8_lf_lbmcMode == _MC_SINGLE_DOWN_))) && (!special_pattern_227_flag)&&(!special_pattern_421_flag))
		{
			u1_lbmc_switch_dhClose = 1;
			pOutput->u8_lbmc_switch_holdfrm = pParam->u5_lbmc_switch_holdfrm ;
		}
		else if(pOutput->u8_lbmc_switch_holdfrm > 0)
		{
			u1_lbmc_switch_dhClose = 1;
			pOutput->u8_lbmc_switch_holdfrm = pOutput->u8_lbmc_switch_holdfrm - 1;
		}
		else
		{
			u1_lbmc_switch_dhClose = 0;
			pOutput->u8_lbmc_switch_holdfrm = 0;
		}	
#endif

	

	}
	else
	{
			u1_lbmc_switch_dhClose = 0;
			pOutput->u8_lbmc_switch_holdfrm = 0;
	}

	//Panning
	if (pParam->u1_panning_en == 1)
	{
		#if PQL_DYNAMIC_ADJUST_FIX_ISSUE		

		
		ReadRegister(HARDWARE_HARDWARE_06_reg,30,30, &printlog_en);//for 0819_pak3, Terrence
		
		if(enableBit == 1){
#if RTK_MEMC_Performance_tunging_from_tv001
			gmv_cnt_th2 = 30000;//20000 //26000
			gmv_cnt_th3 = 30000;//12000--22000--25000 //25000	
#else
			gmv_cnt_th2 = 30500;//20000 //26000
			gmv_cnt_th3 = 30500;//12000--22000--25000 //25000	
#endif
			avgDTL_th2 = 1500000;//1100000
			avgDTL_th3 = 2000000;//1600000
			avgDTL_th5 = 1000000;//1300000
			gmv_cnt_th5 =20000;//29000
		}
		
		//if(s_pContext->_output_frc_sceneAnalysis.u8_is_insidePan == 1)
		if((s_pContext->_output_me_sceneAnalysis.u2_panning_flag== 1 || s_pContext->_output_me_sceneAnalysis.u2_panning_flag== 2 ) && u25_avgDTL <500000)
		{
			u1_panning_close = 1;
			pOutput->u8_panning_holdfrm = 4/*pParam->u5_panning_holdfrm */;
			if(printlog_en)
				rtd_pr_memc_emerg("====[1]====\n");
		}
		else if((s_pContext->_output_me_sceneAnalysis.u2_panning_flag== 1 || s_pContext->_output_me_sceneAnalysis.u2_panning_flag== 2 )  && u32_gmv_cnt >gmv_cnt_th2 && u25_avgDTL >avgDTL_th2)
		{
			u1_panning_close = 1;
			pOutput->u8_panning_holdfrm = 4/*pParam->u5_panning_holdfrm */;
			if(printlog_en)
				rtd_pr_memc_emerg("====[2]====gmv:%d > %d, avgDTL:%d > %d\n",u32_gmv_cnt,gmv_cnt_th2,u25_avgDTL,avgDTL_th2);
		}
		else if(s_pContext->_output_me_sceneAnalysis.u2_panning_flag== 1  && u32_gmv_cnt >gmv_cnt_th3&& u25_avgDTL >avgDTL_th3)
		{
			u1_panning_close = 1;
			pOutput->u8_panning_holdfrm = 4/*pParam->u5_panning_holdfrm */;
			if(printlog_en)
				rtd_pr_memc_emerg("====[3]====gmv:%d > %d,avgDTL:%d > %d\n",u32_gmv_cnt,gmv_cnt_th3,u25_avgDTL,avgDTL_th3);
		}
		else if((s_pContext->_output_me_sceneAnalysis.u2_panning_flag== 1 || s_pContext->_output_me_sceneAnalysis.u2_panning_flag== 2 )  && u32_gmv2_cnt >500 && u25_avgDTL >1300000&&u32_gmv_cnt >16000 && u32_gmv_cnt <29000 && u26_aAPLi_rb <2000000 && u26_aAPLi_rb>1200000)
		{
			u1_panning_close = 1;
			pOutput->u8_panning_holdfrm = 4/*pParam->u5_panning_holdfrm */;
			//rtd_pr_memc_emerg("====[4]====\n");
		}
		else if(s_pContext->_output_me_sceneAnalysis.u2_panning_flag   && u25_avgDTL >500000 && u25_avgDTL <avgDTL_th5&&u32_gmv_cnt >16000 && u32_gmv_cnt <gmv_cnt_th5 && u26_aAPLi_rb<2100000)
		{
			u1_panning_close = 1;
			pOutput->u8_panning_holdfrm = 4/*pParam->u5_panning_holdfrm */;
			if(printlog_en)
				rtd_pr_memc_emerg("====[5]====gmv:%d ,avgDTL:%d \n",u32_gmv_cnt,u25_avgDTL);
		}
		else if(pOutput->u8_panning_holdfrm > 0)
		{
			u1_panning_close = 1;
			pOutput->u8_panning_holdfrm = pOutput->u8_panning_holdfrm - 1;
			if(printlog_en)
				rtd_pr_memc_emerg("====[6...]====\n");
		}
		else
		#endif
		{
			u1_panning_close = 0;
			if(printlog_en)
				rtd_pr_memc_emerg("====[0]====\n");
		}
	}
	else
	{
		u1_panning_close = 0;
	}

	//Simple Scene
	if (pParam->u1_simple_scene_en == 1)
	{
		unsigned int simple_scene_score_pre = ((s_pContext->_output_me_sceneAnalysis.u16_simple_scene_score_pre+128)>>8);
		
		//--------------------for hisense pattern#08 insect head broken----start--------------//
		unsigned char condition_insect = 0;// hisense #08_insect
		unsigned char condition1 = 0;
		unsigned int rDTL[32] = {0};
		int i = 0;
		int cnt = 0;

#if RTK_MEMC_Performance_tunging_from_tv001
		ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,20, 20, &special_flag_insect);
#endif
		
		for(i=0; i<32; i++)
		{
			rDTL[i] = s_pContext->_output_read_comreg.u20_me_rDTL_rb[i];
			if(rDTL[i] < 500)
			{
				cnt++;
			}
		}
		condition_insect = (rDTL[31] < 6000) && (rDTL[24] < 6000) && (cnt >= 13);
		ReadRegister(HARDWARE_HARDWARE_26_reg,20,20, &reg_08_dh_off2on_rb_en);//0xB809D568
		condition1 = reg_08_dh_off2on_rb_en ? (!condition_insect) : 1;
		//--------------------for hisense pattern#08 insect head broken----end--------------//
		
		if (simple_scene_score_pre > 160 && condition1)
		{
			pOutput->u1_dh_simple_scene_close = 1;
		}
		else if(condition_insect)
		{
			pOutput->u1_dh_simple_scene_close = 0;
		}
		else
		{
			pOutput->u1_dh_simple_scene_close = 0;
		}

#if RTK_MEMC_Performance_tunging_from_tv001
		pOutput->u1_dh_simple_scene_close = special_flag_insect?0 : pOutput->u1_dh_simple_scene_close;
#endif

		
	}

	if(s_pContext->_output_frc_sceneAnalysis.u1_Swing_true && u25_avgDTL >2200000)
	{
		u1_Swing_close=1;
	}
	else
	{
		u1_Swing_close=0;
	}

	u25_avgDTL_pre = u25_avgDTL;
	u32_gmv_cnt_pre = u32_gmv_cnt;
	u32_gmv2_cnt_pre = u32_gmv2_cnt;
	u26_avgAPLi_pre = u26_aAPLi_rb;

#if RTK_MEMC_Performance_tunging_from_tv001

	if(1)
	{
		int special_pattern_transporter_flag = 0;
		int special_pattern_huapi2_flag = 0;
		ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 18, 18, &special_pattern_transporter_flag);
		ReadRegister(SOFTWARE3_SOFTWARE3_20_reg, 30, 30, &special_pattern_huapi2_flag);

		u1_is_inFormat_PureVideo_dh = ((s_pContext->_output_frc_phtable.u1_is_inFormat_PureVideo) &&  (special_pattern_transporter_flag!=1)&&  (special_pattern_huapi2_flag!=1));

	}
#endif


	pOutput->u1_panning_close	 = (u1_panning_close && s_pContext->_output_me_sceneAnalysis.u1_MainObject_Move == 0);
	pOutput->u8_lbmc_mode        = s_pContext->_output_mc_lbmcswitch.u8_lf_lbmcMode;
	pOutput->u1_lbmc_switch_close= u1_lbmc_switch_dhClose;
	pOutput->u1_fblvl_close      = u1_fblvl_dhClose;
	pOutput->u1_meAllDtl_close   = u1_meAllDtl_dhClose;
	pOutput->u1_pfvconf_close    = u1_fvconf_dhClose;
	pOutput->u1_dh_fadeInOut_close = u1_dh_fadeInOut_close;

#if RTK_MEMC_Performance_tunging_from_tv001
	pOutput->u1_closeDh_sig      = pOutput->u1_dh_simple_scene_close ||
   								   pOutput->u1_panning_close ||
   								   pOutput->u1_lbmc_switch_close ||
   								   pOutput->u1_fblvl_close ||
								   pOutput->u1_meAllDtl_close ||
								   // pOutput->u1_pfvconf_close ||
								   frc_cadTable[u8_cadenceId].u1_isLowFrameRate ||
								   pOutput->u1_dh_fadeInOut_close == 1 ||
								   pOutput->u1_dh_sml_rgnAction_close == 1 ||
								   s_pContext->_output_frc_sceneAnalysis.u1_fastMotion_det||
								   u1_Swing_close ||
								   u1_is_inFormat_PureVideo_dh == 1||
								   s_pContext->_output_frc_phtable.u1_is_inFormat_PureVideo == 1||
								   s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_dhbypass == 1|| //newalgo bypass
								   s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_fadeinout_dhbypass == 1||
								   s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_occl_chaos1_bypass == 1||
								   s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_dhbypass_add == 1; //20220711djn

#else
	pOutput->u1_closeDh_sig      = pOutput->u1_dh_simple_scene_close ||
   								   pOutput->u1_panning_close ||
   								   pOutput->u1_lbmc_switch_close ||
   								   pOutput->u1_fblvl_close ||
								   pOutput->u1_meAllDtl_close ||
								 //  pOutput->u1_pfvconf_close ||
								   frc_cadTable[u8_cadenceId].u1_isLowFrameRate ||
								   pOutput->u1_dh_fadeInOut_close == 1 ||
								   pOutput->u1_dh_sml_rgnAction_close == 1 ||
								   s_pContext->_output_frc_sceneAnalysis.u1_fastMotion_det||
								   u1_Swing_close ||
								   s_pContext->_output_frc_phtable.u1_is_inFormat_PureVideo == 1||
								   s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_dhbypass == 1|| //newalgo bypass
								   s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_fadeinout_dhbypass == 1||//newalgo fadeinout bypass
								   s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_occl_chaos1_bypass == 1||
								   s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_dhbypass_add == 1; //20220711djn
								   //s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_occl_chaos2_bypass == 1; 

#endif

								   
	
	//pOutput->u1_closeDh_sig      = pOutput->u1_panning_close;
	ReadRegister(HARDWARE_HARDWARE_06_reg,30,30, &printlog_en);
	if(printlog_en){
		if(pOutput->u1_closeDh_sig == 1){//if(pOutput->u1_closeDh_sig == 1)		
			rtd_pr_memc_emerg("\n========start:=============\n");
			/*
			if(pOutput->u1_dh_simple_scene_close)
				rtd_pr_memc_emerg("[jimmy_wei]u1_dh_simple_scene_close:%d\n",pOutput->u1_dh_simple_scene_close);
			if(pOutput->u1_panning_close)
				rtd_pr_memc_emerg("[jimmy_wei]u1_panning_close:%d\n",pOutput->u1_panning_close);
			if(pOutput->u1_lbmc_switch_close)
				rtd_pr_memc_emerg("[jimmy_wei]u1_lbmc_switch_close:%d\n",pOutput->u1_lbmc_switch_close);		
			if(pOutput->u1_fblvl_close)
				rtd_pr_memc_emerg("[jimmy_wei]u1_fblvl_close:%d\n",pOutput->u1_fblvl_close);		
			if(pOutput->u1_meAllDtl_close)
				rtd_pr_memc_emerg("[jimmy_wei]u1_meAllDtl_close:%d\n",pOutput->u1_meAllDtl_close);	
			*/
			if(pOutput->u1_pfvconf_close)
				rtd_pr_memc_emerg("[jimmy_wei]u1_pfvconf_close:%d\n",pOutput->u1_pfvconf_close);				
			if(frc_cadTable[u8_cadenceId].u1_isLowFrameRate)
				rtd_pr_memc_emerg("[jimmy_wei]u1_isLowFrameRate:%d\n",frc_cadTable[u8_cadenceId].u1_isLowFrameRate);	
			if(pOutput->u1_dh_fadeInOut_close)
				rtd_pr_memc_emerg("[jimmy_wei]u1_dh_fadeInOut_close:%d\n",pOutput->u1_dh_fadeInOut_close);	
			if(pOutput->u1_dh_sml_rgnAction_close)
				rtd_pr_memc_emerg("[jimmy_wei]u1_dh_sml_rgnAction_close:%d\n",pOutput->u1_dh_sml_rgnAction_close);
			if(s_pContext->_output_frc_sceneAnalysis.u1_fastMotion_det)
				rtd_pr_memc_emerg("[jimmy_wei]u1_fastMotion_det:%d\n",s_pContext->_output_frc_sceneAnalysis.u1_fastMotion_det);
			if(u1_Swing_close)
				rtd_pr_memc_emerg("[jimmy_wei]u1_Swing_close:%d\n",u1_Swing_close);
			if(s_pContext->_output_frc_phtable.u1_is_inFormat_PureVideo)
				rtd_pr_memc_emerg("[johning_dai]u1_is_inFormat_PureVideo:%d\n",s_pContext->_output_frc_phtable.u1_is_inFormat_PureVideo);
            if(pOutput->u1_dh_simple_scene_close)
                rtd_pr_memc_emerg("[johning_dai]u1_dh_simple_scene_close:%d\n",pOutput->u1_dh_simple_scene_close);
            if(pOutput->u1_panning_close)
                rtd_pr_memc_emerg("[johning_dai]u1_panning_close:%d\n",pOutput->u1_panning_close);
            if(pOutput->u1_lbmc_switch_close)
                rtd_pr_memc_emerg("[johning_dai]u1_lbmc_switch_close:%d\n",pOutput->u1_lbmc_switch_close);
            if(pOutput->u1_fblvl_close)
                rtd_pr_memc_emerg("[johning_dai]u1_fblvl_close:%d  special_pattern_flag(%d)\n",pOutput->u1_fblvl_close, special_pattern_flag);
            if(pOutput->u1_meAllDtl_close)
                rtd_pr_memc_emerg("[johning_dai]u1_meAllDtl_close:%d\n",pOutput->u1_meAllDtl_close);   
            if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_dhbypass == 1)
                rtd_pr_memc_emerg("[johning_dai]u8_dh_condition_dhbypass:%d\n",s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_dhbypass);  
            if( s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_fadeinout_dhbypass == 1)
                rtd_pr_memc_emerg("[johning_dai]u8_dh_condition_fadeinout_dhbypass:%d\n",s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_fadeinout_dhbypass);
			//k23
            if(s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_dhbypass_add == 1)
                rtd_pr_memc_emerg("[johning_dai]u8_dh_condition_dhbypass_add:%d\n",s_pContext->_output_frc_sceneAnalysis.u8_dh_condition_dhbypass_add);


            
			rtd_pr_memc_emerg("=========over:==============\n");
		}
	}
	if(pParam->u1_dbg_print_en == 1)
	{
		debug_index_check = (pParam->u1_fblvl_en) + (pParam->u1_dh_fadeInOut_en<<1) + (pParam->u1_dh_sml_rgnAction_en<<2) + (pParam->u1_meAllDtl_en<<3) +
							(pParam->u1_dh_pfvconf_en<<4) + (pParam->u1_lbmc_switch_en<<5) + (pParam->u1_panning_en<<6) + (pParam->u1_simple_scene_en<<7);

		debug_flag_check = (pOutput->u1_dh_simple_scene_close) + (pOutput->u1_panning_close<<1) + (pOutput->u1_lbmc_switch_close<<2) + (pOutput->u1_fblvl_close<<3) +
							(pOutput->u1_meAllDtl_close<<4) + (pOutput->u1_pfvconf_close<<5) + (pOutput->u1_dh_fadeInOut_close<<6) + (pOutput->u1_dh_sml_rgnAction_close<<7) +
							(s_pContext->_output_frc_sceneAnalysis.u1_fastMotion_det<<8) + (u1_Swing_close<<9)+( frc_cadTable[u8_cadenceId].u1_isLowFrameRate <<10)+( s_pContext->_output_frc_phtable.u1_is_inFormat_PureVideo<<11);

		rtd_pr_memc_notice( "[%s][%x,%x]\n",__FUNCTION__,debug_index_check,debug_flag_check);
		rtd_pr_memc_notice( "[%s][panning_close][%d,%d]\n",__FUNCTION__,u1_panning_close, s_pContext->_output_me_sceneAnalysis.u1_MainObject_Move);		
		rtd_pr_memc_notice( "[%s]gmv_cnt=%d,u25_avgDTL=%d \n",__FUNCTION__,u32_gmv_cnt,u25_avgDTL);
	}

///////////////////////////////////////////////////////////////////////////////////////////
}

VOID Dh_close_Init(_OUTPUT_DH_CLOSE *pOutput)
{
	pOutput->u8_dhOff_holdFrm         = 0;
	pOutput->u8_lbmc_mode             = _MC_NORMAL_;
	pOutput->u8_lbmc_switch_holdfrm   = 0;
	pOutput->u1_lbmc_switch_close     = 0;

	pOutput->u8_panning_holdfrm   	  = 0;
	pOutput->u1_panning_close         = 0;

	pOutput->u8_fblvl_holdFrm         = 0;
	pOutput->u1_fblvl_1frm_close      = 0;
	pOutput->u8_pfvconf_holdFrm       = 0;
	pOutput->u1_fblvl_close           = 0;
	pOutput->u1_meAllDtl_close        = 0;

	pOutput->u1_closeDh_sig           = 0;
	pOutput->u1_closeDh_act           = 0;

	pOutput->u1_dh_fadeInOut_close    = 0;
	pOutput->u5_dh_fadeInOut_holdfrm  = 0;

	pOutput->u1_dh_sml_rgnAction_close    = 0;
	pOutput->u5_dh_sml_rgnAction_holdfrm  = 0;

	pOutput->u5_dh_good_rgn_num = 0;
	pOutput->u5_dh_bad_rgn_num  = 0;
}

VOID Dh_close_sml_rgnAction(const _PARAM_DH_CLOSE *pParam, _OUTPUT_DH_CLOSE *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	int i = 0;
	//unsigned int u16_cnt_totalNum1 = (s_pContext->_output_read_comreg.u8_me1_Vbuf_Hsize_rb * s_pContext->_output_read_comreg.u8_me1_Vbuf_Vsize_rb);
	//unsigned int u16_cnt_totalNum = u16_cnt_totalNum1 == 0 ? 1 : u16_cnt_totalNum1/32;
	unsigned int u16_cnt_totalNum = ME1_TotalBlkNum_Golden>>5;
	unsigned int u32_rimRatio = (s_pContext->_output_rimctrl.u32_rimRatio == 0) ? 128 : s_pContext->_output_rimctrl.u32_rimRatio;

	pOutput->u5_dh_good_rgn_num = 0;
	pOutput->u5_dh_bad_rgn_num  = 0;

	for(i=0;i<32;i++)
	{
		if( (_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]) < pParam->u8_dh_sml_rgnAction_gmv_x_thr)&&
			(_ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]) < pParam->u8_dh_sml_rgnAction_gmv_y_thr)&&
			(s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[i] > pParam->u12_dh_sml_rgnAction_gmv_cnt_thr)&&
			(s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[i] < pParam->u12_dh_sml_rgnAction_gmv_ucof_thr)&&
			(s_pContext->_output_read_comreg.u25_me_rSAD_rb[i]/u16_cnt_totalNum < pParam->u10_dh_sml_rgnAction_goodrgnsad_thr)
		//	&&(s_pContext->_output_read_comreg.u20_dh_pfvconf[i] < pParam->u19_dh_sml_rgnAction_good_pfvdiff_thr)
			)
		{
		    pOutput->u5_dh_good_rgn_num++;
		}
		if( (s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[i] < pParam->u12_dh_sml_rgnAction_bad_gmv_cnt_thr)&&
			(s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[i] > pParam->u12_dh_sml_rgnAction_bad_gmv_ucof_thr)&&
			(s_pContext->_output_read_comreg.u25_me_rSAD_rb[i]/u16_cnt_totalNum > pParam->u10_dh_sml_rgnAction_badrgnsad_thr)
		//	&&(s_pContext->_output_read_comreg.u20_dh_pfvconf[i] > pParam->u19_dh_sml_rgnAction_bad_pfvdiff_thr)
			)
		{
			pOutput->u5_dh_bad_rgn_num++;
		}
	}
	if( (pOutput->u5_dh_good_rgn_num >= pParam->u5_dh_sml_rgnAction_goodrgn_cnt_thr) &&
		(pOutput->u5_dh_bad_rgn_num >= pParam->u5_dh_sml_rgnAction_badrgn_cnt_thr)&&
		(((s_pContext->_output_read_comreg.u26_me_aAPLi_rb + s_pContext->_output_read_comreg.u26_me_aAPLp_rb)/2/u16_cnt_totalNum) <
		((pParam->u8_dh_sml_rgnAction_apl_thr * u32_rimRatio) >> 7)))
	{
		pOutput->u1_dh_sml_rgnAction_close = 1;
		pOutput->u5_dh_sml_rgnAction_holdfrm = pParam->u5_dh_sml_rgnAction_holdfrm;

	}
	else if(pOutput->u5_dh_sml_rgnAction_holdfrm >0)
	{
		pOutput->u1_dh_sml_rgnAction_close = 1;
		pOutput->u5_dh_sml_rgnAction_holdfrm = pOutput->u5_dh_sml_rgnAction_holdfrm - 1;
	}
	else
	{
		pOutput->u1_dh_sml_rgnAction_close = 0;
		pOutput->u5_dh_sml_rgnAction_holdfrm = 0;
	}

}


VOID Dh_close_Proc(const _PARAM_DH_CLOSE *pParam, _OUTPUT_DH_CLOSE *pOutput)
{
	// dh close signal detect
	if (pParam->u2_dhClose_bypass == 0)
	{
		Dh_closeSignal_Detect(pParam, pOutput);
	}
	else
	{
		pOutput->u1_closeDh_sig   = pParam->u2_dhClose_bypass - 1;
	}

	// dh close action
	Dh_close_action(pParam, pOutput);
	return;
}





