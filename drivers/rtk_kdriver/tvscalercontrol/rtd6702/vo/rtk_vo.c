/* Kernel Header file */
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>		/* everything... */
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/string.h>	/* memset */
#include <linux/uaccess.h>    /* copy_from_user */
#include <linux/slab.h>   /* kmalloc */
#include <rtk_kdriver/RPCDriver.h>   /* register_kernel_rpc, send_rpc_command */
#include <rtk_kdriver/rtk_vdec.h>
#include <asm/cacheflush.h>
//#include <rtk_kdriver/io.h>
#include <mach/platform.h>

#include "rtk_vo.h"
#include "../scaler_vscdev.h"
#include <tvscalercontrol/scalerdrv/scalerdrv.h> /* Scaler_GetShareMemVirAddr */
#include <tvscalercontrol/scalerdrv/zoom_smoothtoggle_lib.h>
#include <tvscalercontrol/scalerdrv/zoom_smoothtoggle.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/scalerdrv/scaler_hdr_ctrl.h>

// [FILM] film detect
#include <rbus/mdomain_disp_reg.h>

#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <VideoRPC_System.h>
	#include <VideoRPC_System_data.h>
#else
#include <rpc/VideoRPC_System.h>
#include <rpc/VideoRPC_System_data.h>
#endif


#include <linux/pageremap.h> /* dvr_malloc */
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
#include <linux/dma-map-ops.h>
#else
#include <linux/dma-contiguous.h>
#endif
#include <rbus/hdr_all_top_reg.h>
#ifdef CONFIG_RTK_KDRV_DV
#ifdef	CONFIG_DOLBY_IDK_1_6_1_1
#include <dolby_adapter.h>
#else
#include <scaler/dolbyvisionEDR_export.h>
#endif
#endif
#include <rtk_kdriver/rtk_crt.h>
#include <scaler_vbedev.h>
#include <rbus/sb2_reg.h>
#include <rbus/timer_reg.h>
#include <tvscalercontrol/vip/scalerColor_tv006.h>
#include <tvscalercontrol/i3ddma/i3ddma.h>
#include <rtd_log/rtd_module_log.h>
// for register dump
#include <tvscalercontrol/io/ioregdrv.h>

#ifdef ENABLE_DUMP_FRAME_INFO
  // VO frame CRC dump
  #ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <VideoRPC_System.h>
  #else
	#include <rpc/VideoRPC_System.h>
  #endif
  #include <linux/kthread.h>
#ifdef CONFIG_RTK_FEATURE_FOR_GKI
static struct file * file_open(const char *path, int flags, int rights)
{
    struct file* filp = NULL;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
    mm_segment_t oldfs;
    oldfs = get_fs();
    set_fs(KERNEL_DS);
#endif
    filp = filp_open(path, flags, rights);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
    set_fs(oldfs);
#endif
    if(IS_ERR(filp)) {
        return NULL;
    }
    return filp;
}

static void file_close(struct file *file)
{
    filp_close(file, NULL);
}
#ifndef CONFIG_KDRIVER_USE_NEW_COMMON

static int file_write(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size)
{
#ifndef DISABLE_FILE_RW
    int ret;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
    mm_segment_t oldfs;
    oldfs = get_fs();
    set_fs(KERNEL_DS);
#endif

#ifdef CONFIG_SUPPORT_SCALER_MODULE
    ret = kernel_write(file, data, size, &offset);
#else
    ret = vfs_write(file, data, size, &offset);
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
    set_fs(oldfs);
#endif
    return ret;
#else
	return 0;
#endif
}
#endif
#else
static struct file * file_open(const char *path, int flags, int rights)
{
	return NULL;
}

static void file_close(struct file *file)
{
}
#ifndef CONFIG_KDRIVER_USE_NEW_COMMON
static int file_write(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size)
{
	return 0;
}
#endif
#endif
  static void rtkvo_dump_init(void);
#endif

#undef rtd_outl
#define rtd_outl(x, y)     								IoReg_Write32(x,y)
#undef rtd_inl
#define rtd_inl(x)     									IoReg_Read32(x)
#undef rtd_maskl
#define rtd_maskl(x, y, z)     							IoReg_Mask32(x,y,z)
#undef rtd_setbits
#define rtd_setbits(offset, Mask) rtd_outl(offset, (rtd_inl(offset) | Mask))
#undef rtd_clearbits
#define rtd_clearbits(offset, Mask) rtd_outl(offset, ((rtd_inl(offset) & ~(Mask))))

/* //=================== Hardware semaphore =============== // */
/*
#define SB2_HD_SEM_NEW_5_reg 0xB801A634
extern char moduleName[32];

#define SB2_HD_SEM_NEW_5_reg 0xB801A634
#define Get_HD_SEM_NEW_5() while(!IoReg_Read32(SB2_HD_SEM_NEW_5_reg)){rtd_pr_vo_debug("#@# %s wait SB2_HD_SEM_NEW_5_reg ready\n", moduleName); msleep(20);}
#define Set_HD_SEM_NEW_5() IoReg_Write32(SB2_HD_SEM_NEW_5_reg, 0);
*/

/*
//#define Get_HD_SEM_NEW_5() do { while (!rtd_inl(SB2_HD_SEM_NEW_5_reg)); } while (0)
//#define Set_HD_SEM_NEW_5() rtd_outl(SB2_HD_SEM_NEW_5_reg, 0);
*/

/*================= IB =============== */
#define IBU_SEQ_BOUND_2_reg                                 0xb805c068

#define IBU_SEQ_REGION_reg                                  0xb805c084
#define IBU_SEQ_REGION_get_region_2_mode(data)              ((0x00007000&(data))>>12)
#define IBU_SEQ_REGION_get_region_3_mode(data)              ((0x00000070&(data))>>4)

#define MODE_DC1       0x0
#define MODE_DC2       0x1
#define MODE_INTER11   0x2
#define MODE_INTER12   0x4
#define MODE_INTER21   0x5
/*==================================== */
#define VOABS(x)   ((x) >= 0 ? (x) : -(x))

#define IVS_START 4
#define IVS_WIDTH 2

static unsigned char force_change_vo_flag[2] = {FALSE, FALSE}; /* If TRUE also need to set VO change flag */
static struct semaphore VO_InfoSemaphore;
static struct semaphore VO_MemChunkSemaphore;
unsigned char vo_info_change_flag[2] = {FALSE, FALSE};
unsigned char voinfo_flag;

#define DIRECT_VO_STOP 0xff

VO_DATA *vodma_data; /* allocated in vo_module_init */

unsigned char photo_vo_overscan_flag;
unsigned int photo_h_total;
unsigned int photo_v_total;

FILM_SHAREMEM_PARAM *film_mode_param = NULL;

static StructDisplayInfo VO_dispinfo[2];
unsigned long rpcSetMEMCBypass(unsigned long para1, unsigned long para2);

#ifdef CONFIG_HDR_SDR_SEAMLESS
static DEFINE_SPINLOCK(VDEC_SEAMLESS_CHANGE_SPINLOCK);/*Spin lock no context switch. When we need to do change SDR and HDR*/
unsigned char vdec_seamless_change_flag = OFF;//i3ddma format need to change
#endif

#ifdef ENABLE_DUMP_FRAME_INFO
  /* dump VO CRC info to file */
  VO_MALLOC_STRUCT g_pDumpCRC_Send ;
  VO_MALLOC_STRUCT g_pDumpCRC ;
  VO_DUMP_STRUCT DumpCRC_Config;
  struct file *file_DumpCRC = 0 ;
  static struct task_struct *rtkvo_dumpcrc_tsk;
#ifndef CONFIG_KDRIVER_USE_NEW_COMMON
  static int data;
  static int rtkvo_dumpcrc_thread(void *arg);
 #endif
  static unsigned long long f_offset_dumpcrc = 0 ;
  static int rtkvo_dumpCRC_enable(const char *pattern, int length);
  static void rtkvo_dumpCRC_disable(void);
#endif

  static dev_t vo_devno; /* vo device number */
  static struct cdev vo_cdev;
  // -------------------------------


int vodev_open(struct inode *inode, struct file *filp)
{
	return 0;
}

ssize_t vodev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	return 0;
}

ssize_t vodev_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	long ret = count;
	char cmd_buf[100] = {0};

	#ifndef CONFIG_ARM64
	rtd_pr_vo_info("%s(): count=%d, buf=0x%08lx\n", __func__, count, (long)buf);
	#endif

	if (count >= 100)
		return -EFAULT;

	if (copy_from_user(cmd_buf, buf, count)) {
		ret = -EFAULT;
	}

#ifdef ENABLE_DUMP_FRAME_INFO
	if ((cmd_buf[0] == 't') && (cmd_buf[1] == 'e') && (cmd_buf[2] == 's') && (cmd_buf[3] == 't') && (cmd_buf[4] == 'm')
            && (cmd_buf[5] == 'o') && (cmd_buf[6] == 'd') && (cmd_buf[7] == 'e') && (cmd_buf[8] == '='))

    {
            // shell command as: echo testmode=1 testresult=/tmp/var/log/xxxxx > /dev/vodev
            if((cmd_buf[9] == '1') && (cmd_buf[11] == 't') && (cmd_buf[20] == 't') && (cmd_buf[21] == '=')){
			rtd_pr_vo_notice("\n=== rtkvo: enable dump CRC =============\n");
                   rtkvo_dumpCRC_enable(&cmd_buf[22], (count-22)-1);
			rtd_pr_vo_notice("\n=======================================\n");
            } // shell command as: echo testmode=1 /tmp/var/log/xxxxx > /dev/vodev
		else if ((cmd_buf[9] == '1') && (cmd_buf[11] == '/')) {
			rtd_pr_vo_notice("\n=== rtkvo: enable dump CRC =============\n");
                   rtkvo_dumpCRC_enable(&cmd_buf[11], (count-11)-1);
			rtd_pr_vo_notice("\n=======================================\n");
            } // shell command as: echo testmode=0 > /dev/vodev
		else if (cmd_buf[9] == '0') {
			rtd_pr_vo_notice("\n=== rtkvo: disable dump CRC =============\n");
			rtkvo_dumpCRC_disable();
			rtd_pr_vo_notice("\n=======================================\n");
		}
		else {
			rtd_pr_vo_notice("%s\n", cmd_buf);
		}
	}
	else if ((cmd_buf[0] == 'd') && (cmd_buf[1] == 'c') && (cmd_buf[2] == 'r') && (cmd_buf[3] == 'c') && (cmd_buf[4] == '_')) {
		if ((cmd_buf[5] == 'e') && (cmd_buf[6] == 'n')) {
			rtd_pr_vo_notice("\n=== rtkvo: enable dump CRC =============\n");
			rtkvo_dumpCRC_enable(&cmd_buf[8], (count-8)-1);
			rtd_pr_vo_notice("\n=======================================\n");
		}
		else if ((cmd_buf[5] == 'd') && (cmd_buf[6] == 'i')) {
			rtd_pr_vo_notice("\n=== rtkvo: disable dump CRC =============\n");
			rtkvo_dumpCRC_disable();
			rtd_pr_vo_notice("\n=======================================\n");
		}
		else {
			rtd_pr_vo_notice("%s\n", cmd_buf);
		}
	}
        else {
		rtd_pr_vo_notice("%s\n", cmd_buf);
	}
#endif

	return ret;
}

int vodev_release(struct inode *inode, struct file *filp)
{
	return 0;
}

long vodev_ioctl(struct file *filp, unsigned int cmd,  unsigned long arg)
{
	int ret = 0;

	if (_IOC_TYPE(cmd) != VO_IOC_MAGIC || _IOC_NR(cmd) > VO_IOC_MAXNR) {
		rtd_pr_vo_err("VO: ioctl code = %x is invalid!!!!!!!!!!!!!!!\n", cmd);
		return -ENOTTY;
	}

	rtd_pr_vo_info("VO: ioctl code = %x\n", cmd);

	switch (cmd) {
	case VO_IOC_OPEN:
		{
		VO_OPEN_CMD_T data;

		if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(VO_OPEN_CMD_T))) {
			ret = -EFAULT;
			rtd_pr_vo_err("VO: ioctl code = %ld failed!!!!!!!!!!!!!!!\n", (unsigned long)VO_IOC_OPEN);
		} else {
			ret = VO_Open((VO_OPEN_CMD_T *)&data);

			if (copy_to_user((void __user *)arg, (const void *)&data, sizeof(VO_OPEN_CMD_T))) {
				ret = -EFAULT;
				rtd_pr_vo_err("VO: ioctl code = %ld failed!!!!!!!!!!!!!!!\n", (unsigned long)VO_IOC_OPEN);
			}
		}
		}
		break;
	case VO_IOC_CONFIG:
		{
		VO_CONFIG_CMD_T data;

		if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(VO_CONFIG_CMD_T))) {
			ret = -EFAULT;
			rtd_pr_vo_err("VO: ioctl code = %ld failed!!!!!!!!!!!!!!!\n", (unsigned long)VO_IOC_CONFIG);
		} else {
			ret = VO_Config(data.ch, data.cfg);
		}
		}
		break;
	case VO_IOC_DISPLAYPICTTURE:
		{
		VO_DISPLAY_CMD_T data;

		if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(VO_DISPLAY_CMD_T))) {
			ret = -EFAULT;
			rtd_pr_vo_err("VO: ioctl code = %ld failed!!!!!!!!!!!!!!!\n", (unsigned long)VO_IOC_DISPLAYPICTTURE);
		} else {
			//ret = VO_DisplayPicture(data.ch, data.image, false);
			rtd_pr_vo_err("VO:VO_IOC_DISPLAYPICTTURE=x=%d\n", data.image.rect.x);
			ret = VO_Set_Photo_Shift_ShareMEM_Value(data.image.rect.x);
			VO_Wait_Set_Photo_Shift_ShareMEM_finish();
		}
		}
		break;
	case VO_IOC_REDRAWPICTURE:
		{
		unsigned int data;

		if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(unsigned int))) {
			ret = -EFAULT;
			rtd_pr_vo_err("VO: ioctl code = %ld failed!!!!!!!!!!!!!!!\n", (unsigned long)VO_IOC_REDRAWPICTURE);
		} else {
			ret = VO_RedrawPicture((unsigned int)data);
		}
		}
		break;
	case VO_IOC_CLOSE:
		{
		unsigned int data;

		if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(unsigned int))) {
			ret = -EFAULT;
			rtd_pr_vo_err("VO: ioctl code = %ld failed!!!!!!!!!!!!!!!\n", (unsigned long)VO_IOC_CLOSE);
		} else {
			ret = VO_Close((unsigned int)data);
		}
		}
		break;
    case VO_IOC_SETPICTUREINFO:
    	{
	    VO_DISPLAY_CMD_T data;
	    if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(VO_DISPLAY_CMD_T))) {
		    ret = -EFAULT;
		    rtd_pr_vo_err("VO: ioctl code = %ld failed!!!!!!!!!!!!!!!\n", (unsigned long)VO_IOC_SETPICTUREINFO);
	    } else {
		    ret = VO_SetPictureInfo(data.ch, (HAL_VO_IMAGE_T *)&data.image);
	    }
	    }
	    break;
	default:
		rtd_pr_vo_err("VO: ioctl code = %d is invalid!!!!!!!!!!!!!!!\n", cmd);
		break;
	}

	return ret;
}

const struct file_operations vo_fops = {
	.owner			=	THIS_MODULE,
	.open			=	vodev_open,
	.release		=	vodev_release,
	.read			=	vodev_read,
	.write			=	vodev_write,
	.unlocked_ioctl	=	vodev_ioctl,
#if defined(CONFIG_ARM64) && defined(CONFIG_COMPAT)
    .compat_ioctl   =   vodev_ioctl,
#endif
};

static struct class *vo_class;
static struct platform_device *vo_platform_devs;
static struct platform_driver vo_platform_driver = {
	.driver = {
		.name	=	"vodev",
		.bus	=	&platform_bus_type,
	},
};

static char *vo_devnode(struct device *dev, umode_t *mode)
{
	return NULL;
}

void vo_module_exit(void)
{
	if ((!vo_platform_devs) /*|| (class_destroy == NULL)*/)
		BUG();

	kfree(vodma_data);
	vodma_data = NULL;

	device_destroy(vo_class, vo_devno);
	cdev_del(&vo_cdev);

	platform_driver_unregister(&vo_platform_driver);
	platform_device_unregister(vo_platform_devs);
	vo_platform_devs = NULL;

	class_destroy(vo_class);
	vo_class = NULL;

	unregister_chrdev_region(vo_devno, 1);

	rtd_pr_vo_info("\n\n\n\n *****************  VO module exit  *********************\n\n\n\n");
}

int vo_module_init(void)
{
	int result;
	int ch = 0;
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	long ret = 0;
#endif
	unsigned char VoReady;

	VoReady = 1;

	rtd_pr_vo_info("\n\n\n\n *****************  VO module init  *********************\n\n\n\n");

	result = alloc_chrdev_region(&vo_devno, 0, 1, "vodevno");

	if (result != 0) {
		rtd_pr_vo_crit("Cannot allocate VO device number\n");
		return result;
	}

	vo_class = class_create(THIS_MODULE, "vodev");
	if (IS_ERR(vo_class)) {
		rtd_pr_vo_crit("vo: can not create class...\n");
		result = PTR_ERR(vo_class);
		goto fail_class_create;
	}

	vo_class->devnode = vo_devnode;

	vo_platform_devs = platform_device_register_simple("vodev", -1, NULL, 0);
	if (platform_driver_register(&vo_platform_driver) != 0) {
		rtd_pr_vo_crit("vo: can not register platform driver...\n");
		result = -ENOMEM;
		goto fail_platform_driver_register;
	}

	cdev_init(&vo_cdev, &vo_fops);
	result = cdev_add(&vo_cdev, vo_devno, 1);
	if (result < 0) {
		rtd_pr_vo_crit("vo: can not add character device...\n");
		goto fail_cdev_init;
	}

	device_create(vo_class, NULL, vo_devno, NULL, "vodev");

	vodma_data = kmalloc(sizeof(VO_DATA), GFP_KERNEL);
	if (!vodma_data) {
		result = -ENOMEM;
		vo_module_exit();   /* fail */
		return result;
	}
	sema_init(&VO_InfoSemaphore, 1);
	sema_init(&VO_MemChunkSemaphore, 1);
	memset(vodma_data, 0, sizeof(VO_DATA));

	memset(&VO_dispinfo[0], 0, sizeof(StructDisplayInfo));
	memset(&VO_dispinfo[1], 0, sizeof(StructDisplayInfo));

	for (ch = 0; ch < VODMA_DEV_NUM; ch++)
		vodma_data[0].vo_state[ch] = VO_STATE_INIT;

	for (ch = 0; ch < VODMA_DEV_NUM; ch++)
		vodma_data[0].avsync[ch] = 1;

	for (ch = 0; ch < VODMA_DEV_NUM; ch++)
		vodma_data[0].filmDetectDone[ch] = 1;

	/* register RPC command mapping handler */
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	if (register_kernel_rpc(RPC_VCPU_ID_0x110_VO_READY, rpcVoReady) == 1)
		rtd_pr_vo_crit("Register RPC VO_READY failed!\n");

	if (register_kernel_rpc(RPC_VCPU_ID_0x111_VO_NOSIGNAL, rpcVoNoSignal) == 1)
		rtd_pr_vo_crit("Register RPC VO_NOSIGNAL failed!\n");

	if (register_kernel_rpc(RPC_VCPU_ID_0x114_VO_AVSYNC, rpcVoAVSync) == 1)
		rtd_pr_vo_crit("Register RPC VO_AVSYNC failed!\n");

	if (register_kernel_rpc(RPC_VCPU_ID_0x112_HDR_FRAMEINFO, rpcVoHdrFrameInfo) == 1)
		rtd_pr_vo_crit("Register RPC HDR_FRAMEINFO failed!\n");

// FixMe, 20190920
#if 0
	if (register_kernel_rpc(RPC_VCPU_ID_0x115_VO_EOS, rpcVoEOS) == 1)
		rtd_pr_vo_crit("Register RPC VO_READY failed!\n");
#endif
	if (register_kernel_rpc(RPC_VCPU_ID_0x116_FILM_DETECT_DONE, rpcFilmDetectDone) == 1)
		rtd_pr_vo_crit("Register RPC VO_AVSYNC failed!\n");
#if 1
	if (register_kernel_rpc(RPC_VCPU_ID_0x117_MEMC_BYPASS_MODE, rpcSetMEMCBypass) == 1)
		rtd_pr_vo_crit("Register RPC SET_MECM_BYPASS failed\n");
#endif

#endif

#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	if (send_rpc_command(RPC_VIDEO, VIDEO_RPC_VOUT_ToAgent_VoDriverReady, VoReady, 0, &ret))
		rtd_pr_vo_crit("VoReady RPC fail!!\n");
#endif

#ifdef ENABLE_DUMP_FRAME_INFO
	// dump VO frame CRC info
	rtkvo_dump_init();
#endif

	rtd_pr_vo_info("VO module_init finish\n");
	return 0; /* Success */

fail_cdev_init:
	platform_driver_unregister(&vo_platform_driver);
fail_platform_driver_register:
	platform_device_unregister(vo_platform_devs);
	vo_platform_devs = NULL;
	class_destroy(vo_class);
fail_class_create:
	vo_class = NULL;
	unregister_chrdev_region(vo_devno, 1);
	return result;
}

#ifdef CONFIG_SUPPORT_SCALER_MODULE
// the module init/exit will be moved to scaler_module.c if scaler was built as a kernel module
#else
module_init(vo_module_init);
module_exit(vo_module_exit);
#endif

extern VDEC_VBM_MEM_LIST VBM_Memory[RTKVDEC_MAX_VBM_MEM_NUM];
#ifdef CONFIG_VBM_HEAP
int VBM_ReturnMemChunk(unsigned long dmabuf_hndl, unsigned long buffer_addr_in, unsigned long buffer_size_in)
{
	unsigned long phy_addr;

	if ((buffer_addr_in == 0) || (buffer_size_in == 0) || IS_ERR((struct dma_buf *)dmabuf_hndl)) {
		rtd_pr_vo_err("[vo] Return an invalid address %x, dmabuf(%lx)\n", (unsigned int)buffer_addr_in, (unsigned long)dmabuf_hndl);
		return -1;
	}

	phy_addr = dvr_dmabuf_to_phys((struct dma_buf *)dmabuf_hndl);
	if (phy_addr != buffer_addr_in) {
		rtd_pr_vo_err("[vo] Return an invalid address %x, dmabuf_to_phys(%lx)\n", (unsigned int)buffer_addr_in, (unsigned long)phy_addr);
		return -1;
	}

	rtd_pr_vo_info("[vo] VBM_ReturnMemChunk: return addr %x, size %x\n", (unsigned int)buffer_addr_in, (unsigned int)buffer_size_in);
	down(&VO_MemChunkSemaphore);
	dvr_dmabuf_free((struct dma_buf *)dmabuf_hndl);
	up(&VO_MemChunkSemaphore);
	return 0;
}

int VBM_BorrowMemChunk(unsigned long *dmabuf_hndl, unsigned long *buffer_addr, unsigned long *buffer_size, UINT32 reqSize, UINT32 type)
{
	unsigned long phy_addr;
	struct dma_buf *dmabuf;
	unsigned long ret=0;

	if ((!buffer_addr) || (!buffer_size) || (reqSize == 0)) {
		rtd_pr_vo_err("[vo] BorrowVBMMemChunk parameter fail!!\n");
		return -1;
	}

	rtd_pr_vo_info("[vo] VBM_BorrowMemChunk: request size %d, type =%d\n", reqSize, type);

	down(&VO_MemChunkSemaphore);
	dmabuf = dvr_dmabuf_chunk_malloc(reqSize, VBM_BUF_DECOMP, DMA_HEAP_USER_COBUFFER, false, (unsigned long **)&phy_addr);
	if (phy_addr == INVALID_VAL) {
		rtd_pr_vo_warn("[%s %d]alloc VBM_CMA memory fail\n", __func__, __LINE__);
		*buffer_addr = 0;
		*buffer_size = 0;
		up(&VO_MemChunkSemaphore);
		return -1;
	}

	*buffer_addr = phy_addr;
	*buffer_size = reqSize;
	if (dmabuf_hndl)
		*dmabuf_hndl = (unsigned long)dmabuf;
	rtd_pr_vo_info("[vo] VBM_BorrowMemChunk: get addr %x, size %d\n", (unsigned int)*buffer_addr, (unsigned int)*buffer_size);
	if (send_rpc_command(RPC_VIDEO, VIDEO_RPC_VO_PHOTO_ToAgent_SetAddr , 0, 0, &ret) == RPC_FAIL) {
			rtd_pr_vo_err("[vo] VIDEO_RPC_VO_PHOTO_ToAgent_SetAddr fail!!\n");
			up(&VO_MemChunkSemaphore);
			return -1;
	}
	up(&VO_MemChunkSemaphore);
	return 0;
}
#else
int VBM_ReturnMemChunk(unsigned long buffer_addr_in, unsigned long buffer_size_in)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_VDEC)
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	unsigned long result;
#endif
	unsigned long vir_addr, vir_addr_noncache;
	unsigned long buffer_addr = 0, buffer_size = 0;
	unsigned int phy_addr;
#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
	int i = 0;
#endif
	VIDEO_RPC_VBM_RETURNMEMCHUNK *rpc_data;

	if ((buffer_addr_in == 0) || (buffer_size_in == 0))
		return -1;
#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
	for(i = 0; i < RTKVDEC_MAX_VBM_MEM_NUM ; i++) {
		if(buffer_addr_in == VBM_Memory[i].aligned_addr) {
			buffer_addr = VBM_Memory[i].addr;
			buffer_size = VBM_Memory[i].size;
			break;
		}
	}
#else
	buffer_addr = buffer_addr_in;
	buffer_size = buffer_size_in;
#endif
	if(buffer_addr == 0){
		rtd_pr_vo_err("[vo] Return an invalid address %x\n", (unsigned int)buffer_addr);
		return -1;
	}
	rtd_pr_vo_info("[vo] VBM_ReturnMemChunk: return addr %x, size %x\n", (unsigned int)buffer_addr, (unsigned int)buffer_size);
	down(&VO_MemChunkSemaphore);
	vir_addr = (unsigned long)dvr_malloc_uncached_specific(sizeof(VIDEO_RPC_VBM_RETURNMEMCHUNK), GFP_DCU1_LIMIT, (void **)&vir_addr_noncache);
	if (vir_addr) {
		phy_addr = (unsigned int)dvr_to_phys((void *)vir_addr);
		rpc_data = (VIDEO_RPC_VBM_RETURNMEMCHUNK *)vir_addr_noncache;
		rpc_data->memChunkAddr = htonl(buffer_addr);
		rpc_data->memChunkSize = htonl(buffer_size);
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
		if (send_rpc_command(RPC_VIDEO, VIDEO_RPC_ToAgent_ReturnVBMMemChunk, phy_addr, 0, &result) == RPC_FAIL) {
			rtd_pr_vo_err("[vo] VIDEO_RPC_ToAgent_ReturnVBMMemChunk fail!!\n");
			dvr_free((void *)vir_addr);
			up(&VO_MemChunkSemaphore);
			return -1;
		}
#endif
		dvr_free((void *)vir_addr);
	}
	up(&VO_MemChunkSemaphore);
#endif
	return 0;
}

#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
extern unsigned int svp_aligned_size;
#endif
int VBM_BorrowMemChunk(unsigned long *buffer_addr, unsigned long *buffer_size, UINT32 reqSize, UINT32 type)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_VDEC)
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	unsigned long result;
#endif
	unsigned long vir_addr, vir_addr_noncache;
#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
	unsigned long buffer_addr_original = 0, buffer_size_original = 0;
	int i = 0;
