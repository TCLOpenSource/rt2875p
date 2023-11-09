#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/pageremap.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/string.h>

#include <base_types.h>
#include <tp/tp_def.h>
#include <tp/tp_drv_api.h>

#define CFG_RTK_CMA_MAP 0

#include <linux/kernel.h>
#include <linux/tee.h>
#include <linux/uuid.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/tee_drv.h>
#include <rbus/timer_reg.h>
#include <tp/tp_dbg.h>
#include "rtkdemux.h"
#include "rtkdemux_debug.h"
extern void rtk_flush_range (const void *, const void *);
extern void rtk_inv_range   (const void *, const void *);
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
#include <asm/cacheflush.h>
#else
#include <asm/outercache.h>
#include <asm/cacheflush.h>
#endif

#define TEE_NUM_PARAM 4
//////////////////////////////////////////////////this  part should sync with re//////////////////////////////////////////////////////////////
static const uuid_t demux_uuid =
	UUID_INIT(0x9bdca48d, 0x0244, 0x4cb4, 
			  0xb0, 0x53, 0xf2, 0xd5, 0x0f, 0x98, 0x43, 0xd8);


#define DEMUX_TA_ECP_SETUP                   0
#define DEMUX_TA_ECP_DESTROY                 1
#define DEMUX_TA_ECP_PARSE                   2
#define DEMUX_TA_ECP_DELIVERDATA             3
#define DEMUX_TA_ECP_UPDATE_PININFO          4
#define DEMUX_TA_ECP_UPDATE_PES_BUFFERINFO   5
#define DEMUX_TA_ECP_DELIVER_INBAND_COMMAND  6
#define DEMUX_TA_ECP_INV_RANGE               7


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct optee_ta {
    struct tee_context *ctx;
    __u32 session;
};

static struct optee_ta DEMUX_TA;
static unsigned long dmxDevInfo[]={
	(unsigned long)&(((demux_dev*)0)->totalDroppingPin),                          //0
	(unsigned long)&(((demux_dev*)0)->pinMap),                                    //1
	(unsigned long)&(((demux_dev*)0)->ecp_pes_flag),                              //2
#if defined(DEMUX_AUDIO_USE_GLOBAL_BUFFER)
	//(unsigned long)&(((demux_dev*)0)->audioBuffer),
#endif
};
static unsigned long chInfo[]={
	(unsigned long)&(((demux_channel*)0)->state),                                  //0
	(unsigned long)&(((demux_channel*)0)->tunerHandle),                            //1
	(unsigned long)&(((demux_channel*)0)->tsPacketSize),                           //2
	(unsigned long)&(((demux_channel*)0)->demuxTargetTable),                       //3
	(unsigned long)&(((demux_channel*)0)->demuxTargetTableCB),                     //4
	(unsigned long)&(((demux_channel*)0)->activePESTarget),                        //5
	(unsigned long)&(((demux_channel*)0)->pesMap),                                 //6
	(unsigned long)&(((demux_channel*)0)->demuxIn),                                //7
	(unsigned long)&(((demux_channel*)0)->demuxOut),                               //8
	(unsigned long)&(((demux_channel*)0)->navBuffers),                             //9
	(unsigned long)&(((demux_channel*)0)->pinInfo),                                //10
	(unsigned long)&(((demux_channel*)0)->PCRPID),                                 //11
	(unsigned long)&(((demux_channel*)0)->demuxTargetInfo),                        //12
	(unsigned long)&(((demux_channel*)0)->pcrProcesser),                           //13
	(unsigned long)&(((demux_channel*)0)->pPcrRecoveryManager),                    //14
	(unsigned long)&(((demux_channel*)0)->adInfo),                                 //15
	(unsigned long)&(((demux_channel*)0)->audio_newformat_info),                   //16
	(unsigned long)&(((demux_channel*)0)->AdChannelTarget),                        //17
	(unsigned long)&(((demux_channel*)0)->phyAddrOffset),                          //18
	(unsigned long)&(((demux_channel*)0)->tpBuffer),                               //19
	(unsigned long)&(((demux_channel*)0)->tp_src),                                 //20
	(unsigned long)&(((demux_channel*)0)->bDropflag),                              //21
	(unsigned long)&(((demux_channel*)0)->ringFullCheckTimer),                     //22
	(unsigned long)&(((demux_channel*)0)->startStreaming),                         //23
	(unsigned long)&(((demux_channel*)0)->bsDecBuf),					         	//24
	(unsigned long)&(((demux_channel*)0)->bsDecBufH),			                  	//25
	(unsigned long)&(((demux_channel*)0)->ibDecBuf),						   		//26
	(unsigned long)&(((demux_channel*)0)->ibDecBufH),						   		//27
	(unsigned long)&(((demux_channel*)0)->pidFilterListCB),						   	//28
	(unsigned long)&(((demux_channel*)0)->psiBuffer),                         //29
	(unsigned long)&(((demux_channel*)0)->psiBufferH),						//30
	//(unsigned long)&(((demux_channel*)0)->audio_deliver_size),                     //28
	//(unsigned long)&(((demux_channel*)0)->video_deliver_size),                     //29

};
static int DEMUX_TA_match(struct tee_ioctl_version_data *data, const void *vers)
{
	return 1;
}


