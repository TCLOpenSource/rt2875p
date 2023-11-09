/**
 * @file hal_me.h
  * @brief This file is for kme me register setting
  * @date Nov.14.2014
  * @version V0.0.1
  * @author Martin Yan
  * @par Copyright (C) 2014 Real-Image Co. LTD
  * @par History:
  * 		 Version: 0.0.1
 */

#ifndef _HAL_ME_INCLUDE_
#define _HAL_ME_INCLUDE_

#include "memc_isr/include/kw_type.h"
#include "memc_isr/Common/memc_type.h"

typedef enum
{
	_ME2_Candidate_Table_60Hz,
	_ME2_Candidate_Table_120Hz,
	_ME2_Candidate_Table_Max
} _ME2_Candidate_Table_Type;

typedef enum
{
	_ME1_Candidate_Table_60Hz,
	_ME1_Candidate_Table_120Hz,
	_ME1_Candidate_Table_Max
} _ME1_Candidate_Table_Type;

typedef struct {
	//1st
	unsigned char _1st_ph_cand_num;
	unsigned char _1st_ph_offset_cand_num;
	unsigned char _1st_update_cand_num;
	unsigned char _1st_pfv0_cand_num;
	unsigned char _1st_pfv0_offset_cand_num;
	unsigned char _1st_pfv1_cand_num;
	unsigned char _1st_pfv1_offset_cand_num;
	unsigned char _1st_ph_cand_dediff_en;
	unsigned char _1st_pfv0_cand_dediff_en;
	unsigned char _1st_pfv1_cand_dediff_en;
	unsigned char _1st_zmv_en;
	unsigned char _1st_gmv_en;
	//2nd
	unsigned char _2nd_ph_cand_num;
	unsigned char _2nd_ph_offset_cand_num;
	unsigned char _2nd_update_cand_num;
	unsigned char _2nd_pfv0_cand_num;
	unsigned char _2nd_pfv0_offset_cand_num;
	unsigned char _2nd_pfv1_cand_num;
	unsigned char _2nd_pfv1_offset_cand_num;
	unsigned char _2nd_ph_cand_dediff_en;
	unsigned char _2nd_pfv0_cand_dediff_en;
	unsigned char _2nd_pfv1_cand_dediff_en;
	unsigned char _2nd_zmv_en;
	unsigned char _2nd_gmv_en;
}_str_cand_num;

typedef struct {
	char offset_x;
	char offset_y;
}_str_location_offset;

typedef struct {
	_str_location_offset _ph_cand0;
	_str_location_offset _ph_cand1;
	_str_location_offset _ph_cand2;
	_str_location_offset _ph_cand3;
	_str_location_offset _ph_cand4;
	_str_location_offset _ph_cand5;
	_str_location_offset _ph_cand6;
	_str_location_offset _ph_cand7;

}_str_1st_ph_cand;

typedef struct {
	_str_location_offset _update_cand0;
	_str_location_offset _update_cand1;
	_str_location_offset _update_cand2;
	_str_location_offset _update_cand3;

}_str_1st_update_cand;

typedef struct {
	_str_location_offset _pfv0_cand0;
	_str_location_offset _pfv0_cand1;
	_str_location_offset _pfv0_cand2;
	_str_location_offset _pfv0_cand3;
	_str_location_offset _pfv0_cand4;
	_str_location_offset _pfv0_cand5;
	_str_location_offset _pfv0_cand6;
	_str_location_offset _pfv0_cand7;

}_str_1st_pfv0_cand;

typedef struct {
	_str_location_offset _pfv1_cand0;
	_str_location_offset _pfv1_cand1;
	_str_location_offset _pfv1_cand2;
	_str_location_offset _pfv1_cand3;
	_str_location_offset _pfv1_cand4;
	_str_location_offset _pfv1_cand5;
	_str_location_offset _pfv1_cand6;
	_str_location_offset _pfv1_cand7;

}_str_1st_pfv1_cand;


typedef struct {
	unsigned char ph_pnt0;
	unsigned char ph_pnt1;
	unsigned char ph_pnt2;
	unsigned char ph_pnt3;
	unsigned char ph_pnt4;
	unsigned char ph_pnt5;
	unsigned char ph_pnt6;
	unsigned char ph_pnt7;

}_str_1st_ph_pnt;

