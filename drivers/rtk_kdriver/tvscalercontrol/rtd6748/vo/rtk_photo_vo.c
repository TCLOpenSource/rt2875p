/* Kernel Header file */
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>       /* everything... */
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/string.h>   /* memset */
#include <linux/uaccess.h>  /* copy_from_user */
#include <linux/slab.h>   /* kmalloc */
#include <rtk_kdriver/rtk_vdec.h>
#include <rtk_kdriver/RPCDriver.h>   /* register_kernel_rpc, send_rpc_command */
#include <asm/cacheflush.h>
#include <mach/platform.h>

#include "rtk_photo_vo.h"
#include "rtk_vo.h"
#include <rbus/sub_vgip_reg.h>

VO_DATA *photovodma_data; /* allocated in vo_module_init */
static dev_t photovo_devno; /* vo device number */
static struct cdev photovo_cdev;
static struct semaphore VO_MemChunkSemaphore;
static struct semaphore VO_OpenCloseSemaphore;
// -------------------------------


int photovodev_open(struct inode *inode, struct file *filp)
{
    //pr_err("%s():\n",__FUNCTION__);
    return 0;
}

ssize_t photovodev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    return 0;
}

ssize_t photovodev_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    long ret = count;
    char cmd_buf[100] = {0};

    #ifndef CONFIG_ARM64
    pr_info("%s(): count=%d, buf=0x%08lx\n", __func__, count, (long)buf);
    #endif

    if (count >= 100)
        return -EFAULT;

    if (copy_from_user(cmd_buf, buf, count)) {
        ret = -EFAULT;
    }
    return ret;
}

int photovodev_release(struct inode *inode, struct file *filp)
{
    //pr_err("%s():\n",__FUNCTION__);
    return 0;
}

long photovodev_ioctl(struct file *filp, unsigned int cmd,  unsigned long arg)
{
    int ret = 0;

    if (_IOC_TYPE(cmd) != PHOTOVO_IOC_MAGIC || _IOC_NR(cmd) > PHOTOVO_IOC_MAXNR)
    {
        pr_err("PHOTOVO: ioctl code = %x is invalid!!!!!!!!!!!!!!!\n", cmd);
        return -ENOTTY;
    }

    pr_info("PHOTOVO: ioctl code = %x\n", cmd);

    switch (cmd) {
    case PHOTOVO_IOC_OPEN:
        {
            VO_OPEN_CMD_T data;
            //copy data from args(user) to val(kernel)
            if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(VO_OPEN_CMD_T)))
            {
                ret = -EFAULT;
                pr_err("VO: ioctl code = %ld failed!!!!!!!!!!!!!!!\n", (unsigned long)PHOTOVO_IOC_OPEN);
            }
            else
            {
                ret = PHOTO_VO_Open((VO_OPEN_CMD_T *)&data);
                //printk("%s():get val from user = %d\n",__FUNCTION__,val);
                //copy data from val(kernel) to args(user)
                if (copy_to_user((void __user *)arg, (const void *)&data, sizeof(VO_OPEN_CMD_T)))
                {
                    ret = -EFAULT;
                    pr_err("VO: ioctl code = %ld failed!!!!!!!!!!!!!!!\n", (unsigned long)PHOTOVO_IOC_OPEN);
                }
                //printk("%s():set val to %d\n",__FUNCTION__, val);
            }
        }
        break;
    case PHOTOVO_IOC_CLOSE:
        {
            unsigned int data;

            if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(unsigned int)))
            {
                ret = -EFAULT;
                pr_err("VO: ioctl code = %ld failed!!!!!!!!!!!!!!!\n", (unsigned long)PHOTOVO_IOC_CLOSE);
            } else
            {
                ret = PHOTO_VO_Close((unsigned int)data);
            }
        }
        break;
    case PHOTOVO_IOC_DISPLAYPIC_DONE:
        {
            VO_DISPLAY_CMD_T data;
            if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(VO_DISPLAY_CMD_T))) {
                ret = -EFAULT;
                pr_err("VO: ioctl code = %ld failed!!!!!!!!!!!!!!!\n", (unsigned long)PHOTOVO_IOC_DISPLAYPIC_DONE);
            }
            else
            {
                pr_info("VO: ioctl code = VO_IOC_DISPLAYPIC_DONE \n");
                ret = PHOTO_VO_DisplayPicture_Done();
            }
        }
        break;
    default:
        pr_err("VO: ioctl code = %d is invalid!!!!!!!!!!!!!!!\n", cmd);
        break;
	}

    return ret;
}

