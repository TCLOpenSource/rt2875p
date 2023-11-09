#include "rtk_vdec_struct.h"

static struct task_struct *rtkvdec_debug_check_tsk;
static int rtkvdec_debug_check_data;
static int rtkvdec_debug_check_thread(void *arg);

/* for qos mode */
unsigned int is_8k = 0 ;


/* for debug : shared memory */
unsigned long SharedMemUNVirtAddr ;
unsigned long SharedMemPhyAddr ;
unsigned long SharedMemVirtAddr ;
unsigned long vcpu1_is_crash = 0 ;

#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
/* for debug : vcpu2 shared memory */
unsigned long SharedMemUNVirtAddr_V2 ;
unsigned long SharedMemPhyAddr_V2 ;
unsigned long SharedMemVirtAddr_V2 ;
#endif

/* for dma buf use */
extern unsigned long rtkvdec_dma_array_pa;

static int rtkvdec_debug_check_thread(void *arg)
{
	VDEC_SHARE_MEM_STRUCT *gpShmem;

	if ( !SharedMemUNVirtAddr ) {
		rtd_pr_vdec_err("[%s %d] no shared memory.\n", __func__, __LINE__);
		return 0;
	}

	gpShmem = (VDEC_SHARE_MEM_STRUCT *)SharedMemUNVirtAddr;

	for (;;) {
		set_freezable();

		if ( kthread_should_stop() ) break;
		if ( !vcpu1_is_crash ) {
			if ( gpShmem->video_is_crash ) {
				vcpu1_is_crash = 1 ;
				/* print message */
			}
		}

		msleep(10); /* sleep 10 ms */
	}
	return 0;
}


void rtkvdec_send_sharedmemory()
{
	unsigned long return_value ;

	SharedMemVirtAddr = (unsigned long)dvr_malloc_uncached_specific(sizeof(VDEC_SHARE_MEM_STRUCT), GFP_DCU1_LIMIT, (void **)(&SharedMemUNVirtAddr));
	if (!SharedMemVirtAddr) {
		rtd_pr_vdec_err("[%s %d]alloc sharedmemory fail\n",__FUNCTION__,__LINE__);
		return;
	}
	SharedMemPhyAddr = (unsigned long)dvr_to_phys((void *)SharedMemVirtAddr);

	rtkvdec_memset((void *)SharedMemUNVirtAddr, '\0', sizeof(VDEC_SHARE_MEM_STRUCT));

	rtd_pr_vdec_notice("rtkvdec: sharedmemory %lx %lx %lx\n", SharedMemVirtAddr, SharedMemUNVirtAddr, SharedMemPhyAddr);

	if ( send_rpc_command(RPC_VIDEO, RPC_VCPU_SET_SHARED_MEMORY, SharedMemPhyAddr, 0, &return_value) )
		rtd_pr_vdec_debug("rtkvdec: RPC_VCPU_SET_SHARED_MEMORY fail %ld\n", return_value);


#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
	SharedMemVirtAddr_V2 = (unsigned long)dvr_malloc_uncached_specific(sizeof(VDEC_SHARE_MEM_STRUCT), GFP_DCU1_LIMIT, (void **)(&SharedMemUNVirtAddr_V2));
	if (!SharedMemVirtAddr_V2) {
		rtd_pr_vdec_err("[%s %d]alloc sharedmemory fail\n",__FUNCTION__,__LINE__);
		return;
	}
	SharedMemPhyAddr_V2 = (unsigned long)dvr_to_phys((void *)SharedMemVirtAddr_V2);

	rtkvdec_memset((void *)SharedMemUNVirtAddr_V2, '\0', sizeof(VDEC_SHARE_MEM_STRUCT));

	rtd_pr_vdec_notice("rtkvdec: sharedmemory %lx %lx %lx\n", SharedMemVirtAddr_V2, SharedMemUNVirtAddr_V2, SharedMemPhyAddr_V2);

	if ( send_rpc_command(RPC_VIDEO2, RPC_VCPU_SET_SHARED_MEMORY, SharedMemPhyAddr_V2, 0, &return_value) )
		rtd_pr_vdec_debug("rtkvdec: RPC_VCPU_SET_SHARED_MEMORY fail %ld\n", return_value);
#endif

	rtkvdec_debug_check_tsk = kthread_create(rtkvdec_debug_check_thread, &rtkvdec_debug_check_data, "rtkvdec_debug_check_thread");
	if ( IS_ERR(rtkvdec_debug_check_tsk) ) {
		rtkvdec_debug_check_tsk = NULL;
		return ;
	}

	wake_up_process(rtkvdec_debug_check_tsk);

	return;
}


