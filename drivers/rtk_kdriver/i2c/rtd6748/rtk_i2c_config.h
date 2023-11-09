#ifndef __RTK_I2C_CONFIG_H__
#define __RTK_I2C_CONFIG_H__

#include <rbus/iso_misc_off_reg.h>
#include <rbus/misc_reg.h>
#include <rbus/stb_reg.h>
#include <rbus/sys_reg_reg.h>
#include <rbus/pll27x_reg_reg.h>
#include <rbus/pll_reg_reg.h>
#include <rbus/misc_i2c_reg.h>
#include <rbus/misc_i2c4_reg.h>
#include <rbus/iso_i2c_reg.h>
#include "rtk_i2c_priv.h"

#define RTK_I2C_ISO_MIS_OFF_SCPU_INT_EN_shift   SYS_REG_INT_CTRL_SCPU_standby_iso_misc_off_int_scpu_routing_en_shift

#define RTK_ISO_I2C0_SDA_OUTPUT_DELAY_databit_mask  \
    (ISO_I2C_I2C0_SDA_OUTPUT_DELAY_CTRL_databitdelayenable_mask | ISO_I2C_I2C0_SDA_OUTPUT_DELAY_CTRL_databitdelaysel_mask)
#define RTK_MIS_I2C1_SDA_OUTPUT_DELAY_databit_mask  \
    (MISC_I2C_I2C1_SDA_OUTPUT_DELAY_CTRL_databitdelayenable_mask | MISC_I2C_I2C1_SDA_OUTPUT_DELAY_CTRL_databitdelaysel_mask)
#define RTK_MIS_I2C2_SDA_OUTPUT_DELAY_databit_mask  \
    (MISC_I2C_I2C2_SDA_OUTPUT_DELAY_CTRL_databitdelayenable_mask | MISC_I2C_I2C2_SDA_OUTPUT_DELAY_CTRL_databitdelaysel_mask)
#define RTK_MIS_I2C3_SDA_OUTPUT_DELAY_databit_mask  \
    (MISC_I2C_I2C3_SDA_OUTPUT_DELAY_CTRL_databitdelayenable_mask | MISC_I2C_I2C3_SDA_OUTPUT_DELAY_CTRL_databitdelaysel_mask)
#define RTK_MIS_I2C4_SDA_OUTPUT_DELAY_databit_mask  \
    (MISC_I2C4_I2C4_SDA_OUTPUT_DELAY_CTRL_databitdelayenable_mask | MISC_I2C4_I2C4_SDA_OUTPUT_DELAY_CTRL_databitdelaysel_mask)
#define RTK_MIS_I2C5_SDA_OUTPUT_DELAY_databit_mask  \
    (ISO_I2C_I2C5_SDA_OUTPUT_DELAY_CTRL_databitdelayenable_mask | ISO_I2C_I2C5_SDA_OUTPUT_DELAY_CTRL_databitdelaysel_mask)

#define I2C_GPIO_STATUS_GPIO_SCL_shift      ISO_I2C_I2C0_GUARD_INTERVAL_STATUS_gpi_ic_gpio_scl_shift
#define I2C_GPIO_STATUS_GPIO_SDA_shift      ISO_I2C_I2C0_GUARD_INTERVAL_STATUS_gpi_ic_gpio_sda_shift

#define I2C_GPIO_CTRL_SCL_shift             ISO_I2C_I2C0_GPIO_MODE_CTRL_ic_gpio_scl_shift
#define I2C_GPIO_CTRL_SDA_shift             ISO_I2C_I2C0_GPIO_MODE_CTRL_ic_gpio_sda_shift
#define I2C_GPIO_CTRL_SEL_shift             ISO_I2C_I2C0_GPIO_MODE_CTRL_ic_gpio_sel_shift

//MISC_I2C_IC1_CON_reg
#define IC_SLAVE_DISABLE                    (1 << MISC_I2C_IC1_CON_ic_slave_disable_shift)
#define IC_RESTART_EN                       (1 << MISC_I2C_IC1_CON_ic_restart_en_shift)
#define IC_10BITADDR_MASTER                 (1 << MISC_I2C_IC1_CON_ic_10bitaddr_master_shift)
#define IC_10BITADDR_SLAVE                  (1 << MISC_I2C_IC1_CON_ic_10bitaddr_slave_shift)
#define IC_SPEED_MASK                       MISC_I2C_IC1_CON_speed_mask
#define SPEED_SS                            MISC_I2C_IC1_CON_speed(0x1)
#define SPEED_FS                            MISC_I2C_IC1_CON_speed(0x2)
#define SPEED_HS                            MISC_I2C_IC1_CON_speed(0x3)
#define IC_MASTER_MODE                      (1 << MISC_I2C_IC1_CON_master_mode_shift)

//MISC_I2C_IC1_DATA_CMD_reg
#define READ_CMD                            (1 << MISC_I2C_IC1_DATA_CMD_cmd_shift)

//MISC_I2C_IC1_INTR_STAT_reg
#define GEN_CALL_BIT                        (1 << MISC_I2C_IC1_INTR_STAT_r_gen_call_shift)
#define START_DET_BIT                       (1 << MISC_I2C_IC1_INTR_STAT_r_start_det_shift)
#define STOP_DET_BIT                        (1 << MISC_I2C_IC1_INTR_STAT_r_stop_det_shift)
#define ACTIVITY_BIT                        (1 << MISC_I2C_IC1_INTR_STAT_r_activity_shift)
#define RX_DONE_BIT                         (1 << MISC_I2C_IC1_INTR_STAT_r_rx_done_shift)
#define TX_ABRT_BIT                         (1 << MISC_I2C_IC1_INTR_STAT_r_tx_abrt_shift)
#define RD_REQ_BIT                          (1 << MISC_I2C_IC1_INTR_STAT_r_rd_req_shift)
#define TX_EMPTY_BIT                        (1 << MISC_I2C_IC1_INTR_STAT_r_tx_empty_shift)
#define TX_OVER_BIT                         (1 << MISC_I2C_IC1_INTR_STAT_r_tx_over_shift)
#define RX_FULL_BIT                         (1 << MISC_I2C_IC1_INTR_STAT_r_rx_full_shift)
#define RX_OVER_BIT                         (1 << MISC_I2C_IC1_INTR_STAT_r_rx_over_shift)
#define RX_UNDER_BIT                        (1 << MISC_I2C_IC1_INTR_STAT_r_rx_under_shift)
#define GD_INTERVAL_ST_BIT                  (1 << MISC_I2C_I2C1_GUARD_INTERVAL_CTRL_ic_guard_interval_en_shift)
#define GD_INTERVAL_SEL_BIT                 (1 << MISC_I2C_I2C1_GUARD_INTERVAL_CTRL_ic_guard_interval_sel_shift)
#define GD_INTERVAL_SEL(x)                  MISC_I2C_I2C1_GUARD_INTERVAL_CTRL_ic_guard_interval_sel(x)
#define GD_INTERVAL_INT_EN_BIT              (1 << MISC_I2C_I2C1_GUARD_INTERVAL_CTRL_ic_guard_interval_int_en_shift)
#define GD_INTERVAL_1000US                  MISC_I2C_I2C1_GUARD_INTERVAL_CTRL_ic_guard_interval_delay_sel(0x0)
#define GD_INTERVAL_4000US                  MISC_I2C_I2C1_GUARD_INTERVAL_CTRL_ic_guard_interval_delay_sel(0x6)
#define GD_INTERVAL_3000US                  MISC_I2C_I2C1_GUARD_INTERVAL_CTRL_ic_guard_interval_delay_sel(0x5)
#define GD_INTERVAL_2000US                  MISC_I2C_I2C1_GUARD_INTERVAL_CTRL_ic_guard_interval_delay_sel(0x4)
#define GD_INTERVAL_400US                   MISC_I2C_I2C1_GUARD_INTERVAL_CTRL_ic_guard_interval_delay_sel(0x3)
#define GD_INTERVAL_600US                   MISC_I2C_I2C1_GUARD_INTERVAL_CTRL_ic_guard_interval_delay_sel(0x2)
#define GD_INTERVAL_800US                   MISC_I2C_I2C1_GUARD_INTERVAL_CTRL_ic_guard_interval_delay_sel(0x1)
#define GD_INTERVAL_EN_BIT                  (1 << MISC_I2C_I2C1_GUARD_INTERVAL_CTRL_ic_guard_interval_en_shift)
#define GD_INTERVAL_EEPROM                  (GD_INTERVAL_INT_EN_BIT | GD_INTERVAL_2000US | GD_INTERVAL_EN_BIT)