const struct file_operations photovo_fops = {
    .owner          =   THIS_MODULE,
    .open           =   photovodev_open,
    .release        =   photovodev_release,
    .read           =   photovodev_read,
    .write          =   photovodev_write,
    .unlocked_ioctl =   photovodev_ioctl,
#if defined(CONFIG_ARM64) && defined(CONFIG_COMPAT)
    .compat_ioctl   =   photovodev_ioctl,
#endif
};

static struct class *photovo_class;
static struct platform_device *photovo_platform_devs;
static struct platform_driver photovo_platform_driver = {
    .driver = {
        .name   =   "photovodev",
        .bus    =   &platform_bus_type,
    },
};

static char *photovodev_devnode(struct device *dev, mode_t *mode)
{
    return NULL;
}

void photovo_module_exit(void)
{
    if ((!photovo_platform_devs) /*|| (class_destroy == NULL)*/)
        BUG();

    kfree(photovodma_data);
    photovodma_data = NULL;

    device_destroy(photovo_class, photovo_devno);
    cdev_del(&photovo_cdev);

    platform_driver_unregister(&photovo_platform_driver);
    platform_device_unregister(photovo_platform_devs);
    photovo_platform_devs = NULL;

    class_destroy(photovo_class);
    photovo_class = NULL;

    unregister_chrdev_region(photovo_devno, 1);

    pr_info("\n\n\n\n *****************  VO module exit  *********************\n\n\n\n");
}

int photovo_module_init(void)
{
    int result;
    int ch = 0;
    unsigned char VoReady;

    VoReady = 1;

    pr_info("\n\n\n\n *****************  PHOTO VO module init  *********************\n\n\n\n");

    result = alloc_chrdev_region(&photovo_devno, 0, 1, "photovodevno");

    if (result != 0) {
        pr_crit("Cannot allocate PHOTOVO device number\n");
        return result;
    }

    photovo_class = class_create(THIS_MODULE, "photovodev");
    if (IS_ERR(photovo_class)) {
        pr_crit("photovo: can not create class...\n");
        result = PTR_ERR(photovo_class);
        goto fail_class_create;
    }

    photovo_class->devnode = (void *)photovodev_devnode;

    photovo_platform_devs = platform_device_register_simple("photovodev", -1, NULL, 0);
    if (platform_driver_register(&photovo_platform_driver) != 0) {
        pr_crit("photovo: can not register platform driver...\n");
        result = -ENOMEM;
        goto fail_platform_driver_register;
    }

    cdev_init(&photovo_cdev, &photovo_fops);
    result = cdev_add(&photovo_cdev, photovo_devno, 1);
    if (result < 0) {
        pr_crit("photovo: can not add character device...\n");
        goto fail_cdev_init;
    }

    device_create(photovo_class, NULL, photovo_devno, NULL, "photovodev");

    photovodma_data = kmalloc(sizeof(VO_DATA), GFP_KERNEL);
    if (!photovodma_data) {
        result = -ENOMEM;
        photovo_module_exit();   /* fail */
        return result;
    }

    sema_init(&VO_MemChunkSemaphore, 1);
    sema_init(&VO_OpenCloseSemaphore, 1);
    memset(photovodma_data, 0, sizeof(VO_DATA));

    for (ch = 0; ch < VODMA_DEV_NUM; ch++)
        photovodma_data[0].vo_state[ch] = VO_STATE_INIT;

    for (ch = 0; ch < VODMA_DEV_NUM; ch++)
        photovodma_data[0].avsync[ch] = 1;

    for (ch = 0; ch < VODMA_DEV_NUM; ch++)
        photovodma_data[0].filmDetectDone[ch] = 1;

    pr_info("PHOTOVO module_init finish\n");
    return 0; /* Success */

fail_cdev_init:
    platform_driver_unregister(&photovo_platform_driver);
fail_platform_driver_register:
    platform_device_unregister(photovo_platform_devs);
    photovo_platform_devs = NULL;
    class_destroy(photovo_class);
fail_class_create:
    photovo_class = NULL;
    unregister_chrdev_region(photovo_devno, 1);
    return result;
}

module_init(photovo_module_init);
module_exit(photovo_module_exit);

