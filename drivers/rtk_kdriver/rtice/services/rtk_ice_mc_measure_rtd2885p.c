#include "rtk_ice_mc_measure.h"
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <io.h>

#define CONFIG_RTICE_CPU_LOADING
#ifdef CONFIG_RTICE_CPU_LOADING
#include <linux/sched/cputime.h>
#include <linux/limits.h>
#include <linux/fs.h>
#include <linux/kernel_stat.h>
#include <rtk_kdriver/rtk_clock.h>
#include <rtk_kdriver/rtk_thermal_sensor.h>
#include <rtk_kdriver/rtk_ai.h>
#include <rtd_log/rtd_module_log.h>
#endif

/*****************************MC Measurement**********************************/
/*count of mc mear*/
#define RTICE_DBUS_BUS_ID_NUM 16
#define RTICE_DBUS_CLIENT_ID_NUM 16
#define RTICE_MC_TOTAL_NUM 2
/*data buffer for mc mear*/
#if IS_ENABLED(CONFIG_RTK_KDRV_QOS_CONTROL)
#include <rtk_kdriver/rtk_qos_export.h>
extern unsigned int total_measure_frames;
extern unsigned char *qos_bw_measure_p[RTICE_MC_TOTAL_NUM];
#endif

static DECLARE_WAIT_QUEUE_HEAD (rtice_mc_wait_main);

/**
 * mc mear param
*/
typedef struct
{
    struct task_struct *p_thread;/*< thread handler of bw checking.*/
    int m_fRunning;
    int m_errCode;
    unsigned int m_checkTime;//measure time(s) in continue mode
}CMcMeasure;
static CMcMeasure g_McMeasure = {
    NULL,//thread handler of bw checking
    false,//is_running
    0, //error_code
    1//check time in qos
};

/**
 * Returns total frames count in measurement
 */
static unsigned int mearTotalFrames(void)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_QOS_CONTROL)
    return total_measure_frames;
#else
    return 0;
#endif
}

static unsigned int mearTotalSize(void)
{
    unsigned int frameCnt = mearTotalFrames();

    if(g_McMeasure.m_fRunning)/*exit while mear is running*/
        return 0;
#if IS_ENABLED(CONFIG_RTK_KDRV_QOS_CONTROL)
    return frameCnt * sizeof(SRAM_RECORD_t) * RTICE_MC_TOTAL_NUM;;
#else
    return frameCnt;
#endif
}

/**
 * Dump pure data using in mc measure.Returns count of dumped data or error code
 */
static int mearDataDump(unsigned int offset,unsigned int size,unsigned char*pOutBuf)
{
    unsigned int totalSize = mearTotalSize();

    if(!pOutBuf || (offset + size) > totalSize)
        return -RTICE_MC_MEASURE_INVALID_PARAM;
#if IS_ENABLED(CONFIG_RTK_KDRV_QOS_CONTROL)
    /*copy data by mc*/
    {
        unsigned int mcSize = totalSize / RTICE_MC_TOTAL_NUM;
        unsigned int mcIdx = offset / mcSize;
        unsigned int innerOffset = offset % mcSize;
        unsigned int rSize = 0;

        while(mcIdx < RTICE_MC_TOTAL_NUM && rSize < size){
            unsigned int nSize = size - rSize;
            unsigned char *pBuf = qos_bw_measure_p[mcIdx] + innerOffset;

            if((innerOffset + nSize) > mcSize)
                nSize = mcSize - innerOffset;
            if(!qos_bw_measure_p[mcIdx])
                return -RTICE_MC_MEASURE_INVALID_PARAM;
            memcpy(pOutBuf + rSize,pBuf,nSize);
            rSize += nSize;
            /*next mc*/
            mcIdx++;
            innerOffset = 0;
        }
        return rSize;
    }
#else
    return totalSize;
#endif
}

/**
 * @brief mearCount Returns read or write count of an client in measurement
 * @param fRead 0:write,otherwise:read
 * @param frame frame index
 * @param busId bus id
 * @param clientId client id
 * @param mc mc index
 */
static unsigned int mearCount(int fRead,unsigned int fIdx,unsigned char busId,unsigned char clientId,unsigned char mc)
{
    unsigned int frameCnt = mearTotalFrames();

    if(fIdx >= frameCnt || busId >= RTICE_DBUS_BUS_ID_NUM || clientId >= RTICE_DBUS_CLIENT_ID_NUM || mc >= RTICE_MC_TOTAL_NUM)
        return 0;
#if IS_ENABLED(CONFIG_RTK_KDRV_QOS_CONTROL)
    if(mc >= RTICE_MC_TOTAL_NUM || qos_bw_measure_p[mc] == NULL)
        return 0;
    {
        SRAM_RECORD_t *pRec = (SRAM_RECORD_t*)(qos_bw_measure_p[mc] + fIdx * sizeof(SRAM_RECORD_t));
        if(fRead)
            return pRec->r_count[busId][clientId];
        else
            return pRec->w_count[busId][clientId];
    }
#else
    return 0;
#endif
}

