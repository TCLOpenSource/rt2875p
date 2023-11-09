#ifndef __RTICE_BW_H__
#define __RTICE_BW_H__
#include "../core/rtk_ice.h"
#include <linux/math64.h>

#define RTICE_BW_TIMER
#define BW_RESULT_HIS_LEN 256
/*                   self def data type                  */
/*bw check mode*/
typedef enum{
	BW_CHECK_CONTINUE = 0,
	BW_CHECK_SINGLE,
	BW_CHECK_COUNTER,
	BW_CHECK_MODE_SIZE/*< total size of checking mode.*/
}ENUM_BW_CHECK_MODE;
/*bw check status*/
typedef enum{
	BW_CHECK_STATUS_STOP = 0,/*< End or Stop of current check thread.*/
	BW_CHECK_STATUS_RUN,       /*< Check thread is running now.*/

	BW_CHECK_STATUS_SIZE     /*< Total size of check status.*/
}ENUM_BW_CHECK_STATUS;
/*ip bw info data type*/
typedef enum{
	BW_IP_QUERY_CURRENT = 0,/*< query current bw*/
	BW_IP_QUERY_MAX,/*< Maxmum of history bw*/
	BW_IP_QUERY_MIN,/*< Minmum of history bw*/
	BW_IP_QUERY_MEAN,/*< Mean of history bw*/
	BW_IP_QUERY_HISTORY,      /*< query history bw*/
	BW_IP_QUERY_SIZE
} ENUM_BW_QUERY;
/*bw check ip and check result*/
typedef struct _CBwIp{
	unsigned char id;/*< id of current ip*/
	unsigned int m_curBw;/*< current bw*/
	unsigned int m_maxBw;/*< max bw of history checking*/
	unsigned int m_minBw;/*< min bw of history checking*/
	u64 m_sumBw;/*< mean bw of history checking*/
	u64 m_hisLen;/*< Length of valid history result data in sum section.*/
	unsigned short m_curLat;/*current latency*/
	unsigned short m_maxLat;/*max latency*/
	unsigned short m_minLat;/*min latency*/
	u64 m_sumLat;/* sum lat of history checking*/
	u64 m_hisLenLat;/*< Length of valid history result data in sum section.*/
	struct _CBwIp *p_prev;
	struct _CBwIp *p_next;
} CBwIp;
/*bw check param*/
typedef struct{
	struct timer_list *p_timer;/*< timer list for bw checking*/
	unsigned int m_curCnt;/*< Current count.*/
	CBwIp *p_curIp;/*< Point to the next ip that is need checking.*/
	int f_run;/*< whether thread function has been or need to return.*/
	int f_stop;/*< Send stop single to check thread.*/
	int f_stoped;/*< whether current check thread is stoped.*/
/*< Total count of checking (0:means continue check till recieve end check cmd*/
	unsigned char m_count;
	unsigned short m_period;/*< interval between twice check.(ms unit)*/
	unsigned short m_measPeriod;/*< Period of efficient measure.*/
	unsigned char m_grpNum;/*< Number of group for checking.*/
	unsigned int m_ipListCnt;/*< Total count of buffer that can save info of ip.*/
	CBwIp *p_ipHeader;/*< point to the header of ips list..*/
	CBwIp *p_ipTail;/*< point to the tail of ips list..*/
	CBwIp m_total_cycle;
	CBwIp m_total_ack;
	CBwIp m_total_idle;
	CBwIp m_scpu;
	CBwIp m_eff_total;
	CBwIp m_eff_rd;
	CBwIp m_eff_wr;
	CBwIp m_bankConflict;
	CBwIp m_w2rCnt;
	CBwIp m_r2wCnt;
	CBwIp m_total_cycle_dcu2;
	CBwIp m_total_ack_dcu2;
	CBwIp m_total_idle_dcu2;
	CBwIp m_scpu2;
	CBwIp m_eff_total2;
	CBwIp m_eff_rd2;
	CBwIp m_eff_wr2;
	CBwIp m_bankConflict2;
	CBwIp m_w2rCnt2;
	CBwIp m_r2wCnt2;
} CBwCheck;
/*macro def          */
/*OPCode Code*/
/*enter bw check process*/
#define RTICE_IOMEM_CMD_ENTER_BW_CHECK \
	RTICE_OP_CODE(RTICE_CMD_GROUP_BW_CHECK, 0)
/*query bw check result.*/
#define RTICE_IOMEM_CMD_BW_CHECK_RESULT \
	RTICE_OP_CODE(RTICE_CMD_GROUP_BW_CHECK, 1)
/*exit bw check process*/
#define RTICE_IOMEM_CMD_EXIT_BW_CHECK \
	RTICE_OP_CODE(RTICE_CMD_GROUP_BW_CHECK, 2)
/*status of bw check process*/
#define RTICE_IOMEM_CMD_BW_CHECK_STATUS \
	RTICE_OP_CODE(RTICE_CMD_GROUP_BW_CHECK, 3)
/*start check latency process*/
#define RTICE_IOMEM_CMD_START_LATENCY_CHECK \
	RTICE_OP_CODE(RTICE_CMD_GROUP_BW_CHECK, 4)
/*start check latency process*/
#define RTICE_IOMEM_CMD_STOP_LATENCY_CHECK \
	RTICE_OP_CODE(RTICE_CMD_GROUP_BW_CHECK, 5)
/*get latency of all clients and bridges*/
#define RTICE_IOMEM_CMD_GET_ALL_LATENCY \
	RTICE_OP_CODE(RTICE_CMD_GROUP_BW_CHECK, 6)
/* Error Code*/
#define RTICE_IOMEM_BW_INVALID_PARAM			RTICE_FUNC_ERR(0)
#define RTICE_IOMEM_BW_INVALID_MODE			RTICE_FUNC_ERR(1)
#define RTICE_IOMEM_BW_CHECK_BUSY			RTICE_FUNC_ERR(2)
#define RTICE_IOMEM_BW_CHECK_THREAD			RTICE_FUNC_ERR(3)
#define RTICE_IOMEM_BW_WRONG_GROUPNUM		RTICE_FUNC_ERR(4)
#define RTICE_IOMEM_BW_WAKEUP_TIMEOUT		RTICE_FUNC_ERR(5)
#define RTICE_IOMEM_BW_STOP_FAIL			RTICE_FUNC_ERR(6)
#define RTICE_IOMEM_BW_BUSY				RTICE_FUNC_ERR(7)

#endif/*~__RTICE_BW_H__*/
