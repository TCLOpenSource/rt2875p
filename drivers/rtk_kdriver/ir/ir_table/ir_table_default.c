#include "ir_input.h"

static IR_USER_KEY rtk_user_custom_default_keys[] = {
    {0, 0x0000, 0xffffffff},
};

IR_USER_KEY_TABLE rtk_user_custom_default_table = {
    .keys = rtk_user_custom_default_keys,
    .size = ARRAY_SIZE(rtk_user_custom_default_keys),
};

