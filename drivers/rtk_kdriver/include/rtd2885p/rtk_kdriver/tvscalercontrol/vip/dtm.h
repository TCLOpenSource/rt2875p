#ifndef _DTM_H_
#define _DTM_H_

#ifdef __cplusplus
extern "C" {
#endif

#define CHMAX 2
#define TBMAX 3
#define RGBSEGMAX 256

unsigned char drvif_color_dtm_write_RGB_Table(unsigned short dtm_rgb_table[CHMAX][TBMAX][RGBSEGMAX]);
unsigned char drvif_color_dtm_get_RGB_Table(unsigned short dtm_rgb_table[CHMAX][TBMAX][RGBSEGMAX]);
unsigned char drvif_color_dtm_write_Table(int  dtm_table[TBMAX][RGBSEGMAX]);unsigned char drvif_color_dtm_get_current_Table(int  dtm_table[TBMAX][RGBSEGMAX]);
unsigned char drvif_color_dtm_get_sub_One_Table(unsigned short *dtm_one_table);

#ifdef __cplusplus
}
#endif
#endif