/* 0: success, -N: failure (N: value) */
static int Demux_ta_Init(void)
{
	int ret = 0, rc = 0;
	struct tee_ioctl_open_session_arg secion_open_arg;
	struct tee_ioctl_version_data vers = {
		.impl_id = TEE_IMPL_ID_OPTEE,
		.impl_caps = TEE_OPTEE_CAP_TZ,
		.gen_caps = TEE_GEN_CAP_GP,};


	memset(&DEMUX_TA, 0, sizeof(DEMUX_TA));
	DEMUX_TA.ctx = tee_client_open_context(NULL, DEMUX_TA_match, NULL, &vers);
	if (DEMUX_TA.ctx == NULL) {
		dmx_err("optee_demux_tp: no ta context\n");
		ret = -EINVAL;
		goto err;
	}

	memset(&secion_open_arg, 0, sizeof(secion_open_arg));
	memcpy(secion_open_arg.uuid, demux_uuid.b, TEE_IOCTL_UUID_LEN);
	secion_open_arg.clnt_login = TEE_IOCTL_LOGIN_PUBLIC;
	dmx_dbg("secion_open_arg uuid %pUl \n", secion_open_arg.uuid);

	rc = tee_client_open_session(DEMUX_TA.ctx, &secion_open_arg, NULL);
	if(rc){
		dmx_err("optee_demux_tp: open_session failed ret %x secion_open_arg %x", rc, secion_open_arg.ret);
		ret = -EINVAL;
		goto err;
	}
	if (secion_open_arg.ret) {
			ret = -EINVAL;
			goto err;
	}

	DEMUX_TA.session = secion_open_arg.session;

	dmx_dbg("open_session ok\n");
	return 0;
err:
	if (DEMUX_TA.session) {
		tee_client_close_session(DEMUX_TA.ctx, DEMUX_TA.session);
		dmx_err("optee_demux_tp: open failed close session \n");
		DEMUX_TA.session = 0;
	}
	if (DEMUX_TA.ctx) {
		tee_client_close_context(DEMUX_TA.ctx);
		dmx_err("optee_demux_tp: open failed close context\n");
		DEMUX_TA.ctx = NULL;
	}
	dmx_err("open_session fail\n");

	return ret;
}


