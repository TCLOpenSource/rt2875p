#ifndef _FRC_DEC_PHT_GENERATE_H
#define _FRC_DEC_PHT_GENERATE_H

/*-----------------------------------------------------------------------------------------------/
/ YJL: 2015-07-29.
/ 1. provide HW 64 bit & SW member pack & unpack function.
/
/ 2. control in_wrt_table generate and pack based on: cadence
/
/ 3. generate phase table based on:
/        gen_en/flbk_en/cadence/n2m/in_3dFormat/out_3dFormat/filmPh0_inPh/outLR_samePh/ini_status
/
/-------------------------------------------------------------------------------------------------*/

//------------------------------------------
// Definitions of Bits
//------------------------------------------
#define _ZERO                       0x00
#define _BIT0                       0x01
#define _BIT1                       0x02
#define _BIT2                       0x04
#define _BIT3                       0x08
#define _BIT4                       0x10
#define _BIT5                       0x20
#define _BIT6                       0x40
#define _BIT7                       0x80
#define _BIT8                       0x0100
#define _BIT9                       0x0200
#define _BIT10                      0x0400
#define _BIT11                      0x0800
#define _BIT12                      0x1000
#define _BIT13                      0x2000
#define _BIT14                      0x4000
#define _BIT15                      0x8000
#define _BIT16                      0x10000
#define _BIT17                      0x20000
#define _BIT18                      0x40000
#define _BIT19                      0x80000
#define _BIT20                      0x100000
#define _BIT21                      0x200000
#define _BIT22                      0x400000
#define _BIT23                      0x800000
#define _BIT24                      0x1000000
#define _BIT25                      0x2000000
#define _BIT26                      0x4000000
#define _BIT27                      0x8000000
#define _BIT28                      0x10000000
#define _BIT29                      0x20000000
#define _BIT30                      0x40000000
#define _BIT31                      0x80000000


//////////////////////////////////////////////////////////////////////////
#include "memc_isr/PQL/FRC_glb_Context.h"

#if 1
#define _PHASE_FRC_ME1_TOTAL_BUF_SIZE_     8
#if CONFIG_MC_8_BUFFER
#define _PHASE_FRC_MC_TOTAL_BUF_SIZE_      8
#define _PHASE_FRC_MCFRM_BUF_SIZE_	2
#else
#define _PHASE_FRC_MC_TOTAL_BUF_SIZE_      6
#define _PHASE_FRC_MCFRM_BUF_SIZE_         1
#endif
#define _PHASE_FRC_PPFV_BUF_SIZE_      3
#define _PHASE_FRC_ME1FRM_BUF_SIZE_    2
#else
#define _PHASE_FRC_ME1_TOTAL_BUF_SIZE_     6
#define _PHASE_FRC_ME2_TOTAL_BUF_SIZE_     6
#define _PHASE_FRC_PPFV_BUF_SIZE_      3
#define _PHASE_FRC_ME1FRM_BUF_SIZE_    2
#define _PHASE_FRC_ME2FRM_BUF_SIZE_    1
#endif

#define  ME_PHASE_PRECISION          6
#define  MC_PHASE_PRECISION          7

//////////////////////////////////////////////////////////////////////////
typedef  enum { // phase decoder info names, & pack start bit.
	MSB_OUT_3D_ENABLE = 0,
	MSB_FILM_MODE,
	MSB_OUT_PHASE,
	MSB_FILM_PHASE,   // film phase must be next to out-phase. if changed, re-check of _32bit_unpack().

	MSB_ME2_PPFV_BUF_IDX,
	MSB_ME2_IBME_P_INDEX_OFFSET,
	MSB_ME2_IBME_I_INDEX_OFFSET,
	MSB_ME2_PHASE,
	MSB_ME2_FIRST_PHASE,

	MSB_CRTC_MC_HF_INDEX,

	MSB_NAMES_NUM,
}_PH_DECINFO_MSB_NAMES;
typedef   enum    {

	LSB_CRTC_MC_LF_INDEX = 0,

	LSB_ME_LAST_PHASE,
	LSB_ME_DTS_P_INDEX,
	LSB_ME_DTS_I_INDEX,

	LSB_MC_MODE_HF_UPDATE,
	LSB_MC_MODE_LF_UPDATE,
	LSB_MC_CTS_P_INDEX,
	LSB_MC_CTS_I_INDEX,

	LSB_MC_PHASE,
	LSB_MC_HF_INDEX,
	LSB_MC_LF_INDEX,

	LSB_NAMES_NUM,

}_PH_DECINFO_LSB_NAMES_ST;
typedef struct PhaseDecoderPackInfo
{
	unsigned char u8_msb[MSB_NAMES_NUM];
	unsigned char u8_lsb[LSB_NAMES_NUM];
	unsigned char u8_inPhaseLen;
} Ph_Dec_Pack_Info;