typedef struct {
	unsigned char update_pnt0;
	unsigned char update_pnt1;
	unsigned char update_pnt2;
	unsigned char update_pnt3;

}_str_1st_update_pnt;

typedef struct {
	unsigned char pfv0_pnt0;
	unsigned char pfv0_pnt1;
	unsigned char pfv0_pnt2;
	unsigned char pfv0_pnt3;
	unsigned char pfv0_pnt4;
	unsigned char pfv0_pnt5;
	unsigned char pfv0_pnt6;
	unsigned char pfv0_pnt7;

}_str_1st_pfv0_pnt;


typedef struct {
	unsigned char pfv1_pnt0;
	unsigned char pfv1_pnt1;
	unsigned char pfv1_pnt2;
	unsigned char pfv1_pnt3;
	unsigned char pfv1_pnt4;
	unsigned char pfv1_pnt5;
	unsigned char pfv1_pnt6;
	unsigned char pfv1_pnt7;

}_str_1st_pfv1_pnt;


typedef struct {
	unsigned char _1st_ph_cand_dediff_pnt;
	unsigned char _1st_pfv0_cand_dediff_pnt;
	unsigned char _1st_pfv1_cand_dediff_pnt;
	unsigned char _1st_zmv_pnt;
	unsigned char _1st_gmv_pnt;

}_str_1st_others_pnt;

typedef struct {
	_str_location_offset _ph_cand0;
	_str_location_offset _ph_cand1;
	_str_location_offset _ph_cand2;
	_str_location_offset _ph_cand3;
	_str_location_offset _ph_cand4;
	_str_location_offset _ph_cand5;
	_str_location_offset _ph_cand6;
	_str_location_offset _ph_cand7;

}_str_2nd_ph_cand;

typedef struct {
	_str_location_offset _update_cand0;
	_str_location_offset _update_cand1;
	_str_location_offset _update_cand2;
	_str_location_offset _update_cand3;

}_str_2nd_update_cand;

typedef struct {
	_str_location_offset _pfv0_cand0;
	_str_location_offset _pfv0_cand1;
	_str_location_offset _pfv0_cand2;
	_str_location_offset _pfv0_cand3;
	_str_location_offset _pfv0_cand4;
	_str_location_offset _pfv0_cand5;
	_str_location_offset _pfv0_cand6;
	_str_location_offset _pfv0_cand7;

}_str_2nd_pfv0_cand;

typedef struct {
	_str_location_offset _pfv1_cand0;
	_str_location_offset _pfv1_cand1;
	_str_location_offset _pfv1_cand2;
	_str_location_offset _pfv1_cand3;
	_str_location_offset _pfv1_cand4;
	_str_location_offset _pfv1_cand5;
	_str_location_offset _pfv1_cand6;
	_str_location_offset _pfv1_cand7;

}_str_2nd_pfv1_cand;

typedef struct {
	unsigned char ph_pnt0;
	unsigned char ph_pnt1;
	unsigned char ph_pnt2;
	unsigned char ph_pnt3;
	unsigned char ph_pnt4;
	unsigned char ph_pnt5;
	unsigned char ph_pnt6;
	unsigned char ph_pnt7;

}_str_2nd_ph_pnt;

typedef struct {
	unsigned char update_pnt0;
	unsigned char update_pnt1;
	unsigned char update_pnt2;
	unsigned char update_pnt3;

}_str_2nd_update_pnt;

typedef struct {
	unsigned char pfv0_pnt0;
	unsigned char pfv0_pnt1;
	unsigned char pfv0_pnt2;
	unsigned char pfv0_pnt3;
	unsigned char pfv0_pnt4;
	unsigned char pfv0_pnt5;
	unsigned char pfv0_pnt6;
	unsigned char pfv0_pnt7;

}_str_2nd_pfv0_pnt;


typedef struct {
	unsigned char pfv1_pnt0;
	unsigned char pfv1_pnt1;
	unsigned char pfv1_pnt2;
	unsigned char pfv1_pnt3;
	unsigned char pfv1_pnt4;
	unsigned char pfv1_pnt5;
	unsigned char pfv1_pnt6;
	unsigned char pfv1_pnt7;

}_str_2nd_pfv1_pnt;

