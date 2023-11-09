#ifndef _FRC_PH_TABLE_DEF_
#define _FRC_PH_TABLE_DEF_

#include "memc_isr/PQL/FRC_decPhT_generate.h"

#ifdef CONFIG_ARM64 //ARM32 compatible
//#include "io.h"

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
#else
#include "io.h"
#endif


#if 1//#if RTK_MEMC_Performance_tunging_from_tv001
#include <tvscalercontrol/io/ioregdrv.h>
#endif


//////////////////////////////////////////////////////////////////////////
typedef struct  
{
	unsigned char  u1_phT_autoGen_en;

	unsigned char  u8_dejudder_lvl;
	unsigned char  u8_deblur_lvl;
	unsigned char  u2_candece_id_mode;
	unsigned char  u4_cadence_id;

	unsigned char  in_3dFormat;
	unsigned char  out_3dFormat;
	unsigned char  u2_outMode;

	unsigned char  u1_outLR_LRsamePh;
	unsigned char  u1_filmPh_sw_en;
	unsigned char  u1_filmPh_pf_en;

	unsigned char  u8_input_dly;

	unsigned char u1_Ph_delay_calc_en;

	unsigned char u1_force_gen_table_en;
}_PARAM_FRC_PH_TABLE;

typedef struct  
{
	unsigned char                 u8_system_start_lock;
	unsigned char                 u1_prt_trig;
	unsigned char                 u8_phT_print_en_pre;
	unsigned char                 u8_phT_print_frmCnt0;

	////
	PQL_INPUT_3D_FORMAT   in3d_format_pre;
	PQL_OUTPUT_3D_FORMAT  out3d_format_pre;
	unsigned char                 u8_sys_N_pre;
	unsigned char                 u8_sys_M_pre;
	unsigned char                 u8_cadence_id_pre;
	unsigned char                 u8_film_cadence_id_pre;

	unsigned char                 u8_deblur_lvl_pre;
	unsigned char                 u8_dejudder_lvl_pre;
	unsigned char                 u8_pulldown_mode_pre;

	unsigned char                 u8_filmPh;
	unsigned char                 u8_filmPh0_inPh_pre;
	unsigned char                 u8_filmPh0_cnt;

	unsigned char                 u1_outMode_pre;

	/////
	unsigned char                 u1_generate_PC_en;
	unsigned char                 u1_generate_video_en;
	unsigned char                 u1_generate_film_en;
	unsigned char                 u1_generate_vd_22_32_en;

	unsigned char                 u1_phFlbk_film_en;
	unsigned char                 u1_phFlbk_video_en;
	unsigned char                 u1_phFlbk_22_en;
	unsigned char                 u1_phFlbk_32_en;
	
	unsigned char                 u1_inTable_wrt_en;
	unsigned char                 u1_outTable_wrt_en;
	unsigned char                 u1_filmPh_wrt_en;

	/////
	unsigned short                u16_phTable_wrt_vd_22_32_endIdx;
	unsigned short                u16_phTable_wrt_stIdx;
	unsigned short                u16_phTable_wrt_endIdx_p1;
	unsigned short		    u16_phTable_wrt_vd_endIdx_p1;

	////////
	unsigned int                u32_debug_LSB[2];
	unsigned int                u32_debug_MSB[2];

	unsigned char                 u1_is_inFormat_PureVideo;

	//// after image
	unsigned char                 u1_after_img;
	unsigned char                 u1_after_img_type;
	unsigned char                 u8_after_img_cadence_id_pre;

	unsigned char                 u1_lowdelaystate;
	unsigned char                 u1_UI_GameMode_state;
	unsigned char                 u1_lowdelay_regen;
	unsigned char                 u1_lowdelay_inframe;//k23
	unsigned char                 u1_inframe_pre;
	unsigned char                 u1_outframe_pre;
	unsigned short		     u8_source_type_pre;
	unsigned char			u8_memc_mode_pre;
	unsigned char			u1_VR360_en_pre;
	unsigned char			u1_adaptivestream_flag_pre;
	unsigned char			u1_direct_media_flag_pre;
	unsigned char			u1_vdec_direct_lowdelay_flag_pre;
	unsigned char			u1_timing_status_pre;
	unsigned char			QMS_24hz_state_pre;
	unsigned char			u1_qms_flag_pre;
	unsigned char			QMS_FrmRateChg_hold_cnt;
	unsigned char			QMS_FrmRateChg_hold_cnt_max;
	unsigned char			QMS_FrmRateChg_case_ID;
}_OUTPUT_FRC_PH_TABLE;

VOID FRC_phTable_Init( _OUTPUT_FRC_PH_TABLE *pOutput);
VOID FRC_PhaseTable_Init( _OUTPUT_FRC_PH_TABLE *pOutput);

