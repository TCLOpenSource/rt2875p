#ifndef __MEMC_REG_DEF_H__
#define __MEMC_REG_DEF_H__


//page 0X
#include <rbus/kmc_top_reg.h>			//00
#include <rbus/ippre_reg.h>				//01
#include <rbus/pqc_pqdc_reg.h>			//02, 08 and 0D
#include <rbus/kmc_decpr_ptg_reg.h>			//03
#include <rbus/mc_dma_reg.h>			//04 and 1E
#include <rbus/ippre1_reg.h>			//05
#include <rbus/mc_reg.h>				//06
#include <rbus/mc2_reg.h>				//07
#include <rbus/lbmc_reg.h>				//09
#include <rbus/bbd_reg.h>				//0A
#include <rbus/kmc_meter_top_reg.h>		//0B
#include <rbus/mc3_reg.h>				//0C

//page 2X
#include <rbus/kme_top_reg.h>			// 20
#include <rbus/mvinfo_dma_reg.h>		// 21
#include <rbus/kme_vbuf_top_reg.h>		// 22
#include <rbus/kme_me1_top1_reg.h>		// 23
#include <rbus/kme_me1_top2_reg.h>		// 23
#include <rbus/kme_me1_top3_reg.h>		// 24
#include <rbus/kme_me1_top4_reg.h>		// 25
#include <rbus/kme_me1_top5_reg.h>		// 26
#include <rbus/me_share_dma_reg.h>		// 27
#include <rbus/kme_me1_top10_reg.h>		// 28
#include <rbus/kme_lbme_top_reg.h>		// 29
#include <rbus/kme_me2_calc1_reg.h>		// 2A
#include <rbus/kme_logo2_reg.h>			// 2B
#include <rbus/kme_ipme_reg.h>			// 2C
#include <rbus/kme_ipme1_reg.h>			// 2D
#include <rbus/kme_logo0_reg.h>			// 2E
#include <rbus/kme_logo1_reg.h>			// 2F

//page 3X
#include <rbus/kme_dm_top0_reg.h>		// 30
#include <rbus/kme_dm_top1_reg.h>		// 31
#include <rbus/kme_dm_top2_reg.h>		// 32
#include <rbus/kme_dehalo3_reg.h>		// 33
#include <rbus/kme_me1_top0_reg.h>		// 34
#include <rbus/kme_lbme2_top_reg.h>		// 36
#include <rbus/kme_me2_vbuf_top_reg.h>	// 37
#include <rbus/kme_me2_calc0_reg.h>		// 38
#include <rbus/kme_me1_top6_reg.h>		// 39
#include <rbus/kme_me1_top7_reg.h>		// 3A
#include <rbus/kme_me1_top8_reg.h>		// 3B
#include <rbus/kme_me1_top9_reg.h>		// 3C
#include <rbus/kme_dehalo_reg.h>		// 3D
#include <rbus/kme_dehalo2_reg.h>		// 3E

//page 4X
#include <rbus/kpost_top_reg.h>			// 40
#include <rbus/crtc1_reg.h>				// 41
#include <rbus/kphase_reg.h>			// 44
#include <rbus/hardware_reg.h>			// 45
#include <rbus/software_reg.h>			// 46
#include <rbus/software1_reg.h>			// 47
#include <rbus/software2_reg.h>			// 48
#include <rbus/software3_reg.h>			// 49


//page 5X
#include <rbus/dbus_wrapper_reg.h>		// 50 and 51
//#include <rbus/kme_me1_top11_reg.h>	// 56
//#include <rbus/kme_dehalo6_reg.h>		// 57
#include <rbus/kme_pqc_pqdc_reg.h>		// 58
#include <rbus/kme_dehalo4_reg.h>		// 5A
#include <rbus/kme_dehalo5_reg.h>		// 5B
#include <rbus/kme_algo_reg.h>			// 5D
#include <rbus/kme_me1_bg0_reg.h>		// 5E
#include <rbus/kme_me1_bg1_reg.h>		// 5F

//page 6X
#include <rbus/kme_bist_reg.h>			// 60
#include <rbus/kmc_bist_bisr_reg.h>		// 61
#include <rbus/kmc_bi_reg.h>			// 62

//others
#include <rbus/sys_reg_reg.h>			//for MEMC CLK control
#include <rbus/ppoverlay_reg.h>	
#include <rbus/timer_reg.h>	
#include <rbus/memc_mux_reg.h>
#include <memc_isr/scalerMEMC.h>

#ifndef MARK2_HALO
#define MARK2_HALO (1)
#endif

