#include "rtk_vdec_struct.h"

unsigned char VideoGITHASH[8];

/* for SVP memory manage */
VDEC_SVPMEM_LIST      memory_list[RTKVDEC_MAX_SVPMEM_NUM] ;
VDEC_SVPMEM_USED_LIST   used_list[RTKVDEC_MAX_SVPMEM_NUM] ;
static DEFINE_SEMAPHORE(svp_memory_sem);
#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
unsigned int svp_aligned_size = 1;
#endif

/* for Inband cobuf manage */
VDEC_IBBMEM_LIST IBBuf[RTKVDEC_MAX_IBBUF_NUM] ;
unsigned int ibbuf_debug_en = 0;
VDEC_IBBMEM_LIST IBBuf_dbg[RTKVDEC_MAX_IBBUF_NUM] ;
static DEFINE_SEMAPHORE(IBBuf_sem);

/* for VMM memory manage */
VDEC_VBM_MEM_LIST VMM_Memory[RTKVDEC_MAX_VMM_MEM_NUM] ;
unsigned long VMM_total = 0;
unsigned long VMM_start = 0;

/* for VBM memory manage */
#ifdef CONFIG_VBM_HEAP
unsigned long VBM_total = 0;
unsigned long VBM_start = 0;
#else
VDEC_VBM_MEM_LIST VBM_Memory[RTKVDEC_MAX_VBM_MEM_NUM] ;
unsigned long VBM_total = 0;
unsigned long VBM_start = 0;
static DEFINE_SEMAPHORE(vbm_memory_sem);
#endif


/* for VBM frame base */
static struct list_head rtkvdec_frame_buf_head;
static DEFINE_SEMAPHORE(rtkvdec_frame_buf_sem);

extern unsigned long SharedMemUNVirtAddr;
#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
extern unsigned long SharedMemUNVirtAddr_V2 ;
#endif


void rtkvdec_mm_init(void)
{
	rtkvdec_VMM_memory_init();
	rtkvdec_VBM_memory_init();
	rtkvdec_svp_memory_init();
	rtkvdec_InbandBuf_init();
	rtkvdec_vbm_by_frame_init();
}


void rtkvdec_show_svp_status(void)
{
	int i ;
	rtd_pr_vdec_err("svp memory_list:\n");
	for ( i = 0; i < RTKVDEC_MAX_SVPMEM_NUM; i++ ) {
		rtd_pr_vdec_err("[%d] : used(%d) addr(%x) size(%x)\n", i, memory_list[i].used, memory_list[i].addr, memory_list[i].size);
	}
	rtd_pr_vdec_err("\n");
	rtd_pr_vdec_err("svp used_list:\n");
	for ( i = 0; i < RTKVDEC_MAX_SVPMEM_NUM; i++ ) {
		if ( used_list[i].addr ) {
			rtd_pr_vdec_err("[%d] : pid(%d) addr(%x) size(%x) prot(%d) index(%d) depth(%d)\n", i, used_list[i].pid, used_list[i].addr, used_list[i].size, used_list[i].prot, used_list[i].index, used_list[i].depth);
		}
	}
	rtd_pr_vdec_err("\n");
}


void rtkvdec_show_ibbuf_status(void)
{
	int i ;
	for (i = 0; i < RTKVDEC_MAX_IBBUF_NUM; i++) {
		rtd_pr_vdec_err("IBBuf[%d]%d %d %x %x\n", i, IBBuf[i].pid, IBBuf[i].used, IBBuf[i].addr, IBBuf[i].size);
		rtd_pr_vdec_err("  dbg[%d]%d %d %x %x\n", i, IBBuf_dbg[i].pid, IBBuf_dbg[i].used, IBBuf_dbg[i].addr, IBBuf_dbg[i].size);
	}
}


void rtkvdec_show_vmm_status(void)
{
	int i ;
	rtd_pr_vdec_err("\n============= VMM status =============\n");
	for (i = 0; i < RTKVDEC_MAX_VMM_MEM_NUM; i++) {
		rtd_pr_vdec_err("VMM chunk[%d]%d %lx %x\n", i, VMM_Memory[i].used, VMM_Memory[i].addr, VMM_Memory[i].size);
	}
}


void rtkvdec_show_vbm_status(void)
{
#ifndef CONFIG_VBM_HEAP
	int i ;
	rtd_pr_vdec_err("\n============= VBM chunk =============\n");
	for (i = 0; i < RTKVDEC_MAX_VBM_MEM_NUM; i++) {
		if (VBM_Memory[i].used) {
			rtd_pr_vdec_notice("VBM chunk[%d]used=%d addr=%lx size=%x\n", i, VBM_Memory[i].used, VBM_Memory[i].addr, VBM_Memory[i].size);
#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
			rtd_pr_vdec_notice("aligned addr=%lx size=%x\n", VBM_Memory[i].aligned_addr, VBM_Memory[i].aligned_size);
#endif
		}
	}
#endif
}


void rtkvdec_vbm_by_frame_summary()
{
	struct frmbuf_info *frmbuf_info, *tmp_frmbuf_info;

	rtd_pr_vdec_err("rtkvdec show frame buf info :\n");

	down(&rtkvdec_frame_buf_sem);
	list_for_each_entry_safe(frmbuf_info, tmp_frmbuf_info, &rtkvdec_frame_buf_head, frmbuf_list)
	{
		if ( frmbuf_info->frmbuf_ptr)
			rtd_pr_vdec_err("pa %x hdr_pa %x\n"          , (int)frmbuf_info->phy_addr, (int)frmbuf_info->hdr_phy_addr);
		else
			rtd_pr_vdec_err("pa %x hdr_pa %x 2k_Chroma\n", (int)frmbuf_info->phy_addr, (int)frmbuf_info->hdr_phy_addr);
	}
	up(&rtkvdec_frame_buf_sem);
}


bool rtkvdec_query_svp_protect(void)
{
	int i;

	for ( i = 0; i < RTKVDEC_MAX_SVPMEM_NUM; i++ ) {
		if ( used_list[i].addr && used_list[i].prot ) {
			return true;
		}
	}
	return false;
}


void rtkvdec_svp_memory_init(void)
{
	int i;
	unsigned int size = 0 ;
	unsigned long addr = 0 ;

	size = (unsigned int)carvedout_buf_query(CARVEDOUT_VDEC_RINGBUF, (void *)&addr) ;
	rtd_pr_vdec_notice("rtkvdec_SVP_memory_init %lx %x\n", addr, size);
	if ( addr == 0 || size == 0 ) {
		rtd_pr_vdec_err("[%s %d] ERR : %lx %x\n", __func__, __LINE__, addr, size);
		BUG();
	}
	rtkvdec_memset((void *)memory_list, 0, sizeof(memory_list));
	rtkvdec_memset((void *)used_list  , 0, sizeof(used_list));

	for ( i = 0; i < RTKVDEC_MAX_SVPMEM_NUM; i++ ) {
		if ( size >= RTKVDEC_SVPMEM_SIZE_4M ) {
			memory_list[i].addr = addr ;
			memory_list[i].size = RTKVDEC_SVPMEM_SIZE_4M ;
			size -= RTKVDEC_SVPMEM_SIZE_4M ;
			addr += RTKVDEC_SVPMEM_SIZE_4M ;
		}
		else if ( size != 0 ) {
			memory_list[i].addr = addr ;
			memory_list[i].size = size ;
			size = 0 ;
			addr += size ;
		}
	}
}


