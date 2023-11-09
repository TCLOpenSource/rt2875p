#include "rtk_vdec_dmabuf.h"
#include "rtk_vdec_ringbuf_update.h"
#include "rtk_vdec_struct.h"

#include <linux/slab.h>
#include <linux/dma-buf.h>
#include <linux/dma-heap.h>
#include <linux/uaccess.h>
#include <rpc_common.h>
#include <mach/rtk_platform.h>
#include <rtd_log/rtd_module_log.h>
#include <rtk_vdec.h>

static DEFINE_SEMAPHORE(DmabufLock);

struct rtk_vdec_dma_heap {
	struct dma_heap *heap;
};

struct rtk_vdec_dma_heap_buffer {
	struct rtk_vdec_dma_heap *heap;
	struct list_head attachments;
	struct mutex lock;
	unsigned long len;
	struct sg_table *sg_table;
	void *vaddr;
};

struct dma_heap_attachment {
	struct device *dev;
	struct sg_table *table;
	struct list_head list;
};

struct rtk_vdec_dmabuf_mem_addr {
	unsigned long physical;
	unsigned long virtual;
	unsigned long non_cache_memory;
};

struct rtk_vdec_dmabuf_priv_data {
	int magic;
	int flags;
	struct {
		VDEC_SVPMEM_STRUCT svpmem;
	} cpb;
	struct {
		struct rtk_vdec_dmabuf_mem_addr addr;
		uint32_t size;
	} cma;
	struct {
		struct rtk_vdec_dmabuf_mem_addr addr;
		uint32_t size;
	} header;
};

#define RTK_VDEC_DMABUF_CREATE_BUF(privdata, dmabuf, length)                   \
	exp_info.ops = &rtk_dma_buf_ops;                                       \
	exp_info.size = length;                                                \
	exp_info.flags = O_RDWR;                                               \
	exp_info.priv = privdata;                                              \
	down(&DmabufLock);                                                     \
	dmabuf = dma_buf_export(&exp_info);                                    \
	if (IS_ERR_OR_NULL(dmabuf)) {                                          \
		ret = -EFAULT;                                                 \
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] buf export err\n",       \
				__LINE__);                                     \
		up(&DmabufLock);                                               \
		goto err;                                                      \
	}                                                                      \
	up(&DmabufLock);

#define RTK_VDEC_DMABUF_CHECK_ADDRESS(addr)                                    \
	if (!addr) {                                                           \
		rtd_pr_vdec_err(                                               \
			"[rtk_vdec_dmabuf:%d] Didn't allocate buffer\n",       \
			__LINE__);                                             \
		goto err;                                                      \
	}

#define ALIGN_PAGE(length)                                                     \
	((PAGE_SIZE - 1) & length) ?                                           \
		((length + PAGE_SIZE) & ~(PAGE_SIZE - 1)) :                    \
		length

#define SVPMEM_INIT                                                            \
	{                                                                      \
		0, 0, 0                                                        \
	}

struct dmabuf_container {
	struct rtk_vdec_dmabuf_priv_data *priv;
	uint32_t size;
	uint32_t counter;
};

#define MAX_DMABUF_COUNT 2
#define MAX_DMABUF_TOTAL_SIZE RTKVDEC_SVPMEM_SIZE_32M

static struct dmabuf_container buf_array[MAX_DMABUF_COUNT];

static const int kMagic = '\xc2svp';

static int validate_dmabuf_data(struct rtk_vdec_dmabuf_priv_data *privdata) {
	if (!privdata) {
		return -1;
	}
	return (privdata->magic == kMagic) ? 0 : -1;
}

static void release_privdata(struct rtk_vdec_dmabuf_priv_data *privdata)
{
	uint32_t svpmem_addr = 0;
	unsigned long header_addr = 0, cma_addr = 0;
	VDEC_SVPMEM_STRUCT svpmem = SVPMEM_INIT;
	if (privdata->flags & FLAG_CPB) {
		svpmem_addr = privdata->cpb.svpmem.addr;
		if (svpmem_addr) {
			rtd_pr_vdec_info(
				"[rtk_vdec_dmabuf:%d] free mem size=%d\n",
				__LINE__, privdata->cpb.svpmem.size);
			svpmem.addr = svpmem_addr;
			rtkvdec_svp_free(&svpmem);
		}
	} else {
		cma_addr = privdata->cma.addr.virtual;
		if (cma_addr)
			dvr_free((void *)cma_addr);
	}

	header_addr = privdata->header.addr.virtual;
	if (header_addr)
		dvr_free((void *)header_addr);

	kfree(privdata);
}

