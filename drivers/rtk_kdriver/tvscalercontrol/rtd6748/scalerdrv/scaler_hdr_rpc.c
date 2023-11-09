#if defined(CONFIG_ENABLE_HDR10_HDMI_AUTO_DETECT)|| defined(CONFIG_ENABLE_DOLBY_VISION_HDMI_AUTO_DETECT)
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/slab.h>     /* kmalloc()      */
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/buffer_head.h>
#include <linux/file.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/random.h>
#include <linux/pageremap.h>
#include <linux/kthread.h>  /* for threads */
#include <linux/freezer.h>
#include <linux/vmalloc.h>
#include <asm/barrier.h> /*dsb()*/
#include <asm/cacheflush.h>

#include <rtk_kdriver/RPCDriver.h>
#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <VideoRPC_System.h>
#else
#include <rpc/VideoRPC_System.h>
#endif

#include <tvscalercontrol/scalerdrv/scaler_hdr_rpc.h>
#include <rtd_log/rtd_module_log.h>

int Scaler_HDR_vrpc_DolbyVision_Hdmi_SetDetectEnable(VIDEO_RPC_DOLBY_VISION_HDMI_SET_DETECT_ENABLE *pSetDetectInfo)
{
	unsigned char ret=0;
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	unsigned long result =0;
#endif
	VIDEO_RPC_DOLBY_VISION_HDMI_SET_DETECT_ENABLE *info;
	unsigned long  vir_addr, vir_addr_noncache;
	unsigned int  phy_addr;

	//rtd_pr_hdr_debug("%s\n", __FUNCTION__);

	vir_addr = (unsigned long)dvr_malloc_uncached_specific(sizeof(VIDEO_RPC_DOLBY_VISION_HDMI_SET_DETECT_ENABLE), GFP_DCU1, (void**)&vir_addr_noncache);
	phy_addr = (unsigned int)dvr_to_phys((void*)vir_addr);

	if(!vir_addr || !vir_addr_noncache || !phy_addr){
		rtd_pr_hdr_debug("[ERROR] NULL PTR %lx/%lx/%x\n", vir_addr, vir_addr_noncache, phy_addr);
		return 0;
	}
	//rtd_pr_hdr_debug("[DolbyVision] Addr Vir/NonC/Phy=%x/%x/%x\n", vir_addr, vir_addr_noncache, phy_addr);

	info =  (VIDEO_RPC_DOLBY_VISION_HDMI_SET_DETECT_ENABLE *)vir_addr_noncache;
	memcpy(info, pSetDetectInfo, sizeof(VIDEO_RPC_DOLBY_VISION_HDMI_SET_DETECT_ENABLE));

#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	if (RPC_FAIL==send_rpc_command(RPC_VIDEO, VIDEO_RPC_DOLBY_VISION_HDMI_Set_Detect_Enable,phy_addr, 0, &result))
	{
		rtd_pr_hdr_debug("RPC fail!!\n");
		ret=1;
	}
#endif
	dvr_free((void *)vir_addr);

	//rtd_pr_hdr_debug("res=%x\n", (res? *res: 0));

    return ret;
}



int Scaler_HDR_vrpc_DolbyVision_Hdmi_Get_Detect_Info(VIDEO_RPC_DOLBY_VISION_HDMI_GET_DETECT_INFO *pGetDetectInfo)
{
	unsigned char ret=0;
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	unsigned long result =0;
#endif
	VIDEO_RPC_DOLBY_VISION_HDMI_GET_DETECT_INFO *info;
	unsigned long  vir_addr, vir_addr_noncache;
	unsigned int  phy_addr;

	//rtd_pr_hdr_debug("%s\n", __FUNCTION__);

	vir_addr = (unsigned long)dvr_malloc_uncached_specific(sizeof(VIDEO_RPC_DOLBY_VISION_HDMI_GET_DETECT_INFO), GFP_DCU1, (void**)&vir_addr_noncache);
	phy_addr = (unsigned int)dvr_to_phys((void*)vir_addr);

	info =  (VIDEO_RPC_DOLBY_VISION_HDMI_GET_DETECT_INFO *)vir_addr_noncache;
	memcpy(info, pGetDetectInfo, sizeof(VIDEO_RPC_DOLBY_VISION_HDMI_GET_DETECT_INFO));

#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	if (RPC_FAIL==send_rpc_command(RPC_VIDEO, VIDEO_RPC_DOLBY_VISION_HDMI_Get_Detect_Info,phy_addr, 0, &result))
	{
		rtd_pr_hdr_debug("RPC fail!!\n");
		ret=1;
	}
#endif

	pGetDetectInfo->detect_ret = info->detect_ret;
	//rtd_pr_hdr_debug("[DolbyV] ret=%d/%x\n", htonl(info->detect_ret), htonl(info->detect_ret));

	//memcpy(pGetDetectInfo, &info, sizeof(VIDEO_RPC_DOLBY_VISION_HDMI_GET_DETECT_INFO));
	dvr_free((void *)vir_addr);
	//rtd_pr_hdr_debug("res=%x\n", (res? *res: 0));

    return ret;
}



int Scaler_HDR_vrpc_DolbyVision_Hdmi_SetDisplayEnable(VIDEO_RPC_DOLBY_VISION_HDMI_SET_DISPLAY_ENABLE *pSetDisplayInfo)
{
	unsigned char ret=0;
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	unsigned long result =0;
#endif
	VIDEO_RPC_DOLBY_VISION_HDMI_SET_DISPLAY_ENABLE *info;
	unsigned long  vir_addr, vir_addr_noncache;
	unsigned int  phy_addr;

	//rtd_pr_hdr_debug("%s\n", __FUNCTION__);

	vir_addr = (unsigned long)dvr_malloc_uncached_specific(sizeof(VIDEO_RPC_DOLBY_VISION_HDMI_SET_DISPLAY_ENABLE ), GFP_DCU1, (void**)&vir_addr_noncache);
	phy_addr = (unsigned int)dvr_to_phys((void*)vir_addr);

	info =  (VIDEO_RPC_DOLBY_VISION_HDMI_SET_DISPLAY_ENABLE *)vir_addr_noncache;

	//rtd_pr_hdr_debug("%s\n", __FUNCTION__);

	memcpy(info, pSetDisplayInfo, sizeof(VIDEO_RPC_DOLBY_VISION_HDMI_SET_DISPLAY_ENABLE));
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	if (RPC_FAIL==send_rpc_command(RPC_VIDEO, VIDEO_RPC_DOLBY_VISION_HDMI_Set_Display_Enable,phy_addr, 0, &result))
	{
		rtd_pr_hdr_debug("RPC fail!!\n");
		ret=1;
	}
#endif
	dvr_free((void *)vir_addr);

	//rtd_pr_hdr_debug("res=%x\n", (res? *res: 0));

    return ret;
}
#endif