static void Demux_ta_Deinit(void)
{
	if (DEMUX_TA.session) {
		tee_client_close_session(DEMUX_TA.ctx, DEMUX_TA.session);
		DEMUX_TA.session = 0;
	}

	if (DEMUX_TA.ctx) {
		tee_client_close_context(DEMUX_TA.ctx);
		DEMUX_TA.ctx = NULL;
	}
}
static int Demux_ta_Construct(demux_channel * pCh){

	int ret = 0, rc = 0;
	int chInfoLen = 0, devInfoLen = 0;
	struct tee_ioctl_invoke_arg invoke_arg;
	struct tee_param *param = NULL;
	struct tee_shm *chInfo_buf = NULL, *devInfo_buf = NULL;
	void * chInfo_va = NULL, *devInfo_va = NULL;
	if(DEMUX_TA.ctx == NULL) {
		dmx_err("optee_meminfo: no ta context\n");
		ret = -EINVAL;
		goto END;
	}
	//CALL PREPARE
	memset(&invoke_arg, 0, sizeof(invoke_arg));
	invoke_arg.func = DEMUX_TA_ECP_SETUP;
	invoke_arg.session = DEMUX_TA.session;
	invoke_arg.num_params = TEE_NUM_PARAM;

	//PARAM PREPARE
	param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
	if(NULL == param) {
		dmx_err("Demux_ta_Construct: kcalloc fail\n");
		ret = -EINVAL;
		goto END;
	}
	memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);

	param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[0].u.value.a = virt_to_phys((void*)demux_device); // channel PHYSICAL
	param[0].u.value.b = sizeof(demux_dev);

	param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[1].u.value.a = virt_to_phys((void*)pCh); // channel PHYSICAL
	param[1].u.value.b = sizeof(demux_channel);


	param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
	chInfoLen = sizeof(chInfo);
	chInfo_buf = tee_shm_alloc(DEMUX_TA.ctx, chInfoLen, TEE_SHM_MAPPED);
	if (chInfo_buf == NULL){
		ret = -ENOMEM;
		goto END;
	}
	chInfo_va = tee_shm_get_va(chInfo_buf, 0);
	if (chInfo_va == NULL) {
		ret = -ENOMEM;
		goto END;
	}
	memcpy(chInfo_va,chInfo, chInfoLen);
	param[2].u.memref.shm = chInfo_buf;
	param[2].u.memref.size = chInfoLen;

	param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
	devInfoLen = sizeof(dmxDevInfo);
	devInfo_buf = tee_shm_alloc(DEMUX_TA.ctx, devInfoLen, TEE_SHM_MAPPED);
	if (devInfo_buf == NULL){
		ret = -ENOMEM;
		goto END;
	}
	devInfo_va = tee_shm_get_va(devInfo_buf, 0);
	if (devInfo_va == NULL) {
		ret = -ENOMEM;
		goto END;
	}
	memcpy(devInfo_va,dmxDevInfo, devInfoLen);
	param[3].u.memref.shm = devInfo_buf;
	param[3].u.memref.size = devInfoLen;

	// unmap tp buffer
	rc = tee_client_invoke_func(DEMUX_TA.ctx, &invoke_arg, param);
	if (rc || invoke_arg.ret) {
		dmx_err("optee_meminfo: invoke failed ret %x invoke_arg.ret %x\n", rc, invoke_arg.ret);
		ret = -EINVAL;
		goto END;
	}
	ret = 0;
END:
	if (chInfo_buf){
		tee_shm_free(chInfo_buf);
		chInfo_buf = NULL;
	}
	if (devInfo_buf){
		tee_shm_free(devInfo_buf);
		devInfo_buf = NULL;
	}
	if (param){
		kfree(param);
		param = NULL;
	}

	return ret;
}


static int Demux_ta_Deconstruct(demux_channel *pCh){
	int ret = 0, rc = 0;
	struct tee_ioctl_invoke_arg invoke_arg;
	struct tee_param *param = NULL;


	if(DEMUX_TA.ctx == NULL) {
		dmx_err("optee_meminfo: no ta context\n");
		ret = -EINVAL;
		goto END;
	}
	//CALL PREPARE
	memset(&invoke_arg, 0, sizeof(invoke_arg));
	invoke_arg.func = DEMUX_TA_ECP_DESTROY;
	invoke_arg.session = DEMUX_TA.session;
	invoke_arg.num_params = TEE_NUM_PARAM;

	//PARAM PREPARE
	param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
	if(NULL == param) {
		dmx_err("Demux_ta_Deconstruct: kcalloc fail\n");
		ret = -EINVAL;
		goto END;
	}
	memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);


	param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[0].u.value.a = pCh->tunerHandle;

	param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

	param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

	param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

	rc = tee_client_invoke_func(DEMUX_TA.ctx, &invoke_arg, param);
	if (rc || invoke_arg.ret) {
		dmx_err("optee_meminfo: invoke failed ret %x invoke_arg.ret %x\n", rc, invoke_arg.ret);
		ret = -EINVAL;
		goto END;
	}
	ret = 0;
