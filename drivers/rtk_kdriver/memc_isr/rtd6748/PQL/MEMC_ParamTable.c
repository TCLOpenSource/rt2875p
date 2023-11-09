//#include <memc_isr/include/memc_lib.h>
#include "memc_isr/PQL/PQLPlatformDefs.h"
#include "memc_isr/PQL/PQLContext.h"
//#ifdef H5C2
//#include "../common/include/rbus/mdomain_disp_main_reg.h"  //mdomain_disp_reg.h //spec_change
//#endif
//#include <tvscalercontrol/scaler/scalerstruct.h>
//#include "memc_isr/MID/mid_mode.h"
//#include <tvscalercontrol/panel/panelapi.h>
//#include "memc_reg_def.h"
//#include "memc_isr/include/PQLAPI.h"
#include "memc_isr/PQL/MEMC_ParamTable.h"

const _MEMC_PARAM_TABLE MEMC_PARAM_TABLE = {

	//SAD_SHIFT_PARAM 
	//get the SAD_SHIFT_INFO method: https://wiki.realtek.com/x/9Rs2Eg
	{
	//top_rim	sad_offset
		{10,	0x3bfe2}, /*_PQL_OUT_1920x1080*/
		{10,	0x3bfe2}, /*_PQL_OUT_1920x540*/
		{10,	0x3bfe2}, /*_PQL_OUT_3840x2160*/
		{10,	0x3bfe2}, /*_PQL_OUT_3840x1080*/
		{10,	0x3bfe2}, /*_PQL_OUT_3840x540*/
		{10,	0x3bfe2}, /*_PQL_OUT_768x480*/
		{10,	0x3bfe2}, /*_PQL_OUT_1280x720*/
		{10,	0x3bfe2}, /*_PQL_OUT_7680x4320*/
		{ 8,	0x27FEC}, /*_PQL_OUT_2560x1440*/
	},
	
	//RIM_Param
	{
	//												RimCtrl_Param																	  BBD_ACTIVE_TH_Param
	//	height width		bound(T,B,R,L)		   divide	slowin_min_shift   RimDiffTh_cof	apl_size	scale(H,V)		H_PRECISE	H_SKETCHY	V_PRECISE	V_SKETCHY
		{{1920, 1080,	{180,  900, 1600,  320},	2, 1,		12, 12, 			1,			64800,		2, 2},			{64,		256,		32, 		128}},	/*_PQL_OUT_1920x1080*/	
		{{1920,	540,	{ 90,  450,  160, 3200},	2, 0,		12,  6, 			1,			64800,		2, 1},			{64,		256,		16, 		64 }},	/*_PQL_OUT_1920x540*/	
		{{3840, 2160,	{360, 1800, 3200,  640},	3, 2,		24, 24, 			1,			64800,		4, 4},			{128,		512,		64, 		256}},	/*_PQL_OUT_3840x2160*/	
		{{3840, 1080,	{180,  900, 3200,  640},	3, 1,		24, 12, 			1,			64800,		4, 2},			{128,		512,		32, 		128}},	/*_PQL_OUT_3840x1080*/	
		{{3840,	540,	{ 90,  450, 3200,  640},	3, 0,		24,  6, 			1,			64800,		4, 1},			{128,		512,		16, 		64 }},	/*_PQL_OUT_3840x540*/	
		{{ 768,	480,	{ 80,  400,  640,  128},	0, 0,		 5,  5, 			1,			64800,		1, 1},			{26,		102,		14, 		57 }},	/*_PQL_OUT_768x480*/	
		{{1280,	720,	{120,  600, 1066,  214},	1, 0,		 8,  8, 			1,			64800,		1, 1},			{43,		171,		21, 		85 }},	/*_PQL_OUT_1280x720*/	
		{{7680, 4320,	{720, 3600, 6400, 1280},	4, 3,		48, 48, 			2,			64800,		8, 8},			{256,		1024,		128,		512}},	/*_PQL_OUT_7680x4320*/	
		{{2560, 1440,	{240, 1200, 2100,  420},	3, 2,		24, 24, 			1,			64800,		3, 3},			{96,		384,		48, 		192}},	/*_PQL_OUT_2560x1440*/	
	},
	
	//LOGO_Param
	{	
	// 	  LOGO_DETECT_EDGE_Param			 MC_LOGO_EROSION_TH_Param
	//	hor 	P45 	ver 	N45			panning	fast-motion   normal
		{{0x3,	0x6,	0x4,	0xb},		{3, 		6,			6}},	/*_PQL_OUT_1920x1080*/
		{{0x3,	0x6,	0x4,	0xb},		{3, 		6,			6}},	/*_PQL_OUT_1920x540*/
		{{0x3,	0x6,	0x4,	0xb},		{3, 		6,			6}},	/*_PQL_OUT_3840x2160*/ //{0x9, 0xc, 0x8, 0xb}
		{{0x3,	0x6,	0x4,	0xb},		{3, 		6,			6}},	/*_PQL_OUT_3840x1080*/
		{{0x3,	0x6,	0x4,	0xb},		{3, 		6,			6}},	/*_PQL_OUT_3840x540*/
		{{0x3,	0x6,	0x4,	0xb},		{3, 		6,			6}},	/*_PQL_OUT_768x480*/
		{{0x3,	0x6,	0x4,	0xb},		{3, 		6,			6}},	/*_PQL_OUT_1280x720*/
		{{0x3,	0x6,	0x4,	0xb},		{3, 		6,			6}},	/*_PQL_OUT_7680x4320*/
		{{0xa,	0x15,	0xe,	0x26},		{5, 		6,			8}},	/*_PQL_OUT_2560x1440*/
	},
	
	//FB_Param
	{
	//			FB_BAD_REGION_TH					FB_TC_TH					MC_FBLVL_FILTER_PARAM
	//	 low th 	mid th		high th 	  low th	high th 	enable	mode	cut 	th		avg_th	num
		{{0xe5a8,	0x1C710,	0x4A710},	{0x177e20,	0x1d7e20},	{0x1,	0x2,	0x1e,	0x07,	0x04,	0x8}}, /*_PQL_OUT_1920x1080*/
		{{0xe5a8,	0x1C710,	0x4A710},	{0x177e20,	0x1d7e20},	{0x1,	0x2,	0x1e,	0x07,	0x04,	0x8}}, /*_PQL_OUT_1920x540*/
		{{0xDF51,	0x3A651,	0x5CF28},	{0x177e20,	0x1d7e20},	{0x1,	0x0,	0x1e,	0x0c,	0x04,	0xf}}, /*_PQL_OUT_3840x2160*/
//		{{0x22E4C,	0x91fcc,	0xe85e6},	{0x177e20,	0x1d7e20},	{0x1,	0x0,	0x1e,	0x0c,	0x04,	0xf}}, /*_PQL_OUT_3840x2160*/
//		{{0x1bea3,	0x74ca3,	0xb9e51},	{0x1071e3,	0x1d7e20},	{0x1,	0x0,	0x1e,	0x0c,	0x04,	0xf}}, /*_PQL_OUT_3840x2160*/
		{{0xe5a8,	0x1C710,	0x4A710},	{0x177e20,	0x1d7e20},	{0x1,	0x0,	0x1e,	0x0c,	0x04,	0xf}}, /*_PQL_OUT_3840x1080*/
		{{0xe5a8,	0x1C710,	0x4A710},	{0x177e20,	0x1d7e20},	{0x1,	0x0,	0x1e,	0x0c,	0x04,	0xf}}, /*_PQL_OUT_3840x540*/
		{{0xe5a8,	0x1C710,	0x4A710},	{0x177e20,	0x1d7e20},	{0x1,	0x2,	0x1e,	0x07,	0x04,	0x8}}, /*_PQL_OUT_768x480*/
		{{0xe5a8,	0x1C710,	0x4A710},	{0x177e20,	0x1d7e20},	{0x1,	0x2,	0x1e,	0x07,	0x04,	0x8}}, /*_PQL_OUT_1280x720*/
		{{0xe5a8,	0x1C710,	0x4A710},	{0x177e20,	0x1d7e20},	{0x1,	0x0,	0x1e,	0x0c,	0x04,	0xf}}, /*_PQL_OUT_7680x4320*/
		{{0x257d9,	0x4afb2,	0xbc73d},	{0x177e20,	0x1d7e20},	{0x1,	0x0,	0x1e,	0x0c,	0x04,	0xf}}, /*_PQL_OUT_2560x1440*/
	},
	
	//SC_Param
	{
	//		scCtrl_tuning_th
	//	pure_video	 	panning
		{{0x980000,		0x400000}}, /*_PQL_OUT_1920x1080*/
		{{0x980000,		0x400000}}, /*_PQL_OUT_1920x540*/
		{{0x8e7fba,		0x3a497c}}, /*_PQL_OUT_3840x2160*/
		{{0x980000,		0x400000}}, /*_PQL_OUT_3840x1080*/
		{{0x980000,		0x400000}}, /*_PQL_OUT_3840x540*/
		{{0x980000,		0x400000}}, /*_PQL_OUT_768x480*/
		{{0x980000,		0x400000}}, /*_PQL_OUT_1280x720*/
		{{0x980000,		0x400000}}, /*_PQL_OUT_7680x4320*/
		{{0x1370d92, 	0x4dcd6c}}, /*_PQL_OUT_2560x1440*/
	},
	
	//MV_SEARCHRANGE_Param
	{
	//		  SEARCH_RANGE_CLIP
	// 	   oversearch	   normal
		{ {{0x2d, 0x2d}, {0x2a, 0x2a}} }, /*_PQL_OUT_1920x1080*/	
		{ {{0x2d, 0x2d}, {0x2a, 0x2a}} }, /*_PQL_OUT_1920x540*/	
		{ {{0x2d, 0x2d}, {0x2a, 0x2a}} }, /*_PQL_OUT_3840x2160*/	
		{ {{0x2d, 0x2d}, {0x2a, 0x2a}} }, /*_PQL_OUT_3840x1080*/	
		{ {{0x2d, 0x2d}, {0x2a, 0x2a}} }, /*_PQL_OUT_3840x540*/	
		{ {{0x2d, 0x2d}, {0x2a, 0x2a}} }, /*_PQL_OUT_768x480*/	
		{ {{0x2d, 0x2d}, {0x2a, 0x2a}} }, /*_PQL_OUT_1280x720*/	
		{ {{0x2d, 0x2d}, {0x2a, 0x2a}} }, /*_PQL_OUT_7680x4320*/	
		{ {{0x1e, 0x1e}, {0x1c, 0x1c}} }, /*_PQL_OUT_2560x1440*/	
	},

	//MC_Param
	{
	//mc_vartap_sel
		{0}, /*_PQL_OUT_1920x1080*/
		{0}, /*_PQL_OUT_1920x540*/
		{2}, /*_PQL_OUT_3840x2160*/
		{2}, /*_PQL_OUT_3840x1080*/
		{2}, /*_PQL_OUT_3840x540*/
		{0}, /*_PQL_OUT_768x480*/
		{0}, /*_PQL_OUT_1280x720*/
		{2}, /*_PQL_OUT_7680x4320*/
		{1}, /*_PQL_OUT_2560x1440*/
	},

	//DEHALO_PARAM
	{
	//dh_phflt_med9flt_data_sel
		{1}, /*_PQL_OUT_1920x1080*/
		{1}, /*_PQL_OUT_1920x540*/
		{2}, /*_PQL_OUT_3840x2160*/
		{2}, /*_PQL_OUT_3840x1080*/
		{2}, /*_PQL_OUT_3840x540*/
		{1}, /*_PQL_OUT_768x480*/
		{1}, /*_PQL_OUT_1280x720*/
		{2}, /*_PQL_OUT_7680x4320*/
		{2}, /*_PQL_OUT_2560x1440*/
	},
	
	//BI_PARAM
	{
	//bi_blk_res_sel
		{1}, /*_PQL_OUT_1920x1080*/
		{1}, /*_PQL_OUT_1920x540*/
		{0}, /*_PQL_OUT_3840x2160*/
		{0}, /*_PQL_OUT_3840x1080*/
		{0}, /*_PQL_OUT_3840x540*/
		{1}, /*_PQL_OUT_768x480*/
		{1}, /*_PQL_OUT_1280x720*/
		{0}, /*_PQL_OUT_7680x4320*/
		{0}, /*_PQL_OUT_2560x1440*/
	},
};

