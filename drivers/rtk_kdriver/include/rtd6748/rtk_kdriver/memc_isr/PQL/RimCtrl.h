#ifndef _RIMCTRL_DEF_H_
#define _RIMCTRL_DEF_H_

#define _Rim_Hor_Bin_        16
#define _ME_BLK_SIZE_SHIFT_  2

typedef enum
{
	_RIM_TOP,  // Rim list in this order corresponding to BBD read-back order.
	_RIM_BOT,
	_RIM_RHT,
	_RIM_LFT,
	_RIM_NUM,
}_PQL_RIM_TYPE;
 
typedef enum
{
    _RIM_HSIZE,
	_RIM_VSIZE,	
}_PQL_RIM_SIZE;
 
typedef struct
{
	unsigned char  u1_RimCtrl_en;
	unsigned short u12_RimDiffTh;
	unsigned char  u8_RimBiggerTh;
	unsigned char  u8_RimSmallerTh;
	unsigned char  u8_RimSmallerSCTh;
	unsigned char  u8_RimIIRAlpha;
	unsigned char  u5_RimLRSmallShift;
//	unsigned char  u1_RimAbNoramlProcEn;
//	unsigned char  u1_RimAllBlackProcEn;
//	unsigned char  u8_RimAllBlackThr;

//	unsigned short u12_Logo_Rim_Top_thr;
//	unsigned int u8_Logo_Rim_unbalace_holdfrm;

	unsigned char  u8_mc_ptRim_shrink;
	unsigned char  u8_me_pixRim_shrink;
	unsigned char  u8_lbme_pixRim_shrink;
	unsigned char  u8_lbme_pixRim_comp;

	unsigned char  u8_LogoBlkRimHOffset;
	unsigned char  u8_LogoBlkRimVOffset;
	unsigned char  u8_LogoPixRimHOffset;
	unsigned char  u8_LogoPixRimVOffset;
}_PARAM_RimCtrl;

typedef struct
{
	unsigned short u12_out_resolution[_RIM_NUM];   // rim resolution
	unsigned short u12_Rim_Cur[_RIM_NUM];	       // current rim  0:top 1:bottom 2:right 3:left
	unsigned short u12_Rim_Target[_RIM_NUM];	   // rim target of fast-in and slow-out
	unsigned short u12_Rim_Pre[_RIM_NUM];          // previous rim
	unsigned short u12_Rim_Result[_RIM_NUM];       // rim result
	unsigned short u12_Rim_Pre_Det[_RIM_NUM];          // rim result

	char  s8_BiggerCnt[_RIM_NUM];
	char  s8_SmallerCnt[_RIM_NUM];

	unsigned char  u1_RimChange;
	unsigned char  u1_RimCompflag;
	unsigned int u32_rimRatio;//value:0~128
	unsigned char  u4_outResolution_pre;

	unsigned char  u1_RimControl_pre;
	unsigned char  u1_Resolution_change;

	//ME1
	unsigned char  u8_me1_pixH_sft_bit, u8_me1_pixV_sft_bit;
	unsigned char  u8_me1_blkH_sft_bit, u8_me1_blkV_sft_bit;

	unsigned short u12_me1_vbuf_blk_rim[_RIM_NUM];
	unsigned short u12_me1_vbuf_pix_rim[_RIM_NUM];
	unsigned short u12_me1_lbme_rim[_RIM_NUM];
	unsigned short u12_me1_vbuf_meander_blk_rim[2];
	unsigned short u12_me1_vbuf_meander_pix_rim[2];

	// ME2
	unsigned char  u8_me2_pixH_sft_bit, u8_me2_pixV_sft_bit;
	unsigned char  u8_me2_BlkH_sft_bit, u8_me2_BlkV_sft_bit;

	// Dehalo
	unsigned char  u8_dh_BlkH_sft_bit, u8_dh_BlkV_sft_bit;

	unsigned short u12_me2_vbuf_pfv_blk_rim[_RIM_NUM];
	unsigned short u12_me2_vbuf_ph_blk_rim[_RIM_NUM];
	unsigned short u12_me2_vbuf_pix_rim[_RIM_NUM];
	unsigned short u12_me2_lbme_rim[_RIM_NUM];

	//Logo
	unsigned char  u8_logo_pixH_sft_bit, u8_logo_pixV_sft_bit;
	unsigned char  u8_logo_blkH_sft_bit, u8_logo_blkV_sft_bit;


	unsigned char  u8_LogoRim_Blk[_RIM_NUM]; //
	unsigned short u12_LogoRim_Pxl[_RIM_NUM]; // 960 x 540

	unsigned char  u1_LogoRim_unbalace;
	unsigned char  u1_RimTwoSide_balancing;

	//MC
	unsigned short u12_mc_rim0[_RIM_NUM];  // pt rim, need shrink
	unsigned short u12_mc_rim1[_RIM_NUM];  // self rim.

	//IPME
	unsigned short u12_ipme_rim[_RIM_NUM]; // 960 x 540

	// Dehalo
	unsigned short u9_dh_blk_rim[_RIM_NUM];
	unsigned short u9_dh_blk_oftrim[_RIM_NUM];	// offset rim

	// 5Region&12Region;
	unsigned short u12_5Rgn1_v0;
	unsigned short u12_5Rgn1_v1;
	unsigned short u12_5Rgn2_v0;
	unsigned short u12_5Rgn2_v1;
	unsigned short u12_5Rgn3_h0;
	unsigned short u12_5Rgn3_h1;
	unsigned short u12_5Rgn4_h0;
	unsigned short u12_5Rgn4_h1;

	unsigned short u12_12Rgn_v0;
	unsigned short u12_12Rgn_v1;
	unsigned short u12_12Rgn_v2;
	unsigned short u12_12Rgn_v3;
	unsigned short u12_12Rgn_h0;
	unsigned short u12_12Rgn_h1;
	unsigned short u12_12Rgn_h2;
	unsigned short u12_12Rgn_h3;
	unsigned short u12_12Rgn_h4;
}_OUTPUT_RimCtrl;

