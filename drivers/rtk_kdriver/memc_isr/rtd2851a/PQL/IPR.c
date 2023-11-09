#include "memc_isr/PQL/PQLPlatformDefs.h"
#include "memc_isr/PQL/PQLContext.h"
#include "memc_isr/PQL/PQLGlobalDefs.h"
#include "memc_isr/PQL/IPR.h"
#include "memc_isr/Common/memc_type.h"
#include "memc_isr/HAL/hal_ip.h"
#include "memc_isr/HAL/hal_post.h"
//#include <tvscalercontrol/scaler/scalerstruct.h>
#include "memc_reg_def.h"
#include "rbus/kme_dehalo4_reg.h"
#include <rbus/od_reg.h>
//dehalo me1 normal
#include "common/include/rbus/kme_dehalo5_reg.h"
#include "common/include/rbus/kme_me1_bg0_reg.h"
#include "common/include/rbus/kme_me1_bg1_reg.h"
#include "common/include/rbus/color_reg.h"

#ifndef Start_Print_info_t
typedef struct
{
	// fwused_2
	/*[0:7]*/ unsigned char Delay_Time;
	/*[8 ] */ unsigned char Y_Hist;
	/*[9 ] */ unsigned char Hue_Hist;
	/*[10] */ unsigned char Sat_Mean;
	/*[11] */ unsigned char Dcc_1;
	/*[12] */ unsigned char Dcc_2;
	/*[13] */ unsigned char DCC_3;
	/*[14] */ unsigned char Rtnr_1;
	/*[15] */ unsigned char Rtnr_2;
	/*[16] */ unsigned char Mpeg_Nr;
	/*[17] */ unsigned char Snr;
	/*[18] */ unsigned char Sharpness_1;
	/*[19] */ unsigned char Sharpness_2;
	/*[20] */ unsigned char DI_1;
	/*[21] */ unsigned char DI_2;
	/*[22] */ unsigned char Film_Mode;
	/*[23] */ unsigned char HIST;
	/*[24] */ unsigned char Reverse_2;
	/*[25] */ unsigned char Reverse_3;
	/*[26] */ unsigned char FormattedLog;
	// fwused_3
	/*[0 ]*/ unsigned char ID_Pattern_ALL;
	/*[1 ]*/ unsigned char ID_Pattern_2;
	/*[2 ]*/ unsigned char ID_Pattern_4;
	/*[3 ]*/ unsigned char ID_Pattern_5;
	/*[4 ]*/ unsigned char ID_Pattern_7;
	/*[5 ]*/ unsigned char ID_Pattern_12;
	/*[6 ]*/ unsigned char ID_Pattern_23;
	/*[7 ]*/ unsigned char ID_Pattern_24;
	/*[8 ]*/ unsigned char ID_Pattern_35;
	/*[9 ]*/ unsigned char ID_Pattern_47;
	/*[10]*/ unsigned char ID_Pattern_107;
	/*[11]*/ unsigned char ID_Pattern_123;
	/*[12]*/ unsigned char ID_Pattern_132;
	/*[13]*/ unsigned char ID_Pattern_133;
	/*[14]*/ unsigned char ID_Pattern_154;
	/*[15]*/ unsigned char ID_Pattern_191;
	/*[16]*/ unsigned char ID_Pattern_192;
	/*[17]*/ unsigned char ID_Pattern_193;
	/*[18]*/ unsigned char ID_Pattern_194;
	/*[19]*/ unsigned char ID_Pattern_195;
	/*[20]*/ unsigned char ID_Pattern_139;
	/*[21]*/ unsigned char ID_Pattern_196;
	/*[22]*/ unsigned char ID_Pattern_14;
	/*[23]*/ unsigned char someothers;
	/*[24]*/ unsigned char ID_Pattern_32;
	/*[25]*/ unsigned char ID_Pattern_147;
	/*[26]*/ unsigned char Reverse_10;
	/*[27]*/ unsigned char Reverse_11;
	/*[28]*/ unsigned char VD_LOG_ENABLE;
	/*[29]*/ unsigned char ColorBar;
	/*[30]*/ unsigned char Slow_Motion1;
	/*[31]*/ unsigned char Slow_Motion2;
	/*[32]*/ unsigned char RF_flower;
}Start_Print_info_t;
#endif
extern Start_Print_info_t Start_Print;
//extern Start_Print_info_t Start_Print={1};//for scalerDi.cpp ROSprintrf( %0) issue !!!
#ifndef debug_print_delay_time
#define debug_print_delay_time ((Start_Print.Delay_Time == 0)?(1):(Start_Print.Delay_Time))
#endif

extern int LowDelay_mode;
extern unsigned char u8_MEMCMode;
extern unsigned int dejudder;
extern unsigned int g_cur_cadence_id;
extern unsigned int g_cur_cadence_out;
extern unsigned int vpq_project_id;
extern unsigned int vpqex_project_id;
unsigned char g_Q13722_flag = 0;
unsigned char g_181_flag = 0;
extern unsigned int MA_print_count;
extern unsigned char scalerVIP_Get_MEMCPatternFlag_Identification(unsigned short prj,unsigned short nNumber);
extern unsigned char MEMC_Lib_get_Adaptive_Stream_Flag(VOID);
VOID Identification_Pattern_preProcess_TV001(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	_PQLCONTEXT *s_pContext = GetPQLContext_m();
	static unsigned char  u5_blend_holdfrm_preProcess_a[255] = {0};
	static unsigned char  nIdentifiedNum_preProcess_a[255] = {0};
	unsigned char  u8_Index;
	unsigned int  u32_RB_val;
	unsigned int  u32_pattern214_forbindden_en;
	unsigned char  u1_still_frame=s_pContext->_output_frc_sceneAnalysis.u1_still_frame;
	short  u11_gmv_mvx = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
	short  u10_gmv_mvy = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;
	unsigned int  u25_me_aDTL = s_pContext->_output_read_comreg.u25_me_aDTL_rb;
	unsigned short  u32_gmv_unconf = s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb;
	int  fwgmvtest  , fwgmven ; 
	unsigned int BOT_ratio = 0;
	int diff = 0;

	int gmv_calc_x = 0;
    int gmv_calc_y = 0;
	static unsigned int u32_RgnApl[32] = {0};
	int s11_rmv_mvx[32] = {0};
	int s10_rmv_mvy[32] = {0};
	
	int gmv_calc_rapl_min_idx = 0;
	int gmv_calc_rapl_min_temp = 0;
	static int pre_calc_gmv_mvx = 0;
	static int pre_calc_gmv_rgn_idx = 0;
	int rmv_temp_x = 0;
	static int frameCnt_tmp = 1;

	int reg_gmvd_th = 10;
	int cond0 = 0,cond1 = 0;
	int gmvd_calc_x = 0;
	int gmvd_calc_x_min = 255;
	int gmvd_calc_x_min_idx = 0;
	unsigned int nIdxX = 0;

	static int incnt_for_299 = 0;

	unsigned int swap_s1_flag = 0;
	unsigned int swap_s2_flag = 0;
	//unsigned int print_final_en = 0;
	unsigned int print_detail_en = 0;//for debug.
	int right_rim_pre;
	
#if 0
	unsigned int u25_me_unconf = (s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[9] + s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[10]
					     + s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[11] + s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[12]
					     + s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[13] + s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[14]
					     + s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[17] + s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[18]
					     + s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[19] + s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[20]
					     + s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[21] + s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[22])/12;
#endif

	unsigned int u32_ID_Log_en=0,MEMC_info_Log_en = 0;
	ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 24,  24, &u32_ID_Log_en);//YE Test debug dummy register
	ReadRegister(SOFTWARE1_SOFTWARE1_25_reg, 18, 18, &MEMC_info_Log_en);
	diff = s_pContext->_output_rimctrl.u12_out_resolution[_RIM_BOT] - s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_BOT];
	BOT_ratio = ((_ABS_(diff))<<10)>>11;

	#if Pro_TCL
	//unsigned char  u8_cadence_Id = s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL];
	//PQL_INPUT_FRAME_RATE in_fmRate = s_pContext->_external_data._input_frameRate;
	unsigned char nblend_y_alpha_Target=0xa0;
	unsigned int u32_RB_kmc_blend_y_alpha;
	unsigned int u25_rgnSad_rb12 = s_pContext->_output_read_comreg.u25_me_rSAD_rb[12];
	unsigned int u25_rgnSad_rb23 = s_pContext->_output_read_comreg.u25_me_rSAD_rb[23];

     	//TOP_ratio = ((_ABS_( s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_TOP] - s_pContext->_output_rimctrl.u12_out_resolution[_RIM_TOP]))<<10)>>11;
	BOT_ratio = ((_ABS_(s_pContext->_output_rimctrl.u12_out_resolution[_RIM_BOT] - s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_BOT]))<<10)>>11;

	

	ReadRegister(IPPRE_IPPRE_04_reg, 4, 11, &u32_RB_kmc_blend_y_alpha);
	#endif

	ReadRegister(HARDWARE_HARDWARE_58_reg, 0, 0, &u32_RB_val);   //  bit0
	ReadRegister(HARDWARE_HARDWARE_58_reg, 1, 1, &u32_pattern214_forbindden_en);   //  bit1

	if(MEMC_info_Log_en==1)//B809D764 [18]
	{
		if(MA_print_count % debug_print_delay_time == 0)
		{
			rtd_pr_memc_err("[MEMC_info] prjid = %x,%x\n", vpq_project_id, vpqex_project_id);
			rtd_pr_memc_err("[MEMC_info] gmv_x, gmv_y, u25_me_aDTL, u1_still_frame = %d,%d,%d,%d\n", u11_gmv_mvx, u10_gmv_mvy, u25_me_aDTL, u1_still_frame);
			rtd_pr_memc_err("[MEMC_info] other (%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)\n", 
				s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb, s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb, s_pContext->_output_read_comreg.u27_me_aTC_rb, s_pContext->_output_read_comreg.u27_me_aSC_rb,
				s_pContext->_output_read_comreg.u30_me_aSAD_rb, s_pContext->_output_read_comreg.u25_me_aDTL_rb, s_pContext->_output_read_comreg.u26_me_aAPLi_rb, s_pContext->_output_read_comreg.u26_me_aAPLp_rb,
				s_pContext->_output_read_comreg.u8_me1_Vbuf_Hsize_rb, s_pContext->_output_read_comreg.u8_me1_Vbuf_Vsize_rb, s_pContext->_output_read_comreg.u1_sc_status_rb, s_pContext->_output_read_comreg.u1_sc_status_logo_rb,
				s_pContext->_output_read_comreg.u1_sc_status_dh_rb, s_pContext->_output_read_comreg.u17_me_ZMV_cnt_rb, s_pContext->_output_read_comreg.u17_me_ZMV_dtl_cnt_rb, s_pContext->_output_read_comreg.u31_me_ZMV_psad_dc_rb,
				s_pContext->_output_read_comreg.u31_me_ZMV_psad_ac_rb, s_pContext->_output_read_comreg.u20_me_ZMV_DcGreatAc_cnt_rb, s_pContext->_output_read_comreg.u20_me_ZMV_AcGreatDc_cnt_rb, s_pContext->_output_read_comreg.u6_me_ZGMV_cnt_rb,
				s_pContext->_output_read_comreg.u20_me_Hpan_cnt_rb, s_pContext->_output_read_comreg.u20_me_Vpan_cnt_rb, s_pContext->_output_read_comreg.u16_mc_Hdtl_cnt, s_pContext->_output_read_comreg.u16_mc_Vdtl_cnt,
				s_pContext->_output_read_comreg.u17_me_posCnt_0_rb, s_pContext->_output_read_comreg.u17_me_posCnt_1_rb, s_pContext->_output_read_comreg.u17_me_posCnt_2_rb, s_pContext->_output_read_comreg.u17_me_posCnt_3_rb,
				s_pContext->_output_read_comreg.u17_me_negCnt_0_rb, s_pContext->_output_read_comreg.u17_me_negCnt_1_rb, s_pContext->_output_read_comreg.u17_me_negCnt_2_rb, s_pContext->_output_read_comreg.u17_me_negCnt_3_rb,
				s_pContext->_output_read_comreg.s11_me_GMV_2nd_vx_rb, s_pContext->_output_read_comreg.s10_me_GMV_2nd_vy_rb, s_pContext->_output_read_comreg.u17_me_GMV_2nd_cnt_rb, s_pContext->_output_read_comreg.u12_me_GMV_2nd_unconf_rb,
				s_pContext->_output_read_comreg.u27_ipme_aMot_rb, s_pContext->_output_read_comreg.u3_ipme_filmMode_rb, s_pContext->_output_read_comreg.u1_kphase_inLR, s_pContext->_output_read_comreg.u4_kphase_inPhase,
				s_pContext->_output_read_comreg.u8_sys_N_rb, s_pContext->_output_read_comreg.u8_sys_M_rb, s_pContext->_output_read_comreg.u13_ip_vtrig_dly, s_pContext->_output_read_comreg.u13_me2_org_vtrig_dly,
				s_pContext->_output_read_comreg.u13_me2_vtrig_dly, s_pContext->_output_read_comreg.u13_dec_vtrig_dly, s_pContext->_output_read_comreg.u13_vtotal, s_pContext->_output_read_comreg.u16_inHTotal,
				s_pContext->_output_read_comreg.u16_inVAct, s_pContext->_output_read_comreg.u16_inFrm_Pos, s_pContext->_output_read_comreg.u19_me_statis_glb_prd, s_pContext->_output_read_comreg.u8_gmv_blend_en);
			
			rtd_pr_memc_err("[MEMC_info]rmv_x (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)(%d,%d)\n", 
				s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[0],  s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[1],	s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[2],   s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[3], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[4],  s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[5],	s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[6],  s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[7], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[8], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[9],
				s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[10], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[11], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[12], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[13],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[14], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[15], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[16], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[17],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[18], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[19],
				s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[20], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[21], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[22], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[23],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[24], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[25], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[26], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[27],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[28], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[29],
				s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[30], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[31]);
			
			rtd_pr_memc_err("[MEMC_info]rmv_y (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)(%d,%d)\n", 
				s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[0],  s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[1],	s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[2],  s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[3], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[4],  s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[5],  s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[6],  s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[7], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[8],	s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[9],
				s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[10], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[11], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[12], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[13],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[14], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[15], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[16], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[17],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[18], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[19],
				s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[20], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[21], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[22], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[23],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[24], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[25], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[26], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[27],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[28], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[29],
				s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[30], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[31]);
				
			rtd_pr_memc_err("[MEMC_info]rmv_cnt (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)(%d,%d)\n", 
				s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[0],  s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[1],  s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[2],  s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[3], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[4],  s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[5],  s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[6],	s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[7], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[8],  s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[9],
				s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[10], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[11], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[12], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[13],s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[14], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[15], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[16], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[17],s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[18], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[19],
				s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[20], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[21], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[22], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[23],s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[24], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[25], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[26], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[27],s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[28], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[29],
				s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[30], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[31]);
		
			rtd_pr_memc_err("[MEMC_info]rmv_unconf (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)(%d,%d)\n", 
				s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[0],  s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[1],	s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[2],  s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[3], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[4],  s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[5],  s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[6],  s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[7], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[8],	s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[9],
				s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[10], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[11], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[12], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[13],s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[14], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[15], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[16], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[17],s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[18], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[19],
				s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[20], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[21], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[22], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[23],s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[24], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[25], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[26], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[27],s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[28], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[29],
				s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[30], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[31]);
		
			rtd_pr_memc_err("[MEMC_info]rDTL (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)(%d,%d)\n", 
				s_pContext->_output_read_comreg.u20_me_rDTL_rb[0],	s_pContext->_output_read_comreg.u20_me_rDTL_rb[1],	s_pContext->_output_read_comreg.u20_me_rDTL_rb[2],	s_pContext->_output_read_comreg.u20_me_rDTL_rb[3], s_pContext->_output_read_comreg.u20_me_rDTL_rb[4],  s_pContext->_output_read_comreg.u20_me_rDTL_rb[5],  s_pContext->_output_read_comreg.u20_me_rDTL_rb[6],  s_pContext->_output_read_comreg.u20_me_rDTL_rb[7], s_pContext->_output_read_comreg.u20_me_rDTL_rb[8],  s_pContext->_output_read_comreg.u20_me_rDTL_rb[9],
				s_pContext->_output_read_comreg.u20_me_rDTL_rb[10], s_pContext->_output_read_comreg.u20_me_rDTL_rb[11], s_pContext->_output_read_comreg.u20_me_rDTL_rb[12], s_pContext->_output_read_comreg.u20_me_rDTL_rb[13],s_pContext->_output_read_comreg.u20_me_rDTL_rb[14], s_pContext->_output_read_comreg.u20_me_rDTL_rb[15], s_pContext->_output_read_comreg.u20_me_rDTL_rb[16], s_pContext->_output_read_comreg.u20_me_rDTL_rb[17],s_pContext->_output_read_comreg.u20_me_rDTL_rb[18], s_pContext->_output_read_comreg.u20_me_rDTL_rb[19],
				s_pContext->_output_read_comreg.u20_me_rDTL_rb[20], s_pContext->_output_read_comreg.u20_me_rDTL_rb[21], s_pContext->_output_read_comreg.u20_me_rDTL_rb[22], s_pContext->_output_read_comreg.u20_me_rDTL_rb[23],s_pContext->_output_read_comreg.u20_me_rDTL_rb[24], s_pContext->_output_read_comreg.u20_me_rDTL_rb[25], s_pContext->_output_read_comreg.u20_me_rDTL_rb[26], s_pContext->_output_read_comreg.u20_me_rDTL_rb[27],s_pContext->_output_read_comreg.u20_me_rDTL_rb[28], s_pContext->_output_read_comreg.u20_me_rDTL_rb[29],
				s_pContext->_output_read_comreg.u20_me_rDTL_rb[30], s_pContext->_output_read_comreg.u20_me_rDTL_rb[31]);
			
			rtd_pr_memc_err("[MEMC_info]rAPLi (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)(%d,%d)\n", 
				s_pContext->_output_read_comreg.u20_me_rAPLi_rb[0],  s_pContext->_output_read_comreg.u20_me_rAPLi_rb[1],  s_pContext->_output_read_comreg.u20_me_rAPLi_rb[2],	s_pContext->_output_read_comreg.u20_me_rAPLi_rb[3], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[4],  s_pContext->_output_read_comreg.u20_me_rAPLi_rb[5],  s_pContext->_output_read_comreg.u20_me_rAPLi_rb[6],  s_pContext->_output_read_comreg.u20_me_rAPLi_rb[7], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[8],	s_pContext->_output_read_comreg.u20_me_rAPLi_rb[9],
				s_pContext->_output_read_comreg.u20_me_rAPLi_rb[10], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[11], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[12], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[13],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[14], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[15], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[16], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[17],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[18], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[19],
				s_pContext->_output_read_comreg.u20_me_rAPLi_rb[20], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[21], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[22], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[23],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[24], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[25], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[26], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[27],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[28], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[29],
				s_pContext->_output_read_comreg.u20_me_rAPLi_rb[30], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[31]);
		
			rtd_pr_memc_err("[MEMC_info]rAPLp (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)(%d,%d)\n", 
				s_pContext->_output_read_comreg.u20_me_rAPLp_rb[0], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[1], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[2], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[3], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[4],  s_pContext->_output_read_comreg.u20_me_rAPLp_rb[5],  s_pContext->_output_read_comreg.u20_me_rAPLp_rb[6],  s_pContext->_output_read_comreg.u20_me_rAPLp_rb[7], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[8],	s_pContext->_output_read_comreg.u20_me_rAPLp_rb[9],
				s_pContext->_output_read_comreg.u20_me_rAPLp_rb[10], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[11], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[12], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[13],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[14], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[15], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[16], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[17],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[18], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[19],
				s_pContext->_output_read_comreg.u20_me_rAPLp_rb[20], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[21], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[22], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[23],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[24], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[25], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[26], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[27],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[28], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[29],
				s_pContext->_output_read_comreg.u20_me_rAPLp_rb[30], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[31]);
		}
	}


	if((pParam->u1_Identification_Pattern_en== 0)||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_Test_0_flag ==0))
		return;

	//reset
	pOutput->u1_IP_preProcess_true=0;

	if(s_pContext->_output_me_vst_ctrl.u1_detect_blackBG_VST_trure)
	{
		pOutput->u1_casino_RP_detect_true =0;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
	}

	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,3) == TRUE  )
	{
		pOutput->u1_casino_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[0] = 12;
		nIdentifiedNum_preProcess_a[0]=3;
	}
	else if(u5_blend_holdfrm_preProcess_a[0] >0)
	{
		pOutput->u1_casino_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[0]--;
	}
	else if(nIdentifiedNum_preProcess_a[0] == 3 )
	{
		pOutput->u1_casino_RP_detect_true =0;
		nIdentifiedNum_preProcess_a[0] = 0;
	}
	
	//=============================================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,5) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		if(s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true >= 3){
			pOutput->u1_casino_RP_detect_true =1;
			u5_blend_holdfrm_preProcess_a[1] = 3;
			nIdentifiedNum_preProcess_a[1]=5;

		}
	}
	else if(u5_blend_holdfrm_preProcess_a[1] >0)
	{
		pOutput->u1_casino_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[1]--;
	}
	else if(nIdentifiedNum_preProcess_a[1] == 5 )
	{
		pOutput->u1_casino_RP_detect_true =0;
		nIdentifiedNum_preProcess_a[1] = 0;
	}
	//=============================================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,6) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =3;
		u5_blend_holdfrm_preProcess_a[2] = 16;
		nIdentifiedNum_preProcess_a[2]=6;
	}
	else if(u5_blend_holdfrm_preProcess_a[2] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =3;
		u5_blend_holdfrm_preProcess_a[2]--;
	}
	else if(nIdentifiedNum_preProcess_a[2] == 6 )
	{
		nIdentifiedNum_preProcess_a[2] = 0;
	}
	//=============================================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,7) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
		u5_blend_holdfrm_preProcess_a[3] = 16;
		nIdentifiedNum_preProcess_a[3]=7;
	}
	else if(u5_blend_holdfrm_preProcess_a[3] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
		u5_blend_holdfrm_preProcess_a[3]--;
	}
	else if(nIdentifiedNum_preProcess_a[3] == 7 )
	{
		nIdentifiedNum_preProcess_a[3] = 0;
	}
	//=============================================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,22) == TRUE  )
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[3] = 5;
		nIdentifiedNum_preProcess_a[3]=22;
	}
	else if(u5_blend_holdfrm_preProcess_a[3] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[3]--;
	}
	else if(nIdentifiedNum_preProcess_a[3] == 22 )
	{
		nIdentifiedNum_preProcess_a[3] = 0;
	}
#if 0
	#if Pro_TCL
	//=============================================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,8) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, (u32_RB_kmc_blend_y_alpha+nblend_y_alpha_Target)/2);
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type =1;  // more me1
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true =1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =2;
		rtd_clearbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_clearbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[4] = 4;
		nIdentifiedNum_preProcess_a[4]=8;
	}
	else if(u5_blend_holdfrm_preProcess_a[4] >0)
	{
		WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, (u32_RB_kmc_blend_y_alpha+nblend_y_alpha_Target)/2);
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type =1;  // more me1
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true =1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =2;
		rtd_clearbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_clearbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[4]--;
	}
	else if(nIdentifiedNum_preProcess_a[4] == 8 && (u32_RB_kmc_blend_y_alpha !=s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha))
	{
		if(u32_RB_kmc_blend_y_alpha <(s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha))
		{
			if((s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha) -u32_RB_kmc_blend_y_alpha >1)
				WriteRegister(IPPRE_IPPRE_04_reg, 4, 11,(u32_RB_kmc_blend_y_alpha+1));
			else
				WriteRegister(IPPRE_IPPRE_04_reg, 4, 11,s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha);
		}
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type =0;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true =0;
		rtd_setbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_setbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
	}
	else if(nIdentifiedNum_preProcess_a[4] == 8 )
	{
		nIdentifiedNum_preProcess_a[4] = 0;
	}
	//============================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,9) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		u5_blend_holdfrm_preProcess_a[5] = 16;
		nIdentifiedNum_preProcess_a[5]=9;
	}
	else if(u5_blend_holdfrm_preProcess_a[5] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		u5_blend_holdfrm_preProcess_a[5]--;
	}
	else if(nIdentifiedNum_preProcess_a[5] == 9 )
	{
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=0;
		nIdentifiedNum_preProcess_a[5] = 0;
	}
	//===========================================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,10) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		rtd_clearbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_clearbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;
		u5_blend_holdfrm_preProcess_a[6] = 16;
		nIdentifiedNum_preProcess_a[6]=10;
	}
	else if(u5_blend_holdfrm_preProcess_a[6] >0)
	{
		rtd_clearbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_clearbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;
		u5_blend_holdfrm_preProcess_a[6]--;
	}
	else if(nIdentifiedNum_preProcess_a[6] == 10 )
	{
		rtd_setbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_setbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=0;
		nIdentifiedNum_preProcess_a[6] = 0;
	}
	//===========================================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,11) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[7] = 8;
		nIdentifiedNum_preProcess_a[7]=11;
	}
	else if(u5_blend_holdfrm_preProcess_a[7] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[7]--;
	}
	else if(nIdentifiedNum_preProcess_a[7] == 11 )
	{
		nIdentifiedNum_preProcess_a[7] = 0;
	}
	//===================================================
	//S&M saech on a hammock
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,13) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ) )
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
		s_pContext->_output_frc_sceneAnalysis.u1_Swing_true = 1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		if(Pro_SONY)
		{
	        WriteRegister(KME_ME1_TOP10_LOGO_HALO_reg,0,0, 0);//YE Test 20201006 new
			WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_48_reg,31,31, 0);//YE Test 20201006 new
			WriteRegister(MC_MC_B0_reg,16,18, 0x1);//YE Test 20201006 new
			WriteRegister(MC_MC_B4_reg,16,23, 0);//YE Test 20201006 new
			WriteRegister(MC_MC_B4_reg,24,31, 0);//YE Test 20201006 new
			WriteRegister(MC_MC_B8_reg,11,11, 0);//YE Test 20201006 new
		}
		u5_blend_holdfrm_preProcess_a[8] = 16;
		nIdentifiedNum_preProcess_a[8]=13;
	}
	else if(u5_blend_holdfrm_preProcess_a[8] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
		s_pContext->_output_frc_sceneAnalysis.u1_Swing_true = 1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		if(Pro_SONY)
		{
	        WriteRegister(KME_ME1_TOP10_LOGO_HALO_reg,0,0, 0);//YE Test 20201006 new
			WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_48_reg,31,31, 0);//YE Test 20201006 new
			WriteRegister(MC_MC_B0_reg,16,18, 0x1);//YE Test 20201006 new
			WriteRegister(MC_MC_B4_reg,16,23, 0);//YE Test 20201006 new
			WriteRegister(MC_MC_B4_reg,24,31, 0);//YE Test 20201006 new
			WriteRegister(MC_MC_B8_reg,11,11, 0);//YE Test 20201006 new
		}
		u5_blend_holdfrm_preProcess_a[8]--;
	}
	else if(nIdentifiedNum_preProcess_a[8] == 13 )
	{
		s_pContext->_output_frc_sceneAnalysis.u1_Swing_true = 0;
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		if(Pro_SONY)
		{
	        WriteRegister(KME_ME1_TOP10_LOGO_HALO_reg,0,0, 0);//YE Test 20201006 new
			WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_48_reg,31,31, 0);//YE Test 20201006 new
			WriteRegister(MC_MC_B0_reg,16,18, 0x1);//YE Test 20201006 new
			WriteRegister(MC_MC_B4_reg,16,23, 0);//YE Test 20201006 new
			WriteRegister(MC_MC_B4_reg,24,31, 0);//YE Test 20201006 new
			WriteRegister(MC_MC_B8_reg,11,11, 0);//YE Test 20201006 new
		}
		nIdentifiedNum_preProcess_a[8] = 0;
	}
	//==============================================
	#endif
#endif

#if 0
	//==============================================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,15) == TRUE  )
	{
		//s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[9] = 8;
		nIdentifiedNum_preProcess_a[9]=15;
	}
	else if(u5_blend_holdfrm_preProcess_a[9] >0)
	{
		//s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[9]--;
	}
	else if(nIdentifiedNum_preProcess_a[9] == 15 )
	{
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		nIdentifiedNum_preProcess_a[9] = 0;
	}
	//=====================================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,16) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))    //  can't use motion or dtl info condition
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		WriteRegister(IPPRE_IPPRE_0C_reg, 0, 7, 0x10);
		rtd_clearbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		u5_blend_holdfrm_preProcess_a[10] = 8;
		nIdentifiedNum_preProcess_a[10]=16;
	}
	else if(u5_blend_holdfrm_preProcess_a[10] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		WriteRegister(IPPRE_IPPRE_0C_reg, 0, 7, 0x10);
		rtd_clearbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		u5_blend_holdfrm_preProcess_a[10]--;
	}
	else if(nIdentifiedNum_preProcess_a[10] == 16 )
	{
		WriteRegister(IPPRE_IPPRE_0C_reg, 0, 7, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_logo_y_alpha);
		rtd_setbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		nIdentifiedNum_preProcess_a[10] = 0;
	}
#endif
	//==================================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,17) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))  //  can't use motion or dtl info condition
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
		u5_blend_holdfrm_preProcess_a[11] = 8;
		nIdentifiedNum_preProcess_a[11]=17;
	}
	else if(u5_blend_holdfrm_preProcess_a[11] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
		u5_blend_holdfrm_preProcess_a[11]--;
	}
	else if(nIdentifiedNum_preProcess_a[11] == 17 )
	{
		nIdentifiedNum_preProcess_a[11] = 0;
	}
	//==================================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,20) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[12] = 16;
		nIdentifiedNum_preProcess_a[12]=20;
	}
	else if(u5_blend_holdfrm_preProcess_a[12] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[12]--;
	}
	else if(nIdentifiedNum_preProcess_a[12] == 20 )
	{
		nIdentifiedNum_preProcess_a[12] = 0;
	}
	//=================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,23) == TRUE  )
	{
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true =1;
		s_pContext->_output_me_sceneAnalysis.u2_panning_flag =2;
		u5_blend_holdfrm_preProcess_a[13] = 16;
		nIdentifiedNum_preProcess_a[13]=23;
	}
	else if(u5_blend_holdfrm_preProcess_a[13] >0)
	{
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true=1;
		s_pContext->_output_me_sceneAnalysis.u2_panning_flag =2;
		u5_blend_holdfrm_preProcess_a[13]--;
	}
	else if(nIdentifiedNum_preProcess_a[13] == 23 )
	{
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true =0;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[13] = 0;
	}
	//========================================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,24) == TRUE )
	{
		if((_ABS_(u11_gmv_mvx>>1) <=1   && _ABS_(u10_gmv_mvy>>1) <=1)&& (u25_me_aDTL > 1100000)&&( u1_still_frame == 0))
		{
			s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true =1;
			s_pContext->_output_me_sceneAnalysis.u2_panning_flag =2;
			u5_blend_holdfrm_preProcess_a[14] = 16;
			nIdentifiedNum_preProcess_a[14]=24;
		}

		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;

	}
	else if(u5_blend_holdfrm_preProcess_a[14] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true =1;
		s_pContext->_output_me_sceneAnalysis.u2_panning_flag =2;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[14]--;
	}
	else if(nIdentifiedNum_preProcess_a[14] == 24 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true =0;
		nIdentifiedNum_preProcess_a[14] = 0;
	}
	//===============================================
	#if Pro_TCL
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,25) == TRUE &&( u1_still_frame == 0)&&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, (u32_RB_kmc_blend_y_alpha+0x50)/2);
		s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVx_Step=0;
		s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVy_Step=0;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =3;
		u5_blend_holdfrm_preProcess_a[15] = 8;
		nIdentifiedNum_preProcess_a[15]=25;
	}
	else if(u5_blend_holdfrm_preProcess_a[15] >0)
	{
		WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, (u32_RB_kmc_blend_y_alpha+0x50)/2);
		s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVx_Step=0;
		s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVy_Step=0;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =3;
		u5_blend_holdfrm_preProcess_a[15]--;
	}
	else if(nIdentifiedNum_preProcess_a[15] == 25 )
	{
		WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha);
		nIdentifiedNum_preProcess_a[15] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,26) == TRUE  &&( u1_still_frame == 0)&&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true =1;
		u5_blend_holdfrm_preProcess_a[16] = 8;
		nIdentifiedNum_preProcess_a[16]=26;
	}
	else if(u5_blend_holdfrm_preProcess_a[16] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true =1;
		u5_blend_holdfrm_preProcess_a[16]--;
	}
	else if(nIdentifiedNum_preProcess_a[16] == 26 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true =0;
		nIdentifiedNum_preProcess_a[16] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,27) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[17] = 8;
		nIdentifiedNum_preProcess_a[17]=27;
	}
	else if(u5_blend_holdfrm_preProcess_a[17] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[17]--;
	}
	else if(nIdentifiedNum_preProcess_a[17] == 27 )
	{
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		nIdentifiedNum_preProcess_a[17] = 0;
	}
	#endif

	 //YE Test not me add this  it's 376_CMR_XmenApocalypse  ID=28
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,28) == TRUE && (u25_me_aDTL > 700000)&&( u1_still_frame == 0))
	{
		if(((u32_RB_val) & 0x01) ==1){
			s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true =1;
			s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=1;
		}
		else
			s_pContext->_output_wrt_comreg.u1_GFB_specialScene_true=1;
		u5_blend_holdfrm_preProcess_a[18] = 8;
		nIdentifiedNum_preProcess_a[18]=28;
	}
	else if(u5_blend_holdfrm_preProcess_a[18] >0)
	{
		if(((u32_RB_val) & 0x01) ==1){
			s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true =1;
			s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=1;
		}
		else
			s_pContext->_output_wrt_comreg.u1_GFB_specialScene_true=1;
		u5_blend_holdfrm_preProcess_a[18]--;
	}
	else if(nIdentifiedNum_preProcess_a[18] == 28 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true =0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=0;

		s_pContext->_output_wrt_comreg.u1_GFB_specialScene_true=0;
		nIdentifiedNum_preProcess_a[18] = 0;
	}

	#if Pro_TCL
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,29) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[19] = 8;
		nIdentifiedNum_preProcess_a[19]=29;
	}
	else if(u5_blend_holdfrm_preProcess_a[19] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[19]--;
	}
	else if(nIdentifiedNum_preProcess_a[19] == 29 )
	{
		nIdentifiedNum_preProcess_a[19] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,31) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_GFB_specialScene_true=1;
		u5_blend_holdfrm_preProcess_a[20] = 8;
		nIdentifiedNum_preProcess_a[20]=31;
	}
	else if(u5_blend_holdfrm_preProcess_a[20] >0)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_specialScene_true=1;
		u5_blend_holdfrm_preProcess_a[20]--;
	}
	else if(nIdentifiedNum_preProcess_a[20] == 31 )
	{
		s_pContext->_output_wrt_comreg.u1_GFB_specialScene_true=0;
		nIdentifiedNum_preProcess_a[20] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,32) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u2_panning_flag =1;
		s_pContext->_output_wrt_comreg.u1_MC_SmallObject_type=1;
		u5_blend_holdfrm_preProcess_a[21] = 8;
		nIdentifiedNum_preProcess_a[21]=32;
	}
	else if(u5_blend_holdfrm_preProcess_a[21] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u2_panning_flag =1;
		s_pContext->_output_wrt_comreg.u1_MC_SmallObject_type=1;
		u5_blend_holdfrm_preProcess_a[21]--;
	}
	else if(nIdentifiedNum_preProcess_a[21] == 32 )
	{
		s_pContext->_output_wrt_comreg.u1_MC_SmallObject_type=0;
		nIdentifiedNum_preProcess_a[21] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,33) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u2_panning_flag =1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		s_pContext->_output_wrt_comreg.u1_MC_SmallObject_type=1;
		u5_blend_holdfrm_preProcess_a[22] = 12;
		nIdentifiedNum_preProcess_a[22]=33;
	}
	else if(u5_blend_holdfrm_preProcess_a[22] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u2_panning_flag =1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		s_pContext->_output_wrt_comreg.u1_MC_SmallObject_type=1;
		u5_blend_holdfrm_preProcess_a[22]--;
	}
	else if(nIdentifiedNum_preProcess_a[22] == 33 )
	{
		s_pContext->_output_wrt_comreg.u1_MC_SmallObject_type=0;
		nIdentifiedNum_preProcess_a[22] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,34) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =3;
		u5_blend_holdfrm_preProcess_a[23] = 8;
		nIdentifiedNum_preProcess_a[23]=34;
	}
	else if(u5_blend_holdfrm_preProcess_a[23] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =3;
		u5_blend_holdfrm_preProcess_a[23]--;
	}
	else if(nIdentifiedNum_preProcess_a[23] == 34 )
	{
		nIdentifiedNum_preProcess_a[23] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,35) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[24] = 8;
		nIdentifiedNum_preProcess_a[24]=35;
	}
	else if(u5_blend_holdfrm_preProcess_a[24] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[24]--;
	}
	else if(nIdentifiedNum_preProcess_a[24] == 35 )
	{
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		nIdentifiedNum_preProcess_a[24] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,36) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[25] = 8;
		nIdentifiedNum_preProcess_a[25]=36;
	}
	else if(u5_blend_holdfrm_preProcess_a[25] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[25]--;
	}
	else if(nIdentifiedNum_preProcess_a[25] == 36 )
	{
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		nIdentifiedNum_preProcess_a[25] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,37) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;
		u5_blend_holdfrm_preProcess_a[26] = 8;
		nIdentifiedNum_preProcess_a[26]=37;
	}
	else if(u5_blend_holdfrm_preProcess_a[26] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;
		u5_blend_holdfrm_preProcess_a[26]--;
	}
	else if(nIdentifiedNum_preProcess_a[26] == 37 )
	{
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=0;
		nIdentifiedNum_preProcess_a[26] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,38) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_MC_SmallObject_type=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[27] = 8;
		nIdentifiedNum_preProcess_a[27]=38;
	}
	else if(u5_blend_holdfrm_preProcess_a[27] >0)
	{
		s_pContext->_output_wrt_comreg.u1_MC_SmallObject_type=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[27]--;
	}
	else if(nIdentifiedNum_preProcess_a[27] == 38 )
	{
		s_pContext->_output_wrt_comreg.u1_MC_SmallObject_type=0;
		nIdentifiedNum_preProcess_a[27] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,39) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=3;
		s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[28] = 8;
		nIdentifiedNum_preProcess_a[28]=39;
	}
	else if(u5_blend_holdfrm_preProcess_a[28] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=3;
		s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[28]--;
	}
	else if(nIdentifiedNum_preProcess_a[28] == 39 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=0;
		s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type=0;
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		nIdentifiedNum_preProcess_a[28] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,40) == TRUE &&u25_rgnSad_rb12 ==0&&u25_rgnSad_rb23<= 250 &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=2;  // more me2
		s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;  // Because pq adjustment
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;
		WriteRegister(MC_MC_B0_reg, 8, 10, 0x4);
		WriteRegister(KME_TOP_KME_TOP_04_reg,30, 30, 0x1);
		u5_blend_holdfrm_preProcess_a[29] = 8;
		nIdentifiedNum_preProcess_a[29]=40;
	}
	else if(u5_blend_holdfrm_preProcess_a[29] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=2;  // more me2
		s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;
		WriteRegister(MC_MC_B0_reg, 8, 10, 0x4);
		WriteRegister(KME_TOP_KME_TOP_04_reg,30, 30, 0x1);
		u5_blend_holdfrm_preProcess_a[29]--;
	}
	else if(nIdentifiedNum_preProcess_a[29] == 40 )
	{
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=0;
		s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=0;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=0;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=0;
		WriteRegister(MC_MC_B0_reg, 8, 10, 0x3);
		WriteRegister(KME_TOP_KME_TOP_04_reg,30, 30, 0x0);
		nIdentifiedNum_preProcess_a[29] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,41) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[30] = 16;
		nIdentifiedNum_preProcess_a[30]=41;
	}
	else if(u5_blend_holdfrm_preProcess_a[30] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[30]--;
	}
	else if(nIdentifiedNum_preProcess_a[30] == 41 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		nIdentifiedNum_preProcess_a[30] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,42) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[31] = 16;
		nIdentifiedNum_preProcess_a[31]=42;
	}
	else if(u5_blend_holdfrm_preProcess_a[31] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[31]--;
	}
	else if(nIdentifiedNum_preProcess_a[31] == 42 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		nIdentifiedNum_preProcess_a[31] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,43) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_clearbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[32] = 8;
		nIdentifiedNum_preProcess_a[32]=43;
	}
	else if(u5_blend_holdfrm_preProcess_a[32] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_clearbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[32]--;
	}
	else if(nIdentifiedNum_preProcess_a[32] == 43 )
	{
		rtd_setbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_setbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		nIdentifiedNum_preProcess_a[32] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,44) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
		u5_blend_holdfrm_preProcess_a[33] = 8;
		nIdentifiedNum_preProcess_a[33]=44;
	}
	else if(u5_blend_holdfrm_preProcess_a[33] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
		u5_blend_holdfrm_preProcess_a[33]--;
	}
	else if(nIdentifiedNum_preProcess_a[33] == 44 )
	{
		nIdentifiedNum_preProcess_a[33] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,45) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[34] = 8;
		nIdentifiedNum_preProcess_a[34]=45;
	}
	else if(u5_blend_holdfrm_preProcess_a[34] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[34]--;
	}
	else if(nIdentifiedNum_preProcess_a[34] == 45 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[34] = 0;
	}
	#endif
	//==================================
	//======================
	#if Pro_SONY
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,46) == TRUE  && s_pContext->_output_me_sceneAnalysis.u2_panning_flag)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_dh_close.u1_closeDh_act =0;
		u5_blend_holdfrm_preProcess_a[35] = 16;
		nIdentifiedNum_preProcess_a[35]=46;
	}
	else if(u5_blend_holdfrm_preProcess_a[35] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_dh_close.u1_closeDh_act =0;
		u5_blend_holdfrm_preProcess_a[35]--;
	}
	else if(nIdentifiedNum_preProcess_a[35] == 46 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[35] = 0;
	}

	//++ YE Test  sony for_Periodical_Pattern_[Hannibal]
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,47) == TRUE )
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_adptpnt_rnd_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=3;
		u5_blend_holdfrm_preProcess_a[36] = 16;
		nIdentifiedNum_preProcess_a[36]=47;
	}
	else if(u5_blend_holdfrm_preProcess_a[36] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_adptpnt_rnd_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=3;
		u5_blend_holdfrm_preProcess_a[36]--;
	}
	else if(nIdentifiedNum_preProcess_a[36] == 47 )
	{
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true =0;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_adptpnt_rnd_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=0;
		nIdentifiedNum_preProcess_a[36] = 0;
	}

	//++ YE Test  sony Periodical_Pattern_[glee]
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,48) == TRUE && (BOT_ratio==0))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=2;
		s_pContext->_output_wrt_comreg.u1_ME1_adptpnt_rnd_gain_true=1;
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=2;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=1;

		u5_blend_holdfrm_preProcess_a[37] = 24;
		nIdentifiedNum_preProcess_a[37]=48;
	}
	else if(u5_blend_holdfrm_preProcess_a[37] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=2;
		s_pContext->_output_wrt_comreg.u1_ME1_adptpnt_rnd_gain_true=1;
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=2;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=1;

		u5_blend_holdfrm_preProcess_a[37]--;
	}
	else if(nIdentifiedNum_preProcess_a[37] == 48 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=0;
		s_pContext->_output_wrt_comreg.u1_ME1_adptpnt_rnd_gain_true=0;
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=0;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=0;
		nIdentifiedNum_preProcess_a[37] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,49) == TRUE  )
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=2;
		s_pContext->_output_wrt_comreg.u1_ME1_adptpnt_rnd_gain_true=1;
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=2;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=1;

		u5_blend_holdfrm_preProcess_a[38] = 24;
		nIdentifiedNum_preProcess_a[38]=49;
	}
	else if(u5_blend_holdfrm_preProcess_a[38] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=2;
		s_pContext->_output_wrt_comreg.u1_ME1_adptpnt_rnd_gain_true=1;
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=2;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=1;

		u5_blend_holdfrm_preProcess_a[38]--;
	}
	else if(nIdentifiedNum_preProcess_a[38] == 49 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=0;
		s_pContext->_output_wrt_comreg.u1_ME1_adptpnt_rnd_gain_true=0;
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=0;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=0;
		nIdentifiedNum_preProcess_a[38] = 0;
	}

	//++YE Test Sony 3:2 Film&ScrollSmallText 60i 20200904 fine tune
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,50)== TRUE &&(u10_gmv_mvy==0))
	{
		s_pContext->_output_wrt_comreg.u1_FILM_force_mix_true=1;
		u5_blend_holdfrm_preProcess_a[39] = 60;
		nIdentifiedNum_preProcess_a[39]=50;
	}
	else if(u5_blend_holdfrm_preProcess_a[39] >0)
	{
		s_pContext->_output_wrt_comreg.u1_FILM_force_mix_true=1;
		u5_blend_holdfrm_preProcess_a[39]--;
	}
	else if(nIdentifiedNum_preProcess_a[39] == 50 )
	{
		s_pContext->_output_wrt_comreg.u1_FILM_force_mix_true=0;
		nIdentifiedNum_preProcess_a[39] = 0;
	}
	#endif
	#if Pro_TCL
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,51) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[40] = 16;
		nIdentifiedNum_preProcess_a[40]=51;
	}
	else if(u5_blend_holdfrm_preProcess_a[40] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[40]--;
	}
	else if(nIdentifiedNum_preProcess_a[40] == 51 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=0;
		nIdentifiedNum_preProcess_a[40] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,52) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=3;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Fast_Convergence_type =1;
		s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVx_Step=0;
		s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVy_Step=0;
		u5_blend_holdfrm_preProcess_a[41] = 24;
		nIdentifiedNum_preProcess_a[41]=52;
	}
	else if(u5_blend_holdfrm_preProcess_a[41] >0)
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=3;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Fast_Convergence_type =1;
		s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVx_Step=0;
		s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVy_Step=0;
		u5_blend_holdfrm_preProcess_a[41]--;
	}
	else if(nIdentifiedNum_preProcess_a[41] == 52 )
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Fast_Convergence_type =0;
		nIdentifiedNum_preProcess_a[41] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,53) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Fast_Convergence_type =2;
		s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_KME_SR_increaseY_true=1;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Fast_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[42] = 24;
		nIdentifiedNum_preProcess_a[42]=53;
	}
	else if(u5_blend_holdfrm_preProcess_a[42] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Fast_Convergence_type =2;
		s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_KME_SR_increaseY_true=1;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Fast_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[42]--;
	}
	else if(nIdentifiedNum_preProcess_a[42] == 53 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Fast_Convergence_type =0;
		s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=0;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=0;
		s_pContext->_output_wrt_comreg.u1_KME_SR_increaseY_true=0;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type =0;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Fast_Convergence_true=0;
		nIdentifiedNum_preProcess_a[42] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,54) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[43] = 8;
		nIdentifiedNum_preProcess_a[43]=54;
	}
	else if(u5_blend_holdfrm_preProcess_a[43] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[43]--;
	}
	else if(nIdentifiedNum_preProcess_a[43] == 54 )
	{
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		nIdentifiedNum_preProcess_a[43] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,55) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_dh_close.u1_dh_sml_rgnAction_close=1;
		s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
		u5_blend_holdfrm_preProcess_a[44] = 8;
		nIdentifiedNum_preProcess_a[44]=55;
	}
	else if(u5_blend_holdfrm_preProcess_a[44] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_dh_close.u1_dh_sml_rgnAction_close=1;
		s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
		u5_blend_holdfrm_preProcess_a[44]--;
	}
	else if(nIdentifiedNum_preProcess_a[44] == 55 )
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=0;
		nIdentifiedNum_preProcess_a[44] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,56) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=3;
		s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
		u5_blend_holdfrm_preProcess_a[45] = 8;
		nIdentifiedNum_preProcess_a[45]=56;
	}
	else if(u5_blend_holdfrm_preProcess_a[45] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=3;
		s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
		u5_blend_holdfrm_preProcess_a[45]--;
	}
	else if(nIdentifiedNum_preProcess_a[45] == 56 )
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=0;
		s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=0;
		nIdentifiedNum_preProcess_a[45] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,57) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_dh_close.u1_closeDh_act =0;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=3;
		u5_blend_holdfrm_preProcess_a[46] = 8;
		nIdentifiedNum_preProcess_a[46]=57;
	}
	else if(u5_blend_holdfrm_preProcess_a[46] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =0;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=3;
		u5_blend_holdfrm_preProcess_a[46]--;
	}
	else if(nIdentifiedNum_preProcess_a[46] == 57 )
	{
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=0;
		nIdentifiedNum_preProcess_a[46] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,58) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=1;
		u5_blend_holdfrm_preProcess_a[47] = 8;
		nIdentifiedNum_preProcess_a[47]=58;
	}
	else if(u5_blend_holdfrm_preProcess_a[47] >0)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=1;
		u5_blend_holdfrm_preProcess_a[47]--;
	}
	else if(nIdentifiedNum_preProcess_a[47] == 58 )
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=0;
		nIdentifiedNum_preProcess_a[47] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,59) == TRUE  &&(u25_me_aDTL >=3800000)&&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_dh_close.u1_closeDh_act =0;
		pOutput->u1_casino_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[48] = 8;
		nIdentifiedNum_preProcess_a[48]=59;
	}
	else if(u5_blend_holdfrm_preProcess_a[48] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_dh_close.u1_closeDh_act =0;
		pOutput->u1_casino_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[48]--;
	}
	else if(nIdentifiedNum_preProcess_a[48] == 59 )
	{
		pOutput->u1_casino_RP_detect_true =0;
		nIdentifiedNum_preProcess_a[48] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,4) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		rtd_clearbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_clearbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;

		u5_blend_holdfrm_preProcess_a[49] = 20;
		nIdentifiedNum_preProcess_a[49]=4;
	}
	else if(u5_blend_holdfrm_preProcess_a[49] >0)
	{
		rtd_clearbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_clearbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;

		u5_blend_holdfrm_preProcess_a[49]--;
	}
	else if(nIdentifiedNum_preProcess_a[49] == 4 )
	{
		rtd_setbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_setbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply

		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=0;

		nIdentifiedNum_preProcess_a[49] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,60) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		rtd_clearbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_clearbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;

		u5_blend_holdfrm_preProcess_a[50] = 20;
		nIdentifiedNum_preProcess_a[50]=60;
	}
	else if(u5_blend_holdfrm_preProcess_a[50] >0)
	{
		rtd_clearbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_clearbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;

		u5_blend_holdfrm_preProcess_a[50]--;
	}
	else if(nIdentifiedNum_preProcess_a[50] == 60 )
	{
		rtd_setbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_setbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=0;

		nIdentifiedNum_preProcess_a[50] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,61) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		rtd_clearbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_clearbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;

		u5_blend_holdfrm_preProcess_a[51] = 20;
		nIdentifiedNum_preProcess_a[51]=61;
	}
	else if(u5_blend_holdfrm_preProcess_a[51] >0)
	{
		rtd_clearbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_clearbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;

		u5_blend_holdfrm_preProcess_a[51]--;
	}
	else if(nIdentifiedNum_preProcess_a[51] == 61 )
	{
		rtd_setbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_setbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=0;
		nIdentifiedNum_preProcess_a[51] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,62) == TRUE &&(u11_gmv_mvx >=-58 && u11_gmv_mvx <=-36)&&(u10_gmv_mvy >=-72 && u10_gmv_mvy <=-33)  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_dh_close.u1_dh_sml_rgnAction_close=1;
		if(u32_RB_kmc_blend_y_alpha >0x70)
			WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, (u32_RB_kmc_blend_y_alpha-3));
		WriteRegister(IPPRE_IPPRE_04_reg, 16, 23, 0xa0);
		WriteRegister(IPPRE_IPPRE_04_reg, 24, 25, 0x1);
		u5_blend_holdfrm_preProcess_a[52] = 8;
		nIdentifiedNum_preProcess_a[52]=62;
	}
	else if(u5_blend_holdfrm_preProcess_a[52] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_dh_close.u1_dh_sml_rgnAction_close=1;
		if(u32_RB_kmc_blend_y_alpha >0x70)
			WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, (u32_RB_kmc_blend_y_alpha-3));
		WriteRegister(IPPRE_IPPRE_04_reg, 16, 23, 0xa0);
		WriteRegister(IPPRE_IPPRE_04_reg, 24, 25, 0x1);
		u5_blend_holdfrm_preProcess_a[52]--;
	}
	else if(nIdentifiedNum_preProcess_a[52] == 62 && (u32_RB_kmc_blend_y_alpha !=s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha) )
	{
		if(u32_RB_kmc_blend_y_alpha <(s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha))
		{
			if((s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha) -u32_RB_kmc_blend_y_alpha >1)
				WriteRegister(IPPRE_IPPRE_04_reg, 4, 11,(u32_RB_kmc_blend_y_alpha+1));
			else
				WriteRegister(IPPRE_IPPRE_04_reg, 4, 11,s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha);
		}
		s_pContext->_output_dh_close.u1_dh_sml_rgnAction_close=0;
		WriteRegister(IPPRE_IPPRE_04_reg, 16, 23, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_uv_alpha);
		WriteRegister(IPPRE_IPPRE_04_reg, 24, 25, 0x2);
	}
	else if(nIdentifiedNum_preProcess_a[52] == 62)
	{
		nIdentifiedNum_preProcess_a[52] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,63) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		u5_blend_holdfrm_preProcess_a[53] = 8;
		nIdentifiedNum_preProcess_a[53]=63;
	}
	else if(u5_blend_holdfrm_preProcess_a[53] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		u5_blend_holdfrm_preProcess_a[53]--;
	}
	else if(nIdentifiedNum_preProcess_a[53] == 63 )
	{
		nIdentifiedNum_preProcess_a[53] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,64) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_dh_close.u1_dh_sml_rgnAction_close=1;
		u5_blend_holdfrm_preProcess_a[54] = 16;
		nIdentifiedNum_preProcess_a[54]=64;
	}
	else if(u5_blend_holdfrm_preProcess_a[54] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_dh_close.u1_dh_sml_rgnAction_close=1;
		u5_blend_holdfrm_preProcess_a[54]--;
	}
	else if(nIdentifiedNum_preProcess_a[54] == 64 )
	{
		nIdentifiedNum_preProcess_a[54] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,65) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;
		u5_blend_holdfrm_preProcess_a[55] = 16;
		nIdentifiedNum_preProcess_a[55]=65;
	}
	else if(u5_blend_holdfrm_preProcess_a[55] >0)
	{
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;
		u5_blend_holdfrm_preProcess_a[55]--;
	}
	else if(nIdentifiedNum_preProcess_a[55] == 65 )
	{
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=0;
		nIdentifiedNum_preProcess_a[55] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,66) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		u5_blend_holdfrm_preProcess_a[56] = 8;
		nIdentifiedNum_preProcess_a[56]=66;
	}
	else if(u5_blend_holdfrm_preProcess_a[56] >0)
	{
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		u5_blend_holdfrm_preProcess_a[56]--;
	}
	else if(nIdentifiedNum_preProcess_a[56] == 66 )
	{
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=0;
		nIdentifiedNum_preProcess_a[56] = 0;
	}
	#endif
	#if Pro_SONY
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,67) == TRUE && scalerVIP_Get_MEMCPatternFlag_Identification(0,49) == FALSE)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_KME_gmvd_cost_type=1;
		u5_blend_holdfrm_preProcess_a[57] = 24;
		nIdentifiedNum_preProcess_a[57]=67;
	}
	else if(u5_blend_holdfrm_preProcess_a[57] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_KME_gmvd_cost_type=1;
		u5_blend_holdfrm_preProcess_a[57]--;
	}
	//else if(nIdentifiedNum_preProcess_a[57] == 67&&(scalerVIP_Get_MEMCPatternFlag_Identification(0,48) == TRUE || scalerVIP_Get_MEMCPatternFlag_Identification(0,49) == TRUE ) )
	//{
	//	s_pContext->_output_wrt_comreg.u1_KME_gmvd_cost_type=0;
	//	nIdentifiedNum_preProcess_a[57] = 0;
	//}
	else if(nIdentifiedNum_preProcess_a[57] == 67 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_KME_gmvd_cost_type=0;
		nIdentifiedNum_preProcess_a[57] = 0;
	}
	#endif
	#if Pro_TCL
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,68) == TRUE && s_pContext->_output_me_sceneAnalysis.u2_panning_flag)
	{
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=2;
		WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, 0xb0);
		u5_blend_holdfrm_preProcess_a[58] = 8;
		nIdentifiedNum_preProcess_a[58]=68;
	}
	else if(u5_blend_holdfrm_preProcess_a[58] >0)
	{
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=2;
		WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, 0xb0);
		u5_blend_holdfrm_preProcess_a[58]--;
	}
	else if(nIdentifiedNum_preProcess_a[58] == 68 && (u32_RB_kmc_blend_y_alpha !=s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha) )
	{
		if(u32_RB_kmc_blend_y_alpha <(s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha))
		{
			if((s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha) -u32_RB_kmc_blend_y_alpha >1)
				WriteRegister(IPPRE_IPPRE_04_reg, 4, 11,(u32_RB_kmc_blend_y_alpha+1));
			else
				WriteRegister(IPPRE_IPPRE_04_reg, 4, 11,s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha);
		}
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=0;
	}
	else if(nIdentifiedNum_preProcess_a[58] == 68 )
	{
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=0;
		nIdentifiedNum_preProcess_a[58] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,69) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=1;
		u5_blend_holdfrm_preProcess_a[59] = 8;
		nIdentifiedNum_preProcess_a[59]=69;
	}
	else if(u5_blend_holdfrm_preProcess_a[59] >0)
	{
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=1;
		u5_blend_holdfrm_preProcess_a[59]--;
	}
	else if(nIdentifiedNum_preProcess_a[59] == 69 )
	{
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=0;
		nIdentifiedNum_preProcess_a[59] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,70) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[60] = 8;
		nIdentifiedNum_preProcess_a[60]=70;
	}
	else if(u5_blend_holdfrm_preProcess_a[60] >0)
	{
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[60]--;
	}
	else if(nIdentifiedNum_preProcess_a[60] == 70 )
	{
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		nIdentifiedNum_preProcess_a[60] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,71) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
		u5_blend_holdfrm_preProcess_a[61] = 8;
		nIdentifiedNum_preProcess_a[61]=71;
	}
	else if(u5_blend_holdfrm_preProcess_a[61] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
		u5_blend_holdfrm_preProcess_a[61]--;
	}
	else if(nIdentifiedNum_preProcess_a[61] == 71 )
	{
		nIdentifiedNum_preProcess_a[61] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,72) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
		u5_blend_holdfrm_preProcess_a[62] = 8;
		nIdentifiedNum_preProcess_a[62]=72;
	}
	else if(u5_blend_holdfrm_preProcess_a[62] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
		u5_blend_holdfrm_preProcess_a[62]--;
	}
	else if(nIdentifiedNum_preProcess_a[62] == 72 )
	{
		nIdentifiedNum_preProcess_a[62] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,73) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[63] = 8;
		nIdentifiedNum_preProcess_a[63]=73;
	}
	else if(u5_blend_holdfrm_preProcess_a[63] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[63]--;
	}
	else if(nIdentifiedNum_preProcess_a[63] == 73 )
	{
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		nIdentifiedNum_preProcess_a[63] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,74) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[64] = 8;
		nIdentifiedNum_preProcess_a[64]=74;
	}
	else if(u5_blend_holdfrm_preProcess_a[64] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[64]--;
	}
	else if(nIdentifiedNum_preProcess_a[64] == 74 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[64] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,75) == TRUE &&  (u11_gmv_mvx >=40 && u11_gmv_mvx <=50)&&(u10_gmv_mvy >=-61 && u10_gmv_mvy <=-31)&&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		if(u32_RB_kmc_blend_y_alpha >0x70)
			WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, (u32_RB_kmc_blend_y_alpha-3));
		WriteRegister(IPPRE_IPPRE_04_reg, 16, 23, 0x80);
		WriteRegister(IPPRE_IPPRE_04_reg, 24, 25, 0x1);

		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[65] = 8;
		nIdentifiedNum_preProcess_a[65]=75;
	}
	else if(u5_blend_holdfrm_preProcess_a[65] >0)
	{
		if(u32_RB_kmc_blend_y_alpha >0x70)
			WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, (u32_RB_kmc_blend_y_alpha-3));
		WriteRegister(IPPRE_IPPRE_04_reg, 16, 23, 0x80);
		WriteRegister(IPPRE_IPPRE_04_reg, 24, 25, 0x1);

		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[65]--;
	}
	else if(nIdentifiedNum_preProcess_a[65] == 75 && (u32_RB_kmc_blend_y_alpha !=s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha) )
	{
		if(u32_RB_kmc_blend_y_alpha <(s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha))
		{
			if((s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha) -u32_RB_kmc_blend_y_alpha >1)
				WriteRegister(IPPRE_IPPRE_04_reg, 4, 11,(u32_RB_kmc_blend_y_alpha+1));
			else
				WriteRegister(IPPRE_IPPRE_04_reg, 4, 11,s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha);
		}
		WriteRegister(IPPRE_IPPRE_04_reg, 16, 23, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_uv_alpha);
		WriteRegister(IPPRE_IPPRE_04_reg, 24, 25, 0x2);
	}
	else if(nIdentifiedNum_preProcess_a[65] == 75 )
	{
		nIdentifiedNum_preProcess_a[65] = 0;
	}

	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,76) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_dh_close.u1_dh_sml_rgnAction_close=1;
		u5_blend_holdfrm_preProcess_a[66] = 16;
		nIdentifiedNum_preProcess_a[66]=76;
	}
	else if(u5_blend_holdfrm_preProcess_a[66] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_dh_close.u1_dh_sml_rgnAction_close=1;
		u5_blend_holdfrm_preProcess_a[66]--;
	}
	else if(nIdentifiedNum_preProcess_a[66] == 76 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[66] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,77) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		u5_blend_holdfrm_preProcess_a[67] = 8;
		nIdentifiedNum_preProcess_a[67]=77;
	}
	else if(u5_blend_holdfrm_preProcess_a[67] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		u5_blend_holdfrm_preProcess_a[67]--;
	}
	else if(nIdentifiedNum_preProcess_a[67] == 77 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[67] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,78) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=2;
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=1;
		s_pContext->_output_wrt_comreg.u1_ME1_DRP_type = 1;
		u5_blend_holdfrm_preProcess_a[68] = 8;
		nIdentifiedNum_preProcess_a[68]=78;
	}
	else if(u5_blend_holdfrm_preProcess_a[68] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=2;
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=1;
		s_pContext->_output_wrt_comreg.u1_ME1_DRP_type = 1;
		u5_blend_holdfrm_preProcess_a[68]--;
	}
	else if(nIdentifiedNum_preProcess_a[68] == 78 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=0;
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=0;
		s_pContext->_output_wrt_comreg.u1_ME1_DRP_type = 0;
		nIdentifiedNum_preProcess_a[68] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,79) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=2;
		u5_blend_holdfrm_preProcess_a[69] = 8;
		nIdentifiedNum_preProcess_a[69]=79;
	}
	else if(u5_blend_holdfrm_preProcess_a[69] >0)
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=2;
		u5_blend_holdfrm_preProcess_a[69]--;
	}
	else if(nIdentifiedNum_preProcess_a[69] == 79 )
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=0;
		nIdentifiedNum_preProcess_a[69] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,80) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=2;
		u5_blend_holdfrm_preProcess_a[70] = 8;
		nIdentifiedNum_preProcess_a[70]=80;
	}
	else if(u5_blend_holdfrm_preProcess_a[70] >0)
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=2;
		u5_blend_holdfrm_preProcess_a[70]--;
	}
	else if(nIdentifiedNum_preProcess_a[70] == 80 )
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=0;
		nIdentifiedNum_preProcess_a[70] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,81) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=2;
		if(u32_RB_kmc_blend_y_alpha >0xb0)
			WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, (u32_RB_kmc_blend_y_alpha-3));
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[71] = 8;
		nIdentifiedNum_preProcess_a[71]=81;
	}
	else if(u5_blend_holdfrm_preProcess_a[71] >0)
	{
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=2;
		if(u32_RB_kmc_blend_y_alpha >0xb0)
			WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, (u32_RB_kmc_blend_y_alpha-3));
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[71]--;
	}
	else if(nIdentifiedNum_preProcess_a[71] == 81 && (u32_RB_kmc_blend_y_alpha !=s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha) )
	{
		if(u32_RB_kmc_blend_y_alpha <(s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha))
		{
			if((s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha) -u32_RB_kmc_blend_y_alpha >1)
				WriteRegister(IPPRE_IPPRE_04_reg, 4, 11,(u32_RB_kmc_blend_y_alpha+1));
			else
				WriteRegister(IPPRE_IPPRE_04_reg, 4, 11,s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha);
		}
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=0;
	}
	else if(nIdentifiedNum_preProcess_a[71] == 81 )
	{
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=0;
		nIdentifiedNum_preProcess_a[71] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,82) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[72] = 8;
		nIdentifiedNum_preProcess_a[72]=82;
	}
	else if(u5_blend_holdfrm_preProcess_a[72] >0)
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[72]--;
	}
	else if(nIdentifiedNum_preProcess_a[72] == 82 )
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[72] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,83) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type=1;
		s_pContext->_output_wrt_comreg.u1_GFB_modify_true=0;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=1;
		u5_blend_holdfrm_preProcess_a[73] = 8;
		nIdentifiedNum_preProcess_a[73]=83;
	}
	else if(u5_blend_holdfrm_preProcess_a[73] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type=1;
		s_pContext->_output_wrt_comreg.u1_GFB_modify_true=0;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=1;
		u5_blend_holdfrm_preProcess_a[73]--;
	}
	else if(nIdentifiedNum_preProcess_a[73] == 83 && pOutput->u8_FB_lvl>0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type=0;
		s_pContext->_output_wrt_comreg.u1_GFB_modify_true=1;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=0;
		pOutput->u8_FB_lvl --;
	}
	else if(nIdentifiedNum_preProcess_a[73] == 83 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type=0;
		s_pContext->_output_wrt_comreg.u1_GFB_modify_true=0;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=0;
		nIdentifiedNum_preProcess_a[73] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,84) == TRUE  && (u11_gmv_mvx >=190 && u11_gmv_mvx <=310)&&(u10_gmv_mvy >=-7 && u10_gmv_mvy <=7)&&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		if(pOutput->u8_FB_lvl<0x60)
		{
			pOutput->u8_FB_lvl ++;
			s_pContext->_output_wrt_comreg.u1_GFB_modify_true=1;
		}
		else if( s_pContext->_output_fblevelctrl.u8_FBLevel >0x60)
		{
			s_pContext->_output_wrt_comreg.u1_GFB_modify_true=0;
		}
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=1;
		u5_blend_holdfrm_preProcess_a[74] = 16;
		nIdentifiedNum_preProcess_a[74]=84;
	}
	else if(u5_blend_holdfrm_preProcess_a[74] >0)
	{
		if(pOutput->u8_FB_lvl<0x60)
		{
			pOutput->u8_FB_lvl ++;
			s_pContext->_output_wrt_comreg.u1_GFB_modify_true=1;
		}
		else if( s_pContext->_output_fblevelctrl.u8_FBLevel >0x60)
		{
			s_pContext->_output_wrt_comreg.u1_GFB_modify_true=0;
		}
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=1;
		u5_blend_holdfrm_preProcess_a[74]--;
	}
	else if(nIdentifiedNum_preProcess_a[74] == 84 && pOutput->u8_FB_lvl>0)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_modify_true=1;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=1;
		pOutput->u8_FB_lvl --;
	}
	else if(nIdentifiedNum_preProcess_a[74] == 84 )
	{
		s_pContext->_output_wrt_comreg.u1_GFB_modify_true=0;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=0;
		nIdentifiedNum_preProcess_a[74] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,85) == TRUE   && (u11_gmv_mvx >=190 && u11_gmv_mvx <=310)&&(u10_gmv_mvy >=-7 && u10_gmv_mvy <=7)&&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		if(pOutput->u8_FB_lvl<0x60)
		{
			pOutput->u8_FB_lvl ++;
			s_pContext->_output_wrt_comreg.u1_GFB_modify_true=1;
		}
		else if( s_pContext->_output_fblevelctrl.u8_FBLevel >0x60)
		{
			s_pContext->_output_wrt_comreg.u1_GFB_modify_true=0;
		}
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=1;
		u5_blend_holdfrm_preProcess_a[75] = 16;
		nIdentifiedNum_preProcess_a[75]=85;
	}
	else if(u5_blend_holdfrm_preProcess_a[75] >0)
	{
		if(pOutput->u8_FB_lvl<0x60)
		{
			pOutput->u8_FB_lvl ++;
			s_pContext->_output_wrt_comreg.u1_GFB_modify_true=1;
		}
		else if( s_pContext->_output_fblevelctrl.u8_FBLevel >0x60)
		{
			s_pContext->_output_wrt_comreg.u1_GFB_modify_true=0;
		}
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=1;
		u5_blend_holdfrm_preProcess_a[75]--;
	}
	else if(nIdentifiedNum_preProcess_a[75] == 85 && pOutput->u8_FB_lvl>0)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_modify_true=1;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=1;
		pOutput->u8_FB_lvl --;
	}
	else if(nIdentifiedNum_preProcess_a[75] == 85 )
	{
		s_pContext->_output_wrt_comreg.u1_GFB_modify_true=0;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=0;
		nIdentifiedNum_preProcess_a[75] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,86) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		u5_blend_holdfrm_preProcess_a[76] = 8;
		nIdentifiedNum_preProcess_a[76]=86;
	}
	else if(u5_blend_holdfrm_preProcess_a[76] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		u5_blend_holdfrm_preProcess_a[76]--;
	}
	else if(nIdentifiedNum_preProcess_a[76] == 86 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[76] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,87) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[77] = 8;
		nIdentifiedNum_preProcess_a[77]=87;
	}
	else if(u5_blend_holdfrm_preProcess_a[77] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[77]--;
	}
	else if(nIdentifiedNum_preProcess_a[77] == 87 )
	{
		nIdentifiedNum_preProcess_a[77] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,88) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=4;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[78] = 16;
		nIdentifiedNum_preProcess_a[78]=88;
	}
	else if(u5_blend_holdfrm_preProcess_a[78] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=4;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[78]--;
	}
	else if(nIdentifiedNum_preProcess_a[78] == 88 )
	{
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[78] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,89) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		u5_blend_holdfrm_preProcess_a[79] = 8;
		nIdentifiedNum_preProcess_a[79]=89;
	}
	else if(u5_blend_holdfrm_preProcess_a[79] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		u5_blend_holdfrm_preProcess_a[79]--;
	}
	else if(nIdentifiedNum_preProcess_a[79] == 89 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[79] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,90) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		u5_blend_holdfrm_preProcess_a[80] = 8;
		nIdentifiedNum_preProcess_a[80]=90;
	}
	else if(u5_blend_holdfrm_preProcess_a[80] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		u5_blend_holdfrm_preProcess_a[80]--;
	}
	else if(nIdentifiedNum_preProcess_a[80] == 90 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[80] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,91) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_DRP_type = 1;
		u5_blend_holdfrm_preProcess_a[81] = 8;
		nIdentifiedNum_preProcess_a[81]=91;
	}
	else if(u5_blend_holdfrm_preProcess_a[81] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_DRP_type = 1;
		u5_blend_holdfrm_preProcess_a[81]--;
	}
	else if(nIdentifiedNum_preProcess_a[81] == 91 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_DRP_type = 0;
		nIdentifiedNum_preProcess_a[81] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,92) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;
		u5_blend_holdfrm_preProcess_a[82] = 8;
		nIdentifiedNum_preProcess_a[82]=92;
	}
	else if(u5_blend_holdfrm_preProcess_a[82] >0)
	{
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;
		u5_blend_holdfrm_preProcess_a[82]--;
	}
	else if(nIdentifiedNum_preProcess_a[82] == 92 )
	{
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=0;
		nIdentifiedNum_preProcess_a[82] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,93) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		u5_blend_holdfrm_preProcess_a[83] = 8;
		nIdentifiedNum_preProcess_a[83]=93;
	}
	else if(u5_blend_holdfrm_preProcess_a[83] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		u5_blend_holdfrm_preProcess_a[83]--;
	}
	else if(nIdentifiedNum_preProcess_a[83] == 93)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[83] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,94) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type =3;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[84] = 16;
		nIdentifiedNum_preProcess_a[84]=94;
	}
	else if(u5_blend_holdfrm_preProcess_a[84] >0)
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type =3;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[84]--;
	}
	else if(nIdentifiedNum_preProcess_a[84] == 94)
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type =0;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[84] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,95) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[85] = 16;
		nIdentifiedNum_preProcess_a[85]=95;
	}
	else if(u5_blend_holdfrm_preProcess_a[85] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[85]--;
	}
	else if(nIdentifiedNum_preProcess_a[85] == 95)
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type =0;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[85] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,96) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[86] = 16;
		nIdentifiedNum_preProcess_a[86]=96;
	}
	else if(u5_blend_holdfrm_preProcess_a[86] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[86]--;
	}
	else if(nIdentifiedNum_preProcess_a[86] == 96)
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type =0;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[86] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,97) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[87] = 16;
		nIdentifiedNum_preProcess_a[87]=97;
	}
	else if(u5_blend_holdfrm_preProcess_a[87] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[87]--;
	}
	else if(nIdentifiedNum_preProcess_a[87] == 97)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[87] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,98) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=3;
		u5_blend_holdfrm_preProcess_a[88] = 16;
		nIdentifiedNum_preProcess_a[88]=98;
	}
	else if(u5_blend_holdfrm_preProcess_a[88] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=3;
		u5_blend_holdfrm_preProcess_a[88]--;
	}
	else if(nIdentifiedNum_preProcess_a[88] == 98)
	{
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=0;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=0;
		nIdentifiedNum_preProcess_a[88] = 0;
	}
	#endif
	#if Pro_SONY
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,99) == TRUE  )
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=2;
		s_pContext->_output_wrt_comreg.u1_ME1_adptpnt_rnd_gain_true=1;
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=2;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=1;

		u5_blend_holdfrm_preProcess_a[89] = 24;
		nIdentifiedNum_preProcess_a[89]=99;
	}
	else if(u5_blend_holdfrm_preProcess_a[89] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=2;
		s_pContext->_output_wrt_comreg.u1_ME1_adptpnt_rnd_gain_true=1;
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=2;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=1;

		u5_blend_holdfrm_preProcess_a[89]--;
	}
	else if(nIdentifiedNum_preProcess_a[89] == 99 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=0;
		s_pContext->_output_wrt_comreg.u1_ME1_adptpnt_rnd_gain_true=0;
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=0;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=0;
		nIdentifiedNum_preProcess_a[89] = 0;
	}
	#endif
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,102) == TRUE  )
	{
		s_pContext->_output_wrt_comreg.u1_GFB_Special_apply=1;
		u5_blend_holdfrm_preProcess_a[90] = 8;
		nIdentifiedNum_preProcess_a[90]=102;
	}
	else if(u5_blend_holdfrm_preProcess_a[90] >0)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_Special_apply=1;
		u5_blend_holdfrm_preProcess_a[90]--;
	}
	else if(nIdentifiedNum_preProcess_a[90] == 102 )
	{
		s_pContext->_output_wrt_comreg.u1_GFB_Special_apply=0;
		nIdentifiedNum_preProcess_a[90] = 0;
	}

	//++ YE Test for TV006 01_Black_19YO1 issue
	#if 1
     if((scalerVIP_Get_MEMCPatternFlag_Identification(0,103) == TRUE  )&&(u11_gmv_mvx <=0 && u10_gmv_mvy <=2))
	{
		pOutput->u8_01_Black_19YO1_flag=1;
		u5_blend_holdfrm_preProcess_a[91] = 24;
		nIdentifiedNum_preProcess_a[91]=103;
	}
	else if(u5_blend_holdfrm_preProcess_a[91] >0)
	{
		pOutput->u8_01_Black_19YO1_flag=1;
		u5_blend_holdfrm_preProcess_a[91]--;
	}
	else if(nIdentifiedNum_preProcess_a[91] == 103 )
	{
             pOutput->u8_01_Black_19YO1_flag=0;
		nIdentifiedNum_preProcess_a[91] = 0;
	}
	#endif 
	//-- YE Test for LG 01_Black_19YO1 issue

	//++ Q-13722 issue
	#if 1
     if((scalerVIP_Get_MEMCPatternFlag_Identification(0,104) == TRUE  )&&(u11_gmv_mvx ==0 && u10_gmv_mvy ==0))
	{
//		pOutput->u1_Q13722_flag =1;
		g_Q13722_flag = 1;
		u5_blend_holdfrm_preProcess_a[92] = 60;
		nIdentifiedNum_preProcess_a[92]=104;
	}
	else if(u5_blend_holdfrm_preProcess_a[104] >0)
	{
//		pOutput->u1_Q13722_flag=1;
		g_Q13722_flag = 1;
		u5_blend_holdfrm_preProcess_a[92]--;
	}
	else if(nIdentifiedNum_preProcess_a[92] == 104 )
	{
 //		pOutput->u1_Q13722_flag=0;
		g_Q13722_flag = 0;
		nIdentifiedNum_preProcess_a[92] = 0;
	}
	#endif 
	//-- Q-13722 issue

	#if 1//YE Test it's new place
	// ++ YE Test Edge Shaking
	if((s_pContext->_output_bRMV_rFB_ctrl.u8_ES_439_ID_flag ==1)||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_413_ID_flag ==1)
		||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_421_ID_flag ==1)||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_380_ID_flag ==1)
		||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag==1)	||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_21_ID_flag==1))
	{
	      if (s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag==1)
	      	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
		/*
		if(u32_ID_Log_en==1){
				rtd_pr_memc_info( "=====AAA GFB   ,weak=%d   ,custom=%d, MAX=%d  \n",s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type,
				s_pContext->_output_wrt_comreg.u1_GFB_force_Custom_true,s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true);
			}
		*/

	      	}
		else if((s_pContext->_output_bRMV_rFB_ctrl.u8_ES_413_ID_flag ==1)||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_421_ID_flag ==1)
			||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_380_ID_flag ==1)||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_21_ID_flag==1))
		{
			s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
			//s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type=1;
			/*
			if(u32_ID_Log_en==1)
			rtd_pr_memc_info( "=====BBB GFB   ,weak=%d   ,custom=%d, MAX=%d  \n",s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type,
			s_pContext->_output_wrt_comreg.u1_GFB_force_Custom_true,s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true);
			*/

		}
		else if (s_pContext->_output_bRMV_rFB_ctrl.u8_ES_439_ID_flag ==1)
		{
			s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
			//s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type=3;
			/*
			if(u32_ID_Log_en==1)
			rtd_pr_memc_info( "=====DDD GFB   ,weak=%d   ,custom=%d, MAX=%d  \n",s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type,
			s_pContext->_output_wrt_comreg.u1_GFB_force_Custom_true,s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true);
			*/

		}

		u5_blend_holdfrm_preProcess_a[93] = 24;
		nIdentifiedNum_preProcess_a[93]=109;
	}
	else if(u5_blend_holdfrm_preProcess_a[93] >0)
	{

	      if (s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag==1)
	      	{
			s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
			/*
			if(u32_ID_Log_en==1)
			rtd_pr_memc_info( "=====AAA GFB   ,weak=%d   ,custom=%d, MAX=%d  \n",s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type=1,
			s_pContext->_output_wrt_comreg.u1_GFB_force_Custom_true,s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true);
			*/

	      	}
		else if((s_pContext->_output_bRMV_rFB_ctrl.u8_ES_413_ID_flag ==1)||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_421_ID_flag ==1)
			||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_380_ID_flag ==1)||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_21_ID_flag==1))
		{
			s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
			//s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type=1;
			/*
			if(u32_ID_Log_en==1)
			rtd_pr_memc_info( "=====BBB GFB   ,weak=%d   ,custom=%d, MAX=%d  \n",s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type,
			s_pContext->_output_wrt_comreg.u1_GFB_force_Custom_true,s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true);
			*/

		}
		else if (s_pContext->_output_bRMV_rFB_ctrl.u8_ES_439_ID_flag ==1)
		{
			s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
			//s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type=3;
			/*
			if(u32_ID_Log_en==1)
			rtd_pr_memc_info( "=====DDD GFB   ,weak=%d   ,custom=%d, MAX=%d  \n",s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type,
			s_pContext->_output_wrt_comreg.u1_GFB_force_Custom_true,s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true);
			*/

		}

		u5_blend_holdfrm_preProcess_a[93]--;
	}
	else if(nIdentifiedNum_preProcess_a[93] == 109 )  // YE Test // it must be updated
	{

		if((s_pContext->_output_bRMV_rFB_ctrl.u8_ES_439_ID_flag ==0)||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_413_ID_flag ==0)
		||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_421_ID_flag ==0)||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_380_ID_flag ==0)
		||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag==0)	||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_21_ID_flag==0))
			{
				s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type=0;
				s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=0;
				s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=0;
				s_pContext->_output_wrt_comreg.u1_GFB_force_Custom_true=0;
				pOutput->u8_FB_lvl=0;
				s_pContext->_output_wrt_comreg.u1_GFB_modify_true=0;
			}
			//if(u32_ID_Log_en==1)
			//rtd_pr_memc_info( "=====EEE GFB   ,weak=%d   ,custom=%d, MAX=%d  \n",s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type,
			//s_pContext->_output_wrt_comreg.u1_GFB_force_Custom_true,s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true);


		nIdentifiedNum_preProcess_a[93] = 0;
	}
	// -- YE Test Edge Shaking
	#endif

	#if 1
	// ++ YE Test Rassia Fields
//     if(scalerVIP_Get_MEMCPatternFlag_Identification(0,111) == TRUE  )
     if((scalerVIP_Get_MEMCPatternFlag_Identification(0,111) == TRUE  )&&(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_Rassia_ID_flag ==1))
	{

		s_pContext->_output_wrt_comreg.u1_GFB_force_Custom_true=1;

		u5_blend_holdfrm_preProcess_a[94] = 24;
		nIdentifiedNum_preProcess_a[94]=111;
	}
	else if(u5_blend_holdfrm_preProcess_a[94] >0)
	{

		s_pContext->_output_wrt_comreg.u1_GFB_force_Custom_true=1;

		u5_blend_holdfrm_preProcess_a[94]--;
	}
	else if(nIdentifiedNum_preProcess_a[94] == 111 )
	{

			if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_Rassia_ID_flag ==0)
			{
			s_pContext->_output_wrt_comreg.u1_GFB_force_Custom_true=0;
			pOutput->u8_FB_lvl=0;
			s_pContext->_output_wrt_comreg.u1_GFB_modify_true=0;
			}
		nIdentifiedNum_preProcess_a[94] = 0;
	}
	// -- YE Test Edge Shaking
	#endif

// -- YE Test Rassia Fields
	// ++  merlin6 new algo for #pattern214

	if((scalerVIP_Get_MEMCPatternFlag_Identification(0,120) == TRUE)&&(u11_gmv_mvx>=25)&&(u11_gmv_mvx<=80)) //&&( u25_me_aDTL >= 656575 )&&(u25_me_aDTL<=1221649)&&(u11_gmv_mvx>=25)&&(u11_gmv_mvx<=80)&&(u10_gmv_mvy<=5)&&(u10_gmv_mvy>=-11)
	{
	// if(u32_pattern214_forbindden_en == 1){
			//++solution settings:
			//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 4, 7, 0x1);
#if 0 // merlin7_bring_up_dehalo
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 29 , 29 , 1 ); // new algo switch
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 28 , 28 , 1 ); //UL
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 27 , 27 , 1 ); //UR
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 26 , 26 , 0 ); //DL
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 25 , 25 , 1 ); //DR
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 24 , 24 , 0 ); //APL sel
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 18 , 23 , 20 );//hi_APL_num_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 12 , 17 , 20 );//low_APL_num_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 6 , 11 ,  20 );//near_Gmv_num_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 0 , 5 ,  20 ); //far_Gmv_num_th

			WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 0 , 7 ,  20 ); //low_Gmv_diff_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 8 , 15 ,  40 ); //high_Gmv_diff_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 16 , 23 ,  40 ); //low_APL_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 24 , 31 ,  50 ); //high_APL_th

			WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 0 , 4 , 0 ); //gmv_num
			WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 6 , 7 , 1 ); //gmv_len
			WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 8 , 15 , 50 ); //set_occl_apl_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 16 , 23 , 50 ); //apl_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 24 , 31 , 2); //Gmv_diff_th

			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 0 , 0 , 1 ); //rtgmvd_bypass
			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 1 , 1 , 0 ); //gmv_bypass
			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 2 , 3 , 1 ); //case_mark_mode
			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 4 , 4 , 0 ); //result_apply
			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 5 , 5 , 0 );//pred_occl_en
			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 6 , 6 , 1 ); //occl_hor_en
			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 7 , 7 , 1 ); //occl_ver_en
			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 8 , 8 , 0 ); //debug mode

			WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 0 , 2 , 0 );  //res_num
			WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 3 , 4 , 0 );  //res_mode
			WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 5 , 9 , 2 );  //rim down
			WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 10 , 14 , 2 ); //rim up
			WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 15 , 19 , 0 );  //rim right
			WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 20 , 24 , 0 ); //rim left

			WriteRegister(KME_DEHALO4_RT2_MOD_1_reg , 0 , 0 , 0 ); //fix1 en
			WriteRegister(KME_DEHALO4_RT2_MOD_1_reg , 1 , 1 , 0 ); //fix1 en
			WriteRegister(KME_ME1_TOP10_APL_FILTER0_reg , 0 , 0 , 0 );
			//--solution settings:
#endif			
		//dehalo 214 djntest
        WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,28,29,0x1); //pick bg mv
        WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x1);  //penalty_en
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
        WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,0x9); //me1 pred th0
        WriteRegister(KME_DEHALO5_PHMV_FIX_1A_reg,26,31,0x0); //correct cut
        WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,10,13,0x0); //extend cut
        WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0xa); //extend ucov th0
        WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0xa);  //extend cov th0
        WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x19); // color protect th max0
        WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,2,2,0x1); // mv filter en
        WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x258); //posdiff th1
        WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,18,26,0xdf); //posdiff th2
        WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,16,24,0x1a4); //posdiff th3
        WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,0,6,0x57); //useless bg th
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
        WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23, 0x14);
        WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25, 0x3ff);
        WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,24,30, 0x64);
        WriteRegister(KME_DEHALO5_PHMV_FIX_10_reg,24,31, 0x3f);

        WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x0);
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x4);        
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x1);
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x4);  
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x1); 
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x4);        
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x1);
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x4);
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x1);
		
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

        WriteRegister(HARDWARE_HARDWARE_24_reg , 19, 28, 0x0);
        WriteRegister(HARDWARE_HARDWARE_24_reg , 31, 31, 0x0);    

        WriteRegister(HARDWARE_HARDWARE_26_reg,31,31, 0x0); //u8_dh_condition_dhbypass         
		//    rtd_pr_memc_info("in214");
	//	}

		u5_blend_holdfrm_preProcess_a[95] = 30;
		nIdentifiedNum_preProcess_a[95]=120;
	}
	else if(u5_blend_holdfrm_preProcess_a[95] >0){
		//if(u32_pattern214_forbindden_en == 1){
			//++solution settings:
			//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 4, 7, 0x2);
#if 0 // merlin7_bring_up_dehalo
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 29 , 29 , 1 ); // new algo switch
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 28 , 28 , 1 ); //UL
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 27 , 27 , 1 ); //UR
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 26 , 26 , 0 ); //DL
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 25 , 25 , 1 ); //DR
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 24 , 24 , 0 ); //APL sel
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 18 , 23 , 20 );//hi_APL_num_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 12 , 17 , 20 );//low_APL_num_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 6 , 11 ,  20 );//near_Gmv_num_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 0 , 5 ,  20 ); //far_Gmv_num_th

			WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 0 , 7 ,  20 ); //low_Gmv_diff_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 8 , 15 ,  40 ); //high_Gmv_diff_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 16 , 23 ,  40 ); //low_APL_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 24 , 31 ,  50 ); //high_APL_th

			WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 0 , 4 , 0 ); //gmv_num
			WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 6 , 7 , 1 ); //gmv_len
			WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 8 , 15 , 50 ); //set_occl_apl_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 16 , 23 , 50 ); //apl_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 24 , 31 , 2); //Gmv_diff_th

			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 0 , 0 , 1 ); //rtgmvd_bypass
			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 1 , 1 , 0 ); //gmv_bypass
			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 2 , 3 , 1 ); //case_mark_mode
			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 4 , 4 , 0 ); //result_apply
			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 5 , 5 , 0 );//pred_occl_en
			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 6 , 6 , 1 ); //occl_hor_en
			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 7 , 7 , 1 ); //occl_ver_en
			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 8 , 8 , 0 ); //debug mode

			WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 0 , 2 , 0 );  //res_num
			WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 3 , 4 , 0 );  //res_mode
			WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 5 , 9 , 2 );  //rim down
			WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 10 , 14 , 2 ); //rim up
			WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 15 , 19 , 0 );  //rim right
			WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 20 , 24 , 0 ); //rim left

			WriteRegister(KME_DEHALO4_RT2_MOD_1_reg , 0 , 0 , 0 ); //fix1 en
			WriteRegister(KME_DEHALO4_RT2_MOD_1_reg , 1 , 1 , 0 ); //fix1 en
			WriteRegister(KME_ME1_TOP10_APL_FILTER0_reg , 0 , 0 , 0 );
			//--solution settings:
#endif			
		WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,28,29,0x1); //pick bg mv
        WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x1);  //penalty_en
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
        WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,0x9); //me1 pred th0
        WriteRegister(KME_DEHALO5_PHMV_FIX_1A_reg,26,31,0x0); //correct cut
        WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,10,13,0x0); //extend cut
        WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0xa); //extend ucov th0
        WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0xa);  //extend cov th0
        WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x19); // color protect th max0
        WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,2,2,0x1); // mv filter en
        WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x258); //posdiff th1
        WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,18,26,0xdf); //posdiff th2
        WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,16,24,0x1a4); //posdiff th3
        WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,0,6,0x57); //useless bg th
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

		WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23, 0x14);
        WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25, 0x3ff);
        WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,24,30, 0x64);
        WriteRegister(KME_DEHALO5_PHMV_FIX_10_reg,24,31, 0x3f);

		WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x0);
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x4);        
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x1);
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x4);  
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x1); 
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x4);        
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x1);
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x4);
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x1);

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
			
        WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,16,20,0x10); //filter cov th0
        WriteRegister(KME_DEHALO5_PHMV_FIX_7_reg,8,12,0x10); //filter ucov th0
        WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x1a); // color protect th max0
        
        WriteRegister(HARDWARE_HARDWARE_24_reg , 19, 28, 0);
        WriteRegister(HARDWARE_HARDWARE_24_reg , 31, 31, 0);         
         //      rtd_pr_memc_info("in214 hold\n");    
        WriteRegister(HARDWARE_HARDWARE_26_reg,31,31, 0x0); //u8_dh_condition_dhbypass

        u5_blend_holdfrm_preProcess_a[95]--;
	}
	else if(nIdentifiedNum_preProcess_a[95] == 120 ){
	//	if(u32_pattern214_forbindden_en == 1){
			//++solution settings:
			//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 4, 7, 0x0);
#if 0 // merlin7_bring_up_dehalo
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 29 , 29 , 0 ); // new algo switch
			WriteRegister(KME_DEHALO4_RT2_MOD_1_reg , 0 , 0 , 0 ); //fix1 en
			WriteRegister(KME_DEHALO4_RT2_MOD_1_reg , 1 , 1 , 0 ); //fix1 en
			WriteRegister(KME_ME1_TOP10_APL_FILTER0_reg , 0 , 0 , 0 );
			//--solution settings:
#endif			
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
        WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,0x16);
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
        WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23, 0x0);
        WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25, 0x12c);
        WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,24,30, 0x57);
        WriteRegister(KME_DEHALO5_PHMV_FIX_10_reg,24,31, 0x0);

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
            
       //     WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 8, 11, 0xE);
       //     rtd_pr_memc_info("out214");
            
        WriteRegister(HARDWARE_HARDWARE_24_reg , 19, 28, 0x3FF);
        WriteRegister(HARDWARE_HARDWARE_24_reg , 31, 31, 1);     
        WriteRegister(HARDWARE_HARDWARE_26_reg,31,31, 0x1); //u8_dh_condition_dhbypass  

    //    }

		nIdentifiedNum_preProcess_a[95] = 0;
	}
	
	// 153 merilin7 
	if((scalerVIP_Get_MEMCPatternFlag_Identification(0,119) == TRUE)) //&&( u25_me_aDTL >= 656575 )&&(u25_me_aDTL<=1221649)&&(u11_gmv_mvx>=25)&&(u11_gmv_mvx<=80)&&(u10_gmv_mvy<=5)&&(u10_gmv_mvy>=-11)
	{
		gmv_calc_x = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[14];
		gmv_calc_y = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[14];
		
		WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x1);
		
		//   WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 12, 15, 0xF);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 1);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, gmv_calc_x);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, gmv_calc_y);
		
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, gmv_calc_x);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, gmv_calc_y);

        WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x1);  //penalty_en
        WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x1); //divide en
            
		u5_blend_holdfrm_preProcess_a[97] = 30;
		nIdentifiedNum_preProcess_a[97]=119;
		 //	rtd_pr_memc_info("in153");
	}
	else if(u5_blend_holdfrm_preProcess_a[97] >0){
		WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x1);  
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 12, 15, 0xF);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, gmv_calc_x);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, gmv_calc_y);

		WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x1);  //penalty_en
        WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x1); //divide en

		//	rtd_pr_memc_info("in153");
		
		 u5_blend_holdfrm_preProcess_a[97]--;
	}
	else if(nIdentifiedNum_preProcess_a[97] == 119 ){
	 
		WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x0);	// init
	 	// 	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 12, 15, 0xE);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0);

		WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x0);	//penalty_en
		WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x0); //divide en
		// 	rtd_pr_memc_info("out153");

		nIdentifiedNum_preProcess_a[97] = 0;
	}
	

	// --  merlin6 new algo for #pattern214

	// ++  #181

	if((scalerVIP_Get_MEMCPatternFlag_Identification(0,117) == TRUE)&&(u11_gmv_mvx<=-34 && u11_gmv_mvx>=-44)&&(u10_gmv_mvy<=-55 && u10_gmv_mvy>=-68)
		&& (u32_gmv_unconf<=6) && (u25_me_aDTL<=3260000 && u25_me_aDTL>=3200000)){
		WriteRegister(HARDWARE_HARDWARE_57_reg, 14, 14, 0x0); // u1_scCtrl_wrt_en;
		WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg, 9, 13, 0x0);
		u5_blend_holdfrm_preProcess_a[96] = 10;
		nIdentifiedNum_preProcess_a[96]=117;
	}else if(u5_blend_holdfrm_preProcess_a[96] >0){
		WriteRegister(HARDWARE_HARDWARE_57_reg, 14, 14, 0x0); // u1_scCtrl_wrt_en;
		WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg, 9, 13, 0x0);
		u5_blend_holdfrm_preProcess_a[96]--;
	}else if(nIdentifiedNum_preProcess_a[96] == 117){
		WriteRegister(HARDWARE_HARDWARE_57_reg, 14, 14, 0x1); // u1_scCtrl_wrt_en;
		nIdentifiedNum_preProcess_a[96] = 0;
	}

	if(u5_blend_holdfrm_preProcess_a[96] >0)
		g_181_flag = 1;
	else
		g_181_flag = 0;

	// --  #181


    //---------------------------189------------------------------//
    right_rim_pre=s_pContext->_output_rimctrl.u9_dh_blk_rim[_RIM_RHT];
    if(scalerVIP_Get_MEMCPatternFlag_Identification(0,121) == TRUE) //189
    {
        u5_blend_holdfrm_preProcess_a[121] = 30;
        nIdentifiedNum_preProcess_a[121]=117;
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0x1);
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, 0x28);
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0x0);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0x1);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, 0x28);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, 0x0);
        WriteRegister(HARDWARE_HARDWARE_25_reg , 9, 9, 1);

		ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,31,31, &fwgmven);
        ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,0,10, &fwgmvtest);
        if(fwgmven)
        {
             WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, fwgmvtest);   
        }
        
        WriteRegister(HARDWARE_HARDWARE_25_reg , 9, 9, 1);
        
        WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,9,17,  right_rim_pre-15);
    }
    else if(u5_blend_holdfrm_preProcess_a[121] >0)
    {
    	u5_blend_holdfrm_preProcess_a[121]--;
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0x1);
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, 0x28);
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0x0);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0x1);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, 0x28);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, 0x0);
	    ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,31,31, &fwgmven);
        ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,0,10, &fwgmvtest);
        if(fwgmven)
        {
             WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,  fwgmvtest);   
        }     
        WriteRegister(HARDWARE_HARDWARE_25_reg , 9, 9, 1);
        WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,9,17,  right_rim_pre-15);
    }
    else if(nIdentifiedNum_preProcess_a[121] == 117)
    {
    	 nIdentifiedNum_preProcess_a[121] = 0;
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0x0);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0x0);
        WriteRegister(HARDWARE_HARDWARE_25_reg , 9, 9, 0);
		WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,9,17,  right_rim_pre);
    }
    
    //old 189 and 299, no use
    if(0)
    {
    	if((scalerVIP_Get_MEMCPatternFlag_Identification(0,122) == TRUE) && (scalerVIP_Get_MEMCPatternFlag_Identification(0,121) == 0)) 
    	{

    		for(nIdxX=0; nIdxX<32; nIdxX++)
    		{
    			s11_rmv_mvx[nIdxX] = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[nIdxX];
    			s10_rmv_mvy[nIdxX] = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nIdxX];
    		}

			for(nIdxX=0; nIdxX<32; nIdxX++)
			{
				u32_RgnApl[nIdxX] = 0;
				u32_RgnApl[nIdxX] = (s_pContext->_output_read_comreg.u20_me_rAPLi_rb[nIdxX] + s_pContext->_output_read_comreg.u20_me_rAPLp_rb[nIdxX]+1012)/2024;
			}
			
			gmv_calc_rapl_min_temp = u32_RgnApl[8];
			gmv_calc_rapl_min_idx = 8;
								
			for (nIdxX = 0; nIdxX <= 31; nIdxX++)
			{
				if((nIdxX != 1) &&(nIdxX != 2)&&(nIdxX != 11)&&(nIdxX != 22))
				{
					if(u32_RgnApl[nIdxX] < gmv_calc_rapl_min_temp)
					{
						gmv_calc_rapl_min_temp = u32_RgnApl[nIdxX];
						gmv_calc_rapl_min_idx = nIdxX;
					}
				}					
			}
					
			gmv_calc_x = s11_rmv_mvx[gmv_calc_rapl_min_idx];
			gmv_calc_y = s10_rmv_mvy[gmv_calc_rapl_min_idx];
			rmv_temp_x = s11_rmv_mvx[gmv_calc_rapl_min_idx];
			swap_s1_flag = 1;
			if(frameCnt_tmp % 2 == 1)
			{
				cond1 = (_ABS_DIFF_(gmv_calc_x , pre_calc_gmv_mvx) > reg_gmvd_th) ? 1 : 0;
				pre_calc_gmv_mvx = gmv_calc_x;
				pre_calc_gmv_rgn_idx = gmv_calc_rapl_min_idx;		
			}
			else
			{
				cond0 = (_ABS_DIFF_(gmv_calc_x , pre_calc_gmv_mvx) > reg_gmvd_th) ? 1 : 0; 
				pre_calc_gmv_mvx = gmv_calc_x;
				pre_calc_gmv_rgn_idx = gmv_calc_rapl_min_idx;
			}

			if(cond0 || cond1)
			{
				for(nIdxX=0; nIdxX<32; nIdxX++)
				{
					if((nIdxX != 1)&&(nIdxX != 2)&&(nIdxX != 11)&&(nIdxX != 22)&&(nIdxX != 31)&&(nIdxX != 25)&&(nIdxX != 7))
					{
						if(u32_RgnApl[nIdxX] < 80)
						{
							gmvd_calc_x = _ABS_DIFF_(pre_calc_gmv_mvx , s11_rmv_mvx[nIdxX]);
							//gmvd_calc_y = abs(pre_calc_gmv_mvy - s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nIdxX]);
							if(gmvd_calc_x_min > gmvd_calc_x)
							{
												  gmvd_calc_x_min = gmvd_calc_x;  
												  gmvd_calc_x_min_idx = nIdxX;
							}
						}	
					}
				}
				gmv_calc_x = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[gmvd_calc_x_min_idx];
				gmv_calc_y = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[gmvd_calc_x_min_idx]; 
				swap_s2_flag = 1;
			}			
			if(print_detail_en)
			{
				rtd_pr_memc_emerg("func:189_path\n");	
			}
			//pOutput->s1_gmv_calc_mvx = gmv_calc_x;
			//pOutput->s1_gmv_calc_mvy = gmv_calc_y;
			WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 1);
			WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, gmv_calc_x);
			WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0);
			WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
			WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, gmv_calc_x);
			WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, gmv_calc_y);

			//WriteRegister(HARDWARE_HARDWARE_57_reg, 14, 14, 0x0); // u1_scCtrl_wrt_en;
    		//WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg, 9, 13, 0x0);
    	    //WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 1);
    		//WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, 45);
    		//WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0);

            //WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
            //WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, 45);
            //WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, 0);

            WriteRegister(HARDWARE_HARDWARE_25_reg , 8,8, 1);
            
    		u5_blend_holdfrm_preProcess_a[122] = 10;
    		nIdentifiedNum_preProcess_a[122]=118;
    	}
    	else if(u5_blend_holdfrm_preProcess_a[122] >0)
    	{
            WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 1);
            WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, gmv_calc_x);
            WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0);
            WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
            WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, gmv_calc_x);
            WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, gmv_calc_y);
    		//WriteRegister(HARDWARE_HARDWARE_57_reg, 14, 14, 0x0); // u1_scCtrl_wrt_en;
    		//WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg, 9, 13, 0x0);
    		//WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 1);
    		//WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, 45);
    		//WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0);
		
            //WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
            //WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, 45);
            //WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, 0);
                //WriteRegister(HARDWARE_HARDWARE_25_reg , 8, 8, 1);
            
    		u5_blend_holdfrm_preProcess_a[122]--;
    	}
    	else if(nIdentifiedNum_preProcess_a[122] == 118)
    	{
            WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0);
                //WriteRegister(HARDWARE_HARDWARE_57_reg, 14, 14, 0x1); // u1_scCtrl_wrt_en;

                WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0);

				 nIdentifiedNum_preProcess_a[122] = 0;
				 WriteRegister(HARDWARE_HARDWARE_25_reg , 8, 8, 0);
		 }
	 }
	 
	 //---------------------------299 last------------------------------//
    if((scalerVIP_Get_MEMCPatternFlag_Identification(0,130) == TRUE) && (scalerVIP_Get_MEMCPatternFlag_Identification(0,121) == 0)) //299
     {
         u5_blend_holdfrm_preProcess_a[130] = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 240 : 120;
         nIdentifiedNum_preProcess_a[130]=118;
         WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0x1);
         WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, 0x7d6);
         WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0x0);
         WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0x1);
         WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, 0x7d6);
         WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, 0x0);
         WriteRegister(HARDWARE_HARDWARE_25_reg , 8, 8, 1);
         //WriteRegister(HARDWARE_HARDWARE_25_reg , 12, 12, 0);
        //  WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 0, 3, 0x1); 
     }
     else if(u5_blend_holdfrm_preProcess_a[130] >0)
     {
         u5_blend_holdfrm_preProcess_a[130]--;
         WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0x1);
         WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, 0x7d6);
         WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0x0);
         WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0x1);
         WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, 0x7d6);
         WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, 0x0);
         WriteRegister(HARDWARE_HARDWARE_25_reg , 8, 8, 1);
         //WriteRegister(HARDWARE_HARDWARE_25_reg , 12, 12, 0);
    
       //   WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 0, 3, 0x2); 
     }
     else if(nIdentifiedNum_preProcess_a[130] == 118)
     {
          nIdentifiedNum_preProcess_a[130] = 0;
         WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0x0);
         WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0x0);
         WriteRegister(HARDWARE_HARDWARE_25_reg , 8, 8, 0);
        // WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 0, 3, 0x0); 
         //WriteRegister(HARDWARE_HARDWARE_25_reg , 12, 12, 0);
     }
    
     //---------------------------299 begin------------------------------//
    if((scalerVIP_Get_MEMCPatternFlag_Identification(0,129) == TRUE) && (scalerVIP_Get_MEMCPatternFlag_Identification(0,130) == TRUE) && (scalerVIP_Get_MEMCPatternFlag_Identification(0,121) == 0)) //299
     {
         incnt_for_299++;
         incnt_for_299 = (incnt_for_299 >= 15) ? 15 : incnt_for_299;
     }
    else
    {
         incnt_for_299 = 0;
    }
    
    if(incnt_for_299 == 15)
    {
         u5_blend_holdfrm_preProcess_a[129] = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 31 : 30;
         nIdentifiedNum_preProcess_a[129]=119;
         WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0x1);
         WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, 0x7e7);
         WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0x0);
         WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0x1);
         WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, 0x7e7);
         WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, 0x0);
         WriteRegister(HARDWARE_HARDWARE_25_reg , 8, 8, 1);
         WriteRegister(HARDWARE_HARDWARE_25_reg , 12, 12, 1);
    }
     else if(u5_blend_holdfrm_preProcess_a[129] >0)
     {
          u5_blend_holdfrm_preProcess_a[129]--;
         WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0x1);
         WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, 0x7e7);
         WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0x0);
         WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0x1);
         WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, 0x7e7);
         WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, 0x0);
         WriteRegister(HARDWARE_HARDWARE_25_reg , 8, 8, 1);
         WriteRegister(HARDWARE_HARDWARE_25_reg , 12, 12, 1);
     }
     else if(nIdentifiedNum_preProcess_a[129] == 119)
     {
          nIdentifiedNum_preProcess_a[129] = 0;
         WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0x0);
         WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0x0);
         WriteRegister(HARDWARE_HARDWARE_25_reg , 8, 8, 0);
         WriteRegister(HARDWARE_HARDWARE_25_reg , 12, 12, 0);
     }
	 
	 //rtd_pr_memc_info("189: %d %d  299: %d %d", (scalerVIP_Get_MEMCPatternFlag_Identification(0,121) == TRUE), u5_blend_holdfrm_preProcess_a[121], ((scalerVIP_Get_MEMCPatternFlag_Identification(0,122) == TRUE) && (scalerVIP_Get_MEMCPatternFlag_Identification(0,121) == 0)), u5_blend_holdfrm_preProcess_a[122]);
	 //rtd_pr_memc_info("299 begin(%d)  incnt(%d) after(%d)", u5_blend_holdfrm_preProcess_a[129], incnt_for_299, u5_blend_holdfrm_preProcess_a[130]);
//#45  djntest
	if((scalerVIP_Get_MEMCPatternFlag_Identification(0,123) == TRUE)&&(s_pContext->_output_read_comreg.me1_gmv_bgcnt>8000)) //&&( u25_me_aDTL >= 656575 )&&(u25_me_aDTL<=1221649)&&(u11_gmv_mvx>=25)&&(u11_gmv_mvx<=80)&&(u10_gmv_mvy<=5)&&(u10_gmv_mvy>=-11)
    {
		WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,12,12,0x0);  //reg_dh_only_mv_fix_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x0);//reg_dh_bg_extend_cond3_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,1,1,0x1);//reg_dh_pstflt_occl_sel 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,0,0,0x1);//reg_dh_pstflt_mv_sel 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x64);  //reg_dh_color_protect_th0_max 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,25,30,0x6);  //reg_dh_pred_bg_change_pfv_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_36_reg,10,15,0x6);  //reg_dh_pred_bg_change_ppfv_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,11,11,0x1);  //reg_dh_fg_r_big_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,9,9,0x1);  //reg_dh_fg_l_big_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,7,7,0x1);  //reg_dh_fg_c_big_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,6,6,0x1);  //reg_dh_fg_protect_by_apl_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_3B_reg,24,31,0xf);  //reg_dh_fg_lr_big_min_cur_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_3B_reg,16,23,0xf);  //reg_dh_fg_lr_big_min_pre_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_3B_reg,8,15,0xf);  //reg_dh_fg_lr_big_max_cur_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_3B_reg,0,7,0xf);  //reg_dh_fg_lr_big_max_pre_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_43_reg,8,13,0x5);  //reg_dh_fg_me2_sim_max_diff_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_43_reg,0,5,0x5);  //reg_dh_fg_me2_sim_min_diff_th 45

		WriteRegister(HARDWARE_HARDWARE_24_reg , 19, 28, 0x0);
		WriteRegister(HARDWARE_HARDWARE_24_reg , 31, 31, 0x0);	
		u5_blend_holdfrm_preProcess_a[99] = 30;
		nIdentifiedNum_preProcess_a[99]=123;

	}
	else if(u5_blend_holdfrm_preProcess_a[99] >0)
	{
		WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,12,12,0x0);  //reg_dh_only_mv_fix_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x0);//reg_dh_bg_extend_cond3_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,1,1,0x1);//reg_dh_pstflt_occl_sel 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,0,0,0x1);//reg_dh_pstflt_mv_sel 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x64);  //reg_dh_color_protect_th0_max 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,25,30,0x6);  //reg_dh_pred_bg_change_pfv_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_36_reg,10,15,0x6);  //reg_dh_pred_bg_change_ppfv_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,11,11,0x1);  //reg_dh_fg_r_big_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,9,9,0x1);  //reg_dh_fg_l_big_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,7,7,0x1);  //reg_dh_fg_c_big_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,6,6,0x1);  //reg_dh_fg_protect_by_apl_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_3B_reg,24,31,0xf);  //reg_dh_fg_lr_big_min_cur_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_3B_reg,16,23,0xf);  //reg_dh_fg_lr_big_min_pre_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_3B_reg,8,15,0xf);  //reg_dh_fg_lr_big_max_cur_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_3B_reg,0,7,0xf);  //reg_dh_fg_lr_big_max_pre_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_43_reg,8,13,0x5);  //reg_dh_fg_me2_sim_max_diff_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_43_reg,0,5,0x5);  //reg_dh_fg_me2_sim_min_diff_th 45
					
		WriteRegister(HARDWARE_HARDWARE_24_reg , 19, 28, 0x0);
		WriteRegister(HARDWARE_HARDWARE_24_reg , 31, 31, 0x0);	
	 
		u5_blend_holdfrm_preProcess_a[99]--;
	}
	else if(nIdentifiedNum_preProcess_a[99] == 123 )
	{		  
		WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,12,12,0x1);  //reg_dh_only_mv_fix_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x1);//reg_dh_bg_extend_cond3_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,1,1,0x0);//reg_dh_pstflt_occl_sel 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,0,0,0x0);//reg_dh_pstflt_mv_sel 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x22);  //reg_dh_color_protect_th0_max 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,25,30,0x8);  //reg_dh_pred_bg_change_pfv_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_36_reg,10,15,0x8);  //reg_dh_pred_bg_change_ppfv_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,11,11,0x0);  //reg_dh_fg_r_big_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,9,9,0x0);  //reg_dh_fg_l_big_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,7,7,0x0);  //reg_dh_fg_c_big_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,6,6,0x0);  //reg_dh_fg_protect_by_apl_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_3B_reg,24,31,0x12);	//reg_dh_fg_lr_big_min_cur_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_3B_reg,16,23,0x12);	//reg_dh_fg_lr_big_min_pre_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_3B_reg,8,15,0x12);  //reg_dh_fg_lr_big_max_cur_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_3B_reg,0,7,0x12);  //reg_dh_fg_lr_big_max_pre_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_43_reg,8,13,0x0);  //reg_dh_fg_me2_sim_max_diff_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_43_reg,0,5,0x9);  //reg_dh_fg_me2_sim_min_diff_th 45

		WriteRegister(HARDWARE_HARDWARE_24_reg , 19, 28, 0x3FF);
		WriteRegister(HARDWARE_HARDWARE_24_reg , 31, 31, 1); 
	
		nIdentifiedNum_preProcess_a[99] = 0;
	}
	

//#298 end

    
	/*if((scalerVIP_Get_MEMCPatternFlag_Identification(0,122) == TRUE))
	{
		if(print_detail_en)
		{
			rtd_pr_memc_emerg("func:299_path\n");	
		}
		//WriteRegister(HARDWARE_HARDWARE_57_reg, 14, 14, 0x0); // u1_scCtrl_wrt_en;
		//WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg, 9, 13, 0x0);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 1);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, -40);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0);
       // WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,25); 
      //  WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,25); 
      //  WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,25); 

       WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
       WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, 45);
       WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, 0);

       WriteRegister(HARDWARE_HARDWARE_24_reg , 17, 17, 0);

       
		u5_blend_holdfrm_preProcess_a[122] = 10;
		nIdentifiedNum_preProcess_a[122]=117;
	}
	else if(u5_blend_holdfrm_preProcess_a[121] >0)
	{
		//WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 1);
		//WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, gmv_calc_x);
		//WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, gmv_calc_y);
		//WriteRegister(HARDWARE_HARDWARE_57_reg, 14, 14, 0x0); // u1_scCtrl_wrt_en;
		//WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg, 9, 13, 0x0);
	    WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 1);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, 45);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0);


        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, 45);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, 0);

        WriteRegister(HARDWARE_HARDWARE_24_reg , 17, 17, 0);
		u5_blend_holdfrm_preProcess_a[122]--;
	}
	else if(nIdentifiedNum_preProcess_a[122] == 117)
	{
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0);
		//WriteRegister(HARDWARE_HARDWARE_57_reg, 14, 14, 0x1); // u1_scCtrl_wrt_en;

    WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0);

    WriteRegister(HARDWARE_HARDWARE_24_reg , 17, 17, 1);
		nIdentifiedNum_preProcess_a[122] = 0;
	}
    */

    //------299  
	//---memc_identification test
	if((scalerVIP_Get_MEMCPatternFlag_Identification(0,123) == TRUE))
	{
		u5_blend_holdfrm_preProcess_a[99] = 30;
		nIdentifiedNum_preProcess_a[99]=123;
	}
	else if(u5_blend_holdfrm_preProcess_a[99] >0)
	{
		u5_blend_holdfrm_preProcess_a[99]--;
	}
	else if(nIdentifiedNum_preProcess_a[99] == 123 )
	{         
		nIdentifiedNum_preProcess_a[99] = 0;
	}

// --  #189


	//===debug show
	for(u8_Index =0;u8_Index<255;u8_Index++)
	{
		if(nIdentifiedNum_preProcess_a[u8_Index] !=0){
			if(u32_ID_Log_en==1)
			rtd_pr_memc_emerg("===== [IPRpre_001] nIdentifiedNum_preProcess_a>>[%d][%d]\n", u8_Index,nIdentifiedNum_preProcess_a[u8_Index]);

			pOutput->u1_IP_preProcess_true=1;

		}
	}
	//===
}


VOID Identification_Pattern_preProcess_TV002(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	_PQLCONTEXT *s_pContext = GetPQLContext_m();
	static unsigned short  u5_blend_holdfrm_preProcess_a[255] = {0};
	static unsigned char  nIdentifiedNum_preProcess_a[255] = {0};
	unsigned char  u8_Index;	
	unsigned int  u32_pattern214_forbindden_en = 0;
	//unsigned int	u32_RB_val;
	//unsigned int	u32_pattern214_forbindden_en;
	//unsigned char  u1_still_frame=s_pContext->_output_frc_sceneAnalysis.u1_still_frame;
	short  u11_gmv_mvx = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
	//short  u10_gmv_mvy = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;
	//unsigned int	u25_me_aDTL = s_pContext->_output_read_comreg.u25_me_aDTL_rb;
	//unsigned short  u32_gmv_unconf = s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb;
	//UBYTE  u8_cadence_Id = s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL];
	//PQL_INPUT_FRAME_RATE in_fmRate = s_pContext->_external_data._input_frameRate;
	//unsigned char nblend_y_alpha_Target=0xa0;
	//unsigned int u32_RB_kmc_blend_y_alpha;
	//unsigned int u25_rgnSad_rb12 = s_pContext->_output_read_comreg.u25_me_rSAD_rb[12];
	//unsigned int u25_rgnSad_rb23 = s_pContext->_output_read_comreg.u25_me_rSAD_rb[23];
	unsigned int u32_ID_Log_en=0;//,ES_Test_Value=0;
	unsigned int BOT_ratio = 0;
	int diff = 0;
	ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 24,  24, &u32_ID_Log_en);//YE Test debug dummy register
	//ReadRegister(FRC_TOP__IPPRE__reg_kmc_blend_y_alpha_ADDR, 4, 11, &u32_RB_kmc_blend_y_alpha);
	//ReadRegister(FRC_TOP__PQL_0__pql_patch_dummy_e8_ADDR, 0, 31, &u32_RB_val);   //  bit0
	
	diff = s_pContext->_output_rimctrl.u12_out_resolution[_RIM_BOT] - s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_BOT];
	BOT_ratio = ((_ABS_(diff))<<10)>>11;
	//reset
	pOutput->u1_IP_preProcess_true=0;

	if(pParam->u1_Identification_Pattern_en== 0)
	{
		return;
	}
		// -- YE Test Rassia Fields
		// ++  merlin6 new algo for #pattern214		
		if((scalerVIP_Get_MEMCPatternFlag_Identification(1,1) == TRUE)&&(u11_gmv_mvx>=25)&&(u11_gmv_mvx<=80)) //&&( u25_me_aDTL >= 656575 )&&(u25_me_aDTL<=1221649)&&(u11_gmv_mvx>=25)&&(u11_gmv_mvx<=80)&&(u10_gmv_mvy<=5)&&(u10_gmv_mvy>=-11)
		{
			if(u32_pattern214_forbindden_en == 1){
				//++solution settings:
				//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 4, 7, 0x1);
#if 0//Merlin7 does not have those register
				WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 29 , 29 , 1 ); // new algo switch
				WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 28 , 28 , 1 ); //UL
				WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 27 , 27 , 1 ); //UR
				WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 26 , 26 , 0 ); //DL
				WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 25 , 25 , 1 ); //DR
				WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 24 , 24 , 0 ); //APL sel
				WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 18 , 23 , 20 );//hi_APL_num_th
				WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 12 , 17 , 20 );//low_APL_num_th
				WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 6 , 11 ,	20 );//near_Gmv_num_th
				WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 0 , 5 ,  20 ); //far_Gmv_num_th
		
				WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 0 , 7 ,  20 ); //low_Gmv_diff_th
				WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 8 , 15 ,	40 ); //high_Gmv_diff_th
				WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 16 , 23 ,  40 ); //low_APL_th
				WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 24 , 31 ,  50 ); //high_APL_th
		
				WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 0 , 4 , 0 ); //gmv_num
				WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 6 , 7 , 1 ); //gmv_len
				WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 8 , 15 , 50 ); //set_occl_apl_th
				WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 16 , 23 , 50 ); //apl_th
				WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 24 , 31 , 2); //Gmv_diff_th
		
				WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 0 , 0 , 1 ); //rtgmvd_bypass
				WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 1 , 1 , 0 ); //gmv_bypass
				WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 2 , 3 , 1 ); //case_mark_mode
				WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 4 , 4 , 0 ); //result_apply
				WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 5 , 5 , 0 );//pred_occl_en
				WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 6 , 6 , 1 ); //occl_hor_en
				WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 7 , 7 , 1 ); //occl_ver_en
				WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 8 , 8 , 0 ); //debug mode
		
				WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 0 , 2 , 0 );	//res_num
				WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 3 , 4 , 0 );	//res_mode
				WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 5 , 9 , 2 );	//rim down
				WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 10 , 14 , 2 ); //rim up
				WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 15 , 19 , 0 );  //rim right
				WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 20 , 24 , 0 ); //rim left
		
				WriteRegister(KME_DEHALO4_RT2_MOD_1_reg , 0 , 0 , 0 ); //fix1 en
				WriteRegister(KME_DEHALO4_RT2_MOD_1_reg , 1 , 1 , 0 ); //fix1 en
				WriteRegister(KME_ME1_TOP10_APL_FILTER0_reg , 0 , 0 , 0 );
#endif
				//--solution settings:
			}
		
			u5_blend_holdfrm_preProcess_a[95] = 30;
			nIdentifiedNum_preProcess_a[95]=112;
		}
		else if(u5_blend_holdfrm_preProcess_a[95] >0){
			if(u32_pattern214_forbindden_en == 1){
				//++solution settings:
				//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 4, 7, 0x2);
#if 0//Merlin7 does not have those register
				WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 29 , 29 , 1 ); // new algo switch
				WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 28 , 28 , 1 ); //UL
				WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 27 , 27 , 1 ); //UR
				WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 26 , 26 , 0 ); //DL
				WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 25 , 25 , 1 ); //DR
				WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 24 , 24 , 0 ); //APL sel
				WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 18 , 23 , 20 );//hi_APL_num_th
				WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 12 , 17 , 20 );//low_APL_num_th
				WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 6 , 11 ,	20 );//near_Gmv_num_th
				WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 0 , 5 ,  20 ); //far_Gmv_num_th
		
				WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 0 , 7 ,  20 ); //low_Gmv_diff_th
				WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 8 , 15 ,	40 ); //high_Gmv_diff_th
				WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 16 , 23 ,  40 ); //low_APL_th
				WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 24 , 31 ,  50 ); //high_APL_th
		
				WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 0 , 4 , 0 ); //gmv_num
				WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 6 , 7 , 1 ); //gmv_len
				WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 8 , 15 , 50 ); //set_occl_apl_th
				WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 16 , 23 , 50 ); //apl_th
				WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 24 , 31 , 2); //Gmv_diff_th
		
				WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 0 , 0 , 1 ); //rtgmvd_bypass
				WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 1 , 1 , 0 ); //gmv_bypass
				WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 2 , 3 , 1 ); //case_mark_mode
				WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 4 , 4 , 0 ); //result_apply
				WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 5 , 5 , 0 );//pred_occl_en
				WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 6 , 6 , 1 ); //occl_hor_en
				WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 7 , 7 , 1 ); //occl_ver_en
				WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 8 , 8 , 0 ); //debug mode
		
				WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 0 , 2 , 0 );	//res_num
				WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 3 , 4 , 0 );	//res_mode
				WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 5 , 9 , 2 );	//rim down
				WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 10 , 14 , 2 ); //rim up
				WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 15 , 19 , 0 );  //rim right
				WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 20 , 24 , 0 ); //rim left
		
				WriteRegister(KME_DEHALO4_RT2_MOD_1_reg , 0 , 0 , 0 ); //fix1 en
				WriteRegister(KME_DEHALO4_RT2_MOD_1_reg , 1 , 1 , 0 ); //fix1 en
				WriteRegister(KME_ME1_TOP10_APL_FILTER0_reg , 0 , 0 , 0 );
#endif
				//--solution settings:
			}
		
			u5_blend_holdfrm_preProcess_a[95]--;
		}else if(nIdentifiedNum_preProcess_a[95] == 112 ){
			if(u32_pattern214_forbindden_en == 1){
				//++solution settings:
				//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 4, 7, 0x0);
#if 0//Merlin7 does not have those register
				WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 29 , 29 , 0 ); // new algo switch
				WriteRegister(KME_DEHALO4_RT2_MOD_1_reg , 0 , 0 , 0 ); //fix1 en
				WriteRegister(KME_DEHALO4_RT2_MOD_1_reg , 1 , 1 , 0 ); //fix1 en
				WriteRegister(KME_ME1_TOP10_APL_FILTER0_reg , 0 , 0 , 0 );
#endif
				//--solution settings:
			}
		
			nIdentifiedNum_preProcess_a[95] = 0;
		}

		//++ YE Test Sony #3 For32to55modeTest_60i_HDV
	#if Pro_SONY
			if(scalerVIP_Get_MEMCPatternFlag_Identification(1,2) == TRUE  )  //check scaler_vip number is 115
			{
				s_pContext->_output_wrt_comreg.For32to55modeTEST_60i_HDV_Flag=1;
				//WriteRegister(PQL0_00_ADDR_PQL0_06_ADDR_reg,0, 1, 0x1);//Merlin7 does not have those register
				WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_04_reg,8, 15, 0x14);
				WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_0C_reg,0, 7, 0xA5);
				WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,16, 16, 0x0);
				WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg,19, 19, 0x0);
				// merlin4
			/*
				WriteRegister(FRC_TOP__PQL_0__pql_dh_close_bypass_ADDR,0, 1, 0x1);
				WriteRegister(FRC_TOP__KME_ME2_VBUF_TOP__reg_kme_me2_vbuf_dm_pfv_req_intr_interval_ADDR,8, 15, 0x14);
				WriteRegister(FRC_TOP__KME_ME2_VBUF_TOP__reg_kme_me2_vbuf_dm_ppfv_req_intr_interval_ADDR,0, 7, 0xA5);
				WriteRegister(FRC_TOP__KME_ME2_VBUF_TOP__reg_kme_me2_vbuf_1st_zmv_en_ADDR,16, 16, 0x0);
				WriteRegister(FRC_TOP__KME_ME2_VBUF_TOP__reg_kme_me2_vbuf_2nd_zmv_en_ADDR,19, 19, 0x0);
			*/
		
				u5_blend_holdfrm_preProcess_a[96] = 120;
				nIdentifiedNum_preProcess_a[96]=115;
			}
			else if(u5_blend_holdfrm_preProcess_a[96] >0)
			{
				s_pContext->_output_wrt_comreg.For32to55modeTEST_60i_HDV_Flag=1;
				//WriteRegister(PQL0_00_ADDR_PQL0_06_ADDR_reg,0, 1, 0x1);//Merlin7 does not have those register
				WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_04_reg,8, 15, 0x14);
				WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_0C_reg,0, 7, 0xA5);
				WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,16, 16, 0x0);
				WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg,19, 19, 0x0);
				u5_blend_holdfrm_preProcess_a[96]--;
			}
			else if(nIdentifiedNum_preProcess_a[96] == 115 )
			{
				s_pContext->_output_wrt_comreg.For32to55modeTEST_60i_HDV_Flag=0;
				// ++ write back to original setting
				//WriteRegister(PQL0_00_ADDR_PQL0_06_ADDR_reg,0, 1, 0x0);//Merlin7 does not have those register
				WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_04_reg,8, 15, 0x8);
				WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_0C_reg,0, 7, 0x8);
				WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,16, 16, 0x1);
				WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg,19, 19, 0x1);
				nIdentifiedNum_preProcess_a[96] = 0;
				// -- write back to original setting
			}
	#endif
		//-- YE Test Sony #3 For32to55modeTest_60i_HDV
		
		//++  YE Test Sony MEMC Issue #5 Periodical_Pattern[007_Quantum of Solace]
	#if 1
			if(scalerVIP_Get_MEMCPatternFlag_Identification(1,5) == TRUE  )
			{
				s_pContext->_output_wrt_comreg.u1_GFB_force_Custom_true=1;
				u5_blend_holdfrm_preProcess_a[97] = 120;//u32_delay_test_en;//60;
				nIdentifiedNum_preProcess_a[97]=122;
			}
			else if(u5_blend_holdfrm_preProcess_a[97] >0)
			{
				s_pContext->_output_wrt_comreg.u1_GFB_force_Custom_true=1;
				u5_blend_holdfrm_preProcess_a[97]--;
			}
			else if(nIdentifiedNum_preProcess_a[97] == 122 )
			{
				s_pContext->_output_wrt_comreg.u1_GFB_force_Custom_true=0;
				pOutput->u8_FB_lvl=0;
				s_pContext->_output_wrt_comreg.u1_GFB_modify_true=0;
				nIdentifiedNum_preProcess_a[97] = 0;
			}
	#endif
		//--  YE Test Sony MEMC Issue #5 Periodical_Pattern[007_Quantum of Solace]
		//++YE Test Sony new issue ultraviolet_1 2:31 earth
	#if Pro_SONY
			if(scalerVIP_Get_MEMCPatternFlag_Identification(1,7) == TRUE )
			{
				s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
				s_pContext->_output_wrt_comreg.u1_KME_gmvd_cost_type=1;
		
				u5_blend_holdfrm_preProcess_a[99] = 60;
				nIdentifiedNum_preProcess_a[99]=125;
			}
			else if(u5_blend_holdfrm_preProcess_a[99] >0)
			{
				s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
				s_pContext->_output_wrt_comreg.u1_KME_gmvd_cost_type=1;
		
				u5_blend_holdfrm_preProcess_a[99]--;
			}
			else if(nIdentifiedNum_preProcess_a[99] == 125 )
			{
				s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
				s_pContext->_output_wrt_comreg.u1_KME_gmvd_cost_type=0;
				nIdentifiedNum_preProcess_a[99] = 0;
			}
	#endif
		//--YE Test Sony new issue ultraviolet_1 2:31 earth
		
		//++YE Test Sony Fallback_(i-Robert_03)
	#if Pro_SONY
			if((scalerVIP_Get_MEMCPatternFlag_Identification(1,10) == TRUE )&&(BOT_ratio==0))
			{
				s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type=2;
		
				u5_blend_holdfrm_preProcess_a[100] = 120;
				nIdentifiedNum_preProcess_a[100]=128;
			}
			else if(u5_blend_holdfrm_preProcess_a[100] >0)
			{
		
				s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type=2;
				u5_blend_holdfrm_preProcess_a[100]--;
			}
			else if(nIdentifiedNum_preProcess_a[100] == 128 )
			{
		
				s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type=0;
		
				nIdentifiedNum_preProcess_a[100] = 0;
			}
	#endif
		//--YE Test Sony Fallback_(i-Robert_03)
		
		
		 //++ YE Test Sony Bluray OSD + Train 3:2
#if Pro_SONY
		
			if(scalerVIP_Get_MEMCPatternFlag_Identification(1,11) == TRUE  )
			{
		
				s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
		
				u5_blend_holdfrm_preProcess_a[101] = 180;
				nIdentifiedNum_preProcess_a[101]=129;
			}
			else if(u5_blend_holdfrm_preProcess_a[101] >0)
			{
				s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
		
				u5_blend_holdfrm_preProcess_a[101]--;
			}
			else if(nIdentifiedNum_preProcess_a[101] == 129 )
			{
				s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=0;
				nIdentifiedNum_preProcess_a[101] = 0;
			}
#endif
		
		//-- YE Test Sony Bluray OSD + Train 3:2
		
		//++ YE Test Periodical_Pattern_[007_Casino_Royale] 20200904
	#if Pro_SONY
			if(scalerVIP_Get_MEMCPatternFlag_Identification(1,14) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
			//if(1)  //not working 0831 testing
			{
				s_pContext->_output_dh_close.u1_closeDh_act =1;
				s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
				u5_blend_holdfrm_preProcess_a[87] = 16;
				nIdentifiedNum_preProcess_a[87]=132;
			}
			else if(u5_blend_holdfrm_preProcess_a[87] >0)
			{
				s_pContext->_output_dh_close.u1_closeDh_act =1;
				s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
				u5_blend_holdfrm_preProcess_a[87]--;
			}
			else if(nIdentifiedNum_preProcess_a[87] == 132)
			{
				s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
				nIdentifiedNum_preProcess_a[87] = 0;
			}
	#endif
		
		 //++ YE Test Sony ultraviolet_1 SCREEN GEMS
#if Pro_SONY
		
			if(scalerVIP_Get_MEMCPatternFlag_Identification(1,16) == TRUE  )
			{
		
				s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
		
				u5_blend_holdfrm_preProcess_a[102] = 180;
				nIdentifiedNum_preProcess_a[102]=134;
			}
			else if(u5_blend_holdfrm_preProcess_a[102] >0)
			{
				s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
		
				u5_blend_holdfrm_preProcess_a[102]--;
			}
			else if(nIdentifiedNum_preProcess_a[102] == 134 )
			{
				s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=0;
				nIdentifiedNum_preProcess_a[102] = 0;
			}
#endif
		
		 //-- YE Test Sony ultraviolet_1 SCREEN GEMS


#if 1
		//===debug show
		for(u8_Index =0;u8_Index<255;u8_Index++)
		{
			if(nIdentifiedNum_preProcess_a[u8_Index] !=0){
				if(u32_ID_Log_en==1)
				rtd_pr_memc_emerg("===== [IPRpre_002] nIdentifiedNum_preProcess_a>>[%d][%d]\n", u8_Index,nIdentifiedNum_preProcess_a[u8_Index]);
	
				pOutput->u1_IP_preProcess_true=1;
	
			}
		}
#endif

}

//extern unsigned char logo_check_K24014_flag;
VOID Identification_Pattern_preProcess_TV006(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	_PQLCONTEXT *s_pContext = GetPQLContext_m();
	static unsigned short  u5_blend_holdfrm_preProcess_a[255] = {0};
	static unsigned char  nIdentifiedNum_preProcess_a[255] = {0};
	unsigned char  u8_Index;
	//unsigned int	u32_RB_val;
	//unsigned int	u32_pattern214_forbindden_en;
	//unsigned char  u1_still_frame=s_pContext->_output_frc_sceneAnalysis.u1_still_frame;
	//short  u11_gmv_mvx = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
	//short  u10_gmv_mvy = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;
	//unsigned int	u25_me_aDTL = s_pContext->_output_read_comreg.u25_me_aDTL_rb;
	//unsigned short  u32_gmv_unconf = s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb;
	//UBYTE  u8_cadence_Id = s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL];
	PQL_INPUT_FRAME_RATE in_fmRate = s_pContext->_external_data._input_frameRate;
	PQL_OUTPUT_FRAME_RATE out_fmRate = s_pContext->_external_data._output_frameRate;
	//unsigned char nblend_y_alpha_Target=0xa0;
	//unsigned int u32_RB_kmc_blend_y_alpha;
	//unsigned int u25_rgnSad_rb12 = s_pContext->_output_read_comreg.u25_me_rSAD_rb[12];
	//unsigned int u25_rgnSad_rb23 = s_pContext->_output_read_comreg.u25_me_rSAD_rb[23];
	unsigned int u32_ID_Log_en=0;//,ES_Test_Value=0;
	unsigned char  u8_Mixmode_flag = s_pContext->_output_filmDetectctrl.u1_mixMode;
	unsigned char  u8_cadence_Id = s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL];
	ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 24,  24, &u32_ID_Log_en);//YE Test debug dummy register
	//ReadRegister(FRC_TOP__IPPRE__reg_kmc_blend_y_alpha_ADDR, 4, 11, &u32_RB_kmc_blend_y_alpha);
	//ReadRegister(FRC_TOP__PQL_0__pql_patch_dummy_e8_ADDR, 0, 31, &u32_RB_val);   //  bit0

	//reset
	pOutput->u1_IP_preProcess_true=0;

	if(pParam->u1_Identification_Pattern_en== 0)
		return;

	//01_PerfumeLand_0915_M_4K_V1_01.mp4 8s  KTASKWBS-23028
	if( scalerVIP_Get_MEMCPatternFlag_Identification(2,0) == TRUE && in_fmRate == _PQL_IN_60HZ && out_fmRate == _PQL_OUT_120HZ && u8_Mixmode_flag == 0 && u8_cadence_Id == _CAD_VIDEO &&
		s_pContext->_output_read_comreg.u20_me_rDTL_rb[10] > 60000 && s_pContext->_output_read_comreg.u20_me_rDTL_rb[11] > 60000 && s_pContext->_output_read_comreg.u20_me_rDTL_rb[12] > 60000 && s_pContext->_output_read_comreg.u20_me_rDTL_rb[13] > 60000 && 
		s_pContext->_output_read_comreg.u20_me_rDTL_rb[18] > 60000 && s_pContext->_output_read_comreg.u20_me_rDTL_rb[19] > 60000 && s_pContext->_output_read_comreg.u20_me_rDTL_rb[20] > 60000 && s_pContext->_output_read_comreg.u20_me_rDTL_rb[21] > 60000)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true = 1;
		u5_blend_holdfrm_preProcess_a[1] = 60;
		nIdentifiedNum_preProcess_a[1]=1;
	}
	else if(u5_blend_holdfrm_preProcess_a[1] > 0)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true = 1;
		u5_blend_holdfrm_preProcess_a[1]--;
	}
	else if(nIdentifiedNum_preProcess_a[1] == 1 )
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true = 0;
		nIdentifiedNum_preProcess_a[1] = 0;
	}
#if 0
	//PQ_Issue_SAN_ANDREAS_01_repeat.mp4 KTASKWBS-22870
	if(scalerVIP_Get_MEMCPatternFlag_Identification(2,1) == TRUE && s_pContext->_output_me_sceneAnalysis.u1_SAN_ANDREAS_DRP_detect_true>0)		
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true = 4;
		u5_blend_holdfrm_preProcess_a[2] = 180;
		nIdentifiedNum_preProcess_a[2]=2;
	}
	else if(u5_blend_holdfrm_preProcess_a[2] > 0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true = 4;
		u5_blend_holdfrm_preProcess_a[2]--;
	}
	else if(nIdentifiedNum_preProcess_a[2] == 2 )
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true = 0;
		nIdentifiedNum_preProcess_a[2] = 0;
	}
#endif
#if 1 //K24014_v3
	if(scalerVIP_Get_MEMCPatternFlag_Identification(2,2) == TRUE && 
		(s_pContext->_output_FRC_LgDet.u1_logo_PurePanning_status == 2 || s_pContext->_output_FRC_LgDet.u1_logo_PurePanning_status == 7) )		
	{
		s_pContext->_output_wrt_comreg.u1_SquidGame_subTitle_flag = 1;
		u5_blend_holdfrm_preProcess_a[3] = 180;
		nIdentifiedNum_preProcess_a[3]=3;
	}
	else if(u5_blend_holdfrm_preProcess_a[3] > 0)
	{		
		s_pContext->_output_wrt_comreg.u1_SquidGame_subTitle_flag = 1;
		u5_blend_holdfrm_preProcess_a[3]--;
	}
	else if(nIdentifiedNum_preProcess_a[3] == 3 )
	{
		s_pContext->_output_wrt_comreg.u1_SquidGame_subTitle_flag = 0;
		nIdentifiedNum_preProcess_a[3] = 0;
	}
#endif

#if 1
	//===debug show
	for(u8_Index =0;u8_Index<255;u8_Index++)
	{
		if(nIdentifiedNum_preProcess_a[u8_Index] !=0){
			if(u32_ID_Log_en==1)
			rtd_pr_memc_emerg("===== [IPRpre_006] nIdentifiedNum_preProcess_a>>[%d][%d]\n", u8_Index,nIdentifiedNum_preProcess_a[u8_Index]);

			pOutput->u1_IP_preProcess_true=1;

		}
	}
#endif
}

VOID Identification_Pattern_preProcess_TV010(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{

}

VOID Identification_Pattern_preProcess_TV011(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	_PQLCONTEXT *s_pContext = GetPQLContext_m();
	//	static unsigned short  u5_blend_holdfrm_preProcess_a[255] = {0};
	static unsigned char  nIdentifiedNum_preProcess_a[255] = {0};
	unsigned char  u8_Index;
	//unsigned int  u32_RB_val;
	//unsigned int  u32_pattern214_forbindden_en;
	//unsigned char  u1_still_frame=s_pContext->_output_frc_sceneAnalysis.u1_still_frame;
	//short  u11_gmv_mvx = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
	//short  u10_gmv_mvy = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;
	//unsigned int  u25_me_aDTL = s_pContext->_output_read_comreg.u25_me_aDTL_rb;
	//unsigned short  u32_gmv_unconf = s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb;
#if 1//Pro_TCL
	//UBYTE  u8_cadence_Id = s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL];
	//PQL_INPUT_FRAME_RATE in_fmRate = s_pContext->_external_data._input_frameRate;
	//unsigned char nblend_y_alpha_Target=0xa0;
	//unsigned int u32_RB_kmc_blend_y_alpha;
	//unsigned int u25_rgnSad_rb12 = s_pContext->_output_read_comreg.u25_me_rSAD_rb[12];
	//unsigned int u25_rgnSad_rb23 = s_pContext->_output_read_comreg.u25_me_rSAD_rb[23];
	unsigned int u32_ID_Log_en=0;//,ES_Test_Value=0;
	ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 24,  24, &u32_ID_Log_en);//YE Test debug dummy register
	//ReadRegister(FRC_TOP__IPPRE__reg_kmc_blend_y_alpha_ADDR, 4, 11, &u32_RB_kmc_blend_y_alpha);
#endif
	//ReadRegister(FRC_TOP__PQL_0__pql_patch_dummy_e8_ADDR, 0, 31, &u32_RB_val);   //  bit0
	
	//reset
	pOutput->u1_IP_preProcess_true=0;

	if(pParam->u1_Identification_Pattern_en== 0)
		return;

	if(s_pContext->_output_me_vst_ctrl.u1_detect_blackBG_VST_trure)
	{
		pOutput->u1_casino_RP_detect_true =0;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
	}

#if 0
	//--#04_Montage_P 5s
	if( scalerVIP_Get_MEMCPatternFlag_Identification(3,2) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true = 2;
		u5_blend_holdfrm_preProcess_a[1] = 40;
		nIdentifiedNum_preProcess_a[1]=1;
	}
	else if(u5_blend_holdfrm_preProcess_a[1] > 0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true = 2;
		u5_blend_holdfrm_preProcess_a[1]--;
	}
	else if(nIdentifiedNum_preProcess_a[1] == 1 )
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true = 0;
		nIdentifiedNum_preProcess_a[1] = 0;
	}
#endif



	//===debug show
	for(u8_Index =0;u8_Index<255;u8_Index++)
	{
		if(nIdentifiedNum_preProcess_a[u8_Index] !=0){
			if(u32_ID_Log_en==1)
			rtd_pr_memc_emerg("===== [IPRpre_011] nIdentifiedNum_preProcess_a>>[%d][%d]\n", u8_Index,nIdentifiedNum_preProcess_a[u8_Index]);

			pOutput->u1_IP_preProcess_true=1;

		}
	}

}
VOID Identification_Pattern_preProcess_TV030(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
#if 0
	_PQLCONTEXT *s_pContext = GetPQLContext_m();
	static unsigned short  u5_blend_holdfrm_preProcess_a[255] = {0};
	static unsigned char  nIdentifiedNum_preProcess_a[255] = {0};
	unsigned char  u8_Index;
	unsigned int u32_ID_Log_en=0;
	ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 24,  24, &u32_ID_Log_en);//YE Test debug dummy register

	//reset
	pOutput->u1_IP_preProcess_true=0;

	if(pParam->u1_Identification_Pattern_en== 0)
	{
		return;
	}

	
	for(u8_Index =0;u8_Index<255;u8_Index++)
	{
		if(nIdentifiedNum_preProcess_a[u8_Index] !=0){
			if(u32_ID_Log_en==1)
			rtd_pr_memc_emerg("===== [IPRpre_030] nIdentifiedNum_preProcess_a>>[%d][%d]\n", u8_Index,nIdentifiedNum_preProcess_a[u8_Index]);

			pOutput->u1_IP_preProcess_true=1;

		}
	}
#endif
}
VOID Identification_Pattern_preProcess_TV043(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	_PQLCONTEXT *s_pContext = GetPQLContext_m();
	static unsigned short  u5_blend_holdfrm_preProcess_a[255] = {0};
	static unsigned char  nIdentifiedNum_preProcess_a[255] = {0};
	unsigned char  u8_Index;
	//unsigned int  u32_RB_val;
	//unsigned int  u32_pattern214_forbindden_en;
	unsigned char  u1_still_frame=s_pContext->_output_frc_sceneAnalysis.u1_still_frame;
	short  u11_gmv_mvx = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
	short  u10_gmv_mvy = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;
	//unsigned int  u25_me_aDTL = s_pContext->_output_read_comreg.u25_me_aDTL_rb;
	//unsigned short  u32_gmv_unconf = s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb;
	#if 1//Pro_TCL
	//UBYTE  u8_cadence_Id = s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL];
	//PQL_INPUT_FRAME_RATE in_fmRate = s_pContext->_external_data._input_frameRate;
	//unsigned char nblend_y_alpha_Target=0xa0;
	//unsigned int u32_RB_kmc_blend_y_alpha;
	//unsigned int u25_rgnSad_rb12 = s_pContext->_output_read_comreg.u25_me_rSAD_rb[12];
	//unsigned int u25_rgnSad_rb23 = s_pContext->_output_read_comreg.u25_me_rSAD_rb[23];
	unsigned int u32_ID_Log_en=0;//,ES_Test_Value=0;
	ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 24,  24, &u32_ID_Log_en);//YE Test debug dummy register
	//ReadRegister(FRC_TOP__IPPRE__reg_kmc_blend_y_alpha_ADDR, 4, 11, &u32_RB_kmc_blend_y_alpha);
	#endif
	//ReadRegister(FRC_TOP__PQL_0__pql_patch_dummy_e8_ADDR, 0, 31, &u32_RB_val);   //  bit0
	
	//reset
	pOutput->u1_IP_preProcess_true=0;

	if(pParam->u1_Identification_Pattern_en== 0)
		return;

	if(s_pContext->_output_me_vst_ctrl.u1_detect_blackBG_VST_trure)
	{
		pOutput->u1_casino_RP_detect_true =0;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
	}


	/*avsync_cont-4000*/	
	if((_ABS_(u10_gmv_mvy) < 1) && (_ABS_(u11_gmv_mvx) < 1) && (scalerVIP_Get_MEMCPatternFlag_Identification(5,1) == TRUE))
	{
		//s_pContext->_output_wrt_comreg.u1_GFB_specialScene_true=1;		
		
		if(pOutput->u8_FB_lvl<0xfe)
		{
			pOutput->u8_FB_lvl += 2;
			s_pContext->_output_wrt_comreg.u1_GFB_modify_true=1;
		}
		else if( s_pContext->_output_fblevelctrl.u8_FBLevel >= 0xfe)
		{
			s_pContext->_output_wrt_comreg.u1_GFB_modify_true=0;
		}
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=1;
		u5_blend_holdfrm_preProcess_a[1] = 240;
		nIdentifiedNum_preProcess_a[1]=1;
	}
	else if(u5_blend_holdfrm_preProcess_a[1] >0)
	{
		if(pOutput->u8_FB_lvl<0xfe)
		{
			pOutput->u8_FB_lvl += 2;
			s_pContext->_output_wrt_comreg.u1_GFB_modify_true=1;
		}
		else if( s_pContext->_output_fblevelctrl.u8_FBLevel >= 0xfe)
		{
			s_pContext->_output_wrt_comreg.u1_GFB_modify_true=0;
		}
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=1;
		u5_blend_holdfrm_preProcess_a[1]--;
	}
	else if(nIdentifiedNum_preProcess_a[1] == 1 && pOutput->u8_FB_lvl>0)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_modify_true=1;
		pOutput->u8_FB_lvl --;
	}
	else if(nIdentifiedNum_preProcess_a[1] == 1)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=0;
		s_pContext->_output_wrt_comreg.u1_GFB_modify_true=0;
		nIdentifiedNum_preProcess_a[1] = 0;
	}

	{
		static unsigned int kmc_blend_y_alpha = 255, kmc_blend_uv_alpha = 32;
		/*06.Sharks.mp4*/	
		/*H_Move_fast(1080_24p)*/
		if(scalerVIP_Get_MEMCPatternFlag_Identification(5,2) == TRUE)
		{
			if(nIdentifiedNum_preProcess_a[2] == 0)
			{
				ReadRegister(IPPRE_IPPRE_04_reg, 4, 11, &kmc_blend_y_alpha);
				ReadRegister(IPPRE_IPPRE_04_reg, 16, 23, &kmc_blend_uv_alpha);
			}
			s_pContext->_output_dh_close.u1_closeDh_act =1;
			s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
			WriteRegister(MC_MC_28_reg, 14, 14, 1);
			WriteRegister(MC_MC_28_reg, 15, 22, 0);
			WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, 100);
			WriteRegister(IPPRE_IPPRE_04_reg, 16, 23, 255);
			u5_blend_holdfrm_preProcess_a[2] = 360;
			nIdentifiedNum_preProcess_a[2]=2;
		}
		else if(u5_blend_holdfrm_preProcess_a[2] >0)
		{
			s_pContext->_output_dh_close.u1_closeDh_act =1;
			s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
			WriteRegister(MC_MC_28_reg, 14, 14, 1);
			WriteRegister(MC_MC_28_reg, 15, 22, 0);
			WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, 100);
			WriteRegister(IPPRE_IPPRE_04_reg, 16, 23, 255);
			u5_blend_holdfrm_preProcess_a[2]--;
		}
		else if(nIdentifiedNum_preProcess_a[2] == 2)
		{
			s_pContext->_output_dh_close.u1_closeDh_act =0;
			s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=0;
			WriteRegister(MC_MC_28_reg, 14, 14, 0);
			WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, kmc_blend_y_alpha);
			WriteRegister(IPPRE_IPPRE_04_reg, 16, 23, kmc_blend_uv_alpha);
			nIdentifiedNum_preProcess_a[2] = 0;
		}
	}

	/*7.12.02.movZonePlate_AVC1080P60.mp4*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,3) == TRUE)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=1;
		u5_blend_holdfrm_preProcess_a[3] = 16;
		nIdentifiedNum_preProcess_a[3]=3;
	}
	else if(u5_blend_holdfrm_preProcess_a[3] >0)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=1;
		u5_blend_holdfrm_preProcess_a[3]--;
	}
	else if(nIdentifiedNum_preProcess_a[3] == 3)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=0;
		nIdentifiedNum_preProcess_a[3] = 0;
	}

	/*Transformer.3.fly_men_short*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,5) == TRUE)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=1;
		u5_blend_holdfrm_preProcess_a[4] = 32;
		nIdentifiedNum_preProcess_a[4]=4;
	}
	else if(u5_blend_holdfrm_preProcess_a[4] >0)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=1;
		u5_blend_holdfrm_preProcess_a[4]--;
	}
	else if(nIdentifiedNum_preProcess_a[4] == 4)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=0;
		nIdentifiedNum_preProcess_a[4] = 0;
	}

	/*Rotterdam_1080P24_short*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,6) == TRUE) //histogram from h5x
	{
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=3;
		u5_blend_holdfrm_preProcess_a[5] = 8;
		nIdentifiedNum_preProcess_a[5]=5;
	}
	else if(u5_blend_holdfrm_preProcess_a[5] >0)
	{
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=3;
		u5_blend_holdfrm_preProcess_a[5]--;
	}
	else if(nIdentifiedNum_preProcess_a[5] == 5)
	{
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=0;
		s_pContext->_output_dh_close.u1_closeDh_act =0;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=0;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=0;
		nIdentifiedNum_preProcess_a[5] = 0;
	}
	
	/*fusion_PW_120Hz_l_Robot_(SD_NTSC)_02.mpg*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,7) == TRUE) //histogram from h5x
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;
		u5_blend_holdfrm_preProcess_a[6] = 60;
		nIdentifiedNum_preProcess_a[6]=6;
	}
	else if(u5_blend_holdfrm_preProcess_a[6] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;
		u5_blend_holdfrm_preProcess_a[6]--;
	}
	else if(nIdentifiedNum_preProcess_a[6] == 6)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =0;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=0;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=0;
		nIdentifiedNum_preProcess_a[6] = 0;
	}

	/*taran YouTube 4K bitrate test pattern v3 [2160p].webm*/
	if(u11_gmv_mvx == 0 && u10_gmv_mvy == 0 && (scalerVIP_Get_MEMCPatternFlag_Identification(5,10) == TRUE)) //histogram from h5x
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[7] = 240;
		nIdentifiedNum_preProcess_a[7]=7;
	}
	else if(u5_blend_holdfrm_preProcess_a[7] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[7]--;
	}
	else if(nIdentifiedNum_preProcess_a[7] == 7)
	{
		nIdentifiedNum_preProcess_a[7] = 0;
	}

	/*MI_III.c03.short.mp4*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,11) == TRUE) //histogram from h5x
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[8] = 100;
		nIdentifiedNum_preProcess_a[8]=8;
	}
	else if(u5_blend_holdfrm_preProcess_a[8] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[8]--;
	}
	else if(nIdentifiedNum_preProcess_a[8] == 8)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =0;
		nIdentifiedNum_preProcess_a[8] = 0;
	}

	/*Transformers.4khdr_19.mp4*/
	if(s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[12] > 5 && (scalerVIP_Get_MEMCPatternFlag_Identification(5,12) == TRUE)) //histogram from h5x
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=1;
		u5_blend_holdfrm_preProcess_a[9] = 120;
		nIdentifiedNum_preProcess_a[9]=9;
	}
	else if(u5_blend_holdfrm_preProcess_a[9] >0)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=1;
		u5_blend_holdfrm_preProcess_a[9]--;
	}
	else if(nIdentifiedNum_preProcess_a[9] == 9)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=0;
		nIdentifiedNum_preProcess_a[9] = 0;
	}

	/*Transformers.4khdr_1.mp4*/
	if(s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb > 10 && (scalerVIP_Get_MEMCPatternFlag_Identification(5,13) == TRUE)) //histogram from h5x
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[10] = 30;
		nIdentifiedNum_preProcess_a[10]=10;
	}
	else if(u5_blend_holdfrm_preProcess_a[10] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[10]--;
	}
	else if(nIdentifiedNum_preProcess_a[10] == 10)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =0;
		nIdentifiedNum_preProcess_a[10] = 0;
	}

	
	/*Martian.4KHDR_4_contrast.mp4*/
	if((s_pContext->_output_read_comreg.u20_me_rAPLp_rb[17] > 60000 && s_pContext->_output_read_comreg.u20_me_rAPLp_rb[2] > 20000 && (s_pContext->_output_read_comreg.u20_me_rAPLp_rb[4] - s_pContext->_output_read_comreg.u20_me_rAPLi_rb[4] > 400) &&
		(scalerVIP_Get_MEMCPatternFlag_Identification(5,15) == TRUE)) //histogram from h5x
		|| 
		/*Blade Runner.4khdr_06.mp4*/
		((scalerVIP_Get_MEMCPatternFlag_Identification(5,24) == TRUE))
		||
		/*CAPTAIN_AMERICA__THE_FIRST_AVENGER_4KHDR15.mp4*/
		(s_pContext->_output_read_comreg.u20_me_rAPLp_rb[17] > 140000 && s_pContext->_output_read_comreg.u20_me_rAPLp_rb[18] > 150000 &&
		(scalerVIP_Get_MEMCPatternFlag_Identification(5,25) == TRUE))) //histogram from h5x
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[11] = 96;
		nIdentifiedNum_preProcess_a[11]=11;
	}
	else if(u5_blend_holdfrm_preProcess_a[11] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[11]--;
	}
	else if(nIdentifiedNum_preProcess_a[11] == 11)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =0;
		nIdentifiedNum_preProcess_a[11] = 0;
	}

	/*TobyFree.com - 8K UHD Test Pattern H.264 MP4 [4320p].mp4*/
	if((_ABS_(u11_gmv_mvx) <1  && _ABS_(u10_gmv_mvy) <1) /*&& ( u1_still_frame == 1)*/ && (scalerVIP_Get_MEMCPatternFlag_Identification(5,16) == TRUE)) //histogram from h5x
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=1;
		u5_blend_holdfrm_preProcess_a[12] = 30;
		nIdentifiedNum_preProcess_a[12]=12;
	}
	else if(u5_blend_holdfrm_preProcess_a[12] >0)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=1;
		u5_blend_holdfrm_preProcess_a[12]--;
	}
	else if(nIdentifiedNum_preProcess_a[12] == 12)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=0;
		nIdentifiedNum_preProcess_a[12] = 0;
	}

	/*MI3_17_M2TS*/
	if(s_pContext->_output_read_comreg.u20_me_rAPLp_rb[16] > 160000 && 
		(scalerVIP_Get_MEMCPatternFlag_Identification(5,17) == TRUE)) //histogram from h5x
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=1;
		u5_blend_holdfrm_preProcess_a[13] = 24;
		nIdentifiedNum_preProcess_a[13]=13;
	}
	else if(u5_blend_holdfrm_preProcess_a[13] >0)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=1;
		u5_blend_holdfrm_preProcess_a[13]--;
	}
	else if(nIdentifiedNum_preProcess_a[13] == 13)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=0;
		nIdentifiedNum_preProcess_a[13] = 0;
	}

	//pink.ts: fense
	if((_ABS_(u11_gmv_mvx) <1  && _ABS_(u10_gmv_mvy) <1) && 
		(scalerVIP_Get_MEMCPatternFlag_Identification(5,20) == TRUE))  //histogram from h5x
	{
		//s_pContext->_output_wrt_comreg.u1_GFB_Special_apply=1;
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=1;
		u5_blend_holdfrm_preProcess_a[14] = 8;
		nIdentifiedNum_preProcess_a[14]=14;
	}
	else if(u5_blend_holdfrm_preProcess_a[14] >0)
	{
		//s_pContext->_output_wrt_comreg.u1_GFB_Special_apply=1;
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=1;
		u5_blend_holdfrm_preProcess_a[14]--;
	}
	else if(nIdentifiedNum_preProcess_a[14] == 14)
	{
		//s_pContext->_output_wrt_comreg.u1_GFB_Special_apply=0;
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=0;
		nIdentifiedNum_preProcess_a[14] = 0;
	}

//sync from hisense
	/*Guanyin_24p_07_Rotation*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,37) == TRUE) //histogram from h5x
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[15] = 16;
		//WriteRegister(KIWI_REG(FRC_TOP__MC__reg_mc_fb_lvl_force_en), 1);
		//WriteRegister(KIWI_REG(FRC_TOP__MC__reg_mc_fb_lvl_force_value), 192);
		nIdentifiedNum_preProcess_a[15]=15;
	}
	else if(u5_blend_holdfrm_preProcess_a[15] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		//WriteRegister(KIWI_REG(FRC_TOP__MC__reg_mc_fb_lvl_force_en), 1);
		//WriteRegister(KIWI_REG(FRC_TOP__MC__reg_mc_fb_lvl_force_value), 192);
		u5_blend_holdfrm_preProcess_a[15]--;
	}
	else if(nIdentifiedNum_preProcess_a[15] == 15 )
	{
		s_pContext->_output_dh_close.u1_closeDh_act =0;
		//WriteRegister(KIWI_REG(FRC_TOP__MC__reg_mc_fb_lvl_force_en), 0);
		//WriteRegister(KIWI_REG(FRC_TOP__MC__reg_mc_fb_lvl_force_value), 0);
		nIdentifiedNum_preProcess_a[15] = 0;
	}
	/*Satelite_1920x1080_24_362.avi*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,38) == TRUE) //histogram from h5x
	{
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		u5_blend_holdfrm_preProcess_a[16] = 8;
		nIdentifiedNum_preProcess_a[16]=16;
	}
	else if(u5_blend_holdfrm_preProcess_a[16] >0)
	{
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		u5_blend_holdfrm_preProcess_a[16]--;
	}
	else if(nIdentifiedNum_preProcess_a[16] == 16 )
	{
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=0;
		nIdentifiedNum_preProcess_a[16] = 0;
	}
	//Montage_M_ROT-720p50.mpg
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,39) == TRUE) //histogram from h5x
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[17] = 16;
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 0);
		nIdentifiedNum_preProcess_a[17]=17;
	}
	else if(u5_blend_holdfrm_preProcess_a[17] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 0);
		u5_blend_holdfrm_preProcess_a[17]--;
	}
	else if(nIdentifiedNum_preProcess_a[17] == 17 )
	{
		s_pContext->_output_dh_close.u1_closeDh_act =0;
		WriteRegister(MC_MC_28_reg, 14, 14, 0);
		nIdentifiedNum_preProcess_a[17] = 0;
	}
#if 1
	/*monosco4K_Hscroll_10pix_50Hz_long_50p-3840*2160*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,41) == TRUE) //histogram from h5x
	{
		rtd_clearbits(0xb802b500, _BIT0);//  sharpness peaking
		rtd_clearbits(0xb802b950, _BIT0);//  sharpness dlti
		rtd_clearbits(0xb8023c00, _BIT0);//  Multiband peaking
		rtd_setbits(0xb8023c30, _BIT0);//  Multiband peaking DB apply
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true = 4;
		u5_blend_holdfrm_preProcess_a[18] = 120;
		nIdentifiedNum_preProcess_a[18]=18;
	}
	else if(u5_blend_holdfrm_preProcess_a[18] >0)
	{
		rtd_clearbits(0xb802b500, _BIT0);//  sharpness peaking
		rtd_clearbits(0xb802b950, _BIT0);//  sharpness dlti
		rtd_clearbits(0xb8023c00, _BIT0);//  Multiband peaking
		rtd_setbits(0xb8023c30, _BIT0);//  Multiband peaking DB apply
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true = 4;
		u5_blend_holdfrm_preProcess_a[18]--;
	}
	else if(nIdentifiedNum_preProcess_a[18] == 18 )
	{
		rtd_setbits(0xb802b500, _BIT0);//  sharpness peaking
		rtd_setbits(0xb802b950, _BIT0);//  sharpness dlti
		rtd_setbits(0xb8023c00, _BIT0);//  Multiband peaking
		rtd_setbits(0xb8023c30, _BIT0);//  Multiband peaking DB apply
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
		nIdentifiedNum_preProcess_a[18] = 0;
	}
#endif
	/*4K60FPS?��?*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,42) == TRUE) //histogram from h5x
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 128);
		u5_blend_holdfrm_preProcess_a[19] = 16;
		nIdentifiedNum_preProcess_a[19]=19;
	}
	else if(u5_blend_holdfrm_preProcess_a[19] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 128);
		u5_blend_holdfrm_preProcess_a[19]--;
	}
	else if(nIdentifiedNum_preProcess_a[19] == 19 )
	{
		s_pContext->_output_dh_close.u1_closeDh_act =0;
		WriteRegister(MC_MC_28_reg, 14, 14, 0);
		WriteRegister(MC_MC_28_reg, 15, 22, 0);
		nIdentifiedNum_preProcess_a[19] = 0;
	}
	/*Midway.4khdr_13_Propeller*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,43) == TRUE) //histogram from h5x
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[20] = 32;
		nIdentifiedNum_preProcess_a[20]=20;
	}else if(u5_blend_holdfrm_preProcess_a[20] >0){
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[20]--;
	}else if(nIdentifiedNum_preProcess_a[20] == 20 ){
		s_pContext->_output_dh_close.u1_closeDh_act =0;
		nIdentifiedNum_preProcess_a[20] = 0;
	}
//sync from hisense end
	//The Art of Flight_00_34.mp4
	if(s_pContext->_output_me_sceneAnalysis.u2_panning_flag && 
		(scalerVIP_Get_MEMCPatternFlag_Identification(5,22) == TRUE)) //histogram from h5x
	{
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		u5_blend_holdfrm_preProcess_a[21] = 32;
		nIdentifiedNum_preProcess_a[21]=21;
	}
	else if(u5_blend_holdfrm_preProcess_a[21] >0)
	{
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		u5_blend_holdfrm_preProcess_a[21]--;
	}
	else if(nIdentifiedNum_preProcess_a[21] == 21)
	{
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		nIdentifiedNum_preProcess_a[21] = 0;
	}
	/*The Art of Flight disk: 46"26 antenna broken when scene pans*/
	if(u11_gmv_mvx > 0 && u11_gmv_mvx < 10 && u10_gmv_mvy == 0 && (scalerVIP_Get_MEMCPatternFlag_Identification(5,23) == TRUE)) //histogram from h5x
	{
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_D4_reg, 16, 16, 0);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_DC_reg, 0, 0, 0);
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 0);
		u5_blend_holdfrm_preProcess_a[22] = 16;
		nIdentifiedNum_preProcess_a[22]=22;
	}
	else if(u5_blend_holdfrm_preProcess_a[22] >0)
	{
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_D4_reg, 16, 16, 0);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_DC_reg, 0, 0, 0);
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 0);
		u5_blend_holdfrm_preProcess_a[22]--;
	}
	else if(nIdentifiedNum_preProcess_a[22] == 22 )
	{
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_D4_reg, 16, 16, 1);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_DC_reg, 0, 0, 1);
		WriteRegister(MC_MC_28_reg, 14, 14, 0);
		nIdentifiedNum_preProcess_a[22] = 0;
	}
	/*HQV_text_rot35_MPG1080P24.m2ts*/
	{		
		int rmv_y_9 = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[9];
		int rmv_y_12 = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[12];
		if(rmv_y_9 > 0 && rmv_y_12 < 0 && nIdentifiedNum_preProcess_a[50] !=60 && (scalerVIP_Get_MEMCPatternFlag_Identification(5,44) == TRUE)) //histogram from h5x
		{
			s_pContext->_output_dh_close.u1_closeDh_act =1;
			WriteRegister(MC_MC_28_reg, 14, 14, 1);
			WriteRegister(MC_MC_28_reg, 15, 22, 0);
			s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;
			u5_blend_holdfrm_preProcess_a[23] = 30;
			nIdentifiedNum_preProcess_a[23]=23;
		}else if(u5_blend_holdfrm_preProcess_a[23] >0){
			s_pContext->_output_dh_close.u1_closeDh_act =1;
			WriteRegister(MC_MC_28_reg, 14, 14, 1);
			WriteRegister(MC_MC_28_reg, 15, 22, 0);
			s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;
			u5_blend_holdfrm_preProcess_a[23]--;
		}else if(nIdentifiedNum_preProcess_a[23] == 23 ){
			s_pContext->_output_dh_close.u1_closeDh_act =0;
			WriteRegister(MC_MC_28_reg, 14, 14, 0);
			s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=0;
			nIdentifiedNum_preProcess_a[23] = 0;
		}
	}
	/*The Art of Flight disk: 21"50 judder when the sculpture pans*/
	if(s_pContext->_output_read_comreg.u26_me_aAPLp_rb > 2000000 && (scalerVIP_Get_MEMCPatternFlag_Identification(5,26) == TRUE)) //histogram from h5x
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
		u5_blend_holdfrm_preProcess_a[24] = 24;
		nIdentifiedNum_preProcess_a[24]=24;
	}
	else if(u5_blend_holdfrm_preProcess_a[24] >0)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
		u5_blend_holdfrm_preProcess_a[24]--;
	}
	else if(nIdentifiedNum_preProcess_a[24] == 24)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=0;
		nIdentifiedNum_preProcess_a[24] = 0;
	}

	/*DMN Cityscapes 720p60-breakup.mp4*//*Samsara 54"42:fire black hole*//*Samsara 1"29"14:sun light flash when passing through high frequency*//*Valley of fire_breakup.mp4*/
	if((scalerVIP_Get_MEMCPatternFlag_Identification(5,50) == TRUE || scalerVIP_Get_MEMCPatternFlag_Identification(5,33) == TRUE || scalerVIP_Get_MEMCPatternFlag_Identification(5,34) == TRUE || scalerVIP_Get_MEMCPatternFlag_Identification(5,35))) //histogram from h5x
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;
		u5_blend_holdfrm_preProcess_a[25] = 24;
		nIdentifiedNum_preProcess_a[25]=25;
	}
	else if(u5_blend_holdfrm_preProcess_a[25] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;
		u5_blend_holdfrm_preProcess_a[25]--;
	}
	else if(nIdentifiedNum_preProcess_a[25] == 25)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =0;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=0;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=0;
		nIdentifiedNum_preProcess_a[25] = 0;
	}

	/*THE_ECLIPSE.fhd24_01.mkv*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,28) == TRUE) //histogram from h5x
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =2;
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8, 11,  1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 19, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 1);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8, 11,  1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 19, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, 1);
		u5_blend_holdfrm_preProcess_a[26] = 16;
		nIdentifiedNum_preProcess_a[26]=26;
	}
	else if(u5_blend_holdfrm_preProcess_a[26] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =2;
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8, 11,  1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 19, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 1);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8, 11,  1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 19, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, 1);
		u5_blend_holdfrm_preProcess_a[26]--;
	}
	else if(nIdentifiedNum_preProcess_a[26] == 26)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
		nIdentifiedNum_preProcess_a[26] = 0;
	}
	/*Hrange_NMC_1080I60.TS*/
	if((_ABS_(u11_gmv_mvx) > 10) && (_ABS_(u10_gmv_mvy) < 5) && (scalerVIP_Get_MEMCPatternFlag_Identification(5,30) == TRUE)) //histogram from h5x	
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=1;
		u5_blend_holdfrm_preProcess_a[27] = 16;
		nIdentifiedNum_preProcess_a[27]=27;
	}
	else if(u5_blend_holdfrm_preProcess_a[27] >0)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=1;
		u5_blend_holdfrm_preProcess_a[27]--;
	}
	else if(nIdentifiedNum_preProcess_a[27] == 27)
	{		
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=0;
		nIdentifiedNum_preProcess_a[27] = 0;
	}

	/*taxi3_ntsc_n1.m2t*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,31) == TRUE) //histogram from h5x	
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_D4_reg, 16, 16, 0);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_DC_reg, 0, 0, 0);
		u5_blend_holdfrm_preProcess_a[28] = 32;
		nIdentifiedNum_preProcess_a[28]=28;
	}
	else if(u5_blend_holdfrm_preProcess_a[28] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_D4_reg, 16, 16, 0);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_DC_reg, 0, 0, 0);
		u5_blend_holdfrm_preProcess_a[28]--;
	}
	else if(nIdentifiedNum_preProcess_a[28] == 28)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =0;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=0;
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_D4_reg, 16, 16, 1);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_DC_reg, 0, 0, 1);
		nIdentifiedNum_preProcess_a[28] = 0;
	}

	/*Samsara 40"26:edge shaking when riding spinning bike*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,32) == TRUE) //histogram from h5x	
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=1;
		u5_blend_holdfrm_preProcess_a[29] = 24;
		nIdentifiedNum_preProcess_a[29]=29;
	}
	else if(u5_blend_holdfrm_preProcess_a[29] >0)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=1;
		u5_blend_holdfrm_preProcess_a[29]--;
	}
	else if(nIdentifiedNum_preProcess_a[29] == 29)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=0;
		nIdentifiedNum_preProcess_a[29] = 0;
	}
	
	//Toshiba-1080I-0003-short.m2ts?-Still
	if((_ABS_(u11_gmv_mvx) > 10) && (_ABS_(u10_gmv_mvy) < 5) && ((scalerVIP_Get_MEMCPatternFlag_Identification(5,29) == TRUE))) //histogram from h5x
	{
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[0]  = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[1]  = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[2]  = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[3]  = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[4]  = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[5]  = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[6]  = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[7]  = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[8]  = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[9]  = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[10] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[11] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[12] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[13] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[14] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[15] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[16] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[17] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[18] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[19] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[20] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[21] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[22] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[23] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[24] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[25] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[26] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[27] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[28] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[29] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[30] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[31] = 0xff;

		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[0] = 0x00;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[1] = 0x44;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[2] = 0x88;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[3] = 0xaa;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[4] = 0x88;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[5] = 0x44;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[6] = 0x22;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[7] = 0x00;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[8] = 0x00;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[9] = 0x44;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[10] = 0x88;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[11] = 0xaa;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[12] = 0x88;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[13] = 0x44;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[14] = 0x22;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[15] = 0x00;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[16] = 0x00;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[17] = 0x44;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[18] = 0x88;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[19] = 0xaa;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[20] = 0x88;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[21] = 0x44;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[22] = 0x22;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[23] = 0x00;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[24] = 0x00;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[25] = 0x44;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[26] = 0x88;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[27] = 0xaa;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[28] = 0x88;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[29] = 0x44;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[30] = 0x22;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[31] = 0x00;

		u5_blend_holdfrm_preProcess_a[30] = 48;
		nIdentifiedNum_preProcess_a[30]=30;
	}
	else if(u5_blend_holdfrm_preProcess_a[30] >0)
	{
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[0]  = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[1]  = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[2]  = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[3]  = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[4]  = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[5]  = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[6]  = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[7]  = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[8]  = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[9]  = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[10] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[11] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[12] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[13] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[14] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[15] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[16] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[17] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[18] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[19] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[20] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[21] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[22] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[23] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[24] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[25] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[26] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[27] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[28] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[29] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[30] = 0xff;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_level[31] = 0xff;
		
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[0] = 0x00;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[1] = 0x44;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[2] = 0x88;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[3] = 0xaa;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[4] = 0x88;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[5] = 0x44;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[6] = 0x22;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[7] = 0x00;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[8] = 0x00;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[9] = 0x44;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[10] = 0x88;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[11] = 0xaa;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[12] = 0x88;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[13] = 0x44;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[14] = 0x22;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[15] = 0x00;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[16] = 0x00;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[17] = 0x44;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[18] = 0x88;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[19] = 0xaa;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[20] = 0x88;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[21] = 0x44;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[22] = 0x22;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[23] = 0x00;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[24] = 0x00;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[25] = 0x44;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[26] = 0x88;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[27] = 0xaa;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[28] = 0x88;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[29] = 0x44;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[30] = 0x22;
		s_pContext->_output_bRMV_rFB_ctrl.u8_rFB_alpha[31] = 0x00;
		u5_blend_holdfrm_preProcess_a[30]--;
	}
	else if(nIdentifiedNum_preProcess_a[30] == 30)
	{
		nIdentifiedNum_preProcess_a[30] = 0;
	}

    /*06-fallback-MI3*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,45) == TRUE) //histogram from h5x
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 34);
		u5_blend_holdfrm_preProcess_a[31] = 16;
		nIdentifiedNum_preProcess_a[31]=31;
	}else if(u5_blend_holdfrm_preProcess_a[31] >0){
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 34);
		u5_blend_holdfrm_preProcess_a[31]--;
	}else if(nIdentifiedNum_preProcess_a[31] == 31 ){
		s_pContext->_output_dh_close.u1_closeDh_act =0;
		WriteRegister(MC_MC_28_reg, 14, 14, 0);
		nIdentifiedNum_preProcess_a[31] = 0;
	}


	/*I Robot(SD_NTSC)_01*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,46) == TRUE) //histogram from h5x
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 160);
		u5_blend_holdfrm_preProcess_a[32] = 16;
		nIdentifiedNum_preProcess_a[32]=32;
	}else if(u5_blend_holdfrm_preProcess_a[32] >0){
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 160);
		u5_blend_holdfrm_preProcess_a[32]--;
	}else if(nIdentifiedNum_preProcess_a[32] == 32 ){
		s_pContext->_output_dh_close.u1_closeDh_act =0;
		WriteRegister(MC_MC_28_reg, 14, 14, 0);
		WriteRegister(MC_MC_28_reg, 15, 22, 0);
		nIdentifiedNum_preProcess_a[32] = 0;
	}


    /*COM_bus_crop_i*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,47) == TRUE) //histogram from h5x
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 34);
		u5_blend_holdfrm_preProcess_a[33] = 16;
		nIdentifiedNum_preProcess_a[33]=33;
	}else if(u5_blend_holdfrm_preProcess_a[33] >0){
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 34);
		u5_blend_holdfrm_preProcess_a[33]--;
	}else if(nIdentifiedNum_preProcess_a[33] == 33 ){
		s_pContext->_output_dh_close.u1_closeDh_act =0;
		WriteRegister(MC_MC_28_reg, 14, 14, 0);
		nIdentifiedNum_preProcess_a[33] = 0;
	}

{
	static unsigned int frame_meander_enable = 0, me1_mvd_dgain_slope2 = 0, me1_mvd_dgain_y1 = 0, me1_mvd_dgain_y2 = 0,
		          me1_mvd_cuv_x1 = 0,me1_mvd_cuv_x2 = 0,me1_mvd_cuv_x3= 0,me1_mvd_cuv_y1 = 0,me1_mvd_cuv_y2 = 0,me1_mvd_cuv_y3 = 0,me1_mvd_cuv_slope1 = 0,me1_mvd_cuv_slope2 = 0,
				  vbuf_pi_1st_cand0_en=0,vbuf_pi_1st_cand1_en=0,vbuf_pi_1st_cand2_en=0,vbuf_pi_1st_cand3_en = 0,vbuf_pi_1st_cand4_en=0,vbuf_pi_1st_cand5_en=0,vbuf_pi_1st_cand6_en=0,vbuf_pi_1st_cand7_en=0,vbuf_pi_1st_cand8_en = 0,
				  vbuf_pi_1st_cand9_en=0,vbuf_pi_1st_update0_en = 0,vbuf_pi_1st_update1_en = 0,vbuf_pi_1st_update2_en = 0,vbuf_pi_1st_update3_en = 0,vbuf_pi_1st_gmv_en = 0,
				  vbuf_pi_1st_lfsr_mask0_x = 0,vbuf_pi_1st_lfsr_mask0_y=0,vbuf_pi_1st_lfsr_mask1_x = 0,vbuf_pi_1st_lfsr_mask1_y=0,vbuf_pi_1st_lfsr_mask2_x = 0,vbuf_pi_1st_lfsr_mask2_y=0,vbuf_pi_1st_lfsr_mask3_x = 0,vbuf_pi_1st_lfsr_mask3_y=0,
				  vbuf_ip_1st_lfsr_mask0_x = 0,vbuf_ip_1st_lfsr_mask0_y=0,vbuf_ip_1st_lfsr_mask1_x = 0,vbuf_ip_1st_lfsr_mask1_y=0,vbuf_ip_1st_lfsr_mask2_x = 0,vbuf_ip_1st_lfsr_mask2_y=0,vbuf_ip_1st_lfsr_mask3_x = 0,vbuf_ip_1st_lfsr_mask3_y=0;
	
	/*DNM_hrnm 720p60*/ //walking stick
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,51) == TRUE) //histogram from h5x
	{
		if(nIdentifiedNum_preProcess_a[34] == 0)
		{
			ReadRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg, 8, 17,&me1_mvd_dgain_y1);
			ReadRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg, 18, 27,&me1_mvd_dgain_y2);
			ReadRegister(KME_ME1_TOP1_ME1_COMMON1_12_reg, 16, 31,&me1_mvd_dgain_slope2);
			ReadRegister(KME_TOP_KME_TOP_04_reg,29,29,&frame_meander_enable);

			ReadRegister(KME_ME1_TOP1_ME1_COMMON1_06_reg,0,9,  &me1_mvd_cuv_x1);
			ReadRegister(KME_ME1_TOP1_ME1_COMMON1_06_reg,10,19,&me1_mvd_cuv_x2);
			ReadRegister(KME_ME1_TOP1_ME1_COMMON1_06_reg,20,29,&me1_mvd_cuv_x3);
			ReadRegister(KME_ME1_TOP1_ME1_COMMON1_07_reg,0,9,  &me1_mvd_cuv_y1);
			ReadRegister(KME_ME1_TOP1_ME1_COMMON1_07_reg,10,19,&me1_mvd_cuv_y2);
			ReadRegister(KME_ME1_TOP1_ME1_COMMON1_07_reg,20,29,&me1_mvd_cuv_y3);
			ReadRegister(KME_ME1_TOP1_ME1_COMMON1_08_reg,0,12, &me1_mvd_cuv_slope1);
			ReadRegister(KME_ME1_TOP1_ME1_COMMON1_08_reg,13,24,&me1_mvd_cuv_slope2);

			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 0, 0, &vbuf_pi_1st_cand0_en);
			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 1, 1, &vbuf_pi_1st_cand1_en);
			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 2, 2, &vbuf_pi_1st_cand2_en);
			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, &vbuf_pi_1st_cand3_en);
			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4, &vbuf_pi_1st_cand4_en);
			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 5, 5, &vbuf_pi_1st_cand5_en);
			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 6, 6, &vbuf_pi_1st_cand6_en);
			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 7, 7, &vbuf_pi_1st_cand7_en);
			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 8, 8, &vbuf_pi_1st_cand8_en);
			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 9, 9, &vbuf_pi_1st_cand9_en);
			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 10, 10, &vbuf_pi_1st_update0_en);
			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 11, 11, &vbuf_pi_1st_update1_en);
			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 12, 12, &vbuf_pi_1st_update2_en);
			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 13, 13, &vbuf_pi_1st_update3_en);
			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 14, 14, &vbuf_pi_1st_gmv_en);
			

			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,  &vbuf_pi_1st_lfsr_mask0_x);
			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,  &vbuf_pi_1st_lfsr_mask0_y);
			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8, 11, &vbuf_pi_1st_lfsr_mask1_x);
			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15,&vbuf_pi_1st_lfsr_mask1_y);
			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 19,&vbuf_pi_1st_lfsr_mask2_x);
			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23,&vbuf_pi_1st_lfsr_mask2_y);
			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27,&vbuf_pi_1st_lfsr_mask3_x);
			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31,&vbuf_pi_1st_lfsr_mask3_y);

			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,  &vbuf_ip_1st_lfsr_mask0_x);
			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,  &vbuf_ip_1st_lfsr_mask0_y);
			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8, 11, &vbuf_ip_1st_lfsr_mask1_x);
			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15,&vbuf_ip_1st_lfsr_mask1_y);
			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 19,&vbuf_ip_1st_lfsr_mask2_x);
			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23,&vbuf_ip_1st_lfsr_mask2_y);
			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27,&vbuf_ip_1st_lfsr_mask3_x);
			ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31,&vbuf_ip_1st_lfsr_mask3_y);
		}
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 0);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg, 8, 17, 32);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg, 18, 27, 32);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_12_reg, 16, 31, 65530);
		WriteRegister(KME_TOP_KME_TOP_04_reg,29,29, 0);
		
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_06_reg,0,9,   0);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_06_reg,10,19, 40);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_06_reg,20,29, 120);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_07_reg,0,9,   0);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_07_reg,10,19, 21);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_07_reg,20,29, 32);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_08_reg,0,12,  34);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_08_reg,13,24, 9);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 0, 0, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 1, 1, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 2, 2, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 5, 5, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 6, 6, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 7, 7, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 8, 8, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 9, 9, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 10, 10, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 11, 11, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 12, 12, 1);
        WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 13, 13, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 14, 14, 1);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,   3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,   3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8, 11,  3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 19, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 3);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,   3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,   3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8, 11,  3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 19, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, 3);
		u5_blend_holdfrm_preProcess_a[34] = 16;
		nIdentifiedNum_preProcess_a[34]=34;
	}else if(u5_blend_holdfrm_preProcess_a[34] >0){
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 0);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg, 8, 17, 32);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg, 18, 27, 32);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_12_reg, 16, 31, 65530);
		WriteRegister(KME_TOP_KME_TOP_04_reg,29,29, 0);

		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_06_reg,0,9,   0);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_06_reg,10,19, 40);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_06_reg,20,29, 120);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_07_reg,0,9,   0);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_07_reg,10,19, 21);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_07_reg,20,29, 32);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_08_reg,0,12,  34);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_08_reg,13,24, 9);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 0, 0, 1);//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_cand0_en
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 1, 1, 1);//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_cand1_en
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 2, 2, 1);//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_cand2_en
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, 1);//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_cand3_en
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4, 0);//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_cand4_en
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 5, 5, 0);//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_cand5_en
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 6, 6, 0);//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_cand6_en
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 7, 7, 0);//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_cand7_en
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 8, 8, 0);//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_cand8_en
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 9, 9, 0);//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_cand9_en
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 10, 10, 1);//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_update0_en
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 11, 11, 1);//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_update1_en
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 12, 12, 1);//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_update2_en
        WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 13, 13, 0);//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_update3_en
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 14, 14, 1);//FRC_TOP__KME_VBUF_TOP__reg_vbuf_pi_1st_gmv_en
		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,   3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,   3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8, 11,  3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 19, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 3);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,   3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,   3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8, 11,  3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 19, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, 3);
		u5_blend_holdfrm_preProcess_a[34]--;
	}else if(nIdentifiedNum_preProcess_a[34] == 34 ){
		WriteRegister(MC_MC_28_reg, 14, 14, 0);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg, 8, 17, me1_mvd_dgain_y1);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg, 18, 27, me1_mvd_dgain_y2);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_12_reg, 16, 31, me1_mvd_dgain_slope2);
		WriteRegister(KME_TOP_KME_TOP_04_reg,29,29, frame_meander_enable);

		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_06_reg,0,9,   me1_mvd_cuv_x1);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_06_reg,10,19, me1_mvd_cuv_x2);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_06_reg,20,29, me1_mvd_cuv_x3);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_07_reg,0,9,   me1_mvd_cuv_y1);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_07_reg,10,19, me1_mvd_cuv_y2);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_07_reg,20,29, me1_mvd_cuv_y3);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_08_reg,0,12,  me1_mvd_cuv_slope1);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_08_reg,13,24, me1_mvd_cuv_slope2);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 0, 0, vbuf_pi_1st_cand0_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 1, 1, vbuf_pi_1st_cand1_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 2, 2, vbuf_pi_1st_cand2_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, vbuf_pi_1st_cand3_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4, vbuf_pi_1st_cand4_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 5, 5, vbuf_pi_1st_cand5_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 6, 6, vbuf_pi_1st_cand6_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 7, 7, vbuf_pi_1st_cand7_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 8, 8, vbuf_pi_1st_cand8_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 9, 9, vbuf_pi_1st_cand9_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 10, 10, vbuf_pi_1st_update0_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 11, 11, vbuf_pi_1st_update1_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 12, 12, vbuf_pi_1st_update2_en);
        WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 13, 13, vbuf_pi_1st_update3_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 14, 14, vbuf_pi_1st_gmv_en);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,   vbuf_pi_1st_lfsr_mask0_x);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,   vbuf_pi_1st_lfsr_mask0_y);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8, 11,  vbuf_pi_1st_lfsr_mask1_x);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, vbuf_pi_1st_lfsr_mask1_y);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 19, vbuf_pi_1st_lfsr_mask2_x);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, vbuf_pi_1st_lfsr_mask2_y);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, vbuf_pi_1st_lfsr_mask3_x);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, vbuf_pi_1st_lfsr_mask3_y);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,   vbuf_ip_1st_lfsr_mask0_x);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,   vbuf_ip_1st_lfsr_mask0_y);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8, 11,  vbuf_ip_1st_lfsr_mask1_x);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, vbuf_ip_1st_lfsr_mask1_y);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 19, vbuf_ip_1st_lfsr_mask2_x);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, vbuf_ip_1st_lfsr_mask2_y);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, vbuf_ip_1st_lfsr_mask3_x);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, vbuf_ip_1st_lfsr_mask3_y);
		nIdentifiedNum_preProcess_a[34] = 0;
	}
}
	/*7_Harry Potter and The Sorerer's Stone*/
	/*DNM_hrnm 720p60*/ //0:47 rolling-blade girl
	/*07_Montage_P*/ //0:48-0:51 rolling-blade girl
	if(u1_still_frame == 0 && (scalerVIP_Get_MEMCPatternFlag_Identification(5,52) == TRUE || (scalerVIP_Get_MEMCPatternFlag_Identification(5,51) == TRUE) || (scalerVIP_Get_MEMCPatternFlag_Identification(5,58) == TRUE))) //histogram from h5x
	{
		rtd_clearbits(0xb802b500, _BIT0);//  sharpness peaking
		rtd_clearbits(0xb802b950, _BIT0);//  sharpness dlti
		rtd_clearbits(0xb8023c00, _BIT0);//  Multiband peaking
		rtd_setbits(0xb8023c30, _BIT0);//  Multiband peaking DB apply
		WriteRegister(KME_DEHALO_KME_DEHALO_70_reg, 4, 4, 1);
		WriteRegister(KME_DEHALO_KME_DEHALO_B0_reg, 12, 19, 0);
		WriteRegister(KME_DEHALO_KME_DEHALO_D4_reg, 18, 24, 63); 
		WriteRegister(KME_DEHALO_KME_DEHALO_D4_reg, 18, 24, 63);
		u5_blend_holdfrm_preProcess_a[35] = 16;
		nIdentifiedNum_preProcess_a[35]=35;
	}else if(u5_blend_holdfrm_preProcess_a[35] >0){
		rtd_clearbits(0xb802b500, _BIT0);//  sharpness peaking
		rtd_clearbits(0xb802b950, _BIT0);//  sharpness dlti
		rtd_clearbits(0xb8023c00, _BIT0);//  Multiband peaking
		rtd_setbits(0xb8023c30, _BIT0);//  Multiband peaking DB apply
		WriteRegister(KME_DEHALO_KME_DEHALO_70_reg, 4, 4, 1);
		WriteRegister(KME_DEHALO_KME_DEHALO_B0_reg, 12, 19, 0);
		WriteRegister(KME_DEHALO_KME_DEHALO_D4_reg, 18, 24, 63); 
		WriteRegister(KME_DEHALO_KME_DEHALO_D4_reg, 18, 24, 63);
		u5_blend_holdfrm_preProcess_a[35]--;
	}else if(nIdentifiedNum_preProcess_a[35] == 35 ){
		rtd_clearbits(0xb802b500, _BIT0);//  sharpness peaking
		rtd_clearbits(0xb802b950, _BIT0);//  sharpness dlti
		rtd_clearbits(0xb8023c00, _BIT0);//  Multiband peaking
		rtd_setbits(0xb8023c30, _BIT0);//  Multiband peaking DB apply
		WriteRegister(KME_DEHALO_KME_DEHALO_70_reg, 4, 4, 0);
		WriteRegister(KME_DEHALO_KME_DEHALO_B0_reg, 12, 19, 4);
		WriteRegister(KME_DEHALO_KME_DEHALO_D4_reg, 18, 24, 0); 
		WriteRegister(KME_DEHALO_KME_DEHALO_D4_reg, 18, 24, 32);
		nIdentifiedNum_preProcess_a[35] = 0;
	}

#if 1
	//TCLBAND-402: Quality.tv_UHD_universaltest_v101_h264.mp4
	if(u1_still_frame == 0 && scalerVIP_Get_MEMCPatternFlag_Identification(5,21) == TRUE)//histogram from h5x
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[36] = 8;
		nIdentifiedNum_preProcess_a[36]=36;
	}
	else if(u5_blend_holdfrm_preProcess_a[36] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[36]--;
	}
	else if(nIdentifiedNum_preProcess_a[36] == 36)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
		nIdentifiedNum_preProcess_a[36] = 0;
	}
#endif
	/*Montage_M*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,54) == TRUE) //histogram from h5x
	{
	    s_pContext->_output_dh_close.u1_closeDh_act =1;
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 110);
		u5_blend_holdfrm_preProcess_a[37] = 16;
		nIdentifiedNum_preProcess_a[37]=37;
	}else if(u5_blend_holdfrm_preProcess_a[37] >0){
	    s_pContext->_output_dh_close.u1_closeDh_act =1;
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 110);
		u5_blend_holdfrm_preProcess_a[37]--;
	}else if(nIdentifiedNum_preProcess_a[37] == 37 ){
		WriteRegister(MC_MC_28_reg, 14, 14, 0);
		nIdentifiedNum_preProcess_a[37] = 0;
	}

	/*4K.znds.com]ShutUpAndPlay.Berlin.2014.2160p.HDTV.HEVC-jTV-50p-3840x2160*/
	//0:28-0:29   0:58-1:00   01:46-01:48	 02:33-02:34
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,55) == TRUE) //histogram from h5x
	{
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 0);
		u5_blend_holdfrm_preProcess_a[38] = 32;
		nIdentifiedNum_preProcess_a[38]=38;
	}else if(u5_blend_holdfrm_preProcess_a[38] >0){
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 0);
		u5_blend_holdfrm_preProcess_a[38]--;
	}else if(nIdentifiedNum_preProcess_a[38] == 38 ){
		WriteRegister(MC_MC_28_reg, 14, 14, 0);
		nIdentifiedNum_preProcess_a[38] = 0;
	}


	/*14-redbull-santorini-action-25-x265-15-25p-3840x2160*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,56) == TRUE) //histogram from h5x
	{
	    s_pContext->_output_dh_close.u1_closeDh_act =1;
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8, 11, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 19, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 15);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8, 11, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 19, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, 15);
		u5_blend_holdfrm_preProcess_a[39] = 16;
		nIdentifiedNum_preProcess_a[39]=39;
	}else if(u5_blend_holdfrm_preProcess_a[39] >0){
	    s_pContext->_output_dh_close.u1_closeDh_act =1;
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8, 11, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 19, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 15);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8, 11, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 19, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, 15);
		u5_blend_holdfrm_preProcess_a[39]--;
	}else if(nIdentifiedNum_preProcess_a[39] == 39 ){
	    s_pContext->_output_dh_close.u1_closeDh_act =0;
		nIdentifiedNum_preProcess_a[39] = 0;
	}

    /*FRC_Spideman_hd_demo_04.mkv*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,59) == TRUE) //histogram from h5x
	{
	    s_pContext->_output_dh_close.u1_closeDh_act =0;
		WriteRegister(KME_DEHALO_KME_DEHALO_70_reg, 4, 4, 1);
		WriteRegister(KME_DEHALO_KME_DEHALO_B0_reg, 12, 19, 0);
		WriteRegister(KME_DEHALO_KME_DEHALO_D4_reg, 18, 24, 63); 
		WriteRegister(KME_DEHALO_KME_DEHALO_D4_reg, 18, 24, 63);
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 95);
		u5_blend_holdfrm_preProcess_a[40] = 32;
		nIdentifiedNum_preProcess_a[40]=40;
	}else if(u5_blend_holdfrm_preProcess_a[40] >0){
	    s_pContext->_output_dh_close.u1_closeDh_act =0;
		WriteRegister(KME_DEHALO_KME_DEHALO_70_reg, 4, 4, 1);
		WriteRegister(KME_DEHALO_KME_DEHALO_B0_reg, 12, 19, 0);
		WriteRegister(KME_DEHALO_KME_DEHALO_D4_reg, 18, 24, 63); 
		WriteRegister(KME_DEHALO_KME_DEHALO_D4_reg, 18, 24, 63);
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 95);
		u5_blend_holdfrm_preProcess_a[40]--;
	}else if(nIdentifiedNum_preProcess_a[40] == 40 ){
		WriteRegister(KME_DEHALO_KME_DEHALO_70_reg, 4, 4, 0);
		WriteRegister(KME_DEHALO_KME_DEHALO_B0_reg, 12, 19, 4);
		WriteRegister(KME_DEHALO_KME_DEHALO_D4_reg, 18, 24, 0); 
		WriteRegister(KME_DEHALO_KME_DEHALO_D4_reg, 18, 24, 32);
		WriteRegister(MC_MC_28_reg, 14, 14, 0);
		nIdentifiedNum_preProcess_a[40] = 0;
	}

	/*Happy_Gilmore_Longdrive_1080I60*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,60) == TRUE) //histogram from h5x
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		WriteRegister(MC_MC_B4_reg, 16, 23, 255);
		WriteRegister(MC_MC_B4_reg, 24, 31, 255);
		WriteRegister(MC_MC_B8_reg, 11, 11, 0);
		u5_blend_holdfrm_preProcess_a[41] = 32;
		nIdentifiedNum_preProcess_a[41]=41;
	}else if(u5_blend_holdfrm_preProcess_a[41] >0){
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		WriteRegister(MC_MC_B4_reg, 16, 23, 255);
		WriteRegister(MC_MC_B4_reg, 24, 31, 255);
		WriteRegister(MC_MC_B8_reg, 11, 11, 0);
		u5_blend_holdfrm_preProcess_a[41]--;
	}else if(nIdentifiedNum_preProcess_a[41] == 41 ){
		s_pContext->_output_dh_close.u1_closeDh_act =0;
		WriteRegister(MC_MC_B4_reg, 16, 23, 104);
		WriteRegister(MC_MC_B4_reg, 24, 31, 192);
		WriteRegister(MC_MC_B8_reg, 11, 11, 1);
		nIdentifiedNum_preProcess_a[41] = 0;
	}

	/*Spider_man_3_broken_52_12-one*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,61) == TRUE)	
	{
	    s_pContext->_output_dh_close.u1_closeDh_act =1;
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 130);
		u5_blend_holdfrm_preProcess_a[42] = 32;
		nIdentifiedNum_preProcess_a[42]=42;
	}else if(u5_blend_holdfrm_preProcess_a[42] >0){
	    s_pContext->_output_dh_close.u1_closeDh_act =1;
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 130);
		u5_blend_holdfrm_preProcess_a[42]--;
	}else if(nIdentifiedNum_preProcess_a[42] == 42 ){
	    s_pContext->_output_dh_close.u1_closeDh_act =0;
		WriteRegister(MC_MC_28_reg, 14, 14, 0);
		nIdentifiedNum_preProcess_a[42] = 0;
	}

	/*Dehalo_MI3_2*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,62) == TRUE)	
	{	
	    s_pContext->_output_dh_close.u1_closeDh_act =1;
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 147);
		u5_blend_holdfrm_preProcess_a[43] = 32;
		nIdentifiedNum_preProcess_a[43]=43;
	}else if(u5_blend_holdfrm_preProcess_a[43] >0){
	    s_pContext->_output_dh_close.u1_closeDh_act =1;
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 147);
		u5_blend_holdfrm_preProcess_a[43]--;
	}else if(nIdentifiedNum_preProcess_a[43] == 43 ){
	    s_pContext->_output_dh_close.u1_closeDh_act =0;
		WriteRegister(MC_MC_28_reg, 14, 14, 0);
		nIdentifiedNum_preProcess_a[43] = 0;
	}

	/*transformer.3.2011.1080p.Fly_men_03_1080p24-one*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,64) == TRUE)	
	{		
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 147);
		u5_blend_holdfrm_preProcess_a[44] = 32;
		nIdentifiedNum_preProcess_a[44]=44;
	}else if(u5_blend_holdfrm_preProcess_a[44] >0){
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 147);
		u5_blend_holdfrm_preProcess_a[44]--;
	}else if(nIdentifiedNum_preProcess_a[44] == 44 ){		
		WriteRegister(MC_MC_28_reg, 14, 14, 0);
		nIdentifiedNum_preProcess_a[44] = 0;
	}

	/*Spider_man_3_broken_52_12-two*/
	if(((_ABS_(u11_gmv_mvx>>1) <1) > (_ABS_(u10_gmv_mvy>>1) <1)) && scalerVIP_Get_MEMCPatternFlag_Identification(5,65) == TRUE)	
	{		
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 205);
		u5_blend_holdfrm_preProcess_a[45] = 32;
		nIdentifiedNum_preProcess_a[45]=45;
	}else if(u5_blend_holdfrm_preProcess_a[45] >0){
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 205);
		u5_blend_holdfrm_preProcess_a[45]--;
	}else if(nIdentifiedNum_preProcess_a[45] == 45 ){		
		WriteRegister(MC_MC_28_reg, 14, 14, 0);
		nIdentifiedNum_preProcess_a[45] = 0;
	}

	/*transformer.3.2011.1080p.Fly_men_03_1080p24-two*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,66) == TRUE)	
	{		
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 180);
		u5_blend_holdfrm_preProcess_a[46] = 32;
		nIdentifiedNum_preProcess_a[46]=46;
	}else if(u5_blend_holdfrm_preProcess_a[46] >0){
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 180);
		u5_blend_holdfrm_preProcess_a[46]--;
	}else if(nIdentifiedNum_preProcess_a[46] == 46 ){		
		WriteRegister(MC_MC_28_reg, 14, 14, 0);
		nIdentifiedNum_preProcess_a[46] = 0;
	}

	/*Vrange_HK_Stairs*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,67) == TRUE)	
	{		
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 200);
		u5_blend_holdfrm_preProcess_a[47] = 32;
		nIdentifiedNum_preProcess_a[47]=47;
	}else if(u5_blend_holdfrm_preProcess_a[47] >0){
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 200);
		u5_blend_holdfrm_preProcess_a[47]--;
	}else if(nIdentifiedNum_preProcess_a[47] == 47 ){		
		WriteRegister(MC_MC_28_reg, 14, 14, 0);
		nIdentifiedNum_preProcess_a[47] = 0;
	}

    /*casino_royale_halo_1_09_00.mp4-two*/
#if 0
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,68) == TRUE)	
	{		
		WriteRegister(KIWI_REG(FRC_TOP__MC__reg_mc_fb_lvl_force_en), 1);
		WriteRegister(KIWI_REG(FRC_TOP__MC__reg_mc_fb_lvl_force_value), 160);
		u5_blend_holdfrm_preProcess_a[48] = 32;
		nIdentifiedNum_preProcess_a[48]=48;
	}else if(u5_blend_holdfrm_preProcess_a[48] >0){
		WriteRegister(KIWI_REG(FRC_TOP__MC__reg_mc_fb_lvl_force_en), 1);
		WriteRegister(KIWI_REG(FRC_TOP__MC__reg_mc_fb_lvl_force_value), 160);
		u5_blend_holdfrm_preProcess_a[48]--;
	}else if(nIdentifiedNum_preProcess_a[48] == 48 ){		
		WriteRegister(KIWI_REG(FRC_TOP__MC__reg_mc_fb_lvl_force_en), 0);
		nIdentifiedNum_preProcess_a[48] = 0;
	}
#endif
	/*14-redbull-santorini-action-25-x265-15-25p-3840x2160-two*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,69) == TRUE)	
	{		
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 90);
		u5_blend_holdfrm_preProcess_a[49] = 32;
		nIdentifiedNum_preProcess_a[49]=49;
	}else if(u5_blend_holdfrm_preProcess_a[49] >0){
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 90);
		u5_blend_holdfrm_preProcess_a[49]--;
	}else if(nIdentifiedNum_preProcess_a[49] == 49 ){		
		WriteRegister(MC_MC_28_reg, 14, 14, 0);
		nIdentifiedNum_preProcess_a[49] = 0;
	}

    /*SICARIO*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,70) == TRUE)	
	{		
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 164);
		u5_blend_holdfrm_preProcess_a[50] = 32;
		nIdentifiedNum_preProcess_a[50]=50;
	}else if(u5_blend_holdfrm_preProcess_a[50] >0){
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 164);
		u5_blend_holdfrm_preProcess_a[50]--;
	}else if(nIdentifiedNum_preProcess_a[50] == 50 ){		
		WriteRegister(MC_MC_28_reg, 14, 14, 0);
		nIdentifiedNum_preProcess_a[50] = 0;
	}

	/*Life of Pi, 00:37:28 ~ 00:37:37*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,71) == TRUE)	
	{		
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 126);
		u5_blend_holdfrm_preProcess_a[51] = 32;
		nIdentifiedNum_preProcess_a[51]=51;
	}else if(u5_blend_holdfrm_preProcess_a[51] >0){
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 126);
		u5_blend_holdfrm_preProcess_a[51]--;
	}else if(nIdentifiedNum_preProcess_a[51] == 51 ){		
		WriteRegister(MC_MC_28_reg, 14, 14, 0);
		nIdentifiedNum_preProcess_a[51] = 0;
	}

	/*S02E02��00��50---01��09*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,72) == TRUE)	
	{		
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 170);
		u5_blend_holdfrm_preProcess_a[52] = 32;
		nIdentifiedNum_preProcess_a[52]=52;
	}else if(u5_blend_holdfrm_preProcess_a[52] >0){
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 170);
		u5_blend_holdfrm_preProcess_a[52]--;
	}else if(nIdentifiedNum_preProcess_a[52] == 52 ){		
		WriteRegister(MC_MC_28_reg, 14, 14, 0);
		nIdentifiedNum_preProcess_a[52] = 0;
	}

    /*04_databorken_pirate*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,73) == TRUE)	
	{		
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 160);
		u5_blend_holdfrm_preProcess_a[53] = 32;
		nIdentifiedNum_preProcess_a[53]=53;
	}else if(u5_blend_holdfrm_preProcess_a[53] >0){
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 160);
		u5_blend_holdfrm_preProcess_a[53]--;
	}else if(nIdentifiedNum_preProcess_a[53] == 53 ){		
		WriteRegister(MC_MC_28_reg, 14, 14, 0);
		nIdentifiedNum_preProcess_a[53] = 0;
	}
#if 0
	//Sicario 01:40:05~01:40:16
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,75) == TRUE)	
	{		
		WriteRegister(KIWI_REG(FRC_TOP__MC__reg_mc_fb_lvl_force_en), 1);
		WriteRegister(KIWI_REG(FRC_TOP__MC__reg_mc_fb_lvl_force_value), 130);
		u5_blend_holdfrm_preProcess_a[54] = 32;
		nIdentifiedNum_preProcess_a[54]=54;
	}else if(u5_blend_holdfrm_preProcess_a[54] >0){
		WriteRegister(KIWI_REG(FRC_TOP__MC__reg_mc_fb_lvl_force_en), 1);
		WriteRegister(KIWI_REG(FRC_TOP__MC__reg_mc_fb_lvl_force_value), 130);
		u5_blend_holdfrm_preProcess_a[54]--;
	}else if(nIdentifiedNum_preProcess_a[54] == 54 ){		
		WriteRegister(KIWI_REG(FRC_TOP__MC__reg_mc_fb_lvl_force_en), 0);
		nIdentifiedNum_preProcess_a[54] = 0;
	}
#endif
	////Spiderman 2, 00:07:35 ~ 00:07:54
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,76) == TRUE)	
	{		
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 130);
		u5_blend_holdfrm_preProcess_a[55] = 32;
		nIdentifiedNum_preProcess_a[55]=55;
	}else if(u5_blend_holdfrm_preProcess_a[55] >0){
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 130);
		u5_blend_holdfrm_preProcess_a[55]--;
	}else if(nIdentifiedNum_preProcess_a[55] == 55 ){		
		WriteRegister(MC_MC_28_reg, 14, 14, 0);
		nIdentifiedNum_preProcess_a[55] = 0;
	}

	//===debug show
	for(u8_Index =0;u8_Index<255;u8_Index++)
	{
		if(nIdentifiedNum_preProcess_a[u8_Index] !=0){
			if(u32_ID_Log_en==1)
			rtd_pr_memc_emerg("===== [IPRpre_043] nIdentifiedNum_preProcess_a>>[%d][%d]\n", u8_Index,nIdentifiedNum_preProcess_a[u8_Index]);

			pOutput->u1_IP_preProcess_true=1;

		}
	}
	//===
}

VOID Identification_Pattern_wrtAction_TV001(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	static unsigned char u5_blend_holdfrm_a[255] = {0};
	static unsigned char nIdentifiedNum_a[255] = {0};
	//unsigned int mot_diff = s_pContext->_output_filmDetectctrl.u27_ipme_motionPool[_FILM_ALL][0];
	static unsigned char nOnceFlag=12;
	unsigned char u8_Index;
	#if Pro_TCL
	//unsigned char  u8_cadence_Id = s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL];
	//PQL_INPUT_FRAME_RATE in_fmRate = s_pContext->_external_data._input_frameRate;
	unsigned char nblend_y_alpha_Target=0x90;
	unsigned char nblend_uv_alpha_Target=0x70;
	unsigned char nblend_logo_y_alpha_Target=0xd0;
	unsigned char nblend_logo_uv_alpha_Target=0x30;
	#endif

	unsigned int u32_RB_Value;
	signed short u11_gmv_mvx = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
	signed short u10_gmv_mvy = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;
	//unsigned int u25_me_aDTL = s_pContext->_output_read_comreg.u25_me_aDTL_rb;
	//unsigned int u26_aAPLi_rb = s_pContext->_output_read_comreg.u26_me_aAPLi_rb;
	unsigned int u32_pattern298_zidoo_player_forbindden_en;

	unsigned char  u8_cadence_Id = s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL];
	unsigned char  u8_Mixmode_flag = s_pContext->_output_filmDetectctrl.u1_mixMode;
    //int gmv_mvx =0;//, gmv_mvy =0 ;
	PQL_INPUT_FRAME_RATE in_fmRate = s_pContext->_external_data._input_frameRate;


	#if 1 //for YE Test temp test
	unsigned int u32_ID_Log_en=0;//,ES_Test_Value=0;
	ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 24,  24, &u32_ID_Log_en);//YE Test debug dummy register
	#endif

	if((pParam->u1_Identification_Pattern_en== 0)||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_Test_0_flag ==0))
		return;

	//reset
	pOutput->u1_IP_wrtAction_true=0;

	//====================================================================
	if( (scalerVIP_Get_MEMCPatternFlag_Identification(0,1) == TRUE)&&((u11_gmv_mvx!=0)&&(u10_gmv_mvy!=0))&&(s_pContext->_output_frc_sceneAnalysis.u8_OSDPictureModeChg_prt_apply!=0))
	{
		WriteRegister(IPPRE_IPPRE_0C_reg, 0, 7, 0x30);

		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x28);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x24);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x18);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0x14);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0xa);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0x8);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x28);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x24);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x18);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0x14);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0xa);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0x8);


		if(nOnceFlag >=1)
		{
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_3C_reg,0,8, 0);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_3C_reg,9,15, 0);
			#if (IC_K5LP || IC_K6LP || IC_K8LP)
			WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_E0_reg,0,7,0);
			WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_E0_reg,8,14,0);
			#endif
			nOnceFlag --;
		}

		u5_blend_holdfrm_a[1] = 20;
		nIdentifiedNum_a[1]=1;

	}
	else if(u5_blend_holdfrm_a[1] >0)
	{
		WriteRegister(IPPRE_IPPRE_0C_reg, 0, 7, 0x30);

		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x28);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x24);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x18);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0x14);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0xa);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0x8);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x28);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x24);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x18);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0x14);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0xa);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0x8);

		if(nOnceFlag >=1)
		{
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_3C_reg,0,8, 0);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_3C_reg,9,15, 0);
			#if (IC_K5LP || IC_K6LP || IC_K8LP)
			WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_E0_reg,0,7,0);
			WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_E0_reg,8,14,0);
			#endif
			nOnceFlag --;
		}

		u5_blend_holdfrm_a[1]--;
	}
	else if(nIdentifiedNum_a[1] == 1 )
	{
		WriteRegister(IPPRE_IPPRE_0C_reg, 0, 7, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_logo_y_alpha);

		nOnceFlag=12;
		nIdentifiedNum_a[1] = 0;
	}
	//====================================================================

	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,2) == TRUE  )
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x1c);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x18);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x14);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0x10);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0xe);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0xa);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x1c);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x18);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x14);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0x10);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0xe);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0xa);


		u5_blend_holdfrm_a[2] = 8;
		nIdentifiedNum_a[2]=2;

	}
	else if(u5_blend_holdfrm_a[2] >0)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x1c);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x18);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x14);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0x10);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0xe);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0xa);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x1c);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x18);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x14);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0x10);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0xe);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0xa);

		u5_blend_holdfrm_a[2]--;
	}
	else if(nIdentifiedNum_a[2] == 2 )
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp0);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp1);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp2);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp3);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp4);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp5);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp6);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp0);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp1);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp2);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp3);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp4);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp5);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp6);


		nIdentifiedNum_a[2] = 0;
	}
//====================================================================
#if 0
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,100) == TRUE  )
	{
		//====fanny_xiong settings for K7 dehalo.====
		WriteRegister(MC2_MC2_50_reg, 0, 1, 0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 13, 13, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 12, 12, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 11, 11, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 12, 12, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 11, 11, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 10, 10, 0x1);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg, 24, 31, 0x28);

		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg, 16, 23, 0x28);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,  8, 15, 0x28);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,  0,  7, 0x28);

		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg, 24, 31, 0x28);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg, 16, 23, 0x30);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,  8, 15, 0x30);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,  0,  7, 0x28);
		//====over=======

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,4,8,  10);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,13,13,  1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,16,23, 16);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,24,31, 16);

		WriteRegister(KME_DEHALO2_KME_DEHALO_POSTCORR_reg,0,3, 12);
		WriteRegister(KME_DEHALO2_KME_DEHALO_POSTCORR_reg,4,7,  12);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,  3);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,  0);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26, 3);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29, 0);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,  1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,  0);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11, 1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14, 0);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,4,5,  1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,8,9, 1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,12,14,   1);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,8,15, 32);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,24,31, 40);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8, 1);

		u5_blend_holdfrm_a[3] = 50;
		nIdentifiedNum_a[3]=100;
	}
	else if( u5_blend_holdfrm_a[3] > 0 )
	{
		//====fanny_xiong settings for K7 dehalo.====
		WriteRegister(MC2_MC2_50_reg, 0, 1, 0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 13, 13, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 12, 12, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 11, 11, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 12, 12, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 11, 11, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 10, 10, 0x1);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg, 24, 31, 0x28);

		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg, 16, 23, 0x28);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,  8, 15, 0x28);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,  0,  7, 0x28);

		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg, 24, 31, 0x28);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg, 16, 23, 0x30);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,  8, 15, 0x30);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,  0,  7, 0x28);
		//====over=======

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,4,8,  10);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,13,13,  1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,16,23, 16);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,24,31, 16);

		WriteRegister(KME_DEHALO2_KME_DEHALO_POSTCORR_reg,0,3, 12);
		WriteRegister(KME_DEHALO2_KME_DEHALO_POSTCORR_reg,4,7,  12);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,  3);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,  0);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26, 3);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29, 0);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,  1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,  0);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11, 1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14, 0);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,4,5,  1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,8,9, 1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,12,14,   1);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,8,15, 32);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,24,31, 40);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8, 1);

		u5_blend_holdfrm_a[3]--;
		
	}
	else if(nIdentifiedNum_a[3]==100)
	{

		//===fanny_xiong settings for K7 dehalo:===
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 0, 3, 0x0);
                if(s_pContext->_external_data._output_mode == _PQL_OUT_VIDEO_MODE)
                        WriteRegister(MC2_MC2_50_reg, 0, 1, 0x3);
                if(s_pContext->_external_data._output_mode == _PQL_OUT_PC_MODE)
                        WriteRegister(MC2_MC2_50_reg, 0, 1, 0x0);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand3_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand4_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand3_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand4_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 13, 13, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update1_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 12, 12, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update2_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 11, 11, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update0_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 12, 12, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update1_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 11, 11, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update2_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 10, 10, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update0_en);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg, 24, 31, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st3);

		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg, 16, 23, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st4);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,  8, 15, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st5);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,  0,  7, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st6);

		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg, 24, 31, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st3);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg, 16, 23, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st4);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,  8, 15, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st5);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,  0,  7, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st6);

		//====over=======
		
		nIdentifiedNum_a[3]=0;
	}

#endif

#if 1    //YE Test temp remark for little boy
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,101) == TRUE  )
	{
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,4,8,  10);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,13,13,  1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,16,23, 16);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,24,31, 16);

		WriteRegister(KME_DEHALO2_KME_DEHALO_POSTCORR_reg,0,3, 12);
		WriteRegister(KME_DEHALO2_KME_DEHALO_POSTCORR_reg,4,7,  12);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,  3);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,  0);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26, 3);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29, 0);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,  1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,  0);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11, 1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14, 0);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,4,5,  1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,8,9, 1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,12,14,   1);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,8,15, 32);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,24,31, 40);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8, 1);

		u5_blend_holdfrm_a[4] = 8;
		nIdentifiedNum_a[4]=101;
	}
	else if( u5_blend_holdfrm_a[101] > 0 )
	{
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,4,8,  10);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,13,13,  1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,16,23, 16);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,24,31, 16);

		WriteRegister(KME_DEHALO2_KME_DEHALO_POSTCORR_reg,0,3, 12);
		WriteRegister(KME_DEHALO2_KME_DEHALO_POSTCORR_reg,4,7,  12);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,  3);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,  0);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26, 3);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29, 0);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,  1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,  0);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11, 1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14, 0);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,4,5,  1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,8,9, 1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,12,14,   1);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,8,15, 32);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,24,31, 40);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8, 1);

		u5_blend_holdfrm_a[4]--;
	}
	else if(nIdentifiedNum_a[4]==101)
	{
		nIdentifiedNum_a[4]=0;
	}
	//===========================

#endif

	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_44_reg,10,19,&u32_RB_Value);

	if((scalerVIP_Get_MEMCPatternFlag_Identification(0,105) == TRUE)&&(u32_RB_Value<=510))
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_4C_reg, 6, 10, 0x8);
		u5_blend_holdfrm_a[5] = 8;
		nIdentifiedNum_a[5]=105;	
	}
	else if(u5_blend_holdfrm_a[5] >0)
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_4C_reg, 6, 10, 0x8);
		u5_blend_holdfrm_a[5]--;
	}
	else if(nIdentifiedNum_a[5] == 105 )
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_4C_reg, 6, 10, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_invalid_cnt_th);
		nIdentifiedNum_a[5] = 0;
	}
	//++ u1_Q13835_flag
	#if 1 //YE Test temp remark for little boy
     if((scalerVIP_Get_MEMCPatternFlag_Identification(0,106) == TRUE  )
	 	&&(( u8_cadence_Id == _CAD_VIDEO) ||(u8_Mixmode_flag==1))
	 	&&(s_pContext->_output_me_sceneAnalysis.u1_Q13835_flag==1))
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_4C_reg, 6, 10, 0x8);
		u5_blend_holdfrm_a[6] = 24;
		nIdentifiedNum_a[6]=106;
	}
	else if(u5_blend_holdfrm_a[6] >0)
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_4C_reg, 6, 10, 0x8);
		u5_blend_holdfrm_a[6]--;
	}
	else if(nIdentifiedNum_a[6] == 106 )
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_4C_reg, 6, 10, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_invalid_cnt_th);

		nIdentifiedNum_a[6] = 0;
	}
	#endif 
	//-- u1_Q13835_flag

	#if 0
	// ++ YE Test Edge Shaking v3
	if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag ==1)
	{

		if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag ==1)
		u5_blend_holdfrm_a[4] = 60;
		else
		u5_blend_holdfrm_a[4] = 12;

		nIdentifiedNum_a[7]=107;
	}
	else if(u5_blend_holdfrm_a[4] >0)
	{
		if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag ==0)
		u5_blend_holdfrm_a[4]=0;
		else
		u5_blend_holdfrm_a[4]--;
	}
	else if(nIdentifiedNum_a[7] == 107 )
	{

		nIdentifiedNum_a[7] = 0;
	}
	// -- YE Test Edge Shaking
	#endif

#if 0 //henry for tv011 demo
 //++ Merlin6 memc pred mv new algo for soccer 
       ReadRegister(HARDWARE_HARDWARE_24_reg, 10, 10, &u32_RB_Value);
       if(u32_RB_Value==1)
       {
		if((scalerVIP_Get_MEMCPatternFlag_Identification(0,114) == TRUE))
		{
			WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_1_reg, 28, 28,  1  );  // new algo switch  enable
			u5_blend_holdfrm_a[7] = 25;
			nIdentifiedNum_a[7]=114;
		}
		else if(u5_blend_holdfrm_a[7] >0)
		{
			WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_1_reg, 28, 28,  1  );  
			u5_blend_holdfrm_a[7]--;
		}
		else if(nIdentifiedNum_a[7] == 114 )
		{
			WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_1_reg, 28, 28,  0  );  
			nIdentifiedNum_a[7] = 0;
		}
       }
 //-- Merlin6 memc pred mv new algo for soccer 
#endif	

#if 1 //YE Test 20200521 remove here
//++#pattern 298 daredevil boy on zidoo player.
	ReadRegister(HARDWARE_HARDWARE_58_reg, 3, 3, &u32_pattern298_zidoo_player_forbindden_en);   //  bit3

	if(u32_pattern298_zidoo_player_forbindden_en== 1){
		if(scalerVIP_Get_MEMCPatternFlag_Identification(0,108) == TRUE  ){
			//========action part:=======
			//====fanny_xiong settings for K7 dehalo.====
			WriteRegister(MC2_MC2_50_reg, 0, 1, 0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 13, 13, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 12, 12, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 11, 11, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 12, 12, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 11, 11, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 10, 10, 0x1);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg, 24, 31, 0x28);

			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg, 16, 23, 0x28);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,  8, 15, 0x28);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,  0,  7, 0x28);

			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg, 24, 31, 0x28);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg, 16, 23, 0x30);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,  8, 15, 0x30);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,  0,  7, 0x28);
			//====over=======

			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,4,8,  10);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,13,13,  1);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,16,23, 16);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,24,31, 16);

			WriteRegister(KME_DEHALO2_KME_DEHALO_POSTCORR_reg,0,3, 12);
			WriteRegister(KME_DEHALO2_KME_DEHALO_POSTCORR_reg,4,7,  12);

			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,  3);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,  0);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26, 3);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29, 0);

			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,  1);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,  0);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11, 1);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14, 0);

			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,4,5,  1);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,8,9, 1);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,12,14,   1);

			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,8,15, 32);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,24,31, 40);

			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8, 1);

			//========over===========

			u5_blend_holdfrm_a[8] = 80;
			nIdentifiedNum_a[8]=108;

		}else if( u5_blend_holdfrm_a[8] > 0 ){
			//========action part:=======
			//====fanny_xiong settings for K7 dehalo.====
			WriteRegister(MC2_MC2_50_reg, 0, 1, 0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 13, 13, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 12, 12, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 11, 11, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 12, 12, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 11, 11, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 10, 10, 0x1);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg, 24, 31, 0x28);

			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg, 16, 23, 0x28);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,  8, 15, 0x28);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,  0,  7, 0x28);

			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg, 24, 31, 0x28);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg, 16, 23, 0x30);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,  8, 15, 0x30);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,  0,  7, 0x28);
			//====over=======

			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,4,8,  10);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,13,13,  1);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,16,23, 16);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,24,31, 16);

			WriteRegister(KME_DEHALO2_KME_DEHALO_POSTCORR_reg,0,3, 12);
			WriteRegister(KME_DEHALO2_KME_DEHALO_POSTCORR_reg,4,7,  12);

			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,  3);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,  0);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26, 3);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29, 0);

			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,  1);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,  0);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11, 1);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14, 0);

			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,4,5,  1);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,8,9, 1);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,12,14,   1);

			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,8,15, 32);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,24,31, 40);

			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8, 1);

			//========over===========

			u5_blend_holdfrm_a[8]--;
		}
		else if(nIdentifiedNum_a[8]==108){
			//========action part:=======

			//===fanny_xiong settings for K7 dehalo:===

			WriteRegister(MC2_MC2_50_reg, 0, 1, 0x0);
			/*
	                if(s_pContext->_external_data._output_mode == _PQL_OUT_VIDEO_MODE)
	                        WriteRegister(MC2_MC2_50_reg, 0, 1, 0x3);
	                if(s_pContext->_external_data._output_mode == _PQL_OUT_PC_MODE)
	                        WriteRegister(MC2_MC2_50_reg, 0, 1, 0x0);
			*/
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand3_en);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand4_en);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand3_en);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand4_en);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 13, 13, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update1_en);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 12, 12, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update2_en);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 11, 11, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update0_en);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 12, 12, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update1_en);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 11, 11, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update2_en);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 10, 10, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update0_en);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg, 24, 31, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st3);

			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg, 16, 23, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st4);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,  8, 15, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st5);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,  0,  7, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st6);

			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg, 24, 31, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st3);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg, 16, 23, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st4);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,  8, 15, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st5);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,  0,  7, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st6);

			//========over===========

			nIdentifiedNum_a[8]=0;
		}
	}
//--#pattern 298 daredevil boy on zidoo player.
#endif




	#if 1
	// ++ YE Test Edge Shaking
	if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_413_ID_flag ==1)
	{

		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_00_reg,24,29,0xd);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_01_reg,0,5,0x6);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_50_reg,24,29,0xd);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_54_reg,0,5,0x6);

		u5_blend_holdfrm_a[9] = 48;


		nIdentifiedNum_a[9]=110;

	}
	else if(u5_blend_holdfrm_a[9] >0)
	{

		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_00_reg,24,29,0xd);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_01_reg,0,5,0x6);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_50_reg,24,29,0xd);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_54_reg,0,5,0x6);

			u5_blend_holdfrm_a[9]--;

		if (s_pContext->_output_bRMV_rFB_ctrl.u8_ES_413_ID_flag ==0)
			u5_blend_holdfrm_a[9]=0;

	}
	else if(nIdentifiedNum_a[9] == 110 )
	{

		nIdentifiedNum_a[9] = 0;
	}
	// -- YE Test Edge Shaking
	#endif
#if 0
      //++ Merlin6 memc new algo for #185 notebook  
       ReadRegister(KME_ME1_TOP2_KME_ME1_TOP2_B0_reg, 0, 31, &u32_RB_Value);
       gmv_mvx = ((u32_RB_Value >> 10) &1) == 0? (u32_RB_Value & 0x7ff) : ((u32_RB_Value& 0x7ff)  - (1<<11));
       ReadRegister(HARDWARE_HARDWARE_24_reg, 8, 8, &u32_RB_Value);
       if(scalerVIP_Get_MEMCPatternFlag_Identification(0,113) == TRUE&&(u32_RB_Value==1)&&((gmv_mvx>-80)&&(gmv_mvx<-5)))
	{
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 0, 15, 1);
		u5_blend_holdfrm_a[7] =150;
		nIdentifiedNum_a[10]=113;
#if 0 // merlin7_bring_up_dehalo
	       WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 29 , 29 , 1 ); // new algo switch
		WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 28 , 28 , 1 ); //UL
		WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 27 , 27 , 1 ); //UR
		WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 26 , 26 , 0 ); //DL
		WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 25 , 25 , 1 ); //DR
		WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 24 , 24 , 0 ); //APL sel
		WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 18 , 23 , 18 );//hi_APL_num_th
		WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 12 , 17 , 18 );//low_APL_num_th
		WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 6 , 11 ,  18 );//near_Gmv_num_th
		WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 0 , 5 ,  25 ); //far_Gmv_num_th

		WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 0 , 7 ,  23 ); //low_Gmv_diff_th
		WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 8 , 15 ,  25 ); //high_Gmv_diff_th
		WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 16 , 23 ,  30 ); //low_APL_th
		WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 24 , 31 ,  70 ); //high_APL_th

		WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 0 , 4 , 0 ); //gmv_num
		WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 6 , 7 , 1 ); //gmv_len
		WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 8 , 15 , 100 ); //set_occl_apl_th
		WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 16 , 23 , 100 ); //apl_th
		WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 24 , 31 , 5); //Gmv_diff_th

		WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 0 , 0 , 1 ); //rtgmvd_bypass
		WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 1 , 1 , 0 ); //gmv_bypass
		WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 2 , 3 , 1 ); //case_mark_mode
		WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 4 , 4 , 1 ); //result_apply
		WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 5 , 5 , 0 );//pred_occl_en
		WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 6 , 6 , 1 ); //occl_hor_en
		WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 7 , 7 , 0 ); //occl_ver_en
		WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 8 , 8 , 0 ); //debug mode

		WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 0 , 2 , 0 );  //res_num
		WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 3 , 4 , 0 );  //res_mode
		WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 5 , 9 , 2 );  //rim down
		WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 10 , 14 , 2 ); //rim up
		WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 15 , 19 , 0 );  //rim right
		WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 20 , 24 , 0 ); //rim left
#endif

		WriteRegister(HARDWARE_HARDWARE_25_reg, 10, 10 , 0x1 );
		WriteRegister(HARDWARE_HARDWARE_24_reg, 0, 7 ,  1);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg , 0 , 0 , 1 );  //sw gmv en
#if 0 // merlin7_bring_up_dehalo
		WriteRegister(KME_DEHALO4_RT2_MOD_1_reg , 0 , 0 , 0 ); //fix1 en
		WriteRegister(KME_DEHALO4_RT2_MOD_1_reg , 1 , 1 , 0 ); //fix1 en
		WriteRegister(KME_ME1_TOP10_APL_FILTER0_reg , 0 , 0 , 0 );
#endif
	}
	else if((u5_blend_holdfrm_a[7] >0)&&(u32_RB_Value==1))
	{
#if 0 // merlin7_bring_up_dehalo	
		WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 29 , 29 , 1 ); // new algo switch
#endif
		u5_blend_holdfrm_a[7]--;
		WriteRegister(HARDWARE_HARDWARE_25_reg, 10, 10 , 0x1 );
	}
	else if((nIdentifiedNum_a[10] == 113 )&&(u32_RB_Value==1))
	{
#if 0 // merlin7_bring_up_dehalo	
              WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 29 , 29 , 0); // new algo switch
#endif
              WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg , 0 , 0 , 0 );
		WriteRegister(HARDWARE_HARDWARE_25_reg, 10, 10 , 0x0 );
		WriteRegister(HARDWARE_HARDWARE_24_reg, 0, 7 ,  0);
		nIdentifiedNum_a[10] = 0;
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 12, 15, 0);
	}
	//-- Merlin6 memc new algo for #185 notebook  
#endif

#if 1 //YE Test edge shaking it's ori place
	// ++ YE Test Edge Shaking
        //if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag	==1)
	if(((s_pContext->_output_bRMV_rFB_ctrl.u8_ES_439_ID_flag ==1)
		||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_21_ID_flag ==1)
		||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_421_ID_flag ==1))
		&&(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag !=1))
	{

		/*
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x3);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x3);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,24,31,0x4);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_04_reg,24,31,0x90);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_44_reg,24,31,0x90);
	      WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_88_reg,0,3,0x0);
		*/
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_00_reg,24,29,0xd);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_01_reg,0,5,0x6);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_01_reg,12,20,0xFC);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_01_reg,21,29,0xFE);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_50_reg,24,29,0xd);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_54_reg,0,5,0x6);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_54_reg,0,5,0x6);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_54_reg,12,20,0xFC);

		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_E0_reg,0,7,0x40);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_3C_reg,0,8,0x40);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,26,27,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,24,25,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,23,23,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,22,22,0x1);

		if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_439_ID_flag ==1){
		u5_blend_holdfrm_a[11] = 60;
		}
		
		if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_421_ID_flag ==1){
		u5_blend_holdfrm_a[11] = 120;
		}

		if (s_pContext->_output_bRMV_rFB_ctrl.u8_ES_21_ID_flag ==1){
		u5_blend_holdfrm_a[11] = 240;//60;
		}
		
		nIdentifiedNum_a[11]=114;
		
	}
	else if(u5_blend_holdfrm_a[11] >0)
	{
		/*
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x3);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x3);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,24,31,0x4);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_04_reg,24,31,0x90);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_44_reg,24,31,0x90);
	      WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_88_reg,0,3,0x0);
		*/
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_00_reg,24,29,0xd);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_01_reg,0,5,0x6);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_01_reg,12,20,0xFC);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_01_reg,21,29,0xFE);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_50_reg,24,29,0xd);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_54_reg,0,5,0x6);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_54_reg,0,5,0x6);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_54_reg,12,20,0xFC);

		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_E0_reg,0,7,0x40);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_3C_reg,0,8,0x40);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,26,27,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,24,25,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,23,23,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,22,22,0x1);

			u5_blend_holdfrm_a[11]--;

		if ((s_pContext->_output_bRMV_rFB_ctrl.u8_ES_21_ID_flag ==0)
			||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_421_ID_flag ==0)
			||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_439_ID_flag ==0))
			u5_blend_holdfrm_a[11]=0;

	}
	else if(nIdentifiedNum_a[11] == 114 )
	{
		nIdentifiedNum_a[11] = 0;
	}
	// -- YE Test Edge Shaking
#endif





#if 1
	// ++ YE Test move edge shaking apply from FBlevelCtrl.c here
	if((s_pContext->_output_bRMV_rFB_ctrl.u8_ES_439_ID_flag ==1)||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_413_ID_flag ==1)
		||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_421_ID_flag ==1)||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_21_ID_flag==1)
		||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_Rassia_ID_flag==1))
	{
		//WriteRegister(MC_MC_28_reg, 14, 14,0x1);
		//WriteRegister(KMC_BI_bi_top_01_reg,2,3,0x2);


		if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_413_ID_flag ==1){
				WriteRegister(KME_DEHALO_KME_DEHALO_F0_reg, 24, 31, 0x00); 
			      WriteRegister(KME_DEHALO_KME_DEHALO_F4_reg, 24, 31, 0x00);
			      WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x00); 
			      WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0x00);
			}
		else if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_439_ID_flag ==1){
		           WriteRegister(KME_DEHALO_KME_DEHALO_F0_reg, 24, 31, 0x0F); 
			     WriteRegister(KME_DEHALO_KME_DEHALO_F4_reg, 24, 31, 0xFF);
			     WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x0F); 
			     WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0xFF);
			}
		else	if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_421_ID_flag ==1)
			{
		           WriteRegister(KME_DEHALO_KME_DEHALO_F0_reg, 24, 31, 0x0F); 
			     WriteRegister(KME_DEHALO_KME_DEHALO_F4_reg, 24, 31, 0xFF);
			     WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x0F); 
			     WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0xFF);
			}
		else	if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_21_ID_flag ==1)
			{
		           WriteRegister(KME_DEHALO_KME_DEHALO_F0_reg, 24, 31, 0x0F); 
			     WriteRegister(KME_DEHALO_KME_DEHALO_F4_reg, 24, 31, 0xFF);
			     WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x0F); 
			     WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0xFF);
			}
		else	if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_Rassia_ID_flag==1)
			{
		           WriteRegister(KME_DEHALO_KME_DEHALO_F0_reg, 24, 31, 0x0F); 
			     WriteRegister(KME_DEHALO_KME_DEHALO_F4_reg, 24, 31, 0xFF);
			     WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x0F); 
			     WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0xFF);
			}		
		u5_blend_holdfrm_a[12] = 24;//60;
		nIdentifiedNum_a[12]=115;

	}
	else if(u5_blend_holdfrm_a[12] >0)
	{
		//WriteRegister(MC_MC_28_reg, 14, 14,0x1);
		//WriteRegister(KMC_BI_bi_top_01_reg,2,3,0x2);


		if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_413_ID_flag ==1){
				WriteRegister(KME_DEHALO_KME_DEHALO_F0_reg, 24, 31, 0x00); 
			      WriteRegister(KME_DEHALO_KME_DEHALO_F4_reg, 24, 31, 0x00);
			      WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x00); 
			      WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0x00);
		}
		else if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_439_ID_flag ==1){
		           WriteRegister(KME_DEHALO_KME_DEHALO_F0_reg, 24, 31, 0x0F); 
			     WriteRegister(KME_DEHALO_KME_DEHALO_F4_reg, 24, 31, 0xFF);
			     WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x0F); 
			     WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0xFF);
		}
		else	{
		           WriteRegister(KME_DEHALO_KME_DEHALO_F0_reg, 24, 31, 0x0F); 
			     WriteRegister(KME_DEHALO_KME_DEHALO_F4_reg, 24, 31, 0xFF);
			     WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x0F); 
			     WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0xFF);
		}
		

		u5_blend_holdfrm_a[12]--;
	}
	//else if(((nIdentifiedNum_a[12] == 115 )||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_Rassia_ID_flag ==0))&&(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag!=0)) //ori
	else if(nIdentifiedNum_a[12] == 115 ) //YE Test 20200803 new remove Rassia to avoid value be locked
	{

				//WriteRegister(MC_MC_28_reg, 14, 14,0x0);
				//WriteRegister(KMC_BI_bi_top_01_reg,2,3,0x1);

				#if 0 //YE Test 20200804 x1/x2/y1/y2 now all be controled by auto, don't need to return value back				
		       	WriteRegister(KME_DEHALO_KME_DEHALO_F0_reg, 24, 31, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x1); 
				WriteRegister(KME_DEHALO_KME_DEHALO_F4_reg, 24, 31, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x2);
				WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_y1); 
			      WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_y2);
				#endif
		nIdentifiedNum_a[12] = 0;
	}
	// -- YE Test move edge shaking apply from FBlevelCtrl.c here

#endif

	#if 1
	// ++ YE Test 035 girl repeat pattern
	if((s_pContext->_output_me_sceneAnalysis.u1_RP_035_flag==1)&&(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag!=1))
	{
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg,24,31,0x2d);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,5,5,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,6,6,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,7,7,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,8,8,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,9,9,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,10,10,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,12,12,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,13,13,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,14,14,0x0);

		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg,24,31,0x2d);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,4,4,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,5,5,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,6,6,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,7,7,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,8,8,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,9,9,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,11,11,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,12,12,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,13,13,0x0);

		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_2C_reg,21,25,0x1);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,18,23,0x0);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,8,10,0x7);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,11,13,0x7);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,20,22,0x1);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,23,25,0x7);

		u5_blend_holdfrm_a[13] = 48;


		nIdentifiedNum_a[13]=116 ;
	}
	else if(u5_blend_holdfrm_a[13] >0)
	{
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg,24,31,0x2d);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,5,5,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,6,6,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,7,7,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,8,8,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,9,9,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,10,10,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,12,12,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,13,13,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,14,14,0x0);

		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg,24,31,0x2d);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,4,4,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,5,5,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,6,6,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,7,7,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,8,8,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,9,9,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,11,11,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,12,12,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,13,13,0x0);

		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_2C_reg,21,25,0x1);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,18,23,0x0);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,8,10,0x7);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,11,13,0x7);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,20,22,0x1);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,23,25,0x7);

		
			u5_blend_holdfrm_a[13]--;

	}
	else if(nIdentifiedNum_a[13] == 116)
	{

		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg,24,31,0x28);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,5,5,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,6,6,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,7,7,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,8,8,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,9,9,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,10,10,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,12,12,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,13,13,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,14,14,0x1);

		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg,24,31,0x28);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,4,4,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,5,5,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,6,6,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,7,7,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,8,8,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,9,9,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,11,11,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,12,12,0x1);
		//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,13,13,0x1);

		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_2C_reg,21,25,0x3);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,18,23,0x1);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,8,10,0x0);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,11,13,0x0);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,20,22,0x0);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,23,25,0x0);

		nIdentifiedNum_a[13] = 0;
		
	}
	// -- YE Test 035 girl repeat pattern
	#endif
	
	#if 1
	// ++ YE Test 053 repeat pattern
	if(s_pContext->_output_me_sceneAnalysis.u1_RP_053_flag==1)
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,0,3,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,4,7,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,8,11,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,0,3,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,0,3,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,4,7,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,8,11,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x3);

		u5_blend_holdfrm_a[14] = 48;


		nIdentifiedNum_a[14]=117 ;
	}
	else if(u5_blend_holdfrm_a[14] >0)
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,0,3,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,4,7,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,8,11,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,0,3,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,0,3,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,4,7,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,8,11,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x3);
		
			u5_blend_holdfrm_a[14]--;

	}
	else if(nIdentifiedNum_a[14] == 117)
	{


		nIdentifiedNum_a[14] = 0;
	}
	// -- YE Test 53 repeat pattern
	#endif


	#if 1
	// ++ YE Test 062 repeat pattern
	if(((s_pContext->_output_me_sceneAnalysis.u1_RP_062_1_flag==1)
		||(s_pContext->_output_me_sceneAnalysis.u1_RP_062_2_flag==1))
		&&(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag!=1))
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,0,3,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,4,7,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,8,11,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,0,3,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,0,3,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,4,7,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,8,11,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x1);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,12,12,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,13,13,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,14,14,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,11,11,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,12,12,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,13,13,0x0);

		
		u5_blend_holdfrm_a[15] = 48;

		nIdentifiedNum_a[15]=118 ;
	}
	else if(u5_blend_holdfrm_a[15] >0)
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,0,3,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,4,7,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,8,11,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,0,3,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,0,3,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,4,7,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,8,11,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x1);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,12,12,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,13,13,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,14,14,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,11,11,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,12,12,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,13,13,0x0);
		
			u5_blend_holdfrm_a[15]--;

	}
	else if(nIdentifiedNum_a[15] == 118)
	{
		nIdentifiedNum_a[15] = 0;
	}
	// -- YE Test 62 repeat pattern
	#endif


	#if 1
	// ++ YE Test 095 repeat pattern
	if(s_pContext->_output_me_sceneAnalysis.u1_RP_095_flag==1)
	{
		
		//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,11,11,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,12,12,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,13,13,0x0);

		//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,10,10,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,11,11,0x0);
		


		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_2C_reg,21,25,0x7);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,18,23,0x4);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,8,10,0x7);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,11,13,0x7);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,20,22,0x1);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,23,25,0x7);

		u5_blend_holdfrm_a[16] = 48;

		nIdentifiedNum_a[16]=118 ;
	}
	else if(u5_blend_holdfrm_a[16] >0)
	{
	
		//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,11,11,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,12,12,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,13,13,0x0);
	
		//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,10,10,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,11,11,0x0);
		

		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_2C_reg,21,25,0x7);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,18,23,0x4);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,8,10,0x7);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,11,13,0x7);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,20,22,0x1);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,23,25,0x7);

		
			u5_blend_holdfrm_a[16]--;

	}
	else if(nIdentifiedNum_a[16] == 118)
	{
		/*
		//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,11,11,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,12,12,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,13,13,0x1);

		//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,10,10,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,11,11,0x1);
		*/


		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_2C_reg,21,25,0x3);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,18,23,0x1);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,8,10,0x0);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,11,13,0x0);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,20,22,0x0);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,23,25,0x0);

		nIdentifiedNum_a[16] = 0;
	}
	// -- YE Test 95 repeat pattern
	#endif

	if((scalerVIP_Get_MEMCPatternFlag_Identification(0,115) == TRUE) && (s_pContext->_output_me_sceneAnalysis.u1_BTS_DRP_detect_true == 1 )&&((in_fmRate == _PQL_IN_30HZ)||((in_fmRate == _PQL_IN_60HZ)&&( u8_cadence_Id == _CAD_22))))
	{	
		pOutput->u1_BTS_Rp_flag=1;
		
		//me1 ip/pi mask
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 	31,	0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 	31,	0x11111111);
		//me2 dediff ph en
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,1,1, 0);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_84_reg,1,1, 0);

		//== zmv ==
		//me1
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 16, 16,	0);//ip_en
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 15, 15,	0);//pi_en		
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_28_reg, 16, 28, 0);//ip_pnt
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 16, 28, 0);//pi_pnt
		//me2
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg, 16, 16, 0);//1st
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg, 16, 16, 0);	//2nd		
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg, 24, 31, 0);//1st		
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_9C_reg, 0, 7, 0);//2nd
		
		//==ip/pi cand==
		//ip
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 2, 2,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 3, 3,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 6, 6,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 7, 7,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 8, 8,	0);
		//pi
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 2, 2,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 6, 6,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 7, 7,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 8, 8,	0);
				
		//== update cand ==
		//me1 ip
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,  13, 13,  0);
		//me1 pi
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,  12, 12,  0);
		//me2 1st
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,  18, 23, 0);
		//me2 2nd
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg,  21, 26,  0);

		u5_blend_holdfrm_a[17] = 20;
		nIdentifiedNum_a[17]=115;
	}
	else if( u5_blend_holdfrm_a[17] > 0 )
	{		
		//me1 ip/pi mask
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 	31,	0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 	31,	0x11111111);
		//me2 dediff ph en
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,1,1, 0);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_84_reg,1,1, 0);

		//== zmv ==
		//me1
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 16, 16,	0);//ip_en
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 15, 15,	0);//pi_en		
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_28_reg, 16, 28, 0);//ip_pnt
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 16, 28, 0);//pi_pnt
		//me2
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg, 16, 16, 0);//1st
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg, 16, 16, 0);	//2nd		
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg, 24, 31, 0);//1st		
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_9C_reg, 0, 7, 0);//2nd
		
		//==ip/pi cand==
		//ip
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 2, 2,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 3, 3,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 6, 6,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 7, 7,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 8, 8,	0);
		//pi
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 2, 2,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 6, 6,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 7, 7,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 8, 8,	0);
				
		//== update cand ==
		//me1 ip
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,  13, 13,  0);
		//me1 pi
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,  12, 12,  0);
		//me2 1st
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,  18, 23, 0);
		//me2 2nd
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg,  21, 26,  0);
		
		u5_blend_holdfrm_a[17]--;
	}
	else if(nIdentifiedNum_a[17]==115)
	{
		pOutput->u1_BTS_Rp_flag=0;
		
		//me2 dediff ph en
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,1,1, 1);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_84_reg,1,1, 1);

		//== zmv ==
		//me1
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 16, 16,	0);//ip_en
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 15, 15,	0);//pi_en		
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_28_reg, 16, 28, 1023);//ip_pnt
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 16, 28, 1023);//pi_pnt
		//me2
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg, 16, 16, 0);//1st
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg, 16, 16, 0);	//2nd		
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg, 24, 31, 96);//1st pnt		
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_9C_reg, 0, 7, 96);//2nd pnt
		
		//==ip/pi cand==
		//ip
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 2, 2,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 3, 3,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 6, 6,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 7, 7,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 8, 8,	1);
		//pi
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 2, 2,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 6, 6,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 7, 7,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 8, 8,	1);
				
		//== update cand ==
		//me1 ip
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,  13, 13,  1);
		//me1 pi
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,  12, 12,  1);
		//me2 1st
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,  18, 23, 2);
		//me2 2nd
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg,  21, 26,  2);
		
		nIdentifiedNum_a[17]=0;
	}

#if	0 //zebra_24fps	
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,118) == TRUE){
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 16, 16, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 15, 15, 1);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_28_reg, 16, 28, 0);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 16, 28, 0);
		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 22, 22, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 23, 23, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 18, 19, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 20, 21, 0);
		
		WriteRegister(KME_DEHALO2_KME_DEHALO2_90_reg, 0, 31, 0x0000f777);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 0, 31, 0x00001fff);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 0, 31, 0x00111fff);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 0, 31, 0x1fff1fff);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 0, 31, 0x1fff1fff);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 0, 31, 0x1fff1fff);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 0, 31, 0x1fff1fff);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 0, 31, 0x1fff1fff);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 0, 31, 0x1fff1fff);
		u5_blend_holdfrm_a[18] = 16;
		nIdentifiedNum_a[18]=118;
	}else if(u5_blend_holdfrm_a[18] >0){
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 16, 16, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 15, 15, 1);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_28_reg, 16, 28, 0);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 16, 28, 0);
		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 22, 22, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 23, 23, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 18, 19, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 20, 21, 0);
		
		WriteRegister(KME_DEHALO2_KME_DEHALO2_90_reg, 0, 31, 0x0000f777);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 0, 31, 0x00001fff);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 0, 31, 0x00111fff);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 0, 31, 0x1fff1fff);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 0, 31, 0x1fff1fff);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 0, 31, 0x1fff1fff);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 0, 31, 0x1fff1fff);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 0, 31, 0x1fff1fff);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 0, 31, 0x1fff1fff);
		u5_blend_holdfrm_a[18]--;
	}else if(nIdentifiedNum_a[18] == 118){
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 16, 16, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_zmv_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 15, 15,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_zmv_en);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_28_reg, 16, 28,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_zmv);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 16, 28,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_zmv);
		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 22, 22,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_dc_obme_mode_sel);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 23, 23,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_dc_obme_mode_sel);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 18, 19,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_dc_obme_mode);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 20, 21,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_dc_obme_mode);

		
		WriteRegister(KME_DEHALO2_KME_DEHALO2_90_reg, 0, 31,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dehalo2_90);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 0, 31,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dehalo2_A8);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 0, 31,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dehalo2_E8);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 0, 31,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dehalo2_14);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 0, 31,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dehalo2_18);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 0, 31,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dehalo2_1C);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 0, 31,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dehalo2_20);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 0, 31,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dehalo2_24);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 0, 31,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dehalo2_28);
		
		nIdentifiedNum_a[18] = 0;
	}
#endif
	#if 0 //Pro_TCL
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,0) == TRUE && s_pContext->_output_me_vst_ctrl.u1_detect_blackBG_VST_trure ==0  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{


		u5_blend_holdfrm_a[0] = 8;
		nIdentifiedNum_a[0]=0;

	}
	else if(u5_blend_holdfrm_a[0] >0)
	{


		u5_blend_holdfrm_a[0]--;
	}
	else if(nIdentifiedNum_a[0] == 0 )
	{

		nIdentifiedNum_a[0] = 0;
	}

	//===========================================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,12) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(MC_MC_B8_reg, 12, 15, 0xa);//0x2
		WriteRegister(MC_MC_BC_reg, 16, 19, 0xa);//0x0

		u5_blend_holdfrm_a[12] = 16;
		nIdentifiedNum_a[12]=12;

	}
	else if(u5_blend_holdfrm_a[12] >0)
	{
		WriteRegister(MC_MC_B8_reg, 12, 15, 0xa);//0x2
		WriteRegister(MC_MC_BC_reg, 16, 19, 0xa);//0x0
		u5_blend_holdfrm_a[12]--;
	}
	else if(nIdentifiedNum_a[12] == 12 )
	{
		WriteRegister(MC_MC_B8_reg, 12, 15, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_mc_sobj_ymin0);
		WriteRegister(MC_MC_BC_reg, 16, 19, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_mc_sobj_ymin1);
		nIdentifiedNum_a[12] = 0;
	}
	//==========================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,18) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 6, 8, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 12, 12, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 22, 24, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 28, 28, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 6, 8, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 12, 12, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 22, 24, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 28, 28, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 6, 8, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 12, 12, 1);//0x0

		WriteRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 0, 0, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 7, 9, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 6, 8, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 12, 12, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 6, 8, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 12, 12, 1);//0x0

		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 6, 8, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 12, 12, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 22, 24, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 28, 28, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 6, 8, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 12, 12, 1);//0x0

		ReadRegister(IPPRE_IPPRE_04_reg, 4, 11, &u32_RB_Value);
		WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, (u32_RB_Value+nblend_y_alpha_Target)/2);
		ReadRegister(IPPRE_IPPRE_04_reg, 16, 23, &u32_RB_Value);
		WriteRegister(IPPRE_IPPRE_04_reg, 16, 23, (u32_RB_Value+nblend_uv_alpha_Target)/2);
		ReadRegister(IPPRE_IPPRE_0C_reg, 0, 7, &u32_RB_Value);
		WriteRegister(IPPRE_IPPRE_0C_reg, 0, 7, (u32_RB_Value+nblend_logo_y_alpha_Target)/2);
		ReadRegister(IPPRE_IPPRE_0C_reg, 9, 16, &u32_RB_Value);
		WriteRegister(IPPRE_IPPRE_0C_reg, 9, 16, (u32_RB_Value+nblend_logo_uv_alpha_Target)/2);


		u5_blend_holdfrm_a[18] = 16;
		nIdentifiedNum_a[18]=18;

	}
	else if(u5_blend_holdfrm_a[18] >0)
	{
		WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 6, 8, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 12, 12, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 22, 24, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 28, 28, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 6, 8, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 12, 12, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 22, 24, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 28, 28, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 6, 8, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 12, 12, 1);//0x0

		WriteRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 0, 0, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 7, 9, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 6, 8, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 12, 12, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 6, 8, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 12, 12, 1);//0x0

		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 6, 8, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 12, 12, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 22, 24, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 28, 28, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 6, 8, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 12, 12, 1);//0x0

		ReadRegister(IPPRE_IPPRE_04_reg, 4, 11, &u32_RB_Value);
		WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, (u32_RB_Value+nblend_y_alpha_Target)/2);
		ReadRegister(IPPRE_IPPRE_04_reg, 16, 23, &u32_RB_Value);
		WriteRegister(IPPRE_IPPRE_04_reg, 16, 23, (u32_RB_Value+nblend_uv_alpha_Target)/2);
		ReadRegister(IPPRE_IPPRE_0C_reg, 0, 7, &u32_RB_Value);
		WriteRegister(IPPRE_IPPRE_0C_reg, 0, 7, (u32_RB_Value+nblend_logo_y_alpha_Target)/2);
		ReadRegister(IPPRE_IPPRE_0C_reg, 9, 16, &u32_RB_Value);
		WriteRegister(IPPRE_IPPRE_0C_reg, 9, 16, (u32_RB_Value+nblend_logo_uv_alpha_Target)/2);

		u5_blend_holdfrm_a[18]--;
	}
	else if(nIdentifiedNum_a[18] == 18 )
	{
		WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 6, 8, 0x4);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 12, 12,0x0);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 22, 24, 0x4);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 28, 28, 0x0);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 6, 8, 0x4);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 12, 12, 0x0);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 22, 24, 0x4);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 28, 28, 0x0);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 6, 8, 0x4);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 12, 12, 0x0);

		WriteRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 0, 0, 0x0);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 7, 9, 0x4);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 6, 8, 0x4);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 12, 12, 0x0);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 6, 8, 0x4);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 12, 12, 0x0);

		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 6, 8, 0x4);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 12, 12, 0x0);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 22, 24, 0x4);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 28, 28, 0x0);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 6, 8, 0x4);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 12, 12, 0x0);


		WriteRegister(IPPRE_IPPRE_04_reg, 4, 11,s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha);
		WriteRegister(IPPRE_IPPRE_04_reg, 16, 23, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_uv_alpha);
		WriteRegister(IPPRE_IPPRE_0C_reg, 0, 7, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_logo_y_alpha);
		WriteRegister(IPPRE_IPPRE_0C_reg, 9, 16, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_logo_uv_alpha);

		nIdentifiedNum_a[18] = 0;
	}
	//==========================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,19) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		//  BG repeat detect
		WriteRegister(HARDWARE_HARDWARE_58_reg, 11, 11, 1);
		//gmvd
		pOutput->u1_casino_RP_detect_true =1;

		//  lfsr_mask
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 19, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 3);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 19, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, 3);
		// ME1 cddpnt rnd
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_24_reg, 16, 23, 0xff);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_24_reg, 24, 31, 0xff);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_28_reg, 0, 7, 0xff);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_64_reg, 16, 23, 0xff);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_64_reg, 24, 31, 0xff);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 0, 7, 0xff);
		// ME1 adptpnt rnd curve
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 0, 5, 0x3f);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 8, 13, 0x3f);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 14, 19,0x3f);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 20, 25, 0x3f);

		u5_blend_holdfrm_a[19] = 8;
		nIdentifiedNum_a[19]=19;

	}
	else if(u5_blend_holdfrm_a[19] >0)
	{
		//  BG repeat detect
		WriteRegister(HARDWARE_HARDWARE_58_reg, 11, 11, 1);
		//gmvd
		pOutput->u1_casino_RP_detect_true =1;

		//  lfsr_mask
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 19, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 3);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 19, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, 3);
		// ME1 cddpnt rnd
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_24_reg, 16, 23, 0xff);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_24_reg, 24, 31, 0xff);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_28_reg, 0, 7, 0xff);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_64_reg, 16, 23, 0xff);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_64_reg, 24, 31, 0xff);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 0, 7, 0xff);
		// ME1 adptpnt rnd curve
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 0, 5, 0x3f);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 8, 13, 0x3f);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 14, 19,0x3f);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 20, 25, 0x3f);

		u5_blend_holdfrm_a[19]--;
	}
	else if(nIdentifiedNum_a[19] == 19 )
	{
		//  BG repeat detect
		WriteRegister(HARDWARE_HARDWARE_58_reg, 11, 11, 0);
		//gmvd
		pOutput->u1_casino_RP_detect_true =0;
		//  lfsr_mask
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask0_x);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask0_y);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask1_x);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask1_y);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 19, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask2_x);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask2_y);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask3_x);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask3_y);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask0_x);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask0_y);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask1_x);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask1_y);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 19, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask2_x);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask2_y);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask3_x);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask3_y);

		// ME1 cddpnt rnd
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_24_reg, 16, 23, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_rnd0);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_24_reg, 24, 31, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_rnd1);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_28_reg, 0, 7,   s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_rnd2);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_64_reg, 16, 23, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_rnd0);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_64_reg, 24, 31, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_rnd1);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 0, 7,   s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_rnd2);
		// ME1 adptpnt rnd curve
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 0, 5,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_adptpnt_rnd_y1);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 8, 13,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_adptpnt_rnd_y2);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 14, 19, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_adptpnt_rnd_y3);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 20, 25, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_adptpnt_rnd_y4);

		nIdentifiedNum_a[19] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0,30) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_a[30] = 8;
		nIdentifiedNum_a[30]=30;

	}
	else if(u5_blend_holdfrm_a[30] >0)
	{
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_a[30]--;
	}
	else if(nIdentifiedNum_a[30] == 30 )
	{
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		nIdentifiedNum_a[30] = 0;
	}
	//===================================================
	//===================================================
	#endif

	//=======================
	//===debug show
	for(u8_Index =0;u8_Index<255;u8_Index++)
	{
		if(nIdentifiedNum_a[u8_Index] !=0){
			pOutput->u1_IP_wrtAction_true=1;
			if(u32_ID_Log_en==1)
				rtd_pr_memc_emerg("[IPR_001]nIdentifiedNum_a>>[%d][%d]   ,\n", u8_Index, nIdentifiedNum_a[u8_Index]);
		}
	}
	//===

}


VOID Identification_Pattern_wrtAction_TV002(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	//_PQLPARAMETER *PatternParam = GetPQLParameter();
	static unsigned char u5_blend_holdfrm_a[255] = {0};
	static unsigned char nIdentifiedNum_a[255] = {0};
	//unsigned int mot_diff = s_pContext->_output_filmDetectctrl.u27_ipme_motionPool[_FILM_ALL][0];
	//static unsigned char nOnceFlag=12;
	unsigned char u8_Index;
	unsigned int BOT_ratio = 0;
	int diff = 0;
	//static bool monosco_flag = 0;
	//unsigned char  u8_cadence_Id = s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL];
	//PQL_INPUT_FRAME_RATE in_fmRate = s_pContext->_external_data._input_frameRate;
	//unsigned char nblend_y_alpha_Target=0x90;
	//unsigned char nblend_uv_alpha_Target=0x70;
	//unsigned char nblend_logo_y_alpha_Target=0xd0;
	//unsigned char nblend_logo_uv_alpha_Target=0x30;
	
	//unsigned int u32_RB_Value;
	//signed short u11_gmv_mvx = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
#if Pro_SONY
	signed short u10_gmv_mvy = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;
#endif
	//unsigned int u25_me_aDTL = s_pContext->_output_read_comreg.u25_me_aDTL_rb;
	//unsigned int u26_aAPLi_rb = s_pContext->_output_read_comreg.u26_me_aAPLi_rb;
	//unsigned int u32_pattern298_zidoo_player_forbindden_en;
	
	//unsigned char  u8_cadence_Id = s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL];
	//unsigned char  u8_Mixmode_flag = s_pContext->_output_filmDetectctrl.u1_mixMode;
	//int gmv_mvx =0;//, gmv_mvy =0 ;
	//PQL_INPUT_FRAME_RATE in_fmRate = s_pContext->_external_data._input_frameRate;
		
		
#if 1 //for YE Test temp test
			unsigned int u32_ID_Log_en=0;//,ES_Test_Value=0;
			ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 24,  24, &u32_ID_Log_en);//YE Test debug dummy register
#endif

	diff = s_pContext->_output_rimctrl.u12_out_resolution[_RIM_BOT] - s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_BOT];
	BOT_ratio = ((_ABS_(diff))<<10)>>11;
	if((pParam->u1_Identification_Pattern_en== 0)||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_Test_0_flag ==0))
		return;

	//reset
	pOutput->u1_IP_wrtAction_true=0;

		//++ YE Test Sony #7  Telop_Protection_J-Dorama1
	#if Pro_SONY
		if((scalerVIP_Get_MEMCPatternFlag_Identification(1,3) == TRUE  )&&(BOT_ratio==0))
		{
	
				//WriteRegister(SOFTWARE1_00_ADDR_SOFTWARE1_42_ADDR_reg, 0, 7, 0xF);//Merlin7 does not have those register
				//pOutput->u1_MixMode_In=1;
				//s_Context._output_filmDetectctrl.u1_mixMode=1;
				WriteRegister(SOFTWARE1_SOFTWARE1_12_reg, 0, 3, 0);
				WriteRegister(SOFTWARE1_SOFTWARE1_12_reg, 4, 4, 1);
			//rtd_pr_memc_info("==========DynamicOptimizeMEMC 101 =========%d %d\n", FRC_TOP__KME_ME1_TOP0__me1_gmvcost_sel_ADDR,FRC_TOP__KME_ME1_TOP0__reg_me1_pi_gmvd_cost_limt_ADDR);
			u5_blend_holdfrm_a[14] = 120;
			nIdentifiedNum_a[16]=120;
		}
		else if(u5_blend_holdfrm_a[14] >0)
		{
	
			//WriteRegister(SOFTWARE1_00_ADDR_SOFTWARE1_42_ADDR_reg, 0, 7, 0xF);//Merlin7 does not have those register
			//pOutput->u1_MixMode_In=1;
			WriteRegister(SOFTWARE1_SOFTWARE1_12_reg, 0, 3, 0);
			WriteRegister(SOFTWARE1_SOFTWARE1_12_reg, 4, 4, 1);
			u5_blend_holdfrm_a[14]--;
		}
		else if(nIdentifiedNum_a[16] == 120 )
		{
			//WriteRegister(SOFTWARE1_00_ADDR_SOFTWARE1_42_ADDR_reg, 0, 7, 0x1);//Merlin7 does not have those register
			//pOutput->u1_MixMode_In=0;
			WriteRegister(SOFTWARE1_SOFTWARE1_12_reg, 0, 3, 0);
			WriteRegister(SOFTWARE1_SOFTWARE1_12_reg, 4, 4, 1);
			nIdentifiedNum_a[16] = 0;
		}
	#endif
		//-- YE Test Sony #7  Telop_Protection_J-Dorama1
	
	#if 1
		//YE Test Sony #8 Telop_Protection_J-Dorama2
		if((scalerVIP_Get_MEMCPatternFlag_Identification(1,4) == TRUE  )&&(BOT_ratio==0))
		{
			WriteRegister(SOFTWARE1_SOFTWARE1_41_reg, 0, 1, 0x3);//all
			WriteRegister(SOFTWARE1_SOFTWARE1_41_reg, 4, 5, 0x3);//bottom
			WriteRegister(SOFTWARE1_SOFTWARE1_41_reg, 6, 7, 0x3);//left
			WriteRegister(SOFTWARE1_SOFTWARE1_41_reg, 8, 9, 0x3); //right
			//pOutput->u1_mixMode = 1;
	
			//WriteRegister(FRC_TOP__KME_IPME__reg_ipme_fmdet_5region2_v0_ADDR, 0, 9, 0x2d5);
			//WriteRegister(FRC_TOP__KME_IPME__reg_ipme_fmdet_5region2_v1_ADDR, 10, 19, 0x3cf);
	
			//rtd_pr_memc_info("==========DynamicOptimizeMEMC 101 =========%d %d\n", FRC_TOP__KME_ME1_TOP0__me1_gmvcost_sel_ADDR,FRC_TOP__KME_ME1_TOP0__reg_me1_pi_gmvd_cost_limt_ADDR);
			u5_blend_holdfrm_a[15] = 180;
			nIdentifiedNum_a[17]=121;
		}
		else if(u5_blend_holdfrm_a[15] >0)
		{
			WriteRegister(SOFTWARE1_SOFTWARE1_41_reg, 0, 1, 0x3);//all
			WriteRegister(SOFTWARE1_SOFTWARE1_41_reg, 4, 5, 0x3);//bottom
			WriteRegister(SOFTWARE1_SOFTWARE1_41_reg, 6, 7, 0x3);//left
			WriteRegister(SOFTWARE1_SOFTWARE1_41_reg, 8, 9, 0x3); //right
			//pOutput->u1_mixMode = 1;
	
			//WriteRegister(FRC_TOP__KME_IPME__reg_ipme_fmdet_5region2_v0_ADDR, 0, 9, 0x2d5);
			//WriteRegister(FRC_TOP__KME_IPME__reg_ipme_fmdet_5region2_v1_ADDR, 10, 19, 0x3cf);
			u5_blend_holdfrm_a[15]--;
		}
		else if(nIdentifiedNum_a[17] == 121 )
		{
			WriteRegister(SOFTWARE1_SOFTWARE1_41_reg, 0, 1, 0x1);//all
			WriteRegister(SOFTWARE1_SOFTWARE1_41_reg, 4, 5, 0x2);//bottom
			WriteRegister(SOFTWARE1_SOFTWARE1_41_reg, 6, 7, 0x1);//left
			WriteRegister(SOFTWARE1_SOFTWARE1_41_reg, 8, 9, 0x1); //right
	
			//WriteRegister(FRC_TOP__KME_IPME__reg_ipme_fmdet_5region2_v0_ADDR, 0, 9, 0x19b);
			//WriteRegister(FRC_TOP__KME_IPME__reg_ipme_fmdet_5region2_v1_ADDR, 10, 19, 0x218);
			nIdentifiedNum_a[17] = 0;
		}
	#endif
	
	
	//++YE Test Sony new issue ultraviolet_1
	#if Pro_SONY
		if((scalerVIP_Get_MEMCPatternFlag_Identification(1,8) == TRUE ) && ((BOT_ratio>15)&&(BOT_ratio<24)))
		{
			//s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
			//s_pContext->_output_wrt_comreg.u1_KME_gmvd_cost_type=1;
			WriteRegister(KME_DEHALO_KME_DEHALO_F0_reg, 24, 31, 0x00);
			WriteRegister(KME_DEHALO_KME_DEHALO_F4_reg, 24, 31, 0x00);
			WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x00);
			WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0x00);
	
			u5_blend_holdfrm_a[16] = 200;
			nIdentifiedNum_a[18]=126;
		}
		else if(u5_blend_holdfrm_a[16] >0)
		{
			WriteRegister(KME_DEHALO_KME_DEHALO_F0_reg, 24, 31, 0x00);
			WriteRegister(KME_DEHALO_KME_DEHALO_F4_reg, 24, 31, 0x00);
			WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x00);
			WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0x00);
	
			u5_blend_holdfrm_a[16]--;
		}
		else if((nIdentifiedNum_a[18] == 126 )||((BOT_ratio>22)||(BOT_ratio<17)))
		{
	
			nIdentifiedNum_a[18] = 0;
		}
	#endif
	//--YE Test Sony new issue ultraviolet_1
	
	//++YE Test Sony new issue ultraviolet_1 2
	#if Pro_SONY
		if((scalerVIP_Get_MEMCPatternFlag_Identification(1,9) == TRUE )&&((BOT_ratio>17)&&(BOT_ratio<22)))
		{
			//s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
			//s_pContext->_output_wrt_comreg.u1_KME_gmvd_cost_type=1;
			WriteRegister(KME_DEHALO_KME_DEHALO_F0_reg, 24, 31, 0x00);
			WriteRegister(KME_DEHALO_KME_DEHALO_F4_reg, 24, 31, 0x00);
			WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x00);
			WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0x00);
	
			u5_blend_holdfrm_a[17] = 160;
			nIdentifiedNum_a[19]=127;
		}
		else if(u5_blend_holdfrm_a[17] >0)
		{
			WriteRegister(KME_DEHALO_KME_DEHALO_F0_reg, 24, 31, 0x00);
			WriteRegister(KME_DEHALO_KME_DEHALO_F4_reg, 24, 31, 0x00);
			WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x00);
			WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0x00);
	
			u5_blend_holdfrm_a[17]--;
		}
		else if((nIdentifiedNum_a[19] == 127 )||((BOT_ratio>22)||(BOT_ratio<17)))
		{
	
			nIdentifiedNum_a[19] = 0;
		}
	#endif
	//--YE Test Sony new issue ultraviolet_1 2
	
	
	
#if Pro_SONY
		//++ YE Test Sony Bluray OSD + Train 3:2
		if((scalerVIP_Get_MEMCPatternFlag_Identification(1,11) == TRUE  )&&((u10_gmv_mvy<=5)&&(BOT_ratio==0)))
		{
			WriteRegister(KME_DEHALO_KME_DEHALO_F0_reg, 24, 31, 0x08);
			WriteRegister(KME_DEHALO_KME_DEHALO_F4_reg, 24, 31, 0x80);
			WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x04);
			WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0x10);
	
			WriteRegister(SOFTWARE1_SOFTWARE1_41_reg, 0, 1, 0);//all
			WriteRegister(SOFTWARE1_SOFTWARE1_41_reg, 4, 5, 0);//bottom
			WriteRegister(SOFTWARE1_SOFTWARE1_41_reg, 6, 7, 0);//left
			WriteRegister(SOFTWARE1_SOFTWARE1_41_reg, 8, 9, 0); //right
			//pOutput->u1_mixMode = 0;
			u5_blend_holdfrm_a[18] = 180;
			nIdentifiedNum_a[20]=129;
		}
		else if(u5_blend_holdfrm_a[18] >0)
		{
			WriteRegister(KME_DEHALO_KME_DEHALO_F0_reg, 24, 31, 0x08);
			WriteRegister(KME_DEHALO_KME_DEHALO_F4_reg, 24, 31, 0x80);
			WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x04);
			WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0x10);
	
			WriteRegister(SOFTWARE1_SOFTWARE1_41_reg, 0, 1, 0);//all
			WriteRegister(SOFTWARE1_SOFTWARE1_41_reg, 4, 5, 0);//bottom
			WriteRegister(SOFTWARE1_SOFTWARE1_41_reg, 6, 7, 0);//left
			WriteRegister(SOFTWARE1_SOFTWARE1_41_reg, 8, 9, 0); //right
			//pOutput->u1_mixMode = 0;
	
			u5_blend_holdfrm_a[18]--;
		}
		else if((nIdentifiedNum_a[20] == 129 )||((u10_gmv_mvy>=5)&&(BOT_ratio!=0)))
		{
	
			WriteRegister(SOFTWARE1_SOFTWARE1_41_reg, 0, 1, 1);//all
			WriteRegister(SOFTWARE1_SOFTWARE1_41_reg, 4, 5, 2);//bottom
			WriteRegister(SOFTWARE1_SOFTWARE1_41_reg, 6, 7, 1);//left
			WriteRegister(SOFTWARE1_SOFTWARE1_41_reg, 8, 9, 1); //right
	
			nIdentifiedNum_a[20] = 0;
		}
		//-- YE Test Sony Bluray OSD + Train 3:2
#endif
	
#if Pro_SONY
		//++ YE Test Sony Animal Train HD(Frame In 3:2)
		if(scalerVIP_Get_MEMCPatternFlag_Identification(1,12) == TRUE&&((u10_gmv_mvy<=9)&&(BOT_ratio==0)))
		{
			WriteRegister(KME_DEHALO_KME_DEHALO_F0_reg, 24, 31, 0x08);
			WriteRegister(KME_DEHALO_KME_DEHALO_F4_reg, 24, 31, 0x80);
			WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x04);
			WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0x10);
	
			u5_blend_holdfrm_a[19] = 180;
			nIdentifiedNum_a[21]=130;
		}
		else if(u5_blend_holdfrm_a[19] >0)
		{
			WriteRegister(KME_DEHALO_KME_DEHALO_F0_reg, 24, 31, 0x08);
			WriteRegister(KME_DEHALO_KME_DEHALO_F4_reg, 24, 31, 0x80);
			WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x04);
			WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0x10);
	
			//pOutput->u1_mixMode = 0;
	
			u5_blend_holdfrm_a[19]--;
		}
		else if((nIdentifiedNum_a[21] == 130 )||((u10_gmv_mvy>=5)&&(BOT_ratio!=0)))
		{
	
			nIdentifiedNum_a[21] = 0;
		}
		//-- YE Test Sony Animal Train HD(Frame In 3:2)
#endif
	
#if Pro_SONY
	//++ YE Test Sony #4 22NotDetection(JapanDorama) 22NotDetection(JapanDorama)_Scene01
		//YE Test Sony #4 22NotDetection(JapanDorama)+007 17s
	
	
		if(scalerVIP_Get_MEMCPatternFlag_Identification(1,13) == TRUE)
		{
			//WriteRegister(PQL1_00_ADDR_PQL1_02_ADDR_reg, 26, 26, 0x0);//Merlin7 does not have those register
			u5_blend_holdfrm_a[20] = 60;
			nIdentifiedNum_a[22]=131;
		}
		else if(u5_blend_holdfrm_a[20] >0)
		{
			//WriteRegister(PQL1_00_ADDR_PQL1_02_ADDR_reg, 26, 26, 0x0);//Merlin7 does not have those register
			u5_blend_holdfrm_a[20]--;
		}
		else if(nIdentifiedNum_a[22] == 131 )
		{
			//WriteRegister(PQL1_00_ADDR_PQL1_02_ADDR_reg, 26, 26, 0x1);//Merlin7 does not have those register
	
			nIdentifiedNum_a[22] = 0;
		}
	//-- YE Test Sony #4 22NotDetection(JapanDorama) 22NotDetection(JapanDorama)_Scene01
#endif
	
	
#if Pro_SONY
#endif
	
	
	//++ YE Test sony Dot32_Float32PinballMotion_EXT 20200902
#if Pro_SONY
		if(scalerVIP_Get_MEMCPatternFlag_Identification(1,6) == TRUE  )
		{
	
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_3C_reg, 0,8,0x40);
			WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_E0_reg,0,7, 0x40);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,3,3,0);	//20200904 new add
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,2,2 ,0);  //20200904 new add
	
			u5_blend_holdfrm_a[22] = 60;
			nIdentifiedNum_a[24]=123;
		}
		else if(u5_blend_holdfrm_a[22] >0)
		{
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_3C_reg, 0,8,0x40);
			WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_E0_reg,0,7, 0x40);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,3,3,0);	 //20200904 new add
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,2,2 ,0);  //20200904 new add
	
			u5_blend_holdfrm_a[22]--;
		}
		else if(nIdentifiedNum_a[24] == 123 )
		{
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,3,3,1);	//20200904 new add
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,2,2 ,1); //20200904 new add
	
			nIdentifiedNum_a[24] = 0;
		}
#endif
	//-- YE Test sony Dot32_Float32PinballMotion_EXT 20200902
	
	//++ YE Test sony Periodical_Pattern_[007_Casino_Royale]
#if Pro_SONY
		if(scalerVIP_Get_MEMCPatternFlag_Identification(1,14) == TRUE  )
		{
	
		// This is dynamic ME don't need to write back================================
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 19, 	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 	1);
	
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 19, 	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31,   1);
		//=================================
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_40_reg,9,12,1);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_40_reg,13,16,1);
	
			u5_blend_holdfrm_a[23] = 120;
			nIdentifiedNum_a[25]=132;
		}
		else if(u5_blend_holdfrm_a[23] >0)
		{
	
			// This is dynamic ME don't need to write back================================
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 19, 	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 	1);
	
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 19, 	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31,   1);
			//=================================
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_40_reg,9,12,1);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_40_reg,13,16,1);
	
	
			u5_blend_holdfrm_a[23]--;
		}
		else if(nIdentifiedNum_a[25] == 132 )
		{
	
			WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_40_reg,9,12,3);
			WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_40_reg,13,16,3);
			nIdentifiedNum_a[25] = 0;
		}
#endif
	//-- YE Test sony Periodical_Pattern_[007_Casino_Royale]
	
#if Pro_SONY
	//++ YE Test Sony 22NotDetection(JapanDorama)_Scene02
	
	
		if(scalerVIP_Get_MEMCPatternFlag_Identification(1,15) == TRUE)
		{
			//WriteRegister(PQL1_00_ADDR_PQL1_02_ADDR_reg, 26, 26, 0x0);
			//WriteRegister(PQL1_00_ADDR_PQL1_02_ADDR_reg, 27, 27, 0x0);//Merlin7 does not have those register
			//WriteRegister(PQL1_00_ADDR_PQL1_02_ADDR_reg, 28, 28, 0x0);//Merlin7 does not have those register
			//WriteRegister(PQL1_00_ADDR_PQL1_02_ADDR_reg, 30, 30, 0x0);//Merlin7 does not have those register
			u5_blend_holdfrm_a[20] = 60;
			nIdentifiedNum_a[22]=133;
		}
		else if(u5_blend_holdfrm_a[20] >0)
		{
			//WriteRegister(PQL1_00_ADDR_PQL1_02_ADDR_reg, 26, 26, 0x0);
			//WriteRegister(PQL1_00_ADDR_PQL1_02_ADDR_reg, 27, 27, 0x0);//Merlin7 does not have those register
			//WriteRegister(PQL1_00_ADDR_PQL1_02_ADDR_reg, 28, 28, 0x0);//Merlin7 does not have those register
			//WriteRegister(PQL1_00_ADDR_PQL1_02_ADDR_reg, 30, 30, 0x0);//Merlin7 does not have those register
	
			u5_blend_holdfrm_a[20]--;
		}
		else if(nIdentifiedNum_a[22] == 133 )
		{
			//WriteRegister(PQL1_00_ADDR_PQL1_02_ADDR_reg, 26, 26, 0x1);
			//WriteRegister(PQL1_00_ADDR_PQL1_02_ADDR_reg, 27, 27, 0x1);//Merlin7 does not have those register
			//WriteRegister(PQL1_00_ADDR_PQL1_02_ADDR_reg, 28, 28, 0x1);//Merlin7 does not have those register
			//WriteRegister(PQL1_00_ADDR_PQL1_02_ADDR_reg, 30, 30, 0x1);//Merlin7 does not have those register
	
			nIdentifiedNum_a[22] = 0;
		}
	//-- YE Test Sony 22NotDetection(JapanDorama)_Scene02
#endif
	
	
	//++YE Test Sony new issue ultraviolet_1
	#if Pro_SONY
		if(scalerVIP_Get_MEMCPatternFlag_Identification(1,16) == TRUE )
		{
			//s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
			//s_pContext->_output_wrt_comreg.u1_KME_gmvd_cost_type=1;
			WriteRegister(KME_DEHALO_KME_DEHALO_F0_reg, 24, 31, 0x00);
			WriteRegister(KME_DEHALO_KME_DEHALO_F4_reg, 24, 31, 0x00);
			WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x00);
			WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0x00);
	
			u5_blend_holdfrm_a[21] = 200;
			nIdentifiedNum_a[23]=134;
		}
		else if(u5_blend_holdfrm_a[21] >0)
		{
			WriteRegister(KME_DEHALO_KME_DEHALO_F0_reg, 24, 31, 0x00);
			  WriteRegister(KME_DEHALO_KME_DEHALO_F4_reg, 24, 31, 0x00);
			  WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x00);
			  WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0x00);
	
			u5_blend_holdfrm_a[21]--;
		}
		else if(nIdentifiedNum_a[23] == 134)
		{
	
			nIdentifiedNum_a[23] = 0;
		}
	#endif
	//--YE Test Sony new issue ultraviolet_1
	
	
	//++YE Test Sony Netflix 1:57:02 ~	1:57:06
	#if Pro_SONY
		if(scalerVIP_Get_MEMCPatternFlag_Identification(1,17) == TRUE )
		{
			pOutput->u1_TV002_Netflix_flag=1;
			//s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
			//s_pContext->_output_wrt_comreg.u1_KME_gmvd_cost_type=1;
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_10_reg, 0, 7, 0x00);  //gain0
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_10_reg, 8, 15, 0x00); //gain1
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_10_reg, 16, 23, 0x64); //gain2
	
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_14_reg, 0, 3, 0x1); //mode
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_14_reg, 4, 13, 0xC8); //limit
	
	
			u5_blend_holdfrm_a[22] = 100;
			nIdentifiedNum_a[24]=135;
		}
		else if(u5_blend_holdfrm_a[22] >0)
		{
			pOutput->u1_TV002_Netflix_flag=1;
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_10_reg, 0, 7, 0x00);  //gain0
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_10_reg, 8, 15, 0x00); //gain1
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_10_reg, 16, 23, 0x64); //gain2
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_14_reg, 0, 3, 0x1); //mode
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_14_reg, 4, 13, 0xC8); //limit
	
			u5_blend_holdfrm_a[22]--;
		}
		else if(nIdentifiedNum_a[24] == 135)
		{
			pOutput->u1_TV002_Netflix_flag=0;
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_10_reg, 0, 7, 0x40);  //gain0
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_10_reg, 8, 15, 0x40); //gain1
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_10_reg, 16, 23, 0x40); //gain2
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_14_reg, 0, 3, 0x0); //mode
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_14_reg, 4, 13, 0x30); //limit
	
			nIdentifiedNum_a[24] = 0;
		}
	#endif
	//--YE Test Sony Netflix 1:57:02 ~	1:57:06

#if 1
	//===debug show
	for(u8_Index =0;u8_Index<255;u8_Index++)
	{
		if(nIdentifiedNum_a[u8_Index] !=0){
			pOutput->u1_IP_wrtAction_true=1;
			if(u32_ID_Log_en==1)
				rtd_pr_memc_emerg("[IPR_002]nIdentifiedNum_a>>[%d][%d]   ,\n", u8_Index, nIdentifiedNum_a[u8_Index]);
		}
	}
#endif
}

VOID Identification_Pattern_wrtAction_TV006(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{

#if 0
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	//_PQLPARAMETER *PatternParam = GetPQLParameter();
	static unsigned char u5_blend_holdfrm_a[255] = {0};
	static unsigned char nIdentifiedNum_a[255] = {0};
	//unsigned int mot_diff = s_pContext->_output_filmDetectctrl.u27_ipme_motionPool[_FILM_ALL][0];
	//static unsigned char nOnceFlag=12;
	unsigned char u8_Index;
	//static bool monosco_flag = 0;
	
	//unsigned int u32_RB_Value;
	//signed short u11_gmv_mvx = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
	//signed short u10_gmv_mvy = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;
	//unsigned int u25_me_aDTL = s_pContext->_output_read_comreg.u25_me_aDTL_rb;
	//unsigned int u26_aAPLi_rb = s_pContext->_output_read_comreg.u26_me_aAPLi_rb;
	//unsigned int u32_pattern298_zidoo_player_forbindden_en;
	
	//unsigned char  u8_cadence_Id = s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL];
	//unsigned char  u8_Mixmode_flag = s_pContext->_output_filmDetectctrl.u1_mixMode;
	//int gmv_mvx =0;//, gmv_mvy =0 ;
	//PQL_INPUT_FRAME_RATE in_fmRate = s_pContext->_external_data._input_frameRate;
	
	
	unsigned int u32_ID_Log_en=0;//,ES_Test_Value=0;
	ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 24,  24, &u32_ID_Log_en);//YE Test debug dummy register
	
	if((pParam->u1_Identification_Pattern_en== 0)||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_Test_0_flag ==0))
		return;
	
	//reset
	pOutput->u1_IP_wrtAction_true=0;
	
#if 0
{
	static unsigned int me1_ip_cddpnt_st7 = 0;
	static unsigned int me1_pi_cddpnt_st7 = 0;
	//PQ_Issue_SAN_ANDREAS_01_repeat.mp4 KTASKWBS-22870
	if(scalerVIP_Get_MEMCPatternFlag_Identification(2,1) == TRUE && s_pContext->_output_me_sceneAnalysis.u1_SAN_ANDREAS_DRP_detect_true>0)		
	{
		if(nIdentifiedNum_a[1]!=1)
		{
			ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg, 24,31, &me1_ip_cddpnt_st7);
			ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg, 24,31, &me1_pi_cddpnt_st7);
		}
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg,24,31,200);//me1_ip_cddpnt_st3
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,0,7,200);//me1_ip_cddpnt_st4
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,8,15,200);//me1_ip_cddpnt_st5
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,16,23,200);//me1_ip_cddpnt_st6
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,24,31,200);//me1_ip_cddpnt_st7
		
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg,24,31,200);//me1_pi_cddpnt_st3
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,0,7,200);//me1_pi_cddpnt_st4
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,8,15,200);//me1_pi_cddpnt_st5
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,16,23,200);//me1_pi_cddpnt_st6
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,24,31,200);//me1_pi_cddpnt_st7
		u5_blend_holdfrm_a[1] = 180;
		nIdentifiedNum_a[1]=1;
	}
	else if(u5_blend_holdfrm_a[1] > 0)
	{
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg,24,31,200);//me1_ip_cddpnt_st3
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,0,7,200);//me1_ip_cddpnt_st4
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,8,15,200);//me1_ip_cddpnt_st5
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,16,23,200);//me1_ip_cddpnt_st6
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,24,31,200);//me1_ip_cddpnt_st7
		
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg,24,31,200);//me1_pi_cddpnt_st3
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,0,7,200);//me1_pi_cddpnt_st4
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,8,15,200);//me1_pi_cddpnt_st5
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,16,23,200);//me1_pi_cddpnt_st6
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,24,31,200);//me1_pi_cddpnt_st7
		u5_blend_holdfrm_a[1]--;
	}
	else if(nIdentifiedNum_a[1]==1)
	{
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,24,31,me1_ip_cddpnt_st7);//me1_ip_cddpnt_st7
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,24,31,me1_pi_cddpnt_st7);//me1_pi_cddpnt_st7
		nIdentifiedNum_a[1]=0;
	}
}
#endif

#if 0 //K24014_v4
{
	kme_logo0_kme_logo0_d8_RBUS kme_logo0_kme_logo0_d8_reg;
	kme_logo0_kme_logo0_e4_RBUS kme_logo0_kme_logo0_e4_reg;
	kme_logo0_kme_logo0_d8_reg.regValue = rtd_inl(KME_LOGO0_KME_LOGO0_D8_reg);
	kme_logo0_kme_logo0_e4_reg.regValue = rtd_inl(KME_LOGO0_KME_LOGO0_E4_reg);
	if(scalerVIP_Get_MEMCPatternFlag_Identification(2,2) == TRUE && logo_check_K24014_flag == 1 )		
	{
		kme_logo0_kme_logo0_d8_reg.km_logo_blkhsty_nstep_y2 = 0x1;
		kme_logo0_kme_logo0_d8_reg.km_logo_blkhsty_nstep_y1 = 0x1;
		kme_logo0_kme_logo0_d8_reg.km_logo_blkhsty_nstep_y0 = 0x1;
		rtd_outl(KME_LOGO0_KME_LOGO0_D8_reg, kme_logo0_kme_logo0_d8_reg.regValue);
		kme_logo0_kme_logo0_e4_reg.km_logo_blkhsty_pstep_y2 = 0xf;
		kme_logo0_kme_logo0_e4_reg.km_logo_blkhsty_pstep_y1 = 0xf;	
		kme_logo0_kme_logo0_e4_reg.km_logo_blkhsty_pstep_y0 = 0xf;
		rtd_outl(KME_LOGO0_KME_LOGO0_E4_reg, kme_logo0_kme_logo0_e4_reg.regValue);
		u5_blend_holdfrm_a[2] = 180;
		nIdentifiedNum_a[2]=2;
	}
	else if(u5_blend_holdfrm_a[2] > 0)
	{
		kme_logo0_kme_logo0_d8_reg.km_logo_blkhsty_nstep_y2 = 0x1;
		kme_logo0_kme_logo0_d8_reg.km_logo_blkhsty_nstep_y1 = 0x1;
		kme_logo0_kme_logo0_d8_reg.km_logo_blkhsty_nstep_y0 = 0x1;
		rtd_outl(KME_LOGO0_KME_LOGO0_D8_reg, kme_logo0_kme_logo0_d8_reg.regValue);
		kme_logo0_kme_logo0_e4_reg.km_logo_blkhsty_pstep_y2 = 0xf;
		kme_logo0_kme_logo0_e4_reg.km_logo_blkhsty_pstep_y1 = 0xf;
		kme_logo0_kme_logo0_e4_reg.km_logo_blkhsty_pstep_y0 = 0xf;
		rtd_outl(KME_LOGO0_KME_LOGO0_E4_reg, kme_logo0_kme_logo0_e4_reg.regValue);
		u5_blend_holdfrm_a[2]--;
	}
	else if(nIdentifiedNum_a[2]==2 )
	{
		kme_logo0_kme_logo0_d8_reg.km_logo_blkhsty_nstep_y2 = 0x8;
		kme_logo0_kme_logo0_d8_reg.km_logo_blkhsty_nstep_y1 = 0x5;
		kme_logo0_kme_logo0_d8_reg.km_logo_blkhsty_nstep_y0 = 0x0;
		rtd_outl(KME_LOGO0_KME_LOGO0_D8_reg, kme_logo0_kme_logo0_d8_reg.regValue);
		kme_logo0_kme_logo0_e4_reg.km_logo_blkhsty_pstep_y2 = 0x5;//0x4;
		kme_logo0_kme_logo0_e4_reg.km_logo_blkhsty_pstep_y1 = 0x3;//0x2;
		kme_logo0_kme_logo0_e4_reg.km_logo_blkhsty_pstep_y0 = 0x0;
		rtd_outl(KME_LOGO0_KME_LOGO0_E4_reg, kme_logo0_kme_logo0_e4_reg.regValue);
		nIdentifiedNum_a[2]=0;
	}
}
#endif
#if 1
	//===debug show
	for(u8_Index =0;u8_Index<255;u8_Index++)
	{
		if(nIdentifiedNum_a[u8_Index] !=0){
			pOutput->u1_IP_wrtAction_true=1;
			if(u32_ID_Log_en==1)
				rtd_pr_memc_emerg("[IPR_006]nIdentifiedNum_a>>[%d][%d]	 ,\n", u8_Index, nIdentifiedNum_a[u8_Index]);
		}
	}
#endif

#endif
}

VOID Identification_Pattern_wrtAction_TV010(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	_PQLCONTEXT *s_pContext_pre = GetPQLContext_m();
	//_PQLPARAMETER *PatternParam = GetPQLParameter();
	static unsigned char u5_blend_holdfrm_a[255] = {0};
	static unsigned char nIdentifiedNum_a[255] = {0};
	//unsigned int mot_diff = s_pContext->_output_filmDetectctrl.u27_ipme_motionPool[_FILM_ALL][0];
	//static unsigned char nOnceFlag=12;
	unsigned char u8_Index;
	unsigned int deleteVal = 0;
	unsigned int special_pattern_transporter;

	//unsigned int u32_RB_Value;
	signed short u11_gmv_mvx = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
	signed short u10_gmv_mvy = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;
	unsigned int u25_me_aDTL = s_pContext->_output_read_comreg.u25_me_aDTL_rb;
	unsigned short u12_gmv_unconf = s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb;
	PQL_INPUT_FRAME_RATE in_fmRate = s_pContext->_external_data._input_frameRate;
	PQL_OUTPUT_FRAME_RATE out_fmRate = s_pContext->_external_data._output_frameRate;
	unsigned char u1_still_frame=s_pContext->_output_frc_sceneAnalysis.u1_still_frame;
	//unsigned int u26_aAPLi_rb = s_pContext->_output_read_comreg.u26_me_aAPLi_rb;
	//unsigned int u32_pattern298_zidoo_player_forbindden_en;

	//unsigned char  u8_cadence_Id = s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL];
	//unsigned char  u8_Mixmode_flag = s_pContext->_output_filmDetectctrl.u1_mixMode;
    //int gmv_mvx =0;//, gmv_mvy =0 ;

#if 1 //for YE Test temp test
	static unsigned int u32_ID_Log_en=0;//,ES_Test_Value=0;
	ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 24,  24, &u32_ID_Log_en);//YE Test debug dummy register
#endif
	ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 18, 18, &special_pattern_transporter);

#if 1
	{
		deleteVal = rtd_inl(HARDWARE_HARDWARE_43_reg);//0xB809D5AC
		if(deleteVal&0x8000) {//bit15
			rtd_pr_memc_emerg("mvx:%d mvy:%d sf:%d aDTL:%u unconf:%d in:%d out:%d\n", 
			u11_gmv_mvx, u10_gmv_mvy, 
			u1_still_frame, u25_me_aDTL, u12_gmv_unconf,
			in_fmRate, out_fmRate);
		}
	}
#endif


	if((pParam->u1_Identification_Pattern_en== 0)||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_Test_0_flag ==0))
		return;

	//reset
	pOutput->u1_IP_wrtAction_true=0;

	//--#60017 下边界破碎1 7s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,2) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 )&&
		(u25_me_aDTL>2300000&&u25_me_aDTL<23000000))
	{
		//WriteRegister(KME_ALGO_ME1_MEDFLT_CTRL_00_reg,18,18,0x1);
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		u5_blend_holdfrm_a[2] = 20;
		nIdentifiedNum_a[2]=2;
	}
	else if(u5_blend_holdfrm_a[2] > 0)
	{
		//WriteRegister(KME_ALGO_ME1_MEDFLT_CTRL_00_reg,18,18,0x1);
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		u5_blend_holdfrm_a[2]--;
	}
	else if(nIdentifiedNum_a[2] == 2 )
	{
		//WriteRegister(KME_ALGO_ME1_MEDFLT_CTRL_00_reg,18,18,0x1);
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x1);
		nIdentifiedNum_a[2] = 0;
	}
	
	//--#003_rotterdam_logo 17s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,3) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
		{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);		// local FB
		WriteRegister(MC_MC_28_reg,14,14,0x1);				// local FB
		WriteRegister(MC_MC_28_reg,15,22,0x0);				// local FB
		u5_blend_holdfrm_a[3] = 20;
			nIdentifiedNum_a[3]=3;
		}
		else if(u5_blend_holdfrm_a[3] > 0)
		{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);		// local FB
		WriteRegister(MC_MC_28_reg,14,14,0x1);				// local FB
		WriteRegister(MC_MC_28_reg,15,22,0x0);				// local FB
		u5_blend_holdfrm_a[3]--;
	}
	else if(nIdentifiedNum_a[3] == 3 )
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);		// local FB
		WriteRegister(MC_MC_28_reg,14,14,0x0);				// local FB
		WriteRegister(MC_MC_28_reg,15,22,0x0);				// local FB
		nIdentifiedNum_a[3] = 0;
	}

	//--#7002 一个桶_25Hz_淡入淡出 5:46
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,4) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		u5_blend_holdfrm_a[4] = 20;
		nIdentifiedNum_a[4]=4;
	}
	else if(u5_blend_holdfrm_a[4] > 0)
	{
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		u5_blend_holdfrm_a[4]--;
	}
	else if(nIdentifiedNum_a[4] == 4 )
	{
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x1);
		nIdentifiedNum_a[4] = 0;
	}

	//--#2049
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,5) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 )
		&&((in_fmRate==_PQL_IN_50HZ && out_fmRate==_PQL_OUT_100HZ)||(in_fmRate==_PQL_IN_60HZ && out_fmRate==_PQL_OUT_120HZ)||(in_fmRate==_PQL_IN_30HZ && out_fmRate==_PQL_OUT_120HZ)))
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);
		WriteRegister(MC_MC_28_reg,14,14,0x1);
		WriteRegister(MC_MC_28_reg,15,22,0x0);

		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 26, 0x502814);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 26, 0x4c2814);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 26, 0x502813);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 26, 0x502814);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 17, 0x2613);

		if(in_fmRate==_PQL_IN_50HZ && out_fmRate==_PQL_OUT_100HZ) {
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x55555555);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x55555555);
		}
		else if(in_fmRate==_PQL_IN_60HZ && out_fmRate==_PQL_OUT_120HZ) {
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0xbbbbbbbb);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0xbbbbbbbb);
		}
		else if(in_fmRate==_PQL_IN_30HZ && out_fmRate==_PQL_OUT_120HZ) {
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		}
		
		u5_blend_holdfrm_a[5] = 50;
		nIdentifiedNum_a[5]=5;
	}
	else if(u5_blend_holdfrm_a[5] > 0)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);
		WriteRegister(MC_MC_28_reg,14,14,0x1);
		WriteRegister(MC_MC_28_reg,15,22,0x0);

		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
		
		if(in_fmRate==_PQL_IN_50HZ && out_fmRate==_PQL_OUT_100HZ) {
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x55555555);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x55555555);
		}
		else if(in_fmRate==_PQL_IN_60HZ && out_fmRate==_PQL_OUT_120HZ) {
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0xbbbbbbbb);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0xbbbbbbbb);
		}
		else if(in_fmRate==_PQL_IN_30HZ && out_fmRate==_PQL_OUT_120HZ) {
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		}
		u5_blend_holdfrm_a[5]--;
	}
	else if(nIdentifiedNum_a[5] == 5)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);
		WriteRegister(MC_MC_28_reg,14,14,0x0);
		WriteRegister(MC_MC_28_reg,15,22,0x0);

		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 64);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 48);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 32);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 16);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 64);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 48);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 32);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 16);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x77777777);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x77777777);

		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x1);
		nIdentifiedNum_a[5] = 0;
	}

	//--#60028自然世界-鹰之力2
	if( scalerVIP_Get_MEMCPatternFlag_Identification(4,6) == TRUE && scalerVIP_Get_MEMCPatternFlag_Identification(4,17) == FALSE &&
		scalerVIP_Get_MEMCPatternFlag_Identification(4,18) == FALSE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 )
		&& u25_me_aDTL<=1300000 && ( u11_gmv_mvx<-200 || (u11_gmv_mvx>=-1 && u11_gmv_mvx<=1 && u10_gmv_mvy>=-1 && u10_gmv_mvy<=1) ) )
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
		
		WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x1);
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 255);	 
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 230);
		WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0);
		
		WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 0);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 0);

		WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0x0);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x11111111);
			
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);		// local FB
		WriteRegister(MC_MC_28_reg,14,14,0x1);				// local FB
		WriteRegister(MC_MC_28_reg,15,22,0x0);				// local FB

		u5_blend_holdfrm_a[6] = 200;
		nIdentifiedNum_a[6]=6;
	}
	else if(u5_blend_holdfrm_a[6] > 0)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
			
		WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x1);
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 255);	 
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 230);
		WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0);

		WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 0);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 0);

		WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0x0);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x11111111);

		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);		// local FB
		WriteRegister(MC_MC_28_reg,14,14,0x1);				// local FB
		WriteRegister(MC_MC_28_reg,15,22,0x0);				// local FB

		u5_blend_holdfrm_a[6]--;
	}
	else if(nIdentifiedNum_a[6] == 6 )
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 64);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 48);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 32);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 16);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 64);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 48);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 32);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 16);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0);
		
		WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x0);
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 0xff);	  
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 0x0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 1);

		WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 1);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 1);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 1);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 1);

		WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0xffffffff);
		WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0xffffffff);
		WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0xffffffff);
		WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0xffffffff);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x77777777);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x77777777);
		
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);		// local FB
		WriteRegister(MC_MC_28_reg,14,14,0x0);				// local FB
		WriteRegister(MC_MC_28_reg,15,22,0x0);				// local FB
		nIdentifiedNum_a[6] = 0;
	}

	//--#8004_脱口秀大会 0s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,7) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);		// local FB
		WriteRegister(MC_MC_28_reg,14,14,0x1);				// local FB
		WriteRegister(MC_MC_28_reg,15,22,0x0);				// local FB
		WriteRegister(KME_LOGO0_KME_LOGO0_64_reg,0,7,255);
		WriteRegister(KME_LOGO0_KME_LOGO0_64_reg,8,15,255);
		WriteRegister(KME_LOGO0_KME_LOGO0_64_reg,16,23,255);
		WriteRegister(KME_LOGO0_KME_LOGO0_64_reg,24,31,255);
		u5_blend_holdfrm_a[7] = 20;
		nIdentifiedNum_a[7]=7;
	}
	else if(u5_blend_holdfrm_a[7] > 0)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);		// local FB
		WriteRegister(MC_MC_28_reg,14,14,0x1);				// local FB
		WriteRegister(MC_MC_28_reg,15,22,0x0);				// local FB
		WriteRegister(KME_LOGO0_KME_LOGO0_64_reg,0,7,255);
		WriteRegister(KME_LOGO0_KME_LOGO0_64_reg,8,15,255);
		WriteRegister(KME_LOGO0_KME_LOGO0_64_reg,16,23,255);
		WriteRegister(KME_LOGO0_KME_LOGO0_64_reg,24,31,255);		
		u5_blend_holdfrm_a[7]--;
	}
	else if(nIdentifiedNum_a[7] == 7 )
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);		// local FB
		WriteRegister(MC_MC_28_reg,14,14,0x0);				// local FB
		WriteRegister(MC_MC_28_reg,15,22,0x0);				// local FB
		WriteRegister(KME_LOGO0_KME_LOGO0_64_reg,0,7,3);
		WriteRegister(KME_LOGO0_KME_LOGO0_64_reg,8,15,6);
		WriteRegister(KME_LOGO0_KME_LOGO0_64_reg,16,23,4);
		WriteRegister(KME_LOGO0_KME_LOGO0_64_reg,24,31,11);
		nIdentifiedNum_a[7] = 0;
	}

	//--#8004_脱口秀大会 5s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,8) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);		// local FB
		WriteRegister(MC_MC_28_reg,14,14,0x1);				// local FB
		WriteRegister(MC_MC_28_reg,15,22,0x0);				// local FB
		u5_blend_holdfrm_a[8] = 20;
		nIdentifiedNum_a[8]=8;
	}
	else if(u5_blend_holdfrm_a[8] > 0)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);		// local FB
		WriteRegister(MC_MC_28_reg,14,14,0x1);				// local FB
		WriteRegister(MC_MC_28_reg,15,22,0x0);				// local FB
		u5_blend_holdfrm_a[8]--;
	}
	else if(nIdentifiedNum_a[8] == 8 )
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);		// local FB
		WriteRegister(MC_MC_28_reg,14,14,0x0);				// local FB
		WriteRegister(MC_MC_28_reg,15,22,0x0);				// local FB
		nIdentifiedNum_a[8] = 0;
	}

	//--#C002samll object handing
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,9) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_1_reg,28,28,0x1);
		u5_blend_holdfrm_a[9] = 20;
		nIdentifiedNum_a[9]=9;
	}
	else if(u5_blend_holdfrm_a[9] > 0)
	{
		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_1_reg,28,28,0x1);
		u5_blend_holdfrm_a[9]--;
	}
	else if(nIdentifiedNum_a[9] == 9 )
	{
		WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_1_reg,28,28,0x0);
		nIdentifiedNum_a[9] = 0;
	}

	//--#4K023transporter_explosion
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,10) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 )&& special_pattern_transporter!=1)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);		// local FB
		WriteRegister(MC_MC_28_reg,14,14,0x1);				// local FB
		WriteRegister(MC_MC_28_reg,15,22,0x0);				// local FB
		u5_blend_holdfrm_a[10] = 20;
		nIdentifiedNum_a[10]=10;
	}
	else if(u5_blend_holdfrm_a[10] > 0)
	{
		if(special_pattern_transporter!=1){
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);		// local FB
		WriteRegister(MC_MC_28_reg,14,14,0x1);				// local FB
		WriteRegister(MC_MC_28_reg,15,22,0x0);				// local FB
		}
		else {
			WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);		// local FB
			WriteRegister(MC_MC_28_reg,14,14,0x0);				// local FB
			WriteRegister(MC_MC_28_reg,15,22,0x0);				// local FB
		}
		u5_blend_holdfrm_a[10]--;
	}
	else if(nIdentifiedNum_a[10] == 10 )
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);		// local FB
		WriteRegister(MC_MC_28_reg,14,14,0x0);				// local FB
		WriteRegister(MC_MC_28_reg,15,22,0x0);				// local FB
		nIdentifiedNum_a[10] = 0;
	}

	//--#60027自然世界-鹰之力1
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,11) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KME_IPME_KME_IPME_2C_reg,10,17,8);
		u5_blend_holdfrm_a[11] = 60;
		nIdentifiedNum_a[11]=11;
	}
	else if(u5_blend_holdfrm_a[11] > 0)
	{
		WriteRegister(KME_IPME_KME_IPME_2C_reg,10,17,8);
		u5_blend_holdfrm_a[11]--;
	}
	else if(nIdentifiedNum_a[11] == 11 )
	{
		WriteRegister(KME_IPME_KME_IPME_2C_reg,10,17,16);
		nIdentifiedNum_a[11] = 0;
	}

	//--#broken sutitle
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,12) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 )&&
		MEMC_Lib_get_Adaptive_Stream_Flag()==1 && s_pContext->_external_data._input_frameRate==_PQL_IN_25HZ && 
		u12_gmv_unconf>=6 && u12_gmv_unconf<=27 && u1_still_frame == 0 )
	{
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 6, 8,7);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg,22,24,7);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 0, 3,0x1);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 4, 7,0x1);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 8,11,0x1);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 0, 3,0xf);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 4, 7,0xf);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 8,11,0xf);
		WriteRegister(MC2_MC2_20_reg,6,9,2);
		WriteRegister(LBMC_LBMC_88_reg,24,25,2);	
		u5_blend_holdfrm_a[12] = 50;
		nIdentifiedNum_a[12]=12;
	}
	else if(u5_blend_holdfrm_a[12] > 0)
	{
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 6, 8,7);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg,22,24,7);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 0, 3,0x1);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 4, 7,0x1);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 8,11,0x1);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 0, 3,0xf);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 4, 7,0xf);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 8,11,0xf);
		WriteRegister(MC2_MC2_20_reg,6,9,2);
		WriteRegister(LBMC_LBMC_88_reg,24,25,2);
		u5_blend_holdfrm_a[12]--;
	}
	else if(nIdentifiedNum_a[12] == 12)
	{
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 6, 8,4);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg,22,24,4);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 0, 3,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 4, 7,0x5);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 8,11,0x8);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 0, 3,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 4, 7,0x3);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 8,11,0x5);
		WriteRegister(MC2_MC2_20_reg,6,9,0);
		WriteRegister(LBMC_LBMC_88_reg,24,25,0);
		nIdentifiedNum_a[12] = 0;
	}

	//--#broken subtitle八百 1:00:15
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,13) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 )&&
		MEMC_Lib_get_Adaptive_Stream_Flag()==1 && s_pContext->_external_data._input_frameRate==_PQL_IN_24HZ &&
		u10_gmv_mvy>=-15 && u10_gmv_mvy<=3 && u1_still_frame==0 && u25_me_aDTL>=175000 && u25_me_aDTL<=260000 && u12_gmv_unconf<=20)
	{
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 6, 8,7);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg,22,24,7);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 0, 3,0x1);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 4, 7,0x1);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 8,11,0x1);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 0, 3,0xf);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 4, 7,0xf);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 8,11,0xf);
		WriteRegister(MC2_MC2_20_reg,6,9,2);
		WriteRegister(LBMC_LBMC_88_reg,24,25,2);	
		u5_blend_holdfrm_a[13] = 70;
		nIdentifiedNum_a[13]=13;
	}
	else if(u5_blend_holdfrm_a[13] > 0)
	{
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 6, 8,7);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg,22,24,7);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 0, 3,0x1);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 4, 7,0x1);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 8,11,0x1);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 0, 3,0xf);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 4, 7,0xf);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 8,11,0xf);
		WriteRegister(MC2_MC2_20_reg,6,9,2);
		WriteRegister(LBMC_LBMC_88_reg,24,25,2);
		u5_blend_holdfrm_a[13]--;
	}
	else if(nIdentifiedNum_a[13] == 13)
	{
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 6, 8,4);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg,22,24,4);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 0, 3,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 4, 7,0x5);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 8,11,0x8);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 0, 3,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 4, 7,0x3);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 8,11,0x5);
		WriteRegister(MC2_MC2_20_reg,6,9,0);
		WriteRegister(LBMC_LBMC_88_reg,24,25,0);
		nIdentifiedNum_a[13] = 0;
	}

	//--#broken subtitle 霹靂嬌娃_伊斯坦堡
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,14) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 )&&
		MEMC_Lib_get_Adaptive_Stream_Flag()==1 && s_pContext->_external_data._input_frameRate==_PQL_IN_24HZ &&
		u10_gmv_mvy>=-4 && u10_gmv_mvy<=3 && u1_still_frame==0 && u25_me_aDTL>=440000 && u25_me_aDTL<=1040000 && u12_gmv_unconf<=30)
	{
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 0, 3,0x2);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 4, 7,0xa);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 8,11,0xf);
		WriteRegister(MC2_MC2_20_reg,6,9,2);
		WriteRegister(LBMC_LBMC_88_reg,24,25,2);
		u5_blend_holdfrm_a[14] = 50;
		nIdentifiedNum_a[14]=14;
	}
	else if(u5_blend_holdfrm_a[14] > 0)
	{
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 0, 3,0x2);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 4, 7,0xa);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 8,11,0xf);
		WriteRegister(MC2_MC2_20_reg,6,9,2);
		WriteRegister(LBMC_LBMC_88_reg,24,25,2);
		u5_blend_holdfrm_a[14]--;
	}
	else if(nIdentifiedNum_a[14] == 14)
	{
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 0, 3,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 4, 7,0x3);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 8,11,0x5);
		WriteRegister(MC2_MC2_20_reg,6,9,0);
		WriteRegister(LBMC_LBMC_88_reg,24,25,0);
		nIdentifiedNum_a[14] = 0;
	}

	//--#broken subtitle 霹靂嬌娃 1:01:00
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,15) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 )&&
		MEMC_Lib_get_Adaptive_Stream_Flag()==1 && s_pContext->_external_data._input_frameRate==_PQL_IN_24HZ &&
		u11_gmv_mvx>=-1 && u11_gmv_mvx<=16 && u1_still_frame==0 && u25_me_aDTL>=200000 && u25_me_aDTL<=550000 && u12_gmv_unconf<=32)
	{
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 6, 8,7);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg,22,24,7);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 0, 3,0x1);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 4, 7,0x1);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 8,11,0x1);
		WriteRegister(MC2_MC2_20_reg,6,9,2);
		WriteRegister(LBMC_LBMC_88_reg,24,25,2);
		u5_blend_holdfrm_a[15] = 50;
		nIdentifiedNum_a[15]=15;
	}
	else if(u5_blend_holdfrm_a[15] > 0)
	{
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 6, 8,7);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg,22,24,7);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 0, 3,0x1);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 4, 7,0x1);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 8,11,0x1);
		WriteRegister(MC2_MC2_20_reg,6,9,2);
		WriteRegister(LBMC_LBMC_88_reg,24,25,2);
		u5_blend_holdfrm_a[15]--;
	}
	else if(nIdentifiedNum_a[15] == 15)
	{
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 6, 8,4);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg,22,24,4);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 0, 3,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 4, 7,0x5);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 8,11,0x8);
		WriteRegister(MC2_MC2_20_reg,6,9,0);
		WriteRegister(LBMC_LBMC_88_reg,24,25,0);
		nIdentifiedNum_a[15] = 0;
	}

	//--#broken subtitle八百 1:00:24
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,16) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 )&&
		MEMC_Lib_get_Adaptive_Stream_Flag()==1 && s_pContext->_external_data._input_frameRate==_PQL_IN_24HZ )
	{
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 6, 8,7);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg,22,24,7);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 0, 3,0x1);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 4, 7,0x1);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 8,11,0x1);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 0, 3,0xf);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 4, 7,0xf);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 8,11,0xf);
		WriteRegister(MC2_MC2_20_reg,6,9,2);
		WriteRegister(LBMC_LBMC_88_reg,24,25,2);	
		u5_blend_holdfrm_a[16] = 50;
		nIdentifiedNum_a[16]=16;
	}
	else if(u5_blend_holdfrm_a[16] > 0)
	{
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 6, 8,7);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg,22,24,7);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 0, 3,0x1);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 4, 7,0x1);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 8,11,0x1);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 0, 3,0xf);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 4, 7,0xf);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 8,11,0xf);
		WriteRegister(MC2_MC2_20_reg,6,9,2);
		WriteRegister(LBMC_LBMC_88_reg,24,25,2);
		u5_blend_holdfrm_a[16]--;
	}
	else if(nIdentifiedNum_a[16] == 16)
	{
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 6, 8,4);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg,22,24,4);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 0, 3,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 4, 7,0x5);
		WriteRegister(KME_LOGO0_KME_LOGO0_D8_reg, 8,11,0x8);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 0, 3,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 4, 7,0x3);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg, 8,11,0x5);
		WriteRegister(MC2_MC2_20_reg,6,9,0);
		WriteRegister(LBMC_LBMC_88_reg,24,25,0);
		nIdentifiedNum_a[16] = 0;
	}

	//--#土耳其 16s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,17) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ) &&
		u11_gmv_mvx>=-2&&u11_gmv_mvx<=0&&u10_gmv_mvy==0&&u1_still_frame==0&&u12_gmv_unconf==0&& u25_me_aDTL>=1700000 )
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 26, 0x502814);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 26, 0x4c2814);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 26, 0x502813);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 26, 0x502814);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 17, 0x2613);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x33333333);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x33333333);
		WriteRegister(KME_ALGO_ME1_MEDFLT_CTRL_00_reg, 18,18,0);
		u5_blend_holdfrm_a[17] = 60;
		nIdentifiedNum_a[17]=17;
	}
	else if(u5_blend_holdfrm_a[17] > 0)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 26, 0x502814);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 26, 0x4c2814);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 26, 0x502813);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 26, 0x502814);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 17, 0x2613);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x33333333);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x33333333);
		WriteRegister(KME_ALGO_ME1_MEDFLT_CTRL_00_reg, 18,18,0);
		u5_blend_holdfrm_a[17]--;
	}
	else if(nIdentifiedNum_a[17] == 17)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 26, 0x806040);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 26, 0x10);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 26, 0xc08000);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 26, 0x2020);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 17, 0x0);
		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x77777777);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x77777777);
		WriteRegister(KME_ALGO_ME1_MEDFLT_CTRL_00_reg, 18,18,1);
		nIdentifiedNum_a[17] = 0;
	}

	//--#土耳其 23s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,18) == TRUE && scalerVIP_Get_MEMCPatternFlag_Identification(4,17) == FALSE &&
		( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ) &&
		u11_gmv_mvx>=0&&u11_gmv_mvx<=1&&u10_gmv_mvy==0&&u1_still_frame==0&&u12_gmv_unconf==0&&u25_me_aDTL>=1800000)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 26, 0x502814);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 26, 0x4c2814);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 26, 0x502813);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 26, 0x502814);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 17, 0x2613);
		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x33333333);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x33333333);
		WriteRegister(KME_ALGO_ME1_MEDFLT_CTRL_00_reg, 18,18,0);
		u5_blend_holdfrm_a[18] = 20;
		nIdentifiedNum_a[18]=18;
	}
	else if(u5_blend_holdfrm_a[18] > 0)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 26, 0x502814);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 26, 0x4c2814);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 26, 0x502813);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 26, 0x502814);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 17, 0x2613);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x33333333);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x33333333);
		WriteRegister(KME_ALGO_ME1_MEDFLT_CTRL_00_reg, 18,18,0);
		u5_blend_holdfrm_a[18]--;
	}
	else if(nIdentifiedNum_a[18] == 18)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 26, 0x806040);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 26, 0x10);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 26, 0xc08000);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 26, 0x2020);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 17, 0x0);
		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x77777777);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x77777777);
		WriteRegister(KME_ALGO_ME1_MEDFLT_CTRL_00_reg, 18,18,1);
		nIdentifiedNum_a[18] = 0;
	}

	//--#土耳其 logo
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,19) == TRUE && scalerVIP_Get_MEMCPatternFlag_Identification(4,6) == FALSE &&
		( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ) &&
		( u11_gmv_mvx==0 && u10_gmv_mvy==0 && u1_still_frame==0 && u12_gmv_unconf==0 && u25_me_aDTL>=750000 && u25_me_aDTL<=850000) )
	{
		WriteRegister(KME_ME1_TOP1_ME1_SCENE1_07_reg,0,30,0xffff);
		u5_blend_holdfrm_a[19] = 150;
		nIdentifiedNum_a[19]=19;
	}
	else if(u5_blend_holdfrm_a[19] > 0)
	{
		WriteRegister(KME_ME1_TOP1_ME1_SCENE1_07_reg,0,30,0xffff);
		u5_blend_holdfrm_a[19]--;
	}
	else if(nIdentifiedNum_a[19] == 19)
	{
		nIdentifiedNum_a[19] = 0;
	}
	
	//--#25005huanlerenjian1 50s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,21) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ) &&
		(u11_gmv_mvx>=-48 && u11_gmv_mvx<=-45) && (u10_gmv_mvy>=-1 && u10_gmv_mvy<=1) && s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb<2)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x11111111);
		u5_blend_holdfrm_a[21] = 50;
		nIdentifiedNum_a[21]=21;
	}
	else if(u5_blend_holdfrm_a[21] > 0)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x11111111);
		u5_blend_holdfrm_a[21]--;
	}
	else if(nIdentifiedNum_a[21] == 21)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x40);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x30);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x10);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x40);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x30);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x10);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x77777777);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x77777777);

		nIdentifiedNum_a[21] = 0;
	}

	//--#聖托里尼 5:09s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,22) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 )&&
		u11_gmv_mvx==0 && u10_gmv_mvy>=0 && u10_gmv_mvy<=1 && u1_still_frame==0 && u12_gmv_unconf<=4 )
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x55555555);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x55555555);
		u5_blend_holdfrm_a[22] = 20;
		nIdentifiedNum_a[22]=22;
	}
	else if(u5_blend_holdfrm_a[22] > 0)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x55555555);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x55555555);
		u5_blend_holdfrm_a[22]--;
	}
	else if(nIdentifiedNum_a[22] == 22)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x40);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x30);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x10);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0);
		
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x40);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x30);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x10);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0);
		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x77777777);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x77777777);
		
		nIdentifiedNum_a[22] = 0;
	}
#if 0
	//--#C005 7s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,22) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		//WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);		// local FB
		//WriteRegister(MC_MC_28_reg,14,14,0x1);				// local FB
		//WriteRegister(MC_MC_28_reg,15,22,0x0);				// local FB
		//WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		//WriteRegister(KME_ALGO_ME1_MEDFLT_CTRL_00_reg,18,18,0);//key
		s_pContext_pre->_output_wrt_comreg.u1_GFB_force_zero_true=1;
		u5_blend_holdfrm_a[22] = 100;
		nIdentifiedNum_a[22]=22;
	}
	else if(u5_blend_holdfrm_a[22] > 0)
	{
		//WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);		// local FB
		//WriteRegister(MC_MC_28_reg,14,14,0x1);				// local FB
		//WriteRegister(MC_MC_28_reg,15,22,0x0);				// local FB
		//WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		//WriteRegister(KME_ALGO_ME1_MEDFLT_CTRL_00_reg,18,18,0);
		s_pContext_pre->_output_wrt_comreg.u1_GFB_force_zero_true=1;
		u5_blend_holdfrm_a[22]--;
	}
	else if(nIdentifiedNum_a[22] == 22)
	{
		//WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);		// local FB
		//WriteRegister(MC_MC_28_reg,14,14,0x0);				// local FB
		//WriteRegister(MC_MC_28_reg,15,22,0x0);				// local FB
		//WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x1);
		//WriteRegister(KME_ALGO_ME1_MEDFLT_CTRL_00_reg,18,18,1);
		s_pContext_pre->_output_wrt_comreg.u1_GFB_force_zero_true=0;
		nIdentifiedNum_a[22] = 0;
	}
#endif

	//--#C011 北京风景人抖动 5 10 11s
	if( (scalerVIP_Get_MEMCPatternFlag_Identification(4,1) == TRUE || 
		scalerVIP_Get_MEMCPatternFlag_Identification(4,23) == TRUE || 
		scalerVIP_Get_MEMCPatternFlag_Identification(4,57) == TRUE || 
		scalerVIP_Get_MEMCPatternFlag_Identification(4,65) == TRUE ) && 
		( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x11111111);
	
		u5_blend_holdfrm_a[23] = 100;
		nIdentifiedNum_a[23]=23;
	}
	else if(u5_blend_holdfrm_a[23] > 0)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x11111111);

		u5_blend_holdfrm_a[23]--;
	}
	else if(nIdentifiedNum_a[23] == 23 )
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x40);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x30);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x10);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0);
	
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x40);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x30);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x10);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x77777777);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x77777777);

		nIdentifiedNum_a[23] = 0;
	}

	//--#240008_DK_Zoomin
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,24) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x11111111);
		u5_blend_holdfrm_a[24] = 100;
		nIdentifiedNum_a[24]=24;
	}
	else if(u5_blend_holdfrm_a[24] > 0)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
		
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x11111111);

		u5_blend_holdfrm_a[24]--;
	}
	else if(nIdentifiedNum_a[24] == 24)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x40);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x30);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x10);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x40);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x30);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x10);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x77777777);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x77777777);

		nIdentifiedNum_a[24] = 0;
	}

	//--#25005huanlerenjian1 15s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,25) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
		
		WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x1);
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 255);	 
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 230);
		WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0);

		WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 0);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 0);

		WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0x0);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x11111111);

		u5_blend_holdfrm_a[25] = 100;
		nIdentifiedNum_a[25]=25;
	}
	else if(u5_blend_holdfrm_a[25] > 0)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
		
		WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x1);
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 255);	 
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 230);
		WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0);

		WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 0);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 0);

		WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0x0);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x11111111);
		u5_blend_holdfrm_a[25]--;
	}
	else if(nIdentifiedNum_a[25] == 25)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x40);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x30);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x10);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x40);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x30);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x10);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0);

		WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x0);
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 0xff);	  
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 0x0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 1);

		WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 1);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 1);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 1);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 1);

		WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0xffffffff);
		WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0xffffffff);
		WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0xffffffff);
		WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0xffffffff);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x77777777);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x77777777);

		nIdentifiedNum_a[25] = 0;
	}

	//--#雪中捍刀行 窗戶破碎
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,26) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 )&&
		u11_gmv_mvx>=-3 && u11_gmv_mvx<=9 && u10_gmv_mvy>=-2 && u10_gmv_mvy<=10 && u1_still_frame==0 && u12_gmv_unconf<=4 && u25_me_aDTL>=1310000 && u25_me_aDTL<=1440000 )
	{
		rtd_pr_memc_emerg("mvx:%d mvy:%d sf:%d aDTL:%u unconf:%d\n", 
			u11_gmv_mvx, u10_gmv_mvy, 
			u1_still_frame, u25_me_aDTL, u12_gmv_unconf);
	
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
	
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x55555555);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x55555555);
	
		WriteRegister(KME_ALGO_ME1_MEDFLT_CTRL_00_reg,18,18,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,1,1,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,2,2,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,4,4,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,5,5,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,0,0,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,1,1,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,3,3,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,4,4,0x0);
		u5_blend_holdfrm_a[26] = 20;
		nIdentifiedNum_a[26]=26;
	}
	else if(u5_blend_holdfrm_a[26] > 0)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
		
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x55555555);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x55555555);

		WriteRegister(KME_ALGO_ME1_MEDFLT_CTRL_00_reg,18,18,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,1,1,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,2,2,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,4,4,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,5,5,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,0,0,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,1,1,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,3,3,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,4,4,0x0);
		u5_blend_holdfrm_a[26]--;
	}
	else if(nIdentifiedNum_a[26] == 26)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x40);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x30);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x10);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0);
	
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x40);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x30);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x10);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x77777777);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x77777777);

		WriteRegister(KME_ALGO_ME1_MEDFLT_CTRL_00_reg,18,18,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,1,1,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,2,2,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,4,4,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,5,5,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,0,0,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,1,1,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,3,3,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,4,4,0x1);
		nIdentifiedNum_a[26] = 0;
	}

	//--#60015-Woke from home 13s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,27) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);		// local FB
		WriteRegister(MC_MC_28_reg,14,14,0x1);				// local FB
		WriteRegister(MC_MC_28_reg,15,22,0x0);				// local FB
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		u5_blend_holdfrm_a[27] = 100;
		nIdentifiedNum_a[27]=27;
	}
	else if(u5_blend_holdfrm_a[27] > 0)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);		// local FB
		WriteRegister(MC_MC_28_reg,14,14,0x1);				// local FB
		WriteRegister(MC_MC_28_reg,15,22,0x0);				// local FB
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		u5_blend_holdfrm_a[27]--;
	}
	else if(nIdentifiedNum_a[27] == 27)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);		// local FB
		WriteRegister(MC_MC_28_reg,14,14,0x0);				// local FB
		WriteRegister(MC_MC_28_reg,15,22,0x0);				// local FB
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x1);
		nIdentifiedNum_a[27] = 0;
	}

	//--#24004 0s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,30) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
	
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
		
		WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x1);
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 255);	 
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 230);
		WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0);
	
		WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 0);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 0);
	
		WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0x0);
	
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x11111111);
		u5_blend_holdfrm_a[30] = 20;
		nIdentifiedNum_a[30]=30;
	}
	else if(u5_blend_holdfrm_a[30] > 0)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
	
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
		
		WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x1);
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 255);	 
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 230);
		WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0);
	
		WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 0);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 0);
	
		WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0x0);
	
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x11111111);
		u5_blend_holdfrm_a[30]--;
	}
	else if(nIdentifiedNum_a[30] == 30)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x40);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x30);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x10);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0);
	
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x40);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x30);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x10);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0);
	
		WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x0);
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 0xff);	  
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 0x0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 1);
	
		WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 1);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 1);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 1);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 1);
	
		WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0xffffffff);
		WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0xffffffff);
		WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0xffffffff);
		WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0xffffffff);
	
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x77777777);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x77777777);
	
		nIdentifiedNum_a[30] = 0;
	}
	
	//--#24004 19s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,31) == TRUE && ( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 )&&
		(u11_gmv_mvx>=-9&&u11_gmv_mvx<=8) && (u10_gmv_mvy>=0&&u10_gmv_mvy<=3) && s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb < 6)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
	
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
		
		WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x1);
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 255);	 
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 230);
		WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0);
	
		WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 0);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 0);
	
		WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0x0);
	
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x11111111);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,11,13,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,10,12,0x0);
		
		u5_blend_holdfrm_a[31] = 8;
		nIdentifiedNum_a[31]=31;
	}
	else if(u5_blend_holdfrm_a[31] > 0)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
	
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
		
		WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x1);
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 255);	 
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 230);
		WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0);
	
		WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 0);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 0);
	
		WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0x0);
	
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x11111111);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,11,13,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,10,12,0x0);
		u5_blend_holdfrm_a[31]--;
	}
	else if(nIdentifiedNum_a[31] == 31)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x40);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x30);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x10);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0);
	
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x40);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x30);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x10);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0);
	
		WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x0);
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 0xff);	  
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 0x0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 1);
	
		WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 1);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 1);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 1);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 1);
	
		WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0xffffffff);
		WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0xffffffff);
		WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0xffffffff);
		WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0xffffffff);
	
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x77777777);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x77777777);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,11,13,0x7);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,10,12,0x7);
		nIdentifiedNum_a[31] = 0;
	}

	//--#30006 45s
	if((scalerVIP_Get_MEMCPatternFlag_Identification(4,33) == TRUE ||scalerVIP_Get_MEMCPatternFlag_Identification(4,54) == TRUE)&&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 )&&
		(u25_me_aDTL>1100000&&u25_me_aDTL<2500000))
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
		
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x11111111);
		
		u5_blend_holdfrm_a[33] = 20;
		nIdentifiedNum_a[33]=33;
	}
	else if(u5_blend_holdfrm_a[33] > 0)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
		
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x11111111);

		u5_blend_holdfrm_a[33]--;
	}
	else if(nIdentifiedNum_a[33] == 33)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x40);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x30);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x10);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0);
		
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x40);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x30);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x10);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0);
		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x77777777);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x77777777);

		nIdentifiedNum_a[33] = 0;
	}

	//--#30012_IDT_Eureka_moon
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,35) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);	// local FB
		WriteRegister(MC_MC_28_reg,14,14,0x1);			// local FB
		WriteRegister(MC_MC_28_reg,15,22,0x0);			// local FB
		u5_blend_holdfrm_a[35] = 50;
		nIdentifiedNum_a[35]=35;
	}
	else if(u5_blend_holdfrm_a[35] > 0)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);	// local FB
		WriteRegister(MC_MC_28_reg,14,14,0x1);			// local FB
		WriteRegister(MC_MC_28_reg,15,22,0x0);			// local FB
		u5_blend_holdfrm_a[35]--;
	}
	else if(nIdentifiedNum_a[35] == 35)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);	// local FB
		WriteRegister(MC_MC_28_reg,14,14,0x0);			// local FB
		WriteRegister(MC_MC_28_reg,15,22,0x0);			// local FB
		nIdentifiedNum_a[35] = 0;
	}

	//--#猫妖传 2:33
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,36) == TRUE && scalerVIP_Get_MEMCPatternFlag_Identification(4,37) == FALSE
		&&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);	// local FB
		WriteRegister(MC_MC_28_reg,14,14,0x1);			// local FB
		WriteRegister(MC_MC_28_reg,15,22,0x0);			// local FB
		WriteRegister(KME_LOGO0_KME_LOGO0_AC_reg,8,15,200);
		u5_blend_holdfrm_a[36] = 20;
		nIdentifiedNum_a[36]=36;
	}
	else if(u5_blend_holdfrm_a[36] > 0)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);	// local FB
		WriteRegister(MC_MC_28_reg,14,14,0x1);			// local FB
		WriteRegister(MC_MC_28_reg,15,22,0x0);			// local FB
		WriteRegister(KME_LOGO0_KME_LOGO0_AC_reg,8,15,200);
		u5_blend_holdfrm_a[36]--;
	}
	else if(nIdentifiedNum_a[36] == 36)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);	// local FB
		WriteRegister(MC_MC_28_reg,14,14,0x0);			// local FB
		WriteRegister(MC_MC_28_reg,15,22,0x0);			// local FB
		WriteRegister(KME_LOGO0_KME_LOGO0_AC_reg,8,15,80);
		nIdentifiedNum_a[36] = 0;
	}

#if 0
	//--#猫妖传 2:36
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,37) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ) &&
		u10_gmv_mvy>=-1 && u10_gmv_mvy<=3 && u25_me_aDTL>=1300000 && u25_me_aDTL<=1550000 && u12_gmv_unconf<=3)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
		
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x11111111);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 1, 1, 0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 0, 0, 0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, 0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 5, 5, 0x0);

		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg, 16, 16, 0x0);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg, 19, 19, 0x0);
		u5_blend_holdfrm_a[37] = 40;
		nIdentifiedNum_a[37]=37;
	}
	else if(u5_blend_holdfrm_a[37] > 0)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
		
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x11111111);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 1, 1, 0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 0, 0, 0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, 0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 5, 5, 0x0);

		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg, 16, 16, 0x0);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg, 19, 19, 0x0);
		u5_blend_holdfrm_a[37]--;
	}
	else if(nIdentifiedNum_a[37] == 37)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x40);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x30);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x10);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0);
	
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x40);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x30);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x10);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0);
	
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x77777777);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x77777777);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 1, 1, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 0, 0, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 5, 5, 0x1);

		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg, 16, 16, 0x1);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg, 19, 19, 0x1);
		nIdentifiedNum_a[37] = 0;
	}
#endif

#if 0
	//--#402 80s
	if( (scalerVIP_Get_MEMCPatternFlag_Identification(4,32) == TRUE ) &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
		
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x11111111);

		u5_blend_holdfrm_a[32] = 150;
		nIdentifiedNum_a[32]=32;
	}
	else if(u5_blend_holdfrm_a[32] > 0)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
		
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x11111111);

		u5_blend_holdfrm_a[32]--;
	}
	else if(nIdentifiedNum_a[32] == 32)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x40);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x30);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x10);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0);
	
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x40);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x30);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x10);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0);
	
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x77777777);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x77777777);
		
		nIdentifiedNum_a[32] = 0;
	}
#endif

	//--#电视的觉醒年代 2:10
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,38) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 26, 0x502814);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 26, 0x4c2814);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 26, 0x502813);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 26, 0x502814);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 17, 0x2613);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x33333333);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x33333333);
		u5_blend_holdfrm_a[38] = 50;
		nIdentifiedNum_a[38]=38;
	}
	else if(u5_blend_holdfrm_a[38] > 0)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 26, 0x502814);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 26, 0x4c2814);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 26, 0x502813);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 26, 0x502814);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 17, 0x2613);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x33333333);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x33333333);
		u5_blend_holdfrm_a[38]--;
	}
	else if(nIdentifiedNum_a[38] == 38)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 26, 0x806040);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 26, 0x10);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 26, 0xc08000);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 26, 0x2020);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 17, 0x0);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x77777777);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x77777777);
		nIdentifiedNum_a[38] = 0;
	}

	//--#瑞士 2:08
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,39) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ) &&
		u11_gmv_mvx>=0 && u11_gmv_mvx<=4 && u10_gmv_mvy==0 && u1_still_frame==0 && u12_gmv_unconf<=1 && u25_me_aDTL>=1510000 && u25_me_aDTL<=1620000)
	{
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);
		WriteRegister(MC_MC_28_reg,14,14,0x1);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		u5_blend_holdfrm_a[39] = 50;
		nIdentifiedNum_a[39]=39;
	}
	else if(u5_blend_holdfrm_a[39] > 0)
	{
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);
		WriteRegister(MC_MC_28_reg,14,14,0x1);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		u5_blend_holdfrm_a[39]--;
	}
	else if(nIdentifiedNum_a[39] == 39)
	{
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x1);
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);
		WriteRegister(MC_MC_28_reg,14,14,0x0);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		nIdentifiedNum_a[39] = 0;
	}

#if 1
	//--#24002 0s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,43) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ) &&
		in_fmRate==_PQL_IN_50HZ && out_fmRate==_PQL_OUT_100HZ)
	{
		//s_pContext->_output_filmDetectctrl.u8_cur_cadence_Id
		WriteRegister(SOFTWARE1_SOFTWARE1_12_reg,0,4,0x10);
		u5_blend_holdfrm_a[43] = 50;
		nIdentifiedNum_a[43]=43;
	}
	else if(u5_blend_holdfrm_a[43] > 0)
	{
		WriteRegister(SOFTWARE1_SOFTWARE1_12_reg,0,4,0x10);
		u5_blend_holdfrm_a[43]--;
	}
	else if(nIdentifiedNum_a[43] == 43)
	{
		WriteRegister(SOFTWARE1_SOFTWARE1_12_reg,0,4,0x0);
		nIdentifiedNum_a[43] = 0;
	}
#endif

	//--#24007 46s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,44) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);	// local FB
		WriteRegister(MC_MC_28_reg,14,14,0x1);			// local FB
		WriteRegister(MC_MC_28_reg,15,22,0x0);			// local FB
		WriteRegister(SOFTWARE_SOFTWARE_10_reg, 0, 0, 0);
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		WriteRegister(KME_ME1_TOP1_ME1_SCENE1_07_reg,0,30,0xffffffff);
		WriteRegister(KME_ALGO_ME1_MEDFLT_CTRL_00_reg,18,18,0x0);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_B0_reg,0,0,0x0);

		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 26, 0x502814);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 26, 0x4c2814);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 26, 0x502813);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 26, 0x502814);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 17, 0x2613);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x57575757);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x57575757);
		
		u5_blend_holdfrm_a[44] = 140;
		nIdentifiedNum_a[44]=44;
	}
	else if(u5_blend_holdfrm_a[44] > 0)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);	// local FB
		WriteRegister(MC_MC_28_reg,14,14,0x1);			// local FB
		WriteRegister(MC_MC_28_reg,15,22,0x0);			// local FB
		WriteRegister(SOFTWARE_SOFTWARE_10_reg, 0, 0, 0);
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		WriteRegister(KME_ME1_TOP1_ME1_SCENE1_07_reg,0,30,0xffffffff);
		WriteRegister(KME_ALGO_ME1_MEDFLT_CTRL_00_reg,18,18,0x0);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_B0_reg,0,0,0x0);

		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 26, 0x502814);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 26, 0x4c2814);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 26, 0x502813);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 26, 0x502814);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 17, 0x2613);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x57575757);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x57575757);
		u5_blend_holdfrm_a[44]--;
	}
	else if(nIdentifiedNum_a[44] == 44)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);	// local FB
		WriteRegister(MC_MC_28_reg,14,14,0x0);			// local FB
		WriteRegister(MC_MC_28_reg,15,22,0x0);			// local FB
		WriteRegister(SOFTWARE_SOFTWARE_10_reg, 0, 0, 1);
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x1);
		//WriteRegister(KME_ME1_TOP1_ME1_SCENE1_07_reg,0,30,0x980000);
		WriteRegister(KME_ALGO_ME1_MEDFLT_CTRL_00_reg,18,18,0x1);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_B0_reg,0,0,0x1);

		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 26, 0x806040);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 26, 0x10);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 26, 0xc08000);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 26, 0x2020);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 17, 0x0);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x77777777);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x77777777);
		nIdentifiedNum_a[44] = 0;
	}

	//--#402 35s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,45) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		//WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_54_reg, 18, 18, 0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_AC_reg, 16, 23, 100);
		WriteRegister(KME_LOGO0_KME_LOGO0_AC_reg, 24, 31, 100);
		WriteRegister(KME_LOGO0_KME_LOGO0_B0_reg, 0, 7, 100);
		WriteRegister(KME_LOGO0_KME_LOGO0_B0_reg, 8, 15, 100);
		WriteRegister(KME_LOGO0_KME_LOGO0_B0_reg, 16, 23, 100);
		u5_blend_holdfrm_a[45] = 20;
		nIdentifiedNum_a[45]=45;
	}
	else if(u5_blend_holdfrm_a[45] > 0)
	{
		//WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_54_reg, 18, 18, 0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_AC_reg, 16, 23, 100);
		WriteRegister(KME_LOGO0_KME_LOGO0_AC_reg, 24, 31, 100);
		WriteRegister(KME_LOGO0_KME_LOGO0_B0_reg, 0, 7, 100);
		WriteRegister(KME_LOGO0_KME_LOGO0_B0_reg, 8, 15, 100);
		WriteRegister(KME_LOGO0_KME_LOGO0_B0_reg, 16, 23, 100);
		u5_blend_holdfrm_a[45]--;
	}
	else if(nIdentifiedNum_a[45] == 45)
	{
		//WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x1);
		//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_54_reg, 18, 18, 0x1);
		WriteRegister(KME_LOGO0_KME_LOGO0_AC_reg, 16, 23, 32);
		WriteRegister(KME_LOGO0_KME_LOGO0_AC_reg, 24, 31, 32);
		WriteRegister(KME_LOGO0_KME_LOGO0_B0_reg, 0, 7, 32);
		WriteRegister(KME_LOGO0_KME_LOGO0_B0_reg, 8, 15, 32);
		WriteRegister(KME_LOGO0_KME_LOGO0_B0_reg, 16, 23, 32);
		nIdentifiedNum_a[45] = 0;
	}

	//--#8008
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,46) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);
		WriteRegister(MC_MC_28_reg,14,14,0x1);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		WriteRegister(SOFTWARE_SOFTWARE_10_reg, 0, 0, 0);
//		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		u5_blend_holdfrm_a[46] = 20;
		nIdentifiedNum_a[46]=46;
	}
	else if(u5_blend_holdfrm_a[46] > 0)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);
		WriteRegister(MC_MC_28_reg,14,14,0x1);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		WriteRegister(SOFTWARE_SOFTWARE_10_reg, 0, 0, 0);
//		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		u5_blend_holdfrm_a[46]--;
	}
	else if(nIdentifiedNum_a[46] == 46)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);
		WriteRegister(MC_MC_28_reg,14,14,0x0);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		WriteRegister(SOFTWARE_SOFTWARE_10_reg, 0, 0, 1);
//		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x1);
		nIdentifiedNum_a[46] = 0;
	}

	//--#25003 95s
	if( (scalerVIP_Get_MEMCPatternFlag_Identification(4,47) == TRUE ||
		 scalerVIP_Get_MEMCPatternFlag_Identification(4,48) == TRUE ||
		 scalerVIP_Get_MEMCPatternFlag_Identification(4,49) == TRUE ||
		 scalerVIP_Get_MEMCPatternFlag_Identification(4,50) == TRUE )
	&&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		WriteRegister(MC_MC_28_reg,14,14,0x1);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		WriteRegister(HARDWARE_HARDWARE_06_reg,0,1,0x0);
		u5_blend_holdfrm_a[47] = 100;
		nIdentifiedNum_a[47]=47;
	}
	else if(u5_blend_holdfrm_a[47] > 0)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		WriteRegister(MC_MC_28_reg,14,14,0x1);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		WriteRegister(HARDWARE_HARDWARE_06_reg,0,1,0x0);
		u5_blend_holdfrm_a[47]--;
	}
	else if(nIdentifiedNum_a[47] == 47)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x1);
		WriteRegister(MC_MC_28_reg,14,14,0x0);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		WriteRegister(HARDWARE_HARDWARE_06_reg,0,1,0x0);
		nIdentifiedNum_a[47] = 0;
	}
#if 0
	//--#25003 110s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,48) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);
		WriteRegister(MC_MC_28_reg,14,14,0x1);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		WriteRegister(HARDWARE_HARDWARE_06_reg,0,1,0x0);
		u5_blend_holdfrm_a[48] = 100;
		nIdentifiedNum_a[48]=48;
	}
	else if(u5_blend_holdfrm_a[48] > 0)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);
		WriteRegister(MC_MC_28_reg,14,14,0x1);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		WriteRegister(HARDWARE_HARDWARE_06_reg,0,1,0x0);
		u5_blend_holdfrm_a[48]--;
	}
	else if(nIdentifiedNum_a[48] == 48)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);
		WriteRegister(MC_MC_28_reg,14,14,0x0);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		WriteRegister(HARDWARE_HARDWARE_06_reg,0,1,0x0);
		nIdentifiedNum_a[48] = 0;
	}

	//--#25003 115s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,49) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);
		WriteRegister(MC_MC_28_reg,14,14,0x1);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		WriteRegister(HARDWARE_HARDWARE_06_reg,0,1,0x0);
		u5_blend_holdfrm_a[49] = 100;
		nIdentifiedNum_a[49]=49;
	}
	else if(u5_blend_holdfrm_a[49] > 0)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);
		WriteRegister(MC_MC_28_reg,14,14,0x1);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		WriteRegister(HARDWARE_HARDWARE_06_reg,0,1,0x0);
		u5_blend_holdfrm_a[49]--;
	}
	else if(nIdentifiedNum_a[49] == 49)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);
		WriteRegister(MC_MC_28_reg,14,14,0x0);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		WriteRegister(HARDWARE_HARDWARE_06_reg,0,1,0x0);
		nIdentifiedNum_a[49] = 0;
	}

	//--#25003 120s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,50) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);
		WriteRegister(MC_MC_28_reg,14,14,0x1);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		WriteRegister(HARDWARE_HARDWARE_06_reg,0,1,0x0);
		u5_blend_holdfrm_a[50] = 100;
		nIdentifiedNum_a[50]=50;
	}
	else if(u5_blend_holdfrm_a[50] > 0)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);
		WriteRegister(MC_MC_28_reg,14,14,0x1);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		WriteRegister(HARDWARE_HARDWARE_06_reg,0,1,0x0);
		u5_blend_holdfrm_a[50]--;
	}
	else if(nIdentifiedNum_a[50] == 50)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);
		WriteRegister(MC_MC_28_reg,14,14,0x0);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		WriteRegister(HARDWARE_HARDWARE_06_reg,0,1,0x0);
		nIdentifiedNum_a[50] = 0;
	}
#endif
	//--#04_Montage_P 5s
	//--#25003 10s
	//--#30006 60s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,51) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(HARDWARE_HARDWARE_00_reg,0,0,0x0);
		u5_blend_holdfrm_a[51] = 80;
		nIdentifiedNum_a[51]=51;
	}
	else if(u5_blend_holdfrm_a[51] > 0)
	{
		WriteRegister(HARDWARE_HARDWARE_00_reg,0,0,0x0);
		u5_blend_holdfrm_a[51]--;
	}
	else if(nIdentifiedNum_a[51] == 51)
	{
		WriteRegister(HARDWARE_HARDWARE_00_reg,0,0,0x1);
		nIdentifiedNum_a[51] = 0;
	}

	//--#60015 5s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,52) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KME_ME1_TOP1_ME1_SCENE1_01_reg,0,31,0x842108);
		WriteRegister(KME_ME1_TOP1_ME1_SCENE1_02_reg,0,31,0x2040803);		
		u5_blend_holdfrm_a[52] = 1;
		nIdentifiedNum_a[52]=52;
	}
	else if(u5_blend_holdfrm_a[52] > 0)
	{
		WriteRegister(KME_ME1_TOP1_ME1_SCENE1_01_reg,0,31,0x842108);
		WriteRegister(KME_ME1_TOP1_ME1_SCENE1_02_reg,0,31,0x2040803);
		u5_blend_holdfrm_a[52]--;
	}
	else if(nIdentifiedNum_a[52] == 52)
	{
		WriteRegister(KME_ME1_TOP1_ME1_SCENE1_01_reg,0,31,0x0);
		WriteRegister(KME_ME1_TOP1_ME1_SCENE1_02_reg,0,31,0x2040103);
		nIdentifiedNum_a[52] = 0;
	}

	//--#30006 33s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,53) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		//s_pContext_pre->_output_frc_sceneAnalysis.u8_dh_condition_dhbypass = 1;
		u5_blend_holdfrm_a[53] = 50;
		nIdentifiedNum_a[53]=53;
	}
	else if(u5_blend_holdfrm_a[53] > 0)
	{
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		//s_pContext_pre->_output_frc_sceneAnalysis.u8_dh_condition_dhbypass = 1;
		u5_blend_holdfrm_a[53]--;
	}
	else if(nIdentifiedNum_a[53] == 53)
	{
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x1);
		//s_pContext_pre->_output_frc_sceneAnalysis.u8_dh_condition_dhbypass = 0;
		nIdentifiedNum_a[53] = 0;
	}

	//--#25003_philips
	if( (scalerVIP_Get_MEMCPatternFlag_Identification(4,54) == TRUE || scalerVIP_Get_MEMCPatternFlag_Identification(4,34) == TRUE) &&
		( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
		
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);

		WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x1);
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 255);	 
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 230);
		WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0);

		WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 0);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 0);

		WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0x0);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x11111111);

		WriteRegister(KME_ALGO_ME1_MEDFLT_CTRL_00_reg,18,18,0x0);

		u5_blend_holdfrm_a[54] = 50;
		nIdentifiedNum_a[54]=54;
	}
	else if(u5_blend_holdfrm_a[54] > 0)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
		
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);

		WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x1);
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 255);	 
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 230);
		WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0);

		WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 0);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 0);

		WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0x0);
		WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0x0);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x11111111);

		WriteRegister(KME_ALGO_ME1_MEDFLT_CTRL_00_reg,18,18,0x0);
		u5_blend_holdfrm_a[54]--;
	}
	else if(nIdentifiedNum_a[54] == 54)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x40);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x30);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x10);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x40);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x30);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x10);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0);

		WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x0);
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 0xff);	  
		WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 0x0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 1);

		WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 1);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 1);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 1);
		WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 1);

		WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0xffffffff);
		WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0xffffffff);
		WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0xffffffff);
		WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0xffffffff);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x77777777);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x77777777);

		WriteRegister(KME_ALGO_ME1_MEDFLT_CTRL_00_reg,18,18,0x1);
		nIdentifiedNum_a[54] = 0;
	}

	//--#30016-hor-dispatch2
	if( (scalerVIP_Get_MEMCPatternFlag_Identification(4,55) == TRUE || scalerVIP_Get_MEMCPatternFlag_Identification(4,42) == TRUE) &&
		( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ) &&
		(_ABS_(u11_gmv_mvx)+_ABS_(u10_gmv_mvy))<=3 && u1_still_frame==0 && u25_me_aDTL>=370000 && u25_me_aDTL<=560000 )
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
		
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x11111111);

		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg, 8,27,0xfffff);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_11_reg, 0,29,0x3fffffff);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_12_reg, 0,31,0xfffcfffc);
		
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,0,4,0x0);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,8,12,0x0);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg,3,7,0x0);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg,11,15,0x0);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,0,16,0xe96d);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,0,15,0x7c94);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg,0,23,0xe413ff);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg,0,23,0xe413ff);

		WriteRegister(KME_LOGO0_KME_LOGO0_AC_reg, 16, 23, 255);
		WriteRegister(KME_LOGO0_KME_LOGO0_AC_reg, 24, 31, 255);
		WriteRegister(KME_LOGO0_KME_LOGO0_B0_reg,  0,  7, 255);
		WriteRegister(KME_LOGO0_KME_LOGO0_B0_reg,  8, 15, 255);
		WriteRegister(KME_LOGO0_KME_LOGO0_B0_reg, 16, 23, 255);

		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg, 0, 0, 0);
		u5_blend_holdfrm_a[55] = 40;
		nIdentifiedNum_a[55]=55;
	}
	else if(u5_blend_holdfrm_a[55] > 0)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
		
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x11111111);

		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg, 8,27,0xfffff);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_11_reg, 0,29,0x3fffffff);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_12_reg, 0,31,0xfffcfffc);
		
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,0,4,0x0);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,8,12,0x0);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg,3,7,0x0);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg,11,15,0x0);
		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,0,16,0xe96d);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,0,15,0x7c94);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg,0,23,0xe413ff);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg,0,23,0xe413ff);

		WriteRegister(KME_LOGO0_KME_LOGO0_AC_reg, 16, 23, 255);
		WriteRegister(KME_LOGO0_KME_LOGO0_AC_reg, 24, 31, 255);
		WriteRegister(KME_LOGO0_KME_LOGO0_B0_reg,  0,  7, 255);
		WriteRegister(KME_LOGO0_KME_LOGO0_B0_reg,  8, 15, 255);
		WriteRegister(KME_LOGO0_KME_LOGO0_B0_reg, 16, 23, 255);

		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg, 0, 0, 0);

		u5_blend_holdfrm_a[55]--;
	}
	else if(nIdentifiedNum_a[55] == 55)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x40);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x30);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x10);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x40);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x30);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x10);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x77777777);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x77777777);

		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg, 8,27,0x0);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_11_reg, 0,29,0x802810);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_12_reg, 0,31,0x50000);
		
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,0,4,0x1);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,8,12,0x1);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg,3,7,0x1);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg,11,15,0x1);
		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,0,16,0xb9ff);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,0,15,0x5cff);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg,0,23,0xe79e3f);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg,0,23,0x239e3f);

		WriteRegister(KME_LOGO0_KME_LOGO0_AC_reg, 16, 23, 32);
		WriteRegister(KME_LOGO0_KME_LOGO0_AC_reg, 24, 31, 32);
		WriteRegister(KME_LOGO0_KME_LOGO0_B0_reg,  0,  7, 32);
		WriteRegister(KME_LOGO0_KME_LOGO0_B0_reg,  8, 15, 32);
		WriteRegister(KME_LOGO0_KME_LOGO0_B0_reg, 16, 23, 32);

		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg, 0, 0, 1);

		nIdentifiedNum_a[55] = 0;
	}

	//--#402 0s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,56) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext_pre->_external_data.u1_ReplaceProtect = true;
		u5_blend_holdfrm_a[56] = 20;
		nIdentifiedNum_a[56]=56;
	}
	else if(u5_blend_holdfrm_a[56] > 0)
	{
		s_pContext_pre->_external_data.u1_ReplaceProtect = true;
		u5_blend_holdfrm_a[56]--;
	}
	else if(nIdentifiedNum_a[56] == 56)
	{
		s_pContext_pre->_external_data.u1_ReplaceProtect = false;
		nIdentifiedNum_a[56] = 0;
	}

	//--#402 70s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,57) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext_pre->_external_data.u1_ReplaceProtect = true;
		s_pContext_pre->_external_data.u8_SpecialCase = 1;
		u5_blend_holdfrm_a[57] = 20;
		nIdentifiedNum_a[57]=57;
	}
	else if(u5_blend_holdfrm_a[57] > 0)
	{
		s_pContext_pre->_external_data.u1_ReplaceProtect = true;
		s_pContext_pre->_external_data.u8_SpecialCase = 1;
		u5_blend_holdfrm_a[57]--;
	}
	else if(nIdentifiedNum_a[57] == 57)
	{
		s_pContext_pre->_external_data.u1_ReplaceProtect = false;
		s_pContext_pre->_external_data.u8_SpecialCase = 0;
		nIdentifiedNum_a[57] = 0;
	}

	//--#24003 11s
	if( scalerVIP_Get_MEMCPatternFlag_Identification(4,58) == TRUE && ( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ) &&
		( u12_gmv_unconf<=5 && u25_me_aDTL>=900000 && u25_me_aDTL<=2580000 && u1_still_frame==0 ) )
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);
		WriteRegister(MC_MC_28_reg,14,14,0x1);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		WriteRegister(SOFTWARE_SOFTWARE_10_reg, 0, 0, 0);
		u5_blend_holdfrm_a[68] = 80;
		nIdentifiedNum_a[68]=68;
	}
	else if(u5_blend_holdfrm_a[68] > 0)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);
		WriteRegister(MC_MC_28_reg,14,14,0x1);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		WriteRegister(SOFTWARE_SOFTWARE_10_reg, 0, 0, 0);
		u5_blend_holdfrm_a[68]--;
	}
	else if(nIdentifiedNum_a[68] == 68)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);
		WriteRegister(MC_MC_28_reg,14,14,0x0);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		WriteRegister(SOFTWARE_SOFTWARE_10_reg, 0, 0, 1);
		nIdentifiedNum_a[68] = 0;
	}

	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,59) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 0, 0, 0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 1, 1, 0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, 0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 5, 5, 0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 6, 6, 0x0);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 1, 1, 0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 2, 2, 0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 7, 7, 0x0);

		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);

		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);		
		WriteRegister(MC_MC_28_reg,14,14,0x1);				
		WriteRegister(MC_MC_28_reg,15,22,0x0);				

		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0xffffffff);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0xffffffff);		
		u5_blend_holdfrm_a[59] = 100;
		nIdentifiedNum_a[59]=59;
	}
	else if(u5_blend_holdfrm_a[59] > 0)
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 0, 0, 0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 1, 1, 0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, 0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 5, 5, 0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 6, 6, 0x0);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 1, 1, 0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 2, 2, 0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 7, 7, 0x0);

		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);

		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);		
		WriteRegister(MC_MC_28_reg,14,14,0x1);				
		WriteRegister(MC_MC_28_reg,15,22,0x0);				

		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0xffffffff);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0xffffffff);
		u5_blend_holdfrm_a[59]--;
	}
	else if(nIdentifiedNum_a[59] == 59)
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 0, 0, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 1, 1, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 5, 5, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 6, 6, 0x1);
		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 1, 1, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 2, 2, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 7, 7, 0x1);
		
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x1);
		
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);		
		WriteRegister(MC_MC_28_reg,14,14,0x0);				
		WriteRegister(MC_MC_28_reg,15,22,0x0);	
		
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x40);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x30);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x10);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0);
		
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x40);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x30);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x10);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0);
		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x77777777);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x77777777);

		nIdentifiedNum_a[59] = 0;
	}

	//--#240001_Satelite 4s white plate
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,60) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);	
		WriteRegister(MC_MC_28_reg,14,14,0x1);			
		WriteRegister(MC_MC_28_reg,15,22,0x0);			
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 0, 16, 0xb969);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 0, 15, 0x5c94);
		u5_blend_holdfrm_a[60] = 100;
		nIdentifiedNum_a[60]=60;
	}
	else if(u5_blend_holdfrm_a[60] > 0)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);	
		WriteRegister(MC_MC_28_reg,14,14,0x1);			
		WriteRegister(MC_MC_28_reg,15,22,0x0);			
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 0, 16, 0xb969);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 0, 15, 0x5c94);
		u5_blend_holdfrm_a[60]--;
	}
	else if(nIdentifiedNum_a[60] == 60)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);	
		WriteRegister(MC_MC_28_reg,14,14,0x0);			
		WriteRegister(MC_MC_28_reg,15,22,0x0);			
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 0, 16, 0xb9ff);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 0, 15, 0x5cff);
		nIdentifiedNum_a[60] = 0;
	}

	//C001 13s
	if( (scalerVIP_Get_MEMCPatternFlag_Identification(4,61) == TRUE || 
		scalerVIP_Get_MEMCPatternFlag_Identification(4,62) == TRUE ||
		scalerVIP_Get_MEMCPatternFlag_Identification(4,64) == TRUE ) &&
		( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ) &&
		 in_fmRate==_PQL_IN_30HZ)
	{
		WriteRegister(KME_LOGO0_KME_LOGO0_A8_reg,0,7,90);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg,0,3,0);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg,4,7,1);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg,8,11,2);
		WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,28,28,1);
		u5_blend_holdfrm_a[61] = 100;
		nIdentifiedNum_a[61]=61;
	}
	else if(u5_blend_holdfrm_a[61] > 0)
	{
		WriteRegister(KME_LOGO0_KME_LOGO0_A8_reg,0,7,90);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg,0,3,0);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg,4,7,1);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg,8,11,2);
		WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,28,28,1);
		u5_blend_holdfrm_a[61]--;
	}
	else if(nIdentifiedNum_a[61] == 61)
	{
		WriteRegister(KME_LOGO0_KME_LOGO0_A8_reg,0,7,32);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg,0,3,0);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg,4,7,3);
		WriteRegister(KME_LOGO0_KME_LOGO0_E4_reg,8,11,5);
		WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,28,28,0);
		nIdentifiedNum_a[61] = 0;
	}

	//--#猫妖传 3:30  
	if((((scalerVIP_Get_MEMCPatternFlag_Identification(4,63) == TRUE) && (special_pattern_transporter!=1))|| scalerVIP_Get_MEMCPatternFlag_Identification(4,29) == TRUE ) &&
		scalerVIP_Get_MEMCPatternFlag_Identification(4,37) == FALSE && ( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,22,22,1);
		u5_blend_holdfrm_a[63] = 30;
		nIdentifiedNum_a[63]=63;
	}
	else if(u5_blend_holdfrm_a[63] > 0)
	{
		WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,22,22,1);
		u5_blend_holdfrm_a[63]--;
	}
	else if(nIdentifiedNum_a[63] == 63)
	{
		WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,22,22,0);
		nIdentifiedNum_a[63] = 0;
	}	

	//--#30003 40s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,66) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(SOFTWARE_SOFTWARE_46_reg,3,3,0);
		u5_blend_holdfrm_a[66] = 100;
		nIdentifiedNum_a[66]=66;
	}
	else if(u5_blend_holdfrm_a[66] > 0)
	{
		WriteRegister(SOFTWARE_SOFTWARE_46_reg,3,3,0);
		u5_blend_holdfrm_a[66]--;
	}
	else if(nIdentifiedNum_a[66] == 66)
	{
		WriteRegister(SOFTWARE_SOFTWARE_46_reg,3,3,1);
		nIdentifiedNum_a[66] = 0;
	}
	
	//--#402 05s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,67) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
		
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x11111111);
		s_pContext_pre->_external_info.u1_402_plane=true;

		u5_blend_holdfrm_a[67] = 50;
		nIdentifiedNum_a[67]=67;
	}
	else if(u5_blend_holdfrm_a[67] > 0)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
		
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x11111111);
		s_pContext_pre->_external_info.u1_402_plane=true;

		u5_blend_holdfrm_a[67]--;
	}
	else if(nIdentifiedNum_a[67] == 67)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x40);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x30);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x10);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x40);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x30);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x10);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x77777777);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x77777777);

		s_pContext_pre->_external_info.u1_402_plane=false;

		nIdentifiedNum_a[67] = 0;
	}

	//--#30018 6s  //--#24003 11s
	if( ( scalerVIP_Get_MEMCPatternFlag_Identification(4,68) == TRUE && scalerVIP_Get_MEMCPatternFlag_Identification(4,58) == FALSE) &&
		( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ) &&
		( (u12_gmv_unconf<=5 && u25_me_aDTL>=800000 && u25_me_aDTL<=1100000 && u1_still_frame==0)||
		  (u11_gmv_mvx>=-22 && u11_gmv_mvx<=-2 && u10_gmv_mvy>=40 && u10_gmv_mvy<=76) ))
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);
		WriteRegister(MC_MC_28_reg,14,14,0x1);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		WriteRegister(SOFTWARE_SOFTWARE_10_reg, 0, 0, 0);
		u5_blend_holdfrm_a[68] = 80;
		nIdentifiedNum_a[68]=68;
	}
	else if(u5_blend_holdfrm_a[68] > 0)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);
		WriteRegister(MC_MC_28_reg,14,14,0x1);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		WriteRegister(SOFTWARE_SOFTWARE_10_reg, 0, 0, 0);
		u5_blend_holdfrm_a[68]--;
	}
	else if(nIdentifiedNum_a[68] == 68)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);
		WriteRegister(MC_MC_28_reg,14,14,0x0);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		WriteRegister(SOFTWARE_SOFTWARE_10_reg, 0, 0, 1);
		nIdentifiedNum_a[68] = 0;
	}

	//--#猫妖传 4:04
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,69) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);
		WriteRegister(MC_MC_28_reg,14,14,0x1);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		WriteRegister(SOFTWARE_SOFTWARE_10_reg, 0, 0, 0);
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_54_reg, 18, 18, 0x0);
		u5_blend_holdfrm_a[69] = 50;
		nIdentifiedNum_a[69]=69;
	}
	else if(u5_blend_holdfrm_a[69] > 0)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);
		WriteRegister(MC_MC_28_reg,14,14,0x1);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		WriteRegister(SOFTWARE_SOFTWARE_10_reg, 0, 0, 0);
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_54_reg, 18, 18, 0x0);
		u5_blend_holdfrm_a[69]--;
	}
	else if(nIdentifiedNum_a[69] == 69)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);
		WriteRegister(MC_MC_28_reg,14,14,0x0);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		WriteRegister(SOFTWARE_SOFTWARE_10_reg, 0, 0, 1);
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_54_reg, 18, 18, 0x1);
		nIdentifiedNum_a[69] = 0;
	}

	//--#0080 0s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,70) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x11111111);

		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg, 8,17, 32);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg,18,27, 32);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_12_reg,16,31, 65532);
		WriteRegister(KME_TOP_KME_TOP_04_reg,29,29, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,28,28, 1);

		u5_blend_holdfrm_a[70] = 50;
		nIdentifiedNum_a[70]=70;
	}
	else if(u5_blend_holdfrm_a[70] > 0)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x11111111);

		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg, 8,17, 32);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg,18,27, 32);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_12_reg,16,31, 65532);
		WriteRegister(KME_TOP_KME_TOP_04_reg,29,29, 0);
		WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,28,28, 1);

		u5_blend_holdfrm_a[70]--;
	}
	else if(nIdentifiedNum_a[70] == 70)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x40);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x30);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x10);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x40);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x30);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x10);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x77777777);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x77777777);

		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg, 8,17, 0);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg,18,27, 0);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_12_reg,16,31, 5);
		WriteRegister(KME_TOP_KME_TOP_04_reg,29,29, 1);
		WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,28,28, 0);

		nIdentifiedNum_a[70] = 0;
	}

	//--#c001 1s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,71) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);
		WriteRegister(MC_MC_28_reg,14,14,0x1);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		u5_blend_holdfrm_a[71] = 50;
		nIdentifiedNum_a[71]=71;
	}
	else if(u5_blend_holdfrm_a[71] > 0)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);
		WriteRegister(MC_MC_28_reg,14,14,0x1);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		u5_blend_holdfrm_a[71]--;
	}
	else if(nIdentifiedNum_a[71] == 71)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);
		WriteRegister(MC_MC_28_reg,14,14,0x0);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		nIdentifiedNum_a[71] = 0;
	}
#if 1
	//--#spiderman 17:03
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4,72) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ) &&
		u11_gmv_mvx<-70&&u11_gmv_mvx>-215&&u10_gmv_mvy<3&&u10_gmv_mvy>-10&&u12_gmv_unconf<55&&u12_gmv_unconf>10&&u1_still_frame==0 && special_pattern_transporter!=1)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);
		WriteRegister(MC_MC_28_reg,14,14,0x1);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		WriteRegister(SOFTWARE_SOFTWARE_10_reg, 0, 0, 0);
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		WriteRegister(SOFTWARE_SOFTWARE_46_reg,3,3,0);
		u5_blend_holdfrm_a[72] = 50;
		nIdentifiedNum_a[72]=72;
	}
	else if(u5_blend_holdfrm_a[72] > 0)
	{
		if(special_pattern_transporter!=1){
			WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);
			WriteRegister(MC_MC_28_reg,14,14,0x1);
			WriteRegister(MC_MC_28_reg,15,22,0x0);
			WriteRegister(SOFTWARE_SOFTWARE_10_reg, 0, 0, 0);
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
			WriteRegister(SOFTWARE_SOFTWARE_46_reg,3,3,0);
		}
		else{
			WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);
			WriteRegister(MC_MC_28_reg,14,14,0x0);
			WriteRegister(MC_MC_28_reg,15,22,0x0);
			WriteRegister(SOFTWARE_SOFTWARE_10_reg, 0, 0, 1);
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x1);
			WriteRegister(SOFTWARE_SOFTWARE_46_reg,3,3,1);
		}
		u5_blend_holdfrm_a[72]--;
	}
	else if(nIdentifiedNum_a[72] == 72)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);
		WriteRegister(MC_MC_28_reg,14,14,0x0);
		WriteRegister(MC_MC_28_reg,15,22,0x0);
		WriteRegister(SOFTWARE_SOFTWARE_10_reg, 0, 0, 1);
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x1);
		WriteRegister(SOFTWARE_SOFTWARE_46_reg,3,3,1);
		nIdentifiedNum_a[72] = 0;
	}
#endif

{
	int left_mv_up_cnt = 0;
	int right_mv_down_cnt = 0;
	int rmv_non_zero_mv_cnt=0;
	for(u8_Index =0;u8_Index<32;u8_Index++)
		rmv_non_zero_mv_cnt++;
	for(u8_Index =0;u8_Index<4;u8_Index++)
	{
		if(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[u8_Index*8]<0)//0,1,8,9,16,17,24,25
			left_mv_up_cnt++;
		else
			left_mv_up_cnt--;
		if(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[u8_Index*8+1]<0)
			left_mv_up_cnt++;
		else
			left_mv_up_cnt--;
		if(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[u8_Index*8+6]>0)//6,7,14,15,22,23,30,31
			right_mv_down_cnt++;
		else
			right_mv_down_cnt--;
		if(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[u8_Index*8+7]>0)
			right_mv_down_cnt++;
		else
			right_mv_down_cnt--;	
	}
	//KTASKWBS-23585
	//if(u32_ID_Log_en==1)
	//	rtd_pr_memc_err("[HUGH] %d,%d,%d,%d\n", u1_still_frame, left_mv_up_cnt, right_mv_down_cnt, s_pContext->_output_read_comreg.u26_me_aAPLi_rb);

	if(/*scalerVIP_Get_MEMCPatternFlag_Identification(2,2) == TRUE &&*/ u11_gmv_mvx < 1 && u10_gmv_mvy < 1 && u1_still_frame == 0 
	&& ((left_mv_up_cnt > 0 && right_mv_down_cnt > 0) || rmv_non_zero_mv_cnt == 0) && in_fmRate == _PQL_IN_60HZ && out_fmRate == _PQL_OUT_120HZ 
	&& s_pContext->_output_read_comreg.u26_me_aAPLi_rb < 80000)		
	{
		WriteRegister(MC_MC_B0_reg,0,2,4);
		u5_blend_holdfrm_a[253] = 60;
		nIdentifiedNum_a[253]=253;
	}
	else if(u5_blend_holdfrm_a[253] > 0)
	{
		WriteRegister(MC_MC_B0_reg,0,2,4);
		u5_blend_holdfrm_a[253]--;
	}
	else if(nIdentifiedNum_a[253]==253)
	{
		WriteRegister(MC_MC_B0_reg,0,2,0);
		nIdentifiedNum_a[253]=0;		
	}
}
	
#if 1
{
	unsigned int read_region_mvx[8];
	int region_mvx[8];
	unsigned char u8_k;
	bool subtitle_flag = false;
	static unsigned char subtitle_cnt = 0;
	static unsigned char subtitle_hold = 0;
	static int sum_mv[6];

	if(MEMC_Lib_get_Adaptive_Stream_Flag()==1 && s_pContext->_external_data._input_frameRate==_PQL_IN_24HZ) {
	
		ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_C0_reg, 0, 10, &read_region_mvx[0]);
		ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_C8_reg, 0, 10, &read_region_mvx[1]);
		ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_D0_reg, 0, 10, &read_region_mvx[2]);
		ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_D8_reg, 0, 10, &read_region_mvx[3]);
		ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_E0_reg, 0, 10, &read_region_mvx[4]);
		ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_E8_reg, 0, 10, &read_region_mvx[5]);
		ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_F0_reg, 0, 10, &read_region_mvx[6]);
		ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_F8_reg, 0, 10, &read_region_mvx[7]);
		
		region_mvx[0] = (read_region_mvx[0]<1024) ? read_region_mvx[0] : read_region_mvx[0] - 2048;
		region_mvx[1] = (read_region_mvx[1]<1024) ? read_region_mvx[1] : read_region_mvx[1] - 2048;
		region_mvx[2] = (read_region_mvx[2]<1024) ? read_region_mvx[2] : read_region_mvx[2] - 2048;
		region_mvx[3] = (read_region_mvx[3]<1024) ? read_region_mvx[3] : read_region_mvx[3] - 2048;
		region_mvx[4] = (read_region_mvx[4]<1024) ? read_region_mvx[4] : read_region_mvx[4] - 2048;
		region_mvx[5] = (read_region_mvx[5]<1024) ? read_region_mvx[5] : read_region_mvx[5] - 2048;
		region_mvx[6] = (read_region_mvx[6]<1024) ? read_region_mvx[6] : read_region_mvx[6] - 2048;
		region_mvx[7] = (read_region_mvx[7]<1024) ? read_region_mvx[7] : read_region_mvx[7] - 2048;	
	
		for(u8_k=1; u8_k<7; u8_k++) {
			if( _ABS_DIFF_(region_mvx[u8_k],region_mvx[u8_k-1]) <= 1 &&
				_ABS_DIFF_(region_mvx[u8_k],region_mvx[u8_k+1]) <= 1 &&
				_ABS_DIFF_(region_mvx[u8_k-1],region_mvx[u8_k+1]) <= 1 &&
				_ABS_(sum_mv[u8_k-1]) < 45 &&
				_ABS_(region_mvx[u8_k-1])>0 && _ABS_(region_mvx[u8_k])>0 && _ABS_(region_mvx[u8_k+1])>0 &&
				_ABS_DIFF_((region_mvx[u8_k-1]+region_mvx[u8_k]+region_mvx[u8_k+1]),sum_mv[u8_k-1]) <= 2){
				subtitle_flag = true;
				break;
			}
		}

		for(u8_k=0; u8_k<6; u8_k++) {
			sum_mv[u8_k] = region_mvx[u8_k]+region_mvx[u8_k+1]+region_mvx[u8_k+2];
		}

		if(subtitle_flag==true){
			subtitle_cnt = (subtitle_cnt<10) ? subtitle_cnt+1 : 10;
		}
		else {
			subtitle_cnt = (subtitle_cnt>0) ? subtitle_cnt-1 : 0;
		}

		if(subtitle_flag==true && subtitle_cnt>3) {
			subtitle_hold = 10;
		}
		else {
			subtitle_hold = (subtitle_hold>0) ? subtitle_hold-1 : 0;
		}

		if(subtitle_hold>0){
	        WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg, 2, 2, 0);
	          
			WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rga_reg, 0, 9,1023);
			WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rga_reg,20,25,0);
			WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rga_reg,26,31,63);
			
			WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rgb_reg, 0, 9,1023);
			WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rgb_reg,20,25,0);
			WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rgb_reg,26,31,63);
			
			WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rgc_reg, 0, 9,1023);
			WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rgc_reg,20,25,0);
			WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rgc_reg,26,31,63);
			
			WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rgd_reg, 0, 9,1023);
			WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rgd_reg,20,25,0);
			WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rgd_reg,26,31,63);
			
			WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rge_reg, 0, 9,1023);
			WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rge_reg,20,25,0);
			WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rge_reg,26,31,63);
			
			WriteRegister(KME_LOGO0_KME_LOGO0_AC_reg,16,23,200);
			WriteRegister(KME_LOGO0_KME_LOGO0_AC_reg,24,31,200);
			WriteRegister(KME_LOGO0_KME_LOGO0_B0_reg, 0, 7,200);
			WriteRegister(KME_LOGO0_KME_LOGO0_B0_reg, 8,15,200);
			WriteRegister(KME_LOGO0_KME_LOGO0_B0_reg,16,23,200);
		}
		else{
	        WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg, 2, 2, 1);
	          
			WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rga_reg, 0, 9,255);
			WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rga_reg,20,25,4);
			WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rga_reg,26,31,3);
			
			WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rgb_reg, 0, 9,255);
			WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rgb_reg,20,25,4);
			WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rgb_reg,26,31,20);
			
			WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rgc_reg, 0, 9,255);
			WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rgc_reg,20,25,2);
			WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rgc_reg,26,31,8);
			
			WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rgd_reg, 0, 9,255);
			WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rgd_reg,20,25,4);
			WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rgd_reg,26,31,20);
			
			WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rge_reg, 0, 9,255);
			WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rge_reg,20,25,4);
			WriteRegister(KME_DEHALO6_dh_logo_ctrl_1_rge_reg,26,31,3);
			
			WriteRegister(KME_LOGO0_KME_LOGO0_AC_reg,16,23,32);
			WriteRegister(KME_LOGO0_KME_LOGO0_AC_reg,24,31,32);
			WriteRegister(KME_LOGO0_KME_LOGO0_B0_reg, 0, 7,32);
			WriteRegister(KME_LOGO0_KME_LOGO0_B0_reg, 8,15,32);
			WriteRegister(KME_LOGO0_KME_LOGO0_B0_reg,16,23,32);
		}

		if(deleteVal&0x400000) {//bit22
			rtd_pr_memc_emerg("f:%d c:%d h:%d [%d,%d,%d,%d,%d,%d,%d,%d][%d,%d,%d,%d,%d,%d,%d]\n",
				subtitle_flag, subtitle_cnt, subtitle_hold,
				region_mvx[0], region_mvx[1], region_mvx[2], region_mvx[3],
				region_mvx[4], region_mvx[5], region_mvx[6], region_mvx[7], 
				sum_mv[0],sum_mv[1],sum_mv[2],sum_mv[3],sum_mv[4],sum_mv[5],sum_mv[6]);
		}
	}
}
#endif



#if 1 // player OSD
{
#define APL_TH	(90000)

	unsigned int TopApl[8];
	unsigned int BotApl[8];
	unsigned int CenApl[16];
	unsigned int SumTop=0, SumBot=0, SumCen=0;
	unsigned int TempApl=0;
	unsigned char u8_i, u8_AplCnt = 0;
	static unsigned char u8_OSD_cnt=0;
	bool DetectOSD = false;
	unsigned int times = 28;
	unsigned short roughValid  = s_pContext->_output_read_comreg.u1_BBD_roughValid_rb[_RIM_BOT];
	unsigned short fineValid  = s_pContext->_output_read_comreg.u1_BBD_fineValid_rb[_RIM_BOT];
	unsigned short roughBotRim = s_pContext->_output_read_comreg.u12_BBD_roughRim_rb[_RIM_BOT];
	unsigned short fineBotRim = s_pContext->_output_read_comreg.u12_BBD_fineRim_rb[_RIM_BOT];
	unsigned short BotRim = fineValid == 1 ? fineBotRim : (roughValid == 1? roughBotRim : s_pContext->_output_rimctrl.u12_out_resolution[_RIM_BOT]);

	if(!(deleteVal&0x8)) {//bit3			
				  
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_80_reg, 0, 19, &TopApl[0]);
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_84_reg, 0, 19, &TopApl[1]);
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_88_reg, 0, 19, &TopApl[2]);
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_8C_reg, 0, 19, &TopApl[3]);
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_90_reg, 0, 19, &TopApl[4]);
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_94_reg, 0, 19, &TopApl[5]);
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_98_reg, 0, 19, &TopApl[6]);
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_9C_reg, 0, 19, &TopApl[7]);

		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_A0_reg, 0, 19, &CenApl[0]);
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_A4_reg, 0, 19, &CenApl[1]);
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_A8_reg, 0, 19, &CenApl[2]);
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_AC_reg, 0, 19, &CenApl[3]);
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_B0_reg, 0, 19, &CenApl[4]);
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_B4_reg, 0, 19, &CenApl[5]);
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_B8_reg, 0, 19, &CenApl[6]);
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_BC_reg, 0, 19, &CenApl[7]);
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_C0_reg, 0, 19, &CenApl[8]);
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_C4_reg, 0, 19, &CenApl[9]);
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_C8_reg, 0, 19, &CenApl[10]);
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_CC_reg, 0, 19, &CenApl[11]);
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_D0_reg, 0, 19, &CenApl[12]);
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_D4_reg, 0, 19, &CenApl[13]);
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_D8_reg, 0, 19, &CenApl[14]);
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_DC_reg, 0, 19, &CenApl[15]);

		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_E0_reg, 0, 19, &BotApl[0]);
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_E4_reg, 0, 19, &BotApl[1]);
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_E8_reg, 0, 19, &BotApl[2]);
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_EC_reg, 0, 19, &BotApl[3]);
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_F0_reg, 0, 19, &BotApl[4]);
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_F4_reg, 0, 19, &BotApl[5]);
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_F8_reg, 0, 19, &BotApl[6]);
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_FC_reg, 0, 19, &BotApl[7]);

		for(u8_i=0; u8_i<8; u8_i++){
			if(TopApl[u8_i]<APL_TH){
				u8_AplCnt++;
			}
			if(BotApl[u8_i]<APL_TH){
				u8_AplCnt++;
			}
		}

		if(u8_AplCnt>=14){
			SumTop = TopApl[0]+TopApl[1]+TopApl[2]+TopApl[3]+TopApl[4]+TopApl[5]+TopApl[6]+TopApl[7];
			SumBot = BotApl[0]+BotApl[1]+BotApl[2]+BotApl[3]+BotApl[4]+BotApl[5]+BotApl[6]+BotApl[7];
			SumCen = CenApl[0]+CenApl[1]+CenApl[2]+CenApl[3]+CenApl[4]+CenApl[5]+CenApl[6]+CenApl[7]+
					 CenApl[8]+CenApl[9]+CenApl[10]+CenApl[11]+CenApl[12]+CenApl[13]+CenApl[14]+CenApl[15];
			TempApl = (SumCen>>5)*11; // SumCen*7/20
			if( ( (TempApl>SumTop&&TempApl>SumBot) || (((SumCen>>2)>SumTop)&&(((SumCen>>6)*29)>SumBot)) )&&
				BotRim>1950&& SumCen<=2200000&&u25_me_aDTL<1700000&&
				scalerVIP_Get_MEMCPatternFlag_Identification(4,17) == FALSE &&scalerVIP_Get_MEMCPatternFlag_Identification(4,18) == FALSE ){
				DetectOSD = true;
				u8_OSD_cnt = (u8_OSD_cnt>=times) ? times : u8_OSD_cnt+1;
			}
			else{
				DetectOSD = false;
				u8_OSD_cnt = (u8_OSD_cnt<=2) ? 0 : u8_OSD_cnt-2;
			}
		}
		else {
			DetectOSD = false;
			u8_OSD_cnt = (u8_OSD_cnt<=2) ? 0 : u8_OSD_cnt-2;
		}

		if(DetectOSD && u8_OSD_cnt<times ){
			WriteRegister(KME_LOGO1_KME_LOGO1_14_reg,  8, 27, 0x22222);
			WriteRegister(KME_LOGO1_KME_LOGO1_1C_reg,  8, 27, 0x22222);

			WriteRegister(HARDWARE_HARDWARE_00_reg, 0, 0, 0x0);
			if(deleteVal&0x4000) {//bit14
				rtd_pr_memc_emerg("[SET]   [%d,%d,%d] BOT:%d D:%d Cnt:%d Times%d Set:%d\n", 
					SumTop, SumBot, (SumCen>>1),
					BotRim, DetectOSD, u8_OSD_cnt, times, 
					(DetectOSD && u8_OSD_cnt<times) ? 1 : 0);
			}
		}
		else {
			WriteRegister(KME_LOGO1_KME_LOGO1_14_reg,  8, 27, 0x78887);
			WriteRegister(KME_LOGO1_KME_LOGO1_1C_reg,  8, 27, 0x77777);
			
			WriteRegister(HARDWARE_HARDWARE_00_reg, 0, 0, 0x1);
			if(deleteVal&0x4000) {//bit14
				rtd_pr_memc_emerg("[%d,%d,%d] BOT:%d D:%d Cnt:%d Times%d Set:%d\n", SumTop, SumBot, (SumCen>>1),BotRim, DetectOSD, u8_OSD_cnt, times, (DetectOSD && u8_OSD_cnt<times) ? 1 : 0);
			}
		}

		
#if 0
		if(DetectOSD && u11_gmv_mvx>=-1 && u11_gmv_mvx<=3 && u10_gmv_mvy>=27 && u10_gmv_mvy<=76 && u1_still_frame == 0) {
			WriteRegister(0xb809b890,31,31,1);
			WriteRegister(0xb809c72c,25,21,3);
			WriteRegister(0xb809c730,16,16,1);
			WriteRegister(0xb809c730,17,17,0);
			WriteRegister(0xb809c730,23,18,2);
			WriteRegister(0xb809c730,31,27,3);
			WriteRegister(0xb809c734,19,19,1);
			WriteRegister(0xb809c734,20,20,0);
			WriteRegister(0xb809c734,26,21,2);
			WriteRegister(HARDWARE_HARDWARE_43_reg,30,30,1);

			if(deleteVal&0x4000) {//bit14
				rtd_pr_memc_emerg("[SET]   [%d][%d][%d]\n", 
					u11_gmv_mvx, u10_gmv_mvy, u1_still_frame);
			}
		}
		else {
			WriteRegister(0xb809b890,31,31,0);
			WriteRegister(0xb809c72c,25,21,0);
			WriteRegister(0xb809c730,16,16,0);
			WriteRegister(0xb809c730,17,17,0);
			WriteRegister(0xb809c730,23,18,0);
			WriteRegister(0xb809c730,31,27,0);
			WriteRegister(0xb809c734,19,19,0);
			WriteRegister(0xb809c734,20,20,0);
			WriteRegister(0xb809c734,26,21,0);
			WriteRegister(HARDWARE_HARDWARE_43_reg,30,30,0);

			if(deleteVal&0x4000) {//bit14
				rtd_pr_memc_emerg("[%d][%d][%d]\n", 
					u11_gmv_mvx, u10_gmv_mvy, u1_still_frame);
			}
		}
		#endif
	}

}
#endif

#if 1 //moving text
{
	unsigned char region_max[4], region_min[4];
	int sum_my;
	bool MovingText=false;
	static char u8_cnt = 0;

	region_max[0] = _MAX_( _MAX_(_MAX_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[0],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[1]),
							_MAX_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[2],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[3])),
					  _MAX_(_MAX_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[4],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[5]),
							_MAX_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[6],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[7])) );

	region_max[1] = _MAX_( _MAX_(_MAX_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[8],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[9]),
							_MAX_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[10],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[11])),
					  _MAX_(_MAX_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[12],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[13]),
							_MAX_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[14],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[15])) );

	region_max[2] = _MAX_( _MAX_(_MAX_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[16],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[17]),
							_MAX_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[18],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[19])),
					  _MAX_(_MAX_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[20],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[21]),
							_MAX_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[22],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[23])) );

	region_max[3] = _MAX_( _MAX_(_MAX_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[24],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[25]),
							_MAX_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[26],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[27])),
					  _MAX_(_MAX_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[28],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[29]),
							_MAX_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[30],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[31])) );


	region_min[0] = _MIN_( _MIN_(_MIN_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[0],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[1]),
						   _MIN_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[2],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[3])),
					 _MIN_(_MIN_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[4],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[5]),
						   _MIN_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[6],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[7])) );
	
	region_min[1] = _MIN_( _MIN_(_MIN_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[8],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[9]),
							_MIN_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[10],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[11])),
					  _MIN_(_MIN_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[12],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[13]),
							_MIN_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[14],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[15])) );

	region_min[2] = _MIN_( _MIN_(_MIN_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[16],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[17]),
							_MIN_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[18],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[19])),
					  _MIN_(_MIN_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[20],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[21]),
							_MIN_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[22],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[23])) );

	region_min[3] = _MIN_( _MIN_(_MIN_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[24],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[25]),
							_MIN_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[26],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[27])),
					  _MIN_(_MIN_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[28],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[29]),
							_MIN_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[30],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[31])) );

	sum_my = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[0] + s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[1] +
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[2] + s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[3] +
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[4] + s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[5] +
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[6] + s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[7] +
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[8] + s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[9] +
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[10] + s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[11] +
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[12] + s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[13] +
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[14] + s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[15] +
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[16] + s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[17] +
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[18] + s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[19] +
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[20] + s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[21] +
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[22] + s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[23] +
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[24] + s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[25] +
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[26] + s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[27] +
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[28] + s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[29] +
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[30] + s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[31];

	if( region_max[0]<=50&&region_min[0]<=12 && region_max[1]<=102&&region_min[1]<=21 && 
		region_max[2]<=102&&region_min[2]<=12 && region_max[3]<=102&&region_min[3]<=15 && region_max[3]>=98 &&
		region_max[0]!=0&&region_max[1]!=0&&region_max[2]!=0&&region_max[3]!=0&&_ABS_(sum_my)<=3) {
		MovingText = true;
		
	}

	if(MovingText) {
		u8_cnt = (u8_cnt>=10) ? 10 : u8_cnt+1;
	}
	else {
		u8_cnt = (u8_cnt<=1) ? 0 : u8_cnt-1;
	}

	if(deleteVal&0x4000000) {//bit26
		rtd_pr_memc_emerg("[%d,%d,%d,%d] [%d,%d,%d,%d] %d %d %d\n", 
			region_max[0], region_max[1], region_max[2], region_max[3],
			region_min[0], region_min[1], region_min[2], region_min[3], sum_my, MovingText, u8_cnt);
	}
	
	if(u8_cnt>5 &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg, 8,17,16);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg,18,27,16);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,1,1,0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,3,3,0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,4,4,0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,5,5,0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,6,6,0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,7,7,0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,8,8,0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,0,0,0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,3,3,0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,5,5,0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,6,6,0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,7,7,0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,8,8,0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,12,12,0);

		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x99999999);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x99999999);

		WriteRegister(HARDWARE_HARDWARE_00_reg,0,0,0);
		u5_blend_holdfrm_a[254] = 30;
		nIdentifiedNum_a[254]=254;
	}
	else if(u5_blend_holdfrm_a[254] > 0)
	{
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg, 8,17,16);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg,18,27,16);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,1,1,0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,3,3,0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,4,4,0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,5,5,0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,6,6,0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,7,7,0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,8,8,0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,0,0,0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,3,3,0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,5,5,0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,6,6,0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,7,7,0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,8,8,0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,12,12,0);

		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x99999999);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x99999999);
		WriteRegister(HARDWARE_HARDWARE_00_reg,0,0,0);
		u5_blend_holdfrm_a[254]--;
	}
	else if(nIdentifiedNum_a[254] == 254)
	{
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg, 8,17,0);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg,18,27,0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,1,1,1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,3,3,1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,4,4,1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,5,5,1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,6,6,1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,7,7,1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,8,8,1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,0,0,1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,3,3,1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,5,5,1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,6,6,1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,7,7,1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,8,8,1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,12,12,1);

		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x40);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x30);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x10);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x40);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x30);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x10);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0,31,0x77777777);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0,31,0x77777777);

		WriteRegister(HARDWARE_HARDWARE_00_reg,0,0,1);
		nIdentifiedNum_a[254] = 0;
	}
}
#endif

	
#if 1
	//===debug show
	for(u8_Index =0;u8_Index<255;u8_Index++)
	{
		if(nIdentifiedNum_a[u8_Index] !=0){
			pOutput->u1_IP_wrtAction_true=1;
			if(u32_ID_Log_en==1) //d758 bit24
				rtd_pr_memc_emerg("[IPR_010]nIdentifiedNum_a>>[%d][%d]	 ,\n", u8_Index, nIdentifiedNum_a[u8_Index]);
		}
	}
#endif

}

VOID Identification_Pattern_wrtAction_TV011(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	_PQLPARAMETER *PatternParam = GetPQLParameter();
	static unsigned char u5_blend_holdfrm_a[255] = {0};
	static unsigned char nIdentifiedNum_a[255] = {0};
	//unsigned int mot_diff = s_pContext->_output_filmDetectctrl.u27_ipme_motionPool[_FILM_ALL][0];
	//static unsigned char nOnceFlag=12;
	unsigned char u8_Index;
	static bool monosco_flag = 0;
#if Pro_TCL
	//unsigned char  u8_cadence_Id = s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL];
	//PQL_INPUT_FRAME_RATE in_fmRate = s_pContext->_external_data._input_frameRate;
	unsigned char nblend_y_alpha_Target=0x90;
	unsigned char nblend_uv_alpha_Target=0x70;
	unsigned char nblend_logo_y_alpha_Target=0xd0;
	unsigned char nblend_logo_uv_alpha_Target=0x30;
#endif

	//unsigned int u32_RB_Value;
	signed short u11_gmv_mvx = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
	signed short u10_gmv_mvy = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;
	//unsigned int u25_me_aDTL = s_pContext->_output_read_comreg.u25_me_aDTL_rb;
	//unsigned int u26_aAPLi_rb = s_pContext->_output_read_comreg.u26_me_aAPLi_rb;
	//unsigned int u32_pattern298_zidoo_player_forbindden_en;

	//unsigned char  u8_cadence_Id = s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL];
	//unsigned char  u8_Mixmode_flag = s_pContext->_output_filmDetectctrl.u1_mixMode;
    //int gmv_mvx =0;//, gmv_mvy =0 ;
	//PQL_INPUT_FRAME_RATE in_fmRate = s_pContext->_external_data._input_frameRate;


#if 1 //for YE Test temp test
	unsigned int u32_ID_Log_en=0;//,ES_Test_Value=0;
	ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 24,  24, &u32_ID_Log_en);//YE Test debug dummy register
#endif

#if 0
{
	unsigned int deleteVal = 0;
	deleteVal = rtd_inl(HARDWARE_HARDWARE_43_reg);//0xB809D5AC

	if(deleteVal & 0x8000) {//bit15
	rtd_pr_memc_emerg("mvx:%d mvy:%d sf:%d aDTL:%d unconf:%d\n", 
	u11_gmv_mvx, u10_gmv_mvy, 
	s_pContext->_output_frc_sceneAnalysis.u1_still_frame, s_pContext->_output_read_comreg.u25_me_aDTL_rb, 
	s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb );
	}
}
#endif


	if((pParam->u1_Identification_Pattern_en== 0)||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_Test_0_flag ==0))
		return;

	//reset
	pOutput->u1_IP_wrtAction_true=0;


	//--#02-11_04_databroken_pirate
		if(scalerVIP_Get_MEMCPatternFlag_Identification(3,1) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
		{
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
			u5_blend_holdfrm_a[1] = 8;
			nIdentifiedNum_a[1]=1;
		}
		else if(u5_blend_holdfrm_a[1] > 0)
		{
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
			u5_blend_holdfrm_a[1]--;
		}
		else if(nIdentifiedNum_a[1] == 1 )
		{
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x1);
			nIdentifiedNum_a[1] = 0;
		}
	
		//--#04_Montage_P 5s
		if( scalerVIP_Get_MEMCPatternFlag_Identification(3,2) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 )
			&& u11_gmv_mvx>=20 && u11_gmv_mvx<=45 && u10_gmv_mvy >= 0 && u10_gmv_mvy <= -10 )
		{
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
	
			WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
				
			WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x1);
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 255);	 
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 230);
			WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0);
	
			WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 0);
	
			WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0x0);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x1);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x1);
	
			u5_blend_holdfrm_a[2] = 50;
			nIdentifiedNum_a[2]=2;
		}
		else if(u5_blend_holdfrm_a[2] > 0)
		{
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
	
			WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
			
			WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x1);
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 255);	 
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 230);
			WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0);
	
			WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 0);
	
			WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0x0);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x1);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x1);
	
			u5_blend_holdfrm_a[2]--;
		}
		else if(nIdentifiedNum_a[2] == 2 )
		{
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 64);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 48);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 32);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 16);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0);
	
			WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 64);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 48);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 32);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 16);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0);
			
			WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x0);
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 0xff);	  
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 0x0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 1);
	
			WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 1);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 1);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 1);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 1);
	
			WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0xffffffff);
			WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0xffffffff);
			WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0xffffffff);
			WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0xffffffff);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x7);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x7);
	
			nIdentifiedNum_a[2] = 0;
		}
	
		//--#04_Montage_P 46s
		if(scalerVIP_Get_MEMCPatternFlag_Identification(3,3) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
		{
			WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg, 26, 30, 10);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 16, 21, 11);
			WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg, 4, 13, 290);
			
			WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x1);
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 255);	 
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 230);
			WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0);
			
			WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 0);
			u5_blend_holdfrm_a[3] = 80;
			nIdentifiedNum_a[3]=3;
		}
		else if(u5_blend_holdfrm_a[3] > 0)
		{
			WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg, 26, 30, 10);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 16, 21, 11);
			WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg, 4, 13, 290);
			
			WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x1);
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 255);	 
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 230);
			WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0);
			
			WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 0);
			u5_blend_holdfrm_a[3]--;
		}
		else if(nIdentifiedNum_a[3] == 3 )
		{
			WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg, 26, 30, 0x16);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 16, 21, 0xb);
			WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg, 4, 13, 0x2bc);
			
			WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x0);
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 0xff);	  
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 0x0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 1);
			
			WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 1);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 1);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 1);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 1);
			nIdentifiedNum_a[3] = 0;
		}
	
		//--#04_DNM hrnm 720p60 5s
		if(scalerVIP_Get_MEMCPatternFlag_Identification(3,4) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
		{
			WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0x0);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x1);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x1);
			u5_blend_holdfrm_a[4] = 20;
			nIdentifiedNum_a[4]=4;
		}
		else if(u5_blend_holdfrm_a[4] > 0)
		{
			WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0x0);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x1);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x1);
			u5_blend_holdfrm_a[4]--;
		}
		else if(nIdentifiedNum_a[4] == 4 )
		{
			WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0xffffffff);
			WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0xffffffff);
			WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0xffffffff);
			WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0xffffffff);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x7);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x7);
			nIdentifiedNum_a[4] = 0;
		}
	
		//--#12-DNM hrnm 720p60 21s
		if(scalerVIP_Get_MEMCPatternFlag_Identification(3,5) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
		{
			WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x1);
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 255);	 
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 230);
			WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0);
			
			WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 0);
			u5_blend_holdfrm_a[5] = 80;
			nIdentifiedNum_a[5]=5;
		}
		else if(u5_blend_holdfrm_a[5] > 0)
		{
			WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x1);
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 255);	 
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 230);
			WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0);
			
			WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 0);
			u5_blend_holdfrm_a[5]--;
		}
		else if(nIdentifiedNum_a[5] == 5 )
		{
			WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0);
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 0xff);	  
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 1);
			
			WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 1);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 1);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 1);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 1);
			nIdentifiedNum_a[5] = 0;
		}
	
		//--#12-DNM hrnm 720p60 17s
		if(scalerVIP_Get_MEMCPatternFlag_Identification(3,6) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
		{
			WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0x0);
			u5_blend_holdfrm_a[6] = 8;
			nIdentifiedNum_a[6]=6;
		}
		else if(u5_blend_holdfrm_a[6] > 0)
		{
			WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0x0);
			u5_blend_holdfrm_a[6]--;
		}
		else if(nIdentifiedNum_a[6] == 6 )
		{
			WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0xffffffff);
			WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0xffffffff);
			WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0xffffffff);
			WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0xffffffff);
			nIdentifiedNum_a[6] = 0;
		}
	
		//--#12-DNM hrnm 720p60 128s
		if(scalerVIP_Get_MEMCPatternFlag_Identification(3,7) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
		{
			WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0x0);
	
			WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);		// local FB
			WriteRegister(MC_MC_28_reg,14,14,0x1);	// local FB
			WriteRegister(MC_MC_28_reg,15,22,0x0);	// local FB
			u5_blend_holdfrm_a[7] = 8;
			nIdentifiedNum_a[7]=7;
		}
		else if(u5_blend_holdfrm_a[7] > 0)
		{
			WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0x0);
	
			WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);		// local FB
			WriteRegister(MC_MC_28_reg,14,14,0x1);	// local FB
			WriteRegister(MC_MC_28_reg,15,22,0x0);	// local FB
			u5_blend_holdfrm_a[7]--;
		}
		else if(nIdentifiedNum_a[7] == 7)
		{
			WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0xffffffff);
			WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0xffffffff);
			WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0xffffffff);
			WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0xffffffff);
	
			WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);		// local FB
			WriteRegister(MC_MC_28_reg,14,14,0x0);	// local FB
			WriteRegister(MC_MC_28_reg,15,22,0x0);	// local FB
			nIdentifiedNum_a[7] = 0;
		}
		
		//--#15-HD_H_R_32_i
		if(scalerVIP_Get_MEMCPatternFlag_Identification(3,8) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 )
			&& s_pContext->_output_read_comreg.u25_me_aDTL_rb > 1290000
			&& u11_gmv_mvx <=2 && u11_gmv_mvx >= 0 && u10_gmv_mvy>=-1 && u10_gmv_mvy<=1)
		{
			WriteRegister(MC2_MC2_50_reg,0,1,0x0);
			PatternParam->_param_read_comreg.pattern_golf_flag = 1;
	
			WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31,1);
			WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,0);
			WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);
			WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,8,15,240);
			WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,24,31,135);
			WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,5);
			WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,5);
			WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,8);
			WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,5);
			WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,1);
			WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,10);
			WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,1);
			WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,6);
			WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,1);
			WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,1);
			WriteRegister(KME_ME1_BG0_ME1_BG_13_reg,26,26,0);
	
			WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,0x2);
			WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x32);
			WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff);
			WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0x5);
			WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,8,14,0xc);
			WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0x5);
			WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,24,30,0xc);
			WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x32);
			WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,0x0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,7,7,0x0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,0x0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,0,0,0x0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xa);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,24,29,0xa);
			WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,0x1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,0,7,0x0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x50);
			WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,0x1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,16,23,0x82);
			WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,24,31,0xff);
			WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x0);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x2);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x2);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x2);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x2);
	
			WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 0);
	
			WriteRegister(COLOR_D_VC_Global_CTRL_reg, 0, 0, 0);// local contrast
			WriteRegister(COLOR_D_VC_Global_CTRL_reg, 21, 21, 0);// local contrast
	
			u5_blend_holdfrm_a[8] = 8;
			nIdentifiedNum_a[8]=8;
		}
		else if(u5_blend_holdfrm_a[8] > 0)
		{
			WriteRegister(MC2_MC2_50_reg,0,1,0x0);
			PatternParam->_param_read_comreg.pattern_golf_flag = 1;
	
			WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31,1);
			WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,0);
			WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);
			WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,8,15,240);
			WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,24,31,135);
			WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,5);
			WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,5);
			WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,8);
			WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,5);
			WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,1);
			WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,10);
			WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,1);
			WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,6);
			WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,1);
			WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,1);
			WriteRegister(KME_ME1_BG0_ME1_BG_13_reg,26,26,0);
	
			WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,0x2);
			WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x32);
			WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,0x3ff);
			WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0x5);
			WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,8,14,0xc);
			WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0x5);
			WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,24,30,0xc);
			WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,0x32);
			WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,0x0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,7,7,0x0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,0x0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,0,0,0x0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,0xa);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,24,29,0xa);
			WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,0x1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,0,7,0x0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x50);
			WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,0x1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,16,23,0x82);
			WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,24,31,0xff);
			WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x0);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x2);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x2);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x2);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x2);
	
			WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 0);
	
			WriteRegister(COLOR_D_VC_Global_CTRL_reg, 0, 0, 0);// local contrast
			WriteRegister(COLOR_D_VC_Global_CTRL_reg, 21, 21, 0);// local contrast
			u5_blend_holdfrm_a[8]--;
		}
		else if(nIdentifiedNum_a[8] == 8 )
		{
			WriteRegister(MC2_MC2_50_reg,0,1,0x1);
			PatternParam->_param_read_comreg.pattern_golf_flag = 0;
	
			WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,31,31,0);
			WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,2);
			WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0);
			WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,8,15,0);
			WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg,24,31,0);
			WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,12);
			WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,12);
			WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,20,29,25);
			WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,12);
			WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,13,17,4);
			WriteRegister(KME_ME1_BG0_ME1_BG_1_reg,8,12,16);
			WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,13,17,10);
			WriteRegister(KME_ME1_BG0_ME1_BG_2_reg,8,12,20);
			WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,30,30,1);
			WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,31,31,1);
			WriteRegister(KME_ME1_BG0_ME1_BG_13_reg,26,26,1);
	
			WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,10);
			WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,900);
			WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25,1023);
			WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,12);
			WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,8,14,5);
			WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,12);
			WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,24,30,5);
			WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg,4,13,700);
			WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg,15,15,1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,7,7,1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,6,6,1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,0,0,1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,16,21,20);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg,24,29,40);
			WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,16,16,0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,0,7,0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,34);
			WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,17,17,0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,16,23,150);
			WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,24,31,255);
			WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,1);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0);
	
			WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 1);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 1);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 1);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 1);
	
			WriteRegister(COLOR_D_VC_Global_CTRL_reg, 0, 0, 1);// local contrast
			WriteRegister(COLOR_D_VC_Global_CTRL_reg, 21, 21, 1); // local contrast
			nIdentifiedNum_a[8] = 0;
		}
#if 0
		//--#16-H_Move_fast
		if(scalerVIP_Get_MEMCPatternFlag_Identification(3,9) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
		{
			WriteRegister(HARDWARE_HARDWARE_26_reg, 20, 20, 0x1);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0x0);//D560
			WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0x0);//D560
	
			WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg, 8, 15, 0x0);//EF3C
			WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg, 24, 31, 0x0);//EF3C
	
			WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0x0);//CDFC
			WriteRegister(HARDWARE_HARDWARE_57_reg, 7, 7, 0x0);//D5E4
	
			WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg, 10, 13, 0xf);//EB74
			WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 0, 6, 0x1f);//EB78
			WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 8, 14, 0x12);//EB78
			WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 16, 22, 0x8);//EB78
			WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 24, 30, 0x13);//EB78
	
			WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg, 4, 13, 0x12c);//EBAC
			WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0x0);//EB14
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0x0);//EB18
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0x0);//EB18
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 16, 21, 0xa);//EB18
			WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0x0);//EBE4
			WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0x0);//EBE0
	
			WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg, 0, 9, 0x1f);//EB20
			WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg, 0, 4, 0x8);//EB10
			WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg, 8, 12, 0x4);//EB10
			WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg, 3, 3, 0x0);//EBD0
			WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg, 4, 4, 0x0);//EBD0
			WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg, 16, 25, 0x1f);//EB94  
			u5_blend_holdfrm_a[9] = 120;
			nIdentifiedNum_a[9]=9;
		}
		else if(u5_blend_holdfrm_a[9] > 0)
		{
			WriteRegister(HARDWARE_HARDWARE_26_reg, 20, 20, 0x1);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0x0);//D560
			WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0x0);//D560
	
			WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg, 8, 15, 0x0);//EF3C
			WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg, 24, 31, 0x0);//EF3C
	
			WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0x0);//CDFC
			WriteRegister(HARDWARE_HARDWARE_57_reg, 7, 7, 0x0);//D5E4
	
			WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg, 10, 13, 0xf);//EB74
			WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 0, 6, 0x1f);//EB78
			WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 8, 14, 0x12);//EB78
			WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 16, 22, 0x8);//EB78
			WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 24, 30, 0x13);//EB78
	
			WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg, 4, 13, 0x12c);//EBAC
			WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0x0);//EB14
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0x0);//EB18
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0x0);//EB18
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 16, 21, 0xa);//EB18
			WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0x0);//EBE4
			WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0x0);//EBE0
	
			WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg, 0, 9, 0x1f);//EB20
			WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg, 0, 4, 0x8);//EB10
			WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg, 8, 12, 0x4);//EB10
			WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg, 3, 3, 0x0);//EBD0
			WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg, 4, 4, 0x0);//EBD0
			WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg, 16, 25, 0x1f);//EB94  
			u5_blend_holdfrm_a[9]--;
		}
		else if(nIdentifiedNum_a[9] == 9 )
		{
			WriteRegister(HARDWARE_HARDWARE_26_reg, 20, 20, 0x0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0x1);//D560
			WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0x1);//D560
	
			WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg, 8, 15, 0x0);//EF3C
			WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg, 24, 31, 0x0);//EF3C
	
			WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0x80);//CDFC
			WriteRegister(HARDWARE_HARDWARE_57_reg, 7, 7, 0x1);//D5E4
	
			WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg, 10, 13, 0x0);//EB74
			WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 0, 6, 0x0c);//EB78
			WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 8, 14, 0x05);//EB78
			WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 16, 22, 0x0c);//EB78
			WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 24, 30, 0x05);//EB78
	
			WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg, 4, 13, 0x2bc);//EBAC
			WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0x1);//EB14
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0x1);//EB18
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0x1);//EB18
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 16, 21, 0x14);//EB18
			WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0x1);//EBE4
			WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0x0);//EBE0
	
			WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg, 0, 9, 0x15);//EB20
			WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg, 0, 4, 0x4);//EB10
			WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg, 8, 12, 0x4);//EB10
			WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg, 3, 3, 0x1);//EBD0
			WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg, 4, 4, 0x1);//EBD0
			WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg, 16, 25, 0x14);//EB94 
			nIdentifiedNum_a[9] = 0;
		}
#endif
	
#if 1
	{
		//--#16-H_Move_fast
		//dehalo enable, close local FB
		unsigned char positive_rmv_counter = 0;
		int i = 0;
		for(i = 0; i < 16; i++)
		{
			if(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i] > 0)
				positive_rmv_counter++;
		}
		if(scalerVIP_Get_MEMCPatternFlag_Identification(3,9) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
		{
			if(positive_rmv_counter < 4)
			{
				/*
				//WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
				WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);		// local FB
				WriteRegister(MC_MC_28_reg,14,14,0x1);	// local FB
				WriteRegister(MC_MC_28_reg,15,22,0x0);	// local FB
				WriteRegister(HARDWARE_HARDWARE_06_reg,0,1,0x1);		// dehalo
				WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,4,0);
				
				WriteRegister(KME_TOP_KME_TOP_04_reg,29,29,1);//meander disble
				*/
				WriteRegister(HARDWARE_HARDWARE_26_reg, 20, 20, 0x1);
				WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0x0);//D560
				WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0x0);//D560
	
				WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg, 8, 15, 0x0);//EF3C
				WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg, 24, 31, 0x0);//EF3C
	
				WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0x0);//CDFC
				WriteRegister(HARDWARE_HARDWARE_57_reg, 7, 7, 0x0);//D5E4
	
				WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg, 10, 13, 0xf);//EB74
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 0, 6, 0x1f);//EB78
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 8, 14, 0x12);//EB78
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 16, 22, 0x8);//EB78
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 24, 30, 0x13);//EB78
	
				WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg, 4, 13, 0x12c);//EBAC
				WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0x0);//EB14
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0x0);//EB18
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0x0);//EB18
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 16, 21, 0xa);//EB18
				WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0x0);//EBE4
				WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0x0);//EBE0
	
				WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg, 0, 9, 0x1f);//EB20
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg, 0, 4, 0x8);//EB10
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg, 8, 12, 0x4);//EB10
				WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg, 3, 3, 0x0);//EBD0
				WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg, 4, 4, 0x0);//EBD0
				WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg, 16, 25, 0x1f);//EB94
	
				WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);
				WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x1);
				WriteRegister(MC_MC_28_reg,14,14,0x0);
				WriteRegister(MC_MC_28_reg,15,22,0x0);
				WriteRegister(HARDWARE_HARDWARE_06_reg,0,1,0x1);		// dehalo
				WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,4,1);
				WriteRegister(KME_TOP_KME_TOP_04_reg,29,29,1);
			}
			else {
	
				WriteRegister(HARDWARE_HARDWARE_26_reg, 20, 20, 0x0);
				WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0x1);//D560
				WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0x1);//D560
	
				WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg, 8, 15, 0x0);//EF3C
				WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg, 24, 31, 0x0);//EF3C
	
				WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0x80);//CDFC
				WriteRegister(HARDWARE_HARDWARE_57_reg, 7, 7, 0x1);//D5E4
	
				WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg, 10, 13, 0x0);//EB74
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 0, 6, 0x0c);//EB78
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 8, 14, 0x05);//EB78
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 16, 22, 0x0c);//EB78
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 24, 30, 0x05);//EB78
	
				WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg, 4, 13, 0x2bc);//EBAC
				WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0x1);//EB14
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0x1);//EB18
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0x1);//EB18
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 16, 21, 0x14);//EB18
				WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0x1);//EBE4
				WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0x0);//EBE0
	
				WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg, 0, 9, 0x15);//EB20
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg, 0, 4, 0x4);//EB10
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg, 8, 12, 0x4);//EB10
				WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg, 3, 3, 0x1);//EBD0
				WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg, 4, 4, 0x1);//EBD0
				WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg, 16, 25, 0x14);//EB94 
			
				WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);
				WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
				WriteRegister(MC_MC_28_reg,14,14,0x1);
				WriteRegister(MC_MC_28_reg,15,22,0x0);
				WriteRegister(HARDWARE_HARDWARE_06_reg,0,1,0x0);		// dehalo
				WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,4,0);
				WriteRegister(KME_TOP_KME_TOP_04_reg,29,29,0);//meander disble
				//WriteRegister(HARDWARE_HARDWARE_06_reg, 0, 1, 0x0);
			}
			u5_blend_holdfrm_a[9] = 120;
			nIdentifiedNum_a[9]=9;
		}
		else if(u5_blend_holdfrm_a[9] > 0)
		{
			if(positive_rmv_counter < 4)
			{
			/*
				//WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
				WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);
				WriteRegister(MC_MC_28_reg,14,14,0x1);
				WriteRegister(MC_MC_28_reg,15,22,0x0);
				WriteRegister(HARDWARE_HARDWARE_06_reg,0,1,0x1);		// dehalo
				WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,4,0);
				
				WriteRegister(KME_TOP_KME_TOP_04_reg,29,29,1);//meander disble
				*/
				WriteRegister(HARDWARE_HARDWARE_26_reg, 20, 20, 0x1);
				WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0x0);//D560
				WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0x0);//D560
	
				WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg, 8, 15, 0x0);//EF3C
				WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg, 24, 31, 0x0);//EF3C
	
				WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0x0);//CDFC
				WriteRegister(HARDWARE_HARDWARE_57_reg, 7, 7, 0x0);//D5E4
	
				WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg, 10, 13, 0xf);//EB74
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 0, 6, 0x1f);//EB78
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 8, 14, 0x12);//EB78
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 16, 22, 0x8);//EB78
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 24, 30, 0x13);//EB78
	
				WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg, 4, 13, 0x12c);//EBAC
				WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0x0);//EB14
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0x0);//EB18
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0x0);//EB18
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 16, 21, 0xa);//EB18
				WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0x0);//EBE4
				WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0x0);//EBE0
	
				WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg, 0, 9, 0x1f);//EB20
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg, 0, 4, 0x8);//EB10
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg, 8, 12, 0x4);//EB10
				WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg, 3, 3, 0x0);//EBD0
				WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg, 4, 4, 0x0);//EBD0
				WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg, 16, 25, 0x1f);//EB94
	
				WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);
				WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x1);
				WriteRegister(MC_MC_28_reg,14,14,0x0);
				WriteRegister(MC_MC_28_reg,15,22,0x0);
				WriteRegister(HARDWARE_HARDWARE_06_reg,0,1,0x1);		// dehalo
				WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,4,1);
				WriteRegister(KME_TOP_KME_TOP_04_reg,29,29,1);
	
			}
			else {
				WriteRegister(HARDWARE_HARDWARE_26_reg, 20, 20, 0x0);
				WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0x1);//D560
				WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0x1);//D560
	
				WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg, 8, 15, 0x0);//EF3C
				WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg, 24, 31, 0x0);//EF3C
	
				WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0x80);//CDFC
				WriteRegister(HARDWARE_HARDWARE_57_reg, 7, 7, 0x1);//D5E4
	
				WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg, 10, 13, 0x0);//EB74
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 0, 6, 0x0c);//EB78
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 8, 14, 0x05);//EB78
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 16, 22, 0x0c);//EB78
				WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 24, 30, 0x05);//EB78
	
				WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg, 4, 13, 0x2bc);//EBAC
				WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0x1);//EB14
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0x1);//EB18
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0x1);//EB18
				WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 16, 21, 0x14);//EB18
				WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0x1);//EBE4
				WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0x0);//EBE0
	
				WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg, 0, 9, 0x15);//EB20
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg, 0, 4, 0x4);//EB10
				WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg, 8, 12, 0x4);//EB10
				WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg, 3, 3, 0x1);//EBD0
				WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg, 4, 4, 0x1);//EBD0
				WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg, 16, 25, 0x14);//EB94 
			
				WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);
				WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
				WriteRegister(MC_MC_28_reg,14,14,0x1);
				WriteRegister(MC_MC_28_reg,15,22,0x0);
				WriteRegister(HARDWARE_HARDWARE_06_reg,0,1,0x0);		// dehalo
				WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,4,0);
				WriteRegister(KME_TOP_KME_TOP_04_reg,29,29,0);//meander disble
				//WriteRegister(HARDWARE_HARDWARE_06_reg, 0, 1, 0x0);
			}
			u5_blend_holdfrm_a[9]--;
		}
		else if(nIdentifiedNum_a[9] == 9 )
		{
			WriteRegister(HARDWARE_HARDWARE_26_reg, 20, 20, 0x0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0x1);//D560
			WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0x1);//D560
	
			WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg, 8, 15, 0x0);//EF3C
			WriteRegister(KME_ME1_BG1_ME_BG_RANGE_2_reg, 24, 31, 0x0);//EF3C
	
			WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0x80);//CDFC
			WriteRegister(HARDWARE_HARDWARE_57_reg, 7, 7, 0x1);//D5E4
	
			WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg, 10, 13, 0x0);//EB74
			WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 0, 6, 0x0c);//EB78
			WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 8, 14, 0x05);//EB78
			WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 16, 22, 0x0c);//EB78
			WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg, 24, 30, 0x05);//EB78
	
			WriteRegister(KME_DEHALO5_PHMV_FIX_2A_reg, 4, 13, 0x2bc);//EBAC
			WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0x1);//EB14
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0x1);//EB18
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0x1);//EB18
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 16, 21, 0x14);//EB18
			WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0x1);//EBE4
			WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0x0);//EBE0
	
			WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg, 0, 9, 0x15);//EB20
			WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg, 0, 4, 0x4);//EB10
			WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg, 8, 12, 0x4);//EB10
			WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg, 3, 3, 0x1);//EBD0
			WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg, 4, 4, 0x1);//EBD0
			WriteRegister(KME_DEHALO5_PHMV_FIX_25_reg, 16, 25, 0x14);//EB94 
			
			WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x1);
			WriteRegister(MC_MC_28_reg,14,14,0x0);
			WriteRegister(MC_MC_28_reg,15,22,0x0);
			WriteRegister(HARDWARE_HARDWARE_06_reg,0,1,0x0);		// dehalo
			WriteRegister(KME_DEHALO5_PHMV_FIX_33_reg,3,4,3);
			WriteRegister(KME_TOP_KME_TOP_04_reg,29,29,1);
	
			//WriteRegister(HARDWARE_HARDWARE_06_reg, 0, 1, 0x0);
			nIdentifiedNum_a[9] = 0;
		}
	}
#endif
	
	//--#19 monosco4K_Hscroll_10pix_50Hz_long-50p-3840x2160
	{
		_PQLPARAMETER *s_pParam 	  = GetPQLParameter();
		if(u11_gmv_mvx < -8 && u11_gmv_mvx > -12 && u10_gmv_mvy == 0 && s_pContext->_output_frc_sceneAnalysis.u1_still_frame == 0 && s_pContext->_output_read_comreg.u25_me_aDTL_rb > 1200000 && s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb < 5 
			&& scalerVIP_Get_MEMCPatternFlag_Identification(3,10) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
		{
			if( monosco_flag == false ) {
				WriteRegister(MC_MC_D8_reg, 31, 31, 0x1);
				WriteRegister(MC_MC_D8_reg, 16, 26, 0x7F6);//2038
				WriteRegister(KME_LOGO0_KME_LOGO0_14_reg, 0, 31, 0x0);
				WriteRegister(KME_LOGO0_KME_LOGO0_18_reg, 0, 31, 0x0);
				WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg, 0, 31, 0x0);
				WriteRegister(KME_LOGO0_KME_LOGO0_20_reg, 0, 31, 0x0);
	
				s_pParam->_param_frc_sceneAnalysis.u1_mc2_var_lpf_wrt_en = 0;
				WriteRegister(HARDWARE_HARDWARE_57_reg,17,17, 0);
				WriteRegister(MC2_MC2_50_reg, 0, 0, 0);
				WriteRegister(MC2_MC2_50_reg, 1, 1, 0);
	
				monosco_flag = true;
			}
			u5_blend_holdfrm_a[10] = 8;
			nIdentifiedNum_a[10]=10;
		}
		else if(u5_blend_holdfrm_a[10] > 0)
		{
			u5_blend_holdfrm_a[10]--;
		}
		else if(nIdentifiedNum_a[10] == 10 )
		{
			WriteRegister(MC_MC_D8_reg, 31, 31, 0x0);
			WriteRegister(MC_MC_D8_reg, 16, 26, 0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_14_reg, 0, 31, 0xffffffff);
			WriteRegister(KME_LOGO0_KME_LOGO0_18_reg, 0, 31, 0xffffffff);
			WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg, 0, 31, 0xffffffff);
			WriteRegister(KME_LOGO0_KME_LOGO0_20_reg, 0, 31, 0xffffffff);
	
			s_pParam->_param_frc_sceneAnalysis.u1_mc2_var_lpf_wrt_en = 1;
			WriteRegister(HARDWARE_HARDWARE_57_reg,17,17, 1);
			WriteRegister(MC2_MC2_50_reg, 0, 0, 1);
			WriteRegister(MC2_MC2_50_reg, 1, 1, 1);
	
			monosco_flag = false;
			nIdentifiedNum_a[10] = 0;
		}
	}	
		//--#20-Montage_M 104s
		if(scalerVIP_Get_MEMCPatternFlag_Identification(3,11) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
			//&& u11_gmv_mvx>=27 && u11_gmv_mvx<=31 && u10_gmv_mvy>=-2 && u10_gmv_mvy<=1 
			//&& s_pContext->_output_frc_sceneAnalysis.u1_still_frame == 0 && s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb <= 2 )
		{
			WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
			WriteRegister(MC_MC_28_reg,14,14,0x1);
			WriteRegister(MC_MC_28_reg,15,22,0x0);
			WriteRegister(HARDWARE_HARDWARE_06_reg,0,1,0x0);		// dehalo
			u5_blend_holdfrm_a[11] = 8;
			nIdentifiedNum_a[11]=11;
		}
		else if(u5_blend_holdfrm_a[11] > 0)
		{
			WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
			WriteRegister(MC_MC_28_reg,14,14,0x1);
			WriteRegister(MC_MC_28_reg,15,22,0x0);
			WriteRegister(HARDWARE_HARDWARE_06_reg,0,1,0x0);		// dehalo
			u5_blend_holdfrm_a[11]--;
		}
		else if(nIdentifiedNum_a[11] == 11 )
		{
			WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x1);
			WriteRegister(MC_MC_28_reg,14,14,0x0);
			WriteRegister(MC_MC_28_reg,15,22,0x0);
			WriteRegister(HARDWARE_HARDWARE_06_reg,0,1,0x0);		// dehalo
			nIdentifiedNum_a[11] = 0;
		}
	
		//--#20-22_PAL_1_VTS_08_1 0s
		if(scalerVIP_Get_MEMCPatternFlag_Identification(3,12) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
		{
			WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x1);
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 255);	 
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 230);
			WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0);
	
			WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 0);
	
			u5_blend_holdfrm_a[12] = 8;
			nIdentifiedNum_a[12]=12;
		}
		else if(u5_blend_holdfrm_a[12] > 0)
		{
			WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x1);
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 255);	 
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 230);
			WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0);
	
			WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 0);
			u5_blend_holdfrm_a[12]--;
		}
		else if(nIdentifiedNum_a[12] == 12 )
		{
			WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x0);
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 0xff);	  
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 0x0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 1);
	
			WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 1);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 1);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 1);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 1);
			nIdentifiedNum_a[12] = 0;
		}
	
		//--#22_PAL_1_VTS_08_1 35s
		if(scalerVIP_Get_MEMCPatternFlag_Identification(3,13) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
		{
			WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0x0);
			u5_blend_holdfrm_a[13] = 8;
			nIdentifiedNum_a[13]=13;
		}
		else if(u5_blend_holdfrm_a[13] > 0)
		{
			WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0x0);
			u5_blend_holdfrm_a[13]--;
		}
		else if(nIdentifiedNum_a[13] == 13 )
		{
			WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0xffffffff);
			WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0xffffffff);
			WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0xffffffff);
			WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0xffffffff);
			nIdentifiedNum_a[13] = 0;
		}
	
		//--#04_Montage_P 5s //--#22_PAL_1_VTS_08_1 184s 102s 109s 187s
		if( ( scalerVIP_Get_MEMCPatternFlag_Identification(3,2) == TRUE || scalerVIP_Get_MEMCPatternFlag_Identification(3,15) == TRUE || scalerVIP_Get_MEMCPatternFlag_Identification(3,16) == TRUE 
			|| scalerVIP_Get_MEMCPatternFlag_Identification(3,19) == TRUE || scalerVIP_Get_MEMCPatternFlag_Identification(3,24) == TRUE) 
			&&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
		{
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
	
			WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);

			WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x1);
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 255);	 
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 230);
			WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0);
	
			WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 0);
	
			WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0x0);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x1);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x1);
	
			u5_blend_holdfrm_a[14] = 50;
			nIdentifiedNum_a[14]=14;
		}
		else if(u5_blend_holdfrm_a[14] > 0)
		{
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
	
			WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
			
			WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x1);
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 255);	 
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 230);
			WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0);
	
			WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 0);
	
			WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0x0);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x1);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x1);
	
			u5_blend_holdfrm_a[14]--;
		}
		else if(nIdentifiedNum_a[14] == 14 )
		{
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x40);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x30);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x10);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0);
	
			WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x40);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x30);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x10);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0);
			
			WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x0);
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 0xff);	  
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 0x0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 1);
	
			WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 1);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 1);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 1);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 1);
	 
			WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0xffffffff);
			WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0xffffffff);
			WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0xffffffff);
			WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0xffffffff);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x7);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x7);
	
			nIdentifiedNum_a[14] = 0;
		}
	
		//--#22_PAL_1_VTS_08_1 229s
		if(scalerVIP_Get_MEMCPatternFlag_Identification(3,17) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
		{
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
	
			WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);

			WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x1);
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 255);	 
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 230);
			WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0);
	
			WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 0);
	
			WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0x0);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x1);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x1);
			u5_blend_holdfrm_a[15] = 20;
			nIdentifiedNum_a[15]=15;
		}
		else if(u5_blend_holdfrm_a[15] > 0)
		{
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
	
			WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);

			WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x1);
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 255);	 
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 230);
			WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0);
	
			WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 0);
	
			WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0x0);
			WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0x0);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x1);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x1);
			u5_blend_holdfrm_a[15]--;
		}
		else if(nIdentifiedNum_a[15] == 15 )
		{
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x40);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x30);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x10);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0);
	
			WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x40);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x30);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x10);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0);
			
			WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x0);
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 0xff);	  
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 0x0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 1);
	
			WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 1);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 1);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 1);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 1);
	 
			WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0xffffffff);
			WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0xffffffff);
			WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0xffffffff);
			WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0xffffffff);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x7);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x7);
			nIdentifiedNum_a[15] = 0;
		}
	
		//--#22_PAL_1_VTS_08_1 10s
		if(scalerVIP_Get_MEMCPatternFlag_Identification(3,18) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
		{
			WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x1);
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 255);	 
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 230);
			WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0);
	
			WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 0);
			u5_blend_holdfrm_a[16] = 20;
			nIdentifiedNum_a[16]=16;
		}
		else if(u5_blend_holdfrm_a[16] > 0)
		{
			WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x1);
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 255);	 
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 230);
			WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 0);
	
			WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 0);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 0);
			u5_blend_holdfrm_a[16]--;
		}
		else if(nIdentifiedNum_a[16] == 16 )
		{
			WriteRegister(KME_ME1_BG1_ME_BG_S4_4_reg, 24, 24, 0x0);
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 0, 7, 0xff);	  
			WriteRegister(KME_ME1_BG1_ME_BG_S4_3_reg, 8, 15, 0x0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_5_reg, 15, 15, 1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 7, 7, 1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_6_reg, 6, 6, 1);
			WriteRegister(KME_DEHALO5_PHMV_FIX_37_reg, 16, 16, 0);
			WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg, 0, 0, 1);
	
			WriteRegister(HARDWARE_HARDWARE_24_reg, 28, 28, 1);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 31, 31, 1);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 30, 30, 1);
			WriteRegister(HARDWARE_HARDWARE_24_reg, 18, 18, 1);
			nIdentifiedNum_a[16] = 0;
		}

		//--#10_03_boundary_BBC1 3s
		if(scalerVIP_Get_MEMCPatternFlag_Identification(3,20) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
		{
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,16,16,1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,15,15,1);
			WriteRegister(MC_MC_B4_reg,24,31,255);
			WriteRegister(MC_MC_B4_reg,16,23,255);
			u5_blend_holdfrm_a[17] = 20;
			nIdentifiedNum_a[17]=17;
		}
		else if(u5_blend_holdfrm_a[17] > 0)
		{
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,16,16,1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,15,15,1);
			WriteRegister(MC_MC_B4_reg,24,31,255);
			WriteRegister(MC_MC_B4_reg,16,23,255);
			u5_blend_holdfrm_a[17]--;
		}
		else if(nIdentifiedNum_a[17] == 17 )
		{
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,16,16,0);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,15,15,0);
			WriteRegister(MC_MC_B4_reg,24,31,192);
			WriteRegister(MC_MC_B4_reg,16,23,104);
			nIdentifiedNum_a[17] = 0;
		}
	
		//--#8.sydney5_motion24p.m2ts 11s
		if(scalerVIP_Get_MEMCPatternFlag_Identification(3,21) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
		{
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
			u5_blend_holdfrm_a[18] = 40;
			nIdentifiedNum_a[18]=18;
		}
		else if(u5_blend_holdfrm_a[18] > 0)
		{
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
			u5_blend_holdfrm_a[18]--;
		}
		else if(nIdentifiedNum_a[18] == 18 )
		{
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x1);
			nIdentifiedNum_a[18] = 0;
		}
	
		//--Hivi_640pixel_00057.m2ts & HIvi_960pixel_00058.m2ts
		if(scalerVIP_Get_MEMCPatternFlag_Identification(3,22) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
		{
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
	
			WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x3);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x3);
			u5_blend_holdfrm_a[19] = 8;
			nIdentifiedNum_a[19]=19;
		}
		else if(u5_blend_holdfrm_a[19] > 0)
		{
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
	
			WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x3);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x3);
			u5_blend_holdfrm_a[19]--;
		}
		else if(nIdentifiedNum_a[19] == 19 )
		{
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x40);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x30);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x10);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0);
	
			WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x40);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x30);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x10);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x7);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x7);
			nIdentifiedNum_a[19] = 0;
		}
	
		//--DMN Cityscapes 720p60.ts
		if(scalerVIP_Get_MEMCPatternFlag_Identification(3,23) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
		{
			WriteRegister(MC_MC_28_reg,14,14, 1);
			WriteRegister(MC_MC_28_reg,15,22, 0);
			WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg, 8,17, 32);
			WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg,18,27, 32);
			WriteRegister(KME_ME1_TOP1_ME1_COMMON1_12_reg,16,31, 65532);
			WriteRegister(KME_TOP_KME_TOP_04_reg,29,29, 0);
			WriteRegister(IPPRE_IPPRE_04_reg,16,23, 150);
			u5_blend_holdfrm_a[20] = 40;
			nIdentifiedNum_a[20]=20;
		}
		else if(u5_blend_holdfrm_a[20] > 0)
		{
			WriteRegister(MC_MC_28_reg,14,14, 1);
			WriteRegister(MC_MC_28_reg,15,22, 0);
			WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg, 8,17, 32);
			WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg,18,27, 32);
			WriteRegister(KME_ME1_TOP1_ME1_COMMON1_12_reg,16,31, 65532);
			WriteRegister(KME_TOP_KME_TOP_04_reg,29,29, 0);
			WriteRegister(IPPRE_IPPRE_04_reg,16,23, 150);
			u5_blend_holdfrm_a[20]--;
		}
		else if(nIdentifiedNum_a[20] == 20 )
		{
			WriteRegister(MC_MC_28_reg,14,14, 0);
			WriteRegister(MC_MC_28_reg,15,22, 0);
			WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg, 8,17, 0);
			WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg,18,27, 0);
			WriteRegister(KME_ME1_TOP1_ME1_COMMON1_12_reg,16,31, 5);
			WriteRegister(KME_TOP_KME_TOP_04_reg,29,29, 1);
			WriteRegister(IPPRE_IPPRE_04_reg,16,23, 0x20);
			nIdentifiedNum_a[20] = 0;
		}
	
		//--#8.sydney5_motion24p.m2ts 9s
		if(scalerVIP_Get_MEMCPatternFlag_Identification(3,25) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
		{
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
			u5_blend_holdfrm_a[21] = 40;
			nIdentifiedNum_a[21]=21;
		}
		else if(u5_blend_holdfrm_a[21] > 0)
		{
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
			u5_blend_holdfrm_a[21]--;
		}
		else if(nIdentifiedNum_a[21] == 21 )
		{
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x1);
			nIdentifiedNum_a[21] = 0;
		}
	
		//--#23_DK_Zoomin_crop_i 0s
		if(scalerVIP_Get_MEMCPatternFlag_Identification(3,27) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
		{
			if( (u11_gmv_mvx >= 2045 || ( u11_gmv_mvx <= 5 && u11_gmv_mvx >= 0 ) ) && u10_gmv_mvy >=0 && u10_gmv_mvy <= 3 ) {
				WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
				WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
				WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
				WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
				WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
				WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
	
				WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
				WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
				WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
				WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
				WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
				WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
				WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
				WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
	
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x1);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x1);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x1);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x1);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x1);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x1);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x1);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x1);
	
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x1);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x1);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x1);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x1);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x1);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x1);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x1);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x1);
				
				u5_blend_holdfrm_a[22] = 40;
				nIdentifiedNum_a[22]=22;
			}
		}
		else if(u5_blend_holdfrm_a[22] > 0)
		{
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
	
			WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x1);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x1);
			u5_blend_holdfrm_a[22]--;
		}
		else if(nIdentifiedNum_a[22] == 22 )
		{
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x40);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x30);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x10);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0);
	
			WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x40);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x30);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x10);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x7);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x7);
			nIdentifiedNum_a[22] = 0;
		}
	
		//--#16_DMN Cityscapes 720p60 32s
		if(scalerVIP_Get_MEMCPatternFlag_Identification(3,28) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
		{
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
	
			WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
				
			u5_blend_holdfrm_a[23] = 40;
			nIdentifiedNum_a[23]=23;
		}
		else if(u5_blend_holdfrm_a[23] > 0)
		{
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
	
			WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
			
			u5_blend_holdfrm_a[23]--;
		}
		else if(nIdentifiedNum_a[23] == 23 )
		{
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x40);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x30);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x10);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0);
	
			WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x40);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x30);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x10);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0);
	
			nIdentifiedNum_a[23] = 0;
		}
	
		//--#13_05_periodical_MI3_stair 0s
		if(scalerVIP_Get_MEMCPatternFlag_Identification(3,29) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
		{
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
	
			WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x1);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x1);
			u5_blend_holdfrm_a[24] = 8;
			nIdentifiedNum_a[24]=24;
		}
		else if(u5_blend_holdfrm_a[24] > 0)
		{
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
	
			WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x1);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x1);
			u5_blend_holdfrm_a[24]--;
		}
		else if(nIdentifiedNum_a[24] == 24 )
		{
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x40);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x30);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x10);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0);
	
			WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x40);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x30);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x10);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x7);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x7);
			nIdentifiedNum_a[24] = 0;
		}
	
		//--#23_DK_Zoomin_crop_i 0s
		if(scalerVIP_Get_MEMCPatternFlag_Identification(3,30) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
		{
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
	
			WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x1);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x1);
			u5_blend_holdfrm_a[25] = 50;
			nIdentifiedNum_a[25]=25;
		}
		else if(u5_blend_holdfrm_a[25] > 0)
		{
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
	
			WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x1);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x1);
			u5_blend_holdfrm_a[25]--;
		}
		else if(nIdentifiedNum_a[25] == 25 )
		{
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x40);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x30);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x10);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0);
	
			WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x40);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x30);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x10);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x7);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x7);
			nIdentifiedNum_a[25] = 0;
		}
	
		//--#4_3.QTEK_6.5ppf_00081 0s
		if(scalerVIP_Get_MEMCPatternFlag_Identification(3,31) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
		{
			if( u11_gmv_mvx >= 23 && u11_gmv_mvx <= 25 ) {
				WriteRegister(MC_MC_D8_reg, 0, 9,0x0);
				WriteRegister(MC_MC_D8_reg,16,26,0x24);
				WriteRegister(MC_MC_D8_reg,31,31,0x1);
	
				WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
				WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
				WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
				WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
				WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
				WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
	
				WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
				WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
				WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
				WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
				WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
				WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
				WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
				WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
	
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x3);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x3);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x3);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x3);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x3);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x3);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x3);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x3);
	
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x3);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x3);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x3);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x3);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x3);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x3);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x3);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x3);
			}
			u5_blend_holdfrm_a[26] = 8;
			nIdentifiedNum_a[26]=26;
		}
		else if(u5_blend_holdfrm_a[26] > 0)
		{
			if( u11_gmv_mvx >= 23 && u11_gmv_mvx <= 25 ) {
				WriteRegister(MC_MC_D8_reg, 0, 9,0x0);
				WriteRegister(MC_MC_D8_reg,16,26,0x24);
				WriteRegister(MC_MC_D8_reg,31,31,0x1);
	
				WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
				WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
				WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
				WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
				WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
				WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
	
				WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
				WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
				WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
				WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
				WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
				WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
				WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
				WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
	
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x3);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x3);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x3);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x3);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x3);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x3);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x3);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x3);
	
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x3);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x3);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x3);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x3);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x3);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x3);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x3);
				WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x3);
			}
			u5_blend_holdfrm_a[26]--;
		}
		else if(nIdentifiedNum_a[26] == 26 )
		{
			WriteRegister(MC_MC_D8_reg, 0, 9,0x0);
			WriteRegister(MC_MC_D8_reg,16,26,0x0);
			WriteRegister(MC_MC_D8_reg,31,31,0x0);	
	
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x40);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x30);
			WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x10);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0);
			WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0);
	
			WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x40);
			WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x30);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x10);
			WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0);
			WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x7);
	
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x7);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x7);
			nIdentifiedNum_a[26] = 0;
		}

	//--#7_8.sydney5_motion24p 22s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(3,33) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		if( u11_gmv_mvx >= -5 && u11_gmv_mvx <= -1 && u10_gmv_mvy >= 5 && u10_gmv_mvy <= 9 &&
			s_pContext->_output_frc_sceneAnalysis.u1_still_frame == 0 &&
			s_pContext->_output_read_comreg.u25_me_aDTL_rb >= 1330000 && s_pContext->_output_read_comreg.u25_me_aDTL_rb <= 1519000 &&
			s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb >= 3 && s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb <= 9
		) {
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
			u5_blend_holdfrm_a[28] = 80;
			nIdentifiedNum_a[28]=28;
		}
		else if( u11_gmv_mvx >= 6 && u11_gmv_mvx <= 10 && u10_gmv_mvy >= 3 && u10_gmv_mvy <= 9 &&
			s_pContext->_output_frc_sceneAnalysis.u1_still_frame == 0 &&
			s_pContext->_output_read_comreg.u25_me_aDTL_rb >= 1080000 && s_pContext->_output_read_comreg.u25_me_aDTL_rb <= 1230500 &&
			s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb >= 0 && s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb <= 5
		) {
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
			u5_blend_holdfrm_a[28] = 80;
			nIdentifiedNum_a[28]=28;
		}
		else if( u11_gmv_mvx >= 11 && u11_gmv_mvx <= 21 && u10_gmv_mvy >= 0 && u10_gmv_mvy <= 4 &&
			s_pContext->_output_frc_sceneAnalysis.u1_still_frame == 0 &&
			s_pContext->_output_read_comreg.u25_me_aDTL_rb >= 924000 && s_pContext->_output_read_comreg.u25_me_aDTL_rb <= 1029000 &&
			s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb >= 1 && s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb <= 2
		) {
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
			u5_blend_holdfrm_a[28] = 80;
			nIdentifiedNum_a[28]=28;
		}
		else if( u11_gmv_mvx >= 4 && u11_gmv_mvx <= 5 && u10_gmv_mvy >= 0 && u10_gmv_mvy <= 1 &&
			s_pContext->_output_frc_sceneAnalysis.u1_still_frame == 0 &&
			s_pContext->_output_read_comreg.u25_me_aDTL_rb >= 892500 && s_pContext->_output_read_comreg.u25_me_aDTL_rb <= 935500 &&
			s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb >= 2 && s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb <= 3
		) {
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
			u5_blend_holdfrm_a[28] = 80;
			nIdentifiedNum_a[28]=28;
		}
	}
	else if(u5_blend_holdfrm_a[28] > 0)
	{
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		u5_blend_holdfrm_a[28]--;
	}
	else if(nIdentifiedNum_a[28] == 28 )
	{
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x1);
		nIdentifiedNum_a[28] = 0;
	}
	
	//--#7_8.sydney5_motion24p 29s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(3,34) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		if( u11_gmv_mvx >= 11 && u11_gmv_mvx <= 13 && u10_gmv_mvy >= 2 && u10_gmv_mvy <= 4 &&
			s_pContext->_output_frc_sceneAnalysis.u1_still_frame == 0 &&
			s_pContext->_output_read_comreg.u25_me_aDTL_rb >= 993000 && s_pContext->_output_read_comreg.u25_me_aDTL_rb <= 1011500 &&
			s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb >= 1 && s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb <= 3
		) {
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		u5_blend_holdfrm_a[29] = 8;
			nIdentifiedNum_a[29]=29;
		}
	}
	else if(u5_blend_holdfrm_a[29] > 0)
	{
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		u5_blend_holdfrm_a[29]--;
	}
	else if(nIdentifiedNum_a[29] == 29 )
	{
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x1);
		nIdentifiedNum_a[29] = 0;
	}

	//--#17_DNM hrnm 720p60 78s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(3,35) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		if( u11_gmv_mvx >= -17 && u11_gmv_mvx <= -15 && u10_gmv_mvy >= 2 && u10_gmv_mvy <= 4 &&
			s_pContext->_output_frc_sceneAnalysis.u1_still_frame == 0 &&
			s_pContext->_output_read_comreg.u25_me_aDTL_rb >= 494200 && s_pContext->_output_read_comreg.u25_me_aDTL_rb <= 494500 &&
			s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb >= 11 && s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb <= 13
		) {
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
	
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
	
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x1);
	
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x1);
		}
		u5_blend_holdfrm_a[30] = 200;
		nIdentifiedNum_a[30]=30;
	}
	else if(u5_blend_holdfrm_a[30] > 0)
	{
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
	
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);
	
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x1);
	
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x1);
		u5_blend_holdfrm_a[30]--;
	}
	else if(nIdentifiedNum_a[30] == 30 )
	{
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x1);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x40);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x30);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x10);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0);
	
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x40);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x30);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x10);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0);
	
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x7);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x7);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x7);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x7);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x7);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x7);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x7);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x7);
	
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x7);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x7);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x7);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x7);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x7);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x7);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x7);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x7);
		nIdentifiedNum_a[30] = 0;
	}

	//--#7_8.sydney5_motion24p 25s
	if(scalerVIP_Get_MEMCPatternFlag_Identification(3,36) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		if( u11_gmv_mvx >= 6 && u11_gmv_mvx <= 10 && u10_gmv_mvy >= 3 && u10_gmv_mvy <= 9 &&
			s_pContext->_output_frc_sceneAnalysis.u1_still_frame == 0 &&
			s_pContext->_output_read_comreg.u25_me_aDTL_rb >= 1080000 && s_pContext->_output_read_comreg.u25_me_aDTL_rb <= 1230500 &&
			s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb >= 0 && s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb <= 5
		) {
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
			u5_blend_holdfrm_a[31] = 160;
			nIdentifiedNum_a[31]=31;
		}
		else if( u11_gmv_mvx >= 11 && u11_gmv_mvx <= 21 && u10_gmv_mvy >= 0 && u10_gmv_mvy <= 4 &&
			s_pContext->_output_frc_sceneAnalysis.u1_still_frame == 0 &&
			s_pContext->_output_read_comreg.u25_me_aDTL_rb >= 924000 && s_pContext->_output_read_comreg.u25_me_aDTL_rb <= 1029000 &&
			s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb >= 1 && s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb <= 2
		) {
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
			u5_blend_holdfrm_a[31] = 160;
			nIdentifiedNum_a[31]=31;
		}
		else if( u11_gmv_mvx >= 4 && u11_gmv_mvx <= 5 && u10_gmv_mvy >= 0 && u10_gmv_mvy <= 1 &&
			s_pContext->_output_frc_sceneAnalysis.u1_still_frame == 0 &&
			s_pContext->_output_read_comreg.u25_me_aDTL_rb >= 892500 && s_pContext->_output_read_comreg.u25_me_aDTL_rb <= 935500 &&
			s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb >= 2 && s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb <= 3
		) {
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
			u5_blend_holdfrm_a[31] = 160;
			nIdentifiedNum_a[31]=31;
		}
	}
	else if(u5_blend_holdfrm_a[31] > 0)
	{
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		u5_blend_holdfrm_a[31]--;
	}
	else if(nIdentifiedNum_a[31] == 31 )
	{
		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x1);
		nIdentifiedNum_a[31] = 0;
	}

	//--#04_Montage_P 85s 90s 95s
	if( (scalerVIP_Get_MEMCPatternFlag_Identification(3,37) == TRUE || scalerVIP_Get_MEMCPatternFlag_Identification(3,38) == TRUE 
		|| scalerVIP_Get_MEMCPatternFlag_Identification(3,39) == TRUE )&&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);		// local FB
		WriteRegister(MC_MC_28_reg,14,14,0x1);	// local FB
		WriteRegister(MC_MC_28_reg,15,22,0x0);	// local FB

		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		u5_blend_holdfrm_a[32] = 80;
		nIdentifiedNum_a[32]=32;
	}
	else if(u5_blend_holdfrm_a[32] > 0)
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x0);		// local FB
		WriteRegister(MC_MC_28_reg,14,14,0x1);	// local FB
		WriteRegister(MC_MC_28_reg,15,22,0x0);	// local FB

		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x0);
		u5_blend_holdfrm_a[32]--;
	}
	else if(nIdentifiedNum_a[32] == 32 )
	{
		WriteRegister(KMC_BI_bi_top_01_reg,0,0,0x1);		// local FB
		WriteRegister(MC_MC_28_reg,14,14,0x0);	// local FB
		WriteRegister(MC_MC_28_reg,15,22,0x0);	// local FB

		WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,0,0,0x1);
		nIdentifiedNum_a[32] = 0;
	}

	//--#20-Montage_M 116s 120s
	if( (scalerVIP_Get_MEMCPatternFlag_Identification(3,40) == TRUE || scalerVIP_Get_MEMCPatternFlag_Identification(3,41) == TRUE )
		&&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KME_LOGO0_KME_LOGO0_64_reg, 0, 7, 60);
		WriteRegister(KME_LOGO0_KME_LOGO0_64_reg, 8,15,120);
		WriteRegister(KME_LOGO0_KME_LOGO0_64_reg,16,23, 80);
		WriteRegister(KME_LOGO0_KME_LOGO0_64_reg,24,31,220);
		u5_blend_holdfrm_a[33] = 80;
		nIdentifiedNum_a[33]=33;
	}
	else if(u5_blend_holdfrm_a[33] > 0)
	{
		WriteRegister(KME_LOGO0_KME_LOGO0_64_reg, 0, 7, 60);
		WriteRegister(KME_LOGO0_KME_LOGO0_64_reg, 8,15,120);
		WriteRegister(KME_LOGO0_KME_LOGO0_64_reg,16,23, 80);
		WriteRegister(KME_LOGO0_KME_LOGO0_64_reg,24,31,220);
		u5_blend_holdfrm_a[33]--;
	}
	else if(nIdentifiedNum_a[33] == 33 )
	{
		WriteRegister(KME_LOGO0_KME_LOGO0_64_reg, 0, 7, 3);
		WriteRegister(KME_LOGO0_KME_LOGO0_64_reg, 8,15, 6);
		WriteRegister(KME_LOGO0_KME_LOGO0_64_reg,16,23, 4);
		WriteRegister(KME_LOGO0_KME_LOGO0_64_reg,24,31,11);
		nIdentifiedNum_a[33] = 0;
	}

	//--#04_Montage_P 100s
	if( scalerVIP_Get_MEMCPatternFlag_Identification(3,42) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
		
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);

		WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0);
		WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0);
		WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0);
		WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0);
		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x1);
		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x1);

		u5_blend_holdfrm_a[34] = 80;
		nIdentifiedNum_a[34]=34;
	}
	else if(u5_blend_holdfrm_a[34] > 0)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 19);
		
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 20);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 19);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 19);

		WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0);
		WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0);
		WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0);
		WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0);
		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x1);
		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x1);

		u5_blend_holdfrm_a[34]--;
	}
	else if(nIdentifiedNum_a[34] == 34 )
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x40);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x30);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x10);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0);
	
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x40);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x30);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x10);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0);

		WriteRegister(KME_LOGO0_KME_LOGO0_14_reg,0,31,0xffffffff);
		WriteRegister(KME_LOGO0_KME_LOGO0_18_reg,0,31,0xffffffff);
		WriteRegister(KME_LOGO0_KME_LOGO0_1C_reg,0,31,0xffffffff);
		WriteRegister(KME_LOGO0_KME_LOGO0_20_reg,0,31,0xffffffff);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,0x7);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,0x7);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11,0x7);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x7);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x7);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x7);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x7);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x7);
	
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,0x7);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,0x7);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11,0x7);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x7);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x7);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x7);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x7);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x7);

		nIdentifiedNum_a[34] = 0;
	}
	//--#04_Montage_P 100s

	//===debug show
	for(u8_Index =0;u8_Index<255;u8_Index++)
	{
		if(nIdentifiedNum_a[u8_Index] !=0){
			pOutput->u1_IP_wrtAction_true=1;
			if(u32_ID_Log_en==1)
			rtd_pr_memc_emerg("[IPR_011]=====nIdentifiedNum_a>>[%d][%d]   ,\n", u8_Index, nIdentifiedNum_a[u8_Index]);
		}
	}
	//===
}

VOID Identification_Pattern_wrtAction_TV030(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
#if 0
			//===debug show
			for(u8_Index =0;u8_Index<255;u8_Index++)
			{
				if(nIdentifiedNum_a[u8_Index] !=0){
					pOutput->u1_IP_wrtAction_true=1;
					if(u32_ID_Log_en==1)
						rtd_pr_memc_emerg("[IPR_030]nIdentifiedNum_a>>[%d][%d]	 ,\n", u8_Index, nIdentifiedNum_a[u8_Index]);
				}
			}
#endif

}

VOID Identification_Pattern_wrtAction_TV043(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	//_PQLPARAMETER *PatternParam = GetPQLParameter();
	static unsigned char u5_blend_holdfrm_a[255] = {0};
	static unsigned char nIdentifiedNum_a[255] = {0};
	//unsigned int mot_diff = s_pContext->_output_filmDetectctrl.u27_ipme_motionPool[_FILM_ALL][0];
	//static unsigned char nOnceFlag=12;
	unsigned char u8_Index;
	//static bool monosco_flag = 0;
#if Pro_TCL
	//unsigned char  u8_cadence_Id = s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL];
	//PQL_INPUT_FRAME_RATE in_fmRate = s_pContext->_external_data._input_frameRate;
	unsigned char nblend_y_alpha_Target=0x90;
	unsigned char nblend_uv_alpha_Target=0x70;
	unsigned char nblend_logo_y_alpha_Target=0xd0;
	unsigned char nblend_logo_uv_alpha_Target=0x30;
#endif

	//unsigned int u32_RB_Value;
	signed short u11_gmv_mvx = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
	signed short u10_gmv_mvy = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;
	//unsigned int u25_me_aDTL = s_pContext->_output_read_comreg.u25_me_aDTL_rb;
	//unsigned int u26_aAPLi_rb = s_pContext->_output_read_comreg.u26_me_aAPLi_rb;
	//unsigned int u32_pattern298_zidoo_player_forbindden_en;
	
	unsigned char u1_still_frame=s_pContext->_output_frc_sceneAnalysis.u1_still_frame;
	//unsigned char  u8_cadence_Id = s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL];
	//unsigned char  u8_Mixmode_flag = s_pContext->_output_filmDetectctrl.u1_mixMode;
	//int gmv_mvx =0;//, gmv_mvy =0 ;
	//PQL_INPUT_FRAME_RATE in_fmRate = s_pContext->_external_data._input_frameRate;


#if 1 //for YE Test temp test
	unsigned int u32_ID_Log_en=0;//,ES_Test_Value=0;
	ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 24,  24, &u32_ID_Log_en);//YE Test debug dummy register
#endif

	if((pParam->u1_Identification_Pattern_en== 0)||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_Test_0_flag ==0))
		return;

	//reset
	pOutput->u1_IP_wrtAction_true=0;

	/*7.12.02.movZonePlate_AVC1080P60.mp4*/
	/*01.E_CMO-23_98psf.mp4*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,3) == TRUE || scalerVIP_Get_MEMCPatternFlag_Identification(5,4) == TRUE) //histogram from h5x
	{
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 0);
		//WriteRegister(KIWI_REG(FRC_TOP__PQL_1__pql_fb_ctrl_en), 0);//Merlin7 do not have it 
		WriteRegister(MC_MC_B0_reg, 20, 27, 0);
		u5_blend_holdfrm_a[1] = 8;
		nIdentifiedNum_a[1]=1;
	
	}
	else if(u5_blend_holdfrm_a[1] >0)
	{
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 0);
		//WriteRegister(KIWI_REG(FRC_TOP__PQL_1__pql_fb_ctrl_en), 0);//Merlin7 do not have it 
		WriteRegister(MC_MC_B0_reg, 20, 27, 0);
		u5_blend_holdfrm_a[1]--;
	}
	else if(nIdentifiedNum_a[1] == 1 )
	{
		WriteRegister(MC_MC_28_reg, 14, 14, 0);
		WriteRegister(MC_MC_B0_reg, 20, 27, 1);
		nIdentifiedNum_a[1] = 0;
	}

	/*7.9.09.IDT_HD_H_R_MPG1080P24.mp4*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,8) == TRUE) //histogram from h5x
	{
		rtd_clearbits(0xb802b500, _BIT0);//  sharpness peaking
		rtd_clearbits(0xb802b950, _BIT0);//  sharpness dlti
		rtd_clearbits(0xb8023c00, _BIT0);//  Multiband peaking
		rtd_setbits(0xb8023c30, _BIT0);//  Multiband peaking DB apply
		WriteRegister(KME_DEHALO_KME_DEHALO_70_reg, 4, 4, 1);
		WriteRegister(KME_DEHALO_KME_DEHALO_B0_reg, 12, 19, 0);
		WriteRegister(KME_DEHALO_KME_DEHALO_D4_reg, 6, 11,15); 
		WriteRegister(MC2_MC2_50_reg,0,1, 0);
		//WriteRegister(FRC_TOP__PQL_0__pql_patch_dummy_e4_ADDR,29,29, 0);//logo_ctrl_wrt_en : disable//Merlin7 does not have it
		WriteRegister(MC2_MC2_20_reg, 0, 1, 0);		
		//WriteRegister(FRC_TOP__PQL_0__pql_patch_dummy_e4_ADDR,5,5, 0);//u1_RepeatPattern_en: disable//Merlin7 does not have it
		u5_blend_holdfrm_a[2] = 8;
		nIdentifiedNum_a[2]=2;
	
	}
	else if(u5_blend_holdfrm_a[2] >0)
	{
		rtd_clearbits(0xb802b500, _BIT0);//  sharpness peaking
		rtd_clearbits(0xb802b950, _BIT0);//  sharpness dlti
		rtd_clearbits(0xb8023c00, _BIT0);//  Multiband peaking
		rtd_setbits(0xb8023c30, _BIT0);//  Multiband peaking DB apply
		WriteRegister(KME_DEHALO_KME_DEHALO_70_reg, 4, 4,1);
		WriteRegister(KME_DEHALO_KME_DEHALO_B0_reg, 12, 19,0);
		WriteRegister(KME_DEHALO_KME_DEHALO_D4_reg, 6, 11,15); 
		WriteRegister(MC2_MC2_50_reg,0,1, 0);
		//WriteRegister(FRC_TOP__PQL_0__pql_patch_dummy_e4_ADDR,29,29, 0);//logo_ctrl_wrt_en : disable//Merlin7 does not have it
		WriteRegister(MC2_MC2_20_reg, 0, 1, 0);
		//WriteRegister(FRC_TOP__PQL_0__pql_patch_dummy_e4_ADDR,5,5, 0);//u1_RepeatPattern_en: disable//Merlin7 does not have it
		u5_blend_holdfrm_a[2]--;
	}
	else if(nIdentifiedNum_a[2] == 2 )
	{
		rtd_setbits(0xb802b500, _BIT0);//  sharpness peaking
		rtd_setbits(0xb802b950, _BIT0);//  sharpness dlti
		rtd_setbits(0xb8023c00, _BIT0);//  Multiband peaking
		rtd_setbits(0xb8023c30, _BIT0);//  Multiband peaking DB apply
		WriteRegister(KME_DEHALO_KME_DEHALO_70_reg, 4, 4,0);
		WriteRegister(KME_DEHALO_KME_DEHALO_B0_reg, 12, 19,4);
		WriteRegister(KME_DEHALO_KME_DEHALO_D4_reg, 6, 11,0);	
		WriteRegister(MC2_MC2_50_reg,0,1, 3);
		//WriteRegister(FRC_TOP__PQL_0__pql_patch_dummy_e4_ADDR,29,29, 1);//logo_ctrl_wrt_en : enable//Merlin7 does not have it
		WriteRegister(MC2_MC2_20_reg, 0, 1, 3);
		//WriteRegister(FRC_TOP__PQL_0__pql_patch_dummy_e4_ADDR,5,5, 1);//u1_RepeatPattern_en: enable//Merlin7 does not have it
		nIdentifiedNum_a[2] = 0;
	}


	/*Passion Cocoa Milk Buns*/// standing at the middle
	if((_ABS_(u11_gmv_mvx>>1) <=1	&& _ABS_(u10_gmv_mvy>>1) <=1) && 
		scalerVIP_Get_MEMCPatternFlag_Identification(5,14) == TRUE) //histogram from h5x
	{
		WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 23, 23, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 6, 6, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 7, 7, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 8, 8, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 12, 12, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 13, 13, 0);
		
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_40_reg, 13, 16, 1);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_40_reg,  9, 12, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8, 11,  1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 21, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8, 11,  1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 21, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 1);
		u5_blend_holdfrm_a[3] = 60;
		nIdentifiedNum_a[3]=3;
	}
	else if(u5_blend_holdfrm_a[3] >0)
	{
		WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 23, 23, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 6, 6, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 7, 7, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 8, 8, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 12, 12, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 13, 13, 0);

		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_40_reg, 13, 16, 1);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_40_reg,  9, 12, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8, 11,  1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 21, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8, 11,  1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 21, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 1);
		u5_blend_holdfrm_a[3]--;
	}
	else if(nIdentifiedNum_a[3] == 3 )
	{
		WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 23, 23, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 6, 6, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 7, 7, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 8, 8, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 12, 12, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 13, 13, 1);

		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_40_reg, 13, 16, 3);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_40_reg,  9, 12, 3);
		nIdentifiedNum_a[3] = 0;
	}
	/*Passion Cocoa Milk Buns*/ // kneading the dough
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,18) == TRUE) //histogram from h5x
	{
		WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 23, 23, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 6, 6, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 7, 7, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 8, 8, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 12, 12, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 13, 13, 0);
		
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_40_reg, 13, 16, 1);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_40_reg,  9, 12, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8, 11,  1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 21, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8, 11,  1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 21, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 1);
		u5_blend_holdfrm_a[4] = 60;
		nIdentifiedNum_a[4]=4;
	}
	else if(u5_blend_holdfrm_a[4] >0)
	{
		WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 23, 23, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 6, 6, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 7, 7, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 8, 8, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 12, 12, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 13, 13, 0);
	
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_40_reg, 13, 16, 1);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_40_reg,  9, 12, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8, 11,  1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 21, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8, 11,  1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 21, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 1);
		u5_blend_holdfrm_a[4]--;
	}
	else if(nIdentifiedNum_a[4] == 4 )
	{
		WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 23, 23, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 6, 6, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 7, 7, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 8, 8, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 12, 12, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 13, 13, 1);
	
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_40_reg, 13, 16, 3);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_40_reg,  9, 12, 3);
		nIdentifiedNum_a[4] = 0;
	}
	
	/*THE MATRIX RELOADED.4khdr_5_short.mp4*/
	if(s_pContext->_output_read_comreg.u25_me_aDTL_rb < 350000 && 
		scalerVIP_Get_MEMCPatternFlag_Identification(5,19) == TRUE) //histogram from h5x
	{		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,   15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,   15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8, 11,  15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 21, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,   15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,   15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8, 11,  15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 21, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 15);
		u5_blend_holdfrm_a[5] = 8;
		nIdentifiedNum_a[5]=5;
	}
	else if(u5_blend_holdfrm_a[5] >0)
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,   15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,   15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8, 11,  15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 21, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,   15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,   15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8, 11,  15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 21, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 15);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 15);
		u5_blend_holdfrm_a[5]--;
	}
	else if(nIdentifiedNum_a[5] == 5 )
	{
		nIdentifiedNum_a[5] = 0;
	}
	

	//Montage_M_ROT-720p50.mpg
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,39) == TRUE) //histogram from h5x
	{
		//WriteRegister(FRC_TOP__PQL_0__pql_patch_dummy_e4_ADDR,7,7, 0);//Merlin7 does not have it
		WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x00);
		WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0,7, 0x00);
		WriteRegister(KMC_BI_bi_top_01_reg,2,2, 0);

		//WriteRegister(FRC_TOP__PQL_1__pql_wr_dhclose_wrt_en_ADDR,4,4, 0);//Merlin7 do not have it 
		WriteRegister(KME_DEHALO_KME_DEHALO_BC_reg,17,22, 0x3f);
		WriteRegister(MC_MC_B8_reg,11,11, 0);
		WriteRegister(MC_MC_B4_reg,16,23, 0x00);
		WriteRegister(MC_MC_B4_reg,24,31, 0x00);
		//WriteRegister(KIWI_REG(FRC_TOP__PQL_1__pql_fb_ctrl_en), 0);//Merlin7 do not have it 
		WriteRegister(MC_MC_B0_reg, 20 ,27, 0);
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 0);
		u5_blend_holdfrm_a[6] = 60;
		nIdentifiedNum_a[6]=6;
	}
	else if(u5_blend_holdfrm_a[6] >0)
	{
		//WriteRegister(FRC_TOP__PQL_0__pql_patch_dummy_e4_ADDR,7,7, 0);//Merlin7 does not have it
		WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x00);
		WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0,7, 0x00);
		WriteRegister(KMC_BI_bi_top_01_reg,2,2, 0);

		//WriteRegister(FRC_TOP__PQL_1__pql_wr_dhclose_wrt_en_ADDR,4,4, 0);//Merlin7 do not have it 
		WriteRegister(KME_DEHALO_KME_DEHALO_BC_reg,17,22, 0x3f);
		WriteRegister(MC_MC_B8_reg,11,11, 0);
		WriteRegister(MC_MC_B4_reg,23,16, 0x00);
		WriteRegister(MC_MC_B4_reg,24,31, 0x00);
		//WriteRegister(KIWI_REG(FRC_TOP__PQL_1__pql_fb_ctrl_en), 0);//Merlin7 do not have it 
		WriteRegister(MC_MC_B0_reg, 20 ,27, 0);
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 0);
		u5_blend_holdfrm_a[6]--;
	}
	else if(nIdentifiedNum_a[6] == 6 )
	{
		//WriteRegister(FRC_TOP__PQL_0__pql_patch_dummy_e4_ADDR,7,7, 1);//Merlin7 does not have it
		WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x0a);
		WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0,7, 0x12);
		WriteRegister(KMC_BI_bi_top_01_reg,2,2, 1);

		//WriteRegister(FRC_TOP__PQL_1__pql_wr_dhclose_wrt_en_ADDR,4,4, 1);//Merlin7 do not have it 
		WriteRegister(KME_DEHALO_KME_DEHALO_BC_reg,17,22, 0x10);
		WriteRegister(MC_MC_B8_reg,11,11, 1);
		WriteRegister(MC_MC_B4_reg,16,23, 0x68);
		WriteRegister(MC_MC_B4_reg,24,31, 0xc0);
		//WriteRegister(KIWI_REG(FRC_TOP__PQL_1__pql_fb_ctrl_en), 1);//Merlin7 do not have it 
		WriteRegister(MC_MC_28_reg, 14, 14, 0);
		nIdentifiedNum_a[6] = 0;
	}
	
	{
	/*Midway.4khdr_13_Propeller*/
	static unsigned int mc_logo_en = 3, dh_dtl_curve_y1 = 0x0a, dh_fb_dtl_shift = 7;
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,43) == TRUE) //histogram from h5x
	{
		WriteRegister(MC2_MC2_20_reg, 0, 1, 0);
		WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x32);
		WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 20, 22, 6);
		u5_blend_holdfrm_a[7] = 32;
		nIdentifiedNum_a[7]=7;
	}
	else if(u5_blend_holdfrm_a[7] >0)
	{
		WriteRegister(MC2_MC2_20_reg, 0, 1, 0);
		WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x32);
		WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 20, 22, 6);
		u5_blend_holdfrm_a[7]--;
	}
	else if(nIdentifiedNum_a[7] == 7 )
	{
		WriteRegister(MC2_MC2_20_reg, 0, 1, mc_logo_en);
		WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, dh_dtl_curve_y1);
		WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 20, 22, dh_fb_dtl_shift);
		nIdentifiedNum_a[7] = 0;
	}
	}
	
	/*THE_ECLIPSE.fhd24_01.mkv*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,28) == TRUE) //histogram from h5x
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8, 11,  1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 21, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8, 11,  1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 21, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 1);
		u5_blend_holdfrm_a[8] = 16;
		nIdentifiedNum_a[8]=8;
	}
	else if(u5_blend_holdfrm_a[8] >0)
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8, 11,  1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 21, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,   1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8, 11,  1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 21, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 1);
		u5_blend_holdfrm_a[8]--;
	}
	else if(nIdentifiedNum_a[8] == 8)
	{
		nIdentifiedNum_a[8] = 0;
	}

	/*Valley of fire_breakup.mp4*/	
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,35) == TRUE) //histogram from h5x
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,15,15, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,16,16, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,14,14, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,15,15, 0);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_34_reg,  0,0, 0);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_A4_reg,9,9, 0);
		u5_blend_holdfrm_a[9] = 16;
		nIdentifiedNum_a[9]=9;
	}
	else if(u5_blend_holdfrm_a[9] >0)
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,15,15, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,16,16, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,14,14, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,15,15, 0);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_34_reg,  0,0, 0);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_A4_reg,9,9, 0);
		u5_blend_holdfrm_a[9]--;
	}
	else if(nIdentifiedNum_a[9] == 9)
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,15,15, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,16,16, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,14,14, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,15,15, 1);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_34_reg,  0,0,1);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_A4_reg,9,9, 1);
		nIdentifiedNum_a[9] = 0;
	}
{	
	/*3.QTEK_6.5PPF_00081*/ /*monosco4K_Hscroll_10pix_50Hz_long_50p-3840*2160*/
	if(s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb > 27000 && (_ABS_(u11_gmv_mvx>>1) >2	&& _ABS_(u10_gmv_mvy>>1) ==0) && (scalerVIP_Get_MEMCPatternFlag_Identification(5,48) == TRUE||scalerVIP_Get_MEMCPatternFlag_Identification(5,41) == TRUE)) //histogram from h5x
	{
		//WriteRegister(FRC_TOP__PQL_0__pql_patch_dummy_e4_ADDR,29,29, 0);//Merlin7 does not have it
		WriteRegister(MC2_MC2_20_reg, 0, 1, 0);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_2C_reg, 0, 12, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 3, 3, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 6, 6, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 7, 7, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 8, 8, 0);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 2, 2, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 5, 5, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 6, 6, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 7, 7, 0);

		rtd_clearbits(0xb802b500, _BIT0);//  sharpness peaking
		rtd_clearbits(0xb802b950, _BIT0);//  sharpness dlti
		rtd_clearbits(0xb8023c00, _BIT0);//  Multiband peaking
		rtd_setbits(0xb8023c30, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_a[10] = 120;
		nIdentifiedNum_a[10]=10;
	}
	else if(u5_blend_holdfrm_a[10] >0)
	{
		//WriteRegister(FRC_TOP__PQL_0__pql_patch_dummy_e4_ADDR,29,29, 0);//Merlin7 does not have it
		WriteRegister(MC2_MC2_20_reg, 0, 1, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 3, 3, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 6, 6, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 7, 7, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 8, 8, 0);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 2, 2, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 5, 5, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 6, 6, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 7, 7, 0);
		rtd_clearbits(0xb802b500, _BIT0);//  sharpness peaking
		rtd_clearbits(0xb802b950, _BIT0);//  sharpness dlti
		rtd_clearbits(0xb8023c00, _BIT0);//  Multiband peaking
		rtd_setbits(0xb8023c30, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_a[10]--;
	}else if(nIdentifiedNum_a[10]==10 ){
		//WriteRegister(FRC_TOP__PQL_0__pql_patch_dummy_e4_ADDR,29,29, 1);//Merlin7 does not have it
		WriteRegister(MC2_MC2_20_reg, 0, 1, 3);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_2C_reg, 0, 12, 300);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 3, 3, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 6, 6, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 7, 7, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 8, 8, 1);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 2, 2, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 5, 5, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 6, 6, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 7, 7, 1);
		rtd_setbits(0xb802b500, _BIT0);//  sharpness peaking
		rtd_setbits(0xb802b950, _BIT0);//  sharpness dlti
		rtd_setbits(0xb8023c00, _BIT0);//  Multiband peaking
		rtd_setbits(0xb8023c30, _BIT0);//  Multiband peaking DB apply
		nIdentifiedNum_a[10]=0;
	}
}

	/*4K.znds.com]ShutUpAndPlay.Berlin.2014.2160p.HDTV.HEVC-jTV-50p-3840x2160*/
	//0:40-0:43
	if(/*u1_still_frame != 0 && */scalerVIP_Get_MEMCPatternFlag_Identification(5,55) == TRUE) //histogram from h5x
	{
		WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 23, 23, 0);
		u5_blend_holdfrm_a[11] = 16;
		nIdentifiedNum_a[11]=11;
	}
	else if(u5_blend_holdfrm_a[11] >0)
	{
		WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 23, 23, 0);
		u5_blend_holdfrm_a[11]--;
	}
	else if(nIdentifiedNum_a[11] == 11 )
	{
		WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 23, 23, 1);
		nIdentifiedNum_a[11] = 0;
	}

{
	static unsigned int frame_meander_enable = 0, me1_mvd_dgain_slope2 = 0, me1_mvd_dgain_y1 = 0, me1_mvd_dgain_y2 = 0, kmc_blend_uv_alpha = 0;
	/*DMN Cityscapes 720p60*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,50) == TRUE) //histogram from h5x
	{
		if(nIdentifiedNum_a[12] == 0)
		{
			ReadRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg, 8, 17, &me1_mvd_dgain_y1);
			ReadRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg, 18, 27,&me1_mvd_dgain_y2);
			ReadRegister(KME_ME1_TOP1_ME1_COMMON1_12_reg, 16, 31,&me1_mvd_dgain_slope2);
			ReadRegister(KME_TOP_KME_TOP_04_reg, 29, 29,&frame_meander_enable);
			ReadRegister(IPPRE_IPPRE_04_reg, 16, 23,&kmc_blend_uv_alpha);
		}
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 0);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg, 8, 17,  32);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg, 18, 27, 32);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_12_reg, 16, 31, 65532);
		WriteRegister(KME_TOP_KME_TOP_04_reg, 29, 29, 0);
		WriteRegister(IPPRE_IPPRE_04_reg, 16, 23, 150);
		u5_blend_holdfrm_a[12] = 16;
		nIdentifiedNum_a[12]=12;
	}
	else if(u5_blend_holdfrm_a[12] >0)
	{
		WriteRegister(MC_MC_28_reg, 14, 14, 1);
		WriteRegister(MC_MC_28_reg, 15, 22, 0);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg, 8, 17,  32);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg, 18, 27, 32);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_12_reg, 16, 31, 65532);
		WriteRegister(KME_TOP_KME_TOP_04_reg, 29, 29, 0);
		WriteRegister(IPPRE_IPPRE_04_reg, 16, 23, 150);
		u5_blend_holdfrm_a[12]--;
	}
	else if(nIdentifiedNum_a[12] == 12 )
	{
		WriteRegister(MC_MC_28_reg, 14, 14, 0);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg, 8, 17,  me1_mvd_dgain_y1);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_10_reg, 18, 27, me1_mvd_dgain_y2);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_12_reg, 16, 31, me1_mvd_dgain_slope2);
		WriteRegister(KME_TOP_KME_TOP_04_reg, 29, 29, frame_meander_enable);
		WriteRegister(IPPRE_IPPRE_04_reg, 16, 23, kmc_blend_uv_alpha);
		nIdentifiedNum_a[12] = 0;
	}
}
	
	/*fusion_PW_120Hz_l_Robot_(SD_NTSC)_02.mpg*/
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5,7) == TRUE) //histogram from h5x
	{
		WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 23, 23, 1);
		WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 20, 22, 5);
		WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 128);
		u5_blend_holdfrm_a[13] = 60;
		nIdentifiedNum_a[13]=13;
	}
	else if(u5_blend_holdfrm_a[13] >0)
	{
		WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 23, 23, 1);
		WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 20, 22, 5);
		WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 128);
		u5_blend_holdfrm_a[13]--;
	}
	else if(nIdentifiedNum_a[13] == 13 )
	{
		WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 23, 23, 0);
		WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 20, 22, 7);
		WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0);
		nIdentifiedNum_a[13] = 0;
	}
	
		/*casino_royale_halo_1_09_00.mp4-one*/
		if(scalerVIP_Get_MEMCPatternFlag_Identification(5,63) == TRUE) //histogram from h5x
		{
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,   15);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,   15);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8, 11,  15);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 15);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 21, 15);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 15);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 15);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, 15);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,   15);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,   15);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8, 11,  15);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 15);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 21, 15);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 15);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 15);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 15);
			//WriteRegister(KIWI_REG(FRC_TOP__MC__reg_mc_fb_lvl_force_en), 1);
			//WriteRegister(KIWI_REG(FRC_TOP__MC__reg_mc_fb_lvl_force_value), 90);
			u5_blend_holdfrm_a[14] = 16;
			nIdentifiedNum_a[14]=14;
		}else if(u5_blend_holdfrm_a[14] >0){
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,   15);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,   15);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8, 11,  15);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 15);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 21, 15);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 15);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 15);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, 15);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,   15);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,   15);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8, 11,  15);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 15);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 21, 15);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 15);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 15);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 15);
			//WriteRegister(KIWI_REG(FRC_TOP__MC__reg_mc_fb_lvl_force_en), 1);
			//WriteRegister(KIWI_REG(FRC_TOP__MC__reg_mc_fb_lvl_force_value), 90);
			u5_blend_holdfrm_a[14]--;
		}else if(nIdentifiedNum_a[14] == 14 ){
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3,   3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7,   3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8, 11,  3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 21, 3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, 3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3,   3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7,   3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8, 11,  3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 21, 3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 3);
			//WriteRegister(KIWI_REG(FRC_TOP__MC__reg_mc_fb_lvl_force_en), 0);
			nIdentifiedNum_a[14] = 0;
		}

		/*Roku ultra box: Netflix: Breaking bad S5E5 33:55 standing man ghost after dust of truck*/
		/*Roku ultra box: Netflix: Lucifer S2E2, 9:20, broken on black building*/
		/*1080P-motion-res-test-pattern*/
		if(scalerVIP_Get_MEMCPatternFlag_Identification(5,74) == TRUE || scalerVIP_Get_MEMCPatternFlag_Identification(5,36) == TRUE || scalerVIP_Get_MEMCPatternFlag_Identification(5,9) == TRUE) //histogram from h5x
		{
			//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 3, 3, 0);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 0);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, 0);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 6, 6, 0);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 7, 7, 0);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 8, 8, 0);
			
			//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 2, 2, 0);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, 0);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4, 0);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 5, 5, 0);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 6, 6, 0);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 7, 7, 0);
			u5_blend_holdfrm_a[15] = 100;
			nIdentifiedNum_a[15]=15;
		}else if(u5_blend_holdfrm_a[15] >0){
			//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 3, 3, 0);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 0);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, 0);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 6, 6, 0);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 7, 7, 0);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 8, 8, 0);
			
			//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 2, 2, 0);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, 0);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4, 0);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 5, 5, 0);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 6, 6, 0);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 7, 7, 0);
			u5_blend_holdfrm_a[15]--;
		}else if(nIdentifiedNum_a[15] == 15 ){
			//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 3, 3, 1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, 1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 6, 6, 1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 7, 7, 1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 8, 8, 1);
			
			//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 2, 2, 1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, 1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4, 1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 5, 5, 1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 6, 6, 1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 7, 7, 1);
			nIdentifiedNum_a[15] = 0;
		}
	//Rting 24P ghost
{
	unsigned char i = 0, rmv_y_zero_count = 0, rmv_y_greater_than_thre_count = 0;
	for(i = 0; i < 32; i++)
		if(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i] == 0)
			rmv_y_zero_count++;
	for(i = 0; i < 32; i++)
		if(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i] > 600 || s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i] < 600)
			rmv_y_greater_than_thre_count++;
	if(!u1_still_frame && rmv_y_zero_count < 32 && rmv_y_zero_count > 25 && rmv_y_greater_than_thre_count > 2 && scalerVIP_Get_MEMCPatternFlag_Identification(5,77) == TRUE)
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 3, 3, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 6, 6, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 7, 7, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 8, 8, 0);
		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 2, 2, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 5, 5, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 6, 6, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 7, 7, 0);
		u5_blend_holdfrm_a[16] = 64;
		nIdentifiedNum_a[16]=16;
	}else if(u5_blend_holdfrm_a[16] >0){
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 3, 3, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 6, 6, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 7, 7, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 8, 8, 0);
		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 2, 2, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 5, 5, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 6, 6, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 7, 7, 0);
		u5_blend_holdfrm_a[16]--;
	}else if(nIdentifiedNum_a[16] == 16 ){
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 3, 3, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 6, 6, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 7, 7, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 8, 8, 1);
		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 2, 2, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 5, 5, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 6, 6, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 7, 7, 1);
		nIdentifiedNum_a[16] = 0;
	}
}




#if 1
	//===debug show
	for(u8_Index =0;u8_Index<255;u8_Index++)
	{
		if(nIdentifiedNum_a[u8_Index] !=0){
			pOutput->u1_IP_wrtAction_true=1;
			if(u32_ID_Log_en==1)
				rtd_pr_memc_emerg("[IPR_043]nIdentifiedNum_a>>[%d][%d]	 ,\n", u8_Index, nIdentifiedNum_a[u8_Index]);
		}
	}
#endif

}


#if 0
VOID Identification_Pattern_preProcess(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	_PQLCONTEXT *s_pContext = GetPQLContext_m();
	static unsigned char  u5_blend_holdfrm_preProcess_a[255] = {0};
	static unsigned char  nIdentifiedNum_preProcess_a[255] = {0};
	unsigned char  u8_Index;
	unsigned int  u32_RB_val;
	unsigned int  u32_pattern214_forbindden_en;
	unsigned char  u1_still_frame=s_pContext->_output_frc_sceneAnalysis.u1_still_frame;
	short  u11_gmv_mvx = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
	short  u10_gmv_mvy = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;
	unsigned int  u25_me_aDTL = s_pContext->_output_read_comreg.u25_me_aDTL_rb;
	unsigned short  u32_gmv_unconf = s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb;
	int  fwgmvtest  , fwgmven ; 

	int gmv_calc_x = 0;
    int gmv_calc_y = 0;
	static unsigned int u32_RgnApl[32] = {0};
	int s11_rmv_mvx[32] = {0};
	int s10_rmv_mvy[32] = {0};
	
	int gmv_calc_rapl_min_idx = 0;
	int gmv_calc_rapl_min_temp = 0;
	static int pre_calc_gmv_mvx = 0;
	static int pre_calc_gmv_rgn_idx = 0;
	int rmv_temp_x = 0;
	static int frameCnt_tmp = 1;

	int reg_gmvd_th = 10;
	int cond0 = 0,cond1 = 0;
	int gmvd_calc_x = 0;
	int gmvd_calc_x_min = 255;
	int gmvd_calc_x_min_idx = 0;
	unsigned int nIdxX = 0;

	static int incnt_for_299 = 0;

	unsigned int swap_s1_flag = 0;
	unsigned int swap_s2_flag = 0;
	//unsigned int print_final_en = 0;
	unsigned int print_detail_en = 0;//for debug.
	int right_rim_pre;
	
#if 0
	unsigned int u25_me_unconf = (s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[9] + s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[10]
					     + s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[11] + s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[12]
					     + s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[13] + s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[14]
					     + s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[17] + s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[18]
					     + s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[19] + s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[20]
					     + s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[21] + s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[22])/12;
#endif

	#if 1 //for YE Test temp test
	unsigned int u32_ID_Log_en=0;//,ES_Test_Value=0;
	ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 24,  24, &u32_ID_Log_en);//YE Test debug dummy register
	#endif

	#if Pro_TCL
	//unsigned char  u8_cadence_Id = s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL];
	//PQL_INPUT_FRAME_RATE in_fmRate = s_pContext->_external_data._input_frameRate;
	unsigned char nblend_y_alpha_Target=0xa0;
	unsigned int u32_RB_kmc_blend_y_alpha;
	unsigned int u25_rgnSad_rb12 = s_pContext->_output_read_comreg.u25_me_rSAD_rb[12];
	unsigned int u25_rgnSad_rb23 = s_pContext->_output_read_comreg.u25_me_rSAD_rb[23];

     	//TOP_ratio = ((_ABS_( s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_TOP] - s_pContext->_output_rimctrl.u12_out_resolution[_RIM_TOP]))<<10)>>11;
	BOT_ratio = ((_ABS_(s_pContext->_output_rimctrl.u12_out_resolution[_RIM_BOT] - s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_BOT]))<<10)>>11;

	

	ReadRegister(IPPRE_IPPRE_04_reg, 4, 11, &u32_RB_kmc_blend_y_alpha);
	#endif

	unsigned char test_en = 0;
	ReadRegister(HARDWARE_HARDWARE_58_reg, 0, 31, &u32_RB_val);   //  bit0
	ReadRegister(HARDWARE_HARDWARE_58_reg, 1, 1, &u32_pattern214_forbindden_en);   //  bit1


	if((pParam->u1_Identification_Pattern_en== 0)||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_Test_0_flag ==0))
		return;

	//reset
	pOutput->u1_IP_preProcess_true=0;

	if(s_pContext->_output_me_vst_ctrl.u1_detect_blackBG_VST_trure)
	{
		pOutput->u1_casino_RP_detect_true =0;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
	}

	if(scalerVIP_Get_MEMCPatternFlag_Identification(3) == TRUE  )
	{
		pOutput->u1_casino_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[0] = 12;
		nIdentifiedNum_preProcess_a[0]=3;
	}
	else if(u5_blend_holdfrm_preProcess_a[0] >0)
	{
		pOutput->u1_casino_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[0]--;
	}
	else if(nIdentifiedNum_preProcess_a[0] == 3 )
	{
		pOutput->u1_casino_RP_detect_true =0;
		nIdentifiedNum_preProcess_a[0] = 0;
	}

	test_en = (rtd_inl(SOFTWARE1_SOFTWARE1_62_reg)>>1)&0x1;
	if(test_en){
		pOutput->u1_casino_RP_detect_true =1;
	}
	
	//=============================================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(5) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		if(s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true >= 3){
			pOutput->u1_casino_RP_detect_true =1;
			u5_blend_holdfrm_preProcess_a[1] = 3;
			nIdentifiedNum_preProcess_a[1]=5;

		}
	}
	else if(u5_blend_holdfrm_preProcess_a[1] >0)
	{
		pOutput->u1_casino_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[1]--;
	}
	else if(nIdentifiedNum_preProcess_a[1] == 5 )
	{
		pOutput->u1_casino_RP_detect_true =0;
		nIdentifiedNum_preProcess_a[1] = 0;
	}
	//=============================================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(6) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =3;
		u5_blend_holdfrm_preProcess_a[2] = 16;
		nIdentifiedNum_preProcess_a[2]=6;
	}
	else if(u5_blend_holdfrm_preProcess_a[2] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =3;
		u5_blend_holdfrm_preProcess_a[2]--;
	}
	else if(nIdentifiedNum_preProcess_a[2] == 6 )
	{
		nIdentifiedNum_preProcess_a[2] = 0;
	}
	//=============================================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(7) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
		u5_blend_holdfrm_preProcess_a[3] = 16;
		nIdentifiedNum_preProcess_a[3]=7;
	}
	else if(u5_blend_holdfrm_preProcess_a[3] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
		u5_blend_holdfrm_preProcess_a[3]--;
	}
	else if(nIdentifiedNum_preProcess_a[3] == 7 )
	{
		nIdentifiedNum_preProcess_a[3] = 0;
	}
	//=============================================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(22) == TRUE  )
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[3] = 5;
		nIdentifiedNum_preProcess_a[3]=22;
	}
	else if(u5_blend_holdfrm_preProcess_a[3] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[3]--;
	}
	else if(nIdentifiedNum_preProcess_a[3] == 22 )
	{
		nIdentifiedNum_preProcess_a[3] = 0;
	}
#if 0
	#if Pro_TCL
	//=============================================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(8) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, (u32_RB_kmc_blend_y_alpha+nblend_y_alpha_Target)/2);
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type =1;  // more me1
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true =1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =2;
		rtd_clearbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_clearbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[4] = 4;
		nIdentifiedNum_preProcess_a[4]=8;
	}
	else if(u5_blend_holdfrm_preProcess_a[4] >0)
	{
		WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, (u32_RB_kmc_blend_y_alpha+nblend_y_alpha_Target)/2);
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type =1;  // more me1
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true =1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =2;
		rtd_clearbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_clearbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[4]--;
	}
	else if(nIdentifiedNum_preProcess_a[4] == 8 && (u32_RB_kmc_blend_y_alpha !=s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha))
	{
		if(u32_RB_kmc_blend_y_alpha <(s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha))
		{
			if((s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha) -u32_RB_kmc_blend_y_alpha >1)
				WriteRegister(IPPRE_IPPRE_04_reg, 4, 11,(u32_RB_kmc_blend_y_alpha+1));
			else
				WriteRegister(IPPRE_IPPRE_04_reg, 4, 11,s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha);
		}
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type =0;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true =0;
		rtd_setbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_setbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
	}
	else if(nIdentifiedNum_preProcess_a[4] == 8 )
	{
		nIdentifiedNum_preProcess_a[4] = 0;
	}
	//============================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(9) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		u5_blend_holdfrm_preProcess_a[5] = 16;
		nIdentifiedNum_preProcess_a[5]=9;
	}
	else if(u5_blend_holdfrm_preProcess_a[5] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		u5_blend_holdfrm_preProcess_a[5]--;
	}
	else if(nIdentifiedNum_preProcess_a[5] == 9 )
	{
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=0;
		nIdentifiedNum_preProcess_a[5] = 0;
	}
	//===========================================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(10) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		rtd_clearbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_clearbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;
		u5_blend_holdfrm_preProcess_a[6] = 16;
		nIdentifiedNum_preProcess_a[6]=10;
	}
	else if(u5_blend_holdfrm_preProcess_a[6] >0)
	{
		rtd_clearbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_clearbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;
		u5_blend_holdfrm_preProcess_a[6]--;
	}
	else if(nIdentifiedNum_preProcess_a[6] == 10 )
	{
		rtd_setbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_setbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=0;
		nIdentifiedNum_preProcess_a[6] = 0;
	}
	//===========================================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(11) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[7] = 8;
		nIdentifiedNum_preProcess_a[7]=11;
	}
	else if(u5_blend_holdfrm_preProcess_a[7] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[7]--;
	}
	else if(nIdentifiedNum_preProcess_a[7] == 11 )
	{
		nIdentifiedNum_preProcess_a[7] = 0;
	}
	//===================================================

	if(scalerVIP_Get_MEMCPatternFlag_Identification(13) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ) )
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
		s_pContext->_output_frc_sceneAnalysis.u1_Swing_true = 1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[8] = 16;
		nIdentifiedNum_preProcess_a[8]=13;
	}
	else if(u5_blend_holdfrm_preProcess_a[8] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
		s_pContext->_output_frc_sceneAnalysis.u1_Swing_true = 1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[8]--;
	}
	else if(nIdentifiedNum_preProcess_a[8] == 13 )
	{
		s_pContext->_output_frc_sceneAnalysis.u1_Swing_true = 0;
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		nIdentifiedNum_preProcess_a[8] = 0;
	}
	//==============================================
	#endif
#endif

#if 0
	//==============================================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(15) == TRUE  )
	{
		//s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[9] = 8;
		nIdentifiedNum_preProcess_a[9]=15;
	}
	else if(u5_blend_holdfrm_preProcess_a[9] >0)
	{
		//s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[9]--;
	}
	else if(nIdentifiedNum_preProcess_a[9] == 15 )
	{
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		nIdentifiedNum_preProcess_a[9] = 0;
	}
	//=====================================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(16) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))    //  can't use motion or dtl info condition
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		WriteRegister(IPPRE_IPPRE_0C_reg, 0, 7, 0x10);
		rtd_clearbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		u5_blend_holdfrm_preProcess_a[10] = 8;
		nIdentifiedNum_preProcess_a[10]=16;
	}
	else if(u5_blend_holdfrm_preProcess_a[10] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		WriteRegister(IPPRE_IPPRE_0C_reg, 0, 7, 0x10);
		rtd_clearbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		u5_blend_holdfrm_preProcess_a[10]--;
	}
	else if(nIdentifiedNum_preProcess_a[10] == 16 )
	{
		WriteRegister(IPPRE_IPPRE_0C_reg, 0, 7, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_logo_y_alpha);
		rtd_setbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		nIdentifiedNum_preProcess_a[10] = 0;
	}
#endif
	//==================================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(17) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))  //  can't use motion or dtl info condition
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
		u5_blend_holdfrm_preProcess_a[11] = 8;
		nIdentifiedNum_preProcess_a[11]=17;
	}
	else if(u5_blend_holdfrm_preProcess_a[11] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
		u5_blend_holdfrm_preProcess_a[11]--;
	}
	else if(nIdentifiedNum_preProcess_a[11] == 17 )
	{
		nIdentifiedNum_preProcess_a[11] = 0;
	}
	//==================================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(20) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[12] = 16;
		nIdentifiedNum_preProcess_a[12]=20;
	}
	else if(u5_blend_holdfrm_preProcess_a[12] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[12]--;
	}
	else if(nIdentifiedNum_preProcess_a[12] == 20 )
	{
		nIdentifiedNum_preProcess_a[12] = 0;
	}
	//=================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(23) == TRUE  )
	{
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true =1;
		s_pContext->_output_me_sceneAnalysis.u2_panning_flag =2;
		u5_blend_holdfrm_preProcess_a[13] = 16;
		nIdentifiedNum_preProcess_a[13]=23;
	}
	else if(u5_blend_holdfrm_preProcess_a[13] >0)
	{
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true=1;
		s_pContext->_output_me_sceneAnalysis.u2_panning_flag =2;
		u5_blend_holdfrm_preProcess_a[13]--;
	}
	else if(nIdentifiedNum_preProcess_a[13] == 23 )
	{
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true =0;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[13] = 0;
	}
	//========================================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(24) == TRUE )
	{
		if((_ABS_(u11_gmv_mvx>>1) <=1   && _ABS_(u10_gmv_mvy>>1) <=1)&& (u25_me_aDTL > 1100000)&&( u1_still_frame == 0))
		{
			s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true =1;
			s_pContext->_output_me_sceneAnalysis.u2_panning_flag =2;
			u5_blend_holdfrm_preProcess_a[14] = 16;
			nIdentifiedNum_preProcess_a[14]=24;
		}

		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;

	}
	else if(u5_blend_holdfrm_preProcess_a[14] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true =1;
		s_pContext->_output_me_sceneAnalysis.u2_panning_flag =2;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[14]--;
	}
	else if(nIdentifiedNum_preProcess_a[14] == 24 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true =0;
		nIdentifiedNum_preProcess_a[14] = 0;
	}
	//===============================================
	#if Pro_TCL
	if(scalerVIP_Get_MEMCPatternFlag_Identification(25) == TRUE &&( u1_still_frame == 0)&&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, (u32_RB_kmc_blend_y_alpha+0x50)/2);
		s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVx_Step=0;
		s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVy_Step=0;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =3;
		u5_blend_holdfrm_preProcess_a[15] = 8;
		nIdentifiedNum_preProcess_a[15]=25;
	}
	else if(u5_blend_holdfrm_preProcess_a[15] >0)
	{
		WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, (u32_RB_kmc_blend_y_alpha+0x50)/2);
		s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVx_Step=0;
		s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVy_Step=0;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =3;
		u5_blend_holdfrm_preProcess_a[15]--;
	}
	else if(nIdentifiedNum_preProcess_a[15] == 25 )
	{
		WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha);
		nIdentifiedNum_preProcess_a[15] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(26) == TRUE  &&( u1_still_frame == 0)&&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true =1;
		u5_blend_holdfrm_preProcess_a[16] = 8;
		nIdentifiedNum_preProcess_a[16]=26;
	}
	else if(u5_blend_holdfrm_preProcess_a[16] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true =1;
		u5_blend_holdfrm_preProcess_a[16]--;
	}
	else if(nIdentifiedNum_preProcess_a[16] == 26 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true =0;
		nIdentifiedNum_preProcess_a[16] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(27) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[17] = 8;
		nIdentifiedNum_preProcess_a[17]=27;
	}
	else if(u5_blend_holdfrm_preProcess_a[17] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[17]--;
	}
	else if(nIdentifiedNum_preProcess_a[17] == 27 )
	{
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		nIdentifiedNum_preProcess_a[17] = 0;
	}
	#endif

	 //YE Test not me add this  it's 376_CMR_XmenApocalypse  ID=28
	if(scalerVIP_Get_MEMCPatternFlag_Identification(28) == TRUE && (u25_me_aDTL > 700000)&&( u1_still_frame == 0))
	{
		if(((u32_RB_val >>  0) & 0x01) ==1){
			s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true =1;
			s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=1;
		}
		else
			s_pContext->_output_wrt_comreg.u1_GFB_specialScene_true=1;
		u5_blend_holdfrm_preProcess_a[18] = 8;
		nIdentifiedNum_preProcess_a[18]=28;
	}
	else if(u5_blend_holdfrm_preProcess_a[18] >0)
	{
		if(((u32_RB_val >>  0) & 0x01) ==1){
			s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true =1;
			s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=1;
		}
		else
			s_pContext->_output_wrt_comreg.u1_GFB_specialScene_true=1;
		u5_blend_holdfrm_preProcess_a[18]--;
	}
	else if(nIdentifiedNum_preProcess_a[18] == 28 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_Slow_Convergence_true =0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=0;

		s_pContext->_output_wrt_comreg.u1_GFB_specialScene_true=0;
		nIdentifiedNum_preProcess_a[18] = 0;
	}

	#if Pro_TCL
	if(scalerVIP_Get_MEMCPatternFlag_Identification(29) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[19] = 8;
		nIdentifiedNum_preProcess_a[19]=29;
	}
	else if(u5_blend_holdfrm_preProcess_a[19] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[19]--;
	}
	else if(nIdentifiedNum_preProcess_a[19] == 29 )
	{
		nIdentifiedNum_preProcess_a[19] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(31) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_GFB_specialScene_true=1;
		u5_blend_holdfrm_preProcess_a[20] = 8;
		nIdentifiedNum_preProcess_a[20]=31;
	}
	else if(u5_blend_holdfrm_preProcess_a[20] >0)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_specialScene_true=1;
		u5_blend_holdfrm_preProcess_a[20]--;
	}
	else if(nIdentifiedNum_preProcess_a[20] == 31 )
	{
		s_pContext->_output_wrt_comreg.u1_GFB_specialScene_true=0;
		nIdentifiedNum_preProcess_a[20] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(32) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u2_panning_flag =1;
		s_pContext->_output_wrt_comreg.u1_MC_SmallObject_type=1;
		u5_blend_holdfrm_preProcess_a[21] = 8;
		nIdentifiedNum_preProcess_a[21]=32;
	}
	else if(u5_blend_holdfrm_preProcess_a[21] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u2_panning_flag =1;
		s_pContext->_output_wrt_comreg.u1_MC_SmallObject_type=1;
		u5_blend_holdfrm_preProcess_a[21]--;
	}
	else if(nIdentifiedNum_preProcess_a[21] == 32 )
	{
		s_pContext->_output_wrt_comreg.u1_MC_SmallObject_type=0;
		nIdentifiedNum_preProcess_a[21] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(33) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u2_panning_flag =1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		s_pContext->_output_wrt_comreg.u1_MC_SmallObject_type=1;
		u5_blend_holdfrm_preProcess_a[22] = 12;
		nIdentifiedNum_preProcess_a[22]=33;
	}
	else if(u5_blend_holdfrm_preProcess_a[22] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u2_panning_flag =1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		s_pContext->_output_wrt_comreg.u1_MC_SmallObject_type=1;
		u5_blend_holdfrm_preProcess_a[22]--;
	}
	else if(nIdentifiedNum_preProcess_a[22] == 33 )
	{
		s_pContext->_output_wrt_comreg.u1_MC_SmallObject_type=0;
		nIdentifiedNum_preProcess_a[22] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(34) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =3;
		u5_blend_holdfrm_preProcess_a[23] = 8;
		nIdentifiedNum_preProcess_a[23]=34;
	}
	else if(u5_blend_holdfrm_preProcess_a[23] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =3;
		u5_blend_holdfrm_preProcess_a[23]--;
	}
	else if(nIdentifiedNum_preProcess_a[23] == 34 )
	{
		nIdentifiedNum_preProcess_a[23] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(35) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[24] = 8;
		nIdentifiedNum_preProcess_a[24]=35;
	}
	else if(u5_blend_holdfrm_preProcess_a[24] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[24]--;
	}
	else if(nIdentifiedNum_preProcess_a[24] == 35 )
	{
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		nIdentifiedNum_preProcess_a[24] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(36) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[25] = 8;
		nIdentifiedNum_preProcess_a[25]=36;
	}
	else if(u5_blend_holdfrm_preProcess_a[25] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[25]--;
	}
	else if(nIdentifiedNum_preProcess_a[25] == 36 )
	{
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		nIdentifiedNum_preProcess_a[25] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(37) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;
		u5_blend_holdfrm_preProcess_a[26] = 8;
		nIdentifiedNum_preProcess_a[26]=37;
	}
	else if(u5_blend_holdfrm_preProcess_a[26] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;
		u5_blend_holdfrm_preProcess_a[26]--;
	}
	else if(nIdentifiedNum_preProcess_a[26] == 37 )
	{
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=0;
		nIdentifiedNum_preProcess_a[26] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(38) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_MC_SmallObject_type=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[27] = 8;
		nIdentifiedNum_preProcess_a[27]=38;
	}
	else if(u5_blend_holdfrm_preProcess_a[27] >0)
	{
		s_pContext->_output_wrt_comreg.u1_MC_SmallObject_type=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[27]--;
	}
	else if(nIdentifiedNum_preProcess_a[27] == 38 )
	{
		s_pContext->_output_wrt_comreg.u1_MC_SmallObject_type=0;
		nIdentifiedNum_preProcess_a[27] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(39) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=3;
		s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[28] = 8;
		nIdentifiedNum_preProcess_a[28]=39;
	}
	else if(u5_blend_holdfrm_preProcess_a[28] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=3;
		s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[28]--;
	}
	else if(nIdentifiedNum_preProcess_a[28] == 39 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=0;
		s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type=0;
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		nIdentifiedNum_preProcess_a[28] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(40) == TRUE &&u25_rgnSad_rb12 ==0&&u25_rgnSad_rb23<= 250 &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=2;  // more me2
		s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;  // Because pq adjustment
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;
		WriteRegister(MC_MC_B0_reg, 8, 10, 0x4);
		WriteRegister(KME_TOP_KME_TOP_04_reg,30, 30, 0x1);
		u5_blend_holdfrm_preProcess_a[29] = 8;
		nIdentifiedNum_preProcess_a[29]=40;
	}
	else if(u5_blend_holdfrm_preProcess_a[29] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=2;  // more me2
		s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;
		WriteRegister(MC_MC_B0_reg, 8, 10, 0x4);
		WriteRegister(KME_TOP_KME_TOP_04_reg,30, 30, 0x1);
		u5_blend_holdfrm_preProcess_a[29]--;
	}
	else if(nIdentifiedNum_preProcess_a[29] == 40 )
	{
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=0;
		s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=0;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=0;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=0;
		WriteRegister(MC_MC_B0_reg, 8, 10, 0x3);
		WriteRegister(KME_TOP_KME_TOP_04_reg,30, 30, 0x0);
		nIdentifiedNum_preProcess_a[29] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(41) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[30] = 16;
		nIdentifiedNum_preProcess_a[30]=41;
	}
	else if(u5_blend_holdfrm_preProcess_a[30] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[30]--;
	}
	else if(nIdentifiedNum_preProcess_a[30] == 41 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		nIdentifiedNum_preProcess_a[30] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(42) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[31] = 16;
		nIdentifiedNum_preProcess_a[31]=42;
	}
	else if(u5_blend_holdfrm_preProcess_a[31] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[31]--;
	}
	else if(nIdentifiedNum_preProcess_a[31] == 42 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		nIdentifiedNum_preProcess_a[31] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(43) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_clearbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[32] = 8;
		nIdentifiedNum_preProcess_a[32]=43;
	}
	else if(u5_blend_holdfrm_preProcess_a[32] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_clearbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[32]--;
	}
	else if(nIdentifiedNum_preProcess_a[32] == 43 )
	{
		rtd_setbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_setbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		nIdentifiedNum_preProcess_a[32] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(44) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
		u5_blend_holdfrm_preProcess_a[33] = 8;
		nIdentifiedNum_preProcess_a[33]=44;
	}
	else if(u5_blend_holdfrm_preProcess_a[33] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
		u5_blend_holdfrm_preProcess_a[33]--;
	}
	else if(nIdentifiedNum_preProcess_a[33] == 44 )
	{
		nIdentifiedNum_preProcess_a[33] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(45) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[34] = 8;
		nIdentifiedNum_preProcess_a[34]=45;
	}
	else if(u5_blend_holdfrm_preProcess_a[34] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[34]--;
	}
	else if(nIdentifiedNum_preProcess_a[34] == 45 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[34] = 0;
	}
	#endif
	//==================================
	//======================
	#if Pro_SONY
	if(scalerVIP_Get_MEMCPatternFlag_Identification(46) == TRUE  && s_pContext->_output_me_sceneAnalysis.u2_panning_flag)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_dh_close.u1_closeDh_act =0;
		u5_blend_holdfrm_preProcess_a[35] = 16;
		nIdentifiedNum_preProcess_a[35]=46;
	}
	else if(u5_blend_holdfrm_preProcess_a[35] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_dh_close.u1_closeDh_act =0;
		u5_blend_holdfrm_preProcess_a[35]--;
	}
	else if(nIdentifiedNum_preProcess_a[35] == 46 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[35] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(47) == TRUE )
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_adptpnt_rnd_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=3;
		u5_blend_holdfrm_preProcess_a[36] = 16;
		nIdentifiedNum_preProcess_a[36]=47;
	}
	else if(u5_blend_holdfrm_preProcess_a[36] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_adptpnt_rnd_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=3;
		u5_blend_holdfrm_preProcess_a[36]--;
	}
	else if(nIdentifiedNum_preProcess_a[36] == 47 )
	{
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true =0;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_adptpnt_rnd_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=0;
		nIdentifiedNum_preProcess_a[36] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(48) == TRUE  )
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=2;
		s_pContext->_output_wrt_comreg.u1_ME1_adptpnt_rnd_gain_true=1;
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=2;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=1;

		u5_blend_holdfrm_preProcess_a[37] = 24;
		nIdentifiedNum_preProcess_a[37]=48;
	}
	else if(u5_blend_holdfrm_preProcess_a[37] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=2;
		s_pContext->_output_wrt_comreg.u1_ME1_adptpnt_rnd_gain_true=1;
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=2;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=1;

		u5_blend_holdfrm_preProcess_a[37]--;
	}
	else if(nIdentifiedNum_preProcess_a[37] == 48 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=0;
		s_pContext->_output_wrt_comreg.u1_ME1_adptpnt_rnd_gain_true=0;
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=0;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=0;
		nIdentifiedNum_preProcess_a[37] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(49) == TRUE  )
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=2;
		s_pContext->_output_wrt_comreg.u1_ME1_adptpnt_rnd_gain_true=1;
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=2;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=1;

		u5_blend_holdfrm_preProcess_a[38] = 24;
		nIdentifiedNum_preProcess_a[38]=49;
	}
	else if(u5_blend_holdfrm_preProcess_a[38] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=2;
		s_pContext->_output_wrt_comreg.u1_ME1_adptpnt_rnd_gain_true=1;
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=2;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=1;

		u5_blend_holdfrm_preProcess_a[38]--;
	}
	else if(nIdentifiedNum_preProcess_a[38] == 49 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=0;
		s_pContext->_output_wrt_comreg.u1_ME1_adptpnt_rnd_gain_true=0;
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=0;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=0;
		nIdentifiedNum_preProcess_a[38] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(50)== TRUE )
	{
		s_pContext->_output_wrt_comreg.u1_FILM_force_mix_true=1;
		u5_blend_holdfrm_preProcess_a[39] = 24;
		nIdentifiedNum_preProcess_a[39]=50;
	}
	else if(u5_blend_holdfrm_preProcess_a[39] >0)
	{
		s_pContext->_output_wrt_comreg.u1_FILM_force_mix_true=1;
		u5_blend_holdfrm_preProcess_a[39]--;
	}
	else if(nIdentifiedNum_preProcess_a[39] == 50 )
	{
		s_pContext->_output_wrt_comreg.u1_FILM_force_mix_true=0;
		nIdentifiedNum_preProcess_a[39] = 0;
	}
	#endif
	#if Pro_TCL
	if(scalerVIP_Get_MEMCPatternFlag_Identification(51) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[40] = 16;
		nIdentifiedNum_preProcess_a[40]=51;
	}
	else if(u5_blend_holdfrm_preProcess_a[40] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[40]--;
	}
	else if(nIdentifiedNum_preProcess_a[40] == 51 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=0;
		nIdentifiedNum_preProcess_a[40] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(52) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=3;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Fast_Convergence_type =1;
		s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVx_Step=0;
		s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVy_Step=0;
		u5_blend_holdfrm_preProcess_a[41] = 24;
		nIdentifiedNum_preProcess_a[41]=52;
	}
	else if(u5_blend_holdfrm_preProcess_a[41] >0)
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=3;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Fast_Convergence_type =1;
		s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVx_Step=0;
		s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVy_Step=0;
		u5_blend_holdfrm_preProcess_a[41]--;
	}
	else if(nIdentifiedNum_preProcess_a[41] == 52 )
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Fast_Convergence_type =0;
		nIdentifiedNum_preProcess_a[41] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(53) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Fast_Convergence_type =2;
		s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_KME_SR_increaseY_true=1;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Fast_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[42] = 24;
		nIdentifiedNum_preProcess_a[42]=53;
	}
	else if(u5_blend_holdfrm_preProcess_a[42] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Fast_Convergence_type =2;
		s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_KME_SR_increaseY_true=1;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Fast_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[42]--;
	}
	else if(nIdentifiedNum_preProcess_a[42] == 53 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Fast_Convergence_type =0;
		s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=0;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=0;
		s_pContext->_output_wrt_comreg.u1_KME_SR_increaseY_true=0;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type =0;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Fast_Convergence_true=0;
		nIdentifiedNum_preProcess_a[42] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(54) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[43] = 8;
		nIdentifiedNum_preProcess_a[43]=54;
	}
	else if(u5_blend_holdfrm_preProcess_a[43] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[43]--;
	}
	else if(nIdentifiedNum_preProcess_a[43] == 54 )
	{
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		nIdentifiedNum_preProcess_a[43] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(55) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_dh_close.u1_dh_sml_rgnAction_close=1;
		s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
		u5_blend_holdfrm_preProcess_a[44] = 8;
		nIdentifiedNum_preProcess_a[44]=55;
	}
	else if(u5_blend_holdfrm_preProcess_a[44] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_dh_close.u1_dh_sml_rgnAction_close=1;
		s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
		u5_blend_holdfrm_preProcess_a[44]--;
	}
	else if(nIdentifiedNum_preProcess_a[44] == 55 )
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=0;
		nIdentifiedNum_preProcess_a[44] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(56) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=3;
		s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
		u5_blend_holdfrm_preProcess_a[45] = 8;
		nIdentifiedNum_preProcess_a[45]=56;
	}
	else if(u5_blend_holdfrm_preProcess_a[45] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=3;
		s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
		u5_blend_holdfrm_preProcess_a[45]--;
	}
	else if(nIdentifiedNum_preProcess_a[45] == 56 )
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=0;
		s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=0;
		nIdentifiedNum_preProcess_a[45] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(57) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_dh_close.u1_closeDh_act =0;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=3;
		u5_blend_holdfrm_preProcess_a[46] = 8;
		nIdentifiedNum_preProcess_a[46]=57;
	}
	else if(u5_blend_holdfrm_preProcess_a[46] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =0;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=3;
		u5_blend_holdfrm_preProcess_a[46]--;
	}
	else if(nIdentifiedNum_preProcess_a[46] == 57 )
	{
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=0;
		nIdentifiedNum_preProcess_a[46] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(58) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=1;
		u5_blend_holdfrm_preProcess_a[47] = 8;
		nIdentifiedNum_preProcess_a[47]=58;
	}
	else if(u5_blend_holdfrm_preProcess_a[47] >0)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=1;
		u5_blend_holdfrm_preProcess_a[47]--;
	}
	else if(nIdentifiedNum_preProcess_a[47] == 58 )
	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=0;
		nIdentifiedNum_preProcess_a[47] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(59) == TRUE  &&(u25_me_aDTL >=3800000)&&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_dh_close.u1_closeDh_act =0;
		pOutput->u1_casino_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[48] = 8;
		nIdentifiedNum_preProcess_a[48]=59;
	}
	else if(u5_blend_holdfrm_preProcess_a[48] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_dh_close.u1_closeDh_act =0;
		pOutput->u1_casino_RP_detect_true =1;
		u5_blend_holdfrm_preProcess_a[48]--;
	}
	else if(nIdentifiedNum_preProcess_a[48] == 59 )
	{
		pOutput->u1_casino_RP_detect_true =0;
		nIdentifiedNum_preProcess_a[48] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(4) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		rtd_clearbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_clearbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;

		u5_blend_holdfrm_preProcess_a[49] = 20;
		nIdentifiedNum_preProcess_a[49]=4;
	}
	else if(u5_blend_holdfrm_preProcess_a[49] >0)
	{
		rtd_clearbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_clearbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;

		u5_blend_holdfrm_preProcess_a[49]--;
	}
	else if(nIdentifiedNum_preProcess_a[49] == 4 )
	{
		rtd_setbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_setbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply

		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=0;

		nIdentifiedNum_preProcess_a[49] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(60) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		rtd_clearbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_clearbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;

		u5_blend_holdfrm_preProcess_a[50] = 20;
		nIdentifiedNum_preProcess_a[50]=60;
	}
	else if(u5_blend_holdfrm_preProcess_a[50] >0)
	{
		rtd_clearbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_clearbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;

		u5_blend_holdfrm_preProcess_a[50]--;
	}
	else if(nIdentifiedNum_preProcess_a[50] == 60 )
	{
		rtd_setbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_setbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=0;

		nIdentifiedNum_preProcess_a[50] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(61) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		rtd_clearbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_clearbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;

		u5_blend_holdfrm_preProcess_a[51] = 20;
		nIdentifiedNum_preProcess_a[51]=61;
	}
	else if(u5_blend_holdfrm_preProcess_a[51] >0)
	{
		rtd_clearbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_clearbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;

		u5_blend_holdfrm_preProcess_a[51]--;
	}
	else if(nIdentifiedNum_preProcess_a[51] == 61 )
	{
		rtd_setbits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, _BIT0);//  sharpness peaking
		rtd_setbits(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, _BIT0);//  sharpness dlti
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=0;
		nIdentifiedNum_preProcess_a[51] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(62) == TRUE &&(u11_gmv_mvx >=-58 && u11_gmv_mvx <=-36)&&(u10_gmv_mvy >=-72 && u10_gmv_mvy <=-33)  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_dh_close.u1_dh_sml_rgnAction_close=1;
		if(u32_RB_kmc_blend_y_alpha >0x70)
			WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, (u32_RB_kmc_blend_y_alpha-3));
		WriteRegister(IPPRE_IPPRE_04_reg, 16, 23, 0xa0);
		WriteRegister(IPPRE_IPPRE_04_reg, 24, 25, 0x1);
		u5_blend_holdfrm_preProcess_a[52] = 8;
		nIdentifiedNum_preProcess_a[52]=62;
	}
	else if(u5_blend_holdfrm_preProcess_a[52] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_dh_close.u1_dh_sml_rgnAction_close=1;
		if(u32_RB_kmc_blend_y_alpha >0x70)
			WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, (u32_RB_kmc_blend_y_alpha-3));
		WriteRegister(IPPRE_IPPRE_04_reg, 16, 23, 0xa0);
		WriteRegister(IPPRE_IPPRE_04_reg, 24, 25, 0x1);
		u5_blend_holdfrm_preProcess_a[52]--;
	}
	else if(nIdentifiedNum_preProcess_a[52] == 62 && (u32_RB_kmc_blend_y_alpha !=s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha) )
	{
		if(u32_RB_kmc_blend_y_alpha <(s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha))
		{
			if((s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha) -u32_RB_kmc_blend_y_alpha >1)
				WriteRegister(IPPRE_IPPRE_04_reg, 4, 11,(u32_RB_kmc_blend_y_alpha+1));
			else
				WriteRegister(IPPRE_IPPRE_04_reg, 4, 11,s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha);
		}
		s_pContext->_output_dh_close.u1_dh_sml_rgnAction_close=0;
		WriteRegister(IPPRE_IPPRE_04_reg, 16, 23, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_uv_alpha);
		WriteRegister(IPPRE_IPPRE_04_reg, 24, 25, 0x2);
	}
	else if(nIdentifiedNum_preProcess_a[52] == 62)
	{
		nIdentifiedNum_preProcess_a[52] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(63) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		u5_blend_holdfrm_preProcess_a[53] = 8;
		nIdentifiedNum_preProcess_a[53]=63;
	}
	else if(u5_blend_holdfrm_preProcess_a[53] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		u5_blend_holdfrm_preProcess_a[53]--;
	}
	else if(nIdentifiedNum_preProcess_a[53] == 63 )
	{
		nIdentifiedNum_preProcess_a[53] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(64) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_dh_close.u1_dh_sml_rgnAction_close=1;
		u5_blend_holdfrm_preProcess_a[54] = 16;
		nIdentifiedNum_preProcess_a[54]=64;
	}
	else if(u5_blend_holdfrm_preProcess_a[54] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_dh_close.u1_dh_sml_rgnAction_close=1;
		u5_blend_holdfrm_preProcess_a[54]--;
	}
	else if(nIdentifiedNum_preProcess_a[54] == 64 )
	{
		nIdentifiedNum_preProcess_a[54] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(65) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;
		u5_blend_holdfrm_preProcess_a[55] = 16;
		nIdentifiedNum_preProcess_a[55]=65;
	}
	else if(u5_blend_holdfrm_preProcess_a[55] >0)
	{
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;
		u5_blend_holdfrm_preProcess_a[55]--;
	}
	else if(nIdentifiedNum_preProcess_a[55] == 65 )
	{
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=0;
		nIdentifiedNum_preProcess_a[55] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(66) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		u5_blend_holdfrm_preProcess_a[56] = 8;
		nIdentifiedNum_preProcess_a[56]=66;
	}
	else if(u5_blend_holdfrm_preProcess_a[56] >0)
	{
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		u5_blend_holdfrm_preProcess_a[56]--;
	}
	else if(nIdentifiedNum_preProcess_a[56] == 66 )
	{
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=0;
		nIdentifiedNum_preProcess_a[56] = 0;
	}
	#endif
	#if Pro_SONY
	if(scalerVIP_Get_MEMCPatternFlag_Identification(67) == TRUE && scalerVIP_Get_MEMCPatternFlag_Identification(49) == FALSE)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_KME_gmvd_cost_type=1;
		u5_blend_holdfrm_preProcess_a[57] = 24;
		nIdentifiedNum_preProcess_a[57]=67;
	}
	else if(u5_blend_holdfrm_preProcess_a[57] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_KME_gmvd_cost_type=1;
		u5_blend_holdfrm_preProcess_a[57]--;
	}
	//else if(nIdentifiedNum_preProcess_a[57] == 67&&(scalerVIP_Get_MEMCPatternFlag_Identification(48) == TRUE || scalerVIP_Get_MEMCPatternFlag_Identification(49) == TRUE ) )
	//{
	//	s_pContext->_output_wrt_comreg.u1_KME_gmvd_cost_type=0;
	//	nIdentifiedNum_preProcess_a[57] = 0;
	//}
	else if(nIdentifiedNum_preProcess_a[57] == 67 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_KME_gmvd_cost_type=0;
		nIdentifiedNum_preProcess_a[57] = 0;
	}
	#endif
	#if Pro_TCL
	if(scalerVIP_Get_MEMCPatternFlag_Identification(68) == TRUE && s_pContext->_output_me_sceneAnalysis.u2_panning_flag)
	{
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=2;
		WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, 0xb0);
		u5_blend_holdfrm_preProcess_a[58] = 8;
		nIdentifiedNum_preProcess_a[58]=68;
	}
	else if(u5_blend_holdfrm_preProcess_a[58] >0)
	{
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=2;
		WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, 0xb0);
		u5_blend_holdfrm_preProcess_a[58]--;
	}
	else if(nIdentifiedNum_preProcess_a[58] == 68 && (u32_RB_kmc_blend_y_alpha !=s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha) )
	{
		if(u32_RB_kmc_blend_y_alpha <(s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha))
		{
			if((s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha) -u32_RB_kmc_blend_y_alpha >1)
				WriteRegister(IPPRE_IPPRE_04_reg, 4, 11,(u32_RB_kmc_blend_y_alpha+1));
			else
				WriteRegister(IPPRE_IPPRE_04_reg, 4, 11,s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha);
		}
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=0;
	}
	else if(nIdentifiedNum_preProcess_a[58] == 68 )
	{
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=0;
		nIdentifiedNum_preProcess_a[58] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(69) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=1;
		u5_blend_holdfrm_preProcess_a[59] = 8;
		nIdentifiedNum_preProcess_a[59]=69;
	}
	else if(u5_blend_holdfrm_preProcess_a[59] >0)
	{
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=1;
		u5_blend_holdfrm_preProcess_a[59]--;
	}
	else if(nIdentifiedNum_preProcess_a[59] == 69 )
	{
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=0;
		nIdentifiedNum_preProcess_a[59] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(70) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[60] = 8;
		nIdentifiedNum_preProcess_a[60]=70;
	}
	else if(u5_blend_holdfrm_preProcess_a[60] >0)
	{
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[60]--;
	}
	else if(nIdentifiedNum_preProcess_a[60] == 70 )
	{
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		nIdentifiedNum_preProcess_a[60] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(71) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
		u5_blend_holdfrm_preProcess_a[61] = 8;
		nIdentifiedNum_preProcess_a[61]=71;
	}
	else if(u5_blend_holdfrm_preProcess_a[61] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
		u5_blend_holdfrm_preProcess_a[61]--;
	}
	else if(nIdentifiedNum_preProcess_a[61] == 71 )
	{
		nIdentifiedNum_preProcess_a[61] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(72) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
		u5_blend_holdfrm_preProcess_a[62] = 8;
		nIdentifiedNum_preProcess_a[62]=72;
	}
	else if(u5_blend_holdfrm_preProcess_a[62] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =0;
		u5_blend_holdfrm_preProcess_a[62]--;
	}
	else if(nIdentifiedNum_preProcess_a[62] == 72 )
	{
		nIdentifiedNum_preProcess_a[62] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(73) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[63] = 8;
		nIdentifiedNum_preProcess_a[63]=73;
	}
	else if(u5_blend_holdfrm_preProcess_a[63] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_preProcess_a[63]--;
	}
	else if(nIdentifiedNum_preProcess_a[63] == 73 )
	{
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		nIdentifiedNum_preProcess_a[63] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(74) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[64] = 8;
		nIdentifiedNum_preProcess_a[64]=74;
	}
	else if(u5_blend_holdfrm_preProcess_a[64] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[64]--;
	}
	else if(nIdentifiedNum_preProcess_a[64] == 74 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[64] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(75) == TRUE &&  (u11_gmv_mvx >=40 && u11_gmv_mvx <=50)&&(u10_gmv_mvy >=-61 && u10_gmv_mvy <=-31)&&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		if(u32_RB_kmc_blend_y_alpha >0x70)
			WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, (u32_RB_kmc_blend_y_alpha-3));
		WriteRegister(IPPRE_IPPRE_04_reg, 16, 23, 0x80);
		WriteRegister(IPPRE_IPPRE_04_reg, 24, 25, 0x1);

		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[65] = 8;
		nIdentifiedNum_preProcess_a[65]=75;
	}
	else if(u5_blend_holdfrm_preProcess_a[65] >0)
	{
		if(u32_RB_kmc_blend_y_alpha >0x70)
			WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, (u32_RB_kmc_blend_y_alpha-3));
		WriteRegister(IPPRE_IPPRE_04_reg, 16, 23, 0x80);
		WriteRegister(IPPRE_IPPRE_04_reg, 24, 25, 0x1);

		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[65]--;
	}
	else if(nIdentifiedNum_preProcess_a[65] == 75 && (u32_RB_kmc_blend_y_alpha !=s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha) )
	{
		if(u32_RB_kmc_blend_y_alpha <(s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha))
		{
			if((s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha) -u32_RB_kmc_blend_y_alpha >1)
				WriteRegister(IPPRE_IPPRE_04_reg, 4, 11,(u32_RB_kmc_blend_y_alpha+1));
			else
				WriteRegister(IPPRE_IPPRE_04_reg, 4, 11,s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha);
		}
		WriteRegister(IPPRE_IPPRE_04_reg, 16, 23, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_uv_alpha);
		WriteRegister(IPPRE_IPPRE_04_reg, 24, 25, 0x2);
	}
	else if(nIdentifiedNum_preProcess_a[65] == 75 )
	{
		nIdentifiedNum_preProcess_a[65] = 0;
	}

	if(scalerVIP_Get_MEMCPatternFlag_Identification(76) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_dh_close.u1_dh_sml_rgnAction_close=1;
		u5_blend_holdfrm_preProcess_a[66] = 16;
		nIdentifiedNum_preProcess_a[66]=76;
	}
	else if(u5_blend_holdfrm_preProcess_a[66] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_dh_close.u1_dh_sml_rgnAction_close=1;
		u5_blend_holdfrm_preProcess_a[66]--;
	}
	else if(nIdentifiedNum_preProcess_a[66] == 76 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[66] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(77) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		u5_blend_holdfrm_preProcess_a[67] = 8;
		nIdentifiedNum_preProcess_a[67]=77;
	}
	else if(u5_blend_holdfrm_preProcess_a[67] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		u5_blend_holdfrm_preProcess_a[67]--;
	}
	else if(nIdentifiedNum_preProcess_a[67] == 77 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[67] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(78) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=2;
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=1;
		s_pContext->_output_wrt_comreg.u1_ME1_DRP_type = 1;
		u5_blend_holdfrm_preProcess_a[68] = 8;
		nIdentifiedNum_preProcess_a[68]=78;
	}
	else if(u5_blend_holdfrm_preProcess_a[68] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=2;
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=1;
		s_pContext->_output_wrt_comreg.u1_ME1_DRP_type = 1;
		u5_blend_holdfrm_preProcess_a[68]--;
	}
	else if(nIdentifiedNum_preProcess_a[68] == 78 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=0;
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=0;
		s_pContext->_output_wrt_comreg.u1_ME1_DRP_type = 0;
		nIdentifiedNum_preProcess_a[68] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(79) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=2;
		u5_blend_holdfrm_preProcess_a[69] = 8;
		nIdentifiedNum_preProcess_a[69]=79;
	}
	else if(u5_blend_holdfrm_preProcess_a[69] >0)
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=2;
		u5_blend_holdfrm_preProcess_a[69]--;
	}
	else if(nIdentifiedNum_preProcess_a[69] == 79 )
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=0;
		nIdentifiedNum_preProcess_a[69] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(80) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=2;
		u5_blend_holdfrm_preProcess_a[70] = 8;
		nIdentifiedNum_preProcess_a[70]=80;
	}
	else if(u5_blend_holdfrm_preProcess_a[70] >0)
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=2;
		u5_blend_holdfrm_preProcess_a[70]--;
	}
	else if(nIdentifiedNum_preProcess_a[70] == 80 )
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=0;
		nIdentifiedNum_preProcess_a[70] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(81) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=2;
		if(u32_RB_kmc_blend_y_alpha >0xb0)
			WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, (u32_RB_kmc_blend_y_alpha-3));
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[71] = 8;
		nIdentifiedNum_preProcess_a[71]=81;
	}
	else if(u5_blend_holdfrm_preProcess_a[71] >0)
	{
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=2;
		if(u32_RB_kmc_blend_y_alpha >0xb0)
			WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, (u32_RB_kmc_blend_y_alpha-3));
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[71]--;
	}
	else if(nIdentifiedNum_preProcess_a[71] == 81 && (u32_RB_kmc_blend_y_alpha !=s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha) )
	{
		if(u32_RB_kmc_blend_y_alpha <(s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha))
		{
			if((s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha) -u32_RB_kmc_blend_y_alpha >1)
				WriteRegister(IPPRE_IPPRE_04_reg, 4, 11,(u32_RB_kmc_blend_y_alpha+1));
			else
				WriteRegister(IPPRE_IPPRE_04_reg, 4, 11,s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha);
		}
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=0;
	}
	else if(nIdentifiedNum_preProcess_a[71] == 81 )
	{
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=0;
		nIdentifiedNum_preProcess_a[71] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(82) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[72] = 8;
		nIdentifiedNum_preProcess_a[72]=82;
	}
	else if(u5_blend_holdfrm_preProcess_a[72] >0)
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[72]--;
	}
	else if(nIdentifiedNum_preProcess_a[72] == 82 )
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[72] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(83) == TRUE   &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type=1;
		s_pContext->_output_wrt_comreg.u1_GFB_modify_true=0;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=1;
		u5_blend_holdfrm_preProcess_a[73] = 8;
		nIdentifiedNum_preProcess_a[73]=83;
	}
	else if(u5_blend_holdfrm_preProcess_a[73] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type=1;
		s_pContext->_output_wrt_comreg.u1_GFB_modify_true=0;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=1;
		u5_blend_holdfrm_preProcess_a[73]--;
	}
	else if(nIdentifiedNum_preProcess_a[73] == 83 && pOutput->u8_FB_lvl>0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type=0;
		s_pContext->_output_wrt_comreg.u1_GFB_modify_true=1;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=0;
		pOutput->u8_FB_lvl --;
	}
	else if(nIdentifiedNum_preProcess_a[73] == 83 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type=0;
		s_pContext->_output_wrt_comreg.u1_GFB_modify_true=0;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=0;
		nIdentifiedNum_preProcess_a[73] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(84) == TRUE  && (u11_gmv_mvx >=190 && u11_gmv_mvx <=310)&&(u10_gmv_mvy >=-7 && u10_gmv_mvy <=7)&&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		if(pOutput->u8_FB_lvl<0x60)
		{
			pOutput->u8_FB_lvl ++;
			s_pContext->_output_wrt_comreg.u1_GFB_modify_true=1;
		}
		else if( s_pContext->_output_fblevelctrl.u8_FBLevel >0x60)
		{
			s_pContext->_output_wrt_comreg.u1_GFB_modify_true=0;
		}
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=1;
		u5_blend_holdfrm_preProcess_a[74] = 16;
		nIdentifiedNum_preProcess_a[74]=84;
	}
	else if(u5_blend_holdfrm_preProcess_a[74] >0)
	{
		if(pOutput->u8_FB_lvl<0x60)
		{
			pOutput->u8_FB_lvl ++;
			s_pContext->_output_wrt_comreg.u1_GFB_modify_true=1;
		}
		else if( s_pContext->_output_fblevelctrl.u8_FBLevel >0x60)
		{
			s_pContext->_output_wrt_comreg.u1_GFB_modify_true=0;
		}
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=1;
		u5_blend_holdfrm_preProcess_a[74]--;
	}
	else if(nIdentifiedNum_preProcess_a[74] == 84 && pOutput->u8_FB_lvl>0)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_modify_true=1;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=1;
		pOutput->u8_FB_lvl --;
	}
	else if(nIdentifiedNum_preProcess_a[74] == 84 )
	{
		s_pContext->_output_wrt_comreg.u1_GFB_modify_true=0;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=0;
		nIdentifiedNum_preProcess_a[74] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(85) == TRUE   && (u11_gmv_mvx >=190 && u11_gmv_mvx <=310)&&(u10_gmv_mvy >=-7 && u10_gmv_mvy <=7)&&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		if(pOutput->u8_FB_lvl<0x60)
		{
			pOutput->u8_FB_lvl ++;
			s_pContext->_output_wrt_comreg.u1_GFB_modify_true=1;
		}
		else if( s_pContext->_output_fblevelctrl.u8_FBLevel >0x60)
		{
			s_pContext->_output_wrt_comreg.u1_GFB_modify_true=0;
		}
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=1;
		u5_blend_holdfrm_preProcess_a[75] = 16;
		nIdentifiedNum_preProcess_a[75]=85;
	}
	else if(u5_blend_holdfrm_preProcess_a[75] >0)
	{
		if(pOutput->u8_FB_lvl<0x60)
		{
			pOutput->u8_FB_lvl ++;
			s_pContext->_output_wrt_comreg.u1_GFB_modify_true=1;
		}
		else if( s_pContext->_output_fblevelctrl.u8_FBLevel >0x60)
		{
			s_pContext->_output_wrt_comreg.u1_GFB_modify_true=0;
		}
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=1;
		u5_blend_holdfrm_preProcess_a[75]--;
	}
	else if(nIdentifiedNum_preProcess_a[75] == 85 && pOutput->u8_FB_lvl>0)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_modify_true=1;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=1;
		pOutput->u8_FB_lvl --;
	}
	else if(nIdentifiedNum_preProcess_a[75] == 85 )
	{
		s_pContext->_output_wrt_comreg.u1_GFB_modify_true=0;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type=0;
		nIdentifiedNum_preProcess_a[75] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(86) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		u5_blend_holdfrm_preProcess_a[76] = 8;
		nIdentifiedNum_preProcess_a[76]=86;
	}
	else if(u5_blend_holdfrm_preProcess_a[76] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		u5_blend_holdfrm_preProcess_a[76]--;
	}
	else if(nIdentifiedNum_preProcess_a[76] == 86 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[76] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(87) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[77] = 8;
		nIdentifiedNum_preProcess_a[77]=87;
	}
	else if(u5_blend_holdfrm_preProcess_a[77] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		u5_blend_holdfrm_preProcess_a[77]--;
	}
	else if(nIdentifiedNum_preProcess_a[77] == 87 )
	{
		nIdentifiedNum_preProcess_a[77] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(88) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=4;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[78] = 16;
		nIdentifiedNum_preProcess_a[78]=88;
	}
	else if(u5_blend_holdfrm_preProcess_a[78] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=4;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[78]--;
	}
	else if(nIdentifiedNum_preProcess_a[78] == 88 )
	{
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[78] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(89) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		u5_blend_holdfrm_preProcess_a[79] = 8;
		nIdentifiedNum_preProcess_a[79]=89;
	}
	else if(u5_blend_holdfrm_preProcess_a[79] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		u5_blend_holdfrm_preProcess_a[79]--;
	}
	else if(nIdentifiedNum_preProcess_a[79] == 89 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[79] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(90) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		u5_blend_holdfrm_preProcess_a[80] = 8;
		nIdentifiedNum_preProcess_a[80]=90;
	}
	else if(u5_blend_holdfrm_preProcess_a[80] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		u5_blend_holdfrm_preProcess_a[80]--;
	}
	else if(nIdentifiedNum_preProcess_a[80] == 90 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[80] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(91) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_DRP_type = 1;
		u5_blend_holdfrm_preProcess_a[81] = 8;
		nIdentifiedNum_preProcess_a[81]=91;
	}
	else if(u5_blend_holdfrm_preProcess_a[81] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_DRP_type = 1;
		u5_blend_holdfrm_preProcess_a[81]--;
	}
	else if(nIdentifiedNum_preProcess_a[81] == 91 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_DRP_type = 0;
		nIdentifiedNum_preProcess_a[81] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(92) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;
		u5_blend_holdfrm_preProcess_a[82] = 8;
		nIdentifiedNum_preProcess_a[82]=92;
	}
	else if(u5_blend_holdfrm_preProcess_a[82] >0)
	{
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=1;
		u5_blend_holdfrm_preProcess_a[82]--;
	}
	else if(nIdentifiedNum_preProcess_a[82] == 92 )
	{
		s_pContext->_output_wrt_comreg.u1_MC_Logo_Close_true=0;
		nIdentifiedNum_preProcess_a[82] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(93) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		u5_blend_holdfrm_preProcess_a[83] = 8;
		nIdentifiedNum_preProcess_a[83]=93;
	}
	else if(u5_blend_holdfrm_preProcess_a[83] >0)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =4;
		u5_blend_holdfrm_preProcess_a[83]--;
	}
	else if(nIdentifiedNum_preProcess_a[83] == 93)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[83] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(94) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type =3;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[84] = 16;
		nIdentifiedNum_preProcess_a[84]=94;
	}
	else if(u5_blend_holdfrm_preProcess_a[84] >0)
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type =3;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[84]--;
	}
	else if(nIdentifiedNum_preProcess_a[84] == 94)
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type =0;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[84] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(95) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[85] = 16;
		nIdentifiedNum_preProcess_a[85]=95;
	}
	else if(u5_blend_holdfrm_preProcess_a[85] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[85]--;
	}
	else if(nIdentifiedNum_preProcess_a[85] == 95)
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type =0;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[85] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(96) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[86] = 16;
		nIdentifiedNum_preProcess_a[86]=96;
	}
	else if(u5_blend_holdfrm_preProcess_a[86] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[86]--;
	}
	else if(nIdentifiedNum_preProcess_a[86] == 96)
	{
		s_pContext->_output_wrt_comreg.u1_LFB_Dtl_weak_type =0;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[86] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(97) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[87] = 16;
		nIdentifiedNum_preProcess_a[87]=97;
	}
	else if(u5_blend_holdfrm_preProcess_a[87] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		u5_blend_holdfrm_preProcess_a[87]--;
	}
	else if(nIdentifiedNum_preProcess_a[87] == 97)
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		nIdentifiedNum_preProcess_a[87] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(98) == TRUE  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=3;
		u5_blend_holdfrm_preProcess_a[88] = 16;
		nIdentifiedNum_preProcess_a[88]=98;
	}
	else if(u5_blend_holdfrm_preProcess_a[88] >0)
	{
		s_pContext->_output_dh_close.u1_closeDh_act =1;
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=1;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=3;
		u5_blend_holdfrm_preProcess_a[88]--;
	}
	else if(nIdentifiedNum_preProcess_a[88] == 98)
	{
		s_pContext->_output_wrt_comreg.u1_LFB_force_zero_true=0;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=0;
		nIdentifiedNum_preProcess_a[88] = 0;
	}
	#endif
	#if Pro_SONY
	if(scalerVIP_Get_MEMCPatternFlag_Identification(99) == TRUE  )
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=2;
		s_pContext->_output_wrt_comreg.u1_ME1_adptpnt_rnd_gain_true=1;
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=2;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=1;

		u5_blend_holdfrm_preProcess_a[89] = 24;
		nIdentifiedNum_preProcess_a[89]=99;
	}
	else if(u5_blend_holdfrm_preProcess_a[89] >0)
	{
		s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true =1;
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=1;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=2;
		s_pContext->_output_wrt_comreg.u1_ME1_adptpnt_rnd_gain_true=1;
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=2;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=1;

		u5_blend_holdfrm_preProcess_a[89]--;
	}
	else if(nIdentifiedNum_preProcess_a[89] == 99 )
	{
		s_pContext->_output_wrt_comreg.u1_ME1_rnd_mask_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCanddpnt_Slow_Convergence_true=0;
		s_pContext->_output_wrt_comreg.u1_ME1_rndCand_Slow_Convergence_type=0;
		s_pContext->_output_wrt_comreg.u1_ME1_adptpnt_rnd_gain_true=0;
		s_pContext->_output_wrt_comreg.u1_IPME_increase_flp_type=0;
		s_pContext->_output_wrt_comreg.u1_ME2_SmallObject_type=0;
		nIdentifiedNum_preProcess_a[89] = 0;
	}
	#endif
	if(scalerVIP_Get_MEMCPatternFlag_Identification(102) == TRUE  )
	{
		s_pContext->_output_wrt_comreg.u1_GFB_Special_apply=1;
		u5_blend_holdfrm_preProcess_a[90] = 8;
		nIdentifiedNum_preProcess_a[90]=102;
	}
	else if(u5_blend_holdfrm_preProcess_a[90] >0)
	{
		s_pContext->_output_wrt_comreg.u1_GFB_Special_apply=1;
		u5_blend_holdfrm_preProcess_a[90]--;
	}
	else if(nIdentifiedNum_preProcess_a[90] == 102 )
	{
		s_pContext->_output_wrt_comreg.u1_GFB_Special_apply=0;
		nIdentifiedNum_preProcess_a[90] = 0;
	}

	//++ YE Test for TV006 01_Black_19YO1 issue
	#if 1
     if((scalerVIP_Get_MEMCPatternFlag_Identification(103) == TRUE  )&&(u11_gmv_mvx <=0 && u10_gmv_mvy <=2))
	{
		pOutput->u8_01_Black_19YO1_flag=1;
		u5_blend_holdfrm_preProcess_a[91] = 24;
		nIdentifiedNum_preProcess_a[91]=103;
	}
	else if(u5_blend_holdfrm_preProcess_a[91] >0)
	{
		pOutput->u8_01_Black_19YO1_flag=1;
		u5_blend_holdfrm_preProcess_a[91]--;
	}
	else if(nIdentifiedNum_preProcess_a[91] == 103 )
	{
             pOutput->u8_01_Black_19YO1_flag=0;
		nIdentifiedNum_preProcess_a[91] = 0;
	}
	#endif 
	//-- YE Test for LG 01_Black_19YO1 issue

	//++ Q-13722 issue
	#if 1
     if((scalerVIP_Get_MEMCPatternFlag_Identification(104) == TRUE  )&&(u11_gmv_mvx ==0 && u10_gmv_mvy ==0))
	{
//		pOutput->u1_Q13722_flag =1;
		g_Q13722_flag = 1;
		u5_blend_holdfrm_preProcess_a[92] = 60;
		nIdentifiedNum_preProcess_a[92]=104;
	}
	else if(u5_blend_holdfrm_preProcess_a[104] >0)
	{
//		pOutput->u1_Q13722_flag=1;
		g_Q13722_flag = 1;
		u5_blend_holdfrm_preProcess_a[92]--;
	}
	else if(nIdentifiedNum_preProcess_a[92] == 104 )
	{
 //		pOutput->u1_Q13722_flag=0;
		g_Q13722_flag = 0;
		nIdentifiedNum_preProcess_a[92] = 0;
	}
	#endif 
	//-- Q-13722 issue

	#if 1//YE Test it's new place
	// ++ YE Test Edge Shaking
	if((s_pContext->_output_bRMV_rFB_ctrl.u8_ES_439_ID_flag ==1)||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_413_ID_flag ==1)
		||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_421_ID_flag ==1)||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_380_ID_flag ==1)
		||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag==1)	||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_21_ID_flag==1))
	{
	      if (s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag==1)
	      	{
		s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
		/*
		if(u32_ID_Log_en==1){
				rtd_pr_memc_info( "=====AAA GFB   ,weak=%d   ,custom=%d, MAX=%d  \n",s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type,
				s_pContext->_output_wrt_comreg.u1_GFB_force_Custom_true,s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true);
			}
		*/

	      	}
		else if((s_pContext->_output_bRMV_rFB_ctrl.u8_ES_413_ID_flag ==1)||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_421_ID_flag ==1)
			||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_380_ID_flag ==1)||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_21_ID_flag==1))
		{
			s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
			//s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type=1;
			/*
			if(u32_ID_Log_en==1)
			rtd_pr_memc_info( "=====BBB GFB   ,weak=%d   ,custom=%d, MAX=%d  \n",s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type,
			s_pContext->_output_wrt_comreg.u1_GFB_force_Custom_true,s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true);
			*/

		}
		else if (s_pContext->_output_bRMV_rFB_ctrl.u8_ES_439_ID_flag ==1)
		{
			s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
			//s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type=3;
			/*
			if(u32_ID_Log_en==1)
			rtd_pr_memc_info( "=====DDD GFB   ,weak=%d   ,custom=%d, MAX=%d  \n",s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type,
			s_pContext->_output_wrt_comreg.u1_GFB_force_Custom_true,s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true);
			*/

		}

		u5_blend_holdfrm_preProcess_a[93] = 24;
		nIdentifiedNum_preProcess_a[93]=109;
	}
	else if(u5_blend_holdfrm_preProcess_a[93] >0)
	{

	      if (s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag==1)
	      	{
			s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
			/*
			if(u32_ID_Log_en==1)
			rtd_pr_memc_info( "=====AAA GFB   ,weak=%d   ,custom=%d, MAX=%d  \n",s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type=1,
			s_pContext->_output_wrt_comreg.u1_GFB_force_Custom_true,s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true);
			*/

	      	}
		else if((s_pContext->_output_bRMV_rFB_ctrl.u8_ES_413_ID_flag ==1)||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_421_ID_flag ==1)
			||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_380_ID_flag ==1)||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_21_ID_flag==1))
		{
			s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
			//s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type=1;
			/*
			if(u32_ID_Log_en==1)
			rtd_pr_memc_info( "=====BBB GFB   ,weak=%d   ,custom=%d, MAX=%d  \n",s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type,
			s_pContext->_output_wrt_comreg.u1_GFB_force_Custom_true,s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true);
			*/

		}
		else if (s_pContext->_output_bRMV_rFB_ctrl.u8_ES_439_ID_flag ==1)
		{
			s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=1;
			//s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type=3;
			/*
			if(u32_ID_Log_en==1)
			rtd_pr_memc_info( "=====DDD GFB   ,weak=%d   ,custom=%d, MAX=%d  \n",s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type,
			s_pContext->_output_wrt_comreg.u1_GFB_force_Custom_true,s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true);
			*/

		}

		u5_blend_holdfrm_preProcess_a[93]--;
	}
	else if(nIdentifiedNum_preProcess_a[93] == 109 )  // YE Test // it must be updated
	{

		if((s_pContext->_output_bRMV_rFB_ctrl.u8_ES_439_ID_flag ==0)||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_413_ID_flag ==0)
		||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_421_ID_flag ==0)||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_380_ID_flag ==0)
		||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag==0)	||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_21_ID_flag==0))
			{
				s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type=0;
				s_pContext->_output_wrt_comreg.u1_GFB_force_zero_true=0;
				s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true=0;
				s_pContext->_output_wrt_comreg.u1_GFB_force_Custom_true=0;
				pOutput->u8_FB_lvl=0;
				s_pContext->_output_wrt_comreg.u1_GFB_modify_true=0;
			}
			//if(u32_ID_Log_en==1)
			//rtd_pr_memc_info( "=====EEE GFB   ,weak=%d   ,custom=%d, MAX=%d  \n",s_pContext->_output_wrt_comreg.u1_GFB_force_weak_type,
			//s_pContext->_output_wrt_comreg.u1_GFB_force_Custom_true,s_pContext->_output_wrt_comreg.u1_GFB_force_MAX_true);


		nIdentifiedNum_preProcess_a[93] = 0;
	}
	// -- YE Test Edge Shaking
	#endif

	#if 1
	// ++ YE Test Rassia Fields
//     if(scalerVIP_Get_MEMCPatternFlag_Identification(111) == TRUE  )
     if((scalerVIP_Get_MEMCPatternFlag_Identification(111) == TRUE  )&&(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_Rassia_ID_flag ==1))
	{

		s_pContext->_output_wrt_comreg.u1_GFB_force_Custom_true=1;

		u5_blend_holdfrm_preProcess_a[94] = 24;
		nIdentifiedNum_preProcess_a[94]=111;
	}
	else if(u5_blend_holdfrm_preProcess_a[94] >0)
	{

		s_pContext->_output_wrt_comreg.u1_GFB_force_Custom_true=1;

		u5_blend_holdfrm_preProcess_a[94]--;
	}
	else if(nIdentifiedNum_preProcess_a[94] == 111 )
	{

			if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_Rassia_ID_flag ==0)
			{
			s_pContext->_output_wrt_comreg.u1_GFB_force_Custom_true=0;
			pOutput->u8_FB_lvl=0;
			s_pContext->_output_wrt_comreg.u1_GFB_modify_true=0;
			}
		nIdentifiedNum_preProcess_a[94] = 0;
	}
	// -- YE Test Edge Shaking
	#endif

// -- YE Test Rassia Fields
	// ++  merlin6 new algo for #pattern214

	if((scalerVIP_Get_MEMCPatternFlag_Identification(120) == TRUE)&&(u11_gmv_mvx>=25)&&(u11_gmv_mvx<=80)) //&&( u25_me_aDTL >= 656575 )&&(u25_me_aDTL<=1221649)&&(u11_gmv_mvx>=25)&&(u11_gmv_mvx<=80)&&(u10_gmv_mvy<=5)&&(u10_gmv_mvy>=-11)
	{
	// if(u32_pattern214_forbindden_en == 1){
			//++solution settings:
			//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 4, 7, 0x1);
#if 0 // merlin7_bring_up_dehalo
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 29 , 29 , 1 ); // new algo switch
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 28 , 28 , 1 ); //UL
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 27 , 27 , 1 ); //UR
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 26 , 26 , 0 ); //DL
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 25 , 25 , 1 ); //DR
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 24 , 24 , 0 ); //APL sel
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 18 , 23 , 20 );//hi_APL_num_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 12 , 17 , 20 );//low_APL_num_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 6 , 11 ,  20 );//near_Gmv_num_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 0 , 5 ,  20 ); //far_Gmv_num_th

			WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 0 , 7 ,  20 ); //low_Gmv_diff_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 8 , 15 ,  40 ); //high_Gmv_diff_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 16 , 23 ,  40 ); //low_APL_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 24 , 31 ,  50 ); //high_APL_th

			WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 0 , 4 , 0 ); //gmv_num
			WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 6 , 7 , 1 ); //gmv_len
			WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 8 , 15 , 50 ); //set_occl_apl_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 16 , 23 , 50 ); //apl_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 24 , 31 , 2); //Gmv_diff_th

			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 0 , 0 , 1 ); //rtgmvd_bypass
			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 1 , 1 , 0 ); //gmv_bypass
			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 2 , 3 , 1 ); //case_mark_mode
			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 4 , 4 , 0 ); //result_apply
			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 5 , 5 , 0 );//pred_occl_en
			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 6 , 6 , 1 ); //occl_hor_en
			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 7 , 7 , 1 ); //occl_ver_en
			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 8 , 8 , 0 ); //debug mode

			WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 0 , 2 , 0 );  //res_num
			WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 3 , 4 , 0 );  //res_mode
			WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 5 , 9 , 2 );  //rim down
			WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 10 , 14 , 2 ); //rim up
			WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 15 , 19 , 0 );  //rim right
			WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 20 , 24 , 0 ); //rim left

			WriteRegister(KME_DEHALO4_RT2_MOD_1_reg , 0 , 0 , 0 ); //fix1 en
			WriteRegister(KME_DEHALO4_RT2_MOD_1_reg , 1 , 1 , 0 ); //fix1 en
			WriteRegister(KME_ME1_TOP10_APL_FILTER0_reg , 0 , 0 , 0 );
			//--solution settings:
#endif			
		//dehalo 214 djntest
        WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,28,29,0x1); //pick bg mv
        WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x1);  //penalty_en
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
        WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,0x9); //me1 pred th0
        WriteRegister(KME_DEHALO5_PHMV_FIX_1A_reg,26,31,0x0); //correct cut
        WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,10,13,0x0); //extend cut
        WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0xa); //extend ucov th0
        WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0xa);  //extend cov th0
        WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x19); // color protect th max0
        WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,2,2,0x1); // mv filter en
        WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x258); //posdiff th1
        WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,18,26,0xdf); //posdiff th2
        WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,16,24,0x1a4); //posdiff th3
        WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,0,6,0x57); //useless bg th
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
        WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23, 0x14);
        WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25, 0x3ff);
        WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,24,30, 0x64);
        WriteRegister(KME_DEHALO5_PHMV_FIX_10_reg,24,31, 0x3f);

        WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x0);
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x4);        
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x1);
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x4);  
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x1); 
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x4);        
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x1);
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x4);
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x1);
		
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

        WriteRegister(HARDWARE_HARDWARE_24_reg , 19, 28, 0x0);
        WriteRegister(HARDWARE_HARDWARE_24_reg , 31, 31, 0x0);    

        WriteRegister(HARDWARE_HARDWARE_26_reg,31,31, 0x0); //u8_dh_condition_dhbypass         
		//    rtd_pr_memc_info("in214");
	//	}

		u5_blend_holdfrm_preProcess_a[95] = 30;
		nIdentifiedNum_preProcess_a[95]=120;
	}
	else if(u5_blend_holdfrm_preProcess_a[95] >0){
		//if(u32_pattern214_forbindden_en == 1){
			//++solution settings:
			//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 4, 7, 0x2);
#if 0 // merlin7_bring_up_dehalo
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 29 , 29 , 1 ); // new algo switch
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 28 , 28 , 1 ); //UL
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 27 , 27 , 1 ); //UR
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 26 , 26 , 0 ); //DL
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 25 , 25 , 1 ); //DR
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 24 , 24 , 0 ); //APL sel
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 18 , 23 , 20 );//hi_APL_num_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 12 , 17 , 20 );//low_APL_num_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 6 , 11 ,  20 );//near_Gmv_num_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 0 , 5 ,  20 ); //far_Gmv_num_th

			WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 0 , 7 ,  20 ); //low_Gmv_diff_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 8 , 15 ,  40 ); //high_Gmv_diff_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 16 , 23 ,  40 ); //low_APL_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 24 , 31 ,  50 ); //high_APL_th

			WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 0 , 4 , 0 ); //gmv_num
			WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 6 , 7 , 1 ); //gmv_len
			WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 8 , 15 , 50 ); //set_occl_apl_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 16 , 23 , 50 ); //apl_th
			WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 24 , 31 , 2); //Gmv_diff_th

			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 0 , 0 , 1 ); //rtgmvd_bypass
			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 1 , 1 , 0 ); //gmv_bypass
			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 2 , 3 , 1 ); //case_mark_mode
			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 4 , 4 , 0 ); //result_apply
			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 5 , 5 , 0 );//pred_occl_en
			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 6 , 6 , 1 ); //occl_hor_en
			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 7 , 7 , 1 ); //occl_ver_en
			WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 8 , 8 , 0 ); //debug mode

			WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 0 , 2 , 0 );  //res_num
			WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 3 , 4 , 0 );  //res_mode
			WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 5 , 9 , 2 );  //rim down
			WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 10 , 14 , 2 ); //rim up
			WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 15 , 19 , 0 );  //rim right
			WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 20 , 24 , 0 ); //rim left

			WriteRegister(KME_DEHALO4_RT2_MOD_1_reg , 0 , 0 , 0 ); //fix1 en
			WriteRegister(KME_DEHALO4_RT2_MOD_1_reg , 1 , 1 , 0 ); //fix1 en
			WriteRegister(KME_ME1_TOP10_APL_FILTER0_reg , 0 , 0 , 0 );
			//--solution settings:
#endif			
		WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,28,29,0x1); //pick bg mv
        WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x1);  //penalty_en
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
        WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,0x9); //me1 pred th0
        WriteRegister(KME_DEHALO5_PHMV_FIX_1A_reg,26,31,0x0); //correct cut
        WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,10,13,0x0); //extend cut
        WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,16,22,0xa); //extend ucov th0
        WriteRegister(KME_DEHALO5_PHMV_FIX_1E_reg,0,6,0xa);  //extend cov th0
        WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x19); // color protect th max0
        WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,2,2,0x1); // mv filter en
        WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,0,11,0x258); //posdiff th1
        WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,18,26,0xdf); //posdiff th2
        WriteRegister(KME_DEHALO5_PHMV_FIX_3_reg,16,24,0x1a4); //posdiff th3
        WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,0,6,0x57); //useless bg th
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

		WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23, 0x14);
        WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25, 0x3ff);
        WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,24,30, 0x64);
        WriteRegister(KME_DEHALO5_PHMV_FIX_10_reg,24,31, 0x3f);

		WriteRegister(SOFTWARE1_SOFTWARE1_49_reg,31,31,0x0);
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,0,1,0x3);
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,0x4);        
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,0x1);
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11,0x4);  
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14,0x1); 
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x4);        
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,0x1);
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x4);
        WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29,0x1);

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
			
        WriteRegister(KME_DEHALO5_PHMV_FIX_4_reg,16,20,0x10); //filter cov th0
        WriteRegister(KME_DEHALO5_PHMV_FIX_7_reg,8,12,0x10); //filter ucov th0
        WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x1a); // color protect th max0
        
        WriteRegister(HARDWARE_HARDWARE_24_reg , 19, 28, 0);
        WriteRegister(HARDWARE_HARDWARE_24_reg , 31, 31, 0);         
         //      rtd_pr_memc_info("in214 hold\n");    
        WriteRegister(HARDWARE_HARDWARE_26_reg,31,31, 0x0); //u8_dh_condition_dhbypass

        u5_blend_holdfrm_preProcess_a[95]--;
	}
	else if(nIdentifiedNum_preProcess_a[95] == 120 ){
	//	if(u32_pattern214_forbindden_en == 1){
			//++solution settings:
			//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 4, 7, 0x0);
#if 0 // merlin7_bring_up_dehalo
			WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 29 , 29 , 0 ); // new algo switch
			WriteRegister(KME_DEHALO4_RT2_MOD_1_reg , 0 , 0 , 0 ); //fix1 en
			WriteRegister(KME_DEHALO4_RT2_MOD_1_reg , 1 , 1 , 0 ); //fix1 en
			WriteRegister(KME_ME1_TOP10_APL_FILTER0_reg , 0 , 0 , 0 );
			//--solution settings:
#endif			
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
        WriteRegister(KME_DEHALO5_PHMV_FIX_8_reg,26,30,0x16);
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
        WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23, 0x0);
        WriteRegister(KME_DEHALO5_PHMV_FIX_1D_reg,16,25, 0x12c);
        WriteRegister(KME_DEHALO5_PHMV_FIX_1C_reg,24,30, 0x57);
        WriteRegister(KME_DEHALO5_PHMV_FIX_10_reg,24,31, 0x0);

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
            
       //     WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 8, 11, 0xE);
       //     rtd_pr_memc_info("out214");
            
        WriteRegister(HARDWARE_HARDWARE_24_reg , 19, 28, 0x3FF);
        WriteRegister(HARDWARE_HARDWARE_24_reg , 31, 31, 1);     
        WriteRegister(HARDWARE_HARDWARE_26_reg,31,31, 0x1); //u8_dh_condition_dhbypass  

    //    }

		nIdentifiedNum_preProcess_a[95] = 0;
	}
	
	// 153 merilin7 
	if((scalerVIP_Get_MEMCPatternFlag_Identification(119) == TRUE)) //&&( u25_me_aDTL >= 656575 )&&(u25_me_aDTL<=1221649)&&(u11_gmv_mvx>=25)&&(u11_gmv_mvx<=80)&&(u10_gmv_mvy<=5)&&(u10_gmv_mvy>=-11)
	{
		gmv_calc_x = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[14];
		gmv_calc_y = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[14];
		
		WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x1);
		
		//   WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 12, 15, 0xF);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 1);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, gmv_calc_x);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, gmv_calc_y);
		
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, gmv_calc_x);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, gmv_calc_y);

        WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x1);  //penalty_en
        WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x1); //divide en
            
		u5_blend_holdfrm_preProcess_a[97] = 30;
		nIdentifiedNum_preProcess_a[97]=119;
		 //	rtd_pr_memc_info("in153");
	}
	else if(u5_blend_holdfrm_preProcess_a[97] >0){
		WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x1);  
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 12, 15, 0xF);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, gmv_calc_x);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, gmv_calc_y);

		WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x1);  //penalty_en
        WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x1); //divide en

		//	rtd_pr_memc_info("in153");
		
		 u5_blend_holdfrm_preProcess_a[97]--;
	}
	else if(nIdentifiedNum_preProcess_a[97] == 119 ){
	 
		WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,15,15,0x0);	// init
	 	// 	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 12, 15, 0xE);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0);

		WriteRegister(KME_DEHALO5_PHMV_FIX_2_reg,4,4,0x0);	//penalty_en
		WriteRegister(KME_DEHALO5_PHMV_FIX_29_reg,24,24,0x0); //divide en
		// 	rtd_pr_memc_info("out153");

		nIdentifiedNum_preProcess_a[97] = 0;
	}
	

	// --  merlin6 new algo for #pattern214

	// ++  #181

	if((scalerVIP_Get_MEMCPatternFlag_Identification(117) == TRUE)&&(u11_gmv_mvx<=-34 && u11_gmv_mvx>=-44)&&(u10_gmv_mvy<=-55 && u10_gmv_mvy>=-68)
		&& (u32_gmv_unconf<=6) && (u25_me_aDTL<=3260000 && u25_me_aDTL>=3200000)){
		WriteRegister(HARDWARE_HARDWARE_57_reg, 14, 14, 0x0); // u1_scCtrl_wrt_en;
		WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg, 9, 13, 0x0);
		u5_blend_holdfrm_preProcess_a[96] = 10;
		nIdentifiedNum_preProcess_a[96]=117;
	}else if(u5_blend_holdfrm_preProcess_a[96] >0){
		WriteRegister(HARDWARE_HARDWARE_57_reg, 14, 14, 0x0); // u1_scCtrl_wrt_en;
		WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg, 9, 13, 0x0);
		u5_blend_holdfrm_preProcess_a[96]--;
	}else if(nIdentifiedNum_preProcess_a[96] == 117){
		WriteRegister(HARDWARE_HARDWARE_57_reg, 14, 14, 0x1); // u1_scCtrl_wrt_en;
		nIdentifiedNum_preProcess_a[96] = 0;
	}

	if(u5_blend_holdfrm_preProcess_a[96] >0)
		g_181_flag = 1;
	else
		g_181_flag = 0;

	// --  #181


    //---------------------------189------------------------------//
    right_rim_pre=s_pContext->_output_rimctrl.u9_dh_blk_rim[_RIM_RHT];
    if(scalerVIP_Get_MEMCPatternFlag_Identification(121) == TRUE) //189
    {
        u5_blend_holdfrm_preProcess_a[121] = 30;
        nIdentifiedNum_preProcess_a[121]=117;
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0x1);
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, 0x28);
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0x0);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0x1);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, 0x28);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, 0x0);
        WriteRegister(HARDWARE_HARDWARE_25_reg , 9, 9, 1);

		ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,31,31, &fwgmven);
        ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,0,10, &fwgmvtest);
        if(fwgmven)
        {
             WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, fwgmvtest);   
        }
        
        WriteRegister(HARDWARE_HARDWARE_25_reg , 9, 9, 1);
        
        WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,9,17,  right_rim_pre-15);
    }
    else if(u5_blend_holdfrm_preProcess_a[121] >0)
    {
    	u5_blend_holdfrm_preProcess_a[121]--;
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0x1);
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, 0x28);
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30,0x0);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0x1);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, 0x28);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, 0x0);
	    ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,31,31, &fwgmven);
        ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,0,10, &fwgmvtest);
        if(fwgmven)
        {
             WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20,  fwgmvtest);   
        }     
        WriteRegister(HARDWARE_HARDWARE_25_reg , 9, 9, 1);
        WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,9,17,  right_rim_pre-15);
    }
    else if(nIdentifiedNum_preProcess_a[121] == 117)
    {
    	 nIdentifiedNum_preProcess_a[121] = 0;
        WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0x0);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0x0);
        WriteRegister(HARDWARE_HARDWARE_25_reg , 9, 9, 0);
		WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,9,17,  right_rim_pre);
    }
    
    //old 189 and 299, no use
    if(0)
    {
    	if((scalerVIP_Get_MEMCPatternFlag_Identification(122) == TRUE) && (scalerVIP_Get_MEMCPatternFlag_Identification(121) == 0)) 
    	{

    		for(nIdxX=0; nIdxX<32; nIdxX++)
    		{
    			s11_rmv_mvx[nIdxX] = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[nIdxX];
    			s10_rmv_mvy[nIdxX] = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nIdxX];
    		}

			for(nIdxX=0; nIdxX<32; nIdxX++)
			{
				u32_RgnApl[nIdxX] = 0;
				u32_RgnApl[nIdxX] = (s_pContext->_output_read_comreg.u20_me_rAPLi_rb[nIdxX] + s_pContext->_output_read_comreg.u20_me_rAPLp_rb[nIdxX]+1012)/2024;
			}
			
			gmv_calc_rapl_min_temp = u32_RgnApl[8];
			gmv_calc_rapl_min_idx = 8;
								
			for (nIdxX = 0; nIdxX <= 31; nIdxX++)
			{
				if((nIdxX != 1) &&(nIdxX != 2)&&(nIdxX != 11)&&(nIdxX != 22))
				{
					if(u32_RgnApl[nIdxX] < gmv_calc_rapl_min_temp)
					{
						gmv_calc_rapl_min_temp = u32_RgnApl[nIdxX];
						gmv_calc_rapl_min_idx = nIdxX;
					}
				}					
			}
					
			gmv_calc_x = s11_rmv_mvx[gmv_calc_rapl_min_idx];
			gmv_calc_y = s10_rmv_mvy[gmv_calc_rapl_min_idx];
			rmv_temp_x = s11_rmv_mvx[gmv_calc_rapl_min_idx];
			swap_s1_flag = 1;
			if(frameCnt_tmp % 2 == 1)
			{
				cond1 = (_ABS_DIFF_(gmv_calc_x , pre_calc_gmv_mvx) > reg_gmvd_th) ? 1 : 0;
				pre_calc_gmv_mvx = gmv_calc_x;
				pre_calc_gmv_rgn_idx = gmv_calc_rapl_min_idx;		
			}
			else
			{
				cond0 = (_ABS_DIFF_(gmv_calc_x , pre_calc_gmv_mvx) > reg_gmvd_th) ? 1 : 0; 
				pre_calc_gmv_mvx = gmv_calc_x;
				pre_calc_gmv_rgn_idx = gmv_calc_rapl_min_idx;
			}

			if(cond0 || cond1)
			{
				for(nIdxX=0; nIdxX<32; nIdxX++)
				{
					if((nIdxX != 1)&&(nIdxX != 2)&&(nIdxX != 11)&&(nIdxX != 22)&&(nIdxX != 31)&&(nIdxX != 25)&&(nIdxX != 7))
					{
						if(u32_RgnApl[nIdxX] < 80)
						{
							gmvd_calc_x = _ABS_DIFF_(pre_calc_gmv_mvx , s11_rmv_mvx[nIdxX]);
							//gmvd_calc_y = abs(pre_calc_gmv_mvy - s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nIdxX]);
							if(gmvd_calc_x_min > gmvd_calc_x)
							{
												  gmvd_calc_x_min = gmvd_calc_x;  
												  gmvd_calc_x_min_idx = nIdxX;
							}
						}	
					}
				}
				gmv_calc_x = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[gmvd_calc_x_min_idx];
				gmv_calc_y = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[gmvd_calc_x_min_idx]; 
				swap_s2_flag = 1;
			}			
			if(print_detail_en)
			{
				rtd_pr_memc_emerg("func:189_path\n");	
			}
			//pOutput->s1_gmv_calc_mvx = gmv_calc_x;
			//pOutput->s1_gmv_calc_mvy = gmv_calc_y;
			WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 1);
			WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, gmv_calc_x);
			WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0);
			WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
			WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, gmv_calc_x);
			WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, gmv_calc_y);

			//WriteRegister(HARDWARE_HARDWARE_57_reg, 14, 14, 0x0); // u1_scCtrl_wrt_en;
    		//WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg, 9, 13, 0x0);
    	    //WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 1);
    		//WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, 45);
    		//WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0);

            //WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
            //WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, 45);
            //WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, 0);

            WriteRegister(HARDWARE_HARDWARE_25_reg , 8,8, 1);
            
    		u5_blend_holdfrm_preProcess_a[122] = 10;
    		nIdentifiedNum_preProcess_a[122]=118;
    	}
    	else if(u5_blend_holdfrm_preProcess_a[122] >0)
    	{
            WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 1);
            WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, gmv_calc_x);
            WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0);
            WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
            WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, gmv_calc_x);
            WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, gmv_calc_y);
    		//WriteRegister(HARDWARE_HARDWARE_57_reg, 14, 14, 0x0); // u1_scCtrl_wrt_en;
    		//WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg, 9, 13, 0x0);
    		//WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 1);
    		//WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, 45);
    		//WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0);
		
            //WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
            //WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, 45);
            //WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, 0);
                //WriteRegister(HARDWARE_HARDWARE_25_reg , 8, 8, 1);
            
    		u5_blend_holdfrm_preProcess_a[122]--;
    	}
    	else if(nIdentifiedNum_preProcess_a[122] == 118)
    	{
            WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0);
                //WriteRegister(HARDWARE_HARDWARE_57_reg, 14, 14, 0x1); // u1_scCtrl_wrt_en;

                WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0);

				 nIdentifiedNum_preProcess_a[122] = 0;
				 WriteRegister(HARDWARE_HARDWARE_25_reg , 8, 8, 0);
		 }
	 }
	 
	 //---------------------------299 last------------------------------//
	if((scalerVIP_Get_MEMCPatternFlag_Identification(130) == TRUE) && (scalerVIP_Get_MEMCPatternFlag_Identification(121) == 0)) //299
	 {
		 u5_blend_holdfrm_preProcess_a[130] = 10;
		 nIdentifiedNum_preProcess_a[130]=118;
		 WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0x1);
		 WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, 0x7d6);
		 WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0x0);
		 WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0x1);
		 WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, 0x7d6);
		 WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, 0x0);
		 WriteRegister(HARDWARE_HARDWARE_25_reg , 8, 8, 1);
		 WriteRegister(HARDWARE_HARDWARE_25_reg , 12, 12, 0);
       //  WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 0, 3, 0x1); 
	 }
	 else if(u5_blend_holdfrm_preProcess_a[130] >0)
	 {
		 u5_blend_holdfrm_preProcess_a[130]--;
		 WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0x1);
		 WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, 0x7d6);
		 WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0x0);
		 WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0x1);
		 WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, 0x7d6);
		 WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, 0x0);
		 WriteRegister(HARDWARE_HARDWARE_25_reg , 8, 8, 1);
		 WriteRegister(HARDWARE_HARDWARE_25_reg , 12, 12, 0);

      //   WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 0, 3, 0x2); 
	 }
	 else if(nIdentifiedNum_preProcess_a[130] == 118)
	 {
		  nIdentifiedNum_preProcess_a[130] = 0;
		 WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0x0);
		 WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0x0);
		 WriteRegister(HARDWARE_HARDWARE_25_reg , 8, 8, 0);
		 // WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 0, 3, 0x0); 
      	 WriteRegister(HARDWARE_HARDWARE_25_reg , 12, 12, 0);
	 }

	 //---------------------------299 begin------------------------------//
	if((scalerVIP_Get_MEMCPatternFlag_Identification(129) == TRUE) && (scalerVIP_Get_MEMCPatternFlag_Identification(130) == TRUE) && (scalerVIP_Get_MEMCPatternFlag_Identification(121) == 0)) //299
	 {
		 incnt_for_299++;
		 incnt_for_299 = (incnt_for_299 >= 15) ? 15 : incnt_for_299;
	 }
	else
	{
		 incnt_for_299 = 0;
	}

	if(incnt_for_299 == 15)
	{
		 u5_blend_holdfrm_preProcess_a[129] = 60;
		 nIdentifiedNum_preProcess_a[129]=119;
		 WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0x1);
		 WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, 0x7e7);
		 WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0x0);
		 WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0x1);
		 WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, 0x7e7);
		 WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, 0x0);
		 WriteRegister(HARDWARE_HARDWARE_25_reg , 8, 8, 1);
	}
	 else if(u5_blend_holdfrm_preProcess_a[129] >0)
	 {
		  u5_blend_holdfrm_preProcess_a[129]--;
		 WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0x1);
		 WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, 0x7e7);
		 WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0x0);
		 WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0x1);
		 WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, 0x7e7);
		 WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, 0x0);
		 WriteRegister(HARDWARE_HARDWARE_25_reg , 8, 8, 1);
		 WriteRegister(HARDWARE_HARDWARE_25_reg , 12, 12, 1);
	 }
	 else if(nIdentifiedNum_preProcess_a[129] == 119)
	 {
		  nIdentifiedNum_preProcess_a[129] = 0;
		 WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0x0);
		 WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0x0);
		 WriteRegister(HARDWARE_HARDWARE_25_reg , 8, 8, 0);
		 // WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 0, 3, 0x0); 
         WriteRegister(HARDWARE_HARDWARE_25_reg , 12, 12, 0);
	 }
	 
	 //rtd_pr_memc_info("189: %d %d  299: %d %d", (scalerVIP_Get_MEMCPatternFlag_Identification(121) == TRUE), u5_blend_holdfrm_preProcess_a[121], ((scalerVIP_Get_MEMCPatternFlag_Identification(122) == TRUE) && (scalerVIP_Get_MEMCPatternFlag_Identification(121) == 0)), u5_blend_holdfrm_preProcess_a[122]);
	 //rtd_pr_memc_info("299 begin(%d)  incnt(%d) after(%d)", u5_blend_holdfrm_preProcess_a[129], incnt_for_299, u5_blend_holdfrm_preProcess_a[130]);
//#45  djntest
	if((scalerVIP_Get_MEMCPatternFlag_Identification(123) == TRUE)&&(s_pContext->_output_read_comreg.me1_gmv_bgcnt>8000)) //&&( u25_me_aDTL >= 656575 )&&(u25_me_aDTL<=1221649)&&(u11_gmv_mvx>=25)&&(u11_gmv_mvx<=80)&&(u10_gmv_mvy<=5)&&(u10_gmv_mvy>=-11)
    {
		WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,12,12,0x0);  //reg_dh_only_mv_fix_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x0);//reg_dh_bg_extend_cond3_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,1,1,0x1);//reg_dh_pstflt_occl_sel 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,0,0,0x1);//reg_dh_pstflt_mv_sel 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x64);  //reg_dh_color_protect_th0_max 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,25,30,0x6);  //reg_dh_pred_bg_change_pfv_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_36_reg,10,15,0x6);  //reg_dh_pred_bg_change_ppfv_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,11,11,0x1);  //reg_dh_fg_r_big_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,9,9,0x1);  //reg_dh_fg_l_big_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,7,7,0x1);  //reg_dh_fg_c_big_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,6,6,0x1);  //reg_dh_fg_protect_by_apl_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_3B_reg,24,31,0xf);  //reg_dh_fg_lr_big_min_cur_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_3B_reg,16,23,0xf);  //reg_dh_fg_lr_big_min_pre_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_3B_reg,8,15,0xf);  //reg_dh_fg_lr_big_max_cur_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_3B_reg,0,7,0xf);  //reg_dh_fg_lr_big_max_pre_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_43_reg,8,13,0x5);  //reg_dh_fg_me2_sim_max_diff_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_43_reg,0,5,0x5);  //reg_dh_fg_me2_sim_min_diff_th 45

		WriteRegister(HARDWARE_HARDWARE_24_reg , 19, 28, 0x0);
		WriteRegister(HARDWARE_HARDWARE_24_reg , 31, 31, 0x0);	
		u5_blend_holdfrm_preProcess_a[99] = 30;
		nIdentifiedNum_preProcess_a[99]=123;

	}
	else if(u5_blend_holdfrm_preProcess_a[99] >0)
	{
		WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,12,12,0x0);  //reg_dh_only_mv_fix_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x0);//reg_dh_bg_extend_cond3_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,1,1,0x1);//reg_dh_pstflt_occl_sel 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,0,0,0x1);//reg_dh_pstflt_mv_sel 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x64);  //reg_dh_color_protect_th0_max 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,25,30,0x6);  //reg_dh_pred_bg_change_pfv_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_36_reg,10,15,0x6);  //reg_dh_pred_bg_change_ppfv_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,11,11,0x1);  //reg_dh_fg_r_big_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,9,9,0x1);  //reg_dh_fg_l_big_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,7,7,0x1);  //reg_dh_fg_c_big_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,6,6,0x1);  //reg_dh_fg_protect_by_apl_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_3B_reg,24,31,0xf);  //reg_dh_fg_lr_big_min_cur_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_3B_reg,16,23,0xf);  //reg_dh_fg_lr_big_min_pre_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_3B_reg,8,15,0xf);  //reg_dh_fg_lr_big_max_cur_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_3B_reg,0,7,0xf);  //reg_dh_fg_lr_big_max_pre_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_43_reg,8,13,0x5);  //reg_dh_fg_me2_sim_max_diff_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_43_reg,0,5,0x5);  //reg_dh_fg_me2_sim_min_diff_th 45
					
		WriteRegister(HARDWARE_HARDWARE_24_reg , 19, 28, 0x0);
		WriteRegister(HARDWARE_HARDWARE_24_reg , 31, 31, 0x0);	
	 
		u5_blend_holdfrm_preProcess_a[99]--;
	}
	else if(nIdentifiedNum_preProcess_a[99] == 123 )
	{		  
		WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,12,12,0x1);  //reg_dh_only_mv_fix_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,14,14,0x1);//reg_dh_bg_extend_cond3_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,1,1,0x0);//reg_dh_pstflt_occl_sel 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_1_reg,0,0,0x0);//reg_dh_pstflt_mv_sel 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_21_reg,8,15,0x22);  //reg_dh_color_protect_th0_max 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,25,30,0x8);  //reg_dh_pred_bg_change_pfv_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_36_reg,10,15,0x8);  //reg_dh_pred_bg_change_ppfv_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,11,11,0x0);  //reg_dh_fg_r_big_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,9,9,0x0);  //reg_dh_fg_l_big_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,7,7,0x0);  //reg_dh_fg_c_big_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_38_reg,6,6,0x0);  //reg_dh_fg_protect_by_apl_en 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_3B_reg,24,31,0x12);	//reg_dh_fg_lr_big_min_cur_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_3B_reg,16,23,0x12);	//reg_dh_fg_lr_big_min_pre_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_3B_reg,8,15,0x12);  //reg_dh_fg_lr_big_max_cur_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_3B_reg,0,7,0x12);  //reg_dh_fg_lr_big_max_pre_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_43_reg,8,13,0x0);  //reg_dh_fg_me2_sim_max_diff_th 45
		WriteRegister(KME_DEHALO5_PHMV_FIX_43_reg,0,5,0x9);  //reg_dh_fg_me2_sim_min_diff_th 45

		WriteRegister(HARDWARE_HARDWARE_24_reg , 19, 28, 0x3FF);
		WriteRegister(HARDWARE_HARDWARE_24_reg , 31, 31, 1); 
	
		nIdentifiedNum_preProcess_a[99] = 0;
	}
	

//#298 end

    
	/*if((scalerVIP_Get_MEMCPatternFlag_Identification(122) == TRUE))
	{
		if(print_detail_en)
		{
			rtd_pr_memc_emerg("func:299_path\n");	
		}
		//WriteRegister(HARDWARE_HARDWARE_57_reg, 14, 14, 0x0); // u1_scCtrl_wrt_en;
		//WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg, 9, 13, 0x0);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 1);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, -40);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0);
       // WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,25); 
      //  WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,10,19,25); 
      //  WriteRegister(KME_ME1_BG0_ME1_BG_14_reg,0,7,25); 

       WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
       WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, 45);
       WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, 0);

       WriteRegister(HARDWARE_HARDWARE_24_reg , 17, 17, 0);

       
		u5_blend_holdfrm_preProcess_a[122] = 10;
		nIdentifiedNum_preProcess_a[122]=117;
	}
	else if(u5_blend_holdfrm_preProcess_a[121] >0)
	{
		//WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 1);
		//WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, gmv_calc_x);
		//WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, gmv_calc_y);
		//WriteRegister(HARDWARE_HARDWARE_57_reg, 14, 14, 0x0); // u1_scCtrl_wrt_en;
		//WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg, 9, 13, 0x0);
	    WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 1);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,10,20, 45);
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg,21,30, 0);


        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 1);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,8,18, 45);
        WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,19,28, 0);

        WriteRegister(HARDWARE_HARDWARE_24_reg , 17, 17, 0);
		u5_blend_holdfrm_preProcess_a[122]--;
	}
	else if(nIdentifiedNum_preProcess_a[122] == 117)
	{
		WriteRegister(KME_ME1_BG0_ME1_BG_15_reg, 31, 31, 0);
		//WriteRegister(HARDWARE_HARDWARE_57_reg, 14, 14, 0x1); // u1_scCtrl_wrt_en;

    WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg,0,0, 0);

    WriteRegister(HARDWARE_HARDWARE_24_reg , 17, 17, 1);
		nIdentifiedNum_preProcess_a[122] = 0;
	}
    */

    //------299  

// --  #189


	//===debug show
	for(u8_Index =0;u8_Index<255;u8_Index++)
	{
		if(nIdentifiedNum_preProcess_a[u8_Index] !=0){
			if(u32_ID_Log_en==1)
			rtd_pr_memc_emerg("===== *** Pre *** nIdentifiedNum_preProcess_a>>[%d][%d]\n", u8_Index,nIdentifiedNum_preProcess_a[u8_Index]);

			pOutput->u1_IP_preProcess_true=1;

		}
	}
	//===
}
#endif
#if 0
VOID Identification_Pattern_wrtAction(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	static unsigned char u5_blend_holdfrm_a[255] = {0};
	static unsigned char nIdentifiedNum_a[255] = {0};
	//unsigned int mot_diff = s_pContext->_output_filmDetectctrl.u27_ipme_motionPool[_FILM_ALL][0];
	static unsigned char nOnceFlag=12;
	unsigned char u8_Index;
	#if Pro_TCL
	//unsigned char  u8_cadence_Id = s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL];
	//PQL_INPUT_FRAME_RATE in_fmRate = s_pContext->_external_data._input_frameRate;
	unsigned char nblend_y_alpha_Target=0x90;
	unsigned char nblend_uv_alpha_Target=0x70;
	unsigned char nblend_logo_y_alpha_Target=0xd0;
	unsigned char nblend_logo_uv_alpha_Target=0x30;
	#endif

	unsigned int u32_RB_Value;
	signed short u11_gmv_mvx = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
	signed short u10_gmv_mvy = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;
	//unsigned int u25_me_aDTL = s_pContext->_output_read_comreg.u25_me_aDTL_rb;
	//unsigned int u26_aAPLi_rb = s_pContext->_output_read_comreg.u26_me_aAPLi_rb;
	unsigned int u32_pattern298_zidoo_player_forbindden_en;

	unsigned char  u8_cadence_Id = s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL];
	unsigned char  u8_Mixmode_flag = s_pContext->_output_filmDetectctrl.u1_mixMode;
    //int gmv_mvx =0;//, gmv_mvy =0 ;
	PQL_INPUT_FRAME_RATE in_fmRate = s_pContext->_external_data._input_frameRate;


	#if 1 //for YE Test temp test
	unsigned int u32_ID_Log_en=0;//,ES_Test_Value=0;
	ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 24,  24, &u32_ID_Log_en);//YE Test debug dummy register
	#endif

	if((pParam->u1_Identification_Pattern_en== 0)||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_Test_0_flag ==0))
		return;

	//reset
	pOutput->u1_IP_wrtAction_true=0;

	//====================================================================
	if( (scalerVIP_Get_MEMCPatternFlag_Identification(1) == TRUE)&&((u11_gmv_mvx!=0)&&(u10_gmv_mvy!=0))&&(s_pContext->_output_frc_sceneAnalysis.u8_OSDPictureModeChg_prt_apply!=0))
	{
		WriteRegister(IPPRE_IPPRE_0C_reg, 0, 7, 0x30);

		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x28);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x24);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x18);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0x14);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0xa);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0x8);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x28);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x24);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x18);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0x14);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0xa);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0x8);


		if(nOnceFlag >=1)
		{
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_3C_reg,0,8, 0);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_3C_reg,9,15, 0);
			#if (IC_K5LP || (IC_K6LP || IC_K8LP))
			WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_E0_reg,0,7,0);
			WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_E0_reg,8,14,0);
			#endif
			nOnceFlag --;
		}

		u5_blend_holdfrm_a[1] = 20;
		nIdentifiedNum_a[1]=1;

	}
	else if(u5_blend_holdfrm_a[1] >0)
	{
		WriteRegister(IPPRE_IPPRE_0C_reg, 0, 7, 0x30);

		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x28);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x24);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x20);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x18);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0x14);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0xa);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0x8);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x28);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x24);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x20);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x18);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0x14);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0xa);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0x8);

		if(nOnceFlag >=1)
		{
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_3C_reg,0,8, 0);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_3C_reg,9,15, 0);
			#if (IC_K5LP || (IC_K6LP || IC_K8LP))
			WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_E0_reg,0,7,0);
			WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_E0_reg,8,14,0);
			#endif
			nOnceFlag --;
		}

		u5_blend_holdfrm_a[1]--;
	}
	else if(nIdentifiedNum_a[1] == 1 )
	{
		WriteRegister(IPPRE_IPPRE_0C_reg, 0, 7, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_logo_y_alpha);

		nOnceFlag=12;
		nIdentifiedNum_a[1] = 0;
	}
	//====================================================================

	if(scalerVIP_Get_MEMCPatternFlag_Identification(2) == TRUE  )
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x1c);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x18);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x14);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0x10);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0xe);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0xa);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x1c);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x18);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x14);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0x10);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0xe);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0xa);


		u5_blend_holdfrm_a[2] = 8;
		nIdentifiedNum_a[2]=2;

	}
	else if(u5_blend_holdfrm_a[2] >0)
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, 0x20);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, 0x1c);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, 0x18);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, 0x14);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, 0x10);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, 0xe);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, 0xa);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, 0x20);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, 0x1c);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, 0x18);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, 0x14);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, 0x10);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, 0xe);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, 0xa);

		u5_blend_holdfrm_a[2]--;
	}
	else if(nIdentifiedNum_a[2] == 2 )
	{
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp0);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp1);
		WriteRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp2);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 0, 8, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp3);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 9, 17, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp4);
		WriteRegister(KME_IPME_KME_IPME_90_reg, 18, 26, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp5);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 8, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp6);

		WriteRegister(KME_IPME_KME_IPME_94_reg, 9, 17, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp0);
		WriteRegister(KME_IPME_KME_IPME_94_reg, 18, 26, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp1);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 0, 8, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp2);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 9, 17, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp3);
		WriteRegister(KME_IPME_KME_IPME_98_reg, 18, 26, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp4);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 0, 8, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp5);
		WriteRegister(KME_IPME_KME_IPME_04_reg, 9, 17, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp6);


		nIdentifiedNum_a[2] = 0;
	}
//====================================================================
#if 0
	if(scalerVIP_Get_MEMCPatternFlag_Identification(100) == TRUE  )
	{
		//====fanny_xiong settings for K7 dehalo.====
		WriteRegister(MC2_MC2_50_reg, 0, 1, 0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 13, 13, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 12, 12, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 11, 11, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 12, 12, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 11, 11, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 10, 10, 0x1);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg, 24, 31, 0x28);

		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg, 16, 23, 0x28);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,  8, 15, 0x28);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,  0,  7, 0x28);

		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg, 24, 31, 0x28);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg, 16, 23, 0x30);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,  8, 15, 0x30);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,  0,  7, 0x28);
		//====over=======

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,4,8,  10);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,13,13,  1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,16,23, 16);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,24,31, 16);

		WriteRegister(KME_DEHALO2_KME_DEHALO_POSTCORR_reg,0,3, 12);
		WriteRegister(KME_DEHALO2_KME_DEHALO_POSTCORR_reg,4,7,  12);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,  3);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,  0);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26, 3);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29, 0);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,  1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,  0);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11, 1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14, 0);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,4,5,  1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,8,9, 1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,12,14,   1);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,8,15, 32);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,24,31, 40);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8, 1);

		u5_blend_holdfrm_a[3] = 50;
		nIdentifiedNum_a[3]=100;
	}
	else if( u5_blend_holdfrm_a[3] > 0 )
	{
		//====fanny_xiong settings for K7 dehalo.====
		WriteRegister(MC2_MC2_50_reg, 0, 1, 0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 13, 13, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 12, 12, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 11, 11, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 12, 12, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 11, 11, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 10, 10, 0x1);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg, 24, 31, 0x28);

		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg, 16, 23, 0x28);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,  8, 15, 0x28);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,  0,  7, 0x28);

		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg, 24, 31, 0x28);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg, 16, 23, 0x30);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,  8, 15, 0x30);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,  0,  7, 0x28);
		//====over=======

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,4,8,  10);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,13,13,  1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,16,23, 16);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,24,31, 16);

		WriteRegister(KME_DEHALO2_KME_DEHALO_POSTCORR_reg,0,3, 12);
		WriteRegister(KME_DEHALO2_KME_DEHALO_POSTCORR_reg,4,7,  12);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,  3);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,  0);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26, 3);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29, 0);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,  1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,  0);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11, 1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14, 0);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,4,5,  1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,8,9, 1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,12,14,   1);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,8,15, 32);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,24,31, 40);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8, 1);

		u5_blend_holdfrm_a[3]--;
		
	}
	else if(nIdentifiedNum_a[3]==100)
	{

		//===fanny_xiong settings for K7 dehalo:===
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 0, 3, 0x0);
                if(s_pContext->_external_data._output_mode == _PQL_OUT_VIDEO_MODE)
                        WriteRegister(MC2_MC2_50_reg, 0, 1, 0x3);
                if(s_pContext->_external_data._output_mode == _PQL_OUT_PC_MODE)
                        WriteRegister(MC2_MC2_50_reg, 0, 1, 0x0);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand3_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand4_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand3_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand4_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 13, 13, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update1_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 12, 12, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update2_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 11, 11, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update0_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 12, 12, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update1_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 11, 11, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update2_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 10, 10, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update0_en);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg, 24, 31, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st3);

		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg, 16, 23, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st4);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,  8, 15, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st5);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,  0,  7, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st6);

		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg, 24, 31, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st3);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg, 16, 23, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st4);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,  8, 15, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st5);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,  0,  7, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st6);

		//====over=======
		
		nIdentifiedNum_a[3]=0;
	}

#endif

#if 1    //YE Test temp remark for little boy
	if(scalerVIP_Get_MEMCPatternFlag_Identification(101) == TRUE  )
	{
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,4,8,  10);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,13,13,  1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,16,23, 16);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,24,31, 16);

		WriteRegister(KME_DEHALO2_KME_DEHALO_POSTCORR_reg,0,3, 12);
		WriteRegister(KME_DEHALO2_KME_DEHALO_POSTCORR_reg,4,7,  12);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,  3);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,  0);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26, 3);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29, 0);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,  1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,  0);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11, 1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14, 0);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,4,5,  1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,8,9, 1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,12,14,   1);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,8,15, 32);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,24,31, 40);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8, 1);

		u5_blend_holdfrm_a[4] = 8;
		nIdentifiedNum_a[4]=101;
	}
	else if( u5_blend_holdfrm_a[101] > 0 )
	{
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,4,8,  10);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,13,13,  1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,16,23, 16);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,24,31, 16);

		WriteRegister(KME_DEHALO2_KME_DEHALO_POSTCORR_reg,0,3, 12);
		WriteRegister(KME_DEHALO2_KME_DEHALO_POSTCORR_reg,4,7,  12);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,  3);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,  0);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26, 3);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29, 0);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,  1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,  0);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11, 1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14, 0);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,4,5,  1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,8,9, 1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,12,14,   1);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,8,15, 32);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,24,31, 40);

		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8, 1);

		u5_blend_holdfrm_a[4]--;
	}
	else if(nIdentifiedNum_a[4]==101)
	{
		nIdentifiedNum_a[4]=0;
	}
	//===========================

#endif

	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_44_reg,10,19,&u32_RB_Value);

	if((scalerVIP_Get_MEMCPatternFlag_Identification( 0, 105) == TRUE)&&(u32_RB_Value<=510))
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_4C_reg, 6, 10, 0x8);
		u5_blend_holdfrm_a[5] = 8;
		nIdentifiedNum_a[5]=105;	
	}
	else if(u5_blend_holdfrm_a[5] >0)
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_4C_reg, 6, 10, 0x8);
		u5_blend_holdfrm_a[5]--;
	}
	else if(nIdentifiedNum_a[5] == 105 )
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_4C_reg, 6, 10, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_invalid_cnt_th);
		nIdentifiedNum_a[5] = 0;
	}
	//++ u1_Q13835_flag
	#if 1 //YE Test temp remark for little boy
     if((scalerVIP_Get_MEMCPatternFlag_Identification(106) == TRUE  )
	 	&&(( u8_cadence_Id == _CAD_VIDEO) ||(u8_Mixmode_flag==1))
	 	&&(s_pContext->_output_me_sceneAnalysis.u1_Q13835_flag==1))
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_4C_reg, 6, 10, 0x8);
		u5_blend_holdfrm_a[6] = 24;
		nIdentifiedNum_a[6]=106;
	}
	else if(u5_blend_holdfrm_a[6] >0)
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_4C_reg, 6, 10, 0x8);
		u5_blend_holdfrm_a[6]--;
	}
	else if(nIdentifiedNum_a[6] == 106 )
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_4C_reg, 6, 10, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_invalid_cnt_th);

		nIdentifiedNum_a[6] = 0;
	}
	#endif 
	//-- u1_Q13835_flag

	#if 0
	// ++ YE Test Edge Shaking v3
	if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag ==1)
	{

		if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag ==1)
		u5_blend_holdfrm_a[4] = 60;
		else
		u5_blend_holdfrm_a[4] = 12;

		nIdentifiedNum_a[7]=107;
	}
	else if(u5_blend_holdfrm_a[4] >0)
	{
		if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag ==0)
		u5_blend_holdfrm_a[4]=0;
		else
		u5_blend_holdfrm_a[4]--;
	}
	else if(nIdentifiedNum_a[7] == 107 )
	{

		nIdentifiedNum_a[7] = 0;
	}
	// -- YE Test Edge Shaking
	#endif

#if 1
 //++ Merlin6 memc pred mv new algo for soccer 
       ReadRegister(HARDWARE_HARDWARE_24_reg, 10, 10, &u32_RB_Value);
       if(u32_RB_Value==1)
       {
		if((scalerVIP_Get_MEMCPatternFlag_Identification(114) == TRUE))
		{
			WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_1_reg, 28, 28,  1  );  // new algo switch  enable
			u5_blend_holdfrm_a[7] = 25;
			nIdentifiedNum_a[7]=114;
		}
		else if(u5_blend_holdfrm_a[7] >0)
		{
			WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_1_reg, 28, 28,  1  );  
			u5_blend_holdfrm_a[7]--;
		}
		else if(nIdentifiedNum_a[7] == 114 )
		{
			WriteRegister(KME_ME1_TOP10_PRED_MV_CAND_1_reg, 28, 28,  0  );  
			nIdentifiedNum_a[7] = 0;
		}
       }
 //-- Merlin6 memc pred mv new algo for soccer 
#endif	

#if 1 //YE Test 20200521 remove here
//++#pattern 298 daredevil boy on zidoo player.
	ReadRegister(HARDWARE_HARDWARE_58_reg, 3, 3, &u32_pattern298_zidoo_player_forbindden_en);   //  bit3

	if(u32_pattern298_zidoo_player_forbindden_en== 1){
		if(scalerVIP_Get_MEMCPatternFlag_Identification(108) == TRUE  ){
			//========action part:=======
			//====fanny_xiong settings for K7 dehalo.====
			WriteRegister(MC2_MC2_50_reg, 0, 1, 0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 13, 13, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 12, 12, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 11, 11, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 12, 12, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 11, 11, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 10, 10, 0x1);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg, 24, 31, 0x28);

			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg, 16, 23, 0x28);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,  8, 15, 0x28);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,  0,  7, 0x28);

			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg, 24, 31, 0x28);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg, 16, 23, 0x30);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,  8, 15, 0x30);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,  0,  7, 0x28);
			//====over=======

			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,4,8,  10);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,13,13,  1);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,16,23, 16);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,24,31, 16);

			WriteRegister(KME_DEHALO2_KME_DEHALO_POSTCORR_reg,0,3, 12);
			WriteRegister(KME_DEHALO2_KME_DEHALO_POSTCORR_reg,4,7,  12);

			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,  3);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,  0);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26, 3);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29, 0);

			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,  1);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,  0);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11, 1);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14, 0);

			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,4,5,  1);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,8,9, 1);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,12,14,   1);

			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,8,15, 32);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,24,31, 40);

			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8, 1);

			//========over===========

			u5_blend_holdfrm_a[8] = 80;
			nIdentifiedNum_a[8]=108;

		}else if( u5_blend_holdfrm_a[8] > 0 ){
			//========action part:=======
			//====fanny_xiong settings for K7 dehalo.====
			WriteRegister(MC2_MC2_50_reg, 0, 1, 0x3);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 13, 13, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 12, 12, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 11, 11, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 12, 12, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 11, 11, 0x1);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 10, 10, 0x1);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg, 24, 31, 0x28);

			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg, 16, 23, 0x28);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,  8, 15, 0x28);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,  0,  7, 0x28);

			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg, 24, 31, 0x28);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg, 16, 23, 0x30);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,  8, 15, 0x30);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,  0,  7, 0x28);
			//====over=======

			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,4,8,  10);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,13,13,  1);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,16,23, 16);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,24,31, 16);

			WriteRegister(KME_DEHALO2_KME_DEHALO_POSTCORR_reg,0,3, 12);
			WriteRegister(KME_DEHALO2_KME_DEHALO_POSTCORR_reg,4,7,  12);

			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,  3);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,19,21,  0);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26, 3);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,27,29, 0);

			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,2,4,  1);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,5,7,  0);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,9,11, 1);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,12,14, 0);

			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,4,5,  1);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,8,9, 1);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_2_reg,12,14,   1);

			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,8,15, 32);
			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_3_reg,24,31, 40);

			WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8, 1);

			//========over===========

			u5_blend_holdfrm_a[8]--;
		}
		else if(nIdentifiedNum_a[8]==108){
			//========action part:=======

			//===fanny_xiong settings for K7 dehalo:===

			WriteRegister(MC2_MC2_50_reg, 0, 1, 0x0);
			/*
	                if(s_pContext->_external_data._output_mode == _PQL_OUT_VIDEO_MODE)
	                        WriteRegister(MC2_MC2_50_reg, 0, 1, 0x3);
	                if(s_pContext->_external_data._output_mode == _PQL_OUT_PC_MODE)
	                        WriteRegister(MC2_MC2_50_reg, 0, 1, 0x0);
			*/
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand3_en);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand4_en);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand3_en);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand4_en);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 13, 13, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update1_en);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 12, 12, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update2_en);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 11, 11, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update0_en);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 12, 12, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update1_en);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 11, 11, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update2_en);
			WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 10, 10, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update0_en);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg, 24, 31, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st3);

			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg, 16, 23, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st4);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,  8, 15, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st5);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg,  0,  7, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st6);

			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg, 24, 31, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st3);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg, 16, 23, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st4);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,  8, 15, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st5);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg,  0,  7, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st6);

			//========over===========

			nIdentifiedNum_a[8]=0;
		}
	}
//--#pattern 298 daredevil boy on zidoo player.
#endif




	#if 1
	// ++ YE Test Edge Shaking
	if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_413_ID_flag ==1)
	{

		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_00_reg,24,29,0xd);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_01_reg,0,5,0x6);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_50_reg,24,29,0xd);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_54_reg,0,5,0x6);

		u5_blend_holdfrm_a[9] = 48;


		nIdentifiedNum_a[9]=110;

	}
	else if(u5_blend_holdfrm_a[9] >0)
	{

		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_00_reg,24,29,0xd);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_01_reg,0,5,0x6);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_50_reg,24,29,0xd);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_54_reg,0,5,0x6);

			u5_blend_holdfrm_a[9]--;

		if (s_pContext->_output_bRMV_rFB_ctrl.u8_ES_413_ID_flag ==0)
			u5_blend_holdfrm_a[9]=0;

	}
	else if(nIdentifiedNum_a[9] == 110 )
	{

		nIdentifiedNum_a[9] = 0;
	}
	// -- YE Test Edge Shaking
	#endif
#if 0
      //++ Merlin6 memc new algo for #185 notebook  
       ReadRegister(KME_ME1_TOP2_KME_ME1_TOP2_B0_reg, 0, 31, &u32_RB_Value);
       gmv_mvx = ((u32_RB_Value >> 10) &1) == 0? (u32_RB_Value & 0x7ff) : ((u32_RB_Value& 0x7ff)  - (1<<11));
       ReadRegister(HARDWARE_HARDWARE_24_reg, 8, 8, &u32_RB_Value);
       if(scalerVIP_Get_MEMCPatternFlag_Identification(113) == TRUE&&(u32_RB_Value==1)&&((gmv_mvx>-80)&&(gmv_mvx<-5)))
	{
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 0, 15, 1);
		u5_blend_holdfrm_a[7] =150;
		nIdentifiedNum_a[10]=113;
#if 0 // merlin7_bring_up_dehalo
	       WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 29 , 29 , 1 ); // new algo switch
		WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 28 , 28 , 1 ); //UL
		WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 27 , 27 , 1 ); //UR
		WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 26 , 26 , 0 ); //DL
		WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 25 , 25 , 1 ); //DR
		WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 24 , 24 , 0 ); //APL sel
		WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 18 , 23 , 18 );//hi_APL_num_th
		WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 12 , 17 , 18 );//low_APL_num_th
		WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 6 , 11 ,  18 );//near_Gmv_num_th
		WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 0 , 5 ,  25 ); //far_Gmv_num_th

		WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 0 , 7 ,  23 ); //low_Gmv_diff_th
		WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 8 , 15 ,  25 ); //high_Gmv_diff_th
		WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 16 , 23 ,  30 ); //low_APL_th
		WriteRegister(KME_DEHALO4_DH_MV_CORR_30_reg , 24 , 31 ,  70 ); //high_APL_th

		WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 0 , 4 , 0 ); //gmv_num
		WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 6 , 7 , 1 ); //gmv_len
		WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 8 , 15 , 100 ); //set_occl_apl_th
		WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 16 , 23 , 100 ); //apl_th
		WriteRegister(KME_DEHALO4_DH_MV_CORR_34_reg , 24 , 31 , 5); //Gmv_diff_th

		WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 0 , 0 , 1 ); //rtgmvd_bypass
		WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 1 , 1 , 0 ); //gmv_bypass
		WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 2 , 3 , 1 ); //case_mark_mode
		WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 4 , 4 , 1 ); //result_apply
		WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 5 , 5 , 0 );//pred_occl_en
		WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 6 , 6 , 1 ); //occl_hor_en
		WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 7 , 7 , 0 ); //occl_ver_en
		WriteRegister(KME_DEHALO4_DH_MV_CORR_38_reg , 8 , 8 , 0 ); //debug mode

		WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 0 , 2 , 0 );  //res_num
		WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 3 , 4 , 0 );  //res_mode
		WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 5 , 9 , 2 );  //rim down
		WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 10 , 14 , 2 ); //rim up
		WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 15 , 19 , 0 );  //rim right
		WriteRegister(KME_DEHALO4_DH_MV_CORR_3C_reg , 20 , 24 , 0 ); //rim left
#endif

		WriteRegister(HARDWARE_HARDWARE_25_reg, 10, 10 , 0x1 );
		WriteRegister(HARDWARE_HARDWARE_24_reg, 0, 7 ,  1);
		WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg , 0 , 0 , 1 );  //sw gmv en
#if 0 // merlin7_bring_up_dehalo
		WriteRegister(KME_DEHALO4_RT2_MOD_1_reg , 0 , 0 , 0 ); //fix1 en
		WriteRegister(KME_DEHALO4_RT2_MOD_1_reg , 1 , 1 , 0 ); //fix1 en
		WriteRegister(KME_ME1_TOP10_APL_FILTER0_reg , 0 , 0 , 0 );
#endif
	}
	else if((u5_blend_holdfrm_a[7] >0)&&(u32_RB_Value==1))
	{
#if 0 // merlin7_bring_up_dehalo	
		WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 29 , 29 , 1 ); // new algo switch
#endif
		u5_blend_holdfrm_a[7]--;
		WriteRegister(HARDWARE_HARDWARE_25_reg, 10, 10 , 0x1 );
	}
	else if((nIdentifiedNum_a[10] == 113 )&&(u32_RB_Value==1))
	{
#if 0 // merlin7_bring_up_dehalo	
              WriteRegister(KME_DEHALO4_DH_MV_CORR_2C_reg , 29 , 29 , 0); // new algo switch
#endif
              WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_80_reg , 0 , 0 , 0 );
		WriteRegister(HARDWARE_HARDWARE_25_reg, 10, 10 , 0x0 );
		WriteRegister(HARDWARE_HARDWARE_24_reg, 0, 7 ,  0);
		nIdentifiedNum_a[10] = 0;
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 12, 15, 0);
	}
	//-- Merlin6 memc new algo for #185 notebook  
#endif

#if 1 //YE Test edge shaking it's ori place
	// ++ YE Test Edge Shaking
        //if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag	==1)
	if(((s_pContext->_output_bRMV_rFB_ctrl.u8_ES_439_ID_flag ==1)
		||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_21_ID_flag ==1)
		||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_421_ID_flag ==1))
		&&(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag !=1))
	{

		/*
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x3);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x3);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,24,31,0x4);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_04_reg,24,31,0x90);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_44_reg,24,31,0x90);
	      WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_88_reg,0,3,0x0);
		*/
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_00_reg,24,29,0xd);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_01_reg,0,5,0x6);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_01_reg,12,20,0xFC);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_01_reg,21,29,0xFE);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_50_reg,24,29,0xd);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_54_reg,0,5,0x6);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_54_reg,0,5,0x6);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_54_reg,12,20,0xFC);

		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_E0_reg,0,7,0x40);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_3C_reg,0,8,0x40);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,26,27,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,24,25,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,23,23,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,22,22,0x1);

		if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_439_ID_flag ==1){
		u5_blend_holdfrm_a[11] = 60;
		}
		
		if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_421_ID_flag ==1){
		u5_blend_holdfrm_a[11] = 120;
		}

		if (s_pContext->_output_bRMV_rFB_ctrl.u8_ES_21_ID_flag ==1){
		u5_blend_holdfrm_a[11] = 240;//60;
		}
		
		nIdentifiedNum_a[11]=114;
		
	}
	else if(u5_blend_holdfrm_a[11] >0)
	{
		/*
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,16,18,0x3);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,24,26,0x3);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_0_reg,8,8,0x1);
		WriteRegister(KME_DEHALO2_DH_OCCL_EXTEND_1_reg,24,31,0x4);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_04_reg,24,31,0x90);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_44_reg,24,31,0x90);
	      WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_88_reg,0,3,0x0);
		*/
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_00_reg,24,29,0xd);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_01_reg,0,5,0x6);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_01_reg,12,20,0xFC);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_01_reg,21,29,0xFE);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_50_reg,24,29,0xd);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_54_reg,0,5,0x6);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_54_reg,0,5,0x6);
		WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_54_reg,12,20,0xFC);

		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_E0_reg,0,7,0x40);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_3C_reg,0,8,0x40);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,26,27,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,24,25,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,23,23,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg,22,22,0x1);

			u5_blend_holdfrm_a[11]--;

		if ((s_pContext->_output_bRMV_rFB_ctrl.u8_ES_21_ID_flag ==0)
			||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_421_ID_flag ==0)
			||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_439_ID_flag ==0))
			u5_blend_holdfrm_a[11]=0;

	}
	else if(nIdentifiedNum_a[11] == 114 )
	{
		nIdentifiedNum_a[11] = 0;
	}
	// -- YE Test Edge Shaking
#endif





#if 1
	// ++ YE Test move edge shaking apply from FBlevelCtrl.c here
	if((s_pContext->_output_bRMV_rFB_ctrl.u8_ES_439_ID_flag ==1)||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_413_ID_flag ==1)
		||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_421_ID_flag ==1)||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_21_ID_flag==1)
		||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_Rassia_ID_flag==1))
	{
		//WriteRegister(MC_MC_28_reg, 14, 14,0x1);
		//WriteRegister(KMC_BI_bi_top_01_reg,2,3,0x2);


		if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_413_ID_flag ==1){
				WriteRegister(KME_DEHALO_KME_DEHALO_F0_reg, 24, 31, 0x00); 
			      WriteRegister(KME_DEHALO_KME_DEHALO_F4_reg, 24, 31, 0x00);
			      WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x00); 
			      WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0x00);
			}
		else if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_439_ID_flag ==1){
		           WriteRegister(KME_DEHALO_KME_DEHALO_F0_reg, 24, 31, 0x0F); 
			     WriteRegister(KME_DEHALO_KME_DEHALO_F4_reg, 24, 31, 0xFF);
			     WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x0F); 
			     WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0xFF);
			}
		else	if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_421_ID_flag ==1)
			{
		           WriteRegister(KME_DEHALO_KME_DEHALO_F0_reg, 24, 31, 0x0F); 
			     WriteRegister(KME_DEHALO_KME_DEHALO_F4_reg, 24, 31, 0xFF);
			     WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x0F); 
			     WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0xFF);
			}
		else	if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_21_ID_flag ==1)
			{
		           WriteRegister(KME_DEHALO_KME_DEHALO_F0_reg, 24, 31, 0x0F); 
			     WriteRegister(KME_DEHALO_KME_DEHALO_F4_reg, 24, 31, 0xFF);
			     WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x0F); 
			     WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0xFF);
			}
		else	if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_Rassia_ID_flag==1)
			{
		           WriteRegister(KME_DEHALO_KME_DEHALO_F0_reg, 24, 31, 0x0F); 
			     WriteRegister(KME_DEHALO_KME_DEHALO_F4_reg, 24, 31, 0xFF);
			     WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x0F); 
			     WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0xFF);
			}		
		u5_blend_holdfrm_a[12] = 24;//60;
		nIdentifiedNum_a[12]=115;

	}
	else if(u5_blend_holdfrm_a[12] >0)
	{
		//WriteRegister(MC_MC_28_reg, 14, 14,0x1);
		//WriteRegister(KMC_BI_bi_top_01_reg,2,3,0x2);


		if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_413_ID_flag ==1){
				WriteRegister(KME_DEHALO_KME_DEHALO_F0_reg, 24, 31, 0x00); 
			      WriteRegister(KME_DEHALO_KME_DEHALO_F4_reg, 24, 31, 0x00);
			      WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x00); 
			      WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0x00);
		}
		else if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_439_ID_flag ==1){
		           WriteRegister(KME_DEHALO_KME_DEHALO_F0_reg, 24, 31, 0x0F); 
			     WriteRegister(KME_DEHALO_KME_DEHALO_F4_reg, 24, 31, 0xFF);
			     WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x0F); 
			     WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0xFF);
		}
		else	{
		           WriteRegister(KME_DEHALO_KME_DEHALO_F0_reg, 24, 31, 0x0F); 
			     WriteRegister(KME_DEHALO_KME_DEHALO_F4_reg, 24, 31, 0xFF);
			     WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, 0x0F); 
			     WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, 0xFF);
		}
		

		u5_blend_holdfrm_a[12]--;
	}
	//else if(((nIdentifiedNum_a[12] == 115 )||(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_Rassia_ID_flag ==0))&&(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag!=0)) //ori
	else if(nIdentifiedNum_a[12] == 115 ) //YE Test 20200803 new remove Rassia to avoid value be locked
	{

				//WriteRegister(MC_MC_28_reg, 14, 14,0x0);
				//WriteRegister(KMC_BI_bi_top_01_reg,2,3,0x1);

				#if 0 //YE Test 20200804 x1/x2/y1/y2 now all be controled by auto, don't need to return value back				
		       	WriteRegister(KME_DEHALO_KME_DEHALO_F0_reg, 24, 31, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x1); 
				WriteRegister(KME_DEHALO_KME_DEHALO_F4_reg, 24, 31, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x2);
				WriteRegister(KME_DEHALO_KME_DEHALO_F8_reg, 24, 31, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_y1); 
			      WriteRegister(KME_DEHALO_KME_DEHALO_FC_reg, 0, 7, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_y2);
				#endif
		nIdentifiedNum_a[12] = 0;
	}
	// -- YE Test move edge shaking apply from FBlevelCtrl.c here

#endif

	#if 1
	// ++ YE Test 035 girl repeat pattern
	if((s_pContext->_output_me_sceneAnalysis.u1_RP_035_flag==1)&&(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag!=1))
	{
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg,24,31,0x2d);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,5,5,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,6,6,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,7,7,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,8,8,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,9,9,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,10,10,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,12,12,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,13,13,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,14,14,0x0);

		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg,24,31,0x2d);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,4,4,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,5,5,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,6,6,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,7,7,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,8,8,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,9,9,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,11,11,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,12,12,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,13,13,0x0);

		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_2C_reg,21,25,0x1);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,18,23,0x0);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,8,10,0x7);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,11,13,0x7);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,20,22,0x1);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,23,25,0x7);

		u5_blend_holdfrm_a[13] = 48;


		nIdentifiedNum_a[13]=116 ;
	}
	else if(u5_blend_holdfrm_a[13] >0)
	{
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg,24,31,0x2d);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,5,5,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,6,6,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,7,7,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,8,8,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,9,9,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,10,10,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,12,12,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,13,13,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,14,14,0x0);

		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg,24,31,0x2d);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,4,4,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,5,5,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,6,6,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,7,7,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,8,8,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,9,9,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,11,11,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,12,12,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,13,13,0x0);

		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_2C_reg,21,25,0x1);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,18,23,0x0);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,8,10,0x7);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,11,13,0x7);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,20,22,0x1);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,23,25,0x7);

		
			u5_blend_holdfrm_a[13]--;

	}
	else if(nIdentifiedNum_a[13] == 116)
	{

		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg,24,31,0x28);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,5,5,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,6,6,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,7,7,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,8,8,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,9,9,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,10,10,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,12,12,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,13,13,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,14,14,0x1);

		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg,24,31,0x28);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,4,4,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,5,5,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,6,6,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,7,7,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,8,8,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,9,9,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,11,11,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,12,12,0x1);
		//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,13,13,0x1);

		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_2C_reg,21,25,0x3);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,18,23,0x1);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,8,10,0x0);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,11,13,0x0);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,20,22,0x0);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,23,25,0x0);

		nIdentifiedNum_a[13] = 0;
		
	}
	// -- YE Test 035 girl repeat pattern
	#endif
	
	#if 1
	// ++ YE Test 053 repeat pattern
	if(s_pContext->_output_me_sceneAnalysis.u1_RP_053_flag==1)
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,0,3,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,4,7,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,8,11,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,0,3,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,0,3,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,4,7,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,8,11,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x3);

		u5_blend_holdfrm_a[14] = 48;


		nIdentifiedNum_a[14]=117 ;
	}
	else if(u5_blend_holdfrm_a[14] >0)
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,0,3,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,4,7,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,8,11,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,0,3,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,0,3,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,4,7,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,8,11,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x3);
		
			u5_blend_holdfrm_a[14]--;

	}
	else if(nIdentifiedNum_a[14] == 117)
	{


		nIdentifiedNum_a[14] = 0;
	}
	// -- YE Test 53 repeat pattern
	#endif


	#if 1
	// ++ YE Test 062 repeat pattern
	if(((s_pContext->_output_me_sceneAnalysis.u1_RP_062_1_flag==1)
		||(s_pContext->_output_me_sceneAnalysis.u1_RP_062_2_flag==1))
		&&(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag!=1))
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,0,3,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,4,7,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,8,11,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,0,3,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,0,3,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,4,7,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,8,11,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x1);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,12,12,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,13,13,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,14,14,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,11,11,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,12,12,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,13,13,0x0);

		
		u5_blend_holdfrm_a[15] = 48;

		nIdentifiedNum_a[15]=118 ;
	}
	else if(u5_blend_holdfrm_a[15] >0)
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,0,3,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,4,7,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,8,11,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,12,15,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,16,19,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,20,23,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,24,27,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg,28,31,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,0,3,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,0,3,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,4,7,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,8,11,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,12,15,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,16,19,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,20,23,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,24,27,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg,28,31,0x1);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,12,12,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,13,13,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,14,14,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,11,11,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,12,12,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,13,13,0x0);
		
			u5_blend_holdfrm_a[15]--;

	}
	else if(nIdentifiedNum_a[15] == 118)
	{
		nIdentifiedNum_a[15] = 0;
	}
	// -- YE Test 62 repeat pattern
	#endif


	#if 1
	// ++ YE Test 095 repeat pattern
	if(s_pContext->_output_me_sceneAnalysis.u1_RP_095_flag==1)
	{
		
		//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,11,11,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,12,12,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,13,13,0x0);

		//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,10,10,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,11,11,0x0);
		


		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_2C_reg,21,25,0x7);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,18,23,0x4);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,8,10,0x7);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,11,13,0x7);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,20,22,0x1);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,23,25,0x7);

		u5_blend_holdfrm_a[16] = 48;

		nIdentifiedNum_a[16]=118 ;
	}
	else if(u5_blend_holdfrm_a[16] >0)
	{
	
		//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,11,11,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,12,12,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,13,13,0x0);
	
		//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,10,10,0x0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,11,11,0x0);
		

		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_2C_reg,21,25,0x7);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,18,23,0x4);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,8,10,0x7);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,11,13,0x7);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,20,22,0x1);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,23,25,0x7);

		
			u5_blend_holdfrm_a[16]--;

	}
	else if(nIdentifiedNum_a[16] == 118)
	{
		/*
		//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,11,11,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,12,12,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,13,13,0x1);

		//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,10,10,0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,11,11,0x1);
		*/


		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_2C_reg,21,25,0x3);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,18,23,0x1);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,8,10,0x0);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,11,13,0x0);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,20,22,0x0);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,23,25,0x0);

		nIdentifiedNum_a[16] = 0;
	}
	// -- YE Test 95 repeat pattern
	#endif

	if((scalerVIP_Get_MEMCPatternFlag_Identification(115) == TRUE) && (s_pContext->_output_me_sceneAnalysis.u1_BTS_DRP_detect_true == 1 )&&((in_fmRate == _PQL_IN_30HZ)||((in_fmRate == _PQL_IN_60HZ)&&( u8_cadence_Id == _CAD_22))))
	{	
		//me1 ip/pi mask
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 	31,	0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 	31,	0x11111111);
		//me2 dediff ph en
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,1,1, 0);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_84_reg,1,1, 0);

		//== zmv ==
		//me1
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 16, 16,	0);//ip_en
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 15, 15,	0);//pi_en		
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_28_reg, 16, 28, 0);//ip_pnt
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 16, 28, 0);//pi_pnt
		//me2
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg, 16, 16, 0);//1st
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg, 16, 16, 0);	//2nd		
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg, 24, 31, 0);//1st		
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_9C_reg, 0, 7, 0);//2nd
		
		//==ip/pi cand==
		//ip
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 2, 2,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 3, 3,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 6, 6,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 7, 7,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 8, 8,	0);
		//pi
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 2, 2,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 6, 6,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 7, 7,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 8, 8,	0);
				
		//== update cand ==
		//me1 ip
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,  13, 13,  0);
		//me1 pi
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,  12, 12,  0);
		//me2 1st
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,  18, 23, 0);
		//me2 2nd
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg,  21, 26,  0);

		u5_blend_holdfrm_a[17] = 10;
		nIdentifiedNum_a[17]=115;
	}
	else if( u5_blend_holdfrm_a[17] > 0 )
	{		
		//me1 ip/pi mask
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 	31,	0x11111111);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 	31,	0x11111111);
		//me2 dediff ph en
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,1,1, 0);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_84_reg,1,1, 0);

		//== zmv ==
		//me1
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 16, 16,	0);//ip_en
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 15, 15,	0);//pi_en		
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_28_reg, 16, 28, 0);//ip_pnt
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 16, 28, 0);//pi_pnt
		//me2
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg, 16, 16, 0);//1st
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg, 16, 16, 0);	//2nd		
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg, 24, 31, 0);//1st		
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_9C_reg, 0, 7, 0);//2nd
		
		//==ip/pi cand==
		//ip
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 2, 2,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 3, 3,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 6, 6,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 7, 7,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 8, 8,	0);
		//pi
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 2, 2,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 6, 6,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 7, 7,	0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 8, 8,	0);
				
		//== update cand ==
		//me1 ip
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,  13, 13,  0);
		//me1 pi
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,  12, 12,  0);
		//me2 1st
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,  18, 23, 0);
		//me2 2nd
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg,  21, 26,  0);
		
		u5_blend_holdfrm_a[17]--;
	}
	else if(nIdentifiedNum_a[17]==115)
	{
		//me2 dediff ph en
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,1,1, 1);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_84_reg,1,1, 1);

		//== zmv ==
		//me1
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 16, 16,	0);//ip_en
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 15, 15,	0);//pi_en		
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_28_reg, 16, 28, 1023);//ip_pnt
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 16, 28, 1023);//pi_pnt
		//me2
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg, 16, 16, 0);//1st
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg, 16, 16, 0);	//2nd		
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg, 24, 31, 96);//1st pnt		
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_9C_reg, 0, 7, 96);//2nd pnt
		
		//==ip/pi cand==
		//ip
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 2, 2,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 3, 3,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 6, 6,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 7, 7,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 8, 8,	1);
		//pi
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 2, 2,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 6, 6,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 7, 7,	1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 8, 8,	1);
				
		//== update cand ==
		//me1 ip
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg,  13, 13,  1);
		//me1 pi
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg,  12, 12,  1);
		//me2 1st
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,  18, 23, 2);
		//me2 2nd
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg,  21, 26,  2);
		
		nIdentifiedNum_a[17]=0;
	}

#if	0 //zebra_24fps	
	if(scalerVIP_Get_MEMCPatternFlag_Identification(118) == TRUE){
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 16, 16, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 15, 15, 1);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_28_reg, 16, 28, 0);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 16, 28, 0);
		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 22, 22, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 23, 23, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 18, 19, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 20, 21, 0);
		
		WriteRegister(KME_DEHALO2_KME_DEHALO2_90_reg, 0, 31, 0x0000f777);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 0, 31, 0x00001fff);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 0, 31, 0x00111fff);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 0, 31, 0x1fff1fff);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 0, 31, 0x1fff1fff);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 0, 31, 0x1fff1fff);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 0, 31, 0x1fff1fff);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 0, 31, 0x1fff1fff);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 0, 31, 0x1fff1fff);
		u5_blend_holdfrm_a[18] = 16;
		nIdentifiedNum_a[18]=118;
	}else if(u5_blend_holdfrm_a[18] >0){
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 16, 16, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 15, 15, 1);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_28_reg, 16, 28, 0);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 16, 28, 0);
		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 22, 22, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 23, 23, 1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 18, 19, 0);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 20, 21, 0);
		
		WriteRegister(KME_DEHALO2_KME_DEHALO2_90_reg, 0, 31, 0x0000f777);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 0, 31, 0x00001fff);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 0, 31, 0x00111fff);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 0, 31, 0x1fff1fff);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 0, 31, 0x1fff1fff);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 0, 31, 0x1fff1fff);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 0, 31, 0x1fff1fff);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 0, 31, 0x1fff1fff);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 0, 31, 0x1fff1fff);
		u5_blend_holdfrm_a[18]--;
	}else if(nIdentifiedNum_a[18] == 118){
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 16, 16, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_zmv_en);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 15, 15,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_zmv_en);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_28_reg, 16, 28,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_zmv);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 16, 28,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_zmv);
		
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 22, 22,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_dc_obme_mode_sel);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 23, 23,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_dc_obme_mode_sel);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 18, 19,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_dc_obme_mode);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 20, 21,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_dc_obme_mode);

		
		WriteRegister(KME_DEHALO2_KME_DEHALO2_90_reg, 0, 31,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dehalo2_90);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 0, 31,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dehalo2_A8);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 0, 31,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dehalo2_E8);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 0, 31,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dehalo2_14);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 0, 31,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dehalo2_18);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 0, 31,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dehalo2_1C);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 0, 31,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dehalo2_20);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 0, 31,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dehalo2_24);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 0, 31,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_dehalo2_28);
		
		nIdentifiedNum_a[18] = 0;
	}
#endif
	#if 0 //Pro_TCL
	if(scalerVIP_Get_MEMCPatternFlag_Identification(0) == TRUE && s_pContext->_output_me_vst_ctrl.u1_detect_blackBG_VST_trure ==0  &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{


		u5_blend_holdfrm_a[0] = 8;
		nIdentifiedNum_a[0]=0;

	}
	else if(u5_blend_holdfrm_a[0] >0)
	{


		u5_blend_holdfrm_a[0]--;
	}
	else if(nIdentifiedNum_a[0] == 0 )
	{

		nIdentifiedNum_a[0] = 0;
	}

	//===========================================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(12) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(MC_MC_B8_reg, 12, 15, 0xa);//0x2
		WriteRegister(MC_MC_BC_reg, 16, 19, 0xa);//0x0

		u5_blend_holdfrm_a[12] = 16;
		nIdentifiedNum_a[12]=12;

	}
	else if(u5_blend_holdfrm_a[12] >0)
	{
		WriteRegister(MC_MC_B8_reg, 12, 15, 0xa);//0x2
		WriteRegister(MC_MC_BC_reg, 16, 19, 0xa);//0x0
		u5_blend_holdfrm_a[12]--;
	}
	else if(nIdentifiedNum_a[12] == 12 )
	{
		WriteRegister(MC_MC_B8_reg, 12, 15, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_mc_sobj_ymin0);
		WriteRegister(MC_MC_BC_reg, 16, 19, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_mc_sobj_ymin1);
		nIdentifiedNum_a[12] = 0;
	}
	//==========================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(18) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 6, 8, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 12, 12, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 22, 24, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 28, 28, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 6, 8, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 12, 12, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 22, 24, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 28, 28, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 6, 8, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 12, 12, 1);//0x0

		WriteRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 0, 0, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 7, 9, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 6, 8, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 12, 12, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 6, 8, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 12, 12, 1);//0x0

		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 6, 8, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 12, 12, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 22, 24, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 28, 28, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 6, 8, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 12, 12, 1);//0x0

		ReadRegister(IPPRE_IPPRE_04_reg, 4, 11, &u32_RB_Value);
		WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, (u32_RB_Value+nblend_y_alpha_Target)/2);
		ReadRegister(IPPRE_IPPRE_04_reg, 16, 23, &u32_RB_Value);
		WriteRegister(IPPRE_IPPRE_04_reg, 16, 23, (u32_RB_Value+nblend_uv_alpha_Target)/2);
		ReadRegister(IPPRE_IPPRE_0C_reg, 0, 7, &u32_RB_Value);
		WriteRegister(IPPRE_IPPRE_0C_reg, 0, 7, (u32_RB_Value+nblend_logo_y_alpha_Target)/2);
		ReadRegister(IPPRE_IPPRE_0C_reg, 9, 16, &u32_RB_Value);
		WriteRegister(IPPRE_IPPRE_0C_reg, 9, 16, (u32_RB_Value+nblend_logo_uv_alpha_Target)/2);


		u5_blend_holdfrm_a[18] = 16;
		nIdentifiedNum_a[18]=18;

	}
	else if(u5_blend_holdfrm_a[18] >0)
	{
		WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 6, 8, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 12, 12, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 22, 24, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 28, 28, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 6, 8, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 12, 12, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 22, 24, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 28, 28, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 6, 8, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 12, 12, 1);//0x0

		WriteRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 0, 0, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 7, 9, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 6, 8, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 12, 12, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 6, 8, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 12, 12, 1);//0x0

		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 6, 8, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 12, 12, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 22, 24, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 28, 28, 1);//0x0
		WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 6, 8, 7);//0x4
		WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 12, 12, 1);//0x0

		ReadRegister(IPPRE_IPPRE_04_reg, 4, 11, &u32_RB_Value);
		WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, (u32_RB_Value+nblend_y_alpha_Target)/2);
		ReadRegister(IPPRE_IPPRE_04_reg, 16, 23, &u32_RB_Value);
		WriteRegister(IPPRE_IPPRE_04_reg, 16, 23, (u32_RB_Value+nblend_uv_alpha_Target)/2);
		ReadRegister(IPPRE_IPPRE_0C_reg, 0, 7, &u32_RB_Value);
		WriteRegister(IPPRE_IPPRE_0C_reg, 0, 7, (u32_RB_Value+nblend_logo_y_alpha_Target)/2);
		ReadRegister(IPPRE_IPPRE_0C_reg, 9, 16, &u32_RB_Value);
		WriteRegister(IPPRE_IPPRE_0C_reg, 9, 16, (u32_RB_Value+nblend_logo_uv_alpha_Target)/2);

		u5_blend_holdfrm_a[18]--;
	}
	else if(nIdentifiedNum_a[18] == 18 )
	{
		WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 6, 8, 0x4);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 12, 12,0x0);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 22, 24, 0x4);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 28, 28, 0x0);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 6, 8, 0x4);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 12, 12, 0x0);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 22, 24, 0x4);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 28, 28, 0x0);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 6, 8, 0x4);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 12, 12, 0x0);

		WriteRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 0, 0, 0x0);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 7, 9, 0x4);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 6, 8, 0x4);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 12, 12, 0x0);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 6, 8, 0x4);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 12, 12, 0x0);

		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 6, 8, 0x4);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 12, 12, 0x0);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 22, 24, 0x4);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 28, 28, 0x0);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 6, 8, 0x4);
		WriteRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 12, 12, 0x0);


		WriteRegister(IPPRE_IPPRE_04_reg, 4, 11,s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha);
		WriteRegister(IPPRE_IPPRE_04_reg, 16, 23, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_uv_alpha);
		WriteRegister(IPPRE_IPPRE_0C_reg, 0, 7, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_logo_y_alpha);
		WriteRegister(IPPRE_IPPRE_0C_reg, 9, 16, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_kmc_blend_logo_uv_alpha);

		nIdentifiedNum_a[18] = 0;
	}
	//==========================
	if(scalerVIP_Get_MEMCPatternFlag_Identification(19) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		//  BG repeat detect
		WriteRegister(HARDWARE_HARDWARE_58_reg, 11, 11, 1);
		//gmvd
		pOutput->u1_casino_RP_detect_true =1;

		//  lfsr_mask
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 19, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 3);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 19, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, 3);
		// ME1 cddpnt rnd
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_24_reg, 16, 23, 0xff);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_24_reg, 24, 31, 0xff);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_28_reg, 0, 7, 0xff);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_64_reg, 16, 23, 0xff);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_64_reg, 24, 31, 0xff);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 0, 7, 0xff);
		// ME1 adptpnt rnd curve
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 0, 5, 0x3f);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 8, 13, 0x3f);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 14, 19,0x3f);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 20, 25, 0x3f);

		u5_blend_holdfrm_a[19] = 8;
		nIdentifiedNum_a[19]=19;

	}
	else if(u5_blend_holdfrm_a[19] >0)
	{
		//  BG repeat detect
		WriteRegister(HARDWARE_HARDWARE_58_reg, 11, 11, 1);
		//gmvd
		pOutput->u1_casino_RP_detect_true =1;

		//  lfsr_mask
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 19, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, 3);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 19, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, 3);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, 3);
		// ME1 cddpnt rnd
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_24_reg, 16, 23, 0xff);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_24_reg, 24, 31, 0xff);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_28_reg, 0, 7, 0xff);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_64_reg, 16, 23, 0xff);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_64_reg, 24, 31, 0xff);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 0, 7, 0xff);
		// ME1 adptpnt rnd curve
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 0, 5, 0x3f);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 8, 13, 0x3f);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 14, 19,0x3f);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 20, 25, 0x3f);

		u5_blend_holdfrm_a[19]--;
	}
	else if(nIdentifiedNum_a[19] == 19 )
	{
		//  BG repeat detect
		WriteRegister(HARDWARE_HARDWARE_58_reg, 11, 11, 0);
		//gmvd
		pOutput->u1_casino_RP_detect_true =0;
		//  lfsr_mask
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask0_x);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask0_y);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask1_x);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask1_y);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 19, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask2_x);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask2_y);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask3_x);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask3_y);

		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask0_x);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask0_y);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask1_x);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask1_y);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 19, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask2_x);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask2_y);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask3_x);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask3_y);

		// ME1 cddpnt rnd
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_24_reg, 16, 23, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_rnd0);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_24_reg, 24, 31, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_rnd1);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_28_reg, 0, 7,   s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_rnd2);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_64_reg, 16, 23, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_rnd0);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_64_reg, 24, 31, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_rnd1);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 0, 7,   s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_rnd2);
		// ME1 adptpnt rnd curve
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 0, 5,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_adptpnt_rnd_y1);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 8, 13,  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_adptpnt_rnd_y2);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 14, 19, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_adptpnt_rnd_y3);
		WriteRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 20, 25, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_adptpnt_rnd_y4);

		nIdentifiedNum_a[19] = 0;
	}
	if(scalerVIP_Get_MEMCPatternFlag_Identification(30) == TRUE &&( vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000 ))
	{
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_a[30] = 8;
		nIdentifiedNum_a[30]=30;

	}
	else if(u5_blend_holdfrm_a[30] >0)
	{
		rtd_clearbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		u5_blend_holdfrm_a[30]--;
	}
	else if(nIdentifiedNum_a[30] == 30 )
	{
		rtd_setbits(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, _BIT0);//  Multiband peaking
		rtd_setbits(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, _BIT0);//  Multiband peaking DB apply
		nIdentifiedNum_a[30] = 0;
	}
	//===================================================
	//===================================================
	#endif

	//=======================
	//===debug show
	for(u8_Index =0;u8_Index<18;u8_Index++)
	{
		if(nIdentifiedNum_a[u8_Index] !=0){
			pOutput->u1_IP_wrtAction_true=1;
			if(u32_ID_Log_en==1)
				rtd_pr_memc_emerg("[IPR_001]nIdentifiedNum_a>>[%d][%d]   ,\n", u8_Index, nIdentifiedNum_a[u8_Index]);
		}
	}
	//===


}
#endif
