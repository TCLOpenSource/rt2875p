#ifndef __RTK_CEC_SYSFS_H__
#define __RTK_CEC_SYSFS_H__

#include "../core/rtk_cec.h"
#include "../core/rtk_cm_buff.h"


unsigned char *get_logic_addr_name_by_logic_addr(unsigned char logic_addr);

void cec_msg_decode(cm_buff *msg,  unsigned char *mode, unsigned char *buf, unsigned int buf_len);

void cec_msg_dump(cm_buff *msg, unsigned char *status);


#endif
