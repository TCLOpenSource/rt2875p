#ifndef __RTK_VAD_COMM_H__
#define __RTK_VAD_COMM_H__

#include <rtd_log/rtd_module_log.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VAD_PROC_DIR                        "rtkvad"
#define VAD_PROC_ENTRY                      "dbg"

#ifdef CONFIG_ARM64
typedef unsigned long DUALADDRESS;
#define PT_UDEC_DUALADDRESS "%lu"
#define PT_DEC_DUALADDRESS "%ld"
#define PT_HEX_DUALADDRESS "%lx"
#else
typedef unsigned int DUALADDRESS;
#define PT_UDEC_DUALADDRESS "%u"
#define PT_DEC_DUALADDRESS "%d"
#define PT_HEX_DUALADDRESS "%x"
#endif

typedef enum {
    VAD_PROC_SETLOGONOFF,
    VAD_PROC_COUNT,                   /* max number */
} VAD_PROC_CMD;

typedef struct _RTK_VAD_DEBUG_CMD {
    const char *vad_cmd_str;
    VAD_PROC_CMD proc_cmd;
} SRTK_VAD_DEBUG_CMD, *PS_RTK_VAD_DEBUG_CMD;

typedef enum {
    RTK_VAD_PITCH_HIGH_SENSITIVITY = 0,
    RTK_VAD_PITCH_MID_SENSITIVITY,
    RTK_VAD_PITCH_LOW_SENSITIVITY,
} RTK_VAD_PITCH_MODE;

extern unsigned char vad_log_onoff;

#define rtd_printk_vad(fmt, args...)  \
{ \
    if (vad_log_onoff) \
    { \
        rtd_printk( fmt, ##args); \
    } \
}

#define VAD_DBG(fmt, args...)          rtd_printk_vad(KERN_DEBUG, "VAD", "VAD_DEBUG, " fmt, ##args);
#define VAD_WARNING(fmt, args...)      rtd_printk_vad(KERN_WARNING, "VAD", "VAD_WARNING, " fmt, ##args);
#define VAD_INFO(fmt, args...)         rtd_printk_vad(KERN_INFO, "VAD", "VAD_INFO, " fmt, ##args);

#ifdef __cplusplus
    }
#endif


#endif  //__RTK_VAD_COMM_H__
