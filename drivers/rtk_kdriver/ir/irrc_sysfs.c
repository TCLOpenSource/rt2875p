#include "irrc_sysfs.h"
#include "ir_input.h"
#include "ir_input_intercept.h"
#include "irrc_filter.h"

#define IR_INPUT_DEVICE_TEST    0

#ifdef CONFIG_RTK_KDRV_INPUT_DEVICE
extern IR_Keycode_List intercept_keycode_list;
#endif
extern uint32_t ir_disable_irda_decode_key;
extern IR_SCANCODE_FILTER_LIST g_scancode_filter_list;
extern void ir_decode_key_on_off(bool on) ;
extern void ir_gen_keyup_event(uint32_t keycode);
extern void ir_gen_keydown_event(uint32_t keycode);

const char* ir_core_get_cur_remote_control_name(void);

static ssize_t ir_protocol_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int tlen = 0;
    unsigned int i = 0;

    snprintf(buf, PAGE_SIZE, "\nThe current selection is [%d] %s\n", ir_support_protocols[0],
            irda_proto_list[ir_support_protocols[0]].proto_name);

    for(i = 1; i < IRDA_PROTO_LIST_SIZE ; i++) {
        tlen = strlen(buf);
        snprintf((buf + tlen), PAGE_SIZE - tlen, "\n[%d] %s", irda_proto_list[i].proto_id, irda_proto_list[i].proto_name);
    }
    tlen = strlen(buf);
    snprintf((buf + tlen), PAGE_SIZE - tlen, "\n\nYou can \"echo id > /sys/venus_ir/ir_protocol\" to switch protocol\n");

    return strlen(buf);
}

static ssize_t ir_protocol_store(struct kobject * kobj, struct kobj_attribute * attr, const char *buf, size_t n)
{
    unsigned int val = 0;

    if (kstrtouint(buf, 10, &val) == 0) {

        if(val < IRDA_PROTO_LIST_SIZE) {
            ir_support_protocols[0] = val;
            ir_protocol = val;

            spin_lock(&ir_lock);
            if(IR_Init(ir_support_protocols[0], ir_support_protocols[1], ON) == 0)
                kfifo_reset(&ir_fifo);
#ifdef CONFIG_RTK_KDRV_INPUT_DEVICE
            venus_ir_input_set_input_dev_name(ir_core_get_cur_remote_control_name());  
#endif    
            spin_unlock(&ir_lock);

            return n;
        } else {
            IR_WARNING("You don't select any current irda protocol\n");
        }
    }

    return -EINVAL;
}

ir_attr(ir_protocol);

static ssize_t ir_protocol_main0_show(struct kobject * kobj, struct kobj_attribute * attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%d\n", ir_support_protocols[1]);
}

static ssize_t ir_protocol_main0_store(struct kobject * kobj, struct kobj_attribute * attr, const char *buf, size_t n)
{
    unsigned int val = 0;

    if (kstrtouint(buf, 10, &val) == 0) {
        ir_support_protocols[1] = val;
        ir_protocol = val;

        spin_lock(&ir_lock);
        if(IR_Init(ir_support_protocols[0], ir_support_protocols[1], ON) == 0)
            kfifo_reset(&ir_fifo);
        spin_unlock(&ir_lock);

        return n;
    }

    return -EINVAL;
}

ir_attr(ir_protocol_main0);


static int ir_count_protocol_attr_value(unsigned int setting, unsigned int multiplication)
{
    unsigned int irsbc = 20;/*us*/
    return setting * irsbc * multiplication;
}

static int ir_get_register_bits_val(unsigned long reg, unsigned int msb,
                                    unsigned int lsb, unsigned int *val, unsigned int multiplication)
{
    unsigned int reg_val;
    if (msb < lsb || lsb > 31 || msb > 31 || val == NULL)
        return -1;
    reg_val = rtd_inl(reg);
    reg_val = (reg_val & IR_BITS_MASK(msb, lsb)) >> lsb;
    *val = ir_count_protocol_attr_value(reg_val, multiplication);
    return 0;
}