#if (IC_K8LP || MARK2_HALO)
#define 	KME_DEHALO6_dh_logo_ctrl_func_en_tmv_mode_reg	0xB809E700
#define 	KME_DEHALO6_dh_logo_ctrl_1_rg3_reg				0xB809E704
#define 	KME_DEHALO6_dh_logo_ctrl_1_rg4_reg				0xB809E708
#define 	KME_DEHALO6_dh_logo_ctrl_1_rg5_reg				0xB809E70C
#define 	KME_DEHALO6_dh_logo_ctrl_1_rg6_reg				0xB809E710
#define 	KME_DEHALO6_dh_logo_ctrl_1_rg7_reg				0xB809E714
#define 	KME_DEHALO6_dh_logo_ctrl_1_rg8_reg				0xB809E718
#define 	KME_DEHALO6_dh_logo_ctrl_1_rg9_reg				0xB809E71C
#define 	KME_DEHALO6_dh_logo_ctrl_1_rga_reg				0xB809E720
#define 	KME_DEHALO6_dh_logo_ctrl_1_rgb_reg				0xB809E724
#define 	KME_DEHALO6_dh_logo_ctrl_1_rgc_reg				0xB809E728
#define 	KME_DEHALO6_dh_logo_ctrl_1_rgd_reg				0xB809E72C
#define 	KME_DEHALO6_dh_logo_ctrl_1_rge_reg				0xB809E730
#define 	KME_DEHALO6_dh_logo_ctrl_2_rg3_rg4_reg			0xB809E734
#define 	KME_DEHALO6_dh_logo_ctrl_2_rg5_rg6_reg			0xB809E738
#define 	KME_DEHALO6_dh_logo_ctrl_2_rg7_rg8_reg			0xB809E73C
#define 	KME_DEHALO6_dh_logo_ctrl_2_rg9_rga_reg			0xB809E740
#define 	KME_DEHALO6_dh_logo_ctrl_2_rgb_rgc_reg			0xB809E744
#define 	KME_DEHALO6_dh_logo_ctrl_2_rgd_rge_reg			0xB809E748

#define	KME_DEHALO5_PHMV_FIX_33_reg					0xB809EBD0
#define	KME_DEHALO5_PHMV_FIX_34_reg					0xB809EBD4
#define	KME_DEHALO5_PHMV_FIX_35_reg					0xB809EBD8
#define 	KME_DEHALO5_PHMV_FIX_36_reg 					0xB809EBDC
#define	KME_DEHALO5_PHMV_FIX_37_reg					0xB809EBE0
#define	KME_DEHALO5_PHMV_FIX_38_reg 					0xB809EBE4
#define	KME_DEHALO5_PHMV_FIX_39_reg 					0xB809EBE8
#define 	KME_DEHALO5_PHMV_FIX_3A_reg 					0xB809EBEC
#define 	KME_DEHALO5_PHMV_FIX_3B_reg 					0xB809EBF0
#define 	KME_DEHALO5_PHMV_FIX_3C_reg 					0xB809EBF4
#define 	KME_DEHALO5_PHMV_FIX_3D_reg 					0xB809EBF8
#define 	KME_DEHALO5_PHMV_FIX_3E_reg 					0xB809EBFC

#define 	KME_DEHALO5_PHMV_FIX_3F_reg 					0xB809EC00
#define 	KME_DEHALO5_PHMV_FIX_40_reg 					0xB809EC04
#define 	KME_DEHALO5_PHMV_FIX_41_reg 					0xB809EC08
#define 	KME_DEHALO5_PHMV_FIX_42_reg 					0xB809EC0C
#define 	KME_DEHALO5_PHMV_FIX_43_reg 					0xB809EC10
#define 	KME_DEHALO5_PHMV_FIX_44_reg 					0xB809EC14

#define 	KME_ME1_BG1_ME_BG_S4_1_reg					0xB809EF80
#define 	KME_ME1_BG1_ME_BG_S4_2_reg					0xB809EF84
#define 	KME_ME1_BG1_ME_BG_S4_3_reg					0xB809EF88
#define 	KME_ME1_BG1_ME_BG_S4_4_reg					0xB809EF8C
#define 	KME_ME1_BG1_ME_BG_S4_5_reg					0xB809EF90
#define 	KME_ME1_BG1_ME_BG_S4_6_reg					0xB809EF94
#define 	KME_ME1_BG1_ME_BG_2x2_1_reg					0xB809EF98
#define 	KME_ME1_BG1_ME_BG_2x2_2_reg					0xB809EF9C
#define 	KME_ME1_BG1_ME_BG_2x2_3_reg					0xB809EFA0
#define 	KME_ME1_BG1_ME_BG_S3_0_reg  					0xB809EFA4
#define	KME_ME1_BG1_ME_BG_TH0_reg					0xB809EFA8
#define 	KME_ME1_BG1_ME_BG_TH1_reg   					0xB809EFAC
#define	KME_ME1_BG1_ME_BG_TH2_reg					0xB809EFB0

#endif



#endif

/*-----------------------------------------------------------------------------------------*/
