#include <linux/version.h>
#include <linux/slab.h>
#include <linux/dma-buf.h>
#include <rtk_kdriver/rmm/rmm.h>
#include <rtk_kdriver/rmm/pageremap.h>
#include <rtd_log/rtd_module_log.h>

static DEFINE_SEMAPHORE(rtkaudio_dmabuf_lock);

static void rtkaudio_dmabuf_release(struct dma_buf *dmabuf)
{
	down(&rtkaudio_dmabuf_lock);
	if (dmabuf->priv) {
		rtd_pr_adsp_info("%s: free vaddr %lx\n", __func__, (unsigned long) dmabuf->priv);
		dvr_free(dmabuf->priv);
		dmabuf->priv = NULL;
	}
	up(&rtkaudio_dmabuf_lock);
}

static struct sg_table *
rtkaudio_map_dmabuf(struct dma_buf_attachment *attachment,
		    enum dma_data_direction direction)
{
	return NULL;
}

static void rtkaudio_unmap_dmabuf(struct dma_buf_attachment *attachment,
				  struct sg_table *table,
				  enum dma_data_direction direction)
{
}

static int rtkaudio_dmabuf_mmap(struct dma_buf *dmabuf,	struct vm_area_struct *vma)
{
	void *vaddr;
	uint32_t size;
	vaddr = dmabuf->priv;
	size = PAGE_ALIGN(dmabuf->size);

	rtd_pr_adsp_info("[%s] size %d, vaddr %lx\n", __func__, size, (unsigned long) vaddr);

	return remap_pfn_range(vma, vma->vm_start, (dvr_to_phys(vaddr) >> PAGE_SHIFT), size, vma->vm_page_prot);
}

static const struct dma_buf_ops rtkaudio_dma_buf_ops = {
	.map_dma_buf = rtkaudio_map_dmabuf,
	.unmap_dma_buf = rtkaudio_unmap_dmabuf,
	.release = rtkaudio_dmabuf_release,
	.mmap = rtkaudio_dmabuf_mmap,
};

struct dma_buf *rtkaudio_dmabuf_allocate(int size)
{
	struct dma_buf *buf = NULL;
	DEFINE_DMA_BUF_EXPORT_INFO(exp_info);
	void *vaddr = NULL;

	size = PAGE_ALIGN(size);
	vaddr = dvr_malloc_specific(size, GFP_DCU1);
	if (!vaddr) {
		rtd_pr_adsp_err("%s: vaddr malloc failed\n", __func__);
		return NULL;
	}

	rtd_pr_adsp_info("%s: malloc vaddr %lx\n", __func__, (unsigned long) vaddr);

	exp_info.ops = &rtkaudio_dma_buf_ops;
	exp_info.size = size;
	exp_info.flags = O_RDWR;
	exp_info.priv = vaddr;
	down(&rtkaudio_dmabuf_lock);
	buf = dma_buf_export(&exp_info);
	if (IS_ERR_OR_NULL(buf)) {
		rtd_pr_adsp_err("%s: dmabuf export err\n", __func__);
		up(&rtkaudio_dmabuf_lock);
		goto err;
	}
	up(&rtkaudio_dmabuf_lock);

	return buf;

err:
	if (vaddr) {
		dvr_free(vaddr);
	}
	return NULL;
}


int rtkaudio_dmabuf_fd_allocate(int size)
{
	int fd = -1;
	struct dma_buf *buf = NULL;

	buf = rtkaudio_dmabuf_allocate(size);

	if (!buf) {
		rtd_pr_adsp_err("%s: malloc dmabuf failed\n", __func__);
		return -1;
	}

	fd = dma_buf_fd(buf, O_RDWR);

	if (fd < 0) {
		dma_buf_put(buf);
		rtd_pr_adsp_err("%s: get dmabuf fd failed\n", __func__);
		return -EFAULT;
	}

	return fd;
}
EXPORT_SYMBOL_GPL(rtkaudio_dmabuf_fd_allocate);

void* rtkaudio_dmabuf_get_vaddr(int fd)
{
	void *vaddr = NULL;
	struct dma_buf *buf = NULL;

	down(&rtkaudio_dmabuf_lock);
	buf = dma_buf_get(fd);
	if (IS_ERR_OR_NULL(buf)) {
		rtd_pr_adsp_err("%s: get dmabuf fd failed\n", __func__);
		up(&rtkaudio_dmabuf_lock);
		return NULL;
	}

	vaddr = buf->priv;
	dma_buf_put(buf);
	up(&rtkaudio_dmabuf_lock);

	return vaddr;
}
EXPORT_SYMBOL_GPL(rtkaudio_dmabuf_get_vaddr);

int rtkaudio_ioctl_dmabuf_mmap(int fd, struct vm_area_struct *area)
{
	int ret = 0;
	struct dma_buf *buf = NULL;

	down(&rtkaudio_dmabuf_lock);
	buf = dma_buf_get(fd);
	if (IS_ERR_OR_NULL(buf)) {
		rtd_pr_adsp_err("%s: get dmabuf fd failed\n", __func__);
		up(&rtkaudio_dmabuf_lock);
		return -1;
	}

	ret = buf->ops->mmap(buf, area);
	dma_buf_put(buf);
	up(&rtkaudio_dmabuf_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(rtkaudio_ioctl_dmabuf_mmap);
