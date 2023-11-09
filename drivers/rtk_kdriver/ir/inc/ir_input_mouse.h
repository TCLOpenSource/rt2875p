#ifndef __IR_INPUT_MOUSE_H
#define __IR_INPUT_MOUSE_H

#include <linux/input.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/delay.h>
#include "ir_debug.h"

int venus_ir_mouse_init(struct venus_ir_data *data);
void venus_ir_mouse_cleanup(struct venus_ir_data *data);
void venus_ir_mouse_report_event(struct venus_ir_data *data, uint32_t value);
void venus_ir_mouse_report_end(struct venus_ir_data *data, uint32_t value);
int venus_ir_is_simulated_key(struct venus_ir_data *data, u32 key);

#endif


