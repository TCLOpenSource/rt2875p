#include <linux/pageremap.h>
#include <rtk_kdriver/rtkaudio_debug.h>
#include "audio_rpc.h"
#include "audio_inc.h"
#include "hresult.h"

#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
#include <rtk_kdriver/RPCDriver.h>
#endif

#ifdef CONFIG_ARM64
#define ALIGN4 (0xFFFFFFFFFFFFFFFCLL)
#else
#define ALIGN4 (0xFFFFFFFC)
#endif

#define dummy_htons(_X_) htons(_X_)
#define dummy_ntohl(_X_) ntohl(_X_)
#define dummy_ntohs(_X_) ntohs(_X_)
#define dummy_htonl(_X_) htonl(_X_)

#define UINT32 unsigned int

struct mutex rpc_mem_usage;
struct rpc_mem_info mem_info[RPC_MEM_NUM];

void init_rpc_mem(void)
{
	int i;

	mutex_init(&rpc_mem_usage);
	memset(mem_info, 0, sizeof(struct rpc_mem_info) * RPC_MEM_NUM);

	for (i = 0;i < RPC_MEM_NUM; i++) {
		mem_info[i].vir_addr = dvr_malloc_uncached_specific(RPC_DVR_MALLOC_UNCACHED_SIZE, GFP_DCU1, &mem_info[i].unvir_addr);
		rtd_pr_adsp_info("get_rpc_mem%d, vir %lx, unvir_addr %lx\n", i, mem_info[i].vir_addr, mem_info[i].unvir_addr);
	}
}

void *get_rpc_mem(void **unvir_addr)
{
	int i;
	void *vir_addr = NULL;

	mutex_lock(&rpc_mem_usage);
	for (i = 0;i < RPC_MEM_NUM; i++) {
		if (!mem_info[i].used) {
			*unvir_addr = mem_info[i].unvir_addr;
			mem_info[i].used = 1;

			//rtd_pr_adsp_info("[rtkaudio] get_rpc_mem vir %lx\n", mem_info[i].vir_addr);
			mutex_unlock(&rpc_mem_usage);
			return mem_info[i].vir_addr;
		}
	}
	mutex_unlock(&rpc_mem_usage);

	vir_addr = dvr_malloc_uncached_specific(RPC_DVR_MALLOC_UNCACHED_SIZE, GFP_DCU1, unvir_addr);
	/* rtd_pr_adsp_info("[rtkaudio] get_rpc_mem from dvr vir %lx, Pool %lx(%d), %lx(%d), %lx(%d), %lx(%d)\n",
		vir_addr, mem_info[0].vir_addr, mem_info[1].vir_addr, mem_info[2].vir_addr, mem_info[3].vir_addr,
		mem_info[0].used, mem_info[1].used, mem_info[2].used, mem_info[3].used); */
	return vir_addr;
}

void put_rpc_mem(void *vir_addr)
{
	int i;

	mutex_lock(&rpc_mem_usage);
	for (i = 0;i < RPC_MEM_NUM; i++) {
		if (mem_info[i].vir_addr == vir_addr) {
			mem_info[i].used = 0;

			//rtd_pr_adsp_info("[rtkaudio] put_rpc_mem vir %lx\n", mem_info[i].vir_addr);
			mutex_unlock(&rpc_mem_usage);
			return;
		}
	}
	mutex_unlock(&rpc_mem_usage);

	//rtd_pr_adsp_info("[rtkaudio] put_rpc_mem dvr_free vir %lx\n", vir_addr);
	dvr_free(vir_addr);
}

static void rpc_dvr_malloc_uncached(unsigned long *vir_addr, unsigned long *phy_addr, void **info)
{
    *vir_addr = (ulong) get_rpc_mem(info);

    if ((void*)*vir_addr == NULL) {
        ERROR("[%s %d]alloc memory fail\n", __FUNCTION__, __LINE__);
        /*return -ENOMEM;*/
    }

    *phy_addr = (unsigned long)dvr_to_phys((void*)*vir_addr);
    if ((void*)*phy_addr == NULL) {
        ERROR("[%s %d]alloc memory fail\n", __FUNCTION__, __LINE__);
        if (*vir_addr) {
            put_rpc_mem((void*)*vir_addr);
        }
        /*return -ENOMEM;*/
    }
}

