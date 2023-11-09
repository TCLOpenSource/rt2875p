#ifndef __RTK_SVAD_DEF_H__
#define __RTK_SVAD_DEF_H__

#include <rtd_log/rtd_module_log.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SVAD_DEVICE_NAME                     "rtksvad"
#define SVAD_PROC_ENTRY                      "dbg"
//#define MIO_I2C_MODE

enum svad_ioc_cmd
{
    SVAD_IOC_CMD_INIT = 0,
    SVAD_IOC_CMD_HWD_START = 1,
    SVAD_IOC_CMD_HWD_STOP = 2,
    SVAD_IOC_CMD_GET_HWD_RESULT = 3,
    SVAD_IOC_CMD_RESET_HWD_RESULT = 4,
};

/// Function return status
enum FUNCTION_RETURN_STATUS {
	FUNCTION_SUCCESS,			///<   Execute function successfully.
	FUNCTION_ERROR,				///<   Execute function unsuccessfully.
};


#define SVAD_IOC_MAGIC 'z'
#define SVAD_IOC_INIT                        _IO(SVAD_IOC_MAGIC, SVAD_IOC_CMD_INIT)
#define SVAD_IOC_HWD_START                   _IO(SVAD_IOC_MAGIC, SVAD_IOC_CMD_HWD_START)
#define SVAD_IOC_HWD_STOP                    _IO(SVAD_IOC_MAGIC, SVAD_IOC_CMD_HWD_STOP)
#define SVAD_IOC_GET_HWD_RESULT              _IOR(SVAD_IOC_MAGIC, SVAD_IOC_CMD_GET_HWD_RESULT, unsigned int)
#define SVAD_IOC_RESET_HWD_RESULT            _IO(SVAD_IOC_MAGIC, SVAD_IOC_CMD_RESET_HWD_RESULT)

#define MIO_SYS_SPI_ADDR_SHIFT               0x80
#define MIO_SVAD_SPI_ADDR_SHIFT              0x80
#define MIO_PINMUX_SPI_ADDR_SHIFT            0x100

typedef struct {
    unsigned int    hwd_result;
} RTK_SVAD_GET_HWD_RESULT;

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
    SVAD_PROC_SETLOGONOFF,
	SVAD_PROC_SETMODEL01,
    SVAD_PROC_SETMODEL02,
    SVAD_PROC_COUNT,                   /* max number */
} SVAD_PROC_CMD;

typedef struct _RTK_SVAD_DEBUG_CMD {
    const char *svad_cmd_str;
    SVAD_PROC_CMD proc_cmd;
} SRTK_SVAD_DEBUG_CMD, *PS_RTK_SVAD_DEBUG_CMD;

extern unsigned char svad_log_onoff;

#define rtd_printk_svad(fmt, args...)  \
{ \
    if (svad_log_onoff) \
    { \
        rtd_printk( fmt, ##args); \
    } \
}

#define SVAD_DBG(fmt, args...)          rtd_printk_svad(KERN_DEBUG, "SVAD", "SVAD_DEBUG, " fmt, ##args);
#define SVAD_WARNING(fmt, args...)      rtd_printk_svad(KERN_WARNING, "SVAD", "SVAD_WARNING, " fmt, ##args);
#define SVAD_INFO(fmt, args...)         rtd_printk_svad(KERN_INFO, "SVAD", "SVAD_INFO, " fmt, ##args);

#ifdef __cplusplus
    }
#endif

#endif  //__RTK_SVAD_DEF_H__
