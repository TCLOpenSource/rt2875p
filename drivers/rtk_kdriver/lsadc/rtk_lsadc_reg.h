/**
 **
 */

#ifndef _RTK_LSADC_REG_H_
#define _RTK_LSADC_REG_H_

#include <rbus/rbus_types.h>
#include "rtk_lsadc_reg_def.h"
#include <rbus/lsadc_reg.h>
#include <rtk_kdriver/rtk_crt.h>

typedef struct _rtk_lsdac_reg_map_t {
    unsigned int padCtrl;
    unsigned int lsadc_ctrl;
    unsigned int status;
    unsigned int analog_ctrl;
    unsigned int padCompare0;
    unsigned int padCompare1;
    unsigned int padCompare2;
    unsigned int padCompare3;
    unsigned int padCompare4;
    unsigned int padCompare5;
    unsigned int compareStatus;
} rtk_lsdac_reg_map ;

typedef struct _rtk_lsadc_phy_t {
    const rtk_lsdac_reg_map *p_reg_map;
} rtk_lsadc_phy;

static const rtk_lsdac_reg_map lsadc_0_reg = {
    .padCtrl      =LSADC_ST_pad0_reg,
    .lsadc_ctrl   = LSADC_ST_LSADC_ctrl_reg,
    .status       = LSADC_ST_LSADC_status_reg,
    .analog_ctrl  = LSADC_ST_LSADC_analog_ctrl_reg,
};

static const rtk_lsdac_reg_map lsadc_1_reg = {
    .padCtrl      = LSADC_ST_pad1_reg,
    .lsadc_ctrl   = LSADC_ST_LSADC_ctrl_reg,
    .status       = LSADC_ST_LSADC_status_reg,
    .analog_ctrl  = LSADC_ST_LSADC_analog_ctrl_reg,
};

static const rtk_lsdac_reg_map lsadc_2_reg = {
    .padCtrl      = LSADC_ST_pad2_reg,
    .lsadc_ctrl   = LSADC_ST_LSADC_ctrl_reg,
    .status       = LSADC_ST_LSADC_status_reg,
    .analog_ctrl  = LSADC_ST_LSADC_analog_ctrl_reg,
};

static const rtk_lsdac_reg_map lsadc_3_reg = {
    .padCtrl      = LSADC_ST_pad3_reg,
    .lsadc_ctrl   = LSADC_ST_LSADC_ctrl_reg,
    .status       = LSADC_ST_LSADC_status_reg,
    .analog_ctrl  = LSADC_ST_LSADC_analog_ctrl_reg,
};



static const rtk_lsdac_reg_map lsadc_4_reg = {
    .padCtrl      = LSADC_ST_pad4_reg,
    .lsadc_ctrl   = LSADC_ST_LSADC_ctrl_reg,
    .status       = LSADC_ST_LSADC_status_reg,
    .analog_ctrl  = LSADC_ST_LSADC_analog_ctrl_reg,
};



static const rtk_lsdac_reg_map lsadc_5_reg = {
    .padCtrl      = LSADC_ST_pad5_reg,
    .lsadc_ctrl   = LSADC_ST_LSADC_ctrl_reg,
    .status       = LSADC_ST_LSADC_status_reg,
    .analog_ctrl  = LSADC_ST_LSADC_analog_ctrl_reg,
    .padCompare0  = LSADC_LSADC_pad1_level_set0_reg,
    .padCompare1  = LSADC_LSADC_pad1_level_set1_reg,
    .padCompare2  = LSADC_LSADC_pad1_level_set2_reg,
    .padCompare3  = LSADC_LSADC_pad1_level_set3_reg,
    .padCompare4  = LSADC_LSADC_pad1_level_set4_reg,
    .padCompare5  = LSADC_LSADC_pad1_level_set5_reg,
    .compareStatus = LSADC_LSADC_INT_pad1_reg,
};


