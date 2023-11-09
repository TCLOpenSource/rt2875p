#ifndef __RTK_I2C_CONFIG_H__
#define __RTK_I2C_CONFIG_H__

#include <rbus/iso_misc_off_reg.h>
#include <rbus/misc_reg.h>
#include <rbus/stb_reg.h>
#include <rbus/sys_reg_reg.h>
#include <rbus/pll27x_reg_reg.h>
#include <rbus/pll_reg_reg.h>
#include <rbus/misc_i2c_reg.h>
#include <rbus/iso_i2c_reg.h>
#include "rtk_i2c_priv.h"

#define RTK_I2C0_SCPU_INT_EN_shift                  SYS_REG_INT_CTRL_SCPU_standby_iso_misc_off_int_scpu_routing_en_shift
#define RTK_I2C1_SCPU_INT_EN_shift                  SYS_REG_INT_CTRL_SCPU_standby_iso_misc_int_scpu_routing_en_shift
#define RTK_I2C2_SCPU_INT_EN_shift                  SYS_REG_INT_CTRL_SCPU_standby_iso_misc_int_scpu_routing_en_shift

#define RTK_ISO_I2C0_SDA_OUTPUT_DELAY_databit_mask  (ISO_I2C_I2C0_SDA_OUTPUT_DELAY_CTRL_databitdelayenable_mask | ISO_I2C_I2C0_SDA_OUTPUT_DELAY_CTRL_databitdelaysel_mask)
#define RTK_MIS_I2C1_SDA_OUTPUT_DELAY_databit_mask  (MISC_I2C_I2C1_SDA_OUTPUT_DELAY_CTRL_databitdelayenable_mask | MISC_I2C_I2C1_SDA_OUTPUT_DELAY_CTRL_databitdelaysel_mask)
#define RTK_MIS_I2C2_SDA_OUTPUT_DELAY_databit_mask  (MISC_I2C_I2C2_SDA_OUTPUT_DELAY_CTRL_databitdelayenable_mask | MISC_I2C_I2C2_SDA_OUTPUT_DELAY_CTRL_databitdelaysel_mask)

#define I2C_GPIO_STATUS_GUARD_INT_shift             ISO_I2C_I2C0_GUARD_INTERVAL_STATUS_ic_guard_interval_status_shift   //(0)
#define I2C_GPIO_STATUS_GPIO_SCL_shift              ISO_I2C_I2C0_GUARD_INTERVAL_STATUS_gpi_ic_gpio_scl_shift            //(1)
#define I2C_GPIO_STATUS_GPIO_SDA_shift              ISO_I2C_I2C0_GUARD_INTERVAL_STATUS_gpi_ic_gpio_sda_shift            //(2)

#define I2C_GPIO_CTRL_SCL_shift                     ISO_I2C_I2C0_GPIO_MODE_CTRL_ic_gpio_scl_shift //(0)
#define I2C_GPIO_CTRL_SDA_shift                     ISO_I2C_I2C0_GPIO_MODE_CTRL_ic_gpio_sda_shift //(1)
#define I2C_GPIO_CTRL_SEL_shift                     ISO_I2C_I2C0_GPIO_MODE_CTRL_ic_gpio_sel_shift //(2)

#define  RTK_ISO_MISC_OFF_SYSTEM_i2c0_scpu_and_8051_int_sel_mask        (0x10000000)
#define  RTK_ISO_MISC_OFF_SYSTEM_i2c0_scpu_and_8051_int_en_mask         (0x04000000)
#define  RTK_ISO_MISC_OFF_SYSTEM_i2c0_scpu_and_8051_int_sel(data)       (0x10000000&((data)<<28))
#define  RTK_ISO_MISC_OFF_SYSTEM_i2c0_scpu_and_8051_int_en(data)        (0x04000000&((data)<<26))

//ISO_I2C_IC0_CON_reg
#define IC_SLAVE_DISABLE        (1 << ISO_I2C_IC0_CON_ic_slave_disable_shift)       // 6
#define IC_RESTART_EN           (1 << ISO_I2C_IC0_CON_ic_restart_en_shift)          // 5
#define IC_10BITADDR_MASTER     (1 << ISO_I2C_IC0_CON_ic_10bitaddr_master_rd_only_shift)   // 4
#define IC_10BITADDR_SLAVE      (1 << ISO_I2C_IC0_CON_ic_10bitaddr_slave_shift)     // 3
#define IC_SPEED_MASK           ISO_I2C_IC0_CON_speed_mask                          //(0x3 << 1)
#define SPEED_SS                ISO_I2C_IC0_CON_speed(0x1)                          //(0x1 << 1)
#define SPEED_FS                ISO_I2C_IC0_CON_speed(0x2)                          //(0x2 << 1)
#define SPEED_HS                ISO_I2C_IC0_CON_speed(0x3)                          //(0x3 << 1)
#define IC_MASTER_MODE          (1 << ISO_I2C_IC0_CON_master_mode_shift)            // 0

