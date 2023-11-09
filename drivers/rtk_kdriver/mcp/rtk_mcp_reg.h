#ifndef __MCP_H__
#define __MCP_H__

#include <rtk_kdriver/io.h>
#include <rbus/cp_reg.h>

#define MCP_DEV_FILE_NAME                       "mcp/core"
#define MCP_DESC_ENTRY_COUNT                    512

#define MCP_DESCRIPTOR_HW_ALIGN                      64

#define MCP_INTERRUPT_ENABLE

/* =============================================================================== */
/*  REGISTER MAP                                                                                                                                   */
/* =============================================================================== */
#define MCP_GO                                          (0x01 << 1)
#define MCP_IDEL                                        (0x01 << 2)
#define MCP_SWAP                                        (0x01 << 3)
#define MCP_CLEAR                                       (0x01 << 4)
/* 0 : run-robin for all
*  1 : run-robin for TP0/1 and put priotity as TP > MD > NF > MCP
*/
#define MCP_ARB_MODE(x)                                 ((x & 0x01)<<6)

#define MCP_WRITE_DATA                                  (0x01 << 0)
#define MCP_RING_EMPTY                                  (0x01 << 1)
#define MCP_ERROR                                       (0x01 << 2)
#define MCP_COMPARE                                     (0x01 << 3)
#define MCP_ACP_ERR                                     (0x01 << 6)
#define MCP_K_RING_EMPTY                                (0x01 << 7)
#define MCP_K_COMPARE                                   (0x01 << 8)
#define MCP_K_ERROR                                     (0x01 << 9)
#define MCP_MODE_ERR                                    (0x01 << 10)
#define MCP_CHAIN_ERR                                   (0x01 << 11)
#define MCP_BC_ERR                                      (0x01 << 12)
#define MCP_OTP_ERR                                     (0x01 << 13)
#define MCP_ENDE_ERR                                    (0x01 << 14)
#define MCP_KEY_SRC_ERR                                 (0x01 << 15)
#define MCP_DESC_ERR                                    (0x01 << 16)
#define MCP_ADR_ERR                                     (0x01 << 17)

#define MCP_ROUND_NO(x)                                 ((x & 0xFF))
#define MCP_CSA_ENTROPY(x)                              ((x & 0x3) << 8)
#define ORIGIONAL_MODE                                      (0x0)
#define CSA_MODE_1                                          (0x1)
#define CSA_MODE_2                                          (0x2)
#define MCP_AES_PAD_OFF(x)                              ((x & 0x1) << 10)

#define MCP_DMA_BURST_LEN(x)                       (((x) & 0x3F) << 16)


#define OTP_LOAD                                    (0x01 << 0)


/* =============================================================================== */
/*  Descriptor Definition                                                                                                                  */
/* =============================================================================== */
#define MCP_ALGO_LOAD_MICRO_CODE                    (0x0E)
#define MCP_ALGO_MICRO_CODE                         (0x0F)

#define MCP_PATTERN(x)                              ((x & 0x3) << 15)
#define MCP_FULL_CRYPT                              (0x00)
#define MCP_FULL_SKIP                               (0x01)
#define MCP_CRYPT_SKIP                              (0x02)

#define MCP_KEY_SEL(x)                              ((x & 0x03) << 12)
#define MCP_KEY_SEL_CW                                  (0x02)
#define MCP_KEY_SEL_OTP                                 (0x01)
#define MCP_KEY_SEL_DESC                                (0x00)
#define MCP_GET_KEY_SEL_FLAG(x)             (((x) >> 12) & 0x3)                 

#define MCP_IV_SEL3(x)                               ((x & 0x01) << 17)
#define MCP_IV_SEL3_DDR                                 (0x01)
#define MCP_IV_SEL3_SEL                                 (0x00)

#define MCP_IV_SEL2(x)                               ((x & 0x01) << 14)
#define MCP_IV_SEL2_CW                                  (0x01)
#define MCP_IV_SEL2_SEL                                 (0x00)

#define MCP_IV_SEL(x)                               ((x & 0x01) << 11)
#define MCP_IV_SEL_REG                                  (0x01)
#define MCP_IV_SEL_DESC                                 (0x00)

#define MCP_DOUBLE_DADR(x)                          ((x & 0x1) << 10)
#define MCP_ONE_DADR                                (0x0)
#define MCP_TWO_DADR                                (0x1)

#define MCP_CW_KEY(x)                               (((x) & 0x7F)<<8)

