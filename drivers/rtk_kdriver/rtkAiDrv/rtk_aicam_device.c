/************************************************************************
 *  Include files
 ************************************************************************/
#include <linux/uaccess.h>
#include <linux/bitops.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/export.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/io.h>
//#include <mach/io.h>
#include <linux/irq.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/input.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/kfifo.h>
#include <linux/kthread.h>
#include <linux/of.h>
#include <linux/pci.h>
#include <linux/poll.h>
#include <linux/platform_device.h>
#include <linux/random.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/signal.h>
#include <linux/stat.h>
#include <linux/string.h>
#include <linux/pageremap.h>
#include <linux/compat.h>
#include <linux/timer.h>
#include <linux/semaphore.h>
#include <ioctrl/ai/aicam_cmd_id.h>
#include "rtk_otp_region_api.h"
#include <rtd_log/rtd_module_log.h>
#include <rtk_kdriver/RPCDriver.h>
#include <scaler/scalerCommon.h>
#include <scaler_vscdev.h>
#include <scaler_vpqdev.h>
#include "rbus/dc_sys_reg.h"
#include <rtk_kdriver/rtk_dc_mt.h>

/************************************************************************
 *  Definitions
 ************************************************************************/
#define AICAM_DEBUG(fmt, args...)  rtd_pr_ai_dbg_notice("[AICAM]"fmt, ## args)
#define AICAM_INFO(fmt, args...)   rtd_pr_ai_dbg_info("[AICAM]" fmt, ## args)
#define AICAM_WARN(fmt, args...)   rtd_pr_ai_dbg_warn("[AICAM]" fmt, ## args)
#define AICAM_ERROR(fmt, args...)  rtd_pr_ai_dbg_err("[AICAM]" fmt, ## args)

//#define TRUE		1
//#define FALSE		0
#define VSC_ZOOM_TEST	1
#define ENABLE_SE_ZOOM	0
#define ENABLE_RELEASE_CMA	1
#define ENABLE_DCMT_SETUP	0

/************************************************************************
*  Public variables
************************************************************************/
static struct cdev *dev_cdevp_aicam = NULL;
static struct class *_aicam_class;
static struct device *aicam_device_st;
static struct platform_device *rtk_aicam_platform_dev;
static int aicam_dev_major;
static int aicam_dev_minor;

AI_CAM_INFO *aiCamInfo=0;
int aiCamInit=0;
static int refCount=0;
static unsigned char *buff_cache[AI_CAM_MAX_FRAME_NUM];
static unsigned char *buff_uncache[AI_CAM_MAX_FRAME_NUM];
static unsigned char *result_cache[AI_CAM_MAX_FRAME_NUM];
static unsigned char *result_uncache[AI_CAM_MAX_FRAME_NUM];
static unsigned char *caminfo_cache=NULL;
static struct timer_list aicam_timer;
struct semaphore aicamSem;

extern unsigned char rtk_hal_vsc_Zoom_Init(void *camInfoPtr);
extern unsigned char rtk_hal_vsc_Zoom_Run(void);
extern unsigned char rtk_hal_vsc_Zoom_Stop(void);
extern unsigned char rtk_hal_vsc_Zoom_Flush(void);
extern unsigned char rtk_hal_vsc_open(VIDEO_WID_T wid);
extern unsigned char rtk_hal_vsc_close(VIDEO_WID_T wid);
extern unsigned char rtk_hal_vsc_Connect(VIDEO_WID_T wid, KADP_VSC_INPUT_SRC_INFO_T inputSrcInfo, KADP_VSC_OUTPUT_MODE_T outputMode);
extern unsigned char rtk_hal_vsc_Disconnect(VIDEO_WID_T wid, KADP_VSC_INPUT_SRC_INFO_T inputSrcInfo, KADP_VSC_OUTPUT_MODE_T outputMode);
extern unsigned char rtk_hal_vsc_SetInputRegionEx(VIDEO_WID_T wid, VIDEO_RECT_T  inregion, VIDEO_RECT_T originalInput);
extern unsigned char rtk_hal_vsc_SetOutputRegion(VIDEO_WID_T wid, KADP_VIDEO_RECT_T outregion, unsigned short Wide, unsigned short High);
extern unsigned char rtk_hal_vsc_SetWinBlank(VIDEO_WID_T wid, bool bonoff, KADP_VIDEO_DDI_WIN_COLOR_T color);
extern unsigned char Camera_Dump_DC2H_buffer(void);
extern void Camera_DC2H_init(unsigned int startAddr,unsigned int capW, unsigned int capH);
extern void Camera_DC2H_uninit(void);

#define CAM_OUTPUT_W AI_CAM_MODEL_MAX_OUTPUT_W
#define CAM_OUTPUT_H AI_CAM_MODEL_MAX_OUTPUT_H

/************************************************************************
*  Function body
************************************************************************/

#if VSC_ZOOM_TEST

#include <vgip_isr/scalerAI.h>
#include <gal/kadp_hal_gal.h>

static AI_CAM_INFO *aiCamInfoPtr;
static AI_AUDIO aiAudioResult;
static unsigned int nnBwLimit=0x20;
#if ENABLE_SE_ZOOM
static struct task_struct *aicamTestTsk;
static int aicamTestData;
static int aiCamScalerStop=1;
static unsigned int nnBwLimit=0x20;

