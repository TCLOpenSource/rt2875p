#if 0
// INCLUDE FILES
#endif

#if 0 // K8_bring_up
//#include "io.h"
#include <tvscalercontrol/io/ioregdrv.h>
#include "memc_isr/Platform/memc_change_size.h"
#include "memc_reg_def.h"
#include "memc_isr/PQL/PQLPlatformDefs.h"
#include "memc_isr/include/PQLAPI.h"
#include <tvscalercontrol/panel/panelapi.h>


#define MERLIN6 1
#define Min(a, b)   ( ((a) < (b)) ? (a) : (b) )
#if 0
// Global Parameters
#endif
static MEMC_SIZE_PARAM gMEMC_SizeParamTable[MEMC_SIZE_NUM] =
{
	/* 2K */ {	2200, 1920, 1080, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 6, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 2, 5, 1, 0, 0, 0 },
	/* 4K */
	{
	4400, // ImageHTotal
	3840, // ImageHSize
	2160, // ImageVSize
	0, // Mode
	0, // FHD_Mode
	0, // NO_MERLIN4_MEMC_SIM
	0, // debug
	0, // RealTiming
	0, // OutputNumOf4Pxl
	1, // SceneOn
	0, // SearchRangeLimit
	1, // Merlin5Timing
	1, // BringUpLogo
	0, // InputFrameRate
	0, // OutputFrameRate
	1, // ME12_Merge
	1, // FullDMASetting
	0, // MEBaseAddr
	6, // IPPRE_DummyLine
	4, // IPME_DnRatio
	1, // LOGO_Enable
	1, // LOGO_Switch
	1, // ME1_DoubleScan
	1, // ME1_Meander
	1, // ME2_2x2
	1, // ME2_DoubleSacn
	0, // ME2_RefineScan
	1, // LBMC_FullRange
	0, // LBMC_NLineOption
	1, // LBMC_PwrOpt
	2, // MC_LineNum
	4, // PQC_Mode
	0, // PQC_MEMode
	0, // PC_Mode
	0, // PC_RepeatMode
	0, // PC_SelPZ
	0, // 8_buffer
	},
	/* 8K */ {	8800, 7680, 4320, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 6, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 2, 5, 1, 0, 0, 0 },
};

// Global parameter for input
unsigned int GS_image_htotal = 0;
unsigned int GS_image_hsize = 0;
unsigned int GS_image_vsize = 0;
unsigned int GS_MODE = 0;
unsigned int GS_me2_2x2 = 0;
unsigned int GS_me1_double_scan = 0;
unsigned int GS_me2_double_scan = 0;
unsigned int GS_me2_refine_scan = 0;
unsigned int GS_logo_enable = 0;
unsigned int GS_ipme_dn_ratio = 0;
unsigned int GS_fhd_mode = 0;
unsigned int GS_NO_MERLIN4_MEMC_SIM = 0;
unsigned int GS_debug = 0;
unsigned int GS_real_timing = 0;
unsigned int GS_lbmc_full_range = 0;
unsigned int GS_ippre_dummy_line = 0;
unsigned int GS_output_num_of_4xpl = 0;
unsigned int GS_mc_line_num = 0;
unsigned int GS_scene_on = 0;
unsigned int GS_pqc_mode = 0;
unsigned int GS_pc_mode = 0;
unsigned int GS_pc_repeat_mode = 0;
unsigned int GS_pc_sel_pz = 0;
unsigned int GS_sr_limit = 0;
unsigned int GS_lbmc_nline_option = 0;
unsigned int GS_merlin5_timing = 0;
unsigned int GS_bringup_logo = 0;
unsigned int GS_input_frame_rate = 0;
unsigned int GS_output_frame_rate = 0;
unsigned int GS_meander = 0;
unsigned int GS_me12_merge = 0;
unsigned int GS_lbmc_pwr_opt = 0;
unsigned int GS_me_pqc_mode = 0;
unsigned int GS_logo_switch = 0;
unsigned int GS_full_dma_setting = 0;
unsigned int GS_me_base_addr = 0;
unsigned int GS_MC_8_buffer = 0;

// Global paramter for internal calculation
unsigned int GSC_me_end_addr = 0;
unsigned int GSC_mc_end_addr = 0;
unsigned int GS_MCBuffer_Size = 6;
unsigned int GS_pqc_selection = 5;
void MEMC_ChangeSize_init(const unsigned char MEMC_SZ_SEL)
{
	GS_MODE = gMEMC_SizeParamTable[MEMC_SZ_SEL].Mode;
	GS_me2_2x2 = gMEMC_SizeParamTable[MEMC_SZ_SEL].ME2_2x2;
	GS_me1_double_scan = gMEMC_SizeParamTable[MEMC_SZ_SEL].ME1_DoubleScan;
	GS_me2_double_scan = gMEMC_SizeParamTable[MEMC_SZ_SEL].ME2_DoubleSacn;
	GS_me2_refine_scan = gMEMC_SizeParamTable[MEMC_SZ_SEL].ME2_RefreshScan;
	GS_logo_enable = gMEMC_SizeParamTable[MEMC_SZ_SEL].LOGO_Enable;
	GS_ipme_dn_ratio = gMEMC_SizeParamTable[MEMC_SZ_SEL].IPME_DnRatio;
	GS_image_htotal = gMEMC_SizeParamTable[MEMC_SZ_SEL].ImageHTotal;
	GS_image_hsize = gMEMC_SizeParamTable[MEMC_SZ_SEL].ImageHSize;
	GS_image_vsize = gMEMC_SizeParamTable[MEMC_SZ_SEL].ImageVSize;
	GS_fhd_mode = gMEMC_SizeParamTable[MEMC_SZ_SEL].FHD_Mode;
	GS_NO_MERLIN4_MEMC_SIM = gMEMC_SizeParamTable[MEMC_SZ_SEL].NO_MERLIN4_MEMC_SIM;
	GS_debug = gMEMC_SizeParamTable[MEMC_SZ_SEL].debug;
	GS_real_timing = gMEMC_SizeParamTable[MEMC_SZ_SEL].RealTiming;
	GS_lbmc_full_range = gMEMC_SizeParamTable[MEMC_SZ_SEL].LBMC_FullRange;
	GS_ippre_dummy_line = gMEMC_SizeParamTable[MEMC_SZ_SEL].IPPRE_DummyLine;
	GS_output_num_of_4xpl = gMEMC_SizeParamTable[MEMC_SZ_SEL].OutputNumOf4Pxl;
	GS_mc_line_num = gMEMC_SizeParamTable[MEMC_SZ_SEL].MC_LineNum;
	GS_scene_on = gMEMC_SizeParamTable[MEMC_SZ_SEL].SceneOn;
	GS_sr_limit = gMEMC_SizeParamTable[MEMC_SZ_SEL].SearchRangeLimit;
	GS_lbmc_nline_option = gMEMC_SizeParamTable[MEMC_SZ_SEL].LBMC_NLineOption;
	GS_merlin5_timing = gMEMC_SizeParamTable[MEMC_SZ_SEL].Merlin5Timing;
	GS_bringup_logo = gMEMC_SizeParamTable[MEMC_SZ_SEL].BringUpLogo;
	GS_input_frame_rate = gMEMC_SizeParamTable[MEMC_SZ_SEL].InputFrameRate;
	GS_output_frame_rate = gMEMC_SizeParamTable[MEMC_SZ_SEL].OutputFrameRate;
	GS_meander = gMEMC_SizeParamTable[MEMC_SZ_SEL].ME1_Meander;
	GS_me12_merge = gMEMC_SizeParamTable[MEMC_SZ_SEL].ME12_Merge;
	GS_lbmc_pwr_opt = gMEMC_SizeParamTable[MEMC_SZ_SEL].LBMC_PwrOpt;
	GS_pqc_mode = gMEMC_SizeParamTable[MEMC_SZ_SEL].PQC_Mode;
	GS_me_pqc_mode = gMEMC_SizeParamTable[MEMC_SZ_SEL].PQC_MEMode;
	GS_logo_switch = gMEMC_SizeParamTable[MEMC_SZ_SEL].LOGO_Switch;
	GS_full_dma_setting = gMEMC_SizeParamTable[MEMC_SZ_SEL].FullDMASetting;
	GS_me_base_addr = gMEMC_SizeParamTable[MEMC_SZ_SEL].MEBaseAddr;
	GS_pqc_mode = gMEMC_SizeParamTable[MEMC_SZ_SEL].PQC_Mode;
	GS_pc_mode = gMEMC_SizeParamTable[MEMC_SZ_SEL].PC_Mode;
	GS_pc_repeat_mode = gMEMC_SizeParamTable[MEMC_SZ_SEL].PC_RepeatMode;

}
#else
#include "io.h"
#include "memc_isr/Platform/memc_change_size.h"
#include "memc_reg_def.h"
#include "memc_isr/PQL/PQLPlatformDefs.h"
#include "memc_isr/include/PQLAPI.h"
#include <tvscalercontrol/panel/panelapi.h>
#ifndef MIN
#define MIN(a, b)   ( ((a) < (b)) ? (a) : (b) )
#endif
// #define MERLIN6 1
#define MERLIN7
#if 1
// Global Parameters
#endif
static MEMC_SIZE_PARAM gMEMC_SizeParamTable[MEMC_SIZE_NUM] =
{
	/* 2K */ {	2200, 1920, 1080, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 6, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 2, 5, 1, 0, 0, 0, 1, 1, 0, 0 },
	/* 4K */
	{
	4400, // ImageHTotal
	3840, // ImageHSize
	2160, // ImageVSize
	0, // Mode
	0, // FHD_Mode
	0, // NO_MERLIN4_MEMC_SIM
	0, // debug
	0, // RealTiming
	0, // OutputNumOf4Pxl
	1, // SceneOn
	0, // SearchRangeLimit
	1, // Merlin5Timing
	1, // BringUpLogo
	0, // InputFrameRate
	0, // OutputFrameRate
	1, // ME12_Merge
	1, // FullDMASetting
	0, // MEBaseAddr
	6, // IPPRE_DummyLine
	4, // IPME_DnRatio
	1, // LOGO_Enable
	1, // LOGO_Switch
	1, // ME1_DoubleScan
	1, // ME1_Meander
	1, // ME2_2x2
	1, // ME2_DoubleSacn
	0, // ME2_RefineScan
	1, // LBMC_FullRange
	0, // LBMC_NLineOption
	1, // LBMC_PwrOpt
	2, // MC_LineNum
	4, // PQC_Mode
	0, // PQC_MEMode
	0, // PC_Mode
	0, // PC_RepeatMode
	0, // PC_SelPZ
	0, // 8_buffer
	1, // kme_dm_bg_1bit
	1, // kme_me1_bg_mask_en
	0, // normal_hf_prefetch_unit_x2
	0  // normal_hf_sr_unit_x2
	},
	/* 8K */ {	8800, 7680, 4320, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 6, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 2, 5, 1, 0, 0, 0, 1, 1, 0, 0 },
};

// Global parameter for input
unsigned int GS_image_htotal = 0;
unsigned int GS_image_hsize = 0;
unsigned int GS_image_vsize = 0;
unsigned int GS_MODE = 0;
unsigned int GS_me2_2x2 = 0;
unsigned int GS_me1_double_scan = 0;
unsigned int GS_me2_double_scan = 0;
unsigned int GS_me2_refine_scan = 0;
unsigned int GS_logo_enable = 0;
unsigned int GS_ipme_dn_ratio = 0;
unsigned int GS_fhd_mode = 0;
unsigned int GS_NO_MERLIN4_MEMC_SIM = 0;
unsigned int GS_debug = 0;
unsigned int GS_real_timing = 0;
unsigned int GS_lbmc_full_range = 0;
unsigned int GS_ippre_dummy_line = 0;
unsigned int GS_output_num_of_4xpl = 0;
unsigned int GS_mc_line_num = 0;
unsigned int GS_scene_on = 0;
unsigned int GS_pqc_mode = 0;
unsigned int GS_pc_mode = 0;
unsigned int GS_pc_repeat_mode = 0;
unsigned int GS_pc_sel_pz = 0;
unsigned int GS_sr_limit = 0;
unsigned int GS_lbmc_nline_option = 0;
unsigned int GS_merlin5_timing = 0;
unsigned int GS_bringup_logo = 0;
unsigned int GS_input_frame_rate = 0;
unsigned int GS_output_frame_rate = 0;
unsigned int GS_meander = 0;
unsigned int GS_me12_merge = 0;
unsigned int GS_lbmc_pwr_opt = 0;
unsigned int GS_me_pqc_mode = 0;
unsigned int GS_logo_switch = 0;
unsigned int GS_full_dma_setting = 0;
unsigned int GS_me_base_addr = 0;
unsigned int GS_MC_8_buffer = 0;

// Global paramter for internal calculation
unsigned int GSC_me_end_addr = 0;
unsigned int GSC_mc_end_addr = 0;
unsigned int GS_MCBuffer_Size = 6;
unsigned int GS_pqc_selection = 5;

// Mark2 bg new algo
int GS_kme_dm_bg_1bit = 0;
int GS_kme_me1_bg_mask_en = 0;  // bg_mask dma enable

// below fixed value
int GS_normal_hf_prefetch_unit_x2 = 0;
int GS_normal_hf_sr_unit_x2 = 0;

/* GS_pqc_selection
  0 : user mode
  1 : bypass
  2 : lossless
  3 : pc YUV422 frame 18 bits
  4 : pc YUV422 line 18 bits
  5 : 422 frame 10 bits
  6 : 422 line 10 bits
  7 : 420 frame 8 bits
  8 : 420 line 8 bits
  9 : 420 frame 7 bits
  10: 420 line 7 bits
  11: 422 frame 10 bits, repeat mode
  12: 422 line 10 bits, repeat mode
  13: 420 frame 8 bits, repeat mode
  14: 420 line 8 bits, repeat mode
  15: 420 frame 7 bits, repeat mode
  16: 420 line 7 bits, repeat mode
  17: pc YUV444 frame 18 bits
  18: pc YUV444 line 18 bits
  19: pc RGB444 frame 18 bits
  20: pc RGB444 line 18 bits
  21: pc YUV444 frame 30 bits
  22: pc YUV444 line 30 bits
*/

#if 0
// Main function
#endif
void MEMC_ChangeSize_IPPRE(void);
void MEMC_ChangeSize_ME1(void);
void MEMC_ChangeSize_ME2(void);
void MEMC_ChangeSize_DH(void);
void MEMC_ChangeSize_MC(void);
void MEMC_ChangeSize_LOGO(void);
void MEMC_ChangeSize_CRTC(void);
void MEMC_ChangeSize_LBMC(void);
void MEMC_ChangeSize_KME_TOP(void);
void MEMC_ChangeSize_mvinfo_dma(void);
void MEMC_ChangeSize_MC2(void);
void MEMC_ChangeSize_KMC_TOP(void);
void MEMC_ChangeSize_MEDMA(void);
void MEMC_ChangeSize_MCDMA(void);
int range_rand(int a, int b){return a;}
void MEMC_ChangeSize_init(const unsigned char MEMC_SZ_SEL)
{
	GS_MODE = gMEMC_SizeParamTable[MEMC_SZ_SEL].Mode;
	GS_me2_2x2 = gMEMC_SizeParamTable[MEMC_SZ_SEL].ME2_2x2;
	GS_me1_double_scan = gMEMC_SizeParamTable[MEMC_SZ_SEL].ME1_DoubleScan;
	GS_me2_double_scan = gMEMC_SizeParamTable[MEMC_SZ_SEL].ME2_DoubleSacn;
	GS_me2_refine_scan = gMEMC_SizeParamTable[MEMC_SZ_SEL].ME2_RefreshScan;
	GS_logo_enable = gMEMC_SizeParamTable[MEMC_SZ_SEL].LOGO_Enable;
	GS_ipme_dn_ratio = gMEMC_SizeParamTable[MEMC_SZ_SEL].IPME_DnRatio;
	GS_image_htotal = gMEMC_SizeParamTable[MEMC_SZ_SEL].ImageHTotal;
	GS_image_hsize = gMEMC_SizeParamTable[MEMC_SZ_SEL].ImageHSize;
	GS_image_vsize = gMEMC_SizeParamTable[MEMC_SZ_SEL].ImageVSize;
	GS_fhd_mode = gMEMC_SizeParamTable[MEMC_SZ_SEL].FHD_Mode;
	GS_NO_MERLIN4_MEMC_SIM = gMEMC_SizeParamTable[MEMC_SZ_SEL].NO_MERLIN4_MEMC_SIM;
	GS_debug = gMEMC_SizeParamTable[MEMC_SZ_SEL].debug;
	GS_real_timing = gMEMC_SizeParamTable[MEMC_SZ_SEL].RealTiming;
	GS_lbmc_full_range = gMEMC_SizeParamTable[MEMC_SZ_SEL].LBMC_FullRange;
	GS_ippre_dummy_line = gMEMC_SizeParamTable[MEMC_SZ_SEL].IPPRE_DummyLine;
	GS_output_num_of_4xpl = gMEMC_SizeParamTable[MEMC_SZ_SEL].OutputNumOf4Pxl;
	GS_mc_line_num = gMEMC_SizeParamTable[MEMC_SZ_SEL].MC_LineNum;
	GS_scene_on = gMEMC_SizeParamTable[MEMC_SZ_SEL].SceneOn;
	GS_sr_limit = gMEMC_SizeParamTable[MEMC_SZ_SEL].SearchRangeLimit;
	GS_lbmc_nline_option = gMEMC_SizeParamTable[MEMC_SZ_SEL].LBMC_NLineOption;
	GS_merlin5_timing = gMEMC_SizeParamTable[MEMC_SZ_SEL].Merlin5Timing;
	GS_bringup_logo = gMEMC_SizeParamTable[MEMC_SZ_SEL].BringUpLogo;
	GS_input_frame_rate = gMEMC_SizeParamTable[MEMC_SZ_SEL].InputFrameRate;
	GS_output_frame_rate = gMEMC_SizeParamTable[MEMC_SZ_SEL].OutputFrameRate;
	GS_meander = gMEMC_SizeParamTable[MEMC_SZ_SEL].ME1_Meander;
	GS_me12_merge = gMEMC_SizeParamTable[MEMC_SZ_SEL].ME12_Merge;
	GS_lbmc_pwr_opt = gMEMC_SizeParamTable[MEMC_SZ_SEL].LBMC_PwrOpt;
	GS_pqc_mode = gMEMC_SizeParamTable[MEMC_SZ_SEL].PQC_Mode;
	GS_me_pqc_mode = gMEMC_SizeParamTable[MEMC_SZ_SEL].PQC_MEMode;
	GS_logo_switch = gMEMC_SizeParamTable[MEMC_SZ_SEL].LOGO_Switch;
	GS_full_dma_setting = gMEMC_SizeParamTable[MEMC_SZ_SEL].FullDMASetting;
	GS_me_base_addr = gMEMC_SizeParamTable[MEMC_SZ_SEL].MEBaseAddr;
	GS_pqc_mode = gMEMC_SizeParamTable[MEMC_SZ_SEL].PQC_Mode;
	GS_pc_mode = gMEMC_SizeParamTable[MEMC_SZ_SEL].PC_Mode;
	GS_pc_repeat_mode = gMEMC_SizeParamTable[MEMC_SZ_SEL].PC_RepeatMode;
	GS_kme_dm_bg_1bit = gMEMC_SizeParamTable[MEMC_SZ_SEL].kme_dm_bg_1bit;
	GS_kme_me1_bg_mask_en = gMEMC_SizeParamTable[MEMC_SZ_SEL].kme_me1_bg_mask_en;
	GS_normal_hf_prefetch_unit_x2 = gMEMC_SizeParamTable[MEMC_SZ_SEL].normal_hf_prefetch_unit_x2;
	GS_normal_hf_sr_unit_x2 = gMEMC_SizeParamTable[MEMC_SZ_SEL].normal_hf_sr_unit_x2;

}
void MEMC_ChangeSize_main(const unsigned char MEMC_SZ_SEL, unsigned int ME_base_addr, MEMC_INOUT_MODE emom, MEMC_OUTPUT_FRAMERATE Out_FrameRate)
{
	// global paramter assign
	if( MEMC_SZ_SEL >= MEMC_SIZE_NUM )
	{
		return;
	}
	GS_MODE = gMEMC_SizeParamTable[MEMC_SZ_SEL].Mode;
	GS_me2_2x2 = gMEMC_SizeParamTable[MEMC_SZ_SEL].ME2_2x2;
	GS_me1_double_scan = gMEMC_SizeParamTable[MEMC_SZ_SEL].ME1_DoubleScan;
	#if 1
			GS_me2_double_scan = (Out_FrameRate <= OUTPUT_60HZ) ? gMEMC_SizeParamTable[MEMC_SZ_SEL].ME2_DoubleSacn : 0;
	#else
			GS_me2_double_scan = gMEMC_SizeParamTable[MEMC_SZ_SEL].ME2_DoubleSacn;
	#endif
	GS_me2_refine_scan = gMEMC_SizeParamTable[MEMC_SZ_SEL].ME2_RefreshScan;
	GS_logo_enable = gMEMC_SizeParamTable[MEMC_SZ_SEL].LOGO_Enable;
	GS_ipme_dn_ratio = gMEMC_SizeParamTable[MEMC_SZ_SEL].IPME_DnRatio;
	GS_image_htotal = gMEMC_SizeParamTable[MEMC_SZ_SEL].ImageHTotal;
	GS_image_hsize = gMEMC_SizeParamTable[MEMC_SZ_SEL].ImageHSize;
	GS_image_vsize = gMEMC_SizeParamTable[MEMC_SZ_SEL].ImageVSize;
	GS_fhd_mode = gMEMC_SizeParamTable[MEMC_SZ_SEL].FHD_Mode;
	GS_NO_MERLIN4_MEMC_SIM = gMEMC_SizeParamTable[MEMC_SZ_SEL].NO_MERLIN4_MEMC_SIM;
	GS_debug = gMEMC_SizeParamTable[MEMC_SZ_SEL].debug;
	GS_real_timing = gMEMC_SizeParamTable[MEMC_SZ_SEL].RealTiming;
	GS_lbmc_full_range = gMEMC_SizeParamTable[MEMC_SZ_SEL].LBMC_FullRange;
	GS_ippre_dummy_line = gMEMC_SizeParamTable[MEMC_SZ_SEL].IPPRE_DummyLine;
	GS_output_num_of_4xpl = gMEMC_SizeParamTable[MEMC_SZ_SEL].OutputNumOf4Pxl;
	GS_mc_line_num = gMEMC_SizeParamTable[MEMC_SZ_SEL].MC_LineNum;
	GS_scene_on = gMEMC_SizeParamTable[MEMC_SZ_SEL].SceneOn;
	GS_sr_limit = gMEMC_SizeParamTable[MEMC_SZ_SEL].SearchRangeLimit;
	GS_lbmc_nline_option = gMEMC_SizeParamTable[MEMC_SZ_SEL].LBMC_NLineOption;
	GS_merlin5_timing = gMEMC_SizeParamTable[MEMC_SZ_SEL].Merlin5Timing;
	GS_bringup_logo = gMEMC_SizeParamTable[MEMC_SZ_SEL].BringUpLogo;
	GS_input_frame_rate = gMEMC_SizeParamTable[MEMC_SZ_SEL].InputFrameRate;
	GS_output_frame_rate = Out_FrameRate; // gMEMC_SizeParamTable[MEMC_SZ_SEL].OutputFrameRate;
	GS_meander = gMEMC_SizeParamTable[MEMC_SZ_SEL].ME1_Meander;
	GS_me12_merge = gMEMC_SizeParamTable[MEMC_SZ_SEL].ME12_Merge;
	GS_lbmc_pwr_opt = gMEMC_SizeParamTable[MEMC_SZ_SEL].LBMC_PwrOpt;
	GS_pqc_mode = gMEMC_SizeParamTable[MEMC_SZ_SEL].PQC_Mode;
	GS_me_pqc_mode = gMEMC_SizeParamTable[MEMC_SZ_SEL].PQC_MEMode;
	GS_logo_switch = gMEMC_SizeParamTable[MEMC_SZ_SEL].LOGO_Switch;
	GS_full_dma_setting = gMEMC_SizeParamTable[MEMC_SZ_SEL].FullDMASetting;
	GS_me_base_addr = ME_base_addr; // gMEMC_SizeParamTable[MEMC_SZ_SEL].MEBaseAddr;
	GS_kme_dm_bg_1bit = gMEMC_SizeParamTable[MEMC_SZ_SEL].kme_dm_bg_1bit;
	GS_kme_me1_bg_mask_en = gMEMC_SizeParamTable[MEMC_SZ_SEL].kme_me1_bg_mask_en;
	GS_normal_hf_prefetch_unit_x2 = gMEMC_SizeParamTable[MEMC_SZ_SEL].normal_hf_prefetch_unit_x2;
	GS_normal_hf_sr_unit_x2 = gMEMC_SizeParamTable[MEMC_SZ_SEL].normal_hf_sr_unit_x2;
	#if CONFIG_MC_8_BUFFER
	GS_MC_8_buffer = 1;
	#else
	GS_MC_8_buffer = gMEMC_SizeParamTable[MEMC_SZ_SEL].MC_8_buffer;
	#endif
	if((emom == MEMC_RGB_IN_VIDEO_OUT) || (emom == MEMC_YUV_IN_VIDEO_OUT) || (emom == MEMC_RGB_IN_VIDEO_FREERUN_OUT) || (emom == MEMC_RGB_IN_PC_FREERUN_OUT)){
		if((emom == MEMC_RGB_IN_VIDEO_FREERUN_OUT) || (emom == MEMC_RGB_IN_PC_FREERUN_OUT)){
			GS_MCBuffer_Size = 4;
			GS_pqc_selection = 5;
		}
		else if(GS_MC_8_buffer){
			GS_MCBuffer_Size = 8;
			if(Out_FrameRate > OUTPUT_60HZ){
				GS_pqc_selection = 5;
			}
			else{
				GS_pqc_selection = 7;
			}
		}else{
			GS_MCBuffer_Size = 6;
			GS_pqc_selection = 5;
		}
	}
	else{
		GS_MCBuffer_Size = 3;
		GS_pqc_selection = 19;
	}
	rtd_pr_memc_notice("[%s][%d][%d,%d,%d]\n\r", __FUNCTION__, __LINE__, GS_MC_8_buffer, GS_MCBuffer_Size, GS_pqc_selection);
	GS_pc_sel_pz = gMEMC_SizeParamTable[MEMC_SZ_SEL].PC_SelPZ;

	switch (GS_pqc_selection)
	{
		case 0:
			break;
		case 1:
			GS_pqc_mode = 0;
			GS_pc_mode = 0;
			GS_pc_repeat_mode = 0;
			break;
		case 2:
			GS_pqc_mode = 1;
			GS_pc_mode = 0;
			GS_pc_repeat_mode = 0;
			break;
		case 3:
			GS_pqc_mode = 2;
			GS_pc_mode = 1;
			GS_pc_repeat_mode = 1;
			break;
		case 4:
			GS_pqc_mode = 3;
			GS_pc_mode = 1;
			GS_pc_repeat_mode = 1;
			break;
		case 5:
			GS_pqc_mode = 4;
			GS_pc_mode = 0;
			GS_pc_repeat_mode = 0;
			break;
		case 6:
			GS_pqc_mode = 5;
			GS_pc_mode = 0;
			GS_pc_repeat_mode = 0;
			break;
		case 7:
			GS_pqc_mode = 6;
			GS_pc_mode = 0;
			GS_pc_repeat_mode = 0;
			break;
		case 8:
			GS_pqc_mode = 7;
			GS_pc_mode = 0;
			GS_pc_repeat_mode = 0;
			break;
		case 9:
			GS_pqc_mode = 8;
			GS_pc_mode = 0;
			GS_pc_repeat_mode = 0;
			break;
		case 10:
			GS_pqc_mode = 9;
			GS_pc_mode = 0;
			GS_pc_repeat_mode = 0;
			break;
		case 11:
			GS_pqc_mode = 4;
			GS_pc_mode = 1;
			GS_pc_repeat_mode = 1;
			break;
		case 12:
			GS_pqc_mode = 5;
			GS_pc_mode = 1;
			GS_pc_repeat_mode = 1;
			break;
		case 13:
			GS_pqc_mode = 6;
			GS_pc_mode = 1;
			GS_pc_repeat_mode = 1;
			break;
		case 14:
			GS_pqc_mode = 7;
			GS_pc_mode = 1;
			GS_pc_repeat_mode = 1;
			break;
		case 15:
			GS_pqc_mode = 8;
			GS_pc_mode = 1;
			GS_pc_repeat_mode = 1;
			break;
		case 16:
			GS_pqc_mode = 9;
			GS_pc_mode = 1;
			GS_pc_repeat_mode = 1;
			break;
		case 17:
			GS_pqc_mode = 10;
			GS_pc_mode = 1;
			GS_pc_repeat_mode = 1;
			break;
		case 18:
			GS_pqc_mode = 11;
			GS_pc_mode = 1;
			GS_pc_repeat_mode = 1;
			break;
		case 19:
			GS_pqc_mode = 12;
			GS_pc_mode = 1;
			GS_pc_repeat_mode = 1;
			break;
		case 20:
			GS_pqc_mode = 13;
			GS_pc_mode = 1;
			GS_pc_repeat_mode = 1;
			break;
		case 21:
			GS_pqc_mode = 14;
			GS_pc_mode = 1;
			GS_pc_repeat_mode = 1;
			break;
		case 22:
			GS_pqc_mode = 15;
			GS_pc_mode = 1;
			GS_pc_repeat_mode = 1;
			break;
		default:
			GS_pqc_mode = gMEMC_SizeParamTable[MEMC_SZ_SEL].PQC_Mode;
			GS_pc_mode = gMEMC_SizeParamTable[MEMC_SZ_SEL].PC_Mode;
			GS_pc_repeat_mode = gMEMC_SizeParamTable[MEMC_SZ_SEL].PC_RepeatMode;
			break;
	}

	// Change size main
	MEMC_ChangeSize_IPPRE();
	MEMC_ChangeSize_ME1();
	MEMC_ChangeSize_ME2();
	MEMC_ChangeSize_DH();
	MEMC_ChangeSize_MC();
	MEMC_ChangeSize_LOGO();
	MEMC_ChangeSize_CRTC();
	MEMC_ChangeSize_LBMC();
	MEMC_ChangeSize_KME_TOP();
	MEMC_ChangeSize_mvinfo_dma();
	MEMC_ChangeSize_MC2();
	MEMC_ChangeSize_KMC_TOP();
	MEMC_ChangeSize_MEDMA();
	MEMC_ChangeSize_MCDMA();

}

void MEMC_RTICE_Trigger()
{
	// Change size main
	MEMC_ChangeSize_IPPRE();
	MEMC_ChangeSize_ME1();
	MEMC_ChangeSize_ME2();
	MEMC_ChangeSize_DH();
	MEMC_ChangeSize_MC();
	MEMC_ChangeSize_LOGO();
	MEMC_ChangeSize_CRTC();
	MEMC_ChangeSize_LBMC();
	MEMC_ChangeSize_KME_TOP();
	MEMC_ChangeSize_mvinfo_dma();
	MEMC_ChangeSize_MC2();
	MEMC_ChangeSize_KMC_TOP();
	MEMC_ChangeSize_MEDMA();
	MEMC_ChangeSize_MCDMA();
}

void MEMC_RTICE_SetParam(MEMC_SIZE_PARAM* ptr)
{
	GS_MODE = ptr->Mode;
	GS_me2_2x2 = ptr->ME2_2x2;
	GS_me1_double_scan = ptr->ME1_DoubleScan;
	GS_me2_double_scan = ptr->ME2_DoubleSacn;
	GS_me2_refine_scan = ptr->ME2_RefreshScan;
	GS_logo_enable = ptr->LOGO_Enable;
	GS_ipme_dn_ratio = ptr->IPME_DnRatio;
	GS_image_htotal = ptr->ImageHTotal;
	GS_image_hsize = ptr->ImageHSize;
	GS_image_vsize = ptr->ImageVSize;
	GS_fhd_mode = ptr->FHD_Mode;
	GS_NO_MERLIN4_MEMC_SIM = ptr->NO_MERLIN4_MEMC_SIM;
	GS_debug = ptr->debug;
	GS_real_timing = ptr->RealTiming;
	GS_lbmc_full_range = ptr->LBMC_FullRange;
	GS_ippre_dummy_line = ptr->IPPRE_DummyLine;
	GS_output_num_of_4xpl = ptr->OutputNumOf4Pxl;
	GS_mc_line_num = ptr->MC_LineNum;
	GS_scene_on = ptr->SceneOn;
	GS_pqc_mode = ptr->PQC_Mode;
	GS_pc_mode = ptr->PC_Mode;
	GS_pc_repeat_mode = ptr->PC_RepeatMode;
	GS_pc_sel_pz = ptr->PC_SelPZ;
	GS_sr_limit = ptr->SearchRangeLimit;
	GS_lbmc_nline_option = ptr->LBMC_NLineOption;
	GS_merlin5_timing = ptr->Merlin5Timing;
	GS_bringup_logo = ptr->BringUpLogo;
	GS_input_frame_rate = ptr->InputFrameRate;
	GS_output_frame_rate = ptr->OutputFrameRate;
	GS_meander = ptr->ME1_Meander;
	GS_me12_merge = ptr->ME12_Merge;
	GS_lbmc_pwr_opt = ptr->LBMC_PwrOpt;
	GS_me_pqc_mode = ptr->PQC_MEMode;
	GS_logo_switch = ptr->LOGO_Switch;
	GS_full_dma_setting = ptr->FullDMASetting;
	GS_me_base_addr = ptr->MEBaseAddr;
}

void MEMC_RTICE_GetParam(MEMC_SIZE_PARAM* ptr)
{
	ptr->Mode = GS_MODE;
	ptr->ME2_2x2 = GS_me2_2x2;
	ptr->ME1_DoubleScan = GS_me1_double_scan;
	ptr->ME2_DoubleSacn = GS_me2_double_scan;
	ptr->ME2_RefreshScan = GS_me2_refine_scan;
	ptr->LOGO_Enable = GS_logo_enable;
	ptr->IPME_DnRatio = GS_ipme_dn_ratio;
	ptr->ImageHTotal = GS_image_htotal;
	ptr->ImageHSize = GS_image_hsize;
	ptr->ImageVSize = GS_image_vsize;
	ptr->FHD_Mode = GS_fhd_mode;
	ptr->NO_MERLIN4_MEMC_SIM = GS_NO_MERLIN4_MEMC_SIM;
	ptr->debug = GS_debug;
	ptr->RealTiming = GS_real_timing;
	ptr->LBMC_FullRange = GS_lbmc_full_range;
	ptr->IPPRE_DummyLine = GS_ippre_dummy_line;
	ptr->OutputNumOf4Pxl = GS_output_num_of_4xpl;
	ptr->MC_LineNum = GS_mc_line_num;
	ptr->SceneOn = GS_scene_on;
	ptr->PQC_Mode = GS_pqc_mode;
	ptr->PC_Mode = GS_pc_mode;
	ptr->PC_RepeatMode = GS_pc_repeat_mode;
	ptr->PC_SelPZ = GS_pc_sel_pz;
	ptr->SearchRangeLimit = GS_sr_limit;
	ptr->LBMC_NLineOption = GS_lbmc_nline_option;
	ptr->Merlin5Timing = GS_merlin5_timing;
	ptr->BringUpLogo = GS_bringup_logo;
	ptr->InputFrameRate = GS_input_frame_rate;
	ptr->OutputFrameRate = GS_output_frame_rate;
	ptr->ME1_Meander = GS_meander;
	ptr->ME12_Merge = GS_me12_merge;
	ptr->LBMC_PwrOpt = GS_lbmc_pwr_opt;
	ptr->PQC_MEMode = GS_me_pqc_mode;
	ptr->LOGO_Switch = GS_logo_switch;
	ptr->FullDMASetting = GS_full_dma_setting;
	ptr->MEBaseAddr = GS_me_base_addr;
}

#if 1 // AUTO_GEN at 04/29/21 19:11:03
void MEMC_ChangeSize_CRTC(void)
{
	// parameter declaration
	//======= out timing
	int out_h_div = 2<<GS_output_num_of_4xpl;
	int hsync_width = (GS_image_htotal - GS_image_hsize)/4;
	int h_bporch = hsync_width;

	//======= vtrig
	int me2_one_row_eq_mc_row_num;
	int me2_work_mode = 0;

	int dh_pre_flt_vtrig =0;

	int dh_pst_flt_vtrig;
	int dh_extcor_vtrig;
	int dh_extend_vtrig;
	int mc_vtrig;
	//skyreddint lbmc_lfi	     = 29;
	int lbmc_lfi	     = 29+128;
	int me2_vtrig        = 21;
#if (defined(MARK2)||defined(MERLIN7)||defined(MERLIN6)||defined(UNUSED_VARIABLE))
	int dec_vtrig_ic = (GS_image_vsize / 2 + 50);
	int lbme1_vtrig_ic = (GS_image_vsize - 6);
	int lbme2_vtrig_ic = (GS_image_vsize - 6);
	//skyreddint lbmc_vtrig_ic = (GS_image_vsize - 60);
	int lbmc_vtrig_ic = (GS_image_vsize - (lbmc_lfi+1)*2); //stutter if setting wrong, ex:#100
#endif

	int dec_vtrig_sim;
	int lbme1_vtrig_sim;
	int lbme2_vtrig_sim;
	int lbmc_vtrig_sim;

	int dec_vtrig;
	int lbme1_vtrig;
	int lbme2_vtrig;
	int lbmc_vtrig;

	int org_vtrig_dly = 5;
	int me1_vtrig = 18;
	int crtc1_ip_vtrig_dly = 16;
	int frc_vtrig_dly = 0;
	int out_vs_width = 8;
	int frc_vs_bporch = 35;
	//nouse//int frc_prede_lnum = 0;

	int out_hs_width = hsync_width/out_h_div;
	int frc_prede_pnum = 25;

	int mc_hdly = 458;
	int frc_htrig_dly;
	int frc_htrig_dly_div2;

	int mc_pxl_logo_v_size;

	int dec_vtrig_mid,dh_extend_vtrig_mid; // init

	// register declaration
	crtc1_crtc1_00_RBUS crtc1_crtc1_00_reg;
	crtc1_crtc1_14_RBUS crtc1_crtc1_14_reg;
	crtc1_crtc1_18_RBUS crtc1_crtc1_18_reg;
	crtc1_crtc1_1c_RBUS crtc1_crtc1_1c_reg;
	crtc1_crtc1_20_RBUS crtc1_crtc1_20_reg;
	crtc1_crtc1_24_RBUS crtc1_crtc1_24_reg;
	crtc1_crtc1_28_RBUS crtc1_crtc1_28_reg;
	crtc1_crtc1_2c_RBUS crtc1_crtc1_2c_reg;
	crtc1_crtc1_30_RBUS crtc1_crtc1_30_reg;
	crtc1_crtc1_34_RBUS crtc1_crtc1_34_reg;
	crtc1_crtc1_38_RBUS crtc1_crtc1_38_reg;
	crtc1_crtc1_40_RBUS crtc1_crtc1_40_reg;
	crtc1_crtc1_44_RBUS crtc1_crtc1_44_reg;
	crtc1_crtc1_48_RBUS crtc1_crtc1_48_reg;
	crtc1_crtc1_88_RBUS crtc1_crtc1_88_reg;
	crtc1_crtc1_8c_RBUS crtc1_crtc1_8c_reg;
	crtc1_crtc1_94_RBUS crtc1_crtc1_94_reg;
	crtc1_crtc1_98_RBUS crtc1_crtc1_98_reg;
	crtc1_crtc1_9c_RBUS crtc1_crtc1_9c_reg;

	// register read back
	crtc1_crtc1_00_reg.regValue = rtd_inl(CRTC1_CRTC1_00_reg);
	crtc1_crtc1_14_reg.regValue = rtd_inl(CRTC1_CRTC1_14_reg);
	crtc1_crtc1_18_reg.regValue = rtd_inl(CRTC1_CRTC1_18_reg);
	crtc1_crtc1_1c_reg.regValue = rtd_inl(CRTC1_CRTC1_1C_reg);
	crtc1_crtc1_20_reg.regValue = rtd_inl(CRTC1_CRTC1_20_reg);
	crtc1_crtc1_24_reg.regValue = rtd_inl(CRTC1_CRTC1_24_reg);
	crtc1_crtc1_28_reg.regValue = rtd_inl(CRTC1_CRTC1_28_reg);
	crtc1_crtc1_2c_reg.regValue = rtd_inl(CRTC1_CRTC1_2C_reg);
	crtc1_crtc1_30_reg.regValue = rtd_inl(CRTC1_CRTC1_30_reg);
	crtc1_crtc1_34_reg.regValue = rtd_inl(CRTC1_CRTC1_34_reg);
	crtc1_crtc1_38_reg.regValue = rtd_inl(CRTC1_CRTC1_38_reg);
	crtc1_crtc1_40_reg.regValue = rtd_inl(CRTC1_CRTC1_40_reg);
	crtc1_crtc1_44_reg.regValue = rtd_inl(CRTC1_CRTC1_44_reg);
	crtc1_crtc1_48_reg.regValue = rtd_inl(CRTC1_CRTC1_48_reg);
	crtc1_crtc1_88_reg.regValue = rtd_inl(CRTC1_CRTC1_88_reg);
	crtc1_crtc1_8c_reg.regValue = rtd_inl(CRTC1_CRTC1_8C_reg);
	crtc1_crtc1_94_reg.regValue = rtd_inl(CRTC1_CRTC1_94_reg);
	crtc1_crtc1_98_reg.regValue = rtd_inl(CRTC1_CRTC1_98_reg);
	crtc1_crtc1_9c_reg.regValue = rtd_inl(CRTC1_CRTC1_9C_reg);

	// main function body
	if((GS_ipme_dn_ratio == 8) &  (GS_me2_2x2 == 1)) //H5  8k MC16x16
	{
		me2_one_row_eq_mc_row_num = 16;
		me2_work_mode = 0;
	}
	else if((GS_ipme_dn_ratio == 4) &  (GS_me2_2x2 == 0)) //H5 4k MC16x16
	{
		me2_one_row_eq_mc_row_num = 16;
		me2_work_mode = 0;
	}
	else if((GS_ipme_dn_ratio == 4) &  (GS_me2_2x2 == 1)) // 4K MC8x8
	{
		me2_one_row_eq_mc_row_num = 8;
		me2_work_mode = 1;
	}
	else //if((GS_ipme_dn_ratio == 2) &  (GS_me2_2x2 == 0)) //2k MC8x8
	{
		if(GS_me2_2x2){
			me2_work_mode =2;
			me2_one_row_eq_mc_row_num = 4;
		}
		else {
			me2_work_mode =1;
			me2_one_row_eq_mc_row_num = 8;
		}
	}

	if(GS_me2_double_scan){
		dh_pre_flt_vtrig = 0                        + me2_one_row_eq_mc_row_num * 4;
	}
	else{
		dh_pre_flt_vtrig = 0                        + me2_one_row_eq_mc_row_num * 2;
	}

	dh_pst_flt_vtrig = dh_pre_flt_vtrig 	+ me2_one_row_eq_mc_row_num * 2;
	dh_extcor_vtrig  = dh_pst_flt_vtrig 	+ me2_one_row_eq_mc_row_num * 2;
	dh_extend_vtrig  = dh_extcor_vtrig		+ me2_one_row_eq_mc_row_num * 4;
	mc_vtrig	     = dh_extend_vtrig		+ me2_one_row_eq_mc_row_num * 5;

	dec_vtrig_sim = mc_vtrig + 4;
	lbme1_vtrig_sim = dec_vtrig_sim + 2;
	lbme2_vtrig_sim = dec_vtrig_sim + 2;
	lbmc_vtrig_sim = GS_image_vsize - lbmc_lfi*2 - 2 ;
	
#if defined(MERLIN6)
	dec_vtrig   = (GS_image_vsize >= 240)? dec_vtrig_ic		:dec_vtrig_sim;
	lbme1_vtrig = (GS_image_vsize >= 240)? lbme1_vtrig_ic	:lbme1_vtrig_sim;
	lbme2_vtrig = (GS_image_vsize >= 240)? lbme2_vtrig_ic	:lbme2_vtrig_sim;
	lbmc_vtrig  = (GS_image_vsize >= 240)? lbmc_vtrig_ic	:lbmc_vtrig_sim;
#endif
#if defined(MARK2) || defined(MERLIN7)
	//dec_vtrig   = (dec_vtrig_ic > mc_vtrig)? dec_vtrig_ic		:dec_vtrig_sim;  
	dec_vtrig   = dec_vtrig_ic;
	lbme1_vtrig = lbme1_vtrig_ic;
	lbme2_vtrig = lbme2_vtrig_ic;
	lbmc_vtrig  = lbmc_vtrig_ic; 
#endif

	org_vtrig_dly = 5;
	me1_vtrig = 18;
	me2_vtrig = 18;

	dec_vtrig_mid=dec_vtrig;
	dh_extend_vtrig_mid=dh_extend_vtrig;

	if(GS_me12_merge) {
		crtc1_ip_vtrig_dly =0;
		org_vtrig_dly=0;
		frc_vtrig_dly=frc_vtrig_dly+16;
		mc_vtrig = mc_vtrig+ 25;
		dec_vtrig = dec_vtrig_mid+0x1e;
		dh_extend_vtrig  = dh_extend_vtrig_mid+0x19;
		dh_extcor_vtrig  = dh_extend_vtrig-2;
	}


	frc_vtrig_dly = (mc_vtrig + me2_vtrig + org_vtrig_dly + 1) - (out_vs_width + frc_vs_bporch) + 1;
#if defined(MERLIN6)
	frc_htrig_dly_div2 = (mc_hdly + 32 + 92 - (out_hs_width + h_bporch))/2;
#endif
#if defined(MARK2) || defined(MERLIN7)
	frc_htrig_dly_div2 = (mc_hdly + 32 + 122 - (out_hs_width + h_bporch))/2;
#endif
	frc_htrig_dly = frc_htrig_dly_div2*2 - 1;

#if defined(MARK2) || defined(MERLIN7)
	dec_vtrig   = (dec_vtrig > mc_vtrig)? dec_vtrig : mc_vtrig+4;
	lbme1_vtrig   = (lbme1_vtrig > dec_vtrig)? lbme1_vtrig : dec_vtrig+4;
	lbme2_vtrig   = (lbme2_vtrig > dec_vtrig)? lbme2_vtrig : dec_vtrig+4;
#endif
	//======= register
	crtc1_crtc1_00_reg.crtc1_out_hs_width = hsync_width / out_h_div;
	crtc1_crtc1_00_reg.crtc1_out_htotal =  GS_image_htotal / out_h_div;
	crtc1_crtc1_14_reg.crtc1_frc_hact = GS_image_hsize / out_h_div;
	crtc1_crtc1_14_reg.crtc1_frc_vact = GS_image_vsize;
	crtc1_crtc1_14_reg.crtc1_frc_prede_pnum = frc_prede_pnum;
	crtc1_crtc1_18_reg.crtc1_frc_hs_bporch = hsync_width / out_h_div;
	crtc1_crtc1_1c_reg.crtc1_me2_org_vtrig_dly = org_vtrig_dly;
	crtc1_crtc1_1c_reg.crtc1_ip_vtrig_dly = crtc1_ip_vtrig_dly;
	crtc1_crtc1_20_reg.crtc1_frc_vtrig_dly = frc_vtrig_dly;
	crtc1_crtc1_20_reg.crtc1_out_vtrig_dly = 0;
	crtc1_crtc1_24_reg.crtc1_lbme2_vtrig_dly = lbme2_vtrig;
	crtc1_crtc1_24_reg.crtc1_dec_vtrig_dly = dec_vtrig;
	crtc1_crtc1_24_reg.crtc1_me2_vtrig_dly = me2_vtrig;
	crtc1_crtc1_28_reg.crtc1_lbmc_vtrig_dly = lbmc_vtrig;
	crtc1_crtc1_28_reg.crtc1_mc_vtrig_dly = mc_vtrig;
	crtc1_crtc1_2c_reg.crtc1_lbmc_lfidx0_dly = lbmc_lfi;
	crtc1_crtc1_2c_reg.crtc1_lbmc_lfidx1_dly = lbmc_lfi;
	crtc1_crtc1_2c_reg.crtc1_lbmc_lfidx2_dly = lbmc_lfi;
	crtc1_crtc1_2c_reg.crtc1_lbmc_lfidx3_dly = lbmc_lfi;
	crtc1_crtc1_30_reg.crtc1_lbmc_lfidx4_dly = lbmc_lfi;
	crtc1_crtc1_30_reg.crtc1_lbmc_lfidx5_dly = lbmc_lfi;
	crtc1_crtc1_30_reg.crtc1_lbmc_lfidx6_dly = lbmc_lfi;
	crtc1_crtc1_30_reg.crtc1_lbmc_lfidx7_dly = lbmc_lfi;
	crtc1_crtc1_34_reg.crtc1_lbmc_lfidx8_dly = lbmc_lfi;
	//skyreddcrtc1_crtc1_38_reg.crtc1_lbmc_hfcidx0_dly = 29;
	crtc1_crtc1_38_reg.crtc1_lbmc_hfcidx0_dly = lbmc_lfi;
	crtc1_crtc1_38_reg.crtc1_frc_hdly = frc_htrig_dly;
	crtc1_crtc1_40_reg.crtc1_me2_row_type	= me2_work_mode;
	crtc1_crtc1_40_reg.crtc1_dehalo_vact	= GS_image_vsize + 24; 
	crtc1_crtc1_40_reg.crtc1_me2_vact	= GS_image_vsize + 24;
	crtc1_crtc1_44_reg.crtc1_mc_vact	= GS_image_vsize / GS_mc_line_num;


	crtc1_crtc1_48_reg.crtc1_plogo_fetch_row_num = 20;
	if(GS_ipme_dn_ratio==4){
		crtc1_crtc1_48_reg.crtc1_plogo_row_num = GS_image_vsize/2;
		crtc1_crtc1_48_reg.crtc1_plogo_fetch_row_type = 1;
		crtc1_crtc1_48_reg.crtc1_plogo_row_type = 1;
	}
	else{
		crtc1_crtc1_48_reg.crtc1_plogo_row_num = GS_image_vsize/2;
		crtc1_crtc1_48_reg.crtc1_plogo_fetch_row_type = 2;
		crtc1_crtc1_48_reg.crtc1_plogo_row_type = 2;
	}
#if defined(MARK2)
	crtc1_crtc1_48_reg.crtc1_plogo_row_num = GS_image_vsize/GS_ipme_dn_ratio;
	if(GS_ipme_dn_ratio==8){
		crtc1_crtc1_48_reg.crtc1_plogo_fetch_row_type = 0;
		crtc1_crtc1_48_reg.crtc1_plogo_row_type = 0;
	}
	else if(GS_ipme_dn_ratio==4){
		crtc1_crtc1_48_reg.crtc1_plogo_fetch_row_type = 1;
		crtc1_crtc1_48_reg.crtc1_plogo_row_type = 1;
	}
	else{
		crtc1_crtc1_48_reg.crtc1_plogo_fetch_row_type = 2;
		crtc1_crtc1_48_reg.crtc1_plogo_row_type = 2;
	}      
#endif	

	crtc1_crtc1_88_reg.crtc1_lbme1_vtrig_dly= lbme1_vtrig;
	crtc1_crtc1_88_reg.crtc1_me1_vtrig_dly = me1_vtrig;
	crtc1_crtc1_88_reg.crtc1_me1_org_vtrig_dly = org_vtrig_dly;
	crtc1_crtc1_8c_reg.crtc1_me1_vact	= GS_image_vsize + 24;
	crtc1_crtc1_94_reg.crtc1_me2dh_extcorr_vtrig_dly	= dh_extcor_vtrig;
	crtc1_crtc1_94_reg.crtc1_me2dh_dobflt_vtrig_dly		= dh_extend_vtrig;
	crtc1_crtc1_94_reg.crtc1_me2dh_pstflt_vtrig_dly		= dh_pst_flt_vtrig;
	crtc1_crtc1_94_reg.crtc1_me2dh_preflt_vtrig_dly		= dh_pre_flt_vtrig;
#if defined(MARK2) || defined(MERLIN7)
	crtc1_crtc1_98_reg.crtc1_out_htotal_long =  GS_image_htotal / out_h_div;
	crtc1_crtc1_98_reg.crtc1_mc_vtrig_dly_long = mc_vtrig;
	crtc1_crtc1_9c_reg.crtc1_me2dh_extcorr_vtrig_dly_long	= dh_extcor_vtrig;
	crtc1_crtc1_9c_reg.crtc1_me2dh_dobflt_vtrig_dly_long	= dh_extend_vtrig;
	crtc1_crtc1_98_reg.crtc1_vtrig_long_en = 1 ;
#endif	

	//====== MC LOGO
	if(GS_ipme_dn_ratio == 8) //8K
	{
		mc_pxl_logo_v_size = 2 ;  
	}
	else if(GS_ipme_dn_ratio == 4) //4K
	{
		mc_pxl_logo_v_size = 1 ;
	}
	else //2K
	{
		mc_pxl_logo_v_size = 0 ;  
	}

	// register write
	rtd_outl(CRTC1_CRTC1_00_reg, crtc1_crtc1_00_reg.regValue);
	rtd_outl(CRTC1_CRTC1_14_reg, crtc1_crtc1_14_reg.regValue);
	rtd_outl(CRTC1_CRTC1_18_reg, crtc1_crtc1_18_reg.regValue);
	rtd_outl(CRTC1_CRTC1_1C_reg, crtc1_crtc1_1c_reg.regValue);
	rtd_outl(CRTC1_CRTC1_20_reg, crtc1_crtc1_20_reg.regValue);
	rtd_outl(CRTC1_CRTC1_24_reg, crtc1_crtc1_24_reg.regValue);
	rtd_outl(CRTC1_CRTC1_28_reg, crtc1_crtc1_28_reg.regValue);
	rtd_outl(CRTC1_CRTC1_2C_reg, crtc1_crtc1_2c_reg.regValue);
	rtd_outl(CRTC1_CRTC1_30_reg, crtc1_crtc1_30_reg.regValue);
	rtd_outl(CRTC1_CRTC1_34_reg, crtc1_crtc1_34_reg.regValue);
	rtd_outl(CRTC1_CRTC1_38_reg, crtc1_crtc1_38_reg.regValue);
	rtd_outl(CRTC1_CRTC1_40_reg, crtc1_crtc1_40_reg.regValue);
	rtd_outl(CRTC1_CRTC1_44_reg, crtc1_crtc1_44_reg.regValue);
	rtd_outl(CRTC1_CRTC1_48_reg, crtc1_crtc1_48_reg.regValue);
	rtd_outl(CRTC1_CRTC1_88_reg, crtc1_crtc1_88_reg.regValue);
	rtd_outl(CRTC1_CRTC1_8C_reg, crtc1_crtc1_8c_reg.regValue);
	rtd_outl(CRTC1_CRTC1_94_reg, crtc1_crtc1_94_reg.regValue);
	rtd_outl(CRTC1_CRTC1_98_reg, crtc1_crtc1_98_reg.regValue);
	rtd_outl(CRTC1_CRTC1_9C_reg, crtc1_crtc1_9c_reg.regValue);

}
void MEMC_ChangeSize_DH(void)
{
	// parameter declaration

	// register declaration

	// register read back

	// main function body

	// register write

}
void MEMC_ChangeSize_IPPRE(void)
{
	// parameter declaration
	int dummy_line;
	int pc_mode_turn_on;
	int pc_mode_turn_off;
	int indata_pc_mode_turn_on;
	int indata_pc_mode_turn_off;

	// register declaration
	ippre_ippre_04_RBUS ippre_ippre_04_reg;
	ippre_ippre_08_RBUS ippre_ippre_08_reg;
	ippre_ippre_10_RBUS ippre_ippre_10_reg;
	ippre_ippre_54_RBUS ippre_ippre_54_reg;
	ippre_ippre_64_RBUS ippre_ippre_64_reg;
	ippre_ippre_b8_RBUS ippre_ippre_b8_reg;
	ippre_ippre_bc_RBUS ippre_ippre_bc_reg;
	ippre_ippre_e0_RBUS ippre_ippre_e0_reg;
	ippre_ippre_e4_RBUS ippre_ippre_e4_reg;
	ippre1_ippre1_color_prot_2_RBUS ippre1_ippre1_color_prot_2_reg;
	ippre1_ippre1_54_RBUS ippre1_ippre1_54_reg;
	ippre1_ippre1_64_RBUS ippre1_ippre1_64_reg;
	ippre1_ippre1_b8_RBUS ippre1_ippre1_b8_reg;
	ippre1_ippre1_bc_RBUS ippre1_ippre1_bc_reg;
	ippre1_ippre1_c0_RBUS ippre1_ippre1_c0_reg;
	ippre1_ippre1_c4_RBUS ippre1_ippre1_c4_reg;

	// register read back
	ippre_ippre_04_reg.regValue = rtd_inl(IPPRE_IPPRE_04_reg);
	ippre_ippre_08_reg.regValue = rtd_inl(IPPRE_IPPRE_08_reg);
	ippre_ippre_10_reg.regValue = rtd_inl(IPPRE_IPPRE_10_reg);
	ippre_ippre_54_reg.regValue = rtd_inl(IPPRE_IPPRE_54_reg);
	ippre_ippre_64_reg.regValue = rtd_inl(IPPRE_IPPRE_64_reg);
	ippre_ippre_b8_reg.regValue = rtd_inl(IPPRE_IPPRE_B8_reg);
	ippre_ippre_bc_reg.regValue = rtd_inl(IPPRE_IPPRE_BC_reg);
	ippre_ippre_e0_reg.regValue = rtd_inl(IPPRE_IPPRE_E0_reg);
	ippre_ippre_e4_reg.regValue = rtd_inl(IPPRE_IPPRE_E4_reg);
	ippre1_ippre1_color_prot_2_reg.regValue = rtd_inl(IPPRE1_IPPRE1_COLOR_PROT_2_reg);
	ippre1_ippre1_54_reg.regValue = rtd_inl(IPPRE1_IPPRE1_54_reg);
	ippre1_ippre1_64_reg.regValue = rtd_inl(IPPRE1_IPPRE1_64_reg);
	ippre1_ippre1_b8_reg.regValue = rtd_inl(IPPRE1_IPPRE1_B8_reg);
	ippre1_ippre1_bc_reg.regValue = rtd_inl(IPPRE1_IPPRE1_BC_reg);
	ippre1_ippre1_c0_reg.regValue = rtd_inl(IPPRE1_IPPRE1_C0_reg);
	ippre1_ippre1_c4_reg.regValue = rtd_inl(IPPRE1_IPPRE1_C4_reg);

	// main function body
	//dummy line setting
	if (GS_ipme_dn_ratio == 8) {
		dummy_line = GS_ippre_dummy_line * 2;
	} 
	else {
		dummy_line = GS_ippre_dummy_line ;
	}
	//pc_mode
	pc_mode_turn_on  = (GS_pc_mode)? 0x1 : 0x0;
	pc_mode_turn_off = (GS_pc_mode)? 0x0 : 0x1;
	//indata_pc_mode
	indata_pc_mode_turn_on  = ((GS_pc_mode==1) & (GS_pc_repeat_mode==0))? 0x1 : 0x0;
	indata_pc_mode_turn_off = ((GS_pc_mode==1) & (GS_pc_repeat_mode==0))? 0x0 : 0x1;

	//======= register
	ippre_ippre_04_reg.ippre_444to422_en = indata_pc_mode_turn_off;
	ippre_ippre_08_reg.ippre_retiming_dummy_num = dummy_line;
	ippre_ippre_08_reg.ippre_retiming_hact = GS_image_hsize/2;
	ippre_ippre_08_reg.ippre_retiming_vact = GS_image_vsize;
	//0406
	ippre_ippre_10_reg.patt_hact = GS_image_hsize/2;
	ippre_ippre_10_reg.patt_vact = GS_image_vsize;
	//0406
#if defined(MERLIN6)
	ippre_ippre_54_reg.bar_hight = GS_image_vsize;
	ippre_ippre_54_reg.bar_width = GS_image_hsize/2;
	ippre_ippre_64_reg.circle_center_v = GS_image_vsize;
	ippre_ippre_64_reg.circle_center_h = GS_image_hsize/2;
	ippre_ippre_b8_reg.ippre_mute_vblank_max = 0x5a;
	ippre_ippre_b8_reg.ippre_mute_vact_max = GS_image_vsize+1;
	ippre_ippre_b8_reg.ippre_mute_vact_min = GS_image_vsize;		
	ippre_ippre_bc_reg.ippre_mute_htotal_max = GS_image_hsize/2;
	ippre_ippre_e0_reg.ippre_mute_vact_max_det_en = 1;
	ippre_ippre_e0_reg.ippre_mute_vact_min_det_en = 1;
	ippre_ippre_e0_reg.ippre_mute_vblank_det_en = 1;
	ippre_ippre_e0_reg.ippre_mute_htotal_max_det_en = 1;
	ippre_ippre_e0_reg.ippre_mute_hact_max_det_en = 1;
	ippre_ippre_e0_reg.ippre_mute_hact_min_det_en = 1;
	ippre_ippre_e0_reg.ippre_mute_clk_det_en = 1;
	ippre_ippre_e0_reg.ippre_mute_det_function_en = 1;
	ippre_ippre_e0_reg.ippre_mute_vact = GS_image_vsize;
	ippre_ippre_e0_reg.ippre_mute_hact = GS_image_hsize/2;
	ippre_ippre_e4_reg.ippre_mute_hact_max = GS_image_hsize/2;
	ippre_ippre_e4_reg.ippre_mute_hact_min = GS_image_hsize/2;
#endif

  

#if defined(MARK2) || defined(MERLIN7)
	if(GS_ipme_dn_ratio == 8){
       ippre1_ippre1_color_prot_2_reg.colorprotect_fetch_mode=3;
  	} else if(GS_ipme_dn_ratio == 4){  
       ippre1_ippre1_color_prot_2_reg.colorprotect_fetch_mode=2;   
    } else {   
       ippre1_ippre1_color_prot_2_reg.colorprotect_fetch_mode=1;  
    } 

	ippre_ippre_b8_reg.ippre_mute_vblank_max = 0x5a;		
	ippre_ippre_e0_reg.ippre_mute_vact_max_det_en = 1;
	ippre_ippre_e0_reg.ippre_mute_vact_min_det_en = 1;
	ippre_ippre_e0_reg.ippre_mute_vblank_det_en = 1;
	ippre_ippre_e0_reg.ippre_mute_htotal_max_det_en = 1;
	ippre_ippre_e0_reg.ippre_mute_hact_max_det_en = 1;
	ippre_ippre_e0_reg.ippre_mute_hact_min_det_en = 1;
	ippre_ippre_e0_reg.ippre_mute_clk_det_en = 1;
	ippre_ippre_e0_reg.ippre_mute_det_function_en = 1;

	ippre1_ippre1_54_reg.bar_hight = 200; // GS_image_vsize;
	ippre1_ippre1_54_reg.bar_width = 150; // GS_image_hsize/2;
	ippre1_ippre1_64_reg.circle_center_v = GS_image_vsize;
	ippre1_ippre1_64_reg.circle_center_h = GS_image_hsize/2;	
	ippre1_ippre1_b8_reg.ippre_mute_vact_max = GS_image_vsize+1;
	ippre1_ippre1_b8_reg.ippre_mute_vact_min = GS_image_vsize;	
	ippre1_ippre1_bc_reg.ippre_mute_htotal_max = GS_image_htotal/2+1;
	ippre1_ippre1_c0_reg.ippre_mute_vact = GS_image_vsize;
	ippre1_ippre1_c0_reg.ippre_mute_hact = GS_image_hsize/2;
	ippre1_ippre1_c4_reg.ippre_mute_hact_max = GS_image_hsize/2;
	ippre1_ippre1_c4_reg.ippre_mute_hact_min = GS_image_hsize/2;	
#endif

	// register write
	rtd_outl(IPPRE_IPPRE_04_reg, ippre_ippre_04_reg.regValue);
	rtd_outl(IPPRE_IPPRE_08_reg, ippre_ippre_08_reg.regValue);
	rtd_outl(IPPRE_IPPRE_10_reg, ippre_ippre_10_reg.regValue);
	rtd_outl(IPPRE_IPPRE_54_reg, ippre_ippre_54_reg.regValue);
	rtd_outl(IPPRE_IPPRE_64_reg, ippre_ippre_64_reg.regValue);
	rtd_outl(IPPRE_IPPRE_B8_reg, ippre_ippre_b8_reg.regValue);
	rtd_outl(IPPRE_IPPRE_BC_reg, ippre_ippre_bc_reg.regValue);
	rtd_outl(IPPRE_IPPRE_E0_reg, ippre_ippre_e0_reg.regValue);
	rtd_outl(IPPRE_IPPRE_E4_reg, ippre_ippre_e4_reg.regValue);
	rtd_outl(IPPRE1_IPPRE1_COLOR_PROT_2_reg, ippre1_ippre1_color_prot_2_reg.regValue);
	rtd_outl(IPPRE1_IPPRE1_54_reg, ippre1_ippre1_54_reg.regValue);
	rtd_outl(IPPRE1_IPPRE1_64_reg, ippre1_ippre1_64_reg.regValue);
	rtd_outl(IPPRE1_IPPRE1_B8_reg, ippre1_ippre1_b8_reg.regValue);
	rtd_outl(IPPRE1_IPPRE1_BC_reg, ippre1_ippre1_bc_reg.regValue);
	rtd_outl(IPPRE1_IPPRE1_C0_reg, ippre1_ippre1_c0_reg.regValue);
	rtd_outl(IPPRE1_IPPRE1_C4_reg, ippre1_ippre1_c4_reg.regValue);

}
void MEMC_ChangeSize_KMC_TOP(void)
{
	// parameter declaration

	// register declaration
	kmc_top_kmc_top_04_RBUS kmc_top_kmc_top_04_reg;
	kpost_top_kpost_top_14_RBUS kpost_top_kpost_top_14_reg;
	kmc_top_kmc_top_3c_RBUS kmc_top_kmc_top_3c_reg;
	kmc_top_kmc_top_40_RBUS kmc_top_kmc_top_40_reg;

	// register read back
	kmc_top_kmc_top_04_reg.regValue = rtd_inl(KMC_TOP_kmc_top_04_reg);
	kpost_top_kpost_top_14_reg.regValue = rtd_inl(KPOST_TOP_KPOST_TOP_14_reg);
	kmc_top_kmc_top_3c_reg.regValue = rtd_inl(KMC_TOP_kmc_top_3C_reg);
	kmc_top_kmc_top_40_reg.regValue = rtd_inl(KMC_TOP_kmc_top_40_reg);

	// main function body
	//======= register
	//0406
	kmc_top_kmc_top_04_reg.top_patt_vact = GS_image_vsize/2;
	kmc_top_kmc_top_04_reg.top_patt_hact = GS_image_hsize/2;
	kpost_top_kpost_top_14_reg.post_patt_vact = GS_image_vsize;
	kpost_top_kpost_top_14_reg.post_patt_hact = GS_image_hsize/2;
	//0406
	kmc_top_kmc_top_3c_reg.kmc_ipmc_hde_center_value = GS_image_hsize/4;

	kmc_top_kmc_top_40_reg.kmc_ipmc_v_active   = GS_image_vsize;
	kmc_top_kmc_top_40_reg.kmc_ipmc_hde_active = GS_image_hsize/2;

#if defined(MARK2)

	int lbmc_mode;
	
	if(GS_ipme_dn_ratio ==8) {
		lbmc_mode =0;
	}
	else if(GS_ipme_dn_ratio ==4) {
		if(GS_me2_2x2 ==1) {
			lbmc_mode =1;
		}
		else {
			lbmc_mode=0;
		}
	}
	else {
		lbmc_mode =1;
	}

	if(lbmc_mode ==0) {
		kmc_top_kmc_top_40_reg.kmc_ipmc_422_en = 1;
		
	}
		
#endif


	// register write
	rtd_outl(KMC_TOP_kmc_top_04_reg, kmc_top_kmc_top_04_reg.regValue);
	rtd_outl(KPOST_TOP_KPOST_TOP_14_reg, kpost_top_kpost_top_14_reg.regValue);
	rtd_outl(KMC_TOP_kmc_top_3C_reg, kmc_top_kmc_top_3c_reg.regValue);
	rtd_outl(KMC_TOP_kmc_top_40_reg, kmc_top_kmc_top_40_reg.regValue);

}
void MEMC_ChangeSize_KME_TOP(void)
{
	// parameter declaration
	int image_me_hsize;
	unsigned int image_me_vsize;
	int ipme_pre_down2;
	int mc_scale;
	int me2_work_mode;
	int me2_one_row_eq_mc_row_num;
	int ipme_dn;
	int vbuf_ip_1st_cand6_en;
	int vbuf_ip_1st_cand7_en;
	int image_me1_blk_hsize;
	int image_me1_blk_vsize;
	int image_me2_blk_pfv_hsize;
	unsigned int image_me2_blk_pfv_vsize;
	int reg_mv_hnum;
	int reg_me_hnum;
	int lbme_rim_x_end;
	int lbme_rim_y_end;
	int me1_blk_rim_x_end;
	int me1_blk_rim_y_end;
	int vbuf_meander_blk_rim_bot;
	int vbuf_meander_pix_rim_bot;
	int lbme_meander_rim_y_end;
	int lbme_meander_rim_x_end;
	int reg_kphase_me_work_mode;
	int reg_kme_me_work_mode;
	int vbuf_wait_cycle_frame2_mv_trig;
	int vbuf_wait_cycle_frame2_me_trig;

	int frame_meander_enable = 0;
	int me1_vbuf_row_type;
	int image_lbme_addr_perline;

	int me2_1st_ph_cand_num;
	int me2_2nd_ph_cand_num;

	int me2_lbme_dm_num_line;
	int me2_vbuf_pfv_mv_fetch_ln_1_row;

	int kme_me2_vbuf_ph_pfv_h_ratio;
	int kme_me2_vbuf_ph_pfv_v_ratio;
	int kme_me2_vbuf_ph_lbme_v_ratio;
	int kme_me2_vbuf_ph_lbme_h_ratio;

	int me2_wait_row_nums;

	int reg_dh_rt1_xidx_shift =2;
	int reg_dh_rt1_yidx_shift =2;
	int reg_dh_rt2_hidx_shift =2;
	int reg_dh_rt2_vidx_shift =2;

	int image_me2_blk_ph_hsize;
	int image_me2_blk_ph_vsize;

	int dh_blk_ph_hsize;
	int dh_blk_ph_vsize;
	int me2_s1_blk_ph_hsize;
	int me2_s1_blk_ph_vsize;

	int dh_rim_blk, dh_rim_row, dh_vbuf_row_mask, dh_me2cal_in_480x270_mod;

	int me2crtc_vact_x2 = GS_me2_2x2;
	int me1crtc_vact = GS_image_vsize/GS_ipme_dn_ratio/4;

	int me2crtc_preflt_dly = 0x2;
	int me2crtc_pstflt_dly = 0x4;
	int me2crtc_extcor_dly = 0x6;
	int me2crtc_dobflt_dly = 0x8;

	int me2_vbuf_row_finish_dly = 0x1a4;

	// register declaration
	kme_top_kme_top_04_RBUS kme_top_kme_top_04_reg;
	kme_top_kme_top_08_RBUS kme_top_kme_top_08_reg;
	kme_top_kme_top_30_RBUS kme_top_kme_top_30_reg;
	kme_top_kme_top_3c_RBUS kme_top_kme_top_3c_reg;
	kme_top_kme_top_crtc_f8_RBUS kme_top_kme_top_crtc_f8_reg;
	kme_top_kme_top_crtc_fc_RBUS kme_top_kme_top_crtc_fc_reg;
	kme_top_kme_top_crtc_e0_RBUS kme_top_kme_top_crtc_e0_reg;
	kme_top_kme_top_crtc_f4_RBUS kme_top_kme_top_crtc_f4_reg;
	kme_vbuf_top_kme_vbuf_top_00_RBUS kme_vbuf_top_kme_vbuf_top_00_reg;
	kme_vbuf_top_kme_vbuf_top_10_RBUS kme_vbuf_top_kme_vbuf_top_10_reg;
	kme_vbuf_top_kme_vbuf_top_30_RBUS kme_vbuf_top_kme_vbuf_top_30_reg;
	kme_vbuf_top_kme_vbuf_top_34_RBUS kme_vbuf_top_kme_vbuf_top_34_reg;
	kme_vbuf_top_kme_vbuf_top_38_RBUS kme_vbuf_top_kme_vbuf_top_38_reg;
	kme_vbuf_top_kme_vbuf_top_40_RBUS kme_vbuf_top_kme_vbuf_top_40_reg;
	kme_vbuf_top_kme_vbuf_top_44_RBUS kme_vbuf_top_kme_vbuf_top_44_reg;
	kme_vbuf_top_kme_vbuf_top_48_RBUS kme_vbuf_top_kme_vbuf_top_48_reg;
	kme_vbuf_top_kme_vbuf_top_84_RBUS kme_vbuf_top_kme_vbuf_top_84_reg;
	kme_me1_top2_kme_me1_top2_04_RBUS kme_me1_top2_kme_me1_top2_04_reg;
	kme_lbme_top_kme_lbme_top_00_RBUS kme_lbme_top_kme_lbme_top_00_reg;
	kme_lbme_top_kme_lbme_top_04_RBUS kme_lbme_top_kme_lbme_top_04_reg;
	kme_lbme_top_kme_lbme_top_14_RBUS kme_lbme_top_kme_lbme_top_14_reg;
	kme_lbme_top_kme_lbme_top_18_RBUS kme_lbme_top_kme_lbme_top_18_reg;
	kme_lbme_top_kme_lbme_top_1c_RBUS kme_lbme_top_kme_lbme_top_1c_reg;
	kme_lbme_top_kme_lbme_top_20_RBUS kme_lbme_top_kme_lbme_top_20_reg;
	kme_ipme_kme_ipme_00_RBUS kme_ipme_kme_ipme_00_reg;
	kme_ipme_kme_ipme_0c_RBUS kme_ipme_kme_ipme_0c_reg;
	kme_ipme_kme_ipme_24_RBUS kme_ipme_kme_ipme_24_reg;
	kme_me1_top1_me1_scene1_00_RBUS kme_me1_top1_me1_scene1_00_reg;
	kme_lbme2_top_kme_lbme2_top_00_RBUS kme_lbme2_top_kme_lbme2_top_00_reg;
	kme_lbme2_top_kme_lbme2_top_04_RBUS kme_lbme2_top_kme_lbme2_top_04_reg;
	kme_lbme2_top_kme_lbme2_top_14_RBUS kme_lbme2_top_kme_lbme2_top_14_reg;
	kme_lbme2_top_kme_lbme2_top_18_RBUS kme_lbme2_top_kme_lbme2_top_18_reg;
	kme_me2_vbuf_top_kme_me2_vbuf_top_04_RBUS kme_me2_vbuf_top_kme_me2_vbuf_top_04_reg;
	kme_me2_vbuf_top_kme_me2_vbuf_top_0c_RBUS kme_me2_vbuf_top_kme_me2_vbuf_top_0c_reg;
	kme_me2_vbuf_top_kme_me2_vbuf_top_08_RBUS kme_me2_vbuf_top_kme_me2_vbuf_top_08_reg;
	kme_me2_vbuf_top_kme_me2_vbuf_top_1c_RBUS kme_me2_vbuf_top_kme_me2_vbuf_top_1c_reg;
	kme_me2_vbuf_top_kme_me2_vbuf_top_20_RBUS kme_me2_vbuf_top_kme_me2_vbuf_top_20_reg;
	kme_me2_vbuf_top_kme_me2_vbuf_top_24_RBUS kme_me2_vbuf_top_kme_me2_vbuf_top_24_reg;
	kme_me2_vbuf_top_kme_me2_vbuf_top_28_RBUS kme_me2_vbuf_top_kme_me2_vbuf_top_28_reg;
	kme_me2_vbuf_top_kme_me2_vbuf_top_38_RBUS kme_me2_vbuf_top_kme_me2_vbuf_top_38_reg;
	kme_me2_vbuf_top_kme_me2_vbuf_top_d4_RBUS kme_me2_vbuf_top_kme_me2_vbuf_top_d4_reg;
	kme_me2_vbuf_top_kme_me2_vbuf_top_d8_RBUS kme_me2_vbuf_top_kme_me2_vbuf_top_d8_reg;
	kme_me2_vbuf_top_kme_me2_vbuf_top_e0_RBUS kme_me2_vbuf_top_kme_me2_vbuf_top_e0_reg;
	kme_me2_vbuf_top_kme_me2_vbuf_top_2c_RBUS kme_me2_vbuf_top_kme_me2_vbuf_top_2c_reg;
	kme_me2_vbuf_top_kme_me2_vbuf_top_c8_RBUS kme_me2_vbuf_top_kme_me2_vbuf_top_c8_reg;
	kme_me2_vbuf_top_kme_me2_vbuf_top_30_RBUS kme_me2_vbuf_top_kme_me2_vbuf_top_30_reg;
	kme_me2_calc0_kme_me2_calc0_74_RBUS kme_me2_calc0_kme_me2_calc0_74_reg;
	kme_me1_top6_kme_me1_top6_00_RBUS kme_me1_top6_kme_me1_top6_00_reg;
	kme_dehalo_kme_dehalo_10_RBUS kme_dehalo_kme_dehalo_10_reg;
	kme_dehalo_kme_dehalo_14_RBUS kme_dehalo_kme_dehalo_14_reg;
	kme_dehalo_kme_dehalo_18_RBUS kme_dehalo_kme_dehalo_18_reg;
	kme_dehalo_kme_dehalo_1c_RBUS kme_dehalo_kme_dehalo_1c_reg;
	kme_dehalo_kme_dehalo_4c_RBUS kme_dehalo_kme_dehalo_4c_reg;
	kme_dehalo_kme_dehalo_50_RBUS kme_dehalo_kme_dehalo_50_reg;
	kme_dehalo_kme_dehalo_78_RBUS kme_dehalo_kme_dehalo_78_reg;
	kme_dehalo_kme_dehalo_34_RBUS kme_dehalo_kme_dehalo_34_reg;
	kpost_top_kpost_top_28_RBUS kpost_top_kpost_top_28_reg;

	// register read back
	kme_top_kme_top_04_reg.regValue = rtd_inl(KME_TOP_KME_TOP_04_reg);
	kme_top_kme_top_08_reg.regValue = rtd_inl(KME_TOP_KME_TOP_08_reg);
	kme_top_kme_top_30_reg.regValue = rtd_inl(KME_TOP_KME_TOP_30_reg);
	kme_top_kme_top_3c_reg.regValue = rtd_inl(KME_TOP_KME_TOP_3C_reg);
	kme_top_kme_top_crtc_f8_reg.regValue = rtd_inl(KME_TOP_KME_TOP_CRTC_F8_reg);
	kme_top_kme_top_crtc_fc_reg.regValue = rtd_inl(KME_TOP_KME_TOP_CRTC_FC_reg);
	kme_top_kme_top_crtc_e0_reg.regValue = rtd_inl(KME_TOP_KME_TOP_CRTC_E0_reg);
	kme_top_kme_top_crtc_f4_reg.regValue = rtd_inl(KME_TOP_KME_TOP_CRTC_F4_reg);
	kme_vbuf_top_kme_vbuf_top_00_reg.regValue = rtd_inl(KME_VBUF_TOP_KME_VBUF_TOP_00_reg);
	kme_vbuf_top_kme_vbuf_top_10_reg.regValue = rtd_inl(KME_VBUF_TOP_KME_VBUF_TOP_10_reg);
	kme_vbuf_top_kme_vbuf_top_30_reg.regValue = rtd_inl(KME_VBUF_TOP_KME_VBUF_TOP_30_reg);
	kme_vbuf_top_kme_vbuf_top_34_reg.regValue = rtd_inl(KME_VBUF_TOP_KME_VBUF_TOP_34_reg);
	kme_vbuf_top_kme_vbuf_top_38_reg.regValue = rtd_inl(KME_VBUF_TOP_KME_VBUF_TOP_38_reg);
	kme_vbuf_top_kme_vbuf_top_40_reg.regValue = rtd_inl(KME_VBUF_TOP_KME_VBUF_TOP_40_reg);
	kme_vbuf_top_kme_vbuf_top_44_reg.regValue = rtd_inl(KME_VBUF_TOP_KME_VBUF_TOP_44_reg);
	kme_vbuf_top_kme_vbuf_top_48_reg.regValue = rtd_inl(KME_VBUF_TOP_KME_VBUF_TOP_48_reg);
	kme_vbuf_top_kme_vbuf_top_84_reg.regValue = rtd_inl(KME_VBUF_TOP_KME_VBUF_TOP_84_reg);
	kme_me1_top2_kme_me1_top2_04_reg.regValue = rtd_inl(KME_ME1_TOP2_KME_ME1_TOP2_04_reg);
	kme_lbme_top_kme_lbme_top_00_reg.regValue = rtd_inl(KME_LBME_TOP_KME_LBME_TOP_00_reg);
	kme_lbme_top_kme_lbme_top_04_reg.regValue = rtd_inl(KME_LBME_TOP_KME_LBME_TOP_04_reg);
	kme_lbme_top_kme_lbme_top_14_reg.regValue = rtd_inl(KME_LBME_TOP_KME_LBME_TOP_14_reg);
	kme_lbme_top_kme_lbme_top_18_reg.regValue = rtd_inl(KME_LBME_TOP_KME_LBME_TOP_18_reg);
	kme_lbme_top_kme_lbme_top_1c_reg.regValue = rtd_inl(KME_LBME_TOP_KME_LBME_TOP_1C_reg);
	kme_lbme_top_kme_lbme_top_20_reg.regValue = rtd_inl(KME_LBME_TOP_KME_LBME_TOP_20_reg);
	kme_ipme_kme_ipme_00_reg.regValue = rtd_inl(KME_IPME_KME_IPME_00_reg);
	kme_ipme_kme_ipme_0c_reg.regValue = rtd_inl(KME_IPME_KME_IPME_0C_reg);
	kme_ipme_kme_ipme_24_reg.regValue = rtd_inl(KME_IPME_KME_IPME_24_reg);
	kme_me1_top1_me1_scene1_00_reg.regValue = rtd_inl(KME_ME1_TOP1_ME1_SCENE1_00_reg);
	kme_lbme2_top_kme_lbme2_top_00_reg.regValue = rtd_inl(KME_LBME2_TOP_KME_LBME2_TOP_00_reg);
	kme_lbme2_top_kme_lbme2_top_04_reg.regValue = rtd_inl(KME_LBME2_TOP_KME_LBME2_TOP_04_reg);
	kme_lbme2_top_kme_lbme2_top_14_reg.regValue = rtd_inl(KME_LBME2_TOP_KME_LBME2_TOP_14_reg);
	kme_lbme2_top_kme_lbme2_top_18_reg.regValue = rtd_inl(KME_LBME2_TOP_KME_LBME2_TOP_18_reg);
	kme_me2_vbuf_top_kme_me2_vbuf_top_04_reg.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_04_reg);
	kme_me2_vbuf_top_kme_me2_vbuf_top_0c_reg.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_0C_reg);
	kme_me2_vbuf_top_kme_me2_vbuf_top_08_reg.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_08_reg);
	kme_me2_vbuf_top_kme_me2_vbuf_top_1c_reg.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_1C_reg);
	kme_me2_vbuf_top_kme_me2_vbuf_top_20_reg.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_20_reg);
	kme_me2_vbuf_top_kme_me2_vbuf_top_24_reg.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_24_reg);
	kme_me2_vbuf_top_kme_me2_vbuf_top_28_reg.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_28_reg);
	kme_me2_vbuf_top_kme_me2_vbuf_top_38_reg.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_38_reg);
	kme_me2_vbuf_top_kme_me2_vbuf_top_d4_reg.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D4_reg);
	kme_me2_vbuf_top_kme_me2_vbuf_top_d8_reg.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg);
	kme_me2_vbuf_top_kme_me2_vbuf_top_e0_reg.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_E0_reg);
	kme_me2_vbuf_top_kme_me2_vbuf_top_2c_reg.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_2C_reg);
	kme_me2_vbuf_top_kme_me2_vbuf_top_c8_reg.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_C8_reg);
	kme_me2_vbuf_top_kme_me2_vbuf_top_30_reg.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg);
	kme_me2_calc0_kme_me2_calc0_74_reg.regValue = rtd_inl(KME_ME2_CALC0_KME_ME2_CALC0_74_reg);
	kme_me1_top6_kme_me1_top6_00_reg.regValue = rtd_inl(KME_ME1_TOP6_KME_ME1_TOP6_00_reg);
	kme_dehalo_kme_dehalo_10_reg.regValue = rtd_inl(KME_DEHALO_KME_DEHALO_10_reg);
	kme_dehalo_kme_dehalo_14_reg.regValue = rtd_inl(KME_DEHALO_KME_DEHALO_14_reg);
	kme_dehalo_kme_dehalo_18_reg.regValue = rtd_inl(KME_DEHALO_KME_DEHALO_18_reg);
	kme_dehalo_kme_dehalo_1c_reg.regValue = rtd_inl(KME_DEHALO_KME_DEHALO_1C_reg);
	kme_dehalo_kme_dehalo_4c_reg.regValue = rtd_inl(KME_DEHALO_KME_DEHALO_4C_reg);
	kme_dehalo_kme_dehalo_50_reg.regValue = rtd_inl(KME_DEHALO_KME_DEHALO_50_reg);
	kme_dehalo_kme_dehalo_78_reg.regValue = rtd_inl(KME_DEHALO_KME_DEHALO_78_reg);
	kme_dehalo_kme_dehalo_34_reg.regValue = rtd_inl(KME_DEHALO_KME_DEHALO_34_reg);
	kpost_top_kpost_top_28_reg.regValue = rtd_inl(KPOST_TOP_KPOST_TOP_28_reg);

	// main function body
	if(GS_meander){
		frame_meander_enable = 1;
	}

	if (GS_me2_2x2) {
		reg_dh_rt1_xidx_shift =1;
		reg_dh_rt1_yidx_shift =1;
		reg_dh_rt2_hidx_shift =0;
		reg_dh_rt2_vidx_shift =0;
	} else {
		reg_dh_rt1_xidx_shift =2;
		reg_dh_rt1_yidx_shift =2;
		reg_dh_rt2_hidx_shift =2;
		reg_dh_rt2_vidx_shift =2;
	}

	if(GS_ipme_dn_ratio == 8){
		image_me_hsize = (GS_image_hsize / 8 );
		image_me_vsize = (GS_image_vsize / 8 );
		ipme_pre_down2 = 1;
		if(GS_me2_2x2) {
			mc_scale = 0xfa;
			me2_work_mode = 0;                  // #  16 mc row = 1 me2 row
			me2_one_row_eq_mc_row_num = 16; //0x10
		} else {
			mc_scale = 0xfa;
			me2_work_mode = 0;
			me2_one_row_eq_mc_row_num = 32; //0x20

		}
		ipme_dn = 2;
		if(GS_me1_double_scan){
			me1_vbuf_row_type = 3;
			vbuf_ip_1st_cand6_en = 1;
			vbuf_ip_1st_cand7_en = 1;
		} else {
			me1_vbuf_row_type = 4;
			vbuf_ip_1st_cand6_en = 1;
			vbuf_ip_1st_cand7_en = 1;
		}
	}
	else if(GS_ipme_dn_ratio == 4){
		image_me_hsize = (GS_image_hsize / 4 );
		image_me_vsize = (GS_image_vsize / 4 );
		ipme_pre_down2 = 0;
		if(GS_me2_2x2) {
			mc_scale = 0xa5;
			me2_work_mode = 1;         //#  8 mc row = 1 me2 row
			me2_one_row_eq_mc_row_num = 8;
		} else {
			mc_scale = 0xaa;
			me2_work_mode = 0;         //#  16 mc row = 1 me2 row
			me2_one_row_eq_mc_row_num = 16; //
		}
		ipme_dn = 2;
		if(GS_me1_double_scan){
			me1_vbuf_row_type = 3;
			vbuf_ip_1st_cand6_en = 1;
			vbuf_ip_1st_cand7_en = 1;
		} else {
			me1_vbuf_row_type = 4;
			vbuf_ip_1st_cand6_en = 1;
			vbuf_ip_1st_cand7_en = 1;
		}
	} else {
		image_me_hsize = (GS_image_hsize % 2048)/ 2;
		image_me_vsize = (GS_image_vsize % 2048)/ 2;
		ipme_pre_down2 = 0;
		mc_scale = 85;//0x55
		ipme_dn = 1;
		if(GS_me1_double_scan){
			me1_vbuf_row_type = 2;
			vbuf_ip_1st_cand6_en = 0;
			vbuf_ip_1st_cand7_en = 0;
		} else {
			me1_vbuf_row_type = 3;
			vbuf_ip_1st_cand6_en = 1;
			vbuf_ip_1st_cand7_en = 1;
		}
		if(GS_me2_2x2) {
			me2_work_mode = 2;              //     #  4 mc row = 1 me2 row
			me2_one_row_eq_mc_row_num = 4;
		} else {
			me2_work_mode = 1;              //     #  8 mc row = 1 me2 row
			me2_one_row_eq_mc_row_num = 8;
		}
	}

	if(GS_me12_merge)
	{
		me1_vbuf_row_type = 0;
	}

	image_me_hsize = image_me_hsize % 1023;
	image_me_vsize = image_me_vsize % 1023;
	image_me1_blk_hsize = (image_me_hsize / 4) %512;
	image_me1_blk_vsize = (image_me_vsize / 4) %512;
	image_me2_blk_pfv_hsize = image_me1_blk_hsize;
	image_me2_blk_pfv_vsize = image_me1_blk_vsize;
	reg_mv_hnum = (image_me_hsize / 4) % 256;
	reg_me_hnum = (image_me_hsize / 16) % 256;


	lbme_rim_x_end = (image_me_hsize - 1);
	lbme_rim_y_end = (image_me_vsize - 1);
	me1_blk_rim_x_end = image_me1_blk_hsize - 1;
	me1_blk_rim_y_end = image_me1_blk_vsize - 1;

	vbuf_meander_blk_rim_bot = me1_blk_rim_y_end;
	vbuf_meander_pix_rim_bot = lbme_rim_y_end;
	lbme_meander_rim_y_end   = lbme_rim_y_end;
	lbme_meander_rim_x_end   = lbme_rim_x_end;

	if(GS_me1_double_scan){
		reg_kphase_me_work_mode = 0;
		reg_kme_me_work_mode = 0;
		vbuf_wait_cycle_frame2_mv_trig = 1280;
		
#if defined(MARK2)
		if (GS_kme_me1_bg_mask_en)
			vbuf_wait_cycle_frame2_mv_trig = 0x1a800;	// temp for me1_bg DT case 
#endif

#if defined(MERLIN7)
		if (GS_kme_me1_bg_mask_en)
			vbuf_wait_cycle_frame2_mv_trig = 0x20000;	// temp for me1_bg DT case 
#endif
		
		vbuf_wait_cycle_frame2_me_trig = 8192;
	}
	else {
		reg_kphase_me_work_mode = 1;
		reg_kme_me_work_mode = 1;
		vbuf_wait_cycle_frame2_mv_trig = 0;
		vbuf_wait_cycle_frame2_me_trig = 0;
	}

    if (GS_me2_refine_scan){
			me2_1st_ph_cand_num=5;
			me2_2nd_ph_cand_num=3;
	} else if(GS_me2_2x2) {
		if(GS_me2_double_scan)  {
			me2_1st_ph_cand_num=3;
			me2_2nd_ph_cand_num=3;
		}
		else
		{
			me2_1st_ph_cand_num=5;
			me2_2nd_ph_cand_num=4;
		}

	} else {
		me2_1st_ph_cand_num=5;
		me2_2nd_ph_cand_num=4;

	}

	image_lbme_addr_perline = (image_me_hsize / 16) % 64;
	
	if (GS_me2_refine_scan) {
		me2_lbme_dm_num_line = (image_me_hsize *4/16) % 512; 
		me2_vbuf_pfv_mv_fetch_ln_1_row = 1;
		// ph_pfv_ratio = 0x0;
		// lbme_ratio   = 0xa;
		kme_me2_vbuf_ph_pfv_h_ratio  = 0;
		kme_me2_vbuf_ph_pfv_v_ratio  = 0;
		kme_me2_vbuf_ph_lbme_v_ratio = 2;
		kme_me2_vbuf_ph_lbme_h_ratio = 2;
	} else if(GS_me2_2x2) {
		me2_lbme_dm_num_line = (image_me_hsize *2/16) % 512;
		me2_vbuf_pfv_mv_fetch_ln_1_row = 0;
		// ph_pfv_ratio = 0xa;
		// lbme_ratio   = 0x5;
		kme_me2_vbuf_ph_pfv_h_ratio  = 2;
		kme_me2_vbuf_ph_pfv_v_ratio  = 2;
		kme_me2_vbuf_ph_lbme_v_ratio = 1; 
		kme_me2_vbuf_ph_lbme_h_ratio = 1;
	} else {
		me2_lbme_dm_num_line = (image_me_hsize *4/16) % 512;
		me2_vbuf_pfv_mv_fetch_ln_1_row = 1;
		// ph_pfv_ratio = 0x0;
		// lbme_ratio   = 0xa;
		kme_me2_vbuf_ph_pfv_h_ratio  = 0;
		kme_me2_vbuf_ph_pfv_v_ratio  = 0;
		kme_me2_vbuf_ph_lbme_v_ratio = 2;
		kme_me2_vbuf_ph_lbme_h_ratio = 2;
	}
	
	if (GS_me2_refine_scan) {
		me2_wait_row_nums = 3;
	} else {
		me2_wait_row_nums = 2;  // reg default value
	}

	if(GS_me2_2x2) {
		image_me2_blk_ph_hsize = ((image_me1_blk_hsize % 256) *2 );
		image_me2_blk_ph_vsize = ((image_me1_blk_vsize % 256) *2 );
	} else {
		image_me2_blk_ph_hsize = image_me1_blk_hsize;
		image_me2_blk_ph_vsize = image_me1_blk_vsize;
	}

	dh_blk_ph_hsize = image_me2_blk_ph_hsize;
	dh_blk_ph_vsize = image_me2_blk_ph_vsize;
	me2_s1_blk_ph_hsize = GS_me2_refine_scan ? image_me1_blk_hsize : image_me2_blk_ph_hsize;
	me2_s1_blk_ph_vsize = GS_me2_refine_scan ? image_me1_blk_vsize : image_me2_blk_ph_vsize;

	if(GS_me2_2x2) {
		dh_rim_blk = 0;// rtl fixed in Merlin5 ###(dh_blk_ph_hsize >> 1);
		dh_rim_row = 0;// rtl fixed in Merlin5 ###(dh_blk_ph_vsize >> 1);
		dh_vbuf_row_mask =1;
		dh_me2cal_in_480x270_mod=1;
	} else {
		dh_rim_blk = 0;
		dh_rim_row = 0;
		dh_vbuf_row_mask =0;
		dh_me2cal_in_480x270_mod=0;
	}

	if(GS_me2_double_scan){
		me2crtc_preflt_dly = 0x4;
		me2crtc_pstflt_dly = 0x6;
		me2crtc_extcor_dly = 0x8;
		me2crtc_dobflt_dly = 0xa;
	}
	if(GS_me2_refine_scan){
		me2crtc_preflt_dly = 0x5;
		me2crtc_pstflt_dly = 0x7;
		me2crtc_extcor_dly = 0x9;
		me2crtc_dobflt_dly = 0xb;
	}

	if(GS_image_vsize < 300){
		if(GS_me2_2x2==0){
			if(GS_image_hsize<=960){
				me2_vbuf_row_finish_dly = 0x1000;
			}
			else{
				me2_vbuf_row_finish_dly = 0x34bc;
			}
		}
		else{
			if(GS_image_hsize<=960){
				me2_vbuf_row_finish_dly = 0x0c00;
			}
			else if(GS_image_hsize>1920){
				me2_vbuf_row_finish_dly = 0x1a5e;
				if(GS_me2_double_scan){
					me2_vbuf_row_finish_dly = me2_vbuf_row_finish_dly/2;

#if defined(MERLIN7)
					if (GS_kme_me1_bg_mask_en)
					me2_vbuf_row_finish_dly = me2_vbuf_row_finish_dly/2;
#endif
				}
			}
			else{
				me2_vbuf_row_finish_dly = 0x34bc;
			}
		}
	}


	//==register
	#if defined(MERLIN6)
		kme_top_kme_top_04_reg.frame_meander_enable = frame_meander_enable;

		kme_top_kme_top_08_reg.me12_buff_merge = GS_me12_merge;
		kme_top_kme_top_08_reg.kme_vbuf_blk_vact = image_me1_blk_vsize;
		kme_top_kme_top_08_reg.kme_vbuf_row_type = me1_vbuf_row_type;
		kme_top_kme_top_08_reg.kphase_me_work_mode = reg_kphase_me_work_mode;

		kme_top_kme_top_30_reg.vbuf_meander_pix_rim_bot = vbuf_meander_pix_rim_bot;
		kme_top_kme_top_3c_reg.kme_dummy3 = 0x00000100; //gmv_cost_rtl_bug

		kme_top_kme_top_crtc_f8_reg.me2crtc_vact_x2         = me2crtc_vact_x2;
		kme_top_kme_top_crtc_f8_reg.me1crtc_vact            = me1crtc_vact;
		kme_top_kme_top_crtc_f8_reg.me2_vbuf_row_finish_dly = me2_vbuf_row_finish_dly;

		kme_top_kme_top_crtc_fc_reg.me2crtc_me2dh_preflt_vtrig_dly = me2crtc_preflt_dly;
		kme_top_kme_top_crtc_fc_reg.me2crtc_me2dh_pstflt_vtrig_dly = me2crtc_pstflt_dly;
		kme_top_kme_top_crtc_fc_reg.me2crtc_me2dh_extcor_vtrig_dly = me2crtc_extcor_dly;
		kme_top_kme_top_crtc_fc_reg.me2crtc_me2dh_dobflt_vtrig_dly = me2crtc_dobflt_dly;

	//0408
		kme_top_kme_top_crtc_e0_reg.me1crtc_ref_row_htotal_hold_en = 0x1;
		kme_top_kme_top_crtc_e0_reg.me1crtc_ref_row_htotal_hold_offset = 0x1;
		kme_top_kme_top_crtc_e0_reg.me1crtc_ref_row_en = 0x1;
		kme_top_kme_top_crtc_e0_reg.me1crtc_htotal = 0x7fff;
		kme_top_kme_top_crtc_f4_reg.me2crtc_ref_row_htotal_hold_en = 0x1;
		kme_top_kme_top_crtc_f4_reg.me2crtc_ref_row_htotal_hold_offset = 0x1;
		kme_top_kme_top_crtc_f4_reg.me2crtc_ref_row_en = 0x1;
		kme_top_kme_top_crtc_f4_reg.me2crtc_htotal =0x7fff;
	//0408
	#else
		kme_top_kme_top_04_reg.reg_frame_meander_enable = frame_meander_enable;

	
		kme_top_kme_top_08_reg.reg_kme_vbuf_blk_vact = image_me1_blk_vsize;
		kme_top_kme_top_08_reg.reg_kme_vbuf_row_type = me1_vbuf_row_type;
		kme_top_kme_top_08_reg.reg_kphase_me_work_mode = reg_kphase_me_work_mode;

		kme_top_kme_top_30_reg.reg_vbuf_meander_pix_rim_bot = vbuf_meander_pix_rim_bot;
		kme_top_kme_top_3c_reg.reg_kme_dummy3 = 0x00000100; //gmv_cost_rtl_bug

		kme_top_kme_top_crtc_f8_reg.reg_me2crtc_vact_x2         = me2crtc_vact_x2;
		kme_top_kme_top_crtc_f8_reg.reg_me1crtc_vact            = me1crtc_vact;
		kme_top_kme_top_crtc_f8_reg.reg_me2_vbuf_row_finish_dly = me2_vbuf_row_finish_dly;

		kme_top_kme_top_crtc_fc_reg.reg_me2crtc_me2dh_preflt_vtrig_dly = me2crtc_preflt_dly;
		kme_top_kme_top_crtc_fc_reg.reg_me2crtc_me2dh_pstflt_vtrig_dly = me2crtc_pstflt_dly;
		kme_top_kme_top_crtc_fc_reg.reg_me2crtc_me2dh_extcor_vtrig_dly = me2crtc_extcor_dly;
		kme_top_kme_top_crtc_fc_reg.reg_me2crtc_me2dh_dobflt_vtrig_dly = me2crtc_dobflt_dly;

	//0408
		kme_top_kme_top_crtc_e0_reg.reg_me1crtc_ref_row_htotal_hold_en = 0x1;
		kme_top_kme_top_crtc_e0_reg.reg_me1crtc_ref_row_htotal_hold_offset = 0x1;
		kme_top_kme_top_crtc_e0_reg.reg_me1crtc_ref_row_en = 0x1;
		kme_top_kme_top_crtc_e0_reg.reg_me1crtc_htotal = 0x7fff;
		kme_top_kme_top_crtc_f4_reg.reg_me2crtc_ref_row_htotal_hold_en = 0x1;
		kme_top_kme_top_crtc_f4_reg.reg_me2crtc_ref_row_htotal_hold_offset = 0x1;
		kme_top_kme_top_crtc_f4_reg.reg_me2crtc_ref_row_en = 0x1;
		kme_top_kme_top_crtc_f4_reg.reg_me2crtc_htotal =0x7fff;		
	#endif

	kme_vbuf_top_kme_vbuf_top_00_reg.vbuf_vactive = image_me1_blk_vsize;
	kme_vbuf_top_kme_vbuf_top_00_reg.vbuf_hactive = image_me1_blk_hsize;

	kme_vbuf_top_kme_vbuf_top_10_reg.vbuf_ip_1st_cand6_en = vbuf_ip_1st_cand6_en;
	kme_vbuf_top_kme_vbuf_top_10_reg.vbuf_ip_1st_cand7_en = vbuf_ip_1st_cand7_en;
	kme_vbuf_top_kme_vbuf_top_30_reg.kme_me_work_mode = reg_kme_me_work_mode;
	kme_vbuf_top_kme_vbuf_top_30_reg.vbuf_lbme_trig_mid_row_num = image_me1_blk_vsize-3;
	kme_vbuf_top_kme_vbuf_top_34_reg.vbuf_wait_cycle_frame2_mv_trig = vbuf_wait_cycle_frame2_mv_trig;
	kme_vbuf_top_kme_vbuf_top_38_reg.vbuf_wait_cycle_frame2_me_trig = vbuf_wait_cycle_frame2_me_trig;
	kme_vbuf_top_kme_vbuf_top_40_reg.vbuf_blk_rim_bot = me1_blk_rim_y_end;
	kme_vbuf_top_kme_vbuf_top_40_reg.vbuf_blk_rim_right = me1_blk_rim_x_end;
	kme_vbuf_top_kme_vbuf_top_44_reg.vbuf_pix_rim_bot = lbme_rim_y_end;
	kme_vbuf_top_kme_vbuf_top_48_reg.vbuf_pix_rim_right = lbme_rim_x_end;
	kme_vbuf_top_kme_vbuf_top_84_reg.vbuf_meander_blk_rim_bot = me1_blk_rim_y_end;

	kme_me1_top2_kme_me1_top2_04_reg.me1_rmv_vact = image_me1_blk_vsize;
	kme_me1_top2_kme_me1_top2_04_reg.me1_rmv_hact = image_me1_blk_hsize;

	kme_lbme_top_kme_lbme_top_00_reg.lbme_rim_x_end = lbme_rim_x_end;
	kme_lbme_top_kme_lbme_top_04_reg.lbme_rim_y_end = lbme_rim_y_end;

	kme_lbme_top_kme_lbme_top_14_reg.lbme_total_pixel_num = image_me_hsize;
	kme_lbme_top_kme_lbme_top_14_reg.lbme_total_line_num = image_me_vsize;

	kme_lbme_top_kme_lbme_top_18_reg.lbme_blk_num_perline = image_lbme_addr_perline;
	if(image_me_vsize<kme_lbme_top_kme_lbme_top_18_reg.lbme_prefetch_linebuf_num)
		kme_lbme_top_kme_lbme_top_18_reg.lbme_prefetch_linebuf_num = image_me_vsize;

	kme_lbme_top_kme_lbme_top_1c_reg.lbme_meander_rim_x_end = lbme_meander_rim_x_end;

	kme_lbme_top_kme_lbme_top_20_reg.lbme_meander_rim_y_end = lbme_meander_rim_y_end;
	#if defined(MARK2) || defined (MERLIN7) 
	kme_ipme_kme_ipme_00_reg.ipme_pre_down2 = ipme_pre_down2;
	kme_ipme_kme_ipme_00_reg.ipme_h1_ration = 0x40;
	kme_ipme_kme_ipme_00_reg.ipme_h0_ration = 0x40;
	kme_ipme_kme_ipme_0c_reg.ipme_v1_ration = 0x10;
	kme_ipme_kme_ipme_0c_reg.ipme_v0_ration = 0x10;
	#endif
	kme_ipme_kme_ipme_00_reg.ipme_v_fetch_mode = ipme_dn;
	kme_ipme_kme_ipme_00_reg.ipme_h_fetch_mode = ipme_dn;


	kme_ipme_kme_ipme_0c_reg.ipme_scaler_h_active = image_me_hsize;
	kme_ipme_kme_ipme_24_reg.ipme_pscaler_v_active = GS_image_vsize;

	kme_me1_top1_me1_scene1_00_reg.me1_sc_mc_en = GS_scene_on;

	kme_lbme2_top_kme_lbme2_top_00_reg.lbme2_rim_x_end = image_me_hsize-1;
	kme_lbme2_top_kme_lbme2_top_04_reg.lbme2_rim_y_end = image_me_vsize-1;
	kme_lbme2_top_kme_lbme2_top_14_reg.lbme2_total_line_num = image_me_vsize;
	kme_lbme2_top_kme_lbme2_top_14_reg.lbme2_total_pixel_num = image_me_hsize;

	kme_lbme2_top_kme_lbme2_top_18_reg.lbme2_blk_num_perline = image_me_hsize/16;
	if(image_me_vsize<kme_lbme2_top_kme_lbme2_top_18_reg.lbme2_prefetch_linebuf_num)
		kme_lbme2_top_kme_lbme2_top_18_reg.lbme2_prefetch_linebuf_num = image_me_vsize;

	if(image_me2_blk_pfv_vsize<kme_me2_vbuf_top_kme_me2_vbuf_top_04_reg.kme_me2_vbuf_pfv_mv_prefetch_ln_num){
		kme_me2_vbuf_top_kme_me2_vbuf_top_04_reg.kme_me2_vbuf_pfv_mv_prefetch_ln_num = image_me2_blk_pfv_vsize;
		kme_me2_vbuf_top_kme_me2_vbuf_top_04_reg.kme_me2_vbuf_dm_pfv_req_interval = 2;
	}
	if(image_me2_blk_pfv_vsize<kme_me2_vbuf_top_kme_me2_vbuf_top_0c_reg.kme_me2_vbuf_ppfv_mv_prefetch_ln_num){
		kme_me2_vbuf_top_kme_me2_vbuf_top_0c_reg.kme_me2_vbuf_ppfv_mv_prefetch_ln_num = image_me2_blk_pfv_vsize;
		kme_me2_vbuf_top_kme_me2_vbuf_top_08_reg.kme_me2_vbuf_dm_ppfv_req_interval = 2;
	}
	#if (defined(MARK2)||defined(MERLIN7))
	if(GS_kme_dm_bg_1bit)
	{
		kme_me2_vbuf_top_kme_me2_vbuf_top_04_reg.kme_me2_vbuf_dm_pfv_req_interval = 2;
		kme_me2_vbuf_top_kme_me2_vbuf_top_08_reg.kme_me2_vbuf_dm_ppfv_req_interval = 2;
	}
	#endif
	kme_me2_vbuf_top_kme_me2_vbuf_top_04_reg.kme_me2_vbuf_pfv_mv_fetch_ln_1_row = me2_vbuf_pfv_mv_fetch_ln_1_row;
	kme_me2_vbuf_top_kme_me2_vbuf_top_0c_reg.kme_me2_vbuf_ppfv_mv_fetch_ln_1_row = me2_vbuf_pfv_mv_fetch_ln_1_row;

	kme_me2_vbuf_top_kme_me2_vbuf_top_1c_reg.kme_me2_vbuf_ph_blksize_hact = image_me2_blk_ph_hsize;
	kme_me2_vbuf_top_kme_me2_vbuf_top_1c_reg.kme_me2_vbuf_pfv_blksize_vact = image_me2_blk_pfv_vsize;
	kme_me2_vbuf_top_kme_me2_vbuf_top_1c_reg.kme_me2_vbuf_pfv_blksize_hact = image_me2_blk_pfv_hsize;

	kme_me2_vbuf_top_kme_me2_vbuf_top_20_reg.kme_me2_vbuf_pfv_blk_rim_right = image_me2_blk_ph_hsize-1;
	kme_me2_vbuf_top_kme_me2_vbuf_top_20_reg.kme_me2_vbuf_ph_blksize_vact = image_me2_blk_ph_vsize;

	kme_me2_vbuf_top_kme_me2_vbuf_top_24_reg.kme_me2_vbuf_pfv_blk_rim_bot =image_me2_blk_ph_vsize-1;

	kme_me2_vbuf_top_kme_me2_vbuf_top_28_reg.kme_me2_vbuf_ph_blk_rim_bot = image_me2_blk_ph_vsize - 1;
	kme_me2_vbuf_top_kme_me2_vbuf_top_28_reg.kme_me2_vbuf_ph_blk_rim_right = image_me2_blk_ph_hsize - 1;

	kme_me2_vbuf_top_kme_me2_vbuf_top_38_reg.kme_me2_vbuf_lbme_dm_num_1row = me2_lbme_dm_num_line;

	kme_me2_vbuf_top_kme_me2_vbuf_top_d4_reg.kme_me2_vbuf_pix_rim_right = image_me_hsize -1;
	kme_me2_vbuf_top_kme_me2_vbuf_top_d8_reg.kme_me2_vbuf_pix_rim_bot = image_me_vsize -1;
	
	kme_me2_vbuf_top_kme_me2_vbuf_top_e0_reg.kme_me2_row135to270_wait_row_nums = me2_wait_row_nums;

#if defined(MARK2)
	kme_me2_vbuf_top_kme_me2_vbuf_top_2c_reg.me2_refine_process 			= GS_me2_refine_scan;
#endif
	kme_me2_vbuf_top_kme_me2_vbuf_top_2c_reg.kme_me2_vbuf_doublescan_en 	= GS_me2_double_scan;
	kme_me2_vbuf_top_kme_me2_vbuf_top_2c_reg.kme_me2_vbuf_ph_pfv_h_ratio	= kme_me2_vbuf_ph_pfv_h_ratio;
	kme_me2_vbuf_top_kme_me2_vbuf_top_2c_reg.kme_me2_vbuf_ph_pfv_v_ratio	= kme_me2_vbuf_ph_pfv_v_ratio;
	kme_me2_vbuf_top_kme_me2_vbuf_top_c8_reg.kme_me2_vbuf_ph_lbme_v_ratio	= kme_me2_vbuf_ph_lbme_v_ratio;
	kme_me2_vbuf_top_kme_me2_vbuf_top_c8_reg.kme_me2_vbuf_ph_lbme_h_ratio	= kme_me2_vbuf_ph_lbme_h_ratio;

	//kme_me2_vbuf_top_kme_me2_vbuf_top_2c_reg.kme_me2_vbuf_1st_ph_candi_nums = me2_1st_ph_cand_num;
	//kme_me2_vbuf_top_kme_me2_vbuf_top_30_reg.kme_me2_vbuf_2nd_ph_candi_nums = me2_2nd_ph_cand_num;

	kme_me2_calc0_kme_me2_calc0_74_reg.me2_rmv_vact = image_me2_blk_ph_vsize;
	kme_me2_calc0_kme_me2_calc0_74_reg.me2_rmv_hact = image_me2_blk_ph_hsize;
	
	kme_me1_top6_kme_me1_top6_00_reg.me1_statis_vact = image_me1_blk_vsize;
	kme_me1_top6_kme_me1_top6_00_reg.me1_statis_hact = image_me1_blk_hsize;	// JIRA RL6702-1330

	kme_dehalo_kme_dehalo_10_reg.dh_mv_blk_num = image_me2_blk_ph_hsize;

	kme_dehalo_kme_dehalo_14_reg.dh_me2cal_in_480x270_mod =  dh_me2cal_in_480x270_mod;
	kme_dehalo_kme_dehalo_14_reg.dh_logo_row_num = image_me1_blk_vsize;
	kme_dehalo_kme_dehalo_14_reg.dh_logo_blk_num = image_me1_blk_hsize;
	kme_dehalo_kme_dehalo_14_reg.dh_mv_row_num = image_me2_blk_ph_vsize;
	kme_dehalo_kme_dehalo_18_reg.dh_right_ofst = dh_rim_blk;
	kme_dehalo_kme_dehalo_1c_reg.dh_vbuf_row_mask = dh_vbuf_row_mask;
	kme_dehalo_kme_dehalo_1c_reg.dh_bot_ofst = dh_rim_row;
	kme_dehalo_kme_dehalo_4c_reg.dh_rt1_xidx_shift = reg_dh_rt1_xidx_shift;
	kme_dehalo_kme_dehalo_50_reg.dh_rt1_yidx_shift = reg_dh_rt1_yidx_shift;
	kme_dehalo_kme_dehalo_78_reg.dh_rt2_vidx_ofst = reg_dh_rt2_vidx_shift;
	kme_dehalo_kme_dehalo_78_reg.dh_rt2_hidx_ofst = reg_dh_rt2_hidx_shift;

	//0406
	kme_dehalo_kme_dehalo_34_reg.dh_patt_row_num = image_me2_blk_ph_vsize;
	kme_dehalo_kme_dehalo_34_reg.dh_patt_blk_num = image_me2_blk_ph_hsize;
	//0406
	kpost_top_kpost_top_28_reg.kpost_4port_hde_center_value = GS_image_hsize/8;
	kpost_top_kpost_top_28_reg.hde_center_value = GS_image_hsize/4;

	// register write
	rtd_outl(KME_TOP_KME_TOP_04_reg, kme_top_kme_top_04_reg.regValue);
	rtd_outl(KME_TOP_KME_TOP_08_reg, kme_top_kme_top_08_reg.regValue);
	rtd_outl(KME_TOP_KME_TOP_30_reg, kme_top_kme_top_30_reg.regValue);
	rtd_outl(KME_TOP_KME_TOP_3C_reg, kme_top_kme_top_3c_reg.regValue);
	rtd_outl(KME_TOP_KME_TOP_CRTC_F8_reg, kme_top_kme_top_crtc_f8_reg.regValue);
	rtd_outl(KME_TOP_KME_TOP_CRTC_FC_reg, kme_top_kme_top_crtc_fc_reg.regValue);
	rtd_outl(KME_TOP_KME_TOP_CRTC_E0_reg, kme_top_kme_top_crtc_e0_reg.regValue);
	rtd_outl(KME_TOP_KME_TOP_CRTC_F4_reg, kme_top_kme_top_crtc_f4_reg.regValue);
	rtd_outl(KME_VBUF_TOP_KME_VBUF_TOP_00_reg, kme_vbuf_top_kme_vbuf_top_00_reg.regValue);
	rtd_outl(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, kme_vbuf_top_kme_vbuf_top_10_reg.regValue);
	rtd_outl(KME_VBUF_TOP_KME_VBUF_TOP_30_reg, kme_vbuf_top_kme_vbuf_top_30_reg.regValue);
	rtd_outl(KME_VBUF_TOP_KME_VBUF_TOP_34_reg, kme_vbuf_top_kme_vbuf_top_34_reg.regValue);
	rtd_outl(KME_VBUF_TOP_KME_VBUF_TOP_38_reg, kme_vbuf_top_kme_vbuf_top_38_reg.regValue);
	rtd_outl(KME_VBUF_TOP_KME_VBUF_TOP_40_reg, kme_vbuf_top_kme_vbuf_top_40_reg.regValue);
	rtd_outl(KME_VBUF_TOP_KME_VBUF_TOP_44_reg, kme_vbuf_top_kme_vbuf_top_44_reg.regValue);
	rtd_outl(KME_VBUF_TOP_KME_VBUF_TOP_48_reg, kme_vbuf_top_kme_vbuf_top_48_reg.regValue);
	rtd_outl(KME_VBUF_TOP_KME_VBUF_TOP_84_reg, kme_vbuf_top_kme_vbuf_top_84_reg.regValue);
	rtd_outl(KME_ME1_TOP2_KME_ME1_TOP2_04_reg, kme_me1_top2_kme_me1_top2_04_reg.regValue);
	rtd_outl(KME_LBME_TOP_KME_LBME_TOP_00_reg, kme_lbme_top_kme_lbme_top_00_reg.regValue);
	rtd_outl(KME_LBME_TOP_KME_LBME_TOP_04_reg, kme_lbme_top_kme_lbme_top_04_reg.regValue);
	rtd_outl(KME_LBME_TOP_KME_LBME_TOP_14_reg, kme_lbme_top_kme_lbme_top_14_reg.regValue);
	rtd_outl(KME_LBME_TOP_KME_LBME_TOP_18_reg, kme_lbme_top_kme_lbme_top_18_reg.regValue);
	rtd_outl(KME_LBME_TOP_KME_LBME_TOP_1C_reg, kme_lbme_top_kme_lbme_top_1c_reg.regValue);
	rtd_outl(KME_LBME_TOP_KME_LBME_TOP_20_reg, kme_lbme_top_kme_lbme_top_20_reg.regValue);
	rtd_outl(KME_IPME_KME_IPME_00_reg, kme_ipme_kme_ipme_00_reg.regValue);
	rtd_outl(KME_IPME_KME_IPME_0C_reg, kme_ipme_kme_ipme_0c_reg.regValue);
	rtd_outl(KME_IPME_KME_IPME_24_reg, kme_ipme_kme_ipme_24_reg.regValue);
	rtd_outl(KME_ME1_TOP1_ME1_SCENE1_00_reg, kme_me1_top1_me1_scene1_00_reg.regValue);
	rtd_outl(KME_LBME2_TOP_KME_LBME2_TOP_00_reg, kme_lbme2_top_kme_lbme2_top_00_reg.regValue);
	rtd_outl(KME_LBME2_TOP_KME_LBME2_TOP_04_reg, kme_lbme2_top_kme_lbme2_top_04_reg.regValue);
	rtd_outl(KME_LBME2_TOP_KME_LBME2_TOP_14_reg, kme_lbme2_top_kme_lbme2_top_14_reg.regValue);
	rtd_outl(KME_LBME2_TOP_KME_LBME2_TOP_18_reg, kme_lbme2_top_kme_lbme2_top_18_reg.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_04_reg, kme_me2_vbuf_top_kme_me2_vbuf_top_04_reg.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_0C_reg, kme_me2_vbuf_top_kme_me2_vbuf_top_0c_reg.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_08_reg, kme_me2_vbuf_top_kme_me2_vbuf_top_08_reg.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_1C_reg, kme_me2_vbuf_top_kme_me2_vbuf_top_1c_reg.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_20_reg, kme_me2_vbuf_top_kme_me2_vbuf_top_20_reg.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_24_reg, kme_me2_vbuf_top_kme_me2_vbuf_top_24_reg.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_28_reg, kme_me2_vbuf_top_kme_me2_vbuf_top_28_reg.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_38_reg, kme_me2_vbuf_top_kme_me2_vbuf_top_38_reg.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D4_reg, kme_me2_vbuf_top_kme_me2_vbuf_top_d4_reg.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg, kme_me2_vbuf_top_kme_me2_vbuf_top_d8_reg.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_E0_reg, kme_me2_vbuf_top_kme_me2_vbuf_top_e0_reg.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_2C_reg, kme_me2_vbuf_top_kme_me2_vbuf_top_2c_reg.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_C8_reg, kme_me2_vbuf_top_kme_me2_vbuf_top_c8_reg.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg, kme_me2_vbuf_top_kme_me2_vbuf_top_30_reg.regValue);
	rtd_outl(KME_ME2_CALC0_KME_ME2_CALC0_74_reg, kme_me2_calc0_kme_me2_calc0_74_reg.regValue);
	rtd_outl(KME_ME1_TOP6_KME_ME1_TOP6_00_reg, kme_me1_top6_kme_me1_top6_00_reg.regValue);
	rtd_outl(KME_DEHALO_KME_DEHALO_10_reg, kme_dehalo_kme_dehalo_10_reg.regValue);
	rtd_outl(KME_DEHALO_KME_DEHALO_14_reg, kme_dehalo_kme_dehalo_14_reg.regValue);
	rtd_outl(KME_DEHALO_KME_DEHALO_18_reg, kme_dehalo_kme_dehalo_18_reg.regValue);
	rtd_outl(KME_DEHALO_KME_DEHALO_1C_reg, kme_dehalo_kme_dehalo_1c_reg.regValue);
	rtd_outl(KME_DEHALO_KME_DEHALO_4C_reg, kme_dehalo_kme_dehalo_4c_reg.regValue);
	rtd_outl(KME_DEHALO_KME_DEHALO_50_reg, kme_dehalo_kme_dehalo_50_reg.regValue);
	rtd_outl(KME_DEHALO_KME_DEHALO_78_reg, kme_dehalo_kme_dehalo_78_reg.regValue);
	rtd_outl(KME_DEHALO_KME_DEHALO_34_reg, kme_dehalo_kme_dehalo_34_reg.regValue);
	rtd_outl(KPOST_TOP_KPOST_TOP_28_reg, kpost_top_kpost_top_28_reg.regValue);

}
void MEMC_ChangeSize_LBMC(void)
{
	// parameter declaration
	//======= limit
	int lbmc_mode = 1;
	int lbmc_pr_lrinv = 0;//pwr_opt
	int prefetch_limit;
	int prefetch_limit_s;
	int prefetch_limit_lf = 0;
	int prefetch_limit_lf_s = 0;
	//int sr_limit_lf;
	//int sr_limit_lf_si;
	//int sr_limit_lf_sp;
	//int sr_limit_hf;
	//int sr_limit_hf_si;
	//int sr_limit_hf_sp;
	int prefetch_lf0 = 0;
	int prefetch_lf1 = 0;
	int prefetch_lf2 = 0;
	int prefetch_lf3 = 0;
	int prefetch_lf4 = 0;
	int prefetch_lf5 = 0;
	int prefetch_lf6 = 0;
	int prefetch_lf7 = 0;
	int prefetch_lf8 = 0; //reg_bit
	int prefetch_lf0_si = 0;
	int prefetch_lf1_si = 0;
	int prefetch_lf2_si = 0;
	int prefetch_lf3_si = 0;
	int prefetch_lf4_si = 0;
	int prefetch_lf5_si = 0;
	int prefetch_lf6_si = 0;
	int prefetch_lf7_si = 0;
	int prefetch_lf8_si = 0;
	int prefetch_lf0_sp = 0;
	int prefetch_lf1_sp = 0;
	int prefetch_lf2_sp = 0;
	int prefetch_lf3_sp = 0;
	int prefetch_lf4_sp = 0;
	int prefetch_lf5_sp = 0;
	int prefetch_lf6_sp = 0;
	int prefetch_lf7_sp = 0;
	int prefetch_lf8_sp = 0;

	int prefetch_hf0 = 0;
	int prefetch_hf1 = 0;
	int prefetch_hf2 = 0;
	int prefetch_hf0_si = 0;
	int prefetch_hf1_si = 0;
	int prefetch_hf2_si = 0;
	int prefetch_hf0_sp = 0;
	int prefetch_hf1_sp = 0;
	int prefetch_hf2_sp = 0;

	//======= pcmode
	//pc_mode
	int pc_mode_turn_on  = (GS_pc_mode)? 0x1 : 0x0;
	//int pc_mode_turn_off = (GS_pc_mode)? 0x0 : 0x1;
	//indata_pc_mode
	//int indata_pc_mode_turn_on  = ((GS_pc_mode==1) & (GS_pc_repeat_mode==0))? 0x1 : 0x0;
	//int indata_pc_mode_turn_off = ((GS_pc_mode==1) & (GS_pc_repeat_mode==0))? 0x0 : 0x1;



	int prefetch_lfh0 = 0x7;//odd
	int prefetch_lfh1 = 0xd;
	int prefetch_lfh2 = 0xd; //0x14 //reg_bit
	int prefetch_lfh0_si = 0x5;
	int prefetch_lfh1_si = 0x11;
	int prefetch_lfh2_si = 0x1d;
	int prefetch_lfh0_sp = 0x7; 
	int prefetch_lfh1_sp = 0x7;
	int prefetch_lfh2_sp = 0x7;



	//sr
	int lf_sr0    = 0;
	int lf_sr1    = 0;
	int lf_sr2    = 0;
	int lf_sr3    = 0;
	int lf_sr4    = 0;
	int lf_sr5    = 0;
	int lf_sr6    = 0;
	int lf_sr7    = 0;
	int lf_sr8    = 0;
	int lf_sr0_si = 0;
	int lf_sr1_si = 0;
	int lf_sr2_si = 0;
	int lf_sr3_si = 0;
	int lf_sr4_si = 0;
	int lf_sr5_si = 0;
	int lf_sr6_si = 0;
	int lf_sr7_si = 0;
	int lf_sr8_si = 0;
	int lf_sr0_sp = 0;
	int lf_sr1_sp = 0;
	int lf_sr2_sp = 0;
	int lf_sr3_sp = 0;
	int lf_sr4_sp = 0;
	int lf_sr5_sp = 0;
	int lf_sr6_sp = 0;
	int lf_sr7_sp = 0;
	int lf_sr8_sp = 0;

	int hf_sr0    = 0;
	int hf_sr1    = 0;
	int hf_sr2    = 0;
	int hf_sr0_si = 0;
	int hf_sr1_si = 0;
	int hf_sr2_si = 0;
	int hf_sr0_sp = 0;
	int hf_sr1_sp = 0;
	int hf_sr2_sp = 0;

	int lfh_sr0    = 0;
	int lfh_sr1    = 0;
	int lfh_sr2    = 0;
	int lfh_sr0_si = 0;
	int lfh_sr1_si = 0;
	int lfh_sr2_si = 0;
	int lfh_sr0_sp = 0;
	int lfh_sr1_sp = 0;
	int lfh_sr2_sp = 0;

	// register declaration
	lbmc_lbmc_00_RBUS lbmc_lbmc_00_reg;
	lbmc_lbmc_04_RBUS lbmc_lbmc_04_reg;
	lbmc_lbmc_08_RBUS lbmc_lbmc_08_reg;
	lbmc_lbmc_0c_RBUS lbmc_lbmc_0c_reg;
	lbmc_lbmc_10_RBUS lbmc_lbmc_10_reg;
	lbmc_lbmc_14_RBUS lbmc_lbmc_14_reg;
	lbmc_lbmc_20_RBUS lbmc_lbmc_20_reg;
	lbmc_lbmc_24_RBUS lbmc_lbmc_24_reg;
	lbmc_lbmc_2c_RBUS lbmc_lbmc_2c_reg;
	lbmc_lbmc_30_RBUS lbmc_lbmc_30_reg;
	lbmc_lbmc_34_RBUS lbmc_lbmc_34_reg;
	lbmc_lbmc_38_RBUS lbmc_lbmc_38_reg;
	lbmc_lbmc_3c_RBUS lbmc_lbmc_3c_reg;
	lbmc_lbmc_40_RBUS lbmc_lbmc_40_reg;
	lbmc_lbmc_44_RBUS lbmc_lbmc_44_reg;
	lbmc_lbmc_48_RBUS lbmc_lbmc_48_reg;
	lbmc_lbmc_4c_RBUS lbmc_lbmc_4c_reg;
	lbmc_lbmc_50_RBUS lbmc_lbmc_50_reg;
	lbmc_lbmc_54_RBUS lbmc_lbmc_54_reg;
	lbmc_lbmc_58_RBUS lbmc_lbmc_58_reg;
	lbmc_lbmc_5c_RBUS lbmc_lbmc_5c_reg;
	lbmc_lbmc_60_RBUS lbmc_lbmc_60_reg;
	lbmc_lbmc_88_RBUS lbmc_lbmc_88_reg;
	lbmc_lbmc_d0_RBUS lbmc_lbmc_d0_reg;
	lbmc_lbmc_d4_RBUS lbmc_lbmc_d4_reg;
	lbmc_lbmc_d8_RBUS lbmc_lbmc_d8_reg;
	lbmc_lbmc_dc_RBUS lbmc_lbmc_dc_reg;

	// register read back
	lbmc_lbmc_00_reg.regValue = rtd_inl(LBMC_LBMC_00_reg);
	lbmc_lbmc_04_reg.regValue = rtd_inl(LBMC_LBMC_04_reg);
	lbmc_lbmc_08_reg.regValue = rtd_inl(LBMC_LBMC_08_reg);
	lbmc_lbmc_0c_reg.regValue = rtd_inl(LBMC_LBMC_0C_reg);
	lbmc_lbmc_10_reg.regValue = rtd_inl(LBMC_LBMC_10_reg);
	lbmc_lbmc_14_reg.regValue = rtd_inl(LBMC_LBMC_14_reg);
	lbmc_lbmc_20_reg.regValue = rtd_inl(LBMC_LBMC_20_reg);
	lbmc_lbmc_24_reg.regValue = rtd_inl(LBMC_LBMC_24_reg);
	lbmc_lbmc_2c_reg.regValue = rtd_inl(LBMC_LBMC_2C_reg);
	lbmc_lbmc_30_reg.regValue = rtd_inl(LBMC_LBMC_30_reg);
	lbmc_lbmc_34_reg.regValue = rtd_inl(LBMC_LBMC_34_reg);
	lbmc_lbmc_38_reg.regValue = rtd_inl(LBMC_LBMC_38_reg);
	lbmc_lbmc_3c_reg.regValue = rtd_inl(LBMC_LBMC_3C_reg);
	lbmc_lbmc_40_reg.regValue = rtd_inl(LBMC_LBMC_40_reg);
	lbmc_lbmc_44_reg.regValue = rtd_inl(LBMC_LBMC_44_reg);
	lbmc_lbmc_48_reg.regValue = rtd_inl(LBMC_LBMC_48_reg);
	lbmc_lbmc_4c_reg.regValue = rtd_inl(LBMC_LBMC_4C_reg);
	lbmc_lbmc_50_reg.regValue = rtd_inl(LBMC_LBMC_50_reg);
	lbmc_lbmc_54_reg.regValue = rtd_inl(LBMC_LBMC_54_reg);
	lbmc_lbmc_58_reg.regValue = rtd_inl(LBMC_LBMC_58_reg);
	lbmc_lbmc_5c_reg.regValue = rtd_inl(LBMC_LBMC_5C_reg);
	lbmc_lbmc_60_reg.regValue = rtd_inl(LBMC_LBMC_60_reg);
	lbmc_lbmc_88_reg.regValue = rtd_inl(LBMC_LBMC_88_reg);
	lbmc_lbmc_d0_reg.regValue = rtd_inl(LBMC_LBMC_D0_reg);
	lbmc_lbmc_d4_reg.regValue = rtd_inl(LBMC_LBMC_D4_reg);
	lbmc_lbmc_d8_reg.regValue = rtd_inl(LBMC_LBMC_D8_reg);
	lbmc_lbmc_dc_reg.regValue = rtd_inl(LBMC_LBMC_DC_reg);

	// main function body
	if(GS_merlin5_timing)
	{
		if((GS_image_hsize>3840) || (GS_image_vsize > 2160))//theoretical value = vporch(180) * 1100(2px 2200/2)/1920=103
		{
			prefetch_limit = 101;
			prefetch_limit_s = 100;
		}
		else if((GS_image_hsize>1920) || (GS_image_vsize > 1080))//theoretical value = vporch(90) * 1100(2px 2200/2)/1920=51
		{
			prefetch_limit = 49;
			prefetch_limit_s = 50;
		}
		else //theoretical value = vporch(45) * 1100(2px 2200/2)/1920=25
		{
			prefetch_limit = 21;
			prefetch_limit_s = 20;
		}
	}
	else
	{
		prefetch_limit = 255;
		prefetch_limit_s = 254;
	}

	#if (defined(MERLIN6) || defined(MERLIN7))
	{
		prefetch_limit_lf = prefetch_limit;
		prefetch_limit_lf_s = prefetch_limit_s;	
	}
	#endif

	#if defined(MARK2)//lf pqdc 2px,prefetch*2
	{
		prefetch_limit_lf = 2*prefetch_limit-1;
		prefetch_limit_lf_s = 2*prefetch_limit_s-1;
	}
	#endif

        	//==lbmc_mode
	if(GS_ipme_dn_ratio == 8)
	{
		lbmc_mode = 0;
	} 
	else if(GS_ipme_dn_ratio == 4)
	{
		if(GS_me2_2x2 == 1)
		{
			lbmc_mode = 1;
		} 
		else
		{
			lbmc_mode = 0;
		}
	} 
	else
	{
		lbmc_mode = 1;
	}


	#if defined(MARK2)
	{
		if(lbmc_mode == 0)
		{lbmc_pr_lrinv =0;}//rtl bug
		else 
		{lbmc_pr_lrinv =GS_lbmc_pwr_opt;}
	}
	#endif

	//======= prefetch
	//prefetch
	#if (defined(MERLIN6) || defined(MERLIN7))
	prefetch_lf0 = 0x5;
	prefetch_lf1 = 0x9;
	prefetch_lf2 = 0xd;
	prefetch_lf3 = 0x11;
	prefetch_lf4 = 0x15;
	prefetch_lf5 = 0x19;
	prefetch_lf6 = 0x1d;
	prefetch_lf7 = 0x21;
	prefetch_lf8 = 0x25; //reg_bit
	prefetch_lf0_si = 0x2;
	prefetch_lf1_si = 0xa;
	prefetch_lf2_si = 0x12;
	prefetch_lf3_si = 0x1a;
	prefetch_lf4_si = 0x22;
	prefetch_lf5_si = 0x2a;
	prefetch_lf6_si = 0x32;
	prefetch_lf7_si = 0x3a;
	prefetch_lf8_si = 0x42;
	prefetch_lf0_sp = 0x8;
	prefetch_lf1_sp = 0x8;
	prefetch_lf2_sp = 0x8;
	prefetch_lf3_sp = 0x8;
	prefetch_lf4_sp = 0x8;
	prefetch_lf5_sp = 0x8;
	prefetch_lf6_sp = 0x8;
	prefetch_lf7_sp = 0x8;
	prefetch_lf8_sp = 0x8;

	prefetch_hf0 = (GS_lbmc_nline_option==0)? 0x5 : 0x3;
	prefetch_hf1 = 0x7;
	prefetch_hf2 = (GS_lbmc_nline_option==0)? 0x9 : 0xb;
	
	prefetch_hf0_si = 0x6;
	prefetch_hf1_si = 0xa;
	prefetch_hf2_si = 0xe;
	prefetch_hf0_sp = 0x4;
	prefetch_hf1_sp = 0x4;
	prefetch_hf2_sp = 0x4;
	#endif

	#if defined(MARK2)
                //mark2_prefetch_lf  = 2*m6_prefetch_lf
	prefetch_lf0 = 0x9;
	prefetch_lf1 = 0x11;
	prefetch_lf2 = 0x19;
	prefetch_lf3 = 0x21;
	prefetch_lf4 = 0x29;
	prefetch_lf5 = 0x31;
	prefetch_lf6 = 0x39;
	prefetch_lf7 = 0x41;
	prefetch_lf8 = 0x41; //reg_bit
	prefetch_lf0_si = 0x9;
	prefetch_lf1_si = 0x19;
	prefetch_lf2_si = 0x29;
	prefetch_lf3_si = 0x39;
	prefetch_lf4_si = 0x49;
	prefetch_lf5_si = 0x59;
	prefetch_lf6_si = 0x69;
	prefetch_lf7_si = 0x79;
	prefetch_lf8_si = 0x7d;
	prefetch_lf0_sp = 0x7;
	prefetch_lf1_sp = 0x7;
	prefetch_lf2_sp = 0x7;
	prefetch_lf3_sp = 0x7;
	prefetch_lf4_sp = 0x7;
	prefetch_lf5_sp = 0x7;
	prefetch_lf6_sp = 0x7;
	prefetch_lf7_sp = 0x7;
	prefetch_lf8_sp = 0x7;
	
	if(lbmc_mode==0)//modeA 16x16
	{
	 prefetch_hf0 = (GS_lbmc_nline_option==0)? 0x5 : 0x3;
	 prefetch_hf1 = 0x7;
	 prefetch_hf2 = (GS_lbmc_nline_option==0)? 0x9 : 0xb;
	 prefetch_hf0_si = 0x5;
	 prefetch_hf1_si = 0x9;
	 prefetch_hf2_si = 0xd;
	}
  else 
	{
   prefetch_hf0 =0x5;
	 prefetch_hf1 =0x7;
	 prefetch_hf2 =0x9;
	 prefetch_hf0_si = 0x5;
	 prefetch_hf1_si = 0x11;
	 prefetch_hf2_si = 0x1d;
  }
	prefetch_hf0_sp = 0x5;
	prefetch_hf1_sp = 0x5;
	prefetch_hf2_sp = 0x5;
	#endif
	
	//prefetch limit
	prefetch_lf0    =  MIN(prefetch_lf0   , prefetch_limit_lf); // (prefetch_lf0   > prefetch_limit_lf)? prefetch_limit_lf :  prefetch_lf0   ; 
	prefetch_lf1    =  MIN(prefetch_lf1   , prefetch_limit_lf); // (prefetch_lf1   > prefetch_limit_lf)? prefetch_limit_lf :  prefetch_lf1   ; 
	prefetch_lf2    =  MIN(prefetch_lf2   , prefetch_limit_lf); // (prefetch_lf2   > prefetch_limit_lf)? prefetch_limit_lf :  prefetch_lf2   ; 
	prefetch_lf3    =  MIN(prefetch_lf3   , prefetch_limit_lf); // (prefetch_lf3   > prefetch_limit_lf)? prefetch_limit_lf :  prefetch_lf3   ; 
	prefetch_lf4    =  MIN(prefetch_lf4   , prefetch_limit_lf); // (prefetch_lf4   > prefetch_limit_lf)? prefetch_limit_lf :  prefetch_lf4   ; 
	prefetch_lf5    =  MIN(prefetch_lf5   , prefetch_limit_lf); // (prefetch_lf5   > prefetch_limit_lf)? prefetch_limit_lf :  prefetch_lf5   ; 
	prefetch_lf6    =  MIN(prefetch_lf6   , prefetch_limit_lf); // (prefetch_lf6   > prefetch_limit_lf)? prefetch_limit_lf :  prefetch_lf6   ; 
	prefetch_lf7    =  MIN(prefetch_lf7   , prefetch_limit_lf); // (prefetch_lf7   > prefetch_limit_lf)? prefetch_limit_lf :  prefetch_lf7   ; 
	prefetch_lf8    =  MIN(prefetch_lf8   , prefetch_limit_lf); // (prefetch_lf8   > prefetch_limit_lf)? prefetch_limit_lf :  prefetch_lf8   ; 
	prefetch_lf0_si =  MIN(prefetch_lf0_si, prefetch_limit_lf_s); // (prefetch_lf0_si> prefetch_limit_lf_s)? prefetch_limit_lf_s :  prefetch_lf0_si; 
	prefetch_lf1_si =  MIN(prefetch_lf1_si, prefetch_limit_lf_s); // (prefetch_lf1_si> prefetch_limit_lf_s)? prefetch_limit_lf_s :  prefetch_lf1_si; 
	prefetch_lf2_si =  MIN(prefetch_lf2_si, prefetch_limit_lf_s); // (prefetch_lf2_si> prefetch_limit_lf_s)? prefetch_limit_lf_s :  prefetch_lf2_si; 
	prefetch_lf3_si =  MIN(prefetch_lf3_si, prefetch_limit_lf_s); // (prefetch_lf3_si> prefetch_limit_lf_s)? prefetch_limit_lf_s :  prefetch_lf3_si; 
	prefetch_lf4_si =  MIN(prefetch_lf4_si, prefetch_limit_lf_s); // (prefetch_lf4_si> prefetch_limit_lf_s)? prefetch_limit_lf_s :  prefetch_lf4_si; 
	prefetch_lf5_si =  MIN(prefetch_lf5_si, prefetch_limit_lf_s); // (prefetch_lf5_si> prefetch_limit_lf_s)? prefetch_limit_lf_s :  prefetch_lf5_si; 
	prefetch_lf6_si =  MIN(prefetch_lf6_si, prefetch_limit_lf_s); // (prefetch_lf6_si> prefetch_limit_lf_s)? prefetch_limit_lf_s :  prefetch_lf6_si; 
	prefetch_lf7_si =  MIN(prefetch_lf7_si, prefetch_limit_lf_s); // (prefetch_lf7_si> prefetch_limit_lf_s)? prefetch_limit_lf_s :  prefetch_lf7_si; 
	prefetch_lf8_si =  MIN(prefetch_lf8_si, prefetch_limit_lf_s); // (prefetch_lf8_si> prefetch_limit_lf_s)? prefetch_limit_lf_s :  prefetch_lf8_si; 
	prefetch_lf0_sp =  MIN(prefetch_lf0_sp, prefetch_limit_lf_s); // (prefetch_lf0_sp> prefetch_limit_lf_s)? prefetch_limit_lf_s :  prefetch_lf0_sp; 
	prefetch_lf1_sp =  MIN(prefetch_lf1_sp, prefetch_limit_lf_s); // (prefetch_lf1_sp> prefetch_limit_lf_s)? prefetch_limit_lf_s :  prefetch_lf1_sp; 
	prefetch_lf2_sp =  MIN(prefetch_lf2_sp, prefetch_limit_lf_s); // (prefetch_lf2_sp> prefetch_limit_lf_s)? prefetch_limit_lf_s :  prefetch_lf2_sp; 
	prefetch_lf3_sp =  MIN(prefetch_lf3_sp, prefetch_limit_lf_s); // (prefetch_lf3_sp> prefetch_limit_lf_s)? prefetch_limit_lf_s :  prefetch_lf3_sp; 
	prefetch_lf4_sp =  MIN(prefetch_lf4_sp, prefetch_limit_lf_s); // (prefetch_lf4_sp> prefetch_limit_lf_s)? prefetch_limit_lf_s :  prefetch_lf4_sp; 
	prefetch_lf5_sp =  MIN(prefetch_lf5_sp, prefetch_limit_lf_s); // (prefetch_lf5_sp> prefetch_limit_lf_s)? prefetch_limit_lf_s :  prefetch_lf5_sp; 
	prefetch_lf6_sp =  MIN(prefetch_lf6_sp, prefetch_limit_lf_s); // (prefetch_lf6_sp> prefetch_limit_lf_s)? prefetch_limit_lf_s :  prefetch_lf6_sp; 
	prefetch_lf7_sp =  MIN(prefetch_lf7_sp, prefetch_limit_lf_s); // (prefetch_lf7_sp> prefetch_limit_lf_s)? prefetch_limit_lf_s :  prefetch_lf7_sp; 
	prefetch_lf8_sp =  MIN(prefetch_lf8_sp, prefetch_limit_lf_s); // (prefetch_lf8_sp> prefetch_limit_lf_s)? prefetch_limit_lf_s :  prefetch_lf8_sp; 

	prefetch_hf0    =  MIN(prefetch_hf0   , prefetch_limit);   // (prefetch_hf0   > prefetch_limit)? prefetch_limit :  prefetch_hf0   ; 
	prefetch_hf1    =  MIN(prefetch_hf1   , prefetch_limit);   // (prefetch_hf1   > prefetch_limit)? prefetch_limit :  prefetch_hf1   ; 
	prefetch_hf2    =  MIN(prefetch_hf2   , prefetch_limit);   // (prefetch_hf2   > prefetch_limit)? prefetch_limit :  prefetch_hf2   ; 
	#if (defined(MERLIN6) || defined(MERLIN7))
	prefetch_hf0_si =  MIN(prefetch_hf0_si, prefetch_limit_s); // (prefetch_hf0_si> prefetch_limit_s)? prefetch_limit_s :  prefetch_hf0_si; 
	prefetch_hf1_si =  MIN(prefetch_hf1_si, prefetch_limit_s); // (prefetch_hf1_si> prefetch_limit_s)? prefetch_limit_s :  prefetch_hf1_si; 
	prefetch_hf2_si =  MIN(prefetch_hf2_si, prefetch_limit_s); // (prefetch_hf2_si> prefetch_limit_s)? prefetch_limit_s :  prefetch_hf2_si; 
	prefetch_hf0_sp =  MIN(prefetch_hf0_sp, prefetch_limit_s); // (prefetch_hf0_sp> prefetch_limit_s)? prefetch_limit_s :  prefetch_hf0_sp; 
	prefetch_hf1_sp =  MIN(prefetch_hf1_sp, prefetch_limit_s); // (prefetch_hf1_sp> prefetch_limit_s)? prefetch_limit_s :  prefetch_hf1_sp; 
	prefetch_hf2_sp =  MIN(prefetch_hf2_sp, prefetch_limit_s); // (prefetch_hf2_sp> prefetch_limit_s)? prefetch_limit_s :  prefetch_hf2_sp; 
	#endif
	#if defined(MARK2)
	prefetch_hf0_si =  MIN(prefetch_hf0_si, prefetch_limit_s-1); // (prefetch_hf0_si> prefetch_limit_s)? prefetch_limit_s :  prefetch_hf0_si; 
	prefetch_hf1_si =  MIN(prefetch_hf1_si, prefetch_limit_s-1); // (prefetch_hf1_si> prefetch_limit_s)? prefetch_limit_s :  prefetch_hf1_si; 
	prefetch_hf2_si =  MIN(prefetch_hf2_si, prefetch_limit_s-1); // (prefetch_hf2_si> prefetch_limit_s)? prefetch_limit_s :  prefetch_hf2_si; 
	prefetch_hf0_sp =  MIN(prefetch_hf0_sp, prefetch_limit_s-1); // (prefetch_hf0_sp> prefetch_limit_s)? prefetch_limit_s :  prefetch_hf0_sp; 
	prefetch_hf1_sp =  MIN(prefetch_hf1_sp, prefetch_limit_s-1); // (prefetch_hf1_sp> prefetch_limit_s)? prefetch_limit_s :  prefetch_hf1_sp; 
	prefetch_hf2_sp =  MIN(prefetch_hf2_sp, prefetch_limit_s-1); // (prefetch_hf2_sp> prefetch_limit_s)? prefetch_limit_s :  prefetch_hf2_sp; 
	#endif
	prefetch_lfh0    =  MIN(prefetch_lfh0   , prefetch_limit);   // (prefetch_lfh0   > prefetch_limit)? prefetch_limit :  prefetch_lfh0   ; 
	prefetch_lfh1    =  MIN(prefetch_lfh1   , prefetch_limit);   // (prefetch_lfh1   > prefetch_limit)? prefetch_limit :  prefetch_lfh1   ; 
	prefetch_lfh2    =  MIN(prefetch_lfh2   , prefetch_limit);   // (prefetch_lfh2   > prefetch_limit)? prefetch_limit :  prefetch_lfh2   ; 
	#if (defined(MERLIN6) || defined(MERLIN7))
	prefetch_lfh0_si =  MIN(prefetch_lfh0_si, prefetch_limit_s); // (prefetch_lfh0_si> prefetch_limit_s)? prefetch_limit_s :  prefetch_lfh0_si; 
	prefetch_lfh1_si =  MIN(prefetch_lfh1_si, prefetch_limit_s); // (prefetch_lfh1_si> prefetch_limit_s)? prefetch_limit_s :  prefetch_lfh1_si; 
	prefetch_lfh2_si =  MIN(prefetch_lfh2_si, prefetch_limit_s); // (prefetch_lfh2_si> prefetch_limit_s)? prefetch_limit_s :  prefetch_lfh2_si; 
	prefetch_lfh0_sp =  MIN(prefetch_lfh0_sp, prefetch_limit_s); // (prefetch_lfh0_sp> prefetch_limit_s)? prefetch_limit_s :  prefetch_lfh0_sp; 
	prefetch_lfh1_sp =  MIN(prefetch_lfh1_sp, prefetch_limit_s); // (prefetch_lfh1_sp> prefetch_limit_s)? prefetch_limit_s :  prefetch_lfh1_sp; 
	prefetch_lfh2_sp =  MIN(prefetch_lfh2_sp, prefetch_limit_s); // (prefetch_lfh2_sp> prefetch_limit_s)? prefetch_limit_s :  prefetch_lfh2_sp; 
	#endif
	#if defined(MARK2)
	prefetch_lfh0_si =  MIN(prefetch_lfh0_si, prefetch_limit_s-1); // (prefetch_lfh0_si> prefetch_limit_s)? prefetch_limit_s :  prefetch_lfh0_si; 
	prefetch_lfh1_si =  MIN(prefetch_lfh1_si, prefetch_limit_s-1); // (prefetch_lfh1_si> prefetch_limit_s)? prefetch_limit_s :  prefetch_lfh1_si; 
	prefetch_lfh2_si =  MIN(prefetch_lfh2_si, prefetch_limit_s-1); // (prefetch_lfh2_si> prefetch_limit_s)? prefetch_limit_s :  prefetch_lfh2_si; 
	prefetch_lfh0_sp =  MIN(prefetch_lfh0_sp, prefetch_limit_s-1); // (prefetch_lfh0_sp> prefetch_limit_s)? prefetch_limit_s :  prefetch_lfh0_sp; 
	prefetch_lfh1_sp =  MIN(prefetch_lfh1_sp, prefetch_limit_s-1); // (prefetch_lfh1_sp> prefetch_limit_s)? prefetch_limit_s :  prefetch_lfh1_sp; 
	prefetch_lfh2_sp =  MIN(prefetch_lfh2_sp, prefetch_limit_s-1); // (prefetch_lfh2_sp> prefetch_limit_s)? prefetch_limit_s :  prefetch_lfh2_sp; 
	#endif

	//sr
	lf_sr0    =prefetch_lf0 - 0x2;
	lf_sr1    =prefetch_lf1 - 0x2;
	lf_sr2    =prefetch_lf2 - 0x2;
	lf_sr3    =prefetch_lf3 - 0x2;
	lf_sr4    =prefetch_lf4 - 0x2;
	lf_sr5    =prefetch_lf5 - 0x2;
	lf_sr6    =prefetch_lf6 - 0x2;
	lf_sr7    =prefetch_lf7 - 0x2;
	lf_sr8    =prefetch_lf8 - 0x2;
	lf_sr0_si =prefetch_lf0_si - 0x2;
	lf_sr1_si =prefetch_lf1_si - 0x2;
	lf_sr2_si =prefetch_lf2_si - 0x2;
	lf_sr3_si =prefetch_lf3_si - 0x2;
	lf_sr4_si =prefetch_lf4_si - 0x2;
	lf_sr5_si =prefetch_lf5_si - 0x2;
	lf_sr6_si =prefetch_lf6_si - 0x2;
	lf_sr7_si =prefetch_lf7_si - 0x2;
	lf_sr8_si =prefetch_lf8_si - 0x2;
	lf_sr0_sp =prefetch_lf0_sp - 0x2;
	lf_sr1_sp =prefetch_lf1_sp - 0x2;
	lf_sr2_sp =prefetch_lf2_sp - 0x2;
	lf_sr3_sp =prefetch_lf3_sp - 0x2;
	lf_sr4_sp =prefetch_lf4_sp - 0x2;
	lf_sr5_sp =prefetch_lf5_sp - 0x2;
	lf_sr6_sp =prefetch_lf6_sp - 0x2;
	lf_sr7_sp =prefetch_lf7_sp - 0x2;
	lf_sr8_sp =prefetch_lf8_sp - 0x2;

	hf_sr0    = prefetch_hf0 - 0x2;
	hf_sr1    = prefetch_hf1 - 0x2;
	hf_sr2    = prefetch_hf2 - 0x2;
	hf_sr0_si = prefetch_hf0_si - 0x2;
	hf_sr1_si = prefetch_hf1_si - 0x2;
	hf_sr2_si = prefetch_hf2_si - 0x2;
	hf_sr0_sp = prefetch_hf0_sp - 0x2;
	hf_sr1_sp = prefetch_hf1_sp - 0x2;
	hf_sr2_sp = prefetch_hf2_sp - 0x2;

	lfh_sr0    = prefetch_lfh0 - 0x2;
	lfh_sr1    = prefetch_lfh1 - 0x2;
	lfh_sr2    = prefetch_lfh2 - 0x2;
	lfh_sr0_si = prefetch_lfh0_si - 0x2;
	lfh_sr1_si = prefetch_lfh1_si - 0x2;
	lfh_sr2_si = prefetch_lfh2_si - 0x2;
	lfh_sr0_sp = prefetch_lfh0_sp - 0x2;
	lfh_sr1_sp = prefetch_lfh1_sp - 0x2;
	lfh_sr2_sp = prefetch_lfh2_sp - 0x2;

	//GS_sr_limit
	//if defined(MERLIN6)
	//{
	//	sr_limit_lf =  0x3f;
	//	sr_limit_lf_si =  0xff;
	//	sr_limit_lf_sp =  0x7;
	//	sr_limit_lf =  0xf;
	//	sr_limit_lf_si =  0x1f;
	//	sr_limit_lf_sp =  0x7;
	//}
	lf_sr0 = (lf_sr0 > 0x3f)? GS_sr_limit : lf_sr0; 
	lf_sr1 = (lf_sr1 > 0x3f)? GS_sr_limit : lf_sr1;
	lf_sr2 = (lf_sr2 > 0x3f)? GS_sr_limit : lf_sr2;
	lf_sr3 = (lf_sr3 > 0x3f)? GS_sr_limit : lf_sr3;
	lf_sr4 = (lf_sr4 > 0x3f)? GS_sr_limit : lf_sr4;
	lf_sr5 = (lf_sr5 > 0x3f)? GS_sr_limit : lf_sr5;
	lf_sr6 = (lf_sr6 > 0x3f)? GS_sr_limit : lf_sr6;
	lf_sr7 = (lf_sr7 > 0x3f)? GS_sr_limit : lf_sr7;
	lf_sr8 = (lf_sr8 > 0x3f)? GS_sr_limit : lf_sr8;
	lf_sr0_si = (lf_sr0_si > 0xff)? GS_sr_limit : lf_sr0_si; 
	lf_sr1_si = (lf_sr1_si > 0xff)? GS_sr_limit : lf_sr1_si;
	lf_sr2_si = (lf_sr2_si > 0xff)? GS_sr_limit : lf_sr2_si;
	lf_sr3_si = (lf_sr3_si > 0xff)? GS_sr_limit : lf_sr3_si;
	lf_sr4_si = (lf_sr4_si > 0xff)? GS_sr_limit : lf_sr4_si;
	lf_sr5_si = (lf_sr5_si > 0xff)? GS_sr_limit : lf_sr5_si;
	lf_sr6_si = (lf_sr6_si > 0xff)? GS_sr_limit : lf_sr6_si;
	lf_sr7_si = (lf_sr7_si > 0xff)? GS_sr_limit : lf_sr7_si;
	lf_sr8_si = (lf_sr8_si > 0xff)? GS_sr_limit : lf_sr8_si;
	lf_sr0_sp = (lf_sr0_sp > 0x7)? GS_sr_limit : lf_sr0_sp; 
	lf_sr1_sp = (lf_sr1_sp > 0x7)? GS_sr_limit : lf_sr1_sp;
	lf_sr2_sp = (lf_sr2_sp > 0x7)? GS_sr_limit : lf_sr2_sp;
	lf_sr3_sp = (lf_sr3_sp > 0x7)? GS_sr_limit : lf_sr3_sp;
	lf_sr4_sp = (lf_sr4_sp > 0x7)? GS_sr_limit : lf_sr4_sp;
	lf_sr5_sp = (lf_sr5_sp > 0x7)? GS_sr_limit : lf_sr5_sp;
	lf_sr6_sp = (lf_sr6_sp > 0x7)? GS_sr_limit : lf_sr6_sp;
	lf_sr7_sp = (lf_sr7_sp > 0x7)? GS_sr_limit : lf_sr7_sp;
	lf_sr8_sp = (lf_sr8_sp > 0x7)? GS_sr_limit : lf_sr8_sp;

	hf_sr0 = (hf_sr0 > 0xf)? GS_sr_limit : hf_sr0;
	hf_sr1 = (hf_sr1 > 0xf)? GS_sr_limit : hf_sr1;
	hf_sr2 = (hf_sr2 > 0xf)? GS_sr_limit : hf_sr2;
	hf_sr0_si = (hf_sr0_si > 0x1f)? GS_sr_limit : hf_sr0_si;
	hf_sr1_si = (hf_sr1_si > 0x1f)? GS_sr_limit : hf_sr1_si;
	hf_sr2_si = (hf_sr2_si > 0x1f)? GS_sr_limit : hf_sr2_si;
	hf_sr0_sp = (hf_sr0_sp > 0x7)? GS_sr_limit : hf_sr0_sp;
	hf_sr1_sp = (hf_sr1_sp > 0x7)? GS_sr_limit : hf_sr1_sp;
	hf_sr2_sp = (hf_sr2_sp > 0x7)? GS_sr_limit : hf_sr2_sp;

	lfh_sr0 = (lfh_sr0 > 0xf)? 0xf : lfh_sr0;
	lfh_sr1 = (lfh_sr1 > 0xf)? 0xf : lfh_sr1;
	lfh_sr2 = (lfh_sr2 > 0xf)? 0xf : lfh_sr2;
	lfh_sr0_si = (lfh_sr0_si > 0x1f)? 0x1f : lfh_sr0_si;
	lfh_sr1_si = (lfh_sr1_si > 0x1f)? 0x1f : lfh_sr1_si;
	lfh_sr2_si = (lfh_sr2_si > 0x1f)? 0x1f : lfh_sr2_si;
	lfh_sr0_sp = (lfh_sr0_sp > 0x7)? 0x7 : lfh_sr0_sp;
	lfh_sr1_sp = (lfh_sr1_sp > 0x7)? 0x7 : lfh_sr1_sp;
	lfh_sr2_sp = (lfh_sr2_sp > 0x7)? 0x7 : lfh_sr2_sp;

	if(GS_normal_hf_prefetch_unit_x2)
	{
	prefetch_hf0 = prefetch_hf0/2;
	prefetch_hf1 = prefetch_hf1/2;
	prefetch_hf2 = prefetch_hf2/2;
	hf_sr0 = prefetch_hf0*2 - 0x2;
	hf_sr1 = prefetch_hf1*2 - 0x2;
	hf_sr2 = prefetch_hf2*2 - 0x2;
	}
	if(GS_normal_hf_sr_unit_x2)
	{
	hf_sr0 = hf_sr0/2;
	hf_sr1 = hf_sr1/2;
	hf_sr2 = hf_sr2/2;
	}

	//======= register
	if(GS_lbmc_full_range ==1){
		lbmc_lbmc_00_reg.lbmc0_lf_sr0  = lf_sr0;
		lbmc_lbmc_00_reg.lbmc0_lf_sr1  = lf_sr1;
		lbmc_lbmc_00_reg.lbmc0_lf_sr2  = lf_sr2;
		lbmc_lbmc_00_reg.lbmc0_lf_sr3  = lf_sr3;
		lbmc_lbmc_00_reg.lbmc0_lf_sr4  = lf_sr4;

		lbmc_lbmc_04_reg.lbmc1_lf_sr0_pos = lf_sr0_si;
		lbmc_lbmc_04_reg.lbmc0_lf_sr5     = lf_sr5;
		lbmc_lbmc_04_reg.lbmc0_lf_sr6     = lf_sr6;
		lbmc_lbmc_04_reg.lbmc0_lf_sr7     = lf_sr7;
		lbmc_lbmc_04_reg.lbmc0_lf_sr8     = lf_sr8;

		lbmc_lbmc_08_reg.lbmc1_lf_sr4_pos = lf_sr4_si;
		lbmc_lbmc_08_reg.lbmc1_lf_sr3_pos = lf_sr3_si;
		lbmc_lbmc_08_reg.lbmc1_lf_sr2_pos = lf_sr2_si;
		lbmc_lbmc_08_reg.lbmc1_lf_sr1_pos = lf_sr1_si;

		lbmc_lbmc_0c_reg.lbmc1_lf_sr8_pos = lf_sr8_si;
		lbmc_lbmc_0c_reg.lbmc1_lf_sr7_pos = lf_sr7_si;
		lbmc_lbmc_0c_reg.lbmc1_lf_sr6_pos = lf_sr6_si;
		lbmc_lbmc_0c_reg.lbmc1_lf_sr5_pos = lf_sr5_si;
	}
	lbmc_lbmc_10_reg.lbmc0_hf_sr0     = hf_sr0; 
	lbmc_lbmc_10_reg.lbmc1_lf_sr8_neg = lf_sr8_sp;
	lbmc_lbmc_10_reg.lbmc1_lf_sr7_neg = lf_sr7_sp;
	lbmc_lbmc_10_reg.lbmc1_lf_sr6_neg = lf_sr6_sp;
	lbmc_lbmc_10_reg.lbmc1_lf_sr5_neg = lf_sr5_sp;
	lbmc_lbmc_10_reg.lbmc1_lf_sr4_neg = lf_sr4_sp;
	lbmc_lbmc_10_reg.lbmc1_lf_sr3_neg = lf_sr3_sp;
	lbmc_lbmc_10_reg.lbmc1_lf_sr2_neg = lf_sr2_sp;
	lbmc_lbmc_10_reg.lbmc1_lf_sr1_neg = lf_sr1_sp;
	lbmc_lbmc_10_reg.lbmc1_lf_sr0_neg = lf_sr0_sp;

	lbmc_lbmc_14_reg.lbmc1_hf_sr2_neg = hf_sr2_sp;
	lbmc_lbmc_14_reg.lbmc1_hf_sr1_neg = hf_sr1_sp;
	lbmc_lbmc_14_reg.lbmc1_hf_sr0_neg = hf_sr0_sp;
	lbmc_lbmc_14_reg.lbmc1_hf_sr2_pos = hf_sr2_si;
	lbmc_lbmc_14_reg.lbmc1_hf_sr1_pos = hf_sr1_si;
	lbmc_lbmc_14_reg.lbmc1_hf_sr0_pos = hf_sr0_si;
	lbmc_lbmc_14_reg.lbmc0_hf_sr2     = hf_sr2;
	lbmc_lbmc_14_reg.lbmc0_hf_sr1     = hf_sr1;

	lbmc_lbmc_20_reg.lbmc_mode     =  lbmc_mode;
	lbmc_lbmc_20_reg.fhd_mod       =  GS_fhd_mode;
	lbmc_lbmc_20_reg.lbmc_v_act    = GS_image_vsize;
	lbmc_lbmc_20_reg.lbmc_pr_lrinv = lbmc_pr_lrinv;
	lbmc_lbmc_20_reg.lbmc_h_act    = GS_image_hsize;

	lbmc_lbmc_24_reg.lbmcpc_mode_sel_p            = GS_pc_sel_pz;                      
	lbmc_lbmc_24_reg.lbmc_hf_nline_option         = GS_lbmc_nline_option;                        
	lbmc_lbmc_24_reg.force_iz_when_mc_phase_eq0   = GS_lbmc_pwr_opt;                        
	lbmc_lbmc_24_reg.lbmc_z_replace_chv           = GS_lbmc_pwr_opt;                         
	lbmc_lbmc_24_reg.lbmcpc_mode                  = pc_mode_turn_on;

	lbmc_lbmc_2c_reg.nor_ilf_idx3_prefetch_num = prefetch_lf3;  
	lbmc_lbmc_2c_reg.nor_ilf_idx2_prefetch_num = prefetch_lf2; 
	lbmc_lbmc_2c_reg.nor_ilf_idx1_prefetch_num = prefetch_lf1; 
	lbmc_lbmc_2c_reg.nor_ilf_idx0_prefetch_num = prefetch_lf0; 

	lbmc_lbmc_30_reg.nor_ilf_idx7_prefetch_num = prefetch_lf7;
	lbmc_lbmc_30_reg.nor_ilf_idx6_prefetch_num = prefetch_lf6;
	lbmc_lbmc_30_reg.nor_ilf_idx5_prefetch_num = prefetch_lf5;
	lbmc_lbmc_30_reg.nor_ilf_idx4_prefetch_num = prefetch_lf4;

	lbmc_lbmc_34_reg.nor_ilf_idx8_prefetch_num = prefetch_lf8;

	lbmc_lbmc_38_reg.nor_plf_idx3_prefetch_num = prefetch_lf5; 
	lbmc_lbmc_38_reg.nor_plf_idx2_prefetch_num = prefetch_lf6; 
	lbmc_lbmc_38_reg.nor_plf_idx1_prefetch_num = prefetch_lf7; 
	lbmc_lbmc_38_reg.nor_plf_idx0_prefetch_num = prefetch_lf8;

	lbmc_lbmc_3c_reg.nor_plf_idx7_prefetch_num = prefetch_lf1;
	lbmc_lbmc_3c_reg.nor_plf_idx6_prefetch_num = prefetch_lf2;
	lbmc_lbmc_3c_reg.nor_plf_idx5_prefetch_num = prefetch_lf3;
	lbmc_lbmc_3c_reg.nor_plf_idx4_prefetch_num = prefetch_lf4;

	lbmc_lbmc_40_reg.nor_plf_idx8_prefetch_num = prefetch_lf0; 

	lbmc_lbmc_44_reg.nor_phf_idx2_prefetch_num = prefetch_hf0;
	lbmc_lbmc_44_reg.nor_phf_idx1_prefetch_num = prefetch_hf1;
	lbmc_lbmc_44_reg.nor_phf_idx0_prefetch_num = prefetch_hf2;
	lbmc_lbmc_44_reg.nor_ihf_idx2_prefetch_num = prefetch_hf2;
	lbmc_lbmc_44_reg.nor_ihf_idx1_prefetch_num = prefetch_hf1;
	lbmc_lbmc_44_reg.nor_ihf_idx0_prefetch_num = prefetch_hf0;

	lbmc_lbmc_48_reg.sing_ilf_idx3_prefetch_num = prefetch_lf3_si;
	lbmc_lbmc_48_reg.sing_ilf_idx2_prefetch_num = prefetch_lf2_si;
	lbmc_lbmc_48_reg.sing_ilf_idx1_prefetch_num = prefetch_lf1_si;
	lbmc_lbmc_48_reg.sing_ilf_idx0_prefetch_num = prefetch_lf0_si;

	lbmc_lbmc_4c_reg.sing_ilf_idx7_prefetch_num = prefetch_lf7_si;
	lbmc_lbmc_4c_reg.sing_ilf_idx6_prefetch_num = prefetch_lf6_si;
	lbmc_lbmc_4c_reg.sing_ilf_idx5_prefetch_num = prefetch_lf5_si;
	lbmc_lbmc_4c_reg.sing_ilf_idx4_prefetch_num = prefetch_lf4_si;

	lbmc_lbmc_50_reg.sing_ilf_idx8_prefetch_num = prefetch_lf8_si;

	lbmc_lbmc_54_reg.sing_plf_idx3_prefetch_num = prefetch_lf3_sp;
	lbmc_lbmc_54_reg.sing_plf_idx2_prefetch_num = prefetch_lf2_sp;
	lbmc_lbmc_54_reg.sing_plf_idx1_prefetch_num = prefetch_lf1_sp;
	lbmc_lbmc_54_reg.sing_plf_idx0_prefetch_num = prefetch_lf0_sp;

	lbmc_lbmc_58_reg.sing_plf_idx7_prefetch_num = prefetch_lf7_sp;
	lbmc_lbmc_58_reg.sing_plf_idx6_prefetch_num = prefetch_lf6_sp;
	lbmc_lbmc_58_reg.sing_plf_idx5_prefetch_num = prefetch_lf5_sp;
	lbmc_lbmc_58_reg.sing_plf_idx4_prefetch_num = prefetch_lf4_sp;

	lbmc_lbmc_5c_reg.sing_plf_idx8_prefetch_num = prefetch_lf8_sp;

	lbmc_lbmc_60_reg.sing_phf_idx2_prefetch_num = prefetch_hf2_sp;
	lbmc_lbmc_60_reg.sing_phf_idx1_prefetch_num = prefetch_hf1_sp;
	lbmc_lbmc_60_reg.sing_phf_idx0_prefetch_num = prefetch_hf0_sp;
	lbmc_lbmc_60_reg.sing_ihf_idx2_prefetch_num = prefetch_hf2_si;
	lbmc_lbmc_60_reg.sing_ihf_idx1_prefetch_num = prefetch_hf1_si;
	lbmc_lbmc_60_reg.sing_ihf_idx0_prefetch_num = prefetch_hf0_si;

	lbmc_lbmc_88_reg.lbmc_c_replace_hv              = GS_lbmc_pwr_opt;                                
	lbmc_lbmc_88_reg.lbmc_c_replace_hv_pwr_opt_post = GS_lbmc_pwr_opt;                    
	lbmc_lbmc_88_reg.lbmc_c_replace_hv_pwr_opt_sram = GS_lbmc_pwr_opt;   

	lbmc_lbmc_d0_reg.lbmc0_lfh_sr0 = lfh_sr0;
	lbmc_lbmc_d0_reg.lbmc0_lfh_sr2 = lfh_sr2;
	lbmc_lbmc_d0_reg.lbmc0_lfh_sr1 = lfh_sr1;

	lbmc_lbmc_d4_reg.lbmc1_lfh_sr2_neg = lfh_sr2_sp;
	lbmc_lbmc_d4_reg.lbmc1_lfh_sr1_neg = lfh_sr1_sp;
	lbmc_lbmc_d4_reg.lbmc1_lfh_sr0_neg = lfh_sr0_sp;
	lbmc_lbmc_d4_reg.lbmc1_lfh_sr2_pos = lfh_sr2_si;
	lbmc_lbmc_d4_reg.lbmc1_lfh_sr1_pos = lfh_sr1_si;
	lbmc_lbmc_d4_reg.lbmc1_lfh_sr0_pos = lfh_sr0_si;

	lbmc_lbmc_d8_reg.nor_plfh_idx2_prefetch_num = prefetch_lfh0;
	lbmc_lbmc_d8_reg.nor_plfh_idx1_prefetch_num = prefetch_lfh1;
	lbmc_lbmc_d8_reg.nor_plfh_idx0_prefetch_num = prefetch_lfh2;
	lbmc_lbmc_d8_reg.nor_ilfh_idx2_prefetch_num = prefetch_lfh2;
	lbmc_lbmc_d8_reg.nor_ilfh_idx1_prefetch_num = prefetch_lfh1;
	lbmc_lbmc_d8_reg.nor_ilfh_idx0_prefetch_num = prefetch_lfh0;

	lbmc_lbmc_dc_reg.sing_plfh_idx2_prefetch_num = prefetch_lfh2_sp;
	lbmc_lbmc_dc_reg.sing_plfh_idx1_prefetch_num = prefetch_lfh1_sp;
	lbmc_lbmc_dc_reg.sing_plfh_idx0_prefetch_num = prefetch_lfh0_sp;
	lbmc_lbmc_dc_reg.sing_ilfh_idx2_prefetch_num = prefetch_lfh2_si;
	lbmc_lbmc_dc_reg.sing_ilfh_idx1_prefetch_num = prefetch_lfh1_si;
	lbmc_lbmc_dc_reg.sing_ilfh_idx0_prefetch_num = prefetch_lfh0_si;

	// register write
	rtd_outl(LBMC_LBMC_00_reg, lbmc_lbmc_00_reg.regValue);
	rtd_outl(LBMC_LBMC_04_reg, lbmc_lbmc_04_reg.regValue);
	rtd_outl(LBMC_LBMC_08_reg, lbmc_lbmc_08_reg.regValue);
	rtd_outl(LBMC_LBMC_0C_reg, lbmc_lbmc_0c_reg.regValue);
	rtd_outl(LBMC_LBMC_10_reg, lbmc_lbmc_10_reg.regValue);
	rtd_outl(LBMC_LBMC_14_reg, lbmc_lbmc_14_reg.regValue);
	rtd_outl(LBMC_LBMC_20_reg, lbmc_lbmc_20_reg.regValue);
	rtd_outl(LBMC_LBMC_24_reg, lbmc_lbmc_24_reg.regValue);
	rtd_outl(LBMC_LBMC_2C_reg, lbmc_lbmc_2c_reg.regValue);
	rtd_outl(LBMC_LBMC_30_reg, lbmc_lbmc_30_reg.regValue);
	rtd_outl(LBMC_LBMC_34_reg, lbmc_lbmc_34_reg.regValue);
	rtd_outl(LBMC_LBMC_38_reg, lbmc_lbmc_38_reg.regValue);
	rtd_outl(LBMC_LBMC_3C_reg, lbmc_lbmc_3c_reg.regValue);
	rtd_outl(LBMC_LBMC_40_reg, lbmc_lbmc_40_reg.regValue);
	rtd_outl(LBMC_LBMC_44_reg, lbmc_lbmc_44_reg.regValue);
	rtd_outl(LBMC_LBMC_48_reg, lbmc_lbmc_48_reg.regValue);
	rtd_outl(LBMC_LBMC_4C_reg, lbmc_lbmc_4c_reg.regValue);
	rtd_outl(LBMC_LBMC_50_reg, lbmc_lbmc_50_reg.regValue);
	rtd_outl(LBMC_LBMC_54_reg, lbmc_lbmc_54_reg.regValue);
	rtd_outl(LBMC_LBMC_58_reg, lbmc_lbmc_58_reg.regValue);
	rtd_outl(LBMC_LBMC_5C_reg, lbmc_lbmc_5c_reg.regValue);
	rtd_outl(LBMC_LBMC_60_reg, lbmc_lbmc_60_reg.regValue);
	rtd_outl(LBMC_LBMC_88_reg, lbmc_lbmc_88_reg.regValue);
	rtd_outl(LBMC_LBMC_D0_reg, lbmc_lbmc_d0_reg.regValue);
	rtd_outl(LBMC_LBMC_D4_reg, lbmc_lbmc_d4_reg.regValue);
	rtd_outl(LBMC_LBMC_D8_reg, lbmc_lbmc_d8_reg.regValue);
	rtd_outl(LBMC_LBMC_DC_reg, lbmc_lbmc_dc_reg.regValue);

}
void MEMC_ChangeSize_LOGO(void)
{
	// parameter declaration
	int logo_dn;
	int logo_dn_single;

	int bringup_logo_pre_v_active;
	int bringup_logo_blk_rim_v1 = 0;
	int bringup_logo_blk_rim_v0 = 0;
	int bringup_logo_blk_rim_h1 = 0;
	int bringup_logo_blk_rim_h0 = 0;
	int bringup_logo_pxl_rim_h0 = 0;
	int bringup_logo_pxl_rim_h1 = 0;
	int bringup_logo_pxl_rim_v0 = 0;
	int bringup_logo_pxl_rim_v1 = 0;

	// register declaration
	kme_logo2_kme_logo2_30_RBUS kme_logo2_kme_logo2_30_reg;
	kme_logo2_kme_logo2_34_RBUS kme_logo2_kme_logo2_34_reg;
	kme_logo0_kme_logo0_00_RBUS kme_logo0_kme_logo0_00_reg;
	kme_logo0_kme_logo0_10_RBUS kme_logo0_kme_logo0_10_reg;
	kme_logo0_kme_logo0_f0_RBUS kme_logo0_kme_logo0_f0_reg;
	kme_logo1_kme_logo1_04_RBUS kme_logo1_kme_logo1_04_reg;
	kme_logo1_kme_logo1_d8_RBUS kme_logo1_kme_logo1_d8_reg;
	kme_logo1_kme_logo1_dc_RBUS kme_logo1_kme_logo1_dc_reg;
	kme_logo1_kme_logo1_e0_RBUS kme_logo1_kme_logo1_e0_reg;
	kme_logo0_kme_logo0_f4_RBUS kme_logo0_kme_logo0_f4_reg;
	kme_logo0_kme_logo0_f8_RBUS kme_logo0_kme_logo0_f8_reg;

	// register read back
	kme_logo2_kme_logo2_30_reg.regValue = rtd_inl(KME_LOGO2_KME_LOGO2_30_reg);
	kme_logo2_kme_logo2_34_reg.regValue = rtd_inl(KME_LOGO2_KME_LOGO2_34_reg);
	kme_logo0_kme_logo0_00_reg.regValue = rtd_inl(KME_LOGO0_KME_LOGO0_00_reg);
	kme_logo0_kme_logo0_10_reg.regValue = rtd_inl(KME_LOGO0_KME_LOGO0_10_reg);
	kme_logo0_kme_logo0_f0_reg.regValue = rtd_inl(KME_LOGO0_KME_LOGO0_F0_reg);
	kme_logo1_kme_logo1_04_reg.regValue = rtd_inl(KME_LOGO1_KME_LOGO1_04_reg);
	kme_logo1_kme_logo1_d8_reg.regValue = rtd_inl(KME_LOGO1_KME_LOGO1_D8_reg);
	kme_logo1_kme_logo1_dc_reg.regValue = rtd_inl(KME_LOGO1_KME_LOGO1_DC_reg);
	kme_logo1_kme_logo1_e0_reg.regValue = rtd_inl(KME_LOGO1_KME_LOGO1_E0_reg);
	kme_logo0_kme_logo0_f4_reg.regValue = rtd_inl(KME_LOGO0_KME_LOGO0_F4_reg);
	kme_logo0_kme_logo0_f8_reg.regValue = rtd_inl(KME_LOGO0_KME_LOGO0_F8_reg);

	// main function body
	if (GS_ipme_dn_ratio == 8) {
		logo_dn_single = 3; 
	} else if (GS_ipme_dn_ratio == 4) {
		logo_dn = 10;
		logo_dn_single = 2; 
	} else {
		logo_dn = 5;  
		logo_dn_single = 1; 
	}

	if (GS_bringup_logo == 1)
	{
		bringup_logo_pre_v_active = GS_image_vsize;   
  
		bringup_logo_blk_rim_h1 = 239;
		bringup_logo_blk_rim_v1 = 134;

		bringup_logo_pxl_rim_h1 = 960;
		bringup_logo_pxl_rim_v1 = 540;
	}

	if(GS_ipme_dn_ratio == 8){
		//TEMP//kme_logo2_kme_logo2_30_reg.plogo_row_num = GS_image_hsize/64;
		kme_logo2_kme_logo2_34_reg.plogo_cursor_y = GS_image_vsize/8;
	}
	else if(GS_ipme_dn_ratio == 4){
		//TEMP//kme_logo2_kme_logo2_30_reg.plogo_row_num = GS_image_hsize/64;
		kme_logo2_kme_logo2_34_reg.plogo_cursor_y = GS_image_vsize/4;
	}
	else{
		//TEMP//kme_logo2_kme_logo2_30_reg.plogo_row_num = GS_image_hsize/32;
		kme_logo2_kme_logo2_34_reg.plogo_cursor_y = GS_image_vsize/2;
	}

#if defined(MERLIN6)
	kme_logo0_kme_logo0_00_reg.km_logo_pscaler_v_active = GS_image_vsize;
#endif
#if defined(MARK2) || defined(MERLIN7) 
	kme_logo0_kme_logo0_10_reg.km_logo_pscaler_v_active = GS_image_vsize;
	kme_logo0_kme_logo0_00_reg.km_logo_h6_ration = 0xf0;
#endif

	kme_logo0_kme_logo0_00_reg.km_logo_h_fetch_mode = logo_dn_single;
	kme_logo0_kme_logo0_00_reg.km_logo_v_fetch_mode = logo_dn_single;

	kme_logo0_kme_logo0_f0_reg.km_logo_blk_v_active = (GS_image_vsize/GS_ipme_dn_ratio)/4;
	kme_logo0_kme_logo0_f0_reg.km_logo_blk_h_active = (GS_image_hsize/GS_ipme_dn_ratio)/4;
	
	if(GS_bringup_logo){
		kme_logo1_kme_logo1_04_reg.km_logo_blk_rim_v1 = bringup_logo_blk_rim_v1;
		kme_logo1_kme_logo1_04_reg.km_logo_blk_rim_v0 = bringup_logo_blk_rim_v0;
		kme_logo1_kme_logo1_04_reg.km_logo_blk_rim_h1 = bringup_logo_blk_rim_h1;
		kme_logo1_kme_logo1_04_reg.km_logo_blk_rim_h0 = bringup_logo_blk_rim_h0;

		kme_logo1_kme_logo1_d8_reg.km_logo_pxlrim_h1 = bringup_logo_pxl_rim_h1;
		kme_logo1_kme_logo1_d8_reg.km_logo_pxlrim_h0 = bringup_logo_pxl_rim_h0;

		kme_logo1_kme_logo1_dc_reg.km_logo_pxlrim_v1 = bringup_logo_pxl_rim_v1;
		kme_logo1_kme_logo1_dc_reg.km_logo_pxlrim_v0 = bringup_logo_pxl_rim_v0;
	}
	
	if (GS_logo_enable){

		kme_logo1_kme_logo1_e0_reg.km_logo_pxl_v_active = (GS_image_vsize/GS_ipme_dn_ratio);
		kme_logo1_kme_logo1_e0_reg.km_logo_pxl_h_active = (GS_image_hsize/GS_ipme_dn_ratio);
		kme_logo1_kme_logo1_e0_reg.cadnewfrm_blkhsty_en_sel =1;
		kme_logo1_kme_logo1_e0_reg.cadnewfrm_blkhsty_en = 1;
		kme_logo1_kme_logo1_e0_reg.cadnewfrm_blklogo_en_sel = 1;
		kme_logo1_kme_logo1_e0_reg.cadnewfrm_blklogo_en = 1;
		kme_logo1_kme_logo1_e0_reg.cadnewfrm_iir_en_sel = 1;
		kme_logo1_kme_logo1_e0_reg.cadnewfrm_iir_en = 1;
		kme_logo1_kme_logo1_e0_reg.cadnewfrm_pxlhsty_en_sel = 1;
		kme_logo1_kme_logo1_e0_reg.cadnewfrm_pxlhsty_en = 1;
		kme_logo1_kme_logo1_e0_reg.cadnewfrm_pxllogo_en_sel = 1;
		kme_logo1_kme_logo1_e0_reg.cadnewfrm_pxllogo_en = 1;
	}
	else{
		kme_logo1_kme_logo1_e0_reg.km_logo_pxl_v_active = (GS_image_vsize/GS_ipme_dn_ratio);
		kme_logo1_kme_logo1_e0_reg.km_logo_pxl_h_active = (GS_image_hsize/GS_ipme_dn_ratio);
		kme_logo1_kme_logo1_e0_reg.cadnewfrm_blkhsty_en_sel =0;
		kme_logo1_kme_logo1_e0_reg.cadnewfrm_blkhsty_en = 0;
		kme_logo1_kme_logo1_e0_reg.cadnewfrm_blklogo_en_sel = 0;
		kme_logo1_kme_logo1_e0_reg.cadnewfrm_blklogo_en = 0;
		kme_logo1_kme_logo1_e0_reg.cadnewfrm_iir_en_sel = 0;
		kme_logo1_kme_logo1_e0_reg.cadnewfrm_iir_en = 0;
		kme_logo1_kme_logo1_e0_reg.cadnewfrm_pxlhsty_en_sel = 0;
		kme_logo1_kme_logo1_e0_reg.cadnewfrm_pxlhsty_en = 0;
		kme_logo1_kme_logo1_e0_reg.cadnewfrm_pxllogo_en_sel = 0;
		kme_logo1_kme_logo1_e0_reg.cadnewfrm_pxllogo_en = 0;
	}

#if defined(MARK2)
	kme_logo0_kme_logo0_f4_reg.km_logo_dummy_regs0 = 2; //[1]:pxl hsty eco_en  
#endif

	kme_logo0_kme_logo0_f8_reg.logo_sram_sel = 0;
	/*if(GS_logo_switch){
        kme_logo0_kme_logo0_f8_reg.logo_sram_sel = 1;
	}
	else{
        kme_logo0_kme_logo0_f8_reg.logo_sram_sel = 0;
	}*/

	// register write
	rtd_outl(KME_LOGO2_KME_LOGO2_30_reg, kme_logo2_kme_logo2_30_reg.regValue);
	rtd_outl(KME_LOGO2_KME_LOGO2_34_reg, kme_logo2_kme_logo2_34_reg.regValue);
	rtd_outl(KME_LOGO0_KME_LOGO0_00_reg, kme_logo0_kme_logo0_00_reg.regValue);
	rtd_outl(KME_LOGO0_KME_LOGO0_10_reg, kme_logo0_kme_logo0_10_reg.regValue);
	rtd_outl(KME_LOGO0_KME_LOGO0_F0_reg, kme_logo0_kme_logo0_f0_reg.regValue);
	rtd_outl(KME_LOGO1_KME_LOGO1_04_reg, kme_logo1_kme_logo1_04_reg.regValue);
	rtd_outl(KME_LOGO1_KME_LOGO1_D8_reg, kme_logo1_kme_logo1_d8_reg.regValue);
	rtd_outl(KME_LOGO1_KME_LOGO1_DC_reg, kme_logo1_kme_logo1_dc_reg.regValue);
	rtd_outl(KME_LOGO1_KME_LOGO1_E0_reg, kme_logo1_kme_logo1_e0_reg.regValue);
	rtd_outl(KME_LOGO0_KME_LOGO0_F4_reg, kme_logo0_kme_logo0_f4_reg.regValue);
	rtd_outl(KME_LOGO0_KME_LOGO0_F8_reg, kme_logo0_kme_logo0_f8_reg.regValue);

}
void MEMC_ChangeSize_MC(void)
{
	// parameter declaration
	//======= pcmode
	//pc_mode
	int pc_mode_turn_on  = (GS_pc_mode)? 0x1 : 0x0;
	int mc_blk_num; //8x8:hszie/8; 16x16:hsize/16
	int mv_row_num; //8x8:vszie/8; 16x16:visize/16
	int mv_rd_intval;//0:8; 1:16
	int mv_row_size;//1:8x8; 2:16x16
	int mc_mv_scale;//[1:0],hor;[3:2] ver; 1:x2,2: x4i ,3:x8
	int mc_blk_size;//[1:0],hor;[3:2] ver; 1:8 ,2: 16
	int mc_scene_change_fb;
	int mc_scene_change_on;
	int mc_scene_change_mode;
	int mc_blk_logo_h_num ;
	int mc_blk_logo_v_num ;
	int mc_pxl_logo_h_num ;
	int mc_pxl_logo_v_num ;

	int mc_blk_logo_h_size ;
	int mc_blk_logo_v_size ;
	int mc_pxl_logo_h_size ;
	int mc_pxl_logo_v_size ;

	int blklogo_mc_de_size ;

	// register declaration
	mc_mc_14_RBUS mc_mc_14_reg;
	mc_mc_18_RBUS mc_mc_18_reg;
	mc_mc_2c_RBUS mc_mc_2c_reg;
	mc_mc_30_RBUS mc_mc_30_reg;
	mc_mc_34_RBUS mc_mc_34_reg;
	mc_mc_38_RBUS mc_mc_38_reg;
	mc_mc_3c_RBUS mc_mc_3c_reg;
	mc_mc_ac_RBUS mc_mc_ac_reg;

	// register read back
	mc_mc_14_reg.regValue = rtd_inl(MC_MC_14_reg);
	mc_mc_18_reg.regValue = rtd_inl(MC_MC_18_reg);
	mc_mc_2c_reg.regValue = rtd_inl(MC_MC_2C_reg);
	mc_mc_30_reg.regValue = rtd_inl(MC_MC_30_reg);
	mc_mc_34_reg.regValue = rtd_inl(MC_MC_34_reg);
	mc_mc_38_reg.regValue = rtd_inl(MC_MC_38_reg);
	mc_mc_3c_reg.regValue = rtd_inl(MC_MC_3C_reg);
	mc_mc_ac_reg.regValue = rtd_inl(MC_MC_AC_reg);

	// main function body
	if((GS_ipme_dn_ratio == 8) &  (GS_me2_2x2 == 1)) //H5  8k MC16x16
	{
		mc_blk_num = GS_image_hsize/16;
		mv_row_num = GS_image_vsize/16;
		mv_rd_intval = 1;
		mv_row_size = 2;
		mc_mv_scale = 0xf;
		mc_blk_size = 0xa;

	}
	else if((GS_ipme_dn_ratio == 4) &  (GS_me2_2x2 == 0)) //H5 4k MC16x16
	{
		mc_blk_num = GS_image_hsize/16;
		mv_row_num = GS_image_vsize/16;
		mv_rd_intval = 1;
		mv_row_size = 2;
		mc_mv_scale = 0xa;
		mc_blk_size = 0xa;
	}
	else if((GS_ipme_dn_ratio == 4) &  (GS_me2_2x2 == 1)) // 4K MC8x8
	{
		mc_blk_num = GS_image_hsize/8;
		mv_row_num = GS_image_vsize/8;
		mv_rd_intval = 0;
		mv_row_size = 1;
		mc_mv_scale = 0xa;
		mc_blk_size = 0x5;
	}
	else //if((GS_ipme_dn_ratio == 2) &  (GS_me2_2x2 == 0)) //2k MC8x8
	{
		mc_blk_num = GS_image_hsize/8;
		mv_row_num = GS_image_vsize/8;
		mv_rd_intval = 0;
		mv_row_size = 1;
		mc_mv_scale = 0x5;
		mc_blk_size = 0x5;
	}

	if(GS_scene_on == 1) 
	{
		mc_scene_change_fb = 1;
		mc_scene_change_on = 0;
		mc_scene_change_mode = 1;
	}
	else 
	{
		mc_scene_change_fb = 0;
		mc_scene_change_on = 0;
		mc_scene_change_mode = 0;
	}

	mc_mc_14_reg.mc_blklogo_prefetch_num = 7;
	mc_mc_14_reg.mc_blklogo_htotal = GS_image_htotal;

	if(GS_logo_enable)
	{
		mc_mc_18_reg.mc_blklogo_ern_sel    = 1; 
		mc_mc_18_reg.mc_blklogo_erosion_en = 1;
	}
	if(GS_ipme_dn_ratio==4)
	{
		mc_mc_18_reg.mc_blklogo_row_num = GS_image_vsize/16;
	}
	else
	{
		mc_mc_18_reg.mc_blklogo_row_num = GS_image_vsize/8;
	}
	mc_mc_18_reg.mc_pc_mode_pwr_otp = pc_mode_turn_on;
	mc_mc_18_reg.mc_blklogo_erosion_timing_mode = 1;


	mc_mc_2c_reg.mc_timing_check_en = 1;
	mc_mc_2c_reg.mcdh_mv_row_num = mv_row_num;
	mc_mc_2c_reg.mcdh_mv_blk_num = mc_blk_num;
	mc_mc_2c_reg.mcdh_mv_rd_intval = mv_rd_intval;
	mc_mc_2c_reg.mcdh_mv_row_size = mv_row_size;

	mc_mc_30_reg.mc_scene_change_fb = mc_scene_change_fb;
	mc_mc_30_reg.mc_scene_change_on = mc_scene_change_on;
	mc_mc_30_reg.mc_scene_change_mode = mc_scene_change_mode;

	mc_mc_34_reg.mc_sram_ls = 0;
	mc_mc_34_reg.mc_v_act = GS_image_vsize;
	mc_mc_34_reg.mc_h_act = GS_image_hsize/2;

	mc_mc_38_reg.mc_mv_scale = mc_mv_scale;
	mc_mc_38_reg.mc_blk_size = mc_blk_size;

	mc_mc_3c_reg.mc_blk_num = mc_blk_num;//row num no use

	//mc_mc_ac_reg.mc_obmc_mode = obmc_mode; // obmc_mode=0


	//====== MC LOGO
	if(GS_ipme_dn_ratio == 8) //8K
	{
		mc_blk_logo_h_num = GS_image_hsize/32 ;
		mc_blk_logo_v_num = GS_image_vsize/32 ;
		mc_pxl_logo_h_num = GS_image_hsize/8 ;
		mc_pxl_logo_v_num = GS_image_vsize/8 ;

		mc_blk_logo_h_size = 2 ;
		mc_blk_logo_v_size = 2 ;
		mc_pxl_logo_h_size = 2 ;
		mc_pxl_logo_v_size = 2 ;  

		blklogo_mc_de_size = 3 ;
	}
	else if(GS_ipme_dn_ratio == 4) //4K
	{
		mc_blk_logo_h_num = GS_image_hsize/16 ;
		mc_blk_logo_v_num = GS_image_vsize/16 ;
		mc_pxl_logo_h_num = GS_image_hsize/4 ;
		mc_pxl_logo_v_num = GS_image_vsize/4 ;

		mc_blk_logo_h_size = 1 ;
		mc_blk_logo_v_size = 1 ;
		mc_pxl_logo_h_size = 1 ;
		mc_pxl_logo_v_size = 1 ;

		blklogo_mc_de_size = 1 ;
	}
	else //2K
	{
		mc_blk_logo_h_num = GS_image_hsize/8 ;
		mc_blk_logo_v_num = GS_image_vsize/8 ;
		mc_pxl_logo_h_num = GS_image_hsize/2 ;
		mc_pxl_logo_v_num = GS_image_vsize/2 ;

		mc_blk_logo_h_size = 0 ;
		mc_blk_logo_v_size = 0 ;
		mc_pxl_logo_h_size = 0 ;
		mc_pxl_logo_v_size = 0 ;  

		blklogo_mc_de_size = 0 ;
	}

	mc_mc_18_reg.mc_blklogo_row_num = mc_blk_logo_v_num ;
	mc_mc_18_reg.mc_blklogo_mc_de_size = blklogo_mc_de_size ;
	mc_mc_14_reg.mc_blklogo_htotal = GS_image_htotal ;
	mc_mc_14_reg.mc_blklogo_hact = mc_pxl_logo_h_num ;

	// register write
	rtd_outl(MC_MC_14_reg, mc_mc_14_reg.regValue);
	rtd_outl(MC_MC_18_reg, mc_mc_18_reg.regValue);
	rtd_outl(MC_MC_2C_reg, mc_mc_2c_reg.regValue);
	rtd_outl(MC_MC_30_reg, mc_mc_30_reg.regValue);
	rtd_outl(MC_MC_34_reg, mc_mc_34_reg.regValue);
	rtd_outl(MC_MC_38_reg, mc_mc_38_reg.regValue);
	rtd_outl(MC_MC_3C_reg, mc_mc_3c_reg.regValue);
	rtd_outl(MC_MC_AC_reg, mc_mc_ac_reg.regValue);

}
void MEMC_ChangeSize_MC2(void)
{
	
	// parameter declaration
	int mc_blk_logo_h_num ;
	int mc_blk_logo_v_num ;
	int mc_pxl_logo_h_num ;
	int mc_pxl_logo_v_num ;

	int mc_blk_logo_h_size ;
	int mc_blk_logo_v_size ;
	int mc_pxl_logo_h_size ;
	int mc_pxl_logo_v_size ;

	int blklogo_mc_de_size ;

	// register declaration
	mc2_mc2_20_RBUS mc2_mc2_20_reg;
	mc2_mc2_24_RBUS mc2_mc2_24_reg;
	mc2_mc2_28_RBUS mc2_mc2_28_reg;
	mc2_mc2_74_RBUS mc2_mc2_74_reg;
	mc2_mc2_7c_RBUS mc2_mc2_7c_reg;
	mc2_mc2_80_RBUS mc2_mc2_80_reg;
	mc2_mc2_a0_RBUS mc2_mc2_a0_reg;
	mc2_mc2_84_RBUS mc2_mc2_84_reg;
	mc2_mc2_a8_RBUS mc2_mc2_a8_reg;
	mc2_mc2_88_RBUS mc2_mc2_88_reg;
	mc2_mc2_8c_RBUS mc2_mc2_8c_reg;
	mc2_mc2_90_RBUS mc2_mc2_90_reg;
	mc2_mc2_94_RBUS mc2_mc2_94_reg;
	mc2_mc2_98_RBUS mc2_mc2_98_reg;
	mc2_mc2_9c_RBUS mc2_mc2_9c_reg;
	mc2_mc2_a4_RBUS mc2_mc2_a4_reg;
	mc_mc_18_RBUS mc_mc_18_reg;

	// register read back
	mc2_mc2_20_reg.regValue = rtd_inl(MC2_MC2_20_reg);
	mc2_mc2_24_reg.regValue = rtd_inl(MC2_MC2_24_reg);
	mc2_mc2_28_reg.regValue = rtd_inl(MC2_MC2_28_reg);
	mc2_mc2_74_reg.regValue = rtd_inl(MC2_MC2_74_reg);
	mc2_mc2_7c_reg.regValue = rtd_inl(MC2_MC2_7C_reg);
	mc2_mc2_80_reg.regValue = rtd_inl(MC2_MC2_80_reg);
	mc2_mc2_a0_reg.regValue = rtd_inl(MC2_MC2_A0_reg);
	mc2_mc2_84_reg.regValue = rtd_inl(MC2_MC2_84_reg);
	mc2_mc2_a8_reg.regValue = rtd_inl(MC2_MC2_A8_reg);
	mc2_mc2_88_reg.regValue = rtd_inl(MC2_MC2_88_reg);
	mc2_mc2_8c_reg.regValue = rtd_inl(MC2_MC2_8C_reg);
	mc2_mc2_90_reg.regValue = rtd_inl(MC2_MC2_90_reg);
	mc2_mc2_94_reg.regValue = rtd_inl(MC2_MC2_94_reg);
	mc2_mc2_98_reg.regValue = rtd_inl(MC2_MC2_98_reg);
	mc2_mc2_9c_reg.regValue = rtd_inl(MC2_MC2_9C_reg);
	mc2_mc2_a4_reg.regValue = rtd_inl(MC2_MC2_A4_reg);
	mc_mc_18_reg.regValue = rtd_inl(MC_MC_18_reg);

	// main function body
	//if(GS_logo_enable){
	//mc2_mc2_20_reg.mc_pxl_logo_pat_en = 0;
	//mc2_mc2_20_reg.mc_logo_self_lpf_en = 1;
	//mc2_mc2_20_reg.mc_logo_merge_type = 15;
	//mc2_mc2_20_reg.mc_logo_en = 3;
	//}
	//if(GS_ipme_dn_ratio == 4){
	//mc2_mc2_20_reg.mc_pxl_logo_v_size = 1;
	//mc2_mc2_20_reg.mc_pxl_logo_h_size = 1;
	//mc2_mc2_20_reg.mc_blk_logo_v_size = 1;
	//mc2_mc2_20_reg.mc_blk_logo_h_size = 1;
	//}

	if(GS_ipme_dn_ratio == 4)
	{
		mc2_mc2_24_reg.mc_blk_logo_v_num = GS_image_vsize/16;
		mc2_mc2_24_reg.mc_blk_logo_h_num = GS_image_hsize/16;
		mc2_mc2_28_reg.mc_pxl_logo_v_num = GS_image_vsize/4;
		mc2_mc2_28_reg.mc_pxl_logo_h_num = GS_image_hsize/4;
	}
	else
	{
		mc2_mc2_24_reg.mc_blk_logo_v_num = GS_image_vsize/8;
		mc2_mc2_24_reg.mc_blk_logo_h_num = GS_image_hsize/8;
		mc2_mc2_28_reg.mc_pxl_logo_v_num = GS_image_vsize/2;
		mc2_mc2_28_reg.mc_pxl_logo_h_num = GS_image_hsize/2;
	}

	mc2_mc2_74_reg.lbmc_logo_y2 = 192;
	mc2_mc2_74_reg.lbmc_logo_y1 = 240;
	mc2_mc2_74_reg.lbmc_logo_diff_th = 5;
	mc2_mc2_74_reg.mc_varfir17_coef8 = 11;

	mc2_mc2_7c_reg.lbmc_logo_dil_coef2 = 8;
	mc2_mc2_7c_reg.lbmc_logo_dil_coef1 = 2;
	mc2_mc2_7c_reg.lbmc_logo_dil_coef0 = 0;
	mc2_mc2_7c_reg.lbmc_logo_continu_bypa = 0;
	mc2_mc2_7c_reg.lbmc_logo_zdiff_byps = 0;
	mc2_mc2_7c_reg.lbmc_logo_color_bypa = 0;
	mc2_mc2_7c_reg.lbmc_logo_avg_th = 3;
#if defined(MERLIN6)
	mc2_mc2_80_reg.lbmc_logo_y_range = 1;
#endif
#if defined(MARK2) || defined(MERLIN7)
	mc2_mc2_a0_reg.lbmc_logo_y_range = 1;
#endif
	mc2_mc2_80_reg.lbmc_logo_rgn_x2 = 3584;
	mc2_mc2_80_reg.lbmc_logo_rgn_x1 = 0;
#if defined(MERLIN6)
	mc2_mc2_84_reg.lbmc_logo_u = 5;
#endif
#if defined(MARK2) || defined(MERLIN7)
	mc2_mc2_a8_reg.lbmc_logo_u = 5;
#endif
	mc2_mc2_84_reg.lbmc_logo_rgn_x4 = 2560;
	mc2_mc2_84_reg.lbmc_logo_rgn_x3 = 960;
#if defined(MERLIN6)
	mc2_mc2_88_reg.lbmc_logo_v = 64;
#endif
#if defined(MARK2) || defined(MERLIN7)
	mc2_mc2_a8_reg.lbmc_logo_v = 64;
#endif
	mc2_mc2_88_reg.lbmc_logo_rgn_y1 = 0;
	mc2_mc2_88_reg.lbmc_logo_rgn_x5 = 1920;
#if defined(MERLIN6)
	mc2_mc2_8c_reg.lbmc_logo_c_range = 40;
#endif
#if defined(MARK2) || defined(MERLIN7)
	mc2_mc2_a0_reg.lbmc_logo_c_range = 40;
#endif
	mc2_mc2_8c_reg.lbmc_logo_rgn_y3 = 320;
	mc2_mc2_8c_reg.lbmc_logo_rgn_y2 = 80;

	mc2_mc2_90_reg.lbmc_logo_coef2 = 0;
#if defined(MERLIN6)
	mc2_mc2_90_reg.lbmc_logo_coef1 = 15;
#endif
#if defined(MARK2) || defined(MERLIN7)
	mc2_mc2_a0_reg.lbmc_logo_coef1 = 15;
#endif
	mc2_mc2_90_reg.lbmc_logo_tran_sw_en = 0;
	mc2_mc2_90_reg.lbmc_logo_rgn_y4 = 240;

	mc2_mc2_94_reg.lbmc_logo_rs4 = 3;
	mc2_mc2_94_reg.lbmc_logo_rs3 = 3;
	mc2_mc2_94_reg.lbmc_logo_rs2 = 3;
	mc2_mc2_94_reg.lbmc_logo_rs1 = 3;
	mc2_mc2_94_reg.lbmc_logo_coef4 = 2;
	mc2_mc2_94_reg.lbmc_logo_coef3 = 4;
	mc2_mc2_94_reg.lbmc_logo_transbit = 4095;

	mc2_mc2_98_reg.lbmc_logo_tran_cond3_en =1;
	mc2_mc2_98_reg.lbmc_logo_tran_cond2_en =1;
	mc2_mc2_98_reg.lbmc_logo_tran_cond1_en =1;

	mc2_mc2_9c_reg.mc_blk_logo_db_rim_top = 0;
	mc2_mc2_9c_reg.mc_blk_logo_db_rim_righ = 240;
	mc2_mc2_9c_reg.mc_blk_logo_db_rim_left = 0;

	mc2_mc2_a0_reg.mc_blk_logo_db_rim_bot = 135;

	mc2_mc2_a4_reg.mc_pix_logo_db_rim_top = 0;
	mc2_mc2_a4_reg.mc_pix_logo_db_rim_righ = 960;
	mc2_mc2_a4_reg.mc_pix_logo_db_rim_left = 0;
	mc2_mc2_a8_reg.mc_pix_logo_db_rim_bot = 120;

	//====== MC LOGO
	if(GS_ipme_dn_ratio == 8) //8K
	{
		mc_blk_logo_h_num = GS_image_hsize/32 + ((GS_image_hsize%32)!=0);
		mc_blk_logo_v_num = GS_image_vsize/32 + ((GS_image_vsize%32)!=0);
		mc_pxl_logo_h_num = GS_image_hsize/8 + ((GS_image_hsize%8)!=0);
		mc_pxl_logo_v_num = GS_image_vsize/8 + ((GS_image_vsize%8)!=0);

		mc_blk_logo_h_size = 2 ;
		mc_blk_logo_v_size = 2 ;
		mc_pxl_logo_h_size = 2 ;
		mc_pxl_logo_v_size = 2 ;  

		blklogo_mc_de_size = 1 ;
	}
	else if(GS_ipme_dn_ratio == 4) //4K
	{
		mc_blk_logo_h_num = GS_image_hsize/16 + ((GS_image_hsize%16)!=0);
		mc_blk_logo_v_num = GS_image_vsize/16 + ((GS_image_vsize%16)!=0);
		mc_pxl_logo_h_num = GS_image_hsize/4 + ((GS_image_hsize%4)!=0);
		mc_pxl_logo_v_num = GS_image_vsize/4 + ((GS_image_vsize%4)!=0);

		mc_blk_logo_h_size = 1 ;
		mc_blk_logo_v_size = 1 ;
		mc_pxl_logo_h_size = 1 ;
		mc_pxl_logo_v_size = 1 ;

		blklogo_mc_de_size = 1 ;
	}
	else //2K
	{
		mc_blk_logo_h_num = GS_image_hsize/8 + ((GS_image_hsize%8)!=0);
		mc_blk_logo_v_num = GS_image_vsize/8 + ((GS_image_vsize%8)!=0);
		mc_pxl_logo_h_num = GS_image_hsize/2 + ((GS_image_hsize%2)!=0);
		mc_pxl_logo_v_num = GS_image_vsize/2 + ((GS_image_vsize%2)!=0);

		mc_blk_logo_h_size = 0 ;
		mc_blk_logo_v_size = 0 ;
		mc_pxl_logo_h_size = 0 ;
		mc_pxl_logo_v_size = 0 ;  

		blklogo_mc_de_size = 0 ;
	}
	mc2_mc2_24_reg.mc_blk_logo_v_num = mc_blk_logo_v_num ;
	mc2_mc2_24_reg.mc_blk_logo_h_num = mc_blk_logo_h_num ;
	mc2_mc2_28_reg.mc_pxl_logo_v_num = mc_pxl_logo_v_num ;
	mc2_mc2_28_reg.mc_pxl_logo_h_num = mc_pxl_logo_h_num ;

	if(GS_logo_enable && mc_mc_18_reg.mc_blklogo_erosion_en)
	{
		mc2_mc2_20_reg.mc_logo_pt_clip_en    = 3; 
	}

	mc2_mc2_20_reg.mc_logo_en = GS_logo_enable*2 + GS_logo_enable;
	
#if defined(MERLIN6)
	mc2_mc2_20_reg.mc_pxl_logo_v_size = mc_pxl_logo_v_size ;
	mc2_mc2_20_reg.mc_pxl_logo_h_size = mc_pxl_logo_h_size ;
#endif
#if defined(MARK2) || defined(MERLIN7)
	mc2_mc2_28_reg.mc_pxl_logo_v_size = mc_pxl_logo_v_size ;
	mc2_mc2_28_reg.mc_pxl_logo_h_size = mc_pxl_logo_h_size ;
#endif
	mc2_mc2_20_reg.mc_blk_logo_v_size = mc_blk_logo_v_size ;
	mc2_mc2_20_reg.mc_blk_logo_h_size = mc_blk_logo_h_size ;

	// register write
	rtd_outl(MC2_MC2_20_reg, mc2_mc2_20_reg.regValue);
	rtd_outl(MC2_MC2_24_reg, mc2_mc2_24_reg.regValue);
	rtd_outl(MC2_MC2_28_reg, mc2_mc2_28_reg.regValue);
	rtd_outl(MC2_MC2_74_reg, mc2_mc2_74_reg.regValue);
	rtd_outl(MC2_MC2_7C_reg, mc2_mc2_7c_reg.regValue);
	rtd_outl(MC2_MC2_80_reg, mc2_mc2_80_reg.regValue);
	rtd_outl(MC2_MC2_A0_reg, mc2_mc2_a0_reg.regValue);
	rtd_outl(MC2_MC2_84_reg, mc2_mc2_84_reg.regValue);
	rtd_outl(MC2_MC2_A8_reg, mc2_mc2_a8_reg.regValue);
	rtd_outl(MC2_MC2_88_reg, mc2_mc2_88_reg.regValue);
	rtd_outl(MC2_MC2_8C_reg, mc2_mc2_8c_reg.regValue);
	rtd_outl(MC2_MC2_90_reg, mc2_mc2_90_reg.regValue);
	rtd_outl(MC2_MC2_94_reg, mc2_mc2_94_reg.regValue);
	rtd_outl(MC2_MC2_98_reg, mc2_mc2_98_reg.regValue);
	rtd_outl(MC2_MC2_9C_reg, mc2_mc2_9c_reg.regValue);
	rtd_outl(MC2_MC2_A4_reg, mc2_mc2_a4_reg.regValue);
	rtd_outl(MC_MC_18_reg, mc_mc_18_reg.regValue);

}
void MEMC_ChangeSize_MCDMA(void)
{
	// parameter declaration
#if defined(MERLIN7) || defined(MERLIN6) || defined(MARK2)
	int width = GS_image_hsize;
#endif
	int height = GS_image_vsize;
	int line_total = 0;
	int lbmc_mode;
	int pqc_bypass = 0, pqc_line_mode = 0, pqc_lf_qp = 0, pqc_lfh_qp = 0, pqc_hf_qp = 0, pqc_lf_db = 0, pqc_lfh_db = 0, pqc_hf_db = 0, pqc_lf_fmt = 0, pqc_lfh_fmt = 0, pqc_hf_fmt = 0, pqc_lf_do_422 = 0, pqc_lfh_do_422 = 0, pqc_hf_do_422 = 0, pqc_lf_flb = 0, pqc_lfh_flb = 0, pqc_hf_flb = 0, pqc_lf_guarantee = 0, pqc_lfh_guarantee = 0, pqc_hf_guarantee = 0;
	bool dat_fmt;
	bool alpha_en;
	bool comp_lf_two_line;
	bool comp_lfh_two_line;
	bool comp_hf_two_line;
	int  comp_line_mode;
	int  comp_data_color;
	int  comp_lf_qp;
	int  comp_lfh_qp;
	int  comp_hf_qp;
	int  comp_lf_data_bit;
	int  comp_lfh_data_bit;
	int  comp_hf_data_bit;
	int  comp_lf_fmt;
	int  comp_lfh_fmt;
	int  comp_hf_fmt;
	int  comp_lf_flb = 0;
	int  comp_lfh_flb = 0;
	int  comp_hf_flb = 0;
	int  first_line_more_bit;
	int  g_ratio;
	int  g_ratio_min;
	int  g_ratio_max;
	int  fisrt_line_more_en;
	int  llb_offset;
	int file_cmd; //TEMP
	int dat_bit; //TEMP
	int mc_dma_bl;
	int dbus_width; 
	/*
	FILE *file, *fp;
	char name[100], nm[100], nm_cmd[20];
	*/
	/* move to main.cpp
	int mode_a            = 0;
	int dat_fmt           = range_rand(0, 1);
	int dat_bit           = range_rand(0, 1);
	int vflip             = 0;
	int pqc_line_mode     = range_rand(0, 1);
	int pqc_lf_qp         = range_rand(0, 1);
	int pqc_lfh_qp        = range_rand(0, 2);
	int pqc_hf_qp         = range_rand(0, 2);
	int pqc_lf_data_bit   = range_rand(0, 1);
	int pqc_lfh_data_bit  = range_rand(0, 1);
	int pqc_hf_data_bit   = range_rand(0, 1);
	int pqc_lf_fmt        = range_rand(0, 1);
	int pqc_lfh_fmt       = pqc_lf_fmt;
	int pqc_hf_fmt        = (pqc_lf_fmt == 0) ? 0: (range_rand(0, 1) == 0) ? 1 : 3;
	int pqc_lf_flb	      = 0;
	int pqc_lfh_flb       = 0;
	int pqc_hf_flb        = 0;
	int pqc_lf_do_422     = range_rand(0, 2);
	int pqc_lfh_do_422    = range_rand(0, 2);
	int pqc_hf_do_422     = range_rand(0, 2);
	int pqc_lf_guarantee  = 0;
	int pqc_lfh_guarantee = 0;
	int pqc_hf_guarantee  = 0;
	int pqc_bypass        = 0;
	*/
	/* runtime error remove section
	if ((fp=fopen("mc_dma_pqc.cmd", "r")) == NULL){
	printf("Failed to open mc_dma_pqc.cmd to config mc dma & pqc!\n");
	file_cmd = 0;
	}
	if(file_cmd == 1) {
	while(fscanf(fp, "%s", nm_cmd) != EOF){
	if(strcmp(nm_cmd, "HACT") == 0){
	fscanf(fp, "%d", &width);
	printf("HACT = %d\n",  width );
	} else if (strcmp(nm_cmd, "VACT") == 0) {
	fscanf(fp, "%d", &height);
	printf("VACT = %d\n",  height );
	} else if (strcmp(nm_cmd, "LBMC_MODE_A") == 0) {
	fscanf(fp, "%d", &mode_a);
	printf("mode_a = %d\n",  mode_a );
	} else if (strcmp(nm_cmd, "MC_DMA_dat_fmt") == 0) {
	fscanf(fp, "%d", &dat_fmt);
	printf("dma_dat_fmt = %d\n",  dat_fmt );
	} else if (strcmp(nm_cmd, "MC_DMA_dat_bit") == 0) {
	fscanf(fp, "%d", &dat_bit);
	printf("dma_dat_bit = %d\n",   dat_bit );
	} else if (strcmp(nm_cmd, "MC_DMA_VFLIP") == 0) {
	fscanf(fp, "%d", &vflip);
	printf("vflip = %d\n",  vflip);
	} else if (strcmp(nm_cmd, "MC_PQC_line_mode") == 0) {
	fscanf(fp, "%d", &pqc_line_mode);
	printf("pqc_line = %d\n",  pqc_line_mode );
	} else if (strcmp(nm_cmd, "MC_PQC_lf_qp") == 0) {
	fscanf(fp, "%d", &pqc_lf_qp);
	printf("pqc_lf_qp = %d\n",  pqc_lf_qp );
	//} else if (strcmp(nm_cmd, "MC_PQC_lfh_qp") == 0) {
	//	fscanf(fp, "%d", &pqc_lfh_qp);
	//	printf("pqc_lfh_qp = %d\n",   pqc_lfh_qp );
	} else if (strcmp(nm_cmd, "MC_PQC_hf_qp") == 0) {
	fscanf(fp, "%d", &pqc_hf_qp);
	printf("pqc_hf_qp = %d\n",   pqc_hf_qp );
	} else if (strcmp(nm_cmd, "MC_PQC_lf_db") == 0) {
	fscanf(fp, "%d", &pqc_lf_data_bit);
	printf("pqc_lf_dat_bit = %d\n",   pqc_lf_data_bit );
	//	} else if (strcmp(nm_cmd, "MC_PQC_lfh_db") == 0) {
	//		fscanf(fp, "%d", &pqc_lfh_data_bit);
	//		printf("pqc_lfh_dat_bit = %d\n",   pqc_lfh_data_bit );
	} else if (strcmp(nm_cmd, "MC_PQC_hf_db") == 0) {
	fscanf(fp, "%d", &pqc_hf_data_bit);
	printf("pqc_hf_db = %d\n",   pqc_hf_data_bit );
	} else if (strcmp(nm_cmd, "MC_PQC_lf_fmt") == 0) {
	fscanf(fp, "%d", &pqc_lf_fmt);
	printf("pqc_lf_fmt = %d\n",   pqc_lf_fmt );
	//	} else if (strcmp(nm_cmd, "MC_PQC_lfh_fmt") == 0) {
	//		fscanf(fp, "%d", &pqc_lfh_fmt);
	//		printf("pqc_lfh_fmt = %d\n",   pqc_lfh_fmt );
	} else if (strcmp(nm_cmd, "MC_PQC_hf_fmt") == 0) {
	fscanf(fp, "%d", &pqc_hf_fmt);
	printf("pqc_hf_fmt = %d\n",   pqc_hf_fmt );
	} else if (strcmp(nm_cmd, "MC_PQC_lf_flb") == 0) {
	fscanf(fp, "%d", &pqc_lf_flb);
	printf("pqc_lf_flb = %d\n",   pqc_lf_flb );
	//	} else if (strcmp(nm_cmd, "MC_PQC_lfh_flb") == 0) {
	//		fscanf(fp, "%d", &pqc_lfh_flb);
	//		printf("pqc_lfh_flb = %d\n",   pqc_lfh_flb );
	} else if (strcmp(nm_cmd, "MC_PQC_hf_flb") == 0) {
	fscanf(fp, "%d", &pqc_hf_flb);
	printf("pqc_hf_flb = %d\n",   pqc_hf_flb);
	} else if (strcmp(nm_cmd, "MC_PQC_lf_do_422") == 0) {
	fscanf(fp, "%d", &pqc_lf_do_422);
	printf("pqc_lf_do_422 = %d\n",   pqc_lf_do_422 );
	//	} else if (strcmp(nm_cmd, "MC_PQC_lfh_do_422") == 0) {
	//		fscanf(fp, "%d", &pqc_lfh_do_422);
	//		printf("pqc_lfh_do_422 = %d\n",   pqc_lfh_do_422 );
	} else if (strcmp(nm_cmd, "MC_PQC_hf_do_422") == 0) {
	fscanf(fp, "%d", &pqc_hf_do_422);
	printf("pqc_hf_do_422 = %d\n",   pqc_hf_do_422 );
	} else if (strcmp(nm_cmd, "MC_PQC_lf_guarantee") == 0) {
	fscanf(fp, "%d", &pqc_lf_guarantee);
	printf("pqc_lf_guarantee = %d\n",   pqc_lf_guarantee );
	//	} else if (strcmp(nm_cmd, "MC_PQC_lfh_guarantee") == 0) {
	//		fscanf(fp, "%d", &pqc_lfh_guarantee);
	//		printf("pqc_lfh_guarantee = %d\n",   pqc_lfh_guarantee );
	} else if (strcmp(nm_cmd, "MC_PQC_hf_guarantee") == 0) {
	fscanf(fp, "%d", &pqc_hf_guarantee);
	printf("pqc_hf_guarantee = %d\n",   pqc_hf_guarantee );
	} else if (strcmp(nm_cmd, "MC_PQC_bypass") == 0) {
	fscanf(fp, "%d", &pqc_bypass);
	printf("pqc_bypass = %d\n",  pqc_bypass );
	} else {
	printf("failed to find : %s\n", nm_cmd);
	}
	}

	fclose(fp);
	} 
	sprintf(name, "mc_dma_rand_set.cmd");
	file = fopen(name, "w");
	*/
	//if(file_cmd == 0) {
	//	if (memc_size_sel == 2) {
	//		width = 3840;
	//		height = 240;
	//	} else if (memc_size_sel == 1) {
	//		width = 1920;
	//		height = 1080;
	//	} else {
	//		width = 1920;
	//		height = 240;
	//	}
	//	if(memc_size_sel< 2)
	//		mode_a = 0;
	//}

	// register declaration
	mc_dma_mc_wdma_ctrl1_RBUS mc_dma_mc_wdma_ctrl1_reg;
	mc_dma_mc_rdma_ctrl1_RBUS mc_dma_mc_rdma_ctrl1_reg;
	pqc_pqdc_mc_pq_cmp_RBUS pqc_pqdc_mc_pq_cmp_reg;
	pqc_pqdc_mc_pq_decmp_RBUS pqc_pqdc_mc_pq_decmp_reg;
	pqc_pqdc_mc_lf_pq_cmp_pair_RBUS pqc_pqdc_mc_lf_pq_cmp_pair_reg;
	pqc_pqdc_mc_lf_pq_cmp_bit_RBUS pqc_pqdc_mc_lf_pq_cmp_bit_reg;
	pqc_pqdc_mc_lf_pq_decmp_pair_RBUS pqc_pqdc_mc_lf_pq_decmp_pair_reg;
	pqc_pqdc_mc_lf_pq_cmp_poor_RBUS pqc_pqdc_mc_lf_pq_cmp_poor_reg;
	pqc_pqdc_mc_lf_pq_cmp_balance_RBUS pqc_pqdc_mc_lf_pq_cmp_balance_reg;
	pqc_pqdc_mc_hf_pq_cmp_bit_RBUS pqc_pqdc_mc_hf_pq_cmp_bit_reg;
	pqc_pqdc_mc_hf_pq_cmp_pair_RBUS pqc_pqdc_mc_hf_pq_cmp_pair_reg;
	pqc_pqdc_mc_hf_pq_decmp_pair_RBUS pqc_pqdc_mc_hf_pq_decmp_pair_reg;
	pqc_pqdc_mc_hf_pq_cmp_poor_RBUS pqc_pqdc_mc_hf_pq_cmp_poor_reg;
	pqc_pqdc_mc_hf_pq_cmp_balance_RBUS pqc_pqdc_mc_hf_pq_cmp_balance_reg;
	pqc_pqdc_mc_lf_pq_cmp_allocate_RBUS pqc_pqdc_mc_lf_pq_cmp_allocate_reg;
	pqc_pqdc_mc_lf_pq_cmp_enable_RBUS pqc_pqdc_mc_lf_pq_cmp_enable_reg;
	pqc_pqdc_mc_hf_pq_cmp_allocate_RBUS pqc_pqdc_mc_hf_pq_cmp_allocate_reg;
	pqc_pqdc_mc_hf_pq_cmp_enable_RBUS pqc_pqdc_mc_hf_pq_cmp_enable_reg;
	mc_dma_mc_wdma_ctrl0_RBUS mc_dma_mc_wdma_ctrl0_reg;
	mc_dma_mc_lf_wdma_num_bl_RBUS mc_dma_mc_lf_wdma_num_bl_reg;
	mc_dma_mc_lf_wdma_lstep_RBUS mc_dma_mc_lf_wdma_lstep_reg;
	mc_dma_mc_lf_dma_wr_ctrl_RBUS mc_dma_mc_lf_dma_wr_ctrl_reg;
	mc_dma_mc_hf_wdma_num_bl_RBUS mc_dma_mc_hf_wdma_num_bl_reg;
	mc_dma_mc_hf_wdma_lstep_RBUS mc_dma_mc_hf_wdma_lstep_reg;
	mc_dma_mc_hf_dma_wr_ctrl_RBUS mc_dma_mc_hf_dma_wr_ctrl_reg;
	mc_dma_mc_rdma_ctrl_RBUS mc_dma_mc_rdma_ctrl_reg;
	mc_dma_mc_lf_i_rdma_num_bl_RBUS mc_dma_mc_lf_i_rdma_num_bl_reg;
	mc_dma_mc_lf_i_dma_rd_ctrl_RBUS mc_dma_mc_lf_i_dma_rd_ctrl_reg;
	mc_dma_mc_hf_i_rdma_num_bl_RBUS mc_dma_mc_hf_i_rdma_num_bl_reg;
	mc_dma_mc_hf_i_dma_rd_ctrl_RBUS mc_dma_mc_hf_i_dma_rd_ctrl_reg;
	mc_dma_mc_lf_p_rdma_num_bl_RBUS mc_dma_mc_lf_p_rdma_num_bl_reg;
	mc_dma_mc_lf_p_dma_rd_ctrl_RBUS mc_dma_mc_lf_p_dma_rd_ctrl_reg;
	mc_dma_mc_hf_p_rdma_num_bl_RBUS mc_dma_mc_hf_p_rdma_num_bl_reg;
	mc_dma_mc_hf_p_dma_rd_ctrl_RBUS mc_dma_mc_hf_p_dma_rd_ctrl_reg;
	mc_dma_mc_lf_wdma_mstart2_RBUS mc_dma_mc_lf_wdma_mstart2_reg;
	mc_dma_mc_lf_wdma_mstart3_RBUS mc_dma_mc_lf_wdma_mstart3_reg;
	mc_dma_mc_lf_wdma_mstart4_RBUS mc_dma_mc_lf_wdma_mstart4_reg;
	mc_dma_mc_lf_wdma_mstart5_RBUS mc_dma_mc_lf_wdma_mstart5_reg;
	mc_dma_mc_lf_wdma_mstart6_RBUS mc_dma_mc_lf_wdma_mstart6_reg;
	mc_dma_mc_lf_wdma_mstart7_RBUS mc_dma_mc_lf_wdma_mstart7_reg;
	mc_dma_mc_lf_wdma_mstart0_RBUS mc_dma_mc_lf_wdma_mstart0_reg;
	mc_dma_mc_lf_wdma_mend0_RBUS mc_dma_mc_lf_wdma_mend0_reg;
	mc_dma_mc_hf_wdma_mstart0_RBUS mc_dma_mc_hf_wdma_mstart0_reg;
	mc_dma_mc_hf_wdma_mend0_RBUS mc_dma_mc_hf_wdma_mend0_reg;
	mc_dma_mc_lf_wdma_mstart1_RBUS mc_dma_mc_lf_wdma_mstart1_reg;
	mc_dma_mc_lf_wdma_mend1_RBUS mc_dma_mc_lf_wdma_mend1_reg;
	mc_dma_mc_hf_wdma_mstart1_RBUS mc_dma_mc_hf_wdma_mstart1_reg;
	mc_dma_mc_hf_wdma_mend1_RBUS mc_dma_mc_hf_wdma_mend1_reg;
	mc_dma_mc_lf_wdma_mend2_RBUS mc_dma_mc_lf_wdma_mend2_reg;
	mc_dma_mc_hf_wdma_mstart2_RBUS mc_dma_mc_hf_wdma_mstart2_reg;
	mc_dma_mc_hf_wdma_mend2_RBUS mc_dma_mc_hf_wdma_mend2_reg;
	mc_dma_mc_lf_wdma_mend3_RBUS mc_dma_mc_lf_wdma_mend3_reg;
	mc_dma_mc_hf_wdma_mstart3_RBUS mc_dma_mc_hf_wdma_mstart3_reg;
	mc_dma_mc_hf_wdma_mend3_RBUS mc_dma_mc_hf_wdma_mend3_reg;
	mc_dma_mc_lf_wdma_mend4_RBUS mc_dma_mc_lf_wdma_mend4_reg;
	mc_dma_mc_hf_wdma_mstart4_RBUS mc_dma_mc_hf_wdma_mstart4_reg;
	mc_dma_mc_hf_wdma_mend4_RBUS mc_dma_mc_hf_wdma_mend4_reg;
	mc_dma_mc_lf_wdma_mend5_RBUS mc_dma_mc_lf_wdma_mend5_reg;
	mc_dma_mc_hf_wdma_mstart5_RBUS mc_dma_mc_hf_wdma_mstart5_reg;
	mc_dma_mc_hf_wdma_mend5_RBUS mc_dma_mc_hf_wdma_mend5_reg;
	mc_dma_mc_lf_wdma_mend6_RBUS mc_dma_mc_lf_wdma_mend6_reg;
	mc_dma_mc_hf_wdma_mstart6_RBUS mc_dma_mc_hf_wdma_mstart6_reg;
	mc_dma_mc_hf_wdma_mend6_RBUS mc_dma_mc_hf_wdma_mend6_reg;
	mc_dma_mc_lf_wdma_mend7_RBUS mc_dma_mc_lf_wdma_mend7_reg;
	mc_dma_mc_hf_wdma_mstart7_RBUS mc_dma_mc_hf_wdma_mstart7_reg;
	mc_dma_mc_hf_wdma_mend7_RBUS mc_dma_mc_hf_wdma_mend7_reg;
	kphase_kphase_10_RBUS kphase_kphase_10_reg;
	mc_dma_mc_lf_dma_wr_rule_check_up_RBUS mc_dma_mc_lf_dma_wr_rule_check_up_reg;
	mc_dma_mc_lf_i_dma_rd_rule_check_up_RBUS mc_dma_mc_lf_i_dma_rd_rule_check_up_reg;
	mc_dma_mc_lf_p_dma_rd_rule_check_up_RBUS mc_dma_mc_lf_p_dma_rd_rule_check_up_reg;
	mc_dma_mc_lf_dma_wr_rule_check_low_RBUS mc_dma_mc_lf_dma_wr_rule_check_low_reg;
	mc_dma_mc_lf_i_dma_rd_rule_check_low_RBUS mc_dma_mc_lf_i_dma_rd_rule_check_low_reg;
	mc_dma_mc_lf_p_dma_rd_rule_check_low_RBUS mc_dma_mc_lf_p_dma_rd_rule_check_low_reg;
	mc_dma_mc_hf_dma_wr_rule_check_up_RBUS mc_dma_mc_hf_dma_wr_rule_check_up_reg;
	mc_dma_mc_hf_i_dma_rd_rule_check_up_RBUS mc_dma_mc_hf_i_dma_rd_rule_check_up_reg;
	mc_dma_mc_hf_p_dma_rd_rule_check_up_RBUS mc_dma_mc_hf_p_dma_rd_rule_check_up_reg;
	mc_dma_mc_hf_dma_wr_rule_check_low_RBUS mc_dma_mc_hf_dma_wr_rule_check_low_reg;
	mc_dma_mc_hf_i_dma_rd_rule_check_low_RBUS mc_dma_mc_hf_i_dma_rd_rule_check_low_reg;
	mc_dma_mc_hf_p_dma_rd_rule_check_low_RBUS mc_dma_mc_hf_p_dma_rd_rule_check_low_reg;

	// register read back
	mc_dma_mc_wdma_ctrl1_reg.regValue = rtd_inl(MC_DMA_MC_WDMA_CTRL1_reg);
	mc_dma_mc_rdma_ctrl1_reg.regValue = rtd_inl(MC_DMA_MC_RDMA_CTRL1_reg);
	pqc_pqdc_mc_pq_cmp_reg.regValue = rtd_inl(PQC_PQDC_MC_PQ_CMP_reg);
	pqc_pqdc_mc_pq_decmp_reg.regValue = rtd_inl(PQC_PQDC_MC_PQ_DECMP_reg);
	pqc_pqdc_mc_lf_pq_cmp_pair_reg.regValue = rtd_inl(PQC_PQDC_MC_LF_PQ_CMP_PAIR_reg);
	pqc_pqdc_mc_lf_pq_cmp_bit_reg.regValue = rtd_inl(PQC_PQDC_MC_LF_PQ_CMP_BIT_reg);
	pqc_pqdc_mc_lf_pq_decmp_pair_reg.regValue = rtd_inl(PQC_PQDC_MC_LF_PQ_DECMP_PAIR_reg);
	pqc_pqdc_mc_lf_pq_cmp_poor_reg.regValue = rtd_inl(PQC_PQDC_MC_LF_PQ_CMP_POOR_reg);
	pqc_pqdc_mc_lf_pq_cmp_balance_reg.regValue = rtd_inl(PQC_PQDC_MC_LF_PQ_CMP_BALANCE_reg);
	pqc_pqdc_mc_hf_pq_cmp_bit_reg.regValue = rtd_inl(PQC_PQDC_MC_HF_PQ_CMP_BIT_reg);
	pqc_pqdc_mc_hf_pq_cmp_pair_reg.regValue = rtd_inl(PQC_PQDC_MC_HF_PQ_CMP_PAIR_reg);
	pqc_pqdc_mc_hf_pq_decmp_pair_reg.regValue = rtd_inl(PQC_PQDC_MC_HF_PQ_DECMP_PAIR_reg);
	pqc_pqdc_mc_hf_pq_cmp_poor_reg.regValue = rtd_inl(PQC_PQDC_MC_HF_PQ_CMP_POOR_reg);
	pqc_pqdc_mc_hf_pq_cmp_balance_reg.regValue = rtd_inl(PQC_PQDC_MC_HF_PQ_CMP_BALANCE_reg);
	pqc_pqdc_mc_lf_pq_cmp_allocate_reg.regValue = rtd_inl(PQC_PQDC_MC_LF_PQ_CMP_ALLOCATE_reg);
	pqc_pqdc_mc_lf_pq_cmp_enable_reg.regValue = rtd_inl(PQC_PQDC_MC_LF_PQ_CMP_ENABLE_reg);
	pqc_pqdc_mc_hf_pq_cmp_allocate_reg.regValue = rtd_inl(PQC_PQDC_MC_HF_PQ_CMP_ALLOCATE_reg);
	pqc_pqdc_mc_hf_pq_cmp_enable_reg.regValue = rtd_inl(PQC_PQDC_MC_HF_PQ_CMP_ENABLE_reg);
	mc_dma_mc_wdma_ctrl0_reg.regValue = rtd_inl(MC_DMA_MC_WDMA_CTRL0_reg);
	mc_dma_mc_lf_wdma_num_bl_reg.regValue = rtd_inl(MC_DMA_MC_LF_WDMA_NUM_BL_reg);
	mc_dma_mc_lf_wdma_lstep_reg.regValue = rtd_inl(MC_DMA_MC_LF_WDMA_LSTEP_reg);
	mc_dma_mc_lf_dma_wr_ctrl_reg.regValue = rtd_inl(MC_DMA_MC_LF_DMA_WR_Ctrl_reg);
	mc_dma_mc_hf_wdma_num_bl_reg.regValue = rtd_inl(MC_DMA_MC_HF_WDMA_NUM_BL_reg);
	mc_dma_mc_hf_wdma_lstep_reg.regValue = rtd_inl(MC_DMA_MC_HF_WDMA_LSTEP_reg);
	mc_dma_mc_hf_dma_wr_ctrl_reg.regValue = rtd_inl(MC_DMA_MC_HF_DMA_WR_Ctrl_reg);
	mc_dma_mc_rdma_ctrl_reg.regValue = rtd_inl(MC_DMA_MC_RDMA_CTRL_reg);
	mc_dma_mc_lf_i_rdma_num_bl_reg.regValue = rtd_inl(MC_DMA_MC_LF_I_RDMA_NUM_BL_reg);
	mc_dma_mc_lf_i_dma_rd_ctrl_reg.regValue = rtd_inl(MC_DMA_MC_LF_I_DMA_RD_Ctrl_reg);
	mc_dma_mc_hf_i_rdma_num_bl_reg.regValue = rtd_inl(MC_DMA_MC_HF_I_RDMA_NUM_BL_reg);
	mc_dma_mc_hf_i_dma_rd_ctrl_reg.regValue = rtd_inl(MC_DMA_MC_HF_I_DMA_RD_Ctrl_reg);
	mc_dma_mc_lf_p_rdma_num_bl_reg.regValue = rtd_inl(MC_DMA_MC_LF_P_RDMA_NUM_BL_reg);
	mc_dma_mc_lf_p_dma_rd_ctrl_reg.regValue = rtd_inl(MC_DMA_MC_LF_P_DMA_RD_Ctrl_reg);
	mc_dma_mc_hf_p_rdma_num_bl_reg.regValue = rtd_inl(MC_DMA_MC_HF_P_RDMA_NUM_BL_reg);
	mc_dma_mc_hf_p_dma_rd_ctrl_reg.regValue = rtd_inl(MC_DMA_MC_HF_P_DMA_RD_Ctrl_reg);
	mc_dma_mc_lf_wdma_mstart2_reg.regValue = rtd_inl(MC_DMA_MC_LF_WDMA_MSTART2_reg);
	mc_dma_mc_lf_wdma_mstart3_reg.regValue = rtd_inl(MC_DMA_MC_LF_WDMA_MSTART3_reg);
	mc_dma_mc_lf_wdma_mstart4_reg.regValue = rtd_inl(MC_DMA_MC_LF_WDMA_MSTART4_reg);
	mc_dma_mc_lf_wdma_mstart5_reg.regValue = rtd_inl(MC_DMA_MC_LF_WDMA_MSTART5_reg);
	mc_dma_mc_lf_wdma_mstart6_reg.regValue = rtd_inl(MC_DMA_MC_LF_WDMA_MSTART6_reg);
	mc_dma_mc_lf_wdma_mstart7_reg.regValue = rtd_inl(MC_DMA_MC_LF_WDMA_MSTART7_reg);
	mc_dma_mc_lf_wdma_mstart0_reg.regValue = rtd_inl(MC_DMA_MC_LF_WDMA_MSTART0_reg);
	mc_dma_mc_lf_wdma_mend0_reg.regValue = rtd_inl(MC_DMA_MC_LF_WDMA_MEND0_reg);
	mc_dma_mc_hf_wdma_mstart0_reg.regValue = rtd_inl(MC_DMA_MC_HF_WDMA_MSTART0_reg);
	mc_dma_mc_hf_wdma_mend0_reg.regValue = rtd_inl(MC_DMA_MC_HF_WDMA_MEND0_reg);
	mc_dma_mc_lf_wdma_mstart1_reg.regValue = rtd_inl(MC_DMA_MC_LF_WDMA_MSTART1_reg);
	mc_dma_mc_lf_wdma_mend1_reg.regValue = rtd_inl(MC_DMA_MC_LF_WDMA_MEND1_reg);
	mc_dma_mc_hf_wdma_mstart1_reg.regValue = rtd_inl(MC_DMA_MC_HF_WDMA_MSTART1_reg);
	mc_dma_mc_hf_wdma_mend1_reg.regValue = rtd_inl(MC_DMA_MC_HF_WDMA_MEND1_reg);
	mc_dma_mc_lf_wdma_mend2_reg.regValue = rtd_inl(MC_DMA_MC_LF_WDMA_MEND2_reg);
	mc_dma_mc_hf_wdma_mstart2_reg.regValue = rtd_inl(MC_DMA_MC_HF_WDMA_MSTART2_reg);
	mc_dma_mc_hf_wdma_mend2_reg.regValue = rtd_inl(MC_DMA_MC_HF_WDMA_MEND2_reg);
	mc_dma_mc_lf_wdma_mend3_reg.regValue = rtd_inl(MC_DMA_MC_LF_WDMA_MEND3_reg);
	mc_dma_mc_hf_wdma_mstart3_reg.regValue = rtd_inl(MC_DMA_MC_HF_WDMA_MSTART3_reg);
	mc_dma_mc_hf_wdma_mend3_reg.regValue = rtd_inl(MC_DMA_MC_HF_WDMA_MEND3_reg);
	mc_dma_mc_lf_wdma_mend4_reg.regValue = rtd_inl(MC_DMA_MC_LF_WDMA_MEND4_reg);
	mc_dma_mc_hf_wdma_mstart4_reg.regValue = rtd_inl(MC_DMA_MC_HF_WDMA_MSTART4_reg);
	mc_dma_mc_hf_wdma_mend4_reg.regValue = rtd_inl(MC_DMA_MC_HF_WDMA_MEND4_reg);
	mc_dma_mc_lf_wdma_mend5_reg.regValue = rtd_inl(MC_DMA_MC_LF_WDMA_MEND5_reg);
	mc_dma_mc_hf_wdma_mstart5_reg.regValue = rtd_inl(MC_DMA_MC_HF_WDMA_MSTART5_reg);
	mc_dma_mc_hf_wdma_mend5_reg.regValue = rtd_inl(MC_DMA_MC_HF_WDMA_MEND5_reg);
	mc_dma_mc_lf_wdma_mend6_reg.regValue = rtd_inl(MC_DMA_MC_LF_WDMA_MEND6_reg);
	mc_dma_mc_hf_wdma_mstart6_reg.regValue = rtd_inl(MC_DMA_MC_HF_WDMA_MSTART6_reg);
	mc_dma_mc_hf_wdma_mend6_reg.regValue = rtd_inl(MC_DMA_MC_HF_WDMA_MEND6_reg);
	mc_dma_mc_lf_wdma_mend7_reg.regValue = rtd_inl(MC_DMA_MC_LF_WDMA_MEND7_reg);
	mc_dma_mc_hf_wdma_mstart7_reg.regValue = rtd_inl(MC_DMA_MC_HF_WDMA_MSTART7_reg);
	mc_dma_mc_hf_wdma_mend7_reg.regValue = rtd_inl(MC_DMA_MC_HF_WDMA_MEND7_reg);
	kphase_kphase_10_reg.regValue = rtd_inl(KPHASE_kphase_10_reg);
	mc_dma_mc_lf_dma_wr_rule_check_up_reg.regValue = rtd_inl(MC_DMA_MC_LF_DMA_WR_Rule_check_up_reg);
	mc_dma_mc_lf_i_dma_rd_rule_check_up_reg.regValue = rtd_inl(MC_DMA_MC_LF_I_DMA_RD_Rule_check_up_reg);
	mc_dma_mc_lf_p_dma_rd_rule_check_up_reg.regValue = rtd_inl(MC_DMA_MC_LF_P_DMA_RD_Rule_check_up_reg);
	mc_dma_mc_lf_dma_wr_rule_check_low_reg.regValue = rtd_inl(MC_DMA_MC_LF_DMA_WR_Rule_check_low_reg);
	mc_dma_mc_lf_i_dma_rd_rule_check_low_reg.regValue = rtd_inl(MC_DMA_MC_LF_I_DMA_RD_Rule_check_low_reg);
	mc_dma_mc_lf_p_dma_rd_rule_check_low_reg.regValue = rtd_inl(MC_DMA_MC_LF_P_DMA_RD_Rule_check_low_reg);
	mc_dma_mc_hf_dma_wr_rule_check_up_reg.regValue = rtd_inl(MC_DMA_MC_HF_DMA_WR_Rule_check_up_reg);
	mc_dma_mc_hf_i_dma_rd_rule_check_up_reg.regValue = rtd_inl(MC_DMA_MC_HF_I_DMA_RD_Rule_check_up_reg);
	mc_dma_mc_hf_p_dma_rd_rule_check_up_reg.regValue = rtd_inl(MC_DMA_MC_HF_P_DMA_RD_Rule_check_up_reg);
	mc_dma_mc_hf_dma_wr_rule_check_low_reg.regValue = rtd_inl(MC_DMA_MC_HF_DMA_WR_Rule_check_low_reg);
	mc_dma_mc_hf_i_dma_rd_rule_check_low_reg.regValue = rtd_inl(MC_DMA_MC_HF_I_DMA_RD_Rule_check_low_reg);
	mc_dma_mc_hf_p_dma_rd_rule_check_low_reg.regValue = rtd_inl(MC_DMA_MC_HF_P_DMA_RD_Rule_check_low_reg);

	// main function body
	mc_dma_bl = 60;
	
	if(GS_ipme_dn_ratio == 8){
		lbmc_mode = 0;
	}
	else if(GS_ipme_dn_ratio == 4) {
		if(GS_me2_2x2 == 1) {
			lbmc_mode = 1;
		}
		else {
			lbmc_mode = 0;
		}
	}
	else {
		lbmc_mode = 1;
	}

	//lf wdma
#if defined(MARK2)
	if (lbmc_mode == 0) {
		mc_dma_mc_lf_wdma_ctrl1_reg.width = width / 2;
		
		mc_dma_mc_lfh_wdma_ctrl1_reg.width = width / 2;
	}
	else {
		mc_dma_mc_lf_wdma_ctrl1_reg.width = width;
		
		mc_dma_mc_lfh_wdma_ctrl1_reg.width = width;
	}
	mc_dma_mc_hf_wdma_ctrl1_reg.width = width;
	
	mc_dma_mc_lf_wdma_ctrl1_reg.height    = height / 2;
	mc_dma_mc_hf_wdma_ctrl1_reg.height = height / 2;
	mc_dma_mc_lfh_wdma_ctrl1_reg.height= height / 2;
	
	mc_dma_mc_lf_rdma_ctrl1_reg.regValue = mc_dma_mc_lf_wdma_ctrl1_reg.regValue;
	mc_dma_mc_hf_rdma_ctrl1_reg.regValue = mc_dma_mc_hf_wdma_ctrl1_reg.regValue;
	mc_dma_mc_lfh_rdma_ctrl1_reg.regValue = mc_dma_mc_lfh_wdma_ctrl1_reg.regValue;
#endif
#if defined(MERLIN6) || defined(MERLIN7)

	mc_dma_mc_wdma_ctrl1_reg.width = width;

	mc_dma_mc_wdma_ctrl1_reg.height    = height / 2;
	mc_dma_mc_rdma_ctrl1_reg.regValue = mc_dma_mc_wdma_ctrl1_reg.regValue;
#endif

	//hf wdma
	//	i_mc_hf_wdma_ctrl1.width  = width;
	//	i_mc_hf_wdma_ctrl1.height = height / 2;

	//	//lfh wdma
	//	i_mc_lfh_wdma_ctrl1.width  = mc_dma_mc_wdma_ctrl1_reg.width;
	//	i_mc_lfh_wdma_ctrl1.height = mc_dma_mc_wdma_ctrl1_reg.height;

	//pqc default
	comp_data_color = 1;
	first_line_more_bit = 32;
	g_ratio = 16;
	g_ratio_min = 13;
	g_ratio_max = 16;
	fisrt_line_more_en = 1;
	llb_offset = 5;
	//pqc default
	//pqc & pqdc

	
	if(GS_pqc_mode == 0){
		//bypass
		pqc_bypass = 1;
		pqc_line_mode = 0;
		pqc_lf_qp    = 0;
		pqc_lfh_qp    = 0;
		pqc_hf_qp     = 0;
		pqc_lf_db    = 1;
		pqc_lfh_db    = 2;
		pqc_hf_db     = 2;
		pqc_lf_fmt   = 0;
		pqc_lfh_fmt   = 0;
		pqc_hf_fmt    = 0;
		pqc_lf_do_422 = 2;
		pqc_lfh_do_422 = 2;
		pqc_hf_do_422  = 2;
		pqc_lf_flb   = 9;
		pqc_lfh_flb   = 10;
		pqc_hf_flb    = 10;
		pqc_lf_guarantee   = 1;
		pqc_lfh_guarantee   = 1;
		pqc_hf_guarantee    = 1;
	} else if (GS_pqc_mode == 1) {
		//lossless
		pqc_bypass = 0;
		pqc_line_mode = 0;
		pqc_lf_qp    = 0;
		pqc_lfh_qp    = 0;
		pqc_hf_qp     = 0;
		pqc_lf_db    = 1;
		pqc_lfh_db    = 2;
		pqc_hf_db     = 2;
		pqc_lf_fmt   = 0;
		pqc_lfh_fmt   = 0;
		pqc_hf_fmt    = 0;
		pqc_lf_do_422 = 2;
		pqc_lfh_do_422 = 2;
		pqc_hf_do_422  = 2;
		pqc_lf_flb   = 30;
		pqc_lfh_flb   = 33;
		pqc_hf_flb    = 33;
		pqc_lf_guarantee   = 1;
		pqc_lfh_guarantee   = 1;
		pqc_hf_guarantee    = 1;
	} else if (GS_pqc_mode == 2) {
		//pc YUV422_frame_18bit
		pqc_bypass = 0;
		pqc_line_mode = 0;
		pqc_lf_qp    = 1;
		pqc_lfh_qp    = 1;
		pqc_hf_qp     = 1;
		pqc_lf_db    = 1;
		pqc_lfh_db    = 2;
		pqc_hf_db     = 2;
		pqc_lf_fmt   = 0;
		pqc_lfh_fmt   = 0;
		pqc_hf_fmt    = 0;
		pqc_lf_do_422 = 1;
		pqc_lfh_do_422 = 1;
		pqc_hf_do_422  = 1;
		pqc_lf_flb   = 20;
		pqc_lfh_flb   = 20;
		pqc_hf_flb    = 16;
		pqc_lf_guarantee   = 0;
		pqc_lfh_guarantee   = 0;
		pqc_hf_guarantee    = 0;
	} else if (GS_pqc_mode == 3) {
		//pc YUV422_line_18bit
		pqc_bypass = 0;
		pqc_line_mode = 1;
		pqc_lf_qp    = 1;
		pqc_lfh_qp    = 1;
		pqc_hf_qp     = 1;
		pqc_lf_db    = 1;
		pqc_lfh_db    = 2;
		pqc_hf_db     = 2;
		pqc_lf_fmt   = 0;
		pqc_lfh_fmt   = 0;
		pqc_hf_fmt    = 0;
		pqc_lf_do_422 = 1;
		pqc_lfh_do_422 = 1;
		pqc_hf_do_422  = 1;
		pqc_lf_flb   = 20;
		pqc_lfh_flb   = 20;
		pqc_hf_flb    = 16;
		pqc_lf_guarantee   = 0;
		pqc_lfh_guarantee   = 0;
		pqc_hf_guarantee    = 0;
		fisrt_line_more_en = 0;
	} else if (GS_pqc_mode == 4) {
		//422 frame 10bit
		pqc_bypass = 0;
		pqc_line_mode = 0;
		pqc_lf_qp    = 1;
		pqc_lfh_qp    = 1;
		pqc_hf_qp     = 1;
		pqc_lf_db    = 1;
		pqc_lfh_db    = 2;
		pqc_hf_db     = 2;
		pqc_lf_fmt   = 1;
		pqc_lfh_fmt   = 1;
		pqc_hf_fmt    = 1;
		pqc_lf_do_422 = 0;
		pqc_lfh_do_422 = 0;
		pqc_hf_do_422  = 0;
		pqc_lf_flb   = 9;
		pqc_lfh_flb   = 10;
		pqc_hf_flb    = 10;
		pqc_lf_guarantee   = 0;
		pqc_lfh_guarantee   = 0;
		pqc_hf_guarantee    = 0;
	} else if (GS_pqc_mode == 5) {
		//422 line 10bit
		pqc_bypass = 0;
		pqc_line_mode = 1;
		pqc_lf_qp    = 1;
		pqc_lfh_qp    = 1;
		pqc_hf_qp     = 1;
		pqc_lf_db    = 1;
		pqc_lfh_db    = 2;
		pqc_hf_db     = 2;
		pqc_lf_fmt   = 1;
		pqc_lfh_fmt   = 1;
		pqc_hf_fmt    = 1;
		pqc_lf_do_422 = 0;
		pqc_lfh_do_422 = 0;
		pqc_hf_do_422  = 0;
		pqc_lf_flb   = 9;
		pqc_lfh_flb   = 10;
		pqc_hf_flb    = 10;
		pqc_lf_guarantee   = 0;
		pqc_lfh_guarantee   = 0;
		pqc_hf_guarantee    = 0;
		fisrt_line_more_en = 0;
	} else if (GS_pqc_mode == 6) {
		//420 frame 8bit
		pqc_bypass = 0;
		pqc_line_mode = 0;
		pqc_lf_qp    = 1;
		pqc_lfh_qp    = 1;
		pqc_hf_qp     = 1;
		pqc_lf_db    = 1;
		pqc_lfh_db    = 2;
		pqc_hf_db     = 2;
		pqc_lf_fmt   = 1;
		pqc_lfh_fmt   = 1;
		pqc_hf_fmt    = 3;
		pqc_lf_do_422 = 0;
		pqc_lfh_do_422 = 0;
		pqc_hf_do_422  = 0;
		pqc_lf_flb   = 9;
		pqc_lfh_flb   = 9;
		pqc_hf_flb    = 6;
		pqc_lf_guarantee   = 0;
		pqc_lfh_guarantee   = 0;
		pqc_hf_guarantee    = 0;
	} else if (GS_pqc_mode == 7) {
		//420 line 8bit
		pqc_bypass = 0;
		pqc_line_mode = 1;
		pqc_lf_qp    = 1;
		pqc_lfh_qp    = 1;
		pqc_hf_qp     = 1;
		pqc_lf_db    = 1;
		pqc_lfh_db    = 2;
		pqc_hf_db     = 2;
		pqc_lf_fmt   = 1;
		pqc_lfh_fmt   = 1;
		pqc_hf_fmt    = 3;
		pqc_lf_do_422 = 0;
		pqc_lfh_do_422 = 0;
		pqc_hf_do_422  = 0;
		pqc_lf_flb   = 9;
		pqc_lfh_flb   = 9;
		pqc_hf_flb    = 6;
		pqc_lf_guarantee   = 0;
		pqc_lfh_guarantee   = 0;
		pqc_hf_guarantee    = 0;
		fisrt_line_more_en = 0;
	} else if (GS_pqc_mode == 8) {
		//420 frame 7bit
		pqc_bypass = 0;
		pqc_line_mode = 0;
		pqc_lf_qp    = 1;
		pqc_lfh_qp    = 1;
		pqc_hf_qp     = 1;
		pqc_lf_db    = 1;
		pqc_lfh_db    = 2;
		pqc_hf_db     = 2;
		pqc_lf_fmt   = 1;
		pqc_lfh_fmt   = 1;
		pqc_hf_fmt    = 3;
		pqc_lf_do_422 = 0;
		pqc_lfh_do_422 = 0;
		pqc_hf_do_422  = 0;
		pqc_lf_flb   = 9;
		pqc_lfh_flb   = 9;
		pqc_hf_flb    = 5;
		pqc_lf_guarantee   = 0;
		pqc_lfh_guarantee   = 0;
		pqc_hf_guarantee    = 0;
	} else if (GS_pqc_mode == 9) {
		//420 line 7bit
		pqc_bypass = 0;
		pqc_line_mode = 1;
		pqc_lf_qp    = 1;
		pqc_lfh_qp    = 1;
		pqc_hf_qp     = 1;
		pqc_lf_db    = 1;
		pqc_lfh_db    = 2;
		pqc_hf_db     = 2;
		pqc_lf_fmt   = 1;
		pqc_lfh_fmt   = 1;
		pqc_hf_fmt    = 3;
		pqc_lf_do_422 = 0;
		pqc_lfh_do_422 = 0;
		pqc_hf_do_422  = 0;
		pqc_lf_flb   = 9;
		pqc_lfh_flb   = 9;
		pqc_hf_flb    = 5;
		pqc_lf_guarantee   = 0;
		pqc_lfh_guarantee   = 0;
		pqc_hf_guarantee    = 0;
		fisrt_line_more_en = 0;
	} else if (GS_pqc_mode == 10) {
		//pc YUV444_frame_18bit
		pqc_bypass = 0;
		pqc_line_mode = 0;
		pqc_lf_qp    = 1;
		pqc_lfh_qp    = 1;
		pqc_hf_qp     = 1;
		pqc_lf_db    = 1;
		pqc_lfh_db    = 2;
		pqc_hf_db     = 2;
		pqc_lf_fmt   = 0;
		pqc_lfh_fmt   = 0;
		pqc_hf_fmt    = 0;
		pqc_lf_do_422 = 2;
		pqc_lfh_do_422 = 2;
		pqc_hf_do_422  = 2;
		pqc_lf_flb   = 20;
		pqc_lfh_flb   = 20;
		pqc_hf_flb    = 16;
		pqc_lf_guarantee   = 0;
		pqc_lfh_guarantee   = 0;
		pqc_hf_guarantee    = 0;
		g_ratio = 12;
		g_ratio_min = 9;
		g_ratio_max = 13;
	} else if (GS_pqc_mode == 11) {
		//pc YUV444 line 18bit
		pqc_bypass = 0;
		pqc_line_mode = 1;
		pqc_lf_qp    = 1;
		pqc_lfh_qp    = 1;
		pqc_hf_qp     = 1;
		pqc_lf_db    = 1;
		pqc_lfh_db    = 2;
		pqc_hf_db     = 2;
		pqc_lf_fmt   = 0;
		pqc_lfh_fmt   = 0;
		pqc_hf_fmt    = 0;
		pqc_lf_do_422 = 2;
		pqc_lfh_do_422 = 2;
		pqc_hf_do_422  = 2;
		pqc_lf_flb   = 20;
		pqc_lfh_flb   = 20;
		pqc_hf_flb    = 16;
		pqc_lf_guarantee   = 0;
		pqc_lfh_guarantee   = 0;
		pqc_hf_guarantee    = 0;
		g_ratio = 12;
		g_ratio_min = 9;
		g_ratio_max = 13;
		fisrt_line_more_en = 0;
	} else if (GS_pqc_mode == 12) {
		//pc RGB444_frame_18bit
		pqc_bypass = 0;
		pqc_line_mode = 0;
		pqc_lf_qp    = 1;
		pqc_lfh_qp    = 1;
		pqc_hf_qp     = 1;
		pqc_lf_db    = 1;
		pqc_lfh_db    = 2;
		pqc_hf_db     = 2;
		pqc_lf_fmt   = 0;
		pqc_lfh_fmt   = 0;
		pqc_hf_fmt    = 0;
		pqc_lf_do_422 = 2;
		pqc_lfh_do_422 = 2;
		pqc_hf_do_422  = 2;
		pqc_lf_flb   = 20;
		pqc_lfh_flb   = 20;
		pqc_hf_flb    = 16;
		pqc_lf_guarantee   = 0;
		pqc_lfh_guarantee   = 0;
		pqc_hf_guarantee    = 0;
		comp_data_color = 0;
		g_ratio = 12;
		g_ratio_min = 9;
		g_ratio_max = 13;
	} else if (GS_pqc_mode == 13) {
		//pc RGB444 line 18bit
		pqc_bypass = 0;
		pqc_line_mode = 1;
		pqc_lf_qp    = 1;
		pqc_lfh_qp    = 1;
		pqc_hf_qp     = 1;
		pqc_lf_db    = 1;
		pqc_lfh_db    = 2;
		pqc_hf_db     = 2;
		pqc_lf_fmt   = 0;
		pqc_lfh_fmt   = 0;
		pqc_hf_fmt    = 0;
		pqc_lf_do_422 = 2;
		pqc_lfh_do_422 = 2;
		pqc_hf_do_422  = 2;
		pqc_lf_flb   = 20;
		pqc_lfh_flb   = 20;
		pqc_hf_flb    = 16;
		pqc_lf_guarantee   = 0;
		pqc_lfh_guarantee   = 0;
		pqc_hf_guarantee    = 0;
		comp_data_color = 0;
		g_ratio = 12;
		g_ratio_min = 9;
		g_ratio_max = 13;
		fisrt_line_more_en = 0;
	} else if (GS_pqc_mode == 14) {
		//pc YUV444_frame_30bit
		pqc_bypass = 0;
		pqc_line_mode = 0;
		pqc_lf_qp    = 1;
		pqc_lfh_qp    = 1;
		pqc_hf_qp     = 1;
		pqc_lf_db    = 1;
		pqc_lfh_db    = 2;
		pqc_hf_db     = 2;
		pqc_lf_fmt   = 0;
		pqc_lfh_fmt   = 0;
		pqc_hf_fmt    = 0;
		pqc_lf_do_422 = 2;
		pqc_lfh_do_422 = 2;
		pqc_hf_do_422  = 2;
		pqc_lf_flb   = 24;
		pqc_lfh_flb   = 27;
		pqc_hf_flb    = 27;
		pqc_lf_guarantee   = 0;
		pqc_lfh_guarantee   = 0;
		pqc_hf_guarantee    = 0;
		g_ratio = 12;
		g_ratio_min = 9;
		g_ratio_max = 13;
	} else if (GS_pqc_mode == 15) {
		//pc YUV444 line 30bit
		pqc_bypass = 0;
		pqc_line_mode = 1;
		pqc_lf_qp    = 1;
		pqc_lfh_qp    = 1;
		pqc_hf_qp     = 1;
		pqc_lf_db    = 1;
		pqc_lfh_db    = 2;
		pqc_hf_db     = 2;
		pqc_lf_fmt   = 0;
		pqc_lfh_fmt   = 0;
		pqc_hf_fmt    = 0;
		pqc_lf_do_422 = 2;
		pqc_lfh_do_422 = 2;
		pqc_hf_do_422  = 2;
		pqc_lf_flb   = 24;
		pqc_lfh_flb   = 27;
		pqc_hf_flb    = 27;
		pqc_lf_guarantee   = 0;
		pqc_lfh_guarantee   = 0;
		pqc_hf_guarantee    = 0;
		g_ratio = 12;
		g_ratio_min = 9;
		g_ratio_max = 13;
		fisrt_line_more_en = 0;
	}
		
	if((GS_pc_mode==1) && (GS_pc_repeat_mode==0)) {
		dat_fmt =1;
	} else if((GS_pc_mode==1) && (GS_pc_repeat_mode==1)) {
		dat_fmt =0;
	} else {
		dat_fmt =0;
	}


	alpha_en = 0;
	comp_lf_two_line = 1;
	comp_lfh_two_line = 1;
	comp_hf_two_line  = 0;
	comp_line_mode = pqc_line_mode;
	comp_lf_qp = pqc_lf_qp;
	comp_lfh_qp = pqc_lfh_qp;
	comp_hf_qp  = pqc_hf_qp;
	comp_lf_data_bit = pqc_lf_db;
	comp_lfh_data_bit = pqc_lfh_db;
	comp_hf_data_bit = pqc_hf_db;
	comp_lf_fmt = pqc_lf_fmt;
	comp_lfh_fmt = pqc_lfh_fmt;
	comp_hf_fmt  = pqc_hf_fmt;

	file_cmd = 1; //TEMP
	dat_bit = 1; //TEMP

	if(file_cmd == 1)
		comp_lf_flb = pqc_lf_flb;
	else if(comp_lf_fmt== 0 && comp_lf_data_bit == 0 && comp_lf_qp == 0)
		comp_lf_flb = range_rand(9, 10);
	else if(comp_lf_fmt== 0 && comp_lf_data_bit == 1)
		comp_lf_flb = range_rand((16 - comp_lf_qp * 2), (18 - comp_lf_qp * 2));
	else if(comp_lf_fmt== 0 && comp_lf_data_bit == 2)
		comp_lf_flb = range_rand((18 - comp_lf_qp * 2), (22 - comp_lf_qp * 2));
	else if(comp_lf_fmt== 1 && comp_lf_data_bit == 0 && comp_lf_qp == 0)
		comp_lf_flb = range_rand(6, 7);
	else if (comp_lf_fmt== 1 && comp_lf_data_bit == 1)
		comp_lf_flb = range_rand((10 - comp_lf_qp * 2), (11 - comp_lf_qp * 2));
	else if (comp_lf_fmt== 1 && comp_lf_data_bit == 2)
		comp_lf_flb = range_rand((12 - comp_lf_qp * 2), (13 - comp_lf_qp * 2));

	if(file_cmd == 1)
		comp_lfh_flb = pqc_lfh_flb;
	else if(comp_lfh_fmt== 0 && comp_lfh_data_bit == 0 && comp_lfh_qp == 0)
		comp_lfh_flb = range_rand(9, 10);
	else if(comp_lfh_fmt== 0 && comp_lfh_data_bit == 1)
		comp_lfh_flb = range_rand((16 - comp_lfh_qp * 2), (18 - comp_lfh_qp * 2));
	else if(comp_lfh_fmt== 0 && comp_lfh_data_bit == 2)
		comp_lfh_flb = range_rand((18 - comp_lfh_qp * 2), (22 - comp_lfh_qp * 2));
	else if(comp_lfh_fmt== 1 && comp_lfh_data_bit == 0 && comp_lfh_qp == 0)
		comp_lfh_flb = range_rand(6, 7);
	else if (comp_lfh_fmt== 1 && comp_lfh_data_bit == 1)
		comp_lfh_flb = range_rand((10 - comp_lfh_qp * 2), (11 - comp_lfh_qp * 2));
	else if (comp_lfh_fmt== 1 && comp_lfh_data_bit == 2)
		comp_lfh_flb = range_rand((12 - comp_lfh_qp * 2), (13 - comp_lfh_qp * 2));


	if(file_cmd == 1)
		comp_hf_flb = pqc_hf_flb;
	else if(comp_hf_fmt== 0 && comp_hf_data_bit == 0 && comp_hf_qp == 0)
		comp_hf_flb = range_rand(9, 10);
	else if(comp_hf_fmt== 0 && comp_hf_data_bit == 1)
		comp_hf_flb = range_rand((16 - comp_hf_qp * 2), (18 - comp_hf_qp * 2));
	else if(comp_hf_fmt== 0 && comp_hf_data_bit == 2)
		comp_hf_flb = range_rand((18 - comp_hf_qp * 2), (22 - comp_hf_qp * 2));
	else if(comp_hf_fmt== 1 && comp_hf_data_bit == 0 && comp_hf_qp == 0)
		comp_hf_flb = range_rand(6, 7);
	else if (comp_hf_fmt== 1 && comp_hf_data_bit == 1)
		comp_hf_flb = range_rand((10 - comp_hf_qp * 2), (11 - comp_hf_qp * 2));
	else if (comp_hf_fmt== 1 && comp_hf_data_bit == 2)
		comp_hf_flb = range_rand((12 - comp_hf_qp * 2), (13 - comp_hf_qp * 2));
	else if (comp_hf_fmt== 3 && comp_hf_data_bit == 0 && comp_hf_qp == 0)
		comp_hf_flb = range_rand(4, 5);
	else if (comp_hf_fmt== 3 && comp_hf_data_bit == 1)
		comp_hf_flb = range_rand((4 - comp_hf_qp * 2), (5 - comp_hf_qp * 2));
	else if (comp_hf_fmt== 3 && comp_hf_data_bit == 2)
		comp_hf_flb = range_rand((5 - comp_hf_qp * 2), (6 - comp_hf_qp * 2));

	comp_lf_flb = comp_lf_flb << 0; //for 2bit float
	comp_lfh_flb = comp_lfh_flb << 0;
	comp_hf_flb  = comp_hf_flb << 0;


	//pqc_pqdc_mc_pq_cmp_reg;//0xb8099200
#if defined(MARK2)
	pqc_pqdc_mc_lfl_pq_cmp_reg.cmp_en = 1 - pqc_bypass;
	pqc_pqdc_mc_lfl_pq_cmp_reg.cmp_width_div32 = mc_dma_mc_lf_wdma_ctrl1_reg.width / 32;
	pqc_pqdc_mc_lfl_pq_cmp_reg.cmp_height      = mc_dma_mc_lf_wdma_ctrl1_reg.height;
	
	if (lbmc_mode == 0) 
		pqc_pqdc_mc_lfh_pq_cmp_reg.cmp_en = 1 - pqc_bypass;
	else
		pqc_pqdc_mc_lfh_pq_cmp_reg.cmp_en = 0;
		
	pqc_pqdc_mc_lfh_pq_cmp_reg.cmp_width_div32 = mc_dma_mc_lfh_wdma_ctrl1_reg.width / 32;
	pqc_pqdc_mc_lfh_pq_cmp_reg.cmp_height      = mc_dma_mc_lfh_wdma_ctrl1_reg.height;
	
	pqc_pqdc_mc_lfl_pq_decmp_reg.decmp_en = 1 - pqc_bypass;
	pqc_pqdc_mc_lfl_pq_decmp_reg.decmp_width_div32 = pqc_pqdc_mc_lfl_pq_cmp_reg.cmp_width_div32;
	pqc_pqdc_mc_lfl_pq_decmp_reg.decmp_height = pqc_pqdc_mc_lfl_pq_cmp_reg.cmp_height;
	
	if (lbmc_mode == 0)
		pqc_pqdc_mc_lfh_pq_decmp_reg.decmp_en = 1 - pqc_bypass;
	else
		pqc_pqdc_mc_lfh_pq_decmp_reg.decmp_en = 0;
		 
	pqc_pqdc_mc_lfh_pq_decmp_reg.decmp_width_div32 = pqc_pqdc_mc_lfh_pq_cmp_reg.cmp_width_div32;
	pqc_pqdc_mc_lfh_pq_decmp_reg.decmp_height = pqc_pqdc_mc_lfh_pq_cmp_reg.cmp_height;
	
	pqc_pqdc_mc_hf_pq_cmp_reg.cmp_en = 1 - pqc_bypass;
	pqc_pqdc_mc_hf_pq_cmp_reg.cmp_width_div32 = mc_dma_mc_hf_wdma_ctrl1_reg.width / 32;
	pqc_pqdc_mc_hf_pq_cmp_reg.cmp_height      = mc_dma_mc_hf_wdma_ctrl1_reg.height;

	pqc_pqdc_mc_hf_pq_decmp_reg.decmp_en = 1 - pqc_bypass;
	pqc_pqdc_mc_hf_pq_decmp_reg.decmp_width_div32 = pqc_pqdc_mc_hf_pq_cmp_reg.cmp_width_div32;
	pqc_pqdc_mc_hf_pq_decmp_reg.decmp_height = pqc_pqdc_mc_hf_pq_cmp_reg.cmp_height;
#endif

#if defined(MERLIN6) || defined(MERLIN7)
	pqc_pqdc_mc_pq_cmp_reg.cmp_en = 1 - pqc_bypass;
	pqc_pqdc_mc_pq_cmp_reg.cmp_width_div32 = mc_dma_mc_wdma_ctrl1_reg.width / 32;
	pqc_pqdc_mc_pq_cmp_reg.cmp_height      = mc_dma_mc_wdma_ctrl1_reg.height;
	
	pqc_pqdc_mc_pq_decmp_reg.decmp_en = 1 - pqc_bypass;
	pqc_pqdc_mc_pq_decmp_reg.decmp_width_div32 = pqc_pqdc_mc_pq_cmp_reg.cmp_width_div32;
	pqc_pqdc_mc_pq_decmp_reg.decmp_height = pqc_pqdc_mc_pq_cmp_reg.cmp_height;
#endif

	
	//i_mc_hf_pq_cmp.cmp_en  = 1 - pqc_bypass;
	//i_mc_hf_pq_cmp.cmp_width_div32 = mc_dma_mc_wdma_ctrl1_reg.width / 32;
	//i_mc_hf_pq_cmp.cmp_height  = height / 2;

	//i_mc_hf_pq_decmp.decmp_en = i_mc_hf_pq_cmp.cmp_en;
	//i_mc_hf_pq_decmp.decmp_width_div32 = i_mc_hf_pq_cmp.cmp_width_div32;
	//i_mc_hf_pq_decmp.decmp_height = i_mc_hf_pq_cmp.cmp_height;
	//ceil((cmp_width_div32*32* line_limit_bit+256)/128)
#if defined(MARK2)
	pqc_pqdc_mc_lfl_pq_cmp_bit_reg.frame_limit_bit         = comp_lf_flb*4;
	//pqc_pqdc_mc_lfl_pq_cmp_bit_reg.line_limit_bit         = (comp_line_mode == 1) ? comp_lf_flb: (comp_lf_flb + llb_offset);
	pqc_pqdc_mc_lfl_pq_cmp_bit_reg.block_limit_bit         = 0x3f;
	pqc_pqdc_mc_lfl_pq_cmp_bit_reg.first_line_more_bit     = first_line_more_bit;

	pqc_pqdc_mc_lfl_pq_cmp_bit_llb_reg.llb_init = 0xc0;
	pqc_pqdc_mc_lfl_pq_cmp_bit_llb_reg.llb_step = 0;
	pqc_pqdc_mc_lfl_pq_cmp_bit_llb_reg.llb_x_blk_sta = 0;

	pqc_pqdc_mc_lfl_pq_cmp_pair_reg.cmp_data_format        = comp_lf_fmt;
	pqc_pqdc_mc_lfl_pq_cmp_pair_reg.cmp_data_bit_width     = comp_lf_data_bit;
	pqc_pqdc_mc_lfl_pq_cmp_pair_reg.cmp_data_color         = comp_data_color;
	pqc_pqdc_mc_lfl_pq_cmp_pair_reg.cmp_line_mode          = comp_line_mode;
	pqc_pqdc_mc_lfl_pq_cmp_pair_reg.two_line_prediction_en = comp_lf_two_line;
	//pqc_pqdc_mc_lf_pq_cmp_pair_reg.cmp_alpha_en           = alpha_en;
	pqc_pqdc_mc_lfl_pq_cmp_pair_reg.cmp_qp_mode            = comp_lf_qp;
		//1p:pqc_pqdc_mc_lfl_pq_cmp_pair_reg.cmp_line_sum_bit       = (pqc_pqdc_mc_lfl_pq_cmp_reg.cmp_width_div32 * 32 * pqc_pqdc_mc_lfl_pq_cmp_bit_reg.line_limit_bit + 256 + 127) / 128;
	pqc_pqdc_mc_lfl_pq_cmp_pair_reg.cmp_line_sum_bit       = (pqc_pqdc_mc_lfl_pq_cmp_reg.cmp_width_div32 * 32 * comp_lf_flb * 4/ 4 + 672 + 127) / 128;
	pqc_pqdc_mc_lfl_pq_decmp_pair_reg.regValue             = pqc_pqdc_mc_lfl_pq_cmp_pair_reg.regValue;
#endif
#if defined(MERLIN6) || defined(MERLIN7)
	pqc_pqdc_mc_lf_pq_cmp_bit_reg.frame_limit_bit         = comp_lf_flb;
	pqc_pqdc_mc_lf_pq_cmp_bit_reg.line_limit_bit         = (comp_line_mode == 1) ? comp_lf_flb: (comp_lf_flb + llb_offset);
	pqc_pqdc_mc_lf_pq_cmp_bit_reg.block_limit_bit         = 0x3f;
	pqc_pqdc_mc_lf_pq_cmp_bit_reg.first_line_more_bit     = first_line_more_bit;


	pqc_pqdc_mc_lf_pq_cmp_pair_reg.cmp_data_format        = comp_lf_fmt;
	pqc_pqdc_mc_lf_pq_cmp_pair_reg.cmp_data_bit_width     = comp_lf_data_bit;
	pqc_pqdc_mc_lf_pq_cmp_pair_reg.cmp_data_color         = comp_data_color;
	pqc_pqdc_mc_lf_pq_cmp_pair_reg.cmp_line_mode          = comp_line_mode;
	pqc_pqdc_mc_lf_pq_cmp_pair_reg.two_line_prediction_en = comp_lf_two_line;
	//pqc_pqdc_mc_lf_pq_cmp_pair_reg.cmp_alpha_en           = alpha_en;
	pqc_pqdc_mc_lf_pq_cmp_pair_reg.cmp_qp_mode            = comp_lf_qp;
	//1p:pqc_pqdc_mc_lfl_pq_cmp_pair_reg.cmp_line_sum_bit       = (pqc_pqdc_mc_lfl_pq_cmp_reg.cmp_width_div32 * 32 * pqc_pqdc_mc_lfl_pq_cmp_bit_reg.line_limit_bit + 256 + 127) / 128;
	pqc_pqdc_mc_lf_pq_cmp_pair_reg.cmp_line_sum_bit       = (pqc_pqdc_mc_pq_cmp_reg.cmp_width_div32 * 32 * comp_lf_flb + 256 + 127) / 128;
	pqc_pqdc_mc_lf_pq_decmp_pair_reg.regValue             = pqc_pqdc_mc_lf_pq_cmp_pair_reg.regValue;
	pqc_pqdc_mc_lf_pq_cmp_poor_reg.poor_limit_th_qp4			= 0xc;
	pqc_pqdc_mc_lf_pq_cmp_poor_reg.poor_limit_th_qp3			= 0x6;
	pqc_pqdc_mc_lf_pq_cmp_poor_reg.poor_limit_th_qp2			= 0x3;
	pqc_pqdc_mc_lf_pq_cmp_poor_reg.poor_limit_th_qp1			= 0x1;
	pqc_pqdc_mc_lf_pq_cmp_balance_reg.balance_rb_ov_th		= 0x3;
	pqc_pqdc_mc_lf_pq_cmp_balance_reg.balance_rb_ud_th		= 0x0;
	pqc_pqdc_mc_lf_pq_cmp_balance_reg.balance_rb_give			= 0x3;
	pqc_pqdc_mc_lf_pq_cmp_balance_reg.balance_g_ov_th			= 0x3;
	pqc_pqdc_mc_lf_pq_cmp_balance_reg.balance_g_ud_th			= 0x0;
	pqc_pqdc_mc_lf_pq_cmp_balance_reg.balance_g_give			= 0x3;
#endif
#if (defined(MARK2)||defined(MERLIN7)||defined(MERLIN6)||defined(NEED_FIX_IT))
	pqc_pqdc_mc_hf_pq_cmp_bit_reg.frame_limit_bit         = comp_hf_flb;
	pqc_pqdc_mc_hf_pq_cmp_bit_reg.line_limit_bit         = (comp_line_mode == 1) ? comp_hf_flb: comp_hf_flb + llb_offset;
	pqc_pqdc_mc_hf_pq_cmp_bit_reg.block_limit_bit         = 0x3f;
	pqc_pqdc_mc_hf_pq_cmp_bit_reg.first_line_more_bit     = first_line_more_bit;


	pqc_pqdc_mc_hf_pq_cmp_pair_reg.cmp_data_format         = comp_hf_fmt;
	pqc_pqdc_mc_hf_pq_cmp_pair_reg.cmp_data_bit_width      = comp_hf_data_bit;
	pqc_pqdc_mc_hf_pq_cmp_pair_reg.cmp_data_color          = comp_data_color;
	pqc_pqdc_mc_hf_pq_cmp_pair_reg.cmp_line_mode           = comp_line_mode;

	//pqc_pqdc_mc_hf_pq_cmp_pair_reg.two_line_prediction_en  = comp_hf_two_line;

	//pqc_pqdc_mc_hf_pq_cmp_pair_reg.cmp_alpha_en            = alpha_en;
	pqc_pqdc_mc_hf_pq_cmp_pair_reg.cmp_qp_mode             = comp_hf_qp;
#endif

#if defined(MARK2)
	pqc_pqdc_mc_hf_pq_cmp_pair_reg.cmp_line_sum_bit        = (pqc_pqdc_mc_hf_pq_cmp_reg.cmp_width_div32 * 32 * comp_hf_flb + 256 + 127) / 128;
#endif
#if defined(MERLIN6) || defined(MERLIN7)
	pqc_pqdc_mc_hf_pq_cmp_pair_reg.cmp_line_sum_bit        = (pqc_pqdc_mc_pq_cmp_reg.cmp_width_div32 * 32 * comp_hf_flb + 256 + 127) / 128;
#endif
#if (defined(MARK2)||defined(MERLIN7)||defined(MERLIN6)||defined(NEED_FIX_IT))
	pqc_pqdc_mc_hf_pq_decmp_pair_reg.regValue              = pqc_pqdc_mc_hf_pq_cmp_pair_reg.regValue;
	pqc_pqdc_mc_hf_pq_cmp_poor_reg.poor_limit_th_qp4			= 0xc;
	pqc_pqdc_mc_hf_pq_cmp_poor_reg.poor_limit_th_qp3			= 0x6;
	pqc_pqdc_mc_hf_pq_cmp_poor_reg.poor_limit_th_qp2			= 0x3;
	pqc_pqdc_mc_hf_pq_cmp_poor_reg.poor_limit_th_qp1			= 0x1;
	pqc_pqdc_mc_hf_pq_cmp_balance_reg.balance_rb_ov_th		= 0x3;
	pqc_pqdc_mc_hf_pq_cmp_balance_reg.balance_rb_ud_th		= 0x0;
	pqc_pqdc_mc_hf_pq_cmp_balance_reg.balance_rb_give			= 0x3;
	pqc_pqdc_mc_hf_pq_cmp_balance_reg.balance_g_ov_th			= 0x3;
	pqc_pqdc_mc_hf_pq_cmp_balance_reg.balance_g_ud_th			= 0x0;
	pqc_pqdc_mc_hf_pq_cmp_balance_reg.balance_g_give			= 0x3;
#endif

#if defined(MARK2)
	pqc_pqdc_mc_lfh_pq_cmp_bit_reg.frame_limit_bit         = comp_lfh_flb;
	pqc_pqdc_mc_lfh_pq_cmp_bit_reg.line_limit_bit         = (comp_line_mode == 1) ? comp_lfh_flb: (comp_lfh_flb + llb_offset);
	pqc_pqdc_mc_lfh_pq_cmp_bit_reg.block_limit_bit         = 0x3f;
	pqc_pqdc_mc_lfh_pq_cmp_bit_reg.first_line_more_bit     = first_line_more_bit;


	pqc_pqdc_mc_lfh_pq_cmp_pair_reg.cmp_data_format        = comp_lfh_fmt;
	pqc_pqdc_mc_lfh_pq_cmp_pair_reg.cmp_data_bit_width     = comp_lfh_data_bit;
	pqc_pqdc_mc_lfh_pq_cmp_pair_reg.cmp_data_color         = comp_data_color;
	pqc_pqdc_mc_lfh_pq_cmp_pair_reg.cmp_line_mode          = comp_line_mode;
	//pqc_pqdc_mc_lfh_pq_cmp_pair_reg.two_line_prediction_en = comp_lfh_two_line;
	//pqc_pqdc_mc_lf_pq_cmp_pair_reg.cmp_alpha_en           = alpha_en;
	pqc_pqdc_mc_lfh_pq_cmp_pair_reg.cmp_qp_mode            = comp_lfh_qp;
	pqc_pqdc_mc_lfh_pq_cmp_pair_reg.cmp_line_sum_bit       = (pqc_pqdc_mc_lfh_pq_cmp_reg.cmp_width_div32 * 32 * comp_lfh_flb + 256 + 127) / 128;

	pqc_pqdc_mc_lfh_pq_decmp_pair_reg.regValue             = pqc_pqdc_mc_lfh_pq_cmp_pair_reg.regValue;
#endif 

#if defined(MARK2)
	pqc_pqdc_mc_lfl_pq_cmp_allocate_reg.dynamic_allocate_ratio_max	= g_ratio_max;
	pqc_pqdc_mc_lfl_pq_cmp_allocate_reg.dynamic_allocate_ratio_min	= g_ratio_min;
	pqc_pqdc_mc_lfl_pq_cmp_enable_reg.g_ratio              = g_ratio;
	pqc_pqdc_mc_lfl_pq_cmp_enable_reg.add_last2blk_over_curve_bit = 1;
	pqc_pqdc_mc_lfl_pq_cmp_enable_reg.variation_maxmin_en  = 0;
	pqc_pqdc_mc_lfl_pq_cmp_enable_reg.dynamic_sum_line_rst = 1;
	pqc_pqdc_mc_lfl_pq_cmp_enable_reg.blk0_add_en          = 1;
	pqc_pqdc_mc_lfl_pq_cmp_enable_reg.dynamic_allocate_ratio_en   = 1;
	pqc_pqdc_mc_lfl_pq_cmp_enable_reg.poor_limit_min_qp_en = 1;
	pqc_pqdc_mc_lfl_pq_cmp_enable_reg.balance_en           = 1;
	//pqc_pqdc_mc_lfl_pq_cmp_enable_reg.fisrt_line_more_en   = fisrt_line_more_en;
	pqc_pqdc_mc_lfl_pq_cmp_enable_reg.blk0_add_half_en     = 1;
	pqc_pqdc_mc_lfl_pq_cmp_enable_reg.do_422_mode          = pqc_lf_do_422;   //0, 1, 2
	pqc_pqdc_mc_lfl_pq_cmp_enable_reg.first_predict_nc_en  = 1;
	pqc_pqdc_mc_lfl_pq_cmp_enable_reg.not_enough_bit_do_422_en     = 1;
	pqc_pqdc_mc_lfl_pq_cmp_enable_reg.not_rich_do_422_en   = 1;
	pqc_pqdc_mc_lfl_pq_cmp_enable_reg.rb_lossy_do_422_en   = 1;
	pqc_pqdc_mc_lfl_pq_cmp_enable_reg.guarantee_max_qp_en  = pqc_lf_guarantee;
	
	pqc_pqdc_mc_lfh_pq_cmp_allocate_reg.dynamic_allocate_ratio_max	= g_ratio_max;
	pqc_pqdc_mc_lfh_pq_cmp_allocate_reg.dynamic_allocate_ratio_min	= g_ratio_min;
	pqc_pqdc_mc_lfh_pq_cmp_enable_reg.g_ratio              = g_ratio;
	pqc_pqdc_mc_lfh_pq_cmp_enable_reg.add_last2blk_over_curve_bit = 1;
	pqc_pqdc_mc_lfh_pq_cmp_enable_reg.variation_maxmin_en  = 0;
	pqc_pqdc_mc_lfh_pq_cmp_enable_reg.dynamic_sum_line_rst = 1;
	pqc_pqdc_mc_lfh_pq_cmp_enable_reg.blk0_add_en          = 1;
	pqc_pqdc_mc_lfh_pq_cmp_enable_reg.dynamic_allocate_ratio_en   = 1;
	pqc_pqdc_mc_lfh_pq_cmp_enable_reg.poor_limit_min_qp_en = 1;
	pqc_pqdc_mc_lfh_pq_cmp_enable_reg.balance_en           = 1;
	pqc_pqdc_mc_lfh_pq_cmp_enable_reg.fisrt_line_more_en   = fisrt_line_more_en;
	pqc_pqdc_mc_lfh_pq_cmp_enable_reg.blk0_add_half_en     = 1;
	pqc_pqdc_mc_lfh_pq_cmp_enable_reg.do_422_mode          = pqc_lfh_do_422;   //0, 1, 2
	pqc_pqdc_mc_lfh_pq_cmp_enable_reg.first_predict_nc_en  = 1;
	pqc_pqdc_mc_lfh_pq_cmp_enable_reg.not_enough_bit_do_422_en     = 1;
	pqc_pqdc_mc_lfh_pq_cmp_enable_reg.not_rich_do_422_en   = 1;
	pqc_pqdc_mc_lfh_pq_cmp_enable_reg.rb_lossy_do_422_en   = 1;
	pqc_pqdc_mc_lfh_pq_cmp_enable_reg.guarantee_max_qp_en  = pqc_lfh_guarantee;
#endif
#if defined(MERLIN6) || defined(MERLIN7)
	pqc_pqdc_mc_lf_pq_cmp_allocate_reg.dynamic_allocate_ratio_max	= g_ratio_max;
	pqc_pqdc_mc_lf_pq_cmp_allocate_reg.dynamic_allocate_ratio_min	= g_ratio_min;
	pqc_pqdc_mc_lf_pq_cmp_enable_reg.g_ratio              = g_ratio;
	pqc_pqdc_mc_lf_pq_cmp_enable_reg.add_last2blk_over_curve_bit = 1;
	pqc_pqdc_mc_lf_pq_cmp_enable_reg.variation_maxmin_en  = 0;
	pqc_pqdc_mc_lf_pq_cmp_enable_reg.dynamic_sum_line_rst = 1;
	pqc_pqdc_mc_lf_pq_cmp_enable_reg.blk0_add_en          = 1;
	pqc_pqdc_mc_lf_pq_cmp_enable_reg.dynamic_allocate_ratio_en   = 1;
	pqc_pqdc_mc_lf_pq_cmp_enable_reg.poor_limit_min_qp_en = 1;
	pqc_pqdc_mc_lf_pq_cmp_enable_reg.balance_en           = 1;
	pqc_pqdc_mc_lf_pq_cmp_enable_reg.fisrt_line_more_en   = fisrt_line_more_en;
	pqc_pqdc_mc_lf_pq_cmp_enable_reg.blk0_add_half_en     = 1;
	pqc_pqdc_mc_lf_pq_cmp_enable_reg.do_422_mode          = pqc_lf_do_422;   //0, 1, 2
	pqc_pqdc_mc_lf_pq_cmp_enable_reg.first_predict_nc_en  = 1;
	pqc_pqdc_mc_lf_pq_cmp_enable_reg.not_enough_bit_do_422_en     = 1;
	pqc_pqdc_mc_lf_pq_cmp_enable_reg.not_rich_do_422_en   = 1;
	pqc_pqdc_mc_lf_pq_cmp_enable_reg.rb_lossy_do_422_en   = 1;
	pqc_pqdc_mc_lf_pq_cmp_enable_reg.guarantee_max_qp_en  = pqc_lf_guarantee;
#endif
#if (defined(MARK2)||defined(MERLIN7)||defined(MERLIN6)||defined(NEED_FIX_IT))
	pqc_pqdc_mc_hf_pq_cmp_allocate_reg.dynamic_allocate_ratio_max	= g_ratio_max;
	pqc_pqdc_mc_hf_pq_cmp_allocate_reg.dynamic_allocate_ratio_min	= g_ratio_min;
	pqc_pqdc_mc_hf_pq_cmp_enable_reg.g_ratio              = g_ratio;
	pqc_pqdc_mc_hf_pq_cmp_enable_reg.add_last2blk_over_curve_bit = 1;
	pqc_pqdc_mc_hf_pq_cmp_enable_reg.variation_maxmin_en  = 0;
	pqc_pqdc_mc_hf_pq_cmp_enable_reg.dynamic_sum_line_rst = 1;
	pqc_pqdc_mc_hf_pq_cmp_enable_reg.blk0_add_en          = 1;
	pqc_pqdc_mc_hf_pq_cmp_enable_reg.dynamic_allocate_ratio_en   = 1;
	pqc_pqdc_mc_hf_pq_cmp_enable_reg.poor_limit_min_qp_en = 1;
	pqc_pqdc_mc_hf_pq_cmp_enable_reg.balance_en           = 1;
	pqc_pqdc_mc_hf_pq_cmp_enable_reg.fisrt_line_more_en   = fisrt_line_more_en;
	pqc_pqdc_mc_hf_pq_cmp_enable_reg.blk0_add_half_en     = 1;
	pqc_pqdc_mc_hf_pq_cmp_enable_reg.do_422_mode          = pqc_hf_do_422;   //0, 1, 2
	pqc_pqdc_mc_hf_pq_cmp_enable_reg.first_predict_nc_en  = 1;
	pqc_pqdc_mc_hf_pq_cmp_enable_reg.not_enough_bit_do_422_en     = 1;
	pqc_pqdc_mc_hf_pq_cmp_enable_reg.not_rich_do_422_en   = 1;
	pqc_pqdc_mc_hf_pq_cmp_enable_reg.rb_lossy_do_422_en   = 1;
	pqc_pqdc_mc_hf_pq_cmp_enable_reg.guarantee_max_qp_en  = pqc_hf_guarantee;
#endif



	mc_dma_mc_wdma_ctrl0_reg.kmc_disable_frame_hold = 0;
	mc_dma_mc_wdma_ctrl0_reg.dat_fmt                = dat_fmt;//range_rand(0, 1);
	mc_dma_mc_wdma_ctrl0_reg.dat_bit                = dat_bit;//range_rand(0, 1);

#if defined(H5C2)
	dbus_width = 256;
#else
	dbus_width = 128; 
#endif 

#if defined(MARK2) 
	//LFL 2p mode
	line_total = (int)((mc_dma_mc_wdma_ctrl0_reg.dat_fmt + 2) * ((mc_dma_mc_wdma_ctrl0_reg.dat_bit == 0) ? 8 : 10) * mc_dma_mc_lf_wdma_ctrl1_reg.width  + dbus_width - 1) / dbus_width;
	if(pqc_bypass == 0 /*&& comp_line_mode == 1*/)
		line_total = (pqc_pqdc_mc_lfl_pq_cmp_pair_reg.cmp_line_sum_bit );
#endif
#if defined(MERLIN6) || defined(MERLIN7)
	//LFL 2p mode
	line_total = (int)((mc_dma_mc_wdma_ctrl0_reg.dat_fmt + 2) * ((mc_dma_mc_wdma_ctrl0_reg.dat_bit == 0) ? 8 : 10) * mc_dma_mc_wdma_ctrl1_reg.width  + dbus_width - 1) / dbus_width;
	if(pqc_bypass == 0 /*&& comp_line_mode == 1*/)
		line_total = (pqc_pqdc_mc_lf_pq_cmp_pair_reg.cmp_line_sum_bit );
#endif
#if defined(H5C2)
	//LFL 2p mode
	line_total = (int)((mc_dma_mc_wdma_ctrl0_reg.dat_fmt + 2) * ((mc_dma_mc_wdma_ctrl0_reg.dat_bit == 0) ? 8 : 10) * mc_dma_mc_lf_wdma_ctrl1_reg.width  + dbus_width - 1) / dbus_width * 2;
#if (defined(MARK2)||defined(MERLIN7)||defined(MERLIN6)||defined(NEED_FIX_IT))
	if(pqc_bypass == 0 /*&& comp_line_mode == 1*/)
		line_total = (mc_pqc4p_pqdc4p_mc_lf_pq_cmp_pair_reg.cmp_line_sum_bit );
#endif
#endif
	mc_dma_mc_lf_wdma_num_bl_reg.bl       = mc_dma_bl;//range_rand(10, 63) * 2;
	if(line_total % mc_dma_mc_lf_wdma_num_bl_reg.bl == 0){
		mc_dma_mc_lf_wdma_num_bl_reg.remain = mc_dma_mc_lf_wdma_num_bl_reg.bl;
		mc_dma_mc_lf_wdma_num_bl_reg.num    = line_total / mc_dma_mc_lf_wdma_num_bl_reg.bl - 1;
	} else {
		mc_dma_mc_lf_wdma_num_bl_reg.num    = line_total / mc_dma_mc_lf_wdma_num_bl_reg.bl;
		mc_dma_mc_lf_wdma_num_bl_reg.remain = line_total % mc_dma_mc_lf_wdma_num_bl_reg.bl;
	}
	mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr = ((int)((line_total+5)/6)) *6; //96B align,unit:128
  
	mc_dma_mc_lf_dma_wr_ctrl_reg.max_outstanding = 15;

#if defined(MARK2)
	line_total = (int)((mc_dma_mc_wdma_ctrl0_reg.dat_fmt + 2) * ((mc_dma_mc_wdma_ctrl0_reg.dat_bit == 0) ? 9 : 11) * mc_dma_mc_hf_wdma_ctrl1_reg.width + dbus_width - 1) / dbus_width;
#endif
#if defined(H5C2)
	line_total = (int)((mc_dma_mc_wdma_ctrl0_reg.dat_fmt + 2) * ((mc_dma_mc_wdma_ctrl0_reg.dat_bit == 0) ? 9 : 11) * mc_dma_mc_hf_wdma_ctrl1_reg.width + dbus_width - 1) / dbus_width * 2;
#endif
#if defined(MERLIN6) || defined(MERLIN7)
	line_total = (int)((mc_dma_mc_wdma_ctrl0_reg.dat_fmt + 2) * ((mc_dma_mc_wdma_ctrl0_reg.dat_bit == 0) ? 9 : 11) * mc_dma_mc_wdma_ctrl1_reg.width + dbus_width - 1) / dbus_width;
#endif
#if (defined(MARK2)||defined(MERLIN7)||defined(MERLIN6))
	if(pqc_bypass == 0 /*&& comp_line_mode == 1*/)
		line_total = (pqc_pqdc_mc_hf_pq_cmp_pair_reg.cmp_line_sum_bit );
#endif
#if defined(H5C2)
#if (defined(MARK2)||defined(MERLIN7)||defined(MERLIN6)||defined(NEED_FIX_IT))
	if(pqc_bypass == 0 /*&& comp_line_mode == 1*/)
		line_total = (mc_pqc4p_pqdc4p_mc_hf_pq_cmp_pair_reg.cmp_line_sum_bit );
#endif
#endif
	mc_dma_mc_hf_wdma_num_bl_reg.bl      = mc_dma_bl;//range_rand(10, 63) * 2; 
	if(line_total % mc_dma_mc_hf_wdma_num_bl_reg.bl == 0){
		mc_dma_mc_hf_wdma_num_bl_reg.remain = mc_dma_mc_hf_wdma_num_bl_reg.bl;
		mc_dma_mc_hf_wdma_num_bl_reg.num    = line_total / mc_dma_mc_hf_wdma_num_bl_reg.bl - 1;
	} else {
		mc_dma_mc_hf_wdma_num_bl_reg.num    = line_total / mc_dma_mc_hf_wdma_num_bl_reg.bl;
		mc_dma_mc_hf_wdma_num_bl_reg.remain = line_total % mc_dma_mc_hf_wdma_num_bl_reg.bl;
	}

	mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr = ((int)((line_total+5)/6)) *6; //96B align,unit:128

	mc_dma_mc_hf_dma_wr_ctrl_reg.max_outstanding = 15;

#if defined(MARK2)|| defined(H5C2)
#if defined (MARK2)
	line_total = (int)((mc_dma_mc_wdma_ctrl0_reg.dat_fmt + 2) * ((mc_dma_mc_wdma_ctrl0_reg.dat_bit == 0) ? 9 : 11) * mc_dma_mc_lfh_wdma_ctrl1_reg.width + dbus_width - 1) / dbus_width;
	if(pqc_bypass == 0 /*&& comp_line_mode == 1*/)
		line_total = (pqc_pqdc_mc_lfh_pq_cmp_pair_reg.cmp_line_sum_bit );
#elif defined(H5C2)
	line_total = (int)((mc_dma_mc_wdma_ctrl0_reg.dat_fmt + 2) * ((mc_dma_mc_wdma_ctrl0_reg.dat_bit == 0) ? 9 : 11) * mc_dma_mc_lfh_wdma_ctrl1_reg.width + dbus_width - 1) / dbus_width * 2;
#if (defined(MARK2)||defined(MERLIN7)||defined(MERLIN6)||defined(NEED_FIX_IT))
	if(pqc_bypass == 0 /*&& comp_line_mode == 1*/)
		line_total = (mc_pqc4p_pqdc4p_mc_lfh_pq_cmp_pair_reg.cmp_line_sum_bit );
#endif
#endif
	mc_dma_mc_lfh_wdma_num_bl_reg.bl      = mc_dma_bl;//range_rand(10, 63) * 2; 
	if(line_total % mc_dma_mc_lfh_wdma_num_bl_reg.bl == 0){
		mc_dma_mc_lfh_wdma_num_bl_reg.remain = mc_dma_mc_lfh_wdma_num_bl_reg.bl;
		mc_dma_mc_lfh_wdma_num_bl_reg.num    = line_total / mc_dma_mc_lfh_wdma_num_bl_reg.bl - 1;
	} else {
		mc_dma_mc_lfh_wdma_num_bl_reg.num    = line_total / mc_dma_mc_lfh_wdma_num_bl_reg.bl;
		mc_dma_mc_lfh_wdma_num_bl_reg.remain = line_total % mc_dma_mc_lfh_wdma_num_bl_reg.bl;
	}

	mc_dma_mc_lfh_wdma_lstep_reg.line_offset_addr = ((int)((line_total+5)/6)) *6; //96B align,unit:128

	
	mc_dma_mc_lfh_dma_wr_ctrl_reg.max_outstanding = 15;

#endif
	//rdma
	mc_dma_mc_rdma_ctrl_reg.vflip_enable = 0; //nouse
	mc_dma_mc_rdma_ctrl_reg.dat_fmt      = mc_dma_mc_wdma_ctrl0_reg.dat_fmt;//???
	mc_dma_mc_rdma_ctrl_reg.dat_bit      = mc_dma_mc_wdma_ctrl0_reg.dat_bit;//???

	//lf iframe rdma
#if defined(MARK2)|| defined(H5C2)
	mc_dma_mc_lf_rdma_ctrl1_reg.regValue = mc_dma_mc_lf_wdma_ctrl1_reg.regValue;
	mc_dma_mc_hf_rdma_ctrl1_reg.regValue = mc_dma_mc_hf_wdma_ctrl1_reg.regValue;
	mc_dma_mc_lfh_rdma_ctrl1_reg.regValue = mc_dma_mc_lfh_wdma_ctrl1_reg.regValue;
	
	
#if defined(MARK2)
	line_total = (int)((mc_dma_mc_rdma_ctrl_reg.dat_fmt + 2) * ((mc_dma_mc_rdma_ctrl_reg.dat_bit == 0) ? 8 : 10) * mc_dma_mc_lf_rdma_ctrl1_reg.width + dbus_width - 1) / dbus_width;
	if(pqc_bypass == 0/* && comp_line_mode == 1*/)
		line_total = (pqc_pqdc_mc_lfl_pq_cmp_pair_reg.cmp_line_sum_bit );
#elif defined(H5C2)
	line_total = (int)((mc_dma_mc_rdma_ctrl_reg.dat_fmt + 2) * ((mc_dma_mc_rdma_ctrl_reg.dat_bit == 0) ? 8 : 10) * mc_dma_mc_lf_rdma_ctrl1_reg.width + dbus_width - 1) / dbus_width * 2;
#if (defined(MARK2)||defined(MERLIN7)||defined(MERLIN6)||defined(NEED_FIX_IT))
	if(pqc_bypass == 0/* && comp_line_mode == 1*/)
		line_total = (mc_pqc4p_pqdc4p_mc_lf_pq_cmp_pair_reg.cmp_line_sum_bit );
#endif
#endif
#endif
#if defined(MERLIN6) || defined(MERLIN7)
	mc_dma_mc_rdma_ctrl1_reg.regValue = mc_dma_mc_wdma_ctrl1_reg.regValue;
			
	line_total = (int)((mc_dma_mc_rdma_ctrl_reg.dat_fmt + 2) * ((mc_dma_mc_rdma_ctrl_reg.dat_bit == 0) ? 8 : 10) * mc_dma_mc_rdma_ctrl1_reg.width + dbus_width - 1) / dbus_width;
		if(pqc_bypass == 0/* && comp_line_mode == 1*/)
		line_total = (pqc_pqdc_mc_lf_pq_cmp_pair_reg.cmp_line_sum_bit );
#endif
	mc_dma_mc_lf_i_rdma_num_bl_reg.bl       = mc_dma_bl;//range_rand(10, 63) * 2; 
	if(line_total % mc_dma_mc_lf_i_rdma_num_bl_reg.bl == 0){
		mc_dma_mc_lf_i_rdma_num_bl_reg.remain = mc_dma_mc_lf_i_rdma_num_bl_reg.bl;
		mc_dma_mc_lf_i_rdma_num_bl_reg.num    = line_total / mc_dma_mc_lf_i_rdma_num_bl_reg.bl - 1;
	} else {
		mc_dma_mc_lf_i_rdma_num_bl_reg.num    = line_total / mc_dma_mc_lf_i_rdma_num_bl_reg.bl;
		mc_dma_mc_lf_i_rdma_num_bl_reg.remain = line_total % mc_dma_mc_lf_i_rdma_num_bl_reg.bl;
	}
#if defined(MARK2)|| defined(H5C2)
	mc_dma_mc_lf_i_dma_rd_ctrl_reg.max_outstanding = 15;
#endif
#if defined(MERLIN6) || defined(MERLIN7)
	mc_dma_mc_lf_i_dma_rd_ctrl_reg.max_outstanding = 7;
#endif
	//hf iframe rdma
	//i_mc_hf_rdma_ctrl1.width = mc_dma_mc_wdma_ctrl1_reg.width;
	//i_mc_hf_rdma_ctrl1.height = mc_dma_mc_wdma_ctrl1_reg.height;
#if defined(MARK2)
	line_total = (int)((mc_dma_mc_rdma_ctrl_reg.dat_fmt + 2) * ((mc_dma_mc_rdma_ctrl_reg.dat_bit == 0) ? 9 : 11) * mc_dma_mc_hf_rdma_ctrl1_reg.width + dbus_width - 1) / dbus_width;
#endif
#if defined(H5C2)
	line_total = (int)((mc_dma_mc_rdma_ctrl_reg.dat_fmt + 2) * ((mc_dma_mc_rdma_ctrl_reg.dat_bit == 0) ? 9 : 11) * mc_dma_mc_hf_rdma_ctrl1_reg.width + dbus_width - 1) / dbus_width * 2;
#endif
#if defined(MERLIN6) || defined(MERLIN7)
	line_total = (int)((mc_dma_mc_rdma_ctrl_reg.dat_fmt + 2) * ((mc_dma_mc_rdma_ctrl_reg.dat_bit == 0) ? 9 : 11) * mc_dma_mc_rdma_ctrl1_reg.width + dbus_width - 1) / dbus_width;
#endif
#if (defined(MARK2)||defined(MERLIN7)||defined(MERLIN6))
	if(pqc_bypass == 0 /*&& comp_line_mode == 1*/)
		line_total = (pqc_pqdc_mc_hf_pq_cmp_pair_reg.cmp_line_sum_bit );
#endif
#if defined(H5C2)
#if (defined(MARK2)||defined(MERLIN7)||defined(MERLIN6)||defined(NEED_FIX_IT))
	if(pqc_bypass == 0 /*&& comp_line_mode == 1*/)
		line_total = (mc_pqc4p_pqdc4p_mc_hf_pq_cmp_pair_reg.cmp_line_sum_bit );
#endif
#endif
	mc_dma_mc_hf_i_rdma_num_bl_reg.bl      = mc_dma_bl;//range_rand(10, 63) * 2; 
	if(line_total % mc_dma_mc_hf_i_rdma_num_bl_reg.bl == 0){
		mc_dma_mc_hf_i_rdma_num_bl_reg.remain = mc_dma_mc_hf_i_rdma_num_bl_reg.bl;
		mc_dma_mc_hf_i_rdma_num_bl_reg.num    = line_total / mc_dma_mc_hf_i_rdma_num_bl_reg.bl - 1;
	} else {
		mc_dma_mc_hf_i_rdma_num_bl_reg.num    = line_total / mc_dma_mc_hf_i_rdma_num_bl_reg.bl;
		mc_dma_mc_hf_i_rdma_num_bl_reg.remain = line_total % mc_dma_mc_hf_i_rdma_num_bl_reg.bl;
	}

#if defined(MARK2)|| defined(H5C2)
	mc_dma_mc_hf_i_dma_rd_ctrl_reg.max_outstanding = 15;
#endif
#if defined(MERLIN6) || defined(MERLIN7)
	mc_dma_mc_hf_i_dma_rd_ctrl_reg.max_outstanding = 7;
#endif
	//lf pframe rdma
#if defined(MARK2)
	line_total = (int)((mc_dma_mc_rdma_ctrl_reg.dat_fmt + 2) * ((mc_dma_mc_rdma_ctrl_reg.dat_bit == 0) ? 8 : 10)* mc_dma_mc_lf_rdma_ctrl1_reg.width + dbus_width - 1) / dbus_width;
	if(pqc_bypass == 0 /*&& comp_line_mode == 1*/)
		line_total = (pqc_pqdc_mc_lfl_pq_cmp_pair_reg.cmp_line_sum_bit );
#endif
#if defined(MERLIN6) || defined(MERLIN7)
	line_total = (int)((mc_dma_mc_rdma_ctrl_reg.dat_fmt + 2) * ((mc_dma_mc_rdma_ctrl_reg.dat_bit == 0) ? 8 : 10)* mc_dma_mc_rdma_ctrl1_reg.width + dbus_width - 1) / dbus_width;
	if(pqc_bypass == 0 /*&& comp_line_mode == 1*/)
		line_total = (pqc_pqdc_mc_lf_pq_cmp_pair_reg.cmp_line_sum_bit );
#endif
#if defined(H5C2)
	line_total = (int)((mc_dma_mc_rdma_ctrl_reg.dat_fmt + 2) * ((mc_dma_mc_rdma_ctrl_reg.dat_bit == 0) ? 8 : 10)* mc_dma_mc_lf_rdma_ctrl1_reg.width + dbus_width - 1) / dbus_width * 2;
#if (defined(MARK2)||defined(MERLIN7)||defined(MERLIN6)||defined(NEED_FIX_IT))
	if(pqc_bypass == 0 /*&& comp_line_mode == 1*/)
		line_total = (mc_pqc4p_pqdc4p_mc_lf_pq_cmp_pair_reg.cmp_line_sum_bit );
#endif
#endif
	mc_dma_mc_lf_p_rdma_num_bl_reg.bl       = mc_dma_bl;//range_rand(10, 63) * 2; 
	if(line_total % mc_dma_mc_lf_p_rdma_num_bl_reg.bl == 0){
		mc_dma_mc_lf_p_rdma_num_bl_reg.remain = mc_dma_mc_lf_p_rdma_num_bl_reg.bl;
		mc_dma_mc_lf_p_rdma_num_bl_reg.num    = line_total / mc_dma_mc_lf_p_rdma_num_bl_reg.bl - 1;
	} else {
		mc_dma_mc_lf_p_rdma_num_bl_reg.num    = line_total / mc_dma_mc_lf_p_rdma_num_bl_reg.bl;
		mc_dma_mc_lf_p_rdma_num_bl_reg.remain = line_total % mc_dma_mc_lf_p_rdma_num_bl_reg.bl;
	}

#if defined(MARK2)|| defined(H5C2)
	mc_dma_mc_lf_p_dma_rd_ctrl_reg.max_outstanding = 15;
#endif
#if defined(MERLIN6) || defined(MERLIN7)
	mc_dma_mc_lf_p_dma_rd_ctrl_reg.max_outstanding = 7;
#endif
	//hf pframe rdma
#if defined(MARK2)
	line_total = (int)((mc_dma_mc_rdma_ctrl_reg.dat_fmt + 2) * ((mc_dma_mc_rdma_ctrl_reg.dat_bit == 0) ? 9 : 11) * mc_dma_mc_hf_rdma_ctrl1_reg.width + dbus_width - 1) / dbus_width;
#endif
#if defined(H5C2)
	line_total = (int)((mc_dma_mc_rdma_ctrl_reg.dat_fmt + 2) * ((mc_dma_mc_rdma_ctrl_reg.dat_bit == 0) ? 9 : 11) * mc_dma_mc_hf_rdma_ctrl1_reg.width + dbus_width - 1) / dbus_width * 2;
#endif
#if defined(MERLIN6) || defined(MERLIN7)
	line_total = (int)((mc_dma_mc_rdma_ctrl_reg.dat_fmt + 2) * ((mc_dma_mc_rdma_ctrl_reg.dat_bit == 0) ? 9 : 11) * mc_dma_mc_rdma_ctrl1_reg.width + dbus_width - 1) / dbus_width;
#endif
#if (defined(MARK2)||defined(MERLIN7)||defined(MERLIN6))
	if(pqc_bypass == 0 /*&& comp_line_mode == 1*/)
		line_total = (pqc_pqdc_mc_hf_pq_cmp_pair_reg.cmp_line_sum_bit );
#endif
#if defined(H5C2)
#if (defined(MARK2)||defined(MERLIN7)||defined(MERLIN6)||defined(NEED_FIX_IT))
	if(pqc_bypass == 0 /*&& comp_line_mode == 1*/)
		line_total = (mc_pqc4p_pqdc4p_mc_hf_pq_cmp_pair_reg.cmp_line_sum_bit );
#endif
#endif
	mc_dma_mc_hf_p_rdma_num_bl_reg.bl      = mc_dma_bl;//range_rand(10, 63) * 2; //range_rand();???
	if(line_total % mc_dma_mc_hf_p_rdma_num_bl_reg.bl == 0){
		mc_dma_mc_hf_p_rdma_num_bl_reg.remain = mc_dma_mc_hf_p_rdma_num_bl_reg.bl;
		mc_dma_mc_hf_p_rdma_num_bl_reg.num    = line_total / mc_dma_mc_hf_p_rdma_num_bl_reg.bl - 1;
	} else {
		mc_dma_mc_hf_p_rdma_num_bl_reg.num    = line_total / mc_dma_mc_hf_p_rdma_num_bl_reg.bl;
		mc_dma_mc_hf_p_rdma_num_bl_reg.remain = line_total % mc_dma_mc_hf_p_rdma_num_bl_reg.bl;
	}

#if defined(MARK2)|| defined(H5C2)
	mc_dma_mc_hf_p_dma_rd_ctrl_reg.max_outstanding = 15;
#endif
#if defined(MERLIN6)  || defined(MERLIN7)
	mc_dma_mc_hf_p_dma_rd_ctrl_reg.max_outstanding = 7;
#endif
    //lfh iframe
#if defined(MARK2)|| defined(H5C2)
	
#if defined(MARK2)
	line_total = (int)((mc_dma_mc_rdma_ctrl_reg.dat_fmt + 2) * ((mc_dma_mc_rdma_ctrl_reg.dat_bit == 0) ? 9 : 11) * mc_dma_mc_lfh_rdma_ctrl1_reg.width + dbus_width - 1) / dbus_width;
	if(pqc_bypass == 0/* && comp_line_mode == 1*/)
		line_total = (pqc_pqdc_mc_lfh_pq_cmp_pair_reg.cmp_line_sum_bit );
#elif defined(H5C2)
	line_total = (int)((mc_dma_mc_rdma_ctrl_reg.dat_fmt + 2) * ((mc_dma_mc_rdma_ctrl_reg.dat_bit == 0) ? 9 : 11) * mc_dma_mc_lfh_rdma_ctrl1_reg.width + dbus_width - 1) / dbus_width * 2;
#if (defined(MARK2)||defined(MERLIN7)||defined(MERLIN6)||defined(NEED_FIX_IT))
	if(pqc_bypass == 0/* && comp_line_mode == 1*/)
		line_total = (mc_pqc4p_pqdc4p_mc_lfh_pq_cmp_pair_reg.cmp_line_sum_bit );
#endif
#endif

	mc_dma_mc_lfh_i_rdma_num_bl_reg.bl       = mc_dma_bl;//range_rand(10, 63) * 2; 
	if(line_total % mc_dma_mc_lfh_i_rdma_num_bl_reg.bl == 0){
		mc_dma_mc_lfh_i_rdma_num_bl_reg.remain = mc_dma_mc_lfh_i_rdma_num_bl_reg.bl;
		mc_dma_mc_lfh_i_rdma_num_bl_reg.num    = line_total / mc_dma_mc_lfh_i_rdma_num_bl_reg.bl - 1;
	} else {
		mc_dma_mc_lfh_i_rdma_num_bl_reg.num    = line_total / mc_dma_mc_lfh_i_rdma_num_bl_reg.bl;
		mc_dma_mc_lfh_i_rdma_num_bl_reg.remain = line_total % mc_dma_mc_lfh_i_rdma_num_bl_reg.bl;
	}

	
	mc_dma_mc_lfh_i_dma_rd_ctrl_reg.max_outstanding = 15;
	
		//lfh pframe rdma

	
#if defined(MARK2)
	line_total = (int)((mc_dma_mc_rdma_ctrl_reg.dat_fmt + 2) * ((mc_dma_mc_rdma_ctrl_reg.dat_bit == 0) ? 9 : 11)* mc_dma_mc_lfh_rdma_ctrl1_reg.width + dbus_width - 1) / dbus_width;
	if(pqc_bypass == 0 /*&& comp_line_mode == 1*/)
		line_total = (pqc_pqdc_mc_lfh_pq_cmp_pair_reg.cmp_line_sum_bit );
#elif defined(H5C2)
	line_total = (int)((mc_dma_mc_rdma_ctrl_reg.dat_fmt + 2) * ((mc_dma_mc_rdma_ctrl_reg.dat_bit == 0) ? 9 : 11)* mc_dma_mc_lfh_rdma_ctrl1_reg.width + dbus_width - 1) / dbus_width * 2;
#if (defined(MARK2)||defined(MERLIN7)||defined(MERLIN6)||defined(NEED_FIX_IT))
	if(pqc_bypass == 0 /*&& comp_line_mode == 1*/)
		line_total = (mc_pqc4p_pqdc4p_mc_lfh_pq_cmp_pair_reg.cmp_line_sum_bit );
#endif
#endif

	mc_dma_mc_lfh_p_rdma_num_bl_reg.bl       = mc_dma_bl;//range_rand(10, 63) * 2; 
	if(line_total % mc_dma_mc_lf_p_rdma_num_bl_reg.bl == 0){
		mc_dma_mc_lfh_p_rdma_num_bl_reg.remain = mc_dma_mc_lfh_p_rdma_num_bl_reg.bl;
		mc_dma_mc_lfh_p_rdma_num_bl_reg.num    = line_total / mc_dma_mc_lfh_p_rdma_num_bl_reg.bl - 1;
	} else {
		mc_dma_mc_lfh_p_rdma_num_bl_reg.num    = line_total / mc_dma_mc_lfh_p_rdma_num_bl_reg.bl;
		mc_dma_mc_lfh_p_rdma_num_bl_reg.remain = line_total % mc_dma_mc_lfh_p_rdma_num_bl_reg.bl;
	}


	mc_dma_mc_lfh_p_dma_rd_ctrl_reg.max_outstanding = 15;
#endif	
	//addr
	//mc_dma_mc_lf_wdma_mstart2_reg.start_address2 = 0x38e400;//0xb809941c
	//mc_dma_mc_lf_wdma_mstart3_reg.start_address3 = 0x523400;//0xb8099420
	//mc_dma_mc_lf_wdma_mstart4_reg.start_address4 = 0x6b8400;//0xb8099424
	//mc_dma_mc_lf_wdma_mstart5_reg.start_address5 = 0x84d400;//0xb8099428
	//mc_dma_mc_lf_wdma_mstart6_reg.start_address6 = 0x84d400;//0xb809942c
	//mc_dma_mc_lf_wdma_mstart7_reg.start_address7 = 0x84d400;//0xb8099430
	if(GS_full_dma_setting)
	{mc_dma_mc_lf_wdma_mstart0_reg.start_address0 = GSC_me_end_addr>>4;}//0xb8099414
	else
	{mc_dma_mc_lf_wdma_mstart0_reg.start_address0 = 0x6ffc0;}//0xb8099414

	if(GS_full_dma_setting)
	{
		mc_dma_mc_lf_wdma_mend0_reg.end_address0 = mc_dma_mc_lf_wdma_mstart0_reg.start_address0 + ((GS_MCBuffer_Size>=1)?(mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr * (height/*2160*/ / 2 - 0)):0);//0xb8099434

		mc_dma_mc_hf_wdma_mstart0_reg.start_address0 = mc_dma_mc_lf_wdma_mend0_reg.end_address0  /*+ mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr*/;//0xb8099484
		mc_dma_mc_hf_wdma_mend0_reg.end_address0     = mc_dma_mc_hf_wdma_mstart0_reg.start_address0 + ((GS_MCBuffer_Size>=1)?(mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);

		mc_dma_mc_lf_wdma_mstart1_reg.start_address1 = mc_dma_mc_hf_wdma_mend0_reg.end_address0 /*+ mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr*/;//0xb8099418
		mc_dma_mc_lf_wdma_mend1_reg.end_address1     = mc_dma_mc_lf_wdma_mstart1_reg.start_address1 + ((GS_MCBuffer_Size>=2)?(mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);//0xb8099434

		mc_dma_mc_hf_wdma_mstart1_reg.start_address1 = mc_dma_mc_lf_wdma_mend1_reg.end_address1 /*+ mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr*/;
		mc_dma_mc_hf_wdma_mend1_reg.end_address1     = mc_dma_mc_hf_wdma_mstart1_reg.start_address1 + ((GS_MCBuffer_Size>=2)?(mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);

		mc_dma_mc_lf_wdma_mstart2_reg.start_address2 = mc_dma_mc_hf_wdma_mend1_reg.end_address1 /*+ mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr*/;//0xb8099418
		mc_dma_mc_lf_wdma_mend2_reg.end_address2     = mc_dma_mc_lf_wdma_mstart2_reg.start_address2 + ((GS_MCBuffer_Size>=3)?(mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);//0xb8099434

		mc_dma_mc_hf_wdma_mstart2_reg.start_address2 = mc_dma_mc_lf_wdma_mend2_reg.end_address2 /*+ mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr*/;
		mc_dma_mc_hf_wdma_mend2_reg.end_address2     = mc_dma_mc_hf_wdma_mstart2_reg.start_address2 + ((GS_MCBuffer_Size>=3)?(mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);

		mc_dma_mc_lf_wdma_mstart3_reg.start_address3 = mc_dma_mc_hf_wdma_mend2_reg.end_address2 /*+ mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr*/;//0xb8099418
		mc_dma_mc_lf_wdma_mend3_reg.end_address3     = mc_dma_mc_lf_wdma_mstart3_reg.start_address3 + ((GS_MCBuffer_Size>=4)?(mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);//0xb8099434

		mc_dma_mc_hf_wdma_mstart3_reg.start_address3 = mc_dma_mc_lf_wdma_mend3_reg.end_address3 /*+ mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr*/;
		mc_dma_mc_hf_wdma_mend3_reg.end_address3     = mc_dma_mc_hf_wdma_mstart3_reg.start_address3 + ((GS_MCBuffer_Size>=4)?(mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);

		mc_dma_mc_lf_wdma_mstart4_reg.start_address4 = mc_dma_mc_hf_wdma_mend3_reg.end_address3 /*+ mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr*/;//0xb8099418
		mc_dma_mc_lf_wdma_mend4_reg.end_address4     = mc_dma_mc_lf_wdma_mstart4_reg.start_address4 + ((GS_MCBuffer_Size>=5)?(mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);//0xb8099434

		mc_dma_mc_hf_wdma_mstart4_reg.start_address4 = mc_dma_mc_lf_wdma_mend4_reg.end_address4 /*+ mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr*/;
		mc_dma_mc_hf_wdma_mend4_reg.end_address4     = mc_dma_mc_hf_wdma_mstart4_reg.start_address4 + ((GS_MCBuffer_Size>=5)?(mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);

		mc_dma_mc_lf_wdma_mstart5_reg.start_address5 = mc_dma_mc_hf_wdma_mend4_reg.end_address4 /*+ mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr*/;//0xb8099418
		mc_dma_mc_lf_wdma_mend5_reg.end_address5     = mc_dma_mc_lf_wdma_mstart5_reg.start_address5 + ((GS_MCBuffer_Size>=6)?(mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);//0xb8099434

		mc_dma_mc_hf_wdma_mstart5_reg.start_address5 = mc_dma_mc_lf_wdma_mend5_reg.end_address5 /*+ mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr*/;
		mc_dma_mc_hf_wdma_mend5_reg.end_address5     = mc_dma_mc_hf_wdma_mstart5_reg.start_address5 + ((GS_MCBuffer_Size>=6)?(mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);

		mc_dma_mc_lf_wdma_mstart6_reg.start_address6 = mc_dma_mc_hf_wdma_mend5_reg.end_address5;
		mc_dma_mc_lf_wdma_mend6_reg.end_address6     = mc_dma_mc_lf_wdma_mstart6_reg.start_address6 + ((GS_MCBuffer_Size>=7)?(mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);

		mc_dma_mc_hf_wdma_mstart6_reg.start_address6 = mc_dma_mc_lf_wdma_mend6_reg.end_address6;
		mc_dma_mc_hf_wdma_mend6_reg.end_address6     = mc_dma_mc_hf_wdma_mstart6_reg.start_address6 + ((GS_MCBuffer_Size>=7)?(mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);

		mc_dma_mc_lf_wdma_mstart7_reg.start_address7 = mc_dma_mc_hf_wdma_mend6_reg.end_address6;
		mc_dma_mc_lf_wdma_mend7_reg.end_address7     = mc_dma_mc_lf_wdma_mstart7_reg.start_address7 + ((GS_MCBuffer_Size>=8)?(mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);

		mc_dma_mc_hf_wdma_mstart7_reg.start_address7 = mc_dma_mc_lf_wdma_mend7_reg.end_address7;
		mc_dma_mc_hf_wdma_mend7_reg.end_address7     = mc_dma_mc_hf_wdma_mstart7_reg.start_address7 + ((GS_MCBuffer_Size>=8)?(mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);
#if defined(MARK2)|| defined(H5C2)
		mc_dma_mc_lfh_wdma_mstart0_reg.start_address0 = mc_dma_mc_hf_wdma_mend7_reg.end_address7  /*+ mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr*/;//0xb8099484
		mc_dma_mc_lfh_wdma_mend0_reg.end_address0     = mc_dma_mc_lfh_wdma_mstart0_reg.start_address0 + ((GS_MCBuffer_Size>=1)?(mc_dma_mc_lfh_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);
		mc_dma_mc_lfh_wdma_mstart1_reg.start_address1 = mc_dma_mc_lfh_wdma_mend0_reg.end_address0 /*+ mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr*/;
		mc_dma_mc_lfh_wdma_mend1_reg.end_address1     = mc_dma_mc_lfh_wdma_mstart1_reg.start_address1 + ((GS_MCBuffer_Size>=2)?(mc_dma_mc_lfh_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);
		mc_dma_mc_lfh_wdma_mstart2_reg.start_address2 = mc_dma_mc_lfh_wdma_mend1_reg.end_address1 /*+ mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr*/;
		mc_dma_mc_lfh_wdma_mend2_reg.end_address2     = mc_dma_mc_lfh_wdma_mstart2_reg.start_address2 + ((GS_MCBuffer_Size>=3)?(mc_dma_mc_lfh_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);
		mc_dma_mc_lfh_wdma_mstart3_reg.start_address3 = mc_dma_mc_lfh_wdma_mend2_reg.end_address2 /*+ mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr*/;
		mc_dma_mc_lfh_wdma_mend3_reg.end_address3     = mc_dma_mc_lfh_wdma_mstart3_reg.start_address3 + ((GS_MCBuffer_Size>=4)?(mc_dma_mc_lfh_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);
		mc_dma_mc_lfh_wdma_mstart4_reg.start_address4 = mc_dma_mc_lfh_wdma_mend3_reg.end_address3 /*+ mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr*/;
		mc_dma_mc_lfh_wdma_mend4_reg.end_address4     = mc_dma_mc_lfh_wdma_mstart4_reg.start_address4 + ((GS_MCBuffer_Size>=5)?(mc_dma_mc_lfh_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);
		mc_dma_mc_lfh_wdma_mstart5_reg.start_address5 = mc_dma_mc_lfh_wdma_mend4_reg.end_address4 /*+ mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr*/;
		mc_dma_mc_lfh_wdma_mend5_reg.end_address5     = mc_dma_mc_lfh_wdma_mstart5_reg.start_address5 + ((GS_MCBuffer_Size>=6)?(mc_dma_mc_lfh_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);
		mc_dma_mc_lfh_wdma_mstart6_reg.start_address6 = mc_dma_mc_lfh_wdma_mend5_reg.end_address5;
		mc_dma_mc_lfh_wdma_mend6_reg.end_address6     = mc_dma_mc_lfh_wdma_mstart6_reg.start_address6 + ((GS_MCBuffer_Size>=7)?(mc_dma_mc_lfh_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);
		mc_dma_mc_lfh_wdma_mstart7_reg.start_address7 = mc_dma_mc_lfh_wdma_mend6_reg.end_address6;
		mc_dma_mc_lfh_wdma_mend7_reg.end_address7     = mc_dma_mc_lfh_wdma_mstart7_reg.start_address7 + ((GS_MCBuffer_Size>=8)?(mc_dma_mc_lfh_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);
#endif
	}
	else
	{
		mc_dma_mc_lf_wdma_mend0_reg.end_address0     = mc_dma_mc_lf_wdma_mstart0_reg.start_address0 + ((GS_MCBuffer_Size>=1)?(mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);//0xb8099434
		mc_dma_mc_lf_wdma_mstart1_reg.start_address1 = mc_dma_mc_lf_wdma_mend0_reg.end_address0 /*+ mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr*/;//0xb8099418
		mc_dma_mc_lf_wdma_mend1_reg.end_address1     = mc_dma_mc_lf_wdma_mstart1_reg.start_address1 + ((GS_MCBuffer_Size>=2)?(mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);//0xb8099434
		mc_dma_mc_lf_wdma_mstart2_reg.start_address2 = mc_dma_mc_lf_wdma_mend1_reg.end_address1 /*+ mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr*/;//0xb8099418
		mc_dma_mc_lf_wdma_mend2_reg.end_address2     = mc_dma_mc_lf_wdma_mstart2_reg.start_address2 + ((GS_MCBuffer_Size>=3)?(mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);//0xb8099434
		mc_dma_mc_lf_wdma_mstart3_reg.start_address3 = mc_dma_mc_lf_wdma_mend2_reg.end_address2 /*+ mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr*/;//0xb8099418
		mc_dma_mc_lf_wdma_mend3_reg.end_address3     = mc_dma_mc_lf_wdma_mstart3_reg.start_address3 + ((GS_MCBuffer_Size>=4)?(mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);//0xb8099434
		mc_dma_mc_lf_wdma_mstart4_reg.start_address4 = mc_dma_mc_lf_wdma_mend3_reg.end_address3 /*+ mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr*/;//0xb8099418
		mc_dma_mc_lf_wdma_mend4_reg.end_address4     = mc_dma_mc_lf_wdma_mstart4_reg.start_address4 + ((GS_MCBuffer_Size>=5)?(mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);//0xb8099434
		mc_dma_mc_lf_wdma_mstart5_reg.start_address5 = mc_dma_mc_lf_wdma_mend4_reg.end_address4 /*+ mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr*/;//0xb8099418
		mc_dma_mc_lf_wdma_mend5_reg.end_address5     = mc_dma_mc_lf_wdma_mstart5_reg.start_address5 + ((GS_MCBuffer_Size>=6)?(mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);//0xb8099434
		mc_dma_mc_lf_wdma_mstart6_reg.start_address6 = mc_dma_mc_lf_wdma_mend5_reg.end_address5;
		mc_dma_mc_lf_wdma_mend6_reg.end_address6     = mc_dma_mc_lf_wdma_mstart6_reg.start_address6 + ((GS_MCBuffer_Size>=7)?(mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);
		mc_dma_mc_lf_wdma_mstart7_reg.start_address7 = mc_dma_mc_lf_wdma_mend6_reg.end_address6;
		mc_dma_mc_lf_wdma_mend7_reg.end_address7     = mc_dma_mc_lf_wdma_mstart7_reg.start_address7 + ((GS_MCBuffer_Size>=8)?(mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);
		mc_dma_mc_hf_wdma_mstart0_reg.start_address0 = mc_dma_mc_lf_wdma_mend7_reg.end_address7  /*+ mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr*/;//0xb8099484
		mc_dma_mc_hf_wdma_mend0_reg.end_address0     = mc_dma_mc_hf_wdma_mstart0_reg.start_address0 + ((GS_MCBuffer_Size>=1)?(mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);
		mc_dma_mc_hf_wdma_mstart1_reg.start_address1 = mc_dma_mc_hf_wdma_mend0_reg.end_address0 /*+ mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr*/;
		mc_dma_mc_hf_wdma_mend1_reg.end_address1     = mc_dma_mc_hf_wdma_mstart1_reg.start_address1 + ((GS_MCBuffer_Size>=2)?(mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);
		mc_dma_mc_hf_wdma_mstart2_reg.start_address2 = mc_dma_mc_hf_wdma_mend1_reg.end_address1 /*+ mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr*/;
		mc_dma_mc_hf_wdma_mend2_reg.end_address2     = mc_dma_mc_hf_wdma_mstart2_reg.start_address2 + ((GS_MCBuffer_Size>=3)?(mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);
		mc_dma_mc_hf_wdma_mstart3_reg.start_address3 = mc_dma_mc_hf_wdma_mend2_reg.end_address2 /*+ mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr*/;
		mc_dma_mc_hf_wdma_mend3_reg.end_address3     = mc_dma_mc_hf_wdma_mstart3_reg.start_address3 + ((GS_MCBuffer_Size>=4)?(mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);
		mc_dma_mc_hf_wdma_mstart4_reg.start_address4 = mc_dma_mc_hf_wdma_mend3_reg.end_address3 /*+ mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr*/;
		mc_dma_mc_hf_wdma_mend4_reg.end_address4     = mc_dma_mc_hf_wdma_mstart4_reg.start_address4 + ((GS_MCBuffer_Size>=5)?(mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);
		mc_dma_mc_hf_wdma_mstart5_reg.start_address5 = mc_dma_mc_hf_wdma_mend4_reg.end_address4 /*+ mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr*/;
		mc_dma_mc_hf_wdma_mend5_reg.end_address5     = mc_dma_mc_hf_wdma_mstart5_reg.start_address5 + ((GS_MCBuffer_Size>=6)?(mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);
		mc_dma_mc_hf_wdma_mstart6_reg.start_address6 = mc_dma_mc_hf_wdma_mend5_reg.end_address5;
		mc_dma_mc_hf_wdma_mend6_reg.end_address6     = mc_dma_mc_hf_wdma_mstart6_reg.start_address6 + ((GS_MCBuffer_Size>=7)?(mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);
		mc_dma_mc_hf_wdma_mstart7_reg.start_address7 = mc_dma_mc_hf_wdma_mend6_reg.end_address6;
		mc_dma_mc_hf_wdma_mend7_reg.end_address7     = mc_dma_mc_hf_wdma_mstart7_reg.start_address7 + ((GS_MCBuffer_Size>=8)?(mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);

#if defined(MARK2)|| defined(H5C2)
		mc_dma_mc_lfh_wdma_mstart0_reg.start_address0 = mc_dma_mc_hf_wdma_mend7_reg.end_address7  /*+ mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr*/;//0xb8099484
		mc_dma_mc_lfh_wdma_mend0_reg.end_address0     = mc_dma_mc_lfh_wdma_mstart0_reg.start_address0 + ((GS_MCBuffer_Size>=1)?(mc_dma_mc_lfh_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);
		mc_dma_mc_lfh_wdma_mstart1_reg.start_address1 = mc_dma_mc_lfh_wdma_mend0_reg.end_address0 /*+ mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr*/;
		mc_dma_mc_lfh_wdma_mend1_reg.end_address1     = mc_dma_mc_lfh_wdma_mstart1_reg.start_address1 + ((GS_MCBuffer_Size>=2)?(mc_dma_mc_lfh_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);
		mc_dma_mc_lfh_wdma_mstart2_reg.start_address2 = mc_dma_mc_lfh_wdma_mend1_reg.end_address1 /*+ mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr*/;
		mc_dma_mc_lfh_wdma_mend2_reg.end_address2     = mc_dma_mc_lfh_wdma_mstart2_reg.start_address2 + ((GS_MCBuffer_Size>=3)?(mc_dma_mc_lfh_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);
		mc_dma_mc_lfh_wdma_mstart3_reg.start_address3 = mc_dma_mc_lfh_wdma_mend2_reg.end_address2 /*+ mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr*/;
		mc_dma_mc_lfh_wdma_mend3_reg.end_address3     = mc_dma_mc_lfh_wdma_mstart3_reg.start_address3 + ((GS_MCBuffer_Size>=4)?(mc_dma_mc_lfh_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);
		mc_dma_mc_lfh_wdma_mstart4_reg.start_address4 = mc_dma_mc_lfh_wdma_mend3_reg.end_address3 /*+ mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr*/;
		mc_dma_mc_lfh_wdma_mend4_reg.end_address4     = mc_dma_mc_lfh_wdma_mstart4_reg.start_address4 + ((GS_MCBuffer_Size>=5)?(mc_dma_mc_lfh_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);
		mc_dma_mc_lfh_wdma_mstart5_reg.start_address5 = mc_dma_mc_lfh_wdma_mend4_reg.end_address4 /*+ mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr*/;
		mc_dma_mc_lfh_wdma_mend5_reg.end_address5     = mc_dma_mc_lfh_wdma_mstart5_reg.start_address5 + ((GS_MCBuffer_Size>=6)?(mc_dma_mc_lfh_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);
		mc_dma_mc_lfh_wdma_mstart6_reg.start_address6 = mc_dma_mc_lfh_wdma_mend5_reg.end_address5;
		mc_dma_mc_lfh_wdma_mend6_reg.end_address6     = mc_dma_mc_lfh_wdma_mstart6_reg.start_address6 + ((GS_MCBuffer_Size>=7)?(mc_dma_mc_lfh_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);
		mc_dma_mc_lfh_wdma_mstart7_reg.start_address7 = mc_dma_mc_lfh_wdma_mend6_reg.end_address6;
		mc_dma_mc_lfh_wdma_mend7_reg.end_address7     = mc_dma_mc_lfh_wdma_mstart7_reg.start_address7 + ((GS_MCBuffer_Size>=8)?(mc_dma_mc_lfh_wdma_lstep_reg.line_offset_addr * (height / 2 - 0)):0);
#endif
	}

	kphase_kphase_10_reg.kphase_in_mc_index_bsize = GS_MCBuffer_Size;

  //0406
	mc_dma_mc_lf_dma_wr_rule_check_up_reg.dma_up_limit     = mc_dma_mc_lf_wdma_mend7_reg.end_address7;
	mc_dma_mc_lf_i_dma_rd_rule_check_up_reg.dma_up_limit   = mc_dma_mc_lf_wdma_mend7_reg.end_address7;
	mc_dma_mc_lf_p_dma_rd_rule_check_up_reg.dma_up_limit   = mc_dma_mc_lf_wdma_mend7_reg.end_address7;
	mc_dma_mc_lf_dma_wr_rule_check_low_reg.dma_low_limit   = mc_dma_mc_lf_wdma_mstart0_reg.start_address0;
	mc_dma_mc_lf_i_dma_rd_rule_check_low_reg.dma_low_limit = mc_dma_mc_lf_wdma_mstart0_reg.start_address0;
	mc_dma_mc_lf_p_dma_rd_rule_check_low_reg.dma_low_limit = mc_dma_mc_lf_wdma_mstart0_reg.start_address0;
	
	
	mc_dma_mc_hf_dma_wr_rule_check_up_reg.dma_up_limit     = mc_dma_mc_hf_wdma_mend7_reg.end_address7;
	mc_dma_mc_hf_i_dma_rd_rule_check_up_reg.dma_up_limit   = mc_dma_mc_hf_wdma_mend7_reg.end_address7;
	mc_dma_mc_hf_p_dma_rd_rule_check_up_reg.dma_up_limit   = mc_dma_mc_hf_wdma_mend7_reg.end_address7;
	mc_dma_mc_hf_dma_wr_rule_check_low_reg.dma_low_limit   = mc_dma_mc_hf_wdma_mstart0_reg.start_address0;
	mc_dma_mc_hf_i_dma_rd_rule_check_low_reg.dma_low_limit = mc_dma_mc_hf_wdma_mstart0_reg.start_address0;
	mc_dma_mc_hf_p_dma_rd_rule_check_low_reg.dma_low_limit = mc_dma_mc_hf_wdma_mstart0_reg.start_address0;
#if defined(MARK2)	|| defined(H5C2)
	mc_dma_mc_lfh_dma_wr_rule_check_up_reg.dma_up_limit     = mc_dma_mc_lfh_wdma_mend7_reg.end_address7;
	mc_dma_mc_lfh_i_dma_rd_rule_check_up_reg.dma_up_limit   = mc_dma_mc_lfh_wdma_mend7_reg.end_address7;
	mc_dma_mc_lfh_p_dma_rd_rule_check_up_reg.dma_up_limit   = mc_dma_mc_lfh_wdma_mend7_reg.end_address7;
	mc_dma_mc_lfh_dma_wr_rule_check_low_reg.dma_low_limit   = mc_dma_mc_lfh_wdma_mstart0_reg.start_address0;
	mc_dma_mc_lfh_i_dma_rd_rule_check_low_reg.dma_low_limit = mc_dma_mc_lfh_wdma_mstart0_reg.start_address0;
	mc_dma_mc_lfh_p_dma_rd_rule_check_low_reg.dma_low_limit = mc_dma_mc_lfh_wdma_mstart0_reg.start_address0;
  //0406

	GSC_mc_end_addr = (mc_dma_mc_lfh_dma_wr_rule_check_up_reg.dma_up_limit<<4);
#else
	GSC_mc_end_addr = (mc_dma_mc_hf_dma_wr_rule_check_up_reg.dma_up_limit<<4);
#endif

	// register write
	rtd_outl(MC_DMA_MC_WDMA_CTRL1_reg, mc_dma_mc_wdma_ctrl1_reg.regValue);
	rtd_outl(MC_DMA_MC_RDMA_CTRL1_reg, mc_dma_mc_rdma_ctrl1_reg.regValue);
	rtd_outl(PQC_PQDC_MC_PQ_CMP_reg, pqc_pqdc_mc_pq_cmp_reg.regValue);
	rtd_outl(PQC_PQDC_MC_PQ_DECMP_reg, pqc_pqdc_mc_pq_decmp_reg.regValue);
	rtd_outl(PQC_PQDC_MC_LF_PQ_CMP_PAIR_reg, pqc_pqdc_mc_lf_pq_cmp_pair_reg.regValue);
	rtd_outl(PQC_PQDC_MC_LF_PQ_CMP_BIT_reg, pqc_pqdc_mc_lf_pq_cmp_bit_reg.regValue);
	rtd_outl(PQC_PQDC_MC_LF_PQ_DECMP_PAIR_reg, pqc_pqdc_mc_lf_pq_decmp_pair_reg.regValue);
	rtd_outl(PQC_PQDC_MC_LF_PQ_CMP_POOR_reg, pqc_pqdc_mc_lf_pq_cmp_poor_reg.regValue);
	rtd_outl(PQC_PQDC_MC_LF_PQ_CMP_BALANCE_reg, pqc_pqdc_mc_lf_pq_cmp_balance_reg.regValue);
	rtd_outl(PQC_PQDC_MC_HF_PQ_CMP_BIT_reg, pqc_pqdc_mc_hf_pq_cmp_bit_reg.regValue);
	rtd_outl(PQC_PQDC_MC_HF_PQ_CMP_PAIR_reg, pqc_pqdc_mc_hf_pq_cmp_pair_reg.regValue);
	rtd_outl(PQC_PQDC_MC_HF_PQ_DECMP_PAIR_reg, pqc_pqdc_mc_hf_pq_decmp_pair_reg.regValue);
	rtd_outl(PQC_PQDC_MC_HF_PQ_CMP_POOR_reg, pqc_pqdc_mc_hf_pq_cmp_poor_reg.regValue);
	rtd_outl(PQC_PQDC_MC_HF_PQ_CMP_BALANCE_reg, pqc_pqdc_mc_hf_pq_cmp_balance_reg.regValue);
	rtd_outl(PQC_PQDC_MC_LF_PQ_CMP_ALLOCATE_reg, pqc_pqdc_mc_lf_pq_cmp_allocate_reg.regValue);
	rtd_outl(PQC_PQDC_MC_LF_PQ_CMP_ENABLE_reg, pqc_pqdc_mc_lf_pq_cmp_enable_reg.regValue);
	rtd_outl(PQC_PQDC_MC_HF_PQ_CMP_ALLOCATE_reg, pqc_pqdc_mc_hf_pq_cmp_allocate_reg.regValue);
	rtd_outl(PQC_PQDC_MC_HF_PQ_CMP_ENABLE_reg, pqc_pqdc_mc_hf_pq_cmp_enable_reg.regValue);
	rtd_outl(MC_DMA_MC_WDMA_CTRL0_reg, mc_dma_mc_wdma_ctrl0_reg.regValue);
	rtd_outl(MC_DMA_MC_LF_WDMA_NUM_BL_reg, mc_dma_mc_lf_wdma_num_bl_reg.regValue);
	rtd_outl(MC_DMA_MC_LF_WDMA_LSTEP_reg, mc_dma_mc_lf_wdma_lstep_reg.regValue);
	rtd_outl(MC_DMA_MC_LF_DMA_WR_Ctrl_reg, mc_dma_mc_lf_dma_wr_ctrl_reg.regValue);
	rtd_outl(MC_DMA_MC_HF_WDMA_NUM_BL_reg, mc_dma_mc_hf_wdma_num_bl_reg.regValue);
	rtd_outl(MC_DMA_MC_HF_WDMA_LSTEP_reg, mc_dma_mc_hf_wdma_lstep_reg.regValue);
	rtd_outl(MC_DMA_MC_HF_DMA_WR_Ctrl_reg, mc_dma_mc_hf_dma_wr_ctrl_reg.regValue);
	rtd_outl(MC_DMA_MC_RDMA_CTRL_reg, mc_dma_mc_rdma_ctrl_reg.regValue);
	rtd_outl(MC_DMA_MC_LF_I_RDMA_NUM_BL_reg, mc_dma_mc_lf_i_rdma_num_bl_reg.regValue);
	rtd_outl(MC_DMA_MC_LF_I_DMA_RD_Ctrl_reg, mc_dma_mc_lf_i_dma_rd_ctrl_reg.regValue);
	rtd_outl(MC_DMA_MC_HF_I_RDMA_NUM_BL_reg, mc_dma_mc_hf_i_rdma_num_bl_reg.regValue);
	rtd_outl(MC_DMA_MC_HF_I_DMA_RD_Ctrl_reg, mc_dma_mc_hf_i_dma_rd_ctrl_reg.regValue);
	rtd_outl(MC_DMA_MC_LF_P_RDMA_NUM_BL_reg, mc_dma_mc_lf_p_rdma_num_bl_reg.regValue);
	rtd_outl(MC_DMA_MC_LF_P_DMA_RD_Ctrl_reg, mc_dma_mc_lf_p_dma_rd_ctrl_reg.regValue);
	rtd_outl(MC_DMA_MC_HF_P_RDMA_NUM_BL_reg, mc_dma_mc_hf_p_rdma_num_bl_reg.regValue);
	rtd_outl(MC_DMA_MC_HF_P_DMA_RD_Ctrl_reg, mc_dma_mc_hf_p_dma_rd_ctrl_reg.regValue);
	rtd_outl(MC_DMA_MC_LF_WDMA_MSTART2_reg, mc_dma_mc_lf_wdma_mstart2_reg.regValue);
	rtd_outl(MC_DMA_MC_LF_WDMA_MSTART3_reg, mc_dma_mc_lf_wdma_mstart3_reg.regValue);
	rtd_outl(MC_DMA_MC_LF_WDMA_MSTART4_reg, mc_dma_mc_lf_wdma_mstart4_reg.regValue);
	rtd_outl(MC_DMA_MC_LF_WDMA_MSTART5_reg, mc_dma_mc_lf_wdma_mstart5_reg.regValue);
	rtd_outl(MC_DMA_MC_LF_WDMA_MSTART6_reg, mc_dma_mc_lf_wdma_mstart6_reg.regValue);
	rtd_outl(MC_DMA_MC_LF_WDMA_MSTART7_reg, mc_dma_mc_lf_wdma_mstart7_reg.regValue);
	rtd_outl(MC_DMA_MC_LF_WDMA_MSTART0_reg, mc_dma_mc_lf_wdma_mstart0_reg.regValue);
	rtd_outl(MC_DMA_MC_LF_WDMA_MEND0_reg, mc_dma_mc_lf_wdma_mend0_reg.regValue);
	rtd_outl(MC_DMA_MC_HF_WDMA_MSTART0_reg, mc_dma_mc_hf_wdma_mstart0_reg.regValue);
	rtd_outl(MC_DMA_MC_HF_WDMA_MEND0_reg, mc_dma_mc_hf_wdma_mend0_reg.regValue);
	rtd_outl(MC_DMA_MC_LF_WDMA_MSTART1_reg, mc_dma_mc_lf_wdma_mstart1_reg.regValue);
	rtd_outl(MC_DMA_MC_LF_WDMA_MEND1_reg, mc_dma_mc_lf_wdma_mend1_reg.regValue);
	rtd_outl(MC_DMA_MC_HF_WDMA_MSTART1_reg, mc_dma_mc_hf_wdma_mstart1_reg.regValue);
	rtd_outl(MC_DMA_MC_HF_WDMA_MEND1_reg, mc_dma_mc_hf_wdma_mend1_reg.regValue);
	rtd_outl(MC_DMA_MC_LF_WDMA_MEND2_reg, mc_dma_mc_lf_wdma_mend2_reg.regValue);
	rtd_outl(MC_DMA_MC_HF_WDMA_MSTART2_reg, mc_dma_mc_hf_wdma_mstart2_reg.regValue);
	rtd_outl(MC_DMA_MC_HF_WDMA_MEND2_reg, mc_dma_mc_hf_wdma_mend2_reg.regValue);
	rtd_outl(MC_DMA_MC_LF_WDMA_MEND3_reg, mc_dma_mc_lf_wdma_mend3_reg.regValue);
	rtd_outl(MC_DMA_MC_HF_WDMA_MSTART3_reg, mc_dma_mc_hf_wdma_mstart3_reg.regValue);
	rtd_outl(MC_DMA_MC_HF_WDMA_MEND3_reg, mc_dma_mc_hf_wdma_mend3_reg.regValue);
	rtd_outl(MC_DMA_MC_LF_WDMA_MEND4_reg, mc_dma_mc_lf_wdma_mend4_reg.regValue);
	rtd_outl(MC_DMA_MC_HF_WDMA_MSTART4_reg, mc_dma_mc_hf_wdma_mstart4_reg.regValue);
	rtd_outl(MC_DMA_MC_HF_WDMA_MEND4_reg, mc_dma_mc_hf_wdma_mend4_reg.regValue);
	rtd_outl(MC_DMA_MC_LF_WDMA_MEND5_reg, mc_dma_mc_lf_wdma_mend5_reg.regValue);
	rtd_outl(MC_DMA_MC_HF_WDMA_MSTART5_reg, mc_dma_mc_hf_wdma_mstart5_reg.regValue);
	rtd_outl(MC_DMA_MC_HF_WDMA_MEND5_reg, mc_dma_mc_hf_wdma_mend5_reg.regValue);
	rtd_outl(MC_DMA_MC_LF_WDMA_MEND6_reg, mc_dma_mc_lf_wdma_mend6_reg.regValue);
	rtd_outl(MC_DMA_MC_HF_WDMA_MSTART6_reg, mc_dma_mc_hf_wdma_mstart6_reg.regValue);
	rtd_outl(MC_DMA_MC_HF_WDMA_MEND6_reg, mc_dma_mc_hf_wdma_mend6_reg.regValue);
	rtd_outl(MC_DMA_MC_LF_WDMA_MEND7_reg, mc_dma_mc_lf_wdma_mend7_reg.regValue);
	rtd_outl(MC_DMA_MC_HF_WDMA_MSTART7_reg, mc_dma_mc_hf_wdma_mstart7_reg.regValue);
	rtd_outl(MC_DMA_MC_HF_WDMA_MEND7_reg, mc_dma_mc_hf_wdma_mend7_reg.regValue);
	rtd_outl(KPHASE_kphase_10_reg, kphase_kphase_10_reg.regValue);
	rtd_outl(MC_DMA_MC_LF_DMA_WR_Rule_check_up_reg, mc_dma_mc_lf_dma_wr_rule_check_up_reg.regValue);
	rtd_outl(MC_DMA_MC_LF_I_DMA_RD_Rule_check_up_reg, mc_dma_mc_lf_i_dma_rd_rule_check_up_reg.regValue);
	rtd_outl(MC_DMA_MC_LF_P_DMA_RD_Rule_check_up_reg, mc_dma_mc_lf_p_dma_rd_rule_check_up_reg.regValue);
	rtd_outl(MC_DMA_MC_LF_DMA_WR_Rule_check_low_reg, mc_dma_mc_lf_dma_wr_rule_check_low_reg.regValue);
	rtd_outl(MC_DMA_MC_LF_I_DMA_RD_Rule_check_low_reg, mc_dma_mc_lf_i_dma_rd_rule_check_low_reg.regValue);
	rtd_outl(MC_DMA_MC_LF_P_DMA_RD_Rule_check_low_reg, mc_dma_mc_lf_p_dma_rd_rule_check_low_reg.regValue);
	rtd_outl(MC_DMA_MC_HF_DMA_WR_Rule_check_up_reg, mc_dma_mc_hf_dma_wr_rule_check_up_reg.regValue);
	rtd_outl(MC_DMA_MC_HF_I_DMA_RD_Rule_check_up_reg, mc_dma_mc_hf_i_dma_rd_rule_check_up_reg.regValue);
	rtd_outl(MC_DMA_MC_HF_P_DMA_RD_Rule_check_up_reg, mc_dma_mc_hf_p_dma_rd_rule_check_up_reg.regValue);
	rtd_outl(MC_DMA_MC_HF_DMA_WR_Rule_check_low_reg, mc_dma_mc_hf_dma_wr_rule_check_low_reg.regValue);
	rtd_outl(MC_DMA_MC_HF_I_DMA_RD_Rule_check_low_reg, mc_dma_mc_hf_i_dma_rd_rule_check_low_reg.regValue);
	rtd_outl(MC_DMA_MC_HF_P_DMA_RD_Rule_check_low_reg, mc_dma_mc_hf_p_dma_rd_rule_check_low_reg.regValue);

}
void MEMC_ChangeSize_ME1(void)
{
	// parameter declaration

	// register declaration

	// register read back

	// main function body

	// register write

}
void MEMC_ChangeSize_ME2(void)
{
	// parameter declaration

	// register declaration

	// register read back

	// main function body

	// register write

}
void MEMC_ChangeSize_MEDMA(void)
{
	// parameter declaration
	int image_me_hsize, image_me_vsize, image_me1_blk_hsize, image_me1_blk_vsize;

	int image_me2_blk_ph_hsize;
	int image_me2_blk_ph_vsize;

	int ipme_bit;
	int ipme_line_step;
	int ipme_addr_base;

	int ipme_frame_step;

	int ipme_addr_0_sta;
	int ipme_addr_0_end;
	int ipme_addr_1_sta;
	int ipme_addr_1_end;
	int ipme_addr_2_sta;
	int ipme_addr_2_end;
	int ipme_addr_3_sta;
	int ipme_addr_3_end;
	int ipme_addr_4_sta;
	int ipme_addr_4_end;
	int ipme_addr_5_sta;
	int ipme_addr_5_end;
	int ipme_addr_6_sta;
	int ipme_addr_6_end;
	int ipme_addr_7_sta;
	int ipme_addr_7_end;

	int kme08_logo_iir_bit;
	int kme08_logo_iir_line_step;
	int kme08_logo_iir_frame_step;
	int kme08_logo_iir_addr_0_sta;
	int kme08_logo_iir_addr_0_end;
	int kme08_logo_iir_addr_1_sta;
	int kme08_logo_iir_addr_1_end;

	int kme10_pxl_hsty_bit;
	int kme10_pxl_hsty_line_step;
	int kme10_pxl_hsty_frame_step;
	int kme10_pxl_hsty_addr_0_sta;
	int kme10_pxl_hsty_addr_0_end;
	int kme10_pxl_hsty_addr_1_sta;
	int kme10_pxl_hsty_addr_1_end;

	int kme12_pxl_logo_bit;
	int kme12_pxl_logo_line_step;
	int kme12_pxl_logo_frame_step;
	int kme12_pxl_logo_addr_0_sta;
	int kme12_pxl_logo_addr_0_end;
	int kme12_pxl_logo_addr_1_sta;
	int kme12_pxl_logo_addr_1_end;

	int kme14_blk_hsty_bit;
	int kme14_blk_hsty_line_step;
	int kme14_blk_hsty_frame_step;
	int kme14_blk_hsty_addr_0_sta;
	int kme14_blk_hsty_addr_0_end;
	int kme14_blk_hsty_addr_1_sta;
	int kme14_blk_hsty_addr_1_end;

	int mv01_me2_ph_bit;
	int mv01_me2_ph_line_step;
	int mv01_me2_ph_frame_step;
	int mv01_me2_ph_addr_0_sta;
	int mv01_me2_ph_addr_0_end;

	int mv02_me1_pi_addr_base;
	int mv02_me1_pi_line_step;
	int mv02_me1_pi_bit;

	int mv02_me1_pi_frame_step;
	int mv02_me1_pi_addr_0_sta;
	int mv02_me1_pi_addr_0_end;

	int mv04_me1_ppfv_addr_base;
	int mv04_me1_ppfv_line_step;
	int mv04_me1_ppfv_bit;

	int mv04_me1_ppfv_frame_step;
	int mv04_me1_ppfv_addr_0_sta;
	int mv04_me1_ppfv_addr_0_end;
	int mv04_me1_ppfv_addr_1_sta;
	int mv04_me1_ppfv_addr_1_end;
	int mv04_me1_ppfv_addr_2_sta;
	int mv04_me1_ppfv_addr_2_end;

	int mv05_me1_pfv_addr_base;

	int mv05_me1_pfv_bit;
	int mv05_me1_pfv_line_step;
	int mv05_me1_pfv_frame_step;
	int mv05_me1_pfv_addr_0_sta;
	int mv05_me1_pfv_addr_0_end;
	int mv05_me1_pfv_addr_1_sta;
	int mv05_me1_pfv_addr_1_end;

	int meshr_rpt_pfv_addr_0_base;
	int meshr_rpt_pfv_bit;
	int meshr_rpt_pfv_line_step;
	int meshr_rpt_pfv_frame_step;
	int meshr_rpt_pfv_addr_0_sta;
	int meshr_rpt_pfv_addr_0_end;
	int meshr_rpt_pfv_addr_1_sta;
	int meshr_rpt_pfv_addr_1_end;

	int meshr_rpt_ppfv_bit;
	int meshr_rpt_ppfv_line_step;
	int meshr_rpt_ppfv_frame_step;
	int meshr_rpt_ppfv_addr_0_sta;
	int meshr_rpt_ppfv_addr_0_end;
	int meshr_rpt_ppfv_addr_1_sta;
	int meshr_rpt_ppfv_addr_1_end;
	int meshr_rpt_ppfv_addr_2_sta;
	int meshr_rpt_ppfv_addr_2_end;

	int mvinfo_dma_bit;
	int mvinfo_dma_line_step;

	int mvinfo_dma_frame_step;
	int mvinfo_dma_addr_0_sta = 0;
	int mvinfo_dma_addr_0_end;

	int reg_iplogo_hnum0;
	int reg_mlogo_hnum0;
	int reg_plogo_hnum0;
	int reg_hlogo_hnum0;
	int reg_iplogo_hnum;
	int reg_mlogo_hnum;
	int reg_plogo_hnum;
	int reg_hlogo_hnum;

	int reg_mv01_hnum0;
	int reg_mv02_hnum0;
	int reg_mv04_hnum0;
	int reg_mv05_hnum0;
	int reg_mv01_hnum;
	int reg_mv02_hnum;
	int reg_mv04_hnum;
	int reg_mv05_hnum;

	int me_pqc_width_div32;
	int me_pqc_height;
	int me_pqc_line_sum_bit;
	int me_pqc_ipme_en;
	int me_pqc_logo_en;
	int me_pqc_bpp;
	
#if (defined(MARK2)||defined(MERLIN7))
    int meshr_logoprotect_addr_0_base;
    int meshr_logoprotect_bit;
    int meshr_logoprotect_line_step;
    int meshr_logoprotect_frame_step;
    int meshr_logoprotect_addr_0_sta, meshr_logoprotect_addr_0_end;
    int meshr_logoprotect_addr_1_sta, meshr_logoprotect_addr_1_end;
    int meshr_logoprotect_addr_2_sta, meshr_logoprotect_addr_2_end;
    int meshr_logoprotect_addr_3_sta, meshr_logoprotect_addr_3_end;
    int meshr_logoprotect_addr_4_sta, meshr_logoprotect_addr_4_end;
    int meshr_logoprotect_addr_5_sta, meshr_logoprotect_addr_5_end;
    int meshr_logoprotect_addr_6_sta, meshr_logoprotect_addr_6_end;
    int meshr_logoprotect_addr_7_sta, meshr_logoprotect_addr_7_end;
    int meshr_logoprotect_bl;
#endif

	int mvinfo_line_128;

	// register declaration
	kme_dm_top0_kme_00_start_address0_RBUS kme_dm_top0_kme_00_start_address0_reg;
	kme_dm_top0_kme_00_start_address1_RBUS kme_dm_top0_kme_00_start_address1_reg;
	kme_dm_top0_kme_00_start_address2_RBUS kme_dm_top0_kme_00_start_address2_reg;
	kme_dm_top0_kme_00_start_address3_RBUS kme_dm_top0_kme_00_start_address3_reg;
	kme_dm_top0_kme_00_start_address4_RBUS kme_dm_top0_kme_00_start_address4_reg;
	kme_dm_top0_kme_00_start_address5_RBUS kme_dm_top0_kme_00_start_address5_reg;
	kme_dm_top2_kme_00_start_address6_RBUS kme_dm_top2_kme_00_start_address6_reg;
	kme_dm_top2_kme_00_start_address7_RBUS kme_dm_top2_kme_00_start_address7_reg;
	kme_dm_top0_kme_00_end_address0_RBUS kme_dm_top0_kme_00_end_address0_reg;
	kme_dm_top0_kme_00_end_address1_RBUS kme_dm_top0_kme_00_end_address1_reg;
	kme_dm_top0_kme_00_end_address2_RBUS kme_dm_top0_kme_00_end_address2_reg;
	kme_dm_top0_kme_00_end_address3_RBUS kme_dm_top0_kme_00_end_address3_reg;
	kme_dm_top0_kme_00_end_address4_RBUS kme_dm_top0_kme_00_end_address4_reg;
	kme_dm_top0_kme_00_end_address5_RBUS kme_dm_top0_kme_00_end_address5_reg;
	kme_dm_top2_kme_00_end_address6_RBUS kme_dm_top2_kme_00_end_address6_reg;
	kme_dm_top2_kme_00_end_address7_RBUS kme_dm_top2_kme_00_end_address7_reg;
	kme_dm_top0_kme_00_line_offset_addr_RBUS kme_dm_top0_kme_00_line_offset_addr_reg;
	kme_dm_top2_mv02_start_address11_RBUS kme_dm_top2_mv02_start_address11_reg;
	kme_dm_top2_mv02_end_address0_RBUS kme_dm_top2_mv02_end_address0_reg;
	kme_dm_top2_mv_02_line_offset_addr_RBUS kme_dm_top2_mv_02_line_offset_addr_reg;
	kme_dm_top2_mv05_start_address0_RBUS kme_dm_top2_mv05_start_address0_reg;
	kme_dm_top2_mv05_start_address1_RBUS kme_dm_top2_mv05_start_address1_reg;
	kme_dm_top2_mv05_end_address0_RBUS kme_dm_top2_mv05_end_address0_reg;
	kme_dm_top2_mv05_end_address1_RBUS kme_dm_top2_mv05_end_address1_reg;
	kme_dm_top2_mv_05_line_offset_addr_RBUS kme_dm_top2_mv_05_line_offset_addr_reg;
	kme_dm_top2_mv04_start_address0_RBUS kme_dm_top2_mv04_start_address0_reg;
	kme_dm_top2_mv04_start_address1_RBUS kme_dm_top2_mv04_start_address1_reg;
	kme_dm_top2_mv04_end_address0_RBUS kme_dm_top2_mv04_end_address0_reg;
	kme_dm_top2_mv04_end_address1_RBUS kme_dm_top2_mv04_end_address1_reg;
	kme_dm_top0_mv04_start_address2_RBUS kme_dm_top0_mv04_start_address2_reg;
	kme_dm_top0_mv04_end_address2_RBUS kme_dm_top0_mv04_end_address2_reg;
	kme_dm_top2_mv_04_line_offset_addr_RBUS kme_dm_top2_mv_04_line_offset_addr_reg;
	kme_dm_top1_kme_dm_top1_4c_RBUS kme_dm_top1_kme_dm_top1_4c_reg;
	kme_dm_top1_kme_dm_top1_50_RBUS kme_dm_top1_kme_dm_top1_50_reg;
	mvinfo_dma_mvinfo_wdma_lstep_RBUS mvinfo_dma_mvinfo_wdma_lstep_reg;
	kme_dm_top1_kme_dm_top1_00_RBUS kme_dm_top1_kme_dm_top1_00_reg;
	kme_dm_top1_kme_dm_top1_04_RBUS kme_dm_top1_kme_dm_top1_04_reg;
	kme_dm_top1_kme_dm_top1_08_RBUS kme_dm_top1_kme_dm_top1_08_reg;
	kme_dm_top1_kme_dm_top1_0c_RBUS kme_dm_top1_kme_dm_top1_0c_reg;
	kme_dm_top1_kme_dm_top1_10_RBUS kme_dm_top1_kme_dm_top1_10_reg;
	kme_dm_top1_kme_dm_top1_24_RBUS kme_dm_top1_kme_dm_top1_24_reg;
	kme_dm_top1_kme_dm_top1_28_RBUS kme_dm_top1_kme_dm_top1_28_reg;
	kme_dm_top1_kme_dm_top1_2c_RBUS kme_dm_top1_kme_dm_top1_2c_reg;
	kme_dm_top1_kme_dm_top1_30_RBUS kme_dm_top1_kme_dm_top1_30_reg;
	kme_dm_top1_kme_dm_top1_34_RBUS kme_dm_top1_kme_dm_top1_34_reg;
	kme_dm_top1_kme_dm_top1_48_RBUS kme_dm_top1_kme_dm_top1_48_reg;
	kme_dm_top1_kme_dm_top1_54_RBUS kme_dm_top1_kme_dm_top1_54_reg;
	kme_dm_top1_kme_dm_top1_58_RBUS kme_dm_top1_kme_dm_top1_58_reg;
	kme_dm_top1_kme_dm_top1_6c_RBUS kme_dm_top1_kme_dm_top1_6c_reg;
	kme_dm_top1_kme_dm_top1_70_RBUS kme_dm_top1_kme_dm_top1_70_reg;
	kme_dm_top1_kme_dm_top1_74_RBUS kme_dm_top1_kme_dm_top1_74_reg;
	kme_dm_top1_kme_dm_top1_78_RBUS kme_dm_top1_kme_dm_top1_78_reg;
	kme_dm_top1_kme_dm_top1_7c_RBUS kme_dm_top1_kme_dm_top1_7c_reg;
	kme_dm_top2_mv01_start_address0_RBUS kme_dm_top2_mv01_start_address0_reg;
	kme_dm_top2_mv01_end_address0_RBUS kme_dm_top2_mv01_end_address0_reg;
	kme_dm_top2_mv_01_line_offset_addr_RBUS kme_dm_top2_mv_01_line_offset_addr_reg;
	dbus_wrapper_me_uplimit_addr_RBUS dbus_wrapper_me_uplimit_addr_reg;
	dbus_wrapper_me_downlimit_addr_RBUS dbus_wrapper_me_downlimit_addr_reg;
	dbus_wrapper_memc_dcu1_latcnt_cr11_RBUS dbus_wrapper_memc_dcu1_latcnt_cr11_reg;
	dbus_wrapper_memc_dcu1_latcnt_cr12_RBUS dbus_wrapper_memc_dcu1_latcnt_cr12_reg;
	me_share_dma_me1_wdma0_lstep_RBUS me_share_dma_me1_wdma0_lstep_reg;
	me_share_dma_me1_wdma0_mstart0_RBUS me_share_dma_me1_wdma0_mstart0_reg;
	me_share_dma_me1_wdma0_mstart1_RBUS me_share_dma_me1_wdma0_mstart1_reg;
	me_share_dma_me1_wdma0_mend0_RBUS me_share_dma_me1_wdma0_mend0_reg;
	me_share_dma_me1_wdma0_mend1_RBUS me_share_dma_me1_wdma0_mend1_reg;
	me_share_dma_me1_wdma1_lstep_RBUS me_share_dma_me1_wdma1_lstep_reg;
	me_share_dma_me1_wdma1_mstart0_RBUS me_share_dma_me1_wdma1_mstart0_reg;
	me_share_dma_me1_wdma1_mstart1_RBUS me_share_dma_me1_wdma1_mstart1_reg;
	me_share_dma_me1_wdma1_mstart2_RBUS me_share_dma_me1_wdma1_mstart2_reg;
	me_share_dma_me1_wdma1_mend0_RBUS me_share_dma_me1_wdma1_mend0_reg;
	me_share_dma_me1_wdma1_mend1_RBUS me_share_dma_me1_wdma1_mend1_reg;
	me_share_dma_me1_wdma1_mend2_RBUS me_share_dma_me1_wdma1_mend2_reg;
	me_share_dma_ipme_wdma_ctrl_RBUS me_share_dma_ipme_wdma_ctrl_reg;
	me_share_dma_me1_rdma0_ctrl_RBUS me_share_dma_me1_rdma0_ctrl_reg;
	me_share_dma_ipme_wdma_num_bl_RBUS me_share_dma_ipme_wdma_num_bl_reg;
	me_share_dma_me1_rdma0_num_bl_RBUS me_share_dma_me1_rdma0_num_bl_reg;
	me_share_dma_ipme_wdma_lstep_RBUS me_share_dma_ipme_wdma_lstep_reg;
	me_share_dma_ipme_wdma_mstart0_RBUS me_share_dma_ipme_wdma_mstart0_reg;
	me_share_dma_ipme_wdma_mstart1_RBUS me_share_dma_ipme_wdma_mstart1_reg;
	me_share_dma_ipme_wdma_mstart2_RBUS me_share_dma_ipme_wdma_mstart2_reg;
	me_share_dma_ipme_wdma_mstart3_RBUS me_share_dma_ipme_wdma_mstart3_reg;
	me_share_dma_ipme_wdma_mstart4_RBUS me_share_dma_ipme_wdma_mstart4_reg;
	me_share_dma_ipme_wdma_mstart5_RBUS me_share_dma_ipme_wdma_mstart5_reg;
	me_share_dma_ipme_wdma_mstart6_RBUS me_share_dma_ipme_wdma_mstart6_reg;
	me_share_dma_ipme_wdma_mstart7_RBUS me_share_dma_ipme_wdma_mstart7_reg;
	me_share_dma_ipme_wdma_mend0_RBUS me_share_dma_ipme_wdma_mend0_reg;
	me_share_dma_ipme_wdma_mend1_RBUS me_share_dma_ipme_wdma_mend1_reg;
	me_share_dma_ipme_wdma_mend2_RBUS me_share_dma_ipme_wdma_mend2_reg;
	me_share_dma_ipme_wdma_mend3_RBUS me_share_dma_ipme_wdma_mend3_reg;
	me_share_dma_ipme_wdma_mend4_RBUS me_share_dma_ipme_wdma_mend4_reg;
	me_share_dma_ipme_wdma_mend5_RBUS me_share_dma_ipme_wdma_mend5_reg;
	me_share_dma_ipme_wdma_mend6_RBUS me_share_dma_ipme_wdma_mend6_reg;
	me_share_dma_ipme_wdma_mend7_RBUS me_share_dma_ipme_wdma_mend7_reg;
	me_share_dma_me_dma_wr_rule_check_up_RBUS me_share_dma_me_dma_wr_rule_check_up_reg;
	me_share_dma_me_dma_wr_rule_check_low_RBUS me_share_dma_me_dma_wr_rule_check_low_reg;
	me_share_dma_me_dma_rd_rule_check_up_RBUS me_share_dma_me_dma_rd_rule_check_up_reg;
	me_share_dma_me_dma_rd_rule_check_low_RBUS me_share_dma_me_dma_rd_rule_check_low_reg;
	mvinfo_dma_mvinfo_wdma_mstart0_RBUS mvinfo_dma_mvinfo_wdma_mstart0_reg;
	mvinfo_dma_mvinfo_wdma_mend0_RBUS mvinfo_dma_mvinfo_wdma_mend0_reg;
	mvinfo_dma_mvinfo_wr_rule_check_up_RBUS mvinfo_dma_mvinfo_wr_rule_check_up_reg;
	mvinfo_dma_mvinfo_wr_rule_check_low_RBUS mvinfo_dma_mvinfo_wr_rule_check_low_reg;
	mvinfo_dma_mvinfo_rd_rule_check_up_RBUS mvinfo_dma_mvinfo_rd_rule_check_up_reg;
	mvinfo_dma_mvinfo_rd_rule_check_low_RBUS mvinfo_dma_mvinfo_rd_rule_check_low_reg;
	kme_dm_top2_mv_source_mode_RBUS kme_dm_top2_mv_source_mode_reg;
	kme_dm_top0_kme_me_hnum_RBUS kme_dm_top0_kme_me_hnum_reg;
	kme_dm_top1_kme_dm_top1_a0_RBUS kme_dm_top1_kme_dm_top1_a0_reg;
	kme_dm_top2_mv01_agent_RBUS kme_dm_top2_mv01_agent_reg;
	kme_dm_top2_mv02_agent_RBUS kme_dm_top2_mv02_agent_reg;
	kme_dm_top2_mv04_agent_RBUS kme_dm_top2_mv04_agent_reg;
	kme_dm_top2_mv05_agent_RBUS kme_dm_top2_mv05_agent_reg;
	mvinfo_dma_mvinfo_wdma_num_bl_RBUS mvinfo_dma_mvinfo_wdma_num_bl_reg;
	mvinfo_dma_mvinfo_rdma_num_bl_RBUS mvinfo_dma_mvinfo_rdma_num_bl_reg;
	kme_pqc_pqdc_kme_pq_cmp_RBUS kme_pqc_pqdc_kme_pq_cmp_reg;
	kme_pqc_pqdc_kme_pq_decmp_RBUS kme_pqc_pqdc_kme_pq_decmp_reg;
	kme_pqc_pqdc_kme_pq_cmp_bit_RBUS kme_pqc_pqdc_kme_pq_cmp_bit_reg;
	kme_pqc_pqdc_kme_pq_cmp_pair_RBUS kme_pqc_pqdc_kme_pq_cmp_pair_reg;
	kme_pqc_pqdc_kme_pq_decmp_pair_RBUS kme_pqc_pqdc_kme_pq_decmp_pair_reg;
	kme_dm_top0_kme_00_agent_RBUS kme_dm_top0_kme_00_agent_reg;
	kme_dm_top0_kme_01_agent_RBUS kme_dm_top0_kme_01_agent_reg;
	kme_dm_top0_kme_02_agent_RBUS kme_dm_top0_kme_02_agent_reg;
	kme_dm_top0_kme_03_agent_RBUS kme_dm_top0_kme_03_agent_reg;
	kme_dm_top0_kme_04_agent_RBUS kme_dm_top0_kme_04_agent_reg;
	kme_dm_top0_kme_05_agent_RBUS kme_dm_top0_kme_05_agent_reg;
	kme_dm_top0_kme06agent_RBUS kme_dm_top0_kme06agent_reg;
	kme_dm_top0_kme_07_agent_RBUS kme_dm_top0_kme_07_agent_reg;
	kme_dm_top1_kme_dm_top1_14_RBUS kme_dm_top1_kme_dm_top1_14_reg;
	kme_dm_top1_kme_dm_top1_1c_RBUS kme_dm_top1_kme_dm_top1_1c_reg;
	kme_dm_top1_kme_dm_top1_38_RBUS kme_dm_top1_kme_dm_top1_38_reg;
	kme_dm_top1_kme_dm_top1_40_RBUS kme_dm_top1_kme_dm_top1_40_reg;
	kme_dm_top1_kme_dm_top1_5c_RBUS kme_dm_top1_kme_dm_top1_5c_reg;
	kme_dm_top1_kme_dm_top1_64_RBUS kme_dm_top1_kme_dm_top1_64_reg;
	kme_dm_top1_kme_dm_top1_80_RBUS kme_dm_top1_kme_dm_top1_80_reg;
	kme_dm_top1_kme_dm_top1_88_RBUS kme_dm_top1_kme_dm_top1_88_reg;
	kme_dm_top2_mv03_agent_RBUS kme_dm_top2_mv03_agent_reg;
	kme_dm_top2_mv06_agent_RBUS kme_dm_top2_mv06_agent_reg;
	kme_dm_top2_mv07_agent_RBUS kme_dm_top2_mv07_agent_reg;
	kme_dm_top2_mv08_agent_RBUS kme_dm_top2_mv08_agent_reg;
	kme_dm_top2_mv09_agent_RBUS kme_dm_top2_mv09_agent_reg;
	kme_dm_top2_mv10_agent_RBUS kme_dm_top2_mv10_agent_reg;
	kme_dm_top2_mv11_agent_RBUS kme_dm_top2_mv11_agent_reg;
	kme_dm_top2_mv12_agent_RBUS kme_dm_top2_mv12_agent_reg;
	kme_dm_top0_kme_me_resolution_RBUS kme_dm_top0_kme_me_resolution_reg;
	kme_dm_top0_kme_mv_resolution_RBUS kme_dm_top0_kme_mv_resolution_reg;
	kme_dm_top1_kme_dm_top1_90_RBUS kme_dm_top1_kme_dm_top1_90_reg;
	kme_dm_top1_kme_dm_top1_94_RBUS kme_dm_top1_kme_dm_top1_94_reg;
	kme_dm_top1_kme_dm_top1_98_RBUS kme_dm_top1_kme_dm_top1_98_reg;
	kme_dm_top1_kme_dm_top1_9c_RBUS kme_dm_top1_kme_dm_top1_9c_reg;
	kme_dm_top2_mv02_resolution_RBUS kme_dm_top2_mv02_resolution_reg;
	kme_dm_top2_mv01_resolution_RBUS kme_dm_top2_mv01_resolution_reg;
	kme_dm_top2_mv04_resolution_RBUS kme_dm_top2_mv04_resolution_reg;
	kme_dm_top2_mv05_resolution_RBUS kme_dm_top2_mv05_resolution_reg;
	mvinfo_dma_mvinfo_wdma_ctrl_RBUS mvinfo_dma_mvinfo_wdma_ctrl_reg;
	mvinfo_dma_mvinfo_rdma_ctrl_RBUS mvinfo_dma_mvinfo_rdma_ctrl_reg;
	kme_pqc_pqdc_kme_pq_cmp_enable_RBUS kme_pqc_pqdc_kme_pq_cmp_enable_reg;

	// register read back
	kme_dm_top0_kme_00_start_address0_reg.regValue = rtd_inl(KME_DM_TOP0_KME_00_START_ADDRESS0_reg);
	kme_dm_top0_kme_00_start_address1_reg.regValue = rtd_inl(KME_DM_TOP0_KME_00_START_ADDRESS1_reg);
	kme_dm_top0_kme_00_start_address2_reg.regValue = rtd_inl(KME_DM_TOP0_KME_00_START_ADDRESS2_reg);
	kme_dm_top0_kme_00_start_address3_reg.regValue = rtd_inl(KME_DM_TOP0_KME_00_START_ADDRESS3_reg);
	kme_dm_top0_kme_00_start_address4_reg.regValue = rtd_inl(KME_DM_TOP0_KME_00_START_ADDRESS4_reg);
	kme_dm_top0_kme_00_start_address5_reg.regValue = rtd_inl(KME_DM_TOP0_KME_00_START_ADDRESS5_reg);
	kme_dm_top2_kme_00_start_address6_reg.regValue = rtd_inl(KME_DM_TOP2_KME_00_START_ADDRESS6_reg);
	kme_dm_top2_kme_00_start_address7_reg.regValue = rtd_inl(KME_DM_TOP2_KME_00_START_ADDRESS7_reg);
	kme_dm_top0_kme_00_end_address0_reg.regValue = rtd_inl(KME_DM_TOP0_KME_00_END_ADDRESS0_reg);
	kme_dm_top0_kme_00_end_address1_reg.regValue = rtd_inl(KME_DM_TOP0_KME_00_END_ADDRESS1_reg);
	kme_dm_top0_kme_00_end_address2_reg.regValue = rtd_inl(KME_DM_TOP0_KME_00_END_ADDRESS2_reg);
	kme_dm_top0_kme_00_end_address3_reg.regValue = rtd_inl(KME_DM_TOP0_KME_00_END_ADDRESS3_reg);
	kme_dm_top0_kme_00_end_address4_reg.regValue = rtd_inl(KME_DM_TOP0_KME_00_END_ADDRESS4_reg);
	kme_dm_top0_kme_00_end_address5_reg.regValue = rtd_inl(KME_DM_TOP0_KME_00_END_ADDRESS5_reg);
	kme_dm_top2_kme_00_end_address6_reg.regValue = rtd_inl(KME_DM_TOP2_KME_00_END_ADDRESS6_reg);
	kme_dm_top2_kme_00_end_address7_reg.regValue = rtd_inl(KME_DM_TOP2_KME_00_END_ADDRESS7_reg);
	kme_dm_top0_kme_00_line_offset_addr_reg.regValue = rtd_inl(KME_DM_TOP0_KME_00_LINE_OFFSET_ADDR_reg);
	kme_dm_top2_mv02_start_address11_reg.regValue = rtd_inl(KME_DM_TOP2_MV02_START_ADDRESS11_reg);
	kme_dm_top2_mv02_end_address0_reg.regValue = rtd_inl(KME_DM_TOP2_MV02_END_ADDRESS0_reg);
	kme_dm_top2_mv_02_line_offset_addr_reg.regValue = rtd_inl(KME_DM_TOP2_MV_02_LINE_OFFSET_ADDR_reg);
	kme_dm_top2_mv05_start_address0_reg.regValue = rtd_inl(KME_DM_TOP2_MV05_START_ADDRESS0_reg);
	kme_dm_top2_mv05_start_address1_reg.regValue = rtd_inl(KME_DM_TOP2_MV05_START_ADDRESS1_reg);
	kme_dm_top2_mv05_end_address0_reg.regValue = rtd_inl(KME_DM_TOP2_MV05_END_ADDRESS0_reg);
	kme_dm_top2_mv05_end_address1_reg.regValue = rtd_inl(KME_DM_TOP2_MV05_END_ADDRESS1_reg);
	kme_dm_top2_mv_05_line_offset_addr_reg.regValue = rtd_inl(KME_DM_TOP2_MV_05_LINE_OFFSET_ADDR_reg);
	kme_dm_top2_mv04_start_address0_reg.regValue = rtd_inl(KME_DM_TOP2_MV04_START_ADDRESS0_reg);
	kme_dm_top2_mv04_start_address1_reg.regValue = rtd_inl(KME_DM_TOP2_MV04_START_ADDRESS1_reg);
	kme_dm_top2_mv04_end_address0_reg.regValue = rtd_inl(KME_DM_TOP2_MV04_END_ADDRESS0_reg);
	kme_dm_top2_mv04_end_address1_reg.regValue = rtd_inl(KME_DM_TOP2_MV04_END_ADDRESS1_reg);
	kme_dm_top0_mv04_start_address2_reg.regValue = rtd_inl(KME_DM_TOP0_MV04_START_ADDRESS2_reg);
	kme_dm_top0_mv04_end_address2_reg.regValue = rtd_inl(KME_DM_TOP0_MV04_END_ADDRESS2_reg);
	kme_dm_top2_mv_04_line_offset_addr_reg.regValue = rtd_inl(KME_DM_TOP2_MV_04_LINE_OFFSET_ADDR_reg);
	kme_dm_top1_kme_dm_top1_4c_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_4C_reg);
	kme_dm_top1_kme_dm_top1_50_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_50_reg);
	mvinfo_dma_mvinfo_wdma_lstep_reg.regValue = rtd_inl(MVINFO_DMA_MVINFO_WDMA_LSTEP_reg);
	kme_dm_top1_kme_dm_top1_00_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_00_reg);
	kme_dm_top1_kme_dm_top1_04_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_04_reg);
	kme_dm_top1_kme_dm_top1_08_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_08_reg);
	kme_dm_top1_kme_dm_top1_0c_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_0C_reg);
	kme_dm_top1_kme_dm_top1_10_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_10_reg);
	kme_dm_top1_kme_dm_top1_24_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_24_reg);
	kme_dm_top1_kme_dm_top1_28_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_28_reg);
	kme_dm_top1_kme_dm_top1_2c_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_2C_reg);
	kme_dm_top1_kme_dm_top1_30_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_30_reg);
	kme_dm_top1_kme_dm_top1_34_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_34_reg);
	kme_dm_top1_kme_dm_top1_48_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_48_reg);
	kme_dm_top1_kme_dm_top1_54_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_54_reg);
	kme_dm_top1_kme_dm_top1_58_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_58_reg);
	kme_dm_top1_kme_dm_top1_6c_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_6C_reg);
	kme_dm_top1_kme_dm_top1_70_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_70_reg);
	kme_dm_top1_kme_dm_top1_74_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_74_reg);
	kme_dm_top1_kme_dm_top1_78_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_78_reg);
	kme_dm_top1_kme_dm_top1_7c_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_7C_reg);
	kme_dm_top2_mv01_start_address0_reg.regValue = rtd_inl(KME_DM_TOP2_MV01_START_ADDRESS0_reg);
	kme_dm_top2_mv01_end_address0_reg.regValue = rtd_inl(KME_DM_TOP2_MV01_END_ADDRESS0_reg);
	kme_dm_top2_mv_01_line_offset_addr_reg.regValue = rtd_inl(KME_DM_TOP2_MV_01_LINE_OFFSET_ADDR_reg);
	dbus_wrapper_me_uplimit_addr_reg.regValue = rtd_inl(DBUS_WRAPPER_Me_uplimit_addr_reg);
	dbus_wrapper_me_downlimit_addr_reg.regValue = rtd_inl(DBUS_WRAPPER_Me_downlimit_addr_reg);
	dbus_wrapper_memc_dcu1_latcnt_cr11_reg.regValue = rtd_inl(DBUS_WRAPPER_MEMC_DCU1_LATCNT_CR11_reg);
	dbus_wrapper_memc_dcu1_latcnt_cr12_reg.regValue = rtd_inl(DBUS_WRAPPER_MEMC_DCU1_LATCNT_CR12_reg);
	me_share_dma_me1_wdma0_lstep_reg.regValue = rtd_inl(ME_SHARE_DMA_ME1_WDMA0_LSTEP_reg);
	me_share_dma_me1_wdma0_mstart0_reg.regValue = rtd_inl(ME_SHARE_DMA_ME1_WDMA0_MSTART0_reg);
	me_share_dma_me1_wdma0_mstart1_reg.regValue = rtd_inl(ME_SHARE_DMA_ME1_WDMA0_MSTART1_reg);
	me_share_dma_me1_wdma0_mend0_reg.regValue = rtd_inl(ME_SHARE_DMA_ME1_WDMA0_MEND0_reg);
	me_share_dma_me1_wdma0_mend1_reg.regValue = rtd_inl(ME_SHARE_DMA_ME1_WDMA0_MEND1_reg);
	me_share_dma_me1_wdma1_lstep_reg.regValue = rtd_inl(ME_SHARE_DMA_ME1_WDMA1_LSTEP_reg);
	me_share_dma_me1_wdma1_mstart0_reg.regValue = rtd_inl(ME_SHARE_DMA_ME1_WDMA1_MSTART0_reg);
	me_share_dma_me1_wdma1_mstart1_reg.regValue = rtd_inl(ME_SHARE_DMA_ME1_WDMA1_MSTART1_reg);
	me_share_dma_me1_wdma1_mstart2_reg.regValue = rtd_inl(ME_SHARE_DMA_ME1_WDMA1_MSTART2_reg);
	me_share_dma_me1_wdma1_mend0_reg.regValue = rtd_inl(ME_SHARE_DMA_ME1_WDMA1_MEND0_reg);
	me_share_dma_me1_wdma1_mend1_reg.regValue = rtd_inl(ME_SHARE_DMA_ME1_WDMA1_MEND1_reg);
	me_share_dma_me1_wdma1_mend2_reg.regValue = rtd_inl(ME_SHARE_DMA_ME1_WDMA1_MEND2_reg);
	me_share_dma_ipme_wdma_ctrl_reg.regValue = rtd_inl(ME_SHARE_DMA_IPME_WDMA_CTRL_reg);
	me_share_dma_me1_rdma0_ctrl_reg.regValue = rtd_inl(ME_SHARE_DMA_ME1_RDMA0_CTRL_reg);
	me_share_dma_ipme_wdma_num_bl_reg.regValue = rtd_inl(ME_SHARE_DMA_IPME_WDMA_NUM_BL_reg);
	me_share_dma_me1_rdma0_num_bl_reg.regValue = rtd_inl(ME_SHARE_DMA_ME1_RDMA0_NUM_BL_reg);
	me_share_dma_ipme_wdma_lstep_reg.regValue = rtd_inl(ME_SHARE_DMA_IPME_WDMA_LSTEP_reg);
	me_share_dma_ipme_wdma_mstart0_reg.regValue = rtd_inl(ME_SHARE_DMA_IPME_WDMA_MSTART0_reg);
	me_share_dma_ipme_wdma_mstart1_reg.regValue = rtd_inl(ME_SHARE_DMA_IPME_WDMA_MSTART1_reg);
	me_share_dma_ipme_wdma_mstart2_reg.regValue = rtd_inl(ME_SHARE_DMA_IPME_WDMA_MSTART2_reg);
	me_share_dma_ipme_wdma_mstart3_reg.regValue = rtd_inl(ME_SHARE_DMA_IPME_WDMA_MSTART3_reg);
	me_share_dma_ipme_wdma_mstart4_reg.regValue = rtd_inl(ME_SHARE_DMA_IPME_WDMA_MSTART4_reg);
	me_share_dma_ipme_wdma_mstart5_reg.regValue = rtd_inl(ME_SHARE_DMA_IPME_WDMA_MSTART5_reg);
	me_share_dma_ipme_wdma_mstart6_reg.regValue = rtd_inl(ME_SHARE_DMA_IPME_WDMA_MSTART6_reg);
	me_share_dma_ipme_wdma_mstart7_reg.regValue = rtd_inl(ME_SHARE_DMA_IPME_WDMA_MSTART7_reg);
	me_share_dma_ipme_wdma_mend0_reg.regValue = rtd_inl(ME_SHARE_DMA_IPME_WDMA_MEND0_reg);
	me_share_dma_ipme_wdma_mend1_reg.regValue = rtd_inl(ME_SHARE_DMA_IPME_WDMA_MEND1_reg);
	me_share_dma_ipme_wdma_mend2_reg.regValue = rtd_inl(ME_SHARE_DMA_IPME_WDMA_MEND2_reg);
	me_share_dma_ipme_wdma_mend3_reg.regValue = rtd_inl(ME_SHARE_DMA_IPME_WDMA_MEND3_reg);
	me_share_dma_ipme_wdma_mend4_reg.regValue = rtd_inl(ME_SHARE_DMA_IPME_WDMA_MEND4_reg);
	me_share_dma_ipme_wdma_mend5_reg.regValue = rtd_inl(ME_SHARE_DMA_IPME_WDMA_MEND5_reg);
	me_share_dma_ipme_wdma_mend6_reg.regValue = rtd_inl(ME_SHARE_DMA_IPME_WDMA_MEND6_reg);
	me_share_dma_ipme_wdma_mend7_reg.regValue = rtd_inl(ME_SHARE_DMA_IPME_WDMA_MEND7_reg);
	me_share_dma_me_dma_wr_rule_check_up_reg.regValue = rtd_inl(ME_SHARE_DMA_ME_DMA_WR_Rule_check_up_reg);
	me_share_dma_me_dma_wr_rule_check_low_reg.regValue = rtd_inl(ME_SHARE_DMA_ME_DMA_WR_Rule_check_low_reg);
	me_share_dma_me_dma_rd_rule_check_up_reg.regValue = rtd_inl(ME_SHARE_DMA_ME_DMA_RD_Rule_check_up_reg);
	me_share_dma_me_dma_rd_rule_check_low_reg.regValue = rtd_inl(ME_SHARE_DMA_ME_DMA_RD_Rule_check_low_reg);
	mvinfo_dma_mvinfo_wdma_mstart0_reg.regValue = rtd_inl(MVINFO_DMA_MVINFO_WDMA_MSTART0_reg);
	mvinfo_dma_mvinfo_wdma_mend0_reg.regValue = rtd_inl(MVINFO_DMA_MVINFO_WDMA_MEND0_reg);
	mvinfo_dma_mvinfo_wr_rule_check_up_reg.regValue = rtd_inl(MVINFO_DMA_MVINFO_WR_Rule_check_up_reg);
	mvinfo_dma_mvinfo_wr_rule_check_low_reg.regValue = rtd_inl(MVINFO_DMA_MVINFO_WR_Rule_check_low_reg);
	mvinfo_dma_mvinfo_rd_rule_check_up_reg.regValue = rtd_inl(MVINFO_DMA_MVINFO_RD_Rule_check_up_reg);
	mvinfo_dma_mvinfo_rd_rule_check_low_reg.regValue = rtd_inl(MVINFO_DMA_MVINFO_RD_Rule_check_low_reg);
	kme_dm_top2_mv_source_mode_reg.regValue = rtd_inl(KME_DM_TOP2_MV_SOURCE_MODE_reg);
	kme_dm_top0_kme_me_hnum_reg.regValue = rtd_inl(KME_DM_TOP0_KME_ME_HNUM_reg);
	kme_dm_top1_kme_dm_top1_a0_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_A0_reg);
	kme_dm_top2_mv01_agent_reg.regValue = rtd_inl(KME_DM_TOP2_MV01_AGENT_reg);
	kme_dm_top2_mv02_agent_reg.regValue = rtd_inl(KME_DM_TOP2_MV02_AGENT_reg);
	kme_dm_top2_mv04_agent_reg.regValue = rtd_inl(KME_DM_TOP2_MV04_AGENT_reg);
	kme_dm_top2_mv05_agent_reg.regValue = rtd_inl(KME_DM_TOP2_MV05_AGENT_reg);
	mvinfo_dma_mvinfo_wdma_num_bl_reg.regValue = rtd_inl(MVINFO_DMA_MVINFO_WDMA_NUM_BL_reg);
	mvinfo_dma_mvinfo_rdma_num_bl_reg.regValue = rtd_inl(MVINFO_DMA_MVINFO_RDMA_NUM_BL_reg);
	kme_pqc_pqdc_kme_pq_cmp_reg.regValue = rtd_inl(KME_PQC_PQDC_KME_PQ_CMP_reg);
	kme_pqc_pqdc_kme_pq_decmp_reg.regValue = rtd_inl(KME_PQC_PQDC_KME_PQ_DECMP_reg);
	kme_pqc_pqdc_kme_pq_cmp_bit_reg.regValue = rtd_inl(KME_PQC_PQDC_KME_PQ_CMP_BIT_reg);
	kme_pqc_pqdc_kme_pq_cmp_pair_reg.regValue = rtd_inl(KME_PQC_PQDC_KME_PQ_CMP_PAIR_reg);
	kme_pqc_pqdc_kme_pq_decmp_pair_reg.regValue = rtd_inl(KME_PQC_PQDC_KME_PQ_DECMP_PAIR_reg);
	kme_dm_top0_kme_00_agent_reg.regValue = rtd_inl(KME_DM_TOP0_KME_00_AGENT_reg);
	kme_dm_top0_kme_01_agent_reg.regValue = rtd_inl(KME_DM_TOP0_KME_01_AGENT_reg);
	kme_dm_top0_kme_02_agent_reg.regValue = rtd_inl(KME_DM_TOP0_KME_02_AGENT_reg);
	kme_dm_top0_kme_03_agent_reg.regValue = rtd_inl(KME_DM_TOP0_KME_03_AGENT_reg);
	kme_dm_top0_kme_04_agent_reg.regValue = rtd_inl(KME_DM_TOP0_KME_04_AGENT_reg);
	kme_dm_top0_kme_05_agent_reg.regValue = rtd_inl(KME_DM_TOP0_KME_05_AGENT_reg);
	kme_dm_top0_kme06agent_reg.regValue = rtd_inl(KME_DM_TOP0_KME06AGENT_reg);
	kme_dm_top0_kme_07_agent_reg.regValue = rtd_inl(KME_DM_TOP0_KME_07_AGENT_reg);
	kme_dm_top1_kme_dm_top1_14_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_14_reg);
	kme_dm_top1_kme_dm_top1_1c_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_1C_reg);
	kme_dm_top1_kme_dm_top1_38_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_38_reg);
	kme_dm_top1_kme_dm_top1_40_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_40_reg);
	kme_dm_top1_kme_dm_top1_5c_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_5C_reg);
	kme_dm_top1_kme_dm_top1_64_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_64_reg);
	kme_dm_top1_kme_dm_top1_80_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_80_reg);
	kme_dm_top1_kme_dm_top1_88_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_88_reg);
	kme_dm_top2_mv03_agent_reg.regValue = rtd_inl(KME_DM_TOP2_MV03_AGENT_reg);
	kme_dm_top2_mv06_agent_reg.regValue = rtd_inl(KME_DM_TOP2_MV06_AGENT_reg);
	kme_dm_top2_mv07_agent_reg.regValue = rtd_inl(KME_DM_TOP2_MV07_AGENT_reg);
	kme_dm_top2_mv08_agent_reg.regValue = rtd_inl(KME_DM_TOP2_MV08_AGENT_reg);
	kme_dm_top2_mv09_agent_reg.regValue = rtd_inl(KME_DM_TOP2_MV09_AGENT_reg);
	kme_dm_top2_mv10_agent_reg.regValue = rtd_inl(KME_DM_TOP2_MV10_AGENT_reg);
	kme_dm_top2_mv11_agent_reg.regValue = rtd_inl(KME_DM_TOP2_MV11_AGENT_reg);
	kme_dm_top2_mv12_agent_reg.regValue = rtd_inl(KME_DM_TOP2_MV12_AGENT_reg);
	kme_dm_top0_kme_me_resolution_reg.regValue = rtd_inl(KME_DM_TOP0_KME_ME_RESOLUTION_reg);
	kme_dm_top0_kme_mv_resolution_reg.regValue = rtd_inl(KME_DM_TOP0_KME_MV_RESOLUTION_reg);
	kme_dm_top1_kme_dm_top1_90_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_90_reg);
	kme_dm_top1_kme_dm_top1_94_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_94_reg);
	kme_dm_top1_kme_dm_top1_98_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_98_reg);
	kme_dm_top1_kme_dm_top1_9c_reg.regValue = rtd_inl(KME_DM_TOP1_KME_DM_TOP1_9C_reg);
	kme_dm_top2_mv02_resolution_reg.regValue = rtd_inl(KME_DM_TOP2_MV02_RESOLUTION_reg);
	kme_dm_top2_mv01_resolution_reg.regValue = rtd_inl(KME_DM_TOP2_MV01_RESOLUTION_reg);
	kme_dm_top2_mv04_resolution_reg.regValue = rtd_inl(KME_DM_TOP2_MV04_RESOLUTION_reg);
	kme_dm_top2_mv05_resolution_reg.regValue = rtd_inl(KME_DM_TOP2_MV05_RESOLUTION_reg);
	mvinfo_dma_mvinfo_wdma_ctrl_reg.regValue = rtd_inl(MVINFO_DMA_MVINFO_WDMA_CTRL_reg);
	mvinfo_dma_mvinfo_rdma_ctrl_reg.regValue = rtd_inl(MVINFO_DMA_MVINFO_RDMA_CTRL_reg);
	kme_pqc_pqdc_kme_pq_cmp_enable_reg.regValue = rtd_inl(KME_PQC_PQDC_KME_PQ_CMP_ENABLE_reg);

	// main function body
	if(GS_ipme_dn_ratio == 8){
		image_me_hsize = (GS_image_hsize / 8 );
		image_me_vsize = (GS_image_vsize / 8 );
	} 
	else if(GS_ipme_dn_ratio == 4){
		image_me_hsize = (GS_image_hsize / 4 );
		image_me_vsize = (GS_image_vsize / 4 );
	} else {
		image_me_hsize = (GS_image_hsize % 2048)/ 2;
		image_me_vsize = (GS_image_vsize % 2048)/ 2;
	}

	image_me_hsize = image_me_hsize % 1023;
	image_me_vsize = image_me_vsize % 1023;
	image_me1_blk_hsize = (image_me_hsize / 4) %512;
	image_me1_blk_vsize = (image_me_vsize / 4) %512;

	if(GS_me2_2x2) {
		image_me2_blk_ph_hsize = ((image_me1_blk_hsize % 256) *2 );
		image_me2_blk_ph_vsize = ((image_me1_blk_vsize % 256) *2 );
	} else {
		image_me2_blk_ph_hsize = image_me1_blk_hsize;
		image_me2_blk_ph_vsize = image_me1_blk_vsize;
	}

	//====================================
	//bit
	//line_step 96byte = 768bit align
	//frame_step , always biggest
	//address
	//ori_unit = byte,  new spec unit = 128bit
	ipme_bit = 8;
	if(GS_full_dma_setting)
	{
		ipme_addr_base = GS_me_base_addr;
		ipme_line_step = (int)((image_me_hsize * ipme_bit+767)/768)*768/8;
	}
	else
	{
		ipme_addr_base = 0;
		ipme_line_step = (int)((GS_image_hsize * ipme_bit+127)/128)*128/8;
	}
	ipme_frame_step = image_me_vsize * ipme_line_step;

	ipme_addr_0_sta = ipme_addr_base;
	ipme_addr_0_end = ipme_addr_0_sta + ipme_frame_step;
	ipme_addr_1_sta = ipme_addr_0_end;
	ipme_addr_1_end = ipme_addr_1_sta + ipme_frame_step;
	ipme_addr_2_sta = ipme_addr_1_end;
	ipme_addr_2_end = ipme_addr_2_sta + ipme_frame_step;
	ipme_addr_3_sta = ipme_addr_2_end;
	ipme_addr_3_end = ipme_addr_3_sta + ipme_frame_step;
	ipme_addr_4_sta = ipme_addr_3_end;
	ipme_addr_4_end = ipme_addr_4_sta + ipme_frame_step;
	ipme_addr_5_sta = ipme_addr_4_end;
	ipme_addr_5_end = ipme_addr_5_sta + ipme_frame_step;
	ipme_addr_6_sta = ipme_addr_5_end;
	ipme_addr_6_end = ipme_addr_6_sta + ipme_frame_step;
	ipme_addr_7_sta = ipme_addr_6_end;
	ipme_addr_7_end = ipme_addr_7_sta + ipme_frame_step;

	kme08_logo_iir_bit = 8;
	kme08_logo_iir_line_step = (int)((image_me_hsize * kme08_logo_iir_bit+767)/768)*768/8;
	kme08_logo_iir_frame_step = image_me_vsize * kme08_logo_iir_line_step;
	kme08_logo_iir_addr_0_sta = ipme_addr_7_end;
	kme08_logo_iir_addr_0_end = kme08_logo_iir_addr_0_sta + kme08_logo_iir_frame_step;
	kme08_logo_iir_addr_1_sta = kme08_logo_iir_addr_0_sta;
	kme08_logo_iir_addr_1_end = kme08_logo_iir_addr_0_end;

	kme10_pxl_hsty_bit = 4;
	kme10_pxl_hsty_line_step = (int)((image_me_hsize * kme10_pxl_hsty_bit+767)/768)*768/8;
	kme10_pxl_hsty_frame_step = image_me_vsize * kme10_pxl_hsty_line_step;
	kme10_pxl_hsty_addr_0_sta = kme08_logo_iir_addr_1_end;
	kme10_pxl_hsty_addr_0_end = kme10_pxl_hsty_addr_0_sta + kme10_pxl_hsty_frame_step;
	kme10_pxl_hsty_addr_1_sta = kme10_pxl_hsty_addr_0_sta;
	kme10_pxl_hsty_addr_1_end = kme10_pxl_hsty_addr_0_end;

	kme12_pxl_logo_bit = 1;
	kme12_pxl_logo_line_step = (int)((image_me_hsize * kme12_pxl_logo_bit+767)/768)*768/8;
	kme12_pxl_logo_frame_step = image_me_vsize * kme12_pxl_logo_line_step;
	kme12_pxl_logo_addr_0_sta = kme10_pxl_hsty_addr_1_end;
	kme12_pxl_logo_addr_0_end = kme12_pxl_logo_addr_0_sta + kme12_pxl_logo_frame_step;
	kme12_pxl_logo_addr_1_sta = kme12_pxl_logo_addr_0_end;
	kme12_pxl_logo_addr_1_end = kme12_pxl_logo_addr_1_sta + kme12_pxl_logo_frame_step;

	kme14_blk_hsty_bit = 4;
	kme14_blk_hsty_line_step = (int)((image_me1_blk_hsize * kme14_blk_hsty_bit+767)/768)*768/8;
	kme14_blk_hsty_frame_step = image_me1_blk_vsize * kme14_blk_hsty_line_step;
	kme14_blk_hsty_addr_0_sta = kme12_pxl_logo_addr_1_end;
	kme14_blk_hsty_addr_0_end = kme14_blk_hsty_addr_0_sta + kme14_blk_hsty_frame_step;
	kme14_blk_hsty_addr_1_sta = kme14_blk_hsty_addr_0_sta;
	kme14_blk_hsty_addr_1_end = kme14_blk_hsty_addr_0_end;

	mv01_me2_ph_bit = 21;
	mv01_me2_ph_line_step = (int)((image_me2_blk_ph_hsize * mv01_me2_ph_bit+767)/768)*768/8;
	mv01_me2_ph_frame_step = image_me2_blk_ph_vsize * mv01_me2_ph_line_step;
	mv01_me2_ph_addr_0_sta = kme14_blk_hsty_addr_1_end;
	mv01_me2_ph_addr_0_end = mv01_me2_ph_addr_0_sta + mv01_me2_ph_frame_step;

	mv02_me1_pi_bit = 40;
#if (defined(MARK2)||defined(MERLIN7))
	if(GS_kme_dm_bg_1bit)
	{
		mv02_me1_pi_bit = 50;
	}
#endif	
	if(GS_full_dma_setting)
	{ 
		mv02_me1_pi_addr_base  = mv01_me2_ph_addr_0_end;
		mv02_me1_pi_line_step = (int)((image_me1_blk_hsize * mv02_me1_pi_bit+767)/768)*768/8;
	}
	else
	{ 
		mv02_me1_pi_addr_base  = 0x0052b000;
		mv02_me1_pi_line_step = (int)((image_me1_blk_hsize * mv02_me1_pi_bit+127)/128)*128/8;
	}
	mv02_me1_pi_frame_step = image_me1_blk_vsize * mv02_me1_pi_line_step;
	mv02_me1_pi_addr_0_sta = mv02_me1_pi_addr_base;
	mv02_me1_pi_addr_0_end = mv02_me1_pi_addr_0_sta + mv02_me1_pi_frame_step;

	mv04_me1_ppfv_bit = 33;
#if (defined(MARK2)||defined(MERLIN7))
	if(GS_kme_dm_bg_1bit)
	{
		#if defined(MARK2)
		mv04_me1_ppfv_bit = 34;
		#endif
		#if defined(MERLIN7)
		mv04_me1_ppfv_bit = 38;
		#endif
	}
#endif
	if(GS_full_dma_setting)
	{ 
		mv04_me1_ppfv_addr_base  = mv02_me1_pi_addr_0_end;
		mv04_me1_ppfv_line_step  = (int)((image_me1_blk_hsize * mv04_me1_ppfv_bit+767)/768)*768/8;
	}
	else
	{
		mv04_me1_ppfv_addr_base  = 0x00556000;
		mv04_me1_ppfv_line_step  = (int)((image_me1_blk_hsize * mv04_me1_ppfv_bit+127)/128)*128/8;
	}
	mv04_me1_ppfv_frame_step = image_me1_blk_vsize * mv04_me1_ppfv_line_step;
	mv04_me1_ppfv_addr_0_sta = mv04_me1_ppfv_addr_base;
	mv04_me1_ppfv_addr_0_end = mv04_me1_ppfv_addr_0_sta + mv04_me1_ppfv_frame_step;
	mv04_me1_ppfv_addr_1_sta = mv04_me1_ppfv_addr_0_end;
	mv04_me1_ppfv_addr_1_end = mv04_me1_ppfv_addr_1_sta + mv04_me1_ppfv_frame_step;
	mv04_me1_ppfv_addr_2_sta = mv04_me1_ppfv_addr_1_end;
	mv04_me1_ppfv_addr_2_end = mv04_me1_ppfv_addr_2_sta + mv04_me1_ppfv_frame_step;

	if(GS_full_dma_setting)
	{ mv05_me1_pfv_addr_base  = mv04_me1_ppfv_addr_2_end;}
	else
	{ mv05_me1_pfv_addr_base  = 0x005bc000;}
	
	mv05_me1_pfv_bit = 64;
#if (defined(MARK2)||defined(MERLIN7))
	if(GS_kme_dm_bg_1bit)
	{
		#if defined(MARK2)
		mv05_me1_pfv_bit = 65;
		#endif
		#if defined(MERLIN7)
		mv05_me1_pfv_bit = 69;
		#endif
	}
#endif

	mv05_me1_pfv_line_step  = (int)((image_me1_blk_hsize * mv05_me1_pfv_bit+767)/768)*768/8;
	mv05_me1_pfv_frame_step = image_me1_blk_vsize * mv05_me1_pfv_line_step;
	mv05_me1_pfv_addr_0_sta = mv05_me1_pfv_addr_base;
	mv05_me1_pfv_addr_0_end = mv05_me1_pfv_addr_0_sta + mv05_me1_pfv_frame_step;
	mv05_me1_pfv_addr_1_sta = mv05_me1_pfv_addr_0_end;
	mv05_me1_pfv_addr_1_end = mv05_me1_pfv_addr_1_sta + mv05_me1_pfv_frame_step;

	//GSC_me_end_addr = mv05_me1_pfv_addr_1_end;

	//meshr_rpt_pfv_addr_0_base = GSC_mc_end_addr;
	meshr_rpt_pfv_addr_0_base = mv05_me1_pfv_addr_1_end;
	meshr_rpt_pfv_bit = 6;
	meshr_rpt_pfv_line_step  = (int)((image_me1_blk_hsize * meshr_rpt_pfv_bit+767)/768)*768/8;
	meshr_rpt_pfv_frame_step = image_me1_blk_vsize * meshr_rpt_pfv_line_step;
	meshr_rpt_pfv_addr_0_sta = meshr_rpt_pfv_addr_0_base;
	meshr_rpt_pfv_addr_0_end = meshr_rpt_pfv_addr_0_sta + meshr_rpt_pfv_frame_step;
	meshr_rpt_pfv_addr_1_sta = meshr_rpt_pfv_addr_0_end;
	meshr_rpt_pfv_addr_1_end = meshr_rpt_pfv_addr_1_sta + meshr_rpt_pfv_frame_step;

	meshr_rpt_ppfv_bit = 6;
	meshr_rpt_ppfv_line_step  = (int)((image_me1_blk_hsize * meshr_rpt_ppfv_bit+767)/768)*768/8;
	meshr_rpt_ppfv_frame_step = image_me1_blk_vsize * meshr_rpt_ppfv_line_step;
	meshr_rpt_ppfv_addr_0_sta = meshr_rpt_pfv_addr_1_end;
	meshr_rpt_ppfv_addr_0_end = meshr_rpt_ppfv_addr_0_sta + meshr_rpt_ppfv_frame_step;
	meshr_rpt_ppfv_addr_1_sta = meshr_rpt_ppfv_addr_0_end;
	meshr_rpt_ppfv_addr_1_end = meshr_rpt_ppfv_addr_1_sta + meshr_rpt_ppfv_frame_step;
	meshr_rpt_ppfv_addr_2_sta = meshr_rpt_ppfv_addr_1_end;
	meshr_rpt_ppfv_addr_2_end = meshr_rpt_ppfv_addr_2_sta + meshr_rpt_ppfv_frame_step;

#if (defined(H5C2)||defined(MARK2)||defined(MERLIN7))
	meshr_logoprotect_addr_0_base = meshr_rpt_ppfv_addr_2_end;
	
#if defined(MARK2)
	meshr_logoprotect_bit = 3;
#endif
#if defined(MERLIN7)
	meshr_logoprotect_bit = 12;
#endif
#if defined(H5C2)
		meshr_logoprotect_bit = 13;
#endif

	meshr_logoprotect_line_step  = (int)((image_me1_blk_hsize * meshr_logoprotect_bit+767)/768)*768/8;
	meshr_logoprotect_frame_step = image_me1_blk_vsize * meshr_logoprotect_line_step;
	meshr_logoprotect_addr_0_sta = meshr_logoprotect_addr_0_base;
	meshr_logoprotect_addr_0_end = meshr_logoprotect_addr_0_sta + meshr_logoprotect_frame_step;
	meshr_logoprotect_addr_1_sta = meshr_logoprotect_addr_0_end;
	meshr_logoprotect_addr_1_end = meshr_logoprotect_addr_1_sta + meshr_logoprotect_frame_step;
	meshr_logoprotect_addr_2_sta = meshr_logoprotect_addr_1_end;
	meshr_logoprotect_addr_2_end = meshr_logoprotect_addr_2_sta + meshr_logoprotect_frame_step;
	meshr_logoprotect_addr_3_sta = meshr_logoprotect_addr_2_end;
	meshr_logoprotect_addr_3_end = meshr_logoprotect_addr_3_sta + meshr_logoprotect_frame_step;
	meshr_logoprotect_addr_4_sta = meshr_logoprotect_addr_3_end;
	meshr_logoprotect_addr_4_end = meshr_logoprotect_addr_4_sta + meshr_logoprotect_frame_step;
	meshr_logoprotect_addr_5_sta = meshr_logoprotect_addr_4_end;
	meshr_logoprotect_addr_5_end = meshr_logoprotect_addr_5_sta + meshr_logoprotect_frame_step;
	meshr_logoprotect_addr_6_sta = meshr_logoprotect_addr_5_end;
	meshr_logoprotect_addr_6_end = meshr_logoprotect_addr_6_sta + meshr_logoprotect_frame_step;
	meshr_logoprotect_addr_7_sta = meshr_logoprotect_addr_6_end;
	meshr_logoprotect_addr_7_end = meshr_logoprotect_addr_7_sta + meshr_logoprotect_frame_step;
	meshr_logoprotect_bl = (int)((image_me1_blk_hsize * meshr_logoprotect_bit+127)/128);
#endif

	mvinfo_dma_bit = 35;
	if(GS_full_dma_setting)
	{
		mvinfo_dma_line_step  = (int)((image_me2_blk_ph_hsize * mvinfo_dma_bit+767)/768)*768/8;
	}
	else
	{
		mvinfo_dma_line_step  = (int)((image_me2_blk_ph_hsize * mvinfo_dma_bit+127)/128)*128/8;
	}
	mvinfo_dma_frame_step = image_me2_blk_ph_vsize * mvinfo_dma_line_step;
#if (defined(MARK2)||defined(MERLIN7))
    mvinfo_dma_addr_0_sta = meshr_logoprotect_addr_7_end;
#endif
#if defined(Merlin6)
	mvinfo_dma_addr_0_sta = meshr_rpt_ppfv_addr_2_end;
#endif

	mvinfo_dma_addr_0_end = mvinfo_dma_addr_0_sta + mvinfo_dma_frame_step;

  GSC_me_end_addr = mvinfo_dma_addr_0_end;

	//TEMP for compare start
	if(GS_meander){
		kme_dm_top0_kme_00_start_address0_reg.kme_00_start_address0 = ipme_addr_0_sta;
		kme_dm_top0_kme_00_start_address1_reg.kme_00_start_address1 = ipme_addr_1_sta;
		kme_dm_top0_kme_00_start_address2_reg.kme_00_start_address2 = ipme_addr_2_sta;
		kme_dm_top0_kme_00_start_address3_reg.kme_00_start_address3 = ipme_addr_3_sta;
		kme_dm_top0_kme_00_start_address4_reg.kme_00_start_address4 = ipme_addr_4_sta;
		kme_dm_top0_kme_00_start_address5_reg.kme_00_start_address5 = ipme_addr_5_sta;
		kme_dm_top2_kme_00_start_address6_reg.kme_00_start_address6 = ipme_addr_6_sta;
		kme_dm_top2_kme_00_start_address7_reg.kme_00_start_address7 = ipme_addr_7_sta;
		kme_dm_top0_kme_00_end_address0_reg.kme_00_end_address0 = ipme_addr_0_end;
		kme_dm_top0_kme_00_end_address1_reg.kme_00_end_address1 = ipme_addr_1_end;
		kme_dm_top0_kme_00_end_address2_reg.kme_00_end_address2 = ipme_addr_2_end;
		kme_dm_top0_kme_00_end_address3_reg.kme_00_end_address3 = ipme_addr_3_end;
		kme_dm_top0_kme_00_end_address4_reg.kme_00_end_address4 = ipme_addr_4_end;
		kme_dm_top0_kme_00_end_address5_reg.kme_00_end_address5 = ipme_addr_5_end;
		kme_dm_top2_kme_00_end_address6_reg.kme_00_end_address6 = ipme_addr_6_end;
		kme_dm_top2_kme_00_end_address7_reg.kme_00_end_address7 = ipme_addr_7_end;
		kme_dm_top0_kme_00_line_offset_addr_reg.kme_00_line_offset_addr = ipme_line_step;

		kme_dm_top2_mv02_start_address11_reg.mv02_start_address0 = mv02_me1_pi_addr_0_sta;
		kme_dm_top2_mv02_end_address0_reg.mv02_end_address0 = mv02_me1_pi_addr_0_end;
		kme_dm_top2_mv_02_line_offset_addr_reg.mv02_line_offset_addr = mv02_me1_pi_line_step;

		kme_dm_top2_mv05_start_address0_reg.mv05_start_address0 = mv05_me1_pfv_addr_0_sta;
		kme_dm_top2_mv05_start_address1_reg.mv05_start_address1 = mv05_me1_pfv_addr_1_sta;
		kme_dm_top2_mv05_end_address0_reg.mv05_end_address0 = mv05_me1_pfv_addr_0_end;
		kme_dm_top2_mv05_end_address1_reg.mv05_end_address1 = mv05_me1_pfv_addr_1_end;
		kme_dm_top2_mv_05_line_offset_addr_reg.mv05_line_offset_addr = mv05_me1_pfv_line_step;

		kme_dm_top2_mv04_start_address0_reg.mv04_start_address0 = mv04_me1_ppfv_addr_0_sta;
		kme_dm_top2_mv04_start_address1_reg.mv04_start_address1 = mv04_me1_ppfv_addr_1_sta;
		kme_dm_top2_mv04_end_address0_reg.mv04_end_address0 = mv04_me1_ppfv_addr_0_end;
		kme_dm_top2_mv04_end_address1_reg.mv04_end_address1 = mv04_me1_ppfv_addr_1_end;
		kme_dm_top0_mv04_start_address2_reg.mv04_start_address2 = mv04_me1_ppfv_addr_2_sta;
		kme_dm_top0_mv04_end_address2_reg.mv04_end_address2     = mv04_me1_ppfv_addr_2_end;
		kme_dm_top2_mv_04_line_offset_addr_reg.mv04_line_offset_addr = mv04_me1_ppfv_line_step;
	}

	if(GS_logo_switch){
		kme_dm_top1_kme_dm_top1_4c_reg.kme_12_start_address1 = 0x004c8000;
		kme_dm_top1_kme_dm_top1_50_reg.kme_12_end_address0 = 0x004c7fff;
	}
	else{
		kme_dm_top1_kme_dm_top1_4c_reg.kme_12_start_address1 = 0x004c0000;
		kme_dm_top1_kme_dm_top1_50_reg.kme_12_end_address0 = 0x004d0e00;
	}

	mvinfo_dma_mvinfo_wdma_lstep_reg.line_step = mvinfo_dma_line_step/16;
	//TEMP for compare end

	if(GS_full_dma_setting)
	{
		kme_dm_top0_kme_00_start_address0_reg.kme_00_start_address0 = ipme_addr_0_sta;
		kme_dm_top0_kme_00_start_address1_reg.kme_00_start_address1 = ipme_addr_1_sta;
		kme_dm_top0_kme_00_start_address2_reg.kme_00_start_address2 = ipme_addr_2_sta;
		kme_dm_top0_kme_00_start_address3_reg.kme_00_start_address3 = ipme_addr_3_sta;
		kme_dm_top0_kme_00_start_address4_reg.kme_00_start_address4 = ipme_addr_4_sta;
		kme_dm_top0_kme_00_start_address5_reg.kme_00_start_address5 = ipme_addr_5_sta;
		kme_dm_top2_kme_00_start_address6_reg.kme_00_start_address6 = ipme_addr_6_sta;
		kme_dm_top2_kme_00_start_address7_reg.kme_00_start_address7 = ipme_addr_7_sta;
		kme_dm_top0_kme_00_end_address0_reg.kme_00_end_address0 = ipme_addr_0_end;
		kme_dm_top0_kme_00_end_address1_reg.kme_00_end_address1 = ipme_addr_1_end;
		kme_dm_top0_kme_00_end_address2_reg.kme_00_end_address2 = ipme_addr_2_end;
		kme_dm_top0_kme_00_end_address3_reg.kme_00_end_address3 = ipme_addr_3_end;
		kme_dm_top0_kme_00_end_address4_reg.kme_00_end_address4 = ipme_addr_4_end;
		kme_dm_top0_kme_00_end_address5_reg.kme_00_end_address5 = ipme_addr_5_end;
		kme_dm_top2_kme_00_end_address6_reg.kme_00_end_address6 = ipme_addr_6_end;
		kme_dm_top2_kme_00_end_address7_reg.kme_00_end_address7 = ipme_addr_7_end;
		kme_dm_top0_kme_00_line_offset_addr_reg.kme_00_line_offset_addr = ipme_line_step;

		kme_dm_top1_kme_dm_top1_00_reg.kme_08_start_address0	= kme08_logo_iir_addr_0_sta;
		kme_dm_top1_kme_dm_top1_04_reg.kme_08_start_address1    = kme08_logo_iir_addr_1_sta;
		kme_dm_top1_kme_dm_top1_08_reg.kme_08_end_address0      = kme08_logo_iir_addr_0_end;
		kme_dm_top1_kme_dm_top1_0c_reg.kme_08_end_address1      = kme08_logo_iir_addr_1_end;
		kme_dm_top1_kme_dm_top1_10_reg.kme_08_line_offset_addr  = kme08_logo_iir_line_step;
		kme_dm_top1_kme_dm_top1_24_reg.kme_10_start_address0    = kme10_pxl_hsty_addr_0_sta;
		kme_dm_top1_kme_dm_top1_28_reg.kme_10_start_address1    = kme10_pxl_hsty_addr_1_sta;
		kme_dm_top1_kme_dm_top1_2c_reg.kme_10_end_address0      = kme10_pxl_hsty_addr_0_end;
		kme_dm_top1_kme_dm_top1_30_reg.kme_10_end_address1      = kme10_pxl_hsty_addr_1_end;
		kme_dm_top1_kme_dm_top1_34_reg.kme_10_line_offset_addr  = kme10_pxl_hsty_line_step;
		kme_dm_top1_kme_dm_top1_48_reg.kme_12_start_address0    = kme12_pxl_logo_addr_0_sta;
		kme_dm_top1_kme_dm_top1_4c_reg.kme_12_start_address1    = kme12_pxl_logo_addr_1_sta;
		kme_dm_top1_kme_dm_top1_50_reg.kme_12_end_address0      = kme12_pxl_logo_addr_0_end;
		kme_dm_top1_kme_dm_top1_54_reg.kme_12_end_address1      = kme12_pxl_logo_addr_1_end;
		kme_dm_top1_kme_dm_top1_58_reg.kme_12_line_offset_addr  = kme12_pxl_logo_line_step;
		kme_dm_top1_kme_dm_top1_6c_reg.kme_14_start_address0    = kme14_blk_hsty_addr_0_sta;
		kme_dm_top1_kme_dm_top1_70_reg.kme_14_start_address1    = kme14_blk_hsty_addr_1_sta;
		kme_dm_top1_kme_dm_top1_74_reg.kme_14_end_address0      = kme14_blk_hsty_addr_0_end;
		kme_dm_top1_kme_dm_top1_78_reg.kme_14_end_address1      = kme14_blk_hsty_addr_1_end;
		kme_dm_top1_kme_dm_top1_7c_reg.kme_14_line_offset_addr  = kme14_blk_hsty_line_step;

		kme_dm_top2_mv01_start_address0_reg.mv01_start_address0  = mv01_me2_ph_addr_0_sta;
		kme_dm_top2_mv01_end_address0_reg.mv01_end_address0      = mv01_me2_ph_addr_0_end;
		kme_dm_top2_mv_01_line_offset_addr_reg.mv01_line_offset_addr = mv01_me2_ph_line_step;

		kme_dm_top2_mv02_start_address11_reg.mv02_start_address0 = mv02_me1_pi_addr_0_sta;
		kme_dm_top2_mv02_end_address0_reg.mv02_end_address0 = mv02_me1_pi_addr_0_end;
		kme_dm_top2_mv_02_line_offset_addr_reg.mv02_line_offset_addr = mv02_me1_pi_line_step;

		kme_dm_top2_mv04_start_address0_reg.mv04_start_address0 = mv04_me1_ppfv_addr_0_sta;
		kme_dm_top2_mv04_start_address1_reg.mv04_start_address1 = mv04_me1_ppfv_addr_1_sta;
		kme_dm_top2_mv04_end_address0_reg.mv04_end_address0 = mv04_me1_ppfv_addr_0_end;
		kme_dm_top2_mv04_end_address1_reg.mv04_end_address1 = mv04_me1_ppfv_addr_1_end;
		kme_dm_top0_mv04_start_address2_reg.mv04_start_address2 = mv04_me1_ppfv_addr_2_sta;
		kme_dm_top0_mv04_end_address2_reg.mv04_end_address2     = mv04_me1_ppfv_addr_2_end;
		kme_dm_top2_mv_04_line_offset_addr_reg.mv04_line_offset_addr = mv04_me1_ppfv_line_step;

		kme_dm_top2_mv05_start_address0_reg.mv05_start_address0 = mv05_me1_pfv_addr_0_sta;
		kme_dm_top2_mv05_start_address1_reg.mv05_start_address1 = mv05_me1_pfv_addr_1_sta;
		kme_dm_top2_mv05_end_address0_reg.mv05_end_address0 = mv05_me1_pfv_addr_0_end;
		kme_dm_top2_mv05_end_address1_reg.mv05_end_address1 = mv05_me1_pfv_addr_1_end;
		kme_dm_top2_mv_05_line_offset_addr_reg.mv05_line_offset_addr = mv05_me1_pfv_line_step;

		dbus_wrapper_me_uplimit_addr_reg.kme_uplimit_addr = mv05_me1_pfv_addr_1_end/16; 
		dbus_wrapper_me_downlimit_addr_reg.kme_downlimit_addr =  ipme_addr_0_sta/16;
		//0408
		dbus_wrapper_memc_dcu1_latcnt_cr11_reg.client10_latcnt_end = 0x0;
		dbus_wrapper_memc_dcu1_latcnt_cr11_reg.client10_latcnt_beg = 0x0;
		dbus_wrapper_memc_dcu1_latcnt_cr12_reg.client11_latcnt_end = 0x0;
		dbus_wrapper_memc_dcu1_latcnt_cr12_reg.client11_latcnt_beg = 0x0;

		me_share_dma_me1_wdma0_lstep_reg.line_step		= meshr_rpt_pfv_line_step/16;
		me_share_dma_me1_wdma0_mstart0_reg.start_address0       = meshr_rpt_pfv_addr_0_sta/16;
		me_share_dma_me1_wdma0_mstart1_reg.start_address1       = meshr_rpt_pfv_addr_1_sta/16;
		me_share_dma_me1_wdma0_mend0_reg.end_address0           = meshr_rpt_pfv_addr_0_end/16;
		me_share_dma_me1_wdma0_mend1_reg.end_address1           = meshr_rpt_pfv_addr_1_end/16;
		me_share_dma_me1_wdma1_lstep_reg.line_step              = meshr_rpt_ppfv_line_step/16;
		me_share_dma_me1_wdma1_mstart0_reg.start_address0       = meshr_rpt_ppfv_addr_0_sta/16;
		me_share_dma_me1_wdma1_mstart1_reg.start_address1       = meshr_rpt_ppfv_addr_1_sta/16;
		me_share_dma_me1_wdma1_mstart2_reg.start_address2       = meshr_rpt_ppfv_addr_2_sta/16;
		me_share_dma_me1_wdma1_mend0_reg.end_address0           = meshr_rpt_ppfv_addr_0_end/16;
		me_share_dma_me1_wdma1_mend1_reg.end_address1           = meshr_rpt_ppfv_addr_1_end/16;
		me_share_dma_me1_wdma1_mend2_reg.end_address2           = meshr_rpt_ppfv_addr_2_end/16;

#if (defined(MARK2)||defined(MERLIN7))
		me_share_dma_ipme_wdma_ctrl_reg.height=image_me1_blk_vsize;
		me_share_dma_ipme_wdma_ctrl_reg.width=image_me1_blk_hsize;
		me_share_dma_me1_rdma0_ctrl_reg.height=image_me1_blk_vsize;
		me_share_dma_me1_rdma0_ctrl_reg.width=image_me1_blk_hsize;
		me_share_dma_ipme_wdma_num_bl_reg.num=0;
		me_share_dma_ipme_wdma_num_bl_reg.bl=meshr_logoprotect_bl;
		me_share_dma_ipme_wdma_num_bl_reg.remain=meshr_logoprotect_bl;
		me_share_dma_me1_rdma0_num_bl_reg.num=0;
		me_share_dma_me1_rdma0_num_bl_reg.bl=meshr_logoprotect_bl;
		me_share_dma_me1_rdma0_num_bl_reg.remain=meshr_logoprotect_bl;
		me_share_dma_ipme_wdma_lstep_reg.line_step=meshr_logoprotect_line_step/16;
		me_share_dma_ipme_wdma_mstart0_reg.start_address0=meshr_logoprotect_addr_0_sta/16;
		me_share_dma_ipme_wdma_mstart1_reg.start_address1=meshr_logoprotect_addr_1_sta/16;
		me_share_dma_ipme_wdma_mstart1_reg.start_address1=meshr_logoprotect_addr_1_sta/16;
		me_share_dma_ipme_wdma_mstart2_reg.start_address2=meshr_logoprotect_addr_2_sta/16;
		me_share_dma_ipme_wdma_mstart3_reg.start_address3=meshr_logoprotect_addr_3_sta/16;
		me_share_dma_ipme_wdma_mstart4_reg.start_address4=meshr_logoprotect_addr_4_sta/16;
		me_share_dma_ipme_wdma_mstart5_reg.start_address5=meshr_logoprotect_addr_5_sta/16;
		me_share_dma_ipme_wdma_mstart6_reg.start_address6=meshr_logoprotect_addr_6_sta/16;
		me_share_dma_ipme_wdma_mstart7_reg.start_address7=meshr_logoprotect_addr_7_sta/16;
		me_share_dma_ipme_wdma_mend0_reg.end_address0=meshr_logoprotect_addr_0_end/16;
		me_share_dma_ipme_wdma_mend1_reg.end_address1=meshr_logoprotect_addr_1_end/16;
		me_share_dma_ipme_wdma_mend1_reg.end_address1=meshr_logoprotect_addr_1_end/16;
		me_share_dma_ipme_wdma_mend2_reg.end_address2=meshr_logoprotect_addr_2_end/16;
		me_share_dma_ipme_wdma_mend3_reg.end_address3=meshr_logoprotect_addr_3_end/16;
		me_share_dma_ipme_wdma_mend4_reg.end_address4=meshr_logoprotect_addr_4_end/16;
		me_share_dma_ipme_wdma_mend5_reg.end_address5=meshr_logoprotect_addr_5_end/16;
		me_share_dma_ipme_wdma_mend6_reg.end_address6=meshr_logoprotect_addr_6_end/16;
		me_share_dma_ipme_wdma_mend7_reg.end_address7=meshr_logoprotect_addr_7_end/16;
#endif

#if (defined(MARK2)||defined(MERLIN7))
		me_share_dma_me_dma_wr_rule_check_up_reg.dma_up_limit = meshr_logoprotect_addr_7_end/16;
		me_share_dma_me_dma_wr_rule_check_low_reg.dma_low_limit = meshr_rpt_pfv_addr_0_sta/16;
		me_share_dma_me_dma_rd_rule_check_up_reg.dma_up_limit = meshr_logoprotect_addr_7_end/16;
		me_share_dma_me_dma_rd_rule_check_low_reg.dma_low_limit = meshr_rpt_pfv_addr_0_sta/16;
#endif
#if defined(Merlin6)
		me_share_dma_me_dma_wr_rule_check_up_reg.dma_up_limit = meshr_rpt_ppfv_addr_2_end/16;
		me_share_dma_me_dma_wr_rule_check_low_reg.dma_low_limit = meshr_rpt_pfv_addr_0_sta/16;
		me_share_dma_me_dma_rd_rule_check_up_reg.dma_up_limit = meshr_rpt_ppfv_addr_2_end/16;
		me_share_dma_me_dma_rd_rule_check_low_reg.dma_low_limit = meshr_rpt_pfv_addr_0_sta/16;
#endif

		mvinfo_dma_mvinfo_wdma_lstep_reg.line_step = mvinfo_dma_line_step/16;
		mvinfo_dma_mvinfo_wdma_mstart0_reg.start_address0 = mvinfo_dma_addr_0_sta/16;
		mvinfo_dma_mvinfo_wdma_mend0_reg.end_address0 = mvinfo_dma_addr_0_end/16;
		mvinfo_dma_mvinfo_wr_rule_check_up_reg.dma_up_limit = mvinfo_dma_addr_0_end/16;
		mvinfo_dma_mvinfo_wr_rule_check_low_reg.dma_low_limit = mvinfo_dma_addr_0_sta/16;
		mvinfo_dma_mvinfo_rd_rule_check_up_reg.dma_up_limit = mvinfo_dma_addr_0_end/16;
		mvinfo_dma_mvinfo_rd_rule_check_low_reg.dma_low_limit = mvinfo_dma_addr_0_sta/16;
	}

	//====================================
	//hnum
	//burst length
	//pqc

	reg_mv01_hnum0 = (image_me2_blk_ph_hsize * 21) % 16384;
	if((reg_mv01_hnum0 % 128) == 0) {
		reg_mv01_hnum = (reg_mv01_hnum0 /128) % 256;
	} else {
		reg_mv01_hnum = ((reg_mv01_hnum0 /128) + 1) % 256;
	}

	reg_mv02_hnum0 = (image_me1_blk_hsize * mv02_me1_pi_bit) % 16384;
	if((reg_mv02_hnum0 %128) == 0){
		reg_mv02_hnum = (reg_mv02_hnum0 /128 ) %256;
	} else {
		reg_mv02_hnum = ((reg_mv02_hnum0 /128) + 1) %256;
	}


	reg_mv04_hnum0 = (image_me1_blk_hsize * mv04_me1_ppfv_bit) %16384;
	if((reg_mv04_hnum0 %128) == 0){
		reg_mv04_hnum = (reg_mv04_hnum0 /128 ) & 0xff;
	} else {
		reg_mv04_hnum = ((reg_mv04_hnum0 /128) + 1) %256;
	}

	reg_mv05_hnum0 = (image_me1_blk_hsize * mv05_me1_pfv_bit)       ;
	if((reg_mv05_hnum0 %128) == 0){
		reg_mv05_hnum = (reg_mv05_hnum0 /128 ) & 0xff;
	} else {
		reg_mv05_hnum = ((reg_mv05_hnum0 /128) + 1) %256;
	}	
//	reg_mv05_hnum = image_me1_blk_hsize / 2;
	
#if (defined(MARK2)||defined(MERLIN7))
	if(GS_kme_dm_bg_1bit)
	{
		kme_dm_top2_mv_source_mode_reg.dm_bg_info_enable=1;
	}
	else
	{
		kme_dm_top2_mv_source_mode_reg.dm_bg_info_enable=0;
	}
#endif

	//me_pqc
	if(GS_me_pqc_mode ==0)
	{
		me_pqc_ipme_en = 0;
		me_pqc_logo_en = 0;
	}
	else if (GS_me_pqc_mode ==1)
	{
		me_pqc_ipme_en = 1;
		me_pqc_logo_en = 0;
	}
	else if (GS_me_pqc_mode ==2)
	{
		me_pqc_ipme_en = 0;
		me_pqc_logo_en = 1;
	}
	else// if (GS_me_pqc_mode ==3)
	{
		me_pqc_ipme_en = 1;
		me_pqc_logo_en = 1;
	}
	
	//me_pqc_bpp = 6;
	me_pqc_bpp = 4;
	me_pqc_width_div32 = image_me_hsize / 32;
	me_pqc_height = image_me_vsize;
	me_pqc_line_sum_bit = ((image_me_hsize * me_pqc_bpp) + 256) / 128;

	kme_dm_top0_kme_me_hnum_reg.kme_mv_hnum = (image_me_hsize / 4) %256; //nouse
	kme_dm_top0_kme_me_hnum_reg.kme_me_hnum = (me_pqc_ipme_en ==1)? me_pqc_line_sum_bit : ((image_me_hsize >> 4) %256);

    reg_iplogo_hnum0 = (image_me_hsize *8) %16384;
    if((reg_iplogo_hnum0 %128) == 0){
		reg_iplogo_hnum = reg_iplogo_hnum0/128;
    } else {
		reg_iplogo_hnum = (reg_iplogo_hnum0/128 + 1) %256;
    }
    
    kme_dm_top1_kme_dm_top1_a0_reg.kme_iplogo_hnum = (me_pqc_logo_en ==1)? me_pqc_line_sum_bit : reg_iplogo_hnum;

    reg_mlogo_hnum0 = (image_me_hsize *4) %16384;
    if((reg_mlogo_hnum0 %128) == 0){
            reg_mlogo_hnum = reg_mlogo_hnum0/128;
    } else {
            reg_mlogo_hnum = (reg_mlogo_hnum0/128 + 1) %256;
    }    
    
    kme_dm_top1_kme_dm_top1_a0_reg.kme_masklogo_hnum = reg_mlogo_hnum;
    
    reg_plogo_hnum0 = (image_me_hsize *1) %16384;
    if((reg_plogo_hnum0 %128) == 0){
            reg_plogo_hnum = reg_plogo_hnum0/128;
    } else {
            reg_plogo_hnum = (reg_plogo_hnum0/128 + 1) %256;
    }    
    
    kme_dm_top1_kme_dm_top1_a0_reg.kme_plogo_hnum = reg_plogo_hnum;    
    
    reg_hlogo_hnum0 = ((image_me_hsize /4) *4) %16384;
    if((reg_hlogo_hnum0 %128) == 0){
            reg_hlogo_hnum = reg_hlogo_hnum0/128;
    } else {
            reg_hlogo_hnum = (reg_hlogo_hnum0/128 + 1) %256;
    }
    kme_dm_top1_kme_dm_top1_a0_reg.kme_hlogo_hnum = reg_hlogo_hnum;  //blk hsty 

    kme_dm_top2_mv01_agent_reg.mv01_hnum = reg_mv01_hnum;
	kme_dm_top2_mv02_agent_reg.mv02_hnum = reg_mv02_hnum;
	kme_dm_top2_mv04_agent_reg.mv04_hnum = reg_mv04_hnum;
	kme_dm_top2_mv05_agent_reg.mv05_hnum = reg_mv05_hnum;

	mvinfo_line_128 = (int)((image_me2_blk_ph_hsize*mvinfo_dma_bit+127)/128);
	mvinfo_dma_mvinfo_wdma_num_bl_reg.line_128_num = mvinfo_line_128;
	mvinfo_dma_mvinfo_rdma_num_bl_reg.line_128_num = mvinfo_line_128;

	//me pqc
	//0406
	kme_pqc_pqdc_kme_pq_cmp_reg.cmp_en = (me_pqc_ipme_en ==1);
	kme_pqc_pqdc_kme_pq_cmp_reg.logo_cmp_en = (me_pqc_logo_en ==1);
	kme_pqc_pqdc_kme_pq_decmp_reg.decmp_en = (me_pqc_ipme_en ==1);
	kme_pqc_pqdc_kme_pq_decmp_reg.logo_decmp_en = (me_pqc_logo_en ==1);
	kme_pqc_pqdc_kme_pq_cmp_bit_reg.frame_limit_bit = 4;
	kme_pqc_pqdc_kme_pq_cmp_bit_reg.line_limit_bit = 4;
	//0406
	kme_pqc_pqdc_kme_pq_cmp_reg.cmp_width_div32 = me_pqc_width_div32 ;
	kme_pqc_pqdc_kme_pq_cmp_reg.cmp_height = me_pqc_height ;
	kme_pqc_pqdc_kme_pq_cmp_pair_reg.cmp_line_sum_bit = me_pqc_line_sum_bit;
	kme_pqc_pqdc_kme_pq_decmp_reg.decmp_width_div32 = me_pqc_width_div32 ;
	kme_pqc_pqdc_kme_pq_decmp_reg.decmp_height = me_pqc_height ;
	kme_pqc_pqdc_kme_pq_decmp_pair_reg.decmp_line_sum_bit = me_pqc_line_sum_bit;

	//0406
	if(GS_full_dma_setting)
	{
	kme_dm_top0_kme_00_agent_reg.kme_00_alen = 29;
	kme_dm_top0_kme_01_agent_reg.kme_01_alen = 59;
	kme_dm_top0_kme_02_agent_reg.kme_02_alen = 29;
	kme_dm_top0_kme_03_agent_reg.kme_03_alen = 29;
	kme_dm_top0_kme_04_agent_reg.kme_04_alen = 29;
	kme_dm_top0_kme_05_agent_reg.kme_05_alen = 29;
	kme_dm_top0_kme06agent_reg.kme_06_alen = 29;
	kme_dm_top0_kme_07_agent_reg.kme_07_alen = 29;
	kme_dm_top1_kme_dm_top1_14_reg.kme_08_alen = 29;
	kme_dm_top1_kme_dm_top1_1c_reg.kme_09_alen = 59;
	kme_dm_top1_kme_dm_top1_38_reg.kme_10_alen = 29;
	kme_dm_top1_kme_dm_top1_40_reg.kme_11_alen = 59;
	kme_dm_top1_kme_dm_top1_5c_reg.kme_12_alen = 29;
	kme_dm_top1_kme_dm_top1_64_reg.kme_13_alen = 29;
	kme_dm_top1_kme_dm_top1_80_reg.kme_14_alen = 29;
	kme_dm_top1_kme_dm_top1_88_reg.kme_15_alen = 59;
	kme_dm_top2_mv01_agent_reg.mv01_alen = 29;
	kme_dm_top2_mv02_agent_reg.mv02_alen = 29;
	kme_dm_top2_mv03_agent_reg.mv03_alen = 47;
	kme_dm_top2_mv04_agent_reg.mv04_alen = 29;
	kme_dm_top2_mv05_agent_reg.mv05_alen = 29;
	kme_dm_top2_mv06_agent_reg.mv06_alen = 59;
	kme_dm_top2_mv07_agent_reg.mv07_alen = 47;
	kme_dm_top2_mv08_agent_reg.mv08_alen = 47;
	kme_dm_top2_mv09_agent_reg.mv09_alen = 59;
	kme_dm_top2_mv10_agent_reg.mv10_alen = 59;
	kme_dm_top2_mv11_agent_reg.mv11_alen = 59;
	kme_dm_top2_mv12_agent_reg.mv12_alen = 47;
	mvinfo_dma_mvinfo_wdma_num_bl_reg.bl = 24;
	mvinfo_dma_mvinfo_rdma_num_bl_reg.bl = 24;
	//0408
	kme_dm_top2_mv12_agent_reg.mv12_cmdlen = 1;
	}
	//0406
	//====================================
	//register

	kme_dm_top0_kme_me_resolution_reg.kme_me_vactive = image_me_vsize;
	kme_dm_top0_kme_me_resolution_reg.kme_me_hactive = image_me_hsize;

	kme_dm_top0_kme_mv_resolution_reg.kme_mv_vactive = image_me1_blk_vsize;
	kme_dm_top0_kme_mv_resolution_reg.kme_mv_hactive = image_me1_blk_hsize;

	kme_dm_top1_kme_dm_top1_90_reg.kme_iplogo_vactive = image_me_vsize;
	kme_dm_top1_kme_dm_top1_90_reg.kme_iplogo_hactive = image_me_hsize/4;   //dma128->32(8bit/pxl)
	kme_dm_top1_kme_dm_top1_94_reg.kme_mlogo_vactive  = image_me_vsize; 
	kme_dm_top1_kme_dm_top1_94_reg.kme_mlogo_hactive  = image_me_hsize/4;   //dma128->16(4bit/pxl)
	kme_dm_top1_kme_dm_top1_98_reg.kme_plogo_vactive  = image_me_vsize;
	kme_dm_top1_kme_dm_top1_98_reg.kme_plogo_hactive  = image_me_hsize/16;  //dma128->16(1bit/pxl)
	kme_dm_top1_kme_dm_top1_9c_reg.kme_hlogo_vactive  = image_me_vsize/4;   //size/4 
	kme_dm_top1_kme_dm_top1_9c_reg.kme_hlogo_hactive  = image_me_hsize/4;   //size/4 ,dma128->4(4bit/pxl)

	kme_dm_top2_mv02_resolution_reg.mv02_vactive = image_me1_blk_vsize;
	kme_dm_top2_mv02_resolution_reg.mv02_hactive = image_me1_blk_hsize;
	kme_dm_top2_mv01_resolution_reg.mv01_vactive = image_me2_blk_ph_vsize;
	kme_dm_top2_mv01_resolution_reg.mv01_hactive = image_me2_blk_ph_hsize;

	kme_dm_top2_mv02_resolution_reg.mv02_vactive = image_me1_blk_vsize;
	kme_dm_top2_mv02_resolution_reg.mv02_hactive = image_me1_blk_hsize;

	kme_dm_top2_mv04_resolution_reg.mv04_vactive = image_me1_blk_vsize;
	kme_dm_top2_mv04_resolution_reg.mv04_hactive = image_me1_blk_hsize;

	kme_dm_top2_mv05_resolution_reg.mv05_vactive = image_me1_blk_vsize;
	kme_dm_top2_mv05_resolution_reg.mv05_hactive = image_me1_blk_hsize;

	//mvinfo dma
	mvinfo_dma_mvinfo_wdma_ctrl_reg.height = image_me2_blk_ph_vsize;
	mvinfo_dma_mvinfo_wdma_ctrl_reg.width = image_me2_blk_ph_hsize;
	mvinfo_dma_mvinfo_rdma_ctrl_reg.height = image_me2_blk_ph_vsize;
	mvinfo_dma_mvinfo_rdma_ctrl_reg.width = image_me2_blk_ph_hsize; 

	// register write
	rtd_outl(KME_DM_TOP0_KME_00_START_ADDRESS0_reg, kme_dm_top0_kme_00_start_address0_reg.regValue);
	rtd_outl(KME_DM_TOP0_KME_00_START_ADDRESS1_reg, kme_dm_top0_kme_00_start_address1_reg.regValue);
	rtd_outl(KME_DM_TOP0_KME_00_START_ADDRESS2_reg, kme_dm_top0_kme_00_start_address2_reg.regValue);
	rtd_outl(KME_DM_TOP0_KME_00_START_ADDRESS3_reg, kme_dm_top0_kme_00_start_address3_reg.regValue);
	rtd_outl(KME_DM_TOP0_KME_00_START_ADDRESS4_reg, kme_dm_top0_kme_00_start_address4_reg.regValue);
	rtd_outl(KME_DM_TOP0_KME_00_START_ADDRESS5_reg, kme_dm_top0_kme_00_start_address5_reg.regValue);
	rtd_outl(KME_DM_TOP2_KME_00_START_ADDRESS6_reg, kme_dm_top2_kme_00_start_address6_reg.regValue);
	rtd_outl(KME_DM_TOP2_KME_00_START_ADDRESS7_reg, kme_dm_top2_kme_00_start_address7_reg.regValue);
	rtd_outl(KME_DM_TOP0_KME_00_END_ADDRESS0_reg, kme_dm_top0_kme_00_end_address0_reg.regValue);
	rtd_outl(KME_DM_TOP0_KME_00_END_ADDRESS1_reg, kme_dm_top0_kme_00_end_address1_reg.regValue);
	rtd_outl(KME_DM_TOP0_KME_00_END_ADDRESS2_reg, kme_dm_top0_kme_00_end_address2_reg.regValue);
	rtd_outl(KME_DM_TOP0_KME_00_END_ADDRESS3_reg, kme_dm_top0_kme_00_end_address3_reg.regValue);
	rtd_outl(KME_DM_TOP0_KME_00_END_ADDRESS4_reg, kme_dm_top0_kme_00_end_address4_reg.regValue);
	rtd_outl(KME_DM_TOP0_KME_00_END_ADDRESS5_reg, kme_dm_top0_kme_00_end_address5_reg.regValue);
	rtd_outl(KME_DM_TOP2_KME_00_END_ADDRESS6_reg, kme_dm_top2_kme_00_end_address6_reg.regValue);
	rtd_outl(KME_DM_TOP2_KME_00_END_ADDRESS7_reg, kme_dm_top2_kme_00_end_address7_reg.regValue);
	rtd_outl(KME_DM_TOP0_KME_00_LINE_OFFSET_ADDR_reg, kme_dm_top0_kme_00_line_offset_addr_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV02_START_ADDRESS11_reg, kme_dm_top2_mv02_start_address11_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV02_END_ADDRESS0_reg, kme_dm_top2_mv02_end_address0_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV_02_LINE_OFFSET_ADDR_reg, kme_dm_top2_mv_02_line_offset_addr_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV05_START_ADDRESS0_reg, kme_dm_top2_mv05_start_address0_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV05_START_ADDRESS1_reg, kme_dm_top2_mv05_start_address1_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV05_END_ADDRESS0_reg, kme_dm_top2_mv05_end_address0_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV05_END_ADDRESS1_reg, kme_dm_top2_mv05_end_address1_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV_05_LINE_OFFSET_ADDR_reg, kme_dm_top2_mv_05_line_offset_addr_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV04_START_ADDRESS0_reg, kme_dm_top2_mv04_start_address0_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV04_START_ADDRESS1_reg, kme_dm_top2_mv04_start_address1_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV04_END_ADDRESS0_reg, kme_dm_top2_mv04_end_address0_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV04_END_ADDRESS1_reg, kme_dm_top2_mv04_end_address1_reg.regValue);
	rtd_outl(KME_DM_TOP0_MV04_START_ADDRESS2_reg, kme_dm_top0_mv04_start_address2_reg.regValue);
	rtd_outl(KME_DM_TOP0_MV04_END_ADDRESS2_reg, kme_dm_top0_mv04_end_address2_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV_04_LINE_OFFSET_ADDR_reg, kme_dm_top2_mv_04_line_offset_addr_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_4C_reg, kme_dm_top1_kme_dm_top1_4c_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_50_reg, kme_dm_top1_kme_dm_top1_50_reg.regValue);
	rtd_outl(MVINFO_DMA_MVINFO_WDMA_LSTEP_reg, mvinfo_dma_mvinfo_wdma_lstep_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_00_reg, kme_dm_top1_kme_dm_top1_00_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_04_reg, kme_dm_top1_kme_dm_top1_04_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_08_reg, kme_dm_top1_kme_dm_top1_08_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_0C_reg, kme_dm_top1_kme_dm_top1_0c_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_10_reg, kme_dm_top1_kme_dm_top1_10_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_24_reg, kme_dm_top1_kme_dm_top1_24_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_28_reg, kme_dm_top1_kme_dm_top1_28_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_2C_reg, kme_dm_top1_kme_dm_top1_2c_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_30_reg, kme_dm_top1_kme_dm_top1_30_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_34_reg, kme_dm_top1_kme_dm_top1_34_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_48_reg, kme_dm_top1_kme_dm_top1_48_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_54_reg, kme_dm_top1_kme_dm_top1_54_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_58_reg, kme_dm_top1_kme_dm_top1_58_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_6C_reg, kme_dm_top1_kme_dm_top1_6c_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_70_reg, kme_dm_top1_kme_dm_top1_70_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_74_reg, kme_dm_top1_kme_dm_top1_74_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_78_reg, kme_dm_top1_kme_dm_top1_78_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_7C_reg, kme_dm_top1_kme_dm_top1_7c_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV01_START_ADDRESS0_reg, kme_dm_top2_mv01_start_address0_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV01_END_ADDRESS0_reg, kme_dm_top2_mv01_end_address0_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV_01_LINE_OFFSET_ADDR_reg, kme_dm_top2_mv_01_line_offset_addr_reg.regValue);
	rtd_outl(DBUS_WRAPPER_Me_uplimit_addr_reg, dbus_wrapper_me_uplimit_addr_reg.regValue);
	rtd_outl(DBUS_WRAPPER_Me_downlimit_addr_reg, dbus_wrapper_me_downlimit_addr_reg.regValue);
	rtd_outl(DBUS_WRAPPER_MEMC_DCU1_LATCNT_CR11_reg, dbus_wrapper_memc_dcu1_latcnt_cr11_reg.regValue);
	rtd_outl(DBUS_WRAPPER_MEMC_DCU1_LATCNT_CR12_reg, dbus_wrapper_memc_dcu1_latcnt_cr12_reg.regValue);
	rtd_outl(ME_SHARE_DMA_ME1_WDMA0_LSTEP_reg, me_share_dma_me1_wdma0_lstep_reg.regValue);
	rtd_outl(ME_SHARE_DMA_ME1_WDMA0_MSTART0_reg, me_share_dma_me1_wdma0_mstart0_reg.regValue);
	rtd_outl(ME_SHARE_DMA_ME1_WDMA0_MSTART1_reg, me_share_dma_me1_wdma0_mstart1_reg.regValue);
	rtd_outl(ME_SHARE_DMA_ME1_WDMA0_MEND0_reg, me_share_dma_me1_wdma0_mend0_reg.regValue);
	rtd_outl(ME_SHARE_DMA_ME1_WDMA0_MEND1_reg, me_share_dma_me1_wdma0_mend1_reg.regValue);
	rtd_outl(ME_SHARE_DMA_ME1_WDMA1_LSTEP_reg, me_share_dma_me1_wdma1_lstep_reg.regValue);
	rtd_outl(ME_SHARE_DMA_ME1_WDMA1_MSTART0_reg, me_share_dma_me1_wdma1_mstart0_reg.regValue);
	rtd_outl(ME_SHARE_DMA_ME1_WDMA1_MSTART1_reg, me_share_dma_me1_wdma1_mstart1_reg.regValue);
	rtd_outl(ME_SHARE_DMA_ME1_WDMA1_MSTART2_reg, me_share_dma_me1_wdma1_mstart2_reg.regValue);
	rtd_outl(ME_SHARE_DMA_ME1_WDMA1_MEND0_reg, me_share_dma_me1_wdma1_mend0_reg.regValue);
	rtd_outl(ME_SHARE_DMA_ME1_WDMA1_MEND1_reg, me_share_dma_me1_wdma1_mend1_reg.regValue);
	rtd_outl(ME_SHARE_DMA_ME1_WDMA1_MEND2_reg, me_share_dma_me1_wdma1_mend2_reg.regValue);
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_CTRL_reg, me_share_dma_ipme_wdma_ctrl_reg.regValue);
	rtd_outl(ME_SHARE_DMA_ME1_RDMA0_CTRL_reg, me_share_dma_me1_rdma0_ctrl_reg.regValue);
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_NUM_BL_reg, me_share_dma_ipme_wdma_num_bl_reg.regValue);
	rtd_outl(ME_SHARE_DMA_ME1_RDMA0_NUM_BL_reg, me_share_dma_me1_rdma0_num_bl_reg.regValue);
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_LSTEP_reg, me_share_dma_ipme_wdma_lstep_reg.regValue);
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MSTART0_reg, me_share_dma_ipme_wdma_mstart0_reg.regValue);
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MSTART1_reg, me_share_dma_ipme_wdma_mstart1_reg.regValue);
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MSTART2_reg, me_share_dma_ipme_wdma_mstart2_reg.regValue);
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MSTART3_reg, me_share_dma_ipme_wdma_mstart3_reg.regValue);
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MSTART4_reg, me_share_dma_ipme_wdma_mstart4_reg.regValue);
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MSTART5_reg, me_share_dma_ipme_wdma_mstart5_reg.regValue);
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MSTART6_reg, me_share_dma_ipme_wdma_mstart6_reg.regValue);
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MSTART7_reg, me_share_dma_ipme_wdma_mstart7_reg.regValue);
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MEND0_reg, me_share_dma_ipme_wdma_mend0_reg.regValue);
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MEND1_reg, me_share_dma_ipme_wdma_mend1_reg.regValue);
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MEND2_reg, me_share_dma_ipme_wdma_mend2_reg.regValue);
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MEND3_reg, me_share_dma_ipme_wdma_mend3_reg.regValue);
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MEND4_reg, me_share_dma_ipme_wdma_mend4_reg.regValue);
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MEND5_reg, me_share_dma_ipme_wdma_mend5_reg.regValue);
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MEND6_reg, me_share_dma_ipme_wdma_mend6_reg.regValue);
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MEND7_reg, me_share_dma_ipme_wdma_mend7_reg.regValue);
	rtd_outl(ME_SHARE_DMA_ME_DMA_WR_Rule_check_up_reg, me_share_dma_me_dma_wr_rule_check_up_reg.regValue);
	rtd_outl(ME_SHARE_DMA_ME_DMA_WR_Rule_check_low_reg, me_share_dma_me_dma_wr_rule_check_low_reg.regValue);
	rtd_outl(ME_SHARE_DMA_ME_DMA_RD_Rule_check_up_reg, me_share_dma_me_dma_rd_rule_check_up_reg.regValue);
	rtd_outl(ME_SHARE_DMA_ME_DMA_RD_Rule_check_low_reg, me_share_dma_me_dma_rd_rule_check_low_reg.regValue);
	rtd_outl(MVINFO_DMA_MVINFO_WDMA_MSTART0_reg, mvinfo_dma_mvinfo_wdma_mstart0_reg.regValue);
	rtd_outl(MVINFO_DMA_MVINFO_WDMA_MEND0_reg, mvinfo_dma_mvinfo_wdma_mend0_reg.regValue);
	rtd_outl(MVINFO_DMA_MVINFO_WR_Rule_check_up_reg, mvinfo_dma_mvinfo_wr_rule_check_up_reg.regValue);
	rtd_outl(MVINFO_DMA_MVINFO_WR_Rule_check_low_reg, mvinfo_dma_mvinfo_wr_rule_check_low_reg.regValue);
	rtd_outl(MVINFO_DMA_MVINFO_RD_Rule_check_up_reg, mvinfo_dma_mvinfo_rd_rule_check_up_reg.regValue);
	rtd_outl(MVINFO_DMA_MVINFO_RD_Rule_check_low_reg, mvinfo_dma_mvinfo_rd_rule_check_low_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV_SOURCE_MODE_reg, kme_dm_top2_mv_source_mode_reg.regValue);
	rtd_outl(KME_DM_TOP0_KME_ME_HNUM_reg, kme_dm_top0_kme_me_hnum_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_A0_reg, kme_dm_top1_kme_dm_top1_a0_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV01_AGENT_reg, kme_dm_top2_mv01_agent_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV02_AGENT_reg, kme_dm_top2_mv02_agent_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV04_AGENT_reg, kme_dm_top2_mv04_agent_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV05_AGENT_reg, kme_dm_top2_mv05_agent_reg.regValue);
	rtd_outl(MVINFO_DMA_MVINFO_WDMA_NUM_BL_reg, mvinfo_dma_mvinfo_wdma_num_bl_reg.regValue);
	rtd_outl(MVINFO_DMA_MVINFO_RDMA_NUM_BL_reg, mvinfo_dma_mvinfo_rdma_num_bl_reg.regValue);
	rtd_outl(KME_PQC_PQDC_KME_PQ_CMP_reg, kme_pqc_pqdc_kme_pq_cmp_reg.regValue);
	rtd_outl(KME_PQC_PQDC_KME_PQ_DECMP_reg, kme_pqc_pqdc_kme_pq_decmp_reg.regValue);
	rtd_outl(KME_PQC_PQDC_KME_PQ_CMP_BIT_reg, kme_pqc_pqdc_kme_pq_cmp_bit_reg.regValue);
	rtd_outl(KME_PQC_PQDC_KME_PQ_CMP_PAIR_reg, kme_pqc_pqdc_kme_pq_cmp_pair_reg.regValue);
	rtd_outl(KME_PQC_PQDC_KME_PQ_DECMP_PAIR_reg, kme_pqc_pqdc_kme_pq_decmp_pair_reg.regValue);
	rtd_outl(KME_DM_TOP0_KME_00_AGENT_reg, kme_dm_top0_kme_00_agent_reg.regValue);
	rtd_outl(KME_DM_TOP0_KME_01_AGENT_reg, kme_dm_top0_kme_01_agent_reg.regValue);
	rtd_outl(KME_DM_TOP0_KME_02_AGENT_reg, kme_dm_top0_kme_02_agent_reg.regValue);
	rtd_outl(KME_DM_TOP0_KME_03_AGENT_reg, kme_dm_top0_kme_03_agent_reg.regValue);
	rtd_outl(KME_DM_TOP0_KME_04_AGENT_reg, kme_dm_top0_kme_04_agent_reg.regValue);
	rtd_outl(KME_DM_TOP0_KME_05_AGENT_reg, kme_dm_top0_kme_05_agent_reg.regValue);
	rtd_outl(KME_DM_TOP0_KME06AGENT_reg, kme_dm_top0_kme06agent_reg.regValue);
	rtd_outl(KME_DM_TOP0_KME_07_AGENT_reg, kme_dm_top0_kme_07_agent_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_14_reg, kme_dm_top1_kme_dm_top1_14_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_1C_reg, kme_dm_top1_kme_dm_top1_1c_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_38_reg, kme_dm_top1_kme_dm_top1_38_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_40_reg, kme_dm_top1_kme_dm_top1_40_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_5C_reg, kme_dm_top1_kme_dm_top1_5c_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_64_reg, kme_dm_top1_kme_dm_top1_64_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_80_reg, kme_dm_top1_kme_dm_top1_80_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_88_reg, kme_dm_top1_kme_dm_top1_88_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV03_AGENT_reg, kme_dm_top2_mv03_agent_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV06_AGENT_reg, kme_dm_top2_mv06_agent_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV07_AGENT_reg, kme_dm_top2_mv07_agent_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV08_AGENT_reg, kme_dm_top2_mv08_agent_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV09_AGENT_reg, kme_dm_top2_mv09_agent_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV10_AGENT_reg, kme_dm_top2_mv10_agent_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV11_AGENT_reg, kme_dm_top2_mv11_agent_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV12_AGENT_reg, kme_dm_top2_mv12_agent_reg.regValue);
	rtd_outl(KME_DM_TOP0_KME_ME_RESOLUTION_reg, kme_dm_top0_kme_me_resolution_reg.regValue);
	rtd_outl(KME_DM_TOP0_KME_MV_RESOLUTION_reg, kme_dm_top0_kme_mv_resolution_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_90_reg, kme_dm_top1_kme_dm_top1_90_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_94_reg, kme_dm_top1_kme_dm_top1_94_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_98_reg, kme_dm_top1_kme_dm_top1_98_reg.regValue);
	rtd_outl(KME_DM_TOP1_KME_DM_TOP1_9C_reg, kme_dm_top1_kme_dm_top1_9c_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV02_RESOLUTION_reg, kme_dm_top2_mv02_resolution_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV01_RESOLUTION_reg, kme_dm_top2_mv01_resolution_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV04_RESOLUTION_reg, kme_dm_top2_mv04_resolution_reg.regValue);
	rtd_outl(KME_DM_TOP2_MV05_RESOLUTION_reg, kme_dm_top2_mv05_resolution_reg.regValue);
	rtd_outl(MVINFO_DMA_MVINFO_WDMA_CTRL_reg, mvinfo_dma_mvinfo_wdma_ctrl_reg.regValue);
	rtd_outl(MVINFO_DMA_MVINFO_RDMA_CTRL_reg, mvinfo_dma_mvinfo_rdma_ctrl_reg.regValue);
	rtd_outl(KME_PQC_PQDC_KME_PQ_CMP_ENABLE_reg, kme_pqc_pqdc_kme_pq_cmp_enable_reg.regValue);

}
void MEMC_ChangeSize_mvinfo_dma(void)
{
	// parameter declaration

	// register declaration

	// register read back

	// main function body

	// register write

}

#endif


#endif