#endif
	unsigned int phy_addr;
	VIDEO_RPC_VBM_BORROWMEMCHUNK *rpc_data;

	if ((!buffer_addr) || (!buffer_size) || (reqSize == 0)) {
		rtd_pr_vo_err("[vo] BorrowVBMMemChunk parameter fail!!\n");
		return -1;
	}

	rtd_pr_vo_info("[vo] VBM_BorrowMemChunk: request size %d,type =%d\n", reqSize,type);
	down(&VO_MemChunkSemaphore);
	vir_addr = (unsigned long)dvr_malloc_uncached_specific(sizeof(VIDEO_RPC_VBM_BORROWMEMCHUNK), GFP_DCU1_LIMIT, (void **)&vir_addr_noncache);
	if (vir_addr) {
		memset((void *)vir_addr_noncache, 0, sizeof(VIDEO_RPC_VBM_BORROWMEMCHUNK));
		phy_addr = (unsigned int)dvr_to_phys((void *)vir_addr);
		rpc_data = (VIDEO_RPC_VBM_BORROWMEMCHUNK *)vir_addr_noncache;
		rpc_data->memChunkAddr = 0;
		rpc_data->memChunkSize = 0;
		rpc_data->reqSize = htonl(reqSize);
		rpc_data->type =type;

#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
		if (send_rpc_command(RPC_VIDEO, VIDEO_RPC_ToAgent_BorrowVBMMemChunk, phy_addr, 0, &result) == RPC_FAIL) {
			rtd_pr_vo_err("[vo] VIDEO_RPC_ToAgent_BorrowVBMMemChunk fail!!\n");
			dvr_free((void *)vir_addr);
			up(&VO_MemChunkSemaphore);
			return -1;
		}
#endif
#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
		buffer_addr_original = ntohl(*(unsigned int *)vir_addr_noncache);
		buffer_size_original = ntohl(*(unsigned int *)(vir_addr_noncache + sizeof(rpc_data->memChunkAddr)));
		for(i = 0; i < RTKVDEC_MAX_VBM_MEM_NUM ; i++) {
			if(buffer_addr_original == VBM_Memory[i].addr) {
				*buffer_addr = VBM_Memory[i].aligned_addr;
				*buffer_size = VBM_Memory[i].aligned_size;
				break;
			}
		}
		if(*buffer_addr == 0){
			rtd_pr_vo_err("[vo] VBM_BorrowMemChunk: get an invalid address %x\n", (unsigned int)buffer_addr_original);
			dvr_free((void *)vir_addr);
			up(&VO_MemChunkSemaphore);
			return -1;
		}
#else
		*buffer_addr = ntohl(*(unsigned int *)vir_addr_noncache);
		*buffer_size = ntohl(*(unsigned int *)(vir_addr_noncache + sizeof(rpc_data->memChunkAddr)));
#endif

		dvr_free((void *)vir_addr);
	}

	rtd_pr_vo_info("[vo] VBM_BorrowMemChunk: get addr %x, size %d\n", (unsigned int)*buffer_addr, (unsigned int)*buffer_size);
	if (*buffer_addr == 0) {
		up(&VO_MemChunkSemaphore);
		rtd_pr_vo_err("[vo] Borrow VBM address %x size %x fail\n", (unsigned int)buffer_addr, (unsigned int)buffer_size);
		return -1;
	}
#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
	if (*buffer_size < (reqSize - svp_aligned_size)) {
#else
	if (*buffer_size < reqSize) {
#endif
		VBM_ReturnMemChunk((unsigned long)(*buffer_addr), (unsigned long)(*buffer_size));
		*buffer_addr = 0;
		*buffer_size = 0;
		up(&VO_MemChunkSemaphore);
		rtd_pr_vo_err("[vo] Borrow VBM address %x size %x < reqSize %x fail\n", (unsigned int)buffer_addr, (unsigned int)buffer_size, reqSize);
		return -1;
	}
	up(&VO_MemChunkSemaphore);
#endif
	return 0;
}
#endif

void vo_free_hal_buffer(unsigned int ch)
{
	VO_DATA *vo = &vodma_data[0];

    int i = 0;
    for (i = 0; i < VODMA_HALBUF_NUM; i++) {
        if (vo->hal_phy_addr[ch][i] && vo->hal_phy_addr[ch][i] != INVALID_VAL) {
            if (vo->borrow_from_vbm[ch][i])
            {
                rtd_pr_vo_info("[vo] free hal_phy_addr[%d][%d]: vbm buf:%x, size:%d\n", ch, i, (unsigned int)vo->hal_phy_addr[ch][i], (unsigned int)vo->hal_buf_size[ch][i]);
#ifdef CONFIG_VBM_HEAP
                VBM_ReturnMemChunk(vo->dmabuf_hndl[ch][i], vo->hal_phy_addr[ch][i], vo->hal_buf_size[ch][i]);
#else
                VBM_ReturnMemChunk(vo->hal_phy_addr[ch][i], vo->hal_buf_size[ch][i]);
#endif
            }
            else
            {
                rtd_pr_vo_info("[vo] free hal_phy_addr[%d][%d]: pli buf:%x, size:%d\n", ch, i, (unsigned int)vo->hal_phy_addr[ch][i], (unsigned int)vo->hal_buf_size[ch][i]);
                pli_free(vo->hal_phy_addr[ch][i]);
            }
        }
        vo->borrow_from_vbm[ch][i] = 0;
        vo->borrow_size[ch][i] = 0;
        vo->hal_buf_size[ch][i] = 0;
        vo->hal_phy_addr[ch][i] = 0;
#ifdef CONFIG_VBM_HEAP
        vo->dmabuf_hndl[ch][i] = 0;
#endif
    }
}

int vo_malloc_hal_buffer(unsigned int ch)
{
	VO_DATA *vo = &vodma_data[0];
	int reqSize = 0;
	//bool DCUSizeDiff = false;
	int i = 0;

	vo_free_hal_buffer(ch);

	if (get_platform() == PLATFORM_KXLP) {

		//DCUSizeDiff = true;
	}

	if (1) {
		if (vo->panel_type[ch] == HAL_VO_PANEL_TYPE_UHD)
			reqSize = PANEL_UHD_WIDTH*2*PANEL_UHD_HEIGHT*3;  /* WIDTH*2 for slide */
		else
			reqSize = PANEL_FHD_WIDTH*2*PANEL_FHD_HEIGHT*3;  /* WIDTH*2 for slide */

        for (i = 0; i < VODMA_HALBUF_NUM; i++)
        {
            if (vo->hal_phy_addr[ch][i] == 0)
            {
#ifdef CONFIG_VBM_HEAP
                if (VBM_BorrowMemChunk(&vo->dmabuf_hndl[ch][i], (unsigned long *)(&vo->hal_phy_addr[ch][i]), (unsigned long *)(&vo->hal_buf_size[ch][i]), reqSize, i+1) == 0)
#else
                if (VBM_BorrowMemChunk((unsigned long *)(&vo->hal_phy_addr[ch][i]), (unsigned long *)(&vo->hal_buf_size[ch][i]), reqSize, i+1) == 0)
#endif
                {
                    rtd_pr_vo_info("[vo] borrow vbm buf: hal_phy_addr[%d][%d]:%x, size=%d\n", ch, i, (unsigned int)vo->hal_phy_addr[ch][i], (unsigned int)vo->hal_buf_size[ch][i]);
                    vo->borrow_from_vbm[ch][i] = 1;
                    vo->borrow_size[ch][i] = vo->hal_buf_size[ch][i];
                }
                else
                {
                    rtd_pr_vo_info("[vo] hal_phy_addr[%d][%d] VBM_BorrowMemChunk fail, trying to malloc from cma\n", ch, i);
                    vo->borrow_from_vbm[ch][i] = 0;
                    vo->borrow_size[ch][i] = 0;

                    vo->hal_phy_addr[ch][i] = (unsigned long)pli_malloc(reqSize, GFP_DCU2_FIRST);
                    vo->hal_buf_size[ch][i] = reqSize;
                    rtd_pr_vo_info("[vo] pli_malloc buf: hal_phy_addr[%d][%d]:%x, size=%d\n", ch, i, (unsigned int)vo->hal_phy_addr[ch][i], reqSize);

                    if (vo->hal_phy_addr[ch][i] == INVALID_VAL)
                    {
                        rtd_pr_vo_info("[vo] error: malloc memory fail\n");
                        vo_free_hal_buffer(ch);

                        return -1;
                    }
                }
            }
        }

	   return 0;

	}

	return -1;
}

#if 0
void vo_free_photo_buffer(unsigned int ch)
{
	VO_DATA *vo = &vodma_data[0];
	int i = 0;

	for (i = 0; i < VODMA_BUF_NUM; i++) {
		if (vo->phy_addr[ch][i] && vo->phy_addr[ch][i] != INVALID_VAL) {
			rtd_pr_vo_info("[vo] free buf: phy_addr[%d][%d]:%x\n", ch, i, (unsigned int)vo->phy_addr[ch][i]);

			if (vo->borrow_from_vbm[ch][i])
				VBM_ReturnMemChunk(vo->phy_addr[ch][i], vo->borrow_size[ch][i]);
			else
				pli_free(vo->phy_addr[ch][i]);

			vo->borrow_from_vbm[ch][i] = 0;
			vo->borrow_size[ch][i] = 0;
			vo->phy_addr[ch][i] = 0;
		}
	}
}

int vo_malloc_photo_buffer(unsigned int ch, unsigned long *buffer_addr, unsigned int *buffer_size)
{
	VO_DATA *vo = &vodma_data[0];
	int i = 0;
	int reqSize = 0;
	int getSize = 0;
	bool DCUSizeDiff = false;

	vo_free_photo_buffer(ch);

	if (get_platform() == PLATFORM_KXLP) {

		DCUSizeDiff = true;
	}

	if (1) {
		reqSize = buffer_size[0] + buffer_size[1];
		rtd_pr_vo_info("[vo][%s][%d] call VBM_BorrowMemChunk\n", __FUNCTION__, __LINE__);
		if (VBM_BorrowMemChunk((unsigned long *)(&buffer_addr[0]), (unsigned long *)(&getSize), reqSize ,2 ) == 0) {
			buffer_addr[1] = buffer_addr[0] + buffer_size[0];
			vo->borrow_from_vbm[ch][0] = 1;
			vo->borrow_size[ch][0] = getSize;
			vo->borrow_from_vbm[ch][1] = 1;
			vo->borrow_size[ch][1] = 0;
			return 0;
		} else {
			vo_free_photo_buffer(ch);

			reqSize = buffer_size[0];
			rtd_pr_vo_info("[vo][%s][%d] call VBM_BorrowMemChunk\n", __FUNCTION__, __LINE__);
			if (VBM_BorrowMemChunk((unsigned long *)(&buffer_addr[0]), (unsigned long *)(&getSize), reqSize, 2) == 0) {
				vo->borrow_from_vbm[ch][0] = 1;
				vo->borrow_size[ch][0] = getSize;
			}

			reqSize = buffer_size[1];
			rtd_pr_vo_info("[vo][%s][%d] call VBM_BorrowMemChunk\n", __FUNCTION__, __LINE__);
			if (VBM_BorrowMemChunk((unsigned long *)(&buffer_addr[1]), (unsigned long *)(&getSize), reqSize, 2) == 0) {
				vo->borrow_from_vbm[ch][1] = 1;
				vo->borrow_size[ch][1] = getSize;
			}
		}
	}

	for (i = 0; i < VODMA_BUF_NUM; i++) {
		if (buffer_addr[i] == 0) {
			/* if use mixer plane alpha, no need temp buffer for se alpha blending*/
			if (i == (VODMA_BUF_NUM-1))
				return 0;

			buffer_addr[i] = (unsigned long)pli_malloc(buffer_size[i], GFP_DCU2_FIRST);
			rtd_pr_vo_info("[vo] malloc buf: phy_addr[%d][%d]:%x, size=%d\n", ch, i, (unsigned int)buffer_addr[i], buffer_size[i]);
			if (buffer_addr[i] == INVALID_VAL) {
				rtd_pr_vo_err("[vo] error: malloc memory fail\n");
				vo_free_photo_buffer(ch);
				return -1;
			}
		}
	}

	return 0;
}

#endif

/**
* @brief Open the video output module.
* @param   ch  [in]    channel port number to connect with display engine
* @param   def [in]    video panel type
* @return if success HAL_VO_STATE_OK, else one of error in HAL_VO_STATE_T.
**/
HAL_VO_STATE_T VO_Open(VO_OPEN_CMD_T *cmd)
{
	VO_DATA *vo = &vodma_data[0];
	unsigned int buf_w, buf_h, buf_size;
	unsigned int ch = cmd->ch;
	HAL_VO_PANEL_TYPE panel_type = cmd->def;

	rtd_pr_vo_info("[vo] VO_Open: ch %d, panel %d\n", ch, (int)panel_type);

	if (ch >= VODMA_DEV_NUM) {
		rtd_pr_vo_err("[vo] error: ch %d doesn't exist\n", ch);
		return HAL_VO_STATE_NOT_AVAILABLE;
	}

	if (panel_type >= HAL_VO_PANEL_TYPE_MAX || panel_type <= HAL_VO_PANEL_TYPE_NONE) {
		rtd_pr_vo_err("[vo] error: panel_type %d doesn't exist\n", panel_type);
		return HAL_VO_STATE_INVALID_PARAM;
	}

	if (vo->vo_state[ch] == VO_STATE_OPEN || vo->vo_state[ch] == VO_STATE_DISPLAY) {
		rtd_pr_vo_err("[vo] error: open already\n");
		set_vo_nosignal_flag(ch, 1);
		set_vo_EOS_flag(ch, 0);
		set_film_detect_done_flag(ch, 0);
		vo->panel_type[ch] = panel_type;
		vo->videoPlane[ch].Ddomain_Chroma_fmt = FMT_422;
		vo->alpha_blend[ch] = 255;
		vo->vo_state[ch] = VO_STATE_OPEN;
		vo->buf_idx[ch] = 0;
		// 2 hal_buffer
		cmd->buf_addr[0] = vo->hal_phy_addr[ch][0];
		cmd->buf_addr[1] = vo->hal_phy_addr[ch][1];
		cmd->buf_size= vo->hal_buf_size[ch][0];
		rtd_pr_vo_err("[vo]error :setup vo_malloc_hal_buffer: phy_addr[0]:%x,[1]:%x,size =%d\n", cmd->buf_addr[0],cmd->buf_addr[1],cmd->buf_size);
		return HAL_VO_STATE_OK;
	}

	if (panel_type == HAL_VO_PANEL_TYPE_UHD) {
		buf_w = PANEL_UHD_WIDTH * 2;  /* for slide */
		buf_h = PANEL_UHD_HEIGHT;
	} else {
		buf_w = PANEL_FHD_WIDTH * 2;  /* for slide */
		buf_h = PANEL_FHD_HEIGHT;
	}

	buf_w = (((buf_w + 511)>>9)<<9);
	buf_h = (((buf_h + 63)>>6)<<6);
	buf_size = buf_w * buf_h; /* buffer sizef for seq yuv420i */
#if 0
	/* yuv420i */
	vo->buffer_size[ch][0] = (buf_size*2); /* y*2  */
	vo->buffer_size[ch][1] = (buf_size); /* c*2  */
	vo->buffer_size[ch][2] = (buf_size*3/2); /* y&c for tmp pic*/

	if (vo_malloc_photo_buffer(ch, vo->phy_addr[ch], vo->buffer_size[ch]) == -1)
		return HAL_VO_STATE_NOT_AVAILABLE;

	vo->phy_addr_y[ch][0] = vo->phy_addr[ch][0];
	vo->phy_addr_y[ch][1] = vo->phy_addr[ch][0] + buf_size;
	vo->phy_addr_c[ch][0] = vo->phy_addr[ch][1];
	vo->phy_addr_c[ch][1] = vo->phy_addr[ch][1] + (buf_size/2);
#else
	/* yuv444sp */
	vo->buffer_size[ch][0] = (buf_size*3);
	vo->buffer_size[ch][1] = (buf_size*3);
	vo->buffer_size[ch][2] = 0;

//	if (vo_malloc_photo_buffer(ch, vo->phy_addr[ch], vo->buffer_size[ch]) == -1)
//		return HAL_VO_STATE_NOT_AVAILABLE;

	vo->phy_addr_y[ch][0] = vo->phy_addr[ch][0];
	vo->phy_addr_y[ch][1] = vo->phy_addr[ch][1];
	vo->phy_addr_c[ch][0] = vo->phy_addr[ch][0];
	vo->phy_addr_c[ch][1] = vo->phy_addr[ch][1];
#endif


	if (vo->phy_addr[ch][2]) {
		vo->phy_addr_y[ch][2] = vo->phy_addr[ch][2];
		vo->phy_addr_c[ch][2] = vo->phy_addr[ch][2] + buf_size;
	} else {
		vo->phy_addr_y[ch][2] = vo->phy_addr_y[ch][0];
		vo->phy_addr_c[ch][2] = vo->phy_addr_c[ch][0];
	}

	set_vo_nosignal_flag(ch, 1);
	set_vo_EOS_flag(ch, 0);
	set_film_detect_done_flag(ch, 0);

	vo->panel_type[ch] = panel_type;

	vo->videoPlane[ch].Ddomain_Chroma_fmt = FMT_422;

	vo->alpha_blend[ch] = 255;

	vo->vo_state[ch] = VO_STATE_OPEN;

	vo->buf_idx[ch] = 0;
// 2 hal_buffer
	if (vo_malloc_hal_buffer(ch) == 0) {
		cmd->buf_addr[0] = vo->hal_phy_addr[ch][0];
		cmd->buf_addr[1] = vo->hal_phy_addr[ch][1];
		cmd->buf_size= vo->hal_buf_size[ch][0];
		rtd_pr_vo_info("[vo]vo_malloc_hal_buffer: phy_addr[0]:%x,[1]:%x,size =%d\n", cmd->buf_addr[0],cmd->buf_addr[1],cmd->buf_size);
	} else {
		cmd->buf_addr[0] = 0;
		cmd->buf_addr[1] = 0;
		cmd->buf_size= 0;
	}


	return HAL_VO_STATE_OK;
}

/**
* @brief Configure the video output
* @param   ch  [in]    channel port number to connect with display engine
* @param   cfg [in]    configuration to set video output
* @return if success HAL_VO_STATE_OK, else one of error in HAL_VO_STATE_T.
**/
/* //static int phaze; */
HAL_VO_STATE_T VO_Config(unsigned int ch, HAL_VO_CFG_T cfg)
{
	VO_DATA *vo = &vodma_data[0];

	rtd_pr_vo_info("[vo] VO_Config: ch %d, type %d, value %d\n", ch, (int)cfg.type, cfg.value.alpha);

	if (ch >= VODMA_DEV_NUM) {
		rtd_pr_vo_err("[vo] error: ch %d doesn't exist\n", ch);
		return HAL_VO_STATE_NOT_AVAILABLE;
	}

	if (cfg.type >= HAL_VO_CFG_TYPE_MAX || cfg.type <= HAL_VO_CFG_TYPE_NONE) {
		rtd_pr_vo_err("[vo] error: type %d invalid\n", cfg.type);
		return HAL_VO_STATE_INVALID_PARAM;
	}

	switch (cfg.type) {
	case HAL_VO_CFG_TYPE_ALPHA_BLEND:
		if (cfg.value.alpha > 255) {
			rtd_pr_vo_err("[vo] VO_Config: ch %d, alpha %d invalid\n", ch, (int)cfg.value.alpha);
			return HAL_VO_STATE_INVALID_PARAM;
		}
		vo->alpha_blend[ch] = cfg.value.alpha;

#if 0 /* //change photo alpha linearly */
	int tmp_count = 0;
	int tmp_Alpha = (int)vo->alpha_blend[ch];
	int save_Alpha = (int)vo->alpha_blend[ch];

	if (save_Alpha == 255)
		phaze = -2;
	else if (save_Alpha == 0)
		phaze = 2;
	/* //ROSPrintf("phaze:%d\n",phaze); */
	if ((save_Alpha < 255) && (save_Alpha > 0)) {
		while ((tmp_count < 17)) {
			tmp_Alpha = save_Alpha + (phaze*tmp_count);
			if ((tmp_Alpha <= 0) || (tmp_Alpha >= 255))
				break;
			IoReg_Write32(0xb800c0a4, tmp_Alpha);
			tmp_count = tmp_count + 1;
			msleep(1);
			/* //rtd_pr_vo_err("############[pool test]config,alpha_reg:%d###############\n", IoReg_Read32(0xb800c0a4)); */
		}
	} else if (save_Alpha == 0) /* //((save_Alpha == 255 )|| (save_Alpha == 0)) { */
		IoReg_Write32(0xb800c0a4, tmp_Alpha);
	}
#endif

		rtd_pr_vo_info("[vo] VO_Config: ch %d, alpha_blend %d\n", ch, (int)vo->alpha_blend[ch]);
		if (vo->vo_state[ch] == VO_STATE_DISPLAY) {
			Mixer_Alpha(vo->alpha_blend[ch]);
			return HAL_VO_STATE_OK;
		}
		break;
	case HAL_VO_CFG_TYPE_PIXEL_FORMAT:
		if ((cfg.value.pixel_format >= HAL_VO_PIXEL_FORMAT_MAX) || (cfg.value.pixel_format <= HAL_VO_PIXEL_FORMAT_NONE)) {
			rtd_pr_vo_err("[vo] VO_Config: ch %d, pixel_format %d doesn't exist\n", ch, (int)cfg.value.pixel_format);
			return HAL_VO_STATE_INVALID_PARAM;
		}
		vo->image[ch].pixel_format = cfg.value.pixel_format;
		rtd_pr_vo_info("[vo] VO_Config: ch %d, pixel_format %d\n", ch, (int)vo->image[ch].pixel_format);
		break;
	default:
		rtd_pr_vo_err("[vo] error: type %d doesn't exist\n", cfg.type);
		return HAL_VO_STATE_INVALID_PARAM;
	}

	return HAL_VO_STATE_OK;
}

bool VP_Sequential2Block(UINT32 DC_index, UINT32 seqBufAddr, UINT32 width, UINT32 height, UINT32 pitch, UINT32 x_offset, UINT32 y_offset)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	unsigned long result;
#endif
	unsigned long vir_addr, vir_addr_noncache;
	unsigned int phy_addr;
	VIDEO_RPC_VP_SEQUENTIAL2BLOCK *rpc_data;

	vir_addr = (unsigned long)dvr_malloc_uncached_specific(sizeof(VIDEO_RPC_VP_SEQUENTIAL2BLOCK), GFP_DCU1_LIMIT, (void **)&vir_addr_noncache);
	if (vir_addr) {
		phy_addr = (unsigned int)dvr_to_phys((void *)vir_addr);
		rpc_data = (VIDEO_RPC_VP_SEQUENTIAL2BLOCK *)vir_addr_noncache;
		rpc_data->DC_index = htonl(DC_index);
		rpc_data->seqBufAddr = htonl(seqBufAddr);
		rpc_data->width = htonl(width);
		rpc_data->height = htonl(height);
		rpc_data->pitch = htonl(pitch);
		rpc_data->x_offset = htonl(x_offset);
		rpc_data->y_offset = htonl(y_offset);
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
		if (send_rpc_command(RPC_VIDEO, VIDEO_RPC_ToAgent_VP_Sequential2Block, phy_addr, 0, &result) == RPC_FAIL) {
			rtd_pr_vo_err("[vo] RPC VP_Sequential2Block fail!!\n");
			dvr_free((void *)vir_addr);
			return FALSE;
		}
#endif
		dvr_free((void *)vir_addr);
	}
	return TRUE;
};

bool SE_Alpha(UINT32 src_addrY, UINT32 src_addrC, UINT32 dst_addrY, UINT32 dst_addrC, UINT32 width, UINT32 height, UINT32 constAlpha)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	unsigned long result;
#endif
	unsigned long vir_addr, vir_addr_noncache;
	unsigned int phy_addr;
	VIDEO_RPC_SE_ALPHA *rpc_data;

	vir_addr = (unsigned long)dvr_malloc_uncached_specific(sizeof(VIDEO_RPC_SE_ALPHA), GFP_DCU1_LIMIT, (void **)&vir_addr_noncache);
	if (vir_addr) {
		phy_addr = (unsigned int)dvr_to_phys((void *)vir_addr);
		rpc_data = (VIDEO_RPC_SE_ALPHA *)vir_addr_noncache;
		rpc_data->src_addrY = htonl(src_addrY);
		rpc_data->src_addrC = htonl(src_addrC);
		rpc_data->dst_addrY = htonl(dst_addrY);
		rpc_data->dst_addrC = htonl(dst_addrC);
		rpc_data->width = htonl(width);
		rpc_data->height = htonl(height);
		rpc_data->constAlpha = htonl(constAlpha);
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
		if (send_rpc_command(RPC_VIDEO, VIDEO_RPC_ToAgent_SE_Alpha, phy_addr, 0, &result) == RPC_FAIL) {
			rtd_pr_vo_err("[vo] RPC SE_Alpha fail!!\n");
			dvr_free((void *)vir_addr);
			return FALSE;
		}
#endif
		dvr_free((void *)vir_addr);
	}
	return TRUE;
};

bool Mixer_Alpha(unsigned char alpha)
{
	osdovl_mixer_ctrl2_RBUS mixer_ctrl2;
	osdovl_mixer_layer_sel_RBUS mixer_layer_sel;
	osdovl_osd_db_ctrl_RBUS osd_db_ctrl;
	unsigned char layer;
	osdovl_mixer_c0_plane_alpha1_RBUS mixer_plane_alpha1;
	osdovl_mixer_c0_plane_alpha2_RBUS mixer_plane_alpha2;
	osdovl_mixer_c0_plane_offset_RBUS mixer_plane_offset;
	unsigned int mixer_plane_alpha1_reg;
	unsigned int mixer_plane_alpha2_reg;
	unsigned int mixer_plane_offset_reg;
	int timeout;
	int nloop;

	osd_db_ctrl.regValue = rtd_inl(OSDOVL_OSD_Db_Ctrl_reg);
	osd_db_ctrl.db_load = 0;
	rtd_outl(OSDOVL_OSD_Db_Ctrl_reg, osd_db_ctrl.regValue);

	mixer_ctrl2.regValue = rtd_inl(OSDOVL_Mixer_CTRL2_reg);
	if (mixer_ctrl2.mixer_en == 0) {
		mixer_ctrl2.mixer_en = 1;
		rtd_outl(OSDOVL_Mixer_CTRL2_reg, mixer_ctrl2.regValue);
	}

	mixer_layer_sel.regValue = rtd_inl(OSDOVL_Mixer_layer_sel_reg);
	layer = 255;
	if (mixer_layer_sel.c0_sel == 0) { /* 0:video */
		layer = 0;
		mixer_layer_sel.c0_plane_alpha_en = (alpha == 255) ? 0 : 1;
	} else if (mixer_layer_sel.c1_sel == 0) {
		layer = 1;
		mixer_layer_sel.c1_plane_alpha_en = (alpha == 255) ? 0 : 1;
	} else if (mixer_layer_sel.c2_sel == 0) {
		layer = 2;
		mixer_layer_sel.c2_plane_alpha_en = (alpha == 255) ? 0 : 1;
	} else if (mixer_layer_sel.c3_sel == 0) {
		layer = 3;
		mixer_layer_sel.c3_plane_alpha_en = (alpha == 255) ? 0 : 1;
	}
	rtd_outl(OSDOVL_Mixer_layer_sel_reg, mixer_layer_sel.regValue);

	if (layer != 255) {
		mixer_plane_alpha1_reg = OSDOVL_Mixer_c0_plane_alpha1_reg + (layer * 3 * 4);
		mixer_plane_alpha2_reg = OSDOVL_Mixer_c0_plane_alpha2_reg + (layer * 3 * 4);
		mixer_plane_offset_reg = OSDOVL_Mixer_c0_plane_offset_reg + (layer * 3 * 4);

		mixer_plane_alpha1.regValue = rtd_inl(mixer_plane_alpha1_reg);
		mixer_plane_alpha1.plane_alpha_a = 255;
		mixer_plane_alpha1.plane_alpha_r = alpha;
		rtd_outl(mixer_plane_alpha1_reg, mixer_plane_alpha1.regValue);

		mixer_plane_alpha2.regValue = rtd_inl(mixer_plane_alpha2_reg);
		mixer_plane_alpha2.plane_alpha_g = alpha;
		mixer_plane_alpha2.plane_alpha_b = alpha;
		rtd_outl(mixer_plane_alpha2_reg, mixer_plane_alpha2.regValue);

		mixer_plane_offset.regValue = rtd_inl(mixer_plane_offset_reg);
		mixer_plane_offset.plane_offset_a = 0;
		mixer_plane_offset.plane_offset_r = 0;
		mixer_plane_offset.plane_offset_g = 0;
		mixer_plane_offset.plane_offset_b = 0;
		rtd_outl(mixer_plane_offset_reg, mixer_plane_offset.regValue);
	}

	osd_db_ctrl.regValue = rtd_inl(OSDOVL_OSD_Db_Ctrl_reg);
	osd_db_ctrl.db_load = 1;
	rtd_outl(OSDOVL_OSD_Db_Ctrl_reg, osd_db_ctrl.regValue);

	/* make sure alpha == 0 while change picture */
	if (alpha == 0) {
		osd_db_ctrl.regValue = rtd_inl(OSDOVL_OSD_Db_Ctrl_reg);
		timeout = 10;
		nloop = 0;
		while (osd_db_ctrl.db_load && nloop < timeout) {
			nloop++;
			msleep(10);
			osd_db_ctrl.regValue = rtd_inl(OSDOVL_OSD_Db_Ctrl_reg);
		}
	}

	return TRUE;
}

