#ifndef __RTK_CW_REG_H__
#define __RTK_CW_REG_H__
#include <rbus/rbus_types.h>
#include <rbus/tp_reg.h>


#define  CW_KEY_INFO_VADDR                                      (TP_TP_KEY_INFO_0_reg)
#define  CW_KEY_CTRL_VADDR                                      (TP_TP_KEY_CTRL_reg)
#define TPK_CW_LENGTH           8

typedef union {
    RBus_UInt32 regValue;
    struct {
        RBus_UInt32  idx: 7;
        RBus_UInt32  r_w: 1;
        RBus_UInt32  res1: 24;
    };
} cw_key_ctrl_RBUS;

#define READ_REG32(addr)                rtd_inl(addr)
#define WRITE_REG32(addr, val)          rtd_outl(addr,val)

#endif /* #ifndef __RTK_CW_REG_H__ */
