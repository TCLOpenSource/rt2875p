#include "rtk_ice_mc_measure.h"
#include <linux/kthread.h>
#include <linux/slab.h> //kmalloc
#include <linux/delay.h>
#include <rtk_kdriver/io.h>
#include <rtd_log/rtd_module_log.h>

// MC1 Measure
#include <rbus/dc1_mc_reg.h>
//#define Enable_DC2_SUPPORT
#ifdef CONFIG_RTK_KDRV_QOS_CONTROL
#include <rtk_kdriver/rtk_qos_export.h>
extern unsigned int total_measure_frames;
const int copyBufCntPerTime = 2000;
#define MC_1 0
#ifdef Enable_DC2_SUPPORT
#define MC_TOTAL_NUM 2
#define MC_2 1
#else
#define MC_TOTAL_NUM 1
#endif
extern unsigned char *qos_bw_measure_p[MC_TOTAL_NUM];
#endif
#define MC_RECORD_NUMBER 8
#define MC_MEASURE_PATCH_NUMERATOR 3
#define MC_MEASURE_PATCH_DENOMINATOR 4
static DECLARE_WAIT_QUEUE_HEAD (rtice_mc_wait_main);

static CMcMeasure g_McMeasure = {NULL,NULL,NULL,
    false,//f_stop_thread
    false,//is_running
    0, //error_code
    MC_MEASURE_QUERY_CURRENT,
    false,//has extend command
    0x0,//mc measure threshold,
    false,//not continue mode
    0,//measure time(s) in continue mode
#ifdef CONFIG_RTK_KDRV_QOS_CONTROL
    0,0,0,0,//the index is zero
#endif
};
//static unsigned int gMeasureCount=0;
//#define MC_READ_DATA(x,y) (((y)==MC_MEASURE_QUERY_CURRENT)?(x)->mc1_read:(x)->max_mc1_read)
static unsigned int mc_address(unsigned int addr,ENUM_MC_CATEGORY mc)
{
	if (mc == MC_CATEGORY_MC1){
		addr = addr & 0xffff0fff;
		addr = addr | 0x00008000;
	}
	return addr;
}

