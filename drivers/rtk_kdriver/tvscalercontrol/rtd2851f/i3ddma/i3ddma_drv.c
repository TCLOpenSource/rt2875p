#if 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif
#include <rbus/dma_vgip_reg.h>
#include <rbus/h3ddma_rgb2yuv_dither_4xxto4xx_reg.h>
#include <rbus/h3ddma_reg.h>
//#include <rbus/h3ddma_hsd_reg.h>
#include <rbus/scaleup_reg.h>
//#include <rbus/scaler/rbusPpOverlayReg.h>
#include <rbus/mdomain_cap_reg.h>
#include <rbus/mdomain_disp_reg.h>
//#include <rbus/scaler/rbusPpOverlayReg.h>
#include <rbus/ppoverlay_reg.h>

#ifndef BUILD_QUICK_SHOW
#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/uaccess.h>
#include <asm/cacheflush.h>
#include <linux/pageremap.h>

//#include <mach/video_driver.h>
#include <rtk_kdriver/RPCDriver.h>


#include <uapi/linux/const.h>
#include <linux/mm.h>

#endif
#include <tvscalercontrol/i3ddma/i3ddma.h>
#include <tvscalercontrol/i3ddma/i3ddma_drv.h>
#include <tvscalercontrol/i3ddma/i3ddma_uzd.h>
#include <tvscalercontrol/i3ddma/i3ddmacolorfmtconvert.h>
#include <tvscalercontrol/scaler/modeState.h>
//#include <tvscalercontrol/scaler/scalerlib.h>
#include <tvscalercontrol/vip/ultrazoom.h>
#include <tvscalercontrol/panel/panelapi.h>
#include <tvscalercontrol/adcsource/vga.h>
#include <tvscalercontrol/adcsource/adcctrl.h>
#include <scaler_vfedev.h>
//#ifdef DTV_SCART_OUT_ENABLE
//#include "tvscalercontrol/tve/tve.h"
#include <rbus/dma_vgip_reg.h>

#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerDrvCommon.h>
#else
#include <scalercommon/scalerDrvCommon.h>
#endif

#ifndef BUILD_QUICK_SHOW

#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerCommon.h>
#else
#include <scalercommon/scalerCommon.h>
#endif

//#endif
//#include <rbus/dc2h_dma_reg.h>
#include <tvscalercontrol/scalerdrv/scaler_i2rnd.h>

#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <VideoRPC_System.h>
	#include <VideoRPC_System_data.h>
#else
	#include <rtk_kdriver/rpc/VideoRPC_System.h>
	#include <rtk_kdriver/rpc/VideoRPC_System_data.h>
#endif
//#include <tvscalercontrol/hdmirx/hdmiInternal.h>
#endif


#define UNCAC_BASE		_AC(0xa0000000, UL)

#include <rtd_log/rtd_module_log.h>

void drvif_I3DDMA_Reset_Vgip_Source_Sel(void)
{
	dma_vgip_dma_vgip_ctrl_RBUS dma_vgip_chn1_ctrl_reg;

	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_in_sel = 2;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);

	return;
}

#ifndef BUILD_QUICK_SHOW
extern unsigned char Get_Factory_SelfDiagnosis_Mode(void);




void drvif_I3DDMA_Enable_Vgip(void)
{
	//dma_vgip_ctrl_RBUS dma_vgip_chn1_ctrl_reg;
	dma_vgip_dma_vgip_ctrl_RBUS dma_vgip_chn1_ctrl_reg;

	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_ivrun = 1;
	dma_vgip_chn1_ctrl_reg.dma_in_clk_en = 1;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);

	return;
}



void drvif_I3DDMA_Disable_Vgip(void)
{
	//dma_vgip_ctrl_RBUS dma_vgip_chn1_ctrl_reg;
	dma_vgip_dma_vgip_ctrl_RBUS dma_vgip_chn1_ctrl_reg;

	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_ivrun = 0;
	dma_vgip_chn1_ctrl_reg.dma_in_clk_en = 0;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);

	return;
}

void drvif_I3DDMA_freeze_enable(unsigned char enable)
{
	h3ddma_cap0_cap_ctl0_RBUS cap_ctl0_reg;

	cap_ctl0_reg.regValue = IoReg_Read32(H3DDMA_CAP0_Cap_CTL0_reg);
	if(enable)
		cap_ctl0_reg.cap0_freeze_en = 1;
	else
		cap_ctl0_reg.cap0_freeze_en = 0;
	IoReg_Write32(H3DDMA_CAP0_Cap_CTL0_reg, cap_ctl0_reg.regValue);

	return;
}

//extern "C"
//{
extern unsigned int Get_DISP_DEN_STA_HPOS(void);
extern unsigned int Get_DISP_DEN_STA_VPOS(void);
//}
static void I3DDMA_Set_Vgip(unsigned char src, DISPD_CAP_SRC dispD_sel, unsigned char mode)
{
	//dma_vgip_ctrl_RBUS dma_vgip_chn1_ctrl_reg;
	dma_vgip_dma_vgip_ctrl_RBUS dma_vgip_chn1_ctrl_reg;
	//dma_vgip_act_hsta_width_RBUS    dma_vgip_chn1_act_hsta_width_reg;
	dma_vgip_dma_vgip_act_hsta_width_RBUS dma_vgip_chn1_act_hsta_width_reg;
	//dma_vgip_act_vsta_length_RBUS   dma_vgip_chn1_act_vsta_length_reg;
	dma_vgip_dma_vgip_act_vsta_length_RBUS dma_vgip_chn1_act_vsta_length_reg;
	//dma_vgip_delay_RBUS dma_vgip_chn1_delay_reg;
	dma_vgip_dma_vgip_delay_RBUS  dma_vgip_chn1_delay_reg;
	//dma_vgip_cts_fifo_ctl_RBUS 			 	 dma_cts_fifo_ctl_reg;
	dma_vgip_dma_vgip_cts_fifo_ctl_RBUS  dma_cts_fifo_ctl_reg;
	//int dispD_hstaOffset=0;

	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_in_clk_en = 0;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);

	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_in_sel = src;
	dma_vgip_chn1_ctrl_reg.dma_digital_mode = mode;

	if(dispD_sel == SCART_OUT_DTV)
		dma_vgip_chn1_ctrl_reg.dma_field_det_en = 0;
	else
		dma_vgip_chn1_ctrl_reg.dma_field_det_en = 1;

	dma_vgip_chn1_ctrl_reg.dma_field_inv = 1;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);

	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_in_clk_en = 1;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);


	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_ivrun = 1;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);


	dma_vgip_chn1_act_hsta_width_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg);
	dma_vgip_chn1_act_vsta_length_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_ACT_VSTA_Length_reg);

#if 0
	// [FIX-ME] unknown DispD horizontal offset
	switch(dispD_sel){
	  case CAP_SRC_SUBTITLE:
		dispD_hstaOffset = -2;
		break;
	  case CAP_SRC_OSD:
		dispD_hstaOffset = 11;
		break;
	  default:
		break;
	}
#endif

	#ifdef HDMI_4K2K_I3DDMA
	if(Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE) == HDMI_4K2K_TIMING_WIDTH && Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE) == HDMI_4K2K_TIMING_LENGTH)//for 4k2k
	{
		dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_sta = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_STA_PRE);
		dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_wid = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE);
		dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_sta = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA_PRE);
		dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_len= Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE);
	}
	#endif

	//change this item
	if(dispD_sel == SCART_OUT_DTV){
		dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_sta = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_STA_PRE);
		dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_wid = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE);
		dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_sta = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA_PRE);
		if((Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE) < 288 ) && Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_THRIP))//SE do scaling up to 720x288 interlace
			dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_len= 288;
		else if((Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE) < 576)  && !Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_THRIP))//SE do scaling up to 720x576 prograssive
			dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_len= 576;
		else
			dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_len= Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE);
		if(Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE) < 720)
			dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_wid = 720;
		else
			dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_wid = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE);
	}
	else{
		if(dispD_sel == CAP_SRC_SUBTITLE || dispD_sel == CAP_SRC_OSD ||dispD_sel == CAP_SRC_DITHER)
		{
			rtd_pr_i3ddma_debug("[dsip2tve]CAP_SRC_SUBTITLE||CAP_SRC_OSD||CAP_SRC_DITHER..._DISP_WID=%x,_DISP_LEN=%x\n",_DISP_WID,_DISP_LEN);
			dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_sta = Get_DISP_DEN_STA_HPOS();// + dispD_hstaOffset;
			dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_wid = Get_DISP_ACT_END_HPOS();
			dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_sta = Get_DISP_DEN_STA_VPOS();
			dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_len= Get_DISP_ACT_END_VPOS();
		}
		else if(dispD_sel == CAP_SRC_UZU)
		{
			dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_sta = Get_DISP_DEN_STA_HPOS();
			dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_wid = Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID);
			dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_sta = Get_DISP_DEN_STA_VPOS();
			dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_len= Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN);
		}
	}

	IoReg_Write32(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg, dma_vgip_chn1_act_hsta_width_reg.regValue);
	IoReg_Write32(DMA_VGIP_DMA_VGIP_ACT_VSTA_Length_reg, dma_vgip_chn1_act_vsta_length_reg.regValue);

	// IHS/IVS delay
	dma_vgip_chn1_delay_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_DELAY_reg);
	dma_vgip_chn1_delay_reg.dma_ihs_dly = 0;
	dma_vgip_chn1_delay_reg.dma_ivs_dly = 0;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_DELAY_reg, dma_vgip_chn1_delay_reg.regValue);

	dma_cts_fifo_ctl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTS_FIFO_CTL_reg);
	dma_cts_fifo_ctl_reg.dma_en_ctsfifo_vsrst_vgip = 1;
	dma_cts_fifo_ctl_reg.dma_force_ctsfifo_rstn_vgip = 1;
	dma_cts_fifo_ctl_reg.dma_force_ctsfifo_rstn_sdnr = 1;
	dma_cts_fifo_ctl_reg.dma_en_ctsfifo_vsrst_sdnr = 0;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTS_FIFO_CTL_reg, dma_cts_fifo_ctl_reg.regValue);

	return;
}

#if 1

unsigned char drvif_I3DDMA_StartCaptureRpc(int width /* = 720 */, int height /* = 480*/, DISPD_CAP_SRC mode /* = SUB_MIXER */)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
#ifndef CONFIG_SCALER_BRING_UP
	unsigned long ret;
#endif
#endif

	VIDEO_RPC_CAP_START_CAPTURE *start;