//MISC_I2C_IC1_STATUS_reg
#define ST_RFF_BIT                          (1 << MISC_I2C_IC1_STATUS_rff_shift)
#define ST_RFNE_BIT                         (1 << MISC_I2C_IC1_STATUS_rfne_shift)
#define ST_TFE_BIT                          (1 << MISC_I2C_IC1_STATUS_tfe_shift)
#define ST_TFNF_BIT                         (1 << MISC_I2C_IC1_STATUS_tfnf_shift)
#define ST_ACTIVITY_BIT                     (1 << MISC_I2C_IC1_STATUS_activity_shift)

static const rtk_i2c_reg_map RTK_I2C_i2c0_reg =
{
    .IC_ISR_EN             = SYS_REG_INT_CTRL_SCPU_reg,
    .IC_ISR_EN_MASK        = (1 << ISO_MISC_OFF_ISR_i2c0_int_shift),
    .IC_ISR                = ISO_MISC_OFF_ISR_reg,
    .IC_INT                = (1 << ISO_MISC_OFF_ISR_i2c0_int_shift),
    .IC_CLKSEL             = STB_ST_CLKMUX_reg,
    .IC_CLKSEL_MASK        = STB_ST_CLKMUX_i2c0_off_clksel_mask,
    .IC_INTERVAL_MODE_CTRL = ISO_I2C_I2C0_GUARD_INTERVAL_CTRL_reg,
    .IC_GUARD_STATUS       = ISO_I2C_I2C0_GUARD_INTERVAL_STATUS_reg,
    .IC_GPIO_MODE_CTRL     = ISO_I2C_I2C0_GPIO_MODE_CTRL_reg,
    .IC_PH_CR              = ISO_I2C_I2C0_PH_CR_reg,
    .IC_CON                = ISO_I2C_IC0_CON_reg,
    .IC_TAR                = ISO_I2C_IC0_TAR_reg,
    .IC_SAR                = ISO_I2C_IC0_SAR_reg,
    .IC_HS_MADDR           = ISO_I2C_IC0_HS_MADDR_reg,
    .IC_DATA_CMD           = ISO_I2C_IC0_DATA_CMD_reg,
    .IC_SS_SCL_HCNT        = ISO_I2C_IC0_SS_SCL_HCNT_reg,
    .IC_SS_SCL_LCNT        = ISO_I2C_IC0_SS_SCL_LCNT_reg,
    .IC_FS_SCL_HCNT        = ISO_I2C_IC0_FS_SCL_HCNT_reg,
    .IC_FS_SCL_LCNT        = ISO_I2C_IC0_FS_SCL_LCNT_reg,
    .IC_INTR_STAT          = ISO_I2C_IC0_INTR_STAT_reg,
    .IC_INTR_MASK          = ISO_I2C_IC0_INTR_MASK_reg,
    .IC_RAW_INTR_STAT      = ISO_I2C_IC0_RAW_INTR_STAT_reg,
    .IC_RX_TL              = ISO_I2C_IC0_RX_TL_reg,
    .IC_TX_TL              = ISO_I2C_IC0_TX_TL_reg,
    .IC_CLR_INTR           = ISO_I2C_IC0_CLR_INTR_reg,
    .IC_CLR_RX_UNDER       = ISO_I2C_IC0_CLR_RX_UNDER_reg,
    .IC_CLR_RX_OVER        = ISO_I2C_IC0_CLR_RX_OVER_reg,
    .IC_CLR_TX_OVER        = ISO_I2C_IC0_CLR_TX_OVER_reg,
    .IC_CLR_RD_REQ         = ISO_I2C_IC0_CLR_RD_REQ_reg,
    .IC_CLR_TX_ABRT        = ISO_I2C_IC0_CLR_TX_ABRT_reg,
    .IC_CLR_RX_DONE        = ISO_I2C_IC0_CLR_RX_DONE_reg,
    .IC_CLR_ACTIVITY       = ISO_I2C_IC0_CLR_ACTIVITY_reg,
    .IC_CLR_STOP_DET       = ISO_I2C_IC0_CLR_STOP_DET_reg,
    .IC_CLR_START_DET      = ISO_I2C_IC0_CLR_START_DET_reg,
    .IC_CLR_GEN_CALL       = ISO_I2C_IC0_CLR_GEN_CALL_reg,
    .IC_ENABLE             = ISO_I2C_IC0_ENABLE_reg,
    .IC_STATUS             = ISO_I2C_IC0_STATUS_reg,
    .IC_TXFLR              = ISO_I2C_IC0_TXFLR_reg,
    .IC_RXFLR              = ISO_I2C_IC0_RXFLR_reg,
    .IC_SDA_HOLD           = ISO_I2C_IC0_SDA_HOLD_reg,
    .IC_TX_ABRT_SOURCE     = ISO_I2C_IC0_TX_ABRT_SOURCE_reg,
    .IC_SLV_DATA_NACK      = ISO_I2C_IC0_SLV_DATA_NACK_ONLY_reg,
    .IC_SDA_SETUP          = ISO_I2C_IC0_SDA_SETUP_reg,
    .IC_ACK_GEN_CALL       = ISO_I2C_IC0_ACK_GENERAL_CALL_reg,
    .IC_ENABLE_STATUS      = ISO_I2C_IC0_ENABLE_STATUS_reg,
    .IC_COMP_PARAM_1       = ISO_I2C_IC0_COMP_PARAM_1_reg,
    .IC_COMP_VERSION       = ISO_I2C_IC0_COMP_VERSION_reg,
    .IC_COMP_TYPE          = ISO_I2C_IC0_COMP_TYPE_reg,
    .IC_SDA_DEL            = ISO_I2C_I2C0_SDA_OUTPUT_DELAY_CTRL_reg,
};