typedef struct {
	unsigned char _2nd_ph_cand_dediff_pnt;
	unsigned char _2nd_pfv0_cand_dediff_pnt;
	unsigned char _2nd_pfv1_cand_dediff_pnt;
	unsigned char _2nd_zmv_pnt;
	unsigned char _2nd_gmv_pnt;

}_str_2nd_others_pnt;

typedef struct {
	unsigned char offset;
	unsigned char gain;

}_str_ph_offset_gain;

typedef struct {
	_str_ph_offset_gain _ph_index0;
	_str_ph_offset_gain _ph_index1;
	_str_ph_offset_gain _ph_index2;
	_str_ph_offset_gain _ph_index3;

}_str_ph_index_setting;

typedef struct {
	_str_1st_ph_cand 		_1st_ph_cand;
	_str_1st_update_cand	_1st_update_cand;
	_str_1st_pfv0_cand 		_1st_pfv0_cand;
	_str_1st_pfv1_cand 		_1st_pfv1_cand;

	_str_2nd_ph_cand 		_2nd_ph_cand;
	_str_2nd_update_cand 	_2nd_update_cand;
	_str_2nd_pfv0_cand 		_2nd_pfv0_cand;
	_str_2nd_pfv1_cand 		_2nd_pfv1_cand;

}_str_cand_location;


typedef struct {
	_str_1st_ph_pnt			_1st_ph_pnt;
	_str_1st_update_pnt		_1st_update_pnt;
	_str_1st_pfv0_pnt 		_1st_pfv0_pnt;
	_str_1st_pfv1_pnt 		_1st_pfv1_pnt;
	_str_1st_others_pnt 	_1st_others_pnt;

	_str_2nd_ph_pnt 		_2nd_ph_pnt;
	_str_2nd_update_pnt 	_2nd_update_pnt;
	_str_2nd_pfv0_pnt 		_2nd_pfv0_pnt;
	_str_2nd_pfv1_pnt 		_2nd_pfv1_pnt;
	_str_2nd_others_pnt		_2nd_others_pnt;

}_str_cand_pnt;

typedef struct {
	_str_cand_num			_cand_num;
	_str_cand_location		_cand_location;
	_str_cand_pnt			_cand_pnt;
	_str_ph_index_setting	_ph_index_setting;
}_str_ME2_Candidate_table;

#if 1
typedef struct {
	unsigned char _zmv_en;
	unsigned char _gmv_en;
	unsigned char _cand_en[10];
	unsigned char _upt_cand_en[4];
} _str_me1_cand_num;

typedef struct {
	_str_location_offset _cand[10];
	_str_location_offset _upt[4];
} _str_me1_cand_location;

typedef struct {
	unsigned short _cand_zmv_pnt;
	unsigned short _cand_gmv_pnt;
	unsigned short _cand_ppi_pnt;
	unsigned char _cand_pnt[10];
	unsigned char _cand_upt_pnt[4];
} _str_me1_cand_pnt;

typedef struct {
	_str_me1_cand_num		_cand_ip_num;
	_str_me1_cand_num		_cand_pi_num;
	_str_me1_cand_location	_cand_ip_location;
	_str_me1_cand_location	_cand_pi_location;
	_str_me1_cand_pnt		_cand_ip_pnt;
	_str_me1_cand_pnt		_cand_ip_meander_pnt;
	_str_me1_cand_pnt		_cand_pi_pnt;
	_str_me1_cand_pnt		_cand_pi_meander_pnt;
} _str_ME1_Candidate_table;
#endif

/**
 * @brief This function set KME ipme in mode
 * @param [in] unsigned int u32In_mode
 * @retval VOID
*/
VOID HAL_KME_IPME_SetInMode(unsigned int u32In_mode);

/**
 * @brief This function set KME ipme h fetch mode
 * @param [in] unsigned int u32HFetch_Mode
 * @retval VOID
*/
VOID HAL_KME_IPME_SetHFetchMode(unsigned int u32HFetch_Mode);

/**
 * @brief This function set KME ipme v fetch mode
 * @param [in] unsigned int u32VFetch_Mode
 * @retval VOID
*/
VOID HAL_KME_IPME_SetVFetchMode(unsigned int u32VFetch_Mode);

/**
 * @brief This function set KME ipme2 v fetch mode
 * @param [in] unsigned int u32VFetch_Mode
 * @retval VOID
*/
VOID HAL_KME_IPME2_SetVFetchMode(unsigned int u32VFetch_Mode);

