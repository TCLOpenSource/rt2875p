#ifndef _HAL_VT_H_
#define _HAL_VT_H_

#include <ioctrl/scaler/vt_cmd_id.h>

unsigned char get_vt_EnableFRCMode(void);
unsigned char get_vdec_securestatus(void);
void set_vdec_securestatus(unsigned char value);
void set_dtv_securestatus(unsigned char status);
extern unsigned char get_svp_protect_status(void);
void set_VT_Pixel_Format(VT_CAP_FMT value);
VT_CAP_FMT get_VT_Pixel_Format(void);
unsigned int get_vt_VtBufferNum(void);
void set_vt_VtBufferNum(unsigned int value);
unsigned char Capture_BufferMemInit_VT(unsigned int buffernum);
unsigned int get_vt_capBuf_Addr(unsigned int idx);
unsigned int get_vt_capBuf_Size(void);
void Capture_BufferMemDeInit_VT(unsigned int buffernum);

#endif