static const rtk_i2c_port RTK_I2C_i2c0_port[] =
{
	{
		.gpio_mapped       = 1,
		.g2c_scl           = rtk_gpio_id(ISO_GPIO, 9),
		.g2c_sda           = rtk_gpio_id(ISO_GPIO, 8),
		.input_mux[0]      = {0xB8060248, (0x3<<20), (0x0<<20)},//SCL
		.input_mux[1]      = {0xB8060248, (0x3<<16), (0x0<<16)},//SDA
		.pin_mux[0]        = {0xb8060210, (0xF<<20), (0xB<<20), (0xF<<20)},
		.pin_mux[1]        = {0xb8060210, (0xF<<28), (0xB<<28), (0xF<<28)},
	},
	{
		.gpio_mapped       = 1,
		.g2c_scl           = rtk_gpio_id(ISO_GPIO, 51),
		.g2c_sda           = rtk_gpio_id(ISO_GPIO, 52),
		.input_mux[0]      = {0xB8060248, (0x3<<20), (0x1<<20)},
		.input_mux[1]      = {0xB8060248, (0x3<<16), (0x1<<16)},
		.pin_mux[0]        = {0xB8060230, (0xF<<28), (0xB<<28), (0xF<<28)},
		.pin_mux[1]        = {0xB8060230, (0xF<<20), (0xB<<20), (0xF<<20)},
	},
};

static rtk_i2c_init_reg RTK_I2C_i2c0_init_reg[] =
{
    {ISO_I2C_I2C0_SDA_OUTPUT_DELAY_CTRL_reg, RTK_ISO_I2C0_SDA_OUTPUT_DELAY_databit_mask, ISO_I2C_I2C0_SDA_OUTPUT_DELAY_CTRL_databitdelaysel(0x07)},
    {ISO_I2C_IC0_SDA_SETUP_reg, ISO_I2C_IC0_SDA_SETUP_sda_setup_mask, ISO_I2C_IC0_SDA_SETUP_sda_setup(0x64)},
    {SYS_REG_INT_CTRL_SCPU_reg, (~0), (1 << RTK_I2C_ISO_MIS_OFF_SCPU_INT_EN_shift) | (1 << SYS_REG_INT_CTRL_SCPU_write_data_shift)},    // enable iso misc off SCPU Routing
    {ISO_MISC_OFF_SYSTEM_reg, ISO_MISC_OFF_SYSTEM_i2c0_scpu_and_8051_int_sel_mask, ISO_MISC_OFF_SYSTEM_i2c0_scpu_and_8051_int_sel(1)},  // sel i2c0 iso misc off SCPU Routing
    {ISO_MISC_OFF_SYSTEM_reg, ISO_MISC_OFF_SYSTEM_i2c0_scpu_and_8051_int_en_mask, ISO_MISC_OFF_SYSTEM_i2c0_scpu_and_8051_int_en(1)},    // en i2c0 iso misc off SCPU Routing
};

static const rtk_i2c_reg_map RTK_I2C_i2c1_reg =
{
    .IC_ISR_EN             = SYS_REG_INT_CTRL_SCPU_reg,
    .IC_ISR_EN_MASK        = (1 << MISC_isr_i2c1_int_shift),
    .IC_ISR                = MISC_isr_reg,
    .IC_INT                = (1 << MISC_isr_i2c1_int_shift),
    .IC_INTERVAL_MODE_CTRL = MISC_I2C_I2C1_GUARD_INTERVAL_CTRL_reg,
    .IC_CLKSEL             = SYS_REG_SYS_CLKDIV_reg,
    .IC_CLKSEL_MASK        = SYS_REG_SYS_CLKDIV_i2c1_clksel_mask,
    .IC_GUARD_STATUS       = MISC_I2C_I2C1_GUARD_INTERVAL_STATUS_reg,
    .IC_GPIO_MODE_CTRL     = MISC_I2C_I2C1_GPIO_MODE_CTRL_reg,
    .IC_PH_CR              = MISC_I2C_I2C1_PH_CR_reg,
    .IC_CON                = MISC_I2C_IC1_CON_reg,
    .IC_TAR                = MISC_I2C_IC1_TAR_reg,
    .IC_SAR                = MISC_I2C_IC1_SAR_reg,
    .IC_HS_MADDR           = MISC_I2C_IC1_HS_MADDR_reg,
    .IC_DATA_CMD           = MISC_I2C_IC1_DATA_CMD_reg,
    .IC_SS_SCL_HCNT        = MISC_I2C_IC1_SS_SCL_HCNT_reg,
    .IC_SS_SCL_LCNT        = MISC_I2C_IC1_SS_SCL_LCNT_reg,
    .IC_FS_SCL_HCNT        = MISC_I2C_IC1_FS_SCL_HCNT_reg,
    .IC_FS_SCL_LCNT        = MISC_I2C_IC1_FS_SCL_LCNT_reg,
    .IC_INTR_STAT          = MISC_I2C_IC1_INTR_STAT_reg,
    .IC_INTR_MASK          = MISC_I2C_IC1_INTR_MASK_reg,
    .IC_RAW_INTR_STAT      = MISC_I2C_IC1_RAW_INTR_STAT_reg,
    .IC_RX_TL              = MISC_I2C_IC1_RX_TL_reg,
    .IC_TX_TL              = MISC_I2C_IC1_TX_TL_reg,
    .IC_CLR_INTR           = MISC_I2C_IC1_CLR_INTR_reg,
    .IC_CLR_RX_UNDER       = MISC_I2C_IC1_CLR_RX_UNDER_reg,
    .IC_CLR_RX_OVER        = MISC_I2C_IC1_CLR_RX_OVER_reg,
    .IC_CLR_TX_OVER        = MISC_I2C_IC1_CLR_TX_OVER_reg,
    .IC_CLR_RD_REQ         = MISC_I2C_IC1_CLR_RD_REQ_reg,
    .IC_CLR_TX_ABRT        = MISC_I2C_IC1_CLR_TX_ABRT_reg,
    .IC_CLR_RX_DONE        = MISC_I2C_IC1_CLR_RX_DONE_reg,
    .IC_CLR_ACTIVITY       = MISC_I2C_IC1_CLR_ACTIVITY_reg,
    .IC_CLR_STOP_DET       = MISC_I2C_IC1_CLR_STOP_DET_reg,
    .IC_CLR_START_DET      = MISC_I2C_IC1_CLR_START_DET_reg,
    .IC_CLR_GEN_CALL       = MISC_I2C_IC1_CLR_GEN_CALL_reg,
    .IC_ENABLE             = MISC_I2C_IC1_ENABLE_reg,
    .IC_STATUS             = MISC_I2C_IC1_STATUS_reg,
    .IC_TXFLR              = MISC_I2C_IC1_TXFLR_reg,
    .IC_RXFLR              = MISC_I2C_IC1_RXFLR_reg,
    .IC_SDA_HOLD           = MISC_I2C_IC1_SDA_HOLD_reg,
    .IC_TX_ABRT_SOURCE     = MISC_I2C_IC1_TX_ABRT_SOURCE_reg,
    .IC_SLV_DATA_NACK      = MISC_I2C_IC1_SLV_DATA_NACK_ONLY_reg,
    .IC_SDA_SETUP          = MISC_I2C_IC1_SDA_SETUP_reg,
    .IC_ACK_GEN_CALL       = MISC_I2C_IC1_ACK_GENERAL_CALL_reg,
    .IC_ENABLE_STATUS      = MISC_I2C_IC1_ENABLE_STATUS_reg,
    .IC_COMP_PARAM_1       = MISC_I2C_IC1_COMP_PARAM_1_reg,
    .IC_COMP_VERSION       = MISC_I2C_IC1_COMP_VERSION_reg,
    .IC_COMP_TYPE          = MISC_I2C_IC1_COMP_TYPE_reg,
    .IC_SDA_DEL            = MISC_I2C_I2C1_SDA_OUTPUT_DELAY_CTRL_reg,
};