typedef struct
{
	unsigned short s16_resolution_width;
	unsigned short s16_resolution_height;
	unsigned short s16_rim_bound[_RIM_NUM];
	unsigned char u8_H_Divide;
	unsigned char u8_V_Divide;
	unsigned char u8_SlowIn_MinShiftX;
	unsigned char u8_SlowIn_MinShiftY;
	unsigned char u8_RimDiffTh_cof;
	unsigned int u32_apl_size;
	unsigned char u8_scale_H;
	unsigned char u8_scale_V;
}_Param_By_Resolution_RimCtrl;

const _Param_By_Resolution_RimCtrl *GetRimParam(PQL_OUTPUT_RESOLUTION resolution_id);
VOID RimCtrl_Init(_OUTPUT_RimCtrl *pOutput);
VOID RimDet_by_KMC_YUV_Meter(const _PARAM_RimCtrl *pParam, _OUTPUT_RimCtrl *pOutput);
//unsigned int  RimDet_Check_Right(unsigned int H_line_search_position);
unsigned int  RimDet_Check_Right(unsigned int H_line_search_position,unsigned int v_line_total);
VOID RimCtrl_Proc(const _PARAM_RimCtrl *pParam, _OUTPUT_RimCtrl *pOutput);

DEBUGMODULE_BEGIN(_PARAM_RimCtrl,_OUTPUT_RimCtrl)
	ADD_PARAM_DES(unsigned char,  u1_RimCtrl_en,   1, "")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned short, u12_RimDiffTh,   256,  "u12.0")
	ADD_PARAM_DES(unsigned char,  u8_RimBiggerTh,    1,   "u8.0")
	ADD_PARAM_DES(unsigned char,  u8_RimSmallerTh,   3,   "u8.0")
	ADD_PARAM_DES(unsigned char,  u8_RimSmallerSCTh,   3,   "u8.0")
	ADD_PARAM_DES(unsigned char,  u8_RimIIRAlpha,  200, "u8.0")
	ADD_PARAM_DES(unsigned char,  u5_RimLRSmallShift,  1, "u5.0")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned char,  u8_mc_ptRim_shrink,   4, "")
    ADD_PARAM_DES(unsigned char,  u8_me_pixRim_shrink,  0, "")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned char,  u8_LogoBlkRimHOffset,  2, "")
	ADD_PARAM_DES(unsigned char,  u8_LogoBlkRimVOffset,  2, "")
	ADD_PARAM_DES(unsigned char,  u8_LogoPixRimHOffset,  8, "")
	ADD_PARAM_DES(unsigned char,  u8_LogoPixRimVOffset,  8, "")

	//////////////////////////////////////////////////////////////////////////
	ADD_OUTPUT_ARRAY(unsigned short, u12_Rim_Cur,    4)
	ADD_OUTPUT_ARRAY(unsigned short, u12_Rim_Pre,    4)
	ADD_OUTPUT_ARRAY(unsigned short, u12_Rim_Result, 4)

	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT_ARRAY(unsigned char,  u8_LogoRim_Blk, 4)
	ADD_OUTPUT_ARRAY(unsigned short, u12_LogoRim_Pxl,4)

	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT_ARRAY(unsigned short, u12_out_resolution, 4)

	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT_ARRAY(char,  s8_BiggerCnt,   4)
	ADD_OUTPUT_ARRAY(char,  s8_SmallerCnt,  4)
	ADD_OUTPUT(unsigned char,   u1_RimChange)
	ADD_OUTPUT(unsigned char,   u1_RimCompflag)
	ADD_OUTPUT(unsigned int,  u32_rimRatio)
	ADD_OUTPUT(unsigned char,   u4_outResolution_pre)
DEBUGMODULE_END

#endif

unsigned char scalerVIP_Get_MEMCRimFlag(void);