//		unsigned long ret;
	unsigned long  vir_addr, vir_addr_noncache = 0;
	unsigned int  phy_addr;
	vir_addr = (unsigned long)dvr_malloc_uncached_specific(sizeof(VIDEO_RPC_CAP_START_CAPTURE), GFP_DCU1, (void*)&vir_addr_noncache);
	//phy_addr = (unsigned int)virt_to_phys((void*)vir_addr);
	phy_addr = (unsigned int)dvr_to_phys((void*)vir_addr);


	start = (VIDEO_RPC_CAP_START_CAPTURE *)vir_addr_noncache;
	start->instanceID = 0;
	start->instanceID = htonl(start->instanceID);
	start->startMode = mode;
	start->startMode = htonl(start->startMode);
	start->cap_width = width;
	start->cap_width = htonl(start->cap_width);
	start->cap_height = height;
	start->cap_height = htonl(start->cap_height);
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
#ifndef CONFIG_SCALER_BRING_UP
	if (send_rpc_command(RPC_VIDEO, VIDEO_RPC_CAP_ToAgent_StartCapture, phy_addr, 0, &ret))
	{
		rtd_pr_i3ddma_debug("RPC fail!!\n");
		dvr_free((void*)vir_addr);
		return FALSE;
	}
#endif
#endif
	dvr_free((void*)vir_addr);

	return TRUE;
}

unsigned char drvif_I3DDMA_StopCaptureRpc(void)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
#ifndef CONFIG_SCALER_BRING_UP
	unsigned long ret;
#endif
#endif

	VIDEO_RPC_CAP_STOP_CAPTURE *stop;
//		unsigned long ret;
	unsigned long  vir_addr, vir_addr_noncache = 0;
	unsigned int  phy_addr;
	vir_addr = (unsigned long)dvr_malloc_uncached_specific(sizeof(VIDEO_RPC_CAP_STOP_CAPTURE), GFP_DCU1, (void*)&vir_addr_noncache);
	//phy_addr = (unsigned int)virt_to_phys((void*)vir_addr);
	phy_addr = (unsigned int)dvr_to_phys((void*)vir_addr);

	stop = (VIDEO_RPC_CAP_STOP_CAPTURE *)vir_addr_noncache;
	stop->instanceID = 0;
	stop->instanceID = htonl(stop->instanceID);
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
#ifndef CONFIG_SCALER_BRING_UP
	if (send_rpc_command(RPC_VIDEO, VIDEO_RPC_CAP_ToAgent_StopCapture, phy_addr, 0, &ret))
	{
		rtd_pr_i3ddma_debug("RPC fail!!\n");
		dvr_free((void*)vir_addr);
		return 1;
	}
#endif
#endif
	dvr_free((void*)vir_addr);
	return TRUE;
}

#endif

static void  drvif_I3DDMA_Enable_ViCapture(unsigned char enable, IDMA_DISPD_CAPTURE_INFO *capInfo)
{


	if(enable){
		// Enable VI capture state machine
		rtd_pr_i3ddma_debug("[kernel] drvif_I3DDMA_StartCaptureRpcdrvif_I3DDMA_StartCaptureRpc start\n");
		drvif_I3DDMA_StartCaptureRpc(capInfo->capWid, capInfo->capLen, capInfo->capSrc);
		rtd_pr_i3ddma_debug("[kernel] drvif_I3DDMA_StartCaptureRpcdrvif_I3DDMA_StartCaptureRpc end\n");
	}else{
		// Disable VI capture state machine
		rtd_pr_i3ddma_debug("[kernel] drvif_I3DDMA_StopCaptureRpcdrvif_I3DDMA_StopCaptureRpc start\n");
		drvif_I3DDMA_StopCaptureRpc();
		rtd_pr_i3ddma_debug("[kernel] drvif_I3DDMA_StopCaptureRpcdrvif_I3DDMA_StopCaptureRpc end\n");

		I3DDMA_3DDMADeInit();
	}

	return;
}

void drvif_I3DDMA_dispD_to_IdmaVI_idmaConfig(IDMA_DISPD_CAPTURE_INFO capInfo)
{
	//static unsigned char curCapEnable=FALSE;
	SIZE inSize, outSize;

	rtd_pr_i3ddma_debug("[IDMA] DispD capture %s\n", (capInfo.enable? "ENABLE": "DISABLE"));
	rtd_pr_i3ddma_debug("[IDMA] DispD capture %s\n", (capInfo.enable? "ENABLE": "DISABLE"));
	rtd_pr_i3ddma_debug("[IDMA] DispD capture %s\n", (capInfo.enable? "ENABLE": "DISABLE"));
	// IDMA capture enable
	if(capInfo.enable == TRUE){
		//if(curCapEnable == FALSE)
		{
			// IDMA VGIP init
			I3DDMA_Set_Vgip(VGIP_SRC_DISPD, capInfo.capSrc, VGIP_MODE_ANALOG);

			inSize.nWidth = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_DISP_WID);
			inSize.nLength = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_DISP_LEN);


			outSize.nWidth = capInfo.capWid;
			outSize.nLength = capInfo.capLen;

			if((outSize.nWidth == 0 ) || (outSize.nLength == 0)){
				outSize.nWidth = inSize.nWidth;
				outSize.nLength = inSize.nLength;
			}
#if 0
			rtd_pr_i3ddma_debug("[IVI] Capture Src/H/V=%d/%d/%d\n", capInfo.capSrc, outSize.nWidth, outSize.nLength);
			Scaler_DispSetScaleStatus(SLR_SUB_DISPLAY, SLR_SCALE_H_DOWN, (inSize.nWidth > outSize.nWidth)?true:false);
			Scaler_DispSetScaleStatus(SLR_SUB_DISPLAY, SLR_SCALE_V_DOWN, (inSize.nLength > outSize.nLength)?true:false);
			Scaler_DispSetStatus(SLR_SUB_DISPLAY, SLR_DISP_422CAP, true); 	// VI input support 420/422, so need convert input video to 422 format in UZD
			Scaler_DispSetStatus(SLR_SUB_DISPLAY, SLR_DISP_10BIT, true);
			Scaler_DispSetStatus(SLR_SUB_DISPLAY, SLR_DISP_FSYNC_VUZD, 0x2);
			Scaler_DispSetStatus(SLR_SUB_DISPLAY, SLR_DISP_COMP, false);
#endif
			// IDMA color conversion
			I3DDMA_color_colorspacergb2yuvtransfer(capInfo.capSrc);

			// IDMA scaling down setup
			I3DDMA_ultrazoom_config_scaling_down(&inSize, &outSize, 1);

			// IDMA capture setup
			drvif_I3DDMA_Set_ViCapture(outSize);
		}

		// enable IDMA capture
		drvif_I3DDMA_Enable_ViCapture(TRUE, &capInfo);

		//curCapEnable = TRUE;
	} else{ // IDMA capture disable
		// disable IDMA & release allocated memory
		drvif_I3DDMA_Enable_ViCapture(FALSE, (IDMA_DISPD_CAPTURE_INFO*)NULL);
		// disable DMA VGIP when disable IDMA capture
		drvif_I3DDMA_Disable_Vgip();
		//curCapEnable = FALSE;
	}


	return;
}
extern void CVoSqmodeTVERpc(int width , int height , unsigned int framerate);
#if 0//not use
static void  drvif_I3DDMA_Enable_SqCapture(unsigned char enable, IDMA_DISPD_CAPTURE_INFO *capInfo)
{
	i3ddma_i3ddma_enable_RBUS i3ddma_enable_reg;

	if(enable){
		// Enable Sq capture state machine
	/*//[mark by crixus-mac3 linux based]
	if(capInfo->capSrc == CAP_SRC_UZU )
		CVoSqmodeTVERpc(capInfo->capWid, capInfo->capLen, Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ));
	else
		CVoSqmodeTVERpc(capInfo->capWid, capInfo->capLen,600);
	*/
	}else{
		// Disable Sq capture state machine
		//drvif_I3DDMA_StopCaptureRpc();

		// IDMA timeing gen disable
		i3ddma_enable_reg.regValue = IoReg_Read32(I3DDMA_I3DDMA_ENABLE_VADDR);
		i3ddma_enable_reg.frc_en = 0;
		i3ddma_enable_reg.lr_separate_en = 0;
		i3ddma_enable_reg.tg_en = 0;
		i3ddma_enable_reg.cap_en = 0; // disable IDMA capture before VI capture enable
		IoReg_Write32(I3DDMA_I3DDMA_ENABLE_VADDR, i3ddma_enable_reg.regValue );

		// alloc capture buffer
		I3DDMA_3DDMADeInit();
	}

	return;
}


void drvif_I3DDMA_dispDtoI3ddma_TVE(IDMA_DISPD_CAPTURE_INFO capInfo)
{
	SIZE inSize,outSize;
	ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;

// IDMA capture enable
	if(capInfo.enable == TRUE){

		// enable DMA VGIP
		I3DDMA_Set_Vgip(VGIP_SRC_DISPD,capInfo.capSrc, VGIP_MODE_ANALOG);


		inSize.nWidth = Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID);
		inSize.nLength = Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN);
		if(capInfo.capSrc == CAP_SRC_SUBTITLE ||capInfo.capSrc == CAP_SRC_OSD )
		{
			inSize.nWidth  = Get_DISP_ACT_END_HPOS();
			inSize.nLength = Get_DISP_ACT_END_VPOS();
		}
		outSize.nWidth = capInfo.capWid;
		outSize.nLength = capInfo.capLen;
		if((outSize.nWidth == 0 ) || (outSize.nLength == 0)){
			outSize.nWidth = inSize.nWidth;
			outSize.nLength = inSize.nLength;
		}

		Scaler_DispSetScaleStatus(SLR_SUB_DISPLAY, SLR_SCALE_H_DOWN, (inSize.nWidth > outSize.nWidth)?true:false);
		Scaler_DispSetScaleStatus(SLR_SUB_DISPLAY, SLR_SCALE_V_DOWN, (inSize.nLength > outSize.nLength)?true:false);
		Scaler_DispSetStatus(SLR_SUB_DISPLAY, SLR_DISP_422CAP, true);
		Scaler_DispSetStatus(SLR_SUB_DISPLAY, SLR_DISP_10BIT, false);
		Scaler_DispSetStatus(SLR_SUB_DISPLAY, SLR_DISP_FSYNC_VUZD, 0x2);
		Scaler_DispSetStatus(SLR_SUB_DISPLAY, SLR_DISP_COMP, false);

		// IDMA color conversion
		I3DDMA_color_colorspacergb2yuvtransfer(capInfo.capSrc);

		// IDMA scaling down setup
		I3DDMA_ultrazoom_config_scaling_down(&inSize, &outSize, 1);


		display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
		display_timing_ctrl2_reg.frc2fsync_protection = _ENABLE;
		//display_timing_ctrl2_reg.d2i3ddma_en_ctsfifo_vsrst_i3= _ENABLE;
		//display_timing_ctrl2_reg.d2i3ddma_force_ctsfifo_rstn_i3 = _ENABLE;
		display_timing_ctrl2_reg.d2i3ddma_en_ctsfifo_vsrst_i2 =_ENABLE;
		display_timing_ctrl2_reg.d2i3ddma_force_ctsfifo_rstn_i2 = _ENABLE;
		display_timing_ctrl2_reg.d2i3ddma_src_sel = capInfo.capSrc;
		display_timing_ctrl2_reg.repeat_flag_mode = 0x3;
		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);

		//IoReg_Mask32(PPOVERLAY_Display_Timing_CTRL2_reg, ~_BIT16, capInfo.capSrc<<16);   //Dispd to i3ddma src sel
		IoReg_Write32(SCALEUP_D_UZU_GLOBLE_CTRL_VADDR, 0x00000032); 	// D-domain to SUB
		IoReg_Write32(SCALEUP_D_UZU_PATGEN_FRAME_TOGGLE_VADDR, 0x000000EF); 	// D-domain Pattern Gen Color-Bar Width
		rtd_pr_i3ddma_info("I3DDMA:src = %d, insize.width = %d, insize.len = %d, outsize.width = %d, outsize.len = %d\n", capInfo.capSrc, inSize.nWidth, inSize.nLength,outSize.nWidth, outSize.nLength);

		// IDMA capture setup
		drvif_I3DDMA_set_capture_for2D(&outSize);

		// enable IDMA capture
		drvif_I3DDMA_Enable_SqCapture(TRUE, &capInfo);

	}
	else{
		// disable DMA VGIP
		drvif_I3DDMA_Disable_Vgip();

		// disable IDMA & release allocated memory
		drvif_I3DDMA_Enable_SqCapture(FALSE, (IDMA_DISPD_CAPTURE_INFO*)NULL);
	}

}
#endif