#if 1
#define CAM_OUTPUT_W AI_CAM_MODEL_MAX_OUTPUT_W
#define CAM_OUTPUT_H AI_CAM_MODEL_MAX_OUTPUT_H
#else
#define CAM_OUTPUT_W 3840
#define CAM_OUTPUT_H 2160
#endif

static int rtk_aicam_test_thread(void *arg){
	SE_NN_info info;
	bool status = 0;
	
	AICAM_INFO("rtk_aicam_test_thread() start\n" );
	
	while(1){
		msleep(3000);
		Camera_Dump_DC2H_buffer();
		
		if(aiCamScalerStop){
			continue;
		}
#if 0		
		//write buffer task
		if(aiCamInfoPtr->frame[aiCamInfoPtr->scaler_rd].isValid==1&&aiCamInfoPtr->frame[aiCamInfoPtr->scaler_rd].detectDone==1){

			AICAM_INFO("rtk_aicam_test_thread SE update index %d, start\n",aiCamInfoPtr->scaler_rd);	
			//use SE to scale up
			info.src_x=aiCamInfoPtr->frame[aiCamInfoPtr->scaler_rd].inregion.x;
			info.src_y=aiCamInfoPtr->frame[aiCamInfoPtr->scaler_rd].inregion.y;
			info.src_w=aiCamInfoPtr->frame[aiCamInfoPtr->scaler_rd].inregion.w;
			info.src_h=aiCamInfoPtr->frame[aiCamInfoPtr->scaler_rd].inregion.h;
			info.src_pitch_y=aiCamInfoPtr->cam_w;
			info.src_phyaddr=aiCamInfoPtr->frame[aiCamInfoPtr->scaler_rd].phy_addr_align;
			info.src_phyaddr_uv=info.src_phyaddr+(aiCamInfoPtr->cam_w*aiCamInfoPtr->cam_h);
			info.src_fmt = KGAL_PIXEL_FORMAT_NV12;
			info.dst_x=0;//aiCamInfoPtr->frame[aiCamInfoPtr->scaler_rd].outregion.x;
			info.dst_y=0;//aiCamInfoPtr->frame[aiCamInfoPtr->scaler_rd].outregion.y;
			info.dst_w=aiCamInfoPtr->frame[aiCamInfoPtr->scaler_rd].outregion.w;
			info.dst_h=aiCamInfoPtr->frame[aiCamInfoPtr->scaler_rd].outregion.h;
			info.dst_phyaddr=aiCamInfoPtr->frame[aiCamInfoPtr->scaler_rd].result_phy_addr_align;
			info.dst_phyaddr_uv=info.dst_phyaddr+ALIGN_12K(info.dst_w*info.dst_h);
			info.dst_fmt = KGAL_PIXEL_FORMAT_NV12;
			status = scalerAI_SE_stretch_Proc(info);			
			if(status==0){
				AICAM_ERROR("rtk_aicam_test_thread SE fail\n");				
			}
			#if 0
			//update infor
			aiCamInfoPtr->frame[aiCamInfoPtr->scaler_rd].scaleDone=1;
			AICAM_INFO("rtk_aicam_test_thread SE update index %d, frame_max=%d\n",aiCamInfoPtr->scaler_rd,aiCamInfoPtr->frame_max);	
			aiCamInfoPtr->scaler_rd=aiCamInfoPtr->scaler_rd+1;
			if(aiCamInfoPtr->scaler_rd>=aiCamInfoPtr->frame_max){
				aiCamInfoPtr->scaler_rd=0;
			}	
			//AICAM_ERROR("rtk_aicam_test_thread SE next index %d\n",aiCamInfoPtr->scaler_rd);	
			#endif
		}
		#endif
	}
}
static void rtk_aicam_scaler_test(void)
{
	//create test thread
    aicamTestTsk = kthread_create(rtk_aicam_test_thread, &aicamTestData, "AICAM test thread");
    if (IS_ERR(aicamTestTsk)) {
    	int ret;
        ret = PTR_ERR(aicamTestTsk);
        aicamTestTsk = NULL;
        AICAM_ERROR("create rtk_aicam_scaler_test thread fail\n");
        return;
    }
    wake_up_process(aicamTestTsk);	
}
#endif

unsigned char rtk_hal_vsc_Zoom_Init(void *camInfoPtr)
{
	unsigned long ret = 0;
	unsigned long phyAddr;
	AICAM_INFO("%s start\n",__FUNCTION__);
	
	if(camInfoPtr==NULL){
		AICAM_ERROR("%s:camInfoPtr is null\n",__FUNCTION__);
		return FALSE;
	}
	
	aiCamInfoPtr=(AI_CAM_INFO *)camInfoPtr;
	
	phyAddr=dvr_to_phys(camInfoPtr);
	AICAM_INFO("%s:vir=0x%lx,phy=0x%lx\n",__FUNCTION__,(unsigned long)camInfoPtr,phyAddr);
	if (send_rpc_command(RPC_VIDEO, VIDEO_RPC_CAMERA_VO_ToAgent_Open ,phyAddr, 0, &ret) == RPC_FAIL) 
    {
        AICAM_ERROR("[vo] VIDEO_RPC_CAMERA_VO_ToAgent_Open fail!!\n");
        return FALSE;
    }
#if ENABLE_SE_ZOOM	
 	Camera_DC2H_init(AI_CAM_MODEL_MAX_OUTPUT_W, AI_CAM_MODEL_MAX_OUTPUT_H);
	rtk_aicam_scaler_test();
#endif	
	
	return TRUE;
}

