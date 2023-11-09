
#include <tvscalercontrol/vip/edgesmooth.h>

#ifndef __VIP_NR_H__
#define __VIP_NR_H__

#include <scaler/vipCommon.h>	/* in common*/
#include <scaler/vipRPCCommon.h>	/* in common*/

#define NM_LEVEL 6

typedef struct {
	unsigned short vardiff_thd;
	unsigned short varhigh_thd;
	unsigned short varlow_thd;
	unsigned char meandiff_thd;
	unsigned char noiselevel_thd0;
	unsigned char noiselevel_thd1;
	unsigned char noiselevel_thd2;
	unsigned char noiselevel_thd3;
	unsigned char noiselevel_thd4;
	unsigned char noiselevel_thd5;
	unsigned char noiselevel_thd6;
	unsigned char noiselevel_thd7;
	unsigned char noiselevel_thd8;
	unsigned char noiselevel_thd9;
	unsigned char noiselevel_thd10;
	unsigned char noiselevel_thd11;
	unsigned char noiselevel_thd12;
	unsigned char noiselevel_thd13;
	unsigned char noiselevel_thd14;
} DRV_rtnr_nm_thd_t;

typedef struct {
	unsigned char main_enable;
	unsigned char start;
	unsigned char ref_y_en;
	unsigned char fw_yref;
	unsigned char fw_ydiff;
	unsigned char pixmove_en;
	unsigned char varsel;
	unsigned char debug_conti;
	unsigned char samplingbit;
	unsigned char topmargin;
	unsigned char bottommargin;
	unsigned char leftmargin;
	unsigned char rightmargin;
} DRV_rtnr_nm_ctrl_t;



/*move to vipCommon.h for videofw, elieli*/
#if 0
/* chen 0716*/
typedef struct {
	unsigned char rtnr_ma_snr_en;
	unsigned char rtnr_ma_snr_mask;
	unsigned char rtnr_ma_snr_blending_factor1;
	unsigned char rtnr_ma_snr_blending_factor2;
	unsigned char rtnr_ma_snr_blending_factor3;
	unsigned char rtnr_ma_snr_blending_factor4;
	unsigned char rtnr_ma_snr_blending_factor5;
	unsigned char rtnr_ma_snr_motion_th1_low;
	unsigned char rtnr_ma_snr_motion_th2_low;
	unsigned char rtnr_ma_snr_motion_th3_low;
	unsigned char rtnr_ma_snr_motion_th4_low;
	unsigned char rtnr_ma_snr_motion_th1_high;
	unsigned char rtnr_ma_snr_motion_th2_high;
	unsigned char rtnr_ma_snr_motion_th3_high;
	unsigned char rtnr_ma_snr_motion_th4_high;
	unsigned char rtnr_ma_snr_edge_th_low;
	unsigned char rtnr_ma_snr_edge_th_high;
	unsigned char rtnr_ma_snr_motion_slope1;
	unsigned char rtnr_ma_snr_motion_slope2;
	unsigned char rtnr_ma_snr_motion_slope3;
	unsigned char rtnr_ma_snr_motion_slope4;
	unsigned char rtnr_ma_snr_edge_curve_lo_th;
	unsigned char rtnr_ma_snr_edge_curve_slope;
	unsigned char rtnr_ma_snr_output_clamp;
	unsigned char rtnr_ma_snr_strength;
} DRV_Rtnr_Masnr;
#endif

/* end chen 0716*/

typedef struct _VIP_I_De_Contour_Statistic {
	unsigned int decont_contour_cnt;
	unsigned int decont_avfilter_cnt;
	unsigned int decont_range_hist[16];
	unsigned int decont_ydiff_hist[8];
	unsigned int decont_cdiff_hist[8];
} VIP_I_De_Contour_Statistic;

/*#include "rtd_types.h"*/
/*rock_rau	20130227 mark not use function*/ /**/
void drvif_color_noisereduction_spatialy(unsigned char channel, unsigned char level, unsigned char bNoiseReductionMode);/*level:0-255*/
void drvif_color_noisereduction_spatialc(unsigned char channel, unsigned char level, unsigned char bNoiseReductionMode);/*level:0-255*/
void drvif_color_noisereduction_impulse(unsigned char channel , unsigned char level, unsigned char bNoiseReductionMode);/*level:0-255*/
void drvif_color_noisereduction_temporal(unsigned char channel, unsigned char level);/*level:0-255*/
void drvif_color_noise_estimation_init(unsigned char channel, unsigned char enable);
void drvif_color_noise_estimation(void);
unsigned char fwif_color_get_dnr_open(void);