//ISO_I2C_I2C0_PH_CR_reg
#define PH_CR_EN                (1 << ISO_I2C_I2C0_PH_CR_ph_en_shift)               // 24
#define PH_CR_SDA_VTH(x)        ISO_I2C_I2C0_PH_CR_sda_vth(x)                       //(((x) << 20)) & 0xF)
#define PH_CR_SCL_VTH(x)        ISO_I2C_I2C0_PH_CR_scl_vth(x)                       //(((x) << 16)) & 0xF)
#define PH_CR_SDA_VALUE         (1 << ISO_I2C_I2C0_PH_CR_sda_value_shift)           // 8
#define PH_CR_SCL_VALUE         (1 << ISO_I2C_I2C0_PH_CR_scl_value_shift)           // 0

//ISO_I2C_IC0_DATA_CMD_reg
#define READ_CMD                (1 << ISO_I2C_IC0_DATA_CMD_cmd_shift)               // 8

//ISO_I2C_IC0_INTR_MASK_reg
#define GEN_CALL_BIT            (1 << ISO_I2C_IC0_INTR_MASK_m_gen_call_shift)       // 11
#define START_DET_BIT           (1 << ISO_I2C_IC0_INTR_MASK_m_start_det_shift)      // 10
#define STOP_DET_BIT            (1 << ISO_I2C_IC0_INTR_MASK_m_stop_det_shift)       // 9
#define ACTIVITY_BIT            (1 << ISO_I2C_IC0_INTR_MASK_m_activity_shift)       // 8
#define RX_DONE_BIT             (1 << ISO_I2C_IC0_INTR_MASK_m_rx_done_shift)        // 7
#define TX_ABRT_BIT             (1 << ISO_I2C_IC0_INTR_MASK_m_tx_abrt_shift)        // 6
#define RD_REQ_BIT              (1 << ISO_I2C_IC0_INTR_MASK_m_rd_req_shift)         // 5
#define TX_EMPTY_BIT            (1 << ISO_I2C_IC0_INTR_MASK_m_tx_empty_shift)       // 4
#define TX_OVER_BIT             (1 << ISO_I2C_IC0_INTR_MASK_m_tx_over_shift)        // 3
#define RX_FULL_BIT             (1 << ISO_I2C_IC0_INTR_MASK_m_rx_full_shift)        // 2
#define RX_OVER_BIT             (1 << ISO_I2C_IC0_INTR_MASK_m_rx_over_shift)        // 1
#define RX_UNDER_BIT            (1 << ISO_I2C_IC0_INTR_MASK_m_rx_under_shift)       // 0

//ISO_I2C_I2C0_GUARD_INTERVAL_STATUS_reg
#define GD_INTERVAL_ST_BIT      (1 << ISO_I2C_I2C0_GUARD_INTERVAL_STATUS_ic_guard_interval_status_shift)   // 0

