#ifndef __IR_TABLE_H__
#define __IR_TABLE_H__
#include "ir_input.h"
/***********************TCL****************************/
extern struct  venus_key_table tcl_jp_tv_key_table;/*AEHA*/
extern struct  venus_key_table tcl_tv_key_table;/*RCA*/
extern struct  venus_key_table tcl_nec_tv_key_table;
#if defined(CONFIG_RTK_KDRV_TV030_IR )
extern struct venus_key_table tcl_tv_dvd_key_table;
void set_factory_key_table(struct input_dev *input_dev);
unsigned int change_bits_order(unsigned int value, unsigned int bits_len);
#endif
extern struct  venus_key_table tcl_kt1752_key_table;/*NEC*/

/***********************Sony****************************/
extern struct  sony_key_table tv002_key_table;  /*Sony*/

/***********************TV001****************************/
extern struct  venus_key_table rtk_mk5_2_tv_key_table; /*NEC*/

/***********************Hisense****************************/
extern struct  venus_key_table rtk_hisense_tv_key_table; /*NEC*/

/************************Default*****************************/
extern struct  venus_key_table rtk_mk5_tv_key_table; /*NEC*/

/***********************TV036****************************/
extern struct  venus_key_table Hikeen_10D_tv_key_table; /*NEC*/

/***********************TV015****************************/
#if defined(CONFIG_RTK_KDRV_TV015_IR)
extern struct  venus_key_table tv015_ir_rc6_tv_key_table;/*NEC*/
extern struct  venus_key_table tv015_ir_fac_tv_key_table;/*NEC*/
extern struct  venus_key_table tv015_ir_aoc_tv_key_table;/*NEC*/
#endif	

/***********************TV013****************************/
#if defined(CONFIG_RTK_KDRV_HAIER_IR)
extern struct  venus_key_table haier_key_tv_table; /*NEC*/
extern struct  venus_key_table haier_key_fac_table;/*NEC*/
extern struct  venus_key_table haier_key_cvt_table;/*NEC*/
extern struct  venus_key_table haier_key_mtc_table;/*NEC*/
#endif
/***********************TV035****************************/
#if defined(CONFIG_RTK_KDRV_TV035_IR)
extern struct  venus_key_table konka_tv_key_table; /*KONKA*/
#endif

/***********************TV005****************************/
#if defined(CONFIG_RTK_KDRV_TV005_IR)
extern struct  venus_key_table tv005_key_table;
#endif

/***********************TV010****************************/
#if defined(CONFIG_RTK_KDRV_YK_54LU)
extern struct  venus_key_table yk_54lu_key_table;
extern struct  venus_key_table yk_54lu_key_table_ex;
#endif
#if defined(CONFIG_RTK_KDRV_YK_76J)
struct  venus_key_table yk_76j_key_table;
#endif

/***********************user table****************************/
extern IR_USER_KEY_TABLE rtk_user_custom_default_table;
extern IR_USER_KEY_TABLE g_ir_user_key_table;
extern int venus_ir_user_key_table_init(IR_USER_KEY *keys, int keys_num);
void venus_ir_user_key_table_uninit(void);
unsigned short venus_ir_user_key_table_getkeycode_by_scancode(unsigned int scancode, 
                                                                                    unsigned int *p_index, unsigned int *p_protocol);
unsigned short venus_ir_user_key_table_getkeycode_by_index(unsigned int index, 
                                                                                    unsigned int *p_scancode, unsigned int *p_protocol);
int venus_ir_user_key_table_setkeycode(unsigned int scancode, 
                                                                                    unsigned int keycode, unsigned int protocol, 
                                                                                    unsigned int *old_keycode);
int venus_ir_user_key_table_check_keycode_exist(unsigned int keycode);
unsigned int venus_ir_user_key_table_find_scancode(unsigned short protocol, unsigned short keycode);
unsigned int venus_ir_user_key_table_find_scancodes(unsigned short protocol, unsigned short keycode,
                                        unsigned int *scancode_arrays, unsigned int array_len);

#if defined(CONFIG_RTK_KDRV_ANDROID_OTT)
extern struct  venus_key_table rtk_android_ott_stb_key_table;
#endif


#define SET_INPUT_DEV_KEY_BITS(key_table, input_dev) { \
	int i = 0; \
	for(i = 0; i < (key_table)->size; i++) \
		set_bit((key_table)->keys[i].keycode, (input_dev)->keybit); \
    }


#define FIND_AND_REPORT_SCANCODE_TO_KEYCODE(key_table_name, key_table, data, scancode) { \
    int i = 0; \
    for (i = 0; i < (key_table)->size; i++) { \
            if ((scancode) == (key_table)->keys[i].scancode) { \
                (data)->prev_keycode = (data)->keycode; \
                (data)->keycode = (key_table)->keys[i].keycode; \
                IR_DBG("Venus IR: %s report key 0x%x %d\n",  \
                	(key_table_name != NULL) ? key_table_name : "", (scancode), (data)->keycode); \
                return (data)->keycode; \
            } \
        } \
}

#define FIND_AND_REPORT_SCANCODE_TO_KEYCODE_2(key_table_name, key_table, data, custom_code, scancode) { \
    int i = 0; \
    for (i = 0; i < (key_table)->size; i++) { \
             if ((custom_code) == (key_table)->keys[i].custcode  && (scancode) == (key_table)->keys[i].datacode) { \
                (data)->prev_keycode = (data)->keycode; \
                (data)->keycode = (key_table)->keys[i].keycode; \
                IR_DBG("Venus IR: %s report key 0x%x %d\n",  \
                	(key_table_name != NULL) ? key_table_name : "", (scancode), (data)->keycode); \
                return (data)->keycode; \
            } \
        } \
}


#define FIND_AND_REPORT_KEYCODE_TO_SCANCODE(key_table_name, key_table, value) { \
    int i = 0; \
    for (i = 0; i < (key_table)->size; i++) { \
            if ((value) == (key_table)->keys[i].keycode) { \
		scancode = (key_table)->keys[i].scancode; \
		IR_DBG("%s keycode %d to scancode 0x%x\n", (key_table_name), (value), scancode); \
                break; \
            } \
    } \
}

#define FIND_AND_REPORT_KEYCODE_TO_SCANCODE_2(key_table_name, key_table, value) { \
    int i = 0; \
    for (i = 0; i < (key_table)->size; i++) { \
            if ((value) == (key_table)->keys[i].keycode) { \
		scancode = (key_table)->keys[i].datacode; \
		custcode = (key_table)->keys[i].custcode; \
		IR_DBG("%s keycode %d to scancode 0x%x custcode 0x%x\n", (key_table_name), (value), scancode, custcode); \
                break; \
            } \
    } \
}

#define FIND_AND_REPORT_KEYCODE_TO_SCANCODE_3(key_table_name, key_table, value, proto) { \
    int i = 0; \
    for (i = 0; i < (key_table)->size; i++) { \
            if ((value) == (key_table)->keys[i].keycode && (proto) == (key_table)->keys[i].protocol) { \
		scancode = (key_table)->keys[i].scancode; \
		IR_DBG("%s keycode %d to scancode 0x%x\n", (key_table_name), (value), scancode); \
                break; \
            } \
    } \
}


#endif