static int ir_get_protocol_attr(int core, char *attr_name, unsigned int *val)
{
    int ret = 0;
    if(attr_name == NULL)
        return -1;
    if(strcmp(attr_name, "burst_lead") == 0)
        ret = ir_get_register_bits_val(core==0? REG_IR_PSR:REG_IR_PSR_MAIN1, 31, 24, val, 4);
    else if(strcmp(attr_name, "modulation") == 0)
        ret = ir_get_register_bits_val(core==0? REG_IR_PSR:REG_IR_PSR_MAIN1, 23, 16, val, 1);
    else if(strcmp(attr_name, "data0") == 0)
        ret = ir_get_register_bits_val(core==0? REG_IR_PSR:REG_IR_PSR_MAIN1, 15, 8, val, 1);
    else if(strcmp(attr_name, "data1") == 0)
        ret = ir_get_register_bits_val(core==0? REG_IR_PSR:REG_IR_PSR_MAIN1, 7, 0, val, 1);
    else if(strcmp(attr_name, "burst_repeat") == 0)
        ret = ir_get_register_bits_val(core==0? REG_IR_PER:REG_IR_PER_MAIN1, 23, 16, val, 4);
    else if(strcmp(attr_name, "silence_repeat") == 0)
        ret = ir_get_register_bits_val(core==0? REG_IR_PER:REG_IR_PER_MAIN1, 15, 8, val, 4);
    else if(strcmp(attr_name, "silence_lead") == 0)
        ret = ir_get_register_bits_val(core==0? REG_IR_PER:REG_IR_PER_MAIN1, 7, 0, val, 4);

    else if(strcmp(attr_name, "burst_lead_up") == 0)
        ret = ir_get_register_bits_val(core==0? REG_IR_PSR_UP:REG_IR_PSR_UP_MAIN1, 31, 24, val, 4);
    else if(strcmp(attr_name, "modulation_up") == 0)
        ret = ir_get_register_bits_val(core==0? REG_IR_PSR_UP:REG_IR_PSR_UP_MAIN1, 23, 16, val, 1);
    else if(strcmp(attr_name, "data0_up") == 0)
        ret = ir_get_register_bits_val(core==0? REG_IR_PSR_UP:REG_IR_PSR_UP_MAIN1, 15, 8, val, 1);
    else if(strcmp(attr_name, "data1_up") == 0)
        ret = ir_get_register_bits_val(core==0? REG_IR_PSR_UP:REG_IR_PSR_UP_MAIN1, 7, 0, val, 1);
    else if(strcmp(attr_name, "burst_repeat_up") == 0)
        ret = ir_get_register_bits_val(core==0? REG_IR_PER_UP:REG_IR_PER_UP_MAIN1, 31, 24, val, 4);
    else if(strcmp(attr_name, "silence_repeat_up") == 0)
        ret = ir_get_register_bits_val(core==0? REG_IR_PER_UP:REG_IR_PER_UP_MAIN1, 23, 16, val, 4);
    else if(strcmp(attr_name, "end_pattern") == 0)
        ret = ir_get_register_bits_val(core==0? REG_IR_PER_UP:REG_IR_PER_UP_MAIN1, 15, 8, val, 4);
    else if(strcmp(attr_name, "silence_lead_up") == 0)
        ret = ir_get_register_bits_val(core==0? REG_IR_PER_UP:REG_IR_PER_UP_MAIN1, 7, 0, val, 4);
    else if(strcmp(attr_name, "idle_time")==0) //IRIOTCDP
        ret = ir_get_register_bits_val(core==0? REG_IR_DPIR:REG_IR_DPIR_MAIN1, 15, 0, val, 1);
    else if(strcmp(attr_name, "reset_time")==0) //fsm_reset_time
        ret = ir_get_register_bits_val(core==0? REG_IR_DPIR:REG_IR_DPIR_MAIN1, 23, 16, val, 8);
    else
        ret = -1;
    return ret;
}

static int ir_count_protocol_attr_setting(unsigned int val, unsigned int divide)
{
    unsigned int irsbc = 20;/*us*/
    unsigned int setting = 0;
    if(divide == 0)
        divide = 1;
    setting = val / (irsbc * divide);
    return setting;
}
static int ir_set_register_bits_val(unsigned long reg, unsigned int msb,
                                    unsigned int lsb, unsigned int val, unsigned int divide)
{
    unsigned int reg_val;
    if (msb < lsb || lsb > 31 || msb > 31)
        return -1;
    val = ir_count_protocol_attr_setting(val, divide);
    reg_val = rtd_inl(reg);
    IR_DBG( "before %08x == %08x\n", (unsigned int)reg, rtd_inl(reg));
    reg_val = (reg_val & (~IR_BITS_MASK(msb, lsb))) |
              ((val << lsb) & IR_BITS_MASK(msb, lsb));
    rtd_outl(reg, reg_val);
    IR_DBG( "after %08x == %08x\n", (unsigned int)reg, rtd_inl(reg));
    return 0;
}

