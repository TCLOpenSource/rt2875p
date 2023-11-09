#if (defined TV003_ADTV)
#include <Project/Custom/SetupMenu/setupdef_picmode.h>

#else

#ifndef __SETUPDEF_PICMODE_H__
#define __SETUPDEF_PICMODE_H__

#if (defined ENABLE_NEW_DVB_2 || defined BUILD_TV005_1_ATV) || (defined BUILD_TV005_1_ATSC)
typedef enum _PICTURE_MODE {
#if 0 /*FixedME clamp Mac2*/
	COLORSTD_DEFAULT = 0,
	PICTURE_MODE_AUTO_VIEW,
	PICTURE_MODE_DYNAMIC,
	PICTURE_MODE_STANDARD,
	PICTURE_MODE_MILD,
	PICTURE_MODE_MOVIE,
	PICTURE_MODE_ECO,
	PICTURE_MODE_GAME,
	PICTURE_MODE_PC,
	PICTURE_MODE_CRICKET,/*yuan::20130624*/
	PICTURE_MODE_MAX,
#else
	PICTURE_MODE_USER = 0,
	COLORSTD_DEFAULT = PICTURE_MODE_USER,
	PICTURE_MODE_VIVID,
	PICTURE_MODE_STD,
	PICTURE_MODE_GENTLE,
	PICTURE_MODE_MOVIE,
	PICTURE_MODE_SPORT,
	PICTURE_MODE_GAME,
	PICTURE_MODE_AUTO_VIEW,
	PICTURE_MODE_DYNAMIC,
	PICTURE_MODE_STANDARD,
	PICTURE_MODE_MILD,
	PICTURE_MODE_ECO,
	PICTURE_MODE_PC,
	PICTURE_MODE_CRICKET,/*yuan::20130624*/
	PICTURE_MODE_MAX,
#endif
} PICTURE_MODE;

#elif (defined IS_TV003_STYLE_PICTUREMODE)
typedef enum _PICTURE_MODE {
	PICTURE_MODE_STANDARD = 0,
	PICTURE_MODE_STANDARD_ENERGY_STAR,
	PICTURE_MODE_MOVIE,
	PICTURE_MODE_PC,
	PICTURE_MODE_USER,
	PICTURE_MODE_DYNAMIC,
	PICTURE_MODE_DYNAMIC_FIXED,
	PICTURE_MODE_MAX,
} PICTURE_MODE;

#else
typedef enum _PICTURE_MODE {

	PICTURE_MODE_USER = 0,
	PICTURE_MODE_VIVID,
	PICTURE_MODE_STD,
	PICTURE_MODE_GENTLE,
	PICTURE_MODE_MOVIE,
	PICTURE_MODE_SPORT,
	PICTURE_MODE_GAME,
	PICTURE_MODE_AUTO_VIEW,
	PICTURE_MODE_DYNAMIC,
	PICTURE_MODE_STANDARD,
	PICTURE_MODE_MILD,
	PICTURE_MODE_ECO,
	PICTURE_MODE_PC,
	PICTURE_MODE_CRICKET,/*yuan::20130624*/
	PICTURE_MODE_MAX,

} PICTURE_MODE;
#endif

#endif
#endif