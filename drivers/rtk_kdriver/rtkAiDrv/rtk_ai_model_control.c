/************************************************************************
 *  Include files
 ************************************************************************/
//#define AI_VERIFY_ON_ZEBU		1

#include <linux/version.h>
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0))	
	#include <linux/time64.h>
#else
	#include <linux/time.h>
#endif	
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/pageremap.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>
#ifdef AI_VERIFY_ON_ZEBU
	#include <asm-generic/io.h>
#else
	#include <rtk_kdriver/io.h>
#endif
#include <asm/cacheflush.h>
#include "rtk_ai.h"
#include <tvscalercontrol/vip/ai_pq.h>
#include "rtk_ai_common.h"

/************************************************************************
 *  Definitions
 ************************************************************************/
#define AI_MC_TIMER_INTERVAL   HZ												//1 seconds
#define AI_MC_GRANT_TIMEOUT		 (4*AI_MC_TIMER_INTERVAL)	//4 seconds
#define AI_MC_AP_CRASH_TIMEOUT 		6											//6 seconds
#define AI_MC_NN_MAX_CLOCK_RATIO	32
#define AI_LOAD_TIMER_INTERVAL  	(HZ/5)										//200 ms
#define AI_TOTAL_CYCLE_REG		  0xb8073078
#define AI_IDLE_CYCLE_REG		  0xb807307c
#define AI_LOAD_HIST_LEN			5											//200ms x 5 = 1s

/************************************************************************
*  Public variables
************************************************************************/
AI_CONTROL *aiControl;
AI_CONTROL *aiControl_backup;
struct semaphore aiHwSem[AI_HW_MAX];
unsigned long	aiHwSemTimeout[AI_HW_MAX];
struct semaphore aiControlSem;
static int isAiControlInit=0;
static struct timer_list ai_mc_timer;
static struct timer_list ai_load_timer;
extern RTK_AI_PQ_mode aipq_mode;
static unsigned int isNpuLoadCheck=0,npuLoadIdx=0,npuLoadingAve=0;

/************************************************************************
*  Function body
************************************************************************/
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0))	
void mc_checkGrantTimer(struct timer_list *t)
#else
void mc_checkGrantTimer(unsigned long param)
#endif
{
	int i,foundLock=0;
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0))		
	struct timespec64 curTime;
#else
	struct timespec curTime;
#endif	
	if(isAiControlInit==0||aiControl==NULL){
		AI_ERROR("initial fail or memory trash. stop timer\n");
		return;
	}
	
	down(&aiControlSem);
	
	//check PPU
	if((aiControl->curProcessOfUsePPU!=-1)&&(aiHwSemTimeout[AI_HW_PPU]<jiffies)){
		AI_ERROR("PPU: (handle %d) not response and lock sem. unlock it\n",aiControl->curProcessOfUsePPU);
		aiControl->curProcessOfUsePPU=-1;
		foundLock=1;
		//release aiHwSem for ap crash case
		up(&aiHwSem[AI_HW_PPU]);
	}

	//check NNU
	if((aiControl->curProcessOfUseNNU!=-1)&&(aiHwSemTimeout[AI_HW_NNU]<jiffies)){
		AI_ERROR("NNU: (handle %d) not response and lock sem. unlock it\n",aiControl->curProcessOfUseNNU);
		aiControl->curProcessOfUseNNU=-1;
		foundLock=1;
		//release aiHwSem for ap crash case
		up(&aiHwSem[AI_HW_NNU]);
	}
	
	//check Output
	if((aiControl->curProcessOfUseOutput!=-1)&&(aiHwSemTimeout[AI_HW_OUTPUT]<jiffies)){
		AI_ERROR("Output: (handle %d) not response and lock sem. unlock it\n",aiControl->curProcessOfUseOutput);
		aiControl->curProcessOfUseOutput=-1;
		foundLock=1;
		//release aiHwSem for ap crash case
		up(&aiHwSem[AI_HW_OUTPUT]);
	}

	//only check when no locking happen,cause other process do not update lastTime when lock up
	if(foundLock==0){
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0))		
		ktime_get_coarse_real_ts64(&curTime);