static int ir_set_protocol_attr(int core, char *attr_name, unsigned int val)
{
    int ret = 0;
    if(attr_name == NULL)
        return -1;

    if(strcmp(attr_name, "burst_lead") == 0)
        ret = ir_set_register_bits_val(core==0?REG_IR_PSR:REG_IR_PSR_MAIN1, 31, 24, val, 4);
    else if(strcmp(attr_name, "modulation") == 0)
        ret = ir_set_register_bits_val(core==0?REG_IR_PSR:REG_IR_PSR_MAIN1, 23, 16, val, 1);
    else if(strcmp(attr_name, "data0") == 0)
        ret = ir_set_register_bits_val(core==0?REG_IR_PSR:REG_IR_PSR_MAIN1, 15, 8, val, 1);
    else if(strcmp(attr_name, "data1") == 0)
        ret = ir_set_register_bits_val(core==0?REG_IR_PSR:REG_IR_PSR_MAIN1, 7, 0, val, 1);
    else if(strcmp(attr_name, "burst_repeat") == 0)
        ret = ir_set_register_bits_val(core==0?REG_IR_PER:REG_IR_PER_MAIN1, 23, 16, val, 4);
    else if(strcmp(attr_name, "silence_repeat") == 0)
        ret = ir_set_register_bits_val(core==0?REG_IR_PER:REG_IR_PER_MAIN1, 15, 8, val, 4);
    else if(strcmp(attr_name, "silence_lead") == 0)
        ret = ir_set_register_bits_val(core==0?REG_IR_PER:REG_IR_PER_MAIN1, 7, 0, val, 4);

    else if(strcmp(attr_name, "burst_lead_up") == 0)
        ret = ir_set_register_bits_val(core==0?REG_IR_PSR_UP:REG_IR_PSR_UP_MAIN1, 31, 24, val, 4);
    else if(strcmp(attr_name, "modulation_up") == 0)
        ret = ir_set_register_bits_val(core==0?REG_IR_PSR_UP:REG_IR_PSR_UP_MAIN1, 23, 16, val, 1);
    else if(strcmp(attr_name, "data0_up") == 0)
        ret = ir_set_register_bits_val(core==0?REG_IR_PSR_UP:REG_IR_PSR_UP_MAIN1, 15, 8, val, 1);
    else if(strcmp(attr_name, "data1_up") == 0)
        ret = ir_set_register_bits_val(core==0?REG_IR_PSR_UP:REG_IR_PSR_UP_MAIN1, 7, 0, val, 1);
    else if(strcmp(attr_name, "burst_repeat_up") == 0)
        ret = ir_set_register_bits_val(core==0?REG_IR_PER_UP:REG_IR_PER_UP_MAIN1, 31, 24, val, 4);
    else if(strcmp(attr_name, "silence_repeat_up") == 0)
        ret = ir_set_register_bits_val(core==0?REG_IR_PER_UP:REG_IR_PER_UP_MAIN1, 23, 16, val, 4);
    else if(strcmp(attr_name, "end_pattern") == 0)
        ret = ir_set_register_bits_val(core==0?REG_IR_PER_UP:REG_IR_PER_UP_MAIN1, 15, 8, val, 4);
    else if(strcmp(attr_name, "silence_lead_up") == 0)
        ret = ir_set_register_bits_val(core==0?REG_IR_PER_UP:REG_IR_PER_UP_MAIN1, 7, 0, val, 4);
    else if(strcmp(attr_name, "idle_time")==0) //IRIOTCDP
        ret = ir_set_register_bits_val(core==0? REG_IR_DPIR:REG_IR_DPIR_MAIN1, 15, 0, val, 1);
    else if(strcmp(attr_name, "reset_time")==0) //fsm_reset_time
        ret = ir_set_register_bits_val(core==0? REG_IR_DPIR:REG_IR_DPIR_MAIN1, 23, 16, val, 8);
    else
        ret = -1;
    return ret;
}

static ssize_t ir_protocol_attr_show(struct kobject * kobj, struct kobj_attribute * attr, char *buf)
{
    unsigned int val = 0;
    if(ir_get_protocol_attr(1, protocol_attr_name, &val) == 0)
        return snprintf(buf, PAGE_SIZE, "the current setting of %s is %d(us)\n", protocol_attr_name, val);
    return 0;
}