VOID FRC_phTable_Proc(const _PARAM_FRC_PH_TABLE *pParam, _OUTPUT_FRC_PH_TABLE *pOutput);

VOID FRC_phTable_Delay_Calc(const _PARAM_FRC_PH_TABLE *pParam, _OUTPUT_FRC_PH_TABLE *pOutput);

VOID FRC_phTable_Cadence_Change(const _PARAM_FRC_PH_TABLE *pParam, _OUTPUT_FRC_PH_TABLE *pOutput);

//// 
VOID FRC_phTable_StateMachine(const _PARAM_FRC_PH_TABLE *pParam, _OUTPUT_FRC_PH_TABLE *pOutput);
VOID FRC_phTable_Generate(const _PARAM_FRC_PH_TABLE *pParam, _OUTPUT_FRC_PH_TABLE *pOutput);
VOID FRC_phTable_Write(const _PARAM_FRC_PH_TABLE *pParam, _OUTPUT_FRC_PH_TABLE *pOutput);

//// 
VOID FRC_kphase_sram_clear(unsigned short stIdx, unsigned short endIdx);
VOID FRC_kphase_sram_init(VOID);

//// FRC table write control.
VOID   FRC_phTable_Kphase_ctrl_Wrt(const _PARAM_FRC_PH_TABLE *pParam);
VOID   FRC_phTable_inDec_Wrt(VOID);
VOID   FRC_phTable_outDec_Wrt(const _PARAM_FRC_PH_TABLE *pParam, _OUTPUT_FRC_PH_TABLE *pOutput);

VOID   FRC_phTable_allInfo_Pack_Gen(const _PARAM_FRC_PH_TABLE *pParam, _OUTPUT_FRC_PH_TABLE *pOutput, unsigned short phDecInfo_k, Ph_Dec_Pack_Info *pPackInfo);
unsigned int FRC_phTable_tool_commonDivisor(unsigned int v0, unsigned int v1);
VOID   FRC_phTable_inDec_amend(unsigned char  cadence_id);


VOID FRC_phTable_Init_1n2m_video_22_32_intable(VOID);
VOID   FRC_phTable_inDec_Wrt_init(VOID);

unsigned int FRC_Dejudder_By_Motion_Mapping(unsigned int dejudder_lvl);

/////////////////////////////////////////////////////////////////////////////
DEBUGMODULE_BEGIN(_PARAM_FRC_PH_TABLE,_OUTPUT_FRC_PH_TABLE)
	ADD_PARAM_DES(unsigned char, u1_phT_autoGen_en,     1, "1-en")
	ADD_PARAM_DES(unsigned char, u8_dejudder_lvl,      17, "0: PQL, else: forceVal = reg - 1.")
	ADD_PARAM_DES(unsigned char, u8_deblur_lvl,        17, "0: PQL, else: forceVal = reg - 1.")
	ADD_PARAM_DES(unsigned char, u2_candece_id_mode,    0, "0-HW, 1-force")
	ADD_PARAM_DES(unsigned char, u4_cadence_id,         2, "video/22/32/else")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned char,  in_3dFormat,          0, "0: PQL, else: forceVal = reg - 1., 2D/SBS/TB/LR/LBL/FP")
	ADD_PARAM_DES(unsigned char,  out_3dFormat,         0, "0: PQL, else: forceVal = reg - 1., 2D/SG/PR")
	ADD_PARAM_DES(unsigned char,  u2_outMode,           0, "0: PQL, else: forceVal = reg - 1., Video / PCmode")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned char,  u1_outLR_LRsamePh,    0, "if LR-out, LR same memc-phase")
	ADD_PARAM_DES(unsigned char,  u1_filmPh_sw_en,      1, "")
	ADD_PARAM_DES(unsigned char,  u1_filmPh_pf_en,      0, "")
	//ADD_PARAM_DES(unsigned char,  u1_mePhase_nearIP_en, 0, "")
	ADD_PARAM_DES(unsigned char,  u8_input_dly,         0, "")
	ADD_PARAM_DES(unsigned char,  u1_Ph_delay_calc_en,         0, "")

	//////// output
	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT(unsigned char, u8_deblur_lvl_pre)
	ADD_OUTPUT(unsigned char, u8_dejudder_lvl_pre)


	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT(unsigned char, u8_sys_N_pre)
	ADD_OUTPUT(unsigned char, u8_sys_M_pre)
	ADD_OUTPUT(unsigned char, u8_cadence_id_pre)

	ADD_OUTPUT(unsigned int, u32_debug_LSB[0])
	ADD_OUTPUT(unsigned int, u32_debug_LSB[1])
	ADD_OUTPUT(unsigned int, u32_debug_MSB[0])
	ADD_OUTPUT(unsigned int, u32_debug_MSB[1])

	DEBUGMODULE_END

#endif