//ISO_I2C_I2C0_GUARD_INTERVAL_CTRL_reg
#define GD_INTERVAL_SEL_BIT     (1 << ISO_I2C_I2C0_GUARD_INTERVAL_CTRL_ic_guard_interval_sel_shift)     // 5
#define GD_INTERVAL_SEL(x)      ISO_I2C_I2C0_GUARD_INTERVAL_CTRL_ic_guard_interval_sel(x)               //((x & 0x3) << 5)
#define GD_INTERVAL_INT_EN_BIT  (1 << ISO_I2C_I2C0_GUARD_INTERVAL_CTRL_ic_guard_interval_int_en_shift)  // 4
#define GD_INTERVAL_1000US      ISO_I2C_I2C0_GUARD_INTERVAL_CTRL_ic_guard_interval_delay_sel(0x0)       //(0x0)
#define GD_INTERVAL_4000US      ISO_I2C_I2C0_GUARD_INTERVAL_CTRL_ic_guard_interval_delay_sel(0x6)       //(0x6 << 1)
#define GD_INTERVAL_3000US      ISO_I2C_I2C0_GUARD_INTERVAL_CTRL_ic_guard_interval_delay_sel(0x5)       //(0x5 << 1)
#define GD_INTERVAL_2000US      ISO_I2C_I2C0_GUARD_INTERVAL_CTRL_ic_guard_interval_delay_sel(0x4)       //(0x4 << 1)
#define GD_INTERVAL_400US       ISO_I2C_I2C0_GUARD_INTERVAL_CTRL_ic_guard_interval_delay_sel(0x3)       //(0x3 << 1)
#define GD_INTERVAL_600US       ISO_I2C_I2C0_GUARD_INTERVAL_CTRL_ic_guard_interval_delay_sel(0x2)       //(0x2 << 1)
#define GD_INTERVAL_800US       ISO_I2C_I2C0_GUARD_INTERVAL_CTRL_ic_guard_interval_delay_sel(0x1)       //(0x1 << 1)
#define GD_INTERVAL_EN_BIT      (1 << ISO_I2C_I2C0_GUARD_INTERVAL_CTRL_ic_guard_interval_en_shift)      // 0
#define GD_INTERVAL_EEPROM      (GD_INTERVAL_INT_EN_BIT | GD_INTERVAL_2000US | GD_INTERVAL_EN_BIT)

//ISO_I2C_IC0_STATUS_reg
#define ST_RFF_BIT              (1 << ISO_I2C_IC0_STATUS_rff_shift)         // 4
#define ST_RFNE_BIT             (1 << ISO_I2C_IC0_STATUS_rfne_shift)        // 3
#define ST_TFE_BIT              (1 << ISO_I2C_IC0_STATUS_tfe_shift)         // 2
#define ST_TFNF_BIT             (1 << ISO_I2C_IC0_STATUS_tfnf_shift)        // 1
#define ST_ACTIVITY_BIT         (1 << ISO_I2C_IC0_STATUS_activity_shift)    // 0

//ISO_I2C_I2C0_SDA_OUTPUT_DELAY_CTRL_reg
#define I2C_SDA_DEL_MASK        ISO_I2C_I2C0_SDA_OUTPUT_DELAY_CTRL_databitdelaysel_mask             // (0xFF)
#define I2C_SDA_DEL_EN          (1 << ISO_I2C_I2C0_SDA_OUTPUT_DELAY_CTRL_databitdelayenable_shift)  // 15
#define I2C_SDA_DEL_SEL(x)      (x & (I2C_SDA_DEL_MASK|I2C_SDA_DEL_EN))

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
                .g2c_scl           = rtk_gpio_id(MIS_GPIO, 174),
                .g2c_sda           = rtk_gpio_id(MIS_GPIO, 173),
                .input_mux[0]      = {0xb8060240, (0x3<<24), (0x0<<24)},//SCL
                .input_mux[1]      = {0xb8060240, (0x3<<20), (0x0<<20)},//SDA
                .pin_mux[0]        = {0xb8000ca4, (0xF<<16), (0x6<<16), (0xF<<16)},
                .pin_mux[1]        = {0xb8000ca4, (0xF<<20), (0x6<<20), (0xF<<20)},
        },
        {
                .gpio_mapped       = 1,
                .g2c_scl           = rtk_gpio_id(ISO_GPIO, 07),
                .g2c_sda           = rtk_gpio_id(ISO_GPIO, 06),
                .input_mux[0]      = {0xB8060240, (0x3<<24), (0x1<<24)},
                .input_mux[1]      = {0xB8060240, (0x3<<20), (0x1<<20)},
                .pin_mux[0]        = {0xb8060204, (0xF<<4), (0x6<<4), (0xF<<4)},
                .pin_mux[1]        = {0xb8060204, (0xF<<12), (0x6<<12), (0xF<<12)},
        },
};