void vodma_DCU_Set(UINT32 index, UINT32 st_addr, UINT32 width, UINT32 offset_x, UINT32 offset_y)
{
#if 0
	unsigned int dc_misc;
	unsigned int dc_pict_set;
	unsigned int dc_pict_set_offset;
	unsigned int page_size;
	unsigned int width_scale;
	unsigned int page_shift;
	unsigned int init_page;
	unsigned int init_page1, init_page2;
	unsigned int page_offset;
	bool DCUSizeDiff = false;

	width_scale = (width+255)/256;

	/* dcu page size config */
	dc_misc = rtd_inl(DC_SYS_DC_SYS_MISC_reg);
	page_size = DC_SYS_DC_SYS_MISC_get_page_size(dc_misc);
	page_shift = 11 + page_size;
	init_page = st_addr >> page_shift;
	init_page1 = init_page;
	init_page2 = init_page;
	page_offset = (rtd_inl(IBU_SEQ_BOUND_2_reg) >> page_shift);

	if (get_platform() == PLATFORM_KXLP) {

		DCUSizeDiff = true;
	}

	 /* ib  enable */
	if (IBU_SEQ_REGION_get_region_2_mode(rtd_inl(IBU_SEQ_REGION_reg)) > MODE_DC2 &&
	    IBU_SEQ_REGION_get_region_3_mode(rtd_inl(IBU_SEQ_REGION_reg)) > MODE_DC2 &&
	    (init_page >= page_offset)) {
		if ((width_scale & 1))
			rtd_pr_vo_alert("[vo] width_scale not even: %d\n", width_scale);
		if ((init_page & 1)) {
			rtd_pr_vo_err("[vo] init_page not even: %d\n", init_page);
			init_page += 1;
		}
		if ((offset_x & 1)) {
			rtd_pr_vo_err("[vo] offset_x not even: %d\n", offset_x);
			offset_x &= ~1;
		}

		init_page1 = init_page;
		init_page2 = init_page;
		if (DCUSizeDiff) {
			init_page1 = (init_page - page_offset);
			init_page2 = (init_page - page_offset);
		}

		width_scale = ((width+511)/512)<<1;

		width_scale >>= 1;
		init_page1 >>= 1;
		init_page2 >>= 1;
		offset_x >>= 1;

		if (DCUSizeDiff)
			init_page1 += page_offset;
	}

	/* // get HW semaphore // */
	while (!*(volatile unsigned int *)GET_MAPPED_RBUS_ADDR(SB2_HD_SEM_NEW_1_reg))
		udelay(10);

	dc_pict_set = DC_SYS_DC_PICT_SET_pict_width_scale(width_scale) |
		      DC_SYS_DC_PICT_SET_pict_init_page(init_page1);
	rtd_outl(DC_SYS_DC_PICT_SET_reg, dc_pict_set);

	dc_pict_set_offset = DC_SYS_DC_PICT_SET_OFFSET_pict_set_num(index) |
			     DC_SYS_DC_PICT_SET_OFFSET_pict_set_offset_x(offset_x >> 4) |
			     DC_SYS_DC_PICT_SET_OFFSET_pict_set_offset_y(offset_y >> 2);
	rtd_outl(DC_SYS_DC_PICT_SET_OFFSET_reg, dc_pict_set_offset);

	dc_pict_set = DC_SYS_DC_PICT_SET_pict_width_scale(width_scale) |
		      DC_SYS_DC_PICT_SET_pict_init_page(init_page2);
	rtd_outl(DC2_SYS_DC_PICT_SET_reg, dc_pict_set);

	dc_pict_set_offset = DC_SYS_DC_PICT_SET_OFFSET_pict_set_num(index) |
			     DC_SYS_DC_PICT_SET_OFFSET_pict_set_offset_x(offset_x >> 4) |
			     DC_SYS_DC_PICT_SET_OFFSET_pict_set_offset_y(offset_y >> 2);
	rtd_outl(DC2_SYS_DC_PICT_SET_OFFSET_reg, dc_pict_set_offset);

	/* // put HW semaphore // */
	*(volatile unsigned int *)GET_MAPPED_RBUS_ADDR(SB2_HD_SEM_NEW_1_reg) = 0;
#endif
}

unsigned int Mixer_GetCRC(unsigned int ch, unsigned int stable_count)
{
       VO_DATA *vo = &vodma_data[0];
	osdovl_mixer_crc_ctrl_RBUS mixer_crc_ctrl;
	unsigned int starttime = 0;
	unsigned int crc = 0;
	unsigned int cnt = 0;
	unsigned int sleeptime = 34;

	if (ch >= VODMA_DEV_NUM) {
		rtd_pr_vo_err("[vo] error: ch %d doesn't exist\n", ch);
		return HAL_VO_STATE_NOT_AVAILABLE;
	}

	mixer_crc_ctrl.regValue = rtd_inl(OSDOVL_Mixer_CRC_Ctrl_reg);
	mixer_crc_ctrl.mixer_crc_sel = 0; /* //0 : video_input */
	mixer_crc_ctrl.crc_conti = 1;
	mixer_crc_ctrl.crc_start = 1;
	rtd_outl(OSDOVL_Mixer_CRC_Ctrl_reg, mixer_crc_ctrl.regValue);

	starttime = rtd_inl(TIMER_SCPU_CLK90K_LO_reg);
	do {
		if (vo->videoPlane[ch].timemode.FrameRate)
			sleeptime = (1000000 + (vo->videoPlane[ch].timemode.FrameRate - 1)) / vo->videoPlane[ch].timemode.FrameRate;

		msleep(sleeptime);

		if ((cnt == 0) || (crc == rtd_inl(OSDOVL_Mixer_CRC_Result_reg))) {
			crc = rtd_inl(OSDOVL_Mixer_CRC_Result_reg);
			cnt = cnt + 1;
		}
	}
	while ((cnt < stable_count) && (rtd_inl(TIMER_SCPU_CLK90K_LO_reg) - starttime < 90 * 200));
	if (cnt < stable_count)
		rtd_pr_vo_err("[vo] get crc timeout !\n");

	mixer_crc_ctrl.regValue = rtd_inl(OSDOVL_Mixer_CRC_Ctrl_reg);
	mixer_crc_ctrl.crc_start = 0;
	rtd_outl(OSDOVL_Mixer_CRC_Ctrl_reg, mixer_crc_ctrl.regValue);

	return crc;
}

bool Mixer_CheckCRC(unsigned int ch, unsigned int golden)
{
	unsigned int crc = 0;
	unsigned int starttime = 0;
	int ret = true;

	starttime = rtd_inl(TIMER_SCPU_CLK90K_LO_reg);
	do {
		crc = Mixer_GetCRC(ch, 2);
	}
	while ((crc == golden) && (rtd_inl(TIMER_SCPU_CLK90K_LO_reg) - starttime < 90 * 1000));
	if (crc == golden) {
		ret = false;
		rtd_pr_vo_err("[vo] check crc timeout !\n");
	}

	return ret;
}

/**
* @brief display intput image
* @param   ch  [in]    channel port number to connect with display engine
* @param   image   [in]    image to display at video
* @return if success HAL_VO_STATE_OK, else one of error in HAL_VO_STATE_T.
**/
HAL_VO_STATE_T VO_DisplayPicture(unsigned int ch, HAL_VO_IMAGE_T image, bool bRedraw)
{
	VO_DATA *vo = &vodma_data[0];
	VO_TIMING timemode;
	int ret = 0;
//	unsigned int buf_w, buf_h, buf_size;
//	unsigned int width, height;
#if 0
	int i;
	unsigned char dc_idx_v1_y_curr, dc_idx_v1_c_curr, dc_idx_v1_y_curr2, dc_idx_v1_c_curr2;
	unsigned char dc_idx_v2_y_curr, dc_idx_v2_c_curr, dc_idx_v2_y_curr2, dc_idx_v2_c_curr2;
	unsigned char dc_idx_y_tmp, dc_idx_c_tmp;
	unsigned char dcu_idx_y, dcu_idx_c;
	unsigned char dcu_idx_y_tmp, dcu_idx_c_tmp;
	unsigned char dcu_idx_y_out, dcu_idx_c_out;
	bool bDoAlpha;
#endif
	vodma_vodma_reg_db_ctrl_RBUS vodma_reg_db_ctrl;
	unsigned int crc;

	#ifdef CONFIG_ARM64
	rtd_pr_vo_info("[vo]kernel VO_DisplayPicture: ch %d, image %lx, redraw %d\n", ch, (unsigned long)image.buf, bRedraw);
	#else
	rtd_pr_vo_info("[vo]kernel VO_DisplayPicture: ch %d, image %x, redraw %d\n", ch, (unsigned int)image.buf, bRedraw);
	#endif

	if (!(((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_JPEG) && (SLR_MAIN_DISPLAY == ch)) ||
	      ((Get_DisplayMode_Src(SLR_SUB_DISPLAY) == VSC_INPUTSRC_JPEG) && (SLR_SUB_DISPLAY == ch)))) {
		rtd_pr_vo_err("[vo] error: not correct ch %d, m:%d,s:%d\n", ch, Get_DisplayMode_Src(SLR_MAIN_DISPLAY), Get_DisplayMode_Src(SLR_SUB_DISPLAY));
		return HAL_VO_STATE_OK;
	}

	if (ch >= VODMA_DEV_NUM) {
		rtd_pr_vo_err("[vo] error: ch %d doesn't exist\n", ch);
		return HAL_VO_STATE_NOT_AVAILABLE;
	}

	if (vo->vo_state[ch] == VO_STATE_INIT) {
		rtd_pr_vo_err("[vo] error: not open yet\n");
		return HAL_VO_STATE_ERROR;
	}

	if (!image.buf) {
		#ifdef CONFIG_ARM64
		rtd_pr_vo_err("[vo] error: image buffer %lx doesn't exist\n", (unsigned long)image.buf);
		#else
		rtd_pr_vo_err("[vo] error: image buffer %x doesn't exist\n", (unsigned int)image.buf);
		#endif
		return HAL_VO_STATE_INVALID_PARAM;
	}

	if (image.pixel_format >= HAL_VO_PIXEL_FORMAT_MAX || image.pixel_format <= HAL_VO_PIXEL_FORMAT_NONE) {
		rtd_pr_vo_err("[vo] error: image pixel_format %d doesn't exist\n", image.pixel_format);
		return HAL_VO_STATE_INVALID_PARAM;
	}

	if ((image.pixel_format == HAL_VO_PIXEL_FORMAT_PALETTE) ||
	    (image.pixel_format == HAL_VO_PIXEL_FORMAT_YUV444P) ||
	    (image.pixel_format == HAL_VO_PIXEL_FORMAT_YUV422P) ||
	    (image.pixel_format == HAL_VO_PIXEL_FORMAT_YUV420P) ||
	    (image.pixel_format == HAL_VO_PIXEL_FORMAT_YUV224P) ||
	    (image.pixel_format == HAL_VO_PIXEL_FORMAT_YUV224I)) {
		rtd_pr_vo_err("[vo] error: unsupport pixel_format %d\n", image.pixel_format);
		return HAL_VO_STATE_INVALID_PARAM;
	}

	if (image.rect.w <= MIN_VO_WIDTH ||
	    image.rect.h <= MIN_VO_HEIGHT ||
	    image.rect.w > MAX_VO_WIDTH ||
	    image.rect.h > MAX_VO_HEIGHT) {
		rtd_pr_vo_err("[vo] error: image rect %d/%d/%d/%d invalid\n", image.rect.x, image.rect.y, image.rect.w, image.rect.h);
		return HAL_VO_STATE_INVALID_PARAM;
	}

	if (vo->alpha_blend[ch] == 0)
		crc = Mixer_GetCRC(ch, 2);

	memcpy(&vo->image[ch], &image, sizeof(HAL_VO_IMAGE_T));
	vo->image[ch].rect.x &= ~1;
	if (image.pixel_format == HAL_VO_PIXEL_FORMAT_YUV444SP)
		vo->image[ch].rect.x &= ~0x1f;

	#ifdef CONFIG_ARM64
	rtd_pr_vo_info("[vo] image: buf %lx\n", (unsigned long)vo->image[ch].buf);
	rtd_pr_vo_info("[vo] image: ofs_y %lx\n", (unsigned long)vo->image[ch].ofs_y);
	rtd_pr_vo_info("[vo] image: ofs_uv %lx\n", (unsigned long)vo->image[ch].ofs_uv);
	rtd_pr_vo_info("[vo] image: len_buf %d\n", vo->image[ch].len_buf);
	rtd_pr_vo_info("[vo] image: stride %d\n", vo->image[ch].stride);
	rtd_pr_vo_info("[vo] image: rect %d/%d/%d/%d\n", vo->image[ch].rect.x, vo->image[ch].rect.y, vo->image[ch].rect.w, vo->image[ch].rect.h);
	rtd_pr_vo_info("[vo] image: pixel_format %d\n", vo->image[ch].pixel_format);
	#else
	rtd_pr_vo_info("[vo] image: buf %x\n", (unsigned int)vo->image[ch].buf);
	rtd_pr_vo_info("[vo] image: ofs_y %x\n", (unsigned int)vo->image[ch].ofs_y);
	rtd_pr_vo_info("[vo] image: ofs_uv %x\n", (unsigned int)vo->image[ch].ofs_uv);
	rtd_pr_vo_info("[vo] image: len_buf %d\n", vo->image[ch].len_buf);
	rtd_pr_vo_info("[vo] image: stride %d\n", vo->image[ch].stride);
	rtd_pr_vo_info("[vo] image: rect %d/%d/%d/%d\n", vo->image[ch].rect.x, vo->image[ch].rect.y, vo->image[ch].rect.w, vo->image[ch].rect.h);
	rtd_pr_vo_info("[vo] image: pixel_format %d\n", vo->image[ch].pixel_format);
	#endif

	if (image.pixel_format == HAL_VO_PIXEL_FORMAT_YUV420I || image.pixel_format == HAL_VO_PIXEL_FORMAT_YUV422I) {
        rtd_pr_vo_emerg("*** [ERROR] SHOULD NOT BE HERE@%s.%d ***\n", __FUNCTION__, __LINE__);
#if 0 // not used now
		width = vo->image[ch].stride;
		height = (vo->image[ch].rect.y + vo->image[ch].rect.h);
		buf_w = (((vo->image[ch].stride + 511)>>9)<<9);
		buf_h = ((((vo->image[ch].rect.y + vo->image[ch].rect.h) + 63)>>6)<<6);

		buf_size = buf_w * buf_h; /* buffer sizef for seq yuv420i */
		if ((buf_size*2) > vo->buffer_size[ch][0]) {
			if (image.pixel_format == HAL_VO_PIXEL_FORMAT_YUV420I) {
				vo->buffer_size[ch][0] = (buf_size*2); /* y*2  */
				vo->buffer_size[ch][1] = (buf_size); /* c*2  */
				vo->buffer_size[ch][2] = (buf_size*3/2); /* y&c for tmp pic*/
			} else if (image.pixel_format == HAL_VO_PIXEL_FORMAT_YUV422I) {
				vo->buffer_size[ch][0] = (buf_size*2); /* y*2  */
				vo->buffer_size[ch][1] = (buf_size*2); /* c*2  */
				vo->buffer_size[ch][2] = (buf_size*2); /* y&c for tmp pic*/
			}

			if (vo_malloc_photo_buffer(ch, vo->phy_addr[ch], vo->buffer_size[ch]) == -1)
				return HAL_VO_STATE_NOT_AVAILABLE;

			vo->phy_addr_y[ch][0] = vo->phy_addr[ch][0];
			vo->phy_addr_y[ch][1] = vo->phy_addr[ch][0] + buf_size;
			vo->phy_addr_c[ch][0] = vo->phy_addr[ch][1];
			if (image.pixel_format == HAL_VO_PIXEL_FORMAT_YUV420I)
				vo->phy_addr_c[ch][1] = vo->phy_addr[ch][1] + (buf_size/2);
			else if (image.pixel_format == HAL_VO_PIXEL_FORMAT_YUV422I)
				vo->phy_addr_c[ch][1] = vo->phy_addr[ch][1] + buf_size;

			if (vo->phy_addr[ch][2]) {
				vo->phy_addr_y[ch][2] = vo->phy_addr[ch][2];
				vo->phy_addr_c[ch][2] = vo->phy_addr[ch][2] + buf_size;
			} else {
				vo->phy_addr_y[ch][2] = vo->phy_addr_y[ch][vo->buf_idx[ch]];
				vo->phy_addr_c[ch][2] = vo->phy_addr_c[ch][vo->buf_idx[ch]];
			}
		}
		rtd_pr_vo_info("[vo] buf: %d/%d/%d, phy_addr[%d][%d]: %x/%x\n", buf_w, buf_h, buf_size, ch, vo->buf_idx[ch], (unsigned int)vo->phy_addr_y[ch][vo->buf_idx[ch]], (unsigned int)vo->phy_addr_c[ch][vo->buf_idx[ch]]);

		width = (((width + 511)>>9)<<9);
		height = (((height + 63)>>6)<<6);

		dc_idx_v1_y_curr = (in_cma_range(NULL, (unsigned long)(vo->phy_addr_y[0][0] >> PAGE_SHIFT))) ? VP_DC1_IDX_VOUT_V1_Y_CURR : VP_DC_IDX_VOUT_V1_Y_CURR;
		dc_idx_v1_c_curr = (in_cma_range(NULL, (unsigned long)(vo->phy_addr_c[0][0] >> PAGE_SHIFT))) ? VP_DC1_IDX_VOUT_V1_C_CURR : VP_DC_IDX_VOUT_V1_C_CURR;
		dc_idx_v1_y_curr2 = (in_cma_range(NULL, (unsigned long)(vo->phy_addr_y[0][1] >> PAGE_SHIFT))) ? VP_DC1_IDX_VOUT_V1_Y_CURR2 : VP_DC_IDX_VOUT_V1_Y_CURR2;
		dc_idx_v1_c_curr2 = (in_cma_range(NULL, (unsigned long)(vo->phy_addr_c[0][1] >> PAGE_SHIFT))) ? VP_DC1_IDX_VOUT_V1_C_CURR2 : VP_DC_IDX_VOUT_V1_C_CURR2;
		dc_idx_v2_y_curr = (in_cma_range(NULL, (unsigned long)(vo->phy_addr_y[1][0] >> PAGE_SHIFT))) ? VP_DC1_IDX_VOUT_V2_Y_CURR : VP_DC_IDX_VOUT_V2_Y_CURR;
		dc_idx_v2_c_curr = (in_cma_range(NULL, (unsigned long)(vo->phy_addr_c[1][0] >> PAGE_SHIFT))) ? VP_DC1_IDX_VOUT_V2_C_CURR : VP_DC_IDX_VOUT_V2_C_CURR;
		dc_idx_v2_y_curr2 = (in_cma_range(NULL, (unsigned long)(vo->phy_addr_y[1][1] >> PAGE_SHIFT))) ? VP_DC1_IDX_VOUT_V2_Y_CURR2 : VP_DC_IDX_VOUT_V2_Y_CURR2;
		dc_idx_v2_c_curr2 = (in_cma_range(NULL, (unsigned long)(vo->phy_addr_c[1][1] >> PAGE_SHIFT))) ? VP_DC1_IDX_VOUT_V2_C_CURR2 : VP_DC_IDX_VOUT_V2_C_CURR2;
		dc_idx_y_tmp = (in_cma_range(NULL, (unsigned long)(vo->phy_addr_y[ch][(VODMA_BUF_NUM - 1)] >> PAGE_SHIFT))) ? VP_DC1_IDX_VOUT_TMP_Y_CURR : VP_DC_IDX_VOUT_TMP_Y_CURR;
		dc_idx_c_tmp = (in_cma_range(NULL, (unsigned long)(vo->phy_addr_c[ch][(VODMA_BUF_NUM - 1)] >> PAGE_SHIFT))) ? VP_DC1_IDX_VOUT_TMP_C_CURR : VP_DC_IDX_VOUT_TMP_C_CURR;

		dcu_idx_y = ((ch == 0) ? ((vo->buf_idx[0]) ? dc_idx_v1_y_curr2 : dc_idx_v1_y_curr) :
					 ((vo->buf_idx[1]) ? dc_idx_v2_y_curr2 : dc_idx_v2_y_curr));
		dcu_idx_c = ((ch == 0) ? ((vo->buf_idx[0]) ? dc_idx_v1_c_curr2 : dc_idx_v1_c_curr) :
					 ((vo->buf_idx[1]) ? dc_idx_v2_c_curr2 : dc_idx_v2_c_curr));
		vodma_DCU_Set(dcu_idx_y, (vo->phy_addr_y[ch][vo->buf_idx[ch]]), width, 0, 0);
		vodma_DCU_Set(dcu_idx_c, (vo->phy_addr_c[ch][vo->buf_idx[ch]]), width, 0, 0);

		dcu_idx_y_tmp = dc_idx_y_tmp;
		dcu_idx_c_tmp = dc_idx_c_tmp;
		vodma_DCU_Set(dcu_idx_y_tmp, (vo->phy_addr_y[ch][(VODMA_BUF_NUM - 1)]), width, 0, 0);
		vodma_DCU_Set(dcu_idx_c_tmp, (vo->phy_addr_c[ch][(VODMA_BUF_NUM - 1)]), width, 0, 0);

		if (vo->alpha_blend[ch] != 255 && vo->alpha_blend[ch] != 0 && vo->phy_addr[ch][(VODMA_BUF_NUM - 1)])
			bDoAlpha = true;
		else
			bDoAlpha = false;

		/* convert to yuv420i */
		if (bDoAlpha) {
			dcu_idx_y_out = dcu_idx_y_tmp;
			dcu_idx_c_out = dcu_idx_c_tmp;
		} else {
			dcu_idx_y_out = dcu_idx_y;
			dcu_idx_c_out = dcu_idx_c;
		}
		rtd_pr_vo_info("[vo] Sequential2Block: ch %d, dst %x/%x\n", ch,
			 ((bDoAlpha) ? (unsigned int)vo->phy_addr_y[ch][(VODMA_BUF_NUM - 1)] : (unsigned int)vo->phy_addr_y[ch][vo->buf_idx[ch]]),
			 ((bDoAlpha) ? (unsigned int)vo->phy_addr_c[ch][(VODMA_BUF_NUM - 1)] : (unsigned int)vo->phy_addr_c[ch][vo->buf_idx[ch]]));
		VP_Sequential2Block(dcu_idx_y_out, (unsigned int)vo->image[ch].ofs_y, width, height, vo->image[ch].stride, 0, 0);
		if (image.pixel_format == HAL_VO_PIXEL_FORMAT_YUV420I)
			VP_Sequential2Block(dcu_idx_c_out, (unsigned int)(vo->image[ch].ofs_uv), width, (height+1)>>1, vo->image[ch].stride, 0, 0);
		else
			VP_Sequential2Block(dcu_idx_c_out, (unsigned int)(vo->image[ch].ofs_uv), width, height, vo->image[ch].stride, 0, 0);

		/* do alpha */
		if (bDoAlpha) {
			rtd_pr_vo_info("[vo] Alpha: ch %d, alpha %d, src %x/%x, dst %x/%x\n", ch, vo->alpha_blend[ch],
				 (unsigned int)(vo->phy_addr_y[ch][(VODMA_BUF_NUM - 1)]), (unsigned int)(vo->phy_addr_c[ch][(VODMA_BUF_NUM - 1)]),
				 (unsigned int)(vo->phy_addr_y[ch][vo->buf_idx[ch]]), (unsigned int)(vo->phy_addr_c[ch][vo->buf_idx[ch]]));
			for (i = 0; i < width; i += MAX_SE_ALPHA_WIDTH) {
				vodma_DCU_Set(dcu_idx_y_tmp, (vo->phy_addr_y[ch][(VODMA_BUF_NUM - 1)]), width, i, 0);
				vodma_DCU_Set(dcu_idx_c_tmp, (vo->phy_addr_c[ch][(VODMA_BUF_NUM - 1)]), width, i, 0);
				vodma_DCU_Set(dcu_idx_y, (vo->phy_addr_y[ch][vo->buf_idx[ch]]), width, i, 0);
				vodma_DCU_Set(dcu_idx_c, (vo->phy_addr_c[ch][vo->buf_idx[ch]]), width, i, 0);
				SE_Alpha(dcu_idx_y_tmp, dcu_idx_c_tmp, dcu_idx_y, dcu_idx_c, (((width - i) > MAX_SE_ALPHA_WIDTH) ? MAX_SE_ALPHA_WIDTH : (width - i)), height, vo->alpha_blend[ch]);
			}
		}

		if (!bRedraw) {
			vo->displayWin[ch].x = vo->image[ch].rect.x;
			vo->displayWin[ch].y = vo->image[ch].rect.y;
			vo->displayWin[ch].w = vo->image[ch].rect.w;
			vo->displayWin[ch].h = vo->image[ch].rect.h;
		}

		/* set dcu offset */
		vodma_DCU_Set(dcu_idx_y, (vo->phy_addr_y[ch][vo->buf_idx[ch]]), width, 0, 0);
		vodma_DCU_Set(dcu_idx_c, (vo->phy_addr_c[ch][vo->buf_idx[ch]]), width, 0, 0);

		/* //k3l support h-offset up to 8192, no need dcu offset */
		/*
		if (vo->displayWin[ch].x >= 2048) {
			vodma_DCU_Set(dcu_idx_y, (vo->phy_addr_y[ch][vo->buf_idx[ch]]), width, ((vo->displayWin[ch].x>>11)<<11), 0);
			vodma_DCU_Set(dcu_idx_c, (vo->phy_addr_c[ch][vo->buf_idx[ch]]), width, ((vo->displayWin[ch].x>>11)<<11), 0);
			vo->displayWin[ch].x -= ((vo->displayWin[ch].x>>11)<<11);
		}
		*/
#endif
	} else
	{

		vo->displayWin[ch].x = vo->image[ch].rect.x;
		vo->displayWin[ch].y = vo->image[ch].rect.y;
		vo->displayWin[ch].w = vo->image[ch].rect.w;
		vo->displayWin[ch].h = vo->image[ch].rect.h;
	}

	vo->videoPlane[ch].Ddomain_Chroma_fmt = ((vo->image[ch].pixel_format == HAL_VO_PIXEL_FORMAT_RGB) ||
						(vo->image[ch].pixel_format == HAL_VO_PIXEL_FORMAT_BGR) ||
						(vo->image[ch].pixel_format == HAL_VO_PIXEL_FORMAT_ARGB) ||
						(vo->image[ch].pixel_format == HAL_VO_PIXEL_FORMAT_ABGR) ||
						(vo->image[ch].pixel_format == HAL_VO_PIXEL_FORMAT_YUV444P) ||
						(vo->image[ch].pixel_format == HAL_VO_PIXEL_FORMAT_YUV444I) ||
						vo->image[ch].pixel_format == HAL_VO_PIXEL_FORMAT_YUV444SP)
						? FMT_444 : FMT_422;

#ifdef __DOLBY_VTOP_422TO444__
	/* //use v_top to transfer 422to444 */
	vodma_set_dolby_vtop_422to444(vo->videoPlane[ch].Ddomain_Chroma_fmt == FMT_422);
	vo->videoPlane[ch].Ddomain_Chroma_fmt = FMT_444;
#endif

	timemode.HWidth = vodma_SeqWidthAlign(vo->displayWin[ch].w, vo->image[ch].pixel_format);
	timemode.VHeight = vo->displayWin[ch].h;
	timemode.HStartPos = (timemode.HWidth == 3840) ? 320 : ((timemode.HWidth > 3840 && timemode.HWidth <= 4096) ? ((4400 - timemode.HWidth) / 2) : 140);
	timemode.VStartPos = 26;
	timemode.isProg = 1;
	if (photo_vo_overscan_flag) {
		timemode.HTotal = photo_h_total;
		timemode.VTotal = photo_v_total;
	} else {
		timemode.HTotal = timemode.HWidth + (timemode.HStartPos * 2);
		timemode.VTotal = timemode.VHeight + (timemode.VStartPos * 2);
		photo_h_total = timemode.HTotal;
		photo_v_total = timemode.VTotal;
	}
	timemode.FrameRate = 30000;
	timemode.SampleRate = (timemode.HTotal * timemode.VTotal * (timemode.FrameRate / 1000) + 9999) / 10000;

	rtd_pr_vo_info("[vo]vodisplaypicture kernel  timemode:\n");
	rtd_pr_vo_info("[vo] HStartPos %d\n", timemode.HStartPos);
	rtd_pr_vo_info("[vo] VStartPos %d\n", timemode.VStartPos);
	rtd_pr_vo_info("[vo] HWidth %d\n", timemode.HWidth);
	rtd_pr_vo_info("[vo] VHeight %d\n", timemode.VHeight);
	rtd_pr_vo_info("[vo] HTotal %d\n", timemode.HTotal);
	rtd_pr_vo_info("[vo] VTotal %d\n", timemode.VTotal);
	rtd_pr_vo_info("[vo] isProg %d\n", timemode.isProg);
	rtd_pr_vo_info("[vo] FrameRate %d\n", timemode.FrameRate);
	rtd_pr_vo_info("[vo] SampleRate %d\n", timemode.SampleRate);

	memcpy(&vo->videoPlane[ch].timemode, &timemode, sizeof(VO_TIMING));

	timemode.HWidth = vo->displayWin[ch].w;
	down(&VO_InfoSemaphore);

	if (!(((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_JPEG) && (SLR_MAIN_DISPLAY == ch)) ||
	      ((Get_DisplayMode_Src(SLR_SUB_DISPLAY) == VSC_INPUTSRC_JPEG) && (SLR_SUB_DISPLAY == ch)))) {
		rtd_pr_vo_err("[vo] error: not correct ch %d, m:%d,s:%d\n", ch, Get_DisplayMode_Src(SLR_MAIN_DISPLAY), Get_DisplayMode_Src(SLR_SUB_DISPLAY));
		up(&VO_InfoSemaphore);
		return HAL_VO_STATE_OK;
	}

	vodma_reg_db_ctrl.regValue = rtd_inl(VODMA_VODMA_REG_DB_CTRL_reg);
	vodma_reg_db_ctrl.vodma_db_rdy = 0;
	vodma_reg_db_ctrl.vodmavsg_db_rdy = 0;
	rtd_outl(VODMA_VODMA_REG_DB_CTRL_reg, vodma_reg_db_ctrl.regValue);

	if ((vo->vo_state[ch] == VO_STATE_OPEN) || (vomda_set_voinfo(ch, &timemode, 1) == S_OK)) {
		timemode.HWidth = vodma_SeqWidthAlign(vo->displayWin[ch].w, vo->image[ch].pixel_format);

		/* reset sensio */
		ret = vodma_set_sensio();

		/* timing gen */
		ret = vodma_set_timinggen(&timemode);

		/* vsync gen */
		ret = vodma_set_vsyncgen(ch, &timemode);

		/* dma */
		ret = vodma_set_dma(ch, &timemode);
            rtd_outl(VODMA_VODMA_V1_SEQ_3D_L1_reg, vo->updare_seq_ctrl[ch].DMA_v1_staddr.regValue);
            rtd_outl(VODMA_VODMA_V1_LINE_BLK1_reg, vo->updare_seq_ctrl[ch].V1_y_line_sel.regValue);
            rtd_outl(VODMA_VODMA_V1_LINE_BLK2_reg, vo->updare_seq_ctrl[ch].V1_c_line_sel.regValue);

		timemode.HWidth = vo->displayWin[ch].w;
		vomda_set_voinfo(ch, &timemode, 1);
		set_vo_run_smooth_toggle_enable(photo_vo_overscan_flag, SLR_MAIN_DISPLAY);
		Config_VO_Dispinfo(&vo->videoPlane[ch].VOInfo);
		if (force_change_vo_flag[ch])
			force_change_vo_flag[ch] = FALSE;
	} else {
		if (force_change_vo_flag[ch]) {
			rtd_pr_vo_info("\r\n####force_change_vo_flag. set vo_info_change_flag TRUE####\r\n");
			vo_info_change_flag[ch] = TRUE;
			force_change_vo_flag[ch] = FALSE;
		}
	}

	/* dma */
	timemode.HWidth = vodma_SeqWidthAlign(vo->displayWin[ch].w, vo->image[ch].pixel_format);
	ret = vodma_set_dma(ch, &timemode);

	vodma_reg_db_ctrl.regValue = rtd_inl(VODMA_VODMA_REG_DB_CTRL_reg);
	vodma_reg_db_ctrl.vodma_db_rdy = 1;
	vodma_reg_db_ctrl.vodmavsg_db_rdy = 1;
	rtd_outl(VODMA_VODMA_REG_DB_CTRL_reg, vodma_reg_db_ctrl.regValue);

	up(&VO_InfoSemaphore);

	vo->vo_state[ch] = VO_STATE_DISPLAY;

	vo->buf_idx[ch] = vo->buf_idx[ch] ^ 1;

	/* make sure setting of new picture applied before alpha increase */
	if (vo->alpha_blend[ch] == 0) {
		vodma_waitApply();
		Mixer_CheckCRC(ch, crc);
	}
	rtd_pr_vo_info("[vo]kernel return OK,ret=%d\n", ret);
	return HAL_VO_STATE_OK;
}
int VO_Set_Photo_Update_ShareMEM_Flag(unsigned int enable)
{
    unsigned int *photo_update_sharemem = NULL;
    photo_update_sharemem =  (unsigned int *)Scaler_GetShareMemVirAddr(SCALERIOC_PHOTO_UPDATE_SHAREMEM_FALG);
    *photo_update_sharemem = 1;
    return 0;
}
int VO_Set_Photo_Shift_ShareMEM_Value(unsigned int value)
{
   unsigned int *photo_shift_value_sharemem = NULL;
   photo_shift_value_sharemem = (unsigned int *)Scaler_GetShareMemVirAddr(SCALERIOC_PHOTO_SHIFT_SHAREMEM_INFO);
   *photo_shift_value_sharemem = Scaler_ChangeUINT32Endian(value);
   return 0;
}
void VO_Wait_Set_Photo_Shift_ShareMEM_finish(void)
{
	unsigned int timeout = 3;
	unsigned int *photo_update_sharemem;
	photo_update_sharemem = (unsigned int *)Scaler_GetShareMemVirAddr(SCALERIOC_PHOTO_UPDATE_SHAREMEM_FALG);

		while(timeout) {
			if(*photo_update_sharemem == 0){
				break;
			}
			else{
				rtd_pr_vo_err("\r\n####\r\n");
				msleep(5);
				timeout--;
			}

		}
		rtd_pr_vo_err("\r\n####timeout =%d\r\n",timeout);
	*photo_update_sharemem = 1;
	if(timeout == 0)
		rtd_pr_vo_err("\r\n####VO_Wait_Set_Photo_Shift_ShareMEM_finish####\r\n");
}

