#ifndef __RTK_CEC_MSG_H__
#define __RTK_CEC_MSG_H__

#include <uapi/linux/cec.h>

void cec_msg_decode(struct cec_msg *msg,  unsigned char *mode, unsigned char *buf, unsigned int buf_len);

void cec_msg_dump(struct cec_msg *msg, unsigned char *status);

void cec_msg_set_print_enable(int opcode, unsigned char enable);

int cec_msg_get_print_enable(int opcode);

void cec_msg_clear_msg_count(int opcode);

int cec_msg_get_msg_count(int opcode);

#endif
