#ifndef __RTK_SB2_H__
#define __RTK_SB2_H__

#include "rtk_hw_monitor.h"

#if (defined CONFIG_ARCH_RTK2885P)
#define SB2_DBG_ACPU_MONITOR_SUPPORT
#endif

#define SB2_BLOCK_SET_NUM 4
//
#define SB2_ENV_MEM_SET             "sb2_set="
#define SB2_ENV_MEM_BIT_SET      "sb2_bit_set="

//operation label
#define OPERA_MONITOR   0
#define OPERA_CLEAR     1
#define OPERA_HACKON    2 // hack cmd to enable all CPUs' Interrupt status
#define OPERA_HACKOFF   3 // hack cmd to restore all CPUs' Interrupt status
#define OPERA_BLOCK     4
#define OPERA_NONBLOCK  5
#define OPERA_TRAP_TOGGLE  6

//cpu enable/disable label
enum SB2_DBG3_CPU_TYPE{
    CPU_TYPE_ALL = 0,
#ifdef SB2_DBG_ACPU_MONITOR_SUPPORT
    CPU_TYPE_ACPU, 
#endif
    CPU_TYPE_VCPU,
    CPU_TYPE_SCPU,
#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
    CPU_TYPE_VCPU2,
#endif
    CPU_TYPE_MAX,
};


//rw label
#define RW_FLAG_RW      HWM_RW
#define RW_FLAG_RO      HWM_R
#define RW_FLAG_WO      HWM_W
#define RW_FLAG_DFT     RW_FLAG_RW
//str label
#define STR_FLAG_YES    0
#define STR_FLAG_NO     1
#define STR_FLAG_DFT    STR_FLAG_YES
//match label
#define MATCH_FLAG_YES    0
#define MATCH_FLAG_NO     1
//option param offset
#define OPT_RW          0
#define OPT_STR         1
//debug register offset
#define STRREG_INV_INTEN_reg    0
#define STRREG_INV_INTSTAT_reg  1
#define STRREG_DEBUG_REG_reg    2
#define STRREG_DBG_INT_reg      3
#define STRREG_EMCU_TIME_OUT_reg    4

typedef struct sb2dbg_bitmask {
        unsigned int bit_mask;
        unsigned int raw_val;
        unsigned int reg_addr;
} sb2dbg_bitmask;

typedef struct sb2_dbg_param {
        int set_id; // which set to be use,range 1-8
        unsigned int cpu_flag[7]; // which cpu type to be set,0:all 1:a v:2 s:3 v2:4 k:5
        unsigned int start; // dbg start address,phy addr
        unsigned int end; // dbg end address,phy addr
        unsigned char operation; // the main operation of sb2 cmd,0:monitor 1:clear
        unsigned char rw;  // read & write monitor flag
        unsigned char str; // STR support flag
        unsigned int bit_mask; // bit mask param
} sb2_dbg_param;

//need fix=================================
typedef struct sb2_dbg_bit_param {
        int set_id; // which set to be use,range 1-4
        unsigned int cpu_flag[5]; // which cpu type to be set,0:all 1:a v:2 s:3 v2:4
        unsigned int addr; // dbg  address,phy addr
        unsigned char operation; // the main operation of sb2 cmd,0:monitor 1:clear
        unsigned int dbg3_1; // monitor bit
        unsigned int dbg3_2; // monitor data
        unsigned char str; // STR support flag
        unsigned char match; // match support flag
} sb2_dbg_bit_param;

typedef struct sb2dbg3_status_store {
        unsigned int dbg3_0;
        unsigned int dbg3_1;
        unsigned int dbg3_2;
        unsigned int dbg3_ctrl;
} sb2dbg3_status_store;
//=======================================

typedef struct sb2dbg_status_store {
        unsigned int start_addr;
        unsigned int end_addr;
        unsigned int ctrl;
} sb2dbg_status_store;

typedef struct sb2block_status_store {
        unsigned int ctrl;
        unsigned int start_addr[SB2_BLOCK_SET_NUM];
        unsigned int end_addr[SB2_BLOCK_SET_NUM];
} sb2block_status_store;

#endif /*__RTK_SB2_H__ */