static rtk_i2c_init_reg RTK_I2C_i2c0_init_reg[] =
{
    {ISO_I2C_I2C0_SDA_OUTPUT_DELAY_CTRL_reg, RTK_ISO_I2C0_SDA_OUTPUT_DELAY_databit_mask, ISO_I2C_I2C0_SDA_OUTPUT_DELAY_CTRL_databitdelaysel(0x07)},
    {ISO_I2C_IC0_SDA_SETUP_reg, ISO_I2C_IC0_SDA_SETUP_sda_setup_mask, ISO_I2C_IC0_SDA_SETUP_sda_setup(0x64)},
    {SYS_REG_INT_CTRL_SCPU_reg, (~0), (1 << RTK_I2C0_SCPU_INT_EN_shift) | (1 << SYS_REG_INT_CTRL_SCPU_write_data_shift)},
    {ISO_MISC_OFF_SYSTEM_reg, RTK_ISO_MISC_OFF_SYSTEM_i2c0_scpu_and_8051_int_sel_mask, RTK_ISO_MISC_OFF_SYSTEM_i2c0_scpu_and_8051_int_sel(1)},
    {ISO_MISC_OFF_SYSTEM_reg, RTK_ISO_MISC_OFF_SYSTEM_i2c0_scpu_and_8051_int_en_mask, RTK_ISO_MISC_OFF_SYSTEM_i2c0_scpu_and_8051_int_en(1)},
};


static const rtk_i2c_reg_map RTK_I2C_i2c1_reg =
{
    .IC_ISR_EN             = SYS_REG_INT_CTRL_SCPU_reg,
    .IC_ISR_EN_MASK        = (1 << MISC_isr_i2c1_int_shift),
    .IC_ISR                = MISC_isr_reg,
    .IC_INT                = (1 << MISC_isr_i2c1_int_shift),
    .IC_INTERVAL_MODE_CTRL = MISC_I2C_I2C1_GUARD_INTERVAL_CTRL_reg,
    .IC_CLKSEL             = SYS_REG_SYS_CLKDIV_reg,
    .IC_CLKSEL_MASK        = (1 << SYS_REG_SYS_CLKDIV_i2c1_clksel_shift),
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
                .g2c_scl           = rtk_gpio_id(MIS_GPIO, 9),
                .g2c_sda           = rtk_gpio_id(MIS_GPIO, 8),
                .input_mux[0]      = {0xb80008c0, (0x3<<20), (0x0<<20)},
                .input_mux[1]      = {0xb80008c0, (0x3<<16), (0x0<<16)},
                .pin_mux[0]        = {0xb8060208,(0xF<<20),(0x6<<20), (0xF<<20)},
                .pin_mux[1]        = {0xb8060208,(0xF<<28),(0x6<<28), (0xF<<28)},
        },
        {
                .gpio_mapped       = 1,
                .g2c_scl           = rtk_gpio_id(MIS_GPIO, 118),
                .g2c_sda           = rtk_gpio_id(MIS_GPIO, 117),
                .input_mux[0]      = {0xb80008c0, (0x3<<20), (0x1<<20)},
                .input_mux[1]      = {0xb80008c0, (0x3<<16), (0x1<<16)},
                .pin_mux[0]        = {0xb8000cb8, (0xF<<20), (0x6<<20), (0xF<<20)},
                .pin_mux[1]        = {0xb8000cb8, (0xF<<24), (0x6<<24), (0xF<<24)},
        },
        {
                .gpio_mapped       = 1,
                .g2c_scl           = rtk_gpio_id(MIS_GPIO, 04),
                .g2c_sda           = rtk_gpio_id(MIS_GPIO, 03),
                .input_mux[0]      = {0xb80008c0, (0x3<<20), (0x2<<20)},
                .input_mux[1]      = {0xb80008c0, (0x3<<16), (0x2<<16)},
                .pin_mux[0]        = {0xb800081c, (0xF<<28), (0x6<<28), (0xF<<28)},
                .pin_mux[1]        = {0xb8000818, (0xF<<4), (0x6<<4), (0xF<<4)},
        },
};

static rtk_i2c_init_reg RTK_I2C_i2c1_init_reg[] =
{
    {MISC_I2C_I2C1_SDA_OUTPUT_DELAY_CTRL_reg, RTK_MIS_I2C1_SDA_OUTPUT_DELAY_databit_mask, MISC_I2C_I2C1_SDA_OUTPUT_DELAY_CTRL_databitdelaysel(0x07)},
    {MISC_I2C_IC1_SDA_SETUP_reg, MISC_I2C_IC1_SDA_SETUP_sda_setup_mask, MISC_I2C_IC1_SDA_SETUP_sda_setup(0x64)},
    {MISC_isr_switch_reg, MISC_isr_switch_i2c1_int_sel_mask, MISC_isr_switch_i2c1_int_sel(0)},  //select scpu int
};

