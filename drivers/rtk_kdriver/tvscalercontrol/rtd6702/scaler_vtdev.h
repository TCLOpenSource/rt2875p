#ifndef _HAL_VT_H_
#define _HAL_VT_H_

#include <ioctrl/scaler/vt_cmd_id.h>

unsigned char get_vt_EnableFRCMode(void);
unsigned char get_vdec_securestatus(void);
void set_vdec_securestatus(unsigned char value);
void set_dtv_securestatus(unsigned char status);
void set_vt_StreamOn_flag(unsigned char flg);
extern unsigned char get_svp_protect_status(void);

#endif

