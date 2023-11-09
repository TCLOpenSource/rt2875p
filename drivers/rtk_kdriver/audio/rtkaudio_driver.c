/*
 * Realtek Audio driver
 *
 * Copyright (c) 2014 Ivan <ivanlee@realtek.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License.
 *
 * History:
 *
 * 2014-09-23:  Ivan: first version
 */

#include <generated/autoconf.h>
#include <linux/version.h>
#include <linux/compat.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/suspend.h>
#include <asm/cacheflush.h>
#include <linux/proc_fs.h>

#include <rbus/audio_reg.h>

#include <rtk_kdriver/rtkaudio.h>
#include <rtk_kdriver/RPCDriver.h>
#include <rtk_kdriver/rmm/rmm.h>
#include <linux/pageremap.h>

#ifdef CONFIG_RTK_LOW_POWER_MODE
#include <rtk_kdriver/rtk-kdrv-common.h>
#endif

#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/pm_runtime.h>

//#include <mach/io.h>
#include <rtk_kdriver/io.h>
#include <rtk_kdriver/rtkaudio_debug.h>
#include <hal_audio.h>

#include <rtd_log/rtd_module_log.h>
#ifdef CONFIG_ARCH_RTK6702
#include <tvscalercontrol/hdmitx/hdmitx.h>
#endif
#include <linux/dma-mapping.h>
/*#include <sound/rtk_snd.h>*/
#include "AudioRPCBaseDS_data.h"
#include "AudioInbandAPI.h"
#include "audio_rpc.h"
#include "audio_inc.h"

#ifdef CONFIG_ARM64
#define UINTMAX_AUDIO (0xFFFFFFFFFFFFFFFFLL)
#define ALIGN4 (0xFFFFFFFFFFFFFFFCLL)
#else
#define UINTMAX_AUDIO (0xFFFFFFFF)
#define ALIGN4 (0xFFFFFFFC)
#endif

#define RPC_DVR_MALLOC_UNCACHED_SIZE (4096)//can't edit this size

#ifdef CONFIG_HIBERNATION
#include <linux/pm.h>
#endif

#ifdef CONFIG_PM
static int rtkaudio_suspend(struct platform_device *dev, pm_message_t state);
static int rtkaudio_resume(struct platform_device *dev);
#endif

#ifdef CONFIG_LG_SNAPSHOT_BOOT
extern unsigned int snapshot_enable;
#endif

#ifdef CONFIG_HIBERNATION
static int rtkaudio_std_suspend(struct device *dev);
static int rtkaudio_std_resume(struct device *dev);
static int rtkaudio_str_suspend(struct device *dev);
static int rtkaudio_str_resume(struct device *dev);
#endif

static int rtkaudio_runtime_suspend(struct device *dev);
static int rtkaudio_runtime_resume(struct device *dev);

#define GET_RUNTIME_CNT(pdev) (atomic_read(&pdev->power.usage_count))
#define GET_RUNTIME_AUTO(pdev) (pdev->power.runtime_auto)
#define GET_RUNTIME_STATUS(pdev) \
	(pdev->power.runtime_status == RPM_ACTIVE)?"active"\
	:(pdev->power.runtime_status == RPM_RESUMING)?"resuming"\
	:(pdev->power.runtime_status == RPM_SUSPENDED)?"suspended":"suspending"

#define RTKAUDIO_PM_LOG(LEVEL, GROUP, DEV, TYPE, VENDOR, MESSAGE) \
    do { \
        rtd_pr_adsp_info("LowPower::%s::%s::%s::%d::%s-%s::%s::ok::%s::%s::(runtime_status: %s)::(runtime_auto: %d)\n", \
                LEVEL, GROUP, "/dev/rtkaudio", GET_RUNTIME_CNT(DEV), \
                (current->group_leader?current->group_leader->comm:"-"), current->comm, \
                TYPE, VENDOR, MESSAGE, GET_RUNTIME_STATUS(DEV), GET_RUNTIME_AUTO(DEV)); \
    } while(0)


static const struct dev_pm_ops rtkaudio_pm_ops = {
#ifdef CONFIG_HIBERNATION
	.freeze		= rtkaudio_std_suspend,
	.thaw		= rtkaudio_std_resume,
	.poweroff	= rtkaudio_std_suspend,
	.restore	= rtkaudio_std_resume,
	.suspend	= rtkaudio_str_suspend,
	.resume		= rtkaudio_str_resume,
#endif
    .runtime_suspend    = rtkaudio_runtime_suspend,
    .runtime_resume     = rtkaudio_runtime_resume,
};

static BLOCKING_NOTIFIER_HEAD(rtkaudio_chain_head);

static int rtkaudio_major = RTKAUDIO_MAJOR;
static int rtkaudio_minor = RTKAUDIO_MINOR;

module_param(rtkaudio_major, int, S_IRUGO);
module_param(rtkaudio_minor, int, S_IRUGO);

static struct class *rtkaudio_class;
static struct cdev rtkaudio_cdev;
static struct platform_device *rtkaudio_platform_devs;
static struct platform_driver rtkaudio_device_driver = {
#ifdef CONFIG_PM
	.suspend		= rtkaudio_suspend,
	.resume			= rtkaudio_resume,
#endif
	.driver = {
		.name		= "rtkaudio",
		.bus		= &platform_bus_type,
#ifdef CONFIG_PM
		.pm 		= &rtkaudio_pm_ops,
#endif
	},
};

/* for retry defaul dump audio log */
static struct task_struct *rtkaudio_enable_tsk;
static int rtkaudio_enable_thread(void* arg);

/* for record remote malloc memory info */
static struct list_head remote_malloc_info_head;
static DEFINE_SEMAPHORE(buffer_list_sem);

/* for record rtkaudio alloc info */
static struct list_head rtkaudio_alloc_head;
static DEFINE_SEMAPHORE(rtkaudio_alloc_sem);

/* for open info */
static struct list_head rtkaudio_open_info_head;
static DEFINE_SEMAPHORE(rtkaudio_open_info_list_sem);

/* for proc entry */
struct proc_dir_entry *rtkaudio_proc_dir;
struct proc_dir_entry *rtkaudio_proc_entry;

extern void rtk_flush_range (const void *, const void *);

cmd_queue *cq = NULL;   //command queue between voice_ap and rtkaudio_driver
cmd_queue *cq_cached = NULL;

/* from AUDIO_SW_DBG_REG_1_VADDR need check */
#ifndef AUDIO_SW_DBG_REG_1_VADDR
#define AUDIO_SW_DBG_REG_1_VADDR (AUDIO_sw_dbg_reg_1_reg)
#ifndef GST_SHARED_REGISTER
#define GST_SHARED_REGISTER AUDIO_SW_DBG_REG_1_VADDR
#endif
#endif

#ifndef AUDIO_SW_DBG_REG_2_VADDR
#define AUDIO_SW_DBG_REG_2_VADDR (AUDIO_sw_dbg_reg_2_reg)
#endif

#define EQ_NUM_BANDS 16
typedef struct {
    int audio_data_path_status;
    int bOnOff[EQ_NUM_BANDS];
    int filter_type[EQ_NUM_BANDS];
    int fc[EQ_NUM_BANDS];
    int bw[EQ_NUM_BANDS];
    int gain[EQ_NUM_BANDS];
} Equalizer;

Equalizer equalizer;

#define BASS_TREBLE_LR_BANDS 3
typedef struct {
    int audio_data_path_status;
    int bOnOff[BASS_TREBLE_LR_BANDS];
    int filter_type[BASS_TREBLE_LR_BANDS];
    int fc[BASS_TREBLE_LR_BANDS];
    int bw[BASS_TREBLE_LR_BANDS];
    int gain[BASS_TREBLE_LR_BANDS];
} BassTrebleLR;

BassTrebleLR bass_treble_lr = {0};
BassTrebleLR bass_treble_lsrs = {0};
BassTrebleLR bass_treble_csw = {0};

int set_EQ_BASS_enable(ENUM_AUDIO_EQ_BASS_MODE mode, int band_idx, int bOnOff);
int set_EQ_BASS_parameter(ENUM_AUDIO_EQ_BASS_MODE mode, int band_idx, ENUM_AUDIO_EQ_BASS_FILTER_TYPE filter_type, int fc, int bw, int gain);
int set_EQ_BASS_Audio_Data_Path_Enable(ENUM_AUDIO_EQ_BASS_MODE mode, int bOnOff);

static DEFINE_SEMAPHORE(eq_bass_cmd_sem);

struct alsasink_device_info {
	int card;
	int device;
};

static struct alsasink_device_info alsa_info;
static DEFINE_SEMAPHORE(alsa_info_sem);

static DEFINE_SEMAPHORE(gst_debug_info_sem);

struct gst_refc_list *gst_refc_info;
int pr_to_dmesg_en;
int dump_size_scale;
int rtkaudio_dec_agent;

char AudioGITHASH[8];

bool eARC_offon;
bool ARC_offon;

int alsa_debug = 0;
EXPORT_SYMBOL(alsa_debug);

int drv_debug = 0;

int khal_debug = 0;
typedef enum {
    // bit-field setting
    KHAL_DEBUG_ALL_OFF           = 0,
    KHAL_DEBUG_SHOW_DIGITALINFO  = 1 << 0,
} KHAL_DEBUG_FLAG;

static DEFINE_SEMAPHORE(virtualx_cmd_sem);

static void control_status_init(void);

struct device* get_rtkaudio_device_ptr(void)
{
	return &(rtkaudio_platform_devs->dev);
}

extern bool reserve_boot_memory;

struct mutex dec_usage_lock;
static int rtkaudio_dec_inuse = false;
static int rtkaudio_create_decoder_agent(struct platform_device *rtkaudio_dev);
static int rtkaudio_get_audio_caps(void);


/* NUMDEV-220 */
#define QS_STATUS_reg 0xb8060150
#define  STB_SW_DATA5_reg 0xB8060150

typedef union {
    RBus_UInt32 regValue;
    struct {
        RBus_UInt32 quick_show_enable: 1;
        RBus_UInt32 src_is_HDMI: 1;
        RBus_UInt32 src_is_DP: 1;
        RBus_UInt32 panel: 1;
        RBus_UInt32 scaler: 1;
        RBus_UInt32 i2c: 1;
        RBus_UInt32 pq: 1;
        RBus_UInt32 acpu: 1;
        RBus_UInt32 vcpu: 1;
        RBus_UInt32 amp: 1;
        RBus_UInt32 headphone: 1;
        RBus_UInt32 port_num: 3;
        RBus_UInt32 volume: 7;
        RBus_UInt32 VO: 1;
        RBus_UInt32 mute: 1;
        RBus_UInt32 vo: 1;
        RBus_UInt32 magic: 8;
    };
} QS_STATUS_ST;

#define QS_STATUS_magic_valid_data (0xDD)

static long QuickShowEnabled(void)
{
    QS_STATUS_ST quick_show_reg;


    quick_show_reg.regValue = rtd_inl(STB_SW_DATA5_reg); /*NUMDEV-220*/
    if (quick_show_reg.magic == (uint8_t)QS_STATUS_magic_valid_data) {
        return quick_show_reg.quick_show_enable;
    } else {
        return 0;
    }
}

static int rtkaudio_enable_thread(void *arg)
{
	//long ret = 0;
	//int count = 0;

#ifdef CONFIG_LG_SNAPSHOT_BOOT
	long ret = 0;
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

		control_status_init();

		/* send REMOTE MELLOC ENABLE RPC to Audio FW */
		if (send_rpc_command(RPC_AUDIO,
			RPC_ACPU_IS_REMOTE_MALLOC_ENABLE,
			REMOTE_MALLOC_ENABLE, 0, (unsigned long*)&ret)) {
			rtd_pr_adsp_err("rtkaudio: RPC fail %ld\n", ret);
		}

		if (ret != S_OK)
		rtd_pr_adsp_err("rtkaudio: RPC is not S_OK %ld\n", ret);
	}
#endif

	do_exit(0);
	return 0;
}

int register_rtkaudio_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&rtkaudio_chain_head, nb);
}
EXPORT_SYMBOL(register_rtkaudio_notifier);

int unregister_rtkaudio_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&rtkaudio_chain_head, nb);
}
EXPORT_SYMBOL(unregister_rtkaudio_notifier);

int rtkaudio_notifier_call_chain(unsigned long val)
{
	int ret;

	ret = blocking_notifier_call_chain(&rtkaudio_chain_head, val, NULL);
	if (ret == NOTIFY_BAD)
		ret = -EINVAL;
	else
		ret = 0;

	return ret;
}

#ifdef CONFIG_PM
static int rtkaudio_suspend(struct platform_device *dev, pm_message_t state)
{
	unsigned long ret = S_OK;

	rtkaudio_notifier_call_chain(RTKAUDIO_SUSPEND);
	rtkaudio_notifier_call_chain(RTKAUDIO_RESET_PREPARE);
	rtd_pr_adsp_info("rtkaudio: receive supend command\n");
	/* todo resetav_lock(1); */
#ifdef CONFIG_HIBERNATION
	if (state.event != PM_EVENT_SUSPEND)
		rtd_pr_adsp_info("rtkaudio: ignore suspend\n");
	else
#endif

	/* send REMOTE MELLOC DISABLE RPC to Audio FW */
	if (send_rpc_command(RPC_AUDIO,
		RPC_ACPU_IS_REMOTE_MALLOC_ENABLE,
		REMOTE_MALLOC_DISABLE, 0, &ret)) {
		rtd_pr_adsp_err("rtkaudio: RPC fail %ld\n", ret);
	}

	if (ret != S_OK)
		rtd_pr_adsp_err("rtkaudio: RPC is not S_OK %ld\n", ret);

	return 0;
}

static int rtkaudio_resume(struct platform_device *dev)
{
	unsigned long phy_addr;
	unsigned long ret = S_OK;

	rtd_pr_adsp_err("Before rtkaudio_resume\n");
	device_pm_wait_for_dev(&(dev->dev), get_rpc_device_ptr());

	/* todo resetav_unlock(1); */
	rtd_pr_adsp_info("rtkaudio: receive resume command\n");
	rtkaudio_notifier_call_chain(RTKAUDIO_RESET_DONE);
	rtkaudio_notifier_call_chain(RTKAUDIO_RESUME);

	phy_addr = (unsigned long)dvr_to_phys((void*) gst_refc_info);
	rtd_pr_adsp_info("rtkaudio vir_addr %p, phy_addr %p\n", (void*)gst_refc_info, (void*)phy_addr);
	rtd_pr_adsp_info("rtkaudio read reg %x\n", rtd_inl(GST_SHARED_REGISTER));
	rtd_outl(GST_SHARED_REGISTER, (unsigned long) phy_addr);
	rtd_pr_adsp_info("rtkaudio read reg after write %x\n", rtd_inl(GST_SHARED_REGISTER));
	/* send REMOTE MELLOC ENABLE RPC to Audio FW */
	if (send_rpc_command(RPC_AUDIO,
		RPC_ACPU_IS_REMOTE_MALLOC_ENABLE,
		REMOTE_MALLOC_ENABLE, 0, &ret)) {
		rtd_pr_adsp_err("rtkaudio: RPC fail %ld\n", ret);
	}

	if (ret != S_OK)
		rtd_pr_adsp_err("rtkaudio: RPC is not S_OK %ld\n", ret);

	return 0;
}
#endif

#ifdef CONFIG_HIBERNATION
static int rtkaudio_std_suspend(struct device *dev)
{
	rtkaudio_notifier_call_chain(RTKAUDIO_SUSPEND);
	rtkaudio_notifier_call_chain(RTKAUDIO_RESET_PREPARE);
	rtd_pr_adsp_info("rtkaudio: receive supend command\n");

	return 0;
}

static int rtkaudio_std_resume(struct device *dev)
{
	unsigned long ret = S_OK;

	rtd_pr_adsp_err("Before rtkaudio_resume\n");
	device_pm_wait_for_dev(dev, get_rpc_device_ptr());

	/* todo resetav_unlock(1); */
	rtd_pr_adsp_info("rtkaudio: receive resume command\n");
	rtkaudio_notifier_call_chain(RTKAUDIO_RESET_DONE);
	rtkaudio_notifier_call_chain(RTKAUDIO_RESUME);

#ifdef CONFIG_LG_SNAPSHOT_BOOT
	if (snapshot_enable == 0)
#endif
	{
		control_status_init();

		/* send REMOTE MELLOC ENABLE RPC to Audio FW */
		if (send_rpc_command(RPC_AUDIO,
			RPC_ACPU_IS_REMOTE_MALLOC_ENABLE,
			REMOTE_MALLOC_ENABLE, 0, &ret)) {
			rtd_pr_adsp_err("rtkaudio: RPC fail %ld\n", ret);
		}

		if (ret != S_OK)
			rtd_pr_adsp_err("rtkaudio: RPC is not S_OK %ld\n", ret);
	}

#ifdef CONFIG_LG_SNAPSHOT_BOOT
	rtkaudio_enable_tsk = kthread_create(rtkaudio_enable_thread, NULL,
			"rtkaudio_enable_thread");
	if (IS_ERR(rtkaudio_enable_tsk)) {
		rtkaudio_enable_tsk = NULL;
		return -1;
	}
	wake_up_process(rtkaudio_enable_tsk);
#endif
	return 0;
}

static int rtkaudio_str_suspend(struct device *dev)
{
	unsigned long ret = S_OK;
	rtd_pr_adsp_info("rtkaudio: roni receive supend command\n");

	pm_runtime_disable(&rtkaudio_platform_devs->dev);
	rtkaudio_notifier_call_chain(RTKAUDIO_SUSPEND);
	rtkaudio_notifier_call_chain(RTKAUDIO_RESET_PREPARE);
	rtd_pr_adsp_info("rtkaudio: receive supend command\n");
	/* todo resetav_lock(1); */

	/* send REMOTE MELLOC DISABLE RPC to Audio FW */

	if (send_rpc_command(RPC_AUDIO,
		RPC_ACPU_IS_REMOTE_MALLOC_ENABLE,
		REMOTE_MALLOC_DISABLE, 0, &ret)) {
		rtd_pr_adsp_err("rtkaudio: RPC fail %ld\n", ret);
	}

	if (ret != S_OK)
		rtd_pr_adsp_err("rtkaudio: RPC is not S_OK %ld\n", ret);

	RTKAUDIO_PM_LOG("info", "rtkaudio", (&rtkaudio_platform_devs->dev), "stop", "realtek", "device STR PM disable");

	return 0;
}


static int rtkaudio_str_resume(struct device *dev)
{
	unsigned long phy_addr;
	unsigned long ret = S_OK;

	device_pm_wait_for_dev(dev, get_rpc_device_ptr());

	/* todo resetav_unlock(1); */

	rtd_pr_adsp_info("rtkaudio: receive resume command\n");
	rtkaudio_notifier_call_chain(RTKAUDIO_RESET_DONE);
	rtkaudio_notifier_call_chain(RTKAUDIO_RESUME);

	phy_addr = (unsigned long)dvr_to_phys((void*) gst_refc_info);
	rtd_pr_adsp_info("rtkaudio vir_addr %p, phy_addr %p\n",(void*)gst_refc_info, (void*)phy_addr);

	rtd_outl(GST_SHARED_REGISTER, (unsigned long) phy_addr);
	rtd_pr_adsp_info("rtkaudio read reg after write %x\n", rtd_inl(GST_SHARED_REGISTER));
	/* send REMOTE MELLOC ENABLE RPC to Audio FW */

	if (send_rpc_command(RPC_AUDIO,
		RPC_ACPU_IS_REMOTE_MALLOC_ENABLE,
		REMOTE_MALLOC_ENABLE, 0, &ret)) {
		rtd_pr_adsp_err("rtkaudio: RPC fail %ld\n", ret);
	}

	if (ret != S_OK)
		rtd_pr_adsp_err("rtkaudio: RPC is not S_OK %ld\n", ret);

	pm_runtime_enable(&rtkaudio_platform_devs->dev);

	RTKAUDIO_PM_LOG("info", "rtkaudio", (&rtkaudio_platform_devs->dev), "start", "realtek", "device STR PM enable");

	return 0;
}

#endif

void rtkaudio_fw_remote_malloc_summary(struct ret_info *info)
{
	unsigned long sum_request, sum_malloc;
	struct list_head *list;
	struct buffer_info *buf_info;

	sum_malloc = 0;
	sum_request = 0;

	down(&buffer_list_sem);
	list_for_each(list, &remote_malloc_info_head) {
		buf_info = list_entry(list, struct buffer_info, buffer_list);
        if (drv_debug)
        {
            rtd_pr_adsp_debug("vir_addr %lx, phy_addr %lx\n",
                    buf_info->vir_addr, buf_info->phy_addr);
            rtd_pr_adsp_debug("request_size %lx, malloc_size %lx\n",
                    buf_info->request_size, buf_info->malloc_size);
        }
		sum_request += buf_info->request_size;
		sum_malloc += buf_info->malloc_size;
	}
	up(&buffer_list_sem);

    if (drv_debug)
        rtd_pr_adsp_debug("sum_malloc %ld, sum_request %ld\n", sum_malloc, sum_request);

	if (info == NULL)
		return;

	info->private_info[0] = sum_request;
	info->private_info[1] = sum_malloc;
}

void rtkaudio_print_buffer_info(void)
{
	struct list_head *list;
	struct buffer_info *buf_info;

	down(&buffer_list_sem);
	list_for_each(list, &remote_malloc_info_head) {
		buf_info = list_entry(list, struct buffer_info, buffer_list);
		rtd_pr_adsp_debug("vir_addr %lx, phy_addr %lx\n",
			buf_info->vir_addr, buf_info->phy_addr);
		rtd_pr_adsp_debug("request_size %lx, malloc_size %lx\n",
			buf_info->request_size, buf_info->malloc_size);
	}
	up(&buffer_list_sem);
}

/* RPC handler function */
unsigned long rtkaudio_remote_malloc(unsigned long para1, unsigned long para2)
{
	unsigned long vir_addr;
	unsigned long phy_addr;
	struct buffer_info *buf_info;

    if (drv_debug)
        rtd_pr_adsp_debug("rtkaudio_remote_malloc(%lx/%lx)\n", para1, para2);

	vir_addr = (unsigned long)dvr_malloc_specific(para1, GFP_DCU1);
	if ((void*)vir_addr == NULL) {
		rtd_pr_adsp_err("[%s %d] alloc memory fail\n", __func__, __LINE__);
		return -ENOMEM;
	}

	phy_addr = (unsigned long)dvr_to_phys((void*)vir_addr);
	if ((void*)phy_addr == NULL) {
		rtd_pr_adsp_err("[%s %d] mapping physical address fail\n", __func__, __LINE__);
		if (vir_addr) {
			dvr_free((void*)vir_addr);
		}
		return -ENOMEM;
	}

	buf_info = kmalloc(sizeof(struct buffer_info), GFP_KERNEL);
	if (buf_info == NULL) {
		rtd_pr_adsp_err("[%s %d]alloc memory fail\n", __func__, __LINE__);
		if (vir_addr) {
			dvr_free((void*)vir_addr);
		}
		return -ENOMEM;
	}

	INIT_LIST_HEAD(&buf_info->buffer_list);

	buf_info->vir_addr = vir_addr;
	buf_info->phy_addr = phy_addr;
	buf_info->request_size = para1;
	buf_info->malloc_size = PAGE_ALIGN(para1);

    if (drv_debug)
        rtd_pr_adsp_debug("rtkaudio_remote_malloc addr (%lx,%lx)\n", phy_addr, vir_addr);
	down(&buffer_list_sem);
	list_add(&buf_info->buffer_list, &remote_malloc_info_head);
	up(&buffer_list_sem);
	return phy_addr;
}

/* RPC handler function */
unsigned long rtkaudio_remote_free(unsigned long para1, unsigned long para2)
{
	unsigned long vir_addr = 0;
	struct list_head *del_list;
	struct buffer_info *buf_info = NULL;

    if (drv_debug)
        rtd_pr_adsp_debug("rtkaudio_remote_free(%lx/%lx)\n", para1, para2);

	down(&buffer_list_sem);
	list_for_each(del_list, &remote_malloc_info_head) {
		buf_info = list_entry(del_list,
			struct buffer_info,
			buffer_list);
		if (buf_info->phy_addr == para1) {
			vir_addr = buf_info->vir_addr;
			list_del(del_list);
			break;
		}
	}
	up(&buffer_list_sem);

	if ((void*)vir_addr != NULL)
		dvr_free((void *)vir_addr);

	if (buf_info != NULL)
		kfree((void*)buf_info);

	return 0;
}

uint32_t rtkaudio_alloc_uncached(int size, unsigned char** uncached_addr_info)
{
	unsigned long vir_addr;
	unsigned long phy_addr;
	struct buffer_info *buf_info;
	int64_t start_t;

	start_t = getpts();
	if(uncached_addr_info != NULL){
		vir_addr = (unsigned long)dvr_malloc_uncached_specific(size, GFP_DCU1, (void**)uncached_addr_info);
	} else{
		vir_addr = (unsigned long)dvr_malloc_specific(size, GFP_DCU1);
	}
	if(getpts() - start_t > 90*100) {
		pr_err("[%s %d] dvr_malloc_specific size %d takes %d ms\n", __func__, __LINE__, size, (int)(getpts() - start_t)/90);
	}
	if (!vir_addr) {
		pr_err("[%s %d]alloc memory fail for size %d\n", __func__, __LINE__, size);
		return -ENOMEM;
	}

	phy_addr = (unsigned long)dvr_to_phys((void *)vir_addr);
	if (!phy_addr) {
		pr_err("[%s %d]alloc memory fail\n", __func__, __LINE__);
		if (vir_addr) {
			dvr_free((void*)vir_addr);
		}
		return -ENOMEM;
	}
	buf_info = kmalloc(sizeof(struct buffer_info), GFP_KERNEL);
	if (buf_info == NULL) {
		pr_err("[%s %d]alloc memory fail\n", __func__, __LINE__);
		if (vir_addr) {
			dvr_free((void*)vir_addr);
		}
		return -ENOMEM;
	}

	buf_info->vir_addr = vir_addr;
	buf_info->phy_addr = phy_addr;
	buf_info->request_size = size;
	buf_info->malloc_size = PAGE_ALIGN(size);
	buf_info->task_pid = 1; /* for kernel pid */
	pr_debug("rtkaudio_alloc(p:%lx,v:%px) for pid(%d)\n", phy_addr, vir_addr, buf_info->task_pid);

	down(&rtkaudio_alloc_sem);
	list_add(&buf_info->buffer_list, &rtkaudio_alloc_head);
	up(&rtkaudio_alloc_sem);

	return phy_addr;
}

long rtkaudio_alloc(int size, int limit)
{
	unsigned long vir_addr;
	unsigned long phy_addr;
	struct buffer_info *buf_info;

	vir_addr = (unsigned long)dvr_malloc_specific(size, (limit ? GFP_DCU1_LIMIT : GFP_DCU1));
	if ((void*)vir_addr == NULL) {
		rtd_pr_adsp_err("[%s %d] alloc memory fail\n", __func__, __LINE__);
		return -ENOMEM;
	}

	phy_addr = (unsigned long)dvr_to_phys((void *)vir_addr);
	if (!phy_addr) {
		rtd_pr_adsp_err("[%s %d] mapping physical address fail\n", __func__, __LINE__);
		if (vir_addr) {
			dvr_free((void*)vir_addr);
		}
		return -ENOMEM;
	}

	buf_info = kmalloc(sizeof(struct buffer_info), GFP_KERNEL);
	if (buf_info == NULL) {
		rtd_pr_adsp_err("[%s %d] alloc memory fail\n", __func__, __LINE__);
		if (vir_addr) {
			dvr_free((void*)vir_addr);
		}
		return -ENOMEM;
	}

	buf_info->vir_addr = vir_addr;
	buf_info->phy_addr = phy_addr;
	buf_info->request_size = size;
	buf_info->malloc_size = PAGE_ALIGN(size);
	buf_info->task_pid = task_tgid_nr(current);
    if (drv_debug)
    {
        rtd_pr_adsp_debug("Alloc (p:%p,v:%p) for pid(%d), limit(%d)\n",
                (void*)phy_addr, (void*)vir_addr, task_tgid_nr(current), limit);
    }

	down(&rtkaudio_alloc_sem);
	list_add(&buf_info->buffer_list, &rtkaudio_alloc_head);
	up(&rtkaudio_alloc_sem);

	return phy_addr;
}

int rtkaudio_free(unsigned int phy_addr)
{
	unsigned long vir_addr;
	struct buffer_info *buf_info;

	vir_addr = UINTMAX_AUDIO;
	down(&rtkaudio_alloc_sem);
	list_for_each_entry(buf_info, &rtkaudio_alloc_head, buffer_list)
	{
		if (buf_info->phy_addr == phy_addr) {
			vir_addr = buf_info->vir_addr;
			list_del(&buf_info->buffer_list);
			kfree(buf_info);
			break;
		}
	}
	up(&rtkaudio_alloc_sem);

	if (vir_addr == UINTMAX_AUDIO) {
		rtd_pr_adsp_err("[rtkaudio] %x doesn't in memory list\n", phy_addr);
		return 0;
	}

	dvr_free((void *)vir_addr);
	return 0;
}

void rtkaudio_free_by_process(unsigned long process_id)
{
	struct buffer_info *buf_info, *tmp_buf_info;

	down(&rtkaudio_alloc_sem);
	list_for_each_entry_safe(buf_info, tmp_buf_info,
		&rtkaudio_alloc_head, buffer_list)
	{
		if (buf_info->task_pid == process_id) {
			list_del(&buf_info->buffer_list);
			dvr_free((void *)buf_info->vir_addr);
			kfree(buf_info);
		}
	}
	up(&rtkaudio_alloc_sem);
}

