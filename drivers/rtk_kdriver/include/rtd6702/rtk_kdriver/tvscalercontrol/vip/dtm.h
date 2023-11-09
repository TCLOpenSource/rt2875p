#ifndef _DTM_H_
#define _DTM_H_

#ifdef __cplusplus
extern "C" {
#endif

#define TBMAX 3
#define RGBSEGMAX 256
#define HistMax 128

typedef struct  {
	unsigned int dtm_main_en;
	unsigned int dtm_sub_en;
	unsigned int dtm_overlay_en;
	unsigned int bypass_yuv2rgb;
	unsigned int bypass_rgb2hsi;
	unsigned int bypass_dtm_lut;
	unsigned int bypass_h_compensation;
	unsigned int bypass_s_compensation;
	unsigned int dtm_hsi_mode_sel;
	unsigned int dtm_rgb_mode_sel;
       unsigned int dtm_hist_en;
       unsigned int dtm_hist_mode_sel;
} DTM_CTRL_INFO;
unsigned char drvif_color_dtm_write_Table(int  dtm_table[TBMAX][RGBSEGMAX]);
unsigned char drvif_color_dtm_get_current_Table(int  dtm_table[TBMAX][RGBSEGMAX]);
unsigned char drvif_color_dtm_get_curret_Hist(int  dtm_hist[HistMax]);
void drvif_color_dtm_control(DTM_CTRL_INFO dtm_para);

#ifdef __cplusplus
}
#endif
#endif