END:
	if (param){
		kfree(param);
		param = NULL;
	}

	return ret;
}
//=================================================================
int DEMUX_TA_ECP_inv_range(demux_channel *pCh, long pTPRelase, unsigned int len)
{
	int ret = 0, rc = 0;
	struct tee_ioctl_invoke_arg invoke_arg;
	struct tee_param *param = NULL;

	if(DEMUX_TA.ctx == NULL) {
		dmx_err("optee_meminfo: no ta context\n");
		ret = -EINVAL;
		goto END;
	}
	rtk_flush_range(pCh, (void*)((long)(pCh) +sizeof(demux_channel)));
	outer_flush_range(virt_to_phys((void*)pCh), virt_to_phys((void*)pCh)+sizeof(demux_channel));

	rtk_flush_range(demux_device, (void*)((long)demux_device + sizeof(demux_dev)) );
	outer_flush_range(virt_to_phys(demux_device), virt_to_phys(demux_device) + sizeof(demux_dev));

	//CALL PREPARE
	memset(&invoke_arg, 0, sizeof(invoke_arg));
	invoke_arg.func = DEMUX_TA_ECP_INV_RANGE;
	invoke_arg.session = DEMUX_TA.session;
	invoke_arg.num_params = TEE_NUM_PARAM;

	//PARAM PREPARE
	param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
	if(NULL == param) {
		dmx_err("DEMUX_TA_ECP_inv_range: kcalloc fail\n");
		ret = -EINVAL;
		goto END;
	}
	memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);

	param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[0].u.value.a = pCh->tunerHandle;

	param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[1].u.value.a = pTPRelase;
	param[1].u.value.b = len;

	///////////////////////////////////////////////////////////////////////////////
	rc = tee_client_invoke_func(DEMUX_TA.ctx, &invoke_arg, param);
	if (rc || invoke_arg.ret) {
		dmx_err("optee_meminfo: invoke failed ret %x invoke_arg.ret %x\n", rc, invoke_arg.ret);
		ret = -EINVAL;
		goto END;
	}
	rtk_inv_range(pCh, (void*)((long)(pCh) +sizeof(demux_channel)));
	outer_inv_range(virt_to_phys((void*)pCh), virt_to_phys((void*)pCh)+sizeof(demux_channel));
END:
	if (param){
		kfree(param);
		param = NULL;
	}

	return ret;
}

