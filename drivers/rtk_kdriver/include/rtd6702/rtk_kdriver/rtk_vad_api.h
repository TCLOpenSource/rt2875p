#ifndef _RTK_VAD_H
#define _RTK_VAD_H

#include <linux/module.h>
#include <linux/kernel.h>

#include "rtk_vad_def.h"


extern void rtk_vad_pitch_det_init(void);
extern void rtk_vad_set_pitch_det_mode(RTK_VAD_PITCH_MODE vad_pitch_mode);
extern void rtk_vad_pitch_det_start(void);
extern void rtk_vad_pitch_det_stop(void);
extern void rtk_vad_get_pitch_det_result(char *pitch_det);
extern void rtk_vad_reset_pitch_det_result(void);


#endif //_RTK_VAD_H

