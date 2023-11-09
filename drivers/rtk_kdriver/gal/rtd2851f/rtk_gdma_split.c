#include <generated/autoconf.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/dma-mapping.h>
#include <linux/pageremap.h>
#include <linux/kthread.h>
#include <linux/time.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/syscalls.h>
#ifdef CONFIG_ARM64
#include <io.h>
#endif
#include <linux/uaccess.h>
#include <asm/barrier.h>
#include <asm/cacheflush.h>
#include <mach/platform.h>
#include <mach/system.h>
#include <rbus/sb2_reg.h>
#include <rbus/pif_reg.h>
#include <gal/rtk_gdma_split.h>
#include <gal/rtk_gdma_export_user.h>


int GDMA_split_mode(int mode)
{	
	//The following setting should be set on bootcode panel side
	//rtd_outl(PIF_PIF_PN_CTRL_reg,rtd_inl(PIF_PIF_PN_CTRL_reg)|0xfff);
	return 1;
}