/**/
int DEMUX_TA_ECP_DeliverPrivateInfo(demux_channel *pCh, int infoId, char* pInfo, unsigned int len, long ibBuf, long bsBuf)
{

	int ret = 0, rc = 0;
	struct tee_ioctl_invoke_arg invoke_arg;
	struct tee_param *param = NULL;
	struct tee_shm *infoData =  NULL;
	void * infoData_va = NULL;

	if(DEMUX_TA.ctx == NULL) {
		dmx_err("optee_meminfo: no ta context\n");
		ret = -EINVAL;
		goto END;
	}
	rtk_flush_range(pCh, (void*)((long)(pCh) +sizeof(demux_channel)));
	outer_flush_range(virt_to_phys((void*)pCh), virt_to_phys((void*)pCh)+sizeof(demux_channel));	

	rtk_flush_range(demux_device, (void*)((long)demux_device + sizeof(demux_dev)) );
	outer_flush_range(virt_to_phys(demux_device), virt_to_phys(demux_device) + sizeof(demux_dev));
	//CALL PREPARE
	memset(&invoke_arg, 0, sizeof(invoke_arg));
	invoke_arg.func = DEMUX_TA_ECP_DELIVER_INBAND_COMMAND;
	invoke_arg.session = DEMUX_TA.session;
	invoke_arg.num_params = TEE_NUM_PARAM;

	//PARAM PREPARE
	param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
	if(NULL == param) {
		dmx_err("DEMUX_TA_ECP_DeliverPrivateInfo: kcalloc fail\n");
		ret = -EINVAL;
		goto END;
	}
	memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);

	param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[0].u.value.a = pCh->tunerHandle;
	param[0].u.value.b = infoId;

	if (len == 0 ||  pInfo == NULL)
	{
		dmx_err("DEMUX_TA_ECP_DeliverPrivateInfo : NO DATA\n");
		param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
	}
	else
	{
		dmx_err("DEMUX_TA_ECP_DeliverPrivateInfo : HAS DATA size:%d\n", len);
		param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
		infoData = tee_shm_alloc(DEMUX_TA.ctx, len, TEE_SHM_MAPPED);
		if (infoData == NULL){
			ret = -ENOMEM;
			goto END;
		}
		infoData_va = tee_shm_get_va(infoData, 0);
		if (infoData_va == NULL) {
			ret = -ENOMEM;
			goto END;
		}
		memcpy(infoData_va, pInfo, len);
		param[1].u.memref.shm = infoData;
		param[1].u.memref.size = len;
	}


	param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[2].u.value.a = ibBuf;
	param[2].u.value.b = bsBuf;


	param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_OUTPUT;
	///////////////////////////////////////////////////////////////////////////////
	rc = tee_client_invoke_func(DEMUX_TA.ctx, &invoke_arg, param);
	if (rc || invoke_arg.ret) {
		dmx_err("optee_meminfo: invoke failed ret %x invoke_arg.ret %x\n", rc, invoke_arg.ret);
		ret = -EINVAL;
		goto END;
	}
	ret = param[3].u.value.a;

	rtk_inv_range(pCh, (void*)((long)(pCh) +sizeof(demux_channel)));
	outer_inv_range(virt_to_phys((void*)pCh), virt_to_phys((void*)pCh)+sizeof(demux_channel));
END:
	if (infoData){
		tee_shm_free(infoData);
		infoData = NULL;
	}
	if (param){
		kfree(param);
		param = NULL;
	}

	return ret;

}
/*
	pin: pin index  v0/a0/v1/a1
	streamType: stream type of current buffer: bs/ib
	bufferTyle: 0:globle buffer, 1: channel specifical buffer
	buffer_Header: offset of RingbufferHeader, 0: means release pinInfo
	buffer: offset of buffer, 0: means release pinInfo;
*/
int DEMUX_TA_ECP_UpdatePininfo(demux_channel *pCh, unsigned int pin, DEMUX_STREAM_TYPE_T streamType, unsigned char bufferType, unsigned long buffer_Header, unsigned long buffer)
{
	int ret = 0, rc = 0;
	struct tee_ioctl_invoke_arg invoke_arg;
	struct tee_param *param = NULL;
	if(DEMUX_TA.ctx == NULL) {
		dmx_err("optee_meminfo: no ta context\n");
		ret = -EINVAL;
		goto END;
	}
	dmx_err("%s_%d: pin:%d, stream:%d buffer_Header=%lx, buffer=%lx\n", __func__, __LINE__, pin, streamType, buffer_Header, buffer);
	rtk_flush_range(pCh, (void*)((long)(pCh) +sizeof(demux_channel)));
	outer_flush_range(virt_to_phys((void*)pCh), virt_to_phys((void*)pCh)+sizeof(demux_channel));	

	rtk_flush_range(demux_device, (void*)((long)demux_device + sizeof(demux_dev)) );
	outer_flush_range(virt_to_phys(demux_device), virt_to_phys(demux_device) + sizeof(demux_dev));
	//CALL PREPARE
	memset(&invoke_arg, 0, sizeof(invoke_arg));
	invoke_arg.func = DEMUX_TA_ECP_UPDATE_PININFO;
	invoke_arg.session = DEMUX_TA.session;
	invoke_arg.num_params = TEE_NUM_PARAM;

	//PARAM PREPARE
	param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
	if(NULL == param) {
		dmx_err("DEMUX_TA_ECP_UpdatePESBufferinfo: kcalloc fail\n");
		ret = -EINVAL;
		goto END;
	}
	memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);

	param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[0].u.value.a = pCh->tunerHandle;
	param[0].u.value.b = pin;

	param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[1].u.value.a = streamType;
	param[1].u.value.b = bufferType;


	param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[2].u.value.a = buffer_Header;
	param[2].u.value.b = buffer;


	param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
	///////////////////////////////////////////////////////////////////////////////
	rc = tee_client_invoke_func(DEMUX_TA.ctx, &invoke_arg, param);
	if (rc || invoke_arg.ret) {
		dmx_err("optee_meminfo: invoke failed ret %x invoke_arg.ret %x\n", rc, invoke_arg.ret);
		ret = -EINVAL;
		goto END;
	}
	rtk_inv_range(pCh, (void*)((long)(pCh) +sizeof(demux_channel)));
	outer_inv_range(virt_to_phys((void*)pCh), virt_to_phys((void*)pCh)+sizeof(demux_channel));
