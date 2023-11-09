#ifndef _DH_CLOSE_DEF_H
#define _DH_CLOSE_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	unsigned char  u2_dhClose_bypass;	
	unsigned char  u1_fblvl_en;
	unsigned char  u1_meAllDtl_en;
	unsigned char  u8_dhOff_holdFrm;

	unsigned char  u8_fblvl_holdFrm;
	unsigned char  u8_fbLvl_Th0;
	unsigned char  u8_fbLvl_Th1;

	unsigned int u25_meAllDtl_Th0;
	unsigned int u25_meAllDtl_Th1;

	unsigned char  u1_dh_pfvconf_en;
	unsigned int u20_dh_pfvconf_thrL;
	unsigned char  u6_dh_pfvconf_cnt_thr;
	unsigned char  u5_dh_pfvconf_holdfrm;

	unsigned char  u1_lbmc_switch_en;
	unsigned char  u5_lbmc_switch_holdfrm;
	unsigned char  u1_panning_en;
	unsigned char  u5_panning_holdfrm;
	unsigned char  u1_dh_fadeInOut_en;
	unsigned char  u5_dh_fadeInOut_holdfrm;

	unsigned char  u1_dh_sml_rgnAction_en;
	unsigned char  u8_dh_sml_rgnAction_gmv_x_thr;
	unsigned char  u8_dh_sml_rgnAction_gmv_y_thr;
	unsigned int u12_dh_sml_rgnAction_gmv_cnt_thr;
	unsigned int u12_dh_sml_rgnAction_gmv_ucof_thr;
	unsigned int u10_dh_sml_rgnAction_badrgnsad_thr;
	unsigned int u10_dh_sml_rgnAction_goodrgnsad_thr;
	unsigned int u19_dh_sml_rgnAction_bad_pfvdiff_thr;
	unsigned int u19_dh_sml_rgnAction_good_pfvdiff_thr;
	unsigned char  u5_dh_sml_rgnAction_holdfrm;
	unsigned char  u8_dh_sml_rgnAction_apl_thr;
	unsigned char  u5_dh_sml_rgnAction_goodrgn_cnt_thr;
	unsigned char  u5_dh_sml_rgnAction_badrgn_cnt_thr;

	unsigned int u12_dh_sml_rgnAction_bad_gmv_cnt_thr;
	unsigned int u12_dh_sml_rgnAction_bad_gmv_ucof_thr;

	unsigned char  u1_simple_scene_en;

	unsigned char  u1_pfv_cdd_num_en;
	unsigned char  u5_phfv0_candi_num;
	unsigned char  u5_phfv1_candi_num;
	unsigned short u9_pfv_cdd_num_dtl_thr;
	
	unsigned char  u2_dh_me1_ip_dc_obme_mode_off;
	unsigned char  u2_dh_me1_pi_dc_obme_mode_off;
	unsigned char  u2_dh_me1_ip_ac_obme_mode_off;
	unsigned char  u2_dh_me1_pi_ac_obme_mode_off;

	unsigned char  u2_dh_me1_ip_dc_obme_mode_on;
	unsigned char  u2_dh_me1_pi_dc_obme_mode_on;
	unsigned char  u2_dh_me1_ip_ac_obme_mode_on;
	unsigned char  u2_dh_me1_pi_ac_obme_mode_on;
	unsigned char  u1_dbg_print_en;
}_PARAM_DH_CLOSE;

typedef struct
{
	unsigned char  u8_fblvl_holdFrm;
	unsigned char  u8_dhOff_holdFrm;
	unsigned char  u8_pfvconf_holdFrm;
	unsigned char  u8_lbmc_mode;
	unsigned char  u8_lbmc_switch_holdfrm;
//	unsigned char  u8_panning_mode;
	unsigned char  u8_panning_holdfrm;
	unsigned char  u1_panning_close;

	unsigned char  u1_fblvl_1frm_close;
	unsigned char  u1_fblvl_close;
	unsigned char  u1_meAllDtl_close;
	unsigned char  u1_pfvconf_close;
	unsigned char  u1_lbmc_switch_close;

	unsigned char  u1_closeDh_sig;
	unsigned char  u1_closeDh_act;
	unsigned char  u1_dh_fadeInOut_close;
	unsigned char  u5_dh_fadeInOut_holdfrm;

	unsigned char  u1_dh_simple_scene_close;

	unsigned char  u1_dh_sml_rgnAction_close;
	unsigned char  u5_dh_sml_rgnAction_holdfrm;

	unsigned char  u5_dh_good_rgn_num;
	unsigned char  u5_dh_bad_rgn_num; 

}_OUTPUT_DH_CLOSE;

VOID Dh_close_Init(_OUTPUT_DH_CLOSE *pOutput);
VOID Dh_close_Proc(const _PARAM_DH_CLOSE *pParam, _OUTPUT_DH_CLOSE *pOutput);
VOID Dh_close_sml_rgnAction(const _PARAM_DH_CLOSE *pParam, _OUTPUT_DH_CLOSE *pOutput);

/////////////////////////////////////////////////////////////////////////////
DEBUGMODULE_BEGIN(_PARAM_DH_CLOSE,_OUTPUT_DH_CLOSE)
	ADD_PARAM_DES(unsigned char,  u2_dhClose_bypass,        0, "0:PQL, 1: force ON, 2, force OFF")
	ADD_PARAM_DES(unsigned char,  u1_fblvl_en,              1, "")
	ADD_PARAM_DES(unsigned char,  u1_meAllDtl_en,           0, "")
	ADD_PARAM_DES(unsigned char,  u8_dhOff_holdFrm,         0, "extra off hold time, minimum is 0")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned char,  u8_fblvl_holdFrm,         1, "fblvl off hold time")
	ADD_PARAM_DES(unsigned char,  u8_fbLvl_Th0,           160, ">=, dh on2off")
	ADD_PARAM_DES(unsigned char,  u8_fbLvl_Th1,            80, "<=, dh off2on")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned int, u25_meAllDtl_Th0,    100000, "<=, dh on2off")
	ADD_PARAM_DES(unsigned int, u25_meAllDtl_Th1,  10000000, ">=, dh off2on")

	////////////  OUTPUT  //////////////////////////////////////////////////////////////

	ADD_OUTPUT(unsigned char,  u8_fblvl_holdFrm)

	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT(unsigned char,  u1_fblvl_close)
	ADD_OUTPUT(unsigned char,  u1_meAllDtl_close)
	ADD_OUTPUT(unsigned char,  u1_closeDh_sig)
	ADD_OUTPUT(unsigned char,  u1_closeDh_act)

DEBUGMODULE_END

#ifdef __cplusplus
}
#endif

#endif