static unsigned int mearReadCount(unsigned int fIdx,unsigned char busId,unsigned char clientId,unsigned char mc)
{
    return mearCount(1,fIdx,busId,clientId,mc);
}

static unsigned int mearWriteCount(unsigned int fIdx,unsigned char busId,unsigned char clientId,unsigned char mc)
{
    return mearCount(0,fIdx,busId,clientId,mc);
}

/**
 * Thread entry for calling qos mears.
 */
static int mc_measure_thread(void * data)
{
	CMcMeasure * pmc = (CMcMeasure*)data;

	if(pmc == NULL)
		return -RTICE_MC_MEASURE_INVALID_PARAM;
	RTICE_DEBUG("[%s:%d]start mc_measure_thread\r\n", __func__,__LINE__);
    pmc->m_fRunning = true;
	wake_up(&rtice_mc_wait_main);
	/*mc measure flow*/
#if IS_ENABLED(CONFIG_RTK_KDRV_QOS_CONTROL)
    {
        /*start mearue in qos module*/
        char buf[64];

        RTICE_DEBUG("[%s:%d]Now is continue mode.\r\n", __func__,__LINE__);
        memset(buf,0,64);
        sprintf(buf,"bw_start@%d",pmc->m_checkTime);
        pmc->m_errCode = rtk_qos_mode_parsing(buf);
    }
#else
    pmc->m_errCode = -1;
#endif
	/*finish and exit from thread*/
    pmc->m_fRunning = false;
	wake_up(&rtice_mc_wait_main);
	RTICE_DEBUG("[%s:%d]mc_measure_thread stoped\r\n", __func__,__LINE__);
	return 0;
}

/**
 * Start measure thread according to the given cmd
 */
int enter_mc_measure(unsigned char*pCmdData,unsigned int cmdLen)
{
    int nres = exit_mc_masure();

    if(nres)
        return nres;
    /*init param of mc measure*/
    if(cmdLen != 5)
        return -RTICE_MC_MEASURE_CMD_LEN_ERROR;
    if(!pCmdData || pCmdData[0] != 0x85)
        return -RTICE_MC_MEASURE_INVALID_PARAM;
    g_McMeasure.m_fRunning = false;
    g_McMeasure.m_checkTime = B4TL(pCmdData[1],pCmdData[2],pCmdData[3],pCmdData[4]);
    g_McMeasure.m_errCode = 0;
    /*create and start mear thread*/
	g_McMeasure.p_thread = kthread_run(mc_measure_thread,&g_McMeasure,"mc_measure_thread");
    nres = wait_event_timeout(rtice_mc_wait_main,(g_McMeasure.m_fRunning == true),100);
	if(!nres){
		RTICE_DEBUG("[%s:%d]Failed start mc_measure_thread.\r\n", __func__,__LINE__);
		return -RTICE_MC_MEASURE_CREATE_THREAD;
	}
	return 0;
}

/**
 * Exit from measure thread
 */
int exit_mc_masure(void)
{
	int nres = 0;

    if(g_McMeasure.m_fRunning){
        nres = wait_event_timeout(rtice_mc_wait_main,(g_McMeasure.m_fRunning==false),100);
		if(!nres){
			RTICE_DEBUG("[%s:%d]Failed stop mc_measure_thread.\r\n", __func__,__LINE__);
            return -RTICE_MC_MEASURE_TIME_OUT;
		}
	}
	g_McMeasure.p_thread = NULL;
	return 0;
}

/*!
 * lock or unlock qos mode.Returns negative while there is something wrong
 */
int setLockQosMode(int fLock)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_QOS_CONTROL)
    if(fLock)
        rtk_qos_lock_current_mode();
    else
        rtk_qos_unlock_current_mode();
    return 0;
#else
    return 0;
#endif
}
/*!
 * get description about current qos mode.Returns length of the string while everything is ok ,otherwise returns negative
 */
int currentQosMode(unsigned char*pOutBuf,unsigned int outBufLen)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_QOS_CONTROL)
    char * const pModeStr = rtk_qos_get_current_mode();

    if(!pOutBuf || outBufLen < strlen(pModeStr))
        return -RTICE_MC_MEASURE_OUT_BUF_SIZE_SMALL;
    memcpy(pOutBuf,pModeStr,strlen(pModeStr));
    return strlen(pModeStr);
#else
    return -1;
#endif
}

/**
 * @brief clientCount Dump mc count belong to the given client.
 * @param busId,clientId id of the client
 * @param bQueryMax Whether dump maximum frame,othewise dump current frame
 * @param pOutBuf Save dumped data
 * @return Returns length of the data saved in pOutBuf
 */
