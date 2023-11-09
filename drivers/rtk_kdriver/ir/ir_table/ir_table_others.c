#include "ir_input.h"

#if defined(CONFIG_RTK_KDRV_ANDROID_OTT)
static struct venus_key rtk_android_ott_stb_keys[] = {
    {0xe51a, KEY_POWER},
    {0xe41b, KEY_BACK},
    {0xa758, KEY_MENU},
    {0xf906, KEY_UP},
    {0xa25d, KEY_LEFT},
    {0xa35c, KEY_RIGHT},
    {0xe01f, KEY_DOWN},
    {0xe11e, KEY_ENTER},
    {0xbb44, KEY_VOLUMEUP},
    {0xe916, KEY_VOLUMEDOWN},
    {0xaa55, KEY_PLAY}, //PLAY or PAUSE
    {0xa659, KEY_HOME},
};

struct  venus_key_table rtk_android_ott_stb_key_table = {
    .keys = rtk_android_ott_stb_keys,
    .size = ARRAY_SIZE(rtk_android_ott_stb_keys),
};
#endif