static const rtk_i2c_reg_map RTK_I2C_i2c2_reg =
{
    .IC_ISR_EN             = SYS_REG_INT_CTRL_SCPU_reg,
    .IC_ISR_EN_MASK        = (1 << MISC_isr_i2c2_int_shift),
    .IC_ISR                = MISC_isr_reg,
    .IC_INT                = (1 << MISC_isr_i2c2_int_shift),
    .IC_CLKSEL             = SYS_REG_SYS_CLKDIV_reg,
    .IC_CLKSEL_MASK        = (1 << SYS_REG_SYS_CLKDIV_i2c2_clksel_shift),
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
                .g2c_scl           = rtk_gpio_id(MIS_GPIO, 15),
                .g2c_sda           = rtk_gpio_id(MIS_GPIO, 14),
                .input_mux[0]      = {0xb80008c0, (0x3<<12), (0x0<<12)},
                .input_mux[1]      = {0xb80008c0, (0x3<<8), (0x0<<8)},
                .pin_mux[0]        = {0xb8000824, (0xF<<4), (0x6<<4), (0xF<<4)},
                .pin_mux[1]        = {0xb8000824, (0xF<<12), (0x6<<12), (0xF<<12)},
        },
        {
                .gpio_mapped       = 1,
                .g2c_scl           = rtk_gpio_id(MIS_GPIO, 94),
                .g2c_sda           = rtk_gpio_id(MIS_GPIO, 93),
                .input_mux[0]      = {0xb80008c0, (0x3<<12), (0x1<<12)},
                .input_mux[1]      = {0xb80008c0, (0x3<<8), (0x1<<8)},
                .pin_mux[0]        = {0xb80008a8, (0xF<<20), (0x6<<20), (0xF<<20)},
                .pin_mux[1]        = {0xb80008a8, (0xF<<28), (0x6<<28), (0xF<<28)},
        },
        {
                .gpio_mapped       = 1,
                .g2c_scl           = rtk_gpio_id(MIS_GPIO, 92),
                .g2c_sda           = rtk_gpio_id(MIS_GPIO, 91),
                .input_mux[0]      = {0xb80008c0, (0x3<<12), (0x2<<12)},
                .input_mux[1]      = {0xb80008c0, (0x3<<8), (0x2<<8)},
                .pin_mux[0]        = {0xb80008a4, (0xF<<4), (0x6<<4), (0xF<<4)},
                .pin_mux[1]        = {0xb80008a4, (0xF<<12), (0x6<<12), (0xF<<12)},
        },
};

static rtk_i2c_init_reg RTK_I2C_i2c2_init_reg[] =
{
    {MISC_I2C_I2C2_SDA_OUTPUT_DELAY_CTRL_reg, RTK_MIS_I2C2_SDA_OUTPUT_DELAY_databit_mask, MISC_I2C_I2C2_SDA_OUTPUT_DELAY_CTRL_databitdelaysel(0x01)},
    {MISC_I2C_IC2_SDA_SETUP_reg, MISC_I2C_IC2_SDA_SETUP_sda_setup_mask, MISC_I2C_IC2_SDA_SETUP_sda_setup(0x64)},
    {MISC_isr_switch_reg, MISC_isr_switch_i2c2_int_sel_mask, MISC_isr_switch_i2c2_int_sel(0)},  //select scpu int
};

static rtk_i2c_phy i2c_phy[] =
{
    { &RTK_I2C_i2c0_reg, sizeof(RTK_I2C_i2c0_port)/sizeof(rtk_i2c_port), RTK_I2C_i2c0_port, sizeof(RTK_I2C_i2c0_init_reg)/sizeof(rtk_i2c_init_reg), RTK_I2C_i2c0_init_reg},
    { &RTK_I2C_i2c1_reg, sizeof(RTK_I2C_i2c1_port)/sizeof(rtk_i2c_port), RTK_I2C_i2c1_port, sizeof(RTK_I2C_i2c1_init_reg)/sizeof(rtk_i2c_init_reg), RTK_I2C_i2c1_init_reg},
    { &RTK_I2C_i2c2_reg, sizeof(RTK_I2C_i2c2_port)/sizeof(rtk_i2c_port), RTK_I2C_i2c2_port, sizeof(RTK_I2C_i2c2_init_reg)/sizeof(rtk_i2c_init_reg), RTK_I2C_i2c2_init_reg},
};

#endif


