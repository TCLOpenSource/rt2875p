#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
#include <trace/hooks/dmabuf.h>

static void rmm_dmabuf_heap_flag_skip(void *p,
	struct dma_heap *heap, size_t len,
	unsigned int fd_flags, unsigned int heap_flags, bool *skip)
{
	if (skip)
		*skip = true;
}

int rmm_vendor_hook_init(void)
{
	int ret = 0;

	ret = register_trace_android_vh_dmabuf_heap_flags_validation(rmm_dmabuf_heap_flag_skip, NULL);
	if (ret)
		goto err_out;

	return 0;

err_out:
	unregister_trace_android_vh_dmabuf_heap_flags_validation(rmm_dmabuf_heap_flag_skip, NULL);

	return -EINVAL;
}

void rmm_vendor_hook_exit(void)
{
	unregister_trace_android_vh_dmabuf_heap_flags_validation(rmm_dmabuf_heap_flag_skip, NULL);
}
#endif