UINT32 RTKAUDIO_RPC_TOAGENT_INIT_RING_BUFFER_HEADER_SVC(AUDIO_RPC_RINGBUFFER_HEADER *header)
{
	AUDIO_RPC_RINGBUFFER_HEADER *info, *info_audio;
	HRESULT *res, *res_audio;
	unsigned long vir_addr = 0;
	unsigned long phy_addr = 0;
	unsigned long ret;
	int i;

	rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_RINGBUFFER_HEADER *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_RINGBUFFER_HEADER) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_RINGBUFFER_HEADER) + 8) & ALIGN4));

	info->instanceID  = htonl(header->instanceID);
	info->listSize = htonl(header->listSize);
	info->pinID  = htonl(header->pinID);
	info->readIdx = htonl(header->readIdx);
	for (i = 0 ; i < sizeof(header->pRingBufferHeaderList) / sizeof(header->pRingBufferHeaderList[0]); i++) {
		info->pRingBufferHeaderList[i] = htonl(header->pRingBufferHeaderList[i]);
	}

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_INIT_RINGBUF,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
	    ERROR("[%s] %d RPC fail %ld\n", __FUNCTION__, __LINE__, ret);
	}

	if (ret != S_OK)
	{
	    ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
	}

	put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_SET_PP_TSXT_CONFIG_SVC(AUDIO_RPC_TSXT *tsxt)
{
	AUDIO_RPC_TSXT *info, *info_audio;
	HRESULT *res, *res_audio;
	unsigned long vir_addr = 0;
	unsigned long phy_addr = 0;
	unsigned long ret;

	rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_TSXT *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_TSXT) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_TSXT) + 8) & ALIGN4));

	info->instanceID  = htonl(tsxt->instanceID);
	info->pp_Tsxt_Enable = htonl(tsxt->pp_Tsxt_Enable);
	info->ModeType = htonl(tsxt->ModeType);
	info->TruBassGain = htonl(tsxt->TruBassGain);
	info->TruBassSize = htonl(tsxt->TruBassSize);
	info->FocusGain = htonl(tsxt->FocusGain);
	info->InputGain = htonl(tsxt->InputGain);
	info->IsTSXTEnable = htonl(tsxt->IsTSXTEnable);
	info->IsHeadphoneMode = htonl(tsxt->IsHeadphoneMode);
	info->IsTruBassEnable = htonl(tsxt->IsTruBassEnable);
	info->IsFocusEnable = htonl(tsxt->IsFocusEnable);
	info->DefinitionGain = htonl(tsxt->DefinitionGain);
	info->IsDefEnable = htonl(tsxt->IsDefEnable);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_SET_PP_TSXT_CONFIG,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
	    ERROR("[%s] %d RPC fail %ld\n", __FUNCTION__, __LINE__, ret);
	}

	if (ret != S_OK)
	{
	    ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
	}

	put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_SET_CHANNEL_OUT_SWAP_SVC(AUDIO_RPC_AO_CHANNEL_OPT_SWAP *cswap)
{
	AUDIO_RPC_AO_CHANNEL_OPT_SWAP *info, *info_audio;
	HRESULT *res, *res_audio;
	unsigned long vir_addr = 0;
	unsigned long phy_addr = 0;
	unsigned long ret;

	rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_AO_CHANNEL_OPT_SWAP *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_AO_CHANNEL_OPT_SWAP) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_AO_CHANNEL_OPT_SWAP) + 8) & ALIGN4));

	info->instanceID  = htonl(cswap->instanceID);
	info->channelSwapIndex = htonl(cswap->channelSwapIndex);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_SET_CHANNEL_OUT_SWAP,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
	    ERROR("[%s] %d RPC fail %ld\n", __FUNCTION__, __LINE__, ret);
	}

	if (ret != S_OK)
	{
	    ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
	}

	put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_SET_SEND_SPECTRUM_DATA_SVC(AUDIO_SPECTRUM_CFG *cfg)
{
	AUDIO_SPECTRUM_CFG *info, *info_audio;
	HRESULT *res, *res_audio;
	unsigned long vir_addr = 0;
	unsigned long phy_addr = 0;
	unsigned long ret;

	rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_SPECTRUM_CFG *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_SPECTRUM_CFG) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_SPECTRUM_CFG) + 8) & ALIGN4));

	info->bandnum  = htonl(cfg->bandnum);
	info->data_addr = htonl(cfg->data_addr);
	info->enable  = htonl(cfg->enable);
	info->upper_bandlimit_addr = htonl(cfg->upper_bandlimit_addr);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_SET_SEND_SPECTRUM_DATA,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
	    ERROR("[%s] %d RPC fail %ld\n", __FUNCTION__, __LINE__, ret);
	}

	if (ret != S_OK)
	{
	    ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
	}

	put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_SET_AUTHORITY_KEY_SVC(long *customer_key)
{
	long *info, *info_audio;
	HRESULT *res, *res_audio;
	unsigned long vir_addr = 0;
	unsigned long phy_addr = 0;
	unsigned long ret;

	rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(long *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(long) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(long) + 8) & ALIGN4));

	*info  = htonl(*customer_key);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_SET_AUTHORITY_KEY,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
	    ERROR("[%s] %d RPC fail %ld\n", __FUNCTION__, __LINE__, ret);
	}

	if (ret != S_OK)
	{
	    ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
	}

	put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_SET_MODEL_HASH_SVC(AUDIO_DUMMY_DATA *hash)
{
	AUDIO_DUMMY_DATA *info, *info_audio;
	HRESULT *res, *res_audio;
	unsigned long vir_addr = 0;
	unsigned long phy_addr = 0;
	unsigned long ret;
	int i;

	rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_DUMMY_DATA *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_DUMMY_DATA) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_DUMMY_DATA) + 8) & ALIGN4));

	for(i=0;i<8;i++)
	{
		info->parameter[i] =  htonl(hash->parameter[i]);
	}

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_SET_MODEL_HASH,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
	    ERROR("[%s] %d RPC fail %ld\n", __FUNCTION__, __LINE__, ret);
	}

	if (ret != S_OK)
	{
	    ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
	}

	put_rpc_mem((void *) vir_addr);
	return ret;
}
UINT32 RTKAUDIO_RPC_TOAGENT_SET_AUDIO_CAPABILITY_SVC(AUDIO_LICENSE_STATUS_CONFIG *lcns)
{
	AUDIO_LICENSE_STATUS_CONFIG *info, *info_audio;
	HRESULT *res, *res_audio;
	unsigned long vir_addr = 0;
	unsigned long phy_addr = 0;
	unsigned long ret;
	int i;

	rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_LICENSE_STATUS_CONFIG *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_LICENSE_STATUS_CONFIG) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_LICENSE_STATUS_CONFIG) + 8) & ALIGN4));

	info->HighWord  = htonl(lcns->HighWord);

	for(i=0;i<4;i++)
	{
		info->LowWord[i] =  htonl(lcns->LowWord[i]);
	}

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_SET_AUDIO_CAPABILITY,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
	    ERROR("[%s] %d RPC fail %ld\n", __FUNCTION__, __LINE__, ret);
	}

	if (ret != S_OK)
	{
	    ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
	}

	put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_SET_AVSYNC_OFFSET_SVC(AUDIO_HDMI_OUT_VSDB_DATA *data)
{
	AUDIO_HDMI_OUT_VSDB_DATA *info, *info_audio;
	HRESULT *res, *res_audio;
	unsigned long vir_addr = 0;
	unsigned long phy_addr = 0;
	unsigned long ret;

	rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_HDMI_OUT_VSDB_DATA *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_HDMI_OUT_VSDB_DATA) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_HDMI_OUT_VSDB_DATA) + 8) & ALIGN4));

	info->HDMI_VSDB_delay  = htonl(data->HDMI_VSDB_delay);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_SET_AVSYNC_OFFSET,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
	    ERROR("[%s] %d RPC fail %ld\n", __FUNCTION__, __LINE__, ret);
	}

	if (ret != S_OK)
	{
	    ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
	}

	put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_SET_SPDIF_OUT_PIN_SRC_SVC(AUDIO_RPC_SPDIFO_SOURCE *source)
{
	AUDIO_RPC_SPDIFO_SOURCE *info, *info_audio;
	HRESULT *res, *res_audio;
	unsigned long vir_addr = 0;
	unsigned long phy_addr = 0;
	unsigned long ret;

	rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_SPDIFO_SOURCE *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_SPDIFO_SOURCE) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_SPDIFO_SOURCE) + 8) & ALIGN4));

	info->instanceID = htonl(source->instanceID);
	info->source = htonl(source->source);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_SET_SPDIF_OUT_PIN_SRC,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
	    ERROR("[%s] %d RPC fail %ld\n", __FUNCTION__, __LINE__, ret);
	}

	if (ret != S_OK)
	{
	    ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
	}

	put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_SET_TRUVOLUME_SVC(AUDIO_RPC_TRUVOLUME *vol)
{
	AUDIO_RPC_TRUVOLUME *info, *info_audio;
	HRESULT *res, *res_audio;
	unsigned long vir_addr = 0;
	unsigned long phy_addr = 0;
	unsigned long ret;

	rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_TRUVOLUME *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_TRUVOLUME) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_TRUVOLUME) + 8) & ALIGN4));

	info->instanceID = htonl(vol->instanceID);
	info->pp_TruVol_Enable = htonl(vol->pp_TruVol_Enable);
	info->pp_TruVol_Info.BlockSize =  htonl(vol->pp_TruVol_Info.BlockSize);
	info->pp_TruVol_Info.BypassGain =  htonl(vol->pp_TruVol_Info.BypassGain);
	info->pp_TruVol_Info.enable =  htonl(vol->pp_TruVol_Info.enable);
	info->pp_TruVol_Info.InputGain =  htonl(vol->pp_TruVol_Info.InputGain);
	info->pp_TruVol_Info.MaxGain =  htonl(vol->pp_TruVol_Info.MaxGain);
	info->pp_TruVol_Info.mode =	htonl(vol->pp_TruVol_Info.mode);
	info->pp_TruVol_Info.NormalizerEnable =	htonl(vol->pp_TruVol_Info.NormalizerEnable);
	info->pp_TruVol_Info.OutputGain =  htonl(vol->pp_TruVol_Info.OutputGain);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_SET_TRUVOLUME,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
	    ERROR("[%s] %d RPC fail %ld\n", __FUNCTION__, __LINE__, ret);
	}

	if (ret != S_OK)
	{
	    ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
	}

	put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_SET_ATV_CLOCK_SVC(AUDIO_RPC_SET_ATV_CLOCK *atv_clk_info)
{
	AUDIO_RPC_SET_ATV_CLOCK *info, *info_audio;
	HRESULT *res, *res_audio;
	unsigned long vir_addr = 0;
	unsigned long phy_addr = 0;
	unsigned long ret;

	rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_SET_ATV_CLOCK *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_SET_ATV_CLOCK) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_SET_ATV_CLOCK) + 8) & ALIGN4));

	info->instanceID  = htonl(atv_clk_info->instanceID);
	info->atv_type = htonl(atv_clk_info->atv_type);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_AIN_ATV_SET_CLOCK,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
	    ERROR("[%s] %d RPC fail %ld\n", __FUNCTION__, __LINE__, ret);
	}

	if (ret != S_OK)
	{
	    ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
	}

	put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_SET_SIF_ADC_INIT_SVC(AUDIO_RPC_SET_SIF_ADC_INIT *set_info)
{
	AUDIO_RPC_SET_SIF_ADC_INIT *info, *info_audio;
	HRESULT *res, *res_audio;
	unsigned long vir_addr = 0;
	unsigned long phy_addr = 0;
	unsigned long ret = S_OK;

	rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_SET_SIF_ADC_INIT *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_SET_SIF_ADC_INIT) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_SET_SIF_ADC_INIT) + 8) & ALIGN4));

	info->instanceID  = htonl(set_info->instanceID);
	info->sif_adc_init_enable = htonl(set_info->sif_adc_init_enable);
#if 0 //TODO, wait next stage ready.
	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_SET_SIF_ADC_INIT,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
	    ERROR("[%s] %d RPC fail %ld\n", __FUNCTION__, __LINE__, ret);
	}