static void rtk_vdec_dmabuf_release(struct dma_buf *dmabuf)
{
	struct rtk_vdec_dmabuf_priv_data *privdata = dmabuf->priv;
	uint32_t i = 0;
	down(&DmabufLock);
	for (i = 0; i < MAX_DMABUF_COUNT; i++) {
		if (!(privdata->flags & FLAG_IOCTL)) {
			if (privdata && (privdata == buf_array[i].priv)) {
				if (privdata->flags & FLAG_CPB) {
					rtd_pr_vdec_info(
					"[rtk_vdec_dmabuf:%d] release size=%d\n",
					__LINE__, privdata->cpb.svpmem.size);
				}
				buf_array[i].counter--;
				if (buf_array[i].counter == 0) {
					release_privdata(privdata);
					buf_array[i].priv = NULL;
				}
				break;
			}
		} else {
			if (privdata && (privdata == buf_array[i].priv)) {
				release_privdata(privdata);
				buf_array[i].priv = NULL;
				break;
			}
		}
	}
	dmabuf->priv = NULL;
	up(&DmabufLock);
}

static struct sg_table *
rtk_vdec_map_dmabuf(struct dma_buf_attachment *attachment,
		    enum dma_data_direction direction)
{
	return NULL;
}

static void rtk_vdec_unmap_dmabuf(struct dma_buf_attachment *attachment,
				  struct sg_table *table,
				  enum dma_data_direction direction)
{
}

static int rtk_vdec_dmabuf_mmap(struct dma_buf *dmabuf,
				struct vm_area_struct *vma)
{
	struct rtk_vdec_dmabuf_priv_data *privdata = NULL;
	unsigned long addr;
	uint32_t size;
	privdata = dmabuf->priv;
	addr = privdata->cma.addr.virtual;
	size = privdata->cma.size;

	return remap_pfn_range(vma, vma->vm_start, virt_to_pfn((void *)addr),
			       size, vma->vm_page_prot);
}

/*
static void *rtk_vdec_dmabuf_kmap(struct dma_buf *dmabuf, unsigned long offset)
{
	return NULL;
}
*/

// static void *rtk_vdec_dmabuf_map(struct dma_buf *dmabuf, unsigned long page_num)
// {
// 	return NULL;
// }

static const struct dma_buf_ops rtk_dma_buf_ops = {
	.map_dma_buf = rtk_vdec_map_dmabuf,
	.unmap_dma_buf = rtk_vdec_unmap_dmabuf,
	.release = rtk_vdec_dmabuf_release,
	//	.map_atomic = rtk_vdec_dmabuf_kmap,
	.mmap = rtk_vdec_dmabuf_mmap,
};

static long rtk_vdec_dmabuf_alloc_mem(uint32_t size,
				      struct rtk_vdec_dmabuf_mem_addr *addr)
{
	long ret = -EFAULT;
	unsigned long vir_addr_noncache = 0;

	addr->virtual = (unsigned long)dvr_malloc_uncached_specific(
		size, GFP_DCU1, (void **)&vir_addr_noncache);
	if (!addr->virtual) {
		rtd_pr_vdec_err("[%s %d]alloc memory fail\n", __func__,
				__LINE__);
		goto err;
	}

	addr->non_cache_memory = vir_addr_noncache;
	addr->physical = (unsigned long)dvr_to_phys((void *)addr->virtual);
	if (!addr->physical) {
		rtd_pr_vdec_err("[%s %d]alloc memory fail\n", __func__,
				__LINE__);
		goto err;
	}

	if ((addr->physical & 0xe0000000) != 0) {
		rtd_pr_vdec_err("[%s %d]get memory not between 0 ~ 512 MB\n",
				__func__, __LINE__);
		goto err;
	}

	return 0;
err:
	if (addr->virtual) {
		dvr_free((void *)addr->virtual);
		addr->virtual = 0;
	}
	return ret;
}