//////////////////////////////////////////////////////////////////////////
typedef struct FilmWriteInfo
{
	unsigned char logo_en:1;
	unsigned char me1_in_idx:3;
	unsigned char me2_in_idx:3;

	unsigned char mc_finalPhase:1;
	unsigned char mc_in_idx:3;
} Film_Wrt_Info;

//////////////////////////////////////////////////////////////////////////
typedef enum         {
	_2D_VIDEO = 0,
	_2D_22,
	_2D_32,
	_3D_VIDEO,
	_3D_22,
	_3D_32,
	INI_CAD_END
} _PHT_RECORD_CAD;

typedef struct PhaseDecoderInitInfo
{
	unsigned char ini_me2_i_offset:3;
	unsigned char ini_me2_p_offset:3;
#if 1  // k4lp
	unsigned char ini_mc_i_offset:3;
	unsigned char ini_mc_p_offset:3;
#endif
	unsigned char ini_ph_frac_nume;
	unsigned char ini_ph_frac_denom;
	unsigned char ini_me2_chgImg_cnt:3;
} Ph_Dec_Init_Info;

typedef struct PhaseDecoderInfo
{
	unsigned char  out_phase :6;
	unsigned char  film_mode :2;
	unsigned char  film_phase:6;
	unsigned char  in_phase  :4;

	unsigned char  me2_phase:6;
	unsigned char  mc_phase:7;
	unsigned char  frac_phase_nume;

	unsigned char  me1_i_offset:3;
	unsigned char  me1_p_offset:3;

	unsigned char  me2_i_offset:3;
	unsigned char  me2_p_offset:3;

	unsigned char  mc_i_offset:3;
	unsigned char  mc_p_offset:3;

	unsigned char  me1_last:3;
	unsigned char  ppfv_offset:3;

	unsigned char  outPhase_new:4;
	unsigned char  inPhase_new:4;
	unsigned char  cadSeq_new :4;
	unsigned char  me1_wrt_idx_oft:3;
	unsigned char  mc_wrt_idx_oft:3;
//	unsigned char  me2_wrt_idx_oft:3;

} Ph_Dec_Info;


extern Ph_Dec_Info       phTable_DecInfoGen[256];
extern unsigned int            phTable_WrtLSB[256];
extern unsigned int            phTable_WrtMSB[256];
extern Film_Wrt_Info     phTable_WrtInfoGen[64];
extern Film_Wrt_Info     phTable_WrtInfoGen_Amend[64];

//////////////////////////////////////////////////////////////////////////

typedef struct Ph_decoderTable_Gen_Param
{
	unsigned char u1_goldenPhT_gen_en;
	unsigned char u1_phFlbk_en;

	unsigned char u4_filmPh0_inPh;
	unsigned char u1_LRout_samePh;

	unsigned char  u8_cadence_id;
	unsigned char  u4_sys_N;
	unsigned char  u6_sys_M;

	unsigned char  u8_deblur_lvl;
	unsigned char  u8_dejudder_lvl;
	unsigned char  u8_pulldown_mode;

	PQL_INPUT_3D_FORMAT   in3d_format;
	PQL_OUTPUT_3D_FORMAT  out3d_format;

	unsigned short u16_phT_stIdx;

	unsigned short u8_ph_sys_dly;

}Ph_decGen_Param;

typedef struct Ph_decoderTable_Gen_out
{
	FRC_CADENCE           local_FRC_cadence;

	unsigned char                 u8_fracPh_nume;
	unsigned char                 u8_fracPh_denomi;

	unsigned short                u16_phT_stIdx;
	unsigned short                u16_phT_endIdx_p1;
	unsigned short                u16_phT_length;

	unsigned short                u16_filmPh0_tableIdx;
}Ph_decGen_Out;