extern DEBUG_SCALER_VSC_STRUCT DbgSclrFlgTkr;

unsigned char rtk_hal_vsc_Zoom_Run(void)
{
	VIDEO_RECT_T inputregion;
	KADP_VIDEO_RECT_T outputregion ;
	KADP_VSC_INPUT_SRC_INFO_T InputsourceInfo = {KADP_VSC_INPUTSRC_MAXN, 0 , 0};
    unsigned long ret = 0;
	AICAM_INFO("%s start\n",__FUNCTION__);

	inputregion.x = 0;
	inputregion.y = 0;
	inputregion.w = aiCamInfoPtr->cam_w;
	inputregion.h = aiCamInfoPtr->cam_h;
	outputregion.x = 0;
	outputregion.y = 0;
	outputregion.w = aiCamInfoPtr->outregion.w;//CAM_OUTPUT_W;
	outputregion.h = aiCamInfoPtr->outregion.h;//CAM_OUTPUT_H;
	InputsourceInfo.type = KADP_VSC_INPUTSRC_VDEC;

	if(aiCamInfo->frame[0].result_phy_addr_align){
		AICAM_INFO("%s Camera_DC2H_init,addr=0x%x\n",__FUNCTION__,aiCamInfo->frame[0].result_phy_addr_align);
	 	Camera_DC2H_init(aiCamInfo->frame[0].result_phy_addr_align,aiCamInfoPtr->outregion.w, aiCamInfoPtr->outregion.h);
		AICAM_INFO("%s Camera_DC2H_init done\n",__FUNCTION__);
	}

    DbgSclrFlgTkr.adaptive_stream = 1;
	rtk_hal_vsc_open(VIDEO_WID_0);
	rtk_hal_vsc_Connect(VIDEO_WID_0 , InputsourceInfo, KADP_VSC_OUTPUT_DISPLAY_MODE);
	rtk_hal_vsc_SetInputRegionEx(VIDEO_WID_0,inputregion,inputregion);
	rtk_hal_vsc_SetOutputRegion(VIDEO_WID_0,outputregion,0,0);
    rtk_hal_vsc_SetWinBlank(VIDEO_WID_0, 0, KADP_VIDEO_DDI_WIN_COLOR_BLACK);
	AICAM_INFO("%s cam width:%d,height=%d\n",__FUNCTION__, aiCamInfoPtr->cam_w, aiCamInfoPtr->cam_h);
	AICAM_INFO("%s out width:%d,height=%d\n",__FUNCTION__, aiCamInfoPtr->outregion.w, aiCamInfoPtr->outregion.h);
    if (send_rpc_command(RPC_VIDEO, VIDEO_RPC_CAMERA_VO_ToAgent_Run , 0, 0, &ret) == RPC_FAIL) 
    {
        AICAM_ERROR("[vo] VIDEO_RPC_CAMERA_VO_ToAgent_Run fail!!\n");
        return -1;
    }
#if ENABLE_SE_ZOOM	
	aiCamScalerStop=0;
#endif	
	AICAM_INFO("%s done\n",__FUNCTION__);
	return TRUE;
}

unsigned char rtk_hal_vsc_Zoom_Stop(void)
{
  unsigned long ret = 0;
	KADP_VSC_INPUT_SRC_INFO_T InputsourceInfo = {KADP_VSC_INPUTSRC_MAXN, 0 , 0};

	AICAM_INFO("%s start\n",__FUNCTION__);

	rtk_hal_vsc_SetWinBlank(VIDEO_WID_0, 1, KADP_VIDEO_DDI_WIN_COLOR_BLACK);

    if (send_rpc_command(RPC_VIDEO, VIDEO_RPC_CAMERA_VO_ToAgent_Stop , 0, 0, &ret) == RPC_FAIL) 
    {
        AICAM_ERROR("[vo] VIDEO_RPC_CAMERA_VO_ToAgent_Stop fail!!\n");
        return -1;
    }

	InputsourceInfo.type = KADP_VSC_INPUTSRC_VDEC;
	rtk_hal_vsc_Disconnect(VIDEO_WID_0 , InputsourceInfo, KADP_VSC_OUTPUT_DISPLAY_MODE);
	rtk_hal_vsc_close(VIDEO_WID_0);

	Camera_DC2H_uninit();

	AICAM_INFO("%s done\n",__FUNCTION__);

#if ENABLE_SE_ZOOM	
	aiCamScalerStop=1;
#endif
	return TRUE;	
}

unsigned char rtk_hal_vsc_Zoom_Flush(void)
{
	AICAM_INFO("%s start\n",__FUNCTION__);
	return TRUE;
}

#endif