#endif
	if (ret != S_OK)
	{
	    ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
	}

	put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_INITRINGBUFFER_HEADER_SVC(AUDIO_RPC_RINGBUFFER_HEADER *header)
{
	AUDIO_RPC_RINGBUFFER_HEADER *info, *info_audio;
	HRESULT *res, *res_audio;
	unsigned long vir_addr = 0;
	unsigned long phy_addr = 0;
	unsigned long ret;
	int i;

	rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_RINGBUFFER_HEADER *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_RINGBUFFER_HEADER) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_RINGBUFFER_HEADER) + 8) & ALIGN4));

	info->instanceID  = htonl(header->instanceID);
	info->listSize = htonl(header->listSize);
	info->pinID  = htonl(header->pinID);
	info->readIdx = htonl(header->readIdx);
	for (i = 0 ; i < sizeof(header->pRingBufferHeaderList) / sizeof(header->pRingBufferHeaderList[0]); i++) {
		info->pRingBufferHeaderList[i] = htonl(header->pRingBufferHeaderList[i]);
	}

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_INIT_RINGBUF,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
	    ERROR("[%s] %d RPC fail %ld\n", __FUNCTION__, __LINE__, ret);
	}

	if (ret != S_OK)
	{
	    ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
	}

	put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_CONNECT_SVC(AUDIO_RPC_CONNECTION *pconnection)
{
    struct AUDIO_RPC_CONNECTION *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

    info_audio = (void*)(struct AUDIO_RPC_CONNECTION *)(phy_addr);

    res = (void*)(HRESULT*)
        ((((unsigned long)info + sizeof(struct AUDIO_RPC_CONNECTION) + 8) & ALIGN4));
    res_audio = (void*)(HRESULT *)
        ((((phy_addr) + sizeof(struct AUDIO_RPC_CONNECTION) + 8) & ALIGN4));

	info->srcInstanceID = htonl(pconnection->srcInstanceID);
	info->srcPinID = htonl(pconnection->srcPinID);
	info->desInstanceID = htonl(pconnection->desInstanceID);
	info->desPinID = htonl(pconnection->desPinID);

    if (send_rpc_command(RPC_AUDIO,
        ENUM_KERNEL_RPC_CONNECT,
        (unsigned long) info_audio,
        (unsigned long) res_audio, &ret))
    {
        ERROR("[%s] %d RPC fail %ld\n", __FUNCTION__, __LINE__, ret);
        ret = -EFAULT;
    }

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_DISCONNECT_SVC(AUDIO_RPC_CONNECTION *pconnection)
{
    struct AUDIO_RPC_CONNECTION *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

    info_audio = (void*)(struct AUDIO_RPC_CONNECTION *)(phy_addr);

    res = (void*)(HRESULT*)
        ((((unsigned long)info + sizeof(struct AUDIO_RPC_CONNECTION) + 8) & ALIGN4));
    res_audio = (void*)(HRESULT *)
        ((((phy_addr) + sizeof(struct AUDIO_RPC_CONNECTION) + 8) & ALIGN4));

	info->srcInstanceID = htonl(pconnection->srcInstanceID);
	info->srcPinID = htonl(pconnection->srcPinID);
	info->desInstanceID = htonl(pconnection->desInstanceID);
	info->desPinID = htonl(pconnection->desPinID);

    if (send_rpc_command(RPC_AUDIO,
        ENUM_KERNEL_RPC_DISCONNECT,
        (unsigned long) info_audio,
        (unsigned long) res_audio, &ret))
    {
        ERROR("[%s] %d RPC fail %ld\n", __FUNCTION__, __LINE__, ret);
        ret = -EFAULT;
    }

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_SETREFCLOCK_SVC(AUDIO_RPC_REFCLOCK *rpc_refclock)
{
    AUDIO_RPC_REFCLOCK *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;
    unsigned long ret;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_REFCLOCK *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_REFCLOCK) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_REFCLOCK) + 8) & ALIGN4));

    info->instanceID  = htonl(rpc_refclock->instanceID);
    info->pRefClockID = htonl(rpc_refclock->pRefClockID);
    info->pRefClock   = htonl(rpc_refclock->pRefClock);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_SETREFCLOCK,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
        ERROR("[%s] %d RPC fail %ld\n", __FUNCTION__, __LINE__, ret);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_FLUSH_SVC(AUDIO_RPC_SENDIO *sendio)
{
    struct AUDIO_RPC_SENDIO *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;
    unsigned long ret;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(struct AUDIO_RPC_SENDIO *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(struct AUDIO_RPC_SENDIO) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(struct AUDIO_RPC_SENDIO) + 8) & ALIGN4));

	DEBUG("[%s] %ld %d\n", __FUNCTION__, sendio->instanceID, sendio->pinID);

	info->instanceID = htonl(sendio->instanceID);
	info->pinID = htonl(sendio->pinID);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_FLUSH,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
        ERROR("[%s] %d RPC fail %ld\n", __FUNCTION__, __LINE__, ret);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_STOP_SVC(UINT32 *inst_id)
{
    UINT32 *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;
    unsigned long ret;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(UINT32 *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(UINT32) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(UINT32) + 8) & ALIGN4));

	*info = htonl(*inst_id);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_STOP,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
        ERROR("[%s] %d RPC fail %ld\n", __FUNCTION__, __LINE__, ret);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_PAUSE_SVC(UINT32 *inst_id)
{
    UINT32 *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;
    unsigned long ret;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(UINT32 *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(UINT32) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(UINT32) + 8) & ALIGN4));

	*info = htonl(*inst_id);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_PAUSE,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
        ERROR("[%s] %d RPC fail %ld\n", __FUNCTION__, __LINE__, ret);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_RUN_SVC(UINT32 *inst_id)
{
    UINT32 *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;
    unsigned long ret;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(UINT32 *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(UINT32) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(UINT32) + 8) & ALIGN4));

	*info = htonl(*inst_id);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_RUN,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
        ERROR("[%s] %d RPC fail %ld\n", __FUNCTION__, __LINE__, ret);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_DESTROY_SVC(UINT32 *inst_id)
{
    UINT32 *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(UINT32 *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(UINT32) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(UINT32) + 8) & ALIGN4));

	*info = htonl(*inst_id);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_DESTROY,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;

}

UINT32 RTKAUDIO_RPC_CREATE_AGENT_SVC(AUDIO_RPC_INSTANCE *instance, RPCRES_LONG *inst_result)
{
    AUDIO_RPC_INSTANCE *info, *info_audio;
    RPCRES_LONG *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

    info_audio = (void *)(struct AUDIO_RPC_INSTANCE *)phy_addr;
    res = (void *)(struct RPCRES_LONG *)
        ((((unsigned long)info + sizeof(struct AUDIO_RPC_INSTANCE) + 8) & ALIGN4));
    res_audio = (void *)(struct RPCRES_LONG *)
        ((((phy_addr) + sizeof(struct AUDIO_RPC_INSTANCE) + 8) & ALIGN4));

	/* create agent */
	info->instanceID = htonl(instance->instanceID);
	info->type       = htonl(instance->type);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_CREATE_AGENT,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    inst_result->data   = (long)ntohl(res->data);
    inst_result->result = (long)ntohl(res->result);
    DEBUG("Create instance %x\n", (UINT32)ntohl(res->data));

    put_rpc_mem((void *) vir_addr);
    return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_SET_INTERLEAVE_OUT_SVC(AUDIO_RPC_SET_INTERLEAVE_OUT* pInfo)
{
    AUDIO_RPC_SET_INTERLEAVE_OUT *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

    info_audio = (void *)(struct AUDIO_RPC_SET_INTERLEAVE_OUT *)phy_addr;
    res = (void *)(struct HRESULT *)
        ((((unsigned long)info + sizeof(struct AUDIO_RPC_SET_INTERLEAVE_OUT) + 8) & ALIGN4));
    res_audio = (void *)(struct HRESULT *)
        ((((phy_addr) + sizeof(struct AUDIO_RPC_SET_INTERLEAVE_OUT) + 8) & ALIGN4));

	info->instanceID = htonl(pInfo->instanceID);
	info->enable_interleave_out_pb = htonl(pInfo->enable_interleave_out_pb);
	info->enable_interleave_out_ts = htonl(pInfo->enable_interleave_out_ts);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_SET_INTERLEAVE_OUT,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;

}

UINT32 RTKAUDIO_RPC_TOAGENT_BBADC_CONFIG_SVC(AUDIO_RPC_BBADC_CONFIG *pRPC_BBADC_cfg)
{
    AUDIO_RPC_BBADC_CONFIG *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_BBADC_CONFIG *)( phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_BBADC_CONFIG) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_BBADC_CONFIG) + 8) & ALIGN4));

	info->instanceID = htonl(pRPC_BBADC_cfg->instanceID);
	info->config.audioGeneralConfig.interface_en       = htons(pRPC_BBADC_cfg->config.audioGeneralConfig.interface_en);
	info->config.audioGeneralConfig.channel_in         = htons(pRPC_BBADC_cfg->config.audioGeneralConfig.channel_in);
	info->config.audioGeneralConfig.count_down_rec_en  = htons(pRPC_BBADC_cfg->config.audioGeneralConfig.count_down_rec_en);
	info->config.audioGeneralConfig.count_down_rec_cyc = htonl(pRPC_BBADC_cfg->config.audioGeneralConfig.count_down_rec_cyc);
	info->config.sampleInfo.sampling_rate = htonl(pRPC_BBADC_cfg->config.sampleInfo.sampling_rate);
	info->config.sampleInfo.PCM_bitnum    = htonl(pRPC_BBADC_cfg->config.sampleInfo.PCM_bitnum);
	info->config.clk_sel = htonl(pRPC_BBADC_cfg->config.clk_sel);
	info->config.src_sel  = htonl(pRPC_BBADC_cfg->config.src_sel);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_BBADC_CONFIG,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;

}