#ifdef CONFIG_VBM_HEAP
int PHOTO_VBM_ReturnMemChunk(unsigned long dmabuf_hndl, unsigned long buffer_addr_in, unsigned long buffer_size_in)
{
	unsigned long phy_addr;

	if ((buffer_addr_in == 0) || (buffer_size_in == 0) || IS_ERR((struct dma_buf *)dmabuf_hndl)) {
		pr_err("[vo] Return an invalid address %x, dmabuf(%lx)\n", (unsigned int)buffer_addr_in, (unsigned long)dmabuf_hndl);
		return -1;
	}

	phy_addr = dvr_dmabuf_to_phys((struct dma_buf *)dmabuf_hndl);
	if (phy_addr != buffer_addr_in) {
		pr_err("[vo] Return an invalid address %x, dmabuf_to_phys(%lx)\n", (unsigned int)buffer_addr_in, (unsigned long)phy_addr);
		return -1;
	}

	pr_info("[vo] PHOTO_VBM_ReturnMemChunk: return addr %x, size %x\n", (unsigned int)buffer_addr_in, (unsigned int)buffer_size_in);
	down(&VO_MemChunkSemaphore);
	dvr_dmabuf_free((struct dma_buf *)dmabuf_hndl);
	up(&VO_MemChunkSemaphore);
	return 0;
}

int PHOTO_VBM_BorrowMemChunk(unsigned long *dmabuf_hndl, unsigned long *buffer_addr, unsigned long *buffer_size, UINT32 reqSize, UINT32 type)
{
	unsigned long phy_addr;
	struct dma_buf *dmabuf;
	unsigned long ret=0;

	if ((!buffer_addr) || (!buffer_size) || (reqSize == 0)) {
		pr_err("[vo] BorrowVBMMemChunk parameter fail!!\n");
		return -1;
	}

	pr_info("[vo] PHOTO_VBM_BorrowMemChunk: request size %d, type =%d\n", reqSize, type);

	down(&VO_MemChunkSemaphore);
    dmabuf = dvr_dmabuf_chunk_malloc(reqSize, VBM_BUF_DECOMP, DMA_HEAP_USER_COBUFFER, false, (unsigned long **)&phy_addr);
	if (phy_addr == INVALID_VAL) {
		pr_warn("[%s %d]alloc VBM_CMA memory fail\n", __func__, __LINE__);
		*buffer_addr = 0;
		*buffer_size = 0;
		up(&VO_MemChunkSemaphore);
		return -1;
	}

	*buffer_addr = phy_addr;
	*buffer_size = reqSize;
	if (dmabuf_hndl)
		*dmabuf_hndl = (unsigned long)dmabuf;
	pr_info("[vo] PHOTO_VBM_BorrowMemChunk: get addr %x, size %d\n", (unsigned int)*buffer_addr, (unsigned int)*buffer_size);
	if (send_rpc_command(RPC_VIDEO, VIDEO_RPC_VO_PHOTO_ToAgent_SetAddr , 0, 0, &ret) == RPC_FAIL) {
			pr_err("[vo] VIDEO_RPC_VO_PHOTO_ToAgent_SetAddr fail!!\n");
			up(&VO_MemChunkSemaphore);
			return -1;
	}
	up(&VO_MemChunkSemaphore);
	return 0;
}
#endif