/**
* @brief re-flush video output
* @param   ch  [in]    channel port number to connect with display engine
* @return if success HAL_VO_STATE_OK, else one of error in HAL_VO_STATE_T.
**/
HAL_VO_STATE_T VO_RedrawPicture(unsigned int ch)
{
	VO_DATA *vo = &vodma_data[0];
	int ret = 0;

	rtd_pr_vo_info("[vo] VO_RedrawPicture: ch %d\n", ch);

	if (ch >= VODMA_DEV_NUM) {
		rtd_pr_vo_err("[vo] error: ch %d doesn't exist\n", ch);
		return HAL_VO_STATE_NOT_AVAILABLE;
	}

	if (vo->vo_state[ch] == VO_STATE_INIT) {
		rtd_pr_vo_err("[vo] error: not open yet\n");
		return HAL_VO_STATE_ERROR;
	}

	if (vo->vo_state[ch] == VO_STATE_OPEN) {
		rtd_pr_vo_err("[vo] error: not display yet\n");
		return HAL_VO_STATE_ERROR;
	}

	if (!&vo->image[ch]) {
		rtd_pr_vo_err("[vo] error: image %lx doesn't exist\n", (unsigned long)&vo->image[ch]);
		return HAL_VO_STATE_INVALID_PARAM;
	}

	if (!vo->image[ch].buf) {
		rtd_pr_vo_err("[vo] error: image buffer %lx doesn't exist\n", (unsigned long)vo->image[ch].buf);
		return HAL_VO_STATE_INVALID_PARAM;
	}

	if (vo->image[ch].pixel_format >= HAL_VO_PIXEL_FORMAT_MAX || vo->image[ch].pixel_format <= HAL_VO_PIXEL_FORMAT_NONE) {
		rtd_pr_vo_err("[vo] error: image pixel_format %d doesn't exist\n", vo->image[ch].pixel_format);
		return HAL_VO_STATE_INVALID_PARAM;
	}

	if (vo->image[ch].rect.w <= MIN_VO_WIDTH ||
	    vo->image[ch].rect.h <= MIN_VO_HEIGHT ||
	    vo->image[ch].rect.w > MAX_VO_WIDTH ||
	    vo->image[ch].rect.h > MAX_VO_HEIGHT) {
		rtd_pr_vo_err("[vo] error: image rect %d/%d/%d/%d invalid\n", vo->image[ch].rect.x, vo->image[ch].rect.y, vo->image[ch].rect.w, vo->image[ch].rect.h);
		return HAL_VO_STATE_INVALID_PARAM;
	}

	if (get_vo_nosignal_flag(ch))
		return HAL_VO_STATE_OK;

	ret = VO_DisplayPicture(ch, vo->image[ch], true);

	return ret;
}

extern void zoom_photo_smooth_toggle_proc(unsigned char display, StructDisplayInfo *vo_disp);
#define INT_POS 16
HAL_VO_STATE_T vo_SetInputRegion(unsigned int ch, unsigned char display, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
	VO_DATA *vo = &vodma_data[0];
	unsigned int line_cnt;
	vodma_vodma_line_st_RBUS v1_line_status_reg;
	unsigned int line_check = (vo->videoPlane[ch].timemode.VHeight + vo->videoPlane[ch].timemode.VStartPos - 16) * 3/4;
	unsigned int timeout = 0xfffff;

	rtd_pr_vo_info("[vo] vo_SetInputRegion: ch %d, rect %d/%d/%d/%d\n", ch, x, y, w, h);
	vo->displayWin[ch].x = x & ~1;
	vo->displayWin[ch].y = y;
	vo->displayWin[ch].w = w;
	vo->displayWin[ch].h = h;
	photo_vo_overscan_flag = 1;

	while (timeout) {
		v1_line_status_reg.regValue = rtd_inl(VODMA_VODMA_LINE_ST_reg);
		line_cnt = v1_line_status_reg.line_cnt;
		if ((line_cnt < line_check) && (line_cnt >= INT_POS)) {
			if (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_FRAMESYNC)) {
				if (VO_RedrawPicture(ch) == 0)
					zoom_photo_smooth_toggle_proc(display, Get_VO_Dispinfo(ch));
			} else {
				zoom_photo_smooth_toggle_proc(display, Get_VO_Dispinfo(ch));
			}
			break;
		}
		timeout--;
	}
	photo_vo_overscan_flag = 0;

	return HAL_VO_STATE_OK;
}

/**
* @brief Close the video output module.
* @param   ch  [in]    channel port number to connect with display engine
* @return if success HAL_VO_STATE_OK, else one of error in HAL_VO_STATE_T.
**/
HAL_VO_STATE_T VO_Close(unsigned int ch)
{
	VO_DATA *vo = &vodma_data[0];
	rtd_pr_vo_info("[vo] VO_Close: ch %d\n", ch);

	if (ch >= VODMA_DEV_NUM) {
		rtd_pr_vo_err("[vo] error: ch %d doesn't exist\n", ch);
		return HAL_VO_STATE_NOT_AVAILABLE;
	}

	if (vo->vo_state[ch] == VO_STATE_INIT) {
		rtd_pr_vo_err("[vo] error: not open yet\n");
		return HAL_VO_STATE_OK;
	}
	set_vo_nosignal_flag(ch, 1);
	set_vo_EOS_flag(ch, 0);
	set_film_detect_done_flag(ch, 0);

	vo->vo_state[ch] = VO_STATE_INIT;

	//vo_free_photo_buffer(ch);

	vo_free_hal_buffer(ch);

	return HAL_VO_STATE_OK;
}

HAL_VO_STATE_T VO_SetPictureInfo(unsigned int ch,HAL_VO_IMAGE_T *data)
{
    VO_DATA *vo = &vodma_data[0];
	if (ch >= VODMA_DEV_NUM) {
		rtd_pr_vo_err("[vo] error: ch %d doesn't exist\n", ch);
		return HAL_VO_STATE_NOT_AVAILABLE;
	}
    vo->photoChannel = ch;
	memcpy(&vo->image[ch], data, sizeof(HAL_VO_IMAGE_T));
	return HAL_VO_STATE_OK;
}

HAL_VO_IMAGE_T *VO_GetPictureInfo(void)
{

	if (vodma_data->photoChannel >= VODMA_DEV_NUM) {
		rtd_pr_vo_err("[vo] %s error: ch %d doesn't exist\n", __func__, vodma_data->photoChannel);
		vodma_data->photoChannel = 0;
	}
    return &vodma_data->image[vodma_data->photoChannel];
}

int vodma_init_pll(void)
{
	unsigned int nMCode, nNCode, nOCode, regValue, busclk = 0;

	/* enable vodma pll */
	rtd_outl(PLL_REG_SYS_PLL_VODMA2_reg, 0x00000004);  /* PLLVODMA power down, hold reset, output disable */
	rtd_outl(PLL_REG_SYS_PLL_VODMA1_reg, 0x00014613);   /* max 600, set 27*(20+2)/(0+1) = 594MHz */
	rtd_outl(PLL_REG_SYS_PLL_VODMA2_reg, 0x00000005);  /* PLLVODMA power on */
	rtd_outl(PLL_REG_SYS_PLL_VODMA2_reg, 0x00000007);  /* release PLLVODMA reset */
	msleep(1); /* //ScalerTimer_DelayXms(0x10); // wait at least > 150us */
	rtd_outl(PLL_REG_SYS_PLL_VODMA2_reg, 0x00000003);  /* PLLVODMA output enable */

	regValue = rtd_inl(PLL_REG_SYS_PLL_VODMA1_reg);
	nMCode = ((regValue & 0x000ff000)>>12);
	nNCode = ((regValue & 0x00300000)>>20);
	nOCode = ((regValue & 0x00000180)>>7);
	busclk = 27000000*(nMCode+2)/(nNCode+1)/(nOCode+1);
	rtd_pr_vo_info("[vo] pll_vodma: %d/%d/%d/%d, %x\n", nMCode, nNCode, nOCode, busclk, rtd_inl(PLL_REG_SYS_PLL_VODMA1_reg));

	return 0;
}

int vodma_reset(void)
{
	/*
	* enable vodma clk
	* reset flow: CLKEN=0 -> RSTN=0 -> CLKEN=1 -> CLKEN=0 -> RSTN=1 -> CLKEN=1
	*/

	enum CRT_CLK clken = CLK_ON;

	CRT_CLK_OnOff(SCALER_VO_1, clken, NULL);

	return 0;
}

int vodma_waitApply(void)
{
	int timeout;
	int nloop;
	vodma_vodma_reg_db_ctrl_RBUS vodma_reg_db_ctrl;

	vodma_reg_db_ctrl.regValue = rtd_inl(VODMA_VODMA_REG_DB_CTRL_reg);

	timeout = 30;
	nloop = 0;
	while (vodma_reg_db_ctrl.vodma_db_rdy && nloop < timeout) {
		nloop++;
		msleep(10);
		vodma_reg_db_ctrl.regValue = rtd_inl(VODMA_VODMA_REG_DB_CTRL_reg);
	}

	if (nloop >= timeout)
		rtd_pr_vo_err("[vo] wait Apply timeout !\n");

	return 0;
}

int vodma_disable(void)
{
	vodma_vodma_v1_dcfg_RBUS DMA_v1_dcfg;
	vodma_vodma_clkgen_RBUS clkgen;
	vodma_vodma_pvs0_gen_RBUS pvs0;
	vodma_vodma_reg_db_ctrl_RBUS vodma_reg_db_ctrl;

	/* interrupt disable */
	rtd_outl(VODMA_VODMA_VGIP_INTPOS_reg, 0x0);
	rtd_outl(VODMA_VODMA_VGIP_INTST_reg, 0x7);

	/* vodma_go disable, stop fetch data */
	DMA_v1_dcfg.regValue = rtd_inl(VODMA_VODMA_V1_DCFG_reg);
	DMA_v1_dcfg.vodma_go = 0;
	rtd_outl(VODMA_VODMA_V1_DCFG_reg, DMA_v1_dcfg.regValue);

	vodma_reg_db_ctrl.regValue = rtd_inl(VODMA_VODMA_REG_DB_CTRL_reg);
	vodma_reg_db_ctrl.vodma_db_rdy = 1;
	rtd_outl(VODMA_VODMA_REG_DB_CTRL_reg, vodma_reg_db_ctrl.regValue);

	vodma_waitApply();

	/* clk disable */
	clkgen.regValue = rtd_inl(VODMA_VODMA_CLKGEN_reg);
	clkgen.vodma_enclk = 0;
	rtd_outl(VODMA_VODMA_CLKGEN_reg, clkgen.regValue);
	pvs0.regValue = rtd_inl(VODMA_VODMA_PVS0_Gen_reg);
	pvs0.en_pvgen = 0;
	rtd_outl(VODMA_VODMA_PVS0_Gen_reg, pvs0.regValue);

	vodma_reg_db_ctrl.regValue = rtd_inl(VODMA_VODMA_REG_DB_CTRL_reg);
	vodma_reg_db_ctrl.vodmavsg_db_rdy = 1;
	rtd_outl(VODMA_VODMA_REG_DB_CTRL_reg, vodma_reg_db_ctrl.regValue);

	return 0;
}

int vodma_SeqWidthAlign(int width, HAL_VO_PIXEL_FORMAT pxlfmt)
{
	unsigned int width_align = width;
	vodma_vodma_dma_option_RBUS dma_option;

	unsigned int data_bit_width = 8; /* dat bit fixed in 8 bit */
	unsigned char chroma_bit_width = 1;

	dma_option.regValue = rtd_inl(VODMA_VODMA_DMA_OPTION_reg);

	if (pxlfmt == HAL_VO_PIXEL_FORMAT_YUV422YUYV)
		chroma_bit_width = 2;
	else if (pxlfmt == HAL_VO_PIXEL_FORMAT_RGB || pxlfmt == HAL_VO_PIXEL_FORMAT_BGR || pxlfmt == HAL_VO_PIXEL_FORMAT_YUV444SP)
		chroma_bit_width = 3;
	else if (pxlfmt == HAL_VO_PIXEL_FORMAT_ARGB || pxlfmt == HAL_VO_PIXEL_FORMAT_ABGR)
		chroma_bit_width = 4;
	else
		chroma_bit_width = 1;

	switch (chroma_bit_width * data_bit_width) {
	case 16:
		if (dma_option.burst_len == 0) /* burst 32x128 bits */
			width_align = ((width + ((1 << 8) - 1)) >> 8) << 8; /* 256 pixel align */
		else
			width_align = ((width + ((1 << 9) - 1)) >> 9) << 9; /* 512 pixel align */
		break;
	/* // dat bit fixed in 8 bit */
	/*
	case 20:
		if (dma_option.burst_len == 0)  //burst 32x128 bits
			width_align = ((width + ((1 << 10) - 1)) >> 10) << 10; // 1024 pixel align
		else
			width_align = ((width + ((1 << 11) - 1)) >> 11) << 11; // 2048 pixel align
		break;
	*/
	case 24:
		if (dma_option.burst_len == 0) /* burst 32x128 bits */
			width_align = ((width + ((1 << 9) - 1)) >> 9) << 9; /* 512 pixel align */
		else
			width_align = ((width + ((1 << 10) - 1)) >> 10) << 10; /* 1024 pixel align */
		break;
	/* // dat bit fixed in 8 bit */
	/*
	case 30:
		if (dma_option.burst_len == 0) //burst 32x128 bits
			width_align = ((width + ((1 << 11) - 1)) >> 11) << 11; // 2048 pixel align
		else
			width_align = ((width + ((1 << 12) - 1)) >> 12) << 12; // 4096 pixel align
		break;
	*/
	case 32:
		if (dma_option.burst_len == 0) /* burst 32x128 bits */
			width_align = ((width + ((1 << 7) - 1)) >> 7) << 7; /* 128 pixel align */
		else
			width_align = ((width + ((1 << 8) - 1)) >> 8) << 8; /* 256 pixel align */
		break;
	default:
		break;
	}

	return width_align;
}

#ifdef __DOLBY_VTOP_422TO444__
void vodma_set_dolby_vtop_422to444(bool bEnable)
{
	hdr_all_top_top_ctl_RBUS top_ctl_reg;
	hdr_all_top_top_d_buf_RBUS top_d_buf_reg;

	top_ctl_reg.regValue = rtd_inl(HDR_ALL_TOP_TOP_CTL_reg);
	top_ctl_reg.en_422to444_1 = (bEnable ? 1 : 0);
	rtd_outl(HDR_ALL_TOP_TOP_CTL_reg, top_ctl_reg.regValue);

	top_d_buf_reg.regValue = rtd_inl(HDR_ALL_TOP_TOP_D_BUF_reg);
	top_d_buf_reg.dolby_double_apply = 1;
	rtd_outl(HDR_ALL_TOP_TOP_D_BUF_reg, top_d_buf_reg.regValue);
}
#endif

int updateVODMASetting(unsigned int ch)
{
	VO_DATA *vo = &vodma_data[0];
	vodma_vodma_reg_db_ctrl_RBUS    vodma_reg_db_ctrl;

	if (ch >= VODMA_DEV_NUM) {
		return 0;
	}

	if (!(((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_JPEG) && (SLR_MAIN_DISPLAY == ch)) ||
	      ((Get_DisplayMode_Src(SLR_SUB_DISPLAY) == VSC_INPUTSRC_JPEG) && (SLR_SUB_DISPLAY == ch)))) {
		return 0;
	}

	if (vo->vo_state[ch] == VO_STATE_INIT) {
		return 0;
	}

	if (vo->updare_seq_ctrl[ch].f_update && (rtd_inl(VODMA_VODMA_LINE_ST_reg) >= (vo->videoPlane[ch].timemode.VHeight + vo->videoPlane[ch].timemode.VStartPos))) {

		vo->updare_seq_ctrl[ch].f_update = 0;

            rtd_outl(VODMA_VODMA_V1_SEQ_3D_L1_reg, vo->updare_seq_ctrl[ch].DMA_v1_staddr.regValue);
            rtd_outl(VODMA_VODMA_V1_LINE_BLK1_reg, vo->updare_seq_ctrl[ch].V1_y_line_sel.regValue);
            rtd_outl(VODMA_VODMA_V1_LINE_BLK2_reg, vo->updare_seq_ctrl[ch].V1_c_line_sel.regValue);

		vodma_reg_db_ctrl.regValue = rtd_inl(VODMA_VODMA_REG_DB_CTRL_reg);
		vodma_reg_db_ctrl.vodma_db_rdy = 1;
		rtd_outl(VODMA_VODMA_REG_DB_CTRL_reg, vodma_reg_db_ctrl.regValue);
	}

	return 0;
}

