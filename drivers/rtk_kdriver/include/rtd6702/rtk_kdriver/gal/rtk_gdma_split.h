#ifndef _GDMA_SPLIT_H_
#define _GDMA_SPLIT_H_
#include <gal/rtk_gdma_export_user.h>
#include <linux/types.h>

#define GDMA_SPLIT_MAX_X  1920
#define GDMA_SPLIT_MAX_Y  1080
#define GDMA_SPLIT_EXT_Y  0//280

typedef enum
{
	GDMA_SPLIT_MODE_0  =	0,//original
	GDMA_SPLIT_MODE_1  =	1,//mixer
	GDMA_SPLIT_MODE_2  =	2,//2k1k split
	GDMA_SPLIT_MODE_3  =	3,//1k2k split
} _GDMA_SPLIT_T;

int GDMA_split_mode(int mode);

#endif