static long
rtk_vdec_dmabuf_alloc_cma(struct rtk_vdec_dmabuf_allocation_data *data,
			  struct rtk_vdec_dmabuf_priv_data *privdata)
{
	long ret = -EFAULT;
	// int fd = -1;
	unsigned long size = 0;
	// struct rtk_vdec_dmabuf_priv_data *privdata = NULL;
	// DEFINE_DMA_BUF_EXPORT_INFO(exp_info);

	if (!data || !privdata) {
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] Error parameter\n",
				__LINE__);
		goto err;
	}

	size = ALIGN_PAGE(data->len);
	ret = rtk_vdec_dmabuf_alloc_mem(size, &privdata->cma.addr);
	if (ret != 0)
		goto err;

	privdata->flags = data->flags;
	privdata->cma.size = size;

	// RTK_VDEC_DMABUF_CREATE_BUF(privdata, *dmabuf, size);

	// /*Export file descriptor to user*/
	// fd = dma_buf_fd(*dmabuf, O_RDWR);
	// if (fd < 0) {
	// 	ret = -EFAULT;
	// 	rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] Get dma_buf_fd error \n",
	// 	       __LINE__);
	// 	goto err;
	// }

	// data->handle = fd;

	return 0;
err:
	return ret;
}

static long
rtk_vdec_dmabuf_alloc_header(struct rtk_vdec_dmabuf_priv_data *privdata)
{
	long ret = -EFAULT;
	RINGBUFFER_HEADER *header = NULL;
	uint32_t baseaddr = 0, size = 0;

	if (!privdata) {
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] Error parameter\n",
				__LINE__);
		goto err;
	}
	baseaddr = privdata->cpb.svpmem.addr;
	size = privdata->cpb.svpmem.size;
	if (!baseaddr || !size) {
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] Error parameter\n",
				__LINE__);
		goto err;
	}

	/*Allocate continuous memory for ring buffer header*/
	down(&DmabufLock);
	ret = rtk_vdec_dmabuf_alloc_mem(sizeof(*header),
					&privdata->header.addr);
	if (ret != 0) {
		rtd_pr_vdec_err(
			"[rtk_vdec_dmabuf:%d] Allocate ring buffer failed\n",
			__LINE__);
		up(&DmabufLock);
		goto err;
	}

	privdata->header.size = sizeof(*header);
	header = (RINGBUFFER_HEADER *)privdata->header.addr.non_cache_memory;
	/*Fill ring buffer header data*/
	header->size = htonl(size);
	header->numOfReadPtr = htonl(1);
	header->beginAddr = htonl(baseaddr);
	header->writePtr = htonl(baseaddr);
	header->readPtr[0] = htonl(baseaddr);
	header->readPtr[1] = htonl(baseaddr);
	header->readPtr[2] = htonl(baseaddr);
	header->readPtr[3] = htonl(baseaddr);
	header->reserve3 = htonl(baseaddr);
	up(&DmabufLock);

	return 0;
err:
	// if (!IS_ERR_OR_NULL(dmabuf)) {
	// 	dma_buf_put(dmabuf);
	// }
	return ret;
}

static long
rtk_vdec_dmabuf_alloc_cpb(struct rtk_vdec_dmabuf_allocation_data *data,
			  struct rtk_vdec_dmabuf_priv_data *privdata,
			  bool secured)
{
	long ret = -EFAULT;
	// int fd = -1;
	// DEFINE_DMA_BUF_EXPORT_INFO(exp_info);

	if (!data || !privdata) {
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] Error parameter\n",
				__LINE__);
		goto err;
	}
	privdata->cpb.svpmem.addr = 0;
	privdata->cpb.svpmem.size = data->len;
	ret = rtkvdec_svp_alloc(&privdata->cpb.svpmem,
				secured == true ? VDEC_IOC_ALLOCSVPMP :
						  0); //Allocate SVP.
	if (ret != 0)
		goto err;
	privdata->flags = data->flags;

	// RTK_VDEC_DMABUF_CREATE_BUF(privdata, *dmabuf, sizeof(*privdata));

	// /*Export file descriptor to user*/
	// fd = dma_buf_fd(*dmabuf, O_RDWR);
	// if (fd < 0) {
	// 	ret = -EFAULT;
	// 	rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] dma_buf_fd error \n", __LINE__);
	// 	goto err;
	// }

	// data->handle = fd;

	return 0;