//#ifdef DTV_SCART_OUT_ENABLE
//static bool bVodma2I3Dma;
extern void  drvif_I3DDMA_set_capture(SIZE *outsize,unsigned char capformat422);
//extern void drvif_module_tve_interrupt(int enable);
#if 0
static IDMA_VGIP2_CTRL_INFO imdavgip2CtrlInfo = {(SRC_TYPE)0, 0, 0, {0,0,0,0,0,0}};

unsigned int scaler_vodmatoI3ddma_get_vgip2IhWidth(void)
{
	return imdavgip2CtrlInfo.srcInfo.width;
}

unsigned int scaler_vodmatoI3ddma_get_vgip2IhCount(void)
{
	return imdavgip2CtrlInfo.srcInfo.ihCount;
}

unsigned int scaler_vodmatoI3ddma_get_vgip2IvFreq(void)
{
	return imdavgip2CtrlInfo.srcInfo.frame_rate;
}

unsigned int scaler_vodmatoI3ddma_get_vgip2IvHeight(void)
{
	return imdavgip2CtrlInfo.srcInfo.height;
}

unsigned char scaler_vodmatoI3ddma_get_vgip2SrcFrom(void)
{
	return imdavgip2CtrlInfo.srcFrom;
}

unsigned int scaler_vodmatoI3ddma_get_vgip2SrcInterlaced(void)
{
	return imdavgip2CtrlInfo.srcInfo.interlacedVideo;
}

unsigned char scaler_vodmatoI3ddma_get_vgip2SrcIndex(void)
{
	return imdavgip2CtrlInfo.srcIndex;
}

bool scaler_vodmatoI3ddma_get_vgip2CaptFormat(void)
{
	return imdavgip2CtrlInfo.srcInfo.chroma_422Cap;
}

static void scaler_vodmatoI3ddma_save_vgip1Info(void)
{
	imdavgip2CtrlInfo.srcType				= (SRC_TYPE)Scaler_InputSrcGetMainChType();
	imdavgip2CtrlInfo.srcFrom				= Scaler_InputSrcGetMainChFrom();
	imdavgip2CtrlInfo.srcIndex				= Scaler_DispGetInputInfo(SLR_INPUT_INPUT_SRC);
	imdavgip2CtrlInfo.srcInfo.videoMode		= Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR);
	imdavgip2CtrlInfo.srcInfo.width			= Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE);
	imdavgip2CtrlInfo.srcInfo.height		= Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE);
	imdavgip2CtrlInfo.srcInfo.ihCount		= Scaler_DispGetInputInfo(SLR_INPUT_H_LEN);
	imdavgip2CtrlInfo.srcInfo.ihCount_offLine	= Scaler_DispGetInputInfo(SLR_INPUT_H_COUNT);
	imdavgip2CtrlInfo.srcInfo.ivCount		= Scaler_DispGetInputInfo(SLR_INPUT_V_LEN);
	imdavgip2CtrlInfo.srcInfo.chroma_422Cap= Scaler_DispGetStatus((SCALER_DISP_CHANNEL)_CHANNEL1, SLR_DISP_422CAP);
	imdavgip2CtrlInfo.srcInfo.frame_rate		= Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ);
	imdavgip2CtrlInfo.srcInfo.interlacedVideo	= Scaler_DispGetStatus((SCALER_DISP_CHANNEL)_CHANNEL1, SLR_DISP_INTERLACE);

	rtd_pr_i3ddma_info("[vodmatoI3ddma] VGIP1 type/from/Idx/mode, wid/len/Fr[%d/%d/%d/%d, %d/%d/%d]\n",
		imdavgip2CtrlInfo.srcType, imdavgip2CtrlInfo.srcFrom, imdavgip2CtrlInfo.srcIndex, imdavgip2CtrlInfo.srcInfo.videoMode,
		imdavgip2CtrlInfo.srcInfo.width, imdavgip2CtrlInfo.srcInfo.height, imdavgip2CtrlInfo.srcInfo.frame_rate);
}

static void VodmatoI3ddma_Set_Vgip(unsigned char src, unsigned char mode)
{
	dma_vgip_ctrl_RBUS 				dma_vgip_chn1_ctrl_reg;
	dma_vgip_act_hsta_width_RBUS		dma_vgip_chn1_act_hsta_width_reg;
	dma_vgip_act_vsta_length_RBUS   	dma_vgip_chn1_act_vsta_length_reg;
	dma_vgip_cts_fifo_ctl_RBUS					dma_cts_fifo_ctl_reg;

	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_in_clk_en = 0;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);

	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_in_sel = src;
	dma_vgip_chn1_ctrl_reg.dma_digital_mode = mode;
	//dma_vgip_chn1_ctrl_reg.dma_odd_det_en = 0;
	//dma_vgip_chn1_ctrl_reg.dma_odd_inv = 1;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);

	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_in_clk_en = 1;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);


	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_ivrun = 1;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);


	dma_vgip_chn1_act_hsta_width_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg);
	dma_vgip_chn1_act_vsta_length_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_ACT_VSTA_Length_reg);

	dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_sta = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_STA_PRE);
	dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_wid = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE);
	dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_sta = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA_PRE);
	dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_len= Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE);

	IoReg_Write32(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg, dma_vgip_chn1_act_hsta_width_reg.regValue);
	IoReg_Write32(DMA_VGIP_DMA_VGIP_ACT_VSTA_Length_reg, dma_vgip_chn1_act_vsta_length_reg.regValue);

	dma_cts_fifo_ctl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTS_FIFO_CTL_reg);
	dma_cts_fifo_ctl_reg.dma_en_ctsfifo_vsrst_vgip = 1;
	dma_cts_fifo_ctl_reg.dma_force_ctsfifo_rstn_vgip = 1;
	dma_cts_fifo_ctl_reg.dma_force_ctsfifo_rstn_sdnr = 1;
	dma_cts_fifo_ctl_reg.dma_en_ctsfifo_vsrst_sdnr = 0;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTS_FIFO_CTL_reg, dma_cts_fifo_ctl_reg.regValue);

}
#endif
void scaler_vodmatoI3ddma_vgip2_config(SIZE outsize)
{
	unsigned char nMode;
//	int srcIdx;
	unsigned char source;
	unsigned char mode;
	SIZE insize;

	StructDisplayInfo *info;
	rtd_pr_i3ddma_debug("[scart out]###############%s~~1\n",__FUNCTION__);

	// keep the current VGIP1 input source information
	//scaler_vodmatoI3ddma_save_vgip1Info();//this can delete function

	// VGIP2 parameters setup sync from VGIP1
	//scaler_MaincvtSub_config_sync_vgip();
#if 0
	if(Scaler_InputSrcGetMainChType() == _SRC_HDMI)
		source = VGIP_SRC_TMDS;
	else if(Scaler_InputSrcGetMainChType() == _SRC_VO)
		source = VGIP_SRC_HDR1;
	else
		source = VGIP_SRC_ADC;
#endif

	//LGE WebOS Project - only DTV using
	source = VGIP_SRC_HDR1;

	mode = VGIP_MODE_ANALOG;

	rtd_pr_i3ddma_debug("[scart out]input source = %d\n",source);

	//VodmatoI3ddma_Set_Vgip(source, mode);
	I3DDMA_Set_Vgip(source,SCART_OUT_DTV, mode);

	// VGIP2 color format configure
	info = Scaler_DispGetInfoPtr();
	nMode = (info->IPV_ACT_LEN > 480? 1: 0);

	I3DDMA_ColorSpacergb2yuvtransfer(Scaler_InputSrcGetType(SLR_MAIN_DISPLAY), nMode, 0);

	insize.nWidth = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE);
	insize.nLength = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE);

	//check this item
	if(insize.nWidth < 720){//if width < 720, must be the same with input, else always output 720x576
		outsize.nWidth = 720;//insize.nWidth;
	}

	rtd_pr_i3ddma_debug("[scart out]insize.nWidth = %d\n",insize.nWidth);
	rtd_pr_i3ddma_debug("[scart out]insize.nLength = %d\n",insize.nLength);
	rtd_pr_i3ddma_debug("[scart out]outsize.nWidth = %d\n",outsize.nWidth);
	rtd_pr_i3ddma_debug("[scart out]outsize.nLength = %d\n",outsize.nLength);

	if(Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_THRIP))
	{

		outsize.nLength /=2;
		rtd_pr_i3ddma_debug("[scart out]enter DI, nLength = %d\n",outsize.nLength);
		//if(TRUE ==  fwif_scaler_decide_display_cut4line(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)))// for delete 4 lines (Up 2 line and down 2 line)
		//	insize.nLength = insize.nLength -4;
	}

	Scaler_DispSetScaleStatus(SLR_SUB_DISPLAY, SLR_SCALE_H_DOWN, (insize.nWidth > outsize.nWidth)?true:false);
	Scaler_DispSetScaleStatus(SLR_SUB_DISPLAY, SLR_SCALE_V_DOWN, (insize.nLength > outsize.nLength)?true:false);

	Scaler_DispSetStatus(SLR_SUB_DISPLAY, SLR_DISP_10BIT, false);
	Scaler_DispSetStatus(SLR_SUB_DISPLAY, SLR_DISP_FSYNC_VUZD, 0x2);
	Scaler_DispSetStatus(SLR_SUB_DISPLAY, SLR_DISP_COMP, false);
	Scaler_DispSetStatus(SLR_SUB_DISPLAY, SLR_DISP_422CAP, true);
	I3DDMA_ultrazoom_config_scaling_down(&insize, &outsize, 1);

	drvif_I3DDMA_set_capture_for2D(&outsize);
}

