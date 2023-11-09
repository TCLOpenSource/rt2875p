#ifndef _FRC_LOGODETECT_H_
#define _FRC_LOGODETECT_H_


#define PIX_LOGO_RG_PIXCNT  (120*135)
#define PIX_LOGO_PIXCNT     (960*540)
#define BLK_LOGO_RG_BLKCNT  (30*34)
#define BLK_LOGO_BLKCNT     (240*135)
#define RG_15               (15)

#define LOGO_DEBUG_PRINTF   (0)

typedef enum
{
	LOGO_RGN_CORNER,
	LOGO_RGN_TOP,
	LOGO_RGN_BOT,
	LOGO_RGN_MAX
} _MEMC_LOGO_REGION;

/* Param Definition*/
typedef struct
{
	// Logo Registers
	unsigned int blkhsty_pth[15];
	unsigned int blkhsty_nth[15];
	unsigned int blksamethr_l[15];
	unsigned int blksamethr_a[15];
	unsigned int blksamethr_h[15];
	unsigned int reg_mc_logo_vlpf_en;
	unsigned int reg_km_logo_iir_alpha;
	unsigned int reg_mc_logo_en;

	unsigned int reg_km_logo_blkgrdsum2_th;
	unsigned int reg_km_logo_blkadphstystep_en;
	unsigned int reg_km_logo_blkhsty_nstep;
	unsigned int reg_km_logo_blkhsty_pstep;

	unsigned int dh_logo_bypass[15][5]; // hor/ver/smv/mvd/tmv
	unsigned int dh_logo_RgnThr[7][3];

	// Others Register
	unsigned int me1_ip_dc_obme_mode_sel;
	unsigned int me1_ip_dc_obme_mode;
	unsigned int me1_ip_ac_obme_mode;
	unsigned int me1_pi_dc_obme_mode_sel;
	unsigned int me1_pi_dc_obme_mode;
	unsigned int me1_pi_ac_obme_mode;
	unsigned int me2_1st_dc_obme_mode_sel;
	unsigned int me2_1st_dc_obme_mode;
	unsigned int me2_1st_ac_obme_mode;
	unsigned int me2_2nd_dc_obme_mode_sel;
	unsigned int me2_2nd_dc_obme_mode;
	unsigned int me2_2nd_ac_obme_mode;

	unsigned int u6_logo_blklogopostdlt_th;
	unsigned int u6_logo_blklogopost_ero_th;
	unsigned int u6_logo_pxllogopostdlt_th;
} _MEMC_LOGO_DEFAULT_VAL;

