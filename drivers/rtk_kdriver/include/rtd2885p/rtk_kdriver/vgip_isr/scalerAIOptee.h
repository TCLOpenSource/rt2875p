#ifndef SCALER_AI_OPTEE_H
#define SCALER_AI_OPTEE_H

#include <ioctrl/vpq/vpq_cmd_id.h>

bool ai_optee_get_nn_ta_buf(NN_BUF_INFO_T *info);
bool ai_optee_release_nn_ta_buf(NN_BUF_INFO_T *info);
void ai_optee_release_all_ta_buf(void);

bool ai_optee_init(void);
void ai_optee_deinit(void);

#endif

