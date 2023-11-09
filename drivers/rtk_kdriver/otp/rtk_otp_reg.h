#ifndef __RTK_OTP_REG_H__
#define __RTK_OTP_REG_H__

#include <rbus/efuse_reg.h>

#define MAX_OTP_OFST                             (0x800)
#define OTP_DDR_MAP_BASE                         (EFUSE_DATAO_0_reg)
#define OTP_CTRL_PROG                            (EFUSE_EF_CTRL_PROGRAM_reg)
#define OTP_PROG_STATUS1                         (EFUSE_EF_PROGRAM_STATUS1_reg)
#define OTP_PROG_STATUS2                         (EFUSE_EF_PROGRAM_STATUS2_reg)
#define OTP_CTRL                                 (EFUSE_EF_CTRL_reg)
#define OTP_CTRL_CRC_EN                          (EFUSE_EF_CTRL_CRC_EN_reg)
#define OTP_CRC                                  (EFUSE_EF_CRC_reg)
#define OTP_AUTO_RD_REGION                       (EFUSE_EF_AUTO_RD_REGION_reg)

#define OTP_PROG_STATUS_DONE                  (1 << 3)
#define OTP_PROG_STATUS_LOCK                   (1 << 2)
#define OTP_PROG_STATUS_FAIL                     (1 << 1)
#define OTP_PROG_STATUS_SUCCESS              (1 << 0)
#define OTP_PROG_STATUS_CLEAN                  (0x0)

#define OTP_CTRL_PROG_EN_PROG_EFUSE      (0xDEAD)
#define OTP_CTRL_WRITE_EN0                         (1 << 1)
#define OTP_CTRL_AUTO_RD_REQ                    (1 << 0)

#define OTP_CRC_EN(x)                                     ((x) << 0)

#define OTP_AUTO_RD_REGION_START_ADDR(x)    EFUSE_EF_AUTO_RD_REGION_start_addr(((x) & 0xFFFFFFC))
#define OTP_AUTO_RD_REGION_END_ADDR(x)        EFUSE_EF_AUTO_RD_REGION_end_addr(((x) & 0xFFFFFFC))

#define OTP_CRC_CRC_INI                                  (0xFFFF <<16)
#define GET_OTP_CRC_CRC_OUT(x)                    ((x) & 0xFFFF)

#endif

