#include <linux/types.h>
#include <rtd_log/rtd_module_log.h>
#include <ioctrl/vpq/vpq_cmd_id.h>
#include <vgip_isr/scalerAI.h>
#if defined(CONFIG_RTK_KDRV_VDEC_MODULE) 
#define AI_INCLUDE_PREFETCH_BARRIER
#include <../../vdec/rtd6748/rtk_vdec_svp.c>
#else
#include <rtk_vdec_svp.h>
#endif

#define ROUND_UP(x, y) (((x) < y) ? y : (((x) & (y-1)) == 0 ? (x) : (((x) + (y-1))&(~(y-1)))))

typedef struct {
	u32 phy_addr;
	u32 size;
	u8* vir_addr;
} CMA_BUF_INFO_T;

typedef struct {
	void* ptr;
	u32 size;
} MAP_INFO_T;

static CMA_BUF_INFO_T video_heap;
static MAP_INFO_T nn_noncached_mem;
static MAP_INFO_T nn_cached_mem;

extern VIP_NN_BUFFER_LEN vip_nn_buffer_len[VIP_NN_BUFFER_MAX];

static bool get_nn_ta_buf(CMA_BUF_INFO_T *cma, NN_BUF_INFO_T *info)
{
	if (cma->size > 0) {
		if (cma->phy_addr == 0)
			return 0;

		info->pa = cma->phy_addr;
		info->size = cma->size;
		return 1;

	}

	cma->vir_addr = (u8*)dvr_malloc_specific(info->size, GFP_DCU2_FIRST);
	if (cma->vir_addr == 0) {
		rtd_pr_vpq_ai_emerg("[%s %d] out of memory: size=%d\n", __func__, __LINE__, info->size);
		return 0;
	}

	cma->phy_addr = dvr_to_phys(cma->vir_addr);
	cma->size = info->size;
	info->pa = cma->phy_addr;
	return 1;

}

static bool release_nn_ta_buf(CMA_BUF_INFO_T *cma)
{
	if (cma->vir_addr)
		dvr_free(cma->vir_addr);

	memset(cma, 0, sizeof(CMA_BUF_INFO_T));
	return 1;
}

bool ai_optee_get_nn_ta_buf(NN_BUF_INFO_T *info)
{
	VIP_NN_CTRL *nn_ctrl;
	VIP_NN_MEMORY_CTRL *mem;

	switch(info->buf_id)
	{
	case VIP_VIDEO_HEAP:
		return get_nn_ta_buf(&video_heap, info);
	default:
		break;
	}

	nn_ctrl = scalerAI_Access_NN_CTRL_STRUCT();
	if (info->net_id >= VIP_NN_BUFFER_MAX)
		return 0;

	switch(info->buf_id)
	{
	case NET_DATA_IN:
		mem = &nn_ctrl->NN_data_Addr[info->net_id];
		break;
	case NET_INFO:
		mem = &nn_ctrl->NN_info_Addr[info->net_id];
		break;
	case NET_FLAG:
		mem = &nn_ctrl->NN_flag_Addr[info->net_id];
		break;
	default:
		return 0;
	}

	info->pa = mem->phy_addr_align;
	info->size = mem->size;
	return 1;
}

bool ai_optee_release_nn_ta_buf(NN_BUF_INFO_T *info)
{
	switch(info->buf_id)
	{
	case VIP_VIDEO_HEAP:
		return release_nn_ta_buf(&video_heap);
	default:
		return 1;
	}
}
void ai_optee_release_all_ta_buf(void)
{
	release_nn_ta_buf(&video_heap);

}

