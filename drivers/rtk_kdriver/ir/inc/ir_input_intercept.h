#ifndef __IR_INPUT_INTERCEPT_H__
#define __IR_INPUT_INTERCEPT_H__
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/delay.h>

enum {
    KEYCODE_INTERCEPT_DISABLE_IRDA_DECODE = 0x00000001,
};
#define MAX_KEYCODE_ARRY_NUM 16
#define MAX_IR_DISABLE_KEY_TIME  (10 * HZ)
typedef struct {
    u32 keycode_num;
    /*keycode: keycode_array[i][0]; flags: keycode: keycode_array[i][1]*/
    u32 keycode_array[MAX_KEYCODE_ARRY_NUM][2];
    u8 keycode_array_not_empty;
    rwlock_t lock;
    struct work_struct work;
    struct timer_list timer;    
    void *priv_data;
} IR_Keycode_List;


void ir_intercept_function_init(IR_Keycode_List *p_keycode_list, void *priv_data);
void ir_intercept_function_uninit(IR_Keycode_List *p_keycode_list);

int ir_add_keycode_to_list(IR_Keycode_List *p_keycode_list, u32 keycode, u32 flags);
int ir_remove_keycode_from_list(IR_Keycode_List *p_keycode_list, u32 keycode);
int ir_query_keycode_from_list(IR_Keycode_List *p_keycode_list, u32 keycode, u32 *flags) ;
void ir_send_intercept_key_event(struct input_dev *input_dev, u32 keycode);

#endif