void drvif_i3ddma_vactive_end_irq(int enable)
{
	if(enable)
		IoReg_Mask32(DMA_VGIP_DMA_VGIP_CTRL_reg , ~_BIT25, _BIT25);	//enable VSync end interrupt
	else
		IoReg_Mask32(DMA_VGIP_DMA_VGIP_CTRL_reg , ~_BIT25, 0);	//disable VSync end interrupt
}

void disable_I3DDMA_dispDtoSub(void)
{

	// IDMA timeing gen disable
       IoReg_Mask32(H3DDMA_I3DDMA_enable_reg, ~(/*H3DDMA_I3DDMA_enable_cap_en_mask|*/H3DDMA_I3DDMA_enable_lr_separate_en_mask
                               |H3DDMA_I3DDMA_enable_tg_en_mask|H3DDMA_I3DDMA_enable_frc_en_mask),/* H3DDMA_I3DDMA_enable_cap_en(0)
                               |*/H3DDMA_I3DDMA_enable_lr_separate_en(0)|H3DDMA_I3DDMA_enable_tg_en(0)
                               |H3DDMA_I3DDMA_enable_frc_en(0));// FRC, timing gen, 3D LR separate, Capture disable

	// release capture buffer
	I3DDMA_disp2tve_CapBufDeInit();

	// disable DMA VGIP when disable IDMA capture
	drvif_I3DDMA_Disable_Vgip();

	//Not to disable DAC power in based driver - LGE Project @ Crixus 20141110
	//drvif_module_tve_AVout_Enable(TVE_AVOUT_DISABLE);

	 drvif_i3ddma_vactive_end_irq(false);

	 //drvif_module_tve_interrupt(false);

	 //bVodma2I3Dma = false;
}

UINT8 tve_vflip_en_g = false;
static UINT8 fenable_dispDtoI3ddma;
static UINT8 fScartOut_isr_Enable;
//static UINT8 fScartOut_VO_clock_check;
//static UINT8 fScartOut_VO_scaling_enable;



unsigned char scaler_dispDtoI3ddma_get_enable(void)
{
	return fenable_dispDtoI3ddma;
}
void scaler_dispDtoI3ddma_set_enable(unsigned char enable)
{
	fenable_dispDtoI3ddma=enable;
}
unsigned char scaler_get_tve_vflip_enable(void)
{
	return tve_vflip_en_g;
}
void scaler_set_tve_vflip_enable(unsigned char enable)
{
	tve_vflip_en_g = enable;
}
unsigned char scaler_scart_out_isr_get_enable(void)
{
	return fScartOut_isr_Enable;
}
void scaler_scart_out_isr_set_enable(unsigned char enable)
{
	fScartOut_isr_Enable=enable;
}

#if 0	//remove tve
void scaler_vodmatoI3ddma_prog(TVE_VIDEO_MODE_SEL tve_mode, UINT8 tve_vflip_en)
{

	SIZE tve_outsize;
//	UINT32 i3ddmaCapAddr;
	//UINT8 tve_vflip_en = 1;

    	//if (bVodma2I3Dma == true)
    	//{
  	//	return;
    	//}

	if (tve_mode == TVE_NTSC) {
		tve_outsize.nWidth = 720;
		tve_outsize.nLength = 480;
	}
	else if (tve_mode == TVE_PAL_I) {
		tve_outsize.nWidth = 720;
		tve_outsize.nLength = 576;
	} else {
            rtd_pr_i3ddma_debug("wrong params  tve_mode=%d\n",(int)tve_mode);
            return ;
       }
	/*not to run dispD to i3ddma path*/
       	scaler_dispDtoI3ddma_set_enable(FALSE);

	disable_I3DDMA_dispDtoSub();
	//msleep(20);//why add this delay?
	scaler_vodmatoI3ddma_vgip2_config(tve_outsize);
	//drvif_module_tve_init();
	drvif_module_tve_set_mode(tve_mode);
	drvif_module_tve_video_memory_setting(tve_vflip_en);
	drvif_module_tve_change_memory_setting(Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE),tve_vflip_en);

	//small than 576p should send RPC
	#if 0
	if(Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_THRIP)){
		scaler_scart_out_isr_set_enable(TRUE);
	}else{
		scaler_scart_out_isr_set_enable(FALSE);
	}
	#endif
	scaler_scart_out_isr_set_enable(TRUE);
	rtd_pr_i3ddma_debug("[scart out]fScartOut_isr_Enable = %x\n",fScartOut_isr_Enable);

	//VODMA clock fine tune
	if((Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE) <= 720) && (Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE) <= 576)){
		fScartOut_VO_clock_check = TRUE;
		//SE scaling up
		if((Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE) < 720) || (Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE) < 576))
			fScartOut_VO_scaling_enable = TRUE;
		else
			fScartOut_VO_scaling_enable = FALSE;
	}else{
		fScartOut_VO_clock_check = FALSE;
	}

	scaler_set_tve_vflip_enable(tve_vflip_en);

   	 //bVodma2I3Dma = true;
#if 0//M-disp debug using
	//M-domain Disp verify
	IoReg_ClearBits(MDOMAIN_DISP_DDR_MainSubCtrl_reg,_BIT1); //clear double buffer
	IoReg_ClearBits(MDOMAIN_DISP_DDR_MainCtrl_reg,MDOMAIN_DISP_DDR_MainCtrl_main_double_en_mask); //use single buffer. [9:8] = 0 use buffer 0,
									   // [9:8] = 1 use buffer 1
	i3ddmaCapAddr = IoReg_Read32(I3DDMA_CAP_L1_START_VADDR); //capture Buf L1
	//UINT32 i3ddmaCapAddr = IoReg_Read32(I3DDMA_CAP_L2_START_VADDR); //capture Buf L2
	rtd_pr_i3ddma_debug("##### i3ddmaCapAddr : %x ##### \n", i3ddmaCapAddr);  //capture Buf L
	IoReg_Write32(MDOMAIN_DISP_DDR_MainAddr_reg, i3ddmaCapAddr); //M-Disp Buffer0 use capture Buf
#endif
}
#endif

#if 0	//remove tve
void scaler_scart_out_tve_prog(TVE_VIDEO_MODE_SEL mode, UINT32 width){

	rtd_pr_i3ddma_debug("[scart out]@@@@@@%s\n",__FUNCTION__);

	//@ Crixus 20141202
	drvif_module_tve_init();
	drvif_module_tve_set_mode(mode);
	drvif_module_tve_video_memory_setting(scaler_get_tve_vflip_enable());
	drvif_module_tve_change_memory_setting(width, scaler_get_tve_vflip_enable());
}
#endif

extern void I3DDMA_color_colorspacergb2yuvtransfer_dispDtoI3ddma(IDMA_DISPD_CAPTURE_INFO cap_info);
#define FHD_SIZE_H_WID 1920
#define FHD_SIZE_V_LEN 1080
#if 0	//remove tve
void scaler_dispDtoI3ddma_prog(TVE_VIDEO_MODE_SEL tve_mode)
{

	UINT16 tve_outsize_width = 0;
	UINT16 tve_outsize_length = 0;
	IDMA_DISPD_CAPTURE_INFO capInfo;
	SIZE inSize,outSize;
	//unsigned char nMode;
	//StructDisplayInfo *info;
	SIZE tve_outsize;
	//display_timing_ctrl2_RBUS display_timing_ctrl2_reg;
	ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;

	rtd_pr_i3ddma_debug("[scart out]@@@@@@%s[%d]\n",__FUNCTION__,__LINE__);

	if((tve_mode == TVE_NTSC) || (tve_mode == TVE_PAL_I)){
		if(tve_mode == TVE_NTSC){
			tve_outsize_width = 720;
			tve_outsize_length = 480;
		}
		else if(tve_mode == TVE_PAL_I){
			tve_outsize_width = 720;
			tve_outsize_length = 576;
		}

		tve_outsize.nWidth = tve_outsize_width;
		tve_outsize.nLength = tve_outsize_length;

		capInfo.enable = TRUE;
		capInfo.capSrc = CAP_SRC_UZU;
		capInfo.capMode = CAP_MODE_SEQ;
		capInfo.capFmt = CAP_422;
		capInfo.capWid = tve_outsize_width;
		capInfo.capLen = tve_outsize_length;

		/*enable DMA VGIP*/
		I3DDMA_Set_Vgip(VGIP_SRC_DISPD,capInfo.capSrc, VGIP_MODE_ANALOG);

		inSize.nWidth = Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID);
		inSize.nLength = Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN);
		outSize.nWidth = tve_outsize_width;
		outSize.nLength = tve_outsize_length;

		rtd_pr_i3ddma_debug("[scart out]inSize.nWidth = %d\n", inSize.nWidth);
		rtd_pr_i3ddma_debug("[scart out]inSize.nLength = %d\n", inSize.nLength);
		rtd_pr_i3ddma_debug("[scart out]outSize.nWidth = %d\n", outSize.nWidth);
		rtd_pr_i3ddma_debug("[scart out]outSize.nWidth = %d\n", outSize.nLength);

		if((inSize.nWidth == 0) || (inSize.nLength == 0)){
			rtd_pr_i3ddma_debug("[scart out][ERROR]Input size error!! Please call scart out after display is ready!!Not to call early!!\n");
			/*not to run dispD to i3ddma path*/
			scaler_dispDtoI3ddma_set_enable(FALSE);
			return;
		}

		/*enable dispD to i3ddma flag*/
		scaler_dispDtoI3ddma_set_enable(TRUE);
		scaler_set_tve_vflip_enable(Get_PANEL_VFLIP_ENABLE());

		/*use disp-D size 1920x1080 in sirius (CVT project)*/
		/*
		if((inSize.nWidth != FHD_SIZE_H_WID) || (inSize.nLength != FHD_SIZE_V_LEN)){
			inSize.nWidth = FHD_SIZE_H_WID;
			inSize.nLength = FHD_SIZE_V_LEN;
		}
		*/

		Scaler_DispSetScaleStatus(SLR_SUB_DISPLAY, SLR_SCALE_H_DOWN, (inSize.nWidth > outSize.nWidth)?true:false);
		Scaler_DispSetScaleStatus(SLR_SUB_DISPLAY, SLR_SCALE_V_DOWN, (inSize.nLength > outSize.nLength)?true:false);
		Scaler_DispSetStatus(SLR_SUB_DISPLAY, SLR_DISP_422CAP, true);
		Scaler_DispSetStatus(SLR_SUB_DISPLAY, SLR_DISP_10BIT, false);
		Scaler_DispSetStatus(SLR_SUB_DISPLAY, SLR_DISP_FSYNC_VUZD, 0x2);
		Scaler_DispSetStatus(SLR_SUB_DISPLAY, SLR_DISP_COMP, false);

		/*IDMA color conversion*/
		I3DDMA_color_colorspacergb2yuvtransfer_dispDtoI3ddma(capInfo);

		/*IDMA scaling down setup*/
		I3DDMA_ultrazoom_config_scaling_down(&inSize, &outSize, 1);


		display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
		display_timing_ctrl2_reg.frc2fsync_protection = _ENABLE;
		/*display_timing_ctrl2_reg.d2i3ddma_en_ctsfifo_vsrst_i3= _ENABLE;*/
		/*display_timing_ctrl2_reg.d2i3ddma_force_ctsfifo_rstn_i3 = _ENABLE;*/
		/*display_timing_ctrl2_reg.d2i3ddma_en_ctsfifo_vsrst_i2 =_ENABLE;*/
		/*display_timing_ctrl2_reg.d2i3ddma_force_ctsfifo_rstn_i2 = _ENABLE;*/
		display_timing_ctrl2_reg.d2i3ddma_src_sel = capInfo.capSrc;
		//display_timing_ctrl2_reg.repeat_flag_mode = 0x3;
		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);

		/*IDMA capture setup*/
		drvif_I3DDMA_set_capture_for2D(&outSize);

		/*tve process*/
		scaler_scart_out_tve_prog(tve_mode, outSize.nWidth);
	}
	else
	{
		rtd_pr_i3ddma_debug("[scart out]wrong params  tve_mode=%d\n",(int)tve_mode);
		/*not to run dispD to i3ddma path*/
		scaler_dispDtoI3ddma_set_enable(FALSE);
		return ;
	}
	rtd_pr_i3ddma_debug("[scart out]@@@@@@@@@@SCART_OUT_INIT_READY!!\n");
}
#endif