void rtkvdec_InbandBuf_init(void)
{
	int i ;

	for (i = 0; i < RTKVDEC_MAX_IBBUF_NUM; i++) {
		IBBuf[i].pid  = 0 ;
		IBBuf[i].used = 0 ;
		IBBuf[i].addr = 0 ;
		IBBuf[i].size = 0 ;
	}

	IBBuf[0].addr = rtkvdec_remote_malloc(RTKVDEC_IBBUF_SIZE * RTKVDEC_MAX_IBBUF_NUM, 0x90ABCDEF) & 0x7fffffff;
	if (!IBBuf[0].addr) {
		rtd_pr_vdec_err("rtkvdec : allocate InbandBuf fail!!!\n");
		return ;
	}

	IBBuf[0].size = RTKVDEC_IBBUF_SIZE ;
	for (i = 1 ; i < RTKVDEC_MAX_IBBUF_NUM ; i++) {
		IBBuf[i].addr = IBBuf[i-1].addr + RTKVDEC_IBBUF_SIZE ;
		IBBuf[i].size = RTKVDEC_IBBUF_SIZE ;
	}
}


void rtkvdec_VMM_memory_init(void)
{
	int i;

	for (i = 0; i < RTKVDEC_MAX_VMM_MEM_NUM; i++) {
		VMM_Memory[i].used = 0 ;
		VMM_Memory[i].addr = 0 ;
		VMM_Memory[i].size = 0 ;
	}

#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
	VMM_total = 0x400000 ;
#else
	VMM_total = 0x400000 ;
#endif
	VMM_start = rtkvdec_remote_malloc(VMM_total, 0x90ABCDEF) ;
	rtd_pr_vdec_notice("rtkvdec_VMM_memory_init %lx %lx\n", VMM_start, VMM_total);
	if (VMM_start == 0) {
		rtd_pr_vdec_err("[%s %d] ERR : %lx %lx\n", __func__, __LINE__, VMM_start, VMM_total);
		BUG();
	}
}


void rtkvdec_VBM_memory_init(void)
{
	unsigned int tmp ;
#ifndef CONFIG_VBM_HEAP
	int i;

	for (i = 0; i < RTKVDEC_MAX_VBM_MEM_NUM; i++) {
		VBM_Memory[i].used = 0 ;
		VBM_Memory[i].addr = 0 ;
		VBM_Memory[i].size = 0 ;
	}

	VBM_total = (unsigned int)carvedout_buf_query(CARVEDOUT_VDEC_VBM, (void *)&VBM_start) ;
#else
	if ( get_platform_model() == PLATFORM_MODEL_8K ) {
		VBM_total = (unsigned int)carvedout_buf_query(CARVEDOUT_VDEC_VBM, (void *)&VBM_start) ;
	}
	else if ( get_platform_model() == PLATFORM_MODEL_4K || get_platform_model() == PLATFORM_MODEL_2K ) {
		VBM_total = (unsigned int)carvedout_buf_query(CARVEDOUT_CMA_VBM, (void *)&VBM_start) ;
	}
	else {
		rtd_pr_vdec_err("[%s %d] invalid platform type %d\n", __func__, __LINE__, get_platform_model());
		BUG();
	}
#endif

#ifndef CONFIG_VBM_HEAP
#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
	svp_aligned_size = rtk_get_dc_secure_aligned_size(VBM_start);
#endif
#ifdef CONFIG_LG_SNAPSHOT_BOOT
	register_cma_forbidden_region(__phys_to_pfn(VBM_start), VBM_total);
#endif
	rtd_pr_vdec_notice("rtkvdec_VBM_memory_init %lx %lx\n", VBM_start, VBM_total);
	if (VBM_start == 0 || VBM_total == 0) {
		rtd_pr_vdec_err("[%s %d] ERR : %lx %lx\n", __func__, __LINE__, VBM_start, VBM_total);
		BUG();
	}
#endif
	/******************** get Video version ********************/
	tmp = rtd_inl(RTKVDEC_DUMMY_VBM_SIZE);
	snprintf(VideoGITHASH, 8, "%x", tmp);
	rtd_pr_vdec_notice("Video FW Version %s\n", VideoGITHASH);
	/***********************************************************/
	rtd_outl(RTKVDEC_DUMMY_VBM_SIZE, htonl( 0xBEAD0000 | VBM_total>>20));
}


void rtkvdec_vbm_by_frame_init()
{
	INIT_LIST_HEAD(&rtkvdec_frame_buf_head);
}


int rtkvdec_cobuffer_alloc(VDEC_COBUF_TYPE type, int port)
{
	int addr = 0 ;
	int i ;

	switch ( type )
	{
		case VDEC_SVP_BUF:
		{
			VDEC_SVPMEM_STRUCT svpmem;
			svpmem.size = RTKVDEC_SVPMEM_SIZE_8M;
			svpmem.pid  = port;
			svpmem.addr = 0;
			rtkvdec_svp_alloc(&svpmem, 0);
			if ( svpmem.addr == 0 ) {
				rtd_pr_vdec_err("rtkvdec : no free SVP Memory!!!\n");
				return 0;
			}
			addr = svpmem.addr;
			break;
		}

		case VDEC_INBAND_BUF:
		{
			down(&IBBuf_sem);
			for (i = 0; i < RTKVDEC_MAX_IBBUF_NUM; i++) {
				if (IBBuf[i].used == 0 && IBBuf[i].addr != 0) {
					addr = IBBuf[i].addr ;
					IBBuf[i].pid = port ;
					IBBuf[i].used = 1 ;
					if (ibbuf_debug_en && IBBuf_dbg[i].used == 0 && IBBuf_dbg[i].addr == 0) {
						IBBuf_dbg[i].addr = rtkvdec_alloc(4096);
						IBBuf_dbg[i].size = 4096 ;
						IBBuf_dbg[i].pid = port ;
						IBBuf_dbg[i].used = 1 ;
						memory_debug_protect(i, IBBuf_dbg[i].addr, 4096);
						rtd_pr_vdec_notice("rtkvdec: alloc IB debug mem 0x%x\n", IBBuf_dbg[i].addr);
					}
					break ;
				}
			}
			up(&IBBuf_sem);

			if (i == RTKVDEC_MAX_IBBUF_NUM && addr == 0) {
				rtd_pr_vdec_err("rtkvdec : no free IB Buffer!!!\n");
			}
			break;
		}

		default:
		{
			rtd_pr_vdec_debug("rtkvdec_cobuffer_alloc: unknown COBUF type(%d)\n", type);
			break;
		}
	}
	return addr ;
}
EXPORT_SYMBOL_GPL(rtkvdec_cobuffer_alloc);


