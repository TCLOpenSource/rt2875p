#include "rtk_vdec_dmabuf.h"
#include "rtk_vdec_ringbuf_update.h"

#include <linux/slab.h>
#include <linux/dma-buf.h>
#include <linux/uaccess.h>
#include <rpc_common.h>
#include <mach/rtk_platform.h>
#include <rtd_log/rtd_module_log.h>

static DEFINE_SEMAPHORE(DmabufLock);

struct rtk_vdec_dmabuf_mem_addr {
	unsigned long physical;
	unsigned long virtual;
	unsigned long non_cache_memory;
};

struct rtk_vdec_dmabuf_priv_data {
	int flags;
	union {
		struct {
			VDEC_SVPMEM_STRUCT svpmem;
		} cpb;
		struct {
			struct rtk_vdec_dmabuf_mem_addr addr;
			uint32_t size;
		} cma;
	};
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
	down(&DmabufLock);                                                    \
	dmabuf = dma_buf_export(&exp_info);                                    \
	if (IS_ERR_OR_NULL(dmabuf)) {                                          \
		ret = -EFAULT;                                                 \
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] buf export err\n", __LINE__);     \
		up(&DmabufLock);                                              \
		goto err;                                                      \
	}                                                                      \
	up(&DmabufLock);

#define RTK_VDEC_DMABUF_CHECK_ADDRESS(addr)                                    \
	if (!addr) {                                                           \
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] Didn't allocate buffer\n",        \
		       __LINE__);                                              \
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

static void rtk_vdec_dmabuf_release(struct dma_buf *dmabuf)
{
	uint32_t svpmem_addr = 0;
	unsigned long header_addr = 0, cma_addr = 0;
	struct rtk_vdec_dmabuf_priv_data *privdata = dmabuf->priv;
	down(&DmabufLock);
	if (privdata) {
		if (privdata->flags & FLAG_CPB) {
			svpmem_addr = privdata->cpb.svpmem.addr;
			if (svpmem_addr)
				rtkvdec_free_svpmem(svpmem_addr);
		} else {
			cma_addr = privdata->cma.addr.virtual;
			if (cma_addr)
				dvr_free((void *)cma_addr);
		}

		header_addr = privdata->header.addr.virtual;
		if (header_addr)
			dvr_free((void *)header_addr);

		kfree(dmabuf->priv);
		dmabuf->priv = NULL;
	}

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

static void *rtk_vdec_dmabuf_map(struct dma_buf *dmabuf, unsigned long page_num)
{
	return NULL;
}

static const struct dma_buf_ops rtk_dma_buf_ops = {
	.map_dma_buf = rtk_vdec_map_dmabuf,
	.unmap_dma_buf = rtk_vdec_unmap_dmabuf,
	.release = rtk_vdec_dmabuf_release,
//	.map_atomic = rtk_vdec_dmabuf_kmap,
	.map = rtk_vdec_dmabuf_map,
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
		rtd_pr_vdec_err("[%s %d]alloc memory fail\n", __func__, __LINE__);
		goto err;
	}

	addr->non_cache_memory = vir_addr_noncache;
	addr->physical = (unsigned long)dvr_to_phys((void *)addr->virtual);
	if (!addr->physical) {
		rtd_pr_vdec_err("[%s %d]alloc memory fail\n", __func__, __LINE__);
		goto err;
	}

	if ((addr->physical & 0xe0000000) != 0) {
		rtd_pr_vdec_err("[%s %d]get memory not between 0 ~ 512 MB\n", __func__,
		       __LINE__);
		goto err;
	}

	return 0;
err:
	if (addr->virtual)
		dvr_free((void *)addr->virtual);
	return ret;
}

static long
	rtk_vdec_dmabuf_alloc_cma(struct dma_buf **dmabuf,
				  struct rtk_vdec_dmabuf_allocation_data *data)
{
	long ret = -EFAULT;
	int fd = -1;
	unsigned long size = 0;
	struct rtk_vdec_dmabuf_priv_data *privdata = NULL;
	DEFINE_DMA_BUF_EXPORT_INFO(exp_info);