static int rtk_aicam_init(void)
{
	int i,size;
#if ENABLE_DCMT_SETUP	
    DC_MT_DESC mt_desc;
#endif
	
    AICAM_INFO("%s \n" , __func__);
	
	down(&aicamSem);
	
	if(aiCamInit>0){
	    AICAM_ERROR("already init\n");
	    refCount++;
	    up(&aicamSem);
		return TRUE;
	}

	//init buffers
	//smaller than 32KB,use CMA1, otherwise use CMA2
	if(caminfo_cache==NULL){
		caminfo_cache=dvr_malloc_uncached_specific(sizeof(AI_CAM_INFO),GFP_DCU1,(void **)&aiCamInfo);
		memset((void *)aiCamInfo,0,sizeof(AI_CAM_INFO));
	    AICAM_INFO("clear aiCamInfo\n");
	}
	
	if(caminfo_cache==NULL){
		AICAM_ERROR("%s:dvr_malloc_uncached %d Bytes fail\n", __func__,(unsigned int)sizeof(AI_CAM_INFO));
	    up(&aicamSem);
		return FALSE;
	}
	
	AICAM_INFO("aiCamInfo=0x%lx,cached addr=0x%lx\n",(unsigned long)aiCamInfo,caminfo_cache);

	aiCamInfo->frame_max=AI_CAM_MAX_FRAME_NUM;
	aiCamInfo->rtkcam_cur_wd=0;
	aiCamInfo->rtkcam_pre_wd=0;
	aiCamInfo->rtkcam_ai_rd=0;
	aiCamInfo->scaler_rd=0;
	aiCamInfo->cam_w=0;
	aiCamInfo->cam_h=0;
	aiCamInfo->latest_buffer=NULL;
	aiCamInfo->latest_timestamp.tv_sec=0;
	aiCamInfo->latest_timestamp.tv_nsec=0;

	for(i=0;i<aiCamInfo->frame_max;i++){
		aiCamInfo->frame[i].width=0;
		aiCamInfo->frame[i].height=0;
		size=AI_CAM_MAX_W*AI_CAM_MAX_H*3/2;
		size=ALIGN_4K(size);
		if(buff_cache[i]==NULL){
			buff_cache[i]=dvr_malloc_uncached_specific(size, GFP_DCU2,(void **)&aiCamInfo->frame[i].pVir_addr_align);
			buff_uncache[i]=(unsigned char *)aiCamInfo->frame[i].pVir_addr_align;
			if(buff_cache[i]==NULL){
				AICAM_ERROR("buuf[%d] malloc %ld bytes fail\n",i,size);
			    up(&aicamSem);
				return FALSE;
			}else{
				AICAM_INFO("buuf[%d] malloc %ld bytes success\n",i,size);
				aiCamInfo->frame[i].phy_addr_align = (unsigned int)dvr_to_phys((void *)(buff_cache[i]));
				AICAM_INFO("phy=0x%llx, vir=0x%llx\n",aiCamInfo->frame[i].phy_addr_align,aiCamInfo->frame[i].pVir_addr_align);
			}
		}else{
			aiCamInfo->frame[i].pVir_addr_align=(u32 *)buff_uncache[i];
			AICAM_INFO("phy=0x%llx, vir=0x%llx\n",aiCamInfo->frame[i].phy_addr_align,aiCamInfo->frame[i].pVir_addr_align);
		}
		size=AI_CAM_MODEL_MAX_OUTPUT_W*AI_CAM_MODEL_MAX_OUTPUT_H*2;
		size=ALIGN_4K(size);
		if(result_cache[i]==NULL){
			result_cache[i]=dvr_malloc_uncached_specific(size, GFP_DCU1,(void **)&aiCamInfo->frame[i].result_pVir_addr_align);
			result_uncache[i]=(unsigned char *)aiCamInfo->frame[i].result_pVir_addr_align;
			if(result_cache[i]==NULL){
				AICAM_ERROR("result[%d] malloc %ld bytes fail\n",i,size);
			    up(&aicamSem);
				return FALSE;
			}else{
				AICAM_INFO("result[%d] malloc %ld bytes success\n",i,size);
				aiCamInfo->frame[i].result_pVir_addr_align=(u32 *)ALIGN_12K((u64)aiCamInfo->frame[i].result_pVir_addr_align);
				aiCamInfo->frame[i].result_phy_addr_align = (unsigned int)dvr_to_phys((void *)(ALIGN_12K((u64)result_cache[i])));
				AICAM_INFO("phy=0x%llx, vir=0x%llx\n",aiCamInfo->frame[i].result_phy_addr_align,aiCamInfo->frame[i].result_pVir_addr_align);
			}
		}else{
			aiCamInfo->frame[i].result_pVir_addr_align=(u32 *)result_uncache[i];
			AICAM_INFO("phy=0x%llx, vir=0x%llx\n",aiCamInfo->frame[i].result_phy_addr_align,aiCamInfo->frame[i].result_pVir_addr_align);
		}
		aiCamInfo->frame[i].ts_valid.tv_sec=0;
		aiCamInfo->frame[i].ts_valid.tv_nsec=0;
		aiCamInfo->frame[i].ts_detect.tv_sec=0;
		aiCamInfo->frame[i].ts_detect.tv_nsec=0;
		aiCamInfo->frame[i].ts_scale.tv_sec=0;
		aiCamInfo->frame[i].ts_scale.tv_nsec=0;
		aiCamInfo->frame[i].isValid=0;
		aiCamInfo->frame[i].detectDone=0;
		aiCamInfo->frame[i].scaleDone=0;
		
		//test
		aiCamInfo->frame[i].pVir_addr_align[1]=i;
		aiCamInfo->frame[i].result_pVir_addr_align[1]=i;
		AICAM_INFO("test %d ok\n",i);
	}

	//rtk_hal_vsc_Zoom_Init((void *)aiCamInfo);

	aiAudioResult.isVoiced=0;
	aiAudioResult.micAzimuth=0;

	aiCamInit=1;
	
	refCount=1;

    up(&aicamSem);
    
#if ENABLE_DCMT_SETUP

	for(i=0;i<aiCamInfo->frame_max;i++){
	    memset(&mt_desc, 0, sizeof(mt_desc));
	    mt_desc.type = MEMORY_TYPE_SERIAL_BLOCK;
	    mt_desc.start = aiCamInfo->frame[i].result_phy_addr_align;
	    mt_desc.end = aiCamInfo->frame[i].result_phy_addr_align+size;
	    strncpy(mt_desc.name, "DC2H",20);
	    mt_desc.mode = 3;
	    mt_desc.ip_entry[0].module = MODULE_TVSB4_DC2H;//all module
	    mt_desc.ip_entry[0].prot = PROT_MODE_WO; //access type read
	
		dcmt_clear_monitor(15+i);
	    dcmt_set_monitor(15+i,&mt_desc);	
	}
    AICAM_INFO("%s dcmt setup done\n" , __func__);
#endif
	
    AICAM_INFO("%s done\n" , __func__);
	return TRUE;	
}