void photovo_free_hal_buffer(unsigned int ch)
{
    VO_DATA *vo = &photovodma_data[0];

    int i = 0;
    for (i = 0; i < VODMA_HALBUF_NUM; i++) {
        if (vo->hal_phy_addr[ch][i] && vo->hal_phy_addr[ch][i] != INVALID_VAL) {
            if (vo->borrow_from_vbm[ch][i])
            {
#ifdef CONFIG_VBM_HEAP
                PHOTO_VBM_ReturnMemChunk(vo->dmabuf_hndl[ch][i], vo->hal_phy_addr[ch][i], vo->hal_buf_size[ch][i]);
#else
                PHOTO_VBM_ReturnMemChunk(vo->hal_phy_addr[ch][i], vo->hal_buf_size[ch][i]);
#endif
            }
            else
            {
                pr_info("[photo_vo] free hal_phy_addr[%d][%d]: pli buf:%x, size:%d\n", ch, i, (unsigned int)vo->hal_phy_addr[ch][i], (unsigned int)vo->hal_buf_size[ch][i]);
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

int photovo_malloc_hal_buffer(unsigned int ch)
{
    VO_DATA *vo = &photovodma_data[0];
    int reqSize = 0;
    bool DCUSizeDiff = false;
    int i = 0;

    photovo_free_hal_buffer(ch);

    if (get_platform() == PLATFORM_KXLP) {

        DCUSizeDiff = true;
    }

    if (1) {

        for (i = 0; i < VODMA_HALBUF_NUM; i++)
        {
            if (vo->panel_type[ch] == HAL_VO_PANEL_TYPE_UHD)
            {
                if(i==0)
                    reqSize = PANEL_UHD_WIDTH*PANEL_UHD_HEIGHT*8;  /* WIDTH*2 for slide */
                else
                    reqSize = PANEL_FHD_WIDTH*2*PANEL_FHD_HEIGHT*3;  /* WIDTH*2 for slide */
            }
            else
            {
                if(i==0)
                    reqSize = PANEL_UHD_WIDTH*PANEL_UHD_HEIGHT*8;  /* WIDTH*2 for slide */
                else
                    reqSize = PANEL_FHD_WIDTH*2*PANEL_FHD_HEIGHT*3;  /* WIDTH*2 for slide */
            }
            if (vo->hal_phy_addr[ch][i] == 0)
            {
#ifdef CONFIG_VBM_HEAP
                if (PHOTO_VBM_BorrowMemChunk(&vo->dmabuf_hndl[ch][i], (unsigned long *)(&vo->hal_phy_addr[ch][i]), (unsigned long *)(&vo->hal_buf_size[ch][i]), reqSize, i+1) == 0)
#else
                if (PHOTO_VBM_BorrowMemChunk((unsigned long *)(&vo->hal_phy_addr[ch][i]), (unsigned long *)(&vo->hal_buf_size[ch][i]), reqSize, i+1) == 0)
#endif
                {
                    pr_info("[vo] borrow vbm buf: hal_phy_addr[%d][%d]:%x, size=%d\n", ch, i, (unsigned int)vo->hal_phy_addr[ch][i], (unsigned int)vo->hal_buf_size[ch][i]);
                    vo->borrow_from_vbm[ch][i] = 1;
                    vo->borrow_size[ch][i] = vo->hal_buf_size[ch][i];
                }
                else
                {
                    pr_info("[vo] hal_phy_addr[%d][%d] PHOTO_VBM_BorrowMemChunk fail, trying to malloc from cma\n", ch, i);
                    vo->borrow_from_vbm[ch][i] = 0;
                    vo->borrow_size[ch][i] = 0;

                    vo->hal_phy_addr[ch][i] = (unsigned long)pli_malloc(reqSize, GFP_DCU2_FIRST);
                    vo->hal_buf_size[ch][i] = reqSize;
                    pr_info("[vo] pli_malloc buf: hal_phy_addr[%d][%d]:%x, size=%d\n", ch, i, (unsigned int)vo->hal_phy_addr[ch][i], reqSize);

                    if (vo->hal_phy_addr[ch][i] == INVALID_VAL)
                    {
                        pr_info("[vo] error: malloc memory fail\n");
                        photovo_free_hal_buffer(ch);
                        return -1;
                    }
                }
            }
        }

        return 0;

    }

    return -1;
}

/**
* @brief Open the video output module.
* @param   ch  [in]    channel port number to connect with display engine
* @param   def [in]    video panel type
* @return if success HAL_VO_STATE_OK, else one of error in HAL_VO_STATE_T.
**/
HAL_VO_STATE_T PHOTO_VO_Open(VO_OPEN_CMD_T *cmd)
{
    VO_DATA *vo = &photovodma_data[0];
    unsigned int buf_w, buf_h, buf_size;
    unsigned int ch = cmd->ch;
    HAL_VO_PANEL_TYPE panel_type = cmd->def;

    pr_err("[vo] PHOTO_VO_Open: ch %d, panel %d\n", ch, (int)panel_type);
    down(&VO_OpenCloseSemaphore);

    if (ch >= VODMA_DEV_NUM) {
        pr_err("[vo] error: ch %d doesn't exist\n", ch);
        up(&VO_OpenCloseSemaphore);
        return HAL_VO_STATE_NOT_AVAILABLE;
    }

    if (panel_type >= HAL_VO_PANEL_TYPE_MAX || panel_type <= HAL_VO_PANEL_TYPE_NONE) {
        pr_err("[vo] error: panel_type %d doesn't exist\n", panel_type);
        up(&VO_OpenCloseSemaphore);
        return HAL_VO_STATE_INVALID_PARAM;
    }

    if (vo->vo_state[ch] == VO_STATE_OPEN || vo->vo_state[ch] == VO_STATE_DISPLAY) {
        pr_err("[vo] error: open already\n");
        vo->panel_type[ch] = panel_type;
        vo->videoPlane[ch].Ddomain_Chroma_fmt = FMT_422;
        vo->alpha_blend[ch] = 255;
        vo->vo_state[ch] = VO_STATE_OPEN;
        vo->buf_idx[ch] = 0;
        // 2 hal_buffer
        cmd->buf_addr[0] = vo->hal_phy_addr[ch][0];
        cmd->buf_addr[1] = vo->hal_phy_addr[ch][1];
        cmd->buf_size= vo->hal_buf_size[ch][0];
        pr_err("[vo]error :setup photovo_malloc_hal_buffer: phy_addr[0]:%x,[1]:%x,size =%d\n", cmd->buf_addr[0],cmd->buf_addr[1],cmd->buf_size);
        up(&VO_OpenCloseSemaphore);
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

    /* yuv444sp */
    vo->buffer_size[ch][0] = (buf_size*3);
    vo->buffer_size[ch][1] = (buf_size*3);
    vo->buffer_size[ch][2] = 0;

    vo->phy_addr_y[ch][0] = vo->phy_addr[ch][0];
    vo->phy_addr_y[ch][1] = vo->phy_addr[ch][1];

    if (vo->phy_addr[ch][2]) {
        vo->phy_addr_y[ch][2] = vo->phy_addr[ch][2];
        vo->phy_addr_c[ch][2] = vo->phy_addr[ch][2] + buf_size;
    } else {
        vo->phy_addr_y[ch][2] = vo->phy_addr_y[ch][0];
        vo->phy_addr_c[ch][2] = vo->phy_addr_c[ch][0];
    }

    vo->panel_type[ch] = panel_type;

    vo->videoPlane[ch].Ddomain_Chroma_fmt = FMT_422;

    vo->alpha_blend[ch] = 255;

    vo->vo_state[ch] = VO_STATE_OPEN;

    vo->buf_idx[ch] = 0;
// 2 hal_buffer
    if (photovo_malloc_hal_buffer(ch) == 0) {
        cmd->buf_addr[0] = vo->hal_phy_addr[ch][0];
        cmd->buf_addr[1] = vo->hal_phy_addr[ch][1];
        cmd->buf_size= vo->hal_buf_size[ch][0];
        pr_err("[vo]photovo_malloc_hal_buffer: phy_addr[0]:%x,[1]:%x,size =%d\n", cmd->buf_addr[0],cmd->buf_addr[1],cmd->buf_size);
    } else {
        cmd->buf_addr[0] = 0;
        cmd->buf_addr[1] = 0;
        cmd->buf_size= 0;
    }
    up(&VO_OpenCloseSemaphore);
    return HAL_VO_STATE_OK;
}

/**
* @brief Close the video output module.
* @param   ch  [in]    channel port number to connect with display engine
* @return if success HAL_VO_STATE_OK, else one of error in HAL_VO_STATE_T.
**/
HAL_VO_STATE_T PHOTO_VO_Close(unsigned int ch)
{
    VO_DATA *vo = &photovodma_data[0];
    pr_err("[vo] PHOTO_VO_Close: ch %d\n", ch);
    down(&VO_OpenCloseSemaphore);

    if (ch >= VODMA_DEV_NUM) {
        pr_err("[vo] error: ch %d doesn't exist\n", ch);
        up(&VO_OpenCloseSemaphore);
        return HAL_VO_STATE_NOT_AVAILABLE;
    }

    if (vo->vo_state[ch] == VO_STATE_INIT) {
        pr_err("[vo] error: not open yet\n");
        up(&VO_OpenCloseSemaphore);
        return HAL_VO_STATE_OK;
    }
    set_vo_nosignal_flag(ch, 1);
    set_vo_EOS_flag(ch, 0);
    set_film_detect_done_flag(ch, 0);

    vo->vo_state[ch] = VO_STATE_INIT;

    //photovo_free_photo_buffer(ch);

    photovo_free_hal_buffer(ch);

    up(&VO_OpenCloseSemaphore);
    return HAL_VO_STATE_OK;
}

void PHOTO_VO_Force_Close(void)
{
    int ch = 0;
    VO_DATA *vo = &photovodma_data[0];

    if (vo->vo_state[ch] == VO_STATE_INIT) {
        //pr_err("[vo] error: not open yet\n");
        return;
    }
    pr_info("[vo] VO_Force_Close: ch %d\n", ch);
    PHOTO_VO_Close(0);

    return;
}

HAL_VO_STATE_T PHOTO_VO_DisplayPicture_Done()
{
    pr_info("[vo] PHOTO_VO_DisplayPicture_Done \n");
    //WaitFor_IVS2();
    //if(FALSE == WaitFor_IEN_STOP1())
    //    pr_err("[vo] PHOTO_VO_DisplayPicture_Done  WaitFor_IEN_STOP1 timeout\n");
    return 0;
}
