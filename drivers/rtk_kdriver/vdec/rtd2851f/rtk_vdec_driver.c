#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/uaccess.h>
#include <asm/cacheflush.h>
#include <linux/string.h>
#include <linux/proc_fs.h>

#include <rbus/cabac_p2_reg.h>
#include <rbus/codeblkreg_reg.h>
#include <rbus/dc_sys_reg.h>
#include <rbus/seg_reg.h>
#include <rbus/mvdblk_reg.h>
#include <rbus/rl_reg.h>
#include <rbus/predblk_reg.h>
#include <rbus/deblk_reg.h>
#include <rbus/sao_reg.h>
#include <rbus/cmprs_reg.h>

#include "rtk_vdec_internal.h"
#include "rtk_vdec_struct.h"
#include "rtk_vdec_dmabuf.h"
#include <mach/system.h>
#include <linux/dma-mapping.h> /* dma_alloc_coherent */
#include <linux/err.h> /* IS_ERR() PTR_ERR() */
#include <linux/time.h>
#include <linux/auth.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#include <linux/sched/signal.h>
#endif
#ifdef CONFIG_LG_SNAPSHOT_BOOT
#include <linux/suspend.h>
#endif
#include <rtk_dc_mt.h>

#if 0
/*[WOSQRTK-4745]fix logo garbage.*/
#include <rtk_kdriver/gal/rtk_gdma_export.h>
/*[WOSQRTK-6662]fix logo dealy time.*/
#include "tvscalercontrol/panel/panelapi.h"
bool bstoplogo_gdma = false;
extern int is_RTD_K2L(void);
#endif

#ifdef CONFIG_HIBERNATION
static int rtkvdec_suspend(struct device *dev);
static int rtkvdec_resume(struct device *dev);
#endif

#ifdef CONFIG_LG_SNAPSHOT_BOOT
extern unsigned int snapshot_enable;
#endif

static BLOCKING_NOTIFIER_HEAD(rtkvdec_chain_head);

static struct device *rtkvdec_device;
static struct class *rtkvdec_class = NULL;
static struct cdev rtkvdec_cdev;
static struct platform_device *rtkvdec_platform_devs = NULL;

static const struct dev_pm_ops rtkvdec_pm_ops = {
#ifdef CONFIG_HIBERNATION
	.freeze 	= rtkvdec_suspend,
	.thaw   	= rtkvdec_resume,
	.poweroff       = rtkvdec_suspend,
	.restore	= rtkvdec_resume,
#endif
};

static struct platform_driver rtkvdec_device_driver = {
	.driver = {
		.name   = "rtkvdec",
		.bus    = &platform_bus_type,
#ifdef CONFIG_PM
		.pm     = &rtkvdec_pm_ops,
#endif
	},
};

int rtkvdec_major =   RTKVDEC_MAJOR;
int rtkvdec_minor =   0;
int VideoCreatDone  = 0 ;
int VideoCreatDone2 = 0 ;
int VideoCreatFinish = 0 ;

unsigned long EncoderMemAddr  = 0;
unsigned int EncoderMemState = 0;
extern unsigned int notify_rtkvdec_release_pov ;

/* for record video fw remote malloc memory info */
static struct list_head vfw_remote_malloc_info_head;
static DEFINE_SEMAPHORE(vfw_buffer_list_sem);
static struct list_head vfw_remote_malloc_info_specific_head;
static DEFINE_SEMAPHORE(vfw_buffer_list_specific_sem);

/* for debug : dump video f/w log */
unsigned long g_pVideoDebugMemory[2] = {0,0} ;
unsigned long g_pVideoDumpMemory[2] = {0,0} ;
VDEC_DBG_STRUCT dbg_config[2];
struct file *file_videoDump[2] = {0,0} ;
unsigned long debugMemPhyAddr[2] ;
unsigned long debugMemVirtAddr[2] ;
unsigned long g_videoDumpMemPhyAddr[2] ;
unsigned long g_videoDumpMemVirtAddr[2] ;

static struct task_struct *rtkvdec_debug_tsk;
static int data;
static int rtkvdec_debug_thread(void *arg);

static unsigned long long f_offset[2] = {0,0} ;
#define RTKVDEC_DUMP_FILE_MAX_SIZE (1024*1024)
int rtkvdec_dump_size_scale;
int rtkvdec_pr_to_dmesg_en;
#define DMESG_PRINT_MAX (1024)

/* for debug : dump es to file */
VDEC_DVR_MALLOC_STRUCT g_pDumpES_Send ;
VDEC_DVR_MALLOC_STRUCT g_pDumpES ;
VDEC_DBG_STRUCT DumpES_Config;
struct file *file_DumpES = 0 ;
static struct task_struct *rtkvdec_dumpes_tsk;
static int rtkvdec_dumpes_thread(void *arg);
static unsigned long long f_offset_dumpes = 0 ;

/* for send video create rpc */
static struct task_struct *rtkvdec_vcreate_tsk = NULL;
static int data2;
static int rtkvdec_vcreate_thread(void *arg);

/* for gstreamer frame info */
static struct list_head gst_frame_info_head;
static DEFINE_SEMAPHORE(gst_frame_info_list_sem);

/* for record rtkvdec alloc info */
static struct list_head rtkvdec_alloc_head;
static DEFINE_SEMAPHORE(rtkvdec_alloc_sem);

#if 0
//set tvb4 prority for OSD2 [WOSQRTK-4745]fix logo garbage
unsigned int org_tvb4_pro_1=0;
unsigned int org_tvb4_pro_2=0;
#define TVSB4_PRIORITY_VALUE 0x00000010
#include <rbus/rbus-DesignSpec-DC_SYSReg.h>
#include <rbus/rbus-DesignSpec-DC2_SYSReg.h>
#endif


int register_rtkvdec_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&rtkvdec_chain_head, nb);
}
EXPORT_SYMBOL_GPL(register_rtkvdec_notifier);

int unregister_rtkvdec_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&rtkvdec_chain_head, nb);
}
EXPORT_SYMBOL_GPL(unregister_rtkvdec_notifier);

int rtkvdec_notifier_call_chain(unsigned long val)
{
	int ret;

	ret = blocking_notifier_call_chain(&rtkvdec_chain_head, val, NULL);
	if (ret == NOTIFY_BAD)
		ret = -EINVAL;
	else
		ret = 0;

	return ret;
}

#ifdef CONFIG_HIBERNATION
static int rtkvdec_suspend(struct device *dev)
{
	rtkvdec_notifier_call_chain(RTKVDEC_SUSPEND);
	rtkvdec_notifier_call_chain(RTKVDEC_RESET_PREPARE);
	rtd_pr_vdec_info("rtkvdec: receive supend command\n");
	/* todo resetav_lock(1); */

	return 0;
}

static int rtkvdec_resume(struct device *dev)
{
	rtd_pr_vdec_err("Before rtkvdec_resume\n");
	device_pm_wait_for_dev(dev, get_rpc_device_ptr());

	/* todo resetav_unlock(1); */
	rtd_pr_vdec_info("rtkvdec: receive resume command\n");
	rtkvdec_notifier_call_chain(RTKVDEC_RESET_DONE);
	rtkvdec_notifier_call_chain(RTKVDEC_RESUME);

	if(rtkvdec_vcreate_tsk != NULL)
		wake_up_process(rtkvdec_vcreate_tsk);

	return 0;
}
#endif

void rtkvdec_fw_remote_malloc_summary(ret_info_t *info) {
	unsigned long sum_request, sum_malloc;
	struct list_head *list;
	buffer_info_t *buf_info;
	memory_info_t *mem_info;

	sum_malloc = 0;
	sum_request = 0;

	rtkvdec_show_vbm_status();

	rtd_pr_vdec_notice("\n======= rtkvdec_remote_malloc =======\n");
	down(&vfw_buffer_list_sem);
	list_for_each(list, &vfw_remote_malloc_info_head) {
		buf_info = list_entry(list, buffer_info_t, buffer_list);
		rtd_pr_vdec_notice("vir_addr %lx, phy_addr %lx, request_size %lx, malloc_size %lx\n", buf_info->vir_addr, buf_info->phy_addr, buf_info->request_size, buf_info->malloc_size);
		sum_request += buf_info->request_size;
		sum_malloc += buf_info->malloc_size;
	}
	up(&vfw_buffer_list_sem);

	rtd_pr_vdec_notice("\nsum_malloc %ld, sum_request %ld\n", sum_malloc, sum_request);

	rtd_pr_vdec_err("\n=== rtkvdec_remote_malloc_specific ===\n");
	down(&vfw_buffer_list_specific_sem);
	list_for_each(list, &vfw_remote_malloc_info_specific_head) {
		mem_info = list_entry(list, memory_info_t, memory_list);
		if (mem_info->dcu == RTKVDEC_DCU1)
			rtd_pr_vdec_err("DCU1       : phy_addr %lx, alloc_size %lx\n", mem_info->phy_addr, mem_info->malloc_size);
		if (mem_info->dcu == RTKVDEC_DCU1_FIRST)
			rtd_pr_vdec_err("DCU1_FIRST : phy_addr %lx, alloc_size %lx\n", mem_info->phy_addr, mem_info->malloc_size);
		if (mem_info->dcu == RTKVDEC_DCU1_LIMIT)
			rtd_pr_vdec_err("DCU1_LIMIT : phy_addr %lx, alloc_size %lx\n", mem_info->phy_addr, mem_info->malloc_size);
		if (mem_info->dcu == RTKVDEC_DCU2)
			rtd_pr_vdec_err("DCU2       : phy_addr %lx, alloc_size %lx\n", mem_info->phy_addr, mem_info->malloc_size);
		if (mem_info->dcu == RTKVDEC_DCU2_FIRST)
			rtd_pr_vdec_err("DCU2_FIRST : phy_addr %lx, alloc_size %lx\n", mem_info->phy_addr, mem_info->malloc_size);
	}
	up(&vfw_buffer_list_specific_sem);

	rtkvdec_show_vmm_status();

	rtd_pr_vdec_notice("\n=========== EncoderMemAddr ===========\n");
	rtd_pr_vdec_notice("phy_addr %lx, state %d\n", EncoderMemAddr, EncoderMemState);

	if (info == NULL) return;

	info->private_info[0] = sum_request;
	info->private_info[1] = sum_malloc;
}

unsigned long rtkvdec_remote_malloc(unsigned long para1, unsigned long para2)
{
	unsigned long phy_addr = 0 ;
	buffer_info_t *buf_info;

	rtd_pr_vdec_debug("Inside rtkvdec_remote_malloc(%x/%x)\n", (unsigned int)para1, (unsigned int)para2);

#ifndef CONFIG_VBM_HEAP
	if (para1 & 0x80000000) {
		return rtkvdec_vbm_alloc(para1 & 0x7fffffff);
	}
	else
#endif
		phy_addr = pli_malloc(para1 & 0x7ffffff, GFP_DCU1_LIMIT);

	if (phy_addr == INVALID_VAL) {
		rtd_pr_vdec_err("[%s %d]alloc memory fail\n", __func__, __LINE__);
		return 0;
	}

	buf_info = (buffer_info_t *)kmalloc(sizeof(buffer_info_t), GFP_KERNEL);
	INIT_LIST_HEAD(&buf_info->buffer_list);

	buf_info->vir_addr = 0;
	buf_info->phy_addr = phy_addr;
	buf_info->request_size = para1;
	buf_info->malloc_size = PAGE_ALIGN(para1);

	rtd_pr_vdec_debug("dvr_malloc_uncached_specific addr %lx\n", phy_addr);

	down(&vfw_buffer_list_sem);
	list_add(&buf_info->buffer_list, &vfw_remote_malloc_info_head);
	up(&vfw_buffer_list_sem);

	return phy_addr;
}

unsigned long rtkvdec_remote_free(unsigned long para1, unsigned long para2)
{
	struct list_head *del_list;
	buffer_info_t *buf_info;
	int free_OK = 0 ;

	rtd_pr_vdec_debug("Inside rtkvdec_remote_free(%x/%x)\n", (unsigned int)para1, (unsigned int)para2);

#ifndef CONFIG_VBM_HEAP
	if ( 0 == rtkvdec_vbm_free(para1) )
		return 0;
#endif

	down(&vfw_buffer_list_sem);
	list_for_each(del_list, &vfw_remote_malloc_info_head) {
		buf_info = list_entry(del_list, buffer_info_t, buffer_list);
		if (buf_info->phy_addr == para1) {
			list_del(del_list);
			kfree(buf_info);
			pli_free(para1);
			free_OK = 1 ;
			break;
		}
	}
	up(&vfw_buffer_list_sem);

	if (!free_OK) {
		rtd_pr_vdec_err("rtkvdec_remote_free : not found!!!(%x)\n", (unsigned int)para1);
		rtkvdec_fw_remote_malloc_summary(NULL);
	}

	return 0;
}

unsigned long rtkvdec_remote_malloc_specific(unsigned long para1, unsigned long para2)
{
	unsigned long phy_addr = 0;
	memory_info_t *mem_info;
	unsigned int DCU = 0;

	rtd_pr_vdec_debug("Inside rtkvdec_remote_malloc_specific(%x/%x)\n", (unsigned int)para1, (unsigned int)para2);

	if (para1 == RTKVDEC_VCREATE_ALLOC) {
		return rtkvdec_vmm_alloc((para2 + 0x3FF) & ~0x3FF);
	}

	if (para1 == RTKVDEC_ENCODER_ALLOC) {
		if (para2 <= 0x80000) {
			if (EncoderMemAddr && EncoderMemState == 0) {
				phy_addr = EncoderMemAddr;
				EncoderMemState = 1;
				return phy_addr;
			}
		}
		rtd_pr_vdec_err("[%s %d]alloc encoder memory fail %x\n", __func__, __LINE__, (unsigned int)para2);
		return 0 ;
	}

	if (para1 == RTKVDEC_DCU1) {
		phy_addr = pli_malloc(para2, GFP_DCU1);
		if (phy_addr == INVALID_VAL) {
			rtd_pr_vdec_warn("[%s %d]alloc DCU1 memory fail\n", __func__, __LINE__);
			return 0;
		}
		DCU = RTKVDEC_DCU1 ;
	}
	else if (para1 == RTKVDEC_DCU1_FIRST) {
		phy_addr = pli_malloc(para2, GFP_DCU1_FIRST);
		if (phy_addr == INVALID_VAL) {
			rtd_pr_vdec_err("[%s %d]alloc DCU1_FIRST memory fail\n", __func__, __LINE__);
			return 0;
		}
		DCU = RTKVDEC_DCU1_FIRST ;
	}
	else if (para1 == RTKVDEC_DCU2) {
#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
		phy_addr = pli_malloc(para2, GFP_HIGHMEM_BW_FIRST);
#else
		phy_addr = pli_malloc(para2, GFP_DCU2);
#endif
		if (phy_addr == INVALID_VAL) {
			rtd_pr_vdec_err("[%s %d]alloc DCU2 memory fail\n", __func__, __LINE__);
			return 0;
		}
		DCU = RTKVDEC_DCU2 ;
	}
	else if (para1 == RTKVDEC_DCU2_FIRST) {
#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
		phy_addr = pli_malloc(para2, GFP_HIGHMEM_BW_FIRST);
#else
		phy_addr = pli_malloc(para2, GFP_DCU2_FIRST);
#endif
		if (phy_addr == INVALID_VAL) {
			rtd_pr_vdec_warn("[%s %d]alloc DCU2_FIRST memory fail\n", __func__, __LINE__);
			return 0;
		}
		DCU = RTKVDEC_DCU2_FIRST ;
	}
	else if (para1 == RTKVDEC_DCU1_LIMIT) {
		phy_addr = pli_malloc(para2, GFP_DCU1_LIMIT);
		if (phy_addr == INVALID_VAL) {
			rtd_pr_vdec_warn("[%s %d]alloc DCU1_LIMIT memory fail\n", __func__, __LINE__);
			return 0;
		}
		DCU = RTKVDEC_DCU1_LIMIT ;
	}
#ifdef CONFIG_VBM_HEAP
	else if (para1 == RTKVDEC_VBM_ALLOC) {
		if ( para2 >= 1024 * 1024 )
			return rtkvdec_vbm_alloc_by_chunk(para2, false);
		else
			return rtkvdec_vbm_alloc_by_frame(para2);
	}
	else if (para1 == RTKVDEC_VBM_DECIMATE) {
			return rtkvdec_vbm_alloc_by_chunk(para2, true);
	}
	else if (para1 == RTKVDEC_DECIMATE_LUMA) {
		return rtkvdec_get_decimate_addr();
	}
#endif
	else if (para1 == RTKVDEC_VE_SRAM) {
		return rtkvdec_borrow_ve_sram();
	}
	else {
		rtd_pr_vdec_err("[%s %d]unknown type (%x/%x)n", __func__, __LINE__, (unsigned int)para1, (unsigned int)para2);
		return 0;
	}

	mem_info = (memory_info_t *)kmalloc(sizeof(memory_info_t), GFP_KERNEL);
	if (!mem_info) {
		rtd_pr_vdec_err("[%s %d] ERR : failed to kmalloc\n", __func__, __LINE__);
		BUG();
		return 0;
	}
	INIT_LIST_HEAD(&mem_info->memory_list);

	mem_info->dcu = DCU;
	mem_info->phy_addr = phy_addr;
	mem_info->request_size = para2;
	mem_info->malloc_size = PAGE_ALIGN(para2);

	rtd_pr_vdec_debug("pli_malloc addr %lx\n", phy_addr);

	down(&vfw_buffer_list_specific_sem);
	list_add(&mem_info->memory_list, &vfw_remote_malloc_info_specific_head);
	up(&vfw_buffer_list_specific_sem);

	return phy_addr;
}