static ssize_t ir_protocol_attr_store(struct kobject * kobj, struct kobj_attribute * attr, const char *buf, size_t n)
{
    unsigned char action[4];
    memset(action, 0, 4);

    if (sscanf(buf, "%3s", action) == 1) {
        IR_DBG( "ir_protocol_attr_store:buf == %s, action == %s\n", buf, action);
        if(strcmp(action, "set") == 0) {
            unsigned int val = 0;
            char attr_name[32] = {0};
            buf += 4;
            if (sscanf(buf, "%31s%08u", attr_name, &val) == 2) {
                IR_DBG( "ir_protocol_attr_store: set attr_name =%s val =%u\n", attr_name, val);
                if(ir_set_protocol_attr(1,attr_name, val) == 0)
                    return n;
                else
                    return -EINVAL;
            }
        } else if(strcmp(action, "get") == 0) {
            buf += 4;
            if (sscanf(buf, "%31s", protocol_attr_name) == 1) {
                IR_DBG( "ir_protocol_attr_store:get attr_name = %s\n", protocol_attr_name);
                return n;
            } else {
                return -EINVAL;
            }
        }
    }

    return -EINVAL;
}
ir_attr(ir_protocol_attr);


static ssize_t ir_protocol_attr_main0_show(struct kobject * kobj, struct kobj_attribute * attr, char *buf)
{
    unsigned int val = 0;
    if(ir_get_protocol_attr(0, protocol_attr_name, &val) == 0)
        return snprintf(buf, PAGE_SIZE, "the current setting of %s is %d(us)\n", protocol_attr_name, val);
    return 0;
}

static ssize_t ir_protocol_attr_main0_store(struct kobject * kobj, struct kobj_attribute * attr, const char *buf, size_t n)
{
    char action[4];
    memset(action, 0, 4);

    if (sscanf(buf, "%3s", action) == 1) {
        IR_DBG( "ir_protocol_attr_store:buf == %s, action == %s\n", buf, action);
        if(strcmp(action, "set") == 0) {
            unsigned int val = 0;
            char attr_name[32] = {0};
            buf += 4;
            if (sscanf(buf, "%31s%08u", attr_name, &val) == 2) {
                IR_DBG( "ir_protocol_attr_store: set attr_name =%s val =%u\n", attr_name, val);
                if(ir_set_protocol_attr(0, attr_name, val) == 0)
                    return n;
                else
                    return -EINVAL;
            }
        } else if(strcmp(action, "get") == 0) {
            buf += 4;
            if (sscanf(buf, "%31s", protocol_attr_name) == 1) {
                IR_DBG( "ir_protocol_attr_store:get attr_name = %s\n", protocol_attr_name);
                return n;
            } else {
                return -EINVAL;
            }
        }
    }

    return -EINVAL;
}
ir_attr(ir_protocol_attr_main0);


static ssize_t ir_repeat_interval_show(struct kobject * kobj, struct kobj_attribute * attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%d\n", ir_repeat_interval);
}

static ssize_t ir_repeat_interval_store(struct kobject * kobj, struct kobj_attribute * attr, const char *buf, size_t n)
{
    unsigned int val = 0;

    if (kstrtouint(buf, 10, &val) == 0) {
        ir_repeat_interval = val;
        return n;
    }

    return -EINVAL;
}

ir_attr(ir_repeat_interval);

extern unsigned int debounce;
static ssize_t ir_debounce_show(struct kobject * kobj, struct kobj_attribute * attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%d\n", debounce);
}

static ssize_t ir_debounce_store(struct kobject * kobj, struct kobj_attribute * attr, const char *buf, size_t n)
{
    unsigned int val = 0;

    if (kstrtouint(buf, 10, &val) == 0) {
        debounce = val;
        return n;
    }

    return -EINVAL;
}

ir_attr(ir_debounce);

#ifdef CONFIG_DELAY_FIRST_REPEAT
extern unsigned int firstRepeatInterval;
static ssize_t ir_first_repeat_show(struct kobject * kobj, struct kobj_attribute * attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%d\n", firstRepeatInterval);
}

static ssize_t ir_first_repeat_store(struct kobject * kobj, struct kobj_attribute * attr, const char *buf, size_t n)
{
    unsigned int val = 0;

    if (kstrtouint(buf, 10, &val) == 0) {
        firstRepeatInterval = val;
        return n;
    }

    return -EINVAL;
}

ir_attr(ir_first_repeat);
#endif

static ssize_t ir_decode_debug_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    if (strcmp(attr->attr.name, "ir_decode_debug") == 0) {
        return snprintf(buf, PAGE_SIZE, "ir decode debug status:%s\n", ir_decode_dbg_en ? "enable" : "disable");
    }
    return 0;
}

static ssize_t ir_decode_debug_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
    if(strcmp(attr->attr.name, "ir_decode_debug") == 0) {
        unsigned int val = 0;
        if (kstrtouint(buf, 10, &val) == 0) {
            ir_decode_dbg_en = val ? 1 : 0;
            return n;
        }
    }
    return -EINVAL;
}

