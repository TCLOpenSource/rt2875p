/****************************************************************************
* This product contains one or more programs protected under international
* and U.S. copyright laws as unpublished works.  They are confidential and
* proprietary to Dolby Laboratories.  Their reproduction or disclosure, in
* whole or in part, or the production of derivative works therefrom without
* the express permission of Dolby Laboratories is prohibited.
*
*             Copyright 2011 - 2013 by Dolby Laboratories.
*                         All rights reserved.
****************************************************************************/

/*! Copyright &copy; 2013 Dolby Laboratories, Inc.
    All Rights Reserved

    @file  edbec_api.cpp
    @brief EDR decoder backend control software module implementation.

    This file implement the EDR decoder backend control software module.
*/
#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/dma-mapping.h>
#include <linux/uaccess.h>
#include <asm/barrier.h>
#include <asm/cacheflush.h>
#include <mach/platform.h>
#include <mach/system.h>
#include <mach/timex.h>

#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/random.h>
#ifdef CONFIG_CMA_RTK_ALLOCATOR
#include <linux/pageremap.h>
#endif
#include <linux/kthread.h>  /* for threads */
#include <linux/time.h>   /* for using jiffies */
#include <linux/hrtimer.h>

#include <linux/proc_fs.h>
#include <linux/version.h>

#include <rbus/timer_reg.h>
#include <rbus/dmato3dtable_reg.h>
#include <rbus/vgip_reg.h>
#include <scaler/scalerCommon.h>
#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <tvscalercontrol/scalerdrv/scaler_hdr_ctrl.h>
#include <rtk_kdriver/tvscalercontrol/vip/scalerColor.h>

#include <tvscalercontrol/hdmirx/hdmi_vfe.h>
#include <scaler_vpqmemcdev.h>
#include <tvscalercontrol/vip/scalerColor_tv006.h>

#include "dolby_adapter.h"
#include "dolby_device.h"
#include "dolby_debug.h"
#include "dolby_gdbs_delay.h"
#include <rtd_log/rtd_module_log.h>

/* for HDMI interrupt control */
extern spinlock_t *dolby_hdmi_ui_change_spinlock(void);
static struct class *dolbyvisionEDR_class;

typedef struct {
	int		initialized;
	struct cdev	cdev ;         /* Char device structure          */
	struct device	*dev;
	int		reserved;
} DOLBYVISIONEDR_DEV;
DOLBYVISIONEDR_DEV *dolbyvisionEDR_devices;          /* allocated in dolby_device_init */

//>>>>> start ,20180131 pinyen, hdmi vsif flow
vfe_hdmi_vsi_t hdmi_dolby_vsi_content;
//<<<<< end ,20180131 pinyen, hdmi vsif flow

#define DRV_NAME        "dolbyvisionEDR"
static const char  drv_name[] = DRV_NAME;

// for DOLBY picture mode

unsigned int  *b05_aligned_vir_addr = NULL;

/*-----------------------------------------------------------------------------+
 |  Configure the DM for NORMAL mode
 +----------------------------------------------------------------------------*/

int dolbyvisionEDR_major = 0;
int dolbyvisionEDR_minor = 0;
int dolbyvisionEDR_nr_devs = 1;
#ifdef CONFIG_HIBERNATION
extern int in_suspend;
#endif
//RTK add fix compile error

void *get_dolby_vision_devices(void)
{
	return (void *)dolbyvisionEDR_devices;
}
#ifdef CONFIG_PM
static struct platform_device *dolbyvisionEDR_devs;

static const struct dev_pm_ops dolbyvisionEDR_pm_ops = {
	.suspend    = dolby_adapter_dolby_vision_suspend,
	.resume     = dolby_adapter_dolby_vision_resume,
#ifdef CONFIG_HIBERNATION
	.freeze     = dolby_adapter_dolby_vision_suspend,
	.thaw       = dolby_adapter_dolby_vision_resume,
	.poweroff   = dolby_adapter_dolby_vision_suspend,
	.restore    = dolby_adapter_dolby_vision_resume,
#endif
};

static struct platform_driver dolbyvisionEDR_driver = {
	.driver = {
		.name         = (char *)drv_name,
		.bus          = &platform_bus_type,
#ifdef CONFIG_PM
		.pm           = &dolbyvisionEDR_pm_ops,
#endif
	},
};
#endif /* CONFIG_PM */

typedef enum e_dv_mode_ {
	DV_DISABLE_MODE = 0x0,
	DV_HDMI_MODE = 0x1,
	DV_DM_CRF_422_MODE = 0x4,
	DV_DM_CRF_420_MODE = 0x5,
	DV_NORMAL_MODE = 0x2,
	DV_COMPOSER_CRF_MODE = 0x6,
} E_DV_MODE;

typedef struct {

	int pxlBdp;     /* output pixel definition as a bit field */
	int RowNumTtl;
	int ColNumTtl;

	int BL_EL_ratio;        /* for normal mode; 0=> 1:1;  1=> 4:1 */

	char file1Inpath[100/*500*/];
	char file2Inpath[100/*500*/];
	char fileOutpath[100/*500*/];
	char fileMdInpath[100/*500*/];

	unsigned long file1InSize;
	unsigned long file2InSize;
	unsigned long fileMdInSize;

	int frameNo;

	E_DV_MODE dolby_mode;
	char caseNum[10];

} DOLBYVISION_PATTERN;
bool is_memc_low_latency(void)
{
    return ((Scaler_MEMC_GetMotionType() == VPQ_MEMC_TYPE_OFF)
            && (fwif_color_get_cinema_mode_en() == 0));
}

int dolbyvisionEDR_open(struct inode *inode, struct file *filp)
{
	return 0;
}
int dolbyvisionEDR_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static int dolby_ambient_weight = 1 << 16;

int get_dolby_ambient_weight(void)
{
	return dolby_ambient_weight;
}

static struct file * file_open(const char *path, int flags, int rights)
{
#ifdef CONFIG_RTK_FEATURE_FOR_GKI
	struct file* filp = NULL;
	int err = 0;

	filp = filp_open(path, flags, rights);
	if(IS_ERR(filp)) {
		err = PTR_ERR(filp);
		return NULL;
	}

	return filp;
#else
	return NULL;
#endif
}

static void file_close(struct file *file)
{
#ifdef CONFIG_RTK_FEATURE_FOR_GKI
	filp_close(file, NULL);
#endif
}


static int file_read(unsigned char *data, unsigned int size, unsigned long long offset, struct file *file)
{
#ifdef CONFIG_RTK_FEATURE_FOR_GKI
	return kernel_read(file, data, size, &offset);
#else
	return 0;
#endif
}

static loff_t get_filesize(struct file *filp)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0))
	loff_t size = i_size_read(file_inode(filp));
#else
	loff_t size = i_size_read(filp->f_path.dentry->d_inode);