UINT32 RTKAUDIO_RPC_TOAGENT_I2SI_CONFIG_SVC(AUDIO_RPC_I2SI_CONFIG *pRPC_I2S_cfg)
{
    AUDIO_RPC_I2SI_CONFIG *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_I2SI_CONFIG *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_I2SI_CONFIG) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_I2SI_CONFIG) + 8) & ALIGN4));

	info->instanceID = htonl(pRPC_I2S_cfg->instanceID);
	info->config.audioGeneralConfig.interface_en       = htons(pRPC_I2S_cfg->config.audioGeneralConfig.interface_en);
	info->config.audioGeneralConfig.channel_in         = htons(pRPC_I2S_cfg->config.audioGeneralConfig.channel_in);
	info->config.audioGeneralConfig.count_down_rec_en  = htons(pRPC_I2S_cfg->config.audioGeneralConfig.count_down_rec_en);
	info->config.audioGeneralConfig.count_down_rec_cyc = htonl(pRPC_I2S_cfg->config.audioGeneralConfig.count_down_rec_cyc);
	info->config.id        = htons(pRPC_I2S_cfg->config.id);
	info->config.src_sel   = htons(pRPC_I2S_cfg->config.src_sel);
	info->config.sync_type = htons(pRPC_I2S_cfg->config.sync_type);
	info->config.mode_sel  = htons(pRPC_I2S_cfg->config.mode_sel);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_I2SI_CONFIG,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;

}

UINT32 RTKAUDIO_RPC_TOAGENT_SPDIFI_CONFIG_SVC(AUDIO_RPC_SPDIFI_CONFIG *pRPC_SPDIF_cfg)
{
    AUDIO_RPC_SPDIFI_CONFIG *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_SPDIFI_CONFIG *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_SPDIFI_CONFIG) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_SPDIFI_CONFIG) + 8) & ALIGN4));

	info->instanceID = htonl(pRPC_SPDIF_cfg->instanceID);
	info->config.audioGeneralConfig.interface_en       = htons(pRPC_SPDIF_cfg->config.audioGeneralConfig.interface_en);
	info->config.audioGeneralConfig.channel_in         = htons(pRPC_SPDIF_cfg->config.audioGeneralConfig.channel_in);
	info->config.audioGeneralConfig.count_down_rec_en  = htons(pRPC_SPDIF_cfg->config.audioGeneralConfig.count_down_rec_en);
	info->config.audioGeneralConfig.count_down_rec_cyc = htonl(pRPC_SPDIF_cfg->config.audioGeneralConfig.count_down_rec_cyc);
	info->config.src_sel        = htons(pRPC_SPDIF_cfg->config.src_sel);
	info->config.req_after_lock = htons(pRPC_SPDIF_cfg->config.req_after_lock);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_SPDIFI_CONFIG,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;

}

UINT32 RTKAUDIO_RPC_TOAGENT_CHG_IPT_SRC_SVC(AUDIO_RPC_IPT_SRC *pSource)
{
    AUDIO_RPC_IPT_SRC *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_IPT_SRC *)(phy_addr);

	res = (void *)(struct HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_IPT_SRC) + 8) & ALIGN4));
	res_audio = (void *)(struct HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_IPT_SRC) + 8) & ALIGN4));

	info->instanceID = htonl(pSource->instanceID);
    info->focus_in[0] = htonl(pSource->focus_in[0]);    // CH1&CH2
    info->focus_in[1] = htonl(pSource->focus_in[1]);    // CH3&CH4
    info->focus_in[2] = htonl(pSource->focus_in[2]);    // CH5&CH6
    info->focus_in[3] = htonl(pSource->focus_in[3]);    // CH7&CH8
    info->mux_in      = htonl(pSource->mux_in);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_CHG_IPT_SRC,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;

}

UINT32 RTKAUDIO_RPC_TOAGENT_GET_FORMAT_SVC(UINT32 *inst_id, AUDIO_RPC_AIN_FORMAT_INFO *formatInfo)
{
    UINT32 *info, *info_audio;
    AUDIO_RPC_AIN_FORMAT_INFO *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;
    int i;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(UINT32 *)(phy_addr);

	res = (void *)(AUDIO_RPC_AIN_FORMAT_INFO *)
		((((unsigned long)info + sizeof(UINT32) + 8) & ALIGN4));
	res_audio = (void *)(struct AUDIO_RPC_AIN_FORMAT_INFO *)
		((((phy_addr) + sizeof(UINT32) + 8) & ALIGN4));

	*info = htonl(*inst_id);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_GET_AI_FORMAT,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    formatInfo->type = ntohl(res->type);
    formatInfo->nSamplesPerSec = ntohl(res->nSamplesPerSec);
    for(i = 0; i < 4; i++){
        formatInfo->reserved[i] = ntohl(res->reserved[i]);
    }

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;

}

UINT32 RTKAUDIO_RPC_TOAGENT_PP_INITPIN_SVC(UINT32 *inst_id)
{
    UINT32 *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(UINT32 *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(UINT32) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(UINT32) + 8) & ALIGN4));

	*info = htonl(*inst_id);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_PP_INITPIN,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_SUBCHANNEL_SVC(AUDIO_RPC_SUBCHANNEL *sub_ch)
{
    AUDIO_RPC_SUBCHANNEL *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_SUBCHANNEL *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_SUBCHANNEL) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_SUBCHANNEL) + 8) &	ALIGN4));

	info->instanceID      = htonl(sub_ch->instanceID);
	info->SubChannelPinID = htonl(sub_ch->SubChannelPinID);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_SUBCHANNEL,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_SETSEEKING_SVC(AUDIO_RPC_SEEK *seek)
{
    AUDIO_RPC_SEEK *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_SEEK *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_SEEK) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_SEEK) + 8) & ALIGN4));

	info->instanceID = htonl(seek->instanceID);
	info->skip       = htonl(seek->skip);
	info->speed      = htonl(seek->speed);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_SET_SEEKING,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_SWITCH_FOCUS_SVC(AUDIO_RPC_FOCUS *focus)
{
    AUDIO_RPC_FOCUS *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_FOCUS *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_FOCUS) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_FOCUS) + 8) & ALIGN4));

	info->instanceID = htonl(focus->instanceID);
	info->focusID    = htonl(focus->focusID);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_SWITCH_FOCUS,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