int vodma_set_dma(unsigned int ch, VO_TIMING *tmode)
{
	VO_DATA *vo = &vodma_data[0];
	vodma_vodma_reg_db_ctrl_RBUS    vodma_reg_db_ctrl;
	vodma_vodma_v1_dcfg_RBUS        DMA_v1_dcfg;
	vodma_vodma_v1_dsize_RBUS       DMA_v1_dsize;
	vodma_vodma_v1_seq_3d_l1_RBUS DMA_v1_staddr;
	vodma_vodma_v1_blk1_RBUS        DMA_v1_blk1;
	vodma_vodma_v1_blk2_RBUS        DMA_v1_blk2;
	vodma_vodma_v1_line_blk1_RBUS        V1_y_line_sel ;        //0xb800_5008
	vodma_vodma_v1_line_blk2_RBUS        V1_c_line_sel ;        //0xb800_500C
	vodma_vodma_v1chroma_fmt_RBUS   V1_C_up_fmt;
	vodma_vodma_v1_cu_RBUS          V1_C_up_mode420;
	vodma_vodma_v1422_to_444_RBUS   V1_C_up_mode422;
	vodma_vodma_v1vgip_hact_RBUS    vodma_v1vgip_hact_reg;
	vodma_vodma_dma_option_RBUS     dma_option;
	unsigned char seq_color_swap = 0;
	unsigned char encfmt = 4;
	//unsigned char dc_idx_v1_y_curr, dc_idx_v1_c_curr, dc_idx_v1_y_curr2, dc_idx_v1_c_curr2;
	//unsigned char dc_idx_v2_y_curr, dc_idx_v2_c_curr, dc_idx_v2_y_curr2, dc_idx_v2_c_curr2;
	//unsigned char dcu_idx_y, dcu_idx_c;
	//unsigned int luma_x, luma_y, chroma_x, chroma_y;
	unsigned char data_bit_width, chroma_bit_width;
	unsigned int TotalBits;
	//unsigned int starttime;

	DMA_v1_dcfg.regValue = rtd_inl(VODMA_VODMA_V1_DCFG_reg);
	DMA_v1_dsize.regValue = rtd_inl(VODMA_VODMA_V1_DSIZE_reg);
	vodma_v1vgip_hact_reg.regValue = rtd_inl(VODMA_VODMA_V1VGIP_HACT_reg);
	DMA_v1_staddr.regValue = rtd_inl(VODMA_VODMA_V1_SEQ_3D_L1_reg);
	V1_y_line_sel.regValue = rtd_inl(VODMA_VODMA_V1_LINE_BLK1_reg) ;
	V1_c_line_sel.regValue = rtd_inl(VODMA_VODMA_V1_LINE_BLK2_reg) ;
	DMA_v1_blk1.regValue = rtd_inl(VODMA_VODMA_V1_BLK1_reg);
	DMA_v1_blk2.regValue = rtd_inl(VODMA_VODMA_V1_BLK2_reg);
	V1_C_up_fmt.regValue = rtd_inl(VODMA_VODMA_V1CHROMA_FMT_reg);
	V1_C_up_mode420.regValue = rtd_inl(VODMA_VODMA_V1_CU_reg);
	V1_C_up_mode422.regValue = rtd_inl(VODMA_VODMA_V1422_TO_444_reg);
	dma_option.regValue = rtd_inl(VODMA_VODMA_DMA_OPTION_reg);

	vodma_reg_db_ctrl.regValue = rtd_inl(VODMA_VODMA_REG_DB_CTRL_reg);
	vodma_reg_db_ctrl.vodma_db_rdy = 0;
	rtd_outl(VODMA_VODMA_REG_DB_CTRL_reg, vodma_reg_db_ctrl.regValue);

	if (vo->image[ch].pixel_format == HAL_VO_PIXEL_FORMAT_YUV422YUYV)
		chroma_bit_width = 2;
	else if (vo->image[ch].pixel_format == HAL_VO_PIXEL_FORMAT_RGB || vo->image[ch].pixel_format == HAL_VO_PIXEL_FORMAT_BGR || vo->image[ch].pixel_format == HAL_VO_PIXEL_FORMAT_YUV444SP)
		chroma_bit_width = 3;
	else if (vo->image[ch].pixel_format == HAL_VO_PIXEL_FORMAT_ARGB || vo->image[ch].pixel_format == HAL_VO_PIXEL_FORMAT_ABGR)
		chroma_bit_width = 4;
	else
		chroma_bit_width = 1;

	DMA_v1_dcfg.merge_forward = 0;
	DMA_v1_dcfg.merge_backward = 0;
	DMA_v1_dcfg.film_condition = 0;

	DMA_v1_dcfg.field_mode = 0;
	DMA_v1_dcfg.field_fw = 0;
	DMA_v1_dcfg.l_flag_fw = 0;
	DMA_v1_dcfg.force_2d_en = 0;
	DMA_v1_dcfg.force_2d_sel = 0;
	DMA_v1_dcfg.dma_auto_mode = 0;

	DMA_v1_dcfg.seq_data_pack_type = 0; /* Sequence Mode v_flip_en only support line base */
	DMA_v1_dcfg.seq_data_width = 0; /* 8 bits*/

	DMA_v1_dcfg.double_chroma = ((vo->image[ch].pixel_format == HAL_VO_PIXEL_FORMAT_YUV420I) || (vo->image[ch].pixel_format == HAL_VO_PIXEL_FORMAT_YUV420P)) ? 1 : 0;
	DMA_v1_dcfg.uv_off = (vo->image[ch].pixel_format == HAL_VO_PIXEL_FORMAT_GRAYSCALE || vo->image[ch].pixel_format == HAL_VO_PIXEL_FORMAT_YUV400 || vo->image[ch].pixel_format == HAL_VO_PIXEL_FORMAT_ALPHAGRAYSCALE) ? 1 : 0;
	DMA_v1_dcfg.pxl_swap_sel = 0; //not used in Merlin3
	DMA_v1_dcfg.inverse_128_en = 1; // 1: for VE's NV12
	DMA_v1_dcfg.keep_go_en = 0;
	DMA_v1_dcfg.dma_state_reset_en = 1;
	DMA_v1_dcfg.v_flip_en = 0;
	DMA_v1_dcfg.chroma_swap_en = 0;
	//DMA_v1_dcfg.double_pixel_mode = 0;//Mark2 removed

	DMA_v1_dcfg.dma_mode = (vo->image[ch].pixel_format == HAL_VO_PIXEL_FORMAT_YUV420I || vo->image[ch].pixel_format == HAL_VO_PIXEL_FORMAT_YUV422I || vo->image[ch].pixel_format == HAL_VO_PIXEL_FORMAT_YUV444I) ? 1 : 0;
	DMA_v1_dcfg.vodma_go = 1;

	if ((tmode->HWidth == 0) || (tmode->VHeight == 0)) {
		rtd_pr_vo_err("[vo] invalid w/h[%d/%d]\n", tmode->HWidth, tmode->VHeight);
		return 0;
	}
	DMA_v1_dsize.p_y_len = tmode->HWidth;
	DMA_v1_dsize.p_y_nline = tmode->VHeight;

	//dc_idx_v1_y_curr = (in_cma_range(NULL, vo->phy_addr_y[0][0] >> PAGE_SHIFT)) ? VP_DC1_IDX_VOUT_V1_Y_CURR : VP_DC_IDX_VOUT_V1_Y_CURR;
	//dc_idx_v1_c_curr = (in_cma_range(NULL, vo->phy_addr_c[0][0] >> PAGE_SHIFT)) ? VP_DC1_IDX_VOUT_V1_C_CURR : VP_DC_IDX_VOUT_V1_C_CURR;
	//dc_idx_v1_y_curr2 = (in_cma_range(NULL, vo->phy_addr_y[0][1] >> PAGE_SHIFT)) ? VP_DC1_IDX_VOUT_V1_Y_CURR2 : VP_DC_IDX_VOUT_V1_Y_CURR2;
	//dc_idx_v1_c_curr2 = (in_cma_range(NULL, vo->phy_addr_c[0][1] >> PAGE_SHIFT)) ? VP_DC1_IDX_VOUT_V1_C_CURR2 : VP_DC_IDX_VOUT_V1_C_CURR2;
	//dc_idx_v2_y_curr = (in_cma_range(NULL, vo->phy_addr_y[1][0] >> PAGE_SHIFT)) ? VP_DC1_IDX_VOUT_V2_Y_CURR : VP_DC_IDX_VOUT_V2_Y_CURR;
	//dc_idx_v2_c_curr = (in_cma_range(NULL, vo->phy_addr_c[1][0] >> PAGE_SHIFT)) ? VP_DC1_IDX_VOUT_V2_C_CURR : VP_DC_IDX_VOUT_V2_C_CURR;
	//dc_idx_v2_y_curr2 = (in_cma_range(NULL, vo->phy_addr_y[1][1] >> PAGE_SHIFT)) ? VP_DC1_IDX_VOUT_V2_Y_CURR2 : VP_DC_IDX_VOUT_V2_Y_CURR2;
	//dc_idx_v2_c_curr2 = (in_cma_range(NULL, vo->phy_addr_c[1][1] >> PAGE_SHIFT)) ? VP_DC1_IDX_VOUT_V2_C_CURR2 : VP_DC_IDX_VOUT_V2_C_CURR2;
	//dcu_idx_y = ((vo->buf_idx[ch]) ? ((ch == 0) ? dc_idx_v1_y_curr2 : dc_idx_v2_y_curr2) : ((ch == 0) ? dc_idx_v1_y_curr : dc_idx_v2_y_curr));
	//dcu_idx_c = ((vo->buf_idx[ch]) ? ((ch == 0) ? dc_idx_v1_c_curr2 : dc_idx_v2_c_curr2) : ((ch == 0) ? dc_idx_v1_c_curr : dc_idx_v2_c_curr));

	#ifdef CONFIG_ARM64
	DMA_v1_staddr.st_adr = (((unsigned long)(vo->image[ch].buf + (vo->displayWin[ch].y * vo->image[ch].stride * chroma_bit_width) + (vo->displayWin[ch].x * chroma_bit_width)) & ~0xf)) >> 4;
	#else
	DMA_v1_staddr.st_adr = (((unsigned int)(vo->image[ch].buf + (vo->displayWin[ch].y * vo->image[ch].stride * chroma_bit_width) + (vo->displayWin[ch].x * chroma_bit_width)) & ~0xf)) >> 4;
	#endif

	//luma_x = vo->displayWin[ch].x;
	//chroma_x = vo->displayWin[ch].x;
	//luma_y = vo->displayWin[ch].y;
	//chroma_y = luma_y/2;

	DMA_v1_blk1.y_v_offset = 0;//luma_y;
	DMA_v1_blk2.c_v_offset = 0;//chroma_y;

	vodma_v1vgip_hact_reg.h_end = tmode->HStartPos + tmode->HWidth;
	vodma_v1vgip_hact_reg.h_st = tmode->HStartPos;
	DMA_v1_dsize.p_y_len = tmode->HWidth;
	DMA_v1_dsize.p_y_nline = tmode->VHeight;

	if (1) { /* yuv444sp slide need line step */
		data_bit_width = (DMA_v1_dcfg.seq_data_width) ? 10 : 8;
		TotalBits = vo->image[ch].stride * data_bit_width * chroma_bit_width;
		DMA_v1_dcfg.v_flip_en = 1;
		V1_y_line_sel.y_auto_line_step = 0; // 0: FW mode, 1: HW mode
		V1_c_line_sel.c_auto_line_step = 0; // 0: FW mode, 1: HW mode
		V1_y_line_sel.y_line_step = -((TotalBits+127)/128);
		V1_c_line_sel.c_line_step = -((TotalBits+127)/128);
	} else {
		DMA_v1_dcfg.v_flip_en = 0;
            V1_y_line_sel.y_auto_line_step = 1; // 0: FW mode, 1: HW mode
            V1_c_line_sel.c_auto_line_step = 1; // 0: FW mode, 1: HW mode
		V1_y_line_sel.y_line_step = 0;
		V1_c_line_sel.c_line_step = 0;
	}

	V1_C_up_fmt.g_position = 0;
	V1_C_up_fmt.alpha_position = 0;
	switch (vo->image[ch].pixel_format) {
	case HAL_VO_PIXEL_FORMAT_YUV422YUYV:
		seq_color_swap = 2;
#ifdef __DOLBY_VTOP_422TO444__
		encfmt = 3; /* //use v_top to transfer 422to444 */
#else
		encfmt = (vo->videoPlane[ch].Ddomain_Chroma_fmt == FMT_444) ? 2 : 3;
#endif
		break;
	case HAL_VO_PIXEL_FORMAT_RGB:
		seq_color_swap = 0;
		encfmt = 4;
		break;
	case HAL_VO_PIXEL_FORMAT_YUV444SP:
		seq_color_swap = 4;
		encfmt = 4;
		break;
	case HAL_VO_PIXEL_FORMAT_BGR:
		seq_color_swap = 5;
		encfmt = 4;
		break;
	case HAL_VO_PIXEL_FORMAT_ABGR:
		seq_color_swap = 5;
		encfmt = 5;
		break;
	case HAL_VO_PIXEL_FORMAT_YUV420I:
#ifdef __DOLBY_VTOP_422TO444__
		if (DMA_v1_dcfg.double_chroma)
			encfmt = 3; /* //use v_top to transfer 422to444 */
		else
			encfmt = 0; /* //use v_top to transfer 422to444 */
#else
		if (DMA_v1_dcfg.double_chroma)
			encfmt = (vo->videoPlane[ch].Ddomain_Chroma_fmt == FMT_444) ? 2 : 3;
		else
			encfmt = (vo->videoPlane[ch].Ddomain_Chroma_fmt == FMT_444) ? 1 : 0;
#endif
		break;
	case HAL_VO_PIXEL_FORMAT_YUV422I:
#ifdef __DOLBY_VTOP_422TO444__
		encfmt = 3; /* //use v_top to transfer 422to444 */
#else
		encfmt = (vo->videoPlane[ch].Ddomain_Chroma_fmt == FMT_444) ? 2 : 3;
#endif
		break;
	case HAL_VO_PIXEL_FORMAT_ARGB:
	default:
		seq_color_swap = 0;
		encfmt = 5;
		break;
	}
	V1_C_up_fmt.seq_color_swap = seq_color_swap;
	V1_C_up_fmt.encfmt = encfmt;

	V1_C_up_mode420.mode = 0;
	V1_C_up_mode422.fir422_sel = 0;

	V1_y_line_sel.y_line_select = 0;
	V1_c_line_sel.c_line_select = 0;

	dma_option.regValue = rtd_inl(VODMA_VODMA_DMA_OPTION_reg);
	dma_option.burst_len = 1;
	DMA_v1_dcfg.seq_data_width = 0; // 8bit
	dma_option.cmd_trans_en = 0;

	if (1) {
		vo->updare_seq_ctrl[ch].DMA_v1_staddr.regValue = DMA_v1_staddr.regValue;
            vo->updare_seq_ctrl[ch].V1_y_line_sel.regValue = V1_y_line_sel.regValue;
            vo->updare_seq_ctrl[ch].V1_c_line_sel.regValue = V1_c_line_sel.regValue;
		vo->updare_seq_ctrl[ch].f_update = 1;
	}

	rtd_outl(VODMA_VODMA_V1_DCFG_reg, DMA_v1_dcfg.regValue);
	rtd_outl(VODMA_VODMA_V1_DSIZE_reg, DMA_v1_dsize.regValue);
	rtd_outl(VODMA_VODMA_V1VGIP_HACT_reg, vodma_v1vgip_hact_reg.regValue);
	rtd_outl(VODMA_VODMA_V1_BLK1_reg, DMA_v1_blk1.regValue);
	rtd_outl(VODMA_VODMA_V1_BLK2_reg, DMA_v1_blk2.regValue);
	rtd_outl(VODMA_VODMA_V1CHROMA_FMT_reg, V1_C_up_fmt.regValue);
	rtd_outl(VODMA_VODMA_V1_CU_reg, V1_C_up_mode420.regValue);
	rtd_outl(VODMA_VODMA_V1422_TO_444_reg, V1_C_up_mode422.regValue);
	rtd_outl(VODMA_VODMA_V1_LINE_BLK1_reg, V1_y_line_sel.regValue);
	rtd_outl(VODMA_VODMA_V1_LINE_BLK2_reg, V1_c_line_sel.regValue);
	rtd_outl(VODMA_VODMA_DMA_OPTION_reg, dma_option.regValue);

	vodma_reg_db_ctrl.regValue = rtd_inl(VODMA_VODMA_REG_DB_CTRL_reg);
	vodma_reg_db_ctrl.vodma_db_rdy = 1;
	rtd_outl(VODMA_VODMA_REG_DB_CTRL_reg, vodma_reg_db_ctrl.regValue);

	return 0;
}

int vodma_set_timinggen(VO_TIMING *tmode)
{
	vodma_vodma_v1sgen_RBUS         V1_syncGen;
	vodma_vodma_v1int_RBUS          V1_syncGen_mode;
	vodma_vodma_v1vgip_ivs1_RBUS    V1_vgipPos_topIVS;
	vodma_vodma_v1vgip_ivs2_RBUS    V1_vgipPos_botIVS;
	vodma_vodma_v1vgip_ihs_RBUS     V1_vgipPos_IHS;
	vodma_vodma_v1vgip_fd_RBUS      V1_vgipPos_topFld;
	vodma_vodma_v1vgip_fd2_RBUS     V1_vgipPos_botFld;
	vodma_vodma_v1vgip_hact_RBUS    V1_vgipPos_HACT;
	vodma_vodma_v1vgip_vact1_RBUS   V1_vgipPos_topVACT;
	vodma_vodma_v1vgip_vact2_RBUS   V1_vgipPos_botVACT;
	vodma_vodma_v1vgip_hbg_RBUS     V1_vgipPos_HBG;
	vodma_vodma_v1vgip_vbg1_RBUS    V1_vgipPos_topVBG;
	vodma_vodma_v1vgip_vbg2_RBUS    V1_vgipPos_botVBG;
	//vodma_vodma_v1vgip_bg_clr_RBUS  V1_vgipPos_BGCLR;
	vodma_vodma_v1vgip_intpos_RBUS  V1_vgipPos_intrpt;

	unsigned int HTotal, VTotal, HActStart, HActEnd, VActStart, VActEnd;

	HTotal		= tmode->HTotal;
	VTotal		= tmode->VTotal;
	HActStart	= tmode->HStartPos;
	HActEnd		= tmode->HStartPos + tmode->HWidth;
	VActStart	= tmode->VStartPos;
	VActEnd		= tmode->VStartPos + tmode->VHeight;

	V1_syncGen.vthr_rst1 = 0;
	V1_syncGen.v_thr = 0xfff;
	V1_syncGen.h_thr = HTotal;

	V1_syncGen_mode.vgip_en = 1;
	V1_syncGen_mode.interlace = !tmode->isProg;
	V1_syncGen_mode.top_fld_indc = 0;
	V1_syncGen_mode.v_thr = VTotal;

	V1_vgipPos_topIVS.v_end = IVS_START + IVS_WIDTH;
	V1_vgipPos_topIVS.v_st = IVS_START;

	V1_vgipPos_botIVS.v_end = IVS_START + IVS_WIDTH;
	V1_vgipPos_botIVS.v_st = IVS_START;

	V1_vgipPos_IHS.h_st = 0;

	V1_vgipPos_topFld.v_end = 0xfff;
	V1_vgipPos_topFld.v_st = 0xfff;

	V1_vgipPos_botFld.v_end = 0xfff;
	V1_vgipPos_botFld.v_st = IVS_START;

	V1_vgipPos_HACT.h_end = HActEnd;
	V1_vgipPos_HACT.h_st = HActStart;

	V1_vgipPos_topVACT.v_end = VActEnd;
	V1_vgipPos_topVACT.v_st = VActStart;

	V1_vgipPos_botVACT.v_end = V1_vgipPos_topVACT.v_end;
	V1_vgipPos_botVACT.v_st = V1_vgipPos_topVACT.v_st;

	V1_vgipPos_HBG.h_end = HActEnd;
	V1_vgipPos_HBG.h_st = HActStart;

	V1_vgipPos_topVBG.v_end = VActEnd;
	V1_vgipPos_topVBG.v_st = VActStart;

	V1_vgipPos_botVBG.v_end = VActEnd;
	V1_vgipPos_botVBG.v_st = VActStart;

	V1_vgipPos_intrpt.v2 = VActEnd;
	V1_vgipPos_intrpt.v1 = VActEnd;

	rtd_outl(VODMA_VODMA_V1SGEN_reg, V1_syncGen.regValue);
	rtd_outl(VODMA_VODMA_V1INT_reg, V1_syncGen_mode.regValue);
	rtd_outl(VODMA_VODMA_V1VGIP_IVS1_reg, V1_vgipPos_topIVS.regValue);
	rtd_outl(VODMA_VODMA_V1VGIP_IVS2_reg, V1_vgipPos_botIVS.regValue);
	rtd_outl(VODMA_VODMA_V1VGIP_IHS_reg, V1_vgipPos_IHS.regValue);
	rtd_outl(VODMA_VODMA_V1VGIP_FD_reg, V1_vgipPos_topFld.regValue);
	rtd_outl(VODMA_VODMA_V1VGIP_FD2_reg, V1_vgipPos_botFld.regValue);
	rtd_outl(VODMA_VODMA_V1VGIP_HACT_reg, V1_vgipPos_HACT.regValue);
	rtd_outl(VODMA_VODMA_V1VGIP_VACT1_reg, V1_vgipPos_topVACT.regValue);
	rtd_outl(VODMA_VODMA_V1VGIP_VACT2_reg, V1_vgipPos_botVACT.regValue);
	rtd_outl(VODMA_VODMA_V1VGIP_HBG_reg, V1_vgipPos_HBG.regValue);
	rtd_outl(VODMA_VODMA_V1VGIP_VBG1_reg, V1_vgipPos_topVBG.regValue);
	rtd_outl(VODMA_VODMA_V1VGIP_VBG2_reg, V1_vgipPos_botVBG.regValue);
	rtd_outl(VODMA_VODMA_V1VGIP_INTPOS_reg, V1_vgipPos_intrpt.regValue);

	return 0;
}

int vodma_set_vsyncgen(unsigned int ch, VO_TIMING *tmode)
{
	VO_DATA *vo = &vodma_data[0];
	vodma_vodma_reg_db_ctrl_RBUS    vodma_reg_db_ctrl;
	vodma_vodma_clkgen_RBUS         clkgen;
	vodma_vodma_pvs_free_RBUS       pvs_free;
	vodma_vodma_pvs_ctrl_RBUS       pvs_ctrl;
	vodma_vodma_pvs0_gen_RBUS       pvs0;
	vodma_vodma_pvs_width_ctrl_RBUS pvs_width_ctrl;

	unsigned int vodmapll, vodmapll0, freerun_clk;
	unsigned int nMCode, nNCode, nOCode, regValue, busclk = 0;

	vodma_reg_db_ctrl.regValue = rtd_inl(VODMA_VODMA_REG_DB_CTRL_reg);
	vodma_reg_db_ctrl.vodmavsg_db_rdy = 0;
	rtd_outl(VODMA_VODMA_REG_DB_CTRL_reg, vodma_reg_db_ctrl.regValue);

	/* vodma clk gen */
	clkgen.vodma_enclk = 1;
	clkgen.vodma_clk_sel = 0;
	clkgen.vodma_clk_div2_en = !tmode->isProg;
	clkgen.vodma_clk_div_n = 0;
	clkgen.vodma_clk_ratio_n_off = 0;

	switch (clkgen.vodma_clk_sel) {
	case 0:
	default:
		regValue = rtd_inl(PLL_REG_SYS_PLL_VODMA1_reg);
		nMCode = ((regValue & 0x000ff000)>>12);
		nNCode = ((regValue & 0x00300000)>>20);
		nOCode = ((regValue & 0x00000180)>>7);

		busclk = 27000000*(nMCode+2)/(nNCode+1)/(nOCode+1);

		rtd_pr_vo_info("PLL_VODMA m: %d, n: %d, o: %d, clk: %d\n", nMCode, nNCode, nOCode, busclk);

		vodmapll = busclk;

		break;
	}
	vodmapll0 = vodmapll;

#if 0 // [ML4BU-251] HW issue, vodma_clk_div_n need keep in zero to avoid output clock error
	if ((vodmapll/2) > (tmode->SampleRate*10000)) {
		int div_n = (vodmapll/(tmode->SampleRate*10000))-1;
		vodmapll = vodmapll / (div_n + 1);
		clkgen.vodma_clk_div_n = div_n;

		rtd_pr_vo_info("div_n: %d, clk: %d\n", clkgen.vodma_clk_div_n, vodmapll);
	}
#endif

#if 1 // [ML4BU-251] HW issue, vodma_clk_div_n need keep in zero to avoid output clock error
	if ((vodmapll/64*63) > (tmode->SampleRate*10000)) {
		int ratio_n_off = 64 - (tmode->SampleRate*64/(vodmapll/10000)) - 1;
		vodmapll = (vodmapll/64)*(64 - ratio_n_off);
		clkgen.vodma_clk_ratio_n_off = ratio_n_off;

		rtd_pr_vo_info("ratio_n_off: %d, clk: %d\n", clkgen.vodma_clk_ratio_n_off, vodmapll);
	}
#endif

	if (clkgen.vodma_clk_div2_en) {
		vodmapll = vodmapll / 2;

		rtd_pr_vo_info("div2: %d, clk: %d\n", clkgen.vodma_clk_div2_en, vodmapll);
	}

	vo->videoPlane[ch].vsyncgen_pixel_freq = vodmapll;

	clkgen.gating_src_sel = 0;
	clkgen.en_fifo_full_gate = 0;
	clkgen.en_fifo_empty_gate = 0;

	/* pvs freerun */
	pvs_ctrl.pvs0_free_clk_sel = 0;
	pvs_free.pvs0_free_en = 1;		/* 0: disable(clk from vodma), 1:enable(clk from PCK_CLK) */
	pvs_free.pvs0_free_vs_reset_en = 0;
	pvs_free.pvs0_free_period_update = 1;
	switch (pvs_ctrl.pvs0_free_clk_sel) {
	case 1:
		freerun_clk = vodmapll0;
		break;
	case 0:
	default:
		freerun_clk = 27000000;
		break;
	}
	pvs_free.pvs0_free_period = (freerun_clk / ((int)tmode->FrameRate)) * 1000;  /* vfreq unit 0.001Hz */

	pvs_width_ctrl.pvs0_vs_width = pvs_free.pvs0_free_period / tmode->VTotal;

	pvs_ctrl.pvs0_free_l_flag_en = 0;
	pvs_ctrl.pvs0_free_vs_inv_en = 0;
	pvs_ctrl.pvs0_free_vs_sel = 0;
	pvs_ctrl.mask_one_vs = 0;

	/* pvs0 gen */
	pvs0.en_pvgen = 1;
	pvs0.en_pfgen = 1;
	pvs0.pfgen_inv = 0;
	pvs0.iv_inv_en = 0;
	pvs0.iv_src_sel = 0;
	pvs0.iv2pv_dly = 2;

	rtd_outl(VODMA_VODMA_CLKGEN_reg, clkgen.regValue);
	rtd_outl(VODMA_VODMA_PVS_Free_reg, pvs_free.regValue);
	rtd_outl(VODMA_VODMA_PVS_WIDTH_CTRL_reg, pvs_width_ctrl.regValue);
	rtd_outl(VODMA_VODMA_PVS_CTRL_reg, pvs_ctrl.regValue);
	rtd_outl(VODMA_VODMA_PVS0_Gen_reg, pvs0.regValue);

	vodma_reg_db_ctrl.regValue = rtd_inl(VODMA_VODMA_REG_DB_CTRL_reg);
	vodma_reg_db_ctrl.vodmavsg_db_rdy = 1;
	rtd_outl(VODMA_VODMA_REG_DB_CTRL_reg, vodma_reg_db_ctrl.regValue);

	rtd_pr_vo_info("vodma clkgen = %x/%x\n", clkgen.regValue, rtd_inl(VODMA_VODMA_CLKGEN_reg));
	rtd_pr_vo_info("PVS_FREE = %x/%x\n", pvs_free.regValue, rtd_inl(VODMA_VODMA_PVS_Free_reg));
	rtd_pr_vo_info("PVS0_GEN = %x/%x\n", pvs0.regValue, rtd_inl(VODMA_VODMA_PVS0_Gen_reg));

	return 0;
}

int vodma_set_sensio(void)
{
#if 0 // removed in Merlin3
	sensio_sensio_ctrl_3_RBUS sensio_ctrl_3;
	sensio_sensio_ctrl_0_RBUS sensio_ctrl_0;

	sensio_ctrl_3.regValue = rtd_inl(SENSIO_Sensio_ctrl_3_reg);
	sensio_ctrl_3.sensio_double_apply = 0;
	sensio_ctrl_3.sensio_double_en = 1;
	rtd_outl(SENSIO_Sensio_ctrl_3_reg, sensio_ctrl_3.regValue);

	sensio_ctrl_0.regValue = rtd_inl(SENSIO_Sensio_ctrl_0_reg);
	sensio_ctrl_0.re_decom_en = 0;
	sensio_ctrl_0.sensio_en = 0;
	rtd_outl(SENSIO_Sensio_ctrl_0_reg, sensio_ctrl_0.regValue);

	sensio_ctrl_3.sensio_double_apply = 1;
	rtd_outl(SENSIO_Sensio_ctrl_3_reg, sensio_ctrl_3.regValue);
#endif
	return 0;
}

