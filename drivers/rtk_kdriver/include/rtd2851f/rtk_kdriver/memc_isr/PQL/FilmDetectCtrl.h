#ifndef _FILMDETECTCTRL_DEF_H_
#define _FILMDETECTCTRL_DEF_H_
#define	KEEP_FRAME_NUM	60
typedef enum
{
	_FILM_ALL,
	_FILM_5RGN_TOP,
	_FILM_5RGN_BOT,
	_FILM_5RGN_LFT,
	_FILM_5RGN_RHT,
	_FILM_5RGN_CEN,
	_FILM_12RGN_01,
	_FILM_12RGN_02,
	_FILM_12RGN_03,
	_FILM_12RGN_04,
	_FILM_12RGN_05,
	_FILM_12RGN_06,
	_FILM_12RGN_07,
	_FILM_12RGN_08,
	_FILM_12RGN_09,
	_FILM_12RGN_10,
	_FILM_12RGN_11,
	_FILM_12RGN_12,

	_FILM_MOT_NUM,

}_PQL_FILM_MOT_TYPE;

//////////////////////////////////////////////////////////////////////////
typedef struct  
{
	unsigned char   u1_filmDet_bypass;
	unsigned int  u32_filmDet_cadence_en;

	unsigned char   u4_Mot_rgn_diffWgt;
	unsigned char   u4_Mot_all_diffWgt;
	unsigned int  u27_Mot_all_min;

	unsigned char   u4_enter_22_cntGain;
	unsigned char   u4_enter_32_cntGain;
	unsigned char   u4_enter_else_cntGain;

	unsigned char  u1_quit_motWgt_auto_en;
	unsigned char  u4_quit_bigMot_wgt;
	unsigned char  u4_quit_smlMot_wgt;
	unsigned char  u8_quit_cnt_th;
	unsigned char  u8_quit_prd_th;

	unsigned char  u1_mixMode_en;
	unsigned char  u1_mixMode_det_en;
	unsigned char  u1_mixMode_rgnEn[_FILM_MOT_NUM -1];
	unsigned char  u8_mixMode_enter_cntTh;
	unsigned char  u8_mixMode_cnt_max;
	unsigned int u27_mixMode_rgnMove_minMot;
	unsigned char  u4_mixMode_rgnMove_motWgt; // u1.3

	unsigned char u1_FrcCadSupport_En;
	unsigned char u1_QuickSwitch_En;
	unsigned char u1_DbgPrt_FRChg_En;
	unsigned char u1_StillDetect_En;
	unsigned char u3_DbgPrt_Rgn;
	unsigned char  u1_BadEditSlowOut_En;
	
	unsigned char  u5_dbg_param1;
	unsigned int u32_dbg_param2;
	unsigned int u32_dbg_cnt;

}_PARAM_FilmDetectCtrl;

typedef struct  
{
	unsigned int u27_ipme_motionPool[_FILM_MOT_NUM][8]; // pool length = 8.
	unsigned int u32_Mot_sequence[_FILM_MOT_NUM];
	unsigned int u32_Mot_sequence_for_mixmode; //only Film_all
	unsigned char  u1_Mot_selBig;

	/////
	unsigned int u27_quit_bigMot[_FILM_MOT_NUM];
	unsigned int u27_quit_smlMot[_FILM_MOT_NUM];
	unsigned int u27_quit_motTh[_FILM_MOT_NUM];
	unsigned char  u8_quit_cnt[_FILM_MOT_NUM];
	unsigned char  u8_quit_prd[_FILM_MOT_NUM];

	/////
	unsigned char  u8_det_cadence_Id_pre[_FILM_MOT_NUM]; // pre detected cadence according to motion sequence.
	unsigned char  u8_det_cadence_Id[_FILM_MOT_NUM]; // detected cadence according to motion sequence.
	unsigned char  u8_pre_cadence_Id[_FILM_MOT_NUM]; // previous output cadence.
	unsigned char  u8_cur_cadence_Id[_FILM_MOT_NUM]; // current output cadence.
	unsigned char  u8_phase_Idx_tmp[_FILM_MOT_NUM];
	unsigned char  u8_phase_Idx[_FILM_MOT_NUM];
	unsigned char  u8_pre_phase_Idx[_FILM_MOT_NUM];

	/////
	unsigned char  u1_cad_resync_true[_FILM_MOT_NUM];
	unsigned char  u1_cad_fastOut_true;	
	unsigned char  u8_mixMode_cnt;
	unsigned char  u1_mixMode;
	unsigned char  u1_mixMode_chg;

	///
	unsigned int u27_FrmMotion_S[_FILM_MOT_NUM];

	/////
	unsigned int u5_dbg_param1_pre;
	unsigned int u32_dbg_param2_pre;
	unsigned int u32_dbg_cnt_hold;
	unsigned char u8_dbg_motion_t;
	/////
	unsigned char  u8_phT_phase_Idx[_FILM_MOT_NUM];

	/////
	unsigned int u27_FrmMotionSort[26];
	unsigned int u27_FrmMotionSortTick[26];

	/////
	unsigned int u27_g_FrmMot_Th[_FILM_MOT_NUM];

	//--output to other module--
	unsigned char  u8_cur_cadence_out[_FILM_MOT_NUM]; // current output cadence.
	unsigned char  u8_phT_phase_Idx_out[_FILM_MOT_NUM];
	unsigned char  u8_flbk_lvl_cad_cnt;
	//--end of "output to other module"

	/////
	unsigned int u32_FilmFrame_cnt[_FILM_MOT_NUM];
	unsigned int u27_FrameMotionPool[60]; // pool length = _FILM_MOT_NUM.

	unsigned char  u1_32322_BadEdit_flag;
	unsigned char  u8_32322_BadEdit_cnt;
	unsigned char  u8_32322_BadEdit_idx;
	
	unsigned char  u1_32322_BadEdit_flag2;
	unsigned char  u8_32322_BadEdit_cnt2;
	unsigned char  u8_32322_BadEdit_idx2;
	
	unsigned char  u1_Special_60_BadEdit_flag;
	unsigned char  u8_Special_60_BadEdit_cnt;
	unsigned char  u8_Special_60_BadEdit_idx;

	unsigned char  u1_Special_50_BadEdit_flag;
	unsigned char  u8_Special_50_BadEdit_cnt;
	unsigned char  u8_Special_50_BadEdit_idx;

	unsigned char  u1_Special_30_BadEdit_flag;
	unsigned char  u8_Special_30_BadEdit_cnt;
	unsigned char  u8_Special_30_BadEdit_idx;
	
	unsigned char u8_BadEdit_flag;
	unsigned int  u32_BadEdit_cnt;
	unsigned char u8_cadence_sequence[KEEP_FRAME_NUM];
	bool ExchangeCad;
}_OUTPUT_FilmDetectCtrl;