err:
	// if (!IS_ERR_OR_NULL(*dmabuf))
	// 	dma_buf_put(*dmabuf);

	return ret;
}

static long rtk_vdec_dmabuf_get_info(struct file *filp, unsigned long arg)
{
	struct rtk_vdec_dmabuf_info_data data;
	long ret = -EFAULT;
	struct dma_buf *dmabuf = NULL;
	struct rtk_vdec_dmabuf_priv_data *privdata = NULL;

	if (copy_from_user(&data, (const void __user *)arg, sizeof(data)))
		goto err;

	dmabuf = dma_buf_get(data.handle);
	if (IS_ERR_OR_NULL(dmabuf)) {
		rtd_pr_vdec_err(
			"[rtk_vdec_dmabuf:%d] Get dma_buf by fd failed\n",
			__LINE__);
		goto err;
	}

	down(&DmabufLock);
	privdata = dmabuf->priv;
	if (!privdata) {
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] Error dmabuf val\n",
				__LINE__);
		up(&DmabufLock);
		goto err;
	}
	if (validate_dmabuf_data(privdata) != 0) {
		rtd_pr_vdec_err(
			"[rtk_vdec_dmabuf:%d] validate dma_buf failed\n",
			__LINE__);
		up(&DmabufLock);
		goto err;
	}

	switch (data.flags) {
	case FLAG_CPB:
		RTK_VDEC_DMABUF_CHECK_ADDRESS(privdata->cpb.svpmem.addr);
		data.phyaddr = privdata->cpb.svpmem.addr;
		data.len = privdata->cpb.svpmem.size;
		break;
	case FLAG_RINGBUF_HEADER:
		RTK_VDEC_DMABUF_CHECK_ADDRESS(privdata->header.addr.physical);
		data.phyaddr = privdata->header.addr.physical;
		data.len = privdata->header.size;
		break;
	case FLAG_NONE:
		RTK_VDEC_DMABUF_CHECK_ADDRESS(privdata->cma.addr.physical);
		data.phyaddr = privdata->cma.addr.physical;
		data.len = privdata->cma.size;
		break;
	default:
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] Error info flags\n",
				__LINE__);
		up(&DmabufLock);
		goto err;
	}
	up(&DmabufLock);

	if (copy_to_user((void __user *)arg, &data, sizeof(data)))
		goto err;

	dma_buf_put(dmabuf);
	return 0;
err:
	if (!IS_ERR_OR_NULL(dmabuf))
		dma_buf_put(dmabuf);
	return ret;
}

static struct dma_buf *
rtk_vdec_dmabuf_alloc_buf(struct rtk_vdec_dmabuf_allocation_data *data)
{
	long ret = -EFAULT;
	// unsigned long size = 0;
	struct dma_buf *dmabuf;
	struct rtk_vdec_dmabuf_priv_data *privdata = NULL;
	DEFINE_DMA_BUF_EXPORT_INFO(exp_info);
	bool secured = false;

	if (!data) {
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] Error parameter\n",
				__LINE__);
		goto err;
	}

	secured = data->flags & FLAG_PROTECTED;
	privdata = kzalloc(sizeof(*privdata), GFP_KERNEL);
	if (!privdata) {
		rtd_pr_vdec_err(
			"[rtk_vdec_dmabuf:%d] Allocate privdata failed\n",
			__LINE__);
		goto err;
	}
	privdata->magic = kMagic;

	if (data->flags & FLAG_CPB) {
		/*Allocate dma buffer & SVP*/
		ret = rtk_vdec_dmabuf_alloc_cpb(data, privdata, secured);
		if (ret != 0)
			goto err;
	} else {
		if (secured)
			goto err;

		ret = rtk_vdec_dmabuf_alloc_cma(data, privdata);
		if (ret != 0)
			goto err;
	}

	if (data->flags & FLAG_RINGBUF_HEADER) {
		/*Create ring buffer header*/
		ret = rtk_vdec_dmabuf_alloc_header(privdata);
		if (ret != 0)
			goto err;
	}

	exp_info.ops = &rtk_dma_buf_ops;
	if (privdata->cpb.svpmem.addr) {
		exp_info.size = privdata->cpb.svpmem.size;
	} else {
		exp_info.size = privdata->cma.size;
	}
	exp_info.flags = O_RDWR;
	exp_info.priv = privdata;
	down(&DmabufLock);
	dmabuf = dma_buf_export(&exp_info);
	if (IS_ERR_OR_NULL(dmabuf)) {
		ret = -EFAULT;
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] buf export err\n",
				__LINE__);
		up(&DmabufLock);
		goto err;
	}
	up(&DmabufLock);

	return dmabuf;
