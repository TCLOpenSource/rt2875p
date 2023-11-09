/*Kernel Header file*/
#include <linux/delay.h>
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
//#include <rtk_kdriver/RPCDriver.h>
#include <linux/pageremap.h>
#include <linux/kthread.h>
#include <uapi/linux/const.h>
#include <linux/mm.h>
#include <linux/string.h>/*memset*/
#include <linux/init.h>
#include <linux/spinlock_types.h>/*For spinlock*/
#include <linux/suspend.h>

/*RBUS Header file*/
#include <scaler/scalerCommon.h>
#include <scaler/scalerDrvCommon.h>
#include <rbus/mdomain_cap_reg.h>
#include <rbus/memc_mux_reg.h>
#include <rbus/sys_reg_reg.h>
#include <rbus/ppoverlay_reg.h>
#ifdef CONFIG_HW_SUPPORT_MEMC
#include <rbus/memc_dbus_wrap_reg.h>
#endif //CONFIG_HW_SUPPORT_MEMC
#include <rbus/di_reg.h>
#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <rbus/sys_reg_reg.h>


/*TVScaler Header file*/
#include "scaler_vpqmemcdev.h"
#include "ioctrl/vpq/vpq_memc_cmd_id.h"
#include "scaler_vscdev.h"
#include <tvscalercontrol/scalerdrv/scalerdrv.h>
#include <tvscalercontrol/vip/memc_mux.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#include <tvscalercontrol/panel/panelapi.h>
#include "scaler_vbedev.h"
#include <scaler/vipRPCCommon.h>

#include <rtd_log/rtd_module_log.h>


//#define RUN_MERLIN_MEMC_ENABLE
#define MEMC_DEBUG

#ifdef MEMC_DEBUG
#define MEMC_DBG_PRINT(s, args...) rtd_pr_memc_debug(s, ## args)
#else
#define MEMC_DBG_PRINT(s, args...)
#endif

#include "memc_isr/scalerMEMC.h"


#if 0
#define bisr_rstn_addr_kme 	(0xb8000f00)
#define IP_rstn_bits_kme 	(_BIT10)
#define bisr_remap_addr_kme	(0xb8000f10)
#define IP_remap_bits_kme 	(_BIT10)
#define bisr_done_addr_kme 	(0xb8000f20)
#define IP_done_bits_kme 	(_BIT10)
#define bisr_repair_addr_kme (0xb809d0e4)
#define IP_repair_check_bits_kme (_BIT26)
#define bisr_fail_addr_kme 	(0xb809d0e4)
#define IP_fail_bits_kme 	(_BIT24)

#define bisr_rstn_addr_kmc 	(0xb8000f00)
#define IP_rstn_bits_kmc 	(_BIT8|_BIT7|_BIT4|_BIT3|_BIT2|_BIT1)
#define bisr_remap_addr_kmc	(0xb8000f10)
#define IP_remap_bits_kmc 	(_BIT8|_BIT7|_BIT4|_BIT3|_BIT2|_BIT1)
#define bisr_done_addr_kmc 	(0xb8000f20)
#define IP_done_bits_kmc 	(_BIT8|_BIT7|_BIT4|_BIT3|_BIT2|_BIT1)
#define bisr_repair_addr_kmc (0xb8099098)
#define IP_repair_check_bits_kmc (_BIT8|_BIT7|_BIT6|_BIT5|_BIT4|_BIT3|_BIT2|_BIT1|_BIT0)
#define bisr_fail_addr_kmc 	(0xb809909c)
#define IP_fail_bits_kmc 	(_BIT7|_BIT6|_BIT3|_BIT2|_BIT1|_BIT0)
#endif
static struct cdev vpq_memc_cdev;

int vpq_memc_major   = 0;
int vpq_memc_minor   = 0 ;
int vpq_memc_nr_devs = 1;

static struct semaphore vpq_memc_Semaphore;
extern struct semaphore Memc_Realcinema_Semaphore;
extern DEBUG_SCALER_VSC_STRUCT DbgSclrFlgTkr;//extern char memc_realcinema_run_flag;//Run memc mode

module_param(vpq_memc_major, int, S_IRUGO);
module_param(vpq_memc_minor, int, S_IRUGO);
module_param(vpq_memc_nr_devs, int, S_IRUGO);
static struct class *vpq_memc_class;
static UINT8 MEMC_First_Run_Done=0;//using for booting to avoid second initial
static UINT8 MEMC_First_Run_FBG_enable=0;//using for booting fbg checking
static UINT8 MEMC_instanboot_resume_Done=0;
static UINT8 MEMC_snashop_resume_Done=0;
static unsigned int gphy_addr_kme, gphy_addr_kmc00, gphy_addr_kmc01;
static VPQ_MEMC_TYPE_T MEMC_motion_type = VPQ_MEMC_TYPE_OFF;
static char AVD_Unstable_Check_MEMC_OutBG_flag = FALSE;
extern unsigned char fwif_color_get_cinema_mode_en(void);

/*******************************************************************************
****************************VPQ MEMC DRIVER************************************
*******************************************************************************/
//#ifdef CONFIG_ENABLE_MEMC
void HAL_VPQ_MEMC_Initialize(void){
	if(MEMC_First_Run_Done == 0){
		MEMC_First_Run_Done = 1;
		MEMC_First_Run_FBG_enable = _ENABLE;
		Scaler_MEMC_output_force_bg_enable(TRUE);
		Scaler_MEMC_initial_Case();
		rtd_pr_memc_debug("[MEMC]HAL_VPQ_MEMC_Initialize Done!!\n");
	}
}

void HAL_VPQ_MEMC_Uninitialize(void){

}

void memc_setting_displayDtiming_framerate(void)
{
	unsigned int vfreq_ori = 0;
	vfreq_ori = Scaler_DispGetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ);
	modestate_decide_dtg_m_mode();
	if(vfreq_ori != Scaler_DispGetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ))
		scaler_disp_smooth_variable_settingByDisp(0);
}

unsigned int memc_realcinema_get_current_framerate(void)
{
	ppoverlay_double_buffer_ctrl_RBUS ppoverlay_double_buffer_ctrl_reg;
	ppoverlay_dv_total_RBUS dv_total_reg;
	unsigned int vtotal;
	unsigned int frameRates;
	unsigned long flags;//for spin_lock_irqsave

	//down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	ppoverlay_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_read_sel = 1;  //D7_read_sel
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_reg.regValue);
	dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
	vtotal = dv_total_reg.dv_total;
	ppoverlay_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_read_sel = 0;  //D7_read_sel
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_reg.regValue);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	//up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	frameRates = (((Get_DISPLAY_CLOCK_TYPICAL()/Get_DISP_HORIZONTAL_TOTAL())*1000)/vtotal);
	frameRates /= 1000;

	if((frameRates>=47) && (frameRates<=49)){
		frameRates = 48;
	}else if((frameRates>49) && (frameRates<51)){
		frameRates = 50;
	}else if(((frameRates>=51) && (frameRates<=59))||
		((frameRates>=45) && (frameRates<=47))){
		// apply it.
	}
	else if((frameRates>59) && (frameRates<61)){
		frameRates = 60;
	}else{
		frameRates = 60;
	}

	rtd_pr_memc_notice("memc_realcinema_get_current_framerate=%d!!\n", frameRates);
	return frameRates;
}

/**
 *@brief memc_realcinema_framerate Set framerate while turnning on/off real-cinema
 *@return VOID
**/
void memc_realcinema_framerate(void)
{
	/*scaler timing setting is finish?*/
	if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_STATE) != _MODE_STATE_ACTIVE)
		return;

	/*input frame rate must be 24hz or output is 48hz*/
	if(((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_V_FREQ) > 235) && \
			(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_V_FREQ) < 245)) || \
			Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_DTG_MASTER_V_FREQ) == 48) {
		unsigned int vfreq_ori = 0;
		vfreq_ori = memc_realcinema_get_current_framerate(); //Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_DTG_MASTER_V_FREQ);
		modestate_decide_dtg_m_modeByDisp((unsigned char)SLR_MAIN_DISPLAY);
		if(vfreq_ori != Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_DTG_MASTER_V_FREQ))
			scaler_disp_smooth_variable_settingByDisp((unsigned char)SLR_MAIN_DISPLAY);
		rtd_pr_memc_notice("vfreq_ori=%d, DTG_M=%d \n", vfreq_ori, Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_DTG_MASTER_V_FREQ));
	}
}

long HAL_VPQ_MEMC_SetMotionComp(UINT8 blurLevel, UINT8 judderLevel, VPQ_MEMC_TYPE_T motion){
	VPQ_MEMC_SETMOTIONCOMP_T *set_motion_comp_info;
	unsigned int ulCount = 0;
	int ret = 0;
	unsigned int semaphoreCnt = 0;

	//Elsie 20151120, add semaphore
	while((IoReg_Read32(DI_IM_DI_BLEND_AND_WEIGHTING_METHOD_reg)&_BIT30) && semaphoreCnt < 100000) {
		semaphoreCnt++;
	}
	if(semaphoreCnt >= 100000) {
		rtd_pr_memc_debug("[MEMC]HAL_VPQ_MEMC_SetMotionComp timed out!\n");
		return -1;
	}
	IoReg_SetBits(DI_IM_DI_BLEND_AND_WEIGHTING_METHOD_reg, _BIT30);

	rtd_pr_memc_debug("[MEMC]HAL_VPQ_MEMC_SetMotionComp\n");
	rtd_pr_memc_debug("blurLevel = %d\n judderLevel = %d\n motion = %d\n",blurLevel, judderLevel, motion);

	set_motion_comp_info = (VPQ_MEMC_SETMOTIONCOMP_T *)Scaler_GetShareMemVirAddr(SCALERIOC_MEMC_SETMOTIONCOMP);
	ulCount = sizeof(VPQ_MEMC_SETMOTIONCOMP_T) / sizeof(unsigned int);

	set_motion_comp_info->blurLevel = blurLevel;
	set_motion_comp_info->judderLevel = judderLevel;
	set_motion_comp_info->motion = motion;
#ifdef CONFIG_RTK_KDEV_DEBUG_ISR
	//change endian
	set_motion_comp_info->blurLevel = htonl(set_motion_comp_info->blurLevel);
	set_motion_comp_info->judderLevel = htonl(set_motion_comp_info->judderLevel);
	set_motion_comp_info->motion = htonl(set_motion_comp_info->motion);
#endif

	down(&Memc_Realcinema_Semaphore);
	//save current MEMC motion type+	Scaler_MEMC_SetMotionType(motion);
	Scaler_MEMC_SetMotionType(motion);
	//set frame rate
	 
	DbgSclrFlgTkr.memc_realcinema_run_flag = TRUE;//Run memc mode
		//memc_realcinema_framerate();
	
	up(&Memc_Realcinema_Semaphore);
#ifdef CONFIG_RTK_KDEV_DEBUG_ISR
	if (0 != (ret = Scaler_SendRPC(SCALERIOC_MEMC_SETMOTIONCOMP,0,0))){
		MEMC_DBG_PRINT("[MEMC]ret=%d, SCALERIOC_MEMC_SETMOTIONCOMP RPC fail !!!\n", ret);
	}
#else
#ifdef CONFIG_HW_SUPPORT_MEMC

#ifndef CONFIG_MEMC_TASK_QUEUE
	if(!Scaler_MEMC_GetMEMC_Enable())
		return 0;
	#ifndef CONFIG_MEMC_DEFAULT_REPEAT_MODE
	Scaler_MEMC_SetMotionComp(set_motion_comp_info->blurLevel, set_motion_comp_info->judderLevel, set_motion_comp_info->motion);
#else
		Scaler_MEMC_SetMotionComp(0,0,0);
	#endif
#else
		SCALER_MEMC_TASK_T task;

		task.type = SCALERIOC_MEMC_SETMOTIONCOMP;
		memcpy(&task.data,set_motion_comp_info,sizeof(task.data));
		#ifdef CONFIG_MEMC_DEFAULT_REPEAT_MODE
		task.data.memc_set_motion_comp.motion = 0;
		#endif
		Scaler_MEMC_PushTask(&task);
#endif

#endif
#endif

	IoReg_ClearBits(DI_IM_DI_BLEND_AND_WEIGHTING_METHOD_reg,_BIT30);

	return ret;
}

/* not use in WebOS
void HAL_VPQ_MEMC_SetBlurLevel(UINT8 blurLevel){
	int ret;

	MEMC_DBG_PRINT("blurLevel = %d\n",blurLevel);

	if (0 != (ret = Scaler_SendRPC(SCALERIOC_MEMC_SETBLURLEVEL, blurLevel, 1))){
		MEMC_DBG_PRINT("[MEMC]ret=%d, SCALERIOC_MEMC_SETBLURLEVEL RPC fail !!!\n", ret);
	}
}

void HAL_VPQ_MEMC_SetJudderLevel(UINT8 judderLevel){
	int ret;

	MEMC_DBG_PRINT("judderLevel = %d\n",judderLevel);

	if (0 != (ret = Scaler_SendRPC(SCALERIOC_MEMC_SETJUDDERLEVEL, judderLevel, 1))){
		MEMC_DBG_PRINT("[MEMC]ret=%d, SCALERIOC_MEMC_SETJUDDERLEVEL RPC fail !!!\n", ret);
	}
}
*/
long HAL_VPQ_MEMC_MotionCompOnOff(BOOLEAN bOnOff){
	int ret = 0;
	unsigned int semaphoreCnt = 0;

	rtd_pr_memc_debug("[MEMC]HAL_VPQ_MEMC_SetMotionComp, bOnOff = %d\n", bOnOff);

	while((IoReg_Read32(DI_IM_DI_BLEND_AND_WEIGHTING_METHOD_reg)&_BIT30) && semaphoreCnt < 100000) {
		semaphoreCnt++;
	}
	if(semaphoreCnt >= 100000) {
		rtd_pr_memc_debug("[MEMC]HAL_VPQ_MEMC_SetMotionComp timed out!\n");
		return -1;
	}
	IoReg_SetBits(DI_IM_DI_BLEND_AND_WEIGHTING_METHOD_reg,_BIT30);
#ifdef CONFIG_RTK_KDEV_DEBUG_ISR
	if (0 != (ret = Scaler_SendRPC(SCALERIOC_MEMC_MOTIONCOMPONOFF, bOnOff, 1))){
		MEMC_DBG_PRINT("[MEMC]ret=%d, SCALERIOC_MEMC_MOTIONCOMPONOFF RPC fail !!!\n", ret);
	}
#else
#ifdef CONFIG_HW_SUPPORT_MEMC
	#ifndef CONFIG_MEMC_TASK_QUEUE
		if(!Scaler_MEMC_GetMEMC_Enable())
			return 0;
	Scaler_MEMC_MotionCompOnOff(bOnOff);
	#else
		SCALER_MEMC_TASK_T task;

		task.type = SCALERIOC_MEMC_MOTIONCOMPONOFF;
		task.data.value = bOnOff;
		Scaler_MEMC_PushTask(&task);
	#endif

#endif
#endif
	IoReg_ClearBits(DI_IM_DI_BLEND_AND_WEIGHTING_METHOD_reg,_BIT30);
	return ret;
}

long HAL_VPQ_MEMC_LowDelayMode(UINT8 type){
	int ret = 0;
	unsigned int semaphoreCnt = 0;

	while((IoReg_Read32(DI_IM_DI_BLEND_AND_WEIGHTING_METHOD_reg)&_BIT30) && semaphoreCnt < 100000) {
		semaphoreCnt++;
	}
	if(semaphoreCnt >= 100000) {
		rtd_pr_memc_debug("[MEMC]HAL_VPQ_MEMC_LowDelayMode timed out!\n");
		return -1;
	}
	IoReg_SetBits(DI_IM_DI_BLEND_AND_WEIGHTING_METHOD_reg,_BIT30);
#ifdef CONFIG_RTK_KDEV_DEBUG_ISR
	if (0 != (ret = Scaler_SendRPC(SCALERIOC_MEMC_LOWDELAYMODE, type, 1))){
		MEMC_DBG_PRINT("[MEMC]ret=%d, SCALERIOC_MEMC_LOWDELAYMODE RPC fail !!!\n", ret);
	}
#else
#ifdef CONFIG_HW_SUPPORT_MEMC
	#ifndef CONFIG_MEMC_TASK_QUEUE
		if(!Scaler_MEMC_GetMEMC_Enable())
			return 0;
	Scaler_MEMC_LowDelayMode(type);
	#else
		SCALER_MEMC_TASK_T task;

		task.type = SCALERIOC_MEMC_LOWDELAYMODE;
		task.data.value = type;
		Scaler_MEMC_PushTask(&task);
	#endif
#endif
#endif
	IoReg_ClearBits(DI_IM_DI_BLEND_AND_WEIGHTING_METHOD_reg,_BIT30);
	return ret;
}
long HAL_VPQ_MEMC_SetRGBYUVMode(UINT8 mode){
	int ret = 0;

	//if(mode == 0)
	//	mode = VPQ_MEMC_RGB_IN_VIDEO_OUT;
	//else
	//	mode = VPQ_MEMC_YUV_IN_VIDEO_OUT;
	unsigned int semaphoreCnt = 0;

	while((IoReg_Read32(DI_IM_DI_BLEND_AND_WEIGHTING_METHOD_reg)&_BIT30) && semaphoreCnt < 100000) {
		semaphoreCnt++;
	}
	if(semaphoreCnt >= 100000) {
		rtd_pr_memc_debug("[MEMC]HAL_VPQ_MEMC_SetRGBYUVMode timed out!\n");
		return -1;
	}
	IoReg_SetBits(DI_IM_DI_BLEND_AND_WEIGHTING_METHOD_reg,_BIT30);
#ifdef CONFIG_RTK_KDEV_DEBUG_ISR
	if (0 != (ret = Scaler_SendRPC(SCALERIOC_MEMC_SETRGBYUVMode, mode, 1))){
		MEMC_DBG_PRINT("[MEMC]ret=%d, SCALERIOC_MEMC_SETRGBYUVMode RPC fail !!!\n", ret);
	}
#else
#ifdef CONFIG_HW_SUPPORT_MEMC
	#ifndef CONFIG_MEMC_TASK_QUEUE
		if(!Scaler_MEMC_GetMEMC_Enable())
			return 0;
	Scaler_MEMC_SetRGBYUVMode(mode);
	#else
		SCALER_MEMC_TASK_T task;

		task.type = SCALERIOC_MEMC_SETRGBYUVMode;
		task.data.value = mode;
		Scaler_MEMC_PushTask(&task);
	#endif
#endif
#endif
	IoReg_ClearBits(DI_IM_DI_BLEND_AND_WEIGHTING_METHOD_reg,_BIT30);
	return ret;
}
void HAL_VPQ_MEMC_GetFrameDelay (UINT16 *pFrameDelay){
	/*
	int ret;

	if (0 != (ret = Scaler_SendRPC(SCALERIOC_MEMC_GETFRAMEDELAY, 1, 1))){
		rtd_pr_memc_debug("[MEMC]ret=%d, SCALERIOC_MEMC_GETFRAMEDELAY RPC fail !!!\n", ret);
	}
	*/
}


void HAL_VPQ_MEMC_SetVideoBlock(VPQ_MEMC_MUTE_TYPE_T type, BOOLEAN bOnOff) {
	set_display_forcebg_mask(SLR_MAIN_DISPLAY, SLR_FORCE_BG_TYPE_3D, bOnOff);
	mute_control(SLR_MAIN_DISPLAY, bOnOff);
}

/* not use in WebOS
void HAL_VPQ_MEMC_SetTrueMotionDemo(BOOLEAN bOnOff){
	int ret;

	if (0 != (ret = Scaler_SendRPC(SCALERIOC_MEMC_SETTRUEMOTIONDEMO, bOnOff, 1))){
		MEMC_DBG_PRINT("[MEMC]ret=%d, SCALERIOC_MEMC_SETTRUEMOTIONDEMO RPC fail !!!\n", ret);
	}
}
void HAL_VPQ_MEMC_GetFirmwareVersion(UINT16 *pVersion){

	int ret;

	if (0 != (ret = Scaler_SendRPC(SCALERIOC_MEMC_SETBLURLEVEL, blurLevel, 1))){
		rtd_pr_memc_debug("[MEMC]ret=%d, SCALERIOC_MEMC_SETBLURLEVEL RPC fail !!!\n", ret);
	}

}
*/
void HAL_VPQ_MEMC_SetBypassRegion(BOOLEAN bOnOff, VPQ_MEMC_BYPASS_REGION_T region, UINT32 x, UINT32 y, UINT32 w, UINT32 h){
	VPQ_MEMC_SETBYPASSREGION_T*set_bypass_region_info;
	unsigned int ulCount = 0;
#ifdef CONFIG_RTK_KDEV_DEBUG_ISR
	int ret;
#endif
	MEMC_DBG_PRINT("region = %d\n",region);
	MEMC_DBG_PRINT("rect.x = %d, rect.y = %d, rect.w = %d, rect.h = %d\n",x, y, w, h);

	MEMC_DBG_PRINT("bOnOff = %d\n",bOnOff);

	set_bypass_region_info = (VPQ_MEMC_SETBYPASSREGION_T *)Scaler_GetShareMemVirAddr(SCALERIOC_MEMC_SETBYPASSREGION);
	ulCount = sizeof(VPQ_MEMC_SETBYPASSREGION_T) / sizeof(unsigned int);

	set_bypass_region_info->bOnOff = bOnOff;
	set_bypass_region_info->region = region;
	set_bypass_region_info->x = x;
	set_bypass_region_info->y = y;
	set_bypass_region_info->w = w;
	set_bypass_region_info->h = h;
#ifdef CONFIG_RTK_KDEV_DEBUG_ISR
	//change endian
	set_bypass_region_info->bOnOff = htonl(set_bypass_region_info->bOnOff);
	set_bypass_region_info->region = htonl(set_bypass_region_info->region);
	set_bypass_region_info->x = htonl(set_bypass_region_info->x);
	set_bypass_region_info->y = htonl(set_bypass_region_info->y);
	set_bypass_region_info->w = htonl(set_bypass_region_info->w);
	set_bypass_region_info->h = htonl(set_bypass_region_info->h);


	if (0 != (ret = Scaler_SendRPC(SCALERIOC_MEMC_SETBYPASSREGION,0,0))){
		MEMC_DBG_PRINT("[MEMC]ret=%d, SCALERIOC_MEMC_SETBYPASSREGION RPC fail !!!\n", ret);
	}
#else
#ifdef CONFIG_HW_SUPPORT_MEMC
	#ifndef CONFIG_MEMC_TASK_QUEUE
		if(!Scaler_MEMC_GetMEMC_Enable())
			return 0;
	Scaler_MEMC_SetBypassRegion(set_bypass_region_info->bOnOff, set_bypass_region_info->region, set_bypass_region_info->x, set_bypass_region_info->y, set_bypass_region_info->w, set_bypass_region_info->h);
	#else
		SCALER_MEMC_TASK_T task;

		task.type = SCALERIOC_MEMC_SETBYPASSREGION;
		memcpy(&task.data,set_bypass_region_info,sizeof(SCALER_MEMC_SETBYPASSREGION_T));
		Scaler_MEMC_PushTask(&task);
	#endif

#endif
#endif
}
/* not use in WebOS
void HAL_VPQ_MEMC_SetReverseControl(UINT8 u8Mode){
	int ret;

	if (0 != (ret = Scaler_SendRPC(SCALERIOC_MEMC_SETREVERSECONTROL, u8Mode, 1))){
		MEMC_DBG_PRINT("[MEMC]ret=%d, SCALERIOC_MEMC_SETREVERSECONTROL RPC fail !!!\n", ret);
	}
}
void HAL_VPQ_MEMC_Freeze(UINT8 type){
	int ret;

	if (0 != (ret = Scaler_SendRPC(SCALERIOC_MEMC_FREEZE, type, 1))){
		MEMC_DBG_PRINT("[MEMC]ret=%d, SCALERIOC_MEMC_FREEZE RPC fail !!!\n", ret);
	}
}
void HAL_VPQ_MEMC_SetDemoBar(BOOLEAN bOnOff, UINT8 r, UINT8 g, UINT8 b){
	VPQ_MEMC_SETDEMOBAR_T*set_demo_bar_info;
	unsigned int ulCount = 0;
	int ret;

	set_demo_bar_info = (VPQ_MEMC_SETDEMOBAR_T *)Scaler_GetShareMemVirAddr(SCALERIOC_MEMC_SETDEMOBAR);
	ulCount = sizeof(VPQ_MEMC_SETDEMOBAR_T) / sizeof(unsigned int);

	set_demo_bar_info->bOnOff = bOnOff;
	set_demo_bar_info->r = r;
	set_demo_bar_info->g = g;
	set_demo_bar_info->b = b;

	//change endian
	set_demo_bar_info->bOnOff = htonl(set_demo_bar_info->bOnOff);
	set_demo_bar_info->r = htonl(set_demo_bar_info->r);
	set_demo_bar_info->g = htonl(set_demo_bar_info->g);
	set_demo_bar_info->b = htonl(set_demo_bar_info->b);

	if (0 != (ret = Scaler_SendRPC(SCALERIOC_MEMC_SETDEMOBAR,0,0))){
		MEMC_DBG_PRINT("[MEMC]ret=%d, SCALERIOC_MEMC_SETDEMOBAR RPC fail !!!\n", ret);
	}
}
void HAL_VPQ_MEMC_DEBUG(void){

}
*/
void Scaler_MEMC_SetInOutUseCase(VPQ_MEMC_INPUT_RESOLUTION input_re, VPQ_MEMC_OUTPUT_RESOLUTION output_re, VPQ_MEMC_INPUT_FORMAT input_for, VPQ_MEMC_OUTPUT_FORMAT output_for){
	SCALER_MEMC_SETINOUTUSECASE_T* set_inout_usecase_info;
	unsigned int ulCount = 0;
#ifdef CONFIG_RTK_KDEV_DEBUG_ISR
	int ret;
#endif
	set_inout_usecase_info = (SCALER_MEMC_SETINOUTUSECASE_T*)Scaler_GetShareMemVirAddr(SCALERIOC_MEMC_SETINOUTUSECASE);
	ulCount = sizeof(SCALER_MEMC_SETINOUTUSECASE_T) / sizeof(unsigned int);

	set_inout_usecase_info->input_re = input_re;
	set_inout_usecase_info->output_re = output_re;
	set_inout_usecase_info->input_for = input_for;
	set_inout_usecase_info->output_for = output_for;
#ifdef CONFIG_RTK_KDEV_DEBUG_ISR
	//change endian
	set_inout_usecase_info->input_re = htonl(set_inout_usecase_info->input_re);
	set_inout_usecase_info->output_re = htonl(set_inout_usecase_info->output_re);
	set_inout_usecase_info->input_for = htonl(set_inout_usecase_info->input_for);
	set_inout_usecase_info->output_for = htonl(set_inout_usecase_info->output_for);

	if (0 != (ret = Scaler_SendRPC(SCALERIOC_MEMC_SETINOUTUSECASE,0,0))){
		MEMC_DBG_PRINT("[MEMC]ret=%d, SCALERIOC_MEMC_SETINOUTUSECASE RPC fail !!!\n", ret);
	}
#else
#ifdef CONFIG_HW_SUPPORT_MEMC
	#ifndef CONFIG_MEMC_TASK_QUEUE
		if(!Scaler_MEMC_GetMEMC_Enable())
			return 0;
	Scaler_MEMC_SetInOutputUseCase(set_inout_usecase_info->input_re, set_inout_usecase_info->output_re, set_inout_usecase_info->input_for, set_inout_usecase_info->output_for);
	#else
		SCALER_MEMC_TASK_T task;

		task.type = SCALERIOC_MEMC_SETINOUTUSECASE;
		memcpy(&task.data,set_inout_usecase_info,sizeof(SCALER_MEMC_SETINOUTUSECASE_T));
		Scaler_MEMC_PushTask(&task);
	#endif
#endif
#endif
}

void Scaler_MEMC_SetInOutputFormat(VPQ_MEMC_INPUT_FORMAT input_for, VPQ_MEMC_OUTPUT_FORMAT output_for){
	SCALER_MEMC_SETINOUTPUTFORMAT_T* set_inputput_format_info;
	unsigned int ulCount = 0;
#ifdef CONFIG_RTK_KDEV_DEBUG_ISR
	int ret;
#endif
	set_inputput_format_info = (SCALER_MEMC_SETINOUTPUTFORMAT_T*)Scaler_GetShareMemVirAddr(SCALERIOC_MEMC_SETINPUTOUTPUTFORMAT);
	ulCount = sizeof(SCALER_MEMC_SETINOUTPUTFORMAT_T) / sizeof(unsigned int);

	set_inputput_format_info->input_for = input_for;
	set_inputput_format_info->output_for = output_for;
#ifdef CONFIG_RTK_KDEV_DEBUG_ISR
	//change endian
	set_inputput_format_info->input_for = htonl(set_inputput_format_info->input_for);
	set_inputput_format_info->output_for = htonl(set_inputput_format_info->output_for);

	if (0 != (ret = Scaler_SendRPC(SCALERIOC_MEMC_SETINPUTOUTPUTFORMAT,0,0))){
		MEMC_DBG_PRINT("[MEMC]ret=%d, SCALERIOC_MEMC_SETINPUTOUTPUTFORMAT RPC fail !!!\n", ret);
	}
#else
#ifdef CONFIG_HW_SUPPORT_MEMC
	#ifndef CONFIG_MEMC_TASK_QUEUE
		if(!Scaler_MEMC_GetMEMC_Enable())
			return 0;
	Scaler_MEMC_SetInputOutputFormat(set_inputput_format_info->input_for, set_inputput_format_info->output_for);
	#else
		SCALER_MEMC_TASK_T task;

		task.type = SCALERIOC_MEMC_SETINPUTOUTPUTFORMAT;
		memcpy(&task.data,set_inputput_format_info,sizeof(SCALER_MEMC_SETINOUTPUTFORMAT_T));
		Scaler_MEMC_PushTask(&task);
	#endif

#endif
#endif
}

void Scaler_MEMC_SetInOutputResolution(VPQ_MEMC_INPUT_RESOLUTION input_re, VPQ_MEMC_OUTPUT_RESOLUTION output_re){
	SCALER_MEMC_SETINOUTPUTRESOLUTION_T* set_inoutput_resolution_info;
	unsigned int ulCount = 0;
#ifdef CONFIG_RTK_KDEV_DEBUG_ISR
	int ret;
#endif
	set_inoutput_resolution_info = (SCALER_MEMC_SETINOUTPUTRESOLUTION_T*)Scaler_GetShareMemVirAddr(SCALERIOC_MEMC_SETINPUTOUTPUTRESOLUTION);
	ulCount = sizeof(SCALER_MEMC_SETINOUTPUTRESOLUTION_T) / sizeof(unsigned int);

	set_inoutput_resolution_info->input_re = input_re;
	set_inoutput_resolution_info->output_re = output_re;
#ifdef CONFIG_RTK_KDEV_DEBUG_ISR
	//change endian
	set_inoutput_resolution_info->input_re = htonl(set_inoutput_resolution_info->input_re);
	set_inoutput_resolution_info->output_re = htonl(set_inoutput_resolution_info->output_re);

	if (0 != (ret = Scaler_SendRPC(SCALERIOC_MEMC_SETINPUTOUTPUTRESOLUTION,0,0))){
		MEMC_DBG_PRINT("[MEMC]ret=%d, SCALERIOC_MEMC_SETINPUTOUTPUTRESOLUTION RPC fail !!!\n", ret);
	}
#else
#ifdef CONFIG_HW_SUPPORT_MEMC
	#ifndef CONFIG_MEMC_TASK_QUEUE
		if(!Scaler_MEMC_GetMEMC_Enable())
			return 0;
	Scaler_MEMC_SetInputOutputResolution(set_inoutput_resolution_info->input_re, set_inoutput_resolution_info->output_re);
	#else
		SCALER_MEMC_TASK_T task;

		task.type = SCALERIOC_MEMC_SETINPUTOUTPUTRESOLUTION;
		memcpy(&task.data,set_inoutput_resolution_info,sizeof(SCALER_MEMC_SETINOUTPUTRESOLUTION_T));
		Scaler_MEMC_PushTask(&task);
	#endif

#endif
#endif
}
#if 1
void Scaler_MEMC_SetInOutputFrameRate(VPQ_MEMC_INPUT_FRAMERATE input_frame, VPQ_MEMC_OUTPUT_FRAMERATE output_frame){
	SCALER_MEMC_INOUTFRAMERATE_T* set_inoutput_framerate_info;
	unsigned int ulCount = 0;
#ifdef CONFIG_RTK_KDEV_DEBUG_ISR
	int ret;
#endif
	set_inoutput_framerate_info = (SCALER_MEMC_INOUTFRAMERATE_T*)Scaler_GetShareMemVirAddr(SCALERIOC_MEMC_SETINOUTPUTFRAMERATE);
	ulCount = sizeof(SCALER_MEMC_INOUTFRAMERATE_T) / sizeof(unsigned int);

	set_inoutput_framerate_info->input_frame_rate= input_frame;
	set_inoutput_framerate_info->output_frame_rate= output_frame;
#ifdef CONFIG_RTK_KDEV_DEBUG_ISR
	//change endian
	set_inoutput_framerate_info->input_frame_rate = htonl(set_inoutput_framerate_info->input_frame_rate);
	set_inoutput_framerate_info->output_frame_rate = htonl(set_inoutput_framerate_info->output_frame_rate);

	if (0 != (ret = Scaler_SendRPC(SCALERIOC_MEMC_SETINOUTPUTFRAMERATE,0,0))){
		MEMC_DBG_PRINT("[MEMC]ret=%d, SCALERIOC_MEMC_SETINOUTPUTFRAMERATE RPC fail !!!\n", ret);
	}
#else
#ifdef CONFIG_HW_SUPPORT_MEMC
	#ifndef CONFIG_MEMC_TASK_QUEUE
		if(!Scaler_MEMC_GetMEMC_Enable())
			return 0;
	Scaler_MEMC_SetInputFrameRate(set_inoutput_framerate_info->input_frame_rate);
	Scaler_MEMC_SetOutputFrameRate(set_inoutput_framerate_info->output_frame_rate);
	#else
		SCALER_MEMC_TASK_T task;

		task.type = SCALERIOC_MEMC_SETINOUTPUTFRAMERATE;
		memcpy(&task.data,set_inoutput_framerate_info,sizeof(SCALER_MEMC_INOUTFRAMERATE_T));
		Scaler_MEMC_PushTask(&task);
	#endif

#endif
#endif
}
#else
void Scaler_MEMC_SetInputFrameRate(VPQ_MEMC_INPUT_FRAMERATE input_frame){
	int ret;

	if (0 != (ret = Scaler_SendRPC(SCALERIOC_MEMC_SETINPUTFRAMERATE, input_frame, 1))){
		MEMC_DBG_PRINT("[MEMC]ret=%d, SCALERIOC_MEMC_SETINPUTFRAMERATE RPC fail !!!\n", ret);
	}
}

void Scaler_MEMC_SetOutputFrameRate(VPQ_MEMC_OUTPUT_FRAMERATE output_frame){
	int ret;

	if (0 != (ret = Scaler_SendRPC(SCALERIOC_MEMC_SETOUTPUTFRAMERATE, output_frame, 1))){
		MEMC_DBG_PRINT("[MEMC]ret=%d, SCALERIOC_MEMC_SETOUTPUTFRAMERATE RPC fail !!!\n", ret);
	}
}
#endif

long Scaler_MEMC_SetFrameRepeatEnable(BOOL bEnable){
	int ret = 0;
	unsigned int semaphoreCnt = 0;

	while((IoReg_Read32(DI_IM_DI_BLEND_AND_WEIGHTING_METHOD_reg)&_BIT30) && semaphoreCnt < 100000) {
		semaphoreCnt++;
	}
	if(semaphoreCnt >= 100000) {
		rtd_pr_memc_debug("[MEMC]Scaler_MEMC_SetFrameRepeatEnable timed out!\n");
		return -1;
	}
	IoReg_SetBits(DI_IM_DI_BLEND_AND_WEIGHTING_METHOD_reg,_BIT30);
#ifdef CONFIG_RTK_KDEV_DEBUG_ISR
	if (0 != (ret = Scaler_SendRPC(SCALERIOC_MEMC_SETMEMCFRAMEREPEATENABLE, bEnable, 1))){
		MEMC_DBG_PRINT("[MEMC]ret=%d, SCALERIOC_MEMC_SETMEMCFRAMEREPEATENABLE RPC fail !!!\n", ret);
	}
#else
#ifdef CONFIG_HW_SUPPORT_MEMC
	#ifndef CONFIG_MEMC_TASK_QUEUE
		if(!Scaler_MEMC_GetMEMC_Enable())
			return 0;
	Scaler_MEMC_SetMEMCFrameRepeatEnable(bEnable);
	#else
		SCALER_MEMC_TASK_T task;

		task.type = SCALERIOC_MEMC_SETMEMCFRAMEREPEATENABLE;
		task.data.value = bEnable;
		Scaler_MEMC_PushTask(&task);
	#endif

#endif
#endif
	IoReg_ClearBits(DI_IM_DI_BLEND_AND_WEIGHTING_METHOD_reg,_BIT30);
	return ret;
}

void Scaler_MEMC_SetInOutMode(VPQ_MEMC_INOUT_MODE emom){
#ifdef CONFIG_RTK_KDEV_DEBUG_ISR
	int ret;
	if (0 != (ret = Scaler_SendRPC(SCALERIOC_MEMC_SETMEMCINOUTMODE, emom, 1))){
		MEMC_DBG_PRINT("[MEMC]ret=%d, SCALERIOC_MEMC_SETMEMCINOUTMODE RPC fail !!!\n", ret);
	}
#else
#ifdef CONFIG_HW_SUPPORT_MEMC
	#ifndef CONFIG_MEMC_TASK_QUEUE
		if(!Scaler_MEMC_GetMEMC_Enable())
			return 0;
	ISR_Scaler_MEMC_SetInOutMode(emom);
	#else
		SCALER_MEMC_TASK_T task;

		task.type = SCALERIOC_MEMC_SETMEMCINOUTMODE;
		task.data.value = emom;
		Scaler_MEMC_PushTask(&task);
	#endif

#endif
#endif
}

void Scaler_MEMC_Instanboot_InitPhaseTable(BOOL bEnable){
#ifdef CONFIG_RTK_KDEV_DEBUG_ISR
	int ret;
	if (0 != (ret = Scaler_SendRPC(SCALERIOC_MEMC_INSTANBOOT_INITPHASETABLE, bEnable, 1))){
		MEMC_DBG_PRINT("[MEMC]ret=%d, SCALERIOC_MEMC_INSTANBOOT_INITPHASETABLE RPC fail !!!\n", ret);
	}
#else
#ifdef CONFIG_HW_SUPPORT_MEMC
	Scaler_MEMC_SetInstantBootInitPhaseTable();

#endif
#endif
}


void Scaler_MEMC_SetInOutFrameRate(void){
	UINT8 memc_input_frame_rate;
	UINT8 memc_output_frame_rate;

	if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 235) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 245))	//24Hz
	{
		memc_input_frame_rate = VPQ_INPUT_24HZ;
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) == PIXEL_MODE_2_PIXEL){
			memc_output_frame_rate = VPQ_OUTPUT_120HZ; //24Hz => 120Hz
		}
		else{
			if((Scaler_MEMC_GetMotionType() == VPQ_MEMC_TYPE_OFF) && (fwif_color_get_cinema_mode_en() == 1)){
				memc_output_frame_rate = VPQ_OUTPUT_48HZ; //24Hz => 48Hz	//tru-motion off + real cinema
			}else{
				if((vbe_disp_get_adaptivestream_fs_mode() == 1) && (0 == vbe_disp_get_adaptivestream_force_framerate()))
					memc_output_frame_rate = VPQ_OUTPUT_48HZ;
				else
					memc_output_frame_rate = VPQ_OUTPUT_60HZ; //24Hz => 60Hz
			}
		}
	}
	else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 245) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 255))	//25Hz
	{
		memc_input_frame_rate = VPQ_INPUT_25HZ;
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) == PIXEL_MODE_2_PIXEL){
			memc_output_frame_rate = VPQ_OUTPUT_100HZ; //25Hz => 100Hz
		}
		else{
			memc_output_frame_rate = VPQ_OUTPUT_50HZ; //25Hz => 50Hz
		}
	}
	else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 295) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 305))	//30Hz
	{
		memc_input_frame_rate = VPQ_INPUT_30HZ;
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) == PIXEL_MODE_2_PIXEL){
			memc_output_frame_rate = VPQ_OUTPUT_120HZ; //30Hz => 120Hz
		}
		else{
			memc_output_frame_rate = VPQ_OUTPUT_60HZ; //30Hz => 60Hz
		}
	}
	else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 495) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 505))	//50Hz
	{
		memc_input_frame_rate = VPQ_INPUT_50HZ;
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) == PIXEL_MODE_2_PIXEL){
			memc_output_frame_rate = VPQ_OUTPUT_100HZ; //50Hz => 100Hz
		}
		else{
			memc_output_frame_rate = VPQ_OUTPUT_50HZ; //50Hz => 50Hz
		}
	}
	else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 595) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 605))	//60hz
	{
		memc_input_frame_rate = VPQ_INPUT_60HZ;
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) == PIXEL_MODE_2_PIXEL){
			memc_output_frame_rate = VPQ_OUTPUT_120HZ; //60Hz => 120Hz
		}
		else{
			memc_output_frame_rate = VPQ_OUTPUT_60HZ; //60Hz => 60Hz
		}
	}
	else{ // others
		memc_input_frame_rate = VPQ_INPUT_60HZ;
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) == PIXEL_MODE_2_PIXEL){
			memc_output_frame_rate = VPQ_OUTPUT_120HZ; //60Hz => 120Hz
		}
		else{
			memc_output_frame_rate = VPQ_OUTPUT_60HZ; //60Hz => 60Hz
		}
	}

	rtd_pr_memc_debug("[MEMC]UZU DTG input frame_rate = %d\n",memc_input_frame_rate);
	rtd_pr_memc_debug("[MEMC]MEMC DTG output frame_rate = %d\n",memc_output_frame_rate);

	//set MEMC in/out frame rate
	//Scaler_MEMC_SetInputFrameRate(memc_input_frame_rate);
	//Scaler_MEMC_SetOutputFrameRate(memc_output_frame_rate);
	Scaler_MEMC_SetInOutputFrameRate(memc_input_frame_rate, memc_output_frame_rate);
}

void Scaler_MEMC_SetInOutFrameRateByDisp(unsigned char _channel)
{
	SCALER_DISP_CHANNEL channel = (SCALER_DISP_CHANNEL)_channel;
	UINT8 memc_input_frame_rate;
	UINT8 memc_output_frame_rate;

	if((Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) > 235) && (Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) < 245))	//24Hz
	{
		memc_input_frame_rate = VPQ_INPUT_24HZ;
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) == PIXEL_MODE_2_PIXEL){
			memc_output_frame_rate = VPQ_OUTPUT_120HZ; //24Hz => 120Hz
		}
		else{
			if((Scaler_MEMC_GetMotionType() == VPQ_MEMC_TYPE_OFF) && (fwif_color_get_cinema_mode_en() == 1)){
				memc_output_frame_rate = VPQ_OUTPUT_48HZ; //24Hz => 48Hz	//tru-motion off + real cinema
			}else{
				if((vbe_disp_get_adaptivestream_fs_mode() == 1) && (0 == vbe_disp_get_adaptivestream_force_framerate()))
					memc_output_frame_rate = VPQ_OUTPUT_48HZ;
				else
					memc_output_frame_rate = VPQ_OUTPUT_60HZ; //24Hz => 60Hz
			}
		}
	}
	else if((Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) > 245) && (Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) < 255))	//25Hz
	{
		memc_input_frame_rate = VPQ_INPUT_25HZ;
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) == PIXEL_MODE_2_PIXEL){
			memc_output_frame_rate = VPQ_OUTPUT_100HZ; //25Hz => 100Hz
		}
		else{
			memc_output_frame_rate = VPQ_OUTPUT_50HZ; //25Hz => 50Hz
		}
	}
	else if((Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) > 295) && (Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) < 305))	//30Hz
	{
		memc_input_frame_rate = VPQ_INPUT_30HZ;
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) == PIXEL_MODE_2_PIXEL){
			memc_output_frame_rate = VPQ_OUTPUT_120HZ; //30Hz => 120Hz
		}
		else{
			memc_output_frame_rate = VPQ_OUTPUT_60HZ; //30Hz => 60Hz
		}
	}
	else if((Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) > 495) && (Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) < 505))	//50Hz
	{
		memc_input_frame_rate = VPQ_INPUT_50HZ;
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) == PIXEL_MODE_2_PIXEL){
			memc_output_frame_rate = VPQ_OUTPUT_100HZ; //50Hz => 100Hz
		}
		else{
			memc_output_frame_rate = VPQ_OUTPUT_50HZ; //50Hz => 50Hz
		}
	}
	else if((Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) > 595) && (Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) < 605))	//60hz
	{
		memc_input_frame_rate = VPQ_INPUT_60HZ;
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) == PIXEL_MODE_2_PIXEL){
			memc_output_frame_rate = VPQ_OUTPUT_120HZ; //60Hz => 120Hz
		}
		else{
			memc_output_frame_rate = VPQ_OUTPUT_60HZ; //60Hz => 60Hz
		}
	}
	else{ // others
		memc_input_frame_rate = VPQ_INPUT_60HZ;
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) == PIXEL_MODE_2_PIXEL){
			memc_output_frame_rate = VPQ_OUTPUT_120HZ; //60Hz => 120Hz
		}
		else{
			memc_output_frame_rate = VPQ_OUTPUT_60HZ; //60Hz => 60Hz
		}
	}

	rtd_pr_memc_debug("[MEMC]UZU DTG input frame_rate = %d\n",memc_input_frame_rate);
	rtd_pr_memc_debug("[MEMC]MEMC DTG output frame_rate = %d\n",memc_output_frame_rate);

	//set MEMC in/out frame rate
	//Scaler_MEMC_SetInputFrameRate(memc_input_frame_rate);
	//Scaler_MEMC_SetOutputFrameRate(memc_output_frame_rate);
	Scaler_MEMC_SetInOutputFrameRate(memc_input_frame_rate, memc_output_frame_rate);
}

void Scaler_MEMC_CLK_Enable(void){

#ifdef CONFIG_MEMC_BYPASS
	return;
#else
	sys_reg_sys_srst3_RBUS sys_reg_sys_srst3_REG;
	sys_reg_sys_clken3_RBUS sys_reg_sys_clken3_reg;
	sys_reg_sys_srst3_REG.regValue = IoReg_Read32(SYS_REG_SYS_SRST3_reg);
	sys_reg_sys_clken3_reg.regValue = IoReg_Read32(SYS_REG_SYS_CLKEN3_reg);

	rtd_pr_memc_notice("%s %d\n",__func__,__LINE__);
	rtd_pr_memc_notice("CLK Status >> ( %d ,%d ) ",(IoReg_Read32(SYS_REG_SYS_CLKEN3_reg)>>29),(IoReg_Read32(SYS_REG_SYS_SRST3_reg)>>29));

	if(((sys_reg_sys_srst3_REG.regValue>>29) != 7)){
		//SYS_RSTN b800010C
		if(sys_reg_sys_srst3_REG.rstn_memc_rbus != 1){ //Clock enable bit for rbus clock in MEMC , should be set beforce CLKEN_MEMC/CLKEN_MEME_ME
			sys_reg_sys_srst3_REG.regValue = 0;
			sys_reg_sys_srst3_REG.rstn_memc_rbus = 1;
			sys_reg_sys_srst3_REG.write_data = 1;
			IoReg_Write32(SYS_REG_SYS_SRST3_reg,sys_reg_sys_srst3_REG.regValue);
		}

		sys_reg_sys_srst3_REG.regValue = 0;
		sys_reg_sys_srst3_REG.rstn_memc= 1;
		sys_reg_sys_srst3_REG.rstn_memc_me = 1;
		sys_reg_sys_srst3_REG.write_data = 1;
		IoReg_Write32(SYS_REG_SYS_SRST3_reg,sys_reg_sys_srst3_REG.regValue);
	}

	if(((sys_reg_sys_clken3_reg.regValue>>29) != 7)){
		//SYS_CLKEN3 b800011C
		if(sys_reg_sys_clken3_reg.clken_memc_rbus != 1){ //Clock enable bit for rbus clock in MEMC , should be set beforce CLKEN_MEMC/CLKEN_MEME_ME
			sys_reg_sys_clken3_reg.regValue = 0;
			sys_reg_sys_clken3_reg.clken_memc_rbus = 1;
			sys_reg_sys_clken3_reg.write_data = 1;
			IoReg_Write32(SYS_REG_SYS_CLKEN3_reg,sys_reg_sys_clken3_reg.regValue);
		}

		sys_reg_sys_clken3_reg.regValue = 0;
		sys_reg_sys_clken3_reg.clken_memc = 1;
		sys_reg_sys_clken3_reg.clken_memc_me= 1;
		sys_reg_sys_clken3_reg.write_data = 1;
		IoReg_Write32(SYS_REG_SYS_CLKEN3_reg,sys_reg_sys_clken3_reg.regValue);
	}
#endif /* CONFIG_MEMC_BYPASS */
}

unsigned char Scaler_MEMC_CLK_Check(void){

	//rtd_pr_memc_notice( "%s %d\n",__func__,__LINE__);

#ifdef CONFIG_MEMC_BYPASS
	return FALSE;
#else
	sys_reg_sys_srst3_RBUS sys_reg_sys_srst3_REG;
	sys_reg_sys_clken3_RBUS sys_reg_sys_clken3_reg;
	sys_reg_sys_dispclksel_RBUS sys_dispclksel_reg;

	//MEMC in/out clock enable
	sys_dispclksel_reg.regValue = IoReg_Read32(SYS_REG_SYS_DISPCLKSEL_reg);
	//MEMC/me clock enable
	sys_reg_sys_clken3_reg.regValue = IoReg_Read32(SYS_REG_SYS_CLKEN3_reg);
	//MEMC/me clock reset
	sys_reg_sys_srst3_REG.regValue = IoReg_Read32(SYS_REG_SYS_SRST3_reg);

	if((sys_dispclksel_reg.clken_disp_memc_in != 1) || (sys_dispclksel_reg.clken_disp_memc_out != 1) || (((sys_reg_sys_clken3_reg.regValue>>29) != 7) && ((sys_reg_sys_clken3_reg.regValue>>29) != 5)) || (((sys_reg_sys_srst3_REG.regValue>>29) != 7) && ((sys_reg_sys_srst3_REG.regValue>>29) != 5))){
		//rtd_pr_memc_notice( " Warning !! CLK not enable!!( %d, %d, %d,%d )",sys_dispclksel_reg.clken_disp_memc_in, sys_dispclksel_reg.clken_disp_memc_out, (sys_reg_sys_clken3_reg.regValue>>29),(sys_reg_sys_srst3_REG.regValue>>29));
		return FALSE;
	}else{
		return TRUE;
	}
#endif

}

void Scaler_MEMC_CLK(void){
	sys_reg_sys_dispclksel_RBUS sys_dispclksel_reg;

	//MEMC lantency temp setting
	//rtd_outl(MEMC_LATENCY01_ADDR, 0x00000000);
	//rtd_outl(MEMC_LATENCY02_ADDR, 0x00000000);

	//MEMC in/out clock enable
	sys_dispclksel_reg.regValue = IoReg_Read32(SYS_REG_SYS_DISPCLKSEL_reg);
	//sys_dispclksel_reg.clken_disp_memc_in= 1;//mac7p pq compile fix
	//sys_dispclksel_reg.clken_disp_memc_out= 1;//mac7p pq compile fix
	//sys_dispclksel_reg.clken_disp_memc= 1;    //mac7p pq compile fix
	IoReg_Write32(SYS_REG_SYS_DISPCLKSEL_reg, sys_dispclksel_reg.regValue);
}

void Scaler_MEMC_ForceBg(UINT8 bEnable)
{
	ppoverlay_memc_mux_ctrl_RBUS memc_mux_ctrl_reg;
	ppoverlay_double_buffer_ctrl2_RBUS double_buffer_ctrl_reg;

	memc_mux_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg);
	memc_mux_ctrl_reg.memc_out_bg_en = bEnable;
	IoReg_Write32(PPOVERLAY_MEMC_MUX_CTRL_reg, memc_mux_ctrl_reg.regValue);

	//set apply
	double_buffer_ctrl_reg.regValue =  IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	double_buffer_ctrl_reg.uzudtgreg_dbuf_set = 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl_reg.regValue);
	double_buffer_ctrl_reg.regValue =  IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
}


void Scaler_MEMC_Mux(void){
#ifndef CONFIG_MEMC_BYPASS
	//MEMC mux
	ppoverlay_memc_mux_ctrl_RBUS memc_mux_ctrl_reg;
#endif
	ppoverlay_double_buffer_ctrl2_RBUS double_buffer_ctrl_reg;

	double_buffer_ctrl_reg.regValue =  IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	double_buffer_ctrl_reg.uzudtgreg_dbuf_en = 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl_reg.regValue);

#ifndef CONFIG_MEMC_BYPASS
	//MEMC mux enable
	drvif_memc_mux_en(TRUE);

	//DTG MEMC MUX control
	memc_mux_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg);

	memc_mux_ctrl_reg.memc_outmux_sel = 1;
	//memc_mux_ctrl_reg.memcdtg_golden_vs = 0;//disable memc golden vsync
	IoReg_Write32(PPOVERLAY_MEMC_MUX_CTRL_reg, memc_mux_ctrl_reg.regValue);
#endif
	//set apply
	double_buffer_ctrl_reg.regValue =  IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	double_buffer_ctrl_reg.uzudtgreg_dbuf_set = 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl_reg.regValue);
#if 0
	if(vbe_disp_decide_memc_mux_bypass()==TRUE){
		//disable memc clk
		Scaler_MEMC_Bypass_CLK_OnOff(0);
		if(Scaler_VOFromVDEC(Scaler_Get_CurVoInfo_plane()) == SOURCE_VIDEO){//DTV case
			rtd_pr_memc_notice("DTV 4K, bypass PQ Lv2\n");
			Scaler_color_Set_PQ_ByPass_Lv(2);
		}else if(Scaler_VOFromVDEC(Scaler_Get_CurVoInfo_plane()) == SOURCE_DIRECTVO){
			rtd_pr_memc_notice("Playback 4K, bypass PQ Lv3\n");
			Scaler_color_Set_PQ_ByPass_Lv(3);
		}
	}else{
		Scaler_MEMC_Bypass_CLK_OnOff(1);
	}
#else
#if 0
	if(vbe_disp_decide_PQ_power_saveing() == TRUE){
		if(Scaler_VOFromVDEC(Scaler_Get_CurVoInfo_plane()) == SOURCE_VIDEO){//DTV case
			rtd_pr_memc_notice("DTV 4K, bypass PQ Lv2\n");
			Scaler_color_Set_PQ_ByPass_Lv(3);
		}else if(Scaler_VOFromVDEC(Scaler_Get_CurVoInfo_plane()) == SOURCE_DIRECTVO){
			rtd_pr_memc_notice("Playback 4K, bypass PQ Lv3\n");
			Scaler_color_Set_PQ_ByPass_Lv(3);
		}
	}
#endif
#endif
}

void Scaler_MEMC_outMux(UINT8 dbuf_en,UINT8 outmux_sel)
{
	//mac7p pq compile fix
}

//Reset flow : CLKEN = "0" 'RSTN = "0" 'RSTN = "1" ' CLKEN = "1"
void Scaler_MEMC_Bypass_CLK_OnOff(unsigned char clk_OnOff)
{
#if 0
	sys_reg_sys_srst3_RBUS sys_reg_sys_srst3_reg;
	sys_reg_sys_clken3_RBUS sys_reg_sys_clken3_reg;

	//sys_reg_sys_srst3_reg.regValue = IoReg_Read32(SYS_REG_SYS_SRST3_reg);
	//sys_reg_sys_clken3_reg.regValue = IoReg_Read32(SYS_REG_SYS_CLKEN3_reg);
	sys_reg_sys_srst3_reg.regValue = 0;
	sys_reg_sys_clken3_reg.regValue = 0;

	if(clk_OnOff == 0){
		rtd_pr_memc_emerg("MEMC CLK OFF\n");
		// clear memc_in interrupt
		IoReg_ClearBits(0xb8099018, _BIT25);
		// clear memc_out interrupt
		IoReg_ClearBits(0xb809d008, _BIT6);
		sys_reg_sys_clken3_reg.clken_memc = 1;
		sys_reg_sys_clken3_reg.clken_memc_me = 1;
		sys_reg_sys_clken3_reg.write_data = 0;
		IoReg_Write32(SYS_REG_SYS_CLKEN3_reg, sys_reg_sys_clken3_reg.regValue);
		sys_reg_sys_srst3_reg.rstn_memc = 1;
		sys_reg_sys_srst3_reg.rstn_memc_me = 1;
		sys_reg_sys_srst3_reg.write_data = 0;
		IoReg_Write32(SYS_REG_SYS_SRST3_reg, sys_reg_sys_srst3_reg.regValue);
	}else{
		rtd_pr_memc_emerg("MEMC CLK ON\n");
		sys_reg_sys_srst3_reg.rstn_memc = 1;
		sys_reg_sys_srst3_reg.rstn_memc_me = 1;
		sys_reg_sys_srst3_reg.write_data = 1;
		IoReg_Write32(SYS_REG_SYS_SRST3_reg, sys_reg_sys_srst3_reg.regValue);
		sys_reg_sys_clken3_reg.clken_memc = 1;
		sys_reg_sys_clken3_reg.clken_memc_me = 1;
		sys_reg_sys_clken3_reg.write_data = 1;
		IoReg_Write32(SYS_REG_SYS_CLKEN3_reg, sys_reg_sys_clken3_reg.regValue);
		// enable memc_in interrupt
		IoReg_SetBits(0xb8099018, _BIT25);
		// enable memc_out interrupt
		IoReg_SetBits(0xb809d008, _BIT6);
	}
#endif
}

void Scaler_MEMC_Mux_off(void){
	//MEMC mux
	ppoverlay_memc_mux_ctrl_RBUS memc_mux_ctrl_reg;

#ifdef CONFIG_HW_SUPPORT_MEMC
	//MEMC mux enable
	drvif_memc_mux_en(FALSE);
#endif //CONFIG_HW_SUPPORT_MEMC

	//DTG MEMC MUX control
	memc_mux_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg);
	memc_mux_ctrl_reg.memc_outmux_sel = 0;
	IoReg_Write32(PPOVERLAY_MEMC_MUX_CTRL_reg, memc_mux_ctrl_reg.regValue);
}

int Scaler_MEMC_MEMC_CLK_OnOff(unsigned char ME_enable, unsigned char MC_enable, unsigned char bMute)
{
	//mac7p pq compile fix
	return 0;
}

int Scaler_MEMC_DTV_ME_CLK_OnOff(unsigned char enable, unsigned char bMode)
{
	//mac7p pq compile fix
	return 0;
}

void Scaler_MEMC_Bypass_On(unsigned char b_isBypass){
#ifndef CONFIG_MEMC_BYPASS
	ppoverlay_memc_mux_ctrl_RBUS memc_mux_ctrl_reg;
#endif
	unsigned int count = 100;

	ppoverlay_double_buffer_ctrl2_RBUS ppoverlay_double_buffer_ctrl2_reg;
	ppoverlay_double_buffer_ctrl2_reg.regValue =  IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);

	rtd_pr_memc_emerg("VPQ Scaler_MEMC_Bypass_On (%d)\n", b_isBypass);
#ifndef CONFIG_MEMC_BYPASS
	memc_mux_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg);
	if(b_isBypass)
		memc_mux_ctrl_reg.memc_outmux_sel = 0;
	else
		memc_mux_ctrl_reg.memc_outmux_sel = 1;
	IoReg_Write32(PPOVERLAY_MEMC_MUX_CTRL_reg, memc_mux_ctrl_reg.regValue);
#endif
	//set apply
	ppoverlay_double_buffer_ctrl2_reg.regValue =  IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_set = 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);

	while((ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_set)&& --count){
		ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
		msleep(10);
	}
}

unsigned char Scaler_MEMC_get_Bypass_Status(void)
{
	ppoverlay_memc_mux_ctrl_RBUS memc_mux_ctrl_reg;
	memc_mux_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg);
	if(memc_mux_ctrl_reg.memc_outmux_sel)
		return FALSE;
	else
		return TRUE;
}


void Scaler_MEMC_allocate_memory(void){
	//unsigned int vir_addr;/*, phy_addr_kme, phy_addr_kmc00, phy_addr_kmc01;*/
#if 1

	unsigned int memc_size = 0;
	unsigned int memc_addr = 0;
#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
	memc_size = carvedout_buf_query_secure(CARVEDOUT_SCALER_MEMC, (void *)&memc_addr);
#else
	memc_size = carvedout_buf_query(CARVEDOUT_SCALER_MEMC, (void *)&memc_addr);
#endif
	if (memc_addr == 0 || memc_size == 0) {
		rtd_pr_memc_emerg("[%s %d] ERR : %x %x\n", __func__, __LINE__, memc_addr, memc_size);
		return;
	}


	gphy_addr_kme = memc_addr;
	gphy_addr_kmc00 = gphy_addr_kme + KME_TOTAL_SIZE;
	gphy_addr_kmc01 = gphy_addr_kmc00 + MEMC_KMC_00_START_TOTAL_SIZE;
	rtd_pr_memc_emerg("KME start address = %x\n", gphy_addr_kme);
	rtd_pr_memc_emerg("KMC00 start address = %x\n", gphy_addr_kmc00);
	rtd_pr_memc_emerg("KMC01 start address = %x\n", gphy_addr_kmc01);

	if(KME_TOTAL_SIZE > memc_size){
		rtd_pr_memc_emerg("[KME_TOTAL_SIZE] setting error");
		return;
	}

#ifdef CONFIG_HW_SUPPORT_MEMC
	//ME memory boundary
	IoReg_Write32(MEMC_DBUS_WRAP_Me_downlimit_addr_reg, gphy_addr_kme);
	IoReg_Write32(MEMC_DBUS_WRAP_Me_uplimit_addr_reg, (gphy_addr_kme + KME_TOTAL_SIZE));
	//MC memory boundary
	IoReg_Write32(MEMC_DBUS_WRAP_Mc_downlimit_addr_reg, gphy_addr_kmc00);
	IoReg_Write32(MEMC_DBUS_WRAP_Mc_uplimit_addr_reg, (gphy_addr_kmc00 + KMC_TOTAL_SIZE));
#endif //CONFIG_HW_SUPPORT_MEMC
#else
	unsigned char	*m_pCacheStartAddr_kme = NULL;
	unsigned int 	m_allocate_memory_kme = 0;
	unsigned char	*m_pCacheStartAddr_kmc00 = NULL;
	unsigned int 	m_allocate_memory_kmc00 = 0;

	rtd_pr_memc_err("+++Scaler_MEMC_allocate_memory+++\n");

	m_pCacheStartAddr_kme = (unsigned char *)dvr_malloc_specific(KME_TOTAL_SIZE, GFP_DCU2_FIRST);
	if(m_pCacheStartAddr_kme == NULL){
	rtd_pr_memc_err("[ERROR]Allocate DCU2 size=%x fail\n",KME_TOTAL_SIZE);
	BUG();//add kernel crash function when cma not enough
	return FALSE;
	}
	m_allocate_memory_kme = (unsigned int)dvr_to_phys((void*)m_pCacheStartAddr_kme);

	m_pCacheStartAddr_kmc00 = (unsigned char *)dvr_malloc_specific(KMC_TOTAL_SIZE, GFP_DCU2_FIRST);
	if(m_pCacheStartAddr_kmc00 == NULL){
	rtd_pr_memc_err("[ERROR]Allocate DCU2 size=%x fail\n",KMC_TOTAL_SIZE);
	BUG();//add kernel crash function when cma not enough
	return FALSE;
	}
	m_allocate_memory_kmc00 = (unsigned int)dvr_to_phys((void*)m_pCacheStartAddr_kmc00);

	gphy_addr_kme = m_allocate_memory_kme;
	gphy_addr_kmc00 = m_allocate_memory_kmc00;
	gphy_addr_kmc01 = gphy_addr_kmc00 + MEMC_KMC_00_START_TOTAL_SIZE;

	//ME memory boundary
	IoReg_Write32(MEMC_DBUS_WRAP_Me_downlimit_addr_reg, gphy_addr_kme);
	IoReg_Write32(MEMC_DBUS_WRAP_Me_uplimit_addr_reg, (gphy_addr_kme + KME_TOTAL_SIZE));
	//MC memory boundary
	IoReg_Write32(MEMC_DBUS_WRAP_Mc_downlimit_addr_reg, gphy_addr_kmc00);
	IoReg_Write32(MEMC_DBUS_WRAP_Mc_uplimit_addr_reg, (gphy_addr_kmc00 + KMC_TOTAL_SIZE));

	rtd_pr_memc_notice("KME_TOTAL_SIZE=%x\n", KME_TOTAL_SIZE);
	rtd_pr_memc_notice("KMC_TOTAL_SIZE=%x\n", KMC_TOTAL_SIZE);
	rtd_pr_memc_notice("MEMC_KMC_00_START_TOTAL_SIZE=%x\n", MEMC_KMC_00_START_TOTAL_SIZE);
	rtd_pr_memc_notice("MEMC_KMC_01_START_TOTAL_SIZE=%x\n", MEMC_KMC_01_START_TOTAL_SIZE);

	rtd_pr_memc_notice("gphy_addr_kme=%x\n", gphy_addr_kme);
	rtd_pr_memc_notice("gphy_addr_kmc00=%x\n", gphy_addr_kmc00);
	rtd_pr_memc_notice("gphy_addr_kmc01=%x\n", gphy_addr_kmc01);
#endif

}

void Scaler_MEMC_initial_Case(void){
	// MEMC 4k60 in, 4k60 out
#ifdef CONFIG_RTK_KDEV_DEBUG_ISR
	int ret;
#endif
	SCALER_MEMC_DMASTARTADDRESS_T* memc_dma_address_info;
	unsigned int ulCount = 0;
	VPQ_MEMC_INPUT_FORMAT input_format;
	VPQ_MEMC_OUTPUT_FORMAT output_format;
	ppoverlay_memcdtg_dv_den_start_end_RBUS memcdtg_dv_den_start_end_reg;

	memcdtg_dv_den_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_memcdtg_DV_DEN_Start_End_reg);
#ifdef CONFIG_MEMC_BYPASS
	//memcdtg_dv_den_start_end_reg.memcdtg_dv_den_sta = Get_DISP_DEN_STA_VPOS();
	//memcdtg_dv_den_start_end_reg.memcdtg_dv_den_end = Get_DISP_DEN_END_VPOS();
	return;
#else // memcdtg start position should sync to MEMC HW
	memcdtg_dv_den_start_end_reg.memcdtg_dv_den_sta = 45;
	memcdtg_dv_den_start_end_reg.memcdtg_dv_den_end = 2205;
#endif
	IoReg_Write32(PPOVERLAY_memcdtg_DV_DEN_Start_End_reg, memcdtg_dv_den_start_end_reg.regValue);

	fw_scaler_dtg_double_buffer_apply();

	//MEMC CLK Init
	Scaler_MEMC_CLK();
	memc_dma_address_info = (SCALER_MEMC_DMASTARTADDRESS_T*)Scaler_GetShareMemVirAddr(SCALERIOC_MEMC_INITIALIZATION);
	ulCount = sizeof(SCALER_MEMC_DMASTARTADDRESS_T) / sizeof(unsigned int);

	memc_dma_address_info->ME_start_addr= gphy_addr_kme;
	memc_dma_address_info->MC00_start_addr = gphy_addr_kmc00;
	memc_dma_address_info->MC01_start_addr = gphy_addr_kmc01;
#ifdef CONFIG_RTK_KDEV_DEBUG_ISR
	//change endian
	memc_dma_address_info->ME_start_addr = htonl(memc_dma_address_info->ME_start_addr);
	memc_dma_address_info->MC00_start_addr = htonl(memc_dma_address_info->MC00_start_addr);
	memc_dma_address_info->MC01_start_addr = htonl(memc_dma_address_info->MC01_start_addr);

	if (0 != (ret = Scaler_SendRPC(SCALERIOC_MEMC_INITIALIZATION, 0, 0))){
		rtd_pr_memc_debug("[MEMC]ret=%d, SCALERIOC_MEMC_INITIALIZATION RPC fail !!!\n", ret);
	}
#else
#ifdef CONFIG_HW_SUPPORT_MEMC
	Scaler_MEMC_Initialize(memc_dma_address_info->ME_start_addr, memc_dma_address_info->MC00_start_addr, memc_dma_address_info->MC01_start_addr);

#endif
#endif
	input_format = VPQ_INPUT_2D;
	output_format = VPQ_OUTPUT_2D;
	//set MEMC in,out use case
	Scaler_MEMC_SetInOutUseCase(VPQ_INPUT_3840X2160, VPQ_OUTPUT_3840X2160, input_format, output_format);
	//set MEMC in/out frame rate
	Scaler_MEMC_SetInOutFrameRate();
}

unsigned char Get_MEMC_Enable_Dynamic(void);

/*api which have protect by forcebg_semaphore*/
void Scaler_MEMC_fbg_control(void){
	if(MEMC_First_Run_FBG_enable == _ENABLE){
		msleep(50);
		Scaler_MEMC_output_force_bg_enable(FALSE);
		MEMC_First_Run_FBG_enable = _DISABLE;
		rtd_pr_memc_notice("[MEMC]MEMC_First_Run_FBG_enable disable FBG!!\n");
	}
	if(MEMC_instanboot_resume_Done == 1){
		msleep(50);
		Scaler_MEMC_output_force_bg_enable(FALSE);
		MEMC_instanboot_resume_Done = 0;
		rtd_pr_memc_notice("[MEMC]MEMC_instanboot_resume_Done disable FBG!!\n");
	}
	if(MEMC_snashop_resume_Done == 1){
		Scaler_MEMC_output_force_bg_enable(FALSE);
		MEMC_snashop_resume_Done = 0;
		rtd_pr_memc_notice("[MEMC]MEMC_snashop_resume_Done disable FBG!!\n");
	}
//set MEMC in, out frame rate
	//Scaler_MEMC_SetInOutFrameRate();//Can not send RPC here
	rtd_pr_memc_notice("[MEMC]MEMC_instanboot_resume_Done = %d\n", MEMC_instanboot_resume_Done);
}

void Scaler_MEMC_Handler(){
	VPQ_MEMC_INPUT_FORMAT input_format;
	VPQ_MEMC_OUTPUT_FORMAT output_format;

	if(MEMC_First_Run_Done == 0){
		//memc initial RPC
		MEMC_First_Run_Done = 1;
		MEMC_First_Run_FBG_enable = _ENABLE;
		Scaler_MEMC_output_force_bg_enable(TRUE);
		Scaler_MEMC_initial_Case();
		rtd_pr_memc_debug("[MEMC]initial in handler!!\n");
	}
#if 0
	// 2D, 3D switch
	if((get_3D_mode_enable() == true) && (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() == true)){
		//K2L on support PR 3D
		input_format = VPQ_INPUT_3D_LBL;
		output_format = VPQ_OUTPUT_3D_PR;
	}
	else{
		input_format = VPQ_INPUT_2D;
		output_format = VPQ_OUTPUT_2D;
	}

#else
	input_format = VPQ_INPUT_2D;
	output_format = VPQ_OUTPUT_2D;
#endif

	if(MEMC_instanboot_resume_Done == 1){
		Scaler_MEMC_SetInOutUseCase(VPQ_INPUT_3840X2160, VPQ_OUTPUT_3840X2160, VPQ_INPUT_3D_LBL, VPQ_OUTPUT_3D_PR);
		//msleep(100);//Remove by Will for performance
		rtd_pr_memc_debug("[MEMC]reset usecase (instanboot)!!\n");
	}
	//MEMC mux
	Scaler_MEMC_Mux();
	//set MEMC in,out use case
	Scaler_MEMC_SetInOutUseCase(VPQ_INPUT_3840X2160, VPQ_OUTPUT_3840X2160, input_format, output_format);

	//Instanboot to initial phase table
	if(MEMC_instanboot_resume_Done == 1){
		Scaler_MEMC_Instanboot_InitPhaseTable(1);
		rtd_pr_memc_debug("[MEMC]initial phase table (instanboot)!!\n");
	}
	//set MEMC in, out frame rate
	//Scaler_MEMC_SetInOutFrameRate();//Move after disable force bg

	rtd_pr_memc_debug("[MEMC]Scaler_MEMC_Handler Done!!\n");
}
#ifdef CONFIG_HW_SUPPORT_MEMC

#ifdef MEMC_DYNAMIC_ONOFF
void Scaler_ME_OnOff(UINT8 enable)
{
	unsigned int enable_bit;
	if(enable == TRUE)
		enable_bit = 0x00100000;
	else
		enable_bit = 0x0;



	IoReg_Mask32(me_00_write_enable_ADDR, 0xffefffff, enable_bit);
	IoReg_Mask32(me_01_read_enable_ADDR, 0xffefffff, enable_bit);
	IoReg_Mask32(me_02_read_enable_ADDR, 0xffefffff, enable_bit);
	IoReg_Mask32(me_03_read_enable_ADDR, 0xffefffff, enable_bit);
	IoReg_Mask32(me_04_write_enable_ADDR, 0xffefffff, enable_bit);
	IoReg_Mask32(me_05_write_enable_ADDR, 0xffefffff, enable_bit);
	IoReg_Mask32(me_06_read_enable_ADDR, 0xffefffff, enable_bit);
	IoReg_Mask32(me_07_read_enable_ADDR, 0xffefffff, enable_bit);
	IoReg_Mask32(me_08_write_enable_ADDR, 0xffefffff, enable_bit);
	IoReg_Mask32(me_09_read_enable_ADDR, 0xffefffff, enable_bit);
	IoReg_Mask32(me_10_write_enable_ADDR, 0xffefffff, enable_bit);
	IoReg_Mask32(me_11_read_enable_ADDR, 0xffefffff, enable_bit);
	IoReg_Mask32(me_12_write_enable_ADDR, 0xffefffff, enable_bit);
	IoReg_Mask32(me_13_read_enable_ADDR, 0xffefffff, enable_bit);
	IoReg_Mask32(me_14_write_enable_ADDR, 0xffefffff, enable_bit);
	IoReg_Mask32(me_15_read_enable_ADDR, 0xffefffff, enable_bit);
}

void Scaler_MC_OnOff(UINT8 enable){
	unsigned int enable_bit;
	if(enable == TRUE)
		enable_bit = 0x00100000;
	else
		enable_bit = 0x0;

	IoReg_Mask32(mc_00_write_enable_ADDR, 0xffefffff, enable_bit);
	IoReg_Mask32(mc_00_write_enable_ADDR, 0xffefffff, enable_bit);
	IoReg_Mask32(mc_01_write_enable_ADDR, 0xffefffff, enable_bit);
	IoReg_Mask32(mc_02_read_enable_ADDR, 0xffefffff, enable_bit);
	IoReg_Mask32(mc_03_read_enable_ADDR, 0xffefffff, enable_bit);
	IoReg_Mask32(mc_04_read_enable_ADDR, 0xffefffff, enable_bit);
	IoReg_Mask32(mc_05_read_enable_ADDR, 0xffefffff, enable_bit);
	IoReg_Mask32(mc_06_read_enable_ADDR, 0xffefffff, enable_bit);
	IoReg_Mask32(mc_07_read_enable_ADDR, 0xffefffff, enable_bit);
}

static UINT8 no_signal_MEMC_outputbg_check = 0;
void Scaler_MEMC_Booting_NoSignal_Check_OutputBG(UINT8 enable){
	//check MEMC out bg when not run scaler
	ppoverlay_memc_mux_ctrl_RBUS memc_mux_ctrl_reg;
	ppoverlay_double_buffer_ctrl2_RBUS memc_double_buffer_ctrl_reg;
	memc_mux_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg);

	//add checking no signal status @Crixus 20160112
	if(Scaler_MEMC_Get_First_Run_FBG_enable() || Scaler_MEMC_Get_instanboot_resume_Done() || Scaler_MEMC_Get_snashop_resume_Done()){
		if((enable == 0) && (memc_mux_ctrl_reg.memc_out_bg_en == 1)){
			memc_mux_ctrl_reg.memc_out_bg_en = 0;
			no_signal_MEMC_outputbg_check = 1;
			Scaler_MEMC_Handler();
#ifdef CONFIG_HW_SUPPORT_MEMC
			memc_mux_ctrl_reg.memcdtg_golden_vs = 0;
#endif //CONFIG_HW_SUPPORT_MEMC
			rtd_pr_memc_notice("\r\n####[MEMC]No signal and disable MEMC out bg\n");
		}
		else{
			if(no_signal_MEMC_outputbg_check == 1){
				if((enable == 1) && (memc_mux_ctrl_reg.memc_out_bg_en == 0)){
					memc_mux_ctrl_reg.memc_out_bg_en = 1;
#ifdef CONFIG_HW_SUPPORT_MEMC
					memc_mux_ctrl_reg.memcdtg_golden_vs = 1;
#endif //CONFIG_HW_SUPPORT_MEMC
					no_signal_MEMC_outputbg_check = 0;
					rtd_pr_memc_notice("\r\n####[MEMC]No signal and enable MEMC out bg\n");
				}
			}
		}
		IoReg_Write32(PPOVERLAY_MEMC_MUX_CTRL_reg, memc_mux_ctrl_reg.regValue);

		//set apply
		memc_double_buffer_ctrl_reg.regValue =  IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
		memc_double_buffer_ctrl_reg.uzudtgreg_dbuf_set = 1;
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, memc_double_buffer_ctrl_reg.regValue);
	}
}


static UINT8 MEMC_Dynamic_OnOff = 1;//default MEMC enable
extern VSC_INPUT_TYPE_T Get_DisplayMode_Src(unsigned char display);
extern unsigned char rtk_run_scaler(unsigned char display, VSC_INPUT_TYPE_T inputType, KADP_VSC_OUTPUT_MODE_T outputMode);
void Scaler_MEMC_Dynamic_OnOff(){
	//MDOMAIN_CAP_Smooth_tog_ctrl_0_reg[1] => dummy bit to control dynamic MEMC on/off.

	//MEMC change to disable
	if((IoReg_Read32(MDOMAIN_CAP_Smooth_tog_ctrl_0_reg) & _BIT1) && (MEMC_Dynamic_OnOff == 1)){
		rtk_run_scaler(SLR_MAIN_DISPLAY, Get_DisplayMode_Src(SLR_MAIN_DISPLAY), KADP_VSC_OUTPUT_DISPLAY_MODE);
		Scaler_ME_OnOff(FALSE);
		Scaler_MC_OnOff(FALSE);
		Scaler_MEMC_Mux_off();
		MEMC_Dynamic_OnOff = 0;
		rtd_pr_memc_debug("[MEMC]Dynamic off!!!!\n");
	}
	//MEMC change to enable
	else if(!(IoReg_Read32(MDOMAIN_CAP_Smooth_tog_ctrl_0_reg) & _BIT1) && (MEMC_Dynamic_OnOff == 0)){
		rtk_run_scaler(SLR_MAIN_DISPLAY, Get_DisplayMode_Src(SLR_MAIN_DISPLAY), KADP_VSC_OUTPUT_DISPLAY_MODE);
		Scaler_ME_OnOff(TRUE);
		Scaler_MC_OnOff(TRUE);
		MEMC_Dynamic_OnOff = 1;
		rtd_pr_memc_debug("[MEMC]Dynamic on!!!!\n");
	}
}

UINT8 Scaler_MEMC_GetDynamicOnOff(void){
	return MEMC_Dynamic_OnOff;
}
#endif
#endif

/*api which have protect by forcebg_semaphore*/
void Scaler_MEMC_output_force_bg_enable(UINT8 enable){
	//MEMC enable, using MEMC out fbg
	ppoverlay_double_buffer_ctrl2_RBUS double_buffer_ctrl_reg;
	ppoverlay_memc_mux_ctrl_RBUS memc_mux_ctrl_reg;
	UINT8 timeout = 0;

	rtd_pr_memc_debug("%s,MEMC out FBG = %d\n", __FUNCTION__, enable);
	//enable UZU DTG double buffer.
	double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	double_buffer_ctrl_reg.uzudtgreg_dbuf_en = 1;
	double_buffer_ctrl_reg.uzudtg_dbuf_vsync_sel = 3;//using uzudtg vsync
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl_reg.regValue);
	//enable MEMC output fbg
	memc_mux_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg);
	memc_mux_ctrl_reg.memc_out_bg_en = enable;
	IoReg_Write32(PPOVERLAY_MEMC_MUX_CTRL_reg, memc_mux_ctrl_reg.regValue);
	//set apply
	double_buffer_ctrl_reg.regValue =  IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	double_buffer_ctrl_reg.uzudtgreg_dbuf_set = 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl_reg.regValue);
	double_buffer_ctrl_reg.regValue =  IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);

	while(double_buffer_ctrl_reg.uzudtgreg_dbuf_set) {
		timeout++;
		msleep(10);
		double_buffer_ctrl_reg.regValue =  IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
		if(timeout >= 5)
		break;
	}
	//disable UZU DTG double buffer.
	double_buffer_ctrl_reg.regValue =  IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	double_buffer_ctrl_reg.uzudtgreg_dbuf_en = 0;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl_reg.regValue);
}

void Scaler_MEMC_set_output_bg_color(UINT16 color_r, UINT16 color_g, UINT16 color_b){
	ppoverlay_memc_mux_out1_color_value_gb_RBUS memc_mux_out1_color_value_gb_reg;
	ppoverlay_memc_mux_out1_color_value_r_RBUS memc_mux_out1_color_value_r_reg;

	memc_mux_out1_color_value_gb_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_OUT1_COLOR_VALUE_GB_reg);
	memc_mux_out1_color_value_r_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_OUT1_COLOR_VALUE_R_reg);

	memc_mux_out1_color_value_gb_reg.out_y_g_value = color_g;
	memc_mux_out1_color_value_gb_reg.out_cb_b_value = color_b;
	memc_mux_out1_color_value_r_reg.out_cr_r_value = color_r;

	IoReg_Write32(PPOVERLAY_MEMC_MUX_OUT1_COLOR_VALUE_GB_reg, memc_mux_out1_color_value_gb_reg.regValue);
	IoReg_Write32(PPOVERLAY_MEMC_MUX_OUT1_COLOR_VALUE_R_reg, memc_mux_out1_color_value_r_reg.regValue);
}

void Scaler_MEMC_SetMotionType(VPQ_MEMC_TYPE_T motion)
{
	MEMC_motion_type = motion;
}
VPQ_MEMC_TYPE_T Scaler_MEMC_GetMotionType(void)
{
	return MEMC_motion_type;
}
void Scaler_MEMC_Set_CheckMEMC_Outbg(unsigned char mode)
{
	AVD_Unstable_Check_MEMC_OutBG_flag = mode;
}
unsigned char Scaler_MEMC_Get_CheckMEMC_Outbg(void)
{
	return AVD_Unstable_Check_MEMC_OutBG_flag;
}
unsigned char Scaler_MEMC_Get_First_Run_FBG_enable(void)
{
	return MEMC_First_Run_FBG_enable;
}
unsigned char Scaler_MEMC_Get_instanboot_resume_Done(void)
{
	return MEMC_instanboot_resume_Done;
}
unsigned char Scaler_MEMC_Get_snashop_resume_Done(void)
{
	return MEMC_snashop_resume_Done;
}




int Scaler_MEMC_set_cinema_mode_en(unsigned char bEn)
{
	int ret = 0;

	rtd_pr_memc_debug("[MEMC]fwif_color_set_cinema_mode_en, bEn = %d\n", bEn);
#ifdef CONFIG_RTK_KDEV_DEBUG_ISR
	if (0 != (ret = Scaler_SendRPC(SCALERIOC_MEMC_SETCINEMAMODE, bEn, 1))){
		rtd_pr_memc_debug("[MEMC]ret=%d, SCALERIOC_MEMC_SETCINEMAMODE RPC fail !!!\n", ret);
	}
#else
#ifdef CONFIG_HW_SUPPORT_MEMC
	#ifndef CONFIG_MEMC_TASK_QUEUE
		if(!Scaler_MEMC_GetMEMC_Enable())
			return 0;
	Scaler_MEMC_SetCinemaMode(bEn);
	#else
		SCALER_MEMC_TASK_T task;

		task.type = SCALERIOC_MEMC_SETCINEMAMODE;
		task.data.value = bEn;
		Scaler_MEMC_PushTask(&task);
	#endif
#endif
#endif
	return ret;
}

BOOL ScalerMEMC_EnablePowerSave(void)
{

	_clues* SmartPic_clue = (_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SMARTPIC_CLUS);
	_RPC_clues* RPC_SmartPic_clue = (_RPC_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_SMARTPIC_CLUS);

	if(SmartPic_clue == NULL || RPC_SmartPic_clue == NULL)
		return 1;

#ifdef CONFIG_HW_SUPPORT_MEMC
	if(RPC_SmartPic_clue->prePowerSaveStatus == PS_OFF)
	{
		if(Scaler_MEMC_SetPowerSaveOn())
			return 1;

		if(Scaler_MEMC_GetMEMC_Mode() != MEMC_OFF)
		{
			Scaler_MEMC_SetMEMCFrameRepeatEnable(1);
		}

		RPC_SmartPic_clue->prePowerSaveStatus = PS_ON;
	}

#endif
	return 0;

}

BOOL ScalerMEMC_DisablePowerSave(void)
{

	//_clues* SmartPic_clue = (_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SMARTPIC_CLUS);
	_RPC_clues* RPC_SmartPic_clue = (_RPC_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_SMARTPIC_CLUS);

	if(RPC_SmartPic_clue == NULL)
		return 1;

#ifdef CONFIG_HW_SUPPORT_MEMC
	if(RPC_SmartPic_clue->prePowerSaveStatus == PS_ON)
	{
		if(Scaler_MEMC_SetPowerSaveOff())
			return 1;

		if(Scaler_MEMC_GetMEMC_Mode() != MEMC_OFF)
		{
			Scaler_MEMC_SetMEMCFrameRepeatEnable(0);
		}

		RPC_SmartPic_clue->prePowerSaveStatus = PS_OFF;
	}

#endif
	return 0;

}
/*******************************************************************************
****************************VPQ MEMC DEVICE************************************
*******************************************************************************/
#ifdef CONFIG_PM
static int vpq_memc_suspend (struct device *p_dev)
{
	rtd_pr_memc_notice( "%s %d\n",__func__,__LINE__);
	return 0;
}

static int vpq_memc_resume (struct device *p_dev)
{
	rtd_pr_memc_notice( "%s %d\n",__func__,__LINE__);
	return 0;
}
#ifdef CONFIG_HIBERNATION
static int vpq_memc_suspend_snashop (struct device *p_dev)
{
	rtd_pr_memc_notice( "%s %d\n",__func__,__LINE__);
	return 0;
}

static int vpq_memc_resume_snashop (struct device *p_dev)
{
	rtd_pr_memc_notice( "%s %d\n",__func__,__LINE__);
	return 0;
}
#endif

VPQ_MEMC_SUSPEND_RESUME_T vpq_memc_suspend_resume;
void vpq_memc_disp_suspend_snashop(void){
#ifdef CONFIG_HW_SUPPORT_MEMC
	rtd_pr_memc_notice( "%s %d\n",__func__,__LINE__);
	//MEMC mux
	vpq_memc_suspend_resume.MEMC_MUX_MEMC_MUX_FUNC_CTRL_SUSPEND = IoReg_Read32(MEMC_MUX_MEMC_MUX_FUNC_CTRL_reg);
	//FBG
	vpq_memc_suspend_resume.PPOVERLAY_MEMC_MUX_CTRL_SUSPEND = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg);
	//clock
	vpq_memc_suspend_resume.SYS_REG_SYS_DISPCLKSEL_SUSPEND = IoReg_Read32(SYS_REG_SYS_DISPCLKSEL_reg);
	vpq_memc_suspend_resume.MEMC_LATENCY01 = IoReg_Read32(MEMC_LATENCY01_ADDR);
	vpq_memc_suspend_resume.MEMC_LATENCY02 = IoReg_Read32(MEMC_LATENCY02_ADDR);

	if(Scaler_MEMC_CLK_Check() == FALSE){
		return;
	}
	//ME memory boundary
	vpq_memc_suspend_resume.MEMC_DBUS_WRAP_ME_DOWNLIMIT = IoReg_Read32(MEMC_DBUS_WRAP_Me_downlimit_addr_reg);
	vpq_memc_suspend_resume.MEMC_DBUS_WRAP_ME_UPLIMIT = IoReg_Read32(MEMC_DBUS_WRAP_Me_uplimit_addr_reg);
	//MC memory boundary
	vpq_memc_suspend_resume.MEMC_DBUS_WRAP_MC_DOWNLIMIT = IoReg_Read32(MEMC_DBUS_WRAP_Mc_downlimit_addr_reg);
	vpq_memc_suspend_resume.MEMC_DBUS_WRAP_MC_UPLIMIT = IoReg_Read32(MEMC_DBUS_WRAP_Mc_uplimit_addr_reg);
#endif //CONFIG_HW_SUPPORT_MEMC
}

void vpq_memc_disp_resume_snashop(void){
#ifdef CONFIG_HW_SUPPORT_MEMC
	rtd_pr_memc_notice( "%s %d\n",__func__,__LINE__);

	ppoverlay_double_buffer_ctrl2_RBUS double_buffer_ctrl2_reg;
	int timeout = 0xff;

	//MEMC mux
	IoReg_Write32(MEMC_MUX_MEMC_MUX_FUNC_CTRL_reg, vpq_memc_suspend_resume.MEMC_MUX_MEMC_MUX_FUNC_CTRL_SUSPEND);
	//FBG
	double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 1;
	double_buffer_ctrl2_reg.uzudtg_dbuf_vsync_sel = 3;
	//enable double buffer
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl2_reg.regValue);

	vpq_memc_suspend_resume.PPOVERLAY_MEMC_MUX_CTRL_SUSPEND = vpq_memc_suspend_resume.PPOVERLAY_MEMC_MUX_CTRL_SUSPEND | 0x00001000;
	IoReg_Write32(PPOVERLAY_MEMC_MUX_CTRL_reg, vpq_memc_suspend_resume.PPOVERLAY_MEMC_MUX_CTRL_SUSPEND);

	double_buffer_ctrl2_reg.uzudtgreg_dbuf_set = 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl2_reg.regValue);
	//wait double buffer done
	double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	while(double_buffer_ctrl2_reg.uzudtgreg_dbuf_set && timeout--){
		double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
		mdelay(1);
	}
	//disable double buffer
	double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 0;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl2_reg.regValue);

	//clock
	IoReg_Write32(SYS_REG_SYS_DISPCLKSEL_reg, vpq_memc_suspend_resume.SYS_REG_SYS_DISPCLKSEL_SUSPEND);
	IoReg_Write32(MEMC_LATENCY01_ADDR, vpq_memc_suspend_resume.MEMC_LATENCY01);
	IoReg_Write32(MEMC_LATENCY02_ADDR, vpq_memc_suspend_resume.MEMC_LATENCY02);

	if(Scaler_MEMC_CLK_Check() == FALSE){
		return;
	}
	//ME memory boundary
	IoReg_Write32(MEMC_DBUS_WRAP_Me_downlimit_addr_reg, vpq_memc_suspend_resume.MEMC_DBUS_WRAP_ME_DOWNLIMIT);
	IoReg_Write32(MEMC_DBUS_WRAP_Me_uplimit_addr_reg, vpq_memc_suspend_resume.MEMC_DBUS_WRAP_ME_UPLIMIT);
	//MC memory boundary
	IoReg_Write32(MEMC_DBUS_WRAP_Mc_downlimit_addr_reg, vpq_memc_suspend_resume.MEMC_DBUS_WRAP_MC_DOWNLIMIT);
	IoReg_Write32(MEMC_DBUS_WRAP_Mc_uplimit_addr_reg, vpq_memc_suspend_resume.MEMC_DBUS_WRAP_MC_UPLIMIT);
	MEMC_snashop_resume_Done = 1;
	rtd_pr_memc_debug("[MEMC]Snashop Resume done !!!\n");
#endif //CONFIG_HW_SUPPORT_MEMC
}

void vpq_memc_disp_suspend_instanboot(void){
#ifdef CONFIG_HW_SUPPORT_MEMC
	rtd_pr_memc_notice( "%s %d\n",__func__,__LINE__);

#ifdef CONFIG_MEMC_BYPASS
                return;
#endif

	if(Scaler_MEMC_CLK_Check() == FALSE){
		return;
	}


	// disable interrupt
	UINT32 u32_interrupt_reg = 0;
	//reg_kmc_int_en
	u32_interrupt_reg = IoReg_Read32(KMC_TOP__KMC_TOP_18_ADDR);
	u32_interrupt_reg &= 0xf0ffffff;
	IoReg_Write32(KMC_TOP__KMC_TOP_18_ADDR, u32_interrupt_reg);

	//reg_post_int_en
	u32_interrupt_reg = IoReg_Read32(KPOST_TOP__KPOST_TOP_08_ADDR);
	u32_interrupt_reg &= 0xfffffe1f;
	IoReg_Write32(KPOST_TOP__KPOST_TOP_08_ADDR, u32_interrupt_reg);

	//kmc_top
	vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_00 = IoReg_Read32(KMC_TOP__KMC_TOP_00_ADDR);
	vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_04 = IoReg_Read32(KMC_TOP__KMC_TOP_04_ADDR);
	vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_08 = IoReg_Read32(KMC_TOP__KMC_TOP_08_ADDR);
	vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_0C = IoReg_Read32(KMC_TOP__KMC_TOP_0C_ADDR);
	vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_10 = IoReg_Read32(KMC_TOP__KMC_TOP_10_ADDR);
	vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_14 = IoReg_Read32(KMC_TOP__KMC_TOP_14_ADDR);
	vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_18 = IoReg_Read32(KMC_TOP__KMC_TOP_18_ADDR);
	vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_1C = IoReg_Read32(KMC_TOP__KMC_TOP_1C_ADDR);
	vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_20 = IoReg_Read32(KMC_TOP__KMC_TOP_20_ADDR);
	vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_24 = IoReg_Read32(KMC_TOP__KMC_TOP_24_ADDR);
	vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_28 = IoReg_Read32(KMC_TOP__KMC_TOP_28_ADDR);
	vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_2C = IoReg_Read32(KMC_TOP__KMC_TOP_2C_ADDR);
	vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_30 = IoReg_Read32(KMC_TOP__KMC_TOP_30_ADDR);
	vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_34 = IoReg_Read32(KMC_TOP__KMC_TOP_34_ADDR);
	vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_38 = IoReg_Read32(KMC_TOP__KMC_TOP_38_ADDR);
	vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_3C = IoReg_Read32(KMC_TOP__KMC_TOP_3C_ADDR);
	vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_40 = IoReg_Read32(KMC_TOP__KMC_TOP_40_ADDR);
	vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_44 = IoReg_Read32(KMC_TOP__KMC_TOP_44_ADDR);
	vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_48 = IoReg_Read32(KMC_TOP__KMC_TOP_48_ADDR);
	vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_4C = IoReg_Read32(KMC_TOP__KMC_TOP_4C_ADDR);
	vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_50 = IoReg_Read32(KMC_TOP__KMC_TOP_50_ADDR);
	vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_54 = IoReg_Read32(KMC_TOP__KMC_TOP_54_ADDR);
	vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_58 = IoReg_Read32(KMC_TOP__KMC_TOP_58_ADDR);
	vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_5C = IoReg_Read32(KMC_TOP__KMC_TOP_5C_ADDR);
	vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_60 = IoReg_Read32(KMC_TOP__KMC_TOP_60_ADDR);
	vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_64 = IoReg_Read32(KMC_TOP__KMC_TOP_64_ADDR);
	vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_68 = IoReg_Read32(KMC_TOP__KMC_TOP_68_ADDR);
	vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_6C = IoReg_Read32(KMC_TOP__KMC_TOP_6C_ADDR);
	//IPPRE
	vpq_memc_suspend_resume.IPPRE__IPPRE_00 = IoReg_Read32(IPPRE__IPPRE_00_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_04 = IoReg_Read32(IPPRE__IPPRE_04_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_08 = IoReg_Read32(IPPRE__IPPRE_08_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_0C = IoReg_Read32(IPPRE__IPPRE_0C_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_10 = IoReg_Read32(IPPRE__IPPRE_10_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_14 = IoReg_Read32(IPPRE__IPPRE_14_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_18 = IoReg_Read32(IPPRE__IPPRE_18_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_1C = IoReg_Read32(IPPRE__IPPRE_1C_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_20 = IoReg_Read32(IPPRE__IPPRE_20_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_24 = IoReg_Read32(IPPRE__IPPRE_24_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_28 = IoReg_Read32(IPPRE__IPPRE_28_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_2C = IoReg_Read32(IPPRE__IPPRE_2C_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_30 = IoReg_Read32(IPPRE__IPPRE_30_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_34 = IoReg_Read32(IPPRE__IPPRE_34_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_38 = IoReg_Read32(IPPRE__IPPRE_38_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_3C = IoReg_Read32(IPPRE__IPPRE_3C_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_40 = IoReg_Read32(IPPRE__IPPRE_40_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_44 = IoReg_Read32(IPPRE__IPPRE_44_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_48 = IoReg_Read32(IPPRE__IPPRE_48_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_4C = IoReg_Read32(IPPRE__IPPRE_4C_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_50 = IoReg_Read32(IPPRE__IPPRE_50_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_54 = IoReg_Read32(IPPRE__IPPRE_54_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_58 = IoReg_Read32(IPPRE__IPPRE_58_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_5C = IoReg_Read32(IPPRE__IPPRE_5C_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_60 = IoReg_Read32(IPPRE__IPPRE_60_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_64 = IoReg_Read32(IPPRE__IPPRE_64_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_68 = IoReg_Read32(IPPRE__IPPRE_68_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_6C = IoReg_Read32(IPPRE__IPPRE_6C_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_70 = IoReg_Read32(IPPRE__IPPRE_70_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_74 = IoReg_Read32(IPPRE__IPPRE_74_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_78 = IoReg_Read32(IPPRE__IPPRE_78_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_7C = IoReg_Read32(IPPRE__IPPRE_7C_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_80 = IoReg_Read32(IPPRE__IPPRE_80_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_84 = IoReg_Read32(IPPRE__IPPRE_84_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_88 = IoReg_Read32(IPPRE__IPPRE_88_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_8C = IoReg_Read32(IPPRE__IPPRE_8C_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_90 = IoReg_Read32(IPPRE__IPPRE_90_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_94 = IoReg_Read32(IPPRE__IPPRE_94_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_98 = IoReg_Read32(IPPRE__IPPRE_98_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_9C = IoReg_Read32(IPPRE__IPPRE_9C_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_A0 = IoReg_Read32(IPPRE__IPPRE_A0_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_A4 = IoReg_Read32(IPPRE__IPPRE_A4_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_A8 = IoReg_Read32(IPPRE__IPPRE_A8_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_AC = IoReg_Read32(IPPRE__IPPRE_AC_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_B0 = IoReg_Read32(IPPRE__IPPRE_B0_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_B4 = IoReg_Read32(IPPRE__IPPRE_B4_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_B8 = IoReg_Read32(IPPRE__IPPRE_B8_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_BC = IoReg_Read32(IPPRE__IPPRE_BC_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_C0 = IoReg_Read32(IPPRE__IPPRE_C0_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_C4 = IoReg_Read32(IPPRE__IPPRE_C4_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_C8 = IoReg_Read32(IPPRE__IPPRE_C8_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_CC = IoReg_Read32(IPPRE__IPPRE_CC_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_D0 = IoReg_Read32(IPPRE__IPPRE_D0_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_D4 = IoReg_Read32(IPPRE__IPPRE_D4_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_D8 = IoReg_Read32(IPPRE__IPPRE_D8_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_DC = IoReg_Read32(IPPRE__IPPRE_DC_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_E0 = IoReg_Read32(IPPRE__IPPRE_E0_ADDR);
	vpq_memc_suspend_resume.IPPRE__IPPRE_E4 = IoReg_Read32(IPPRE__IPPRE_E4_ADDR);
	//KMC_CPR_TOP0
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_00 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_00_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_04 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_04_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_08 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_08_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_0C = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_0C_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_10 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_10_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_14 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_14_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_18 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_18_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_1C = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_1C_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_20 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_20_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_24 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_24_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_28 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_28_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_2C = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_2C_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_30 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_30_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_34 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_34_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_38 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_38_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_3C = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_3C_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_40 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_40_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_44 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_44_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_48 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_48_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_4C = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_4C_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_50 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_50_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_54 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_54_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_58 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_58_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_5C = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_5C_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_60 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_60_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_64 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_64_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_68 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_68_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_6C = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_6C_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_70 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_70_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_74 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_74_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_78 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_78_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_7C = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_7C_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_80 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_80_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_84 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_84_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_88 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_88_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_8C = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_8C_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_90 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_90_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_94 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_94_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_98 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_98_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_9C = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_9C_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_A0 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_A0_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_A4 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_A4_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_A8 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_A8_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_AC = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_AC_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_B0 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_B0_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_B4 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_B4_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_B8 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_B8_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_BC = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_BC_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_C0 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_C0_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_C4 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_C4_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_C8 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_C8_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_CC = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_CC_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_D0 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_D0_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_D4 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_D4_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_D8 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_D8_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_DC = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_DC_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_E0 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_E0_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_E4 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_E4_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_E8 = IoReg_Read32(KMC_CPR_TOP0__KMC_CPR_TOP0_E8_ADDR);
	//KMC_CPR_TOP1
	vpq_memc_suspend_resume.KMC_CPR_TOP1__PATT_CTRL0 = IoReg_Read32(KMC_CPR_TOP1__PATT_CTRL0_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP1__PATT_LF_I_POSITION0 = IoReg_Read32(KMC_CPR_TOP1__PATT_LF_I_POSITION0_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP1__PATT_LF_I_POSITION1 = IoReg_Read32(KMC_CPR_TOP1__PATT_LF_I_POSITION1_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP1__PATT_HF_I_POSITION0 = IoReg_Read32(KMC_CPR_TOP1__PATT_HF_I_POSITION0_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP1__PATT_HF_I_POSITION1 = IoReg_Read32(KMC_CPR_TOP1__PATT_HF_I_POSITION1_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP1__CPR_PATT_LF_I_SETTING0 = IoReg_Read32(KMC_CPR_TOP1__CPR_PATT_LF_I_SETTING0_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP1__CPR_PATT_LF_I_SETTING1 = IoReg_Read32(KMC_CPR_TOP1__CPR_PATT_LF_I_SETTING1_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP1__CPR_PATT_HF_I_SETTING0 = IoReg_Read32(KMC_CPR_TOP1__CPR_PATT_HF_I_SETTING0_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP1__CPR_PATT_HF_I_SETTING1 = IoReg_Read32(KMC_CPR_TOP1__CPR_PATT_HF_I_SETTING1_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP1__PATT_LF_P_POSITION0 = IoReg_Read32(KMC_CPR_TOP1__PATT_LF_P_POSITION0_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP1__PATT_LF_P_POSITION1 = IoReg_Read32(KMC_CPR_TOP1__PATT_LF_P_POSITION1_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP1__PATT_HF_P_POSITION0 = IoReg_Read32(KMC_CPR_TOP1__PATT_HF_P_POSITION0_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP1__PATT_HF_P_POSITION1 = IoReg_Read32(KMC_CPR_TOP1__PATT_HF_P_POSITION1_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP1__CPR_PATT_LF_P_SETTING0 = IoReg_Read32(KMC_CPR_TOP1__CPR_PATT_LF_P_SETTING0_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP1__CPR_PATT_LF_P_SETTING1 = IoReg_Read32(KMC_CPR_TOP1__CPR_PATT_LF_P_SETTING1_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP1__CPR_PATT_HF_P_SETTING0 = IoReg_Read32(KMC_CPR_TOP1__CPR_PATT_HF_P_SETTING0_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP1__CPR_PATT_HF_P_SETTING1 = IoReg_Read32(KMC_CPR_TOP1__CPR_PATT_HF_P_SETTING1_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP1__PATT_LF_NR_POSITION0 = IoReg_Read32(KMC_CPR_TOP1__PATT_LF_NR_POSITION0_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP1__PATT_LF_NR_POSITION1 = IoReg_Read32(KMC_CPR_TOP1__PATT_LF_NR_POSITION1_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP1__PATT_HF_NR_POSITION = IoReg_Read32(KMC_CPR_TOP1__PATT_HF_NR_POSITION_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP1__PATT_HF_NR_POSITION1 = IoReg_Read32(KMC_CPR_TOP1__PATT_HF_NR_POSITION1_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP1__CPR_PATT_LF_NR_SETTING0 = IoReg_Read32(KMC_CPR_TOP1__CPR_PATT_LF_NR_SETTING0_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP1__CPR_PATT_LF_NR_SETTING1 = IoReg_Read32(KMC_CPR_TOP1__CPR_PATT_LF_NR_SETTING1_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP1__CPR_PATT_HF_NR_SETTING0 = IoReg_Read32(KMC_CPR_TOP1__CPR_PATT_HF_NR_SETTING0_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP1__CPR_PATT_HF_NR_SETTING1 = IoReg_Read32(KMC_CPR_TOP1__CPR_PATT_HF_NR_SETTING1_ADDR);
	vpq_memc_suspend_resume.KMC_CPR_TOP1__CPR_DEBUG_CFG0 = IoReg_Read32(KMC_CPR_TOP1__CPR_DEBUG_CFG0_ADDR);
	//KMC_DM_TOP
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_START_ADDRESS0 = IoReg_Read32(KMC_DM_TOP__KMC_00_START_ADDRESS0_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_START_ADDRESS1 = IoReg_Read32(KMC_DM_TOP__KMC_00_START_ADDRESS1_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_START_ADDRESS2 = IoReg_Read32(KMC_DM_TOP__KMC_00_START_ADDRESS2_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_START_ADDRESS3 = IoReg_Read32(KMC_DM_TOP__KMC_00_START_ADDRESS3_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_START_ADDRESS4 = IoReg_Read32(KMC_DM_TOP__KMC_00_START_ADDRESS4_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_START_ADDRESS5 = IoReg_Read32(KMC_DM_TOP__KMC_00_START_ADDRESS5_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_END_ADDRESS0 = IoReg_Read32(KMC_DM_TOP__KMC_00_END_ADDRESS0_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_END_ADDRESS1 = IoReg_Read32(KMC_DM_TOP__KMC_00_END_ADDRESS1_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_END_ADDRESS2 = IoReg_Read32(KMC_DM_TOP__KMC_00_END_ADDRESS2_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_END_ADDRESS3 = IoReg_Read32(KMC_DM_TOP__KMC_00_END_ADDRESS3_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_END_ADDRESS4 = IoReg_Read32(KMC_DM_TOP__KMC_00_END_ADDRESS4_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_END_ADDRESS5 = IoReg_Read32(KMC_DM_TOP__KMC_00_END_ADDRESS5_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_LR_OFFSET_ADDR = IoReg_Read32(KMC_DM_TOP__KMC_00_LR_OFFSET_ADDR_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_LINE_OFFSET_ADDR = IoReg_Read32(KMC_DM_TOP__KMC_00_LINE_OFFSET_ADDR_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_AGENT = IoReg_Read32(KMC_DM_TOP__KMC_00_AGENT_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_THRESHOLD = IoReg_Read32(KMC_DM_TOP__KMC_00_THRESHOLD_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_START_ADDRESS0 = IoReg_Read32(KMC_DM_TOP__KMC_01_START_ADDRESS0_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_START_ADDRESS1 = IoReg_Read32(KMC_DM_TOP__KMC_01_START_ADDRESS1_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_START_ADDRESS2 = IoReg_Read32(KMC_DM_TOP__KMC_01_START_ADDRESS2_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_START_ADDRESS3 = IoReg_Read32(KMC_DM_TOP__KMC_01_START_ADDRESS3_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_START_ADDRESS4 = IoReg_Read32(KMC_DM_TOP__KMC_01_START_ADDRESS4_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_START_ADDRESS5 = IoReg_Read32(KMC_DM_TOP__KMC_01_START_ADDRESS5_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_END_ADDRESS0 = IoReg_Read32(KMC_DM_TOP__KMC_01_END_ADDRESS0_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_END_ADDRESS1 = IoReg_Read32(KMC_DM_TOP__KMC_01_END_ADDRESS1_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_END_ADDRESS2 = IoReg_Read32(KMC_DM_TOP__KMC_01_END_ADDRESS2_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_END_ADDRESS3 = IoReg_Read32(KMC_DM_TOP__KMC_01_END_ADDRESS3_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_END_ADDRESS4 = IoReg_Read32(KMC_DM_TOP__KMC_01_END_ADDRESS4_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_END_ADDRESS5 = IoReg_Read32(KMC_DM_TOP__KMC_01_END_ADDRESS5_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_LR_OFFSET_ADDR = IoReg_Read32(KMC_DM_TOP__KMC_01_LR_OFFSET_ADDR_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_LINE_OFFSET_ADDR = IoReg_Read32(KMC_DM_TOP__KMC_01_LINE_OFFSET_ADDR_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_AGENT = IoReg_Read32(KMC_DM_TOP__KMC_01_AGENT_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_THRESHOLD = IoReg_Read32(KMC_DM_TOP__KMC_01_THRESHOLD_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_02_AGENT = IoReg_Read32(KMC_DM_TOP__KMC_02_AGENT_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_02_THRESHOLD = IoReg_Read32(KMC_DM_TOP__KMC_02_THRESHOLD_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_03_AGENT = IoReg_Read32(KMC_DM_TOP__KMC_03_AGENT_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_03_THRESHOLD = IoReg_Read32(KMC_DM_TOP__KMC_03_THRESHOLD_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_04_AGENT = IoReg_Read32(KMC_DM_TOP__KMC_04_AGENT_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_04_THRESHOLD = IoReg_Read32(KMC_DM_TOP__KMC_04_THRESHOLD_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_05_AGENT = IoReg_Read32(KMC_DM_TOP__KMC_05_AGENT_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_05_THRESHOLD = IoReg_Read32(KMC_DM_TOP__KMC_05_THRESHOLD_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_06_AGENT = IoReg_Read32(KMC_DM_TOP__KMC_06_AGENT_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_06_THRESHOLD = IoReg_Read32(KMC_DM_TOP__KMC_06_THRESHOLD_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_07_AGENT = IoReg_Read32(KMC_DM_TOP__KMC_07_AGENT_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_07_THRESHOLD = IoReg_Read32(KMC_DM_TOP__KMC_07_THRESHOLD_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_VTOTAL_AND_TIMER = IoReg_Read32(KMC_DM_TOP__KMC_VTOTAL_AND_TIMER_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_HOLD_TIME_G0 = IoReg_Read32(KMC_DM_TOP__KMC_HOLD_TIME_G0_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_HOLD_TIME_G1 = IoReg_Read32(KMC_DM_TOP__KMC_HOLD_TIME_G1_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_SOURCE_MODE = IoReg_Read32(KMC_DM_TOP__KMC_SOURCE_MODE_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_DEBUG_HEAD_G0 = IoReg_Read32(KMC_DM_TOP__KMC_DEBUG_HEAD_G0_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_DEBUG_HEAD_G1 = IoReg_Read32(KMC_DM_TOP__KMC_DEBUG_HEAD_G1_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_DEBUG_HEAD_G2 = IoReg_Read32(KMC_DM_TOP__KMC_DEBUG_HEAD_G2_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_DEBUG_HEAD_G3 = IoReg_Read32(KMC_DM_TOP__KMC_DEBUG_HEAD_G3_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_START_ADDRESS6 = IoReg_Read32(KMC_DM_TOP__KMC_00_START_ADDRESS6_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_END_ADDRESS6 = IoReg_Read32(KMC_DM_TOP__KMC_00_END_ADDRESS6_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_START_ADDRESS7 = IoReg_Read32(KMC_DM_TOP__KMC_00_START_ADDRESS7_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_END_ADDRESS7 = IoReg_Read32(KMC_DM_TOP__KMC_00_END_ADDRESS7_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_START_ADDRESS6 = IoReg_Read32(KMC_DM_TOP__KMC_01_START_ADDRESS6_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_END_ADDRESS6 = IoReg_Read32(KMC_DM_TOP__KMC_01_END_ADDRESS6_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_START_ADDRESS7 = IoReg_Read32(KMC_DM_TOP__KMC_01_START_ADDRESS7_ADDR);
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_END_ADDRESS7 = IoReg_Read32(KMC_DM_TOP__KMC_01_END_ADDRESS7_ADDR);
	//IPPRE1
	vpq_memc_suspend_resume.IPPRE1__IPPRE1_00 = IoReg_Read32(IPPRE1__IPPRE1_00_ADDR);
	vpq_memc_suspend_resume.IPPRE1__IPPRE1_04 = IoReg_Read32(IPPRE1__IPPRE1_04_ADDR);
	//MC
	vpq_memc_suspend_resume.MC__MC_0C = IoReg_Read32(MC__MC_0C_ADDR);
	vpq_memc_suspend_resume.MC__MC_10 = IoReg_Read32(MC__MC_10_ADDR);
	vpq_memc_suspend_resume.MC__MC_14 = IoReg_Read32(MC__MC_14_ADDR);
	vpq_memc_suspend_resume.MC__MC_18 = IoReg_Read32(MC__MC_18_ADDR);
	vpq_memc_suspend_resume.MC__MC_1C = IoReg_Read32(MC__MC_1C_ADDR);
	vpq_memc_suspend_resume.MC__MC_20 = IoReg_Read32(MC__MC_20_ADDR);
	vpq_memc_suspend_resume.MC__MC_24 = IoReg_Read32(MC__MC_24_ADDR);
	vpq_memc_suspend_resume.MC__MC_28 = IoReg_Read32(MC__MC_28_ADDR);
	vpq_memc_suspend_resume.MC__MC_2C = IoReg_Read32(MC__MC_2C_ADDR);
	vpq_memc_suspend_resume.MC__MC_30 = IoReg_Read32(MC__MC_30_ADDR);
	vpq_memc_suspend_resume.MC__MC_34 = IoReg_Read32(MC__MC_34_ADDR);
	vpq_memc_suspend_resume.MC__MC_38 = IoReg_Read32(MC__MC_38_ADDR);
	vpq_memc_suspend_resume.MC__MC_3C = IoReg_Read32(MC__MC_3C_ADDR);
	vpq_memc_suspend_resume.MC__MC_40 = IoReg_Read32(MC__MC_40_ADDR);
	vpq_memc_suspend_resume.MC__MC_44 = IoReg_Read32(MC__MC_44_ADDR);
	vpq_memc_suspend_resume.MC__MC_48 = IoReg_Read32(MC__MC_48_ADDR);
	vpq_memc_suspend_resume.MC__MC_4C = IoReg_Read32(MC__MC_4C_ADDR);
	vpq_memc_suspend_resume.MC__MC_50 = IoReg_Read32(MC__MC_50_ADDR);
	vpq_memc_suspend_resume.MC__MC_54 = IoReg_Read32(MC__MC_54_ADDR);
	vpq_memc_suspend_resume.MC__MC_58 = IoReg_Read32(MC__MC_58_ADDR);
	vpq_memc_suspend_resume.MC__MC_5C = IoReg_Read32(MC__MC_5C_ADDR);
	vpq_memc_suspend_resume.MC__MC_60 = IoReg_Read32(MC__MC_60_ADDR);
	vpq_memc_suspend_resume.MC__MC_64 = IoReg_Read32(MC__MC_64_ADDR);
	vpq_memc_suspend_resume.MC__MC_68 = IoReg_Read32(MC__MC_68_ADDR);
	vpq_memc_suspend_resume.MC__MC_6C = IoReg_Read32(MC__MC_6C_ADDR);
	vpq_memc_suspend_resume.MC__MC_70 = IoReg_Read32(MC__MC_70_ADDR);
	vpq_memc_suspend_resume.MC__MC_74 = IoReg_Read32(MC__MC_74_ADDR);
	vpq_memc_suspend_resume.MC__MC_78 = IoReg_Read32(MC__MC_78_ADDR);
	vpq_memc_suspend_resume.MC__MC_7C = IoReg_Read32(MC__MC_7C_ADDR);
	vpq_memc_suspend_resume.MC__MC_80 = IoReg_Read32(MC__MC_80_ADDR);
	vpq_memc_suspend_resume.MC__MC_84 = IoReg_Read32(MC__MC_84_ADDR);
	vpq_memc_suspend_resume.MC__MC_88 = IoReg_Read32(MC__MC_88_ADDR);
	vpq_memc_suspend_resume.MC__MC_8C = IoReg_Read32(MC__MC_8C_ADDR);
	vpq_memc_suspend_resume.MC__MC_90 = IoReg_Read32(MC__MC_90_ADDR);
	vpq_memc_suspend_resume.MC__MC_94 = IoReg_Read32(MC__MC_94_ADDR);
	vpq_memc_suspend_resume.MC__MC_98 = IoReg_Read32(MC__MC_98_ADDR);
	vpq_memc_suspend_resume.MC__MC_9C = IoReg_Read32(MC__MC_9C_ADDR);
	vpq_memc_suspend_resume.MC__MC_A0 = IoReg_Read32(MC__MC_A0_ADDR);
	vpq_memc_suspend_resume.MC__MC_A4 = IoReg_Read32(MC__MC_A4_ADDR);
	vpq_memc_suspend_resume.MC__MC_A8 = IoReg_Read32(MC__MC_A8_ADDR);
	vpq_memc_suspend_resume.MC__MC_AC = IoReg_Read32(MC__MC_AC_ADDR);
	vpq_memc_suspend_resume.MC__MC_B0 = IoReg_Read32(MC__MC_B0_ADDR);
	vpq_memc_suspend_resume.MC__MC_B4 = IoReg_Read32(MC__MC_B4_ADDR);
	vpq_memc_suspend_resume.MC__MC_B8 = IoReg_Read32(MC__MC_B8_ADDR);
	vpq_memc_suspend_resume.MC__MC_BC = IoReg_Read32(MC__MC_BC_ADDR);
	vpq_memc_suspend_resume.MC__MC_C0 = IoReg_Read32(MC__MC_C0_ADDR);
	vpq_memc_suspend_resume.MC__MC_C4 = IoReg_Read32(MC__MC_C4_ADDR);
	vpq_memc_suspend_resume.MC__MC_C8 = IoReg_Read32(MC__MC_C8_ADDR);
	vpq_memc_suspend_resume.MC__MC_CC = IoReg_Read32(MC__MC_CC_ADDR);
	vpq_memc_suspend_resume.MC__MC_D0 = IoReg_Read32(MC__MC_D0_ADDR);
	vpq_memc_suspend_resume.MC__MC_D4 = IoReg_Read32(MC__MC_D4_ADDR);
	vpq_memc_suspend_resume.MC__MC_D8 = IoReg_Read32(MC__MC_D8_ADDR);
	vpq_memc_suspend_resume.MC__MC_DC = IoReg_Read32(MC__MC_DC_ADDR);
	vpq_memc_suspend_resume.MC__MC_E0 = IoReg_Read32(MC__MC_E0_ADDR);
	//MC2
	vpq_memc_suspend_resume.MC2__MC2_00 = IoReg_Read32(MC2__MC2_00_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_04 = IoReg_Read32(MC2__MC2_04_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_08 = IoReg_Read32(MC2__MC2_08_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_0C = IoReg_Read32(MC2__MC2_0C_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_10 = IoReg_Read32(MC2__MC2_10_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_14 = IoReg_Read32(MC2__MC2_14_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_18 = IoReg_Read32(MC2__MC2_18_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_1C = IoReg_Read32(MC2__MC2_1C_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_20 = IoReg_Read32(MC2__MC2_20_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_24 = IoReg_Read32(MC2__MC2_24_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_28 = IoReg_Read32(MC2__MC2_28_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_2C = IoReg_Read32(MC2__MC2_2C_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_30 = IoReg_Read32(MC2__MC2_30_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_50 = IoReg_Read32(MC2__MC2_50_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_54 = IoReg_Read32(MC2__MC2_54_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_58 = IoReg_Read32(MC2__MC2_58_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_5C = IoReg_Read32(MC2__MC2_5C_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_60 = IoReg_Read32(MC2__MC2_60_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_64 = IoReg_Read32(MC2__MC2_64_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_68 = IoReg_Read32(MC2__MC2_68_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_6C = IoReg_Read32(MC2__MC2_6C_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_70 = IoReg_Read32(MC2__MC2_70_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_74 = IoReg_Read32(MC2__MC2_74_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_7C = IoReg_Read32(MC2__MC2_7C_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_80 = IoReg_Read32(MC2__MC2_80_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_84 = IoReg_Read32(MC2__MC2_84_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_88 = IoReg_Read32(MC2__MC2_88_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_8C = IoReg_Read32(MC2__MC2_8C_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_90 = IoReg_Read32(MC2__MC2_90_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_94 = IoReg_Read32(MC2__MC2_94_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_98 = IoReg_Read32(MC2__MC2_98_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_9C = IoReg_Read32(MC2__MC2_9C_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_A0 = IoReg_Read32(MC2__MC2_A0_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_A4 = IoReg_Read32(MC2__MC2_A4_ADDR);
	vpq_memc_suspend_resume.MC2__MC2_A8 = IoReg_Read32(MC2__MC2_A8_ADDR);
	//LBMC
	vpq_memc_suspend_resume.LBMC__LBMC_00 = IoReg_Read32(LBMC__LBMC_00_ADDR);
	vpq_memc_suspend_resume.LBMC__LBMC_04 = IoReg_Read32(LBMC__LBMC_04_ADDR);
	vpq_memc_suspend_resume.LBMC__LBMC_08 = IoReg_Read32(LBMC__LBMC_08_ADDR);
	vpq_memc_suspend_resume.LBMC__LBMC_0C = IoReg_Read32(LBMC__LBMC_0C_ADDR);
	vpq_memc_suspend_resume.LBMC__LBMC_10 = IoReg_Read32(LBMC__LBMC_10_ADDR);
	vpq_memc_suspend_resume.LBMC__LBMC_14 = IoReg_Read32(LBMC__LBMC_14_ADDR);
	vpq_memc_suspend_resume.LBMC__LBMC_18 = IoReg_Read32(LBMC__LBMC_18_ADDR);
	vpq_memc_suspend_resume.LBMC__LBMC_1C = IoReg_Read32(LBMC__LBMC_1C_ADDR);
	vpq_memc_suspend_resume.LBMC__LBMC_20 = IoReg_Read32(LBMC__LBMC_20_ADDR);
	vpq_memc_suspend_resume.LBMC__LBMC_24 = IoReg_Read32(LBMC__LBMC_24_ADDR);
	vpq_memc_suspend_resume.LBMC__LBMC_28 = IoReg_Read32(LBMC__LBMC_28_ADDR);
	vpq_memc_suspend_resume.LBMC__LBMC_2C = IoReg_Read32(LBMC__LBMC_2C_ADDR);
	vpq_memc_suspend_resume.LBMC__LBMC_30 = IoReg_Read32(LBMC__LBMC_30_ADDR);
	vpq_memc_suspend_resume.LBMC__LBMC_34 = IoReg_Read32(LBMC__LBMC_34_ADDR);
	vpq_memc_suspend_resume.LBMC__LBMC_38 = IoReg_Read32(LBMC__LBMC_38_ADDR);
	vpq_memc_suspend_resume.LBMC__LBMC_3C = IoReg_Read32(LBMC__LBMC_3C_ADDR);
	vpq_memc_suspend_resume.LBMC__LBMC_40 = IoReg_Read32(LBMC__LBMC_40_ADDR);
	vpq_memc_suspend_resume.LBMC__LBMC_44 = IoReg_Read32(LBMC__LBMC_44_ADDR);
	vpq_memc_suspend_resume.LBMC__LBMC_48 = IoReg_Read32(LBMC__LBMC_48_ADDR);
	vpq_memc_suspend_resume.LBMC__LBMC_4C = IoReg_Read32(LBMC__LBMC_4C_ADDR);
	vpq_memc_suspend_resume.LBMC__LBMC_50 = IoReg_Read32(LBMC__LBMC_50_ADDR);
	vpq_memc_suspend_resume.LBMC__LBMC_54 = IoReg_Read32(LBMC__LBMC_54_ADDR);
	vpq_memc_suspend_resume.LBMC__LBMC_58 = IoReg_Read32(LBMC__LBMC_58_ADDR);
	vpq_memc_suspend_resume.LBMC__LBMC_5C = IoReg_Read32(LBMC__LBMC_5C_ADDR);
	vpq_memc_suspend_resume.LBMC__LBMC_60 = IoReg_Read32(LBMC__LBMC_60_ADDR);
	vpq_memc_suspend_resume.LBMC__LBMC_80 = IoReg_Read32(LBMC__LBMC_80_ADDR);
	vpq_memc_suspend_resume.LBMC__LBMC_84 = IoReg_Read32(LBMC__LBMC_84_ADDR);
	vpq_memc_suspend_resume.LBMC__LBMC_88 = IoReg_Read32(LBMC__LBMC_88_ADDR);
	//BBD
	vpq_memc_suspend_resume.BBD__BBD_00 = IoReg_Read32(BBD__BBD_00_ADDR);
	vpq_memc_suspend_resume.BBD__BBD_04 = IoReg_Read32(BBD__BBD_04_ADDR);
	vpq_memc_suspend_resume.BBD__BBD_08 = IoReg_Read32(BBD__BBD_08_ADDR);
	vpq_memc_suspend_resume.BBD__BBD_0C = IoReg_Read32(BBD__BBD_0C_ADDR);
	vpq_memc_suspend_resume.BBD__BBD_10 = IoReg_Read32(BBD__BBD_10_ADDR);
	//KMC_METER_TOP
	vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_00 = IoReg_Read32(KMC_METER_TOP__KMC_METER_TOP_00_ADDR);
	vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_04 = IoReg_Read32(KMC_METER_TOP__KMC_METER_TOP_04_ADDR);
	vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_08 = IoReg_Read32(KMC_METER_TOP__KMC_METER_TOP_08_ADDR);
	vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_0C = IoReg_Read32(KMC_METER_TOP__KMC_METER_TOP_0C_ADDR);
	vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_10 = IoReg_Read32(KMC_METER_TOP__KMC_METER_TOP_10_ADDR);
	vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_14 = IoReg_Read32(KMC_METER_TOP__KMC_METER_TOP_14_ADDR);
	vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_18 = IoReg_Read32(KMC_METER_TOP__KMC_METER_TOP_18_ADDR);
	vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_1C = IoReg_Read32(KMC_METER_TOP__KMC_METER_TOP_1C_ADDR);
	vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_20 = IoReg_Read32(KMC_METER_TOP__KMC_METER_TOP_20_ADDR);
	vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_24 = IoReg_Read32(KMC_METER_TOP__KMC_METER_TOP_24_ADDR);
	vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_28 = IoReg_Read32(KMC_METER_TOP__KMC_METER_TOP_28_ADDR);
	vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_2C = IoReg_Read32(KMC_METER_TOP__KMC_METER_TOP_2C_ADDR);
	vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_30 = IoReg_Read32(KMC_METER_TOP__KMC_METER_TOP_30_ADDR);
	vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_34 = IoReg_Read32(KMC_METER_TOP__KMC_METER_TOP_34_ADDR);
	vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_38 = IoReg_Read32(KMC_METER_TOP__KMC_METER_TOP_38_ADDR);
	vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_3C = IoReg_Read32(KMC_METER_TOP__KMC_METER_TOP_3C_ADDR);
	vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_40 = IoReg_Read32(KMC_METER_TOP__KMC_METER_TOP_40_ADDR);
	vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_44 = IoReg_Read32(KMC_METER_TOP__KMC_METER_TOP_44_ADDR);
	vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_48 = IoReg_Read32(KMC_METER_TOP__KMC_METER_TOP_48_ADDR);
	vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_4C = IoReg_Read32(KMC_METER_TOP__KMC_METER_TOP_4C_ADDR);
	vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_50 = IoReg_Read32(KMC_METER_TOP__KMC_METER_TOP_50_ADDR);
	vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_54 = IoReg_Read32(KMC_METER_TOP__KMC_METER_TOP_54_ADDR);
	vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_58 = IoReg_Read32(KMC_METER_TOP__KMC_METER_TOP_58_ADDR);
	vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_5C = IoReg_Read32(KMC_METER_TOP__KMC_METER_TOP_5C_ADDR);
	vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_60 = IoReg_Read32(KMC_METER_TOP__KMC_METER_TOP_60_ADDR);
	vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_64 = IoReg_Read32(KMC_METER_TOP__KMC_METER_TOP_64_ADDR);
	vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_68 = IoReg_Read32(KMC_METER_TOP__KMC_METER_TOP_68_ADDR);
	vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_6C = IoReg_Read32(KMC_METER_TOP__KMC_METER_TOP_6C_ADDR);
	//KMC_DM_METER_TOP
	vpq_memc_suspend_resume.KMC_DM_METER_TOP__KMC_METER_REG0 = IoReg_Read32(KMC_DM_METER_TOP__KMC_METER_REG0_ADDR);
	vpq_memc_suspend_resume.KMC_DM_METER_TOP__KMC_METER_NUM = IoReg_Read32(KMC_DM_METER_TOP__KMC_METER_NUM_ADDR);
	vpq_memc_suspend_resume.KMC_DM_METER_TOP__KMC_METER_H_NUM0 = IoReg_Read32(KMC_DM_METER_TOP__KMC_METER_H_NUM0_ADDR);
	vpq_memc_suspend_resume.KMC_DM_METER_TOP__KMC_METER_H_NUM1 = IoReg_Read32(KMC_DM_METER_TOP__KMC_METER_H_NUM1_ADDR);
	vpq_memc_suspend_resume.KMC_DM_METER_TOP__METER_TH0 = IoReg_Read32(KMC_DM_METER_TOP__METER_TH0_ADDR);
	vpq_memc_suspend_resume.KMC_DM_METER_TOP__METER_TH1 = IoReg_Read32(KMC_DM_METER_TOP__METER_TH1_ADDR);
	vpq_memc_suspend_resume.KMC_DM_METER_TOP__METER_TH3 = IoReg_Read32(KMC_DM_METER_TOP__METER_TH3_ADDR);
	vpq_memc_suspend_resume.KMC_DM_METER_TOP__METER_TIMER0 = IoReg_Read32(KMC_DM_METER_TOP__METER_TIMER0_ADDR);
	//KME_TOP
	vpq_memc_suspend_resume.KME_TOP__KME_TOP_04 = IoReg_Read32(KME_TOP__KME_TOP_04_ADDR);
	vpq_memc_suspend_resume.KME_TOP__KME_TOP_08 = IoReg_Read32(KME_TOP__KME_TOP_08_ADDR);
	vpq_memc_suspend_resume.KME_TOP__KME_TOP_10 = IoReg_Read32(KME_TOP__KME_TOP_10_ADDR);
	vpq_memc_suspend_resume.KME_TOP__KME_TOP_14 = IoReg_Read32(KME_TOP__KME_TOP_14_ADDR);
	vpq_memc_suspend_resume.KME_TOP__KME_TOP_18 = IoReg_Read32(KME_TOP__KME_TOP_18_ADDR);
	vpq_memc_suspend_resume.KME_TOP__KME_TOP_1C = IoReg_Read32(KME_TOP__KME_TOP_1C_ADDR);
	vpq_memc_suspend_resume.KME_TOP__KME_TOP_20 = IoReg_Read32(KME_TOP__KME_TOP_20_ADDR);
	vpq_memc_suspend_resume.KME_TOP__KME_TOP_24 = IoReg_Read32(KME_TOP__KME_TOP_24_ADDR);
	vpq_memc_suspend_resume.KME_TOP__KME_TOP_28 = IoReg_Read32(KME_TOP__KME_TOP_28_ADDR);
	vpq_memc_suspend_resume.KME_TOP__KME_TOP_2C = IoReg_Read32(KME_TOP__KME_TOP_2C_ADDR);
	vpq_memc_suspend_resume.KME_TOP__KME_TOP_30 = IoReg_Read32(KME_TOP__KME_TOP_30_ADDR);
	vpq_memc_suspend_resume.KME_TOP__KME_TOP_34 = IoReg_Read32(KME_TOP__KME_TOP_34_ADDR);
	vpq_memc_suspend_resume.KME_TOP__KME_TOP_38 = IoReg_Read32(KME_TOP__KME_TOP_38_ADDR);
	vpq_memc_suspend_resume.KME_TOP__KME_TOP_3C = IoReg_Read32(KME_TOP__KME_TOP_3C_ADDR);
	//KME_VBUF_TOP
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_00 = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_00_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_04 = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_04_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_08 = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_08_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_0C = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_0C_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_10 = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_10_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_14 = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_14_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_18 = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_18_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_1C = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_1C_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_20 = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_20_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_24 = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_24_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_28 = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_28_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_2C = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_2C_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_30 = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_30_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_34 = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_34_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_38 = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_38_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_3C = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_3C_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_40 = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_40_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_44 = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_44_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_48 = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_48_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_4C = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_4C_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_50 = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_50_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_54 = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_54_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_58 = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_58_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_5C = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_5C_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_60 = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_60_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_64 = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_64_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_68 = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_68_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_6C = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_6C_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_70 = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_70_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_74 = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_74_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_78 = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_78_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_7C = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_7C_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_80 = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_80_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_84 = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_84_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_88 = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_88_ADDR);
	vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_8C = IoReg_Read32(KME_VBUF_TOP__KME_VBUF_TOP_8C_ADDR);
	//KME_ME1_TOP2
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_00 = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_00_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_04 = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_04_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_08 = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_08_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_0C = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_0C_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_10 = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_10_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_14 = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_14_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_18 = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_18_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_1C = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_1C_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_20 = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_20_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_24 = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_24_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_28 = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_28_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_2C = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_2C_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_30 = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_30_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_34 = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_34_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_38 = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_38_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_3C = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_3C_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_40 = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_40_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_44 = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_44_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_48 = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_48_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_4C = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_4C_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_50 = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_50_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_54 = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_54_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_58 = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_58_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_5C = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_5C_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_60 = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_60_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_64 = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_64_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_80 = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_80_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_84 = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_84_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_88 = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_88_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_8C = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_8C_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_90 = IoReg_Read32(KME_ME1_TOP2__KME_ME1_TOP2_90_ADDR);
	//KME_ME1_TOP3
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_00 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_00_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_04 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_04_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_08 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_08_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_0C = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_0C_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_10 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_10_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_14 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_14_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_18 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_18_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_1C = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_1C_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_20 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_20_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_24 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_24_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_28 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_28_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_2C = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_2C_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_30 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_30_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_34 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_34_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_38 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_38_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_3C = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_3C_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_40 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_40_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_44 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_44_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_48 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_48_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_4C = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_4C_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_50 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_50_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_54 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_54_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_58 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_58_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_5C = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_5C_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_60 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_60_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_64 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_64_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_68 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_68_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_6C = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_6C_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_70 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_70_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_74 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_74_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_78 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_78_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_7C = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_7C_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_80 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_80_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_84 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_84_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_88 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_88_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_8C = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_8C_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_90 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_90_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_94 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_94_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_98 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_98_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_9C = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_9C_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_A0 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_A0_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_A4 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_A4_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_A8 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_A8_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_AC = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_AC_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_B0 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_B0_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_B4 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_B4_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_B8 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_B8_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_BC = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_BC_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_C0 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_C0_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_C4 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_C4_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_C8 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_C8_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_CC = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_CC_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_D0 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_D0_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_D4 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_D4_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_D8 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_D8_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_DC = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_DC_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_E0 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_E0_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_E4 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_E4_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_E8 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_E8_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_EC = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_EC_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_F0 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_F0_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_F4 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_F4_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_F8 = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_F8_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_FC = IoReg_Read32(KME_ME1_TOP3__KME_ME1_TOP3_FC_ADDR);
	//KME_ME1_TOP4
	//KME_ME1_TOP5
	//KME_LBME_TOP
	vpq_memc_suspend_resume.KME_LBME_TOP__KME_LBME_TOP_00 = IoReg_Read32(KME_LBME_TOP__KME_LBME_TOP_00_ADDR);
	vpq_memc_suspend_resume.KME_LBME_TOP__KME_LBME_TOP_04 = IoReg_Read32(KME_LBME_TOP__KME_LBME_TOP_04_ADDR);
	vpq_memc_suspend_resume.KME_LBME_TOP__KME_LBME_TOP_08 = IoReg_Read32(KME_LBME_TOP__KME_LBME_TOP_08_ADDR);
	vpq_memc_suspend_resume.KME_LBME_TOP__KME_LBME_TOP_0C = IoReg_Read32(KME_LBME_TOP__KME_LBME_TOP_0C_ADDR);
	vpq_memc_suspend_resume.KME_LBME_TOP__KME_LBME_TOP_10 = IoReg_Read32(KME_LBME_TOP__KME_LBME_TOP_10_ADDR);
	vpq_memc_suspend_resume.KME_LBME_TOP__KME_LBME_TOP_14 = IoReg_Read32(KME_LBME_TOP__KME_LBME_TOP_14_ADDR);
	vpq_memc_suspend_resume.KME_LBME_TOP__KME_LBME_TOP_18 = IoReg_Read32(KME_LBME_TOP__KME_LBME_TOP_18_ADDR);
	vpq_memc_suspend_resume.KME_LBME_TOP__KME_LBME_TOP_1C = IoReg_Read32(KME_LBME_TOP__KME_LBME_TOP_1C_ADDR);
	vpq_memc_suspend_resume.KME_LBME_TOP__KME_LBME_TOP_20 = IoReg_Read32(KME_LBME_TOP__KME_LBME_TOP_20_ADDR);
	vpq_memc_suspend_resume.KME_LBME_TOP__KME_LBME_TOP_24 = IoReg_Read32(KME_LBME_TOP__KME_LBME_TOP_24_ADDR);
	//KME_ME2_CALC1
	vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_00 = IoReg_Read32(KME_ME2_CALC1__KME_ME2_CALC1_00_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_04 = IoReg_Read32(KME_ME2_CALC1__KME_ME2_CALC1_04_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_08 = IoReg_Read32(KME_ME2_CALC1__KME_ME2_CALC1_08_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_0C = IoReg_Read32(KME_ME2_CALC1__KME_ME2_CALC1_0C_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_10 = IoReg_Read32(KME_ME2_CALC1__KME_ME2_CALC1_10_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_14 = IoReg_Read32(KME_ME2_CALC1__KME_ME2_CALC1_14_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_18 = IoReg_Read32(KME_ME2_CALC1__KME_ME2_CALC1_18_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_1C = IoReg_Read32(KME_ME2_CALC1__KME_ME2_CALC1_1C_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_20 = IoReg_Read32(KME_ME2_CALC1__KME_ME2_CALC1_20_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_24 = IoReg_Read32(KME_ME2_CALC1__KME_ME2_CALC1_24_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_28 = IoReg_Read32(KME_ME2_CALC1__KME_ME2_CALC1_28_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_2C = IoReg_Read32(KME_ME2_CALC1__KME_ME2_CALC1_2C_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_30 = IoReg_Read32(KME_ME2_CALC1__KME_ME2_CALC1_30_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_34 = IoReg_Read32(KME_ME2_CALC1__KME_ME2_CALC1_34_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_38 = IoReg_Read32(KME_ME2_CALC1__KME_ME2_CALC1_38_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_3C = IoReg_Read32(KME_ME2_CALC1__KME_ME2_CALC1_3C_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_40 = IoReg_Read32(KME_ME2_CALC1__KME_ME2_CALC1_40_ADDR);
	//KME_LOGO2
	vpq_memc_suspend_resume.KME_LOGO2__KME_LOGO2_00 = IoReg_Read32(KME_LOGO2__KME_LOGO2_00_ADDR);
	vpq_memc_suspend_resume.KME_LOGO2__KME_LOGO2_04 = IoReg_Read32(KME_LOGO2__KME_LOGO2_04_ADDR);
	vpq_memc_suspend_resume.KME_LOGO2__KME_LOGO2_08 = IoReg_Read32(KME_LOGO2__KME_LOGO2_08_ADDR);
	vpq_memc_suspend_resume.KME_LOGO2__KME_LOGO2_0C = IoReg_Read32(KME_LOGO2__KME_LOGO2_0C_ADDR);
	vpq_memc_suspend_resume.KME_LOGO2__KME_LOGO2_10 = IoReg_Read32(KME_LOGO2__KME_LOGO2_10_ADDR);
	vpq_memc_suspend_resume.KME_LOGO2__KME_LOGO2_14 = IoReg_Read32(KME_LOGO2__KME_LOGO2_14_ADDR);
	vpq_memc_suspend_resume.KME_LOGO2__KME_LOGO2_18 = IoReg_Read32(KME_LOGO2__KME_LOGO2_18_ADDR);
	vpq_memc_suspend_resume.KME_LOGO2__KME_LOGO2_1C = IoReg_Read32(KME_LOGO2__KME_LOGO2_1C_ADDR);
	vpq_memc_suspend_resume.KME_LOGO2__KME_LOGO2_20 = IoReg_Read32(KME_LOGO2__KME_LOGO2_20_ADDR);
	vpq_memc_suspend_resume.KME_LOGO2__KME_LOGO2_24 = IoReg_Read32(KME_LOGO2__KME_LOGO2_24_ADDR);
	vpq_memc_suspend_resume.KME_LOGO2__KME_LOGO2_28 = IoReg_Read32(KME_LOGO2__KME_LOGO2_28_ADDR);
	vpq_memc_suspend_resume.KME_LOGO2__KME_LOGO2_2C = IoReg_Read32(KME_LOGO2__KME_LOGO2_2C_ADDR);
	vpq_memc_suspend_resume.KME_LOGO2__KME_LOGO2_30 = IoReg_Read32(KME_LOGO2__KME_LOGO2_30_ADDR);
	vpq_memc_suspend_resume.KME_LOGO2__KME_LOGO2_34 = IoReg_Read32(KME_LOGO2__KME_LOGO2_34_ADDR);
	//KME_IPME
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_00 = IoReg_Read32(KME_IPME__KME_IPME_00_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_04 = IoReg_Read32(KME_IPME__KME_IPME_04_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_08 = IoReg_Read32(KME_IPME__KME_IPME_08_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_0C = IoReg_Read32(KME_IPME__KME_IPME_0C_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_10 = IoReg_Read32(KME_IPME__KME_IPME_10_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_14 = IoReg_Read32(KME_IPME__KME_IPME_14_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_18 = IoReg_Read32(KME_IPME__KME_IPME_18_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_1C = IoReg_Read32(KME_IPME__KME_IPME_1C_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_20 = IoReg_Read32(KME_IPME__KME_IPME_20_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_24 = IoReg_Read32(KME_IPME__KME_IPME_24_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_28 = IoReg_Read32(KME_IPME__KME_IPME_28_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_2C = IoReg_Read32(KME_IPME__KME_IPME_2C_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_30 = IoReg_Read32(KME_IPME__KME_IPME_30_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_34 = IoReg_Read32(KME_IPME__KME_IPME_34_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_38 = IoReg_Read32(KME_IPME__KME_IPME_38_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_3C = IoReg_Read32(KME_IPME__KME_IPME_3C_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_40 = IoReg_Read32(KME_IPME__KME_IPME_40_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_44 = IoReg_Read32(KME_IPME__KME_IPME_44_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_48 = IoReg_Read32(KME_IPME__KME_IPME_48_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_4C = IoReg_Read32(KME_IPME__KME_IPME_4C_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_50 = IoReg_Read32(KME_IPME__KME_IPME_50_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_54 = IoReg_Read32(KME_IPME__KME_IPME_54_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_58 = IoReg_Read32(KME_IPME__KME_IPME_58_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_5C = IoReg_Read32(KME_IPME__KME_IPME_5C_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_60 = IoReg_Read32(KME_IPME__KME_IPME_60_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_64 = IoReg_Read32(KME_IPME__KME_IPME_64_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_68 = IoReg_Read32(KME_IPME__KME_IPME_68_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_6C = IoReg_Read32(KME_IPME__KME_IPME_6C_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_70 = IoReg_Read32(KME_IPME__KME_IPME_70_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_74 = IoReg_Read32(KME_IPME__KME_IPME_74_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_78 = IoReg_Read32(KME_IPME__KME_IPME_78_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_7C = IoReg_Read32(KME_IPME__KME_IPME_7C_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_80 = IoReg_Read32(KME_IPME__KME_IPME_80_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_84 = IoReg_Read32(KME_IPME__KME_IPME_84_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_88 = IoReg_Read32(KME_IPME__KME_IPME_88_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_8C = IoReg_Read32(KME_IPME__KME_IPME_8C_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_90 = IoReg_Read32(KME_IPME__KME_IPME_90_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_94 = IoReg_Read32(KME_IPME__KME_IPME_94_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_98 = IoReg_Read32(KME_IPME__KME_IPME_98_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_9C = IoReg_Read32(KME_IPME__KME_IPME_9C_ADDR);
	vpq_memc_suspend_resume.KME_IPME__KME_IPME_A0 = IoReg_Read32(KME_IPME__KME_IPME_A0_ADDR);
	//KME_IPME1
	vpq_memc_suspend_resume.KME_IPME1__KME_IPME1_00 = IoReg_Read32(KME_IPME1__KME_IPME1_00_ADDR);
	vpq_memc_suspend_resume.KME_IPME1__KME_IPME1_04 = IoReg_Read32(KME_IPME1__KME_IPME1_04_ADDR);
	vpq_memc_suspend_resume.KME_IPME1__KME_IPME1_08 = IoReg_Read32(KME_IPME1__KME_IPME1_08_ADDR);
	vpq_memc_suspend_resume.KME_IPME1__KME_IPME1_0C = IoReg_Read32(KME_IPME1__KME_IPME1_0C_ADDR);
	//KME_LOGO0
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_00 = IoReg_Read32(KME_LOGO0__KME_LOGO0_00_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_04 = IoReg_Read32(KME_LOGO0__KME_LOGO0_04_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_08 = IoReg_Read32(KME_LOGO0__KME_LOGO0_08_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_0C = IoReg_Read32(KME_LOGO0__KME_LOGO0_0C_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_10 = IoReg_Read32(KME_LOGO0__KME_LOGO0_10_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_14 = IoReg_Read32(KME_LOGO0__KME_LOGO0_14_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_18 = IoReg_Read32(KME_LOGO0__KME_LOGO0_18_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_1C = IoReg_Read32(KME_LOGO0__KME_LOGO0_1C_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_20 = IoReg_Read32(KME_LOGO0__KME_LOGO0_20_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_24 = IoReg_Read32(KME_LOGO0__KME_LOGO0_24_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_28 = IoReg_Read32(KME_LOGO0__KME_LOGO0_28_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_2C = IoReg_Read32(KME_LOGO0__KME_LOGO0_2C_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_30 = IoReg_Read32(KME_LOGO0__KME_LOGO0_30_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_34 = IoReg_Read32(KME_LOGO0__KME_LOGO0_34_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_38 = IoReg_Read32(KME_LOGO0__KME_LOGO0_38_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_3C = IoReg_Read32(KME_LOGO0__KME_LOGO0_3C_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_40 = IoReg_Read32(KME_LOGO0__KME_LOGO0_40_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_44 = IoReg_Read32(KME_LOGO0__KME_LOGO0_44_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_48 = IoReg_Read32(KME_LOGO0__KME_LOGO0_48_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_4C = IoReg_Read32(KME_LOGO0__KME_LOGO0_4C_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_50 = IoReg_Read32(KME_LOGO0__KME_LOGO0_50_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_54 = IoReg_Read32(KME_LOGO0__KME_LOGO0_54_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_58 = IoReg_Read32(KME_LOGO0__KME_LOGO0_58_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_5C = IoReg_Read32(KME_LOGO0__KME_LOGO0_5C_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_60 = IoReg_Read32(KME_LOGO0__KME_LOGO0_60_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_64 = IoReg_Read32(KME_LOGO0__KME_LOGO0_64_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_68 = IoReg_Read32(KME_LOGO0__KME_LOGO0_68_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_6C = IoReg_Read32(KME_LOGO0__KME_LOGO0_6C_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_70 = IoReg_Read32(KME_LOGO0__KME_LOGO0_70_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_74 = IoReg_Read32(KME_LOGO0__KME_LOGO0_74_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_78 = IoReg_Read32(KME_LOGO0__KME_LOGO0_78_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_7C = IoReg_Read32(KME_LOGO0__KME_LOGO0_7C_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_80 = IoReg_Read32(KME_LOGO0__KME_LOGO0_80_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_84 = IoReg_Read32(KME_LOGO0__KME_LOGO0_84_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_88 = IoReg_Read32(KME_LOGO0__KME_LOGO0_88_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_8C = IoReg_Read32(KME_LOGO0__KME_LOGO0_8C_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_90 = IoReg_Read32(KME_LOGO0__KME_LOGO0_90_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_94 = IoReg_Read32(KME_LOGO0__KME_LOGO0_94_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_98 = IoReg_Read32(KME_LOGO0__KME_LOGO0_98_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_9C = IoReg_Read32(KME_LOGO0__KME_LOGO0_9C_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_A0 = IoReg_Read32(KME_LOGO0__KME_LOGO0_A0_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_A4 = IoReg_Read32(KME_LOGO0__KME_LOGO0_A4_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_A8 = IoReg_Read32(KME_LOGO0__KME_LOGO0_A8_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_AC = IoReg_Read32(KME_LOGO0__KME_LOGO0_AC_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_B0 = IoReg_Read32(KME_LOGO0__KME_LOGO0_B0_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_B4 = IoReg_Read32(KME_LOGO0__KME_LOGO0_B4_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_B8 = IoReg_Read32(KME_LOGO0__KME_LOGO0_B8_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_BC = IoReg_Read32(KME_LOGO0__KME_LOGO0_BC_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_C0 = IoReg_Read32(KME_LOGO0__KME_LOGO0_C0_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_C4 = IoReg_Read32(KME_LOGO0__KME_LOGO0_C4_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_C8 = IoReg_Read32(KME_LOGO0__KME_LOGO0_C8_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_CC = IoReg_Read32(KME_LOGO0__KME_LOGO0_CC_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_D0 = IoReg_Read32(KME_LOGO0__KME_LOGO0_D0_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_D4 = IoReg_Read32(KME_LOGO0__KME_LOGO0_D4_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_D8 = IoReg_Read32(KME_LOGO0__KME_LOGO0_D8_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_DC = IoReg_Read32(KME_LOGO0__KME_LOGO0_DC_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_E0 = IoReg_Read32(KME_LOGO0__KME_LOGO0_E0_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_E4 = IoReg_Read32(KME_LOGO0__KME_LOGO0_E4_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_E8 = IoReg_Read32(KME_LOGO0__KME_LOGO0_E8_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_EC = IoReg_Read32(KME_LOGO0__KME_LOGO0_EC_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_F0 = IoReg_Read32(KME_LOGO0__KME_LOGO0_F0_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_F4 = IoReg_Read32(KME_LOGO0__KME_LOGO0_F4_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_F8 = IoReg_Read32(KME_LOGO0__KME_LOGO0_F8_ADDR);
	vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_FC = IoReg_Read32(KME_LOGO0__KME_LOGO0_FC_ADDR);
	//KME_LOGO1
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_00 = IoReg_Read32(KME_LOGO1__KME_LOGO1_00_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_04 = IoReg_Read32(KME_LOGO1__KME_LOGO1_04_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_08 = IoReg_Read32(KME_LOGO1__KME_LOGO1_08_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_0C = IoReg_Read32(KME_LOGO1__KME_LOGO1_0C_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_10 = IoReg_Read32(KME_LOGO1__KME_LOGO1_10_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_14 = IoReg_Read32(KME_LOGO1__KME_LOGO1_14_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_18 = IoReg_Read32(KME_LOGO1__KME_LOGO1_18_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_1C = IoReg_Read32(KME_LOGO1__KME_LOGO1_1C_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_20 = IoReg_Read32(KME_LOGO1__KME_LOGO1_20_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_2C = IoReg_Read32(KME_LOGO1__KME_LOGO1_2C_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_30 = IoReg_Read32(KME_LOGO1__KME_LOGO1_30_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_34 = IoReg_Read32(KME_LOGO1__KME_LOGO1_34_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_38 = IoReg_Read32(KME_LOGO1__KME_LOGO1_38_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_3C = IoReg_Read32(KME_LOGO1__KME_LOGO1_3C_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_40 = IoReg_Read32(KME_LOGO1__KME_LOGO1_40_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_44 = IoReg_Read32(KME_LOGO1__KME_LOGO1_44_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_48 = IoReg_Read32(KME_LOGO1__KME_LOGO1_48_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_4C = IoReg_Read32(KME_LOGO1__KME_LOGO1_4C_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_50 = IoReg_Read32(KME_LOGO1__KME_LOGO1_50_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_54 = IoReg_Read32(KME_LOGO1__KME_LOGO1_54_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_58 = IoReg_Read32(KME_LOGO1__KME_LOGO1_58_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_5C = IoReg_Read32(KME_LOGO1__KME_LOGO1_5C_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_60 = IoReg_Read32(KME_LOGO1__KME_LOGO1_60_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_64 = IoReg_Read32(KME_LOGO1__KME_LOGO1_64_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_68 = IoReg_Read32(KME_LOGO1__KME_LOGO1_68_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_6C = IoReg_Read32(KME_LOGO1__KME_LOGO1_6C_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_70 = IoReg_Read32(KME_LOGO1__KME_LOGO1_70_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_74 = IoReg_Read32(KME_LOGO1__KME_LOGO1_74_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_78 = IoReg_Read32(KME_LOGO1__KME_LOGO1_78_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_7C = IoReg_Read32(KME_LOGO1__KME_LOGO1_7C_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_80 = IoReg_Read32(KME_LOGO1__KME_LOGO1_80_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_84 = IoReg_Read32(KME_LOGO1__KME_LOGO1_84_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_88 = IoReg_Read32(KME_LOGO1__KME_LOGO1_88_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_8C = IoReg_Read32(KME_LOGO1__KME_LOGO1_8C_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_90 = IoReg_Read32(KME_LOGO1__KME_LOGO1_90_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_94 = IoReg_Read32(KME_LOGO1__KME_LOGO1_94_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_98 = IoReg_Read32(KME_LOGO1__KME_LOGO1_98_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_9C = IoReg_Read32(KME_LOGO1__KME_LOGO1_9C_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_A0 = IoReg_Read32(KME_LOGO1__KME_LOGO1_A0_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_A4 = IoReg_Read32(KME_LOGO1__KME_LOGO1_A4_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_A8 = IoReg_Read32(KME_LOGO1__KME_LOGO1_A8_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_AC = IoReg_Read32(KME_LOGO1__KME_LOGO1_AC_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_B0 = IoReg_Read32(KME_LOGO1__KME_LOGO1_B0_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_B4 = IoReg_Read32(KME_LOGO1__KME_LOGO1_B4_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_B8 = IoReg_Read32(KME_LOGO1__KME_LOGO1_B8_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_BC = IoReg_Read32(KME_LOGO1__KME_LOGO1_BC_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_C0 = IoReg_Read32(KME_LOGO1__KME_LOGO1_C0_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_C4 = IoReg_Read32(KME_LOGO1__KME_LOGO1_C4_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_C8 = IoReg_Read32(KME_LOGO1__KME_LOGO1_C8_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_CC = IoReg_Read32(KME_LOGO1__KME_LOGO1_CC_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_D0 = IoReg_Read32(KME_LOGO1__KME_LOGO1_D0_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_D4 = IoReg_Read32(KME_LOGO1__KME_LOGO1_D4_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_D8 = IoReg_Read32(KME_LOGO1__KME_LOGO1_D8_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_DC = IoReg_Read32(KME_LOGO1__KME_LOGO1_DC_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_E0 = IoReg_Read32(KME_LOGO1__KME_LOGO1_E0_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_E4 = IoReg_Read32(KME_LOGO1__KME_LOGO1_E4_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_E8 = IoReg_Read32(KME_LOGO1__KME_LOGO1_E8_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_EC = IoReg_Read32(KME_LOGO1__KME_LOGO1_EC_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_F0 = IoReg_Read32(KME_LOGO1__KME_LOGO1_F0_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_F4 = IoReg_Read32(KME_LOGO1__KME_LOGO1_F4_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_F8 = IoReg_Read32(KME_LOGO1__KME_LOGO1_F8_ADDR);
	vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_FC = IoReg_Read32(KME_LOGO1__KME_LOGO1_FC_ADDR);
	//KME_DM_TOP0
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_START_ADDRESS0 = IoReg_Read32(KME_DM_TOP0__KME_00_START_ADDRESS0_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_START_ADDRESS1 = IoReg_Read32(KME_DM_TOP0__KME_00_START_ADDRESS1_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_START_ADDRESS2 = IoReg_Read32(KME_DM_TOP0__KME_00_START_ADDRESS2_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_START_ADDRESS3 = IoReg_Read32(KME_DM_TOP0__KME_00_START_ADDRESS3_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_START_ADDRESS4 = IoReg_Read32(KME_DM_TOP0__KME_00_START_ADDRESS4_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_START_ADDRESS5 = IoReg_Read32(KME_DM_TOP0__KME_00_START_ADDRESS5_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_END_ADDRESS0 = IoReg_Read32(KME_DM_TOP0__KME_00_END_ADDRESS0_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_END_ADDRESS1 = IoReg_Read32(KME_DM_TOP0__KME_00_END_ADDRESS1_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_END_ADDRESS2 = IoReg_Read32(KME_DM_TOP0__KME_00_END_ADDRESS2_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_END_ADDRESS3 = IoReg_Read32(KME_DM_TOP0__KME_00_END_ADDRESS3_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_END_ADDRESS4 = IoReg_Read32(KME_DM_TOP0__KME_00_END_ADDRESS4_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_END_ADDRESS5 = IoReg_Read32(KME_DM_TOP0__KME_00_END_ADDRESS5_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_LR_OFFSET_ADDR = IoReg_Read32(KME_DM_TOP0__KME_00_LR_OFFSET_ADDR_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_LINE_OFFSET_ADDR = IoReg_Read32(KME_DM_TOP0__KME_00_LINE_OFFSET_ADDR_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_AGENT = IoReg_Read32(KME_DM_TOP0__KME_00_AGENT_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_THRESHOLD = IoReg_Read32(KME_DM_TOP0__KME_00_THRESHOLD_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_01_AGENT = IoReg_Read32(KME_DM_TOP0__KME_01_AGENT_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_01_THRESHOLD = IoReg_Read32(KME_DM_TOP0__KME_01_THRESHOLD_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_02_AGENT = IoReg_Read32(KME_DM_TOP0__KME_02_AGENT_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_02_THRESHOLD = IoReg_Read32(KME_DM_TOP0__KME_02_THRESHOLD_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_03_AGENT = IoReg_Read32(KME_DM_TOP0__KME_03_AGENT_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_03_THRESHOLD = IoReg_Read32(KME_DM_TOP0__KME_03_THRESHOLD_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_04_START_ADDRESS0 = IoReg_Read32(KME_DM_TOP0__KME_04_START_ADDRESS0_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_04_START_ADDRESS1 = IoReg_Read32(KME_DM_TOP0__KME_04_START_ADDRESS1_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_04_END_ADDRESS0 = IoReg_Read32(KME_DM_TOP0__KME_04_END_ADDRESS0_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_04_END_ADDRESS1 = IoReg_Read32(KME_DM_TOP0__KME_04_END_ADDRESS1_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_04_LR_OFFSET_ADDR = IoReg_Read32(KME_DM_TOP0__KME_04_LR_OFFSET_ADDR_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_04_LINE_OFFSET_ADDR = IoReg_Read32(KME_DM_TOP0__KME_04_LINE_OFFSET_ADDR_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_04_AGENT = IoReg_Read32(KME_DM_TOP0__KME_04_AGENT_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_04_THRESHOLD = IoReg_Read32(KME_DM_TOP0__KME_04_THRESHOLD_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_05_START_ADDRESS0 = IoReg_Read32(KME_DM_TOP0__KME_05_START_ADDRESS0_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_05_START_ADDRESS1 = IoReg_Read32(KME_DM_TOP0__KME_05_START_ADDRESS1_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_05_END_ADDRESS0 = IoReg_Read32(KME_DM_TOP0__KME_05_END_ADDRESS0_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_05_END_ADDRESS1 = IoReg_Read32(KME_DM_TOP0__KME_05_END_ADDRESS1_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_05_LR_OFFSET_ADDR = IoReg_Read32(KME_DM_TOP0__KME_05_LR_OFFSET_ADDR_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_05_LINE_OFFSET_ADDR = IoReg_Read32(KME_DM_TOP0__KME_05_LINE_OFFSET_ADDR_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_05_AGENT = IoReg_Read32(KME_DM_TOP0__KME_05_AGENT_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_05_THRESHOLD = IoReg_Read32(KME_DM_TOP0__KME_05_THRESHOLD_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME06AGENT = IoReg_Read32(KME_DM_TOP0__KME06AGENT_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_06_THRESHOLD = IoReg_Read32(KME_DM_TOP0__KME_06_THRESHOLD_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_07_AGENT = IoReg_Read32(KME_DM_TOP0__KME_07_AGENT_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_07_THRESHOLD = IoReg_Read32(KME_DM_TOP0__KME_07_THRESHOLD_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_ME_RESOLUTION = IoReg_Read32(KME_DM_TOP0__KME_ME_RESOLUTION_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_MV_RESOLUTION = IoReg_Read32(KME_DM_TOP0__KME_MV_RESOLUTION_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_SOURCE_MODE = IoReg_Read32(KME_DM_TOP0__KME_SOURCE_MODE_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_ME_HNUM = IoReg_Read32(KME_DM_TOP0__KME_ME_HNUM_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_TIMER_ENABLE = IoReg_Read32(KME_DM_TOP0__KME_TIMER_ENABLE_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_HOLD_TIME0 = IoReg_Read32(KME_DM_TOP0__KME_HOLD_TIME0_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_HOLD_TIME1 = IoReg_Read32(KME_DM_TOP0__KME_HOLD_TIME1_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_HOLD_TIME2 = IoReg_Read32(KME_DM_TOP0__KME_HOLD_TIME2_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_HOLD_TIME3 = IoReg_Read32(KME_DM_TOP0__KME_HOLD_TIME3_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_DM_TOP0_CC = IoReg_Read32(KME_DM_TOP0__KME_DM_TOP0_CC_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__WAIT_TIME1 = IoReg_Read32(KME_DM_TOP0__WAIT_TIME1_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__WAIT_TIME2 = IoReg_Read32(KME_DM_TOP0__WAIT_TIME2_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__MV04_START_ADDRESS2 = IoReg_Read32(KME_DM_TOP0__MV04_START_ADDRESS2_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP0__MV04_END_ADDRESS2 = IoReg_Read32(KME_DM_TOP0__MV04_END_ADDRESS2_ADDR);
	//KME_DM_TOP1
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_08_START_ADDRESS0 = IoReg_Read32(KME_DM_TOP1__KME_08_START_ADDRESS0_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_08_START_ADDRESS1 = IoReg_Read32(KME_DM_TOP1__KME_08_START_ADDRESS1_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_08_END_ADDRESS0 = IoReg_Read32(KME_DM_TOP1__KME_08_END_ADDRESS0_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_08_END_ADDRESS1 = IoReg_Read32(KME_DM_TOP1__KME_08_END_ADDRESS1_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_08_LINE_OFFSET_ADDR = IoReg_Read32(KME_DM_TOP1__KME_08_LINE_OFFSET_ADDR_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_08_AGENT = IoReg_Read32(KME_DM_TOP1__KME_08_AGENT_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_08_THRESHOLD = IoReg_Read32(KME_DM_TOP1__KME_08_THRESHOLD_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_09_AGENT = IoReg_Read32(KME_DM_TOP1__KME_09_AGENT_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_09_THRESHOLD = IoReg_Read32(KME_DM_TOP1__KME_09_THRESHOLD_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_10_START_ADDRESS0 = IoReg_Read32(KME_DM_TOP1__KME_10_START_ADDRESS0_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_10_START_ADDRESS1 = IoReg_Read32(KME_DM_TOP1__KME_10_START_ADDRESS1_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_10_END_ADDRESS0 = IoReg_Read32(KME_DM_TOP1__KME_10_END_ADDRESS0_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_10_END_ADDRESS1 = IoReg_Read32(KME_DM_TOP1__KME_10_END_ADDRESS1_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_10_LINE_OFFSET_ADDR = IoReg_Read32(KME_DM_TOP1__KME_10_LINE_OFFSET_ADDR_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_10_AGENT = IoReg_Read32(KME_DM_TOP1__KME_10_AGENT_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_10_THRESHOLD = IoReg_Read32(KME_DM_TOP1__KME_10_THRESHOLD_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_11_AGENT = IoReg_Read32(KME_DM_TOP1__KME_11_AGENT_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_11_THRESHOLD = IoReg_Read32(KME_DM_TOP1__KME_11_THRESHOLD_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_12_START_ADDRESS0 = IoReg_Read32(KME_DM_TOP1__KME_12_START_ADDRESS0_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_12_START_ADDRESS1 = IoReg_Read32(KME_DM_TOP1__KME_12_START_ADDRESS1_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_12_END_ADDRESS0 = IoReg_Read32(KME_DM_TOP1__KME_12_END_ADDRESS0_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_12_END_ADDRESS1 = IoReg_Read32(KME_DM_TOP1__KME_12_END_ADDRESS1_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_12_LINE_OFFSET_ADDR = IoReg_Read32(KME_DM_TOP1__KME_12_LINE_OFFSET_ADDR_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_12_AGENT = IoReg_Read32(KME_DM_TOP1__KME_12_AGENT_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_12_THRESHOLD = IoReg_Read32(KME_DM_TOP1__KME_12_THRESHOLD_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_13_AGENT = IoReg_Read32(KME_DM_TOP1__KME_13_AGENT_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_13_THRESHOLD = IoReg_Read32(KME_DM_TOP1__KME_13_THRESHOLD_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_14_START_ADDRESS0 = IoReg_Read32(KME_DM_TOP1__KME_14_START_ADDRESS0_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_14_START_ADDRESS1 = IoReg_Read32(KME_DM_TOP1__KME_14_START_ADDRESS1_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_14_END_ADDRESS0 = IoReg_Read32(KME_DM_TOP1__KME_14_END_ADDRESS0_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_14_END_ADDRESS1 = IoReg_Read32(KME_DM_TOP1__KME_14_END_ADDRESS1_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_14_LINE_OFFSET_ADDR = IoReg_Read32(KME_DM_TOP1__KME_14_LINE_OFFSET_ADDR_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_14_AGENT = IoReg_Read32(KME_DM_TOP1__KME_14_AGENT_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_14_THRESHOLD = IoReg_Read32(KME_DM_TOP1__KME_14_THRESHOLD_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_15_AGENT = IoReg_Read32(KME_DM_TOP1__KME_15_AGENT_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_15_THRESHOLD = IoReg_Read32(KME_DM_TOP1__KME_15_THRESHOLD_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_IPLOGO_RESOLUTION = IoReg_Read32(KME_DM_TOP1__KME_IPLOGO_RESOLUTION_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_MLOGO_RESOLUTION = IoReg_Read32(KME_DM_TOP1__KME_MLOGO_RESOLUTION_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_PLOGO_RESOLUTION = IoReg_Read32(KME_DM_TOP1__KME_PLOGO_RESOLUTION_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_HLOGO_RESOLUTION = IoReg_Read32(KME_DM_TOP1__KME_HLOGO_RESOLUTION_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_LOGO_HNUM = IoReg_Read32(KME_DM_TOP1__KME_LOGO_HNUM_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_CLR = IoReg_Read32(KME_DM_TOP1__KME_CLR_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__TH0 = IoReg_Read32(KME_DM_TOP1__TH0_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__TH1 = IoReg_Read32(KME_DM_TOP1__TH1_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__TH2 = IoReg_Read32(KME_DM_TOP1__TH2_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__TH3 = IoReg_Read32(KME_DM_TOP1__TH3_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__TH4 = IoReg_Read32(KME_DM_TOP1__TH4_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP1__METER_TIMER1 = IoReg_Read32(KME_DM_TOP1__METER_TIMER1_ADDR);
	//KME_DM_TOP2
	vpq_memc_suspend_resume.KME_DM_TOP2__MV01_START_ADDRESS0 = IoReg_Read32(KME_DM_TOP2__MV01_START_ADDRESS0_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV01_END_ADDRESS0 = IoReg_Read32(KME_DM_TOP2__MV01_END_ADDRESS0_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV_01_LINE_OFFSET_ADDR = IoReg_Read32(KME_DM_TOP2__MV_01_LINE_OFFSET_ADDR_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV01_LR_OFFSET_ADDR = IoReg_Read32(KME_DM_TOP2__MV01_LR_OFFSET_ADDR_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV01_AGENT = IoReg_Read32(KME_DM_TOP2__MV01_AGENT_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV02_START_ADDRESS11 = IoReg_Read32(KME_DM_TOP2__MV02_START_ADDRESS11_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV02_END_ADDRESS0 = IoReg_Read32(KME_DM_TOP2__MV02_END_ADDRESS0_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV_02_LINE_OFFSET_ADDR = IoReg_Read32(KME_DM_TOP2__MV_02_LINE_OFFSET_ADDR_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV02_LR_OFFSET_ADDR = IoReg_Read32(KME_DM_TOP2__MV02_LR_OFFSET_ADDR_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV02_AGENT = IoReg_Read32(KME_DM_TOP2__MV02_AGENT_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV03_AGENT = IoReg_Read32(KME_DM_TOP2__MV03_AGENT_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV04_START_ADDRESS0 = IoReg_Read32(KME_DM_TOP2__MV04_START_ADDRESS0_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV04_START_ADDRESS1 = IoReg_Read32(KME_DM_TOP2__MV04_START_ADDRESS1_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV04_END_ADDRESS0 = IoReg_Read32(KME_DM_TOP2__MV04_END_ADDRESS0_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV04_END_ADDRESS1 = IoReg_Read32(KME_DM_TOP2__MV04_END_ADDRESS1_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV_04_LINE_OFFSET_ADDR = IoReg_Read32(KME_DM_TOP2__MV_04_LINE_OFFSET_ADDR_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV04_LR_OFFSET_ADDR = IoReg_Read32(KME_DM_TOP2__MV04_LR_OFFSET_ADDR_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV04_AGENT = IoReg_Read32(KME_DM_TOP2__MV04_AGENT_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV05_START_ADDRESS0 = IoReg_Read32(KME_DM_TOP2__MV05_START_ADDRESS0_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV05_START_ADDRESS1 = IoReg_Read32(KME_DM_TOP2__MV05_START_ADDRESS1_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV05_END_ADDRESS0 = IoReg_Read32(KME_DM_TOP2__MV05_END_ADDRESS0_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV05_END_ADDRESS1 = IoReg_Read32(KME_DM_TOP2__MV05_END_ADDRESS1_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV_05_LINE_OFFSET_ADDR = IoReg_Read32(KME_DM_TOP2__MV_05_LINE_OFFSET_ADDR_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV05_LR_OFFSET_ADDR = IoReg_Read32(KME_DM_TOP2__MV05_LR_OFFSET_ADDR_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV05_AGENT = IoReg_Read32(KME_DM_TOP2__MV05_AGENT_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV06_AGENT = IoReg_Read32(KME_DM_TOP2__MV06_AGENT_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV07_AGENT = IoReg_Read32(KME_DM_TOP2__MV07_AGENT_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV08_AGENT = IoReg_Read32(KME_DM_TOP2__MV08_AGENT_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV09_AGENT = IoReg_Read32(KME_DM_TOP2__MV09_AGENT_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV10_AGENT = IoReg_Read32(KME_DM_TOP2__MV10_AGENT_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV11_AGENT = IoReg_Read32(KME_DM_TOP2__MV11_AGENT_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__KME_00_START_ADDRESS6 = IoReg_Read32(KME_DM_TOP2__KME_00_START_ADDRESS6_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__KME_00_START_ADDRESS7 = IoReg_Read32(KME_DM_TOP2__KME_00_START_ADDRESS7_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__KME_00_END_ADDRESS6 = IoReg_Read32(KME_DM_TOP2__KME_00_END_ADDRESS6_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__KME_00_END_ADDRESS7 = IoReg_Read32(KME_DM_TOP2__KME_00_END_ADDRESS7_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV_SOURCE_MODE = IoReg_Read32(KME_DM_TOP2__MV_SOURCE_MODE_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV01_RESOLUTION = IoReg_Read32(KME_DM_TOP2__MV01_RESOLUTION_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV02_RESOLUTION = IoReg_Read32(KME_DM_TOP2__MV02_RESOLUTION_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV04_RESOLUTION = IoReg_Read32(KME_DM_TOP2__MV04_RESOLUTION_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV05_RESOLUTION = IoReg_Read32(KME_DM_TOP2__MV05_RESOLUTION_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV_TIME_ENABLE = IoReg_Read32(KME_DM_TOP2__MV_TIME_ENABLE_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV_HOLD_TIME0 = IoReg_Read32(KME_DM_TOP2__MV_HOLD_TIME0_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV_HOLD_TIME1 = IoReg_Read32(KME_DM_TOP2__MV_HOLD_TIME1_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV_HOLD_TIME2 = IoReg_Read32(KME_DM_TOP2__MV_HOLD_TIME2_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV_WAIT_TIME0 = IoReg_Read32(KME_DM_TOP2__MV_WAIT_TIME0_ADDR);
	vpq_memc_suspend_resume.KME_DM_TOP2__MV_WAIT_TIME1 = IoReg_Read32(KME_DM_TOP2__MV_WAIT_TIME1_ADDR);
	//KME_DEHALO3
	vpq_memc_suspend_resume.KME_DEHALO3__KME_DEHALO3_0C = IoReg_Read32(KME_DEHALO3__KME_DEHALO3_0C_ADDR);
	//KME_ME1_TOP0
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_00 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_00_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_04 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_04_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_08 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_08_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_0C = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_0C_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_10 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_10_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_14 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_14_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_18 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_18_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_1C = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_1C_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_20 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_20_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_24 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_24_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_28 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_28_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_2C = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_2C_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_30 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_30_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_34 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_34_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_38 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_38_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_40 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_40_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_44 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_44_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_48 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_48_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_4C = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_4C_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_50 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_50_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_54 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_54_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_58 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_58_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_5C = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_5C_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_60 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_60_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_64 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_64_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_68 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_68_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_6C = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_6C_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_70 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_70_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_74 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_74_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_78 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_78_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_7C = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_7C_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_80 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_80_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_84 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_84_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_88 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_88_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_8C = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_8C_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_90 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_90_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_94 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_94_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_98 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_98_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_9C = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_9C_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_A0 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_A0_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_A4 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_A4_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_B0 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_B0_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_B4 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_B4_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_B8 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_B8_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_BC = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_BC_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_C0 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_C0_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_C4 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_C4_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_EC = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_EC_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_F0 = IoReg_Read32(KME_ME1_TOP0__KME_ME1_TOP0_F0_ADDR);
	//KME_ME1_TOP1
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_00 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_00_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_01 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_01_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_02 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_02_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_03 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_03_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_04 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_04_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_05 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_05_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_06 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_06_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_07 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_07_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_08 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_08_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_09 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_09_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_10 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_10_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_11 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_11_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_12 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_12_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_13 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_13_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_14 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_14_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_15 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_15_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_16 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_16_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_17 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_17_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_18 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_18_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_19 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_19_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_20 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_20_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_21 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_21_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_22 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_22_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_23 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_23_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_24 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_24_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_25 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_25_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__KME_ME1_TOP1_68 = IoReg_Read32(KME_ME1_TOP1__KME_ME1_TOP1_68_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_27 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_27_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_28 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_28_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_29 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_29_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_30 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_30_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_31 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_31_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_32 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_32_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_33 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_33_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_34 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_34_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_35 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_35_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_36 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_36_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_37 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_37_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_38 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_38_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_39 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_39_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_40 = IoReg_Read32(KME_ME1_TOP1__ME1_COMMON1_40_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_SCENE1_00 = IoReg_Read32(KME_ME1_TOP1__ME1_SCENE1_00_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_SCENE1_01 = IoReg_Read32(KME_ME1_TOP1__ME1_SCENE1_01_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_SCENE1_02 = IoReg_Read32(KME_ME1_TOP1__ME1_SCENE1_02_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_SCENE1_03 = IoReg_Read32(KME_ME1_TOP1__ME1_SCENE1_03_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_SCENE1_04 = IoReg_Read32(KME_ME1_TOP1__ME1_SCENE1_04_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_SCENE1_05 = IoReg_Read32(KME_ME1_TOP1__ME1_SCENE1_05_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_SCENE1_06 = IoReg_Read32(KME_ME1_TOP1__ME1_SCENE1_06_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_SCENE1_07 = IoReg_Read32(KME_ME1_TOP1__ME1_SCENE1_07_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_SCENE1_08 = IoReg_Read32(KME_ME1_TOP1__ME1_SCENE1_08_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_SCENE1_09 = IoReg_Read32(KME_ME1_TOP1__ME1_SCENE1_09_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_SCENE1_10 = IoReg_Read32(KME_ME1_TOP1__ME1_SCENE1_10_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_SCENE1_11 = IoReg_Read32(KME_ME1_TOP1__ME1_SCENE1_11_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_SCENE1_12 = IoReg_Read32(KME_ME1_TOP1__ME1_SCENE1_12_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP1__KME_ME1_TOP1_F4 = IoReg_Read32(KME_ME1_TOP1__KME_ME1_TOP1_F4_ADDR);
	//KME_LBME2_TOP
	vpq_memc_suspend_resume.KME_LBME2_TOP__KME_LBME2_TOP_00 = IoReg_Read32(KME_LBME2_TOP__KME_LBME2_TOP_00_ADDR);
	vpq_memc_suspend_resume.KME_LBME2_TOP__KME_LBME2_TOP_04 = IoReg_Read32(KME_LBME2_TOP__KME_LBME2_TOP_04_ADDR);
	vpq_memc_suspend_resume.KME_LBME2_TOP__KME_LBME2_TOP_08 = IoReg_Read32(KME_LBME2_TOP__KME_LBME2_TOP_08_ADDR);
	vpq_memc_suspend_resume.KME_LBME2_TOP__KME_LBME2_TOP_0C = IoReg_Read32(KME_LBME2_TOP__KME_LBME2_TOP_0C_ADDR);
	vpq_memc_suspend_resume.KME_LBME2_TOP__KME_LBME2_TOP_10 = IoReg_Read32(KME_LBME2_TOP__KME_LBME2_TOP_10_ADDR);
	vpq_memc_suspend_resume.KME_LBME2_TOP__KME_LBME2_TOP_14 = IoReg_Read32(KME_LBME2_TOP__KME_LBME2_TOP_14_ADDR);
	vpq_memc_suspend_resume.KME_LBME2_TOP__KME_LBME2_TOP_18 = IoReg_Read32(KME_LBME2_TOP__KME_LBME2_TOP_18_ADDR);
	//KME_ME2_VBUF_TOP
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_00 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_00_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_04 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_04_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_08 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_08_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_0C = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_0C_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_10 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_10_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_14 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_14_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_18 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_18_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_1C = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_1C_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_20 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_20_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_24 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_24_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_28 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_28_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_2C = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_2C_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_30 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_30_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_34 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_34_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_38 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_38_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_3C = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_3C_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_40 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_40_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_44 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_44_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_48 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_48_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_4C = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_4C_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_50 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_50_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_54 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_54_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_58 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_58_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_5C = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_5C_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_60 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_60_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_64 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_64_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_68 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_68_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_6C = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_6C_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_70 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_70_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_74 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_74_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_78 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_78_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_7C = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_7C_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_80 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_80_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_84 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_84_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_88 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_88_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_8C = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_8C_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_90 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_90_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_94 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_94_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_98 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_98_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_9C = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_9C_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_A0 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_A0_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_A4 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_A4_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_A8 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_A8_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_AC = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_AC_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_B0 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_B0_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_B4 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_B4_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_B8 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_B8_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_BC = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_BC_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_C0 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_C0_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_C4 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_C4_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_C8 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_C8_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_CC = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_CC_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_D0 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_D0_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_D4 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_D4_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_D8 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_D8_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_DC = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_DC_ADDR);
	vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_E0 = IoReg_Read32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_E0_ADDR);
	//KME_ME2_CALC0
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_00 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_00_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_04 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_04_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_08 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_08_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_0C = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_0C_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_10 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_10_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_14 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_14_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_18 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_18_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_1C = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_1C_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_20 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_20_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_24 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_24_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_28 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_28_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_2C = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_2C_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_30 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_30_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_34 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_34_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_38 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_38_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_3C = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_3C_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_40 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_40_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_44 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_44_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_48 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_48_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_4C = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_4C_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_50 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_50_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_54 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_54_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_60 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_60_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_64 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_64_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_68 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_68_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_6C = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_6C_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_70 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_70_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_74 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_74_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_78 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_78_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_7C = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_7C_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_80 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_80_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_84 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_84_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_88 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_88_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_8C = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_8C_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_90 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_90_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_94 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_94_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_98 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_98_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_9C = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_9C_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_A0 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_A0_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_A4 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_A4_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_A8 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_A8_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_AC = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_AC_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_B0 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_B0_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_B4 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_B4_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_B8 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_B8_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_BC = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_BC_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_C0 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_C0_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_C4 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_C4_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_C8 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_C8_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_CC = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_CC_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_D0 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_D0_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_D4 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_D4_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_D8 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_D8_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_DC = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_DC_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_E0 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_E0_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_E4 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_E4_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_E8 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_E8_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_EC = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_EC_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_F0 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_F0_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_F4 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_F4_ADDR);
	vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_F8 = IoReg_Read32(KME_ME2_CALC0__KME_ME2_CALC0_F8_ADDR);
	//KME_ME1_TOP6
	vpq_memc_suspend_resume.KME_ME1_TOP6__KME_ME1_TOP6_00 = IoReg_Read32(KME_ME1_TOP6__KME_ME1_TOP6_00_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP6__KME_ME1_TOP6_04 = IoReg_Read32(KME_ME1_TOP6__KME_ME1_TOP6_04_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP6__KME_ME1_TOP6_08 = IoReg_Read32(KME_ME1_TOP6__KME_ME1_TOP6_08_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP6__KME_ME1_TOP6_0C = IoReg_Read32(KME_ME1_TOP6__KME_ME1_TOP6_0C_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP6__KME_ME1_TOP6_10 = IoReg_Read32(KME_ME1_TOP6__KME_ME1_TOP6_10_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP6__KME_ME1_TOP6_14 = IoReg_Read32(KME_ME1_TOP6__KME_ME1_TOP6_14_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP6__KME_ME1_TOP6_18 = IoReg_Read32(KME_ME1_TOP6__KME_ME1_TOP6_18_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP6__KME_ME1_TOP6_1C = IoReg_Read32(KME_ME1_TOP6__KME_ME1_TOP6_1C_ADDR);
	vpq_memc_suspend_resume.KME_ME1_TOP6__KME_ME1_TOP6_20 = IoReg_Read32(KME_ME1_TOP6__KME_ME1_TOP6_20_ADDR);
	//KME_ME1_TOP7
	//KME_ME1_TOP8
	//KME_ME1_TOP9
	//KME_DEHALO
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_10 = IoReg_Read32(KME_DEHALO__KME_DEHALO_10_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_14 = IoReg_Read32(KME_DEHALO__KME_DEHALO_14_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_18 = IoReg_Read32(KME_DEHALO__KME_DEHALO_18_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_1C = IoReg_Read32(KME_DEHALO__KME_DEHALO_1C_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_20 = IoReg_Read32(KME_DEHALO__KME_DEHALO_20_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_24 = IoReg_Read32(KME_DEHALO__KME_DEHALO_24_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_28 = IoReg_Read32(KME_DEHALO__KME_DEHALO_28_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_2C = IoReg_Read32(KME_DEHALO__KME_DEHALO_2C_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_30 = IoReg_Read32(KME_DEHALO__KME_DEHALO_30_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_34 = IoReg_Read32(KME_DEHALO__KME_DEHALO_34_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_38 = IoReg_Read32(KME_DEHALO__KME_DEHALO_38_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_3C = IoReg_Read32(KME_DEHALO__KME_DEHALO_3C_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_40 = IoReg_Read32(KME_DEHALO__KME_DEHALO_40_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_44 = IoReg_Read32(KME_DEHALO__KME_DEHALO_44_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_48 = IoReg_Read32(KME_DEHALO__KME_DEHALO_48_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_4C = IoReg_Read32(KME_DEHALO__KME_DEHALO_4C_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_50 = IoReg_Read32(KME_DEHALO__KME_DEHALO_50_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_54 = IoReg_Read32(KME_DEHALO__KME_DEHALO_54_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_58 = IoReg_Read32(KME_DEHALO__KME_DEHALO_58_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_5C = IoReg_Read32(KME_DEHALO__KME_DEHALO_5C_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_60 = IoReg_Read32(KME_DEHALO__KME_DEHALO_60_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_64 = IoReg_Read32(KME_DEHALO__KME_DEHALO_64_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_68 = IoReg_Read32(KME_DEHALO__KME_DEHALO_68_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_6C = IoReg_Read32(KME_DEHALO__KME_DEHALO_6C_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_70 = IoReg_Read32(KME_DEHALO__KME_DEHALO_70_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_74 = IoReg_Read32(KME_DEHALO__KME_DEHALO_74_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_78 = IoReg_Read32(KME_DEHALO__KME_DEHALO_78_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_7C = IoReg_Read32(KME_DEHALO__KME_DEHALO_7C_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_80 = IoReg_Read32(KME_DEHALO__KME_DEHALO_80_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_84 = IoReg_Read32(KME_DEHALO__KME_DEHALO_84_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_88 = IoReg_Read32(KME_DEHALO__KME_DEHALO_88_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_8C = IoReg_Read32(KME_DEHALO__KME_DEHALO_8C_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_90 = IoReg_Read32(KME_DEHALO__KME_DEHALO_90_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_94 = IoReg_Read32(KME_DEHALO__KME_DEHALO_94_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_A8 = IoReg_Read32(KME_DEHALO__KME_DEHALO_A8_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_AC = IoReg_Read32(KME_DEHALO__KME_DEHALO_AC_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_B0 = IoReg_Read32(KME_DEHALO__KME_DEHALO_B0_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_B4 = IoReg_Read32(KME_DEHALO__KME_DEHALO_B4_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_B8 = IoReg_Read32(KME_DEHALO__KME_DEHALO_B8_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_BC = IoReg_Read32(KME_DEHALO__KME_DEHALO_BC_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_C0 = IoReg_Read32(KME_DEHALO__KME_DEHALO_C0_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_C4 = IoReg_Read32(KME_DEHALO__KME_DEHALO_C4_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_C8 = IoReg_Read32(KME_DEHALO__KME_DEHALO_C8_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_CC = IoReg_Read32(KME_DEHALO__KME_DEHALO_CC_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_D0 = IoReg_Read32(KME_DEHALO__KME_DEHALO_D0_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_D4 = IoReg_Read32(KME_DEHALO__KME_DEHALO_D4_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_D8 = IoReg_Read32(KME_DEHALO__KME_DEHALO_D8_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_DC = IoReg_Read32(KME_DEHALO__KME_DEHALO_DC_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_E0 = IoReg_Read32(KME_DEHALO__KME_DEHALO_E0_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_E4 = IoReg_Read32(KME_DEHALO__KME_DEHALO_E4_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_E8 = IoReg_Read32(KME_DEHALO__KME_DEHALO_E8_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_EC = IoReg_Read32(KME_DEHALO__KME_DEHALO_EC_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_F0 = IoReg_Read32(KME_DEHALO__KME_DEHALO_F0_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_F4 = IoReg_Read32(KME_DEHALO__KME_DEHALO_F4_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_F8 = IoReg_Read32(KME_DEHALO__KME_DEHALO_F8_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_FC = IoReg_Read32(KME_DEHALO__KME_DEHALO_FC_ADDR);
	//KME_DEHALO2
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_08 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_08_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_10 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_10_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_14 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_14_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_18 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_18_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_1C = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_1C_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_20 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_20_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_24 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_24_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_28 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_28_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_2C = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_2C_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_30 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_30_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_34 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_34_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_38 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_38_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_3C = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_3C_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_40 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_40_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_44 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_44_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_48 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_48_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_4C = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_4C_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_50 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_50_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_54 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_54_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_58 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_58_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_74 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_74_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_78 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_78_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_7C = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_7C_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_80 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_80_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_84 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_84_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_88 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_88_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_8C = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_8C_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_90 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_90_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_94 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_94_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_98 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_98_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_9C = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_9C_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_A0 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_A0_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_A4 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_A4_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_A8 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_A8_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_AC = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_AC_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_B0 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_B0_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_B4 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_B4_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_B8 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_B8_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_BC = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_BC_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_C0 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_C0_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_C4 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_C4_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_C8 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_C8_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_CC = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_CC_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_D0 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_D0_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_D4 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_D4_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_D8 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_D8_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_DC = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_DC_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_E0 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_E0_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_E4 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_E4_ADDR);
	vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_E8 = IoReg_Read32(KME_DEHALO2__KME_DEHALO2_E8_ADDR);
	//KPOST_TOP
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_00 = IoReg_Read32(KPOST_TOP__KPOST_TOP_00_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_04 = IoReg_Read32(KPOST_TOP__KPOST_TOP_04_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_08 = IoReg_Read32(KPOST_TOP__KPOST_TOP_08_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_0C = IoReg_Read32(KPOST_TOP__KPOST_TOP_0C_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_10 = IoReg_Read32(KPOST_TOP__KPOST_TOP_10_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_14 = IoReg_Read32(KPOST_TOP__KPOST_TOP_14_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_18 = IoReg_Read32(KPOST_TOP__KPOST_TOP_18_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_1C = IoReg_Read32(KPOST_TOP__KPOST_TOP_1C_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_20 = IoReg_Read32(KPOST_TOP__KPOST_TOP_20_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_24 = IoReg_Read32(KPOST_TOP__KPOST_TOP_24_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_28 = IoReg_Read32(KPOST_TOP__KPOST_TOP_28_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_30 = IoReg_Read32(KPOST_TOP__KPOST_TOP_30_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_34 = IoReg_Read32(KPOST_TOP__KPOST_TOP_34_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_38 = IoReg_Read32(KPOST_TOP__KPOST_TOP_38_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_3C = IoReg_Read32(KPOST_TOP__KPOST_TOP_3C_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_40 = IoReg_Read32(KPOST_TOP__KPOST_TOP_40_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_44 = IoReg_Read32(KPOST_TOP__KPOST_TOP_44_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_48 = IoReg_Read32(KPOST_TOP__KPOST_TOP_48_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_4C = IoReg_Read32(KPOST_TOP__KPOST_TOP_4C_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_50 = IoReg_Read32(KPOST_TOP__KPOST_TOP_50_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_60 = IoReg_Read32(KPOST_TOP__KPOST_TOP_60_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_64 = IoReg_Read32(KPOST_TOP__KPOST_TOP_64_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_68 = IoReg_Read32(KPOST_TOP__KPOST_TOP_68_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_6C = IoReg_Read32(KPOST_TOP__KPOST_TOP_6C_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_80 = IoReg_Read32(KPOST_TOP__KPOST_TOP_80_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_84 = IoReg_Read32(KPOST_TOP__KPOST_TOP_84_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_88 = IoReg_Read32(KPOST_TOP__KPOST_TOP_88_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_8C = IoReg_Read32(KPOST_TOP__KPOST_TOP_8C_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_90 = IoReg_Read32(KPOST_TOP__KPOST_TOP_90_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_94 = IoReg_Read32(KPOST_TOP__KPOST_TOP_94_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_A0 = IoReg_Read32(KPOST_TOP__KPOST_TOP_A0_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_A4 = IoReg_Read32(KPOST_TOP__KPOST_TOP_A4_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_A8 = IoReg_Read32(KPOST_TOP__KPOST_TOP_A8_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_AC = IoReg_Read32(KPOST_TOP__KPOST_TOP_AC_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_B0 = IoReg_Read32(KPOST_TOP__KPOST_TOP_B0_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_B4 = IoReg_Read32(KPOST_TOP__KPOST_TOP_B4_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_B8 = IoReg_Read32(KPOST_TOP__KPOST_TOP_B8_ADDR);
	vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_BC = IoReg_Read32(KPOST_TOP__KPOST_TOP_BC_ADDR);
	//CRTC1
	vpq_memc_suspend_resume.CRTC1__CRTC1_00 = IoReg_Read32(CRTC1__CRTC1_00_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_04 = IoReg_Read32(CRTC1__CRTC1_04_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_08 = IoReg_Read32(CRTC1__CRTC1_08_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_0C = IoReg_Read32(CRTC1__CRTC1_0C_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_10 = IoReg_Read32(CRTC1__CRTC1_10_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_14 = IoReg_Read32(CRTC1__CRTC1_14_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_18 = IoReg_Read32(CRTC1__CRTC1_18_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_1C = IoReg_Read32(CRTC1__CRTC1_1C_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_20 = IoReg_Read32(CRTC1__CRTC1_20_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_24 = IoReg_Read32(CRTC1__CRTC1_24_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_28 = IoReg_Read32(CRTC1__CRTC1_28_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_2C = IoReg_Read32(CRTC1__CRTC1_2C_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_30 = IoReg_Read32(CRTC1__CRTC1_30_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_34 = IoReg_Read32(CRTC1__CRTC1_34_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_38 = IoReg_Read32(CRTC1__CRTC1_38_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_3C = IoReg_Read32(CRTC1__CRTC1_3C_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_40 = IoReg_Read32(CRTC1__CRTC1_40_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_44 = IoReg_Read32(CRTC1__CRTC1_44_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_48 = IoReg_Read32(CRTC1__CRTC1_48_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_60 = IoReg_Read32(CRTC1__CRTC1_60_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_64 = IoReg_Read32(CRTC1__CRTC1_64_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_68 = IoReg_Read32(CRTC1__CRTC1_68_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_6C = IoReg_Read32(CRTC1__CRTC1_6C_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_70 = IoReg_Read32(CRTC1__CRTC1_70_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_74 = IoReg_Read32(CRTC1__CRTC1_74_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_78 = IoReg_Read32(CRTC1__CRTC1_78_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_7C = IoReg_Read32(CRTC1__CRTC1_7C_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_80 = IoReg_Read32(CRTC1__CRTC1_80_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_84 = IoReg_Read32(CRTC1__CRTC1_84_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_88 = IoReg_Read32(CRTC1__CRTC1_88_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_8C = IoReg_Read32(CRTC1__CRTC1_8C_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_90 = IoReg_Read32(CRTC1__CRTC1_90_ADDR);
	vpq_memc_suspend_resume.CRTC1__CRTC1_94 = IoReg_Read32(CRTC1__CRTC1_94_ADDR);
	//kphase
	vpq_memc_suspend_resume.KPHASE__KPHASE_00 = IoReg_Read32(KPHASE__KPHASE_00_ADDR);
	vpq_memc_suspend_resume.KPHASE__KPHASE_04 = IoReg_Read32(KPHASE__KPHASE_04_ADDR);
	vpq_memc_suspend_resume.KPHASE__KPHASE_08 = IoReg_Read32(KPHASE__KPHASE_08_ADDR);
	vpq_memc_suspend_resume.KPHASE__KPHASE_0C = IoReg_Read32(KPHASE__KPHASE_0C_ADDR);
	vpq_memc_suspend_resume.KPHASE__KPHASE_10 = IoReg_Read32(KPHASE__KPHASE_10_ADDR);
	vpq_memc_suspend_resume.KPHASE__KPHASE_14 = IoReg_Read32(KPHASE__KPHASE_14_ADDR);
	vpq_memc_suspend_resume.KPHASE__KPHASE_18 = IoReg_Read32(KPHASE__KPHASE_18_ADDR);
	vpq_memc_suspend_resume.KPHASE__KPHASE_1C = IoReg_Read32(KPHASE__KPHASE_1C_ADDR);
	vpq_memc_suspend_resume.KPHASE__KPHASE_20 = IoReg_Read32(KPHASE__KPHASE_20_ADDR);
	vpq_memc_suspend_resume.KPHASE__KPHASE_24 = IoReg_Read32(KPHASE__KPHASE_24_ADDR);
	vpq_memc_suspend_resume.KPHASE__KPHASE_28 = IoReg_Read32(KPHASE__KPHASE_28_ADDR);
	vpq_memc_suspend_resume.KPHASE__KPHASE_2C = IoReg_Read32(KPHASE__KPHASE_2C_ADDR);
	vpq_memc_suspend_resume.KPHASE__KPHASE_30 = IoReg_Read32(KPHASE__KPHASE_30_ADDR);
	vpq_memc_suspend_resume.KPHASE__KPHASE_34 = IoReg_Read32(KPHASE__KPHASE_34_ADDR);
	vpq_memc_suspend_resume.KPHASE__KPHASE_38 = IoReg_Read32(KPHASE__KPHASE_38_ADDR);
	vpq_memc_suspend_resume.KPHASE__KPHASE_3C = IoReg_Read32(KPHASE__KPHASE_3C_ADDR);
	vpq_memc_suspend_resume.KPHASE__KPHASE_40 = IoReg_Read32(KPHASE__KPHASE_40_ADDR);
	vpq_memc_suspend_resume.KPHASE__KPHASE_44 = IoReg_Read32(KPHASE__KPHASE_44_ADDR);
	vpq_memc_suspend_resume.KPHASE__KPHASE_48 = IoReg_Read32(KPHASE__KPHASE_48_ADDR);
	vpq_memc_suspend_resume.KPHASE__KPHASE_4C = IoReg_Read32(KPHASE__KPHASE_4C_ADDR);
	vpq_memc_suspend_resume.KPHASE__KPHASE_50 = IoReg_Read32(KPHASE__KPHASE_50_ADDR);
	vpq_memc_suspend_resume.KPHASE__KPHASE_54 = IoReg_Read32(KPHASE__KPHASE_54_ADDR);
	vpq_memc_suspend_resume.KPHASE__KPHASE_58 = IoReg_Read32(KPHASE__KPHASE_58_ADDR);
	vpq_memc_suspend_resume.KPHASE__KPHASE_5C = IoReg_Read32(KPHASE__KPHASE_5C_ADDR);
	vpq_memc_suspend_resume.KPHASE__KPHASE_60 = IoReg_Read32(KPHASE__KPHASE_60_ADDR);
	vpq_memc_suspend_resume.KPHASE__KPHASE_64 = IoReg_Read32(KPHASE__KPHASE_64_ADDR);
	vpq_memc_suspend_resume.KPHASE__KPHASE_68 = IoReg_Read32(KPHASE__KPHASE_68_ADDR);
	vpq_memc_suspend_resume.KPHASE__KPHASE_6C = IoReg_Read32(KPHASE__KPHASE_6C_ADDR);
	vpq_memc_suspend_resume.KPHASE__KPHASE_70 = IoReg_Read32(KPHASE__KPHASE_70_ADDR);
	vpq_memc_suspend_resume.KPHASE__KPHASE_74 = IoReg_Read32(KPHASE__KPHASE_74_ADDR);
	vpq_memc_suspend_resume.KPHASE__KPHASE_78 = IoReg_Read32(KPHASE__KPHASE_78_ADDR);
	//Hardware
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_00 = IoReg_Read32(HARDWARE__HARDWARE_00_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_01 = IoReg_Read32(HARDWARE__HARDWARE_01_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_02 = IoReg_Read32(HARDWARE__HARDWARE_02_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_03 = IoReg_Read32(HARDWARE__HARDWARE_03_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_04 = IoReg_Read32(HARDWARE__HARDWARE_04_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_05 = IoReg_Read32(HARDWARE__HARDWARE_05_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_06 = IoReg_Read32(HARDWARE__HARDWARE_06_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_07 = IoReg_Read32(HARDWARE__HARDWARE_07_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_08 = IoReg_Read32(HARDWARE__HARDWARE_08_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_09 = IoReg_Read32(HARDWARE__HARDWARE_09_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_10 = IoReg_Read32(HARDWARE__HARDWARE_10_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_11 = IoReg_Read32(HARDWARE__HARDWARE_11_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_12 = IoReg_Read32(HARDWARE__HARDWARE_12_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_13 = IoReg_Read32(HARDWARE__HARDWARE_13_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_14 = IoReg_Read32(HARDWARE__HARDWARE_14_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_15 = IoReg_Read32(HARDWARE__HARDWARE_15_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_16 = IoReg_Read32(HARDWARE__HARDWARE_16_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_17 = IoReg_Read32(HARDWARE__HARDWARE_17_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_18 = IoReg_Read32(HARDWARE__HARDWARE_18_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_19 = IoReg_Read32(HARDWARE__HARDWARE_19_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_20 = IoReg_Read32(HARDWARE__HARDWARE_20_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_21 = IoReg_Read32(HARDWARE__HARDWARE_21_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_22 = IoReg_Read32(HARDWARE__HARDWARE_22_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_23 = IoReg_Read32(HARDWARE__HARDWARE_23_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_24 = IoReg_Read32(HARDWARE__HARDWARE_24_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_25 = IoReg_Read32(HARDWARE__HARDWARE_25_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_26 = IoReg_Read32(HARDWARE__HARDWARE_26_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_27 = IoReg_Read32(HARDWARE__HARDWARE_27_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_28 = IoReg_Read32(HARDWARE__HARDWARE_28_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_29 = IoReg_Read32(HARDWARE__HARDWARE_29_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_30 = IoReg_Read32(HARDWARE__HARDWARE_30_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_31 = IoReg_Read32(HARDWARE__HARDWARE_31_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_32 = IoReg_Read32(HARDWARE__HARDWARE_32_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_33 = IoReg_Read32(HARDWARE__HARDWARE_33_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_34 = IoReg_Read32(HARDWARE__HARDWARE_34_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_35 = IoReg_Read32(HARDWARE__HARDWARE_35_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_36 = IoReg_Read32(HARDWARE__HARDWARE_36_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_37 = IoReg_Read32(HARDWARE__HARDWARE_37_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_38 = IoReg_Read32(HARDWARE__HARDWARE_38_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_39 = IoReg_Read32(HARDWARE__HARDWARE_39_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_40 = IoReg_Read32(HARDWARE__HARDWARE_40_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_41 = IoReg_Read32(HARDWARE__HARDWARE_41_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_42 = IoReg_Read32(HARDWARE__HARDWARE_42_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_43 = IoReg_Read32(HARDWARE__HARDWARE_43_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_44 = IoReg_Read32(HARDWARE__HARDWARE_44_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_45 = IoReg_Read32(HARDWARE__HARDWARE_45_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_46 = IoReg_Read32(HARDWARE__HARDWARE_46_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_47 = IoReg_Read32(HARDWARE__HARDWARE_47_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_48 = IoReg_Read32(HARDWARE__HARDWARE_48_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_49 = IoReg_Read32(HARDWARE__HARDWARE_49_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_50 = IoReg_Read32(HARDWARE__HARDWARE_50_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_51 = IoReg_Read32(HARDWARE__HARDWARE_51_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_52 = IoReg_Read32(HARDWARE__HARDWARE_52_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_53 = IoReg_Read32(HARDWARE__HARDWARE_53_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_54 = IoReg_Read32(HARDWARE__HARDWARE_54_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_55 = IoReg_Read32(HARDWARE__HARDWARE_55_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_56 = IoReg_Read32(HARDWARE__HARDWARE_56_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_57 = IoReg_Read32(HARDWARE__HARDWARE_57_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_58 = IoReg_Read32(HARDWARE__HARDWARE_58_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_59 = IoReg_Read32(HARDWARE__HARDWARE_59_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_60 = IoReg_Read32(HARDWARE__HARDWARE_60_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_61 = IoReg_Read32(HARDWARE__HARDWARE_61_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_62 = IoReg_Read32(HARDWARE__HARDWARE_62_ADDR);
	vpq_memc_suspend_resume.HARDWARE__HARDWARE_63 = IoReg_Read32(HARDWARE__HARDWARE_63_ADDR);
	//Software
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_00 = IoReg_Read32(SOFTWARE__SOFTWARE_00_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_01 = IoReg_Read32(SOFTWARE__SOFTWARE_01_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_02 = IoReg_Read32(SOFTWARE__SOFTWARE_02_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_03 = IoReg_Read32(SOFTWARE__SOFTWARE_03_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_04 = IoReg_Read32(SOFTWARE__SOFTWARE_04_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_05 = IoReg_Read32(SOFTWARE__SOFTWARE_05_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_06 = IoReg_Read32(SOFTWARE__SOFTWARE_06_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_07 = IoReg_Read32(SOFTWARE__SOFTWARE_07_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_08 = IoReg_Read32(SOFTWARE__SOFTWARE_08_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_09 = IoReg_Read32(SOFTWARE__SOFTWARE_09_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_10 = IoReg_Read32(SOFTWARE__SOFTWARE_10_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_11 = IoReg_Read32(SOFTWARE__SOFTWARE_11_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_12 = IoReg_Read32(SOFTWARE__SOFTWARE_12_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_13 = IoReg_Read32(SOFTWARE__SOFTWARE_13_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_14 = IoReg_Read32(SOFTWARE__SOFTWARE_14_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_15 = IoReg_Read32(SOFTWARE__SOFTWARE_15_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_16 = IoReg_Read32(SOFTWARE__SOFTWARE_16_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_17 = IoReg_Read32(SOFTWARE__SOFTWARE_17_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_18 = IoReg_Read32(SOFTWARE__SOFTWARE_18_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_19 = IoReg_Read32(SOFTWARE__SOFTWARE_19_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_20 = IoReg_Read32(SOFTWARE__SOFTWARE_20_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_21 = IoReg_Read32(SOFTWARE__SOFTWARE_21_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_22 = IoReg_Read32(SOFTWARE__SOFTWARE_22_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_23 = IoReg_Read32(SOFTWARE__SOFTWARE_23_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_24 = IoReg_Read32(SOFTWARE__SOFTWARE_24_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_25 = IoReg_Read32(SOFTWARE__SOFTWARE_25_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_26 = IoReg_Read32(SOFTWARE__SOFTWARE_26_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_27 = IoReg_Read32(SOFTWARE__SOFTWARE_27_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_28 = IoReg_Read32(SOFTWARE__SOFTWARE_28_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_29 = IoReg_Read32(SOFTWARE__SOFTWARE_29_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_30 = IoReg_Read32(SOFTWARE__SOFTWARE_30_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_31 = IoReg_Read32(SOFTWARE__SOFTWARE_31_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_32 = IoReg_Read32(SOFTWARE__SOFTWARE_32_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_33 = IoReg_Read32(SOFTWARE__SOFTWARE_33_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_34 = IoReg_Read32(SOFTWARE__SOFTWARE_34_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_35 = IoReg_Read32(SOFTWARE__SOFTWARE_35_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_36 = IoReg_Read32(SOFTWARE__SOFTWARE_36_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_37 = IoReg_Read32(SOFTWARE__SOFTWARE_37_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_38 = IoReg_Read32(SOFTWARE__SOFTWARE_38_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_39 = IoReg_Read32(SOFTWARE__SOFTWARE_39_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_40 = IoReg_Read32(SOFTWARE__SOFTWARE_40_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_41 = IoReg_Read32(SOFTWARE__SOFTWARE_41_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_42 = IoReg_Read32(SOFTWARE__SOFTWARE_42_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_43 = IoReg_Read32(SOFTWARE__SOFTWARE_43_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_44 = IoReg_Read32(SOFTWARE__SOFTWARE_44_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_45 = IoReg_Read32(SOFTWARE__SOFTWARE_45_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_46 = IoReg_Read32(SOFTWARE__SOFTWARE_46_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_47 = IoReg_Read32(SOFTWARE__SOFTWARE_47_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_48 = IoReg_Read32(SOFTWARE__SOFTWARE_48_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_49 = IoReg_Read32(SOFTWARE__SOFTWARE_49_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_50 = IoReg_Read32(SOFTWARE__SOFTWARE_50_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_51 = IoReg_Read32(SOFTWARE__SOFTWARE_51_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_52 = IoReg_Read32(SOFTWARE__SOFTWARE_52_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_53 = IoReg_Read32(SOFTWARE__SOFTWARE_53_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_54 = IoReg_Read32(SOFTWARE__SOFTWARE_54_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_55 = IoReg_Read32(SOFTWARE__SOFTWARE_55_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_56 = IoReg_Read32(SOFTWARE__SOFTWARE_56_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_57 = IoReg_Read32(SOFTWARE__SOFTWARE_57_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_58 = IoReg_Read32(SOFTWARE__SOFTWARE_58_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_59 = IoReg_Read32(SOFTWARE__SOFTWARE_59_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_60 = IoReg_Read32(SOFTWARE__SOFTWARE_60_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_61 = IoReg_Read32(SOFTWARE__SOFTWARE_61_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_62 = IoReg_Read32(SOFTWARE__SOFTWARE_62_ADDR);
	vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_63 = IoReg_Read32(SOFTWARE__SOFTWARE_63_ADDR);
	//Software1
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_00 = IoReg_Read32(SOFTWARE1__SOFTWARE1_00_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_01 = IoReg_Read32(SOFTWARE1__SOFTWARE1_01_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_02 = IoReg_Read32(SOFTWARE1__SOFTWARE1_02_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_03 = IoReg_Read32(SOFTWARE1__SOFTWARE1_03_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_04 = IoReg_Read32(SOFTWARE1__SOFTWARE1_04_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_05 = IoReg_Read32(SOFTWARE1__SOFTWARE1_05_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_06 = IoReg_Read32(SOFTWARE1__SOFTWARE1_06_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_07 = IoReg_Read32(SOFTWARE1__SOFTWARE1_07_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_08 = IoReg_Read32(SOFTWARE1__SOFTWARE1_08_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_09 = IoReg_Read32(SOFTWARE1__SOFTWARE1_09_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_10 = IoReg_Read32(SOFTWARE1__SOFTWARE1_10_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_11 = IoReg_Read32(SOFTWARE1__SOFTWARE1_11_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_12 = IoReg_Read32(SOFTWARE1__SOFTWARE1_12_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_13 = IoReg_Read32(SOFTWARE1__SOFTWARE1_13_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_14 = IoReg_Read32(SOFTWARE1__SOFTWARE1_14_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_15 = IoReg_Read32(SOFTWARE1__SOFTWARE1_15_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_16 = IoReg_Read32(SOFTWARE1__SOFTWARE1_16_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_17 = IoReg_Read32(SOFTWARE1__SOFTWARE1_17_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_18 = IoReg_Read32(SOFTWARE1__SOFTWARE1_18_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_19 = IoReg_Read32(SOFTWARE1__SOFTWARE1_19_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_20 = IoReg_Read32(SOFTWARE1__SOFTWARE1_20_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_21 = IoReg_Read32(SOFTWARE1__SOFTWARE1_21_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_22 = IoReg_Read32(SOFTWARE1__SOFTWARE1_22_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_23 = IoReg_Read32(SOFTWARE1__SOFTWARE1_23_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_24 = IoReg_Read32(SOFTWARE1__SOFTWARE1_24_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_25 = IoReg_Read32(SOFTWARE1__SOFTWARE1_25_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_26 = IoReg_Read32(SOFTWARE1__SOFTWARE1_26_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_27 = IoReg_Read32(SOFTWARE1__SOFTWARE1_27_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_28 = IoReg_Read32(SOFTWARE1__SOFTWARE1_28_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_29 = IoReg_Read32(SOFTWARE1__SOFTWARE1_29_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_30 = IoReg_Read32(SOFTWARE1__SOFTWARE1_30_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_31 = IoReg_Read32(SOFTWARE1__SOFTWARE1_31_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_32 = IoReg_Read32(SOFTWARE1__SOFTWARE1_32_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_33 = IoReg_Read32(SOFTWARE1__SOFTWARE1_33_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_34 = IoReg_Read32(SOFTWARE1__SOFTWARE1_34_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_35 = IoReg_Read32(SOFTWARE1__SOFTWARE1_35_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_36 = IoReg_Read32(SOFTWARE1__SOFTWARE1_36_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_37 = IoReg_Read32(SOFTWARE1__SOFTWARE1_37_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_38 = IoReg_Read32(SOFTWARE1__SOFTWARE1_38_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_39 = IoReg_Read32(SOFTWARE1__SOFTWARE1_39_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_40 = IoReg_Read32(SOFTWARE1__SOFTWARE1_40_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_41 = IoReg_Read32(SOFTWARE1__SOFTWARE1_41_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_42 = IoReg_Read32(SOFTWARE1__SOFTWARE1_42_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_43 = IoReg_Read32(SOFTWARE1__SOFTWARE1_43_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_44 = IoReg_Read32(SOFTWARE1__SOFTWARE1_44_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_45 = IoReg_Read32(SOFTWARE1__SOFTWARE1_45_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_46 = IoReg_Read32(SOFTWARE1__SOFTWARE1_46_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_47 = IoReg_Read32(SOFTWARE1__SOFTWARE1_47_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_48 = IoReg_Read32(SOFTWARE1__SOFTWARE1_48_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_49 = IoReg_Read32(SOFTWARE1__SOFTWARE1_49_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_50 = IoReg_Read32(SOFTWARE1__SOFTWARE1_50_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_51 = IoReg_Read32(SOFTWARE1__SOFTWARE1_51_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_52 = IoReg_Read32(SOFTWARE1__SOFTWARE1_52_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_53 = IoReg_Read32(SOFTWARE1__SOFTWARE1_53_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_54 = IoReg_Read32(SOFTWARE1__SOFTWARE1_54_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_55 = IoReg_Read32(SOFTWARE1__SOFTWARE1_55_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_56 = IoReg_Read32(SOFTWARE1__SOFTWARE1_56_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_57 = IoReg_Read32(SOFTWARE1__SOFTWARE1_57_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_58 = IoReg_Read32(SOFTWARE1__SOFTWARE1_58_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_59 = IoReg_Read32(SOFTWARE1__SOFTWARE1_59_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_60 = IoReg_Read32(SOFTWARE1__SOFTWARE1_60_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_61 = IoReg_Read32(SOFTWARE1__SOFTWARE1_61_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_62 = IoReg_Read32(SOFTWARE1__SOFTWARE1_62_ADDR);
	vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_63 = IoReg_Read32(SOFTWARE1__SOFTWARE1_63_ADDR);

	//MEMC mux
	vpq_memc_suspend_resume.MEMC_MUX_MEMC_MUX_FUNC_CTRL_SUSPEND = IoReg_Read32(MEMC_MUX_MEMC_MUX_FUNC_CTRL_reg);
	//FBG
	vpq_memc_suspend_resume.PPOVERLAY_MEMC_MUX_CTRL_SUSPEND = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg);
	//clock
	vpq_memc_suspend_resume.SYS_REG_SYS_DISPCLKSEL_SUSPEND = IoReg_Read32(SYS_REG_SYS_DISPCLKSEL_reg);
	vpq_memc_suspend_resume.MEMC_LATENCY01 = IoReg_Read32(MEMC_LATENCY01_ADDR);
	vpq_memc_suspend_resume.MEMC_LATENCY02 = IoReg_Read32(MEMC_LATENCY02_ADDR);
	//ME memory boundary
	vpq_memc_suspend_resume.MEMC_DBUS_WRAP_ME_DOWNLIMIT = IoReg_Read32(MEMC_DBUS_WRAP_Me_downlimit_addr_reg);
	vpq_memc_suspend_resume.MEMC_DBUS_WRAP_ME_UPLIMIT = IoReg_Read32(MEMC_DBUS_WRAP_Me_uplimit_addr_reg);
	//MC memory boundary
	vpq_memc_suspend_resume.MEMC_DBUS_WRAP_MC_DOWNLIMIT = IoReg_Read32(MEMC_DBUS_WRAP_Mc_downlimit_addr_reg);
	vpq_memc_suspend_resume.MEMC_DBUS_WRAP_MC_UPLIMIT = IoReg_Read32(MEMC_DBUS_WRAP_Mc_uplimit_addr_reg);

	//MC DMA disable
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_AGENT = vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_AGENT = vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_02_AGENT = vpq_memc_suspend_resume.KMC_DM_TOP__KMC_02_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_03_AGENT = vpq_memc_suspend_resume.KMC_DM_TOP__KMC_03_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_04_AGENT = vpq_memc_suspend_resume.KMC_DM_TOP__KMC_04_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_05_AGENT = vpq_memc_suspend_resume.KMC_DM_TOP__KMC_05_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_06_AGENT = vpq_memc_suspend_resume.KMC_DM_TOP__KMC_06_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KMC_DM_TOP__KMC_07_AGENT = vpq_memc_suspend_resume.KMC_DM_TOP__KMC_07_AGENT & 0xffefffff;

	//MV DMA disable
	vpq_memc_suspend_resume.KME_DM_TOP2__MV01_AGENT = vpq_memc_suspend_resume.KME_DM_TOP2__MV01_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KME_DM_TOP2__MV02_AGENT = vpq_memc_suspend_resume.KME_DM_TOP2__MV02_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KME_DM_TOP2__MV03_AGENT = vpq_memc_suspend_resume.KME_DM_TOP2__MV03_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KME_DM_TOP2__MV04_AGENT = vpq_memc_suspend_resume.KME_DM_TOP2__MV04_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KME_DM_TOP2__MV05_AGENT = vpq_memc_suspend_resume.KME_DM_TOP2__MV05_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KME_DM_TOP2__MV06_AGENT = vpq_memc_suspend_resume.KME_DM_TOP2__MV06_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KME_DM_TOP2__MV07_AGENT = vpq_memc_suspend_resume.KME_DM_TOP2__MV07_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KME_DM_TOP2__MV08_AGENT = vpq_memc_suspend_resume.KME_DM_TOP2__MV08_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KME_DM_TOP2__MV09_AGENT = vpq_memc_suspend_resume.KME_DM_TOP2__MV09_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KME_DM_TOP2__MV10_AGENT = vpq_memc_suspend_resume.KME_DM_TOP2__MV10_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KME_DM_TOP2__MV11_AGENT = vpq_memc_suspend_resume.KME_DM_TOP2__MV11_AGENT & 0xffefffff;

	//ME DMA disable
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_AGENT = vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_01_AGENT = vpq_memc_suspend_resume.KME_DM_TOP0__KME_01_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_02_AGENT = vpq_memc_suspend_resume.KME_DM_TOP0__KME_02_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_03_AGENT = vpq_memc_suspend_resume.KME_DM_TOP0__KME_03_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_04_AGENT = vpq_memc_suspend_resume.KME_DM_TOP0__KME_04_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_05_AGENT = vpq_memc_suspend_resume.KME_DM_TOP0__KME_05_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KME_DM_TOP0__KME06AGENT = vpq_memc_suspend_resume.KME_DM_TOP0__KME06AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KME_DM_TOP0__KME_07_AGENT = vpq_memc_suspend_resume.KME_DM_TOP0__KME_07_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_08_AGENT = vpq_memc_suspend_resume.KME_DM_TOP1__KME_08_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_09_AGENT = vpq_memc_suspend_resume.KME_DM_TOP1__KME_09_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_10_AGENT = vpq_memc_suspend_resume.KME_DM_TOP1__KME_10_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_11_AGENT = vpq_memc_suspend_resume.KME_DM_TOP1__KME_11_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_12_AGENT = vpq_memc_suspend_resume.KME_DM_TOP1__KME_12_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_13_AGENT = vpq_memc_suspend_resume.KME_DM_TOP1__KME_13_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_14_AGENT = vpq_memc_suspend_resume.KME_DM_TOP1__KME_14_AGENT & 0xffefffff;
	vpq_memc_suspend_resume.KME_DM_TOP1__KME_15_AGENT = vpq_memc_suspend_resume.KME_DM_TOP1__KME_15_AGENT & 0xffefffff;
#endif 	//CONFIG_HW_SUPPORT_MEMC
}

void vpq_memc_disp_resume_instanboot(void){
#ifdef CONFIG_HW_SUPPORT_MEMC
	rtd_pr_memc_notice( "%s %d\n",__func__,__LINE__);
#ifdef CONFIG_MEMC_BYPASS
		return;
#endif

	if(Scaler_MEMC_CLK_Check() == FALSE){
		return;
	}


#if 0//BISR, BIST will be checked
		//KME BISR
		unsigned int iMaxCount_kme = 100;
		unsigned rstn_bits_kme = IP_rstn_bits_kme;
		unsigned done_bits_kme = IP_done_bits_kme;
		unsigned remap_bits_kme = IP_remap_bits_kme;
		unsigned repair_check_bits_kme = IP_repair_check_bits_kme;
		unsigned fail_bits_kme = IP_fail_bits_kme;

		rtd_pr_memc_debug("\n[MEMC] KME: BISR testing ....\n");
		rtd_maskl(SYS_REG_SYS_SRST3_reg, ~(_BIT29), (_BIT29));	//enable RBUS clk of MEMC reset reg
		rtd_maskl(SYS_REG_SYS_CLKEN3_reg, ~(_BIT29), (_BIT29));   //enable RBUS clk of MEMC
		rtd_maskl(SYS_REG_SYS_CLKEN3_reg, ~(_BIT31|_BIT30), 0);   //disable CLKEN_MEMC/CLKEN_MEME_ME
		rtd_maskl(SYS_REG_SYS_SRST3_reg, ~(_BIT31|_BIT30), 0);//reset of memc
		rtd_maskl(SYS_REG_SYS_SRST3_reg, ~(_BIT31|_BIT30), (_BIT31|_BIT30));//release reset of memc
		rtd_maskl(SYS_REG_SYS_CLKEN3_reg, ~(_BIT31|_BIT30), (_BIT31|_BIT30));	  //enable CLKEN_MEMC/CLKEN_MEME_ME
		rtd_maskl(SYS_REG_SYS_DISPCLKSEL_reg, ~(_BIT27|_BIT26), 0);   //dispd-clock div 2

		rtd_setbits(bisr_rstn_addr_kme, rstn_bits_kme);
		udelay(100);
		while((rtd_inl(bisr_done_addr_kme) & done_bits_kme) != done_bits_kme) {
			iMaxCount_kme--;
			udelay(100);
			if( iMaxCount_kme == 0 ) {
				break;
			}
		}
		rtd_setbits(bisr_remap_addr_kme, remap_bits_kme);

		if((rtd_inl(bisr_repair_addr_kme) & repair_check_bits_kme) != 0) {
			rtd_pr_memc_debug("[MEMC]KME: Need to repair\n");
		}


		if((rtd_inl(bisr_fail_addr_kme) & fail_bits_kme) != 0) {
			rtd_pr_memc_debug("[MEMC]KME: BISR fail!\n");
		}

		rtd_pr_memc_debug("[MEMC]Finish ME BISR testing %s\n\n", (iMaxCount_kme ? "done" : "fail" ) );

		//KMC BISR
		unsigned int iMaxCount_kmc = 100;
		unsigned rstn_bits_kmc = IP_rstn_bits_kmc;
		unsigned done_bits_kmc = IP_done_bits_kmc;
		unsigned remap_bits_kmc = IP_remap_bits_kmc;
		unsigned repair_check_bits_kmc = IP_repair_check_bits_kmc;
		unsigned fail_bits_kmc = IP_fail_bits_kmc;

		rtd_pr_memc_debug("\n[MEMC]KMC: BISR testing ....\n");

		rtd_setbits(bisr_rstn_addr_kmc, rstn_bits_kmc);
		udelay(100);
		while((rtd_inl(bisr_done_addr_kmc) & done_bits_kmc) != done_bits_kmc) {
			iMaxCount_kmc--;
			udelay(100);
			if( iMaxCount_kmc == 0 ) {
				break;
			}
		}
		rtd_setbits(bisr_remap_addr_kmc, remap_bits_kmc);

		if((rtd_inl(bisr_repair_addr_kmc) & repair_check_bits_kmc) != 0) {
			rtd_pr_memc_debug("[MEMC]KMC: Need to repair\n");
		}


		if((rtd_inl(bisr_fail_addr_kmc) & fail_bits_kmc) != 0) {
			rtd_pr_memc_debug("[MEMC]KMC: BISR fail!\n");
		}

		rtd_pr_memc_debug("[MEMC]Finish MC BISR testing %s\n\n", (iMaxCount_kmc ? "done" : "fail" ) );

	rtd_maskl(SYS_REG_SYS_SRST3_reg, ~(_BIT29), (_BIT29));	//enable RBUS clk of MEMC reset reg
	rtd_maskl(SYS_REG_SYS_CLKEN3_reg, ~(_BIT29), (_BIT29));   //enable RBUS clk of MEMC
	rtd_maskl(SYS_REG_SYS_CLKEN3_reg, ~(_BIT31|_BIT30), 0);   //disable CLKEN_MEMC/CLKEN_MEME_ME
	rtd_maskl(SYS_REG_SYS_SRST3_reg, ~(_BIT31|_BIT30), 0);//reset of memc
	rtd_maskl(SYS_REG_SYS_SRST3_reg, ~(_BIT31|_BIT30), (_BIT31|_BIT30));//release reset of memc
	rtd_maskl(SYS_REG_SYS_CLKEN3_reg, ~(_BIT31|_BIT30), (_BIT31|_BIT30));	  //enable CLKEN_MEMC/CLKEN_MEME_ME
	rtd_maskl(SYS_REG_SYS_DISPCLKSEL_reg, ~(_BIT27|_BIT26), 0);   //dispd-clock div 2
#endif
	//kmc_top
	IoReg_Write32(KMC_TOP__KMC_TOP_00_ADDR , vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_00);
	IoReg_Write32(KMC_TOP__KMC_TOP_04_ADDR , vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_04);
	IoReg_Write32(KMC_TOP__KMC_TOP_08_ADDR , vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_08);
	IoReg_Write32(KMC_TOP__KMC_TOP_0C_ADDR , vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_0C);
	IoReg_Write32(KMC_TOP__KMC_TOP_10_ADDR , vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_10);
	IoReg_Write32(KMC_TOP__KMC_TOP_14_ADDR , vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_14);
	IoReg_Write32(KMC_TOP__KMC_TOP_18_ADDR , vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_18);
	IoReg_Write32(KMC_TOP__KMC_TOP_1C_ADDR , vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_1C);
	IoReg_Write32(KMC_TOP__KMC_TOP_20_ADDR , vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_20);
	IoReg_Write32(KMC_TOP__KMC_TOP_24_ADDR , vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_24);
	IoReg_Write32(KMC_TOP__KMC_TOP_28_ADDR , vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_28);
	IoReg_Write32(KMC_TOP__KMC_TOP_2C_ADDR , vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_2C);
	IoReg_Write32(KMC_TOP__KMC_TOP_30_ADDR , vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_30);
	IoReg_Write32(KMC_TOP__KMC_TOP_34_ADDR , vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_34);
	IoReg_Write32(KMC_TOP__KMC_TOP_38_ADDR , vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_38);
	IoReg_Write32(KMC_TOP__KMC_TOP_3C_ADDR , vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_3C);
	IoReg_Write32(KMC_TOP__KMC_TOP_40_ADDR , vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_40);
	IoReg_Write32(KMC_TOP__KMC_TOP_44_ADDR , vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_44);
	IoReg_Write32(KMC_TOP__KMC_TOP_48_ADDR , vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_48);
	IoReg_Write32(KMC_TOP__KMC_TOP_4C_ADDR , vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_4C);
	IoReg_Write32(KMC_TOP__KMC_TOP_50_ADDR , vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_50);
	IoReg_Write32(KMC_TOP__KMC_TOP_54_ADDR , vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_54);
	IoReg_Write32(KMC_TOP__KMC_TOP_58_ADDR , vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_58);
	IoReg_Write32(KMC_TOP__KMC_TOP_5C_ADDR , vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_5C);
	IoReg_Write32(KMC_TOP__KMC_TOP_60_ADDR , vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_60);
	IoReg_Write32(KMC_TOP__KMC_TOP_64_ADDR , vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_64);
	IoReg_Write32(KMC_TOP__KMC_TOP_68_ADDR , vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_68);
	IoReg_Write32(KMC_TOP__KMC_TOP_6C_ADDR , vpq_memc_suspend_resume.KMC_TOP__KMC_TOP_6C);
	//IPPRE
	IoReg_Write32(IPPRE__IPPRE_00_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_00);
	IoReg_Write32(IPPRE__IPPRE_04_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_04);
	IoReg_Write32(IPPRE__IPPRE_08_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_08);
	IoReg_Write32(IPPRE__IPPRE_0C_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_0C);
	IoReg_Write32(IPPRE__IPPRE_10_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_10);
	IoReg_Write32(IPPRE__IPPRE_14_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_14);
	IoReg_Write32(IPPRE__IPPRE_18_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_18);
	IoReg_Write32(IPPRE__IPPRE_1C_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_1C);
	IoReg_Write32(IPPRE__IPPRE_20_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_20);
	IoReg_Write32(IPPRE__IPPRE_24_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_24);
	IoReg_Write32(IPPRE__IPPRE_28_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_28);
	IoReg_Write32(IPPRE__IPPRE_2C_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_2C);
	IoReg_Write32(IPPRE__IPPRE_30_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_30);
	IoReg_Write32(IPPRE__IPPRE_34_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_34);
	IoReg_Write32(IPPRE__IPPRE_38_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_38);
	IoReg_Write32(IPPRE__IPPRE_3C_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_3C);
	IoReg_Write32(IPPRE__IPPRE_40_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_40);
	IoReg_Write32(IPPRE__IPPRE_44_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_44);
	IoReg_Write32(IPPRE__IPPRE_48_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_48);
	IoReg_Write32(IPPRE__IPPRE_4C_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_4C);
	IoReg_Write32(IPPRE__IPPRE_50_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_50);
	IoReg_Write32(IPPRE__IPPRE_54_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_54);
	IoReg_Write32(IPPRE__IPPRE_58_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_58);
	IoReg_Write32(IPPRE__IPPRE_5C_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_5C);
	IoReg_Write32(IPPRE__IPPRE_60_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_60);
	IoReg_Write32(IPPRE__IPPRE_64_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_64);
	IoReg_Write32(IPPRE__IPPRE_68_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_68);
	IoReg_Write32(IPPRE__IPPRE_6C_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_6C);
	IoReg_Write32(IPPRE__IPPRE_70_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_70);
	IoReg_Write32(IPPRE__IPPRE_74_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_74);
	IoReg_Write32(IPPRE__IPPRE_78_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_78);
	IoReg_Write32(IPPRE__IPPRE_7C_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_7C);
	IoReg_Write32(IPPRE__IPPRE_80_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_80);
	IoReg_Write32(IPPRE__IPPRE_84_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_84);
	IoReg_Write32(IPPRE__IPPRE_88_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_88);
	IoReg_Write32(IPPRE__IPPRE_8C_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_8C);
	IoReg_Write32(IPPRE__IPPRE_90_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_90);
	IoReg_Write32(IPPRE__IPPRE_94_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_94);
	IoReg_Write32(IPPRE__IPPRE_98_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_98);
	IoReg_Write32(IPPRE__IPPRE_9C_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_9C);
	IoReg_Write32(IPPRE__IPPRE_A0_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_A0);
	IoReg_Write32(IPPRE__IPPRE_A4_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_A4);
	IoReg_Write32(IPPRE__IPPRE_A8_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_A8);
	IoReg_Write32(IPPRE__IPPRE_AC_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_AC);
	IoReg_Write32(IPPRE__IPPRE_B0_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_B0);
	IoReg_Write32(IPPRE__IPPRE_B4_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_B4);
	IoReg_Write32(IPPRE__IPPRE_B8_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_B8);
	IoReg_Write32(IPPRE__IPPRE_BC_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_BC);
	IoReg_Write32(IPPRE__IPPRE_C0_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_C0);
	IoReg_Write32(IPPRE__IPPRE_C4_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_C4);
	IoReg_Write32(IPPRE__IPPRE_C8_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_C8);
	IoReg_Write32(IPPRE__IPPRE_CC_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_CC);
	IoReg_Write32(IPPRE__IPPRE_D0_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_D0);
	IoReg_Write32(IPPRE__IPPRE_D4_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_D4);
	IoReg_Write32(IPPRE__IPPRE_D8_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_D8);
	IoReg_Write32(IPPRE__IPPRE_DC_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_DC);
	IoReg_Write32(IPPRE__IPPRE_E0_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_E0);
	IoReg_Write32(IPPRE__IPPRE_E4_ADDR , vpq_memc_suspend_resume.IPPRE__IPPRE_E4);
	//KMC_CPR_TOP0
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_00_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_00);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_04_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_04);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_08_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_08);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_0C_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_0C);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_10_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_10);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_14_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_14);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_18_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_18);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_1C_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_1C);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_20_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_20);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_24_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_24);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_28_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_28);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_2C_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_2C);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_30_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_30);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_34_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_34);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_38_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_38);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_3C_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_3C);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_40_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_40);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_44_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_44);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_48_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_48);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_4C_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_4C);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_50_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_50);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_54_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_54);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_58_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_58);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_5C_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_5C);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_60_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_60);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_64_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_64);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_68_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_68);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_6C_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_6C);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_70_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_70);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_74_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_74);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_78_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_78);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_7C_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_7C);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_80_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_80);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_84_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_84);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_88_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_88);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_8C_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_8C);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_90_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_90);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_94_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_94);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_98_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_98);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_9C_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_9C);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_A0_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_A0);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_A4_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_A4);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_A8_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_A8);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_AC_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_AC);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_B0_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_B0);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_B4_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_B4);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_B8_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_B8);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_BC_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_BC);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_C0_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_C0);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_C4_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_C4);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_C8_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_C8);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_CC_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_CC);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_D0_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_D0);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_D4_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_D4);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_D8_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_D8);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_DC_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_DC);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_E0_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_E0);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_E4_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_E4);
	IoReg_Write32(KMC_CPR_TOP0__KMC_CPR_TOP0_E8_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP0__KMC_CPR_TOP0_E8);
	//KMC_CPR_TOP1
	IoReg_Write32(KMC_CPR_TOP1__PATT_CTRL0_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP1__PATT_CTRL0);
	IoReg_Write32(KMC_CPR_TOP1__PATT_LF_I_POSITION0_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP1__PATT_LF_I_POSITION0);
	IoReg_Write32(KMC_CPR_TOP1__PATT_LF_I_POSITION1_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP1__PATT_LF_I_POSITION1);
	IoReg_Write32(KMC_CPR_TOP1__PATT_HF_I_POSITION0_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP1__PATT_HF_I_POSITION0);
	IoReg_Write32(KMC_CPR_TOP1__PATT_HF_I_POSITION1_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP1__PATT_HF_I_POSITION1);
	IoReg_Write32(KMC_CPR_TOP1__CPR_PATT_LF_I_SETTING0_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP1__CPR_PATT_LF_I_SETTING0);
	IoReg_Write32(KMC_CPR_TOP1__CPR_PATT_LF_I_SETTING1_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP1__CPR_PATT_LF_I_SETTING1);
	IoReg_Write32(KMC_CPR_TOP1__CPR_PATT_HF_I_SETTING0_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP1__CPR_PATT_HF_I_SETTING0);
	IoReg_Write32(KMC_CPR_TOP1__CPR_PATT_HF_I_SETTING1_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP1__CPR_PATT_HF_I_SETTING1);
	IoReg_Write32(KMC_CPR_TOP1__PATT_LF_P_POSITION0_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP1__PATT_LF_P_POSITION0);
	IoReg_Write32(KMC_CPR_TOP1__PATT_LF_P_POSITION1_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP1__PATT_LF_P_POSITION1);
	IoReg_Write32(KMC_CPR_TOP1__PATT_HF_P_POSITION0_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP1__PATT_HF_P_POSITION0);
	IoReg_Write32(KMC_CPR_TOP1__PATT_HF_P_POSITION1_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP1__PATT_HF_P_POSITION1);
	IoReg_Write32(KMC_CPR_TOP1__CPR_PATT_LF_P_SETTING0_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP1__CPR_PATT_LF_P_SETTING0);
	IoReg_Write32(KMC_CPR_TOP1__CPR_PATT_LF_P_SETTING1_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP1__CPR_PATT_LF_P_SETTING1);
	IoReg_Write32(KMC_CPR_TOP1__CPR_PATT_HF_P_SETTING0_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP1__CPR_PATT_HF_P_SETTING0);
	IoReg_Write32(KMC_CPR_TOP1__CPR_PATT_HF_P_SETTING1_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP1__CPR_PATT_HF_P_SETTING1);
	IoReg_Write32(KMC_CPR_TOP1__PATT_LF_NR_POSITION0_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP1__PATT_LF_NR_POSITION0);
	IoReg_Write32(KMC_CPR_TOP1__PATT_LF_NR_POSITION1_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP1__PATT_LF_NR_POSITION1);
	IoReg_Write32(KMC_CPR_TOP1__PATT_HF_NR_POSITION_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP1__PATT_HF_NR_POSITION);
	IoReg_Write32(KMC_CPR_TOP1__PATT_HF_NR_POSITION1_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP1__PATT_HF_NR_POSITION1);
	IoReg_Write32(KMC_CPR_TOP1__CPR_PATT_LF_NR_SETTING0_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP1__CPR_PATT_LF_NR_SETTING0);
	IoReg_Write32(KMC_CPR_TOP1__CPR_PATT_LF_NR_SETTING1_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP1__CPR_PATT_LF_NR_SETTING1);
	IoReg_Write32(KMC_CPR_TOP1__CPR_PATT_HF_NR_SETTING0_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP1__CPR_PATT_HF_NR_SETTING0);
	IoReg_Write32(KMC_CPR_TOP1__CPR_PATT_HF_NR_SETTING1_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP1__CPR_PATT_HF_NR_SETTING1);
	IoReg_Write32(KMC_CPR_TOP1__CPR_DEBUG_CFG0_ADDR , vpq_memc_suspend_resume.KMC_CPR_TOP1__CPR_DEBUG_CFG0);
	//KMC_DM_TOP
	IoReg_Write32(KMC_DM_TOP__KMC_00_START_ADDRESS0_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_START_ADDRESS0);
	IoReg_Write32(KMC_DM_TOP__KMC_00_START_ADDRESS1_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_START_ADDRESS1);
	IoReg_Write32(KMC_DM_TOP__KMC_00_START_ADDRESS2_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_START_ADDRESS2);
	IoReg_Write32(KMC_DM_TOP__KMC_00_START_ADDRESS3_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_START_ADDRESS3);
	IoReg_Write32(KMC_DM_TOP__KMC_00_START_ADDRESS4_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_START_ADDRESS4);
	IoReg_Write32(KMC_DM_TOP__KMC_00_START_ADDRESS5_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_START_ADDRESS5);
	IoReg_Write32(KMC_DM_TOP__KMC_00_END_ADDRESS0_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_END_ADDRESS0);
	IoReg_Write32(KMC_DM_TOP__KMC_00_END_ADDRESS1_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_END_ADDRESS1);
	IoReg_Write32(KMC_DM_TOP__KMC_00_END_ADDRESS2_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_END_ADDRESS2);
	IoReg_Write32(KMC_DM_TOP__KMC_00_END_ADDRESS3_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_END_ADDRESS3);
	IoReg_Write32(KMC_DM_TOP__KMC_00_END_ADDRESS4_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_END_ADDRESS4);
	IoReg_Write32(KMC_DM_TOP__KMC_00_END_ADDRESS5_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_END_ADDRESS5);
	IoReg_Write32(KMC_DM_TOP__KMC_00_LR_OFFSET_ADDR_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_LR_OFFSET_ADDR);
	IoReg_Write32(KMC_DM_TOP__KMC_00_LINE_OFFSET_ADDR_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_LINE_OFFSET_ADDR);
	IoReg_Write32(KMC_DM_TOP__KMC_00_AGENT_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_AGENT);
	IoReg_Write32(KMC_DM_TOP__KMC_00_THRESHOLD_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_THRESHOLD);
	IoReg_Write32(KMC_DM_TOP__KMC_01_START_ADDRESS0_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_START_ADDRESS0);
	IoReg_Write32(KMC_DM_TOP__KMC_01_START_ADDRESS1_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_START_ADDRESS1);
	IoReg_Write32(KMC_DM_TOP__KMC_01_START_ADDRESS2_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_START_ADDRESS2);
	IoReg_Write32(KMC_DM_TOP__KMC_01_START_ADDRESS3_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_START_ADDRESS3);
	IoReg_Write32(KMC_DM_TOP__KMC_01_START_ADDRESS4_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_START_ADDRESS4);
	IoReg_Write32(KMC_DM_TOP__KMC_01_START_ADDRESS5_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_START_ADDRESS5);
	IoReg_Write32(KMC_DM_TOP__KMC_01_END_ADDRESS0_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_END_ADDRESS0);
	IoReg_Write32(KMC_DM_TOP__KMC_01_END_ADDRESS1_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_END_ADDRESS1);
	IoReg_Write32(KMC_DM_TOP__KMC_01_END_ADDRESS2_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_END_ADDRESS2);
	IoReg_Write32(KMC_DM_TOP__KMC_01_END_ADDRESS3_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_END_ADDRESS3);
	IoReg_Write32(KMC_DM_TOP__KMC_01_END_ADDRESS4_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_END_ADDRESS4);
	IoReg_Write32(KMC_DM_TOP__KMC_01_END_ADDRESS5_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_END_ADDRESS5);
	IoReg_Write32(KMC_DM_TOP__KMC_01_LR_OFFSET_ADDR_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_LR_OFFSET_ADDR);
	IoReg_Write32(KMC_DM_TOP__KMC_01_LINE_OFFSET_ADDR_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_LINE_OFFSET_ADDR);
	IoReg_Write32(KMC_DM_TOP__KMC_01_AGENT_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_AGENT);
	IoReg_Write32(KMC_DM_TOP__KMC_01_THRESHOLD_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_THRESHOLD);
	IoReg_Write32(KMC_DM_TOP__KMC_02_AGENT_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_02_AGENT);
	IoReg_Write32(KMC_DM_TOP__KMC_02_THRESHOLD_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_02_THRESHOLD);
	IoReg_Write32(KMC_DM_TOP__KMC_03_AGENT_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_03_AGENT);
	IoReg_Write32(KMC_DM_TOP__KMC_03_THRESHOLD_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_03_THRESHOLD);
	IoReg_Write32(KMC_DM_TOP__KMC_04_AGENT_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_04_AGENT);
	IoReg_Write32(KMC_DM_TOP__KMC_04_THRESHOLD_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_04_THRESHOLD);
	IoReg_Write32(KMC_DM_TOP__KMC_05_AGENT_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_05_AGENT);
	IoReg_Write32(KMC_DM_TOP__KMC_05_THRESHOLD_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_05_THRESHOLD);
	IoReg_Write32(KMC_DM_TOP__KMC_06_AGENT_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_06_AGENT);
	IoReg_Write32(KMC_DM_TOP__KMC_06_THRESHOLD_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_06_THRESHOLD);
	IoReg_Write32(KMC_DM_TOP__KMC_07_AGENT_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_07_AGENT);
	IoReg_Write32(KMC_DM_TOP__KMC_07_THRESHOLD_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_07_THRESHOLD);
	IoReg_Write32(KMC_DM_TOP__KMC_VTOTAL_AND_TIMER_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_VTOTAL_AND_TIMER);
	IoReg_Write32(KMC_DM_TOP__KMC_HOLD_TIME_G0_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_HOLD_TIME_G0);
	IoReg_Write32(KMC_DM_TOP__KMC_HOLD_TIME_G1_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_HOLD_TIME_G1);
	IoReg_Write32(KMC_DM_TOP__KMC_SOURCE_MODE_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_SOURCE_MODE);
	IoReg_Write32(KMC_DM_TOP__KMC_DEBUG_HEAD_G0_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_DEBUG_HEAD_G0);
	IoReg_Write32(KMC_DM_TOP__KMC_DEBUG_HEAD_G1_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_DEBUG_HEAD_G1);
	IoReg_Write32(KMC_DM_TOP__KMC_DEBUG_HEAD_G2_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_DEBUG_HEAD_G2);
	IoReg_Write32(KMC_DM_TOP__KMC_DEBUG_HEAD_G3_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_DEBUG_HEAD_G3);
	IoReg_Write32(KMC_DM_TOP__KMC_00_START_ADDRESS6_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_START_ADDRESS6);
	IoReg_Write32(KMC_DM_TOP__KMC_00_END_ADDRESS6_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_END_ADDRESS6);
	IoReg_Write32(KMC_DM_TOP__KMC_00_START_ADDRESS7_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_START_ADDRESS7);
	IoReg_Write32(KMC_DM_TOP__KMC_00_END_ADDRESS7_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_00_END_ADDRESS7);
	IoReg_Write32(KMC_DM_TOP__KMC_01_START_ADDRESS6_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_START_ADDRESS6);
	IoReg_Write32(KMC_DM_TOP__KMC_01_END_ADDRESS6_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_END_ADDRESS6);
	IoReg_Write32(KMC_DM_TOP__KMC_01_START_ADDRESS7_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_START_ADDRESS7);
	IoReg_Write32(KMC_DM_TOP__KMC_01_END_ADDRESS7_ADDR , vpq_memc_suspend_resume.KMC_DM_TOP__KMC_01_END_ADDRESS7);
	//IPPRE1
	IoReg_Write32(IPPRE1__IPPRE1_00_ADDR , vpq_memc_suspend_resume.IPPRE1__IPPRE1_00);
	IoReg_Write32(IPPRE1__IPPRE1_04_ADDR , vpq_memc_suspend_resume.IPPRE1__IPPRE1_04);
	//MC
	IoReg_Write32(MC__MC_0C_ADDR , vpq_memc_suspend_resume.MC__MC_0C);
	IoReg_Write32(MC__MC_10_ADDR , vpq_memc_suspend_resume.MC__MC_10);
	IoReg_Write32(MC__MC_14_ADDR , vpq_memc_suspend_resume.MC__MC_14);
	IoReg_Write32(MC__MC_18_ADDR , vpq_memc_suspend_resume.MC__MC_18);
	IoReg_Write32(MC__MC_1C_ADDR , vpq_memc_suspend_resume.MC__MC_1C);
	IoReg_Write32(MC__MC_20_ADDR , vpq_memc_suspend_resume.MC__MC_20);
	IoReg_Write32(MC__MC_24_ADDR , vpq_memc_suspend_resume.MC__MC_24);
	IoReg_Write32(MC__MC_28_ADDR , vpq_memc_suspend_resume.MC__MC_28);
	IoReg_Write32(MC__MC_2C_ADDR , vpq_memc_suspend_resume.MC__MC_2C);
	IoReg_Write32(MC__MC_30_ADDR , vpq_memc_suspend_resume.MC__MC_30);
	IoReg_Write32(MC__MC_34_ADDR , vpq_memc_suspend_resume.MC__MC_34);
	IoReg_Write32(MC__MC_38_ADDR , vpq_memc_suspend_resume.MC__MC_38);
	IoReg_Write32(MC__MC_3C_ADDR , vpq_memc_suspend_resume.MC__MC_3C);
	IoReg_Write32(MC__MC_40_ADDR , vpq_memc_suspend_resume.MC__MC_40);
	IoReg_Write32(MC__MC_44_ADDR , vpq_memc_suspend_resume.MC__MC_44);
	IoReg_Write32(MC__MC_48_ADDR , vpq_memc_suspend_resume.MC__MC_48);
	IoReg_Write32(MC__MC_4C_ADDR , vpq_memc_suspend_resume.MC__MC_4C);
	IoReg_Write32(MC__MC_50_ADDR , vpq_memc_suspend_resume.MC__MC_50);
	IoReg_Write32(MC__MC_54_ADDR , vpq_memc_suspend_resume.MC__MC_54);
	IoReg_Write32(MC__MC_58_ADDR , vpq_memc_suspend_resume.MC__MC_58);
	IoReg_Write32(MC__MC_5C_ADDR , vpq_memc_suspend_resume.MC__MC_5C);
	IoReg_Write32(MC__MC_60_ADDR , vpq_memc_suspend_resume.MC__MC_60);
	IoReg_Write32(MC__MC_64_ADDR , vpq_memc_suspend_resume.MC__MC_64);
	IoReg_Write32(MC__MC_68_ADDR , vpq_memc_suspend_resume.MC__MC_68);
	IoReg_Write32(MC__MC_6C_ADDR , vpq_memc_suspend_resume.MC__MC_6C);
	IoReg_Write32(MC__MC_70_ADDR , vpq_memc_suspend_resume.MC__MC_70);
	IoReg_Write32(MC__MC_74_ADDR , vpq_memc_suspend_resume.MC__MC_74);
	IoReg_Write32(MC__MC_78_ADDR , vpq_memc_suspend_resume.MC__MC_78);
	IoReg_Write32(MC__MC_7C_ADDR , vpq_memc_suspend_resume.MC__MC_7C);
	IoReg_Write32(MC__MC_80_ADDR , vpq_memc_suspend_resume.MC__MC_80);
	IoReg_Write32(MC__MC_84_ADDR , vpq_memc_suspend_resume.MC__MC_84);
	IoReg_Write32(MC__MC_88_ADDR , vpq_memc_suspend_resume.MC__MC_88);
	IoReg_Write32(MC__MC_8C_ADDR , vpq_memc_suspend_resume.MC__MC_8C);
	IoReg_Write32(MC__MC_90_ADDR , vpq_memc_suspend_resume.MC__MC_90);
	IoReg_Write32(MC__MC_94_ADDR , vpq_memc_suspend_resume.MC__MC_94);
	IoReg_Write32(MC__MC_98_ADDR , vpq_memc_suspend_resume.MC__MC_98);
	IoReg_Write32(MC__MC_9C_ADDR , vpq_memc_suspend_resume.MC__MC_9C);
	IoReg_Write32(MC__MC_A0_ADDR , vpq_memc_suspend_resume.MC__MC_A0);
	IoReg_Write32(MC__MC_A4_ADDR , vpq_memc_suspend_resume.MC__MC_A4);
	IoReg_Write32(MC__MC_A8_ADDR , vpq_memc_suspend_resume.MC__MC_A8);
	IoReg_Write32(MC__MC_AC_ADDR , vpq_memc_suspend_resume.MC__MC_AC);
	IoReg_Write32(MC__MC_B0_ADDR , vpq_memc_suspend_resume.MC__MC_B0);
	IoReg_Write32(MC__MC_B4_ADDR , vpq_memc_suspend_resume.MC__MC_B4);
	IoReg_Write32(MC__MC_B8_ADDR , vpq_memc_suspend_resume.MC__MC_B8);
	IoReg_Write32(MC__MC_BC_ADDR , vpq_memc_suspend_resume.MC__MC_BC);
	IoReg_Write32(MC__MC_C0_ADDR , vpq_memc_suspend_resume.MC__MC_C0);
	IoReg_Write32(MC__MC_C4_ADDR , vpq_memc_suspend_resume.MC__MC_C4);
	IoReg_Write32(MC__MC_C8_ADDR , vpq_memc_suspend_resume.MC__MC_C8);
	IoReg_Write32(MC__MC_CC_ADDR , vpq_memc_suspend_resume.MC__MC_CC);
	IoReg_Write32(MC__MC_D0_ADDR , vpq_memc_suspend_resume.MC__MC_D0);
	IoReg_Write32(MC__MC_D4_ADDR , vpq_memc_suspend_resume.MC__MC_D4);
	IoReg_Write32(MC__MC_D8_ADDR , vpq_memc_suspend_resume.MC__MC_D8);
	IoReg_Write32(MC__MC_DC_ADDR , vpq_memc_suspend_resume.MC__MC_DC);
	IoReg_Write32(MC__MC_E0_ADDR , vpq_memc_suspend_resume.MC__MC_E0);
	//MC2
	IoReg_Write32(MC2__MC2_00_ADDR , vpq_memc_suspend_resume.MC2__MC2_00);
	IoReg_Write32(MC2__MC2_04_ADDR , vpq_memc_suspend_resume.MC2__MC2_04);
	IoReg_Write32(MC2__MC2_08_ADDR , vpq_memc_suspend_resume.MC2__MC2_08);
	IoReg_Write32(MC2__MC2_0C_ADDR , vpq_memc_suspend_resume.MC2__MC2_0C);
	IoReg_Write32(MC2__MC2_10_ADDR , vpq_memc_suspend_resume.MC2__MC2_10);
	IoReg_Write32(MC2__MC2_14_ADDR , vpq_memc_suspend_resume.MC2__MC2_14);
	IoReg_Write32(MC2__MC2_18_ADDR , vpq_memc_suspend_resume.MC2__MC2_18);
	IoReg_Write32(MC2__MC2_1C_ADDR , vpq_memc_suspend_resume.MC2__MC2_1C);
	IoReg_Write32(MC2__MC2_20_ADDR , vpq_memc_suspend_resume.MC2__MC2_20);
	IoReg_Write32(MC2__MC2_24_ADDR , vpq_memc_suspend_resume.MC2__MC2_24);
	IoReg_Write32(MC2__MC2_28_ADDR , vpq_memc_suspend_resume.MC2__MC2_28);
	IoReg_Write32(MC2__MC2_2C_ADDR , vpq_memc_suspend_resume.MC2__MC2_2C);
	IoReg_Write32(MC2__MC2_30_ADDR , vpq_memc_suspend_resume.MC2__MC2_30);
	IoReg_Write32(MC2__MC2_50_ADDR , vpq_memc_suspend_resume.MC2__MC2_50);
	IoReg_Write32(MC2__MC2_54_ADDR , vpq_memc_suspend_resume.MC2__MC2_54);
	IoReg_Write32(MC2__MC2_58_ADDR , vpq_memc_suspend_resume.MC2__MC2_58);
	IoReg_Write32(MC2__MC2_5C_ADDR , vpq_memc_suspend_resume.MC2__MC2_5C);
	IoReg_Write32(MC2__MC2_60_ADDR , vpq_memc_suspend_resume.MC2__MC2_60);
	IoReg_Write32(MC2__MC2_64_ADDR , vpq_memc_suspend_resume.MC2__MC2_64);
	IoReg_Write32(MC2__MC2_68_ADDR , vpq_memc_suspend_resume.MC2__MC2_68);
	IoReg_Write32(MC2__MC2_6C_ADDR , vpq_memc_suspend_resume.MC2__MC2_6C);
	IoReg_Write32(MC2__MC2_70_ADDR , vpq_memc_suspend_resume.MC2__MC2_70);
	IoReg_Write32(MC2__MC2_74_ADDR , vpq_memc_suspend_resume.MC2__MC2_74);
	IoReg_Write32(MC2__MC2_7C_ADDR , vpq_memc_suspend_resume.MC2__MC2_7C);
	IoReg_Write32(MC2__MC2_80_ADDR , vpq_memc_suspend_resume.MC2__MC2_80);
	IoReg_Write32(MC2__MC2_84_ADDR , vpq_memc_suspend_resume.MC2__MC2_84);
	IoReg_Write32(MC2__MC2_88_ADDR , vpq_memc_suspend_resume.MC2__MC2_88);
	IoReg_Write32(MC2__MC2_8C_ADDR , vpq_memc_suspend_resume.MC2__MC2_8C);
	IoReg_Write32(MC2__MC2_90_ADDR , vpq_memc_suspend_resume.MC2__MC2_90);
	IoReg_Write32(MC2__MC2_94_ADDR , vpq_memc_suspend_resume.MC2__MC2_94);
	IoReg_Write32(MC2__MC2_98_ADDR , vpq_memc_suspend_resume.MC2__MC2_98);
	IoReg_Write32(MC2__MC2_9C_ADDR , vpq_memc_suspend_resume.MC2__MC2_9C);
	IoReg_Write32(MC2__MC2_A0_ADDR , vpq_memc_suspend_resume.MC2__MC2_A0);
	IoReg_Write32(MC2__MC2_A4_ADDR , vpq_memc_suspend_resume.MC2__MC2_A4);
	IoReg_Write32(MC2__MC2_A8_ADDR , vpq_memc_suspend_resume.MC2__MC2_A8);
	//LBMC
	IoReg_Write32(LBMC__LBMC_00_ADDR , vpq_memc_suspend_resume.LBMC__LBMC_00);
	IoReg_Write32(LBMC__LBMC_04_ADDR , vpq_memc_suspend_resume.LBMC__LBMC_04);
	IoReg_Write32(LBMC__LBMC_08_ADDR , vpq_memc_suspend_resume.LBMC__LBMC_08);
	IoReg_Write32(LBMC__LBMC_0C_ADDR , vpq_memc_suspend_resume.LBMC__LBMC_0C);
	IoReg_Write32(LBMC__LBMC_10_ADDR , vpq_memc_suspend_resume.LBMC__LBMC_10);
	IoReg_Write32(LBMC__LBMC_14_ADDR , vpq_memc_suspend_resume.LBMC__LBMC_14);
	IoReg_Write32(LBMC__LBMC_18_ADDR , vpq_memc_suspend_resume.LBMC__LBMC_18);
	IoReg_Write32(LBMC__LBMC_1C_ADDR , vpq_memc_suspend_resume.LBMC__LBMC_1C);
	IoReg_Write32(LBMC__LBMC_20_ADDR , vpq_memc_suspend_resume.LBMC__LBMC_20);
	IoReg_Write32(LBMC__LBMC_24_ADDR , vpq_memc_suspend_resume.LBMC__LBMC_24);
	IoReg_Write32(LBMC__LBMC_28_ADDR , vpq_memc_suspend_resume.LBMC__LBMC_28);
	IoReg_Write32(LBMC__LBMC_2C_ADDR , vpq_memc_suspend_resume.LBMC__LBMC_2C);
	IoReg_Write32(LBMC__LBMC_30_ADDR , vpq_memc_suspend_resume.LBMC__LBMC_30);
	IoReg_Write32(LBMC__LBMC_34_ADDR , vpq_memc_suspend_resume.LBMC__LBMC_34);
	IoReg_Write32(LBMC__LBMC_38_ADDR , vpq_memc_suspend_resume.LBMC__LBMC_38);
	IoReg_Write32(LBMC__LBMC_3C_ADDR , vpq_memc_suspend_resume.LBMC__LBMC_3C);
	IoReg_Write32(LBMC__LBMC_40_ADDR , vpq_memc_suspend_resume.LBMC__LBMC_40);
	IoReg_Write32(LBMC__LBMC_44_ADDR , vpq_memc_suspend_resume.LBMC__LBMC_44);
	IoReg_Write32(LBMC__LBMC_48_ADDR , vpq_memc_suspend_resume.LBMC__LBMC_48);
	IoReg_Write32(LBMC__LBMC_4C_ADDR , vpq_memc_suspend_resume.LBMC__LBMC_4C);
	IoReg_Write32(LBMC__LBMC_50_ADDR , vpq_memc_suspend_resume.LBMC__LBMC_50);
	IoReg_Write32(LBMC__LBMC_54_ADDR , vpq_memc_suspend_resume.LBMC__LBMC_54);
	IoReg_Write32(LBMC__LBMC_58_ADDR , vpq_memc_suspend_resume.LBMC__LBMC_58);
	IoReg_Write32(LBMC__LBMC_5C_ADDR , vpq_memc_suspend_resume.LBMC__LBMC_5C);
	IoReg_Write32(LBMC__LBMC_60_ADDR , vpq_memc_suspend_resume.LBMC__LBMC_60);
	IoReg_Write32(LBMC__LBMC_80_ADDR , vpq_memc_suspend_resume.LBMC__LBMC_80);
	IoReg_Write32(LBMC__LBMC_84_ADDR , vpq_memc_suspend_resume.LBMC__LBMC_84);
	IoReg_Write32(LBMC__LBMC_88_ADDR , vpq_memc_suspend_resume.LBMC__LBMC_88);
	//BBD
	IoReg_Write32(BBD__BBD_00_ADDR , vpq_memc_suspend_resume.BBD__BBD_00);
	IoReg_Write32(BBD__BBD_04_ADDR , vpq_memc_suspend_resume.BBD__BBD_04);
	IoReg_Write32(BBD__BBD_08_ADDR , vpq_memc_suspend_resume.BBD__BBD_08);
	IoReg_Write32(BBD__BBD_0C_ADDR , vpq_memc_suspend_resume.BBD__BBD_0C);
	IoReg_Write32(BBD__BBD_10_ADDR , vpq_memc_suspend_resume.BBD__BBD_10);
	//KMC_METER_TOP
	IoReg_Write32(KMC_METER_TOP__KMC_METER_TOP_00_ADDR , vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_00);
	IoReg_Write32(KMC_METER_TOP__KMC_METER_TOP_04_ADDR , vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_04);
	IoReg_Write32(KMC_METER_TOP__KMC_METER_TOP_08_ADDR , vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_08);
	IoReg_Write32(KMC_METER_TOP__KMC_METER_TOP_0C_ADDR , vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_0C);
	IoReg_Write32(KMC_METER_TOP__KMC_METER_TOP_10_ADDR , vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_10);
	IoReg_Write32(KMC_METER_TOP__KMC_METER_TOP_14_ADDR , vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_14);
	IoReg_Write32(KMC_METER_TOP__KMC_METER_TOP_18_ADDR , vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_18);
	IoReg_Write32(KMC_METER_TOP__KMC_METER_TOP_1C_ADDR , vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_1C);
	IoReg_Write32(KMC_METER_TOP__KMC_METER_TOP_20_ADDR , vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_20);
	IoReg_Write32(KMC_METER_TOP__KMC_METER_TOP_24_ADDR , vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_24);
	IoReg_Write32(KMC_METER_TOP__KMC_METER_TOP_28_ADDR , vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_28);
	IoReg_Write32(KMC_METER_TOP__KMC_METER_TOP_2C_ADDR , vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_2C);
	IoReg_Write32(KMC_METER_TOP__KMC_METER_TOP_30_ADDR , vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_30);
	IoReg_Write32(KMC_METER_TOP__KMC_METER_TOP_34_ADDR , vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_34);
	IoReg_Write32(KMC_METER_TOP__KMC_METER_TOP_38_ADDR , vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_38);
	IoReg_Write32(KMC_METER_TOP__KMC_METER_TOP_3C_ADDR , vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_3C);
	IoReg_Write32(KMC_METER_TOP__KMC_METER_TOP_40_ADDR , vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_40);
	IoReg_Write32(KMC_METER_TOP__KMC_METER_TOP_44_ADDR , vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_44);
	IoReg_Write32(KMC_METER_TOP__KMC_METER_TOP_48_ADDR , vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_48);
	IoReg_Write32(KMC_METER_TOP__KMC_METER_TOP_4C_ADDR , vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_4C);
	IoReg_Write32(KMC_METER_TOP__KMC_METER_TOP_50_ADDR , vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_50);
	IoReg_Write32(KMC_METER_TOP__KMC_METER_TOP_54_ADDR , vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_54);
	IoReg_Write32(KMC_METER_TOP__KMC_METER_TOP_58_ADDR , vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_58);
	IoReg_Write32(KMC_METER_TOP__KMC_METER_TOP_5C_ADDR , vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_5C);
	IoReg_Write32(KMC_METER_TOP__KMC_METER_TOP_60_ADDR , vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_60);
	IoReg_Write32(KMC_METER_TOP__KMC_METER_TOP_64_ADDR , vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_64);
	IoReg_Write32(KMC_METER_TOP__KMC_METER_TOP_68_ADDR , vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_68);
	IoReg_Write32(KMC_METER_TOP__KMC_METER_TOP_6C_ADDR , vpq_memc_suspend_resume.KMC_METER_TOP__KMC_METER_TOP_6C);
	//KMC_DM_METER_TOP
	IoReg_Write32(KMC_DM_METER_TOP__KMC_METER_REG0_ADDR , vpq_memc_suspend_resume.KMC_DM_METER_TOP__KMC_METER_REG0);
	IoReg_Write32(KMC_DM_METER_TOP__KMC_METER_NUM_ADDR , vpq_memc_suspend_resume.KMC_DM_METER_TOP__KMC_METER_NUM);
	IoReg_Write32(KMC_DM_METER_TOP__KMC_METER_H_NUM0_ADDR , vpq_memc_suspend_resume.KMC_DM_METER_TOP__KMC_METER_H_NUM0);
	IoReg_Write32(KMC_DM_METER_TOP__KMC_METER_H_NUM1_ADDR , vpq_memc_suspend_resume.KMC_DM_METER_TOP__KMC_METER_H_NUM1);
	IoReg_Write32(KMC_DM_METER_TOP__METER_TH0_ADDR , vpq_memc_suspend_resume.KMC_DM_METER_TOP__METER_TH0);
	IoReg_Write32(KMC_DM_METER_TOP__METER_TH1_ADDR , vpq_memc_suspend_resume.KMC_DM_METER_TOP__METER_TH1);
	IoReg_Write32(KMC_DM_METER_TOP__METER_TH3_ADDR , vpq_memc_suspend_resume.KMC_DM_METER_TOP__METER_TH3);
	IoReg_Write32(KMC_DM_METER_TOP__METER_TIMER0_ADDR , vpq_memc_suspend_resume.KMC_DM_METER_TOP__METER_TIMER0);
	//KME_TOP
	IoReg_Write32(KME_TOP__KME_TOP_04_ADDR , vpq_memc_suspend_resume.KME_TOP__KME_TOP_04);
	IoReg_Write32(KME_TOP__KME_TOP_08_ADDR , vpq_memc_suspend_resume.KME_TOP__KME_TOP_08);
	IoReg_Write32(KME_TOP__KME_TOP_10_ADDR , vpq_memc_suspend_resume.KME_TOP__KME_TOP_10);
	IoReg_Write32(KME_TOP__KME_TOP_14_ADDR , vpq_memc_suspend_resume.KME_TOP__KME_TOP_14);
	IoReg_Write32(KME_TOP__KME_TOP_18_ADDR , vpq_memc_suspend_resume.KME_TOP__KME_TOP_18);
	IoReg_Write32(KME_TOP__KME_TOP_1C_ADDR , vpq_memc_suspend_resume.KME_TOP__KME_TOP_1C);
	IoReg_Write32(KME_TOP__KME_TOP_20_ADDR , vpq_memc_suspend_resume.KME_TOP__KME_TOP_20);
	IoReg_Write32(KME_TOP__KME_TOP_24_ADDR , vpq_memc_suspend_resume.KME_TOP__KME_TOP_24);
	IoReg_Write32(KME_TOP__KME_TOP_28_ADDR , vpq_memc_suspend_resume.KME_TOP__KME_TOP_28);
	IoReg_Write32(KME_TOP__KME_TOP_2C_ADDR , vpq_memc_suspend_resume.KME_TOP__KME_TOP_2C);
	IoReg_Write32(KME_TOP__KME_TOP_30_ADDR , vpq_memc_suspend_resume.KME_TOP__KME_TOP_30);
	IoReg_Write32(KME_TOP__KME_TOP_34_ADDR , vpq_memc_suspend_resume.KME_TOP__KME_TOP_34);
	IoReg_Write32(KME_TOP__KME_TOP_38_ADDR , vpq_memc_suspend_resume.KME_TOP__KME_TOP_38);
	IoReg_Write32(KME_TOP__KME_TOP_3C_ADDR , vpq_memc_suspend_resume.KME_TOP__KME_TOP_3C);
	//KME_VBUF_TOP
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_00_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_00);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_04_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_04);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_08_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_08);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_0C_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_0C);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_10_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_10);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_14_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_14);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_18_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_18);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_1C_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_1C);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_20_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_20);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_24_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_24);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_28_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_28);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_2C_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_2C);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_30_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_30);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_34_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_34);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_38_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_38);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_3C_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_3C);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_40_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_40);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_44_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_44);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_48_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_48);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_4C_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_4C);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_50_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_50);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_54_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_54);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_58_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_58);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_5C_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_5C);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_60_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_60);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_64_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_64);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_68_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_68);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_6C_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_6C);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_70_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_70);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_74_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_74);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_78_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_78);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_7C_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_7C);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_80_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_80);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_84_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_84);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_88_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_88);
	IoReg_Write32(KME_VBUF_TOP__KME_VBUF_TOP_8C_ADDR , vpq_memc_suspend_resume.KME_VBUF_TOP__KME_VBUF_TOP_8C);
	//KME_ME1_TOP2
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_00_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_00);
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_04_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_04);
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_08_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_08);
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_0C_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_0C);
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_10_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_10);
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_14_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_14);
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_18_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_18);
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_1C_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_1C);
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_20_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_20);
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_24_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_24);
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_28_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_28);
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_2C_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_2C);
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_30_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_30);
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_34_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_34);
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_38_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_38);
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_3C_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_3C);
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_40_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_40);
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_44_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_44);
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_48_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_48);
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_4C_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_4C);
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_50_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_50);
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_54_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_54);
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_58_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_58);
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_5C_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_5C);
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_60_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_60);
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_64_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_64);
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_80_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_80);
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_84_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_84);
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_88_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_88);
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_8C_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_8C);
	IoReg_Write32(KME_ME1_TOP2__KME_ME1_TOP2_90_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP2__KME_ME1_TOP2_90);
	//KME_ME1_TOP3
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_00_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_00);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_04_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_04);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_08_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_08);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_0C_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_0C);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_10_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_10);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_14_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_14);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_18_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_18);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_1C_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_1C);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_20_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_20);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_24_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_24);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_28_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_28);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_2C_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_2C);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_30_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_30);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_34_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_34);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_38_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_38);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_3C_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_3C);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_40_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_40);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_44_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_44);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_48_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_48);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_4C_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_4C);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_50_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_50);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_54_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_54);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_58_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_58);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_5C_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_5C);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_60_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_60);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_64_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_64);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_68_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_68);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_6C_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_6C);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_70_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_70);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_74_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_74);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_78_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_78);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_7C_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_7C);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_80_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_80);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_84_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_84);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_88_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_88);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_8C_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_8C);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_90_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_90);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_94_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_94);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_98_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_98);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_9C_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_9C);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_A0_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_A0);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_A4_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_A4);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_A8_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_A8);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_AC_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_AC);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_B0_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_B0);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_B4_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_B4);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_B8_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_B8);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_BC_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_BC);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_C0_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_C0);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_C4_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_C4);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_C8_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_C8);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_CC_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_CC);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_D0_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_D0);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_D4_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_D4);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_D8_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_D8);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_DC_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_DC);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_E0_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_E0);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_E4_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_E4);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_E8_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_E8);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_EC_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_EC);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_F0_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_F0);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_F4_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_F4);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_F8_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_F8);
	IoReg_Write32(KME_ME1_TOP3__KME_ME1_TOP3_FC_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP3__KME_ME1_TOP3_FC);
	//KME_ME1_TOP4
	//KME_ME1_TOP5
	//KME_LBME_TOP
	IoReg_Write32(KME_LBME_TOP__KME_LBME_TOP_00_ADDR , vpq_memc_suspend_resume.KME_LBME_TOP__KME_LBME_TOP_00);
	IoReg_Write32(KME_LBME_TOP__KME_LBME_TOP_04_ADDR , vpq_memc_suspend_resume.KME_LBME_TOP__KME_LBME_TOP_04);
	IoReg_Write32(KME_LBME_TOP__KME_LBME_TOP_08_ADDR , vpq_memc_suspend_resume.KME_LBME_TOP__KME_LBME_TOP_08);
	IoReg_Write32(KME_LBME_TOP__KME_LBME_TOP_0C_ADDR , vpq_memc_suspend_resume.KME_LBME_TOP__KME_LBME_TOP_0C);
	IoReg_Write32(KME_LBME_TOP__KME_LBME_TOP_10_ADDR , vpq_memc_suspend_resume.KME_LBME_TOP__KME_LBME_TOP_10);
	IoReg_Write32(KME_LBME_TOP__KME_LBME_TOP_14_ADDR , vpq_memc_suspend_resume.KME_LBME_TOP__KME_LBME_TOP_14);
	IoReg_Write32(KME_LBME_TOP__KME_LBME_TOP_18_ADDR , vpq_memc_suspend_resume.KME_LBME_TOP__KME_LBME_TOP_18);
	IoReg_Write32(KME_LBME_TOP__KME_LBME_TOP_1C_ADDR , vpq_memc_suspend_resume.KME_LBME_TOP__KME_LBME_TOP_1C);
	IoReg_Write32(KME_LBME_TOP__KME_LBME_TOP_20_ADDR , vpq_memc_suspend_resume.KME_LBME_TOP__KME_LBME_TOP_20);
	IoReg_Write32(KME_LBME_TOP__KME_LBME_TOP_24_ADDR , vpq_memc_suspend_resume.KME_LBME_TOP__KME_LBME_TOP_24);
	//KME_ME2_CALC1
	IoReg_Write32(KME_ME2_CALC1__KME_ME2_CALC1_00_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_00);
	IoReg_Write32(KME_ME2_CALC1__KME_ME2_CALC1_04_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_04);
	IoReg_Write32(KME_ME2_CALC1__KME_ME2_CALC1_08_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_08);
	IoReg_Write32(KME_ME2_CALC1__KME_ME2_CALC1_0C_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_0C);
	IoReg_Write32(KME_ME2_CALC1__KME_ME2_CALC1_10_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_10);
	IoReg_Write32(KME_ME2_CALC1__KME_ME2_CALC1_14_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_14);
	IoReg_Write32(KME_ME2_CALC1__KME_ME2_CALC1_18_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_18);
	IoReg_Write32(KME_ME2_CALC1__KME_ME2_CALC1_1C_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_1C);
	IoReg_Write32(KME_ME2_CALC1__KME_ME2_CALC1_20_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_20);
	IoReg_Write32(KME_ME2_CALC1__KME_ME2_CALC1_24_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_24);
	IoReg_Write32(KME_ME2_CALC1__KME_ME2_CALC1_28_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_28);
	IoReg_Write32(KME_ME2_CALC1__KME_ME2_CALC1_2C_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_2C);
	IoReg_Write32(KME_ME2_CALC1__KME_ME2_CALC1_30_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_30);
	IoReg_Write32(KME_ME2_CALC1__KME_ME2_CALC1_34_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_34);
	IoReg_Write32(KME_ME2_CALC1__KME_ME2_CALC1_38_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_38);
	IoReg_Write32(KME_ME2_CALC1__KME_ME2_CALC1_3C_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_3C);
	IoReg_Write32(KME_ME2_CALC1__KME_ME2_CALC1_40_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC1__KME_ME2_CALC1_40);
	//KME_LOGO2
	IoReg_Write32(KME_LOGO2__KME_LOGO2_00_ADDR , vpq_memc_suspend_resume.KME_LOGO2__KME_LOGO2_00);
	IoReg_Write32(KME_LOGO2__KME_LOGO2_04_ADDR , vpq_memc_suspend_resume.KME_LOGO2__KME_LOGO2_04);
	IoReg_Write32(KME_LOGO2__KME_LOGO2_08_ADDR , vpq_memc_suspend_resume.KME_LOGO2__KME_LOGO2_08);
	IoReg_Write32(KME_LOGO2__KME_LOGO2_0C_ADDR , vpq_memc_suspend_resume.KME_LOGO2__KME_LOGO2_0C);
	IoReg_Write32(KME_LOGO2__KME_LOGO2_10_ADDR , vpq_memc_suspend_resume.KME_LOGO2__KME_LOGO2_10);
	IoReg_Write32(KME_LOGO2__KME_LOGO2_14_ADDR , vpq_memc_suspend_resume.KME_LOGO2__KME_LOGO2_14);
	IoReg_Write32(KME_LOGO2__KME_LOGO2_18_ADDR , vpq_memc_suspend_resume.KME_LOGO2__KME_LOGO2_18);
	IoReg_Write32(KME_LOGO2__KME_LOGO2_1C_ADDR , vpq_memc_suspend_resume.KME_LOGO2__KME_LOGO2_1C);
	IoReg_Write32(KME_LOGO2__KME_LOGO2_20_ADDR , vpq_memc_suspend_resume.KME_LOGO2__KME_LOGO2_20);
	IoReg_Write32(KME_LOGO2__KME_LOGO2_24_ADDR , vpq_memc_suspend_resume.KME_LOGO2__KME_LOGO2_24);
	IoReg_Write32(KME_LOGO2__KME_LOGO2_28_ADDR , vpq_memc_suspend_resume.KME_LOGO2__KME_LOGO2_28);
	IoReg_Write32(KME_LOGO2__KME_LOGO2_2C_ADDR , vpq_memc_suspend_resume.KME_LOGO2__KME_LOGO2_2C);
	IoReg_Write32(KME_LOGO2__KME_LOGO2_30_ADDR , vpq_memc_suspend_resume.KME_LOGO2__KME_LOGO2_30);
	IoReg_Write32(KME_LOGO2__KME_LOGO2_34_ADDR , vpq_memc_suspend_resume.KME_LOGO2__KME_LOGO2_34);
	//KME_IPME
	IoReg_Write32(KME_IPME__KME_IPME_00_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_00);
	IoReg_Write32(KME_IPME__KME_IPME_04_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_04);
	IoReg_Write32(KME_IPME__KME_IPME_08_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_08);
	IoReg_Write32(KME_IPME__KME_IPME_0C_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_0C);
	IoReg_Write32(KME_IPME__KME_IPME_10_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_10);
	IoReg_Write32(KME_IPME__KME_IPME_14_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_14);
	IoReg_Write32(KME_IPME__KME_IPME_18_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_18);
	IoReg_Write32(KME_IPME__KME_IPME_1C_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_1C);
	IoReg_Write32(KME_IPME__KME_IPME_20_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_20);
	IoReg_Write32(KME_IPME__KME_IPME_24_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_24);
	IoReg_Write32(KME_IPME__KME_IPME_28_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_28);
	IoReg_Write32(KME_IPME__KME_IPME_2C_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_2C);
	IoReg_Write32(KME_IPME__KME_IPME_30_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_30);
	IoReg_Write32(KME_IPME__KME_IPME_34_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_34);
	IoReg_Write32(KME_IPME__KME_IPME_38_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_38);
	IoReg_Write32(KME_IPME__KME_IPME_3C_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_3C);
	IoReg_Write32(KME_IPME__KME_IPME_40_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_40);
	IoReg_Write32(KME_IPME__KME_IPME_44_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_44);
	IoReg_Write32(KME_IPME__KME_IPME_48_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_48);
	IoReg_Write32(KME_IPME__KME_IPME_4C_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_4C);
	IoReg_Write32(KME_IPME__KME_IPME_50_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_50);
	IoReg_Write32(KME_IPME__KME_IPME_54_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_54);
	IoReg_Write32(KME_IPME__KME_IPME_58_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_58);
	IoReg_Write32(KME_IPME__KME_IPME_5C_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_5C);
	IoReg_Write32(KME_IPME__KME_IPME_60_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_60);
	IoReg_Write32(KME_IPME__KME_IPME_64_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_64);
	IoReg_Write32(KME_IPME__KME_IPME_68_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_68);
	IoReg_Write32(KME_IPME__KME_IPME_6C_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_6C);
	IoReg_Write32(KME_IPME__KME_IPME_70_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_70);
	IoReg_Write32(KME_IPME__KME_IPME_74_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_74);
	IoReg_Write32(KME_IPME__KME_IPME_78_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_78);
	IoReg_Write32(KME_IPME__KME_IPME_7C_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_7C);
	IoReg_Write32(KME_IPME__KME_IPME_80_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_80);
	IoReg_Write32(KME_IPME__KME_IPME_84_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_84);
	IoReg_Write32(KME_IPME__KME_IPME_88_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_88);
	IoReg_Write32(KME_IPME__KME_IPME_8C_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_8C);
	IoReg_Write32(KME_IPME__KME_IPME_90_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_90);
	IoReg_Write32(KME_IPME__KME_IPME_94_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_94);
	IoReg_Write32(KME_IPME__KME_IPME_98_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_98);
	IoReg_Write32(KME_IPME__KME_IPME_9C_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_9C);
	IoReg_Write32(KME_IPME__KME_IPME_A0_ADDR , vpq_memc_suspend_resume.KME_IPME__KME_IPME_A0);
	//KME_IPME1
	IoReg_Write32(KME_IPME1__KME_IPME1_00_ADDR , vpq_memc_suspend_resume.KME_IPME1__KME_IPME1_00);
	IoReg_Write32(KME_IPME1__KME_IPME1_04_ADDR , vpq_memc_suspend_resume.KME_IPME1__KME_IPME1_04);
	IoReg_Write32(KME_IPME1__KME_IPME1_08_ADDR , vpq_memc_suspend_resume.KME_IPME1__KME_IPME1_08);
	IoReg_Write32(KME_IPME1__KME_IPME1_0C_ADDR , vpq_memc_suspend_resume.KME_IPME1__KME_IPME1_0C);
	//KME_LOGO0
	IoReg_Write32(KME_LOGO0__KME_LOGO0_00_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_00);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_04_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_04);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_08_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_08);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_0C_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_0C);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_10_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_10);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_14_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_14);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_18_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_18);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_1C_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_1C);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_20_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_20);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_24_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_24);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_28_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_28);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_2C_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_2C);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_30_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_30);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_34_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_34);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_38_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_38);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_3C_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_3C);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_40_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_40);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_44_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_44);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_48_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_48);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_4C_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_4C);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_50_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_50);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_54_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_54);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_58_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_58);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_5C_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_5C);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_60_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_60);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_64_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_64);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_68_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_68);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_6C_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_6C);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_70_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_70);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_74_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_74);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_78_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_78);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_7C_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_7C);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_80_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_80);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_84_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_84);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_88_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_88);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_8C_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_8C);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_90_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_90);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_94_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_94);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_98_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_98);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_9C_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_9C);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_A0_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_A0);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_A4_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_A4);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_A8_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_A8);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_AC_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_AC);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_B0_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_B0);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_B4_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_B4);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_B8_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_B8);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_BC_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_BC);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_C0_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_C0);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_C4_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_C4);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_C8_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_C8);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_CC_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_CC);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_D0_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_D0);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_D4_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_D4);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_D8_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_D8);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_DC_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_DC);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_E0_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_E0);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_E4_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_E4);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_E8_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_E8);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_EC_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_EC);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_F0_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_F0);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_F4_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_F4);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_F8_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_F8);
	IoReg_Write32(KME_LOGO0__KME_LOGO0_FC_ADDR , vpq_memc_suspend_resume.KME_LOGO0__KME_LOGO0_FC);
	//KME_LOGO1
	IoReg_Write32(KME_LOGO1__KME_LOGO1_00_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_00);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_04_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_04);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_08_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_08);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_0C_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_0C);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_10_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_10);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_14_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_14);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_18_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_18);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_1C_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_1C);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_20_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_20);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_2C_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_2C);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_30_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_30);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_34_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_34);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_38_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_38);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_3C_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_3C);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_40_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_40);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_44_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_44);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_48_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_48);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_4C_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_4C);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_50_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_50);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_54_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_54);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_58_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_58);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_5C_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_5C);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_60_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_60);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_64_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_64);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_68_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_68);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_6C_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_6C);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_70_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_70);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_74_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_74);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_78_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_78);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_7C_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_7C);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_80_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_80);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_84_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_84);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_88_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_88);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_8C_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_8C);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_90_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_90);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_94_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_94);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_98_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_98);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_9C_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_9C);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_A0_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_A0);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_A4_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_A4);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_A8_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_A8);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_AC_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_AC);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_B0_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_B0);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_B4_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_B4);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_B8_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_B8);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_BC_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_BC);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_C0_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_C0);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_C4_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_C4);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_C8_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_C8);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_CC_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_CC);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_D0_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_D0);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_D4_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_D4);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_D8_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_D8);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_DC_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_DC);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_E0_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_E0);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_E4_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_E4);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_E8_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_E8);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_EC_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_EC);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_F0_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_F0);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_F4_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_F4);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_F8_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_F8);
	IoReg_Write32(KME_LOGO1__KME_LOGO1_FC_ADDR , vpq_memc_suspend_resume.KME_LOGO1__KME_LOGO1_FC);
	//KME_DM_TOP0
	IoReg_Write32(KME_DM_TOP0__KME_00_START_ADDRESS0_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_START_ADDRESS0);
	IoReg_Write32(KME_DM_TOP0__KME_00_START_ADDRESS1_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_START_ADDRESS1);
	IoReg_Write32(KME_DM_TOP0__KME_00_START_ADDRESS2_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_START_ADDRESS2);
	IoReg_Write32(KME_DM_TOP0__KME_00_START_ADDRESS3_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_START_ADDRESS3);
	IoReg_Write32(KME_DM_TOP0__KME_00_START_ADDRESS4_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_START_ADDRESS4);
	IoReg_Write32(KME_DM_TOP0__KME_00_START_ADDRESS5_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_START_ADDRESS5);
	IoReg_Write32(KME_DM_TOP0__KME_00_END_ADDRESS0_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_END_ADDRESS0);
	IoReg_Write32(KME_DM_TOP0__KME_00_END_ADDRESS1_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_END_ADDRESS1);
	IoReg_Write32(KME_DM_TOP0__KME_00_END_ADDRESS2_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_END_ADDRESS2);
	IoReg_Write32(KME_DM_TOP0__KME_00_END_ADDRESS3_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_END_ADDRESS3);
	IoReg_Write32(KME_DM_TOP0__KME_00_END_ADDRESS4_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_END_ADDRESS4);
	IoReg_Write32(KME_DM_TOP0__KME_00_END_ADDRESS5_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_END_ADDRESS5);
	IoReg_Write32(KME_DM_TOP0__KME_00_LR_OFFSET_ADDR_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_LR_OFFSET_ADDR);
	IoReg_Write32(KME_DM_TOP0__KME_00_LINE_OFFSET_ADDR_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_LINE_OFFSET_ADDR);
	IoReg_Write32(KME_DM_TOP0__KME_00_AGENT_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_AGENT);
	IoReg_Write32(KME_DM_TOP0__KME_00_THRESHOLD_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_00_THRESHOLD);
	IoReg_Write32(KME_DM_TOP0__KME_01_AGENT_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_01_AGENT);
	IoReg_Write32(KME_DM_TOP0__KME_01_THRESHOLD_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_01_THRESHOLD);
	IoReg_Write32(KME_DM_TOP0__KME_02_AGENT_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_02_AGENT);
	IoReg_Write32(KME_DM_TOP0__KME_02_THRESHOLD_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_02_THRESHOLD);
	IoReg_Write32(KME_DM_TOP0__KME_03_AGENT_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_03_AGENT);
	IoReg_Write32(KME_DM_TOP0__KME_03_THRESHOLD_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_03_THRESHOLD);
	IoReg_Write32(KME_DM_TOP0__KME_04_START_ADDRESS0_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_04_START_ADDRESS0);
	IoReg_Write32(KME_DM_TOP0__KME_04_START_ADDRESS1_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_04_START_ADDRESS1);
	IoReg_Write32(KME_DM_TOP0__KME_04_END_ADDRESS0_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_04_END_ADDRESS0);
	IoReg_Write32(KME_DM_TOP0__KME_04_END_ADDRESS1_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_04_END_ADDRESS1);
	IoReg_Write32(KME_DM_TOP0__KME_04_LR_OFFSET_ADDR_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_04_LR_OFFSET_ADDR);
	IoReg_Write32(KME_DM_TOP0__KME_04_LINE_OFFSET_ADDR_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_04_LINE_OFFSET_ADDR);
	IoReg_Write32(KME_DM_TOP0__KME_04_AGENT_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_04_AGENT);
	IoReg_Write32(KME_DM_TOP0__KME_04_THRESHOLD_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_04_THRESHOLD);
	IoReg_Write32(KME_DM_TOP0__KME_05_START_ADDRESS0_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_05_START_ADDRESS0);
	IoReg_Write32(KME_DM_TOP0__KME_05_START_ADDRESS1_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_05_START_ADDRESS1);
	IoReg_Write32(KME_DM_TOP0__KME_05_END_ADDRESS0_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_05_END_ADDRESS0);
	IoReg_Write32(KME_DM_TOP0__KME_05_END_ADDRESS1_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_05_END_ADDRESS1);
	IoReg_Write32(KME_DM_TOP0__KME_05_LR_OFFSET_ADDR_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_05_LR_OFFSET_ADDR);
	IoReg_Write32(KME_DM_TOP0__KME_05_LINE_OFFSET_ADDR_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_05_LINE_OFFSET_ADDR);
	IoReg_Write32(KME_DM_TOP0__KME_05_AGENT_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_05_AGENT);
	IoReg_Write32(KME_DM_TOP0__KME_05_THRESHOLD_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_05_THRESHOLD);
	IoReg_Write32(KME_DM_TOP0__KME06AGENT_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME06AGENT);
	IoReg_Write32(KME_DM_TOP0__KME_06_THRESHOLD_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_06_THRESHOLD);
	IoReg_Write32(KME_DM_TOP0__KME_07_AGENT_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_07_AGENT);
	IoReg_Write32(KME_DM_TOP0__KME_07_THRESHOLD_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_07_THRESHOLD);
	IoReg_Write32(KME_DM_TOP0__KME_ME_RESOLUTION_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_ME_RESOLUTION);
	IoReg_Write32(KME_DM_TOP0__KME_MV_RESOLUTION_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_MV_RESOLUTION);
	IoReg_Write32(KME_DM_TOP0__KME_SOURCE_MODE_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_SOURCE_MODE);
	IoReg_Write32(KME_DM_TOP0__KME_ME_HNUM_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_ME_HNUM);
	IoReg_Write32(KME_DM_TOP0__KME_TIMER_ENABLE_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_TIMER_ENABLE);
	IoReg_Write32(KME_DM_TOP0__KME_HOLD_TIME0_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_HOLD_TIME0);
	IoReg_Write32(KME_DM_TOP0__KME_HOLD_TIME1_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_HOLD_TIME1);
	IoReg_Write32(KME_DM_TOP0__KME_HOLD_TIME2_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_HOLD_TIME2);
	IoReg_Write32(KME_DM_TOP0__KME_HOLD_TIME3_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_HOLD_TIME3);
	IoReg_Write32(KME_DM_TOP0__KME_DM_TOP0_CC_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__KME_DM_TOP0_CC);
	IoReg_Write32(KME_DM_TOP0__WAIT_TIME1_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__WAIT_TIME1);
	IoReg_Write32(KME_DM_TOP0__WAIT_TIME2_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__WAIT_TIME2);
	IoReg_Write32(KME_DM_TOP0__MV04_START_ADDRESS2_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__MV04_START_ADDRESS2);
	IoReg_Write32(KME_DM_TOP0__MV04_END_ADDRESS2_ADDR , vpq_memc_suspend_resume.KME_DM_TOP0__MV04_END_ADDRESS2);
	//KME_DM_TOP1
	IoReg_Write32(KME_DM_TOP1__KME_08_START_ADDRESS0_ADDR		 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_08_START_ADDRESS0		);
	IoReg_Write32(KME_DM_TOP1__KME_08_START_ADDRESS1_ADDR		 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_08_START_ADDRESS1		);
	IoReg_Write32(KME_DM_TOP1__KME_08_END_ADDRESS0_ADDR			 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_08_END_ADDRESS0			);
	IoReg_Write32(KME_DM_TOP1__KME_08_END_ADDRESS1_ADDR			 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_08_END_ADDRESS1			);
	IoReg_Write32(KME_DM_TOP1__KME_08_LINE_OFFSET_ADDR_ADDR		 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_08_LINE_OFFSET_ADDR		);
	IoReg_Write32(KME_DM_TOP1__KME_08_AGENT_ADDR				 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_08_AGENT				);
	IoReg_Write32(KME_DM_TOP1__KME_08_THRESHOLD_ADDR			 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_08_THRESHOLD			);
	IoReg_Write32(KME_DM_TOP1__KME_09_AGENT_ADDR				 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_09_AGENT				);
	IoReg_Write32(KME_DM_TOP1__KME_09_THRESHOLD_ADDR			 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_09_THRESHOLD			);
	IoReg_Write32(KME_DM_TOP1__KME_10_START_ADDRESS0_ADDR		 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_10_START_ADDRESS0		);
	IoReg_Write32(KME_DM_TOP1__KME_10_START_ADDRESS1_ADDR		 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_10_START_ADDRESS1		);
	IoReg_Write32(KME_DM_TOP1__KME_10_END_ADDRESS0_ADDR			 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_10_END_ADDRESS0			);
	IoReg_Write32(KME_DM_TOP1__KME_10_END_ADDRESS1_ADDR			 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_10_END_ADDRESS1			);
	IoReg_Write32(KME_DM_TOP1__KME_10_LINE_OFFSET_ADDR_ADDR		 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_10_LINE_OFFSET_ADDR		);
	IoReg_Write32(KME_DM_TOP1__KME_10_AGENT_ADDR				 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_10_AGENT				);
	IoReg_Write32(KME_DM_TOP1__KME_10_THRESHOLD_ADDR			 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_10_THRESHOLD			);
	IoReg_Write32(KME_DM_TOP1__KME_11_AGENT_ADDR				 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_11_AGENT				);
	IoReg_Write32(KME_DM_TOP1__KME_11_THRESHOLD_ADDR			 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_11_THRESHOLD			);
	IoReg_Write32(KME_DM_TOP1__KME_12_START_ADDRESS0_ADDR		 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_12_START_ADDRESS0		);
	IoReg_Write32(KME_DM_TOP1__KME_12_START_ADDRESS1_ADDR		 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_12_START_ADDRESS1		);
	IoReg_Write32(KME_DM_TOP1__KME_12_END_ADDRESS0_ADDR			 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_12_END_ADDRESS0			);
	IoReg_Write32(KME_DM_TOP1__KME_12_END_ADDRESS1_ADDR			 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_12_END_ADDRESS1			);
	IoReg_Write32(KME_DM_TOP1__KME_12_LINE_OFFSET_ADDR_ADDR		 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_12_LINE_OFFSET_ADDR		);
	IoReg_Write32(KME_DM_TOP1__KME_12_AGENT_ADDR				 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_12_AGENT				);
	IoReg_Write32(KME_DM_TOP1__KME_12_THRESHOLD_ADDR			 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_12_THRESHOLD			);
	IoReg_Write32(KME_DM_TOP1__KME_13_AGENT_ADDR				 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_13_AGENT				);
	IoReg_Write32(KME_DM_TOP1__KME_13_THRESHOLD_ADDR			 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_13_THRESHOLD			);
	IoReg_Write32(KME_DM_TOP1__KME_14_START_ADDRESS0_ADDR		 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_14_START_ADDRESS0		);
	IoReg_Write32(KME_DM_TOP1__KME_14_START_ADDRESS1_ADDR		 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_14_START_ADDRESS1		);
	IoReg_Write32(KME_DM_TOP1__KME_14_END_ADDRESS0_ADDR			 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_14_END_ADDRESS0			);
	IoReg_Write32(KME_DM_TOP1__KME_14_END_ADDRESS1_ADDR			 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_14_END_ADDRESS1			);
	IoReg_Write32(KME_DM_TOP1__KME_14_LINE_OFFSET_ADDR_ADDR		 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_14_LINE_OFFSET_ADDR		);
	IoReg_Write32(KME_DM_TOP1__KME_14_AGENT_ADDR				 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_14_AGENT				);
	IoReg_Write32(KME_DM_TOP1__KME_14_THRESHOLD_ADDR			 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_14_THRESHOLD			);
	IoReg_Write32(KME_DM_TOP1__KME_15_AGENT_ADDR				 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_15_AGENT				);
	IoReg_Write32(KME_DM_TOP1__KME_15_THRESHOLD_ADDR			 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_15_THRESHOLD			);
	IoReg_Write32(KME_DM_TOP1__KME_IPLOGO_RESOLUTION_ADDR		 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_IPLOGO_RESOLUTION		);
	IoReg_Write32(KME_DM_TOP1__KME_MLOGO_RESOLUTION_ADDR		 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_MLOGO_RESOLUTION		);
	IoReg_Write32(KME_DM_TOP1__KME_PLOGO_RESOLUTION_ADDR		 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_PLOGO_RESOLUTION		);
	IoReg_Write32(KME_DM_TOP1__KME_HLOGO_RESOLUTION_ADDR		 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_HLOGO_RESOLUTION		);
	IoReg_Write32(KME_DM_TOP1__KME_LOGO_HNUM_ADDR				 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_LOGO_HNUM				);
	IoReg_Write32(KME_DM_TOP1__KME_CLR_ADDR						 , vpq_memc_suspend_resume.KME_DM_TOP1__KME_CLR						);
	IoReg_Write32(KME_DM_TOP1__TH0_ADDR							 , vpq_memc_suspend_resume.KME_DM_TOP1__TH0							);
	IoReg_Write32(KME_DM_TOP1__TH1_ADDR							 , vpq_memc_suspend_resume.KME_DM_TOP1__TH1							);
	IoReg_Write32(KME_DM_TOP1__TH2_ADDR							 , vpq_memc_suspend_resume.KME_DM_TOP1__TH2							);
	IoReg_Write32(KME_DM_TOP1__TH3_ADDR							 , vpq_memc_suspend_resume.KME_DM_TOP1__TH3							);
	IoReg_Write32(KME_DM_TOP1__TH4_ADDR							 , vpq_memc_suspend_resume.KME_DM_TOP1__TH4							);
	IoReg_Write32(KME_DM_TOP1__METER_TIMER1_ADDR				 , vpq_memc_suspend_resume.KME_DM_TOP1__METER_TIMER1				);
	//KME_DM_TOP2
	IoReg_Write32(KME_DM_TOP2__MV01_START_ADDRESS0_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV01_START_ADDRESS0);
	IoReg_Write32(KME_DM_TOP2__MV01_END_ADDRESS0_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV01_END_ADDRESS0);
	IoReg_Write32(KME_DM_TOP2__MV_01_LINE_OFFSET_ADDR_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV_01_LINE_OFFSET_ADDR);
	IoReg_Write32(KME_DM_TOP2__MV01_LR_OFFSET_ADDR_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV01_LR_OFFSET_ADDR);
	IoReg_Write32(KME_DM_TOP2__MV01_AGENT_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV01_AGENT);
	IoReg_Write32(KME_DM_TOP2__MV02_START_ADDRESS11_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV02_START_ADDRESS11);
	IoReg_Write32(KME_DM_TOP2__MV02_END_ADDRESS0_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV02_END_ADDRESS0);
	IoReg_Write32(KME_DM_TOP2__MV_02_LINE_OFFSET_ADDR_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV_02_LINE_OFFSET_ADDR);
	IoReg_Write32(KME_DM_TOP2__MV02_LR_OFFSET_ADDR_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV02_LR_OFFSET_ADDR);
	IoReg_Write32(KME_DM_TOP2__MV02_AGENT_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV02_AGENT);
	IoReg_Write32(KME_DM_TOP2__MV03_AGENT_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV03_AGENT);
	IoReg_Write32(KME_DM_TOP2__MV04_START_ADDRESS0_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV04_START_ADDRESS0);
	IoReg_Write32(KME_DM_TOP2__MV04_START_ADDRESS1_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV04_START_ADDRESS1);
	IoReg_Write32(KME_DM_TOP2__MV04_END_ADDRESS0_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV04_END_ADDRESS0);
	IoReg_Write32(KME_DM_TOP2__MV04_END_ADDRESS1_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV04_END_ADDRESS1);
	IoReg_Write32(KME_DM_TOP2__MV_04_LINE_OFFSET_ADDR_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV_04_LINE_OFFSET_ADDR);
	IoReg_Write32(KME_DM_TOP2__MV04_LR_OFFSET_ADDR_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV04_LR_OFFSET_ADDR);
	IoReg_Write32(KME_DM_TOP2__MV04_AGENT_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV04_AGENT);
	IoReg_Write32(KME_DM_TOP2__MV05_START_ADDRESS0_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV05_START_ADDRESS0);
	IoReg_Write32(KME_DM_TOP2__MV05_START_ADDRESS1_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV05_START_ADDRESS1);
	IoReg_Write32(KME_DM_TOP2__MV05_END_ADDRESS0_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV05_END_ADDRESS0);
	IoReg_Write32(KME_DM_TOP2__MV05_END_ADDRESS1_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV05_END_ADDRESS1);
	IoReg_Write32(KME_DM_TOP2__MV_05_LINE_OFFSET_ADDR_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV_05_LINE_OFFSET_ADDR);
	IoReg_Write32(KME_DM_TOP2__MV05_LR_OFFSET_ADDR_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV05_LR_OFFSET_ADDR);
	IoReg_Write32(KME_DM_TOP2__MV05_AGENT_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV05_AGENT);
	IoReg_Write32(KME_DM_TOP2__MV06_AGENT_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV06_AGENT);
	IoReg_Write32(KME_DM_TOP2__MV07_AGENT_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV07_AGENT);
	IoReg_Write32(KME_DM_TOP2__MV08_AGENT_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV08_AGENT);
	IoReg_Write32(KME_DM_TOP2__MV09_AGENT_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV09_AGENT);
	IoReg_Write32(KME_DM_TOP2__MV10_AGENT_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV10_AGENT);
	IoReg_Write32(KME_DM_TOP2__MV11_AGENT_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV11_AGENT);
	IoReg_Write32(KME_DM_TOP2__KME_00_START_ADDRESS6_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__KME_00_START_ADDRESS6);
	IoReg_Write32(KME_DM_TOP2__KME_00_START_ADDRESS7_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__KME_00_START_ADDRESS7);
	IoReg_Write32(KME_DM_TOP2__KME_00_END_ADDRESS6_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__KME_00_END_ADDRESS6);
	IoReg_Write32(KME_DM_TOP2__KME_00_END_ADDRESS7_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__KME_00_END_ADDRESS7);
	IoReg_Write32(KME_DM_TOP2__MV_SOURCE_MODE_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV_SOURCE_MODE);
	IoReg_Write32(KME_DM_TOP2__MV01_RESOLUTION_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV01_RESOLUTION);
	IoReg_Write32(KME_DM_TOP2__MV02_RESOLUTION_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV02_RESOLUTION);
	IoReg_Write32(KME_DM_TOP2__MV04_RESOLUTION_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV04_RESOLUTION);
	IoReg_Write32(KME_DM_TOP2__MV05_RESOLUTION_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV05_RESOLUTION);
	IoReg_Write32(KME_DM_TOP2__MV_TIME_ENABLE_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV_TIME_ENABLE);
	IoReg_Write32(KME_DM_TOP2__MV_HOLD_TIME0_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV_HOLD_TIME0);
	IoReg_Write32(KME_DM_TOP2__MV_HOLD_TIME1_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV_HOLD_TIME1);
	IoReg_Write32(KME_DM_TOP2__MV_HOLD_TIME2_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV_HOLD_TIME2);
	IoReg_Write32(KME_DM_TOP2__MV_WAIT_TIME0_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV_WAIT_TIME0);
	IoReg_Write32(KME_DM_TOP2__MV_WAIT_TIME1_ADDR , vpq_memc_suspend_resume.KME_DM_TOP2__MV_WAIT_TIME1);
	//KME_DEHALO3
	IoReg_Write32(KME_DEHALO3__KME_DEHALO3_0C_ADDR , vpq_memc_suspend_resume.KME_DEHALO3__KME_DEHALO3_0C);
	//KME_ME1_TOP0
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_00_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_00);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_04_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_04);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_08_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_08);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_0C_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_0C);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_10_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_10);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_14_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_14);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_18_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_18);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_1C_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_1C);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_20_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_20);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_24_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_24);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_28_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_28);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_2C_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_2C);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_30_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_30);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_34_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_34);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_38_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_38);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_40_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_40);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_44_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_44);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_48_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_48);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_4C_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_4C);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_50_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_50);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_54_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_54);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_58_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_58);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_5C_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_5C);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_60_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_60);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_64_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_64);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_68_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_68);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_6C_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_6C);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_70_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_70);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_74_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_74);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_78_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_78);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_7C_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_7C);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_80_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_80);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_84_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_84);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_88_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_88);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_8C_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_8C);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_90_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_90);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_94_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_94);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_98_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_98);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_9C_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_9C);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_A0_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_A0);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_A4_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_A4);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_B0_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_B0);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_B4_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_B4);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_B8_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_B8);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_BC_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_BC);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_C0_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_C0);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_C4_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_C4);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_EC_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_EC);
	IoReg_Write32(KME_ME1_TOP0__KME_ME1_TOP0_F0_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP0__KME_ME1_TOP0_F0);
	//KME_ME1_TOP1
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_00_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_00);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_01_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_01);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_02_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_02);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_03_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_03);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_04_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_04);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_05_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_05);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_06_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_06);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_07_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_07);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_08_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_08);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_09_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_09);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_10_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_10);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_11_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_11);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_12_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_12);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_13_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_13);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_14_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_14);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_15_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_15);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_16_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_16);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_17_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_17);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_18_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_18);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_19_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_19);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_20_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_20);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_21_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_21);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_22_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_22);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_23_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_23);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_24_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_24);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_25_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_25);
	IoReg_Write32(KME_ME1_TOP1__KME_ME1_TOP1_68_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__KME_ME1_TOP1_68);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_27_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_27);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_28_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_28);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_29_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_29);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_30_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_30);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_31_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_31);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_32_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_32);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_33_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_33);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_34_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_34);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_35_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_35);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_36_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_36);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_37_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_37);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_38_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_38);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_39_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_39);
	IoReg_Write32(KME_ME1_TOP1__ME1_COMMON1_40_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_COMMON1_40);
	IoReg_Write32(KME_ME1_TOP1__ME1_SCENE1_00_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_SCENE1_00);
	IoReg_Write32(KME_ME1_TOP1__ME1_SCENE1_01_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_SCENE1_01);
	IoReg_Write32(KME_ME1_TOP1__ME1_SCENE1_02_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_SCENE1_02);
	IoReg_Write32(KME_ME1_TOP1__ME1_SCENE1_03_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_SCENE1_03);
	IoReg_Write32(KME_ME1_TOP1__ME1_SCENE1_04_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_SCENE1_04);
	IoReg_Write32(KME_ME1_TOP1__ME1_SCENE1_05_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_SCENE1_05);
	IoReg_Write32(KME_ME1_TOP1__ME1_SCENE1_06_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_SCENE1_06);
	IoReg_Write32(KME_ME1_TOP1__ME1_SCENE1_07_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_SCENE1_07);
	IoReg_Write32(KME_ME1_TOP1__ME1_SCENE1_08_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_SCENE1_08);
	IoReg_Write32(KME_ME1_TOP1__ME1_SCENE1_09_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_SCENE1_09);
	IoReg_Write32(KME_ME1_TOP1__ME1_SCENE1_10_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_SCENE1_10);
	IoReg_Write32(KME_ME1_TOP1__ME1_SCENE1_11_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_SCENE1_11);
	IoReg_Write32(KME_ME1_TOP1__ME1_SCENE1_12_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__ME1_SCENE1_12);
	IoReg_Write32(KME_ME1_TOP1__KME_ME1_TOP1_F4_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP1__KME_ME1_TOP1_F4);
	//KME_LBME2_TOP
	IoReg_Write32(KME_LBME2_TOP__KME_LBME2_TOP_00_ADDR , vpq_memc_suspend_resume.KME_LBME2_TOP__KME_LBME2_TOP_00);
	IoReg_Write32(KME_LBME2_TOP__KME_LBME2_TOP_04_ADDR , vpq_memc_suspend_resume.KME_LBME2_TOP__KME_LBME2_TOP_04);
	IoReg_Write32(KME_LBME2_TOP__KME_LBME2_TOP_08_ADDR , vpq_memc_suspend_resume.KME_LBME2_TOP__KME_LBME2_TOP_08);
	IoReg_Write32(KME_LBME2_TOP__KME_LBME2_TOP_0C_ADDR , vpq_memc_suspend_resume.KME_LBME2_TOP__KME_LBME2_TOP_0C);
	IoReg_Write32(KME_LBME2_TOP__KME_LBME2_TOP_10_ADDR , vpq_memc_suspend_resume.KME_LBME2_TOP__KME_LBME2_TOP_10);
	IoReg_Write32(KME_LBME2_TOP__KME_LBME2_TOP_14_ADDR , vpq_memc_suspend_resume.KME_LBME2_TOP__KME_LBME2_TOP_14);
	IoReg_Write32(KME_LBME2_TOP__KME_LBME2_TOP_18_ADDR , vpq_memc_suspend_resume.KME_LBME2_TOP__KME_LBME2_TOP_18);
	//KME_ME2_VBUF_TOP
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_00_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_00);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_04_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_04);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_08_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_08);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_0C_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_0C);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_10_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_10);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_14_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_14);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_18_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_18);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_1C_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_1C);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_20_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_20);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_24_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_24);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_28_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_28);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_2C_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_2C);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_30_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_30);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_34_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_34);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_38_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_38);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_3C_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_3C);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_40_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_40);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_44_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_44);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_48_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_48);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_4C_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_4C);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_50_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_50);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_54_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_54);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_58_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_58);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_5C_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_5C);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_60_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_60);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_64_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_64);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_68_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_68);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_6C_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_6C);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_70_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_70);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_74_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_74);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_78_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_78);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_7C_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_7C);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_80_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_80);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_84_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_84);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_88_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_88);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_8C_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_8C);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_90_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_90);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_94_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_94);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_98_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_98);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_9C_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_9C);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_A0_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_A0);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_A4_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_A4);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_A8_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_A8);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_AC_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_AC);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_B0_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_B0);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_B4_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_B4);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_B8_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_B8);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_BC_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_BC);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_C0_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_C0);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_C4_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_C4);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_C8_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_C8);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_CC_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_CC);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_D0_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_D0);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_D4_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_D4);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_D8_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_D8);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_DC_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_DC);
	IoReg_Write32(KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_E0_ADDR , vpq_memc_suspend_resume.KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_E0);
	//KME_ME2_CALC0
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_00_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_00);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_04_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_04);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_08_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_08);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_0C_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_0C);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_10_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_10);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_14_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_14);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_18_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_18);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_1C_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_1C);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_20_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_20);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_24_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_24);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_28_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_28);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_2C_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_2C);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_30_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_30);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_34_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_34);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_38_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_38);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_3C_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_3C);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_40_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_40);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_44_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_44);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_48_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_48);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_4C_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_4C);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_50_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_50);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_54_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_54);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_60_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_60);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_64_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_64);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_68_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_68);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_6C_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_6C);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_70_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_70);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_74_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_74);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_78_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_78);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_7C_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_7C);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_80_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_80);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_84_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_84);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_88_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_88);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_8C_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_8C);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_90_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_90);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_94_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_94);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_98_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_98);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_9C_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_9C);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_A0_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_A0);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_A4_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_A4);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_A8_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_A8);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_AC_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_AC);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_B0_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_B0);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_B4_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_B4);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_B8_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_B8);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_BC_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_BC);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_C0_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_C0);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_C4_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_C4);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_C8_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_C8);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_CC_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_CC);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_D0_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_D0);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_D4_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_D4);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_D8_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_D8);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_DC_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_DC);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_E0_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_E0);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_E4_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_E4);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_E8_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_E8);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_EC_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_EC);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_F0_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_F0);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_F4_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_F4);
	IoReg_Write32(KME_ME2_CALC0__KME_ME2_CALC0_F8_ADDR , vpq_memc_suspend_resume.KME_ME2_CALC0__KME_ME2_CALC0_F8);
	//KME_ME1_TOP6
	IoReg_Write32(KME_ME1_TOP6__KME_ME1_TOP6_00_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP6__KME_ME1_TOP6_00);
	IoReg_Write32(KME_ME1_TOP6__KME_ME1_TOP6_04_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP6__KME_ME1_TOP6_04);
	IoReg_Write32(KME_ME1_TOP6__KME_ME1_TOP6_08_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP6__KME_ME1_TOP6_08);
	IoReg_Write32(KME_ME1_TOP6__KME_ME1_TOP6_0C_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP6__KME_ME1_TOP6_0C);
	IoReg_Write32(KME_ME1_TOP6__KME_ME1_TOP6_10_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP6__KME_ME1_TOP6_10);
	IoReg_Write32(KME_ME1_TOP6__KME_ME1_TOP6_14_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP6__KME_ME1_TOP6_14);
	IoReg_Write32(KME_ME1_TOP6__KME_ME1_TOP6_18_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP6__KME_ME1_TOP6_18);
	IoReg_Write32(KME_ME1_TOP6__KME_ME1_TOP6_1C_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP6__KME_ME1_TOP6_1C);
	IoReg_Write32(KME_ME1_TOP6__KME_ME1_TOP6_20_ADDR , vpq_memc_suspend_resume.KME_ME1_TOP6__KME_ME1_TOP6_20);
	//KME_ME1_TOP7
	//KME_ME1_TOP8
	//KME_ME1_TOP9
	//KME_DEHALO
	IoReg_Write32(KME_DEHALO__KME_DEHALO_10_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_10);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_14_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_14);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_18_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_18);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_1C_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_1C);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_20_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_20);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_24_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_24);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_28_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_28);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_2C_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_2C);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_30_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_30);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_34_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_34);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_38_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_38);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_3C_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_3C);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_40_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_40);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_44_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_44);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_48_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_48);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_4C_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_4C);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_50_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_50);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_54_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_54);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_58_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_58);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_5C_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_5C);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_60_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_60);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_64_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_64);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_68_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_68);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_6C_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_6C);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_70_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_70);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_74_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_74);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_78_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_78);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_7C_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_7C);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_80_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_80);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_84_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_84);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_88_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_88);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_8C_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_8C);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_90_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_90);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_94_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_94);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_A8_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_A8);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_AC_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_AC);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_B0_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_B0);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_B4_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_B4);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_B8_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_B8);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_BC_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_BC);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_C0_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_C0);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_C4_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_C4);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_C8_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_C8);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_CC_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_CC);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_D0_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_D0);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_D4_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_D4);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_D8_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_D8);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_DC_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_DC);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_E0_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_E0);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_E4_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_E4);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_E8_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_E8);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_EC_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_EC);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_F0_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_F0);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_F4_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_F4);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_F8_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_F8);
	IoReg_Write32(KME_DEHALO__KME_DEHALO_FC_ADDR , vpq_memc_suspend_resume.KME_DEHALO__KME_DEHALO_FC);
	//KME_DEHALO2
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_08_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_08);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_10_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_10);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_14_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_14);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_18_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_18);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_1C_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_1C);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_20_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_20);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_24_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_24);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_28_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_28);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_2C_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_2C);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_30_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_30);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_34_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_34);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_38_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_38);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_3C_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_3C);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_40_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_40);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_44_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_44);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_48_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_48);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_4C_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_4C);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_50_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_50);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_54_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_54);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_58_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_58);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_74_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_74);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_78_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_78);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_7C_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_7C);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_80_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_80);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_84_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_84);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_88_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_88);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_8C_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_8C);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_90_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_90);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_94_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_94);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_98_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_98);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_9C_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_9C);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_A0_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_A0);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_A4_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_A4);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_A8_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_A8);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_AC_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_AC);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_B0_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_B0);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_B4_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_B4);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_B8_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_B8);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_BC_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_BC);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_C0_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_C0);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_C4_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_C4);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_C8_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_C8);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_CC_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_CC);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_D0_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_D0);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_D4_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_D4);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_D8_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_D8);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_DC_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_DC);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_E0_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_E0);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_E4_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_E4);
	IoReg_Write32(KME_DEHALO2__KME_DEHALO2_E8_ADDR , vpq_memc_suspend_resume.KME_DEHALO2__KME_DEHALO2_E8);
	//KPOST_TOP
	IoReg_Write32(KPOST_TOP__KPOST_TOP_00_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_00);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_04_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_04);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_08_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_08);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_0C_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_0C);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_10_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_10);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_14_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_14);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_18_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_18);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_1C_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_1C);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_20_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_20);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_24_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_24);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_28_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_28);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_30_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_30);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_34_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_34);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_38_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_38);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_3C_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_3C);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_40_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_40);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_44_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_44);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_48_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_48);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_4C_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_4C);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_50_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_50);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_60_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_60);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_64_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_64);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_68_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_68);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_6C_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_6C);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_80_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_80);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_84_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_84);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_88_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_88);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_8C_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_8C);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_90_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_90);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_94_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_94);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_A0_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_A0);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_A4_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_A4);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_A8_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_A8);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_AC_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_AC);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_B0_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_B0);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_B4_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_B4);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_B8_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_B8);
	IoReg_Write32(KPOST_TOP__KPOST_TOP_BC_ADDR , vpq_memc_suspend_resume.KPOST_TOP__KPOST_TOP_BC);
	//CRTC1
	IoReg_Write32(CRTC1__CRTC1_00_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_00);
	IoReg_Write32(CRTC1__CRTC1_04_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_04);
	IoReg_Write32(CRTC1__CRTC1_08_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_08);
	IoReg_Write32(CRTC1__CRTC1_0C_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_0C);
	IoReg_Write32(CRTC1__CRTC1_10_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_10);
	IoReg_Write32(CRTC1__CRTC1_14_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_14);
	IoReg_Write32(CRTC1__CRTC1_18_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_18);
	IoReg_Write32(CRTC1__CRTC1_1C_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_1C);
	IoReg_Write32(CRTC1__CRTC1_20_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_20);
	IoReg_Write32(CRTC1__CRTC1_24_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_24);
	IoReg_Write32(CRTC1__CRTC1_28_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_28);
	IoReg_Write32(CRTC1__CRTC1_2C_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_2C);
	IoReg_Write32(CRTC1__CRTC1_30_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_30);
	IoReg_Write32(CRTC1__CRTC1_34_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_34);
	IoReg_Write32(CRTC1__CRTC1_38_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_38);
	IoReg_Write32(CRTC1__CRTC1_3C_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_3C);
	IoReg_Write32(CRTC1__CRTC1_40_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_40);
	IoReg_Write32(CRTC1__CRTC1_44_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_44);
	IoReg_Write32(CRTC1__CRTC1_48_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_48);
	IoReg_Write32(CRTC1__CRTC1_60_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_60);
	IoReg_Write32(CRTC1__CRTC1_64_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_64);
	IoReg_Write32(CRTC1__CRTC1_68_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_68);
	IoReg_Write32(CRTC1__CRTC1_6C_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_6C);
	IoReg_Write32(CRTC1__CRTC1_70_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_70);
	IoReg_Write32(CRTC1__CRTC1_74_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_74);
	IoReg_Write32(CRTC1__CRTC1_78_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_78);
	IoReg_Write32(CRTC1__CRTC1_7C_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_7C);
	IoReg_Write32(CRTC1__CRTC1_80_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_80);
	IoReg_Write32(CRTC1__CRTC1_84_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_84);
	IoReg_Write32(CRTC1__CRTC1_88_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_88);
	IoReg_Write32(CRTC1__CRTC1_8C_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_8C);
	IoReg_Write32(CRTC1__CRTC1_90_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_90);
	IoReg_Write32(CRTC1__CRTC1_94_ADDR , vpq_memc_suspend_resume.CRTC1__CRTC1_94);
	//kphase
	IoReg_Write32(KPHASE__KPHASE_00_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_00);
	IoReg_Write32(KPHASE__KPHASE_04_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_04);
	IoReg_Write32(KPHASE__KPHASE_08_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_08);
	IoReg_Write32(KPHASE__KPHASE_0C_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_0C);
	IoReg_Write32(KPHASE__KPHASE_10_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_10);
	IoReg_Write32(KPHASE__KPHASE_14_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_14);
	IoReg_Write32(KPHASE__KPHASE_18_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_18);
	IoReg_Write32(KPHASE__KPHASE_1C_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_1C);
	IoReg_Write32(KPHASE__KPHASE_20_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_20);
	IoReg_Write32(KPHASE__KPHASE_24_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_24);
	IoReg_Write32(KPHASE__KPHASE_28_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_28);
	IoReg_Write32(KPHASE__KPHASE_2C_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_2C);
	IoReg_Write32(KPHASE__KPHASE_30_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_30);
	IoReg_Write32(KPHASE__KPHASE_34_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_34);
	IoReg_Write32(KPHASE__KPHASE_38_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_38);
	IoReg_Write32(KPHASE__KPHASE_3C_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_3C);
	IoReg_Write32(KPHASE__KPHASE_40_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_40);
	IoReg_Write32(KPHASE__KPHASE_44_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_44);
	IoReg_Write32(KPHASE__KPHASE_48_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_48);
	IoReg_Write32(KPHASE__KPHASE_4C_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_4C);
	IoReg_Write32(KPHASE__KPHASE_50_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_50);
	IoReg_Write32(KPHASE__KPHASE_54_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_54);
	IoReg_Write32(KPHASE__KPHASE_58_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_58);
	IoReg_Write32(KPHASE__KPHASE_5C_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_5C);
	IoReg_Write32(KPHASE__KPHASE_60_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_60);
	IoReg_Write32(KPHASE__KPHASE_64_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_64);
	IoReg_Write32(KPHASE__KPHASE_68_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_68);
	IoReg_Write32(KPHASE__KPHASE_6C_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_6C);
	IoReg_Write32(KPHASE__KPHASE_70_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_70);
	IoReg_Write32(KPHASE__KPHASE_74_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_74);
	IoReg_Write32(KPHASE__KPHASE_78_ADDR , vpq_memc_suspend_resume.KPHASE__KPHASE_78);
	//Hardware
	IoReg_Write32(HARDWARE__HARDWARE_00_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_00);
	IoReg_Write32(HARDWARE__HARDWARE_01_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_01);
	IoReg_Write32(HARDWARE__HARDWARE_02_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_02);
	IoReg_Write32(HARDWARE__HARDWARE_03_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_03);
	IoReg_Write32(HARDWARE__HARDWARE_04_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_04);
	IoReg_Write32(HARDWARE__HARDWARE_05_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_05);
	IoReg_Write32(HARDWARE__HARDWARE_06_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_06);
	IoReg_Write32(HARDWARE__HARDWARE_07_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_07);
	IoReg_Write32(HARDWARE__HARDWARE_08_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_08);
	IoReg_Write32(HARDWARE__HARDWARE_09_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_09);
	IoReg_Write32(HARDWARE__HARDWARE_10_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_10);
	IoReg_Write32(HARDWARE__HARDWARE_11_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_11);
	IoReg_Write32(HARDWARE__HARDWARE_12_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_12);
	IoReg_Write32(HARDWARE__HARDWARE_13_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_13);
	IoReg_Write32(HARDWARE__HARDWARE_14_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_14);
	IoReg_Write32(HARDWARE__HARDWARE_15_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_15);
	IoReg_Write32(HARDWARE__HARDWARE_16_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_16);
	IoReg_Write32(HARDWARE__HARDWARE_17_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_17);
	IoReg_Write32(HARDWARE__HARDWARE_18_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_18);
	IoReg_Write32(HARDWARE__HARDWARE_19_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_19);
	IoReg_Write32(HARDWARE__HARDWARE_20_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_20);
	IoReg_Write32(HARDWARE__HARDWARE_21_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_21);
	IoReg_Write32(HARDWARE__HARDWARE_22_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_22);
	IoReg_Write32(HARDWARE__HARDWARE_23_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_23);
	IoReg_Write32(HARDWARE__HARDWARE_24_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_24);
	IoReg_Write32(HARDWARE__HARDWARE_25_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_25);
	IoReg_Write32(HARDWARE__HARDWARE_26_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_26);
	IoReg_Write32(HARDWARE__HARDWARE_27_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_27);
	IoReg_Write32(HARDWARE__HARDWARE_28_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_28);
	IoReg_Write32(HARDWARE__HARDWARE_29_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_29);
	IoReg_Write32(HARDWARE__HARDWARE_30_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_30);
	IoReg_Write32(HARDWARE__HARDWARE_31_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_31);
	IoReg_Write32(HARDWARE__HARDWARE_32_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_32);
	IoReg_Write32(HARDWARE__HARDWARE_33_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_33);
	IoReg_Write32(HARDWARE__HARDWARE_34_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_34);
	IoReg_Write32(HARDWARE__HARDWARE_35_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_35);
	IoReg_Write32(HARDWARE__HARDWARE_36_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_36);
	IoReg_Write32(HARDWARE__HARDWARE_37_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_37);
	IoReg_Write32(HARDWARE__HARDWARE_38_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_38);
	IoReg_Write32(HARDWARE__HARDWARE_39_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_39);
	IoReg_Write32(HARDWARE__HARDWARE_40_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_40);
	IoReg_Write32(HARDWARE__HARDWARE_41_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_41);
	IoReg_Write32(HARDWARE__HARDWARE_42_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_42);
	IoReg_Write32(HARDWARE__HARDWARE_43_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_43);
	IoReg_Write32(HARDWARE__HARDWARE_44_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_44);
	IoReg_Write32(HARDWARE__HARDWARE_45_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_45);
	IoReg_Write32(HARDWARE__HARDWARE_46_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_46);
	IoReg_Write32(HARDWARE__HARDWARE_47_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_47);
	IoReg_Write32(HARDWARE__HARDWARE_48_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_48);
	IoReg_Write32(HARDWARE__HARDWARE_49_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_49);
	IoReg_Write32(HARDWARE__HARDWARE_50_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_50);
	IoReg_Write32(HARDWARE__HARDWARE_51_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_51);
	IoReg_Write32(HARDWARE__HARDWARE_52_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_52);
	IoReg_Write32(HARDWARE__HARDWARE_53_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_53);
	IoReg_Write32(HARDWARE__HARDWARE_54_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_54);
	IoReg_Write32(HARDWARE__HARDWARE_55_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_55);
	IoReg_Write32(HARDWARE__HARDWARE_56_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_56);
	IoReg_Write32(HARDWARE__HARDWARE_57_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_57);
	IoReg_Write32(HARDWARE__HARDWARE_58_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_58);
	IoReg_Write32(HARDWARE__HARDWARE_59_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_59);
	IoReg_Write32(HARDWARE__HARDWARE_60_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_60);
	IoReg_Write32(HARDWARE__HARDWARE_61_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_61);
	IoReg_Write32(HARDWARE__HARDWARE_62_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_62);
	IoReg_Write32(HARDWARE__HARDWARE_63_ADDR , vpq_memc_suspend_resume.HARDWARE__HARDWARE_63);
	//Software
	IoReg_Write32(SOFTWARE__SOFTWARE_00_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_00);
	IoReg_Write32(SOFTWARE__SOFTWARE_01_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_01);
	IoReg_Write32(SOFTWARE__SOFTWARE_02_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_02);
	IoReg_Write32(SOFTWARE__SOFTWARE_03_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_03);
	IoReg_Write32(SOFTWARE__SOFTWARE_04_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_04);
	IoReg_Write32(SOFTWARE__SOFTWARE_05_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_05);
	IoReg_Write32(SOFTWARE__SOFTWARE_06_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_06);
	IoReg_Write32(SOFTWARE__SOFTWARE_07_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_07);
	IoReg_Write32(SOFTWARE__SOFTWARE_08_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_08);
	IoReg_Write32(SOFTWARE__SOFTWARE_09_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_09);
	IoReg_Write32(SOFTWARE__SOFTWARE_10_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_10);
	IoReg_Write32(SOFTWARE__SOFTWARE_11_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_11);
	IoReg_Write32(SOFTWARE__SOFTWARE_12_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_12);
	IoReg_Write32(SOFTWARE__SOFTWARE_13_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_13);
	IoReg_Write32(SOFTWARE__SOFTWARE_14_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_14);
	IoReg_Write32(SOFTWARE__SOFTWARE_15_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_15);
	IoReg_Write32(SOFTWARE__SOFTWARE_16_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_16);
	IoReg_Write32(SOFTWARE__SOFTWARE_17_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_17);
	IoReg_Write32(SOFTWARE__SOFTWARE_18_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_18);
	IoReg_Write32(SOFTWARE__SOFTWARE_19_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_19);
	IoReg_Write32(SOFTWARE__SOFTWARE_20_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_20);
	IoReg_Write32(SOFTWARE__SOFTWARE_21_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_21);
	IoReg_Write32(SOFTWARE__SOFTWARE_22_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_22);
	IoReg_Write32(SOFTWARE__SOFTWARE_23_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_23);
	IoReg_Write32(SOFTWARE__SOFTWARE_24_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_24);
	IoReg_Write32(SOFTWARE__SOFTWARE_25_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_25);
	IoReg_Write32(SOFTWARE__SOFTWARE_26_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_26);
	IoReg_Write32(SOFTWARE__SOFTWARE_27_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_27);
	IoReg_Write32(SOFTWARE__SOFTWARE_28_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_28);
	IoReg_Write32(SOFTWARE__SOFTWARE_29_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_29);
	IoReg_Write32(SOFTWARE__SOFTWARE_30_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_30);
	IoReg_Write32(SOFTWARE__SOFTWARE_31_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_31);
	IoReg_Write32(SOFTWARE__SOFTWARE_32_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_32);
	IoReg_Write32(SOFTWARE__SOFTWARE_33_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_33);
	IoReg_Write32(SOFTWARE__SOFTWARE_34_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_34);
	IoReg_Write32(SOFTWARE__SOFTWARE_35_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_35);
	IoReg_Write32(SOFTWARE__SOFTWARE_36_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_36);
	IoReg_Write32(SOFTWARE__SOFTWARE_37_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_37);
	IoReg_Write32(SOFTWARE__SOFTWARE_38_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_38);
	IoReg_Write32(SOFTWARE__SOFTWARE_39_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_39);
	IoReg_Write32(SOFTWARE__SOFTWARE_40_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_40);
	IoReg_Write32(SOFTWARE__SOFTWARE_41_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_41);
	IoReg_Write32(SOFTWARE__SOFTWARE_42_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_42);
	IoReg_Write32(SOFTWARE__SOFTWARE_43_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_43);
	IoReg_Write32(SOFTWARE__SOFTWARE_44_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_44);
	IoReg_Write32(SOFTWARE__SOFTWARE_45_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_45);
	IoReg_Write32(SOFTWARE__SOFTWARE_46_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_46);
	IoReg_Write32(SOFTWARE__SOFTWARE_47_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_47);
	IoReg_Write32(SOFTWARE__SOFTWARE_48_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_48);
	IoReg_Write32(SOFTWARE__SOFTWARE_49_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_49);
	IoReg_Write32(SOFTWARE__SOFTWARE_50_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_50);
	IoReg_Write32(SOFTWARE__SOFTWARE_51_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_51);
	IoReg_Write32(SOFTWARE__SOFTWARE_52_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_52);
	IoReg_Write32(SOFTWARE__SOFTWARE_53_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_53);
	IoReg_Write32(SOFTWARE__SOFTWARE_54_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_54);
	IoReg_Write32(SOFTWARE__SOFTWARE_55_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_55);
	IoReg_Write32(SOFTWARE__SOFTWARE_56_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_56);
	IoReg_Write32(SOFTWARE__SOFTWARE_57_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_57);
	IoReg_Write32(SOFTWARE__SOFTWARE_58_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_58);
	IoReg_Write32(SOFTWARE__SOFTWARE_59_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_59);
	IoReg_Write32(SOFTWARE__SOFTWARE_60_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_60);
	IoReg_Write32(SOFTWARE__SOFTWARE_61_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_61);
	IoReg_Write32(SOFTWARE__SOFTWARE_62_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_62);
	IoReg_Write32(SOFTWARE__SOFTWARE_63_ADDR , vpq_memc_suspend_resume.SOFTWARE__SOFTWARE_63);
	//Software1
	IoReg_Write32(SOFTWARE1__SOFTWARE1_00_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_00);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_01_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_01);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_02_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_02);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_03_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_03);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_04_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_04);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_05_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_05);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_06_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_06);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_07_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_07);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_08_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_08);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_09_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_09);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_10_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_10);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_11_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_11);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_12_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_12);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_13_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_13);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_14_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_14);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_15_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_15);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_16_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_16);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_17_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_17);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_18_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_18);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_19_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_19);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_20_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_20);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_21_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_21);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_22_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_22);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_23_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_23);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_24_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_24);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_25_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_25);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_26_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_26);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_27_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_27);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_28_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_28);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_29_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_29);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_30_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_30);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_31_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_31);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_32_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_32);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_33_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_33);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_34_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_34);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_35_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_35);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_36_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_36);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_37_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_37);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_38_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_38);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_39_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_39);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_40_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_40);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_41_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_41);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_42_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_42);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_43_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_43);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_44_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_44);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_45_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_45);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_46_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_46);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_47_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_47);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_48_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_48);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_49_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_49);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_50_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_50);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_51_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_51);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_52_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_52);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_53_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_53);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_54_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_54);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_55_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_55);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_56_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_56);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_57_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_57);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_58_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_58);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_59_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_59);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_60_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_60);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_61_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_61);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_62_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_62);
	IoReg_Write32(SOFTWARE1__SOFTWARE1_63_ADDR , vpq_memc_suspend_resume.SOFTWARE1__SOFTWARE1_63);

#if 0
	//MEMC mux
	IoReg_Write32(MEMC_MUX_MEMC_MUX_FUNC_CTRL_reg, vpq_memc_suspend_resume.MEMC_MUX_MEMC_MUX_FUNC_CTRL_SUSPEND);
	//FBG
	vpq_memc_suspend_resume.PPOVERLAY_MEMC_MUX_CTRL_SUSPEND = vpq_memc_suspend_resume.PPOVERLAY_MEMC_MUX_CTRL_SUSPEND | 0x00001000;
	IoReg_Write32(PPOVERLAY_MEMC_MUX_CTRL_reg, vpq_memc_suspend_resume.PPOVERLAY_MEMC_MUX_CTRL_SUSPEND);
	//clock
	IoReg_Write32(SYS_REG_SYS_DISPCLKSEL_reg, vpq_memc_suspend_resume.SYS_REG_SYS_DISPCLKSEL_SUSPEND);
	IoReg_Write32(MEMC_LATENCY01_ADDR, vpq_memc_suspend_resume.MEMC_LATENCY01);
	IoReg_Write32(MEMC_LATENCY02_ADDR, vpq_memc_suspend_resume.MEMC_LATENCY02);

	//ME memory boundary
	IoReg_Write32(MEMC_DBUS_WRAP_Me_downlimit_addr_reg, vpq_memc_suspend_resume.MEMC_DBUS_WRAP_ME_DOWNLIMIT);
	IoReg_Write32(MEMC_DBUS_WRAP_Me_uplimit_addr_reg, vpq_memc_suspend_resume.MEMC_DBUS_WRAP_ME_UPLIMIT);
	//MC memory boundary
	IoReg_Write32(MEMC_DBUS_WRAP_Mc_downlimit_addr_reg, vpq_memc_suspend_resume.MEMC_DBUS_WRAP_MC_DOWNLIMIT);
	IoReg_Write32(MEMC_DBUS_WRAP_Mc_uplimit_addr_reg, vpq_memc_suspend_resume.MEMC_DBUS_WRAP_MC_UPLIMIT);
#else
	//MEMC resume setting
	Scaler_MEMC_Mux();
	Scaler_MEMC_CLK();
	Scaler_MEMC_output_force_bg_enable(TRUE);

	//disable double buffer
	{
	ppoverlay_double_buffer_ctrl2_RBUS double_buffer_ctrl_reg;
	double_buffer_ctrl_reg.regValue =  IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	double_buffer_ctrl_reg.uzudtgreg_dbuf_en = 0;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl_reg.regValue);
	}

	//ME memory boundary
	IoReg_Write32(MEMC_DBUS_WRAP_Me_downlimit_addr_reg, gphy_addr_kme);
	IoReg_Write32(MEMC_DBUS_WRAP_Me_uplimit_addr_reg, (gphy_addr_kme + KME_TOTAL_SIZE));
	//MC memory boundary
	IoReg_Write32(MEMC_DBUS_WRAP_Mc_downlimit_addr_reg, gphy_addr_kmc00);
	IoReg_Write32(MEMC_DBUS_WRAP_Mc_uplimit_addr_reg, (gphy_addr_kmc00 + KMC_TOTAL_SIZE));
#endif
	MEMC_instanboot_resume_Done = 1;

	// enable interrupt
	UINT32 u32_interrupt_reg = 0;
	//reg_kmc_int_en
	u32_interrupt_reg = IoReg_Read32(KMC_TOP__KMC_TOP_18_ADDR);
	u32_interrupt_reg |= 0x02000000;
	IoReg_Write32(KMC_TOP__KMC_TOP_18_ADDR, u32_interrupt_reg);

	//reg_post_int_en
	u32_interrupt_reg = IoReg_Read32(KPOST_TOP__KPOST_TOP_08_ADDR);
	u32_interrupt_reg |= 0x00000040;
	IoReg_Write32(KPOST_TOP__KPOST_TOP_08_ADDR, u32_interrupt_reg);

	//rtd_pr_memc_notice("[MEMC]Instanboot Resume done !!!\n");
	rtd_pr_memc_notice( "[MEMC]Instanboot Resume done\n");
#endif //CONFIG_HW_SUPPORT_MEMC
}
#endif

unsigned char memc_ioctl_cmd_stop[32] = {0};
unsigned char vpq_memc_ioctl_get_stop_run(unsigned int cmd)
{
	return (memc_ioctl_cmd_stop[_IOC_NR(cmd)&0x1f]|memc_ioctl_cmd_stop[0]);
}

unsigned char vpq_memc_ioctl_get_stop_run_by_idx(unsigned char cmd_idx)
{
	return memc_ioctl_cmd_stop[cmd_idx];
}

void vpq_memc_ioctl_set_stop_run_by_idx(unsigned char cmd_idx, unsigned char stop)
{
	memc_ioctl_cmd_stop[cmd_idx] = stop;
}

int vpq_memc_open(struct inode *inode, struct file *filp)
{
	return 0;
}

ssize_t  vpq_memc_read(struct file *filep, char *buffer, size_t count, loff_t *offp)
{
	return 0;
}

ssize_t vpq_memc_write(struct file *filep, const char *buffer, size_t count, loff_t *offp)
{
	return 0;
}

int vpq_memc_release(struct inode *inode, struct file *filep)
{
	return 0;
}

long vpq_memc_ioctl(struct file *file, unsigned int cmd,  unsigned long arg)
{
	int retval = 0;
#ifdef CONFIG_HW_SUPPORT_MEMC
	VPQ_MEMC_SETVIDEOBLOCK_T set_video_block_info;
	//rtd_pr_memc_debug("VPQ MEMC: vpq_memc_ioctl, %x;;agr:%x\n", cmd,*((unsigned int *)arg));
	if (_IOC_TYPE(cmd) != VPQ_IOC_MEMC_MAGIC || _IOC_NR(cmd) > VPQ_MEMC_IOC_MAXNR) return -ENOTTY ;

	if (vpq_memc_ioctl_get_stop_run(cmd))
		return 0;

	switch (cmd)
	{
		case VPQ_IOC_MEMC_INITILIZE:
		{
			rtd_pr_memc_debug("##############[MEMC]VPQ_IOC_MEMC_INITILIZE\n");
			HAL_VPQ_MEMC_Initialize();

			break;
		}
		case VPQ_IOC_MEMC_UNINITILIZE:
		{
			//rtd_pr_memc_debug("##############[MEMC]VPQ_IOC_MEMC_UNINITILIZE\n");
			HAL_VPQ_MEMC_Uninitialize();

			break;
		}
		case VPQ_IOC_MEMC_SETMOTIONCOMP:
		{
			//rtd_pr_memc_emerg( "##############[MEMC]VPQ_IOC_MEMC_SETMOTIONCOMP\n");
			VPQ_MEMC_SETMOTIONCOMP_T set_motion_comp_info;
			if(copy_from_user((void *)&set_motion_comp_info, (const void __user *)arg, sizeof(VPQ_MEMC_SETMOTIONCOMP_T)))
			{
				retval = -EFAULT;
				MEMC_DBG_PRINT("scaler vpq memc ioctl code=VPQ_IOC_MEMC_SETMOTIONCOMP failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				retval = HAL_VPQ_MEMC_SetMotionComp(set_motion_comp_info.blurLevel, set_motion_comp_info.judderLevel, set_motion_comp_info.motion);
			}

			break;
		}
/*
		case VPQ_IOC_MEMC_SETBLURLEVEL:
		{
			//rtd_pr_memc_debug("##############[MEMC]VPQ_IOC_MEMC_SETBLURLEVEL\n");
			UINT8 blurLevel;
			if(copy_from_user((void *)&blurLevel, (const void __user *)arg, sizeof(UINT8)))
			{
				retval = -EFAULT;
				MEMC_DBG_PRINT("scaler vpq memc ioctl code=VPQ_IOC_MEMC_SETBLURLEVEL failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VPQ_MEMC_SetBlurLevel(blurLevel);
			}

			break;
		}
		case VPQ_IOC_MEMC_SETJUDDERLEVEL:
		{
			//rtd_pr_memc_debug("##############[MEMC]VPQ_IOC_MEMC_SETJUDDERLEVEL\n");
			UINT8 judderLevel;
			if(copy_from_user((void *)&judderLevel, (const void __user *)arg, sizeof(UINT8)))
			{
				retval = -EFAULT;
				MEMC_DBG_PRINT("scaler vpq memc ioctl code=VPQ_IOC_MEMC_SETJUDDERLEVEL failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VPQ_MEMC_SetJudderLevel(judderLevel);
			}

			break;
		}
*/
		case VPQ_IOC_MEMC_MOTIONCOMPONOFF:
		{
			//rtd_pr_memc_emerg( "##############[MEMC]VPQ_IOC_MEMC_MOTIONCOMPONOFF\n");
			BOOLEAN bOnOff;
			if(copy_from_user((void *)&bOnOff, (const void __user *)arg, sizeof(BOOLEAN)))
			{
				retval = -EFAULT;
				MEMC_DBG_PRINT("scaler vpq memc ioctl code=VPQ_IOC_MEMC_MOTIONCOMPONOFF failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				retval = HAL_VPQ_MEMC_MotionCompOnOff(bOnOff);
			}

			break;
		}
		case VPQ_IOC_MEMC_LOWDELAYMODE:
		{
			//rtd_pr_memc_debug("##############[MEMC]VPQ_IOC_MEMC_LOWDELAYMODE\n");
			UINT8 type;
			if(copy_from_user((void *)&type, (const void __user *)arg, sizeof(UINT8)))
			{
				retval = -EFAULT;
				MEMC_DBG_PRINT("scaler vpq memc ioctl code=VPQ_IOC_MEMC_LOWDELAYMODE failed!!!!!!!!!!!!!!!\n");
			}
			else
			{

				retval = HAL_VPQ_MEMC_LowDelayMode(type);
			}

			break;
		}
		case VPQ_IOC_MEMC_SETRGBYUVMode:
		{
			//rtd_pr_memc_debug("##############[MEMC]VPQ_IOC_MEMC_SETRGBYUVMode\n");
			UINT8 mode;
			if(copy_from_user((void *)&mode, (const void __user *)arg, sizeof(mode)))
			{
				retval = -EFAULT;
				MEMC_DBG_PRINT("scaler vpq memc ioctl code=VPQ_IOC_MEMC_SETRGBYUVMode failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				retval = HAL_VPQ_MEMC_SetRGBYUVMode(mode);
			}

			break;
		}

		case VPQ_IOC_MEMC_GETFRAMEDELAY:
		{
			//rtd_pr_memc_debug("##############[MEMC]VPQ_IOC_MEMC_GETFRAMEDELAY\n");
			UINT16 *pFrameDelay;
			if(copy_from_user((void *)&pFrameDelay, (const void __user *)arg, sizeof(UINT16)))
			{
				retval = -EFAULT;
				MEMC_DBG_PRINT("scaler vpq memc ioctl code=VPQ_IOC_MEMC_GETFRAMEDELAY failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VPQ_MEMC_GetFrameDelay(pFrameDelay);
			}

			break;
		}

		case VPQ_IOC_MEMC_SETVIDEOBLOCK:
		{
			rtd_pr_memc_debug("\r\n###[MEMC]VPQ_IOC_MEMC_SETVIDEOBLOCK####\r\n");

			if(copy_from_user((void *)&set_video_block_info, (const void __user *)arg, sizeof(VPQ_MEMC_SETVIDEOBLOCK_T)))
			{
				retval = -EFAULT;
				MEMC_DBG_PRINT("scaler vpq memc ioctl code=VPQ_IOC_MEMC_SETVIDEOBLOCK failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VPQ_MEMC_SetVideoBlock(set_video_block_info.type, set_video_block_info.bOnOff);
			}

			break;
		}
/*
		case VPQ_IOC_MEMC_SETTRUEMOTIONDEMO:
		{
			//rtd_pr_memc_debug("##############[MEMC]VPQ_IOC_MEMC_SETTRUEMOTIONDEMO\n");
			BOOLEAN bOnOff;
			if(copy_from_user((void *)&bOnOff, (const void __user *)arg, sizeof(BOOLEAN)))
			{
				retval = -EFAULT;
				MEMC_DBG_PRINT("scaler vpq memc ioctl code=VPQ_IOC_MEMC_SETTRUEMOTIONDEMO failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VPQ_MEMC_SetTrueMotionDemo(bOnOff);
			}

			break;
		}

		case VPQ_IOC_MEMC_GETFIRMWAREVERSION:
		{
			//rtd_pr_memc_debug("##############[MEMC]VPQ_IOC_MEMC_GETFIRMWAREVERSION\n");
			UINT16 *pVersion;
			if(copy_from_user((void *)&pVersion, (const void __user *)arg, sizeof(UINT16)))
			{
				retval = -EFAULT;
				MEMC_DBG_PRINT("scaler vpq memc ioctl code=VPQ_IOC_MEMC_SETBLURLEVEL failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VPQ_MEMC_GetFirmwareVersion(pVersion);
			}

			break;
		}
*/
		case VPQ_IOC_MEMC_SETBYPASSREGION:
		{
			//rtd_pr_memc_debug("##############[MEMC]VPQ_IOC_MEMC_SETBYPASSREGION\n");
			VPQ_MEMC_SETBYPASSREGION_T set_bypass_region_info;
			if(copy_from_user((void *)&set_bypass_region_info, (const void __user *)arg, sizeof(VPQ_MEMC_SETBYPASSREGION_T)))
			{
				retval = -EFAULT;
				MEMC_DBG_PRINT("scaler vpq memc ioctl code=VPQ_IOC_MEMC_SETBYPASSREGION failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VPQ_MEMC_SetBypassRegion(set_bypass_region_info.bOnOff, set_bypass_region_info.region, set_bypass_region_info.x, set_bypass_region_info.y, set_bypass_region_info.w, set_bypass_region_info.h);
			}

			break;
		}
/*
		case VPQ_IOC_MEMC_SETREVERSECONTROL:
		{
			//rtd_pr_memc_debug("##############[MEMC]VPQ_IOC_MEMC_SETREVERSECONTROL\n");
			UINT8 u8Mode;
			if(copy_from_user((void *)&u8Mode, (const void __user *)arg, sizeof(UINT8)))
			{
				retval = -EFAULT;
				MEMC_DBG_PRINT("scaler vpq memc ioctl code=VPQ_IOC_MEMC_SETREVERSECONTROL failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VPQ_MEMC_SetReverseControl(u8Mode);
			}

			break;
		}

		case VPQ_IOC_MEMC_FREEZE:
		{
			//rtd_pr_memc_debug("##############[MEMC]VPQ_IOC_MEMC_FREEZE\n");
			UINT8 type;
			if(copy_from_user((void *)&type, (const void __user *)arg, sizeof(UINT8)))
			{
				retval = -EFAULT;
				MEMC_DBG_PRINT("scaler vpq memc ioctl code=VPQ_IOC_MEMC_FREEZE failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VPQ_MEMC_Freeze(type);
			}

			break;
		}

		case VPQ_IOC_MEMC_SETDEMOBAR:
		{
			//rtd_pr_memc_debug("##############[MEMC]VPQ_IOC_MEMC_SETDEMOBAR\n");
			VPQ_MEMC_SETDEMOBAR_T set_demo_bar_info;
			if(copy_from_user((void *)&set_demo_bar_info, (const void __user *)arg, sizeof(VPQ_MEMC_SETDEMOBAR_T)))
			{
				retval = -EFAULT;
				MEMC_DBG_PRINT("scaler vpq memc ioctl code=VPQ_IOC_MEMC_SETDEMOBAR failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VPQ_MEMC_SetDemoBar(set_demo_bar_info.bOnOff, set_demo_bar_info.r, set_demo_bar_info.g, set_demo_bar_info.b);
			}

			break;
		}

		case VPQ_IOC_MEMC_DEBUG:
		{
			//rtd_pr_memc_debug("##############[MEMC]VPQ_IOC_MEMC_DEBUG\n");
			HAL_VPQ_MEMC_DEBUG();

			break;
		}
*/
		default:
			rtd_pr_memc_debug("Scaler vpq memc: ioctl code = %d is invalid!!!!!!!!!!!!!!!1\n", cmd);
			break ;
		}
#endif //CONFIG_HW_SUPPORT_MEMC
	return retval;
}
//#endif

struct file_operations vpq_memc_fops= {
	.owner =    THIS_MODULE,
	.open  =    vpq_memc_open,
	.release =  vpq_memc_release,
	.read  =    vpq_memc_read,
	.write = 	vpq_memc_write,
	.unlocked_ioctl =    vpq_memc_ioctl,
#ifdef CONFIG_ARM64
#ifdef CONFIG_COMPAT
	.compat_ioctl = vpq_memc_ioctl,
#endif
#endif
};

#ifdef CONFIG_PM
static const struct dev_pm_ops vpq_memc_pm_ops =
{
	.suspend    = vpq_memc_suspend,
	.resume     = vpq_memc_resume,
#ifdef CONFIG_HIBERNATION
	.freeze 	= vpq_memc_suspend_snashop,
	.thaw 		= vpq_memc_resume_snashop,
	.poweroff 	= vpq_memc_suspend_snashop,
	.restore 	= vpq_memc_resume_snashop,
#endif
};
#endif // CONFIG_PM


static struct platform_device *vpq_memc_platform_devs;

static struct platform_driver vpq_memc_device_driver = {
	.driver =
	{
        .name         = VPQ_MEMC_DEVICE_NAME,
        .bus          = &platform_bus_type,
#ifdef CONFIG_PM
	 .pm         = &vpq_memc_pm_ops,
#endif

	},
};

static char *vpq_memc_devnode(struct device *dev, umode_t *mode)
{
	return NULL;
}

static void memc_isr_disable (void)
{
#ifdef CONFIG_HW_SUPPORT_MEMC
    rtd_pr_memc_notice( "%s %d\n",__func__,__LINE__);
    #ifdef CONFIG_MEMC_BYPASS
         return;
    #endif
    if(Scaler_MEMC_CLK_Check() == FALSE){
	return;
    }

    // disable interrupt
    UINT32 u32_interrupt_reg = 0;
    //reg_kmc_int_en
    u32_interrupt_reg = IoReg_Read32(0xB8099018); //MC_TOP__REG_KMC_IN_INT_SEL_ADDR
    u32_interrupt_reg &= 0xf0ffffff;
    IoReg_Write32(0xB8099018, u32_interrupt_reg);

    //reg_post_int_en
    u32_interrupt_reg = IoReg_Read32(0xB809D008); //KPOST_TOP__REG_POST_INT_SOURCE_SEL_ADDR
    u32_interrupt_reg &= 0xfffffe1f;
    IoReg_Write32(0xB809D008, u32_interrupt_reg);
#endif //CONFIG_HW_SUPPORT_MEMC
}

static int memc_pm_event (struct notifier_block *this, unsigned long event, void *ptr)
{
        switch (event) {
    case PM_SUSPEND_PREPARE:
        rtd_pr_memc_info("%s %d\n",__func__,__LINE__);
        memc_isr_disable();
        break;
        default:
                break;
        }

    return NOTIFY_DONE;
}

static struct notifier_block memc_notifier = {
  .notifier_call = memc_pm_event,
};


int vpq_memc_module_init(void)
{
	int result;
	int vpq_memc_devno;
	dev_t devno = 0;//vbe device number
	//PANEL_CONFIG_PARAMETER *panel_parameter = NULL;

	result = alloc_chrdev_region(&devno, vpq_memc_minor, vpq_memc_nr_devs,VPQ_MEMC_DEVICE_NAME);
	vpq_memc_major = MAJOR(devno);
	if (result < 0) {
		rtd_pr_memc_warn("VPQ_MEMC_DEVICE: can't get major %d\n", vpq_memc_major);
		return result;
	}

	rtd_pr_memc_debug("VPQ_MEMC_DEVICE init module major number = %d\n", vpq_memc_major);
	vpq_memc_devno = MKDEV(vpq_memc_major, vpq_memc_minor);

	vpq_memc_class = class_create(THIS_MODULE, VPQ_MEMC_DEVICE_NAME);

	if (IS_ERR(vpq_memc_class))
	{
		rtd_pr_memc_debug("scalevpqmemc: can not create class...\n");
	    result=PTR_ERR(vpq_memc_class);
		goto fail_class_create;
	}
	vpq_memc_class->devnode = vpq_memc_devnode;
	vpq_memc_platform_devs = platform_device_register_simple(VPQ_MEMC_DEVICE_NAME, -1, NULL, 0);
    if((result=platform_driver_register(&vpq_memc_device_driver)) != 0)
	{
		rtd_pr_memc_debug("scaler vpqmemc: can not register platform driver...\n");
    	goto fail_platform_driver_register;
    }

	cdev_init(&vpq_memc_cdev, &vpq_memc_fops);
	vpq_memc_cdev.owner = THIS_MODULE;
   	vpq_memc_cdev.ops = &vpq_memc_fops;
	result = cdev_add (&vpq_memc_cdev, vpq_memc_devno, 1);
	if (result)
	{
		MEMC_DBG_PRINT("Error %d adding VPQ_MEMC_DEVICE!\n", result);
		goto fail_cdev_init;
	}
	device_create(vpq_memc_class, NULL, MKDEV(vpq_memc_major, 0), NULL,VPQ_MEMC_DEVICE_NAME);
	sema_init(&vpq_memc_Semaphore, 1);
	//panel_parameter =  (PANEL_CONFIG_PARAMETER *)&platform_info.panel_parameter;
	//Panel_InitParameter(panel_parameter);
	//Panel_LVDS_To_Hdmi_Converter_Parameter();

        register_pm_notifier(&memc_notifier);

	return 0;//Success

fail_cdev_init:
	platform_driver_unregister(&vpq_memc_device_driver);
fail_platform_driver_register:
	platform_device_unregister(vpq_memc_platform_devs);
	vpq_memc_platform_devs = NULL;
	class_destroy(vpq_memc_class);
fail_class_create:
	vpq_memc_class = NULL;
	unregister_chrdev_region(vpq_memc_devno, vpq_memc_nr_devs);
	return result;
}

void __exit vpq_memc_module_exit(void)
{
	dev_t devno = MKDEV(vpq_memc_major, vpq_memc_minor);
	MEMC_DBG_PRINT("vbe clean module vpq_memc_major = %d\n", vpq_memc_major);

  	device_destroy(vpq_memc_class, MKDEV(vpq_memc_major, 0));
  	class_destroy(vpq_memc_class);
	vpq_memc_class=NULL;
	cdev_del(&vpq_memc_cdev);

   	/* device driver removal */
	if(vpq_memc_platform_devs)
	{
		platform_device_unregister(vpq_memc_platform_devs);
		vpq_memc_platform_devs=NULL;
	}
  	platform_driver_unregister(&vpq_memc_device_driver);

	/* cleanup_module is never called if registering failed */
	unregister_chrdev_region(devno, vpq_memc_nr_devs);
}

module_init(vpq_memc_module_init);
module_exit(vpq_memc_module_exit);