static unsigned long long reverseLongLong(unsigned long long value)
{
    unsigned long long ret;
    unsigned char *des, *src;
    src = (unsigned char *)&value;
    des = (unsigned char *)&ret;
    des[0] = src[7];
    des[1] = src[6];
    des[2] = src[5];
    des[3] = src[4];
    des[4] = src[3];
    des[5] = src[2];
    des[6] = src[1];
    des[7] = src[0];
    return ret;
}
UINT32 RTKAUDIO_RPC_TOAGENT_PTS_MIXER_CONFIG(AUDIO_RPC_PTS_MIXER_CONFIG *pts_mixer)
{
    AUDIO_RPC_PTS_MIXER_CONFIG *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;
    int i;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_PTS_MIXER_CONFIG *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_PTS_MIXER_CONFIG) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_PTS_MIXER_CONFIG) + 8) & ALIGN4));

	info->instanceID = htonl(pts_mixer->instanceID);
	info->config.pinID     = htonl(pts_mixer->config.pinID);
	info->config.mixer_in_change_PTS = (long long)reverseLongLong((unsigned long long)pts_mixer->config.mixer_in_change_PTS);
	info->config.mixer_in_ena = htonl(pts_mixer->config.mixer_in_ena);
	for (i = 0; i < 8; i++) {
		info->config.mixer_gain[i] = htonl(pts_mixer->config.mixer_gain[i]);
	}

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_PTS_MIXER_CONFIG,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

	if (ret != S_OK)
	{
		ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
	}

	put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_DAC_I2S_CONFIG_SVC(AUDIO_CONFIG_DAC_I2S *dac_i2s_config)
{
    AUDIO_CONFIG_DAC_I2S *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_CONFIG_DAC_I2S *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_CONFIG_DAC_I2S) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_CONFIG_DAC_I2S) + 8) &	ALIGN4));

	info->instanceID = htonl(dac_i2s_config->instanceID);
	info->dacConfig.audioGeneralConfig.interface_en        = htons(dac_i2s_config->dacConfig.audioGeneralConfig.interface_en);
	info->dacConfig.audioGeneralConfig.channel_out         = htons(dac_i2s_config->dacConfig.audioGeneralConfig.channel_out);
	info->dacConfig.audioGeneralConfig.count_down_play_en  = htons(dac_i2s_config->dacConfig.audioGeneralConfig.count_down_play_en);
	info->dacConfig.audioGeneralConfig.count_down_play_cyc = htonl(dac_i2s_config->dacConfig.audioGeneralConfig.count_down_play_cyc);
	info->dacConfig.sampleInfo.sampling_rate = htonl(dac_i2s_config->dacConfig.sampleInfo.sampling_rate);
	info->dacConfig.sampleInfo.PCM_bitnum    = htonl(dac_i2s_config->dacConfig.sampleInfo.PCM_bitnum);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_DAC_I2S_CONFIG,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_DAC_SPDIF_CONFIG_SVC(AUDIO_CONFIG_DAC_SPDIF *dac_spdif_config)
{
    AUDIO_CONFIG_DAC_SPDIF *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_CONFIG_DAC_SPDIF *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_CONFIG_DAC_SPDIF) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_CONFIG_DAC_SPDIF) + 8) & ALIGN4));

	info->instanceID  = htonl(dac_spdif_config->instanceID);
	info->spdifConfig.audioGeneralConfig.interface_en        = htons(dac_spdif_config->spdifConfig.audioGeneralConfig.interface_en);
	info->spdifConfig.audioGeneralConfig.channel_out         = htons(dac_spdif_config->spdifConfig.audioGeneralConfig.channel_out);
	info->spdifConfig.audioGeneralConfig.count_down_play_en  = htons(dac_spdif_config->spdifConfig.audioGeneralConfig.count_down_play_en);
	info->spdifConfig.audioGeneralConfig.count_down_play_cyc = htonl(dac_spdif_config->spdifConfig.audioGeneralConfig.count_down_play_cyc);
	info->spdifConfig.sampleInfo.sampling_rate       = htonl(dac_spdif_config->spdifConfig.sampleInfo.sampling_rate);
	info->spdifConfig.sampleInfo.PCM_bitnum          = htonl(dac_spdif_config->spdifConfig.sampleInfo.PCM_bitnum);
	info->spdifConfig.out_cs_info.non_pcm_valid      = htons(dac_spdif_config->spdifConfig.out_cs_info.non_pcm_valid);
	info->spdifConfig.out_cs_info.non_pcm_format     = htons(dac_spdif_config->spdifConfig.out_cs_info.non_pcm_format);
	info->spdifConfig.out_cs_info.audio_format       = htonl(dac_spdif_config->spdifConfig.out_cs_info.audio_format);
	info->spdifConfig.out_cs_info.spdif_consumer_use = htons(dac_spdif_config->spdifConfig.out_cs_info.spdif_consumer_use);
	info->spdifConfig.out_cs_info.copy_right         = htons(dac_spdif_config->spdifConfig.out_cs_info.copy_right);
	info->spdifConfig.out_cs_info.pre_emphasis       = htons(dac_spdif_config->spdifConfig.out_cs_info.pre_emphasis);
	info->spdifConfig.out_cs_info.stereo_channel     = htons(dac_spdif_config->spdifConfig.out_cs_info.stereo_channel);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_DAC_SPDIF_CONFIG,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_PRIVATEINFO_SVC(AUDIO_RPC_PRIVATEINFO_PARAMETERS *parameter, AUDIO_RPC_PRIVATEINFO_RETURNVAL *result)
{
    AUDIO_RPC_PRIVATEINFO_PARAMETERS *info, *info_audio;
    AUDIO_RPC_PRIVATEINFO_RETURNVAL *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;
    int i;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_PRIVATEINFO_PARAMETERS *)(phy_addr);

	res = (void *)(struct AUDIO_RPC_PRIVATEINFO_RETURNVAL *)
		((((unsigned long)info + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS) + 8) & ALIGN4));
	res_audio = (void *)(struct AUDIO_RPC_PRIVATEINFO_RETURNVAL *)
		((((phy_addr) + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS) + 8) & ALIGN4));

	info->instanceID = htonl(parameter->instanceID);
	info->type       = htonl(parameter->type);
    for(i = 0; i < 16; i++){
        info->privateInfo[i] = htonl(parameter->privateInfo[i]);
    }

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_PRIVATEINFO,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

	result->instanceID = ntohl(res->instanceID);
    for(i = 0; i < 16; i++){
        result->privateInfo[i] = ntohl(res->privateInfo[i]);
    }

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_DEC_TOAGENT_SETDUALMONOOUTMODE_SVC(AUDIO_RPC_CHANNEL_OUT_MODE *mode_info)
{
    AUDIO_RPC_CHANNEL_OUT_MODE *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_CHANNEL_OUT_MODE *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_CHANNEL_OUT_MODE) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_CHANNEL_OUT_MODE) + 8) & ALIGN4));

	info->instanceID = htonl(mode_info->instanceID);
	info->mode       = htonl(mode_info->mode);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_SET_DUALMONO_OUTMODE,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_DEC_TOAGENT_GETAUDIOFORMATINFO_SVC(UINT32 *inst_id, AUDIO_RPC_DEC_FORMAT_INFO *foramt_info)
{
    UINT32 *info, *info_audio;
    AUDIO_RPC_DEC_FORMAT_INFO *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;
    int i;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(UINT32 *)(phy_addr);

	res = (void *)(struct AUDIO_RPC_DEC_FORMAT_INFO *)
		((((unsigned long)info + sizeof(UINT32) + 8) & ALIGN4));
	res_audio = (void *)(struct AUDIO_RPC_DEC_FORMAT_INFO *)
		((((phy_addr) + sizeof(UINT32) + 8) & ALIGN4));

	*info = htonl(*inst_id);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_GET_AUDIO_FORMATINFO,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    foramt_info->type            = ntohl(res->type);
    foramt_info->nChannels       = ntohs(res->nChannels);
    foramt_info->wBitsPerSample  = ntohs(res->wBitsPerSample);
    foramt_info->nSamplesPerSec  = ntohl(res->nSamplesPerSec);
    foramt_info->nAvgBytesPerSec = ntohl(res->nAvgBytesPerSec);
    for(i = 0; i < 4; i++){
        foramt_info->reserved[i] = ntohl(res->reserved[i]);
    }

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_AO_TOAGENT_GETTRSENCAUDIOFORMATINFO_SVC(UINT32 *inst_id, AUDIO_RPC_AO_TRSENC_FORMAT_INFO *foramt_info)
{
    UINT32 *info, *info_audio;
    AUDIO_RPC_AO_TRSENC_FORMAT_INFO *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;
    int i;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

    info_audio = (void *)(UINT32 *)(phy_addr);

    res = (void *)(struct AUDIO_RPC_AO_TRSENC_FORMAT_INFO *)
        ((((unsigned long)info + sizeof(UINT32) + 8) & ALIGN4));
    res_audio = (void *)(struct AUDIO_RPC_AO_TRSENC_FORMAT_INFO *)
        ((((phy_addr) + sizeof(UINT32) + 8) & ALIGN4));

    *info = htonl(*inst_id);

    if (send_rpc_command(RPC_AUDIO,
        ENUM_KERNEL_RPC_GET_AO_TRSENC_AUDIO_FORMATINFO,
        (unsigned long) info_audio,
        (unsigned long) res_audio,
        &ret)) {
        ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
    }

    foramt_info->type            = ntohl(res->type);
    foramt_info->nChannels       = ntohl(res->nChannels);
    for(i = 0; i < 4; i++){
        foramt_info->reserved[i] = ntohl(res->reserved[i]);
    }

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
    return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_DECODERCONFIG_SVC(AUDIO_RPC_DECCFG* dec_info)
{
    AUDIO_RPC_DECCFG *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_DECCFG *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_DECCFG) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_DECCFG) + 8) &	ALIGN4));

	info->instanceID         = htonl(dec_info->instanceID);
	info->cfg.scalehi        = htonl(dec_info->cfg.scalehi);
	info->cfg.scalelo        = htonl(dec_info->cfg.scalelo);
	info->cfg.scalehi_2ch    = htonl(dec_info->cfg.scalehi_2ch);
	info->cfg.scalelo_2ch    = htonl(dec_info->cfg.scalelo_2ch);
	info->cfg.mode           = htons(dec_info->cfg.mode);
	info->cfg.LFEMode        = htons(dec_info->cfg.LFEMode);
	info->cfg.dualmono       = htons(dec_info->cfg.dualmono);
	info->cfg.comprMode      = htons(dec_info->cfg.comprMode);
	info->cfg.comprMode_2ch  = htons(dec_info->cfg.comprMode_2ch);
	info->cfg.stereoMode     = htons(dec_info->cfg.stereoMode);
	info->cfg.sub_dec_mode   = htons(dec_info->cfg.sub_dec_mode);
	info->cfg.substream_id   = htons(dec_info->cfg.substream_id);
	info->cfg.dialnorm       = htons(dec_info->cfg.dialnorm);
	info->cfg.prog_ref_level = htons(dec_info->cfg.prog_ref_level);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_DECODER_CONFIG,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_AO_TOAGENT_RESAMPLETABLE_SVC(AUDIO_RPC_RESAMPLE_INFO *aResampleInfo)
{
    AUDIO_RPC_RESAMPLE_INFO *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_RESAMPLE_INFO *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_RESAMPLE_INFO) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_RESAMPLE_INFO) + 8) & ALIGN4));

	info->Taps       = htonl(aResampleInfo->Taps);
	info->SrcSamples = htonl(aResampleInfo->SrcSamples);
	info->DstSamples = htonl(aResampleInfo->DstSamples);
	info->Coefs      = htonl(aResampleInfo->Coefs);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_RESAMPLETABLE,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_AO_TOAGENT_SPDIFO_SOURCE_CONFIG_SVC(AUDIO_RPC_SPDIFO_SOURCE *aSpdifSrc)
{
    AUDIO_RPC_SPDIFO_SOURCE *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_SPDIFO_SOURCE *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_SPDIFO_SOURCE) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_SPDIFO_SOURCE) + 8) & ALIGN4));

	info->instanceID = htonl(aSpdifSrc->instanceID);
	info->source     = htonl(aSpdifSrc->source);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_SPDIFO_CONFIG,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_DEBUG_SVC(char *para, UINT32 len)
{
    char *param, *param_audio;
    UINT32 *param2, *param2_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&param);

	param_audio = (void *)(char *)(phy_addr);

	param2 = (void *)(UINT32 *)
		((((unsigned long)param + len + 8) & ALIGN4));
	param2_audio = (void *)(UINT32 *)
		((((phy_addr) + len + 8) & ALIGN4));

    memset(param, '\0', len+1);
    memcpy(param, para, len);
    *param2 = htonl(len);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_DEBUG_COMMAND,
		(unsigned long) param_audio,
		(unsigned long) param2_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_GET_SPDIF_CS_SVC(AUDIO_SPDIF_CS *spdif_cs)
{
    AUDIO_SPDIF_CS *info, *info_audio;
    AUDIO_SPDIF_CS *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_SPDIF_CS *)(phy_addr);

	res = (void *)(AUDIO_SPDIF_CS *)
		((((unsigned long)info + sizeof(AUDIO_SPDIF_CS) + 8) & ALIGN4));
	res_audio = (void *)(AUDIO_SPDIF_CS *)
		((((phy_addr) + sizeof(AUDIO_SPDIF_CS) + 8) & ALIGN4));

	info->module_type = htonl(spdif_cs->module_type);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_GET_SPDIF_CS,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    spdif_cs->professional           = ntohs(res->professional);
    spdif_cs->data_type              = ntohs(res->data_type);
    spdif_cs->copyright              = ntohs(res->copyright);
    spdif_cs->pre_emphasis           = ntohs(res->pre_emphasis);
    spdif_cs->mode                   = ntohs(res->mode);
    spdif_cs->category_code          = ntohs(res->category_code);
    spdif_cs->source_number          = ntohs(res->source_number);
    spdif_cs->channel_number         = ntohs(res->channel_number);
    spdif_cs->sampling_freq          = ntohs(res->sampling_freq);
    spdif_cs->clock_accuracy         = ntohs(res->clock_accuracy);
    spdif_cs->word_length            = ntohs(res->word_length);
    spdif_cs->original_sampling_freq = ntohs(res->original_sampling_freq);
    spdif_cs->cgms_a                 = ntohs(res->cgms_a);

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}
UINT32 RTKAUDIO_RPC_TOAGENT_AO_LGSE_SETMODE_SVC(AUDIO_RPC_LGSE_SETMODE *lgse_config)
{
    AUDIO_RPC_LGSE_SETMODE *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_LGSE_SETMODE *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_LGSE_SETMODE) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_LGSE_SETMODE) + 8) & ALIGN4));

	info->instanceID = htonl(lgse_config->instanceID);
	info->dataOption = htons(lgse_config->dataOption);
	info->flag       = htonl(lgse_config->flag);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_LGSE_SETMODE,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;

}

