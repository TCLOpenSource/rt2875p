/*Copyright (C) 2019-2025 Realtek Semiconductor Corporation.*/
#ifndef __RTK_IRTX_REG_H__
#define __RTK_IRTX_REG_H__

#include <rbus/misc_irtx_reg.h>
#include <rbus/sys_reg_reg.h>
#include <rbus/pinmux_reg.h>
#include "rtk_irtx_priv.h"

#define IRTX_BUS_CLK_REQ           250000000
#define IRTX_PWM_CLK_PRE_DIV       100
#define IRTX_HW_FIFO_LEN           256
#define EACH_TIMING_INFO_MAX_LEN    0x7FFF


#define IRQ_IRTX                     IRQ_MISC

#define IRTX_PINMUX_REG                  (PINMUX_ST_GPIO_ST_CFG_7_reg)
#define IRTX_PINMUX_MASK            (PINMUX_ST_GPIO_ST_CFG_7_st_gpio_39_ps_mask)
#define IRTX_PINMUX_DATA            (PINMUX_ST_GPIO_ST_CFG_7_st_gpio_39_ps_mask&((0x8)<<PINMUX_ST_GPIO_ST_CFG_7_st_gpio_39_ps_shift))


#define IRTX_INT_EN                  SYS_REG_INT_CTRL_SCPU_reg
#define IRTX_INT_EN_MASK         SYS_REG_INT_CTRL_SCPU_standby_iso_misc_int_scpu_routing_en_mask
#define IRTX_INT_EN_VAL              SYS_REG_INT_CTRL_SCPU_standby_iso_misc_int_scpu_routing_en_mask
#define IRTX_INT_EN_WRITE_DATA       SYS_REG_INT_CTRL_SCPU_write_data_mask


#define IRTX_EN_RST_REG        MISC_IRTX_IRTX_EN_RST_reg
#define IRTX_CLKDIV_REG      MISC_IRTX_IRTX_CLKDIV_reg
#define IRTX_START_REG       MISC_IRTX_IRTX_START_reg
#define IRTX_INT_REG                MISC_IRTX_IRTX_INT_reg
#define IRTX_INT_EN_REG         MISC_IRTX_IRTX_INT_EN_reg
#define IRTX_TI_CTRL_REG       MISC_IRTX_TI_CTRL_reg
#define IRTX_PWM_CTRL_REG   MISC_IRTX_PWM_CTRL_reg
#define IRTX_PWM_CFG_REG     MISC_IRTX_PWM_CFG_reg

#define IRTX_EN_SHIFT            MISC_IRTX_IRTX_EN_RST_irtx_en_shift
#define IRTX_EN_MASK            MISC_IRTX_IRTX_EN_RST_irtx_en_mask
#define IRTX_EN(x)                   (((x) & 0x1) << IRTX_EN_SHIFT)

#define IRTX_RST_SHIFT            MISC_IRTX_IRTX_EN_RST_irtx_rst_shift
#define IRTX_RST_MASK            MISC_IRTX_IRTX_EN_RST_irtx_rst_mask
#define IRTX_RST(x)                   (((x) & 0x1) << IRTX_RST_SHIFT)


#define IRTX_CLKDIV_SHIFT            MISC_IRTX_IRTX_CLKDIV_irtx_clkdiv_shift
#define IRTX_CLKDIV_MASK            MISC_IRTX_IRTX_CLKDIV_irtx_clkdiv_mask
#define IRTX_CLKDIV(x)                   (((x) & 0xFFFF) << IRTX_CLKDIV_SHIFT)

#define IRTX_PWM_EN_SHIFT            MISC_IRTX_PWM_CTRL_irtx_pwm_en_shift
#define IRTX_PWM_EN_MASK            MISC_IRTX_PWM_CTRL_irtx_pwm_en_mask
#define IRTX_PWM_EN(x)                   (((x) & 0x01) << IRTX_PWM_EN_SHIFT)

