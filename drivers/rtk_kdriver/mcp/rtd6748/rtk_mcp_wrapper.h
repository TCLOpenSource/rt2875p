#ifndef __RTK_MCP_WRAPPER_H__
#define __RTK_MCP_WRAPPER_H__
#define MCP_MALLOC_ACTION(size)       dvr_malloc_specific(size, GFP_DCU1_LIMIT)
#define MCP_DEFAULT_DMA_BUST_LEN_SETTING         (0)  //don't change

/* REG0 default 0 */
#define MCP_SCTCH_REG0_DUMMY0_KCPU_SUSPENDED             (0x01 << 0)
#define MCP_SCTCH_REG0_DUMMY0_SCPU_SUSPENDED             (0x01 << 1)
#define MCP_SCTCH_REG0_DUMMY0_KCPU_IDLE                         (0x01 << 2)
#define MCP_SCTCH_REG0_DUMMY0_DECLARE_CLK_OFF            (0x01 << 3)

#define DMA_WRITE_FINISH(x)                          (((x) & 0x01) << 16)

#define HW_MAX_SIZE     127  //RSA max key size = (HW_MAX_SIZE + 1) * 4
#define rsa_check_size(size) ((size) == 128 || (size) == 256 || (size) == 384 || (size) == 512)

#endif