void rtkaudio_user_alloc_summary(struct ret_info *info)
{
	unsigned long sum_request, sum_malloc;
	struct buffer_info *buf_info;
	struct buffer_info *b;
	struct list_head pid_list = LIST_HEAD_INIT(pid_list);

	sum_malloc = sum_request = 0;

	down(&rtkaudio_alloc_sem);
	list_for_each_entry(buf_info, &rtkaudio_alloc_head, buffer_list)
	{
        if (drv_debug)
        {
            rtd_pr_adsp_debug("pid(%d) : malloc %d, request %d\n",
                    (int)buf_info->task_pid,
                    (int)buf_info->malloc_size,
                    (int)buf_info->request_size);
        }

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
	up(&rtkaudio_alloc_sem);
	list_for_each_entry_safe(b, buf_info, &pid_list, buffer_list)
	{
        if (drv_debug)
        {
            rtd_pr_adsp_debug("pid(%d) : sum_malloc %d, sum_request %d\n",
                    (int)b->task_pid,
                    (int)b->malloc_size,
                    (int)b->request_size);
        }
		sum_request += b->request_size;
		sum_malloc  += b->malloc_size;
		list_del(&b->buffer_list);
		kfree(b);
	}

    if (drv_debug)
        rtd_pr_adsp_debug("sum_malloc %ld, sum_request %ld\n", sum_malloc, sum_request);

	if (info == NULL)
		return;

	info->private_info[0] = sum_request;
	info->private_info[1] = sum_malloc;
}

/* RPC handler function */
#ifdef CONFIG_ARCH_RTK6702
unsigned long rtkaudio_set_hdmitx_format(unsigned long para1, unsigned long para2)
{
    HDMITX_INFORMATION tx_info;
    tx_info.value = para1;

    rtd_pr_adsp_debug("rtkaudio_set_hdmitx_format(%lx/%lx):chnum:%d,type:%d,mute:%d\n", para1,para2,tx_info.chnum, tx_info.data_type, tx_info.mute);
#ifdef CONFIG_ENABLE_HDMITX
    setHdmitxAudioChanel(tx_info.chnum);
    setHdmitxAudioType(tx_info.data_type);
    setHdmitxAudioMute(tx_info.mute);
    setHdmitxAudioFreq(para2);
#endif
    return 0;
}
#endif
unsigned long rtkaudio_free_user_mem(unsigned long para1, unsigned long para2)
{
	rtd_pr_adsp_debug("rtkaudio_free_user_mem(%lx/%lx)\n", para1, para2);
	rtkaudio_user_alloc_summary(NULL);
	rtkaudio_free_by_process(para1);
	rtkaudio_user_alloc_summary(NULL);
	return 0;
}

static void control_status_init(void)
{
	int i;
	unsigned long vir_addr;
	unsigned long phy_addr;

	alsa_info.card = 0;
	alsa_info.device = 0;

	vir_addr = (unsigned long)dvr_malloc_specific(4096, GFP_DCU1);
	if ((void*)vir_addr == NULL) {
		rtd_pr_adsp_err("[%s %d]alloc memory fail\n", __func__, __LINE__);
		return;
	}

	phy_addr = (unsigned long)dvr_to_phys((void*)vir_addr);
	if ((void*)phy_addr == NULL) {
		rtd_pr_adsp_err("[%s %d]alloc memory fail\n", __func__, __LINE__);
		if (vir_addr) {
			dvr_free((void*)vir_addr);
		}
		return;
	}

	gst_refc_info = (struct gst_refc_list *) vir_addr;
	rtd_outl(GST_SHARED_REGISTER, (unsigned long) phy_addr);

	gst_refc_info->size = refc_info_size;
	gst_refc_info->index = 0;

	for(i = 0; i < gst_refc_info->size; i++) {
		gst_refc_info->info[i].phy_addr = -1;
		gst_refc_info->info[i].pid = -1;
		gst_refc_info->info[i].port = -1;
	}
}

int read_register_by_type(AUDIO_REGISTER_ACCESS_T *info)
{
	int value = -1;

	switch(info->reg_type) {
		case GST_CLOCK:
			value = rtd_inl(GST_SHARED_REGISTER);
			break;
		case AMIXER_STATUS:
			value = rtd_inl(AUDIO_SW_DBG_REG_2_VADDR);
			break;
		case DEBUG_REGISTER:
			value = rtd_inl(info->reg_value);
			rtd_pr_adsp_err("[rtkaudio] debug read register value %x\n", value);
			break;
		default:
			rtd_pr_adsp_err("[rtkaudio] unsupport read register type %d\n", info->reg_type);
			break;
	}

	return value;
}

void write_register_by_type(AUDIO_REGISTER_ACCESS_T *info) {
	switch(info->reg_type) {
		case DEBUG_REGISTER:
			rtd_outl(info->reg_value, info->data);
			rtd_pr_adsp_info("[rtkaudio] debug write register value %x\n", info->reg_value);
			break;
		default:
			rtd_pr_adsp_err("[rtkaudio] unsupport write register type %d\n", info->reg_type);
			break;
	}
}

int rtkaudio_open(struct inode *inode, struct file *filp)
{
	if (!filp->private_data) {
		struct list_head *list;
		struct rtkaudio_open_list *open_info;
		struct rtkaudio_open_list *info_list;

		down(&rtkaudio_open_info_list_sem);
		list_for_each(list, &rtkaudio_open_info_head) {
			open_info =
				list_entry(list, struct rtkaudio_open_list, open_list);

			if (open_info == NULL) {
				rtd_pr_adsp_warn("[%s] open info is NULL return here %p\n", __func__, (void*)open_info);
				up(&rtkaudio_open_info_list_sem);
				return 0;
			}

			if (open_info->pid == current->tgid) {
				up(&rtkaudio_open_info_list_sem);
				return 0;
			}
		}
		up(&rtkaudio_open_info_list_sem);

		info_list =
			kmalloc(sizeof(struct rtkaudio_open_list), GFP_KERNEL);

		if (info_list == NULL) {
			rtd_pr_adsp_err("%s: failed to allocate rtkaudio_open_list\n", __func__);
			return -ENOMEM;
		}

		INIT_LIST_HEAD(&info_list->open_list);
		info_list->pid = current->tgid;
		info_list->info.gst_open = 0;
		info_list->info.decoded_size = 0llu;
		info_list->info.undecoded_size = 0llu;
		down(&rtkaudio_open_info_list_sem);
		list_add(&info_list->open_list, &rtkaudio_open_info_head);
		up(&rtkaudio_open_info_list_sem);
		filp->private_data = info_list;
	}
	return 0;
}

int rtkaudio_release(struct inode *inode, struct file *filp)
{
	if (filp->private_data) {
		struct rtkaudio_open_list *info_list = filp->private_data;
		struct list_head *del_list;
		struct rtkaudio_open_list *open_info_list;

		down(&rtkaudio_open_info_list_sem);
		list_for_each(del_list, &rtkaudio_open_info_head) {
			open_info_list =
				list_entry(del_list, struct rtkaudio_open_list, open_list);
			if (open_info_list->pid == info_list->pid) {
				list_del(del_list);
				kfree(open_info_list);
				break;
			}
		}
		up(&rtkaudio_open_info_list_sem);
	}
	return 0;
}

long rtkaudio_send_audio_config(AUDIO_CONFIG_COMMAND_RTKAUDIO * cmd)
{
	long ret = S_OK;
	ulong dvr_addr;
	ulong phy_addr;
	int i;

	AUDIO_CONFIG_COMMAND_RTKAUDIO *vir_audioConfig;
	long *vir_res, *phy_res;

	dvr_addr = (ulong)get_rpc_mem((void **)&vir_audioConfig);

	if (!dvr_addr) {
		rtd_pr_adsp_err("[%s %d]alloc memory fail\n", __func__, __LINE__);
		return 0;
	}
	phy_addr = (ulong)dvr_to_phys((void *)dvr_addr);

    if (drv_debug)
        rtd_pr_adsp_debug("rtkaudio_send_audio_config: msg_id=%d\n",cmd->msg_id);

	vir_audioConfig->msg_id = htonl(cmd->msg_id);

	for (i=0;i<15;i++) {
		vir_audioConfig->value[i] = htonl(cmd->value[i]);
	}

	phy_res = (long *) ((phy_addr+ sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO) + 8) &ALIGN4);
	vir_res = (long *) (((long)vir_audioConfig+ sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO) + 8) &ALIGN4);

	if (send_rpc_command(RPC_AUDIO, ENUM_KERNEL_RPC_AUDIO_CONFIG, (unsigned int) phy_addr,
		(unsigned long) phy_res, &ret)) {
		rtd_pr_adsp_err("rtkaudio: send(%s %d) fail %d\n", __FUNCTION__, cmd->msg_id, (int)ret);
		put_rpc_mem((void *) dvr_addr);
		return 0;
	}
	//rtd_pr_adsp_info("phy_res=%x,vir_res=%x,  *vir_res=%d\n", phy_res, vir_res, *vir_res);
	//rtd_pr_adsp_info("phy_addr=%x,vir_audioConfig=%x,  \n", phy_res, vir_audioConfig);

	for (i=0;i<15;i++) {
		cmd->value[i] = ntohl(vir_audioConfig->value[i]);
	}
	ret = ntohl(*vir_res);

	put_rpc_mem((void *) dvr_addr);
	return ret;
}
EXPORT_SYMBOL(rtkaudio_send_audio_config);

long rtkaudio_send_hdmi_fs(long hdmi_fs)
{
	long ret = S_OK;
	AUDIO_CONFIG_COMMAND_RTKAUDIO info;

	info.msg_id = AUDIO_CONFIG_CMD_SEND_HDMI_FS;
	info.value[0] = hdmi_fs;

	ret = rtkaudio_send_audio_config(&info);
	return ret;
}

//static SIMPLE_DEV_PM_OPS(audio_hw_pm_ops, audio_hw_suspend, audio_hw_resume);
int rtkaudio_runtime_suspend(struct device *dev)
{
	long ret = S_OK;
	AUDIO_CONFIG_COMMAND_RTKAUDIO info;

	rtd_pr_adsp_info("debug @(%s:%d)\n", __func__, __LINE__);

	info.msg_id = AUDIO_CONFIG_CMD_AO_LOW_LATENCY;
	info.value[0] = 1;

	ret = rtkaudio_send_audio_config(&info);
	if (ret != S_OK) {
		rtd_pr_adsp_err("[DIAS] rtkaudio_send_audio_config disable continue pin fail !!!\n");
		ret = -EFAULT;

		return ret;
	}

	msleep(30);

	info.msg_id = AUDIO_CONFIG_CMD_DIAS_LOW_POWER;
	info.value[0] = 0;

	ret = rtkaudio_send_audio_config(&info);
	if (ret != S_OK) {
		rtd_pr_adsp_err("[DIAS] rtkaudio_send_audio_config suspend fail !!!\n");
		ret = -EFAULT;

		return ret;
	}

	RTKAUDIO_PM_LOG("info", "rtkaudio", (&rtkaudio_platform_devs->dev), "suspend", "realtek", "system pm suspend");

	return 0;
}

int rtkaudio_runtime_resume(struct device *dev)
{
	long ret = S_OK;
	AUDIO_CONFIG_COMMAND_RTKAUDIO info;

	rtd_pr_adsp_info("debug @(%s:%d)\n", __func__, __LINE__);

	info.msg_id = AUDIO_CONFIG_CMD_DIAS_LOW_POWER;
	info.value[0] = 1;

	ret = rtkaudio_send_audio_config(&info);
	if (ret != S_OK) {
		rtd_pr_adsp_err("[DIAS] rtkaudio_send_audio_config resume fail !!!\n");
		ret = -EFAULT;

		return ret;
	}

	RTKAUDIO_PM_LOG("info", "rtkaudio", (&rtkaudio_platform_devs->dev), "resume", "realtek", "system pm resume");

	return 0;
}
void RHAL_AUDIO_HDMI_GetAudioReturnChannel(bool *onoff)
{
	*onoff = ARC_offon;
}

void RHAL_AUDIO_HDMI_GetEnhancedAudioReturnChannel(bool *onoff)
{
	*onoff = eARC_offon;
}

long send_virtualx_rpc(void* p, virtualx_cmd_info virx_cmd, dma_addr_t dat, int send)
{
	AUDIO_RPC_PRIVATEINFO_PARAMETERS *cmd, *cmd_audio;
	AUDIO_RPC_PRIVATEINFO_RETURNVAL *res, *res_audio;
	long ret = 0;

	int *pSetting;
	int i;

	down(&virtualx_cmd_sem);

	cmd = p;
	if( send ){
		res = (void*)(AUDIO_RPC_PRIVATEINFO_RETURNVAL *) ((((unsigned long)p +
				sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS)+sizeof(virtualx_cmd_info)+8) & ALIGN4));

		cmd_audio = (void*)(AUDIO_RPC_PRIVATEINFO_PARAMETERS *)(dat);
		res_audio = (void*)(AUDIO_RPC_PRIVATEINFO_RETURNVAL *)
					((((unsigned long)(dat) +
					sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS)+sizeof(virtualx_cmd_info)+8) & ALIGN4));
	} else {
		res = (AUDIO_RPC_PRIVATEINFO_RETURNVAL *) ((((unsigned long)p +
				sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS)+sizeof(virtualx_cmd_info)+8) & 0xFFFFFFFC));

		cmd_audio = (AUDIO_RPC_PRIVATEINFO_PARAMETERS *)(dat);
		res_audio = (AUDIO_RPC_PRIVATEINFO_RETURNVAL *)
					((((unsigned long)(dat) +
					sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS)+sizeof(virtualx_cmd_info)+8) & 0xFFFFFFFC));
	}

	memset(cmd, 0, sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS)+sizeof(virtualx_cmd_info));

	cmd->instanceID = htonl(0);
	cmd->type = htonl(ENUM_PRIVATEINFO_AUDIO_CONFIG_VIRTUALX_PARAM);
	cmd->privateInfo[0] = htonl(virx_cmd.type);
	cmd->privateInfo[1] = htonl(virx_cmd.size);

	pSetting = &cmd->privateInfo[2];

	for (i = 0; i < virx_cmd.size/sizeof(unsigned int); i++)
	{
		pSetting[i] = htonl(virx_cmd.data[i]);
	}

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_PRIVATEINFO,
		(unsigned long) cmd_audio,
		(unsigned long) res_audio, &ret))
	{
		rtd_pr_adsp_info("[Virtual X] %s %d RPC fail %ld\n", __func__, __LINE__, ret);
		ret = -EFAULT;
	}

	if (ret != S_OK)
	{
		rtd_pr_adsp_info("[Virtual X] %s %d RPC is not S_OK %ld\n", __func__, __LINE__, ret);
		ret = -EFAULT;
	}

	virx_cmd.result= htonl(res->privateInfo[0]);

	pSetting = &res->privateInfo[2];
	for (i = 0; i < virx_cmd.size/sizeof(unsigned int); i++)
	{
		virx_cmd.data[i]= htonl(pSetting[i]);
	}

	up(&virtualx_cmd_sem);

	return ret;
}

int rtkaudio_get_audio_quick_show_init_table(quickshow_cmd_info *quickshow_cmd)
{
	unsigned long ret;
	AUDIO_RPC_PRIVATEINFO_PARAMETERS *cmd;
	AUDIO_RPC_PRIVATEINFO_RETURNVAL *res;

	unsigned long cmd_audio = 0, res_audio = 0;

	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long phy_addr = 0;

	int *pSetting;
	int i;

	/*rtd_pr_adsp_err("[QuickShow] %s, type:%d\n", __func__, quickshow_cmd->type);*/

	vir_addr = dvr_malloc_uncached(RPC_DVR_MALLOC_UNCACHED_SIZE, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[QuickShow] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = (unsigned long) dvr_to_phys(vir_addr);

	cmd = unvir_addr;
	res = (AUDIO_RPC_PRIVATEINFO_RETURNVAL *)((((unsigned long)unvir_addr + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS) + sizeof(quickshow_cmd_info) + 8) & ALIGN4));

	cmd_audio = phy_addr;
	res_audio = (phy_addr + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS) + sizeof(quickshow_cmd_info) + 8) & ALIGN4;

	memset(unvir_addr, 0, RPC_DVR_MALLOC_UNCACHED_SIZE);

	cmd->instanceID = htonl(0);
	cmd->type = htonl(ENUM_PRIVATEINFO_AUDIO_GET_QUICK_SHOW_INFO);
	cmd->privateInfo[0] = htonl(quickshow_cmd->type);
	cmd->privateInfo[1] = htonl(quickshow_cmd->size);

	pSetting = &cmd->privateInfo[2];
	for (i = 0; i < quickshow_cmd->size/sizeof(unsigned int); i++)
	{
		pSetting[i] = htonl(quickshow_cmd->data[i]);
	}

	if (send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_PRIVATEINFO,
		cmd_audio,
		res_audio, &ret))
	{
		rtd_pr_adsp_err("[QuickShow] %s %d RPC fail %ld\n", __func__, __LINE__, ret);
		ret = -EFAULT;
	}

	if (ret != S_OK)
	{
		rtd_pr_adsp_err("[QuickShow] %s %d RPC is not S_OK %ld\n", __func__, __LINE__, ret);
		ret = -EFAULT;
	}

	quickshow_cmd->result= htonl(res->privateInfo[0]);

	pSetting = &res->privateInfo[2];
	for (i = 0; i < quickshow_cmd->size/sizeof(unsigned int); i++)
	{
		quickshow_cmd->data[i]= htonl(pSetting[i]);
	}
	/*rtd_pr_adsp_err("[QuickShow][%s:%d] ret:%d, result:%d, data[0]:%x, data[1]:%x\n", __func__, __LINE__, ret, quickshow_cmd->result, quickshow_cmd->data[0], quickshow_cmd->data[1]);*/

	dvr_free(vir_addr);

	return ret;
}