void rtkvdec_show_sharedmemory(void)
{
	unsigned long UNVirtAddr ;
	unsigned long PhyAddr ;
	unsigned long VirtAddr ;
	VDEC_SHARE_MEM_STRUCT *gpShmem ;
	int i;

	VirtAddr = (unsigned long)dvr_malloc_uncached_specific(sizeof(VDEC_SHARE_MEM_STRUCT), GFP_DCU2, (void **)(&UNVirtAddr));
	if (!VirtAddr) {
		rtd_pr_vdec_err("[%s %d]alloc sharedmemory fail\n",__FUNCTION__,__LINE__);
		return;
	}
	PhyAddr = (unsigned long)dvr_to_phys((void *)VirtAddr);
	gpShmem = (VDEC_SHARE_MEM_STRUCT *)UNVirtAddr;

	rtd_pr_vdec_err("rtkvdec_user_alloc_summary %ld\n", vcpu1_is_crash);

	if ( !SharedMemUNVirtAddr ) goto FREE_MEMORY ;

	rtkvdec_memcpy((void *)gpShmem, (void *)SharedMemUNVirtAddr, sizeof(VDEC_SHARE_MEM_STRUCT));

	rtd_pr_vdec_err("record VCPU1 isr :\n");
	rtd_pr_vdec_err("last %d\n", ntohl(gpShmem->now));
	for ( i = 0; i < 10; i++ ) {
		rtd_pr_vdec_err("%d: v%d %x/%x, %x/%x, %x/%x/%x\n"
			, i, ntohl(gpShmem->isr_vector[i]), ntohl(gpShmem->isr[i]), ntohl(gpShmem->isr_done[i])
			, ntohl(gpShmem->isr_in_time[i]), ntohl(gpShmem->isr_out_time[i])
			, ntohl(gpShmem->epc[i]), ntohl(gpShmem->isr_in_sb2[i]), ntohl(gpShmem->isr_out_sb2[i]));
	}

#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
	if ( !SharedMemUNVirtAddr_V2 ) goto FREE_MEMORY ;

	rtkvdec_memcpy((void *)gpShmem, (void *)SharedMemUNVirtAddr_V2, sizeof(VDEC_SHARE_MEM_STRUCT));

	rtd_pr_vdec_err("record VCPU2 isr :\n");
	rtd_pr_vdec_err("last %d\n", ntohl(gpShmem->now));
	for (i = 0; i < 10; i++) {
		rtd_pr_vdec_err("%d: v%d %x/%x, %x/%x, %x/%x/%x\n"
			, i, ntohl(gpShmem->isr_vector[i]), ntohl(gpShmem->isr[i]), ntohl(gpShmem->isr_done[i])
			, ntohl(gpShmem->isr_in_time[i]), ntohl(gpShmem->isr_out_time[i])
			, ntohl(gpShmem->epc[i]), ntohl(gpShmem->isr_in_sb2[i]), ntohl(gpShmem->isr_out_sb2[i]));
	}
#endif

FREE_MEMORY :
	dvr_free((void *)VirtAddr);
}
EXPORT_SYMBOL_GPL(rtkvdec_show_sharedmemory);


void rtkvdec_show_calltrace(void)
{
	VDEC_SHARE_MEM_STRUCT *gpShmem;

	if ( !SharedMemUNVirtAddr ) {
		rtd_pr_vdec_err("[%s %d] no shared memory.\n", __func__, __LINE__);
		return ;
	}

	gpShmem = (VDEC_SHARE_MEM_STRUCT *)SharedMemUNVirtAddr ;
	gpShmem->show_ct_no_crash = 1 ;
	rtd_outl(SB2_INV_INTSTAT_reg, 0x5); //SB2 Vint
	/* rtd_pr_vdec_notice("rtkvdec_show_calltrace %x %x\n", show_shmem->show_ct_no_crash, *((volatile unsigned int *) 0xB801A008)); */
}


extern int IO_Direct_Set(char *enum_name, int value);
extern bool rtk_otp_field_read_by_name(const char *name, unsigned char *data, unsigned int len);
extern int rtk_get_iddq_core_power(void);