typedef struct
{
	unsigned char  u1_logo_pql_en                    ;
	unsigned char  u1_logo_sw_clr_en                 ;
	unsigned char  u1_logo_rg_clr_en                 ;
	unsigned char  u1_logo_glb_clr_en                ;
	unsigned char  u1_logo_left_half_clr_en          ;
	unsigned char  u1_logo_right_half_clr_en         ;
	unsigned char  u1_logo_top_half_clr_en           ;
	unsigned char  u1_logo_bot_half_clr_en           ;
	unsigned char  u1_logo_sc_alpha_ctrl_en			 ;
	unsigned char  u1_logo_sc_logo_clr_en            ;
	//////////////////////////////////////////////////////////////////////////

	unsigned char  u8_clr_rg_thr                     ;
	unsigned char  u8_clr_rg_thr_l                   ;
	unsigned char  u8_clr_glb_thr                    ;
	unsigned char  u8_clr_half_thr                   ;
	unsigned char  u6_rgClr_holdtime 		         ;
	unsigned char  u6_glbClr_holdtime		         ;
	unsigned char  u4_blk_pix_merge_type             ;
	unsigned char  u5_iir_alpha				         ;

	unsigned char  u1_RgDhClr_thr_en                 ;
	unsigned int u16_RgDhClr_thr_l                   ;
	unsigned int u16_RgDhClr_thr_h                   ;
	unsigned int u5_RgDhClr_cnt_th                   ;

	unsigned char  u1_RgHstyClr_en                    ;
	unsigned char  u1_GlbHstyClr_en                   ;
	unsigned char  u1_RgHstyClr_Sc_en                 ;

	unsigned int u16_RgHstyClr_mv_thr               ;
	unsigned int u12_RgHstyClr_mv_cnt_thr          ;
	unsigned int u12_RgHstyClr_mv_unconf_thr       ;
	unsigned int u10_RgHstyClr_Rsad_thr             ;

	unsigned int u16_RgHstyClr_Gmv_thr               ;
	unsigned char  u8_RgHstyClr_Gmv_cnt_thr          ;
	unsigned int u12_RgHstyClr_Gmv_unconf_thr       ;
	unsigned int u10_RgHstyClr_Gsad_thr             ;

	unsigned char  u1_RgHstyClr_glbclr_close_en       ;

	unsigned char  u1_RgDhClr_static_en;

	unsigned char   u8_rg_dh_clr_lgclr_hold_frm;
	unsigned char   u8_rg_dh_clr_static_hold_frm;

	unsigned char  u1_logo_sc_detect_en;
	unsigned char  u1_logo_sc_saddif_en;
	unsigned char  u1_logo_sc_Gmv_en;
	unsigned char  u1_logo_sc_dtldif_en;
	unsigned int u31_logo_sc_saddiff_th;
	unsigned char  u6_logo_sc_Gmv_cnt_th;
	unsigned int u12_logo_sc_Gmv_unconf_th;
	unsigned int u20_logo_sc_dtldif_th;
	unsigned int u8_logo_sc_hold_frm;

	unsigned char u1_logo_static_bg_en;
	unsigned char u6_logo_static_bg_th_l;
	unsigned char u6_logo_static_bg_th_h;

	unsigned char u1_param_reg_scdetct;

	unsigned char u1_logo_netflix_detct_en;

	unsigned int u20_logo_netflix_detect_lt_th_l;
	unsigned int u20_logo_netflix_detect_lt_th_h;
	unsigned int u20_logo_netflix_detect_rt_th_l;
	unsigned int u20_logo_netflix_detect_rt_th_h;

	unsigned int u20_logo_netflix_detect_lb_th_l;
	unsigned int u20_logo_netflix_detect_lb_th_h;
	unsigned int u20_logo_netflix_detect_rb_th_l;
	unsigned int u20_logo_netflix_detect_rb_th_h;

	unsigned char  u8_logo_netflix_hold_frm;

	unsigned char  u1_param_reg_netflix_detect;

	unsigned char  u1_param_reg_lg16s_patch;

	unsigned char  u1_logo_lg16s_patch_en;
	unsigned char  u1_logo_lg16s_patch_Gapl_en;
	unsigned char  u1_logo_lg16s_patch_Gdtl_en;
	unsigned char  u1_logo_lg16s_patch_Gsad_en;
	unsigned char  u1_logo_lg16s_patch_Gmv_en;
	unsigned char  u1_logo_lg16s_patch_Rdtl_num_en;

	unsigned int u10_logo_lg16s_patch_Gapl_th_l;
	unsigned int u10_logo_lg16s_patch_Gapl_th_h;

	unsigned int u25_logo_lg16s_patch_Gdtl_th_l;
	unsigned int u25_logo_lg16s_patch_Gdtl_th_h;

	unsigned int u28_logo_lg16s_patch_Gsad_th_l;
	unsigned int u28_logo_lg16s_patch_Gsad_th_h;

	unsigned int u11_logo_lg16s_patch_Gmvx_th_l;
	unsigned int u11_logo_lg16s_patch_Gmvx_th_h;
    unsigned int u11_logo_lg16s_patch_Gmvy_th;
	unsigned char  u6_logo_lg16s_patch_Gmv_cnt_th;
	unsigned int u12_logo_lg16s_patch_Gmv_unconf_th;

	unsigned int u20_logo_lg16s_patch_Rdtl_th_l;
	unsigned int u20_logo_lg16s_patch_Rdtl_th_h;
	unsigned char  u6_logo_lg16s_patch_Rdtl_num_th;

	unsigned char  u8_logo_lg16s_patch_hold_frm;

	unsigned int u16_RgDhadp_Rimunbalance_thr_l;
	unsigned int u16_RgDhadp_Rimunbalance_thr_h ;
	unsigned char  u8_RgDhadp_Rimunbalance_hold_frm;


	unsigned char u1_lg_UXN_patch_en;

	unsigned char u1_logo_sc_FastDet_en;
	unsigned char u8_logo_sc_FastDet_rstholdfrm;

	// for SLD
	unsigned int u1_logo_sld_debug_print_en;
	unsigned int u1_logo_sld_debug_print_period;

	unsigned int u8_logo_rglt_pxldf_apl_thr_l;
	unsigned int u8_logo_rglt_pxldf_apl_thr_h;
	unsigned int u8_logo_rglt_pxldf_val_thr_l;
	unsigned int u8_logo_rglt_pxldf_val_thr_h;

	unsigned int u1_logo_rgclr_debug_print;
	unsigned int u1_logo_rgclr_debug_print_period;

	unsigned char u1_logo_rgclr_apl_en;
	unsigned int u32_logo_rgclr_apl_thr;

	unsigned char u1_logo_rgclr_dtl_en;
	unsigned int u32_logo_rgclr_dtl_thr;
	unsigned int u32_logo_rgclr_apl2dtl_thr;

	unsigned char u1_logo_rgclr_cnt_en;
	unsigned int u32_logo_rgclr_cnt_thr;

	unsigned char u1_logo_rgclr_cmp_en;
	unsigned char u8_logo_rgclr_logo_mv_thr;
	unsigned int u32_logo_rgclr_cmp_gain_thr;
	unsigned int u32_logo_rgclr_cmp_ofst_thr;

	//unsigned char  u1_logo_dynY_en                   ;
	//unsigned char  u1_logo_dynBlkTh_en               ;
	//unsigned char  u1_logo_dynPixTh_en               ;
	//unsigned char  u1_logo_dynBlkStep_en             ;
	//unsigned char  u1_logo_dynPixStep_en             ;
	//unsigned char  u1_logo_dynAlpha_en               ;
	//unsigned char  u1_logo_dynDelogo_en              ;
	//unsigned char u1_logo_pixRgClr_en 				 ;
	//unsigned char u1_logo_pixGlbClr_en				 ;
	//unsigned char u1_logo_blkGlbClrHsty_en			 ;
	//unsigned char u1_logo_pixGlbClrHsty_en			 ;
	//unsigned char u1_logo_blkRgClrHsty_en 			 ;
	//unsigned char u1_logo_pixRgClrHsty_en 			 ;
	//unsigned char u1_logo_delogoGlbClr_en 			 ;
	//unsigned char u1_logo_delogoRgClr_en  			 ;
	//unsigned char u1_logo_delogoGlbClrHsty_en		 ;
	//unsigned char u1_logo_delogoRgClrHsty_en 		 ;
	//unsigned char u1_logo_SCalphaClr_en				 ;
	//unsigned char u1_logo_SCHstyClr_en 				 ;
	//unsigned char u1_logo_SCLogoClr_en 				 ;

	//unsigned char  u8_clr_rg_thr;
	//unsigned char  u8_clr_glb_thr;
	//unsigned char  u8_dynBlkTh_cnt;
	//unsigned char  u8_dynPixTh_cnt;
	//unsigned char  u8_dynBlkStep_cnt;
	//unsigned char  u8_dynPixStep_cnt;
	//unsigned short u16_dynDelogo_mv_th;
	//unsigned char  u8_dynDelogo_rat_th;


	//unsigned char u6_SCalpha_holdtime 		  ;
	//unsigned char u6_SCclrHsty_holdtime 	  ;
	//unsigned char u6_SCclrlogo_holdtime 	  ;
	//unsigned char u8_pixRgClr_thr 			  ;
	//unsigned char u8_pixGlbClr_thr			  ;
	//unsigned char u8_dynY_cnt 				  ;
	//unsigned char u8_delogoRgclr_thr 		  ;
	//unsigned char u8_delogoGlbClr_thr		  ;

	////maybe in output is easy init.

	//unsigned int u32_pxRgYDf_bypass_0_31    ;
	//unsigned char  u4_blklogo_pStep           ;
	//unsigned char  u4_pixlogo_pStep           ;

	//unsigned char  u8_cmo_clear_thr           ;  //(thr)
	//unsigned int u32_cmo_clear_idx          ;  //(idx)
	//unsigned char  u8_gg_mvd12_thr            ;  //good gmv
	//unsigned char  u8_gg_mvx_thr              ;
	//unsigned char  u8_gg_mvy_thr              ;
	//unsigned char  u8_gg_alpha_thr            ;
	//unsigned int u32_gg_sad_thr             ;
	//unsigned char  u8_gg_num_thr              ;
	//unsigned char  u8_reg_mvd                 ;
	//unsigned char  u8_reg_tmv                 ;
	//unsigned char  u1_printf_en               ;
	//unsigned char  u1_blkHsty_upmet           ;
	//


}_PARAM_FRC_LGDet;