VODMA_MODE Mode_Decision(unsigned int width, unsigned int height, bool isProg)
{
	if (height > 0 && height <= 484) { /* NTSC */
		if (width > 1920)
			goto resolution_4k2k;
		else if (width > 1280)
			goto resolution_1080;
		else if (width > 720)
			goto resolution_720p;

		return (!isProg) ? VODMA_858x525_720x480_60I : VODMA_858x525_720x480_60P;
	} else if (height > 484 && height <= 576) { /* PAL */
		if (width > 1920)
			goto resolution_4k2k;
		else if (width > 1280)
			goto resolution_1080;
		else if (width > 720)
			goto resolution_720p;

		return (!isProg) ? VODMA_864x625_720x576_50I : VODMA_864x625_720x576_50P;
	} else if (height > 576 && height <= 720) { /* 720P60 */
		if (width > 1920)
			goto resolution_4k2k;
		else if (width > 1280)
			goto resolution_1080;

resolution_720p:
		return VODMA_1650x750_1280x720_60P;
	} else if (height > 720 && height <= 1088) { /* 1080I60 */
		if (width > 1920)
			goto resolution_4k2k;
resolution_1080:
		/* decide mode is 1080i50/1080i60/1080p50/1080p60 by mode and frame_rate variable */
		return (!isProg) ? VODMA_2200x1125_1920x1080_60I : VODMA_2200x1125_1920x1080_60P;
	} else if (height > 1088) {
resolution_4k2k:
		return (!isProg) ? VODMA_4120x2192_3840x2160_30I : VODMA_4120x2192_3840x2160_30P;
	} else {
		rtd_pr_vo_err("no suitable timing W(%d) H(%d)\n", width, height);
	}
	return VODMA_DEFAULT_MODE;
}

int vomda_set_voinfo(unsigned int ch, VO_TIMING *timemode, unsigned char source)
{
	VO_DATA *vo = &vodma_data[0];
	SLR_VOINFO VOInfo;
	unsigned char isRGB = 0;

	rtd_pr_vo_info("[vo] vomda_set_voinfo\n");

	memset(&VOInfo, 0, sizeof(SLR_VOINFO));

	isRGB = ((vo->image[ch].pixel_format == HAL_VO_PIXEL_FORMAT_ARGB) ||
		 (vo->image[ch].pixel_format == HAL_VO_PIXEL_FORMAT_ABGR) ||
		 (vo->image[ch].pixel_format == HAL_VO_PIXEL_FORMAT_BGR) ||
		 (vo->image[ch].pixel_format == HAL_VO_PIXEL_FORMAT_RGB));

	VOInfo.plane = VO_VIDEO_PLANE_V1;
	VOInfo.port = ch;
	VOInfo.mode = Mode_Decision(vo->image[ch].rect.w, vo->image[ch].rect.h, timemode->isProg);
	VOInfo.h_total = timemode->HTotal;
	VOInfo.v_total = (timemode->isProg) ? timemode->VTotal : timemode->VTotal/2;
	VOInfo.h_start = timemode->HStartPos;
	VOInfo.v_start = timemode->VStartPos - 2 - 1; /* IVS_START = 2 */
	VOInfo.h_width = timemode->HWidth;
	VOInfo.v_length = timemode->VHeight;
	VOInfo.h_freq = vo->videoPlane[ch].vsyncgen_pixel_freq / timemode->HTotal / 100;
	VOInfo.v_freq = timemode->FrameRate/100;
	VOInfo.pixel_clk = vo->videoPlane[ch].vsyncgen_pixel_freq;
	VOInfo.progressive = timemode->isProg;
	VOInfo.chroma_fmt = vo->videoPlane[ch].Ddomain_Chroma_fmt;
	VOInfo.i_ratio = (vo->image[ch].rect.w && ((vo->image[ch].rect.h*100/vo->image[ch].rect.w) > 65)) ? 1 : 0; /* 0. 16:9, 1. 4:3 //480*100/720=66.67 */
	VOInfo.source = (isRGB) ? 3 : 0; /* 3 for rgb, or just set 0 */
	VOInfo.isJPEG = source;
	VOInfo.vdec_source = VOInfo.isJPEG;
	VOInfo.colorspace = (isRGB);
	VOInfo.mode_3d = 0;
	VOInfo.force2d = 0;
	VOInfo.afd = 8;
	VOInfo.pixelAR_hor = 1;
	VOInfo.pixelAR_ver = 1;
	VOInfo.launcher = 0;
	VOInfo.src_h_wid = vo->image[ch].rect.w;
	VOInfo.src_v_len = vo->image[ch].rect.h;

	rtd_pr_vo_info("\nSet Voinfo:\n");
	rtd_pr_vo_info("plane=%d\n", VOInfo.plane);
	rtd_pr_vo_info("port=%d\n", VOInfo.port);
	rtd_pr_vo_info("mode=%d\n", VOInfo.mode);
	rtd_pr_vo_info("h_start=%d\n", VOInfo.h_start);
	rtd_pr_vo_info("v_start=%d\n", VOInfo.v_start);
	rtd_pr_vo_info("h_width=%d\n", VOInfo.h_width);
	rtd_pr_vo_info("v_length=%d\n", VOInfo.v_length);
	rtd_pr_vo_info("h_total=%d\n", VOInfo.h_total);
	rtd_pr_vo_info("v_total=%d\n", VOInfo.v_total);
	rtd_pr_vo_info("chroma_fmt=%d\n", VOInfo.chroma_fmt);
	rtd_pr_vo_info("progressive=%d\n", VOInfo.progressive);
	rtd_pr_vo_info("h_freq=%d\n", VOInfo.h_freq);
	rtd_pr_vo_info("v_freq=%d\n", VOInfo.v_freq);
	rtd_pr_vo_info("pixel_clk=%d\n", VOInfo.pixel_clk);
	rtd_pr_vo_info("i_ratio=%d\n", VOInfo.i_ratio);
	rtd_pr_vo_info("source=%d\n", VOInfo.source);
	rtd_pr_vo_info("isJPEG=%d\n", VOInfo.isJPEG);
	rtd_pr_vo_info("mode_3d=%d\n", VOInfo.mode_3d);
	rtd_pr_vo_info("force2d=%d\n", VOInfo.force2d);
	rtd_pr_vo_info("afd=%d\n", VOInfo.afd);
	rtd_pr_vo_info("HPAR=%d\n", VOInfo.pixelAR_hor);
	rtd_pr_vo_info("VPAR=%d\n", VOInfo.pixelAR_ver);
	rtd_pr_vo_info("src_h_wid=%d\n", VOInfo.src_h_wid);
	rtd_pr_vo_info("src_v_len=%d\n", VOInfo.src_v_len);
	rtd_pr_vo_info("rotate=%d\n", VOInfo.screenRotation);
	rtd_pr_vo_info("launcher=%d\n", VOInfo.launcher);

	set_zoom_smoothtoggle_vo_hwid(VOInfo.src_h_wid);
	set_zoom_smoothtoggle_vo_vlen(VOInfo.src_v_len);
	if ((get_vo_nosignal_flag(ch) == 0) && (memcmp(&vo->videoPlane[ch].VOInfo, &VOInfo, sizeof(SLR_VOINFO) - (sizeof(SLR_AFD_TYPE) + (sizeof(unsigned int)*4))) == 0)) { /* don't compare voinfo AFD, PixelAR and launcher */
		rtd_pr_vo_info("[vo] enter the same timing\n");
		return S_FAIL;
	}

	memcpy(&vo->videoPlane[ch].VOInfo, &VOInfo, sizeof(SLR_VOINFO));
	memcpy(Scaler_VOInfoPointer(ch), &VOInfo, sizeof(SLR_VOINFO));
	Scaler_DispSetInputInfoByDisp(ch, SLR_INPUT_VO_SOURCE_TYPE, VOInfo.isJPEG);
	set_vo_nosignal_flag(ch, 0);
	set_vo_EOS_flag(ch, 0);

	return S_OK;
}

struct semaphore *get_vo_infosemaphore(void)
{
	return &VO_InfoSemaphore;
}

unsigned char get_voinfo_flag(void)
{
	return voinfo_flag;
}

void set_voinfo_flag(unsigned char flag)
{
 	voinfo_flag = flag ;
}

unsigned char get_vo_change_flag(unsigned char ch)
{
	if (ch >= 2) {
		rtd_pr_vo_err("[vo] %s error: ch %d doesn't exist\n", __func__, ch);
		ch = 0;
	}
	return vo_info_change_flag[ch];
}

void set_vo_change_flag(unsigned char ch, unsigned char flag)
{
	if (ch >= 2) {
		rtd_pr_vo_err("[vo] %s error: ch %d doesn't exist\n", __func__, ch);
		ch = 0;
	}
	vo_info_change_flag[ch] = flag;
}

StructDisplayInfo *Get_VO_Dispinfo(unsigned char ch)
{
	if (ch >= 2) {
		rtd_pr_vo_err("[vo] %s error: ch %d doesn't exist\n", __func__, ch);
		ch = 0;
	}
	return &VO_dispinfo[ch];
}


void Update_Filmmode_VO_Dispinfo(SLR_VOINFO *pVOInfo)
{
	unsigned char ch = pVOInfo->port;
	unsigned char ucMode = 0;
	unsigned char display = VO_get_disp_from_port(ch);

	if (ch >= 2) {
		rtd_pr_vo_err("[vo] %s error: ch %d doesn't exist\n", __func__, ch);
		ch = 0;
	}
    rtd_pr_vo_notice("[vo][%s] port=%d,display=%d,ch=%d\n", __FUNCTION__, pVOInfo->port,display,ch);

	memset(&VO_dispinfo[ch], 0, sizeof(StructDisplayInfo));

	/* mode */
	switch (pVOInfo->mode) {
	case VODMA_DEFAULT_MODE:
	case VODMA_858x525_720x480_60I:
		ucMode = _MODE_480I;
		break;
	case VODMA_858x525_720x480_60P:
		ucMode = _MODE_480P;
		break;
	case VODMA_864x625_720x576_50I:
		ucMode = _MODE_576I;
		break;
	case VODMA_864x625_720x576_50P:
		ucMode = _MODE_576P;
		break;
	case VODMA_1980x750_1280x720_50P:
		ucMode = _MODE_720P50;
		break;
	case VODMA_1650x750_1280x720_60P:
		ucMode = _MODE_720P60;
		break;
	case VODMA_2640x1125_1920x1080_50I:
		ucMode = _MODE_1080I25;
		break;
	case VODMA_2200x1125_1920x1080_60I:
		ucMode = _MODE_1080I30;
		break;
	case VODMA_2640x1125_1920x1080_50P:
		ucMode = _MODE_1080P50;
		break;
	case VODMA_2200x1125_1920x1080_60P:
		ucMode = _MODE_1080P60;
		break;
	case VODMA_4120x2192_3840x2160_30I:
		ucMode = _MODE_4k2kI30;
		break;
	case VODMA_4120x2192_3840x2160_30P:
		ucMode = _MODE_4k2kP30;
		break;
	default:
		rtd_pr_vo_err("unknown display mode!!!\n");
		break;
	}

	VO_dispinfo[ch].ucMode_Curr = ucMode;
	VO_dispinfo[ch].IPH_ACT_STA_PRE = pVOInfo->h_start;
	VO_dispinfo[ch].IPV_ACT_STA_PRE = pVOInfo->v_start;
	VO_dispinfo[ch].IPH_ACT_WID_PRE = pVOInfo->h_width;
	VO_dispinfo[ch].IPV_ACT_LEN_PRE = pVOInfo->v_length;
	VO_dispinfo[ch].IHTotal = pVOInfo->h_total;
	VO_dispinfo[ch].IVTotal = pVOInfo->v_total;
	VO_dispinfo[ch].disp_status |= (((pVOInfo->chroma_fmt == FMT_444) ? 0 : 1) << 1);
	VO_dispinfo[ch].disp_status |= ((pVOInfo->progressive ? 0 : 1) << 8);
	VO_dispinfo[ch].IHFreq = pVOInfo->h_freq;
	VO_dispinfo[ch].IVFreq = pVOInfo->v_freq;
	VO_dispinfo[ch].IAspectRatio = pVOInfo->i_ratio;
	VO_dispinfo[ch].afd = pVOInfo->afd;
	VO_dispinfo[ch].color_space = (pVOInfo->chroma_fmt == FMT_444) ? ((pVOInfo->source == 3 || pVOInfo->colorspace) ? VODMA_COLOR_RGB : VODMA_COLOR_YUV444) : VODMA_COLOR_YUV422;
	VO_dispinfo[ch].color_depth = 0;
	VO_dispinfo[ch].color_imetry = 0;
	VO_dispinfo[ch].INPUT_ADC_CLOCK  = 0;
	VO_dispinfo[ch].Hsync = 0;
	VO_dispinfo[ch].Vsync = 0;
	VO_dispinfo[ch].Polarity = 0;
	VO_dispinfo[ch].IHCount_PRE = 0;
	VO_dispinfo[ch].IVCount_PRE = 0;
	VO_dispinfo[ch].IHSyncPulseCount = 0;
	VO_dispinfo[ch].vodma_plane = pVOInfo->plane;
	rtd_pr_vo_info("\nSet VO_Dispinfo[%d]:\n", ch);
	rtd_pr_vo_info("ucMode_Curr=%d\n", VO_dispinfo[ch].ucMode_Curr);
	rtd_pr_vo_info("IPH_ACT_STA_PRE=%d\n", VO_dispinfo[ch].IPH_ACT_STA_PRE);
	rtd_pr_vo_info("IPV_ACT_STA_PRE=%d\n", VO_dispinfo[ch].IPV_ACT_STA_PRE);
	rtd_pr_vo_info("IPH_ACT_WID_PRE=%d\n", VO_dispinfo[ch].IPH_ACT_WID_PRE);
	rtd_pr_vo_info("IPV_ACT_LEN_PRE=%d\n", VO_dispinfo[ch].IPV_ACT_LEN_PRE);
	rtd_pr_vo_info("IHTotal=%d\n", VO_dispinfo[ch].IHTotal);
	rtd_pr_vo_info("IVTotal=%d\n", VO_dispinfo[ch].IVTotal);
	rtd_pr_vo_info("disp_status.422=%d\n", (VO_dispinfo[ch].disp_status & 0x2)>>1);
	rtd_pr_vo_info("disp_status.Interlace=%d\n", (VO_dispinfo[ch].disp_status & 0x100)>>8);
	rtd_pr_vo_info("IHFreq=%d\n", VO_dispinfo[ch].IHFreq);
	rtd_pr_vo_notice("IVFreq=%d\n", VO_dispinfo[ch].IVFreq);
	rtd_pr_vo_info("IAspectRatio=%d\n", VO_dispinfo[ch].IAspectRatio);
	rtd_pr_vo_info("afd=%d\n", VO_dispinfo[ch].afd);
	rtd_pr_vo_info("color_space=%d\n", VO_dispinfo[ch].color_space);
	rtd_pr_vo_info("color_depth=%d\n", VO_dispinfo[ch].color_depth);
	rtd_pr_vo_info("color_imetry=%d\n", VO_dispinfo[ch].color_imetry);
	rtd_pr_vo_info("INPUT_ADC_CLOCK=%d\n", VO_dispinfo[ch].INPUT_ADC_CLOCK);
	rtd_pr_vo_info("Hsync=%d\n", VO_dispinfo[ch].Hsync);
	rtd_pr_vo_info("Vsync=%d\n", VO_dispinfo[ch].Vsync);
	rtd_pr_vo_info("Polarity=%d\n", VO_dispinfo[ch].Polarity);
	rtd_pr_vo_info("IHCount_PRE=%d\n", VO_dispinfo[ch].IHCount_PRE);
	rtd_pr_vo_info("IVCount_PRE=%d\n", VO_dispinfo[ch].IVCount_PRE);
	rtd_pr_vo_info("IHSyncPulseCount=%d\n", VO_dispinfo[ch].IHSyncPulseCount);

	/* up(&VO_InfoSemaphore); */
}
void Config_VO_Dispinfo(SLR_VOINFO *pVOInfo)
{
	unsigned char ch = pVOInfo->port;
	unsigned char ucMode = 0;
	unsigned char display = VO_get_disp_from_port(ch);

	if (ch >= 2) {
		rtd_pr_vo_err("[vo] %s error: ch %d doesn't exist\n", __func__, ch);
		ch = 0;
	}
      rtd_pr_vo_info("[vo] Config_voDispinfo enable=%d ,port=%d,display=%d,ch=%d\n", get_vo_run_smooth_toggle_enable(display),pVOInfo->port,display,ch);
	/* down(&VO_InfoSemaphore); */
	if (get_vo_run_smooth_toggle_enable(display)) {
		;
	} else {
#ifdef NEW_INPUT_OUTPUT_BUFFER_FLOW_CONFIG //avoid not call original output and cause timing issue
		if(ch == 0)
			Scaler_DispSetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE, _MODE_STATE_SEARCH);//For Video FW PQ
		else
			Scaler_DispSetInputInfoByDisp(SLR_SUB_DISPLAY, SLR_INPUT_STATE, _MODE_STATE_SEARCH);//For Video FW PQ
#endif
		vo_info_change_flag[ch] = TRUE;
		rtd_pr_vo_info("\r\n####Config_VO_Dispinfo set vo_info_change_flag TRUE####\r\n");
	}

	memset(&VO_dispinfo[ch], 0, sizeof(StructDisplayInfo));

	/* mode */
	switch (pVOInfo->mode) {
	case VODMA_DEFAULT_MODE:
	case VODMA_858x525_720x480_60I:
		ucMode = _MODE_480I;
		break;
	case VODMA_858x525_720x480_60P:
		ucMode = _MODE_480P;
		break;
	case VODMA_864x625_720x576_50I:
		ucMode = _MODE_576I;
		break;
	case VODMA_864x625_720x576_50P:
		ucMode = _MODE_576P;
		break;
	case VODMA_1980x750_1280x720_50P:
		ucMode = _MODE_720P50;
		break;
	case VODMA_1650x750_1280x720_60P:
		ucMode = _MODE_720P60;
		break;
	case VODMA_2640x1125_1920x1080_50I:
		ucMode = _MODE_1080I25;
		break;
	case VODMA_2200x1125_1920x1080_60I:
		ucMode = _MODE_1080I30;
		break;
	case VODMA_2640x1125_1920x1080_50P:
		ucMode = _MODE_1080P50;
		break;
	case VODMA_2200x1125_1920x1080_60P:
		ucMode = _MODE_1080P60;
		break;
	case VODMA_4120x2192_3840x2160_30I:
		ucMode = _MODE_4k2kI30;
		break;
	case VODMA_4120x2192_3840x2160_30P:
		ucMode = _MODE_4k2kP30;
		break;
	default:
		rtd_pr_vo_err("unknown display mode!!!\n");
		break;
	}

	VO_dispinfo[ch].ucMode_Curr = ucMode;
	VO_dispinfo[ch].IPH_ACT_STA_PRE = pVOInfo->h_start;
	VO_dispinfo[ch].IPV_ACT_STA_PRE = pVOInfo->v_start;
	VO_dispinfo[ch].IPH_ACT_WID_PRE = pVOInfo->h_width;
	VO_dispinfo[ch].IPV_ACT_LEN_PRE = pVOInfo->v_length;
	VO_dispinfo[ch].IHTotal = pVOInfo->h_total;
	VO_dispinfo[ch].IVTotal = pVOInfo->v_total;
	VO_dispinfo[ch].disp_status |= (((pVOInfo->chroma_fmt == FMT_444) ? 0 : 1) << 1);
	VO_dispinfo[ch].disp_status |= ((pVOInfo->progressive ? 0 : 1) << 8);
	VO_dispinfo[ch].IHFreq = pVOInfo->h_freq;
	VO_dispinfo[ch].IVFreq = pVOInfo->v_freq;
	VO_dispinfo[ch].SrcIVFreq = pVOInfo->src_v_freq;
	VO_dispinfo[ch].IAspectRatio = pVOInfo->i_ratio;
	VO_dispinfo[ch].afd = pVOInfo->afd;
	VO_dispinfo[ch].color_space = (pVOInfo->chroma_fmt == FMT_444) ? ((pVOInfo->source == 3 || pVOInfo->colorspace) ? VODMA_COLOR_RGB : VODMA_COLOR_YUV444) : VODMA_COLOR_YUV422;
	VO_dispinfo[ch].color_depth = 0;
	VO_dispinfo[ch].color_imetry = 0;
	VO_dispinfo[ch].INPUT_ADC_CLOCK  = 0;
	VO_dispinfo[ch].Hsync = 0;
	VO_dispinfo[ch].Vsync = 0;
	VO_dispinfo[ch].Polarity = 0;
	VO_dispinfo[ch].IHCount_PRE = 0;
	VO_dispinfo[ch].IVCount_PRE = 0;
	VO_dispinfo[ch].IHSyncPulseCount = 0;
	VO_dispinfo[ch].vodma_plane = pVOInfo->plane;
	rtd_pr_vo_info("\nSet VO_Dispinfo[%d]:\n", ch);
	rtd_pr_vo_info("ucMode_Curr=%d\n", VO_dispinfo[ch].ucMode_Curr);
	rtd_pr_vo_info("IPH_ACT_STA_PRE=%d\n", VO_dispinfo[ch].IPH_ACT_STA_PRE);
	rtd_pr_vo_info("IPV_ACT_STA_PRE=%d\n", VO_dispinfo[ch].IPV_ACT_STA_PRE);
	rtd_pr_vo_info("IPH_ACT_WID_PRE=%d\n", VO_dispinfo[ch].IPH_ACT_WID_PRE);
	rtd_pr_vo_info("IPV_ACT_LEN_PRE=%d\n", VO_dispinfo[ch].IPV_ACT_LEN_PRE);
	rtd_pr_vo_info("IHTotal=%d\n", VO_dispinfo[ch].IHTotal);
	rtd_pr_vo_info("IVTotal=%d\n", VO_dispinfo[ch].IVTotal);
	rtd_pr_vo_info("disp_status.422=%d\n", (VO_dispinfo[ch].disp_status & 0x2)>>1);
	rtd_pr_vo_info("disp_status.Interlace=%d\n", (VO_dispinfo[ch].disp_status & 0x100)>>8);
	rtd_pr_vo_info("IHFreq=%d\n", VO_dispinfo[ch].IHFreq);
	rtd_pr_vo_info("IVFreq=%d\n", VO_dispinfo[ch].IVFreq);
	rtd_pr_vo_info("SrcIVFreq=%d\n", VO_dispinfo[ch].SrcIVFreq);
	rtd_pr_vo_info("IAspectRatio=%d\n", VO_dispinfo[ch].IAspectRatio);
	rtd_pr_vo_info("afd=%d\n", VO_dispinfo[ch].afd);
	rtd_pr_vo_info("color_space=%d\n", VO_dispinfo[ch].color_space);
	rtd_pr_vo_info("color_depth=%d\n", VO_dispinfo[ch].color_depth);
	rtd_pr_vo_info("color_imetry=%d\n", VO_dispinfo[ch].color_imetry);
	rtd_pr_vo_info("INPUT_ADC_CLOCK=%d\n", VO_dispinfo[ch].INPUT_ADC_CLOCK);
	rtd_pr_vo_info("Hsync=%d\n", VO_dispinfo[ch].Hsync);
	rtd_pr_vo_info("Vsync=%d\n", VO_dispinfo[ch].Vsync);
	rtd_pr_vo_info("Polarity=%d\n", VO_dispinfo[ch].Polarity);
	rtd_pr_vo_info("IHCount_PRE=%d\n", VO_dispinfo[ch].IHCount_PRE);
	rtd_pr_vo_info("IVCount_PRE=%d\n", VO_dispinfo[ch].IVCount_PRE);
	rtd_pr_vo_info("IHSyncPulseCount=%d\n", VO_dispinfo[ch].IHSyncPulseCount);

	/* up(&VO_InfoSemaphore); */
}

typedef struct{
	UINT32 dma_vgip_lcnt;
	UINT32 vo_lcnt;
	UINT32 vgip_lcnt;
	UINT32 master_dtg_lcnt;
	UINT32 memc_dtg_lcnt;
}GAME_MODE_LINE_CNT_T;