END:
	if (param){
		kfree(param);
		param = NULL;
	}

	return ret;

}

int DEMUX_TA_ECP_UpdatePESBufferinfo(demux_channel *pCh, unsigned int pin, unsigned int isSetup)
{
	int ret = 0, rc = 0;
	struct tee_ioctl_invoke_arg invoke_arg;
	struct tee_param *param = NULL;
	DEMUX_PES_MAP_T *pMap = &pCh->pesMap[pin];
	if(DEMUX_TA.ctx == NULL) {
		dmx_err("optee_meminfo: no ta context\n");
		ret = -EINVAL;
		goto END;
	}
	dmx_err("%s_%d: pin:%d, \n", __func__, __LINE__, pin);
	rtk_flush_range(pCh, (void*)((long)(pCh) +sizeof(demux_channel)));
	outer_flush_range(virt_to_phys((void*)pCh), virt_to_phys((void*)pCh)+sizeof(demux_channel));	
	rtk_flush_range(demux_device, (void*)((long)demux_device + sizeof(demux_dev)) );
	outer_flush_range(virt_to_phys(demux_device), virt_to_phys(demux_device) + sizeof(demux_dev));
	//CALL PREPARE
	memset(&invoke_arg, 0, sizeof(invoke_arg));
	invoke_arg.func = DEMUX_TA_ECP_UPDATE_PES_BUFFERINFO;
	invoke_arg.session = DEMUX_TA.session;
	invoke_arg.num_params = TEE_NUM_PARAM;


	dmx_err("%s_%d: pin:%d, size=%x phy=%x\n", __func__, __LINE__, pin, pMap->size, pMap->phyAddr);

	//PARAM PREPARE
	param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
	if(NULL == param) {
		dmx_err("DEMUX_TA_ECP_UpdatePESBufferinfo: kcalloc fail\n");
		ret = -EINVAL;
		goto END;
	}
	memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);

	param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[0].u.value.a = pCh->tunerHandle;

	param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[1].u.value.a = pin;
	param[1].u.value.b = isSetup;

	param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;


	param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
	///////////////////////////////////////////////////////////////////////////////
	rc = tee_client_invoke_func(DEMUX_TA.ctx, &invoke_arg, param);
	if (rc || invoke_arg.ret) {
		dmx_err("optee_meminfo: invoke failed ret %x invoke_arg.ret %x\n", rc, invoke_arg.ret);
		ret = -EINVAL;
		goto END;
	}
	rtk_inv_range(pCh, (void*)((long)(pCh) +sizeof(demux_channel)));
	outer_inv_range(virt_to_phys((void*)pCh), virt_to_phys((void*)pCh)+sizeof(demux_channel));