/* Output Definition*/
//typedef struct
//{
//	unsigned int u14_pixlogo_rgcnt[RG_32];
//	unsigned int u20_pixlogo_rgaccY[RG_32];
//}_OUTPUT_FRC_LGDet_RB;
//
//typedef struct
//{
//	unsigned short u14_pixlogo_rgcnt[RG_32];
//	unsigned int u20_pixlogo_rgY[RG_32];
//	unsigned int u32_frm_cnt;
//}_OUTPUT_FRC_LGDet_DynSet;
//
//
////total
//typedef struct
//{
//	_OUTPUT_FRC_LGDet_RB       sLogo_rb;
//	_OUTPUT_FRC_LGDet_DynSet   sLogo_dyn_set;
//}_OUTPUT_FRC_LGDet;


typedef struct
{
	_MEMC_LOGO_DEFAULT_VAL DefaultVal;

	signed short  s16_blklogo_rgclr_cnt[LOGO_RG_32];
	signed short  s16_blklogo_glbclr_cnt;
	unsigned int  u32_blklogo_clr_idx;
	unsigned char	u1_sw_clr_en_out;
	unsigned char   u1_blkclr_glbstatus;
	unsigned char   u4_blk_pix_merge_type_out;
	unsigned char   u5_iir_alpha_out;

	unsigned char   u1_rg_dh_clr_rg0;
	unsigned char   u1_rg_dh_clr_rg1;
	unsigned char   u1_rg_dh_clr_rg2;
	unsigned char   u1_rg_dh_clr_rg3;

	unsigned char   u8_rg_dh_clr_lgclr_hold_frm;
	unsigned char   u8_rg_dh_clr_static_hold_frm;

	unsigned int  u32_logo_hsty_clr_idx;
	unsigned int  u32_logo_hsty_clr_lgclr_idx;
	unsigned char   u1_logo_cnt_glb_clr_status;
	unsigned char   u1_logo_sc_glb_clr_status;

	unsigned char  u1_logo_static_status;

	unsigned char  u1_logo_sc_status;
	unsigned int u31_logo_sad_pre;
	unsigned int u20_logo_dtl_pre[32];
	unsigned char  u1_logo_sc_saddif_status;
	unsigned char  u1_logo_sc_Gmv_status;
	unsigned char  u1_logo_sc_dtldif_status;

	unsigned int u8_logo_sc_hold_frm;

	unsigned char  u8_Rg_static_num;
	unsigned int u32_Rg_unstatic_idx;

	unsigned char  u1_logo_netflix_status;
	unsigned char  u8_logo_netflix_hold_frm;

	unsigned char  u1_logo_lg16s_patch_Gapli_status;
	unsigned char  u1_logo_lg16s_patch_Gaplp_status;
	unsigned char  u1_logo_lg16s_patch_Gdtl_status;
	unsigned char  u1_logo_lg16s_patch_Gsad_status;
	unsigned char  u1_logo_lg16s_patch_Gmv_status;
	unsigned char  u1_logo_lg16s_patch_Rdtl_num_status;
	unsigned char  u1_logo_lg16s_patch_status;
	unsigned char  u8_logo_lg16s_patch_hold_frm;

	unsigned char  u8_RgDhadp_Rimunbalance_hold_frm;
	unsigned char  u1_RgDhadp_Rimunbalance_status;

	// UXN Patch
	unsigned char u1_lg_UXN_patch_det;

	unsigned int u32_logo_closeVar_hold_cnt;

	unsigned char u8_logo_sc_FastDet_cntholdfrm;
	unsigned char u1_logo_sc_FastDet_status;

	unsigned char u1_logo_rgn_distribute[LOGO_RGN_MAX];

	unsigned char u3_logo_dhRgnProc_lvl[6];

	//unsigned int u14_pixlogo_rgcnt[RG_32];
	//unsigned int u20_pixlogo_rgaccY[RG_32];

	//
	//signed short  s16_gmv_x_max[RG_32];
	//signed short  s16_gmv_y_max[RG_32];
	//unsigned char   u8_gmv_rat_max[RG_32];

	//signed short  s16_gmv_x_sec[RG_32];
	//signed short  s16_gmv_y_sec[RG_32];
	//unsigned char   u8_gmv_rat_sec[RG_32];

	//unsigned int  u32_region_sad_sum[RG_32];
	//signed short  s16_pixlogo_rgclr_cnt[RG_32];
	//signed short  s16_pixlogo_glbclr_cnt;

	//signed short  s16_SC_clrAlpha_cnt;
	//signed short  s16_SC_clrLogo_cnt;
	//signed short	s16_SC_holdHsty_cnt;

	//unsigned int u32_frm_cnt;
	//unsigned int u32_acc_pixlg_cnt;


	//unsigned char  u1_pixclr_glbstatus;

	//unsigned char  u1_blkclr_glbHsty_status;
	//unsigned char  u1_pixclr_glbHsty_status;

	//unsigned char  u5_alpha_bld;
	//unsigned char  u4_blklogo_pStep_cur;
	//unsigned char  u4_pixlogo_pStep_cur;


	//unsigned int u32_pixlogo_clr_idx;

	unsigned int u8_PxlRgnDfy[32];
	unsigned int u32_PxlRgClrSig;
	unsigned char  u1_logo_BG_still_status;
	unsigned char  u8_logo_BG_still_hold_frm;
	unsigned char  u1_logo_KeyRgn_status;
	unsigned char  u8_logo_KeyRgn_hold_frm;
	unsigned char  u1_logo_PurePanning_status;
	unsigned char  u8_logo_PurePanning_hold_frm;
	unsigned char  u1_logo_NearRim_logo_status;
	unsigned char  u8_logo_NearRim_logo_hold_frm;
	unsigned char u3_logo_dhRgnProc_case[6];
	unsigned char u3_logo_dhRgnProc_H_Moving_case;
}_OUTPUT_FRC_LGDet;