#else
		curTime=current_kernel_time();
#endif		
		//check crash ap case without lock sem
		for(i=0;i<AI_MAX_PROCESS;i++){
			if(aiControl->aiProcessInfo[i].activated){
				if(((s64)curTime.tv_sec-aiControl->aiProcessInfo[i].lastTime.tv_sec)>=AI_MC_AP_CRASH_TIMEOUT){
					
					aiControl->aiProcessInfo[i].activated=0;
					AI_ERROR("found (handle %d) not response,disable it\n",i);
					if(aiControl->aiProcessNum>=1)
						aiControl->aiProcessNum--;
					else
						aiControl->aiProcessNum=0;					
				}
			}	
		}	
	}

	up(&aiControlSem);
	
  ai_mc_timer.expires = jiffies + AI_MC_TIMER_INTERVAL;
  add_timer(&ai_mc_timer);
}

void mc_dumpNpuLoad(void){
	unsigned long total=0,idle=0,result=0;
	static unsigned int sum=0;
	
	//check if NPU clock is enabled
	if((IoReg_Read32(SYS_REG_SYS_CLKEN0_reg)&SYS_REG_SYS_CLKEN0_clken_nnip_mask)==0){
		AI_ERROR("NPU clock is OFF\n");
		npuLoadingAve=0;
		return;	
	}
	
	//check counter and store
	total=IoReg_Read32(AI_TOTAL_CYCLE_REG);
	idle=IoReg_Read32(AI_IDLE_CYCLE_REG);
	if((total>0)&&(total>idle))
		result=(total-idle)*100/total;
	else
		result=0;
	
		
	//AI_INFO("npuLoading[%d] = %d (idle=%ld,total=%ld)\n",npuLoadIdx,result,idle,total);

	
	sum += (unsigned int)result;
	if(npuLoadIdx==(AI_LOAD_HIST_LEN-1)){
		npuLoadingAve = sum/AI_LOAD_HIST_LEN;
		AI_INFO("ave npuLoading = %d\n",npuLoadingAve);		
		sum =0;
	}

	//update index
	npuLoadIdx++;
	npuLoadIdx=npuLoadIdx%AI_LOAD_HIST_LEN;

	//reset counter
	IoReg_Write32(AI_TOTAL_CYCLE_REG,0);
	IoReg_Write32(AI_IDLE_CYCLE_REG,0);

	
	return;
}

void mc_checkNpuLoadTimer(struct timer_list *t){

	mc_dumpNpuLoad();

	if(isNpuLoadCheck){
		ai_load_timer.expires = jiffies + AI_LOAD_TIMER_INTERVAL;
		add_timer(&ai_load_timer);
	}
}

BOOL rtk_ai_mc_init(void){
	int i;
	unsigned long address;
	
	if(isAiControlInit==1)
	{
		AI_ERROR("already inited\n");
		return TRUE;
	}
	
	//1. init aiControl parameters
	aiControl_backup=(AI_CONTROL *)vmalloc(sizeof(AI_CONTROL));
	//aiControl=(AI_CONTROL *)dvr_malloc_uncached_specific(sizeof(AI_CONTROL),GFP_DCU1_LIMIT,(void **)&address);
	address=(unsigned long)dvr_malloc_uncached(sizeof(AI_CONTROL),(void **)&aiControl);
	AI_INFO("dvr_malloc aiControl=0x%lx,cached addr=0x%lx\n",(unsigned long)aiControl,address);
	
	if(aiControl==NULL){
		if(aiControl_backup)
			vfree(aiControl_backup);
		AI_ERROR("dvr_malloc_uncached %d Bytes fail\n",(unsigned int)sizeof(AI_CONTROL));
		return FALSE;
	}
	
	aiControl->aiProcessNum=0;
	for(i=0;i<AI_MAX_PROCESS;i++){
		aiControl->aiProcessInfo[i].activated=0;
	}

	aiControl->curProcessOfUsePPU=-1;
	aiControl->curProcessOfUseNNU=-1;
	aiControl->curProcessOfUseOutput=-1;
	
	isAiControlInit=1;

	//2.init semphare
	for(i=0;i<AI_HW_MAX;i++){
		sema_init(&aiHwSem[i],1);
	}
	
	sema_init(&aiControlSem,1);
	
	
	//3.init timer
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0))		
	timer_setup(&ai_mc_timer,mc_checkGrantTimer,0);
	ai_mc_timer.expires = jiffies + AI_MC_TIMER_INTERVAL;
	
	timer_setup(&ai_load_timer,mc_checkNpuLoadTimer,0);
