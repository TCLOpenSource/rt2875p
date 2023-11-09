#ifndef __RTK_ICE_MC_MEASURE_H__
#define __RTK_ICE_MC_MEASURE_H__
#include "../core/rtk_ice.h"

/*OPCode Code*/
/*start check latency process*/
#define RTICE_IOMEM_CMD_START_LATENCY_CHECK \
    RTICE_OP_CODE(RTICE_CMD_GROUP_BW_CHECK, 4)
/*start check latency process*/
#define RTICE_IOMEM_CMD_STOP_LATENCY_CHECK \
    RTICE_OP_CODE(RTICE_CMD_GROUP_BW_CHECK, 5)
/*get latency of all clients and bridges*/
#define RTICE_IOMEM_CMD_GET_ALL_LATENCY \
    RTICE_OP_CODE(RTICE_CMD_GROUP_BW_CHECK, 6)
/*error code*/
#ifndef RTICE_IOMEM_BW_INVALID_PARAM
#define RTICE_IOMEM_BW_INVALID_PARAM RTICE_FUNC_ERR(0)
#endif

/*error code*/
#define RTICE_LATENCY_CREATE_THREAD	RTICE_FUNC_ERR(8) /*fail to create checking thread*/
#define RTICE_LATENCY_THREAD_UNRUN	RTICE_FUNC_ERR(9) /*timeout while waiting running of the thread*/
#define RTICE_LATENCY_THREAD_UNSTOP	RTICE_FUNC_ERR(10)/*timeout while waiting stop of the thread*/
#define RTICE_LATENCY_ADDR_SIZE_ERR	RTICE_FUNC_ERR(11)/*params error of getting latency data*/
/** start thread for checking latency of all bridges and clients */
extern int startCheckLatency(unsigned int period,unsigned char fReadOnly);
/** stop checking thread*/
extern int stopCheckLatency(void);
/** read latency buffer start from 'addr' till 'size' bytes,saving latency to 'pOut'.And returns error code or length of readed data*/
extern int getClientsLatency(unsigned int addr,unsigned int size,unsigned char*pOut);
#endif