/* MOdule UI Definition*/
DEBUGMODULE_BEGIN(_PARAM_FRC_LGDet,_OUTPUT_FRC_LGDet)
	//param old
	ADD_PARAM(unsigned char,    u1_logo_pql_en				    ,	  0     )
	ADD_PARAM(unsigned char,    u1_logo_sw_clr_en	    	    ,	  1     )
	ADD_PARAM(unsigned char,    u1_logo_rg_clr_en				,	  0    	)
	ADD_PARAM(unsigned char,    u1_logo_glb_clr_en			    ,	  1    	)
	ADD_PARAM(unsigned char,    u1_logo_left_half_clr_en	    ,	  1    	)
	ADD_PARAM(unsigned char,    u1_logo_right_half_clr_en	    ,	  1    	)
	ADD_PARAM(unsigned char,    u1_logo_top_half_clr_en		    ,	  1    	)
	ADD_PARAM(unsigned char,    u1_logo_bot_half_clr_en		    ,	  1    	)
	ADD_PARAM(unsigned char,    u1_logo_sc_alpha_ctrl_en        ,	  1    	)
	ADD_PARAM(unsigned char,    u1_logo_sc_logo_clr_en          ,	  1    	)

	ADD_DUMMY_PARAM()
	ADD_PARAM(unsigned char,    u8_clr_rg_thr		            ,     215   )
	ADD_PARAM(unsigned char,    u8_clr_glb_thr		            ,     80    )
	ADD_PARAM(unsigned char, 	u8_clr_half_thr     	        ,     70	)
	ADD_PARAM(unsigned char,    u6_rgClr_holdtime		        ,     5     )
	ADD_PARAM(unsigned char,    u6_glbClr_holdtime		        ,     5     )
	ADD_DUMMY_PARAM()
	ADD_PARAM(unsigned char,    u4_blk_pix_merge_type		    ,     0     )
	ADD_PARAM(unsigned char,    u5_iir_alpha                 	,     31    )


	//ADD_PARAM(unsigned char,  u1_logo_dynY_en				,	  0    	)
	//ADD_PARAM(unsigned char,  u1_logo_dynBlkTh_en			,	  1    	)
	//ADD_PARAM(unsigned char,  u1_logo_dynPixTh_en			,	  1    	)
	//ADD_PARAM(unsigned char,  u1_logo_dynBlkStep_en			,	  1    	)
	//ADD_PARAM(unsigned char,  u1_logo_dynPixStep_en			,	  1    	)
	//ADD_PARAM(unsigned char,  u1_logo_dynAlpha_en			,	  1    	)
	//ADD_PARAM(unsigned char,  u1_logo_dynDelogo_en			,	  0    	)
	//ADD_PARAM(unsigned char,  u1_logo_pixRgClr_en 			,     0		)
	//ADD_PARAM(unsigned char,  u1_logo_pixGlbClr_en			,     0		)
	//ADD_PARAM(unsigned char,  u1_logo_blkGlbClrHsty_en		,	  0		)
	//ADD_PARAM(unsigned char,  u1_logo_pixGlbClrHsty_en		,     0		)
	//ADD_PARAM(unsigned char,  u1_logo_blkRgClrHsty_en 		,     0		)
	//ADD_PARAM(unsigned char,  u1_logo_pixRgClrHsty_en 		,     0		)
	//ADD_PARAM(unsigned char,  u1_logo_delogoGlbClr_en 		,     0		)
	//ADD_PARAM(unsigned char,  u1_logo_delogoRgClr_en  		,     0		)
	//ADD_PARAM(unsigned char,  u1_logo_delogoGlbClrHsty_en	,	  0		)
	//ADD_PARAM(unsigned char,  u1_logo_delogoRgClrHsty_en 	,	  0		)
	//ADD_PARAM(unsigned char,  u1_logo_SCalphaClr_en			,	  1		)
	//ADD_PARAM(unsigned char,  u1_logo_SCHstyClr_en 			,	  1		)
	//ADD_PARAM(unsigned char,  u1_logo_SCLogoClr_en 			,	  1		)


	//ADD_PARAM(unsigned char,    u8_clr_rg_thr		        ,     215   	)
	//ADD_PARAM(unsigned char,    u8_clr_glb_thr		        ,     60    	)
	//ADD_PARAM(unsigned char,    u8_dynBlkTh_cnt 		    ,     10    	)
	//ADD_PARAM(unsigned char,    u8_dynPixTh_cnt 		    ,     10    	)
	//ADD_PARAM(unsigned char,    u8_dynBlkStep_cnt			,     10    	)
	//ADD_PARAM(unsigned char,    u8_dynPixStep_cnt			,     10    	)
	//ADD_PARAM(unsigned short,   u16_dynDelogo_mv_th 		,     15  		)
	//ADD_PARAM(unsigned char,    u8_dynDelogo_rat_th	    	,     180		)
	//ADD_PARAM(unsigned char,    u6_rgClr_holdtime 		    ,     5			)
	//ADD_PARAM(unsigned char,    u6_glbClr_holdtime		    ,     5			)
	//ADD_PARAM(unsigned char,    u6_SCalpha_holdtime 	    ,     1			)
	//ADD_PARAM(unsigned char,    u6_SCclrHsty_holdtime 	    ,     1			)
	//ADD_PARAM(unsigned char,    u6_SCclrlogo_holdtime 	    ,     1			)
	//ADD_PARAM(unsigned char,    u8_pixRgClr_thr 		    ,     215		)
	//ADD_PARAM(unsigned char,    u8_pixGlbClr_thr		    ,     67		)
	//ADD_PARAM(unsigned char,    u8_dynY_cnt 			    ,     10		)
	//ADD_PARAM(unsigned char,    u8_delogoRgclr_thr 	        ,     215		)
	//ADD_PARAM(unsigned char,    u8_delogoGlbClr_thr	        ,	  67		)

	//ADD_PARAM(unsigned int,   u32_pxRgYDf_bypass_0_31     ,      0		)
	//ADD_PARAM(unsigned char,    u4_blklogo_pStep            ,      1		)
	//ADD_PARAM(unsigned char,    u4_pixlogo_pStep            ,      1		)

	//ADD_PARAM(unsigned char, 	u8_cmo_clear_thr     		,	  55		)
	//ADD_PARAM(unsigned int, 	u32_cmo_clear_idx    		,	  240		)
	//ADD_PARAM(unsigned char, 	u8_gg_mvd12_thr      		,	  20		)
	//ADD_PARAM(unsigned char, 	u8_gg_mvx_thr        		,	  10		)
	//ADD_PARAM(unsigned char, 	u8_gg_mvy_thr        		,	  10		)
	//ADD_PARAM(unsigned char, 	u8_gg_alpha_thr      		,	  100		)
	//ADD_PARAM(unsigned int, 	u32_gg_sad_thr       		,	  32000		)
	//ADD_PARAM(unsigned char, 	u8_gg_num_thr        		,	  28		)
	//ADD_PARAM(unsigned char, 	u8_reg_mvd           		,	  0			)
	//ADD_PARAM(unsigned char, 	u8_reg_tmv           		,	  0			)
	//ADD_PARAM(unsigned char, 	u1_printf_en         		,	  0			)
	//ADD_PARAM(unsigned char, 	u1_blkHsty_upmet     	    ,      1		)




	//////////////////////////////////////////////////////////////////////////
	//output

	 ADD_OUTPUT(unsigned char,  u1_blkclr_glbstatus)
	 ADD_OUTPUT(unsigned int, u32_blklogo_clr_idx)
	 ADD_OUTPUT(unsigned char,  u5_iir_alpha_out)


	//ADD_OUTPUT_ARRAY(unsigned int, u14_pixlogo_rgcnt,     RG_32)
	//ADD_OUTPUT_ARRAY(unsigned int, u20_pixlogo_rgaccY,    RG_32)


	//ADD_OUTPUT_ARRAY(signed short, s16_gmv_x_max,     RG_32)
	//ADD_OUTPUT_ARRAY(signed short, s16_gmv_y_max,     RG_32)
	//ADD_OUTPUT_ARRAY(unsigned char,  u8_gmv_rat_max,    RG_32)

	//ADD_OUTPUT_ARRAY(signed short,  s16_gmv_x_sec,     RG_32)
	//ADD_OUTPUT_ARRAY(signed short,  s16_gmv_y_sec,     RG_32)
	//ADD_OUTPUT_ARRAY(unsigned char,   u8_gmv_rat_sec,    RG_32)

	//ADD_OUTPUT_ARRAY(unsigned int,   u32_region_sad_sum,    RG_32)

	//ADD_OUTPUT_ARRAY(signed short,  s16_blklogo_rgclr_cnt,     RG_32)
	//ADD_OUTPUT(signed short,  s16_blklogo_glbclr_cnt)

	//ADD_OUTPUT_ARRAY(signed short,  s16_pixlogo_rgclr_cnt,     RG_32)
	//ADD_OUTPUT(signed short,  s16_pixlogo_glbclr_cnt)

	//ADD_OUTPUT(signed short,  s16_SC_clrAlpha_cnt)
	//ADD_OUTPUT(signed short,  s16_SC_clrLogo_cnt)
	//ADD_OUTPUT(signed short,  s16_SC_holdHsty_cnt)

	//ADD_OUTPUT(unsigned int,u32_frm_cnt)
 //	ADD_OUTPUT(unsigned int,u32_acc_pixlg_cnt)


	//ADD_OUTPUT(unsigned char, u1_pixclr_glbstatus)

	//ADD_OUTPUT(unsigned char, u1_blkclr_glbHsty_status)
	//ADD_OUTPUT(unsigned char, u1_pixclr_glbHsty_status)

	//ADD_OUTPUT(unsigned char, u5_alpha_bld)
	//ADD_OUTPUT(unsigned char, u4_blklogo_pStep_cur)
	//ADD_OUTPUT(unsigned char, u4_pixlogo_pStep_cur)

	//ADD_OUTPUT(unsigned int, u32_blklogo_clr_idx)
	//ADD_OUTPUT(unsigned int, u32_pixlogo_clr_idx)

	//ADD_OUTPUT_ARRAY(unsigned int, u14_pixlogo_rgcnt, RG_32)
DEBUGMODULE_END


	/* Function Declaration*/
	VOID FRC_LogoDet_Init(_OUTPUT_FRC_LGDet *pOutput);
    VOID FRC_LogoDet_Proc(const _PARAM_FRC_LGDet *pParam, _OUTPUT_FRC_LGDet *pOutput);

#endif