err:
	if (privdata) {
		if (privdata->cma.addr.virtual) {
			dvr_free((void *)privdata->cma.addr.virtual);
		}
		if (privdata->cpb.svpmem.addr) {
			rtkvdec_svp_free(&privdata->cpb.svpmem);
		}
		if (privdata->header.addr.virtual) {
			dvr_free((void *)privdata->header.addr.virtual);
		}
		kfree(privdata);
		privdata = NULL;
	}
	return ERR_PTR(ret);
}

static long rtk_vdec_dmabuf_ioctl_alloc_buf(struct file *filp,
					    unsigned long arg)
{
	struct rtk_vdec_dmabuf_allocation_data data;
	struct dma_buf *dmabuf = NULL;
	int fd = -1;
	// bool secured = false;
	long ret = -EFAULT;
	(void)filp;

	if (copy_from_user(&data, (const void __user *)arg, sizeof(data)))
		goto err;
	data.flags |= FLAG_IOCTL;
	dmabuf = rtk_vdec_dmabuf_alloc_buf(&data);

	if (IS_ERR(dmabuf)) {
		ret = PTR_ERR(dmabuf);
		goto err;
	}

	/*Export file descriptor to user*/
	fd = dma_buf_fd(dmabuf, O_RDWR);
	if (fd < 0) {
		ret = -EFAULT;
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] dma_buf_fd error \n",
				__LINE__);
		goto err;
	}

	data.handle = fd;

	if (copy_to_user((void __user *)arg, &data, sizeof(data))) {
		ret = -EFAULT;
		goto err;
	}

	return 0;
err:
	return ret;
}

static long rtk_vdec_dmabuf_Start_Ring_Buffer_Updates(struct file *filp,
						      unsigned long arg)
{
	struct rtk_vdec_dmabuf_securebuf_update data;
	struct dma_buf *dmabuf = NULL;
	struct rtk_vdec_dmabuf_priv_data *privdata = NULL;
	long ret = -EFAULT;

	if (copy_from_user(&data, (const void __user *)arg, sizeof(data)))
		goto err;

	dmabuf = dma_buf_get(data.handle);
	if (IS_ERR_OR_NULL(dmabuf)) {
		rtd_pr_vdec_err(
			"[rtk_vdec_dmabuf:%d] Get dma_buf by fd=%d failed\n",
			__LINE__, data.handle);
		goto err;
	}

	down(&DmabufLock);
	privdata = dmabuf->priv;
	if (!privdata) {
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] Error dmabuf val\n",
				__LINE__);
		up(&DmabufLock);
		goto err;
	}

	if (validate_dmabuf_data(privdata) != 0) {
		rtd_pr_vdec_err(
			"[rtk_vdec_dmabuf:%d] validate dma_buf failed\n",
			__LINE__);
		up(&DmabufLock);
		goto err;
	}

	if (!(privdata->flags & FLAG_CPB) ||
	    !(privdata->flags & FLAG_RINGBUF_HEADER)) {
		rtd_pr_vdec_err(
			"[rtk_vdec_dmabuf:%d] Get wrong privdata->flags\n",
			__LINE__);
		up(&DmabufLock);
		goto err;
	}

	ret = rtk_vdec_dmabuf_ringbuf_start_update(
		(void *)privdata->header.addr.non_cache_memory, data.length,
		data.offset ,&data.map, privdata->cpb.svpmem.addr,
		privdata->cpb.svpmem.size);
	if (ret != 0) {
		rtd_pr_vdec_err(
			"[rtk_vdec_dmabuf:%d] Start secure buffer updates failed",
			__LINE__);
		up(&DmabufLock);
		goto err;
	}
	up(&DmabufLock);

	if (copy_to_user((void __user *)arg, &data, sizeof(data))) {
		ret = -EFAULT;
		goto err;
	}

	dma_buf_put(dmabuf);
	return 0;
err:
	if (!IS_ERR_OR_NULL(dmabuf))
		dma_buf_put(dmabuf);
	return ret;
}