	if (!data) {
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] Error parameter\n", __LINE__);
		goto err;
	}

	privdata = kzalloc(sizeof(*privdata), GFP_KERNEL);
	if (!privdata) {
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] Allocate privdata failed\n",
		       __LINE__);
		goto err;
	}

	size = ALIGN_PAGE(data->len);
	ret = rtk_vdec_dmabuf_alloc_mem(size, &privdata->cma.addr);
	if (ret != 0)
		goto err;

	privdata->flags = data->flags;
	privdata->cma.size = size;
	RTK_VDEC_DMABUF_CREATE_BUF(privdata, *dmabuf, size);

	/*Export file descriptor to user*/
	fd = dma_buf_fd(*dmabuf, O_RDWR);
	if (fd < 0) {
		ret = -EFAULT;
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] Get dma_buf_fd error \n",
		       __LINE__);
		goto err;
	}

	data->handle = fd;

	return 0;
err:
	if (!IS_ERR_OR_NULL(*dmabuf)) {
		dma_buf_put(*dmabuf);
	} else if (privdata) {
		if (privdata->cma.addr.virtual) {
			dvr_free((void *)privdata->cma.addr.virtual);
		}
		kfree(privdata);
		privdata = NULL;
	}
	return ret;
}

static long rtk_vdec_dmabuf_alloc_header(uint32_t baseaddr, uint32_t size,
					 struct dma_buf *dmabuf)
{
	long ret = -EFAULT;
	RINGBUFFER_HEADER *header = NULL;
	struct rtk_vdec_dmabuf_priv_data *privdata = NULL;

	if (!baseaddr || !size || !dmabuf) {
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] Error parameter\n", __LINE__);
		goto err;
	}

	privdata = dmabuf->priv;
	if (!privdata) {
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] Error dmabuf val\n", __LINE__);
		goto err;
	}

	/*Allocate continuous memory for ring buffer header*/
	down(&DmabufLock);
	ret = rtk_vdec_dmabuf_alloc_mem(sizeof(*header),
					&privdata->header.addr);
	if (ret != 0) {
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] Allocate ring buffer failed\n",
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
	if (!IS_ERR_OR_NULL(dmabuf)) {
		dma_buf_put(dmabuf);
	} else if (privdata) {
		if (privdata->header.addr.virtual) {
			dvr_free((void *)privdata->header.addr.virtual);
		}
		kfree(privdata);
		privdata = NULL;
	}
	return ret;
}

static long rtk_vdec_dmabuf_alloc_cpb(
	struct dma_buf **dmabuf, VDEC_SVPMEM_STRUCT *svpmem,
	struct rtk_vdec_dmabuf_allocation_data *data, bool secured)
{
	long ret = -EFAULT;
	int fd = -1;
	struct rtk_vdec_dmabuf_priv_data *privdata = NULL;
	DEFINE_DMA_BUF_EXPORT_INFO(exp_info);

	if (!data || !svpmem) {
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] Error parameter\n", __LINE__);
		goto err;
	}

	privdata = kzalloc(sizeof(*privdata), GFP_KERNEL);
	if (!privdata) {
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] Allocate privdata failed\n",
		       __LINE__);
		goto err;
	}

	svpmem->addr = 0;
	svpmem->size = data->len;
	ret = rtkvdec_alloc_svpmem(svpmem, secured); //Allocate SVP.
	if (ret != 0)
		goto err;

	privdata->flags = data->flags;
	privdata->cpb.svpmem = *svpmem;
	RTK_VDEC_DMABUF_CREATE_BUF(privdata, *dmabuf, sizeof(*privdata));

	/*Export file descriptor to user*/
	fd = dma_buf_fd(*dmabuf, O_RDWR);
	if (fd < 0) {
		ret = -EFAULT;
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] dma_buf_fd error \n", __LINE__);
		goto err;
	}

	data->handle = fd;

	return 0;