#else
	init_timer(&ai_mc_timer);
	ai_mc_timer.function = &mc_checkGrantTimer;
	ai_mc_timer.data = ((unsigned long) 0);
	ai_mc_timer.expires = jiffies + AI_MC_TIMER_INTERVAL;
	//add_timer(&ai_mc_timer); 

#endif
	
	AI_INFO("rtk_ai_mc_init done\n");
	
	return TRUE;
}

BOOL rtk_ai_mc_unInit(void){
	int i;

	aiControl->aiProcessNum=0;
	for(i=0;i<AI_MAX_PROCESS;i++){
		aiControl->aiProcessInfo[i].activated=0;
	}

	aiControl->curProcessOfUsePPU=-1;
	aiControl->curProcessOfUseNNU=-1;
	aiControl->curProcessOfUseOutput=-1;
	
	vfree(aiControl_backup);
	dvr_free((void *)aiControl);
	aiControl=NULL;
	
	del_timer(&ai_mc_timer);
	
	isAiControlInit=0;

	AI_INFO("rtk_ai_mc_unInit done\n");
	
	return TRUE;
}


BOOL rtk_ai_mc_registModel(AI_PROCESS_INFO *info,int handle){
	int i;
	unsigned long addr_start,addr_end;
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0))		
	struct timespec64 curTime;
#else
	struct timespec curTime;
#endif
	
	if(aiControl->aiProcessNum<0 || aiControl->aiProcessNum>=(AI_MAX_PROCESS-1)){
		AI_ERROR("aiProcessNum is out of range(%d)\n",aiControl->aiProcessNum);
		return FALSE;
	}

	if( handle<0 || handle>=AI_MAX_PROCESS){	
		AI_ERROR("handle is out of range 0~%d (%d)\n",(AI_MAX_PROCESS-1),handle);
		return FALSE;
	}

	//check parameters of info
	if(info->modelNum<=0||info->inputVideoWidth<=0||info->inputVideoHeight<=0){
		AI_ERROR("error info\n");
		return FALSE;
	}
	
#if 1
	if(info->inputVideoWidth>480||info->inputVideoHeight>480){
		AI_ERROR("not support size( %d x %d )\n",info->inputVideoWidth,info->inputVideoHeight);
		return FALSE;
	}

#else
	if(!((info->inputVideoWidth==416&&info->inputVideoHeight==416)	\
	||(info->inputVideoWidth==224&&info->inputVideoHeight==224)		\
	||(info->inputVideoWidth==480&&info->inputVideoHeight==256)		\
	||(info->inputVideoWidth==320&&info->inputVideoHeight==240)		\
	||(info->inputVideoWidth==960&&info->inputVideoHeight==540))){
		AI_ERROR("not support size( %d x %d )\n",info->inputVideoWidth,info->inputVideoHeight);
		return FALSE;
	}