int clientCount(unsigned char busId,unsigned char clientId,int type,unsigned char*pOutBuf,unsigned int outBufLen)
{
    int ret = 0;
    unsigned char m = 0;
    unsigned int f = 0,fIdx = 0,frameCnt = 0,maxCnt = 0,cnt = 0;

    frameCnt = mearTotalFrames();
    if(busId >= RTICE_DBUS_BUS_ID_NUM || clientId >= RTICE_DBUS_CLIENT_ID_NUM)
        return -RTICE_MC_MEASURE_INVALID_PARAM;
    if(g_McMeasure.m_fRunning || frameCnt == 0)
        return -RTICE_MC_MEASURE_QOS_MEAR_ONGOING;
    if(g_McMeasure.m_errCode < 0)
        return -RTICE_MC_MEASURE_QOS_MEAR_ERROR;
    if(!pOutBuf || outBufLen < 8 * RTICE_MC_TOTAL_NUM)
        return -RTICE_MC_MEASURE_OUT_BUF_SIZE_SMALL;
    /*calculate max and total count of the bus*/
    for(f = 0;f < frameCnt && type != 0;f++){
        cnt = 0;
        for(m = 0;m < RTICE_MC_TOTAL_NUM;m++)
            cnt += mearReadCount(f,busId,clientId,m) + mearWriteCount(f,busId,clientId,m);
        if(cnt > maxCnt){
            maxCnt = cnt;
            fIdx = f;
        }
    }
    /*dump max and avg one mc by another*/
    for(m = 0;m < RTICE_MC_TOTAL_NUM;m++){
        unsigned char*pBuf = pOutBuf + 8 * m;
        /*read*/
        cnt = mearReadCount(fIdx,busId,clientId,m);
        LTB4(cnt,pBuf[0],pBuf[1],pBuf[2],pBuf[3]);
        /*write*/
        cnt = mearWriteCount(fIdx,busId,clientId,m);
        LTB4(cnt,pBuf[4],pBuf[5],pBuf[6],pBuf[7]);
        /*next mc*/
        ret += 8;
    }
    /*finish*/
    return ret;
}

/**
 * @brief busStatisticCount Dump max and avg count belong to the given bus on mc by another
 * @param busId Indicate the bus
 * @param pOutBuf,outBufLen output buffer
 * @return Returns length of dumped data.Returns negative while something wrong
 */
int busStatisticCount(unsigned char busId,unsigned char*pOutBuf,int outBufLen)
{
    int ret = 0;
    unsigned int f = 0,frameCnt = 0;
    unsigned char m = 0,c = 0;
    unsigned int maxCnt = 0,cnt = 0,totalCnt = 0;
    unsigned int maxCntMCs[RTICE_MC_TOTAL_NUM];
    unsigned int totalCntMCs[RTICE_MC_TOTAL_NUM];
    unsigned char*pBuf = NULL;

    frameCnt = mearTotalFrames();
    if(busId >= RTICE_DBUS_BUS_ID_NUM)
        return -RTICE_MC_MEASURE_INVALID_PARAM;
    if(g_McMeasure.m_fRunning || frameCnt == 0)
        return -RTICE_MC_MEASURE_QOS_MEAR_ONGOING;
    if(g_McMeasure.m_errCode < 0)
        return -RTICE_MC_MEASURE_QOS_MEAR_ERROR;
    if(!pOutBuf || outBufLen < 8 * (RTICE_MC_TOTAL_NUM + 1))
        return -RTICE_MC_MEASURE_OUT_BUF_SIZE_SMALL;
    /*calculate max and total count of the bus*/
    memset(maxCntMCs,0,sizeof(maxCntMCs));
    memset(totalCntMCs,0,sizeof(totalCntMCs));
    for(f = 0;f < frameCnt;f++){
        unsigned int cntMCs[RTICE_MC_TOTAL_NUM];
        cnt = 0;
        for(m = 0;m < RTICE_MC_TOTAL_NUM;m++){
            cntMCs[m] = 0;
            for(c = 0;c < RTICE_DBUS_CLIENT_ID_NUM;c++)
                cntMCs[m] += mearReadCount(f,busId,c,m) + mearWriteCount(f,busId,c,m);
            totalCntMCs[m] += cntMCs[m];
            cnt += cntMCs[m];
        }
        totalCnt += cnt;
        if(cnt > maxCnt){
            maxCnt = cnt;
            memcpy(maxCntMCs,cntMCs,sizeof(cntMCs));
        }
    }
    /*dump max and avg one mc by another*/
    for(m = 0;m < RTICE_MC_TOTAL_NUM;m++){
        pBuf = pOutBuf + 8 * m;
        /*max*/
        cnt = maxCntMCs[m];
        LTB4(cnt,pBuf[0],pBuf[1],pBuf[2],pBuf[3]);
        /*avg*/
        cnt = totalCntMCs[m] / frameCnt;
        LTB4(cnt,pBuf[4],pBuf[5],pBuf[6],pBuf[7]);
        /*next mc*/
        ret += 8;
    }
    /*dump total max and avg*/
    pBuf = pOutBuf + 8 * RTICE_MC_TOTAL_NUM;
    cnt = maxCnt;
    LTB4(cnt,pBuf[0],pBuf[1],pBuf[2],pBuf[3]);
    cnt = totalCnt / frameCnt;
    LTB4(cnt,pBuf[4],pBuf[5],pBuf[6],pBuf[7]);
    ret += 8;
    /*finish*/
    return ret;
}