static int read_client(CMcClient* pmc,ENUM_MC_CATEGORY mc)
{
	unsigned int value=0x0,timeout_cnt=0,client_id=0;
	unsigned int read_cnt=0,write_cnt=0,tmp_cnt=0;
	unsigned int i=0,check_bit_times=0;
	int nret = 0;
	
	if(pmc == NULL)
		return -RTICE_MC_MEASURE_INVALID_PARAM;
	if(mc == MC_CATEGORY_ALL_MC){
		nret |= read_client(pmc, MC_CATEGORY_MC1);
		return nret;
	}
	RTICE_DEBUG("Bus_id=%d,client_Id=%d mc=%d-->\r\n",pmc->bus_id,pmc->client_id,(int)mc);
	value = 1;
	// enable mc measure
	rtd_outl(mc_address(DC1_MC_CLIENT_MEAS_0_reg, mc),value);
	while(1){
		if(g_McMeasure.f_stop_thread)
			return 0;
		if(rtd_inl(mc_address(DC1_MC_CLIENT_MEAS_0_reg, mc)) ==0){// wait record done
			msleep(10);
			break;
		}
		timeout_cnt++;
		if(timeout_cnt > 0x80000000){
			RTICE_DEBUG("[%s:%d]read bus[%d], client[%d] time out.\r\n", __func__,__LINE__,pmc->bus_id,pmc->client_id);
			return -RTICE_MC_MEASURE_TIME_OUT;
		}
		msleep(10);
	}
	for(i=0;i<8;i++){       
		//write record number and client id
		value = 0;
		//set record num
		value =  DC1_MC_CLIENT_MEAS_1_meas_sram_num(i);
		//set client id
		client_id = ((pmc->bus_id& 0xf) << 4) | pmc->client_id;
		value = value | DC1_MC_CLIENT_MEAS_1_meas_sram_address(client_id);
		rtd_outl(mc_address(DC1_MC_CLIENT_MEAS_1_reg,mc),value);
		/*check done bit*/
		check_bit_times=0;
		while((rtd_inl(mc_address(DC1_MC_CLIENT_MEAS_1_reg,mc)) & DC1_MC_CLIENT_MEAS_1_meas_id_update_done_mask) != DC1_MC_CLIENT_MEAS_1_meas_id_update_done_mask) {// wait for done bit
			check_bit_times++;
			if(check_bit_times >= 20)
				break;
			msleep(5);
		}
		if(check_bit_times >= 20){            
			RTICE_DEBUG("\tcheck bit done failed\n");
			continue;
		}        
		RTICE_DEBUG("\trecord_num=0x%x,client_id=0x%x check_bit_times=%d:\r\n",i,client_id,check_bit_times);
		// read client's read count
		tmp_cnt = rtd_inl(mc_address(DC1_MC_CLIENT_MEAS_2_reg,mc));
		RTICE_DEBUG("\tread_cnt=0x%x",tmp_cnt);
		if(tmp_cnt > read_cnt)
			read_cnt = tmp_cnt;
		//read client's write count
		tmp_cnt = rtd_inl(mc_address(DC1_MC_CLIENT_MEAS_3_reg,mc));
		RTICE_DEBUG("\twrite_cnt=0x%x\r\n",tmp_cnt);
		if(tmp_cnt > write_cnt)
			write_cnt = tmp_cnt;
	}   
	RTICE_DEBUG("read=%u, write =%u.\r\n",read_cnt,write_cnt);
	if(mc == MC_CATEGORY_MC1){
		pmc->measure_number[0] = read_cnt;
		pmc->measure_number[1] = write_cnt;
		for(i=0;i<2;i++){
			if(pmc->measure_number[i] > pmc->max_measure_number[i]){
				pmc->max_measure_number[i] = pmc->measure_number[i]; 
				pmc->max_measure_count[i] = pmc->measure_count+1;
			}
		}
	}
	return 0;
}
static int mc_measure_thread(void * data)
{
	CMcClient * pclient = NULL;
	int i=0;
#ifdef CONFIG_RTK_KDRV_QOS_CONTROL
	char buf[64];
#endif
	CMcMeasure * pmc = (CMcMeasure*)data;

	if(pmc == NULL)
		return -RTICE_MC_MEASURE_INVALID_PARAM;
	RTICE_DEBUG("[%s:%d]start mc_measure_thread\r\n", __func__,__LINE__);
	pmc->f_is_running = true;
	wake_up(&rtice_mc_wait_main);
	/*if not continue mode*/
	while(!pmc->isContinueMode){ 
		pclient = pmc->_phead;
		while(pclient != NULL){
			pmc->error_code = read_client(pclient,pclient->mc_category);
			if(pmc->error_code){
				break;
			}else{
				pclient->measure_count++;
				for(i=0;i<MC_MEASURE_ITEM_COUNT;i++){
				    pclient->prev_measure_number[i] = pclient->measure_number[i];
				}
			}
			RTICE_DEBUG("[%s:%d]bus_%d_client_%d_count %d \r\n", __func__,__LINE__,\
			pclient->bus_id,pclient->client_id,pclient->measure_count);
			pclient = pclient->_pnext;
			if(pmc->f_stop_thread || kthread_should_stop())
				break;
		}
		if(pmc->f_stop_thread || kthread_should_stop())
			break;
		if(pmc->error_code)
			break;
		if(pmc->f_stop_thread || kthread_should_stop())
			break;
		msleep(1);
	}
	if(pmc->isContinueMode){
	#ifdef CONFIG_RTK_KDRV_QOS_CONTROL
		RTICE_DEBUG("[%s:%d]Now is continue mode.\r\n", __func__,__LINE__);
		memset(buf,0,64);
		sprintf(buf,"bw_start@%d",pmc->checkTimeInContinue);
		pmc->error_code = rtk_qos_mode_parsing(buf);
		pmc->curMcIndex=0;
		pmc->curFrameIndexOfPass= 0;
		pmc->curBusIndexOfPass= 0;
		pmc->curClientIndexOfPass= 0;
	#endif
	}
	pmc->f_is_running = false;    
	wake_up(&rtice_mc_wait_main);
	RTICE_DEBUG("[%s:%d]mc_measure_thread stoped\r\n", __func__,__LINE__);
	return 0;
}
int init_client_list(unsigned char * cmd_data,unsigned long cmd_len)
{    
	CMcClient *p = NULL, *dummy = NULL;
	unsigned long i = 0,j=0;
	unsigned char busId = 0;
	unsigned int tmpVal = 0;
	
	RTICE_DEBUG("[%s:%d]init_client_list.\r\n", __func__,__LINE__);
	clear_client_list();
	g_McMeasure.isContinueMode = false;
	p = g_McMeasure._phead;
	g_McMeasure.hasExtendCommand = false;
	g_McMeasure.mc_threshold = 0x0;
	while(i<cmd_len){
		busId = cmd_data[i];
		if(busId & 0x80){//it is extend command count
			busId = busId & 0x7f;
			//get mc measure threshold
			i++;
			if(i+4 > cmd_len)
				return -RTICE_MC_MEASURE_CMD_LEN_ERROR;
			for(j=0;j<4;j++){
				tmpVal |= (cmd_data[i+j]<<(8*(3-j)));
			}
			if(busId == 4){
				g_McMeasure.mc_threshold = tmpVal;
			}else if(busId == 5){
				g_McMeasure.isContinueMode = true;
				g_McMeasure.checkTimeInContinue = tmpVal;
			}
			i+=4;
		}else{
			// create new CMcClient and init 
			if(p == NULL)
				p = (CMcClient*)kmalloc(sizeof(CMcClient),GFP_ATOMIC);
			p->bus_id = cmd_data[i];//bus id
			i++;
			p->client_id = cmd_data[i];//client id
			i++;
			p->mc_category = (ENUM_MC_CATEGORY)cmd_data[i];
			i++;
			memset(&*p->measure_number,0,MC_MEASURE_ITEM_COUNT*sizeof(unsigned int));
			memset(&*p->prev_measure_number,0,MC_MEASURE_ITEM_COUNT*sizeof(unsigned int));
			memset(&*p->max_measure_number,0,MC_MEASURE_ITEM_COUNT*sizeof(unsigned int));
			memset(&*p->max_measure_count,0,MC_MEASURE_ITEM_COUNT*sizeof(unsigned int));
			p->measure_count = 0;
			p->_pprev = dummy;
			p->_pnext = NULL;
			// set head
			if(g_McMeasure._phead == NULL)
				g_McMeasure._phead = p;
			// set prev CMcClient's _pnext is current
			if(dummy)
				dummy->_pnext = p;
			dummy = p;
			p = p->_pnext;
		}
	}
	return 0;
}