#endif	
	if(info->outputFunIdx<AI_MC_VPQ_FACE||info->outputFunIdx>=AI_MC_VPQ_MAX){
		AI_ERROR("not support vpq function %d\n",info->outputFunIdx);
		AI_ERROR("vpq function %d for face detect\n",AI_MC_VPQ_FACE);
		AI_ERROR("vpq function %d for scene detect\n",AI_MC_VPQ_SCENE);
		AI_ERROR("vpq function %d for resolution detect\n",AI_MC_VPQ_RESOLUTION);
		return FALSE;
	}
	
	
	if(isAiControlInit!=1){
		AI_ERROR("not inited\n");
		return FALSE;
	}

	down(&aiControlSem);
	
	//check handle. already registed when call rtk_ai_mc_getNextHandle
	if(aiControl->aiProcessInfo[handle].activated!=1){
		up(&aiControlSem);
		AI_ERROR("handle %d status is wrong(%d)\n",handle,aiControl->aiProcessInfo[handle].activated);
		return FALSE;
	}

	aiControl->aiProcessInfo[handle].modelNum=info->modelNum;
	memcpy((void *)&aiControl->aiProcessInfo[handle].modelFile,(void *)&info->modelFile,AI_MAX_MODEL_FILE*AI_MAX_MODEL_NAME_LEN);
	aiControl->aiProcessInfo[handle].inputVideoWidth=info->inputVideoWidth;
	aiControl->aiProcessInfo[handle].inputVideoHeight=info->inputVideoHeight;
	aiControl->aiProcessInfo[handle].inputAudioFmt=info->inputAudioFmt;
	aiControl->aiProcessInfo[handle].outputFunIdx=info->outputFunIdx;
	aiControl->aiProcessInfo[handle].targetFps=info->targetFps;
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0))		
	ktime_get_coarse_real_ts64(&curTime);
#else
	curTime=current_kernel_time();
#endif
	aiControl->aiProcessInfo[handle].lastTime.tv_sec=(s64)curTime.tv_sec;
	aiControl->aiProcessInfo[handle].lastTime.tv_nsec=(s64)curTime.tv_nsec;
	
	for(i=0;i<AI_HW_MAX;i++)
		aiHwSemTimeout[i]=jiffies+AI_MC_GRANT_TIMEOUT;

	//flush cache
	addr_start=(unsigned long)aiControl;
	addr_end=addr_start+sizeof(AI_CONTROL);
	//dmac_flush_range((void *)addr_start,(void *)addr_end);	

	up(&aiControlSem);

	AI_INFO("flush 0x%lx to 0x%lx\n",addr_start,addr_end);
	AI_INFO("rtk_ai_mc_registModel done, aiProcessNum=%d\n",aiControl->aiProcessNum);
	AI_INFO("rtk_ai_mc_registModel: UsePPU=%d,UseNNU=%d,UseOutput=%d\n",aiControl->curProcessOfUsePPU,aiControl->curProcessOfUseNNU,aiControl->curProcessOfUseOutput);

	return TRUE;
}

BOOL rtk_ai_mc_unRegistModel(int handle){

	if(aiControl->aiProcessNum<0 || aiControl->aiProcessNum>=AI_MAX_PROCESS){
		AI_ERROR("aiProcessNum is out of range(%d)\n",aiControl->aiProcessNum);
		return FALSE;
	}

	if( handle<0 || handle>=AI_MAX_PROCESS){	
		AI_ERROR("handle is out of range 0~%d (%d)\n",(AI_MAX_PROCESS-1),handle);
		return FALSE;
	}

	if(isAiControlInit!=1){
		AI_ERROR("not inited\n");
		return FALSE;
	}

	down(&aiControlSem);
	
	aiControl->aiProcessInfo[handle].activated=0;
	if(aiControl->aiProcessNum>=1)
		aiControl->aiProcessNum--;
	else
		aiControl->aiProcessNum=0;

	up(&aiControlSem);


	AI_INFO("rtk_ai_mc_unRegistModel done\n");
	
	return TRUE;
}	

BOOL rtk_ai_mc_getShareMem(unsigned int *phy_addr){
	
	if(isAiControlInit!=1){
		AI_ERROR("not inited\n");
		return FALSE;
	}

	if(phy_addr==NULL){
		AI_ERROR("phy_addr is null\n");
		return FALSE;
	}
	
        *phy_addr=(unsigned long)dvr_to_phys((void *)aiControl);
        AI_INFO("phy_addr =0x%lx,virt_addr=0x%lx\n",(unsigned long)*phy_addr,(unsigned long)aiControl);
        AI_INFO("aiControl->aiProcessNum=%d\n",aiControl->aiProcessNum);
	
	return TRUE;
}

