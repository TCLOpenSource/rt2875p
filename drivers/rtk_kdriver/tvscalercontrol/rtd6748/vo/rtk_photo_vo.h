#include "rtk_vo.h"

HAL_VO_STATE_T PHOTO_VO_Open(VO_OPEN_CMD_T *);
//HAL_PHOTO_VO_STATE_T VO_DisplayPicture(unsigned int, HAL_VO_IMAGE_T, bool);
HAL_VO_STATE_T PHOTO_VO_Close(unsigned int);
HAL_VO_STATE_T PHOTO_VO_DisplayPicture_Done(void);

#define PHOTOVO_IOC_MAGIC            'v'
#define PHOTOVO_IOC_OPEN             _IOW(PHOTOVO_IOC_MAGIC, 0, int)
#define PHOTOVO_IOC_CLOSE            _IOW(PHOTOVO_IOC_MAGIC, 1, int)
#define PHOTOVO_IOC_DISPLAYPIC_DONE  _IOW(PHOTOVO_IOC_MAGIC, 2, int)
#define PHOTOVO_IOC_MAXNR            2