END:
	if (param){
		kfree(param);
		param = NULL;
	}

	return ret;

}
int DEMUX_TA_ECP_Parse(demux_channel *pCh, unsigned long demuxIn, unsigned long demuxOut)
{
	int ret = 0, rc = 0;
	struct tee_ioctl_invoke_arg invoke_arg;
	struct tee_param *param = NULL;
	if(DEMUX_TA.ctx == NULL) {
		dmx_err("optee_meminfo: no ta context\n");
		ret = -EINVAL;
		goto END;
	}
	rtk_flush_range(pCh, (void*)((long)(pCh) +sizeof(demux_channel)));
	outer_flush_range(virt_to_phys((void*)pCh), virt_to_phys((void*)pCh)+sizeof(demux_channel));
	rtk_flush_range(demux_device, (void*)((long)demux_device + sizeof(demux_dev)) );
	outer_flush_range(virt_to_phys(demux_device), virt_to_phys(demux_device) + sizeof(demux_dev));
	//dmx_err("%s_%d: demuxin:%ld, demuxOut:%ld \n", __func__, __LINE__, demuxIn, demuxOut);
	//CALL PREPARE
	memset(&invoke_arg, 0, sizeof(invoke_arg));
	invoke_arg.func = DEMUX_TA_ECP_PARSE;
	invoke_arg.session = DEMUX_TA.session;
	invoke_arg.num_params = TEE_NUM_PARAM;

	//PARAM PREPARE
	param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
	if(NULL == param) {
		dmx_err("DEMUX_TA_ECP_Parse: kcalloc fail\n");
		ret = -EINVAL;
		goto END;
	}
	memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
	param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[0].u.value.a = pCh->tunerHandle;

	param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[1].u.value.a = demuxIn; // channel PHYSICAL
	param[1].u.value.b = demuxOut;

	param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_OUTPUT;

	param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

	///////////////////////////////////////////////////////////////////////////////
	rc = tee_client_invoke_func(DEMUX_TA.ctx, &invoke_arg, param);
	if (rc || invoke_arg.ret) {
		dmx_err("optee_meminfo: invoke failed ret %x invoke_arg.ret %x\n", rc, invoke_arg.ret);
		ret = -EINVAL;
		goto END;
	}
	ret = param[2].u.value.a;
	//dmx_info(0, "%s_%d:ret:%d \n", __func__, __LINE__, ret);
	rtk_inv_range(pCh, (void*)((long)(pCh) +sizeof(demux_channel)));
	outer_inv_range(virt_to_phys((void*)pCh), virt_to_phys((void*)pCh)+sizeof(demux_channel));
END:
	if (param){
		kfree(param);
		param = NULL;
	}

	return ret;
}

int DEMUX_TA_ECP_DeliverData(demux_channel *pCh, unsigned long demuxOut, REFCLOCK *pRefClock)
{
	int ret = 0, rc = 0;
	int pRefClock_len;
	struct tee_shm *pRefClock_buf = NULL;
	void * pRefClock_va = NULL;
	struct tee_ioctl_invoke_arg invoke_arg;
	struct tee_param *param = NULL;
	if(DEMUX_TA.ctx == NULL) {
		dmx_err("optee_meminfo: no ta context\n");
		ret = -EINVAL;
		goto END;
	}
	rtk_flush_range(pCh, (void*)((long)(pCh) +sizeof(demux_channel)));
	outer_flush_range(virt_to_phys((void*)pCh), virt_to_phys((void*)pCh)+sizeof(demux_channel));
	rtk_flush_range(demux_device, (void*)((long)demux_device + sizeof(demux_dev)) );
	outer_flush_range(virt_to_phys(demux_device), virt_to_phys(demux_device) + sizeof(demux_dev));
	//dmx_err("%s_%d: demuxOut:%ld \n", __func__, __LINE__, demuxOut);
	//CALL PREPARE
	memset(&invoke_arg, 0, sizeof(invoke_arg));
	invoke_arg.func = DEMUX_TA_ECP_DELIVERDATA;
	invoke_arg.session = DEMUX_TA.session;
	invoke_arg.num_params = TEE_NUM_PARAM;

	//PARAM PREPARE
	param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
	if(NULL == param) {
		dmx_err("DEMUX_TA_ECP_DeliverData: kcalloc fail\n");
		ret = -EINVAL;
		goto END;
	}
	memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);

	param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[0].u.value.a = pCh->tunerHandle;
	param[0].u.value.b = demuxOut;

	param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_OUTPUT;

	param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
	pRefClock_len = sizeof(REFCLOCK);
	pRefClock_buf = tee_shm_alloc(DEMUX_TA.ctx, pRefClock_len, TEE_SHM_MAPPED);
	if (pRefClock_buf == NULL){
		ret = -ENOMEM;
		goto END;
	}
	pRefClock_va = tee_shm_get_va(pRefClock_buf, 0);
	if (pRefClock_va == NULL) {
		ret = -ENOMEM;
		goto END;
	}
	memcpy(pRefClock_va, pRefClock, pRefClock_len);
	param[2].u.memref.shm = pRefClock_buf;
	param[2].u.memref.size = pRefClock_len;

	param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

	rc = tee_client_invoke_func(DEMUX_TA.ctx, &invoke_arg, param);
	if (rc || invoke_arg.ret) {
		dmx_err("optee_meminfo: invoke failed ret %x invoke_arg.ret %x\n", rc, invoke_arg.ret);
		ret = -EINVAL;
		goto END;
	}
	ret = param[1].u.value.a;
	//dmx_info(0, "%s_%d:ret:%d \n", __func__, __LINE__, ret);
	rtk_inv_range(pCh, (void*)((long)(pCh) +sizeof(demux_channel)));
	outer_inv_range(virt_to_phys((void*)pCh), virt_to_phys((void*)pCh)+sizeof(demux_channel));