#endif

	return size;
}
static long dolby_set_pq_table(char *filepath)
{
	long ret = 0;
	loff_t ppos = 0;
	loff_t filesize;
	void *file_buffer;
	size_t read_bytes;

	struct file *filefp = file_open(filepath , O_RDONLY, 0);
	if (filefp == NULL) {
		rtd_pr_hdr_err("[DolbyVision] file %s open failed\n", filepath);
		goto dolby_set_pq_table_open_file_error;
	}

	filesize = get_filesize(filefp);
	rtd_pr_hdr_err("[DolbyVision] %s size %d\n", filepath, filesize);
	if (filesize <= 0) {
		rtd_pr_hdr_err("[DolbyVision] get file size error\n");
		goto dolby_set_pq_table_get_file_error;
	}

	file_buffer = vmalloc(filesize);
	if (file_buffer == NULL) {
		//rtd_pr_hdr_err("[DolbyVision] alloc buffer %d bytes fail\n", filesize);//fix error
		goto dolby_set_pq_table_get_file_error;
	}

	read_bytes = file_read((unsigned char *)file_buffer, filesize, 0, filefp);

	ret = dolby_adapter_pq_table_bin_write(NULL, file_buffer, read_bytes, &ppos, false);

	vfree(file_buffer);
dolby_set_pq_table_get_file_error:
	file_close(filefp);
dolby_set_pq_table_open_file_error:
	return ret;
}

static int func_dolby_pq_text2bin(unsigned long arg)
{
	int ret = 0;
	struct dolby_vision_pq_text_buf pq_text_buf;

	char *buf_factory = NULL;
	size_t size_factory = 0;
	char *buf_best_pq = NULL;
	size_t size_best_pq = 0;
	char *buf_pq_bin = NULL;
	unsigned long n;

	size_t pq_bin_total_size = -1;


	if (copy_from_user(&pq_text_buf, (struct dolby_vision_pq_text_buf *)arg, sizeof(struct dolby_vision_pq_text_buf))) {
		ret = -EFAULT;
		rtd_pr_hdr_err("dolbyvisionEDR: DOLBYVISIONEDR_DRV_PQ_TEXT2BIN failed!\n");
		goto pq_text2bin_error;
	}
	if (pq_text_buf.size_factory) {
		buf_factory = vmalloc(pq_text_buf.size_factory);
		if (buf_factory == NULL) {
			//rtd_pr_hdr_err("[DolbyVision] allocate memory %04d bytes for factroy config fail\n", pq_text_buf.size_factory);//fix error
			ret = -ENOMEM;
			goto pq_text2bin_error;
		}

		size_factory = pq_text_buf.size_factory;
		n = copy_from_user(buf_factory, pq_text_buf.buf_factory, pq_text_buf.size_factory);
		if (n)
			rtd_pr_hdr_err("[DolbyVision] copy factory cfg error\n");
	}

	if (pq_text_buf.size_best_pq) {
		buf_best_pq = vmalloc(pq_text_buf.size_best_pq);
		if (buf_best_pq == NULL) {
			//rtd_pr_hdr_err("[DolbyVision] allocate memory %04d bytes for best config fail\n", pq_text_buf.size_best_pq);//fix error
			ret = -ENOMEM;
			goto pq_text2bin_best_pq_fail;
		}

		size_best_pq = pq_text_buf.size_best_pq;
		n = copy_from_user(buf_best_pq, pq_text_buf.buf_best_pq, pq_text_buf.size_best_pq);
		if (n)
			rtd_pr_hdr_err("[DolbyVision] copy best cfg error\n");
	}

	buf_pq_bin = dolby_adapter_pq_text2bin(buf_factory, size_factory, buf_best_pq, size_best_pq, &pq_bin_total_size);
	dolby_adapter_clear_view_mode_id();

	if (buf_pq_bin != NULL) {
		if (pq_text_buf.buf_pq_bin != NULL) {
			n = copy_to_user(pq_text_buf.buf_pq_bin, buf_pq_bin, min(pq_text_buf.size_pq_bin, pq_bin_total_size));
			if (n)
				rtd_pr_hdr_err("[DolbyVision] copy pq bin error\n");
			ret = pq_bin_total_size - n;
		}
		vfree(buf_pq_bin);
	} else {
		ret = -1;
	}

	if (buf_best_pq != NULL)
		vfree(buf_best_pq);

pq_text2bin_best_pq_fail:
	if (buf_factory != NULL)
		vfree(buf_factory);

pq_text2bin_error:

	return ret;
}

#ifdef CONFIG_RTK_KDRV_DV_IDK_DUMP
static int32_t force_input_region_mode = 0;

static void set_dolby_viiosn_forece_input_region_mode(int32_t mode)
{
	rtd_pr_hdr_info("[DOLBY] set force input region mode %d\n", mode);
	force_input_region_mode = mode;
}

int32_t get_force_input_region_mode(void)
{
	return force_input_region_mode;
}
#endif

#ifdef CONFIG_ARM64
long quirk_for_arm64(unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	switch (_IOC_NR(cmd)) {
	case 57:
	{
		struct dolby_vision_capability_data data;
		void *capability_table = NULL;
		unsigned long ptr;
		int n;

		if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(struct dolby_vision_capability_data))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: DOLBYVISIONEDR_DRV_CHECK_VIDEO_CAP failed!\n");
			goto QUIRK_DOLBYVISIONEDR_DRV_CHECK_VIDEO_CAP_ERR;
		}
		if (data.size <= 0){
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: DOLBYVISIONEDR_DRV_CHECK_VIDEO_CAP failed!data.size <= 0\n");
			goto QUIRK_DOLBYVISIONEDR_DRV_CHECK_VIDEO_CAP_ERR;
		}
		capability_table = vmalloc(data.size); //&table_data[0]; //vmalloc(data.size);

		if (capability_table == NULL) {
			rtd_pr_hdr_err("dolbyvisionEDR: allocate data error\n");
			ret = -EFAULT;
			goto QUIRK_DOLBYVISIONEDR_DRV_CHECK_VIDEO_CAP_ERR;
		}

		ptr = data.data;
		n = copy_from_user((void *)capability_table, (void *)ptr, data.size);
		if (n) {
			ret = -EFAULT;
			rtd_pr_hdr_err("[DolbyVision][%s:%s:%d]  %d\n", __FILE__, __func__, __LINE__, n);
			goto QUIRK_DOLBYVISIONEDR_DRV_CHECK_VIDEO_CAP_COPY_ERROR;
		}

		dolby_adapter_check_capability(capability_table, data.size);

QUIRK_DOLBYVISIONEDR_DRV_CHECK_VIDEO_CAP_COPY_ERROR:
		vfree(capability_table);
QUIRK_DOLBYVISIONEDR_DRV_CHECK_VIDEO_CAP_ERR:
		rtd_pr_hdr_debug("ioctl DOLBYVISIONEDR_DRV_CHECK_VIDEO_CAP\n");
	}
	break;
	case 58:
	{
		const int hash_size = 32;
		char model_hash[32];
		int n;

		n = copy_from_user((void *)model_hash, (const void __user *)arg, hash_size);
		if (n) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: DOLBYVISIONEDR_DRV_SET_MODEL_HASH failed!\n");
		} else {
			dolby_adapter_set_model_hash((void *)model_hash, hash_size);
		}

	}
	break;
	case 59:
	{
		int8_t content_type[4];
		dolby_adapter_get_l11_content_type(content_type);
		if (copy_to_user((int *)arg, content_type, sizeof(content_type))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!!!!!!!!!!!!!!!\n", _IOC_NR(cmd));
			break;
		}
	}
	break;
	case 63:
	{
		ret = func_dolby_pq_text2bin(arg);
		break;
	}
	default:
	break;
	}
	return ret;
}
#endif