BOOL rtk_ai_mc_getGrant(int handle,AI_HW_INDEX index){
	static int count=0;

	if( index<AI_HW_PPU || index>AI_HW_OUTPUT){	
		AI_ERROR("index is out of range 0~%d (%d)\n",AI_HW_OUTPUT,index);
		return FALSE;
	}

	if(isAiControlInit!=1){
		AI_ERROR("not inited\n");
		return FALSE;
	}

	//AI_DEBUG("[grant]handle=%d,index=%d before sem\n",handle,index);

	if(down_timeout(&aiHwSem[index],msecs_to_jiffies(500))){
		AI_ERROR("get grant timeout,handle=%d,index=%d,UsePPU=%d,UseNNU=%d,UseOutput=%d\n",handle,index,aiControl->curProcessOfUsePPU,aiControl->curProcessOfUseNNU,aiControl->curProcessOfUseOutput);
		return FALSE;
	}

	AI_DEBUG("[grant]handle=%d,index=%d get sem\n",handle,index);
	aiHwSemTimeout[index]=jiffies+AI_MC_GRANT_TIMEOUT;

	down(&aiControlSem);
#if 1
	if(index==AI_HW_PPU)
		aiControl->curProcessOfUsePPU=handle;
	if(index==AI_HW_NNU)
		aiControl->curProcessOfUseNNU=handle;
	if(index==AI_HW_OUTPUT)
		aiControl->curProcessOfUseOutput=handle; 
#else	
	switch(index){
		case AI_HW_PPU:
			aiControl->curProcessOfUsePPU=handle;
			break;
		case AI_HW_NNU:
			aiControl->curProcessOfUseNNU=handle;
			break;
		case AI_HW_OUTPUT:
			aiControl->curProcessOfUseOutput=handle;
			break;
		default:
			break;
	}	
#endif

	up(&aiControlSem);

	while(aipq_mode.clock_status==0){
		
		if(count>50){
			AI_ERROR("AI clock is disable.grant Need to wait\n");
			count=0;
		}else
			count++;
		msleep(200);
	}

	return TRUE;
}

BOOL rtk_ai_mc_releaseGrant(int handle,AI_HW_INDEX index){
	static int count=0;

	if( index<AI_HW_PPU || index>AI_HW_OUTPUT){	
		AI_ERROR("index is out of range 0~%d (%d)\n",AI_HW_OUTPUT,index);
		return FALSE;
	}

	if(isAiControlInit!=1){
		AI_ERROR("not inited\n");
		return FALSE;
	}
	
	while(aipq_mode.clock_status==0){
		if(count>50){
			AI_ERROR("AI clock is disable.release Need to wait\n");
			count=0;
		}else
			count++;
		msleep(200);
	}	
	
	down(&aiControlSem);

#if 1
	if(index==AI_HW_PPU)
	{
		if(handle!=aiControl->curProcessOfUsePPU){
			AI_ERROR("not correct handle %d(%d)\n",handle,aiControl->curProcessOfUsePPU);
			up(&aiControlSem);
			return FALSE;
		}

		aiControl->curProcessOfUsePPU=-1;
	}
	
	if(index==AI_HW_NNU)
	{
		if(handle!=aiControl->curProcessOfUseNNU){
			AI_ERROR("not correct handle %d(%d)\n",handle,aiControl->curProcessOfUseNNU);
			up(&aiControlSem);
			return FALSE;
		}

		aiControl->curProcessOfUseNNU=-1;
	}
	
	if(index==AI_HW_OUTPUT)
	{
		if(handle!=aiControl->curProcessOfUseOutput){
			AI_ERROR("not correct handle %d(%d)\n",handle,aiControl->curProcessOfUseOutput);
			up(&aiControlSem);
			return FALSE;
		}
		aiControl->curProcessOfUseOutput=-1;
	}

#else	
	switch(index){
		case AI_HW_PPU:
			if(handle!=aiControl->curProcessOfUsePPU){
				AI_ERROR("not correct handle %d(%d)\n",handle,aiControl->curProcessOfUsePPU);
				up(&aiControlSem);
				return FALSE;
			}
			break;
		case AI_HW_NNU:
			if(handle!=aiControl->curProcessOfUseNNU){
				AI_ERROR("not correct handle %d(%d)\n",handle,aiControl->curProcessOfUseNNU);
				up(&aiControlSem);
				return FALSE;
			}
			break;
		case AI_HW_OUTPUT:
			if(handle!=aiControl->curProcessOfUseOutput){
				AI_ERROR("not correct handle %d(%d)\n",handle,aiControl->curProcessOfUseOutput);
				up(&aiControlSem);
				return FALSE;
			}
			break;
		default:
			break;
	}	

	switch(index){
		case AI_HW_PPU:
			aiControl->curProcessOfUsePPU=-1;
			break;
		case AI_HW_NNU:
			aiControl->curProcessOfUseNNU=-1;
			break;
		case AI_HW_OUTPUT:
			aiControl->curProcessOfUseOutput=-1;
			break;
		default:
			break;
	}	
#endif

	up(&aiControlSem);
	up(&aiHwSem[index]);
	
	AI_DEBUG("[grant] release handle=%d,index=%d\n",handle,index);

	return TRUE;
}