UINT32 RTKAUDIO_RPC_TOAGENT_AO_LGSE_SETMAIN_SVC(AUDIO_RPC_LGSE_SETMAIN *lgse_config)
{
    AUDIO_RPC_LGSE_SETMAIN *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_LGSE_SETMODE *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_LGSE_SETMAIN) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_LGSE_SETMAIN) + 8) & ALIGN4));

	info->instanceID = htonl(lgse_config->instanceID);
	info->dataOption = htons(lgse_config->dataOption);
	info->dataInit   = lgse_config->dataInit;
	info->dataVar    = lgse_config->dataVar;

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_LGSE_SETMAIN,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_AO_LGSE_SETFN000_SVC(AUDIO_RPC_LGSE_SETFN000 *lgse000)
{
    AUDIO_RPC_LGSE_SETFN000 *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_LGSE_SETFN000 *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_LGSE_SETFN000) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_LGSE_SETFN000) + 8) & ALIGN4));

	info->instanceID = htonl(lgse000->instanceID);
	info->dataOption = htons(lgse000->dataOption);
	info->dataVar    = lgse000->dataVar;
	info->rpc_LGSE00855_set_address = htonl(lgse000->rpc_LGSE00855_set_address);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_LGSE_SETFN000,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;

}

UINT32 RTKAUDIO_RPC_TOAGENT_AO_LGSE_SETFN001_SVC(AUDIO_RPC_LGSE_SETFN001 *lgse001)
{
    AUDIO_RPC_LGSE_SETFN001 *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_LGSE_SETFN001 *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_LGSE_SETFN001) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_LGSE_SETFN001) + 8) & ALIGN4));

	info->instanceID = htonl(lgse001->instanceID);
	info->dataOption = htons(lgse001->dataOption);
	info->dataInit   = lgse001->dataInit;
	info->rpc_LGSE00869_set_address = htonl(lgse001->rpc_LGSE00869_set_address);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_LGSE_SETFN001,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;

}

UINT32 RTKAUDIO_RPC_TOAGENT_AO_LGSE_SETFN004_SVC(AUDIO_RPC_LGSE_SETFN004 *lgse004)
{
    AUDIO_RPC_LGSE_SETFN004 *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;
    UINT32 i;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_LGSE_SETFN004 *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_LGSE_SETFN004) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_LGSE_SETFN004) + 8) & ALIGN4));

	info->instanceID = htonl(lgse004->instanceID);
	info->dataOption = htons(lgse004->dataOption);
	for (i = 0; i < 25; i++) {
        info->data[i] = htonl(lgse004->data[i]);
	}

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_LGSE_SETFN004,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;

}