//////////////////////////////////////////////////////////////////////////
//// 1. HW 64 bit & SW member pack & unpack function.
VOID   FRC_phTable_Init_64bitConfig(VOID);
VOID   FRC_phTable_Init_1n2m_video_22_32(VOID);
VOID   phaseTable_pack(Ph_Dec_Pack_Info *pPackInfo, unsigned int *pU32_pack_LSB, unsigned int *pU32_pack_MSB);
VOID   phaseTable_unpack(unsigned int ui_pack_LSB, unsigned int ui_pack_MSB, Ph_Dec_Info *pPh_DecInfo);
VOID   _32bit_pack(unsigned int *pui_packWord, unsigned char u8_stBit, unsigned char u8_endBit, unsigned int uiVal);
unsigned int _32bit_unpack(unsigned int *pui_packWord, unsigned char u8_stBit, unsigned char u8_endBit);

//// 2. control in_wrt_table generate and pack.
VOID   FRC_phTable_WrtStep_PCmode_Gen(VOID);
VOID   FRC_phTable_WrtStep_VideoMode_Gen( FRC_CADENCE cadInfo);
VOID   FRC_phTable_WrtStep_VideoMode_Gen_idx(FRC_CADENCE cadInfo,unsigned char st_idx);
unsigned char  FRC_phTable_WrtStep_Check_me1(unsigned char u8_filmPh, unsigned char in3d_format);
unsigned char  FRC_phTable_WrtStep_Check_me2(unsigned char u8_filmPh, unsigned char in3d_format);
unsigned char  FRC_phTable_WrtStep_Check_mc(unsigned char u8_filmPh, unsigned char in3d_format);
unsigned char  FRC_phTable_WrtStep_Check_mc_vd_22_32(unsigned char cadence_id, unsigned char u8_filmPh, unsigned char in3d_format);
//unsigned char  FRC_phTable_WrtStep_Check_me2_vd_22_32(unsigned char cadence_id, unsigned char u8_filmPh, unsigned char in3d_format);
unsigned int phTable_WrtStep_pack(unsigned char u8_stIdx);
unsigned int phTable_me2_WrtStep_pack(unsigned char u8_stIdx);
unsigned int phTable_WrtStep_pack_vd_22_32(unsigned char u8_stIdx);
unsigned int phTable_me2_WrtStep_pack_vd_22_32(unsigned char u8_stIdx);

//// 3. generate phase table.
VOID FRC_phTable_Init_decGen_config(VOID);
VOID FRC_phTable_LowDelay_Gen_Video(const Ph_decGen_Param *pParam, Ph_decGen_Out *pOutput, unsigned short u16_k , PQL_INPUT_FRAME_RATE in_fmRate, PQL_OUTPUT_FRAME_RATE out_fmRate, unsigned char u1_lowdelaystate);
VOID FRC_phTable_NewLowDelay_Gen(const Ph_decGen_Param *pParam, Ph_decGen_Out *pOutput, unsigned short u16_k , PQL_INPUT_FRAME_RATE in_fmRate, PQL_OUTPUT_FRAME_RATE out_fmRate, FRC_CADENCE_ID cadence_id);
VOID FRC_phTable_QMS_Gen(const Ph_decGen_Param *pParam, Ph_decGen_Out *pOutput, unsigned short u16_k , PQL_INPUT_FRAME_RATE in_fmRate, PQL_OUTPUT_FRAME_RATE out_fmRate);
VOID FRC_phTable_VideoMode_Proc(const Ph_decGen_Param *pParam, Ph_decGen_Out *pOutput);
VOID FRC_decPhT_PCmode_Proc(const Ph_decGen_Param *pParam, Ph_decGen_Out *pOutput);

////////
VOID FRC_decPhT_goldenGen_Proc(const Ph_decGen_Param *pParam, Ph_decGen_Out *pOutput);
VOID FRC_decPhT_flbk_proc(const Ph_decGen_Param *pParam, Ph_decGen_Out *pOutput);
VOID FRC_phTable_IntpInfo_PackGen(const Ph_decGen_Param *pParam, Ph_decGen_Out *pOutput, unsigned short phDecInfo_k, Ph_Dec_Pack_Info *pPackInfo);

