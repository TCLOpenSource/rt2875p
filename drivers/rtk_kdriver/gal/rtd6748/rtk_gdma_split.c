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
#include <rtk_kdriver/io.h>
#endif
#include <rtk_kdriver/rtk-kdrv-common.h>
#include <linux/uaccess.h>
#include <asm/barrier.h>
#include <asm/cacheflush.h>
#include <mach/platform.h>
#include <mach/system.h>
#include <rbus/sb2_reg.h>
#include <rbus/pif_reg.h>
#include <gal/rtk_gdma_split.h>
#include <gal/rtk_gdma_export_user.h>

extern int change_OSD_mode;
int GDMA_split_mode(int mode)
{	
	//The following setting should be set on bootcode panel side
	//rtd_outl(PIF_PIF_PN_CTRL_reg,rtd_inl(PIF_PIF_PN_CTRL_reg)|0xfff);
	return 1;
}

#ifdef CONFIG_RTK_KDRV_GDMA
static int __init early_parse_osd_split_mode(char *cmdline)
{
    static char *mode;
    mode = cmdline;
    if(cmdline != NULL){
        change_OSD_mode = (int)simple_strtoul(mode, &mode, 10);
		pr_debug("[GDMA] osd_split_mode=%d\n", change_OSD_mode);
    }
    return 0;
}



early_param("osd_split_mode", early_parse_osd_split_mode);
#else
void rtk_parse_osd_split_mode(void)
{
    char str[4]={0};
    char *mode;
    if(rtk_parse_commandline_equal("osd_split_mode", str, sizeof(str)) == 0)
    {
        pr_err("[GDMA] WARNING: can't get osd_split_mode from bootargs, use default value change_OSD_mode:%d\n",change_OSD_mode);
        return;
    }

    mode = str;
    change_OSD_mode = (int)simple_strtoul(mode, &mode, 10);
    pr_debug("[GDMA] osd_split_mode=%d\n", change_OSD_mode);
    return;
}
#endif