ir_attr(ir_decode_debug);

static ssize_t ir_irda_key_decode_on_off_show(struct kobject * kobj, struct kobj_attribute * attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "irda_key_on_off == %s\n", (ir_disable_irda_decode_key > 0) ? "on" : "off");
}

static ssize_t ir_irda_key_decode_on_off_store(struct kobject * kobj, struct kobj_attribute * attr, const char *buf, size_t n)
{
    uint32_t val = 0;
    if (kstrtouint(buf, 10, &val) == 0) {
        val = (val == 0) ? 0 : 1;
        if(val) {
#ifdef CONFIG_RTK_KDRV_INPUT_DEVICE
            del_timer_sync(&intercept_keycode_list.timer);
#endif
            IR_WARNING("Disable IR decode key Func\n");

            ir_decode_key_on_off(OFF);
        } else {
#ifdef CONFIG_RTK_KDRV_INPUT_DEVICE
            del_timer_sync(&intercept_keycode_list.timer);
#endif
            ir_decode_key_on_off(ON);
        }
        return n;
    }
    return -EINVAL;
}


ir_attr(ir_irda_key_decode_on_off);

static ssize_t ir_scancode_filter_show(struct kobject * kobj, struct kobj_attribute * attr, char *buf)
{
    return sprintf(buf, "echo add/del,rp_mask,rp_value,rp2_mask,rp2_value >/sys/venus_ir/ir_scancode_filter\n");
}

static ssize_t ir_scancode_filter_store(struct kobject * kobj, struct kobj_attribute * attr, const char *buf, size_t n)
{
    char action[4] = {0};
    u32 rp_mask = 0;
    u32 rp_value = 0;
    u32 rp2_mask = 0;
    u32 rp2_value = 0;

    if (sscanf(buf, "%3s,%x,%x,%x,%x", action, &rp_mask, &rp_value, &rp2_mask, &rp2_value) == 5) {
        if(strncmp(action, "add", 3) == 0) {
            ir_add_scancode_filter(&g_scancode_filter_list, rp_mask, rp_value, rp2_mask, rp2_value);
        } else if(strncmp(action, "del", 3) == 0) {
            ir_del_scancode_filter(&g_scancode_filter_list, rp_mask, rp_value, rp2_mask, rp2_value);
        } else {
            return -EINVAL;
        }
        return n;
    }

    return -EINVAL;
}


ir_attr(ir_scancode_filter);

static ssize_t ir_remote_name_show(struct kobject * kobj, struct kobj_attribute * attr, char *buf)
{
    const char *remote_name = ir_core_get_cur_remote_control_name();
    if(remote_name)
        return snprintf(buf, PAGE_SIZE, "%s", remote_name);
    else
        return 0;
}

static ssize_t ir_remote_name_store(struct kobject * kobj, struct kobj_attribute * attr, 
                                                                    const char *buf, size_t n)
{
    return n;
}

ir_attr(ir_remote_name);

#ifdef CONFIG_RTK_KDRV_INPUT_DEVICE
static unsigned int scancode = 0;
static unsigned int keycode = 0;

static ssize_t ir_gen_keycode_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return 0;
}

static ssize_t ir_gen_keycode_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
    if(strcmp(attr->attr.name, "ir_gen_keycode") == 0) {
        if (kstrtouint(buf, 16, &keycode) == 0) {
            venus_ir_gen_keyupdown_event(keycode);
            return n;
        }
    }
    return -EINVAL;
}

ir_attr(ir_gen_keycode);

static ssize_t ir_poll_interval_show(struct kobject * kobj, struct kobj_attribute * attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "[MAIN1]poll_enable = %u, interval = %u\n", ir_poll_settings[MAIN1][0], ir_poll_settings[MAIN1][1]);
}

static ssize_t ir_poll_interval_store(struct kobject * kobj, struct kobj_attribute * attr, const char *buf, size_t n)
{
    int val, enable;

    if (sscanf(buf, "%08d %08d", &enable, &val) == 2) {
        ir_poll_settings[MAIN1][0] = enable;
        if(val < 150){
            ir_poll_settings[MAIN1][1] = val;
        }else{
            IR_WARNING("val:%d is invaild\n",val);
        }
        return n;
    }

    return -EINVAL;
}

ir_attr(ir_poll_interval);