END:
	if (pRefClock_buf){
		tee_shm_free(pRefClock_buf);
		pRefClock_buf = NULL;
	}
	if (param)
		kfree(param);

	return ret;
}

//==================================================================================================	

int DEMUX_TA_ECP_INIT(demux_channel *pCh){
	if (Demux_ta_Init()<0)
	{
		dmx_err("%s_%d: init DEMUX TA FAIL\n",__func__, __LINE__);
	}
	///
	if (pCh == NULL)
		return 0;
	dmx_err("%s_%d: tpaddrssoffset:%ld\n", __func__, __LINE__, pCh->phyAddrOffset);
	rtk_flush_range(pCh, (void*)((long)(pCh) +sizeof(demux_channel)));
	outer_flush_range(virt_to_phys((void*)pCh), virt_to_phys((void*)pCh)+sizeof(demux_channel));
	rtk_flush_range(demux_device, (void*)((long)demux_device + sizeof(demux_dev)) );
	outer_flush_range(virt_to_phys(demux_device), virt_to_phys(demux_device) + sizeof(demux_dev));

	if (Demux_ta_Construct(pCh) < 0){
		dmx_err("%s_%d: Demux_ta_Construct FAIL\n",__func__, __LINE__);
	}

	rtk_inv_range(pCh, (void*)((long)(pCh) +sizeof(demux_channel)));
	outer_inv_range(virt_to_phys((void*)pCh), virt_to_phys((void*)pCh)+sizeof(demux_channel));
	dmx_err("%s_%d: tpaddrssoffset:%ld\n", __func__, __LINE__, pCh->phyAddrOffset);

	return 0;
}

int DEMUX_TA_ECP_UNINIT(demux_channel *pCh){
	if (pCh != NULL){
		rtk_flush_range(pCh, (void*)((long)(pCh) +sizeof(demux_channel)));
		outer_flush_range(virt_to_phys((void*)pCh), virt_to_phys((void*)pCh)+sizeof(demux_channel));
		rtk_flush_range(demux_device, (void*)((long)demux_device + sizeof(demux_dev)) );
		outer_flush_range(virt_to_phys(demux_device), virt_to_phys(demux_device) + sizeof(demux_dev));
		dmx_err("%s_%d: tpaddrssoffset:%ld\n", __func__, __LINE__, pCh->phyAddrOffset);
		if (Demux_ta_Deconstruct(pCh) <0){
			dmx_err("%s_%d: Demux_ta_Deconstruct FAIL\n",__func__, __LINE__);

		}
		dmx_err("%s_%d: tpaddrssoffset:%ld\n", __func__, __LINE__, pCh->phyAddrOffset);
		rtk_inv_range(pCh, (void*)((long)(pCh) +sizeof(demux_channel)));
		outer_inv_range(virt_to_phys((void*)pCh), virt_to_phys((void*)pCh)+sizeof(demux_channel));
	}
	Demux_ta_Deinit();
	return 0;
}