int rtkvdec_cobuffer_free(VDEC_COBUF_TYPE type, unsigned int phy_addr)
{
	int i;
	switch ( type )
	{
		case VDEC_SVP_BUF:
		{
			down(&svp_memory_sem);
			for ( i = 0; i < RTKVDEC_MAX_SVPMEM_NUM; i++ ) {
				if ( used_list[i].addr == phy_addr ) {
					if ( used_list[i].prot ) {
#if IS_ENABLED(CONFIG_RTK_KDRV_TEE)
						if (rtkvdec_svp_disable_cpb_protection(used_list[i].addr, used_list[i].size) != 0) {
							rtd_pr_vdec_err("rtkvdec: failed to disable cpb protection\n");
						}
#endif
					}
					__rtkvdec_svp_free(i);
					break ;
				}
			}
			up(&svp_memory_sem);
			break;
		}

		case VDEC_INBAND_BUF:
		{
			down(&IBBuf_sem);
			for (i = 0; i < RTKVDEC_MAX_IBBUF_NUM; i++) {
				if (IBBuf[i].addr == phy_addr) {
					IBBuf[i].used = 0 ;
					IBBuf[i].pid = 0 ;
					if (IBBuf_dbg[i].used && IBBuf_dbg[i].addr != 0) {
						rtd_pr_vdec_notice("rtkvdec: free IB debug mem 0x%x\n", IBBuf_dbg[i].addr);
						memory_debug_unprotect(i);
						rtkvdec_free(IBBuf_dbg[i].addr);
						IBBuf_dbg[i].addr = 0 ;
						IBBuf_dbg[i].size = 0 ;
						IBBuf_dbg[i].used = 0 ;
						IBBuf_dbg[i].pid = 0 ;
					}
					break ;
				}
			}
			up(&IBBuf_sem);
			break ;
		}

		default:
		{
			rtd_pr_vdec_debug("rtkvdec_cobuffer_free: unknown COBUF type(%d)\n", type);
			break;
		}
	}

	return 0 ;
}
EXPORT_SYMBOL_GPL(rtkvdec_cobuffer_free);


int rtkvdec_svp_alloc(VDEC_SVPMEM_STRUCT *svpmem, unsigned int cmd)
{
	int i;
	unsigned int index = 255;
	unsigned int depth = 0;
	unsigned int protection = 0;
	down(&svp_memory_sem);
	switch ( svpmem->size )
	{
		case RTKVDEC_SVPMEM_SIZE_4M:
		{
			depth = 1;
			if ( memory_list[0].used == 0 && memory_list[0].addr != 0 ) {
				index = 0;
			}
			else if ( memory_list[3].used == 0 && memory_list[3].addr != 0 ) {
				index = 3;
			}
			else if ( memory_list[1].used == 0 && memory_list[1].addr != 0 ) {
				index = 1;
			}
			else if ( memory_list[2].used == 0 && memory_list[2].addr != 0 ) {
				index = 2;
			}
			else if ( memory_list[4].used == 0 && memory_list[4].addr != 0 ) {
				index = 4;
			}
			else if ( memory_list[7].used == 0 && memory_list[7].addr != 0 ) {
				index = 7;
			}
			else if ( memory_list[5].used == 0 && memory_list[5].addr != 0 ) {
				index = 5;
			}
			else if ( memory_list[6].used == 0 && memory_list[6].addr != 0 ) {
				index = 6;
			}
			break;
		}

		case RTKVDEC_SVPMEM_SIZE_8M:
		{
			depth = 2;
			if ( memory_list[0].used == 0 && memory_list[0].addr != 0 && memory_list[1].used == 0 && memory_list[1].addr != 0 ) {
				index = 0;
			}
			else if ( memory_list[2].used == 0 && memory_list[2].addr != 0 && memory_list[3].used == 0 && memory_list[3].addr != 0 ) {
				index = 2;
			}
			else if ( memory_list[1].used == 0 && memory_list[1].addr != 0 && memory_list[2].used == 0 && memory_list[2].addr != 0 ) {
				index = 1;
			}
			else if ( memory_list[4].used == 0 && memory_list[4].addr != 0 && memory_list[5].used == 0 && memory_list[5].addr != 0 ) {
				index = 4;
			}
			else if ( memory_list[6].used == 0 && memory_list[6].addr != 0 && memory_list[7].used == 0 && memory_list[7].addr != 0 ) {
				index = 6;
			}
			else if ( memory_list[5].used == 0 && memory_list[5].addr != 0 && memory_list[6].used == 0 && memory_list[6].addr != 0 ) {
				index = 5;
			}
			break;
		}

		case RTKVDEC_SVPMEM_SIZE_12M:
		{
			depth = 3;
			if ( memory_list[0].used == 0 && memory_list[0].addr != 0 && memory_list[1].used == 0 && memory_list[1].addr != 0 && memory_list[2].used == 0 && memory_list[2].addr != 0 ) {
				index = 0;
			}
			else if ( memory_list[1].used == 0 && memory_list[1].addr != 0 && memory_list[2].used == 0 && memory_list[2].addr != 0 && memory_list[3].used == 0 && memory_list[3].addr != 0 ) {
				index = 1;
			}
			else if ( memory_list[4].used == 0 && memory_list[4].addr != 0 && memory_list[5].used == 0 && memory_list[5].addr != 0 && memory_list[6].used == 0 && memory_list[6].addr != 0 ) {
				index = 4;
			}
			else if ( memory_list[5].used == 0 && memory_list[5].addr != 0 && memory_list[6].used == 0 && memory_list[6].addr != 0 && memory_list[7].used == 0 && memory_list[7].addr != 0 ) {
				index = 5;
			}
			break;
		}

		case RTKVDEC_SVPMEM_SIZE_16M:
		{
			depth = 4;
			if ( memory_list[0].used == 0 && memory_list[0].addr != 0 && memory_list[1].used == 0 && memory_list[1].addr != 0 && memory_list[2].used == 0 && memory_list[2].addr != 0 && memory_list[3].used == 0 && memory_list[3].addr != 0 ) {
				index = 0;
			}
			else if ( memory_list[4].used == 0 && memory_list[4].addr != 0 && memory_list[5].used == 0 && memory_list[5].addr != 0 && memory_list[6].used == 0 && memory_list[6].addr != 0 && memory_list[7].used == 0 && memory_list[7].addr != 0 ) {
				index = 4;
			}
			break;
		}

		case RTKVDEC_SVPMEM_SIZE_32M:
		{
			depth = 8;
			if ( memory_list[0].used == 0 && memory_list[0].addr != 0 && memory_list[1].used == 0 && memory_list[1].addr != 0 && memory_list[2].used == 0 && memory_list[2].addr != 0 && memory_list[3].used == 0 && memory_list[3].addr != 0 &&
			     memory_list[4].used == 0 && memory_list[4].addr != 0 && memory_list[5].used == 0 && memory_list[5].addr != 0 && memory_list[6].used == 0 && memory_list[6].addr != 0 && memory_list[7].used == 0 && memory_list[7].addr != 0 ) {
				index = 0;
			}
			break;
		}

		default:
		{
			rtd_pr_vdec_err("rtkvdec : invalid allocate size %x!!!\n", svpmem->size);
			rtkvdec_show_svp_status();
			up(&svp_memory_sem);
			return -EFAULT;
			break;
		}
	}

	if ( index == 255 ) {
		rtd_pr_vdec_err("rtkvdec : no free SVP Memory!!!\n");
		rtkvdec_show_svp_status();
		up(&svp_memory_sem);
		return -EFAULT;
	}

	svpmem->addr = memory_list[index].addr ;
	for ( i = 0; i < depth; i++ )
		memory_list[index + i].used = 1 ;

	if ( cmd == VDEC_IOC_ALLOCSVPMP ) {
#if IS_ENABLED(CONFIG_RTK_KDRV_TEE)
		if ( rtkvdec_svp_enable_cpb_protection(svpmem->addr, svpmem->size) != 0 ) {
			rtd_pr_vdec_err("rtkvdec : failed to enable cpb protection\n");
			BUG();
		}
#endif
		protection = 1;
		rtd_pr_vdec_debug("rtkvdec : enable_cpb_protection(%d) addr %x SIZE %d \n", i, svpmem->addr, svpmem->size);
	}

	for ( i = 0; i < RTKVDEC_MAX_SVPMEM_NUM; i++ ) {
		if ( used_list[i].addr == 0 ) {
			used_list[i].pid  = svpmem->pid;
			used_list[i].addr = svpmem->addr;
			used_list[i].size = svpmem->size;
			used_list[i].index = index;
			used_list[i].depth = depth;
			used_list[i].prot  = protection;
			break;
		}
	}
	up(&svp_memory_sem);

	return 0;
}


