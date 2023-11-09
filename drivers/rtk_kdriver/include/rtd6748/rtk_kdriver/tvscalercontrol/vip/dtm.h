#ifndef _DTM_H_
#define _DTM_H_

#ifdef __cplusplus
extern "C" {
#endif

#define TBMAX 3
#define RGBSEGMAX 256
unsigned char drvif_color_dtm_write_Table(int  dtm_table[TBMAX][RGBSEGMAX]);
unsigned char drvif_color_dtm_get_current_Table(int  dtm_table[TBMAX][RGBSEGMAX]);

#ifdef __cplusplus
}
#endif
#endif