/**
 * @brief This function set KME ipme h center value
 * @param [in] unsigned int u32H_Center_Value
 * @retval VOID
*/
VOID HAL_KME_IPME_SetHCenterValue(unsigned int u32H_Center_Value);

/**
 * @brief This function set KME ipme v center value
 * @param [in] unsigned int u32V_Center_Value
 * @retval VOID
*/
VOID HAL_KME_IPME_SetVCenterValue(unsigned int u32V_Center_Value);


/**
 * @brief This function set KME ipme2 v center value
 * @param [in] unsigned int u32V_Center_Value
 * @retval VOID
*/
VOID HAL_KME_IPME2_SetVCenterValue(unsigned int u32V_Center_Value);

/**
 * @brief This function set KME ipme h scaler active
 * @param [in] unsigned int u32Scaler_Hactive
 * @retval VOID
*/
VOID HAL_KME_IPME_SetScalerHactive(unsigned int u32Scaler_Hactive);

/**
 * @brief This function set KME ipme pscaler v active
 * @param [in] unsigned int u32Pscaler_Vactive
 * @retval VOID
*/
VOID HAL_KME_IPME_SetPscalerVactive(unsigned int u32Pscaler_Vactive);

/**
 * @brief This function set KME LBME total line number
 * @param [in] u32 u32TotalLineNum
 * @retval VOID
*/
VOID HAL_KME_LBME_SetTotalLineNum(unsigned int u32TotalLineNum);

/**
 * @brief This function set KME LBME rim
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_KME_LBME_SetRim(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right);

#if 1  // K4Lp
/**
 * @brief This function set KME LBME meander rim
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_KME_LBME_SetMeanderRim(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right);

/**
 * @brief This function set KME LBME2 rim
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_KME_LBME2_SetRim(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right);
#endif

/**
 * @brief This function set KME LBME total pixel number
 * @param [in] u32 u32TotalPixelNum
 * @retval VOID
*/
VOID HAL_KME_LBME_SetTotalPixelNum(unsigned int u32TotalPixelNum);

/**
 * @brief This function set KME LBME block number per line
 * @param [in] u32 u32blk_num_perline
 * @retval VOID
*/
VOID HAL_KME_LBME_SetBlkNumPerLine(unsigned int u32blk_num_perline);

/**
 * @brief This function set KME LBME debug hactive
 * @param [in] u32 u32debug_hactive
 * @retval VOID
*/
VOID HAL_KME_LBME_SetDebugHactive(unsigned int u32debug_hactive);

/**
 * @brief This function set KME LBME total linebuf number
 * @param [in] u32 u32TotalLinebufNum
 * @retval VOID
*/
VOID HAL_KME_LBME_SetTotalLinebufNum(unsigned int u32TotalLinebufNum);

/**
 * @brief This function set KME LBME2 total line number
 * @param [in] u32 u32TotalLineNum
 * @retval VOID
*/
VOID HAL_KME2_LBME_SetTotalLineNum(unsigned int u32TotalLineNum);

/**
 * @brief This function set KME LBME2 total pixel number
 * @param [in] u32 u32TotalPixelNum
 * @retval VOID
*/
VOID HAL_KME2_LBME_SetTotalPixelNum(unsigned int u32TotalPixelNum);

/**
 * @brief This function set KME2 LBME block number per line
 * @param [in] u32 u32blk_num_perline
 * @retval VOID
*/
VOID HAL_KME2_LBME_SetBlkNumPerLine(unsigned int u32blk_num_perline);

/**
 * @brief This function set KME2 LBME debug hactive
 * @param [in] u32 u32debug_hactive
 * @retval VOID
*/
VOID HAL_KME2_LBME_SetDebugHactive(unsigned int u32debug_hactive);

/**
 * @brief This function set KME2 LBME total linebuf number
 * @param [in] u32 u32TotalLinebufNum
 * @retval VOID
*/
VOID HAL_KME2_LBME_SetTotalLinebufNum(unsigned int u32TotalLinebufNum);

/**
 * @brief This function set kme vbuf hactive
 * @param [in] unsigned int u32VBuf_Hactive
 * @retval VOID
*/
VOID HAL_KME_VBUF_Set_Hactive(unsigned int u32VBuf_Hactive);

/**
 * @brief This function set kme vbuf vactive
 * @param [in] unsigned int u32VBuf_Vactive
 * @retval VOID
*/
VOID HAL_KME_VBUF_Set_Vactive(unsigned int u32VBuf_Vactive);