long dolbyvisionEDR_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret =  0;

	if (cmd != DOLBYVISIONEDR_DRV_VPQ_GETDOLBY_WAIT_CFG_PATH)
		rtd_pr_hdr_info("dolbyvisionEDR: dolbyvisionEDR_ioctl, %d\n", _IOC_NR(cmd));
	if (_IOC_TYPE(cmd) != DOLBYVISIONEDR_IOC_MAGIC || _IOC_NR(cmd) > DOLBYVISIONEDR_IOC_MAXNR)
		return -ENOTTY;

	switch (cmd) {

	case DOLBYVISIONEDR_IOC_RB_INIT: {
		DOLBYVISION_INIT_STRUCT data;
		if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(DOLBYVISION_INIT_STRUCT))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!!!!!!!!!!!!!!!\n", _IOC_NR(cmd));
		} else {
			rtd_pr_hdr_debug("data.inband_rbh_addr = 0x%x \n", data.inband_rbh_addr);
			rtd_pr_hdr_debug("data.md_rbh_addr = 0x%x \n", data.md_rbh_addr);
			rtd_pr_hdr_debug("data.md_output_buf_addr = 0x%x \n", data.md_output_buf_addr);
			rtd_pr_hdr_debug("data.md_output_buf_size = 0x%x \n", data.md_output_buf_size);
			ret = dolby_adapter_dolby_vision_ringbuffer_init(&data);
			dolby_adapter_set_g_picModeUpdateFlag(1);
		}
		break;
	}

	case DOLBYVISIONEDR_IOC_RUN: {
		ret = dolby_adapter_dolby_vision_run();
		break;
	}
	case DOLBYVISIONEDR_IOC_STOP: {
		ret = dolby_adapter_dolby_vision_stop();
		break;
	}
	case DOLBYVISIONEDR_IOC_PAUSE: {
		ret = dolby_adapter_dolby_vision_pause();
		break;
	}
	case DOLBYVISIONEDR_IOC_RB_FLUSH: {
		ret = dolby_adapter_dolby_vision_flush();
		break;
	}
	case DOLBYVISIONEDR_HDMI_IN_TEST: {
		rtd_pr_hdr_debug("dolbyvisionEDR: DOLBYVISIONEDR_HDMI_IN_TEST ioctl code!!!!!!!!!!!!!!!\n");
		break;
	}
	case DOLBYVISIONEDR_NORMAL_IN_TEST: {
		rtd_pr_hdr_debug("dolbyvisionEDR: DOLBYVISIONEDR_NORMAL_IN_TEST ioctl code!!!!!!!!!!!!!!!\n");
		break;
	}
	case DOLBYVISIONEDR_DM_CRF_420_TEST: {
		rtd_pr_hdr_debug("dolbyvisionEDR: DOLBYVISIONEDR_DM_CRF_420_TEST ioctl code!!!!!!!!!!!!!!!\n");
		break;
	}
	case DOLBYVISIONEDR_DM_CRF_422_TEST: {
		rtd_pr_hdr_debug("dolbyvisionEDR: DOLBYVISIONEDR_DM_CRF_422_TEST ioctl code!!!!!!!!!!!!!!!\n");
		break;
	}
	case DOLBYVISIONEDR_COMPOSER_CRF_TEST: {
		rtd_pr_hdr_debug("dolbyvisionEDR: DOLBYVISIONEDR_COMPOSER_CRF_TEST ioctl code!!!!!!!!!!!!!!!\n");
		break;
	}
	case DOLBYVISIONEDR_CRF_DUMP_TEST: {
		DOLBYVISION_PATTERN data;

		rtd_pr_hdr_debug("dolbyvisionEDR: DOLBYVISIONEDR_CRF_DUMP_TEST ioctl code!!!!!!!!!!!!!!!\n");

		if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(DOLBYVISION_PATTERN))) {
		        ret = -EFAULT;
		        rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!!!!!!!!!!!!!!!\n", _IOC_NR(cmd));
		} else {

			rtd_pr_hdr_debug("pxlBdp=%d, RowNumTtl=%d, ColNumTtl=%d \n", data.pxlBdp, data.RowNumTtl, data.ColNumTtl);
			rtd_pr_hdr_debug("BL_EL_ratio=%d \n", data.BL_EL_ratio);
			rtd_pr_hdr_debug("file1Inpath=%s \n", data.file1Inpath);
			rtd_pr_hdr_debug("file2Inpath=%s \n", data.file2Inpath);
			rtd_pr_hdr_debug("fileOutpath=%s \n", data.fileOutpath);
			rtd_pr_hdr_debug("fileMdInpath=%s \n", data.fileMdInpath);
			rtd_pr_hdr_debug("file1InSize=%ld \n", data.file1InSize);
			rtd_pr_hdr_debug("file2InSize=%ld \n", data.file2InSize);
			rtd_pr_hdr_debug("fileMdInSize=%ld \n", data.fileMdInSize);
			rtd_pr_hdr_debug("frameNo=%d \n", data.frameNo);
			rtd_pr_hdr_debug("caseNum=%s \n", data.caseNum);
			dolby_adapter_dm_crf_dump_testflow(&data);
		}
		break;
	}
	case DOLBYVISIONEDR_CRF_TRIGGER_TEST: {
		uint32_t mode;

		if (copy_from_user((void *)&mode, (const void __user *)arg, sizeof(uint32_t))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!!!!!!!!!!!!!!!\n", _IOC_NR(cmd));
			return ret;
		}

		switch (mode) {
		case 0:
			dolby_adapter_wait_next_frame();
			break;
		case 1:
			#ifdef CONFIG_RTK_KDRV_DV_IDK_DUMP
			dolby_adapter_wait_dolby_vision_start();
			#endif
			break;
		}
		break;
	}
	case DOLBYVISIONEDR_TEST_TEST: {
		break;
	}
	case DOLBYVISIONEDR_DRV_VPQ_SETDOLBYPICMODE: {
#ifndef CONFIG_RTK_KDRV_DV_IDK_DUMP
		uint8_t pic_mode;
		if (copy_from_user((void *)&pic_mode, (const void __user *)arg, sizeof(uint8_t))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!!!!!!!!!!!!!!!\n", _IOC_NR(cmd));
			return ret;
		}

		//Change picture mode //
		dolby_adapter_g_dv_pq_sem_down();
		dolby_adapter_set_ui_dv_picmode(pic_mode);

		dolby_adapter_g_dv_pq_sem_up();
		dolby_adapter_set_hdmi_ui_change_flag(TRUE);
		rtd_pr_hdr_info("[DOLBY][%s:%d] set picture mode %d -> %d\n", __func__, __LINE__, dolby_adapter_get_ui_dv_picmode(), pic_mode);
#endif
		break;
	}
	case DOLBYVISIONEDR_DRV_VPQ_SETDOLBYBACKLIGHT: {
		UINT8 uBacklight;

		if (copy_from_user((void *)&uBacklight, (const void __user *)arg, sizeof(UINT8))) {
			ret = -EFAULT;
			rtd_pr_hdr_debug("dolbyvisionEDR: ioctl code = %d failed!!!!!!!!!!!!!!!\n", _IOC_NR(cmd));
		} else {
			if (dolby_adapter_get_ui_dv_backlight_value() == uBacklight) {
				rtd_pr_hdr_notice("dolbyvisionEDR: the same backlight value\n");
				break;
			}
			if (uBacklight > 100) {
				rtd_pr_hdr_debug("dolbyvisionEDR: uBacklight=%d is too big !!!!!!!!!!!!!!!\n", uBacklight);
				break;
			}
			dolby_adapter_g_dv_pq_sem_down();
			dolby_adapter_set_ui_dv_backlight_value(uBacklight);
			dolby_adapter_g_dv_pq_sem_up();

			dolby_adapter_set_hdmi_ui_change_flag(TRUE);
			rtd_pr_hdr_notice("\r\nchange back light value %d\n", uBacklight);
		}
		break;
	}
	case DOLBYVISIONEDR_DRV_VPQ_SETDOLBYCONTRAST: {
		UINT8 uContrast;
		if (copy_from_user((void *)&uContrast, (const void __user *)arg, sizeof(UINT8))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!!!!!!!!!!!!!!!\n", _IOC_NR(cmd));
		} else {
			// do nothing
			rtd_pr_hdr_debug("%s: uContrast=%d \n", __func__, uContrast);
		}
		break;
	}
	case DOLBYVISIONEDR_DRV_VPQ_SETDOLBYBRIGHTNESS: {
		UINT8 uBrightness;
		if (copy_from_user((void *)&uBrightness, (const void __user *)arg, sizeof(UINT8))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!!!!!!!!!!!!!!!\n", _IOC_NR(cmd));
		} else {
			// do nothing
			rtd_pr_hdr_debug("%s: uBrightness=%d \n", __func__, uBrightness);
		}
		break;
	}
	case DOLBYVISIONEDR_DRV_VPQ_SETDOLBYCOLOR: {
		UINT8 uColor;
		if (copy_from_user((void *)&uColor, (const void __user *)arg, sizeof(UINT8))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!!!!!!!!!!!!!!!\n", _IOC_NR(cmd));
		}
		break;
	}
	case DOLBYVISIONEDR_DRV_VPQ_INITDOLBYPICCONFIG: {
		char *configFilePath[5] = {NULL};
		long ret = 0;
		if (copy_from_user((void *)&configFilePath, (const void __user *)arg, sizeof(configFilePath))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!!!!!!!!!!!!!!!\n", _IOC_NR(cmd));
		} else {
			char *pq_table_filepath = (char *)kzalloc(PATH_MAX, GFP_KERNEL);

			if (pq_table_filepath == NULL)
				break;

			ret = strncpy_from_user(pq_table_filepath, configFilePath[0], PATH_MAX - 1);
			if(ret<=0)
			{
				kfree(pq_table_filepath);
				ret = -EFAULT;
				rtd_pr_hdr_err("strncpy_from_user fail\n");
				break;
			}
			rtd_pr_hdr_info("%s: pq table path=%s\n", __func__, pq_table_filepath);

			dolby_adapter_g_dv_pq_sem_down();
			//ret = dolby_adapter_dolby_load_pq_file(pq_table_filepath);
			ret = dolby_set_pq_table(pq_table_filepath);
			if (ret == 0) {
				rtd_pr_hdr_err("load dolby vision pq table file fail\n");
				ret = -EFAULT;
			}
			dolby_adapter_g_dv_pq_sem_up();
			kfree(pq_table_filepath);
		}
		break;
	}
	case DOLBYVISIONEDR_DRV_VPQ_GETDOLBYSWVERSION: {
		char sw_version[64];
		int size = sizeof(sw_version);

		memset(sw_version, 0, size);
		strncpy(sw_version, dolby_adapter_get_dolby_vision_software_version(), size - 1);
		if (copy_to_user((void *)arg, sw_version, strlen(sw_version))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed\n", _IOC_NR(cmd));
		} else {
			rtd_pr_hdr_debug("dolbyvisionEDR: ioctl code = %d success\n", _IOC_NR(cmd));
		}
		break;
	}
	case DOLBYVISIONEDR_DRV_SUPPORT_STATUS: {
		unsigned int st = 0x20 | 0x100 | 0x200 | 0x00100000; //dolby_adapter_get_support_status();
		ret = 0;
		if (copy_to_user((void __user *)arg, &st, sizeof(unsigned int))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!!!!!!!!!!!!!!!\n", _IOC_NR(cmd));
		}
		
		break;
	}
	case DOLBYVISIONEDR_DRV_VPQ_GETDOLBYIDKVERSION:
	{
		char idk_version[64];
		int size = sizeof(idk_version);

		ret = 0;

		memset(idk_version, 0, size);

		dolby_adapter_get_dolby_vision_idk_version(idk_version, size);
		if (copy_to_user((void *)arg, idk_version, strlen(idk_version))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!\n", _IOC_NR(cmd));
		}
	}
	break;
	case DOLBYVISIONEDR_DRV_VPQ_GETDOLBYDMVERSION:
	{
		char dm_version[64];
		int size = sizeof(dm_version);

		ret = 0;

		memset(dm_version, 0, size);

		dolby_adapter_get_dolby_vision_dm_version(dm_version, size);
		if (copy_to_user((void *)arg, dm_version, strlen(dm_version))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!\n", _IOC_NR(cmd));
		}
	}
	break;
	case DOLBYVISIONEDR_DRV_VPQ_GET_CURRENT_GD_DELAY:
	{
		int gd_delay = dolby_adapter_get_current_gd_delay();

		ret = 0;
		if (copy_to_user((int *)arg, &gd_delay, sizeof(gd_delay))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!\n", _IOC_NR(cmd));
		}
	}
	break;
	case DOLBYVISIONEDR_DRV_VPQ_GET_GD_DELAY_TABLE:
	{
		size_t size;
		const uint8_t *ptr = dolby_adapter_get_gd_delay_table(&size);

		ret = 0;
		if (copy_to_user((uint8_t *)arg, ptr, size)) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!\n", _IOC_NR(cmd));
		}
	}
	break;
	case DOLBYVISIONEDR_DRV_VPQ_GET_CURRENT_TARGET_CONFIG:
	{
		size_t size;
		const uint8_t *ptr = dolby_adapter_get_current_target_config(&size);

		ret = 0;
		if (copy_to_user((uint8_t *)arg, ptr, size)) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!\n", _IOC_NR(cmd));
		}
	}
	break;
	case DOLBYVISIONEDR_DRV_VPQ_GETDOLBYPICMODE:
	{
		int pic_mode = dolby_adapter_get_ui_dv_picmode();
		if (copy_to_user((int *)arg, &pic_mode, sizeof(int))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!\n", _IOC_NR(cmd));
		}
		break;
	}
	case DOLBYVISIONEDR_DRV_VPQ_GETDOLBYBACKLIGHT:
	{
		int backlight = dolby_adapter_get_ui_dv_backlight_value();
		if (copy_to_user((int *)arg, &backlight, sizeof(int))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!\n", _IOC_NR(cmd));
		}
		break;

	}
	case DOLBYVISIONEDR_DRV_VPQ_GETDOLBYCONTRAST:
	{
		int contrast = dolby_adapter_get_ui_dv_contrast_value();
		if (copy_to_user((int *)arg, &contrast, sizeof(int))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!\n", _IOC_NR(cmd));
		}
		break;
	}
	case DOLBYVISIONEDR_DRV_VPQ_GETDOLBYBRIGHTNESS:
	{
		int brightness = dolby_adapter_get_ui_dv_brightness_value();
		if (copy_to_user((int *)arg, &brightness, sizeof(int))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!\n", _IOC_NR(cmd));
		}
		break;
	}
	case DOLBYVISIONEDR_DRV_VPQ_GETDOLBY_CFG_PATH:
	{
		char cfg_path[128];
		int size = sizeof(cfg_path);

		ret = 0;

		memset(cfg_path, 0, size);

		dolby_adapter_get_pq_table_cfg_path(cfg_path, size);
		if (copy_to_user((void *)arg, cfg_path, strlen(cfg_path))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!\n", _IOC_NR(cmd));
		}
	}
	break;
	case DOLBYVISIONEDR_DRV_VPQ_GETDOLBY_RUNNING_MODE:
	{
		int running_mode = dolby_adapter_get_running_mode();
		if (copy_to_user((int *)arg, &running_mode, sizeof(int))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!\n", _IOC_NR(cmd));
		}
		break;
	}
	case DOLBYVISIONEDR_DRV_VPQ_GETDOLBY_RESOLUTION:
	{
		int resolution[2] = { 0 };
		dolby_adapter_get_resolution(resolution);
		if (copy_to_user((int *)arg, resolution, sizeof(resolution))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!\n", _IOC_NR(cmd));
		}
		break;
	}
	case DOLBYVISIONEDR_DRV_VPQ_GETDOLBY_WAIT_CFG_PATH:
	{
		char cfg_path[128];
		int size = sizeof(cfg_path);

		ret = 0;

		memset(cfg_path, 0, size);

		ret = dolby_adapter_wait_pq_table_cfg_path(cfg_path, size);
		if (ret < 0) {
			break;
		}

		if (copy_to_user((void *)arg, cfg_path, size)) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!\n", _IOC_NR(cmd));
		}
	}
	break;
	case DOLBYVISIONEDR_DRV_VPQ_SETDOLBY_CFG_PATH:
	{
		char cfg_path[128];
		char **path = kzalloc(sizeof(char *) * 3, GFP_KERNEL);
		int size = sizeof(cfg_path);

		if (path == NULL) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: allocate path error\n");
			break;
		}

		ret = 0;

		memset(cfg_path, 0, size);
		if (copy_from_user(cfg_path, (void *)arg, size)) {
			ret = -EFAULT;
			kfree(path);
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!\n", _IOC_NR(cmd));
			break;
		}

		path[0] = cfg_path;
		ret = dolby_adapter_set_pq_table_cfg_path(path, 3);
		if (ret < 0) {
			ret = -EFAULT;
		}

		kfree(path);

	}
	break;
	case DOLBYVISIONEDR_DRV_VPQ_SET_DOLBY_AMBIENT_MODE:
	{
		int mode = (int)arg;

		dolby_ambient_weight = mode;
		dolby_adapter_set_ambient_mode(mode);
	}
	break;
	case DOLBYVISIONEDR_DRV_VPQ_GET_DOLBY_AMBIENT_MODE:
	{
		int mode = dolby_adapter_get_ambient_mode();
		if (copy_to_user((int *)arg, &mode, sizeof(int))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!\n", _IOC_NR(cmd));
		}
	}
	break;
	case DOLBYVISIONEDR_DRV_VPQ_SET_DOLBY_AMBIENT_FRONT:
	{
		int64_t tFrontLux;
		if (copy_from_user((void *)&tFrontLux, (const void __user *)arg, sizeof(tFrontLux))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!!!!!!!!!!!!!!!\n", _IOC_NR(cmd));
			break;
		}

		dolby_adapter_set_ambient_front(tFrontLux);
	}
	break;
	case DOLBYVISIONEDR_DRV_VPQ_GET_DOLBY_AMBIENT_FRONT:
	{
		int64_t tFrontLux = dolby_adapter_get_ambient_front();
		if (copy_to_user((int *)arg, &tFrontLux, sizeof(tFrontLux))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!\n", _IOC_NR(cmd));
		}
	}
	break;
	case DOLBYVISIONEDR_DRV_VPQ_SET_DOLBY_AMBIENT_REAR:
	{
		int64_t tRearLum;
		if (copy_from_user((void *)&tRearLum, (const void __user *)arg, sizeof(tRearLum))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!!!!!!!!!!!!!!!\n", _IOC_NR(cmd));
			break;
		}

		dolby_adapter_set_ambient_rear(tRearLum);
	}
	break;
	case DOLBYVISIONEDR_DRV_VPQ_GET_DOLBY_AMBIENT_REAR:
	{
		int64_t tRearLum = dolby_adapter_get_ambient_rear();
		if (copy_to_user((int *)arg, &tRearLum, sizeof(tRearLum))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!\n", _IOC_NR(cmd));
		}
	}
	break;
	case DOLBYVISIONEDR_DRV_VPQ_SET_DOLBY_AMBIENT_WHITEXY:
	{
		int tWhiteXY[2];
		if (copy_from_user((void *)tWhiteXY, (const void __user *)arg, sizeof(tWhiteXY))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!!!!!!!!!!!!!!!\n", _IOC_NR(cmd));
			break;
		}

		dolby_adapter_set_ambient_whitexy(tWhiteXY[0], tWhiteXY[1]);
	}
	break;
	case DOLBYVISIONEDR_DRV_VPQ_GET_DOLBY_AMBIENT_WHITEXY:
	{
		int tWhiteXY[2];
		dolby_adapter_get_ambient_whitexy(&tWhiteXY[0], &tWhiteXY[1]);
		if (copy_to_user((int *)arg, tWhiteXY, sizeof(tWhiteXY))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!\n", _IOC_NR(cmd));
		}
	}
	break;
	case DOLBYVISIONEDR_DRV_VPQ_CLEAR_VIEW_MODE_ID:
	{
		dolby_adapter_clear_view_mode_id();
	}
	break;
	case DOLBYVISIONEDR_DRV_VPQ_SET_BLOCK_AMBIENT_CALL:
	{
		bool enable = (bool) arg;

		dolby_adapter_set_block_ambient_call(enable);
	}
	break;
	case DOLBYVISIONEDR_DRV_VPQ_GET_BLOCK_AMBIENT_CALL:
	{
		bool enable = dolby_adapter_get_block_ambient_call();
		if (copy_to_user((bool *)arg, &enable, sizeof(enable))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!\n", _IOC_NR(cmd));
		}
	}
	break;
	case DOLBYVISIONEDR_DRV_VPQ_SET_DOLBY_AMBIENT_FRONT_FORCE:
	{
		int64_t tFrontLux;
		if (copy_from_user((void *)&tFrontLux, (const void __user *)arg, sizeof(tFrontLux))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!!!!!!!!!!!!!!!\n", _IOC_NR(cmd));
			break;
		}

		dolby_adapter_set_ambient_front_force(tFrontLux);
	}
	break;
	case DOLBYVISIONEDR_DRV_VPQ_SET_DOLBY_AMBIENT_MODE_FORCE:
	{
		int mode = arg;

		dolby_ambient_weight = mode;
		dolby_adapter_set_ambient_mode_force(mode);
	}
	break;
	case DOLBYVISIONEDR_DRV_VPQ_SET_DOLBY_AMBIENT_FORNT_LUX:
	{
		int lux = (int)arg;

		dolby_adapter_set_ambient_front_lux(lux);
	}
	break;
	case DOLBYVISIONEDR_DRV_VPQ_GET_DOLBY_AMBIENT_FRONT_LUX:
	{
		int lux = dolby_adapter_get_ambient_front_lux();

		if (copy_to_user((int *)arg, &lux, sizeof(lux))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!\n", _IOC_NR(cmd));
		}
	}
	break;
    case DOLBYVISIONEDR_DRV_VPQ_SET_DOLBY_GD_DELAY_BASE_HDMI:
	{
		int delay = (int)arg;
		dolby_adapter_set_gd_delay_base_hdmi(delay);
	}
	break;
	case DOLBYVISIONEDR_DRV_VPQ_SET_DOLBY_GD_DELAY_BASE_OTT:
	{
		int delay = (int)arg;
		dolby_adapter_set_gd_delay_base_ott(delay);
	}
	break;
	case DOLBYVISIONEDR_DRV_VPQ_SET_DOLBY_GD_DELAY_BASE_HDMILL:
	{
		int delay = (int)arg;
		dolby_adapter_set_gd_delay_base_hdmi_ll(delay);
	}
	break;
    case DOLBYVISIONEDR_DRV_SET_OTT_PAUSE:
	{
		bool enable = (bool)arg;
		dolby_adapater_set_ott_pause(enable);
	}
	break;
	case DOLBYVISIONEDR_DRV_GET_OTT_PAUSE_STATUS:
	{
		bool enable = dolby_adapater_get_ott_pause_status();

		if (copy_to_user((bool *)arg, &enable, sizeof(bool))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!\n", _IOC_NR(cmd));
		}
	}
	break;
	case DOLBYVISIONEDR_DRV_CHECK_VIDEO_CAP:
	{
		struct dolby_vision_capability_data data;
		void *capability_table;
		unsigned long ptr;

		if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(struct dolby_vision_capability_data))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: DOLBYVISIONEDR_DRV_CHECK_VIDEO_CAP failed!\n");
			goto DOLBYVISIONEDR_DRV_CHECK_VIDEO_CAP_ERR;
		}
		if (data.size <= 0){
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: DOLBYVISIONEDR_DRV_CHECK_VIDEO_CAP failed!\n");
			goto DOLBYVISIONEDR_DRV_CHECK_VIDEO_CAP_ERR;
		}		
		capability_table = kzalloc(data.size, GFP_KERNEL);

		if (capability_table == NULL) {
			rtd_pr_hdr_err("dolbyvisionEDR: allocate data error\n");
			ret = -EFAULT;
			goto DOLBYVISIONEDR_DRV_CHECK_VIDEO_CAP_ERR;
		}

		ptr = data.data;
		if (copy_from_user((void *)capability_table, (void *)ptr, data.size)) {
			ret = -EFAULT;
			rtd_pr_hdr_err("[DolbyVision][%s:%s:%d]\n", __FILE__, __func__, __LINE__);
			goto DOLBYVISIONEDR_DRV_CHECK_VIDEO_CAP_COPY_ERROR;
		}

		dolby_adapter_check_capability(capability_table, data.size);