static int rtk_aicam_unInit(void)
{
#if ENABLE_RELEASE_CMA
	int i;
	
#if ENABLE_DCMT_SETUP
	DC_MT_DESC mt_desc;

	for(i=0;i<aiCamInfo->frame_max;i++){
	    memset(&mt_desc, 0, sizeof(mt_desc));
	    mt_desc.type = MEMORY_TYPE_SERIAL_BLOCK;
	    mt_desc.start = aiCamInfo->frame[i].result_phy_addr_align;
	    mt_desc.end = aiCamInfo->frame[i].result_phy_addr_align+ALIGN_4K(AI_CAM_MODEL_MAX_OUTPUT_W*AI_CAM_MODEL_MAX_OUTPUT_H*2);
	    strncpy(mt_desc.name, "DC2H",20);
	    mt_desc.mode = 1;
	    mt_desc.ip_entry[0].module = MODULE_TVSB4_DC2H;//all module
	    mt_desc.ip_entry[0].prot = PROT_MODE_WO; //access type read
	
		dcmt_clear_monitor(15+i);
	    dcmt_set_monitor(15+i,&mt_desc);	
	}
    AICAM_INFO("%s dcmt setup done\n" , __func__);
#endif


    down(&aicamSem);
    refCount--;
    
    if(aiCamInit==1){
    	
    	if(refCount>0){
		    AICAM_INFO("%s, refCount(%d) > 0, do nothing\n", __func__,refCount);
    		up(&aicamSem);
    		return TRUE;
    		
    	}else{
		    AICAM_INFO("%s, release CMA \n", __func__);

		    //release memory
			for(i=0;i<aiCamInfo->frame_max;i++){
		    	if(buff_cache[i]){
		    		dvr_free(buff_cache[i]);
		    		buff_cache[i]=NULL;
		    	}
		    	if(result_cache[i]){
		    		dvr_free(result_cache[i]);
		    		result_cache[i]=NULL;
		    	}
			}
			
			if(caminfo_cache){
				dvr_free(caminfo_cache);
				caminfo_cache=NULL;
			}
		}
	}
#else
    AICAM_INFO("%s \n", __func__);

    down(&aicamSem);
#endif

	aiCamInit=0;
	
    up(&aicamSem);
    
    AICAM_INFO("%s done\n" , __func__);
	
	return TRUE;	
}

static int rtk_aicam_release_check(void)
{
	//may not run the complete release flow
	if(aiCamInit&&refCount<=1){

		AICAM_ERROR("release flow is not complete. just release all\n");
		//stop nnsr
		down(&aicamSem);
		rtk_hal_vsc_Zoom_Stop();
		up(&aicamSem);
		nnBwLimit=nnBwLimit/2;
		rtd_outl(DC_SYS_bw_request_threshold_9_reg,nnBwLimit&0x00FFFFFF);

		if(del_timer(&aicam_timer))
			AICAM_ERROR("del aicam_timer fail\n");
		else
			AICAM_INFO("del aicam_timer success\n");						
		
		
		rtk_aicam_unInit();
	}else{
		AICAM_ERROR("aiCamInit=%d,refCount=%d,do no release\n",aiCamInit,refCount);
	}

	return TRUE;	
}

static void rtk_aicam_backup_info(void)
{
    AICAM_INFO("%s \n" , __func__);
    if(aiCamInit>0){
		AICAM_INFO("cam width:%d,height=%d\n",aiCamInfoPtr->cam_w, aiCamInfoPtr->cam_h);
		AICAM_INFO("out width:%d,height=%d\n",aiCamInfoPtr->outregion.w, aiCamInfoPtr->outregion.h);
	}
	return;
}