/**
 * @brief This function set lbme update num
 * @param [in] unsigned int u32Lbme_Update_Num
 * @retval VOID
*/
VOID HAL_KME_VBUF_Set_LBMEUpdateNum(unsigned int u32Lbme_Update_Num);

/**
 * @brief This function set lbme trig mid row num
 * @param [in] unsigned int u32Lbme_TrigMidRow_Num
 * @retval VOID
*/
VOID HAL_KME_VBUF_Set_LBMETrigMidRowNum(unsigned int u32Lbme_TrigMidRow_Num);

/**
 * @brief This function set lbme h shift bit
 * @param [in] unsigned int u32Lbme_HShift
 * @retval VOID
*/
VOID HAL_KME_VBUF_Set_LBMEHShift(unsigned int u32Lbme_HShift);

/**
 * @brief This function set lbme v shift bit
 * @param [in] unsigned int u32Lbme_VShift
 * @retval VOID
*/
VOID HAL_KME_VBUF_Set_LBMEVShift(unsigned int u32Lbme_VShift);

/**
 * @brief This function set kme2 vbuf hactive
 * @param [in] unsigned int u32VBuf_Hactive
 * @retval VOID
*/
VOID HAL_KME2_VBUF_Set_PFVHactive(unsigned int u32VBuf_Hactive);

/**
 * @brief This function set kme vbuf vactive
 * @param [in] unsigned int u32VBuf_Vactive
 * @retval VOID
*/
VOID HAL_KME2_VBUF_Set_PFVVactive(unsigned int u32VBuf_Vactive);

#if 1  // K4Lp
/**
 * @brief This function set kme2 vbuf phase hactive
 * @param [in] unsigned int u32VBuf_Hactive
 * @retval VOID
*/
VOID HAL_KME2_VBUF_Set_PHHactive(unsigned int u32VBuf_Hactive);

/**
 * @brief This function set kme vbuf phase vactive
 * @param [in] unsigned int u32VBuf_Vactive
 * @retval VOID
*/
VOID HAL_KME2_VBUF_Set_PHVactive(unsigned int u32VBuf_Vactive);
#endif

/**
 * @brief This function set kme2 vbuf 3dpr enable
 * @param [in] unsigned int u32VBuf_Vactive
 * @retval VOID
*/
VOID HAL_KME2_VBUF_Set_3DPREnable(BOOL bEnable);

/**
 * @brief This function set kme logo0 h fetch mode
 * @param [in] unsigned int u32Logo_Hfetch_mode
 * @retval VOID
*/
VOID HAL_KME_LOGO0_Set_HFetchMode(unsigned int u32Logo_Hfetch_mode);

/**
 * @brief This function set kme logo0 v fetch mode
 * @param [in] unsigned int u32Logo_Vfetch_mode
 * @retval VOID
*/
VOID HAL_KME_LOGO0_Set_VFetchMode(unsigned int u32Logo_Vfetch_mode);

/**
 * @brief This function set kme logo0 pscaler v active
 * @param [in] unsigned int u32Logo_Pscaler_Vactive
 * @retval VOID
*/
VOID HAL_KME_LOGO0_Set_PscalerVactive(unsigned int u32Logo_Pscaler_Vactive);

/**
 * @brief This function set clear MV enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_KME_METOP1_Set_ClearMVEnable(BOOL bEnable);

/**
 * @brief This function set me1_sc_en
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_KME_METOP1_Set_SC_en(BOOL bEnable);

/**
 * @brief This function set me1 rmv size
 * @param [in] unsigned int u32RMV_Hact
 * @param [in] unsigned int u32RMV_Vact
 * @retval VOID
*/
VOID HAL_KME_ME1_Set_RMVSize(unsigned int u32RMV_Hact, unsigned int u32RMV_Vact);

/**
 * @brief This function set me2 rmv size
 * @param [in] unsigned int u32RMV_Hact
 * @param [in] unsigned int u32RMV_Vact
 * @retval VOID
*/
VOID HAL_KME_ME2_Set_RMVSize(unsigned int u32RMV_Hact, unsigned int u32RMV_Vact);