int __rtkvdec_svp_free(int index)
{
	int i;
	VDEC_SVPMEM_USED_LIST *used_node = &used_list[index];
	for ( i = 0; i < used_node->depth; i++ )
		memory_list[used_node->index + i].used = 0 ;

	used_node->pid   = 0;
	used_node->addr  = 0;
	used_node->size  = 0;
	used_node->index = 0;
	used_node->depth = 0;
	used_node->prot  = 0;
	return 0;
}


int rtkvdec_svp_free(VDEC_SVPMEM_STRUCT *svpmem)
{
	int i;

	down(&svp_memory_sem);
	for ( i = 0; i < RTKVDEC_MAX_SVPMEM_NUM; i++ ) {
		if ( used_list[i].addr == svpmem->addr ) {
			if ( used_list[i].prot ) {
#if IS_ENABLED(CONFIG_RTK_KDRV_TEE)
				if (rtkvdec_svp_disable_cpb_protection(used_list[i].addr, used_list[i].size) != 0) {
					rtd_pr_vdec_err("rtkvdec: failed to disable cpb protection\n");
				}
#endif
			}
			__rtkvdec_svp_free(i);
			break ;
		}
	}
	up(&svp_memory_sem);
	return 0;
}


void rtkvdec_svp_free_by_pid(unsigned long pid)
{
	int i;

	down(&svp_memory_sem);
	for ( i = 0; i < RTKVDEC_MAX_SVPMEM_NUM; i++ ) {
		if ( used_list[i].pid == pid ) {
			if ( used_list[i].prot ) {
#if IS_ENABLED(CONFIG_RTK_KDRV_TEE)
				if (rtkvdec_svp_disable_cpb_protection(used_list[i].addr, used_list[i].size) != 0) {
					rtd_pr_vdec_err("rtkvdec: failed to disable cpb protection\n");
				}
#endif
			}
			__rtkvdec_svp_free(i);
			break ;
		}
	}
	up(&svp_memory_sem);
}


void rtkvdec_ibbuf_free_by_pid(unsigned long pid)
{
	int i;

	down(&IBBuf_sem);
	for (i = 0; i < RTKVDEC_MAX_IBBUF_NUM; i++) {
		if (IBBuf[i].pid == pid) {
			IBBuf[i].used = 0 ;
			IBBuf[i].pid = 0 ;
		}
		if (IBBuf_dbg[i].used && IBBuf_dbg[i].addr != 0 && IBBuf_dbg[i].pid == pid) {
			rtd_pr_vdec_notice("rtkvdec: free IB debug mem 0x%x\n", IBBuf_dbg[i].addr);
			memory_debug_unprotect(i);
			rtkvdec_free(IBBuf_dbg[i].addr);
			IBBuf_dbg[i].addr = 0 ;
			IBBuf_dbg[i].size = 0 ;
			IBBuf_dbg[i].used = 0 ;
			IBBuf_dbg[i].pid = 0 ;
		}
	}
	up(&IBBuf_sem);
}


unsigned long rtkvdec_vbm_alloc(unsigned long size)
{
#ifndef CONFIG_VBM_HEAP
	unsigned long phy_addr = 0 ;
	unsigned int i;

	if ( VBM_total >= size ) {
		for ( i = 0; i < RTKVDEC_MAX_VBM_MEM_NUM; i++ ) {
			if ( VBM_Memory[i].used == 0 ) {
				phy_addr = VBM_Memory[i].addr = (i == 0)? VBM_start : VBM_Memory[i-1].addr + VBM_Memory[i-1].size ;
				VBM_Memory[i].size = size ;
				VBM_Memory[i].used = 1 ;
#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
				VBM_Memory[i].aligned_size = ALIGNED_DOWN((VBM_Memory[i].addr + VBM_Memory[i].size), svp_aligned_size) - ALIGNED_UP(VBM_Memory[i].addr, svp_aligned_size);
				VBM_Memory[i].aligned_addr = ALIGNED_UP(VBM_Memory[i].addr, svp_aligned_size);
#endif
				VBM_total -= size ;
				return phy_addr ;
			}
		}
	}

	rtd_pr_vdec_err("[%s %d]alloc VBM fail %x\n", __func__, __LINE__, (unsigned int)size);
	rtkvdec_show_vbm_status();
	BUG();
#endif
	return 0;
}


unsigned long rtkvdec_vbm_free(unsigned long addr)
{
#ifndef CONFIG_VBM_HEAP
	int i = 0;

	for ( i = 0; i < RTKVDEC_MAX_VBM_MEM_NUM ; i++ ) {
		if (addr == VBM_Memory[i].addr) {
			VBM_total += VBM_Memory[i].size ;
			VBM_Memory[i].addr =
			VBM_Memory[i].size =
			VBM_Memory[i].used = 0 ;
#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
			VBM_Memory[i].aligned_addr = 0;
			VBM_Memory[i].aligned_size = 0;
#endif
			return 0;
		}
	}
	return 1;
#else
	return 0;
#endif
}