unsigned long rtkvdec_remote_free_specific(unsigned long para1, unsigned long para2)
{
	struct list_head *del_list;
	memory_info_t *mem_info;
	int free_OK = 0 ;

	rtd_pr_vdec_debug("Inside rtkvdec_remote_free(%x/%x)\n", (unsigned int)para1, (unsigned int)para2);

#ifdef CONFIG_VBM_HEAP
	if ( para1 == RTKVDEC_VBM_FREE ) {
		rtkvdec_vbm_free_by_frame(1);
#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
		rtkvdec_vbm_free_by_frame(2);
#endif
		return 0;
	}
	rtkvdec_vbm_free_by_chunk(para1);
#endif

	if ( para1 == RTKVDEC_VE_SRAM ) {
		rtkvdec_return_ve_sram();
		return 0;
	}

	if ( 0 == rtkvdec_vmm_free(para1) )
		return 0;

	if (para1 == EncoderMemAddr) {
		EncoderMemState = 0;
		return 0;
	}

	down(&vfw_buffer_list_specific_sem);
	list_for_each(del_list, &vfw_remote_malloc_info_specific_head) {
		mem_info = list_entry(del_list, memory_info_t, memory_list);
		if (mem_info->phy_addr == para1) {
			list_del(del_list);
			pli_free(para1);
			kfree(mem_info);
			free_OK = 1 ;
			break;
		}
	}
	up(&vfw_buffer_list_specific_sem);

	if (!free_OK) {
		rtd_pr_vdec_err("rtkvdec_remote_free_specific : not found!!!(%x)\n", (unsigned int)para1);
		rtkvdec_fw_remote_malloc_summary(NULL);
	}

	return 0;
}


static void rtkvdec_print_to_dmesg(unsigned char *ptr, uint count)
{
	unsigned char *dmgp = NULL, *temp_ptr;
	char *tok;
	uint print_count;

	dmgp = kmalloc(DMESG_PRINT_MAX, GFP_KERNEL);
	if (!dmgp) {
		rtd_pr_vdec_err("[%s %d]alloc memory fail\n", __func__, __LINE__);
		return ;
	}

	while (count > 0) {
		rtkvdec_memset(dmgp, '\0', DMESG_PRINT_MAX);
		if (count > (DMESG_PRINT_MAX - 64)) {
			rtkvdec_memcpy(dmgp, ptr, DMESG_PRINT_MAX - 64);
			temp_ptr = dmgp;

			tok = strsep((char **)&temp_ptr, "\n");

			while (tok != NULL) {
				print_count = strlen(tok);
				tok = strsep((char **)&temp_ptr, "\n");
			}

			if (print_count == DMESG_PRINT_MAX - 64) {
				rtkvdec_memcpy(dmgp, ptr, DMESG_PRINT_MAX - 64);
				temp_ptr = dmgp;

				tok = strsep((char **)&temp_ptr, "\n");

				while (tok != NULL) {
					rtd_pr_vdec_info("3.(%d) %s\n", count, tok);
					tok = strsep((char **)&temp_ptr, "\n");
				}

				count = count - (DMESG_PRINT_MAX - 64);
				ptr = ptr + (DMESG_PRINT_MAX - 64);
			}
			else {
				rtkvdec_memset(dmgp, '\0', DMESG_PRINT_MAX);
				rtkvdec_memcpy(dmgp, ptr, DMESG_PRINT_MAX - 64 - print_count);

				temp_ptr = dmgp;

				tok = strsep((char **)&temp_ptr, "\n");

				while (tok != NULL) {
					rtd_pr_vdec_info("1.(%d) %s\n", count, tok);
					tok = strsep((char **)&temp_ptr, "\n");
				}

				count = count - (DMESG_PRINT_MAX - 64 - print_count);
				ptr = ptr + (DMESG_PRINT_MAX - 64 - print_count);
			}
		} else {
			rtkvdec_memcpy(dmgp, ptr, count);
			temp_ptr = dmgp;

			tok = strsep((char **)&temp_ptr, "\n");

			while (tok != NULL) {
				print_count = strlen(tok);
				rtd_pr_vdec_info("2.(%d) %s\n", count, tok);
				tok = strsep((char **)&temp_ptr, "\n");
			}

			count = 0;
			ptr = ptr + count;
		}
	}

	kfree(dmgp);
}


static int rtkvdec_debug_thread(void *arg)
{
	unsigned long magic, size, wr, rd;
	unsigned char *wrPtr, *rdPtr, *basePtr, *limitPtr;
	int i ;
	unsigned long long tmp ;

	for (;;) {
		set_freezable();

		if (kthread_should_stop()) break;
		for (i = 0; i < 2; i++) {
			if (file_videoDump[i] != 0 && g_pVideoDumpMemory[i] != 0 && g_videoDumpMemPhyAddr[i] != 0 && dbg_config[i].enable) {
				DEBUG_BUFFER_HEADER *header;
				header = (DEBUG_BUFFER_HEADER*) g_pVideoDumpMemory[i];
				magic = ntohl(header->magic) ;
				size  = ntohl(header->size) ;
				rd    = ntohl(header->rd) ;
				wr    = ntohl(header->wr) ;
				wrPtr = (unsigned char *)(g_pVideoDumpMemory[i] + wr - g_videoDumpMemPhyAddr[i]); /* make virtual address */
				rdPtr = (unsigned char *)(g_pVideoDumpMemory[i] + rd - g_videoDumpMemPhyAddr[i]); /* make virtual address */
				basePtr  = (unsigned char *)(g_pVideoDumpMemory[i] + sizeof(DEBUG_BUFFER_HEADER));
				size -= sizeof(DEBUG_BUFFER_HEADER);
				limitPtr = basePtr+ size;

				/*
				rtd_pr_vdec_debug("Header %x, m=%x size=%x r(%x) w(%x)\n", (unsigned int)header, magic, size, rd, wr);
				rtd_pr_vdec_debug("Ptr w(%x) r(%x) b(%x) l(%x)\n", wrPtr, rdPtr, basePtr, limitPtr);
				*/

				if (wrPtr < rdPtr) {
					wrPtr = wrPtr + size;
				}

				if (wrPtr > rdPtr) {
					if (wrPtr > limitPtr) {
						if (limitPtr - rdPtr > DUMP_GET_MAX_SZIE) {
							file_write(file_videoDump[i], f_offset[i], rdPtr, DUMP_GET_MAX_SZIE) ;
							f_offset[i] += DUMP_GET_MAX_SZIE ;

							if (rtkvdec_pr_to_dmesg_en != 0)
								rtkvdec_print_to_dmesg(rdPtr, DUMP_GET_MAX_SZIE);

							header->rd = htonl(rd + DUMP_GET_MAX_SZIE) ;
						}
						else {
							file_write(file_videoDump[i], f_offset[i], rdPtr, limitPtr -rdPtr) ;

							if (rtkvdec_pr_to_dmesg_en != 0)
								rtkvdec_print_to_dmesg(rdPtr, limitPtr - rdPtr);

							f_offset[i] += limitPtr -rdPtr ;

							if (wrPtr - limitPtr > DUMP_GET_MAX_SZIE) {
								file_write(file_videoDump[i], f_offset[i], basePtr, DUMP_GET_MAX_SZIE) ;

								if (rtkvdec_pr_to_dmesg_en != 0)
									rtkvdec_print_to_dmesg(basePtr, DUMP_GET_MAX_SZIE);

								f_offset[i] += DUMP_GET_MAX_SZIE ;

								header->rd = htonl(g_videoDumpMemPhyAddr[i] + sizeof(DEBUG_BUFFER_HEADER) + DUMP_GET_MAX_SZIE) ;
							}
							else {
								file_write(file_videoDump[i], f_offset[i], basePtr, wrPtr - limitPtr) ;

								if (rtkvdec_pr_to_dmesg_en != 0)
									rtkvdec_print_to_dmesg(basePtr, wrPtr - limitPtr);

								f_offset[i] += wrPtr - limitPtr ;

								header->rd = htonl(g_videoDumpMemPhyAddr[i] + sizeof(DEBUG_BUFFER_HEADER) + (wrPtr - limitPtr)) ;
							}
						}
					}
					else {
						if (wrPtr - rdPtr > DUMP_GET_MAX_SZIE) {
							file_write(file_videoDump[i], f_offset[i], rdPtr, DUMP_GET_MAX_SZIE) ;
							f_offset[i] += DUMP_GET_MAX_SZIE ;

							if (rtkvdec_pr_to_dmesg_en != 0)
								rtkvdec_print_to_dmesg(rdPtr, DUMP_GET_MAX_SZIE);

							header->rd = htonl(rd + DUMP_GET_MAX_SZIE) ;
						}
						else {
							file_write(file_videoDump[i], f_offset[i], rdPtr, wrPtr - rdPtr) ;
							f_offset[i] += wrPtr - rdPtr ;

							if (rtkvdec_pr_to_dmesg_en != 0)
								rtkvdec_print_to_dmesg(rdPtr, wrPtr - rdPtr);

							header->rd = htonl(rd + (wrPtr - rdPtr)) ;
						}
					}
				}
			}

			if (rtkvdec_dump_size_scale != 0) {
				tmp = (unsigned long long) rtkvdec_dump_size_scale ;
				if (f_offset[i] > (RTKVDEC_DUMP_FILE_MAX_SIZE * tmp)) {
					f_offset[i] = 0;
				}
			}

			msleep(50); /* sleep 10 ms */
		}
	}

	/* rtd_pr_vdec_debug("rtkvdec_debug_thread break\n"); */

	return 0;
}