/**
 * @brief This function set me1 static size
 * @param [in] unsigned int u32Statis_Hact
 * @param [in] unsigned int u32Statis_Vact
 * @retval VOID
*/
VOID HAL_KME_ME1_Set_StatisSize(unsigned int u32Statis_Hact, unsigned int u32Statis_Vact);

#if 1  // K4Lp
/**
 * @brief This function set ph pfv h ratio
 * @param [in] unsigned int u32ph_pfv_h_ratio
 * @retval VOID
*/
VOID HAL_KME_VBUF_Set_PHPFHRatio(unsigned int u32ph_pfv_h_ratio);
#endif

/**
 * @brief This function set ph pfv v ratio
 * @param [in] unsigned int u32ph_pfv_v_ratio
 * @retval VOID
*/
VOID HAL_KME_VBUF_Set_PHPFVRatio(unsigned int u32ph_pfv_v_ratio);

#if 1
/**
 * @brief This function set ph lbme h ratio
 * @param [in] unsigned int u32ph_lbme_h_ratio
 * @retval VOID
*/
VOID HAL_KME_VBUF_Set_PHLBMEHRatio(unsigned int u32ph_lbme_h_ratio);

/**
 * @brief This function set ph lbme v ratio
 * @param [in] unsigned int u32ph_lbme_v_ratio
 * @retval VOID
*/
VOID HAL_KME_VBUF_Set_PHLBMEVRatio(unsigned int u32ph_lbme_v_ratio);
#endif

/**
 * @brief This function set pfv mv fetch in 1 row
 * @param [in] unsigned int u32val
 * @retval VOID
*/
VOID HAL_KME_VBUF_Set_PFVMVFetchIn1Row(unsigned int u32val);

/**
 * @brief This function set ppfv mv fetch in 1 row
 * @param [in] unsigned int u32val
 * @retval VOID
*/
VOID HAL_KME_VBUF_Set_PPFVMVFetchIn1Row(unsigned int u32val);


/**
 * @brief This function set vbuf blk vact
 * @param [in] unsigned int u32val
 * @retval VOID
*/
VOID HAL_KME_VBUF_Set_BlockVact(unsigned int u32val);


/**
 * @brief This function set vbuf row type
 * @param [in] unsigned int u32val
 * @retval VOID
*/
VOID HAL_KME_VBUF_Set_RowType(unsigned int u32val);

/**
 * @brief This function set vbuf wait cycle
 * @param [in] unsigned int u32val
 * @retval VOID
*/
VOID HAL_KME_VBUF_Set_WaitCycle(unsigned int u32MV_Trig, unsigned int u32ME_Trig);

/**
 * @brief This function set work mode
 * @param [in] unsigned int u32val
 * @retval VOID
*/
VOID HAL_KME_Set_KME_WorkMode(unsigned int u32val);

/**
 * @brief This function set work mode
 * @param [in] unsigned int u32val
 * @retval VOID
*/
VOID HAL_KME_Set_Vspll_WorkMode(unsigned int u32val);

/**
 * @brief This function set meander enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_KME_Set_MeanderEnable(BOOL bEnable);

/**
 * @brief This function set LBME2 3DPR Enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_KME_Set_LBME23DPREnable(BOOL bEnable);

#if 1  // K4Lp
/**
 * @brief This function set ME1 vbuf pixel rim
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_KME_Set_ME1VbufPixelRIM(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right);
#endif

/**
 * @brief This function set ME1 vbuf block rim
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_KME_Set_ME1VbufBlockRIM(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right);

/**
 * @brief This function set ME2 vbuf pfv block rim
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_KME_Set_ME2VbufPFVBlockRIM(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right);

#if 1  // K4Lp
/**
 * @brief This function set ME2 vbuf phase block rim
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_KME_Set_ME2VbufPHBlockRIM(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right);

/**
 * @brief This function set ME2 vbuf pixel rim
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_KME_Set_ME2VbufPixelRIM(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right);

/**
 * @brief This function set me2 135 to 270
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_KME_SetME2135To270(BOOL bEnable);

/**
 * @brief This function set me2 135 to 270 read interval
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_KME_SetME2135To270RDInterval(unsigned int u32Val);

/**
 * @brief This function set me2 135 to 270 wait row numbers
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_KME_SetME2135To270WaitRowNum(unsigned int u32Val);
#endif

/**
 * @brief This function set BBD window
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_KME_Set_BBDWindow(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right);

/**
 * @brief This function set ME2 LR Dec enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_KME_Set_ME2LRDecEnable(BOOL bEnable);

/**
 * @brief This function set 3d ppfv index
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_KME_Set_3DPPFVIDX(BOOL bEnable);

/**
 * @brief This function set crtc lr enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_KME_SetLREnable(BOOL bEnable);

/**
 * @brief This function set crtc lr cycle
 * @param [in] unsigned int u32Cycle
 * @retval VOID
*/
VOID HAL_KME_SetLRCycle(unsigned int u32Cycle);