unsigned long rtkvdec_vmm_alloc(unsigned long size)
{
	unsigned long phy_addr = 0 ;
	unsigned int i;

	if (VMM_total >= size) {
		for (i = 0; i < RTKVDEC_MAX_VMM_MEM_NUM; i++) {
			if ( VMM_Memory[i].used == 0 ) {
				phy_addr = VMM_Memory[i].addr = (i == 0)? VMM_start : VMM_Memory[i-1].addr + VMM_Memory[i-1].size;
				VMM_Memory[i].size = size;
				VMM_Memory[i].used = 1;
				VMM_total -= size;
				return phy_addr;
			}
		}
	}

	rtd_pr_vdec_err("[%s %d]alloc VMM fail %x\n", __func__, __LINE__, (unsigned int)size);
	rtkvdec_show_vmm_status();
	BUG();
	return 0;
}


unsigned long rtkvdec_vmm_free(unsigned long addr)
{
	int i = 0;

	for ( i = 0; i < RTKVDEC_MAX_VMM_MEM_NUM ; i++ ) {
		if ( addr == VMM_Memory[i].addr ) {
			VMM_total += VMM_Memory[i].size ;
			VMM_Memory[i].addr =
			VMM_Memory[i].size =
			VMM_Memory[i].used = 0 ;
			return 0;
		}
	}
	return 1;
}


void rtkvdec_vmm_destroy(void)
{
	int i = 0;

	if (VMM_start) {
		rtkvdec_remote_free(VMM_start, 0x90ABCDEF);
		for (i = 0; i < RTKVDEC_MAX_VMM_MEM_NUM; i++) {
			VMM_Memory[i].used = 0 ;
			VMM_Memory[i].addr = 0 ;
			VMM_Memory[i].size = 0 ;
		}
		VMM_start = VMM_total = 0 ;
	}
}


#ifdef CONFIG_VBM_HEAP
void rtkvdec_vbm_free_by_chunk(unsigned long phy_addr)
{
	struct frmbuf_info *frmbuf_info, *tmp_frmbuf_info;

	down(&rtkvdec_frame_buf_sem);
	/* free addr */
	list_for_each_entry_safe(frmbuf_info, tmp_frmbuf_info, &rtkvdec_frame_buf_head, frmbuf_list)
	{
		if ( frmbuf_info->phy_addr == phy_addr ) {
			dvr_dmabuf_free(frmbuf_info->frmbuf_ptr);
			list_del(&frmbuf_info->frmbuf_list);
			kfree(frmbuf_info);
			break;
		}
	}
	up(&rtkvdec_frame_buf_sem);
	return;
}


int rtkvdec_vbm_free_by_frame(int vcpu_id)
{
	VDEC_SHARE_MEM_STRUCT *gpShmem;
	struct vbm_frm_mm *frm_mm_ptr = NULL;

	unsigned int i, j, check, addr, hdraddr;
	struct frmbuf_info *frmbuf_info, *tmp_frmbuf_info;

	if ( vcpu_id == 1 )
	{
		if ( !SharedMemUNVirtAddr ) {
			rtd_pr_vdec_err("[%s %d] no shared memory.\n", __func__, __LINE__);
			return -1;
		}

		gpShmem = (VDEC_SHARE_MEM_STRUCT *)SharedMemUNVirtAddr;
	}
#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
	else if ( vcpu_id == 2 )
	{
		if ( !SharedMemUNVirtAddr_V2 ) {
			rtd_pr_vdec_err("[%s %d] no shared memory.\n", __func__, __LINE__);
			return -1;
		}

		gpShmem = (VDEC_SHARE_MEM_STRUCT *)SharedMemUNVirtAddr_V2;
	}
#endif
	else
	{
		rtd_pr_vdec_err("[%s %d] wrong vcpu_id %d\n", __func__, __LINE__, vcpu_id);
		return -1;
	}

	down(&rtkvdec_frame_buf_sem);

	for ( j = 0; j < 8; j++ ) {
		frm_mm_ptr = &(gpShmem->frm_mm[j]);
		check  = ntohl(frm_mm_ptr->memCheck);

		if ( check != VBM_CHECK_TYPE_FREE_START )
			continue;

		/* free luma */
		for ( i = 0; i < FRAME_ARRAY_SIZE; i++) {
			if ( 0 != (addr = ntohl(frm_mm_ptr->memFreeLuAddr[i])) ) {
				hdraddr = ntohl(frm_mm_ptr->memFreeLuHdrAddr[i]);

				/* free addr and hdraddr */
				list_for_each_entry_safe(frmbuf_info, tmp_frmbuf_info, &rtkvdec_frame_buf_head, frmbuf_list)
				{
					if ( frmbuf_info->phy_addr == (unsigned long)addr ) {
						dvr_dmabuf_free(frmbuf_info->frmbuf_ptr);
						if ( hdraddr != 0 ) {
							if ( frmbuf_info->hdr_phy_addr == (unsigned long)hdraddr )
								pli_free(frmbuf_info->hdr_phy_addr);
							else
								rtd_pr_vdec_err("[%s %d] Warning : hdraddr isn't match! %x %x\n", __func__, __LINE__, hdraddr, (unsigned int)frmbuf_info->hdr_phy_addr);
						}
						list_del(&frmbuf_info->frmbuf_list);
						kfree(frmbuf_info);
						break;
					}
				}

				frm_mm_ptr->memFreeLuAddr[i]    = htonl(0);
				frm_mm_ptr->memFreeLuHdrAddr[i] = htonl(0);
			}
		}

		/* free chroma */
		for ( i = 0; i < FRAME_ARRAY_SIZE; i++) {
			if ( 0 != (addr = ntohl(frm_mm_ptr->memFreeChAddr[i])) ) {
				hdraddr = ntohl(frm_mm_ptr->memFreeChHdrAddr[i]);

				/* free addr and hdraddr */
				list_for_each_entry_safe(frmbuf_info, tmp_frmbuf_info, &rtkvdec_frame_buf_head, frmbuf_list)
				{
					if ( frmbuf_info->phy_addr == (unsigned long)addr ) {
						if ( frmbuf_info->frmbuf_ptr )
							dvr_dmabuf_free(frmbuf_info->frmbuf_ptr);
						if ( hdraddr != 0 ) {
							if ( frmbuf_info->hdr_phy_addr == (unsigned long)hdraddr )
								pli_free(frmbuf_info->hdr_phy_addr);
							else
								rtd_pr_vdec_err("[%s %d] Warning : hdraddr isn't match! %x %x\n", __func__, __LINE__, hdraddr, (unsigned int)frmbuf_info->hdr_phy_addr);
						}
						list_del(&frmbuf_info->frmbuf_list);
						kfree(frmbuf_info);
						break;
					}
				}

				frm_mm_ptr->memFreeChAddr[i]    = htonl(0);
				frm_mm_ptr->memFreeChHdrAddr[i] = htonl(0);
			}
		}

		frm_mm_ptr->memCheck = htonl(VBM_CHECK_TYPE_EMPTY);
	}

	up(&rtkvdec_frame_buf_sem);

	return 0;
}