//////// generate functions
VOID FRC_phTable_basicInfo_Gen(const Ph_decGen_Param *pParam, Ph_decGen_Out *pOutput);
VOID FRC_phTable_inPh_outPh_Gen(const Ph_decGen_Param *pParam, Ph_decGen_Out *pOutput);
VOID FRC_phTable_filmPh_Gen(const Ph_decGen_Param *pParam, Ph_decGen_Out *pOutput);

VOID FRC_phTable_IP_oft_Gen(const Ph_decGen_Param *pParam, Ph_decGen_Out *pOutput);
VOID FRC_phTable_IP_oft_Gen_IPns_mc(Ph_Dec_Info *pDecInfo_pre, Ph_Dec_Info *pDecInfo_cur,unsigned char chgImg_cnt_me2, FRC_CADENCE *pLocalCadence);
VOID FRC_phTable_IP_oft_Gen_IPns_me2(Ph_Dec_Info *pDecInfo_pre, Ph_Dec_Info *pDecInfo_cur,unsigned char chgImg_cnt_me2, FRC_CADENCE *pLocalCadence);
VOID FRC_phTable_IP_oft_Gen_IPns_me1(Ph_Dec_Info *pDecInfo_pre, Ph_Dec_Info *pDecInfo_cur,unsigned char chgImg_cnt_me1, FRC_CADENCE *pLocalCadence);
VOID FRC_phTable_IP_oft_Gen_IPs_mc(const Ph_decGen_Param *pParam, Ph_Dec_Info *pDecInfo_pre, Ph_Dec_Info *pDecInfo_cur, unsigned char chgImg_cnt_me2, FRC_CADENCE *pLocalCadence);
VOID FRC_phTable_IP_oft_Gen_IPs_me2(const Ph_decGen_Param *pParam, Ph_Dec_Info *pDecInfo_pre, Ph_Dec_Info *pDecInfo_cur, unsigned char chgImg_cnt_me2, FRC_CADENCE *pLocalCadence);
VOID FRC_phTable_IP_oft_Gen_IPs_me1(const Ph_decGen_Param *pParam, Ph_Dec_Info *pDecInfo_pre, Ph_Dec_Info *pDecInfo_cur, unsigned char chgImg_cnt_me1, FRC_CADENCE *pLocalCadence);
VOID FRC_phTable_IP_oft_newUsableP_check_me1(unsigned char in3d_format, FRC_CADENCE *pLocalCadence, unsigned char u8_filmPh, unsigned char *pPoffset);
VOID FRC_phTable_IP_oft_newUsableP_check_me2(unsigned char in3d_format, FRC_CADENCE *pLocalCadence, unsigned char u8_filmPh, unsigned char *pPoffset);
VOID FRC_phTable_IP_oft_newUsableP_check_mc(unsigned char in3d_format, FRC_CADENCE *pLocalCadence, unsigned char u8_filmPh, unsigned char *pPoffset);
////////  flbk functions.
VOID FRC_dejudder_phFlbk_ctrl(unsigned char deJudder_lvl, Ph_decGen_Out *pOutput);
VOID FRC_deblur_phFlbk_ctrl(unsigned char deBlur_lvl, Ph_decGen_Out *pOutput);
VOID FRC_deblur_phFlbk_IPs(Ph_Dec_Info *pDecInfo_n1, Ph_Dec_Info *pDecInfo_0, Ph_Dec_Info *pDecInfo_p1, unsigned char flbk_lvl);

VOID FRC_phTable_me1_deriva_me1(const Ph_decGen_Param *pParam, Ph_Dec_Info *pDecInfo_cur, unsigned char chgImg_cnt_me1, FRC_CADENCE *pLocalCadence);


///////after image
unsigned char after_image_proc(unsigned char u8_filmPhase_pre, unsigned char u8_filmPhase_dtc,unsigned char u8_outPhase, unsigned char u8_cadence_id_pre, unsigned char u8_cadence_id_dtc, unsigned char in_3dFormat,  unsigned char u8_sys_M_pre, unsigned short u16_end_idx);
#endif