//#ifdef DTV_SCART_OUT_ENABLE
static unsigned char vo_se_stretch_scaling_enable=FALSE;
extern unsigned int tve_new_linestep;

unsigned char vo_se_scaling_get_enable(void)
{
	return vo_se_stretch_scaling_enable;
}
void vo_se_scaling_set_enable(unsigned char enable)
{
	vo_se_stretch_scaling_enable=enable;
}

void VO_SE_Scaling(bool enable)
{
#if 0
	SCALER_DISPLAY_WINDOW *display_window;
	//unsigned int *pulTemp = NULL;
	unsigned int ulCount = 0;//, i = 0;
	int ret;

	rtd_pr_i3ddma_debug("[scart out]###############%s~~1 \n",__FUNCTION__);

	vo_se_scaling_set_enable(enable);

	display_window = (SCALER_DISPLAY_WINDOW *)Scaler_GetShareMemVirAddr(SCALERIOC_VO_SE_SCALING_ENABLE);
	ulCount = sizeof(SCALER_DISPLAY_WINDOW) / sizeof(unsigned int);

	display_window->outputWidth=720;
    	display_window->outputHeight=576;
	display_window->tve_linestep = tve_new_linestep;
	display_window->vo_scaling_enable = 0;//fScartOut_VO_scaling_enable;
	display_window->vo_scaling_alloc_mem_enable= enable;
	display_window->tve_vflip_en = scaler_get_tve_vflip_enable();
        	display_window->vo_clock_modify = fScartOut_VO_clock_check;
	display_window->scart_isr_enable = scaler_scart_out_isr_get_enable();

	//change endian
	display_window->outputWidth = htonl(display_window->outputWidth);
	display_window->outputHeight = htonl(display_window->outputHeight);
	display_window->tve_linestep = htonl(display_window->tve_linestep);
	display_window->vo_scaling_enable = htonl(display_window->vo_scaling_enable);
	display_window->vo_scaling_alloc_mem_enable = htonl(display_window->vo_scaling_alloc_mem_enable);
	display_window->tve_vflip_en = htonl(display_window->tve_vflip_en);
	display_window->vo_clock_modify = htonl(display_window->vo_clock_modify);
	display_window->scart_isr_enable = htonl(display_window->scart_isr_enable);

	if (0 != (ret = Scaler_SendRPC(SCALERIOC_VO_SE_SCALING_ENABLE,0,0))){
		rtd_pr_i3ddma_debug("[scart out]ret=%d, Deinit VO ROTATE fail !!!\n", ret);
	}
#endif
	//tve isr is not ready @ 20140930
	//if(Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_THRIP))
		{
		//drvif_i3ddma_vactive_end_irq(true);
		drvif_scaler_vactive_end_irq(_ENABLE, SLR_SUB_DISPLAY);
		drvif_scaler_vactive_sta_irq(_ENABLE, SLR_SUB_DISPLAY);
		//drvif_module_tve_interrupt(true);
	}
	rtd_pr_i3ddma_debug("[scart out]###############%s~~3\n",__FUNCTION__);
}

void vo_se_scaling_ctrl(int enable)
{
    	VO_SE_Scaling(enable);
}

#ifdef CONFIG_DTV_SCART_OUT_ENABLE_444_FORMAT
long Scaler_VO_Set_VoForce422_enable(unsigned char bEnable)
{
	long ret = 0;

	rtd_pr_i3ddma_debug("[scart out]###############%s\n",__FUNCTION__);
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
#ifndef CONFIG_SCALER_BRING_UP
	if (send_rpc_command(RPC_VIDEO, VIDEO_RPC_VOUT_ToAgent_Force_VO_Chroma_422, bEnable, 0, &ret))
		rtd_pr_i3ddma_debug("RPC fail!!\n");
#endif
#endif

	return ret;
}
#endif



//#endif

static void I3DDMA_Set_MHL_Vgip(unsigned char src,  unsigned char mode)
{
	//dma_vgip_ctrl_RBUS dma_vgip_chn1_ctrl_reg;
	dma_vgip_dma_vgip_ctrl_RBUS dma_vgip_chn1_ctrl_reg;
	//dma_vgip_act_hsta_width_RBUS    dma_vgip_chn1_act_hsta_width_reg;
	dma_vgip_dma_vgip_act_hsta_width_RBUS dma_vgip_chn1_act_hsta_width_reg;
	//dma_vgip_act_vsta_length_RBUS   dma_vgip_chn1_act_vsta_length_reg;
	dma_vgip_dma_vgip_act_vsta_length_RBUS dma_vgip_chn1_act_vsta_length_reg;
	//dma_vgip_delay_RBUS dma_vgip_chn1_delay_reg;
	dma_vgip_dma_vgip_delay_RBUS  dma_vgip_chn1_delay_reg;
	//dma_vgip_cts_fifo_ctl_RBUS 			 	 dma_cts_fifo_ctl_reg;
	dma_vgip_dma_vgip_cts_fifo_ctl_RBUS  dma_cts_fifo_ctl_reg;
	//int dispD_hstaOffset=0;

	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_in_clk_en = 0;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);

	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_in_sel = src;
	dma_vgip_chn1_ctrl_reg.dma_digital_mode = mode;

	dma_vgip_chn1_ctrl_reg.dma_field_det_en = 0;

	dma_vgip_chn1_ctrl_reg.dma_field_inv = 1;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);

	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_in_clk_en = 1;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);


	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_ivrun = 1;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);


	dma_vgip_chn1_act_hsta_width_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg);
	dma_vgip_chn1_act_vsta_length_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_ACT_VSTA_Length_reg);

	dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_sta = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_STA_PRE);
	dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_wid = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE);
	dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_sta = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA_PRE);
	dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_len= Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE);

	IoReg_Write32(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg, dma_vgip_chn1_act_hsta_width_reg.regValue);
	IoReg_Write32(DMA_VGIP_DMA_VGIP_ACT_VSTA_Length_reg, dma_vgip_chn1_act_vsta_length_reg.regValue);

	// IHS/IVS delay
	dma_vgip_chn1_delay_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_DELAY_reg);
	dma_vgip_chn1_delay_reg.dma_ihs_dly = 0;
	dma_vgip_chn1_delay_reg.dma_ivs_dly = 0;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_DELAY_reg, dma_vgip_chn1_delay_reg.regValue);

	dma_cts_fifo_ctl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTS_FIFO_CTL_reg);
	dma_cts_fifo_ctl_reg.dma_en_ctsfifo_vsrst_vgip = 1;
	dma_cts_fifo_ctl_reg.dma_force_ctsfifo_rstn_vgip = 1;
	dma_cts_fifo_ctl_reg.dma_force_ctsfifo_rstn_sdnr = 1;
	dma_cts_fifo_ctl_reg.dma_en_ctsfifo_vsrst_sdnr = 0;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTS_FIFO_CTL_reg, dma_cts_fifo_ctl_reg.regValue);

	return;
}

unsigned char drvif_rpc_confighdmi(void)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
#ifndef CONFIG_SCALER_BRING_UP
	unsigned long result = 0;
#endif
#endif
	VIDEO_RPC_VOUT_CONFIG_HDMI_3D *config_data;

	//struct page *page = 0;
	//page = alloc_page(GFP_KERNEL);
	//config_datatmp = (VIDEO_RPC_VOUT_SET_3D_MODE *)page_address(page);
	unsigned long vir_addr, vir_addr_noncache = 0;
	unsigned int phy_addr;

	vir_addr = (unsigned long)dvr_malloc_uncached_specific(sizeof(VIDEO_RPC_VOUT_CONFIG_HDMI_3D), GFP_DCU1, (void **)&vir_addr_noncache);
	//phy_addr = (unsigned int)virt_to_phys((void*)vir_addr);
	phy_addr = (unsigned int)dvr_to_phys((void*)vir_addr);

	config_data = (VIDEO_RPC_VOUT_CONFIG_HDMI_3D *)vir_addr_noncache;

	memset(config_data, 0, sizeof(VIDEO_RPC_VOUT_CONFIG_HDMI_3D));
	config_data->width = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE);
	config_data->height = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE);
	config_data->chroma_fmt = VO_CHROMA_YUV422;//(vgip2CtrlInfo.srcInfo.chroma_422Cap? VO_CHROMA_YUV422: VO_CHROMA_YUV444);
	config_data->data_bit_width = 8;
	config_data->framerate =  Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) * 100;
	config_data->progressive = Scaler_DispGetStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_DISP_INTERLACE)?0:1;
	config_data->l1_st_adr = rtd_inl(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_0_reg);
	config_data->r1_st_adr = rtd_inl(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_0_reg);
	config_data->l2_st_adr = rtd_inl(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_1_reg);
	config_data->r2_st_adr = rtd_inl(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_1_reg);
	config_data->hw_auto_mode = 1;
	config_data->quincunx_en = 0;
	config_data->videoPlane = VO_VIDEO_PLANE_V1;

	config_data->width = htonl(config_data->width);
	config_data->height = htonl(config_data->height);
	config_data->chroma_fmt = htonl(config_data->chroma_fmt);
	config_data->data_bit_width = htonl(config_data->data_bit_width);
	config_data->framerate =  htonl(config_data->framerate);
	config_data->progressive = htonl(config_data->progressive);
	config_data->l1_st_adr = htonl(config_data->l1_st_adr);
	config_data->r1_st_adr = htonl(config_data->r1_st_adr);
	config_data->l2_st_adr = htonl(config_data->l2_st_adr);
	config_data->r2_st_adr = htonl(config_data->r2_st_adr);
	config_data->hw_auto_mode = htonl(config_data->hw_auto_mode);
	config_data->quincunx_en = htonl(config_data->quincunx_en);
	config_data->videoPlane = htonl(config_data->videoPlane);