/*extern UINT32 PQModeInfo_flag[5];
extern void dynamic_change_hdr_setting(unsigned char hdr_enable, I3DDMA_COLOR_SPACE_T color_format);*/
extern void set_vdec_securestatus(unsigned char value);
extern void qos_mode_switch(unsigned int);
void vbe_disp_orbit_set_vo_overscan(unsigned char bEnable);
extern StructSrcRect vbe_orbit_input_size;
extern unsigned char vbe_disp_oled_orbit_enable;
extern unsigned char vbe_disp_oled_orbit_mode;
extern int ori_input_width;
extern int ori_input_height;
extern void set_vdec_securestatus_aipq(unsigned char status);// 0520 lsy
extern void vsc_set_film_mode_skip_check_framerate_change(unsigned char bEnable);
unsigned long rpcVoReady(unsigned long para1, unsigned long para2)
{
	VO_DATA *vo = &vodma_data[0];
	SLR_VOINFO VOInfo;
	SLR_VOINFO *pVOInfo = NULL;
	unsigned char ch = 0;

	unsigned int i = 0, ulVOInfoItemCount = 0;
	unsigned int *pulTemp;

	unsigned char vo_sm_info;
	unsigned char display;

	rtd_pr_vo_info("[vo] rpcVoReady\n");

	down(&VO_InfoSemaphore); /* Lock VO Semaphore */

	pVOInfo = &VOInfo;

	memcpy(pVOInfo, (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GETVOINFO), sizeof(SLR_VOINFO));

	/* change endian */
	ulVOInfoItemCount = sizeof(SLR_VOINFO) /  sizeof(UINT32);
	pulTemp = (unsigned int *)pVOInfo;
	for (i = 0; i < ulVOInfoItemCount; i++)
		pulTemp[i] = Scaler_ChangeUINT32Endian(pulTemp[i]);

	rtd_pr_vo_info("\nreceive Voinfo:\n");
	rtd_pr_vo_info("plane=%d\n", (int)pVOInfo->plane);
	rtd_pr_vo_info("port=%d\n", (int)pVOInfo->port);
	rtd_pr_vo_info("mode=%d\n", (int)pVOInfo->mode);
	rtd_pr_vo_info("h_start=%d\n", (int)pVOInfo->h_start);
	rtd_pr_vo_info("v_start=%d\n", (int)pVOInfo->v_start);
	rtd_pr_vo_info("h_width=%d\n", (int)pVOInfo->h_width);
	rtd_pr_vo_info("v_length=%d\n", (int)pVOInfo->v_length);
	rtd_pr_vo_info("h_total=%d\n", (int)pVOInfo->h_total);
	rtd_pr_vo_info("v_total=%d\n", (int)pVOInfo->v_total);
	rtd_pr_vo_info("chroma_fmt=%d\n", (int)pVOInfo->chroma_fmt);
	rtd_pr_vo_info("progressive=%d\n", (int)pVOInfo->progressive);
	rtd_pr_vo_info("h_freq=%d\n", (int)pVOInfo->h_freq);
	rtd_pr_vo_info("v_freq=%d\n", (int)pVOInfo->v_freq);
	rtd_pr_vo_info("pixel_clk=%d\n", (int)pVOInfo->pixel_clk);
	rtd_pr_vo_info("i_ratio=%d\n", (int)pVOInfo->i_ratio);
	rtd_pr_vo_info("source=%d\n", (int)pVOInfo->source);
	rtd_pr_vo_info("isJPEG=%d\n", (int)pVOInfo->isJPEG);
	rtd_pr_vo_info("vdec=%d\n", (int)pVOInfo->vdec_source);
	rtd_pr_vo_info("color=%d\n", (int)pVOInfo->colorspace);
	rtd_pr_vo_info("mode_3d=%d\n", (int)pVOInfo->mode_3d);
	rtd_pr_vo_info("force2d=%d\n", (int)pVOInfo->force2d);
	rtd_pr_vo_info("afd=%d\n", (int)pVOInfo->afd);
	rtd_pr_vo_info("HPAR=%d\n", (int)pVOInfo->pixelAR_hor);
	rtd_pr_vo_info("VPAR=%d\n", (int)pVOInfo->pixelAR_ver);
	rtd_pr_vo_info("src_h_wid=%d\n", (int)pVOInfo->src_h_wid);
	rtd_pr_vo_info("src_v_len=%d\n", (int)pVOInfo->src_v_len);
	rtd_pr_vo_info("xvYCC=%d\n", (int)pVOInfo->xvYCC);
	rtd_pr_vo_info("trans_char=%d\n", (int)pVOInfo->transfer_characteristics);
	rtd_pr_vo_info("mt_coeff=%d\n", (int)pVOInfo->matrix_coefficiets);
	rtd_pr_vo_info("full_range=%d\n", (int)pVOInfo->video_full_range_flag);
	rtd_pr_vo_info("c_prim=%d\n", (int)pVOInfo->colour_primaries);
	rtd_pr_vo_info("MaxCLL=%d\n", (int)pVOInfo->MaxCLL);
	rtd_pr_vo_info("MaxFALL=%d\n", (int)pVOInfo->MaxFALL);
	rtd_pr_vo_info("security=%d\n", (int)pVOInfo->security);
	rtd_pr_vo_info("isVP9=%d\n", (int)pVOInfo->isVP9);
	rtd_pr_vo_info("rotate=%d\n", (int)pVOInfo->screenRotation);
	rtd_pr_vo_info("isVeComp:%d\n",(int)pVOInfo->isVeComp);

	rtd_pr_vo_info("launcher=%d\n", (int)pVOInfo->launcher);
	rtd_pr_vo_notice("film_mode=%d\n", (int)pVOInfo->film_mode);
	rtd_pr_vo_info("is2KCP=%d\n", (int)pVOInfo->is2KCP);

	ch = pVOInfo->port;

	if ((ch == 0) || (ch == 1)) {
		if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC) && (pVOInfo->vdec_source == 0)){
			is_DTV_flag_set(_ENABLE);
			rtd_pr_vo_notice("this is DTV source \n");
		}
	}
	if(ch == 0 && (vbe_disp_oled_orbit_mode ==_VBE_PANEL_ORBIT_AUTO_MODE|| vbe_disp_oled_orbit_mode ==_VBE_PANEL_ORBIT_STORE_MODE) && vbe_disp_oled_orbit_enable)
	{
		if(Scaler_DispGetInputInfoByDisp(ch,SLR_INPUT_IPH_ACT_WID) !=pVOInfo->h_width)
		{
			//vbe_disp_orbit_set_vo_overscan(0);

			rtd_pr_vo_notice("ori_input_width=%d, ori_input_height=%d ,src_wid=%d, pVOInfo->h_width=%d, SLR_INPUT_IPH_ACT_WID=%d \n",ori_input_width,ori_input_height,vbe_orbit_input_size.src_wid,pVOInfo->h_width,Scaler_DispGetInputInfoByDisp(ch,SLR_INPUT_IPH_ACT_WID));
			if(vbe_orbit_input_size.src_wid<=Scaler_DispGetInputInfoByDisp(ch,SLR_INPUT_IPH_ACT_WID))
			{//avoid udpate twice then calculate wrong

				vbe_orbit_input_size.srcx = vbe_orbit_input_size.srcx *pVOInfo->h_width/Scaler_DispGetInputInfoByDisp(ch,SLR_INPUT_IPH_ACT_WID);
				vbe_orbit_input_size.srcy = vbe_orbit_input_size.srcy *pVOInfo->v_length/Scaler_DispGetInputInfoByDisp(ch,SLR_INPUT_IPV_ACT_LEN);
				vbe_orbit_input_size.src_wid = vbe_orbit_input_size.src_wid*pVOInfo->h_width/Scaler_DispGetInputInfoByDisp(ch,SLR_INPUT_IPH_ACT_WID);
				vbe_orbit_input_size.src_height = vbe_orbit_input_size.src_height*pVOInfo->v_length/Scaler_DispGetInputInfoByDisp(ch,SLR_INPUT_IPV_ACT_LEN);

				ori_input_width = vbe_orbit_input_size.src_wid;
				ori_input_height = vbe_orbit_input_size.src_height;
			}

			rtd_pr_vo_notice("ori_input_width=%d, ori_input_height=%d ,src_wid=%d, pVOInfo->h_width=%d, SLR_INPUT_IPH_ACT_WID=%d \n",ori_input_width,ori_input_height,vbe_orbit_input_size.src_wid,pVOInfo->h_width,Scaler_DispGetInputInfoByDisp(ch,SLR_INPUT_IPH_ACT_WID));
		}
	}
	if (pVOInfo->film_mode)
	{
		if (pVOInfo->film_mode == _VO_CAD_BAD_EDIT)
			qos_mode_switch(1);
		else
			qos_mode_switch(0);
	}

	//Eric@20180813 if force_change_vo_flag == true, need go scaler flow
	if ((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC) &&
		(pVOInfo->film_mode) &&
		!((pVOInfo->h_width > DATAFS_VDEC_RUN_DATAFS_WIDTH) &&
		(pVOInfo->h_width > DATAFS_VDEC_RUN_DATAFS_WIDTH)) &&
		!force_change_vo_flag[ch])
	{
		// [K5LG-1111] set M-domain input fast mode when switch to video mode
        if(((&vo->videoPlane[ch].VOInfo)->film_mode < _VO_CAD_BAD_EDIT) && (pVOInfo->film_mode >= _VO_CAD_BAD_EDIT)){
            //set_frc_style_input_fast_than_display(TRUE); // apply setting in imd_smooth_toggle_proc()
            // [K5LG-1111] wait input fast setting apply
            WaitFor_DEN_STOP_Done();
            rtd_pr_vo_notice("[FILM] INPUT FAST=%d@VideoMode\n", MDOMAIN_DISP_DDR_MainCtrl_get_main_frc_style(IoReg_Read32(MDOMAIN_DISP_DDR_MainCtrl_reg)));
        }
#ifndef CONFIG_DATA_FRAMESYNC_FORCE_422_CAP
		if (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_FRAMESYNC) == FALSE)
#endif
			vsc_set_film_mode_param(1, pVOInfo->film_mode, pVOInfo->v_freq);
		rtd_pr_vo_notice("[FILM] FILM_MODE type =%d, freq = %d, scaler state = %d\n", pVOInfo->film_mode, pVOInfo->v_freq, Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE));
		memcpy(&vo->videoPlane[ch].VOInfo, pVOInfo, sizeof(SLR_VOINFO));
		memcpy(Scaler_VOInfoPointer(ch), pVOInfo, sizeof(SLR_VOINFO));	/* scaler driver would use it */
		//KTASKWBS-9862, DTV4k50->DTV4k25(Film) change to youtube 4K25
		//vo check it with same timing, it will make scaler strucutre error.(use 50hz)
		//solution is update vo_dispinfo for scaler structure update.
		Update_Filmmode_VO_Dispinfo(&vo->videoPlane[ch].VOInfo);
		up(&VO_InfoSemaphore);
		vsc_set_film_mode_skip_check_framerate_change(FALSE);
		return 0;
	}
	else{
		rtd_pr_vo_notice("[FILM] external source or reset film type, do not go film mode\n");
		vsc_set_film_mode_param(0, 0, pVOInfo->v_freq);

		if ((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC) && (pVOInfo->film_mode))
		{
			vsc_set_film_mode_skip_check_framerate_change(TRUE);
		}
		else
		{
			vsc_set_film_mode_skip_check_framerate_change(FALSE);
		}
	}


	if (!(((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC) && (SLR_MAIN_DISPLAY == ch)) ||
		((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_JPEG) && (SLR_MAIN_DISPLAY == ch)) ||
		((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_JPEG) && (SLR_SUB_DISPLAY == ch)) ||
		((Get_DisplayMode_Src(SLR_SUB_DISPLAY) == VSC_INPUTSRC_VDEC) && (SLR_SUB_DISPLAY == ch)) ||
#ifdef CONFIG_FORCE_RUN_I3DDMA
		(
		#if defined(CONFIG_ENABLE_HDR10_HDMI_AUTO_DETECT)|| defined(CONFIG_ENABLE_DOLBY_VISION_HDMI_AUTO_DETECT)
		Scaler_Hdmi_DM_Get_Status() == SLR_HDMI_DM_CONNECT ||
		#endif
		get_force_i3ddma_enable(SLR_MAIN_DISPLAY) == true))
#else
	     (
	     #if defined(CONFIG_ENABLE_HDR10_HDMI_AUTO_DETECT)|| defined(CONFIG_ENABLE_DOLBY_VISION_HDMI_AUTO_DETECT)
	     Scaler_Hdmi_DM_Get_Status() == SLR_HDMI_DM_CONNECT &&
	     #endif
		 Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI))
#endif
		)
	{
		rtd_pr_vo_err("[vo] error: not correct ch %d, m:%d,s:%d\n", ch, Get_DisplayMode_Src(SLR_MAIN_DISPLAY), Get_DisplayMode_Src(SLR_SUB_DISPLAY));
		up(&VO_InfoSemaphore); /* Release VO Semaphore */
		return HAL_VO_STATE_OK;
	}

	if (ch >= 2) {
		rtd_pr_vo_err("[vo] %s error: ch %d doesn't exist\n", __func__, ch);
		ch = 0;
	}

	Scaler_DispSetInputInfoByDisp(ch, SLR_INPUT_VO_SOURCE_TYPE, ((pVOInfo->source == 3) ? SOURCE_DIRECTVO : pVOInfo->isJPEG));

	set_vdec_securestatus(pVOInfo->security);
	set_vdec_securestatus_aipq(pVOInfo->security);// 0520 lsy
	/* //set_vo_source_type(pVOInfo->source); */
	set_zoom_smoothtoggle_vo_hwid(pVOInfo->src_h_wid);
	set_zoom_smoothtoggle_vo_vlen(pVOInfo->src_v_len);
	set_vo_3d_mode(pVOInfo->mode_3d);
#if 0//def CONFIG_ENABLE_SE_ROTATE_M_DOMAIN_BLOCK_MODE
	//if ((pVOInfo->screenRotation == DIRECT_VO_FRAME_ORIENTATION_ROTATE_0) && (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC))
	//	pVOInfo->screenRotation =DIRECT_VO_FRAME_ORIENTATION_ROTATE_180;
	if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC) && ((DIRECT_VO_FRAME_ORIENTATION_ROTATE_90 == pVOInfo->screenRotation) ||(DIRECT_VO_FRAME_ORIENTATION_ROTATE_180 == pVOInfo->screenRotation) ||(DIRECT_VO_FRAME_ORIENTATION_ROTATE_270 == pVOInfo->screenRotation)) )
	{
		//Set_rotate_function(SLR_MAIN_DISPLAY, TRUE);
		set_rotate_mode(SLR_MAIN_DISPLAY, pVOInfo->screenRotation);
		rtd_pr_vo_emerg("get_rotate_mode =%d\n",get_rotate_mode(SLR_MAIN_DISPLAY));
	}
	else
	{
		set_rotate_mode(SLR_MAIN_DISPLAY, 0);
		//Set_rotate_function(SLR_MAIN_DISPLAY, FALSE);
	}
#endif

	/* //get VP9 flag */
	set_vo_vp9_flag(ch, pVOInfo->isVP9);

	// set VE comp status
	set_vo_veComp_flag(ch,pVOInfo->isVeComp);

	/* para1 bit4:display; bit3:smooth toggle fail flag;bit2-1:smooth toggle state decide open smooth toggle or not;bit0:curr timing go smooth toggle or not */
	/* para1  bit5:imd_framesync */
	display = (para1>>4)&0x01; rtd_pr_vo_notice("rpcVoReady:display:%d\n", display);
	VO_set_disp_to_port(ch, display);

	vo_sm_info = (para1>>1)&0x03; rtd_pr_vo_info("set_vo_open_smooth_toggle_enable:%d\n", vo_sm_info);
	set_vo_open_smooth_toggle_enable(vo_sm_info, display);
	vo_sm_info = para1&0x01; rtd_pr_vo_info("set_vo_run_smooth_toggle_enable:%d\n", vo_sm_info);
	set_vo_run_smooth_toggle_enable(vo_sm_info, display);
	vo_sm_info = (para1>>7)&0x01; rtd_pr_vo_info("frameratechange:%d\n", vo_sm_info);
   	set_vo_frameratechange_enable(vo_sm_info,display);
	vo_sm_info = (para1>>3)&0x01; rtd_pr_vo_info("vo_sm_info:%d\n", vo_sm_info);
   #ifdef SMOOTH_TOGGLE_DATAFRCFS_TEST
	if (vbe_disp_get_adaptivestream_fs_mode()) {
		vo_sm_info = (para1>>5)&0x1; rtd_pr_vo_notice("rpcVoReady:fs:%d\n", vo_sm_info);
		set_vo_smooth_toggle_fsmode(vo_sm_info, display);
	}
   #endif

	if (!get_vo_run_smooth_toggle_enable(display)){
		if ((Get_DisplayMode_Src(display) == VSC_INPUTSRC_VDEC)||(Get_DisplayMode_Src(display) == VSC_INPUTSRC_JPEG)){
			set_voinfo_flag(1);
			rtd_pr_vo_info("set_voinfo_flag true\n");
		}else{
			set_voinfo_flag(0);
		}
	}else{
			set_voinfo_flag(0);
	}

	if (vo_sm_info) {
		/* smooth toggle fail,need to restart */
	} else {
#ifdef CONFIG_HDR_SDR_SEAMLESS
		/* don't compare voinfo HDR item and launcher for seamless case @Crixus 20170215*/
		#ifdef CONFIG_ARM64
 		if ((memcmp(&vo->videoPlane[ch].VOInfo, pVOInfo, ((unsigned long)&(pVOInfo->source)-(unsigned long)&(pVOInfo->plane))) == 0))/* only compare between plane and pixel_clk */
		#else
		if ((memcmp(&vo->videoPlane[ch].VOInfo, pVOInfo, ((unsigned int)&(pVOInfo->source)-(unsigned int)&(pVOInfo->plane))) == 0))/* only compare between plane and pixel_clk */
		#endif
#else
		if ((memcmp(&vo->videoPlane[ch].VOInfo, pVOInfo, sizeof(SLR_VOINFO) - (sizeof(SLR_AFD_TYPE) + (sizeof(unsigned int)*4))) == 0))/* don't compare voinfo AFD, PixelAR and launcher */
#endif
		{
			rtd_pr_vo_info("\nenter the same timing\n");
			set_vo_nosignal_flag(ch, 0);
			set_vo_EOS_flag(ch, 0);
			if (!get_vo_run_smooth_toggle_enable(display) && (((para1>>1)&0x03) == SLR_SMOOTH_TOGGLE_INIT) && (get_vo_smoothtoggle_state(display) == SLR_SMOOTH_TOGGLE_ACTIVE)) {
				set_reset_smooth_toggle_state(_ENABLE, display);
				rtd_pr_vo_info("reset smooth toggle state\n");
			}
			if (force_change_vo_flag[ch]) {
				if(!rtk_hal_vsc_Getdualdecoder_run()){  /*dual decoder case will change vo port*/
				    vo_info_change_flag[ch] = TRUE;
				    rtd_pr_vo_emerg("\r\n####force_change_vo_flag. set vo_info_change_flag TRUE####\r\n");
		        }
				force_change_vo_flag[ch] = FALSE;
			}
			up(&VO_InfoSemaphore); /* Release VO Semaphore */

#ifdef CONFIG_HDR_SDR_SEAMLESS
			memcpy(&vo->videoPlane[ch].VOInfo, pVOInfo, sizeof(SLR_VOINFO));
			memcpy(Scaler_VOInfoPointer(ch), pVOInfo, sizeof(SLR_VOINFO));	/* scaler driver would use it */
#endif
			return 0;
		}
	}
	memcpy(&vo->videoPlane[ch].VOInfo, pVOInfo, sizeof(SLR_VOINFO));
	memcpy(Scaler_VOInfoPointer(ch), pVOInfo, sizeof(SLR_VOINFO));	/* scaler driver would use it */
	set_vo_nosignal_flag(ch, 0);
	set_vo_EOS_flag(ch, 0);
	Config_VO_Dispinfo(&vo->videoPlane[ch].VOInfo);

	/* //update voinfo immediately for adaptive stream */
	if(display == SLR_MAIN_DISPLAY)
	{
		if (vsc_get_adaptivestream_flag(SLR_MAIN_DISPLAY)) {
			Scaler_SET_VSCDispinfo_WithVFEDispinfo(display, Get_VO_Dispinfo(ch));
		}
	}

	if (force_change_vo_flag[ch])
		force_change_vo_flag[ch] = FALSE;

	//Eric@20181009 protect framerate change flow can run completely, set dynamic_framerate_change_flag = 1
	if ((Get_DisplayMode_Src(display) == VSC_INPUTSRC_VDEC)
		&& (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE)
		&& (get_vo_run_smooth_toggle_enable(display))
#ifdef CONFIG_DATA_FRAMESYNC_FORCE_422_CAP
		&& !((pVOInfo->h_width > DATAFS_DISABLE_GATTING_WIDTH)
		&& (pVOInfo->h_width > DATAFS_DISABLE_GATTING_HEIGHT))
#endif

		&& (get_vo_frameratechange_enable(display)==1)
		&& (!vsc_get_adaptivestream_flag(display))
		&& (scaler_get_current_framesync_mode(display) ==0)
		&& (!is_DTV_flag_get() || ((pVOInfo->progressive) && Scaler_DispGetStatus(display,SLR_DISP_INTERLACE)))
		&& (pVOInfo->v_freq != Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ))
		&& (pVOInfo->h_width == Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_IPH_ACT_WID_PRE))
		&& (pVOInfo->v_length == Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_IPV_ACT_LEN_PRE)))
    {
	    Scaler_set_dynamic_framerate_change_flag(_ENABLE);
    }

	up(&VO_InfoSemaphore); /* Release VO Semaphore */
#if 0	/* move into "Scaler_hdr_setting_SEAMLESS()"*/
	//IoReg_Write32(0xb802e4f4, IoReg_Read32(0xb802e4f4) | ((pVOInfo->transfer_characteristics) << 8));
	static UINT8 pre_transfer_characteristics = 0;
	if (pre_transfer_characteristics != pVOInfo->transfer_characteristics)
	{
		//IoReg_Write32(0xb802e4f4, IoReg_Read32(0xb802e4f4) + 1);
		if (pVOInfo->transfer_characteristics == 16)
		{
			//IoReg_Write32(0xb802e4f4, IoReg_Read32(0xb802e4f4) | 0x00010000);
			PQModeInfo_flag[0] = HAL_VPQ_HDR_MODE_HDR10;
			PQModeInfo_flag[1] = HAL_VPQ_COLORIMETRY_BT2020;
			//dynamic_change_hdr_setting(ON, I3DDMA_COLOR_YUV422);
		}
		else if (pVOInfo->transfer_characteristics == 18)
		{
			//IoReg_Write32(0xb802e4f4, IoReg_Read32(0xb802e4f4) | 0x00020000);
			PQModeInfo_flag[0] = HAL_VPQ_HDR_MODE_HLG;
			PQModeInfo_flag[1] = HAL_VPQ_COLORIMETRY_BT2020;
			//dynamic_change_hdr_setting(ON, I3DDMA_COLOR_YUV422);
		}
		else
		{
			//IoReg_Write32(0xb802e4f4, IoReg_Read32(0xb802e4f4) | 0x00040000);
			//dynamic_change_hdr_setting(OFF, I3DDMA_COLOR_YUV422);
			PQModeInfo_flag[0] = HAL_VPQ_HDR_MODE_SDR;
			PQModeInfo_flag[1] = HAL_VPQ_COLORIMETRY_BT709;
		}
		//rtd_pr_vo_emerg("[vo] transfer_characteristics changed. set change_vgip_pq\n");
	}

	pre_transfer_characteristics = pVOInfo->transfer_characteristics;
#endif
	return 0;
}


unsigned long rpcVoNoSignal(unsigned long para1, unsigned long para2)
{
	rtd_pr_vo_info("[vo] rpcVoNoSignal\n");

	set_vo_nosignal_flag((unsigned int)para1, 1);

	return 0;
}

unsigned char get_vo_nosignal_flag(unsigned int ch)
{
	VO_DATA *vo = &vodma_data[0];

	if (ch >= 2) {
		rtd_pr_vo_err("[vo] %s error: ch %d doesn't exist\n", __func__, ch);
		ch = 0;
	}

	return vo->nosignal[ch];
}

void set_vo_nosignal_flag(unsigned int ch, unsigned char flag)
{
	VO_DATA *vo = &vodma_data[0];

	if (ch >= 2) {
		rtd_pr_vo_err("[vo] %s error: ch %d doesn't exist\n", __func__, ch);
		return;
	}

	vo->nosignal[ch] = flag;

	rtd_pr_vo_info("[vo] set vo(%d) nosignal: %d\n", ch, vo->nosignal[ch]);
}

unsigned long rpcVoAVSync(unsigned long para1, unsigned long para2)
{
	rtd_pr_vo_info("[vo] rpcVoAVSync\n");

	set_vo_avsync_flag((unsigned int)para1, (unsigned char)para2);

	return 0;
}

unsigned char get_vo_avsync_flag(unsigned int ch)
{
#if 1
	SCALER_AV_SYNC_INFO *av_sync_info = NULL;
	av_sync_info = (SCALER_AV_SYNC_INFO *)Scaler_GetShareMemVirAddr(SCALERIOC_AV_SYNC_READY_FLAG);
	if(av_sync_info)
	{
		if(ch)
		{
			return(av_sync_info->AV_SYNC_SUB_INFO ? TRUE : FALSE);
		}
		else
		{
			return(av_sync_info->AV_SYNC_MAIN_INFO ? TRUE : FALSE);
		}
	}
	return TRUE;
#else
	VO_DATA *vo = &vodma_data[0];

	if (ch >= 2) {
		rtd_pr_vo_err("[vo] %s error: ch %d doesn't exist\n", __func__, ch);
		ch = 0;
	}

	return vo->avsync[ch];
#endif
}

void set_vo_avsync_flag(unsigned int ch, unsigned char flag)
{
	VO_DATA *vo = &vodma_data[0];

	if (ch >= 2) {
		rtd_pr_vo_err("[vo] %s error: ch %d doesn't exist\n", __func__, ch);
		return;
	}

	vo->avsync[ch] = flag;

	rtd_pr_vo_info("[vo] set vo(%d) avsync: %d\n", ch, vo->avsync[ch]);
}

unsigned long rpcVoEOS(unsigned long para1, unsigned long para2)
{
    if(para1 >= VODMA_DEV_NUM){
        rtd_pr_vo_err("[vo] Invalid rpcVoEOS[%d]\n", (unsigned int)para1);
        return 0;
    }

    rtd_pr_vo_notice("[vo] P[%d] rpcVoEOS\n", (unsigned int)para1);
    set_vo_EOS_flag((unsigned int)para1, 1);

    return 0;
}

void set_vo_EOS_flag(unsigned int ch, unsigned char flag)
{
    VO_DATA *vo = &vodma_data[0];

    if (ch >= VODMA_DEV_NUM) {
        rtd_pr_vo_err("[vo] %s error: ch %d doesn't exist\n", __func__, ch);
        return;
    }

    if(vo->eos[ch] != flag){
        vo->eos[ch] = flag;
        rtd_pr_vo_notice("[vo] set vo(%d) EOS: %d\n", ch, vo->eos[ch]);
    }
}

unsigned char get_vo_EOS_flag(unsigned int ch)
{
	VO_DATA *vo = &vodma_data[0];

	if (ch >= VODMA_DEV_NUM) {
		rtd_pr_vo_err("[vo] %s error: ch %d doesn't exist\n", __func__, ch);
		ch = 0;
	}

	return vo->eos[ch];
}


// Film mode detect check
unsigned long rpcFilmDetectDone(unsigned long para1, unsigned long para2)
{
    if(para1 >= VODMA_DEV_NUM){
        rtd_pr_vo_err("[FILM] Invalid rpcFilmDetectDone[%d]\n", (unsigned int)para1);
        return 0;
    }

    rtd_pr_vo_notice("[FILM] P[%d] rpcFilmDetectDone=%d\n", (unsigned char)para1, (unsigned char)para2);
    set_film_detect_done_flag((unsigned char)para1, (unsigned char)para2);

    return 0;
}

void set_film_detect_done_flag(unsigned int ch, unsigned char flag)
{
    VO_DATA *vo = &vodma_data[0];

    if (ch >= VODMA_DEV_NUM) {
        rtd_pr_vo_err("[FILM] %s error: ch %d doesn't exist\n", __func__, ch);
        return;
    }

    if(vo->filmDetectDone[ch] != flag){
        vo->filmDetectDone[ch] = flag;
        rtd_pr_vo_notice("[FILM] set vo(%d) Film Detect Done=%d@%d\n", ch, vo->filmDetectDone[ch], rtd_inl(TIMER_SCPU_CLK90K_LO_reg));
    }
}

unsigned char get_film_detect_done_flag(unsigned int ch)
{
	VO_DATA *vo = &vodma_data[0];

	if (ch >= VODMA_DEV_NUM) {
		rtd_pr_vo_err("[FILM] %s error: ch %d doesn't exist\n", __func__, ch);
		ch = 0;
	}

	return vo->filmDetectDone[ch];
}

unsigned long rpcSetMEMCBypass(unsigned long para1, unsigned long para2)
{
	extern int Scaler_MEMC_MEMC_CLK_OnOff(unsigned char ME_enable, unsigned char MC_enable, unsigned char bMute);
	extern void Scaler_MEMC_Set_EnableToBypass_Flag(unsigned char u1_enable);
	extern void Scaler_MEMC_Set_BypassToEnable_Flag(unsigned char u1_enable);
	extern void Scaler_MEMC_SetInOutFrameRateByDisp(unsigned char _channel);
	extern unsigned int Get_DISPLAY_REFRESH_RATE(void);
//	extern void vbe_dtg_memec_bypass_switch_set(int mode);

	if (para1 > 2) {
		rtd_pr_vo_err("%s Invalid para1 = %ld\n", __func__, para1);
		return 0;
	}

	rtd_pr_vo_notice("%s param1 = %ld\n", __func__, para1);

	zoom_update_scaler_info_from_vo_smooth_toggle(SLR_MAIN_DISPLAY,0);//sync smooth toggle timing info
	modestate_decide_fractional_framesync();

	if(Get_DISPLAY_REFRESH_RATE() < 120){
		if (1 == para1) {	// set ME off, MC on
	//		vbe_dtg_memec_bypass_switch_set(1);
			Scaler_MEMC_Set_EnableToBypass_Flag(1);
			Scaler_MEMC_MEMC_CLK_OnOff(0, 1, 0);
			Scaler_MEMC_SetInOutFrameRateByDisp(SLR_MAIN_DISPLAY);
		}
		else if (2 == para1) {	// set MEMC on
	//		vbe_dtg_memec_bypass_switch_set(1);
			Scaler_MEMC_Set_BypassToEnable_Flag(1);
			Scaler_MEMC_MEMC_CLK_OnOff(1, 1, 0);
			Scaler_MEMC_SetInOutFrameRateByDisp(SLR_MAIN_DISPLAY);
		}
		else{
			Scaler_MEMC_SetInOutFrameRateByDisp(SLR_MAIN_DISPLAY);
		}
	}
	else{
		rtd_pr_vo_notice("%s 120Hz panel keep MEMC on\n", __func__);
		Scaler_MEMC_SetInOutFrameRateByDisp(SLR_MAIN_DISPLAY);
	}

	return 0;
}