DOLBYVISIONEDR_DRV_CHECK_VIDEO_CAP_COPY_ERROR:
		kfree(capability_table);
DOLBYVISIONEDR_DRV_CHECK_VIDEO_CAP_ERR:
		rtd_pr_hdr_debug("ioctl DOLBYVISIONEDR_DRV_CHECK_VIDEO_CAP\n");
	}
	break;
	case DOLBYVISIONEDR_DRV_SET_MODEL_HASH:
	{
		const int hash_size = 32;
		char model_hash[32];

		if (copy_from_user((void *)model_hash, (const void __user *)arg, hash_size)) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: DOLBYVISIONEDR_DRV_SET_MODEL_HASH failed!\n");
		} else {
			dolby_adapter_set_model_hash((void *)model_hash, hash_size);
		}
	}
	break;
	case DOLBYVISIONEDR_DRV_GET_CONTENT_TYPE:
	{
		int8_t content_type[4];
		dolby_adapter_get_l11_content_type(content_type);
		if (copy_to_user((int *)arg, content_type, sizeof(content_type))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!!!!!!!!!!!!!!!\n", _IOC_NR(cmd));
			break;
		}
	}
	break;
	case DOLBYVISIONEDR_DRV_PQ_TEXT2BIN:
	{
		ret = func_dolby_pq_text2bin(arg);
		break;
	}
	case DOLBYVISIONEDR_DRV_SET_GDBS_DELAY_MATRIX_TABLE:
	{
		struct dolby_vision_gdbs_delay_data table;
		rtd_pr_hdr_info("====== DOLBYVISIONEDR_DRV_SET_GDBS_DELAY_MATRIX_TABLE  =====\n");

                if (copy_from_user((void *)&table, (const void __user *)arg, sizeof(struct dolby_vision_gdbs_delay_data))) {
                        ret = -EFAULT;
                        rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!!!!!!!!!!!!!!!\n", _IOC_NR(cmd));
                } else {
			rtd_pr_hdr_info("[DolbyVision] DOLBYVISIONEDR_DRV_SET_GDBS_DELAY_MATRIX_TABLE, FHD delay=%d, %d, %d, %d  =====\n",
					table.delay_ott_fhd_fps_24,
					table.delay_ott_fhd_fps_30,
					table.delay_ott_fhd_fps_50,
					table.delay_ott_fhd_fps_60);
			rtd_pr_hdr_info("[DolbyVision] DOLBYVISIONEDR_DRV_SET_GDBS_DELAY_MATRIX_TABLE, UHD delay=%d, %d, %d, %d  =====\n",
					table.delay_ott_uhd_fps_24,
					table.delay_ott_uhd_fps_30,
					table.delay_ott_uhd_fps_50,
					table.delay_ott_uhd_fps_60);
			rtd_pr_hdr_info("[DolbyVision] DOLBYVISIONEDR_DRV_SET_GDBS_DELAY_MATRIX_TABLE, HDMI delay=%d, %d, %d, %d  =====\n",
					table.delay_hdmi_fps_24,
					table.delay_hdmi_fps_30,
					table.delay_hdmi_fps_50,
					table.delay_hdmi_fps_60);

			set_gdbs_delay_matrix_table(table);
                }
	}
	break;
	case DOLBYVISIONEDR_DRV_ENABLE_PWM:
	{
		bool enable = (bool)arg;
		enabe_doolby_vision_pwm_func(enable);
	}
	break;
	case DOLBYVISIONEDR_DRV_SET_DARK_DETAIL:
	{
		bool enable = (bool)arg;
		dolby_adapter_set_dark_detail(enable);
	}
	break;
