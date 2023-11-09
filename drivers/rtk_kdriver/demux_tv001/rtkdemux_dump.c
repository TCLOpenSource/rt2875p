#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <asm/cacheflush.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/pageremap.h>
#include "rtkdemux.h"

#include <linux/version.h>
#include <linux/file.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <linux/uaccess.h>

/* use IS_ERR(x) to check the (struct file *) */
void DUMP_CreateFile(const char *filename, struct file **handle)
{
#if IS_ENABLED(CONFIG_RTK_FEATURE_FOR_GKI)
	*handle = filp_open(filename, O_RDWR | O_CREAT | O_TRUNC, 0777);
#else
	*handle = NULL;
#endif
}

void DUMP_CloseFile(struct file *handle)
{
#if IS_ENABLED(CONFIG_RTK_FEATURE_FOR_GKI)
	filp_close(handle, NULL);
#endif
}

int DUMP_WriteFile(struct file *handle, char *buffer, unsigned int size, loff_t *pOffset)
{
	int ret = 0;
#if IS_ENABLED(CONFIG_RTK_FEATURE_FOR_GKI)
	if (handle && pOffset) {
		kernel_write(handle, buffer, size, pOffset);
	}
#endif
	return ret;
}

