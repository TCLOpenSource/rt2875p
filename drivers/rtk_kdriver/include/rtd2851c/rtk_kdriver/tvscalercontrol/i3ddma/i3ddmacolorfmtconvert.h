#ifndef I3DDMA_COLOR_FMT_CONVERT_H_
#define I3DDMA_COLOR_FMT_CONVERT_H_

#include <tvscalercontrol/i3ddma/i3ddma.h>


char I3DDMA_4xxto4xx_Downsample(I3DDMA_COLOR_SPACE_T source, I3DDMA_COLOR_SPACE_T target);
void I3DDMA_ColorSpacergb2yuvtransfer(unsigned char nSrcType, unsigned char nSD_HD, unsigned char VO_Jpeg_Back_YUV);


#endif

