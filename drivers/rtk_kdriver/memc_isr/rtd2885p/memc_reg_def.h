#ifndef __MEMC_REG_DEF_H__
#define __MEMC_REG_DEF_H__


//page 0X
#include <rbus/kmc_top_reg.h>				//00
#include <rbus/ippre_reg.h>				//01
#include <rbus/pqc_pqdc_reg.h>				//02, 08
#include <rbus/decpr_ptg_reg.h>			//03
#include <rbus/mc_dma_reg.h>				//04, 1E
#include <rbus/ippre1_reg.h>				//05
#include <rbus/mc_reg.h>					//06
#include <rbus/mc2_reg.h>					//07
#include <rbus/lbmc_reg.h>					//09
#include <rbus/bbd_reg.h>					//0A
#include <rbus/kmc_meter_top_reg.h>		//0B
#include <rbus/mc3_reg.h>					//0C

//page 2X

#include <rbus/kme_top_reg.h>				// 20
#include <rbus/mvinfo_dma_reg.h>			// 21
#include <rbus/kme_vbuf_top_reg.h>			// 22
#include <rbus/kme_me1_top2_reg.h>		// 23
#include <rbus/kme_me1_top3_reg.h>		// 24
#include <rbus/kme_me1_top4_reg.h>		// 25
#include <rbus/kme_me1_top5_reg.h>		// 26
#include <rbus/me_share_dma_reg.h>		// 27, 53, 69
#include <rbus/kme_me1_top10_reg.h>		// 28
#include <rbus/kme_lbme_top_reg.h>		// 29
#include <rbus/kme_me2_calc1_reg.h>		// 2A
#include <rbus/kme_logo2_reg.h>			// 2B
#include <rbus/kme_ipme_reg.h>			// 2C
#include <rbus/kme_ipme1_reg.h>			// 2D
#include <rbus/kme_logo0_reg.h>			// 2E
#include <rbus/kme_logo1_reg.h>			// 2F

//page 3X
#include <rbus/kme_dm_top0_reg.h>			// 30
#include <rbus/kme_dm_top1_reg.h>			// 31
#include <rbus/kme_dm_top2_reg.h>			// 32
#include <rbus/kme_dehalo3_reg.h>			// 33
#include <rbus/kme_me1_top0_reg.h>		// 34
#include <rbus/kme_me1_top1_reg.h>		// 35
#include <rbus/kme_lbme2_top_reg.h>		// 36
#include <rbus/kme_me2_vbuf_top_reg.h>	// 37
#include <rbus/kme_me2_calc0_reg.h>		// 38
#include <rbus/kme_me1_top6_reg.h>		// 39
#include <rbus/kme_me1_top7_reg.h>		// 3A
#include <rbus/kme_me1_top8_reg.h>		// 3B
#include <rbus/kme_me1_top9_reg.h>		// 3C
#include <rbus/kme_dehalo_reg.h>			// 3D
#include <rbus/kme_dehalo2_reg.h>			// 3E

//page 4X
#include <rbus/kpost_top_reg.h>			// 40
#include <rbus/crtc1_reg.h>					// 41
#include <rbus/kme_logo3_reg.h>			// 42
#include <rbus/kphase2_reg.h>				// 43
#include <rbus/kphase_reg.h>				// 44
#include <rbus/hardware_reg.h>				// 45
#include <rbus/software_reg.h>				// 46
#include <rbus/software1_reg.h>			// 47
#include <rbus/software2_reg.h>			// 48
#include <rbus/software3_reg.h>			// 49

//page 5X
#include <rbus/dbus_wrapper_reg.h>			// 50, 51
//#include <rbus/me_share_dma_reg.h>		// 53
#include <rbus/kme_ipme2_reg.h>			// 54
#include <rbus/kme_me1_top11_reg.h>		// 56
#include <rbus/kme_dehalo6_reg.h>			// 57
#include <rbus/kme_pqc_pqdc_reg.h>		// 58, 59
#include <rbus/kme_dehalo4_reg.h>			// 5A
#include <rbus/kme_dehalo5_reg.h>			// 5B, 5C
#include <rbus/kme_algo_reg.h>				// 5D
#include <rbus/kme_me1_bg0_reg.h>			// 5E
#include <rbus/kme_me1_bg1_reg.h>			// 5F

//page 6X
#include <rbus/kme_bist_reg.h>				// 60
#include <rbus/kmc_bist_bisr_reg.h>			// 61
#include <rbus/kmc_bi_reg.h>				// 62, 63
#include <rbus/kme_algo2_reg.h>			// 64
#include <rbus/kme_me0_reg.h>				// 65
#include <rbus/kme_me1_bg2_reg.h>			// 66
#include <rbus/kme_dm_top3_reg.h>			// 67, 68
//#include <rbus/me_share_dma_reg.h>		// 69
#include <rbus/kme_algo3_reg.h>			// 6A

//others
#include <rbus/sys_reg_reg.h>			//for MEMC CLK control
#include <rbus/ppoverlay_reg.h>	
#include <rbus/timer_reg.h>	
#include <rbus/memc_mux_reg.h>
#include <rbus/kme_dehalo4_reg.h>
#endif

/*-----------------------------------------------------------------------------------------*/