static ssize_t ir_poll_settings_show(struct kobject * kobj, struct kobj_attribute * attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "[MAIN1]poll_enable = %u, interval = %u\n[MAIN0]poll_enable = %u, interval = %u\n", 
                                    ir_poll_settings[MAIN1][0], ir_poll_settings[MAIN1][1],
                                    ir_poll_settings[MAIN0][0], ir_poll_settings[MAIN0][1]);
}

static ssize_t ir_poll_settings_store(struct kobject * kobj, struct kobj_attribute * attr, const char *buf, size_t n)
{
    ssize_t ret = -EINVAL;
    unsigned int which , val, enable;
    if (sscanf(buf, "%08u,%08u,%08u", &which, &enable, &val) == 3) {
        if((which <= MAIN1) && (enable <= 1)) {
            ir_poll_settings[which][0] = enable;
            if(val < 150){
                ir_poll_settings[which][1] = val;
            }else{
                IR_WARNING("val:%d is invaild\n",val);
            }
            ret = n;
        }
    }
    return n;
}

ir_attr(ir_poll_settings);

static ssize_t ir_keycode_to_scancode_show(struct kobject * kobj, struct kobj_attribute * attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "keycode = %d, scancode = 0x%x\n", keycode, scancode);
}

static ssize_t ir_keycode_to_scancode_store(struct kobject * kobj, struct kobj_attribute * attr, const char *buf, size_t n)
{
    if (kstrtouint(buf, 10, &keycode) == 0) {
        unsigned int tmp[1] = {0};
        venus_ir_keycode_to_scancode(keycode, tmp, 1);
        scancode = tmp[0];
        return n;
    }

    return -EINVAL;
}

ir_attr(ir_keycode_to_scancode);

static ssize_t ir_mute_irda_keyevent_show(struct kobject * kobj, struct kobj_attribute * attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "irda mute status = %s\n", ir_mute_flag ? "mute" : "unmute");
}

static ssize_t ir_mute_irda_keyevent_store(struct kobject * kobj, struct kobj_attribute * attr, const char *buf, size_t n)
{
    unsigned int tmp = 0;
    if (kstrtouint(buf, 10, &tmp) == 0) {
        ir_mute_flag = tmp ? 1 : 0;
        return n;
    }
    return -EINVAL;
}

ir_attr(ir_mute_irda_keyevent);

static ssize_t ir_to_mouse_show(struct kobject * kobj, struct kobj_attribute * attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%d\n", ir_to_mouse);
}

static ssize_t ir_to_mouse_store(struct kobject * kobj, struct kobj_attribute * attr, const char *buf, size_t n)
{
    unsigned int val = 0;

    if (kstrtouint(buf, 10, &val) == 0) {
        ir_to_mouse = val;

        return n;
    }

    return -EINVAL;
}

ir_attr(ir_to_mouse);

static ssize_t ir_repeat_event_show(struct kobject * kobj, struct kobj_attribute * attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%d\n", repeat_event);
}

static ssize_t ir_repeat_event_store(struct kobject * kobj, struct kobj_attribute * attr, const char *buf, size_t n)
{
    unsigned int val = 0;

    if (kstrtouint(buf, 10, &val) == 0) {
        repeat_event = val;

        return n;
    }

    return -EINVAL;
}

ir_attr(ir_repeat_event);

static ssize_t ir_rawkey_event_show(struct kobject * kobj, struct kobj_attribute * attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%d\n", rawkey_event);
}

static ssize_t ir_rawkey_event_store(struct kobject * kobj, struct kobj_attribute * attr, const char *buf, size_t n)
{
    unsigned int val = 0;

    if (kstrtouint(buf, 10, &val) == 0) {
        rawkey_event = val;

        return n;
    }

    return -EINVAL;
}

ir_attr(ir_rawkey_event);

static ssize_t ir_report_mode_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    unsigned int i = 0;
    if (strcmp(attr->attr.name, "ir_report_mode") == 0)
    {
        int tlen = 0;
        snprintf(buf, PAGE_SIZE, "\nThe current selection is [%d] %s\n"
                ,ir_report_mode
                ,ir_report_mode_list[ir_report_mode].name);

        for(i = 0; i < IRDA_REPORT_MODE_MAX ; i++) {
            tlen = strlen(buf);
            snprintf(buf + tlen, PAGE_SIZE- tlen, "\n[%d] %s", i, ir_report_mode_list[i].name);
        }
        tlen = strlen(buf);
        snprintf((buf + tlen), PAGE_SIZE- tlen, "\n\nYou can \"echo id > /sys/venus_ir/ir_report_mode\" to switch report mode\n");
        return strlen(buf);
    }
    return 0;
}