err:
	if (!IS_ERR_OR_NULL(*dmabuf)) {
		dma_buf_put(*dmabuf);
	} else if (privdata) {
		if (svpmem->addr) {
			rtkvdec_free_svpmem(svpmem->addr);
		}
		kfree(privdata);
		privdata = NULL;
	}
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

	down(&DmabufLock);
	dmabuf = dma_buf_get(data.handle);
	if (IS_ERR_OR_NULL(dmabuf)) {
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] Get dma_buf by fd failed\n",
		       __LINE__);
		up(&DmabufLock);
		goto err;
	}

	privdata = dmabuf->priv;
	dma_buf_put(dmabuf);

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
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] Error info flags\n", __LINE__);
		up(&DmabufLock);
		goto err;
	}
	up(&DmabufLock);

	if (copy_to_user((void __user *)arg, &data, sizeof(data)))
		goto err;

	return 0;
err:
	return ret;
}

static long rtk_vdec_dmabuf_alloc_buf(struct file *filp, unsigned long arg)
{
	struct rtk_vdec_dmabuf_allocation_data data;
	VDEC_SVPMEM_STRUCT svpmem = SVPMEM_INIT;
	struct dma_buf *dmabuf = NULL;
	bool secured = false;
	long ret = -EFAULT;

	if (copy_from_user(&data, (const void __user *)arg, sizeof(data)))
		goto err;

	secured = data.flags & FLAG_PROTECTED;
	if (data.flags & FLAG_CPB) {
		/*Allocate dma buffer & SVP*/
		ret = rtk_vdec_dmabuf_alloc_cpb(&dmabuf, &svpmem, &data,
						secured);
		if (ret != 0)
			goto err;
	} else {
		if (secured)
			goto err;

		ret = rtk_vdec_dmabuf_alloc_cma(&dmabuf, &data);
		if (ret != 0)
			goto err;
	}

	if (data.flags & FLAG_RINGBUF_HEADER) {
		/*Create ring buffer header*/
		ret = rtk_vdec_dmabuf_alloc_header(svpmem.addr, svpmem.size,
						   dmabuf);
		if (ret != 0)
			goto err;
	}

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
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] Get dma_buf by fd failed\n",
		       __LINE__);
		goto err;
	}

	down(&DmabufLock);
	privdata = dmabuf->priv;
	if (!privdata) {
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] Error dmabuf val\n", __LINE__);
		up(&DmabufLock);
		goto err;
	}

	if (!(privdata->flags & FLAG_CPB) ||
	    !(privdata->flags & FLAG_RINGBUF_HEADER)) {
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] Get wrong privdata->flags\n",
		       __LINE__);
		up(&DmabufLock);
		goto err;
	}

	ret = rtk_vdec_dmabuf_ringbuf_start_update(
		(void *)privdata->header.addr.non_cache_memory, data.length,
		// currently use zero for offset, but this is wrong!!
		// need to add offset to struct rtk_vdec_dmabuf_securebuf_update.
		0,
		&data.map, privdata->cpb.svpmem.addr,
		privdata->cpb.svpmem.size);
	if (ret != 0) {
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] Start secure buffer updates failed",
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
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] Get dma_buf by fd failed\n",
		       __LINE__);
		goto err;
	}

	down(&DmabufLock);
	privdata = dmabuf->priv;
	if (!privdata) {
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] Error dmabuf val\n", __LINE__);
		up(&DmabufLock);
		goto err;
	}

	if (!(privdata->flags & FLAG_CPB) ||
	    !(privdata->flags & FLAG_RINGBUF_HEADER)) {
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] Get wrong privdata->flags\n",
		       __LINE__);
		up(&DmabufLock);
		goto err;
	}

	ret = rtk_vdec_dmabuf_ringbuf_commit_update(
		(void *)privdata->header.addr.non_cache_memory, data.length,
		privdata->cpb.svpmem.addr, privdata->cpb.svpmem.size);
	if (ret != 0) {
		rtd_pr_vdec_err("[rtk_vdec_dmabuf:%d] Commit secure buffer updates failed",
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
		ret = rtk_vdec_dmabuf_alloc_buf(filp, arg);
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