long rtkaudio_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	long ret = 0;

    if (drv_debug)
        rtd_pr_adsp_debug("rtkaudio: receive ioctl(cmd:0x%08x, arg:0x%08lx)\n", cmd, arg);

	switch (cmd) {
	case RTKAUDIO_IOC_INIT:
		blocking_notifier_call_chain(&rtkaudio_chain_head,
			RTKAUDIO_RESET_PREPARE, NULL);
        if (drv_debug)
            rtd_pr_adsp_debug("RTKAUDIO_IOC_INIT\n");
		blocking_notifier_call_chain(&rtkaudio_chain_head,
			RTKAUDIO_RESET_DONE, NULL);
		break;
	case RTKAUDIO_IOC_DEINIT:
        if (drv_debug)
            rtd_pr_adsp_debug("RTKAUDIO_IOCTL_DEINIT\n");
		break;
	case RTKAUDIO_IOC_OPEN:
        if (drv_debug)
            rtd_pr_adsp_debug("RTKAUDIO_IOCTL_OPEN\n");
		break;
	case RTKAUDIO_IOC_CLOSE:
        if (drv_debug)
            rtd_pr_adsp_debug("RTKAUDIO_IOCTL_CLOSE\n");
		break;
	case RTKAUDIO_IOC_CONNECT:
        if (drv_debug)
            rtd_pr_adsp_debug("RTKAUDIO_IOCTL_CONNECT\n");
		break;
	case RTKAUDIO_IOC_SET:
	{
		struct cntl_info info;
        if (drv_debug)
            rtd_pr_adsp_debug("RTKAUDIO_IOCTL_SET\n");
		if (copy_from_user((void*)&info, (const void __user*)arg, sizeof(info))) {
			ret = -EFAULT;
		}

		switch(info.resource_type) {
        case SET_RHAL_INFO:
        {
            DTV_STATUS_T status;
            HAL_AUDIO_RHAL_INFO_T rhalinfo;

            if (copy_from_user(&rhalinfo, (const void __user *) (long)info.addr, info.size))
            {
                rtd_pr_adsp_err("%s [SET_DECODER_INPUT_MUTE] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }

            switch(rhalinfo.type)
            {
                case SET_TP_OPEN:
                    status = RHAL_AUDIO_TP_Open(rhalinfo.tpIndex);
                    break;
                case SET_ADEC_OPEN:
                    status = RHAL_AUDIO_ADEC_Open(rhalinfo.adecindex);
                    break;
                case SET_ADC_OPEN:
                    status = RHAL_AUDIO_ADC_Open(rhalinfo.portNum);
                    break;
                case SET_HDMI_OPEN:
                    status = RHAL_AUDIO_HDMI_Open();
                    break;
                case SET_DP_OPEN:
                    status = RHAL_AUDIO_DP_Open();
                    break;
                case SET_AAD_OPEN:
                    status = RHAL_AUDIO_AAD_Open();
                    break;
                case SET_SNDOUT_OPEN:
                    status = RHAL_AUDIO_SNDOUT_Open(rhalinfo.soundOutType);
                    break;
                case SET_TP_CLOSE:
                    status = RHAL_AUDIO_TP_Close(rhalinfo.tpIndex);
                    break;
                case SET_ADEC_CLOSE:
                    status = RHAL_AUDIO_ADEC_Close(rhalinfo.adecindex);
                    break;
                case SET_ADC_CLOSE:
                    status = RHAL_AUDIO_ADC_Close(rhalinfo.portNum);
                    break;
                case SET_HDMI_CLOSE:
                    status = RHAL_AUDIO_HDMI_Close();
                    break;
                case SET_DP_CLOSE:
                    status = RHAL_AUDIO_DP_Close();
                    break;
                case SET_AAD_CLOSE:
                    status = RHAL_AUDIO_AAD_Close();
                    break;
                case SET_SNDOUT_CLOSE:
                    status = RHAL_AUDIO_SNDOUT_Close(rhalinfo.soundOutType);
                    break;
                case SET_SPDIFOUT_DELAYTIME:
                    status = RHAL_AUDIO_SetSPDIFOutDelayTime(rhalinfo.delayTime, rhalinfo.enable);
                    break;
                case SET_ARC_OUTPUT_TYPE:
                    status = RHAL_AUDIO_ARC_SetOutputType(rhalinfo.eARCMode, rhalinfo.enable);
                    break;
                case SET_START_DECODING:
                    status = RHAL_AUDIO_StartDecoding(rhalinfo.adecindex, rhalinfo.audiotype);
                    break;
                case SET_STOP_DECODING:
                    status = RHAL_AUDIO_StopDecoding(rhalinfo.adecindex);
                    break;
                case SET_ADC_CONNECT:
                    status = RHAL_AUDIO_ADC_Connect(rhalinfo.portNum);
                    break;
                case SET_ADC_DISCONNECT:
                    status = RHAL_AUDIO_ADC_Disconnect(rhalinfo.portNum);
                    break;
                case SET_TP_CONNECT:
                    status = RHAL_AUDIO_TP_Connect(rhalinfo.adecinfo.prevTPInputPort, rhalinfo.adecinfo.curTPInputPort);
                    break;
                case SET_TP_DISCONNECT:
                    status = RHAL_AUDIO_TP_Disconnect(rhalinfo.adecinfo.prevTPInputPort, rhalinfo.adecinfo.curTPInputPort);
                    break;
                case SET_ADEC_CONNECT:
                    status = RHAL_AUDIO_ADEC_Connect(rhalinfo.adecinfo.prevTPInputPort, rhalinfo.adecinfo.curTPInputPort);
                    break;
                case SET_ADEC_DISCONNECT:
                    status = RHAL_AUDIO_ADEC_Disconnect(rhalinfo.adecinfo.prevTPInputPort, rhalinfo.adecinfo.curTPInputPort);
                    break;
                case SET_AMIX_CONNECT:
                    status = RHAL_AUDIO_AMIX_Connect(rhalinfo.adecinfo.prevTPInputPort, rhalinfo.adecinfo.curTPInputPort);
                    break;
                case SET_AMIX_DISCONNECT:
                    status = RHAL_AUDIO_AMIX_Disconnect(rhalinfo.adecinfo.prevTPInputPort, rhalinfo.adecinfo.curTPInputPort);
                    break;
                case SET_SE_CONNECT:
                    status = RHAL_AUDIO_SE_Connect(rhalinfo.inputConnect);
                    break;
                case SET_SE_DISCONNECT:
                    status = RHAL_AUDIO_SE_Disconnect(rhalinfo.inputConnect);
                    break;
                case SET_SNDOUT_CONNECT:
                    status = RHAL_AUDIO_SNDOUT_Connect(rhalinfo.adecinfo.prevTPInputPort, rhalinfo.adecinfo.curTPInputPort);
                    break;
                case SET_SNDOUT_DISCONNECT:
                    status = RHAL_AUDIO_SNDOUT_Disconnect(rhalinfo.adecinfo.prevTPInputPort, rhalinfo.adecinfo.curTPInputPort);
                    break;
                case SET_PAUSE_DECODING:
                    status = RHAL_AUDIO_PauseAdec(rhalinfo.adecindex);
                    break;
                case SET_PAUSE_AND_STOP_DECODING:
                    status = RHAL_AUDIO_PauseAndStopFlowAdec(rhalinfo.adecindex);
                    break;
                case SET_RESUME_AND_RUN_DECODING:
                    status = RHAL_AUDIO_ResumeAndRunAdec(rhalinfo.adecindex);
                    break;
                case SET_RESUME_DECODING:
                    status = RHAL_AUDIO_ResumeAdec(rhalinfo.adecindex);
                    break;
                case SET_FLUSH_DECODING:
                    status = RHAL_AUDIO_FlushAdec(rhalinfo.adecindex);
                    break;
                case SET_MAIN_DECODER_OUTPUT:
                    status = RHAL_AUDIO_SetMainDecoderOutput(rhalinfo.adecindex);
                    break;
                case SET_MAIN_AUDIO_OUTPUT:
                    status = RHAL_AUDIO_SetMainAudioOutput(rhalinfo.audioIndex);
                    break;
                case SET_DECODER_INPUT_MUTE:
                    status = RHAL_AUDIO_SetDecoderInputMute(rhalinfo.adecindex, rhalinfo.enable);
                    break;
                case SET_DECODER_DELAY_TIME:
                    status = RHAL_AUDIO_SetDecoderDelayTime(rhalinfo.adecindex, rhalinfo.delayTime);
                    break;
                case SET_DECODER_INPUT_GAIN:
                    status = RHAL_AUDIO_SetDecoderInputGain(rhalinfo.adecindex, rhalinfo.volume);
                    break;
                case SET_AD_MAIN:
                    status = RHAL_AUDIO_TP_SetAudioDescriptionMain(rhalinfo.adecindex, rhalinfo.enable);
                    break;
                case SET_TRICK_MODE:
                    status = RHAL_AUDIO_SetTrickMode(rhalinfo.adecindex, rhalinfo.eTrickMode);
                    break;
                case SET_TRACK_MODE:
                    status = RHAL_AUDIO_setTrackMode(rhalinfo.eTrackMode);
                    break;
                case SET_ATMOS_ENCODE:
                    status = RHAL_AUDIO_ATMOS_EncodeOnOff(rhalinfo.enable);
                    break;
                case SET_SPKOUT_MUTE:
                    status = RHAL_AUDIO_SetSPKOutMute(rhalinfo.enable);
                    break;
                case SET_SPDIFOUT_MUTE:
                    status = RHAL_AUDIO_SetSPDIFOutMute(rhalinfo.enable);
                    break;
                case SET_HPOUT_MUTE:
                    status = RHAL_AUDIO_SetHPOutMute(rhalinfo.enable);
                    break;
                case SET_SIF_MODE_SETUP:
                    status = RHAL_AUDIO_SIF_SetModeSetup(rhalinfo.sifStandard);
                    break;
                case SET_SIF_BAND_SETUP:
                    status = RHAL_AUDIO_SIF_SetBandSetup(rhalinfo.eSifType, rhalinfo.setSoundSystem);
                    break;
                case SET_SIF_INPUT_SOURCE:
                    status = RHAL_AUDIO_SIF_SetInputSource(rhalinfo.sifSource);
                    break;
                case SET_SIF_NO_SIGNAL_MUTE:
                    status = RHAL_AUDIO_SIF_NoSignalMute(rhalinfo.enable);
                    break;
                case SET_SIF_AUTO_CHANGE_SOUND_MODE:
                    status = RHAL_AUDIO_SIF_AutoChangeSoundMode(rhalinfo.enable);
                    break;
                case SET_SIF_FIRSTTIME_PLAYMONO:
                    status = RHAL_AUDIO_SIF_FirstTimePlayMono(rhalinfo.enable);
                    break;
                case SET_SIF_FW_PRIORITY:
                    status = RHAL_AUDIO_SIF_SetFwPriority(rhalinfo.u16var);
                    break;
                case SET_SIF_MIS_PRIORITY:
                    status = RHAL_Audio_SIF_SET_MTS_PRIORITY(rhalinfo.u16var);
                    break;
                case SET_SIF_HIGH_DEVMODE:
                    status = RHAL_AUDIO_SIF_SetHighDevMode(rhalinfo.enable);
                    break;
                case SET_SIF_USER_ANALOG_MODE:
                    status = RHAL_AUDIO_SIF_SetUserAnalogMode(rhalinfo.sifAudioMode);
                    break;
                case SET_SIF_A2_THRESHOLD_LEVEL:
                    status = RHAL_AUDIO_SIF_SetA2ThresholdLevel(rhalinfo.u16var);
                    break;
                case SET_SIF_NICAM_THRESHOLD_LEVEL:
                    status = RHAL_AUDIO_SIF_SetNicamThresholdLevel(rhalinfo.u16var);
                    break;
                case SET_SIF_AUDIO_EQ_MODE:
                    status = RHAL_AUDIO_SIF_SetAudioEQMode(rhalinfo.enable);
                    break;
                case SET_SIF_OVER_DEVIATION:
                    status = RHAL_AUDIO_SIF_SetOverDeviation(rhalinfo.overDeviation);
                    break;
                case SET_SIF_AUTO_CHANGE_SOUND_STD:
                    status = RHAL_AUDIO_SIF_AutoChangeSoundStd(rhalinfo.enable);
                    break;
                case SET_MIXER_INPUT_MUTE:
                    status = RHAL_AUDIO_SetMixerInputMute(rhalinfo.mixerIndex, rhalinfo.enable);
                    break;
                case SET_AC4_AUTO_PRESENTATION_AD_MIXING:
                    status = RHAL_AUDIO_AC4_SetAutoPresentationADMixing(rhalinfo.adecindex, rhalinfo.enable);
                    break;
                case SET_AC4_AUTO_PRESENTATION_AD_TYPE:
                    status = RHAL_AUDIO_AC4_SetAutoPresentationADType(rhalinfo.adecindex, rhalinfo.enADType);
                    break;
                case SET_AC4_AUTO_PRESENTATION_FIRST_LANGUAGE:
                    status = RHAL_AUDIO_AC4_SetAutoPresentationFirstLanguage(rhalinfo.adecindex, rhalinfo.enCodeType, rhalinfo.firstLang);
                    break;
                case SET_AC4_DIALOG_ENHANCEMENT_GAIN:
                    status = RHAL_AUDIO_AC4_SetDialogueEnhancementGain(rhalinfo.adecindex, rhalinfo.dialEnhanceGain);
                    break;
                case SET_HDMI_AUDIO_RETURN_CHANNEL:
                    status = RHAL_AUDIO_HDMI_SetAudioReturnChannel(rhalinfo.enable);
                    break;
                case SET_HDMI_AUDIO_ENHANCE_RETURN_CHANNEL:
                    status = RHAL_AUDIO_HDMI_SetEnhancedAudioReturnChannel(rhalinfo.enable);
                    break;
                case SET_DOLBY_DRCMODE:
                    status = RHAL_AUDIO_SetDolbyDRCMode(rhalinfo.adecindex, rhalinfo.drcMode);
                    break;
                case SET_DOLBY_DOWNMIXMODE:
                    status = RHAL_AUDIO_SetDownMixMode(rhalinfo.adecindex, rhalinfo.downmixMode);
                    break;
                case SET_DOLBY_DUALMONO_OUTPUTMODE:
                    status = RHAL_AUDIO_SetDualMonoOutMode(rhalinfo.adecindex, rhalinfo.outputMode);
                    break;
                case SET_HDMI_CONNECT:
                    status = RHAL_AUDIO_HDMI_Connect();
                    break;
                case SET_HDMI_DISCONNECT:
                    status = RHAL_AUDIO_HDMI_Disconnect();
                    break;
                case SET_DP_CONNECT:
                    status = RHAL_AUDIO_DP_Connect();
                    break;
                case SET_DP_DISCONNECT:
                    status = RHAL_AUDIO_DP_Disconnect();
                    break;
                case SET_AAD_CONNECT:
                    status = RHAL_AUDIO_AAD_Connect();
                    break;
                case SET_AAD_DISCONNECT:
                    status = RHAL_AUDIO_AAD_Disconnect();
                    break;
                case SET_HDMI_OPEN_PORT:
                    status = RHAL_AUDIO_HDMI_OpenPort(rhalinfo.hdmiIndex);
                    break;
                case SET_HDMI_CLOSE_PORT:
                    status = RHAL_AUDIO_HDMI_ClosePort(rhalinfo.hdmiIndex);
                    break;
                case SET_HDMI_CONNECT_PORT:
                    status = RHAL_AUDIO_HDMI_ConnectPort(rhalinfo.hdmiIndex);
                    break;
                case SET_HDMI_DISCONNECT_PORT:
                    status = RHAL_AUDIO_HDMI_DisconnectPort(rhalinfo.hdmiIndex);
                    break;
                case SET_GLOBAL_DECODER_GAIN:
                    status = RHAL_AUDIO_SetGlobalDecoderGain(rhalinfo.volume);
                    break;
                case SET_MIXER_INPUT_GAIN:
                    status = RHAL_AUDIO_SetMixerInputGain(rhalinfo.mixerIndex, rhalinfo.volume);
                    break;
                case SET_SPKOUT_VOLUME:
                    status = RHAL_AUDIO_SetSPKOutVolume(rhalinfo.volume);
                    break;
                case SET_SPDIFOUT_VOLUME:
                    status = SetSPDIFOutVolume(rhalinfo.volume);
                    break;
                case SET_HPOUT_VOLUME:
                    status = RHAL_AUDIO_SetHPOutVolume(rhalinfo.volume, rhalinfo.enable);
                    break;
                case SET_SCARTOUT_VOLUME:
                    status = RHAL_AUDIO_SetSCARTOutVolume(rhalinfo.volume, rhalinfo.enable);
                    break;
                case SET_AD_VOLUME:
                    status = RHAL_AUDIO_SetAudioDescriptionVolume(rhalinfo.adecindex, rhalinfo.volume);
                    break;
                case SET_SPKOUT_DELAY_TIME:
                    status = RHAL_AUDIO_SetSPKOutDelayTime(rhalinfo.delayTime, rhalinfo.enable);
                    break;
                case SET_HPOUT_DELAY_TIME:
                    status = RHAL_AUDIO_SetHPOutDelayTime(rhalinfo.delayTime, rhalinfo.enable);
                    break;
                case SET_SCARTOUT_DELAY_TIME:
                    status = RHAL_AUDIO_SetSCARTOutDelayTime(rhalinfo.delayTime, rhalinfo.enable);
                    break;
                case SET_SPDIFOUT_TYPE:
                    status = RHAL_AUDIO_SPDIF_SetOutputType(rhalinfo.eSPDIFMode, rhalinfo.enable);
                    break;
                case SET_SPDIF_COPY_INFO:
                    status = RHAL_AUDIO_SPDIF_SetCopyInfo(rhalinfo.copyInfo);
                    break;
                case SET_SPDIF_CATEGORY_CODE:
                    status = RHAL_AUDIO_SPDIF_SetCategoryCode(rhalinfo.u32var[0]);
                    break;
                case SET_PCM_VOLUME:
                    status = RHAL_AUDIO_PCM_SetVolume(rhalinfo.apcmIndex, rhalinfo.volume);
                    break;
                case SET_PCM_VOLUME_BYPIN:
                    status = RHAL_AUDIO_PCM_SetVolume_ByPin(rhalinfo.apcmIndex, rhalinfo.volume, rhalinfo.pinID);
                    break;
                case SET_PCM_MUTE_BYPIN:
                    status = RHAL_AUDIO_PCM_SetMute_ByPin(rhalinfo.enable, rhalinfo.pinID);
                    break;
                case SET_DBXTV_SOUND_EFFECT:
                    status = RHAL_AUDIO_DBXTV_SET_SOUND_EFFECT(rhalinfo.paramType, rhalinfo.u32var[0]);
                    break;
                case SET_INPUT_VOLUME:
                    status = RHAL_AUDIO_SetInputVolume(rhalinfo.eSource, rhalinfo.s32var);
                    break;
                case SET_VOLUME_WITH_FADE:
                    status = RHAL_AUDIO_SetVolumeWithFade(rhalinfo.volume, rhalinfo.u32var[0], rhalinfo.u32var[1]);
                    break;
                case SET_SPDIF_ENABLE:
                    status = RHAL_AUDIO_SPDIF_SetEnable(rhalinfo.enable);
                    break;
                case SET_DTS_DECODER_PARAM:
                    status = RHAL_AUDIO_SetDTSDecoderParam(rhalinfo.dts_paramType, rhalinfo.u32var[0]);
                    break;
                case SET_ANALOG_ADJ_DB:
                    status = RHAL_AUDIO_SetAnalogAdjDB(rhalinfo.area_type);
                    break;
                case SET_DOLBY_OTT_MODE:
                    status = RHAL_AUDIO_SetDolbyOTTMode(rhalinfo.u32var[0], rhalinfo.u32var[1]);
                    break;
                case SET_AC4_AUTO_PRESENTATION_SECOND_LANGUAGE:
                    status = RHAL_AUDIO_AC4_SetAutoPresentationSecondLanguage(rhalinfo.adecindex, rhalinfo.enCodeType, rhalinfo.u32var[0]);
                    break;
                case SET_AC4_AUTO_PRESENTATION_PRIORITIZE_AD_TYPE:
                    status = RHAL_AUDIO_AC4_SetAutoPresentationPrioritizeADType(rhalinfo.adecindex, rhalinfo.u32var[0]);
                    break;
                case SET_AC4_AUTO_PRESENTATION_GROUP_INDEX:
                    status = RHAL_AUDIO_AC4_SetPresentationGroupIndex(rhalinfo.adecindex, rhalinfo.u32var[0]);
                    break;
                case SET_SOUND_EFFECT_LEVEL:
                    status = RHAL_AUDIO_SoundEffect_Level(rhalinfo.u32var[0], rhalinfo.u32var[1], rhalinfo.u32var[2]);
                    break;
                case SET_LOW_LATENCY_MODE:
                    status = RHAL_AUDIO_SetLowLatencyMode(rhalinfo.enable);
                    break;
                case SET_AI_SOUND_ENABLE:
                    status = RHAL_AUDIO_AISound_Enable(rhalinfo.enable);
                    break;
                case SET_BT_MIC_ENABLE:
                    status = RHAL_AUDIO_BTMic_Enable(rhalinfo.enable);
                    break;
                case SET_AEC_REFERENCE_SOURCE:
                    status = RHAL_AUDIO_SetAECReference_Source(rhalinfo.aec_source);
                    break;
                default:
                    status = NOT_OK;
                    break;
            }
            if(status != OK)
                ret = -EFAULT;
            break;
        }
        case SET_CHANNEL_OUT_SWAP:
        {
            AUDIO_RPC_AO_CHANNEL_OPT_SWAP sel;
            unsigned long rpc_res;

            if (copy_from_user(&sel, (const void __user *) (long)info.addr, info.size))
            {
                rtd_pr_adsp_err("%s SET_CHANNEL_OUT_SWAP fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }
            rpc_res = RTKAUDIO_RPC_TOAGENT_SET_CHANNEL_OUT_SWAP_SVC(&sel);
            if(rpc_res != S_OK)
                ret = -EFAULT;

            break;
        }
        case SET_SEND_SPECTRUM_DATA:
        {
            AUDIO_SPECTRUM_CFG cfg;
            unsigned long rpc_res;

            if (copy_from_user(&cfg, (const void __user *) (long)info.addr, info.size))
            {
                rtd_pr_adsp_err("%s SEND_SPECTRUM_DATA fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }
            rpc_res = RTKAUDIO_RPC_TOAGENT_SET_SEND_SPECTRUM_DATA_SVC(&cfg);
            if(rpc_res != S_OK)
                ret = -EFAULT;

            break;
        }
        case SET_AUDIO_AUTH_KEY:
        {
            long customer_key;
            unsigned long rpc_res;

            if (copy_from_user(&customer_key, (const void __user *) (long)info.addr, info.size))
            {
                rtd_pr_adsp_err("%s [SET_AUDIO_AUTH_KEY] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }
            rpc_res = RTKAUDIO_RPC_TOAGENT_SET_AUTHORITY_KEY_SVC(&customer_key);
            if(rpc_res != S_OK)
                ret = -EFAULT;

            break;
        }
        case SET_MODEL_HASH:
        {
            AUDIO_DUMMY_DATA hash;
            unsigned long rpc_res;

            if (copy_from_user(&hash, (const void __user *) (long)info.addr, info.size))
            {
                rtd_pr_adsp_err("%s [SET_MODEL_HASH] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }
            rpc_res = RTKAUDIO_RPC_TOAGENT_SET_MODEL_HASH_SVC(&hash);
            if(rpc_res != S_OK)
                ret = -EFAULT;

            break;
        }
        case SET_AUDIO_CAPABILITY:
        {
            AUDIO_LICENSE_STATUS_CONFIG lcns;
            unsigned long rpc_res;

            if (copy_from_user(&lcns, (const void __user *) (long)info.addr, info.size))
            {
                rtd_pr_adsp_err("%s [SET_AUDIO_CAPABILITY] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }
            rpc_res = RTKAUDIO_RPC_TOAGENT_SET_AUDIO_CAPABILITY_SVC(&lcns);
            if(rpc_res != S_OK)
                ret = -EFAULT;

            break;
        }
        case SET_AVSYNC_OFFSET:
        {
            AUDIO_HDMI_OUT_VSDB_DATA data;
            unsigned long rpc_res;

            if (copy_from_user(&data, (const void __user *) (long)info.addr, info.size))
            {
                rtd_pr_adsp_err("%s [SET_AVSYNC_OFFSET] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }
            rpc_res = RTKAUDIO_RPC_TOAGENT_SET_AVSYNC_OFFSET_SVC(&data);
            if(rpc_res != S_OK)
                ret = -EFAULT;

            break;
        }
        case SET_INIT_RING_BUFFER_HEADER:
        {
            AUDIO_RPC_RINGBUFFER_HEADER header;
            unsigned long rpc_res;

            if (copy_from_user(&header, (const void __user *) (long)info.addr, info.size))
            {
                rtd_pr_adsp_err("%s [INIT_RING_BUFFER_HEADER] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }
            rpc_res = RTKAUDIO_RPC_TOAGENT_INIT_RING_BUFFER_HEADER_SVC(&header);
            if(rpc_res != S_OK)
                ret = -EFAULT;

            break;
        }
        case SET_SPDIFOUT_PIN_SRC:
        {
            AUDIO_RPC_SPDIFO_SOURCE source;
            unsigned long rpc_res;

            if (copy_from_user(&source, (const void __user *) (long)info.addr, info.size))
            {
                rtd_pr_adsp_err("%s [SET_SPDIFOUT_PIN_SRC] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }
            rpc_res = RTKAUDIO_RPC_TOAGENT_SET_SPDIF_OUT_PIN_SRC_SVC(&source);
            if(rpc_res != S_OK)
                ret = -EFAULT;

            break;
        }
        case SET_TRUVOLUME:
        {
            AUDIO_RPC_TRUVOLUME vol;
            unsigned long rpc_res;

            if (copy_from_user(&vol, (const void __user *) (long)info.addr, info.size))
            {
                rtd_pr_adsp_err("%s [SET_TRUVOLUME] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }
            rpc_res = RTKAUDIO_RPC_TOAGENT_SET_TRUVOLUME_SVC(&vol);
            if(rpc_res != S_OK)
                ret = -EFAULT;

            break;
        }
        case SET_SWP_SRS_TRUSURROUNDHD:
        {
            DTV_STATUS_T status;
            HAL_AUDIO_RHAL_CONFIG_T rhalconfig;
            HAL_APP_TSXT_CFG TSXT;

            if (copy_from_user(&rhalconfig, (const void __user *) (long)info.addr, info.size))
            {
                rtd_pr_adsp_err("%s [SET_TRUVOLUMEHD_PARAM] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }

            if (copy_from_user(&TSXT, (const void __user *) (long)rhalconfig.addr[0], rhalconfig.size[0]))
            {
                rtd_pr_adsp_err("%s [SET_SWP_SRS_TRUSURROUNDHD] rhalinfo.addr copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
            }

            status = RHAL_AUDIO_SwpSetSRS_TrusurroundHD(rhalconfig.rhalinfo.u32var[0], &TSXT);
            if(status != OK) {
                rtd_pr_adsp_err("KADP_AUDIO_SwpSetSRS_TrusurroundHD return fail\n");
                ret = -EFAULT;
            }
            break;
        }
        case SET_VX_PARAM:
        {
            DTV_STATUS_T status;
            HAL_AUDIO_RHAL_CONFIG_T rhalconfig;
            void* pData;
            if (copy_from_user(&rhalconfig, (const void __user *) (long)info.addr, info.size))
            {
                rtd_pr_adsp_err("%s [SET_VX_PARAM] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }

            pData = kmalloc(rhalconfig.size[0], GFP_KERNEL);

            if (pData == NULL) {
                rtd_pr_adsp_err("[rtkaudio] malloc memory failed, %d\n", __LINE__);
                return -EFAULT;
            }

            if (copy_from_user(pData, (const void __user *) (long)rhalconfig.addr[0], rhalconfig.size[0]))
            {
                rtd_pr_adsp_err("%s [SET_VX_PARAM] rhalinfo.addr copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
            }

            status = RHAL_AUDIO_VX_SetParameter(rhalconfig.rhalinfo.vx_paramtype, pData);
            if(status != OK) {
                rtd_pr_adsp_err("RHAL_AUDIO_VX_SetParameter return fail\n");
                ret = -EFAULT;
            }
            kfree(pData);
            break;
        }
        case SET_TRUVOLUMEHD_PARAM:
        {
            DTV_STATUS_T status;
            HAL_AUDIO_RHAL_CONFIG_T rhalconfig;
            void* pData;
            if (copy_from_user(&rhalconfig, (const void __user *) (long)info.addr, info.size))
            {
                rtd_pr_adsp_err("%s [SET_TRUVOLUMEHD_PARAM] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }

            pData = kmalloc(rhalconfig.size[0], GFP_KERNEL);

            if (pData == NULL) {
                rtd_pr_adsp_err("[rtkaudio] malloc memory failed, %d\n", __LINE__);
                return -EFAULT;
            }

            if (copy_from_user(pData, (const void __user *) (long)rhalconfig.addr[0], rhalconfig.size[0]))
            {
                rtd_pr_adsp_err("%s [SET_TRUVOLUMEHD_PARAM] rhalinfo.addr copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
            }

            status = RHAL_AUDIO_TruvolumeHD_SetParameter(rhalconfig.rhalinfo.tvolhd_paramType, pData);
            if(status != OK) {
                rtd_pr_adsp_err("RHAL_AUDIO_TruvolumeHD_SetParameter return fail\n");
                ret = -EFAULT;
            }
            kfree(pData);
            break;
        }
        case SET_DBXTV_TABLE:
        {
            DTV_STATUS_T status;
            HAL_AUDIO_RHAL_CONFIG_T rhalconfig;
            UINT32* table;

            if (copy_from_user(&rhalconfig, (const void __user *) (long)info.addr, info.size))
            {
                rtd_pr_adsp_err("%s [SET_DBXTV_TABLE] info.addr copy_from_user fail\n",__FUNCTION__);
                return -EFAULT;
            }

            table = kmalloc(rhalconfig.size[0], GFP_KERNEL);
            if (table == NULL) {
                rtd_pr_adsp_err("[rtkaudio] malloc memory failed, %d\n", __LINE__);
                return -EFAULT;
            }

            if (copy_from_user(table, (const void __user *) (long)rhalconfig.addr[0], rhalconfig.size[0]))
            {
                rtd_pr_adsp_err("%s [SET_DBXTV_TABLE] rhalinfo.addr copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
            }

            status = RHAL_AUDIO_DBXTV_SET_TABLE(table, rhalconfig.size[0]);
            if(status != OK) {
                rtd_pr_adsp_err("RHAL_AUDIO_DBXTV_SET_TABLE return fail\n");
                ret = -EFAULT;
            }
            kfree(table);
            break;
        }
        case SET_DAP_TUNING_DATA:
        {
            DTV_STATUS_T status;
            HAL_AUDIO_RHAL_CONFIG_T rhalconfig;
            void* pData;

            if (copy_from_user(&rhalconfig, (const void __user *) (long)info.addr, info.size))
            {
                rtd_pr_adsp_err("%s [SET_DAP_TUNING_DATA] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }

            pData = kmalloc(rhalconfig.size[0], GFP_KERNEL);

            if (pData == NULL) {
                rtd_pr_adsp_err("[rtkaudio] malloc memory failed, %d\n", __LINE__);
                return -EFAULT;
            }

            if (copy_from_user(pData, (const void __user *) (long)rhalconfig.addr[0], rhalconfig.size[0]))
            {
                rtd_pr_adsp_err("%s [SET_DAP_TUNING_DATA] rhalinfo.addr copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
            }

            status = RHAL_AUDIO_SetDAPTuningData(pData, rhalconfig.size[0]);
            if(status != OK) {
                rtd_pr_adsp_err("RHAL_AUDIO_SetDAPTuningData return fail\n");
                ret = -EFAULT;
            }
            kfree(pData);
            break;
        }
		case VIRTUALX_CMD_INFO:
		{
			virtualx_cmd_info virx_cmd;
			void *vir_addr = NULL;
			void *unvir_addr = NULL;
			unsigned long phy_addr = 0;

            if (drv_debug)
                rtd_pr_adsp_debug("[Virtual X] %s\n", __func__);

			if (info.size != sizeof(virtualx_cmd_info))
			{
				ret = -EFAULT;
				break;
			}

			if (copy_from_user(&virx_cmd, (const void __user *) (long) info.addr, info.size))
			{
				ret = -EFAULT;
				break;
			}

			vir_addr = get_rpc_mem(&unvir_addr);
			if (!vir_addr) {
				rtd_pr_adsp_err("[Virtual X] %d alloc memory fail\n", __LINE__);
				return -ENOMEM;
			}
			phy_addr = (unsigned long) dvr_to_phys(vir_addr);

			/* set parameter to 0, for snapshot */
			memset(unvir_addr, 0, 4096);

			send_virtualx_rpc(unvir_addr, virx_cmd, phy_addr, 1);

			put_rpc_mem(vir_addr);

			if (copy_to_user((void __user *) (long) info.addr, &virx_cmd, info.size))
			{
				ret = -EFAULT;
			}

			break;
		}
		case ACMD_INFO:
		{
			omx_rpc_cmd_t omx_cmd;
			unsigned int rpc_res;
			if (copy_from_user(&omx_cmd, (const void __user *)(long) info.addr,sizeof(omx_rpc_cmd_t)))
			{
				ret = -EFAULT;
				break;
			}
			switch(omx_cmd.cmd_type)
			{
				case ACMD_PRIVATE_INFO:
				{
					AUDIO_RPC_PRIVATEINFO_PARAMETERS parameter;
					AUDIO_RPC_PRIVATEINFO_RETURNVAL res;
					if (copy_from_user(&parameter, (const void __user *)(long) omx_cmd.addr,sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS)))
					{
						ret = -EFAULT;
						break;
					}
					rpc_res = RTKAUDIO_RPC_TOAGENT_PRIVATEINFO_SVC(&parameter, &res);
					if(rpc_res != S_OK) rtd_pr_adsp_err("[ACMD_PRIVATE_INFO] RPC return != S_OK\n");
					if (copy_to_user((void __user *) (long) omx_cmd.addr, &res, sizeof(AUDIO_RPC_PRIVATEINFO_RETURNVAL)))
						ret = -EFAULT;
					if (copy_to_user((void __user *) (long) info.addr, &omx_cmd, sizeof(omx_rpc_cmd_t)))
						ret = -EFAULT;
				}
				break;

			default:
				ret = -EFAULT;
				break;
			}
			break;
		}
		case EQ_BASS_CMD_INFO:
		{
			eq_bass_cmd_info eq_bass_cmd;
			ENUM_AUDIO_EQ_BASS_MODE mode;
			ENUM_AUDIO_EQ_BASS_FILTER_TYPE filter_type;
			int band_idx, bOnOff, fc, bw, gain;

			if (info.size != sizeof(eq_bass_cmd_info))
			{
				rtd_pr_adsp_err("rtkaudio_ioctl_set %d, info.size:%d, eq_bass_cmd_info size:%d\n", __LINE__, info.size, sizeof(eq_bass_cmd_info));
				ret = -EFAULT;
				break;
			}

			if (copy_from_user(&eq_bass_cmd, (const void __user *) (long) info.addr, info.size))
			{
				ret = -EFAULT;
				break;
			}

			mode = (ENUM_AUDIO_EQ_BASS_MODE)eq_bass_cmd.data[0];

			switch(eq_bass_cmd.type)
			{
				case ENUM_EQ_BASS_CMD_TYPE_SETENABLE:
				{
					if (eq_bass_cmd.size != sizeof(int)*3)
					{
						rtd_pr_adsp_err("ENUM_EQ_BASS_CMD_TYPE_SETENABLE fail\n");
						return -EFAULT;
					}
					band_idx = eq_bass_cmd.data[1];
					bOnOff   = eq_bass_cmd.data[2];
					set_EQ_BASS_enable(mode, band_idx, bOnOff);
					break;
				}
				case ENUM_EQ_BASS_CMD_TYPE_SETPARAM:
				{
					if (eq_bass_cmd.size != sizeof(int)*6)
					{
						rtd_pr_adsp_err("ENUM_EQ_BASS_CMD_TYPE_SETPARAM fail\n");
						return -EFAULT;
					}
					band_idx = eq_bass_cmd.data[1];
					filter_type = (ENUM_AUDIO_EQ_BASS_FILTER_TYPE)eq_bass_cmd.data[2];
					fc = eq_bass_cmd.data[3];
					bw = eq_bass_cmd.data[4];
					gain =eq_bass_cmd.data[5];
					set_EQ_BASS_parameter(mode, band_idx, filter_type, fc, bw, gain);
					break;
				}
				case ENUM_EQ_BASS_CMD_TYPE_SET_AUDIO_DATA_PATH:
				{
					if (eq_bass_cmd.size != sizeof(int)*2)
					{
						rtd_pr_adsp_err("ENUM_EQ_BASS_CMD_TYPE_SET_AUDIO_DATA_PATH fail\n");
						return -EFAULT;
					}
					bOnOff = eq_bass_cmd.data[1];
					set_EQ_BASS_Audio_Data_Path_Enable(mode, bOnOff);
					break;
				}
			}

			if (copy_to_user((void __user *) (long) info.addr, &eq_bass_cmd, info.size))
			{
				ret = -EFAULT;
			}

			break;
		}

		default:
			ret = -EFAULT;
		}
		break;
	}
	case RTKAUDIO_IOC_GET:
	{
		struct cntl_info info;
        if (drv_debug)
            rtd_pr_adsp_debug("RTKAUDIO_IOCTL_GET\n");


		if (copy_from_user((void*)&info, (const void __user*)arg, sizeof(info))) {
			ret = -EFAULT;
		}

		switch(info.resource_type) {
        case GET_SIF_IS_EXIST:
        {
            BOOLEAN IsExist;
            HAL_AUDIO_RHAL_INFO_T rhalinfo;

            if (copy_from_user(&rhalinfo, (const void __user *) (long)info.addr, info.size))
            {
                rtd_pr_adsp_err("%s [GET_SIF_IS_EXIST] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }

            IsExist = RHAL_AUDIO_SIF_IsSIFExist();
            rhalinfo.IsExist = IsExist;

            if (copy_to_user((void __user *)(long)info.addr, &rhalinfo, info.size)) {
                rtd_pr_adsp_err("%s [GET_SIF_IS_EXIST] copy_to_user fail\n",__FUNCTION__);
                return -EFAULT;
            }
            break;
        }
        case GET_TP_IS_ES_EXIST:
        {
            BOOLEAN IsExist;
            HAL_AUDIO_RHAL_INFO_T rhalinfo;

            if (copy_from_user(&rhalinfo, (const void __user *) (long)info.addr, info.size))
            {
                rtd_pr_adsp_err("%s [GET_TP_IS_ES_EXIST] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }

            IsExist = RHAL_AUDIO_TP_IsESExist(rhalinfo.adecindex);
            rhalinfo.IsExist = IsExist;

            if (copy_to_user((void __user *)(long)info.addr, &rhalinfo, info.size)) {
                rtd_pr_adsp_err("%s [GET_TP_IS_ES_EXIST] copy_to_user fail\n",__FUNCTION__);
                return -EFAULT;
            }
            break;
        }
        case GET_RHAL_INFO:
        {
            DTV_STATUS_T status;
            HAL_AUDIO_RHAL_CONFIG_T rhalconfig;
            void *p_var;

            if (copy_from_user(&rhalconfig, (const void __user *) (long)info.addr, info.size))
            {
                rtd_pr_adsp_err("%s [GET_RHAL_INFO] info.addr copy_from_user fail\n",__FUNCTION__);
                return -EFAULT;
            }

            switch(rhalconfig.type)
            {
                case GET_TP_AUDIO_PTS:
                {
                    UINT32 pts;
                    p_var = &pts;

                    if (copy_from_user(p_var, (const void __user *) (long)rhalconfig.addr[0], rhalconfig.size[0]))
                    {
                        rtd_pr_adsp_err("%s [GET_DAP_PARAM] rhalinfo.addr copy_from_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    status = RHAL_AUDIO_TP_GetAudioPTS(rhalconfig.rhalinfo.adecindex, p_var);

                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }

                    if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;

                }
                case GET_TP_AUDIO_PTS64:
                {
                    UINT64 pts;
                    p_var = &pts;

                    if (copy_from_user(p_var, (const void __user *) (long)rhalconfig.addr[0], rhalconfig.size[0]))
                    {
                        rtd_pr_adsp_err("%s [GET_DAP_PARAM] rhalinfo.addr copy_from_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    status = RHAL_AUDIO_TP_GetAudioPTS64(rhalconfig.rhalinfo.adecindex, p_var);

                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }

                    if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_TP_BUFFER_STATUS:
                {
                    HAL_AUDIO_RHAL_BUFFER_INFO_T bufinfo;
                    p_var = &bufinfo;
                    if (copy_from_user(p_var, (const void __user *) (long)rhalconfig.addr[0], rhalconfig.size[0]))
                    {
                        rtd_pr_adsp_err("%s [GET_DAP_PARAM] rhalinfo.addr copy_from_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    status = RHAL_AUDIO_TP_GetBufferStatus(rhalconfig.rhalinfo.adecindex, &bufinfo.MaxSize, &bufinfo.FreeSize);

                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }

                    if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_ES_INFO:
                {
                    HAL_AUDIO_ES_INFO_T ESInfo;
                    p_var = &ESInfo;

                    if (copy_from_user(&ESInfo, (const void __user *) (long)rhalconfig.addr[0], rhalconfig.size[0]))
                    {
                        rtd_pr_adsp_err("%s [GET_ES_INFO] rhalinfo.addr copy_from_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    status = RHAL_AUDIO_GetESInfo(rhalconfig.rhalinfo.adecindex, &ESInfo);
                    if(status != OK) {
                        rtd_pr_adsp_err("RHAL_AUDIO_TP_GetESInfo return fail\n");
                        return -EFAULT;
                    }

                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }

                    if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_DECODER_INPUT_GAIN:
                {
                    HAL_AUDIO_VOLUME_T volinfo;
                    p_var = &volinfo;

                    if (copy_from_user(p_var, (const void __user *) (long)rhalconfig.addr[0], rhalconfig.size[0]))
                    {
                        rtd_pr_adsp_err("%s [GET_DAP_PARAM] rhalinfo.addr copy_from_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    status = RHAL_AUDIO_GetDecoderInputGain(rhalconfig.rhalinfo.adecindex, p_var);

                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }

                    if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_TP_ES_INFO:
                {
                    HAL_AUDIO_ES_INFO_T ESInfo;
                    p_var = &ESInfo;

                    if (copy_from_user(p_var, (const void __user *) (long)rhalconfig.addr[0], rhalconfig.size[0]))
                    {
                        rtd_pr_adsp_err("%s [GET_DAP_PARAM] rhalinfo.addr copy_from_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    status = RHAL_AUDIO_TP_GetESInfo(rhalconfig.rhalinfo.adecindex, p_var);

                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }

                    if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_DECODE_TYPE:
                {
                    HAL_AUDIO_SRC_TYPE_T SRCInfo;
                    p_var = &SRCInfo;

                    if (copy_from_user(p_var, (const void __user *) (long)rhalconfig.addr[0], rhalconfig.size[0]))
                    {
                        rtd_pr_adsp_err("%s [GET_DAP_PARAM] rhalinfo.addr copy_from_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    status = RHAL_AUDIO_GetDecodingType(rhalconfig.rhalinfo.adecindex, p_var);

                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }

                    if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_ADEC_STATUS:
                {
                    HAL_AUDIO_ADEC_INFO_T AdecInfo;
                    p_var = &AdecInfo;

                    if (copy_from_user(p_var, (const void __user *) (long)rhalconfig.addr[0], rhalconfig.size[0]))
                    {
                        rtd_pr_adsp_err("%s [GET_DAP_PARAM] rhalinfo.addr copy_from_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    status = RHAL_AUDIO_GetAdecStatus(rhalconfig.rhalinfo.adecindex, p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_SIF_DETECT_COMPLETE:
                {
                    SINT32 isOff;
                    p_var = &isOff;
                    status = RHAL_AUDIO_SIF_DetectComplete(p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_STATUS_INFO:
                {
                    HAL_AUDIO_COMMON_INFO_T AudioStatusInfo;
                    p_var = &AudioStatusInfo;
                    status = RHAL_AUDIO_GetStatusInfo(p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_SCARTOUT_MUTE_STATUS:
                {
                    BOOLEAN bOnOff;
                    p_var = &bOnOff;
                    status = RHAL_AUDIO_GetSCARTOutMuteStatus(p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_HPOUT_MUTE_STATUS:
                {
                    BOOLEAN bOnOff;
                    p_var = &bOnOff;
                    status = RHAL_AUDIO_GetHPOutMuteStatus(p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_SPDIFOUT_MUTE_STATUS:
                {
                    BOOLEAN bOnOff;
                    p_var = &bOnOff;
                    status = RHAL_AUDIO_GetSPDIFOutMuteStatus(p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_SPKOUT_MUTE_STATUS:
                {
                    BOOLEAN bOnOff;
                    p_var = &bOnOff;
                    status = RHAL_AUDIO_GetSPKOutMuteStatus(p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_DECODER_INPUT_MUTE:
                {
                    BOOLEAN bOnOff;
                    p_var = &bOnOff;
                    status = RHAL_AUDIO_GetDecoderInputMute(rhalconfig.rhalinfo.adecindex, p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_MIXER_INPUT_MUTE:
                {
                    BOOLEAN bOnOff;
                    p_var = &bOnOff;
                    status = RHAL_AUDIO_GetMixerInputMute(rhalconfig.rhalinfo.mixerIndex, p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_MIXER_INPUT_GAIN:
                {
                    HAL_AUDIO_VOLUME_T volume;
                    p_var = &volume;
                    status = RHAL_AUDIO_GetMixerInputGain(rhalconfig.rhalinfo.mixerIndex, p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_SNDOUT_LATENCY:
                {
                    int latency_ms;
                    p_var = &latency_ms;
                    status = RHAL_AUDIO_SNDOUT_GetLatency(rhalconfig.rhalinfo.soundOutType, p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_DIGITAL_INFO:
                {
                    HAL_AUDIO_DIGITAL_INFO_T AudioDigitalInfo;
                    p_var = &AudioDigitalInfo;
                    status = RHAL_AUDIO_GetDigitalInfo(rhalconfig.rhalinfo.adecindex, p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_SIF_SOUND_STANDARD:
                {
                    HAL_AUDIO_SIF_STANDARD_T standard;
                    p_var = &standard;
                    status = RHAL_AUDIO_SIF_GetSoundStandard(p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_SIF_CURRENT_ANALOG_MODE:
                {
                    HAL_AUDIO_SIF_MODE_GET_T SifAudioMode;
                    p_var = &SifAudioMode;
                    status = RHAL_AUDIO_SIF_GetCurAnalogMode(p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_SIF_NICAM_STABLE:
                {
                    UINT16 IsStable;
                    p_var = &IsStable;
                    status = RHAL_AUDIO_SIF_GetNicamStable(p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_SIF_NICAM_THRESHOLD_LEVEL:
                {
                    UINT16 Level;
                    p_var = &Level;
                    status = RHAL_AUDIO_SIF_GetNicamThresholdLevel(p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_SIF_A2_STEREO_LEVEL:
                {
                    UINT16 Level;
                    p_var = &Level;
                    status = RHAL_AUDIO_SIF_GetA2StereoLevel(p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_SIF_CHECK_A2DK:
                {
                    HAL_AUDIO_SIF_EXISTENCE_INFO_T Availability;
                    p_var = &Availability;
                    status = RHAL_AUDIO_SIF_CheckA2DK(rhalconfig.rhalinfo.sifStandard, p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_SIF_CHECK_AVAILABLE_SYSTEM:
                {
                    HAL_AUDIO_SIF_EXISTENCE_INFO_T Availability;
                    p_var = &Availability;
                    status = RHAL_AUDIO_SIF_CheckAvailableSystem(rhalconfig.rhalinfo.availStandard, p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_SIF_CHECK_NICAM_DIGIT:
                {
                    HAL_AUDIO_SIF_EXISTENCE_INFO_T IsNicamDetect;
                    p_var = &IsNicamDetect;
                    status = RHAL_AUDIO_SIF_CheckNicamDigital(p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_SIF_BAND_DETECT:
                {
                    UINT32 BandStrength;
                    p_var = &BandStrength;
                    status = RHAL_AUDIO_SIF_GetBandDetect(rhalconfig.rhalinfo.setSoundSystem, p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_SIF_SOUND_INFO:
                {
                    HAL_ATV_SOUND_INFO_T SoundInfo;
                    p_var = &SoundInfo;
                    status = RHAL_AUDIO_SIF_GetSoundInfo(p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                default:
                    status = NOT_OK;
                    rtd_pr_adsp_err("GET_RHAL_INFO fail in unknown type = %d\n", rhalconfig.type);
                    return -EFAULT;
            }
#if 0 // p_var  points to local variable
            if(status != OK) {
                rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                return -EFAULT;
            }

            if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var, rhalconfig.size[0])) {
                rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                return -EFAULT;
            }
#endif
        }
        case GET_DBXTV_DEBUG_CMD:
        {
            DTV_STATUS_T status;
            HAL_AUDIO_RHAL_CONFIG_T rhalconfig;
            UINT32 *param;

            if (copy_from_user(&rhalconfig, (const void __user *) (long)info.addr, info.size))
            {
                rtd_pr_adsp_err("%s [GET_DBXTV_DEBUG_CMD] copy_from_user info.addr fail\n",__FUNCTION__);
                return -EFAULT;
            }

            param = kmalloc(rhalconfig.size[0], GFP_KERNEL);

            if (param == NULL) {
                rtd_pr_adsp_err("[GET_DBXTV_DEBUG_CMD] malloc memory failed, %d\n", __LINE__);
                return -EFAULT;
            }

            if (copy_from_user(param, (const void __user *) (long)rhalconfig.addr[0], rhalconfig.size[0]))
            {
                rtd_pr_adsp_err("%s [GET_DBXTV_DEBUG_CMD] copy_from_user addr[0] fail\n",__FUNCTION__);
                ret = -EFAULT;
            }

            status = RHAL_AUDIO_DBXTV_DEBUG_CMD(rhalconfig.rhalinfo.u32var[0], param, rhalconfig.size[0]);
            if(status != OK) {
                rtd_pr_adsp_err("RHAL_AUDIO_DBXTV_DEBUG_CMD return fail\n");
                ret = -EFAULT;
            }

            if (copy_to_user((void __user *)(long)rhalconfig.addr[0], param, rhalconfig.size[0])) {
                rtd_pr_adsp_err("%s [GET_DBXTV_DEBUG_CMD] copy_to_user fail\n",__FUNCTION__);
                ret = -EFAULT;
            }
            kfree(param);
            break;
        }
        case GET_VX_PARAM:
        {
            DTV_STATUS_T status;
            HAL_AUDIO_RHAL_CONFIG_T rhalconfig;
            void* pData;
            if (copy_from_user(&rhalconfig, (const void __user *) (long)info.addr, info.size))
            {
                rtd_pr_adsp_err("%s [GET_VX_PARAM] copy_from_user info.addr fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }

            pData = kmalloc(rhalconfig.size[0], GFP_KERNEL);

            if (pData == NULL) {
                rtd_pr_adsp_err("[GET_VX_PARAM] malloc memory failed, %d\n", __LINE__);
                return -EFAULT;
            }

            if (copy_from_user(pData, (const void __user *) (long)rhalconfig.addr[0], rhalconfig.size[0]))
            {
                rtd_pr_adsp_err("%s [GET_VX_PARAM] copy_from_user addr[0] fail\n",__FUNCTION__);
                ret = -EFAULT;
            }

            status = RHAL_AUDIO_VX_GetParameter(rhalconfig.rhalinfo.vx_paramtype, pData);
            if(status != OK) {
                rtd_pr_adsp_err("RHAL_AUDIO_VX_GetParameter return fail\n");
                ret = -EFAULT;
            }

            if (copy_to_user((void __user *)(long)rhalconfig.addr[0], pData, rhalconfig.size[0])) {
                  rtd_pr_adsp_err("%s [GET_VX_PARAM] copy_to_user fail\n",__FUNCTION__);
                  ret = -EFAULT;
            }
            kfree(pData);
            break;
        }
        case GET_TRUVOLUMEHD_PARAM:
        {
            DTV_STATUS_T status;
            HAL_AUDIO_RHAL_CONFIG_T rhalconfig;
            void* pData;
            if (copy_from_user(&rhalconfig, (const void __user *) (long)info.addr, info.size))
            {
                rtd_pr_adsp_err("%s [GET_TRUVOLUMEHD_PARAM] copy_from_user info.addr fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }

            pData = kmalloc(rhalconfig.size[0], GFP_KERNEL);

            if (pData == NULL) {
                rtd_pr_adsp_err("[GET_TRUVOLUMEHD_PARAM] malloc memory failed, %d\n", __LINE__);
                return -EFAULT;
            }

            if (copy_from_user(pData, (const void __user *) (long)rhalconfig.addr[0], rhalconfig.size[0]))
            {
                rtd_pr_adsp_err("%s [GET_TRUVOLUMEHD_PARAM] copy_from_user addr[0] fail\n",__FUNCTION__);
                ret = -EFAULT;
            }

            status = RHAL_AUDIO_TruvolumeHD_GetParameter(rhalconfig.rhalinfo.tvolhd_paramType, pData);
            if(status != OK) {
                rtd_pr_adsp_err("RHAL_AUDIO_TruvolumeHD_GetParameter return fail\n");
                ret = -EFAULT;
            }

            if (copy_to_user((void __user *)(long)rhalconfig.addr[0], pData, rhalconfig.size[0])) {
                  rtd_pr_adsp_err("%s [GET_TRUVOLUMEHD_PARAM] copy_to_user fail\n",__FUNCTION__);
                  ret = -EFAULT;
            }
            kfree(pData);
            break;
        }
        case GET_DAP_PARAM:
        {
            DTV_STATUS_T status;
            HAL_AUDIO_RHAL_CONFIG_T rhalconfig;
            void *paramValue;

            if (copy_from_user(&rhalconfig, (const void __user *) (long)info.addr, info.size))
            {
                rtd_pr_adsp_err("%s [GET_DAP_PARAM] info.addr copy_from_user fail\n",__FUNCTION__);
                return -EFAULT;
            }

            paramValue = kmalloc(rhalconfig.size[0], GFP_KERNEL);

            if (paramValue == NULL) {
                rtd_pr_adsp_err("[GET_DAP_PARAM] malloc memory failed, %d\n", __LINE__);
                return -EFAULT;
            }

            if (copy_from_user(paramValue, (const void __user *) (long)rhalconfig.addr[0], rhalconfig.size[0]))
            {
                rtd_pr_adsp_err("%s [GET_DAP_PARAM] rhalinfo.addr copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
            }

            status = RHAL_AUDIO_GetDAPParam(rhalconfig.rhalinfo.dap_paramType, paramValue, rhalconfig.size[0]);
            if(status != OK) {
                rtd_pr_adsp_err("RHAL_AUDIO_GetDAPParam return fail\n");
                ret = -EFAULT;
            }

            if (copy_to_user((void __user *)(long)rhalconfig.addr[0], paramValue, rhalconfig.size[0])) {
                rtd_pr_adsp_err("%s [GET_DAP_PARAM] copy_to_user fail\n",__FUNCTION__);
                ret = -EFAULT;
            }
            kfree(paramValue);
            break;
        }
        case GET_SIF_DETECT_SOUND_SYSTEM_STD:
        {
            DTV_STATUS_T status;
            HAL_AUDIO_RHAL_CONFIG_T rhalconfig;
            HAL_AUDIO_SIF_SOUNDSYSTEM_T DetectSoundSystem;
            void *p_var0 = &DetectSoundSystem;
            HAL_AUDIO_SIF_STANDARD_T DetectSoundStd;
            void *p_var1 = &DetectSoundStd;
            UINT32 SignalQuality;
            void *p_var2 = &SignalQuality;

            if (copy_from_user(&rhalconfig, (const void __user *) (long)info.addr, info.size))
            {
                rtd_pr_adsp_err("%s [GET_SIF_DETECT_SOUND_SYSTEM_STD] info.addr copy_from_user fail\n",__FUNCTION__);
                return -EFAULT;
            }

            status = RHAL_AUDIO_SIF_DetectSoundSystemStd(rhalconfig.rhalinfo.setSoundSystem, rhalconfig.rhalinfo.bManualMode,
                p_var0, p_var1, p_var2);
            if(status != OK) {
                rtd_pr_adsp_err("RHAL_AUDIO_SIF_DetectSoundSystemStd return fail\n");
                return -EFAULT;
            }

            if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var0, rhalconfig.size[0])) {
                rtd_pr_adsp_err("%s [GET_SIF_DETECT_SOUND_SYSTEM_STD] copy_to_user addr[0] fail\n",__FUNCTION__);
                return -EFAULT;
            }

            if (copy_to_user((void __user *)(long)rhalconfig.addr[1], p_var1, rhalconfig.size[1])) {
                rtd_pr_adsp_err("%s [GET_SIF_DETECT_SOUND_SYSTEM_STD] copy_to_user addr[1] fail\n",__FUNCTION__);
                return -EFAULT;
            }

            if (copy_to_user((void __user *)(long)rhalconfig.addr[2], p_var2, rhalconfig.size[2])) {
                rtd_pr_adsp_err("%s [GET_SIF_DETECT_SOUND_SYSTEM_STD] copy_to_user addr[2] fail\n",__FUNCTION__);
                return -EFAULT;
            }
            break;
        }
        case GET_SIF_DETECT_SOUND_SYSTEM:
        {
            DTV_STATUS_T status;
            HAL_AUDIO_RHAL_CONFIG_T rhalconfig;
            HAL_AUDIO_SIF_SOUNDSYSTEM_T DetectSoundSystem;
            void *p_var0 = &DetectSoundSystem;
            UINT32 SignalQuality;
            void *p_var1 = &SignalQuality;
            if (copy_from_user(&rhalconfig, (const void __user *) (long)info.addr, info.size))
            {
                rtd_pr_adsp_err("%s [GET_SIF_DETECT_SOUND_SYSTEM] info.addr copy_from_user fail\n",__FUNCTION__);
                return -EFAULT;
            }

            status = RHAL_AUDIO_SIF_DetectSoundSystem(rhalconfig.rhalinfo.setSoundSystem, rhalconfig.rhalinfo.bManualMode,
                p_var0, p_var1);
            if(status != OK) {
                rtd_pr_adsp_err("RHAL_AUDIO_SIF_DetectSoundSystem return fail\n");
                return -EFAULT;
            }

            if (copy_to_user((void __user *)(long)rhalconfig.addr[0], p_var0, rhalconfig.size[0])) {
                rtd_pr_adsp_err("%s [GET_SIF_DETECT_SOUND_SYSTEM] copy_to_user addr[0] fail\n",__FUNCTION__);
                return -EFAULT;
            }

            if (copy_to_user((void __user *)(long)rhalconfig.addr[1], p_var1, rhalconfig.size[1])) {
                rtd_pr_adsp_err("%s [GET_SIF_DETECT_SOUND_SYSTEM] copy_to_user addr[1] fail\n",__FUNCTION__);
                return -EFAULT;
            }
            break;
        }
        case GET_HDMI_GET_COPY_INFO:
        {
            DTV_STATUS_T status;
            HAL_AUDIO_SPDIF_COPYRIGHT_T pCopyInfo;

            if (copy_from_user(&pCopyInfo, (const void __user *) (long)info.addr, info.size))
            {
                rtd_pr_adsp_err("%s [GET_HDMI_GET_COPY_INFO] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }

            status = RHAL_AUDIO_HDMI_GetCopyInfo(&pCopyInfo);
            if(status != OK) {
                rtd_pr_adsp_err("RHAL_AUDIO_HDMI_GetCopyInfo return fail\n");
                return -EFAULT;
            }

            if (copy_to_user((void __user *)(long)info.addr, &pCopyInfo, info.size)) {
                rtd_pr_adsp_err("%s [GET_HDMI_GET_COPY_INFO] copy_to_user fail\n",__FUNCTION__);
                return -EFAULT;
            }
            break;
        }
        case GET_PRIVATE_INFO:
        {
            unsigned long rpc_res;
            AUDIO_RPC_PRIVATEINFO_PARAMETERS parameter;
            AUDIO_RPC_PRIVATEINFO_RETURNVAL res;
            int i;

            if (copy_from_user(&parameter, (const void __user*)(long)info.addr,sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS)))  {
                rtd_pr_adsp_err("[GET_PRIVATE_INFO] copy_from_user fail\n");
                return -EFAULT;
            }

            rpc_res = RTKAUDIO_RPC_TOAGENT_PRIVATEINFO_SVC(&parameter, &res);
            if(rpc_res != S_OK) {
                rtd_pr_adsp_err("RTKAUDIO_RPC_TOAGENT_PRIVATEINFO_SVC return fail\n");
                return -EFAULT;
            }

            for(i=0;i<16;i++)
                parameter.privateInfo[i] = res.privateInfo[i];

            if (copy_to_user((void __user *)(long)info.addr, &parameter, info.size)) {
                rtd_pr_adsp_err("[GET_PRIVATE_INFO] copy_to_user fail\n");
                return -EFAULT;
            }
            break;
        }
        case GET_HDMI_AUDIO_MODE:
        {
            DTV_STATUS_T status;
            HAL_AUDIO_HDMI_TYPE_T HDMIMode;

            if (copy_from_user(&HDMIMode, (const void __user*) (long)info.addr,sizeof(HAL_AUDIO_HDMI_TYPE_T)))  {
                rtd_pr_adsp_err("[GET_HDMI_AUDIO_MODE] copy_from_user fail\n");
                return -EFAULT;
            }

            status = RHAL_AUDIO_HDMI_GetAudioMode(&HDMIMode);
            if(status != OK) {
                rtd_pr_adsp_err("RHAL_AUDIO_HDMI_GetAudioMode return fail\n");
                return -EFAULT;
            }

            if (copy_to_user((void __user *)(long)info.addr, &HDMIMode, info.size)) {
                rtd_pr_adsp_err("[GET_HDMI_AUDIO_MODE] copy_to_user fail\n");
                return -EFAULT;
            }
            break;
        }
		case VIRTUALX_CMD_INFO:
		{
			virtualx_cmd_info virx_cmd;
			void *vir_addr = NULL;
			void *unvir_addr = NULL;
			unsigned long phy_addr = 0;

            if (drv_debug)
                rtd_pr_adsp_debug("[Virtual X] %s\n", __func__);

			if (info.size != sizeof(virtualx_cmd_info))
			{
				ret = -EFAULT;
				break;
			}

			/* 64-bit User App may have problem */
			if (copy_from_user(&virx_cmd, (const void __user *) (long) info.addr, info.size))
			{
				ret = -EFAULT;
				break;
			}

			vir_addr = get_rpc_mem(&unvir_addr);
			if (!vir_addr) {
				rtd_pr_adsp_err("[Virtual X] %d alloc memory fail\n", __LINE__);
				return -ENOMEM;
			}
			phy_addr = (unsigned long) dvr_to_phys(vir_addr);

			/* set parameter to 0, for snapshot */
			memset(unvir_addr, 0, 4096);

			send_virtualx_rpc(unvir_addr, virx_cmd, phy_addr, 0);

			put_rpc_mem(vir_addr);

			/* 64-bit User App may have problem */
			if (copy_to_user((void __user *) (long) info.addr, &virx_cmd, info.size))
			{
				ret = -EFAULT;
			}

			break;
		}
		case INIT_VOICEAP_CMD_Q:
		{
			unsigned long q_head_phy;
			if (info.size != sizeof(long))
			{
				ret = -EFAULT;
				break;
			}

			if (copy_from_user(&q_head_phy, (const void __user *) (long) info.addr, info.size))
			{
				ret = -EFAULT;
				break;
			}
			rtd_pr_adsp_err("[INIT_VOICEAP_CMD_Q] success\n");

			cmd_queue_create(CMDQ_NUM);

			q_head_phy = (unsigned long)(cq->pkt_array_user) & 0x1fffffffUL;

			rtd_pr_adsp_err("q_head_phy 0x%lx\n", q_head_phy);

			if (copy_to_user((void __user *) (long) info.addr, &q_head_phy, info.size))
			{
				ret = -EFAULT;
			}
			break;
		}
		case 0xff:
		{
			long addr = 0;

			addr = rtkaudio_alloc(info.size, 1); // memory limit
			if (addr == -ENOMEM) {
				ret = -EFAULT;
			} else {
				if (copy_to_user((void __user *)(long) info.addr, &addr, sizeof(int)))
				{
					rtd_pr_adsp_err("[%s %d] get limited memory(%x) user copy failure\n", __func__, __LINE__, (unsigned int)addr);
					rtkaudio_free((unsigned int)addr);
					ret = -EFAULT;
				}
			}

			rtd_pr_adsp_info("[%s %d] get limited memory(%x)\n", __func__, __LINE__, (unsigned int)addr);
			break;
		}
#ifdef CONFIG_ARCH_RTK2851A
		case GET_OPTEE_CARVEOUT_MEM_INFO:
		{
			ret_info_t ret_info;
			unsigned long size, addr;
			rtd_pr_adsp_debug("GET_OPTEE_CARVEOUT_MEM_INFO\n");

			size = carvedout_buf_query(CARVEDOUT_AUDIO_RINGBUF, (void *)&addr);
			ret_info.private_info[0] = (unsigned int)addr;
			ret_info.private_info[1] = (unsigned int)size;

			if (copy_to_user((void __user *)(long) info.addr, &ret_info, sizeof(ret_info_t)))
			{
				ret = -EFAULT;
			}
			rtd_pr_adsp_debug("GET_OPTEE_CARVEOUT_MEM_INFO done\n");
			break;
		}
#endif
		case QUICKSHOW_CMD_INFO:
		{
			quickshow_cmd_info quickshow_cmd;
			if (info.size != sizeof(quickshow_cmd_info))
			{
				ret = -EFAULT;
				break;
			}

			/* 64-bit User App may have problem */
			if (copy_from_user(&quickshow_cmd, (const void __user *) (long) info.addr, info.size))
			{
				ret = -EFAULT;
				break;
			}

			ret = rtkaudio_get_audio_quick_show_init_table(&quickshow_cmd);

			if (copy_to_user((void __user *) (long) info.addr, &quickshow_cmd, info.size))
			{
				ret = -EFAULT;
			}
			break;
		}
		default:
			ret = -EFAULT;
		}
		break;
	}
	case RTKAUDIO_IOC_GET_ALL:
        if (drv_debug)
            rtd_pr_adsp_debug("RTKAUDIO_IOCTL_GET_ALL\n");
		break;
	case RTKAUDIO_IOC_FW_REMOTE_MALLOC_SUM:
	{
		struct ret_info *ret_s;
		struct ret_info __user *_ret_s = (struct ret_info *)arg;

        if (drv_debug)
            rtd_pr_adsp_debug("RTKAUDIO_IOC_FW_REMOTE_MALLOC_SUM\n");

		ret_s = kmalloc(sizeof(struct ret_info), GFP_KERNEL);

		if (ret_s == NULL) {
			rtd_pr_adsp_err("[rtkaudio] malloc memory failed, %d\n", __LINE__);
			break;
		}

		rtkaudio_fw_remote_malloc_summary(ret_s);

		if (copy_to_user(_ret_s, ret_s, sizeof(struct ret_info)))
			ret = -EFAULT;

		kfree(ret_s);
		break;
	}
	case RTKAUDIO_IOC_ALLOC:
        if (drv_debug)
            rtd_pr_adsp_debug("RTKAUDIO_IOCTL_ALLOC\n");
		ret = rtkaudio_alloc(arg, 0);
		break;
	case RTKAUDIO_IOC_FREE:
        if (drv_debug)
            rtd_pr_adsp_debug("RTKAUDIO_IOCTL_FREE\n");
		ret = rtkaudio_free(arg);
		break;
	case RTKAUDIO_IOC_ALLOC_SUM:
	{
		struct ret_info *ret_info;
		struct ret_info __user *_ret_info = (struct ret_info *)arg;

        if (drv_debug)
            rtd_pr_adsp_debug("RTKAUDIO_IOCTL_ALLOC_SUM\n");

		ret_info = kmalloc(sizeof(struct ret_info), GFP_KERNEL);
		if (ret_info == NULL) {
			rtd_pr_adsp_err("[rtkaudio] malloc memory failed, %d\n", __LINE__);
			break;
		}

		rtkaudio_user_alloc_summary(ret_info);

		if (copy_to_user(_ret_info, ret_info, sizeof(struct ret_info)))
			ret = -EFAULT;

		kfree(ret_info);
		break;
	}
	case RTKAUDIO_IOC_AUDIO_CONFIG:
	{
		AUDIO_CONFIG_COMMAND_RTKAUDIO info;
		AUDIO_CONFIG_COMMAND_RTKAUDIO __user *_info = (AUDIO_CONFIG_COMMAND_RTKAUDIO *) arg;

		if (copy_from_user(&info, _info, sizeof(info))) {
			ret = -EFAULT;
			return ret;
		}
		ret = rtkaudio_send_audio_config(&info);
		if (ret != S_OK) {
			ret = -EFAULT;
		}
		if (copy_to_user(_info, &info, sizeof(info))) {
			ret = -EFAULT;
		}
		break;
	}
	case RTKAUDIO_IOC_READ_REG:
	{
		AUDIO_REGISTER_ACCESS_T info;

		if (copy_from_user((void*)&info, (const void __user*)arg, sizeof(info))) {
			ret = -EFAULT;
			return ret;
		}

		ret = read_register_by_type(&info);
		break;
	}
	case RTKAUDIO_IOC_WRITE_REG:
	{
		AUDIO_REGISTER_ACCESS_T info;

		if (copy_from_user((void*)&info, (const void __user*)arg, sizeof(info))) {
			ret = -EFAULT;
			return ret;
		}

		write_register_by_type(&info);
		break;
	}
	case RTKAUDIO_IOC_OMX_CREATE_DECODER:
	{
		if (rtkaudio_dec_agent == -1) {
            if (drv_debug)
                rtd_pr_adsp_debug("[rtkaudio] RTKAUDIO_IOC_OMX_CREATE_DECODER\n");
			rtkaudio_create_decoder_agent(rtkaudio_platform_devs);
			mutex_init(&dec_usage_lock);
		}
		break;
	}
	case RTKAUDIO_IOC_OMX_GET_OFFLOAD_DECODER:
	{
        if (drv_debug)
            rtd_pr_adsp_debug("[rtkaudio] RTKAUDIO_IOC_OMX_GET_OFFLOAD_DECODER\n");
		mutex_lock(&dec_usage_lock);
		if ((!rtkaudio_dec_inuse) && (rtkaudio_dec_agent != -1)) {
			if (copy_to_user((void __user *)arg, (void *)&rtkaudio_dec_agent, sizeof(int))) {
				ret = -EFAULT;
			} else {
				rtkaudio_dec_inuse = true;
			}
		} else {
			rtd_pr_adsp_debug("[rtkaudio] omx decoder is using or did not create, create decoder through orignal flow.\n");
			ret = -EFAULT;
		}
		mutex_unlock(&dec_usage_lock);

        if (drv_debug)
            rtd_pr_adsp_debug("[rtkaudio] RTKAUDIO_IOC_OMX_GET_OFFLOAD_DECODER done\n");
		break;
	}
	case RTKAUDIO_IOC_OMX_RELEASE_OFFLOAD_DECODER:
	{
        if (drv_debug)
            rtd_pr_adsp_debug("[rtkaudio] RTKAUDIO_IOC_OMX_RELEASE_OFFLOAD_DECODER\n");

		mutex_lock(&dec_usage_lock);
		if (rtkaudio_dec_agent != -1) {
			rtkaudio_dec_inuse = false;
		} else {
			rtd_pr_adsp_debug("[rtkaudio] omx decoder did not create.\n");
		}
		mutex_unlock(&dec_usage_lock);

        if (drv_debug)
            rtd_pr_adsp_debug("[rtkaudio] RTKAUDIO_IOC_OMX_RELEASE_OFFLOAD_DECODER done\n");
		break;
	}
	case RTKAUDIO_IOC_GET_FW_CAPABILITY:
	{
		int caps;
		caps = rtkaudio_get_audio_caps();

		if (copy_to_user((void __user *)arg, (void *)&caps, sizeof(int))) {
			ret = -EFAULT;
		}
		break;
	}
	case RTKAUDIO_IOC_SET_DMX_MODE:
	{
		int dmx_mode = 0;
		AUDIO_CONFIG_COMMAND_RTKAUDIO info;
		memset(&info, 0, sizeof(info));

		if (copy_from_user(&dmx_mode, (const void __user*)arg, sizeof(dmx_mode))) {
			ret = -EFAULT;
			return ret;
		}

		rtd_pr_adsp_info("[rtkaudio] set downmix mode %d\n", dmx_mode);

		info.msg_id = AUDIO_CONFIG_CMD_DD_DOWNMIXMODE;
		info.value[1] = 0;
		info.value[2] = 0x00002379;

		if (dmx_mode == 1) {
			/* LORO */
			info.value[0] = MODE_STEREO;
		} else if (dmx_mode == 2) {
			/* ARIB */
			info.value[0] = MODE_ARIB;
		} else {
			/* default: LTRT */
			info.value[0] = MODE_DOLBY_SURROUND;
		}

		ret = rtkaudio_send_audio_config(&info);

		if (ret != S_OK) {
			ret = -EFAULT;
		}

		break;
	}
	default:
		rtd_pr_adsp_info("rtkaudio: unknown ioctl(0x%08x)\n", cmd);
		break;
	};

	return ret;
}

#ifdef CONFIG_ARM64
#ifdef CONFIG_COMPAT
long rtkaudio_ioctl_compat(struct file *filp, unsigned int cmd, unsigned long arg)
{
	long ret = 0;

    if (drv_debug)
        rtd_pr_adsp_debug("rtkaudio: receive ioctl(cmd:0x%08x, arg:0x%08lx)\n", cmd, arg);

	switch (cmd) {
	case RTKAUDIO_IOC_INIT:
		blocking_notifier_call_chain(&rtkaudio_chain_head,
			RTKAUDIO_RESET_PREPARE, NULL);
        if (drv_debug)
            rtd_pr_adsp_debug("RTKAUDIO_IOC_INIT\n");
		blocking_notifier_call_chain(&rtkaudio_chain_head,
			RTKAUDIO_RESET_DONE, NULL);
		break;
	case RTKAUDIO_IOC_DEINIT:
        if (drv_debug)
            rtd_pr_adsp_debug("RTKAUDIO_IOCTL_DEINIT\n");
		break;
	case RTKAUDIO_IOC_OPEN:
        if (drv_debug)
            rtd_pr_adsp_debug("RTKAUDIO_IOCTL_OPEN\n");
		break;
	case RTKAUDIO_IOC_CLOSE:
        if (drv_debug)
            rtd_pr_adsp_debug("RTKAUDIO_IOCTL_CLOSE\n");
		break;
	case RTKAUDIO_IOC_CONNECT:
        if (drv_debug)
            rtd_pr_adsp_debug("RTKAUDIO_IOCTL_CONNECT\n");
		break;
	case RTKAUDIO_IOC_SET:
	{
		struct cntl_info info;
        if (drv_debug)
            rtd_pr_adsp_debug("RTKAUDIO_IOCTL_SET\n");
		if (copy_from_user((void*)&info, (const void __user*)arg, sizeof(info))) {
			ret = -EFAULT;
			rtd_pr_adsp_err("%s copy_from_user fail for resource_type %d\n", __FUNCTION__, info.resource_type);
		}

		switch(info.resource_type) {
        case SET_RHAL_INFO:
        {
            DTV_STATUS_T status;
            HAL_AUDIO_RHAL_INFO_T rhalinfo;

            if (copy_from_user(&rhalinfo, (const void __user *) compat_ptr(info.addr), info.size))
            {
                rtd_pr_adsp_err("%s [SET_DECODER_INPUT_MUTE] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }

            switch(rhalinfo.type)
            {
                case SET_TP_OPEN:
                    status = RHAL_AUDIO_TP_Open(rhalinfo.tpIndex);
                    break;
                case SET_ADEC_OPEN:
                    status = RHAL_AUDIO_ADEC_Open(rhalinfo.adecindex);
                    break;
                case SET_ADC_OPEN:
                    status = RHAL_AUDIO_ADC_Open(rhalinfo.portNum);
                    break;
                case SET_HDMI_OPEN:
                    status = RHAL_AUDIO_HDMI_Open();
                    break;
                case SET_DP_OPEN:
                    status = RHAL_AUDIO_DP_Open();
                    break;
                case SET_AAD_OPEN:
                    status = RHAL_AUDIO_AAD_Open();
                    break;
                case SET_SNDOUT_OPEN:
                    status = RHAL_AUDIO_SNDOUT_Open(rhalinfo.soundOutType);
                    break;
                case SET_TP_CLOSE:
                    status = RHAL_AUDIO_TP_Close(rhalinfo.tpIndex);
                    break;
                case SET_ADEC_CLOSE:
                    status = RHAL_AUDIO_ADEC_Close(rhalinfo.adecindex);
                    break;
                case SET_ADC_CLOSE:
                    status = RHAL_AUDIO_ADC_Close(rhalinfo.portNum);
                    break;
                case SET_HDMI_CLOSE:
                    status = RHAL_AUDIO_HDMI_Close();
                    break;
                case SET_DP_CLOSE:
                    status = RHAL_AUDIO_DP_Close();
                    break;
                case SET_AAD_CLOSE:
                    status = RHAL_AUDIO_AAD_Close();
                    break;
                case SET_SNDOUT_CLOSE:
                    status = RHAL_AUDIO_SNDOUT_Close(rhalinfo.soundOutType);
                    break;
                case SET_SPDIFOUT_DELAYTIME:
                    status = RHAL_AUDIO_SetSPDIFOutDelayTime(rhalinfo.delayTime, rhalinfo.enable);
                    break;
                case SET_ARC_OUTPUT_TYPE:
                    status = RHAL_AUDIO_ARC_SetOutputType(rhalinfo.eARCMode, rhalinfo.enable);
                    break;
                case SET_START_DECODING:
                    status = RHAL_AUDIO_StartDecoding(rhalinfo.adecindex, rhalinfo.audiotype);
                    break;
                case SET_STOP_DECODING:
                    status = RHAL_AUDIO_StopDecoding(rhalinfo.adecindex);
                    break;
                case SET_ADC_CONNECT:
                    status = RHAL_AUDIO_ADC_Connect(rhalinfo.portNum);
                    break;
                case SET_ADC_DISCONNECT:
                    status = RHAL_AUDIO_ADC_Disconnect(rhalinfo.portNum);
                    break;
                case SET_TP_CONNECT:
                    status = RHAL_AUDIO_TP_Connect(rhalinfo.adecinfo.prevTPInputPort, rhalinfo.adecinfo.curTPInputPort);
                    break;
                case SET_TP_DISCONNECT:
                    status = RHAL_AUDIO_TP_Disconnect(rhalinfo.adecinfo.prevTPInputPort, rhalinfo.adecinfo.curTPInputPort);
                    break;
                case SET_ADEC_CONNECT:
                    status = RHAL_AUDIO_ADEC_Connect(rhalinfo.adecinfo.prevTPInputPort, rhalinfo.adecinfo.curTPInputPort);
                    break;
                case SET_ADEC_DISCONNECT:
                    status = RHAL_AUDIO_ADEC_Disconnect(rhalinfo.adecinfo.prevTPInputPort, rhalinfo.adecinfo.curTPInputPort);
                    break;
                case SET_AMIX_CONNECT:
                    status = RHAL_AUDIO_AMIX_Connect(rhalinfo.adecinfo.prevTPInputPort, rhalinfo.adecinfo.curTPInputPort);
                    break;
                case SET_AMIX_DISCONNECT:
                    status = RHAL_AUDIO_AMIX_Disconnect(rhalinfo.adecinfo.prevTPInputPort, rhalinfo.adecinfo.curTPInputPort);
                    break;
                case SET_SE_CONNECT:
                    status = RHAL_AUDIO_SE_Connect(rhalinfo.inputConnect);
                    break;
                case SET_SE_DISCONNECT:
                    status = RHAL_AUDIO_SE_Disconnect(rhalinfo.inputConnect);
                    break;
                case SET_SNDOUT_CONNECT:
                    status = RHAL_AUDIO_SNDOUT_Connect(rhalinfo.adecinfo.prevTPInputPort, rhalinfo.adecinfo.curTPInputPort);
                    break;
                case SET_SNDOUT_DISCONNECT:
                    status = RHAL_AUDIO_SNDOUT_Disconnect(rhalinfo.adecinfo.prevTPInputPort, rhalinfo.adecinfo.curTPInputPort);
                    break;
                case SET_PAUSE_DECODING:
                    status = RHAL_AUDIO_PauseAdec(rhalinfo.adecindex);
                    break;
                case SET_PAUSE_AND_STOP_DECODING:
                    status = RHAL_AUDIO_PauseAndStopFlowAdec(rhalinfo.adecindex);
                    break;
                case SET_RESUME_AND_RUN_DECODING:
                    status = RHAL_AUDIO_ResumeAndRunAdec(rhalinfo.adecindex);
                    break;
                case SET_RESUME_DECODING:
                    status = RHAL_AUDIO_ResumeAdec(rhalinfo.adecindex);
                    break;
                case SET_FLUSH_DECODING:
                    status = RHAL_AUDIO_FlushAdec(rhalinfo.adecindex);
                    break;
                case SET_MAIN_DECODER_OUTPUT:
                    status = RHAL_AUDIO_SetMainDecoderOutput(rhalinfo.adecindex);
                    break;
                case SET_MAIN_AUDIO_OUTPUT:
                    status = RHAL_AUDIO_SetMainAudioOutput(rhalinfo.audioIndex);
                    break;
                case SET_DECODER_INPUT_MUTE:
                    status = RHAL_AUDIO_SetDecoderInputMute(rhalinfo.adecindex, rhalinfo.enable);
                    break;
                case SET_DECODER_DELAY_TIME:
                    status = RHAL_AUDIO_SetDecoderDelayTime(rhalinfo.adecindex, rhalinfo.delayTime);
                    break;
                case SET_DECODER_INPUT_GAIN:
                    status = RHAL_AUDIO_SetDecoderInputGain(rhalinfo.adecindex, rhalinfo.volume);
                    break;
                case SET_AD_MAIN:
                    status = RHAL_AUDIO_TP_SetAudioDescriptionMain(rhalinfo.adecindex, rhalinfo.enable);
                    break;
                case SET_TRICK_MODE:
                    status = RHAL_AUDIO_SetTrickMode(rhalinfo.adecindex, rhalinfo.eTrickMode);
                    break;
                case SET_TRACK_MODE:
                    status = RHAL_AUDIO_setTrackMode(rhalinfo.eTrackMode);
                    break;
                case SET_ATMOS_ENCODE:
                    status = RHAL_AUDIO_ATMOS_EncodeOnOff(rhalinfo.enable);
                    break;
                case SET_SPKOUT_MUTE:
                    status = RHAL_AUDIO_SetSPKOutMute(rhalinfo.enable);
                    break;
                case SET_SPDIFOUT_MUTE:
                    status = RHAL_AUDIO_SetSPDIFOutMute(rhalinfo.enable);
                    break;
                case SET_HPOUT_MUTE:
                    status = RHAL_AUDIO_SetHPOutMute(rhalinfo.enable);
                    break;
                case SET_SIF_MODE_SETUP:
                    status = RHAL_AUDIO_SIF_SetModeSetup(rhalinfo.sifStandard);
                    break;
                case SET_SIF_BAND_SETUP:
                    if (rhalinfo.setSoundSystem == 0xFF)
                        status = RHAL_AUDIO_Init_ATV_Driver(rhalinfo.eSifType);
                    else
                        status = RHAL_AUDIO_SIF_SetBandSetup(rhalinfo.eSifType, rhalinfo.setSoundSystem);
                    break;
                case SET_SIF_INPUT_SOURCE:
                    status = RHAL_AUDIO_SIF_SetInputSource(rhalinfo.sifSource);
                    break;
                case SET_SIF_NO_SIGNAL_MUTE:
                    status = RHAL_AUDIO_SIF_NoSignalMute(rhalinfo.enable);
                    break;
                case SET_SIF_AUTO_CHANGE_SOUND_MODE:
                    status = RHAL_AUDIO_SIF_AutoChangeSoundMode(rhalinfo.enable);
                    break;
                case SET_SIF_FIRSTTIME_PLAYMONO:
                    status = RHAL_AUDIO_SIF_FirstTimePlayMono(rhalinfo.enable);
                    break;
                case SET_SIF_FW_PRIORITY:
                    status = RHAL_AUDIO_SIF_SetFwPriority(rhalinfo.u16var);
                    break;
                case SET_SIF_MIS_PRIORITY:
                    status = RHAL_Audio_SIF_SET_MTS_PRIORITY(rhalinfo.u16var);
                    break;
                case SET_SIF_HIGH_DEVMODE:
                    status = RHAL_AUDIO_SIF_SetHighDevMode(rhalinfo.enable);
                    break;
                case SET_SIF_USER_ANALOG_MODE:
                    status = RHAL_AUDIO_SIF_SetUserAnalogMode(rhalinfo.sifAudioMode);
                    break;
                case SET_SIF_A2_THRESHOLD_LEVEL:
                    status = RHAL_AUDIO_SIF_SetA2ThresholdLevel(rhalinfo.u16var);
                    break;
                case SET_SIF_NICAM_THRESHOLD_LEVEL:
                    status = RHAL_AUDIO_SIF_SetNicamThresholdLevel(rhalinfo.u16var);
                    break;
                case SET_SIF_AUDIO_EQ_MODE:
                    status = RHAL_AUDIO_SIF_SetAudioEQMode(rhalinfo.enable);
                    break;
                case SET_SIF_OVER_DEVIATION:
                    status = RHAL_AUDIO_SIF_SetOverDeviation(rhalinfo.overDeviation);
                    break;
                case SET_SIF_AUTO_CHANGE_SOUND_STD:
                    status = RHAL_AUDIO_SIF_AutoChangeSoundStd(rhalinfo.enable);
                    break;
                case SET_MIXER_INPUT_MUTE:
                    status = RHAL_AUDIO_SetMixerInputMute(rhalinfo.mixerIndex, rhalinfo.enable);
                    break;
                case SET_AC4_AUTO_PRESENTATION_AD_MIXING:
                    status = RHAL_AUDIO_AC4_SetAutoPresentationADMixing(rhalinfo.adecindex, rhalinfo.enable);
                    break;
                case SET_AC4_AUTO_PRESENTATION_AD_TYPE:
                    status = RHAL_AUDIO_AC4_SetAutoPresentationADType(rhalinfo.adecindex, rhalinfo.enADType);
                    break;
                case SET_AC4_AUTO_PRESENTATION_FIRST_LANGUAGE:
                    status = RHAL_AUDIO_AC4_SetAutoPresentationFirstLanguage(rhalinfo.adecindex, rhalinfo.enCodeType, rhalinfo.firstLang);
                    break;
                case SET_AC4_DIALOG_ENHANCEMENT_GAIN:
                    status = RHAL_AUDIO_AC4_SetDialogueEnhancementGain(rhalinfo.adecindex, rhalinfo.dialEnhanceGain);
                    break;
                case SET_HDMI_AUDIO_RETURN_CHANNEL:
                    status = RHAL_AUDIO_HDMI_SetAudioReturnChannel(rhalinfo.enable);
                    break;
                case SET_HDMI_AUDIO_ENHANCE_RETURN_CHANNEL:
                    status = RHAL_AUDIO_HDMI_SetEnhancedAudioReturnChannel(rhalinfo.enable);
                    break;
                case SET_DOLBY_DRCMODE:
                    status = RHAL_AUDIO_SetDolbyDRCMode(rhalinfo.adecindex, rhalinfo.drcMode);
                    break;
                case SET_DOLBY_DOWNMIXMODE:
                    status = RHAL_AUDIO_SetDownMixMode(rhalinfo.adecindex, rhalinfo.downmixMode);
                    break;
                case SET_DOLBY_DUALMONO_OUTPUTMODE:
                    status = RHAL_AUDIO_SetDualMonoOutMode(rhalinfo.adecindex, rhalinfo.outputMode);
                    break;
                case SET_HDMI_CONNECT:
                    status = RHAL_AUDIO_HDMI_Connect();
                    break;
                case SET_HDMI_DISCONNECT:
                    status = RHAL_AUDIO_HDMI_Disconnect();
                    break;
                case SET_DP_CONNECT:
                    status = RHAL_AUDIO_DP_Connect();
                    break;
                case SET_DP_DISCONNECT:
                    status = RHAL_AUDIO_DP_Disconnect();
                    break;
                case SET_AAD_CONNECT:
                    status = RHAL_AUDIO_AAD_Connect();
                    break;
                case SET_AAD_DISCONNECT:
                    status = RHAL_AUDIO_AAD_Disconnect();
                    break;
                case SET_HDMI_OPEN_PORT:
                    status = RHAL_AUDIO_HDMI_OpenPort(rhalinfo.hdmiIndex);
                    break;
                case SET_HDMI_CLOSE_PORT:
                    status = RHAL_AUDIO_HDMI_ClosePort(rhalinfo.hdmiIndex);
                    break;
                case SET_HDMI_CONNECT_PORT:
                    status = RHAL_AUDIO_HDMI_ConnectPort(rhalinfo.hdmiIndex);
                    break;
                case SET_HDMI_DISCONNECT_PORT:
                    status = RHAL_AUDIO_HDMI_DisconnectPort(rhalinfo.hdmiIndex);
                    break;
                case SET_GLOBAL_DECODER_GAIN:
                    status = RHAL_AUDIO_SetGlobalDecoderGain(rhalinfo.volume);
                    break;
                case SET_MIXER_INPUT_GAIN:
                    status = RHAL_AUDIO_SetMixerInputGain(rhalinfo.mixerIndex, rhalinfo.volume);
                    break;
                case SET_SPKOUT_VOLUME:
                    status = RHAL_AUDIO_SetSPKOutVolume(rhalinfo.volume);
                    break;
                case SET_SPDIFOUT_VOLUME:
                    status = SetSPDIFOutVolume(rhalinfo.volume);
                    break;
                case SET_HPOUT_VOLUME:
                    status = RHAL_AUDIO_SetHPOutVolume(rhalinfo.volume, rhalinfo.enable);
                    break;
                case SET_SCARTOUT_VOLUME:
                    status = RHAL_AUDIO_SetSCARTOutVolume(rhalinfo.volume, rhalinfo.enable);
                    break;
                case SET_AD_VOLUME:
                    status = RHAL_AUDIO_SetAudioDescriptionVolume(rhalinfo.adecindex, rhalinfo.volume);
                    break;
                case SET_SPKOUT_DELAY_TIME:
                    status = RHAL_AUDIO_SetSPKOutDelayTime(rhalinfo.delayTime, rhalinfo.enable);
                    break;
                case SET_HPOUT_DELAY_TIME:
                    status = RHAL_AUDIO_SetHPOutDelayTime(rhalinfo.delayTime, rhalinfo.enable);
                    break;
                case SET_SCARTOUT_DELAY_TIME:
                    status = RHAL_AUDIO_SetSCARTOutDelayTime(rhalinfo.delayTime, rhalinfo.enable);
                    break;
                case SET_SPDIFOUT_TYPE:
                    status = RHAL_AUDIO_SPDIF_SetOutputType(rhalinfo.eSPDIFMode, rhalinfo.enable);
                    break;
                case SET_SPDIF_COPY_INFO:
                    status = RHAL_AUDIO_SPDIF_SetCopyInfo(rhalinfo.copyInfo);
                    break;
                case SET_SPDIF_CATEGORY_CODE:
                    status = RHAL_AUDIO_SPDIF_SetCategoryCode(rhalinfo.u32var[0]);
                    break;
                case SET_PCM_VOLUME:
                    status = RHAL_AUDIO_PCM_SetVolume(rhalinfo.apcmIndex, rhalinfo.volume);
                    break;
                case SET_PCM_VOLUME_BYPIN:
                    status = RHAL_AUDIO_PCM_SetVolume_ByPin(rhalinfo.apcmIndex, rhalinfo.volume, rhalinfo.pinID);
                    break;
                case SET_PCM_MUTE_BYPIN:
                    status = RHAL_AUDIO_PCM_SetMute_ByPin(rhalinfo.enable, rhalinfo.pinID);
                    break;
                case SET_DBXTV_SOUND_EFFECT:
                    status = RHAL_AUDIO_DBXTV_SET_SOUND_EFFECT(rhalinfo.paramType, rhalinfo.u32var[0]);
                    break;
                case SET_INPUT_VOLUME:
                    status = RHAL_AUDIO_SetInputVolume(rhalinfo.eSource, rhalinfo.s32var);
                    break;
                case SET_VOLUME_WITH_FADE:
                    status = RHAL_AUDIO_SetVolumeWithFade(rhalinfo.volume, rhalinfo.u32var[0], rhalinfo.u32var[1]);
                    break;
                case SET_SPDIF_ENABLE:
                    status = RHAL_AUDIO_SPDIF_SetEnable(rhalinfo.enable);
                    break;
                case SET_DTS_DECODER_PARAM:
                    status = RHAL_AUDIO_SetDTSDecoderParam(rhalinfo.dts_paramType, rhalinfo.u32var[0]);
                    break;
                case SET_ANALOG_ADJ_DB:
                    status = RHAL_AUDIO_SetAnalogAdjDB(rhalinfo.area_type);
                    break;
                case SET_DOLBY_OTT_MODE:
                    status = RHAL_AUDIO_SetDolbyOTTMode(rhalinfo.u32var[0], rhalinfo.u32var[1]);
                    break;
                case SET_AC4_AUTO_PRESENTATION_SECOND_LANGUAGE:
                    status = RHAL_AUDIO_AC4_SetAutoPresentationSecondLanguage(rhalinfo.adecindex, rhalinfo.enCodeType, rhalinfo.u32var[0]);
                    break;
                case SET_AC4_AUTO_PRESENTATION_PRIORITIZE_AD_TYPE:
                    status = RHAL_AUDIO_AC4_SetAutoPresentationPrioritizeADType(rhalinfo.adecindex, rhalinfo.u32var[0]);
                    break;
                case SET_AC4_AUTO_PRESENTATION_GROUP_INDEX:
                    status = RHAL_AUDIO_AC4_SetPresentationGroupIndex(rhalinfo.adecindex, rhalinfo.u32var[0]);
                    break;
                case SET_SOUND_EFFECT_LEVEL:
                    status = RHAL_AUDIO_SoundEffect_Level(rhalinfo.u32var[0], rhalinfo.u32var[1], rhalinfo.u32var[2]);
                    break;
                case SET_LOW_LATENCY_MODE:
                    status = RHAL_AUDIO_SetLowLatencyMode(rhalinfo.enable);
                    break;
                case SET_AI_SOUND_ENABLE:
                    status = RHAL_AUDIO_AISound_Enable(rhalinfo.enable);
                    break;
                case SET_BT_MIC_ENABLE:
                    status = RHAL_AUDIO_BTMic_Enable(rhalinfo.enable);
                    break;
                case SET_AEC_REFERENCE_SOURCE:
                    status = RHAL_AUDIO_SetAECReference_Source(rhalinfo.aec_source);
                    break;
                default:
                    status = NOT_OK;
                    break;
            }
            if(status != OK)
                ret = -EFAULT;
            break;
        }
        case SET_CHANNEL_OUT_SWAP:
        {
            AUDIO_RPC_AO_CHANNEL_OPT_SWAP sel;
            unsigned long rpc_res;

            if (copy_from_user(&sel, (const void __user *) compat_ptr(info.addr), info.size))
            {
                rtd_pr_adsp_err("%s [SET_CHANNEL_OUT_SWAP] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }
            rpc_res = RTKAUDIO_RPC_TOAGENT_SET_CHANNEL_OUT_SWAP_SVC(&sel);
            if(rpc_res != S_OK)
                ret = -EFAULT;

            break;
        }
        case SET_SEND_SPECTRUM_DATA:
        {
            AUDIO_SPECTRUM_CFG cfg;
            unsigned long rpc_res;

            if (copy_from_user(&cfg, (const void __user *) compat_ptr(info.addr), info.size))
            {
                rtd_pr_adsp_err("%s [SEND_SPECTRUM_DATA] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }
            rpc_res = RTKAUDIO_RPC_TOAGENT_SET_SEND_SPECTRUM_DATA_SVC(&cfg);
            if(rpc_res != S_OK)
                ret = -EFAULT;

            break;
        }
        case SET_AUDIO_AUTH_KEY:
        {
            long customer_key;
            unsigned long rpc_res;

            if (copy_from_user(&customer_key, (const void __user *) compat_ptr(info.addr), info.size))
            {
                rtd_pr_adsp_err("%s [SET_AUDIO_AUTH_KEY] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }
            rpc_res = RTKAUDIO_RPC_TOAGENT_SET_AUTHORITY_KEY_SVC(&customer_key);
            if(rpc_res != S_OK)
                ret = -EFAULT;

            break;
        }
        case SET_MODEL_HASH:
        {
            AUDIO_DUMMY_DATA hash;
            unsigned long rpc_res;

            if (copy_from_user(&hash, (const void __user *) compat_ptr(info.addr), info.size))
            {
                rtd_pr_adsp_err("%s [SET_MODEL_HASH] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }
            rpc_res = RTKAUDIO_RPC_TOAGENT_SET_MODEL_HASH_SVC(&hash);
            if(rpc_res != S_OK)
                ret = -EFAULT;

            break;
        }
        case SET_AUDIO_CAPABILITY:
        {
            AUDIO_LICENSE_STATUS_CONFIG lcns;
            unsigned long rpc_res;

            if (copy_from_user(&lcns, (const void __user *) compat_ptr(info.addr), info.size))
            {
                rtd_pr_adsp_err("%s [SET_AUDIO_CAPABILITY] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }
            rpc_res = RTKAUDIO_RPC_TOAGENT_SET_AUDIO_CAPABILITY_SVC(&lcns);
            if(rpc_res != S_OK)
                ret = -EFAULT;

            break;
        }
        case SET_AVSYNC_OFFSET:
        {
            AUDIO_HDMI_OUT_VSDB_DATA data;
            unsigned long rpc_res;

            if (copy_from_user(&data, (const void __user *) compat_ptr(info.addr), info.size))
            {
                rtd_pr_adsp_err("%s [SET_AVSYNC_OFFSET] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }
            rpc_res = RTKAUDIO_RPC_TOAGENT_SET_AVSYNC_OFFSET_SVC(&data);
            if(rpc_res != S_OK)
                ret = -EFAULT;

            break;
        }
        case SET_INIT_RING_BUFFER_HEADER:
        {
            AUDIO_RPC_RINGBUFFER_HEADER header;
            unsigned long rpc_res;

            if (copy_from_user(&header, (const void __user *) compat_ptr(info.addr), info.size))
            {
                rtd_pr_adsp_err("%s [INIT_RING_BUFFER_HEADER] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }
            rpc_res = RTKAUDIO_RPC_TOAGENT_INIT_RING_BUFFER_HEADER_SVC(&header);
            if(rpc_res != S_OK)
                ret = -EFAULT;

            break;
        }
        case SET_SPDIFOUT_PIN_SRC:
        {
            AUDIO_RPC_SPDIFO_SOURCE source;
            unsigned long rpc_res;

            if (copy_from_user(&source, (const void __user *) compat_ptr(info.addr), info.size))
            {
                rtd_pr_adsp_err("%s [SET_SPDIFOUT_PIN_SRC] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }
            rpc_res = RTKAUDIO_RPC_TOAGENT_SET_SPDIF_OUT_PIN_SRC_SVC(&source);
            if(rpc_res != S_OK)
                ret = -EFAULT;

            break;
        }
        case SET_TRUVOLUME:
        {
            AUDIO_RPC_TRUVOLUME vol;
            unsigned long rpc_res;

            if (copy_from_user(&vol, (const void __user *) compat_ptr(info.addr), info.size))
            {
                rtd_pr_adsp_err("%s [SET_TRUVOLUME] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }
            rpc_res = RTKAUDIO_RPC_TOAGENT_SET_TRUVOLUME_SVC(&vol);
            if(rpc_res != S_OK)
                ret = -EFAULT;

            break;
        }
        case SET_SWP_SRS_TRUSURROUNDHD:
        {
            DTV_STATUS_T status;
            HAL_AUDIO_RHAL_CONFIG_T rhalconfig;
            HAL_APP_TSXT_CFG TSXT;

            if (copy_from_user(&rhalconfig, (const void __user *) compat_ptr(info.addr), info.size))
            {
                rtd_pr_adsp_err("%s [SET_TRUVOLUMEHD_PARAM] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }

            if (copy_from_user(&TSXT, (const void __user *) compat_ptr(rhalconfig.addr[0]), rhalconfig.size[0]))
            {
                rtd_pr_adsp_err("%s [SET_SWP_SRS_TRUSURROUNDHD] rhalconfig.addr[0] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
            }

            status = RHAL_AUDIO_SwpSetSRS_TrusurroundHD(rhalconfig.rhalinfo.u32var[0], &TSXT);
            if(status != OK) {
                rtd_pr_adsp_err("KADP_AUDIO_SwpSetSRS_TrusurroundHD return fail\n");
                ret = -EFAULT;
            }
            break;
        }
        case SET_VX_PARAM:
        {
            DTV_STATUS_T status;
            HAL_AUDIO_RHAL_CONFIG_T rhalconfig;
            void* pData;
            if (copy_from_user(&rhalconfig, (const void __user *) compat_ptr(info.addr), info.size))
            {
                rtd_pr_adsp_err("%s [SET_VX_PARAM] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }

            pData = kmalloc(rhalconfig.size[0], GFP_KERNEL);

            if (pData == NULL) {
                rtd_pr_adsp_err("[rtkaudio] malloc memory failed, %d\n", __LINE__);
                return -EFAULT;
            }

            if (copy_from_user(pData, (const void __user *) compat_ptr(rhalconfig.addr[0]), rhalconfig.size[0]))
            {
                rtd_pr_adsp_err("%s [SET_VX_PARAM] rhalconfig.addr[0] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
            }

            status = RHAL_AUDIO_VX_SetParameter(rhalconfig.rhalinfo.vx_paramtype, pData);
            if(status != OK) {
                rtd_pr_adsp_err("RHAL_AUDIO_VX_SetParameter return fail\n");
                ret = -EFAULT;
            }
            kfree(pData);
            break;
        }
        case SET_TRUVOLUMEHD_PARAM:
        {
            DTV_STATUS_T status;
            HAL_AUDIO_RHAL_CONFIG_T rhalconfig;
            void* pData;
            if (copy_from_user(&rhalconfig, (const void __user *) compat_ptr(info.addr), info.size))
            {
                rtd_pr_adsp_err("%s [SET_TRUVOLUMEHD_PARAM] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }

            pData = kmalloc(rhalconfig.size[0], GFP_KERNEL);

            if (pData == NULL) {
                rtd_pr_adsp_err("[rtkaudio] malloc memory failed, %d\n", __LINE__);
                return -EFAULT;
            }

            if (copy_from_user(pData, (const void __user *) compat_ptr(rhalconfig.addr[0]), rhalconfig.size[0]))
            {
                rtd_pr_adsp_err("%s [SET_TRUVOLUMEHD_PARAM] rhalconfig.addr[0] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
            }

            status = RHAL_AUDIO_TruvolumeHD_SetParameter(rhalconfig.rhalinfo.tvolhd_paramType, pData);
            if(status != OK) {
                rtd_pr_adsp_err("RHAL_AUDIO_TruvolumeHD_SetParameter return fail\n");
                ret = -EFAULT;
            }
            kfree(pData);
            break;
        }
        case SET_DBXTV_TABLE:
        {
            DTV_STATUS_T status;
            HAL_AUDIO_RHAL_CONFIG_T rhalconfig;
            UINT32* table;

            if (copy_from_user(&rhalconfig, (const void __user *) compat_ptr(info.addr), info.size))
            {
                rtd_pr_adsp_err("%s [SET_DBXTV_TABLE] info.addr copy_from_user fail\n",__FUNCTION__);
                return -EFAULT;
            }

            table = kmalloc(rhalconfig.size[0], GFP_KERNEL);
            if (table == NULL) {
                rtd_pr_adsp_err("[rtkaudio] malloc memory failed, %d\n", __LINE__);
                return -EFAULT;
            }

            if (copy_from_user(table, (const void __user *) compat_ptr(rhalconfig.addr[0]), rhalconfig.size[0]))
            {
                rtd_pr_adsp_err("%s [SET_DBXTV_TABLE] rhalconfig.addr[0] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
            }

            status = RHAL_AUDIO_DBXTV_SET_TABLE(table, rhalconfig.size[0]);
            if(status != OK) {
                rtd_pr_adsp_err("RHAL_AUDIO_DBXTV_SET_TABLE return fail\n");
                ret = -EFAULT;
            }
            kfree(table);
            break;
        }
        case SET_DAP_TUNING_DATA:
        {
            DTV_STATUS_T status;
            HAL_AUDIO_RHAL_CONFIG_T rhalconfig;
            void* pData;

            if (copy_from_user(&rhalconfig, (const void __user *) compat_ptr(info.addr), info.size))
            {
                rtd_pr_adsp_err("%s [SET_DAP_TUNING_DATA] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }

            pData = kmalloc(rhalconfig.size[0], GFP_KERNEL);

            if (pData == NULL) {
                rtd_pr_adsp_err("[rtkaudio] malloc memory failed, %d\n", __LINE__);
                return -EFAULT;
            }

            if (copy_from_user(pData, (const void __user *) compat_ptr(rhalconfig.addr[0]), rhalconfig.size[0]))
            {
                rtd_pr_adsp_err("%s [SET_DAP_TUNING_DATA] rhalconfig.addr[0] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
            }
            status = RHAL_AUDIO_SetDAPTuningData(pData, rhalconfig.size[0]);
            if(status != OK) {
                rtd_pr_adsp_err("RHAL_AUDIO_SetDAPTuningData return fail\n");
                ret = -EFAULT;
            }
            kfree(pData);
            break;
        }
		case VIRTUALX_CMD_INFO:
		{
			virtualx_cmd_info virx_cmd;
			void *vir_addr = NULL;
			void *unvir_addr = NULL;
			unsigned long phy_addr = 0;

            if (drv_debug)
                rtd_pr_adsp_debug("[Virtual X] %s\n", __func__);

			if (info.size != sizeof(virtualx_cmd_info))
			{
				ret = -EFAULT;
				break;
			}

			if (copy_from_user(&virx_cmd, (const void __user *) compat_ptr(info.addr), info.size))
			{
				ret = -EFAULT;
				break;
			}

			vir_addr = get_rpc_mem(&unvir_addr);
			if (!vir_addr) {
				rtd_pr_adsp_err("[Virtual X] %d alloc memory fail\n", __LINE__);
				return -ENOMEM;
			}
			phy_addr = (unsigned long) dvr_to_phys(vir_addr);

			/* set parameter to 0, for snapshot */
			memset(unvir_addr, 0, 4096);

			send_virtualx_rpc(unvir_addr, virx_cmd, phy_addr, 1);

			put_rpc_mem(vir_addr);

			if (copy_to_user((void __user *) compat_ptr(info.addr), &virx_cmd, info.size))
			{
				ret = -EFAULT;
			}
			break;
		}
		case ACMD_INFO:
		{
			omx_rpc_cmd_t omx_cmd;
			unsigned int rpc_res;
			if (copy_from_user(&omx_cmd, compat_ptr(info.addr),sizeof(omx_rpc_cmd_t)))
			{
				ret = -EFAULT;
				break;
			}
			switch(omx_cmd.cmd_type)
			{
				case ACMD_PRIVATE_INFO:
				{
					AUDIO_RPC_PRIVATEINFO_PARAMETERS parameter;
					AUDIO_RPC_PRIVATEINFO_RETURNVAL res;
					if (copy_from_user(&parameter, compat_ptr(omx_cmd.addr),sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS)))
					{
						ret = -EFAULT;
						break;
					}
					rpc_res = RTKAUDIO_RPC_TOAGENT_PRIVATEINFO_SVC(&parameter, &res);
					if(rpc_res != S_OK) rtd_pr_adsp_err("[ACMD_PRIVATE_INFO] RPC return != S_OK\n");
					if (copy_to_user(compat_ptr(omx_cmd.addr), &res, info.size))
						ret = -EFAULT;
				}
				break;

			default:
				ret = -EFAULT;
				break;
			}
			break;
		}
		case EQ_BASS_CMD_INFO:
		{
			eq_bass_cmd_info eq_bass_cmd;
			ENUM_AUDIO_EQ_BASS_MODE mode;
			ENUM_AUDIO_EQ_BASS_FILTER_TYPE filter_type;
			int band_idx, bOnOff, fc, bw, gain;

			if (info.size != sizeof(eq_bass_cmd_info))
			{
				rtd_pr_adsp_err("rtkaudio_ioctl_set %d, info.size:%d, eq_bass_cmd_info size:%d\n", __LINE__, info.size, sizeof(eq_bass_cmd_info));
				ret = -EFAULT;
				break;
			}

			if (copy_from_user(&eq_bass_cmd, (const void __user *) compat_ptr(info.addr), info.size))
			{
				ret = -EFAULT;
				break;
			}

			mode = (ENUM_AUDIO_EQ_BASS_MODE)eq_bass_cmd.data[0];

			switch(eq_bass_cmd.type)
			{
				case ENUM_EQ_BASS_CMD_TYPE_SETENABLE:
				{
					if (eq_bass_cmd.size != sizeof(int)*3)
					{
						rtd_pr_adsp_err("ENUM_EQ_BASS_CMD_TYPE_SETENABLE fail\n");
						return -EFAULT;
					}
					band_idx = eq_bass_cmd.data[1];
					bOnOff   = eq_bass_cmd.data[2];
					set_EQ_BASS_enable(mode, band_idx, bOnOff);
					break;
				}
				case ENUM_EQ_BASS_CMD_TYPE_SETPARAM:
				{
					if (eq_bass_cmd.size != sizeof(int)*6)
					{
						rtd_pr_adsp_err("ENUM_EQ_BASS_CMD_TYPE_SETPARAM fail\n");
						return -EFAULT;
					}
					band_idx = eq_bass_cmd.data[1];
					filter_type = (ENUM_AUDIO_EQ_BASS_FILTER_TYPE)eq_bass_cmd.data[2];
					fc = eq_bass_cmd.data[3];
					bw = eq_bass_cmd.data[4];
					gain =eq_bass_cmd.data[5];
					set_EQ_BASS_parameter(mode, band_idx, filter_type, fc, bw, gain);
					break;
				}
				case ENUM_EQ_BASS_CMD_TYPE_SET_AUDIO_DATA_PATH:
				{
					if (eq_bass_cmd.size != sizeof(int)*2)
					{
						rtd_pr_adsp_err("ENUM_EQ_BASS_CMD_TYPE_SET_AUDIO_DATA_PATH fail\n");
						return -EFAULT;
					}
					bOnOff = eq_bass_cmd.data[1];
					set_EQ_BASS_Audio_Data_Path_Enable(mode, bOnOff);
					break;
				}
			}

			if (copy_to_user((void __user *) compat_ptr(info.addr), &eq_bass_cmd, info.size))
			{
				ret = -EFAULT;
			}

			break;
		}

		default:
			ret = -EFAULT;
			rtd_pr_adsp_err("%s Unknown source type %d\n", __FUNCTION__, info.resource_type);
		}
		break;
	}
	case RTKAUDIO_IOC_GET:
	{
		struct cntl_info info;
        if (drv_debug)
            rtd_pr_adsp_debug("RTKAUDIO_IOCTL_GET\n");

		if (copy_from_user((void*)&info, (const void __user*)arg, sizeof(info))) {
			ret = -EFAULT;
		}

		switch(info.resource_type) {
        case GET_SIF_IS_EXIST:
        {
            BOOLEAN IsExist;
            HAL_AUDIO_RHAL_INFO_T rhalinfo;

            if (copy_from_user(&rhalinfo, (const void __user *) compat_ptr(info.addr), info.size))
            {
                rtd_pr_adsp_err("%s [GET_SIF_IS_EXIST] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }

            IsExist = RHAL_AUDIO_SIF_IsSIFExist();
            rhalinfo.IsExist = IsExist;

            if (copy_to_user((void __user *)compat_ptr(info.addr), &rhalinfo, info.size)) {
                rtd_pr_adsp_err("%s [GET_SIF_IS_EXIST] copy_to_user fail\n",__FUNCTION__);
                return -EFAULT;
            }
            break;
        }
        case GET_TP_IS_ES_EXIST:
        {
            BOOLEAN IsExist;
            HAL_AUDIO_RHAL_INFO_T rhalinfo;

            if (copy_from_user(&rhalinfo, (const void __user *) compat_ptr(info.addr), info.size))
            {
                rtd_pr_adsp_err("%s [GET_TP_IS_ES_EXIST] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }

            IsExist = RHAL_AUDIO_TP_IsESExist(rhalinfo.adecindex);
            rhalinfo.IsExist = IsExist;

            if (copy_to_user((void __user *)compat_ptr(info.addr), &rhalinfo, info.size)) {
                rtd_pr_adsp_err("%s [GET_TP_IS_ES_EXIST] copy_to_user fail\n",__FUNCTION__);
                return -EFAULT;
            }
            break;
        }
        case GET_RHAL_INFO:
        {
            DTV_STATUS_T status;
            HAL_AUDIO_RHAL_CONFIG_T rhalconfig;
            void *p_var;

            if (copy_from_user(&rhalconfig, (const void __user *) compat_ptr(info.addr), info.size))
            {
                rtd_pr_adsp_err("%s [GET_RHAL_INFO] info.addr copy_from_user fail\n",__FUNCTION__);
                return -EFAULT;
            }

            switch(rhalconfig.type)
            {
                case GET_TP_AUDIO_PTS:
                {
                    UINT32 pts;
                    p_var = &pts;

                    if (copy_from_user(p_var, (const void __user *) compat_ptr(rhalconfig.addr[0]), rhalconfig.size[0]))
                    {
                        rtd_pr_adsp_err("%s [GET_DAP_PARAM] rhalinfo.addr copy_from_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    status = RHAL_AUDIO_TP_GetAudioPTS(rhalconfig.rhalinfo.adecindex, p_var);

                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }

                    if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }

                    break;
                }
                case GET_TP_AUDIO_PTS64:
                {
                    UINT64 pts;
                    p_var = &pts;

                    if (copy_from_user(p_var, (const void __user *) compat_ptr(rhalconfig.addr[0]), rhalconfig.size[0]))
                    {
                        rtd_pr_adsp_err("%s [GET_DAP_PARAM] rhalinfo.addr copy_from_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    status = RHAL_AUDIO_TP_GetAudioPTS64(rhalconfig.rhalinfo.adecindex, p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }

                    if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }

                    break;
                }
                case GET_TP_BUFFER_STATUS:
                {
                    HAL_AUDIO_RHAL_BUFFER_INFO_T bufinfo;
                    p_var = &bufinfo;
                    if (copy_from_user(p_var, (const void __user *) compat_ptr(rhalconfig.addr[0]), rhalconfig.size[0]))
                    {
                        rtd_pr_adsp_err("%s [GET_DAP_PARAM] rhalinfo.addr copy_from_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    status = RHAL_AUDIO_TP_GetBufferStatus(rhalconfig.rhalinfo.adecindex, &bufinfo.MaxSize, &bufinfo.FreeSize);

                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }

                    if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }

                    break;
                }
                case GET_ES_INFO:
                {
                    HAL_AUDIO_ES_INFO_T ESInfo;
                    p_var = &ESInfo;

                    if (copy_from_user(&ESInfo, (const void __user *) compat_ptr(rhalconfig.addr[0]), rhalconfig.size[0]))
                    {
                        rtd_pr_adsp_err("%s [GET_ES_INFO] rhalinfo.addr copy_from_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    status = RHAL_AUDIO_GetESInfo(rhalconfig.rhalinfo.adecindex, &ESInfo);
                    if(status != OK) {
                        rtd_pr_adsp_err("RHAL_AUDIO_TP_GetESInfo return fail\n");
                        return -EFAULT;
                    }

                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }

                    if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_DECODER_INPUT_GAIN:
                {
                    HAL_AUDIO_VOLUME_T volinfo;
                    p_var = &volinfo;

                    if (copy_from_user(p_var, (const void __user *) compat_ptr(rhalconfig.addr[0]), rhalconfig.size[0]))
                    {
                        rtd_pr_adsp_err("%s [GET_DAP_PARAM] rhalinfo.addr copy_from_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    status = RHAL_AUDIO_GetDecoderInputGain(rhalconfig.rhalinfo.adecindex, p_var);


                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }

                    if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_TP_ES_INFO:
                {
                    HAL_AUDIO_ES_INFO_T ESInfo;
                    p_var = &ESInfo;

                    if (copy_from_user(p_var, (const void __user *) compat_ptr(rhalconfig.addr[0]), rhalconfig.size[0]))
                    {
                        rtd_pr_adsp_err("%s [GET_DAP_PARAM] rhalinfo.addr copy_from_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    status = RHAL_AUDIO_TP_GetESInfo(rhalconfig.rhalinfo.adecindex, p_var);


                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }

                    if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_DECODE_TYPE:
                {
                    HAL_AUDIO_SRC_TYPE_T SRCInfo;
                    p_var = &SRCInfo;

                    if (copy_from_user(p_var, (const void __user *) compat_ptr(rhalconfig.addr[0]), rhalconfig.size[0]))
                    {
                        rtd_pr_adsp_err("%s [GET_DAP_PARAM] rhalinfo.addr copy_from_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    status = RHAL_AUDIO_GetDecodingType(rhalconfig.rhalinfo.adecindex, p_var);

                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }

                    if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_ADEC_STATUS:
                {
                    HAL_AUDIO_ADEC_INFO_T AdecInfo;
                    p_var = &AdecInfo;

                    if (copy_from_user(p_var, (const void __user *) compat_ptr(rhalconfig.addr[0]), rhalconfig.size[0]))
                    {
                        rtd_pr_adsp_err("%s [GET_DAP_PARAM] rhalinfo.addr copy_from_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    status = RHAL_AUDIO_GetAdecStatus(rhalconfig.rhalinfo.adecindex, p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_SIF_DETECT_COMPLETE:
                {
                    SINT32 isOff;
                    p_var = &isOff;
                    status = RHAL_AUDIO_SIF_DetectComplete(p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_STATUS_INFO:
                {
                    HAL_AUDIO_COMMON_INFO_T AudioStatusInfo;
                    p_var = &AudioStatusInfo;
                    status = RHAL_AUDIO_GetStatusInfo(p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_SCARTOUT_MUTE_STATUS:
                {
                    BOOLEAN bOnOff;
                    p_var = &bOnOff;
                    status = RHAL_AUDIO_GetSCARTOutMuteStatus(p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_HPOUT_MUTE_STATUS:
                {
                    BOOLEAN bOnOff;
                    p_var = &bOnOff;
                    status = RHAL_AUDIO_GetHPOutMuteStatus(p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_SPDIFOUT_MUTE_STATUS:
                {
                    BOOLEAN bOnOff;
                    p_var = &bOnOff;
                    status = RHAL_AUDIO_GetSPDIFOutMuteStatus(p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_SPKOUT_MUTE_STATUS:
                {
                    BOOLEAN bOnOff;
                    p_var = &bOnOff;
                    status = RHAL_AUDIO_GetSPKOutMuteStatus(p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_DECODER_INPUT_MUTE:
                {
                    BOOLEAN bOnOff;
                    p_var = &bOnOff;
                    status = RHAL_AUDIO_GetDecoderInputMute(rhalconfig.rhalinfo.adecindex, p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_MIXER_INPUT_MUTE:
                {
                    BOOLEAN bOnOff;
                    p_var = &bOnOff;
                    status = RHAL_AUDIO_GetMixerInputMute(rhalconfig.rhalinfo.mixerIndex, p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_MIXER_INPUT_GAIN:
                {
                    HAL_AUDIO_VOLUME_T volume;
                    p_var = &volume;
                    status = RHAL_AUDIO_GetMixerInputGain(rhalconfig.rhalinfo.mixerIndex, p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_SNDOUT_LATENCY:
                {
                    int latency_ms;
                    p_var = &latency_ms;
                    status = RHAL_AUDIO_SNDOUT_GetLatency(rhalconfig.rhalinfo.soundOutType, p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_DIGITAL_INFO:
                {
                    HAL_AUDIO_DIGITAL_INFO_T AudioDigitalInfo;
                    p_var = &AudioDigitalInfo;
                    status = RHAL_AUDIO_GetDigitalInfo(rhalconfig.rhalinfo.adecindex, p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_SIF_SOUND_STANDARD:
                {
                    HAL_AUDIO_SIF_STANDARD_T standard;
                    p_var = &standard;
                    status = RHAL_AUDIO_SIF_GetSoundStandard(p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_SIF_CURRENT_ANALOG_MODE:
                {
                    HAL_AUDIO_SIF_MODE_GET_T SifAudioMode;
                    p_var = &SifAudioMode;
                    status = RHAL_AUDIO_SIF_GetCurAnalogMode(p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_SIF_NICAM_STABLE:
                {
                    UINT16 IsStable;
                    p_var = &IsStable;
                    status = RHAL_AUDIO_SIF_GetNicamStable(p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_SIF_NICAM_THRESHOLD_LEVEL:
                {
                    UINT16 Level;
                    p_var = &Level;
                    status = RHAL_AUDIO_SIF_GetNicamThresholdLevel(p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_SIF_A2_STEREO_LEVEL:
                {
                    UINT16 Level;
                    p_var = &Level;
                    status = RHAL_AUDIO_SIF_GetA2StereoLevel(p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_SIF_CHECK_A2DK:
                {
                    HAL_AUDIO_SIF_EXISTENCE_INFO_T Availability;
                    p_var = &Availability;
                    status = RHAL_AUDIO_SIF_CheckA2DK(rhalconfig.rhalinfo.sifStandard, p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_SIF_CHECK_AVAILABLE_SYSTEM:
                {
                    HAL_AUDIO_SIF_EXISTENCE_INFO_T Availability;
                    p_var = &Availability;
                    status = RHAL_AUDIO_SIF_CheckAvailableSystem(rhalconfig.rhalinfo.availStandard, p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_SIF_CHECK_NICAM_DIGIT:
                {
                    HAL_AUDIO_SIF_EXISTENCE_INFO_T IsNicamDetect;
                    p_var = &IsNicamDetect;
                    status = RHAL_AUDIO_SIF_CheckNicamDigital(p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_SIF_BAND_DETECT:
                {
                    UINT32 BandStrength;
                    p_var = &BandStrength;
                    status = RHAL_AUDIO_SIF_GetBandDetect(rhalconfig.rhalinfo.setSoundSystem, p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                case GET_SIF_SOUND_INFO:
                {
                    HAL_ATV_SOUND_INFO_T SoundInfo;
                    p_var = &SoundInfo;
                    status = RHAL_AUDIO_SIF_GetSoundInfo(p_var);
                    if(status != OK) {
                        rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                        return -EFAULT;
                    }
                    if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var, rhalconfig.size[0])) {
                        rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                        return -EFAULT;
                    }
                    break;
                }
                default:
                    status = NOT_OK;
                    rtd_pr_adsp_err("GET_RHAL_INFO fail in unknown type = %d\n", rhalconfig.type);
                    return -EFAULT;
            }
#if 0 //p_var points to local variable
            if(status != OK) {
                rtd_pr_adsp_err("GET_RHAL_INFO return fail in type = %d\n", rhalconfig.type);
                return -EFAULT;
            }

            if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var, rhalconfig.size[0])) {
                rtd_pr_adsp_err("%s [GET_RHAL_INFO] copy_to_user fail\n",__FUNCTION__);
                return -EFAULT;
            }
#endif
            break;
        }
        case GET_DBXTV_DEBUG_CMD:
        {
            DTV_STATUS_T status;
            HAL_AUDIO_RHAL_CONFIG_T rhalconfig;
            UINT32 *param;

            if (copy_from_user(&rhalconfig, (const void __user *) compat_ptr(info.addr), info.size))
            {
                rtd_pr_adsp_err("%s [GET_DBXTV_DEBUG_CMD] copy_from_user info.addr fail\n",__FUNCTION__);
                return -EFAULT;
            }

            param = kmalloc(rhalconfig.size[0], GFP_KERNEL);

            if (param == NULL) {
                rtd_pr_adsp_err("[GET_DBXTV_DEBUG_CMD] malloc memory failed, %d\n", __LINE__);
                return -EFAULT;
            }

            if (copy_from_user(param, (const void __user *) compat_ptr(rhalconfig.addr[0]), rhalconfig.size[0]))
            {
                rtd_pr_adsp_err("%s [GET_DBXTV_DEBUG_CMD] copy_from_user addr[0] fail\n",__FUNCTION__);
                ret = -EFAULT;
            }

            status = RHAL_AUDIO_DBXTV_DEBUG_CMD(rhalconfig.rhalinfo.u32var[0], param, rhalconfig.size[0]);
            if(status != OK) {
                rtd_pr_adsp_err("RHAL_AUDIO_DBXTV_DEBUG_CMD return fail\n");
                ret = -EFAULT;
            }

            if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), param, rhalconfig.size[0])) {
                rtd_pr_adsp_err("%s [GET_DBXTV_DEBUG_CMD] copy_to_user fail\n",__FUNCTION__);
                ret = -EFAULT;
            }
            kfree(param);
            break;
        }
        case GET_VX_PARAM:
        {
            DTV_STATUS_T status;
            HAL_AUDIO_RHAL_CONFIG_T rhalconfig;
            void* pData;
            if (copy_from_user(&rhalconfig, (const void __user *) compat_ptr(info.addr), info.size))
            {
                rtd_pr_adsp_err("%s [GET_VX_PARAM] copy_from_user info.addr fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }

            pData = kmalloc(rhalconfig.size[0], GFP_KERNEL);

            if (pData == NULL) {
                rtd_pr_adsp_err("[GET_VX_PARAM] malloc memory failed, %d\n", __LINE__);
                return -EFAULT;
            }

            if (copy_from_user(pData, (const void __user *) compat_ptr(rhalconfig.addr[0]), rhalconfig.size[0]))
            {
                rtd_pr_adsp_err("%s [GET_VX_PARAM] copy_from_user addr[0] fail\n",__FUNCTION__);
                ret = -EFAULT;
            }

            status = RHAL_AUDIO_VX_GetParameter(rhalconfig.rhalinfo.vx_paramtype, pData);
            if(status != OK) {
                rtd_pr_adsp_err("RHAL_AUDIO_VX_GetParameter return fail\n");
                ret = -EFAULT;
            }

            if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), pData, rhalconfig.size[0])) {
                  rtd_pr_adsp_err("%s [GET_VX_PARAM] copy_to_user fail\n",__FUNCTION__);
                  ret = -EFAULT;
            }
            kfree(pData);
            break;
        }
        case GET_TRUVOLUMEHD_PARAM:
        {
            DTV_STATUS_T status;
            HAL_AUDIO_RHAL_CONFIG_T rhalconfig;
            void* pData;
            if (copy_from_user(&rhalconfig, (const void __user *) compat_ptr(info.addr), info.size))
            {
                rtd_pr_adsp_err("%s [GET_TRUVOLUMEHD_PARAM] copy_from_user info.addr fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }

            pData = kmalloc(rhalconfig.size[0], GFP_KERNEL);

            if (pData == NULL) {
                rtd_pr_adsp_err("[GET_TRUVOLUMEHD_PARAM] malloc memory failed, %d\n", __LINE__);
                return -EFAULT;
            }

            if (copy_from_user(pData, (const void __user *) compat_ptr(rhalconfig.addr[0]), rhalconfig.size[0]))
            {
                rtd_pr_adsp_err("%s [GET_TRUVOLUMEHD_PARAM] copy_from_user addr[0] fail\n",__FUNCTION__);
                ret = -EFAULT;
            }

            status = RHAL_AUDIO_TruvolumeHD_GetParameter(rhalconfig.rhalinfo.tvolhd_paramType, pData);
            if(status != OK) {
                rtd_pr_adsp_err("RHAL_AUDIO_TruvolumeHD_GetParameter return fail\n");
                ret = -EFAULT;
            }

            if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), pData, rhalconfig.size[0])) {
                  rtd_pr_adsp_err("%s [GET_TRUVOLUMEHD_PARAM] copy_to_user fail\n",__FUNCTION__);
                  ret = -EFAULT;
            }
            kfree(pData);
            break;
        }
        case GET_DAP_PARAM:
        {
            DTV_STATUS_T status;
            HAL_AUDIO_RHAL_CONFIG_T rhalconfig;
            void *paramValue;

            if (copy_from_user(&rhalconfig, (const void __user *) compat_ptr(info.addr), info.size))
            {
                rtd_pr_adsp_err("%s [GET_DAP_PARAM] info.addr copy_from_user fail\n",__FUNCTION__);
                return -EFAULT;
            }

            paramValue = kmalloc(rhalconfig.size[0], GFP_KERNEL);

            if (paramValue == NULL) {
                rtd_pr_adsp_err("[GET_DAP_PARAM] malloc memory failed, %d\n", __LINE__);
                return -EFAULT;
            }

            if (copy_from_user(paramValue, (const void __user *) compat_ptr(rhalconfig.addr[0]), rhalconfig.size[0]))
            {
                rtd_pr_adsp_err("%s [GET_DAP_PARAM] rhalinfo.addr copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
            }

            status = RHAL_AUDIO_GetDAPParam(rhalconfig.rhalinfo.dap_paramType, paramValue, rhalconfig.size[0]);
            if(status != OK) {
                rtd_pr_adsp_err("RHAL_AUDIO_GetDAPParam return fail\n");
                ret = -EFAULT;
            }

            if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), paramValue, rhalconfig.size[0])) {
                rtd_pr_adsp_err("%s [GET_DAP_PARAM] copy_to_user fail\n",__FUNCTION__);
                ret = -EFAULT;
            }
            kfree(paramValue);
            break;
        }
        case GET_SIF_DETECT_SOUND_SYSTEM_STD:
        {
            DTV_STATUS_T status;
            HAL_AUDIO_RHAL_CONFIG_T rhalconfig;
            HAL_AUDIO_SIF_SOUNDSYSTEM_T DetectSoundSystem;
            void *p_var0 = &DetectSoundSystem;
            HAL_AUDIO_SIF_STANDARD_T DetectSoundStd;
            void *p_var1 = &DetectSoundStd;
            UINT32 SignalQuality;
            void *p_var2 = &SignalQuality;

            if (copy_from_user(&rhalconfig, (const void __user *) compat_ptr(info.addr), info.size))
            {
                rtd_pr_adsp_err("%s [GET_SIF_DETECT_SOUND_SYSTEM_STD] info.addr copy_from_user fail\n",__FUNCTION__);
                return -EFAULT;
            }

            status = RHAL_AUDIO_SIF_DetectSoundSystemStd(rhalconfig.rhalinfo.setSoundSystem, rhalconfig.rhalinfo.bManualMode,
            p_var0, p_var1, p_var2);
            if(status != OK) {
                rtd_pr_adsp_err("RHAL_AUDIO_SIF_DetectSoundSystemStd return fail\n");
                return -EFAULT;
            }

            if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var0, rhalconfig.size[0])) {
                rtd_pr_adsp_err("%s [GET_SIF_DETECT_SOUND_SYSTEM_STD] copy_to_user addr[0] fail\n",__FUNCTION__);
                return -EFAULT;
            }

            if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[1]), p_var1, rhalconfig.size[1])) {
                rtd_pr_adsp_err("%s [GET_SIF_DETECT_SOUND_SYSTEM_STD] copy_to_user addr[1] fail\n",__FUNCTION__);
                return -EFAULT;
            }

            if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[2]), p_var2, rhalconfig.size[2])) {
                rtd_pr_adsp_err("%s [GET_SIF_DETECT_SOUND_SYSTEM_STD] copy_to_user addr[2] fail\n",__FUNCTION__);
                return -EFAULT;
            }
            break;
        }
        case GET_SIF_DETECT_SOUND_SYSTEM:
        {
            DTV_STATUS_T status;
            HAL_AUDIO_RHAL_CONFIG_T rhalconfig;
            HAL_AUDIO_SIF_SOUNDSYSTEM_T DetectSoundSystem;
            void *p_var0 = &DetectSoundSystem;
            UINT32 SignalQuality;
            void *p_var1 = &SignalQuality;
            if (copy_from_user(&rhalconfig, (const void __user *) compat_ptr(info.addr), info.size))
            {
                rtd_pr_adsp_err("%s [GET_SIF_DETECT_SOUND_SYSTEM] info.addr copy_from_user fail\n",__FUNCTION__);
                return -EFAULT;
            }

            status = RHAL_AUDIO_SIF_DetectSoundSystem(rhalconfig.rhalinfo.setSoundSystem, rhalconfig.rhalinfo.bManualMode,
            p_var0, p_var1);
            if(status != OK) {
                rtd_pr_adsp_err("RHAL_AUDIO_SIF_DetectSoundSystem return fail\n");
                return -EFAULT;
            }

            if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[0]), p_var0, rhalconfig.size[0])) {
                rtd_pr_adsp_err("%s [GET_SIF_DETECT_SOUND_SYSTEM] copy_to_user addr[0] fail\n",__FUNCTION__);
                return -EFAULT;
            }

            if (copy_to_user((void __user *)compat_ptr(rhalconfig.addr[1]), p_var1, rhalconfig.size[1])) {
                rtd_pr_adsp_err("%s [GET_SIF_DETECT_SOUND_SYSTEM] copy_to_user addr[1] fail\n",__FUNCTION__);
                return -EFAULT;
            }
            break;
        }
        case GET_HDMI_GET_COPY_INFO:
        {
            DTV_STATUS_T status;
            HAL_AUDIO_SPDIF_COPYRIGHT_T pCopyInfo;

            if (copy_from_user(&pCopyInfo, (const void __user *) compat_ptr(info.addr), info.size))
            {
                rtd_pr_adsp_err("%s [GET_HDMI_GET_COPY_INFO] copy_from_user fail\n",__FUNCTION__);
                ret = -EFAULT;
                break;
            }

            status = RHAL_AUDIO_HDMI_GetCopyInfo(&pCopyInfo);
            if(status != OK) {
                rtd_pr_adsp_err("RHAL_AUDIO_HDMI_GetCopyInfo return fail\n");
                return -EFAULT;
            }

            if (copy_to_user((void __user *)compat_ptr(info.addr), &pCopyInfo, info.size)) {
                rtd_pr_adsp_err("%s [GET_HDMI_GET_COPY_INFO] copy_to_user fail\n",__FUNCTION__);
                return -EFAULT;
            }
            break;
        }
        case GET_PRIVATE_INFO:
        {
            unsigned long rpc_res;
            AUDIO_RPC_PRIVATEINFO_PARAMETERS parameter;
            AUDIO_RPC_PRIVATEINFO_RETURNVAL res;
            int i;

            if (copy_from_user(&parameter, (const void __user*)compat_ptr(info.addr),sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS)))  {
                rtd_pr_adsp_err("[GET_PRIVATE_INFO] copy_from_user fail\n");
                return -EFAULT;
            }

            rpc_res = RTKAUDIO_RPC_TOAGENT_PRIVATEINFO_SVC(&parameter, &res);
            if(rpc_res != S_OK) {
                rtd_pr_adsp_err("RTKAUDIO_RPC_TOAGENT_PRIVATEINFO_SVC return fail\n");
                return -EFAULT;
            }

            for(i=0;i<16;i++)
                parameter.privateInfo[i] = res.privateInfo[i];

            if (copy_to_user((void __user *)compat_ptr(info.addr), &parameter, info.size)) {
                rtd_pr_adsp_err("[GET_PRIVATE_INFO] copy_to_user fail\n");
                return -EFAULT;
            }
            break;
        }
        case GET_HDMI_AUDIO_MODE:
        {
            DTV_STATUS_T status;
            HAL_AUDIO_HDMI_TYPE_T HDMIMode;

            if (copy_from_user(&HDMIMode, (const void __user*) compat_ptr(info.addr),sizeof(HAL_AUDIO_HDMI_TYPE_T)))  {
                rtd_pr_adsp_err("[GET_HDMI_AUDIO_MODE] copy_from_user fail\n");
                return -EFAULT;
            }

            status = RHAL_AUDIO_HDMI_GetAudioMode(&HDMIMode);
            if(status != OK) {
                rtd_pr_adsp_err("RHAL_AUDIO_HDMI_GetAudioMode return fail\n");
                return -EFAULT;
            }

            if (copy_to_user((void __user *)compat_ptr(info.addr), &HDMIMode, info.size)) {
                rtd_pr_adsp_err("[GET_HDMI_AUDIO_MODE] copy_to_user fail\n");
                return -EFAULT;
            }
            break;
        }
		case VIRTUALX_CMD_INFO:
		{
			virtualx_cmd_info virx_cmd;
			void *vir_addr = NULL;
			void *unvir_addr = NULL;
			unsigned long phy_addr = 0;

            if (drv_debug)
                rtd_pr_adsp_debug("[Virtual X] %s\n", __func__);

			if (info.size != sizeof(virtualx_cmd_info))
			{
				ret = -EFAULT;
				break;
			}

			if (copy_from_user(&virx_cmd, (const void __user *) compat_ptr(info.addr), info.size))
			{
				ret = -EFAULT;
				break;
			}

			vir_addr = get_rpc_mem(&unvir_addr);
			if (!vir_addr) {
				rtd_pr_adsp_err("[Virtual X] %d alloc memory fail\n", __LINE__);
				return -ENOMEM;
			}
			phy_addr = (unsigned long) dvr_to_phys(vir_addr);

			/* set parameter to 0, for snapshot */
			memset(unvir_addr, 0, 4096);

			send_virtualx_rpc(unvir_addr, virx_cmd, phy_addr, 0);

			put_rpc_mem(vir_addr);

			if (copy_to_user((void __user *) compat_ptr(info.addr), &virx_cmd, info.size))
			{
				ret = -EFAULT;
			}

			break;
			}
		case 0xff:
		{
			int32_t addr = 0;

			addr = rtkaudio_alloc(info.size, 1); // memory limit
			if (addr == -ENOMEM) {
				ret = -EFAULT;
			} else {
				if (copy_to_user((void __user *) compat_ptr(info.addr), &addr, sizeof(int32_t)))
				{
					rtd_pr_adsp_err("[%s %d] get limited memory(%x) user copy failure\n", __func__, __LINE__, (unsigned int)addr);
					rtkaudio_free((unsigned int)addr);
					ret = -EFAULT;
				}
			}

			rtd_pr_adsp_info("[%s %d] get limited memory(%x)\n", __func__, __LINE__, (unsigned int)addr);
			break;
		}
#ifdef CONFIG_ARCH_RTK2851A
		case GET_OPTEE_CARVEOUT_MEM_INFO:
		{
			ret_info_t ret_info;
			unsigned long size, addr;
			rtd_pr_adsp_debug("GET_OPTEE_CARVEOUT_MEM_INFO\n");

			size = carvedout_buf_query(CARVEDOUT_AUDIO_RINGBUF, (void *)&addr);
			ret_info.private_info[0] = (unsigned int)addr;
			ret_info.private_info[1] = (unsigned int)size;

			if (copy_to_user((void __user *)(long) info.addr, &ret_info, sizeof(ret_info_t)))
			{
				ret = -EFAULT;
			}
			rtd_pr_adsp_debug("GET_OPTEE_CARVEOUT_MEM_INFO done\n");
			break;
		}
#endif
		case QUICKSHOW_CMD_INFO:
		{
			quickshow_cmd_info quickshow_cmd;
			if (info.size != sizeof(quickshow_cmd_info))
			{
				ret = -EFAULT;
				break;
			}

			/* 64-bit User App may have problem */
			if (copy_from_user(&quickshow_cmd, (const void __user *) compat_ptr(info.addr), info.size))
			{
				ret = -EFAULT;
				break;
			}

			ret = rtkaudio_get_audio_quick_show_init_table(&quickshow_cmd);

			if (copy_to_user((void __user *) compat_ptr(info.addr), &quickshow_cmd, info.size))
			{
				ret = -EFAULT;
			}
			break;
		}
		default:
			ret = -EFAULT;
		}
		break;
	}
	case RTKAUDIO_IOC_GET_ALL:
        if (drv_debug)
            rtd_pr_adsp_debug("RTKAUDIO_IOCTL_GET_ALL\n");
		break;
	case RTKAUDIO_IOC_FW_REMOTE_MALLOC_SUM:
	{
		struct ret_info *ret_s;
		struct ret_info __user *_ret_s = (struct ret_info *)arg;

        if (drv_debug)
            rtd_pr_adsp_debug("RTKAUDIO_IOC_FW_REMOTE_MALLOC_SUM\n");

		ret_s = kmalloc(sizeof(struct ret_info), GFP_KERNEL);

		if (ret_s == NULL) {
			rtd_pr_adsp_err("[rtkaudio] malloc memory failed, %d\n", __LINE__);
			break;
		}

		rtkaudio_fw_remote_malloc_summary(ret_s);

		if (copy_to_user(_ret_s, ret_s, sizeof(struct ret_info)))
			ret = -EFAULT;

		kfree(ret_s);
		break;
	}
	case RTKAUDIO_IOC_ALLOC:
        if (drv_debug)
            rtd_pr_adsp_debug("RTKAUDIO_IOCTL_ALLOC\n");
		ret = rtkaudio_alloc(arg, 0);
		break;
	case RTKAUDIO_IOC_FREE:
        if (drv_debug)
            rtd_pr_adsp_debug("RTKAUDIO_IOCTL_FREE\n");
		ret = rtkaudio_free(arg);
		break;
	case RTKAUDIO_IOC_ALLOC_SUM:
	{
		struct ret_info *ret_info;
		struct ret_info __user *_ret_info = (struct ret_info *)arg;

        if (drv_debug)
            rtd_pr_adsp_debug("RTKAUDIO_IOCTL_ALLOC_SUM\n");

		ret_info = kmalloc(sizeof(struct ret_info), GFP_KERNEL);
		if (ret_info == NULL) {
			rtd_pr_adsp_err("[rtkaudio] malloc memory failed, %d\n", __LINE__);
			break;
		}

		rtkaudio_user_alloc_summary(ret_info);

		if (copy_to_user(_ret_info, ret_info, sizeof(struct ret_info)))
			ret = -EFAULT;

		kfree(ret_info);
		break;
	}
	case RTKAUDIO_IOC_AUDIO_CONFIG:
	{
		AUDIO_CONFIG_COMMAND_RTKAUDIO info;
		AUDIO_CONFIG_COMMAND_RTKAUDIO __user *_info = (AUDIO_CONFIG_COMMAND_RTKAUDIO *) arg;

		if (copy_from_user(&info, _info, sizeof(info))) {
			ret = -EFAULT;
			return ret;
		}
		ret = rtkaudio_send_audio_config(&info);
		if (ret != S_OK) {
			ret = -EFAULT;
		}
		if (copy_to_user(_info, &info, sizeof(info))) {
			ret = -EFAULT;
		}
		break;
	}
	case RTKAUDIO_IOC_READ_REG:
	{
		AUDIO_REGISTER_ACCESS_T info;

		if (copy_from_user((void*)&info, (const void __user*) compat_ptr(arg), sizeof(info))) {
			ret = -EFAULT;
			return ret;
		}

		ret = read_register_by_type(&info);
		break;
	}
	case RTKAUDIO_IOC_WRITE_REG:
	{
		AUDIO_REGISTER_ACCESS_T info;

		if (copy_from_user((void*)&info, (const void __user*) compat_ptr(arg), sizeof(info))) {
			ret = -EFAULT;
			return ret;
		}

		write_register_by_type(&info);
		break;
	}
	case RTKAUDIO_IOC_OMX_CREATE_DECODER:
	{
		if (rtkaudio_dec_agent == -1) {
            if (drv_debug)
                rtd_pr_adsp_debug("[rtkaudio] RTKAUDIO_IOC_OMX_CREATE_DECODER\n");
			rtkaudio_create_decoder_agent(rtkaudio_platform_devs);
			mutex_init(&dec_usage_lock);
		}
		break;
	}
	case RTKAUDIO_IOC_OMX_GET_OFFLOAD_DECODER:
	{
        if (drv_debug)
            rtd_pr_adsp_debug("[rtkaudio] RTKAUDIO_IOC_OMX_GET_OFFLOAD_DECODER\n");

		mutex_lock(&dec_usage_lock);
		if ((!rtkaudio_dec_inuse) && (rtkaudio_dec_agent != -1)){
			if (copy_to_user((void __user *) compat_ptr(arg), (void *)&rtkaudio_dec_agent, sizeof(int))) {
				ret = -EFAULT;
			} else {
				rtkaudio_dec_inuse = true;
			}
		} else {
			rtd_pr_adsp_debug("[rtkaudio] omx decoder is using or did not create, create decoder through orignal flow.\n");
			ret = -EFAULT;
		}
		mutex_unlock(&dec_usage_lock);

        if (drv_debug)
            rtd_pr_adsp_debug("[rtkaudio] RTKAUDIO_IOC_OMX_GET_OFFLOAD_DECODER done\n");
		break;
	}
	case RTKAUDIO_IOC_OMX_RELEASE_OFFLOAD_DECODER:
	{
        if (drv_debug)
            rtd_pr_adsp_debug("[rtkaudio] RTKAUDIO_IOC_OMX_RELEASE_OFFLOAD_DECODER\n");

		mutex_lock(&dec_usage_lock);
		if (rtkaudio_dec_agent != -1) {
			rtkaudio_dec_inuse = false;
		} else {
			rtd_pr_adsp_debug("[rtkaudio] omx decoder did not create.\n");
		}
		mutex_unlock(&dec_usage_lock);

        if (drv_debug)
            rtd_pr_adsp_debug("[rtkaudio] RTKAUDIO_IOC_OMX_RELEASE_OFFLOAD_DECODER done\n");
		break;
	}
	case RTKAUDIO_IOC_GET_FW_CAPABILITY:
	{
		int caps;
		caps = rtkaudio_get_audio_caps();

		if (copy_to_user((void __user *) compat_ptr(arg), (void *)&caps, sizeof(int))) {
			ret = -EFAULT;
		}
		break;
	}
	case RTKAUDIO_IOC_SET_DMX_MODE:
	{
		int dmx_mode = 0;
		AUDIO_CONFIG_COMMAND_RTKAUDIO info;
		memset(&info, 0, sizeof(info));

		if (copy_from_user(&dmx_mode, (const void __user*) compat_ptr(arg), sizeof(dmx_mode))) {
			ret = -EFAULT;
			return ret;
		}

		rtd_pr_adsp_info("[rtkaudio] set downmix mode %d\n", dmx_mode);

		info.msg_id = AUDIO_CONFIG_CMD_DD_DOWNMIXMODE;
		info.value[1] = 0;
		info.value[2] = 0x00002379;

		if (dmx_mode == 1) {
			/* LORO */
			info.value[0] = MODE_STEREO;
		} else if (dmx_mode == 2) {
			/* ARIB */
			info.value[0] = MODE_ARIB;
		} else {
			/* default: LTRT */
			info.value[0] = MODE_DOLBY_SURROUND;
		}

		ret = rtkaudio_send_audio_config(&info);

		if (ret != S_OK) {
			ret = -EFAULT;
		}

		break;
	}
	default:
		rtd_pr_adsp_info("rtkaudio: unknown ioctl(0x%08x)\n", cmd);
		break;
	};

	return ret;
}
#endif
#endif

void rtkaudio_help(void)
{
	rtd_pr_adsp_warn(" echo f > /dev/rtkaudio        ## print audio f/w alloc memory summary\n");
	rtd_pr_adsp_warn(" echo u > /dev/rtkaudio        ## print user space alloc memory summary\n");
	rtd_pr_adsp_warn(" echo dno > /dev/rtkaudio      ## debug: stop write debug file\n");
	rtd_pr_adsp_warn(" echo dlg > /dev/rtkaudio      ## debug: write audio log to file\n");
	/* rtd_pr_adsp_warn(" echo dpf > /dev/rtkaudio      ## debug: write pp focus data to file\n"); */
	/* rtd_pr_adsp_warn(" echo dop > /dev/rtkaudio      ## debug: write ao pcm data to file\n"); */
	/* rtd_pr_adsp_warn(" echo dor > /dev/rtkaudio      ## debug: write ao raw data to file\n"); */
	/* rtd_pr_adsp_warn(" echo d n > /dev/rtkaudio      ## debug: write self-define to file, 4 < n <= 9\n"); */
	rtd_pr_adsp_warn(" echo fw@ [s] > /dev/rtkaudio  ## debug: send a string [s] to f/w\n");
	rtd_pr_adsp_warn(" echo show res > /dev/rtkaudio ## print audio resource status\n");
	rtd_pr_adsp_warn(" echo show dec > /dev/rtkaudio ## print audio decoder status\n");
	rtd_pr_adsp_warn(" echo show main > /dev/rtkaudio ## print main decoder status\n");
	rtd_pr_adsp_warn(" echo khaldebug show_digital_info x > /dev/rtkaudio ## print on/off log for RHAL_AUDIO_GetDigitalInfo, where x: 0/1 disable/enable\n");
	rtd_pr_adsp_warn(" echo khaldebug off > /dev/rtkaudio ## disble khaldebug\n");
}


int cmd_queue_create(int num)
{
	unsigned long vir_addr;
	unsigned long phy_addr;

	if(cq != NULL) {
		return 1;
	}
	cq_cached = (cmd_queue*)dvr_malloc_uncached_specific(sizeof(cmd_queue) + num * sizeof(cmd_packet), GFP_DCU1, (void**)&cq);

	vir_addr = (unsigned long) cq;
	if ((void*)vir_addr == NULL) {
		rtd_pr_adsp_err("[%s %d]alloc memory fail\n", __func__, __LINE__);
		return -1;
	}

	phy_addr = (unsigned long)dvr_to_phys((void *)vir_addr);
	if (!phy_addr) {
		rtd_pr_adsp_err("[%s %d]alloc memory fail\n", __func__, __LINE__);
		if (vir_addr) {
			dvr_free((void*)vir_addr);
		}
		return -1;
	}

	cq->pkt_array_kernel = (cmd_packet*)(vir_addr) + sizeof(cmd_packet);
	cq->pkt_array_user   = (cmd_packet*)(phy_addr);
	cq->header.num = num;
	cq->header.r_index = 0;
	cq->header.w_index = 0;
	rtd_pr_adsp_err("cq: header 0x%lx, user 0x%lx, kernel 0x%lx\n", cq, cq->pkt_array_user, cq->pkt_array_kernel);
	return 0;
}
int cmd_queue_isFull(void)
{
	int next_w_index = (cq->header.w_index + 1) % cq->header.num;
	return (next_w_index == cq->header.r_index);
}
int cmd_queue_isEmpty(void)
{
	return cq->header.w_index == cq->header.r_index;
}
int cmd_queue_write(cmd_packet* cmd)
{
	if(cq == NULL)
		return -1;

	//check cmd cq is full or not
	if(cmd_queue_isFull()){
		return -1;
	}

	cq->header.w_index = (cq->header.w_index + 1) % cq->header.num;
	memcpy(cq->pkt_array_kernel + cq->header.w_index, cmd, sizeof(cmd_packet));
	rtd_pr_adsp_err("[%s] cmd_type %d, w_index %d, r_index %d\n",__FUNCTION__,cmd->cmd_type,cq->header.w_index, cq->header.r_index);

	/*rtk_flush_range(cq, (void*)((int)cq + sizeof(cmd_queue) + CMDQ_NUM * sizeof(cmd_packet)));*/
	/*outer_flush_range((phys_addr_t)(unsigned long)pSrcPhy, (phys_addr_t)(unsigned long)pSrcPhy + bytes) ;*/
	return 0;
}

//pass down&up ring info to voice_ap
void info_up_down_ring(unsigned int down_header_phy, unsigned int up_header_phy)
{
	cmd_packet cmd;

	memset(&cmd, 0, sizeof(cmd_packet));
	cmd.cmd_type = CMD_INIT_DOWNRING;
	cmd.cmd[0] = (down_header_phy&0x1fffffff);
	cmd_queue_write(&cmd);

	memset(&cmd, 0, sizeof(cmd_packet));
	cmd.cmd_type = CMD_INIT_UPRING;
	cmd.cmd[0] = (up_header_phy&0x1fffffff);
	cmd_queue_write(&cmd);
}
EXPORT_SYMBOL(info_up_down_ring);

void suspend_voice(void)
{
	int cnt = 0;
	cmd_packet cmd;
	if(cq == NULL)
		return ;

	memset(&cmd, 0, sizeof(cmd_packet));
	cmd.cmd_type = CMD_SUSPEND;
	cmd_queue_write(&cmd);
	do {
	   mdelay(5); // wait until voice_ap done processing the command
	   cnt++;
	}
	while(!cmd_queue_isEmpty() && (cnt < 40));
}
EXPORT_SYMBOL(suspend_voice);

void rtkaudio_dap_onoff(int enable)
{
	char cmd_buf[100] = {0};

    if (enable) enable = 1;
    else enable = 0;

    snprintf(cmd_buf, 11, "ms12_dap %d", enable);
	rtkaudio_send_string(&cmd_buf[0], 11);
    rtd_pr_adsp_info("[rtkaudio] set dap status %s\n", (enable ? "ON" : "OFF"));
}
EXPORT_SYMBOL_GPL(rtkaudio_dap_onoff);

ssize_t rtkaudio_write(struct file *filp,
	const char *buf, size_t count, loff_t *f_pos)
{
	long ret = count;
	char cmd_buf[100] = {0};

    if (drv_debug)
        rtd_pr_adsp_debug("%s(): count=%d, buf=%p\n", __func__, (int)count, (void*)buf);

	if (count >= 100)
		return -EFAULT;

	if (copy_from_user(cmd_buf, buf, count)){
		ret = -EFAULT;
	}

	if ((cmd_buf[0] == 'f') && (count == 2)) {
		rtd_pr_adsp_notice("\n=== rtkaudio: show f/w memory usage ===\n");
		rtkaudio_fw_remote_malloc_summary(NULL);
		rtd_pr_adsp_notice("\n=======================================\n");
	} else if ((cmd_buf[0] == 'u') && (count == 2)) {
		rtd_pr_adsp_notice("\n=== rtkaudio: show usr memory usage ===\n");
		rtkaudio_user_alloc_summary(NULL);
		rtd_pr_adsp_notice("\n=======================================\n");
	} else if ((cmd_buf[0] == 'h') && (count == 2)) {
		rtd_pr_adsp_notice("\n=== rtkaudio write usage: =============\n");
		rtkaudio_help();
		rtd_pr_adsp_notice("\n=======================================\n");
	} else if ((cmd_buf[0] == 'f') && (cmd_buf[1] == 'w') && (cmd_buf[2] == '@')) {
        if (drv_debug)
            rtd_pr_adsp_debug("\n=== rtkaudio: send debug string: =============\n");
		rtkaudio_send_string(&cmd_buf[4], (count-4)-1);
        if (drv_debug)
            rtd_pr_adsp_debug("\n=======================================\n");
	} else if ((cmd_buf[0] == 's') && (cmd_buf[1] == 'h') &&
			(cmd_buf[2] == 'o') && (cmd_buf[3] == 'w')) {
		if ((cmd_buf[5] == 'a') && (cmd_buf[6] == 'l') &&
			(cmd_buf[7] == 's') && (cmd_buf[8] == 'a')) {
			int i;
			rtd_pr_adsp_err("alsa card %d, alsa device %d\n", alsa_info.card, alsa_info.device);
			rtd_pr_adsp_err("gst refclock info %p, Register value %x\n", (void*)gst_refc_info, rtd_inl(GST_SHARED_REGISTER));
			rtd_pr_adsp_err("size %d, index %d\n", gst_refc_info->size, gst_refc_info->index);
			for (i = 0; i < refc_info_size; i++) {
				rtd_pr_adsp_err("clock addr %x, pid %d\n", (unsigned int)gst_refc_info->info[i].phy_addr,
					gst_refc_info->info[i].pid);
			}
			rtd_pr_adsp_err("alsa card %d, alsa device %d\n",
				alsa_info.card, alsa_info.device);
		} else if ((cmd_buf[5] == 'g') && (cmd_buf[6] == 's') &&
			(cmd_buf[7] == 't') && (cmd_buf[8] == 'i') &&
			(cmd_buf[9] == 'n') && (cmd_buf[10] == 'f') &&
			(cmd_buf[11] == 'o')) {
				struct list_head *list;
				struct rtkaudio_open_list *open_info;

				down(&rtkaudio_open_info_list_sem);
				rtd_pr_adsp_err("rtkaudio_open_info_head %p\n", (void*)&rtkaudio_open_info_head);
				list_for_each(list, &rtkaudio_open_info_head) {
					open_info =
						list_entry(list, struct rtkaudio_open_list, open_list);
					rtd_pr_adsp_err("list %p, pid %d\n", (void*)list, open_info->pid);
				}
				up(&rtkaudio_open_info_list_sem);
		}
	} else if ((cmd_buf[0] == 'a') && (cmd_buf[1] == 'l') &&
			(cmd_buf[2] == 's') && (cmd_buf[3] == 'a')) {
		if ((cmd_buf[4] == 'c') && (cmd_buf[5] == 'a') &&
			(cmd_buf[6] == 'r') && (cmd_buf[7] == 'd')) {
			if ((int) cmd_buf[9] > 47 && (int) cmd_buf[9] < 56)
				alsa_info.card = (int) cmd_buf[9] - 48;
		} else if ((cmd_buf[4] == 'd') && (cmd_buf[5] == 'e') &&
			(cmd_buf[6] == 'v')) {
			if ((int) cmd_buf[8] > 47 && (int) cmd_buf[8] < 56)
				alsa_info.device = (int) cmd_buf[8] - 48;
		} else if ((cmd_buf[4] == 's') && (cmd_buf[5] == 'c') &&
			(cmd_buf[6] == 'a') && (cmd_buf[7] == 'n')) {
			alsa_info.card = 255;
		}
	} else if ((cmd_buf[0] == 'p') && (cmd_buf[1] == 'r') &&
			   (cmd_buf[2] == 'd') && (cmd_buf[3] == 'm') &&
			   (cmd_buf[4] == 'g') && (count == 8)) {
		if (cmd_buf[6] == '0')
			pr_to_dmesg_en = 0;
		else if (cmd_buf[6] == '1')
			pr_to_dmesg_en = 1;
	} else if ((cmd_buf[0] == 'd') && (cmd_buf[1] == 's') &&
			   (cmd_buf[2] == 'i') && (cmd_buf[3] == 'z') &&
			   (cmd_buf[4] == 'e') && (count == 8)) {
		if ((int) cmd_buf[6] > 47 && (int) cmd_buf[6] < 56)
			dump_size_scale = (int) cmd_buf[6] - 48;
		rtd_pr_adsp_err("dump_size_scale %d\n", dump_size_scale);
	} else if ((cmd_buf[0] == 'r') && (cmd_buf[1] == 'd') &&
			   (cmd_buf[2] == 'r') && (cmd_buf[3] == 'e') &&
			   (cmd_buf[4] == 'g')) {
		AUDIO_REGISTER_ACCESS_T info;
		char *ptr;
		char *endptr;

		ptr = cmd_buf+6;
		info.reg_type = DEBUG_REGISTER;
		info.reg_value = simple_strtol(ptr, &endptr, 16);

		rtd_pr_adsp_err("[rtkaudio] dbg read reg %x\n", info.reg_value);

		read_register_by_type(&info);
	} else if ((cmd_buf[0] == 'w') && (cmd_buf[1] == 't') &&
			   (cmd_buf[2] == 'r') && (cmd_buf[3] == 'e') &&
			   (cmd_buf[4] == 'g')) {
		AUDIO_REGISTER_ACCESS_T info;
		char *ptr;
		char *endptr;

		ptr = cmd_buf+6;
		info.reg_type = DEBUG_REGISTER;
		info.reg_value = simple_strtol(ptr, &endptr, 16);

		ptr = endptr+1;
		info.data = simple_strtol(ptr, &endptr, 16);

		rtd_pr_adsp_err("[rtkaudio] dbg write reg %x, value %x\n", info.reg_value, info.data);
		write_register_by_type(&info);
	} else if ((cmd_buf[0] == 'e') && (cmd_buf[1] == 'a') &&
			   (cmd_buf[2] == 'r') && (cmd_buf[3] == 'c') &&
			   (cmd_buf[4] == 'o') && (cmd_buf[5] == 'n')) {
		char *ptr;
		char *endptr;
		int onoff;

		ptr = cmd_buf + 7;
		onoff = simple_strtol(ptr, &endptr, 10);

		if (onoff) {
			eARC_offon = true;
		} else {
			eARC_offon = false;
		}

		rtd_pr_adsp_info("[rtkaudio] set earc status %s\n", (eARC_offon ? "ON" : "OFF"));
	} else if ((cmd_buf[0] == 'a') && (cmd_buf[1] == 'r') &&
			   (cmd_buf[2] == 'c') && (cmd_buf[3] == 'o') &&
			   (cmd_buf[4] == 'n')) {
		char *ptr;
		char *endptr;
		int onoff;

		ptr = cmd_buf + 6;
		onoff = simple_strtol(ptr, &endptr, 10);

		if (onoff) {
			ARC_offon = true;
		} else {
			ARC_offon = false;
		}

		rtd_pr_adsp_info("[rtkaudio] set arc status %s\n", (ARC_offon ? "ON" : "OFF"));
	} else if ((cmd_buf[0] == 'a') && (cmd_buf[1] == 'r') &&
			   (cmd_buf[2] == 'c') && (cmd_buf[3] == 's') &&
			   (cmd_buf[4] == 't') && (cmd_buf[5] == 'a') &&
			   (cmd_buf[6] == 't') && (cmd_buf[7] == 'u') &&
			   (cmd_buf[8] == 's')) {
		bool onoff;
		RHAL_AUDIO_HDMI_GetAudioReturnChannel(&onoff);
		rtd_pr_adsp_info("[rtkaudio] arc  status %s\n", (onoff ? "ON" : "OFF"));
		RHAL_AUDIO_HDMI_GetEnhancedAudioReturnChannel(&onoff);
		rtd_pr_adsp_info("[rtkaudio] earc status %s\n", (onoff ? "ON" : "OFF"));
	} else if ((cmd_buf[0] == 'a') && (cmd_buf[1] == 'l') &&
			(cmd_buf[2] == 's') && (cmd_buf[3] == 'a')) {
		if ((cmd_buf[4] == 'd') && (cmd_buf[5] == 'e') &&
			(cmd_buf[6] == 'b') && (cmd_buf[7] == 'u') &&
            (cmd_buf[8] == 'g')) {
			if ((int) cmd_buf[10] == 1)
                alsa_debug = 1;
            else
                alsa_debug = 0;

		    rtd_pr_adsp_info("[rtkaudio] alsa debug %s\n", (alsa_debug ? "ON" : "OFF"));
        }
    }else if ((cmd_buf[0] == 'd') && (cmd_buf[1] == 'e') &&
			(cmd_buf[2] == 'v')) {
		if ((cmd_buf[3] == 'd') && (cmd_buf[4] == 'e') &&
			(cmd_buf[5] == 'b') && (cmd_buf[6] == 'u') &&
            (cmd_buf[7] == 'g')) {
			if ((int) cmd_buf[9] == '1')
                drv_debug = 1;
            else
                drv_debug = 0;

		    rtd_pr_adsp_info("[rtkaudio] driver debug %s\n", (drv_debug ? "ON" : "OFF"));
        }
    }
    //khal debug command: "echo fw@ khaldebug show_digital_info x > /dev/rtkaudio", where "x" is 0 or 1
    else if (strncmp(cmd_buf, "khaldebug show_digital_info 0", 29) == 0){
		khal_debug &= ~KHAL_DEBUG_SHOW_DIGITALINFO;    // print off
		rtd_pr_adsp_info("[rtkaudio] khaldebug show_digital_info 0, print off\n");
    }
    else if (strncmp(cmd_buf, "khaldebug show_digital_info 1", 29) == 0){
		khal_debug |= KHAL_DEBUG_SHOW_DIGITALINFO;     // print on
		rtd_pr_adsp_info("[rtkaudio] khaldebug show_digital_info 1, print on\n");
    }
    //khal debug command: "echo fw@ khaldebug off > /dev/rtkaudio", disable all khal debug commands
    else if (strncmp(cmd_buf, "khaldebug off", 13) == 0){
		khal_debug = 0;     // disable khal debug
		rtd_pr_adsp_info("[rtkaudio] khaldebug off !\n");
    }else {
		rtd_pr_adsp_notice("%s\n", cmd_buf);
	}

	return ret;
}

const struct file_operations rtkaudio_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = rtkaudio_ioctl,
#ifdef CONFIG_ARM64
#ifdef CONFIG_COMPAT
	.compat_ioctl = rtkaudio_ioctl_compat,
#endif
#endif
	.write = rtkaudio_write,
	.open = rtkaudio_open,
	.release = rtkaudio_release,
};

static int rtkaudio_proc_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	struct list_head *list;
	struct rtkaudio_open_list *info_list;
	char *str = kmalloc(4096, GFP_KERNEL);
	int len, total = 0;

	if (str == NULL) {
		rtd_pr_adsp_err("[rtkaudio] malloc memory failed, %d\n", __LINE__);
		return 0;
	}

	memset(str, 0, 4096);
	len = snprintf(str, 4096, "gst_debug_info:\n");
	total += len;

	down(&rtkaudio_open_info_list_sem);
	list_for_each(list, &rtkaudio_open_info_head) {
		info_list = list_entry(list, struct rtkaudio_open_list, open_list);
		if (info_list->info.gst_open) {
			len = snprintf(str + total, 4096 - total, "pid = %d\n", info_list->pid);
			total += len;
			len = snprintf(str + total, 4096 - total, "undecoded_size = %llu\n",
				info_list->info.undecoded_size);
			total += len;
			len = snprintf(str + total, 4096 - total, "decoded_size = %llu\n",
				info_list->info.decoded_size);
			total += len;
		}
	}
	up(&rtkaudio_open_info_list_sem);

	if (copy_to_user(buf, str, total)) {
		rtd_pr_adsp_err("rtkaudio:%d Copy proc data to user space failed\n", __LINE__);
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
static const struct proc_ops rtkaudio_proc_fops = {
	.proc_read = (void*)rtkaudio_proc_read,
};
#else
static const struct file_operations rtkaudio_proc_fops = {
	.owner = THIS_MODULE,
	.read = (void*)rtkaudio_proc_read,
};
#endif

static void rtkaudio_debug_proc_init(void)
{
	if (rtkaudio_proc_dir == NULL && rtkaudio_proc_entry == NULL) {
		rtkaudio_proc_dir = proc_mkdir(RTKAUDIO_PROC_DIR , NULL);

		if (rtkaudio_proc_dir != NULL) {
			rtkaudio_proc_entry = proc_create(RTKAUDIO_PROC_ENTRY, 0666, rtkaudio_proc_dir, &rtkaudio_proc_fops);

			if (rtkaudio_proc_entry == NULL) {
				proc_remove(rtkaudio_proc_dir);
				rtkaudio_proc_dir = NULL;
				rtd_pr_adsp_err("failed to get proc entry for %s/%s\n", RTKAUDIO_PROC_DIR, RTKAUDIO_PROC_ENTRY);
				return;
			}
		} else {
			rtd_pr_adsp_err("create rtkaudio dir proc entry (%s) failed\n",
				RTKAUDIO_PROC_DIR);
			return;
		}
	}
}

static void rtkaudio_debug_proc_uninit(void)
{
	if (rtkaudio_proc_entry) {
		proc_remove(rtkaudio_proc_entry);
		rtkaudio_proc_entry = NULL;
	}

	if (rtkaudio_proc_dir) {
		proc_remove(rtkaudio_proc_dir);
		rtkaudio_proc_dir = NULL;
	}
}

static char *rtkaudio_devnode(struct device *dev, umode_t *mode)
{
	*mode = 0666;
	return NULL;
}

int pm_count = 0;
int init_done = 0;

void rtkaudio_runtime_get(void)
{
	if (init_done) {
		rtd_pr_adsp_info("rtkaudio:pm_runtime_get:pm_count=%d\n", pm_count);
		pm_runtime_get_sync(&rtkaudio_platform_devs->dev);
	} else {
		pm_count++;
	}
}
EXPORT_SYMBOL(rtkaudio_runtime_get);

void rtkaudio_runtime_put(void)
{
	if (init_done) {
		rtd_pr_adsp_info("rtkaudio:pm_runtime_put:pm_count=%d\n", pm_count);
		pm_runtime_put(&rtkaudio_platform_devs->dev);
	} else {
		pm_count--;
	}
}
EXPORT_SYMBOL(rtkaudio_runtime_put);

static int rtkaudio_init(void)
{
	int result;
	int i;
	unsigned long vir_addr = 0;
	unsigned long phy_addr = 0;
	dev_t dev = 0;
	unsigned long ret = S_OK;
	rtkaudio_class = NULL;
	rtkaudio_platform_devs = NULL;
	rtkaudio_dec_agent = -1;
	eARC_offon = false;
	ARC_offon = false;

	rtd_pr_adsp_info("rtkaudio: audio driver for Realtek TV Platform\n");

	if (rtkaudio_major) {
		dev = MKDEV(RTKAUDIO_MAJOR, 0);
		result = register_chrdev_region(dev, 1, "rtkaudio");
	} else {
		result = alloc_chrdev_region(&dev,
			rtkaudio_minor, 1, "rtkaudio");
		rtkaudio_major = MAJOR(dev);
	}
	if (result < 0) {
		rtd_pr_adsp_info("rtkaudio: can not get chrdev region...\n");
		return result;
	}

	rtkaudio_class = class_create(THIS_MODULE, "rtkaudio");
	if (IS_ERR(rtkaudio_class)) {
		rtd_pr_adsp_info("rtkaudio: can not create class...\n");
		result = PTR_ERR(rtkaudio_class);
		goto fail_class_create;
	}

	rtkaudio_class->devnode = rtkaudio_devnode;

	rtkaudio_platform_devs =
		platform_device_register_simple("rtkaudio", -1, NULL, 0);

	if (platform_driver_register(&rtkaudio_device_driver) != 0) {
		rtd_pr_adsp_info("rtkaudio: can not register platform driver...\n");
		result = -EINVAL;
		goto fail_platform_driver_register;
	}

	cdev_init(&rtkaudio_cdev, &rtkaudio_fops);
	rtkaudio_cdev.owner = THIS_MODULE;
	rtkaudio_cdev.ops = &rtkaudio_fops;
	result = cdev_add(&rtkaudio_cdev, dev, 1);
	if (result < 0) {
		rtd_pr_adsp_info("rtkaudio: can not add character device...\n");
		goto fail_cdev_init;
	}
	device_create(rtkaudio_class, NULL, dev, NULL, "rtkaudio");


	device_enable_async_suspend(&(rtkaudio_platform_devs->dev));
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	set_rtkaudio_device_ptr(&(rtkaudio_platform_devs->dev));
#endif
	INIT_LIST_HEAD(&remote_malloc_info_head);
	INIT_LIST_HEAD(&rtkaudio_alloc_head);
	INIT_LIST_HEAD(&rtkaudio_open_info_head);

	if (register_kernel_rpc(RPC_ACPU_REMOTE_MALLOC,
		rtkaudio_remote_malloc) == 1)
		rtd_pr_adsp_err("Register rtkaudio_remote_malloc failed!\n");
	if (register_kernel_rpc(RPC_ACPU_REMOTE_FREE,
		rtkaudio_remote_free) == 1)
		rtd_pr_adsp_err("Register rtkaudio_remote_free failed!\n");
	if (register_kernel_rpc(RPC_ACPU_FREE_USER_MEM,
		rtkaudio_free_user_mem) == 1)
		rtd_pr_adsp_err("Register rtkaudio_free_user_mem failed!\n");
#ifdef CONFIG_ARCH_RTK6702
	if (register_kernel_rpc(RPC_ACPU_SET_HDMITX_FORMAT,
		rtkaudio_set_hdmitx_format) == 1)
		rtd_pr_adsp_err("Register rtkaudio_set_hdmitx_format failed!\n");
#endif

	rtd_pr_adsp_info("rtkaudio: send REMOTE MELLOC ENABLE RPC\n");

	/******************** get audio version ********************/
	memset(AudioGITHASH, 0, 8);

	vir_addr = (unsigned long)dvr_malloc_specific(4096, GFP_DCU1);

	if ((void*)vir_addr == NULL) {
		rtd_pr_adsp_err("[%s %d] alloc memory fail\n", __func__, __LINE__);
		return -ENOMEM;
	}

	phy_addr = (unsigned long)dvr_to_phys((void*)vir_addr);
	if ((void*)phy_addr == NULL) {
		rtd_pr_adsp_err("[%s %d] mapping physical address fail\n", __func__, __LINE__);
		if (vir_addr) {
			dvr_free((void*)vir_addr);
		}
		return -ENOMEM;
	}
	/*************************************************************/

#ifdef CONFIG_LG_SNAPSHOT_BOOT
	if (snapshot_enable == 0) {
		/* send REMOTE MELLOC ENABLE RPC to Audio FW */
		if (send_rpc_command(RPC_AUDIO,
			RPC_ACPU_IS_REMOTE_MALLOC_ENABLE,
			REMOTE_MALLOC_ENABLE, phy_addr, &ret)) {
			rtd_pr_adsp_err("rtkaudio: REMOTE_MALLOC_ENABLE RPC fail %ld\n", ret);
		}

		rtkaudio_enable_tsk = kthread_create(rtkaudio_enable_thread, NULL,
				"rtkaudio_enable_thread");
		if (IS_ERR(rtkaudio_enable_tsk)) {
			rtkaudio_enable_tsk = NULL;
			return -1;
		}
		wake_up_process(rtkaudio_enable_tsk);
		control_status_init();
	}
#else
	/* send REMOTE MELLOC ENABLE RPC to Audio FW */
	if (send_rpc_command(RPC_AUDIO,
		RPC_ACPU_IS_REMOTE_MALLOC_ENABLE,
		REMOTE_MALLOC_ENABLE, phy_addr, &ret)) {
		rtd_pr_adsp_err("rtkaudio: REMOTE_MALLOC_ENABLE RPC fail %ld\n", ret);
	}

	rtkaudio_enable_tsk = kthread_create(rtkaudio_enable_thread, NULL,
			"rtkaudio_enable_thread");
	if (IS_ERR(rtkaudio_enable_tsk)) {
		rtkaudio_enable_tsk = NULL;
		return -1;
	}
	wake_up_process(rtkaudio_enable_tsk);
	control_status_init();
#endif

	init_rpc_mem();

	/******************** get audio version ********************/
	memcpy(AudioGITHASH, (char *)vir_addr, 7);
	AudioGITHASH[7] = 0;
	dvr_free((void *) vir_addr);
	/***********************************************************/

	rtkaudio_proc_dir = NULL;
	rtkaudio_proc_entry = NULL;

	rtkaudio_debug_proc_init();

	/* enable print ADSP log to dmesg at init */
	pr_to_dmesg_en = 0;

	/* set dump log file init size to 1MB */
	dump_size_scale = 1;

	if (ret != S_OK)
		rtd_pr_adsp_err("rtkaudio: REMOTE_MALLOC_ENABLE RPC is not S_OK %ld\n", ret);

	pm_runtime_forbid(&rtkaudio_platform_devs->dev);
	pm_runtime_set_active(&rtkaudio_platform_devs->dev);
	pm_runtime_enable(&rtkaudio_platform_devs->dev);//enable runtime PM
#ifdef CONFIG_RTK_LOW_POWER_MODE
	rtk_lpm_add_device(&rtkaudio_platform_devs->dev);
#endif

	init_done = 1;

	for (i = 0; i < pm_count; i++) {
		pm_runtime_get(&rtkaudio_platform_devs->dev);
	}

	rtd_pr_adsp_info("rtkaudio:pm_runtime_get:pm_count=%d\n", pm_count);

    if (QuickShowEnabled()) {
        RHAL_QS_AUDIO_InitializeModule(HAL_AUDIO_SIF_TYPE_NONE);
    } else {
	    RHAL_AUDIO_InitializeModule(HAL_AUDIO_SIF_TYPE_NONE);
    }


	//pm_runtime_forbid(&rtkaudio_platform_devs->dev);
	//pm_runtime_set_active(&rtkaudio_platform_devs->dev);
	//pm_runtime_enable(&rtkaudio_platform_devs->dev);//enable runtime PM
#ifdef CONFIG_RTK_LOW_POWER_MODE
	rtk_lpm_add_device(&rtkaudio_platform_devs->dev);
#endif

	//init_done = 1;

	//for (i = 0; i < pm_count; i++) {
	//	pm_runtime_get(&rtkaudio_platform_devs->dev);
	//}

	//pr_info("rtkaudio:pm_runtime_get:pm_count=%d\n", pm_count);

	return 0;

fail_cdev_init:
	platform_driver_unregister(&rtkaudio_device_driver);
fail_platform_driver_register:
	platform_device_unregister(rtkaudio_platform_devs);
	rtkaudio_platform_devs = NULL;
	class_destroy(rtkaudio_class);
fail_class_create:
	rtkaudio_class = NULL;
	unregister_chrdev_region(dev, 1);
	return result;
}

static void rtkaudio_exit(void)
{
	dev_t dev = MKDEV(rtkaudio_major, rtkaudio_minor);
	rtkaudio_debug_proc_uninit();

	if (rtkaudio_platform_devs == NULL)
		BUG();

	device_destroy(rtkaudio_class, dev);
	cdev_del(&rtkaudio_cdev);

	platform_driver_unregister(&rtkaudio_device_driver);
	platform_device_unregister(rtkaudio_platform_devs);
	rtkaudio_platform_devs = NULL;

	class_destroy(rtkaudio_class);
	rtkaudio_class = NULL;

	unregister_chrdev_region(dev, 1);

	RHAL_AUDIO_FinalizeModule();
}

module_init(rtkaudio_init);
module_exit(rtkaudio_exit);
MODULE_AUTHOR("realtek.com.tw");
MODULE_LICENSE("GPL");

void halt_acpu(void)
{
	unsigned long ret;
	int status;
	// use non-blcok mode here, since acpu will go to while(1) after this rpc
	status = send_krpc(RPC_AUDIO, KRPC_PROCEDUREID_KERNEL,KRPC_NON_BLOCK_MODE, RPC_ACPU_AUDIO_HALT, 0, 0, &ret);
	if( status == RPC_FAIL)
		rtd_pr_adsp_err("%s: fail.\n", __func__);
	else
		mdelay(50); // wait until acpu is done.
}

static int audio_send_rpc_command(int opt,
	 unsigned long command, unsigned long param1,
	 unsigned long param2, unsigned long param2_LE,
	 unsigned long *retvalue)
{
	int ret, count;
	RPCRES_LONG *audio_ret;
	ret = 0;
	count = 0;

#ifdef CONFIG_CUSTOMER_TV001
	do {
		ret = send_rpc_command(opt, command, param1, param2, retvalue);

		if (ret == RPC_FAIL) {
			rtd_pr_adsp_err("[rtkaudio] RPC to ACPU fail!!\n");
			return -1;
		}

		if (ret == RPC_OK)
			break;

		// RPC_ACPU_NOT_READY
		msleep(100);
		count++;
	} while (count <= 100);

	if (ret == RPC_ACPU_NOT_READY) {
		rtd_pr_adsp_err("[rtkaudio] wait ACPU ready timeout!!!\n");
		return -1;
	}
#else
	if (send_rpc_command(opt, command, param1, param2, retvalue))
		ret = -1;
#endif

	audio_ret = (RPCRES_LONG *)param2_LE;

	if (command == ENUM_KERNEL_RPC_CHECK_READY ||
	command == ENUM_KERNEL_RPC_PRIVATEINFO ||
	command == ENUM_KERNEL_RPC_GET_MUTE_N_VOLUME) {
		if (*retvalue != S_OK) {
		   rtd_pr_adsp_err("[rtkaudio] RPC S_OK fail\n");
		   rtd_pr_adsp_err("[rtkaudio] retvalue %lx\n", *retvalue);
		   ret = -1;
		}
	} else {
		if (*retvalue != S_OK || ntohl(audio_ret->result) != S_OK) {
			rtd_pr_adsp_err("[rtkaudio] RPC S_OK fail\n");
			rtd_pr_adsp_err("[rtkaudio] retvalue %lx, result %x, command %lx\n",
				*retvalue, ntohl(audio_ret->result), command);
			ret = -1;
		}
	}

	return ret;
}

static int rtkaudio_create_decoder_agent(struct platform_device *rtkaudio_dev)
{
	struct AUDIO_RPC_INSTANCE *info, *info_audio;
	RPCRES_LONG *res, *res_audio;
	unsigned long ret;

	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long phy_addr = 0;

	vir_addr = get_rpc_mem(&unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[audio] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = (unsigned long) dvr_to_phys(vir_addr);

	/* set parameter to 0, for snapshot */
	memset(unvir_addr, 0, RPC_DVR_MALLOC_UNCACHED_SIZE);

	info = unvir_addr;
	info_audio = (void *)(struct AUDIO_RPC_INSTANCE *)((ulong) phy_addr);

	res = (void *)(RPCRES_LONG *) ((((unsigned long)unvir_addr + sizeof(struct AUDIO_RPC_INSTANCE) + 8) & ALIGN4));
	res_audio = (void *)(RPCRES_LONG *) (((((unsigned long) phy_addr) + sizeof(struct AUDIO_RPC_INSTANCE) + 8) & ALIGN4));

	/* create decoder */
	info->type = htonl(AUDIO_DECODER);
	info->instanceID = htonl(-1);

	if (audio_send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_CREATE_AGENT,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		(unsigned long) res,
		&ret)) {
		rtd_pr_adsp_err("[rtkaudio] %s %d RPC fail\n", __func__, __LINE__);
		put_rpc_mem(vir_addr);
		return -1;
	}

	rtkaudio_dec_agent = ntohl(res->data);

	put_rpc_mem(vir_addr);
	rtd_pr_adsp_info("[rtkaudio] Create decode instance %x\n", rtkaudio_dec_agent);
	return 0;
}

static int rtkaudio_get_audio_caps(void)
{
	int caps = 0;

	unsigned long ret;
	AUDIO_RPC_PRIVATEINFO_PARAMETERS *cmd, *cmd_audio;
	AUDIO_RPC_PRIVATEINFO_RETURNVAL *res, *res_audio;

	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long phy_addr = 0;

	vir_addr = get_rpc_mem(&unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[audio] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = (unsigned long) dvr_to_phys(vir_addr);

	/* set parameter to 0, for snapshot */
	memset(unvir_addr, 0, RPC_DVR_MALLOC_UNCACHED_SIZE);

	cmd = unvir_addr;
	res = (void *)(AUDIO_RPC_PRIVATEINFO_RETURNVAL *)((((unsigned long)unvir_addr + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS) + 8) & ALIGN4));

	cmd_audio = (void *)(AUDIO_RPC_PRIVATEINFO_PARAMETERS *) ((ulong) phy_addr);
	res_audio = (void *)(AUDIO_RPC_PRIVATEINFO_RETURNVAL *) (((((ulong) phy_addr) + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS) + 8) & ALIGN4));

	memset(cmd, 0, sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS));
	cmd->type = htonl(ENUM_PRIVATEINFO_AUDIO_FIRMWARE_CAPABILITY);

	if (audio_send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_PRIVATEINFO,
		(unsigned long) cmd_audio,
		(unsigned long) res_audio,
		(unsigned long) res, &ret)) {
		rtd_pr_adsp_err("[audio] %s %d RPC fail\n", __func__, __LINE__);
		put_rpc_mem(vir_addr);
		return -1;
	}

	if (ret == S_OK) {
		caps = res->privateInfo[3];
		rtd_pr_adsp_info("[audio] get ADSP caps %x\n", caps);
	}

	put_rpc_mem(vir_addr);

	return caps;
}

int get_EQ_BASS_enable(ENUM_AUDIO_EQ_BASS_MODE mode, int band_idx, int *bOnOff)
{
	int ret = -EFAULT;

	switch(mode)
	{
		case ENUM_AUDIO_EQ:
			if (band_idx < EQ_NUM_BANDS) {
				*bOnOff = equalizer.bOnOff[band_idx];
				ret = RPC_OK;
			} else {
				ret = -EFAULT;
			}
			break;
		case ENUM_AUDIO_BASS_TREBLE_LR:
			if (band_idx < BASS_TREBLE_LR_BANDS) {
				*bOnOff = bass_treble_lr.bOnOff[band_idx];
				ret = RPC_OK;
			} else {
				ret = -EFAULT;
			}
			break;
		case ENUM_AUDIO_BASS_TREBLE_LSRS:
			if (band_idx < BASS_TREBLE_LR_BANDS) {
				*bOnOff = bass_treble_lsrs.bOnOff[band_idx];
				ret = RPC_OK;
			} else {
				ret = -EFAULT;
			}
			break;
		case ENUM_AUDIO_BASS_TREBLE_CSW:
			if (band_idx < BASS_TREBLE_LR_BANDS) {
				*bOnOff = bass_treble_csw.bOnOff[band_idx];
				ret = RPC_OK;
			} else {
				ret = -EFAULT;
			}
			break;
		default:
			break;
	}

	return ret;
}

int get_EQ_BASS_parameter(ENUM_AUDIO_EQ_BASS_MODE mode, int band_idx, int *fc, int *bw, int *gain)
{
	int ret = -EFAULT;

	switch(mode)
	{
		case ENUM_AUDIO_EQ:
			if (band_idx < EQ_NUM_BANDS) {
				*fc = equalizer.fc[band_idx];
				*bw = equalizer.bw[band_idx];
				*gain = equalizer.gain[band_idx];
				ret = RPC_OK;
			} else {
				ret = -EFAULT;
			}
			break;
		case ENUM_AUDIO_BASS_TREBLE_LR:
			if (band_idx < BASS_TREBLE_LR_BANDS) {
				*fc = bass_treble_lr.fc[band_idx];
				*bw = bass_treble_lr.bw[band_idx];
				*gain = bass_treble_lr.gain[band_idx];
				ret = RPC_OK;
			} else {
				ret = -EFAULT;
			}
			break;
		case ENUM_AUDIO_BASS_TREBLE_LSRS:
			if (band_idx < BASS_TREBLE_LR_BANDS) {
				*fc = bass_treble_lsrs.fc[band_idx];
				*bw = bass_treble_lsrs.bw[band_idx];
				*gain = bass_treble_lsrs.gain[band_idx];
				ret = RPC_OK;
			} else {
				ret = -EFAULT;
			}
			break;
		case ENUM_AUDIO_BASS_TREBLE_CSW:
			if (band_idx < BASS_TREBLE_LR_BANDS) {
				*fc = bass_treble_csw.fc[band_idx];
				*bw = bass_treble_csw.bw[band_idx];
				*gain = bass_treble_csw.gain[band_idx];
				ret = RPC_OK;
			} else {
				ret = -EFAULT;
			}
			break;
		default:
			break;

	}

	return ret;
}

int get_EQ_BASS_audio_data_path_status(ENUM_AUDIO_EQ_BASS_MODE mode, int *bOnOff)
{
	int ret = -EFAULT;

	switch(mode)
	{
		case ENUM_AUDIO_EQ:
			*bOnOff = equalizer.audio_data_path_status;
			ret = RPC_OK;
			break;
		case ENUM_AUDIO_BASS_TREBLE_LR:
			*bOnOff = bass_treble_lr.audio_data_path_status;
			ret = RPC_OK;
			break;
		case ENUM_AUDIO_BASS_TREBLE_LSRS:
			*bOnOff = bass_treble_lsrs.audio_data_path_status;
			ret = RPC_OK;
			break;
		case ENUM_AUDIO_BASS_TREBLE_CSW:
			*bOnOff = bass_treble_csw.audio_data_path_status;
			ret = RPC_OK;
			break;
		default:
			break;
	}

	return ret;
}

static int update_local_Onoff(ENUM_AUDIO_EQ_BASS_MODE mode, int band_idx, int bOnOff)
{
	long ret = -EFAULT;
	switch(mode) {

		case ENUM_AUDIO_EQ:
			if (band_idx < EQ_NUM_BANDS) {
				equalizer.bOnOff[band_idx] = bOnOff;
				ret = RPC_OK;
			} else {
				ret = -EFAULT;
			}
			break;
		case ENUM_AUDIO_BASS_TREBLE_LR:
			if (band_idx < BASS_TREBLE_LR_BANDS) {
				bass_treble_lr.bOnOff[band_idx] = bOnOff;
				ret = RPC_OK;
			} else {
				ret = -EFAULT;
			}
			break;
		case ENUM_AUDIO_BASS_TREBLE_LSRS:
			if (band_idx < BASS_TREBLE_LR_BANDS) {
				bass_treble_lsrs.bOnOff[band_idx] = bOnOff;
				ret = RPC_OK;
			} else {
				ret = -EFAULT;
			}
			break;
		case ENUM_AUDIO_BASS_TREBLE_CSW:
			if (band_idx < BASS_TREBLE_LR_BANDS) {
				bass_treble_csw.bOnOff[band_idx] = bOnOff;
				ret = RPC_OK;
			} else {
				ret = -EFAULT;
			}
			break;
		default:
			break;
	}

	return ret;
}

int set_EQ_BASS_enable(ENUM_AUDIO_EQ_BASS_MODE mode, int band_idx, int bOnOff)
{
	long ret = 0;
	AUDIO_RPC_PRIVATEINFO_PARAMETERS *cmd;
	AUDIO_RPC_PRIVATEINFO_RETURNVAL *res;
	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long cmd_audio = 0, res_audio = 0;
	unsigned long phy_addr = 0;

	vir_addr = dvr_malloc_uncached(4096, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[SWEQ] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = dvr_to_phys(vir_addr);

	down(&eq_bass_cmd_sem);

	cmd = unvir_addr;
	res = (AUDIO_RPC_PRIVATEINFO_RETURNVAL *) (((unsigned long)unvir_addr + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS) + sizeof(eq_bass_cmd_info) + 8) & ALIGN4);

	cmd_audio = phy_addr;
	res_audio = (phy_addr + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS) + sizeof(eq_bass_cmd_info) + 8) & ALIGN4;
	memset(cmd, 0, sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS) + sizeof(eq_bass_cmd_info));

	cmd->instanceID = htonl(0);
	cmd->type = htonl(ENUM_PRIVATEINFO_AUDIO_CONFIG_EQ_BASS_PARAM);
	cmd->privateInfo[0] = htonl(ENUM_EQ_BASS_CMD_TYPE_SETENABLE);
	cmd->privateInfo[1] = htonl(sizeof(int) * 3);
	cmd->privateInfo[2] = htonl((int)mode);
	cmd->privateInfo[3] = htonl(band_idx);
	cmd->privateInfo[4] = htonl(bOnOff);

	if (send_rpc_command(RPC_AUDIO, ENUM_KERNEL_RPC_PRIVATEINFO, cmd_audio, res_audio, &ret)) {
		rtd_pr_adsp_err("[SWEQ] %s %d rpc fail %ld\n", __func__, __LINE__, ret);
		ret = -EFAULT;
	}
	if (ret != S_OK) {
		rtd_pr_adsp_err("[SWEQ] %s %d rpc is not S_OK %ld\n", __func__, __LINE__, ret);
		ret = -EFAULT;
	}

	up(&eq_bass_cmd_sem);
	dvr_free(vir_addr);

	update_local_Onoff(mode, band_idx, bOnOff);

	return ret;
}

static int update_local_params(ENUM_AUDIO_EQ_BASS_MODE mode, int band_idx, ENUM_AUDIO_EQ_BASS_FILTER_TYPE filter_type, int fc, int bw, int gain)
{
	long ret = -EFAULT;
	switch(mode) {

		case ENUM_AUDIO_EQ:
			if (band_idx < EQ_NUM_BANDS) {
				equalizer.fc[band_idx]          = fc;
				equalizer.bw[band_idx]          = bw;
				equalizer.gain[band_idx]        = gain;
				equalizer.filter_type[band_idx] = filter_type;
				ret = RPC_OK;
			} else {
				ret = -EFAULT;
			}
			break;
		case ENUM_AUDIO_BASS_TREBLE_LR:
			if (band_idx < BASS_TREBLE_LR_BANDS) {
				bass_treble_lr.fc[band_idx]          = fc;
				bass_treble_lr.bw[band_idx]          = bw;
				bass_treble_lr.gain[band_idx]        = gain;
				bass_treble_lr.filter_type[band_idx] = filter_type;
				ret = RPC_OK;
			} else {
				ret = -EFAULT;
			}
			break;
		case ENUM_AUDIO_BASS_TREBLE_LSRS:
			if (band_idx < BASS_TREBLE_LR_BANDS) {
				bass_treble_lsrs.fc[band_idx]          = fc;
				bass_treble_lsrs.bw[band_idx]          = bw;
				bass_treble_lsrs.gain[band_idx]        = gain;
				bass_treble_lsrs.filter_type[band_idx] = filter_type;
				ret = RPC_OK;
			} else {
				ret = -EFAULT;
			}
			break;
		case ENUM_AUDIO_BASS_TREBLE_CSW:
			if (band_idx < BASS_TREBLE_LR_BANDS) {
				bass_treble_csw.fc[band_idx]          = fc;
				bass_treble_csw.bw[band_idx]          = bw;
				bass_treble_csw.gain[band_idx]        = gain;
				bass_treble_csw.filter_type[band_idx] = filter_type;
				ret = RPC_OK;
			} else {
				ret = -EFAULT;
			}
			break;
		default:
			break;
	}

	return ret;
}

int set_EQ_BASS_parameter(ENUM_AUDIO_EQ_BASS_MODE mode, int band_idx, ENUM_AUDIO_EQ_BASS_FILTER_TYPE filter_type, int fc, int bw, int gain)
{
	long ret = 0;
	AUDIO_RPC_PRIVATEINFO_PARAMETERS *cmd;
	AUDIO_RPC_PRIVATEINFO_RETURNVAL *res;
	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long cmd_audio = 0, res_audio = 0;
	unsigned long phy_addr = 0;

	vir_addr = dvr_malloc_uncached(4096, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[SWEQ] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = dvr_to_phys(vir_addr);

	down(&eq_bass_cmd_sem);

	cmd = unvir_addr;
	res = (AUDIO_RPC_PRIVATEINFO_RETURNVAL *) (((unsigned long)unvir_addr + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS) + sizeof(eq_bass_cmd_info) + 8) & ALIGN4);

	cmd_audio = phy_addr;
	res_audio = (phy_addr + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS) + sizeof(eq_bass_cmd_info) + 8) & ALIGN4;
	memset(cmd, 0, sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS)+sizeof(eq_bass_cmd_info));

	cmd->instanceID = htonl(0);
	cmd->type = htonl(ENUM_PRIVATEINFO_AUDIO_CONFIG_EQ_BASS_PARAM);
	cmd->privateInfo[0] = htonl(ENUM_EQ_BASS_CMD_TYPE_SETPARAM);
	cmd->privateInfo[1] = htonl(sizeof(int) * 6);
	cmd->privateInfo[2] = htonl((int)mode);
	cmd->privateInfo[3] = htonl(band_idx);
	cmd->privateInfo[4] = htonl((int)filter_type);
	cmd->privateInfo[5] = htonl(fc);
	cmd->privateInfo[6] = htonl(bw);
	cmd->privateInfo[7] = htonl(gain);

	if (send_rpc_command(RPC_AUDIO, ENUM_KERNEL_RPC_PRIVATEINFO, cmd_audio, res_audio, &ret)) {
		rtd_pr_adsp_err("[SWEQ] %s %d rpc fail %ld\n", __func__, __LINE__, ret);
		ret = -EFAULT;
	}
	if (ret != S_OK) {
		rtd_pr_adsp_err("[SWEQ] %s %d rpc is not S_OK %ld\n", __func__, __LINE__, ret);
		ret = -EFAULT;
	}

	up(&eq_bass_cmd_sem);
	dvr_free(vir_addr);

	update_local_params(mode, band_idx, filter_type, fc, bw, gain);

	return ret;
}

static int update_local_audio_data_path_status(ENUM_AUDIO_EQ_BASS_MODE mode, int bOnOff)
{
	int ret = -EFAULT;
	switch(mode) {
		case ENUM_AUDIO_EQ:
			equalizer.audio_data_path_status = bOnOff;
			ret = RPC_OK;
			break;
		case ENUM_AUDIO_BASS_TREBLE_LR:
			bass_treble_lr.audio_data_path_status = bOnOff;
			ret = RPC_OK;
			break;
		case ENUM_AUDIO_BASS_TREBLE_LSRS:
			bass_treble_lsrs.audio_data_path_status = bOnOff;
			ret = RPC_OK;
			break;
		case ENUM_AUDIO_BASS_TREBLE_CSW:
			bass_treble_csw.audio_data_path_status = bOnOff;
			ret = RPC_OK;
			break;
		default:
			break;
	}

	return ret;
}

int set_EQ_BASS_Audio_Data_Path_Enable(ENUM_AUDIO_EQ_BASS_MODE mode, int bOnOff)
{
	long ret = 0;
	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long cmd_audio = 0, res_audio = 0;
	unsigned long phy_addr = 0;
	AUDIO_RPC_PRIVATEINFO_PARAMETERS *cmd;
	AUDIO_RPC_PRIVATEINFO_RETURNVAL *res;

	vir_addr = dvr_malloc_uncached(RPC_DVR_MALLOC_UNCACHED_SIZE, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[audio] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = dvr_to_phys(vir_addr);

	down(&eq_bass_cmd_sem);

	cmd = unvir_addr;
	res = (AUDIO_RPC_PRIVATEINFO_RETURNVAL *) (((unsigned long)unvir_addr + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS) + sizeof(eq_bass_cmd_info) + 8) & ALIGN4);

	cmd_audio = phy_addr;
	res_audio = (phy_addr + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS) + sizeof(eq_bass_cmd_info) + 8) & ALIGN4;

	memset(cmd, 0, sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS) + sizeof(eq_bass_cmd_info));

	cmd->instanceID = htonl(0);
	cmd->type = htonl(ENUM_PRIVATEINFO_AUDIO_CONFIG_EQ_BASS_PARAM);
	cmd->privateInfo[0] = htonl(ENUM_EQ_BASS_CMD_TYPE_SET_AUDIO_DATA_PATH);
	cmd->privateInfo[1] = htonl(sizeof(int)*2);
	cmd->privateInfo[2] = htonl((int)mode);
	cmd->privateInfo[3] = htonl(bOnOff);

	if (send_rpc_command(RPC_AUDIO, ENUM_KERNEL_RPC_PRIVATEINFO, cmd_audio, res_audio, &ret)) {
		rtd_pr_adsp_err("[SWEQ] %s %d rpc fail %ld\n", __func__, __LINE__, ret);
		ret = -EFAULT;
	}
	if (ret != S_OK) {
		rtd_pr_adsp_err("[SWEQ] %s %d rpc is not S_OK %ld\n", __func__, __LINE__, ret);
		ret = -EFAULT;
	}

	update_local_audio_data_path_status(mode, bOnOff);

	up(&eq_bass_cmd_sem);
	dvr_free(vir_addr);

	return ret;
}
