#ifndef __RTICE_SYSTEM_H__
#define __RTICE_SYSTEM_H__

#include "../core/rtk_ice.h"

#define RTICE_SYS_CMD(x) \
	RTICE_OP_CODE(RTICE_CMD_GROUP_ID_SYSTEM, x)
#define RTICE_SYS_CMD_READ_SYSTEM_INFO \
	RTICE_OP_CODE(RTICE_CMD_GROUP_ID_SYSTEM, 0)
#define RTICE_SYS_CMD_READ_PCB_PARAMETER \
	RTICE_OP_CODE(RTICE_CMD_GROUP_ID_SYSTEM, 1)
#define RTICE_SYS_CMD_SET_BUSERR_IGNORE_ADDR \
	RTICE_OP_CODE(RTICE_CMD_GROUP_ID_SYSTEM, 2)
#define RTICE_SYS_CMD_FS_OPT \
	RTICE_OP_CODE(RTICE_CMD_GROUP_ID_SYSTEM, 3)
#define RTICE_SYS_CMD_AUDIO_PARAM \
	RTICE_OP_CODE(RTICE_CMD_GROUP_ID_SYSTEM, 4)
#define RTICE_SYS_CMD_PWM_PANEL \
	RTICE_OP_CODE(RTICE_CMD_GROUP_ID_SYSTEM, 5)

typedef enum { 
	RTICE_SYSTEM_INFO_ID_CPU_ID =0x00, 
	RTICE_SYSTEM_INFO_ID_BOARD_ID = 0x01, 
	RTICE_SYSTEM_INFO_ID_BOOTLOADER_VERSION = 0x02, 
	RTICE_SYSTEM_INFO_ID_AUDIO_FW_VERSION = 0x03, 
	RTICE_SYSTEM_INFO_ID_VIDEO_FW_VERSION = 0x04, 
	RTICE_SYSTEM_INFO_ID_KERNEL_SOURCE_CODE_INFO = 0x05,
	RTICE_SYSTEM_INFO_ID_PANEL_NAME = 0x06,
	RTICE_SYSTEM_INFO_ID_SYSTEM_PARAMETERS = 0x80,
} RTICE_SYSTEM_INFO_ID;
typedef enum { 
	RTICE_PCB_INFO_SEARCH = 0x00, 
	RTICE_PCB_INFO_GET_ALL_NAME = 0x01,
} RTICE_PCB_INFO_ID;

typedef enum {
	RTICE_FS_OPT_OPEN = 0,
	RTICE_FS_OPT_READ,
	RTICE_FS_OPT_CLOSE,
	RTICE_FS_OPT_WRITE,
	RTICE_FS_OPT_OPEN_RW,
	ENUM_RTICE_FS_OPT_NUM
}ENUM_RTICE_FS_OPT;


/*operator type of audio param*/
typedef enum{
	otRead = 0,
	otWrite,
	ENUM_AUDIO_OPT_TYPE_SIZE
}ENUM_AUDIO_OPT_TYPE;

/*Type of supported audio param*/
typedef enum{
	apVX = 0,
	apDAP,
	apEQBass,
	ENUM_RTICE_AUDIO_PARAM_GROUP_SIZE
}ENUM_RTICE_AUDIO_PARAM_GROUP;
/*enum for indicating param in EQ Bass filter*/
typedef enum{
    aefModuleEn = 0,
    aefFilterEn,
    aefFilterFc,
    aefFilterBw,
    aefFilterGain,
    ENUM_AUDIO_EQBassTreble_FILTER_PARAM_SIZE
}ENUM_AUDIO_EQBassTreble_FILTER_PARAM;


#define RTICE_IOMEM_ERR_INVALID_SYSTEM_INFO	        RTICE_FUNC_ERR(0)
#define RTICE_IOMEM_ERR_INVALID_PCB_PARAMETER		RTICE_FUNC_ERR(1)

#define RTICE_FS_OPT_ERR_OPEN				RTICE_FUNC_ERR(3)
#define RTICE_FS_OPT_ERR_WRONG_SIZE			RTICE_FUNC_ERR(4)
#define RTICE_FS_OPT_ERR_CLOSE				RTICE_FUNC_ERR(5)
#define RTICE_AUDIO_VX_UNSUPPORT			RTICE_FUNC_ERR(7)
#define RTICE_AUDIO_VX_FAILED				RTICE_FUNC_ERR(8)
#define RTICE_AUDIO_EQ_UNSUPPORT_PARAM		RTICE_FUNC_ERR(9)
#define RTICE_AUDIO_EQ_AUDIO_DRIVER_ERR		RTICE_FUNC_ERR(10)


#ifdef DEBUG_RTICE_SYS
#define RTICE_SYS_DEBUG(fmt, args...)   rtd_pr_rtice_debug(fmt, ##args)
#else	/*  */
#define RTICE_SYS_DEBUG(args...)
#endif	/*  */

#endif
