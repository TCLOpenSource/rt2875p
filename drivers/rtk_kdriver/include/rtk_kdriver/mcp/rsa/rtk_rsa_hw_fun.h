#ifndef __RTK_RSA_HW_FUN_H__
#define __RTK_RSA_HW_FUN_H__

#include <linux/mpi.h>
#include <rbus/cp_reg.h>

#define TYPE_HW_MONT    0
#define TYPE_HW_IMONT   1

#define REG_RSA_X       CP_RSA_X_0_reg
#define REG_RSA_Y       CP_RSA_Y_0_reg
#define REG_RSA_M       CP_RSA_M_0_reg
#define REG_RSA_A       CP_RSA_A_0_reg
#define REG_RSA_CTRL    CP_RSA_CTRL_reg
#define REG_RSA_NUM     CP_RSA_NUM_reg
#define REG_RSA_CLEAR   CP_RSA_CLEAR_reg

#define RSA_go          CP_RSA_CTRL_rsa_go_mask
#define RSA_X2Y         CP_RSA_CTRL_rsa_xeqy_mask
#define RSA_A2X         CP_RSA_CTRL_rsa_xeqa_mask

#define RSA_clear_a     CP_RSA_CLEAR_rsa_clear_a_mask
#define RSA_clear_m     CP_RSA_CLEAR_rsa_clear_m_mask
#define RSA_clear_y     CP_RSA_CLEAR_rsa_clear_y_mask
#define RSA_clear_x     CP_RSA_CLEAR_rsa_clear_x_mask
#define RSA_clear_all   (RSA_clear_a | RSA_clear_m | RSA_clear_y | RSA_clear_x)

int rtk_rsa_get_r2(MPI mod, MPI r2);
int hwsu_mont_exp_fast(
    MPI out, MPI in, MPI mod, MPI r2, MPI key);

int rtk_rsa_core_init(void);
int rtk_rsa_core_exit(void);

#endif