static ssize_t ir_report_mode_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
    if(0 == strcmp(attr->attr.name, "ir_report_mode"))
    {
        unsigned int val = 0;
        if (kstrtouint(buf, 10, &val) == 0)
        {
            if(val < IRDA_REPORT_MODE_MAX)
            {
                ir_report_mode = val;
            }
            else
            {
                IR_WARNING("you don't select any ir_report_mode and set it to %s\n"
                          ,ir_report_mode_list[ir_report_mode].name);
                n = -EINVAL;
            }
            return n;
        }
    }
    return -EINVAL;
}
ir_attr(ir_report_mode);

static ssize_t ir_is_upload_more_key_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    if (strcmp(attr->attr.name, "ir_is_upload_more_key") == 0)
    {
        return snprintf(buf, PAGE_SIZE, "ir_is_upload_more_key:%s\n", ir_is_upload_more_key?"enable" : "disable");;
    }
    return 0;
}

static ssize_t ir_is_upload_more_key_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
    if(0 == strcmp(attr->attr.name, "ir_is_upload_more_key"))
    {
        unsigned int val = 0;
        if (kstrtouint(buf, 10, &val) == 0)
        {
            ir_is_upload_more_key = val ? 1 : 0;
            return n;
        }
    }
    return -EINVAL;
}
ir_attr(ir_is_upload_more_key);

static ssize_t ir_set_intercept_key_show(struct kobject * kobj, struct kobj_attribute * attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "echo add/del,keycode >/sys/venus/ir_set_intercept_key\n");
}

static ssize_t ir_set_intercept_key_store(struct kobject * kobj, struct kobj_attribute * attr, const char *buf, size_t n)
{
    char action[4] = {0};
    u32 keycode = 0;
    u32 flags = 0;

    if (sscanf(buf, "%3s,%08x, %10x", action, &keycode, &flags) == 3) {
        if(strncmp(action, "add", 3) == 0) {
            ir_add_keycode_to_list(&intercept_keycode_list, keycode, flags);
        } else if(strncmp(action, "del", 3) == 0) {
            ir_remove_keycode_from_list(&intercept_keycode_list, keycode);
        } else {
            return -EINVAL;
        }
        return n;
    }

    return -EINVAL;
}


ir_attr(ir_set_intercept_key);

#endif

#ifdef CONFIG_RTK_KDRV_INPUT_DEVICE_MOUSE
extern int ir_mouse_pointer_speed;
extern int ir_mouse_pointer_threshold;
extern int ir_mouse_pointer_timer;
extern int ir_mouse_pointer_step;

static ssize_t ir_mouse_pointer_speed_show(struct kobject * kobj, struct kobj_attribute * attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%d\n", ir_mouse_pointer_speed);
}

static ssize_t ir_mouse_pointer_speed_store(struct kobject * kobj, struct kobj_attribute * attr, const char *buf, size_t n)
{
    unsigned int val = 0;

    if (kstrtouint(buf, 10, &val) == 0) {
        ir_mouse_pointer_speed = val;

        return n;
    }

    return -EINVAL;
}

ir_attr(ir_mouse_pointer_speed);

static ssize_t ir_mouse_pointer_threshold_show(struct kobject * kobj, struct kobj_attribute * attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%d\n", ir_mouse_pointer_threshold);
}

static ssize_t ir_mouse_pointer_threshold_store(struct kobject * kobj, struct kobj_attribute * attr, const char *buf, size_t n)
{
    unsigned int val = 0;

    if (kstrtouint(buf, 10, &val) == 0) {
        ir_mouse_pointer_threshold = val;

        return n;
    }

    return -EINVAL;
}

ir_attr(ir_mouse_pointer_threshold);

static ssize_t ir_mouse_pointer_timer_show(struct kobject * kobj, struct kobj_attribute * attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%d\n", ir_mouse_pointer_timer);
}

static ssize_t ir_mouse_pointer_timer_store(struct kobject * kobj, struct kobj_attribute * attr, const char *buf, size_t n)
{
    unsigned int val = 0;

    if (kstrtouint(buf, 10, &val) == 0) {
        ir_mouse_pointer_timer = val;

        return n;
    }

    return -EINVAL;
}

ir_attr(ir_mouse_pointer_timer);

static ssize_t ir_mouse_pointer_step_show(struct kobject * kobj, struct kobj_attribute * attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%d\n", ir_mouse_pointer_step);
}

static ssize_t ir_mouse_pointer_step_store(struct kobject * kobj, struct kobj_attribute * attr, const char *buf, size_t n)
{
    unsigned int val = 0;

    if (kstrtouint(buf, 10, &val) == 0) {
        ir_mouse_pointer_step = val;

        return n;
    }

    return -EINVAL;
}