int clear_client_list(void)
{
	CMcClient *p = g_McMeasure._phead;
	CMcClient * ptmp = NULL;
	while(p != NULL){
		ptmp = p->_pnext;
		kfree(p);
		p = ptmp;
	}
	g_McMeasure._phead = NULL;
	return 0;
}

int enter_mc_measure(void)
{
	int nres = 0;
	if(!g_McMeasure.isContinueMode)
		exit_mc_masure();
	g_McMeasure.p_thread = kthread_run(mc_measure_thread,&g_McMeasure,"mc_measure_thread");
	nres = wait_event_timeout(rtice_mc_wait_main,(g_McMeasure.f_is_running == true),100);
	if(!nres){
		RTICE_DEBUG("[%s:%d]Failed start mc_measure_thread.\r\n", __func__,__LINE__);
		return -RTICE_MC_MEASURE_CREATE_THREAD;
	}
	return 0;
}

int exit_mc_masure(void)
{
	int nres = 0;

	if(g_McMeasure.f_is_running){
		g_McMeasure.f_stop_thread = true;
		nres = wait_event_timeout(rtice_mc_wait_main,(g_McMeasure.f_is_running==false),100);
		if(!nres){
			RTICE_DEBUG("[%s:%d]Failed stop mc_measure_thread.\r\n", __func__,__LINE__);
			return -RTICE_MC_MEASURE_CREATE_THREAD;
		}
	}
	g_McMeasure.p_thread = NULL;
	g_McMeasure.f_stop_thread = false;
	return 0;
}

/**
 * Returns max and average bw count of the bus and its clients.
 */
