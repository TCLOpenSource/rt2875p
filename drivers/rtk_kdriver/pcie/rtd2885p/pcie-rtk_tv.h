/******************************************************************************
*
*   Copyright(c) 2018 Realtek Semiconductor Corp. All rights reserved.
*
*   @author realtek.com
*
*   Definition about statndard PCIE spec
******************************************************************************/

#ifndef _PCIE_DUT_H_
#define _PCIE_DUT_H_

#include <rtd_log/rtd_module_log.h>
#define PCIE_DEV_NAME "PCIE"

/************************************************************************
 *  rtk log define
 ************************************************************************/
#define PCIE_EMERG(fmt, args...)      \
    rtd_pr_pcie_emerg(fmt , ## args)

#define PCIE_ALERT(fmt, args...)   \
    rtd_pr_pcie_alert(fmt , ## args)

#define PCIE_CRIT(fmt, args...)   \
    rtd_pr_pcie_crit(fmt , ## args)

#define PCIE_ERR(fmt, args...)   \
    rtd_pr_pcie_err(fmt , ## args)

#define PCIE_WARN(fmt, args...)   \
    rtd_pr_pcie_warn(fmt , ## args)

#define PCIE_NOTICE(fmt, args...)   \
    rtd_pr_pcie_notice(fmt , ## args)

#define PCIE_INFO(fmt, args...)   \
    rtd_pr_pcie_info(fmt , ## args)

#define PCIE_DEBUG(fmt, args...)   \
    rtd_pr_pcie_debug(fmt , ## args)

//#define RTK_PCIE_DEBUG
#ifdef RTK_PCIE_DEBUG
#define PCIE_FORCE(fmt, args...)   \
    PCIE_EMERG(fmt , ## args)
#else
#define PCIE_FORCE(fmt, args...)
#endif


// PCIE Configrations
#define CX_NFUNC 1
#define PCIE_CX_MAX_MTU_SEL (1)  // 0 : 128, 1: 256, 2: 512, 3:1024, 4: 2048, 5: 4096
#define PCIE_CX_MAX_MTU     (128<<PCIE_CX_MAX_MTU_SEL)

// LTSSM States
typedef enum  {
    S_DETECT_QUIET       =  0x00,
    S_DETECT_ACT         =  0x01,
    S_POLL_ACTIVE        =  0x02,
    S_POLL_COMPLIANCE    =  0x03,
    S_POLL_CONFIG        =  0x04,
    S_PRE_DETECT_QUIET   =  0x05,
    S_DETECT_WAIT        =  0x06,
    S_CFG_LINKWD_START   =  0x07,
    S_CFG_LINKWD_ACEPT   =  0x08,
    S_CFG_LANENUM_WAIT   =  0x09,
    S_CFG_LANENUM_ACEPT  =  0x0A,
    S_CFG_COMPLETE       =  0x0B,
    S_CFG_IDLE           =  0x0C,
    S_RCVRY_LOCK         =  0x0D,
    S_RCVRY_SPEED        =  0x0E,
    S_RCVRY_RCVRCFG      =  0x0F,
    S_RCVRY_IDLE         =  0x10,
    S_RCVRY_EQ0          =  0x20,
    S_RCVRY_EQ1          =  0x21,
    S_RCVRY_EQ2          =  0x22,
    S_RCVRY_EQ3          =  0x23,
    S_L0                 =  0x11,
    S_L0S                =  0x12,
    S_L123_SEND_EIDLE    =  0x13,
    S_L1_IDLE            =  0x14,
    S_L2_IDLE            =  0x15,
    S_L2_WAKE            =  0x16,
    S_DISABLED_ENTRY     =  0x17,
    S_DISABLED_IDLE      =  0x18,
    S_DISABLED           =  0x19,
    S_LPBK_ENTRY         =  0x1A,
    S_LPBK_ACTIVE        =  0x1B,
    S_LPBK_EXIT          =  0x1C,
    S_LPBK_EXIT_TIMEOUT  =  0x1D,
    S_HOT_RESET_ENTRY    =  0x1E,
    S_HOT_RESET          =  0x1F
} e_ltssm_state;

typedef enum {
    PCIE_CFG_TYPE0,
    PCIE_CFG_TYPE1
} e_pcie_cfgtype;

typedef enum {
    GEN1 = 1,
    GEN2 = 2,
    GEN3 = 3,
    GEN4 = 4,
    GEN5
} e_pciegen;

typedef enum {
    BUS_BIG_EDN    = 1,
    BUS_LITTLE_EDN = 2,
} e_bus_endian;

typedef struct pcie_reaq_ctx {
    u32 addr;
    u32 data;
    u8  is_read;
    u8  tlp_type;
    u8  resp;
    u8  np;
    u8  a_ep;
    u8  ns;
    u8  ro;
    u8  tc;
    u8  msg_code;
    u8  dbi_access;
    u8  hdr_3rd_dw;
    u8  hdr_4th_dw;
    u8  d_ep;
    u8  silentDrop;
    u8  atu_bypass;
    u8  p_tag;
} PCIE_REQ_CTX;




/* Pcie core register map *** */
#define PCIE_DEVICE_ID_VENDOR_ID_REG                                 0x0
#define PCIE_STATUS_COMMAND_REG                                      0x4
#define PCIE_CLASS_CODE_REVISION_ID                                  0x8
#define PCIE_BIST_HEADER_TYPE_LATENCY_CACHE_LINE_SIZE_REG            0xc
#define PCIE_BAR0_REG                                                0x10
#define PCIE_BAR1_REG                                                0x14
#define PCIE_BAR2_REG                                                0x18
#define PCIE_BAR3_REG                                                0x1c
#define PCIE_BAR4_REG                                                0x20
#define PCIE_BAR5_REG                                                0x24
#define PCIE_CARDBUS_CIS_PTR_REG                                     0x28
#define PCIE_SUBSYSTEM_ID_SUBSYSTEM_VENDOR_ID_REG                    0x2c
#define PCIE_EXP_ROM_BASE_ADDR_REG                                   0x30
#define PCIE_PCI_CAP_PTR_REG                                         0x34
#define PCIE_MAX_LATENCY_MIN_GRANT_INTERRUPT_PIN_INTERRUPT_LINE_REG  0x3c
#define PCIE_CAP_ID_NXT_PTR_REG                                      0x40
#define PCIE_CON_STATUS_REG                                          0x44
#define PCIE_PCI_MSI_CAP_ID_NEXT_CTRL_REG                            0x50
#define PCIE_MSI_CAP_OFF_04H_REG                                     0x54
#define PCIE_MSI_CAP_OFF_08H_REG                                     0x58
#define PCIE_MSI_CAP_OFF_0CH_REG                                     0x5c
#define PCIE_MSI_CAP_OFF_10H_REG                                     0x60
#define PCIE_MSI_CAP_OFF_14H_REG                                     0x64
#define PCIE_PCIE_CAP_ID_PCIE_NEXT_CAP_PTR_PCIE_CAP_REG              0x70
#define PCIE_DEVICE_CAPABILITIES_REG                                 0x74
#define PCIE_DEVICE_CONTROL_DEVICE_STATUS                            0x78
#define PCIE_LINK_CAPABILITIES_REG                                   0x7c
#define PCIE_LINK_CONTROL_LINK_STATUS_REG                            0x80
#define PCIE_DEVICE_CAPABILITIES2_REG                                0x94
#define PCIE_DEVICE_CONTROL2_DEVICE_STATUS2_REG                      0x98
#define PCIE_LINK_CAPABILITIES2_REG                                  0x9c
#define PCIE_LINK_CONTROL2_LINK_STATUS2_REG                          0xa0
#define PCIE_AER_EXT_CAP_HDR_OFF                                     0x100
#define PCIE_UNCORR_ERR_STATUS_OFF                                   0x104
#define PCIE_UNCORR_ERR_MASK_OFF                                     0x108
#define PCIE_UNCORR_ERR_SEV_OFF                                      0x10c
#define PCIE_CORR_ERR_STATUS_OFF                                     0x110
#define PCIE_CORR_ERR_MASK_OFF                                       0x114
#define PCIE_ADV_ERR_CAP_CTRL_OFF                                    0x118
#define PCIE_HDR_LOG_0_OFF                                           0x11c
#define PCIE_HDR_LOG_1_OFF                                           0x120
#define PCIE_HDR_LOG_2_OFF                                           0x124
#define PCIE_HDR_LOG_3_OFF                                           0x128
#define PCIE_TLP_PREFIX_LOG_1_OFF                                    0x138
#define PCIE_TLP_PREFIX_LOG_2_OFF                                    0x13c
#define PCIE_TLP_PREFIX_LOG_3_OFF                                    0x140
#define PCIE_TLP_PREFIX_LOG_4_OFF                                    0x144
#define PCIE_RAS_DES_CAP_HEADER_REG                                  0x148
#define PCIE_VENDOR_SPECIFIC_HEADER_REG                              0x14c
#define PCIE_EVENT_COUNTER_CONTROL_REG                               0x150
#define PCIE_EVENT_COUNTER_DATA_REG                                  0x154
#define PCIE_TIME_BASED_ANALYSIS_CONTROL_REG                         0x158
#define PCIE_TIME_BASED_ANALYSIS_DATA_REG                            0x15c
#define PCIE_EINJ_ENABLE_REG                                         0x178
#define PCIE_EINJ0_CRC_REG                                           0x17c
#define PCIE_EINJ1_SEQNUM_REG                                        0x180
#define PCIE_EINJ2_DLLP_REG                                          0x184
#define PCIE_EINJ3_SYMBOL_REG                                        0x188
#define PCIE_EINJ4_FC_REG                                            0x18c
#define PCIE_EINJ5_SP_TLP_REG                                        0x190
#define PCIE_EINJ6_COMPARE_POINT_H0_REG                              0x194
#define PCIE_EINJ6_COMPARE_POINT_H1_REG                              0x198
#define PCIE_EINJ6_COMPARE_POINT_H2_REG                              0x19c
#define PCIE_EINJ6_COMPARE_POINT_H3_REG                              0x1a0
#define PCIE_EINJ6_COMPARE_VALUE_H0_REG                              0x1a4
#define PCIE_EINJ6_COMPARE_VALUE_H1_REG                              0x1a8
#define PCIE_EINJ6_COMPARE_VALUE_H2_REG                              0x1ac
#define PCIE_EINJ6_COMPARE_VALUE_H3_REG                              0x1b0
#define PCIE_EINJ6_CHANGE_POINT_H0_REG                               0x1b4
#define PCIE_EINJ6_CHANGE_POINT_H1_REG                               0x1b8
#define PCIE_EINJ6_CHANGE_POINT_H2_REG                               0x1bc
#define PCIE_EINJ6_CHANGE_POINT_H3_REG                               0x1c0
#define PCIE_EINJ6_CHANGE_VALUE_H0_REG                               0x1c4
#define PCIE_EINJ6_CHANGE_VALUE_H1_REG                               0x1c8
#define PCIE_EINJ6_CHANGE_VALUE_H2_REG                               0x1cc
#define PCIE_EINJ6_CHANGE_VALUE_H3_REG                               0x1d0
#define PCIE_EINJ6_TLP_REG                                           0x1d4
#define PCIE_SD_CONTROL1_REG                                         0x1e8
#define PCIE_SD_CONTROL2_REG                                         0x1ec
#define PCIE_SD_STATUS_L1LANE_REG                                    0x1f8
#define PCIE_SD_STATUS_L1LTSSM_REG                                   0x1fc
#define PCIE_SD_STATUS_PM_REG                                        0x200
#define PCIE_SD_STATUS_L2_REG                                        0x204
#define PCIE_SD_STATUS_L3FC_REG                                      0x208
#define PCIE_SD_STATUS_L3_REG                                        0x20c
#define PCIE_ACK_LATENCY_TIMER_OFF                                   0x700
#define PCIE_VENDOR_SPEC_DLLP_OFF                                    0x704
#define PCIE_PORT_FORCE_OFF                                          0x708
#define PCIE_ACK_F_ASPM_CTRL_OFF                                     0x70c
#define PCIE_PORT_LINK_CTRL_OFF                                      0x710
#define PCIE_LANE_SKEW_OFF                                           0x714
#define PCIE_TIMER_CTRL_MAX_FUNC_NUM_OFF                             0x718
#define PCIE_SYMBOL_TIMER_FILTER_1_OFF                               0x71c
#define PCIE_FILTER_MASK_2_OFF                                       0x720
#define PCIE_AMBA_MUL_OB_DECOMP_NP_SUB_REQ_CTRL_OFF                  0x724
#define PCIE_PL_DEBUG0_OFF                                           0x728
#define PCIE_PL_DEBUG1_OFF                                           0x72c
#define PCIE_TX_P_FC_CREDIT_STATUS_OFF                               0x730
#define PCIE_TX_NP_FC_CREDIT_STATUS_OFF                              0x734
#define PCIE_TX_CPL_FC_CREDIT_STATUS_OFF                             0x738
#define PCIE_QUEUE_STATUS_OFF                                        0x73c
#define PCIE_VC_TX_ARBI_1_OFF                                        0x740
#define PCIE_VC_TX_ARBI_2_OFF                                        0x744
#define PCIE_VC0_P_RX_Q_CTRL_OFF                                     0x748
#define PCIE_VC0_NP_RX_Q_CTRL_OFF                                    0x74c
#define PCIE_VC0_CPL_RX_Q_CTRL_OFF                                   0x750
#define PCIE_GEN2_CTRL_OFF                                           0x80c  //definded @pcie-designware.h
#define PCIE_PHY_STATUS_OFF                                          0x810
#define PCIE_PHY_CONTROL_OFF                                         0x814
#define PCIE_TRGT_MAP_CTRL_OFF                                       0x81c
#define PCIE_CLOCK_GATING_CTRL_OFF                                   0x88c
#define PCIE_ORDER_RULE_CTRL_OFF                                     0x8b4
#define PCIE_PIPE_LOOPBACK_CONTROL_OFF                               0x8b8
//#define PCIE_MISC_CONTROL_1_OFF                                      0x8bc  //definded @pcie-designware.h
#define PCIE_MULTI_LANE_CONTROL_OFF                                  0x8c0
#define PCIE_PHY_INTEROP_CTRL_OFF                                    0x8c4
#define PCIE_TRGT_CPL_LUT_DELETE_ENTRY_OFF                           0x8c8
#define PCIE_LINK_FLUSH_CONTROL_OFF                                  0x8cc
#define PCIE_AMBA_ERROR_RESPONSE_DEFAULT_OFF                         0x8d0
#define PCIE_AMBA_LINK_TIMEOUT_OFF                                   0x8d4
#define PCIE_AMBA_ORDERING_CTRL_OFF                                  0x8d8
#define PCIE_COHERENCY_CONTROL_1_OFF                                 0x8e0
#define PCIE_COHERENCY_CONTROL_3_OFF                                 0x8e8
#define PCIE_AXI_MSTR_MSG_ADDR_LOW_OFF                               0x8f0
#define PCIE_AXI_MSTR_MSG_ADDR_HIGH_OFF                              0x8f4
#define PCIE_PCIE_VERSION_NUMBER_OFF                                 0x8f8
#define PCIE_PCIE_VERSION_TYPE_OFF                                   0x8fc
#define PCIE_AUX_CLK_FREQ_OFF                                        0xb40
#define PCIE_PIPE_RELATED_OFF                                        0xb90
#define PCIE_BAR0_MASK_REG                                           0x100010
#define PCIE_BAR1_MASK_REG                                           0x100014
#define PCIE_BAR2_MASK_REG                                           0x100018
#define PCIE_BAR3_MASK_REG                                           0x10001c
#define PCIE_BAR4_MASK_REG                                           0x100020
#define PCIE_BAR5_MASK_REG                                           0x100024
#define PCIE_EXP_ROM_BAR_MASK_REG                                    0x100030
#define PCIE_DMA_CTRL_DATA_ARB_PRIOR_OFF                             0x380000
#define PCIE_DMA_CTRL_OFF                                            0x380008
#define PCIE_DMA_WRITE_ENGINE_EN_OFF                                 0x38000c
#define PCIE_DMA_WRITE_DOORBELL_OFF                                  0x380010
#define PCIE_DMA_WRITE_CHANNEL_ARB_WEIGHT_LOW_OFF                    0x380018
#define PCIE_DMA_WRITE_CHANNEL_ARB_WEIGHT_HIGH_OFF                   0x38001c
#define PCIE_DMA_READ_ENGINE_EN_OFF                                  0x38002c
#define PCIE_DMA_READ_DOORBELL_OFF                                   0x380030
#define PCIE_DMA_READ_CHANNEL_ARB_WEIGHT_LOW_OFF                     0x380038
#define PCIE_DMA_READ_CHANNEL_ARB_WEIGHT_HIGH_OFF                    0x38003c
#define PCIE_DMA_WRITE_INT_STATUS_OFF                                0x38004c
#define PCIE_DMA_WRITE_INT_MASK_OFF                                  0x380054
#define PCIE_DMA_WRITE_INT_CLEAR_OFF                                 0x380058
#define PCIE_DMA_WRITE_ERR_STATUS_OFF                                0x38005c
#define PCIE_DMA_WRITE_DONE_IMWR_LOW_OFF                             0x380060
#define PCIE_DMA_WRITE_DONE_IMWR_HIGH_OFF                            0x380064
#define PCIE_DMA_WRITE_ABORT_IMWR_LOW_OFF                            0x380068
#define PCIE_DMA_WRITE_ABORT_IMWR_HIGH_OFF                           0x38006c
#define PCIE_DMA_WRITE_CH01_IMWR_DATA_OFF                            0x380070
#define PCIE_DMA_WRITE_CH23_IMWR_DATA_OFF                            0x380074
#define PCIE_DMA_WRITE_CH45_IMWR_DATA_OFF                            0x380078
#define PCIE_DMA_WRITE_CH67_IMWR_DATA_OFF                            0x38007c
#define PCIE_DMA_WRITE_LINKED_LIST_ERR_EN_OFF                        0x380090
#define PCIE_DMA_READ_INT_STATUS_OFF                                 0x3800a0
#define PCIE_DMA_READ_INT_MASK_OFF                                   0x3800a8
#define PCIE_DMA_READ_INT_CLEAR_OFF                                  0x3800ac
#define PCIE_DMA_READ_ERR_STATUS_LOW_OFF                             0x3800b4
#define PCIE_DMA_READ_ERR_STATUS_HIGH_OFF                            0x3800b8
#define PCIE_DMA_READ_LINKED_LIST_ERR_EN_OFF                         0x3800c4
#define PCIE_DMA_READ_DONE_IMWR_LOW_OFF                              0x3800cc
#define PCIE_DMA_READ_DONE_IMWR_HIGH_OFF                             0x3800d0
#define PCIE_DMA_READ_ABORT_IMWR_LOW_OFF                             0x3800d4
#define PCIE_DMA_READ_ABORT_IMWR_HIGH_OFF                            0x3800d8
#define PCIE_DMA_READ_CH01_IMWR_DATA_OFF                             0x3800dc
#define PCIE_DMA_READ_CH23_IMWR_DATA_OFF                             0x3800e0
#define PCIE_DMA_READ_CH45_IMWR_DATA_OFF                             0x3800e4
#define PCIE_DMA_READ_CH67_IMWR_DATA_OFF                             0x3800e8
#define PCIE_DMA_CH_CONTROL1_OFF_WRCH_i                              0x380200
#define PCIE_DMA_TRANSFER_SIZE_OFF_WRCH_i                            0x380208
#define PCIE_DMA_SAR_LOW_OFF_WRCH_i                                  0x38020c
#define PCIE_DMA_SAR_HIGH_OFF_WRCH_i                                 0x380210
#define PCIE_DMA_DAR_LOW_OFF_WRCH_i                                  0x380214
#define PCIE_DMA_DAR_HIGH_OFF_WRCH_i                                 0x380218
#define PCIE_DMA_LLP_LOW_OFF_WRCH_i                                  0x38021c
#define PCIE_DMA_LLP_HIGH_OFF_WRCH_i                                 0x380220
#define PCIE_DMA_CH_CONTROL1_OFF_RDCH_i                              0x380300
#define PCIE_DMA_TRANSFER_SIZE_OFF_RDCH_i                            0x380308
#define PCIE_DMA_SAR_LOW_OFF_RDCH_i                                  0x38030c
#define PCIE_DMA_SAR_HIGH_OFF_RDCH_i                                 0x380310
#define PCIE_DMA_DAR_LOW_OFF_RDCH_i                                  0x380314
#define PCIE_DMA_DAR_HIGH_OFF_RDCH_i                                 0x380318
#define PCIE_DMA_LLP_LOW_OFF_RDCH_i                                  0x38031c
#define PCIE_DMA_LLP_HIGH_OFF_RDCH_i                                 0x380320
#define PCIE_BAR0_MASK_REG                                           0x100010
#define PCIE_BAR1_MASK_REG                                           0x100014
#define PCIE_BAR2_MASK_REG                                           0x100018
#define PCIE_BAR3_MASK_REG                                           0x10001c
#define PCIE_BAR4_MASK_REG                                           0x100020
#define PCIE_BAR5_MASK_REG                                           0x100024
#define PCIE_EXP_ROM_BAR_MASK_REG                                    0x100030
/* Pcie core register map &&& */





/////////////////////////////////////////////
// PCI Type 0 header
/////////////////////////////////////////////
#define PCI_TYPE0_VENDOR_ID_POSITION 0
#define PCI_TYPE0_VENDOR_ID_MASK (0xffff << PCI_TYPE0_VENDOR_ID_POSITION)
#define PCI_TYPE0_DEVICE_ID_POSITION 16
#define PCI_TYPE0_DEVICE_ID_MASK (0xffff << PCI_TYPE0_VENDOR_ID_POSITION)

#define PCI_TYPE0_IO_EN_POSITION 0
#define PCI_TYPE0_IO_EN_MASK (1 << PCI_TYPE0_IO_EN_POSITION)

#define PCI_TYPE0_MSE_POSITION 1
#define PCI_TYPE0_MSE_MASK (1 << PCI_TYPE0_MSE_POSITION)

#define PCI_TYPE0_BME_POSITION 2
#define PCI_TYPE0_BME_MASK (1 << PCI_TYPE0_BME_POSITION)

#define PCI_TYPE0_SCO_POSITION 3
#define PCI_TYPE0_SCO_MASK (1 << PCI_TYPE0_SCO_POSITION)

#define PCI_TYPE0_MWI_EN_POSITION 4
#define PCI_TYPE0_MWI_EN_MASK (1 << PCI_TYPE0_MWI_EN_POSITION)

#define PCI_TYPE0_VGAPS_POSITION 5
#define PCI_TYPE0_VGAPS_MASK (1 << PCI_TYPE0_VGAPS_POSITION)

#define PCI_TYPE0_PERREN_POSITION 6
#define PCI_TYPE0_PERREN_MASK (1 << PCI_TYPE0_PERREN_POSITION)

#define PCI_TYPE0_IDSEL_POSITION 7
#define PCI_TYPE0_IDSEL_MASK (1 << PCI_TYPE0_IDSEL_POSITION)

#define PCI_TYPE0_SERREN_POSITION 8
#define PCI_TYPE0_SERREN_MASK (1 << PCI_TYPE0_SERREN_POSITION)

#define PCI_TYPE0_INT_EN_POSITION 10
#define PCI_TYPE0_INT_EN_MASK (1 << PCI_TYPE0_INT_EN_POSITION)

#define PCI_TYPE0_RESERV_POSITION 11
#define PCI_TYPE0_RESERV_MASK (0x1f << PCI_TYPE0_RESERV_POSITION)

#define PCI_TYPE0_IMM_READINESS_POSITION 16
#define PCI_TYPE0_IMM_READINESS_MASK (1<< PCI_TYPE0_IMM_READINESS_POSITION)

#define PCI_TYPE0_INT_STATUS_POSITION 19
#define PCI_TYPE0_INT_STATUS_MASK ( 1 << PCI_TYPE0_INT_STATUS_POSITION)

#define PCI_TYPE0_CAP_LIST_POSITION 20
#define PCI_TYPE0_CAP_LIST_MASK ( 1 << PCI_TYPE0_CAP_LIST_POSITION)

#define PCI_TYPE0_66MHZ_CAP_POSITION 21
#define PCI_TYPE0_66MHZ_CAP_MASK ( 1 << PCI_TYPE0_66MHZ_CAP_POSITION)

#define PCI_TYPE0_FAST_BB_CAP_POSITION 23
#define PCI_TYPE0_FAST_BB_CAP_MASK ( 1 << PCI_TYPE0_FAST_BB_CAP_POSITION)

#define PCI_TYPE0_MDPE_POSITION 24
#define PCI_TYPE0_MDPE_MASK ( 1 << PCI_TYPE0_MDPE_POSITION)

#define PCI_TYPE0_DEVSEL_TIMING_POSITION 25
#define PCI_TYPE0_DEVSEL_TIMING_MASK ( 0x3 << PCI_TYPE0_DEVSEL_TIMING_POSITION  )

#define PCI_TYPE0_SIG_TRGT_ABRT_POSITION 27
#define PCI_TYPE0_SIG_TRGT_ABRT_MASK ( 1 << PCI_TYPE0_SIG_TRGT_ABRT_POSITION)

#define PCI_TYPE0_RCVD_TRGT_ABRT_POSITION 28
#define PCI_TYPE0_RCVD_TRGT_ABRT_MASK ( 1 << PCI_TYPE0_RCVD_TRGT_ABRT_POSITION)

#define PCI_TYPE0_RCVD_MSTR_ABRT_POSITION 29
#define PCI_TYPE0_RCVD_MSTR_ABRT_MASK ( 1 << PCI_TYPE0_RCVD_MSTR_ABRT_POSITION)

#define PCI_TYPE0_SIG_SYS_ERR_POSITION 30
#define PCI_TYPE0_SIG_SYS_ERR_MASK ( 1 << PCI_TYPE0_SIG_SYS_ERR_POSITION)

#define PCI_TYPE0_DPE_POSITION 31
#define PCI_TYPE0_DPE_MASK ( 1 << PCI_TYPE0_DPE_POSITION)


#define PCI_TYPE0_REV_ID_POSITION 0
#define PCI_TYPE0_REV_ID_MASK ( 0xff << PCI_TYPE0_REV_ID_POSITION)

#define PCI_TYPE0_PROGRAM_INTERFACE_POSITION 8
#define PCI_TYPE0_PROGRAM_INTERFACE_MASK (0xff << PCI_TYPE0_PROGRAM_INTERFACE_POSITION)

#define PCI_TYPE0_SUBCLASS_CODE_POSITION 16
#define PCI_TYPE0_SUBCLASS_CODE_MASK (0xff << PCI_TYPE0_SUBCLASS_CODE_POSITION)

#define PCI_TYPE0_BASE_CLASS_CODE_POSITION 24
#define PCI_TYPE0_BASE_CLASS_CODE_MASK (0xff << PCI_TYPE0_BASE_CLASS_CODE_POSITION)

#define PCI_TYPE0_CACHE_LINE_SIZE_POSITION 0
#define PCI_TYPE0_CACHE_LINE_SIZE_MASK ( 0xff << PCI_TYPE0_CACHE_LINE_SIZE_POSITION)

#define PCI_TYPE0_LATENCY_MASTER_TIMER_POSITION 8
#define PCI_TYPE0_LATENCY_MASTER_TIMER_MASK ( 0xff << PCI_TYPE0_LATENCY_MASTER_TIMER_POSITION)

#define PCI_TYPE0_HEADER_TYPE_POSITION 16
#define PCI_TYPE0_HEADER_TYPE_MASK ( 0x7f << PCI_TYPE0_HEADER_TYPE_POSITION)

#define PCI_TYPE0_MULTI_FUNC_POSITION 23
#define PCI_TYPE0_MULTI_FUNC_MASK (0x1 << PCI_TYPE0_MULTI_FUNC_POSITION)


#define PCI_TYPE0_BIST_POSITION 24
#define PCI_TYPE0_BIST_MASK ( 0xff << PCI_TYPE0_BIST_POSITION)

#define PCI_TYPE0_BAR_IO_REQUEST_POSITION 0
#define PCI_TYPE0_BAR_IO_REQUEST_MASK (0x1 << PCI_TYPE0_BAR_IO_REQUEST_POSITION)
#define PCI_TYPE0_BAR_TYPE_POSITION 1
#define PCI_TYPE0_BAR_TYPE_MASK (0x3 << PCI_TYPE0_BAR_TYPE_POSITION)
#define PCI_TYPE0_BAR_PREFETCH_POSITION 3
#define PCI_TYPE0_BAR_PREFETCH_MASK (0x1 << PCI_TYPE0_BAR_PREFETCH_POSITION)
#define PCI_TYPE0_BAR_START_POSITION 4
#define PCI_TYPE0_BAR_START_MASK ( 0xfffffff << PCI_TYPE0_BAR_START_POSITION)

#define PCI_TYPE0_BAR0_ENABLED_POSITION 0
#define PCI_TYPE0_BAR0_ENABLED_MASK ( 0x1 << PCI_TYPE0_BAR0_ENABLED_POSITION )
#define PCI_TYPE0_BAR0_MASK_POSITION 1
#define PCI_TYPE0_BAR0_MASK_MASK (0x7fffffff << PCI_TYPE0_BAR0_MASK_POSITION)

#define PCI_TYPE0_BAR1_ENABLED_POSITION 0
#define PCI_TYPE0_BAR1_ENABLED_MASK ( 0x1 << PCI_TYPE0_BAR1_ENABLED_POSITION )
#define PCI_TYPE0_BAR1_MASK_POSITION 1
#define PCI_TYPE0_BAR1_MASK_MASK (0x7fffffff << PCI_TYPE0_BAR1_MASK_POSITION)

#define PCI_TYPE0_BAR2_ENABLED_POSITION 0
#define PCI_TYPE0_BAR2_ENABLED_MASK ( 0x1 << PCI_TYPE0_BAR2_ENABLED_POSITION )
#define PCI_TYPE0_BAR2_MASK_POSITION 1
#define PCI_TYPE0_BAR2_MASK_MASK (0x7fffffff << PCI_TYPE0_BAR2_MASK_POSITION)

#define PCI_TYPE0_BAR3_ENABLED_POSITION 0
#define PCI_TYPE0_BAR3_ENABLED_MASK ( 0x1 << PCI_TYPE0_BAR3_ENABLED_POSITION )
#define PCI_TYPE0_BAR3_MASK_POSITION 1
#define PCI_TYPE0_BAR3_MASK_MASK (0x7fffffff << PCI_TYPE0_BAR3_MASK_POSITION)

#define PCI_TYPE0_BAR4_ENABLED_POSITION 0
#define PCI_TYPE0_BAR4_ENABLED_MASK ( 0x1 << PCI_TYPE0_BAR4_ENABLED_POSITION )
#define PCI_TYPE0_BAR4_MASK_POSITION 1
#define PCI_TYPE0_BAR4_MASK_MASK (0x7fffffff << PCI_TYPE0_BAR4_MASK_POSITION)

#define PCI_TYPE0_BAR5_ENABLED_POSITION 0
#define PCI_TYPE0_BAR5_ENABLED_MASK ( 0x1 << PCI_TYPE0_BAR5_ENABLED_POSITION )
#define PCI_TYPE0_BAR5_MASK_POSITION 1
#define PCI_TYPE0_BAR5_MASK_MASK (0x7fffffff << PCI_TYPE0_BAR5_MASK_POSITION)

#define PCI_TYPE0_CARD_CIS_PTR_POSITION 0
#define PCI_TYPE0_CARD_CIS_PTR_MASK (0xffffffff << PCI_TYPE0_CARD_CIS_PTR_POSITION)

#define PCI_TYPE0_SUBSYS_VENDOR_ID_POSITION 0
#define PCI_TYPE0_SUBSYS_VENDOR_ID_MASK (0xffff << PCI_TYPE0_SUBSYS_VENDOR_ID_POSITION)

#define PCI_TYPE0_SUBSYS_DEV_ID_POSITION 16
#define PCI_TYPE0_SUBSYS_DEV_ID_MASK (0xffff << PCI_TYPE0_SUBSYS_DEV_ID_POSITION)

#define PCI_TYPE0_ROM_BAR_ENABLE_POSITION 0
#define PCI_TYPE0_ROM_BAR_ENABLE_MASK ( 1 << PCI_TYPE0_ROM_BAR_ENABLE_POSITION)


#define PCI_TYPE0_EXP_ROM_BASE_POSITION 11
#define PCI_TYPE0_EXP_ROM_BASE_MASK (0x1fffff << PCI_TYPE0_EXP_ROM_BASE_POSITION)

#define PCI_TYPE0_ROM_BAR_ENABLED_POSITION 0
#define PCI_TYPE0_ROM_BAR_ENABLED_MASK ( 1 << PCI_TYPE0_ROM_BAR_ENABLED_POSITION)

#define PCI_TYPE0_ROM_MASK_POSITION 1
#define PCI_TYPE0_ROM_MASK_MASK (0x7fffffff << PCI_TYPE0_ROM_MASK_POSITION )

#define PCI_TYPE0_CAP_POINTER_POSITION 0
#define PCI_TYPE0_CAP_POINTER_MASK ( 0xff << PCI_TYPE0_CAP_POINTER_POSITION)

#define PCI_TYPE0_INT_LINE_POSITION 0
#define PCI_TYPE0_INT_LINE_MASK ( 0xff << PCI_TYPE0_INT_LINE_POSITION)

#define PCI_TYPE0_INT_PIN_POSITION 8
#define PCI_TYPE0_INT_PIN_MASK ( 0xff << PCI_TYPE0_INT_PIN_POSITION)


#define PCI_TYPE1_PRIM_BUS_POSITION 0
#define PCI_TYPE1_PRIM_BUS_MASK ( 0xff << PCI_TYPE1_PRIM_BUS_POSITION)

#define PCI_TYPE1_SEC_BUS_POSITION 8
#define PCI_TYPE1_SEC_BUS_MASK ( 0xff << PCI_TYPE1_SEC_BUS_POSITION)

#define PCI_TYPE1_SUB_BUS_POSITION 16
#define PCI_TYPE1_SUB_BUS_MASK ( 0xff << PCI_TYPE1_SUB_BUS_POSITION)

#define PCI_TYPE1_SEC_LAT_TIMER_POSITION 24
#define PCI_TYPE1_SEC_LAT_TIMER_MASK ( 0xff << PCI_TYPE1_SEC_LAT_TIMER_POSITION)

#define PCI_TYPE1_IO_DECODE_POSITION 0
#define PCI_TYPE1_IO_DECODE_MASK (0x1 << PCI_TYPE1_IO_DECODE_POSITION)

#define PCI_TYPE1_IO_RESERV_POSITION 1
#define PCI_TYPE1_IO_RESERV_MASK (0x7 << PCI_TYPE1_IO_RESERV_POSITION)

#define PCI_TYPE1_IO_BASE_POSITION 4
#define PCI_TYPE1_IO_BASE_MASK (0xf << PCI_TYPE1_IO_BASE_POSITION)

#define PCI_TYPE1_IO_DECODE_BIT8_POSITION 8
#define PCI_TYPE1_IO_DECODE_BIT8_MASK (0x1 << PCI_TYPE1_IO_DECODE_BIT8_POSITION)
#define PCI_TYPE1_IO_RESERV1_POSITION 9
#define PCI_TYPE1_IO_RESERV1_MASK (0x7 << PCI_TYPE1_IO_RESERV1_POSITION)

#define PCI_TYPE1_IO_LIMIT_POSITION 12
#define PCI_TYPE1_IO_LIMIT_MASK (0xf << PCI_TYPE1_IO_LIMIT_POSITION)

#define PCI_TYPE1_SEC_STAT_RESERV_POSITION 16
#define PCI_TYPE1_SEC_STAT_RESERV_MASK ( 0x7f << PCI_TYPE1_SEC_STAT_RESERV_POSITION)

#define PCI_TYPE1_SEC_STAT_MDPE_POSITION 24
#define PCI_TYPE1_SEC_STAT_MDPE_MASK (0x1 << PCI_TYPE1_SEC_STAT_MDPE_POSITION)

//#define PCI_TYPE1_SEC_STAT_DEV_SEL_POSITION 25
//#define PCI_TYPE1_SEC_STAT_DEV_SEL_MASK ( 'h3 << PCI_TYPE1_SEC_STAT_DEV_SEL_POSITION)

#define PCI_TYPE1_SEC_STAT_SIG_TRGT_ABRT_POSITION 27
#define PCI_TYPE1_SEC_STAT_SIG_TRGT_ABRT_MASK (0x1 << PCI_TYPE1_SEC_STAT_SIG_TRGT_ABRT_POSITION)

#define PCI_TYPE1_SEC_STAT_RCVD_TRGT_ABRT_POSITION 28
#define PCI_TYPE1_SEC_STAT_RCVD_TRGT_ABRT_MASK (0x1 << PCI_TYPE1_SEC_STAT_RCVD_TRGT_ABRT_POSITION)

#define PCI_TYPE1_SEC_STAT_RCVD_MSTR_ABRT_POSITION 29
#define PCI_TYPE1_SEC_STAT_RCVD_MSTR_ABRT_MASK (0x1 << PCI_TYPE1_SEC_STAT_RCVD_MSTR_ABRT_POSITION)

#define PCI_TYPE1_SEC_STAT_RCVD_SYS_ERR_POSITION 30
#define PCI_TYPE1_SEC_STAT_RCVD_SYS_ERR_MASK (0x1 << PCI_TYPE1_SEC_STAT_RCVD_SYS_ERR_POSITION)

#define PCI_TYPE1_SEC_STAT_DPE_POSITION 31
#define PCI_TYPE1_SEC_STAT_DPE_MASK (0x1 << PCI_TYPE1_SEC_STAT_DPE_POSITION)

#define PCI_TYPE1_MEM_BASE_RESERV_POSITION 0
#define PCI_TYPE1_MEM_BASE_RESERV_MASK ( 0xf << PCI_TYPE1_MEM_BASE_RESERV_POSITION)

#define PCI_TYPE1_MEM_BASE_POSITION 4
#define PCI_TYPE1_MEM_BASE_MASK (0xfff << PCI_TYPE1_MEM_BASE_POSITION)

#define PCI_TYPE1_MEM_LIMIT_RESERV_POSITION 16
#define PCI_TYPE1_MEM_LIMIT_RESERV_MASK ( 0xf << PCI_TYPE1_MEM_LIMIT_RESERV_POSITION)

#define PCI_TYPE1_MEM_LIMIT_POSITION 20
#define PCI_TYPE1_MEM_LIMIT_MASK (0xfff << PCI_TYPE1_MEM_LIMIT_POSITION)

#define PCI_TYPE1_PREF_MEM_DECODE_POSITION 0
#define PCI_TYPE1_PREF_MEM_DECODE_MASK (0x1 << PCI_TYPE1_PREF_MEM_DECODE_POSITION)

#define PCI_TYPE1_PREF_RESERV_POSITION 1
#define PCI_TYPE1_PREF_RESERV_MASK (0x7 << PCI_TYPE1_PREF_RESERV_POSITION)

#define PCI_TYPE1_PREF_MEM_BASE_POSITION 4
#define PCI_TYPE1_PREF_MEM_BASE_MASK (0xfff << PCI_TYPE1_PREF_MEM_BASE_POSITION)


#define PCI_TYPE1_PREF_MEM_LIMIT_DECODE_POSITION 16
#define PCI_TYPE1_PREF_MEM_LIMIT_DECODE_MASK (0x1 << PCI_TYPE1_PREF_MEM_LIMIT_DECODE_POSITION)

#define PCI_TYPE1_PREF_RESERV1_POSITION 17
#define PCI_TYPE1_PREF_RESERV1_MASK (0x7 << PCI_TYPE1_PREF_RESERV1_POSITION)

#define PCI_TYPE1_PREF_MEM_LIMIT_POSITION 20
#define PCI_TYPE1_PREF_MEM_LIMIT_MASK (0xfff << PCI_TYPE1_PREF_MEM_LIMIT_POSITION)


#define PCI_TYPE1_PREF_MEM_BASE_UPPER_POSITION 0
#define PCI_TYPE1_PREF_MEM_BASE_UPPER_MASK (0xffffffff << PCI_TYPE1_PREF_MEM_BASE_UPPER_POSITION)

#define PCI_TYPE1_PREF_MEM_LIMIT_UPPER_POSITION 0
#define PCI_TYPE1_PREF_MEM_LIMIT_UPPER_MASK (0xffffffff << PCI_TYPE1_PREF_MEM_LIMIT_UPPER_POSITION)

#define PCI_TYPE1_IO_BASE_UPPER_POSITION 0
#define PCI_TYPE1_IO_BASE_UPPER_MASK (0xffff << PCI_TYPE1_IO_BASE_UPPER_POSITION)

#define PCI_TYPE1_IO_LIMIT_UPPER_POSITION 16
#define PCI_TYPE1_IO_LIMIT_UPPER_MASK (0xffff << PCI_TYPE1_IO_LIMIT_UPPER_POSITION)

#define PCI_TYPE1_INT_LINE_POSITION 0
#define PCI_TYPE1_INT_LINE_MASK  (0xff << PCI_TYPE1_INT_LINE_POSITION)

#define PCI_TYPE1_INT_PIN_POSITION 8
#define PCI_TYPE1_INT_PIN_MASK  (0xff << PCI_TYPE1_INT_PIN_POSITION)

#define PCI_TYPE1_PERE_POSITION 16
#define PCI_TYPE1_PERE_MASK (0x1 << PCI_TYPE1_PERE_POSITION)

#define PCI_TYPE1_SERR_EN_POSITION 17
#define PCI_TYPE1_SERR_EN_MASK (0x1 << PCI_TYPE1_SERR_EN_POSITION)

#define PCI_TYPE1_ISA_EN_POSITION 18
#define PCI_TYPE1_ISA_EN_MASK (0x1 << PCI_TYPE1_ISA_EN_POSITION)

#define PCI_TYPE1_VGA_EN_POSITION 19
#define PCI_TYPE1_VGA_EN_MASK (0x1 << PCI_TYPE1_VGA_EN_POSITION)

#define PCI_TYPE1_VGA_16B_DEC_POSITION 20
#define PCI_TYPE1_VGA_16B_DEC_MASK (0x1 << PCI_TYPE1_VGA_16B_DEC_POSITION)

#define PCI_TYPE1_MSTR_ABORT_MODE_POSITION 21
#define PCI_TYPE1_MSTR_ABORT_MODE_MASK (0x1 << PCI_TYPE1_MSTR_ABORT_MODE_POSITION)


#define PCI_TYPE1_SBR_POSITION 22
#define PCI_TYPE1_SBR_MASK (0x1 << PCI_TYPE1_SBR_POSITION)

#define PCI_TYPE1_BRIDGE_CTRL_RESERV_POSITION 23
#define PCI_TYPE1_BRIDGE_CTRL_RESERV_MASK ( 0x1ff << PCI_TYPE1_BRIDGE_CTRL_RESERV_POSITION)


/* this did n't work when I tried to use the macro ??

#define CREATE_PCI_TYPE0( name, pos, width) \
  #define PCI_TYPE0_`name``_POSTION pos \
  #define PCI_TYPE0_`name``_MASK ( width << `PCI_TYPE0_``name``_POSTION)

CREATE_PCI_TYPE0( FAST_BB_CAP, 7, 1)
*/

/////////////////////////////////////////////
// SR-IOV cap structure
////////////////////////////////////////////
#define SRIOV_PCIE_EXTENDED_CAP_ID_POSITION 0
#define SRIOV_CAP_VERSION_POSITION  16
#define SRIOV_NEXT_OFFSET_POSITION  20

#define SRIOV_PCIE_EXTENDED_CAP_ID_MASK 320x0000_ffff
#define SRIOV_CAP_VERSION_MASK (0xf << SRIOV_CAP_VERSION_POSITION)
#define SRIOV_NEXT_OFFSET_MASK ( 120xfff << SRIOV_NEXT_OFFSET_POSITION)

#define SRIOV_CAPABILITIES_POSITION 0
#define SRIOV_CAPABILITIES_MASK 320xffff_ffff
#define SRIOV_VF_MIGRATION_CAP_POSITION 0
#define SRIOV_VF_MIGRATION_CAP_MASK 0x1
#define SRIOV_ARI_CAP_HIER_PRESERVED_POSITION 1
#define SRIOV_ARI_CAP_HIER_PRESERVED_MASK  (1 << SRIOV_ARI_CAP_HIER_PRESERVED_POSITION)
#define SRIOV_VF_10BIT_TAG_REQ_CAP_POSITION 2
#define SRIOV_VF_10BIT_TAG_REQ_CAP_MASK (1 << SRIOV_VF_10BIT_TAG_REQ_CAP_POSITION)
#define SRIOV_VF_MIGRATION_INT_MSG_NUM_POSITION 21
#define SRIOV_VF_MIGRATION_INT_MSG_NUM_MASK (0x3ff << SRIOV_VF_MIGRATION_INT_MSG_NUM_POSITION)

#define SRIOV_VF_ENABLE_POSITION 0
#define SRIOV_VF_ENABLE_MASK ( 1 << SRIOV_VF_ENABLE_POSITION)
#define SRIOV_VF_MIGRATION_ENABLE_POSITION 1
#define SRIOV_VF_MIGRATION_ENABLE_MASK (1 << SRIOV_VF_MIGRATION_ENABLE_POSITION)
#define SRIOV_VF_MIGRATION_INT_ENABLE_POSITION 2
#define SRIOV_VF_MIGRATION_INT_ENABLE_MASK (1 << SRIOV_VF_MIGRATION_INT_ENABLE_POSITION)
#define SRIOV_VF_MSE_POSITION 3
#define SRIOV_VF_MSE_MASK (1 << SRIOV_VF_MSE_POSITION)
#define SRIOV_ARI_CAPABLE_HIER_POSITION 4
#define SRIOV_ARI_CAPABLE_HIER_MASK (1<< SRIOV_ARI_CAPABLE_HIER_POSITION)
#define SRIOV_VF_10BIT_TAG_REQ_ENABLE_POSITION 5
#define SRIOV_VF_10BIT_TAG_REQ_ENABLE_MASK (1 << SRIOV_VF_10BIT_TAG_REQ_ENABLE_POSITION)

#define SRIOV_VF_MIGRATION_STATUS_POSITION 16
#define SRIOV_VF_MIGRATION_STATUS_MASK (1<<SRIOV_VF_MIGRATION_STATUS_POSITION)

#define SRIOV_INITIAL_VFS_POSITION 0
#define SRIOV_INITIAL_VFS_MASK (0xffff << SRIOV_INITIAL_VFS_POSITION)
#define SRIOV_TOTAL_VFS_POSITION 16
#define SRIOV_TOTAL_VFS_MASK (0xffff << SRIOV_TOTAL_VFS_POSITION)

#define SHADOW_SRIOV_INITIAL_VFS_POSITION 0
#define SHADOW_SRIOV_INITIAL_VFS_MASK (0xffff << SHADOW_SRIOV_INITIAL_VFS_POSITION)
#define SHADOW_SRIOV_TOTAL_VFS_POSITION 16
#define SHADOW_SRIOV_TOTAL_VFS_MASK (0xffff << SHADOW_SRIOV_TOTAL_VFS_POSITION)

#define SRIOV_NUM_VFS_POSITION 0
#define SRIOV_NUM_VFS_MASK (0xffff << SRIOV_NUM_VFS_POSITION)
#define SRIOV_FDL_POSITION 16
#define SRIOV_FDL_MASK (0xff << SRIOV_FDL_POSITION)

#define SRIOV_VF_OFFSET_POSITION 0
#define SRIOV_VF_OFFSET_MASK (0xffff << SRIOV_VF_OFFSET_POSITION)
#define SRIOV_VF_STRIDE_POSITION 16
#define SRIOV_VF_STRIDE_MASK (0xffff << SRIOV_VF_STRIDE_POSITION)

#define SHADOW_SRIOV_VF_STRIDE_POSITION 0
#define SHADOW_SRIOV_VF_STRIDE_MASK (0xffff <<SHADOW_SRIOV_VF_STRIDE_POSITION )
#define SHADOW_SRIOV_VF_OFFSET_POSITION 16
#define SHADOW_SRIOV_VF_OFFSET_MASK (0xffff << SHADOW_SRIOV_VF_OFFSET_POSITION)

#define SRIOV_VF_DEVICE_ID_POSITION 16
#define SRIOV_VF_DEVICE_ID_MASK (0xffff << SRIOV_VF_DEVICE_ID_POSITION)

#define SRIOV_SUP_PAGE_SIZE_POSITION 0
#define SRIOV_SUP_PAGE_SIZE_MASK (0xffffffff << SRIOV_SUP_PAGE_SIZE_POSITION)

#define SRIOV_SYSTEM_PAGE_SIZE_POSITION 0
#define SRIOV_SYSTEM_PAGE_SIZE_MASK (0xffffffff << SRIOV_SYSTEM_PAGE_SIZE_POSITION)

#define SRIOV_VF_BAR_RSVDP_POSITION 0
#define SRIOV_VF_BAR_RSVDP_MASK (0x1 << SRIOV_VF_BAR_RSVDP_POSITION)
#define SRIOV_VF_BAR_IO_REQUEST_POSITION 0
#define SRIOV_VF_BAR_IO_REQUEST_MASK (1 << SRIOV_VF_BAR_IO_REQUEST_POSITION)
#define SRIOV_VF_BAR_TYPE_POSITION 1
#define SRIOV_VF_BAR_TYPE_MASK (0x3 << SRIOV_VF_BAR_TYPE_POSITION)
#define SRIOV_VF_BAR_PREFETCH_POSITION 3
#define SRIOV_VF_BAR_PREFETCH_MASK (1 << SRIOV_VF_BAR_PREFETCH_POSITION)
#define SRIOV_VF_BAR_START_POSITION 4
#define SRIOV_VF_BAR_START_MASK ( 0xfffffff << SRIOV_VF_BAR_START_POSITION)

#define SRIOV_VF_MIGRATION_STATE_BIR_POSITION 0
#define SRIOV_VF_MIGRATION_STATE_BIR_MASK ( 0x7 << SRIOV_VF_MIGRATION_STATE_BIR_POSITION)
#define SRIOV_VF_MIGRATION_STATE_OFFSET_POSITION 3
#define SRIOV_VF_MIGRATION_STATE_OFFSET_MASK ( 0x1fffffff << SRIOV_VF_MIGRATION_STATE_OFFSET_POSITION)

/////////////////////////////////////////////
// VC Cap Structure
////////////////////////////////////////////

#define VC_PCIE_EXTENDED_CAP_ID_POSITION 0
#define VC_CAP_VERSION_POSITION  16
#define VC_NEXT_OFFSET_POSITION  20

#define VC_PCIE_EXTENDED_CAP_ID_MASK 0xffff
#define VC_CAP_VERSION_MASK  (0xf << VC_CAP_VERSION_POSITION)
#define VC_NEXT_OFFSET_MASK  (0xfff << VC_NEXT_OFFSET_POSITION)

#define VC_EXT_VC_CNT_POSITION 0
#define VC_EXT_VC_CNT_MASK (0x7 << VC_EXT_VC_CNT_POSITION)
#define VC_LOW_PRI_EXT_VC_CNT_POSITION 4
#define VC_LOW_PRI_EXT_VC_CNT_MASK (0x7 << VC_LOW_PRI_EXT_VC_CNT_POSITION)
#define VC_REFERENCE_CLOCK_POSITION 8
#define VC_REFERENCE_CLOCK_MASK (0x3 << VC_REFERENCE_CLOCK_POSITION)
#define VC_PORT_ARBI_TBL_ENTRY_SIZE_POSITION 10
#define VC_PORT_ARBI_TBL_ENTRY_SIZE_MASK (0x3 << VC_PORT_ARBI_TBL_ENTRY_SIZE_POSITION)

#define VC_ARBI_CAP_POSITION 0
#define VC_ARBI_CAP_MASK (0xf << VC_ARBI_CAP_POSITION)
#define VC_ARBI_TABLE_OFFSET_POSITION 24
#define VC_ARBI_TABLE_OFFSET_MASK (0xff << VC_ARBI_TABLE_OFFSET_POSITION)

#define VC_LOAD_VC_ARBI_TABLE_POSITION 0
#define VC_LOAD_VC_ARBI_TABLE_MASK (0x1 << VC_LOAD_VC_ARBI_TABLE_POSITION)
#define VC_ARBI_SELECT_POSITION 1
#define VC_ARBI_SELECT_MASK (0x7 << VC_ARBI_SELECT_POSITION)

#define VC_ARBI_TABLE_STATUS_POSITION 16
#define VC_ARBI_TABLE_STATUS_MASK (0x1 << VC_ARBI_TABLE_STATUS_POSITION)

#define VC_PORT_ARBI_CAP_VC0_POSITION 0
#define VC_PORT_ARBI_CAP_VC0_MASK (0xff << VC_PORT_ARBI_CAP_VC0_POSITION)
#define UNDEFINED_PCIEXPRESS_1_1_VC0_POSITION 14
#define UNDEFINED_PCIEXPRESS_1_1_VC0_MASK (0x1 << UNDEFINED_PCIEXPRESS_1_1_VC0_POSITION)
#define VC_REJECT_SNOOP_TRANS_VC0_POSITION 15
#define VC_REJECT_SNOOP_TRANS_VC0_MASK (0x1 << VC_REJECT_SNOOP_TRANS_VC0_POSITION)
#define VC_MAX_TIME_SLOT_VC0_POSITION 16
#define VC_MAX_TIME_SLOT_VC0_MASK (0x3f << VC_MAX_TIME_SLOT_VC0_POSITION)
#define VC_PORT_ARBI_TABLE_VC0_POSITION 24
#define VC_PORT_ARBI_TABLE_VC0_MASK (0xff << VC_PORT_ARBI_TABLE_VC0_POSITION)

#define VC_TC_MAP_VC0_BIT0_POSITION 0
#define VC_TC_MAP_VC0_BIT0_MASK (0x1 << VC_TC_MAP_VC0_BIT0_POSITION)
#define VC_TC_MAP_VC0_BIT1_POSITION 1
#define VC_TC_MAP_VC0_BIT1_MASK (0x7f << VC_TC_MAP_VC0_BIT1_POSITION)
#define VC_LOAD_PORT_ARBI_TABLE_VC0_POSITION 16
#define VC_LOAD_PORT_ARBI_TABLE_VC0_MASK (0x1 << VC_LOAD_PORT_ARBI_TABLE_VC0_POSITION)
#define VC_PORT_ARBI_SELECT_VC0_POSITION 17
#define VC_PORT_ARBI_SELECT_VC0_MASK (0x1 << VC_PORT_ARBI_SELECT_VC0_POSITION) // SNSPS limitation, 1 bit writable.
#define VC_ID_VC0_POSITION 24
#define VC_ID_VC0_MASK (0x7 << VC_ID_VC0_POSITION)
#define VC_ENABLE_VC0_POSITION 31
#define VC_ENABLE_VC0_MASK (0x1 << VC_ENABLE_VC0_POSITION)

#define VC_PORT_ARBI_TABLE_STATUS_VC0_POSITION 16
#define VC_PORT_ARBI_TABLE_STATUS_VC0_MASK (0x1 << VC_PORT_ARBI_TABLE_STATUS_VC0_POSITION)
#define VC_NEGO_PENDING_VC0_POSITION 17
#define VC_NEGO_PENDING_VC0_MASK (0x1 << VC_NEGO_PENDING_VC0_POSITION)

#define VC_PORT_ARBI_TABLE_STATUS_VC1_POSITION 16
#define VC_PORT_ARBI_TABLE_STATUS_VC1_MASK (0x1 << VC_PORT_ARBI_TABLE_STATUS_VC1_POSITION)
#define VC_NEGO_PENDING_VC1_POSITION 17
#define VC_NEGO_PENDING_VC1_MASK (0x1 << VC_NEGO_PENDING_VC1_POSITION)

#define VC_PORT_ARBI_TABLE_STATUS_VC2_POSITION 16
#define VC_PORT_ARBI_TABLE_STATUS_VC2_MASK (0x1 << VC_PORT_ARBI_TABLE_STATUS_VC2_POSITION)
#define VC_NEGO_PENDING_VC2_POSITION 17
#define VC_NEGO_PENDING_VC2_MASK (0x1 << VC_NEGO_PENDING_VC2_POSITION)

#define VC_PORT_ARBI_TABLE_STATUS_VC3_POSITION 16
#define VC_PORT_ARBI_TABLE_STATUS_VC3_MASK (0x1 << VC_PORT_ARBI_TABLE_STATUS_VC3_POSITION)
#define VC_NEGO_PENDING_VC3_POSITION 17
#define VC_NEGO_PENDING_VC3_MASK (0x1 << VC_NEGO_PENDING_VC3_POSITION)

#define VC_PORT_ARBI_TABLE_STATUS_VC4_POSITION 16
#define VC_PORT_ARBI_TABLE_STATUS_VC4_MASK (0x1 << VC_PORT_ARBI_TABLE_STATUS_VC4_POSITION)
#define VC_NEGO_PENDING_VC4_POSITION 17
#define VC_NEGO_PENDING_VC4_MASK (0x1 << VC_NEGO_PENDING_VC4_POSITION)

#define VC_PORT_ARBI_TABLE_STATUS_VC5_POSITION 16
#define VC_PORT_ARBI_TABLE_STATUS_VC5_MASK (0x1 << VC_PORT_ARBI_TABLE_STATUS_VC5_POSITION)
#define VC_NEGO_PENDING_VC5_POSITION 17
#define VC_NEGO_PENDING_VC5_MASK (0x1 << VC_NEGO_PENDING_VC5_POSITION)

#define VC_PORT_ARBI_TABLE_STATUS_VC6_POSITION 16
#define VC_PORT_ARBI_TABLE_STATUS_VC6_MASK (0x1 << VC_PORT_ARBI_TABLE_STATUS_VC6_POSITION)
#define VC_NEGO_PENDING_VC6_POSITION 17
#define VC_NEGO_PENDING_VC6_MASK (0x1 << VC_NEGO_PENDING_VC6_POSITION)

#define VC_PORT_ARBI_TABLE_STATUS_VC7_POSITION 16
#define VC_PORT_ARBI_TABLE_STATUS_VC7_MASK (0x1 << VC_PORT_ARBI_TABLE_STATUS_VC7_POSITION)
#define VC_NEGO_PENDING_VC7_POSITION 17
#define VC_NEGO_PENDING_VC7_MASK (0x1 << VC_NEGO_PENDING_VC7_POSITION)



/// FOR VC > 0 (VC1 to VC7)
#define VC_PORT_ARBI_CAP_VC_POSITION 0
#define VC_PORT_ARBI_CAP_VC_MASK (0xff << VC_PORT_ARBI_CAP_VC_POSITION)
#define VC_REJECT_SNOOP_TRANS_VC_POSITION 15
#define VC_REJECT_SNOOP_TRANS_VC_MASK (0x1 << VC_REJECT_SNOOP_TRANS_VC_POSITION)
#define VC_MAX_TIME_SLOT_VC_POSITION 16
#define VC_MAX_TIME_SLOT_VC_MASK (0x3f << VC_MAX_TIME_SLOT_VC_POSITION)
#define VC_PORT_ARBI_TABLE_VC_POSITION 24
#define VC_PORT_ARBI_TABLE_VC_MASK (0xff << VC_PORT_ARBI_TABLE_VC_POSITION)

#define VC_TC_MAP_VC_BIT0_POSITION 0
#define VC_TC_MAP_VC_BIT0_MASK (0x1 << VC_TC_MAP_VC_BIT0_POSITION)
#define VC_TC_MAP_VC_BIT1_POSITION 1
#define VC_TC_MAP_VC_BIT1_MASK (0x7f << VC_TC_MAP_VC_BIT1_POSITION)
#define VC_LOAD_PORT_ARBI_TABLE_VC_POSITION 16
#define VC_LOAD_PORT_ARBI_TABLE_VC_MASK (0x1 << VC_LOAD_PORT_ARBI_TABLE_VC_POSITION)
#define VC_PORT_ARBI_SELECT_VC_POSITION 17
#define VC_PORT_ARBI_SELECT_VC_MASK (0x7 << VC_PORT_ARBI_SELECT_VC_POSITION)

#define VC_ID_VC_POSITION 24
#define VC_ID_VC_MASK (0x7 << VC_ID_VC_POSITION)
#define VC_ENABLE_VC_POSITION 31
#define VC_ENABLE_VC_MASK (0x1 << VC_ENABLE_VC_POSITION)

#define VC_PORT_ARBI_TABLE_STATUS_VC_POSITION 16
#define VC_PORT_ARBI_TABLE_STATUS_VC_MASK (0x1 << VC_PORT_ARBI_TABLE_STATUS_VC_POSITION)
#define VC_NEGO_PENDING_VC_POSITION 17
#define VC_NEGO_PENDING_VC_MASK (0x1 << VC_NEGO_PENDING_VC_POSITION)






////////////////////////////////////////////
// SPCIE cap structure
////////////////////////////////////////////
#define SPCIE_EXTENDED_CAP_ID_POSITION 0
#define SPCIE_EXTENDED_CAP_ID_MASK (0xffff << SPCIE_EXTENDED_CAP_ID_POSITION)

#define SPCIE_CAP_VERSION_POSITION 16
#define SPCIE_CAP_VERSION_MASK (0xf << SPCIE_CAP_VERSION_POSITION)

#define SPCIE_NEXT_OFFSET_POSITION 20
#define SPCIE_NEXT_OFFSET_MASK ( 120xfff << SPCIE_NEXT_OFFSET_POSITION)

#define SPCIE_PERFORM_EQ_POSITION 0
#define SPCIE_PERFORM_EQ_MASK (1 << SPCIE_PERFORM_EQ_POSITION)

#define SPCIE_EQ_REQ_INT_EN_POSITION 1
#define SPCIE_EQ_REQ_INT_EN_MASK (1 << SPCIE_EQ_REQ_INT_EN_POSITION)

#define SPCIE_LANE_ERR_STATUS_POSITION 0
#define SPCIE_LANE_ERR_STATUS_MASK ( {CX_NL{0x1}} << `SPCIE_LANE_ERR_STATUS_POSITION)
#define SPCIE_RSVDP_LANE_ERR_STATUS_POSITION CX_NL
#define SPCIE_RSVDP_LANE_ERR_STATUS_MASK ( {(32-CX_NL){0x1}} << `SPCIE_RSVDP_LANE_ERR_STATUS_POSITION)

#define SPCIE_DSP_TX_PRESET0_POSITION 0
#define SPCIE_DSP_TX_PRESET0_MASK (0xf << SPCIE_DSP_TX_PRESET0_POSITION)
#define SPCIE_DSP_RX_PRESET_HINT0_POSITION 4
#define SPCIE_DSP_RX_PRESET_HINT0_MASK (0x7 << SPCIE_DSP_RX_PRESET_HINT0_POSITION)
#define SPCIE_USP_TX_PRESET0_POSITION 8
#define SPCIE_USP_TX_PRESET0_MASK (0xf << SPCIE_USP_TX_PRESET0_POSITION)
#define SPCIE_USP_RX_PRESET_HINT0_POSITION 12
#define SPCIE_USP_RX_PRESET_HINT0_MASK (0x7 << SPCIE_USP_RX_PRESET_HINT0_POSITION)

#define SPCIE_DSP_TX_PRESET1_POSITION 16
#define SPCIE_DSP_TX_PRESET1_MASK (0xf << SPCIE_DSP_TX_PRESET1_POSITION)
#define SPCIE_DSP_RX_PRESET_HINT1_POSITION 20
#define SPCIE_DSP_RX_PRESET_HINT1_MASK (0x7 << SPCIE_DSP_RX_PRESET_HINT1_POSITION)
#define SPCIE_USP_TX_PRESET1_POSITION 24
#define SPCIE_USP_TX_PRESET1_MASK (0xf << SPCIE_USP_TX_PRESET1_POSITION)
#define SPCIE_USP_RX_PRESET_HINT1_POSITION 28
#define SPCIE_USP_RX_PRESET_HINT1_MASK (0x7 << SPCIE_USP_RX_PRESET_HINT1_POSITION)


#define SPCIE_DSP_TX_PRESET2_POSITION 0
#define SPCIE_DSP_TX_PRESET2_MASK (0xf << SPCIE_DSP_TX_PRESET2_POSITION)
#define SPCIE_DSP_RX_PRESET_HINT2_POSITION 4
#define SPCIE_DSP_RX_PRESET_HINT2_MASK (0x7 << SPCIE_DSP_RX_PRESET_HINT2_POSITION)
#define SPCIE_USP_TX_PRESET2_POSITION 8
#define SPCIE_USP_TX_PRESET2_MASK (0xf << SPCIE_USP_TX_PRESET2_POSITION)
#define SPCIE_USP_RX_PRESET_HINT2_POSITION 12
#define SPCIE_USP_RX_PRESET_HINT2_MASK (0x7 << SPCIE_USP_RX_PRESET_HINT2_POSITION)

#define SPCIE_DSP_TX_PRESET3_POSITION 16
#define SPCIE_DSP_TX_PRESET3_MASK (0xf << SPCIE_DSP_TX_PRESET3_POSITION)
#define SPCIE_DSP_RX_PRESET_HINT3_POSITION 20
#define SPCIE_DSP_RX_PRESET_HINT3_MASK (0x7 << SPCIE_DSP_RX_PRESET_HINT3_POSITION)
#define SPCIE_USP_TX_PRESET3_POSITION 24
#define SPCIE_USP_TX_PRESET3_MASK (0xf << SPCIE_USP_TX_PRESET3_POSITION)
#define SPCIE_USP_RX_PRESET_HINT3_POSITION 28
#define SPCIE_USP_RX_PRESET_HINT3_MASK (0x7 << SPCIE_USP_RX_PRESET_HINT3_POSITION)

#define SPCIE_DSP_TX_PRESET4_POSITION 0
#define SPCIE_DSP_TX_PRESET4_MASK (0xf << SPCIE_DSP_TX_PRESET4_POSITION)
#define SPCIE_DSP_RX_PRESET_HINT4_POSITION 4
#define SPCIE_DSP_RX_PRESET_HINT4_MASK (0x7 << SPCIE_DSP_RX_PRESET_HINT4_POSITION)
#define SPCIE_USP_TX_PRESET4_POSITION 8
#define SPCIE_USP_TX_PRESET4_MASK (0xf << SPCIE_USP_TX_PRESET4_POSITION)
#define SPCIE_USP_RX_PRESET_HINT4_POSITION 12
#define SPCIE_USP_RX_PRESET_HINT4_MASK (0x7 << SPCIE_USP_RX_PRESET_HINT4_POSITION)

#define SPCIE_DSP_TX_PRESET5_POSITION 16
#define SPCIE_DSP_TX_PRESET5_MASK (0xf << SPCIE_DSP_TX_PRESET5_POSITION)
#define SPCIE_DSP_RX_PRESET_HINT5_POSITION 20
#define SPCIE_DSP_RX_PRESET_HINT5_MASK (0x7 << SPCIE_DSP_RX_PRESET_HINT5_POSITION)
#define SPCIE_USP_TX_PRESET5_POSITION 24
#define SPCIE_USP_TX_PRESET5_MASK (0xf << SPCIE_USP_TX_PRESET5_POSITION)
#define SPCIE_USP_RX_PRESET_HINT5_POSITION 28
#define SPCIE_USP_RX_PRESET_HINT5_MASK (0x7 << SPCIE_USP_RX_PRESET_HINT5_POSITION)


#define SPCIE_DSP_TX_PRESET6_POSITION 0
#define SPCIE_DSP_TX_PRESET6_MASK (0xf << SPCIE_DSP_TX_PRESET6_POSITION)
#define SPCIE_DSP_RX_PRESET_HINT6_POSITION 4
#define SPCIE_DSP_RX_PRESET_HINT6_MASK (0x7 << SPCIE_DSP_RX_PRESET_HINT6_POSITION)
#define SPCIE_USP_TX_PRESET6_POSITION 8
#define SPCIE_USP_TX_PRESET6_MASK (0xf << SPCIE_USP_TX_PRESET6_POSITION)
#define SPCIE_USP_RX_PRESET_HINT6_POSITION 12
#define SPCIE_USP_RX_PRESET_HINT6_MASK (0x7 << SPCIE_USP_RX_PRESET_HINT6_POSITION)

#define SPCIE_DSP_TX_PRESET7_POSITION 16
#define SPCIE_DSP_TX_PRESET7_MASK (0xf << SPCIE_DSP_TX_PRESET7_POSITION)
#define SPCIE_DSP_RX_PRESET_HINT7_POSITION 20
#define SPCIE_DSP_RX_PRESET_HINT7_MASK (0x7 << SPCIE_DSP_RX_PRESET_HINT7_POSITION)
#define SPCIE_USP_TX_PRESET7_POSITION 24
#define SPCIE_USP_TX_PRESET7_MASK (0xf << SPCIE_USP_TX_PRESET7_POSITION)
#define SPCIE_USP_RX_PRESET_HINT7_POSITION 28
#define SPCIE_USP_RX_PRESET_HINT7_MASK (0x7 << SPCIE_USP_RX_PRESET_HINT7_POSITION)


#define SPCIE_DSP_TX_PRESET8_POSITION 0
#define SPCIE_DSP_TX_PRESET8_MASK (0xf << SPCIE_DSP_TX_PRESET8_POSITION)
#define SPCIE_DSP_RX_PRESET_HINT8_POSITION 4
#define SPCIE_DSP_RX_PRESET_HINT8_MASK (0x7 << SPCIE_DSP_RX_PRESET_HINT8_POSITION)
#define SPCIE_USP_TX_PRESET8_POSITION 8
#define SPCIE_USP_TX_PRESET8_MASK (0xf << SPCIE_USP_TX_PRESET8_POSITION)
#define SPCIE_USP_RX_PRESET_HINT8_POSITION 12
#define SPCIE_USP_RX_PRESET_HINT8_MASK (0x7 << SPCIE_USP_RX_PRESET_HINT8_POSITION)

#define SPCIE_DSP_TX_PRESET9_POSITION 16
#define SPCIE_DSP_TX_PRESET9_MASK (0xf << SPCIE_DSP_TX_PRESET9_POSITION)
#define SPCIE_DSP_RX_PRESET_HINT9_POSITION 20
#define SPCIE_DSP_RX_PRESET_HINT9_MASK (0x7 << SPCIE_DSP_RX_PRESET_HINT9_POSITION)
#define SPCIE_USP_TX_PRESET9_POSITION 24
#define SPCIE_USP_TX_PRESET9_MASK (0xf << SPCIE_USP_TX_PRESET9_POSITION)
#define SPCIE_USP_RX_PRESET_HINT9_POSITION 28
#define SPCIE_USP_RX_PRESET_HINT9_MASK (0x7 << SPCIE_USP_RX_PRESET_HINT9_POSITION)


#define SPCIE_DSP_TX_PRESET10_POSITION 0
#define SPCIE_DSP_TX_PRESET10_MASK (0xf << SPCIE_DSP_TX_PRESET10_POSITION)
#define SPCIE_DSP_RX_PRESET_HINT10_POSITION 4
#define SPCIE_DSP_RX_PRESET_HINT10_MASK (0x7 << SPCIE_DSP_RX_PRESET_HINT10_POSITION)
#define SPCIE_USP_TX_PRESET10_POSITION 8
#define SPCIE_USP_TX_PRESET10_MASK (0xf << SPCIE_USP_TX_PRESET10_POSITION)
#define SPCIE_USP_RX_PRESET_HINT10_POSITION 12
#define SPCIE_USP_RX_PRESET_HINT10_MASK (0x7 << SPCIE_USP_RX_PRESET_HINT10_POSITION)

#define SPCIE_DSP_TX_PRESET11_POSITION 16
#define SPCIE_DSP_TX_PRESET11_MASK (0xf << SPCIE_DSP_TX_PRESET11_POSITION)
#define SPCIE_DSP_RX_PRESET_HINT11_POSITION 20
#define SPCIE_DSP_RX_PRESET_HINT11_MASK (0x7 << SPCIE_DSP_RX_PRESET_HINT11_POSITION)
#define SPCIE_USP_TX_PRESET11_POSITION 24
#define SPCIE_USP_TX_PRESET11_MASK (0xf << SPCIE_USP_TX_PRESET11_POSITION)
#define SPCIE_USP_RX_PRESET_HINT11_POSITION 28
#define SPCIE_USP_RX_PRESET_HINT11_MASK (0x7 << SPCIE_USP_RX_PRESET_HINT11_POSITION)


#define SPCIE_DSP_TX_PRESET12_POSITION 0
#define SPCIE_DSP_TX_PRESET12_MASK (0xf << SPCIE_DSP_TX_PRESET12_POSITION)
#define SPCIE_DSP_RX_PRESET_HINT12_POSITION 4
#define SPCIE_DSP_RX_PRESET_HINT12_MASK (0x7 << SPCIE_DSP_RX_PRESET_HINT12_POSITION)
#define SPCIE_USP_TX_PRESET12_POSITION 8
#define SPCIE_USP_TX_PRESET12_MASK (0xf << SPCIE_USP_TX_PRESET12_POSITION)
#define SPCIE_USP_RX_PRESET_HINT12_POSITION 12
#define SPCIE_USP_RX_PRESET_HINT12_MASK (0x7 << SPCIE_USP_RX_PRESET_HINT12_POSITION)

#define SPCIE_DSP_TX_PRESET13_POSITION 16
#define SPCIE_DSP_TX_PRESET13_MASK (0xf << SPCIE_DSP_TX_PRESET13_POSITION)
#define SPCIE_DSP_RX_PRESET_HINT13_POSITION 20
#define SPCIE_DSP_RX_PRESET_HINT13_MASK (0x7 << SPCIE_DSP_RX_PRESET_HINT13_POSITION)
#define SPCIE_USP_TX_PRESET13_POSITION 24
#define SPCIE_USP_TX_PRESET13_MASK (0xf << SPCIE_USP_TX_PRESET13_POSITION)
#define SPCIE_USP_RX_PRESET_HINT13_POSITION 28
#define SPCIE_USP_RX_PRESET_HINT13_MASK (0x7 << SPCIE_USP_RX_PRESET_HINT13_POSITION)

#define SPCIE_DSP_TX_PRESET14_POSITION 0
#define SPCIE_DSP_TX_PRESET14_MASK (0xf << SPCIE_DSP_TX_PRESET14_POSITION)
#define SPCIE_DSP_RX_PRESET_HINT14_POSITION 4
#define SPCIE_DSP_RX_PRESET_HINT14_MASK (0x7 << SPCIE_DSP_RX_PRESET_HINT14_POSITION)
#define SPCIE_USP_TX_PRESET14_POSITION 8
#define SPCIE_USP_TX_PRESET14_MASK (0xf << SPCIE_USP_TX_PRESET14_POSITION)
#define SPCIE_USP_RX_PRESET_HINT14_POSITION 12
#define SPCIE_USP_RX_PRESET_HINT14_MASK (0x7 << SPCIE_USP_RX_PRESET_HINT14_POSITION)

#define SPCIE_DSP_TX_PRESET15_POSITION 16
#define SPCIE_DSP_TX_PRESET15_MASK (0xf << SPCIE_DSP_TX_PRESET15_POSITION)
#define SPCIE_DSP_RX_PRESET_HINT15_POSITION 20
#define SPCIE_DSP_RX_PRESET_HINT15_MASK (0x7 << SPCIE_DSP_RX_PRESET_HINT15_POSITION)
#define SPCIE_USP_TX_PRESET15_POSITION 24
#define SPCIE_USP_TX_PRESET15_MASK (0xf << SPCIE_USP_TX_PRESET15_POSITION)
#define SPCIE_USP_RX_PRESET_HINT15_POSITION 28
#define SPCIE_USP_RX_PRESET_HINT15_MASK (0x7 << SPCIE_USP_RX_PRESET_HINT15_POSITION)

#ifndef CX_NL_GTR_1
#define SPCIE_DSP_16G_TX_PRESET0_POSITION       16
#define SPCIE_DSP_16G_TX_PRESET0_MASK           (0xf << SPCIE_DSP_16G_TX_PRESET0_POSITION)
#define SPCIE_DSP_16G_RX_PRESET_HINT0_POSITION  20
#define SPCIE_DSP_16G_RX_PRESET_HINT0_MASK      (0x7 << SPCIE_DSP_16G_RX_PRESET_HINT0_POSITION)
#define SPCIE_USP_16G_TX_PRESET0_POSITION       24
#define SPCIE_USP_16G_TX_PRESET0_MASK           (0xf << SPCIE_USP_16G_TX_PRESET0_POSITION)
#define SPCIE_USP_16G_RX_PRESET_HINT0_POSITION  28
#define SPCIE_USP_16G_RX_PRESET_HINT0_MASK      (0x7 << SPCIE_USP_16G_RX_PRESET_HINT0_POSITION)
#else
#define SPCIE_DSP_16G_TX_PRESET0_POSITION       0
#define SPCIE_DSP_16G_TX_PRESET0_MASK           (0xf << SPCIE_DSP_16G_TX_PRESET0_POSITION)
#define SPCIE_DSP_16G_RX_PRESET_HINT0_POSITION  4
#define SPCIE_DSP_16G_RX_PRESET_HINT0_MASK      (0x7 << SPCIE_DSP_16G_RX_PRESET_HINT0_POSITION)
#define SPCIE_USP_16G_TX_PRESET0_POSITION       8
#define SPCIE_USP_16G_TX_PRESET0_MASK           (0xf << SPCIE_USP_16G_TX_PRESET0_POSITION)
#define SPCIE_USP_16G_RX_PRESET_HINT0_POSITION  12
#define SPCIE_USP_16G_RX_PRESET_HINT0_MASK      (0x7 << SPCIE_USP_16G_RX_PRESET_HINT0_POSITION)
#endif
#define SPCIE_DSP_16G_TX_PRESET1_POSITION       16
#define SPCIE_DSP_16G_TX_PRESET1_MASK           (0xf << SPCIE_DSP_16G_TX_PRESET1_POSITION)
#define SPCIE_DSP_16G_RX_PRESET_HINT1_POSITION  20
#define SPCIE_DSP_16G_RX_PRESET_HINT1_MASK      (0x7 << SPCIE_DSP_16G_RX_PRESET_HINT1_POSITION)
#define SPCIE_USP_16G_TX_PRESET1_POSITION       24
#define SPCIE_USP_16G_TX_PRESET1_MASK           (0xf << SPCIE_USP_16G_TX_PRESET1_POSITION)
#define SPCIE_USP_16G_RX_PRESET_HINT1_POSITION  28
#define SPCIE_USP_16G_RX_PRESET_HINT1_MASK      (0x7 << SPCIE_USP_16G_RX_PRESET_HINT1_POSITION)

#define SPCIE_DSP_16G_TX_PRESET2_POSITION       0
#define SPCIE_DSP_16G_TX_PRESET2_MASK           (0xf << SPCIE_DSP_16G_TX_PRESET2_POSITION)
#define SPCIE_DSP_16G_RX_PRESET_HINT2_POSITION  4
#define SPCIE_DSP_16G_RX_PRESET_HINT2_MASK      (0x7 << SPCIE_DSP_16G_RX_PRESET_HINT2_POSITION)
#define SPCIE_USP_16G_TX_PRESET2_POSITION       8
#define SPCIE_USP_16G_TX_PRESET2_MASK           (0xf << SPCIE_USP_16G_TX_PRESET2_POSITION)
#define SPCIE_USP_16G_RX_PRESET_HINT2_POSITION  12
#define SPCIE_USP_16G_RX_PRESET_HINT2_MASK      (0x7 << SPCIE_USP_16G_RX_PRESET_HINT2_POSITION)

#define SPCIE_DSP_16G_TX_PRESET3_POSITION       16
#define SPCIE_DSP_16G_TX_PRESET3_MASK           (0xf << SPCIE_DSP_16G_TX_PRESET3_POSITION)
#define SPCIE_DSP_16G_RX_PRESET_HINT3_POSITION  20
#define SPCIE_DSP_16G_RX_PRESET_HINT3_MASK      (0x7 << SPCIE_DSP_16G_RX_PRESET_HINT3_POSITION)
#define SPCIE_USP_16G_TX_PRESET3_POSITION       24
#define SPCIE_USP_16G_TX_PRESET3_MASK           (0xf << SPCIE_USP_16G_TX_PRESET3_POSITION)
#define SPCIE_USP_16G_RX_PRESET_HINT3_POSITION  28
#define SPCIE_USP_16G_RX_PRESET_HINT3_MASK      (0x7 << SPCIE_USP_16G_RX_PRESET_HINT3_POSITION)

#define SPCIE_DSP_16G_TX_PRESET4_POSITION       0
#define SPCIE_DSP_16G_TX_PRESET4_MASK           (0xf << SPCIE_DSP_16G_TX_PRESET4_POSITION)
#define SPCIE_DSP_16G_RX_PRESET_HINT4_POSITION  4
#define SPCIE_DSP_16G_RX_PRESET_HINT4_MASK      (0x7 << SPCIE_DSP_16G_RX_PRESET_HINT4_POSITION)
#define SPCIE_USP_16G_TX_PRESET4_POSITION       8
#define SPCIE_USP_16G_TX_PRESET4_MASK           (0xf << SPCIE_USP_16G_TX_PRESET4_POSITION)
#define SPCIE_USP_16G_RX_PRESET_HINT4_POSITION  12
#define SPCIE_USP_16G_RX_PRESET_HINT4_MASK      (0x7 << SPCIE_USP_16G_RX_PRESET_HINT4_POSITION)

#define SPCIE_DSP_16G_TX_PRESET5_POSITION       16
#define SPCIE_DSP_16G_TX_PRESET5_MASK           (0xf << SPCIE_DSP_16G_TX_PRESET5_POSITION)
#define SPCIE_DSP_16G_RX_PRESET_HINT5_POSITION  20
#define SPCIE_DSP_16G_RX_PRESET_HINT5_MASK      (0x7 << SPCIE_DSP_16G_RX_PRESET_HINT5_POSITION)
#define SPCIE_USP_16G_TX_PRESET5_POSITION       24
#define SPCIE_USP_16G_TX_PRESET5_MASK           (0xf << SPCIE_USP_16G_TX_PRESET5_POSITION)
#define SPCIE_USP_16G_RX_PRESET_HINT5_POSITION  28
#define SPCIE_USP_16G_RX_PRESET_HINT5_MASK      (0x7 << SPCIE_USP_16G_RX_PRESET_HINT5_POSITION)

#define SPCIE_DSP_16G_TX_PRESET6_POSITION       0
#define SPCIE_DSP_16G_TX_PRESET6_MASK           (0xf << SPCIE_DSP_16G_TX_PRESET6_POSITION)
#define SPCIE_DSP_16G_RX_PRESET_HINT6_POSITION  4
#define SPCIE_DSP_16G_RX_PRESET_HINT6_MASK      (0x7 << SPCIE_DSP_16G_RX_PRESET_HINT6_POSITION)
#define SPCIE_USP_16G_TX_PRESET6_POSITION       8
#define SPCIE_USP_16G_TX_PRESET6_MASK           (0xf << SPCIE_USP_16G_TX_PRESET6_POSITION)
#define SPCIE_USP_16G_RX_PRESET_HINT6_POSITION  12
#define SPCIE_USP_16G_RX_PRESET_HINT6_MASK      (0x7 << SPCIE_USP_16G_RX_PRESET_HINT6_POSITION)

#define SPCIE_DSP_16G_TX_PRESET7_POSITION       16
#define SPCIE_DSP_16G_TX_PRESET7_MASK           (0xf << SPCIE_DSP_16G_TX_PRESET7_POSITION)
#define SPCIE_DSP_16G_RX_PRESET_HINT7_POSITION  20
#define SPCIE_DSP_16G_RX_PRESET_HINT7_MASK      (0x7 << SPCIE_DSP_16G_RX_PRESET_HINT7_POSITION)
#define SPCIE_USP_16G_TX_PRESET7_POSITION       24
#define SPCIE_USP_16G_TX_PRESET7_MASK           (0xf << SPCIE_USP_16G_TX_PRESET7_POSITION)
#define SPCIE_USP_16G_RX_PRESET_HINT7_POSITION  28
#define SPCIE_USP_16G_RX_PRESET_HINT7_MASK      (0x7 << SPCIE_USP_16G_RX_PRESET_HINT7_POSITION)

#define SPCIE_DSP_16G_TX_PRESET8_POSITION       0
#define SPCIE_DSP_16G_TX_PRESET8_MASK           (0xf << SPCIE_DSP_16G_TX_PRESET8_POSITION)
#define SPCIE_DSP_16G_RX_PRESET_HINT8_POSITION  4
#define SPCIE_DSP_16G_RX_PRESET_HINT8_MASK      (0x7 << SPCIE_DSP_16G_RX_PRESET_HINT8_POSITION)
#define SPCIE_USP_16G_TX_PRESET8_POSITION       8
#define SPCIE_USP_16G_TX_PRESET8_MASK           (0xf << SPCIE_USP_16G_TX_PRESET8_POSITION)
#define SPCIE_USP_16G_RX_PRESET_HINT8_POSITION  12
#define SPCIE_USP_16G_RX_PRESET_HINT8_MASK      (0x7 << SPCIE_USP_16G_RX_PRESET_HINT8_POSITION)

#define SPCIE_DSP_16G_TX_PRESET9_POSITION       16
#define SPCIE_DSP_16G_TX_PRESET9_MASK           (0xf << SPCIE_DSP_16G_TX_PRESET9_POSITION)
#define SPCIE_DSP_16G_RX_PRESET_HINT9_POSITION  20
#define SPCIE_DSP_16G_RX_PRESET_HINT9_MASK      (0x7 << SPCIE_DSP_16G_RX_PRESET_HINT9_POSITION)
#define SPCIE_USP_16G_TX_PRESET9_POSITION       24
#define SPCIE_USP_16G_TX_PRESET9_MASK           (0xf << SPCIE_USP_16G_TX_PRESET9_POSITION)
#define SPCIE_USP_16G_RX_PRESET_HINT9_POSITION  28
#define SPCIE_USP_16G_RX_PRESET_HINT9_MASK      (0x7 << SPCIE_USP_16G_RX_PRESET_HINT9_POSITION)

#define SPCIE_DSP_16G_TX_PRESET10_POSITION      0
#define SPCIE_DSP_16G_TX_PRESET10_MASK          (0xf << SPCIE_DSP_16G_TX_PRESET10_POSITION)
#define SPCIE_DSP_16G_RX_PRESET_HINT10_POSITION 4
#define SPCIE_DSP_16G_RX_PRESET_HINT10_MASK     (0x7 << SPCIE_DSP_16G_RX_PRESET_HINT10_POSITION)
#define SPCIE_USP_16G_TX_PRESET10_POSITION      8
#define SPCIE_USP_16G_TX_PRESET10_MASK          (0xf << SPCIE_USP_16G_TX_PRESET10_POSITION)
#define SPCIE_USP_16G_RX_PRESET_HINT10_POSITION 12
#define SPCIE_USP_16G_RX_PRESET_HINT10_MASK     (0x7 << SPCIE_USP_16G_RX_PRESET_HINT10_POSITION)

#define SPCIE_DSP_16G_TX_PRESET11_POSITION      16
#define SPCIE_DSP_16G_TX_PRESET11_MASK          (0xf << SPCIE_DSP_16G_TX_PRESET11_POSITION)
#define SPCIE_DSP_16G_RX_PRESET_HINT11_POSITION 20
#define SPCIE_DSP_16G_RX_PRESET_HINT11_MASK     (0x7 << SPCIE_DSP_16G_RX_PRESET_HINT11_POSITION)
#define SPCIE_USP_16G_TX_PRESET11_POSITION      24
#define SPCIE_USP_16G_TX_PRESET11_MASK          (0xf << SPCIE_USP_16G_TX_PRESET11_POSITION)
#define SPCIE_USP_16G_RX_PRESET_HINT11_POSITION 28
#define SPCIE_USP_16G_RX_PRESET_HINT11_MASK     (0x7 << SPCIE_USP_16G_RX_PRESET_HINT11_POSITION)

#define SPCIE_DSP_16G_TX_PRESET12_POSITION      0
#define SPCIE_DSP_16G_TX_PRESET12_MASK          (0xf << SPCIE_DSP_16G_TX_PRESET12_POSITION)
#define SPCIE_DSP_16G_RX_PRESET_HINT12_POSITION 4
#define SPCIE_DSP_16G_RX_PRESET_HINT12_MASK     (0x7 << SPCIE_DSP_16G_RX_PRESET_HINT12_POSITION)
#define SPCIE_USP_16G_TX_PRESET12_POSITION      8
#define SPCIE_USP_16G_TX_PRESET12_MASK          (0xf << SPCIE_USP_16G_TX_PRESET12_POSITION)
#define SPCIE_USP_16G_RX_PRESET_HINT12_POSITION 12
#define SPCIE_USP_16G_RX_PRESET_HINT12_MASK     (0x7 << SPCIE_USP_16G_RX_PRESET_HINT12_POSITION)

#define SPCIE_DSP_16G_TX_PRESET13_POSITION      16
#define SPCIE_DSP_16G_TX_PRESET13_MASK          (0xf << SPCIE_DSP_16G_TX_PRESET13_POSITION)
#define SPCIE_DSP_16G_RX_PRESET_HINT13_POSITION 20
#define SPCIE_DSP_16G_RX_PRESET_HINT13_MASK     (0x7 << SPCIE_DSP_16G_RX_PRESET_HINT13_POSITION)
#define SPCIE_USP_16G_TX_PRESET13_POSITION      24
#define SPCIE_USP_16G_TX_PRESET13_MASK          (0xf << SPCIE_USP_16G_TX_PRESET13_POSITION)
#define SPCIE_USP_16G_RX_PRESET_HINT13_POSITION 28
#define SPCIE_USP_16G_RX_PRESET_HINT13_MASK     (0x7 << SPCIE_USP_16G_RX_PRESET_HINT13_POSITION)

#define SPCIE_DSP_16G_TX_PRESET14_POSITION      0
#define SPCIE_DSP_16G_TX_PRESET14_MASK          (0xf << SPCIE_DSP_16G_TX_PRESET14_POSITION)
#define SPCIE_DSP_16G_RX_PRESET_HINT14_POSITION 4
#define SPCIE_DSP_16G_RX_PRESET_HINT14_MASK     (0x7 << SPCIE_DSP_16G_RX_PRESET_HINT14_POSITION)
#define SPCIE_USP_16G_TX_PRESET14_POSITION      8
#define SPCIE_USP_16G_TX_PRESET14_MASK          (0xf << SPCIE_USP_16G_TX_PRESET14_POSITION)
#define SPCIE_USP_16G_RX_PRESET_HINT14_POSITION 12
#define SPCIE_USP_16G_RX_PRESET_HINT14_MASK     (0x7 << SPCIE_USP_16G_RX_PRESET_HINT14_POSITION)

#define SPCIE_DSP_16G_TX_PRESET15_POSITION      16
#define SPCIE_DSP_16G_TX_PRESET15_MASK          (0xf << SPCIE_DSP_16G_TX_PRESET15_POSITION)
#define SPCIE_DSP_16G_RX_PRESET_HINT15_POSITION 20
#define SPCIE_DSP_16G_RX_PRESET_HINT15_MASK     (0x7 << SPCIE_DSP_16G_RX_PRESET_HINT15_POSITION)
#define SPCIE_USP_16G_TX_PRESET15_POSITION      24
#define SPCIE_USP_16G_TX_PRESET15_MASK          (0xf << SPCIE_USP_16G_TX_PRESET15_POSITION)
#define SPCIE_USP_16G_RX_PRESET_HINT15_POSITION 28
#define SPCIE_USP_16G_RX_PRESET_HINT15_MASK     (0x7 << SPCIE_USP_16G_RX_PRESET_HINT15_POSITION)

////////////////////////////////////////////////////////
// Physical Layer 16.0 GT/s Extended Capability Structure
////////////////////////////////////////////////////////

// Physical Layer 16.0 GT/s Extended Capability Header
#define PL16G_EXTENDED_CAP_ID_POSITION                   0
#define PL16G_EXTENDED_CAP_ID_MASK                       (0xFFFF << PL16G_EXTENDED_CAP_ID_POSITION)
#define PL16G_CAP_VERSION_POSITION                       16
#define PL16G_CAP_VERSION_MASK                           (0xF << PL16G_CAP_VERSION_POSITION)
#define PL16G_NEXT_OFFSET_POSITION                       20
#define PL16G_NEXT_OFFSET_MASK                           (0xFFF << PL16G_NEXT_OFFSET_POSITION)

// 16.0 GT/s Capabilities Register

// 16.0 GT/s Control Register

// 16.0 GT/s Status Register
#define PL16G_EQ_16G_CPL_POSITION                        0
#define PL16G_EQ_16G_CPL_MASK                            (0x1 << PL16G_EQ_16G_CPL_POSITION)
#define PL16G_EQ_16G_CPL_P1_POSITION                     1
#define PL16G_EQ_16G_CPL_P1_MASK                         (0x1 << PL16G_EQ_16G_CPL_P1_POSITION)
#define PL16G_EQ_16G_CPL_P2_POSITION                     2
#define PL16G_EQ_16G_CPL_P2_MASK                         (0x1 << PL16G_EQ_16G_CPL_P2_POSITION)
#define PL16G_EQ_16G_CPL_P3_POSITION                     3
#define PL16G_EQ_16G_CPL_P3_MASK                         (0x1 << PL16G_EQ_16G_CPL_P3_POSITION)
#define PL16G_LINK_EQ_16G_REQ_POSITION                   4
#define PL16G_LINK_EQ_16G_REQ_MASK                       (0x1 << PL16G_LINK_EQ_16G_REQ_POSITION)

// 16.0 GT/s Local Data Parity Mismatch Status Register
#define PL16G_LC_DPAR_STATUS_POSITION                    0
#define PL16G_LC_DPAR_STATUS_MASK                        ({CX_NL{0x1}} << `PL16G_LC_DPAR_STATUS_POSITION)
#define PL16G_RSVDP_LC_DPAR_STATUS_POSITION              CX_NL
#define PL16G_RSVDP_LC_DPAR_STATUS_MASK                  ({(32-CX_NL){0x1}} << `PL16G_RSVDP_LC_DPAR_STATUS_POSITION)

// 16.0 GT/s First Retimer Data Parity Mismatch Status Register
#define PL16G_FIRST_RETIMER_DPAR_STATUS_POSITION         0
#define PL16G_FIRST_RETIMER_DPAR_STATUS_MASK             ({CX_NL{0x1}} << `PL16G_FIRST_RETIMER_DPAR_STATUS_POSITION)
#define PL16G_RSVDP_FIRST_RETIMER_DPAR_STATUS_POSITION   CX_NL
#define PL16G_RSVDP_FIRST_RETIMER_DPAR_STATUS_MASK       ({(32-CX_NL){0x1}} << `PL16G_RSVDP_FIRST_RETIMER_DPAR_STATUS_POSITION)

// 16.0 GT/s Second Retimer Data Parity Mismatch Status Register
#define PL16G_SECOND_RETIMER_DPAR_STATUS_POSITION        0
#define PL16G_SECOND_RETIMER_DPAR_STATUS_MASK            ({CX_NL{0x1}} << `PL16G_SECOND_RETIMER_DPAR_STATUS_POSITION)
#define PL16G_RSVDP_SECOND_RETIMER_DPAR_STATUS_POSITION  CX_NL
#define PL16G_RSVDP_SECOND_RETIMER_DPAR_STATUS_MASK      ({(32-CX_NL){0x1}} << `PL16G_RSVDP_SECOND_RETIMER_DPAR_STATUS_POSITION)

// 16.0 GT/s Lane Equalization Control Register  for Lane 0-3
#define PL16G_DSP_16G_TX_PRESET0_POSITION                0
#define PL16G_DSP_16G_TX_PRESET0_MASK                    (0xF << PL16G_DSP_16G_TX_PRESET0_POSITION)
#define PL16G_USP_16G_TX_PRESET0_POSITION                4
#define PL16G_USP_16G_TX_PRESET0_MASK                    (0xF << PL16G_USP_16G_TX_PRESET0_POSITION)
#define PL16G_DSP_16G_TX_PRESET1_POSITION                8
#define PL16G_DSP_16G_TX_PRESET1_MASK                    (0xF << PL16G_DSP_16G_TX_PRESET1_POSITION)
#define PL16G_USP_16G_TX_PRESET1_POSITION                12
#define PL16G_USP_16G_TX_PRESET1_MASK                    (0xF << PL16G_USP_16G_TX_PRESET1_POSITION)
#define PL16G_DSP_16G_TX_PRESET2_POSITION                16
#define PL16G_DSP_16G_TX_PRESET2_MASK                    (0xF << PL16G_DSP_16G_TX_PRESET2_POSITION)
#define PL16G_USP_16G_TX_PRESET2_POSITION                20
#define PL16G_USP_16G_TX_PRESET2_MASK                    (0xF << PL16G_USP_16G_TX_PRESET2_POSITION)
#define PL16G_DSP_16G_TX_PRESET3_POSITION                24
#define PL16G_DSP_16G_TX_PRESET3_MASK                    (0xF << PL16G_DSP_16G_TX_PRESET3_POSITION)
#define PL16G_USP_16G_TX_PRESET3_POSITION                28
#define PL16G_USP_16G_TX_PRESET3_MASK                    (0xF << PL16G_USP_16G_TX_PRESET3_POSITION)

// 16.0 GT/s Lane Equalization Control Register  for Lane 4-7
#define PL16G_DSP_16G_TX_PRESET4_POSITION                0
#define PL16G_DSP_16G_TX_PRESET4_MASK                    (0xF << PL16G_DSP_16G_TX_PRESET4_POSITION)
#define PL16G_USP_16G_TX_PRESET4_POSITION                4
#define PL16G_USP_16G_TX_PRESET4_MASK                    (0xF << PL16G_USP_16G_TX_PRESET4_POSITION)
#define PL16G_DSP_16G_TX_PRESET5_POSITION                8
#define PL16G_DSP_16G_TX_PRESET5_MASK                    (0xF << PL16G_DSP_16G_TX_PRESET5_POSITION)
#define PL16G_USP_16G_TX_PRESET5_POSITION                12
#define PL16G_USP_16G_TX_PRESET5_MASK                    (0xF << PL16G_USP_16G_TX_PRESET5_POSITION)
#define PL16G_DSP_16G_TX_PRESET6_POSITION                16
#define PL16G_DSP_16G_TX_PRESET6_MASK                    (0xF << PL16G_DSP_16G_TX_PRESET6_POSITION)
#define PL16G_USP_16G_TX_PRESET6_POSITION                20
#define PL16G_USP_16G_TX_PRESET6_MASK                    (0xF << PL16G_USP_16G_TX_PRESET6_POSITION)
#define PL16G_DSP_16G_TX_PRESET7_POSITION                24
#define PL16G_DSP_16G_TX_PRESET7_MASK                    (0xF << PL16G_DSP_16G_TX_PRESET7_POSITION)
#define PL16G_USP_16G_TX_PRESET7_POSITION                28
#define PL16G_USP_16G_TX_PRESET7_MASK                    (0xF << PL16G_USP_16G_TX_PRESET7_POSITION)

// 16.0 GT/s Lane Equalization Control Register  for Lane 8-11
#define PL16G_DSP_16G_TX_PRESET8_POSITION                0
#define PL16G_DSP_16G_TX_PRESET8_MASK                    (0xF << PL16G_DSP_16G_TX_PRESET8_POSITION)
#define PL16G_USP_16G_TX_PRESET8_POSITION                4
#define PL16G_USP_16G_TX_PRESET8_MASK                    (0xF << PL16G_USP_16G_TX_PRESET8_POSITION)
#define PL16G_DSP_16G_TX_PRESET9_POSITION                8
#define PL16G_DSP_16G_TX_PRESET9_MASK                    (0xF << PL16G_DSP_16G_TX_PRESET9_POSITION)
#define PL16G_USP_16G_TX_PRESET9_POSITION                12
#define PL16G_USP_16G_TX_PRESET9_MASK                    (0xF << PL16G_USP_16G_TX_PRESET9_POSITION)
#define PL16G_DSP_16G_TX_PRESET10_POSITION               16
#define PL16G_DSP_16G_TX_PRESET10_MASK                   (0xF << PL16G_DSP_16G_TX_PRESET10_POSITION)
#define PL16G_USP_16G_TX_PRESET10_POSITION               20
#define PL16G_USP_16G_TX_PRESET10_MASK                   (0xF << PL16G_USP_16G_TX_PRESET10_POSITION)
#define PL16G_DSP_16G_TX_PRESET11_POSITION               24
#define PL16G_DSP_16G_TX_PRESET11_MASK                   (0xF << PL16G_DSP_16G_TX_PRESET11_POSITION)
#define PL16G_USP_16G_TX_PRESET11_POSITION               28
#define PL16G_USP_16G_TX_PRESET11_MASK                   (0xF << PL16G_USP_16G_TX_PRESET11_POSITION)

// 16.0 GT/s Lane Equalization Control Register  for Lane 12-15
#define PL16G_DSP_16G_TX_PRESET12_POSITION               0
#define PL16G_DSP_16G_TX_PRESET12_MASK                   (0xF << PL16G_DSP_16G_TX_PRESET12_POSITION)
#define PL16G_USP_16G_TX_PRESET12_POSITION               4
#define PL16G_USP_16G_TX_PRESET12_MASK                   (0xF << PL16G_USP_16G_TX_PRESET12_POSITION)
#define PL16G_DSP_16G_TX_PRESET13_POSITION               8
#define PL16G_DSP_16G_TX_PRESET13_MASK                   (0xF << PL16G_DSP_16G_TX_PRESET13_POSITION)
#define PL16G_USP_16G_TX_PRESET13_POSITION               12
#define PL16G_USP_16G_TX_PRESET13_MASK                   (0xF << PL16G_USP_16G_TX_PRESET13_POSITION)
#define PL16G_DSP_16G_TX_PRESET14_POSITION               16
#define PL16G_DSP_16G_TX_PRESET14_MASK                   (0xF << PL16G_DSP_16G_TX_PRESET14_POSITION)
#define PL16G_USP_16G_TX_PRESET14_POSITION               20
#define PL16G_USP_16G_TX_PRESET14_MASK                   (0xF << PL16G_USP_16G_TX_PRESET14_POSITION)
#define PL16G_DSP_16G_TX_PRESET15_POSITION               24
#define PL16G_DSP_16G_TX_PRESET15_MASK                   (0xF << PL16G_DSP_16G_TX_PRESET15_POSITION)
#define PL16G_USP_16G_TX_PRESET15_POSITION               28
#define PL16G_USP_16G_TX_PRESET15_MASK                   (0xF << PL16G_USP_16G_TX_PRESET15_POSITION)


////////////////////////////////////////////////////////
// Margining Extended Capability Structure
////////////////////////////////////////////////////////

// Margining Extended Capability Header
#define MARGIN_EXTENDED_CAP_ID_POSITION         0
#define MARGIN_EXTENDED_CAP_ID_MASK             (0xFFFF << MARGIN_EXTENDED_CAP_ID_POSITION)
#define MARGIN_CAP_VERSION_POSITION             16
#define MARGIN_CAP_VERSION_MASK                 (0xF << MARGIN_CAP_VERSION_POSITION)
#define MARGIN_NEXT_OFFSET_POSITION             20
#define MARGIN_NEXT_OFFSET_MASK                 (0xFFF << MARGIN_NEXT_OFFSET_POSITION)

// Margining Port Capabilities and Status Register
#define MARGIN_MARGINING_USES_DRIVER_SOFTWARE_POSITION 0
#define MARGIN_MARGINING_USES_DRIVER_SOFTWARE_MASK     (0x1 << MARGIN_MARGINING_USES_DRIVER_SOFTWARE_POSITION)
#define MARGIN_MARGINING_READY_POSITION                16
#define MARGIN_MARGINING_READY_MASK                    (0x1 << MARGIN_MARGINING_READY_POSITION)
#define MARGIN_MARGINING_SOFTWARE_READY_POSITION       17
#define MARGIN_MARGINING_SOFTWARE_READY_MASK           (0x1 << MARGIN_MARGINING_SOFTWARE_READY_POSITION)

// Margining Lane Control and Status Register for Lane 0-15
#define MARGIN_RECEIVER_NUMBER_POSITION         0
#define MARGIN_RECEIVER_NUMBER_MASK             (0x7 << MARGIN_RECEIVER_NUMBER_POSITION)
#define MARGIN_MARGIN_TYPE_POSITION             3
#define MARGIN_MARGIN_TYPE_MASK                 (0x7 << MARGIN_MARGIN_TYPE_POSITION)
#define MARGIN_USAGE_MODEL_POSITION             6
#define MARGIN_USAGE_MODEL_MASK                 (0x1 << MARGIN_USAGE_MODEL_POSITION)
#define MARGIN_MARGIN_PAYLOAD_POSITION          8
#define MARGIN_MARGIN_PAYLOAD_MASK              (0xFF << MARGIN_MARGIN_PAYLOAD_POSITION)
#define MARGIN_RECEIVER_NUMBER_STATUS_POSITION  16
#define MARGIN_RECEIVER_NUMBER_STATUS_MASK      (0x7 << MARGIN_RECEIVER_NUMBER_STATUS_POSITION)
#define MARGIN_MARGIN_TYPE_STATUS_POSITION      19
#define MARGIN_MARGIN_TYPE_STATUS_MASK          (0x7 << MARGIN_MARGIN_TYPE_STATUS_POSITION)
#define MARGIN_USAGE_MODEL_STATUS_POSITION      22
#define MARGIN_USAGE_MODEL_STATUS_MASK          (0x1 << MARGIN_USAGE_MODEL_STATUS_POSITION)
#define MARGIN_MARGIN_PAYLOAD_STATUS_POSITION   24
#define MARGIN_MARGIN_PAYLOAD_STATUS_MASK       (0xFF << MARGIN_MARGIN_PAYLOAD_STATUS_POSITION)


////////////////////////////////////////////
// L1SUB cap structure
////////////////////////////////////////////

// L1 Substates Extended Capability Header
#define L1SUB_EXTENDED_CAP_ID_POSITION 0
#define L1SUB_CAP_VERSION_POSITION 16
#define L1SUB_NEXT_OFFSET_POSITION 20

#define L1SUB_EXTENDED_CAP_ID_MASK (0xffff << L1SUB_EXTENDED_CAP_ID_POSITION)
#define L1SUB_CAP_VERSION_MASK     (0xf << L1SUB_CAP_VERSION_POSITION)
#define L1SUB_NEXT_OFFSET_MASK     ( 0xfff << L1SUB_NEXT_OFFSET_POSITION)

// L1 Substates Capabilities Register
#define L1SUB_L1_2_PCIPM_SUPPORT_POSITION    0
#define L1SUB_L1_1_PCIPM_SUPPORT_POSITION    1
#define L1SUB_L1_2_ASPM_SUPPORT_POSITION     2
#define L1SUB_L1_1_ASPM_SUPPORT_POSITION     3
#define L1SUB_L1_PMSUB_SUPPORT_POSITION      4
#define L1SUB_COMM_MODE_SUPPORT_POSITION     8
#define L1SUB_PWR_ON_SCALE_SUPPORT_POSITION 16
#define L1SUB_PWR_ON_VALUE_SUPPORT_POSITION 19

#define L1SUB_L1_2_PCIPM_SUPPORT_MASK   (0x1  << L1SUB_L1_2_PCIPM_SUPPORT_POSITION)
#define L1SUB_L1_1_PCIPM_SUPPORT_MASK   (0x1  << L1SUB_L1_1_PCIPM_SUPPORT_POSITION)
#define L1SUB_L1_2_ASPM_SUPPORT_MASK    (0x1  << L1SUB_L1_2_ASPM_SUPPORT_POSITION)
#define L1SUB_L1_1_ASPM_SUPPORT_MASK    (0x1  << L1SUB_L1_1_ASPM_SUPPORT_POSITION)
#define L1SUB_PCIPM_SUPPORT_MASK        (0x1  << L1SUB_PCIPM_SUPPORT_POSITION)
#define L1SUB_ASPM_SUPPORT_MASK         (0x1  << L1SUB_ASPM_SUPPORT_POSITION)
#define L1SUB_L1_PMSUB_SUPPORT_MASK     (0x1  << L1SUB_L1_PMSUB_SUPPORT_POSITION)
#define L1SUB_COMM_MODE_SUPPORT_MASK    (0xff << L1SUB_COMM_MODE_SUPPORT_POSITION)
#define L1SUB_PWR_ON_SCALE_SUPPORT_MASK (0x3  << L1SUB_PWR_ON_SCALE_SUPPORT_POSITION)
#define L1SUB_PWR_ON_VALUE_SUPPORT_MASK (0x1f << L1SUB_PWR_ON_VALUE_SUPPORT_POSITION)

//L1 Substates Control1 Register
#define L1SUB_L1_2_PCIPM_EN_POSITION   0
#define L1SUB_L1_1_PCIPM_EN_POSITION   1
#define L1SUB_L1_2_ASPM_EN_POSITION    2
#define L1SUB_L1_1_ASPM_EN_POSITION    3
#define L1SUB_T_COMMON_MODE_POSITION   8
#define L1SUB_L1_2_TH_VAL_POSITION    16
#define L1SUB_L1_2_TH_SCA_POSITION    29

#define L1SUB_L1_2_PCIPM_EN_MASK   ( 0x1   << L1SUB_L1_2_PCIPM_EN_POSITION)
#define L1SUB_L1_1_PCIPM_EN_MASK   ( 0x1   << L1SUB_L1_1_PCIPM_EN_POSITION)
#define L1SUB_L1_2_ASPM_EN_MASK    ( 0x1   << L1SUB_L1_2_ASPM_EN_POSITION)
#define L1SUB_L1_1_ASPM_EN_MASK    ( 0x1   << L1SUB_L1_1_ASPM_EN_POSITION)
#define L1SUB_T_COMMON_MODE_MASK   ( 0xff  << L1SUB_T_COMMON_MODE_POSITION)
#define L1SUB_L1_2_TH_VAL_MASK     (0x3ff << L1SUB_L1_2_TH_VAL_POSITION)
#define L1SUB_L1_2_TH_SCA_MASK     ( 0x7   << L1SUB_L1_2_TH_SCA_POSITION)

//L1 Substates Control2 Register
#define L1SUB_T_POWER_ON_SCALE_POSITION      0
#define L1SUB_T_POWER_ON_VALUE_POSITION      3

#define L1SUB_T_POWER_ON_SCALE_MASK      ( 0x3   << L1SUB_T_POWER_ON_SCALE_POSITION)
#define L1SUB_T_POWER_ON_VALUE_MASK      ( 0x1f  << L1SUB_T_POWER_ON_VALUE_POSITION)


////////////////////////////////////////////
// M-PCIe cap structure
////////////////////////////////////////////

// M-PCIe Extended Capability Header
#define MPCIE_EXTENDED_CAP_ID_POSITION                  0
#define MPCIE_EXTENDED_CAP_ID_MASK                      (0xFFFF << MPCIE_EXTENDED_CAP_ID_POSITION)
#define MPCIE_CAP_VERSION_POSITION                      16
#define MPCIE_CAP_VERSION_MASK                          (0xF << MPCIE_CAP_VERSION_POSITION)
#define MPCIE_NEXT_OFFSET_POSITION                      20
#define MPCIE_NEXT_OFFSET_MASK                          (0xFFF << MPCIE_NEXT_OFFSET_POSITION)

// M-PCIe Capabilities Register
#define MPCIE_MPCIE_LINK_SPEED_CAP_POSITION             0
#define MPCIE_MPCIE_LINK_SPEED_CAP_MASK                 (0x3 << MPCIE_MPCIE_LINK_SPEED_CAP_POSITION)
#define MPCIE_RESERVED_0X04_14_2_POSITION               2
#define MPCIE_RESERVED_0X04_14_2_MASK                   (0x1FFF << MPCIE_RESERVED_0X04_14_2_POSITION)
#define MPCIE_CONFIG_SOFTWARE_SUPPORTED_POSITION        15
#define MPCIE_CONFIG_SOFTWARE_SUPPORTED_MASK            (0x1 << MPCIE_CONFIG_SOFTWARE_SUPPORTED_POSITION)
#define MPCIE_MAX_TX_LANE_WIDTH_CAP_POSITION            16
#define MPCIE_MAX_TX_LANE_WIDTH_CAP_MASK                (0xFF << MPCIE_MAX_TX_LANE_WIDTH_CAP_POSITION)
#define MPCIE_MAX_RX_LANE_WIDTH_CAP_POSITION            24
#define MPCIE_MAX_RX_LANE_WIDTH_CAP_MASK                (0xFF << MPCIE_MAX_RX_LANE_WIDTH_CAP_POSITION)

// M-PCIe Control Register
#define MPCIE_MPCIE_TARGET_LINK_SPEED_CONTROL_POSITION  0
#define MPCIE_MPCIE_TARGET_LINK_SPEED_CONTROL_MASK      (0x3 << MPCIE_MPCIE_TARGET_LINK_SPEED_CONTROL_POSITION)
#define MPCIE_RESERVED_0X08_31_2_POSITION               2
#define MPCIE_RESERVED_0X08_31_2_MASK                   (0x3FFF_FFFF << MPCIE_RESERVED_0X08_31_2_POSITION)

// M-PCIe Status Register
#define MPCIE_MPCIE_CURRENT_LINK_SPEED_STATUS_POSITION  0
#define MPCIE_MPCIE_CURRENT_LINK_SPEED_STATUS_MASK      (0x3 << MPCIE_MPCIE_CURRENT_LINK_SPEED_STATUS_POSITION)
#define MPCIE_RESERVED_0X0C_13_2_POSITION               2
#define MPCIE_RESERVED_0X0C_13_2_MASK                   (0xFFF << MPCIE_RESERVED_0X0C_13_2_POSITION)
#define MPCIE_RATE_SERIES_STATUS_POSITION               14
#define MPCIE_RATE_SERIES_STATUS_MASK                   (0x1 << MPCIE_RATE_SERIES_STATUS_POSITION)
#define MPCIE_MPCIE_CONFIG_SOFTWARE_STATUS_POSITION     15
#define MPCIE_MPCIE_CONFIG_SOFTWARE_STATUS_MASK         (0x1 << MPCIE_MPCIE_CONFIG_SOFTWARE_STATUS_POSITION)
#define MPCIE_TX_LANE_WIDTH_STATUS_POSITION             16
#define MPCIE_TX_LANE_WIDTH_STATUS_MASK                 (0xFF << MPCIE_TX_LANE_WIDTH_STATUS_POSITION)
#define MPCIE_RX_LANE_WIDTH_STATUS_POSITION             24
#define MPCIE_RX_LANE_WIDTH_STATUS_MASK                 (0xFF << MPCIE_RX_LANE_WIDTH_STATUS_POSITION)

// M-PCIe LANE Error Status Register
#define MPCIE_MPCIE_LANE_ERR_STATUS_POSITION            0
#define MPCIE_MPCIE_LANE_ERR_STATUS_MASK                ({CM_RXNL_GUI{0x1}} << `MPCIE_MPCIE_LANE_ERR_STATUS_POSITION)
#define MPCIE_RESERVED_0X10_31_NL_POSITION              CM_RXNL_GUI
#define MPCIE_RESERVED_0X10_31_NL_MASK                  ({(32-CM_RXNL_GUI){0x1}} << `MPCIE_RESERVED_0X10_31_NL_POSITION)

// M-PCIe Phy Control Address Register
#define MPCIE_LOWER_ADDR_7_0_POSITION                   0
#define MPCIE_LOWER_ADDR_7_0_MASK                       (0xFF << MPCIE_LOWER_ADDR_7_0_POSITION)
#define MPCIE_RESERVED_0X14_15_8_POSITION               8
#define MPCIE_RESERVED_0X14_15_8_MASK                   (0xFF << MPCIE_RESERVED_0X14_15_8_POSITION)
#define MPCIE_UPPERADDR_4_0_POSITION                    16
#define MPCIE_UPPERADDR_4_0_MASK                        (0x1F << MPCIE_UPPERADDR_4_0_POSITION)
#define MPCIE_RESERVED_0X14_23_21_POSITION              21
#define MPCIE_RESERVED_0X14_23_21_MASK                  (0x7 << MPCIE_RESERVED_0X14_23_21_POSITION)
#define MPCIE_UPPERADDR_5_POSITION                      24
#define MPCIE_UPPERADDR_5_MASK                          (0x1 << MPCIE_UPPERADDR_5_POSITION)
#define MPCIE_PHY_LOCATION_POSITION                     25
#define MPCIE_PHY_LOCATION_MASK                         (0x7 << MPCIE_PHY_LOCATION_POSITION)
#define MPCIE_RESERVED_0X14_29_28_POSITION              28
#define MPCIE_RESERVED_0X14_29_28_MASK                  (0x3 << MPCIE_RESERVED_0X14_29_28_POSITION)
#define MPCIE_READ_POSITION                             30
#define MPCIE_READ_MASK                                 (0x1 << MPCIE_READ_POSITION)
#define MPCIE_CONFIG_POSITION                           31
#define MPCIE_CONFIG_MASK                               (0x1 << MPCIE_CONFIG_POSITION)

// M-PCIe Phy Control Data Register
//#define MPCIE_PHY_REGISTER_DATA_POSITION                0
//#define MPCIE_PHY_REGISTER_DATA_MASK                    (0xFF << MPCIE_PHY_REGISTER_DATA_POSITION)
#define MPCIE_RESERVED_0X18_28_8_POSITION               0
#define MPCIE_RESERVED_0X18_28_8_MASK                   (0x1F_FFFF << MPCIE_RESERVED_0X18_28_8_POSITION)
#define MPCIE_PHY_CONTROL_ERR_POSITION                  21
#define MPCIE_PHY_CONTROL_ERR_MASK                      (0x1 << MPCIE_PHY_CONTROL_ERR_POSITION)
#define MPCIE_RRAP_ABORT_POSITION                       22
#define MPCIE_RRAP_ABORT_MASK                           (0x1 << MPCIE_RRAP_ABORT_POSITION)
#define MPCIE_PHY_CONTROL_PENDING_POSITION              23
#define MPCIE_PHY_CONTROL_PENDING_MASK                  (0x1 << MPCIE_PHY_CONTROL_PENDING_POSITION)

// M-PCIe Attribute(Viewport)
#define MPCIE_ATTR_LINK_HSGEAR_CAP_POSITION                           0
#define MPCIE_ATTR_LINK_HSGEAR_CAP_MASK                               (0xFF << MPCIE_ATTR_LINK_HSGEAR_CAP_POSITION)
#define MPCIE_ATTR_TX_LANE_WIDTH_SUPPORTED_POSITION                   0
#define MPCIE_ATTR_TX_LANE_WIDTH_SUPPORTED_MASK                       (0xFF << MPCIE_ATTR_TX_LANE_WIDTH_SUPPORTED_POSITION)
#define MPCIE_ATTR_RX_LANE_WIDTH_SUPPORTED_POSITION                   0
#define MPCIE_ATTR_RX_LANE_WIDTH_SUPPORTED_MASK                       (0xFF << MPCIE_ATTR_RX_LANE_WIDTH_SUPPORTED_POSITION)
#define MPCIE_ATTR_CLK_2K_PPM_REQUIRED_POSITION                       0
#define MPCIE_ATTR_CLK_2K_PPM_REQUIRED_MASK                           (0x1 << MPCIE_ATTR_CLK_2K_PPM_REQUIRED_POSITION)
#define MPCIE_ATTR_ASYMMETRIC_LINK_CAP_POSITION                       1
#define MPCIE_ATTR_ASYMMETRIC_LINK_CAP_MASK                           (0x1 << MPCIE_ATTR_ASYMMETRIC_LINK_CAP_POSITION)
#define MPCIE_ATTR_LINK_CFG_RETAIN_CAP_POSITION                       2
#define MPCIE_ATTR_LINK_CFG_RETAIN_CAP_MASK                           (0x1 << MPCIE_ATTR_LINK_CFG_RETAIN_CAP_POSITION)
#define MPCIE_ATTR_RESERVED_0X03_4_3_POSITION                         3
#define MPCIE_ATTR_RESERVED_0X03_4_3_MASK                             (0x3 << MPCIE_ATTR_RESERVED_0X03_4_3_POSITION)
#define MPCIE_ATTR_RATE_CAP_POSITION                                  5
#define MPCIE_ATTR_RATE_CAP_MASK                                      (0x3 << MPCIE_ATTR_RATE_CAP_POSITION)
#define MPCIE_ATTR_HW_AUTO_RATE_RECONFIG_CAP_POSITION                 7
#define MPCIE_ATTR_HW_AUTO_RATE_RECONFIG_CAP_MASK                     (0x1 << MPCIE_ATTR_HW_AUTO_RATE_RECONFIG_CAP_POSITION)
#define MPCIE_ATTR_C_REFCLK_RX_HS_G1_SYNC_LENGTH_CAP_POSITION         0
#define MPCIE_ATTR_C_REFCLK_RX_HS_G1_SYNC_LENGTH_CAP_MASK             (0xFF << MPCIE_ATTR_C_REFCLK_RX_HS_G1_SYNC_LENGTH_CAP_POSITION)
#define MPCIE_ATTR_C_REFCLK_RX_HS_G2_SYNC_LENGTH_CAP_POSITION         0
#define MPCIE_ATTR_C_REFCLK_RX_HS_G2_SYNC_LENGTH_CAP_MASK             (0xFF << MPCIE_ATTR_C_REFCLK_RX_HS_G2_SYNC_LENGTH_CAP_POSITION)
#define MPCIE_ATTR_C_REFCLK_RX_HS_G3_SYNC_LENGTH_CAP_POSITION         0
#define MPCIE_ATTR_C_REFCLK_RX_HS_G3_SYNC_LENGTH_CAP_MASK             (0xFF << MPCIE_ATTR_C_REFCLK_RX_HS_G3_SYNC_LENGTH_CAP_POSITION)
#define MPCIE_ATTR_NC_REFCLK_RX_HS_G1_SYNC_LENGTH_CAP_POSITION        0
#define MPCIE_ATTR_NC_REFCLK_RX_HS_G1_SYNC_LENGTH_CAP_MASK            (0xFF << MPCIE_ATTR_NC_REFCLK_RX_HS_G1_SYNC_LENGTH_CAP_POSITION)
#define MPCIE_ATTR_NC_REFCLK_RX_HS_G2_SYNC_LENGTH_CAP_POSITION        0
#define MPCIE_ATTR_NC_REFCLK_RX_HS_G2_SYNC_LENGTH_CAP_MASK            (0xFF << MPCIE_ATTR_NC_REFCLK_RX_HS_G2_SYNC_LENGTH_CAP_POSITION)
#define MPCIE_ATTR_NC_REFCLK_RX_HS_G3_SYNC_LENGTH_CAP_POSITION        0
#define MPCIE_ATTR_NC_REFCLK_RX_HS_G3_SYNC_LENGTH_CAP_MASK            (0xFF << MPCIE_ATTR_NC_REFCLK_RX_HS_G3_SYNC_LENGTH_CAP_POSITION)
#define MPCIE_ATTR_LINK_RX_HS_G1_PREPARE_LENGTH_CAP_POSITION          0
#define MPCIE_ATTR_LINK_RX_HS_G1_PREPARE_LENGTH_CAP_MASK              (0xF << MPCIE_ATTR_LINK_RX_HS_G1_PREPARE_LENGTH_CAP_POSITION)
#define MPCIE_ATTR_RESERVED_0X0A_7_4_POSITION                         4
#define MPCIE_ATTR_RESERVED_0X0A_7_4_MASK                             (0xF << MPCIE_ATTR_RESERVED_0X0A_7_4_POSITION)
#define MPCIE_ATTR_LINK_RX_HS_G2_PREPARE_LENGTH_CAP_POSITION          0
#define MPCIE_ATTR_LINK_RX_HS_G2_PREPARE_LENGTH_CAP_MASK              (0xF << MPCIE_ATTR_LINK_RX_HS_G2_PREPARE_LENGTH_CAP_POSITION)
#define MPCIE_ATTR_RESERVED_0X0B_7_4_POSITION                         4
#define MPCIE_ATTR_RESERVED_0X0B_7_4_MASK                             (0xF << MPCIE_ATTR_RESERVED_0X0B_7_4_POSITION)
#define MPCIE_ATTR_LINK_RX_HS_G3_PREPARE_LENGTH_CAP_POSITION          0
#define MPCIE_ATTR_LINK_RX_HS_G3_PREPARE_LENGTH_CAP_MASK              (0xF << MPCIE_ATTR_LINK_RX_HS_G3_PREPARE_LENGTH_CAP_POSITION)
#define MPCIE_ATTR_RESERVED_0X0C_7_4_POSITION                         4
#define MPCIE_ATTR_RESERVED_0X0C_7_4_MASK                             (0xF << MPCIE_ATTR_RESERVED_0X0C_7_4_POSITION)
#define MPCIE_ATTR_LINK_RX_PWM_BURST_CLOSURE_LENGTH_CAP_POSITION      0
#define MPCIE_ATTR_LINK_RX_PWM_BURST_CLOSURE_LENGTH_CAP_MASK          (0x1F << MPCIE_ATTR_LINK_RX_PWM_BURST_CLOSURE_LENGTH_CAP_POSITION)
#define MPCIE_ATTR_RESERVED_0X0D_7_5_POSITION                         5
#define MPCIE_ATTR_RESERVED_0X0D_7_5_MASK                             (0x7 << MPCIE_ATTR_RESERVED_0X0D_7_5_POSITION)
#define MPCIE_ATTR_LINK_HIBERN8_TIME_CAP_POSITION                     0
#define MPCIE_ATTR_LINK_HIBERN8_TIME_CAP_MASK                         (0xFF << MPCIE_ATTR_LINK_HIBERN8_TIME_CAP_POSITION)
#define MPCIE_ATTR_LINK_MIN_ACTIVATETIME_CAP_POSITION                 0
#define MPCIE_ATTR_LINK_MIN_ACTIVATETIME_CAP_MASK                     (0xFF << MPCIE_ATTR_LINK_MIN_ACTIVATETIME_CAP_POSITION)
#define MPCIE_ATTR_MPCIE_VENDOR_SPECIFIC_REG_UPPER_ADDR_POSITION      0
#define MPCIE_ATTR_MPCIE_VENDOR_SPECIFIC_REG_UPPER_ADDR_MASK          (0xFF << MPCIE_ATTR_MPCIE_VENDOR_SPECIFIC_REG_UPPER_ADDR_POSITION)
#define MPCIE_ATTR_MPCIE_VENDOR_SPECIFIC_OP_REG_UPPER_ADDR_POSITION   0
#define MPCIE_ATTR_MPCIE_VENDOR_SPECIFIC_OP_REG_UPPER_ADDR_MASK       (0xFF << MPCIE_ATTR_MPCIE_VENDOR_SPECIFIC_OP_REG_UPPER_ADDR_POSITION)
#define MPCIE_ATTR_MPCIE_VENDOR_SPECIFIC_TEST_REG_LOWER_ADDR_POSITION 0
#define MPCIE_ATTR_MPCIE_VENDOR_SPECIFIC_TEST_REG_LOWER_ADDR_MASK     (0xFF << MPCIE_ATTR_MPCIE_VENDOR_SPECIFIC_TEST_REG_LOWER_ADDR_POSITION)
#define MPCIE_ATTR_MPCIE_VENDOR_SPECIFIC_PHY_REG_LOWER_ADDR_POSITION  0
#define MPCIE_ATTR_MPCIE_VENDOR_SPECIFIC_PHY_REG_LOWER_ADDR_MASK      (0xFF << MPCIE_ATTR_MPCIE_VENDOR_SPECIFIC_PHY_REG_LOWER_ADDR_POSITION)
#define MPCIE_ATTR_MPCIE_OP_REG_VENDOR_ID_LOW_POSITION                0
#define MPCIE_ATTR_MPCIE_OP_REG_VENDOR_ID_LOW_MASK                    (0xFF << MPCIE_ATTR_MPCIE_OP_REG_VENDOR_ID_LOW_POSITION)
#define MPCIE_ATTR_MPCIE_OP_REG_VENDOR_ID_HIGH_POSITION               0
#define MPCIE_ATTR_MPCIE_OP_REG_VENDOR_ID_HIGH_MASK                   (0xFF << MPCIE_ATTR_MPCIE_OP_REG_VENDOR_ID_HIGH_POSITION)
#define MPCIE_ATTR_MPCIE_TEST_REG_VENDOR_ID_LOW_POSITION              0
#define MPCIE_ATTR_MPCIE_TEST_REG_VENDOR_ID_LOW_MASK                  (0xFF << MPCIE_ATTR_MPCIE_TEST_REG_VENDOR_ID_LOW_POSITION)
#define MPCIE_ATTR_MPCIE_TEST_REG_VENDOR_ID_HIGH_POSITION             0
#define MPCIE_ATTR_MPCIE_TEST_REG_VENDOR_ID_HIGH_MASK                 (0xFF << MPCIE_ATTR_MPCIE_TEST_REG_VENDOR_ID_HIGH_POSITION)
#define MPCIE_ATTR_MPCIE_PHY_REG_VENDOR_ID_LOW_POSITION               0
#define MPCIE_ATTR_MPCIE_PHY_REG_VENDOR_ID_LOW_MASK                   (0xFF << MPCIE_ATTR_MPCIE_PHY_REG_VENDOR_ID_LOW_POSITION)
#define MPCIE_ATTR_MPCIE_PHY_REG_VENDOR_ID_HIGH_POSITION              0
#define MPCIE_ATTR_MPCIE_PHY_REG_VENDOR_ID_HIGH_MASK                  (0xFF << MPCIE_ATTR_MPCIE_PHY_REG_VENDOR_ID_HIGH_POSITION)
#define MPCIE_ATTR_MPCIE_PHY_REG_VENDOR_ID_ISSUER_POSITION            0
#define MPCIE_ATTR_MPCIE_PHY_REG_VENDOR_ID_ISSUER_MASK                (0xFF << MPCIE_ATTR_MPCIE_PHY_REG_VENDOR_ID_ISSUER_POSITION)
#define MPCIE_ATTR_MPCIE_OP_REG_CAP1_POSITION                         0
#define MPCIE_ATTR_MPCIE_OP_REG_CAP1_MASK                             (0xFF << MPCIE_ATTR_MPCIE_OP_REG_CAP1_POSITION)
#define MPCIE_ATTR_MPCIE_OP_REG_CAP2_POSITION                         0
#define MPCIE_ATTR_MPCIE_OP_REG_CAP2_MASK                             (0xFF << MPCIE_ATTR_MPCIE_OP_REG_CAP2_POSITION)
#define MPCIE_ATTR_MPCIE_OP_REG_CAP3_POSITION                         0
#define MPCIE_ATTR_MPCIE_OP_REG_CAP3_MASK                             (0xFF << MPCIE_ATTR_MPCIE_OP_REG_CAP3_POSITION)
#define MPCIE_ATTR_MPCIE_OP_REG_CAP4_POSITION                         0
#define MPCIE_ATTR_MPCIE_OP_REG_CAP4_MASK                             (0xFF << MPCIE_ATTR_MPCIE_OP_REG_CAP4_POSITION)
#define MPCIE_ATTR_MPCIE_TEST_REG_CAP1_POSITION                       0
#define MPCIE_ATTR_MPCIE_TEST_REG_CAP1_MASK                           (0xFF << MPCIE_ATTR_MPCIE_TEST_REG_CAP1_POSITION)
#define MPCIE_ATTR_MPCIE_TEST_REG_CAP2_POSITION                       0
#define MPCIE_ATTR_MPCIE_TEST_REG_CAP2_MASK                           (0xFF << MPCIE_ATTR_MPCIE_TEST_REG_CAP2_POSITION)
#define MPCIE_ATTR_MPCIE_TEST_REG_CAP3_POSITION                       0
#define MPCIE_ATTR_MPCIE_TEST_REG_CAP3_MASK                           (0xFF << MPCIE_ATTR_MPCIE_TEST_REG_CAP3_POSITION)
#define MPCIE_ATTR_MPCIE_TEST_REG_CAP4_POSITION                       0
#define MPCIE_ATTR_MPCIE_TEST_REG_CAP4_MASK                           (0xFF << MPCIE_ATTR_MPCIE_TEST_REG_CAP4_POSITION)
#define MPCIE_ATTR_MPCIE_PHY_REG_CAP1_POSITION                        0
#define MPCIE_ATTR_MPCIE_PHY_REG_CAP1_MASK                            (0xFF << MPCIE_ATTR_MPCIE_PHY_REG_CAP1_POSITION)
#define MPCIE_ATTR_MPCIE_PHY_REG_CAP2_POSITION                        0
#define MPCIE_ATTR_MPCIE_PHY_REG_CAP2_MASK                            (0xFF << MPCIE_ATTR_MPCIE_PHY_REG_CAP2_POSITION)
#define MPCIE_ATTR_MPCIE_PHY_REG_CAP3_POSITION                        0
#define MPCIE_ATTR_MPCIE_PHY_REG_CAP3_MASK                            (0xFF << MPCIE_ATTR_MPCIE_PHY_REG_CAP3_POSITION)
#define MPCIE_ATTR_MPCIE_PHY_REG_CAP4_POSITION                        0
#define MPCIE_ATTR_MPCIE_PHY_REG_CAP4_MASK                            (0xFF << MPCIE_ATTR_MPCIE_PHY_REG_CAP4_POSITION)
#define MPCIE_ATTR_LINK_HSGEAR_POSITION                               0
#define MPCIE_ATTR_LINK_HSGEAR_MASK                                   (0xFF << MPCIE_ATTR_LINK_HSGEAR_POSITION)
#define MPCIE_ATTR_TX_LANE_WIDTH_POSITION                             0
#define MPCIE_ATTR_TX_LANE_WIDTH_MASK                                 (0xFF << MPCIE_ATTR_TX_LANE_WIDTH_POSITION)
#define MPCIE_ATTR_RX_LANE_WIDTH_POSITION                             0
#define MPCIE_ATTR_RX_LANE_WIDTH_MASK                                 (0xFF << MPCIE_ATTR_RX_LANE_WIDTH_POSITION)
#define MPCIE_ATTR_CLK_2K_PPM_DISABLED_POSITION                       0
#define MPCIE_ATTR_CLK_2K_PPM_DISABLED_MASK                           (0x1 << MPCIE_ATTR_CLK_2K_PPM_DISABLED_POSITION)
#define MPCIE_ATTR_RESERVED_0X13_6_1_POSITION                         1
#define MPCIE_ATTR_RESERVED_0X13_6_1_MASK                             (0x3F << MPCIE_ATTR_RESERVED_0X13_6_1_POSITION)
#define MPCIE_ATTR_HW_AUTO_RATE_RECONFIG_EN_POSITION                  7
#define MPCIE_ATTR_HW_AUTO_RATE_RECONFIG_EN_MASK                      (0x1 << MPCIE_ATTR_HW_AUTO_RATE_RECONFIG_EN_POSITION)
#define MPCIE_ATTR_C_REFCLK_TX_HS_G1_SYNC_LENGTH_CONTROL_POSITION     0
#define MPCIE_ATTR_C_REFCLK_TX_HS_G1_SYNC_LENGTH_CONTROL_MASK         (0xFF << MPCIE_ATTR_C_REFCLK_TX_HS_G1_SYNC_LENGTH_CONTROL_POSITION)
#define MPCIE_ATTR_C_REFCLK_TX_HS_G2_SYNC_LENGTH_CONTROL_POSITION     0
#define MPCIE_ATTR_C_REFCLK_TX_HS_G2_SYNC_LENGTH_CONTROL_MASK         (0xFF << MPCIE_ATTR_C_REFCLK_TX_HS_G2_SYNC_LENGTH_CONTROL_POSITION)
#define MPCIE_ATTR_C_REFCLK_TX_HS_G3_SYNC_LENGTH_CONTROL_POSITION     0
#define MPCIE_ATTR_C_REFCLK_TX_HS_G3_SYNC_LENGTH_CONTROL_MASK         (0xFF << MPCIE_ATTR_C_REFCLK_TX_HS_G3_SYNC_LENGTH_CONTROL_POSITION)
#define MPCIE_ATTR_NC_REFCLK_TX_HS_G1_SYNC_LENGTH_CONTROL_POSITION    0
#define MPCIE_ATTR_NC_REFCLK_TX_HS_G1_SYNC_LENGTH_CONTROL_MASK        (0xFF << MPCIE_ATTR_NC_REFCLK_TX_HS_G1_SYNC_LENGTH_CONTROL_POSITION)
#define MPCIE_ATTR_NC_REFCLK_TX_HS_G2_SYNC_LENGTH_CONTROL_POSITION    0
#define MPCIE_ATTR_NC_REFCLK_TX_HS_G2_SYNC_LENGTH_CONTROL_MASK        (0xFF << MPCIE_ATTR_NC_REFCLK_TX_HS_G2_SYNC_LENGTH_CONTROL_POSITION)
#define MPCIE_ATTR_NC_REFCLK_TX_HS_G3_SYNC_LENGTH_CONTROL_POSITION    0
#define MPCIE_ATTR_NC_REFCLK_TX_HS_G3_SYNC_LENGTH_CONTROL_MASK        (0xFF << MPCIE_ATTR_NC_REFCLK_TX_HS_G3_SYNC_LENGTH_CONTROL_POSITION)
#define MPCIE_ATTR_LINK_TX_HS_G1_PREPARE_LENGTH_CONTROL_POSITION      0
#define MPCIE_ATTR_LINK_TX_HS_G1_PREPARE_LENGTH_CONTROL_MASK          (0xF << MPCIE_ATTR_LINK_TX_HS_G1_PREPARE_LENGTH_CONTROL_POSITION)
#define MPCIE_ATTR_RESERVED_0X1A_7_4_POSITION                         4
#define MPCIE_ATTR_RESERVED_0X1A_7_4_MASK                             (0xF << MPCIE_ATTR_RESERVED_0X1A_7_4_POSITION)
#define MPCIE_ATTR_LINK_TX_HS_G2_PREPARE_LENGTH_CONTROL_POSITION      0
#define MPCIE_ATTR_LINK_TX_HS_G2_PREPARE_LENGTH_CONTROL_MASK          (0xF << MPCIE_ATTR_LINK_TX_HS_G2_PREPARE_LENGTH_CONTROL_POSITION)
#define MPCIE_ATTR_RESERVED_0X1B_7_4_POSITION                         4
#define MPCIE_ATTR_RESERVED_0X1B_7_4_MASK                             (0xF << MPCIE_ATTR_RESERVED_0X1B_7_4_POSITION)
#define MPCIE_ATTR_LINK_TX_HS_G3_PREPARE_LENGTH_CONTROL_POSITION      0
#define MPCIE_ATTR_LINK_TX_HS_G3_PREPARE_LENGTH_CONTROL_MASK          (0xF << MPCIE_ATTR_LINK_TX_HS_G3_PREPARE_LENGTH_CONTROL_POSITION)
#define MPCIE_ATTR_RESERVED_0X1C_7_4_POSITION                         4
#define MPCIE_ATTR_RESERVED_0X1C_7_4_MASK                             (0xF << MPCIE_ATTR_RESERVED_0X1C_7_4_POSITION)
#define MPCIE_ATTR_LINK_TX_HIBERN8_TIME_POSITION                      0
#define MPCIE_ATTR_LINK_TX_HIBERN8_TIME_MASK                          (0xFF << MPCIE_ATTR_LINK_TX_HIBERN8_TIME_POSITION)
#define MPCIE_ATTR_LINK_TX_MIN_ACTIVATETIME_POSITION                  0
#define MPCIE_ATTR_LINK_TX_MIN_ACTIVATETIME_MASK                      (0xFF << MPCIE_ATTR_LINK_TX_MIN_ACTIVATETIME_POSITION)
#define MPCIE_ATTR_LINK_MIN_SAVE_CONFIG_TIME_POSITION                 0
#define MPCIE_ATTR_LINK_MIN_SAVE_CONFIG_TIME_MASK                     (0xFF << MPCIE_ATTR_LINK_MIN_SAVE_CONFIG_TIME_POSITION)
#define MPCIE_ATTR_LINK_CFG_RETRAIN_STATUS_POSITION                   0
#define MPCIE_ATTR_LINK_CFG_RETRAIN_STATUS_MASK                       (0x1 << MPCIE_ATTR_LINK_CFG_RETRAIN_STATUS_POSITION)
#define MPCIE_ATTR_RESERVED_0X20_7_1_POSITION                         1
#define MPCIE_ATTR_RESERVED_0X20_7_1_MASK                             (0x7F << MPCIE_ATTR_RESERVED_0X20_7_1_POSITION)
#define MPCIE_ATTR_MPHY_TEST_MODE_LOWER_POSITION                      0
#define MPCIE_ATTR_MPHY_TEST_MODE_LOWER_MASK                          (0x1 << MPCIE_ATTR_MPHY_TEST_MODE_LOWER_POSITION)
#define MPCIE_ATTR_RESERVED_0XFF_7_1_POSITION                         1
#define MPCIE_ATTR_RESERVED_0XFF_7_1_MASK                             (0x7F << MPCIE_ATTR_RESERVED_0XFF_7_1_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_0_POSITION                           0
#define MPCIE_ATTR_LOOPBACK_LANE_0_MASK                               (0xFF << MPCIE_ATTR_LOOPBACK_LANE_0_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_1_POSITION                           0
#define MPCIE_ATTR_LOOPBACK_LANE_1_MASK                               (0xFF << MPCIE_ATTR_LOOPBACK_LANE_1_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_2_POSITION                           0
#define MPCIE_ATTR_LOOPBACK_LANE_2_MASK                               (0xFF << MPCIE_ATTR_LOOPBACK_LANE_2_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_3_POSITION                           0
#define MPCIE_ATTR_LOOPBACK_LANE_3_MASK                               (0xFF << MPCIE_ATTR_LOOPBACK_LANE_3_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_4_POSITION                           0
#define MPCIE_ATTR_LOOPBACK_LANE_4_MASK                               (0xFF << MPCIE_ATTR_LOOPBACK_LANE_4_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_5_POSITION                           0
#define MPCIE_ATTR_LOOPBACK_LANE_5_MASK                               (0xFF << MPCIE_ATTR_LOOPBACK_LANE_5_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_6_POSITION                           0
#define MPCIE_ATTR_LOOPBACK_LANE_6_MASK                               (0xFF << MPCIE_ATTR_LOOPBACK_LANE_6_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_7_POSITION                           0
#define MPCIE_ATTR_LOOPBACK_LANE_7_MASK                               (0xFF << MPCIE_ATTR_LOOPBACK_LANE_7_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_8_POSITION                           0
#define MPCIE_ATTR_LOOPBACK_LANE_8_MASK                               (0xFF << MPCIE_ATTR_LOOPBACK_LANE_8_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_9_POSITION                           0
#define MPCIE_ATTR_LOOPBACK_LANE_9_MASK                               (0xFF << MPCIE_ATTR_LOOPBACK_LANE_9_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_10_POSITION                          0
#define MPCIE_ATTR_LOOPBACK_LANE_10_MASK                              (0xFF << MPCIE_ATTR_LOOPBACK_LANE_10_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_11_POSITION                          0
#define MPCIE_ATTR_LOOPBACK_LANE_11_MASK                              (0xFF << MPCIE_ATTR_LOOPBACK_LANE_11_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_12_POSITION                          0
#define MPCIE_ATTR_LOOPBACK_LANE_12_MASK                              (0xFF << MPCIE_ATTR_LOOPBACK_LANE_12_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_13_POSITION                          0
#define MPCIE_ATTR_LOOPBACK_LANE_13_MASK                              (0xFF << MPCIE_ATTR_LOOPBACK_LANE_13_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_14_POSITION                          0
#define MPCIE_ATTR_LOOPBACK_LANE_14_MASK                              (0xFF << MPCIE_ATTR_LOOPBACK_LANE_14_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_15_POSITION                          0
#define MPCIE_ATTR_LOOPBACK_LANE_15_MASK                              (0xFF << MPCIE_ATTR_LOOPBACK_LANE_15_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_16_POSITION                          0
#define MPCIE_ATTR_LOOPBACK_LANE_16_MASK                              (0xFF << MPCIE_ATTR_LOOPBACK_LANE_16_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_17_POSITION                          0
#define MPCIE_ATTR_LOOPBACK_LANE_17_MASK                              (0xFF << MPCIE_ATTR_LOOPBACK_LANE_17_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_18_POSITION                          0
#define MPCIE_ATTR_LOOPBACK_LANE_18_MASK                              (0xFF << MPCIE_ATTR_LOOPBACK_LANE_18_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_19_POSITION                          0
#define MPCIE_ATTR_LOOPBACK_LANE_19_MASK                              (0xFF << MPCIE_ATTR_LOOPBACK_LANE_19_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_20_POSITION                          0
#define MPCIE_ATTR_LOOPBACK_LANE_20_MASK                              (0xFF << MPCIE_ATTR_LOOPBACK_LANE_20_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_21_POSITION                          0
#define MPCIE_ATTR_LOOPBACK_LANE_21_MASK                              (0xFF << MPCIE_ATTR_LOOPBACK_LANE_21_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_22_POSITION                          0
#define MPCIE_ATTR_LOOPBACK_LANE_22_MASK                              (0xFF << MPCIE_ATTR_LOOPBACK_LANE_22_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_23_POSITION                          0
#define MPCIE_ATTR_LOOPBACK_LANE_23_MASK                              (0xFF << MPCIE_ATTR_LOOPBACK_LANE_23_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_24_POSITION                          0
#define MPCIE_ATTR_LOOPBACK_LANE_24_MASK                              (0xFF << MPCIE_ATTR_LOOPBACK_LANE_24_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_25_POSITION                          0
#define MPCIE_ATTR_LOOPBACK_LANE_25_MASK                              (0xFF << MPCIE_ATTR_LOOPBACK_LANE_25_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_26_POSITION                          0
#define MPCIE_ATTR_LOOPBACK_LANE_26_MASK                              (0xFF << MPCIE_ATTR_LOOPBACK_LANE_26_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_27_POSITION                          0
#define MPCIE_ATTR_LOOPBACK_LANE_27_MASK                              (0xFF << MPCIE_ATTR_LOOPBACK_LANE_27_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_28_POSITION                          0
#define MPCIE_ATTR_LOOPBACK_LANE_28_MASK                              (0xFF << MPCIE_ATTR_LOOPBACK_LANE_28_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_29_POSITION                          0
#define MPCIE_ATTR_LOOPBACK_LANE_29_MASK                              (0xFF << MPCIE_ATTR_LOOPBACK_LANE_29_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_30_POSITION                          0
#define MPCIE_ATTR_LOOPBACK_LANE_30_MASK                              (0xFF << MPCIE_ATTR_LOOPBACK_LANE_30_POSITION)
#define MPCIE_ATTR_LOOPBACK_LANE_31_POSITION                          0
#define MPCIE_ATTR_LOOPBACK_LANE_31_MASK                              (0xFF << MPCIE_ATTR_LOOPBACK_LANE_31_POSITION)
#define MPCIE_ATTR_TRANSMIT_COMPLIANCE_MODE_LANES_0_POSITION          0
#define MPCIE_ATTR_TRANSMIT_COMPLIANCE_MODE_LANES_0_MASK              (0xFF << MPCIE_ATTR_TRANSMIT_COMPLIANCE_MODE_LANES_0_POSITION)
#define MPCIE_ATTR_TRANSMIT_COMPLIANCE_MODE_LANES_1_POSITION          0
#define MPCIE_ATTR_TRANSMIT_COMPLIANCE_MODE_LANES_1_MASK              (0xFF << MPCIE_ATTR_TRANSMIT_COMPLIANCE_MODE_LANES_1_POSITION)
#define MPCIE_ATTR_TRANSMIT_COMPLIANCE_MODE_LANES_2_POSITION          0
#define MPCIE_ATTR_TRANSMIT_COMPLIANCE_MODE_LANES_2_MASK              (0xFF << MPCIE_ATTR_TRANSMIT_COMPLIANCE_MODE_LANES_2_POSITION)
#define MPCIE_ATTR_TRANSMIT_COMPLIANCE_MODE_LANES_3_POSITION          0
#define MPCIE_ATTR_TRANSMIT_COMPLIANCE_MODE_LANES_3_MASK              (0xFF << MPCIE_ATTR_TRANSMIT_COMPLIANCE_MODE_LANES_3_POSITION)
#define MPCIE_ATTR_MPHY_ANALOG_LOOPBACK_MODE_LANES_0_POSITION         0
#define MPCIE_ATTR_MPHY_ANALOG_LOOPBACK_MODE_LANES_0_MASK             (0xFF << MPCIE_ATTR_MPHY_ANALOG_LOOPBACK_MODE_LANES_0_POSITION)
#define MPCIE_ATTR_MPHY_ANALOG_LOOPBACK_MODE_LANES_1_POSITION         0
#define MPCIE_ATTR_MPHY_ANALOG_LOOPBACK_MODE_LANES_1_MASK             (0xFF << MPCIE_ATTR_MPHY_ANALOG_LOOPBACK_MODE_LANES_1_POSITION)
#define MPCIE_ATTR_MPHY_ANALOG_LOOPBACK_MODE_LANES_2_POSITION         0
#define MPCIE_ATTR_MPHY_ANALOG_LOOPBACK_MODE_LANES_2_MASK             (0xFF << MPCIE_ATTR_MPHY_ANALOG_LOOPBACK_MODE_LANES_2_POSITION)
#define MPCIE_ATTR_MPHY_ANALOG_LOOPBACK_MODE_LANES_3_POSITION         0
#define MPCIE_ATTR_MPHY_ANALOG_LOOPBACK_MODE_LANES_3_MASK             (0xFF << MPCIE_ATTR_MPHY_ANALOG_LOOPBACK_MODE_LANES_3_POSITION)
#define MPCIE_ATTR_COMPLIANCE_MODE_CAPABILITY_POSITION                0
#define MPCIE_ATTR_COMPLIANCE_MODE_CAPABILITY_MASK                    (0x1 << MPCIE_ATTR_COMPLIANCE_MODE_CAPABILITY_POSITION)
#define MPCIE_ATTR_COMPLIANCE_MODE_CONTROL_POSITION                   1
#define MPCIE_ATTR_COMPLIANCE_MODE_CONTROL_MASK                       (0x1 << MPCIE_ATTR_COMPLIANCE_MODE_CONTROL_POSITION)
#define MPCIE_ATTR_RESERVED_0X24_7_2_POSITION                         2
#define MPCIE_ATTR_RESERVED_0X24_7_2_MASK                             (0x3F << MPCIE_ATTR_RESERVED_0X24_7_2_POSITION)
#define MPCIE_ATTR_MPHY_BURST_ERR_COUNT_MODE_0_POSITION               0
#define MPCIE_ATTR_MPHY_BURST_ERR_COUNT_MODE_0_MASK                   (0xFF << MPCIE_ATTR_MPHY_BURST_ERR_COUNT_MODE_0_POSITION)
#define MPCIE_ATTR_MPHY_BURST_ERR_COUNT_MODE_1_POSITION               0
#define MPCIE_ATTR_MPHY_BURST_ERR_COUNT_MODE_1_MASK                   (0xFF << MPCIE_ATTR_MPHY_BURST_ERR_COUNT_MODE_1_POSITION)
#define MPCIE_ATTR_MPHY_BURST_ERR_COUNT_MODE_2_POSITION               0
#define MPCIE_ATTR_MPHY_BURST_ERR_COUNT_MODE_2_MASK                   (0xFF << MPCIE_ATTR_MPHY_BURST_ERR_COUNT_MODE_2_POSITION)
#define MPCIE_ATTR_MPHY_BURST_ERR_COUNT_MODE_3_POSITION               0
#define MPCIE_ATTR_MPHY_BURST_ERR_COUNT_MODE_3_MASK                   (0xFF << MPCIE_ATTR_MPHY_BURST_ERR_COUNT_MODE_3_POSITION)
#define MPCIE_ATTR_RX_BURST_COUNT_0_POSITION                          0
#define MPCIE_ATTR_RX_BURST_COUNT_0_MASK                              (0xFF << MPCIE_ATTR_RX_BURST_COUNT_0_POSITION)
#define MPCIE_ATTR_RX_BURST_COUNT_1_POSITION                          0
#define MPCIE_ATTR_RX_BURST_COUNT_1_MASK                              (0xFF << MPCIE_ATTR_RX_BURST_COUNT_1_POSITION)
#define MPCIE_ATTR_RX_BURST_COUNT_2_POSITION                          0
#define MPCIE_ATTR_RX_BURST_COUNT_2_MASK                              (0xFF << MPCIE_ATTR_RX_BURST_COUNT_2_POSITION)
#define MPCIE_ATTR_RX_BURST_COUNT_3_POSITION                          0
#define MPCIE_ATTR_RX_BURST_COUNT_3_MASK                              (0xFF << MPCIE_ATTR_RX_BURST_COUNT_3_POSITION)
#define MPCIE_ATTR_RX_ERR_COUNT_0_POSITION                            0
#define MPCIE_ATTR_RX_ERR_COUNT_0_MASK                                (0xFF << MPCIE_ATTR_RX_ERR_COUNT_0_POSITION)
#define MPCIE_ATTR_RX_ERR_COUNT_1_POSITION                            0
#define MPCIE_ATTR_RX_ERR_COUNT_1_MASK                                (0xFF << MPCIE_ATTR_RX_ERR_COUNT_1_POSITION)
#define MPCIE_ATTR_RX_ERR_COUNT_2_POSITION                            0
#define MPCIE_ATTR_RX_ERR_COUNT_2_MASK                                (0xFF << MPCIE_ATTR_RX_ERR_COUNT_2_POSITION)
#define MPCIE_ATTR_RX_ERR_COUNT_3_POSITION                            0
#define MPCIE_ATTR_RX_ERR_COUNT_3_MASK                                (0xFF << MPCIE_ATTR_RX_ERR_COUNT_3_POSITION)
#define MPCIE_ATTR_RX_COUNT_RESET_POSITION                            0
#define MPCIE_ATTR_RX_COUNT_RESET_MASK                                (0x1 << MPCIE_ATTR_RX_COUNT_RESET_POSITION)
#define MPCIE_ATTR_RESERVED_0X4C_7_1_POSITION                         1
#define MPCIE_ATTR_RESERVED_0X4C_7_1_MASK                             (0x7F << MPCIE_ATTR_RESERVED_0X4C_7_1_POSITION)


////////////////////////////////////////////////////////
// RAS D.E.S. Vendor Specific Extended Capability Structure
////////////////////////////////////////////////////////

// Vendor-Specific Extended Capability Header
#define RAS_DES_EXTENDED_CAP_ID_POSITION               0
#define RAS_DES_EXTENDED_CAP_ID_MASK                   (0xFFFF << RAS_DES_EXTENDED_CAP_ID_POSITION)
#define RAS_DES_CAP_VERSION_POSITION                   16
#define RAS_DES_CAP_VERSION_MASK                       (0xF << RAS_DES_CAP_VERSION_POSITION)
#define RAS_DES_NEXT_OFFSET_POSITION                   20
#define RAS_DES_NEXT_OFFSET_MASK                       (0xFFF << RAS_DES_NEXT_OFFSET_POSITION)

// Vendor-Specific Header
#define RAS_DES_VSEC_ID_POSITION                       0
#define RAS_DES_VSEC_ID_MASK                           (0xFFFF << RAS_DES_VSEC_ID_POSITION)
#define RAS_DES_VSEC_REV_POSITION                      16
#define RAS_DES_VSEC_REV_MASK                          (0xF << RAS_DES_VSEC_REV_POSITION)
#define RAS_DES_VSEC_LENGTH_POSITION                   20
#define RAS_DES_VSEC_LENGTH_MASK                       (0xFFF << RAS_DES_VSEC_LENGTH_POSITION)

// Event Counter Control
#define RAS_DES_EVENT_COUNTER_CLEAR_POSITION           0
#define RAS_DES_EVENT_COUNTER_CLEAR_MASK               (0x3 << RAS_DES_EVENT_COUNTER_CLEAR_POSITION)
#define RAS_DES_EVENT_COUNTER_ENABLE_POSITION          2
#define RAS_DES_EVENT_COUNTER_ENABLE_MASK              (0x7 << RAS_DES_EVENT_COUNTER_ENABLE_POSITION)
#define RAS_DES_EVENT_COUNTER_STATUS_POSITION          7
#define RAS_DES_EVENT_COUNTER_STATUS_MASK              (0x1 << RAS_DES_EVENT_COUNTER_STATUS_POSITION)
#define RAS_DES_EVENT_COUNTER_LANE_SELECT_POSITION     8
#define RAS_DES_EVENT_COUNTER_LANE_SELECT_MASK         (0xF << RAS_DES_EVENT_COUNTER_LANE_SELECT_POSITION)
#define RAS_DES_EVENT_COUNTER_EVENT_SELECT_POSITION    16
#define RAS_DES_EVENT_COUNTER_EVENT_SELECT_MASK        (0xFFF << RAS_DES_EVENT_COUNTER_EVENT_SELECT_POSITION)

// Event Counter Data
#define RAS_DES_EVENT_COUNTER_DATA_POSITION            0
#define RAS_DES_EVENT_COUNTER_DATA_MASK                (0xFFFF_FFFF << RAS_DES_EVENT_COUNTER_DATA_POSITION)

// PCIe Usage Analysis Control
#define RAS_DES_TIMER_START_POSITION                   0
#define RAS_DES_TIMER_START_MASK                       (0x1 << RAS_DES_TIMER_START_POSITION)
#define RAS_DES_TIME_BASED_DURATION_SELECT_POSITION    8
#define RAS_DES_TIME_BASED_DURATION_SELECT_MASK        (0xFF << RAS_DES_TIME_BASED_DURATION_SELECT_POSITION)
#define RAS_DES_TIME_BASED_REPORT_SELECT_POSITION      24
#define RAS_DES_TIME_BASED_REPORT_SELECT_MASK          (0xFF << RAS_DES_TIME_BASED_REPORT_SELECT_POSITION)

// PCIe Usage Analysis Data
#define RAS_DES_TIME_BASED_ANALYSIS_DATA_POSITION      0
#define RAS_DES_TIME_BASED_ANALYSIS_DATA_MASK          (0xFFFF_FFFF << RAS_DES_TIME_BASED_ANALYSIS_DATA_POSITION)

// Error Injection Enable
#define RAS_DES_ERROR_INJECTION0_ENABLE_POSITION       0
#define RAS_DES_ERROR_INJECTION0_ENABLE_MASK           (0x1 << RAS_DES_ERROR_INJECTION0_ENABLE_POSITION)
#define RAS_DES_ERROR_INJECTION1_ENABLE_POSITION       1
#define RAS_DES_ERROR_INJECTION1_ENABLE_MASK           (0x1 << RAS_DES_ERROR_INJECTION1_ENABLE_POSITION)
#define RAS_DES_ERROR_INJECTION2_ENABLE_POSITION       2
#define RAS_DES_ERROR_INJECTION2_ENABLE_MASK           (0x1 << RAS_DES_ERROR_INJECTION2_ENABLE_POSITION)
#define RAS_DES_ERROR_INJECTION3_ENABLE_POSITION       3
#define RAS_DES_ERROR_INJECTION3_ENABLE_MASK           (0x1 << RAS_DES_ERROR_INJECTION3_ENABLE_POSITION)
#define RAS_DES_ERROR_INJECTION4_ENABLE_POSITION       4
#define RAS_DES_ERROR_INJECTION4_ENABLE_MASK           (0x1 << RAS_DES_ERROR_INJECTION4_ENABLE_POSITION)
#define RAS_DES_ERROR_INJECTION5_ENABLE_POSITION       5
#define RAS_DES_ERROR_INJECTION5_ENABLE_MASK           (0x1 << RAS_DES_ERROR_INJECTION5_ENABLE_POSITION)
#define RAS_DES_ERROR_INJECTION6_ENABLE_POSITION       6
#define RAS_DES_ERROR_INJECTION6_ENABLE_MASK           (0x1 << RAS_DES_ERROR_INJECTION6_ENABLE_POSITION)

// Error Injection Control 0 (CRC Error)
#define RAS_DES_EINJ0_COUNT_POSITION                   0
#define RAS_DES_EINJ0_COUNT_MASK                       (0xFF << RAS_DES_EINJ0_COUNT_POSITION)
#define RAS_DES_EINJ0_CRC_TYPE_POSITION                8
#define RAS_DES_EINJ0_CRC_TYPE_MASK                    (0xF << RAS_DES_EINJ0_CRC_TYPE_POSITION)

// Error Injection Control 1 (SeqNum Error)
#define RAS_DES_EINJ1_COUNT_POSITION                   0
#define RAS_DES_EINJ1_COUNT_MASK                       (0xFF << RAS_DES_EINJ1_COUNT_POSITION)
#define RAS_DES_EINJ1_SEQNUM_TYPE_POSITION             8
#define RAS_DES_EINJ1_SEQNUM_TYPE_MASK                 (0x1 << RAS_DES_EINJ1_SEQNUM_TYPE_POSITION)
#define RAS_DES_EINJ1_BAD_SEQNUM_POSITION              16
#define RAS_DES_EINJ1_BAD_SEQNUM_MASK                  (0x1FFF << RAS_DES_EINJ1_BAD_SEQNUM_POSITION)

// Error Injection Control 2 (DLLP Error)
#define RAS_DES_EINJ2_COUNT_POSITION                   0
#define RAS_DES_EINJ2_COUNT_MASK                       (0xFF << RAS_DES_EINJ2_COUNT_POSITION)
#define RAS_DES_EINJ2_DLLP_TYPE_POSITION               8
#define RAS_DES_EINJ2_DLLP_TYPE_MASK                   (0x3 << RAS_DES_EINJ2_DLLP_TYPE_POSITION)

// Error Injection Control 3 (Symbol Error)
#define RAS_DES_EINJ3_COUNT_POSITION                   0
#define RAS_DES_EINJ3_COUNT_MASK                       (0xFF << RAS_DES_EINJ3_COUNT_POSITION)
#define RAS_DES_EINJ3_SYMBOL_TYPE_POSITION             8
#define RAS_DES_EINJ3_SYMBOL_TYPE_MASK                 (0x7 << RAS_DES_EINJ3_SYMBOL_TYPE_POSITION)

// Error Injection Control 4 (FC Credit Error)
#define RAS_DES_EINJ4_COUNT_POSITION                   0
#define RAS_DES_EINJ4_COUNT_MASK                       (0xFF << RAS_DES_EINJ4_COUNT_POSITION)
#define RAS_DES_EINJ4_UPDFC_TYPE_POSITION              8
#define RAS_DES_EINJ4_UPDFC_TYPE_MASK                  (0x7 << RAS_DES_EINJ4_UPDFC_TYPE_POSITION)
#define RAS_DES_EINJ4_VC_NUMBER_POSITION               12
#define RAS_DES_EINJ4_VC_NUMBER_MASK                   (0x7 << RAS_DES_EINJ4_VC_NUMBER_POSITION)
#define RAS_DES_EINJ4_BAD_UPDFC_VALUE_POSITION         16
#define RAS_DES_EINJ4_BAD_UPDFC_VALUE_MASK             (0x1FFF << RAS_DES_EINJ4_BAD_UPDFC_VALUE_POSITION)

// Error Injection Control 5 (Specific TLP Error)
#define RAS_DES_EINJ5_COUNT_POSITION                   0
#define RAS_DES_EINJ5_COUNT_MASK                       (0xFF << RAS_DES_EINJ5_COUNT_POSITION)
#define RAS_DES_EINJ5_SPECIFIED_TLP_POSITION           8
#define RAS_DES_EINJ5_SPECIFIED_TLP_MASK               (0x1 << RAS_DES_EINJ5_SPECIFIED_TLP_POSITION)

// Error Injection Control 6 (Compare Point H0)
#define RAS_DES_EINJ6_COMPARE_POINT_H0_POSITION        0
#define RAS_DES_EINJ6_COMPARE_POINT_H0_MASK            (0xFFFF_FFFF << RAS_DES_EINJ6_COMPARE_POINT_H0_POSITION)

// Error Injection Control 6 (Compare Point H1)
#define RAS_DES_EINJ6_COMPARE_POINT_H1_POSITION        0
#define RAS_DES_EINJ6_COMPARE_POINT_H1_MASK            (0xFFFF_FFFF << RAS_DES_EINJ6_COMPARE_POINT_H1_POSITION)

// Error Injection Control 6 (Compare Point H2)
#define RAS_DES_EINJ6_COMPARE_POINT_H2_POSITION        0
#define RAS_DES_EINJ6_COMPARE_POINT_H2_MASK            (0xFFFF_FFFF << RAS_DES_EINJ6_COMPARE_POINT_H2_POSITION)

// Error Injection Control 6 (Compare Point H3)
#define RAS_DES_EINJ6_COMPARE_POINT_H3_POSITION        0
#define RAS_DES_EINJ6_COMPARE_POINT_H3_MASK            (0xFFFF_FFFF << RAS_DES_EINJ6_COMPARE_POINT_H3_POSITION)

// Error Injection Control 6 (Compare Value H0)
#define RAS_DES_EINJ6_COMPARE_VALUE_H0_POSITION        0
#define RAS_DES_EINJ6_COMPARE_VALUE_H0_MASK            (0xFFFF_FFFF << RAS_DES_EINJ6_COMPARE_VALUE_H0_POSITION)

// Error Injection Control 6 (Compare Value H1)
#define RAS_DES_EINJ6_COMPARE_VALUE_H1_POSITION        0
#define RAS_DES_EINJ6_COMPARE_VALUE_H1_MASK            (0xFFFF_FFFF << RAS_DES_EINJ6_COMPARE_VALUE_H1_POSITION)

// Error Injection Control 6 (Compare Value H2)
#define RAS_DES_EINJ6_COMPARE_VALUE_H2_POSITION        0
#define RAS_DES_EINJ6_COMPARE_VALUE_H2_MASK            (0xFFFF_FFFF << RAS_DES_EINJ6_COMPARE_VALUE_H2_POSITION)

// Error Injection Control 6 (Compare Value H3)
#define RAS_DES_EINJ6_COMPARE_VALUE_H3_POSITION        0
#define RAS_DES_EINJ6_COMPARE_VALUE_H3_MASK            (0xFFFF_FFFF << RAS_DES_EINJ6_COMPARE_VALUE_H3_POSITION)

// Error Injection Control 6 (Change Point H0)
#define RAS_DES_EINJ6_CHANGE_POINT_H0_POSITION         0
#define RAS_DES_EINJ6_CHANGE_POINT_H0_MASK             (0xFFFF_FFFF << RAS_DES_EINJ6_CHANGE_POINT_H0_POSITION)

// Error Injection Control 6 (Change Point H1)
#define RAS_DES_EINJ6_CHANGE_POINT_H1_POSITION         0
#define RAS_DES_EINJ6_CHANGE_POINT_H1_MASK             (0xFFFF_FFFF << RAS_DES_EINJ6_CHANGE_POINT_H1_POSITION)

// Error Injection Control 6 (Change Point H2)
#define RAS_DES_EINJ6_CHANGE_POINT_H2_POSITION         0
#define RAS_DES_EINJ6_CHANGE_POINT_H2_MASK             (0xFFFF_FFFF << RAS_DES_EINJ6_CHANGE_POINT_H2_POSITION)

// Error Injection Control 6 (Change Point H3)
#define RAS_DES_EINJ6_CHANGE_POINT_H3_POSITION         0
#define RAS_DES_EINJ6_CHANGE_POINT_H3_MASK             (0xFFFF_FFFF << RAS_DES_EINJ6_CHANGE_POINT_H3_POSITION)

// Error Injection Control 6 (Change Value H0)
#define RAS_DES_EINJ6_CHANGE_VALUE_H0_POSITION         0
#define RAS_DES_EINJ6_CHANGE_VALUE_H0_MASK             (0xFFFF_FFFF << RAS_DES_EINJ6_CHANGE_VALUE_H0_POSITION)

// Error Injection Control 6 (Change Value H1)
#define RAS_DES_EINJ6_CHANGE_VALUE_H1_POSITION         0
#define RAS_DES_EINJ6_CHANGE_VALUE_H1_MASK             (0xFFFF_FFFF << RAS_DES_EINJ6_CHANGE_VALUE_H1_POSITION)

// Error Injection Control 6 (Change Value H2)
#define RAS_DES_EINJ6_CHANGE_VALUE_H2_POSITION         0
#define RAS_DES_EINJ6_CHANGE_VALUE_H2_MASK             (0xFFFF_FFFF << RAS_DES_EINJ6_CHANGE_VALUE_H2_POSITION)

// Error Injection Control 6 (Change Value H3)
#define RAS_DES_EINJ6_CHANGE_VALUE_H3_POSITION         0
#define RAS_DES_EINJ6_CHANGE_VALUE_H3_MASK             (0xFFFF_FFFF << RAS_DES_EINJ6_CHANGE_VALUE_H3_POSITION)

// Error Injection Control 6 (Packet Error)
#define RAS_DES_EINJ6_COUNT_POSITION                   0
#define RAS_DES_EINJ6_COUNT_MASK                       (0xFF << RAS_DES_EINJ6_COUNT_POSITION)
#define RAS_DES_EINJ6_INVERTED_CONTROL_POSITION        8
#define RAS_DES_EINJ6_INVERTED_CONTROL_MASK            (0x1 << RAS_DES_EINJ6_INVERTED_CONTROL_POSITION)
#define RAS_DES_EINJ6_PACKET_TYPE_POSITION             9
#define RAS_DES_EINJ6_PACKET_TYPE_MASK                 (0x7 << RAS_DES_EINJ6_PACKET_TYPE_POSITION)

// Silicon Debug Control1
#define RAS_DES_FORCE_DETECT_LANE_POSITION             0
#define RAS_DES_FORCE_DETECT_LANE_MASK                 (0xFFFF << RAS_DES_FORCE_DETECT_LANE_POSITION)
#define RAS_DES_FORCE_DETECT_LANE_EN_POSITION          16
#define RAS_DES_FORCE_DETECT_LANE_EN_MASK              (0x1 << RAS_DES_FORCE_DETECT_LANE_EN_POSITION)
#define RAS_DES_TX_EIOS_NUM_POSITION                   20
#define RAS_DES_TX_EIOS_NUM_MASK                       (0x3 << RAS_DES_TX_EIOS_NUM_POSITION)
#define RAS_DES_LOW_POWER_INTERVAL_POSITION            22
#define RAS_DES_LOW_POWER_INTERVAL_MASK                (0x3 << RAS_DES_LOW_POWER_INTERVAL_POSITION)

// Silicon Debug Control2
#define RAS_DES_HOLD_LTSSM_POSITION                    0
#define RAS_DES_HOLD_LTSSM_MASK                        (0x1 << RAS_DES_HOLD_LTSSM_POSITION)
#define RAS_DES_RECOVERY_REQUEST_POSITION              1
#define RAS_DES_RECOVERY_REQUEST_MASK                  (0x1 << RAS_DES_RECOVERY_REQUEST_POSITION)
#define RAS_DES_NOACK_FORCE_LINKDOWN_POSITION          2
#define RAS_DES_NOACK_FORCE_LINKDOWN_MASK              (0x1 << RAS_DES_NOACK_FORCE_LINKDOWN_POSITION)
#define RAS_DES_DIRECT_RECIDLE_TO_CONFIG_POSITION      8
#define RAS_DES_DIRECT_RECIDLE_TO_CONFIG_MASK          (0x1 << RAS_DES_DIRECT_RECIDLE_TO_CONFIG_POSITION)
#define RAS_DES_DIRECT_POLCOMP_TO_DETECT_POSITION      9
#define RAS_DES_DIRECT_POLCOMP_TO_DETECT_MASK          (0x1 << RAS_DES_DIRECT_POLCOMP_TO_DETECT_POSITION)
#define RAS_DES_DIRECT_LPBKSLV_TO_EXIT_POSITION        10
#define RAS_DES_DIRECT_LPBKSLV_TO_EXIT_MASK            (0x1 << RAS_DES_DIRECT_LPBKSLV_TO_EXIT_POSITION)
#define RAS_DES_FRAMING_ERR_RECOVERY_DISABLE_POSITION  16
#define RAS_DES_FRAMING_ERR_RECOVERY_DISABLE_MASK      (0x1 << RAS_DES_FRAMING_ERR_RECOVERY_DISABLE_POSITION)

// Silicon Debug Control3

// Silicon Debug Control4

// Silicon Debug Status(Layer1 Per-lane)
#define RAS_DES_LANE_SELECT_POSITION                   0
#define RAS_DES_LANE_SELECT_MASK                       (0xF << RAS_DES_LANE_SELECT_POSITION)
#define RAS_DES_PIPE_RXPOLARITY_POSITION               16
#define RAS_DES_PIPE_RXPOLARITY_MASK                   (0x1 << RAS_DES_PIPE_RXPOLARITY_POSITION)
#define RAS_DES_PIPE_DETECT_LANE_POSITION              17
#define RAS_DES_PIPE_DETECT_LANE_MASK                  (0x1 << RAS_DES_PIPE_DETECT_LANE_POSITION)
#define RAS_DES_PIPE_RXVALID_POSITION                  18
#define RAS_DES_PIPE_RXVALID_MASK                      (0x1 << RAS_DES_PIPE_RXVALID_POSITION)
#define RAS_DES_PIPE_RXELECIDLE_POSITION               19
#define RAS_DES_PIPE_RXELECIDLE_MASK                   (0x1 << RAS_DES_PIPE_RXELECIDLE_POSITION)
#define RAS_DES_PIPE_TXELECIDLE_POSITION               20
#define RAS_DES_PIPE_TXELECIDLE_MASK                   (0x1 << RAS_DES_PIPE_TXELECIDLE_POSITION)
#define RAS_DES_DESKEW_POINTER_POSITION                24
#define RAS_DES_DESKEW_POINTER_MASK                    (0xFF << RAS_DES_DESKEW_POINTER_POSITION)

// Silicon Debug Status(Layer1 LTSSM)
#define RAS_DES_FRAMING_ERR_PTR_POSITION               0
#define RAS_DES_FRAMING_ERR_PTR_MASK                   (0x7F << RAS_DES_FRAMING_ERR_PTR_POSITION)
#define RAS_DES_FRAMING_ERR_POSITION                   7
#define RAS_DES_FRAMING_ERR_MASK                       (0x1 << RAS_DES_FRAMING_ERR_POSITION)
#define RAS_DES_PIPE_POWER_DOWN_POSITION               8
#define RAS_DES_PIPE_POWER_DOWN_MASK                   (0x7 << RAS_DES_PIPE_POWER_DOWN_POSITION)
#define RAS_DES_LANE_REVERSAL_POSITION                 15
#define RAS_DES_LANE_REVERSAL_MASK                     (0x1 << RAS_DES_LANE_REVERSAL_POSITION)
#define RAS_DES_LTSSM_VARIABLE_POSITION                16
#define RAS_DES_LTSSM_VARIABLE_MASK                    (0xFFFF << RAS_DES_LTSSM_VARIABLE_POSITION)

// Silicon Debug Status(PM)
#define RAS_DES_INTERNAL_PM_MSTATE_POSITION            0
#define RAS_DES_INTERNAL_PM_MSTATE_MASK                (0x1F << RAS_DES_INTERNAL_PM_MSTATE_POSITION)
#define RAS_DES_INTERNAL_PM_SSTATE_POSITION            8
#define RAS_DES_INTERNAL_PM_SSTATE_MASK                (0xF << RAS_DES_INTERNAL_PM_SSTATE_POSITION)
#define RAS_DES_PME_RESEND_FLAG_POSITION               12
#define RAS_DES_PME_RESEND_FLAG_MASK                   (0x1 << RAS_DES_PME_RESEND_FLAG_POSITION)
#define RAS_DES_L1SUB_STATE_POSITION                   13
#define RAS_DES_L1SUB_STATE_MASK                       (0x7 << RAS_DES_L1SUB_STATE_POSITION)
#define RAS_DES_LATCHED_NFTS_POSITION                  16
#define RAS_DES_LATCHED_NFTS_MASK                      (0xFF << RAS_DES_LATCHED_NFTS_POSITION)

// Silicon Debug Status(Layer2)
#define RAS_DES_TX_TLP_SEQ_NO_POSITION                 0
#define RAS_DES_TX_TLP_SEQ_NO_MASK                     (0xFFF << RAS_DES_TX_TLP_SEQ_NO_POSITION)
#define RAS_DES_RX_ACK_SEQ_NO_POSITION                 12
#define RAS_DES_RX_ACK_SEQ_NO_MASK                     (0xFFF << RAS_DES_RX_ACK_SEQ_NO_POSITION)
#define RAS_DES_DLCMSM_POSITION                        24
#define RAS_DES_DLCMSM_MASK                            (0x3 << RAS_DES_DLCMSM_POSITION)
#define RAS_DES_FC_INIT1_POSITION                      26
#define RAS_DES_FC_INIT1_MASK                          (0x1 << RAS_DES_FC_INIT1_POSITION)
#define RAS_DES_FC_INIT2_POSITION                      27
#define RAS_DES_FC_INIT2_MASK                          (0x1 << RAS_DES_FC_INIT2_POSITION)

// Silicon Debug Status(Layer3 FC)
#define RAS_DES_CREDIT_SEL_VC_POSITION                 0
#define RAS_DES_CREDIT_SEL_VC_MASK                     (0x7 << RAS_DES_CREDIT_SEL_VC_POSITION)
#define RAS_DES_CREDIT_SEL_CREDIT_TYPE_POSITION        3
#define RAS_DES_CREDIT_SEL_CREDIT_TYPE_MASK            (0x1 << RAS_DES_CREDIT_SEL_CREDIT_TYPE_POSITION)
#define RAS_DES_CREDIT_SEL_TLP_TYPE_POSITION           4
#define RAS_DES_CREDIT_SEL_TLP_TYPE_MASK               (0x3 << RAS_DES_CREDIT_SEL_TLP_TYPE_POSITION)
#define RAS_DES_CREDIT_SEL_HD_POSITION                 6
#define RAS_DES_CREDIT_SEL_HD_MASK                     (0x1 << RAS_DES_CREDIT_SEL_HD_POSITION)
#define RAS_DES_CREDIT_DATA0_POSITION                  8
#define RAS_DES_CREDIT_DATA0_MASK                      (0xFFF << RAS_DES_CREDIT_DATA0_POSITION)
#define RAS_DES_CREDIT_DATA1_POSITION                  20
#define RAS_DES_CREDIT_DATA1_MASK                      (0xFFF << RAS_DES_CREDIT_DATA1_POSITION)

// Silicon Debug Status(Layer3)
#define RAS_DES_MFTLP_POINTER_POSITION                 0
#define RAS_DES_MFTLP_POINTER_MASK                     (0x7F << RAS_DES_MFTLP_POINTER_POSITION)
#define RAS_DES_MFTLP_STATUS_POSITION                  7
#define RAS_DES_MFTLP_STATUS_MASK                      (0x1 << RAS_DES_MFTLP_STATUS_POSITION)

// Silicon Debug EQ Control1
#define RAS_DES_EQ_LANE_SEL_POSITION                   0
#define RAS_DES_EQ_LANE_SEL_MASK                       (0xF << RAS_DES_EQ_LANE_SEL_POSITION)
#define RAS_DES_EQ_RATE_SEL_POSITION                   4
#define RAS_DES_EQ_RATE_SEL_MASK                       (0x1 << RAS_DES_EQ_RATE_SEL_POSITION)
#define RAS_DES_EXT_EQ_TIMEOUT_POSITION                8
#define RAS_DES_EXT_EQ_TIMEOUT_MASK                    (0x3 << RAS_DES_EXT_EQ_TIMEOUT_POSITION)
#define RAS_DES_EVAL_INTERVAL_TIME_POSITION            16
#define RAS_DES_EVAL_INTERVAL_TIME_MASK                (0x3 << RAS_DES_EVAL_INTERVAL_TIME_POSITION)
#define RAS_DES_FOM_TARGET_ENABLE_POSITION             23
#define RAS_DES_FOM_TARGET_ENABLE_MASK                 (0x1 << RAS_DES_FOM_TARGET_ENABLE_POSITION)
#define RAS_DES_FOM_TARGET_POSITION                    24
#define RAS_DES_FOM_TARGET_MASK                        (0xFF << RAS_DES_FOM_TARGET_POSITION)

// Silicon Debug EQ Control2
#define RAS_DES_FORCE_LOCAL_TX_PRE_CURSOR_POSITION     0
#define RAS_DES_FORCE_LOCAL_TX_PRE_CURSOR_MASK         (0x3F << RAS_DES_FORCE_LOCAL_TX_PRE_CURSOR_POSITION)
#define RAS_DES_FORCE_LOCAL_TX_CURSOR_POSITION         6
#define RAS_DES_FORCE_LOCAL_TX_CURSOR_MASK             (0x3F << RAS_DES_FORCE_LOCAL_TX_CURSOR_POSITION)
#define RAS_DES_FORCE_LOCAL_TX_POST_CURSOR_POSITION    12
#define RAS_DES_FORCE_LOCAL_TX_POST_CURSOR_MASK        (0x3F << RAS_DES_FORCE_LOCAL_TX_POST_CURSOR_POSITION)
#define RAS_DES_FORCE_LOCAL_RX_HINT_POSITION           18
#define RAS_DES_FORCE_LOCAL_RX_HINT_MASK               (0x7 << RAS_DES_FORCE_LOCAL_RX_HINT_POSITION)
#define RAS_DES_FORCE_LOCAL_TX_PRESET_POSITION         24
#define RAS_DES_FORCE_LOCAL_TX_PRESET_MASK             (0xF << RAS_DES_FORCE_LOCAL_TX_PRESET_POSITION)
#define RAS_DES_FORCE_LOCAL_TX_COEF_ENABLE_POSITION    28
#define RAS_DES_FORCE_LOCAL_TX_COEF_ENABLE_MASK        (0x1 << RAS_DES_FORCE_LOCAL_TX_COEF_ENABLE_POSITION)
#define RAS_DES_FORCE_LOCAL_RX_HINT_ENABLE_POSITION    29
#define RAS_DES_FORCE_LOCAL_RX_HINT_ENABLE_MASK        (0x1 << RAS_DES_FORCE_LOCAL_RX_HINT_ENABLE_POSITION)
#define RAS_DES_FORCE_LOCAL_TX_PRESET_ENABLE_POSITION  30
#define RAS_DES_FORCE_LOCAL_TX_PRESET_ENABLE_MASK      (0x1 << RAS_DES_FORCE_LOCAL_TX_PRESET_ENABLE_POSITION)

// Silicon Debug EQ Control3
#define RAS_DES_FORCE_REMOTE_TX_PRE_CURSOR_POSITION    0
#define RAS_DES_FORCE_REMOTE_TX_PRE_CURSOR_MASK        (0x3F << RAS_DES_FORCE_REMOTE_TX_PRE_CURSOR_POSITION)
#define RAS_DES_FORCE_REMOTE_TX_CURSOR_POSITION        6
#define RAS_DES_FORCE_REMOTE_TX_CURSOR_MASK            (0x3F << RAS_DES_FORCE_REMOTE_TX_CURSOR_POSITION)
#define RAS_DES_FORCE_REMOTE_TX_POST_CURSOR_POSITION   12
#define RAS_DES_FORCE_REMOTE_TX_POST_CURSOR_MASK       (0x3F << RAS_DES_FORCE_REMOTE_TX_POST_CURSOR_POSITION)
#define RAS_DES_FORCE_REMOTE_TX_COEF_ENABLE_POSITION   28
#define RAS_DES_FORCE_REMOTE_TX_COEF_ENABLE_MASK       (0x1 << RAS_DES_FORCE_REMOTE_TX_COEF_ENABLE_POSITION)

// Silicon Debug EQ Control4

// Silicon Debug EQ Status1
#define RAS_DES_EQ_SEQUENCE_POSITION                   0
#define RAS_DES_EQ_SEQUENCE_MASK                       (0x1 << RAS_DES_EQ_SEQUENCE_POSITION)
#define RAS_DES_EQ_CONVERGENCE_INFO_POSITION           1
#define RAS_DES_EQ_CONVERGENCE_INFO_MASK               (0x3 << RAS_DES_EQ_CONVERGENCE_INFO_POSITION)
#define RAS_DES_EQ_RULEA_VIOLATION_POSITION            4
#define RAS_DES_EQ_RULEA_VIOLATION_MASK                (0x1 << RAS_DES_EQ_RULEA_VIOLATION_POSITION)
#define RAS_DES_EQ_RULEB_VIOLATION_POSITION            5
#define RAS_DES_EQ_RULEB_VIOLATION_MASK                (0x1 << RAS_DES_EQ_RULEB_VIOLATION_POSITION)
#define RAS_DES_EQ_RULEC_VIOLATION_POSITION            6
#define RAS_DES_EQ_RULEC_VIOLATION_MASK                (0x1 << RAS_DES_EQ_RULEC_VIOLATION_POSITION)
#define RAS_DES_EQ_REJECT_EVENT_POSITION               7
#define RAS_DES_EQ_REJECT_EVENT_MASK                   (0x1 << RAS_DES_EQ_REJECT_EVENT_POSITION)

// Silicon Debug EQ Status2
#define RAS_DES_EQ_LOCAL_PRE_CURSOR_POSITION           0
#define RAS_DES_EQ_LOCAL_PRE_CURSOR_MASK               (0x3F << RAS_DES_EQ_LOCAL_PRE_CURSOR_POSITION)
#define RAS_DES_EQ_LOCAL_CURSOR_POSITION               6
#define RAS_DES_EQ_LOCAL_CURSOR_MASK                   (0x3F << RAS_DES_EQ_LOCAL_CURSOR_POSITION)
#define RAS_DES_EQ_LOCAL_POST_CURSOR_POSITION          12
#define RAS_DES_EQ_LOCAL_POST_CURSOR_MASK              (0x3F << RAS_DES_EQ_LOCAL_POST_CURSOR_POSITION)
#define RAS_DES_EQ_LOCAL_RX_HINT_POSITION              18
#define RAS_DES_EQ_LOCAL_RX_HINT_MASK                  (0x7 << RAS_DES_EQ_LOCAL_RX_HINT_POSITION)
#define RAS_DES_EQ_LOCAL_FOM_VALUE_POSITION            24
#define RAS_DES_EQ_LOCAL_FOM_VALUE_MASK                (0xFF << RAS_DES_EQ_LOCAL_FOM_VALUE_POSITION)

// Silicon Debug EQ Status3
#define RAS_DES_EQ_REMOTE_PRE_CURSOR_POSITION          0
#define RAS_DES_EQ_REMOTE_PRE_CURSOR_MASK              (0x3F << RAS_DES_EQ_REMOTE_PRE_CURSOR_POSITION)
#define RAS_DES_EQ_REMOTE_CURSOR_POSITION              6
#define RAS_DES_EQ_REMOTE_CURSOR_MASK                  (0x3F << RAS_DES_EQ_REMOTE_CURSOR_POSITION)
#define RAS_DES_EQ_REMOTE_POST_CURSOR_POSITION         12
#define RAS_DES_EQ_REMOTE_POST_CURSOR_MASK             (0x3F << RAS_DES_EQ_REMOTE_POST_CURSOR_POSITION)
#define RAS_DES_EQ_REMOTE_LF_POSITION                  18
#define RAS_DES_EQ_REMOTE_LF_MASK                      (0x3F << RAS_DES_EQ_REMOTE_LF_POSITION)
#define RAS_DES_EQ_REMOTE_FS_POSITION                  24
#define RAS_DES_EQ_REMOTE_FS_MASK                      (0x3F << RAS_DES_EQ_REMOTE_FS_POSITION)

// Silicon Debug EQ Status4


/////////////////////////////////////////////
// PCI Express cap structure
////////////////////////////////////////////

#define  PCIE_CAP_PCIE_CAP_ID_POSITION 0
#define  PCIE_CAP_PCIE_CAP_ID_MASK (0xff << PCIE_CAP_PCIE_CAP_ID_POSITION)
#define  PCIE_CAP_PCIE_NEXT_CAP_PTR_POSITION 8
#define  PCIE_CAP_PCIE_NEXT_CAP_PTR_MASK (0xff << PCIE_CAP_PCIE_NEXT_CAP_PTR_POSITION)
#define  PCIE_CAP_PCIE_CAP_VER_POSITION 16
#define  PCIE_CAP_PCIE_CAP_VER_MASK (0xf  << PCIE_CAP_PCIE_CAP_VER_POSITION)
#define  PCIE_CAP_DEV_PORT_TYPE_POSITION 20
#define  PCIE_CAP_DEV_PORT_TYPE_MASK (0xf  << PCIE_CAP_DEV_PORT_TYPE_POSITION)
#define  PCIE_CAP_SLOT_IMP_POSITION 24
#define  PCIE_CAP_SLOT_IMP_MASK (0x1  << PCIE_CAP_SLOT_IMP_POSITION)
#define  PCIE_CAP_INT_MSG_NUM_POSITION 25
#define  PCIE_CAP_INT_MSG_NUM_MASK (0x1f << PCIE_CAP_INT_MSG_NUM_POSITION)
#define  PCIE_CAP_RSVD_POSITION 30
#define  PCIE_CAP_RSVD_MASK (0x1 << PCIE_CAP_RSVD_POSITION)

#define  PCIE_CAP_MAX_PAYLOAD_SIZE_POSITION 0
#define  PCIE_CAP_MAX_PAYLOAD_SIZE_MASK (0x7 << PCIE_CAP_MAX_PAYLOAD_SIZE_POSITION)
#define  PCIE_CAP_PHANTOM_FUNC_SUPPORT_POSITION 3
#define  PCIE_CAP_PHANTOM_FUNC_SUPPORT_MASK (0x3  << PCIE_CAP_PHANTOM_FUNC_SUPPORT_POSITION)
#define  PCIE_CAP_EXT_TAG_SUPP_POSITION 5
#define  PCIE_CAP_EXT_TAG_SUPP_MASK (0x1 << PCIE_CAP_EXT_TAG_SUPP_POSITION)
#define  PCIE_CAP_EP_L0S_ACCPT_LATENCY_POSITION 6
#define  PCIE_CAP_EP_L0S_ACCPT_LATENCY_MASK (0x7 << PCIE_CAP_EP_L0S_ACCPT_LATENCY_POSITION)
#define  PCIE_CAP_EP_L1_ACCPT_LATENCY_POSITION 9
#define  PCIE_CAP_EP_L1_ACCPT_LATENCY_MASK (0x7  << PCIE_CAP_EP_L1_ACCPT_LATENCY_POSITION)
#define  PCIE_CAP_ROLE_BASED_ERR_REPORT_POSITION 15
#define  PCIE_CAP_ROLE_BASED_ERR_REPORT_MASK (0x1 << PCIE_CAP_ROLE_BASED_ERR_REPORT_POSITION)
#define  PCIE_CAP_CAP_SLOT_PWR_LMT_VALUE_POSITION 18
#define  PCIE_CAP_CAP_SLOT_PWR_LMT_VALUE_MASK (0xff << PCIE_CAP_CAP_SLOT_PWR_LMT_VALUE_POSITION)
#define  PCIE_CAP_CAP_SLOT_PWR_LMT_SCALE_POSITION 26
#define  PCIE_CAP_CAP_SLOT_PWR_LMT_SCALE_MASK (0x3 << PCIE_CAP_CAP_SLOT_PWR_LMT_SCALE_POSITION)
#define  PCIE_CAP_FLR_CAP_POSITION 28
#define  PCIE_CAP_FLR_CAP_MASK (0x1 << PCIE_CAP_FLR_CAP_POSITION)

#define  PCIE_CAP_CORR_ERR_REPORT_EN_POSITION 0
#define  PCIE_CAP_CORR_ERR_REPORT_EN_MASK (0x1 << PCIE_CAP_CORR_ERR_REPORT_EN_POSITION)
#define  PCIE_CAP_NON_FATAL_ERR_REPORT_EN_POSITION 1
#define  PCIE_CAP_NON_FATAL_ERR_REPORT_EN_MASK (0x1 << PCIE_CAP_NON_FATAL_ERR_REPORT_EN_POSITION)
#define  PCIE_CAP_FATAL_ERR_REPORT_EN_POSITION 2
#define  PCIE_CAP_FATAL_ERR_REPORT_EN_MASK (0x1 << PCIE_CAP_FATAL_ERR_REPORT_EN_POSITION)
#define  PCIE_CAP_UNSUPPORT_REQ_REP_EN_POSITION 3
#define  PCIE_CAP_UNSUPPORT_REQ_REP_EN_MASK (0x1 << PCIE_CAP_UNSUPPORT_REQ_REP_EN_POSITION)
#define  PCIE_CAP_EN_REL_ORDER_POSITION 4
#define  PCIE_CAP_EN_REL_ORDER_MASK (0x1 << PCIE_CAP_EN_REL_ORDER_POSITION)
#define  PCIE_CAP_MAX_PAYLOAD_SIZE_CS_POSITION 5
#define  PCIE_CAP_MAX_PAYLOAD_SIZE_CS_MASK (0x7 << PCIE_CAP_MAX_PAYLOAD_SIZE_CS_POSITION)
#define  PCIE_CAP_EXT_TAG_EN_POSITION 8
#define  PCIE_CAP_EXT_TAG_EN_MASK (0x1 << PCIE_CAP_EXT_TAG_EN_POSITION)
#define  PCIE_CAP_PHANTOM_FUNC_EN_POSITION 9
#define  PCIE_CAP_PHANTOM_FUNC_EN_MASK (0x1 << PCIE_CAP_PHANTOM_FUNC_EN_POSITION)
#define  PCIE_CAP_AUX_POWER_PM_EN_POSITION 10
#define  PCIE_CAP_AUX_POWER_PM_EN_MASK (0x1 << PCIE_CAP_AUX_POWER_PM_EN_POSITION)
#define  PCIE_CAP_EN_NO_SNOOP_POSITION 11
#define  PCIE_CAP_EN_NO_SNOOP_MASK (0x1 << PCIE_CAP_EN_NO_SNOOP_POSITION)
#define  PCIE_CAP_MAX_READ_REQ_SIZE_POSITION 12
#define  PCIE_CAP_MAX_READ_REQ_SIZE_MASK (0x7 << PCIE_CAP_MAX_READ_REQ_SIZE_POSITION)
#define  PCIE_CAP_BRIDGE_CONFIG_RETRY_EN_POSITION 15
#define  PCIE_CAP_BRIDGE_CONFIG_RETRY_EN_MASK (0x1 << PCIE_CAP_BRIDGE_CONFIG_RETRY_EN_POSITION)
#define  PCIE_CAP_INITIATE_FLR_POSITION 15
#define  PCIE_CAP_INITIATE_FLR_MASK (0x1 << PCIE_CAP_INITIATE_FLR_POSITION)
#define  PCIE_CAP_CORR_ERR_DETECTED_POSITION 16
#define  PCIE_CAP_CORR_ERR_DETECTED_MASK (0x1 << PCIE_CAP_CORR_ERR_DETECTED_POSITION)
#define  PCIE_CAP_NON_FATAL_ERR_DETECTED_POSITION 17
#define  PCIE_CAP_NON_FATAL_ERR_DETECTED_MASK (0x1 << PCIE_CAP_NON_FATAL_ERR_DETECTED_POSITION)
#define  PCIE_CAP_FATAL_ERR_DETECTED_POSITION 18
#define  PCIE_CAP_FATAL_ERR_DETECTED_MASK (0x1 << PCIE_CAP_FATAL_ERR_DETECTED_POSITION)
#define  PCIE_CAP_UNSUPPORTED_REQ_DETECTED_POSITION 19
#define  PCIE_CAP_UNSUPPORTED_REQ_DETECTED_MASK (0x1 << PCIE_CAP_UNSUPPORTED_REQ_DETECTED_POSITION)
#define  PCIE_CAP_AUX_POWER_DETECTED_POSITION 20
#define  PCIE_CAP_AUX_POWER_DETECTED_MASK (0x1 << PCIE_CAP_AUX_POWER_DETECTED_POSITION)
#define  PCIE_CAP_TRANS_PENDING_POSITION 21
#define  PCIE_CAP_TRANS_PENDING_MASK (0x1 << PCIE_CAP_TRANS_PENDING_POSITION)

#define  PCIE_CAP_MAX_LINK_SPEED_POSITION 0
#define  PCIE_CAP_MAX_LINK_SPEED_MASK (0xf << PCIE_CAP_MAX_LINK_SPEED_POSITION)
#define  PCIE_CAP_MAX_LINK_WIDTH_POSITION 4
#define  PCIE_CAP_MAX_LINK_WIDTH_MASK (0x3f << PCIE_CAP_MAX_LINK_WIDTH_POSITION)
#define  PCIE_CAP_ACTIVE_STATE_LINK_PM_SUPPORT_POSITION 10
#define  PCIE_CAP_ACTIVE_STATE_LINK_PM_SUPPORT_MASK (0x3 << PCIE_CAP_ACTIVE_STATE_LINK_PM_SUPPORT_POSITION)
#define  PCIE_CAP_L0S_EXIT_LATENCY_POSITION 12
#define  PCIE_CAP_L0S_EXIT_LATENCY_MASK (0x7 << PCIE_CAP_L0S_EXIT_LATENCY_POSITION)
#define  PCIE_CAP_L1_EXIT_LATENCY_POSITION 15
#define  PCIE_CAP_L1_EXIT_LATENCY_MASK (0x7 << PCIE_CAP_L1_EXIT_LATENCY_POSITION)
#define  PCIE_CAP_CLOCK_POWER_MAN_POSITION 18
#define  PCIE_CAP_CLOCK_POWER_MAN_MASK (0x1 << PCIE_CAP_CLOCK_POWER_MAN_POSITION)
#define  PCIE_CAP_SURPRISE_DOWN_ERR_REP_CAP_POSITION 19
#define  PCIE_CAP_SURPRISE_DOWN_ERR_REP_CAP_MASK (0x1 << PCIE_CAP_SURPRISE_DOWN_ERR_REP_CAP_POSITION)
#define  PCIE_CAP_DLL_ACTIVE_REP_CAP_POSITION 20
#define  PCIE_CAP_DLL_ACTIVE_REP_CAP_MASK (0x1 << PCIE_CAP_DLL_ACTIVE_REP_CAP_POSITION)
#define  PCIE_CAP_LINK_BW_NOT_CAP_POSITION 21
#define  PCIE_CAP_LINK_BW_NOT_CAP_MASK (0x1 << PCIE_CAP_LINK_BW_NOT_CAP_POSITION)
#define  PCIE_CAP_ASPM_OPT_COMPLIANCE_POSITION 22
#define  PCIE_CAP_ASPM_OPT_COMPLIANCE_MASK (0x1 << PCIE_CAP_ASPM_OPT_COMPLIANCE_POSITION)
#define  PCIE_CAP_PORT_NUM_POSITION 24
#define  PCIE_CAP_PORT_NUM_MASK (0xff << PCIE_CAP_PORT_NUM_POSITION)
#define  SHADOW_PCIE_CAP_L0S_EXIT_LATENCY_POSITION 12
#define  SHADOW_PCIE_CAP_L0S_EXIT_LATENCY_MASK (0x7 << SHADOW_PCIE_CAP_L0S_EXIT_LATENCY_POSITION)
#define  SHADOW_PCIE_CAP_L1_EXIT_LATENCY_POSITION 15
#define  SHADOW_PCIE_CAP_L1_EXIT_LATENCY_MASK (0x7 << SHADOW_PCIE_CAP_L1_EXIT_LATENCY_POSITION)

// Link Control Register
#define  PCIE_CAP_ACTIVE_STATE_LINK_PM_CONTROL_POSITION 0
#define  PCIE_CAP_ACTIVE_STATE_LINK_PM_CONTROL_MASK (0x3 << PCIE_CAP_ACTIVE_STATE_LINK_PM_CONTROL_POSITION)
#define  PCIE_CAP_RCB_POSITION 3
#define  PCIE_CAP_RCB_MASK (0x1 << PCIE_CAP_RCB_POSITION)
#define  PCIE_CAP_LINK_DISABLE_POSITION 4
#define  PCIE_CAP_LINK_DISABLE_MASK (0x1 << PCIE_CAP_LINK_DISABLE_POSITION)
#define  PCIE_CAP_RETRAIN_LINK_POSITION 5
#define  PCIE_CAP_RETRAIN_LINK_MASK (0x1 << PCIE_CAP_RETRAIN_LINK_POSITION)
#define  PCIE_CAP_COMMON_CLK_CONFIG_POSITION 6
#define  PCIE_CAP_COMMON_CLK_CONFIG_MASK (0x1 << PCIE_CAP_COMMON_CLK_CONFIG_POSITION)
#define  PCIE_CAP_EXTENDED_SYNCH_POSITION 7
#define  PCIE_CAP_EXTENDED_SYNCH_MASK (0x1 << PCIE_CAP_EXTENDED_SYNCH_POSITION)
#define  PCIE_CAP_EN_CLK_POWER_MAN_POSITION 8
#define  PCIE_CAP_EN_CLK_POWER_MAN_MASK (0x1 << PCIE_CAP_EN_CLK_POWER_MAN_POSITION)
#define  PCIE_CAP_HW_AUTO_WIDTH_DISABLE_POSITION 9
#define  PCIE_CAP_HW_AUTO_WIDTH_DISABLE_MASK (0x1 << PCIE_CAP_HW_AUTO_WIDTH_DISABLE_POSITION)
#define  PCIE_CAP_LINK_BW_MAN_INT_EN_POSITION 10
#define  PCIE_CAP_LINK_BW_MAN_INT_EN_MASK (0x1 << PCIE_CAP_LINK_BW_MAN_INT_EN_POSITION)
#define  PCIE_CAP_LINK_AUTO_BW_INT_EN_POSITION 11
#define  PCIE_CAP_LINK_AUTO_BW_INT_EN_MASK (0x1 << PCIE_CAP_LINK_AUTO_BW_INT_EN_POSITION)
#define  PCIE_CAP_DRS_SIGNALING_CONTROL_POSITION 14
#define  PCIE_CAP_DRS_SIGNALING_CONTROL_MASK (0x3 << PCIE_CAP_DRS_SIGNALING_CONTROL_POSITION)

#define  PCIE_CAP_LINK_SPEED_POSITION 16
#define  PCIE_CAP_LINK_SPEED_MASK (0xf << PCIE_CAP_LINK_SPEED_POSITION)
#define  PCIE_CAP_NEGO_LINK_WIDTH_POSITION 20
#define  PCIE_CAP_NEGO_LINK_WIDTH_MASK (0x3f << PCIE_CAP_NEGO_LINK_WIDTH_POSITION)
#define  PCIE_CAP_LINK_TRAINING_POSITION 27
#define  PCIE_CAP_LINK_TRAINING_MASK (0x1 << PCIE_CAP_LINK_TRAINING_POSITION)
#define  PCIE_CAP_SLOT_CLK_CONFIG_POSITION 28
#define  PCIE_CAP_SLOT_CLK_CONFIG_MASK (0x1 << PCIE_CAP_SLOT_CLK_CONFIG_POSITION)
#define  PCIE_CAP_DLL_ACTIVE_POSITION 29
#define  PCIE_CAP_DLL_ACTIVE_MASK (0x1 << PCIE_CAP_DLL_ACTIVE_POSITION)
#define  PCIE_CAP_LINK_BW_MAN_STATUS_POSITION 30
#define  PCIE_CAP_LINK_BW_MAN_STATUS_MASK (0x1 << PCIE_CAP_LINK_BW_MAN_STATUS_POSITION)
#define  PCIE_CAP_LINK_AUTO_BW_STATUS_POSITION 31
#define  PCIE_CAP_LINK_AUTO_BW_STATUS_MASK (0x1 << PCIE_CAP_LINK_AUTO_BW_STATUS_POSITION)

// Slot Capabilities
#define  PCIE_CAP_ATTENTION_INDICATOR_BUTTON_POSITION 0
#define  PCIE_CAP_ATTENTION_INDICATOR_BUTTON_MASK (0x1 << PCIE_CAP_ATTENTION_INDICATOR_BUTTON_POSITION)
#define  PCIE_CAP_POWER_CONTROLLER_POSITION 1
#define  PCIE_CAP_POWER_CONTROLLER_MASK (0x1 << PCIE_CAP_POWER_CONTROLLER_POSITION)
#define  PCIE_CAP_MRL_SENSOR_POSITION 2
#define  PCIE_CAP_MRL_SENSOR_MASK (0x1 << PCIE_CAP_MRL_SENSOR_POSITION)
#define  PCIE_CAP_ATTENTION_INDICATOR_POSITION 3
#define  PCIE_CAP_ATTENTION_INDICATOR_MASK (0x1 << PCIE_CAP_ATTENTION_INDICATOR_POSITION)
#define  PCIE_CAP_POWER_INDICATOR_POSITION 4
#define  PCIE_CAP_POWER_INDICATOR_MASK (0x1 << PCIE_CAP_POWER_INDICATOR_POSITION)
#define  PCIE_CAP_HOT_PLUG_SURPRISE_POSITION 5
#define  PCIE_CAP_HOT_PLUG_SURPRISE_MASK (0x1 << PCIE_CAP_HOT_PLUG_SURPRISE_POSITION)
#define  PCIE_CAP_HOT_PLUG_CAPABLE_POSITION 6
#define  PCIE_CAP_HOT_PLUG_CAPABLE_MASK (0x1 << PCIE_CAP_HOT_PLUG_CAPABLE_POSITION)
#define  PCIE_CAP_SLOT_POWER_LIMIT_VALUE_POSITION 7
#define  PCIE_CAP_SLOT_POWER_LIMIT_VALUE_MASK (0xff << PCIE_CAP_SLOT_POWER_LIMIT_VALUE_POSITION)
#define  PCIE_CAP_SLOT_POWER_LIMIT_SCALE_POSITION 15
#define  PCIE_CAP_SLOT_POWER_LIMIT_SCALE_MASK (0x3 << PCIE_CAP_SLOT_POWER_LIMIT_SCALE_POSITION)
#define  PCIE_CAP_ELECTROMECH_INTERLOCK_POSITION 17
#define  PCIE_CAP_ELECTROMECH_INTERLOCK_MASK (0x1 << PCIE_CAP_ELECTROMECH_INTERLOCK_POSITION)
#define  PCIE_CAP_NO_CMD_CPL_SUPPORT_POSITION 18
#define  PCIE_CAP_NO_CMD_CPL_SUPPORT_MASK (0x1 << PCIE_CAP_NO_CMD_CPL_SUPPORT_POSITION)
#define  PCIE_CAP_PHY_SLOT_NUM_POSITION 19
#define  PCIE_CAP_PHY_SLOT_NUM_MASK (0x1fff << PCIE_CAP_PHY_SLOT_NUM_POSITION)

#define  PCIE_CAP_ATTENTION_BUTTON_PRESSED_EN_POSITION 0
#define  PCIE_CAP_ATTENTION_BUTTON_PRESSED_EN_MASK (0x1 << PCIE_CAP_ATTENTION_BUTTON_PRESSED_EN_POSITION)
#define  PCIE_CAP_POWER_FAULT_DETECTED_EN_POSITION 1
#define  PCIE_CAP_POWER_FAULT_DETECTED_EN_MASK (0x1 << PCIE_CAP_POWER_FAULT_DETECTED_EN_POSITION)
#define  PCIE_CAP_MRL_SENSOR_CHANGED_EN_POSITION 2
#define  PCIE_CAP_MRL_SENSOR_CHANGED_EN_MASK (0x1 << PCIE_CAP_MRL_SENSOR_CHANGED_EN_POSITION)
#define  PCIE_CAP_PRESENCE_DETECT_CHANGE_EN_POSITION 3
#define  PCIE_CAP_PRESENCE_DETECT_CHANGE_EN_MASK (0x1 << PCIE_CAP_PRESENCE_DETECT_CHANGE_EN_POSITION)
#define  PCIE_CAP_CMD_CPL_INT_EN_POSITION 4
#define  PCIE_CAP_CMD_CPL_INT_EN_MASK (0x1 << PCIE_CAP_CMD_CPL_INT_EN_POSITION)
#define  PCIE_CAP_HOT_PLUG_INT_EN_POSITION 5
#define  PCIE_CAP_HOT_PLUG_INT_EN_MASK (0x1 << PCIE_CAP_HOT_PLUG_INT_EN_POSITION)
#define  PCIE_CAP_ATTENTION_INDICATOR_CTRL_POSITION 6
#define  PCIE_CAP_ATTENTION_INDICATOR_CTRL_MASK (0x3 << PCIE_CAP_ATTENTION_INDICATOR_CTRL_POSITION)
#define  PCIE_CAP_POWER_INDICATOR_CTRL_POSITION 8
#define  PCIE_CAP_POWER_INDICATOR_CTRL_MASK (0x3 << PCIE_CAP_POWER_INDICATOR_CTRL_POSITION)
#define  PCIE_CAP_POWER_CONTROLLER_CTRL_POSITION 10
#define  PCIE_CAP_POWER_CONTROLLER_CTRL_MASK (0x1 << PCIE_CAP_POWER_CONTROLLER_CTRL_POSITION)
#define  PCIE_CAP_ELECTROMECH_INTERLOCK_CTRL_POSITION 11
#define  PCIE_CAP_ELECTROMECH_INTERLOCK_CTRL_MASK (0x1 << PCIE_CAP_ELECTROMECH_INTERLOCK_CTRL_POSITION)
#define  PCIE_CAP_DLL_STATE_CHANGED_EN_POSITION 12
#define  PCIE_CAP_DLL_STATE_CHANGED_EN_MASK (0x1 << PCIE_CAP_DLL_STATE_CHANGED_EN_POSITION)
#define  PCIE_CAP_ATTENTION_BUTTON_PRESSED_POSITION 16
#define  PCIE_CAP_ATTENTION_BUTTON_PRESSED_MASK (0x1 << PCIE_CAP_ATTENTION_BUTTON_PRESSED_POSITION)
#define  PCIE_CAP_POWER_FAULT_DETECTED_POSITION 17
#define  PCIE_CAP_POWER_FAULT_DETECTED_MASK (0x1 << PCIE_CAP_POWER_FAULT_DETECTED_POSITION)
#define  PCIE_CAP_MRL_SENSOR_CHANGED_POSITION 18
#define  PCIE_CAP_MRL_SENSOR_CHANGED_MASK (0x1 << PCIE_CAP_MRL_SENSOR_CHANGED_POSITION)
#define  PCIE_CAP_PRESENCE_DETECTED_CHANGED_POSITION 19
#define  PCIE_CAP_PRESENCE_DETECTED_CHANGED_MASK (0x1 << PCIE_CAP_PRESENCE_DETECTED_CHANGED_POSITION)
#define  PCIE_CAP_CMD_CPLD_POSITION 20
#define  PCIE_CAP_CMD_CPLD_MASK (0x1 << PCIE_CAP_CMD_CPLD_POSITION)
#define  PCIE_CAP_MRL_SENSOR_STATE_POSITION 21
#define  PCIE_CAP_MRL_SENSOR_STATE_MASK (0x1 << PCIE_CAP_MRL_SENSOR_STATE_POSITION)
#define  PCIE_CAP_PRESENCE_DETECT_STATE_POSITION 22
#define  PCIE_CAP_PRESENCE_DETECT_STATE_MASK (0x1 << PCIE_CAP_PRESENCE_DETECT_STATE_POSITION)
#define  PCIE_CAP_ELECTROMECH_INTERLOCK_STATUS_POSITION 23
#define  PCIE_CAP_ELECTROMECH_INTERLOCK_STATUS_MASK (0x1 << PCIE_CAP_ELECTROMECH_INTERLOCK_STATUS_POSITION)
#define  PCIE_CAP_DLL_STATE_CHANGED_POSITION 24
#define  PCIE_CAP_DLL_STATE_CHANGED_MASK (0x1 << PCIE_CAP_DLL_STATE_CHANGED_POSITION)

#define  PCIE_CAP_SYS_ERR_ON_CORR_ERR_EN_POSITION 0
#define  PCIE_CAP_SYS_ERR_ON_CORR_ERR_EN_MASK (0x1 << PCIE_CAP_SYS_ERR_ON_CORR_ERR_EN_POSITION)
#define  PCIE_CAP_SYS_ERR_ON_NON_FATAL_ERR_EN_POSITION 1
#define  PCIE_CAP_SYS_ERR_ON_NON_FATAL_ERR_EN_MASK (0x1 << PCIE_CAP_SYS_ERR_ON_NON_FATAL_ERR_EN_POSITION)
#define  PCIE_CAP_SYS_ERR_ON_FATAL_ERR_EN_POSITION 2
#define  PCIE_CAP_SYS_ERR_ON_FATAL_ERR_EN_MASK (0x1 << PCIE_CAP_SYS_ERR_ON_FATAL_ERR_EN_POSITION)
#define  PCIE_CAP_PME_INT_EN_POSITION 3
#define  PCIE_CAP_PME_INT_EN_MASK (0x1 << PCIE_CAP_PME_INT_EN_POSITION)
#define  PCIE_CAP_CRS_SW_VISIBILITY_EN_POSITION 4
#define  PCIE_CAP_CRS_SW_VISIBILITY_EN_MASK (0x1 << PCIE_CAP_CRS_SW_VISIBILITY_EN_POSITION)
#define  PCIE_CAP_CRS_SW_VISIBILITY_POSITION 16
#define  PCIE_CAP_CRS_SW_VISIBILITY_MASK (0x1 << PCIE_CAP_CRS_SW_VISIBILITY_POSITION)

#define  PCIE_CAP_PME_REQ_ID_POSITION 0
#define  PCIE_CAP_PME_REQ_ID_MASK (0xffff << PCIE_CAP_PME_REQ_ID_POSITION)
#define  PCIE_CAP_PME_STATUS_POSITION 16
#define  PCIE_CAP_PME_STATUS_MASK (0x1 << PCIE_CAP_PME_STATUS_POSITION)
#define  PCIE_CAP_PME_PENDING_POSITION 17
#define  PCIE_CAP_PME_PENDING_MASK (0x1 << PCIE_CAP_PME_PENDING_POSITION)

#define  PCIE_CAP_CPL_TIMEOUT_RANGE_POSITION 0
#define  PCIE_CAP_CPL_TIMEOUT_RANGE_MASK (0xf << PCIE_CAP_CPL_TIMEOUT_RANGE_POSITION)
#define  PCIE_CAP_CPL_TIMEOUT_DISABLE_SUPPORT_POSITION 4
#define  PCIE_CAP_CPL_TIMEOUT_DISABLE_SUPPORT_MASK (0x1 << PCIE_CAP_CPL_TIMEOUT_DISABLE_SUPPORT_POSITION)
#define  PCIE_CAP_ARI_FORWARD_SUPPORT_POSITION 5
#define  PCIE_CAP_ARI_FORWARD_SUPPORT_MASK (0x1 << PCIE_CAP_ARI_FORWARD_SUPPORT_POSITION)
#define  PCIE_CAP_ATOMIC_ROUTING_SUPP_POSITION 6
#define  PCIE_CAP_ATOMIC_ROUTING_SUPP_MASK (0x1 << PCIE_CAP_ATOMIC_ROUTING_SUPP_POSITION)
#define  PCIE_CAP_32_ATOMIC_CPL_SUPP_POSITION 7
#define  PCIE_CAP_32_ATOMIC_CPL_SUPP_MASK (0x1 << PCIE_CAP_32_ATOMIC_CPL_SUPP_POSITION)
#define  PCIE_CAP_64_ATOMIC_CPL_SUPP_POSITION 8
#define  PCIE_CAP_64_ATOMIC_CPL_SUPP_MASK (0x1 << PCIE_CAP_64_ATOMIC_CPL_SUPP_POSITION)
#define  PCIE_CAP_128_CAS_CPL_SUPP_POSITION 9
#define  PCIE_CAP_128_CAS_CPL_SUPP_MASK (0x1 << PCIE_CAP_128_CAS_CPL_SUPP_POSITION)
#define  PCIE_CAP_NO_RO_EN_PR2PR_PAR_POSITION 10
#define  PCIE_CAP_NO_RO_EN_PR2PR_PAR_MASK (0x1 << PCIE_CAP_NO_RO_EN_PR2PR_PAR_POSITION)
#define  PCIE_CAP_LTR_SUPP_POSITION 11
#define  PCIE_CAP_LTR_SUPP_MASK (0x1 << PCIE_CAP_LTR_SUPP_POSITION)

#define  PCIE_CAP_TPH_CMPLT_SUPPORT_0_POSITION 12
#define  PCIE_CAP_TPH_CMPLT_SUPPORT_0_MASK (0x1 << PCIE_CAP_TPH_CMPLT_SUPPORT_0_POSITION)
#define  PCIE_CAP_TPH_CMPLT_SUPPORT_1_POSITION 13
#define  PCIE_CAP_TPH_CMPLT_SUPPORT_1_MASK (0x1 << PCIE_CAP_TPH_CMPLT_SUPPORT_1_POSITION)
#define  PCIE_CAP2_LN_SYS_CLS_POSITION 14
#define  PCIE_CAP2_LN_SYS_CLS_MASK (0x3 <<  PCIE_CAP2_LN_SYS_CLS_POSITION)
#define  PCIE_CAP2_10_BIT_TAG_COMP_POSITION 16
#define  PCIE_CAP2_10_BIT_TAG_COMP_MASK (0x1 <<  PCIE_CAP2_10_BIT_TAG_COMP_POSITION)
#define  PCIE_CAP2_10_BIT_TAG_REQ_POSITION 17
#define  PCIE_CAP2_10_BIT_TAG_REQ_MASK (0x1 <<  PCIE_CAP2_10_BIT_TAG_REQ_POSITION)
#define  PCIE_CAP_OBFF_SUPPORT_POSITION 18
#define  PCIE_CAP_OBFF_SUPPORT_MASK (0x3 << PCIE_CAP_OBFF_SUPPORT_POSITION)
#define  PCIE_CAP2_CFG_EXTND_FMT_SUPPORT_POSITION 20
#define  PCIE_CAP2_CFG_EXTND_FMT_SUPPORT_MASK (0x1 << PCIE_CAP2_CFG_EXTND_FMT_SUPPORT_POSITION)
#define  PCIE_CAP2_CFG_END2END_TLP_PRFX_SUPPORT_POSITION 21
#define  PCIE_CAP2_CFG_END2END_TLP_PRFX_SUPPORT_MASK (0x1 << PCIE_CAP2_CFG_END2END_TLP_PRFX_SUPPORT_POSITION)
#define  PCIE_CAP2_CFG_MAX_END2END_TLP_PRFXS_POSITION 22
#define  PCIE_CAP2_CFG_MAX_END2END_TLP_PRFXS_MASK (0x3 << PCIE_CAP2_CFG_MAX_END2END_TLP_PRFXS_POSITION)
#define  PCIE_CAP_FRS_SUPPORTED_POSITION 31
#define  PCIE_CAP_FRS_SUPPORTED_MASK (0x1 << PCIE_CAP_FRS_SUPPORTED_POSITION)

#define  PCIE_CAP_CPL_TIMEOUT_VALUE_POSITION 0
#define  PCIE_CAP_CPL_TIMEOUT_VALUE_MASK (0xf << PCIE_CAP_CPL_TIMEOUT_VALUE_POSITION)
#define  PCIE_CAP_CPL_TIMEOUT_DISABLE_POSITION 4
#define  PCIE_CAP_CPL_TIMEOUT_DISABLE_MASK (0x1 << PCIE_CAP_CPL_TIMEOUT_DISABLE_POSITION)
#define  PCIE_CAP_ARI_FORWARD_SUPPORT_CS_POSITION 5
#define  PCIE_CAP_ARI_FORWARD_SUPPORT_CS_MASK (0x1 << PCIE_CAP_ARI_FORWARD_SUPPORT_CS_POSITION)
#define  PCIE_CAP_ATOMIC_REQ_EN_POSITION 6
#define  PCIE_CAP_ATOMIC_REQ_EN_MASK (0x1 << PCIE_CAP_ATOMIC_REQ_EN_POSITION)
#define  PCIE_CAP_ATOMIC_EGRESS_BLK_POSITION 7
#define  PCIE_CAP_ATOMIC_EGRESS_BLK_MASK (0x1 << PCIE_CAP_ATOMIC_EGRESS_BLK_POSITION)
#define  PCIE_CAP_IDO_REQ_EN_POSITION 8
#define  PCIE_CAP_IDO_REQ_EN_MASK (0x1 << PCIE_CAP_IDO_REQ_EN_POSITION)
#define  PCIE_CAP_IDO_CPL_EN_POSITION 9
#define  PCIE_CAP_IDO_CPL_EN_MASK (0x1 << PCIE_CAP_IDO_CPL_EN_POSITION)
#define  PCIE_CAP_LTR_EN_POSITION 10
#define  PCIE_CAP_LTR_EN_MASK (0x1 << PCIE_CAP_LTR_EN_POSITION)
#define  PCIE_CAP_10BITS_TAG_REQ_EN_POSITION 12
#define  PCIE_CAP_10BITS_TAG_REQ_EN_MASK (0x1 << PCIE_CAP_10BITS_TAG_REQ_EN_POSITION)
#define  PCIE_CAP_OBFF_EN_POSITION 13
#define  PCIE_CAP_OBFF_EN_MASK (0x3 << PCIE_CAP_OBFF_EN_POSITION)
#define  PCIE_CTRL2_CFG_END2END_TLP_PFX_BLCK_POSITION 15
#define  PCIE_CTRL2_CFG_END2END_TLP_PFX_BLCK_MASK (0x1 << PCIE_CTRL2_CFG_END2END_TLP_PFX_BLCK_POSITION)

#define  PCIE_CAP_SUPPORT_LINK_SPEED_VECTOR_POSITION 1
#define  PCIE_CAP_SUPPORT_LINK_SPEED_VECTOR_MASK (0x7f<< PCIE_CAP_SUPPORT_LINK_SPEED_VECTOR_POSITION)
#define  PCIE_CAP_CROSS_LINK_SUPPORT_POSITION 8
#define  PCIE_CAP_CROSS_LINK_SUPPORT_MASK (0x1 << PCIE_CAP_CROSS_LINK_SUPPORT_POSITION)
#define  PCIE_CAP_RETIMER_PRE_DET_SUPPORT_POSITION 23
#define  PCIE_CAP_RETIMER_PRE_DET_SUPPORT_MASK (0x1 << PCIE_CAP_RETIMER_PRE_DET_SUPPORT_POSITION)
#define  PCIE_CAP_TWO_RETIMERS_PRE_DET_SUPPORT_POSITION 24
#define  PCIE_CAP_TWO_RETIMERS_PRE_DET_SUPPORT_MASK (0x1 << PCIE_CAP_TWO_RETIMERS_PRE_DET_SUPPORT_POSITION)
#define  PCIE_CAP_DRS_SUPPORTED_POSITION 31
#define  PCIE_CAP_DRS_SUPPORTED_MASK (0x1 << PCIE_CAP_DRS_SUPPORTED_POSITION)

#define  PCIE_CAP_TARGET_LINK_SPEED_POSITION 0
#define  PCIE_CAP_TARGET_LINK_SPEED_MASK (0xf << PCIE_CAP_TARGET_LINK_SPEED_POSITION)
#define  PCIE_CAP_ENTER_COMPLIANCE_POSITION 4
#define  PCIE_CAP_ENTER_COMPLIANCE_MASK (0x1 << PCIE_CAP_ENTER_COMPLIANCE_POSITION)
#define  PCIE_CAP_HW_AUTO_SPEED_DISABLE_POSITION 5
#define  PCIE_CAP_HW_AUTO_SPEED_DISABLE_MASK (0x1 << PCIE_CAP_HW_AUTO_SPEED_DISABLE_POSITION)
#define  PCIE_CAP_SEL_DEEMPHASIS_POSITION 6
#define  PCIE_CAP_SEL_DEEMPHASIS_MASK (0x1 << PCIE_CAP_SEL_DEEMPHASIS_POSITION)
#define  PCIE_CAP_TX_MARGIN_POSITION 7
#define  PCIE_CAP_TX_MARGIN_MASK (0x7 << PCIE_CAP_TX_MARGIN_POSITION)
#define  PCIE_CAP_ENTER_MODIFIED_COMPLIANCE_POSITION 10
#define  PCIE_CAP_ENTER_MODIFIED_COMPLIANCE_MASK (0x1 << PCIE_CAP_ENTER_MODIFIED_COMPLIANCE_POSITION)
#define  PCIE_CAP_COMPLIANCE_SOS_POSITION 11
#define  PCIE_CAP_COMPLIANCE_SOS_MASK (0x1 << PCIE_CAP_COMPLIANCE_SOS_POSITION)
#define  PCIE_CAP_COMPLIANCE_PRESET_POSITION 12
#define  PCIE_CAP_COMPLIANCE_PRESET_MASK (0xf << PCIE_CAP_COMPLIANCE_PRESET_POSITION)
#define  PCIE_CAP_CURR_DEEMPHASIS_POSITION 16
#define  PCIE_CAP_CURR_DEEMPHASIS_MASK (0x1 << PCIE_CAP_CURR_DEEMPHASIS_POSITION)
#define  PCIE_CAP_EQ_CPL_POSITION 17
#define  PCIE_CAP_EQ_CPL_MASK (0x1 << PCIE_CAP_EQ_CPL_POSITION)
#define  PCIE_CAP_EQ_CPL_P1_POSITION 18
#define  PCIE_CAP_EQ_CPL_P1_MASK (0x1 << PCIE_CAP_EQ_CPL_P1_POSITION)
#define  PCIE_CAP_EQ_CPL_P2_POSITION 19
#define  PCIE_CAP_EQ_CPL_P2_MASK (0x1 << PCIE_CAP_EQ_CPL_P2_POSITION)
#define  PCIE_CAP_EQ_CPL_P3_POSITION 20
#define  PCIE_CAP_EQ_CPL_P3_MASK (0x1 << PCIE_CAP_EQ_CPL_P3_POSITION)
#define  PCIE_CAP_LINK_EQ_REQ_POSITION 21
#define  PCIE_CAP_LINK_EQ_REQ_MASK (0x1 << PCIE_CAP_LINK_EQ_REQ_POSITION)
#define  PCIE_CAP_DOWNSTREAM_COMPO_PRESENCE_POSITION 28
#define  PCIE_CAP_DOWNSTREAM_COMPO_PRESENCE_MASK (0x7 << PCIE_CAP_DOWNSTREAM_COMPO_PRESENCE_POSITION)
#define  PCIE_CAP_DRS_MESSAGE_RECEIVED_POSITION 31
#define  PCIE_CAP_DRS_MESSAGE_RECEIVED_MASK  (0x1 << PCIE_CAP_DRS_MESSAGE_RECEIVED_POSITION)

#define PCIE_CAP_RETIMER_PRE_DET_POSITION      22
#define PCIE_CAP_RETIMER_PRE_DET_MASK          (0x1 << PCIE_CAP_RETIMER_PRE_DET_POSITION)
#define PCIE_CAP_TWO_RETIMERS_PRE_DET_POSITION 23
#define PCIE_CAP_TWO_RETIMERS_PRE_DET_MASK     (0x1 << PCIE_CAP_TWO_RETIMERS_PRE_DET_POSITION)
#define PCIE_CAP_CROSSLINK_RESOLUTION_POSITION 24
#define PCIE_CAP_CROSSLINK_RESOLUTION_MASK     (0x3 << PCIE_CAP_CROSSLINK_RESOLUTION_POSITION)
#define PCIE_CAP_EQ_16G_CPL_POSITION      23
#define PCIE_CAP_EQ_16G_CPL_MASK          (0x1 << PCIE_CAP_EQ_16G_CPL_POSITION)
#define PCIE_CAP_EQ_16G_CPL_P1_POSITION   24
#define PCIE_CAP_EQ_16G_CPL_P1_MASK       (0x1 << PCIE_CAP_EQ_16G_CPL_P1_POSITION)
#define PCIE_CAP_EQ_16G_CPL_P2_POSITION   25
#define PCIE_CAP_EQ_16G_CPL_P2_MASK       (0x1 << PCIE_CAP_EQ_16G_CPL_P2_POSITION)
#define PCIE_CAP_EQ_16G_CPL_P3_POSITION   26
#define PCIE_CAP_EQ_16G_CPL_P3_MASK       (0x1 << PCIE_CAP_EQ_16G_CPL_P3_POSITION)
#define PCIE_CAP_LINK_EQ_16G_REQ_POSITION 27
#define PCIE_CAP_LINK_EQ_16G_REQ_MASK     (0x1 << PCIE_CAP_LINK_EQ_16G_REQ_POSITION)

//#define  PCIE_CAP_SLOT_CAPABILITIES2_POSITION   0
//#define  PCIE_CAP_SLOT_CAPABILITIES2_MASK       (0xffffffff << PCIE_CAP_SLOT_CAPABILITIES2_POSITION  )
//
//#define  PCIE_CAP_SLOT_CONTROL2_POSITION        0
//#define  PCIE_CAP_SLOT_CONTROL2_MASK            (0xffff << PCIE_CAP_SLOT_CONTROL2_POSITION           )
//#define  PCIE_CAP_SLOT_STATUS2_POSITION         0
//#define  PCIE_CAP_SLOT_STATUS2_MASK             (0xffff << PCIE_CAP_SLOT_STATUS2_POSITION            )


/////////////////////////////////////////////
// Resizable BAR Capability
/////////////////////////////////////////////
#define  RESBAR_CAP_ID_POSITION               0
#define  RESBAR_CAP_ID_MASK                   (0xffff << RESBAR_CAP_ID_POSITION                   )
#define  RESBAR_CAP_VERSION_POSITION          16
#define  RESBAR_CAP_VERSION_MASK              (0xf << RESBAR_CAP_VERSION_POSITION                  )
#define  RESBAR_CAP_NEXT_OFFSET_POSITION      20
#define  RESBAR_CAP_NEXT_OFFSET_MASK          (0xfff << RESBAR_CAP_NEXT_OFFSET_POSITION           )

#define  RESBAR_CAP_BAR_SIZE_1MB_POSITION     4
#define  RESBAR_CAP_BAR_SIZE_1MB_MASK         (0x1 << RESBAR_CAP_BAR_SIZE_1MB_POSITION             )
#define  RESBAR_CAP_BAR_SIZE_2MB_POSITION     5
#define  RESBAR_CAP_BAR_SIZE_2MB_MASK         (0x1 << RESBAR_CAP_BAR_SIZE_2MB_POSITION             )
#define  RESBAR_CAP_BAR_SIZE_4MB_POSITION     6
#define  RESBAR_CAP_BAR_SIZE_4MB_MASK         (0x1 << RESBAR_CAP_BAR_SIZE_4MB_POSITION             )
#define  RESBAR_CAP_BAR_SIZE_8MB_POSITION     7
#define  RESBAR_CAP_BAR_SIZE_8MB_MASK         (0x1 << RESBAR_CAP_BAR_SIZE_8MB_POSITION             )
#define  RESBAR_CAP_BAR_SIZE_16MB_POSITION    8
#define  RESBAR_CAP_BAR_SIZE_16MB_MASK        (0x1 << RESBAR_CAP_BAR_SIZE_16MB_POSITION            )
#define  RESBAR_CAP_BAR_SIZE_32MB_POSITION    9
#define  RESBAR_CAP_BAR_SIZE_32MB_MASK        (0x1 << RESBAR_CAP_BAR_SIZE_32MB_POSITION            )
#define  RESBAR_CAP_BAR_SIZE_64MB_POSITION    10
#define  RESBAR_CAP_BAR_SIZE_64MB_MASK        (0x1 << RESBAR_CAP_BAR_SIZE_64MB_POSITION            )
#define  RESBAR_CAP_BAR_SIZE_128MB_POSITION   11
#define  RESBAR_CAP_BAR_SIZE_128MB_MASK       (0x1 << RESBAR_CAP_BAR_SIZE_128MB_POSITION           )
#define  RESBAR_CAP_BAR_SIZE_256MB_POSITION   12
#define  RESBAR_CAP_BAR_SIZE_256MB_MASK       (0x1 << RESBAR_CAP_BAR_SIZE_256MB_POSITION           )
#define  RESBAR_CAP_BAR_SIZE_512MB_POSITION   13
#define  RESBAR_CAP_BAR_SIZE_512MB_MASK       (0x1 << RESBAR_CAP_BAR_SIZE_512MB_POSITION           )
#define  RESBAR_CAP_BAR_SIZE_1GB_POSITION     14
#define  RESBAR_CAP_BAR_SIZE_1GB_MASK         (0x1 << RESBAR_CAP_BAR_SIZE_1GB_POSITION             )
#define  RESBAR_CAP_BAR_SIZE_2GB_POSITION     15
#define  RESBAR_CAP_BAR_SIZE_2GB_MASK         (0x1 << RESBAR_CAP_BAR_SIZE_2GB_POSITION             )
#define  RESBAR_CAP_BAR_SIZE_4GB_POSITION     16
#define  RESBAR_CAP_BAR_SIZE_4GB_MASK         (0x1 << RESBAR_CAP_BAR_SIZE_4GB_POSITION             )
#define  RESBAR_CAP_BAR_SIZE_8GB_POSITION     17
#define  RESBAR_CAP_BAR_SIZE_8GB_MASK         (0x1 << RESBAR_CAP_BAR_SIZE_8GB_POSITION             )
#define  RESBAR_CAP_BAR_SIZE_16GB_POSITION    18
#define  RESBAR_CAP_BAR_SIZE_16GB_MASK        (0x1 << RESBAR_CAP_BAR_SIZE_16GB_POSITION            )
#define  RESBAR_CAP_BAR_SIZE_32GB_POSITION    19
#define  RESBAR_CAP_BAR_SIZE_32GB_MASK        (0x1 << RESBAR_CAP_BAR_SIZE_32GB_POSITION            )
#define  RESBAR_CAP_BAR_SIZE_64GB_POSITION    20
#define  RESBAR_CAP_BAR_SIZE_64GB_MASK        (0x1 << RESBAR_CAP_BAR_SIZE_64GB_POSITION            )
#define  RESBAR_CAP_BAR_SIZE_128GB_POSITION   21
#define  RESBAR_CAP_BAR_SIZE_128GB_MASK       (0x1 << RESBAR_CAP_BAR_SIZE_128GB_POSITION           )
#define  RESBAR_CAP_BAR_SIZE_256GB_POSITION   22
#define  RESBAR_CAP_BAR_SIZE_256GB_MASK       (0x1 << RESBAR_CAP_BAR_SIZE_256GB_POSITION           )
#define  RESBAR_CAP_BAR_SIZE_512GB_POSITION   23
#define  RESBAR_CAP_BAR_SIZE_512GB_MASK       (0x1 << RESBAR_CAP_BAR_SIZE_512GB_POSITION           )
#define  RESBAR_CAP_BAR_SIZE_1TB_POSITION     24
#define  RESBAR_CAP_BAR_SIZE_1TB_MASK         (0x1 << RESBAR_CAP_BAR_SIZE_1TB_POSITION             )
#define  RESBAR_CAP_BAR_SIZE_2TB_POSITION     25
#define  RESBAR_CAP_BAR_SIZE_2TB_MASK         (0x1 << RESBAR_CAP_BAR_SIZE_2TB_POSITION             )
#define  RESBAR_CAP_BAR_SIZE_4TB_POSITION     26
#define  RESBAR_CAP_BAR_SIZE_4TB_MASK         (0x1 << RESBAR_CAP_BAR_SIZE_4TB_POSITION             )
#define  RESBAR_CAP_BAR_SIZE_8TB_POSITION     27
#define  RESBAR_CAP_BAR_SIZE_8TB_MASK         (0x1 << RESBAR_CAP_BAR_SIZE_8TB_POSITION             )
#define  RESBAR_CAP_BAR_SIZE_16TB_POSITION    28
#define  RESBAR_CAP_BAR_SIZE_16TB_MASK        (0x1 << RESBAR_CAP_BAR_SIZE_16TB_POSITION            )
#define  RESBAR_CAP_BAR_SIZE_32TB_POSITION    29
#define  RESBAR_CAP_BAR_SIZE_32TB_MASK        (0x1 << RESBAR_CAP_BAR_SIZE_32TB_POSITION            )
#define  RESBAR_CAP_BAR_SIZE_64TB_POSITION    30
#define  RESBAR_CAP_BAR_SIZE_64TB_MASK        (0x1 << RESBAR_CAP_BAR_SIZE_64TB_POSITION            )
#define  RESBAR_CAP_BAR_SIZE_128TB_POSITION   31
#define  RESBAR_CAP_BAR_SIZE_128TB_MASK       (0x1 << RESBAR_CAP_BAR_SIZE_128TB_POSITION           )

#define  RESBAR_CTRL_REG_IDX_POSITION         0
#define  RESBAR_CTRL_REG_IDX_MASK             (0x7 << RESBAR_CTRL_REG_IDX_POSITION                 )
#define  RESBAR_CTRL_REG_NUM_BARS_POSITION    5
#define  RESBAR_CTRL_REG_NUM_BARS_MASK        (0x7 << RESBAR_CTRL_REG_NUM_BARS_POSITION            )
#define  RESBAR_CTRL_REG_BAR_SIZE_POSITION    8
#define  RESBAR_CTRL_REG_BAR_SIZE_MASK        (0x3F << RESBAR_CTRL_REG_BAR_SIZE_POSITION           )

#define  RESBAR_CTRL_BAR_SIZE_256TB_POSITION  16
#define  RESBAR_CTRL_BAR_SIZE_256TB_MASK      (0x1 << RESBAR_CTRL_BAR_SIZE_256TB_POSITION          )
#define  RESBAR_CTRL_BAR_SIZE_512TB_POSITION  17
#define  RESBAR_CTRL_BAR_SIZE_512TB_MASK      (0x1 << RESBAR_CTRL_BAR_SIZE_512TB_POSITION          )
#define  RESBAR_CTRL_BAR_SIZE_1PB_POSITION    18
#define  RESBAR_CTRL_BAR_SIZE_1PB_MASK        (0x1 << RESBAR_CTRL_BAR_SIZE_1PB_POSITION            )
#define  RESBAR_CTRL_BAR_SIZE_2PB_POSITION    19
#define  RESBAR_CTRL_BAR_SIZE_2PB_MASK        (0x1 << RESBAR_CTRL_BAR_SIZE_2PB_POSITION            )
#define  RESBAR_CTRL_BAR_SIZE_4PB_POSITION    20
#define  RESBAR_CTRL_BAR_SIZE_4PB_MASK        (0x1 << RESBAR_CTRL_BAR_SIZE_4PB_POSITION            )
#define  RESBAR_CTRL_BAR_SIZE_8PB_POSITION    21
#define  RESBAR_CTRL_BAR_SIZE_8PB_MASK        (0x1 << RESBAR_CTRL_BAR_SIZE_8PB_POSITION            )
#define  RESBAR_CTRL_BAR_SIZE_16PB_POSITION   22
#define  RESBAR_CTRL_BAR_SIZE_16PB_MASK       (0x1 << RESBAR_CTRL_BAR_SIZE_16PB_POSITION           )
#define  RESBAR_CTRL_BAR_SIZE_32PB_POSITION   23
#define  RESBAR_CTRL_BAR_SIZE_32PB_MASK       (0x1 << RESBAR_CTRL_BAR_SIZE_32PB_POSITION           )
#define  RESBAR_CTRL_BAR_SIZE_64PB_POSITION   24
#define  RESBAR_CTRL_BAR_SIZE_64PB_MASK       (0x1 << RESBAR_CTRL_BAR_SIZE_64PB_POSITION           )
#define  RESBAR_CTRL_BAR_SIZE_128PB_POSITION  25
#define  RESBAR_CTRL_BAR_SIZE_128PB_MASK      (0x1 << RESBAR_CTRL_BAR_SIZE_128PB_POSITION          )
#define  RESBAR_CTRL_BAR_SIZE_256PB_POSITION  26
#define  RESBAR_CTRL_BAR_SIZE_256PB_MASK      (0x1 << RESBAR_CTRL_BAR_SIZE_256PB_POSITION          )
#define  RESBAR_CTRL_BAR_SIZE_512PB_POSITION  27
#define  RESBAR_CTRL_BAR_SIZE_512PB_MASK      (0x1 << RESBAR_CTRL_BAR_SIZE_512PB_POSITION          )
#define  RESBAR_CTRL_BAR_SIZE_1EB_POSITION    28
#define  RESBAR_CTRL_BAR_SIZE_1EB_MASK        (0x1 << RESBAR_CTRL_BAR_SIZE_1EB_POSITION            )
#define  RESBAR_CTRL_BAR_SIZE_2EB_POSITION    29
#define  RESBAR_CTRL_BAR_SIZE_2EB_MASK        (0x1 << RESBAR_CTRL_BAR_SIZE_2EB_POSITION            )
#define  RESBAR_CTRL_BAR_SIZE_4EB_POSITION    30
#define  RESBAR_CTRL_BAR_SIZE_4EB_MASK        (0x1 << RESBAR_CTRL_BAR_SIZE_4EB_POSITION            )
#define  RESBAR_CTRL_BAR_SIZE_8EB_POSITION    31
#define  RESBAR_CTRL_BAR_SIZE_8EB_MASK        (0x1 << RESBAR_CTRL_BAR_SIZE_8EB_POSITION            )

/////////////////////////////////////////////
// VF Resizable BAR Capability
/////////////////////////////////////////////
#define  VF_RESBAR_CAP_ID_POSITION               0
#define  VF_RESBAR_CAP_ID_MASK                   (0xffff << VF_RESBAR_CAP_ID_POSITION                   )
#define  VF_RESBAR_CAP_VERSION_POSITION          16
#define  VF_RESBAR_CAP_VERSION_MASK              (0xf << VF_RESBAR_CAP_VERSION_POSITION                  )
#define  VF_RESBAR_CAP_NEXT_OFFSET_POSITION      20
#define  VF_RESBAR_CAP_NEXT_OFFSET_MASK          (0xfff << VF_RESBAR_CAP_NEXT_OFFSET_POSITION           )

#define  VF_RESBAR_CAP_BAR_SIZE_1MB_POSITION     4
#define  VF_RESBAR_CAP_BAR_SIZE_1MB_MASK         (0x1 << VF_RESBAR_CAP_BAR_SIZE_1MB_POSITION             )
#define  VF_RESBAR_CAP_BAR_SIZE_2MB_POSITION     5
#define  VF_RESBAR_CAP_BAR_SIZE_2MB_MASK         (0x1 << VF_RESBAR_CAP_BAR_SIZE_2MB_POSITION             )
#define  VF_RESBAR_CAP_BAR_SIZE_4MB_POSITION     6
#define  VF_RESBAR_CAP_BAR_SIZE_4MB_MASK         (0x1 << VF_RESBAR_CAP_BAR_SIZE_4MB_POSITION             )
#define  VF_RESBAR_CAP_BAR_SIZE_8MB_POSITION     7
#define  VF_RESBAR_CAP_BAR_SIZE_8MB_MASK         (0x1 << VF_RESBAR_CAP_BAR_SIZE_8MB_POSITION             )
#define  VF_RESBAR_CAP_BAR_SIZE_16MB_POSITION    8
#define  VF_RESBAR_CAP_BAR_SIZE_16MB_MASK        (0x1 << VF_RESBAR_CAP_BAR_SIZE_16MB_POSITION            )
#define  VF_RESBAR_CAP_BAR_SIZE_32MB_POSITION    9
#define  VF_RESBAR_CAP_BAR_SIZE_32MB_MASK        (0x1 << VF_RESBAR_CAP_BAR_SIZE_32MB_POSITION            )
#define  VF_RESBAR_CAP_BAR_SIZE_64MB_POSITION    10
#define  VF_RESBAR_CAP_BAR_SIZE_64MB_MASK        (0x1 << VF_RESBAR_CAP_BAR_SIZE_64MB_POSITION            )
#define  VF_RESBAR_CAP_BAR_SIZE_128MB_POSITION   11
#define  VF_RESBAR_CAP_BAR_SIZE_128MB_MASK       (0x1 << VF_RESBAR_CAP_BAR_SIZE_128MB_POSITION           )
#define  VF_RESBAR_CAP_BAR_SIZE_256MB_POSITION   12
#define  VF_RESBAR_CAP_BAR_SIZE_256MB_MASK       (0x1 << VF_RESBAR_CAP_BAR_SIZE_256MB_POSITION           )
#define  VF_RESBAR_CAP_BAR_SIZE_512MB_POSITION   13
#define  VF_RESBAR_CAP_BAR_SIZE_512MB_MASK       (0x1 << VF_RESBAR_CAP_BAR_SIZE_512MB_POSITION           )
#define  VF_RESBAR_CAP_BAR_SIZE_1GB_POSITION     14
#define  VF_RESBAR_CAP_BAR_SIZE_1GB_MASK         (0x1 << VF_RESBAR_CAP_BAR_SIZE_1GB_POSITION             )
#define  VF_RESBAR_CAP_BAR_SIZE_2GB_POSITION     15
#define  VF_RESBAR_CAP_BAR_SIZE_2GB_MASK         (0x1 << VF_RESBAR_CAP_BAR_SIZE_2GB_POSITION             )
#define  VF_RESBAR_CAP_BAR_SIZE_4GB_POSITION     16
#define  VF_RESBAR_CAP_BAR_SIZE_4GB_MASK         (0x1 << VF_RESBAR_CAP_BAR_SIZE_4GB_POSITION             )
#define  VF_RESBAR_CAP_BAR_SIZE_8GB_POSITION     17
#define  VF_RESBAR_CAP_BAR_SIZE_8GB_MASK         (0x1 << VF_RESBAR_CAP_BAR_SIZE_8GB_POSITION             )
#define  VF_RESBAR_CAP_BAR_SIZE_16GB_POSITION    18
#define  VF_RESBAR_CAP_BAR_SIZE_16GB_MASK        (0x1 << VF_RESBAR_CAP_BAR_SIZE_16GB_POSITION            )
#define  VF_RESBAR_CAP_BAR_SIZE_32GB_POSITION    19
#define  VF_RESBAR_CAP_BAR_SIZE_32GB_MASK        (0x1 << VF_RESBAR_CAP_BAR_SIZE_32GB_POSITION            )
#define  VF_RESBAR_CAP_BAR_SIZE_64GB_POSITION    20
#define  VF_RESBAR_CAP_BAR_SIZE_64GB_MASK        (0x1 << VF_RESBAR_CAP_BAR_SIZE_64GB_POSITION            )
#define  VF_RESBAR_CAP_BAR_SIZE_128GB_POSITION   21
#define  VF_RESBAR_CAP_BAR_SIZE_128GB_MASK       (0x1 << VF_RESBAR_CAP_BAR_SIZE_128GB_POSITION           )
#define  VF_RESBAR_CAP_BAR_SIZE_256GB_POSITION   22
#define  VF_RESBAR_CAP_BAR_SIZE_256GB_MASK       (0x1 << VF_RESBAR_CAP_BAR_SIZE_256GB_POSITION           )
#define  VF_RESBAR_CAP_BAR_SIZE_512GB_POSITION   23
#define  VF_RESBAR_CAP_BAR_SIZE_512GB_MASK       (0x1 << VF_RESBAR_CAP_BAR_SIZE_512GB_POSITION           )
#define  VF_RESBAR_CAP_BAR_SIZE_1TB_POSITION     24
#define  VF_RESBAR_CAP_BAR_SIZE_1TB_MASK         (0x1 << VF_RESBAR_CAP_BAR_SIZE_1TB_POSITION             )
#define  VF_RESBAR_CAP_BAR_SIZE_2TB_POSITION     25
#define  VF_RESBAR_CAP_BAR_SIZE_2TB_MASK         (0x1 << VF_RESBAR_CAP_BAR_SIZE_2TB_POSITION             )
#define  VF_RESBAR_CAP_BAR_SIZE_4TB_POSITION     26
#define  VF_RESBAR_CAP_BAR_SIZE_4TB_MASK         (0x1 << VF_RESBAR_CAP_BAR_SIZE_4TB_POSITION             )
#define  VF_RESBAR_CAP_BAR_SIZE_8TB_POSITION     27
#define  VF_RESBAR_CAP_BAR_SIZE_8TB_MASK         (0x1 << VF_RESBAR_CAP_BAR_SIZE_8TB_POSITION             )
#define  VF_RESBAR_CAP_BAR_SIZE_16TB_POSITION    28
#define  VF_RESBAR_CAP_BAR_SIZE_16TB_MASK        (0x1 << VF_RESBAR_CAP_BAR_SIZE_16TB_POSITION            )
#define  VF_RESBAR_CAP_BAR_SIZE_32TB_POSITION    29
#define  VF_RESBAR_CAP_BAR_SIZE_32TB_MASK        (0x1 << VF_RESBAR_CAP_BAR_SIZE_32TB_POSITION            )
#define  VF_RESBAR_CAP_BAR_SIZE_64TB_POSITION    30
#define  VF_RESBAR_CAP_BAR_SIZE_64TB_MASK        (0x1 << VF_RESBAR_CAP_BAR_SIZE_64TB_POSITION            )
#define  VF_RESBAR_CAP_BAR_SIZE_128TB_POSITION   31
#define  VF_RESBAR_CAP_BAR_SIZE_128TB_MASK       (0x1 << VF_RESBAR_CAP_BAR_SIZE_128TB_POSITION           )

#define  VF_RESBAR_CTRL_REG_IDX_POSITION         0
#define  VF_RESBAR_CTRL_REG_IDX_MASK             (0x7 << VF_RESBAR_CTRL_REG_IDX_POSITION                 )
#define  VF_RESBAR_CTRL_REG_NUM_BARS_POSITION    5
#define  VF_RESBAR_CTRL_REG_NUM_BARS_MASK        (0x7 << VF_RESBAR_CTRL_REG_NUM_BARS_POSITION            )
#define  VF_RESBAR_CTRL_REG_BAR_SIZE_POSITION    8
#define  VF_RESBAR_CTRL_REG_BAR_SIZE_MASK        (0x3F << VF_RESBAR_CTRL_REG_BAR_SIZE_POSITION           )

#define  VF_RESBAR_CTRL_BAR_SIZE_256TB_POSITION  16
#define  VF_RESBAR_CTRL_BAR_SIZE_256TB_MASK      (0x1 << VF_RESBAR_CTRL_BAR_SIZE_256TB_POSITION          )
#define  VF_RESBAR_CTRL_BAR_SIZE_512TB_POSITION  17
#define  VF_RESBAR_CTRL_BAR_SIZE_512TB_MASK      (0x1 << VF_RESBAR_CTRL_BAR_SIZE_512TB_POSITION          )
#define  VF_RESBAR_CTRL_BAR_SIZE_1PB_POSITION    18
#define  VF_RESBAR_CTRL_BAR_SIZE_1PB_MASK        (0x1 << VF_RESBAR_CTRL_BAR_SIZE_1PB_POSITION            )
#define  VF_RESBAR_CTRL_BAR_SIZE_2PB_POSITION    19
#define  VF_RESBAR_CTRL_BAR_SIZE_2PB_MASK        (0x1 << VF_RESBAR_CTRL_BAR_SIZE_2PB_POSITION            )
#define  VF_RESBAR_CTRL_BAR_SIZE_4PB_POSITION    20
#define  VF_RESBAR_CTRL_BAR_SIZE_4PB_MASK        (0x1 << VF_RESBAR_CTRL_BAR_SIZE_4PB_POSITION            )
#define  VF_RESBAR_CTRL_BAR_SIZE_8PB_POSITION    21
#define  VF_RESBAR_CTRL_BAR_SIZE_8PB_MASK        (0x1 << VF_RESBAR_CTRL_BAR_SIZE_8PB_POSITION            )
#define  VF_RESBAR_CTRL_BAR_SIZE_16PB_POSITION   22
#define  VF_RESBAR_CTRL_BAR_SIZE_16PB_MASK       (0x1 << VF_RESBAR_CTRL_BAR_SIZE_16PB_POSITION           )
#define  VF_RESBAR_CTRL_BAR_SIZE_32PB_POSITION   23
#define  VF_RESBAR_CTRL_BAR_SIZE_32PB_MASK       (0x1 << VF_RESBAR_CTRL_BAR_SIZE_32PB_POSITION           )
#define  VF_RESBAR_CTRL_BAR_SIZE_64PB_POSITION   24
#define  VF_RESBAR_CTRL_BAR_SIZE_64PB_MASK       (0x1 << VF_RESBAR_CTRL_BAR_SIZE_64PB_POSITION           )
#define  VF_RESBAR_CTRL_BAR_SIZE_128PB_POSITION  25
#define  VF_RESBAR_CTRL_BAR_SIZE_128PB_MASK      (0x1 << VF_RESBAR_CTRL_BAR_SIZE_128PB_POSITION          )
#define  VF_RESBAR_CTRL_BAR_SIZE_256PB_POSITION  26
#define  VF_RESBAR_CTRL_BAR_SIZE_256PB_MASK      (0x1 << VF_RESBAR_CTRL_BAR_SIZE_256PB_POSITION          )
#define  VF_RESBAR_CTRL_BAR_SIZE_512PB_POSITION  27
#define  VF_RESBAR_CTRL_BAR_SIZE_512PB_MASK      (0x1 << VF_RESBAR_CTRL_BAR_SIZE_512PB_POSITION          )
#define  VF_RESBAR_CTRL_BAR_SIZE_1EB_POSITION    28
#define  VF_RESBAR_CTRL_BAR_SIZE_1EB_MASK        (0x1 << VF_RESBAR_CTRL_BAR_SIZE_1EB_POSITION            )
#define  VF_RESBAR_CTRL_BAR_SIZE_2EB_POSITION    29
#define  VF_RESBAR_CTRL_BAR_SIZE_2EB_MASK        (0x1 << VF_RESBAR_CTRL_BAR_SIZE_2EB_POSITION            )
#define  VF_RESBAR_CTRL_BAR_SIZE_4EB_POSITION    30
#define  VF_RESBAR_CTRL_BAR_SIZE_4EB_MASK        (0x1 << VF_RESBAR_CTRL_BAR_SIZE_4EB_POSITION            )
#define  VF_RESBAR_CTRL_BAR_SIZE_8EB_POSITION    31
#define  VF_RESBAR_CTRL_BAR_SIZE_8EB_MASK        (0x1 << VF_RESBAR_CTRL_BAR_SIZE_8EB_POSITION            )

/////////////////////////////////////////////
// TPH Request Capability
////////////////////////////////////////////
#define  PCIE_EXT_CAP_ID_POSITION               0
#define  PCIE_EXT_CAP_ID_MASK                   (0xffff << PCIE_EXT_CAP_ID_POSITION                  )
#define  PCIE_EXT_CAP_VERSION_POSITION          16
#define  PCIE_EXT_CAP_VERSION_MASK              (0xf << PCIE_EXT_CAP_VERSION_POSITION                 )
#define  PCIE_EXT_CAP_NEXT_OFFSET_POSITION      20
#define  PCIE_EXT_CAP_NEXT_OFFSET_MASK          (0xfff << PCIE_EXT_CAP_NEXT_OFFSET_POSITION          )

#define  TPH_REQ_CAP_NO_ST_POSITION             0
#define  TPH_REQ_CAP_NO_ST_MASK                 (0x1 << TPH_REQ_CAP_NO_ST_POSITION                    )
#define  TPH_REQ_CAP_INT_VEC_POSITION           1
#define  TPH_REQ_CAP_INT_VEC_MASK               (0x1 << TPH_REQ_CAP_INT_VEC_POSITION                  )
#define  TPH_REQ_CAP_DEV_SPEC_POSITION          2
#define  TPH_REQ_CAP_DEV_SPEC_MASK              (0x1 << TPH_REQ_CAP_DEV_SPEC_POSITION                 )
#define  TPH_REQ_CAP_EXT_TPH_REQ_POSITION       8
#define  TPH_REQ_CAP_EXT_TPH_REQ_MASK           (0x1 << TPH_REQ_CAP_EXT_TPH_REQ_POSITION              )
#define  TPH_REQ_CAP_ST_TABLE_LOC_POSITION_0    9
#define  TPH_REQ_CAP_ST_TABLE_LOC_MASK_0        (0x1 << TPH_REQ_CAP_ST_TABLE_LOC_POSITION_0           )
#define  TPH_REQ_CAP_ST_TABLE_LOC_POSITION_1    10
#define  TPH_REQ_CAP_ST_TABLE_LOC_MASK_1        (0x1 << TPH_REQ_CAP_ST_TABLE_LOC_POSITION_1           )
#define  TPH_REQ_CAP_ST_TABLE_SIZE_POSITION     16
#define  TPH_REQ_CAP_ST_TABLE_SIZE_MASK         (0x7ff << TPH_REQ_CAP_ST_TABLE_SIZE_POSITION         )

#define  TPH_REQ_CTRL_ST_MODE_POSITION          0
#define  TPH_REQ_CTRL_ST_MODE_MASK              (0x7 << TPH_REQ_CTRL_ST_MODE_POSITION                 )
#define  TPH_REQ_CTRL_REQ_EN_POSITION           8
#define  TPH_REQ_CTRL_REQ_EN_MASK               (0x3 << TPH_REQ_CTRL_REQ_EN_POSITION                )

#define  TPH_REQ_ST_LOWER_POSITION              (ST_TABLE_IDX*16+0)%32
#define  TPH_REQ_ST_LOWER_MASK                  (0xff << TPH_REQ_ST_LOWER_POSITION                   )
#define  TPH_REQ_ST_UPPER_POSITION              (ST_TABLE_IDX*16+8)%32
#define  TPH_REQ_ST_UPPER_MASK                  (0xff << TPH_REQ_ST_UPPER_POSITION                    )

/////////////////////////////////////////////
// AER Capability
////////////////////////////////////////////
#define  AER_CAP_ID_POSITION 0
#define  AER_CAP_ID_MASK (0xffff << AER_CAP_ID_POSITION)

#define  AER_CAP_VERSION_POSITION 16
#define  AER_CAP_VERSION_MASK (0xf << AER_CAP_VERSION_POSITION)

#define  AER_NEXT_OFFSET_POSITION 20
#define  AER_NEXT_OFFSET_MASK (0xfff << AER_NEXT_OFFSET_POSITION)

#define  AER_DL_PROTOCOL_ERR_STATUS_POSITION 4
#define  AER_DL_PROTOCOL_ERR_STATUS_MASK (1 << AER_DL_PROTOCOL_ERR_STATUS_POSITION)

#define  AER_SURPRISE_DOWN_ERR_STATUS_POSITION 5
#define  AER_SURPRISE_DOWN_ERR_STATUS_MASK (1 << AER_SURPRISE_DOWN_ERR_STATUS_POSITION)

#define  AER_POIS_TLP_ERR_STATUS_POSITION 12
#define  AER_POIS_TLP_ERR_STATUS_MASK (1 << AER_POIS_TLP_ERR_STATUS_POSITION)

#define  AER_FC_PROTOCOL_ERR_STATUS_POSITION 13
#define  AER_FC_PROTOCOL_ERR_STATUS_MASK (1 << AER_FC_PROTOCOL_ERR_STATUS_POSITION)

#define  AER_CMPLT_TIMEOUT_ERR_STATUS_POSITION 14
#define  AER_CMPLT_TIMEOUT_ERR_STATUS_MASK (1 << AER_CMPLT_TIMEOUT_ERR_STATUS_POSITION)

#define  AER_CMPLT_ABORT_ERR_STATUS_POSITION 15
#define  AER_CMPLT_ABORT_ERR_STATUS_MASK (1 << AER_CMPLT_ABORT_ERR_STATUS_POSITION)

#define  AER_UNEXP_CMPLT_ERR_STATUS_POSITION 16
#define  AER_UNEXP_CMPLT_ERR_STATUS_MASK (1 << AER_UNEXP_CMPLT_ERR_STATUS_POSITION)

#define  AER_REC_OVERFLOW_ERR_STATUS_POSITION 17
#define  AER_REC_OVERFLOW_ERR_STATUS_MASK (1 << AER_REC_OVERFLOW_ERR_STATUS_POSITION)

#define  AER_MALF_TLP_ERR_STATUS_POSITION 18
#define  AER_MALF_TLP_ERR_STATUS_MASK (1 << AER_MALF_TLP_ERR_STATUS_POSITION)

#define  AER_ECRC_ERR_STATUS_POSITION 19
#define  AER_ECRC_ERR_STATUS_MASK (1 << AER_ECRC_ERR_STATUS_POSITION)

#define  AER_INTERNAL_ERR_STATUS_POSITION 22
#define  AER_INTERNAL_ERR_STATUS_MASK (1 << AER_INTERNAL_ERR_STATUS_POSITION)

#define  AER_UNSUPPORTED_REQ_ERR_STATUS_POSITION 20
#define  AER_UNSUPPORTED_REQ_ERR_STATUS_MASK (1 << AER_UNSUPPORTED_REQ_ERR_STATUS_POSITION)

#define  AER_ACS_VIOLATION_STATUS_POSITION 21
#define  AER_ACS_VIOLATION_STATUS_MASK (1 << AER_ACS_VIOLATION_STATUS_POSITION)

#define  AER_ATOMIC_EGRESS_BLOCKED_ERR_STATUS_POSITION 24
#define  AER_ATOMIC_EGRESS_BLOCKED_ERR_STATUS_MASK (1 << AER_ATOMIC_EGRESS_BLOCKED_ERR_STATUS_POSITION)

#define  AER_TLP_PRFX_BLOCKED_ERR_STATUS_POSITION 25
#define  AER_TLP_PRFX_BLOCKED_ERR_STATUS_MASK (1 << AER_TLP_PRFX_BLOCKED_ERR_STATUS_POSITION)

#define  AER_DL_PROTOCOL_ERR_MASK_POSITION 4
#define  AER_DL_PROTOCOL_ERR_MASK_MASK (1 << AER_DL_PROTOCOL_ERR_MASK_POSITION)

#define  AER_SURPRISE_DOWN_ERR_MASK_POSITION 5
#define  AER_SURPRISE_DOWN_ERR_MASK_MASK (1 << AER_SURPRISE_DOWN_ERR_MASK_POSITION)

#define  AER_POIS_TLP_ERR_MASK_POSITION 12
#define  AER_POIS_TLP_ERR_MASK_MASK (1 << AER_POIS_TLP_ERR_MASK_POSITION)

#define  AER_FC_PROTOCOL_ERR_MASK_POSITION 13
#define  AER_FC_PROTOCOL_ERR_MASK_MASK (1 << AER_FC_PROTOCOL_ERR_MASK_POSITION)

#define  AER_CMPLT_TIMEOUT_ERR_MASK_POSITION 14
#define  AER_CMPLT_TIMEOUT_ERR_MASK_MASK (1 << AER_CMPLT_TIMEOUT_ERR_MASK_POSITION)

#define  AER_CMPLT_ABORT_ERR_MASK_POSITION 15
#define  AER_CMPLT_ABORT_ERR_MASK_MASK (1 << AER_CMPLT_ABORT_ERR_MASK_POSITION)

#define  AER_UNEXP_CMPLT_ERR_MASK_POSITION 16
#define  AER_UNEXP_CMPLT_ERR_MASK_MASK (1 << AER_UNEXP_CMPLT_ERR_MASK_POSITION)

#define  AER_REC_OVERFLOW_ERR_MASK_POSITION 17
#define  AER_REC_OVERFLOW_ERR_MASK_MASK (1 << AER_REC_OVERFLOW_ERR_MASK_POSITION)

#define  AER_MALF_TLP_ERR_MASK_POSITION 18
#define  AER_MALF_TLP_ERR_MASK_MASK (1 << AER_MALF_TLP_ERR_MASK_POSITION)

#define  AER_ECRC_ERR_MASK_POSITION 19
#define  AER_ECRC_ERR_MASK_MASK (1 << AER_ECRC_ERR_MASK_POSITION)

#define  AER_UNSUPPORTED_REQ_ERR_MASK_POSITION 20
#define  AER_UNSUPPORTED_REQ_ERR_MASK_MASK (1 << AER_UNSUPPORTED_REQ_ERR_MASK_POSITION)

#define  AER_ACS_VIOLATION_MASK_POSITION 21
#define  AER_ACS_VIOLATION_MASK_MASK (1 << AER_ACS_VIOLATION_MASK_POSITION)

#define  AER_INTERNAL_ERR_MASK_POSITION 22
#define  AER_INTERNAL_ERR_MASK_MASK (1 << AER_INTERNAL_ERR_MASK_POSITION)

#define  AER_ATOMIC_EGRESS_BLOCKED_ERR_MASK_POSITION 24
#define  AER_ATOMIC_EGRESS_BLOCKED_ERR_MASK_MASK (1 << AER_ATOMIC_EGRESS_BLOCKED_ERR_MASK_POSITION)

#define  AER_TLP_PRFX_BLOCKED_ERR_MASK_POSITION 25
#define  AER_TLP_PRFX_BLOCKED_ERR_MASK_MASK (1 << AER_TLP_PRFX_BLOCKED_ERR_MASK_POSITION)

#define  AER_DL_PROTOCOL_ERR_SEVERITY_POSITION 4
#define  AER_DL_PROTOCOL_ERR_SEVERITY_MASK (1 << AER_DL_PROTOCOL_ERR_SEVERITY_POSITION)

#define  AER_SURPRISE_DOWN_ERR_SVRITY_POSITION 5
#define  AER_SURPRISE_DOWN_ERR_SVRITY_MASK (1 << AER_SURPRISE_DOWN_ERR_SVRITY_POSITION)

#define  AER_POIS_TLP_ERR_SEVERITY_POSITION 12
#define  AER_POIS_TLP_ERR_SEVERITY_MASK (1 << AER_POIS_TLP_ERR_SEVERITY_POSITION)

#define  AER_FC_PROTOCOL_ERR_SEVERITY_POSITION 13
#define  AER_FC_PROTOCOL_ERR_SEVERITY_MASK (1 << AER_FC_PROTOCOL_ERR_SEVERITY_POSITION)

#define  AER_CMPLT_TIMEOUT_ERR_SEVERITY_POSITION 14
#define  AER_CMPLT_TIMEOUT_ERR_SEVERITY_MASK (1 << AER_CMPLT_TIMEOUT_ERR_SEVERITY_POSITION)

#define  AER_CMPLT_ABORT_ERR_SEVERITY_POSITION 15
#define  AER_CMPLT_ABORT_ERR_SEVERITY_MASK (1 << AER_CMPLT_ABORT_ERR_SEVERITY_POSITION)

#define  AER_UNEXP_CMPLT_ERR_SEVERITY_POSITION 16
#define  AER_UNEXP_CMPLT_ERR_SEVERITY_MASK (1 << AER_UNEXP_CMPLT_ERR_SEVERITY_POSITION)

#define  AER_REC_OVERFLOW_ERR_SEVERITY_POSITION 17
#define  AER_REC_OVERFLOW_ERR_SEVERITY_MASK (1 << AER_REC_OVERFLOW_ERR_SEVERITY_POSITION)

#define  AER_MALF_TLP_ERR_SEVERITY_POSITION 18
#define  AER_MALF_TLP_ERR_SEVERITY_MASK (1 << AER_MALF_TLP_ERR_SEVERITY_POSITION)

#define  AER_ECRC_ERR_SEVERITY_POSITION 19
#define  AER_ECRC_ERR_SEVERITY_MASK (1 << AER_ECRC_ERR_SEVERITY_POSITION)

#define  AER_UNSUPPORTED_REQ_ERR_SEVERITY_POSITION 20
#define  AER_UNSUPPORTED_REQ_ERR_SEVERITY_MASK (1 << AER_UNSUPPORTED_REQ_ERR_SEVERITY_POSITION)

#define  AER_ACS_VIOLATION_SEVERITY_POSITION 21
#define  AER_ACS_VIOLATION_SEVERITY_MASK (1 << AER_ACS_VIOLATION_SEVERITY_POSITION)

#define  AER_INTERNAL_ERR_SEVERITY_POSITION 22
#define  AER_INTERNAL_ERR_SEVERITY_MASK (1 << AER_INTERNAL_ERR_SEVERITY_POSITION)

#define  AER_ATOMIC_EGRESS_BLOCKED_ERR_SEVERITY_POSITION 24
#define  AER_ATOMIC_EGRESS_BLOCKED_ERR_SEVERITY_MASK (1 << AER_ATOMIC_EGRESS_BLOCKED_ERR_SEVERITY_POSITION)

#define  AER_TLP_PRFX_BLOCKED_ERR_SEVERITY_POSITION 25
#define  AER_TLP_PRFX_BLOCKED_ERR_SEVERITY_MASK (1 << AER_TLP_PRFX_BLOCKED_ERR_SEVERITY_POSITION)

#define  AER_RX_ERR_STATUS_POSITION 0
#define  AER_RX_ERR_STATUS_MASK (0x1 << AER_RX_ERR_STATUS_POSITION)

#define  AER_BAD_TLP_STATUS_POSITION 6
#define  AER_BAD_TLP_STATUS_MASK (0x1 << AER_BAD_TLP_STATUS_POSITION)

#define  AER_BAD_DLLP_STATUS_POSITION 7
#define  AER_BAD_DLLP_STATUS_MASK (0x1 << AER_BAD_DLLP_STATUS_POSITION)

#define  AER_REPLAY_NO_ROLEOVER_STATUS_POSITION 8
#define  AER_REPLAY_NO_ROLEOVER_STATUS_MASK (0x1 << AER_REPLAY_NO_ROLEOVER_STATUS_POSITION)

#define  AER_RPL_TIMER_TIMEOUT_STATUS_POSITION 12
#define  AER_RPL_TIMER_TIMEOUT_STATUS_MASK (0x1 << AER_RPL_TIMER_TIMEOUT_STATUS_POSITION)

#define  AER_ADVISORY_NON_FATAL_ERR_STATUS_POSITION 13
#define  AER_ADVISORY_NON_FATAL_ERR_STATUS_MASK (0x1 << AER_ADVISORY_NON_FATAL_ERR_STATUS_POSITION)

#define  AER_CORRECTED_INT_ERR_STATUS_POSITION 14
#define  AER_CORRECTED_INT_ERR_STATUS_MASK (0x1 << AER_CORRECTED_INT_ERR_STATUS_POSITION)

#define  AER_HEADER_LOG_OVERFLOW_STATUS_POSITION 15
#define  AER_HEADER_LOG_OVERFLOW_STATUS_MASK (0x1 << AER_HEADER_LOG_OVERFLOW_STATUS_POSITION)

#define  AER_RX_ERR_MASK_POSITION 0
#define  AER_RX_ERR_MASK_MASK (0x1 << AER_RX_ERR_MASK_POSITION)

#define  AER_BAD_TLP_MASK_POSITION 6
#define  AER_BAD_TLP_MASK_MASK (0x1 << AER_BAD_TLP_MASK_POSITION)

#define  AER_BAD_DLLP_MASK_POSITION 7
#define  AER_BAD_DLLP_MASK_MASK (0x1 << AER_BAD_DLLP_MASK_POSITION)

#define  AER_REPLAY_NO_ROLEOVER_MASK_POSITION 8
#define  AER_REPLAY_NO_ROLEOVER_MASK_MASK (0x1 << AER_REPLAY_NO_ROLEOVER_MASK_POSITION)

#define  AER_RPL_TIMER_TIMEOUT_MASK_POSITION 12
#define  AER_RPL_TIMER_TIMEOUT_MASK_MASK (0x1 << AER_RPL_TIMER_TIMEOUT_MASK_POSITION)

#define  AER_ADVISORY_NON_FATAL_ERR_MASK_POSITION 13
#define  AER_ADVISORY_NON_FATAL_ERR_MASK_MASK (0x1 << AER_ADVISORY_NON_FATAL_ERR_MASK_POSITION)


#define  AER_CORRECTED_INT_ERR_MASK_POSITION 14
#define  AER_CORRECTED_INT_ERR_MASK_MASK (0x1 << AER_CORRECTED_INT_ERR_MASK_POSITION)

#define  AER_HEADER_LOG_OVERFLOW_MASK_POSITION 15
#define  AER_HEADER_LOG_OVERFLOW_MASK_MASK (0x1 << AER_HEADER_LOG_OVERFLOW_MASK_POSITION)

#define  AER_FIRST_ERR_POINTER_POSITION 0
#define  AER_FIRST_ERR_POINTER_MASK (0x1f << AER_FIRST_ERR_POINTER_POSITION)

#define  AER_ECRC_GEN_CAP_POSITION 5
#define  AER_ECRC_GEN_CAP_MASK (0x1 << AER_ECRC_GEN_CAP_POSITION)

#define  AER_ECRC_GEN_EN_POSITION 6
#define  AER_ECRC_GEN_EN_MASK (0x1 << AER_ECRC_GEN_EN_POSITION)

#define  AER_ECRC_CHECK_CAP_POSITION 7
#define  AER_ECRC_CHECK_CAP_MASK (0x1 << AER_ECRC_CHECK_CAP_POSITION)

#define  AER_ECRC_CHECK_EN_POSITION 8
#define  AER_ECRC_CHECK_EN_MASK (0x1 << AER_ECRC_CHECK_EN_POSITION)

#define  AER_MULTIPLE_HEADER_CAP_POSITION 9
#define  AER_MULTIPLE_HEADER_CAP_MASK (0x1 << AER_MULTIPLE_HEADER_CAP_POSITION)

#define  AER_MULTIPLE_HEADER_EN_POSITION 10
#define  AER_MULTIPLE_HEADER_EN_MASK (0x1 << AER_MULTIPLE_HEADER_EN_POSITION)

#define  AER_TLP_PRFX_LOG_PRESENT_POSITION 11
#define  AER_TLP_PRFX_LOG_PRESENT_MASK (1 << AER_TLP_PRFX_LOG_PRESENT_POSITION)

//#define  AER_FIRST_DWORD_POSITION 0
//#define  AER_FIRST_DWORD_MASK (0xffff_ffff << AER_FIRST_DWORD_POSITION)

#define  AER_FIRST_DWORD_FIRST_BYTE_POSITION 0
#define  AER_FIRST_DWORD_FIRST_BYTE_MASK (0xff << AER_FIRST_DWORD_FIRST_BYTE_POSITION)

#define  AER_FIRST_DWORD_SECOND_BYTE_POSITION 8
#define  AER_FIRST_DWORD_SECOND_BYTE_MASK (0xff << AER_FIRST_DWORD_SECOND_BYTE_POSITION)

#define  AER_FIRST_DWORD_THIRD_BYTE_POSITION 16
#define  AER_FIRST_DWORD_THIRD_BYTE_MASK (0xff << AER_FIRST_DWORD_THIRD_BYTE_POSITION)

#define  AER_FIRST_DWORD_FOURTH_BYTE_POSITION 24
#define  AER_FIRST_DWORD_FOURTH_BYTE_MASK (0xff << AER_FIRST_DWORD_FOURTH_BYTE_POSITION)

//#define  AER_SECOND_DWORD_POSITION 0
//#define  AER_SECOND_DWORD_MASK (0xffff_ffff << AER_SECOND_DWORD_POSITION)

#define  AER_SECOND_DWORD_FIRST_BYTE_POSITION 0
#define  AER_SECOND_DWORD_FIRST_BYTE_MASK (0xff << AER_SECOND_DWORD_FIRST_BYTE_POSITION)

#define  AER_SECOND_DWORD_SECOND_BYTE_POSITION 8
#define  AER_SECOND_DWORD_SECOND_BYTE_MASK (0xff << AER_SECOND_DWORD_SECOND_BYTE_POSITION)

#define  AER_SECOND_DWORD_THIRD_BYTE_POSITION 16
#define  AER_SECOND_DWORD_THIRD_BYTE_MASK (0xff << AER_SECOND_DWORD_THIRD_BYTE_POSITION)

#define  AER_SECOND_DWORD_FOURTH_BYTE_POSITION 24
#define  AER_SECOND_DWORD_FOURTH_BYTE_MASK (0xff << AER_SECOND_DWORD_FOURTH_BYTE_POSITION)

//#define  AER_THIRD_DWORD_POSITION 0
//#define  AER_THIRD_DWORD_MASK (0xffff_ffff << AER_THIRD_DWORD_POSITION)

#define  AER_THIRD_DWORD_FIRST_BYTE_POSITION 0
#define  AER_THIRD_DWORD_FIRST_BYTE_MASK (0xff << AER_THIRD_DWORD_FIRST_BYTE_POSITION)

#define  AER_THIRD_DWORD_SECOND_BYTE_POSITION 8
#define  AER_THIRD_DWORD_SECOND_BYTE_MASK (0xff << AER_THIRD_DWORD_SECOND_BYTE_POSITION)

#define  AER_THIRD_DWORD_THIRD_BYTE_POSITION 16
#define  AER_THIRD_DWORD_THIRD_BYTE_MASK (0xff << AER_THIRD_DWORD_THIRD_BYTE_POSITION)

#define  AER_THIRD_DWORD_FOURTH_BYTE_POSITION 24
#define  AER_THIRD_DWORD_FOURTH_BYTE_MASK (0xff << AER_THIRD_DWORD_FOURTH_BYTE_POSITION)

//#define  AER_FOURTH_DWORD_POSITION 0
//#define  AER_FOURTH_DWORD_MASK (0xffff_ffff << AER_FOURTH_DWORD_POSITION)

#define  AER_FOURTH_DWORD_FIRST_BYTE_POSITION 0
#define  AER_FOURTH_DWORD_FIRST_BYTE_MASK (0xff << AER_FOURTH_DWORD_FIRST_BYTE_POSITION)

#define  AER_FOURTH_DWORD_SECOND_BYTE_POSITION 8
#define  AER_FOURTH_DWORD_SECOND_BYTE_MASK (0xff << AER_FOURTH_DWORD_SECOND_BYTE_POSITION)

#define  AER_FOURTH_DWORD_THIRD_BYTE_POSITION 16
#define  AER_FOURTH_DWORD_THIRD_BYTE_MASK (0xff << AER_FOURTH_DWORD_THIRD_BYTE_POSITION)

#define  AER_FOURTH_DWORD_FOURTH_BYTE_POSITION 24
#define  AER_FOURTH_DWORD_FOURTH_BYTE_MASK (0xff << AER_FOURTH_DWORD_FOURTH_BYTE_POSITION)

#define  AER_CORR_ERR_REPORTING_EN_POSITION 0
#define  AER_CORR_ERR_REPORTING_EN_MASK (0x1 << AER_CORR_ERR_REPORTING_EN_POSITION)

#define  AER_NON_FATAL_ERR_REPORTING_EN_POSITION 1
#define  AER_NON_FATAL_ERR_REPORTING_EN_MASK (0x1 << AER_NON_FATAL_ERR_REPORTING_EN_POSITION)

#define  AER_FATAL_ERR_REPORTING_EN_POSITION 2
#define  AER_FATAL_ERR_REPORTING_EN_MASK (0x1 << AER_FATAL_ERR_REPORTING_EN_POSITION)

#define  AER_ERR_COR_RX_POSITION 0
#define  AER_ERR_COR_RX_MASK (0x1 << AER_ERR_COR_RX_POSITION)

#define  AER_MUL_ERR_COR_RX_POSITION 1
#define  AER_MUL_ERR_COR_RX_MASK (0x1 << AER_MUL_ERR_COR_RX_POSITION)

#define  AER_ERR_FATAL_NON_FATAL_RX_POSITION 2
#define  AER_ERR_FATAL_NON_FATAL_RX_MASK (0x1 << AER_ERR_FATAL_NON_FATAL_RX_POSITION)

#define  AER_MUL_ERR_FATAL_NON_FATAL_RX_POSITION 3
#define  AER_MUL_ERR_FATAL_NON_FATAL_RX_MASK (0x1 << AER_MUL_ERR_FATAL_NON_FATAL_RX_POSITION)

#define  AER_FIRST_UNCORR_FATAL_POSITION 4
#define  AER_FIRST_UNCORR_FATAL_MASK (0x1 << AER_FIRST_UNCORR_FATAL_POSITION)

#define  AER_NON_FATAL_ERR_MSG_RX_POSITION 5
#define  AER_NON_FATAL_ERR_MSG_RX_MASK (0x1 << AER_NON_FATAL_ERR_MSG_RX_POSITION)

#define  AER_FATAL_ERR_MSG_RX_POSITION 6
#define  AER_FATAL_ERR_MSG_RX_MASK (0x1 << AER_FATAL_ERR_MSG_RX_POSITION)

#define  AER_ADV_ERR_INT_MSG_NUM_POSITION 27
#define  AER_ADV_ERR_INT_MSG_NUM_MASK (0x1f << AER_ADV_ERR_INT_MSG_NUM_POSITION)

#define  AER_ERR_COR_SOURCE_ID_POSITION 0
#define  AER_ERR_COR_SOURCE_ID_MASK (0xffff << AER_ERR_COR_SOURCE_ID_POSITION)

#define  AER_ERR_FATAL_NON_FATAL_SOURCE_ID_POSITION 16
#define  AER_ERR_FATAL_NON_FATAL_SOURCE_ID_MASK (0xffff << AER_ERR_FATAL_NON_FATAL_SOURCE_ID_POSITION)

#define  AER_CFG_TLP_PFX_LOG_1_FIRST_BYTE_POSITION 0
#define  AER_CFG_TLP_PFX_LOG_1_FIRST_BYTE_MASK (0xff << AER_CFG_TLP_PFX_LOG_1_FIRST_BYTE_POSITION)

#define  AER_CFG_TLP_PFX_LOG_1_SECOND_BYTE_POSITION 8
#define  AER_CFG_TLP_PFX_LOG_1_SECOND_BYTE_MASK (0xff << AER_CFG_TLP_PFX_LOG_1_SECOND_BYTE_POSITION)

#define  AER_CFG_TLP_PFX_LOG_1_THIRD_BYTE_POSITION 16
#define  AER_CFG_TLP_PFX_LOG_1_THIRD_BYTE_MASK (0xff << AER_CFG_TLP_PFX_LOG_1_THIRD_BYTE_POSITION)

#define  AER_CFG_TLP_PFX_LOG_1_FOURTH_BYTE_POSITION 24
#define  AER_CFG_TLP_PFX_LOG_1_FOURTH_BYTE_MASK (0xff << AER_CFG_TLP_PFX_LOG_1_FOURTH_BYTE_POSITION)

#define  AER_CFG_TLP_PFX_LOG_2_FIRST_BYTE_POSITION 0
#define  AER_CFG_TLP_PFX_LOG_2_FIRST_BYTE_MASK (0xff << AER_CFG_TLP_PFX_LOG_2_FIRST_BYTE_POSITION)

#define  AER_CFG_TLP_PFX_LOG_2_SECOND_BYTE_POSITION 8
#define  AER_CFG_TLP_PFX_LOG_2_SECOND_BYTE_MASK (0xff << AER_CFG_TLP_PFX_LOG_2_SECOND_BYTE_POSITION)

#define  AER_CFG_TLP_PFX_LOG_2_THIRD_BYTE_POSITION 16
#define  AER_CFG_TLP_PFX_LOG_2_THIRD_BYTE_MASK (0xff << AER_CFG_TLP_PFX_LOG_2_THIRD_BYTE_POSITION)

#define  AER_CFG_TLP_PFX_LOG_2_FOURTH_BYTE_POSITION 24
#define  AER_CFG_TLP_PFX_LOG_2_FOURTH_BYTE_MASK (0xff << AER_CFG_TLP_PFX_LOG_2_FOURTH_BYTE_POSITION)

#define  AER_CFG_TLP_PFX_LOG_3_FIRST_BYTE_POSITION 0
#define  AER_CFG_TLP_PFX_LOG_3_FIRST_BYTE_MASK (0xff << AER_CFG_TLP_PFX_LOG_3_FIRST_BYTE_POSITION)

#define  AER_CFG_TLP_PFX_LOG_3_SECOND_BYTE_POSITION 8
#define  AER_CFG_TLP_PFX_LOG_3_SECOND_BYTE_MASK (0xff << AER_CFG_TLP_PFX_LOG_3_SECOND_BYTE_POSITION)

#define  AER_CFG_TLP_PFX_LOG_3_THIRD_BYTE_POSITION 16
#define  AER_CFG_TLP_PFX_LOG_3_THIRD_BYTE_MASK (0xff << AER_CFG_TLP_PFX_LOG_3_THIRD_BYTE_POSITION)

#define  AER_CFG_TLP_PFX_LOG_3_FOURTH_BYTE_POSITION 24
#define  AER_CFG_TLP_PFX_LOG_3_FOURTH_BYTE_MASK (0xff << AER_CFG_TLP_PFX_LOG_3_FOURTH_BYTE_POSITION)

#define  AER_CFG_TLP_PFX_LOG_4_FIRST_BYTE_POSITION 0
#define  AER_CFG_TLP_PFX_LOG_4_FIRST_BYTE_MASK (0xff << AER_CFG_TLP_PFX_LOG_4_FIRST_BYTE_POSITION)

#define  AER_CFG_TLP_PFX_LOG_4_SECOND_BYTE_POSITION 8
#define  AER_CFG_TLP_PFX_LOG_4_SECOND_BYTE_MASK (0xff << AER_CFG_TLP_PFX_LOG_4_SECOND_BYTE_POSITION)

#define  AER_CFG_TLP_PFX_LOG_4_THIRD_BYTE_POSITION 16
#define  AER_CFG_TLP_PFX_LOG_4_THIRD_BYTE_MASK (0xff << AER_CFG_TLP_PFX_LOG_4_THIRD_BYTE_POSITION)

#define  AER_CFG_TLP_PFX_LOG_4_FOURTH_BYTE_POSITION 24
#define  AER_CFG_TLP_PFX_LOG_4_FOURTH_BYTE_MASK (0xff << AER_CFG_TLP_PFX_LOG_4_FOURTH_BYTE_POSITION)

/////////////////////////////////////////////
// ATS Capability
////////////////////////////////////////////
#define  ATS_CAP_ID_POSITION               0
#define  ATS_CAP_ID_MASK                   (0xffff << ATS_CAP_ID_POSITION)

#define  ATS_CAP_VERSION_POSITION          16
#define  ATS_CAP_VERSION_MASK              (0xf << ATS_CAP_VERSION_POSITION)

#define  ATS_NEXT_OFFSET_POSITION           20
#define  ATS_NEXT_OFFSET_MASK              (0xfff << ATS_NEXT_OFFSET_POSITION)

#define  ATS_INVALIDATE_Q_DEPTH_POSITION          0
#define  ATS_INVALIDATE_Q_DEPTH_MASK              (0x1f << ATS_INVALIDATE_Q_DEPTH_POSITION)

#define  ATS_PAGE_ALIGNED_REQ_POSITION          5
#define  ATS_PAGE_ALIGNED_REQ_MASK              (1 << ATS_PAGE_ALIGNED_REQ_POSITION)

#define ATS_GLOBAL_INVAL_SPPRTD_POSITION        6
#define ATS_GLOBAL_INVAL_SPPRTD_MASK            (1 << ATS_GLOBAL_INVAL_SPPRTD_POSITION)

#define ATS_STU_POSITION 16
#define ATS_STU_MASK (0x1f << ATS_STU_POSITION)

#define ATS_ENABLE_POSITION 31
#define ATS_ENABLE_MASK (0x1 << ATS_ENABLE_POSITION)

/////////////////////////////////////////////
// ACS Capability
/////////////////////////////////////////////
// Extended Capability Header
#define  ACS_CAP_ID_POSITION                    0
#define  ACS_CAP_ID_MASK                        (0xffff << ACS_CAP_ID_POSITION)

#define  ACS_CAP_VERSION_POSITION               16
#define  ACS_CAP_VERSION_MASK                   (0xf << ACS_CAP_VERSION_POSITION)

#define  ACS_NEXT_OFFSET_POSITION               20
#define  ACS_NEXT_OFFSET_MASK                   (0xfff << ACS_NEXT_OFFSET_POSITION)

// Capability Register
#define  ACS_SRC_VALID_POSITION                 0
#define  ACS_SRC_VALID_MASK                     (1 << ACS_SRC_VALID_POSITION)

#define  ACS_AT_BLOCK_POSITION                  1
#define  ACS_AT_BLOCK_MASK                      (1 << ACS_AT_BLOCK_POSITION)

#define  ACS_P2P_REQ_REDIRECT_POSITION          2
#define  ACS_P2P_REQ_REDIRECT_MASK              (1 << ACS_P2P_REQ_REDIRECT_POSITION)

#define  ACS_P2P_CPL_REDIRECT_POSITION          3
#define  ACS_P2P_CPL_REDIRECT_MASK              (1 << ACS_P2P_CPL_REDIRECT_POSITION)

#define  ACS_USP_FORWARDING_POSITION            4
#define  ACS_USP_FORWARDING_MASK                (1 << ACS_USP_FORWARDING_POSITION)

#define  ACS_P2P_EGRESS_CONTROL_POSITION        5
#define  ACS_P2P_EGRESS_CONTROL_MASK            (1 << ACS_P2P_EGRESS_CONTROL_POSITION)

#define  ACS_DIRECT_TRANSLATED_P2P_POSITION     6
#define  ACS_DIRECT_TRANSLATED_P2P_MASK         (1 << ACS_DIRECT_TRANSLATED_P2P_POSITION)

#define  ACS_EGRESS_CTRL_SIZE_POSITION          8
#define  ACS_EGRESS_CTRL_SIZE_MASK              (0xFF << ACS_EGRESS_CTRL_SIZE_POSITION)

// Control Register
#define  ACS_CTRL_REG_OFFSET                    16
#define  ACS_SRC_VALID_EN_POSITION              0 + ACS_CTRL_REG_OFFSET
#define  ACS_SRC_VALID_EN_MASK                  (1 << ACS_SRC_VALID_EN_POSITION)

#define  ACS_AT_BLOCK_EN_POSITION               1 + ACS_CTRL_REG_OFFSET
#define  ACS_AT_BLOCK_EN_MASK                   (1 << ACS_AT_BLOCK_EN_POSITION)

#define  ACS_P2P_REQ_REDIRECT_EN_POSITION       2 + ACS_CTRL_REG_OFFSET
#define  ACS_P2P_REQ_REDIRECT_EN_MASK           (1 << ACS_P2P_REQ_REDIRECT_EN_POSITION)

#define  ACS_P2P_CPL_REDIRECT_EN_POSITION       3 + ACS_CTRL_REG_OFFSET
#define  ACS_P2P_CPL_REDIRECT_EN_MASK           (1 << ACS_P2P_CPL_REDIRECT_EN_POSITION)

#define  ACS_USP_FORWARDING_EN_POSITION         4 + ACS_CTRL_REG_OFFSET
#define  ACS_USP_FORWARDING_EN_MASK             (1 << ACS_USP_FORWARDING_EN_POSITION)

#define  ACS_P2P_EGRESS_CONTROL_EN_POSITION     5 + ACS_CTRL_REG_OFFSET
#define  ACS_P2P_EGRESS_CONTROL_EN_MASK         (1 << ACS_P2P_EGRESS_CONTROL_EN_POSITION)

#define  ACS_DIRECT_TRANSLATED_P2P_EN_POSITION  6 + ACS_CTRL_REG_OFFSET
#define  ACS_DIRECT_TRANSLATED_P2P_EN_MASK      (1 << ACS_DIRECT_TRANSLATED_P2P_EN_POSITION)

// Egress Control Vector Registers
#define  ACS_EGRESS_CTRL_VECTOR_POSITION        0
#define  ACS_EGRESS_CTRL_VECTOR_MASK            (0xFFFF_FFFF << ACS_EGRESS_CTRL_VECTOR_POSITION)


/////////////////////////////////////////////
// PRS Capability
////////////////////////////////////////////
#define  PRS_CAP_ID_POSITION               0
#define  PRS_CAP_ID_MASK                   (0xffff << PRS_CAP_ID_POSITION)

#define  PRS_CAP_VERSION_POSITION          16
#define  PRS_CAP_VERSION_MASK              (0xf << PRS_CAP_VERSION_POSITION)

#define  PRS_NEXT_OFFSET_POSITION          20
#define  PRS_NEXT_OFFSET_MASK              (0xfff << PRS_NEXT_OFFSET_POSITION)

#define  PRS_ENABLE_POSITION                0
#define  PRS_ENABLE_MASK                    (0x1 << PRS_ENABLE_POSITION)

#define  PRS_RESET_POSITION                 1
#define  PRS_RESET_MASK                     (0x1 << PRS_RESET_POSITION)

#define  PRS_RESP_FAILURE_POSITION          16
#define  PRS_RESP_FAILURE_MASK              (0x1 << PRS_RESP_FAILURE_POSITION)

#define  PRS_UPRGI_POSITION                 17
#define  PRS_UPRGI_MASK                     (0x1 << PRS_UPRGI_POSITION)

#define  PRS_STOPPED_POSITION               24
#define  PRS_STOPPED_MASK                   (0x1 << PRS_STOPPED_POSITION)

#define  PRS_OUTSTANDING_CAPACITY_POSITION  0
#define  PRS_OUTSTANDING_CAPACITY_MASK      (0xffff_ffff << PRS_OUTSTANDING_CAPACITY_POSITION)

#define  PRS_OUTSTANDING_ALLOCATION_POSITION  0
#define  PRS_OUTSTANDING_ALLOCATION_MASK      (0xffff_ffff << PRS_OUTSTANDING_ALLOCATION_POSITION)

/////////////////////////////////////////////
//PASID Capability
////////////////////////////////////////////
#define PASID_EXT_CAP_ID_POSITION  0
#define PASID_EXT_CAP_ID_MASK (0xffff << PASID_EXT_CAP_ID_POSITION)

#define  PASID_CAP_VERSION_POSITION 16
#define  PASID_CAP_VERSION_MASK (0xf << PASID_CAP_VERSION_POSITION)

#define  PASID_NEXT_OFFSET_POSITION 20
#define  PASID_NEXT_OFFSET_MASK (0xfff << PASID_NEXT_OFFSET_POSITION)

#define  PASID_EXECUTE_PERMISSION_SUPPORTED_POSITION 1
#define  PASID_EXECUTE_PERMISSION_SUPPORTED_MASK (1 << PASID_EXECUTE_PERMISSION_SUPPORTED_POSITION)

#define PASID_PRIVILEGED_MODE_SUPPORTED_POSITION 2
#define PASID_PRIVILEGED_MODE_SUPPORTED_MASK (1 << PASID_PRIVILEGED_MODE_SUPPORTED_POSITION)

#define PASID_MAX_PASID_WIDTH_POSITION  8
#define PASID_MAX_PASID_WIDTH_MASK (0x1f << PASID_MAX_PASID_WIDTH_POSITION)

#define PASID_EN_POSITION 16
#define PASID_EN_MASK (1 <<PASID_EN_POSITION)

#define PASID_EXECUTE_PERMISSION_EN_POSITION 17
#define PASID_EXECUTE_PERMISSION_EN_MASK (1 << PASID_EXECUTE_PERMISSION_EN_POSITION)

#define PASID_PRIVILEGED_MODE_EN_POSITION 18
#define PASID_PRIVILEGED_MODE_EN_MASK (1 << PASID_PRIVILEGED_MODE_EN_POSITION)

/////////////////////////////////////////////
//DPA Capability
////////////////////////////////////////////
#define DPA_EXT_CAP_ID_POSITION  0
#define DPA_EXT_CAP_ID_MASK (0xffff << DPA_EXT_CAP_ID_POSITION)

#define  DPA_CAP_VERSION_POSITION 16
#define  DPA_CAP_VERSION_MASK (0xf << DPA_CAP_VERSION_POSITION)

#define  DPA_NEXT_OFFSET_POSITION 20
#define  DPA_NEXT_OFFSET_MASK (0xfff << DPA_NEXT_OFFSET_POSITION)

#define  DPA_SUBSTATE_MAX_POSITION 0
#define  DPA_SUBSTATE_MAX_MASK (0x1f << DPA_SUBSTATE_MAX_POSITION)

#define  DPA_TLUNIT_POSITION 8
#define  DPA_TLUNIT_MASK (0x3 << DPA_TLUNIT_POSITION)

#define  DPA_PAS_POSITION 12
#define  DPA_PAS_MASK (0x3 << DPA_PAS_POSITION)

#define  DPA_XLCY0_POSITION 16
#define  DPA_XLCY0_MASK (0xFF << DPA_XLCY0_POSITION)

#define  DPA_XLCY1_POSITION 24
#define  DPA_XLCY1_MASK (0xFF << DPA_XLCY1_POSITION)

#define DPA_X1_INDICATOR1_POSITION 0
#define DPA_X1_INDICATOR1_MASK ({(DEFAULT_DPA_SUBSTATE_MAX + 1){0x1}} << `DPA_X1_INDICATOR1_POSITION)

#define DPA_SUBSTATE_STATUS_POSITION 0
#define DPA_SUBSTATE_STATUS_MASK ( 0x1f<< DPA_SUBSTATE_STATUS_POSITION)

#define DPA_SUBSTATE_CONTROL_EN_POSITION 8
#define DPA_SUBSTATE_CONTROL_EN_MASK ( 0x1 << DPA_SUBSTATE_CONTROL_EN_POSITION)

#define DPA_SUBSTATE_CONTROL_POSITION 16
#define DPA_SUBSTATE_CONTROL_MASK ( 0x1f<< DPA_SUBSTATE_CONTROL_POSITION)

#define DPA_PWR_ALLOC_VAL0_POSITION 0
#define DPA_PWR_ALLOC_VAL0_MASK ( 0xff << DPA_PWR_ALLOC_VAL0_POSITION)

#define DPA_PWR_ALLOC_VAL1_POSITION 8
#define DPA_PWR_ALLOC_VAL1_MASK ( 0xff << DPA_PWR_ALLOC_VAL1_POSITION)

#define DPA_PWR_ALLOC_VAL2_POSITION 16
#define DPA_PWR_ALLOC_VAL2_MASK ( 0xff << DPA_PWR_ALLOC_VAL2_POSITION)

#define DPA_PWR_ALLOC_VAL3_POSITION 24
#define DPA_PWR_ALLOC_VAL3_MASK ( 0xff << DPA_PWR_ALLOC_VAL3_POSITION)


#define DPA_PWR_ALLOC_VAL4_POSITION 0
#define DPA_PWR_ALLOC_VAL4_MASK ( 0xff << DPA_PWR_ALLOC_VAL4_POSITION)

#define DPA_PWR_ALLOC_VAL5_POSITION 8
#define DPA_PWR_ALLOC_VAL5_MASK ( 0xff << DPA_PWR_ALLOC_VAL5_POSITION)

#define DPA_PWR_ALLOC_VAL6_POSITION 16
#define DPA_PWR_ALLOC_VAL6_MASK ( 0xff << DPA_PWR_ALLOC_VAL6_POSITION)

#define DPA_PWR_ALLOC_VAL7_POSITION 24
#define DPA_PWR_ALLOC_VAL7_MASK ( 0xff << DPA_PWR_ALLOC_VAL7_POSITION)

#define DPA_PWR_ALLOC_VAL8_POSITION 0
#define DPA_PWR_ALLOC_VAL8_MASK ( 0xff << DPA_PWR_ALLOC_VAL8_POSITION)

#define DPA_PWR_ALLOC_VAL9_POSITION 8
#define DPA_PWR_ALLOC_VAL9_MASK ( 0xff << DPA_PWR_ALLOC_VAL9_POSITION)

#define DPA_PWR_ALLOC_VAL10_POSITION 16
#define DPA_PWR_ALLOC_VAL10_MASK ( 0xff << DPA_PWR_ALLOC_VAL10_POSITION)

#define DPA_PWR_ALLOC_VAL11_POSITION 24
#define DPA_PWR_ALLOC_VAL11_MASK ( 0xff << DPA_PWR_ALLOC_VAL11_POSITION)

#define DPA_PWR_ALLOC_VAL12_POSITION 0
#define DPA_PWR_ALLOC_VAL12_MASK ( 0xff << DPA_PWR_ALLOC_VAL12_POSITION)

#define DPA_PWR_ALLOC_VAL13_POSITION 8
#define DPA_PWR_ALLOC_VAL13_MASK ( 0xff << DPA_PWR_ALLOC_VAL13_POSITION)

#define DPA_PWR_ALLOC_VAL14_POSITION 16
#define DPA_PWR_ALLOC_VAL14_MASK ( 0xff << DPA_PWR_ALLOC_VAL14_POSITION)

#define DPA_PWR_ALLOC_VAL15_POSITION 24
#define DPA_PWR_ALLOC_VAL15_MASK ( 0xff << DPA_PWR_ALLOC_VAL15_POSITION)


#define DPA_PWR_ALLOC_VAL16_POSITION 0
#define DPA_PWR_ALLOC_VAL16_MASK ( 0xff << DPA_PWR_ALLOC_VAL16_POSITION)

#define DPA_PWR_ALLOC_VAL17_POSITION 8
#define DPA_PWR_ALLOC_VAL17_MASK ( 0xff << DPA_PWR_ALLOC_VAL17_POSITION)

#define DPA_PWR_ALLOC_VAL18_POSITION 16
#define DPA_PWR_ALLOC_VAL18_MASK ( 0xff << DPA_PWR_ALLOC_VAL18_POSITION)

#define DPA_PWR_ALLOC_VAL19_POSITION 24
#define DPA_PWR_ALLOC_VAL19_MASK ( 0xff << DPA_PWR_ALLOC_VAL19_POSITION)

#define DPA_PWR_ALLOC_VAL20_POSITION 0
#define DPA_PWR_ALLOC_VAL20_MASK ( 0xff << DPA_PWR_ALLOC_VAL20_POSITION)

#define DPA_PWR_ALLOC_VAL21_POSITION 8
#define DPA_PWR_ALLOC_VAL21_MASK ( 0xff << DPA_PWR_ALLOC_VAL21_POSITION)

#define DPA_PWR_ALLOC_VAL22_POSITION 16
#define DPA_PWR_ALLOC_VAL22_MASK ( 0xff << DPA_PWR_ALLOC_VAL22_POSITION)

#define DPA_PWR_ALLOC_VAL23_POSITION 24
#define DPA_PWR_ALLOC_VAL23_MASK ( 0xff << DPA_PWR_ALLOC_VAL23_POSITION)

#define DPA_PWR_ALLOC_VAL24_POSITION 0
#define DPA_PWR_ALLOC_VAL24_MASK ( 0xff << DPA_PWR_ALLOC_VAL24_POSITION)

#define DPA_PWR_ALLOC_VAL25_POSITION 8
#define DPA_PWR_ALLOC_VAL25_MASK ( 0xff << DPA_PWR_ALLOC_VAL25_POSITION)

#define DPA_PWR_ALLOC_VAL26_POSITION 16
#define DPA_PWR_ALLOC_VAL26_MASK ( 0xff << DPA_PWR_ALLOC_VAL26_POSITION)

#define DPA_PWR_ALLOC_VAL27_POSITION 24
#define DPA_PWR_ALLOC_VAL27_MASK ( 0xff << DPA_PWR_ALLOC_VAL27_POSITION)

#define DPA_PWR_ALLOC_VAL28_POSITION 0
#define DPA_PWR_ALLOC_VAL28_MASK ( 0xff << DPA_PWR_ALLOC_VAL28_POSITION)

#define DPA_PWR_ALLOC_VAL29_POSITION 8
#define DPA_PWR_ALLOC_VAL29_MASK ( 0xff << DPA_PWR_ALLOC_VAL29_POSITION)

#define DPA_PWR_ALLOC_VAL30_POSITION 16
#define DPA_PWR_ALLOC_VAL30_MASK ( 0xff << DPA_PWR_ALLOC_VAL30_POSITION)

#define DPA_PWR_ALLOC_VAL31_POSITION 24
#define DPA_PWR_ALLOC_VAL31_MASK ( 0xff << DPA_PWR_ALLOC_VAL31_POSITION)

/////////////////////////////////////////////////////////////////////////////
// FRSQ Capablity
///////////////////////////////////////////////////////////////////////////
#define FRSQ_EXT_CAP_ID_POSITION 0
#define FRSQ_EXT_CAP_ID_MASK (0xffff << FRSQ_EXT_CAP_ID_POSITION)
#define FRSQ_CAP_VERSION_POSITION 16
#define FRSQ_CAP_VERSION_MASK (0xf << FRSQ_CAP_VERSION_POSITION)
#define FRSQ_NEXT_OFFSET_POSITION 20
#define FRSQ_NEXT_OFFSET_MASK   (0xfff << FRSQ_NEXT_OFFSET_POSITION)

#define FRSQ_MAX_DEPTH_POSITION   0
#define FRSQ_MAX_DEPTH_MASK       (0xfff << FRSQ_MAX_DEPTH_POSITION)
#define FRS_INT_MESSAGE_NUMBER_POSITION 16
#define FRS_INT_MESSAGE_NUMBER_MASK  (0x1f << FRS_INT_MESSAGE_NUMBER_POSITION)

#define FRS_MESSAGE_RECEIVED_POSITION 0
#define FRS_MESSAGE_RECEIVED_MASK     (0x1 << FRS_MESSAGE_RECEIVED_POSITION)
#define FRS_MESSAGE_OVERFLOW_POSITION  1
#define FRS_MESSAGE_OVERFLOW_MASK     (0x1 << FRS_MESSAGE_OVERFLOW_POSITION)
#define FRS_INTERRUPT_ENABLE_POSITION  16
#define FRS_INTERRUPT_ENABLE_MASK      (0x1 << FRS_INTERRUPT_ENABLE_POSITION)

#define FRS_MESSAGE_QUE_FUNC_ID_POSITION 0
#define FRS_MESSAGE_QUE_FUNC_ID_MASK    (0xffff << FRS_MESSAGE_QUE_FUNC_ID_POSITION)
#define FRS_MESSAGE_QUE_REASON_POSITION  16
#define FRS_MESSAGE_QUE_REASON_MASK      (0xf << FRS_MESSAGE_QUE_REASON_POSITION)
#define FRS_MESSAGE_QUE_DEPTH_POSITION   20
#define FRS_MESSAGE_QUE_DEPTH_MASK       (0xfff << FRS_MESSAGE_QUE_DEPTH_POSITION)


/////////////////////////////////////////////////////////////////////////////
// RTR Capablity
///////////////////////////////////////////////////////////////////////////
#define RTR_EXT_CAP_ID_POSITION 0
#define RTR_EXT_CAP_ID_MASK (0xffff << RTR_EXT_CAP_ID_POSITION)

#define RTR_CAP_VERSION_POSITION 16
#define RTR_CAP_VERSION_MASK (0xf << RTR_CAP_VERSION_POSITION)

#define RTR_NEXT_OFFSET_POSITION 20
#define RTR_NEXT_OFFSET_MASK   (0xfff << RTR_NEXT_OFFSET_POSITION)

#define RESET_TIME_POSITION   0
#define RESET_TIME_MASK  (0xfff << RESET_TIME_POSITION)

#define DL_UP_TIME_POSITION 12
#define DL_UP_TIME_MASK  (0xfff << DL_UP_TIME_POSITION)

#define RTR_TIME_VALID_POSITION 31
#define RTR_TIME_VALID_MASK (0x1 << RTR_TIME_VALID_POSITION)

#define FLR_TIME_POSITION 0
#define FLR_TIME_MASK   (0xfff << FLR_TIME_POSITION)

#define D3HOT_D0_TIME_POSITION 12
#define D3HOT_D0_TIME_MASK (0xfff << D3HOT_D0_TIME_POSITION)


//////////////////////////////////////////////////////
//LNR
#define LNR_EXT_CAP_ID_POSITION 0
#define LNR_EXT_CAP_ID_MASK  (0xffff << LNR_EXT_CAP_ID_POSITION)

#define LNR_CAP_VERSION_POSITION 16
#define LNR_CAP_VERSION_MASK (0xf << LNR_CAP_VERSION_POSITION)

#define LNR_NEXT_OFFSET_POSITION 20
#define LNR_NEXT_OFFSET_MASK (0xfff << LNR_NEXT_OFFSET_POSITION)

#define LNR_64_SUPPORTED_POSITION 0
#define LNR_64_SUPPORTED_MASK (0x1 << LNR_64_SUPPORTED_POSITION)

#define LNR_128_SUPPORTED_POSITION 1
#define LNR_128_SUPPORTED_MASK (0x1 << LNR_128_SUPPORTED_POSITION)

#define LNR_REGISTRATION_MAX_POSITION 8
#define LNR_REGISTRATION_MAX_MASK (0x1f << LNR_REGISTRATION_MAX_POSITION)

#define LNR_ENABLE_POSITION 16
#define LNR_ENABLE_MASK    (0x1 << LNR_ENABLE_POSITION)

#define LNR_CLS_POSITION 17
#define LNR_CLS_MASK       (0x1 << LNR_CLS_POSITION)

#define LNR_REGISTRATION_LIMIT_POSITION  24
#define LNR_REGISTRATION_LIMIT_MASK  (0x1f << LNR_REGISTRATION_LIMIT_POSITION)


/////////////////////////////////////////////
// LTR Capability
////////////////////////////////////////////

#define  LTR_CAP_ID_POSITION               0
#define  LTR_CAP_ID_MASK                   (0xffff << LTR_CAP_ID_POSITION)

#define  LTR_CAP_VERSION_POSITION          16
#define  LTR_CAP_VERSION_MASK              (0xf << LTR_CAP_VERSION_POSITION)

#define  LTR_NEXT_OFFSET_POSITION           20
#define  LTR_NEXT_OFFSET_MASK              (0xfff << LTR_NEXT_OFFSET_POSITION)

#define  LTR_MAX_SNOOP_LAT_POSITION           0
#define  LTR_MAX_SNOOP_LAT_MASK              (0x3ff << LTR_MAX_SNOOP_LAT_POSITION)

#define  LTR_MAX_SNOOP_LAT_SCALE_POSITION           10
#define  LTR_MAX_SNOOP_LAT_SCALE_MASK              (0x7 << LTR_MAX_SNOOP_LAT_SCALE_POSITION)

#define  LTR_MAX_NO_SNOOP_LAT_POSITION           16
#define  LTR_MAX_NO_SNOOP_LAT_MASK              (0x3ff << LTR_MAX_NO_SNOOP_LAT_POSITION)

#define  LTR_MAX_NO_SNOOP_LAT_SCALE_POSITION           26
#define  LTR_MAX_NO_SNOOP_LAT_SCALE_MASK              (0x7 << LTR_MAX_NO_SNOOP_LAT_SCALE_POSITION)


/////////////////////////////////////////////
// Port Logic
////////////////////////////////////////////
#define  PCIE_PL_ROUND_TRIP_LATENCY_TIME_LIMIT_POSITION 0
#define  PCIE_PL_ROUND_TRIP_LATENCY_TIME_LIMIT_MASK (0xffff << PCIE_PL_ROUND_TRIP_LATENCY_TIME_LIMIT_POSITION)
#define  PCIE_PL_REPLAY_TIME_LIMIT_POSITION 16
#define  PCIE_PL_REPLAY_TIME_LIMIT_MASK (0xffff << PCIE_PL_REPLAY_TIME_LIMIT_POSITION)

#define  PCIE_PL_VENDOR_SPEC_DLLP_POSITION 0
#define  PCIE_PL_VENDOR_SPEC_DLLP_MASK (0xffff_ffff << PCIE_PL_VENDOR_SPEC_DLLP_POSITION)

#define  PCIE_PL_LINK_NUM_POSITION 0
#define  PCIE_PL_LINK_NUM_MASK (0xff << PCIE_PL_LINK_NUM_POSITION)
#define  PCIE_PL_FORCED_LTSSM_POSITION 8
#define  PCIE_PL_FORCED_LTSSM_MASK (0xf << PCIE_PL_FORCED_LTSSM_POSITION)
#define  PCIE_PL_FORCE_EN_POSITION 15
#define  PCIE_PL_FORCE_EN_MASK (0x1 << PCIE_PL_FORCE_EN_POSITION)
#define  PCIE_PL_LINK_STATE_POSITION 16
#define  PCIE_PL_LINK_STATE_MASK (0x3f << PCIE_PL_LINK_STATE_POSITION)
#define  PCIE_PL_DO_DESKEW_FOR_SRIS_POSITION 23
#define  PCIE_PL_DO_DESKEW_FOR_SRIS_MASK (0x1 << PCIE_PL_DO_DESKEW_FOR_SRIS_POSITION)

#define  PCIE_PL_ACK_FREQ_POSITION 0
#define  PCIE_PL_ACK_FREQ_MASK (0xff << PCIE_PL_ACK_FREQ_POSITION)
#define  PCIE_PL_ACK_N_FTS_POSITION 8
#define  PCIE_PL_ACK_N_FTS_MASK (0xff << PCIE_PL_ACK_N_FTS_POSITION)
#define  PCIE_PL_COMMON_CLK_N_FTS_POSITION 16
#define  PCIE_PL_COMMON_CLK_N_FTS_MASK (0xff << PCIE_PL_COMMON_CLK_N_FTS_POSITION)
#define  PCIE_PL_L0S_ENTRANCE_LATENCY_POSITION 24
#define  PCIE_PL_L0S_ENTRANCE_LATENCY_MASK (0x7 << PCIE_PL_L0S_ENTRANCE_LATENCY_POSITION)
#define  PCIE_PL_L1_ENTRANCE_LATENCY_POSITION 27
#define  PCIE_PL_L1_ENTRANCE_LATENCY_MASK (0x7 << PCIE_PL_L1_ENTRANCE_LATENCY_POSITION)
#define  PCIE_PL_ENTER_ASPM_POSITION 30
#define  PCIE_PL_ENTER_ASPM_MASK (0x1 << PCIE_PL_ENTER_ASPM_POSITION)
/////port link control register
#define PCIE_PL_VENDOR_SPECIFIC_DLLP_REQ_POSITION 0
#define PCIE_PL_VENDOR_SPECIFIC_DLLP_REQ_MASK  (0x1<<PCIE_PL_VENDOR_SPECIFIC_DLLP_REQ_POSITION)
#define PCIE_PL_SCRAMBLE_DISABLE_POSITION 1
#define PCIE_PL_SCRAMBLE_DISABLE_MASK  (0x1<<PCIE_PL_SCRAMBLE_DISABLE_POSITION)
#define PCIE_PL_LOOPBACK_ENABLE_POSITION 2
#define PCIE_PL_LOOPBACK_ENABLE_MASK  (0x1<<PCIE_PL_LOOPBACK_ENABLE_POSITION)
#define PCIE_PL_RESET_ASSERT_POSITION 3
#define PCIE_PL_RESET_ASSERT_MASK  (0x1<<PCIE_PL_RESET_ASSERT_POSITION)
#define PCIE_PL_IGNORE_SLOT_POWER_MESSAGE_POSITION 4
#define PCIE_PL_IGNORE_SLOT_POWER_MESSAGE_MASK  (0x1<<PCIE_PL_IGNORE_SLOT_POWER_MESSAGE_POSITION)
#define PCIE_PL_DLL_LINK_EN_POSITION 5
#define PCIE_PL_DLL_LINK_EN_MASK  (0x1<<PCIE_PL_DLL_LINK_EN_POSITION)
#define PCIE_PL_LINK_DISABLE_POSITION 6
#define PCIE_PL_LINK_DISABLE_MASK  (0x1<<PCIE_PL_LINK_DISABLE_POSITION)
#define  PCIE_PL_FAST_LINK_MODE_POSITION 7
#define  PCIE_PL_FAST_LINK_MODE_MASK (0x1 << PCIE_PL_FAST_LINK_MODE_POSITION)

#define PCIE_PL_LINK_RATE_POSITION 8
#define PCIE_PL_LINK_RATE_MASK  (0xf<<PCIE_PL_LINK_RATE_POSITION)

//ifdef ENABLE_FEATURE_SET_T
//    #define PCIE_PL_INTERRUPT_PENDING_POSITION 12
//    #define PCIE_PL_INTERRUPT_PENDING_MASK  (0xf<<PCIE_PL_INTERRUPT_PENDING_POSITION)
//else
//    #define PCIE_PL_RESERVED1_POSITION 12
//    #define PCIE_PL_RESERVED1_MASK  (0xf<<PCIE_PL_RESERVED1_POSITION)
//endif


#define PCIE_PL_LINK_CAPABLE_POSITION 16
#define PCIE_PL_LINK_CAPABLE_MASK  (0x3f<<PCIE_PL_LINK_CAPABLE_POSITION)

   #define PCIE_PL_CROSS_LINK_EN_POSITION 22
   #define PCIE_PL_CROSS_LINK_EN_MASK  (0x1<<PCIE_PL_CROSS_LINK_EN_POSITION)
   #define PCIE_PL_CROSS_LINK_ACTIVE_POSITION 23
   #define PCIE_PL_CROSS_LINK_ACTIVE_MASK  (0x1<<PCIE_PL_CROSS_LINK_ACTIVE_POSITION)
// !CX_CROSSLINK_ENABLE
    #define PCIE_PL_RESERVED2_POSITION 22
    #define PCIE_PL_RESERVED2_MASK (0x3 << PCIE_PL_RESERVED2_POSITION)

#define PCIE_PL_BEACON_ENABLE_POSITION 24
#define PCIE_PL_BEACON_ENABLE_MASK   (0x1<<PCIE_PL_BEACON_ENABLE_POSITION)
#define PCIE_PL_CORRUPT_LCRC_ENABLE_POSITION 25
#define PCIE_PL_CORRUPT_LCRC_ENABLE_MASK   (0x1<<PCIE_PL_CORRUPT_LCRC_ENABLE_POSITION)
#define PCIE_PL_EXTENDED_SYNCH_POSITION 26
#define PCIE_PL_EXTENDED_SYNCH_MASK   (0x1<<PCIE_PL_EXTENDED_SYNCH_POSITION)
#define PCIE_PL_TRANSMIT_LANE_REVERSALE_ENABLE_POSITION 27
#define PCIE_PL_TRANSMIT_LANE_REVERSALE_ENABLE_MASK  (0x1<<PCIE_PL_TRANSMIT_LANE_REVERSALE_ENABLE_POSITION)
//ifdef ENABLE_FEATURE_SET_T
//   #define PCIE_PL_INTERRUPT_ASSERTED_POSITION 28
//   #define PCIE_PL_INTERRUPT_ASSERTED_MASK   (0xf<<PCIE_PL_INTERRUPT_ASSERTED_POSITION)
//else
//   #define PCIE_PL_RESERVED3_POSITION 28
//   #define PCIE_PL_RESERVED3_MASK  (0xf<<PCIE_PL_RESERVED3_POSITION)
//endif

///////
#define  PCIE_PL_INSERT_LANE_SKEW_POSITION 0
#define  PCIE_PL_INSERT_LANE_SKEW_MASK (0xffffff << PCIE_PL_INSERT_LANE_SKEW_POSITION)
#define  PCIE_PL_FLOW_CTRL_DISABLE_POSITION 24
#define  PCIE_PL_FLOW_CTRL_DISABLE_MASK (0x1 << PCIE_PL_FLOW_CTRL_DISABLE_POSITION)
#define  PCIE_PL_ACK_NAK_DISABLE_POSITION 25
#define  PCIE_PL_ACK_NAK_DISABLE_MASK (0x1 << PCIE_PL_ACK_NAK_DISABLE_POSITION)
#define  PCIE_PL_ELASTIC_BUFFER_MODE_POSITION 26
#define  PCIE_PL_ELASTIC_BUFFER_MODE_MASK (0x1 << PCIE_PL_ELASTIC_BUFFER_MODE_POSITION)
#define  PCIE_PL_IMPLEMENT_NUM_LANES_POSITION 27
#define  PCIE_PL_IMPLEMENT_NUM_LANES_MASK (0xf << PCIE_PL_IMPLEMENT_NUM_LANES_POSITION)
#define  PCIE_PL_DISABLE_LANE_TO_LANE_DESKEW_POSITION 31
#define  PCIE_PL_DISABLE_LANE_TO_LANE_DESKEW_MASK (0x1 << PCIE_PL_DISABLE_LANE_TO_LANE_DESKEW_POSITION)

#define  PCIE_PL_MAX_FUNC_NUM_POSITION 0
#define  PCIE_PL_MAX_FUNC_NUM_MASK (0xff << PCIE_PL_MAX_FUNC_NUM_POSITION)
#define  PCIE_PL_TIMER_MOD_REPLAY_TIMER_POSITION 14
#define  PCIE_PL_TIMER_MOD_REPLAY_TIMER_MASK (0x1f << PCIE_PL_TIMER_MOD_REPLAY_TIMER_POSITION)
#define  PCIE_PL_TIMER_MOD_ACK_NAK_POSITION 19
#define  PCIE_PL_TIMER_MOD_ACK_NAK_MASK (0x1f << PCIE_PL_TIMER_MOD_ACK_NAK_POSITION)
#define  PCIE_PL_UPDATE_FREQ_TIMER_POSITION 24
#define  PCIE_PL_UPDATE_FREQ_TIMER_MASK (0x1f << PCIE_PL_UPDATE_FREQ_TIMER_POSITION)
#define  PCIE_PL_FAST_LINK_SCALING_FACTOR_POSITION 29
#define  PCIE_PL_FAST_LINK_SCALING_FACTOR_MASK (0x3 << PCIE_PL_FAST_LINK_SCALING_FACTOR_POSITION)

#define  PCIE_PL_SKP_INT_VAL_POSITION 0
#define  PCIE_PL_SKP_INT_VAL_MASK (0x7ff << PCIE_PL_SKP_INT_VAL_POSITION)
#define  PCIE_PL_EIDLE_TIMER_POSITION 11
#define  PCIE_PL_EIDLE_TIMER_MASK (0xf << PCIE_PL_EIDLE_TIMER_POSITION)
#define  PCIE_PL_DISABLE_FC_WD_TIMER_POSITION 15
#define  PCIE_PL_DISABLE_FC_WD_TIMER_MASK (0x1 << PCIE_PL_DISABLE_FC_WD_TIMER_POSITION)
#define  PCIE_PL_MASK_RADM_1_POSITION 16
#define  PCIE_PL_MASK_RADM_1_MASK (0xffff << PCIE_PL_MASK_RADM_1_POSITION)

#define  PCIE_PL_MASK_RADM_2_POSITION 0
#define  PCIE_PL_MASK_RADM_2_MASK (0xffff_ffff << PCIE_PL_MASK_RADM_2_POSITION)

#define  PCIE_PL_OB_RD_SPLIT_BURST_EN_POSITION 0
#define  PCIE_PL_OB_RD_SPLIT_BURST_EN_MASK (0x1 << PCIE_PL_OB_RD_SPLIT_BURST_EN_POSITION)

#define  PCIE_PL_SIMPLIFIED_REPLAY_TIMER_POSITION 3
#define  PCIE_PL_SIMPLIFIED_REPLAY_TIMER_MASK (0x1 << PCIE_PL_SIMPLIFIED_REPLAY_TIMER_POSITION)

#define  PCIE_PL_DEB_REG_0_POSITION 0
#define  PCIE_PL_DEB_REG_0_MASK (0xffff_ffff << PCIE_PL_DEB_REG_0_POSITION)

#define  PCIE_PL_DEB_REG_1_POSITION 0
#define  PCIE_PL_DEB_REG_1_MASK (0xffff_ffff << PCIE_PL_DEB_REG_1_POSITION)

#define  PCIE_PL_TX_P_DATA_FC_CREDIT_POSITION 0
#define  PCIE_PL_TX_P_DATA_FC_CREDIT_MASK       (0xfff << PCIE_PL_TX_P_DATA_FC_CREDIT_POSITION)
#define  PCIE_PL_TX_P_HEADER_FC_CREDIT_POSITION 12
#define  PCIE_PL_TX_P_HEADER_FC_CREDIT_MASK       (0xff << PCIE_PL_TX_P_HEADER_FC_CREDIT_POSITION)

#define  PCIE_PL_TX_NP_DATA_FC_CREDIT_POSITION 0
#define  PCIE_PL_TX_NP_DATA_FC_CREDIT_MASK       (0xfff << PCIE_PL_TX_NP_DATA_FC_CREDIT_POSITION)
#define  PCIE_PL_TX_NP_HEADER_FC_CREDIT_POSITION 12
#define  PCIE_PL_TX_NP_HEADER_FC_CREDIT_MASK       (0xff << PCIE_PL_TX_NP_HEADER_FC_CREDIT_POSITION)

#define  PCIE_PL_TX_CPL_DATA_FC_CREDIT_POSITION 0
#define  PCIE_PL_TX_CPL_DATA_FC_CREDIT_MASK       (0xfff << PCIE_PL_TX_CPL_DATA_FC_CREDIT_POSITION)
#define  PCIE_PL_TX_CPL_HEADER_FC_CREDIT_POSITION 12
#define  PCIE_PL_TX_CPL_HEADER_FC_CREDIT_MASK       (0xff << PCIE_PL_TX_CPL_HEADER_FC_CREDIT_POSITION)

#define  PCIE_PL_RX_TLP_FC_CREDIT_NON_RETURN_POSITION 0
#define  PCIE_PL_RX_TLP_FC_CREDIT_NON_RETURN_MASK       (0x1 << PCIE_PL_RX_TLP_FC_CREDIT_NON_RETURN_POSITION)
#define  PCIE_PL_TX_RETRY_BUFFER_NE_POSITION 1
#define  PCIE_PL_TX_RETRY_BUFFER_NE_MASK       (0x1 << PCIE_PL_TX_RETRY_BUFFER_NE_POSITION)
#define  PCIE_PL_RX_QUEUE_NON_EMPTY_POSITION 2
#define  PCIE_PL_RX_QUEUE_NON_EMPTY_MASK       (0x1 << PCIE_PL_RX_QUEUE_NON_EMPTY_POSITION)
#define  PCIE_PL_RX_QUEUE_OVERFLOW_POSITION 3
#define  PCIE_PL_RX_QUEUE_OVERFLOW_MASK       (0x1 << PCIE_PL_RX_QUEUE_OVERFLOW_POSITION)
#define  PCIE_PL_RX_SERIALIZATION_Q_NON_EMPTY_POSITION 13
#define  PCIE_PL_RX_SERIALIZATION_Q_NON_EMPTY_MASK       (0x1 << PCIE_PL_RX_SERIALIZATION_Q_NON_EMPTY_POSITION)
#define  PCIE_PL_RX_SERIALIZATION_Q_WRITE_ERR_POSITION 14
#define  PCIE_PL_RX_SERIALIZATION_Q_WRITE_ERR_MASK       (0x1 << PCIE_PL_RX_SERIALIZATION_Q_WRITE_ERR_POSITION)
#define  PCIE_PL_RX_SERIALIZATION_Q_READ_ERR_POSITION 15
#define  PCIE_PL_RX_SERIALIZATION_Q_READ_ERR_MASK       (0x1 << PCIE_PL_RX_SERIALIZATION_Q_READ_ERR_POSITION)
#define  PCIE_PL_TIMER_MOD_FLOW_CONTROL_POSITION 16
#define  PCIE_PL_TIMER_MOD_FLOW_CONTROL_MASK (0x1fff << PCIE_PL_TIMER_MOD_FLOW_CONTROL_POSITION)
#define  PCIE_PL_TIMER_MOD_FLOW_CONTROL_EN_POSITION 31
#define  PCIE_PL_TIMER_MOD_FLOW_CONTROL_EN_MASK (0x1 << PCIE_PL_TIMER_MOD_FLOW_CONTROL_EN_POSITION)

#define  PCIE_PL_WRR_WEIGHT_VC_0_POSITION 0
#define  PCIE_PL_WRR_WEIGHT_VC_0_MASK       (0xff << PCIE_PL_WRR_WEIGHT_VC_0_POSITION)
#define  PCIE_PL_WRR_WEIGHT_VC_1_POSITION 8
#define  PCIE_PL_WRR_WEIGHT_VC_1_MASK       (0xff << PCIE_PL_WRR_WEIGHT_VC_1_POSITION)
#define  PCIE_PL_WRR_WEIGHT_VC_2_POSITION 16
#define  PCIE_PL_WRR_WEIGHT_VC_2_MASK       (0xff << PCIE_PL_WRR_WEIGHT_VC_2_POSITION)
#define  PCIE_PL_WRR_WEIGHT_VC_3_POSITION 24
#define  PCIE_PL_WRR_WEIGHT_VC_3_MASK       (0xff << PCIE_PL_WRR_WEIGHT_VC_3_POSITION)
#define  PCIE_PL_WRR_WEIGHT_VC_4_POSITION 0
#define  PCIE_PL_WRR_WEIGHT_VC_4_MASK       (0xff << PCIE_PL_WRR_WEIGHT_VC_4_POSITION)
#define  PCIE_PL_WRR_WEIGHT_VC_5_POSITION 8
#define  PCIE_PL_WRR_WEIGHT_VC_5_MASK       (0xff << PCIE_PL_WRR_WEIGHT_VC_5_POSITION)
#define  PCIE_PL_WRR_WEIGHT_VC_6_POSITION 16
#define  PCIE_PL_WRR_WEIGHT_VC_6_MASK       (0xff << PCIE_PL_WRR_WEIGHT_VC_6_POSITION)
#define  PCIE_PL_WRR_WEIGHT_VC_7_POSITION 24
#define  PCIE_PL_WRR_WEIGHT_VC_7_MASK       (0xff << PCIE_PL_WRR_WEIGHT_VC_7_POSITION)

#define  PCIE_PL_VC0_P_DATA_CREDIT_POSITION 0
#define  PCIE_PL_VC0_P_DATA_CREDIT_MASK (0xfff << PCIE_PL_VC0_P_DATA_CREDIT_POSITION)
#define  PCIE_PL_VC0_P_HEADER_CREDIT_POSITION 12
#define  PCIE_PL_VC0_P_HEADER_CREDIT_MASK (0xff << PCIE_PL_VC0_P_HEADER_CREDIT_POSITION)
#define  PCIE_PL_RESERVED4_POSITION 20
#define  PCIE_PL_RESERVED4_MASK (0x1 << PCIE_PL_RESERVED4_POSITION)
#define  PCIE_PL_VC0_P_TLP_Q_MODE_POSITION 21
#define  PCIE_PL_VC0_P_TLP_Q_MODE_MASK  (0x7 << PCIE_PL_VC0_P_TLP_Q_MODE_POSITION)
#define  PCIE_PL_VC0_P_HDR_SCALE_POSITION 24
#define  PCIE_PL_VC0_P_HDR_SCALE_MASK  (0x3 << PCIE_PL_VC0_P_HDR_SCALE_POSITION)
#define  PCIE_PL_VC0_P_DATA_SCALE_POSITION 26
#define  PCIE_PL_VC0_P_DATA_SCALE_MASK (0x3 << PCIE_PL_VC0_P_DATA_SCALE_POSITION)
#define  PCIE_PL_RESERVED5_POSITION 28
#define  PCIE_PL_RESERVED5_MASK (0x3 << PCIE_PL_RESERVED5_POSITION)
#define  PCIE_PL_TLP_TYPE_ORDERING_VC0_POSITION 30
#define  PCIE_PL_TLP_TYPE_ORDERING_VC0_MASK    (0x1 << PCIE_PL_TLP_TYPE_ORDERING_VC0_POSITION)
#define  PCIE_PL_VC_ORDERING_RX_Q_POSITION 31
#define  PCIE_PL_VC_ORDERING_RX_Q_MASK       (0x1 << PCIE_PL_VC_ORDERING_RX_Q_POSITION)

#define  PCIE_PL_VC0_NP_DATA_CREDIT_POSITION 0
#define  PCIE_PL_VC0_NP_DATA_CREDIT_MASK       (0xfff << PCIE_PL_VC0_NP_DATA_CREDIT_POSITION)
#define  PCIE_PL_VC0_NP_HEADER_CREDIT_POSITION 12
#define  PCIE_PL_VC0_NP_HEADER_CREDIT_MASK       (0xff << PCIE_PL_VC0_NP_HEADER_CREDIT_POSITION)
#define  PCIE_PL_RESERVED6_POSITION 20
#define  PCIE_PL_RESERVED6_MASK (0x1 << PCIE_PL_RESERVED6_POSITION)
#define  PCIE_PL_VC0_NP_TLP_Q_MODE_POSITION 21
#define  PCIE_PL_VC0_NP_TLP_Q_MODE_MASK   (0x7 << PCIE_PL_VC0_NP_TLP_Q_MODE_POSITION)
#define  PCIE_PL_VC0_NP_HDR_SCALE_POSITION 24
#define  PCIE_PL_VC0_NP_HDR_SCALE_MASK  (0x3 << PCIE_PL_VC0_NP_HDR_SCALE_POSITION)
#define  PCIE_PL_VC0_NP_DATA_SCALE_POSITION 26
#define  PCIE_PL_VC0_NP_DATA_SCALE_MASK (0x3 << PCIE_PL_VC0_NP_DATA_SCALE_POSITION)
#define  PCIE_PL_RESERVED7_POSITION 28
#define  PCIE_PL_RESERVED7_MASK (0xf << PCIE_PL_RESERVED7_POSITION)


#define  PCIE_PL_VC0_CPL_DATA_CREDIT_POSITION 0
#define  PCIE_PL_VC0_CPL_DATA_CREDIT_MASK       (0xfff << PCIE_PL_VC0_CPL_DATA_CREDIT_POSITION)
#define  PCIE_PL_VC0_CPL_HEADER_CREDIT_POSITION 12
#define  PCIE_PL_VC0_CPL_HEADER_CREDIT_MASK      (0xff << PCIE_PL_VC0_CPL_HEADER_CREDIT_POSITION)
#define  PCIE_PL_RESERVED8_POSITION 20
#define  PCIE_PL_RESERVED8_MASK (0x1 << PCIE_PL_RESERVED8_POSITION)
#define  PCIE_PL_VC0_CPL_TLP_Q_MODE_POSITION 21
#define  PCIE_PL_VC0_CPL_TLP_Q_MODE_MASK       (0x7 << PCIE_PL_VC0_CPL_TLP_Q_MODE_POSITION)
#define  PCIE_PL_VC0_CPL_HDR_SCALE_POSITION 24
#define  PCIE_PL_VC0_CPL_HDR_SCALE_MASK  (0x3 << PCIE_PL_VC0_CPL_HDR_SCALE_POSITION)
#define  PCIE_PL_VC0_CPL_DATA_SCALE_POSITION 26
#define  PCIE_PL_VC0_CPL_DATA_SCALE_MASK (0x3 << PCIE_PL_VC0_CPL_DATA_SCALE_POSITION)
#define  PCIE_PL_RESERVED9_POSITION 28
#define  PCIE_PL_RESERVED9_MASK (0xf << PCIE_PL_RESERVED9_POSITION)


#define  PCIE_PL_VC1_P_DATA_CREDIT_POSITION 0
#define  PCIE_PL_VC1_P_DATA_CREDIT_MASK (0xfff << PCIE_PL_VC1_P_DATA_CREDIT_POSITION)
#define  PCIE_PL_VC1_P_HEADER_CREDIT_POSITION 12
#define  PCIE_PL_VC1_P_HEADER_CREDIT_MASK (0xff << PCIE_PL_VC1_P_HEADER_CREDIT_POSITION)
#define  PCIE_PL_RESERVED0_VC1_POSITION 20
#define  PCIE_PL_RESERVED0_VC1_MASK (0x1 << PCIE_PL_RESERVED0_VC1_POSITION)
#define  PCIE_PL_VC1_P_TLP_Q_MODE_POSITION 21
#define  PCIE_PL_VC1_P_TLP_Q_MODE_MASK       (0x7 << PCIE_PL_VC1_P_TLP_Q_MODE_POSITION)
#define  PCIE_PL_VC1_P_HDR_SCALE_POSITION 24
#define  PCIE_PL_VC1_P_HDR_SCALE_MASK  (0x3 << PCIE_PL_VC0_P_HDR_SCALE_POSITION)
#define  PCIE_PL_VC1_P_DATA_SCALE_POSITION 26
#define  PCIE_PL_VC1_P_DATA_SCALE_MASK (0x3 << PCIE_PL_VC0_P_DATA_SCALE_POSITION)
#define  PCIE_PL_RESERVED1_VC1_POSITION 28
#define  PCIE_PL_RESERVED1_VC1_MASK  (0x3 << PCIE_PL_RESERVED1_VC1_POSITION)
#define  PCIE_PL_TLP_TYPE_ORDERING_VC1_POSITION 30
#define  PCIE_PL_TLP_TYPE_ORDERING_VC1_MASK       (0x1 << PCIE_PL_TLP_TYPE_ORDERING_VC1_POSITION)
#define  PCIE_PL_RESERVED2_VC1_POSITION 31
#define  PCIE_PL_RESERVED2_VC1_MASK (0x1 << PCIE_PL_RESERVED2_VC1_POSITION)

#define  PCIE_PL_VC1_NP_DATA_CREDIT_POSITION 0
#define  PCIE_PL_VC1_NP_DATA_CREDIT_MASK       (0xfff << PCIE_PL_VC1_NP_DATA_CREDIT_POSITION)
#define  PCIE_PL_VC1_NP_HEADER_CREDIT_POSITION 12
#define  PCIE_PL_VC1_NP_HEADER_CREDIT_MASK      (0xff << PCIE_PL_VC1_NP_HEADER_CREDIT_POSITION)
#define  PCIE_PL_RESERVED3_VC1_POSITION 20
#define  PCIE_PL_RESERVED3_VC1_MASK (0x1 << PCIE_PL_RESERVED3_VC1_POSITION)
#define  PCIE_PL_VC1_NP_TLP_Q_MODE_POSITION 21
#define  PCIE_PL_VC1_NP_TLP_Q_MODE_MASK       (0x7 << PCIE_PL_VC1_NP_TLP_Q_MODE_POSITION)
#define  PCIE_PL_VC1_NP_HDR_SCALE_POSITION 24
#define  PCIE_PL_VC1_NP_HDR_SCALE_MASK  (0x3 << PCIE_PL_VC0_NP_HDR_SCALE_POSITION)
#define  PCIE_PL_VC1_NP_DATA_SCALE_POSITION 26
#define  PCIE_PL_VC1_NP_DATA_SCALE_MASK (0x3 << PCIE_PL_VC0_NP_DATA_SCALE_POSITION)
#define  PCIE_PL_RESERVED4_VC1_POSITION 28
#define  PCIE_PL_RESERVED4_VC1_MASK (0xf << PCIE_PL_RESERVED4_VC1_POSITION)


#define  PCIE_PL_VC1_CPL_DATA_CREDIT_POSITION 0
#define  PCIE_PL_VC1_CPL_DATA_CREDIT_MASK       (0xfff << PCIE_PL_VC1_CPL_DATA_CREDIT_POSITION)
#define  PCIE_PL_VC1_CPL_HEADER_CREDIT_POSITION 12
#define  PCIE_PL_VC1_CPL_HEADER_CREDIT_MASK       (0xff << PCIE_PL_VC1_CPL_HEADER_CREDIT_POSITION)
#define  PCIE_PL_RESERVED5_VC1_POSITION 20
#define  PCIE_PL_RESERVED5_VC1_MASK (0x1 << PCIE_PL_RESERVED5_VC1_POSITION)
#define  PCIE_PL_VC1_CPL_TLP_Q_MODE_POSITION 21
#define  PCIE_PL_VC1_CPL_TLP_Q_MODE_MASK       (0x7 << PCIE_PL_VC1_CPL_TLP_Q_MODE_POSITION)
#define  PCIE_PL_VC1_CPL_HDR_SCALE_POSITION 24
#define  PCIE_PL_VC1_CPL_HDR_SCALE_MASK  (0x3 << PCIE_PL_VC0_CPL_HDR_SCALE_POSITION)
#define  PCIE_PL_VC1_CPL_DATA_SCALE_POSITION 26
#define  PCIE_PL_VC1_CPL_DATA_SCALE_MASK (0x3 << PCIE_PL_VC0_CPL_DATA_SCALE_POSITION)
#define  PCIE_PL_RESERVED6_VC1_POSITION 28
#define  PCIE_PL_RESERVED6_VC1_MASK (0xf << PCIE_PL_RESERVED6_VC1_POSITION)


#define  PCIE_PL_VC2_P_DATA_CREDIT_POSITION 0
#define  PCIE_PL_VC2_P_DATA_CREDIT_MASK (0xfff << PCIE_PL_VC2_P_DATA_CREDIT_POSITION)
#define  PCIE_PL_VC2_P_HEADER_CREDIT_POSITION 12
#define  PCIE_PL_VC2_P_HEADER_CREDIT_MASK (0xff << PCIE_PL_VC2_P_HEADER_CREDIT_POSITION)
#define  PCIE_PL_RESERVED0_VC2_POSITION 20
#define  PCIE_PL_RESERVED0_VC2_MASK (0x1 << PCIE_PL_RESERVED0_VC2_POSITION)
#define  PCIE_PL_VC2_P_TLP_Q_MODE_POSITION 21
#define  PCIE_PL_VC2_P_TLP_Q_MODE_MASK       (0x7 << PCIE_PL_VC2_P_TLP_Q_MODE_POSITION)
#define  PCIE_PL_RESERVED1_VC2_POSITION 28
#define  PCIE_PL_RESERVED1_VC2_MASK  (0x3 << PCIE_PL_RESERVED1_VC2_POSITION)
#define  PCIE_PL_VC2_P_HDR_SCALE_POSITION 24
#define  PCIE_PL_VC2_P_HDR_SCALE_MASK  (0x3 << PCIE_PL_VC0_P_HDR_SCALE_POSITION)
#define  PCIE_PL_VC2_P_DATA_SCALE_POSITION 26
#define  PCIE_PL_VC2_P_DATA_SCALE_MASK (0x3 << PCIE_PL_VC0_P_DATA_SCALE_POSITION)
#define  PCIE_PL_TLP_TYPE_ORDERING_VC2_POSITION 30
#define  PCIE_PL_TLP_TYPE_ORDERING_VC2_MASK       (0x1 << PCIE_PL_TLP_TYPE_ORDERING_VC2_POSITION)
#define  PCIE_PL_RESERVED2_VC2_POSITION 31
#define  PCIE_PL_RESERVED2_VC2_MASK (0x1 << PCIE_PL_RESERVED2_VC2_POSITION)


#define  PCIE_PL_VC2_NP_DATA_CREDIT_POSITION 0
#define  PCIE_PL_VC2_NP_DATA_CREDIT_MASK      (0xfff << PCIE_PL_VC2_NP_DATA_CREDIT_POSITION)
#define  PCIE_PL_VC2_NP_HEADER_CREDIT_POSITION 12
#define  PCIE_PL_VC2_NP_HEADER_CREDIT_MASK       (0xff << PCIE_PL_VC2_NP_HEADER_CREDIT_POSITION)
#define  PCIE_PL_RESERVED3_VC2_POSITION 20
#define  PCIE_PL_RESERVED3_VC2_MASK (0x1 << PCIE_PL_RESERVED3_VC2_POSITION)
#define  PCIE_PL_VC2_NP_TLP_Q_MODE_POSITION 21
#define  PCIE_PL_VC2_NP_TLP_Q_MODE_MASK       (0x7 << PCIE_PL_VC2_NP_TLP_Q_MODE_POSITION)
#define  PCIE_PL_VC2_NP_HDR_SCALE_POSITION 24
#define  PCIE_PL_VC2_NP_HDR_SCALE_MASK  (0x3 << PCIE_PL_VC0_NP_HDR_SCALE_POSITION)
#define  PCIE_PL_VC2_NP_DATA_SCALE_POSITION 26
#define  PCIE_PL_VC2_NP_DATA_SCALE_MASK (0x3 << PCIE_PL_VC0_NP_DATA_SCALE_POSITION)
#define  PCIE_PL_RESERVED4_VC2_POSITION 28
#define  PCIE_PL_RESERVED4_VC2_MASK (0xf << PCIE_PL_RESERVED4_VC1_POSITION)

#define  PCIE_PL_VC2_CPL_DATA_CREDIT_POSITION 0
#define  PCIE_PL_VC2_CPL_DATA_CREDIT_MASK      (0xfff << PCIE_PL_VC2_CPL_DATA_CREDIT_POSITION)
#define  PCIE_PL_VC2_CPL_HEADER_CREDIT_POSITION 12
#define  PCIE_PL_VC2_CPL_HEADER_CREDIT_MASK       (0xff << PCIE_PL_VC2_CPL_HEADER_CREDIT_POSITION)
#define  PCIE_PL_RESERVED5_VC2_POSITION 20
#define  PCIE_PL_RESERVED5_VC2_MASK (0x1 << PCIE_PL_RESERVED5_VC2_POSITION)
#define  PCIE_PL_VC2_CPL_TLP_Q_MODE_POSITION 21
#define  PCIE_PL_VC2_CPL_TLP_Q_MODE_MASK       (0x7 << PCIE_PL_VC2_CPL_TLP_Q_MODE_POSITION)
#define  PCIE_PL_VC2_CPL_HDR_SCALE_POSITION 24
#define  PCIE_PL_VC2_CPL_HDR_SCALE_MASK  (0x3 << PCIE_PL_VC0_CPL_HDR_SCALE_POSITION)
#define  PCIE_PL_VC2_CPL_DATA_SCALE_POSITION 26
#define  PCIE_PL_VC2_CPL_DATA_SCALE_MASK (0x3 << PCIE_PL_VC0_CPL_DATA_SCALE_POSITION)
#define  PCIE_PL_RESERVED6_VC2_POSITION 28
#define  PCIE_PL_RESERVED6_VC2_MASK (0xf << PCIE_PL_RESERVED6_VC1_POSITION)

#define  PCIE_PL_VC3_P_DATA_CREDIT_POSITION 0
#define  PCIE_PL_VC3_P_DATA_CREDIT_MASK (0xfff << PCIE_PL_VC3_P_DATA_CREDIT_POSITION)
#define  PCIE_PL_VC3_P_HEADER_CREDIT_POSITION 12
#define  PCIE_PL_VC3_P_HEADER_CREDIT_MASK (0xff << PCIE_PL_VC3_P_HEADER_CREDIT_POSITION)
#define  PCIE_PL_RESERVED0_VC3_POSITION 20
#define  PCIE_PL_RESERVED0_VC3_MASK (0x1 << PCIE_PL_RESERVED0_VC3_POSITION)
#define  PCIE_PL_VC3_P_TLP_Q_MODE_POSITION 21
#define  PCIE_PL_VC3_P_TLP_Q_MODE_MASK       (0x7 << PCIE_PL_VC3_P_TLP_Q_MODE_POSITION)
#define  PCIE_PL_VC3_P_HDR_SCALE_POSITION 24
#define  PCIE_PL_VC3_P_HDR_SCALE_MASK  (0x3 << PCIE_PL_VC0_P_HDR_SCALE_POSITION)
#define  PCIE_PL_VC3_P_DATA_SCALE_POSITION 26
#define  PCIE_PL_VC3_P_DATA_SCALE_MASK (0x3 << PCIE_PL_VC0_P_DATA_SCALE_POSITION)
#define  PCIE_PL_RESERVED1_VC3_POSITION 28
#define  PCIE_PL_RESERVED1_VC3_MASK  (0x3 << PCIE_PL_RESERVED1_VC3_POSITION)
#define  PCIE_PL_TLP_TYPE_ORDERING_VC3_POSITION 30
#define  PCIE_PL_TLP_TYPE_ORDERING_VC3_MASK       (0x1 << PCIE_PL_TLP_TYPE_ORDERING_VC3_POSITION)
#define  PCIE_PL_RESERVED2_VC3_POSITION 31
#define  PCIE_PL_RESERVED2_VC3_MASK (0x1 << PCIE_PL_RESERVED2_VC3_POSITION)

#define  PCIE_PL_VC3_NP_DATA_CREDIT_POSITION 0
#define  PCIE_PL_VC3_NP_DATA_CREDIT_MASK      (0xfff << PCIE_PL_VC3_NP_DATA_CREDIT_POSITION)
#define  PCIE_PL_VC3_NP_HEADER_CREDIT_POSITION 12
#define  PCIE_PL_VC3_NP_HEADER_CREDIT_MASK       (0xff << PCIE_PL_VC3_NP_HEADER_CREDIT_POSITION)
#define  PCIE_PL_RESERVED3_VC3_POSITION 20
#define  PCIE_PL_RESERVED3_VC3_MASK (0x1 << PCIE_PL_RESERVED3_VC3_POSITION)
#define  PCIE_PL_VC3_NP_TLP_Q_MODE_POSITION 21
#define  PCIE_PL_VC3_NP_TLP_Q_MODE_MASK       (0x7 << PCIE_PL_VC3_NP_TLP_Q_MODE_POSITION)
#define  PCIE_PL_VC3_NP_HDR_SCALE_POSITION 24
#define  PCIE_PL_VC3_NP_HDR_SCALE_MASK  (0x3 << PCIE_PL_VC0_NP_HDR_SCALE_POSITION)
#define  PCIE_PL_VC3_NP_DATA_SCALE_POSITION 26
#define  PCIE_PL_VC3_NP_DATA_SCALE_MASK (0x3 << PCIE_PL_VC0_NP_DATA_SCALE_POSITION)
#define  PCIE_PL_RESERVED4_VC3_POSITION 28
#define  PCIE_PL_RESERVED4_VC3_MASK (0xf << PCIE_PL_RESERVED4_VC1_POSITION)

#define  PCIE_PL_VC3_CPL_DATA_CREDIT_POSITION 0
#define  PCIE_PL_VC3_CPL_DATA_CREDIT_MASK      (0xfff << PCIE_PL_VC3_CPL_DATA_CREDIT_POSITION)
#define  PCIE_PL_VC3_CPL_HEADER_CREDIT_POSITION 12
#define  PCIE_PL_VC3_CPL_HEADER_CREDIT_MASK       (0xff << PCIE_PL_VC3_CPL_HEADER_CREDIT_POSITION)
#define  PCIE_PL_RESERVED5_VC3_POSITION 20
#define  PCIE_PL_RESERVED5_VC3_MASK (0x1 << PCIE_PL_RESERVED5_VC3_POSITION)
#define  PCIE_PL_VC3_CPL_TLP_Q_MODE_POSITION 21
#define  PCIE_PL_VC3_CPL_TLP_Q_MODE_MASK       (0x7 << PCIE_PL_VC3_CPL_TLP_Q_MODE_POSITION)
#define  PCIE_PL_VC3_CPL_HDR_SCALE_POSITION 24
#define  PCIE_PL_VC3_CPL_HDR_SCALE_MASK  (0x3 << PCIE_PL_VC0_CPL_HDR_SCALE_POSITION)
#define  PCIE_PL_VC3_CPL_DATA_SCALE_POSITION 26
#define  PCIE_PL_VC3_CPL_DATA_SCALE_MASK (0x3 << PCIE_PL_VC0_CPL_DATA_SCALE_POSITION)
#define  PCIE_PL_RESERVED6_VC3_POSITION 28
#define  PCIE_PL_RESERVED6_VC3_MASK (0xf << PCIE_PL_RESERVED6_VC1_POSITION)

#define  PCIE_PL_VC4_P_DATA_CREDIT_POSITION 0
#define  PCIE_PL_VC4_P_DATA_CREDIT_MASK (0xfff << PCIE_PL_VC4_P_DATA_CREDIT_POSITION)
#define  PCIE_PL_VC4_P_HEADER_CREDIT_POSITION 12
#define  PCIE_PL_VC4_P_HEADER_CREDIT_MASK (0xff << PCIE_PL_VC4_P_HEADER_CREDIT_POSITION)
#define  PCIE_PL_RESERVED0_VC4_POSITION 20
#define  PCIE_PL_RESERVED0_VC4_MASK (0x1 << PCIE_PL_RESERVED0_VC4_POSITION)
#define  PCIE_PL_VC4_P_TLP_Q_MODE_POSITION 21
#define  PCIE_PL_VC4_P_TLP_Q_MODE_MASK       (0x7 << PCIE_PL_VC4_P_TLP_Q_MODE_POSITION)
#define  PCIE_PL_VC4_P_HDR_SCALE_POSITION 24
#define  PCIE_PL_VC4_P_HDR_SCALE_MASK  (0x3 << PCIE_PL_VC0_P_HDR_SCALE_POSITION)
#define  PCIE_PL_VC4_P_DATA_SCALE_POSITION 26
#define  PCIE_PL_VC4_P_DATA_SCALE_MASK (0x3 << PCIE_PL_VC0_P_DATA_SCALE_POSITION)
#define  PCIE_PL_RESERVED1_VC4_POSITION 28
#define  PCIE_PL_RESERVED1_VC4_MASK  (0x3 << PCIE_PL_RESERVED1_VC4_POSITION)
#define  PCIE_PL_TLP_TYPE_ORDERING_VC4_POSITION 30
#define  PCIE_PL_TLP_TYPE_ORDERING_VC4_MASK       (0x1 << PCIE_PL_TLP_TYPE_ORDERING_VC4_POSITION)
#define  PCIE_PL_RESERVED2_VC4_POSITION 31
#define  PCIE_PL_RESERVED2_VC4_MASK (0x1 << PCIE_PL_RESERVED2_VC4_POSITION)

#define  PCIE_PL_VC4_NP_DATA_CREDIT_POSITION 0
#define  PCIE_PL_VC4_NP_DATA_CREDIT_MASK      (0xfff << PCIE_PL_VC4_NP_DATA_CREDIT_POSITION)
#define  PCIE_PL_VC4_NP_HEADER_CREDIT_POSITION 12
#define  PCIE_PL_VC4_NP_HEADER_CREDIT_MASK       (0xff << PCIE_PL_VC4_NP_HEADER_CREDIT_POSITION)
#define  PCIE_PL_RESERVED3_VC4_POSITION 20
#define  PCIE_PL_RESERVED3_VC4_MASK (0x1 << PCIE_PL_RESERVED3_VC4_POSITION)
#define  PCIE_PL_VC4_NP_TLP_Q_MODE_POSITION 21
#define  PCIE_PL_VC4_NP_TLP_Q_MODE_MASK       (0x7 << PCIE_PL_VC4_NP_TLP_Q_MODE_POSITION)
#define  PCIE_PL_VC4_NP_HDR_SCALE_POSITION 24
#define  PCIE_PL_VC4_NP_HDR_SCALE_MASK  (0x3 << PCIE_PL_VC0_NP_HDR_SCALE_POSITION)
#define  PCIE_PL_VC4_NP_DATA_SCALE_POSITION 26
#define  PCIE_PL_VC4_NP_DATA_SCALE_MASK (0x3 << PCIE_PL_VC0_NP_DATA_SCALE_POSITION)
#define  PCIE_PL_RESERVED4_VC4_POSITION 28
#define  PCIE_PL_RESERVED4_VC4_MASK (0xf << PCIE_PL_RESERVED4_VC1_POSITION)

#define  PCIE_PL_VC4_CPL_DATA_CREDIT_POSITION 0
#define  PCIE_PL_VC4_CPL_DATA_CREDIT_MASK      (0xfff << PCIE_PL_VC4_CPL_DATA_CREDIT_POSITION)
#define  PCIE_PL_VC4_CPL_HEADER_CREDIT_POSITION 12
#define  PCIE_PL_VC4_CPL_HEADER_CREDIT_MASK       (0xff << PCIE_PL_VC4_CPL_HEADER_CREDIT_POSITION)
#define  PCIE_PL_RESERVED5_VC4_POSITION 20
#define  PCIE_PL_RESERVED5_VC4_MASK (0x1 << PCIE_PL_RESERVED5_VC4_POSITION)
#define  PCIE_PL_VC4_CPL_TLP_Q_MODE_POSITION 21
#define  PCIE_PL_VC4_CPL_TLP_Q_MODE_MASK       (0x7 << PCIE_PL_VC4_CPL_TLP_Q_MODE_POSITION)
#define  PCIE_PL_VC4_CPL_HDR_SCALE_POSITION 24
#define  PCIE_PL_VC4_CPL_HDR_SCALE_MASK  (0x3 << PCIE_PL_VC0_CPL_HDR_SCALE_POSITION)
#define  PCIE_PL_VC4_CPL_DATA_SCALE_POSITION 26
#define  PCIE_PL_VC4_CPL_DATA_SCALE_MASK (0x3 << PCIE_PL_VC0_CPL_DATA_SCALE_POSITION)
#define  PCIE_PL_RESERVED6_VC4_POSITION 28
#define  PCIE_PL_RESERVED6_VC4_MASK (0xf << PCIE_PL_RESERVED6_VC1_POSITION)

#define  PCIE_PL_VC5_P_DATA_CREDIT_POSITION 0
#define  PCIE_PL_VC5_P_DATA_CREDIT_MASK (0xfff << PCIE_PL_VC5_P_DATA_CREDIT_POSITION)
#define  PCIE_PL_VC5_P_HEADER_CREDIT_POSITION 12
#define  PCIE_PL_VC5_P_HEADER_CREDIT_MASK (0xff << PCIE_PL_VC5_P_HEADER_CREDIT_POSITION)
#define  PCIE_PL_RESERVED0_VC5_POSITION 20
#define  PCIE_PL_RESERVED0_VC5_MASK (0x1 << PCIE_PL_RESERVED0_VC5_POSITION)
#define  PCIE_PL_VC5_P_TLP_Q_MODE_POSITION 21
#define  PCIE_PL_VC5_P_TLP_Q_MODE_MASK       (0x7 << PCIE_PL_VC5_P_TLP_Q_MODE_POSITION)
#define  PCIE_PL_VC5_P_HDR_SCALE_POSITION 24
#define  PCIE_PL_VC5_P_HDR_SCALE_MASK  (0x3 << PCIE_PL_VC0_P_HDR_SCALE_POSITION)
#define  PCIE_PL_VC5_P_DATA_SCALE_POSITION 26
#define  PCIE_PL_VC5_P_DATA_SCALE_MASK (0x3 << PCIE_PL_VC0_P_DATA_SCALE_POSITION)
#define  PCIE_PL_RESERVED1_VC5_POSITION 28
#define  PCIE_PL_RESERVED1_VC5_MASK  (0x3 << PCIE_PL_RESERVED1_VC5_POSITION)
#define  PCIE_PL_TLP_TYPE_ORDERING_VC5_POSITION 30
#define  PCIE_PL_TLP_TYPE_ORDERING_VC5_MASK       (0x1 << PCIE_PL_TLP_TYPE_ORDERING_VC5_POSITION)
#define  PCIE_PL_RESERVED2_VC5_POSITION 31
#define  PCIE_PL_RESERVED2_VC5_MASK (0x1 << PCIE_PL_RESERVED2_VC5_POSITION)

#define  PCIE_PL_VC5_NP_DATA_CREDIT_POSITION 0
#define  PCIE_PL_VC5_NP_DATA_CREDIT_MASK      (0xfff << PCIE_PL_VC5_NP_DATA_CREDIT_POSITION)
#define  PCIE_PL_VC5_NP_HEADER_CREDIT_POSITION 12
#define  PCIE_PL_VC5_NP_HEADER_CREDIT_MASK       (0xff << PCIE_PL_VC5_NP_HEADER_CREDIT_POSITION)
#define  PCIE_PL_RESERVED3_VC5_POSITION 20
#define  PCIE_PL_RESERVED3_VC5_MASK (0x1 << PCIE_PL_RESERVED3_VC5_POSITION)
#define  PCIE_PL_VC5_NP_TLP_Q_MODE_POSITION 21
#define  PCIE_PL_VC5_NP_TLP_Q_MODE_MASK       (0x7 << PCIE_PL_VC5_NP_TLP_Q_MODE_POSITION)
#define  PCIE_PL_VC5_NP_HDR_SCALE_POSITION 24
#define  PCIE_PL_VC5_NP_HDR_SCALE_MASK  (0x3 << PCIE_PL_VC0_NP_HDR_SCALE_POSITION)
#define  PCIE_PL_VC5_NP_DATA_SCALE_POSITION 26
#define  PCIE_PL_VC5_NP_DATA_SCALE_MASK (0x3 << PCIE_PL_VC0_NP_DATA_SCALE_POSITION)
#define  PCIE_PL_RESERVED4_VC5_POSITION 28
#define  PCIE_PL_RESERVED4_VC5_MASK (0xf << PCIE_PL_RESERVED4_VC1_POSITION)

#define  PCIE_PL_VC5_CPL_DATA_CREDIT_POSITION 0
#define  PCIE_PL_VC5_CPL_DATA_CREDIT_MASK     (0xfff << PCIE_PL_VC5_CPL_DATA_CREDIT_POSITION)
#define  PCIE_PL_VC5_CPL_HEADER_CREDIT_POSITION 12
#define  PCIE_PL_VC5_CPL_HEADER_CREDIT_MASK       (0xff << PCIE_PL_VC5_CPL_HEADER_CREDIT_POSITION)
#define  PCIE_PL_RESERVED5_VC5_POSITION 20
#define  PCIE_PL_RESERVED5_VC5_MASK (0x1 << PCIE_PL_RESERVED5_VC5_POSITION)
#define  PCIE_PL_VC5_CPL_TLP_Q_MODE_POSITION 21
#define  PCIE_PL_VC5_CPL_TLP_Q_MODE_MASK       (0x7 << PCIE_PL_VC5_CPL_TLP_Q_MODE_POSITION)
#define  PCIE_PL_VC5_CPL_HDR_SCALE_POSITION 24
#define  PCIE_PL_VC5_CPL_HDR_SCALE_MASK  (0x3 << PCIE_PL_VC0_CPL_HDR_SCALE_POSITION)
#define  PCIE_PL_VC5_CPL_DATA_SCALE_POSITION 26
#define  PCIE_PL_VC5_CPL_DATA_SCALE_MASK (0x3 << PCIE_PL_VC0_CPL_DATA_SCALE_POSITION)
#define  PCIE_PL_RESERVED6_VC5_POSITION 28
#define  PCIE_PL_RESERVED6_VC5_MASK (0xf << PCIE_PL_RESERVED6_VC1_POSITION)

#define  PCIE_PL_VC6_P_DATA_CREDIT_POSITION 0
#define  PCIE_PL_VC6_P_DATA_CREDIT_MASK (0xfff << PCIE_PL_VC6_P_DATA_CREDIT_POSITION)
#define  PCIE_PL_VC6_P_HEADER_CREDIT_POSITION 12
#define  PCIE_PL_VC6_P_HEADER_CREDIT_MASK (0xff << PCIE_PL_VC6_P_HEADER_CREDIT_POSITION)
#define  PCIE_PL_RESERVED0_VC6_POSITION 20
#define  PCIE_PL_RESERVED0_VC6_MASK (0x1 << PCIE_PL_RESERVED0_VC6_POSITION)
#define  PCIE_PL_VC6_P_TLP_Q_MODE_POSITION 21
#define  PCIE_PL_VC6_P_TLP_Q_MODE_MASK      (0x7 << PCIE_PL_VC6_P_TLP_Q_MODE_POSITION)
#define  PCIE_PL_VC6_P_HDR_SCALE_POSITION 24
#define  PCIE_PL_VC6_P_HDR_SCALE_MASK  (0x3 << PCIE_PL_VC0_P_HDR_SCALE_POSITION)
#define  PCIE_PL_VC6_P_DATA_SCALE_POSITION 26
#define  PCIE_PL_VC6_P_DATA_SCALE_MASK (0x3 << PCIE_PL_VC0_P_DATA_SCALE_POSITION)
#define  PCIE_PL_RESERVED1_VC6_POSITION 28
#define  PCIE_PL_RESERVED1_VC6_MASK  (0x3 << PCIE_PL_RESERVED1_VC6_POSITION)
#define  PCIE_PL_TLP_TYPE_ORDERING_VC6_POSITION 30
#define  PCIE_PL_TLP_TYPE_ORDERING_VC6_MASK       (0x1 << PCIE_PL_TLP_TYPE_ORDERING_VC6_POSITION)
#define  PCIE_PL_RESERVED2_VC6_POSITION 31
#define  PCIE_PL_RESERVED2_VC6_MASK (0x1 << PCIE_PL_RESERVED2_VC6_POSITION)

#define  PCIE_PL_VC6_NP_DATA_CREDIT_POSITION 0
#define  PCIE_PL_VC6_NP_DATA_CREDIT_MASK      (0xfff << PCIE_PL_VC6_NP_DATA_CREDIT_POSITION)
#define  PCIE_PL_VC6_NP_HEADER_CREDIT_POSITION 12
#define  PCIE_PL_VC6_NP_HEADER_CREDIT_MASK       (0xff << PCIE_PL_VC6_NP_HEADER_CREDIT_POSITION)
#define  PCIE_PL_RESERVED3_VC6_POSITION 20
#define  PCIE_PL_RESERVED3_VC6_MASK (0x1 << PCIE_PL_RESERVED3_VC6_POSITION)
#define  PCIE_PL_VC6_NP_TLP_Q_MODE_POSITION 21
#define  PCIE_PL_VC6_NP_TLP_Q_MODE_MASK       (0x7 << PCIE_PL_VC6_NP_TLP_Q_MODE_POSITION)
#define  PCIE_PL_VC6_NP_HDR_SCALE_POSITION 24
#define  PCIE_PL_VC6_NP_HDR_SCALE_MASK  (0x3 << PCIE_PL_VC0_NP_HDR_SCALE_POSITION)
#define  PCIE_PL_VC6_NP_DATA_SCALE_POSITION 26
#define  PCIE_PL_VC6_NP_DATA_SCALE_MASK (0x3 << PCIE_PL_VC0_NP_DATA_SCALE_POSITION)
#define  PCIE_PL_RESERVED4_VC6_POSITION 28
#define  PCIE_PL_RESERVED4_VC6_MASK (0xf << PCIE_PL_RESERVED4_VC1_POSITION)

#define  PCIE_PL_VC6_CPL_DATA_CREDIT_POSITION 0
#define  PCIE_PL_VC6_CPL_DATA_CREDIT_MASK      (0xfff << PCIE_PL_VC6_CPL_DATA_CREDIT_POSITION)
#define  PCIE_PL_VC6_CPL_HEADER_CREDIT_POSITION 12
#define  PCIE_PL_VC6_CPL_HEADER_CREDIT_MASK       (0xff << PCIE_PL_VC6_CPL_HEADER_CREDIT_POSITION)
#define  PCIE_PL_RESERVED5_VC6_POSITION 20
#define  PCIE_PL_RESERVED5_VC6_MASK (0x1 << PCIE_PL_RESERVED5_VC6_POSITION)
#define  PCIE_PL_VC6_CPL_TLP_Q_MODE_POSITION 21
#define  PCIE_PL_VC6_CPL_TLP_Q_MODE_MASK       (0x7 << PCIE_PL_VC6_CPL_TLP_Q_MODE_POSITION)
#define  PCIE_PL_VC6_CPL_HDR_SCALE_POSITION 24
#define  PCIE_PL_VC6_CPL_HDR_SCALE_MASK  (0x3 << PCIE_PL_VC0_CPL_HDR_SCALE_POSITION)
#define  PCIE_PL_VC6_CPL_DATA_SCALE_POSITION 26
#define  PCIE_PL_VC6_CPL_DATA_SCALE_MASK (0x3 << PCIE_PL_VC0_CPL_DATA_SCALE_POSITION)
#define  PCIE_PL_RESERVED6_VC6_POSITION 28
#define  PCIE_PL_RESERVED6_VC6_MASK (0xf << PCIE_PL_RESERVED6_VC1_POSITION)

#define  PCIE_PL_VC7_P_DATA_CREDIT_POSITION 0
#define  PCIE_PL_VC7_P_DATA_CREDIT_MASK (0xfff << PCIE_PL_VC7_P_DATA_CREDIT_POSITION)
#define  PCIE_PL_VC7_P_HEADER_CREDIT_POSITION 12
#define  PCIE_PL_VC7_P_HEADER_CREDIT_MASK (0xff << PCIE_PL_VC7_P_HEADER_CREDIT_POSITION)
#define  PCIE_PL_RESERVED0_VC7_POSITION 20
#define  PCIE_PL_RESERVED0_VC7_MASK (0x1 << PCIE_PL_RESERVED0_VC7_POSITION)
#define  PCIE_PL_VC7_P_TLP_Q_MODE_POSITION 21
#define  PCIE_PL_VC7_P_TLP_Q_MODE_MASK       (0x7 << PCIE_PL_VC7_P_TLP_Q_MODE_POSITION)
#define  PCIE_PL_VC7_P_HDR_SCALE_POSITION 24
#define  PCIE_PL_VC7_P_HDR_SCALE_MASK  (0x3 << PCIE_PL_VC0_P_HDR_SCALE_POSITION)
#define  PCIE_PL_VC7_P_DATA_SCALE_POSITION 26
#define  PCIE_PL_VC7_P_DATA_SCALE_MASK (0x3 << PCIE_PL_VC0_P_DATA_SCALE_POSITION)
#define  PCIE_PL_RESERVED1_VC7_POSITION 28
#define  PCIE_PL_RESERVED1_VC7_MASK  (0x3 << PCIE_PL_RESERVED1_VC7_POSITION)
#define  PCIE_PL_TLP_TYPE_ORDERING_VC7_POSITION 30
#define  PCIE_PL_TLP_TYPE_ORDERING_VC7_MASK       (0x1 << PCIE_PL_TLP_TYPE_ORDERING_VC7_POSITION)
#define  PCIE_PL_RESERVED2_VC7_POSITION 31
#define  PCIE_PL_RESERVED2_VC7_MASK (0x1 << PCIE_PL_RESERVED2_VC7_POSITION)

#define  PCIE_PL_VC7_NP_DATA_CREDIT_POSITION 0
#define  PCIE_PL_VC7_NP_DATA_CREDIT_MASK      (0xfff << PCIE_PL_VC7_NP_DATA_CREDIT_POSITION)
#define  PCIE_PL_VC7_NP_HEADER_CREDIT_POSITION 12
#define  PCIE_PL_VC7_NP_HEADER_CREDIT_MASK       (0xff << PCIE_PL_VC7_NP_HEADER_CREDIT_POSITION)
#define  PCIE_PL_RESERVED3_VC7_POSITION 20
#define  PCIE_PL_RESERVED3_VC7_MASK (0x1 << PCIE_PL_RESERVED3_VC7_POSITION)
#define  PCIE_PL_VC7_NP_TLP_Q_MODE_POSITION 21
#define  PCIE_PL_VC7_NP_TLP_Q_MODE_MASK       (0x7 << PCIE_PL_VC7_NP_TLP_Q_MODE_POSITION)
#define  PCIE_PL_VC7_NP_HDR_SCALE_POSITION 24
#define  PCIE_PL_VC7_NP_HDR_SCALE_MASK  (0x3 << PCIE_PL_VC0_NP_HDR_SCALE_POSITION)
#define  PCIE_PL_VC7_NP_DATA_SCALE_POSITION 26
#define  PCIE_PL_VC7_NP_DATA_SCALE_MASK (0x3 << PCIE_PL_VC0_NP_DATA_SCALE_POSITION)
#define  PCIE_PL_RESERVED4_VC7_POSITION 28
#define  PCIE_PL_RESERVED4_VC7_MASK (0xf << PCIE_PL_RESERVED4_VC1_POSITION)

#define  PCIE_PL_VC7_CPL_DATA_CREDIT_POSITION 0
#define  PCIE_PL_VC7_CPL_DATA_CREDIT_MASK      (0xfff << PCIE_PL_VC7_CPL_DATA_CREDIT_POSITION)
#define  PCIE_PL_VC7_CPL_HEADER_CREDIT_POSITION 12
#define  PCIE_PL_VC7_CPL_HEADER_CREDIT_MASK       (0xff << PCIE_PL_VC7_CPL_HEADER_CREDIT_POSITION)
#define  PCIE_PL_RESERVED5_VC7_POSITION 20
#define  PCIE_PL_RESERVED5_VC7_MASK (0x1 << PCIE_PL_RESERVED5_VC7_POSITION)
#define  PCIE_PL_VC7_CPL_TLP_Q_MODE_POSITION 21
#define  PCIE_PL_VC7_CPL_TLP_Q_MODE_MASK      (0x7 << PCIE_PL_VC7_CPL_TLP_Q_MODE_POSITION)
#define  PCIE_PL_VC7_CPL_HDR_SCALE_POSITION 24
#define  PCIE_PL_VC7_CPL_HDR_SCALE_MASK  (0x3 << PCIE_PL_VC0_CPL_HDR_SCALE_POSITION)
#define  PCIE_PL_VC7_CPL_DATA_SCALE_POSITION 26
#define  PCIE_PL_VC7_CPL_DATA_SCALE_MASK (0x3 << PCIE_PL_VC0_CPL_DATA_SCALE_POSITION)
#define  PCIE_PL_RESERVED6_VC7_POSITION 28
#define  PCIE_PL_RESERVED6_VC7_MASK (0xf << PCIE_PL_RESERVED6_VC1_POSITION)

#define  PCIE_PL_VC0_P_DATA_Q_DEPTH_POSITION 0
#define  PCIE_PL_VC0_P_DATA_Q_DEPTH_MASK       (0x3fff << PCIE_PL_VC0_P_DATA_Q_DEPTH_POSITION)
#define  PCIE_PL_VC0_P_HEADER_Q_DEPTH_POSITION 16
#define  PCIE_PL_VC0_P_HEADER_Q_DEPTH_MASK       (0x3ff << PCIE_PL_VC0_P_HEADER_Q_DEPTH_POSITION)

#define  PCIE_PL_VC0_NP_DATA_Q_DEPTH_POSITION 0
#define  PCIE_PL_VC0_NP_DATA_Q_DEPTH_MASK       ( 0x3fff<< PCIE_PL_VC0_NP_DATA_Q_DEPTH_POSITION)
#define  PCIE_PL_VC0_NP_HEADER_Q_DEPTH_POSITION 16
#define  PCIE_PL_VC0_NP_HEADER_Q_DEPTH_MASK       (0x3ff << PCIE_PL_VC0_NP_HEADER_Q_DEPTH_POSITION)

#define  PCIE_PL_VC0_CPL_DATA_Q_DEPTH_POSITION 0
#define  PCIE_PL_VC0_CPL_DATA_Q_DEPTH_MASK       (0x3fff << PCIE_PL_VC0_CPL_DATA_Q_DEPTH_POSITION)
#define  PCIE_PL_VC0_CPL_HEADER_Q_DEPTH_POSITION 16
#define  PCIE_PL_VC0_CPL_HEADER_Q_DEPTH_MASK       (0x3ff << PCIE_PL_VC0_CPL_HEADER_Q_DEPTH_POSITION)


#define  PCIE_PL_VC1_P_DATA_Q_DEPTH_POSITION 0
#define  PCIE_PL_VC1_P_DATA_Q_DEPTH_MASK       (0x3fff << PCIE_PL_VC1_P_DATA_Q_DEPTH_POSITION)
#define  PCIE_PL_VC1_P_HEADER_Q_DEPTH_POSITION 16
#define  PCIE_PL_VC1_P_HEADER_Q_DEPTH_MASK       (0x3ff << PCIE_PL_VC1_P_HEADER_Q_DEPTH_POSITION)

#define  PCIE_PL_VC1_NP_DATA_Q_DEPTH_POSITION 0
#define  PCIE_PL_VC1_NP_DATA_Q_DEPTH_MASK       ( 0x03fff<< PCIE_PL_VC1_NP_DATA_Q_DEPTH_POSITION)
#define  PCIE_PL_VC1_NP_HEADER_Q_DEPTH_POSITION 16
#define  PCIE_PL_VC1_NP_HEADER_Q_DEPTH_MASK       (0x3ff << PCIE_PL_VC1_NP_HEADER_Q_DEPTH_POSITION)

#define  PCIE_PL_VC1_CPL_DATA_Q_DEPTH_POSITION 0
#define  PCIE_PL_VC1_CPL_DATA_Q_DEPTH_MASK       (0x3fff << PCIE_PL_VC1_CPL_DATA_Q_DEPTH_POSITION)
#define  PCIE_PL_VC1_CPL_HEADER_Q_DEPTH_POSITION 16
#define  PCIE_PL_VC1_CPL_HEADER_Q_DEPTH_MASK       (0x3ff << PCIE_PL_VC1_CPL_HEADER_Q_DEPTH_POSITION)


#define  PCIE_PL_VC2_P_DATA_Q_DEPTH_POSITION 0
#define  PCIE_PL_VC2_P_DATA_Q_DEPTH_MASK       (0x3fff << PCIE_PL_VC2_P_DATA_Q_DEPTH_POSITION)
#define  PCIE_PL_VC2_P_HEADER_Q_DEPTH_POSITION 16
#define  PCIE_PL_VC2_P_HEADER_Q_DEPTH_MASK       (0x3ff << PCIE_PL_VC2_P_HEADER_Q_DEPTH_POSITION)

#define  PCIE_PL_VC2_NP_DATA_Q_DEPTH_POSITION 0
#define  PCIE_PL_VC2_NP_DATA_Q_DEPTH_MASK       ( 0x3fff<< PCIE_PL_VC2_NP_DATA_Q_DEPTH_POSITION)
#define  PCIE_PL_VC2_NP_HEADER_Q_DEPTH_POSITION 16
#define  PCIE_PL_VC2_NP_HEADER_Q_DEPTH_MASK       (0x3ff << PCIE_PL_VC2_NP_HEADER_Q_DEPTH_POSITION)

#define  PCIE_PL_VC2_CPL_DATA_Q_DEPTH_POSITION 0
#define  PCIE_PL_VC2_CPL_DATA_Q_DEPTH_MASK       (0x3fff << PCIE_PL_VC2_CPL_DATA_Q_DEPTH_POSITION)
#define  PCIE_PL_VC2_CPL_HEADER_Q_DEPTH_POSITION 16
#define  PCIE_PL_VC2_CPL_HEADER_Q_DEPTH_MASK       (0x3ff << PCIE_PL_VC2_CPL_HEADER_Q_DEPTH_POSITION)


#define  PCIE_PL_VC3_P_DATA_Q_DEPTH_POSITION 0
#define  PCIE_PL_VC3_P_DATA_Q_DEPTH_MASK       (0x3fff << PCIE_PL_VC3_P_DATA_Q_DEPTH_POSITION)
#define  PCIE_PL_VC3_P_HEADER_Q_DEPTH_POSITION 16
#define  PCIE_PL_VC3_P_HEADER_Q_DEPTH_MASK       (0x3ff << PCIE_PL_VC3_P_HEADER_Q_DEPTH_POSITION)

#define  PCIE_PL_VC3_NP_DATA_Q_DEPTH_POSITION 0
#define  PCIE_PL_VC3_NP_DATA_Q_DEPTH_MASK       ( 0x3fff<< PCIE_PL_VC3_NP_DATA_Q_DEPTH_POSITION)
#define  PCIE_PL_VC3_NP_HEADER_Q_DEPTH_POSITION 16
#define  PCIE_PL_VC3_NP_HEADER_Q_DEPTH_MASK       (0x3ff << PCIE_PL_VC3_NP_HEADER_Q_DEPTH_POSITION)

#define  PCIE_PL_VC3_CPL_DATA_Q_DEPTH_POSITION 0
#define  PCIE_PL_VC3_CPL_DATA_Q_DEPTH_MASK       (0x3fff << PCIE_PL_VC3_CPL_DATA_Q_DEPTH_POSITION)
#define  PCIE_PL_VC3_CPL_HEADER_Q_DEPTH_POSITION 16
#define  PCIE_PL_VC3_CPL_HEADER_Q_DEPTH_MASK       (0x3ff << PCIE_PL_VC3_CPL_HEADER_Q_DEPTH_POSITION)

#define  PCIE_PL_VC4_P_DATA_Q_DEPTH_POSITION 0
#define  PCIE_PL_VC4_P_DATA_Q_DEPTH_MASK       (0x3fff << PCIE_PL_VC4_P_DATA_Q_DEPTH_POSITION)
#define  PCIE_PL_VC4_P_HEADER_Q_DEPTH_POSITION 16
#define  PCIE_PL_VC4_P_HEADER_Q_DEPTH_MASK       (0x3ff << PCIE_PL_VC4_P_HEADER_Q_DEPTH_POSITION)

#define  PCIE_PL_VC4_NP_DATA_Q_DEPTH_POSITION 0
#define  PCIE_PL_VC4_NP_DATA_Q_DEPTH_MASK       ( 0x3fff<< PCIE_PL_VC4_NP_DATA_Q_DEPTH_POSITION)
#define  PCIE_PL_VC4_NP_HEADER_Q_DEPTH_POSITION 16
#define  PCIE_PL_VC4_NP_HEADER_Q_DEPTH_MASK       (0x3ff << PCIE_PL_VC4_NP_HEADER_Q_DEPTH_POSITION)

#define  PCIE_PL_VC4_CPL_DATA_Q_DEPTH_POSITION 0
#define  PCIE_PL_VC4_CPL_DATA_Q_DEPTH_MASK       (0x3fff << PCIE_PL_VC4_CPL_DATA_Q_DEPTH_POSITION)
#define  PCIE_PL_VC4_CPL_HEADER_Q_DEPTH_POSITION 16
#define  PCIE_PL_VC4_CPL_HEADER_Q_DEPTH_MASK       (0x3ff << PCIE_PL_VC4_CPL_HEADER_Q_DEPTH_POSITION)

#define  PCIE_PL_VC5_P_DATA_Q_DEPTH_POSITION 0
#define  PCIE_PL_VC5_P_DATA_Q_DEPTH_MASK       (0x3fff << PCIE_PL_VC5_P_DATA_Q_DEPTH_POSITION)
#define  PCIE_PL_VC5_P_HEADER_Q_DEPTH_POSITION 16
#define  PCIE_PL_VC5_P_HEADER_Q_DEPTH_MASK       (0x3ff << PCIE_PL_VC5_P_HEADER_Q_DEPTH_POSITION)

#define  PCIE_PL_VC5_NP_DATA_Q_DEPTH_POSITION 0
#define  PCIE_PL_VC5_NP_DATA_Q_DEPTH_MASK       ( 0x3fff<< PCIE_PL_VC5_NP_DATA_Q_DEPTH_POSITION)
#define  PCIE_PL_VC5_NP_HEADER_Q_DEPTH_POSITION 16
#define  PCIE_PL_VC5_NP_HEADER_Q_DEPTH_MASK       (0x3ff << PCIE_PL_VC5_NP_HEADER_Q_DEPTH_POSITION)

#define  PCIE_PL_VC5_CPL_DATA_Q_DEPTH_POSITION 0
#define  PCIE_PL_VC5_CPL_DATA_Q_DEPTH_MASK       (0x3fff << PCIE_PL_VC5_CPL_DATA_Q_DEPTH_POSITION)
#define  PCIE_PL_VC5_CPL_HEADER_Q_DEPTH_POSITION 16
#define  PCIE_PL_VC5_CPL_HEADER_Q_DEPTH_MASK       (0x3ff << PCIE_PL_VC5_CPL_HEADER_Q_DEPTH_POSITION)

#define  PCIE_PL_VC6_P_DATA_Q_DEPTH_POSITION 0
#define  PCIE_PL_VC6_P_DATA_Q_DEPTH_MASK       (0x3fff << PCIE_PL_VC6_P_DATA_Q_DEPTH_POSITION)
#define  PCIE_PL_VC6_P_HEADER_Q_DEPTH_POSITION 16
#define  PCIE_PL_VC6_P_HEADER_Q_DEPTH_MASK       (0x3ff << PCIE_PL_VC6_P_HEADER_Q_DEPTH_POSITION)

#define  PCIE_PL_VC6_NP_DATA_Q_DEPTH_POSITION 0
#define  PCIE_PL_VC6_NP_DATA_Q_DEPTH_MASK       ( 0x3fff<< PCIE_PL_VC6_NP_DATA_Q_DEPTH_POSITION)
#define  PCIE_PL_VC6_NP_HEADER_Q_DEPTH_POSITION 16
#define  PCIE_PL_VC6_NP_HEADER_Q_DEPTH_MASK       (0x3ff << PCIE_PL_VC6_NP_HEADER_Q_DEPTH_POSITION)

#define  PCIE_PL_VC6_CPL_DATA_Q_DEPTH_POSITION 0
#define  PCIE_PL_VC6_CPL_DATA_Q_DEPTH_MASK       (0x3fff << PCIE_PL_VC6_CPL_DATA_Q_DEPTH_POSITION)
#define  PCIE_PL_VC6_CPL_HEADER_Q_DEPTH_POSITION 16
#define  PCIE_PL_VC6_CPL_HEADER_Q_DEPTH_MASK       (0x3ff << PCIE_PL_VC6_CPL_HEADER_Q_DEPTH_POSITION)

#define  PCIE_PL_VC7_P_DATA_Q_DEPTH_POSITION 0
#define  PCIE_PL_VC7_P_DATA_Q_DEPTH_MASK       (0x3fff << PCIE_PL_VC7_P_DATA_Q_DEPTH_POSITION)
#define  PCIE_PL_VC7_P_HEADER_Q_DEPTH_POSITION 16
#define  PCIE_PL_VC7_P_HEADER_Q_DEPTH_MASK       (0x3ff << PCIE_PL_VC7_P_HEADER_Q_DEPTH_POSITION)

#define  PCIE_PL_VC7_NP_DATA_Q_DEPTH_POSITION 0
#define  PCIE_PL_VC7_NP_DATA_Q_DEPTH_MASK       ( 0x3fff<< PCIE_PL_VC7_NP_DATA_Q_DEPTH_POSITION)
#define  PCIE_PL_VC7_NP_HEADER_Q_DEPTH_POSITION 16
#define  PCIE_PL_VC7_NP_HEADER_Q_DEPTH_MASK       (0x3ff << PCIE_PL_VC7_NP_HEADER_Q_DEPTH_POSITION)

#define  PCIE_PL_VC7_CPL_DATA_Q_DEPTH_POSITION 0
#define  PCIE_PL_VC7_CPL_DATA_Q_DEPTH_MASK       (0x3fff << PCIE_PL_VC7_CPL_DATA_Q_DEPTH_POSITION)
#define  PCIE_PL_VC7_CPL_HEADER_Q_DEPTH_POSITION 16
#define  PCIE_PL_VC7_CPL_HEADER_Q_DEPTH_MASK       (0x3ff << PCIE_PL_VC7_CPL_HEADER_Q_DEPTH_POSITION)

#define  PCIE_PL_FAST_TRAINING_SEQ_POSITION 0
#define  PCIE_PL_FAST_TRAINING_SEQ_MASK       (0xff << PCIE_PL_FAST_TRAINING_SEQ_POSITION)

#define  PCIE_PL_NUM_OF_LANES_POSITION 8
#define  PCIE_PL_NUM_OF_LANES_MASK       (0x1f << PCIE_PL_NUM_OF_LANES_POSITION)

#define  PCIE_PL_PRE_DET_LANE_POSITION 13
#define  PCIE_PL_PRE_DET_LANE_MASK       (0x7 << PCIE_PL_PRE_DET_LANE_POSITION)

#define  PCIE_PL_AUTO_LANE_FLIP_CTRL_EN_POSITION 16
#define  PCIE_PL_AUTO_LANE_FLIP_CTRL_EN_MASK       (1 << PCIE_PL_AUTO_LANE_FLIP_CTRL_EN_POSITION)

#define  PCIE_PL_DIRECT_SPEED_CHANGE_POSITION 17
#define  PCIE_PL_DIRECT_SPEED_CHANGE_MASK       (0x1 << PCIE_PL_DIRECT_SPEED_CHANGE_POSITION)
#define  PCIE_PL_CONFIG_PHY_TX_CHANGE_POSITION 18
#define  PCIE_PL_CONFIG_PHY_TX_CHANGE_MASK       (0x1 << PCIE_PL_CONFIG_PHY_TX_CHANGE_POSITION)
#define  PCIE_PL_CONFIG_TX_COMP_RX_POSITION 19
#define  PCIE_PL_CONFIG_TX_COMP_RX_MASK       (0x1 << PCIE_PL_CONFIG_TX_COMP_RX_POSITION)
#define  PCIE_PL_SEL_DEEMPHASIS_POSITION 20
#define  PCIE_PL_SEL_DEEMPHASIS_MASK       (0x1 << PCIE_PL_SEL_DEEMPHASIS_POSITION)
#define  PCIE_PL_GEN1_EI_INFERENCE_POSITION 21
#define  PCIE_PL_GEN1_EI_INFERENCE_MASK (0x1 << PCIE_PL_GEN1_EI_INFERENCE_POSITION)

#define  PCIE_PL_PHY_STATUS_POSITION 0
#define  PCIE_PL_PHY_STATUS_MASK      (0xffff_ffff << PCIE_PL_PHY_STATUS_POSITION)

#define  PCIE_PL_PHY_CONTROL_POSITION 0
#define  PCIE_PL_PHY_CONTROL_MASK      (0xffff_ffff << PCIE_PL_PHY_CONTROL_POSITION)

#define  PCIE_PL_REMOTE_READ_REQ_SIZE_POSITION 0
#define  PCIE_PL_REMOTE_READ_REQ_SIZE_MASK       (0x7 << PCIE_PL_REMOTE_READ_REQ_SIZE_POSITION)
#define  PCIE_PL_REMOTE_MAX_BRIDGE_TAG_POSITION 8
#define  PCIE_PL_REMOTE_MAX_BRIDGE_TAG_MASK      (0xff << PCIE_PL_REMOTE_MAX_BRIDGE_TAG_POSITION)

#define  PCIE_PL_RESIZE_MASTER_RESPONSE_POSITION 0
#define  PCIE_PL_RESIZE_MASTER_RESPONSE_MASK      (0x1 << PCIE_PL_RESIZE_MASTER_RESPONSE_POSITION)

#define  PCIE_PL_MSI_CTRL_ADDR_POSITION 0
#define  PCIE_PL_MSI_CTRL_ADDR_MASK      (0xffff_ffff << PCIE_PL_MSI_CTRL_ADDR_POSITION)

#define  PCIE_PL_MSI_CTRL_UPPER_ADDR_POSITION 0
#define  PCIE_PL_MSI_CTRL_UPPER_ADDR_MASK      (0xffff_ffff << PCIE_PL_MSI_CTRL_UPPER_ADDR_POSITION)

#define  PCIE_PL_MSI_CTRL_INT_0_EN_POSITION 0
#define  PCIE_PL_MSI_CTRL_INT_0_EN_MASK       (0xffff_ffff << PCIE_PL_MSI_CTRL_INT_0_EN_POSITION)
#define  PCIE_PL_MSI_CTRL_INT_0_MASK_POSITION 0
#define  PCIE_PL_MSI_CTRL_INT_0_MASK_MASK       (0xffff_ffff << PCIE_PL_MSI_CTRL_INT_0_MASK_POSITION)
#define  PCIE_PL_MSI_CTRL_INT_0_STATUS_POSITION 0
#define  PCIE_PL_MSI_CTRL_INT_0_STATUS_MASK       (0xffff_ffff << PCIE_PL_MSI_CTRL_INT_0_STATUS_POSITION)

#define  PCIE_PL_MSI_CTRL_INT_1_EN_POSITION 0
#define  PCIE_PL_MSI_CTRL_INT_1_EN_MASK       (0xffff_ffff << PCIE_PL_MSI_CTRL_INT_1_EN_POSITION)
#define  PCIE_PL_MSI_CTRL_INT_1_MASK_POSITION 0
#define  PCIE_PL_MSI_CTRL_INT_1_MASK_MASK       (0xffff_ffff << PCIE_PL_MSI_CTRL_INT_1_MASK_POSITION)
#define  PCIE_PL_MSI_CTRL_INT_1_STATUS_POSITION 0
#define  PCIE_PL_MSI_CTRL_INT_1_STATUS_MASK       (0xffff_ffff << PCIE_PL_MSI_CTRL_INT_1_STATUS_POSITION)

#define  PCIE_PL_MSI_CTRL_INT_2_EN_POSITION 0
#define  PCIE_PL_MSI_CTRL_INT_2_EN_MASK       (0xffff_ffff << PCIE_PL_MSI_CTRL_INT_2_EN_POSITION)
#define  PCIE_PL_MSI_CTRL_INT_2_MASK_POSITION 0
#define  PCIE_PL_MSI_CTRL_INT_2_MASK_MASK       (0xffff_ffff << PCIE_PL_MSI_CTRL_INT_2_MASK_POSITION)
#define  PCIE_PL_MSI_CTRL_INT_2_STATUS_POSITION 0
#define  PCIE_PL_MSI_CTRL_INT_2_STATUS_MASK       (0xffff_ffff << PCIE_PL_MSI_CTRL_INT_2_STATUS_POSITION)

#define  PCIE_PL_MSI_CTRL_INT_3_EN_POSITION 0
#define  PCIE_PL_MSI_CTRL_INT_3_EN_MASK       (0xffff_ffff << PCIE_PL_MSI_CTRL_INT_3_EN_POSITION)
#define  PCIE_PL_MSI_CTRL_INT_3_MASK_POSITION 0
#define  PCIE_PL_MSI_CTRL_INT_3_MASK_MASK       (0xffff_ffff << PCIE_PL_MSI_CTRL_INT_3_MASK_POSITION)
#define  PCIE_PL_MSI_CTRL_INT_3_STATUS_POSITION 0
#define  PCIE_PL_MSI_CTRL_INT_3_STATUS_MASK       (0xffff_ffff << PCIE_PL_MSI_CTRL_INT_3_STATUS_POSITION)

#define  PCIE_PL_MSI_CTRL_INT_4_EN_POSITION 0
#define  PCIE_PL_MSI_CTRL_INT_4_EN_MASK       (0xffff_ffff << PCIE_PL_MSI_CTRL_INT_4_EN_POSITION)
#define  PCIE_PL_MSI_CTRL_INT_4_MASK_POSITION 0
#define  PCIE_PL_MSI_CTRL_INT_4_MASK_MASK       (0xffff_ffff << PCIE_PL_MSI_CTRL_INT_4_MASK_POSITION)
#define  PCIE_PL_MSI_CTRL_INT_4_STATUS_POSITION 0
#define  PCIE_PL_MSI_CTRL_INT_4_STATUS_MASK       (0xffff_ffff << PCIE_PL_MSI_CTRL_INT_4_STATUS_POSITION)

#define  PCIE_PL_MSI_CTRL_INT_5_EN_POSITION 0
#define  PCIE_PL_MSI_CTRL_INT_5_EN_MASK       (0xffff_ffff << PCIE_PL_MSI_CTRL_INT_5_EN_POSITION)
#define  PCIE_PL_MSI_CTRL_INT_5_MASK_POSITION 0
#define  PCIE_PL_MSI_CTRL_INT_5_MASK_MASK       (0xffff_ffff << PCIE_PL_MSI_CTRL_INT_5_MASK_POSITION)
#define  PCIE_PL_MSI_CTRL_INT_5_STATUS_POSITION 0
#define  PCIE_PL_MSI_CTRL_INT_5_STATUS_MASK       (0xffff_ffff << PCIE_PL_MSI_CTRL_INT_5_STATUS_POSITION)

#define  PCIE_PL_MSI_CTRL_INT_6_EN_POSITION 0
#define  PCIE_PL_MSI_CTRL_INT_6_EN_MASK       (0xffff_ffff << PCIE_PL_MSI_CTRL_INT_6_EN_POSITION)
#define  PCIE_PL_MSI_CTRL_INT_6_MASK_POSITION 0
#define  PCIE_PL_MSI_CTRL_INT_6_MASK_MASK       (0xffff_ffff << PCIE_PL_MSI_CTRL_INT_6_MASK_POSITION)
#define  PCIE_PL_MSI_CTRL_INT_6_STATUS_POSITION 0
#define  PCIE_PL_MSI_CTRL_INT_6_STATUS_MASK       (0xffff_ffff << PCIE_PL_MSI_CTRL_INT_6_STATUS_POSITION)

#define  PCIE_PL_MSI_CTRL_INT_7_EN_POSITION 0
#define  PCIE_PL_MSI_CTRL_INT_7_EN_MASK       (0xffff_ffff << PCIE_PL_MSI_CTRL_INT_7_EN_POSITION)
#define  PCIE_PL_MSI_CTRL_INT_7_MASK_POSITION 0
#define  PCIE_PL_MSI_CTRL_INT_7_MASK_MASK       (0xffff_ffff << PCIE_PL_MSI_CTRL_INT_7_MASK_POSITION)
#define  PCIE_PL_MSI_CTRL_INT_7_STATUS_POSITION 0
#define  PCIE_PL_MSI_CTRL_INT_7_STATUS_MASK       (0xffff_ffff << PCIE_PL_MSI_CTRL_INT_7_STATUS_POSITION)

#define  PCIE_PL_MSI_GPIO_REG_POSITION 0
#define  PCIE_PL_MSI_GPIO_REG_MASK      (0xffff_ffff << PCIE_PL_MSI_GPIO_REG_POSITION)

#define  PCIE_PL_GEN3_ZRXDC_NONCOMPL_POSITION 0
#define  PCIE_PL_GEN3_ZRXDC_NONCOMPL_MASK      (0x1 << PCIE_PL_GEN3_ZRXDC_NONCOMPL_POSITION)
#define  PCIE_PL_DISABLE_SCRAMBLER_GEN_3_POSITION 8
#define  PCIE_PL_DISABLE_SCRAMBLER_GEN_3_MASK      (0x1 << PCIE_PL_DISABLE_SCRAMBLER_GEN_3_POSITION)
#define  PCIE_PL_EQ_PHASE_2_3_POSITION 9
#define  PCIE_PL_EQ_PHASE_2_3_MASK      (0x1 << PCIE_PL_EQ_PHASE_2_3_POSITION)
#define  PCIE_PL_EQ_EIEOS_CNT_POSITION 10
#define  PCIE_PL_EQ_EIEOS_CNT_MASK       (0x1 << PCIE_PL_EQ_EIEOS_CNT_POSITION)
#define  PCIE_PL_EQ_REDO_POSITION 11
#define  PCIE_PL_EQ_REDO_MASK       (0x1 << PCIE_PL_EQ_REDO_POSITION)
#define  PCIE_PL_RXEQ_PH01_EN_POSITION 12
#define  PCIE_PL_RXEQ_PH01_EN_MASK       (0x1 << PCIE_PL_RXEQ_PH01_EN_POSITION)
#define  PCIE_PL_RXEQ_RGRDLESS_RXTS_POSITION 13
#define  PCIE_PL_RXEQ_RGRDLESS_RXTS_MASK       (0x1 << PCIE_PL_RXEQ_RGRDLESS_RXTS_POSITION)
#define  PCIE_PL_GEN3_EQUALIZATION_DISABLE_POSITION 16
#define  PCIE_PL_GEN3_EQUALIZATION_DISABLE_MASK (0x1 << PCIE_PL_GEN3_EQUALIZATION_DISABLE_POSITION)
#define  PCIE_PL_GEN3_DLLP_XMT_DELAY_DISABLE_POSITION 17
#define  PCIE_PL_GEN3_DLLP_XMT_DELAY_DISABLE_MASK (0x1 << PCIE_PL_GEN3_DLLP_XMT_DELAY_DISABLE_POSITION)
#define  PCIE_PL_GEN3_DC_BALANCE_DISABLE_POSITION 18
#define  PCIE_PL_GEN3_DC_BALANCE_DISABLE_MASK (0x1 << PCIE_PL_GEN3_DC_BALANCE_DISABLE_POSITION)
#define  PCIE_PL_GEN3_EQ_INVREQ_EVAL_DIFF_DISABLE_POSITION 23
#define  PCIE_PL_GEN3_EQ_INVREQ_EVAL_DIFF_DISABLE_MASK (0x1 << PCIE_PL_GEN3_EQ_INVREQ_EVAL_DIFF_DISABLE_POSITION)

#define PCIE_PL_RE_EQ_REQUEST_ENABLE_POSITION        19
#define PCIE_PL_RE_EQ_REQUEST_ENABLE_MASK            (0x1 << PCIE_PL_RE_EQ_REQUEST_ENABLE_POSITION)
#define PCIE_PL_USP_EQ_REQ_INT_ENABLE_POSITION       20
#define PCIE_PL_USP_EQ_REQ_INT_ENABLE_MASK           (0x1 << PCIE_PL_USP_EQ_REQ_INT_ENABLE_POSITION)
#define PCIE_PL_AUTO_EQ_DISABLE_POSITION             21
#define PCIE_PL_AUTO_EQ_DISABLE_MASK                 (0x1 << PCIE_PL_AUTO_EQ_DISABLE_POSITION)
#define PCIE_PL_USP_SEND_8GT_EQ_TS2_DISABLE_POSITION 22
#define PCIE_PL_USP_SEND_8GT_EQ_TS2_DISABLE_MASK     (0x1 << PCIE_PL_USP_SEND_8GT_EQ_TS2_DISABLE_POSITION)
#define PCIE_PL_RATE_SHADOW_SEL_POSITION             24
#define PCIE_PL_RATE_SHADOW_SEL_MASK                 (0x3 << PCIE_PL_RATE_SHADOW_SEL_POSITION)

#define PCIE_PL_GEN3_EQ_LOCAL_LF_POSITION 0
#define PCIE_PL_GEN3_EQ_LOCAL_LF_MASK   (0x3F << PCIE_PL_GEN3_EQ_LOCAL_LF_POSITION  )

#define PCIE_PL_GEN3_EQ_LOCAL_FS_POSITION 6
#define PCIE_PL_GEN3_EQ_LOCAL_FS_MASK   (0x3F << PCIE_PL_GEN3_EQ_LOCAL_FS_POSITION  )

#define PCIE_PL_GEN3_EQ_POST_CURSOR_PSET_POSITION 12
#define PCIE_PL_GEN3_EQ_POST_CURSOR_PSET_MASK   (0x3F << PCIE_PL_GEN3_EQ_POST_CURSOR_PSET_POSITION  )

#define PCIE_PL_GEN3_EQ_CURSOR_PSET_POSITION 6
#define PCIE_PL_GEN3_EQ_CURSOR_PSET_MASK   (0x3F << PCIE_PL_GEN3_EQ_CURSOR_PSET_POSITION  )

#define PCIE_PL_GEN3_EQ_PRE_CURSOR_PSET_POSITION 0
#define PCIE_PL_GEN3_EQ_PRE_CURSOR_PSET_MASK   (0x3F << PCIE_PL_GEN3_EQ_PRE_CURSOR_PSET_POSITION  )

#define PCIE_PL_GEN3_EQ_PSET_INDEX_POSITION 0
#define PCIE_PL_GEN3_EQ_PSET_INDEX_MASK   (0xf << PCIE_PL_GEN3_EQ_PSET_INDEX_POSITION  )

#define PCIE_PL_GEN3_EQ_FB_MODE_POSITION 0
#define PCIE_PL_GEN3_EQ_FB_MODE_MASK   (0xf << PCIE_PL_GEN3_EQ_FB_MODE_POSITION  )

#define PCIE_PL_GEN3_EQ_PHASE23_EXIT_MODE_POSITION 4
#define PCIE_PL_GEN3_EQ_PHASE23_EXIT_MODE_MASK   (0x1 << PCIE_PL_GEN3_EQ_PHASE23_EXIT_MODE_POSITION  )

#define PCIE_PL_GEN3_EQ_EVAL_2MS_DISABLE_POSITION 5
#define PCIE_PL_GEN3_EQ_EVAL_2MS_DISABLE_MASK (0x1 << PCIE_PL_GEN3_EQ_EVAL_2MS_DISABLE_POSITION)

#define PCIE_PL_GEN3_LOWER_RATE_EQ_REDO_ENABLE_POSITION 6
#define PCIE_PL_GEN3_LOWER_RATE_EQ_REDO_ENABLE_MASK (0x1 << PCIE_PL_GEN3_LOWER_RATE_EQ_REDO_ENABLE_POSITION)

#define PCIE_PL_GEN3_EQ_PSET_REQ_VEC_POSITION 8
#define PCIE_PL_GEN3_EQ_PSET_REQ_VEC_MASK   (0xffff << PCIE_PL_GEN3_EQ_PSET_REQ_VEC_POSITION  )

#define PCIE_PL_GEN3_EQ_FOM_INC_INITIAL_EVAL_POSITION 24
#define PCIE_PL_GEN3_EQ_FOM_INC_INITIAL_EVAL_MASK   (0x1 << PCIE_PL_GEN3_EQ_FOM_INC_INITIAL_EVAL_POSITION  )

#define PCIE_PL_GEN3_EQ_PSET_REQ_AS_COEF_POSITION  25
#define PCIE_PL_GEN3_EQ_PSET_REQ_AS_COEF_MASK (0x1 << PCIE_PL_GEN3_EQ_PSET_REQ_AS_COEF_POSITION)

#define PCIE_PL_GEN3_REQ_SEND_CONSEC_EIEOS_FOR_PSET_MAP_POSITION 26
#define PCIE_PL_GEN3_REQ_SEND_CONSEC_EIEOS_FOR_PSET_MAP_MASK (0x1 <<PCIE_PL_GEN3_REQ_SEND_CONSEC_EIEOS_FOR_PSET_MAP_POSITION)

//#define PCIE_PL_GEN3_EQ_FMDC_INIT_PSET_REQ_POSITION 0
//#define PCIE_PL_GEN3_EQ_FMDC_INIT_PSET_REQ_MASK   (0x1 << PCIE_PL_GEN3_EQ_FMDC_INIT_PSET_REQ_POSITION  )

//#define PCIE_PL_GEN3_EQ_FMDC_INIT_PSET_VAL_POSITION 1
//#define PCIE_PL_GEN3_EQ_FMDC_INIT_PSET_VAL_MASK   (0xf << PCIE_PL_GEN3_EQ_FMDC_INIT_PSET_VAL_POSITION  )

#define PCIE_PL_GEN3_EQ_FMDC_N_EVALS_POSITION 5
#define PCIE_PL_GEN3_EQ_FMDC_N_EVALS_MASK   (0x1f << PCIE_PL_GEN3_EQ_FMDC_N_EVALS_POSITION  )

#define PCIE_PL_GEN3_EQ_FMDC_MAX_PRE_CUSROR_DELTA_POSITION 10
#define PCIE_PL_GEN3_EQ_FMDC_MAX_PRE_CUSROR_DELTA_MASK   (0xf << PCIE_PL_GEN3_EQ_FMDC_MAX_PRE_CUSROR_DELTA_POSITION  )

#define PCIE_PL_GEN3_EQ_FMDC_MAX_POST_CUSROR_DELTA_POSITION 14
#define PCIE_PL_GEN3_EQ_FMDC_MAX_POST_CUSROR_DELTA_MASK   (0xf << PCIE_PL_GEN3_EQ_FMDC_MAX_POST_CUSROR_DELTA_POSITION  )

#define PCIE_PL_GEN3_EQ_FMDC_T_MIN_PHASE23_POSITION 0
#define PCIE_PL_GEN3_EQ_FMDC_T_MIN_PHASE23_MASK   (0x1f << PCIE_PL_GEN3_EQ_FMDC_T_MIN_PHASE23_POSITION  )

#define PCIE_PL_GEN3_EQ_VIOLATE_COEF_RULES_POSITION 0
#define PCIE_PL_GEN3_EQ_VIOLATE_COEF_RULES_MASK   (0x1 << PCIE_PL_GEN3_EQ_VIOLATE_COEF_RULES_POSITION  )

#define PCIE_PL_RXTS_EQ_MISMATCH_POSITION       0
#define PCIE_PL_RXTS_EQ_MISMATCH_MASK           (0x1 << PCIE_PL_RXTS_EQ_MISMATCH_POSITION)
#define PCIE_PL_RCVD_REQ_EQ_GUA_0_POSITION      1
#define PCIE_PL_RCVD_REQ_EQ_GUA_0_MASK          (0x1 << PCIE_PL_RCVD_REQ_EQ_GUA_0_POSITION)
#define PCIE_PL_RCVD_REQ_EQ_GUA_1_POSITION      2
#define PCIE_PL_RCVD_REQ_EQ_GUA_1_MASK          (0x1 << PCIE_PL_RCVD_REQ_EQ_GUA_1_POSITION)
#define PCIE_PL_RCVD_REQ_EQ_GUA_1_RATE_POSITION 3
#define PCIE_PL_RCVD_REQ_EQ_GUA_1_RATE_MASK     (0x1 << PCIE_PL_RCVD_REQ_EQ_GUA_1_RATE_POSITION)

#define PCIE_PL_NP_PASS_P_POSITION 0
#define PCIE_PL_NP_PASS_P_MASK  (0xFF << PCIE_PL_NP_PASS_P_POSITION)

#define PCIE_PL_CPL_PASS_P_POSITION 8
#define PCIE_PL_CPL_PASS_P_MASK  (0xFF << PCIE_PL_CPL_PASS_P_POSITION)

#define PCIE_PL_DELETE_EN_POSITION 31
#define PCIE_PL_DELETE_EN_MASK  (0x1 << PCIE_PL_DELETE_EN_POSITION)

#define PCIE_PL_LOOK_UP_ID_POSITION 0
#define PCIE_PL_LOOK_UP_ID_MASK  (0x7fffffff << PCIE_PL_LOOK_UP_ID_POSITION)

#define PCIE_PL_RADM_CLK_GATING_EN_POSITION 0
#define PCIE_PL_RADM_CLK_GATING_EN_MASK  (0x1 << PCIE_PL_RADM_CLK_GATING_EN_POSITION)

#define PCIE_PL_LPBK_RXVALID_POSITION 0
#define PCIE_PL_LPBK_RXVALID_MASK  (0xffff << PCIE_PL_LPBK_RXVALID_POSITION)

#define PCIE_PL_RXSTATUS_LANE_POSITION 16
#define PCIE_PL_RXSTATUS_LANE_MASK  (0x3f << PCIE_PL_RXSTATUS_LANE_POSITION)

#define PCIE_PL_RXSTATUS_VALUE_POSITION 24
#define PCIE_PL_RXSTATUS_VALUE_MASK (0x7 << PCIE_PL_RXSTATUS_VALUE_POSITION)

#define PCIE_PL_PIPE_LOOPBACK_POSITION 31
#define PCIE_PL_PIPE_LOOPBACK_MASK  (0x1 << PCIE_PL_PIPE_LOOPBACK_POSITION)

#define PCIE_PL_DBI_RO_WR_EN_POSITION 0
#define PCIE_PL_DBI_RO_WR_EN_MASK ( 0x1 << PCIE_PL_DBI_RO_WR_EN_POSITION)

#define PCIE_PL_DEFAULT_TARGET_POSITION 1
#define PCIE_PL_DEFAULT_TARGET_MASK ( 0x1 << PCIE_PL_DEFAULT_TARGET_POSITION)

#define PCIE_PL_UR_CA_MASK_4_TRGT1_POSITION 2
#define PCIE_PL_UR_CA_MASK_4_TRGT1_MASK ( 0x1 << PCIE_PL_UR_CA_MASK_4_TRGT1_POSITION)

#define  PCIE_PL_SIMPLIFIED_REPLAY_TIMER_POSITION 3
#define  PCIE_PL_SIMPLIFIED_REPLAY_TIMER_MASK (0x1 << PCIE_PL_SIMPLIFIED_REPLAY_TIMER_POSITION)

#define  PCIE_PL_DISABLE_AUTO_LTR_CLR_MSG_POSITION 4
#define  PCIE_PL_DISABLE_AUTO_LTR_CLR_MSG_MASK (0x1 << PCIE_PL_DISABLE_AUTO_LTR_CLR_MSG_POSITION)

#define  PCIE_PL_ARI_DEVICE_NUMBER_POSITION 5
#define  PCIE_PL_ARI_DEVICE_NUMBER_MASK (0x1 << PCIE_PL_ARI_DEVICE_NUMBER_POSITION)

#define PCIE_PL_TARGET_LINK_WIDTH_POSITION 0
#define PCIE_PL_TARGET_LINK_WIDTH_MASK ( 0x3f<< PCIE_PL_TARGET_LINK_WIDTH_POSITION)

#define PCIE_PL_DIRECT_LINK_WIDTH_CHANGE_POSITION 6
#define PCIE_PL_DIRECT_LINK_WIDTH_CHANGE_MASK ( 0x1 << PCIE_PL_DIRECT_LINK_WIDTH_CHANGE_POSITION)

#define PCIE_PL_UPCONFIGURE_SUPPORT_POSITION 7
#define PCIE_PL_UPCONFIGURE_SUPPORT_MASK ( 0x1 << PCIE_PL_UPCONFIGURE_SUPPORT_POSITION)

#define  PCIE_PL_RXSTANDBY_CONTROL_POSITION 0
#define  PCIE_PL_L1SUB_EXIT_MODE_POSITION   8
#define  PCIE_PL_L1_NOWAIT_P1_POSITION      9
#define  PCIE_PL_L1_CLK_SEL_POSITION        10
#define  PCIE_PL_RXSTANDBY_CONTROL_MASK (0x7F << PCIE_PL_RXSTANDBY_CONTROL_POSITION)
#define  PCIE_PL_L1SUB_EXIT_MODE_MASK   (0x1 << PCIE_PL_L1SUB_EXIT_MODE_POSITION)
#define  PCIE_PL_L1_NOWAIT_P1_MASK      (0x1 << PCIE_PL_L1_NOWAIT_P1_POSITION)
#define  PCIE_PL_L1_CLK_SEL_MASK        (0x1 << PCIE_PL_L1_CLK_SEL_POSITION)

#define  PCIE_PL_PF0_HIDDEN_POSITION    0
#define  PCIE_PL_PF0_HIDDEN_MASK        (0x3 << PCIE_PL_PF0_HIDDEN_POSITION)

#define  PCIE_PL_PF1_HIDDEN_POSITION    2
#define  PCIE_PL_PF1_HIDDEN_MASK        (0x3 << PCIE_PL_PF1_HIDDEN_POSITION)

#define  PCIE_PL_PF2_HIDDEN_POSITION    4
#define  PCIE_PL_PF2_HIDDEN_MASK        (0x3 << PCIE_PL_PF2_HIDDEN_POSITION)

#define  PCIE_PL_PF3_HIDDEN_POSITION    6
#define  PCIE_PL_PF3_HIDDEN_MASK        (0x3 << PCIE_PL_PF3_HIDDEN_POSITION)

#define  PCIE_PL_PF4_HIDDEN_POSITION    8
#define  PCIE_PL_PF4_HIDDEN_MASK        (0x3 << PCIE_PL_PF4_HIDDEN_POSITION)

#define  PCIE_PL_PF5_HIDDEN_POSITION    10
#define  PCIE_PL_PF5_HIDDEN_MASK        (0x3 << PCIE_PL_PF5_HIDDEN_POSITION)

#define  PCIE_PL_PF6_HIDDEN_POSITION    12
#define  PCIE_PL_PF6_HIDDEN_MASK        (0x3 << PCIE_PL_PF6_HIDDEN_POSITION)

#define  PCIE_PL_PF7_HIDDEN_POSITION    14
#define  PCIE_PL_PF7_HIDDEN_MASK        (0x3 << PCIE_PL_PF7_HIDDEN_POSITION)

#define  PCIE_PL_PF8_HIDDEN_POSITION    16
#define  PCIE_PL_PF8_HIDDEN_MASK        (0x3 << PCIE_PL_PF8_HIDDEN_POSITION)

#define  PCIE_PL_PF9_HIDDEN_POSITION    18
#define  PCIE_PL_PF9_HIDDEN_MASK        (0x3 << PCIE_PL_PF9_HIDDEN_POSITION)

#define  PCIE_PL_PF10_HIDDEN_POSITION   20
#define  PCIE_PL_PF10_HIDDEN_MASK       (0x3 << PCIE_PL_PF10_HIDDEN_POSITION)

#define  PCIE_PL_PF11_HIDDEN_POSITION   22
#define  PCIE_PL_PF11_HIDDEN_MASK       (0x3 << PCIE_PL_PF11_HIDDEN_POSITION)

#define  PCIE_PL_PF12_HIDDEN_POSITION   24
#define  PCIE_PL_PF12_HIDDEN_MASK       (0x3 << PCIE_PL_PF12_HIDDEN_POSITION)

#define  PCIE_PL_PF13_HIDDEN_POSITION   26
#define  PCIE_PL_PF13_HIDDEN_MASK       (0x3 << PCIE_PL_PF13_HIDDEN_POSITION)

#define  PCIE_PL_PF14_HIDDEN_POSITION   28
#define  PCIE_PL_PF14_HIDDEN_MASK       (0x3 << PCIE_PL_PF14_HIDDEN_POSITION)

#define  PCIE_PL_PF15_HIDDEN_POSITION   30
#define  PCIE_PL_PF15_HIDDEN_MASK       (0x3 << PCIE_PL_PF15_HIDDEN_POSITION)

#define  PCIE_PL_REGION_INDEX_POSITION 0
#define  PCIE_PL_REGION_INDEX_MASK      ( {ATU_LOG2_REGIONS {0x1}} << PCIE_PL_REGION_INDEX_POSITION)
#define  PCIE_PL_VP_RSVD_POSITION ATU_LOG2_REGIONS
#define  PCIE_PL_VP_RSVD_MASK       ({31-ATU_LOG2_REGIONS{0x1}}<< PCIE_PL_VP_RSVD_POSITION)
#define  PCIE_PL_REGION_DIR_POSITION 31
#define  PCIE_PL_REGION_DIR_MASK       (0x1 << PCIE_PL_REGION_DIR_POSITION)

#define  PCIE_PL_TYPE_POSITION 0
#define  PCIE_PL_TYPE_MASK       (0x1f << PCIE_PL_TYPE_POSITION)
#define  PCIE_PL_TC_POSITION 5
#define  PCIE_PL_TC_MASK       (0x7 << PCIE_PL_TC_POSITION)
#define  PCIE_PL_TD_POSITION 8
#define  PCIE_PL_TD_MASK       (0x1 << PCIE_PL_TD_POSITION)
#define  PCIE_PL_ATTR_POSITION 9
#define  PCIE_PL_ATTR_MASK     (0x3 << PCIE_PL_ATTR_POSITION)
#define  PCIE_PL_IDO_POSITION  11
#define  PCIE_PL_IDO_MASK      (0x1 << PCIE_PL_IDO_POSITION)

#define  PCIE_PL_TH_POSITION 12
#define  PCIE_PL_TH_MASK    (0x1 << PCIE_PL_TH_POSITION)

#define  PCIE_PL_INCREASE_REGION_SIZE_POSITION 13
#define  PCIE_PL_INCREASE_REGION_SIZE_MASK (0x1 << PCIE_PL_INCREASE_REGION_SIZE_POSITION)

#define  PCIE_PL_AT_POSITION 16
#define  PCIE_PL_AT_MASK     (0x3 << PCIE_PL_AT_POSITION)

#define  PCIE_PL_PH_POSITION 18
#define  PCIE_PL_PH_MASK      (0x3 << PCIE_PL_PH_POSITION)


#define  PCIE_PL_CTRL_1_FUNC_NUM_POSITION 20
#define  PCIE_PL_CTRL_1_FUNC_NUM_MASK    ({CX_NFUNC_WD{0x1}} << PCIE_PL_CTRL_1_FUNC_NUM_POSITION)

#define  PCIE_PL_MSG_CODE_POSITION 0
#define  PCIE_PL_MSG_CODE_MASK       (0xff << PCIE_PL_MSG_CODE_POSITION)
#define  PCIE_PL_BAR_NUM_POSITION 8
#define  PCIE_PL_BAR_NUM_MASK       (0x7 << PCIE_PL_BAR_NUM_POSITION)
#define  PCIE_PL_TC_MATCH_EN_POSITION 14
#define  PCIE_PL_TC_MATCH_EN_MASK       (0x1 << PCIE_PL_TC_MATCH_EN_POSITION)
#define  PCIE_PL_TD_MATCH_EN_POSITION 15
#define  PCIE_PL_TD_MATCH_EN_MASK       (0x1 << PCIE_PL_TD_MATCH_EN_POSITION)
#define  PCIE_PL_ATTR_MATCH_EN_POSITION 16
#define  PCIE_PL_ATTR_MATCH_EN_MASK       (0x1 << PCIE_PL_ATTR_MATCH_EN_POSITION)
#define  PCIE_PL_TH_MATCH_EN_POSITION 17
#define  PCIE_PL_TH_MATCH_EN_MASK       (0x1 << PCIE_PL_TH_MATCH_EN_POSITION)
#define  PCIE_PL_SINGLE_ADDR_LOC_TRANS_EN_POSITION 23
#define  PCIE_PL_SINGLE_ADDR_LOC_TRANS_EN_MASK (0x1 << PCIE_PL_SINGLE_ADDR_LOC_TRANS_EN_POSITION)
#define  PCIE_PL_MSG_TYPE_MATCH_MODE_POSITION 13
#define  PCIE_PL_MSG_TYPE_MATCH_MODE_MASK (0x1 << PCIE_PL_MSG_TYPE_MATCH_MODE_POSITION)

#define  PCIE_PL_TAG_POSITION   8
#define  PCIE_PL_TAG_MASK       (0xff << PCIE_PL_TAG_POSITION)
#define  PCIE_PL_TAG_SUBSTITUTE_EN_POSITION 16
#define  PCIE_PL_TAG_SUBSTITUTE_EN_MASK (0x1 << PCIE_PL_TAG_SUBSTITUTE_EN_POSITION)
#define  PCIE_PL_MSB2BITS_TAG_POSITION 17
#define  PCIE_PL_MSB2BITS_TAG_MASK (0x3 << PCIE_PL_MSB2BITS_TAG_POSITION)
#define  PCIE_PL_INHIBIT_PAYLOAD_POSITION 22
#define  PCIE_PL_INHIBIT_PAYLOAD_MASK (0x1 << PCIE_PL_INHIBIT_PAYLOAD_POSITION)
#define  PCIE_PL_HEADER_SUBSTITUTE_EN_POSITION 23
#define  PCIE_PL_HEADER_SUBSTITUTE_EN_MASK    (0x1 << PCIE_PL_HEADER_SUBSTITUTE_EN_POSITION)
#define  PCIE_PL_AT_MATCH_EN_POSITION 18
#define  PCIE_PL_AT_MATCH_EN_MASK       (0x1 << PCIE_PL_AT_MATCH_EN_POSITION)
#define  PCIE_PL_FUNC_NUM_MATCH_EN_POSITION 19
#define  PCIE_PL_FUNC_NUM_MATCH_EN_MASK       (0x1 << PCIE_PL_FUNC_NUM_MATCH_EN_POSITION)
#define  PCIE_PL_SNP_POSITION 20
#define  PCIE_PL_SNP_MASK               (0x1 << PCIE_PL_SNP_POSITION)
#define  PCIE_PL_FUNC_BYPASS_POSITION 19
#define  PCIE_PL_FUNC_BYPASS_MASK       (0x1 << PCIE_PL_FUNC_NUM_MATCH_EN_POSITION)
#define  PCIE_PL_VF_MATCH_EN_POSITION 20
#define  PCIE_PL_VF_MATCH_EN_MASK       (0x1 << PCIE_PL_VF_MATCH_EN_POSITION)
#define  PCIE_PL_MSG_CODE_MATCH_EN_POSITION 21
#define  PCIE_PL_MSG_CODE_MATCH_EN_MASK       (0x1 << PCIE_PL_MSG_CODE_MATCH_EN_POSITION)
#define  PCIE_PL_PH_MATCH_EN_POSITION 22
#define  PCIE_PL_PH_MATCH_EN_MASK       (0x1 << PCIE_PL_PH_MATCH_EN_POSITION)

#define  PCIE_PL_RESPONSE_CODE_POSITION 24
#define  PCIE_PL_RESPONSE_CODE_MASK       (0x3 << PCIE_PL_RESPONSE_CODE_POSITION)
#define  PCIE_PL_VFBAR_MATCH_MODE_EN_POSITION     26
#define  PCIE_PL_VFBAR_MATCH_MODE_EN_MASK     (0x1 << PCIE_PL_VFBAR_MATCH_MODE_EN_POSITION)
#define  PCIE_PL_FUZZY_TYPE_MATCH_CODE_POSITION 27
#define  PCIE_PL_FUZZY_TYPE_MATCH_CODE_MASK       (0x1 << PCIE_PL_FUZZY_TYPE_MATCH_CODE_POSITION)
#define  PCIE_PL_DMA_BYPASS_POSITION 27
#define  PCIE_PL_DMA_BYPASS_MASK (0x1 << PCIE_PL_DMA_BYPASS_POSITION)
#define  PCIE_PL_CFG_SHIFT_MODE_POSITION 28
#define  PCIE_PL_CFG_SHIFT_MODE_MASK       (0x1 << PCIE_PL_CFG_SHIFT_MODE_POSITION)
#define  PCIE_PL_INVERT_MODE_POSITION 29
#define  PCIE_PL_INVERT_MODE_MASK       (0x1 << PCIE_PL_INVERT_MODE_POSITION)
#define  PCIE_PL_MATCH_MODE_POSITION 30
#define  PCIE_PL_MATCH_MODE_MASK       (0x1 << PCIE_PL_MATCH_MODE_POSITION)
#define  PCIE_PL_REGION_EN_POSITION 31
#define  PCIE_PL_REGION_EN_MASK       (0x1 << PCIE_PL_REGION_EN_POSITION)

#define  PCIE_PL_LWR_BASE_HW_POSITION 0

#define  PCIE_PL_LWR_BASE_HW_MASK   (CX_ATU_MIN_REGION_SIZE==65536) ? (0xffff << `PCIE_PL_LWR_BASE_HW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==32768) ? (0x7fff << `PCIE_PL_LWR_BASE_HW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==16384) ? (0x3fff << `PCIE_PL_LWR_BASE_HW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==8192) ? (0x1fff << `PCIE_PL_LWR_BASE_HW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==4096) ? (0xfff<< `PCIE_PL_LWR_BASE_HW_POSITION) : (0xffff << `PCIE_PL_LWR_BASE_HW_POSITION)

#define  PCIE_PL_LWR_BASE_RW_POSITION  ((CX_ATU_MIN_REGION_SIZE==65536) ? 16 : (`CX_ATU_MIN_REGION_SIZE==32768) ? 15 : (`CX_ATU_MIN_REGION_SIZE==16384) ? 14 : (`CX_ATU_MIN_REGION_SIZE==8192) ? 13 : (`CX_ATU_MIN_REGION_SIZE==4096) ? 12 : 16)

#define  PCIE_PL_LWR_BASE_RW_MASK  (CX_ATU_MIN_REGION_SIZE==65536) ? (0xffff << `PCIE_PL_LWR_BASE_RW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==32768) ? (0x1ffff << `PCIE_PL_LWR_BASE_RW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==16384) ? (0x3ffff << `PCIE_PL_LWR_BASE_RW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==8192) ? (0x7ffff << `PCIE_PL_LWR_BASE_RW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==4096) ? (0xfffff << `PCIE_PL_LWR_BASE_RW_POSITION) : (0xffff << `PCIE_PL_LWR_BASE_RW_POSITION)

#define  PCIE_PL_UPPER_BASE_RW_POSITION 0
#define  PCIE_PL_UPPER_BASE_RW_MASK      (0xffff_ffff << PCIE_PL_UPPER_BASE_RW_POSITION)

#define  PCIE_PL_LIMIT_ADDR_HW_POSITION 0

#define  PCIE_PL_LIMIT_ADDR_HW_MASK    (CX_ATU_MIN_REGION_SIZE==65536) ? (0xffff << `PCIE_PL_LIMIT_ADDR_HW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==32768) ? (0x7fff << `PCIE_PL_LIMIT_ADDR_HW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==16384) ? (0x3fff<< `PCIE_PL_LIMIT_ADDR_HW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==8192) ? (0x1fff << `PCIE_PL_LIMIT_ADDR_HW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==4096) ? (0xfff << `PCIE_PL_LIMIT_ADDR_HW_POSITION): (0xffff << `PCIE_PL_LIMIT_ADDR_HW_POSITION)

#define  PCIE_PL_LIMIT_ADDR_RW_POSITION ((CX_ATU_MIN_REGION_SIZE==65536) ? 16 : (`CX_ATU_MIN_REGION_SIZE==32768) ? 15 : (`CX_ATU_MIN_REGION_SIZE==16384) ? 14 : (`CX_ATU_MIN_REGION_SIZE==8192) ? 13 : (`CX_ATU_MIN_REGION_SIZE==4096) ? 12 : 16)

#define  PCIE_PL_LIMIT_ADDR_RW_MASK     (CX_ATU_MIN_REGION_SIZE==65536) ? (0xffff << `PCIE_PL_LIMIT_ADDR_RW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==32768) ? (0x1ffff << `PCIE_PL_LIMIT_ADDR_RW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==16384) ? (0x3ffff << `PCIE_PL_LIMIT_ADDR_RW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==8192) ? (0x7ffff << `PCIE_PL_LIMIT_ADDR_RW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==4096) ? (0xfffff << `PCIE_PL_LIMIT_ADDR_RW_POSITION) : (0xffff << `PCIE_PL_LIMIT_ADDR_RW_POSITION)

// 1'b0 used below to avoid Zero width replication error when max_size=0/32
//TMP #define  PCIE_PL_UPPR_LIMIT_ADDR_RW_POSITION 0
//TMP #define  PCIE_PL_UPPR_LIMIT_ADDR_RW_MASK ({1'b0, {(CX_ATU_MAX_REGION_SIZE){0x1}}} << `PCIE_PL_UPPR_LIMIT_ADDR_RW_POSITION)
//TMP #define  PCIE_PL_UPPR_LIMIT_ADDR_HW_POSITION CX_ATU_MAX_REGION_SIZE
//TMP #define  PCIE_PL_UPPR_LIMIT_ADDR_HW_MASK ({1'b0, {32-(CX_ATU_MAX_REGION_SIZE){0x1}}} << `PCIE_PL_UPPR_LIMIT_ADDR_HW_POSITION)
//TMP
//TMP #define  PCIE_PL_LWR_TARGET_HW_POSITION 0
//TMP
//TMP #define  PCIE_PL_LWR_TARGET_HW_MASK    (CX_ATU_MIN_REGION_SIZE==65536) ? (0xffff << `PCIE_PL_LWR_TARGET_HW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==32768) ? (0x7fff << `PCIE_PL_LWR_TARGET_HW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==16384) ? (0x3fff << `PCIE_PL_LWR_TARGET_HW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==8192) ? (0x1fff << `PCIE_PL_LWR_TARGET_HW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==4096) ? (0xfff<< `PCIE_PL_LWR_TARGET_HW_POSITION): (0xffff << `PCIE_PL_LWR_TARGET_HW_POSITION)
//TMP
//TMP #define  PCIE_PL_LWR_TARGET_RW_POSITION ((CX_ATU_MIN_REGION_SIZE==65536) ? 16 : (`CX_ATU_MIN_REGION_SIZE==32768) ? 15 : (`CX_ATU_MIN_REGION_SIZE==16384) ? 14 : (`CX_ATU_MIN_REGION_SIZE==8192) ? 13 : (`CX_ATU_MIN_REGION_SIZE==4096) ? 12 : 16)
//TMP
//TMP #define  PCIE_PL_LWR_TARGET_RW_MASK     (CX_ATU_MIN_REGION_SIZE==65536) ? (0xffff << `PCIE_PL_LWR_TARGET_RW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==32768) ? (0x1ffff << `PCIE_PL_LWR_TARGET_RW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==16384) ? (0x3ffff << `PCIE_PL_LWR_TARGET_RW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==8192) ? (0x7ffff << `PCIE_PL_LWR_TARGET_RW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==4096) ? (0xfffff << `PCIE_PL_LWR_TARGET_RW_POSITION) : (0xffff << `PCIE_PL_LWR_TARGET_RW_POSITION)

#define PCIE_PL_LWR_TARGET_RW_OUTBOUND_POSITION 0
#define PCIE_PL_LWR_TARGET_RW_OUTBOUND_MASK (0xffffffff << PCIE_PL_LWR_TARGET_RW_OUTBOUND_POSITION)

#define  PCIE_PL_UPPER_TARGET_RW_POSITION 0
#define  PCIE_PL_UPPER_TARGET_RW_MASK (0xffff_ffff << PCIE_PL_UPPER_TARGET_RW_POSITION)

#define  PCIE_PL_VF_ACTIVE_POSITION 31
#define  PCIE_PL_VF_ACTIVE_MASK (0x1 << PCIE_PL_VF_ACTIVE_POSITION)

#define  PCIE_PL_VF_NUMBER_POSITION 0
#define  PCIE_PL_VF_NUMBER_MASK ({(CX_NVFUNC_WD-1){0x1}}  << `PCIE_PL_VF_NUMBER_POSITION)
#define  PCIE_PL_RSVDP_VF_NUMBER_POSITION (CX_NVFUNC_WD-1)
#define  PCIE_PL_RSVDP_VF_NUMBER_MASK ({(31-(CX_NVFUNC_WD-1)){0x1}}  << `PCIE_PL_RSVDP_VF_NUMBER_POSITION)

#define  PCIE_PL_RTRGT1_WEIGHT_POSITION 0
#define  PCIE_PL_RTRGT1_WEIGHT_MASK (0x7 << PCIE_PL_RTRGT1_WEIGHT_POSITION)

#define  PCIE_PL_WR_CTRL_TRGT_WEIGHT_POSITION 3
#define  PCIE_PL_WR_CTRL_TRGT_WEIGHT_MASK (0x7 << PCIE_PL_WR_CTRL_TRGT_WEIGHT_POSITION)

#define  PCIE_PL_RD_CTRL_TRGT_WEIGHT_POSITION 6
#define  PCIE_PL_RD_CTRL_TRGT_WEIGHT_MASK (0x7 << PCIE_PL_RD_CTRL_TRGT_WEIGHT_POSITION)

#define  PCIE_PL_RDBUFF_TRGT_WEIGHT_POSITION 9
#define  PCIE_PL_RDBUFF_TRGT_WEIGHT_MASK (0x7 << PCIE_PL_RDBUFF_TRGT_WEIGHT_POSITION)

#define  PCIE_PL_NUM_DMA_WR_CHAN_POSITION 0
#define  PCIE_PL_NUM_DMA_WR_CHAN_MASK (0xF << PCIE_PL_NUM_DMA_WR_CHAN_POSITION)

#define  PCIE_PL_NUM_DMA_RD_CHAN_POSITION 16
#define  PCIE_PL_NUM_DMA_RD_CHAN_MASK (0xF << PCIE_PL_NUM_DMA_RD_CHAN_POSITION)

#define  PCIE_PL_DIS_C2W_CACHE_WR_POSITION 24
#define  PCIE_PL_DIS_C2W_CACHE_WR_MASK (0x1 << PCIE_PL_DIS_C2W_CACHE_WR_POSITION)

#define  PCIE_PL_DIS_C2W_CACHE_RD_POSITION 25
#define  PCIE_PL_DIS_C2W_CACHE_RD_MASK (0x1 << PCIE_PL_DIS_C2W_CACHE_RD_POSITION)

#define  PCIE_PL_DMA_WRITE_ENGINE_POSITION 0
#define  PCIE_PL_DMA_WRITE_ENGINE_MASK      (0x1 << PCIE_PL_DMA_WRITE_ENGINE_POSITION)

#define  PCIE_PL_WR_DOORBELL_NUM_POSITION 0
#define  PCIE_PL_WR_DOORBELL_NUM_MASK      (0x7 << PCIE_PL_WR_DOORBELL_NUM_POSITION)
#define  PCIE_PL_WR_STOP_POSITION 31
#define  PCIE_PL_WR_STOP_MASK       (0x1 << PCIE_PL_WR_STOP_POSITION)

#define PCIE_PL_WR_P_TIMER_LIMIT_POSITION 0
#define PCIE_PL_WR_P_TIMER_LIMIT_MASK (0xFFF << PCIE_PL_WR_P_TIMER_LIMIT_POSITION)
#define PCIE_PL_WR_P_TIMER_DISABLE_POSITION 15
#define PCIE_PL_WR_P_TIMER_DISABLE_MASK (0x1 << PCIE_PL_WR_P_TIMER_DISABLE_POSITION)

#define  PCIE_PL_WRITE_CHANNEL0_WEIGHT_POSITION 0
#define  PCIE_PL_WRITE_CHANNEL0_WEIGHT_MASK (0x1f << PCIE_PL_WRITE_CHANNEL0_WEIGHT_POSITION)

#define  PCIE_PL_WRITE_CHANNEL1_WEIGHT_POSITION 5
#define  PCIE_PL_WRITE_CHANNEL1_WEIGHT_MASK (0x1f << PCIE_PL_WRITE_CHANNEL1_WEIGHT_POSITION)

#define  PCIE_PL_WRITE_CHANNEL2_WEIGHT_POSITION 10
#define  PCIE_PL_WRITE_CHANNEL2_WEIGHT_MASK  (0x1f << PCIE_PL_WRITE_CHANNEL2_WEIGHT_POSITION)

#define  PCIE_PL_WRITE_CHANNEL3_WEIGHT_POSITION 15
#define  PCIE_PL_WRITE_CHANNEL3_WEIGHT_MASK  (0x1f << PCIE_PL_WRITE_CHANNEL3_WEIGHT_POSITION)

#define  PCIE_PL_WRITE_CHANNEL4_WEIGHT_POSITION 0
#define  PCIE_PL_WRITE_CHANNEL4_WEIGHT_MASK (0x1f << PCIE_PL_WRITE_CHANNEL4_WEIGHT_POSITION)

#define  PCIE_PL_WRITE_CHANNEL5_WEIGHT_POSITION 5
#define  PCIE_PL_WRITE_CHANNEL5_WEIGHT_MASK (0x1f << PCIE_PL_WRITE_CHANNEL5_WEIGHT_POSITION)

#define  PCIE_PL_WRITE_CHANNEL6_WEIGHT_POSITION 10
#define  PCIE_PL_WRITE_CHANNEL6_WEIGHT_MASK (0x1f << PCIE_PL_WRITE_CHANNEL6_WEIGHT_POSITION)

#define  PCIE_PL_WRITE_CHANNEL7_WEIGHT_POSITION 15
#define  PCIE_PL_WRITE_CHANNEL7_WEIGHT_MASK (0x1f << PCIE_PL_WRITE_CHANNEL7_WEIGHT_POSITION)

#define  PCIE_PL_DMA_READ_ENGINE_POSITION 0
#define  PCIE_PL_DMA_READ_ENGINE_MASK      (0x1 << PCIE_PL_DMA_READ_ENGINE_POSITION)

#define  PCIE_PL_RD_DOORBELL_NUM_POSITION 0
#define  PCIE_PL_RD_DOORBELL_NUM_MASK     (0x7 << PCIE_PL_RD_DOORBELL_NUM_POSITION)
#define  PCIE_PL_RD_STOP_POSITION 31
#define  PCIE_PL_RD_STOP_MASK       (0x1 << PCIE_PL_RD_STOP_POSITION)


#define  PCIE_PL_READ_CHANNEL0_WEIGHT_POSITION 0
#define  PCIE_PL_READ_CHANNEL0_WEIGHT_MASK (0x1f << PCIE_PL_READ_CHANNEL0_WEIGHT_POSITION)

#define  PCIE_PL_READ_CHANNEL1_WEIGHT_POSITION 5
#define  PCIE_PL_READ_CHANNEL1_WEIGHT_MASK (0x1f << PCIE_PL_READ_CHANNEL1_WEIGHT_POSITION)

#define  PCIE_PL_READ_CHANNEL2_WEIGHT_POSITION 10
#define  PCIE_PL_READ_CHANNEL2_WEIGHT_MASK  (0x1f << PCIE_PL_READ_CHANNEL2_WEIGHT_POSITION)

#define  PCIE_PL_READ_CHANNEL3_WEIGHT_POSITION 15
#define  PCIE_PL_READ_CHANNEL3_WEIGHT_MASK  (0x1f << PCIE_PL_READ_CHANNEL3_WEIGHT_POSITION)

#define  PCIE_PL_READ_CHANNEL4_WEIGHT_POSITION 0
#define  PCIE_PL_READ_CHANNEL4_WEIGHT_MASK (0x1f << PCIE_PL_READ_CHANNEL4_WEIGHT_POSITION)

#define  PCIE_PL_READ_CHANNEL5_WEIGHT_POSITION 5
#define  PCIE_PL_READ_CHANNEL5_WEIGHT_MASK (0x1f << PCIE_PL_READ_CHANNEL5_WEIGHT_POSITION)

#define  PCIE_PL_READ_CHANNEL6_WEIGHT_POSITION 10
#define  PCIE_PL_READ_CHANNEL6_WEIGHT_MASK (0x1f << PCIE_PL_READ_CHANNEL6_WEIGHT_POSITION)

#define  PCIE_PL_READ_CHANNEL7_WEIGHT_POSITION 15
#define  PCIE_PL_READ_CHANNEL7_WEIGHT_MASK (0x1f << PCIE_PL_READ_CHANNEL7_WEIGHT_POSITION)

#define  PCIE_PL_WR_DONE_INT_STATUS_POSITION 0
#define  PCIE_PL_WR_DONE_INT_STATUS_MASK       (0xff << PCIE_PL_WR_DONE_INT_STATUS_POSITION)
#define  PCIE_PL_WR_ABORT_INT_STATUS_POSITION 16
#define  PCIE_PL_WR_ABORT_INT_STATUS_MASK       (0xff << PCIE_PL_WR_ABORT_INT_STATUS_POSITION)

#define  PCIE_PL_WR_DONE_INT_MASK_POSITION 0
#define  PCIE_PL_WR_DONE_INT_MASK_MASK      (dma_wr_chan_reset << PCIE_PL_WR_DONE_INT_MASK_POSITION)
#define  PCIE_PL_WR_ABORT_INT_MASK_POSITION 16
#define  PCIE_PL_WR_ABORT_INT_MASK_MASK       (dma_wr_chan_reset << PCIE_PL_WR_ABORT_INT_MASK_POSITION)

#define  PCIE_PL_WR_DONE_INT_CLEAR_POSITION 0
#define  PCIE_PL_WR_DONE_INT_CLEAR_MASK      (dma_wr_chan_reset  << PCIE_PL_WR_DONE_INT_CLEAR_POSITION)
#define  PCIE_PL_WR_ABORT_INT_CLEAR_POSITION 16
#define  PCIE_PL_WR_ABORT_INT_CLEAR_MASK       (dma_wr_chan_reset  << PCIE_PL_WR_DONE_INT_CLEAR_POSITION)

#define  PCIE_PL_APP_READ_ERR_DETECT_POSITION 0
#define  PCIE_PL_APP_READ_ERR_DETECT_MASK       (dma_wr_chan_reset  << PCIE_PL_APP_READ_ERR_DETECT_POSITION)
#define  PCIE_PL_LINKLIST_ELEMENT_FETCH_ERR_DETECT_POSITION 16
#define  PCIE_PL_LINKLIST_ELEMENT_FETCH_ERR_DETECT_MASK       (dma_wr_chan_reset  << PCIE_PL_LINKLIST_ELEMENT_FETCH_ERR_DETECT_POSITION)

#define  PCIE_PL_DMA_WRITE_DONE_LOW_REG_POSITION 0
#define  PCIE_PL_DMA_WRITE_DONE_LOW_REG_MASK      (0xffff_ffff << PCIE_PL_DMA_WRITE_DONE_LOW_REG_POSITION)

#define  PCIE_PL_DMA_WRITE_DONE_HIGH_REG_POSITION 0
#define  PCIE_PL_DMA_WRITE_DONE_HIGH_REG_MASK      (0xffff_ffff << PCIE_PL_DMA_WRITE_DONE_HIGH_REG_POSITION)

#define  PCIE_PL_DMA_WRITE_ABORT_LOW_REG_POSITION 0
#define  PCIE_PL_DMA_WRITE_ABORT_LOW_REG_MASK      (0xffff_ffff << PCIE_PL_DMA_WRITE_ABORT_LOW_REG_POSITION)

#define  PCIE_PL_DMA_WRITE_ABORT_HIGH_REG_POSITION 0
#define  PCIE_PL_DMA_WRITE_ABORT_HIGH_REG_MASK      (0xffff_ffff << PCIE_PL_DMA_WRITE_ABORT_HIGH_REG_POSITION)

#define  PCIE_PL_WR_CHANNEL_0_DATA_POSITION 0
#define  PCIE_PL_WR_CHANNEL_0_DATA_MASK      (0xffff << PCIE_PL_WR_CHANNEL_0_DATA_POSITION)
#define  PCIE_PL_RESERVED_WR_CHANNEL_0_DATA_POSITION 16
#define  PCIE_PL_RESERVED_WR_CHANNEL_0_DATA_MASK      (0xffff << PCIE_PL_RESERVED_WR_CHANNEL_0_DATA_POSITION)
#define  PCIE_PL_WR_CHANNEL_1_DATA_POSITION 16
#define  PCIE_PL_WR_CHANNEL_1_DATA_MASK      (0xffff << PCIE_PL_WR_CHANNEL_1_DATA_POSITION)

#define  PCIE_PL_WR_CHANNEL_2_DATA_POSITION 0
#define  PCIE_PL_WR_CHANNEL_2_DATA_MASK      (0xffff << PCIE_PL_WR_CHANNEL_2_DATA_POSITION)
#define  PCIE_PL_RESERVED_WR_CHANNEL_2_DATA_POSITION 16
#define  PCIE_PL_RESERVED_WR_CHANNEL_2_DATA_MASK (0xffff << PCIE_PL_RESERVED_WR_CHANNEL_2_DATA_POSITION)
#define  PCIE_PL_WR_CHANNEL_3_DATA_POSITION 16
#define  PCIE_PL_WR_CHANNEL_3_DATA_MASK      (0xffff << PCIE_PL_WR_CHANNEL_3_DATA_POSITION)

#define  PCIE_PL_WR_CHANNEL_4_DATA_POSITION 0
#define  PCIE_PL_WR_CHANNEL_4_DATA_MASK      (0xffff << PCIE_PL_WR_CHANNEL_4_DATA_POSITION)
#define  PCIE_PL_RESERVED_WR_CHANNEL_4_DATA_POSITION 16
#define  PCIE_PL_RESERVED_WR_CHANNEL_4_DATA_MASK (0xffff << PCIE_PL_RESERVED_WR_CHANNEL_4_DATA_POSITION)
#define  PCIE_PL_WR_CHANNEL_5_DATA_POSITION 16
#define  PCIE_PL_WR_CHANNEL_5_DATA_MASK      (0xffff << PCIE_PL_WR_CHANNEL_5_DATA_POSITION)

#define  PCIE_PL_WR_CHANNEL_6_DATA_POSITION 0
#define  PCIE_PL_WR_CHANNEL_6_DATA_MASK      (0xffff << PCIE_PL_WR_CHANNEL_6_DATA_POSITION)
#define  PCIE_PL_RESERVED_WR_CHANNEL_6_DATA_POSITION 16
#define  PCIE_PL_RESERVED_WR_CHANNEL_6_DATA_MASK (0xffff << PCIE_PL_RESERVED_WR_CHANNEL_6_DATA_POSITION)
#define  PCIE_PL_WR_CHANNEL_7_DATA_POSITION 16
#define  PCIE_PL_WR_CHANNEL_7_DATA_MASK      (0xffff << PCIE_PL_WR_CHANNEL_7_DATA_POSITION)

#define  PCIE_PL_WR_CHANNEL_LLRAIE_POSITION 0
#define  PCIE_PL_WR_CHANNEL_LLRAIE_MASK      (dma_wr_chan_reset << PCIE_PL_WR_CHANNEL_LLRAIE_POSITION)
#define  PCIE_PL_WR_CHANNEL_LLLAIE_POSITION 16
#define  PCIE_PL_WR_CHANNEL_LLLAIE_MASK       (dma_wr_chan_reset  << PCIE_PL_WR_CHANNEL_LLLAIE_POSITION)

#define  PCIE_PL_RD_DONE_INT_STATUS_POSITION 0
#define  PCIE_PL_RD_DONE_INT_STATUS_MASK      (dma_rd_chan_reset  << PCIE_PL_RD_DONE_INT_STATUS_POSITION)
#define  PCIE_PL_RD_ABORT_INT_STATUS_POSITION 16
#define  PCIE_PL_RD_ABORT_INT_STATUS_MASK     (dma_rd_chan_reset  << PCIE_PL_RD_ABORT_INT_STATUS_POSITION)

#define  PCIE_PL_RD_DONE_INT_MASK_POSITION 0
#define  PCIE_PL_RD_DONE_INT_MASK_MASK       (dma_rd_chan_reset << PCIE_PL_RD_DONE_INT_MASK_POSITION)
#define  PCIE_PL_RD_ABORT_INT_MASK_POSITION 16
#define  PCIE_PL_RD_ABORT_INT_MASK_MASK       (dma_rd_chan_reset  << PCIE_PL_RD_ABORT_INT_MASK_POSITION)

#define  PCIE_PL_RD_DONE_INT_CLEAR_POSITION 0
#define  PCIE_PL_RD_DONE_INT_CLEAR_MASK       (dma_rd_chan_reset << PCIE_PL_RD_DONE_INT_CLEAR_POSITION)
#define  PCIE_PL_RD_ABORT_INT_CLEAR_POSITION 16
#define  PCIE_PL_RD_ABORT_INT_CLEAR_MASK       (dma_rd_chan_reset << PCIE_PL_RD_ABORT_INT_CLEAR_POSITION)

#define  PCIE_PL_APP_WR_ERR_DETECT_POSITION 0
#define  PCIE_PL_APP_WR_ERR_DETECT_MASK      (0xff << PCIE_PL_APP_WR_ERR_DETECT_POSITION)
#define  PCIE_PL_LINK_LIST_ELEMENT_FETCH_ERR_DETECT_POSITION 16
#define  PCIE_PL_LINK_LIST_ELEMENT_FETCH_ERR_DETECT_MASK       (0xff << PCIE_PL_LINK_LIST_ELEMENT_FETCH_ERR_DETECT_POSITION)

#define  PCIE_PL_UNSUPPORTED_REQ_POSITION 0
#define  PCIE_PL_UNSUPPORTED_REQ_MASK      (0xff << PCIE_PL_UNSUPPORTED_REQ_POSITION)
#define  PCIE_PL_CPL_ABORT_POSITION 8
#define  PCIE_PL_CPL_ABORT_MASK      (0xff << PCIE_PL_CPL_ABORT_POSITION)
#define  PCIE_PL_CPL_TIMEOUT_POSITION 16
#define  PCIE_PL_CPL_TIMEOUT_MASK       (0xff << PCIE_PL_CPL_TIMEOUT_POSITION)
#define  PCIE_PL_DATA_POISIONING_POSITION 24
#define  PCIE_PL_DATA_POISIONING_MASK       (0xff << PCIE_PL_DATA_POISIONING_POSITION)

#define  PCIE_PL_RD_CHANNEL_LLRAIE_POSITION 0
#define  PCIE_PL_RD_CHANNEL_LLRAIE_MASK      (dma_rd_chan_reset << PCIE_PL_RD_CHANNEL_LLRAIE_POSITION)
#define  PCIE_PL_RD_CHANNEL_LLLAIE_POSITION 16
#define  PCIE_PL_RD_CHANNEL_LLLAIE_MASK       (dma_rd_chan_reset << PCIE_PL_RD_CHANNEL_LLLAIE_POSITION)

#define  PCIE_PL_DMA_READ_DONE_LOW_REG_POSITION 0
#define  PCIE_PL_DMA_READ_DONE_LOW_REG_MASK       (0xffff_ffff << PCIE_PL_DMA_READ_DONE_LOW_REG_POSITION)

#define  PCIE_PL_DMA_READ_DONE_HIGH_REG_POSITION 0
#define  PCIE_PL_DMA_READ_DONE_HIGH_REG_MASK       (0xffff_ffff << PCIE_PL_DMA_READ_DONE_HIGH_REG_POSITION)

#define  PCIE_PL_DMA_READ_ABORT_LOW_REG_POSITION 0
#define  PCIE_PL_DMA_READ_ABORT_LOW_REG_MASK      (0xffff_ffff << PCIE_PL_DMA_READ_ABORT_LOW_REG_POSITION)

#define  PCIE_PL_DMA_READ_ABORT_HIGH_REG_POSITION 0
#define  PCIE_PL_DMA_READ_ABORT_HIGH_REG_MASK      (0xffff_ffff << PCIE_PL_DMA_READ_ABORT_HIGH_REG_POSITION)

#define  PCIE_PL_RD_CHANNEL_0_DATA_POSITION 0
#define  PCIE_PL_RD_CHANNEL_0_DATA_MASK      (0xffff << PCIE_PL_RD_CHANNEL_0_DATA_POSITION)
#define  PCIE_PL_RESERVED_RD_CHANNEL_0_DATA_POSITION 16
#define  PCIE_PL_RESERVED_RD_CHANNEL_0_DATA_MASK (0xffff << PCIE_PL_RESERVED_RD_CHANNEL_0_DATA_POSITION)
#define  PCIE_PL_RD_CHANNEL_1_DATA_POSITION 16
#define  PCIE_PL_RD_CHANNEL_1_DATA_MASK      (0xffff << PCIE_PL_RD_CHANNEL_1_DATA_POSITION)

#define  PCIE_PL_RD_CHANNEL_2_DATA_POSITION 0
#define  PCIE_PL_RD_CHANNEL_2_DATA_MASK      (0xffff << PCIE_PL_RD_CHANNEL_2_DATA_POSITION)
#define  PCIE_PL_RESERVED_RD_CHANNEL_2_DATA_POSITION 16
#define  PCIE_PL_RESERVED_RD_CHANNEL_2_DATA_MASK (0xffff << PCIE_PL_RESERVED_RD_CHANNEL_2_DATA_POSITION)
#define  PCIE_PL_RD_CHANNEL_3_DATA_POSITION 16
#define  PCIE_PL_RD_CHANNEL_3_DATA_MASK      (0xffff << PCIE_PL_RD_CHANNEL_3_DATA_POSITION)

#define  PCIE_PL_RD_CHANNEL_4_DATA_POSITION 0
#define  PCIE_PL_RD_CHANNEL_4_DATA_MASK      (0xffff << PCIE_PL_RD_CHANNEL_4_DATA_POSITION)
#define  PCIE_PL_RESERVED_RD_CHANNEL_4_DATA_POSITION 16
#define  PCIE_PL_RESERVED_RD_CHANNEL_4_DATA_MASK (0xffff << PCIE_PL_RESERVED_RD_CHANNEL_4_DATA_POSITION)
#define  PCIE_PL_RD_CHANNEL_5_DATA_POSITION 16
#define  PCIE_PL_RD_CHANNEL_5_DATA_MASK      (0xffff << PCIE_PL_RD_CHANNEL_5_DATA_POSITION)

#define  PCIE_PL_RD_CHANNEL_6_DATA_POSITION 0
#define  PCIE_PL_RD_CHANNEL_6_DATA_MASK      (0xffff << PCIE_PL_RD_CHANNEL_6_DATA_POSITION)
#define  PCIE_PL_RESERVED_RD_CHANNEL_6_DATA_POSITION 16
#define  PCIE_PL_RESERVED_RD_CHANNEL_6_DATA_MASK (0xffff << PCIE_PL_RESERVED_RD_CHANNEL_6_DATA_POSITION)

#define  PCIE_PL_RD_CHANNEL_7_DATA_POSITION 16
#define  PCIE_PL_RD_CHANNEL_7_DATA_MASK      (0xffff << PCIE_PL_RD_CHANNEL_7_DATA_POSITION)


#define  PCIE_PL_CHANNEL_DIR_POSITION 31
#define  PCIE_PL_CHANNEL_DIR_MASK     (0x1 << PCIE_PL_CHANNEL_DIR_POSITION)
#define  PCIE_PL_CHANNEL_NUM_POSITION 0
#define  PCIE_PL_CHANNEL_NUM_MASK     (0x7 << PCIE_PL_CHANNEL_NUM_POSITION)

#define  PCIE_PL_CB_POSITION 0
#define  PCIE_PL_CB_MASK      (0x1 << PCIE_PL_CB_POSITION)
#define  PCIE_PL_TCB_POSITION 1
#define  PCIE_PL_TCB_MASK      (0x1 << PCIE_PL_TCB_POSITION)
#define  PCIE_PL_LLP_POSITION 2
#define  PCIE_PL_LLP_MASK      (0x1 << PCIE_PL_LLP_POSITION)
#define  PCIE_PL_LIE_POSITION 3
#define  PCIE_PL_LIE_MASK      (0x1 << PCIE_PL_LIE_POSITION)
#define  PCIE_PL_RIE_POSITION 4
#define  PCIE_PL_RIE_MASK      (0x1 << PCIE_PL_RIE_POSITION)
#define  PCIE_PL_CS_POSITION 5
#define  PCIE_PL_CS_MASK      (0x3 << PCIE_PL_CS_POSITION)
#define  PCIE_PL_DMA_RESERVED0_POSITION 7
#define  PCIE_PL_DMA_RESERVED0_MASK (0x1 << PCIE_PL_DMA_RESERVED0_POSITION)
#define  PCIE_PL_CCS_POSITION 8
#define  PCIE_PL_CCS_MASK      (0x1 << PCIE_PL_CCS_POSITION)
#define  PCIE_PL_LLE_POSITION 9
#define  PCIE_PL_LLE_MASK      (0x1 << PCIE_PL_LLE_POSITION)
#define  PCIE_PL_DMA_RESERVED1_POSITION 10
#define  PCIE_PL_DMA_RESERVED1_MASK (0x3 << PCIE_PL_DMA_RESERVED0_POSITION)
#define  PCIE_PL_DMA_FUNC_NUM_POSITION 12
#define  PCIE_PL_DMA_FUNC_NUM_MASK       (0x1f << PCIE_PL_DMA_FUNC_NUM_POSITION)
#define  PCIE_PL_DMA_RESERVED2_POSITION 17
#define  PCIE_PL_DMA_RESERVED2_MASK (0x3f << PCIE_PL_DMA_RESERVED2_POSITION)
#define  PCIE_PL_DMA_NS_DST_POSITION 23
#define  PCIE_PL_DMA_NS_DST_MASK       (0x1 << PCIE_PL_DMA_NS_DST_POSITION)
#define  PCIE_PL_DMA_NS_SRC_POSITION 24
#define  PCIE_PL_DMA_NS_SRC_MASK       (0x1 << PCIE_PL_DMA_NS_SRC_POSITION)
#define  PCIE_PL_DMA_RO_POSITION 25
#define  PCIE_PL_DMA_RO_MASK       (0x1 << PCIE_PL_DMA_RO_POSITION)
#define  PCIE_PL_DMA_RESERVED5_POSITION 26
#define  PCIE_PL_DMA_RESERVED5_MASK       (0x1 << PCIE_PL_DMA_RESERVED5_POSITION)
#define  PCIE_PL_DMA_TC_POSITION 27
#define  PCIE_PL_DMA_TC_MASK       (0x7 << PCIE_PL_DMA_TC_POSITION)
#define  PCIE_PL_DMA_AT_POSITION 30
#define  PCIE_PL_DMA_AT_MASK       (0x3 << PCIE_PL_DMA_AT_POSITION)

#define  PCIE_PL_DMA_VF_EN_POSITION 0
#define  PCIE_PL_DMA_VF_EN_MASK     (0x1 << PCIE_PL_DMA_VF_EN_POSITION)
#define  PCIE_PL_DMA_VF_POSITION    1
#define  PCIE_PL_DMA_VF_MASK        (0xFF << PCIE_PL_DMA_VF_POSITION)
#define  PCIE_PL_DMA_VF_RESERVED_POSITION    9
#define  PCIE_PL_DMA_VF_RESERVED_MASK        (0xFF << PCIE_PL_DMA_VF_RESERVED_POSITION)
#define  PCIE_PL_DMA_RESERVED4_POSITION 0
#define  PCIE_PL_DMA_RESERVED4_MASK (0x1ffff << PCIE_PL_DMA_RESERVED4_POSITION)
#define  PCIE_PL_DMA_ST_POSITION    17
#define  PCIE_PL_DMA_ST_MASK        (0xFF << PCIE_PL_DMA_ST_POSITION)
#define  PCIE_PL_DMA_RESERVED3_POSITION 25
#define  PCIE_PL_DMA_RESERVED3_MASK (0xf << PCIE_PL_DMA_RESERVED3_POSITION)
#define  PCIE_PL_DMA_PH_POSITION    29
#define  PCIE_PL_DMA_PH_MASK        (0x3 << PCIE_PL_DMA_PH_POSITION)
#define  PCIE_PL_DMA_TH_POSITION    31
#define  PCIE_PL_DMA_TH_MASK        (0x1 << PCIE_PL_DMA_TH_POSITION)
#define  PCIE_PL_DMA_ST_RESERVED_POSITION 17
#define  PCIE_PL_DMA_ST_RESERVED_MASK  (0x7fff << PCIE_PL_DMA_ST_RESERVED_POSITION)

#define  PCIE_PL_DMA_TRANSFER_SIZE_POSITION 0
#define  PCIE_PL_DMA_TRANSFER_SIZE_MASK      (0xffff_ffff << PCIE_PL_DMA_TRANSFER_SIZE_POSITION)

#define  PCIE_PL_DST_ADDR_REG_LOW_POSITION 0
#define  PCIE_PL_DST_ADDR_REG_LOW_MASK      (0xffff_ffff << PCIE_PL_DST_ADDR_REG_LOW_POSITION)

#define  PCIE_PL_DST_ADDR_REG_HIGH_POSITION 0
#define  PCIE_PL_DST_ADDR_REG_HIGH_MASK      (0xffff_ffff << PCIE_PL_DST_ADDR_REG_HIGH_POSITION)

#define  PCIE_PL_SRC_ADDR_REG_LOW_POSITION 0
#define  PCIE_PL_SRC_ADDR_REG_LOW_MASK      (0xffff_ffff << PCIE_PL_SRC_ADDR_REG_LOW_POSITION)

#define  PCIE_PL_SRC_ADDR_REG_HIGH_POSITION 0
#define  PCIE_PL_SRC_ADDR_REG_HIGH_MASK      (0xffff_ffff << PCIE_PL_SRC_ADDR_REG_HIGH_POSITION)

#define  PCIE_PL_LLP_LOW_POSITION 0
#define  PCIE_PL_LLP_LOW_MASK       (0xffff_ffff << PCIE_PL_LLP_LOW_POSITION)

#define  PCIE_PL_LLP_HIGH_POSITION 0
#define  PCIE_PL_LLP_HIGH_MASK      (0xffff_ffff << PCIE_PL_LLP_HIGH_POSITION)

#define  PCIE_PL_SNOOP_LATENCY_VALUE_POSITION 0
#define  PCIE_PL_SNOOP_LATENCY_VALUE_MASK      (0x3ff << PCIE_PL_SNOOP_LATENCY_VALUE_POSITION)
#define  PCIE_PL_SNOOP_LATENCY_SCALE_POSITION 10
#define  PCIE_PL_SNOOP_LATENCY_SCALE_MASK      (0x7 << PCIE_PL_SNOOP_LATENCY_SCALE_POSITION)
#define  PCIE_PL_SNOOP_LATENCY_REQUIRE_POSITION 15
#define  PCIE_PL_SNOOP_LATENCY_REQUIRE_MASK      (0x1 << PCIE_PL_SNOOP_LATENCY_REQUIRE_POSITION)
#define  PCIE_PL_NO_SNOOP_LATENCY_VALUE_POSITION 16
#define  PCIE_PL_NO_SNOOP_LATENCY_VALUE_MASK      (0x3ff << PCIE_PL_NO_SNOOP_LATENCY_VALUE_POSITION)
#define  PCIE_PL_NO_SNOOP_LATENCY_SCALE_POSITION 26
#define  PCIE_PL_NO_SNOOP_LATENCY_SCALE_MASK      (0x7 << PCIE_PL_NO_SNOOP_LATENCY_SCALE_POSITION)
#define  PCIE_PL_NO_SNOOP_LATENCY_REQUIRE_POSITION 31
#define  PCIE_PL_NO_SNOOP_LATENCY_REQUIRE_MASK      (0x1 << PCIE_PL_NO_SNOOP_LATENCY_REQUIRE_POSITION)
#define  PCIE_PL_AUX_CLK_FREQ_POSITION 0
#define  PCIE_PL_AUX_CLK_FREQ_MASK      (0x3ff << PCIE_PL_AUX_CLK_FREQ_POSITION)

#define  PCIE_PL_L1SUB_T_POWER_OFF_POSITION  0
#define  PCIE_PL_L1SUB_T_L1_2_POSITION       2
#define  PCIE_PL_L1SUB_T_PCLKACK_POSITION    6

#define  PCIE_PL_L1SUB_T_POWER_OFF_MASK     (0x3 << PCIE_PL_L1SUB_T_POWER_OFF_POSITION)
#define  PCIE_PL_L1SUB_T_L1_2_MASK          (0xf << PCIE_PL_L1SUB_T_L1_2_POSITION)
#define  PCIE_PL_L1SUB_T_PCLKACK_MASK       (0x3 << PCIE_PL_L1SUB_T_PCLKACK_POSITION)

#define  PCIE_PL_AMBA_ORDRMGR_WDOG_EN_POSITION 31
#define  PCIE_PL_AMBA_ORDRMGR_WDOG_EN_MASK (0x1 << PCIE_PL_AMBA_ORDRMGR_WDOG_EN_POSITION)

#define  PCIE_PL_AMBA_ORDRMGR_WDOG_POSITION 0
#define  PCIE_PL_AMBA_ORDRMGR_WDOG_MASK (0xffff << PCIE_PL_AMBA_ORDRMGR_WDOG_POSITION)

#define  PCIE_PL_LINK_TIMEOUT_PERIOD_DEFAULT_POSITION 0
#define  PCIE_PL_LINK_TIMEOUT_PERIOD_DEFAULT_MASK (0xff << PCIE_PL_LINK_TIMEOUT_PERIOD_DEFAULT_POSITION)

#define  PCIE_PL_LINK_TIMEOUT_ENABLE_DEFAULT_POSITION 8
#define  PCIE_PL_LINK_TIMEOUT_ENABLE_DEFAULT_MASK (0x1 << PCIE_PL_LINK_TIMEOUT_ENABLE_DEFAULT_POSITION)

#define  PCIE_PL_AUTO_FLUSH_EN_POSITION 0
#define  PCIE_PL_AUTO_FLUSH_EN_MASK (0x1 << PCIE_PL_AUTO_FLUSH_EN_POSITION)

#define  PCIE_PL_RSVD_I_8_POSITION 24
#define  PCIE_PL_RSVD_I_8_MASK (0xff << PCIE_PL_RSVD_I_8_POSITION)

#define  PCIE_PL_AMBA_ERROR_RESPONSE_GLOBAL_POSITION 0
#define  PCIE_PL_AMBA_ERROR_RESPONSE_GLOBAL_MASK (0x1 << PCIE_PL_AMBA_ERROR_RESPONSE_GLOBAL_POSITION)

#define  PCIE_PL_AMBA_ERROR_RESPONSE_VENDORID_POSITION 2
#define  PCIE_PL_AMBA_ERROR_RESPONSE_VENDORID_MASK (0x1 << PCIE_PL_AMBA_ERROR_RESPONSE_VENDORID_POSITION)

#define  PCIE_PL_AMBA_ERROR_RESPONSE_CRS_POSITION 3
#define  PCIE_PL_AMBA_ERROR_RESPONSE_CRS_MASK (0x3 << PCIE_PL_AMBA_ERROR_RESPONSE_CRS_POSITION)

#define  PCIE_PL_AMBA_ERROR_RESPONSE_MAP_POSITION 10
#define  PCIE_PL_AMBA_ERROR_RESPONSE_MAP_MASK (0x3f << PCIE_PL_AMBA_ERROR_RESPONSE_MAP_POSITION)

#define PCIE_PL_AX_MSTR_ZEROLREAD_FW_POSITION 7
#define PCIE_PL_AX_MSTR_ZEROLREAD_FW_MASK (0x1 << PCIE_PL_AX_MSTR_ZEROLREAD_FW_POSITION)

#define PCIE_PL_AX_MSTR_ORDR_P_EVENT_SEL_POSITION 3
#define PCIE_PL_AX_MSTR_ORDR_P_EVENT_SEL_MASK (0x3 << PCIE_PL_AX_MSTR_ORDR_P_EVENT_SEL_POSITION)

#define PCIE_PL_AX_SNP_EN_POSITION 1
#define PCIE_PL_AX_SNP_EN_MASK (0x1 << PCIE_PL_AX_SNP_EN_POSITION)

// Dynamic Re-configuration control
#define PCIE_PL_RECOVERY_REQUEST_POSITION                   0
#define PCIE_PL_RECOVERY_REQUEST_MASK                       (0x1 << PCIE_PL_RECOVERY_REQUEST_POSITION)
#define PCIE_PL_RESERVED_0X450_7_1_POSITION                 1
#define PCIE_PL_RESERVED_0X450_7_1_MASK                     (0x7F << PCIE_PL_RESERVED_0X450_7_1_POSITION)
#define PCIE_PL_TGT_RATE_SERIES_POSITION                    8
#define PCIE_PL_TGT_RATE_SERIES_MASK                        (0x3 << PCIE_PL_TGT_RATE_SERIES_POSITION)
#define PCIE_PL_RESERVED_0X450_11_10_POSITION               10
#define PCIE_PL_RESERVED_0X450_11_10_MASK                   (0x3 << PCIE_PL_RESERVED_0X450_11_10_POSITION)
#define PCIE_PL_TGT_HS_GEAR_SPEED_POSITION                  12
#define PCIE_PL_TGT_HS_GEAR_SPEED_MASK                      (0xF << PCIE_PL_TGT_HS_GEAR_SPEED_POSITION)
#define PCIE_PL_TGT_TX_LANE_WIDTH_POSITION                  16
#define PCIE_PL_TGT_TX_LANE_WIDTH_MASK                      (0x3F << PCIE_PL_TGT_TX_LANE_WIDTH_POSITION)
#define PCIE_PL_RESERVED_0X450_23_22_POSITION               22
#define PCIE_PL_RESERVED_0X450_23_22_MASK                   (0x3 << PCIE_PL_RESERVED_0X450_23_22_POSITION)
#define PCIE_PL_TGT_RX_LANE_WIDTH_POSITION                  24
#define PCIE_PL_TGT_RX_LANE_WIDTH_MASK                      (0x3F << PCIE_PL_TGT_RX_LANE_WIDTH_POSITION)
#define PCIE_PL_RESERVED_0X450_31_30_POSITION               30
#define PCIE_PL_RESERVED_0X450_31_30_MASK                   (0x3 << PCIE_PL_RESERVED_0X450_31_30_POSITION)

// M-PCIe Control
#define PCIE_PL_STALL_ENTRY_ENABLE_POSITION                 0
#define PCIE_PL_STALL_ENTRY_ENABLE_MASK                     (0x1 << PCIE_PL_STALL_ENTRY_ENABLE_POSITION)
#define PCIE_PL_RESERVED_0X454_3_1_POSITION                 1
#define PCIE_PL_RESERVED_0X454_3_1_MASK                     (0x7 << PCIE_PL_RESERVED_0X454_3_1_POSITION)
#define PCIE_PL_LRC_ACCESS_CONTROL_POSITION                 4
#define PCIE_PL_LRC_ACCESS_CONTROL_MASK                     (0x3 << PCIE_PL_LRC_ACCESS_CONTROL_POSITION)
#define PCIE_PL_USP_PHY_CONTROL_REG_ENABLE_POSITION         6
#define PCIE_PL_USP_PHY_CONTROL_REG_ENABLE_MASK             (0x1 << PCIE_PL_USP_PHY_CONTROL_REG_ENABLE_POSITION)
#define PCIE_PL_LRC_ENABLE_POSITION                         7
#define PCIE_PL_LRC_ENABLE_MASK                             (0x1 << PCIE_PL_LRC_ENABLE_POSITION)
#define PCIE_PL_RESERVED_0X454_11_8_POSITION                8
#define PCIE_PL_RESERVED_0X454_11_8_MASK                    (0xF << PCIE_PL_RESERVED_0X454_11_8_POSITION)
#define PCIE_PL_PM_CLOCK_POLICY_POSITION                    12
#define PCIE_PL_PM_CLOCK_POLICY_MASK                        (0x3 << PCIE_PL_PM_CLOCK_POLICY_POSITION)
#define PCIE_PL_PM_ANALOG_POLICY_POSITION                   14
#define PCIE_PL_PM_ANALOG_POLICY_MASK                       (0x1 << PCIE_PL_PM_ANALOG_POLICY_POSITION)
#define PCIE_PL_RESERVED_0X454_15_POSITION                  15
#define PCIE_PL_RESERVED_0X454_15_MASK                      (0x1 << PCIE_PL_RESERVED_0X454_15_POSITION)
#define PCIE_PL_EBUF_THRESHOLD_POSITION                     16
#define PCIE_PL_EBUF_THRESHOLD_MASK                         (0x3F << PCIE_PL_EBUF_THRESHOLD_POSITION)
#define PCIE_PL_RESERVED_0X454_23_22_POSITION               22
#define PCIE_PL_RESERVED_0X454_23_22_MASK                   (0x3 << PCIE_PL_RESERVED_0X454_23_22_POSITION)
#define PCIE_PL_LINE_RESET_REQUEST_POSITION                 24
#define PCIE_PL_LINE_RESET_REQUEST_MASK                     (0x1 << PCIE_PL_LINE_RESET_REQUEST_POSITION)
#define PCIE_PL_CFGUPDT_REQUEST_POSITION                    25
#define PCIE_PL_CFGUPDT_REQUEST_MASK                        (0x1 << PCIE_PL_CFGUPDT_REQUEST_POSITION)
#define PCIE_PL_RESERVED_0X454_31_26_POSITION               26
#define PCIE_PL_RESERVED_0X454_31_26_MASK                   (0x3F << PCIE_PL_RESERVED_0X454_31_26_POSITION)

// M-PCIe LRC RRC initialization Status
#define PCIE_PL_RRC_TO_STATUS_POSITION                      0
#define PCIE_PL_RRC_TO_STATUS_MASK                          (0x1 << PCIE_PL_RRC_TO_STATUS_POSITION)
#define PCIE_PL_RRC_BIT_ERR_STATUS_POSITION                 1
#define PCIE_PL_RRC_BIT_ERR_STATUS_MASK                     (0x1 << PCIE_PL_RRC_BIT_ERR_STATUS_POSITION)
#define PCIE_PL_RRC_UNEXP_RESP_STATUS_POSITION              2
#define PCIE_PL_RRC_UNEXP_RESP_STATUS_MASK                  (0x1 << PCIE_PL_RRC_UNEXP_RESP_STATUS_POSITION)
#define PCIE_PL_LRC_ADDR_ERR_STATUS_POSITION                3
#define PCIE_PL_LRC_ADDR_ERR_STATUS_MASK                    (0x1 << PCIE_PL_LRC_ADDR_ERR_STATUS_POSITION)
#define PCIE_PL_PHY_CNTROL_REQ_ERR_STATUS_POSITION          4
#define PCIE_PL_PHY_CNTROL_REQ_ERR_STATUS_MASK              (0x1 << PCIE_PL_PHY_CNTROL_REQ_ERR_STATUS_POSITION)
#define PCIE_PL_RRC_ABORT_STATUS_POSITION                   5
#define PCIE_PL_RRC_ABORT_STATUS_MASK                       (0x1 << PCIE_PL_RRC_ABORT_STATUS_POSITION)
#define PCIE_PL_RESERVED_0X458_6_POSITION                   6
#define PCIE_PL_RESERVED_0X458_6_MASK                       (0x1 << PCIE_PL_RESERVED_0X458_6_POSITION)
#define PCIE_PL_RRAP_TARGET_STATUS_POSITION                 7
#define PCIE_PL_RRAP_TARGET_STATUS_MASK                     (0x1 << PCIE_PL_RRAP_TARGET_STATUS_POSITION)
#define PCIE_PL_RESERVED_0X458_15_8_POSITION                8
#define PCIE_PL_RESERVED_0X458_15_8_MASK                    (0xFF << PCIE_PL_RESERVED_0X458_15_8_POSITION)
#define PCIE_PL_HWRRAP_DONE_POSITION                        16
#define PCIE_PL_HWRRAP_DONE_MASK                            (0x1 << PCIE_PL_HWRRAP_DONE_POSITION)
#define PCIE_PL_HWRRAP_NOT_CONFIGURED_POSITION              17
#define PCIE_PL_HWRRAP_NOT_CONFIGURED_MASK                  (0x1 << PCIE_PL_HWRRAP_NOT_CONFIGURED_POSITION)
#define PCIE_PL_RESERVED_0X458_18_POSITION                  18
#define PCIE_PL_RESERVED_0X458_18_MASK                      (0x1 << PCIE_PL_RESERVED_0X458_18_POSITION)
#define PCIE_PL_HWRRAP_IN_PROGRESS_POSITION                 19
#define PCIE_PL_HWRRAP_IN_PROGRESS_MASK                     (0x1 << PCIE_PL_HWRRAP_IN_PROGRESS_POSITION)
#define PCIE_PL_RESERVED_0X458_23_20_POSITION               20
#define PCIE_PL_RESERVED_0X458_23_20_MASK                   (0xF << PCIE_PL_RESERVED_0X458_23_20_POSITION)
#define PCIE_PL_HWRRAP_PROGRAM_COUNTER_POSITION             24
#define PCIE_PL_HWRRAP_PROGRAM_COUNTER_MASK                 (0xFF << PCIE_PL_HWRRAP_PROGRAM_COUNTER_POSITION)

// M-PCIe Link Status
#define PCIE_PL_CURRENT_MPCIE_LTSSM_MAIN_STATE_POSITION     0
#define PCIE_PL_CURRENT_MPCIE_LTSSM_MAIN_STATE_MASK         (0xF << PCIE_PL_CURRENT_MPCIE_LTSSM_MAIN_STATE_POSITION)
#define PCIE_PL_CURRENT_MPCIE_LTSSM_SUB_STATE_POSITION      4
#define PCIE_PL_CURRENT_MPCIE_LTSSM_SUB_STATE_MASK          (0x7 << PCIE_PL_CURRENT_MPCIE_LTSSM_SUB_STATE_POSITION)
#define PCIE_PL_CURRENT_MPCIE_LTSSM_SUB_SUB_STATE_POSITION  7
#define PCIE_PL_CURRENT_MPCIE_LTSSM_SUB_SUB_STATE_MASK      (0x7 << PCIE_PL_CURRENT_MPCIE_LTSSM_SUB_SUB_STATE_POSITION)
#define PCIE_PL_RESERVED_0X45C_14_10_POSITION               10
#define PCIE_PL_RESERVED_0X45C_14_10_MASK                   (0x1F << PCIE_PL_RESERVED_0X45C_14_10_POSITION)
#define PCIE_PL_LINK_UP_DOWN_POSITION                       15
#define PCIE_PL_LINK_UP_DOWN_MASK                           (0x1 << PCIE_PL_LINK_UP_DOWN_POSITION)
#define PCIE_PL_CURRENT_HS_LIN_SPEED_POSITION               16
#define PCIE_PL_CURRENT_HS_LIN_SPEED_MASK                   (0xF << PCIE_PL_CURRENT_HS_LIN_SPEED_POSITION)
#define PCIE_PL_CURRENT_RATE_SERIES_POSITION                20
#define PCIE_PL_CURRENT_RATE_SERIES_MASK                    (0x3 << PCIE_PL_CURRENT_RATE_SERIES_POSITION)
#define PCIE_PL_RESERVED_0X45C_31_22_POSITION               22
#define PCIE_PL_RESERVED_0X45C_31_22_MASK                   (0x3FF << PCIE_PL_RESERVED_0X45C_31_22_POSITION)

// M-PCIe Lane Status
#define PCIE_PL_SELECT_DISPLAY_STATUS_POSITION              0
#define PCIE_PL_SELECT_DISPLAY_STATUS_MASK                  (0xF << PCIE_PL_SELECT_DISPLAY_STATUS_POSITION)
#define PCIE_PL_RESERVED_0X460_7_4_POSITION                 4
#define PCIE_PL_RESERVED_0X460_7_4_MASK                     (0xF << PCIE_PL_RESERVED_0X460_7_4_POSITION)
#define PCIE_PL_CLEAR_LANE_STATUS_POSITION                  8
#define PCIE_PL_CLEAR_LANE_STATUS_MASK                      (0x1 << PCIE_PL_CLEAR_LANE_STATUS_POSITION)
#define PCIE_PL_ANY_ERROR_STATUS_POSITION                   9
#define PCIE_PL_ANY_ERROR_STATUS_MASK                       (0x1 << PCIE_PL_ANY_ERROR_STATUS_POSITION)
#define PCIE_PL_RESERVED_0X460_15_10_POSITION               10
#define PCIE_PL_RESERVED_0X460_15_10_MASK                   (0x3F << PCIE_PL_RESERVED_0X460_15_10_POSITION)
#define PCIE_PL_LANE_STATUS_L0_POSITION                     16
#define PCIE_PL_LANE_STATUS_L0_MASK                         (0x1 << PCIE_PL_LANE_STATUS_L0_POSITION)
#define PCIE_PL_LANE_STATUS_L1_POSITION                     17
#define PCIE_PL_LANE_STATUS_L1_MASK                         (0x1 << PCIE_PL_LANE_STATUS_L1_POSITION)
#define PCIE_PL_LANE_STATUS_L2_POSITION                     18
#define PCIE_PL_LANE_STATUS_L2_MASK                         (0x1 << PCIE_PL_LANE_STATUS_L2_POSITION)
#define PCIE_PL_LANE_STATUS_L3_POSITION                     19
#define PCIE_PL_LANE_STATUS_L3_MASK                         (0x1 << PCIE_PL_LANE_STATUS_L3_POSITION)
#define PCIE_PL_LANE_STATUS_L4_POSITION                     20
#define PCIE_PL_LANE_STATUS_L4_MASK                         (0x1 << PCIE_PL_LANE_STATUS_L4_POSITION)
#define PCIE_PL_LANE_STATUS_L5_POSITION                     21
#define PCIE_PL_LANE_STATUS_L5_MASK                         (0x1 << PCIE_PL_LANE_STATUS_L5_POSITION)
#define PCIE_PL_LANE_STATUS_L6_POSITION                     22
#define PCIE_PL_LANE_STATUS_L6_MASK                         (0x1 << PCIE_PL_LANE_STATUS_L6_POSITION)
#define PCIE_PL_LANE_STATUS_L7_POSITION                     23
#define PCIE_PL_LANE_STATUS_L7_MASK                         (0x1 << PCIE_PL_LANE_STATUS_L7_POSITION)
#define PCIE_PL_LANE_STATUS_L8_POSITION                     24
#define PCIE_PL_LANE_STATUS_L8_MASK                         (0x1 << PCIE_PL_LANE_STATUS_L8_POSITION)
#define PCIE_PL_LANE_STATUS_L9_POSITION                     25
#define PCIE_PL_LANE_STATUS_L9_MASK                         (0x1 << PCIE_PL_LANE_STATUS_L9_POSITION)
#define PCIE_PL_LANE_STATUS_L10_POSITION                    26
#define PCIE_PL_LANE_STATUS_L10_MASK                        (0x1 << PCIE_PL_LANE_STATUS_L10_POSITION)
#define PCIE_PL_LANE_STATUS_L11_POSITION                    27
#define PCIE_PL_LANE_STATUS_L11_MASK                        (0x1 << PCIE_PL_LANE_STATUS_L11_POSITION)
#define PCIE_PL_LANE_STATUS_L12_POSITION                    28
#define PCIE_PL_LANE_STATUS_L12_MASK                        (0x1 << PCIE_PL_LANE_STATUS_L12_POSITION)
#define PCIE_PL_LANE_STATUS_L13_POSITION                    29
#define PCIE_PL_LANE_STATUS_L13_MASK                        (0x1 << PCIE_PL_LANE_STATUS_L13_POSITION)
#define PCIE_PL_LANE_STATUS_L14_POSITION                    30
#define PCIE_PL_LANE_STATUS_L14_MASK                        (0x1 << PCIE_PL_LANE_STATUS_L14_POSITION)
#define PCIE_PL_LANE_STATUS_L15_POSITION                    31
#define PCIE_PL_LANE_STATUS_L15_MASK                        (0x1 << PCIE_PL_LANE_STATUS_L15_POSITION)

// Target_Mep_Ctrl structure
#define PCIE_PL_TARGET_MAP_PF_POSITION                      0
#define PCIE_PL_TARGET_MAP_PF_MASK                          (0x3f  << PCIE_PL_TARGET_MAP_PF_POSITION)
#define PCIE_PL_TARGET_MAP_ROM_POSITION                     6
#define PCIE_PL_TARGET_MAP_ROM_MASK                         (0x1       << PCIE_PL_TARGET_MAP_ROM_POSITION)
#define PCIE_PL_TARGET_MAP_VF_POSITION                      7
#define PCIE_PL_TARGET_MAP_VF_MASK                          (0x3f  << PCIE_PL_TARGET_MAP_VF_POSITION)
#define PCIE_PL_TARGET_MAP_RESERVED_13_15_POSITION          13
#define PCIE_PL_TARGET_MAP_RESERVED_13_15_MASK              (0x7     << PCIE_PL_TARGET_MAP_RESERVED_13_15_POSITION)
#define PCIE_PL_TARGET_MAP_INDEX_POSITION                   16
#define PCIE_PL_TARGET_MAP_INDEX_MASK                       (0x1f   << PCIE_PL_TARGET_MAP_INDEX_POSITION)
#define PCIE_PL_TARGET_MAP_RESERVED_21_31_POSITION          21
#define PCIE_PL_TARGET_MAP_RESERVED_21_31_MASK              (0x7FF    << PCIE_PL_TARGET_MAP_RESERVED_21_31_POSITION)

 // PHY Register Viewport Control / Status
#define PCIE_PL_PHY_VIEWPORT_ADDR_POSITION                  0
#define PCIE_PL_PHY_VIEWPORT_ADDR_MASK                      (0xFFFF <<PCIE_PL_PHY_VIEWPORT_ADDR_POSITION)
#define PCIE_PL_PHY_VIEWPORT_NUM_POSITION                   16
#define PCIE_PL_PHY_VIEWPORT_NUM_MASK                       (0xF <<PCIE_PL_PHY_VIEWPORT_NUM_POSITION)
#define PCIE_PL_PHY_VIEWPORT_READ_POSITION                  20
#define PCIE_PL_PHY_VIEWPORT_READ_MASK                      (0x1 <<PCIE_PL_PHY_VIEWPORT_READ_POSITION)
#define PCIE_PL_PHY_VIEWPORT_BCWR_POSITION                  21
#define PCIE_PL_PHY_VIEWPORT_BCWR_MASK                      (0x1 <<PCIE_PL_PHY_VIEWPORT_BCWR_POSITION)
#define PCIE_PL_PHY_VIEWPORT_TIMEOUT_DISABLE_POSITION       22
#define PCIE_PL_PHY_VIEWPORT_TIMEOUT_DISABLE_MASK           (0x1 <<PCIE_PL_PHY_VIEWPORT_TIMEOUT_DISABLE_POSITION)
#define PCIE_PL_PHY_VIEWPORT_STATUS_POSITION                30
#define PCIE_PL_PHY_VIEWPORT_STATUS_MASK                    (0x1 <<PCIE_PL_PHY_VIEWPORT_STATUS_POSITION)
#define PCIE_PL_PHY_VIEWPORT_PENDING_POSITION               31
#define PCIE_PL_PHY_VIEWPORT_PENDING_MASK                   (0x1 <<PCIE_PL_PHY_VIEWPORT_PENDING_POSITION)

 // PHY Register Viewport Data
#define PCIE_PL_PHY_VIEWPORT_DATA_POSITION                  0
#define PCIE_PL_PHY_VIEWPORT_DATA_MASK                      ({CX_PHY_VIEWPORT_DATA{0x1}} <<`PCIE_PL_PHY_VIEWPORT_DATA_POSITION)

 // Gen4 Lane Margining 1
#define PCIE_PL_MARGINING_NUM_TIMING_STEPS_POSITION         0
#define PCIE_PL_MARGINING_NUM_TIMING_STEPS_MASK             (0x3F << PCIE_PL_MARGINING_NUM_TIMING_STEPS_POSITION)
#define PCIE_PL_MARGINING_MAX_TIMING_OFFSET_POSITION        8
#define PCIE_PL_MARGINING_MAX_TIMING_OFFSET_MASK            (0x3F << PCIE_PL_MARGINING_MAX_TIMING_OFFSET_POSITION)
#define PCIE_PL_MARGINING_NUM_VOLTAGE_STEPS_POSITION        16
#define PCIE_PL_MARGINING_NUM_VOLTAGE_STEPS_MASK            (0x7F << PCIE_PL_MARGINING_NUM_VOLTAGE_STEPS_POSITION)
#define PCIE_PL_MARGINING_MAX_VOLTAGE_OFFSET_POSITION       24
#define PCIE_PL_MARGINING_MAX_VOLTAGE_OFFSET_MASK           (0x3F << PCIE_PL_MARGINING_MAX_VOLTAGE_OFFSET_POSITION)

 // Gen4 Lane Margining 2
#define PCIE_PL_MARGINING_SAMPLE_RATE_VOLTAGE_POSITION      0
#define PCIE_PL_MARGINING_SAMPLE_RATE_VOLTAGE_MASK          (0x3F << PCIE_PL_MARGINING_SAMPLE_RATE_VOLTAGE_POSITION)
#define PCIE_PL_MARGINING_SAMPLE_RATE_TIMING_POSITION       8
#define PCIE_PL_MARGINING_SAMPLE_RATE_TIMING_MASK           (0x3F << PCIE_PL_MARGINING_SAMPLE_RATE_TIMING_POSITION)
#define PCIE_PL_MARGINING_MAXLANES_POSITION                 16
#define PCIE_PL_MARGINING_MAXLANES_MASK                     (0x1F << PCIE_PL_MARGINING_MAXLANES_POSITION)
#define PCIE_PL_MARGINING_VOLTAGE_SUPPORTED_POSITION        24
#define PCIE_PL_MARGINING_VOLTAGE_SUPPORTED_MASK            (0x1 << PCIE_PL_MARGINING_VOLTAGE_SUPPORTED_POSITION)
#define PCIE_PL_MARGINING_IND_UP_DOWN_VOLTAGE_POSITION      25
#define PCIE_PL_MARGINING_IND_UP_DOWN_VOLTAGE_MASK          (0x1 << PCIE_PL_MARGINING_IND_UP_DOWN_VOLTAGE_POSITION)
#define PCIE_PL_MARGINING_IND_LEFT_RIGHT_TIMING_POSITION    26
#define PCIE_PL_MARGINING_IND_LEFT_RIGHT_TIMING_MASK        (0x1 << PCIE_PL_MARGINING_IND_LEFT_RIGHT_TIMING_POSITION)
#define PCIE_PL_MARGINING_SAMPLE_REPORTING_METHOD_POSITION  27
#define PCIE_PL_MARGINING_SAMPLE_REPORTING_METHOD_MASK      (0x1 << PCIE_PL_MARGINING_SAMPLE_REPORTING_METHOD_POSITION)
#define PCIE_PL_MARGINING_IND_ERROR_SAMPLER_POSITION        28
#define PCIE_PL_MARGINING_IND_ERROR_SAMPLER_MASK            (0x1 << PCIE_PL_MARGINING_IND_ERROR_SAMPLER_POSITION)
#define PCIE_PL_MARGINING_RATE_MODE_POSITION                31
#define PCIE_PL_MARGINING_RATE_MODE_MASK                    (0x1 << PCIE_PL_MARGINING_RATE_MODE_POSITION)

 // PIPE Related Register
#define PCIE_PL_RX_MESSAGE_BUS_WRITE_BUFFER_DEPTH_POSITION  0
#define PCIE_PL_RX_MESSAGE_BUS_WRITE_BUFFER_DEPTH_MASK      (0xF << PCIE_PL_RX_MESSAGE_BUS_WRITE_BUFFER_DEPTH_POSITION)
#define PCIE_PL_TX_MESSAGE_BUS_WRITE_BUFFER_DEPTH_POSITION  4
#define PCIE_PL_TX_MESSAGE_BUS_WRITE_BUFFER_DEPTH_MASK      (0xF << PCIE_PL_TX_MESSAGE_BUS_WRITE_BUFFER_DEPTH_POSITION)
#define PCIE_PL_PIPE_GARBAGE_DATA_MODE_POSITION             8
#define PCIE_PL_PIPE_GARBAGE_DATA_MODE_MASK                 (0x1 << PCIE_PL_PIPE_GARBAGE_DATA_MODE_POSITION)

// 512b: Radm Formation Queue Overflow Prevention
#define  PCIE_PL_ALMOST_FULL_THRESHOLD_POSITION  0
#define  PCIE_PL_ALMOST_FULL_THRESHOLD_MASK      (0xffff << PCIE_PL_ALMOST_FULL_THRESHOLD_POSITION)
#define  PCIE_PL_ALMOST_FULL_THRESHOLD_ADJ_VAL_POSITION  16
#define  PCIE_PL_ALMOST_FULL_THRESHOLD_ADJ_VAL_MASK      (0xfff << PCIE_PL_ALMOST_FULL_THRESHOLD_ADJ_VAL_POSITION)
#define  PCIE_PL_ALMOST_FULL_THRESHOLD_ADJ_SIGN_POSITION  30
#define  PCIE_PL_ALMOST_FULL_THRESHOLD_ADJ_SIGN_MASK      (0x1 << PCIE_PL_ALMOST_FULL_THRESHOLD_ADJ_SIGN_POSITION)
#define  PCIE_PL_RX_SERIALIZATION_OVFLW_PRVNTN_EN_POSITION  31
#define  PCIE_PL_RX_SERIALIZATION_OVFLW_PRVNTN_EN_MASK      (0x1 << PCIE_PL_RX_SERIALIZATION_OVFLW_PRVNTN_EN_POSITION)

////////////////////////////////////////////
// AXI4 cap structure
////////////////////////////////////////////
#define  PCIE_PL_CFG_MEMTYPE_VALUE_POSITION 0
#define  PCIE_PL_CFG_MEMTYPE_VALUE_MASK (0x1 << PCIE_PL_CFG_MEMTYPE_VALUE_POSITION)
#define  PCIE_PL_CFG_MEMTYPE_BOUNDARY_LOW_ADDR_POSITION 2
#define  PCIE_PL_CFG_MEMTYPE_BOUNDARY_LOW_ADDR_MASK ({30{0x1}} << PCIE_PL_CFG_MEMTYPE_BOUNDARY_LOW_ADDR_POSITION)
#define  PCIE_PL_CFG_MEMTYPE_BOUNDARY_HIGH_ADDR_POSITION 0
#define  PCIE_PL_CFG_MEMTYPE_BOUNDARY_HIGH_ADDR_MASK  (0xffff_ffff << PCIE_PL_CFG_MEMTYPE_BOUNDARY_HIGH_ADDR_POSITION)


#define  PCIE_PL_CFG_MSTR_ARDOMAIN_MODE_POSITION 0
#define  PCIE_PL_CFG_MSTR_ARDOMAIN_MODE_MASK (0x3 << PCIE_PL_CFG_MSTR_ARDOMAIN_MODE_POSITION)
#define  PCIE_PL_CFG_MSTR_ARDOMAIN_MODE_RESERVED0_POSITION 0
#define  PCIE_PL_CFG_MSTR_ARDOMAIN_MODE_RESERVED0_MASK (0x7 << PCIE_PL_CFG_MSTR_ARDOMAIN_MODE_RESERVED0_POSITION)
#define  PCIE_PL_CFG_MSTR_ARCACHE_MODE_POSITION 3
#define  PCIE_PL_CFG_MSTR_ARCACHE_MODE_MASK  (0xf << PCIE_PL_CFG_MSTR_ARCACHE_MODE_POSITION)
#define  PCIE_PL_CFG_MSTR_ARCACHE_MODE_RESERVED0_POSITION 2
#define  PCIE_PL_CFG_MSTR_ARCACHE_MODE_RESERVED0_MASK (1 << PCIE_PL_CFG_MSTR_ARCACHE_MODE_RESERVED0_POSITION)
#define  PCIE_PL_CFG_MSTR_ARCACHE_MODE_RESERVED1_POSITION 7
#define  PCIE_PL_CFG_MSTR_ARCACHE_MODE_RESERVED1_MASK (0x1 << PCIE_PL_CFG_MSTR_ARCACHE_MODE_RESERVED1_POSITION)
#define  PCIE_PL_CFG_MSTR_AWDOMAIN_MODE_POSITION 8
#define  PCIE_PL_CFG_MSTR_AWDOMAIN_MODE_MASK (0x3 << PCIE_PL_CFG_MSTR_AWDOMAIN_MODE_POSITION)
#define  PCIE_PL_CFG_MSTR_AWDOMAIN_MODE_RESERVED0_POSITION 8
#define  PCIE_PL_CFG_MSTR_AWDOMAIN_MODE_RESERVED0_MASK (0x7 << PCIE_PL_CFG_MSTR_AWDOMAIN_MODE_RESERVED0_POSITION)
#define  PCIE_PL_CFG_MSTR_AWCACHE_MODE_POSITION  11
#define  PCIE_PL_CFG_MSTR_AWCACHE_MODE_MASK  (0xf << PCIE_PL_CFG_MSTR_AWCACHE_MODE_POSITION)
#define  PCIE_PL_CFG_MSTR_AWCACHE_MODE_RESERVED0_POSITION 10
#define  PCIE_PL_CFG_MSTR_AWCACHE_MODE_RESERVED0_MASK (1 << PCIE_PL_CFG_MSTR_AWCACHE_MODE_RESERVED0_POSITION)
#define  PCIE_PL_CFG_MSTR_AWCACHE_MODE_RESERVED1_POSITION 15
#define  PCIE_PL_CFG_MSTR_AWCACHE_MODE_RESERVED1_MASK (0x1 << PCIE_PL_CFG_MSTR_AWCACHE_MODE_RESERVED1_POSITION)
#define  PCIE_PL_CFG_MSTR_ARDOMAIN_VALUE_POSITION 16
#define  PCIE_PL_CFG_MSTR_ARDOMAIN_VALUE_MASK  (0x3 << PCIE_PL_CFG_MSTR_ARDOMAIN_VALUE_POSITION)
#define  PCIE_PL_CFG_MSTR_ARDOMAIN_VALUE_RESERVED0_POSITION 16
#define  PCIE_PL_CFG_MSTR_ARDOMAIN_VALUE_RESERVED0_MASK (0x7 << PCIE_PL_CFG_MSTR_ARDOMAIN_VALUE_RESERVED0_POSITION)
#define  PCIE_PL_CFG_MSTR_ARCACHE_VALUE_POSITION  19
#define  PCIE_PL_CFG_MSTR_ARCACHE_VALUE_MASK   (0xf << PCIE_PL_CFG_MSTR_ARCACHE_VALUE_POSITION)
#define  PCIE_PL_CFG_MSTR_ARCACHE_VALUE_RESERVED0_POSITION 18
#define  PCIE_PL_CFG_MSTR_ARCACHE_VALUE_RESERVED0_MASK (1<< PCIE_PL_CFG_MSTR_ARCACHE_VALUE_RESERVED0_POSITION)
#define  PCIE_PL_CFG_MSTR_ARCACHE_VALUE_RESERVED1_POSITION 23
#define  PCIE_PL_CFG_MSTR_ARCACHE_VALUE_RESERVED1_MASK (1 << PCIE_PL_CFG_MSTR_ARCACHE_VALUE_RESERVED1_POSITION)
#define  PCIE_PL_CFG_MSTR_AWDOMAIN_VALUE_POSITION 24
#define  PCIE_PL_CFG_MSTR_AWDOMAIN_VALUE_MASK  (0x3 << PCIE_PL_CFG_MSTR_AWDOMAIN_VALUE_POSITION)
#define  PCIE_PL_CFG_MSTR_AWDOMAIN_VALUE_RESERVED0_POSITION 24
#define  PCIE_PL_CFG_MSTR_AWDOMAIN_VALUE_RESERVED0_MASK (0x7 << PCIE_PL_CFG_MSTR_AWDOMAIN_VALUE_RESERVED0_POSITION)
#define  PCIE_PL_CFG_MSTR_AWCACHE_VALUE_POSITION  27
#define  PCIE_PL_CFG_MSTR_AWCACHE_VALUE_MASK   (0xf << PCIE_PL_CFG_MSTR_AWCACHE_VALUE_POSITION)
#define  PCIE_PL_CFG_MSTR_AWCACHE_VALUE_RESERVED0_POSITION 26
#define  PCIE_PL_CFG_MSTR_AWCACHE_VALUE_RESERVED0_MASK (0x1 << PCIE_PL_CFG_MSTR_AWCACHE_VALUE_RESERVED0_POSITION)
#define  PCIE_PL_CFG_MSTR_AWCACHE_VALUE_RESERVED1_POSITION 31
#define  PCIE_PL_CFG_MSTR_AWCACHE_VALUE_RESERVED1_MASK (0x1 << PCIE_PL_CFG_MSTR_AWCACHE_VALUE_RESERVED1_POSITION)

/////////////////////////////////
//AXI MASTER MSG ADDRESS REGISTER
/////////////////////////////////
#define  PCIE_PL_CFG_AXIMSTR_MSG_ADDR_LOW_RESERVED_POSITION 0
#define  PCIE_PL_CFG_AXIMSTR_MSG_ADDR_LOW_RESERVED_MASK (0xfff << PCIE_PL_CFG_AXIMSTR_MSG_ADDR_LOW_RESERVED_POSITION)
#define  PCIE_PL_CFG_AXIMSTR_MSG_ADDR_LOW_POSITION 12
#define  PCIE_PL_CFG_AXIMSTR_MSG_ADDR_LOW_MASK (0xfffff << PCIE_PL_CFG_AXIMSTR_MSG_ADDR_LOW_POSITION)
#define  PCIE_PL_CFG_AXIMSTR_MSG_ADDR_HIGH_POSITION 0
#define  PCIE_PL_CFG_AXIMSTR_MSG_ADDR_HIGH_MASK (0xffffffff << PCIE_PL_CFG_AXIMSTR_MSG_ADDR_HIGH_POSITION)

///////////////////////////////////
//PCIE RELEASE VERSION NUMBER REGISTER
#define PCIE_PL_VERSION_NUMBER_POSITION 0
#define PCIE_PL_VERSION_NUMBER_MASK  (0xffffffff << PCIE_PL_VERSION_NUMBER_POSITION)

//PCIE RELEASE VERSIOIN TYPE REGISTER
#define PCIE_PL_VERSION_TYPE_POSITION 0
#define PCIE_PL_VERSION_TYPE_MASK  (0xffffffff << PCIE_PL_VERSION_TYPE_POSITION)

//CDM Register Checking
#define PCIE_PL_CHK_REG_START_POSITION 0
#define PCIE_PL_CHK_REG_START_MASK  (0x1 << PCIE_PL_CHK_REG_START_POSITION)
#define PCIE_PL_CHK_REG_CONTINUOUS_POSITION 1
#define PCIE_PL_CHK_REG_CONTINUOUS_MASK  (0x1 << PCIE_PL_CHK_REG_CONTINUOUS_POSITION)
#define PCIE_PL_CHK_REG_COMPARISON_ERROR_POSITION 16
#define PCIE_PL_CHK_REG_COMPARISON_ERROR_MASK  (0x1 << PCIE_PL_CHK_REG_COMPARISON_ERROR_POSITION)
#define PCIE_PL_CHK_REG_LOGIC_ERROR_POSITION 17
#define PCIE_PL_CHK_REG_LOGIC_ERROR_MASK  (0x1 << PCIE_PL_CHK_REG_LOGIC_ERROR_POSITION)
#define PCIE_PL_CHK_REG_COMPLETE_POSITION 18
#define PCIE_PL_CHK_REG_COMPLETE_MASK  (0x1 << PCIE_PL_CHK_REG_COMPLETE_POSITION)
#define PCIE_PL_CHK_REG_START_ADDR_POSITION 0
#define PCIE_PL_CHK_REG_START_ADDR_MASK  (0xffff << PCIE_PL_CHK_REG_START_ADDR_POSITION)
#define PCIE_PL_CHK_REG_END_ADDR_POSITION 16
#define PCIE_PL_CHK_REG_END_ADDR_MASK  (0xffff << PCIE_PL_CHK_REG_END_ADDR_POSITION)
#define PCIE_PL_CHK_REG_ERR_ADDR_POSITION 0
#define PCIE_PL_CHK_REG_ERR_ADDR_MASK  (0xffffffff << PCIE_PL_CHK_REG_ERR_ADDR_POSITION)
#define PCIE_PL_CHK_REG_PF_ERR_NUMBER_POSITION 0
#define PCIE_PL_CHK_REG_PF_ERR_NUMBER_MASK  (0x1f << PCIE_PL_CHK_REG_PF_ERR_NUMBER_POSITION)
#define PCIE_PL_CHK_REG_VF_ERR_NUMBER_POSITION 16
#define PCIE_PL_CHK_REG_VF_ERR_NUMBER_MASK  (0xfff << PCIE_PL_CHK_REG_VF_ERR_NUMBER_POSITION)

////////////////////////////////////////////
// MSI-X TABLE address match registers
////////////////////////////////////////////
#define  PCIE_PL_MSIX_ADDRESS_MATCH_EN_POSITION 0
#define  PCIE_PL_MSIX_ADDRESS_MATCH_EN_MASK (0x1 << PCIE_PL_MSIX_ADDRESS_MATCH_EN_POSITION)
#define  PCIE_PL_MSIX_ADDRESS_MATCH_RESERVED_1_POSITION 1
#define  PCIE_PL_MSIX_ADDRESS_MATCH_RESERVED_1_MASK (0x1 << PCIE_PL_MSIX_ADDRESS_MATCH_RESERVED_1_POSITION)
#define  PCIE_PL_MSIX_ADDRESS_MATCH_LOW_POSITION 2
#define  PCIE_PL_MSIX_ADDRESS_MATCH_LOW_MASK (0x3fffffff << PCIE_PL_MSIX_ADDRESS_MATCH_LOW_POSITION)
#define  PCIE_PL_MSIX_ADDRESS_MATCH_HIGH_POSITION 0
#define  PCIE_PL_MSIX_ADDRESS_MATCH_HIGH_MASK (0xffffffff << PCIE_PL_MSIX_ADDRESS_MATCH_HIGH_POSITION)
#define  PCIE_PL_MSIX_DOORBELL_VECTOR_POSITION 0
#define  PCIE_PL_MSIX_DOORBELL_VECTOR_MASK (0x7ff << PCIE_PL_MSIX_DOORBELL_VECTOR_POSITION)
#define  PCIE_PL_MSIX_DOORBELL_RESERVED_11_POSITION 11
#define  PCIE_PL_MSIX_DOORBELL_RESERVED_11_MASK (0x1 << PCIE_PL_MSIX_DOORBELL_RESERVED_11_POSITION)
#define  PCIE_PL_MSIX_DOORBELL_TC_POSITION 12
#define  PCIE_PL_MSIX_DOORBELL_TC_MASK (0x7 << PCIE_PL_MSIX_DOORBELL_TC_POSITION)
#define  PCIE_PL_MSIX_DOORBELL_VF_ACTIVE_POSITION 15
#define  PCIE_PL_MSIX_DOORBELL_VF_ACTIVE_MASK (0x1 << PCIE_PL_MSIX_DOORBELL_VF_ACTIVE_POSITION)
#define  PCIE_PL_MSIX_DOORBELL_VF_POSITION 16
#define  PCIE_PL_MSIX_DOORBELL_VF_MASK (0xff << PCIE_PL_MSIX_DOORBELL_VF_POSITION)
#define  PCIE_PL_MSIX_DOORBELL_PF_POSITION 24
#define  PCIE_PL_MSIX_DOORBELL_PF_MASK (0x1f << PCIE_PL_MSIX_DOORBELL_PF_POSITION)
#define  PCIE_PL_MSIX_DOORBELL_RESERVED_29_31_POSITION 29
#define  PCIE_PL_MSIX_DOORBELL_RESERVED_29_31_MASK (0x7 << PCIE_PL_MSIX_DOORBELL_RESERVED_29_31_POSITION)
#define  PCIE_PL_MSIX_RAM_CTRL_TABLE_DS_POSITION 0
#define  PCIE_PL_MSIX_RAM_CTRL_TABLE_DS_MASK (0x1 << PCIE_PL_MSIX_RAM_CTRL_TABLE_DS_POSITION)
#define  PCIE_PL_MSIX_RAM_CTRL_TABLE_SD_POSITION 1
#define  PCIE_PL_MSIX_RAM_CTRL_TABLE_SD_MASK (0x1 << PCIE_PL_MSIX_RAM_CTRL_TABLE_SD_POSITION)
#define  PCIE_PL_MSIX_RAM_CTRL_RESERVED_2_7_POSITION 2
#define  PCIE_PL_MSIX_RAM_CTRL_RESERVED_2_7_MASK (0x3f << PCIE_PL_MSIX_RAM_CTRL_RESERVED_2_7_POSITION)
#define  PCIE_PL_MSIX_RAM_CTRL_PBA_DS_POSITION 8
#define  PCIE_PL_MSIX_RAM_CTRL_PBA_DS_MASK (0x1 << PCIE_PL_MSIX_RAM_CTRL_PBA_DS_POSITION)
#define  PCIE_PL_MSIX_RAM_CTRL_PBA_SD_POSITION 9
#define  PCIE_PL_MSIX_RAM_CTRL_PBA_SD_MASK (0x1 << PCIE_PL_MSIX_RAM_CTRL_PBA_SD_POSITION)
#define  PCIE_PL_MSIX_RAM_CTRL_RESERVED_10_15_POSITION 10
#define  PCIE_PL_MSIX_RAM_CTRL_RESERVED_10_15_MASK (0x3f << PCIE_PL_MSIX_RAM_CTRL_RESERVED_10_15_POSITION)
#define  PCIE_PL_MSIX_RAM_CTRL_BYPASS_POSITION 16
#define  PCIE_PL_MSIX_RAM_CTRL_BYPASS_MASK (0x1 << PCIE_PL_MSIX_RAM_CTRL_BYPASS_POSITION)
#define  PCIE_PL_MSIX_RAM_CTRL_RESERVED_17_23_POSITION 17
#define  PCIE_PL_MSIX_RAM_CTRL_RESERVED_17_23_MASK (0x7f << PCIE_PL_MSIX_RAM_CTRL_RESERVED_17_23_POSITION)
#define  PCIE_PL_MSIX_RAM_CTRL_DBG_TABLE_POSITION 24
#define  PCIE_PL_MSIX_RAM_CTRL_DBG_TABLE_MASK (0x1 << PCIE_PL_MSIX_RAM_CTRL_DBG_TABLE_POSITION)
#define  PCIE_PL_MSIX_RAM_CTRL_DBG_PBA_POSITION 25
#define  PCIE_PL_MSIX_RAM_CTRL_DBG_PBA_MASK (0x1 << PCIE_PL_MSIX_RAM_CTRL_DBG_PBA_POSITION)
#define  PCIE_PL_MSIX_RAM_CTRL_RESERVED_26_31_POSITION 26
#define  PCIE_PL_MSIX_RAM_CTRL_RESERVED_26_31_MASK (0x3f << PCIE_PL_MSIX_RAM_CTRL_RESERVED_26_31_POSITION)

////////////////////////////////////////////
// SN cap structure
////////////////////////////////////////////

#define SN_PCIE_EXTENDED_CAP_ID_POSITION 0
#define SN_CAP_VERSION_POSITION  16
#define SN_NEXT_OFFSET_POSITION  20

#define SN_PCIE_EXTENDED_CAP_ID_MASK 0xffff
#define SN_CAP_VERSION_MASK (0xf << SN_CAP_VERSION_POSITION)
#define SN_NEXT_OFFSET_MASK ( 0xfff<< SN_NEXT_OFFSET_POSITION)

#define SN_SER_NUM_REG_1_DW_POSITION 0
#define SN_SER_NUM_REG_1_DW_MASK (0xffff_ffff << SN_SER_NUM_REG_1_DW_POSITION )

#define SN_SER_NUM_REG_2_DW_POSITION 0
#define SN_SER_NUM_REG_2_DW_MASK (0xffff_ffff << SN_SER_NUM_REG_2_DW_POSITION )

////////////////////////////////////////////
// ARI cap structure
////////////////////////////////////////////

#define ARI_PCIE_EXTENDED_CAP_ID_POSITION 0
#define ARI_CAP_VERSION_POSITION  16
#define ARI_NEXT_OFFSET_POSITION  20

#define ARI_PCIE_EXTENDED_CAP_ID_MASK 0xffff
#define ARI_CAP_VERSION_MASK (0xf << ARI_CAP_VERSION_POSITION)
#define ARI_NEXT_OFFSET_MASK ( 0xfff<< ARI_NEXT_OFFSET_POSITION)

#define ARI_MFVC_FUN_GRP_CAP_POSITION 0
#define ARI_MFVC_FUN_GRP_CAP_MASK (0x1 << ARI_MFVC_FUN_GRP_CAP_POSITION )
#define ARI_ACS_FUN_GRP_CAP_POSITION 1
#define ARI_ACS_FUN_GRP_CAP_MASK (0x1 << ARI_ACS_FUN_GRP_CAP_POSITION )
#define ARI_NEXT_FUN_NUM_POSITION 8
#define ARI_NEXT_FUN_NUM_MASK (0xff << ARI_NEXT_FUN_NUM_POSITION)

#define ARI_MFVC_FUN_GRP_EN_POSITION 16
#define ARI_MFVC_FUN_GRP_EN_MASK (0x1 << ARI_MFVC_FUN_GRP_EN_POSITION )
#define ARI_ACS_FUN_GRP_EN_POSITION 17
#define ARI_ACS_FUN_GRP_EN_MASK (0x1 << ARI_ACS_FUN_GRP_EN_POSITION )
#define ARI_FUN_GRP_POSITION 20
#define ARI_FUN_GRP_MASK (0x7 << ARI_FUN_GRP_POSITION )

////////////////////////////////////////////
// VPD cap structure
////////////////////////////////////////////

#define VPD_PCI_EXTENDED_CAP_ID_POSITION 0
#define VPD_PCI_EXTENDED_CAP_ID_MASK ( 0xff<< VPD_PCI_EXTENDED_CAP_ID_POSITION)
#define VPD_NEXT_OFFSET_POSITION 8
#define VPD_NEXT_OFFSET_MASK ( 0xff<< VPD_NEXT_OFFSET_POSITION)
#define VPD_ADDRESS_POSITION 16
#define VPD_ADDRESS_MASK ( 0x7fff<< VPD_ADDRESS_POSITION)
#define VPD_FLAG_POSITION 31
#define VPD_FLAG_MASK ( 0x1 << VPD_FLAG_POSITION)

#define VPD_DATA_POSITION 0
#define VPD_DATA_MASK (0xffff_ffff << VPD_DATA_POSITION )

////////////////////////////////////////////
// Slot Numbering  cap structure
////////////////////////////////////////////

#define SLOTNUM_CAP_ID_POSITION 0
#define SLOTNUM_CAP_ID_MASK (0xff << SLOTNUM_CAP_ID_POSITION)
#define SLOTNUM_NEXT_POINTER_POSITION 8
#define SLOTNUM_NEXT_POINTER_MASK (0xff << SLOTNUM_NEXT_POINTER_POSITION)
#define SLOTNUM_ADD_IN_CARD_SLOT_POSITION 16
#define SLOTNUM_ADD_IN_CARD_SLOT_MASK (0x1f << SLOTNUM_ADD_IN_CARD_SLOT_POSITION)
#define SLOTNUM_FIRST_IN_CHASSIS_POSITION 21
#define SLOTNUM_FIRST_IN_CHASSIS_MASK (0x1 << SLOTNUM_FIRST_IN_CHASSIS_POSITION)
#define SLOTNUM_CHASSIS_NUM_POSITION 24
#define SLOTNUM_CHASSIS_NUM_MASK (0xff << SLOTNUM_CHASSIS_NUM_POSITION)
#define SLOTNUM_PCIE_EXTENDED_CAP_ID_POSITION 0
#define SLOTNUM_PCIE_EXTENDED_CAP_ID_MASK (0xff << SLOTNUM_PCIE_EXTENDED_CAP_ID_POSITION)

////////////////////////////////////////////
// Power Management cap structure
////////////////////////////////////////////

#define PM_CAP_ID_POSITION 0
#define PM_CAP_ID_MASK (0xff << PM_CAP_ID_POSITION)
#define PM_NEXT_POINTER_POSITION 8
#define PM_NEXT_POINTER_MASK (0xff << PM_NEXT_POINTER_POSITION)
#define PM_SPEC_VER_POSITION 16
#define PM_SPEC_VER_MASK (0x7 << PM_SPEC_VER_POSITION)
#define PME_CLK_POSITION 19
#define PME_CLK_MASK (0x1 << PME_CLK_POSITION)
#define PME_IMM_READI_RETURN_D0_POSITION 20
#define PME_IMM_READI_RETURN_D0_MASK (0x1 << PME_IMM_READI_RETURN_D0_POSITION)
#define DSI_POSITION 21
#define DSI_MASK (0x1 << DSI_POSITION)
#define AUX_CURR_POSITION 22
#define AUX_CURR_MASK (0x7 << AUX_CURR_POSITION)
#define D1_SUPPORT_POSITION 25
#define D1_SUPPORT_MASK (0x1 << D1_SUPPORT_POSITION)
#define D2_SUPPORT_POSITION 26
#define D2_SUPPORT_MASK (0x1 << D2_SUPPORT_POSITION)
#define PME_SUPPORT_POSITION 27
#define PME_SUPPORT_MASK (0x1f << PME_SUPPORT_POSITION)

#define POWER_STATE_POSITION 0
#define POWER_STATE_MASK (0x3 << POWER_STATE_POSITION)
#define NO_SOFT_RST_POSITION 3
#define NO_SOFT_RST_MASK (0x1 << NO_SOFT_RST_POSITION)
#define PME_ENABLE_POSITION 8
#define PME_ENABLE_MASK (0x1 << PME_ENABLE_POSITION)
#define DATA_SELECT_POSITION 9
#define DATA_SELECT_MASK (0xf << DATA_SELECT_POSITION)
#define DATA_SCALE_POSITION 13
#define DATA_SCALE_MASK (0x3 << DATA_SCALE_POSITION)
#define PME_STATUS_POSITION 15
#define PME_STATUS_MASK (0x1 << PME_STATUS_POSITION)

#define B2_B3_SUPPORT_POSITION 22
#define B2_B3_SUPPORT_MASK (0x1 << B2_B3_SUPPORT_POSITION)

#define BUS_PWR_CLK_CON_EN_POSITION 23
#define BUS_PWR_CLK_CON_EN_MASK (0x1 << BUS_PWR_CLK_CON_EN_POSITION)

#define DATA_REG_ADD_INFO_POSITION 24
#define DATA_REG_ADD_INFO_MASK (0xff << DATA_REG_ADD_INFO_POSITION)

////////////////////////////////////////////
// Power Budgeting cap structure
////////////////////////////////////////////

#define PB_PCIE_EXTENDED_CAP_ID_POSITION 0
#define PB_CAP_VERSION_POSITION  16
#define PB_NEXT_OFFSET_POSITION  20

#define PB_PCIE_EXTENDED_CAP_ID_MASK 0xffff
#define PB_CAP_VERSION_MASK (0xf << PB_CAP_VERSION_POSITION)
#define PB_NEXT_OFFSET_MASK ( 0xfff<< PB_NEXT_OFFSET_POSITION)

#define PB_DATA_SEL_POSITION 0
#define PB_DATA_SEL_MASK  (0xff << PB_DATA_SEL_POSITION)
#define PB_BASE_POWER_POSITION 0
#define PB_BASE_POWER_MASK ( 0xff<< PB_BASE_POWER_POSITION)
#define PB_DATA_SCALE_POSITION 8
#define PB_DATA_SCALE_MASK ( 0x3 << PB_DATA_SCALE_POSITION)
#define PB_PM_SUB_STATE_POSITION 10
#define PB_PM_SUB_STATE_MASK ( 0x7 << PB_PM_SUB_STATE_POSITION)
#define PB_PM_STATE_POSITION 13
#define PB_PM_STATE_MASK ( 0x3 << PB_PM_STATE_POSITION)
#define PB_TYPE_POSITION 15
#define PB_TYPE_MASK ( 0x7 << PB_TYPE_POSITION)
#define PB_POWER_RAIL_POSITION 18
#define PB_POWER_RAIL_MASK ( 0x7 << PB_POWER_RAIL_POSITION)

#define PB_SYS_ALLOC_POSITION 0
#define PB_SYS_ALLOC_MASK ( 0x1 << PB_SYS_ALLOC_POSITION)

///////////////////////////////////////////
// MSI Cap structure
////////////////////////////////////////////

#define  PCI_MSI_CAP_ID_POSITION 0
#define  PCI_MSI_CAP_ID_MASK (0xff << PCI_MSI_CAP_ID_POSITION)

#define  PCI_MSI_CAP_NEXT_OFFSET_POSITION 8
#define  PCI_MSI_CAP_NEXT_OFFSET_MASK (0xff << PCI_MSI_CAP_NEXT_OFFSET_POSITION)

#define  PCI_MSI_ENABLE_POSITION 16
#define  PCI_MSI_ENABLE_MASK (0x1 << PCI_MSI_ENABLE_POSITION)
#define  PCI_MSI_MULTIPLE_MSG_CAP_POSITION 17
#define  PCI_MSI_MULTIPLE_MSG_CAP_MASK (0x7 << PCI_MSI_MULTIPLE_MSG_CAP_POSITION)
#define  PCI_MSI_MULTIPLE_MSG_EN_POSITION 20
#define  PCI_MSI_MULTIPLE_MSG_EN_MASK (0x7 << PCI_MSI_MULTIPLE_MSG_EN_POSITION)
#define  PCI_MSI_64_BIT_ADDR_CAP_POSITION 23
#define  PCI_MSI_64_BIT_ADDR_CAP_MASK (0x1 << PCI_MSI_64_BIT_ADDR_CAP_POSITION)
#define  PCI_PVM_SUPPORT_POSITION 24
#define  PCI_PVM_SUPPORT_MASK (0x1 << PCI_PVM_SUPPORT_POSITION)
#define  PCI_MSI_EXT_DATA_CAP_POSITION 25
#define  PCI_MSI_EXT_DATA_CAP_MASK (0x1 << PCI_MSI_EXT_DATA_CAP_POSITION)
#define  PCI_MSI_EXT_DATA_EN_POSITION 26
#define  PCI_MSI_EXT_DATA_EN_MASK (0x1 << PCI_MSI_EXT_DATA_EN_POSITION)

#define  PCI_MSI_LOWER_32_POSITION 2
#define  PCI_MSI_LOWER_32_MASK (0x3fff_ffff << PCI_MSI_LOWER_32_POSITION)

#define  PCI_MSI_08H_OFF_POSITION 0
#define  PCI_MSI_08H_OFF_MASK (0xffff << PCI_MSI_08H_OFF_POSITION)

#define  PCI_MSI_0AH_OFF_POSITION 16
#define  PCI_MSI_0AH_OFF_MASK (0xffff << PCI_MSI_0AH_OFF_POSITION)

#define  PCI_MSI_0CH_OFF_POSITION 0
#define  PCI_MSI_0CH_OFF_MASK (0xffff << PCI_MSI_0CH_OFF_POSITION)
#define  PCI_MSI_0EH_OFF_POSITION 16
#define  PCI_MSI_0EH_OFF_MASK (0xffff << PCI_MSI_0EH_OFF_POSITION)


#define  PCI_MSI_10H_OFF_POSITION 0
#define  PCI_MSI_10H_OFF_MASK (0xffff_ffff << PCI_MSI_10H_OFF_POSITION)

#define  PCI_MSI_14H_OFF_POSITION 0
#define  PCI_MSI_14H_OFF_MASK (0xffff_ffff << PCI_MSI_14H_OFF_POSITION)

// Below macros should be obsoleted to comply with new MSI capability format
#define  PCI_MSI_UPPER_32_POSITION 0
#define  PCI_MSI_UPPER_32_MASK (0xffff_ffff << PCI_MSI_UPPER_32_POSITION)

#define  PCI_MSI_DATA_POSITION 0
#define  PCI_MSI_DATA_MASK (0xffff << PCI_MSI_DATA_POSITION)
#define  PCI_MSI_DATA_RESERVED_POSITION 16
#define  PCI_MSI_DATA_RESERVED_MASK (0xffff << PCI_MSI_DATA_RESERVED_POSITION)

#define  PCI_MSI_MASK_POSITION 0
#define  PCI_MSI_MASK_MASK (0xffff_ffff << PCI_MSI_MASK_POSITION)

#define  PCI_MSI_PENDING_BIT_POSITION 0
#define  PCI_MSI_PENDING_BIT_MASK (0xffff_ffff << PCI_MSI_PENDING_BIT_POSITION)
// End of obsolete macros


////////////////////////////////////////////
// MSI-X Cap structure
////////////////////////////////////////////

#define  PCI_MSIX_CAP_ID_POSITION 0
#define  PCI_MSIX_CAP_ID_MASK (0xff << PCI_MSIX_CAP_ID_POSITION)

#define  PCI_MSIX_CAP_NEXT_OFFSET_POSITION 8
#define  PCI_MSIX_CAP_NEXT_OFFSET_MASK (0xff << PCI_MSIX_CAP_NEXT_OFFSET_POSITION)

#define  PCI_MSIX_TABLE_SIZE_POSITION 16
#define  PCI_MSIX_TABLE_SIZE_MASK (0x7ff << PCI_MSIX_TABLE_SIZE_POSITION)
#define  PCI_MSIX_FUNCTION_MASK_POSITION 30
#define  PCI_MSIX_FUNCTION_MASK_MASK (0x1 << PCI_MSIX_FUNCTION_MASK_POSITION)
#define  PCI_MSIX_ENABLE_POSITION 31
#define  PCI_MSIX_ENABLE_MASK (0x1 << PCI_MSIX_ENABLE_POSITION)

#define  PCI_MSIX_BIR_POSITION 0
#define  PCI_MSIX_BIR_MASK (0x7 << PCI_MSIX_BIR_POSITION)
#define  PCI_MSIX_TABLE_OFFSET_POSITION 3
#define  PCI_MSIX_TABLE_OFFSET_MASK (0x1fff_ffff << PCI_MSIX_TABLE_OFFSET_POSITION)

#define  PCI_MSIX_PBA_BIR_POSITION 0
#define  PCI_MSIX_PBA_BIR_MASK (0x7 << PCI_MSIX_PBA_BIR_POSITION)
#define  PCI_MSIX_PBA_OFFSET_POSITION 3
#define  PCI_MSIX_PBA_OFFSET_MASK (0x1fff_ffff << PCI_MSIX_PBA_OFFSET_POSITION)

////////////////////////////////////////////
// SATA Cap structure
////////////////////////////////////////////

#define  SATA_CAP_ID_POSITION 0
#define  SATA_CAP_ID_MASK (0xff << SATA_CAP_ID_POSITION)
#define  SATA_NEXT_OFFSET_POSITION 8
#define  SATA_NEXT_OFFSET_MASK (0xff << SATA_NEXT_OFFSET_POSITION)
#define  SATA_MINREV_POSITION 16
#define  SATA_MINREV_MASK (0xf << SATA_MINREV_POSITION)
#define  SATA_MAJREV_POSITION 20
#define  SATA_MAJREV_MASK (0xf << SATA_MAJREV_POSITION)

#define  SATA_BARLOC_POSITION 0
#define  SATA_BARLOC_MASK (0xf << SATA_BARLOC_POSITION)
#define  SATA_BAROFST_POSITION 4
#define  SATA_BAROFST_MASK (0xfffff << SATA_BAROFST_POSITION)

/////////////////////////////////////////////
// Vendor-Specific Extended Capability - RASD
/////////////////////////////////////////////
#define VSECRAS_ID_POSITION 0
#define VSECRAS_ID_MASK (0xffff << VSECRAS_ID_POSITION)
#define VSECRAS_CAP_POSITION 16
#define VSECRAS_CAP_MASK (0xf << VSECRAS_CAP_POSITION)
#define VSECRAS_NEXT_OFFSET_POSITION 20
#define VSECRAS_NEXT_OFFSET_MASK (0xfff << VSECRAS_NEXT_OFFSET_POSITION)

#define VSECRAS_VSEC_ID_POSITION 0
#define VSECRAS_VSEC_ID_MASK (0xffff << VSECRAS_VSEC_ID_POSITION)
#define VSECRAS_VSEC_REV_POSITION 16
#define VSECRAS_VSEC_REV_MASK (0xf << VSECRAS_VSEC_REV_POSITION)
#define VSECRAS_VSEC_LENGTH_POSITION 20
#define VSECRAS_VSEC_LENGTH_MASK (0xfff << VSECRAS_VSEC_LENGTH_POSITION)

#define VSECRAS_ERROR_PROT_DISABLE_TX_POSITION 0
#define VSECRAS_ERROR_PROT_DISABLE_TX_MASK (0x1 << VSECRAS_ERROR_PROT_DISABLE_TX_POSITION)
#define VSECRAS_ERROR_PROT_DISABLE_AXI_BRIDGE_MASTER_POSITION 1
#define VSECRAS_ERROR_PROT_DISABLE_AXI_BRIDGE_MASTER_MASK (0x1 << VSECRAS_ERROR_PROT_DISABLE_AXI_BRIDGE_MASTER_POSITION)
#define VSECRAS_ERROR_PROT_DISABLE_AXI_BRIDGE_OUTBOUND_POSITION 2
#define VSECRAS_ERROR_PROT_DISABLE_AXI_BRIDGE_OUTBOUND_MASK (0x1 << VSECRAS_ERROR_PROT_DISABLE_AXI_BRIDGE_OUTBOUND_POSITION)
#define VSECRAS_ERROR_PROT_DISABLE_DMA_WRITE_POSITION 3
#define VSECRAS_ERROR_PROT_DISABLE_DMA_WRITE_MASK (0x1 << VSECRAS_ERROR_PROT_DISABLE_DMA_WRITE_POSITION)
#define VSECRAS_ERROR_PROT_DISABLE_LAYER2_TX_POSITION 4
#define VSECRAS_ERROR_PROT_DISABLE_LAYER2_TX_MASK (0x1 << VSECRAS_ERROR_PROT_DISABLE_LAYER2_TX_POSITION)
#define VSECRAS_ERROR_PROT_DISABLE_LAYER3_TX_POSITION 5
#define VSECRAS_ERROR_PROT_DISABLE_LAYER3_TX_MASK (0x1 << VSECRAS_ERROR_PROT_DISABLE_LAYER3_TX_POSITION)
#define VSECRAS_ERROR_PROT_DISABLE_ADM_TX_POSITION 6
#define VSECRAS_ERROR_PROT_DISABLE_ADM_TX_MASK (0x1 << VSECRAS_ERROR_PROT_DISABLE_ADM_TX_POSITION)
#define VSECRAS_ERROR_PROT_DISABLE_RX_POSITION 16
#define VSECRAS_ERROR_PROT_DISABLE_RX_MASK (0x1 << VSECRAS_ERROR_PROT_DISABLE_RX_POSITION)
#define VSECRAS_ERROR_PROT_DISABLE_AXI_BRIDGE_INBOUND_COMPLETION_POSITION 17
#define VSECRAS_ERROR_PROT_DISABLE_AXI_BRIDGE_INBOUND_COMPLETION_MASK (0x1 << VSECRAS_ERROR_PROT_DISABLE_AXI_BRIDGE_INBOUND_COMPLETION_POSITION)
#define VSECRAS_ERROR_PROT_DISABLE_AXI_BRIDGE_INBOUND_REQUEST_POSITION 18
#define VSECRAS_ERROR_PROT_DISABLE_AXI_BRIDGE_INBOUND_REQUEST_MASK (0x1 << VSECRAS_ERROR_PROT_DISABLE_AXI_BRIDGE_INBOUND_REQUEST_POSITION)
#define VSECRAS_ERROR_PROT_DISABLE_DMA_READ_POSITION 19
#define VSECRAS_ERROR_PROT_DISABLE_DMA_READ_MASK (0x1 << VSECRAS_ERROR_PROT_DISABLE_DMA_READ_POSITION)
#define VSECRAS_ERROR_PROT_DISABLE_LAYER2_RX_POSITION 20
#define VSECRAS_ERROR_PROT_DISABLE_LAYER2_RX_MASK (0x1 << VSECRAS_ERROR_PROT_DISABLE_LAYER2_RX_POSITION)
#define VSECRAS_ERROR_PROT_DISABLE_LAYER3_RX_POSITION 21
#define VSECRAS_ERROR_PROT_DISABLE_LAYER3_RX_MASK (0x1 << VSECRAS_ERROR_PROT_DISABLE_LAYER3_RX_POSITION)
#define VSECRAS_ERROR_PROT_DISABLE_ADM_RX_POSITION 22
#define VSECRAS_ERROR_PROT_DISABLE_ADM_RX_MASK (0x1 << VSECRAS_ERROR_PROT_DISABLE_ADM_RX_POSITION)

#define VSECRAS_CORR_CLEAR_COUNTERS_POSITION 0
#define VSECRAS_CORR_CLEAR_COUNTERS_MASK (0x1 << VSECRAS_CORR_CLEAR_COUNTERS_POSITION)
#define VSECRAS_CORR_EN_COUNTERS_POSITION 4
#define VSECRAS_CORR_EN_COUNTERS_MASK (0x1 << VSECRAS_CORR_EN_COUNTERS_POSITION)
#define VSECRAS_CORR_COUNTER_SELECTION_REGION_POSITION 20
#define VSECRAS_CORR_COUNTER_SELECTION_REGION_MASK (0xf << VSECRAS_CORR_COUNTER_SELECTION_REGION_POSITION)
#define VSECRAS_CORR_COUNTER_SELECTION_POSITION 24
#define VSECRAS_CORR_COUNTER_SELECTION_MASK (0xff << VSECRAS_CORR_COUNTER_SELECTION_POSITION)

#define VSECRAS_CORR_COUNTER_POSITION 0
#define VSECRAS_CORR_COUNTER_MASK ( 0xff<< VSECRAS_CORR_COUNTER_POSITION)
#define VSECRAS_CORR_COUNTER_SELECTED_REGION_POSITION 20
#define VSECRAS_CORR_COUNTER_SELECTED_REGION_MASK (0xf << VSECRAS_CORR_COUNTER_SELECTED_REGION_POSITION)
#define VSECRAS_CORR_COUNTER_SELECTED_POSITION 24
#define VSECRAS_CORR_COUNTER_SELECTED_MASK (0xff << VSECRAS_CORR_COUNTER_SELECTED_POSITION)

#define VSECRAS_UNCORR_CLEAR_COUNTERS_POSITION 0
#define VSECRAS_UNCORR_CLEAR_COUNTERS_MASK (0x1 << VSECRAS_UNCORR_CLEAR_COUNTERS_POSITION)
#define VSECRAS_UNCORR_EN_COUNTERS_POSITION 4
#define VSECRAS_UNCORR_EN_COUNTERS_MASK (0x1 << VSECRAS_UNCORR_EN_COUNTERS_POSITION)
#define VSECRAS_UNCORR_COUNTER_SELECTION_REGION_POSITION 20
#define VSECRAS_UNCORR_COUNTER_SELECTION_REGION_MASK (0xf << VSECRAS_UNCORR_COUNTER_SELECTION_REGION_POSITION)
#define VSECRAS_UNCORR_COUNTER_SELECTION_POSITION 24
#define VSECRAS_UNCORR_COUNTER_SELECTION_MASK (0xff << VSECRAS_UNCORR_COUNTER_SELECTION_POSITION)

#define VSECRAS_UNCORR_COUNTER_POSITION 0
#define VSECRAS_UNCORR_COUNTER_MASK ( 0xff<< VSECRAS_UNCORR_COUNTER_POSITION)
#define VSECRAS_UNCORR_COUNTER_SELECTED_REGION_POSITION 20
#define VSECRAS_UNCORR_COUNTER_SELECTED_REGION_MASK (0xf << VSECRAS_UNCORR_COUNTER_SELECTED_REGION_POSITION)
#define VSECRAS_UNCORR_COUNTER_SELECTED_POSITION 24
#define VSECRAS_UNCORR_COUNTER_SELECTED_MASK (0xff << VSECRAS_UNCORR_COUNTER_SELECTED_POSITION)

#define VSECRAS_ERROR_INJ_EN_POSITION 0
#define VSECRAS_ERROR_INJ_EN_MASK (0x1 << VSECRAS_ERROR_INJ_EN_POSITION)
#define VSECRAS_ERROR_INJ_TYPE_POSITION 4
#define VSECRAS_ERROR_INJ_TYPE_MASK (0x3 << VSECRAS_ERROR_INJ_TYPE_POSITION)
#define VSECRAS_ERROR_INJ_COUNT_POSITION 8
#define VSECRAS_ERROR_INJ_COUNT_MASK (0xff << VSECRAS_ERROR_INJ_COUNT_POSITION)
#define VSECRAS_ERROR_INJ_LOC_POSITION 16
#define VSECRAS_ERROR_INJ_LOC_MASK (0xff << VSECRAS_ERROR_INJ_LOC_POSITION)

#define VSECRAS_REG_FIRST_CORR_ERROR_POSITION 4
#define VSECRAS_REG_FIRST_CORR_ERROR_MASK (0xf << VSECRAS_REG_FIRST_CORR_ERROR_POSITION)
#define VSECRAS_LOC_FIRST_CORR_ERROR_POSITION 8
#define VSECRAS_LOC_FIRST_CORR_ERROR_MASK (0xff << VSECRAS_LOC_FIRST_CORR_ERROR_POSITION)
#define VSECRAS_REG_LAST_CORR_ERROR_POSITION 20
#define VSECRAS_REG_LAST_CORR_ERROR_MASK (0xf << VSECRAS_REG_LAST_CORR_ERROR_POSITION)
#define VSECRAS_LOC_LAST_CORR_ERROR_POSITION 24
#define VSECRAS_LOC_LAST_CORR_ERROR_MASK (0xff << VSECRAS_LOC_LAST_CORR_ERROR_POSITION)

#define VSECRAS_REG_FIRST_UNCORR_ERROR_POSITION 4
#define VSECRAS_REG_FIRST_UNCORR_ERROR_MASK (0xf << VSECRAS_REG_FIRST_UNCORR_ERROR_POSITION)
#define VSECRAS_LOC_FIRST_UNCORR_ERROR_POSITION 8
#define VSECRAS_LOC_FIRST_UNCORR_ERROR_MASK (0xff << VSECRAS_LOC_FIRST_UNCORR_ERROR_POSITION)
#define VSECRAS_REG_LAST_UNCORR_ERROR_POSITION 20
#define VSECRAS_REG_LAST_UNCORR_ERROR_MASK (0xf << VSECRAS_REG_LAST_UNCORR_ERROR_POSITION)
#define VSECRAS_LOC_LAST_UNCORR_ERROR_POSITION 24
#define VSECRAS_LOC_LAST_UNCORR_ERROR_MASK (0xff << VSECRAS_LOC_LAST_UNCORR_ERROR_POSITION)

#define VSECRAS_ERROR_MODE_EN_POSITION 0
#define VSECRAS_ERROR_MODE_EN_MASK (0x1 << VSECRAS_ERROR_MODE_EN_POSITION)
#define VSECRAS_AUTO_LINK_DOWN_EN_POSITION 1
#define VSECRAS_AUTO_LINK_DOWN_EN_MASK (0x1 << VSECRAS_AUTO_LINK_DOWN_EN_POSITION)

#define VSECRAS_ERROR_MODE_CLEAR_POSITION 0
#define VSECRAS_ERROR_MODE_CLEAR_MASK (0x1 << VSECRAS_ERROR_MODE_CLEAR_POSITION)

#define VSECRAS_RAM_ADDR_CORR_ERROR_POSITION 0
#define VSECRAS_RAM_ADDR_CORR_ERROR_MASK (0x7ffffff << VSECRAS_RAM_ADDR_CORR_ERROR_POSITION)
#define VSECRAS_RAM_INDEX_CORR_ERROR_POSITION 28
#define VSECRAS_RAM_INDEX_CORR_ERROR_MASK (0xF << VSECRAS_RAM_INDEX_CORR_ERROR_POSITION)

#define VSECRAS_RAM_ADDR_UNCORR_ERROR_POSITION 0
#define VSECRAS_RAM_ADDR_UNCORR_ERROR_MASK (0x7ffffff << VSECRAS_RAM_ADDR_UNCORR_ERROR_POSITION)
#define VSECRAS_RAM_INDEX_UNCORR_ERROR_POSITION 28
#define VSECRAS_RAM_INDEX_UNCORR_ERROR_MASK (0xF << VSECRAS_RAM_INDEX_UNCORR_ERROR_POSITION)

////////////////////////////////////////////////////////
// PTM Vendor Specific Extended Capability Structure
////////////////////////////////////////////////////////
// PCIe Extended Capability Header
#define  PTM_CAP_ID_POSITION 0
#define  PTM_CAP_ID_MASK (0xffff<<PTM_CAP_ID_POSITION)
#define  PTM_CAP_VERSION_POSITION 16
#define  PTM_CAP_VERSION_MASK (0xf<<PTM_CAP_VERSION_POSITION)
#define  PTM_NEXT_OFFSET_POSITION 20
#define  PTM_NEXT_OFFSET_MASK (0xfff<<PTM_NEXT_OFFSET_POSITION)

// PTM Capability Register
#define PTM_REQ_CAPABLE_POSITION 0
#define PTM_REQ_CAPABLE_MASK (0x1<<PTM_REQ_CAPABLE_POSITION)
#define PTM_RES_CAPABLE_POSITION 1
#define PTM_RES_CAPABLE_MASK (0x1<<PTM_RES_CAPABLE_POSITION)
#define PTM_ROOT_CAPABLE_POSITION 2
#define PTM_ROOT_CAPABLE_MASK (0x1<<PTM_ROOT_CAPABLE_POSITION)
#define PTM_CLK_GRAN_POSITION 8
#define PTM_CLK_GRAN_MASK (0xff<<PTM_CLK_GRAN_POSITION)

// PTM Control Register
#define PTM_ENABLE_POSITION 0
#define PTM_ENABLE_MASK (0x1<<PTM_ENABLE_POSITION)
#define ROOT_SELECT_POSITION 1
#define ROOT_SELECT_MASK (0x1<<ROOT_SELECT_POSITION)
#define EFF_GRAN_POSITION 8
#define EFF_GRAN_MASK (0xff<<EFF_GRAN_POSITION)

// Requester Extended Capability Header
#define PTM_REQ_EXT_CAP_ID_POSITION 0
#define PTM_REQ_EXT_CAP_ID_MASK (0xffff << PTM_REQ_EXT_CAP_ID_POSITION)
#define PTM_REQ_EXT_CAP_VER_POSITION 16
#define PTM_REQ_EXT_CAP_VER_MASK (0xf << PTM_REQ_EXT_CAP_VER_POSITION)
#define PTM_REQ_EXT_CAP_NEXT_OFFS_POSITION 20
#define PTM_REQ_EXT_CAP_NEXT_OFFS_MASK (0xfff << PTM_REQ_EXT_CAP_NEXT_OFFS_POSITION)

// Requester Capability Header Fields
#define  PTM_REQ_VSEC_ID_POSITION 0
#define  PTM_REQ_VSEC_ID_MASK (0xffff<<PTM_REQ_VSEC_ID_POSITION)
#define  PTM_REQ_VSEC_REV_POSITION 16
#define  PTM_REQ_VSEC_REV_MASK (0xf<<PTM_REQ_VSEC_REV_POSITION)
#define  PTM_REQ_VSEC_LENGTH_POSITION 20
#define  PTM_REQ_VSEC_LENGTH_MASK (0xfff<<PTM_REQ_VSEC_LENGTH_POSITION)

// Requester Control Register Fields
#define  PTM_REQ_AUTO_UPDATE_ENABLED_POSITION 0
#define  PTM_REQ_AUTO_UPDATE_ENABLED_MASK (0x1<<PTM_REQ_AUTO_UPDATE_ENABLED_POSITION)
#define  PTM_REQ_START_UPDATE_POSITION 1
#define  PTM_REQ_START_UPDATE_MASK (0x1<<PTM_REQ_START_UPDATE_POSITION)
#define  PTM_REQ_FAST_TIMERS_POSITION 2
#define  PTM_REQ_FAST_TIMERS_MASK (0x1<<PTM_REQ_FAST_TIMERS_POSITION)
#define  PTM_REQ_LONG_TIMER_POSITION 8
#define  PTM_REQ_LONG_TIMER_MASK (0xff<<PTM_REQ_LONG_TIMER_POSITION)

// Requester Status Register Fields
#define  PTM_REQ_CONTEXT_VALID_POSITION 0
#define  PTM_REQ_CONTEXT_VALID_MASK (0x1<<PTM_REQ_CONTEXT_VALID_POSITION)
#define  PTM_REQ_MANUAL_UPDATE_ALLOWED_POSITION 1
#define  PTM_REQ_MANUAL_UPDATE_ALLOWED_MASK (0x1<<PTM_REQ_MANUAL_UPDATE_ALLOWED_POSITION)

// Requester Register wide Fields
#define  PTM_REQ_LOCAL_LSB_POSITION 0
#define  PTM_REQ_LOCAL_LSB_MASK (0xffff_ffff<<PTM_REQ_LOCAL_LSB_POSITION)
#define  PTM_REQ_LOCAL_MSB_POSITION 0
#define  PTM_REQ_LOCAL_MSB_MASK (0xffff_ffff<<PTM_REQ_LOCAL_MSB_POSITION)
#define  PTM_REQ_T1_LSB_POSITION 0
#define  PTM_REQ_T1_LSB_MASK (0xffff_ffff<<PTM_REQ_T1_LSB_POSITION)
#define  PTM_REQ_T1_MSB_POSITION 0
#define  PTM_REQ_T1_MSB_MASK (0xffff_ffff<<PTM_REQ_T1_MSB_POSITION)
#define  PTM_REQ_T1P_LSB_POSITION 0
#define  PTM_REQ_T1P_LSB_MASK (0xffff_ffff<<PTM_REQ_T1P_LSB_POSITION)
#define  PTM_REQ_T1P_MSB_POSITION 0
#define  PTM_REQ_T1P_MSB_MASK (0xffff_ffff<<PTM_REQ_T1P_MSB_POSITION)
#define  PTM_REQ_T4_LSB_POSITION 0
#define  PTM_REQ_T4_LSB_MASK (0xffff_ffff<<PTM_REQ_T4_LSB_POSITION)
#define  PTM_REQ_T4_MSB_POSITION 0
#define  PTM_REQ_T4_MSB_MASK (0xffff_ffff<<PTM_REQ_T4_MSB_POSITION)
#define  PTM_REQ_T4P_LSB_POSITION 0
#define  PTM_REQ_T4P_LSB_MASK (0xffff_ffff<<PTM_REQ_T4P_LSB_POSITION)
#define  PTM_REQ_T4P_MSB_POSITION 0
#define  PTM_REQ_T4P_MSB_MASK (0xffff_ffff<<PTM_REQ_T4P_MSB_POSITION)
#define  PTM_REQ_MASTER_LSB_POSITION 0
#define  PTM_REQ_MASTER_LSB_MASK (0xffff_ffff<<PTM_REQ_MASTER_LSB_POSITION)
#define  PTM_REQ_MASTER_MSB_POSITION 0
#define  PTM_REQ_MASTER_MSB_MASK (0xffff_ffff<<PTM_REQ_MASTER_MSB_POSITION)
#define  PTM_REQ_PROP_DELAY_POSITION 0
#define  PTM_REQ_PROP_DELAY_MASK (0xffff_ffff<<PTM_REQ_PROP_DELAY_POSITION)
#define  PTM_REQ_MASTERT1_LSB_POSITION 0
#define  PTM_REQ_MASTERT1_LSB_MASK (0xffff_ffff<<PTM_REQ_MASTERT1_LSB_POSITION)
#define  PTM_REQ_MASTERT1_MSB_POSITION 0
#define  PTM_REQ_MASTERT1_MSB_MASK (0xffff_ffff<<PTM_REQ_MASTERT1_MSB_POSITION)
#define  PTM_REQ_TX_LATENCY_POSITION 0
#define  PTM_REQ_TX_LATENCY_MASK (0x0000_0fff<<PTM_REQ_TX_LATENCY_POSITION)
#define  PTM_REQ_RX_LATENCY_POSITION 0
#define  PTM_REQ_RX_LATENCY_MASK (0x0000_0fff<<PTM_REQ_RX_LATENCY_POSITION)

// Responder Extended Capability Header
#define PTM_RES_EXT_CAP_ID_POSITION 0
#define PTM_RES_EXT_CAP_ID_MASK (0xffff << PTM_RES_EXT_CAP_ID_POSITION)
#define PTM_RES_EXT_CAP_VER_POSITION 16
#define PTM_RES_EXT_CAP_VER_MASK (0xf << PTM_RES_EXT_CAP_VER_POSITION)
#define PTM_RES_EXT_CAP_NEXT_OFFS_POSITION 20
#define PTM_RES_EXT_CAP_NEXT_OFFS_MASK (0xfff << PTM_RES_EXT_CAP_NEXT_OFFS_POSITION)

// Responder Capability Header Fields
#define  PTM_RES_VSEC_ID_POSITION 0
#define  PTM_RES_VSEC_ID_MASK (0xffff<<PTM_RES_VSEC_ID_POSITION)
#define  PTM_RES_VSEC_REV_POSITION 16
#define  PTM_RES_VSEC_REV_MASK (0xf<<PTM_RES_VSEC_REV_POSITION)
#define  PTM_RES_VSEC_LENGTH_POSITION 20
#define  PTM_RES_VSEC_LENGTH_MASK (0xfff<<PTM_RES_VSEC_LENGTH_POSITION)

// Responder Control Register Fields
#define  PTM_RES_CCONTEXT_VALID_POSITION 0
#define  PTM_RES_CCONTEXT_VALID_MASK (0x1<<PTM_RES_CCONTEXT_VALID_POSITION)

// Responder Status Register Fields
#define  PTM_RES_SCONTEXT_VALID_POSITION 0
#define  PTM_RES_SCONTEXT_VALID_MASK (0x1<<PTM_RES_SCONTEXT_VALID_POSITION)
#define  PTM_RES_REQUEST_RECEIVED_POSITION 1
#define  PTM_RES_REQUEST_RECEIVED_MASK (0x1<<PTM_RES_REQUEST_RECEIVED_POSITION)

// Responder Register wide fields
#define  PTM_RES_LOCAL_LSB_POSITION 0
#define  PTM_RES_LOCAL_LSB_MASK (0xffff_ffff<<PTM_RES_LOCAL_LSB_POSITION)
#define  PTM_RES_LOCAL_MSB_POSITION 0
#define  PTM_RES_LOCAL_MSB_MASK (0xffff_ffff<<PTM_RES_LOCAL_MSB_POSITION)
#define  PTM_RES_T2_LSB_POSITION 0
#define  PTM_RES_T2_LSB_MASK (0xffff_ffff<<PTM_RES_T2_LSB_POSITION)
#define  PTM_RES_T2_MSB_POSITION 0
#define  PTM_RES_T2_MSB_MASK (0xffff_ffff<<PTM_RES_T2_MSB_POSITION)
#define  PTM_RES_T2P_LSB_POSITION 0
#define  PTM_RES_T2P_LSB_MASK (0xffff_ffff<<PTM_RES_T2P_LSB_POSITION)
#define  PTM_RES_T2P_MSB_POSITION 0
#define  PTM_RES_T2P_MSB_MASK (0xffff_ffff<<PTM_RES_T2P_MSB_POSITION)
#define  PTM_RES_T3_LSB_POSITION 0
#define  PTM_RES_T3_LSB_MASK (0xffff_ffff<<PTM_RES_T3_LSB_POSITION)
#define  PTM_RES_T3_MSB_POSITION 0
#define  PTM_RES_T3_MSB_MASK (0xffff_ffff<<PTM_RES_T3_MSB_POSITION)
#define  PTM_RES_T3P_LSB_POSITION 0
#define  PTM_RES_T3P_LSB_MASK (0xffff_ffff<<PTM_RES_T3P_LSB_POSITION)
#define  PTM_RES_T3P_MSB_POSITION 0
#define  PTM_RES_T3P_MSB_MASK (0xffff_ffff<<PTM_RES_T3P_MSB_POSITION)
#define  PTM_RES_TX_LATENCY_POSITION 0
#define  PTM_RES_TX_LATENCY_MASK (0x0000_0fff<<PTM_RES_TX_LATENCY_POSITION)
#define  PTM_RES_RX_LATENCY_POSITION 0
#define  PTM_RES_RX_LATENCY_MASK (0x0000_0fff<<PTM_RES_RX_LATENCY_POSITION)

//DATA_LINK_FEATURE_EXTENDED_HDR_OFF
#define DLINK_CAP_DLINK_EXT_CAP_ID_POSITION 0
#define DLINK_CAP_DLINK_EXT_CAP_ID_MASK (0xffff << DLINK_CAP_DLINK_EXT_CAP_ID_POSITION)
#define DLINK_CAP_DLINK_CAP_VERSION_POSITION 16
#define DLINK_CAP_DLINK_CAP_VERSION_MASK (0xf << DLINK_CAP_DLINK_CAP_VERSION_POSITION)
#define DLINK_CAP_DLINK_NEXT_OFFSET_POSITION 20
#define DLINK_CAP_DLINK_NEXT_OFFSET_MASK (0xfff << DLINK_CAP_DLINK_NEXT_OFFSET_POSITION)
//DATA_LINK_FEATURE_CAP
#define DLINK_CAP_SCALED_FLOW_CNTL_SUPPORTED_POSITION 0
#define DLINK_CAP_SCALED_FLOW_CNTL_SUPPORTED_MASK (0x1 << DLINK_CAP_SCALED_FLOW_CNTL_SUPPORTED_POSITION)
#define DLINK_CAP_FUTURE_FEATURE_SUPPORTED_POSITION 1
#define DLINK_CAP_FUTURE_FEATURE_SUPPORTED_MASK (0x3fffff << DLINK_CAP_FUTURE_FEATURE_SUPPORTED_POSITION)
#define DLINK_CAP_DL_FEATURE_EXCHANGE_EN_POSITION 31
#define DLINK_CAP_DL_FEATURE_EXCHANGE_EN_MASK (0x1 << DLINK_CAP_DL_FEATURE_EXCHANGE_EN_POSITION)
//DATA_LINK_FEATURE_STATUS
#define DLINK_CAP_REMOTE_DATA_LINK_FEATURE_SUPPORTED_POSITION 0
#define DLINK_CAP_REMOTE_DATA_LINK_FEATURE_SUPPORTED_MASK (0x7fffff << DLINK_CAP_REMOTE_DATA_LINK_FEATURE_SUPPORTED_POSITION)
#define DLINK_CAP_DATA_LINK_FEATURE_STATUS_VALID_POSITION 31
#define DLINK_CAP_DATA_LINK_FEATURE_STATUS_VALID_MASK (0x1 << DLINK_CAP_DATA_LINK_FEATURE_STATUS_VALID_POSITION)
//##DMA Unroll ##################################################################
#define  DMA_RTRGT1_WEIGHT_POSITION 0
#define  DMA_RTRGT1_WEIGHT_MASK (0x7 << DMA_RTRGT1_WEIGHT_POSITION)
#define  DMA_WR_CTRL_TRGT_WEIGHT_POSITION 3
#define  DMA_WR_CTRL_TRGT_WEIGHT_MASK (0x7 << DMA_WR_CTRL_TRGT_WEIGHT_POSITION)
#define  DMA_RD_CTRL_TRGT_WEIGHT_POSITION 6
#define  DMA_RD_CTRL_TRGT_WEIGHT_MASK (0x7 << DMA_RD_CTRL_TRGT_WEIGHT_POSITION)
#define  DMA_RDBUFF_TRGT_WEIGHT_POSITION 9
#define  DMA_RDBUFF_TRGT_WEIGHT_MASK (0x7 << DMA_RDBUFF_TRGT_WEIGHT_POSITION)
#define  DMA_NUM_DMA_WR_CHAN_POSITION 0
#define  DMA_NUM_DMA_WR_CHAN_MASK (0xF << DMA_NUM_DMA_WR_CHAN_POSITION)
#define  DMA_NUM_DMA_RD_CHAN_POSITION 16
#define  DMA_NUM_DMA_RD_CHAN_MASK (0xF << DMA_NUM_DMA_RD_CHAN_POSITION)
#define  DMA_DIS_C2W_CACHE_WR_POSITION 24
#define  DMA_DIS_C2W_CACHE_WR_MASK (0x1 << DMA_DIS_C2W_CACHE_WR_POSITION)
#define  DMA_DIS_C2W_CACHE_RD_POSITION 25
#define  DMA_DIS_C2W_CACHE_RD_MASK (0x1 << DMA_DIS_C2W_CACHE_RD_POSITION)
#define  DMA_WRITE_ENGINE_POSITION 0
#define  DMA_WRITE_ENGINE_MASK      (0x1 << DMA_WRITE_ENGINE_POSITION)
#define  DMA_WRITE_ENGINE_EN_HSHAKE_CH0_POSITION 16
#define  DMA_WRITE_ENGINE_EN_HSHAKE_CH0_MASK     (0x1 << DMA_WRITE_ENGINE_EN_HSHAKE_CH0_POSITION)
#define  DMA_WRITE_ENGINE_EN_HSHAKE_CH1_POSITION 17
#define  DMA_WRITE_ENGINE_EN_HSHAKE_CH1_MASK     (0x1 << DMA_WRITE_ENGINE_EN_HSHAKE_CH1_POSITION)
#define  DMA_WRITE_ENGINE_EN_HSHAKE_CH2_POSITION 18
#define  DMA_WRITE_ENGINE_EN_HSHAKE_CH2_MASK     (0x1 << DMA_WRITE_ENGINE_EN_HSHAKE_CH2_POSITION)
#define  DMA_WRITE_ENGINE_EN_HSHAKE_CH3_POSITION 19
#define  DMA_WRITE_ENGINE_EN_HSHAKE_CH3_MASK     (0x1 << DMA_WRITE_ENGINE_EN_HSHAKE_CH3_POSITION)
#define  DMA_WRITE_ENGINE_EN_HSHAKE_CH4_POSITION 20
#define  DMA_WRITE_ENGINE_EN_HSHAKE_CH4_MASK     (0x1 << DMA_WRITE_ENGINE_EN_HSHAKE_CH4_POSITION)
#define  DMA_WRITE_ENGINE_EN_HSHAKE_CH5_POSITION 21
#define  DMA_WRITE_ENGINE_EN_HSHAKE_CH5_MASK     (0x1 << DMA_WRITE_ENGINE_EN_HSHAKE_CH5_POSITION)
#define  DMA_WRITE_ENGINE_EN_HSHAKE_CH6_POSITION 22
#define  DMA_WRITE_ENGINE_EN_HSHAKE_CH6_MASK     (0x1 << DMA_WRITE_ENGINE_EN_HSHAKE_CH6_POSITION)
#define  DMA_WRITE_ENGINE_EN_HSHAKE_CH7_POSITION 23
#define  DMA_WRITE_ENGINE_EN_HSHAKE_CH7_MASK     (0x1 << DMA_WRITE_ENGINE_EN_HSHAKE_CH7_POSITION)
#define  DMA_WR_DOORBELL_NUM_POSITION 0
#define  DMA_WR_DOORBELL_NUM_MASK      (0x7 << DMA_WR_DOORBELL_NUM_POSITION)
#define  DMA_WR_STOP_POSITION 31
#define  DMA_WR_STOP_MASK       (0x1 << DMA_WR_STOP_POSITION)
#define  DMA_WR_P_TIMER_LIMIT_POSITION 0
#define  DMA_WR_P_TIMER_LIMIT_MASK (0xFFF << DMA_WR_P_TIMER_LIMIT_POSITION)
#define  DMA_WR_P_TIMER_DISABLE_POSITION 15
#define  DMA_WR_P_TIMER_DISABLE_MASK (0x1 << DMA_WR_P_TIMER_DISABLE_POSITION)
#define  DMA_WRITE_CHANNEL0_WEIGHT_POSITION 0
#define  DMA_WRITE_CHANNEL0_WEIGHT_MASK (0x1f << DMA_WRITE_CHANNEL0_WEIGHT_POSITION)
#define  DMA_WRITE_CHANNEL1_WEIGHT_POSITION 5
#define  DMA_WRITE_CHANNEL1_WEIGHT_MASK (0x1f << DMA_WRITE_CHANNEL1_WEIGHT_POSITION)
#define  DMA_WRITE_CHANNEL2_WEIGHT_POSITION 10
#define  DMA_WRITE_CHANNEL2_WEIGHT_MASK  (0x1f << DMA_WRITE_CHANNEL2_WEIGHT_POSITION)
#define  DMA_WRITE_CHANNEL3_WEIGHT_POSITION 15
#define  DMA_WRITE_CHANNEL3_WEIGHT_MASK  (0x1f << DMA_WRITE_CHANNEL3_WEIGHT_POSITION)
#define  DMA_WRITE_CHANNEL4_WEIGHT_POSITION 0
#define  DMA_WRITE_CHANNEL4_WEIGHT_MASK (0x1f << DMA_WRITE_CHANNEL4_WEIGHT_POSITION)
#define  DMA_WRITE_CHANNEL5_WEIGHT_POSITION 5
#define  DMA_WRITE_CHANNEL5_WEIGHT_MASK (0x1f << DMA_WRITE_CHANNEL5_WEIGHT_POSITION)
#define  DMA_WRITE_CHANNEL6_WEIGHT_POSITION 10
#define  DMA_WRITE_CHANNEL6_WEIGHT_MASK (0x1f << DMA_WRITE_CHANNEL6_WEIGHT_POSITION)
#define  DMA_WRITE_CHANNEL7_WEIGHT_POSITION 15
#define  DMA_WRITE_CHANNEL7_WEIGHT_MASK (0x1f << DMA_WRITE_CHANNEL7_WEIGHT_POSITION)
#define  DMA_WRITE_ENGINE_CHGROUP_START_POSITION 0
#define  DMA_WRITE_ENGINE_CHGROUP_START_MASK (0xff << DMA_WRITE_ENGINE_CHGROUP_START_POSITION)
#define  DMA_WRITE_ENGINE_CHGROUP_END_POSITION 8
#define  DMA_WRITE_ENGINE_CHGROUP_END_MASK (0xff << DMA_WRITE_ENGINE_CHGROUP_END_POSITION)
#define  DMA_WRITE_ENGINE_CHGROUP_ENABLE_POSITION 16
#define  DMA_WRITE_ENGINE_CHGROUP_ENABLE_MASK (0x1 << DMA_WRITE_ENGINE_CHGROUP_ENABLE_POSITION)
#define  DMA_READ_ENGINE_POSITION 0
#define  DMA_READ_ENGINE_MASK      (0x1 << DMA_READ_ENGINE_POSITION)
#define  DMA_READ_ENGINE_EN_HSHAKE_CH0_POSITION 16
#define  DMA_READ_ENGINE_EN_HSHAKE_CH0_MASK     (0x1 << DMA_READ_ENGINE_EN_HSHAKE_CH0_POSITION)
#define  DMA_READ_ENGINE_EN_HSHAKE_CH1_POSITION 17
#define  DMA_READ_ENGINE_EN_HSHAKE_CH1_MASK     (0x1 << DMA_READ_ENGINE_EN_HSHAKE_CH1_POSITION)
#define  DMA_READ_ENGINE_EN_HSHAKE_CH2_POSITION 18
#define  DMA_READ_ENGINE_EN_HSHAKE_CH2_MASK     (0x1 << DMA_READ_ENGINE_EN_HSHAKE_CH2_POSITION)
#define  DMA_READ_ENGINE_EN_HSHAKE_CH3_POSITION 19
#define  DMA_READ_ENGINE_EN_HSHAKE_CH3_MASK     (0x1 << DMA_READ_ENGINE_EN_HSHAKE_CH3_POSITION)
#define  DMA_READ_ENGINE_EN_HSHAKE_CH4_POSITION 20
#define  DMA_READ_ENGINE_EN_HSHAKE_CH4_MASK     (0x1 << DMA_READ_ENGINE_EN_HSHAKE_CH4_POSITION)
#define  DMA_READ_ENGINE_EN_HSHAKE_CH5_POSITION 21
#define  DMA_READ_ENGINE_EN_HSHAKE_CH5_MASK     (0x1 << DMA_READ_ENGINE_EN_HSHAKE_CH5_POSITION)
#define  DMA_READ_ENGINE_EN_HSHAKE_CH6_POSITION 22
#define  DMA_READ_ENGINE_EN_HSHAKE_CH6_MASK     (0x1 << DMA_READ_ENGINE_EN_HSHAKE_CH6_POSITION)
#define  DMA_READ_ENGINE_EN_HSHAKE_CH7_POSITION 23
#define  DMA_READ_ENGINE_EN_HSHAKE_CH7_MASK     (0x1 << DMA_READ_ENGINE_EN_HSHAKE_CH7_POSITION)
#define  DMA_RD_DOORBELL_NUM_POSITION 0
#define  DMA_RD_DOORBELL_NUM_MASK     (0x7 << DMA_RD_DOORBELL_NUM_POSITION)
#define  DMA_RD_STOP_POSITION 31
#define  DMA_RD_STOP_MASK       (0x1 << DMA_RD_STOP_POSITION)
#define  DMA_READ_CHANNEL0_WEIGHT_POSITION 0
#define  DMA_READ_CHANNEL0_WEIGHT_MASK (0x1f << DMA_READ_CHANNEL0_WEIGHT_POSITION)
#define  DMA_READ_CHANNEL1_WEIGHT_POSITION 5
#define  DMA_READ_CHANNEL1_WEIGHT_MASK (0x1f << DMA_READ_CHANNEL1_WEIGHT_POSITION)
#define  DMA_READ_CHANNEL2_WEIGHT_POSITION 10
#define  DMA_READ_CHANNEL2_WEIGHT_MASK  (0x1f << DMA_READ_CHANNEL2_WEIGHT_POSITION)
#define  DMA_READ_CHANNEL3_WEIGHT_POSITION 15
#define  DMA_READ_CHANNEL3_WEIGHT_MASK  (0x1f << DMA_READ_CHANNEL3_WEIGHT_POSITION)
#define  DMA_READ_CHANNEL4_WEIGHT_POSITION 0
#define  DMA_READ_CHANNEL4_WEIGHT_MASK (0x1f << DMA_READ_CHANNEL4_WEIGHT_POSITION)
#define  DMA_READ_CHANNEL5_WEIGHT_POSITION 5
#define  DMA_READ_CHANNEL5_WEIGHT_MASK (0x1f << DMA_READ_CHANNEL5_WEIGHT_POSITION)
#define  DMA_READ_CHANNEL6_WEIGHT_POSITION 10
#define  DMA_READ_CHANNEL6_WEIGHT_MASK (0x1f << DMA_READ_CHANNEL6_WEIGHT_POSITION)
#define  DMA_READ_CHANNEL7_WEIGHT_POSITION 15
#define  DMA_READ_CHANNEL7_WEIGHT_MASK (0x1f << DMA_READ_CHANNEL7_WEIGHT_POSITION)
#define  DMA_READ_ENGINE_CHGROUP_START_POSITION 0
#define  DMA_READ_ENGINE_CHGROUP_START_MASK (0xff << DMA_READ_ENGINE_CHGROUP_START_POSITION)
#define  DMA_READ_ENGINE_CHGROUP_END_POSITION 8
#define  DMA_READ_ENGINE_CHGROUP_END_MASK (0xff << DMA_READ_ENGINE_CHGROUP_END_POSITION)
#define  DMA_READ_ENGINE_CHGROUP_ENABLE_POSITION 16
#define  DMA_READ_ENGINE_CHGROUP_ENABLE_MASK (0x1 << DMA_READ_ENGINE_CHGROUP_ENABLE_POSITION)
#define  DMA_WR_DONE_INT_STATUS_POSITION 0
#define  DMA_WR_DONE_INT_STATUS_MASK       (0xff << DMA_WR_DONE_INT_STATUS_POSITION)
#define  DMA_WR_DONE_INT_STATUS_POSITION_CH(x) (x+DMA_WR_DONE_INT_STATUS_POSITION)
#define  DMA_WR_DONE_INT_STATUS_MASK_CH(x)      (0x1 << DMA_WR_DONE_INT_STATUS_POSITION_CH(x))

#define  DMA_WR_ABORT_INT_STATUS_POSITION 16
#define  DMA_WR_ABORT_INT_STATUS_MASK       (0xff << DMA_WR_ABORT_INT_STATUS_POSITION)
#define  DMA_WR_DONE_INT_MASK_POSITION 0
#define  DMA_WR_DONE_INT_MASK_MASK      (0xff << DMA_WR_DONE_INT_MASK_POSITION)
#define  DMA_WR_ABORT_INT_MASK_POSITION 16
#define  DMA_WR_ABORT_INT_MASK_MASK       (0xff << DMA_WR_ABORT_INT_MASK_POSITION)
#define  DMA_WR_DONE_INT_CLEAR_POSITION 0
#define  DMA_WR_DONE_INT_CLEAR_MASK      (0xff << DMA_WR_DONE_INT_CLEAR_POSITION)
#define  DMA_WR_ABORT_INT_CLEAR_POSITION 16
#define  DMA_WR_ABORT_INT_CLEAR_MASK       (0xff << DMA_WR_DONE_INT_CLEAR_POSITION)
#define  DMA_APP_READ_ERR_DETECT_POSITION 0
#define  DMA_APP_READ_ERR_DETECT_MASK       (0xff << DMA_APP_READ_ERR_DETECT_POSITION)
#define  DMA_LINKLIST_ELEMENT_FETCH_ERR_DETECT_POSITION 16
#define  DMA_LINKLIST_ELEMENT_FETCH_ERR_DETECT_MASK       (0xff << DMA_LINKLIST_ELEMENT_FETCH_ERR_DETECT_POSITION)
#define  DMA_WRITE_DONE_LOW_REG_POSITION 0
#define  DMA_WRITE_DONE_LOW_REG_MASK      (0xffff_ffff << DMA_WRITE_DONE_LOW_REG_POSITION)
#define  DMA_WRITE_DONE_HIGH_REG_POSITION 0
#define  DMA_WRITE_DONE_HIGH_REG_MASK      (0xffff_ffff << DMA_WRITE_DONE_HIGH_REG_POSITION)
#define  DMA_WRITE_ABORT_LOW_REG_POSITION 0
#define  DMA_WRITE_ABORT_LOW_REG_MASK      (0xffff_ffff << DMA_WRITE_ABORT_LOW_REG_POSITION)
#define  DMA_WRITE_ABORT_HIGH_REG_POSITION 0
#define  DMA_WRITE_ABORT_HIGH_REG_MASK      (0xffff_ffff << DMA_WRITE_ABORT_HIGH_REG_POSITION)
#define  DMA_WR_CHANNEL_0_DATA_POSITION 0
#define  DMA_WR_CHANNEL_0_DATA_MASK      (0xffff << DMA_WR_CHANNEL_0_DATA_POSITION)
#define  DMA_RESERVED_WR_CHANNEL_0_DATA_POSITION 16
#define  DMA_RESERVED_WR_CHANNEL_0_DATA_MASK      (0xffff << DMA_RESERVED_WR_CHANNEL_0_DATA_POSITION)
#define  DMA_WR_CHANNEL_1_DATA_POSITION 16
#define  DMA_WR_CHANNEL_1_DATA_MASK      (0xffff << DMA_WR_CHANNEL_1_DATA_POSITION)
#define  DMA_WR_CHANNEL_2_DATA_POSITION 0
#define  DMA_WR_CHANNEL_2_DATA_MASK      (0xffff << DMA_WR_CHANNEL_2_DATA_POSITION)
#define  DMA_RESERVED_WR_CHANNEL_2_DATA_POSITION 16
#define  DMA_RESERVED_WR_CHANNEL_2_DATA_MASK (0xffff << DMA_RESERVED_WR_CHANNEL_2_DATA_POSITION)
#define  DMA_WR_CHANNEL_3_DATA_POSITION 16
#define  DMA_WR_CHANNEL_3_DATA_MASK      (0xffff << DMA_WR_CHANNEL_3_DATA_POSITION)
#define  DMA_WR_CHANNEL_4_DATA_POSITION 0
#define  DMA_WR_CHANNEL_4_DATA_MASK      (0xffff << DMA_WR_CHANNEL_4_DATA_POSITION)
#define  DMA_RESERVED_WR_CHANNEL_4_DATA_POSITION 16
#define  DMA_RESERVED_WR_CHANNEL_4_DATA_MASK (0xffff << DMA_RESERVED_WR_CHANNEL_4_DATA_POSITION)
#define  DMA_WR_CHANNEL_5_DATA_POSITION 16
#define  DMA_WR_CHANNEL_5_DATA_MASK      (0xffff << DMA_WR_CHANNEL_5_DATA_POSITION)
#define  DMA_WR_CHANNEL_6_DATA_POSITION 0
#define  DMA_WR_CHANNEL_6_DATA_MASK      (0xffff << DMA_WR_CHANNEL_6_DATA_POSITION)
#define  DMA_RESERVED_WR_CHANNEL_6_DATA_POSITION 16
#define  DMA_RESERVED_WR_CHANNEL_6_DATA_MASK (0xffff << DMA_RESERVED_WR_CHANNEL_6_DATA_POSITION)
#define  DMA_WR_CHANNEL_7_DATA_POSITION 16
#define  DMA_WR_CHANNEL_7_DATA_MASK      (0xffff << DMA_WR_CHANNEL_7_DATA_POSITION)
#define  DMA_WR_CHANNEL_LLRAIE_POSITION 0
#define  DMA_WR_CHANNEL_LLRAIE_MASK      (0xff << DMA_WR_CHANNEL_LLRAIE_POSITION)
#define  DMA_WR_CHANNEL_LLLAIE_POSITION 16
#define  DMA_WR_CHANNEL_LLLAIE_MASK       (0xff  << DMA_WR_CHANNEL_LLLAIE_POSITION)
#define  DMA_RD_DONE_INT_STATUS_POSITION 0
#define  DMA_RD_DONE_INT_STATUS_MASK      (0xff  << DMA_RD_DONE_INT_STATUS_POSITION)
#define  DMA_RD_DONE_INT_STATUS_POSITION_CH(x) (x+DMA_RD_DONE_INT_STATUS_POSITION)
#define  DMA_RD_DONE_INT_STATUS_MASK_CH(x)  (0x1  << DMA_RD_DONE_INT_STATUS_POSITION_CH(x))

#define  DMA_RD_ABORT_INT_STATUS_POSITION 16
#define  DMA_RD_ABORT_INT_STATUS_MASK     (0xff  << DMA_RD_ABORT_INT_STATUS_POSITION)
#define  DMA_RD_DONE_INT_MASK_POSITION 0
#define  DMA_RD_DONE_INT_MASK_MASK       (0xff << DMA_RD_DONE_INT_MASK_POSITION)
#define  DMA_RD_ABORT_INT_MASK_POSITION 16
#define  DMA_RD_ABORT_INT_MASK_MASK       (0xff  << DMA_RD_ABORT_INT_MASK_POSITION)
#define  DMA_RD_DONE_INT_CLEAR_POSITION 0
#define  DMA_RD_DONE_INT_CLEAR_MASK       (0xff << DMA_RD_DONE_INT_CLEAR_POSITION)
#define  DMA_RD_ABORT_INT_CLEAR_POSITION 16
#define  DMA_RD_ABORT_INT_CLEAR_MASK       (0xff << DMA_RD_ABORT_INT_CLEAR_POSITION)
#define  DMA_APP_WR_ERR_DETECT_POSITION 0
#define  DMA_APP_WR_ERR_DETECT_MASK      (0xff << DMA_APP_WR_ERR_DETECT_POSITION)
#define  DMA_LINK_LIST_ELEMENT_FETCH_ERR_DETECT_POSITION 16
#define  DMA_LINK_LIST_ELEMENT_FETCH_ERR_DETECT_MASK       (0xff << DMA_LINK_LIST_ELEMENT_FETCH_ERR_DETECT_POSITION)
#define  DMA_UNSUPPORTED_REQ_POSITION 0
#define  DMA_UNSUPPORTED_REQ_MASK      (0xff << DMA_UNSUPPORTED_REQ_POSITION)
#define  DMA_CPL_ABORT_POSITION 8
#define  DMA_CPL_ABORT_MASK      (0xff << DMA_CPL_ABORT_POSITION)
#define  DMA_CPL_TIMEOUT_POSITION 16
#define  DMA_CPL_TIMEOUT_MASK       (0xff << DMA_CPL_TIMEOUT_POSITION)
#define  DMA_DATA_POISIONING_POSITION 24
#define  DMA_DATA_POISIONING_MASK       (0xff << DMA_DATA_POISIONING_POSITION)
#define  DMA_RD_CHANNEL_LLRAIE_POSITION 0
#define  DMA_RD_CHANNEL_LLRAIE_MASK      (dma_rd_chan_reset << DMA_RD_CHANNEL_LLRAIE_POSITION)
#define  DMA_RD_CHANNEL_LLLAIE_POSITION 16
#define  DMA_RD_CHANNEL_LLLAIE_MASK       (dma_rd_chan_reset << DMA_RD_CHANNEL_LLLAIE_POSITION)
#define  DMA_READ_DONE_LOW_REG_POSITION 0
#define  DMA_READ_DONE_LOW_REG_MASK       (0xffff_ffff << DMA_READ_DONE_LOW_REG_POSITION)
#define  DMA_READ_DONE_HIGH_REG_POSITION 0
#define  DMA_READ_DONE_HIGH_REG_MASK       (0xffff_ffff << DMA_READ_DONE_HIGH_REG_POSITION)
#define  DMA_READ_ABORT_LOW_REG_POSITION 0
#define  DMA_READ_ABORT_LOW_REG_MASK      (0xffff_ffff << DMA_READ_ABORT_LOW_REG_POSITION)
#define  DMA_READ_ABORT_HIGH_REG_POSITION 0
#define  DMA_READ_ABORT_HIGH_REG_MASK      (0xffff_ffff << DMA_READ_ABORT_HIGH_REG_POSITION)
#define  DMA_RD_CHANNEL_0_DATA_POSITION 0
#define  DMA_RD_CHANNEL_0_DATA_MASK      (0xffff << DMA_RD_CHANNEL_0_DATA_POSITION)
#define  DMA_RESERVED_RD_CHANNEL_0_DATA_POSITION 16
#define  DMA_RESERVED_RD_CHANNEL_0_DATA_MASK (0xffff << DMA_RESERVED_RD_CHANNEL_0_DATA_POSITION)
#define  DMA_RD_CHANNEL_1_DATA_POSITION 16
#define  DMA_RD_CHANNEL_1_DATA_MASK      (0xffff << DMA_RD_CHANNEL_1_DATA_POSITION)

#define  DMA_RD_CHANNEL_2_DATA_POSITION 0
#define  DMA_RD_CHANNEL_2_DATA_MASK      (0xffff << DMA_RD_CHANNEL_2_DATA_POSITION)
#define  DMA_RESERVED_RD_CHANNEL_2_DATA_POSITION 16
#define  DMA_RESERVED_RD_CHANNEL_2_DATA_MASK (0xffff << DMA_RESERVED_RD_CHANNEL_2_DATA_POSITION)
#define  DMA_RD_CHANNEL_3_DATA_POSITION 16
#define  DMA_RD_CHANNEL_3_DATA_MASK      (0xffff << DMA_RD_CHANNEL_3_DATA_POSITION)

#define  DMA_RD_CHANNEL_4_DATA_POSITION 0
#define  DMA_RD_CHANNEL_4_DATA_MASK      (0xffff << DMA_RD_CHANNEL_4_DATA_POSITION)
#define  DMA_RESERVED_RD_CHANNEL_4_DATA_POSITION 16
#define  DMA_RESERVED_RD_CHANNEL_4_DATA_MASK (0xffff << DMA_RESERVED_RD_CHANNEL_4_DATA_POSITION)
#define  DMA_RD_CHANNEL_5_DATA_POSITION 16
#define  DMA_RD_CHANNEL_5_DATA_MASK      (0xffff << DMA_RD_CHANNEL_5_DATA_POSITION)

#define  DMA_RD_CHANNEL_6_DATA_POSITION 0
#define  DMA_RD_CHANNEL_6_DATA_MASK      (0xffff << DMA_RD_CHANNEL_6_DATA_POSITION)
#define  DMA_RESERVED_RD_CHANNEL_6_DATA_POSITION 16
#define  DMA_RESERVED_RD_CHANNEL_6_DATA_MASK (0xffff << DMA_RESERVED_RD_CHANNEL_6_DATA_POSITION)

#define  DMA_RD_CHANNEL_7_DATA_POSITION 16
#define  DMA_RD_CHANNEL_7_DATA_MASK      (0xffff << DMA_RD_CHANNEL_7_DATA_POSITION)

#define  DMA_WRITE_ENGINE_HSHAKE_CNT_CH0_POSITION 0
#define  DMA_WRITE_ENGINE_HSHAKE_CNT_CH0_MASK     (0x1f << DMA_WRITE_ENGINE_HSHAKE_CNT_CH0_POSITION)
#define  DMA_WRITE_ENGINE_HSHAKE_CNT_CH1_POSITION 8
#define  DMA_WRITE_ENGINE_HSHAKE_CNT_CH1_MASK     (0x1f << DMA_WRITE_ENGINE_HSHAKE_CNT_CH1_POSITION)
#define  DMA_WRITE_ENGINE_HSHAKE_CNT_CH2_POSITION 16
#define  DMA_WRITE_ENGINE_HSHAKE_CNT_CH2_MASK     (0x1f << DMA_WRITE_ENGINE_HSHAKE_CNT_CH2_POSITION)
#define  DMA_WRITE_ENGINE_HSHAKE_CNT_CH3_POSITION 24
#define  DMA_WRITE_ENGINE_HSHAKE_CNT_CH3_MASK     (0x1f << DMA_WRITE_ENGINE_HSHAKE_CNT_CH3_POSITION)

#define  DMA_WRITE_ENGINE_HSHAKE_CNT_CH4_POSITION 0
#define  DMA_WRITE_ENGINE_HSHAKE_CNT_CH4_MASK     (0x1f << DMA_WRITE_ENGINE_HSHAKE_CNT_CH4_POSITION)
#define  DMA_WRITE_ENGINE_HSHAKE_CNT_CH5_POSITION 8
#define  DMA_WRITE_ENGINE_HSHAKE_CNT_CH5_MASK     (0x1f << DMA_WRITE_ENGINE_HSHAKE_CNT_CH5_POSITION)
#define  DMA_WRITE_ENGINE_HSHAKE_CNT_CH6_POSITION 16
#define  DMA_WRITE_ENGINE_HSHAKE_CNT_CH6_MASK     (0x1f << DMA_WRITE_ENGINE_HSHAKE_CNT_CH6_POSITION)
#define  DMA_WRITE_ENGINE_HSHAKE_CNT_CH7_POSITION 24
#define  DMA_WRITE_ENGINE_HSHAKE_CNT_CH7_MASK     (0x1f << DMA_WRITE_ENGINE_HSHAKE_CNT_CH7_POSITION)

#define  DMA_READ_ENGINE_HSHAKE_CNT_CH0_POSITION 0
#define  DMA_READ_ENGINE_HSHAKE_CNT_CH0_MASK     (0x1f << DMA_READ_ENGINE_HSHAKE_CNT_CH0_POSITION)
#define  DMA_READ_ENGINE_HSHAKE_CNT_CH1_POSITION 8
#define  DMA_READ_ENGINE_HSHAKE_CNT_CH1_MASK     (0x1f << DMA_READ_ENGINE_HSHAKE_CNT_CH1_POSITION)
#define  DMA_READ_ENGINE_HSHAKE_CNT_CH2_POSITION 16
#define  DMA_READ_ENGINE_HSHAKE_CNT_CH2_MASK     (0x1f << DMA_READ_ENGINE_HSHAKE_CNT_CH2_POSITION)
#define  DMA_READ_ENGINE_HSHAKE_CNT_CH3_POSITION 24
#define  DMA_READ_ENGINE_HSHAKE_CNT_CH3_MASK     (0x1f << DMA_READ_ENGINE_HSHAKE_CNT_CH3_POSITION)

#define  DMA_READ_ENGINE_HSHAKE_CNT_CH4_POSITION 0
#define  DMA_READ_ENGINE_HSHAKE_CNT_CH4_MASK     (0x1f << DMA_READ_ENGINE_HSHAKE_CNT_CH4_POSITION)
#define  DMA_READ_ENGINE_HSHAKE_CNT_CH5_POSITION 8
#define  DMA_READ_ENGINE_HSHAKE_CNT_CH5_MASK     (0x1f << DMA_READ_ENGINE_HSHAKE_CNT_CH5_POSITION)
#define  DMA_READ_ENGINE_HSHAKE_CNT_CH6_POSITION 16
#define  DMA_READ_ENGINE_HSHAKE_CNT_CH6_MASK     (0x1f << DMA_READ_ENGINE_HSHAKE_CNT_CH6_POSITION)
#define  DMA_READ_ENGINE_HSHAKE_CNT_CH7_POSITION 24
#define  DMA_READ_ENGINE_HSHAKE_CNT_CH7_MASK     (0x1f << DMA_READ_ENGINE_HSHAKE_CNT_CH7_POSITION)

#define  DMA_WRITE_CH0_PWR_EN_POSITION 0
#define  DMA_WRITE_CH0_PWR_EN_MASK     (0x1 << DMA_WRITE_CH0_PWR_EN_POSITION)
#define  DMA_WRITE_CH1_PWR_EN_POSITION 0
#define  DMA_WRITE_CH1_PWR_EN_MASK     (0x1 << DMA_WRITE_CH1_PWR_EN_POSITION)
#define  DMA_WRITE_CH2_PWR_EN_POSITION 0
#define  DMA_WRITE_CH2_PWR_EN_MASK     (0x1 << DMA_WRITE_CH2_PWR_EN_POSITION)
#define  DMA_WRITE_CH3_PWR_EN_POSITION 0
#define  DMA_WRITE_CH3_PWR_EN_MASK     (0x1 << DMA_WRITE_CH3_PWR_EN_POSITION)
#define  DMA_WRITE_CH4_PWR_EN_POSITION 0
#define  DMA_WRITE_CH4_PWR_EN_MASK     (0x1 << DMA_WRITE_CH4_PWR_EN_POSITION)
#define  DMA_WRITE_CH5_PWR_EN_POSITION 0
#define  DMA_WRITE_CH5_PWR_EN_MASK     (0x1 << DMA_WRITE_CH5_PWR_EN_POSITION)
#define  DMA_WRITE_CH6_PWR_EN_POSITION 0
#define  DMA_WRITE_CH6_PWR_EN_MASK     (0x1 << DMA_WRITE_CH6_PWR_EN_POSITION)
#define  DMA_WRITE_CH7_PWR_EN_POSITION 0
#define  DMA_WRITE_CH7_PWR_EN_MASK     (0x1 << DMA_WRITE_CH7_PWR_EN_POSITION)

#define  DMA_READ_CH0_PWR_EN_POSITION 0
#define  DMA_READ_CH0_PWR_EN_MASK     (0x1 << DMA_READ_CH0_PWR_EN_POSITION)
#define  DMA_READ_CH1_PWR_EN_POSITION 0
#define  DMA_READ_CH1_PWR_EN_MASK     (0x1 << DMA_READ_CH1_PWR_EN_POSITION)
#define  DMA_READ_CH2_PWR_EN_POSITION 0
#define  DMA_READ_CH2_PWR_EN_MASK     (0x1 << DMA_READ_CH2_PWR_EN_POSITION)
#define  DMA_READ_CH3_PWR_EN_POSITION 0
#define  DMA_READ_CH3_PWR_EN_MASK     (0x1 << DMA_READ_CH3_PWR_EN_POSITION)
#define  DMA_READ_CH4_PWR_EN_POSITION 0
#define  DMA_READ_CH4_PWR_EN_MASK     (0x1 << DMA_READ_CH4_PWR_EN_POSITION)
#define  DMA_READ_CH5_PWR_EN_POSITION 0
#define  DMA_READ_CH5_PWR_EN_MASK     (0x1 << DMA_READ_CH5_PWR_EN_POSITION)
#define  DMA_READ_CH6_PWR_EN_POSITION 0
#define  DMA_READ_CH6_PWR_EN_MASK     (0x1 << DMA_READ_CH6_PWR_EN_POSITION)
#define  DMA_READ_CH7_PWR_EN_POSITION 0
#define  DMA_READ_CH7_PWR_EN_MASK     (0x1 << DMA_READ_CH7_PWR_EN_POSITION)

#define  DMA_CB_POSITION 0
#define  DMA_CB_MASK      (0x1 << DMA_CB_POSITION)
#define  DMA_TCB_POSITION 1
#define  DMA_TCB_MASK      (0x1 << DMA_TCB_POSITION)
#define  DMA_LLP_POSITION 2
#define  DMA_LLP_MASK      (0x1 << DMA_LLP_POSITION)
#define  DMA_LIE_POSITION 3
#define  DMA_LIE_MASK      (0x1 << DMA_LIE_POSITION)
#define  DMA_RIE_POSITION 4
#define  DMA_RIE_MASK      (0x1 << DMA_RIE_POSITION)
#define  DMA_CS_POSITION 5
#define  DMA_CS_MASK      (0x3 << DMA_CS_POSITION)
#define  DMA_RESERVED0_POSITION 7
#define  DMA_RESERVED0_MASK (0x1 << DMA_RESERVED0_POSITION)
#define  DMA_CCS_POSITION 8
#define  DMA_CCS_MASK      (0x1 << DMA_CCS_POSITION)
#define  DMA_LLE_POSITION 9
#define  DMA_LLE_MASK      (0x1 << DMA_LLE_POSITION)
#define  DMA_RESERVED1_POSITION 10
#define  DMA_RESERVED1_MASK (0x3 << DMA_RESERVED0_POSITION)
#define  DMA_FUNC_NUM_POSITION 12
#define  DMA_FUNC_NUM_MASK       (0x1f << DMA_FUNC_NUM_POSITION)
#define  DMA_RESERVED2_POSITION 17
#define  DMA_RESERVED2_MASK (0x3f << DMA_RESERVED2_POSITION)
#define  DMA_NS_DST_POSITION 23
#define  DMA_NS_DST_MASK       (0x1 << DMA_NS_DST_POSITION)
#define  DMA_NS_SRC_POSITION 24
#define  DMA_NS_SRC_MASK       (0x1 << DMA_NS_SRC_POSITION)
#define  DMA_RO_POSITION 25
#define  DMA_RO_MASK       (0x1 << DMA_RO_POSITION)
#define  DMA_RESERVED5_POSITION 26
#define  DMA_RESERVED5_MASK       (0x1 << DMA_RESERVED5_POSITION)
#define  DMA_TC_POSITION 27
#define  DMA_TC_MASK       (0x7 << DMA_TC_POSITION)
#define  DMA_AT_POSITION 30
#define  DMA_AT_MASK       (0x3 << DMA_AT_POSITION)

#define  DMA_VF_EN_POSITION 0
#define  DMA_VF_EN_MASK     (0x1 << DMA_VF_EN_POSITION)
#define  DMA_VF_POSITION    1
#define  DMA_VF_MASK        (0xFF << DMA_VF_POSITION)
#define  DMA_VF_RESERVED_POSITION    9
#define  DMA_VF_RESERVED_MASK        (0xFF << DMA_VF_RESERVED_POSITION)
#define  DMA_RESERVED4_POSITION 0
#define  DMA_RESERVED4_MASK (0x1ffff << DMA_RESERVED4_POSITION)
#define  DMA_ST_POSITION    17
#define  DMA_ST_MASK        (0xFF << DMA_ST_POSITION)
#define  DMA_RESERVED3_POSITION 25
#define  DMA_RESERVED3_MASK (0xf << DMA_RESERVED3_POSITION)
#define  DMA_PH_POSITION    29
#define  DMA_PH_MASK        (0x3 << DMA_PH_POSITION)
#define  DMA_TH_POSITION    31
#define  DMA_TH_MASK        (0x1 << DMA_TH_POSITION)
#define  DMA_ST_RESERVED_POSITION 17
#define  DMA_ST_RESERVED_MASK  (0x7fff << DMA_ST_RESERVED_POSITION)

#define  DMA_TRANSFER_SIZE_POSITION 0
#define  DMA_TRANSFER_SIZE_MASK      (0xffff_ffff << DMA_TRANSFER_SIZE_POSITION)

#define  DMA_SRC_ADDR_REG_LOW_POSITION 0
#define  DMA_SRC_ADDR_REG_LOW_MASK      (0xffff_ffff << DMA_SRC_ADDR_REG_LOW_POSITION)

#define  DMA_SRC_ADDR_REG_HIGH_POSITION 0
#define  DMA_SRC_ADDR_REG_HIGH_MASK      (0xffff_ffff << DMA_SRC_ADDR_REG_HIGH_POSITION)

#define  DMA_DST_ADDR_REG_LOW_POSITION 0
#define  DMA_DST_ADDR_REG_LOW_MASK      (0xffff_ffff << DMA_DST_ADDR_REG_LOW_POSITION)

#define  DMA_DST_ADDR_REG_HIGH_POSITION 0
#define  DMA_DST_ADDR_REG_HIGH_MASK      (0xffff_ffff << DMA_DST_ADDR_REG_HIGH_POSITION)

#define  DMA_LLP_LOW_POSITION 0
#define  DMA_LLP_LOW_MASK       (0xffff_ffff << DMA_LLP_LOW_POSITION)

#define  DMA_LLP_HIGH_POSITION 0
#define  DMA_LLP_HIGH_MASK      (0xffff_ffff << DMA_LLP_HIGH_POSITION)



//## ATU Unroll ##################################################################

#define  ATU_TYPE_POSITION 0
#define  ATU_TYPE_MASK       (0x1f << ATU_TYPE_POSITION)
#define  ATU_TC_POSITION 5
#define  ATU_TC_MASK       (0x7 << ATU_TC_POSITION)
#define  ATU_TD_POSITION 8
#define  ATU_TD_MASK       (0x1 << ATU_TD_POSITION)
#define  ATU_ATTR_POSITION 9
#define  ATU_ATTR_MASK     (0x3 << ATU_ATTR_POSITION)
#define  ATU_IDO_POSITION  11
#define  ATU_IDO_MASK      (0x1 << ATU_IDO_POSITION)

#define  ATU_TH_POSITION 12
#define  ATU_TH_MASK    (0x1 << ATU_TH_POSITION)

#define  ATU_INCREASE_REGION_SIZE_POSITION 13
#define  ATU_INCREASE_REGION_SIZE_MASK (0x1 << ATU_INCREASE_REGION_SIZE_POSITION)

#define  ATU_AT_POSITION 16
#define  ATU_AT_MASK     (0x3 << ATU_AT_POSITION)

#define  ATU_PH_POSITION 18
#define  ATU_PH_MASK      (0x3 << ATU_PH_POSITION)


#define  ATU_CTRL_1_FUNC_NUM_POSITION 20
#define  ATU_CTRL_1_FUNC_NUM_MASK    ({CX_NFUNC_WD{0x1}} << ATU_CTRL_1_FUNC_NUM_POSITION)

#define  ATU_MSG_CODE_POSITION 0
#define  ATU_MSG_CODE_MASK       (0xff << ATU_MSG_CODE_POSITION)
#define  ATU_BAR_NUM_POSITION 8
#define  ATU_BAR_NUM_MASK       (0x7 << ATU_BAR_NUM_POSITION)
#define  ATU_TC_MATCH_EN_POSITION 14
#define  ATU_TC_MATCH_EN_MASK       (0x1 << ATU_TC_MATCH_EN_POSITION)
#define  ATU_TD_MATCH_EN_POSITION 15
#define  ATU_TD_MATCH_EN_MASK       (0x1 << ATU_TD_MATCH_EN_POSITION)
#define  ATU_ATTR_MATCH_EN_POSITION 16
#define  ATU_ATTR_MATCH_EN_MASK       (0x1 << ATU_ATTR_MATCH_EN_POSITION)
#define  ATU_TH_MATCH_EN_POSITION 17
#define  ATU_TH_MATCH_EN_MASK       (0x1 << ATU_TH_MATCH_EN_POSITION)
#define  ATU_SINGLE_ADDR_LOC_TRANS_EN_POSITION 23
#define  ATU_SINGLE_ADDR_LOC_TRANS_EN_MASK (0x1 << ATU_SINGLE_ADDR_LOC_TRANS_EN_POSITION)
#define  ATU_MSG_TYPE_MATCH_MODE_POSITION 13
#define  ATU_MSG_TYPE_MATCH_MODE_MASK (0x1 << ATU_MSG_TYPE_MATCH_MODE_POSITION)

#define  ATU_TAG_POSITION   8
#define  ATU_TAG_MASK       (0xff << ATU_TAG_POSITION)
#define  ATU_TAG_SUBSTITUTE_EN_POSITION 16
#define  ATU_TAG_SUBSTITUTE_EN_MASK (0x1 << ATU_TAG_SUBSTITUTE_EN_POSITION)
#define  ATU_MSB2BITS_TAG_POSITION 17
#define  ATU_MSB2BITS_TAG_MASK (0x3 << ATU_MSB2BITS_TAG_POSITION)
#define  ATU_INHIBIT_PAYLOAD_POSITION 22
#define  ATU_INHIBIT_PAYLOAD_MASK (0x1 << ATU_INHIBIT_PAYLOAD_POSITION)
#define  ATU_HEADER_SUBSTITUTE_EN_POSITION 23
#define  ATU_HEADER_SUBSTITUTE_EN_MASK    (0x1 << ATU_HEADER_SUBSTITUTE_EN_POSITION)
#define  ATU_AT_MATCH_EN_POSITION 18
#define  ATU_AT_MATCH_EN_MASK       (0x1 << ATU_AT_MATCH_EN_POSITION)
#define  ATU_FUNC_NUM_MATCH_EN_POSITION 19
#define  ATU_FUNC_NUM_MATCH_EN_MASK       (0x1 << ATU_FUNC_NUM_MATCH_EN_POSITION)
#define  ATU_SNP_POSITION 20
#define  ATU_SNP_MASK               (0x1 << ATU_SNP_POSITION)
#define  ATU_FUNC_BYPASS_POSITION 19
#define  ATU_FUNC_BYPASS_MASK       (0x1 << ATU_FUNC_NUM_MATCH_EN_POSITION)
#define  ATU_VF_MATCH_EN_POSITION 20
#define  ATU_VF_MATCH_EN_MASK       (0x1 << ATU_VF_MATCH_EN_POSITION)
#define  ATU_MSG_CODE_MATCH_EN_POSITION 21
#define  ATU_MSG_CODE_MATCH_EN_MASK       (0x1 << ATU_MSG_CODE_MATCH_EN_POSITION)
#define  ATU_PH_MATCH_EN_POSITION 22
#define  ATU_PH_MATCH_EN_MASK       (0x1 << ATU_PH_MATCH_EN_POSITION)

#define  ATU_RESPONSE_CODE_POSITION 24
#define  ATU_RESPONSE_CODE_MASK       (0x3 << ATU_RESPONSE_CODE_POSITION)
#define  ATU_VFBAR_MATCH_MODE_EN_POSITION     26
#define  ATU_VFBAR_MATCH_MODE_EN_MASK     (0x1 << ATU_VFBAR_MATCH_MODE_EN_POSITION)
#define  ATU_FUZZY_TYPE_MATCH_CODE_POSITION 27
#define  ATU_FUZZY_TYPE_MATCH_CODE_MASK       (0x1 << ATU_FUZZY_TYPE_MATCH_CODE_POSITION)
#define  ATU_DMA_BYPASS_POSITION 27
#define  ATU_DMA_BYPASS_MASK (0x1 << ATU_DMA_BYPASS_POSITION)
#define  ATU_CFG_SHIFT_MODE_POSITION 28
#define  ATU_CFG_SHIFT_MODE_MASK       (0x1 << ATU_CFG_SHIFT_MODE_POSITION)
#define  ATU_INVERT_MODE_POSITION 29
#define  ATU_INVERT_MODE_MASK       (0x1 << ATU_INVERT_MODE_POSITION)
#define  ATU_MATCH_MODE_POSITION 30
#define  ATU_MATCH_MODE_MASK       (0x1 << ATU_MATCH_MODE_POSITION)
#define  ATU_REGION_EN_POSITION 31
#define  ATU_REGION_EN_MASK       (0x1 << ATU_REGION_EN_POSITION)

#define  ATU_LWR_BASE_HW_POSITION 0

#define  ATU_LWR_BASE_HW_MASK   (CX_ATU_MIN_REGION_SIZE==65536) ? (0xffff << `ATU_LWR_BASE_HW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==32768) ? (0x7fff << `ATU_LWR_BASE_HW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==16384) ? (0x3fff << `ATU_LWR_BASE_HW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==8192) ? (0x1fff << `ATU_LWR_BASE_HW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==4096) ? (0xfff<< `ATU_LWR_BASE_HW_POSITION) : (0xffff << `ATU_LWR_BASE_HW_POSITION)

#define  ATU_LWR_BASE_RW_POSITION  ((CX_ATU_MIN_REGION_SIZE==65536) ? 16 : (`CX_ATU_MIN_REGION_SIZE==32768) ? 15 : (`CX_ATU_MIN_REGION_SIZE==16384) ? 14 : (`CX_ATU_MIN_REGION_SIZE==8192) ? 13 : (`CX_ATU_MIN_REGION_SIZE==4096) ? 12 : 16)

#define  ATU_LWR_BASE_RW_MASK  (CX_ATU_MIN_REGION_SIZE==65536) ? (0xffff << `ATU_LWR_BASE_RW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==32768) ? (0x1ffff << `ATU_LWR_BASE_RW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==16384) ? (0x3ffff << `ATU_LWR_BASE_RW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==8192) ? (0x7ffff << `ATU_LWR_BASE_RW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==4096) ? (0xfffff << `ATU_LWR_BASE_RW_POSITION) : (0xffff << `ATU_LWR_BASE_RW_POSITION)

#define  ATU_UPPER_BASE_RW_POSITION 0
#define  ATU_UPPER_BASE_RW_MASK      (0xffff_ffff << ATU_UPPER_BASE_RW_POSITION)

#define  ATU_LIMIT_ADDR_HW_POSITION 0

#define  ATU_LIMIT_ADDR_HW_MASK    (CX_ATU_MIN_REGION_SIZE==65536) ? (0xffff << `ATU_LIMIT_ADDR_HW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==32768) ? (0x7fff << `ATU_LIMIT_ADDR_HW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==16384) ? (0x3fff<< `ATU_LIMIT_ADDR_HW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==8192) ? (0x1fff << `ATU_LIMIT_ADDR_HW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==4096) ? (0xfff << `ATU_LIMIT_ADDR_HW_POSITION): (0xffff << `ATU_LIMIT_ADDR_HW_POSITION)

#define  ATU_LIMIT_ADDR_RW_POSITION ((CX_ATU_MIN_REGION_SIZE==65536) ? 16 : (`CX_ATU_MIN_REGION_SIZE==32768) ? 15 : (`CX_ATU_MIN_REGION_SIZE==16384) ? 14 : (`CX_ATU_MIN_REGION_SIZE==8192) ? 13 : (`CX_ATU_MIN_REGION_SIZE==4096) ? 12 : 16)

#define  ATU_LIMIT_ADDR_RW_MASK     (CX_ATU_MIN_REGION_SIZE==65536) ? (0xffff << `ATU_LIMIT_ADDR_RW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==32768) ? (0x1ffff << `ATU_LIMIT_ADDR_RW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==16384) ? (0x3ffff << `ATU_LIMIT_ADDR_RW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==8192) ? (0x7ffff << `ATU_LIMIT_ADDR_RW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==4096) ? (0xfffff << `ATU_LIMIT_ADDR_RW_POSITION) : (0xffff << `ATU_LIMIT_ADDR_RW_POSITION)

// 1'b0 used below to avoid Zero width replication error when max_size=0/32
//RTK #define  ATU_UPPR_LIMIT_ADDR_RW_POSITION 0
//RTK #define  ATU_UPPR_LIMIT_ADDR_RW_MASK ({1'b0, {(CX_ATU_MAX_REGION_SIZE){0x1}}} << `ATU_UPPR_LIMIT_ADDR_RW_POSITION)
//RTK #define  ATU_UPPR_LIMIT_ADDR_HW_POSITION CX_ATU_MAX_REGION_SIZE
//RTK #define  ATU_UPPR_LIMIT_ADDR_HW_MASK ({1'b0, {32-(CX_ATU_MAX_REGION_SIZE){0x1}}} << `ATU_UPPR_LIMIT_ADDR_HW_POSITION)

#define  ATU_LWR_TARGET_HW_POSITION 0

#define  ATU_LWR_TARGET_HW_MASK    (CX_ATU_MIN_REGION_SIZE==65536) ? (0xffff << `ATU_LWR_TARGET_HW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==32768) ? (0x7fff << `ATU_LWR_TARGET_HW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==16384) ? (0x3fff << `ATU_LWR_TARGET_HW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==8192) ? (0x1fff << `ATU_LWR_TARGET_HW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==4096) ? (0xfff<< `ATU_LWR_TARGET_HW_POSITION): (0xffff << `ATU_LWR_TARGET_HW_POSITION)

#define  ATU_LWR_TARGET_RW_POSITION ((CX_ATU_MIN_REGION_SIZE==65536) ? 16 : (`CX_ATU_MIN_REGION_SIZE==32768) ? 15 : (`CX_ATU_MIN_REGION_SIZE==16384) ? 14 : (`CX_ATU_MIN_REGION_SIZE==8192) ? 13 : (`CX_ATU_MIN_REGION_SIZE==4096) ? 12 : 16)

#define  ATU_LWR_TARGET_RW_MASK     (CX_ATU_MIN_REGION_SIZE==65536) ? (0xffff << `ATU_LWR_TARGET_RW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==32768) ? (0x1ffff << `ATU_LWR_TARGET_RW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==16384) ? (0x3ffff << `ATU_LWR_TARGET_RW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==8192) ? (0x7ffff << `ATU_LWR_TARGET_RW_POSITION) : (`CX_ATU_MIN_REGION_SIZE==4096) ? (0xfffff << `ATU_LWR_TARGET_RW_POSITION) : (0xffff << `ATU_LWR_TARGET_RW_POSITION)

#define ATU_LWR_TARGET_RW_OUTBOUND_POSITION 0
#define ATU_LWR_TARGET_RW_OUTBOUND_MASK (0xffffffff << ATU_LWR_TARGET_RW_OUTBOUND_POSITION)

#define  ATU_UPPER_TARGET_RW_POSITION 0
#define  ATU_UPPER_TARGET_RW_MASK (0xffff_ffff << ATU_UPPER_TARGET_RW_POSITION)

#define  ATU_VF_ACTIVE_POSITION 31
#define  ATU_VF_ACTIVE_MASK (0x1 << ATU_VF_ACTIVE_POSITION)
#define  ATU_VF_NUMBER_POSITION 0
#define  ATU_VF_NUMBER_MASK ({(CX_NVFUNC_WD-1){0x1}}  << `ATU_VF_NUMBER_POSITION)
#define  ATU_RSVDP_VF_NUMBER_POSITION (CX_NVFUNC_WD-1)
#define  ATU_RSVDP_VF_NUMBER_MASK ({(31-(CX_NVFUNC_WD-1)){0x1}}  << `ATU_RSVDP_VF_NUMBER_POSITION)

// Interface timer registers
#define PCIE_PL_INTERFACE_TIMER_EN_POSITION       0
#define PCIE_PL_INTERFACE_TIMER_AER_EN_POSITION   1
#define PCIE_PL_INTERFACE_TIMER_SCALING_POSITION  2
#define PCIE_PL_FORCE_PENDING_POSITION  4

#define PCIE_PL_INTERFACE_TIMER_EN_MASK           (0x1 << PCIE_PL_INTERFACE_TIMER_EN_POSITION)
#define PCIE_PL_INTERFACE_TIMER_AER_EN_MASK       (0x1 << PCIE_PL_INTERFACE_TIMER_AER_EN_POSITION)
#define PCIE_PL_INTERFACE_TIMER_SCALING_MASK      (0x3 << PCIE_PL_INTERFACE_TIMER_SCALING_POSITION)
#define PCIE_PL_FORCE_PENDING_MASK                (0x1 << PCIE_PL_FORCE_PENDING_POSITION)

#define PCIE_PL_INTERFACE_TIMER_TARGET_POSITION 0
#define PCIE_PL_INTERFACE_TIMER_TARGET_MASK     (0xffff << PCIE_PL_INTERFACE_TIMER_TARGET_POSITION)

#define PCIE_PL_DBI_SLAVE_RD_ADD_TIMEOUT_POSITION   14
#define PCIE_PL_DBI_SLAVE_WR_DATA_TIMEOUT_POSITION  13
#define PCIE_PL_DBI_SLAVE_WR_ADD_TIMEOUT_POSITION   12
#define PCIE_PL_SLAVE_RD_ADD_TIMEOUT_POSITION       11
#define PCIE_PL_SLAVE_WR_DATA_TIMEOUT_POSITION      10
#define PCIE_PL_SLAVE_WR_ADD_TIMEOUT_POSITION       9
#define PCIE_PL_MASTERH_RD_DATA_TIMEOUT_POSITION    8
#define PCIE_PL_MASTERH_WR_RES_TIMEOUT_POSITION     7
#define PCIE_PL_MASTER_RD_DATA_TIMEOUT_POSITION     6
#define PCIE_PL_MASTER_WR_RES_TIMEOUT_POSITION      5
#define PCIE_PL_CLIENT2_INTERFACE_TIMEOUT_POSITION  4
#define PCIE_PL_CLIENT1_INTERFACE_TIMEOUT_POSITION  3
#define PCIE_PL_CLIENT0_INTERFACE_TIMEOUT_POSITION  2
#define PCIE_PL_CPL_INTERFACE_TIMEOUT_POSITION      1
#define PCIE_PL_MESSAGE_INTERFACE_TIMEOUT_POSITION  0

#define PCIE_PL_DBI_SLAVE_RD_ADD_TIMEOUT_MASK   (0x1 << PCIE_PL_DBI_SLAVE_RD_ADD_TIMEOUT_POSITION)
#define PCIE_PL_DBI_SLAVE_WR_DATA_TIMEOUT_MASK  (0x1 << PCIE_PL_DBI_SLAVE_WR_DATA_TIMEOUT_POSITION)
#define PCIE_PL_DBI_SLAVE_WR_ADD_TIMEOUT_MASK   (0x1 << PCIE_PL_DBI_SLAVE_WR_ADD_TIMEOUT_POSITION)
#define PCIE_PL_SLAVE_RD_ADD_TIMEOUT_MASK       (0x1 << PCIE_PL_SLAVE_RD_ADD_TIMEOUT_POSITION)
#define PCIE_PL_SLAVE_WR_DATA_TIMEOUT_MASK      (0x1 << PCIE_PL_SLAVE_WR_DATA_TIMEOUT_POSITION)
#define PCIE_PL_SLAVE_WR_ADD_TIMEOUT_MASK       (0x1 << PCIE_PL_SLAVE_WR_ADD_TIMEOUT_POSITION)
#define PCIE_PL_MASTERH_RD_DATA_TIMEOUT_MASK    (0x1 << PCIE_PL_MASTERH_RD_DATA_TIMEOUT_POSITION)
#define PCIE_PL_MASTERH_WR_RES_TIMEOUT_MASK     (0x1 << PCIE_PL_MASTERH_WR_RES_TIMEOUT_POSITION)
#define PCIE_PL_MASTER_RD_DATA_TIMEOUT_MASK     (0x1 << PCIE_PL_MASTER_RD_DATA_TIMEOUT_POSITION)
#define PCIE_PL_MASTER_WR_RES_TIMEOUT_MASK      (0x1 << PCIE_PL_MASTER_WR_RES_TIMEOUT_POSITION)
#define PCIE_PL_CLIENT2_INTERFACE_TIMEOUT_MASK  (0x1 << PCIE_PL_CLIENT2_INTERFACE_TIMEOUT_POSITION)
#define PCIE_PL_CLIENT1_INTERFACE_TIMEOUT_MASK  (0x1 << PCIE_PL_CLIENT1_INTERFACE_TIMEOUT_POSITION)
#define PCIE_PL_CLIENT0_INTERFACE_TIMEOUT_MASK  (0x1 << PCIE_PL_CLIENT0_INTERFACE_TIMEOUT_POSITION)
#define PCIE_PL_CPL_INTERFACE_TIMEOUT_MASK      (0x1 << PCIE_PL_CPL_INTERFACE_TIMEOUT_POSITION)
#define PCIE_PL_MESSAGE_INTERFACE_TIMEOUT_MASK  (0x1 << PCIE_PL_MESSAGE_INTERFACE_TIMEOUT_POSITION)

// Functional safety interrupt registers
#define PCIE_PL_SAFETY_INT_MASK_0_POSITION 0
#define PCIE_PL_SAFETY_INT_MASK_1_POSITION 1
#define PCIE_PL_SAFETY_INT_MASK_2_POSITION 2
#define PCIE_PL_SAFETY_INT_MASK_3_POSITION 3
#define PCIE_PL_SAFETY_INT_MASK_4_POSITION 4
#define PCIE_PL_SAFETY_INT_MASK_5_POSITION 5

#define PCIE_PL_SAFETY_INT_MASK_0_MASK     (0x1 << PCIE_PL_SAFETY_INT_MASK_0_POSITION)
#define PCIE_PL_SAFETY_INT_MASK_1_MASK     (0x1 << PCIE_PL_SAFETY_INT_MASK_1_POSITION)
#define PCIE_PL_SAFETY_INT_MASK_2_MASK     (0x1 << PCIE_PL_SAFETY_INT_MASK_2_POSITION)
#define PCIE_PL_SAFETY_INT_MASK_3_MASK     (0x1 << PCIE_PL_SAFETY_INT_MASK_3_POSITION)
#define PCIE_PL_SAFETY_INT_MASK_4_MASK     (0x1 << PCIE_PL_SAFETY_INT_MASK_4_POSITION)
#define PCIE_PL_SAFETY_INT_MASK_5_MASK     (0x1 << PCIE_PL_SAFETY_INT_MASK_5_POSITION)

#define PCIE_PL_SAFETY_INT_STATUS_0_POSITION 0
#define PCIE_PL_SAFETY_INT_STATUS_1_POSITION 1
#define PCIE_PL_SAFETY_INT_STATUS_2_POSITION 2
#define PCIE_PL_SAFETY_INT_STATUS_3_POSITION 3
#define PCIE_PL_SAFETY_INT_STATUS_4_POSITION 4
#define PCIE_PL_SAFETY_INT_STATUS_5_POSITION 5

#define PCIE_PL_SAFETY_INT_STATUS_0_MASK     (0x1 << PCIE_PL_SAFETY_INT_STATUS_0_POSITION)
#define PCIE_PL_SAFETY_INT_STATUS_1_MASK     (0x1 << PCIE_PL_SAFETY_INT_STATUS_1_POSITION)
#define PCIE_PL_SAFETY_INT_STATUS_2_MASK     (0x1 << PCIE_PL_SAFETY_INT_STATUS_2_POSITION)
#define PCIE_PL_SAFETY_INT_STATUS_3_MASK     (0x1 << PCIE_PL_SAFETY_INT_STATUS_3_POSITION)
#define PCIE_PL_SAFETY_INT_STATUS_4_MASK     (0x1 << PCIE_PL_SAFETY_INT_STATUS_4_POSITION)
#define PCIE_PL_SAFETY_INT_STATUS_5_MASK     (0x1 << PCIE_PL_SAFETY_INT_STATUS_5_POSITION)



#endif  //?#ifndef _PCIE_DUT_H_


