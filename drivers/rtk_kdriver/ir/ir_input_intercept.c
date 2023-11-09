#include <linux/version.h>
#include "ir_input.h"
#include "ir_input_intercept.h"
#include "irrc_core.h"
#include "irrc_common.h"

int ir_add_keycode_to_list(IR_Keycode_List *p_keycode_list, u32 keycode, u32 flags)
{
    int ret = 0;
    u32 i = 0;
    if(!p_keycode_list || keycode == 0)
        return ret;
    write_lock(&p_keycode_list->lock);
    if(p_keycode_list->keycode_num < MAX_KEYCODE_ARRY_NUM) {
        for(i = 0; i < p_keycode_list->keycode_num; i++) {
            if(p_keycode_list->keycode_array[i][0] == keycode)
                break;
        }
        if(i == p_keycode_list->keycode_num) {
            p_keycode_list->keycode_array[p_keycode_list->keycode_num][0] = keycode;
            p_keycode_list->keycode_array[p_keycode_list->keycode_num][1] = flags;
            p_keycode_list->keycode_num++;
            ret = 1;  
        }
    }
    if(p_keycode_list->keycode_num > 0)
        p_keycode_list->keycode_array_not_empty = 1;
    write_unlock(&p_keycode_list->lock);
    return ret;
}

int ir_remove_keycode_from_list(IR_Keycode_List *p_keycode_list, u32 keycode)
{
    u32 i = 0;
    if(!p_keycode_list || keycode == 0)
        return 1;
    write_lock(&p_keycode_list->lock);
    for(i = 0; i < p_keycode_list->keycode_num; i++) {
        if(p_keycode_list->keycode_array[i][0] == keycode)
            break;
    }
    if(i < p_keycode_list->keycode_num) {
        for(; i < p_keycode_list->keycode_num - 1; i++) {
            p_keycode_list->keycode_array[i][0] = 
                        p_keycode_list->keycode_array[i + 1][0];
            p_keycode_list->keycode_array[i][1] = 
                        p_keycode_list->keycode_array[i + 1][1];            
        }
        p_keycode_list->keycode_num--;
    }
    if(p_keycode_list->keycode_num == 0)
        p_keycode_list->keycode_array_not_empty = 0;
    write_unlock(&p_keycode_list->lock);
    return 1;
}

int ir_query_keycode_from_list(IR_Keycode_List *p_keycode_list, u32 keycode, u32 *flags) 
{
    int ret = 0;
    u32 i = 0;
    if(!p_keycode_list || keycode == 0)
        return ret;
    read_lock(&p_keycode_list->lock);
    if(p_keycode_list->keycode_num > 0) {
        for(i = 0; i < p_keycode_list->keycode_num; i++) {
            if(p_keycode_list->keycode_array[i][0] == keycode) {
                if(flags)
                    *flags = p_keycode_list->keycode_array[i][1];
                ret = 1;
                break;
            }
        }
    }
    read_unlock(&p_keycode_list->lock);    
    return ret;
}


void ir_send_intercept_key_event(struct input_dev *input_dev, u32 keycode)
{
    char event[64] = {0};
    char *envp[2] = {NULL, NULL};
    
    if(input_dev == NULL)
        return;
        
    snprintf(event, sizeof(event), "intercept_keycode=%x", keycode);
    envp[0]= event;
    envp[1] = NULL;
    kobject_uevent_env(&input_dev->dev.kobj, KOBJ_CHANGE, envp);
}


static void ir_intercept_keycode_work(struct work_struct *work)
{
    IR_Keycode_List *p_keycode_list = container_of(work, IR_Keycode_List, work);
    if(p_keycode_list && p_keycode_list->priv_data) {
	struct venus_ir_data *data = (struct venus_ir_data *)p_keycode_list->priv_data;
    	ir_send_intercept_key_event(data->input_dev, data->keycode);
    }
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
static void venus_ir_key_disable_timer_func(struct timer_list *t)
#else
static void venus_ir_key_disable_timer_func(unsigned long _data)
#endif
{
    ir_decode_key_on_off(ON);
}



void ir_intercept_function_init(IR_Keycode_List *p_keycode_list, void *priv_data)
{
	if(!p_keycode_list)
        	return;
	rwlock_init(&p_keycode_list->lock);
        INIT_WORK(&p_keycode_list->work, ir_intercept_keycode_work);
	p_keycode_list->priv_data = priv_data;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        timer_setup(&p_keycode_list->timer, venus_ir_key_disable_timer_func, 0);
#else
        setup_timer(&p_keycode_list->timer, venus_ir_key_disable_timer_func, 0);
#endif
}

void ir_intercept_function_uninit(IR_Keycode_List *p_keycode_list)
{
	if(!p_keycode_list)
        	return;
	cancel_work_sync(&p_keycode_list->work);
        del_timer_sync(&p_keycode_list->timer);
}

