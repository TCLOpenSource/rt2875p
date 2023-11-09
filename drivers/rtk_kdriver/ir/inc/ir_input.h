#ifndef __IR_INPUT_H
#define __IR_INPUT_H

#include <linux/input.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <rtk_kdriver/input-event-custom-codes.h>
#include "ir_debug.h"

#define LIBRA_MS_CUSTOMER_CODE      0x08F7
#define JAECS_T118_CUSTOMER_CODE    0xFC03
#define RTK_MK3_CUSTOMER_CODE       0xB649
#define YK_70LB_CUSTOMER_CODE       0x0E
#define RTK_MK4_CUSTOMER_CODE       0x6B86
#define RTK_MK5_CUSTOMER_CODE       0x7F80
#define NETG_MS_CUSTOMER_CODE       0x18
#define YK_54LU_CUSTOMER_CODE       0xf1f1
#define YK_54LU_CUSTOMER_CODE_EX       0xf5f5
#define YK_76J_CUSTOMER_CODE        0xf1f1
#define KONKA_CUSTOMER_CODE     0x0200 //wangzhh add for konka 20130407
#define RTK_MK5_2_CUSTOMER_CODE     0xfb04
#define RTK_ANDROID_OTT_CUSTOMER_CODE   0xfd02
#define TV005_CUSTOMER_CODE1        0xBF40
#define TV005_CUSTOMER_CODE2        0xBE40
#define TV005_CUSTOMER_CODE3        0xBC40
#define YK_54LU_SHUTTLE_CUSTOMER_CODE   0x0002
#define YK_76J_SHUTTLE_CUSTOMER_CODE    0x0002
#define TV015_IR_AOC_CUSTOMER_CODE  0xbd00
#define TV015_IR_FAC_CUSTOMER_CODE  0x0820
#define TV015_IR_FAC_TEST_CODE1     0x926d0820
#define TV015_IR_FAC_TEST_CODE2     0x6d92f7df
#define TV015_IR_RC6_CUSTOMER_CODE  0x00
#define TV036_10D_CUSTOMER_CODE        0x7F00
#define TV030_JP_CUSTOMER_CODE          0xB9162C
#define TV030_TCL_NEC_CUSTOMER_CODE     0x9966
#define TV030_KT1752_CUSTOMER_CODE 0x2207
#define TV030_JP_FACTORY_CUSTOMER_CODE  0xB00000 //B0F708 B0F906 B0FA05 B0F40B B0F20D B0F30C B0F708
#define RTK_HISENSE_CUSTOMER_CODE  0xBF00 //Add for Hisense

struct venus_ir_data {
    unsigned int irq;
    struct input_dev *input_dev;
    struct input_dev *mouse_dev;
    u32 scancode;
    u32 keycode;
    u32 prev_keycode;
    u32 dataRepeat;
    u32 mouseRepeat;
};

//#if defined(CONFIG_RTK_KDRV_TV002_IR)
struct sony_key {
    u32 custcode;
    u32 datacode;
    u32 keycode;
};

struct sony_key_table {
    struct sony_key *keys;
    int size;
};
//#endif

struct venus_key {
    u32 scancode;
    u32 keycode;
};

struct venus_key_table {
    struct venus_key *keys;
    int size;
};

struct user_key_data {
    struct venus_key_table key_table;
    unsigned int max_size;
};

/*new ir user key table struct*/
typedef struct {
	unsigned short protocol;
	unsigned short keycode;
	unsigned int scancode;
}IR_USER_KEY;

typedef struct {
    IR_USER_KEY *keys;
    unsigned int size;
    unsigned int is_init;
    rwlock_t lock;
}IR_USER_KEY_TABLE;

int venus_ir_input_init(void);
void venus_ir_input_cleanup(void);
int venus_ir_input_set_input_dev_name(const char *name);
void venus_ir_input_report_key(uint32_t repkey);
void schedule_venus_ir_keyup_handler(uint32_t which);
bool venus_ir_input_set_customcode(int customer);
int venus_ir_input_get_customcode(void);
enum hrtimer_restart venus_ir_input_report_end(struct hrtimer *timer);
u32 venus_ir_scancode_to_keycode(uint32_t value);
u32 venus_ir_keycode_to_scancode(uint32_t value, unsigned int *scancode_arrays, unsigned int array_len);
uint32_t venus_ir_keylist_to_scanlist(uint32_t *keylist, uint32_t keycnt, uint32_t *scanlist, uint32_t scancnt);
void venus_ir_gen_keyupdown_event(uint32_t keycode);

int _venus_ir_getkeycode(struct input_dev *pdev, struct input_keymap_entry *pentry);
int _venus_ir_setkeycode(struct input_dev *dev, const struct input_keymap_entry *pentry, unsigned int *old_keycode);

/**********************QS Releated**********************************/
void set_QS_handle_key(bool enable);
bool get_QS_handle_key(void);
int ir_input_register_QS_key_event_notifier(struct notifier_block *nb);
void ir_input_unregister_QS_key_event_notifier(struct notifier_block *nb);

#define MAKE_QS_EVENT(type, code, value)        (((type) & 0xF) | (((code) & 0xFFF) << 4) |(((value) & 0xFFFF) << 16))
#define QS_GET_EV_TYPE(ev)   ((ev) & 0xF)  //EV_KEY = 1
#define QS_GET_EV_CODE(ev)   (((ev) >> 4) & 0xFFF)  //key
#define QS_GET_EV_VALUE(ev)   (((ev) >> 16) & 0xFFFF)  //value

#endif