/**
 * @brief clientStatisticCount Dump max and avg count belong to the given client on mc by another
 * @param busId,clientId Indicate the client
 * @param pOutBuf,outBufLen output buffer
 * @return Returns length of dumped data.Returns negative while something wrong
 */
int clientStatisticCount(unsigned char busId,unsigned char clientId,unsigned char*pOutBuf,int outBufLen)
{
    int ret = 0;
    unsigned int f = 0,fIdx = 0,frameCnt = 0;
    unsigned char m = 0;
    unsigned int maxCnt = 0,cnt = 0,totalCnt = 0;
    unsigned int totalCntMCs[RTICE_MC_TOTAL_NUM];
    unsigned char*pBuf = NULL;

    frameCnt = mearTotalFrames();
    if(busId >= RTICE_DBUS_BUS_ID_NUM || clientId >= RTICE_DBUS_CLIENT_ID_NUM)
        return -RTICE_MC_MEASURE_INVALID_PARAM;
    if(g_McMeasure.m_fRunning || frameCnt == 0)
        return -RTICE_MC_MEASURE_QOS_MEAR_ONGOING;
    if(g_McMeasure.m_errCode < 0)
        return -RTICE_MC_MEASURE_QOS_MEAR_ERROR;
    if(!pOutBuf || outBufLen < 8 * (RTICE_MC_TOTAL_NUM + 1))
        return -RTICE_MC_MEASURE_OUT_BUF_SIZE_SMALL;
    /*calculate max and total count of the bus*/
    memset(totalCntMCs,0,sizeof(totalCntMCs));
    for(f = 0;f < frameCnt;f++){
        cnt = 0;
        for(m = 0;m < RTICE_MC_TOTAL_NUM;m++){
            unsigned int tcnt = mearReadCount(f,busId,clientId,m) + mearWriteCount(f,busId,clientId,m);
            totalCntMCs[m] += tcnt;
            cnt += tcnt;
        }
        totalCnt += cnt;
        if(cnt > maxCnt){
            maxCnt = cnt;
            fIdx = f;
        }
    }
    /*dump max and avg one mc by another*/
    for(m = 0;m < RTICE_MC_TOTAL_NUM;m++){
        pBuf = pOutBuf + 8 * m;
        /*max*/
        cnt = mearReadCount(fIdx,busId,clientId,m) + mearWriteCount(fIdx,busId,clientId,m);
        LTB4(cnt,pBuf[0],pBuf[1],pBuf[2],pBuf[3]);
        /*avg*/
        cnt = totalCntMCs[m] / frameCnt;
        LTB4(cnt,pBuf[4],pBuf[5],pBuf[6],pBuf[7]);
        /*next mc*/
        ret += 8;
    }
    /*dump total max and avg*/
    pBuf = pOutBuf + 8 * RTICE_MC_TOTAL_NUM;
    cnt = maxCnt;
    LTB4(cnt,pBuf[0],pBuf[1],pBuf[2],pBuf[3]);
    cnt = totalCnt / frameCnt;
    LTB4(cnt,pBuf[4],pBuf[5],pBuf[6],pBuf[7]);
    ret += 8;
    /*finish*/
    return ret;
}

/*****************************Loading Measurement -- Testing code**********************************/
#ifdef CONFIG_RTICE_CPU_LOADING
/**
 * gScpuLoadingData is address of struc system_info[sec + 1] in the latest measurement
 * gScpuLoadingDataLen is sizeof struct system_info[sec + 1]
 */
unsigned char*gScpuLoadingData = NULL;
unsigned int gScpuLoadingDataLen = 0;

#define NUM_OF_CORE 16
typedef struct system_info{
	int bfreq;	//big core frequecny
	int lfreq;	//little core frequecy
	int degree;	//temperature
	int cpu_nums; 	//number of cpus
	char gpu_loading[16];
	uint64_t system_idle;  //system idle count in period
	uint64_t system_total; //system total count in period
	uint64_t cpu_idle[NUM_OF_CORE];  //cpu idle count in period
	uint64_t cpu_total[NUM_OF_CORE]; //cpu total count in period.
	int npu_usage; //npu usage in period. 0~99
}system_info_t;

#define REGS_GPU_WRAPPER_DUMMY_RW (0xb810E060)