static void rtk_aicam_restore_info(void)
{
    AICAM_INFO("%s \n" , __func__);
    if(aiCamInit>0){
		AICAM_INFO("cam width:%d,height=%d\n",aiCamInfoPtr->cam_w, aiCamInfoPtr->cam_h);
		AICAM_INFO("out width:%d,height=%d\n",aiCamInfoPtr->outregion.w, aiCamInfoPtr->outregion.h);
	}
	return;
}

void rtk_aicam_callback(struct timer_list *t)
{
	AICAM_INFO("DC_SYS_bw_request_threshold_9_reg=%d\n",rtd_inl(DC_SYS_bw_request_threshold_9_reg));
	rtd_outl(DC_SYS_bw_request_threshold_9_reg,nnBwLimit&0x00FFFFFF);
	AICAM_INFO("DC_SYS_bw_request_threshold_9_reg=%d\n",rtd_inl(DC_SYS_bw_request_threshold_9_reg));
}

#ifdef CONFIG_PM
/*static int rtk_pwm_suspend(struct platform_device *dev, pm_message_t state)*/
static int rtk_aicam_suspend(struct device *dev)
{
    AICAM_INFO("suspend\n");
    rtk_aicam_backup_info();
    AICAM_INFO("suspend done\n");
    return 0;
}

/*static int rtk_pwm_resume(struct platform_device *dev)*/
static int rtk_aicam_resume(struct device *dev)
{
//	int ret=0;

    AICAM_INFO("resume\n");
    rtk_aicam_restore_info();
    AICAM_INFO("resume done\n");
    return 0;
}

#ifdef CONFIG_HIBERNATION
static int rtk_aicam_suspend_std(struct device *dev)
{
    AICAM_WARN("%s \n" , __func__);
    rtk_aicam_backup_info();
    return 0;
}
static int rtk_aicam_resume_std(struct device *dev)
{
    AICAM_WARN("%s \n" , __func__);
    rtk_aicam_restore_info();
    return 0;
}

static int rtk_aicam_poweroff(struct device *dev)
{
    AICAM_WARN("%s \n" , __func__);
    return 0;
}
static int rtk_aicam_restore(struct device *dev)
{
    AICAM_WARN("%s \n" , __func__);
    rtk_aicam_restore_info();
    return 0;
}
#endif

static const struct dev_pm_ops rtk_aicam_pm_ops = {

    .suspend_noirq = rtk_aicam_suspend,
    .resume_noirq = rtk_aicam_resume,
#ifdef CONFIG_HIBERNATION
    .freeze_noirq     = rtk_aicam_suspend_std,
    .thaw_noirq       = rtk_aicam_resume_std,
    .poweroff_noirq     = rtk_aicam_poweroff,
    .restore_noirq       = rtk_aicam_restore,    
#endif
};

#endif

static int rtk_aicam_probe(struct platform_device *pdev)
{
    AICAM_WARN("%s():\n", __FUNCTION__);
    return 0;
}

static int rtk_aicam_remove(struct platform_device *pdev)
{
    AICAM_WARN("%s():\n", __FUNCTION__);
    return 0;
}

ssize_t rtk_aicam_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    AICAM_WARN("%s():\n", __FUNCTION__);
    return -EFAULT;
}
ssize_t rtk_aicam_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    AICAM_WARN("%s():\n", __FUNCTION__);
    return -EFAULT;
}

int rtk_aicam_open(struct inode *inode, struct file *filp)
{
    AICAM_WARN("%s():\n", __FUNCTION__);
    //filp->private_data = (void *)((unsigned long)rtk_ai_mc_getNextHandle());
    //AI_INFO("%s():private_data=%d\n",(int)__FUNCTION__,(int)filp->private_data);
    return 0;
}

int rtk_aicam_release(struct inode *inode, struct file *filp)
{
    AICAM_WARN("%s():\n", __FUNCTION__);
    //AI_INFO("%s():private_data=%d\n",__FUNCTION__,(int)filp->private_data);
    rtk_aicam_release_check();
    return 0;
}