#ifdef CONFIG_RTK_KDRV_DV_IDK_DUMP
	case DOLBYVISIONEDR_DRV_SET_SIGNAL_FORMAT:
	{
		extern void set_signal_format(unsigned int format);

		unsigned int foramt = (unsigned int )arg;
		set_signal_format(foramt);
	}
	break;
	case DOLBYVISIONEDR_DRV_SET_DUMP_DM_REGISTER:
	{
		extern void set_dump_dm_register(bool enable);

		bool enable = (bool)arg;
		set_dump_dm_register(enable);
		dolby_adapter_clear_dump_dm_register_counter();
	}
	break;
	case DOLBYVISIONEDR_DRV_SET_FORCE_DOLBY:
	{
		extern void set_force_dolby(bool enable);

		bool enable = (bool)arg;
		set_force_dolby(enable);
	}
	break;
	case DOLBYVISIONEDR_DRV_SET_BIT_DEPTH:
	{
		extern void set_bit_depth(unsigned int depth);

		unsigned int depth = (unsigned int )arg;
		set_bit_depth(depth);
	}
	break;
	case DOLBYVISIONEDR_DRV_SET_SRC_COLOR_FORMAT:
	{
		extern void set_dm_src_color_format(unsigned int format);

		unsigned int format = (unsigned int )arg;
		set_dm_src_color_format(format);
	}
	break;
	case DOLBYVISIONEDR_DRV_GET_DM_DATA:
	{
		extern void dolby_adapter_get_dm_data(void *p_comp_reg, void *p_dm_reg, void *p_dm_lut, bool is_user_buffer);

		int n;

		void *dm_data[3];
		n = copy_from_user(dm_data, (void *)arg, sizeof(dm_data));
		if (n) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!!!!!!!!!!!!!!!\n", _IOC_NR(cmd));
			break;
		}

		dolby_adapter_get_dm_data(dm_data[0], dm_data[1], dm_data[2], true);
	}
	break;
	case DOLBYVISIONEDR_DRV_GET_DM_STRUCTURE_SIZE:
	{
		extern void dolby_adapter_get_dm_struct_size(int *size_composer, int *size_dm_register, int *size_dm_lut);

		int n;
		int size_composer = 0;
		int size_dm_register = 0;
		int size_dm_lut = 0;
		int32_t dm_size[3];

		dolby_adapter_get_dm_struct_size(&size_composer, &size_dm_register, &size_dm_lut);
		dm_size[0] = size_composer;
		dm_size[1] = size_dm_register;
		dm_size[2] = size_dm_lut;
		rtd_pr_hdr_info("[DOLBY][%s:%d] size %d %d %d\n", __func__, __LINE__, dm_size[0], dm_size[1], dm_size[2]);

		n = copy_to_user((void *)arg, dm_size, sizeof(dm_size));
		if (n) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!!!!!!!!!!!!!!!\n", _IOC_NR(cmd));
			break;
		}
	}
	break;
	case DOLBYVISIONEDR_DRV_VPQ_SETDOLBYPICMODE_FORCE: {
		uint8_t pic_mode;
		if (copy_from_user((void *)&pic_mode, (const void __user *)arg, sizeof(uint8_t))) {
			ret = -EFAULT;
			rtd_pr_hdr_err("dolbyvisionEDR: ioctl code = %d failed!!!!!!!!!!!!!!!\n", _IOC_NR(cmd));
			return ret;
		}

		//Change picture mode //
		dolby_adapter_g_dv_pq_sem_down();
		dolby_adapter_set_ui_dv_picmode(pic_mode);

		dolby_adapter_g_dv_pq_sem_up();
		dolby_adapter_set_hdmi_ui_change_flag(TRUE);
		rtd_pr_hdr_info("[DOLBY][%s:%d] set picture mode %d -> %d\n", __func__, __LINE__, dolby_adapter_get_ui_dv_picmode(), pic_mode);
		break;
	}
	case DOLBYVISIONEDR_DRV_CALL_FORCE_DOLBY_VISION_HDMI:
	{
		dolby_adapter_HDMI_TEST(1920, 1080, NULL);
		break;
	}
	case DOLBYVISIONEDR_DRV_CALL_FORCE_DOLBY_VISION_OTT:
	{
		dolby_adapter_force_run_ott();
		break;
	}
	case DOLBYVISIONEDR_DRV_SET_CRF_AMBIENT_FLAG:
	{

		bool enable = (bool)arg;
		dolby_adapter_set_update_crf_ambient_flag(enable);
		break;
	}
	case DOLBYVISIONEDR_DRV_SET_FORCE_INPUT_REGION_MODE:
	{

		int32_t mode = (int32_t)arg;
		set_dolby_viiosn_forece_input_region_mode(mode);
		break;
	}
	case DOLBYVISIONEDR_DRV_SET_CRF_DARK_DETAIL_FLAG:
	{

		bool enable = (bool)arg;
		dolby_adapter_set_update_crf_dark_detail_flag(enable);
		break;
	}
	case DOLBYVISIONEDR_DRV_SET_CRF_DTV_FLAG:
	{

		bool enable = (bool)arg;
		dolby_adapter_set_update_crf_dtv_flag(enable);
		break;
	}
	case DOLBYVISIONEDR_DRV_SET_CRF_BEST_PQ_FLAG:
	{

		bool enable = (bool)arg;
		dolby_adapter_set_update_crf_best_pq_flag(enable);
		break;
	}