static u32 get_nn_buf_total_size(u32* flag_len, u32* info_len, u32* data_len)
{
	u32 i;
	u32 flag_size = 0, info_size = 0, data_size = 0;

	for (i=0; i < VIP_NN_BUFFER_NUM; i++) {
		data_size += ROUND_UP(vip_nn_buffer_len[i].data_len, 64);
		flag_size += ROUND_UP(vip_nn_buffer_len[i].flag_len, 64);
		info_size += ROUND_UP(vip_nn_buffer_len[i].info_len, 64);
	}
	if (flag_len)   *flag_len = flag_size;
	if (info_len)   *info_len = info_size;
	if (data_len)   *data_len = data_size;


	return (data_size+flag_size+info_size);
}
static bool assign_nn_buf(u32 phy_addr, u32 size, u8* cached, u8* noncached)
{
	u32 pa, remain_siz, offset, i;
	u32 flag_size = 0, info_size = 0, data_size = 0;
	u8* cached_addr = cached;
	u8* noncached_addr = noncached;
	VIP_NN_CTRL *nn_ctrl = 0;

	nn_ctrl = scalerAI_Access_NN_CTRL_STRUCT();

	rtd_pr_vpq_ai_info("nn buf start =0x%x\n", phy_addr);
	rtd_pr_vpq_ai_info("nn buf size =%d\n", size);
	rtd_pr_vpq_ai_info("nn buf cached start =%px\n", cached);
	rtd_pr_vpq_ai_info("nn buf noncached start =%px\n", noncached);

	pa = ROUND_UP(phy_addr, 64);
	offset = pa - phy_addr;
	remain_siz = size - offset;
	if (cached_addr != 0) cached_addr += offset;
	if (noncached_addr != 0) noncached_addr += offset;


	get_nn_buf_total_size(&flag_size, &info_size, &data_size);

	//flag
	if (noncached_addr == 0) {
		offset = pa & 0xfff;
		noncached_addr = (u8*)dvr_remap_uncached_memory(pa-offset, ROUND_UP(flag_size+offset, 4096), __builtin_return_address(0));
		if (noncached_addr == 0) {
			rtd_pr_vpq_ai_emerg("[%s %d] fail to map noncached memory: pa = 0x%x, size=%d\n", __func__, __LINE__, pa-offset, ROUND_UP(flag_size+offset, 4096));
			return 0;
		}
		nn_noncached_mem.ptr = noncached_addr;
		nn_noncached_mem.size = ROUND_UP(flag_size+offset, 4096);

		noncached_addr += offset;
	}
	for (i=0; i < VIP_NN_BUFFER_NUM; i++) {
		VIP_NN_MEMORY_CTRL * mem = &nn_ctrl->NN_flag_Addr[i];

		mem->phy_addr_align = pa;
		mem->size = ROUND_UP(vip_nn_buffer_len[i].flag_len, 64);
		mem->pVir_addr_align = (unsigned int *)noncached_addr;

		pa += mem->size;
		remain_siz -= mem->size;
		noncached_addr += mem->size;
	}

	offset = pa - phy_addr;
	if (cached != 0) cached_addr = cached + offset;
	if (noncached != 0) noncached_addr = noncached + offset;

	//info
	if (cached_addr == 0) {
		offset = pa & 0xfff;
		cached_addr = (u8*)dvr_remap_cached_memory(pa-offset, ROUND_UP(info_size+offset, 4096), __builtin_return_address(0));
		if (cached_addr == 0) {
			if (nn_noncached_mem.ptr) {
				dvr_unmap_memory(nn_noncached_mem.ptr, nn_noncached_mem.size);
				memset(&nn_noncached_mem, 0, sizeof(nn_noncached_mem));
			}
			rtd_pr_vpq_ai_emerg("[%s %d] fail to map cached memory: pa = 0x%x, size=%d\n", __func__, __LINE__, pa-offset, ROUND_UP(info_size+offset, 4096));

			return 0;
		}
		nn_cached_mem.ptr = cached_addr;
		nn_cached_mem.size = ROUND_UP(info_size+offset, 4096);

		cached_addr += offset;
	}
	for (i=0; i < VIP_NN_BUFFER_NUM; i++) {
		VIP_NN_MEMORY_CTRL * mem = &nn_ctrl->NN_info_Addr[i];

		mem->phy_addr_align = pa;
		mem->size = ROUND_UP(vip_nn_buffer_len[i].info_len, 64);
		mem->pVir_addr_align = (unsigned int *)cached_addr;

		pa += mem->size;
		remain_siz -= mem->size;
		cached_addr += mem->size;
	}

	offset = pa - phy_addr;
	if (cached != 0) cached_addr = cached + offset;
	if (noncached != 0) noncached_addr = noncached + offset;

	//data
	for (i=0; i < VIP_NN_BUFFER_NUM; i++) {
		VIP_NN_MEMORY_CTRL * mem = &nn_ctrl->NN_data_Addr[i];

		mem->phy_addr_align = pa;
		mem->size = vip_nn_buffer_len[i].data_len;
		mem->pVir_addr_align = 0;

		pa += mem->size;
		remain_siz -= mem->size;
	}
	return 1;
}

bool ai_optee_init(void)
{

	s32 total_size, offset;
	NN_BUF_INFO_T info;

	offset = VIPLITE_HEAP_SIZE + MODEL_HEAP_SIZE + NB_BUF_SIZE;
	total_size = offset + get_nn_buf_total_size(0, 0, 0);
	total_size = ROUND_UP(total_size, (1 << 22));

	info.buf_id = VIP_VIDEO_HEAP;
	info.size = total_size;
	if(get_nn_ta_buf(&video_heap, &info) == 0)
		return 0;

	rtkvdec_svp_cma_memory_init(video_heap.phy_addr, VIPLITE_HEAP_SIZE);
	return assign_nn_buf(video_heap.phy_addr+offset, video_heap.size-offset, video_heap.vir_addr+offset, 0);

}
void ai_optee_deinit(void)
{
	if (nn_noncached_mem.ptr) {
		dvr_unmap_memory(nn_noncached_mem.ptr, nn_noncached_mem.size);
		memset(&nn_noncached_mem, 0, sizeof(nn_noncached_mem));
	}

	if (nn_cached_mem.ptr) {
		dvr_unmap_memory(nn_cached_mem.ptr, nn_cached_mem.size);
		memset(&nn_cached_mem, 0, sizeof(nn_cached_mem));
	}

	if (video_heap.phy_addr)
		rtkvdec_svp_cma_memory_free(video_heap.phy_addr, VIPLITE_HEAP_SIZE);

	ai_optee_release_all_ta_buf();
}