void sampleLoadingMear(system_info_t *pInfo, uint64_t *pIdle, uint64_t *pTotal) {
	int i;
	uint64_t idle, total;
	int gpu_loading;

	pInfo->bfreq = 0;
	for(i = 3; i < 7; i++)
	{
		if(cpu_online(i)){
			pInfo->bfreq = cpu_to_le32(rtk_get_speed(i));
			break;
		}
	}
	pInfo->lfreq = 0;
	for(i = 0; i < 2; i++)
	{
		if(cpu_online(i)){
			pInfo->lfreq = cpu_to_le32(rtk_get_speed(i));
			break;
		}
	}
	pInfo->degree = rtk_get_thermal_value();

	for_each_present_cpu(i) {
		if(i >= NUM_OF_CORE)
			break;
		pInfo->cpu_nums++;
		if(cpu_online(i)) {
			struct kernel_cpustat *kcs = &kcpustat_cpu(i);
			idle = kcs->cpustat[CPUTIME_IDLE];
			total = kcs->cpustat[CPUTIME_USER] +
				kcs->cpustat[CPUTIME_NICE] +
				kcs->cpustat[CPUTIME_SYSTEM] +
				kcs->cpustat[CPUTIME_IOWAIT] +
				kcs->cpustat[CPUTIME_IDLE] +
				kcs->cpustat[CPUTIME_STEAL] +
				kcs->cpustat[CPUTIME_NICE] +
				kcs->cpustat[CPUTIME_SOFTIRQ] +
				kcs->cpustat[CPUTIME_IRQ];
			if(idle >= pIdle[i]) {
				pInfo->cpu_idle[i] = cpu_to_le64(idle - pIdle[i]);
				pInfo->system_idle += (idle - pIdle[i]);
			} else {
				pInfo->cpu_idle[i] = cpu_to_le64(ULLONG_MAX - pIdle[i] + idle);
				pInfo->system_idle += (ULLONG_MAX - pIdle[i] + idle);
			}

			if(total >= pTotal[i]) {
				pInfo->cpu_total[i] = cpu_to_le64(total - pTotal[i]);
				pInfo->system_total += (total - pTotal[i]);
			} else {
				pInfo->cpu_total[i] = cpu_to_le64(ULLONG_MAX - total + pTotal[i]);
				pInfo->system_total += (ULLONG_MAX - total + pTotal[i]);
			}
#if 0
			rtd_pr_rtice_info("%s %d cpu %d pInfo->cpu_idle[i] %llx idle %llx pIdle %llx\n",
				__func__, __LINE__, i, pInfo->cpu_idle[i], idle, pIdle[i]);
			rtd_pr_rtice_info("%s %d cpu %d pInfo->cpu_total[i] %llx total %llx pTotal %llx\n",
				__func__, __LINE__, i, pInfo->cpu_total[i], total, pTotal[i]);
#endif
			pIdle[i] = idle;
			pTotal[i] = total;
		}
	}
	gpu_loading = rtd_inl(REGS_GPU_WRAPPER_DUMMY_RW);
	if((gpu_loading > 100) || (gpu_loading < 0)) {
		pInfo->gpu_loading[0] = '\0';
	} else {
		sprintf(pInfo->gpu_loading, "%d", gpu_loading);
	}
	cpu_to_le64s(&(pInfo->system_idle));
	cpu_to_le64s(&(pInfo->system_total));

#if 0
	rtd_pr_rtice_info("%s %d system_idle[i] %llx system_total %llx\n",
		__func__, __LINE__, pInfo->system_idle, pInfo->system_total);
#endif
}

/**
 * @brie mple(2) MM and PC
 * startLoadingMear Start flow about scpu loading,freq,and temp measurement
 * @param sec The duration time of the measurement
 * @return Returns error number while there is something wrong,othewise returns zero.
 */