#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
#ifndef CONFIG_SCALER_BRING_UP
	if (RPC_FAIL==send_rpc_command(RPC_VIDEO, VIDEO_RPC_VOUT_ToAgent_ConfigHDMI_3D,phy_addr, 0, &result))
	{
		rtd_pr_i3ddma_debug("RPC fail!!\n");
		dvr_free((void *)vir_addr);
		return FALSE;
	}
#endif
#endif
	dvr_free((void *)vir_addr);
	return TRUE;
}

void scaler_mhl_i3ddma_config(void)
{
	//unsigned char nMode;
	unsigned char source;
	unsigned char mode;
	SIZE insize;
	SIZE outsize;
	//StructDisplayInfo *info;

	source = VGIP_SRC_TMDS;
	mode = 0;

	I3DDMA_Set_MHL_Vgip(source, mode);

	// VGIP2 color format configure
	//info = Scaler_DispGetInfoPtr();
	//nMode = (info->IPV_ACT_LEN > 480? 1: 0);

	insize.nWidth = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE);
	insize.nLength = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE);
	outsize.nWidth = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE);
	outsize.nLength = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE);

	Scaler_DispSetScaleStatus(SLR_SUB_DISPLAY, SLR_SCALE_H_DOWN, (insize.nWidth > outsize.nWidth)?true:false);
	Scaler_DispSetScaleStatus(SLR_SUB_DISPLAY, SLR_SCALE_V_DOWN, (insize.nLength > outsize.nLength)?true:false);

	Scaler_DispSetStatus(SLR_SUB_DISPLAY, SLR_DISP_10BIT, false);
	Scaler_DispSetStatus(SLR_SUB_DISPLAY, SLR_DISP_FSYNC_VUZD, 0x2);
	Scaler_DispSetStatus(SLR_SUB_DISPLAY, SLR_DISP_COMP, false);
	Scaler_DispSetStatus(SLR_SUB_DISPLAY, SLR_DISP_422CAP, true);

//@@@@@@@@@@@@@@@@@@@@@ MUST FIX ME @@@@@@@
/*	if(GET_HDMI_COLOR_SPACE() == COLOR_RGB)
		I3DDMA_ColorSpacergb2yuvtransfer(_SRC_HDMI, nMode, 0);
*/
//@@@@@@@@@@@@@@@@@@@@@ MUST FIX ME @@@@@@@
	I3DDMA_ultrazoom_config_scaling_down(&insize, &outsize, 1);

	drvif_I3DDMA_set_capture_Mhl(&outsize);

	drvif_rpc_confighdmi();
	return;
}

void I3DDMA_Set_HDMI3D_Vgip(void)
{
	//dma_vgip_ctrl_RBUS dma_vgip_chn1_ctrl_reg;
	dma_vgip_dma_vgip_ctrl_RBUS dma_vgip_chn1_ctrl_reg;
	//dma_vgip_act_hsta_width_RBUS    dma_vgip_chn1_act_hsta_width_reg;
	dma_vgip_dma_vgip_act_hsta_width_RBUS dma_vgip_chn1_act_hsta_width_reg;
	//dma_vgip_act_vsta_length_RBUS   dma_vgip_chn1_act_vsta_length_reg;
	dma_vgip_dma_vgip_act_vsta_length_RBUS dma_vgip_chn1_act_vsta_length_reg;
	//dma_vgip_delay_RBUS dma_vgip_chn1_delay_reg;
	dma_vgip_dma_vgip_delay_RBUS  dma_vgip_chn1_delay_reg;
	//dma_vgip_cts_fifo_ctl_RBUS 			 	 dma_cts_fifo_ctl_reg;
	dma_vgip_dma_vgip_cts_fifo_ctl_RBUS  dma_cts_fifo_ctl_reg;
	//int dispD_hstaOffset=0;

	unsigned char mode =VGIP_MODE_ANALOG;
	unsigned char src =VGIP_SRC_TMDS;

	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_in_clk_en = 0;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);

	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_in_sel = src;
	dma_vgip_chn1_ctrl_reg.dma_digital_mode = mode;

	dma_vgip_chn1_ctrl_reg.dma_field_det_en = 0;

	dma_vgip_chn1_ctrl_reg.dma_field_inv = 1;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);

	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_in_clk_en = 1;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);


	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_ivrun = 1;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);

	// enable IDMA VGIP data end interrupt
	if(get_HDMI_HDR_mode() == HDR_DOLBY_HDMI){
		dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
		dma_vgip_chn1_ctrl_reg.dma_vact_end_ie= 1;
		IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);
	}

	dma_vgip_chn1_act_hsta_width_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg);
	dma_vgip_chn1_act_vsta_length_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_ACT_VSTA_Length_reg);

	dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_sta = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_STA_PRE);
	dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_wid = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE);
	dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_sta = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA_PRE);
	dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_len= Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE);

	IoReg_Write32(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg, dma_vgip_chn1_act_hsta_width_reg.regValue);
	IoReg_Write32(DMA_VGIP_DMA_VGIP_ACT_VSTA_Length_reg, dma_vgip_chn1_act_vsta_length_reg.regValue);

	// IHS/IVS delay
	dma_vgip_chn1_delay_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_DELAY_reg);
	dma_vgip_chn1_delay_reg.dma_ihs_dly = 0;
	dma_vgip_chn1_delay_reg.dma_ivs_dly = 0;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_DELAY_reg, dma_vgip_chn1_delay_reg.regValue);

	dma_cts_fifo_ctl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTS_FIFO_CTL_reg);
	dma_cts_fifo_ctl_reg.dma_en_ctsfifo_vsrst_vgip = 1;
	dma_cts_fifo_ctl_reg.dma_force_ctsfifo_rstn_vgip = 1;
	dma_cts_fifo_ctl_reg.dma_force_ctsfifo_rstn_sdnr = 1;
	dma_cts_fifo_ctl_reg.dma_en_ctsfifo_vsrst_sdnr = 0;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTS_FIFO_CTL_reg, dma_cts_fifo_ctl_reg.regValue);

	return;
}


void I3DDMA_Set_Metadata_Vgip(unsigned char enable)
{
	//dma_vgip_ctrl_RBUS dma_vgip_chn1_ctrl_reg;
	dma_vgip_dma_vgip_ctrl_RBUS     dma_vgip_chn1_ctrl_reg;
	//dma_vgip_act_hsta_width_RBUS    dma_vgip_chn1_act_hsta_width_reg;
	//dma_vgip_dma_vgip_act_hsta_width_RBUS dma_vgip_chn1_act_hsta_width_reg;
	//dma_vgip_act_vsta_length_RBUS   dma_vgip_chn1_act_vsta_length_reg;
	//dma_vgip_dma_vgip_act_vsta_length_RBUS dma_vgip_chn1_act_vsta_length_reg;
	//dma_vgip_delay_RBUS dma_vgip_chn1_delay_reg;
	dma_vgip_dma_vgip_delay_RBUS  dma_vgip_chn1_delay_reg;
	//dma_vgip_cts_fifo_ctl_RBUS 			 	 dma_cts_fifo_ctl_reg;
	dma_vgip_dma_vgip_cts_fifo_ctl_RBUS  dma_cts_fifo_ctl_reg;
	//int dispD_hstaOffset=0;

	unsigned char mode =VGIP_MODE_ANALOG;
	unsigned char src =VGIP_SRC_TMDS;

	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_in_clk_en = 0;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);

	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_in_sel = src;
	dma_vgip_chn1_ctrl_reg.dma_digital_mode = mode;

	dma_vgip_chn1_ctrl_reg.dma_field_det_en = 0;

	dma_vgip_chn1_ctrl_reg.dma_field_inv = 1;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);

	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_in_clk_en = 1;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);


	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_ivrun = 1;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);

	// enable IDMA VGIP data end interrupt
	if(get_HDMI_HDR_mode() == HDR_DOLBY_HDMI){
		dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
		dma_vgip_chn1_ctrl_reg.dma_vact_end_ie= 1;
		IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);
	}

	//dma_vgip_chn1_act_hsta_width_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg);
	//dma_vgip_chn1_act_vsta_length_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_ACT_VSTA_Length_reg);
//@@@@@@@MUST FIX ME@@@@@@@@@@@@@@@@@@@
	//dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_sta = GET_MODE_IHSTA();//Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_STA_PRE);
	//dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_wid = GET_MODE_IHWIDTH();//Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE);
	//dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_sta = GET_MODE_IVSTA();//Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA_PRE);
	//dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_len= GET_MODE_IVHEIGHT();//Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE);

	//IoReg_Write32(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg, dma_vgip_chn1_act_hsta_width_reg.regValue);
	//IoReg_Write32(DMA_VGIP_DMA_VGIP_ACT_VSTA_Length_reg, dma_vgip_chn1_act_vsta_length_reg.regValue);
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// IHS/IVS delay
	dma_vgip_chn1_delay_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_DELAY_reg);
	dma_vgip_chn1_delay_reg.dma_ihs_dly = 0;
	dma_vgip_chn1_delay_reg.dma_ivs_dly = 0;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_DELAY_reg, dma_vgip_chn1_delay_reg.regValue);

	dma_cts_fifo_ctl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTS_FIFO_CTL_reg);
	dma_cts_fifo_ctl_reg.dma_en_ctsfifo_vsrst_vgip = 1;
	dma_cts_fifo_ctl_reg.dma_force_ctsfifo_rstn_vgip = 1;
	dma_cts_fifo_ctl_reg.dma_force_ctsfifo_rstn_sdnr = 1;
	dma_cts_fifo_ctl_reg.dma_en_ctsfifo_vsrst_sdnr = 0;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTS_FIFO_CTL_reg, dma_cts_fifo_ctl_reg.regValue);

	return;
}