ir_attr(ir_mouse_pointer_step);
#endif

//////////////////////////test/////////////////////////
#if IR_INPUT_DEVICE_TEST
static ssize_t get_keycode_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return 0;
}

static ssize_t get_keycode_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
    unsigned int val = 0;
    struct input_keymap_entry entry;
    if(strcmp(attr->attr.name, "get_keycode") == 0)
    {
        if (sscanf(buf, "%x", &val) == 1) {
            entry.len = sizeof(val);
            memcpy(entry.scancode,&val,sizeof(entry.scancode));
            _venus_ir_getkeycode(NULL, &entry);
            IR_ALERT("scancode=0x%x keycode=0x%x\n", val, entry.keycode);
            return n;
        }
    }

    return -EINVAL;
}
ir_attr(get_keycode);

static ssize_t set_keycode_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return 0;
}

static ssize_t set_keycode_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
    unsigned int scancode   = 0;
    unsigned int keycode    = 0;
    unsigned int oldkeycode = 0;
    struct input_keymap_entry entry;
    struct input_dev dev;

    if(strcmp(attr->attr.name, "set_keycode") == 0)
    {
        if (sscanf(buf, "%x %x", &scancode,&keycode) == 2)
        {
            entry.keycode   = keycode;
            entry.len       = sizeof(scancode);
            memcpy(entry.scancode,&scancode,sizeof(scancode));

            IR_ALERT("scancode=0x%x keycode=0x%x\n", scancode, entry.keycode);
            _venus_ir_setkeycode(&dev, &entry, &oldkeycode);
            memcpy(&scancode,entry.scancode,sizeof(scancode));
            IR_ALERT("scancode=0x%x keycode=0x%x\n",scancode, entry.keycode);
            return n;
        }
    }

    return -EINVAL;
}
ir_attr(set_keycode);
#endif
////////////////////////////////////////////////////////

static ssize_t ir_send_key_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "echo keydw/keyup=xxx xxx means key value\n");
}


static ssize_t ir_send_key_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
    unsigned int val = 0;
    char action[6] = {0};
    if(strcmp(attr->attr.name, "ir_send_key") == 0)
    {
        if (sscanf(buf, "%5s=%d,", action, &val) == 2) {
            if(strcmp(action,"keydw") == 0){
                ir_gen_keydown_event(val);
                IR_ALERT("send key down:%d\n",val);
            }else {
                ir_gen_keyup_event(val);
                IR_ALERT("send key up:%d\n",val);
            }
        }
        return n;
    }
    return -EINVAL;
}
ir_attr(ir_send_key);

static struct attribute * ir_attrs[] = {
    &ir_protocol_attr.attr,
    &ir_protocol_main0_attr.attr,
    &ir_protocol_attr_attr.attr,
    &ir_protocol_attr_main0_attr.attr,
    &ir_repeat_interval_attr.attr,
    &ir_debounce_attr.attr,
    &ir_send_key_attr.attr,
#ifdef CONFIG_DELAY_FIRST_REPEAT
    &ir_first_repeat_attr.attr,
#endif
    &ir_decode_debug_attr.attr,
    &ir_irda_key_decode_on_off_attr.attr,
    &ir_scancode_filter_attr.attr,
    &ir_remote_name_attr.attr,
#ifdef CONFIG_RTK_KDRV_INPUT_DEVICE
    &ir_gen_keycode_attr.attr,
    &ir_poll_interval_attr.attr,
    &ir_poll_settings_attr.attr,
    &ir_keycode_to_scancode_attr.attr,
    &ir_mute_irda_keyevent_attr.attr,
    &ir_to_mouse_attr.attr,
    &ir_repeat_event_attr.attr,
    &ir_rawkey_event_attr.attr,
    &ir_report_mode_attr.attr,
    &ir_is_upload_more_key_attr.attr,
    &ir_set_intercept_key_attr.attr,
#endif
#ifdef CONFIG_RTK_KDRV_INPUT_DEVICE_MOUSE
    &ir_mouse_pointer_speed_attr.attr,
    &ir_mouse_pointer_threshold_attr.attr,
    &ir_mouse_pointer_timer_attr.attr,
    &ir_mouse_pointer_step_attr.attr,
#endif
#if IR_INPUT_DEVICE_TEST
    &get_keycode_attr.attr,
    &set_keycode_attr.attr,
#endif
    NULL,
};

struct attribute_group ir_attr_group = {
    .attrs = ir_attrs,
};
