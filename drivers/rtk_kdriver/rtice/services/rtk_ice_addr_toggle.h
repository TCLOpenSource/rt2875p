#ifndef __RTK_ICE_ADDR_TOGGLE_H__
#define __RTK_ICE_ADDR_TOGGLE_H__

#include "../core/rtk_ice.h"

typedef enum
{
	VSYNC_MEASURE_INTERVAL,
	REFRESH_MEASURE_INTERVAL,
}MEASUR_INTERVAL_SOURCE;

typedef struct
{
	MEASUR_INTERVAL_SOURCE toggleMode;
	unsigned int totlaCount;
	unsigned int curCount;
	struct task_struct *p_thread;/*< thread handler of bw checking.*/
	int f_stop_thread;
	int f_is_running;
}ToggleInfo;
int start_address_toggle(unsigned char * cmd_data,unsigned long cmd_len);
int get_address_toggle(unsigned char * out_buf,unsigned int buf_size);
int stop_address_toggle(void);
/*address toggle*/
#define RTICE_IOMEM_CMD_START_ADDR_TOGGLE\
	RTICE_OP_CODE(RTICE_CMD_GROUP_ID_DMA_STATUS, 4)
#define RTICE_IOMEM_CMD_GET_ADDR_TOGGLE\
	RTICE_OP_CODE(RTICE_CMD_GROUP_ID_DMA_STATUS, 5)
#define RTICE_IOMEM_CMD_EXIT_ADDR_TOGGLE\
	RTICE_OP_CODE(RTICE_CMD_GROUP_ID_DMA_STATUS, 6)

/* Error Code*/
#define RTICE_ADDR_TOGGLE_CMD_LEN_ERROR    RTICE_FUNC_ERR_1(20)
#define RTICE_ADDR_TOGGLE_CREATE_THREAD    RTICE_FUNC_ERR_1(21)
#define RTICE_ADDR_TOGGLE_TIME_OUT         RTICE_FUNC_ERR_1(22)
#define RTICE_ADDR_TOGGLE_INVALID_PARAM    RTICE_FUNC_ERR_1(23)
#endif