static long rtk_vdec_dmabuf_Commit_Ring_Buffer_Updates(struct file *filp,
						       unsigned long arg)
{
	struct rtk_vdec_dmabuf_securebuf_update data;
	struct dma_buf *dmabuf = NULL;
	struct rtk_vdec_dmabuf_priv_data *privdata = NULL;
	long ret = -EFAULT;

	if (copy_from_user(&data, (const void __user *)arg, sizeof(data)))
		goto err;

	dmabuf = dma_buf_get(data.handle);
	if (IS_ERR_OR_NULL(dmabuf)) {
		rtd_pr_vdec_err(
			"[rtk_vdec_dmabuf:%d] Get dma_buf by fd=%d failed\n",
			__LINE__, data.handle);
		goto err;
	}

	down(&DmabufLock);
	privdata = dmabuf->priv;
	if (!privdata) {
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] Error dmabuf val\n",
				__LINE__);
		up(&DmabufLock);
		goto err;
	}

	if (validate_dmabuf_data(privdata) != 0) {
		rtd_pr_vdec_err(
			"[rtk_vdec_dmabuf:%d] validate dma_buf failed\n",
			__LINE__);
		up(&DmabufLock);
		goto err;
	}

	if (!(privdata->flags & FLAG_CPB) ||
	    !(privdata->flags & FLAG_RINGBUF_HEADER)) {
		rtd_pr_vdec_err(
			"[rtk_vdec_dmabuf:%d] Get wrong privdata->flags\n",
			__LINE__);
		up(&DmabufLock);
		goto err;
	}

	ret = rtk_vdec_dmabuf_ringbuf_commit_update(
		(void *)privdata->header.addr.non_cache_memory, data.length,
		privdata->cpb.svpmem.addr, privdata->cpb.svpmem.size);
	if (ret != 0) {
		rtd_pr_vdec_err(
			"[rtk_vdec_dmabuf:%d] Commit secure buffer updates failed",
			__LINE__);
		up(&DmabufLock);
		goto err;
	}

	up(&DmabufLock);
	dma_buf_put(dmabuf);
	return 0;
err:
	if (!IS_ERR_OR_NULL(dmabuf))
		dma_buf_put(dmabuf);
	return ret;
}

long rtk_vdec_dmabuf_ioctl(struct file *filp, unsigned int cmd,
			   unsigned long arg)
{
	long ret = -EFAULT;

	switch (cmd) {
	case VDEC_IOC_SVP_DMABUF_ALLOCATE:
		ret = rtk_vdec_dmabuf_ioctl_alloc_buf(filp, arg);
		break;
	case VDEC_IOC_SVP_DMABUF_GET_INFO:
		ret = rtk_vdec_dmabuf_get_info(filp, arg);
		break;
	case VDEC_IOC_SVP_DMABUF_START_RINGBUF_UPDATE:
		ret = rtk_vdec_dmabuf_Start_Ring_Buffer_Updates(filp, arg);
		break;
	case VDEC_IOC_SVP_DMABUF_COMMIT_RINGBUF_UPDATE:
		ret = rtk_vdec_dmabuf_Commit_Ring_Buffer_Updates(filp, arg);
		break;
	default:
		break;
	}
	return ret;
}

////////////////////////////////////////////////////////////////////////////////