BOOL rtk_ai_mc_getCurModVpqCtrl(AI_MC_VPQ_INDEX *index){
	
	if(isAiControlInit!=1){
		AI_ERROR("not inited\n");
		return FALSE;
	}

	if(index==NULL){
		AI_ERROR("index is NULL\n");
		return FALSE;
	}
	
	//not activate and do not print any message
	if(aiControl->curProcessOfUseOutput==-1)
		return FALSE;

	if(aiControl->curProcessOfUseOutput<0 || aiControl->curProcessOfUseOutput>=AI_MAX_PROCESS){
		//AI_ERROR("curProcessOfUseOutput is out of range(%d)\n",aiControl->curProcessOfUseOutput);
		return FALSE;
	}
	
	if(aiControl->aiProcessInfo[aiControl->curProcessOfUseOutput].activated!=1){
		AI_ERROR("aiProcessInfo[%d] is not activated\n",aiControl->curProcessOfUseOutput);
		return FALSE;
	}
	
	*index=aiControl->aiProcessInfo[aiControl->curProcessOfUseOutput].outputFunIdx;
	
	return TRUE;
}	

int rtk_ai_mc_getNextHandle(void){
	int i;
	
	if(isAiControlInit!=1){
		// first handle would be 0
		AI_INFO("rtk_ai_mc_getNextHandle: 0\n");
		return 0;
	}
	
	//check empty handle
	down(&aiControlSem);
	for(i=0;i<AI_MAX_PROCESS;i++){
		if(aiControl->aiProcessInfo[i].activated==0){
			aiControl->aiProcessNum=aiControl->aiProcessNum+1;
			aiControl->aiProcessInfo[i].activated=1;
			break;
		}
	}
	up(&aiControlSem);
	
	if(i>=AI_MAX_PROCESS){
		i=AI_MAX_PROCESS-1;
		AI_ERROR("too much AP to use NN\n");
	}
	
	AI_INFO("rtk_ai_mc_getNextHandle: %d\n",i);
	
	return i;
}

BOOL rtk_ai_mc_crashRelease(int handle){
	int i=0;
	
	//check handle
	if(handle<0||handle>=AI_MAX_PROCESS){
		AI_ERROR("handle is out of range %d(0~%d)\n",handle,AI_MAX_PROCESS);
		return FALSE;
	}	
	
	AI_INFO("rtk_ai_mc_release: UsePPU=%d,UseNNU=%d,UseOutput=%d\n",aiControl->curProcessOfUsePPU,aiControl->curProcessOfUseNNU,aiControl->curProcessOfUseOutput);
	
	//check if crahs ap havd locked some semaphre
	down(&aiControlSem);
	if(aiControl->curProcessOfUsePPU!=-1){
		up(&aiHwSem[AI_HW_PPU]);
		AI_INFO("release PPU sem hold by %d\n",aiControl->curProcessOfUsePPU);
		aiControl->curProcessOfUsePPU=-1;
	}

	if(aiControl->curProcessOfUseNNU!=-1){
		up(&aiHwSem[AI_HW_NNU]);
		AI_INFO("release NNU sem hold by %d\n",aiControl->curProcessOfUseNNU);
		aiControl->curProcessOfUseNNU=-1;
	}

	if(aiControl->curProcessOfUseOutput!=-1){
		AI_INFO("release OUTPUT sem hold by %d\n",aiControl->curProcessOfUseOutput);
		up(&aiHwSem[AI_HW_OUTPUT]);
		aiControl->curProcessOfUseOutput=-1;
	}

	//reset information
	for(i=0;i<AI_MAX_PROCESS;i++){
		if(aiControl->aiProcessInfo[i].activated){
			aiControl->aiProcessInfo[i].activated=0;
		}	
	}	
    aiControl->aiProcessNum=0;

	up(&aiControlSem);

	AI_INFO("rtk_ai_mc_release: handle=%d,aiProcessNum=%d\n",handle,aiControl->aiProcessNum);


	return TRUE; 
}

