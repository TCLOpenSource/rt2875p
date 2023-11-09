#include "ir_input.h"
#include "irrc_core.h"
#include "irrc_common.h"
#include "irrc_filter.h"
int ir_add_scancode_filter(IR_SCANCODE_FILTER_LIST *p_scancode_filter_list, 
                                                            u32 rp_mask, u32 rp_value, u32 rp2_mask, u32 rp2_value)
{
    unsigned long flags;
    int ret = 0;
    u32 i = 0;
    if(!p_scancode_filter_list)
        return ret;
    write_lock_irqsave(&p_scancode_filter_list->lock, flags);
    if(p_scancode_filter_list->filter_num < MAX_SCANCODE_FILTER_ARRAY_NUM) {
        for(i = 0; i < p_scancode_filter_list->filter_num; i++) {
            if((p_scancode_filter_list->filter_array[i].rp_mask == rp_mask) 
                && (p_scancode_filter_list->filter_array[i].rp_value == rp_value)
                && (p_scancode_filter_list->filter_array[i].rp2_mask == rp2_mask) 
                && (p_scancode_filter_list->filter_array[i].rp2_value == rp2_value))
                break;
        }
        if(i == p_scancode_filter_list->filter_num) {
            p_scancode_filter_list->filter_array[i].rp_mask = rp_mask;
            p_scancode_filter_list->filter_array[i].rp_value = rp_value;
            p_scancode_filter_list->filter_array[i].rp2_mask = rp2_mask;
            p_scancode_filter_list->filter_array[i].rp2_value = rp2_value;
            p_scancode_filter_list->filter_num++;
            ret = 1;  
        }
    }
    if(p_scancode_filter_list->filter_num > 0)
        p_scancode_filter_list->filter_array_not_empty = 1;
    write_unlock_irqrestore(&p_scancode_filter_list->lock, flags);
    return ret;
}

int ir_del_scancode_filter(IR_SCANCODE_FILTER_LIST *p_scancode_filter_list, 
                                                        u32 rp_mask, u32 rp_value, u32 rp2_mask, u32 rp2_value)
{
    unsigned long flags;
    u32 i = 0;
    if(!p_scancode_filter_list)
        return 1;
    write_lock_irqsave(&p_scancode_filter_list->lock, flags);
    for(i = 0; i < p_scancode_filter_list->filter_num; i++) {
        if((p_scancode_filter_list->filter_array[i].rp_mask == rp_mask) 
                && (p_scancode_filter_list->filter_array[i].rp_value == rp_value)
                && (p_scancode_filter_list->filter_array[i].rp2_mask == rp2_mask) 
                && (p_scancode_filter_list->filter_array[i].rp2_value == rp2_value))
                break;
    }
    if(i < p_scancode_filter_list->filter_num) {
        for(; i < p_scancode_filter_list->filter_num - 1; i++) {
            p_scancode_filter_list->filter_array[i] = 
                        p_scancode_filter_list->filter_array[i + 1];      
        }
        p_scancode_filter_list->filter_num--;
    }
    if(p_scancode_filter_list->filter_num == 0)
        p_scancode_filter_list->filter_array_not_empty = 0;
    write_unlock_irqrestore(&p_scancode_filter_list->lock, flags);
    return 1;
}

int ir_query_scancode_filter(IR_SCANCODE_FILTER_LIST *p_scancode_filter_list, 
                                                                u32 rp_value, u32 rp2_value) 
{
    unsigned long flags;
    int ret = 0;
    u32 i = 0;
    if(!p_scancode_filter_list)
        return ret;
    read_lock_irqsave(&p_scancode_filter_list->lock, flags);
    if(p_scancode_filter_list->filter_num > 0) {
        for(i = 0; i < p_scancode_filter_list->filter_num; i++) {
            if(((rp_value & p_scancode_filter_list->filter_array[i].rp_mask) == p_scancode_filter_list->filter_array[i].rp_value)
                && ((rp2_value & p_scancode_filter_list->filter_array[i].rp2_mask) == p_scancode_filter_list->filter_array[i].rp2_value)) {
                ret = 1;
                break;
            }
        }
    } else {
        ret = 1;
    }
    read_unlock_irqrestore(&p_scancode_filter_list->lock, flags);    
    return ret;
}



void ir_scancode_filter_function_init(IR_SCANCODE_FILTER_LIST *p_scancode_filter_list, void *priv_data)
{
	if(!p_scancode_filter_list)
        	return;
	rwlock_init(&p_scancode_filter_list->lock);
	p_scancode_filter_list->priv_data = priv_data;
}

void ir_scancode_filter_function_uninit(IR_SCANCODE_FILTER_LIST *p_scancode_filter_list)
{
}

void ir_scancode_filter_parse_params(IR_SCANCODE_FILTER_LIST *p_scancode_filter_list, char *params)
{
    u32 rp_mask = 0;
    u32 rp_value = 0;
    u32 rp2_mask = 0;
    u32 rp2_value = 0;
    char *pTmp = NULL;
    if(!p_scancode_filter_list || !params)
        return;
    while(NULL != ( pTmp = strsep(&params, "-"))) {
	if(sscanf(pTmp, "%x,%x,%x,%x", &rp_mask, &rp_value, &rp2_mask, &rp2_value) == 4)
		ir_add_scancode_filter(p_scancode_filter_list, rp_mask, rp_value, rp2_mask, rp2_value);
    }
}