void rtkvdec_dump_init(void)
{
#ifndef CONFIG_ANDROID
	unsigned char default_file_name [27] = "/tmp/var/log/video_dump.bin" ;
	unsigned char default_file_name2[28] = "/tmp/var/log/video_dump2.bin" ;
	unsigned char default_file_name3[21] = "/tmp/video_dumpes.bin" ;
#else
	unsigned char default_file_name [25] = "/data/data/video_dump.bin" ;
	unsigned char default_file_name2[26] = "/data/data/video_dump2.bin" ;
	unsigned char default_file_name3[27] = "/data/data/video_dumpes.bin" ;
#endif
	int i ;

#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
	for (i = 0; i < 2; i++) {
#else
	for (i = 0; i < 1; i++) {
#endif
		dbg_config[i].enable = 0 ;
		dbg_config[i].mem_size = DUMP_MEMORY_SIZE ;
#ifndef CONFIG_ANDROID
		rtkvdec_memset((void *)dbg_config[i].file_name, '\0', FILE_NAME_SIZE );
		rtkvdec_memcpy((void *)dbg_config[i].file_name, (i == 0) ? (void *)default_file_name : (void *)default_file_name2, ( (i == 0) ? 27 : 28));
#else
		rtkvdec_memset((void *)dbg_config[i].file_name, '\0', ( (i == 0) ? 25 : 26) + 1 );
		rtkvdec_memcpy((void *)dbg_config[i].file_name, (i == 0) ? (void *)default_file_name : (void *)default_file_name2, ( (i == 0) ? 25 : 26));
#endif
	}

	/* set dump log file init size to 1MB */
	rtkvdec_dump_size_scale = 1;

	/* disable print VDSP log to dmesg at init */
	rtkvdec_pr_to_dmesg_en = 0;

	DumpES_Config.enable = 0 ;
	DumpES_Config.mem_size = DUMP_ES_SIZE ;
#ifndef CONFIG_ANDROID
	rtkvdec_memset((void *)DumpES_Config.file_name, '\0', FILE_NAME_SIZE );
	rtkvdec_memcpy((void *)DumpES_Config.file_name, (void *)default_file_name3, 21);
#else
	rtkvdec_memset((void *)DumpES_Config.file_name, '\0', 27 + 1 );
	rtkvdec_memcpy((void *)DumpES_Config.file_name, (void *)default_file_name3, 27);
#endif

	g_pDumpES.Memory = g_pDumpES.PhyAddr = g_pDumpES.VirtAddr = 0 ;
	g_pDumpES_Send.Memory = g_pDumpES_Send.PhyAddr = g_pDumpES_Send.VirtAddr = 0 ;
}

int rtkvdec_dump_enable(void)
{
	unsigned long return_value = 0 ;
	int i ;

	unsigned long vir_addr ;
	DEBUG_BUFFER_HEADER *header;

#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
	for (i = 0; i < 2; i++) {
#else
	for (i = 0; i < 1; i++) {
#endif
		if (dbg_config[i].enable) {
			rtd_pr_vdec_notice("rtkvdec: dbg dump already enable!\n");
			return 0 ;
		}

		file_videoDump[i] = file_open((char *)(dbg_config[i].file_name), O_TRUNC | O_RDWR | O_CREAT, 0666) ;
		if (file_videoDump[i] == 0) {
			rtd_pr_vdec_debug("[%s %d]open log file fail\n",__FUNCTION__,__LINE__);
			return -ENOMEM;
		}

		/* allocate debug memory */
		vir_addr = (unsigned long)dvr_malloc_uncached(sizeof(VIDEO_RPC_DEBUG_MEMORY)+256, (void **)&g_pVideoDebugMemory[i]);
		if (!vir_addr) {
			rtd_pr_vdec_err("[%s %d]alloc debug memory fail\n",__FUNCTION__,__LINE__);
			return -ENOMEM;
		}
		debugMemPhyAddr[i] = (unsigned long)dvr_to_phys((void*)vir_addr);
		debugMemVirtAddr[i] = vir_addr ;
		rtd_pr_vdec_notice("Alloc DBGME v(%lx) p(%lx) vn(%lx)\n"
		, vir_addr, debugMemPhyAddr[i], g_pVideoDebugMemory[i]);


		if ((file_videoDump[i] != 0) && (dbg_config[i].mem_size > 0)) {
			/* alocate dump memory */
			vir_addr = (unsigned long)dvr_malloc_uncached(dbg_config[i].mem_size, (void **)&g_pVideoDumpMemory[i]);
			if (!vir_addr) {
				vir_addr = debugMemVirtAddr[i];
				dvr_free((void*)vir_addr);
				rtd_pr_vdec_err("[%s %d]alloc debug memory fail\n",__FUNCTION__,__LINE__);
				return -ENOMEM;
			}
			g_videoDumpMemPhyAddr[i] = (unsigned long)dvr_to_phys((void*)vir_addr);
			g_videoDumpMemVirtAddr[i] = vir_addr ;

			rtd_pr_vdec_notice("Alloc DUMPME v(%lx) p(%lx) vn(%lx)\n"
			, vir_addr, g_videoDumpMemPhyAddr[i], g_pVideoDumpMemory[i]);

			/* setup debug dump ring buffer header */
			header = (DEBUG_BUFFER_HEADER*) g_pVideoDumpMemory[i];
			header->magic = htonl(0xdeadcafe) ;
			header->size = htonl(dbg_config[i].mem_size) ;
			header->rd = htonl(g_videoDumpMemPhyAddr[i] + sizeof(DEBUG_BUFFER_HEADER)) ;
			header->wr = htonl(g_videoDumpMemPhyAddr[i] + sizeof(DEBUG_BUFFER_HEADER)) ;

			*(unsigned long *)g_pVideoDebugMemory[i] = htonl(g_videoDumpMemPhyAddr[i]) ;

			rtd_pr_vdec_notice("Header %lx, m=%lx size=%lx r(%lx) w(%lx)\n"
			, (unsigned long)header, header->magic, header->size, header->rd, header->wr);
		}
		else {
			rtd_pr_vdec_notice("rtkvdec: no allocate debug dump ring buffer!\n");
			return 0 ;
		}
	}

	rtkvdec_debug_tsk = kthread_create(rtkvdec_debug_thread, &data, "rtkvdec_debug_thread");
	if (IS_ERR(rtkvdec_debug_tsk)) {
		rtkvdec_debug_tsk = NULL;
		return -1 ;
	}
	wake_up_process(rtkvdec_debug_tsk);

	if (send_rpc_command(RPC_VIDEO, RPCCMD_VIDEO_SETDEBUGMEMORY, (unsigned long)debugMemPhyAddr[0], 0, &return_value))
		rtd_pr_vdec_debug("rtkvdec: RPCCMD_VIDEO_SETDEBUGMEMORY fail %ld\n", return_value);
#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
	if (send_rpc_command(RPC_VIDEO2, RPCCMD_VIDEO_SETDEBUGMEMORY, (unsigned long)debugMemPhyAddr[1], 0, &return_value))
		rtd_pr_vdec_debug("rtkvdec: V2 RPCCMD_VIDEO_SETDEBUGMEMORY fail %ld\n", return_value);
#endif

	dbg_config[0].enable = 1 ;
	dbg_config[1].enable = 1 ;

	return 0 ;
}

void rtkvdec_dump_disable(void)
{
	unsigned long return_value = 0 ;
	int ret = 0;
	int i ;
	unsigned long vir_addr ;
	DEBUG_BUFFER_HEADER *header;

#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
	for (i = 0; i < 2; i++) {
#else
	for (i = 0; i < 1; i++) {
#endif
		if (!dbg_config[i].enable) {
			rtd_pr_vdec_notice("rtkvdec: dbg dump not enable!\n");
			return ;
		}

		dbg_config[i].enable = 0 ;
		/* setup debug dump ring buffer header */
		header = (DEBUG_BUFFER_HEADER*) g_pVideoDumpMemory[i];
		header->magic = htonl(0xdeadcafe) ;
		header->size = htonl(16) ;
		header->rd = htonl(g_videoDumpMemPhyAddr[i] + sizeof(DEBUG_BUFFER_HEADER)) ;
		header->wr = htonl(g_videoDumpMemPhyAddr[i] + sizeof(DEBUG_BUFFER_HEADER)) ;
		*(unsigned long *)g_pVideoDebugMemory[i] = htonl(g_videoDumpMemPhyAddr[i]) ;
	}

	if (send_rpc_command(RPC_VIDEO, RPCCMD_VIDEO_SETDEBUGMEMORY, (unsigned long)debugMemPhyAddr[0], 0, &return_value))
		rtd_pr_vdec_debug("rtkvdec: RPCCMD_VIDEO_SETDEBUGMEMORY fail %ld\n", return_value);
#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
	if (send_rpc_command(RPC_VIDEO2, RPCCMD_VIDEO_SETDEBUGMEMORY, (unsigned long)debugMemPhyAddr[1], 0, &return_value))
		rtd_pr_vdec_debug("rtkvdec: V2 RPCCMD_VIDEO_SETDEBUGMEMORY fail %ld\n", return_value);
#endif

	ret = kthread_stop(rtkvdec_debug_tsk);
	if (!ret)
		rtd_pr_vdec_debug("rtkvdec debug thread stopped\n");

#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
	for (i = 0; i < 2; i++) {
#else
	for (i = 0; i < 1; i++) {
#endif
		f_offset[i] = 0 ;
		if (g_videoDumpMemPhyAddr[i]) {
			vir_addr = g_videoDumpMemVirtAddr[i];
			dvr_free((void*)vir_addr);
			g_pVideoDumpMemory[i] = g_videoDumpMemPhyAddr[i] = g_videoDumpMemVirtAddr[i] = 0 ;
		}

		if (debugMemPhyAddr[i]) {
			vir_addr = debugMemVirtAddr[i];
			dvr_free((void*)vir_addr);
			g_pVideoDebugMemory[i] = debugMemPhyAddr[i] = debugMemVirtAddr[i] = 0 ;
		}

		if (file_videoDump[i])
			file_close(file_videoDump[i]) ;
		file_videoDump[i] = 0 ;
	}
}

static int rtkvdec_dumpes_thread(void *arg)
{
	unsigned long magic, size, wr, rd;
	unsigned char *wrPtr, *rdPtr, *basePtr, *limitPtr;

	for (;;) {
		if (kthread_should_stop()) break;
		if (file_DumpES != 0 && DumpES_Config.enable) {
			DEBUG_BUFFER_HEADER *header;
			header = (DEBUG_BUFFER_HEADER*) g_pDumpES.Memory;
			magic = ntohl(header->magic) ;
			size  = ntohl(header->size) ;
			rd    = ntohl(header->rd) ;
			wr    = ntohl(header->wr) ;
			wrPtr = (unsigned char *)(g_pDumpES.Memory + wr - g_pDumpES.PhyAddr); /* make virtual address */
			rdPtr = (unsigned char *)(g_pDumpES.Memory + rd - g_pDumpES.PhyAddr); /* make virtual address */
			basePtr  = (unsigned char *)(g_pDumpES.Memory + sizeof(DEBUG_BUFFER_HEADER));
			size -= sizeof(DEBUG_BUFFER_HEADER);
			limitPtr = basePtr+ size;

			/*
			rtd_pr_vdec_debug("Header %x, m=%lx size=%lx r(%lx) w(%lx)\n", (unsigned int)header, magic, size, rd, wr);
			rtd_pr_vdec_debug("Ptr w(%lx) r(%lx) b(%lx) l(%lx)\n", wrPtr, rdPtr, basePtr, limitPtr);
			*/

			if (wrPtr < rdPtr) {
				wrPtr = wrPtr + size;
			}

			if (wrPtr > rdPtr) {
				if (wrPtr > limitPtr) {
					file_write(file_DumpES, f_offset_dumpes, rdPtr, limitPtr -rdPtr) ;
					f_offset_dumpes += limitPtr -rdPtr ;
					file_write(file_DumpES, f_offset_dumpes, basePtr, wrPtr - limitPtr) ;
					f_offset_dumpes += wrPtr - limitPtr ;
				}
				else {
					file_write(file_DumpES, f_offset_dumpes, rdPtr, wrPtr - rdPtr) ;
					f_offset_dumpes += wrPtr - rdPtr ;
				}
				header->rd = htonl(wr) ;
			}
		}

		msleep(25); /* sleep 10 ms */
	}

	/* rtd_pr_vdec_debug("rtkvdec_dumpes_thread break\n"); */

	return 0;
}

int rtkvdec_dumpes_enable(const char *pattern, int length)
{
	unsigned long return_value = 0 ;
	DEBUG_BUFFER_HEADER *header;
	unsigned long vir_addr ;

	if (length > FILE_NAME_SIZE - 1) {
		rtd_pr_vdec_warn("rtkvdec: file name is too long(<%d).\n", FILE_NAME_SIZE - 1);
		return 0;
	}
	else if (length > 0) {
		rtkvdec_memset((void *)DumpES_Config.file_name, '\0', length+1);
		rtkvdec_memcpy((void *)DumpES_Config.file_name, (void *)pattern, length);
	}

	rtd_pr_vdec_notice("rtkvdec: dumpes_file_name(%s)\n", DumpES_Config.file_name);

	if (DumpES_Config.enable) {
		rtd_pr_vdec_notice("rtkvdec: es dump already enable!\n");
		return 0 ;
	}

	file_DumpES = file_open((char *)(DumpES_Config.file_name), O_TRUNC | O_RDWR | O_CREAT, 0666) ;
	if (file_DumpES == 0) {
		rtd_pr_vdec_err("[%s %d]open log file fail\n",__FUNCTION__,__LINE__);
		return -ENOMEM;
	}

	/* allocate debug memory */
	vir_addr = (unsigned long)dvr_malloc_uncached_specific(sizeof(VIDEO_RPC_DEBUG_MEMORY)+256, GFP_DCU1_LIMIT, (void **)(&g_pDumpES_Send.Memory));
	if (!vir_addr) {
		rtd_pr_vdec_err("[%s %d]alloc debug memory fail\n",__FUNCTION__,__LINE__);
		return -ENOMEM;
	}
	g_pDumpES_Send.PhyAddr = (unsigned long)dvr_to_phys((void*)vir_addr);
	g_pDumpES_Send.VirtAddr = vir_addr ;
	rtd_pr_vdec_notice("Alloc DESME v(%lx) p(%lx) vn(%lx)\n"
	, vir_addr, g_pDumpES_Send.PhyAddr, g_pDumpES_Send.Memory);


	if ((file_DumpES != 0) && (DumpES_Config.mem_size > 0)) {
		/* alocate dump memory */
		vir_addr = (unsigned long)dvr_malloc_uncached_specific(DumpES_Config.mem_size, GFP_DCU1_LIMIT, (void **)(&g_pDumpES.Memory));
		if (!vir_addr) {
			vir_addr = g_pDumpES_Send.VirtAddr ;
			dvr_free((void*)vir_addr);
			rtd_pr_vdec_err("[%s %d]alloc debug memory fail\n",__FUNCTION__,__LINE__);
			return -ENOMEM;
		}
		g_pDumpES.PhyAddr = (unsigned long)dvr_to_phys((void*)vir_addr);
		g_pDumpES.VirtAddr = vir_addr ;

		rtd_pr_vdec_notice("Alloc DUMPESME v(%lx) p(%lx) vn(%lx)\n"
		, vir_addr, g_pDumpES.PhyAddr, g_pDumpES.Memory);

		/* setup dump es ring buffer header */
		header = (DEBUG_BUFFER_HEADER*) g_pDumpES.Memory;
		header->magic = htonl(0xdeadcafe) ;
		header->size = htonl(DumpES_Config.mem_size) ;
		header->rd = htonl(g_pDumpES.PhyAddr + sizeof(DEBUG_BUFFER_HEADER)) ;
		header->wr = htonl(g_pDumpES.PhyAddr + sizeof(DEBUG_BUFFER_HEADER)) ;

		*(unsigned long *)g_pDumpES_Send.Memory = htonl(g_pDumpES.PhyAddr) ;

		rtd_pr_vdec_notice("Header %lx, m=%lx size=%lx r(%lx) w(%lx)\n"
		, (unsigned long)header, header->magic, header->size, header->rd, header->wr);
	}
	else {
		rtd_pr_vdec_notice("rtkvdec: no allocate debug dump ring buffer!\n");
		return 0 ;
	}

	rtkvdec_dumpes_tsk = kthread_create(rtkvdec_dumpes_thread, &data, "rtkvdec_dumpes_thread");
	if (IS_ERR(rtkvdec_dumpes_tsk)) {
		rtkvdec_dumpes_tsk = NULL;
		return -1 ;
	}
	wake_up_process(rtkvdec_dumpes_tsk);

	if (send_rpc_command(RPC_VIDEO, RPC_VCPU_SET_DUMPES_MEMORY, (unsigned long)g_pDumpES_Send.PhyAddr, 0, &return_value))
		rtd_pr_vdec_debug("rtkvdec: RPC_VCPU_SET_DUMPES_MEMORY fail %ld\n", return_value);

	DumpES_Config.enable = 1 ;

	return 0 ;
}

void rtkvdec_dumpes_disable(void)
{
	unsigned long return_value = 0 ;
	int ret = 0;
	unsigned long vir_addr ;
	DEBUG_BUFFER_HEADER *header;

	if (!DumpES_Config.enable) {
		rtd_pr_vdec_notice("rtkvdec: es dump not enable!\n");
		return ;
	}

	DumpES_Config.enable = 0 ;
	/* setup debug dump ring buffer header */
	header = (DEBUG_BUFFER_HEADER*) g_pDumpES.Memory;
	header->magic = htonl(0xdeadcafe) ;
	header->size = htonl(16) ;
	header->rd = htonl(g_pDumpES.PhyAddr + sizeof(DEBUG_BUFFER_HEADER)) ;
	header->wr = htonl(g_pDumpES.PhyAddr + sizeof(DEBUG_BUFFER_HEADER)) ;
	*(unsigned long *)g_pDumpES_Send.Memory = htonl(g_pDumpES.PhyAddr) ;


	if (send_rpc_command(RPC_VIDEO, RPC_VCPU_SET_DUMPES_MEMORY, (unsigned long)g_pDumpES_Send.PhyAddr, 0, &return_value))
		rtd_pr_vdec_debug("rtkvdec: RPC_VCPU_SET_DUMPES_MEMORY fail %ld\n", return_value);

	ret = kthread_stop(rtkvdec_dumpes_tsk);
	if (!ret)
		rtd_pr_vdec_debug("rtkvdec dumpes thread stopped\n");

	f_offset_dumpes = 0 ;
	if (g_pDumpES.PhyAddr) {
		vir_addr = g_pDumpES.VirtAddr;
		dvr_free((void*)vir_addr);
		g_pDumpES.Memory = g_pDumpES.PhyAddr = g_pDumpES.VirtAddr = 0 ;
	}

	if (g_pDumpES_Send.PhyAddr) {
		vir_addr = g_pDumpES_Send.VirtAddr;
		dvr_free((void*)vir_addr);
		g_pDumpES_Send.Memory = g_pDumpES_Send.PhyAddr = g_pDumpES_Send.VirtAddr = 0 ;
	}

	if (file_DumpES)
		file_close(file_DumpES) ;
	file_DumpES = 0 ;

}

int rtkvdec_exportcrc_enable(const char *pattern, int length)
{
	unsigned long return_value = 0 ;
	DEBUG_BUFFER_HEADER *header;
	unsigned long vir_addr ;

	if (length > FILE_NAME_SIZE - 1) {
		rtd_pr_vdec_warn("rtkvdec: file name is too long(<%d).\n", FILE_NAME_SIZE - 1);
		return 0;
	}
	else if (length > 0) {
		rtkvdec_memset((void *)DumpES_Config.file_name, '\0', length+1);
		rtkvdec_memcpy((void *)DumpES_Config.file_name, (void *)pattern, length);
	}

	rtd_pr_vdec_notice("rtkvdec: dumpes_file_name(%s)\n", DumpES_Config.file_name);

	if (DumpES_Config.enable) {
		rtd_pr_vdec_notice("rtkvdec: es dump already enable!\n");
		return 0 ;
	}

	file_DumpES = file_open((char *)(DumpES_Config.file_name), O_TRUNC | O_RDWR | O_CREAT, 0666) ;
	if (file_DumpES == 0) {
		rtd_pr_vdec_err("[%s %d]open log file fail\n",__FUNCTION__,__LINE__);
		return -ENOMEM;
	}

	/* allocate debug memory */
	vir_addr = (unsigned long)dvr_malloc_uncached_specific(sizeof(VIDEO_RPC_DEBUG_MEMORY)+256, GFP_DCU1_LIMIT, (void **)(&g_pDumpES_Send.Memory));
	if (!vir_addr) {
		rtd_pr_vdec_err("[%s %d]alloc debug memory fail\n",__FUNCTION__,__LINE__);
		return -ENOMEM;
	}
	g_pDumpES_Send.PhyAddr = (unsigned long)dvr_to_phys((void*)vir_addr);
	g_pDumpES_Send.VirtAddr = vir_addr ;
	rtd_pr_vdec_notice("Alloc DESME v(%lx) p(%lx) vn(%lx)\n"
	, vir_addr, g_pDumpES_Send.PhyAddr, g_pDumpES_Send.Memory);


	if ((file_DumpES != 0) && (DumpES_Config.mem_size > 0)) {
		/* alocate dump memory */
		vir_addr = (unsigned long)dvr_malloc_uncached_specific(DumpES_Config.mem_size, GFP_DCU1_LIMIT, (void **)(&g_pDumpES.Memory));
		if (!vir_addr) {
			vir_addr = g_pDumpES_Send.VirtAddr ;
			dvr_free((void*)vir_addr);
			rtd_pr_vdec_err("[%s %d]alloc debug memory fail\n",__FUNCTION__,__LINE__);
			return -ENOMEM;
		}
		g_pDumpES.PhyAddr = (unsigned long)dvr_to_phys((void*)vir_addr);
		g_pDumpES.VirtAddr = vir_addr ;

		rtd_pr_vdec_notice("Alloc DUMPESME v(%lx) p(%lx) vn(%lx)\n"
		, vir_addr, g_pDumpES.PhyAddr, g_pDumpES.Memory);

		/* setup dump es ring buffer header */
		header = (DEBUG_BUFFER_HEADER*) g_pDumpES.Memory;
		header->magic = htonl(0xdeadcafe) ;
		header->size = htonl(DumpES_Config.mem_size) ;
		header->rd = htonl(g_pDumpES.PhyAddr + sizeof(DEBUG_BUFFER_HEADER)) ;
		header->wr = htonl(g_pDumpES.PhyAddr + sizeof(DEBUG_BUFFER_HEADER)) ;

		*(unsigned long *)g_pDumpES_Send.Memory = htonl(g_pDumpES.PhyAddr) ;

		rtd_pr_vdec_notice("Header %lx, m=%lx size=%lx r(%lx) w(%lx)\n"
		, (unsigned long)header, header->magic, header->size, header->rd, header->wr);
	}
	else {
		rtd_pr_vdec_notice("rtkvdec: no allocate debug dump ring buffer!\n");
		return 0 ;
	}

	rtkvdec_dumpes_tsk = kthread_create(rtkvdec_dumpes_thread, &data, "rtkvdec_dumpes_thread");
	if (IS_ERR(rtkvdec_dumpes_tsk)) {
		rtkvdec_dumpes_tsk = NULL;
		return -1 ;
	}
	wake_up_process(rtkvdec_dumpes_tsk);

	if (send_rpc_command(RPC_VIDEO, RPC_VCPU_SET_EXPORTCRC_MEMORY, (unsigned long)g_pDumpES_Send.PhyAddr, 0, &return_value))
		rtd_pr_vdec_debug("rtkvdec: RPC_VCPU_SET_EXPORTCRC_MEMORY fail %ld\n", return_value);

	DumpES_Config.enable = 1 ;

	return 0 ;
}

void rtkvdec_exportcrc_disable(void)
{
	unsigned long return_value = 0 ;
	int ret = 0;
	unsigned long vir_addr ;
	DEBUG_BUFFER_HEADER *header;

	if (!DumpES_Config.enable) {
		rtd_pr_vdec_notice("rtkvdec: es dump not enable!\n");
		return ;
	}

	DumpES_Config.enable = 0 ;
	/* setup debug dump ring buffer header */
	header = (DEBUG_BUFFER_HEADER*) g_pDumpES.Memory;
	header->magic = htonl(0xdeadcafe) ;
	header->size = htonl(16) ;
	header->rd = htonl(g_pDumpES.PhyAddr + sizeof(DEBUG_BUFFER_HEADER)) ;
	header->wr = htonl(g_pDumpES.PhyAddr + sizeof(DEBUG_BUFFER_HEADER)) ;
	*(unsigned long *)g_pDumpES_Send.Memory = htonl(g_pDumpES.PhyAddr) ;


	if (send_rpc_command(RPC_VIDEO, RPC_VCPU_SET_EXPORTCRC_MEMORY, (unsigned long)g_pDumpES_Send.PhyAddr, 0, &return_value))
		rtd_pr_vdec_debug("rtkvdec: RPC_VCPU_SET_EXPORTCRC_MEMORY fail %ld\n", return_value);

	ret = kthread_stop(rtkvdec_dumpes_tsk);
	if (!ret)
		rtd_pr_vdec_debug("rtkvdec dumpes thread stopped\n");

	f_offset_dumpes = 0 ;
	if (g_pDumpES.PhyAddr) {
		vir_addr = g_pDumpES.VirtAddr;
		dvr_free((void*)vir_addr);
		g_pDumpES.Memory = g_pDumpES.PhyAddr = g_pDumpES.VirtAddr = 0 ;
	}

	if (g_pDumpES_Send.PhyAddr) {
		vir_addr = g_pDumpES_Send.VirtAddr;
		dvr_free((void*)vir_addr);
		g_pDumpES_Send.Memory = g_pDumpES_Send.PhyAddr = g_pDumpES_Send.VirtAddr = 0 ;
	}

	if (file_DumpES)
		file_close(file_DumpES) ;
	file_DumpES = 0 ;

}

int rtkvdec_importcrc_enable(const char *pattern, int length)
{
	unsigned long return_value = 0 ;
	DEBUG_BUFFER_HEADER *header;
	unsigned long vir_addr ;
	int headerSize = sizeof(DEBUG_BUFFER_HEADER);
	int fileSize;
	int maxFileSize = (10 * 1024 * 1024);

	if (length > FILE_NAME_SIZE - 1) {
		rtd_pr_vdec_warn("rtkvdec: file name is too long(<%d).\n", FILE_NAME_SIZE - 1);
		return 0;
	}
	else if (length > 0) {
		rtkvdec_memset((void *)DumpES_Config.file_name, '\0', length+1);
		rtkvdec_memcpy((void *)DumpES_Config.file_name, (void *)pattern, length);
	}

	rtd_pr_vdec_notice("rtkvdec: dumpes_file_name(%s)\n", DumpES_Config.file_name);

	if (DumpES_Config.enable) {
		rtd_pr_vdec_notice("rtkvdec: es dump already enable!\n");
		return 0 ;
	}

	file_DumpES = file_open((char *)(DumpES_Config.file_name), O_RDONLY, 0666) ;
	if (file_DumpES == 0) {
		rtd_pr_vdec_err("[%s %d]open log file fail\n",__FUNCTION__,__LINE__);
		return -ENOMEM;
	}
	fileSize = file_size(DumpES_Config.file_name);
	if (fileSize <= 0 || fileSize >= maxFileSize)
	{
		rtd_pr_vdec_err("[%s %d]invalid file size %d, max %d\n",__FUNCTION__,__LINE__,fileSize,maxFileSize);
		return -ENOMEM;
	}


	if ((file_DumpES != 0)) {
		int i;
		unsigned int readData;
		int readSize = sizeof(readData);
		unsigned int *pRead = NULL;
		unsigned long long offset = 0;

		/* alocate dump memory */
		vir_addr = (unsigned long)dvr_malloc_uncached_specific((headerSize + fileSize), GFP_DCU1_LIMIT, (void **)(&g_pDumpES_Send.Memory));
		if (!vir_addr) {
			rtd_pr_vdec_err("[%s %d]alloc debug memory fail\n",__FUNCTION__,__LINE__);
			return -ENOMEM;
		}
		g_pDumpES_Send.PhyAddr = (unsigned long)dvr_to_phys((void*)vir_addr);
		g_pDumpES_Send.VirtAddr = vir_addr ;

		rtd_pr_vdec_notice("Alloc DUMPESME v(%lx) p(%lx) vn(%lx)\n"
		, vir_addr, g_pDumpES_Send.PhyAddr, g_pDumpES_Send.Memory);

		pRead = (unsigned int *)(g_pDumpES_Send.Memory + headerSize);
		for (i=0; i<fileSize; i+=readSize, offset+=readSize)
		{
			file_read(file_DumpES, offset, (unsigned char *)(&readData), sizeof(readData));
			*pRead = readData;
			pRead++;
		}

		/* setup dump es ring buffer header */
		header = (DEBUG_BUFFER_HEADER*) g_pDumpES_Send.Memory;
		header->magic = htonl(0xdeadcafe) ;
		header->size = htonl(fileSize) ;
		header->rd = htonl(g_pDumpES_Send.PhyAddr + headerSize) ;
		header->wr = htonl(g_pDumpES_Send.PhyAddr + headerSize) ;

		rtd_pr_vdec_notice("Header %lx, m=%lx size=%lx r(%lx) w(%lx)\n"
		, (unsigned long)header, header->magic, header->size, header->rd, header->wr);
	}
	else {
		rtd_pr_vdec_notice("rtkvdec: no allocate debug dump ring buffer!\n");
		return 0 ;
	}

	if (send_rpc_command(RPC_VIDEO, RPC_VCPU_SET_IMPORTCRC_MEMORY, (unsigned long)g_pDumpES_Send.PhyAddr, 0, &return_value))
		rtd_pr_vdec_debug("rtkvdec: RPC_VCPU_SET_IMPORTCRC_MEMORY fail %ld\n", return_value);

	DumpES_Config.enable = 1 ;

	return 0 ;
}

void rtkvdec_importcrc_disable(void)
{
	unsigned long return_value = 0 ;
	unsigned long vir_addr ;
	DEBUG_BUFFER_HEADER *header;
	int headerSize = sizeof(DEBUG_BUFFER_HEADER);

	if (!DumpES_Config.enable) {
		rtd_pr_vdec_notice("rtkvdec: es dump not enable!\n");
		return ;
	}

	DumpES_Config.enable = 0 ;
	/* setup debug dump ring buffer header */
	header = (DEBUG_BUFFER_HEADER*) g_pDumpES_Send.VirtAddr;
	header->magic = htonl(0xdeadcafe) ;
	header->size = htonl(16) ;
	header->rd = htonl(g_pDumpES_Send.PhyAddr + headerSize) ;
	header->wr = htonl(g_pDumpES_Send.PhyAddr + headerSize) ;


	if (send_rpc_command(RPC_VIDEO, RPC_VCPU_SET_IMPORTCRC_MEMORY, (unsigned long)g_pDumpES_Send.PhyAddr, 0, &return_value))
		rtd_pr_vdec_debug("rtkvdec: RPC_VCPU_SET_EXPORTCRC_MEMORY fail %ld\n", return_value);

	if (g_pDumpES_Send.PhyAddr) {
		vir_addr = g_pDumpES_Send.VirtAddr;
		dvr_free((void*)vir_addr);
		g_pDumpES_Send.Memory = g_pDumpES_Send.PhyAddr = g_pDumpES_Send.VirtAddr = 0 ;
	}

	if (file_DumpES)
		file_close(file_DumpES) ;
	file_DumpES = 0 ;

}

extern bool reserve_boot_memory;

static int rtkvdec_vcreate_thread(void *arg)
{
	unsigned long return_value = 0 ;
	unsigned long ret=S_OK;
	int platform = 0;
#if !defined(CONFIG_ARM64) && !IS_ENABLED(CONFIG_REALTEK_LOGBUF)
	int ret2 = 0;
	int count = 0;
#endif
	VIDEO_INIT_DATA *init, *init_addr ;
	void *p;
	unsigned long pUva = 0;
	unsigned long DRAM_size = 0;

#ifdef CONFIG_LG_SNAPSHOT_BOOT
	if (snapshot_enable == 1)
	{
		unsigned long timeout = jiffies + HZ * 30; //30 seconds
		bool is_timeout = false;
		// waiting for saving image finish, this patch is used to prevent migration failure
		while (reserve_boot_memory) {
			msleep(10);
			if (time_before(jiffies, timeout) == 0) {
				is_timeout = true;
				break;
			}
		}

		if (is_timeout)
			BUG();
	}
#endif
	/* set platform type */
	if      ( get_platform_model() == PLATFORM_MODEL_8K ) platform = 1;
	else if ( get_platform_model() == PLATFORM_MODEL_4K ) platform = 2;
	else if ( get_platform_model() == PLATFORM_MODEL_2K ) platform = 3;
	else                                                  platform = 0;

	DRAM_size = get_memory_size(GFP_DCU1) + get_memory_size(GFP_DCU2);

	if ( get_product_type() == PRODUCT_TYPE_DIAS ) {
		if   ( DRAM_size >= 0x60000000 ) platform += 8; // 1.5G
		else                             platform += 4; // 1G
	}
	else if ( get_product_type() == PRODUCT_TYPE_DEMETER ) {
	}
	else {
		rtd_pr_vdec_err("[%s %d] invaild projuct type!!!\n", __FUNCTION__, __LINE__);
		platform = 0;
	}

	rtd_pr_vdec_err("[%s %d] platform model %d\n", __FUNCTION__, __LINE__, platform);

	rtd_pr_vdec_debug("rtkvdec: init\n");

	p = dvr_malloc_uncached_specific(PAGE_SIZE, GFP_DCU1_LIMIT, (void **)&pUva);
	if (!p) {
		rtd_pr_vdec_err("[%s %d]alloc INIT memory fail\n",__FUNCTION__,__LINE__);
		return -ENOMEM;
	}

	rtd_pr_vdec_err("[%s %d] alloc pass!\n" , __FUNCTION__, __LINE__);

	init = (VIDEO_INIT_DATA *)pUva ;
	init->boardType = htonl(0) ; /* use this para to do kernel/fw video create handshake*/
	init_addr = (VIDEO_INIT_DATA *)(dvr_to_phys(p));

	// KWarning: checked ok by a0945150@realtek.com
	for (;;) {
		ret = send_rpc_command(RPC_VIDEO, RPCCMD_VIDEO_CREATE, (unsigned long)init_addr, (unsigned long)platform, &return_value) ;

		if (!ret) {
			if (ntohl(init->boardType) == 0) {
				VideoCreatDone = 1 ;
				rtd_pr_vdec_debug("rtkvdec: RPCCMD_VIDEO_CREATE success %d\n", (unsigned int)ret);
				break ;
			}
			else { //video fw in pov
				while (!notify_rtkvdec_release_pov) {
					msleep(200); // sleep 10 ms
				}
				notify_rtkvdec_release_pov = 0 ;
			}
		}

		msleep(100); /* sleep 10 ms */
	}

#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
	init->boardType = htonl(0) ; /*use this para to do kernel/fw video create handshake */

	for (;;) {
		ret = send_rpc_command(RPC_VIDEO2, RPCCMD_VIDEO_CREATE, (unsigned long)init_addr, (unsigned long)platform, &return_value) ;

		if (!ret && ntohl(init->boardType) == 0) {
			VideoCreatDone2 = 1 ;
			rtd_pr_vdec_debug("rtkvdec: RPCCMD_VIDEO_CREATE V2 success %ld\n", ret);
			break ;
		}

		msleep(100); /* sleep 10 ms */
	}
#endif

	dvr_free(p);
	rtd_pr_vdec_debug("rtkvdec_vcreate_thread end\n");

#if !defined(CONFIG_ARM64) && !IS_ENABLED(CONFIG_REALTEK_LOGBUF)
	for (;;) {
		ret2 = rtkvdec_dump_enable();

		if (ret2 < 0) {
			rtd_pr_vdec_debug("wait to enable video dump file, retry...\n");
			msleep(1000); /* sleep 1s */
			count++;
		} else if (ret2 == 0){
			break;
		}

		if (count == 10) {
			rtd_pr_vdec_err("wait enable video dump file timeout\n");
			break;
		}
	}
#endif

	rtkvdec_send_sharedmemory();
	VideoCreatFinish = 1 ;

#if 0
	msleep(30000);
	rtkvdec_vbm_by_frame_testcase3();
	msleep(5000);
	rtkvdec_vbm_by_frame_testcase4();
#endif

	return 0;
}


int rtkvdec_alloc(int size)
{
	unsigned long vir_addr;
	unsigned long phy_addr;
	struct buffer_info *buf_info;

	vir_addr = (unsigned long)dvr_malloc_specific(size, GFP_DCU1_LIMIT);
	phy_addr = (unsigned long)dvr_to_phys((void *)vir_addr);

	if (!phy_addr) {
		rtd_pr_vdec_err("[%s %d]alloc memory fail\n", __func__, __LINE__);
		return 0;
	}

	buf_info = kmalloc(sizeof(struct buffer_info), GFP_KERNEL);
	buf_info->vir_addr = vir_addr;
	buf_info->phy_addr = phy_addr;
	buf_info->request_size = size;
	buf_info->malloc_size = PAGE_ALIGN(size);
	buf_info->task_pid = task_tgid_nr(current);
	rtd_pr_vdec_debug("Alloc (p:%lx,v:%lx) for pid(%d)\n",
		phy_addr, vir_addr, task_tgid_nr(current));

	down(&rtkvdec_alloc_sem);
	list_add(&buf_info->buffer_list, &rtkvdec_alloc_head);
	up(&rtkvdec_alloc_sem);

	return phy_addr;
}

int rtkvdec_free(unsigned int phy_addr)
{
	unsigned long vir_addr=0;
	struct buffer_info *buf_info;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	struct kernel_siginfo    sinfo;    /* signal information */
#else
	struct siginfo    sinfo;    /* signal information */
#endif
	pid_t  pid;                   /* user program process id */
	struct task_struct *task;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	rtkvdec_memset(&sinfo, 0, sizeof(struct kernel_siginfo));
#else
	rtkvdec_memset(&sinfo, 0, sizeof(struct siginfo));
#endif
	sinfo.si_signo = SIGSEGV;    // Config the signals: SIGIO
	sinfo.si_code  = -1;         //__SI_RT

	if (phy_addr == 0) {
		rtd_pr_vdec_err("[%s %d] Error!! phyaddr %x\n", __func__, __LINE__, phy_addr);
		pid = task_tgid_nr(current);
		task = pid_task(find_pid_ns(pid, task_active_pid_ns(current)), PIDTYPE_PID);
		if (task == NULL) {
			rtd_pr_vdec_err("Cannot find pid from user program\r\n");
			return 0;
		}
		/* send SIGIO to the user program */
		send_sig_info (SIGSEGV, &sinfo, task); /* Send signal to user program */
		return 0 ;
	}

	down(&rtkvdec_alloc_sem);
	list_for_each_entry(buf_info, &rtkvdec_alloc_head, buffer_list)
	{
		if (buf_info->phy_addr == phy_addr) {
			vir_addr = buf_info->vir_addr;
			list_del(&buf_info->buffer_list);
			kfree(buf_info);
			break;
		}
	}
	up(&rtkvdec_alloc_sem);

	if (vir_addr != 0) {
		dvr_free((void *)vir_addr);
	}
	else {
		rtd_pr_vdec_err("[%s %d] Error!! no find phyaddr %x\n", __func__, __LINE__, phy_addr);
		pid = task_tgid_nr(current);
		task = pid_task(find_pid_ns(pid, task_active_pid_ns(current)), PIDTYPE_PID);
		if (task == NULL) {
			rtd_pr_vdec_err("Cannot find pid from user program\r\n");
			return 0;
		}
		/* send SIGIO to the user program */
		send_sig_info (SIGSEGV, &sinfo, task); /* Send signal to user program */
	}

	return 0;
}

void rtkvdec_user_alloc_query(RINGBUFFER_HEADER *ringBuf_H, unsigned int pid, unsigned int size, unsigned long phy_addr )
{
	struct buffer_info *buf_info;
	RINGBUFFER_HEADER *RBH = 0 ;

	down(&rtkvdec_alloc_sem);
	list_for_each_entry(buf_info, &rtkvdec_alloc_head, buffer_list)
	{
		if (buf_info->task_pid == pid && buf_info->request_size <= size) {
			RBH = (RINGBUFFER_HEADER *)buf_info->vir_addr ;
			if (ntohl(RBH->beginAddr) == phy_addr) {
				rtd_pr_vdec_notice("start %x, size %x, w %x, R %x\n"
					, ntohl(RBH->beginAddr), ntohl(RBH->size)
					, ntohl(RBH->writePtr ), ntohl(RBH->readPtr[0]));
				ringBuf_H->beginAddr  = ntohl(RBH->beginAddr) ;
				ringBuf_H->size       = ntohl(RBH->size) ;
				ringBuf_H->writePtr   = ntohl(RBH->writePtr) ;
				ringBuf_H->readPtr[0] = ntohl(RBH->readPtr[0]) ;
				up(&rtkvdec_alloc_sem);
				return ;
			}
		}
	}
	up(&rtkvdec_alloc_sem);
}

void rtkvdec_user_alloc_summary(struct ret_info *info)
{
	unsigned long sum_request, sum_malloc;
	struct buffer_info *buf_info;
	struct buffer_info *b;
	struct list_head pid_list = LIST_HEAD_INIT(pid_list);

	sum_malloc = sum_request = 0;

	down(&rtkvdec_alloc_sem);
	rtd_pr_vdec_notice("rtkvdec_user_alloc_summary\n");
	list_for_each_entry(buf_info, &rtkvdec_alloc_head, buffer_list)
	{
		rtd_pr_vdec_notice("pid(%d) : vir_addr %x, phy_addr %x, malloc %d, request %d\n",
			(int)buf_info->task_pid,
			(int)buf_info->vir_addr,
			(int)buf_info->phy_addr,
			(int)buf_info->malloc_size,
			(int)buf_info->request_size);

		if (list_empty(&pid_list)) {
			b = kmalloc(sizeof(struct buffer_info), GFP_KERNEL);
			b->task_pid = buf_info->task_pid;
			b->request_size = b->malloc_size = 0;
			list_add(&b->buffer_list, &pid_list);
		} else {
			int pid_exist = false;
			list_for_each_entry(b, &pid_list, buffer_list)
			{
				if (b->task_pid == buf_info->task_pid) {
					pid_exist = true;
					break;
				}
			}
			if (pid_exist == false) {
				b = kmalloc(sizeof(struct buffer_info),
					GFP_KERNEL);
				b->task_pid = buf_info->task_pid;
				b->request_size = b->malloc_size = 0;
				list_add(&b->buffer_list, &pid_list);
			}
		}
		b->request_size += buf_info->request_size;
		b->malloc_size += buf_info->malloc_size;
	}
	up(&rtkvdec_alloc_sem);
	list_for_each_entry_safe(b, buf_info, &pid_list, buffer_list)
	{
		rtd_pr_vdec_notice("pid(%d) : sum_malloc %d, sum_request %d\n",
			(int)b->task_pid,
			(int)b->malloc_size,
			(int)b->request_size);
		sum_request += b->request_size;
		sum_malloc  += b->malloc_size;
		list_del(&b->buffer_list);
		kfree(b);
	}

	rtd_pr_vdec_notice("sum_malloc %ld, sum_request %ld\n", sum_malloc, sum_request);

	if (info == NULL)
		return;

	info->private_info[0] = sum_request;
	info->private_info[1] = sum_malloc;
}

void rtkvdec_free_by_process(unsigned long process_id)
{
	struct buffer_info *buf_info, *tmp_buf_info;
	struct task_struct *task;

	rtkvdec_svp_free_by_pid(process_id);
	rtkvdec_ibbuf_free_by_pid(process_id);

	down(&rtkvdec_alloc_sem);
	list_for_each_entry_safe(buf_info, tmp_buf_info,
		&rtkvdec_alloc_head, buffer_list)
	{
		if (buf_info->task_pid == process_id) {
			list_del(&buf_info->buffer_list);
			dvr_free((void *)buf_info->vir_addr);
			kfree(buf_info);
		}
		else {
			task = pid_task(find_vpid(buf_info->task_pid), PIDTYPE_PID);
			if(task == NULL) {
				list_del(&buf_info->buffer_list);
				dvr_free((void *)buf_info->vir_addr);
				kfree(buf_info);
			}
		}
	}
	up(&rtkvdec_alloc_sem);
}

/* RPC handler function */
unsigned long rtkvdec_free_user_mem(unsigned long para1, unsigned long para2)
{
	rtd_pr_vdec_debug("rtkvdec_free_user_mem(%lx/%lx)\n", para1, para2);
	/* rtkvdec_user_alloc_summary(NULL); */
	if (para2 == 0x1) {
#ifdef CONFIG_RTK_KDRV_DV
#ifdef CONFIG_DOLBY_IDK_1_6_1_1
		extern int dolby_adapter_DV_ForceSTOP(unsigned long process_id);
		dolby_adapter_DV_ForceSTOP(para1);
#endif
#endif
		rtkvdec_free_by_process(para1);
	}
	/* rtkvdec_user_alloc_summary(NULL); */
	return 0;
}

long rtkvdec_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
//	unsigned long return_value ;
	long ret = 0;

	rtd_pr_vdec_debug("rtkvdec: receive ioctl(cmd:0x%08x, arg:0x%08x)\n", cmd, (unsigned int)arg);
	switch (cmd) {
		case VDEC_IOC_FRAMEINFO:
		{
			/* rtd_pr_vdec_debug("rtkvdec: frame info\n"); */
			VDEC_FRAMEINFO_STRUCT frame_info;

			struct list_head *del_list;
			VDEC_FRAMEINFO_LIST *buf_info_list;
			if (copy_from_user((void *)&frame_info, (const void __user *)arg, sizeof(VDEC_FRAMEINFO_STRUCT))) {
				ret = -EFAULT;
				rtd_pr_vdec_err("rtkvdec ioctl code=VDEC_IOC_FRAMEINFO failed!!!!!!!!!!!!!!!\n");
				goto out;
			}

			down(&gst_frame_info_list_sem);
			list_for_each(del_list, &gst_frame_info_head) {
				buf_info_list = list_entry(del_list, VDEC_FRAMEINFO_LIST, buffer_list);
				if (buf_info_list->frame_info.pid == frame_info.pid) {
					VDEC_FRAMEINFO_STRUCT *buf_info = &buf_info_list->frame_info ;
					buf_info->frame_dropped = frame_info.frame_dropped ;
					buf_info->frame_displaying = frame_info.frame_displaying ;
					buf_info->frame_matched = frame_info.frame_matched ;
					buf_info->frame_freerun = frame_info.frame_freerun ;
					break;
				}
			}
			up(&gst_frame_info_list_sem);
			/* rtd_pr_vdec_debug("rtkvdec : path %s msize %x\n", dbg_config.file_name, dbg_config.mem_size); */

			break ;
		}

		case VDEC_IOC_ALLOCSVPMP:
		case VDEC_IOC_ALLOCSVPM:
		{
			/* rtd_pr_vdec_debug("rtkvdec: allocate SVP memory\n"); */
			VDEC_SVPMEM_STRUCT svpmem ;

			if (copy_from_user((void *)&svpmem, (const void __user *)arg, sizeof(VDEC_SVPMEM_STRUCT))) {
				ret = -EFAULT;
				rtd_pr_vdec_err("rtkvdec ioctl code=VDEC_IOC_FREESVPM failed!!!!!!!!!!!!!!!\n");
				goto out;
			}

			svpmem.addr = 0 ;
			ret = rtkvdec_svp_alloc(&svpmem, cmd);

			if (copy_to_user((void __user *)arg, (void *)&svpmem, sizeof(VDEC_SVPMEM_STRUCT))) {
				ret = -EFAULT;
				goto out;
			}
			break;
		}

		case VDEC_IOC_FREESVPM:
		{
			/* rtd_pr_vdec_debug("rtkvdec: free SVP memory\n"); */
			VDEC_SVPMEM_STRUCT svpmem ;

			if (copy_from_user((void *)&svpmem, (const void __user *)arg, sizeof(VDEC_SVPMEM_STRUCT))) {
				ret = -EFAULT;
				rtd_pr_vdec_err("rtkvdec ioctl code=VDEC_IOC_FREESVPM failed!!!!!!!!!!!!!!!\n");
				goto out;
			}

			ret = rtkvdec_svp_free(&svpmem);
			break;
		}

		case VDEC_IOC_ALLOC_IBBUF:
		{
			/* rtd_pr_vdec_debug("rtkvdec: allocate IBBUF\n"); */
			VDEC_SVPMEM_STRUCT IBBuf ;
			if (copy_from_user((void *)&IBBuf, (const void __user *)arg, sizeof(VDEC_SVPMEM_STRUCT))) {
				ret = -EFAULT;
				rtd_pr_vdec_err("rtkvdec ioctl code=VDEC_IOC_ALLOC_IBBUF failed!!!!!!!!!!!!!!!\n");
				goto out;
			}

			IBBuf.addr = 0 ;
			IBBuf.addr = rtkvdec_cobuffer_alloc(VDEC_INBAND_BUF, IBBuf.pid) ;

			if (copy_to_user((void __user *)arg, (void *)&IBBuf, sizeof(VDEC_SVPMEM_STRUCT))) {
				ret = -EFAULT;
				goto out;
			}
			break;
		}

		case VDEC_IOC_FREE_IBBUF:
		{
			/* rtd_pr_vdec_debug("rtkvdec: free IBBUF\n"); */
			VDEC_SVPMEM_STRUCT IBBuf ;
			if (copy_from_user((void *)&IBBuf, (const void __user *)arg, sizeof(VDEC_SVPMEM_STRUCT))) {
				ret = -EFAULT;
				rtd_pr_vdec_err("rtkvdec ioctl code=VDEC_IOC_FREE_IBBUF failed!!!!!!!!!!!!!!!\n");
				goto out;
			}

			rtkvdec_cobuffer_free(VDEC_INBAND_BUF, IBBuf.addr) ;

			break;
		}

		case VDEC_IOC_ALLOC:
		{
			rtd_pr_vdec_debug("rtkvdec: VDEC_IOC_ALLOC\n");
			ret = rtkvdec_alloc(arg);
			break;
		}

		case VDEC_IOC_FREE:
		{
			rtd_pr_vdec_debug("rtkvdec: VDEC_IOC_FREE\n");
			ret = rtkvdec_free(arg);
			break;
		}

		case VDEC_IOC_HALLOG:
		{
			unsigned char g_HalLogBuf[128];
			rtd_pr_vdec_debug("rtkvdec: VDEC_IOC_HALLOG %lx\n", arg);
			rtkvdec_memset(g_HalLogBuf, 0, 128);
			if (copy_from_user((void *)g_HalLogBuf, (const void __user *)arg, 128)) {
				ret = -EFAULT;
				rtd_pr_vdec_err("rtkvdec ioctl code=VDEC_IOC_HALLOG failed!!!!!!!!!!!!!!!\n");
				goto out;
			}
			rtd_pr_vdec_info("%s\n", g_HalLogBuf);
			break;
		}

		case VDEC_IOC_QVCDONE:
		{
			rtd_pr_vdec_debug("rtkvdec: VDEC_IOC_QVCDONE %d\n", VideoCreatFinish);
			ret = VideoCreatFinish ;
			break;
		}

		case VDEC_IOC_QUERY_NOV:
		{
			rtd_pr_vdec_debug("rtkvdec: VDEC_IOC_QUERY_NOV\n");
			ret = rtkvdec_query_num_of_vcpu();
			break;
		}
		case VDEC_IOC_SVP_DMABUF_ALLOCATE:
		case VDEC_IOC_SVP_DMABUF_GET_INFO:
		case VDEC_IOC_SVP_DMABUF_START_RINGBUF_UPDATE:
		case VDEC_IOC_SVP_DMABUF_COMMIT_RINGBUF_UPDATE:
		{
			ret = rtk_vdec_dmabuf_ioctl(filp, cmd, arg);
			break;
		}
		default:
		{
			rtd_pr_vdec_debug("rtkvdec: unknown ioctl(0x%08x)\n", cmd);
			break;
		}
	};

	out:
	return ret;
}


#ifdef CONFIG_ARM64
#ifdef CONFIG_COMPAT
long rtkvdec_ioctl_compat(struct file *filp, unsigned int cmd, unsigned long arg)
{
//	unsigned long return_value ;
	long ret = 0;

	rtd_pr_vdec_debug("rtkvdec: receive ioctl(cmd:0x%08x, arg:0x%08x)\n", cmd, (unsigned int)arg);
	switch (cmd) {
		case VDEC_IOC_FRAMEINFO:
		{
			/* rtd_pr_vdec_debug("rtkvdec: frame info\n"); */
			VDEC_FRAMEINFO_STRUCT frame_info;

			struct list_head *del_list;
			VDEC_FRAMEINFO_LIST *buf_info_list;
			if (copy_from_user((void *)&frame_info, (const void __user *)arg, sizeof(VDEC_FRAMEINFO_STRUCT))) {
				ret = -EFAULT;
				rtd_pr_vdec_err("rtkvdec ioctl code=VDEC_IOC_FRAMEINFO failed!!!!!!!!!!!!!!!\n");
				goto out;
			}

			down(&gst_frame_info_list_sem);
			list_for_each(del_list, &gst_frame_info_head) {
				buf_info_list = list_entry(del_list, VDEC_FRAMEINFO_LIST, buffer_list);
				if (buf_info_list->frame_info.pid == frame_info.pid) {
					VDEC_FRAMEINFO_STRUCT *buf_info = &buf_info_list->frame_info ;
					buf_info->frame_dropped = frame_info.frame_dropped ;
					buf_info->frame_displaying = frame_info.frame_displaying ;
					buf_info->frame_matched = frame_info.frame_matched ;
					buf_info->frame_freerun = frame_info.frame_freerun ;
					break;
				}
			}
			up(&gst_frame_info_list_sem);
			/* rtd_pr_vdec_debug("rtkvdec : path %s msize %x\n", dbg_config.file_name, dbg_config.mem_size); */

			break ;
		}

		case VDEC_IOC_ALLOCSVPMP:
		case VDEC_IOC_ALLOCSVPM:
		{
			/* rtd_pr_vdec_debug("rtkvdec: allocate SVP memory\n"); */
			VDEC_SVPMEM_STRUCT svpmem ;

			if (copy_from_user((void *)&svpmem, (const void __user *)arg, sizeof(VDEC_SVPMEM_STRUCT))) {
				ret = -EFAULT;
				rtd_pr_vdec_err("rtkvdec ioctl code=VDEC_IOC_FREESVPM failed!!!!!!!!!!!!!!!\n");
				goto out;
			}

			svpmem.addr = 0 ;
			ret = rtkvdec_svp_alloc(&svpmem, cmd);

			if (copy_to_user((void __user *)arg, (void *)&svpmem, sizeof(VDEC_SVPMEM_STRUCT))) {
				ret = -EFAULT;
				goto out;
			}
			break;
		}

		case VDEC_IOC_FREESVPM:
		{
			/* rtd_pr_vdec_debug("rtkvdec: free SVP memory\n"); */
			VDEC_SVPMEM_STRUCT svpmem ;

			if (copy_from_user((void *)&svpmem, (const void __user *)arg, sizeof(VDEC_SVPMEM_STRUCT))) {
				ret = -EFAULT;
				rtd_pr_vdec_err("rtkvdec ioctl code=VDEC_IOC_FREESVPM failed!!!!!!!!!!!!!!!\n");
				goto out;
			}

			ret = rtkvdec_svp_free(&svpmem);
			break;
		}

		case VDEC_IOC_ALLOC_IBBUF:
		{
			/* rtd_pr_vdec_debug("rtkvdec: allocate IBBUF\n"); */
			VDEC_SVPMEM_STRUCT IBBuf ;
			if (copy_from_user((void *)&IBBuf, (const void __user *)arg, sizeof(VDEC_SVPMEM_STRUCT))) {
				ret = -EFAULT;
				rtd_pr_vdec_err("rtkvdec ioctl code=VDEC_IOC_ALLOC_IBBUF failed!!!!!!!!!!!!!!!\n");
				goto out;
			}

			IBBuf.addr = 0 ;
			IBBuf.addr = rtkvdec_cobuffer_alloc(VDEC_INBAND_BUF, IBBuf.pid) ;

			if (copy_to_user((void __user *)arg, (void *)&IBBuf, sizeof(VDEC_SVPMEM_STRUCT))) {
				ret = -EFAULT;
				goto out;
			}
			break;
		}

		case VDEC_IOC_FREE_IBBUF:
		{
			/* rtd_pr_vdec_debug("rtkvdec: free IBBUF\n"); */
			VDEC_SVPMEM_STRUCT IBBuf ;
			if (copy_from_user((void *)&IBBuf, (const void __user *)arg, sizeof(VDEC_SVPMEM_STRUCT))) {
				ret = -EFAULT;
				rtd_pr_vdec_err("rtkvdec ioctl code=VDEC_IOC_FREE_IBBUF failed!!!!!!!!!!!!!!!\n");
				goto out;
			}

			rtkvdec_cobuffer_free(VDEC_INBAND_BUF, IBBuf.addr) ;

			break;
		}

		case VDEC_IOC_ALLOC:
		{
			rtd_pr_vdec_debug("rtkvdec: VDEC_IOC_ALLOC\n");
			ret = rtkvdec_alloc(arg);
			break;
		}

		case VDEC_IOC_FREE:
		{
			rtd_pr_vdec_debug("rtkvdec: VDEC_IOC_FREE\n");
			ret = rtkvdec_free(arg);
			break;
		}

		case VDEC_IOC_HALLOG:
		{
			unsigned char g_HalLogBuf[128];
			rtd_pr_vdec_debug("rtkvdec: VDEC_IOC_HALLOG %lx\n", arg);
			rtkvdec_memset(g_HalLogBuf, 0, 128);
			if (copy_from_user((void *)g_HalLogBuf, (const void __user *)arg, 128)) {
				ret = -EFAULT;
				rtd_pr_vdec_err("rtkvdec ioctl code=VDEC_IOC_HALLOG failed!!!!!!!!!!!!!!!\n");
				goto out;
			}
			rtd_pr_vdec_info("%s\n", g_HalLogBuf);
			break;
		}

		case VDEC_IOC_QVCDONE:
		{
			rtd_pr_vdec_debug("rtkvdec: VDEC_IOC_QVCDONE %d\n", VideoCreatFinish);
			ret = VideoCreatFinish ;
			break;
		}

		case VDEC_IOC_QUERY_NOV:
		{
			rtd_pr_vdec_debug("rtkvdec: VDEC_IOC_QUERY_NOV\n");
			ret = rtkvdec_query_num_of_vcpu();
			break;
		}
		case VDEC_IOC_SVP_DMABUF_ALLOCATE:
		case VDEC_IOC_SVP_DMABUF_GET_INFO:
		case VDEC_IOC_SVP_DMABUF_START_RINGBUF_UPDATE:
		case VDEC_IOC_SVP_DMABUF_COMMIT_RINGBUF_UPDATE:
		{
			ret = rtk_vdec_dmabuf_ioctl(filp, cmd, arg);
			break;
		}
		default:
		{
			rtd_pr_vdec_debug("rtkvdec: unknown ioctl(0x%08x)\n", cmd);
			break;
		}
	};

	out:
	return ret;
}
#endif
#endif


int rtkvdec_open(struct inode *inode, struct file *filp)
{
	VDEC_FRAMEINFO_STRUCT *frame_info ;
	if (!filp->private_data) {
		VDEC_FRAMEINFO_LIST *frame_info_list = kmalloc(sizeof(VDEC_FRAMEINFO_LIST), GFP_KERNEL);
		if (frame_info_list == NULL) {
			rtd_pr_vdec_err("%s: failed to allocate VDEC_FRAMEINFO_LIST\n", __func__);
			return -ENOMEM;
		}
		INIT_LIST_HEAD(&frame_info_list->buffer_list);
		frame_info = &frame_info_list->frame_info ;
		frame_info->pid = current->tgid ;
		frame_info->frame_dropped =
		frame_info->frame_displaying =
		frame_info->frame_matched =
		frame_info->frame_freerun = 0 ;
		down(&gst_frame_info_list_sem) ;
		list_add(&frame_info_list->buffer_list, &gst_frame_info_head) ;
		up(&gst_frame_info_list_sem) ;
		filp->private_data = frame_info_list ;
	}
	return 0 ;
}

int rtkvdec_release(struct inode *inode, struct file *filp)
{
	if (filp->private_data) {
		VDEC_FRAMEINFO_LIST *frame_info_list = filp->private_data;
		struct list_head *del_list;
		VDEC_FRAMEINFO_LIST *buf_info_list;
		VDEC_FRAMEINFO_STRUCT *frame_info = &frame_info_list->frame_info ;

		int find = 0 ;
		down(&gst_frame_info_list_sem);
		list_for_each(del_list, &gst_frame_info_head) {
			buf_info_list = list_entry(del_list, VDEC_FRAMEINFO_LIST, buffer_list);
			if (frame_info_list == buf_info_list && buf_info_list->frame_info.pid == frame_info->pid) {
				find = 1 ;
				break;
			}
		}

		if (find) list_del(del_list);
		up(&gst_frame_info_list_sem);

		if (find) kfree(buf_info_list);
	}
	else {
		rtd_pr_vdec_warn("rtkvdec_release: filp->private_data is null!!\n");
	}
	return 0 ;
}

#define DCMT_ADDR_SIZE 6
    unsigned int DCMT_ADDCMD_ADDR[DCMT_ADDR_SIZE][2]={
        {DC_SYS_DC_MT_ADDCMD_HI_0_reg,DC_SYS_DC_MT_ADDCMD_LO_0_reg},
        {DC_SYS_DC_MT_ADDCMD_HI_SYS2_0_reg,DC_SYS_DC_MT_ADDCMD_LO_SYS2_0_reg},
        {DC_SYS_DC_MT_ADDCMD_HI_SYS3_0_reg,DC_SYS_DC_MT_ADDCMD_LO_SYS3_0_reg},
        {DC_SYS_DC_MTEX_ADDCMD_HI0_0_reg,DC_SYS_DC_MTEX_ADDCMD_LO_0_reg},
        {DC_SYS_DC_MTEX_ADDCMD_HI0_SYS2_0_reg,DC_SYS_DC_MTEX_ADDCMD_LO_SYS2_0_reg},
        {DC_SYS_DC_MTEX_ADDCMD_HI0_SYS3_0_reg,DC_SYS_DC_MTEX_ADDCMD_LO_SYS3_0_reg},
    };


static __maybe_unused void rtkvdec_DCMTCallbackFunction(void)
{
    unsigned int addcmd_lo_addr=0;
    unsigned int addcmd_hi_addr=0;
    unsigned int addcmd_lo2=0;
    unsigned int addcmd_hi=0;
    unsigned int isBlockMode=0xffff;
    int i=0, count=0;

    //check trash module
    if(!isDcmtTrap("SB2_VCPU"))
    {
        rtd_pr_vdec_err("not SB2_VCPU module!\n");
        return;
    }
/*
    Get BlockMode or Sequence mode
*/
    for(i=0 ;i<4 ; i++)
    {
        for(count=0 ;count<DCMT_ADDR_SIZE ;count++)
        {
            addcmd_hi_addr=DCMT_ADDCMD_ADDR[count][0];
            addcmd_lo_addr=DCMT_ADDCMD_ADDR[count][1];
            addcmd_lo2=rtd_inl(addcmd_lo_addr + (i<<2));
            addcmd_hi=rtd_inl(addcmd_hi_addr + (i<<2));
            if( addcmd_hi & (1 << 22) )//ever_trap bit: Indicate if ever trap.
            {
                 isBlockMode=addcmd_lo2 &0x0001;
                 i=4;//break other loop
                 break;
            }
        }
    }
    if(0xffff==isBlockMode)
        {
        rtd_pr_vdec_err("[VDEC]DCMT Error to get block mode or not\n");
        return;
    }
    //check block mode or sequence mode

    rtd_pr_vdec_err( "====VE DCMT reg dump isBlockMode=0x%x with addcmd=0x%x====\n",isBlockMode,addcmd_lo2);
    if(isBlockMode)//sequence mode
    {
        rtd_pr_vdec_err("[VDEC]DCMT block mode no message for now\n");
    }
    else
    {
#if 0
        rtd_pr_vdec_err( "====VE DCMT reg dump=====\n");
           rtd_pr_vdec_err( "====VE CODEBLKREG     =====\n");
            rtd_pr_vdec_err( "====CODEBLKREG_Base reg:%x, value=%x     \n",CODEBLKREG_ve_bsbase_reg,rtd_inl(CODEBLKREG_ve_bsbase_reg));
            rtd_pr_vdec_err( "====CODEBLKREG_Limit reg:%x, value=%x     \n",CODEBLKREG_ve_bslimit_reg,rtd_inl(CODEBLKREG_ve_bslimit_reg));
            rtd_pr_vdec_err( "====CODEBLKREG_Wptr reg:%x,  value=%x     \n",CODEBLKREG_ve_bswrptr_reg,rtd_inl(CODEBLKREG_ve_bswrptr_reg));
            rtd_pr_vdec_err( "====CODEBLKREG_Rptr reg:%x, value=%x    \n",CODEBLKREG_ve_bscurptr_reg,rtd_inl(CODEBLKREG_ve_bscurptr_reg));
            rtd_pr_vdec_err( "====CODEBLKREG_BSPTR reg:%x, value=%x    \n",CODEBLKREG_ve_decbsptr_reg,rtd_inl(CODEBLKREG_ve_decbsptr_reg));
            rtd_pr_vdec_err( "========================\n");
            rtd_pr_vdec_err( "====VE CABAC_P2_NDB =====\n");
            rtd_pr_vdec_err( "====CABAC_P2_NDB_CNTL reg:%x, value=%x     \n",CABAC_P2_p2_ndb_ctl_reg,rtd_inl(CABAC_P2_p2_ndb_ctl_reg));
            rtd_pr_vdec_err( "====P2_NDB_Base reg:%x, value=%x     \n",CABAC_P2_ndb_cabac_base_reg,rtd_inl(CABAC_P2_ndb_cabac_base_reg));
            rtd_pr_vdec_err( "====P2_NDB_Limit reg:%x, value=%x     \n",CABAC_P2_ndb_cabac_limit_reg,rtd_inl(CABAC_P2_ndb_cabac_limit_reg));
            rtd_pr_vdec_err("====P2_NDB_Wptr reg:%x, value=%x     \n",CABAC_P2_ndb_cabac_wptr_reg,rtd_inl(CABAC_P2_ndb_cabac_wptr_reg));
            rtd_pr_vdec_err("====P2_NDB_Rptr reg:%x, value=%x     \n",CABAC_P2_ndb_cabac_rptr_reg,rtd_inl(CABAC_P2_ndb_cabac_rptr_reg));
            rtd_pr_vdec_err( "====VE CODEBLKREG_CAVLC_NDB =====\n");
            rtd_pr_vdec_err( "====CAVLC_NDB_Base reg:%x, value=%x     \n",CODEBLKREG_ndb_cavlc_base_reg,rtd_inl(CODEBLKREG_ndb_cavlc_base_reg));
            rtd_pr_vdec_err( "====CAVLC_NDB_Limit reg:%x, value=%x     \n",CODEBLKREG_ndb_cavlc_limit_reg,rtd_inl(CODEBLKREG_ndb_cavlc_limit_reg));
            rtd_pr_vdec_err("====CAVLC_NDB_Wptr reg:%x, value=%x     \n",CODEBLKREG_ndb_cavlc_wptr_reg,rtd_inl(CODEBLKREG_ndb_cavlc_wptr_reg));
            rtd_pr_vdec_err("====CAVLC_NDB_Rptr reg:%x, value=%x     \n",CODEBLKREG_ndb_cavlc_rptr_reg,rtd_inl(CODEBLKREG_ndb_cavlc_rptr_reg));
            rtd_pr_vdec_err( "========================\n");
            rtd_pr_vdec_err("====VE CABAC_P2_P2=====\n");
            rtd_pr_vdec_err("====P2_INT_STATUS reg%x, value= 0x%x   \n",   CABAC_P2_cabacp2_int_status_reg,rtd_inl(CABAC_P2_cabacp2_int_status_reg));
            rtd_pr_vdec_err( "====P2_INT_EN reg%x, value= 0x%x   \n",            CABAC_P2_cabacp2_int_en_reg,  rtd_inl(CABAC_P2_cabacp2_int_en_reg));
            rtd_pr_vdec_err("====P2_WFIFO_BASE   reg%x, value= 0x%x   \n", CABAC_P2_p2_wfifo_base_reg, rtd_inl(CABAC_P2_p2_wfifo_base_reg));
            rtd_pr_vdec_err("====P2_WFIFO_LIMIT  reg%x, value= 0x%x   \n",CABAC_P2_p2_wfifo_limit_reg, rtd_inl(CABAC_P2_p2_wfifo_limit_reg));
            rtd_pr_vdec_err("====P2_WFIFO_WPTR   reg%x, value= 0x%x   \n", CABAC_P2_p2_wfifo_wptr_reg, rtd_inl(CABAC_P2_p2_wfifo_wptr_reg));
            rtd_pr_vdec_err("====P2_WFIFO_RPTR   reg%x, value= 0x%x   \n",CABAC_P2_p2_wfifo_rptr_reg,  rtd_inl(CABAC_P2_p2_wfifo_rptr_reg));
            rtd_pr_vdec_err( "========================\n");
            rtd_pr_vdec_err("====P2_RFIFO_BASE   reg%x, value= 0x%x   \n",   CABAC_P2_p2_rfifo_base_reg, rtd_inl(CABAC_P2_p2_rfifo_base_reg));
            rtd_pr_vdec_err("====P2_RFIFO_LIMIT  reg%x, value= 0x%x   \n",  CABAC_P2_p2_rfifo_limit_reg, rtd_inl(CABAC_P2_p2_rfifo_limit_reg));
            rtd_pr_vdec_err("====P2_RFIFO_WPTR   reg%x, value= 0x%x   \n", CABAC_P2_p2_rfifo_wptr_reg, rtd_inl(CABAC_P2_p2_rfifo_wptr_reg));
            rtd_pr_vdec_err("====P2_RFIFO_RPTR   reg%x, value= 0x%x   \n",  CABAC_P2_p2_rfifo_rptr_reg,  rtd_inl(CABAC_P2_p2_rfifo_rptr_reg));
            rtd_pr_vdec_err("====P2_RFIFO_BSPTR  reg%x, value= 0x%x   \n", CABAC_P2_p2_rfifo_decbsptr_reg, rtd_inl(CABAC_P2_p2_rfifo_decbsptr_reg));
            rtd_pr_vdec_err("========================\n");
            rtd_pr_vdec_err("====SEG TBCMD  reg%x, value= 0x%x   \n", SEG_tbcmd_reg, rtd_inl(SEG_tbcmd_reg));
            rtd_pr_vdec_err("====SEG SLCCFG  reg%x, value= 0x%x   \n", SEG_slccfg_reg, rtd_inl(SEG_slccfg_reg));
            rtd_pr_vdec_err("========================\n");
            rtd_pr_vdec_err("====MVDBLK H264CO1  reg%x, value= 0x%x   \n", MVDBLK_h264co1_reg, rtd_inl(MVDBLK_h264co1_reg));
            rtd_pr_vdec_err("====MVDBLK H264CO2  reg%x, value= 0x%x   \n", MVDBLK_h264co2_reg, rtd_inl(MVDBLK_h264co2_reg));
            rtd_pr_vdec_err("====MVDBLK H264NH1  reg%x, value= 0x%x   \n", MVDBLK_h264nh1_reg, rtd_inl(MVDBLK_h264nh1_reg));
            rtd_pr_vdec_err("====MVDBLK H264NH2  reg%x, value= 0x%x   \n", MVDBLK_h264nh2_reg, rtd_inl(MVDBLK_h264nh2_reg));
            rtd_pr_vdec_err("====MVDBLK HEVCSAO  reg%x, value= 0x%x   \n", MVDBLK_hevcsao_reg, rtd_inl(MVDBLK_hevcsao_reg));
            rtd_pr_vdec_err("====MVDBLK HEVCSAO1  reg%x, value= 0x%x   \n", MVDBLK_hevcsao1_reg, rtd_inl(MVDBLK_hevcsao1_reg));
            rtd_pr_vdec_err("========================\n");
            rtd_pr_vdec_err("====RL_hdr_str_addr_lu_0  reg%x, value= 0x%x   \n", RL_hdr_str_addr_lu_0_reg, rtd_inl(RL_hdr_str_addr_lu_0_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_lu_1  reg%x, value= 0x%x   \n", RL_hdr_str_addr_lu_1_reg, rtd_inl(RL_hdr_str_addr_lu_1_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_lu_2  reg%x, value= 0x%x   \n", RL_hdr_str_addr_lu_2_reg, rtd_inl(RL_hdr_str_addr_lu_2_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_lu_3  reg%x, value= 0x%x   \n", RL_hdr_str_addr_lu_3_reg, rtd_inl(RL_hdr_str_addr_lu_3_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_lu_4  reg%x, value= 0x%x   \n", RL_hdr_str_addr_lu_4_reg, rtd_inl(RL_hdr_str_addr_lu_4_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_lu_5  reg%x, value= 0x%x   \n", RL_hdr_str_addr_lu_5_reg, rtd_inl(RL_hdr_str_addr_lu_5_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_lu_6  reg%x, value= 0x%x   \n", RL_hdr_str_addr_lu_6_reg, rtd_inl(RL_hdr_str_addr_lu_6_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_lu_7  reg%x, value= 0x%x   \n", RL_hdr_str_addr_lu_7_reg, rtd_inl(RL_hdr_str_addr_lu_7_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_lu_8  reg%x, value= 0x%x   \n", RL_hdr_str_addr_lu_8_reg, rtd_inl(RL_hdr_str_addr_lu_8_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_lu_9  reg%x, value= 0x%x   \n", RL_hdr_str_addr_lu_9_reg, rtd_inl(RL_hdr_str_addr_lu_9_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_lu_10  reg%x, value= 0x%x   \n", RL_hdr_str_addr_lu_10_reg, rtd_inl(RL_hdr_str_addr_lu_10_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_lu_11  reg%x, value= 0x%x   \n", RL_hdr_str_addr_lu_11_reg, rtd_inl(RL_hdr_str_addr_lu_11_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_lu_12  reg%x, value= 0x%x   \n", RL_hdr_str_addr_lu_12_reg, rtd_inl(RL_hdr_str_addr_lu_12_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_lu_13  reg%x, value= 0x%x   \n", RL_hdr_str_addr_lu_13_reg, rtd_inl(RL_hdr_str_addr_lu_13_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_lu_14  reg%x, value= 0x%x   \n", RL_hdr_str_addr_lu_14_reg, rtd_inl(RL_hdr_str_addr_lu_14_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_lu_15  reg%x, value= 0x%x   \n", RL_hdr_str_addr_lu_15_reg, rtd_inl(RL_hdr_str_addr_lu_15_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_lu_16  reg%x, value= 0x%x   \n", RL_hdr_str_addr_lu_16_reg, rtd_inl(RL_hdr_str_addr_lu_16_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_lu_17  reg%x, value= 0x%x   \n", RL_hdr_str_addr_lu_17_reg, rtd_inl(RL_hdr_str_addr_lu_17_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_lu_18  reg%x, value= 0x%x   \n", RL_hdr_str_addr_lu_18_reg, rtd_inl(RL_hdr_str_addr_lu_18_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_lu_19  reg%x, value= 0x%x   \n", RL_hdr_str_addr_lu_19_reg, rtd_inl(RL_hdr_str_addr_lu_19_reg));
            rtd_pr_vdec_err("========================\n");
            rtd_pr_vdec_err("====RL_hdr_str_addr_ch_0  reg%x, value= 0x%x   \n", RL_hdr_str_addr_ch_0_reg, rtd_inl(RL_hdr_str_addr_ch_0_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_ch_1  reg%x, value= 0x%x   \n", RL_hdr_str_addr_ch_1_reg, rtd_inl(RL_hdr_str_addr_ch_1_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_ch_2  reg%x, value= 0x%x   \n", RL_hdr_str_addr_ch_2_reg, rtd_inl(RL_hdr_str_addr_ch_2_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_ch_3  reg%x, value= 0x%x   \n", RL_hdr_str_addr_ch_3_reg, rtd_inl(RL_hdr_str_addr_ch_3_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_ch_4  reg%x, value= 0x%x   \n", RL_hdr_str_addr_ch_4_reg, rtd_inl(RL_hdr_str_addr_ch_4_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_ch_5  reg%x, value= 0x%x   \n", RL_hdr_str_addr_ch_5_reg, rtd_inl(RL_hdr_str_addr_ch_5_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_ch_6  reg%x, value= 0x%x   \n", RL_hdr_str_addr_ch_6_reg, rtd_inl(RL_hdr_str_addr_ch_6_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_ch_7  reg%x, value= 0x%x   \n", RL_hdr_str_addr_ch_7_reg, rtd_inl(RL_hdr_str_addr_ch_7_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_ch_8  reg%x, value= 0x%x   \n", RL_hdr_str_addr_ch_8_reg, rtd_inl(RL_hdr_str_addr_ch_8_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_ch_9  reg%x, value= 0x%x   \n", RL_hdr_str_addr_ch_9_reg, rtd_inl(RL_hdr_str_addr_ch_9_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_ch_10  reg%x, value= 0x%x   \n", RL_hdr_str_addr_ch_10_reg, rtd_inl(RL_hdr_str_addr_ch_10_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_ch_11  reg%x, value= 0x%x   \n", RL_hdr_str_addr_ch_11_reg, rtd_inl(RL_hdr_str_addr_ch_11_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_ch_12  reg%x, value= 0x%x   \n", RL_hdr_str_addr_ch_12_reg, rtd_inl(RL_hdr_str_addr_ch_12_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_ch_13  reg%x, value= 0x%x   \n", RL_hdr_str_addr_ch_13_reg, rtd_inl(RL_hdr_str_addr_ch_13_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_ch_14  reg%x, value= 0x%x   \n", RL_hdr_str_addr_ch_14_reg, rtd_inl(RL_hdr_str_addr_ch_14_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_ch_15  reg%x, value= 0x%x   \n", RL_hdr_str_addr_ch_15_reg, rtd_inl(RL_hdr_str_addr_ch_15_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_ch_16  reg%x, value= 0x%x   \n", RL_hdr_str_addr_ch_16_reg, rtd_inl(RL_hdr_str_addr_ch_16_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_ch_17  reg%x, value= 0x%x   \n", RL_hdr_str_addr_ch_17_reg, rtd_inl(RL_hdr_str_addr_ch_17_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_ch_18  reg%x, value= 0x%x   \n", RL_hdr_str_addr_ch_18_reg, rtd_inl(RL_hdr_str_addr_ch_18_reg));
            rtd_pr_vdec_err("====RL_hdr_str_addr_ch_19  reg%x, value= 0x%x   \n", RL_hdr_str_addr_ch_19_reg, rtd_inl(RL_hdr_str_addr_ch_19_reg));
            rtd_pr_vdec_err("========================\n");
            rtd_pr_vdec_err("====Pred BLK SDMAADDR  reg%x, value= 0x%x   \n", PREDBLK_SDMAADR_reg, rtd_inl(PREDBLK_SDMAADR_reg));
            rtd_pr_vdec_err("====Pred BLK DMAMSB   reg%x, value= 0x%x   \n", PREDBLK_DMAMSB_reg, rtd_inl(PREDBLK_DMAMSB_reg));
            rtd_pr_vdec_err("====DEBLK SDMAADDR  reg%x, value= 0x%x   \n", DEBLK_SDMAADR_reg, rtd_inl(DEBLK_SDMAADR_reg));
            //rtd_pr_vdec_err("====SAO DMAMSB   reg%x, value= 0x%x   \n", SAO_SAO_DMA_reg, rtd_inl(SAO_SAO_DMA_reg));
            rtd_pr_vdec_err("====CMPRS HDAADDR_0  reg%x, value= 0x%x   \n", CMPRS_HDMAADR_0_reg, rtd_inl(CMPRS_HDMAADR_0_reg));
            rtd_pr_vdec_err("====CMPRS HDAADDR_1   reg%x, value= 0x%x   \n", CMPRS_HDMAADR_1_reg, rtd_inl(CMPRS_HDMAADR_1_reg));
            rtd_pr_vdec_err("========================\n");
#endif
    }
}

void rtkvdec_help(void)
{
	rtd_pr_vdec_notice(" echo f > /dev/rtkvdec          ## print video f/w alloc memory summary\n");
	rtd_pr_vdec_notice(" echo sdf@ [s] > /dev/rtkvdec   ## debug: set string [s] as dump file name\n");
	rtd_pr_vdec_notice(" echo dlg > /dev/rtkvdec        ## debug: enable write video fw log to file\n");
	rtd_pr_vdec_notice(" echo dno > /dev/rtkvdec        ## debug: disable write video fw log to file\n");
	rtd_pr_vdec_notice(" echo des_en [s] > /dev/rtkvdec ## debug: enable dump es to file[S]\n");
	rtd_pr_vdec_notice(" echo des_di	 > /dev/rtkvdec    ## debug: disable dump es to file\n");
	rtd_pr_vdec_notice(" echo fw@ [s] > /dev/rtkvdec    ## debug: send a string [s] to f/w\n");
	rtd_pr_vdec_notice(" echo fw1@ [s] > /dev/rtkvdec   ## debug: send a string [s] to f/w(VCPU1)\n");
	rtd_pr_vdec_notice(" echo fw2@ [s] > /dev/rtkvdec   ## debug: send a string [s] to f/w(VCPU2)\n");
}

void rtkvdec_set_dump_file(const char *pattern, int length)
{
	if (length > FILE_NAME_SIZE - 2) {
		rtd_pr_vdec_warn("rtkvdec: file name is too long(<%d).\n", FILE_NAME_SIZE - 1);
		return;
	}

	rtkvdec_memset((void *)dbg_config[0].file_name, '\0', length+1);
	rtkvdec_memcpy((void *)dbg_config[0].file_name, (void *)pattern, length);
	rtkvdec_memset((void *)dbg_config[1].file_name, '\0', length+2);
	rtkvdec_memset((void *)dbg_config[1].file_name, '2', length+1);
	rtkvdec_memcpy((void *)dbg_config[1].file_name, (void *)pattern, length);
	rtd_pr_vdec_notice("rtkvdec: dump_file_name(%s)\n", dbg_config[0].file_name);
	rtd_pr_vdec_notice("rtkvdec: dump_file_name(%s)\n", dbg_config[1].file_name);
	return;
}

void rtkvdec_send_string(const char *pattern, int length, int send_to)
{
	unsigned long ret = S_OK;
	unsigned long vir_addr;
	unsigned long phy_addr;
	char *command = NULL;

	vir_addr = (unsigned long)dvr_malloc_uncached_specific(length+1, GFP_DCU1_LIMIT, (void **)&command);
	if (!vir_addr) {
		rtd_pr_vdec_err("[%s %d]alloc string memory fail\n",__FUNCTION__,__LINE__);
		return;
	}
	phy_addr = (unsigned long)dvr_to_phys((void *)vir_addr);
	rtkvdec_memset((void *)command, '\0', length+1);
	rtkvdec_memcpy((void *)command, (void *)pattern, length);

	rtd_pr_vdec_debug("rtkvdec: debug string(%s), length(%d)\n", command, length);
	if (send_to & 0x1)
		if (send_rpc_command(RPC_VIDEO, RPC_VCPU_DEBUG_COMMAND, phy_addr, length, &ret))
			rtd_pr_vdec_err("rtkvdec: debug string(%s) fail %d\n", pattern, (int)ret);
#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
	if (send_to & 0x2)
		if (send_rpc_command(RPC_VIDEO2, RPC_VCPU_DEBUG_COMMAND, phy_addr, length, &ret))
			rtd_pr_vdec_err("rtkvdec: V2 debug string(%s) fail %d\n", pattern, (int)ret);
#endif

	dvr_free((void *)vir_addr);
	return;
}

void rtkvdec_vtm_state(void)
{
	char cmd_buf[10] = "vtms";
	rtkvdec_send_string(cmd_buf, 4, 0x1);
}
EXPORT_SYMBOL_GPL(rtkvdec_vtm_state);


ssize_t rtkvdec_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	long ret = count;
	char cmd_buf[100] = {0};

	rtd_pr_vdec_debug("%s(): count=%lx, buf=%lx\n", __func__, ret, (unsigned long)buf);

	if (count >= 100)
		return -EFAULT;

	if (copy_from_user(cmd_buf, buf, count)) {
		ret = -EFAULT;
	}

	if ((cmd_buf[0] == 'f') && (count == 2)) {
		rtd_pr_vdec_notice("\n=== rtkvdec: show f/w memory usage ===\n");
		rtkvdec_fw_remote_malloc_summary(NULL);
		rtd_pr_vdec_notice("\n=======================================\n");
	} else if ((cmd_buf[0] == 'u') && (count == 2)) {
		rtd_pr_vdec_notice("\n=== rtkvdec: show usr memory usage ===\n");
		rtkvdec_user_alloc_summary(NULL);
		rtd_pr_vdec_notice("\n=======================================\n");
	} else if ((cmd_buf[0] == 's') && (count == 2)) {
		rtd_pr_vdec_notice("\n=== rtkvdec show svp & ibbuf state: ==============\n");
		rtkvdec_show_svp_status();
		rtkvdec_show_ibbuf_status();
		rtd_pr_vdec_notice("\n=======================================\n");
	} else if ((cmd_buf[0] == 'f') && (cmd_buf[1] == 'r') && (cmd_buf[2] == 'm') && (count == 4)) {
		rtd_pr_vdec_notice("\n=== rtkvdec show frame buf: ==============\n");
		//rtkvdec_vbm_by_frame_testcase1();
		//rtkvdec_vbm_by_frame_testcase2();
		//rtkvdec_vbm_by_frame_testcase3();
		//rtkvdec_vbm_by_frame_testcase4();
		rtkvdec_vbm_by_frame_summary();
		rtd_pr_vdec_notice("\n=======================================\n");
	} else if ((cmd_buf[0] == 'h') && (count == 2)) {
		rtd_pr_vdec_notice("\n=== rtkvdec write usage: ==============\n");
		rtkvdec_help();
		rtd_pr_vdec_notice("\n=======================================\n");
	} else if ((cmd_buf[0] == 'd') && (count == 2)) {
		rtd_pr_vdec_notice("\n=== rtkvdec debug: ==============\n");
		rtkvdec_show_sharedmemory();
		rtd_pr_vdec_notice("\n=======================================\n");
	} else if ((cmd_buf[0] == 's') && (cmd_buf[1] == 't') && (cmd_buf[2] == 'a') && (cmd_buf[3] == 'c') && (cmd_buf[4] == 'k') && (count == 6)) {
		rtd_pr_vdec_notice("\n=== rtkvdec calltrace: ==============\n");
		rtkvdec_show_calltrace();
		rtd_pr_vdec_notice("\n=======================================\n");
	} else if ((cmd_buf[0] == 's') && (cmd_buf[1] == 'd') && (cmd_buf[2] == 'f') && (cmd_buf[3] == '@')) {
		rtd_pr_vdec_notice("\n=== rtkvdec: set dump file name: =============\n");
		rtkvdec_set_dump_file(&cmd_buf[5], (count-5)-1);
		rtd_pr_vdec_notice("\n=======================================\n");
	} else if ((cmd_buf[0] == 'd') && (cmd_buf[1] == 'l') && (cmd_buf[2] == 'g')) {
		if (count == 4) {
		}
		else if ((int) cmd_buf[4] > 47 && (int) cmd_buf[4] < 56 && (count == 6)) {
			rtkvdec_dump_size_scale = (int) cmd_buf[4] - 48;
		}
		rtd_pr_vdec_notice("rtkvdec_dump_size_scale %d MB\n", rtkvdec_dump_size_scale);
		rtd_pr_vdec_notice("\n=== rtkvdec: enable dump video fw log =============\n");
		rtkvdec_dump_enable();
		rtd_pr_vdec_notice("\n=======================================\n");
	} else if ((cmd_buf[0] == 'd') && (cmd_buf[1] == 'n') && (cmd_buf[2] == 'o') && (count == 4)) {
		rtd_pr_vdec_notice("\n=== rtkvdec: disable dump video fw log =============\n");
		rtkvdec_dump_disable();
		rtd_pr_vdec_notice("\n=======================================\n");
	} else if ((cmd_buf[0] == 'd') && (cmd_buf[1] == 'e') && (cmd_buf[2] == 's') && (cmd_buf[3] == '_')) {
		if ((cmd_buf[4] == 'e') && (cmd_buf[5] == 'n')) {
			rtd_pr_vdec_notice("\n=== rtkvdec: enable dump es =============\n");
			rtkvdec_dumpes_enable(&cmd_buf[7], (count-7)-1);
			rtd_pr_vdec_notice("\n=======================================\n");
		}
		else if ((cmd_buf[4] == 'd') && (cmd_buf[5] == 'i')) {
			rtd_pr_vdec_notice("\n=== rtkvdec: disable dump es =============\n");
			rtkvdec_dumpes_disable();
			rtd_pr_vdec_notice("\n=======================================\n");
		}
		else {
			rtd_pr_vdec_notice("%s\n", cmd_buf);
		}
	} else if ((cmd_buf[0] == 'c') && (cmd_buf[1] == 'r') && (cmd_buf[2] == 'c') && (cmd_buf[3] == '_') && (cmd_buf[4] == 'e') && (cmd_buf[5] == 'x') && (cmd_buf[6] == '_')) {
		if ((cmd_buf[7] == 'e') && (cmd_buf[8] == 'n')) {
			rtd_pr_vdec_notice("\n=== rtkvdec: enable export crc =============\n");
			rtkvdec_exportcrc_enable(&cmd_buf[10], (count-10)-1);
			rtd_pr_vdec_notice("\n=======================================\n");
		}
		else if ((cmd_buf[7] == 'd') && (cmd_buf[8] == 'i')) {
			rtd_pr_vdec_notice("\n=== rtkvdec: disable export crc =============\n");
			rtkvdec_exportcrc_disable();
			rtd_pr_vdec_notice("\n=======================================\n");
		}
		else {
			rtd_pr_vdec_notice("%s\n", cmd_buf);
		}
	} else if ((cmd_buf[0] == 'c') && (cmd_buf[1] == 'r') && (cmd_buf[2] == 'c') && (cmd_buf[3] == '_') && (cmd_buf[4] == 'i') && (cmd_buf[5] == 'm') && (cmd_buf[6] == '_')) {
		if ((cmd_buf[7] == 'e') && (cmd_buf[8] == 'n')) {
			rtd_pr_vdec_notice("\n=== rtkvdec: enable import crc =============\n");
			rtkvdec_importcrc_enable(&cmd_buf[10], (count-10)-1);
			rtd_pr_vdec_notice("\n=======================================\n");
		}
		else if ((cmd_buf[7] == 'd') && (cmd_buf[8] == 'i')) {
			rtd_pr_vdec_notice("\n=== rtkvdec: disable import crc =============\n");
			rtkvdec_importcrc_disable();
			rtd_pr_vdec_notice("\n=======================================\n");
		}
		else {
			rtd_pr_vdec_notice("%s\n", cmd_buf);
		}
	} else if ((cmd_buf[0] == 'p') && (cmd_buf[1] == 'r') &&
			   (cmd_buf[2] == 'd') && (cmd_buf[3] == 'm') &&
			   (cmd_buf[4] == 'g') && (count == 8)) {
		if (cmd_buf[6] == '0')
			rtkvdec_pr_to_dmesg_en = 0;
		else if (cmd_buf[6] == '1')
			rtkvdec_pr_to_dmesg_en = 1;
	} else if ((count == 8) && /* "ibdbg 1\n", "ibdbg 0\n" */
		(cmd_buf[0] == 'i') && (cmd_buf[1] == 'b') && (cmd_buf[2] == 'd') &&
		(cmd_buf[3] == 'b') && (cmd_buf[4] == 'g') && (cmd_buf[5] == ' ')) {
		extern unsigned int ibbuf_debug_en;
		ibbuf_debug_en = cmd_buf[6] == '1' ? 1 : 0;
		rtd_pr_vdec_notice("\n=== rtkvdec: enable ib debug===========\n");
		rtd_pr_vdec_notice("inband debug = %d\n", ibbuf_debug_en);
		rtd_pr_vdec_notice("\n=======================================\n");
	} else if ((cmd_buf[0] == 'f') && (cmd_buf[1] == 'w') && (cmd_buf[2] == '@')) {
		rtd_pr_vdec_notice("\n=== rtkvdec: send debug string: =============\n");
		rtkvdec_send_string(&cmd_buf[4], (count-4)-1, 0x3);
		rtd_pr_vdec_notice("\n=======================================\n");
	} else if ((cmd_buf[0] == 'f') && (cmd_buf[1] == 'w') && (cmd_buf[2] == '1') && (cmd_buf[3] == '@')) {
		rtd_pr_vdec_notice("\n=== rtkvdec: send debug string: =============\n");
		rtkvdec_send_string(&cmd_buf[5], (count-5)-1, 0x1);
		rtd_pr_vdec_notice("\n=======================================\n");
	} else if ((cmd_buf[0] == 'f') && (cmd_buf[1] == 'w') && (cmd_buf[2] == '2') && (cmd_buf[3] == '@')) {
		rtd_pr_vdec_notice("\n=== rtkvdec: send debug string: =============\n");
		rtkvdec_send_string(&cmd_buf[5], (count-5)-1, 0x2);
		rtd_pr_vdec_notice("\n=======================================\n");
	} else {
		rtd_pr_vdec_notice("%s\n", cmd_buf);
	}

	return ret;
}

static ssize_t rtkvdec_proc_read( struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	char *str = kmalloc(4096, GFP_KERNEL);
	struct list_head *del_list;
	VDEC_FRAMEINFO_LIST *buf_info_list;
	VDEC_FRAMEINFO_STRUCT *buf_info ;
	int len, total = 0 ;
	len = sprintf(str, "frame_info\n");
	total += len ;

	down(&gst_frame_info_list_sem);
	list_for_each(del_list, &gst_frame_info_head) {
		buf_info_list = list_entry(del_list, VDEC_FRAMEINFO_LIST, buffer_list);
		buf_info = &buf_info_list->frame_info ;
		len = sprintf(str + total, "pid=%d\n", buf_info->pid);
		total += len ;
		len = sprintf(str + total, "frame_dropped=%d\n", buf_info->frame_dropped);
		total += len ;
		len = sprintf(str + total, "frame_displaying=%d\n", buf_info->frame_displaying);
		total += len ;
		len = sprintf(str + total, "frame_matched=%d\n", buf_info->frame_matched);
		total += len ;
		len = sprintf(str + total, "frame_freerun=%d\n\n", buf_info->frame_freerun);
		total += len ;
	}
	up(&gst_frame_info_list_sem);

	if (copy_to_user(buf, str, total)) {
		rtd_pr_vdec_err("rtkvdec_debug:%d Copy proc data to user space failed\n", __LINE__);
		kfree(str);
		return 0;
	}

	if (*ppos == 0)
		*ppos += total;
	else
		total = 0;

	kfree(str);
	return total;
}


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
static const struct proc_ops rtkvdec_proc_fops = {
	.proc_read = rtkvdec_proc_read,
};
#else
static const struct file_operations rtkvdec_proc_fops = {
	.owner = THIS_MODULE,
	.read = rtkvdec_proc_read,
};
#endif


struct proc_dir_entry *rtkvdec_proc_dir;
struct proc_dir_entry *rtkvdec_proc_entry;

bool rtkvdec_debug_proc_init(void)
{
	if (rtkvdec_proc_dir == NULL && rtkvdec_proc_entry == NULL) {
		rtkvdec_proc_dir = proc_mkdir(RTKVDEC_PROC_DIR , NULL);

		if (rtkvdec_proc_dir != NULL) {
			rtkvdec_proc_entry =
				proc_create(RTKVDEC_PROC_ENTRY, 0666,
					rtkvdec_proc_dir, &rtkvdec_proc_fops);

			if (rtkvdec_proc_entry == NULL) {
				proc_remove(rtkvdec_proc_dir);
				rtkvdec_proc_dir = NULL;
				rtd_pr_vdec_err("failed to get proc entry for %s/%s\n", RTKVDEC_PROC_DIR, RTKVDEC_PROC_ENTRY);
				return false;
			}
		} else {
			rtd_pr_vdec_err("create rtkvdec dir proc entry (%s) failed\n",
				RTKVDEC_PROC_DIR);
			return false;
		}
	}

	return true;
}

bool rtkvdec_debug_proc_uninit(void)
{
	if (rtkvdec_proc_entry) {
		proc_remove(rtkvdec_proc_entry);
		rtkvdec_proc_entry = NULL;
	}

	if (rtkvdec_proc_dir) {
		proc_remove(rtkvdec_proc_dir);
		rtkvdec_proc_dir = NULL;
	}
	return true;
}

struct file_operations rtkvdec_fops = {
	.owner  		= THIS_MODULE,
	.unlocked_ioctl = rtkvdec_ioctl,
#ifdef CONFIG_ARM64
#ifdef CONFIG_COMPAT
	.compat_ioctl   = rtkvdec_ioctl_compat,
#endif
#endif
	.write  		= rtkvdec_write,
	.open   		= rtkvdec_open,
	.release		= rtkvdec_release,
};

static char *rtkvdec_devnode(struct device *dev, umode_t *mode)
{
	*mode = 0666;
	return NULL;
}

DCMT_DEBUG_INFO_DECLARE(DCMT_VDEC,rtkvdec_DCMTCallbackFunction);
//DCMT_DEBUG_INFO_UNREGISTER(DCMT_VDEC,rtkvdec_DCMTCallbackFunction);

static int rtkvdec_init(void)
{
	int result;
	dev_t dev = 0;

	rtd_pr_vdec_notice("\n\n\n\n *****************  rtkvdec init module  *********************\n\n\n\n");
	if (rtkvdec_major) {
		dev = MKDEV(rtkvdec_major, rtkvdec_minor);
		result = register_chrdev_region(dev, 1, "rtkvdec");
	} else {
		result = alloc_chrdev_region(&dev, rtkvdec_minor, 1, "rtkvdec");
		rtkvdec_major = MAJOR(dev);
	}

	if (result < 0) {
		rtd_pr_vdec_debug("rtkvdec: can not get chrdev region...\n");
		return result;
	}

	DCMT_DEBUG_INFO_REGISTER(DCMT_VDEC,rtkvdec_DCMTCallbackFunction);

	/* rtd_pr_vdec_debug("rtkvdec init module major number = %d\n", rtkvdec_major); */

	rtkvdec_class = class_create(THIS_MODULE, "rtkvdec");
	if (IS_ERR(rtkvdec_class)) {
		rtd_pr_vdec_debug("rtkvdec: can not create class...\n");
		result = PTR_ERR(rtkvdec_class);
		goto fail_class_create;
	}

	rtkvdec_class->devnode = rtkvdec_devnode ;

	rtkvdec_platform_devs = platform_device_register_simple("rtkvdec", -1, NULL, 0);
	if (platform_driver_register(&rtkvdec_device_driver) != 0) {
		rtd_pr_vdec_debug("rtkvdec: can not register platform driver...\n");
		result = -EINVAL;
		goto fail_platform_driver_register;
	}

	cdev_init(&rtkvdec_cdev, &rtkvdec_fops);
	rtkvdec_cdev.owner = THIS_MODULE;
	rtkvdec_cdev.ops = &rtkvdec_fops;
	result = cdev_add(&rtkvdec_cdev, dev, 1);
	if (result < 0) {
		rtd_pr_vdec_debug("rtkvdec: can not add character device...\n");
		goto fail_cdev_init;
	}
	rtkvdec_device = device_create(rtkvdec_class, NULL, dev, NULL, "rtkvdec");

	rtkvdec_device->dma_mask = &rtkvdec_device->coherent_dma_mask;
	if(dma_set_mask(rtkvdec_device, DMA_BIT_MASK(32))) {
		rtd_pr_vdec_err("DMA not supported\n");
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)) && defined(CONFIG_ARM64)
	arch_setup_dma_ops(rtkvdec_device, 0, 0, NULL, false);
#endif

	INIT_LIST_HEAD(&vfw_remote_malloc_info_head);
	INIT_LIST_HEAD(&vfw_remote_malloc_info_specific_head);
	INIT_LIST_HEAD(&gst_frame_info_head);
	INIT_LIST_HEAD(&rtkvdec_alloc_head);

	/* register RPC command ID 0 mapping handler */
	if (register_kernel_rpc(RPC_VCPU_ID_0x102_rtkvdec_remote_malloc, rtkvdec_remote_malloc) == 1)
		rtd_pr_vdec_debug("Register rtkvdec_remote_malloc failed!\n");
	if (register_kernel_rpc(RPC_VCPU_ID_0x103_rtkvdec_remote_free, rtkvdec_remote_free) == 1)
		rtd_pr_vdec_debug("Register rtkvdec_remote_free failed!\n");
	if (register_kernel_rpc(RPC_VCPU_ID_0x106_rtkvdec_remote_malloc_specific, rtkvdec_remote_malloc_specific) == 1)
		rtd_pr_vdec_debug("Register rtkvdec_remote_malloc_specific failed!\n");
	if (register_kernel_rpc(RPC_VCPU_ID_0x107_rtkvdec_remote_free_specific, rtkvdec_remote_free_specific) == 1)
		rtd_pr_vdec_debug("Register rtkvdec_remote_free_specific failed!\n");
	if (register_kernel_rpc(RPC_VCPU_FREE_USER_MEM, rtkvdec_free_user_mem) == 1)
		rtd_pr_vdec_debug("Register rtkvdec_RPC_VCPU_FREE_USER_MEM failed!\n");
	if (register_kernel_rpc(RPC_VCPU_ID_0x109_rtkvdec_set_command, rtkvdec_set_command) == 1)
		rtd_pr_vdec_debug("Register rtkvdec_set_command failed!\n");

	rtkvdec_dump_init() ;
	rtkvdec_mm_init();
	if (!rtk_vdec_dma_heap_create()) {
		rtd_pr_vdec_debug("rtk_vdec_dma_heap_create failed!\n");
	};
#if IS_ENABLED(CONFIG_RTK_KDRV_TEE)
	rtkvdec_svp_early_init();
	rtkvdec_svp_laycout_sync_tee();

#endif

	EncoderMemAddr = rtkvdec_remote_malloc(0x80000, 0x90ABCDEF) ;
	if (!EncoderMemAddr) {
		goto fail_cdev_init;
	}
	EncoderMemState = 0 ;

	rtkvdec_vcreate_tsk = kthread_create(rtkvdec_vcreate_thread, &data2, "rtkvdec_vcreate_thread");
	if (IS_ERR(rtkvdec_vcreate_tsk)) {
		result = PTR_ERR(rtkvdec_vcreate_tsk);
		rtkvdec_vcreate_tsk = NULL;
		goto fail_cdev_init;
	}

	rtkvdec_debug_proc_init();

#ifdef CONFIG_LG_SNAPSHOT_BOOT
	if(snapshot_enable == 0)
		wake_up_process(rtkvdec_vcreate_tsk);
#else
	wake_up_process(rtkvdec_vcreate_tsk);
#endif

#if 0
	/*[WOSQRTK-4745]fix logo garbage.*/ /*[WOSQRTK-6662]increase logo dealy.*/
	if((bstoplogo_gdma == false) && (is_RTD_K2L() == 0) && Get_DISPLAY_PANEL_CUSTOM_INDEX() != VBY_ONE_PANEL_innolux_4K2K_600M_1S_8L )
	{
		bstoplogo_gdma = true;
		GDMA_ConfigOSDxMixerEnableNoDelay(GDMA_PLANE_OSD2,0);
	}
	else if((bstoplogo_gdma == false) && (is_RTD_K2L() == 0) && (Get_DISPLAY_PANEL_CUSTOM_INDEX() == VBY_ONE_PANEL_innolux_4K2K_600M_1S_8L ) )
	{
		bstoplogo_gdma = true;
		//set tvb4 prority for OSD2 [WOSQRTK-4745]fix logo garbage
		org_tvb4_pro_1 = rtd_inl(DC_PRIORITY_CTRLB_VADDR);
		org_tvb4_pro_2 = rtd_inl(DC2_PRIORITY_CTRLB_VADDR);
		rtd_outl(DC_PRIORITY_CTRLB_VADDR,TVSB4_PRIORITY_VALUE);
		rtd_outl(DC2_PRIORITY_CTRLB_VADDR,TVSB4_PRIORITY_VALUE);
	}
#endif

	return 0;

fail_cdev_init:
	platform_driver_unregister(&rtkvdec_device_driver);
fail_platform_driver_register:
	platform_device_unregister(rtkvdec_platform_devs);
	rtkvdec_platform_devs = NULL;
	class_destroy(rtkvdec_class);
fail_class_create:
	rtkvdec_class = NULL;
	unregister_chrdev_region(dev, 1);
	return result;
}

static void rtkvdec_exit(void)
{
	dev_t dev ;
	rtkvdec_debug_proc_uninit() ;
	rtkvdec_vmm_destroy();

	dev = MKDEV(rtkvdec_major, rtkvdec_minor);

	/* if ((rtkvdec_platform_devs == NULL) || (class_destroy == NULL)) ?? */
	if ((rtkvdec_platform_devs == NULL) || (rtkvdec_class == NULL))
		BUG();

	device_destroy(rtkvdec_class, dev);
	cdev_del(&rtkvdec_cdev);

	platform_driver_unregister(&rtkvdec_device_driver);
	platform_device_unregister(rtkvdec_platform_devs);
	rtkvdec_platform_devs = NULL;

	class_destroy(rtkvdec_class);
	rtkvdec_class = NULL;
           DCMT_DEBUG_INFO_UNREGISTER(DCMT_VDEC,rtkvdec_DCMTCallbackFunction);

	unregister_chrdev_region(dev, 1);
}


module_init(rtkvdec_init);
module_exit(rtkvdec_exit);
MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);
MODULE_AUTHOR("Realtek.com");
MODULE_LICENSE("GPL");