BOOL rtk_ai_mc_backup(void){
	down(&aiControlSem);
	memcpy((void *)aiControl_backup,(void *)aiControl,sizeof(AI_CONTROL));
	AI_INFO("rtk_ai_mc_backup: UsePPU=%d,UseNNU=%d,UseOutput=%d\n",aiControl->curProcessOfUsePPU,aiControl->curProcessOfUseNNU,aiControl->curProcessOfUseOutput);
	up(&aiControlSem);
	AI_INFO("rtk_ai_mc_backup done\n");
	return TRUE; 
}	

BOOL rtk_ai_mc_restore(void){
	down(&aiControlSem);
	memcpy((void *)aiControl,(void *)aiControl_backup,sizeof(AI_CONTROL));
	AI_INFO("rtk_ai_mc_restore: UsePPU=%d,UseNNU=%d,UseOutput=%d\n",aiControl->curProcessOfUsePPU,aiControl->curProcessOfUseNNU,aiControl->curProcessOfUseOutput);
	up(&aiControlSem);
	AI_INFO("rtk_ai_mc_restore done\n");
	return TRUE; 
}	

BOOL rtk_ai_mc_setClockRatio(int ratio){
	int value=0;
	
	//check handle
	if(ratio<=0||ratio>AI_MC_NN_MAX_CLOCK_RATIO){
		AI_ERROR("ratio is out of range %d(1~%d)\n",ratio,AI_MC_NN_MAX_CLOCK_RATIO);
		return FALSE;
	}else{
		AI_INFO("set clock ratio %d\n",ratio);
	}

	value=AI_MC_NN_MAX_CLOCK_RATIO-ratio;
#if defined(CONFIG_ARCH_RTK2885P)
#ifdef AI_VERIFY_ON_ZEBU
	//in rescue kernel
	writel(value,(void *)NNWP_nn_clkratio_ctrl_0_reg);
	writel(0x1,(void *)NNWP_nn_clkratio_ctrl_1_reg);		//apply
#else
	rtd_outl(NNWP_nn_clkratio_ctrl_0_reg,value);
	rtd_outl(NNWP_nn_clkratio_ctrl_1_reg,0x1);		//apply
#endif
#else
	AI_ERROR("setClockRatio is not supported in this platfom\n");
#endif
	
	return TRUE;
}

BOOL rtk_ai_mc_enableNpuLoadCheck(int enable){
	
	if(isNpuLoadCheck!=enable){
		if(enable){
			AI_INFO("start NPU loading check\n");
			ai_load_timer.expires = jiffies + AI_LOAD_TIMER_INTERVAL;
			add_timer(&ai_load_timer);

			//reset counter
			if((IoReg_Read32(SYS_REG_SYS_CLKEN0_reg)&SYS_REG_SYS_CLKEN0_clken_nnip_mask)){
				IoReg_Write32(AI_TOTAL_CYCLE_REG,0);
				IoReg_Write32(AI_IDLE_CYCLE_REG,0);
			}
		}
		npuLoadingAve=0;
		isNpuLoadCheck=enable;
	}

	AI_INFO("set isNpuLoadCheck = %d\n",enable);
	return TRUE;	
}

int rtk_ai_mc_getNpuLoad(void){
	AI_INFO("get npuLoading = %d\n",npuLoadingAve);
	return npuLoadingAve;
}

EXPORT_SYMBOL(rtk_ai_mc_enableNpuLoadCheck);
EXPORT_SYMBOL(rtk_ai_mc_getNpuLoad);