int rtkvdec_vbm_alloc_by_chunk(int size)
{
	struct dma_buf *dmabuf_Ptr = NULL;
	unsigned long phy_addr = 0;
	struct frmbuf_info *frmbuf_info;

	down(&rtkvdec_frame_buf_sem);

	dmabuf_Ptr = dvr_dmabuf_chunk_malloc(size, VBM_BUF_DECOMP, DMA_HEAP_USER_COBUFFER, false, (unsigned long **)&phy_addr);
	if ( IS_ERR_OR_NULL(dmabuf_Ptr) ) {
		rtd_pr_vdec_err("[%s %d] ERR : allocate %x fail\n", __func__, __LINE__, size);
		up(&rtkvdec_frame_buf_sem);
		return 0;
	}

	/* record memory*/
	frmbuf_info = kmalloc(sizeof(struct frmbuf_info), GFP_KERNEL);
	if ( !frmbuf_info ) {
		rtd_pr_vdec_err("[%s %d] ERR : failed to kmalloc\n", __func__, __LINE__);
		BUG();
		up(&rtkvdec_frame_buf_sem);
		return 0;
	}

	frmbuf_info->frmbuf_ptr   = (struct dma_buf *)dmabuf_Ptr;
	frmbuf_info->phy_addr     = dvr_dmabuf_to_phys((struct dma_buf *)dmabuf_Ptr);
	frmbuf_info->hdr_phy_addr = 0;
	list_add(&frmbuf_info->frmbuf_list, &rtkvdec_frame_buf_head);
	up(&rtkvdec_frame_buf_sem);

	return frmbuf_info->phy_addr;
}