/**
 * @brief This function set crtc lr pattern
 * @param [in] unsigned int u32Pattern
 * @retval VOID
*/
VOID HAL_KME_SetLRPattern(unsigned int u32Pattern);

/**
 * @brief This function set ME1 HV Ratio
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_KME_Set_ME1HVRatio(BOOL bEnable);

/**
 * @brief This function set meander block rim
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @retval VOID
*/
VOID HAL_KME_SetMeanderBLKRIM(unsigned int u32Top, unsigned int u32Bottom);
/**
 * @brief This function set meander pixel rim
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @retval VOID
*/
VOID HAL_KME_SetMeanderPIXRIM(unsigned int u32Top, unsigned int u32Bottom);

/**
 * @brief This function set DM PFV PPFV interval
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_KME_ME2_SetDMPFVPPFVInterval(unsigned int u32Val);

#if 1
/**
 * @brief This function set lbme dm number in 1 row
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_KME_ME2_SetVbufLBMEDMNum1Row(unsigned int u32Val);

/**
 * @brief This function get 1st GMV offset number
 * @param [in] VOID
 * @retval u32Val
*/
unsigned int HAL_KME_ME2_Get1stGMVOffetNum(VOID);

/**
 * @brief This function get 1st phase candidate number
 * @param [in] VOID
 * @retval u32Val
*/
unsigned int HAL_KME_ME2_Get1stPHCandiNum(VOID);

/**
 * @brief This function get 1st phase candidate offset number
 * @param [in] VOID
 * @retval u32Val
*/
unsigned int HAL_KME_ME2_Get1stPHCandiOffsetNum(VOID);

/**
 * @brief This function get 1st phv0 candidate number
 * @param [in] VOID
 * @retval u32Val
*/
unsigned int HAL_KME_ME2_Get1stPFV0CandiNum(VOID);

/**
 * @brief This function get 1st phv0 candidate offset number
 * @param [in] VOID
 * @retval u32Val
*/
unsigned int HAL_KME_ME2_Get1stPFV0CandiOffsetNum(VOID);

/**
 * @brief This function get 1st phv1 candidate number
 * @param [in] VOID
 * @retval u32Val
*/
unsigned int HAL_KME_ME2_Get1stPFV1CandiNum(VOID);

/**
 * @brief This function get 1st phv1 candidate offset number
 * @param [in] VOID
 * @retval u32Val
*/
unsigned int HAL_KME_ME2_Get1stPFV1CandiOffsetNum(VOID);

/**
 * @brief This function get 1st update candidate number
 * @param [in] VOID
 * @retval u32Val
*/
unsigned int HAL_KME_ME2_Get1stUpdateCandiNums(VOID);

/**
 * @brief This function set block logo rim
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_KME_LOGO_Set_BlockRim(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right);
/**
 * @brief This function set pixel pixel rim
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_KME_LOGO_Set_PixelRim(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right);

/**
 * @brief This function set ME2 double scan enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_KME_Set_ME2DoubleScanEnable(BOOL bEnable);

/**
 * @brief This function set me vflip
 * @param [in] BOOL bVflip
 * @retval VOID
*/
VOID HAL_KME_SetVflip(BOOL bVflip);

/**
 * @brief This function set me vbuf_en
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_KME_Set_Vbuf_Enable(BOOL bEnable);

#endif

/**
 * @brief This function set me2 cand setting
 * @param [in] _str_ME2_Candidate_table *CandiTbl
 * @retval VOID
*/
VOID HAL_ME2_cand_wrtDriver(_str_ME2_Candidate_table *CandiTbl);

/**
 * @brief This function set me1 cand setting
 * @param [in] _str_ME1_Candidate_table *CandiTbl
 * @retval VOID
*/
VOID HAL_ME1_cand_wrtDriver( _str_ME1_Candidate_table *CandiTbl);

#endif

