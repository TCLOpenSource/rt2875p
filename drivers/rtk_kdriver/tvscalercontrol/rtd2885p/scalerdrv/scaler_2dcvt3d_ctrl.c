//#include <stdio.h>
//#include <stdlib.h>
//#include <sys/ioctl.h>
//#include <unistd.h>
//#include <fcntl.h>
//#include <string.h>
//#include <assert.h>
#ifndef BUILD_QUICK_SHOW
#include <linux/kernel.h>
#include <linux/delay.h>
#else
#include <no_os/printk.h>
#include <no_os/semaphore.h>
#include <no_os/spinlock.h>
#include <no_os/spinlock_types.h>
#include <malloc.h>
#include <div64.h>
#include <rtd_log/rtd_module_log.h>
#include <sysdefs.h>
#endif


#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerCommon.h>
#else
#include <scalercommon/scalerCommon.h>
#endif
#include <rtd_log/rtd_module_log.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/scalerdrv/conversion_2d_3d.h>
#include <tvscalercontrol/scalerdrv/scaler_2dcvt3d_ctrl.h>


// =========================== define & Macro ================================
#ifdef ABS
  #undef ABS
#endif
#define ABS( a )     ( (int32_t)(a) < 0 ? -(a) : (a) )

// 2D 3D convert mode
// [2D3D] mode enable control (disable/enable: 0/1)
#define USER_DEFAULT 		FALSE


// ======================= Variable Declaration ================================
static unsigned char cvtEnableStatus_user = USER_DEFAULT;
static unsigned char tiltEnableStatus = FALSE;


// ======================= Function Declaration ================================



// =========================== Function implementation =========================


unsigned char scaler_2Dcvt3D_get_userStatus(void)
{
	return cvtEnableStatus_user;
}



void scaler_2Dcvt3D_set_userEnableStatus(unsigned char userEnableFlag)
{
	#if 0
	if ((cvtEnableStatus_user == userEnableFlag) && !userEnableFlag)
		return;

	cvtEnableStatus_user = userEnableFlag;
	rtd_pr_vbe_debug("[2D3D] Set_USER=[%d]\n", cvtEnableStatus_user);
	#endif
	if(userEnableFlag == TRUE){
		// set shift mode: shift L+R enabled
		drv_2D_to_3D_en_shiftmode(SHIFTMODE_L_R);
		if(scalerDrv_2D_to_3D_Obj_get_en()) //tilt and object are both enable
		{
		        drv_2D_to_3D_set_object_depth_en(0); //0~7 (please see .cpp file to know how to set value)
		}

	}else{
		// set shift mode: shift L+R disabled
		//drv_2D_to_3D_set_tilt_depth_en(0);
		drv_2D_to_3D_en_shiftmode(SHIFTMODE_NO);

	}
}


void scaler_2Dcvt3D_set_shifter(unsigned int Strength,unsigned int Depth)
{
	#if HORIZONTAL_SHIFT_NO_BORDER
	drv_2D_to_3D_H_shift_no_border(Depth);
	#elif HORIZONTAL_SHIFT
	drv_2D_to_3D_H_shift(Depth); //strength:static , focal: 0~32 ,for TV15_1_3D,12
	#else
	drv_2D_to_3D_shifter(Strength, Depth);
	#endif
}

void scaler_2Dcvt3D_set_tilt_enable(unsigned char enableFlag)
{
	tiltEnableStatus = enableFlag;
	return;
}


unsigned char scaler_2Dcvt3D_get_tilt_enable(void)
{
	return tiltEnableStatus;
}



void scaler_2Dcvt3D_set_tilt_depth(unsigned char enableFlag)
{

	//rtd_pr_vbe_debug("scaler_2Dcvt3D_set_tilt_depth>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	#if HORIZONTAL_SHIFT
	#elif HORIZONTAL_SHIFT_NO_BORDER
	#else
	drv_2D_to_3D_set_tilt_depth_en(enableFlag);
	#endif
}

/* ********************************************************
* scaler_2Dcvt3D_set_hactStaShift(): change the 3D depth in 2D convert to 3D mode
* Parameter: shiftCount:
*  -1			: disable update the 3D depth (reset)
*  > IPH_ACT_STA	: disable update the 3D depth (invalid value)
*  other			: update the L/R frame 3D depth by request (normal)
******************************************************** */
void scaler_2Dcvt3D_set_hactStaShift(StructDisplayInfo* info, char shiftCount)
{
	// 3D depth range: +/- 15
	// [NEW] 720p case �U, ���n�W�L -11
	// -11 = ((-11*1.5)*(1920/1280) = 24 real pixel)
	// -15 = ((-15*1.5)*(1920/1280) = 33 real pixel)
	shiftCount = (shiftCount > DISP_2DCVT3D_3D_DEPTH_MAX_VALUE? DISP_2DCVT3D_3D_DEPTH_MAX_VALUE:
		(shiftCount < -DISP_2DCVT3D_3D_DEPTH_MAX_VALUE? -DISP_2DCVT3D_3D_DEPTH_MAX_VALUE: shiftCount));


	return;
}



// Shift Direction
// 0: L: left, R: right
// 1: L: right, R:left
unsigned char scaler_2Dcvt3D_get_shiftDirection(void)
{
	rtd_pr_vbe_debug("[TODO][3D]new Function@%s\n", __FUNCTION__);
	return 0;// bit 16~23
}




// Shift Direction
// 0: L: left, R: right
// 1: L: right, R:left
void scaler_2Dcvt3D_set_shiftDirection(bool shiftWay)
{
	rtd_pr_vbe_debug("[TODO][3D]new Function@%s\n", __FUNCTION__);
	return;
}


int scaler_2Dcvt3D_init(void)
{

	drv_2D_to_3D_Initialize();
	return 0;
}

void scaler_2Dcvt3D_LR_swap(bool lr_swap)
{
#ifndef BUILD_QUICK_SHOW
    drv_2D_to_3D_LR_swap(lr_swap);
#endif
	return ;
}