static const rtk_i2c_port RTK_I2C_i2c1_port[] =
{
	{
		.gpio_mapped       = 1,
		.g2c_scl           = rtk_gpio_id(MIS_GPIO, 60),
		.g2c_sda           = rtk_gpio_id(MIS_GPIO, 59),
		.input_mux[0]      = {0x0, (0), (0)},//table not match register
		.input_mux[1]      = {0x0, (0), (0)},
		.pin_mux[0]        = {0xb8000840,(0xF<<4), (0xB<<4),  (0xF<<4)},
		.pin_mux[1]        = {0xb8000840,(0xF<<12),(0xB<<12), (0xF<<12)},
	},
};

static rtk_i2c_init_reg RTK_I2C_i2c1_init_reg[] =
{
    {MISC_I2C_I2C1_SDA_OUTPUT_DELAY_CTRL_reg, RTK_MIS_I2C1_SDA_OUTPUT_DELAY_databit_mask, MISC_I2C_I2C1_SDA_OUTPUT_DELAY_CTRL_databitdelaysel(0x07)},
    {MISC_I2C_IC1_SDA_SETUP_reg, MISC_I2C_IC1_SDA_SETUP_sda_setup_mask, MISC_I2C_IC1_SDA_SETUP_sda_setup(0x64)},
    {MISC_isr_switch_reg, MISC_isr_switch_i2c1_int_sel_mask, MISC_isr_switch_i2c1_int_sel(0)},   //select scpu int
    {MISC_SYSTEM_reg, MISC_SYSTEM_i2c1_int_en_mask, MISC_SYSTEM_i2c1_int_en(1)},   //select scpu int
};

static const rtk_i2c_reg_map RTK_I2C_i2c2_reg =
{
    .IC_ISR_EN             = SYS_REG_INT_CTRL_SCPU_reg,
    .IC_ISR_EN_MASK        = (1 << MISC_isr_i2c2_int_shift),
    .IC_ISR                = MISC_isr_reg,
    .IC_INT                = (1 << MISC_isr_i2c2_int_shift),
    .IC_CLKSEL             = SYS_REG_SYS_CLKDIV_reg,
    .IC_CLKSEL_MASK        = SYS_REG_SYS_CLKDIV_i2c2_clksel_mask,
    .IC_INTERVAL_MODE_CTRL = MISC_I2C_I2C2_GUARD_INTERVAL_CTRL_reg,
    .IC_GUARD_STATUS       = MISC_I2C_I2C2_GUARD_INTERVAL_STATUS_reg,
    .IC_GPIO_MODE_CTRL     = MISC_I2C_I2C2_GPIO_MODE_CTRL_reg,
    .IC_PH_CR              = MISC_I2C_I2C2_PH_CR_reg,
    .IC_CON                = MISC_I2C_IC2_CON_reg,
    .IC_TAR                = MISC_I2C_IC2_TAR_reg,
    .IC_SAR                = MISC_I2C_IC2_SAR_reg,
    .IC_HS_MADDR           = MISC_I2C_IC2_HS_MADDR_reg,
    .IC_DATA_CMD           = MISC_I2C_IC2_DATA_CMD_reg,
    .IC_SS_SCL_HCNT        = MISC_I2C_IC2_SS_SCL_HCNT_reg,
    .IC_SS_SCL_LCNT        = MISC_I2C_IC2_SS_SCL_LCNT_reg,
    .IC_FS_SCL_HCNT        = MISC_I2C_IC2_FS_SCL_HCNT_reg,
    .IC_FS_SCL_LCNT        = MISC_I2C_IC2_FS_SCL_LCNT_reg,
    .IC_INTR_STAT          = MISC_I2C_IC2_INTR_STAT_reg,
    .IC_INTR_MASK          = MISC_I2C_IC2_INTR_MASK_reg,
    .IC_RAW_INTR_STAT      = MISC_I2C_IC2_RAW_INTR_STAT_reg,
    .IC_RX_TL              = MISC_I2C_IC2_RX_TL_reg,
    .IC_TX_TL              = MISC_I2C_IC2_TX_TL_reg,
    .IC_CLR_INTR           = MISC_I2C_IC2_CLR_INTR_reg,
    .IC_CLR_RX_UNDER       = MISC_I2C_IC2_CLR_RX_UNDER_reg,
    .IC_CLR_RX_OVER        = MISC_I2C_IC2_CLR_RX_OVER_reg,
    .IC_CLR_TX_OVER        = MISC_I2C_IC2_CLR_TX_OVER_reg,
    .IC_CLR_RD_REQ         = MISC_I2C_IC2_CLR_RD_REQ_reg,
    .IC_CLR_TX_ABRT        = MISC_I2C_IC2_CLR_TX_ABRT_reg,
    .IC_CLR_RX_DONE        = MISC_I2C_IC2_CLR_RX_DONE_reg,
    .IC_CLR_ACTIVITY       = MISC_I2C_IC2_CLR_ACTIVITY_reg,
    .IC_CLR_STOP_DET       = MISC_I2C_IC2_CLR_STOP_DET_reg,
    .IC_CLR_START_DET      = MISC_I2C_IC2_CLR_START_DET_reg,
    .IC_CLR_GEN_CALL       = MISC_I2C_IC2_CLR_GEN_CALL_reg,
    .IC_ENABLE             = MISC_I2C_IC2_ENABLE_reg,
    .IC_STATUS             = MISC_I2C_IC2_STATUS_reg,
    .IC_TXFLR              = MISC_I2C_IC2_TXFLR_reg,
    .IC_RXFLR              = MISC_I2C_IC2_RXFLR_reg,
    .IC_SDA_HOLD           = MISC_I2C_IC2_SDA_HOLD_reg,
    .IC_TX_ABRT_SOURCE     = MISC_I2C_IC2_TX_ABRT_SOURCE_reg,
    .IC_SLV_DATA_NACK      = MISC_I2C_IC2_SLV_DATA_NACK_ONLY_reg,
    .IC_SDA_SETUP          = MISC_I2C_IC2_SDA_SETUP_reg,
    .IC_ACK_GEN_CALL       = MISC_I2C_IC2_ACK_GENERAL_CALL_reg,
    .IC_ENABLE_STATUS      = MISC_I2C_IC2_ENABLE_STATUS_reg,
    .IC_COMP_PARAM_1       = MISC_I2C_IC2_COMP_PARAM_1_reg,
    .IC_COMP_VERSION       = MISC_I2C_IC2_COMP_VERSION_reg,
    .IC_COMP_TYPE          = MISC_I2C_IC2_COMP_TYPE_reg,
    .IC_SDA_DEL            = MISC_I2C_I2C2_SDA_OUTPUT_DELAY_CTRL_reg,
};