#define S_RSA_SEL_REG               CP_S_RSA_SEL_reg
#define S_RSA_ST_REG                CP_S_RSA_ST_reg
#define S_RSA_SEL_ENGINE            0x00000001
#define S_RSA_SEL_ENGINE_MASK       0x00000001
#define S_RSA_ENGINE_READY          0x00000001
#define S_RSA_ENGINE_READY_MASK     0x00000001
/* =============================================================================== */
/*  MACROS                                                                                                                                                 */
/* =============================================================================== */
#define MCP_CTRL                                    CP_MCP_CTRL_reg
#define MCP_STATUS                                  CP_MCP_STATUS_reg
#define MCP_EN                                      CP_MCP_EN_reg
#define MCP_BASE                                    CP_MCP_BASE_reg
#define MCP_LIMIT                                   CP_MCP_LIMIT_reg
#define MCP_RDPTR                                   CP_MCP_RDPTR_reg
#define MCP_WRPTR                                   CP_MCP_WRPTR_reg
#define MCP_CTRL1                                   CP_MCP_CTRL1_reg
#define MCP_OTP_LOAD                                CP_CP_OTP_LOAD_reg
#define MCP_AES_INI_KEY0                            CP_MCP_AES_INI_KEY_0_reg
#define MCP_AES_INI_KEY1                            CP_MCP_AES_INI_KEY_1_reg
#define MCP_AES_INI_KEY2                            CP_MCP_AES_INI_KEY_2_reg
#define MCP_AES_INI_KEY3                            CP_MCP_AES_INI_KEY_3_reg
#define MCP_CAME_INI_KEY0                           CP_MCP_CAME_INI_KEY_0_reg
#define MCP_CAME_INI_KEY1                           CP_MCP_CAME_INI_KEY_1_reg
#define MCP_CAME_INI_KEY2                           CP_MCP_CAME_INI_KEY_2_reg
#define MCP_CAME_INI_KEY3                           CP_MCP_CAME_INI_KEY_3_reg
#define MCP_SCTCH                                   CP_CP_SCTCH_reg

#define SET_MCP_CTRL(x)                             rtd_outl(MCP_CTRL, x)
#define SET_MCP_STATUS(x)                           rtd_outl(MCP_STATUS, x)
#define SET_MCP_EN(x)                               rtd_outl(MCP_EN, x)
#define SET_MCP_BASE(x)                             rtd_outl(MCP_BASE, x)
#define SET_MCP_LIMIT(x)                            rtd_outl(MCP_LIMIT, x)
#define SET_MCP_RDPTR(x)                            rtd_outl(MCP_RDPTR, x)
#define SET_MCP_WRPTR(x)                            rtd_outl(MCP_WRPTR, x)
#define SET_MCP_CTRL1(x)                            rtd_outl(MCP_CTRL1, x)
#define SET_MCP_OTP_LOAD(x)                         rtd_outl(MCP_OTP_LOAD, x)
#define SET_MCP_SCTCH(x)                            rtd_outl(MCP_SCTCH, x)
#define SET_S_RSA_SEL(x)                            rtd_outl(S_RSA_SEL_REG, x)

#define GET_MCP_CTRL()                              rtd_inl(MCP_CTRL)
#define GET_MCP_STATUS()                            rtd_inl(MCP_STATUS)
#define GET_MCP_EN()                                rtd_inl(MCP_EN)
#define GET_MCP_BASE()                              rtd_inl(MCP_BASE)
#define GET_MCP_LIMIT()                             rtd_inl(MCP_LIMIT)
#define GET_MCP_RDPTR()                             rtd_inl(MCP_RDPTR)
#define GET_MCP_WRPTR()                             rtd_inl(MCP_WRPTR)
#define GET_MCP_CTRL1()                             rtd_inl(MCP_CTRL1)
#define GET_MCP_OTP_LOAD()                          rtd_inl(MCP_OTP_LOAD)
#define GET_MCP_SCTCH()                             rtd_inl(MCP_SCTCH)
#define GET_MCP_AES_INI_KEY0()                      rtd_inl(MCP_AES_INI_KEY0)
#define GET_MCP_AES_INI_KEY1()                      rtd_inl(MCP_AES_INI_KEY1)
#define GET_MCP_AES_INI_KEY2()                      rtd_inl(MCP_AES_INI_KEY2)
#define GET_MCP_AES_INI_KEY3()                      rtd_inl(MCP_AES_INI_KEY3)
#define GET_MCP_CAME_INI_KEY0()                     rtd_inl(MCP_CAME_INI_KEY0)
#define GET_MCP_CAME_INI_KEY1()                     rtd_inl(MCP_CAME_INI_KEY1)
#define GET_MCP_CAME_INI_KEY2()                     rtd_inl(MCP_CAME_INI_KEY2)
#define GET_MCP_CAME_INI_KEY3()                     rtd_inl(MCP_CAME_INI_KEY3)

#define GET_S_RSA_ST()                              rtd_inl(S_RSA_ST_REG)
#define GET_S_RSA_SEL()                             rtd_inl(S_RSA_SEL_REG)
#endif  // __MCP_REG_H__