VOID FilmDetect_Init(_OUTPUT_FilmDetectCtrl *pOutput);
VOID FilmDetectCtrl_Proc(const _PARAM_FilmDetectCtrl *pParam, _OUTPUT_FilmDetectCtrl *pOutput);
VOID PowerSaving_Proc(unsigned char type);
VOID FilmDetect_PowerSavingJudge(const _PARAM_FilmDetectCtrl *pParam, _OUTPUT_FilmDetectCtrl *pOutput);

//////////////////////////////////////////////////////////////////////////
DEBUGMODULE_BEGIN(_PARAM_FilmDetectCtrl,_OUTPUT_FilmDetectCtrl)
	ADD_PARAM_DES(unsigned char,   u1_filmDet_bypass,         1, "1, force cadId = VIDEO")
	ADD_PARAM_DES(unsigned int,  u32_filmDet_cadence_en, 8191, "1 bit for 1 cadence, ordering as cadId. video/22/32/...")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned char,   u4_Mot_rgn_diffWgt,    8, "u1.3, sequence gen, motion Th gain")
	ADD_PARAM_DES(unsigned char,   u4_Mot_all_diffWgt,    8, "u1.3")
	ADD_PARAM_DES(unsigned int,  u27_Mot_all_min,    4096, "for motion seq = 1, motion >= minMot. only for Mot_all")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned char,   u4_enter_22_cntGain,   2, "u3.1, enter_cntTh = cad_outLen * gain >> 1")
	ADD_PARAM_DES(unsigned char,   u4_enter_32_cntGain,   2, "u3.1")
	ADD_PARAM_DES(unsigned char,   u4_enter_else_cntGain, 1, "u3.1, except 11'23, others cadence enter_cntTh")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned char,   u1_quit_motWgt_auto_en, 0, "1, bigMot/smlMot auto IIR Alpha based on pre & cur Motion")
	ADD_PARAM_DES(unsigned char,   u4_quit_bigMot_wgt,     5, "u1.3, motTh = (bigMot * wgt + smlMot * wgt)>>3.")
	ADD_PARAM_DES(unsigned char,   u4_quit_smlMot_wgt,     3, "")
	ADD_PARAM_DES(unsigned char,   u8_quit_cnt_th,         4, "")
	ADD_PARAM_DES(unsigned char,   u8_quit_prd_th,         2, "")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned char,   u1_mixMode_det_en,          0, "1, detect mixMode")
	ADD_PARAM_ARRAY_DES(unsigned char , u1_mixMode_rgnEn, _FILM_MOT_NUM-1,   0, "L->M: rgn lft/rht/top/bot/cen")
	ADD_PARAM_DES(unsigned char,   u8_mixMode_enter_cntTh,     0, "")
	ADD_PARAM_DES(unsigned char,   u8_mixMode_cnt_max,         0, "")
	ADD_PARAM_DES(unsigned int,  u27_mixMode_rgnMove_minMot, 0, "")
	ADD_PARAM_DES(unsigned char,   u4_mixMode_rgnMove_motWgt,  0, "u1.3")

	ADD_PARAM_DES(unsigned char,   u1_FrcCadSupport_En,  1, "choose case support")
	ADD_PARAM_DES(unsigned char,   u1_DbgPrt_FRChg_En,  1, "for debug print")
	ADD_PARAM_DES(unsigned char,   u1_StillDetect_En,  1, "for still detect")
	ADD_PARAM_DES(unsigned char,   u3_DbgPrt_Rgn,  1, "choose which rgn dbg print")

	///// output
	ADD_OUTPUT_DES(unsigned char, u8_cur_cadence_Id[_FILM_ALL], "cadence ID")
DEBUGMODULE_END

#endif