static const rtk_i2c_port RTK_I2C_i2c2_port[] =
{
	{
		.gpio_mapped       = 1,
		.g2c_scl           = rtk_gpio_id(MIS_GPIO, 58),
		.g2c_sda           = rtk_gpio_id(MIS_GPIO, 57),
		.input_mux[0]      = {0xb80008f0, (0x3<<12), (0x0<<12)},
		.input_mux[1]      = {0xb80008f0, (0x3<<8),  (0x0<<8)},
		.pin_mux[0]        = {0xb8000840, (0xF<<20), (0xB<<20), (0xF<<20)},
		.pin_mux[1]        = {0xb8000840, (0xF<<28), (0xB<<28), (0xF<<28)},
	},
	{
		.gpio_mapped       = 1,
		.g2c_scl           = rtk_gpio_id(ISO_GPIO, 4),
		.g2c_sda           = rtk_gpio_id(ISO_GPIO, 5),
		.input_mux[0]      = {0xb80008f0, (0x3<<12), (0x1<<12)},
		.input_mux[1]      = {0xb80008f0, (0x3<<8),  (0x1<<8)},
		.pin_mux[0]        = {0xb806020c, (0xF<<28), (0xB<<28), (0xF<<28)},
		.pin_mux[1]        = {0xb806020c, (0xF<<20), (0xB<<20), (0xF<<20)},
	},
};

static rtk_i2c_init_reg RTK_I2C_i2c2_init_reg[] =
{
    {MISC_I2C_I2C2_SDA_OUTPUT_DELAY_CTRL_reg, RTK_MIS_I2C2_SDA_OUTPUT_DELAY_databit_mask, MISC_I2C_I2C2_SDA_OUTPUT_DELAY_CTRL_databitdelaysel(0x01)},
    {MISC_I2C_IC2_SDA_SETUP_reg, MISC_I2C_IC2_SDA_SETUP_sda_setup_mask, MISC_I2C_IC2_SDA_SETUP_sda_setup(0x64)},
    {MISC_isr_switch_reg, MISC_isr_switch_i2c2_int_sel_mask, MISC_isr_switch_i2c2_int_sel(0)},   //select scpu int
    {MISC_SYSTEM_reg, MISC_SYSTEM_i2c2_int_en_mask, MISC_SYSTEM_i2c2_int_en(1)},   //select scpu int
};

static const rtk_i2c_reg_map RTK_I2C_i2c3_reg =
{
    .IC_ISR_EN             = SYS_REG_INT_CTRL_SCPU_reg,
    .IC_ISR_EN_MASK        = (1 << MISC_isr_i2c3_int_shift),
    .IC_ISR                = MISC_isr_reg,
    .IC_INT                = (1 << MISC_isr_i2c3_int_shift),
    .IC_CLKSEL             = SYS_REG_SYS_CLKDIV_reg,
    .IC_CLKSEL_MASK        = SYS_REG_SYS_CLKDIV_i2c3_clksel_mask,
    .IC_INTERVAL_MODE_CTRL = MISC_I2C_I2C3_GUARD_INTERVAL_CTRL_reg,
    .IC_GUARD_STATUS       = MISC_I2C_I2C3_GUARD_INTERVAL_STATUS_reg,
    .IC_GPIO_MODE_CTRL     = MISC_I2C_I2C3_GPIO_MODE_CTRL_reg,
    .IC_PH_CR              = MISC_I2C_I2C3_PH_CR_reg,
    .IC_CON                = MISC_I2C_IC3_CON_reg,
    .IC_TAR                = MISC_I2C_IC3_TAR_reg,
    .IC_SAR                = MISC_I2C_IC3_SAR_reg,
    .IC_HS_MADDR           = MISC_I2C_IC3_HS_MADDR_reg,
    .IC_DATA_CMD           = MISC_I2C_IC3_DATA_CMD_reg,
    .IC_SS_SCL_HCNT        = MISC_I2C_IC3_SS_SCL_HCNT_reg,
    .IC_SS_SCL_LCNT        = MISC_I2C_IC3_SS_SCL_LCNT_reg,
    .IC_FS_SCL_HCNT        = MISC_I2C_IC3_FS_SCL_HCNT_reg,
    .IC_FS_SCL_LCNT        = MISC_I2C_IC3_FS_SCL_LCNT_reg,
    .IC_INTR_STAT          = MISC_I2C_IC3_INTR_STAT_reg,
    .IC_INTR_MASK          = MISC_I2C_IC3_INTR_MASK_reg,
    .IC_RAW_INTR_STAT      = MISC_I2C_IC3_RAW_INTR_STAT_reg,
    .IC_RX_TL              = MISC_I2C_IC3_RX_TL_reg,
    .IC_TX_TL              = MISC_I2C_IC3_TX_TL_reg,
    .IC_CLR_INTR           = MISC_I2C_IC3_CLR_INTR_reg,
    .IC_CLR_RX_UNDER       = MISC_I2C_IC3_CLR_RX_UNDER_reg,
    .IC_CLR_RX_OVER        = MISC_I2C_IC3_CLR_RX_OVER_reg,
    .IC_CLR_TX_OVER        = MISC_I2C_IC3_CLR_TX_OVER_reg,
    .IC_CLR_RD_REQ         = MISC_I2C_IC3_CLR_RD_REQ_reg,
    .IC_CLR_TX_ABRT        = MISC_I2C_IC3_CLR_TX_ABRT_reg,
    .IC_CLR_RX_DONE        = MISC_I2C_IC3_CLR_RX_DONE_reg,
    .IC_CLR_ACTIVITY       = MISC_I2C_IC3_CLR_ACTIVITY_reg,
    .IC_CLR_STOP_DET       = MISC_I2C_IC3_CLR_STOP_DET_reg,
    .IC_CLR_START_DET      = MISC_I2C_IC3_CLR_START_DET_reg,
    .IC_CLR_GEN_CALL       = MISC_I2C_IC3_CLR_GEN_CALL_reg,
    .IC_ENABLE             = MISC_I2C_IC3_ENABLE_reg,
    .IC_STATUS             = MISC_I2C_IC3_STATUS_reg,
    .IC_TXFLR              = MISC_I2C_IC3_TXFLR_reg,
    .IC_RXFLR              = MISC_I2C_IC3_RXFLR_reg,
    .IC_SDA_HOLD           = MISC_I2C_IC3_SDA_HOLD_reg,
    .IC_TX_ABRT_SOURCE     = MISC_I2C_IC3_TX_ABRT_SOURCE_reg,
    .IC_SLV_DATA_NACK      = MISC_I2C_IC3_SLV_DATA_NACK_ONLY_reg,
    .IC_SDA_SETUP          = MISC_I2C_IC3_SDA_SETUP_reg,
    .IC_ACK_GEN_CALL       = MISC_I2C_IC3_ACK_GENERAL_CALL_reg,
    .IC_ENABLE_STATUS      = MISC_I2C_IC3_ENABLE_STATUS_reg,
    .IC_COMP_PARAM_1       = MISC_I2C_IC3_COMP_PARAM_1_reg,
    .IC_COMP_VERSION       = MISC_I2C_IC3_COMP_VERSION_reg,
    .IC_COMP_TYPE          = MISC_I2C_IC3_COMP_TYPE_reg,
    .IC_SDA_DEL            = MISC_I2C_I2C3_SDA_OUTPUT_DELAY_CTRL_reg,
};

