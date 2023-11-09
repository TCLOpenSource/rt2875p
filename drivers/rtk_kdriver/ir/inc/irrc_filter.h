#ifndef __IRRC_FILTER_H__
#define __IRRC_FILTER_H__
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/delay.h>

#define MAX_SCANCODE_FILTER_ARRAY_NUM 16

typedef struct {
    u32 rp_mask;
    u32 rp_value;
    u32 rp2_mask;
    u32 rp2_value;
}IR_SCANCODE_FILTER_ELEMENT;

typedef struct {
    u32 filter_num;
    IR_SCANCODE_FILTER_ELEMENT filter_array[MAX_SCANCODE_FILTER_ARRAY_NUM];
    u8 filter_array_not_empty;
    rwlock_t lock;
    void *priv_data;
} IR_SCANCODE_FILTER_LIST;


void ir_scancode_filter_function_init(IR_SCANCODE_FILTER_LIST *p_scancode_filter_list, void *priv_data);
void ir_scancode_filter_function_uninit(IR_SCANCODE_FILTER_LIST *p_scancode_filter_list);
void ir_scancode_filter_parse_params(IR_SCANCODE_FILTER_LIST *p_scancode_filter_list, char *params);
int ir_add_scancode_filter(IR_SCANCODE_FILTER_LIST *p_scancode_filter_list, 
                                                        u32 rp_mask, u32 rp_value, u32 rp2_mask, u32 rp2_value);
int ir_del_scancode_filter(IR_SCANCODE_FILTER_LIST *p_scancode_filter_list, 
                                                        u32 rp_mask, u32 rp_value, u32 rp2_mask, u32 rp2_value);
int ir_query_scancode_filter(IR_SCANCODE_FILTER_LIST *p_scancode_filter_list, u32 rp_value, u32 rp2_value);


#endif