int startLoadingMear(int sec)
{
	int i = 0;
	uint64_t cpu_idle[NUM_OF_CORE];
	uint64_t cpu_total[NUM_OF_CORE];
	/*free buffer in the last mear*/
	if(gScpuLoadingData)
		vfree(gScpuLoadingData);

	gScpuLoadingData = NULL;
	gScpuLoadingDataLen = 0;
	/*create data buffer for mear*/
	gScpuLoadingDataLen = sizeof(system_info_t) * (sec + 1);
	gScpuLoadingData = (unsigned char*)vmalloc(gScpuLoadingDataLen);
	if (gScpuLoadingData == NULL){
		rtd_pr_rtice_err("%s:%d [ERR] gScpuLoadingData is NULL\n", __FUNCTION__, __LINE__);
		return 1;
	}
	memset(gScpuLoadingData, 0, gScpuLoadingDataLen);
	for_each_present_cpu(i) {
		if(cpu_online(i)) {
			struct kernel_cpustat *kcs = &kcpustat_cpu(i);
			cpu_idle[i] = kcs->cpustat[CPUTIME_IDLE];
			cpu_total[i] = kcs->cpustat[CPUTIME_USER] +
				kcs->cpustat[CPUTIME_NICE] +
				kcs->cpustat[CPUTIME_SYSTEM] +
				kcs->cpustat[CPUTIME_IOWAIT] +
				kcs->cpustat[CPUTIME_IDLE] +
				kcs->cpustat[CPUTIME_STEAL] +
				kcs->cpustat[CPUTIME_NICE] +
				kcs->cpustat[CPUTIME_SOFTIRQ] +
				kcs->cpustat[CPUTIME_IRQ];
#if 0
			rtd_pr_rtice_info("%s %d cpu %d cpu_idle[i] %llx\n",
				__func__, __LINE__, i, cpu_idle[i]);
			rtd_pr_rtice_info("%s %d cpu %d cpu_total[i] %llx\n",
				__func__, __LINE__, i, cpu_total[i]);
#endif
		}
	}

	#if IS_ENABLED(CONFIG_RTK_AI_DRV)
	rtk_ai_mc_enableNpuLoadCheck(1);
	#endif

	/*start measure per second*/
	for(i = 1;i <= sec;i++){
		system_info_t *pInfo = (system_info_t*)(gScpuLoadingData + sizeof(system_info_t) * i);
		msleep(1000);
		sampleLoadingMear(pInfo, cpu_idle, cpu_total);
		#if IS_ENABLED(CONFIG_RTK_AI_DRV)
		pInfo->npu_usage=rtk_ai_mc_getNpuLoad();
		#else
		pInfo->npu_usage=0;
		#endif
	}

	#if IS_ENABLED(CONFIG_RTK_AI_DRV)
	rtk_ai_mc_enableNpuLoadCheck(0);
	#endif


	/*finish*/
	return 0;
}

/*****************************Loading Measurement**********************************/
static DECLARE_WAIT_QUEUE_HEAD (rtice_loading_wait_main);
/**
 * Parameters for loading measurement
 */
static CMcMeasure g_loadingMeasure = {
    NULL,//thread handler of bw checking
    false,//is_running
    0, //error_code
    1//check time in qos
};
static int loading_measure_thread(void * data);

/**
 * Exit from measure thread
 */
int exit_loading_masure(void)
{
	int nres = 0;

    if(g_loadingMeasure.m_fRunning){
        nres = wait_event_timeout(rtice_loading_wait_main,(g_loadingMeasure.m_fRunning==false),100);
		if(!nres){
			RTICE_DEBUG("[%s:%d]Failed stop loading_measure_thread.\r\n", __func__,__LINE__);
            return -RTICE_LOADING_MEASURE_TIME_OUT;
		}
	}
	g_loadingMeasure.p_thread = NULL;
	return 0;
}

/**
 * Start measure thread according to the given cmd
 */
int enter_loading_measure(unsigned char*pCmdData,unsigned int cmdLen)
{
    int nres = exit_loading_masure();

    if(nres)
        return nres;
    /*init param of mc measure*/
    if(cmdLen != 5)
        return -RTICE_MC_MEASURE_CMD_LEN_ERROR;
    if(!pCmdData || pCmdData[0] != 0x86)
        return -RTICE_MC_MEASURE_INVALID_PARAM;
    g_loadingMeasure.m_fRunning = false;
    g_loadingMeasure.m_checkTime = B4TL(pCmdData[1],pCmdData[2],pCmdData[3],pCmdData[4]);
    g_loadingMeasure.m_errCode = 0;
    /*create and start mear thread*/
	g_loadingMeasure.p_thread = kthread_run(loading_measure_thread,&g_loadingMeasure,"loading_measure_thread");
	if(g_loadingMeasure.p_thread == NULL || IS_ERR(g_loadingMeasure.p_thread)){
		RTICE_DEBUG("[%s:%d]Failed start loading_measure_thread.\r\n", __func__,__LINE__);
		return -RTICE_LOADING_MEASURE_CREATE_THREAD;
	}
	return 0;
}

/**
 * Main Entity of loading thread.
 */
static int loading_measure_thread(void * data)
{
	CMcMeasure * pmc = (CMcMeasure*)data;

	if(pmc == NULL)
		return -RTICE_MC_MEASURE_INVALID_PARAM;
    pmc->m_fRunning = true;
	wake_up(&rtice_loading_wait_main);
	/*Call interface of loading measurement*/
    pmc->m_errCode = startLoadingMear(pmc->m_checkTime);
	/*finish and exit from thread*/
    pmc->m_fRunning = false;
	wake_up(&rtice_loading_wait_main);
	return 0;
}

/**
 * Returns total size of the buffer in loading measurement.
 * If the thread is not finish ,returns zero insead.
 */
unsigned int loadingTotalSize(void)
{
    if(g_loadingMeasure.m_fRunning || gScpuLoadingData == NULL)
        return 0;
    return gScpuLoadingDataLen;
}

/**
 * Dump pure data in latest loading measurement.Returns count of dumped data or error code
 */