static const rtk_i2c_port RTK_I2C_i2c3_port[] =
{
	{
		.gpio_mapped       = 1,
		.g2c_scl           = rtk_gpio_id(MIS_GPIO, 25),
		.g2c_sda           = rtk_gpio_id(MIS_GPIO, 24),
		.input_mux[0]      = {0xb80008f0, (0x3<<4),  (0x0<<4)},
		.input_mux[1]      = {0xb80008f0, (0x3<<0),  (0x0<<0)},
		.pin_mux[0]        = {0xb8000818, (0xF<<20), (0xB<<20), (0xF<<20)},
		.pin_mux[1]        = {0xb8000818, (0xF<<28), (0xB<<28), (0xF<<28)},
	},
	{
		.gpio_mapped       = 1,
		.g2c_scl           = rtk_gpio_id(MIS_GPIO, 47),
		.g2c_sda           = rtk_gpio_id(MIS_GPIO, 43),
		.input_mux[0]      = {0xb80008f0, (0x3<<4), (0x1<<4)},
		.input_mux[1]      = {0xb80008f0, (0x3<<0), (0x1<<0)},
		.pin_mux[0]        = {0xb8000834, (0xF<<24),(0xB<<24), (0xF<<24)},
		.pin_mux[1]        = {0xb8000830, (0xF<<16),(0xB<<16), (0xF<<16)},
	},
};

static rtk_i2c_init_reg RTK_I2C_i2c3_init_reg[] =
{
    {MISC_I2C_I2C3_SDA_OUTPUT_DELAY_CTRL_reg, RTK_MIS_I2C3_SDA_OUTPUT_DELAY_databit_mask, MISC_I2C_I2C3_SDA_OUTPUT_DELAY_CTRL_databitdelaysel(0x01)},
    {MISC_I2C_IC3_SDA_SETUP_reg, MISC_I2C_IC3_SDA_SETUP_sda_setup_mask, MISC_I2C_IC3_SDA_SETUP_sda_setup(0x64)},
    {MISC_isr_switch_reg, MISC_isr_switch_i2c3_int_sel_mask, MISC_isr_switch_i2c3_int_sel(0)},   //select scpu int
    {MISC_SYSTEM_reg, MISC_SYSTEM_i2c3_int_en_mask, MISC_SYSTEM_i2c3_int_en(1)},   //select scpu int
};

static const rtk_i2c_reg_map RTK_I2C_i2c4_reg =
{
    .IC_ISR_EN             = SYS_REG_INT_CTRL_SCPU_reg,
    .IC_ISR_EN_MASK        = (1 << MISC_isr_i2c4_int_shift),
    .IC_ISR                = MISC_isr_reg,
    .IC_INT                = (1 << MISC_isr_i2c4_int_shift),
    .IC_CLKSEL             = SYS_REG_SYS_CLKDIV_reg,
    .IC_CLKSEL_MASK        = SYS_REG_SYS_CLKDIV_i2c4_clksel_mask,
    .IC_INTERVAL_MODE_CTRL = MISC_I2C4_I2C4_GUARD_INTERVAL_CTRL_reg,
    .IC_GUARD_STATUS       = MISC_I2C4_I2C4_GUARD_INTERVAL_STATUS_reg,
    .IC_GPIO_MODE_CTRL     = MISC_I2C4_I2C4_GPIO_MODE_CTRL_reg,
    .IC_CON                = MISC_I2C4_IC4_CON_reg,
    .IC_PH_CR              = MISC_I2C4_I2C4_PH_CR_reg,
    .IC_TAR                = MISC_I2C4_IC4_TAR_reg,
    .IC_SAR                = MISC_I2C4_IC4_SAR_reg,
    .IC_HS_MADDR           = MISC_I2C4_IC4_HS_MADDR_reg,
    .IC_DATA_CMD           = MISC_I2C4_IC4_DATA_CMD_reg,
    .IC_SS_SCL_HCNT        = MISC_I2C4_IC4_SS_SCL_HCNT_reg,
    .IC_SS_SCL_LCNT        = MISC_I2C4_IC4_SS_SCL_LCNT_reg,
    .IC_FS_SCL_HCNT        = MISC_I2C4_IC4_FS_SCL_HCNT_reg,
    .IC_FS_SCL_LCNT        = MISC_I2C4_IC4_FS_SCL_LCNT_reg,
    .IC_INTR_STAT          = MISC_I2C4_IC4_INTR_STAT_reg,
    .IC_INTR_MASK          = MISC_I2C4_IC4_INTR_MASK_reg,
    .IC_RAW_INTR_STAT      = MISC_I2C4_IC4_RAW_INTR_STAT_reg,
    .IC_RX_TL              = MISC_I2C4_IC4_RX_TL_reg,
    .IC_TX_TL              = MISC_I2C4_IC4_TX_TL_reg,
    .IC_CLR_INTR           = MISC_I2C4_IC4_CLR_INTR_reg,
    .IC_CLR_RX_UNDER       = MISC_I2C4_IC4_CLR_RX_UNDER_reg,
    .IC_CLR_RX_OVER        = MISC_I2C4_IC4_CLR_RX_OVER_reg,
    .IC_CLR_TX_OVER        = MISC_I2C4_IC4_CLR_TX_OVER_reg,
    .IC_CLR_RD_REQ         = MISC_I2C4_IC4_CLR_RD_REQ_reg,
    .IC_CLR_TX_ABRT        = MISC_I2C4_IC4_CLR_TX_ABRT_reg,
    .IC_CLR_RX_DONE        = MISC_I2C4_IC4_CLR_RX_DONE_reg,
    .IC_CLR_ACTIVITY       = MISC_I2C4_IC4_CLR_ACTIVITY_reg,
    .IC_CLR_STOP_DET       = MISC_I2C4_IC4_CLR_STOP_DET_reg,
    .IC_CLR_START_DET      = MISC_I2C4_IC4_CLR_START_DET_reg,
    .IC_CLR_GEN_CALL       = MISC_I2C4_IC4_CLR_GEN_CALL_reg,
    .IC_ENABLE             = MISC_I2C4_IC4_ENABLE_reg,
    .IC_STATUS             = MISC_I2C4_IC4_STATUS_reg,
    .IC_TXFLR              = MISC_I2C4_IC4_TXFLR_reg,
    .IC_RXFLR              = MISC_I2C4_IC4_RXFLR_reg,
    .IC_SDA_HOLD           = MISC_I2C4_IC4_SDA_HOLD_reg,
    .IC_TX_ABRT_SOURCE     = MISC_I2C4_IC4_TX_ABRT_SOURCE_reg,
    .IC_SLV_DATA_NACK      = MISC_I2C4_IC4_SLV_DATA_NACK_ONLY_reg,
    .IC_SDA_SETUP          = MISC_I2C4_IC4_SDA_SETUP_reg,
    .IC_ACK_GEN_CALL       = MISC_I2C4_IC4_ACK_GENERAL_CALL_reg,
    .IC_ENABLE_STATUS      = MISC_I2C4_IC4_ENABLE_STATUS_reg,
    .IC_COMP_PARAM_1       = MISC_I2C4_IC4_COMP_PARAM_1_reg,
    .IC_COMP_VERSION       = MISC_I2C4_IC4_COMP_VERSION_reg,
    .IC_COMP_TYPE          = MISC_I2C4_IC4_COMP_TYPE_reg,
    .IC_SDA_DEL            = MISC_I2C4_I2C4_SDA_OUTPUT_DELAY_CTRL_reg,
};