void I3DDMA_Parse_HDMI3D_pattern(void)
{
       //unsigned char nMode;
       SIZE insize;
       SIZE outsize;
       //StructDisplayInfo *info;

       I3DDMA_Set_HDMI3D_Vgip();

       // VGIP2 color format configure
       //info = Scaler_DispGetInfoPtr();
       //nMode = (info->IPV_ACT_LEN > 480? 1: 0);

       insize.nWidth = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE);
       insize.nLength = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE);
       outsize.nWidth = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE);
       outsize.nLength = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE);

       Scaler_DispSetScaleStatus(SLR_SUB_DISPLAY, SLR_SCALE_H_DOWN, (insize.nWidth > outsize.nWidth)?true:false);
       Scaler_DispSetScaleStatus(SLR_SUB_DISPLAY, SLR_SCALE_V_DOWN, (insize.nLength > outsize.nLength)?true:false);

       Scaler_DispSetStatus(SLR_SUB_DISPLAY, SLR_DISP_10BIT, false);
       Scaler_DispSetStatus(SLR_SUB_DISPLAY, SLR_DISP_FSYNC_VUZD, 0x2);
       Scaler_DispSetStatus(SLR_SUB_DISPLAY, SLR_DISP_COMP, false);
       Scaler_DispSetStatus(SLR_SUB_DISPLAY, SLR_DISP_422CAP, true);

//@@@@@@@@@@@@@@@@@@@@@ MUST FIX ME @@@@@@@
/*       if(GET_HDMI_COLOR_SPACE() == COLOR_RGB)
               I3DDMA_ColorSpacergb2yuvtransfer(_SRC_HDMI, nMode, 0);
   */
//@@@@@@@@@@@@@@@@@@@@@ MUST FIX ME @@@@@@@

       I3DDMA_ultrazoom_config_scaling_down(&insize, &outsize, 1);

       drvif_I3DDMA_set_capture_Mhl(&outsize);

       return;
}
#endif

void fw_i3ddma_set_vgip(unsigned char src, unsigned char mode)
{
	//dma_vgip_dma_vgip_htotal_RBUS dma_vgip_dma_vgip_htotal_reg;
	dma_vgip_dma_vgip_ctrl_RBUS dma_vgip_chn1_ctrl_reg;
	dma_vgip_dma_vgip_act_hsta_width_RBUS dma_vgip_chn1_act_hsta_width_reg;
	dma_vgip_dma_vgip_act_vsta_length_RBUS dma_vgip_chn1_act_vsta_length_reg;
	dma_vgip_dma_vgip_delay_RBUS dma_vgip_chn1_delay_reg;
	dma_vgip_dma_vgip_cts_fifo_ctl_RBUS dma_cts_fifo_ctl_reg;
	dma_vgip_dma_vgip_misc_RBUS dma_vgip_dma_vgip_misc_reg;
	//int dispD_hstaOffset=0;
	RBus_UInt32 u32_cal_ivs_dly= 0;	//default
	unsigned int polarity=0;

#ifdef CONFIG_SUPPORT_SRC_ADC
	StructDisplayInfo *p_dispinfo;
	p_dispinfo = Get_ADC_Dispinfo();

	if(p_dispinfo ==NULL){
		rtd_pr_i3ddma_err("#####[%s(%d)] error must connect ADC!!!", __FUNCTION__, __LINE__);
		return ;
	}
#endif

	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_in_clk_en = 0;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);

	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_in_sel = src;
	dma_vgip_chn1_ctrl_reg.dma_digital_mode = mode;

	//modify the field setting by liyu chen suggestion
	dma_vgip_chn1_ctrl_reg.dma_field_det_en = 0;	//bit16
	dma_vgip_chn1_ctrl_reg.dma_field_sync_edge = 1;	//bit17
	dma_vgip_chn1_ctrl_reg.dma_field_inv = 0;		//bit18

	if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY)  == VSC_INPUTSRC_HDMI) && (Get_Val_DP_Vsync_Regen_Pos() == DP_NO_REGEN_VSYNC))
	{
		polarity=Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_V_SYNC_HIGH_PERIOD);//fix K5LG-1397

		if ((polarity>>1)&0x01)
			dma_vgip_chn1_ctrl_reg.dma_vs_inv = 0;
		else
			dma_vgip_chn1_ctrl_reg.dma_vs_inv = 1;

		if (polarity&0x01)
			dma_vgip_chn1_ctrl_reg.dma_hs_inv = 0;
		else
			dma_vgip_chn1_ctrl_reg.dma_hs_inv = 1;
	}

	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);

	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_in_clk_en = 1;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);


	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_ivrun = 1;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);

	// enable IDMA VGIP data end interrupt
	if(get_HDMI_HDR_mode() == HDR_DOLBY_HDMI){
		dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
		dma_vgip_chn1_ctrl_reg.dma_vact_end_ie= 1;
		IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);
	}

#if 0//no need
	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	if(dma_vgip_chn1_ctrl_reg.dma_digital_mode)
	{//digital mode
		dma_vgip_chn1_ctrl_reg.dma_vs_by_hs_en_n = 1;
	}
	else
	{
		dma_vgip_chn1_ctrl_reg.dma_vs_by_hs_en_n = 0;
	}
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);
#endif

	if(mode == VGIP_MODE_ANALOG)
	{
#ifndef BUILD_QUICK_SHOW
		if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_AVD))	//av
		{
			if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_SYNC_HIGH_PERIOD)==0)	//0 => inverse
			{
				u32_cal_ivs_dly = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_COUNT) - Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IVSYNCPULSECOUNT) + 1;
			}
			else
			{
				u32_cal_ivs_dly = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IVSYNCPULSECOUNT) + 1;
			}
		}
		else if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_ADC))	//ypo
#endif
		{
			if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_SYNC_HIGH_PERIOD)==0)	//0 => inverse
			{
				u32_cal_ivs_dly = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_COUNT) - Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IVSYNCPULSECOUNT) + 1;
			}
			else
			{
				u32_cal_ivs_dly = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IVSYNCPULSECOUNT) + 1;
			}
		}
	}
	else if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI))	//hdmi
	{
		if(Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE))
		{//only interlace need delay
			if((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_SYNC_HIGH_PERIOD)&_BIT1)==0)	//b'00 => inverse
			{
				u32_cal_ivs_dly = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_COUNT) - Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IVSYNCPULSECOUNT) + 1;
			}
			else	// b'10
			{
				u32_cal_ivs_dly = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IVSYNCPULSECOUNT);	//already add in offline measure
			}
		}
	}
	else if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_DP))	//dprx
	{
		rtd_pr_i3ddma_notice("dprx re-gen vsync!!!\n");
	}
	else
	{
		//error
		rtd_pr_i3ddma_warn("Warning: dma source incorrect %d\n", mode);
	}

	if(u32_cal_ivs_dly>8)	//protection
	{
		rtd_pr_ioreg_emerg("%s(%d)-%s: Warning:dma delay incorrect=%d \n",__FILE__,__LINE__,__FUNCTION__,u32_cal_ivs_dly);
		u32_cal_ivs_dly = 8;
	}

	if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_COUNT)==0 && Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IVSYNCPULSECOUNT)==0)
	{	//fail to get offline measure data protection
		u32_cal_ivs_dly =0;
	}

	dma_vgip_chn1_act_hsta_width_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg);
	dma_vgip_chn1_act_vsta_length_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_ACT_VSTA_Length_reg);
#ifndef BUILD_QUICK_SHOW
#ifdef CONFIG_SUPPORT_SRC_ADC
	if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_ADC) {
		/*ADC Analog mode: Real start = X+2*/
		dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_sta = (mode == VGIP_MODE_ANALOG) ? (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPH_ACT_STA_PRE)+2) : 0;
	}
    else
#endif//#ifdef CONFIG_SUPPORT_SRC_ADC
#endif
    {
		dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_sta = (mode == VGIP_MODE_ANALOG) ? Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPH_ACT_STA_PRE) : 0;
	}
	dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_wid = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPH_ACT_WID_PRE);
#ifndef BUILD_QUICK_SHOW
    if(get_i3ddma_4k120_flag() == TRUE)
	{
		dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_wid = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPH_ACT_WID_PRE) / 2;
	}
#endif
	dma_vgip_dma_vgip_misc_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_MISC_reg);
	//dma_vgip_dma_vgip_htotal_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_HTOTAL_reg);
	if(dma_vgip_chn1_ctrl_reg.dma_digital_mode == VGIP_MODE_ANALOG)
	{
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_STA_PRE)>=u32_cal_ivs_dly)	// according liyu chen suggestion
		{
			dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_sta= Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_STA_PRE)-u32_cal_ivs_dly;
		}
		else
		{
			dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_sta= Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_STA_PRE);
			rtd_pr_i3ddma_warn("Warning: i3ddma active start(%d) is smaller than %d\n", dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_sta, u32_cal_ivs_dly);
		}
		dma_vgip_dma_vgip_misc_reg.dma_hporch_num = 0;
		//dma_vgip_dma_vgip_misc_reg.dma_hs_mode = 1;//hs mode 
		//dma_vgip_dma_vgip_htotal_reg.dma_htotal_num = 0;//set h toal 
	}else {//digital mode vstart need to be 0
		dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_sta = 0;
		dma_vgip_dma_vgip_misc_reg.dma_hporch_num = 0;	//no need to set h porch number for no hs mode
		//dma_vgip_dma_vgip_misc_reg.dma_hs_mode = 0;//no hs mode 
		//dma_vgip_dma_vgip_htotal_reg.dma_htotal_num = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_H_LEN) - 1;//set h toal 
#ifndef BUILD_QUICK_SHOW
        if(get_i3ddma_4k120_flag() == TRUE)
		{
			dma_vgip_dma_vgip_misc_reg.dma_v1_v3_swap = 1;
			dma_vgip_dma_vgip_misc_reg.dma_v2_v3_swap = 1;
		}
		else
#endif
		{
			dma_vgip_dma_vgip_misc_reg.dma_v1_v3_swap = 0;
			dma_vgip_dma_vgip_misc_reg.dma_v2_v3_swap = 0;
		}
    }
	dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_len= Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE);

	//v start lenght keeps no change, if digital mode.

	IoReg_Write32(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg, dma_vgip_chn1_act_hsta_width_reg.regValue);
	IoReg_Write32(DMA_VGIP_DMA_VGIP_ACT_VSTA_Length_reg, dma_vgip_chn1_act_vsta_length_reg.regValue);
	IoReg_Write32(DMA_VGIP_DMA_VGIP_MISC_reg, dma_vgip_dma_vgip_misc_reg.regValue);
	//IoReg_Write32(DMA_VGIP_DMA_VGIP_HTOTAL_reg, dma_vgip_dma_vgip_htotal_reg.regValue);

	// IHS/IVS delay
	dma_vgip_chn1_delay_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_DELAY_reg);