unsigned char vo_run_smoothtoggle[MAX_DISP_CHANNEL_NUM];
unsigned char vo_open_smoothtoggle[MAX_DISP_CHANNEL_NUM];
unsigned char vo_smoothtoggle_fsmode[MAX_DISP_CHANNEL_NUM];
VO_3D_MODE_TYPE vo_3d_mode = VO_2D_MODE;
unsigned char vo_imd_disable[MAX_DISP_CHANNEL_NUM];
unsigned char vo_sequential_mode;
unsigned char vo_reset_sm_state[MAX_DISP_CHANNEL_NUM];
unsigned char vo_disp_port[VODMA_DEV_NUM];
unsigned char vo_vp9_flag[VODMA_DEV_NUM];
unsigned char vo_veComp_flag[VODMA_DEV_NUM]={0,0};
unsigned char vo_framerate_change[MAX_DISP_CHANNEL_NUM]={0,0};

void set_vo_vp9_flag(unsigned char ch, unsigned char enable)
{
	if (ch >= VODMA_DEV_NUM) {
		rtd_pr_vo_err("[vo]unvalid port\n");
		return;
	}
	vo_vp9_flag[ch] = enable;
}

unsigned char get_vo_vp9_flag(unsigned char ch)
{
	if (ch >= VODMA_DEV_NUM) {
		rtd_pr_vo_err("[vo]unvalid port\n");
		return 0;
	}
	return vo_vp9_flag[ch];
}


void set_vo_veComp_flag(unsigned char ch,unsigned char enable)
{
	if(ch >= VODMA_DEV_NUM){
		rtd_pr_vo_err("[vo]unvalid port@set_vo_veComp_flag\n");
		return;
	}
	vo_veComp_flag[ch] = enable;
}

unsigned char get_vo_veComp_flag(unsigned char ch)
{
	if(ch >= VODMA_DEV_NUM){
		rtd_pr_vo_err("[vo]unvalid port@get_vo_veComp_flag\n");
		return 0;
	}
	return vo_veComp_flag[ch];
}

unsigned char get_reset_smooth_toggle_state(unsigned char display)
{
	if (display >= MAX_DISP_CHANNEL_NUM)
		display = 0;
	return vo_reset_sm_state[display];
}

void set_reset_smooth_toggle_state(unsigned char enable, unsigned char display)
{
	if (display >= MAX_DISP_CHANNEL_NUM)
		display = 0;
	vo_reset_sm_state[display] = enable;
}

unsigned char get_vo_imd_disable(unsigned char display)
{
	if (display >= MAX_DISP_CHANNEL_NUM)
		display = 0;
	return vo_imd_disable[display];
}

void set_vo_imd_disable(unsigned char enable, unsigned char display)
{
	if (display >= MAX_DISP_CHANNEL_NUM)
		display = 0;
	vo_imd_disable[display] = enable;
}

unsigned char get_vo_mode_sequential(void)
{
	return vo_sequential_mode;
}

void set_vo_mode_sequential(unsigned char enable)
{
	vo_sequential_mode = enable;
}

unsigned char VO_get_disp_from_port(unsigned char port)
{
	if (port >= 2)
		port = 0;
	return vo_disp_port[port];
}

void VO_set_disp_to_port(unsigned char port, unsigned char display)
{
	if ((port >= 2) || (display >= MAX_DISP_CHANNEL_NUM)) {
		rtd_pr_vo_info("[wrong]port or display error\n");
		return;
	}
	vo_disp_port[port] = display;
}

unsigned char get_vo_run_smooth_toggle_enable(unsigned char display)
{
	if (display >= MAX_DISP_CHANNEL_NUM)
		display = 0;
	return vo_run_smoothtoggle[display];
}

#ifdef SMOOTH_TOGGLE_DATAFRCFS_TEST
unsigned char get_vo_smooth_toggle_fsmode(unsigned char display)
{
	if (display >= MAX_DISP_CHANNEL_NUM)
		display = 0;

	return vo_smoothtoggle_fsmode[display];
}

void set_vo_smooth_toggle_fsmode(unsigned char fsmode, unsigned char display)
{
	if (display >= MAX_DISP_CHANNEL_NUM)
		display = 0;
	vo_smoothtoggle_fsmode[display] = fsmode;
}
#endif

void set_vo_run_smooth_toggle_enable(unsigned char enable, unsigned char display)
{
	if (display >= MAX_DISP_CHANNEL_NUM)
		display = 0;
	vo_run_smoothtoggle[display] = enable;
	set_vo_imd_disable(enable, display);
}

unsigned char get_vo_open_smooth_toggle_enable(unsigned char display)
{
	if (display >= MAX_DISP_CHANNEL_NUM)
		display = 0;
	return vo_open_smoothtoggle[display];
}

void set_vo_open_smooth_toggle_enable(unsigned char enable, unsigned char display)
{
	if (display >= MAX_DISP_CHANNEL_NUM)
		display = 0;
	if ((enable >= SLR_SMOOTH_TOGGLE_INIT) && (enable < SLR_SMOOTH_TOGGLE_RELEASE))
		vo_open_smoothtoggle[display] = 1;
	else
		vo_open_smoothtoggle[display] = 0;
}

VO_3D_MODE_TYPE get_vo_3d_mode(void)
{
	return vo_3d_mode;
}

void set_vo_3d_mode(VO_3D_MODE_TYPE mode)
{
	vo_3d_mode = mode;
}

void set_force_change_vo_flag(unsigned char ch, unsigned char flag)
{
	if (ch >= 2) {
		rtd_pr_vo_err("[vo] %s error: ch %d doesn't exist\n", __func__, ch);
		ch = 0;
	}
	force_change_vo_flag[ch] = flag;
}
unsigned char get_vo_frameratechange_enable(unsigned char display)
{
	if (display >= MAX_DISP_CHANNEL_NUM)
		return 0;
	return vo_framerate_change[display];
}

void set_vo_frameratechange_enable(unsigned char enable, unsigned char display)
{
	if (display >= MAX_DISP_CHANNEL_NUM)
		return;
	vo_framerate_change[display] = enable;
}
unsigned long rpcVoHdrFrameInfo(unsigned long mode, unsigned long para1)
{
	extern uint8_t *get_hdmi_metadata_base_address(void);
	if (mode == HDR_DOLBY_COMPOSER) {
		/* //rtd_pr_vo_info("%x\n", mdOutputAddr); */
                #ifdef CONFIG_RTK_KDRV_DV
#ifdef	CONFIG_DOLBY_IDK_1_6_1_1
		extern void Normal_TEST(void *p_mdOutput, unsigned int rpcType);
#else
		extern void Normal_TEST(DOLBYVISION_MD_OUTPUT *p_mdOutput, unsigned int rpcType);
#endif
                unsigned int mdOutputAddr ;
                #endif
		flush_cache_all();
		/* //unsigned int stc = IoReg_Read32(VCPU_CLK90K_LO_reg); */
#ifdef CONFIG_RTK_KDRV_DV
		mdOutputAddr = Scaler_ChangeUINT32Endian(para1);
#ifdef	CONFIG_DOLBY_IDK_1_6_1_1
		dolby_adapter_Normal_TEST(phys_to_virt(mdOutputAddr), 1);
#else
		Normal_TEST((DOLBYVISION_MD_OUTPUT *)phys_to_virt(mdOutputAddr), 1);
#endif
#endif
		/* //rtd_pr_vo_info("%d\n", IoReg_Read32(VCPU_CLK90K_LO_reg) - stc); */
	} else if (mode == HDR_DOLBY_HDMI) {
		if ((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE) && (Scaler_DispGetInputInfo(SLR_INPUT_THROUGH_I3DDMA) == TRUE)) {
			extern void HDMI_TEST(unsigned int wid, unsigned int len, unsigned int mdAddr);
			VIDEO_RPC_DOBLBY_VISION_SHM *pHdrDvShm = (VIDEO_RPC_DOBLBY_VISION_SHM *)Scaler_GetShareMemVirAddr(SCALERIOC_GET_HDMI_HDR_FRAMEINFO);
			static unsigned int lastStc;
			unsigned int stc = 0;//useTime;
			SLR_VOINFO *pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());

			if (pHdrDvShm) {
				static unsigned int lastIdxRd;
				unsigned int idxRd, idxWrt;
				VIDEO_RPC_DOLBY_VISION_FRAME_INFO frameInfo;
				unsigned int *pulTemp;
				unsigned int i = 0, ulItemCount = 0;

				/* change endian */
				idxWrt = Scaler_ChangeUINT32Endian(pHdrDvShm->picQWr);
				idxRd  = (idxWrt == 0 ? 15 : idxWrt-1);
				pHdrDvShm->picQRd = Scaler_ChangeUINT32Endian(idxRd);
				if (idxRd <= 15) {
					memcpy(&frameInfo, &pHdrDvShm->frameInfo[idxRd], sizeof(VIDEO_RPC_DOLBY_VISION_FRAME_INFO));
					ulItemCount = sizeof(VIDEO_RPC_DOLBY_VISION_FRAME_INFO) /  sizeof(UINT32);
					pulTemp = (unsigned int *)&frameInfo;
					for (i = 0; i < ulItemCount; i++)
						pulTemp[i] = Scaler_ChangeUINT32Endian(pulTemp[i]);

			#if 1 /* // Dolby Vision HDMI mode DM setting */
				if ((frameInfo.md_bufPhyAddr != 0) && frameInfo.md_pktSize && frameInfo.picWidth && frameInfo.picLen) {
					static unsigned int last_pts;
					unsigned char bRptPkt = 0;

					stc = rtd_inl(TIMER_SCPU_CLK90K_LO_reg);
					/* // check repeat packet: (repeat flag==1) && (write index continue) && (pts diff < 24hz) */
					if ((frameInfo.md_pts & 0x1) && ((idxRd - lastIdxRd == 1) || (lastIdxRd - idxRd == 15)) && ((frameInfo.md_pts - last_pts) < (42*90))) {
						/* // check in HDMI mode */
						//if ((IoReg_Read32(DOLBY_V_TOP_TOP_CTL_reg) & DOLBY_V_TOP_TOP_CTL_dolby_mode_mask) == 1) //Mark by will
							bRptPkt = 1;
						/* //rtd_pr_vo_info("R.%d-%d(%d)\n", lastIdxRd, idxRd, (frameInfo.md_pts - last_pts) / 90); */
					}

					if ((lastIdxRd != idxRd) && !bRptPkt) {
						/* // [FIX-ME] DM setting apply need move to Video FW data end ISR */
#ifdef CONFIG_RTK_KDRV_DV
#ifdef	CONFIG_DOLBY_IDK_1_6_1_1
						dolby_adapter_HDMI_TEST(frameInfo.picWidth, frameInfo.picLen, frameInfo.md_bufPhyAddr + get_hdmi_metadata_base_address());
#else
						HDMI_TEST(frameInfo.picWidth, frameInfo.picLen, frameInfo.md_bufPhyAddr + get_hdmi_metadata_base_address());
#endif
#endif
						/* //rtd_pr_vo_info("n\n"); */
					}
					//useTime = rtd_inl(TIMER_SCPU_CLK90K_LO_reg) - stc;
					last_pts = frameInfo.md_pts;
					lastIdxRd = idxRd;
				 } else {
					/* //rtd_pr_vo_info("[DV]Info=%dx%d(%d)@%x\n", frameInfo.picWidth, frameInfo.picLen, frameInfo.md_pktSize, frameInfo.md_bufPhyAddr); */
					rtd_pr_vo_notice("[DV]Err=%x\n", frameInfo.md_status);
				}
			#else
					/* // Dump write index frame info */
					rtd_pr_vo_info("%d|%d, %d, %x, %d, %x\n", idxRd, idxWrt,
						 frameInfo.md_status, frameInfo.md_pts, frameInfo.md_pktSize, frameInfo.md_bufPhyAddr);
			#endif

			#if 0 /* // dump packet body data */
				if (1) {
					static unsigned int dbgCnt;

					frameInfo.md_bufPhyAddr = (frameInfo.md_bufPhyAddr & 0x1fffffff)|0xc0000000; /* // physical address cast to SCPU mapping address */
					if (++dbgCnt % 20 == 0) {
						int i;
						unsigned int dumpCnt = (frameInfo.md_pktSize/4)+(frameInfo.md_pktSize%4 ? 1 : 0);
						/* // dump packet body */
						for (i = 0; i < dumpCnt; i += 4) {
							rtd_pr_vo_info("\n%08x%08x%08x%08x",
								 Scaler_ChangeUINT32Endian(*((unsigned int *)(frameInfo.md_bufPhyAddr+((i+0)*4)))),
								 Scaler_ChangeUINT32Endian(*((unsigned int *)(frameInfo.md_bufPhyAddr+((i+1)*4)))),
								 Scaler_ChangeUINT32Endian(*((unsigned int *)(frameInfo.md_bufPhyAddr+((i+2)*4)))),
								 Scaler_ChangeUINT32Endian(*((unsigned int *)(frameInfo.md_bufPhyAddr+((i+3)*4)))));
						}
						rtd_pr_vo_info("\n");
					}
				}
			#endif
				} else {
					rtd_pr_vo_info("[ERR][DV] idxRd[%d] > 15\n", idxRd);
				}
#if 1 /* // [TEST] Dolby Vision check DM LUT convert time */
			/* // check (use time > 5ms)||(rpc/shm index not match)||(rpc stc diff > 17ms) */
			if (/*(useTime > 5*90) ||*/(para1 != idxRd) || (stc - lastStc > ((10000/pVOInfo->v_freq)+1)*90)) {
				/* //rtd_pr_vo_info("%d/%d/%d=%d\n", para1, idxRd, idxWrt, (stc - lastStc)/90); */
				if (para1 != idxRd)
					rtd_pr_vo_info("%d/%d\n", (unsigned int)para1, idxRd);
				else if (stc - lastStc > ((10000/pVOInfo->v_freq)+1)*90)
					rtd_pr_vo_info("dis=%d(%d)\n", (stc - lastStc)/90, ((10000/pVOInfo->v_freq)+1)*90);
			}
			lastStc = stc;
#endif
			}
		}
	} else {
		rtd_pr_vo_info("[DolbyVision] UNKNOWN MODE[%d]\n", (unsigned int)mode);
	}

	return 0;
}
#if 0
unsigned char get_vo_plane_flag(unsigned char display)
{
	VO_DATA *vo = &vodma_data[0];
	unsigned int ch = (display);

	if (ch >= 2) {
		rtd_pr_vo_err("[vo] %s error: ch %d doesn't exist\n", __func__, ch);
		ch = 0;
	}
	return vo->videoPlane[ch].VOInfo.plane;
}
#endif
void enable_vodma_empty_gate(unsigned char plane)
{
	if (plane == VO_VIDEO_PLANE_V1) {
		/* //VO decompress mode don't support empty gate */
		if ((rtd_inl(VODMA_DECOMP_CTRL0_reg) & _BIT0) == 0)
			rtd_maskl(VODMA_VODMA_CLKGEN_reg, ~(_BIT3), _BIT3);
	} else if (plane == VO_VIDEO_PLANE_V2) {
		/* //VO decompress mode don't support empty gate */
		if ((rtd_inl(VODMA2_DECOMP2_CTRL0_reg) & _BIT0) == 0)
			rtd_maskl(VODMA2_VODMA2_CLKGEN_reg, ~(_BIT3), _BIT3);
	}
}


#ifdef CONFIG_HDR_SDR_SEAMLESS
spinlock_t* get_vdec_seamless_change_spinlock(void)
{//this is for vdec_seamless_change_flag protection
	return &VDEC_SEAMLESS_CHANGE_SPINLOCK;
}

void set_vdec_seamless_change_flag(unsigned char TorF)
{
	vdec_seamless_change_flag = TorF;
}

unsigned char get_vdec_seamless_change_flag(void)
{
	return vdec_seamless_change_flag;
}
#endif


#ifdef ENABLE_DUMP_FRAME_INFO
// Dump VO frame CRC info to file
#ifndef CONFIG_KDRIVER_USE_NEW_COMMON
static int rtkvo_dumpcrc_thread(void *arg)
{
	unsigned long magic, size, wr, rd;
	unsigned char *wrPtr, *rdPtr, *basePtr, *limitPtr;

	for (;;) {
		if (kthread_should_stop()) break;
		if (file_DumpCRC != 0 && DumpCRC_Config.enable) {
			VO_DUMP_BUFFER_HEADER *header;
			header = (VO_DUMP_BUFFER_HEADER*) g_pDumpCRC.Memory;
			magic = ntohl(header->magic) ;
			size  = ntohl(header->size) ;
			rd    = ntohl(header->rd) ;
			wr    = ntohl(header->wr) ;
			wrPtr = (unsigned char *)(g_pDumpCRC.Memory + wr - g_pDumpCRC.PhyAddr); /* make virtual address */
			rdPtr = (unsigned char *)(g_pDumpCRC.Memory + rd - g_pDumpCRC.PhyAddr); /* make virtual address */
			basePtr  = (unsigned char *)(g_pDumpCRC.Memory + sizeof(VO_DUMP_BUFFER_HEADER));
			size -= sizeof(VO_DUMP_BUFFER_HEADER);
			limitPtr = basePtr+ size;

                #if 0 // TEST
                    static unsigned long last_wr=0;
                    if(last_wr != wr){
                        rtd_pr_vo_notice("[vo] Header %x, m=%x size=%x r(%x) w(%x)\n", (unsigned int)header, (unsigned int)magic, (unsigned int)size, (unsigned int)rd, (unsigned int)wr);
                        rtd_pr_vo_notice("[vo] Ptr w(%x) r(%x) b(%x) (%x), offset=%x\n", (unsigned int)wrPtr, (unsigned int)rdPtr, (unsigned int)basePtr, (unsigned int)limitPtr, (unsigned int)f_offset_dumpcrc);
                        last_wr = wr;
                    }
                #endif

			if (wrPtr < rdPtr) {
				wrPtr = wrPtr + size;
			}

			if (wrPtr > rdPtr) {
				if (wrPtr > limitPtr) {
					file_write(file_DumpCRC, f_offset_dumpcrc, rdPtr, limitPtr -rdPtr) ;
					f_offset_dumpcrc += limitPtr -rdPtr ;
					file_write(file_DumpCRC, f_offset_dumpcrc, basePtr, wrPtr - limitPtr) ;
					f_offset_dumpcrc += wrPtr - limitPtr ;
				}
				else {
					file_write(file_DumpCRC, f_offset_dumpcrc, rdPtr, wrPtr - rdPtr) ;
					f_offset_dumpcrc += wrPtr - rdPtr ;
				}
				header->rd = htonl(wr) ;
                #if 0 // TEST
                         rtd_pr_vo_notice("[vo] f_offset_dumpcrc=%x\n", (unsigned int)f_offset_dumpcrc);
                #endif
			}
		}

		msleep(100); /* sleep 10 ms */
	}

	/* rtd_pr_vo_info("rtkvdec_dumpes_thread break\n"); */
	rtd_pr_vo_err("rtkvo_dumpcrc_thread break\n");
	return 0;
}
#endif


static void rtkvo_dump_init(void)
{
#ifndef CONFIG_ANDROID
	unsigned char default_file_name3[26] = "/tmp/video_dumpvocrc.bin" ;
#else
	unsigned char default_file_name3[32] = "/data/data/video_dumpvocrc.bin" ;
#endif

	DumpCRC_Config.enable = 0 ;
	DumpCRC_Config.mem_size = DUMP_ES_SIZE ;

#ifndef CONFIG_ANDROID
	memset((void *)DumpCRC_Config.file_name, '\0', FILE_NAME_SIZE );
	memcpy((void *)DumpCRC_Config.file_name, (void *)default_file_name3, 21);
#else
	memset((void *)DumpCRC_Config.file_name, '\0', 27 + 1 );
	memcpy((void *)DumpCRC_Config.file_name, (void *)default_file_name3, 27);
#endif

	g_pDumpCRC.Memory = g_pDumpCRC.PhyAddr = g_pDumpCRC.VirtAddr = 0 ;
	g_pDumpCRC_Send.Memory = g_pDumpCRC_Send.PhyAddr = g_pDumpCRC_Send.VirtAddr = 0 ;
}



int rtkvo_dumpCRC_enable(const char *pattern, int length)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	//unsigned long return_value ;
#endif
#ifndef CONFIG_KDRIVER_USE_NEW_COMMON
	unsigned long vir_addr = 0;
#endif
	if (length > FILE_NAME_SIZE - 1) {
		rtd_pr_vo_warn("rtkvdec: file name is too long(<%d).\n", FILE_NAME_SIZE - 1);
		return 0;
	}
	else if (length > 0) {
		memset((void *)DumpCRC_Config.file_name, '\0', length+1);
		memcpy((void *)DumpCRC_Config.file_name, (void *)pattern, length);
	}

	rtd_pr_vo_notice("rtkvo: dumpes_file_name(%s)\n", DumpCRC_Config.file_name);

	if (DumpCRC_Config.enable) {
		rtd_pr_vo_notice("rtkvo: es dump already enable!\n");
		return 0 ;
	}

	file_DumpCRC = file_open((char *)(DumpCRC_Config.file_name), O_TRUNC | O_RDWR | O_CREAT,0x755) ;
	if (file_DumpCRC == 0) {
		rtd_pr_vo_err("[%s %d]open log file fail\n",__FUNCTION__,__LINE__);
		return -ENOMEM;
	}

	/* allocate debug memory */
#ifndef CONFIG_KDRIVER_USE_NEW_COMMON
	vir_addr = (unsigned long)dvr_malloc_uncached_specific(sizeof(VO_RPC_DEBUG_MEMORY)+256, GFP_DCU1_LIMIT, (void **)(&g_pDumpCRC_Send.Memory));
	if (!vir_addr) {
		rtd_pr_vo_err("[%s %d]alloc debug memory fail\n",__FUNCTION__,__LINE__);
		return -ENOMEM;
	}

	g_pDumpCRC_Send.PhyAddr = (unsigned long)dvr_to_phys((void*)vir_addr);
	g_pDumpCRC_Send.VirtAddr = vir_addr ;
	rtd_pr_vo_notice("Alloc DVOCRCME v(%lx) p(%lx) vn(%lx)\n", vir_addr, g_pDumpCRC_Send.PhyAddr, g_pDumpCRC_Send.Memory);

	if ((file_DumpCRC != 0) && (DumpCRC_Config.mem_size > 0)) {
		/* alocate dump memory */
		vir_addr = (unsigned long)dvr_malloc_uncached_specific(DumpCRC_Config.mem_size, GFP_DCU1_LIMIT, (void **)(&g_pDumpCRC.Memory));
		if (!vir_addr) {
			vir_addr = g_pDumpCRC_Send.VirtAddr ;
			dvr_free((void*)vir_addr);
			rtd_pr_vo_err("[%s %d]alloc debug memory fail\n",__FUNCTION__,__LINE__);
			return -ENOMEM;
		}
		g_pDumpCRC.PhyAddr = (unsigned long)dvr_to_phys((void*)vir_addr);
		g_pDumpCRC.VirtAddr = vir_addr ;

		rtd_pr_vo_notice("Alloc DUMPVOME v(%lx) p(%lx) vn(%lx)\n", vir_addr, g_pDumpCRC.PhyAddr, g_pDumpCRC.Memory);

		/* setup dump es ring buffer header */
		header = (VO_DUMP_BUFFER_HEADER*) g_pDumpCRC.Memory;
		header->magic = htonl(0xdeadcafe) ;
		header->size = htonl(DumpCRC_Config.mem_size) ;
		header->rd = htonl(g_pDumpCRC.PhyAddr + sizeof(VO_DUMP_BUFFER_HEADER)) ;
		header->wr = htonl(g_pDumpCRC.PhyAddr + sizeof(VO_DUMP_BUFFER_HEADER)) ;

		*(unsigned long *)g_pDumpCRC_Send.Memory = htonl(g_pDumpCRC.PhyAddr) ;

		rtd_pr_vo_notice("[vo] Header %lx, m=%x size=%x r(%x) w(%x)\n", (unsigned long)header, header->magic, header->size, header->rd, header->wr);
	}
	else {
		rtd_pr_vo_notice("rtkvdec: no allocate debug dump ring buffer!\n");
		return 0 ;
	}

	rtkvo_dumpcrc_tsk = kthread_create(rtkvo_dumpcrc_thread, &data, "rtkvo_dumpcrc_thread");
	if (IS_ERR(rtkvo_dumpcrc_tsk)) {
		rtkvo_dumpcrc_tsk = NULL;
		return -1 ;
	}
	wake_up_process(rtkvo_dumpcrc_tsk);

	DumpCRC_Config.enable = 1 ;
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
// FixMe, 20190920

#if 0
	if (send_rpc_command(RPC_VIDEO, VIDEO_RPC_VOUT_ToAgent_DumpFrameCRCInfo, (unsigned long)g_pDumpCRC_Send.PhyAddr, _ENABLE, &return_value)){
            rtd_pr_vo_err("rtkvo: VIDEO_RPC_VOUT_ToAgent_DumpFrameCRCInfo fail %ld\n", return_value);
            rtkvo_dumpCRC_disable();
            return -1;
       }
#endif

#endif
#endif //CONFIG_KDRIVER_USE_NEW_COMMON
	return 0;
}


void rtkvo_dumpCRC_disable(void)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	//unsigned long return_value ;
#endif
	int ret = 0;
	unsigned long vir_addr ;
	VO_DUMP_BUFFER_HEADER *header;

	if (!DumpCRC_Config.enable) {
		rtd_pr_vo_notice("rtkvo: es dump not enable!\n");
		return ;
	}

	DumpCRC_Config.enable = 0 ;
	/* setup debug dump ring buffer header */
	header = (VO_DUMP_BUFFER_HEADER*) g_pDumpCRC.Memory;
	header->magic = htonl(0xdeadcafe) ;
	header->size = htonl(16) ;
	header->rd = htonl(g_pDumpCRC.PhyAddr + sizeof(VO_DUMP_BUFFER_HEADER)) ;
	header->wr = htonl(g_pDumpCRC.PhyAddr + sizeof(VO_DUMP_BUFFER_HEADER)) ;
	*(unsigned long *)g_pDumpCRC_Send.Memory = htonl(g_pDumpCRC.PhyAddr) ;

#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
// FixMe, 20190920
#if 0
	if (send_rpc_command(RPC_VIDEO, VIDEO_RPC_VOUT_ToAgent_DumpFrameCRCInfo, 0, 0, &return_value))
		rtd_pr_vo_err("rtkvo: VIDEO_RPC_VOUT_ToAgent_DumpFrameCRCInfo fail %ld\n", return_value);
#endif
#endif
	if(rtkvo_dumpcrc_tsk){
		ret = kthread_stop(rtkvo_dumpcrc_tsk);
		if (!ret){
			rtkvo_dumpcrc_tsk = NULL;
			rtd_pr_vo_info("rtkvo dumpes thread stopped\n");
		}
	}else{
		rtd_pr_vo_info("rtkvo dumpes thread stopped already\n");
	}
	f_offset_dumpcrc = 0 ;
	if (g_pDumpCRC.PhyAddr) {
		vir_addr = g_pDumpCRC.VirtAddr;
		dvr_free((void*)vir_addr);
		g_pDumpCRC.Memory = g_pDumpCRC.PhyAddr = g_pDumpCRC.VirtAddr = 0 ;
	}

	if (g_pDumpCRC_Send.PhyAddr) {
		vir_addr = g_pDumpCRC_Send.VirtAddr;
		dvr_free((void*)vir_addr);
		g_pDumpCRC_Send.Memory = g_pDumpCRC_Send.PhyAddr = g_pDumpCRC_Send.VirtAddr = 0 ;
	}

	if (file_DumpCRC)
		file_close(file_DumpCRC) ;
	file_DumpCRC = 0 ;
}
#endif

