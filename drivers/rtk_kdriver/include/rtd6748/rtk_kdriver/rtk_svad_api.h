#ifndef _RTK_SVAD_API_H
#define _RTK_SVAD_API_H

#include <linux/module.h>
#include <linux/kernel.h>

#include "rtk_svad_def.h"


extern void rtk_svad_model_init(void);
extern void rtk_svad_det_start(void);
extern void rtk_svad_det_stop(void);
extern void rtk_svad_get_det_result(unsigned int *svad_det_result);
extern void rtk_svad_reset_det_result(void);


#endif //_RTK_SVAD_API_H