#ifndef BUILD_QUICK_SHOW
#ifdef CONFIG_SUPPORT_SRC_ADC
	if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_ADC) && (get_ADC_Input_Source() == _SRC_VGA))
		dma_vgip_chn1_delay_reg.dma_ihs_dly = FIX_VGIP_H_DELAY;
	else
#endif//#ifdef CONFIG_SUPPORT_SRC_ADC
#endif
		dma_vgip_chn1_delay_reg.dma_ihs_dly = 0;
	dma_vgip_chn1_delay_reg.dma_ivs_dly = u32_cal_ivs_dly;	// according liyu chen suggestion
	IoReg_Write32(DMA_VGIP_DMA_VGIP_DELAY_reg, dma_vgip_chn1_delay_reg.regValue);

	if(src == VGIP_SRC_ATV)
	{
		dma_vgip_chn1_delay_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_DELAY_reg);
		if(VD_27M_OFF != fw_video_get_27mhz_mode((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)))
		{
			dma_vgip_chn1_delay_reg.dma_ihs_dly = _H_POSITION_MOVE_TOLERANCE * 2;
		}
		else
		{
			dma_vgip_chn1_delay_reg.dma_ihs_dly = _H_POSITION_MOVE_TOLERANCE;
		}

		IoReg_Write32(DMA_VGIP_DMA_VGIP_DELAY_reg, dma_vgip_chn1_delay_reg.regValue);

		dma_vgip_chn1_act_hsta_width_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg);
		dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_sta = dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_sta - dma_vgip_chn1_delay_reg.dma_ihs_dly;
		IoReg_Write32(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg, dma_vgip_chn1_act_hsta_width_reg.regValue);
	}

#ifdef CONFIG_SUPPORT_SRC_ADC
	if ((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_ADC) && (get_ADC_Input_Source() == _SRC_VGA)){
		Scaler_SetHPosition(Scaler_GetHPosition());
		Scaler_SetVPosition(Scaler_GetVPosition());
	}
#endif//#ifdef CONFIG_SUPPORT_SRC_ADC

	if(dvrif_i3ddma_compression_get_enable()) {
		// if((src == VGIP_SRC_ATV) || (src == VGIP_SRC_ADC)) {
		if(src == VGIP_SRC_ATV) {
			dma_vgip_chn1_act_hsta_width_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg);
			/*analog mode enable pqc, if h_start>32 do (h_delay +=32, h_start -=32)*/
			if(dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_sta > 32) {
				dma_vgip_chn1_delay_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_DELAY_reg);
				dma_vgip_chn1_delay_reg.dma_ihs_dly = dma_vgip_chn1_delay_reg.dma_ihs_dly + 32;
				IoReg_Write32(DMA_VGIP_DMA_VGIP_DELAY_reg, dma_vgip_chn1_delay_reg.regValue);
				dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_sta = dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_sta - 32;
				IoReg_Write32(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg, dma_vgip_chn1_act_hsta_width_reg.regValue);
			}
		}
		else if(src == VGIP_SRC_TMDS) {
			dma_vgip_chn1_delay_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_DELAY_reg);
			dma_vgip_chn1_delay_reg.dma_ihs_dly = 32;
			IoReg_Write32(DMA_VGIP_DMA_VGIP_DELAY_reg, dma_vgip_chn1_delay_reg.regValue);
		}
		else if(src == VGIP_SRC_DPRX) {
			dma_vgip_chn1_delay_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_DELAY_reg);
			dma_vgip_chn1_delay_reg.dma_ihs_dly = 32;
			IoReg_Write32(DMA_VGIP_DMA_VGIP_DELAY_reg, dma_vgip_chn1_delay_reg.regValue);
		}
	}

	dma_cts_fifo_ctl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTS_FIFO_CTL_reg);
	dma_cts_fifo_ctl_reg.dma_en_ctsfifo_vsrst_vgip = 1;
	dma_cts_fifo_ctl_reg.dma_force_ctsfifo_rstn_vgip = 1;
	dma_cts_fifo_ctl_reg.dma_force_ctsfifo_rstn_sdnr = 1;
	dma_cts_fifo_ctl_reg.dma_en_ctsfifo_vsrst_sdnr = 0;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTS_FIFO_CTL_reg, dma_cts_fifo_ctl_reg.regValue);

	return;
}

void fw_scaler_ctrl_dma_vgip_regen_vsync(unsigned char enable)
{
	dma_vgip_dma_vgip_regen_vs_ctrl1_RBUS dma_vgip_dma_vgip_regen_vs_ctrl1_reg;
	dma_vgip_dma_vgip_regen_vs_ctrl2_RBUS dma_vgip_dma_vgip_regen_vs_ctrl2_reg;

	unsigned int input_src_htotal = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_H_LEN);
	unsigned int input_src_vtotal = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_LEN);
	unsigned int input_src_v_act  = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE);

	if(enable) {
		if((input_src_vtotal - input_src_v_act) < FAKE_PORCH_MIN_V_BLANK) {
			rtd_pr_i3ddma_info("#####[%s(%d)]Abnormal timing can not enable fake vsync!\n", __FUNCTION__, __LINE__);
			return;
		}
		Set_Val_DP_Vsync_Regen_Pos(DP_REGEN_VSYNC_AT_DMA_VGIP);
		dma_vgip_dma_vgip_regen_vs_ctrl1_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_REGEN_VS_CTRL1_reg);
		dma_vgip_dma_vgip_regen_vs_ctrl1_reg.dma_fake_porch_thr_line = FAKE_PORCH_THR_LINE;
		IoReg_Write32(DMA_VGIP_DMA_VGIP_REGEN_VS_CTRL1_reg, dma_vgip_dma_vgip_regen_vs_ctrl1_reg.regValue);

		dma_vgip_dma_vgip_regen_vs_ctrl2_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_REGEN_VS_CTRL2_reg);
		dma_vgip_dma_vgip_regen_vs_ctrl2_reg.dma_fake_porch_en = enable;
		dma_vgip_dma_vgip_regen_vs_ctrl2_reg.dma_fake_porch_sel = 1;
		dma_vgip_dma_vgip_regen_vs_ctrl2_reg.dma_fake_vs_width = 2;
		dma_vgip_dma_vgip_regen_vs_ctrl2_reg.dma_fake_porch_line_total = input_src_htotal;
		IoReg_Write32(DMA_VGIP_DMA_VGIP_REGEN_VS_CTRL2_reg, dma_vgip_dma_vgip_regen_vs_ctrl2_reg.regValue);
	} else {
		dma_vgip_dma_vgip_regen_vs_ctrl2_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_REGEN_VS_CTRL2_reg);
		dma_vgip_dma_vgip_regen_vs_ctrl2_reg.dma_fake_porch_en = 0;
		dma_vgip_dma_vgip_regen_vs_ctrl2_reg.dma_fake_porch_sel = 0;
		IoReg_Write32(DMA_VGIP_DMA_VGIP_REGEN_VS_CTRL2_reg, dma_vgip_dma_vgip_regen_vs_ctrl2_reg.regValue);

		I3DDMA_reset_i3tovo_vs_sel(); // reset i3tovo_vs_sel
	}
}

#ifndef BUILD_QUICK_SHOW
void avd_start_compensation(void)
{//for avd i3ddma start compensation, because final state maybe need compensation
	extern StructDisplayInfo *Get_AVD_ScalerDispinfo(void);
	extern struct semaphore* get_vdc_detectsemaphore(void);
	extern unsigned char get_avd_start_need_compensation(void);
	extern void set_avd_start_need_compensation(unsigned char TorF);
	StructDisplayInfo *avd_info = NULL;
	dma_vgip_dma_vgip_act_hsta_width_RBUS dma_vgip_act_hsta_width_reg;
	dma_vgip_dma_vgip_act_vsta_length_RBUS dma_vgip_act_vsta_length_reg;
	dma_vgip_dma_vgip_delay_RBUS dma_vgip_chn1_delay_reg;
	if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) != VSC_INPUTSRC_AVD)
		return;
	if(!get_avd_start_need_compensation())//no need compensation
		return;
	set_avd_start_need_compensation(FALSE);
	dma_vgip_chn1_delay_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_DELAY_reg);
	dma_vgip_act_hsta_width_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg);
	dma_vgip_act_vsta_length_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_ACT_VSTA_Length_reg);
	avd_info =  Get_AVD_ScalerDispinfo();
	if((avd_info->IPH_ACT_STA_PRE != (dma_vgip_chn1_delay_reg.dma_ihs_dly + dma_vgip_act_hsta_width_reg.dma_ih_act_sta)) ||
		(avd_info->IPV_ACT_STA_PRE != (dma_vgip_chn1_delay_reg.dma_ivs_dly + dma_vgip_act_vsta_length_reg.dma_iv_act_sta))
		)
	{
		down(get_vdc_detectsemaphore());
		dma_vgip_chn1_delay_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_DELAY_reg);
		dma_vgip_act_hsta_width_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg);
		dma_vgip_act_hsta_width_reg.dma_ih_act_sta = avd_info->IPH_ACT_STA_PRE - dma_vgip_chn1_delay_reg.dma_ihs_dly;
		IoReg_Write32(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg, dma_vgip_act_hsta_width_reg.regValue);
		dma_vgip_act_vsta_length_reg.regValue 	= IoReg_Read32(DMA_VGIP_DMA_VGIP_ACT_VSTA_Length_reg);
		dma_vgip_act_vsta_length_reg.dma_iv_act_sta = avd_info->IPV_ACT_STA_PRE - dma_vgip_chn1_delay_reg.dma_ivs_dly;
		IoReg_Write32(DMA_VGIP_DMA_VGIP_ACT_VSTA_Length_reg, dma_vgip_act_vsta_length_reg.regValue);
		up(get_vdc_detectsemaphore());
		rtd_pr_i3ddma_notice("#####[%s(%d)]run avd_start_compensation####\n",__func__,__LINE__);
	}
}


/*merlin4 new function*/
//Re-order control for HDMI Dolby
void drvif_I3DDMA_set_reorder_mode(unsigned char enable)
{
	h3ddma_i3ddma_enable_RBUS i3ddma_enable_reg;

	i3ddma_enable_reg.regValue = IoReg_Read32(H3DDMA_I3DDMA_enable_reg);
	i3ddma_enable_reg.reorder = (enable) ? 1 : 0;
	IoReg_Write32(H3DDMA_I3DDMA_enable_reg, i3ddma_enable_reg.regValue );
}
#endif