int rtkvdec_vbm_alloc_by_frame(int NumofUse)
{
	VDEC_SHARE_MEM_STRUCT *gpShmem;
	struct vbm_frm_mm *frm_mm_ptr = NULL;

	struct dma_buf *dmabuf_LuPtr = NULL;
	struct dma_buf *dmabuf_ChPtr = NULL;
	unsigned long dmabuf_LuArray[FRAME_ARRAY_SIZE] = {0};
	unsigned long dmabuf_ChArray[FRAME_ARRAY_SIZE] = {0};
	unsigned long LuHdrPA       [FRAME_ARRAY_SIZE] = {0};
	unsigned long ChHdrPA       [FRAME_ARRAY_SIZE] = {0};

	struct frmbuf_info *frmbuf_info;
	unsigned long first_phy_addr = 0;
	unsigned long shmem_unva = 0;
	unsigned int check, type, LuSize, ChSize, hdrsize, cnt, get_Lucnt, get_Chcnt;
	int i;
	bool bProt = false;

	if ( NumofUse >= 256 )
	{
		NumofUse -= 256;
		shmem_unva = SharedMemUNVirtAddr_V2;
	}
	else
	{
		shmem_unva = SharedMemUNVirtAddr;
	}

	if ( !shmem_unva ) {
		rtd_pr_vdec_err("[%s %d] no shared memory.\n", __func__, __LINE__);
		return -1;
	}

	if ( NumofUse < 0 || NumofUse >= 8 ) {
		rtd_pr_vdec_err("[%s %d] invalid parameter(%d).\n", __func__, __LINE__, NumofUse);
		return -1;
	}

	down(&rtkvdec_frame_buf_sem);

	gpShmem = (VDEC_SHARE_MEM_STRUCT *)shmem_unva;
	frm_mm_ptr = &(gpShmem->frm_mm[NumofUse]);
	check  = ntohl(frm_mm_ptr->memCheck);

	if ( check != VBM_CHECK_TYPE_ALLOC_START ) {
		frm_mm_ptr->memCheck = htonl(VBM_CHECK_TYPE_ALLOC_FAIL);
		up(&rtkvdec_frame_buf_sem);
		rtd_pr_vdec_err("[%s %d] allocate status(%d) is wrong.\n", __func__, __LINE__, check);
		return -1;
	}

	type    = ntohl(frm_mm_ptr->memType);
	LuSize  = rtkvdec_align(ntohl(frm_mm_ptr->memLuSize), 4096);
	ChSize  = rtkvdec_align(ntohl(frm_mm_ptr->memChSize), 4096);
	hdrsize = ntohl(frm_mm_ptr->memHdrSize);
	cnt     = ntohl(frm_mm_ptr->memCnt);
	frm_mm_ptr->memCheck = htonl(VBM_CHECK_TYPE_ALLOC_DOING);
	bProt = rtkvdec_query_svp_protect();

	rtd_pr_vdec_err("[%s %d] alloc info type %d Lu %x Ch %x Hdr %x cnt %d\n", __func__, __LINE__, type, LuSize, ChSize, hdrsize, cnt);

	if ( type == VBM_REQ_TYPE_4K ) {
		/* allocate Luma */
		dmabuf_LuPtr = dvr_dmabuf_frames_malloc(LuSize, cnt, VBM_BUF_DECOMP, DMA_HEAP_USER_VIDEO_4K_LUMA, bProt, (unsigned long **)&first_phy_addr);
		if ( IS_ERR_OR_NULL(dmabuf_LuPtr) ) {
			rtd_pr_vdec_err("[%s %d] alloc dmabuf fail\n", __func__, __LINE__);
			goto ERR_CASE_LUMA_FAIL;
		}
		else {
			/* allocate Chroma */
			first_phy_addr = 0;
			dmabuf_ChPtr = dvr_dmabuf_frames_malloc(ChSize, cnt, VBM_BUF_DECOMP, DMA_HEAP_USER_VIDEO_4K_CHROMA, bProt, (unsigned long **)&first_phy_addr);
			if ( IS_ERR_OR_NULL(dmabuf_ChPtr) ) {
				rtd_pr_vdec_err("[%s %d] alloc dmabuf fail\n", __func__, __LINE__);
				goto ERR_CASE_CHROMA_FAIL;
			}
		}

		/* get luma and chroma*/
		get_Lucnt = dvr_dmabuf_to_list(dmabuf_LuPtr, dmabuf_LuArray, cnt);
		get_Chcnt = dvr_dmabuf_to_list(dmabuf_ChPtr, dmabuf_ChArray, cnt);
		if ( get_Lucnt != cnt || get_Chcnt != cnt ) {
			rtd_pr_vdec_err("[%s %d] cnt(%d) isn't match Lu(%d) or Ch(%d)\n", __func__, __LINE__, cnt, get_Lucnt, get_Lucnt);
			goto ERR_CASE_CNT_NOT_MATCH;
		}

		if ( hdrsize != 0 ) {
			/* allocate luma and chroma Hdr*/
			for ( i = 0; i < cnt; i++ ) {
				LuHdrPA[i] = pli_malloc(hdrsize, GFP_DCU2_VDEC_CMPHDR);
				if ( LuHdrPA[i] == INVALID_VAL ) {
					rtd_pr_vdec_err("[%s %d] allocate LuHdrPA[%d] fail\n", __func__, __LINE__, i);
					goto ERR_CASE_HDR_FAIL;
				}

				ChHdrPA[i] = pli_malloc(hdrsize>>1, GFP_DCU2_VDEC_CMPHDR);
				if ( ChHdrPA[i] == INVALID_VAL ) {
					rtd_pr_vdec_err("[%s %d] allocate ChHdrPA[%d] fail\n", __func__, __LINE__, i);
					goto ERR_CASE_HDR_FAIL;
				}
			}
		}

		/* record luma*/
		for ( i = 0; i < cnt; i++ ) {
			frmbuf_info = kmalloc(sizeof(struct frmbuf_info), GFP_KERNEL);
			if ( !frmbuf_info ) {
				rtd_pr_vdec_err("[%s %d] ERR : failed to kmalloc\n", __func__, __LINE__);
				BUG();
				return 0;
			}

			frmbuf_info->frmbuf_ptr   = (struct dma_buf *)dmabuf_LuArray[i];
			frmbuf_info->phy_addr     = dvr_dmabuf_to_phys((struct dma_buf *)dmabuf_LuArray[i]);
			frmbuf_info->hdr_phy_addr = LuHdrPA[i];
			list_add(&frmbuf_info->frmbuf_list, &rtkvdec_frame_buf_head);

			frm_mm_ptr->memLuAddr[i]    = htonl((unsigned int)frmbuf_info->phy_addr);
			frm_mm_ptr->memLuHdrAddr[i] = htonl((unsigned int)frmbuf_info->hdr_phy_addr);
		}

		/* record chroma*/
		for ( i = 0; i < cnt; i++ ) {
			frmbuf_info = kmalloc(sizeof(struct frmbuf_info), GFP_KERNEL);
			if ( !frmbuf_info ) {
				rtd_pr_vdec_err("[%s %d] ERR : failed to kmalloc\n", __func__, __LINE__);
				BUG();
				return 0;
			}

			frmbuf_info->frmbuf_ptr   = (struct dma_buf *)dmabuf_ChArray[i];
			frmbuf_info->phy_addr     = dvr_dmabuf_to_phys((struct dma_buf *)dmabuf_ChArray[i]);
			frmbuf_info->hdr_phy_addr = ChHdrPA[i];
			list_add(&frmbuf_info->frmbuf_list, &rtkvdec_frame_buf_head);

			frm_mm_ptr->memChAddr[i]    = htonl((unsigned int)frmbuf_info->phy_addr);
			frm_mm_ptr->memChHdrAddr[i] = htonl((unsigned int)frmbuf_info->hdr_phy_addr);
		}

		frm_mm_ptr->memCheck = htonl(VBM_CHECK_TYPE_ALLOC_FINISH);
	}
	else if ( type == VBM_REQ_TYPE_2K ) {
		/* allocate Luma + Chroma */
		dmabuf_LuPtr = dvr_dmabuf_frames_malloc(LuSize + ChSize, cnt, VBM_BUF_DECOMP, DMA_HEAP_USER_VIDEO_2K, bProt, (unsigned long **)&first_phy_addr);
		if ( IS_ERR_OR_NULL(dmabuf_LuPtr) ) {
			rtd_pr_vdec_err("[%s %d] alloc dmabuf fail\n", __func__, __LINE__);
			goto ERR_CASE_LUMA_FAIL;
		}

		/* get luma*/
		get_Lucnt = dvr_dmabuf_to_list(dmabuf_LuPtr, dmabuf_LuArray, cnt);
		if ( get_Lucnt != cnt ) {
			rtd_pr_vdec_err("[%s %d] cnt(%d) isn't match Lu(%d) or Ch(%d)\n", __func__, __LINE__, cnt, get_Lucnt, get_Lucnt);
			goto ERR_CASE_CNT_NOT_MATCH;
		}

		if ( hdrsize != 0 ) {
			/* allocate luma and chroma Hdr*/
			for ( i = 0; i < cnt; i++ ) {
				LuHdrPA[i] = pli_malloc(hdrsize, GFP_DCU2_VDEC_CMPHDR);
				if ( LuHdrPA[i] == INVALID_VAL ) {
					rtd_pr_vdec_err("[%s %d] allocate LuHdrPA[%d] fail\n", __func__, __LINE__, i);
					goto ERR_CASE_HDR_FAIL;
				}

				ChHdrPA[i] = pli_malloc(hdrsize>>1, GFP_DCU2_VDEC_CMPHDR);
				if ( ChHdrPA[i] == INVALID_VAL ) {
					rtd_pr_vdec_err("[%s %d] allocate ChHdrPA[%d] fail\n", __func__, __LINE__, i);
					goto ERR_CASE_HDR_FAIL;
				}
			}
		}

		/* record luma*/
		for ( i = 0; i < cnt; i++ ) {
			frmbuf_info = kmalloc(sizeof(struct frmbuf_info), GFP_KERNEL);
			if ( !frmbuf_info ) {
				rtd_pr_vdec_err("[%s %d] ERR : failed to kmalloc\n", __func__, __LINE__);
				BUG();
				return 0;
			}

			frmbuf_info->frmbuf_ptr   = (struct dma_buf *)dmabuf_LuArray[i];
			frmbuf_info->phy_addr     = dvr_dmabuf_to_phys((struct dma_buf *)dmabuf_LuArray[i]);
			frmbuf_info->hdr_phy_addr = LuHdrPA[i];
			list_add(&frmbuf_info->frmbuf_list, &rtkvdec_frame_buf_head);

			frm_mm_ptr->memLuAddr[i]    = htonl((unsigned int)frmbuf_info->phy_addr);
			frm_mm_ptr->memLuHdrAddr[i] = htonl((unsigned int)frmbuf_info->hdr_phy_addr);
		}

		/* record chroma*/
		for ( i = 0; i < cnt; i++ ) {
			frmbuf_info = kmalloc(sizeof(struct frmbuf_info), GFP_KERNEL);
			if ( !frmbuf_info ) {
				rtd_pr_vdec_err("[%s %d] ERR : failed to kmalloc\n", __func__, __LINE__);
				BUG();
				return 0;
			}

			frmbuf_info->frmbuf_ptr   = NULL;
			frmbuf_info->phy_addr     = dvr_dmabuf_to_phys((struct dma_buf *)dmabuf_LuArray[i]) + LuSize;
			frmbuf_info->hdr_phy_addr = ChHdrPA[i];
			list_add(&frmbuf_info->frmbuf_list, &rtkvdec_frame_buf_head);

			frm_mm_ptr->memChAddr[i]    = htonl((unsigned int)frmbuf_info->phy_addr);
			frm_mm_ptr->memChHdrAddr[i] = htonl((unsigned int)frmbuf_info->hdr_phy_addr);
		}

		frm_mm_ptr->memCheck = htonl(VBM_CHECK_TYPE_ALLOC_FINISH);
	}
	else {
		rtd_pr_vdec_err("[%s %d] unknown type %d\n", __func__, __LINE__, type);
	}

	up(&rtkvdec_frame_buf_sem);
	return -1;

ERR_CASE_HDR_FAIL :
	for ( i = 0; i < cnt; i++ ) {
		if ( LuHdrPA[i] != 0 && LuHdrPA[i] != INVALID_VAL ) { pli_free(LuHdrPA[i]); LuHdrPA[i] = 0; }
		if ( ChHdrPA[i] != 0 && ChHdrPA[i] != INVALID_VAL ) { pli_free(ChHdrPA[i]); ChHdrPA[i] = 0; }
	}

ERR_CASE_CNT_NOT_MATCH :
	/* cnt isn't match, need to free luma and chroma */
	if ( dmabuf_ChPtr ) {
		get_Chcnt = dvr_dmabuf_to_list(dmabuf_ChPtr, dmabuf_ChArray, cnt);
		for ( i = 0; i < get_Chcnt; i++ ) {
			dvr_dmabuf_free((struct dma_buf *)dmabuf_ChArray[i]);
			dmabuf_ChArray[i] = 0;
		}
	}

ERR_CASE_CHROMA_FAIL :
	/* allocate Chroma fail, need to free luma */
	if ( dmabuf_LuPtr ) {
		get_Lucnt = dvr_dmabuf_to_list(dmabuf_LuPtr, dmabuf_LuArray, cnt);
		for ( i = 0; i < get_Lucnt; i++ ) {
			dvr_dmabuf_free((struct dma_buf *)dmabuf_LuArray[i]);
			dmabuf_LuArray[i] = 0;
		}
	}

ERR_CASE_LUMA_FAIL :
	frm_mm_ptr->memCheck = htonl(VBM_CHECK_TYPE_ALLOC_FAIL);
	up(&rtkvdec_frame_buf_sem);
	rtkvdec_vbm_by_frame_summary();
	return -1;
}