static const rtk_i2c_port RTK_I2C_i2c4_port[] =
{
	{
		.gpio_mapped       = 1,
		.g2c_scl           = rtk_gpio_id(MIS_GPIO, 35),
		.g2c_sda           = rtk_gpio_id(MIS_GPIO, 34),
		.input_mux[0]      = {0xb80008f4, (0x3<<28), (0x0<<28)},
		.input_mux[1]      = {0xb80008f4, (0x3<<24), (0x0<<24)},
		.pin_mux[0]        = {0xb8000828, (0xF<<16), (0xB<<16), (0xF<<16)},
		.pin_mux[1]        = {0xb8000828, (0xF<<20), (0xB<<20), (0xF<<20)},
	},
	{
		.gpio_mapped       = 1,
		.g2c_scl           = rtk_gpio_id(MIS_GPIO, 19),
		.g2c_sda           = rtk_gpio_id(MIS_GPIO, 18),
		.input_mux[0]      = {0xb80008f4, (0x3<<28), (0x1<<28)},
		.input_mux[1]      = {0xb80008f4, (0x3<<24), (0x1<<24)},
		.pin_mux[0]        = {0xb8000810, (0xF<<4),  (0xB<<4),  (0xF<<4)},
		.pin_mux[1]        = {0xb8000810, (0xF<<12), (0xB<<12), (0xF<<12)},
	},
};

static rtk_i2c_init_reg RTK_I2C_i2c4_init_reg[] =
{
    {MISC_I2C4_I2C4_SDA_OUTPUT_DELAY_CTRL_reg, RTK_MIS_I2C4_SDA_OUTPUT_DELAY_databit_mask, MISC_I2C4_I2C4_SDA_OUTPUT_DELAY_CTRL_databitdelaysel(0x01)},
    {MISC_I2C4_IC4_SDA_SETUP_reg, MISC_I2C4_IC4_SDA_SETUP_sda_setup_mask, MISC_I2C4_IC4_SDA_SETUP_sda_setup(0x64)},
    {MISC_isr_switch_reg, MISC_isr_switch_i2c4_int_sel_mask, MISC_isr_switch_i2c4_int_sel(0)},   //select scpu int
    {MISC_SYSTEM_reg,   MISC_SYSTEM_i2c4_int_en_mask,      MISC_SYSTEM_i2c4_int_en(1)},
};

static const rtk_i2c_reg_map RTK_I2C_i2c5_reg =
{
    .IC_ISR_EN             = SYS_REG_INT_CTRL_SCPU_reg,
    .IC_ISR_EN_MASK        = (1 << ISO_MISC_OFF_ISR_i2c5_int_shift),
    .IC_ISR                = ISO_MISC_OFF_ISR_reg,
    .IC_INT                = (1 << ISO_MISC_OFF_ISR_i2c5_int_shift),
    .IC_CLKSEL             = STB_ST_CLKMUX_reg,
    .IC_CLKSEL_MASK        = STB_ST_CLKMUX_i2c5_off_clksel_mask,
    .IC_INTERVAL_MODE_CTRL = ISO_I2C_I2C5_GUARD_INTERVAL_CTRL_reg,
    .IC_GUARD_STATUS       = ISO_I2C_I2C5_GUARD_INTERVAL_STATUS_reg,
    .IC_GPIO_MODE_CTRL     = ISO_I2C_I2C5_GPIO_MODE_CTRL_reg,
    .IC_PH_CR              = ISO_I2C_I2C5_PH_CR_reg,
    .IC_CON                = ISO_I2C_IC5_CON_reg,
    .IC_TAR                = ISO_I2C_IC5_TAR_reg,
    .IC_SAR                = ISO_I2C_IC5_SAR_reg,
    .IC_HS_MADDR           = ISO_I2C_IC5_HS_MADDR_reg,
    .IC_DATA_CMD           = ISO_I2C_IC5_DATA_CMD_reg,
    .IC_SS_SCL_HCNT        = ISO_I2C_IC5_SS_SCL_HCNT_reg,
    .IC_SS_SCL_LCNT        = ISO_I2C_IC5_SS_SCL_LCNT_reg,
    .IC_FS_SCL_HCNT        = ISO_I2C_IC5_FS_SCL_HCNT_reg,
    .IC_FS_SCL_LCNT        = ISO_I2C_IC5_FS_SCL_LCNT_reg,
    .IC_INTR_STAT          = ISO_I2C_IC5_INTR_STAT_reg,
    .IC_INTR_MASK          = ISO_I2C_IC5_INTR_MASK_reg,
    .IC_RAW_INTR_STAT      = ISO_I2C_IC5_RAW_INTR_STAT_reg,
    .IC_RX_TL              = ISO_I2C_IC5_RX_TL_reg,
    .IC_TX_TL              = ISO_I2C_IC5_TX_TL_reg,
    .IC_CLR_INTR           = ISO_I2C_IC5_CLR_INTR_reg,
    .IC_CLR_RX_UNDER       = ISO_I2C_IC5_CLR_RX_UNDER_reg,
    .IC_CLR_RX_OVER        = ISO_I2C_IC5_CLR_RX_OVER_reg,
    .IC_CLR_TX_OVER        = ISO_I2C_IC5_CLR_TX_OVER_reg,
    .IC_CLR_RD_REQ         = ISO_I2C_IC5_CLR_RD_REQ_reg,
    .IC_CLR_TX_ABRT        = ISO_I2C_IC5_CLR_TX_ABRT_reg,
    .IC_CLR_RX_DONE        = ISO_I2C_IC5_CLR_RX_DONE_reg,
    .IC_CLR_ACTIVITY       = ISO_I2C_IC5_CLR_ACTIVITY_reg,
    .IC_CLR_STOP_DET       = ISO_I2C_IC5_CLR_STOP_DET_reg,
    .IC_CLR_START_DET      = ISO_I2C_IC5_CLR_START_DET_reg,
    .IC_CLR_GEN_CALL       = ISO_I2C_IC5_CLR_GEN_CALL_reg,
    .IC_ENABLE             = ISO_I2C_IC5_ENABLE_reg,
    .IC_STATUS             = ISO_I2C_IC5_STATUS_reg,
    .IC_TXFLR              = ISO_I2C_IC5_TXFLR_reg,
    .IC_RXFLR              = ISO_I2C_IC5_RXFLR_reg,
    .IC_SDA_HOLD           = ISO_I2C_IC5_SDA_HOLD_reg,
    .IC_TX_ABRT_SOURCE     = ISO_I2C_IC5_TX_ABRT_SOURCE_reg,
    .IC_SLV_DATA_NACK      = ISO_I2C_IC5_SLV_DATA_NACK_ONLY_reg,
    .IC_SDA_SETUP          = ISO_I2C_IC5_SDA_SETUP_reg,
    .IC_ACK_GEN_CALL       = ISO_I2C_IC5_ACK_GENERAL_CALL_reg,
    .IC_ENABLE_STATUS      = ISO_I2C_IC5_ENABLE_STATUS_reg,
    .IC_COMP_PARAM_1       = ISO_I2C_IC5_COMP_PARAM_1_reg,
    .IC_COMP_VERSION       = ISO_I2C_IC5_COMP_VERSION_reg,
    .IC_COMP_TYPE          = ISO_I2C_IC5_COMP_TYPE_reg,
    .IC_SDA_DEL            = ISO_I2C_I2C5_SDA_OUTPUT_DELAY_CTRL_reg,
};