void drvif_color_MpegNR_H(unsigned char channel, unsigned char level);
void drvif_color_MpegNR_V(unsigned char channel, unsigned char level);
void drvif_color_DARK_K(unsigned char channel, unsigned char level);
void drvif_color_MCNR(unsigned char channel, unsigned char level);

/* chen 0716*/
void drvif_color_DRV_RTNR_MASNR(DRV_RTNR_MASNR *ptr);
/* end chen 0716*/



#if 0 /* hsliao 20081231, move to linux driver*/
void drvif_color_histogram_isr(void);
void drvif_color_histogram_irqinit(void);
#endif
void drvif_color_rtnr_grid_remover_mask(unsigned char channel, unsigned char r_c[4], unsigned char l_c[4], unsigned char v_c[4], unsigned char h_c[4]);
void drvif_color_rtnr_grid_remover_thl(unsigned char channel, unsigned char osd_level);
void drvif_color_rtnr_grid_remover_type(unsigned char channel, unsigned short type);
unsigned char drvif_color_nrrestrict(void);
void drvif_color_rtnr_init(unsigned char channel);
void drvif_color_rtnr_c_th(unsigned char channel, unsigned char *Th_C);
void drvif_color_rtnr_y_th(unsigned char channel, unsigned char *Th_Y);
void drvif_color_rtnr_y_k(unsigned char channel, unsigned int Y_K0_K7, unsigned int Y_K8_K15);
void drvif_color_rtnr_c_k(unsigned char channel, unsigned int K_C);
void drivf_color_rtnr_correction_set(unsigned char level);
/*void drvif_color_rtnr_k(unsigned char channel, unsigned short K_Y, unsigned int K_C);*/
void drvif_color_rtnr_off(unsigned char channel);
void drvif_color_rtnr_on(unsigned char channel);

void drvif_color_rtnr_progressive_check(unsigned char interlace_flag, unsigned char channel, unsigned char db_apply);
/*unsigned char drvif_color_rtnr_3A_check(void);*/
/*void drvif_color_rtnr_disable_tnr(void);*/
unsigned char drvif_color_getHistCnt(unsigned int *dst);
/* */ /* added by Jerry Wu 20090326*/
void drvif_color_mpegnoisereduction_off(unsigned char channel);
void driver_color_dcr_old(unsigned int *histogram, unsigned int *level);
/*void driver_color_dcr(unsigned int *histogram, unsigned int *level);*/ /*adjust by jason9.ccc, dynamic control backlight*/

/* New RTNR*/

void drvif_color_newrtnr_init(void);
void drvif_color_newrtnr(unsigned char enable);
void drvif_color_newrtnr_sad_method(unsigned char value);
void drvif_color_newrtnr_mixcal_weight(unsigned char mixWeight);
void drvif_color_newrtnr_sta_motion_sad_th(unsigned char th, unsigned char th2, unsigned char th3);
void drvif_color_newrtnr_K_Temporal_Compare(unsigned char enable, unsigned char snrEn);
void drvif_color_newrtnr_moredetail_continue(unsigned char enable);
void drvif_color_newrtnr_moredetail_comtinue(unsigned char enable);
void drvif_color_newrtnr_sad_th(unsigned char edge, unsigned char *th);
void drvif_color_newrtnr_sta_th(unsigned char *th);
void drvif_color_newrtnr_edgetypediff_th(unsigned char *th);
void drvif_color_newrtnr_snr_edge_th(unsigned char *th);
void drvif_color_newrtnr_snr(void);
void drvif_color_newrtnr_rtnr_k(unsigned char row_select, unsigned char *th);
void drvif_color_newrtnr_mask(unsigned char mask_select);


unsigned char drvif_color_rtnr_k_force(unsigned char level, unsigned char offset);


/* I Edge smooth*/
void drvif_color_Mixer_ctrl_Vfilter(unsigned char channel, int vfilter_weight);
void drvif_color_iEdgeSmooth_line_mode_set(unsigned char select);
void drvif_color_iEdgeSmooth_prelpf_for_detection_en(unsigned char enable);
void drvif_color_iEdgeSmooth_prelpf_for_blending_en(unsigned char weight);
void drvif_color_iEdgeSmooth_Addr_Boundary_ini(unsigned int addr_sta, unsigned int addr_end);