long rtk_aicam_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int retval = 0;
    unsigned int paramters=0;
    unsigned int phyAddr;

    //AICAM_INFO("%s() enter %d\n", __FUNCTION__,cmd);

    switch(cmd){
    case AICAM_IOC_INIT:
    	if(rtk_aicam_init()==FALSE){
				AICAM_ERROR("already init\n");
				retval=-EFAULT;
    	}
        break;
    case AICAM_IOC_UNINIT:
    	rtk_aicam_unInit();
        break;
    case AICAM_IOC_OPEN:
        break;
    case AICAM_IOC_CLOSE:
        break;
    case AICAM_IOC_GET_SHARE_MEM:
		if(aiCamInit>0){
			phyAddr=(unsigned long)dvr_to_phys((void *)aiCamInfo);
			AICAM_INFO("phyAddr=0x%llx,aiCamInfo=0x%llx\n",phyAddr,(unsigned long)aiCamInfo);
			if(copy_to_user((unsigned int __user*)arg,&phyAddr,4)){
				AICAM_ERROR("AICAM_IOC_GET_SHARE_MEM failed, copy data to user space failed\n");
				retval=-EFAULT;
			}
		}else{
			AICAM_ERROR("AICAM_IOC_GET_SHARE_MEM failed, not initialized\n");
			retval=-EFAULT;
		}
        break;
    case AICAM_IOC_SET_SCALER:
    	if(aiCamInit==0){
			AICAM_ERROR("AICAM_IOC_GET_SHARE_MEM failed, not initialized\n");
			retval=-EFAULT;
    	}else{
    		if(copy_from_user(&paramters, (void __user *)arg,sizeof(unsigned int))) {
				AICAM_ERROR("AICAM_IOC_CMD_SET_SCALER failed, copy data from user space failed\n");
				retval=-EFAULT;
        	}else{
				AICAM_INFO("AICAM_IOC_CMD_SET_SCALER command=%d\n",paramters);
		        switch(paramters){
		        	case AICAM_SCALER_START:
		        		down(&aicamSem);
		        		rtk_hal_vsc_Zoom_Init((void *)aiCamInfo);
		        		rtk_hal_vsc_Zoom_Run();
		        		up(&aicamSem);
		        		nnBwLimit=rtd_inl(DC_SYS_bw_request_threshold_9_reg);//store original value
						nnBwLimit=nnBwLimit*2;
						
						timer_setup(&aicam_timer,rtk_aicam_callback,0);
						if (mod_timer(&aicam_timer, jiffies + msecs_to_jiffies(5000)))
							AICAM_ERROR("setup aicam_timer fail\n");
						else
							AICAM_INFO("setup aicam_timer success\n");						
		        		
		        		break;
		        	case AICAM_SCALER_STOP:
		        		down(&aicamSem);
		        		rtk_hal_vsc_Zoom_Stop();
		        		up(&aicamSem);
		        		nnBwLimit=nnBwLimit/2;
		        		rtd_outl(DC_SYS_bw_request_threshold_9_reg,nnBwLimit&0x00FFFFFF);

						if(del_timer(&aicam_timer))
							AICAM_ERROR("del aicam_timer fail\n");
						else
							AICAM_INFO("del aicam_timer success\n");						

		        		break;
		        	case AICAM_SCALER_FLUSH:
		        		rtk_hal_vsc_Zoom_Flush();
		        		break;
		        	default:
		        		AICAM_ERROR("AICAM_IOC_CMD_SET_SCALER failed, unknown command %d\n",paramters);
		        		retval=-EFAULT;
		        		break;
		        }
	    	}
    	}
        break;
    case AICAM_IOC_SET_AUDIO:
		if(copy_from_user(&aiAudioResult, (void __user *)arg,sizeof(AI_AUDIO))) {
			AICAM_ERROR("AICAM_IOC_SET_AUDIO failed, copy data from user space failed\n");
			retval=-EFAULT;
    	}else{
			//AICAM_ERROR("AICAM_IOC_SET_AUDIO ok, isVoiced=%d,micAzimuth=%d\n",aiAudioResult.isVoiced,aiAudioResult.micAzimuth);
		}
    	
		break;
    case AICAM_IOC_GET_AUDIO:
		if(copy_to_user((unsigned int __user*)arg,&aiAudioResult,sizeof(AI_AUDIO))){
			AICAM_ERROR("AICAM_IOC_GET_AUDIO failed, copy data to user space failed\n");
			retval=-EFAULT;
		}else{
			//AICAM_ERROR("AICAM_IOC_GET_AUDIO ok, isVoiced=%d,micAzimuth=%d\n",aiAudioResult.isVoiced,aiAudioResult.micAzimuth);
		}
		break;
    default:
        retval = -ENOTTY;
        break;
    }

    //AICAM_INFO("%s() exit %d\n", __FUNCTION__,retval);

    return retval;

}

#ifdef CONFIG_COMPAT
long rtk_aicam_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    void __user *compat_arg = compat_ptr(arg);
    return rtk_aicam_ioctl(filp, cmd, (unsigned long)compat_arg);
}
#endif

static struct file_operations rtk_aicam_fops = {
    .owner = THIS_MODULE,
    .read = rtk_aicam_read,
    .write = rtk_aicam_write,
    .unlocked_ioctl = rtk_aicam_ioctl,
    .release = rtk_aicam_release,
    .open = rtk_aicam_open,
#ifdef CONFIG_COMPAT
    .compat_ioctl = rtk_aicam_compat_ioctl,
#endif
};


static const struct of_device_id rtk_aicam_devices[] = {
    {.compatible = "rtk-aicam",},
    {},
};

static struct platform_driver rtk_aicam_driver = {
    .driver = {
            .name = "rtk-aicam",
            .owner = THIS_MODULE,
            .of_match_table = of_match_ptr(rtk_aicam_devices),
            #ifdef CONFIG_PM
            .pm    = &rtk_aicam_pm_ops,
            #endif
        },
    .probe = rtk_aicam_probe,
    .remove = rtk_aicam_remove,
};