static struct dma_buf *rtk_vdec_dma_heap_allocate(struct dma_heap *heap,
						  unsigned long len,
						  unsigned long fd_flags,
						  unsigned long heap_flags)
{
	struct rtk_vdec_dma_heap *rtk_vdec_dma_heap =
		dma_heap_get_drvdata(heap);
	// struct rtk_vdec_dma_heap_buffer *buffer;
	struct rtk_vdec_dmabuf_allocation_data data;
	DEFINE_DMA_BUF_EXPORT_INFO(exp_info);
	struct dma_buf *dmabuf = NULL;
	int ret = -EFAULT;
	int32_t index = -1;
	uint32_t total_size = 0, i = 0;
	(void)rtk_vdec_dma_heap;
	data.len = len * 2;

	if (data.len <= RTKVDEC_SVPMEM_SIZE_4M) {
		data.len = RTKVDEC_SVPMEM_SIZE_4M;
	} else if (data.len <= RTKVDEC_SVPMEM_SIZE_8M) {
		data.len = RTKVDEC_SVPMEM_SIZE_8M;
	} else if (data.len <= RTKVDEC_SVPMEM_SIZE_12M) {
		data.len = RTKVDEC_SVPMEM_SIZE_12M;
	} else if (data.len <= RTKVDEC_SVPMEM_SIZE_16M) {
		data.len = RTKVDEC_SVPMEM_SIZE_16M;
	} else if (data.len <= RTKVDEC_SVPMEM_SIZE_32M) {
		data.len = RTKVDEC_SVPMEM_SIZE_32M;
	} else {
		rtd_pr_vdec_err(
			"[rtk_vdec_dmabuf:%d] allocate size too large=%d failed\n",
			__LINE__, data.len);
		goto free_other;
	}

	rtd_pr_vdec_info(
			"[rtk_vdec_dmabuf:%d] request size=%d x 2 -> allocate size=%d\n",
			__LINE__, len, data.len);

	data.flags = FLAG_PROTECTED | FLAG_CPB | FLAG_RINGBUF_HEADER;
	for (i = 0; i < MAX_DMABUF_COUNT; i++) {
		if(buf_array[i].priv != NULL) {
			total_size += buf_array[i].size;
			if (buf_array[i].size == data.len) {
				index = i;
			}
		} else if (buf_array[i].priv == NULL && index == -1) {
			index = i;
		}
	}
	if (buf_array[index].priv == NULL) {
		if (total_size + data.len > MAX_DMABUF_TOTAL_SIZE) {
			rtd_pr_vdec_err(
				"[rtk_vdec_dmabuf:%d] total allocate size too large=%d > "
				"max total svp size = %d failed\n",
				__LINE__, total_size + data.len, MAX_DMABUF_TOTAL_SIZE);
			goto free_other;
		}
		dmabuf = rtk_vdec_dmabuf_alloc_buf(&data);
		buf_array[index].priv = dmabuf->priv;
		buf_array[index].size = dmabuf->size;
		buf_array[index].counter++;
	} else {
		exp_info.ops = &rtk_dma_buf_ops;
		exp_info.size = buf_array[index].size;
		exp_info.flags = O_RDWR;
		exp_info.priv = buf_array[index].priv;
		dmabuf = dma_buf_export(&exp_info);
		buf_array[index].counter++;
	}

	// INIT_LIST_HEAD(&buffer->attachments);
	// mutex_init(&buffer->lock);
	// buffer->len = len;

	/* Alloc / Initalize custom buffer structure data here */

	/* Fill in buffer->sg_table here */

	/* create the dmabuf */
	// exp_info.ops = &rtk_vdec_dma_heap_buf_ops;
	// exp_info.size = buffer->len;
	// exp_info.flags = fd_flags;
	// exp_info.priv = buffer;
	// dmabuf = dma_buf_export(&exp_info);
	if (IS_ERR_OR_NULL(dmabuf)) {
		// ret = PTR_ERR(dmabuf);
		goto free_other;
	}

	return dmabuf;

free_other:
	/*Free custom buffer structure data here */
	// free_buffer:

	return ERR_PTR(ret);
}

static struct dma_heap_ops rtk_vdec_dma_heap_ops = {
	.allocate = rtk_vdec_dma_heap_allocate,
};

int rtk_vdec_dma_heap_create(void)
{
	struct rtk_vdec_dma_heap *rtk_vdec_dma_heap;
	struct dma_heap_export_info exp_info;

	rtk_vdec_dma_heap = kzalloc(sizeof(*rtk_vdec_dma_heap), GFP_KERNEL);
	if (!rtk_vdec_dma_heap)
		return -ENOMEM;

	/* Initialize any custom rtk_vdec_dma_heap structure values here */

	exp_info.name = "system-secure-rtkvdec";
	exp_info.ops = &rtk_vdec_dma_heap_ops;
	exp_info.priv = rtk_vdec_dma_heap;
	rtk_vdec_dma_heap->heap = dma_heap_add(&exp_info);
	if (IS_ERR(rtk_vdec_dma_heap->heap)) {
		int ret = PTR_ERR(rtk_vdec_dma_heap->heap);
		kfree(rtk_vdec_dma_heap);
		return ret;
	}
	memset(&buf_array, 0, sizeof(struct dmabuf_container) * MAX_DMABUF_COUNT);

	return 0;
}