#endif
	case DOLBYVISIONEDR_DRV_SET_CFG_BIN:
	{
		struct dolby_vision_cfg_bin data;
		loff_t ppos = 0;
		if (copy_from_user(&data, (struct dolby_vision_cfg_bin *)arg, sizeof(struct dolby_vision_cfg_bin))) {
			rtd_pr_hdr_err("[DolbyVision] get cfg bin from userspace error\n");
		} else {
			ssize_t write_bytes = dolby_adapter_pq_table_bin_write(NULL, data.data, data.size, &ppos, true);
			rtd_pr_hdr_info("[DolbyVision] write dolby vision cfg size %d\n", data.size);
			if (write_bytes != (ssize_t)data.size)
				rtd_pr_hdr_err("[DolbyVision] write dolby vision cfg fail\n");
		}
	}
	break;
	default:
#ifdef CONFIG_ARM64
		quirk_for_arm64(cmd, arg);
#endif
		rtd_pr_hdr_debug("dolbyvisionEDR: ioctl code = %d is invalid!!!!!!!!!!!!!!!\n", _IOC_NR(cmd));
		break;
	}

	return ret;
}

ssize_t dolbyvisionEDR_write(struct file *filp, const char __user *buffer, size_t count, loff_t *ppos)
{
	//rtd_pr_hdr_info("[%s:%d][Dolby] get buffer count = %d offset = %lld\n", __func__, __LINE__, count, *ppos);//fix error
	return dolby_adapter_pq_table_bin_write((void *)filp, buffer, count, ppos, true);
}