void rtkvdec_setqos(void)
{
	unsigned int package_value;

	if ( rtk_otp_field_read_by_name("hw_cpu_package_size", (unsigned char*)&package_value, sizeof(package_value)) != true )
		panic("%s: read hw_cpu_package_size otp fail\n", __func__);
	else
		rtd_pr_vdec_info("%s: read hw_cpu_package_size otp(%d)\n", __func__, package_value);

	if ( package_value == PACKAGE_BGA25_22_7 ) {
		if ( is_8k ) {
			IO_Direct_Set("PIN_CORE_VID_0", 1);
			IO_Direct_Set("PIN_CORE_VID_1", 0);
			udelay(50);

			rtd_outl(PLL_REG_SYS_PLL_VCPU_1_reg,0x0013d01a);
		}
		else {
			if ( rtk_get_iddq_core_power() >= 75 ) { /* IDDQ >= 75mA */
				IO_Direct_Set("PIN_CORE_VID_1", 0);
				IO_Direct_Set("PIN_CORE_VID_0", 0);
				udelay(50);

				rtd_outl(PLL_REG_SYS_PLL_VCPU_1_reg,0x0012e01a);
			}
			else { /* IDDQ < 75mA */
				IO_Direct_Set("PIN_CORE_VID_1", 1);
				IO_Direct_Set("PIN_CORE_VID_0", 0);
				udelay(50);

				rtd_outl(PLL_REG_SYS_PLL_VCPU_1_reg,0x0012e01a);
			}
		}
	}
}


unsigned long rtkvdec_set_command(unsigned long para1, unsigned long para2)
{
	rtd_pr_vdec_err("rtkvdec_set_command(%lx/%lx)\n", para1, para2);
	if (para1 == RTKVDEC_STECMD_TYPE_8KDEC) {
		if (para2 == 1) {
			is_8k = 1;
		}
		else {
			is_8k = 0;
		}
		rtkvdec_setqos();
	}
	return 0;
}


extern int sb2_dbg_monitor (sb2_dbg_param *pParam);
extern int sb2_dbg_clear (sb2_dbg_param *pParam);
void memory_debug_protect(unsigned int idx, unsigned int addr, unsigned int size)
{
	sb2_dbg_param sb2_dbg;

	if (idx >= IBBUF_DEBUG_SB2_CNT) {
		return;
	}

	rtkvdec_memset((void *)&sb2_dbg, 0, sizeof(sb2_dbg_param));
	sb2_dbg.set_id = IBBUF_DEBUG_SB2_ID + idx;
	sb2_dbg.cpu_flag[0] = 1;
	sb2_dbg.cpu_flag[1] = 1;
	sb2_dbg.cpu_flag[2] = 1;
	sb2_dbg.cpu_flag[3] = 1;
	sb2_dbg.cpu_flag[4] = 1;
	sb2_dbg.cpu_flag[5] = 1;
	sb2_dbg.cpu_flag[6] = 1;
	sb2_dbg.start = addr;
	sb2_dbg.end = addr + size - 1;
	sb2_dbg.operation = 0;
	sb2_dbg.rw = 0; /* RW_FLAG_RW */
	sb2_dbg.str = 0; /* STR_FLAG_DFT == STR_FLAG_YES */
#ifdef RTK_KDRV_SB2
	sb2_dbg_monitor(&sb2_dbg);
#endif
}

void memory_debug_unprotect(unsigned int idx)
{
	sb2_dbg_param sb2_dbg;

	if (idx >= IBBUF_DEBUG_SB2_CNT) {
		return;
	}

	rtkvdec_memset((void *)&sb2_dbg, 0, sizeof(sb2_dbg_param));
	sb2_dbg.set_id = IBBUF_DEBUG_SB2_ID + idx;
	sb2_dbg.str = 0; /* STR_FLAG_DFT == STR_FLAG_YES */
#ifdef RTK_KDRV_SB2
	sb2_dbg_clear(&sb2_dbg);
#endif
}


int rtkvdec_query_num_of_vcpu(void)
{
	unsigned long ret;
#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU3
	ret = 3;
#else
	ret = 2;
#endif
#else
	ret = 1;
#endif
	rtd_pr_vdec_err("[%s %d] num_of_vcpu %d\n",__FUNCTION__,__LINE__, (int)ret);
	return ret;
}