int busClientsBw(int bus,unsigned char*pBuf)
{
#ifdef CONFIG_RTK_KDRV_QOS_CONTROL
	int mc = 0,frame = 0,client = 0;
	int ret = 0;

	if(bus > DBUS_BUS_ID_NUM)
		return 0;
	for(client = 0;client < DBUS_CLIENT_ID_NUM;client++){
		unsigned int maxCntTotal = 0,avgCntTotal = 0;
		
		for(mc = 0;mc < MC_TOTAL_NUM;mc++){
			unsigned int maxCnt = 0,avgCnt = 0;
			
			for(frame = 0;frame < total_measure_frames;frame++){
				SRAM_RECORD_t*pRec = (SRAM_RECORD_t*)(qos_bw_measure_p[mc] + frame * sizeof(SRAM_RECORD_t));
				unsigned int tCnt = pRec->r_count[bus][client] + pRec->w_count[bus][client];
				if(tCnt > maxCnt)
					maxCnt = tCnt;
				avgCnt += tCnt;
			}
			LTB4(maxCnt,pBuf[0],pBuf[1],pBuf[2],pBuf[3]);
			pBuf += 4;
			ret += 4;
			avgCnt = (total_measure_frames == 0) ? 0 : (avgCnt / total_measure_frames);
			LTB4(avgCnt,pBuf[0],pBuf[1],pBuf[2],pBuf[3]);
			pBuf += 4;
			ret += 4;	
		}
		/*total*/
		for(frame = 0;frame < total_measure_frames;frame++){
			unsigned int tCnt = 0;
			for(mc = 0;mc < MC_TOTAL_NUM;mc++){
				SRAM_RECORD_t*pRec = (SRAM_RECORD_t*)(qos_bw_measure_p[mc] + frame * sizeof(SRAM_RECORD_t));
				tCnt += pRec->r_count[bus][client] + pRec->w_count[bus][client];
			}
			if(tCnt > maxCntTotal)
				maxCntTotal = tCnt;
			avgCntTotal += tCnt;
		}
		LTB4(maxCntTotal,pBuf[0],pBuf[1],pBuf[2],pBuf[3]);
		pBuf += 4;
		ret += 4;	
		avgCntTotal = (total_measure_frames == 0) ? 0 : (avgCntTotal / total_measure_frames);
		LTB4(avgCntTotal,pBuf[0],pBuf[1],pBuf[2],pBuf[3]);
		pBuf += 4;
		ret += 4;	
	}
	/*finish*/
	return ret;
#else
	return 0;
#endif
}