static int register_aicam_cdev(void) {
    dev_t dev;
    int ret = 0;


    ret = alloc_chrdev_region(&dev, 0, 1, AICAM_DEVICE_NAME);
    if(ret){
        AICAM_ERROR("can't alloc chrdev\n");
        return -1;
    }

    aicam_dev_major = MAJOR(dev);
    aicam_dev_minor = MINOR(dev);


    dev_cdevp_aicam = kzalloc(sizeof(struct cdev), GFP_KERNEL);
    if(dev_cdevp_aicam == NULL){
        AICAM_ERROR("kzalloc failed\n");
        goto FAIL_TO_ALLOCATE_AICAM_CDEV;
    }
    cdev_init(dev_cdevp_aicam, &rtk_aicam_fops);
    dev_cdevp_aicam->owner = THIS_MODULE;
    dev_cdevp_aicam->ops = &rtk_aicam_fops;
    ret = cdev_add(dev_cdevp_aicam, MKDEV(aicam_dev_major, aicam_dev_minor), 1);
    if(ret < 0){
        AICAM_WARN(" add chr dev failed\n");
        goto FAIL_TO_ADD_AICAM_CDEV;
    }

    aicam_device_st = device_create(_aicam_class, NULL, MKDEV(aicam_dev_major, aicam_dev_minor), NULL, AICAM_DEVICE_NAME);
    if(aicam_device_st == NULL){
        AICAM_WARN(" device create aicam dev failed\n");
        goto FAIL_TO_CREATE_AICAM_DEVICE;
    }
    AICAM_WARN("register chrdev(%d,%d) success.\n", aicam_dev_major, aicam_dev_minor);
    return 0;
    
FAIL_TO_CREATE_AICAM_DEVICE:
    cdev_del(dev_cdevp_aicam);
FAIL_TO_ADD_AICAM_CDEV:
    if(dev_cdevp_aicam){
        kfree(dev_cdevp_aicam);
        dev_cdevp_aicam = NULL;
    }
FAIL_TO_ALLOCATE_AICAM_CDEV:
    unregister_chrdev_region(MKDEV(aicam_dev_major, aicam_dev_minor), 1);
    aicam_dev_major = 0;
    aicam_dev_minor = 0;
    return -1;

}


static int unregister_aicam_cdev(void)
{
    device_destroy(_aicam_class, MKDEV(aicam_dev_major, aicam_dev_minor));
    cdev_del(dev_cdevp_aicam);
    kfree(dev_cdevp_aicam);
    dev_cdevp_aicam = NULL;
    unregister_chrdev_region(MKDEV(aicam_dev_major, aicam_dev_minor), 1);
    aicam_dev_major = 0;
    aicam_dev_minor = 0;
    return 0;

}



int __init rtk_aicam_dev_init(void)
{
    int ret = 0,i;
    
	if(rtk_is_NNIP_function_disable()){
		AICAM_ERROR( "[%s] not support at this chip\n",__func__);
		goto FAILED_REGISTER_PLATFORM_DRIVER;
    }
    
    ret = platform_driver_register(&rtk_aicam_driver);
    if(ret != 0){
        goto FAILED_REGISTER_PLATFORM_DRIVER;
	}

    _aicam_class = class_create(THIS_MODULE, "rtk-aicam");

    if(_aicam_class == NULL){
        goto FAILED_CREATE_AICAM_CLASS;
	}

    rtk_aicam_platform_dev =platform_device_register_simple("rtk-aicam", -1, NULL, 0);
    if(rtk_aicam_platform_dev == NULL){
        goto FAILED_REGISTER_PLATFORM_DEVICE;
	}

	if(register_aicam_cdev() != 0){
		goto FAILED_REGISTER_AICAM_CDEV;
	}

	sema_init(&aicamSem,1);

	for(i=0;i<AI_CAM_MAX_FRAME_NUM;i++){
		buff_cache[i]=NULL;
		result_cache[i]=NULL;
	}

    AICAM_INFO( "[%s] rtkaicam device init ok !\n", __func__);

    goto FINISH_DONE;
    
FAILED_REGISTER_AICAM_CDEV:
    platform_device_unregister(rtk_aicam_platform_dev);
FAILED_REGISTER_PLATFORM_DEVICE:
    device_destroy(_aicam_class, MKDEV(0, 0));
    class_destroy(_aicam_class);
FAILED_CREATE_AICAM_CLASS:
    platform_driver_unregister(&rtk_aicam_driver);
FAILED_REGISTER_PLATFORM_DRIVER:
    AICAM_ERROR( "[%s] ======================================\n",__func__);
    AICAM_ERROR( "[%s] AICAM Register Error!\n", __func__);
    AICAM_ERROR( "[%s] ======================================\n",__func__);
	ret=-1;
FINISH_DONE:
	return ret;


}

static void __exit rtk_aicam_dev_uninit(void)
{
    unregister_aicam_cdev();
    platform_device_unregister(rtk_aicam_platform_dev);
    device_destroy(_aicam_class, MKDEV(0, 0));
    class_destroy(_aicam_class);
    platform_driver_unregister(&rtk_aicam_driver);
    //rtk_aicam_unInit();
}

EXPORT_SYMBOL(aiCamInit);
module_init(rtk_aicam_dev_init);
module_exit(rtk_aicam_dev_uninit);
MODULE_AUTHOR("joshlin, Realtek Semiconductor");
MODULE_LICENSE("GPL");