static int loadingDataDump(unsigned int offset,unsigned int size,unsigned char*pOutBuf)
{
    unsigned int totalSize = loadingTotalSize();

    if(!pOutBuf || (offset + size) > totalSize)
        return -RTICE_MC_MEASURE_INVALID_PARAM;
    if(!gScpuLoadingData)
        return -RTICE_MC_MEASURE_INVALID_PARAM;
    memcpy(pOutBuf,gScpuLoadingData + offset,size);
    return size;
}
#endif
/*****************************CMD Handler/Entry of the Measurement**********************************/
/**
 * Entry of measurement commands
 */ 
int get_mc_measure(unsigned char*pCmdData,unsigned int cmdLen,unsigned char * pOutBuf,unsigned int outBufLen)
{
    int ret = 0;
    ENUM_MC_MEASURE_QUERY query = ENUM_MC_MEASURE_QUERY_SIZE;

    if(!pOutBuf || outBufLen < 2046)
        return -RTICE_MC_MEASURE_OUT_BUF_SIZE_SMALL;
    if(!pCmdData || cmdLen < 1)
        return -RTICE_MC_MEASURE_INVALID_PARAM;
    query = (ENUM_MC_MEASURE_QUERY)pCmdData[0];
    pCmdData++;
    cmdLen--;
    switch(query){
    case MC_MEASURE_BYTES_COUNT:
    {
        unsigned int cnt = mearTotalSize();

        LTB4(cnt,pOutBuf[0],pOutBuf[1],pOutBuf[2],pOutBuf[3]);
        ret = 4;
    }
    break;
    case MC_MEASURE_QUERY_BYTES:
    {
        unsigned int offset = 0,size = 0;

        offset = B4TL(pCmdData[0],pCmdData[1],pCmdData[2],pCmdData[3]);
        size = B4TL(pCmdData[4],pCmdData[5],pCmdData[6],pCmdData[7]);
        if(size > outBufLen){
            ret = -RTICE_MC_MEASURE_INVALID_PARAM;
            break;
        }
        if(g_McMeasure.m_fRunning){
            ret = -RTICE_MC_MEASURE_QOS_MEAR_ONGOING;
            break;
        }
        if(g_McMeasure.m_errCode < 0){
            ret = -RTICE_MC_MEASURE_QOS_MEAR_ERROR;
            break;
        }
        ret = mearDataDump(offset,size,pOutBuf);
    }
    break;
    case MC_MEASURE_QUERY_BUS_BW:
    {
        unsigned char busId = 0;
        ret = 0;
        for(busId = 0;busId < RTICE_DBUS_BUS_ID_NUM;busId++){
            int tret = busStatisticCount(busId,pOutBuf + ret,(int)outBufLen - ret);
            if(tret < 0){
                ret = tret;
                break;
            }
            ret += tret;
        }
    }
    break;
    case MC_MEASURE_QUERY_CLIENT_BW:
    {
        unsigned char busId = 0,clientId = 0;
        if(cmdLen < 2){
            ret = -RTICE_MC_MEASURE_INVALID_PARAM;
            break;
        }
        busId = pCmdData[0];
        clientId = pCmdData[1];
        ret = clientStatisticCount(busId,clientId,pOutBuf,(int)outBufLen);
    }
    break;
    case MC_MEASURE_QUERY_BUS_CLIENTS_BW:
    {
        unsigned char busId = 0,clientId = 0;
        if(cmdLen < 1){
            ret = -RTICE_MC_MEASURE_INVALID_PARAM;
            break;
        }
        busId = pCmdData[0];
        ret = 0;
        for(clientId = 0;clientId < RTICE_DBUS_CLIENT_ID_NUM;clientId++){
            int tret = clientStatisticCount(busId,clientId,pOutBuf + ret,(int)outBufLen - ret);
            if(tret < 0){
                ret = tret;
                break;
            }
            ret += tret;
        }
    }
    break;
    case MC_MEASURE_QUERY_CLIENT_COUNT:
    {
        unsigned char busId = 0,clientId = 0,type = 0/*current*/;
        if(cmdLen < 3){
            ret = -RTICE_MC_MEASURE_INVALID_PARAM;
            break;
        }
        busId = pCmdData[0];
        clientId = pCmdData[1];
        type = pCmdData[2];
        ret = clientCount(busId,clientId,type,pOutBuf,(int)outBufLen);
    }
    break;
#ifdef CONFIG_RTICE_CPU_LOADING
    case LOADING_MEASURE_BYTES_COUNT:
    {
        unsigned int cnt = loadingTotalSize();

        LTB4(cnt,pOutBuf[0],pOutBuf[1],pOutBuf[2],pOutBuf[3]);
        ret = 4;
    }
    break;
    case LOADING_MEASURE_QUERY_BYTES:
    {
        unsigned int offset = 0,size = 0;

        offset = B4TL(pCmdData[0],pCmdData[1],pCmdData[2],pCmdData[3]);
        size = B4TL(pCmdData[4],pCmdData[5],pCmdData[6],pCmdData[7]);
        if(size > outBufLen){
            ret = -RTICE_MC_MEASURE_INVALID_PARAM;
            break;
        }
        if(g_loadingMeasure.m_fRunning){
            ret = -RTICE_LOADING_MEASURE_MEAR_ONGOING;
            break;
        }
        if(g_loadingMeasure.m_errCode < 0){
            ret = -RTICE_LOADING_MEASURE_MEAR_ERROR;
            break;
        }
        ret = loadingDataDump(offset,size,pOutBuf);
    }
    break;
#endif
    default:
        ret = -RTICE_MC_MEASURE_INVALID_PARAM;
    }
    return ret;
}