struct file_operations dolbyvisionEDR_fops
	= {
	.owner    =    THIS_MODULE,
	.unlocked_ioctl    = dolbyvisionEDR_ioctl,
	.compat_ioctl      = dolbyvisionEDR_ioctl,
	.open     =    dolbyvisionEDR_open,
	.release  =    dolbyvisionEDR_release,
	.write    =    dolbyvisionEDR_write,
};

static void dolbyvisionEDR_setup_cdev(DOLBYVISIONEDR_DEV *dev, int index)
{
	int err, devno = MKDEV(dolbyvisionEDR_major, dolbyvisionEDR_minor + index);

	cdev_init(&dev->cdev, &dolbyvisionEDR_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops   = &dolbyvisionEDR_fops;
	err = cdev_add(&dev->cdev, devno, 1);

	if (err)
		rtd_pr_hdr_err("Error %d adding se%d", err, index);

	device_create(dolbyvisionEDR_class, NULL, MKDEV(dolbyvisionEDR_major, index), NULL, "dolbyvisionEDR%d", index);
}

static char *dolbyvisionEDR_devnode(struct device *dev, umode_t *mode)
{
	*mode = 0666;//set Permissions
	return NULL;
}

void dolby_device_exit(void)
{
	int i = 0;
	dev_t devno;

	release_dolby_vision_isr(get_dolby_vision_devices());
	remove_dolby_debugfs();
	devno = MKDEV(dolbyvisionEDR_major, dolbyvisionEDR_minor);

	rtd_pr_hdr_info("dolbyvisionEDR clean module dolbyvisionEDR_major = %d\n", dolbyvisionEDR_major);

	if (dolbyvisionEDR_devices) {
		for (i = 0; i < dolbyvisionEDR_nr_devs; i++) {
			cdev_del(&dolbyvisionEDR_devices[i].cdev);
			device_destroy(dolbyvisionEDR_class, MKDEV(dolbyvisionEDR_major, i));
		}
		kfree(dolbyvisionEDR_devices);
	}


	class_destroy(dolbyvisionEDR_class);

	/* cleanup_module is never called if registering failed */
	unregister_chrdev_region(devno, dolbyvisionEDR_nr_devs);

}

static int dolbyvisionEDR_register_device(DOLBYVISIONEDR_DEV *dolbyvisionEDR)
{
	dev_t dev = 0;
	int result, i;

	if (dolbyvisionEDR_major) {
		dev = MKDEV(dolbyvisionEDR_major, dolbyvisionEDR_minor);
		result = register_chrdev_region(dev, dolbyvisionEDR_nr_devs, "dolbyvisionEDR");
	} else {
		result = alloc_chrdev_region(&dev, dolbyvisionEDR_minor, dolbyvisionEDR_nr_devs, "dolbyvisionEDR");
		dolbyvisionEDR_major = MAJOR(dev);
	}

	if (result < 0) {
		rtd_pr_hdr_err("dolbyvisionEDR: can't get major %d\n", dolbyvisionEDR_major);
		return result;
	}

	rtd_pr_hdr_info("dolbyvisionEDR init module major number = %d\n", dolbyvisionEDR_major);

	dolbyvisionEDR_class = class_create(THIS_MODULE, "dolbyvisionEDR");
	if (IS_ERR((dolbyvisionEDR_class)))
		return PTR_ERR(dolbyvisionEDR_class);

	dolbyvisionEDR_class->devnode = dolbyvisionEDR_devnode;

	dolbyvisionEDR_devices = kmalloc(dolbyvisionEDR_nr_devs * sizeof(DOLBYVISIONEDR_DEV), GFP_KERNEL);
	if (!(dolbyvisionEDR_devices)) {
		result = -ENOMEM;
		dolby_device_exit();   /* fail */
		return result;
	}

#ifdef CONFIG_PM
	dolbyvisionEDR_devs = platform_device_register_simple(DRV_NAME, -1, NULL, 0);
	result = platform_driver_register(&dolbyvisionEDR_driver);
	if ((result) != 0) {
		rtd_pr_hdr_err("Can't register dolbyvisionEDR device driver...\n");
	} else {
		rtd_pr_hdr_info("register dolbyvisionEDR device driver...\n");
	}
#endif

	memset(dolbyvisionEDR_devices, 0, dolbyvisionEDR_nr_devs * sizeof(DOLBYVISIONEDR_DEV));

	for (i = 0; i < dolbyvisionEDR_nr_devs; i++) {
		dolbyvisionEDR = &dolbyvisionEDR_devices[i];
		dolbyvisionEDR_setup_cdev(dolbyvisionEDR, i);
	}

#ifdef CONFIG_PM
	dolbyvisionEDR_devices->dev = &(dolbyvisionEDR_devs->dev);
	dolbyvisionEDR_devices->dev->dma_mask = &dolbyvisionEDR_devices->dev->coherent_dma_mask;
	if (dma_set_mask(dolbyvisionEDR_devices->dev, DMA_BIT_MASK(32))) {
		rtd_pr_hdr_info("[dolbyvisionEDR] DMA not supported\n");
	}
#endif

	dev = MKDEV(dolbyvisionEDR_major, dolbyvisionEDR_minor + dolbyvisionEDR_nr_devs);

	return result;
}

static void dolby_vision_worker(struct work_struct *work)
{
	dolby_adapter_worker();
}

static DECLARE_WORK(dolby_vision_work, dolby_vision_worker);

struct workqueue_struct *dolby_vision_wq;
void trigger_dolby_vision_worker(void)
{
	if (dolby_vision_wq != NULL)
		queue_work(dolby_vision_wq, &dolby_vision_work);
}

int dolby_device_init(void)
{
	DOLBYVISIONEDR_DEV *dolbyvisionEDR = &dolbyvisionEDR_devices[0];

	rtd_pr_hdr_info("[%s:%s] Dolby Vision dirver\n", __FILE__, __func__);

	if (dolbyvisionEDR_register_device(dolbyvisionEDR) < 0) {
		return -1;
	}

	create_dolby_debugfs();
	dolby_vision_wq = alloc_workqueue("events_dolby_vision", WQ_HIGHPRI | WQ_UNBOUND, 0);
	register_dolby_vision_isr(get_dolby_vision_devices());
	rtd_pr_hdr_info("dolbyvisionEDR module_init finish\n");

	initial_timer_work();

	return 0;
}

#ifdef CONFIG_SUPPORT_SCALER_MODULE
// the module init/exit will be moved to scaler_module.c if scaler was built as a kernel module
#else
module_init(dolby_device_init);
module_exit(dolby_device_exit);
#endif
MODULE_LICENSE("GPL");