void drvif_color_nr_HMCNR(unsigned char level);
void drvif_color_nr_HMCNR_blending(unsigned char blending_ratio);
void drvif_color_nr_HMCNR_UV(unsigned char enable);
//void drvif_color_nr_HMCNR_Top_Line(unsigned char level);

/*SNR*/
void drvif_color_snr_mode_select(unsigned char mode);
void drvif_color_nr_Curvemapping_En(unsigned char enable, int select);
void drvif_color_nr_Curvemapping_step_set(int select);
void drvif_color_nr_Curvemapping_weight_set(int w1_select, int w2_select);
void drvif_color_nr_modified_lpf_En(unsigned char enable);
void drvif_color_nr_modified_thd_set(int thd);
void drvif_color_nr_Zoran_filter_set_Y(int select);
void drvif_color_nr_Zoran_filter_set_C(int select);
void drvif_color_nr_mosquitoNR_set(unsigned char src_idx, unsigned char value);

void drvif_color_DRV_RTNR_General_ctrl(DRV_RTNR_General_ctrl *ptr);
void drvif_color_DRV_RTNR_Old_Y(DRV_RTNR_Old_Y *ptr);
void drvif_color_DRV_RTNR_Dark_K(DRV_RTNR_Dark_K *ptr);

// chen 170522
//void drvif_color_DRV_RTNR_New_Method(DRV_RTNR_New_Method *ptr);
//void drvif_color_DRV_RTNR_Mark(DRV_RTNR_Mask *ptr);
//void drvif_color_DRV_RTNR_By_Y(DRV_RTNR_By_Y *ptr);
// end chen 170522

void drvif_color_DRV_RTNR_LSB(DRV_RTNR_LSB *ptr);
// chen 170809
#if 0
void drvif_color_DRV_MCNR_TMV(unsigned char block_size);
#endif
// chen 170809
void drvif_color_DRV_MCNR_table(DRV_MCNR_Table *ptr);
/*void drvif_color_DRV_RTNR_SNR(DRV_RTNR_SNR *ptr);		*/ /*using MASNR instead, elieli*/
void drvif_color_DRV_RTNR_C(DRV_RTNR_C *ptr);


/*void drvif_color_RTNR_flow(DRV_NR_Item *ptr);*/
void drvif_color_DRV_SNR_General_ctrl(DRV_SNR_General_ctrl *ptr, unsigned char bIsVDSrc);
void drvif_color_DRV_SNR_Spatial_NR_Y(DRV_SNR_Spatial_NR_Y *ptr);
void drvif_color_DRV_SNR_Spatial_NR_C(DRV_SNR_Spatial_NR_C *ptr);
void drvif_color_DRV_SNR_Modified_LPF(DRV_SNR_Modified_LPF *ptr);
void drvif_color_DRV_SNR_Impulse_NR(DRV_SNR_Impulse_NR *ptr);
void drvif_color_DRV_SNR_Curve_Mapping_Mode(DRV_SNR_Curve_Mapping_Mode *ptr);
void drvif_color_DRV_SNR_NR_Mask(DRV_SNR_NR_Mask *ptr);
void drvif_color_DRV_SNR_NR_Mask_W1W2(DRV_SNR_NR_Mask_W1W2 *ptr); /*Mac3, leonard_wu@20140722*/
void drvif_color_DRV_SNR_Veritcal_NR(DRV_SNR_Veritcal_NR *ptr);
void drvif_color_DRV_SNR_Mosquito_NR(DRV_SNR_Mosquito_NR *ptr);
void drvif_color_DRV_SNR_Mosquito_NR_En(unsigned char En_flag);	/* Mosquito NR control is in general control*/
void drvif_color_DRV_SNR_Weight_Adjust(DRV_SNR_WeightAdjust *ptr) ; /*mag2*/
void drvif_color_DRV_SNR_UVEdge(DRV_SNR_UVEdge *ptr);/*Merlin*/
void drvif_color_DRV_SNR_VD_Spatial_NR_Y(DRV_SNR_Spatial_NR_Y *ptr);
/*void drvif_color_SNR_flow(DRV_NR_Item *ptr);*/
void drvif_color_DRV_MPG_General_ctrl(DRV_MPG_General_ctrl *ptr);
void drvif_color_DRV_MPG_H(DRV_MPEGNR_16_Table *ptr);
void drvif_color_DRV_MPG_H_tv006(DRV_MPEGNR_16_Table *ptr, unsigned char skipFWreg);
void drvif_color_DRV_MPG_V(DRV_MPEGNR_16_Table *ptr);
/*void drvif_color_MPEGNR_flow(DRV_NR_Item *ptr);*/
void drvif_color_set_DRV_MPG_H_SW_Ctrl(DRV_MPEGNR_16_SW_CTRL *ptr);
void drvif_color_get_DRV_MPG_H_SW_Ctrl(DRV_MPEGNR_16_SW_CTRL *ptr);
void drvif_color_set_DRV_MPG_V_SW_Ctrl(DRV_MPEGNR_16_SW_CTRL *ptr);
void drvif_color_get_DRV_MPG_V_SW_Ctrl(DRV_MPEGNR_16_SW_CTRL *ptr);