/*------------------------------------------------------------------
 * Func : rtice_mc_command_probe
 *
 * Desc : proc mc command
 *
 * Parm : opcode
 *
 * Retn :
 *------------------------------------------------------------------*/
int rtice_mc_command_probe(unsigned char op_code)
{
    return 1;//(GET_OP_CODE_COMMAND(op_code) <= 3) ? 1 : 0;
}
/*------------------------------------------------------------------
 * Func : rtice_mc_command_handler
 *
 * Desc : proc mc command
 *
 * Parm : cmd_data : command data
 *        cmd_len  : command data len
 *        rdata  : response data
 *
 * Retn :  < 0    : error
 *         others : length of response data
 *------------------------------------------------------------------*/
int rtice_mc_command_handler(
                  unsigned char opcode, unsigned char *cmd_data,
                  unsigned int cmd_len,
                  unsigned char *response_data,
                  unsigned int response_buff_size )
{
    int ret = 0;

    RTICE_CMD_DEBUG(cmd_data, cmd_len, "command_data=");
    switch(opcode){
    case RTICE_IOMEM_CMD_ENTER_MC_MEASURE:
        RTICE_DEBUG("[%s:%d]Enter mc measure.\r\n", __func__,__LINE__);
        if(cmd_len != 5)
            return -RTICE_MC_MEASURE_CMD_LEN_ERROR;
        if(cmd_data[0] == 0x85)    
            ret = enter_mc_measure(cmd_data,cmd_len);
    #ifdef CONFIG_RTICE_CPU_LOADING
        else if(cmd_data[0] == 0x86)
            ret = enter_loading_measure(cmd_data,cmd_len);
    #endif
        else
            return -RTICE_MC_MEASURE_INVALID_PARAM; 
    break;
    case RTICE_IOMEM_CMD_GET_MC_MEASURE:
        RTICE_DEBUG("[%s:%d]get_mc_measure.\r\n", __func__,__LINE__);
        if((ENUM_MC_MEASURE_QUERY)cmd_data[0] == MC_MEASURE_LOCK_QOS_MODE)
            ret = setLockQosMode(1);
        else if((ENUM_MC_MEASURE_QUERY)cmd_data[0] == MC_MEASURE_UNLOCK_QOS_MODE)
            ret = setLockQosMode(0);
        else if((ENUM_MC_MEASURE_QUERY)cmd_data[0] == MC_MEASURE_QUERY_QOS_MODE)
            ret = currentQosMode(response_data,response_buff_size);
        else
            ret = get_mc_measure(cmd_data,cmd_len,response_data,response_buff_size);
        break;
    case RTICE_IOMEM_CMD_EXIT_MC_MEASURE:
        RTICE_DEBUG("[%s:%d]exit_mc_masure.\r\n", __func__,__LINE__);
        ret = exit_mc_masure();
    #ifdef CONFIG_RTICE_CPU_LOADING
        if(ret == 0)
            ret = exit_loading_masure();
    #endif
    break;
    default:
        break;
    }
    return ret;
}

rtice_command_handler_t mc_cmd_handler = \
{
    RTICE_CMD_GROUP_ID_DMA_STATUS,
    rtice_mc_command_probe,
    rtice_mc_command_handler
};

/*------------------------------------------------------------------
 * Func : rtice_mc_tool_init
 *
 * Desc : register rtice mc mear tool
 *
 * Parm : N/A
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/

int rtice_mc_tool_init(void)
{
    rtice_register_command_handler(&mc_cmd_handler,\
		sizeof(mc_cmd_handler) / sizeof(rtice_command_handler_t));
    rtd_pr_rtice_info("rtice_register_command_handler() has been executed for MC_MEAR.\n");
	return 0;
}

/*------------------------------------------------------------------
 * Func : rtice_mc_tool_exit
 *
 * Desc : exit rtice mc mear tool
 *
 * Parm : N/A
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
void rtice_mc_tool_exit(void)
{
    rtice_unregister_command_handler(&mc_cmd_handler,\
		sizeof(mc_cmd_handler) / sizeof(rtice_command_handler_t));
    rtd_pr_rtice_info("rtice_unregister_command_handler() has been executed for MC_MEAR.\n");
}