int get_mc_measure(unsigned char * out_buf,unsigned int buf_size,ENUM_MC_MEASURE_QUERY query)
{
	CMcClient * p = NULL;
	int i = 0,j = 0;
	unsigned int t32 = 0;
	unsigned char* presData = out_buf;
#ifdef CONFIG_RTK_KDRV_QOS_CONTROL
	unsigned int mcId = 0,frameId = 0,busId = 0,clId = 0,curCopyBusCnt = 0;
	SRAM_RECORD_t * sram_record_p = NULL;
#endif
	if(!g_McMeasure.isContinueMode){
		if(out_buf == NULL || g_McMeasure._phead == NULL)
			return -RTICE_MC_MEASURE_INVALID_PARAM;
		p = g_McMeasure._phead;
		while(p != NULL){
			if(query == MC_MEASURE_QUERY_CURRENT){
				t32 = p->measure_count;
				LTB4(t32 , presData[0], presData[1], presData[2],presData[3]); 
				presData += 4;
				i+=4;    
			}
			if(p->mc_category == MC_CATEGORY_MC1 || p->mc_category == MC_CATEGORY_ALL_MC){
				for(j = 0;j < 2;j++){
					if(query == MC_MEASURE_QUERY_MAX){//output count
						t32 = p->max_measure_count[j];
						LTB4(t32 , presData[0], presData[1], presData[2],presData[3]); 
						presData += 4;
						i+=4;
					}  
					t32 = (query == MC_MEASURE_QUERY_CURRENT) ? p->prev_measure_number[j]: ((query == MC_MEASURE_QUERY_MAX) ? p->max_measure_number[j] : 0);
					LTB4(t32 , presData[0], presData[1], presData[2],presData[3]); 
					presData += 4;
					i+=4;
				}
			}
			RTICE_DEBUG("[GET_MC]bus=%d,client=%d,read=%d,write=%d.\r\n",p->bus_id,p->client_id,p->measure_number[0],p->measure_number[1]);
			p = p->_pnext;
		}
	}else{
	#ifdef CONFIG_RTK_KDRV_QOS_CONTROL
		//continue mode    
		if(query == MC_MEASURE_BYTES_COUNT){
			t32 = g_McMeasure.f_is_running ? 0 : (total_measure_frames * sizeof(SRAM_RECORD_t) * MC_TOTAL_NUM);
			RTICE_DEBUG("[%s:%d]Get data count(t32=%u,f_is_running=%d,total_measure_frames=%d\r\n",__func__,__LINE__,t32,g_McMeasure.f_is_running,total_measure_frames);
			LTB4(t32 , presData[0], presData[1], presData[2],presData[3]);
			i+=4;  
			/*reset MC_MEASURE_QUERY_CONTINUE_MODE*/
			g_McMeasure.curMcIndex = 0;
			g_McMeasure.curFrameIndexOfPass=0;
			g_McMeasure.curBusIndexOfPass=0;
			g_McMeasure.curClientIndexOfPass=0;
		}else if(query == MC_MEASURE_QUERY_CONTINUE_MODE){
			if(g_McMeasure.error_code != 0x80){
				i = -RTICE_MC_MEASURE_CONTINUE_MODE_ERROR;
				RTICE_DEBUG("measure error. error code=0x%x\r\n",g_McMeasure.error_code);
			}else{
				//each pass 15 bus data.The bytes cnt is 15*16*8=1920
				#define MAX_COPY_BUS_NUM 15
				while(g_McMeasure.curMcIndex < MC_TOTAL_NUM && curCopyBusCnt < MAX_COPY_BUS_NUM){
					mcId = g_McMeasure.curMcIndex;
					RTICE_DEBUG("[%s:%d]mcId=%u\r\n",__func__,__LINE__,mcId);
					/*if all frames data are copied, mcIndex inc*/
					if(g_McMeasure.curFrameIndexOfPass >= total_measure_frames){
						g_McMeasure.curMcIndex++;
						g_McMeasure.curFrameIndexOfPass=0;
						g_McMeasure.curBusIndexOfPass=0;
						g_McMeasure.curClientIndexOfPass=0;
						continue;
					}
					while(g_McMeasure.curFrameIndexOfPass < total_measure_frames && curCopyBusCnt < MAX_COPY_BUS_NUM){
						frameId = g_McMeasure.curFrameIndexOfPass;
						RTICE_DEBUG("[%s:%d]frameId=%u\r\n",__func__,__LINE__,frameId);
						sram_record_p = (SRAM_RECORD_t *)(qos_bw_measure_p[mcId] + frameId * sizeof(SRAM_RECORD_t));
						if(g_McMeasure.curBusIndexOfPass >= DBUS_BUS_ID_NUM){
							g_McMeasure.curFrameIndexOfPass++;
							g_McMeasure.curBusIndexOfPass=0;
							g_McMeasure.curClientIndexOfPass=0;
							continue;
						}
						while(g_McMeasure.curBusIndexOfPass < DBUS_BUS_ID_NUM && curCopyBusCnt < MAX_COPY_BUS_NUM){
							busId = g_McMeasure.curBusIndexOfPass;
							RTICE_DEBUG("[%s:%d]busId=%u\r\n",__func__,__LINE__,busId);
							while(g_McMeasure.curClientIndexOfPass < DBUS_CLIENT_ID_NUM){
								clId = g_McMeasure.curClientIndexOfPass;
								/*copy read count*/
								LTB4(sram_record_p->r_count[busId][clId],presData[0], presData[1], presData[2],presData[3]);
								presData += 4;
								i += 4;
								/*copy w count*/
								LTB4(sram_record_p->w_count[busId][clId],presData[0], presData[1], presData[2],presData[3]);
								presData += 4;
								i += 4;
								g_McMeasure.curClientIndexOfPass++;
								RTICE_DEBUG("MC_%u,F_%u,Bus_%u,client_%u,%u,%u\r\n",mcId,frameId,busId,clId,\
								            sram_record_p->r_count[busId][clId],
								            sram_record_p->w_count[busId][clId]);
							}
							g_McMeasure.curBusIndexOfPass++;
							g_McMeasure.curClientIndexOfPass=0;
							curCopyBusCnt++;
						}
					}
				}
			}    
		}else if(query == MC_MEASURE_QUERY_BUS_BW){
			int bus = g_McMeasure.curBusIndexOfPass;
			/*Returns average and max count belong to one bus*/
			i = 0;/*count of return's data*/
			/*bus*/
			if(bus < DBUS_BUS_ID_NUM){
				int mc = 0,frame = 0;
				unsigned int maxCntTotal = 0,avgCntTotal = 0;
				for(mc = 0;mc < MC_TOTAL_NUM;mc++){
					unsigned int maxCnt = 0,avgCnt = 0;
					/*frame*/
					for(frame = 0;frame < total_measure_frames;frame++){
						int client = 0;
						unsigned int tCnt = 0;
						/*clients*/
						for(client = 0;client < DBUS_CLIENT_ID_NUM;client++){
							SRAM_RECORD_t*pRec = (SRAM_RECORD_t*)(qos_bw_measure_p[mc] + frame * sizeof(SRAM_RECORD_t));
							tCnt += pRec->r_count[bus][client] + pRec->w_count[bus][client];
						}	
						if(maxCnt < tCnt)
							maxCnt = tCnt;
						avgCnt += tCnt;
					}	
					if(total_measure_frames)
						avgCnt /= total_measure_frames;
					/*copy max and avg to output buffer*/
					LTB4(maxCnt,presData[0],presData[1],presData[2],presData[3]);
					presData += 4;
					i += 4;
					LTB4(avgCnt,presData[0],presData[1],presData[2],presData[3]);
					presData += 4;
					i += 4;	
				}
				/*total max and avg*/
				for(frame = 0;frame < total_measure_frames;frame++){
					unsigned int tCnt = 0;
					for(mc = 0;mc < MC_TOTAL_NUM;mc++){
						int client = 0;
						/*clients*/
						for(client = 0;client < DBUS_CLIENT_ID_NUM;client++){
							SRAM_RECORD_t*pRec = (SRAM_RECORD_t*)(qos_bw_measure_p[mc] + frame * sizeof(SRAM_RECORD_t));
							tCnt += pRec->r_count[bus][client] + pRec->w_count[bus][client];
						}
					}
					if(maxCntTotal < tCnt)
						maxCntTotal = tCnt;
					avgCntTotal += tCnt;
				}
				if(total_measure_frames)
					avgCntTotal /= total_measure_frames;
				LTB4(maxCntTotal,presData[0],presData[1],presData[2],presData[3]);
				presData += 4;
				i += 4;
				LTB4(avgCntTotal,presData[0],presData[1],presData[2],presData[3]);
				presData += 4;
				i += 4;	
			}
			/*next bus*/
			g_McMeasure.curBusIndexOfPass++;
		}else if(query == MC_MEASURE_QUERY_CLIENT_BW){
			i = busClientsBw(g_McMeasure.curBusIndexOfPass++,presData);	
		}
	#endif
	}
	return i;
}
/*!
 * lock or unlock qos mode.Returns negative while there is something wrong
 */