void drvif_IESM_format_convert(void);// Merlin4 add
//void drvif_color_DRV_DLTI_2D(DRV_DLTI_2D *ptr); //mac6p removed IESM DLTI2D
/*void drvif_color_IEDGESMOOTH_flow(DRV_IEdgeSmooth_Coef *ptr);*/
void drvif_color_DRV_MA_ISNR_GetMotion(DRV_MA_ISNR_GetMotion *ptr);
void drvif_color_DRV_MA_SNR_Motion_Weight1(DRV_MA_SNR_Motion_Weight1 *ptr);
void drvif_color_DRV_MA_SNR_Motion_Weight2(DRV_MA_SNR_Motion_Weight2 *ptr);
void drvif_color_DRV_MA_IESM_Motion_Ctrl(DRV_MA_IESM_Motion_Ctrl *ptr);
void drvif_color_DRV_MA_IESM_Motion_Weight(DRV_MA_IESM_Motion_Weight *ptr);
void drvif_color_DRV_MA_VLPF_Motion_Weight(DRV_MA_VLPF_Motion_Weight *ptr);
void drvif_color_MASNR_DB_Enable(void);
void drvif_color_MASNR_DB_Apply(void);
/* RTNR Noise Measure*/
void drvif_color_DRV_NM_Control(DRV_rtnr_nm_ctrl_t *ptr);
void drvif_color_DRV_NM_Thd(DRV_rtnr_nm_thd_t *ptr);
//void drvif_color_DRV_NM_Init(DRV_rtnr_nm_ctrl_t *nm_ctrl, DRV_rtnr_nm_thd_t *nm_thd);


void drvif_color_Get_MAD_Value(DRV_MAD_counter *ptr);
#ifdef CONFIG_HW_SUPPORT_I_DE_XC
unsigned char  drvif_color_Set_I_De_XC(VIP_I_De_XC_TBL *ptr);
#endif //CONFIG_HW_SUPPORT_I_DE_XC
unsigned char  drvif_color_Set_I_De_Contour(VIP_I_De_Contour_TBL *ptr);
unsigned char  drvif_color_Get_I_De_Contour(VIP_I_De_Contour_TBL *ptr);
unsigned char drvif_color_Get_I_De_Contour_Stats(VIP_I_De_Contour_Statistic* ptr);
void drvif_color_I_De_Contour_line_buffer_check(void);
void drvif_color_Set_DRV_SNR_Clock(SCALER_DISP_CHANNEL display, unsigned char En_flag);
unsigned char drvif_color_Get_DRV_SNR_Clock(SCALER_DISP_CHANNEL display, unsigned char En_flag);

void drvif_IESM_format_convert(void);
void drvif_IESM_frontporch_set(unsigned char game_mode_check_flag);
void drvif_color_Set_I_De_Contour_DB_Apply(void);
void drvif_color_set_DI_RTNR_Timing_Measurement(void);
char drvif_color_Get_DI_RTNR_Timing_Measurement(unsigned int *pRTNR, unsigned int *pDI);

void drvif_color_DRV_SNR_Mosquito_NR_DEMO(void);
void drvif_color_DRV_SNR_Modified_LPF_DEMO(void);
void drvif_color_DRV_SNR_Mosquito_NR_reset_DEMO(void);
#endif