#define IRTX_PWM_CLKSRC_DIV_SHIFT            MISC_IRTX_PWM_CFG_irtx_pwm_clksrc_div_shift
#define IRTX_PWM_CLKSRC_DIV_MASK            MISC_IRTX_PWM_CFG_irtx_pwm_clksrc_div_mask
#define IRTX_PWM_CLKSRC_DIV(x)                   (((x) & 0xF) << IRTX_PWM_CLKSRC_DIV_SHIFT)

#define IRTX_PWM_CLK_DUTY_SHIFT            MISC_IRTX_PWM_CFG_irtx_pwm_clk_duty_shift
#define IRTX_PWM_CLK_DUTY_MASK            MISC_IRTX_PWM_CFG_irtx_pwm_clk_duty_mask
#define IRTX_PWM_CLK_DUTY(x)                   ((((x) - 1) & 0xFF) << IRTX_PWM_CLK_DUTY_SHIFT)

#define IRTX_PWM_CLK_DIV_SHIFT            MISC_IRTX_PWM_CFG_irtx_pwm_clk_div_shift
#define IRTX_PWM_CLK_DIV_MASK            MISC_IRTX_PWM_CFG_irtx_pwm_clk_div_mask
#define IRTX_PWM_CLK_DIV(x)                   ((((x) - 1) & 0xFF) << IRTX_PWM_CLK_DIV_SHIFT)

#define IRTX_START_SHIFT            MISC_IRTX_IRTX_START_irtx_start_shift
#define IRTX_START_MASK            MISC_IRTX_IRTX_START_irtx_start_mask
#define IRTX_START(x)                   (((x) & 0x1) << IRTX_START_SHIFT)

#define IRTX_LEN_SHIFT            MISC_IRTX_IRTX_START_irtx_len_shift
#define IRTX_LEN_MASK            MISC_IRTX_IRTX_START_irtx_len_mask
#define IRTX_LEN(x)                   ((((x) - 1) & 0xFF) << IRTX_LEN_SHIFT)

#define IRTX_DONE_INT_SHIFT            MISC_IRTX_IRTX_INT_irtx_done_int_shift
#define IRTX_DONE_INT_MASK            MISC_IRTX_IRTX_INT_irtx_done_int_mask

#define IRTX_DONE_INT_EN_SHIFT            MISC_IRTX_IRTX_INT_EN_irtx_done_int_en_shift
#define IRTX_DONE_INT_EN_MASK            MISC_IRTX_IRTX_INT_EN_irtx_done_int_en_mask
#define IRTX_DONE_INT_EN(x)                   (((x) & 0x1) << IRTX_DONE_INT_EN_SHIFT)


#define IRTX_TI_BUF_WDATA_LENGTH_SHIFT            17
#define IRTX_TI_BUF_WDATA_LENGTH_MASK            0xFFFE0000
#define IRTX_TI_BUF_WDATA_LENGTH(x)                   (((x) & 0x7FFF) << IRTX_TI_BUF_WDATA_LENGTH_SHIFT)

#define IRTX_TI_BUF_WDATA_LEVEL_SHIFT            16
#define IRTX_TI_BUF_WDATA_LEVEL_MASK            0x00010000
#define IRTX_TI_BUF_WDATA_LEVEL(x)                   (((x) & 0x01) << IRTX_TI_BUF_WDATA_LEVEL_SHIFT)


#define IRTX_TI_BUF_IDX_SHIFT            MISC_IRTX_TI_CTRL_irtx_ti_buf_idx_shift
#define IRTX_TI_BUF_IDX_MASK            MISC_IRTX_TI_CTRL_irtx_ti_buf_idx_mask
#define IRTX_TI_BUF_IDX(x)                   (((x) & 0xFF) << IRTX_TI_BUF_IDX_SHIFT)

#define IRTX_TI_BUF_WR_SHIFT            MISC_IRTX_TI_CTRL_irtx_ti_buf_wr_shift
#define IRTX_TI_BUF_WR_MASK            MISC_IRTX_TI_CTRL_irtx_ti_buf_wr_mask
#define IRTX_TI_BUF_WR(x)                   (((x) & 0x01) << IRTX_TI_BUF_WR_SHIFT)


#define WRITE_REG(addr, val)      rtd_outl((addr), (val))
#define READ_REG(addr)            rtd_inl((addr))

#endif // __CEC_RTK_REG_H__