int setLockQosMode(int fLock)
{
#ifdef CONFIG_RTK_KDRV_QOS_CONTROL
	if(fLock)
		rtk_qos_lock_current_mode();
	else
		rtk_qos_unlock_current_mode();
	return 0;
#else
	return -1;
#endif
}
/*!
 * get description about current qos mode.Returns length of the string while everything is ok ,otherwise returns negative
 */
int currentQosMode(unsigned char*pOutBuf)
{

#ifdef CONFIG_RTK_KDRV_QOS_CONTROL
	char * const pModeStr = rtk_qos_get_current_mode();
	memcpy(pOutBuf,pModeStr,strlen(pModeStr));
	return strlen(pModeStr);
#else
	return -1;
#endif
}

/*------------------------------------------------------------------
 * Func : rtice_mc_command_probe
 *
 * Desc : proc mc mear command
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
 * Desc : proc mc mear command
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
    //parse cmd
    int ret = 0;
    switch(opcode){
    case RTICE_IOMEM_CMD_ENTER_MC_MEASURE:
        RTICE_DEBUG("[%s:%d]Enter mc measure.\r\n", __func__,__LINE__);
        ret = init_client_list(cmd_data, cmd_len);
        if(ret == 0){
            RTICE_DEBUG("[%s:%d]Enter mc measure.\r\n", __func__,__LINE__);
        	ret = enter_mc_measure();
        }
    break;
    case RTICE_IOMEM_CMD_GET_MC_MEASURE:
        RTICE_DEBUG("[%s:%d]get_mc_measure.\r\n", __func__,__LINE__);
		if((ENUM_MC_MEASURE_QUERY)cmd_data[0] == MC_MEASURE_LOCK_QOS_MODE)
			ret = setLockQosMode(1);
		else if((ENUM_MC_MEASURE_QUERY)cmd_data[0] == MC_MEASURE_UNLOCK_QOS_MODE)
			ret = setLockQosMode(0);
		else if((ENUM_MC_MEASURE_QUERY)cmd_data[0] == MC_MEASURE_QUERY_QOS_MODE)
			ret = currentQosMode(response_data);
		else
			ret = get_mc_measure(response_data, response_buff_size,(ENUM_MC_MEASURE_QUERY)cmd_data[0]);
    break;
    case RTICE_IOMEM_CMD_EXIT_MC_MEASURE:
        RTICE_DEBUG("[%s:%d]exit_mc_masure.\r\n", __func__,__LINE__);
    	ret = exit_mc_masure();
        ret = clear_client_list();
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
