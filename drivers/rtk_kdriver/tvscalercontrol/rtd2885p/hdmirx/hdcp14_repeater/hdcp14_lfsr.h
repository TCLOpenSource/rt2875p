#ifndef __HDCP_LFSR_MODULE_H__
#define __HDCP_LFSR_MODULE_H__

#include "hdcp14_api.h"

#define GETBIT(x,i) (((x) & ((0x1ll) << (i))) >> i)

typedef struct {
	u32 lfsr0;//:13;
	u32 lfsr1;//':14;
	u32 lfsr2;//:16;
	u32 lfsr3;//:17;
	u32 sh0;//:2;
	u32 sh1;//:2;
	u32 sh2;//:2;
	u32 sh3;//:2;
	u32 inited;
	u32 clocks;
}LFSR;

LFSR* lfsr_init(u8 *input);
void lfsr_print(LFSR* lfsr_handle);
u32 do_lfsr(LFSR* lfsr_handle);
void lfsr_uninit(LFSR* lfsr_handle);


#endif