static const rtk_lsdac_reg_map lsadc_6_reg = {
    .padCtrl      = LSADC_ST_pad6_reg,
    .lsadc_ctrl   = LSADC_ST_LSADC_ctrl_reg,
    .status       = LSADC_ST_LSADC_status_reg,
    .analog_ctrl  = LSADC_ST_LSADC_analog_ctrl_reg,
    .padCompare0  = LSADC_LSADC_pad2_level_set0_reg,
    .padCompare1  = LSADC_LSADC_pad2_level_set1_reg,
    .padCompare2  = LSADC_LSADC_pad2_level_set2_reg,
    .padCompare3  = LSADC_LSADC_pad2_level_set3_reg,
    .padCompare4  = LSADC_LSADC_pad2_level_set4_reg,
    .padCompare5  = LSADC_LSADC_pad2_level_set5_reg,
    .compareStatus = LSADC_LSADC_INT_pad2_reg,
};


static const rtk_lsdac_reg_map lsadc_7_reg = {
    .padCtrl      = LSADC_ST_pad7_reg,
    .lsadc_ctrl   = LSADC_ST_LSADC_ctrl_reg,
    .status       = LSADC_ST_LSADC_status_reg,
    .analog_ctrl  = LSADC_ST_LSADC_analog_ctrl_reg,
    .padCompare0  = LSADC_LSADC_pad3_level_set0_reg,
    .padCompare1  = LSADC_LSADC_pad3_level_set1_reg,
    .padCompare2  = LSADC_LSADC_pad3_level_set2_reg,
    .padCompare3  = LSADC_LSADC_pad3_level_set3_reg,
    .padCompare4  = LSADC_LSADC_pad3_level_set4_reg,
    .padCompare5  = LSADC_LSADC_pad3_level_set5_reg,
    .compareStatus = LSADC_LSADC_INT_pad3_reg,
};

static const rtk_lsdac_reg_map lsadc_8_reg = {
    .padCtrl      = LSADC_ST_pad8_reg,
    .lsadc_ctrl   = LSADC_ST_LSADC_ctrl_reg,
    .status       = LSADC_ST_LSADC_status_reg,
    .analog_ctrl  = LSADC_ST_LSADC_analog_ctrl_reg,
};

#if (defined(CONFIG_ARCH_RTK2851A) || defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2851F))

#else
static const rtk_lsdac_reg_map lsadc_9_reg = {
    .padCtrl      = LSADC_ST_pad9_reg,
    .lsadc_ctrl   = LSADC_ST_LSADC_ctrl_reg,
    .status       = LSADC_ST_LSADC_status_reg,
    .analog_ctrl  = LSADC_ST_LSADC_analog_ctrl_reg,
};

static const rtk_lsdac_reg_map lsadc_10_reg = {
    .padCtrl      = LSADC_ST_pad10_reg,
    .lsadc_ctrl   = LSADC_ST_LSADC_ctrl_reg,
    .status       = LSADC_ST_LSADC_status_reg,
    .analog_ctrl  = LSADC_ST_LSADC_analog_ctrl_reg,
};
#endif

static const rtk_lsadc_phy lsadcArray[] = {

    {&lsadc_0_reg},
    {&lsadc_1_reg},
    {&lsadc_2_reg},
    {&lsadc_3_reg},
    {&lsadc_4_reg},
    {&lsadc_5_reg},
    {&lsadc_6_reg},
    {&lsadc_7_reg},
    {&lsadc_8_reg},
#if (defined(CONFIG_ARCH_RTK2851A) || defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2851F))
#else
    {&lsadc_9_reg},
    {&lsadc_10_reg},
#endif
};


/*
typedef enum {
    PCB_LSADC_TYPE_VOLTAGE,
    PCB_LSADC_TYPE_CURRENT,
    PCB_LSADC_TYPE_UNDEF,
} PCB_LSADC_TYPE_T;
*/


#endif