static const rtk_i2c_port RTK_I2C_i2c5_port[] =
{
	{
		.gpio_mapped       = 1,
		.g2c_scl           = rtk_gpio_id(ISO_GPIO, 6),
		.g2c_sda           = rtk_gpio_id(ISO_GPIO, 7),
		.input_mux[0]      = {0xb8060248, (0x3<<12), (0x0<<12)},
		.input_mux[1]      = {0xb8060248, (0x3<<8),  (0x0<<8)},
		.pin_mux[0]        = {0xb806020c, (0xF<<12), (0xB<<12), (0xF<<12)},
		.pin_mux[1]        = {0xb806020c, (0xF<<4),  (0xB<<4),  (0xF<<4)},
	},
	{
		.gpio_mapped       = 1,
		.g2c_scl           = rtk_gpio_id(ISO_GPIO, 43),
		.g2c_sda           = rtk_gpio_id(ISO_GPIO, 46),
		.input_mux[0]      = {0xb8060248, (0x3<<12), (0x1<<12)},
		.input_mux[1]      = {0xb8060248, (0x3<<8),  (0x1<<8)},
		.pin_mux[0]        = {0xb8060228, (0xF<<28), (0xB<<28), (0xF<<28)},
		.pin_mux[1]        = {0xb8060228, (0xF<<4),  (0xB<<4),  (0xF<<4)},
	},
	{
		.gpio_mapped       = 1,
		.g2c_scl           = rtk_gpio_id(ISO_GPIO, 24),
		.g2c_sda           = rtk_gpio_id(ISO_GPIO, 25),
		.input_mux[0]      = {0xb8060248, (0x3<<12), (0x2<<12)},
		.input_mux[1]      = {0xb8060248, (0x3<<8),  (0x2<<8)},
		.pin_mux[0]        = {0xb8060214, (0xF<<20), (0xB<<20), (0xF<<20)},
		.pin_mux[1]        = {0xb8060214, (0xF<<12), (0xB<<12), (0xF<<12)},
	},
};

static rtk_i2c_init_reg RTK_I2C_i2c5_init_reg[] =
{
    {ISO_I2C_I2C5_SDA_OUTPUT_DELAY_CTRL_reg, RTK_MIS_I2C5_SDA_OUTPUT_DELAY_databit_mask, ISO_I2C_I2C5_SDA_OUTPUT_DELAY_CTRL_databitdelaysel(0x07)},
    {ISO_I2C_IC5_SDA_SETUP_reg, ISO_I2C_IC5_SDA_SETUP_sda_setup_mask, ISO_I2C_IC5_SDA_SETUP_sda_setup(0x64)},
    {SYS_REG_INT_CTRL_SCPU_reg, (~0), (1 << RTK_I2C_ISO_MIS_OFF_SCPU_INT_EN_shift) | (1 << SYS_REG_INT_CTRL_SCPU_write_data_shift)},    // enable iso misc off SCPU Routing
    {ISO_MISC_OFF_SYSTEM_reg, ISO_MISC_OFF_SYSTEM_i2c5_scpu_and_8051_int_sel_mask, ISO_MISC_OFF_SYSTEM_i2c5_scpu_and_8051_int_sel(1)},  // sel i2c5 iso misc off SCPU Routing
    {ISO_MISC_OFF_SYSTEM_reg, ISO_MISC_OFF_SYSTEM_i2c5_scpu_and_8051_int_en_mask, ISO_MISC_OFF_SYSTEM_i2c5_scpu_and_8051_int_en(1)},    // en i2c5 iso misc off SCPU Routing
};

static rtk_i2c_phy i2c_phy[] =
{
    { &RTK_I2C_i2c0_reg, sizeof(RTK_I2C_i2c0_port)/sizeof(rtk_i2c_port), RTK_I2C_i2c0_port, sizeof(RTK_I2C_i2c0_init_reg)/sizeof(rtk_i2c_init_reg), RTK_I2C_i2c0_init_reg},
    { &RTK_I2C_i2c1_reg, sizeof(RTK_I2C_i2c1_port)/sizeof(rtk_i2c_port), RTK_I2C_i2c1_port, sizeof(RTK_I2C_i2c1_init_reg)/sizeof(rtk_i2c_init_reg), RTK_I2C_i2c1_init_reg},
    { &RTK_I2C_i2c2_reg, sizeof(RTK_I2C_i2c2_port)/sizeof(rtk_i2c_port), RTK_I2C_i2c2_port, sizeof(RTK_I2C_i2c2_init_reg)/sizeof(rtk_i2c_init_reg), RTK_I2C_i2c2_init_reg},
    { &RTK_I2C_i2c3_reg, sizeof(RTK_I2C_i2c3_port)/sizeof(rtk_i2c_port), RTK_I2C_i2c3_port, sizeof(RTK_I2C_i2c3_init_reg)/sizeof(rtk_i2c_init_reg), RTK_I2C_i2c3_init_reg},
    { &RTK_I2C_i2c4_reg, sizeof(RTK_I2C_i2c4_port)/sizeof(rtk_i2c_port), RTK_I2C_i2c4_port, sizeof(RTK_I2C_i2c4_init_reg)/sizeof(rtk_i2c_init_reg), RTK_I2C_i2c4_init_reg},
    { &RTK_I2C_i2c5_reg, sizeof(RTK_I2C_i2c5_port)/sizeof(rtk_i2c_port), RTK_I2C_i2c5_port, sizeof(RTK_I2C_i2c5_init_reg)/sizeof(rtk_i2c_init_reg), RTK_I2C_i2c5_init_reg},
};

#endif