unsigned long rtkvdec_get_decimate_addr (void)
{
	return VBM_start + 320 * 1024 * 1024;
}


void rtkvdec_vbm_by_frame_testcase_alloc (int NumofUse, int type, int size, int hdrsize, int cnt)
{
	VDEC_SHARE_MEM_STRUCT *gpShmem;
	struct vbm_frm_mm *frm_mm_ptr = NULL;
	unsigned int i;

	if ( !SharedMemUNVirtAddr ) {
		rtd_pr_vdec_err("[%s %d] no shared memory.\n", __func__, __LINE__);
		return;
	}

	gpShmem = (VDEC_SHARE_MEM_STRUCT *)SharedMemUNVirtAddr;
	frm_mm_ptr = &(gpShmem->frm_mm[NumofUse]);

	frm_mm_ptr->memType    = htonl(type);
	frm_mm_ptr->memLuSize  = htonl(size);
	frm_mm_ptr->memChSize  = htonl(size>>1);
	frm_mm_ptr->memHdrSize = htonl(hdrsize);
	frm_mm_ptr->memCnt     = htonl(cnt);
	frm_mm_ptr->memCheck   = htonl(VBM_CHECK_TYPE_ALLOC_START);
	rtkvdec_vbm_alloc_by_frame(NumofUse);
	rtkvdec_vbm_by_frame_summary();

	for ( i = 0; i < FRAME_ARRAY_SIZE; i++ ) {
		rtd_pr_vdec_err("%d: lu %x %x ch %x %x\n", i
			, ntohl(frm_mm_ptr->memLuAddr[i]), ntohl(frm_mm_ptr->memLuHdrAddr[i])
			, ntohl(frm_mm_ptr->memChAddr[i]), ntohl(frm_mm_ptr->memChHdrAddr[i]));
	}
}


void rtkvdec_vbm_by_frame_testcase_free (int NumofUse)
{
	VDEC_SHARE_MEM_STRUCT *gpShmem;
	struct vbm_frm_mm *frm_mm_ptr = NULL;
	unsigned int i;

	if ( !SharedMemUNVirtAddr ) {
		rtd_pr_vdec_err("[%s %d] no shared memory.\n", __func__, __LINE__);
		return;
	}

	gpShmem = (VDEC_SHARE_MEM_STRUCT *)SharedMemUNVirtAddr;
	frm_mm_ptr = &(gpShmem->frm_mm[NumofUse]);

	for ( i = 0; i < FRAME_ARRAY_SIZE; i++ ) {
		frm_mm_ptr->memFreeLuAddr[i] = frm_mm_ptr->memLuAddr[i];
		frm_mm_ptr->memFreeLuHdrAddr[i] = frm_mm_ptr->memLuHdrAddr[i];
		frm_mm_ptr->memFreeChAddr[i] = frm_mm_ptr->memChAddr[i];
		frm_mm_ptr->memFreeChHdrAddr[i] = frm_mm_ptr->memChHdrAddr[i];
	}
	frm_mm_ptr->memCheck   = htonl(VBM_CHECK_TYPE_FREE_START);
	rtkvdec_vbm_free_by_frame(1);
	rtkvdec_vbm_by_frame_summary();
}


void rtkvdec_vbm_by_frame_testcase1 ()
{
	rtd_pr_vdec_err("allocate 4K start\n");
	rtkvdec_vbm_by_frame_testcase_alloc(0, VBM_REQ_TYPE_4K, 3072*2176, 0x4C800, 10);
	rtd_pr_vdec_err("allocate 2K start\n");
	rtkvdec_vbm_by_frame_testcase_alloc(1, VBM_REQ_TYPE_2K, 1280*720 , 0x8700 , 10);
	rtd_pr_vdec_err("free 4K start\n");
	rtkvdec_vbm_by_frame_testcase_free(0);
	rtd_pr_vdec_err("free 2K start\n");
	rtkvdec_vbm_by_frame_testcase_free(1);
	rtd_pr_vdec_err("testcase1 done\n");
}


void rtkvdec_vbm_by_frame_testcase2 ()
{
	rtd_pr_vdec_err("allocate 1st 2K start\n");
	rtkvdec_vbm_by_frame_testcase_alloc(0, VBM_REQ_TYPE_2K, 1280*720 , 0x8700 , 10);
	rtd_pr_vdec_err("allocate 2nd 2K start\n");
	rtkvdec_vbm_by_frame_testcase_alloc(1, VBM_REQ_TYPE_2K, 1280*720 , 0x8700 , 10);
	rtd_pr_vdec_err("allocate 3rd 2K start\n");
	rtkvdec_vbm_by_frame_testcase_alloc(2, VBM_REQ_TYPE_2K, 1280*720 , 0x8700 , 10);
	rtd_pr_vdec_err("allocate 4th 2K start\n");
	rtkvdec_vbm_by_frame_testcase_alloc(3, VBM_REQ_TYPE_2K, 1280*720 , 0x8700 , 10);
	rtd_pr_vdec_err("free 1st 2K start\n");
	rtkvdec_vbm_by_frame_testcase_free(0);
	rtd_pr_vdec_err("free 2nd 2K start\n");
	rtkvdec_vbm_by_frame_testcase_free(1);
	rtd_pr_vdec_err("free 3rd 2K start\n");
	rtkvdec_vbm_by_frame_testcase_free(2);
	rtd_pr_vdec_err("free 4th 2K start\n");
	rtkvdec_vbm_by_frame_testcase_free(3);
	rtd_pr_vdec_err("testcase1 done\n");
}
#endif