UINT32 RTKAUDIO_RPC_TOAGENT_AO_LGSE_SETFN005_SVC(AUDIO_RPC_LGSE_SETFN005 *lgse005)
{
    AUDIO_RPC_LGSE_SETFN005 *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;
    UINT32 i;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_LGSE_SETFN005 *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_LGSE_SETFN005) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_LGSE_SETFN005) + 8) & ALIGN4));

	info->instanceID = htonl(lgse005->instanceID);
	info->dataOption = htons(lgse005->dataOption);
	for (i = 0; i < 15; i++) {
        info->data[i] = htonl(lgse005->data[i]);
	}

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_LGSE_SETFN005,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_AO_LGSE_SETFN008_SVC(AUDIO_RPC_LGSE_SETFN008 *lgse008)
{
    AUDIO_RPC_LGSE_SETFN008 *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_LGSE_SETFN008 *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_LGSE_SETFN008) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_LGSE_SETFN008) + 8) & ALIGN4));

	info->instanceID = htonl(lgse008->instanceID);
	info->dataOption = htons(lgse008->dataOption);
	info->dataVar    = lgse008->dataVar;
	info->rpc_LGSE00858_set_address = htonl(lgse008->rpc_LGSE00858_set_address);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_LGSE_SETFN008,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_AO_LGSE_SETFN009_SVC(AUDIO_RPC_LGSE_SETFN009 *lgse009)
{
    AUDIO_RPC_LGSE_SETFN009 *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_LGSE_SETFN009 *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_LGSE_SETFN009) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_LGSE_SETFN009) + 8) & ALIGN4));

	info->instanceID = htonl(lgse009->instanceID);
	info->dataOption = htons(lgse009->dataOption);
	info->dataVar    = lgse009->dataVar;
	info->dataInit   = lgse009->dataInit;

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_LGSE_SETFN009,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_AO_LGSE_SETFN010_SVC(AUDIO_RPC_LGSE_SETFN010 *lgse010, AUDIO_RPC_LGSE_RETURNVAL *lgse_ret)
{
    AUDIO_RPC_LGSE_SETFN010  *info, *info_audio;
    AUDIO_RPC_LGSE_RETURNVAL *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;
    UINT32 i;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_LGSE_SETFN010 *)(phy_addr);

	res = (void *)(AUDIO_RPC_LGSE_RETURNVAL *)
		((((unsigned long)info + sizeof(AUDIO_RPC_LGSE_SETFN010) + 8) & ALIGN4));
	res_audio = (void *)(AUDIO_RPC_LGSE_RETURNVAL *)
		((((phy_addr) + sizeof(AUDIO_RPC_LGSE_SETFN010) + 8) & ALIGN4));

	info->instanceID = htonl(lgse010->instanceID);
	info->dataOption = htons(lgse010->dataOption);
	info->dataVar    = lgse010->dataVar;
	info->dataInit   = lgse010->dataInit;

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_LGSE_SETFN010,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

	lgse_ret->ret = ntohl(res->ret);
    for(i = 0; i < 16; i++){
        lgse_ret->value[i] = ntohl(res->value[i]);
    }

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_AO_LGSE_SETFN014_SVC(AUDIO_RPC_LGSE_SETFN014 *lgse014)
{
    AUDIO_RPC_LGSE_SETFN014 *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_LGSE_SETFN014 *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_LGSE_SETFN014) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_LGSE_SETFN014) + 8) & ALIGN4));

	info->instanceID = htonl(lgse014->instanceID);
	info->dataOption = htons(lgse014->dataOption);
	info->dataVar    = lgse014->dataVar;
	info->rpc_LGSE00853_set_address = htonl(lgse014->rpc_LGSE00853_set_address);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_LGSE_SETFN014,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_AO_LGSE_SETFN016_SVC(AUDIO_RPC_LGSE_SETFN016 *lgse016)
{
    AUDIO_RPC_LGSE_SETFN016 *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_LGSE_SETFN016 *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_LGSE_SETFN016) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_LGSE_SETFN016) + 8) & ALIGN4));

	info->instanceID = htonl(lgse016->instanceID);
	info->dataOption = htons(lgse016->dataOption);
	info->rpc_LGSE03520_set_address = htonl(lgse016->rpc_LGSE03520_set_address);
	info->rpc_LGSE03521_set_address = htonl(lgse016->rpc_LGSE03521_set_address);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_LGSE_SETFN016,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_AO_LGSE_SETFN024_SVC(AUDIO_RPC_LGSE_SETFN024 *lgse024)
{
    AUDIO_RPC_LGSE_SETFN024 *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_LGSE_SETFN024 *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_LGSE_SETFN024) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_LGSE_SETFN024) + 8) & ALIGN4));

	info->instanceID = htonl(lgse024->instanceID);
	info->dataOption = htons(lgse024->dataOption);
	info->dataInit   = lgse024->dataInit;

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_LGSE_SETFN024,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}
UINT32 RTKAUDIO_RPC_TOAGENT_AO_LGSE_SETFN017_SVC(AUDIO_RPC_LGSE_SETFN017 *lgse017)
{
    AUDIO_RPC_LGSE_SETFN017 *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_LGSE_SETFN017 *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_LGSE_SETFN017) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_LGSE_SETFN017) + 8) & ALIGN4));

	info->instanceID = htonl(lgse017->instanceID);
	info->dataOption = htons(lgse017->dataOption);
	info->rpc_LGSE00879_set_address = htonl(lgse017->rpc_LGSE00879_set_address);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_LGSE_SETFN017,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_AO_LGSE_SETFN019_SVC(AUDIO_RPC_LGSE_SETFN019 *lgse019)
{
    AUDIO_RPC_LGSE_SETFN019 *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_LGSE_SETFN019 *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_LGSE_SETFN019) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_LGSE_SETFN019) + 8) & ALIGN4));

	info->instanceID = htonl(lgse019->instanceID);
	info->dataOption = htons(lgse019->dataOption);
	info->dataVar    = lgse019->dataVar;
	info->rpc_LGSE00856_set_address = htonl(lgse019->rpc_LGSE00856_set_address);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_LGSE_SETFN019,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_AO_LGSE_SETFN022_SVC(AUDIO_RPC_LGSE_SETFN022 *lgse022)
{
    AUDIO_RPC_LGSE_SETFN022 *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_LGSE_SETFN022 *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_LGSE_SETFN022) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_LGSE_SETFN022) + 8) & ALIGN4));

	info->instanceID = htonl(lgse022->instanceID);
	info->dataOption = htons(lgse022->dataOption);
	info->dataInit   = lgse022->dataInit;
	info->rpc_LGSE00878_set_address = htonl(lgse022->rpc_LGSE00878_set_address);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_LGSE_SETFN022,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_AO_LGSE_SETFN026_SVC(AUDIO_RPC_LGSE_SETFN026 *lgse026)
{
    AUDIO_RPC_LGSE_SETFN026 *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_LGSE_SETFN026 *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_LGSE_SETFN026) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_LGSE_SETFN026) + 8) & ALIGN4));

	info->instanceID = htonl(lgse026->instanceID);
	info->dataOption = htons(lgse026->dataOption);
	info->rpc_data026_set_address = htonl(lgse026->rpc_data026_set_address);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_LGSE_SETFN026,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_AO_LGSE_SETFN028_SVC(AUDIO_RPC_LGSE_SETFN028 *lgse028)
{
    AUDIO_RPC_LGSE_SETFN028 *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_LGSE_SETFN028 *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_LGSE_SETFN028) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_LGSE_SETFN028) + 8) & ALIGN4));

	info->instanceID = htonl(lgse028->instanceID);
	info->dataOption = htons(lgse028->dataOption);
	info->dataVar    = lgse028->dataVar;

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_LGSE_SETFN028,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_AO_LGSE_SETFN029_SVC(AUDIO_RPC_LGSE_SETFN029 *lgse029)
{
    AUDIO_RPC_LGSE_SETFN029 *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_LGSE_SETFN029 *)(phy_addr);

	res = (void *)(HRESULT *)
		((((unsigned long)info + sizeof(AUDIO_RPC_LGSE_SETFN029) + 8) & ALIGN4));
	res_audio = (void *)(HRESULT *)
		((((phy_addr) + sizeof(AUDIO_RPC_LGSE_SETFN029) + 8) & ALIGN4));

	info->instanceID = htonl(lgse029->instanceID);
	info->dataOption = htonl(lgse029->dataOption);
	info->rpc_LGSE02869_set_address = htonl(lgse029->rpc_LGSE02869_set_address);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_LGSE_SETFN029,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_AO_LGSE_GETDATA_SVC(AUDIO_RPC_LGSE_GETDATA *lgse_getdata, AUDIO_RPC_LGSE_RETURNVAL *lgse_ret)
{
    AUDIO_RPC_LGSE_GETDATA *info, *info_audio;
    AUDIO_RPC_LGSE_RETURNVAL *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;
    UINT32 i;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

	info_audio = (void *)(AUDIO_RPC_LGSE_GETDATA *)(phy_addr);

	res = (void *)(AUDIO_RPC_LGSE_RETURNVAL *)
		((((unsigned long)info + sizeof(AUDIO_RPC_LGSE_GETDATA) + 8) & ALIGN4));
	res_audio = (void *)(AUDIO_RPC_LGSE_RETURNVAL *)
		((((phy_addr) + sizeof(AUDIO_RPC_LGSE_GETDATA) + 8) & ALIGN4));

	info->instanceID    = htonl(lgse_getdata->instanceID);
	info->function_list = htonl(lgse_getdata->function_list);
	info->rw            = htonl(lgse_getdata->rw);
	info->size          = htonl(lgse_getdata->size);

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_LGSE_GETDATA,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}

	lgse_ret->ret = ntohl(res->ret);
    for(i = 0; i < 16; i++){
        lgse_ret->value[i] = ntohl(res->value[i]);
    }

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_TOAGENT_VX_PRIVATEINFO_SVC(virtualx_cmd_info *cmd_info)
{
    AUDIO_RPC_PRIVATEINFO_PARAMETERS *info, *info_audio;
    AUDIO_RPC_PRIVATEINFO_RETURNVAL *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;
    int i;

    virtualx_cmd_info *virx_cmd = cmd_info;
    int *pSetting;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

    info_audio = (void*)(struct AUDIO_RPC_PRIVATEINFO_PARAMETERS *)(phy_addr);

    res = (void*)(struct AUDIO_RPC_PRIVATEINFO_RETURNVAL *)
        ((((unsigned long)info + sizeof(struct AUDIO_RPC_PRIVATEINFO_PARAMETERS)+sizeof(virtualx_cmd_info) + 8) & ALIGN4));
    res_audio = (void*)(struct AUDIO_RPC_PRIVATEINFO_RETURNVAL *)
        (((phy_addr + sizeof(struct AUDIO_RPC_PRIVATEINFO_PARAMETERS)+sizeof(virtualx_cmd_info) + 8) & ALIGN4));

    info->instanceID = htonl(virx_cmd->index);
    info->type = htonl(ENUM_PRIVATEINFO_AUDIO_CONFIG_VIRTUALX_PARAM);
    info->privateInfo[0] = htonl(virx_cmd->type);
    info->privateInfo[1] = htonl(virx_cmd->size);

    pSetting = &info->privateInfo[2];
    //memcpy(pSetting, &virx_cmd.data[0], virx_cmd.size);
    for (i = 0; i < virx_cmd->size/sizeof(unsigned int); i++)
    {
        pSetting[i] = htonl(virx_cmd->data[i]);
    }

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_PRIVATEINFO,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		&ret)) {
		ERROR("[%s] %d RPC fail\n", __FUNCTION__, __LINE__);
	}


    virx_cmd->result= ntohl(res->privateInfo[0]);

    pSetting = &res->privateInfo[2];
    for (i = 0; i < virx_cmd->size/sizeof(unsigned int); i++)
    {
        virx_cmd->data[i]= ntohl(pSetting[i]);
    }

    if (ret != S_OK)
    {
        ERROR("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_ENC_SETENCODER_SVC(AUDIO_RPC_ENC_INFO *enc_info)
{
    AUDIO_RPC_ENC_INFO *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

    info_audio = (void*)(struct AUDIO_RPC_ENC_INFO*)(phy_addr);

    res = (void*)(HRESULT*)
        ((((unsigned long)info + sizeof(struct AUDIO_RPC_ENC_INFO) + 8) & ALIGN4));
    res_audio = (void*)(HRESULT *)
        ((((phy_addr) + sizeof(struct AUDIO_RPC_ENC_INFO) + 8) & ALIGN4));

	info->instanceID  = htonl(enc_info->instanceID);
	info->encode_type = htonl(enc_info->encode_type);

    if (send_rpc_command(RPC_AUDIO,
        ENUM_KERNEL_RPC_SETENCODER,
        (unsigned long) info_audio,
        (unsigned long) res_audio, &ret))
    {
        rtd_pr_adsp_err("[%s] %d RPC fail %ld\n", __FUNCTION__, __LINE__, ret);
        ret = -EFAULT;
    }

    if (ret != S_OK)
    {
        rtd_pr_adsp_err("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_ENC_TOAGENT_STARTENCODER_SVC(AUDIO_RPC_SEND_LONG *mLong)
{
    AUDIO_RPC_SEND_LONG *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

    info_audio = (void*)(struct AUDIO_RPC_SEND_LONG*)(phy_addr);

    res = (void*)(HRESULT*)
        ((((unsigned long)info + sizeof(struct AUDIO_RPC_SEND_LONG) + 8) & ALIGN4));
    res_audio = (void*)(HRESULT *)
        ((((phy_addr) + sizeof(struct AUDIO_RPC_SEND_LONG) + 8) & ALIGN4));

	info->instanceID = htonl(mLong->instanceID);
	info->data       = htonl(mLong->data);

    if (send_rpc_command(RPC_AUDIO,
        ENUM_KERNEL_RPC_STARTENCODER,
        (unsigned long) info_audio,
        (unsigned long) res_audio, &ret))
    {
        rtd_pr_adsp_err("[%s] %d RPC fail %ld\n", __FUNCTION__, __LINE__, ret);
        ret = -EFAULT;
    }

    if (ret != S_OK)
    {
        rtd_pr_adsp_err("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_ENC_TOAGENT_STOPENCODER_SVC(AUDIO_RPC_SEND_LONG *mLong)
{
    AUDIO_RPC_SEND_LONG *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

    info_audio = (void*)(struct AUDIO_RPC_SEND_LONG*)(phy_addr);

    res = (void*)(HRESULT*)
        ((((unsigned long)info + sizeof(struct AUDIO_RPC_SEND_LONG) + 8) & ALIGN4));
    res_audio = (void*)(HRESULT *)
        ((((phy_addr) + sizeof(struct AUDIO_RPC_SEND_LONG) + 8) & ALIGN4));

	info->instanceID = htonl(mLong->instanceID);
	info->data       = htonl(mLong->data);

    if (send_rpc_command(RPC_AUDIO,
        ENUM_KERNEL_RPC_STOPENCODER,
        (unsigned long) info_audio,
        (unsigned long) res_audio, &ret))
    {
        rtd_pr_adsp_err("[%s] %d RPC fail %ld\n", __FUNCTION__, __LINE__, ret);
        ret = -EFAULT;
    }

    if (ret != S_OK)
    {
        rtd_pr_adsp_err("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}

UINT32 RTKAUDIO_RPC_ENC_TOAGENT_COMMAND_SVC(AUDIO_RPC_ENC_COMMAND *pCommand)
{
    AUDIO_RPC_ENC_COMMAND *info, *info_audio;
    HRESULT *res, *res_audio;
    unsigned long ret;
    unsigned long vir_addr = 0;
    unsigned long phy_addr = 0;

    rpc_dvr_malloc_uncached(&vir_addr, &phy_addr,(void**)&info);

    info_audio = (void*)(struct AUDIO_RPC_SEND_LONG*)(phy_addr);

    res = (void*)(HRESULT*)
        ((((unsigned long)info + sizeof(struct AUDIO_RPC_SEND_LONG) + 8) & ALIGN4));
    res_audio = (void*)(HRESULT *)
        ((((phy_addr) + sizeof(struct AUDIO_RPC_SEND_LONG) + 8) & ALIGN4));

	info->instanceID            = htonl(pCommand->instanceID);
    info->enc_config.inputmode  = htonl(pCommand->enc_config.inputmode);
    info->enc_config.outputmode = htonl(pCommand->enc_config.outputmode);
    info->enc_config.DRC1       = htonl(pCommand->enc_config.DRC1);
    info->enc_config.DRC2       = htonl(pCommand->enc_config.DRC2);
    info->enc_config.datarate   = htonl(pCommand->enc_config.datarate);
    info->enc_config.samprate   = htonl(pCommand->enc_config.samprate);
    info->enc_config.LorR       = htonl(pCommand->enc_config.LorR);

    if (send_rpc_command(RPC_AUDIO,
        ENUM_KERNEL_RPC_STOPENCODER,
        (unsigned long) info_audio,
        (unsigned long) res_audio, &ret))
    {
        rtd_pr_adsp_err("[%s] %d RPC fail %ld\n", __FUNCTION__, __LINE__, ret);
        ret = -EFAULT;
    }

    if (ret != S_OK)
    {
        rtd_pr_adsp_err("[%s] %d RPC is not S_OK %ld\n", __FUNCTION__, __LINE__, ret);
    }

    put_rpc_mem((void *) vir_addr);
	return ret;
}
