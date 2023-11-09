/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2014
  * All rights reserved.
  * ========================================================================*/
  /*================= File Description =======================================*/
  /**
   * @file
   *  This file is for Local Contrast related functions.
   *
   * @author  $Author$
   * @date	  $Date$
   * @version	  $Revision$
   */

  /*============================ Module dependency	===========================*/
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/workqueue.h>
#include <linux/string.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <linux/kernel.h>
#include "tvscalercontrol/vip/still_logo_detect.h"
#include <tvscalercontrol/scaler/vipinclude.h> // for typedef that used in VIP_reg_def.h
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <rtd_log/rtd_module_log.h>


#include <linux/mtd/mtd.h>
#include <linux/uaccess.h>
#include <asm/cacheflush.h>
#include <linux/pageremap.h>
#include <uapi/linux/const.h>
#include <linux/mm.h>
#include <linux/auth.h>
#include <linux/semaphore.h>//for semaphore
#include <linux/version.h>
//#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/scaler/scalerstruct.h>
#include <tvscalercontrol/scalerdrv/mode.h>
#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <tvscalercontrol/scaler/source.h>
#include <tvscalercontrol/scalerdrv/scalerdrv.h>
//#include <tvscalercontrol/vip/vip_reg_def.h>
#include <tvscalercontrol/vip/gibi_od.h>
#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#include <rtd_log/rtd_module_log.h>
#include <rbus/lgd_dither_reg.h>
#include <rbus/pcid_reg.h>
#include <rbus/od_dma_reg.h>
#include "rtk_vip_logger.h"
#include <tvscalercontrol/vip/color.h>
#include "tvscalercontrol/vip/localdimming.h"
#include <tvscalercontrol/vip/scalerColor.h>

#include <rbus/osdovl_reg.h>
#include <rbus/blu_reg.h>
//#include <rbus/kme_dm_top1_reg.h>
#ifndef CONFIG_MEMC_NOTSUPPORT
#include "memc_reg_def.h"
#endif

#undef VIPprintf
#define VIPprintf(fmt, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_GIBI_OD_DEBUG,fmt,##args)

#define SLD_Debug 0

#ifndef ABS_
#define ABS_(x)   ((x) >= 0 ? (x) : -(x))
#endif

unsigned int APL_pre[LC_MAX_H_NUM*LC_MAX_V_NUM] = {0};
unsigned int APL_cur[LC_MAX_H_NUM*LC_MAX_V_NUM] = {0};
unsigned int APL_delta[LC_MAX_H_NUM*LC_MAX_V_NUM] = {0};

#define SLD_BLK_APL_MAX (120*68)		// max for 4k. 
unsigned short APL_pre_uint16[SLD_BLK_APL_MAX] = {0};
unsigned short APL_cur_uint16[SLD_BLK_APL_MAX] = {0};
unsigned short APL_delta_uint16[SLD_BLK_APL_MAX] = {0};

//Add by kuan 2021.08.03
unsigned int scene_change_flag = 0;
int scene_change_speed_up_count_ = 0;
int APL_avg_cur = 0, APL_avg_pre = 0;
unsigned int memc_scene_change_flag_global = 0, apl_scene_change_flag_global = 0;
unsigned int still_image_count = 0 ;

DRV_sld_condition_struct sld_condition;
DRV_SLD_sld_fw_read_only fw_read_only_data;
unsigned char LR_LOGO_Flag_R = 0;
unsigned char LR_LOGO_Flag_L = 0;

int thl_temporal_dY = 0;
int thl_counter_NEWS_case = 0;
int SLD_cmd_2 = 0;

//#ifdef CONFIG_ARM64 //ARM32 compatible
// for register dump
#include <tvscalercontrol/io/ioregdrv.h>
#undef rtd_outl
#define rtd_outl(x, y)     								IoReg_Write32(x,y)
#undef rtd_inl
#define rtd_inl(x)     									IoReg_Read32(x)
#undef rtd_maskl
#define rtd_maskl(x, y, z)     							IoReg_Mask32(x,y,z)
#undef rtd_setbits
#define rtd_setbits(offset, Mask) rtd_outl(offset, (rtd_inl(offset) | Mask))
#undef rtd_clearbits
#define rtd_clearbits(offset, Mask) rtd_outl(offset, ((rtd_inl(offset) & ~(Mask))))
//#endif
extern BOOL WriteRegister(unsigned int uiAddr, int ibitStart, int ibitEnd, unsigned int uiVal);


unsigned int cur_frame_idx_global = 0;
unsigned int counter_box_logo_case = 0;
unsigned int index_box_logo_case = 0;
unsigned int counter_NEWS_case = 0;
unsigned int index_NEWS_case = 0;
unsigned int counter_noise_case = 0;
unsigned int index_noise_case = 0;
unsigned int flag_noise_in = 0;
unsigned int flag_rgb_box_in = 0;
unsigned short lock_fw_gain = 0, lock_rgb_box_fw_step = 0;
unsigned int LC_time = 0; 


int cmd_dY_preCur_mouseCon=0;
int cmd_LC_period=0;
int cmd_thl_counter_UR_con=0;
int cmd_counter_frame_highY_blk=0;
int cmd_lock_fw_gain_frame=0;
int cmd_pixel_gain_step=0;
int cmd_thl_counter_blk_pre_cur_same_mouseCon=0;
int cmd_thl_error_time_box_logo_case=0;
int cmd_thl_counter_box_logo_case=0;
int cmd_thl_error_time_NEWS_case=0;
int cmd_thl_counter_NEWS_case=0;
int cmd_thl_error_time_noise_case=0;
int cmd_thl_counter_noise_case=0;
int cmd_thl_dC=0;
int cmd_thl_dUDL=0;
int cmd_rgb_box_leave_idx_delete_step=0;
int cmd_box_logo_pixel_gain=0;
int cmd_box_logo_maxGain_time=0;
int cmd_box_lock_frame=0;


unsigned char SLD_DMA_Ready = 0;

#define logo_circle4_x_sta  388 // 393  // 398
#define logo_circle4_x_end  470// 465   // 473
#define logo_circle4_y_sta  8 // 18 // 13
#define logo_circle4_y_end  46 // 36    // 32

#define logo_circle4_apply_x_sta  393   // 398
#define logo_circle4_apply_x_end  465   // 473
#define logo_circle4_apply_y_sta  18    // 13
#define logo_circle4_apply_y_end  36    // 32

#define logo_circle4_h (logo_circle4_x_end-logo_circle4_x_sta+1)
#define logo_circle4_v (logo_circle4_y_end-logo_circle4_y_sta+1)
unsigned char logo_flag_4circle[logo_circle4_h * logo_circle4_v] = {
#if 1
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
#else
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
    0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,

#endif
};

#define logo_circle4_x_sta_271x136  189 // 393  // 398
#define logo_circle4_x_end_271x136  240// 465   // 473
#define logo_circle4_y_sta_271x136  0 // 18 // 13
#define logo_circle4_y_end_271x136  25 // 36    // 32

#define logo_circle4_apply_x_sta_271x136  195   // 398
#define logo_circle4_apply_x_end_271x136  237   // 473
#define logo_circle4_apply_y_sta_271x136  6    // 13
#define logo_circle4_apply_y_end_271x136  18    // 32

#define logo_circle4_h_271x136 (logo_circle4_x_end_271x136-logo_circle4_x_sta_271x136+1)
#define logo_circle4_v_271x136 (logo_circle4_y_end_271x136-logo_circle4_y_sta_271x136+1)

unsigned char logo_flag_4circle_16_9_justscan_on_271x136[logo_circle4_h_271x136 * logo_circle4_v_271x136] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

unsigned char logo_flag_4circle_16_9_justscan_auto_271x136[logo_circle4_h_271x136 * logo_circle4_v_271x136] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};
/*
void drvif_color_set_SLD_cnt_thl(unsigned int cnt_th)
{
	color_sld_sld_drop_RBUS color_sld_drop;

	color_sld_drop.regValue = IoReg_Read32(COLOR_SLD_SLD_DROP_reg);

	color_sld_drop.sld_cnt_thl = cnt_th;

	IoReg_Write32(COLOR_SLD_SLD_DROP_reg, color_sld_drop.regValue);

}
*/

static unsigned short SLD_BLK_APL[SLD_BLK_APL_MAX];	
static VIP_SLD_DMA_CTRL SLD_APL_DMA_CTRL[2] = {0};

int ceil_SLD(int a, int b){
	return (a/b + ((a%b)!=0) );
}

VIP_SLD_DMA_CTRL* drvif_fwif_color_Get_SLD_APL_DMA_CTRL(unsigned char index)
{
	return &SLD_APL_DMA_CTRL[index];
}

char drvif_fwif_color_Get_SLD_APL(unsigned short *pTBL, unsigned int num)
{
	if (pTBL == NULL) {
		rtd_pr_vpq_emerg("SLD_HW, drvif_fwif_color_Get_SLD_APL=NULL\n");
		return -1;
	}

	if (num > SLD_BLK_APL_MAX) {
		rtd_pr_vpq_emerg("SLD_HW, drvif_fwif_color_Get_SLD_APL num exceed, num = %d,\n", num);
		return -1;
	}

	memcpy(pTBL, &SLD_BLK_APL[0], sizeof(short)*num);
	return 0;
}

bool drvif_set_sld_dma(void)
{
	unsigned int sld_size = 0;
	unsigned long sld_addr = 0;
	unsigned long align_value =0;

	if (drvif_color_get_HW_MEX_STATUS(MEX_SLD) != 1) {
		rtd_pr_vpq_info("HW_MEX,  MEX_SLD != 1, DMA\n");
		return false;
	}
#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
	sld_size = carvedout_buf_query_secure(CARVEDOUT_SCALER_OD, (void*) &sld_addr);
#else
	sld_size = carvedout_buf_query(CARVEDOUT_SCALER_OD,(void*) &sld_addr);
#endif
	if (sld_addr == 0 || sld_size == 0) {
		VIPprintf("[%s %d] ERR : %lx %x\n", __func__, __LINE__, sld_addr, sld_size);
                return false;
	}

#if 1
    align_value = dvr_memory_alignment((unsigned long)sld_addr, dvr_size_alignment(500000)); //96 Alignment for Bandwidth Request

    if (align_value == 0)
        BUG();
#else
        align_value = sld_addr; //origin value
#endif
        VIPprintf("aligned phy addr=%lx,\n", align_value);

	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_addr_0_reg, align_value); //must be check 96 alignment
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_addr_1_reg, align_value+0x02f00000); //Cap_sta_addr_0, must be check 96 alignment

	IoReg_Write32(OD_DMA_ODDMA_WR_Rule_check_up_reg, align_value+0x02f00000+0x2f000000  );//Cap_up_limit
	IoReg_Write32(OD_DMA_ODDMA_WR_Rule_check_low_reg, sld_addr);
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_addr_0_reg, align_value);//must be check 96 alignment	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_addr_0_reg, align_value);//must be check 96 alignment
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_addr_1_reg, align_value+0x02f00000); //Cap_sta_addr_0, must be check 96 alignment

	IoReg_Write32(OD_DMA_ODDMA_RD_Rule_check_up_reg, align_value+0x02f00000+0x2f000000  );//Disp_up_limit
	IoReg_Write32(OD_DMA_ODDMA_RD_Rule_check_low_reg, sld_addr);

	VIPprintf("[sld]drvif_set_sld_dma...end\n");

	return true;
}

void drvif_color_set_SLD_4k_dma_setting_K7(void)
{
	//unsigned char sld_dma_return = false;
        color_sld_sld_size_RBUS color_sld_sld_size;
        od_dma_oddma_wr_ctrl_RBUS      od_dma_oddma_wr_ctrl;
        od_dma_oddma_rd_ctrl_RBUS       od_dma_oddma_rd_ctrl;
        //od_dma_oddma_wr_num_bl_wrap_addr_0_RBUS        od_dma_oddma_wr_num_bl_wrap_addr_0;
        //od_dma_oddma_wr_num_bl_wrap_addr_1_RBUS        od_dma_oddma_wr_num_bl_wrap_addr_1;
        //od_dma_oddma_rd_num_bl_wrap_addr_0_RBUS od_dma_oddma_rd_num_bl_wrap_addr_0;
        od_dma_oddma_wr_num_bl_wrap_word_RBUS   od_dma_oddma_wr_num_bl_wrap_word;
        //od_dma_oddma_rd_num_bl_wrap_addr_1_RBUS od_dma_oddma_rd_num_bl_wrap_addr_1;
        od_dma_oddma_rd_num_bl_wrap_word_RBUS    od_dma_oddma_rd_num_bl_wrap_word;
        od_dma_oddma_rd_num_bl_wrap_line_step_RBUS      od_dma_oddma_rd_num_bl_wrap_line_step;
        od_dma_oddma_wr_num_bl_wrap_line_step_RBUS     od_dma_oddma_wr_num_bl_wrap_line_step;
        od_dma_oddma_wr_num_bl_wrap_ctl_RBUS       od_dma_oddma_wr_num_bl_wrap_ctl;
        od_dma_oddma_rd_num_bl_wrap_ctl_RBUS        od_dma_oddma_rd_num_bl_wrap_ctl;
        od_dma_od_dma_ctrl_RBUS  od_dma_od_dma_ctrl;
        od_dma_od_dma_db_en_RBUS od_dma_od_dma_db_en;
	 color_sld_sld_control_RBUS color_sld_control;
	 //od_dma_oddma_wr_rule_check_up_RBUS  od_dma_oddma_wr_rule_check_up;
	 //od_dma_oddma_rd_rule_check_up_RBUS	 od_dma_oddma_rd_rule_check_up;
	 od_dma_oddma_pq_cmp_RBUS od_dma_oddma_pq_cmp;
	 od_dma_oddma_pq_decmp_RBUS od_dma_oddma_pq_decmp;

       od_dma_od_dma_db_en.regValue = IoReg_Read32(OD_DMA_OD_DMA_DB_EN_reg);
	od_dma_od_dma_db_en.cap_db_en = 0;
	od_dma_od_dma_db_en.disp_db_en = 0;
	od_dma_od_dma_db_en.smooth_tog_en = 0;
	IoReg_Write32(OD_DMA_OD_DMA_DB_EN_reg,0x00000000); //db disable

	// sld_dma_return = drvif_set_sld_dma();

        color_sld_sld_size.regValue = IoReg_Read32(COLOR_SLD_SLD_SIZE_reg);
        od_dma_oddma_wr_ctrl.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_Ctrl_reg);
        od_dma_oddma_rd_ctrl.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_Ctrl_reg);
        //od_dma_oddma_wr_num_bl_wrap_addr_0.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_addr_0_reg);
        //od_dma_oddma_wr_num_bl_wrap_addr_1.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_addr_1_reg);
        //od_dma_oddma_rd_num_bl_wrap_addr_0.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_addr_0_reg);
        //od_dma_oddma_rd_num_bl_wrap_addr_1.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_addr_1_reg);
        od_dma_oddma_wr_num_bl_wrap_word.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_word_reg);
        od_dma_oddma_rd_num_bl_wrap_word.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_word_reg);
        od_dma_oddma_rd_num_bl_wrap_line_step.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_line_step_reg);
        od_dma_oddma_wr_num_bl_wrap_line_step.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_line_step_reg);
        od_dma_oddma_wr_num_bl_wrap_ctl.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_ctl_reg);
        od_dma_oddma_rd_num_bl_wrap_ctl.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_ctl_reg);
        od_dma_od_dma_ctrl.regValue = IoReg_Read32(OD_DMA_OD_DMA_CTRL_reg);
	 color_sld_control.regValue = IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg);
	 //od_dma_oddma_wr_rule_check_up.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_Rule_check_up_reg);
	 //od_dma_oddma_rd_rule_check_up.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_Rule_check_up_reg);
	 od_dma_oddma_pq_cmp.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_reg);
	od_dma_oddma_pq_decmp.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_DECMP_reg);
	
	//IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_reg,0x003c0438); //pq_cmp return default disable, k7 remove
	//IoReg_Write32(OD_DMA_ODDMA_PQ_DECMP_reg,0x003c0438); //pq_dcmp return default disable, k7 remove
	//IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_rolling_reg,0x00000439); //cap rolling disable, k7 remove
	//IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_rolling_reg,0x00000439); //disp rolling disable, k7 remove

	color_sld_sld_size.sld_width = 3840;
	color_sld_sld_size.sld_height = 2160;
	IoReg_Write32(COLOR_SLD_SLD_SIZE_reg,color_sld_sld_size.regValue); //scale down width ,0x 0x1e0, height ,0x 0x10e

	od_dma_oddma_wr_num_bl_wrap_line_step.cap_line_step = 0x44;
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_line_step_reg,od_dma_oddma_wr_num_bl_wrap_line_step.regValue); 
	
	od_dma_oddma_rd_num_bl_wrap_line_step.disp_line_step = 0x44;
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_line_step_reg,od_dma_oddma_rd_num_bl_wrap_line_step.regValue); 
	
	od_dma_oddma_wr_num_bl_wrap_word.cap_addr_toggle_mode=0x1;
	od_dma_oddma_wr_num_bl_wrap_word.cap_line_128_num = 0x44;
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_word_reg,od_dma_oddma_wr_num_bl_wrap_word.regValue);

	od_dma_oddma_rd_num_bl_wrap_word.disp_addr_toggle_mode=0x1;
	od_dma_oddma_rd_num_bl_wrap_word.disp_line_128_num=0x44;
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_word_reg,od_dma_oddma_rd_num_bl_wrap_word.regValue);


	od_dma_oddma_wr_num_bl_wrap_ctl.cap_burst_len=0x44;
	od_dma_oddma_wr_num_bl_wrap_ctl.cap_line_num=0x10e;
	od_dma_oddma_wr_num_bl_wrap_ctl.cap_line_toggle_offset=0x5;
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_ctl_reg,od_dma_oddma_wr_num_bl_wrap_ctl.regValue);


	od_dma_oddma_rd_num_bl_wrap_ctl.disp_line_toggle_offset=0x7;
	od_dma_oddma_rd_num_bl_wrap_ctl.disp_line_num = 0x10e;
	od_dma_oddma_rd_num_bl_wrap_ctl.disp_burst_len = 0x44;
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_ctl_reg,od_dma_oddma_rd_num_bl_wrap_ctl.regValue); 

	od_dma_od_dma_ctrl.oddma_mode_sel =0;
	//od_dma_od_dma_ctrl.auto_block_sel_en 
	//od_dma_od_dma_ctrl.set_block_sel
	//od_dma_od_dma_ctrl.cap_block_fw
	od_dma_od_dma_ctrl.pixel_encoding = 0;
	//od_dma_od_dma_ctrl.bit_sel
	IoReg_Write32(OD_DMA_OD_DMA_CTRL_reg,od_dma_od_dma_ctrl.regValue);
	


	od_dma_oddma_wr_ctrl.cap_force_all_rst = 1; //force_all rst
	od_dma_oddma_rd_ctrl.disp_force_all_rst = 1;//force_all rst

	od_dma_oddma_wr_ctrl.cap_en = 1; //cap_en
	od_dma_oddma_rd_ctrl.disp_en = 1;//disp_en

	IoReg_Write32(OD_DMA_ODDMA_WR_Ctrl_reg,od_dma_oddma_wr_ctrl.regValue); 
	IoReg_Write32(OD_DMA_ODDMA_RD_Ctrl_reg,od_dma_oddma_rd_ctrl.regValue);

        //ODDMA double buffer apply
        od_dma_od_dma_db_en.regValue = IoReg_Read32(OD_DMA_OD_DMA_DB_EN_reg);
        od_dma_od_dma_db_en.disp_db_apply = 1;
        od_dma_od_dma_db_en.cap_db_apply= 1;
        IoReg_Write32(OD_DMA_OD_DMA_DB_EN_reg,od_dma_od_dma_ctrl.regValue);

        #ifdef SLD_Debug
        VIPprintf("drvif_color_set_SLD_4k_dma_setting_K7\n");
        #endif

}


char drvif_color_set_SLD_CTRL(unsigned int phy_addr, unsigned int buff_size, unsigned char scale_mode, int panel_width, int panel_height)
{
	//unsigned char sld_dma_return = false;
	color_sld_sld_size_RBUS color_sld_sld_size;
	od_dma_oddma_wr_ctrl_RBUS	   od_dma_oddma_wr_ctrl;
	od_dma_oddma_rd_ctrl_RBUS		od_dma_oddma_rd_ctrl;
	od_dma_oddma_wr_num_bl_wrap_addr_0_RBUS		 od_dma_oddma_wr_num_bl_wrap_addr_0;
	od_dma_oddma_wr_num_bl_wrap_addr_1_RBUS		 od_dma_oddma_wr_num_bl_wrap_addr_1;
	//od_dma_oddma_rd_num_bl_wrap_addr_0_RBUS od_dma_oddma_rd_num_bl_wrap_addr_0;
	od_dma_oddma_wr_num_bl_wrap_word_RBUS	od_dma_oddma_wr_num_bl_wrap_word;
	//od_dma_oddma_rd_num_bl_wrap_addr_1_RBUS od_dma_oddma_rd_num_bl_wrap_addr_1;
	od_dma_oddma_rd_num_bl_wrap_word_RBUS	 od_dma_oddma_rd_num_bl_wrap_word;
	od_dma_oddma_rd_num_bl_wrap_line_step_RBUS		od_dma_oddma_rd_num_bl_wrap_line_step;
	od_dma_oddma_wr_num_bl_wrap_line_step_RBUS	   od_dma_oddma_wr_num_bl_wrap_line_step;
	od_dma_oddma_wr_num_bl_wrap_ctl_RBUS	   od_dma_oddma_wr_num_bl_wrap_ctl;
	od_dma_oddma_rd_num_bl_wrap_ctl_RBUS		od_dma_oddma_rd_num_bl_wrap_ctl;
	od_dma_od_dma_ctrl_RBUS  od_dma_od_dma_ctrl;
	od_dma_od_dma_db_en_RBUS od_dma_od_dma_db_en;
	color_sld_sld_control_RBUS color_sld_control;
	//od_dma_oddma_wr_rule_check_up_RBUS  od_dma_oddma_wr_rule_check_up;
	//od_dma_oddma_rd_rule_check_up_RBUS	od_dma_oddma_rd_rule_check_up;
	od_dma_oddma_pq_cmp_RBUS od_dma_oddma_pq_cmp;
	od_dma_oddma_pq_decmp_RBUS od_dma_oddma_pq_decmp;
	od_od_ctrl_RBUS od_od_ctrl;
	od_dma_oddma_wr_num_bl_wrap_rolling_RBUS od_dma_oddma_wr_num_bl_wrap_rolling;
	od_dma_oddma_rd_num_bl_wrap_rolling_RBUS od_dma_oddma_rd_num_bl_wrap_rolling;
	#if 0
	od_dma_oddma_wr_rule_check_up_RBUS od_dma_oddma_wr_rule_check_up_reg;
	od_dma_oddma_wr_rule_check_low_RBUS od_dma_oddma_wr_rule_check_low_reg;
	od_dma_oddma_rd_num_bl_wrap_addr_0_RBUS od_dma_oddma_rd_num_bl_wrap_addr_0_reg;
	od_dma_oddma_rd_num_bl_wrap_addr_1_RBUS od_dma_oddma_rd_num_bl_wrap_addr_1_reg;
	od_dma_oddma_rd_rule_check_up_RBUS od_dma_oddma_rd_rule_check_up_reg;
	od_dma_oddma_rd_rule_check_low_RBUS od_dma_oddma_rd_rule_check_low_reg;
	#endif

	int cap_line_step=0, disp_line_step=0;
	int cap_line_128_num = 0, disp_line_128_num = 0;
	int cap_line_num = 0, disp_line_num = 0;
	int cap_burst_len = 0, disp_burst_len = 0;
	if (drvif_color_get_HW_MEX_STATUS(MEX_SLD) != 1) {
		rtd_pr_vpq_info("HW_MEX,  MEX_SLD != 1, SLD CTRL return\n");
		return -1;
	}

	
	od_dma_oddma_wr_ctrl.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_Ctrl_reg);
	od_dma_oddma_rd_ctrl.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_Ctrl_reg);
	od_dma_oddma_wr_num_bl_wrap_addr_0.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_addr_0_reg);
	od_dma_oddma_wr_num_bl_wrap_addr_1.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_addr_1_reg);
	od_dma_oddma_wr_num_bl_wrap_word.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_word_reg);
	od_dma_oddma_rd_num_bl_wrap_word.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_word_reg);
	od_dma_oddma_wr_num_bl_wrap_line_step.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_line_step_reg);
	od_dma_oddma_rd_num_bl_wrap_line_step.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_line_step_reg);
	od_dma_oddma_wr_num_bl_wrap_ctl.regValue =  IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_ctl_reg);
	od_dma_oddma_rd_num_bl_wrap_ctl.regValue =  IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_ctl_reg);
	od_dma_od_dma_ctrl.regValue =  IoReg_Read32(OD_DMA_OD_DMA_CTRL_reg);
	od_dma_od_dma_db_en.regValue = IoReg_Read32(OD_DMA_OD_DMA_DB_EN_reg);
	color_sld_control.regValue = IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg);
	od_dma_oddma_pq_cmp.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_reg);
	od_dma_oddma_pq_decmp.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_DECMP_reg);
	od_od_ctrl.regValue = IoReg_Read32(OD_OD_CTRL_reg);
	od_dma_oddma_wr_num_bl_wrap_rolling.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_rolling_reg);
	od_dma_oddma_rd_num_bl_wrap_rolling.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_rolling_reg);
	
	od_dma_od_dma_db_en.cap_db_en = 0;
	od_dma_od_dma_db_en.disp_db_en = 0;
	od_dma_od_dma_db_en.smooth_tog_en = 0;
	IoReg_Write32(OD_DMA_OD_DMA_DB_EN_reg, od_dma_od_dma_db_en.regValue); //db disable

	//sld_dma_return = drvif_set_sld_dma();
#if 1
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_addr_0_reg, phy_addr); //must be check 96 alignment
	//IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_addr_1_reg, align_value+0x02f00000); //Cap_sta_addr_0, must be check 96 alignment
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_addr_1_reg, phy_addr + buff_size); //Cap_sta_addr_0, must be check 96 alignment
	
	IoReg_Write32(OD_DMA_ODDMA_WR_Rule_check_up_reg, phy_addr + buff_size + buff_size); //Cap_up_limit
	IoReg_Write32(OD_DMA_ODDMA_WR_Rule_check_low_reg, phy_addr);
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_addr_0_reg, phy_addr);//must be check 96 alignment IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_addr_0_reg, align_value);//must be check 96 alignment
	//IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_addr_1_reg, align_value+0x02f00000); //Cap_sta_addr_0, must be check 96 alignment
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_addr_1_reg, phy_addr + buff_size); //Cap_sta_addr_0, must be check 96 alignment
	
	
	IoReg_Write32(OD_DMA_ODDMA_RD_Rule_check_up_reg, phy_addr + buff_size + buff_size); //Disp_up_limit
	IoReg_Write32(OD_DMA_ODDMA_RD_Rule_check_low_reg, phy_addr);
#else
	od_dma_oddma_wr_num_bl_wrap_addr_0.cap_sta_addr_0 = phy_addr;
	od_dma_oddma_wr_num_bl_wrap_addr_1.cap_sta_addr_1 = phy_addr + buff_size;
	od_dma_oddma_wr_rule_check_up_reg.cap_up_limit = phy_addr + buff_size + buff_size;
	od_dma_oddma_wr_rule_check_low_reg.cap_low_limit = phy_addr;
	od_dma_oddma_rd_num_bl_wrap_addr_0_reg.disp_sta_addr_0 = phy_addr;
	od_dma_oddma_rd_num_bl_wrap_addr_1_reg.disp_sta_addr_1 = phy_addr + buff_size;
	od_dma_oddma_rd_rule_check_up_reg.disp_up_limit = phy_addr + buff_size + buff_size;
	od_dma_oddma_rd_rule_check_low_reg.disp_low_limit = phy_addr;

	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_addr_0_reg, od_dma_oddma_wr_num_bl_wrap_addr_0.regValue);
#endif
	
	color_sld_sld_size.regValue = IoReg_Read32(COLOR_SLD_SLD_SIZE_reg);
	od_dma_oddma_wr_ctrl.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_Ctrl_reg);
	od_dma_oddma_rd_ctrl.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_Ctrl_reg);
	//od_dma_oddma_wr_num_bl_wrap_addr_0.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_addr_0_reg);
	//od_dma_oddma_wr_num_bl_wrap_addr_1.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_addr_1_reg);
	//od_dma_oddma_rd_num_bl_wrap_addr_0.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_addr_0_reg);
	//od_dma_oddma_rd_num_bl_wrap_addr_1.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_addr_1_reg);
	od_dma_oddma_wr_num_bl_wrap_word.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_word_reg);
	od_dma_oddma_rd_num_bl_wrap_word.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_word_reg);
	od_dma_oddma_rd_num_bl_wrap_line_step.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_line_step_reg);
	od_dma_oddma_wr_num_bl_wrap_line_step.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_line_step_reg);
	od_dma_oddma_wr_num_bl_wrap_ctl.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_ctl_reg);
	od_dma_oddma_rd_num_bl_wrap_ctl.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_ctl_reg);
	od_dma_od_dma_ctrl.regValue = IoReg_Read32(OD_DMA_OD_DMA_CTRL_reg);
	color_sld_control.regValue = IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg);
	//od_dma_oddma_wr_rule_check_up.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_Rule_check_up_reg);
	//od_dma_oddma_rd_rule_check_up.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_Rule_check_up_reg);

	

	//IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_reg,0x003c0438); //pq_cmp return default disable, k7 remove
	//IoReg_Write32(OD_DMA_ODDMA_PQ_DECMP_reg,0x003c0438); //pq_dcmp return default disable, k7 remove

	od_od_ctrl.oddma_clk_en = 0x1;

	IoReg_Write32(OD_OD_CTRL_reg, od_od_ctrl.regValue); //od_clk en

	//IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_rolling_reg,0x00000439); //cap rolling disable, k7 remove
	//IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_rolling_reg,0x00000439); //disp rolling disable, k7 remove
#if 0
	if (scale_mode == 1) {
		color_sld_sld_size.sld_width = 3840;
		color_sld_sld_size.sld_height = 2160;
	} else {
		color_sld_sld_size.sld_width = 1920;
		color_sld_sld_size.sld_height = 1080;
	}
	IoReg_Write32(COLOR_SLD_SLD_SIZE_reg, color_sld_sld_size.regValue); //scale down width ,0x 0x1e0, height ,0x 0x10e

	od_dma_oddma_wr_num_bl_wrap_line_step.cap_line_step = 0x44;
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_line_step_reg, od_dma_oddma_wr_num_bl_wrap_line_step.regValue);

	od_dma_oddma_rd_num_bl_wrap_line_step.disp_line_step = 0x44;
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_line_step_reg, od_dma_oddma_rd_num_bl_wrap_line_step.regValue);

	od_dma_oddma_wr_num_bl_wrap_word.cap_addr_toggle_mode = 0x1;
	od_dma_oddma_wr_num_bl_wrap_word.cap_line_128_num = 0x44;
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_word_reg, od_dma_oddma_wr_num_bl_wrap_word.regValue);

	od_dma_oddma_rd_num_bl_wrap_word.disp_addr_toggle_mode = 0x1;
	od_dma_oddma_rd_num_bl_wrap_word.disp_line_128_num = 0x44;
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_word_reg, od_dma_oddma_rd_num_bl_wrap_word.regValue);


	od_dma_oddma_wr_num_bl_wrap_ctl.cap_burst_len = 0x44;
	od_dma_oddma_wr_num_bl_wrap_ctl.cap_line_num = 0x10e;
	od_dma_oddma_wr_num_bl_wrap_ctl.cap_line_toggle_offset = 0x5;
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_ctl_reg, od_dma_oddma_wr_num_bl_wrap_ctl.regValue);


	od_dma_oddma_rd_num_bl_wrap_ctl.disp_line_toggle_offset = 0x7;
	od_dma_oddma_rd_num_bl_wrap_ctl.disp_line_num = 0x10e;
	od_dma_oddma_rd_num_bl_wrap_ctl.disp_burst_len = 0x44;
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_ctl_reg, od_dma_oddma_rd_num_bl_wrap_ctl.regValue);

	od_dma_oddma_wr_num_bl_wrap_rolling.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_rolling_reg);
	od_dma_oddma_wr_num_bl_wrap_rolling.cap_rolling_enable = 0;
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_rolling_reg, od_dma_oddma_wr_num_bl_wrap_rolling.regValue);


	od_dma_oddma_rd_num_bl_wrap_rolling.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_rolling_reg);
	od_dma_oddma_rd_num_bl_wrap_rolling.disp_rolling_enable = 0;
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_rolling_reg, od_dma_oddma_rd_num_bl_wrap_rolling.regValue);	
#else
		
		if (panel_width==3840 && panel_height==2160) {
			color_sld_sld_size.sld_width = 3840;
			color_sld_sld_size.sld_height = 2160;
			color_sld_control.sld_scale_mode = 0;
		} else if (panel_width==1920 && panel_height==1080) {
			color_sld_sld_size.sld_width = 1920;
			color_sld_sld_size.sld_height = 1080;
			color_sld_control.sld_scale_mode = 0;
		}else if (panel_width==7680 && panel_height==4320){
			color_sld_sld_size.sld_width = 7680;
			color_sld_sld_size.sld_height = 4320;
			color_sld_control.sld_scale_mode = 2;
		}
		IoReg_Write32(COLOR_SLD_SLD_SIZE_reg, color_sld_sld_size.regValue);
		IoReg_Write32(COLOR_SLD_SLD_CONTROL_reg, color_sld_control.regValue);

		rtd_pr_vpq_info("SLD_HW, panel_width:%d,  panel_height:%d  sld_scale_mode:%d\n",panel_width,panel_height, color_sld_control.sld_scale_mode);
		cap_line_128_num = ceil_SLD((color_sld_sld_size.sld_width/(4*(1<<color_sld_control.sld_scale_mode))) * 18 , 128);
		disp_line_128_num = ceil_SLD((color_sld_sld_size.sld_width/(4*(1<<color_sld_control.sld_scale_mode))) * 18 , 128);
		
		cap_line_step = cap_line_128_num;
		disp_line_step = disp_line_128_num;
		
	      cap_line_num = ceil_SLD(color_sld_sld_size.sld_height, (4*(1<<color_sld_control.sld_scale_mode)));
		disp_line_num = ceil_SLD(color_sld_sld_size.sld_height, (4*(1<<color_sld_control.sld_scale_mode)));
		
	 	cap_burst_len = cap_line_128_num;
		disp_burst_len = disp_line_128_num;

		od_dma_oddma_wr_num_bl_wrap_line_step.cap_line_step = cap_line_step;
		IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_line_step_reg, od_dma_oddma_wr_num_bl_wrap_line_step.regValue);

		od_dma_oddma_rd_num_bl_wrap_line_step.disp_line_step = disp_line_step;
		IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_line_step_reg, od_dma_oddma_rd_num_bl_wrap_line_step.regValue);

		od_dma_oddma_wr_num_bl_wrap_word.cap_addr_toggle_mode = 0x1;
		od_dma_oddma_wr_num_bl_wrap_word.cap_line_128_num = cap_line_128_num;
		IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_word_reg, od_dma_oddma_wr_num_bl_wrap_word.regValue);

		od_dma_oddma_rd_num_bl_wrap_word.disp_addr_toggle_mode = 0x1;
		od_dma_oddma_rd_num_bl_wrap_word.disp_line_128_num = disp_line_128_num;
		IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_word_reg, od_dma_oddma_rd_num_bl_wrap_word.regValue);


		od_dma_oddma_wr_num_bl_wrap_ctl.cap_burst_len = cap_burst_len;
		od_dma_oddma_wr_num_bl_wrap_ctl.cap_line_num = cap_line_num;
		od_dma_oddma_wr_num_bl_wrap_ctl.cap_line_toggle_offset = 0x5;
		IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_ctl_reg, od_dma_oddma_wr_num_bl_wrap_ctl.regValue);


		od_dma_oddma_rd_num_bl_wrap_ctl.disp_line_toggle_offset = 0x7;
		od_dma_oddma_rd_num_bl_wrap_ctl.disp_line_num = disp_line_num;
		od_dma_oddma_rd_num_bl_wrap_ctl.disp_burst_len = disp_burst_len;
		IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_ctl_reg, od_dma_oddma_rd_num_bl_wrap_ctl.regValue);

		od_dma_oddma_wr_num_bl_wrap_rolling.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_rolling_reg);
		od_dma_oddma_wr_num_bl_wrap_rolling.cap_rolling_enable = 0;
		IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_rolling_reg, od_dma_oddma_wr_num_bl_wrap_rolling.regValue);


		od_dma_oddma_rd_num_bl_wrap_rolling.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_rolling_reg);
		od_dma_oddma_rd_num_bl_wrap_rolling.disp_rolling_enable = 0;
		IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_rolling_reg, od_dma_oddma_rd_num_bl_wrap_rolling.regValue);		

		rtd_pr_vpq_info("SLD_HW, cap_line_128_num:%d,  disp_line_128_num:%d\n",cap_line_128_num,disp_line_128_num);
		rtd_pr_vpq_info("SLD_HW, cap_line_num:%d,  disp_line_num:%d\n",cap_line_num,disp_line_num);
#endif




	od_dma_od_dma_ctrl.oddma_mode_sel = 0;
	//od_dma_od_dma_ctrl.auto_block_sel_en
	//od_dma_od_dma_ctrl.set_block_sel
	//od_dma_od_dma_ctrl.cap_block_fw
	od_dma_od_dma_ctrl.pixel_encoding = 0;
	//od_dma_od_dma_ctrl.bit_sel
	IoReg_Write32(OD_DMA_OD_DMA_CTRL_reg, od_dma_od_dma_ctrl.regValue);



	od_dma_oddma_wr_ctrl.cap_force_all_rst = 1; //force_all rst
	od_dma_oddma_rd_ctrl.disp_force_all_rst = 1;//force_all rst

	od_dma_oddma_wr_ctrl.cap_en = 1; //cap_en
	od_dma_oddma_rd_ctrl.disp_en = 1;//disp_en

	IoReg_Write32(OD_DMA_ODDMA_WR_Ctrl_reg, od_dma_oddma_wr_ctrl.regValue);
	IoReg_Write32(OD_DMA_ODDMA_RD_Ctrl_reg, od_dma_oddma_rd_ctrl.regValue);

	//ODDMA double buffer apply
	od_dma_od_dma_db_en.regValue = IoReg_Read32(OD_DMA_OD_DMA_DB_EN_reg);
	od_dma_od_dma_db_en.disp_db_apply = 1;
	od_dma_od_dma_db_en.cap_db_apply = 1;
	IoReg_Write32(OD_DMA_OD_DMA_DB_EN_reg, od_dma_od_dma_ctrl.regValue);

	return 1;
}	
/*
void SLD_k7_HAL(int Y1, int Y2, int Y3, int gain_low,unsigned char UIsel_L)
{

#ifdef VPQ_COMPILER_ERROR_ENABLE_MERLIN6
	int reg_seg0 = 256;  //fixed
	int reg_seg1 = 768;  //fixed
	 
	int reg_gain0 = 0 ;//fixed
	int reg_gain1 = 0 ; //fixed
	int reg_gain2 = 0 ; //fixed
	 
	int reg_offset0 = 0; //fixed
	int reg_offset1 = 0;
	int reg_offset2 = 0;

	color_sld_sld_control_RBUS color_sld_control;
	color_sld_sld_drop_RBUS color_sld_drop;
	
	DRV_SLD_CurveMap sld_curve_map;
	 
	int y_out_target[4] = {0};

	color_sld_control.regValue = IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg);
	color_sld_drop.regValue = IoReg_Read32(COLOR_SLD_SLD_DROP_reg);
	
	color_sld_control.sld_en = 0;
	color_sld_control.sld_blend_en= 1;
	color_sld_control.sld_global_drop= 0;
	color_sld_control.sld_hpf_type= 0;
	color_sld_control.sld_hpf_thl= 255;
	color_sld_control.sld_scale_mode= 1;
	color_sld_drop.sld_cnt_thl = 0;
	color_sld_drop.sld_hpf_diff_thl = 16;
	color_sld_drop.sld_pixel_diff_thl = 16;
	

	IoReg_Write32(COLOR_SLD_SLD_CONTROL_reg, color_sld_control.regValue);
	IoReg_Write32(COLOR_SLD_SLD_DROP_reg, color_sld_drop.regValue);

 
	if(UIsel_L == 1)
	{
	        y_out_target[0] =  (0    *gain_low)/1024;
	        y_out_target[1] =  ( Y1 *gain_low)/1024;
	        y_out_target[2] =  ( Y2 *gain_low)/1024;
	        y_out_target[3] =  ( Y3 *gain_low)/1024;
	}
	else
	{
	        y_out_target[0] =  (0   *1024)/1024;
	        y_out_target[1] =  (Y1 *1024)/1024;
	        y_out_target[2] =  (Y2 *1024)/1024;
	        y_out_target[3] =  (Y3 *1024)/1024;
	 
	}
	 
	reg_offset0 = y_out_target[0];
	reg_gain0 =( (y_out_target[1]- reg_offset0)*64)/reg_seg0;
	reg_gain1 =( (y_out_target[2]- reg_offset0-((reg_gain0*reg_seg0)/64))*64)/(reg_seg1-reg_seg0);
	reg_gain2 =( (y_out_target[3]- reg_offset0-((reg_gain0*reg_seg0)/64)-((reg_gain1*(reg_seg1-reg_seg0))/64))*64)/(1023-reg_seg1);
	 
	reg_offset1 =  reg_offset0 + (reg_gain0*reg_seg0)/64 - (reg_gain1*reg_seg0)/64;
	reg_offset2 =  reg_offset1 + (reg_gain1*reg_seg1)/64 - (reg_gain2*reg_seg1)/64;

 	sld_curve_map.sld_curve_seg_0 = 256; 	
	sld_curve_map.sld_curve_seg_1 = 768;
	
	if(reg_gain0 < 0)
	{
	 	sld_curve_map.sld_curve_gain_0= 256 - (short)reg_gain0; 	
	}
	else
	{
	 	sld_curve_map.sld_curve_gain_0= (short)reg_gain0; 	
	}
	if(reg_gain1 < 0)
	{
	 	sld_curve_map.sld_curve_gain_1= 256 - (short)reg_gain1; 	
	}
	else
	{
	 	sld_curve_map.sld_curve_gain_1= (short)reg_gain1; 	
	}
	if(reg_gain2 < 0)
	{
	 	sld_curve_map.sld_curve_gain_2= 256 - (short)reg_gain2; 	
	}
	else
	{
	 	sld_curve_map.sld_curve_gain_2= (short)reg_gain2; 	
	}


	if(reg_offset0 < 0)
	{
		sld_curve_map.sld_curve_offset_0= 2048 - (short)reg_offset0; 		
	}
	else
	{
		sld_curve_map.sld_curve_offset_0= (short)reg_offset0; 		
	}

	if(reg_offset1 < 0)
	{
		sld_curve_map.sld_curve_offset_1= 2048 - (short)reg_offset1; 		
	}
	else
	{
		sld_curve_map.sld_curve_offset_1= (short)reg_offset1; 		
	}

	if(reg_offset2 < 0)
	{
		sld_curve_map.sld_curve_offset_2= 2048 - (short)reg_offset2; 		
	}
	else
	{
		sld_curve_map.sld_curve_offset_2= (short)reg_offset2; 		
	}


	drvif_color_set_SLD_curve_map(&sld_curve_map);

#endif
	

}
*/
#if 1//k7 add

void drvif_color_set_SLD_Enable(unsigned char enable)
{
	color_sld_sld_control_RBUS color_sld_control;
	color_sld_control.regValue = IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg);

	color_sld_control.sld_en= enable;

	#ifdef SLD_Debug
	VIPprintf("sld_valid = %d\n",color_sld_control.sld_en);
	#endif

	IoReg_Write32(COLOR_SLD_SLD_CONTROL_reg, color_sld_control.regValue);
}

void drvif_color_set_SLD_global_drop(void)
{

	color_sld_sld_control_RBUS color_sld_control;
	color_sld_control.regValue = IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg);
	
	color_sld_control.sld_global_drop						= 1;
	
	IoReg_Write32(COLOR_SLD_SLD_CONTROL_reg, color_sld_control.regValue);
}

void drvif_color_set_SLD_disable_global_drop(void)
{

	color_sld_sld_control_RBUS color_sld_control;
	color_sld_control.regValue = IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg);
	
	color_sld_control.sld_global_drop						= 0;
	
	IoReg_Write32(COLOR_SLD_SLD_CONTROL_reg, color_sld_control.regValue);
}


void drvif_color_set_SLD_control(DRV_SLD_sld_control *ptr)
{

	color_sld_sld_control_RBUS color_sld_control;
	color_sld_control.regValue = IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg);
	
	color_sld_control.sld_add_comp_dy							= ptr->sld_add_comp_dy;
	//color_sld_control.sld_scale_mode						= ptr->sld_scale_mode;
	color_sld_control.sld_disable_scale_up				= ptr->sld_disable_scale_up;
	color_sld_control.sld_disable_lpf						= ptr->sld_disable_lpf;
	color_sld_control.sld_idx_count_period				= ptr->sld_idx_count_period;
	color_sld_control.sld_idx_input_sel					= ptr->sld_idx_input_sel;
	color_sld_control.sld_window_en						= ptr->sld_window_en;
	color_sld_control.sld_hpf_type							= ptr->sld_hpf_type;
	color_sld_control.sld_global_drop						= ptr->sld_global_drop;
	color_sld_control.sld_blend_en							= ptr->sld_blend_en;
	color_sld_control.sld_debug_mode						= ptr->sld_debug_mode;
	////color_sld_control.sld_en								= ptr->sld_en;
	
	IoReg_Write32(COLOR_SLD_SLD_CONTROL_reg, color_sld_control.regValue);
}

void drvif_color_set_SLD_window_en(DRV_SLD_sld_control *ptr)
{

	color_sld_sld_control_RBUS color_sld_control;
	color_sld_control.regValue = IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg);
	
	color_sld_control.sld_window_en						= ptr->sld_window_en;
	
	IoReg_Write32(COLOR_SLD_SLD_CONTROL_reg, color_sld_control.regValue);
}


#if 0
void drvif_color_set_SLD_size(DRV_SLD_sld_size *ptr)
{

	color_sld_sld_size_RBUS color_sld_size;
	color_sld_size.regValue = IoReg_Read32(COLOR_SLD_SLD_SIZE_reg);
	
	color_sld_size.sld_width							= ptr->sld_width;
	color_sld_size.sld_height						= ptr->sld_height;
	
	IoReg_Write32(COLOR_SLD_SLD_SIZE_reg, color_sld_size.regValue);
}

void drvif_color_set_SLD_rgb2y_coef(DRV_SLD_sld_rgb2y_coef *ptr)
{

	color_sld_sld_rgb2y_coef_1_RBUS color_rgb2y_coef_1;
	color_sld_sld_rgb2y_coef_2_RBUS color_rgb2y_coef_2;
	color_rgb2y_coef_1.regValue = IoReg_Read32(COLOR_SLD_SLD_RGB2Y_coef_1_reg);
	color_rgb2y_coef_2.regValue = IoReg_Read32(COLOR_SLD_SLD_RGB2Y_coef_2_reg);
	
	color_rgb2y_coef_1.coef_r							= ptr->coef_r;
	color_rgb2y_coef_1.coef_g						    = ptr->coef_g;
	color_rgb2y_coef_2.coef_b				            = ptr->coef_b;
	
	IoReg_Write32(COLOR_SLD_SLD_RGB2Y_coef_1_reg, color_rgb2y_coef_1.regValue);
	IoReg_Write32(COLOR_SLD_SLD_RGB2Y_coef_2_reg, color_rgb2y_coef_2.regValue);
}
#endif

void drvif_color_set_SLD_Y_curve_map(DRV_SLD_sld_Y_curve_map *ptr)
{

	color_sld_sld_curve_map_seg_RBUS            sld_Y_curve_map;        
	color_sld_sld_new_curve_map_seg_1_RBUS      sld_new_curve_map_seg_1;  
	color_sld_sld_new_curve_map_gain_0_RBUS     sld_new_curve_map_gain_0; 
	color_sld_sld_new_curve_map_gain_1_RBUS     sld_new_curve_map_gain_1;   
	color_sld_sld_new_curve_map_offset_0_RBUS   sld_new_curve_map_offset_0; 
	color_sld_sld_new_curve_map_offset_1_RBUS   sld_new_curve_map_offset_1;
		
	sld_Y_curve_map.regValue 		 	= IoReg_Read32(COLOR_SLD_SLD_CURVE_MAP_SEG_reg);
	sld_new_curve_map_seg_1.regValue 	= IoReg_Read32(COLOR_SLD_SLD_NEW_CURVE_MAP_SEG_1_reg);
	sld_new_curve_map_gain_0.regValue 	= IoReg_Read32(COLOR_SLD_SLD_NEW_CURVE_MAP_GAIN_0_reg);
	sld_new_curve_map_gain_1.regValue 	= IoReg_Read32(COLOR_SLD_SLD_NEW_CURVE_MAP_GAIN_1_reg);
	sld_new_curve_map_offset_0.regValue = IoReg_Read32(COLOR_SLD_SLD_NEW_CURVE_MAP_OFFSET_0_reg);
	sld_new_curve_map_offset_1.regValue = IoReg_Read32(COLOR_SLD_SLD_NEW_CURVE_MAP_OFFSET_1_reg);
	
	sld_Y_curve_map.sld_curve_seg_0							= ptr->sld_curve_seg_0;
	sld_Y_curve_map.sld_curve_seg_1						    = ptr->sld_curve_seg_1;
	sld_new_curve_map_seg_1.sld_curve_seg_2				    = ptr->sld_curve_seg_2;

	sld_new_curve_map_gain_0.sld_new_curve_gain_0			= ptr->sld_new_curve_gain_0;
	sld_new_curve_map_gain_0.sld_new_curve_gain_1			= ptr->sld_new_curve_gain_1;
	sld_new_curve_map_gain_1.sld_new_curve_gain_2			= ptr->sld_new_curve_gain_2;
	sld_new_curve_map_gain_1.sld_new_curve_gain_3			= ptr->sld_new_curve_gain_3;


	sld_new_curve_map_offset_0.sld_new_curve_offset_0		= ptr->sld_new_curve_offset_0;
	sld_new_curve_map_offset_0.sld_new_curve_offset_1		= ptr->sld_new_curve_offset_1;
	sld_new_curve_map_offset_1.sld_new_curve_offset_2		= ptr->sld_new_curve_offset_2;
	sld_new_curve_map_offset_1.sld_new_curve_offset_3		= ptr->sld_new_curve_offset_3;


	IoReg_Write32(COLOR_SLD_SLD_CURVE_MAP_SEG_reg			, sld_Y_curve_map.regValue 			    );		 	
	IoReg_Write32(COLOR_SLD_SLD_NEW_CURVE_MAP_SEG_1_reg     , sld_new_curve_map_seg_1.regValue 	    );
	IoReg_Write32(COLOR_SLD_SLD_NEW_CURVE_MAP_GAIN_0_reg    , sld_new_curve_map_gain_0.regValue 	);
	IoReg_Write32(COLOR_SLD_SLD_NEW_CURVE_MAP_GAIN_1_reg    , sld_new_curve_map_gain_1.regValue 	);
	IoReg_Write32(COLOR_SLD_SLD_NEW_CURVE_MAP_OFFSET_0_reg  , sld_new_curve_map_offset_0.regValue   );
	IoReg_Write32(COLOR_SLD_SLD_NEW_CURVE_MAP_OFFSET_1_reg  , sld_new_curve_map_offset_1.regValue   );
	
	
	
}

void drvif_color_set_SLD_Idx_curve_map(DRV_SLD_sld_idx_map_seg *ptr)
{

	color_sld_sld_idx_map_seg_RBUS            sld_idx_map_seg_;        
	color_sld_sld_new_idx_map_seg_1_RBUS      sld_new_curve_map_seg_1;  
	color_sld_sld_new_idx_map_gain_0_RBUS     sld_new_curve_map_gain_0; 
	color_sld_sld_new_idx_map_gain_1_RBUS     sld_new_curve_map_gain_1;   
	color_sld_sld_new_idx_map_offset_0_RBUS   sld_new_curve_map_offset_0; 
	color_sld_sld_new_idx_map_offset_1_RBUS   sld_new_curve_map_offset_1;
		
	sld_idx_map_seg_.regValue 		 	= IoReg_Read32(COLOR_SLD_SLD_CURVE_MAP_SEG_reg);
	sld_new_curve_map_seg_1.regValue 	= IoReg_Read32(COLOR_SLD_SLD_NEW_CURVE_MAP_SEG_1_reg);
	sld_new_curve_map_gain_0.regValue 	= IoReg_Read32(COLOR_SLD_SLD_NEW_CURVE_MAP_GAIN_0_reg);
	sld_new_curve_map_gain_1.regValue 	= IoReg_Read32(COLOR_SLD_SLD_NEW_CURVE_MAP_GAIN_1_reg);
	sld_new_curve_map_offset_0.regValue = IoReg_Read32(COLOR_SLD_SLD_NEW_CURVE_MAP_OFFSET_0_reg);
	sld_new_curve_map_offset_1.regValue = IoReg_Read32(COLOR_SLD_SLD_NEW_CURVE_MAP_OFFSET_1_reg);
	
	sld_idx_map_seg_.sld_idx_seg_0							= ptr->sld_idx_seg_0;
	sld_idx_map_seg_.sld_idx_seg_1						    = ptr->sld_idx_seg_1;
	sld_new_curve_map_seg_1.sld_idx_seg_2				    = ptr->sld_idx_seg_2;

	sld_new_curve_map_gain_0.sld_new_idx_gain_0			= ptr->sld_new_idx_gain_0;
	sld_new_curve_map_gain_0.sld_new_idx_gain_1			= ptr->sld_new_idx_gain_1;
	sld_new_curve_map_gain_1.sld_new_idx_gain_2			= ptr->sld_new_idx_gain_2;
	sld_new_curve_map_gain_1.sld_new_idx_gain_3			= ptr->sld_new_idx_gain_3;


	sld_new_curve_map_offset_0.sld_new_idx_offset_0		= ptr->sld_new_idx_offset_0;
	sld_new_curve_map_offset_0.sld_new_idx_offset_1		= ptr->sld_new_idx_offset_1;
	sld_new_curve_map_offset_1.sld_new_idx_offset_2		= ptr->sld_new_idx_offset_2;
	sld_new_curve_map_offset_1.sld_new_idx_offset_3		= ptr->sld_new_idx_offset_3;


	IoReg_Write32(COLOR_SLD_SLD_CURVE_MAP_SEG_reg			, sld_idx_map_seg_.regValue 			);		 	
	IoReg_Write32(COLOR_SLD_SLD_NEW_CURVE_MAP_SEG_1_reg     , sld_new_curve_map_seg_1.regValue 	    );
	IoReg_Write32(COLOR_SLD_SLD_NEW_CURVE_MAP_GAIN_0_reg    , sld_new_curve_map_gain_0.regValue 	);
	IoReg_Write32(COLOR_SLD_SLD_NEW_CURVE_MAP_GAIN_1_reg    , sld_new_curve_map_gain_1.regValue 	);
	IoReg_Write32(COLOR_SLD_SLD_NEW_CURVE_MAP_OFFSET_0_reg  , sld_new_curve_map_offset_0.regValue   );
	IoReg_Write32(COLOR_SLD_SLD_NEW_CURVE_MAP_OFFSET_1_reg  , sld_new_curve_map_offset_1.regValue   );
		
}

void drvif_color_set_SLD_Idx_add_con(DRV_SLD_sld_acc_0 *ptr)
{

	color_sld_sld_acc_0_RBUS            sld_sld_acc_0;        
	color_sld_sld_hpf_0_RBUS      sld_sld_hpf_0;  
	color_sld_sld_acc_1_RBUS     sld_acc_1; 
		
	sld_sld_acc_0.regValue 		 	= IoReg_Read32(COLOR_SLD_SLD_ACC_0_reg);
	sld_sld_hpf_0.regValue 	= IoReg_Read32(COLOR_SLD_SLD_HPF_0_reg);
	sld_acc_1.regValue 	= IoReg_Read32(COLOR_SLD_SLD_ACC_1_reg);
	
	sld_sld_acc_0.sld_idx_acc_condition_and							= ptr->sld_idx_acc_condition_and;
	sld_sld_acc_0.sld_max_idx_thl									= ptr->sld_max_idx_thl;
	sld_sld_acc_0.sld_frame_diffy_thl								= ptr->sld_frame_diffy_thl;	
	sld_sld_hpf_0.sld_hpf_logo_add_thl								= ptr->sld_hpf_logo_add_thl;
	sld_acc_1.sld_y_idx_thl											= ptr->sld_y_idx_thl;



	IoReg_Write32(COLOR_SLD_SLD_ACC_0_reg			, sld_sld_acc_0.regValue 			);		 	
	IoReg_Write32(COLOR_SLD_SLD_HPF_0_reg     , sld_sld_hpf_0.regValue 	    );
	IoReg_Write32(COLOR_SLD_SLD_ACC_1_reg    , sld_acc_1.regValue 	);
		
}

void drvif_color_set_SLD_8dir_12tab(DRV_SLD_sld_in_logo_thl_0 *ptr)
{

	color_sld_sld_in_logo_thl_0_RBUS   sld_in_logo_thl_0;        
	color_sld_sld_in_logo_thl_1_RBUS   sld_in_logo_thl_1;        
	color_sld_sld_in_logo_thl_2_RBUS   sld_in_logo_thl_2;        
	color_sld_sld_in_logo_thl_3_RBUS   sld_in_logo_thl_3;        
	color_sld_sld_in_logo_thl_4_RBUS   sld_in_logo_thl_4;        
	color_sld_sld_in_logo_thl_5_RBUS   sld_in_logo_thl_5;        
	
	sld_in_logo_thl_0.regValue 		 	= IoReg_Read32(COLOR_SLD_SLD_IN_LOGO_THL_0_reg);
	sld_in_logo_thl_1.regValue 		 	= IoReg_Read32(COLOR_SLD_SLD_IN_LOGO_THL_1_reg);
	sld_in_logo_thl_2.regValue 		 	= IoReg_Read32(COLOR_SLD_SLD_IN_LOGO_THL_2_reg);
	sld_in_logo_thl_3.regValue 		 	= IoReg_Read32(COLOR_SLD_SLD_IN_LOGO_THL_3_reg);
	sld_in_logo_thl_4.regValue 		 	= IoReg_Read32(COLOR_SLD_SLD_IN_LOGO_THL_4_reg);
	sld_in_logo_thl_5.regValue 		 	= IoReg_Read32(COLOR_SLD_SLD_IN_LOGO_THL_5_reg);
	
	sld_in_logo_thl_0.sld_inter_thl_0							= ptr->sld_inter_thl_0;
	sld_in_logo_thl_0.sld_inter_thl_1							= ptr->sld_inter_thl_1;
	sld_in_logo_thl_1.sld_inter_thl_2							= ptr->sld_inter_thl_2;
	sld_in_logo_thl_1.sld_inter_thl_3							= ptr->sld_inter_thl_3;
	sld_in_logo_thl_2.sld_inter_thl_4							= ptr->sld_inter_thl_4;
	sld_in_logo_thl_2.sld_inter_thl_5							= ptr->sld_inter_thl_5;
	sld_in_logo_thl_3.sld_inter_thl_6							= ptr->sld_inter_thl_6;
	sld_in_logo_thl_3.sld_inter_thl_7							= ptr->sld_inter_thl_7;
	sld_in_logo_thl_4.sld_inter_thl_8							= ptr->sld_inter_thl_8;
	sld_in_logo_thl_4.sld_inter_thl_9							= ptr->sld_inter_thl_9;
	sld_in_logo_thl_5.sld_inter_thl_10							= ptr->sld_inter_thl_10;
	sld_in_logo_thl_5.sld_inter_thl_11							= ptr->sld_inter_thl_11;
	
	

	IoReg_Write32(COLOR_SLD_SLD_IN_LOGO_THL_0_reg			, sld_in_logo_thl_0.regValue 			);	
	IoReg_Write32(COLOR_SLD_SLD_IN_LOGO_THL_1_reg			, sld_in_logo_thl_1.regValue 			);	
	IoReg_Write32(COLOR_SLD_SLD_IN_LOGO_THL_2_reg			, sld_in_logo_thl_2.regValue 			);	
	IoReg_Write32(COLOR_SLD_SLD_IN_LOGO_THL_3_reg			, sld_in_logo_thl_3.regValue 			);	
	IoReg_Write32(COLOR_SLD_SLD_IN_LOGO_THL_4_reg			, sld_in_logo_thl_4.regValue 			);	
	IoReg_Write32(COLOR_SLD_SLD_IN_LOGO_THL_5_reg			, sld_in_logo_thl_5.regValue 			);	
	
}


void drvif_color_set_SLD_lpf(DRV_SLD_sld_lpf_0 *ptr)
{

	color_sld_sld_lpf_0_RBUS	 sld_lpf_0;
	color_sld_sld_lpf_1_RBUS     sld_lpf_1;
	color_sld_sld_lpf_2_RBUS     sld_lpf_2;
	color_sld_sld_lpf_3_RBUS     sld_lpf_3;
	color_sld_sld_lpf_4_RBUS     sld_lpf_4;
	color_sld_sld_lpf_5_RBUS     sld_lpf_5;
	color_sld_sld_lpf_6_RBUS     sld_lpf_6;
	color_sld_sld_lpf_7_RBUS     sld_lpf_7;
	color_sld_sld_lpf_8_RBUS     sld_lpf_8;
	color_sld_sld_lpf_9_RBUS     sld_lpf_9;
	color_sld_sld_lpf_10_RBUS    sld_lpf_10;
	color_sld_sld_lpf_11_RBUS    sld_lpf_11;
	color_sld_sld_lpf_12_RBUS    sld_lpf_12;
	color_sld_sld_lpf_div_RBUS   sld_lpf_div;
	
	sld_lpf_0.regValue 		 	    = IoReg_Read32(COLOR_SLD_SLD_LPF_0_reg );
	sld_lpf_1.regValue 		 	    = IoReg_Read32(COLOR_SLD_SLD_LPF_1_reg );
	sld_lpf_2.regValue 		 	    = IoReg_Read32(COLOR_SLD_SLD_LPF_2_reg );
	sld_lpf_3.regValue 		 	    = IoReg_Read32(COLOR_SLD_SLD_LPF_3_reg );
	sld_lpf_4.regValue 		 	    = IoReg_Read32(COLOR_SLD_SLD_LPF_4_reg );
	sld_lpf_5.regValue 		 	    = IoReg_Read32(COLOR_SLD_SLD_LPF_5_reg );
	sld_lpf_6.regValue 		 	    = IoReg_Read32(COLOR_SLD_SLD_LPF_6_reg );
	sld_lpf_7.regValue 		 	    = IoReg_Read32(COLOR_SLD_SLD_LPF_7_reg );
	sld_lpf_8.regValue 		 	    = IoReg_Read32(COLOR_SLD_SLD_LPF_8_reg );
	sld_lpf_9.regValue 		 	    = IoReg_Read32(COLOR_SLD_SLD_LPF_9_reg );
	sld_lpf_10.regValue 		 	= IoReg_Read32(COLOR_SLD_SLD_LPF_10_reg);
	sld_lpf_11.regValue 		 	= IoReg_Read32(COLOR_SLD_SLD_LPF_11_reg);
	sld_lpf_12.regValue 		 	= IoReg_Read32(COLOR_SLD_SLD_LPF_12_reg);
	sld_lpf_div.regValue 		 	= IoReg_Read32(COLOR_SLD_SLD_LPF_DIV_reg);
	        
	sld_lpf_0.coef_0_0							= ptr->coef_0_0;
	sld_lpf_0.coef_0_1							= ptr->coef_0_1;
	
	sld_lpf_1.coef_0_2							= ptr->coef_0_2;
	sld_lpf_1.coef_0_3							= ptr->coef_0_3;
	
	sld_lpf_2.coef_0_4							= ptr->coef_0_4;
	sld_lpf_2.coef_1_0							= ptr->coef_1_0;
	                                                   
	sld_lpf_3.coef_1_1							= ptr->coef_1_1;
	sld_lpf_3.coef_1_2							= ptr->coef_1_2;
	                                                   
	sld_lpf_4.coef_1_3							= ptr->coef_1_3;
	sld_lpf_4.coef_1_4							= ptr->coef_1_4;
	                                                   
	sld_lpf_5.coef_2_0							= ptr->coef_2_0;
	sld_lpf_5.coef_2_1							= ptr->coef_2_1;
	                                                   
	sld_lpf_6.coef_2_2							= ptr->coef_2_2;
	sld_lpf_6.coef_2_3							= ptr->coef_2_3;
	                                                   
	sld_lpf_7.coef_2_4							= ptr->coef_2_4;
	sld_lpf_7.coef_3_0							= ptr->coef_3_0;
	                                                   
	sld_lpf_8.coef_3_1							= ptr->coef_3_1;
	sld_lpf_8.coef_3_2							= ptr->coef_3_2;
	                                                   
	sld_lpf_9.coef_3_3							= ptr->coef_3_3;
	sld_lpf_9.coef_3_4							= ptr->coef_3_4;
	
	sld_lpf_10.coef_4_0							= ptr->coef_4_0;
	sld_lpf_10.coef_4_1							= ptr->coef_4_1;
	                                                   
	sld_lpf_11.coef_4_2							= ptr->coef_4_2;
	sld_lpf_11.coef_4_3							= ptr->coef_4_3;
	                                                   
	sld_lpf_12.coef_4_4							= ptr->coef_4_4;
	
	sld_lpf_div.lpf_shift_bit							= ptr->lpf_shift_bit;
	

	IoReg_Write32(COLOR_SLD_SLD_LPF_0_reg 			, sld_lpf_0.regValue 			);	
	IoReg_Write32(COLOR_SLD_SLD_LPF_1_reg 			, sld_lpf_1.regValue 			);	
	IoReg_Write32(COLOR_SLD_SLD_LPF_2_reg 			, sld_lpf_2.regValue 			);	
	IoReg_Write32(COLOR_SLD_SLD_LPF_3_reg 			, sld_lpf_3.regValue 			);	
	IoReg_Write32(COLOR_SLD_SLD_LPF_4_reg 			, sld_lpf_4.regValue 			);	
	IoReg_Write32(COLOR_SLD_SLD_LPF_5_reg 			, sld_lpf_5.regValue 			);	
	IoReg_Write32(COLOR_SLD_SLD_LPF_6_reg 			, sld_lpf_6.regValue 			);	
	IoReg_Write32(COLOR_SLD_SLD_LPF_7_reg 			, sld_lpf_7.regValue 			);	
	IoReg_Write32(COLOR_SLD_SLD_LPF_8_reg 			, sld_lpf_8.regValue 			);	
	IoReg_Write32(COLOR_SLD_SLD_LPF_9_reg 			, sld_lpf_9.regValue 			);	
	IoReg_Write32(COLOR_SLD_SLD_LPF_10_reg			, sld_lpf_10.regValue 			);	
	IoReg_Write32(COLOR_SLD_SLD_LPF_11_reg			, sld_lpf_11.regValue 			);	
	IoReg_Write32(COLOR_SLD_SLD_LPF_12_reg			, sld_lpf_12.regValue 			);	
	IoReg_Write32(COLOR_SLD_SLD_LPF_DIV_reg			, sld_lpf_div.regValue 			);	

}

void drvif_color_set_SLD_step(DRV_SLD_sld_hpf_1 *ptr)
{

	color_sld_sld_hpf_1_RBUS	 sld_hpf_1;
	
	sld_hpf_1.regValue  = IoReg_Read32(COLOR_SLD_SLD_HPF_1_reg );
	        
	sld_hpf_1.sld_idx_sign_0	= ptr->sld_idx_sign_0;
	sld_hpf_1.sld_idx_step_0	= ptr->sld_idx_step_0;
	sld_hpf_1.sld_idx_sign_2	= ptr->sld_idx_sign_2;
	sld_hpf_1.sld_idx_step_2	= ptr->sld_idx_step_2;
	

	IoReg_Write32(COLOR_SLD_SLD_HPF_1_reg 			, sld_hpf_1.regValue 			);	

}

void drvif_color_set_SLD_drop(DRV_SLD_sld_drop *ptr,DRV_SLD_sld_idx_not_drop_0 *ptr_in_logo, DRV_SLD_sld_neighbor_not_drop *ptr_n_notDrop,DRV_SLD_sld_neighbor_drop_0 *ptr_nDrop, DRV_SLD_sld_idx_protect_by_neighbor_0 *ptr_dropProtect)
{

	color_sld_sld_drop_RBUS	 sld_drop;
	color_sld_sld_idx_not_drop_0_RBUS idx_not_drop_0;
	color_sld_sld_idx_not_drop_1_RBUS idx_not_drop_1;
	color_sld_sld_neighbor_not_drop_RBUS neighbor_not_drop;
	color_sld_sld_neighbor_drop_0_RBUS neighbor_drop_0 ;
	color_sld_sld_neighbor_drop_1_RBUS neighbor_drop_1 ;
	color_sld_sld_idx_protect_by_neighbor_0_RBUS protect_by_neighbor;
	color_sld_sld_idx_protect_by_neighbor_1_RBUS protect_by_neighbor_1;	
	
	sld_drop.regValue  					= IoReg_Read32(COLOR_SLD_SLD_DROP_reg 			   );
	idx_not_drop_0.regValue  			= IoReg_Read32(COLOR_SLD_SLD_Idx_not_drop_0_reg    );
	idx_not_drop_1.regValue  			= IoReg_Read32(COLOR_SLD_SLD_Idx_not_drop_1_reg    );
	neighbor_not_drop.regValue  		= IoReg_Read32(COLOR_SLD_SLD_NEIGHBOR_NOT_DROP_reg );
	neighbor_drop_0.regValue  			= IoReg_Read32(COLOR_SLD_SLD_NEIGHBOR_DROP_0_reg   );
	neighbor_drop_1.regValue  			= IoReg_Read32(COLOR_SLD_SLD_NEIGHBOR_DROP_1_reg   );
	protect_by_neighbor.regValue  			= IoReg_Read32(COLOR_SLD_SLD_Idx_Protect_By_Neighbor_0_reg   );
	protect_by_neighbor_1.regValue  			= IoReg_Read32(COLOR_SLD_SLD_Idx_Protect_By_Neighbor_1_reg   );
	        
	sld_drop.sld_cnt_thl	    = ptr->sld_cnt_thl;
	sld_drop.sld_hpf_diff_thl	= ptr->sld_hpf_diff_thl;
	sld_drop.sld_pixel_diff_thl	= ptr->sld_pixel_diff_thl;
	
	idx_not_drop_0.in_logo_idx_thl = ptr_in_logo->in_logo_idx_thl;
	
	idx_not_drop_1.hpf_diff_thl_for_logo_region   = ptr_in_logo->hpf_diff_thl_for_logo_region;
	idx_not_drop_1.pixel_diff_thl_for_logo_region = ptr_in_logo->pixel_diff_thl_for_logo_region;
	
	neighbor_not_drop.neighbor_diffy_notdrop_thl = ptr_n_notDrop->neighbor_diffy_notdrop_thl;
	
	neighbor_drop_0.neighbor0_diffy_drop_thl = ptr_nDrop->neighbor0_diffy_drop_thl;
	neighbor_drop_0.neighbor1_diffy_drop_thl = ptr_nDrop->neighbor1_diffy_drop_thl;
	neighbor_drop_1.neighbor2_diffy_drop_thl = ptr_nDrop->neighbor2_diffy_drop_thl;
	
	protect_by_neighbor.idx_drop_protect_counter_neighbor_sameidx = ptr_dropProtect->idx_drop_protect_counter_neighbor_sameidx;
	protect_by_neighbor_1.idx_drop_protect_idx_thl = ptr_dropProtect->idx_drop_protect_idx_thl;
	protect_by_neighbor_1.idx_drop_protect_idx_neighbor_tolerance = ptr_dropProtect->idx_drop_protect_idx_neighbor_tolerance;
	
	

	IoReg_Write32(COLOR_SLD_SLD_DROP_reg 			   						, sld_drop.regValue  			 			);	
	IoReg_Write32(COLOR_SLD_SLD_Idx_not_drop_0_reg    						, idx_not_drop_0.regValue  	 			    );	
	IoReg_Write32(COLOR_SLD_SLD_Idx_not_drop_1_reg    						, idx_not_drop_1.regValue  	 			    );	
	IoReg_Write32(COLOR_SLD_SLD_NEIGHBOR_NOT_DROP_reg 						, neighbor_not_drop.regValue   			    );	
	IoReg_Write32(COLOR_SLD_SLD_NEIGHBOR_DROP_0_reg   						, neighbor_drop_0.regValue  	 			);	
	IoReg_Write32(COLOR_SLD_SLD_NEIGHBOR_DROP_1_reg   						, neighbor_drop_1.regValue  	 			);	
	IoReg_Write32(COLOR_SLD_SLD_Idx_Protect_By_Neighbor_0_reg   			, protect_by_neighbor.regValue  	 			);	
	IoReg_Write32(COLOR_SLD_SLD_Idx_Protect_By_Neighbor_1_reg   			, protect_by_neighbor_1.regValue  	 			);	

}


void drvif_color_set_SLD_compensate_spatial(DRV_SLD_sld_idx_compensate_by_neighbor_0 *ptr)
{

	color_sld_sld_idx_compensate_by_neighbor_0_RBUS	 compensate_by_neighbor_0;
	color_sld_sld_idx_compensate_by_neighbor_1_RBUS compensate_by_neighbor_1;
	
	compensate_by_neighbor_0.regValue  					= IoReg_Read32(COLOR_SLD_SLD_Idx_Compensate_By_Neighbor_0_reg );
	compensate_by_neighbor_1.regValue  					= IoReg_Read32(COLOR_SLD_SLD_Idx_Compensate_By_Neighbor_1_reg );
	        
	compensate_by_neighbor_1.idx_compensate_start_thl	            = ptr->idx_compensate_start_thl;
	compensate_by_neighbor_1.compensate_neighbor_tolerance	        = ptr->compensate_neighbor_tolerance;
	compensate_by_neighbor_0.idx_compensate_neighbor_counter_thl	= ptr->idx_compensate_neighbor_counter_thl;
	
	

	IoReg_Write32(COLOR_SLD_SLD_Idx_Compensate_By_Neighbor_0_reg 			   				, compensate_by_neighbor_0.regValue);	
	IoReg_Write32(COLOR_SLD_SLD_Idx_Compensate_By_Neighbor_1_reg    						, compensate_by_neighbor_1.regValue);	

}

void drvif_color_set_SLD_compensate_temporal(DRV_SLD_sld_temporal_idx_compensate *ptr)
{

	color_sld_sld_temporal_idx_compensate_RBUS	 temporal_idx_compensate;
	
	temporal_idx_compensate.regValue  					= IoReg_Read32(COLOR_SLD_SLD_Temporal_Idx_Compensate_reg 			   );
	        
	temporal_idx_compensate.idx_period_check_once	            = ptr->idx_period_check_once;
	temporal_idx_compensate.do_connect_seg_idx_lower_bound	        = ptr->do_connect_seg_idx_lower_bound;
	
	

	IoReg_Write32(COLOR_SLD_SLD_Temporal_Idx_Compensate_reg 			   				, temporal_idx_compensate.regValue);	

}

void drvif_color_set_SLD_control_minus_step(DRV_SLD_sld_drop_slow_0 *ptr)
{

	color_sld_sld_drop_slow_0_RBUS	 sld_drop_slow_0;
	color_sld_sld_drop_slow_1_RBUS	 sld_drop_slow_1;
	
	sld_drop_slow_0.regValue  					= IoReg_Read32(COLOR_SLD_SLD_Drop_Slow_0_reg 			   );
	sld_drop_slow_1.regValue  					= IoReg_Read32(COLOR_SLD_SLD_Drop_Slow_1_reg 			   );
	        
	sld_drop_slow_0.idx_slow_down_l	            = ptr->idx_slow_down_l;
	sld_drop_slow_0.idx_slow_down_h	            = ptr->idx_slow_down_h;
	sld_drop_slow_1.idx_slow_down_step	        = ptr->idx_slow_down_step;
	
	

	IoReg_Write32(COLOR_SLD_SLD_Drop_Slow_0_reg 			   				, sld_drop_slow_0.regValue);	
	IoReg_Write32(COLOR_SLD_SLD_Drop_Slow_1_reg 			   				, sld_drop_slow_1.regValue);	

}
void drvif_color_set_SLD_fw_control_step(DRV_SLD_sld_fw_control_0 *ptr)
{

	color_sld_sld_fw_control_0_RBUS	 fw_control_0;
	color_sld_sld_fw_control_1_RBUS	 fw_control_1;
	
	fw_control_0.regValue  					= IoReg_Read32(COLOR_SLD_SLD_FW_control_0_reg 			   );
	fw_control_1.regValue  					= IoReg_Read32(COLOR_SLD_SLD_FW_control_1_reg 			   );
	        
	fw_control_0.fw_idx_ctrl_step	            = ptr->fw_idx_ctrl_step;
	//fw_control_0.fw_idx_freeze_for_full_frame	            = ptr->fw_idx_freeze_for_full_frame;
	fw_control_0.fw_idx_ctrl_en	            = ptr->fw_idx_ctrl_en;
	fw_control_1.fw_pixel_gain	            = ptr->fw_pixel_gain;
	
	

	IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg 			   				, fw_control_0.regValue);	
	IoReg_Write32(COLOR_SLD_SLD_FW_control_1_reg 			   				, fw_control_1.regValue);	

}




void drvif_color_set_SLD_fw_control(DRV_SLD_sld_fw_control_0 *ptr)
{

	color_sld_sld_fw_control_0_RBUS	 fw_control_0;
	color_sld_sld_fw_control_1_RBUS	 fw_control_1;
	
	fw_control_0.regValue  					= IoReg_Read32(COLOR_SLD_SLD_FW_control_0_reg 			   );
	fw_control_1.regValue  					= IoReg_Read32(COLOR_SLD_SLD_FW_control_1_reg 			   );
	        
	fw_control_0.fw_idx_ctrl_step	            = ptr->fw_idx_ctrl_step;
	fw_control_0.fw_idx_freeze_for_full_frame	            = ptr->fw_idx_freeze_for_full_frame;
	fw_control_0.fw_idx_ctrl_en	            = ptr->fw_idx_ctrl_en;
	fw_control_1.fw_pixel_gain	            = ptr->fw_pixel_gain;
	
	

	IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg 			   				, fw_control_0.regValue);	
	IoReg_Write32(COLOR_SLD_SLD_FW_control_1_reg 			   				, fw_control_1.regValue);	

}

void drvif_color_set_SLD_sld_temporal_noise(DRV_SLD_sld_temporal_noise_1 *ptr)
{

	color_sld_sld_temporal_noise_0_RBUS	 temporal_noise_0;
	color_sld_sld_temporal_noise_1_RBUS	 temporal_noise_1;
	color_sld_sld_temporal_noise_3_RBUS	 temporal_noise_3;
	
	temporal_noise_0.regValue  					= IoReg_Read32(COLOR_SLD_SLD_Temporal_Noise_0_reg 			   );
	temporal_noise_1.regValue  					= IoReg_Read32(COLOR_SLD_SLD_Temporal_Noise_1_reg 			   );
	temporal_noise_3.regValue  					= IoReg_Read32(COLOR_SLD_SLD_Temporal_Noise_3_reg 			   );
	        
	temporal_noise_0.temporal_diff	            = ptr->temporal_diff;
	temporal_noise_1.big_temporal_diff_low	            = ptr->big_temporal_diff_low;
	temporal_noise_1.big_temporal_diff_up	            = ptr->big_temporal_diff_up;
	temporal_noise_3.temporal_counter_thl	            = ptr->temporal_counter_thl;
	
	

	IoReg_Write32(COLOR_SLD_SLD_Temporal_Noise_0_reg 			   				, temporal_noise_0.regValue);	
	IoReg_Write32(COLOR_SLD_SLD_Temporal_Noise_1_reg 			   				, temporal_noise_1.regValue);	
	IoReg_Write32(COLOR_SLD_SLD_Temporal_Noise_3_reg 			   				, temporal_noise_3.regValue);	

}

void drvif_color_set_SLD_sld_window(DRV_SLD_sld_apply_window_0 *ptr)
{

	color_sld_sld_control_RBUS color_sld_control;

	color_sld_sld_apply_window_0_RBUS	 apply_window_0;
	color_sld_sld_apply_window_1_RBUS	 apply_window_1;

	color_sld_control.regValue = IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg);
	apply_window_0.regValue  					= IoReg_Read32(COLOR_SLD_SLD_Apply_Window_0_reg 			   );
	apply_window_1.regValue  					= IoReg_Read32(COLOR_SLD_SLD_Apply_Window_1_reg 			   );

	//color_sld_control.sld_window_en				= 0;
	apply_window_0.sld_window_apply_out	            = ptr->sld_window_apply_out;
	apply_window_0.sld_window_pos0_y	            = ptr->sld_window_pos0_y;
	apply_window_0.sld_window_pos0_x	            = ptr->sld_window_pos0_x;
	apply_window_1.sld_window_pos1_y	            = ptr->sld_window_pos1_y;
	apply_window_1.sld_window_pos1_x	            = ptr->sld_window_pos1_x;
	

	IoReg_Write32(COLOR_SLD_SLD_Apply_Window_0_reg 			   				, apply_window_0.regValue);	
	IoReg_Write32(COLOR_SLD_SLD_Apply_Window_1_reg 			   				, apply_window_1.regValue);	
	IoReg_Write32(COLOR_SLD_SLD_CONTROL_reg, color_sld_control.regValue);




}
void drvif_color_set_SLD_sld_histogram(DRV_SLD_sld_histogram_1 *ptr, DRV_SLD_sld_histogram_0 *ptr_0)
{

	color_sld_sld_histogram_0_RBUS	 histogram_0;
	color_sld_sld_histogram_1_RBUS	 histogram_1;
	color_sld_sld_histogram_2_RBUS	 histogram_2;
	color_sld_sld_histogram_3_RBUS	 histogram_3;
	color_sld_sld_histogram_4_RBUS	 histogram_4;
	
	histogram_0.regValue  					= IoReg_Read32(COLOR_SLD_SLD_histogram_0_reg 			   );
	histogram_1.regValue  					= IoReg_Read32(COLOR_SLD_SLD_histogram_1_reg 			   );
	histogram_2.regValue  					= IoReg_Read32(COLOR_SLD_SLD_histogram_2_reg 			   );
	histogram_3.regValue  					= IoReg_Read32(COLOR_SLD_SLD_histogram_3_reg 			   );
	histogram_4.regValue  					= IoReg_Read32(COLOR_SLD_SLD_histogram_4_reg 			   );
	        
	histogram_0.hist_idx_thl	            = ptr_0->hist_idx_thl;
	histogram_0.hist_shift_bit	            = ptr_0->hist_shift_bit;
	histogram_1.y_lower_bound_hist_0	            = ptr->y_lower_bound_hist_0;
	histogram_1.y_lower_bound_hist_1	            = ptr->y_lower_bound_hist_1;
	histogram_2.y_lower_bound_hist_2	            = ptr->y_lower_bound_hist_2;
	histogram_2.y_lower_bound_hist_3	            = ptr->y_lower_bound_hist_3;
	histogram_3.y_lower_bound_hist_4	            = ptr->y_lower_bound_hist_4;
	histogram_3.y_lower_bound_hist_5	            = ptr->y_lower_bound_hist_5;
	histogram_4.y_lower_bound_hist_6	            = ptr->y_lower_bound_hist_6;
	histogram_4.y_lower_bound_hist_7	            = ptr->y_lower_bound_hist_7;

	IoReg_Write32(COLOR_SLD_SLD_histogram_0_reg 			   				, histogram_0.regValue);	
	IoReg_Write32(COLOR_SLD_SLD_histogram_1_reg 			   				, histogram_1.regValue);	
	IoReg_Write32(COLOR_SLD_SLD_histogram_2_reg 			   				, histogram_2.regValue);	
	IoReg_Write32(COLOR_SLD_SLD_histogram_3_reg 			   				, histogram_3.regValue);	
	IoReg_Write32(COLOR_SLD_SLD_histogram_4_reg 			   				, histogram_4.regValue);	

}

void drvif_color_set_SLD_LGD_Tnoise_pattern(DRV_SLD_sld_noise_pattern_0 *ptr)
{
	color_sld_sld_noise_pattern_1_RBUS	 noise_pattern_1;
	color_sld_sld_noise_pattern_2_RBUS   noise_pattern_2;
	color_sld_sld_noise_pattern_3_RBUS	 noise_pattern_3;
	color_sld_sld_noise_pattern_0_RBUS   noise_pattern_0;
	
	noise_pattern_0.regValue  					= IoReg_Read32(COLOR_SLD_SLD_noise_pattern_0_reg 			   );
	noise_pattern_1.regValue  					= IoReg_Read32(COLOR_SLD_SLD_noise_pattern_1_reg 			   );
	noise_pattern_2.regValue  					= IoReg_Read32(COLOR_SLD_SLD_noise_pattern_2_reg 			   );
	noise_pattern_3.regValue  					= IoReg_Read32(COLOR_SLD_SLD_noise_pattern_3_reg 			   );

	noise_pattern_0.detect_tnoise_en		      = ptr->detect_tnoise_en;
	noise_pattern_0.grouph_diffmaxmin		      = ptr->grouph_diffmaxmin;
	noise_pattern_0.diffavg		      			= ptr->diffavg;	
	noise_pattern_3.check_drop_pattern_start	            = ptr->check_drop_pattern_start;
	noise_pattern_1.patternedge_y	            = ptr->patternedge_y;
	noise_pattern_1.patternedge_hpf	            = ptr->patternedge_hpf;
	noise_pattern_2.patternregion_hpf	            = ptr->patternregion_hpf;
	noise_pattern_2.patternregion_y	            = ptr->patternregion_y;
	

	IoReg_Write32(COLOR_SLD_SLD_noise_pattern_0_reg 			   				, noise_pattern_0.regValue);	
	IoReg_Write32(COLOR_SLD_SLD_noise_pattern_1_reg 			   				, noise_pattern_1.regValue);	
	IoReg_Write32(COLOR_SLD_SLD_noise_pattern_2_reg 			   				, noise_pattern_2.regValue);	
	IoReg_Write32(COLOR_SLD_SLD_noise_pattern_3_reg 			   				, noise_pattern_3.regValue);	

}


void drvif_color_set_SLD_max_pre_logo_tolerance(DRV_SLD_sld_fw_control_2 *ptr)
{

	color_sld_sld_fw_control_2_RBUS	 fw_control_2;
	
	fw_control_2.regValue  					= IoReg_Read32(COLOR_SLD_SLD_FW_control_2_reg 			   );
	        
	fw_control_2.max_pre_logo_tolerance	            = ptr->max_pre_logo_tolerance;
	
	IoReg_Write32(COLOR_SLD_SLD_FW_control_2_reg , fw_control_2.regValue);	

}



void  drvif_solor_set_SLD_apply_leave_RGB_box_logo(unsigned int cmd_rgb_box_leave_idx_delete_step)
{
	unsigned int sld_max_pre_idx = 0;
	//DRV_SLD_sld_apply_window_0 sld_apply_window_data;
	DRV_SLD_sld_control sld_control_data;
	DRV_SLD_sld_fw_control_0 fw_control_0_data;
	//DRV_SLD_sld_Y_curve_map Y_curve_map_data;

 	drvif_color_get_SLD_max_pre_idx(&sld_max_pre_idx);


		//apply demo window
		/*
		sld_apply_window_data.sld_window_apply_out = 0;
		sld_apply_window_data.sld_window_pos0_x = 1775;
		sld_apply_window_data.sld_window_pos0_y = 75;
		sld_apply_window_data.sld_window_pos1_x = 1848;
		sld_apply_window_data.sld_window_pos1_y = 148;
		*/
		
		sld_control_data.sld_window_en = 0;
		
		//fw control step
		fw_control_0_data.fw_idx_ctrl_en   = 1;
		fw_control_0_data.fw_idx_ctrl_step = 8192-cmd_rgb_box_leave_idx_delete_step;//step : -128
		fw_control_0_data.fw_idx_freeze_for_full_frame = 0;//step : -128
		fw_control_0_data.fw_pixel_gain = 4096;

		rtd_pr_vpq_info("sld_max_pre_idx = %d################\n",sld_max_pre_idx);

		//fw control gain	
		
		//fw_control_0_data.fw_pixel_gain=8192<<3;//debug use
		//fw_control_0_data.fw_pixel_gain=CLAMP_SLD(((8192)* cmd_LC_period * (counter_box_logo_case - cmd_thl_counter_box_logo_case))/(13*60),8192,0) ;

		
/*
		//R,G,B box Y curve
		Y_curve_map_data.sld_curve_seg_0 = 120;
		Y_curve_map_data.sld_curve_seg_1 = 160;
		Y_curve_map_data.sld_curve_seg_2 = 800;
		Y_curve_map_data.sld_new_curve_gain_0 = 0;
		Y_curve_map_data.sld_new_curve_gain_1 = 0;
		Y_curve_map_data.sld_new_curve_gain_2 = 80;
		Y_curve_map_data.sld_new_curve_gain_3 = 64;
		Y_curve_map_data.sld_new_curve_offset_0 = 0;
		Y_curve_map_data.sld_new_curve_offset_1 = 0;
		Y_curve_map_data.sld_new_curve_offset_2 = 65535;
		Y_curve_map_data.sld_new_curve_offset_3 =  0;
*/

		//R,G,B box Y curve
		/*
		Y_curve_map_data.sld_curve_seg_0 = 256;
		Y_curve_map_data.sld_curve_seg_1 = 512;
		Y_curve_map_data.sld_curve_seg_2 = 768;
		Y_curve_map_data.sld_new_curve_gain_0 = 32;
		Y_curve_map_data.sld_new_curve_gain_1 = 32;
		Y_curve_map_data.sld_new_curve_gain_2 = 32;
		Y_curve_map_data.sld_new_curve_gain_3 = 32;
		Y_curve_map_data.sld_new_curve_offset_0 = 0;
		Y_curve_map_data.sld_new_curve_offset_1 = 0;
		Y_curve_map_data.sld_new_curve_offset_2 = 0;
		Y_curve_map_data.sld_new_curve_offset_3 =  0;
		*/

		
	    //drvif_color_set_SLD_sld_window(&sld_apply_window_data);
	        drvif_color_set_SLD_window_en(&sld_control_data);
	        drvif_color_set_SLD_fw_control_step(&fw_control_0_data);
 		//drvif_color_set_SLD_Y_curve_map(&Y_curve_map_data);
		

}

void  drvif_color_SLD_LPF_ur(void)
{
/*
	IoReg_Write32(0xb802c770,0x00060001);
	IoReg_Write32(0xb802c774,0x00040006);
	IoReg_Write32(0xb802c778,0x00040001);
	IoReg_Write32(0xb802c77c,0x00180018);
	IoReg_Write32(0xb802c780,0x00040010);
	IoReg_Write32(0xb802c784,0x00240006);
	IoReg_Write32(0xb802c788,0x00180024);
	IoReg_Write32(0xb802c78c,0x00060006);
	IoReg_Write32(0xb802c790,0x00240024);
	IoReg_Write32(0xb802c794,0x00060018);
	IoReg_Write32(0xb802c798,0x00240006);
	IoReg_Write32(0xb802c79c,0x00180024);
	IoReg_Write32(0xb802c7a0,0x00000006);
	IoReg_Write32(0xb802c7a4,0x00000009);
*/
	DRV_SLD_sld_lpf_0 sld_lpf_0_data;

	sld_lpf_0_data.coef_0_0=1;
	sld_lpf_0_data.coef_0_1=6;
	sld_lpf_0_data.coef_0_2=6;
	sld_lpf_0_data.coef_0_3=6;
	sld_lpf_0_data.coef_0_4=1;
	
	sld_lpf_0_data.coef_1_0=6;
	sld_lpf_0_data.coef_1_1=36;
	sld_lpf_0_data.coef_1_2=36;
	sld_lpf_0_data.coef_1_3=36;
	sld_lpf_0_data.coef_1_4=6;
	
	sld_lpf_0_data.coef_2_0=6;
	sld_lpf_0_data.coef_2_1=36;
	sld_lpf_0_data.coef_2_2=36;
	sld_lpf_0_data.coef_2_3=24;
	sld_lpf_0_data.coef_2_4=6;
	
	sld_lpf_0_data.coef_3_0=6;
	sld_lpf_0_data.coef_3_1=36;
	sld_lpf_0_data.coef_3_2=36;
	sld_lpf_0_data.coef_3_3=24;
	sld_lpf_0_data.coef_3_4=6;
	
	sld_lpf_0_data.coef_4_0=6;
	sld_lpf_0_data.coef_4_1=24;
	sld_lpf_0_data.coef_4_2=36;
	sld_lpf_0_data.coef_4_3=24;
	sld_lpf_0_data.coef_4_4=6;
	
	sld_lpf_0_data.lpf_shift_bit=9;
 	drvif_color_set_SLD_lpf((DRV_SLD_sld_lpf_0*) &sld_lpf_0_data);	
}


//get///////////////////////////////
void drvif_color_get_SLD_max_pre_idx(unsigned int* sld_max_pre_idx)
{

	color_sld_sld_cnt_2_RBUS	 sld_cnt_2;
	
	sld_cnt_2.regValue  					    = IoReg_Read32(COLOR_SLD_SLD_CNT_2_reg 			   );
	        
	*sld_max_pre_idx        				=sld_cnt_2.sld_max_pre_idx;

}



void drvif_color_get_SLD_fw_read_only(DRV_SLD_sld_fw_read_only *ptr)
{

	color_sld_sld_fw_read_only_0_RBUS	 fw_read_only_0;
	color_sld_sld_fw_read_only_1_RBUS	 fw_read_only_1;
	color_sld_sld_fw_read_only_2_RBUS	 fw_read_only_2;
	color_sld_sld_fw_read_only_3_RBUS	 fw_read_only_3;
	color_sld_sld_cnt_0_RBUS	 sld_cnt_0;
	color_sld_sld_cnt_1_RBUS	 sld_cnt_1;
	color_sld_sld_cnt_2_RBUS	 sld_cnt_2;
	
	fw_read_only_0.regValue  					= IoReg_Read32(COLOR_SLD_SLD_FW_read_only_0_reg 			   );
	fw_read_only_1.regValue  					= IoReg_Read32(COLOR_SLD_SLD_FW_read_only_1_reg 			   );
	fw_read_only_2.regValue  					= IoReg_Read32(COLOR_SLD_SLD_FW_read_only_2_reg 			   );
	fw_read_only_3.regValue  					= IoReg_Read32(COLOR_SLD_SLD_FW_read_only_3_reg 			   );
	sld_cnt_0.regValue  					    = IoReg_Read32(COLOR_SLD_SLD_CNT_0_reg 			   );
	sld_cnt_1.regValue  					    = IoReg_Read32(COLOR_SLD_SLD_CNT_1_reg 			   );
	sld_cnt_2.regValue  					    = IoReg_Read32(COLOR_SLD_SLD_CNT_2_reg 			   );
	        
	ptr->max_pre_logo_idx_counter             =fw_read_only_0.max_pre_logo_idx_counter	;
	ptr->near_max_pre_logo_idx_counter        =fw_read_only_1.near_max_pre_logo_idx_counter;
	ptr->hpf_line_count        				  =fw_read_only_3.hpf_line_count;
	ptr->blocknum_with_alpha_noise        	  =fw_read_only_2.blocknum_with_alpha_noise;
	ptr->sld_logo_cnt        					=sld_cnt_0.sld_logo_cnt;
	ptr->sld_drop_cnt        					=sld_cnt_1.sld_drop_cnt;
	ptr->sld_max_pre_idx        				=sld_cnt_2.sld_max_pre_idx;
	
	
	


}

void drvif_SLD_table_set(unsigned char tbl_sel, DRV_SLD_local_gain_tbl *ptr_tbl){
	int i=0;
	color_sld_sld_lut_ctrl_RBUS color_sld_sld_lut_ctrl_reg;
	color_sld_sld_lut_data_RBUS color_sld_sld_lut_data_reg;

	color_sld_sld_lut_ctrl_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_LUT_CTRL_reg);
	color_sld_sld_lut_data_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_LUT_DATA_reg);

	/*0:local gain table, 1:outY gain table*/
	if(tbl_sel==0){
		color_sld_sld_lut_ctrl_reg.sld_lut_sel = tbl_sel;
		color_sld_sld_lut_ctrl_reg.sld_32x18_lut_en = 1;
		IoReg_Write32(COLOR_SLD_SLD_LUT_CTRL_reg, color_sld_sld_lut_ctrl_reg.regValue);

		for(i=0;i<(32*18);i++){
			color_sld_sld_lut_data_reg.sld_lut_data= ptr_tbl->SLD_local_gain_table[i];
			IoReg_Write32(COLOR_SLD_SLD_LUT_DATA_reg, color_sld_sld_lut_data_reg.regValue);
		}

		color_sld_sld_lut_ctrl_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_LUT_CTRL_reg);
		color_sld_sld_lut_ctrl_reg.sld_32x18_lut_en = 0;
		IoReg_Write32(COLOR_SLD_SLD_LUT_CTRL_reg, color_sld_sld_lut_ctrl_reg.regValue);
		
	}else if(tbl_sel==1){
		color_sld_sld_lut_ctrl_reg.sld_lut_sel = tbl_sel;
		color_sld_sld_lut_ctrl_reg.sld_32x18_lut_en = 1;
		IoReg_Write32(COLOR_SLD_SLD_LUT_CTRL_reg, color_sld_sld_lut_ctrl_reg.regValue);
		for(i=0;i<(32*18);i++){
			color_sld_sld_lut_data_reg.sld_lut_data= ptr_tbl->SLD_local_gain_table[i];
			IoReg_Write32(COLOR_SLD_SLD_LUT_DATA_reg, color_sld_sld_lut_data_reg.regValue);
		}		

		color_sld_sld_lut_ctrl_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_LUT_CTRL_reg);
		color_sld_sld_lut_ctrl_reg.sld_32x18_lut_en = 0;
		IoReg_Write32(COLOR_SLD_SLD_LUT_CTRL_reg, color_sld_sld_lut_ctrl_reg.regValue);		
	}else{
		rtd_pr_vpq_emerg("SLD table sel failed\n");
		return;
	}
	
}

void drvif_SLD_table_get(unsigned char tbl_sel, unsigned short *ptr_tbl){
	int i=0;
	color_sld_sld_lut_ctrl_RBUS color_sld_sld_lut_ctrl_reg;
	color_sld_sld_lut_data_RBUS color_sld_sld_lut_data_reg;

	color_sld_sld_lut_ctrl_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_LUT_CTRL_reg);
	color_sld_sld_lut_data_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_LUT_DATA_reg);

	/*0:local gain table, 1:outY gain table*/
	if(tbl_sel==0){
		color_sld_sld_lut_ctrl_reg.sld_lut_sel = tbl_sel;
		color_sld_sld_lut_ctrl_reg.sld_32x18_lut_en = 1;
		color_sld_sld_lut_ctrl_reg.sld_32x18_lut_read_en = 1;
		IoReg_Write32(COLOR_SLD_SLD_LUT_CTRL_reg, color_sld_sld_lut_ctrl_reg.regValue);

		for(i=0;i<(32*18);i++){
			color_sld_sld_lut_data_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_LUT_DATA_reg);	
			ptr_tbl[i] = color_sld_sld_lut_data_reg.sld_lut_data;
			
		}

		color_sld_sld_lut_ctrl_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_LUT_CTRL_reg);
		color_sld_sld_lut_ctrl_reg.sld_32x18_lut_en = 0;
		IoReg_Write32(COLOR_SLD_SLD_LUT_CTRL_reg, color_sld_sld_lut_ctrl_reg.regValue);
		
	}else if(tbl_sel==1){
		color_sld_sld_lut_ctrl_reg.sld_lut_sel = tbl_sel;
		color_sld_sld_lut_ctrl_reg.sld_32x18_lut_en = 1;
		color_sld_sld_lut_ctrl_reg.sld_32x18_lut_read_en = 1;
		IoReg_Write32(COLOR_SLD_SLD_LUT_CTRL_reg, color_sld_sld_lut_ctrl_reg.regValue);
		for(i=0;i<(32*18);i++){
			color_sld_sld_lut_data_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_LUT_DATA_reg);	
			ptr_tbl[i] = color_sld_sld_lut_data_reg.sld_lut_data;
		}	

		color_sld_sld_lut_ctrl_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_LUT_CTRL_reg);
		color_sld_sld_lut_ctrl_reg.sld_32x18_lut_en = 0;
		IoReg_Write32(COLOR_SLD_SLD_LUT_CTRL_reg, color_sld_sld_lut_ctrl_reg.regValue);		
	}else{
		rtd_pr_vpq_emerg("SLD table sel failed\n");
		return;
	}
	
}

void drvif_SLD_table_enable(unsigned char tbl_sel, unsigned char enable){
	color_sld_sld_control_RBUS color_sld_sld_control_reg;
	color_sld_sld_control_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg);

	if(tbl_sel==0)
		color_sld_sld_control_reg.local_gain_table_en = enable;
	else if(tbl_sel==1)
		color_sld_sld_control_reg.outy_gain_table_en= enable;
		
	IoReg_Write32(COLOR_SLD_SLD_CONTROL_reg, color_sld_sld_control_reg.regValue);
}

uint32_t reverse_uint32(uint32_t n){

    n = ((0xffff0000 & n) >> 16) | ((0x0000ffff & n) << 16);
    n = ((0xff00ff00 & n) >> 8) | ((0x00ff00ff & n) << 8);
    n = ((0xf0f0f0f0 & n) >> 4) | ((0x0f0f0f0f & n) << 4);
    n = ((0xcccccccc & n) >> 2) | ((0x33333333 & n) << 2);
    n = ((0xaaaaaaaa & n) >> 1) | ((0x55555555 & n) << 1);
    return n;
}

uint16_t reverse_uint16(uint16_t n) {
    uint16_t x = (uint16_t)(reverse_uint32(n) >> 16);
    return x;
}

char drvif_SLD_Block_APL_Read_by_DMA_ISR(void/*unsigned short *pout_tbl*/)
{
	#define SLD_APL_Blk_Size 32
	#define SLD_APL_num_per_32b 3
	color_sld_sld_wdma_db_en_RBUS color_sld_sld_wdma_db_en_reg;
	color_sld_sld_wdma_wr_ctrl_RBUS color_sld_sld_wdma_wr_ctrl_reg;
	color_sld_sld_size_RBUS color_sld_sld_size_reg;
	int sld_width,sld_height;
	VIP_SLD_DMA_CTRL *SLD_DMA_CTRL = NULL;
	unsigned int i, read_cnt, read_3_cnt, idx_byte, last_line_pixel;
	unsigned short read_w, read_h, last_blk_w, last_blk_h;
	unsigned char *pVa;
	unsigned int *pVa_tmp_uint32;
	unsigned short APL_tmp[3]={0};
	color_sld_sld_wdma_db_en_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_WDMA_DB_EN_reg);
	if (color_sld_sld_wdma_db_en_reg.cap_db_apply == 1) {
		//rtd_pr_vpq_emerg("SLD_HW, APL cap_db_apply=1\n");
		return 1;
	}

	color_sld_sld_wdma_wr_ctrl_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_WDMA_WR_Ctrl_reg);	
	if(color_sld_sld_wdma_wr_ctrl_reg.cap_block_sel == 0) {
		SLD_DMA_CTRL = drvif_fwif_color_Get_SLD_APL_DMA_CTRL(1);
		color_sld_sld_wdma_wr_ctrl_reg.cap_block_sel = 1;
		
	} else {
		SLD_DMA_CTRL = drvif_fwif_color_Get_SLD_APL_DMA_CTRL(0);
		color_sld_sld_wdma_wr_ctrl_reg.cap_block_sel = 0;		
	}

	if (/*pout_tbl == NULL || */SLD_DMA_CTRL->pVir_addr_align == NULL) {
		rtd_pr_vpq_emerg("SLD_HW, TBL NULL\n");
		return -1;
	}
	color_sld_sld_size_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_SIZE_reg);
	sld_width = color_sld_sld_size_reg.sld_width;
	sld_height = color_sld_sld_size_reg.sld_height;
	read_w = (sld_width + (SLD_APL_Blk_Size-1))>>5;
	read_h = (sld_height + (SLD_APL_Blk_Size-1))>>5;
	last_blk_w = sld_width - SLD_APL_Blk_Size * (read_w - 1);
	last_blk_h = sld_height - SLD_APL_Blk_Size * (read_h - 1);
	read_cnt = read_w * read_h;
	read_3_cnt = (read_cnt + (SLD_APL_num_per_32b-1))/SLD_APL_num_per_32b;
	last_line_pixel = last_blk_w * last_blk_h;

	pVa_tmp_uint32 =  SLD_DMA_CTRL->pVir_addr_align;
	for(i=0;i<read_3_cnt;i++){
		pVa_tmp_uint32[i]=reverse_uint32(pVa_tmp_uint32[i]);
	}
	pVa = (unsigned char*)(pVa_tmp_uint32);

	idx_byte = 0;
	for (i=0;i<read_3_cnt;i++) {
		APL_tmp[0] = reverse_uint16(((((unsigned short)pVa[idx_byte])&0x3f)<<4) + ((((unsigned short)pVa[idx_byte+1])&0xf0)>>4))>>6;
		APL_tmp[1] = reverse_uint16(((((unsigned short)pVa[idx_byte+1])&0x0f)<<6) + ((((unsigned short)pVa[idx_byte+2])&0xfc)>>2))>>6;
		APL_tmp[2] =	reverse_uint16(((((unsigned short)pVa[idx_byte+2])&0x03)<<8) + (((unsigned short)pVa[idx_byte+3])&0xff))>>6;
		//if(i==0)
		//	rtd_pr_vpq_emerg("SLD_HW, APL APL_tmp[0]:%x, APL_tmp[1]:%x, APL_tmp[2]:%x\n",APL_tmp[0] ,APL_tmp[1],APL_tmp[2]  );
		
		SLD_BLK_APL[i*SLD_APL_num_per_32b+0] = APL_tmp[0] ;
		SLD_BLK_APL[i*SLD_APL_num_per_32b+1] = APL_tmp[1] ;
		SLD_BLK_APL[i*SLD_APL_num_per_32b+2] = APL_tmp[2] ;
		//SLD_BLK_APL[i*SLD_APL_num_per_32b+0] = ((((unsigned short)pVa[idx_byte])&0x3f)<<4) + ((((unsigned short)pVa[idx_byte+1])&0xf0)>>4);
		//SLD_BLK_APL[i*SLD_APL_num_per_32b+1] = ((((unsigned short)pVa[idx_byte+1])&0x0f)<<6) + ((((unsigned short)pVa[idx_byte+2])&0xfc)>>2);
		//SLD_BLK_APL[i*SLD_APL_num_per_32b+2] = ((((unsigned short)pVa[idx_byte+2])&0x03)<<8) + (((unsigned short)pVa[idx_byte+3])&0xff);
		if (((i * SLD_APL_num_per_32b)  >= (read_w * (read_h - 1))) && (last_line_pixel != 0)) {	// last h line
			SLD_BLK_APL[i*SLD_APL_num_per_32b+0] = SLD_BLK_APL[i*SLD_APL_num_per_32b+0] * SLD_APL_Blk_Size * SLD_APL_Blk_Size / last_line_pixel;
			SLD_BLK_APL[i*SLD_APL_num_per_32b+1] = SLD_BLK_APL[i*SLD_APL_num_per_32b+1] * SLD_APL_Blk_Size * SLD_APL_Blk_Size / last_line_pixel;
			SLD_BLK_APL[i*SLD_APL_num_per_32b+2] = SLD_BLK_APL[i*SLD_APL_num_per_32b+2] * SLD_APL_Blk_Size * SLD_APL_Blk_Size / last_line_pixel;

		}
		idx_byte = idx_byte + 4;
	}

	IoReg_Write32(COLOR_SLD_SLD_WDMA_WR_Ctrl_reg,color_sld_sld_wdma_wr_ctrl_reg.regValue);

	color_sld_sld_wdma_db_en_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_WDMA_DB_EN_reg);
	color_sld_sld_wdma_db_en_reg.cap_db_apply = 1;
	IoReg_Write32(COLOR_SLD_SLD_WDMA_DB_EN_reg,color_sld_sld_wdma_db_en_reg.regValue);
	return 0;
}

void drvif_SLD_Block_APL_init(unsigned int p_addr0, unsigned int p_addr1)
{
	color_sld_sld_wdma_db_en_RBUS color_sld_sld_wdma_db_en_reg;
	color_sld_sld_wdma_status_RBUS color_sld_sld_wdma_status_reg;
	color_sld_sld_wdma_wr_ctrl_RBUS color_sld_sld_wdma_wr_ctrl_reg;
	color_sld_sld_dma_wr_num_bl_wrap_addr_0_RBUS color_sld_sld_dma_wr_num_bl_wrap_addr_0_reg;
	color_sld_sld_dma_wr_num_bl_wrap_addr_1_RBUS color_sld_sld_dma_wr_num_bl_wrap_addr_1_reg;
	color_sld_sld_dma_wr_num_bl_wrap_ctl_RBUS color_sld_sld_dma_wr_num_bl_wrap_ctl_reg;
	color_sld_sld_size_RBUS color_sld_sld_size_reg;
	int sld_width,sld_height;

	color_sld_sld_size_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_SIZE_reg);
	sld_width = color_sld_sld_size_reg.sld_width;
	sld_height = color_sld_sld_size_reg.sld_height;
	
	color_sld_sld_wdma_db_en_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_WDMA_DB_EN_reg);
	color_sld_sld_wdma_db_en_reg.cap_db_en = 1;
	IoReg_Write32(COLOR_SLD_SLD_WDMA_DB_EN_reg,color_sld_sld_wdma_db_en_reg.regValue);

	color_sld_sld_dma_wr_num_bl_wrap_addr_0_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_DMA_WR_num_bl_wrap_addr_0_reg);
	color_sld_sld_dma_wr_num_bl_wrap_addr_1_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_DMA_WR_num_bl_wrap_addr_1_reg);
	color_sld_sld_dma_wr_num_bl_wrap_addr_0_reg.cap_sta_addr_0 = p_addr0>>4;
	color_sld_sld_dma_wr_num_bl_wrap_addr_1_reg.cap_sta_addr_1 = p_addr1>>4;
	IoReg_Write32(COLOR_SLD_SLD_DMA_WR_num_bl_wrap_addr_0_reg,color_sld_sld_dma_wr_num_bl_wrap_addr_0_reg.regValue);
	IoReg_Write32(COLOR_SLD_SLD_DMA_WR_num_bl_wrap_addr_1_reg,color_sld_sld_dma_wr_num_bl_wrap_addr_1_reg.regValue);
	
	color_sld_sld_wdma_status_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_WDMA_Status_reg);	
	color_sld_sld_wdma_wr_ctrl_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_WDMA_WR_Ctrl_reg);
	color_sld_sld_dma_wr_num_bl_wrap_ctl_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_DMA_WR_num_bl_wrap_ctl_reg);
	color_sld_sld_dma_wr_num_bl_wrap_ctl_reg.cap_line_128_num= ((sld_width/32)*10*(sld_height/32)+119)/120;
	color_sld_sld_dma_wr_num_bl_wrap_ctl_reg.cap_burst_len = 1;
	color_sld_sld_wdma_wr_ctrl_reg.cap_en = 1;
	color_sld_sld_wdma_wr_ctrl_reg.cap_block_sel = 0;		// start from addr 0

	IoReg_Write32(COLOR_SLD_SLD_WDMA_Status_reg,color_sld_sld_wdma_status_reg.regValue);
	IoReg_Write32(COLOR_SLD_SLD_WDMA_WR_Ctrl_reg,color_sld_sld_wdma_wr_ctrl_reg.regValue);
	IoReg_Write32(COLOR_SLD_SLD_DMA_WR_num_bl_wrap_addr_0_reg,color_sld_sld_dma_wr_num_bl_wrap_addr_0_reg.regValue);
	IoReg_Write32(COLOR_SLD_SLD_DMA_WR_num_bl_wrap_addr_1_reg,color_sld_sld_dma_wr_num_bl_wrap_addr_1_reg.regValue);
	IoReg_Write32(COLOR_SLD_SLD_DMA_WR_num_bl_wrap_ctl_reg,color_sld_sld_dma_wr_num_bl_wrap_ctl_reg.regValue);

	color_sld_sld_wdma_db_en_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_WDMA_DB_EN_reg);
	color_sld_sld_wdma_db_en_reg.cap_db_apply =1;
	IoReg_Write32(COLOR_SLD_SLD_WDMA_DB_EN_reg,color_sld_sld_wdma_db_en_reg.regValue);
}

#if 0

void drvif_color_set_SLD_hpf_in_line_thl(DRV_SLD_sld_hpf_in_line *ptr)
{

	color_sld_sld_hpf_in_line_RBUS	 hpf_in_line;
	
	hpf_in_line.regValue  					= IoReg_Read32(COLOR_SLD_SLD_hpf_in_line_reg 			   );
	        
	hpf_in_line.hpf_in_line_thl	            = ptr->hpf_in_line_thl;
	
	

	IoReg_Write32(COLOR_SLD_SLD_hpf_in_line_reg 			   				, hpf_in_line.regValue);	

}


void drvif_color_get_histogram_bin(DRV_SLD_sld_histogram_ro_0 *ptr)
{

	color_sld_sld_histogram_ro_0_RBUS	 histogram_ro_0;
	color_sld_sld_histogram_ro_1_RBUS	 histogram_ro_1;
	color_sld_sld_histogram_ro_2_RBUS	 histogram_ro_2;
	color_sld_sld_histogram_ro_3_RBUS	 histogram_ro_3;
	
	histogram_ro_0.regValue  					= IoReg_Read32(COLOR_SLD_SLD_histogram_ro_0_reg 			   );
	histogram_ro_1.regValue  					= IoReg_Read32(COLOR_SLD_SLD_histogram_ro_1_reg 			   );
	histogram_ro_2.regValue  					= IoReg_Read32(COLOR_SLD_SLD_histogram_ro_2_reg 			   );
	histogram_ro_3.regValue  					= IoReg_Read32(COLOR_SLD_SLD_histogram_ro_3_reg 			   );
	        
	ptr->hist_0  = histogram_ro_0.hist_0;	
	ptr->hist_1  = histogram_ro_0.hist_1;	
	ptr->hist_2  = histogram_ro_1.hist_2;	
	ptr->hist_3  = histogram_ro_1.hist_3;	
	ptr->hist_4  = histogram_ro_2.hist_4;	
	ptr->hist_5  = histogram_ro_2.hist_5;	
	ptr->hist_6  = histogram_ro_3.hist_6;	
	ptr->hist_7  = histogram_ro_3.hist_7;	

}
#endif


int CLAMP_SLD(int value, int max, int min){

	if(value > max)
		return(max);
	else if(value < min)
		return(min);
	
	return(value);

}


void  fwif_color_SLD_default_window(void)
{
	DRV_SLD_sld_apply_window_0 sld_apply_window_data;
	DRV_SLD_sld_control sld_control_data;

	//apply demo window
	sld_apply_window_data.sld_window_apply_out = 0;
	sld_apply_window_data.sld_window_pos0_x = 0;
	sld_apply_window_data.sld_window_pos0_y = 0;
	sld_apply_window_data.sld_window_pos1_x = 1919;
	sld_apply_window_data.sld_window_pos1_y = 200;

	sld_control_data.sld_window_en = 1;
		
    drvif_color_set_SLD_sld_window(&sld_apply_window_data);
    drvif_color_set_SLD_window_en(&sld_control_data);
		

}

unsigned short LC_detect_NEWS(unsigned int *pAPL_cur)
{
	unsigned int APL_4 = 0, APL_3 = 0, APL_2 = 0, APL_1 = 0;
	int abs1 = 0, abs2 = 0, abs3 = 0;

//get APL
	APL_4 = pAPL_cur[1293];
	APL_3 = pAPL_cur[1292];
	APL_2 = pAPL_cur[1291];
	APL_1 = pAPL_cur[1290];

	//rtd_pr_vpq_info("abs----%d,%d\n",ABS_(-1),ABS_(1));
	abs1 = (APL_4 - APL_3);
	abs2 = (APL_3 - APL_2);
	abs3 = (APL_2 - APL_1);
	/*
	rtd_pr_vpq_info("APL_4/3/2/1,con,abs----%d,%d,%d,%d,%d,%d,%d,%d\t",APL_4,APL_3,APL_2,APL_1,(
		(ABS_(abs1) < 70)
		&&(ABS_(abs2) < 70)
		&&(ABS_(abs3) < 70)
	),ABS_((APL_4 - APL_3)),ABS_((APL_3 - APL_2)),ABS_((APL_2 - APL_1)));*/
	if(
		(ABS_(abs1) < 70)
		&&(ABS_(abs2) < 70)
		&&(ABS_(abs3) < 70)
	){
		//rtd_pr_vpq_info("return 1\n");

		return 1;		
	}
	else{
		//rtd_pr_vpq_info("return 0\n");
		
		return 0;
	}
	
}


unsigned short LC_detect_RGB_box_logo(unsigned int *pAPL_cur,unsigned int *pAPL_pre,int cmd_thl_dC,int cmd_thl_dUDL)
{
	unsigned short flag_pre_current = 0, C_APL = 0, U_APL = 0, D_APL = 0, L_APL = 0, UL_APL = 0, DL_APL = 0, condition_neighbor_brighter = 0, condition_neighbor_darker = 0, neighbor_dY_condition = 0, counter_neighbor_brighter = 0, counter_neighbor_darker = 0, counter_neighbor_UDL = 0,  strick_neighbor_dY_condition = 0, center_Y_condition = 0, condition_counter = 0;
	int d_U_D = 0, d_L_D = 0,  d_L_U = 0, d_C = 0, d_U = 0, d_D = 0, d_L = 0;
	unsigned short C_APL_pre = 0, U_APL_pre = 0, D_APL_pre = 0, L_APL_pre = 0;
	
//get APL
	C_APL = pAPL_cur[189];
	U_APL = pAPL_cur[141];
	D_APL = pAPL_cur[237];
	L_APL = pAPL_cur[188];
	UL_APL = pAPL_cur[140];
	DL_APL = pAPL_cur[236];


//get APL pre
	C_APL_pre = pAPL_pre[189];
	U_APL_pre = pAPL_pre[141];
	D_APL_pre = pAPL_pre[237];
	L_APL_pre = pAPL_pre[188];
	//UL_APL_pre = pAPL_pre[140];
	//DL_APL_pre = pAPL_pre[236];

	d_C = C_APL-C_APL_pre;
	d_U = U_APL-U_APL_pre;
	d_D = D_APL-D_APL_pre;
	d_L = L_APL-L_APL_pre;

//condition pre current
	if(
		(ABS_(d_C) < cmd_thl_dC)
		&&(ABS_(d_U) < cmd_thl_dUDL )
		&&(ABS_(d_D) < cmd_thl_dUDL )
		&&(ABS_(d_L) < cmd_thl_dUDL )  
	)
	{
		flag_pre_current = 1;
	}else{
		flag_pre_current = 0;
	}

	
//condition_neighbor_brighter
	if(U_APL >= C_APL){
		counter_neighbor_brighter +=1;	
	}
	if(D_APL >= C_APL){
		counter_neighbor_brighter +=1;	
	}
	if(L_APL >= C_APL){
		counter_neighbor_brighter +=1;	
	}
	if(UL_APL >= L_APL){
		counter_neighbor_brighter +=1;	
	}
	if(DL_APL >= L_APL){
		counter_neighbor_brighter +=1;	
	}

	condition_neighbor_brighter = ( counter_neighbor_brighter >= 4 );
									
//condition_neighbor_darker		
	if(U_APL <= C_APL){
		counter_neighbor_darker +=1;	
	}
	if(D_APL <= C_APL){
		counter_neighbor_darker +=1;	
	}
	if(L_APL <= C_APL){
		counter_neighbor_darker +=1;	
	}
	if(UL_APL <= L_APL){
		counter_neighbor_darker +=1;	
	}
	if(DL_APL <= L_APL){
		counter_neighbor_darker +=1;	
	}

	condition_neighbor_darker = ( counter_neighbor_darker >= 4 );
	
//neighbor_dY_condition
	d_U_D=U_APL - D_APL;
	d_L_D=L_APL - D_APL;
	d_L_U=L_APL - U_APL;

	neighbor_dY_condition = ( (ABS_(d_U_D )<=300)
							   &&(ABS_(d_L_D )<=300)
							   &&(ABS_(d_L_U )<=300)
							);

//condition_U_D_L	
	if(ABS_(d_U_D )<=50){
		counter_neighbor_UDL +=1;	
	}
	if(ABS_(d_L_D )<=100){
		counter_neighbor_UDL +=1;	
	}
	if(ABS_(d_L_U )<=100){
		counter_neighbor_UDL +=1;	
	}

	strick_neighbor_dY_condition = (counter_neighbor_UDL >= 2);

//Y condition
	//if(C_APL <= 600){
	if(C_APL <= 1023){
		center_Y_condition = 1;
	}else{center_Y_condition = 0;}

//condition counter
	if(condition_neighbor_brighter){
		condition_counter +=1;
	}
	if(condition_neighbor_darker){
		condition_counter +=1;
	}
	if(neighbor_dY_condition){
		condition_counter +=1;
	}
	if(strick_neighbor_dY_condition){
		condition_counter +=1;
	}
	if(center_Y_condition){
		condition_counter +=1;
	}
 
	
	if((condition_counter >= 4)&&flag_pre_current){
			//rtd_pr_vpq_info("box_logo\n");
			return 1;
	}
	else{
			//rtd_pr_vpq_info("not box_logo, 1=%04d, 2=%04d, 3=%04d, 4=%04d, 5=%04d, 6=%04d\n",UL_APL,L_APL,DL_APL,U_APL,C_APL,D_APL);
			return 0;
	}


}



unsigned short LC_detect_noise(unsigned int *pAPL_cur)
{
	unsigned int APL_UL = 0, APL_UR = 0, APL_DL = 0, APL_DR = 0,APL_ULL = 0, APL_DLL = 0;
	unsigned short flag_noise_pattern=0, flag_t_noise_pattern = 0;
	//int abs1 = 0, abs2 = 0, abs3 = 0;
	unsigned short idx_r = 0, idx_c = 0, idx_arr = 0;
	int num_hblk = 0, num_vblk = 0;	
	lc_lc_global_ctrl2_RBUS blu_lc_global_ctrl2;
	unsigned int counter_lower512Y_blk = 0;






//get APL
	APL_ULL = pAPL_cur[234];
	APL_UL = pAPL_cur[235];
	APL_UR = pAPL_cur[236];
	APL_DLL = pAPL_cur[282];
	APL_DL = pAPL_cur[283];
	APL_DR = pAPL_cur[284];


	flag_noise_pattern = 0;
	flag_t_noise_pattern = 0;
	
	if(
		(APL_UL >= 990)
		&&(APL_UR >= 990)
		&&(APL_DL >= 990)
		&&(APL_DR >= 990)
		&&(APL_UL-APL_ULL >= 400)
		&&(APL_DL-APL_DLL >= 400)
	){
		flag_noise_pattern = 1;

	}

	if(
		((APL_UL >= 700)&&(APL_UL <= 800))
		&&((APL_UR >= 700)&&(APL_UR <= 800))
		&&((APL_DL >= 700)&&(APL_DL <= 800))
		&&((APL_DR >= 700)&&(APL_DR <= 800))
		&&(APL_UL-APL_ULL >= 300)
		&&(APL_DL-APL_DLL >= 300)
	){
		flag_t_noise_pattern = 1;

	}

//full frame apl < 512

	
	blu_lc_global_ctrl2.regValue = rtd_inl(LC_LC_Global_Ctrl2_reg);

	num_hblk = blu_lc_global_ctrl2.lc_blk_hnum + 1 ;
	num_vblk = blu_lc_global_ctrl2.lc_blk_vnum + 1;

	idx_arr = 0;
	counter_lower512Y_blk = 0;
	for(idx_r=0;idx_r<(num_vblk);idx_r++){
		for(idx_c=0;idx_c<(num_hblk);idx_c++){
			if(pAPL_cur[idx_arr] <= 512){
				counter_lower512Y_blk = counter_lower512Y_blk + 1;
			}	
			idx_arr = idx_arr+1;
		}
	}


	rtd_pr_vpq_info("counter_lower512Y_blk,APL_ULL,APL_UL,APL_UR,APL_DLL,APL_DL,APL_DR----%d,%d,%d,%d,%d,%d,%d\n",counter_lower512Y_blk,APL_ULL,APL_UL,APL_UR,APL_DLL,APL_DL,APL_DR);

	
	if(
		(
			(
				((flag_noise_pattern == 1) && (flag_t_noise_pattern!=1))
				||((flag_noise_pattern != 1) && (flag_t_noise_pattern==1))
			)
			&&(pAPL_cur[139] <= 800)
			&&(pAPL_cur[140] <= 800)
			&&(pAPL_cur[141] <= 800)
			
		)
		&&(counter_lower512Y_blk >= 1550) 

	){
		//rtd_pr_vpq_info("return 1\n");

		return 1;		
	}
	else{
		//rtd_pr_vpq_info("return 0\n");
		
		return 0;
	}
	
}

void  LC_apply_RGB_box_logo(unsigned int cmd_LC_period, unsigned short cmd_thl_counter_box_logo_case, unsigned short cmd_box_logo_pixel_gain,unsigned short cmd_box_logo_maxGain_time)
{
	DRV_SLD_sld_apply_window_0 sld_apply_window_data;
	DRV_SLD_sld_control sld_control_data;
	DRV_SLD_sld_fw_control_0 fw_control_0_data;
	DRV_SLD_sld_Y_curve_map Y_curve_map_data;

		//apply demo window
		sld_apply_window_data.sld_window_apply_out = 0;
		sld_apply_window_data.sld_window_pos0_x = 1775;
		sld_apply_window_data.sld_window_pos0_y = 75;
		sld_apply_window_data.sld_window_pos1_x = 1848;
		sld_apply_window_data.sld_window_pos1_y = 148;

		sld_control_data.sld_window_en = 1;
		
		//fw control step
		fw_control_0_data.fw_idx_ctrl_en =1;
		fw_control_0_data.fw_idx_ctrl_step=4095;

		//fw control gain	
		
		//fw_control_0_data.fw_pixel_gain=8192<<3;//debug use
		//fw_control_0_data.fw_pixel_gain=CLAMP_SLD(((8192)* cmd_LC_period * (counter_box_logo_case - cmd_thl_counter_box_logo_case))/(13*60),8192,0) ;
		if(counter_box_logo_case > cmd_thl_counter_box_logo_case){
			fw_control_0_data.fw_pixel_gain=CLAMP_SLD(((cmd_box_logo_pixel_gain)* cmd_LC_period * (counter_box_logo_case - cmd_thl_counter_box_logo_case))/cmd_box_logo_maxGain_time,cmd_box_logo_pixel_gain,0) ;
		}
		else{
			fw_control_0_data.fw_pixel_gain=0;
		}
		
/*
		//R,G,B box Y curve
		Y_curve_map_data.sld_curve_seg_0 = 120;
		Y_curve_map_data.sld_curve_seg_1 = 160;
		Y_curve_map_data.sld_curve_seg_2 = 800;
		Y_curve_map_data.sld_new_curve_gain_0 = 0;
		Y_curve_map_data.sld_new_curve_gain_1 = 0;
		Y_curve_map_data.sld_new_curve_gain_2 = 80;
		Y_curve_map_data.sld_new_curve_gain_3 = 64;
		Y_curve_map_data.sld_new_curve_offset_0 = 0;
		Y_curve_map_data.sld_new_curve_offset_1 = 0;
		Y_curve_map_data.sld_new_curve_offset_2 = 65535;
		Y_curve_map_data.sld_new_curve_offset_3 =  0;
*/

		//R,G,B box Y curve
		Y_curve_map_data.sld_curve_seg_0 = 256;
		Y_curve_map_data.sld_curve_seg_1 = 512;
		Y_curve_map_data.sld_curve_seg_2 = 768;
		Y_curve_map_data.sld_new_curve_gain_0 = 32;
		Y_curve_map_data.sld_new_curve_gain_1 = 32;
		Y_curve_map_data.sld_new_curve_gain_2 = 32;
		Y_curve_map_data.sld_new_curve_gain_3 = 32;
		Y_curve_map_data.sld_new_curve_offset_0 = 0;
		Y_curve_map_data.sld_new_curve_offset_1 = 0;
		Y_curve_map_data.sld_new_curve_offset_2 = 0;
		Y_curve_map_data.sld_new_curve_offset_3 =  0;


		
	    	drvif_color_set_SLD_sld_window(&sld_apply_window_data);
        	drvif_color_set_SLD_window_en(&sld_control_data);
        	drvif_color_set_SLD_fw_control_step(&fw_control_0_data);
 		drvif_color_set_SLD_Y_curve_map(&Y_curve_map_data);
		

}


void LC_compute_corner_UR_Y(unsigned int *pAPL_cur, unsigned short  cmd_thl_counter_UR_con,unsigned short* counter_UR)
{
	unsigned short idx_r = 0, idx_c = 0, idx_arr = 0;
	int num_hblk = 0, num_vblk = 0;
	
	lc_lc_global_ctrl2_RBUS blu_lc_global_ctrl2;
	
	blu_lc_global_ctrl2.regValue = rtd_inl(LC_LC_Global_Ctrl2_reg);

	num_hblk = blu_lc_global_ctrl2.lc_blk_hnum + 1 ;
	num_vblk = blu_lc_global_ctrl2.lc_blk_vnum + 1;


	for(idx_r=0;idx_r<(num_vblk);idx_r++){
		for(idx_c=0;idx_c<(num_hblk);idx_c++){
			
			if((idx_c >= (num_hblk-1-14))
				&&(idx_r <= (7))
				&&(pAPL_cur[idx_arr] >= 700)
			){//UR corner high
				*counter_UR = *counter_UR + 1;
			}
			idx_arr = idx_arr + 1;
			
		}
	}

}



void  SLD_apply_noise_pattern(unsigned int cmd_LC_period, unsigned short cmd_thl_counter_noise_case )
{
	DRV_SLD_sld_apply_window_0 sld_apply_window_data;
	DRV_SLD_sld_control sld_control_data;
	DRV_SLD_sld_fw_control_0 fw_control_0_data;
	DRV_SLD_sld_Y_curve_map Y_curve_map_data;

		//apply demo window
		sld_apply_window_data.sld_window_apply_out = 0;
		sld_apply_window_data.sld_window_pos0_x = 1720;
		sld_apply_window_data.sld_window_pos0_y = 100;
		sld_apply_window_data.sld_window_pos1_x = 1819;
		sld_apply_window_data.sld_window_pos1_y = 199;

		sld_control_data.sld_window_en = 1;
		
		//fw control step
		fw_control_0_data.fw_idx_ctrl_en =1;
		fw_control_0_data.fw_idx_ctrl_step=4095;

		//fw control gain	
		
		//fw_control_0_data.fw_pixel_gain=8192<<3;//debug use
		fw_control_0_data.fw_pixel_gain=CLAMP_SLD(((4096)* cmd_LC_period * (counter_noise_case - cmd_thl_counter_noise_case))/(13*60),4096,0) ;

		
/*
		//R,G,B box Y curve
		Y_curve_map_data.sld_curve_seg_0 = 120;
		Y_curve_map_data.sld_curve_seg_1 = 160;
		Y_curve_map_data.sld_curve_seg_2 = 800;
		Y_curve_map_data.sld_new_curve_gain_0 = 0;
		Y_curve_map_data.sld_new_curve_gain_1 = 0;
		Y_curve_map_data.sld_new_curve_gain_2 = 80;
		Y_curve_map_data.sld_new_curve_gain_3 = 64;
		Y_curve_map_data.sld_new_curve_offset_0 = 0;
		Y_curve_map_data.sld_new_curve_offset_1 = 0;
		Y_curve_map_data.sld_new_curve_offset_2 = 65535;
		Y_curve_map_data.sld_new_curve_offset_3 =  0;
*/

		//R,G,B box Y curve
		Y_curve_map_data.sld_curve_seg_0 = 256;
		Y_curve_map_data.sld_curve_seg_1 = 512;
		Y_curve_map_data.sld_curve_seg_2 = 768;
		Y_curve_map_data.sld_new_curve_gain_0 = 32;
		Y_curve_map_data.sld_new_curve_gain_1 = 32;
		Y_curve_map_data.sld_new_curve_gain_2 = 32;
		Y_curve_map_data.sld_new_curve_gain_3 = 32;
		Y_curve_map_data.sld_new_curve_offset_0 = 0;
		Y_curve_map_data.sld_new_curve_offset_1 = 0;
		Y_curve_map_data.sld_new_curve_offset_2 = 0;
		Y_curve_map_data.sld_new_curve_offset_3 =  0;


        	drvif_color_set_SLD_fw_control_step(&fw_control_0_data);		
	    	drvif_color_set_SLD_sld_window(&sld_apply_window_data);
        	drvif_color_set_SLD_window_en(&sld_control_data);
 		drvif_color_set_SLD_Y_curve_map(&Y_curve_map_data);
		

}



void fwif_color_SLD_fw_read_only(unsigned short  cmd_lock_fw_gain_frame)
{

	DRV_SLD_sld_fw_read_only fw_read_only_data;
	DRV_SLD_sld_idx_compensate_by_neighbor_0 idx_compensate_by_neighbor_data;
	DRV_SLD_sld_fw_control_2  fw_control_max_logo_tolerance_data;
	//DRV_SLD_sld_Y_curve_map sld_Y_curve_data;
	//color_sld_sld_fw_control_0_RBUS	 fw_control_0;
	//color_sld_sld_fw_control_1_RBUS	 fw_control_1;



	drvif_color_get_SLD_fw_read_only((DRV_SLD_sld_fw_read_only*) &fw_read_only_data);

	//rtd_pr_vpq_info("max_pre_logo_idx_counter = %d\n",fw_read_only_data.max_pre_logo_idx_counter);
	//rtd_pr_vpq_info("near_max_pre_logo_idx_counter = %d\n",fw_read_only_data.near_max_pre_logo_idx_counter);
	//rtd_pr_vpq_info("hpf_line_count = %d\n",fw_read_only_data.hpf_line_count);
	//rtd_pr_vpq_info("blocknum_with_alpha_noise = %d\n",fw_read_only_data.blocknum_with_alpha_noise);
	//rtd_pr_vpq_info("sld_logo_cnt = %d\n",fw_read_only_data.sld_logo_cnt);
	//rtd_pr_vpq_info("sld_drop_cnt = %d\n",fw_read_only_data.sld_drop_cnt);
	//rtd_pr_vpq_info("sld_max_pre_idx = %d\n",fw_read_only_data.sld_max_pre_idx);

//still image--------------------------------------------------------------------------------------------------------



//logo idx > 3000 has 1/4 area--------------------------------------------------------------------------------------------------------
	fw_control_max_logo_tolerance_data.max_pre_logo_tolerance = (fw_read_only_data.sld_max_pre_idx - 1023);
	drvif_color_set_SLD_max_pre_logo_tolerance((DRV_SLD_sld_fw_control_2*) &fw_control_max_logo_tolerance_data);	

	drvif_color_get_SLD_fw_read_only((DRV_SLD_sld_fw_read_only*) &fw_read_only_data);
	
	if(
		0
		&&(fw_read_only_data.near_max_pre_logo_idx_counter > (129600/2))
		&&(fw_read_only_data.sld_max_pre_idx >= 3000)

	){//logo idx : 4095 ~ 2500
		    rtd_pr_vpq_info("logo idx > 3000 has 1/4 area condition----------------\n");
			/*	
			fw_control_0.regValue  					= rtd_inl(COLOR_SLD_SLD_FW_control_0_reg 			   );
			fw_control_1.regValue  					= rtd_inl(COLOR_SLD_SLD_FW_control_1_reg 			   );

			rtd_outl(COLOR_SLD_SLD_FW_control_0_reg , fw_control_0.regValue);	

			fw_control_1.fw_pixel_gain	            = 4096>>4;
			rtd_outl(COLOR_SLD_SLD_FW_control_1_reg , fw_control_1.regValue);	
			*/
			lock_fw_gain = cmd_lock_fw_gain_frame;

	}

//spatial compensate--------------------------------------------------------------------------------------------------------
	idx_compensate_by_neighbor_data.compensate_neighbor_tolerance = 1023;
	idx_compensate_by_neighbor_data.idx_compensate_start_thl = 1023;
	if(fw_read_only_data.near_max_pre_logo_idx_counter < (129600/15)){
		idx_compensate_by_neighbor_data.idx_compensate_neighbor_counter_thl = 3;
		rtd_pr_vpq_info("spatial compensate condition = 3----------------\n");
		
	}else{
		idx_compensate_by_neighbor_data.idx_compensate_neighbor_counter_thl = 9;
		//rtd_pr_vpq_info("spatial compensate condition = 9----------------\n");		
	}
	drvif_color_set_SLD_compensate_spatial((DRV_SLD_sld_idx_compensate_by_neighbor_0*) &idx_compensate_by_neighbor_data);


}


void fwif_color_SLD_add_con(void)
{
	DRV_SLD_sld_fw_read_only fw_read_only_data;
	DRV_SLD_sld_acc_0 SLD_Idx_add_con_data;	

	drvif_color_get_SLD_fw_read_only((DRV_SLD_sld_fw_read_only*) &fw_read_only_data);

	SLD_Idx_add_con_data.sld_hpf_logo_add_thl = 128;
	SLD_Idx_add_con_data.sld_y_idx_thl = 256;
	SLD_Idx_add_con_data.sld_frame_diffy_thl = 256;
	SLD_Idx_add_con_data.sld_max_idx_thl = 4095;

	if(
		(fw_read_only_data.max_pre_logo_idx_counter < 256) 
		&&(cur_frame_idx_global%2 == 0)
	)
	{
		SLD_Idx_add_con_data.sld_idx_acc_condition_and = 0;
		drvif_color_set_SLD_Idx_add_con((DRV_SLD_sld_acc_0*) &SLD_Idx_add_con_data);
		//rtd_pr_vpq_info("add_con = %d\n",SLD_Idx_add_con_data.sld_idx_acc_condition_and);
	}
	else 
	{
		SLD_Idx_add_con_data.sld_idx_acc_condition_and = 1;
		drvif_color_set_SLD_Idx_add_con((DRV_SLD_sld_acc_0*) &SLD_Idx_add_con_data);
		//rtd_pr_vpq_info("add_con = %d\n",SLD_Idx_add_con_data.sld_idx_acc_condition_and);
	}

}

void Count_num_highY_block_up_Left_Frame(int num_hblk, unsigned short *pAPL_cur, unsigned int thl_highY_frame, unsigned int *counter_highY_blk)
{
    unsigned short idx_r = 0, idx_c = 0, start_x = 0;
    int h_num = 0;


    start_x  = num_hblk * 2 / 3; //start from 2/3
    h_num = num_hblk - start_x; //remain 1/3
    *counter_highY_blk = 0;
    //apl_hi_block_count = 0;
    for (idx_r = 0; idx_r < 10; idx_r++) {
        for (idx_c = start_x; idx_c < (num_hblk); idx_c++) {
            if (pAPL_cur[idx_r * h_num + idx_c] > thl_highY_frame) {
                *counter_highY_blk = *counter_highY_blk + 1;
            }
            /*
            if(pAPL_cur[idx_r * h_num + idx_c]>=1800){
                apl_hi_block_count++;
            }*/
        }
    }

}


void LC_count_num_highY_block_upFrame(unsigned int *pAPL_cur, unsigned int thl_highY_frame,unsigned int* counter_highY_blk)
{
	unsigned short idx_r = 0, idx_c = 0, idx_arr = 0;
	int num_hblk = 0;
	
	lc_lc_global_ctrl2_RBUS blu_lc_global_ctrl2;
	
	blu_lc_global_ctrl2.regValue = rtd_inl(LC_LC_Global_Ctrl2_reg);

	num_hblk = blu_lc_global_ctrl2.lc_blk_hnum + 1 ;
	//num_vblk = blu_lc_global_ctrl2.lc_blk_vnum + 1;

	*counter_highY_blk = 0;
	for(idx_r=0;idx_r<10;idx_r++){
		for(idx_c=0;idx_c<(num_hblk);idx_c++){
			if(pAPL_cur[idx_arr] > thl_highY_frame){
				*counter_highY_blk = *counter_highY_blk + 1;
			}						
		}
	}

}

void Compute_pre_cur_deltaY(int num_hblk, int num_vblk, unsigned short *pAPL_pre, unsigned short *pAPL_cur, unsigned int *APL_avg_cur, unsigned short *pAPL_delta, unsigned int *counter_blk_pre_cur_same, int thl_temporal_dY)
{
    unsigned short idx_r = 0, idx_c = 0, idx_arr = 0;
    int dY_pre_cur = 0;

    for (idx_r = 0; idx_r < (num_vblk); idx_r++) {
        for (idx_c = 0; idx_c < (num_hblk); idx_c++) {
            *APL_avg_cur += pAPL_cur[idx_arr];
            dY_pre_cur = pAPL_pre[idx_arr] - pAPL_cur[idx_arr];
            pAPL_delta[idx_arr] = ABS_(dY_pre_cur);

            if (pAPL_delta[idx_arr] < thl_temporal_dY) {
                *counter_blk_pre_cur_same = *counter_blk_pre_cur_same + 1;
            }
            idx_arr = idx_arr + 1;
        }
    }
    *APL_avg_cur = *APL_avg_cur / (num_vblk * num_hblk);

}


void LC_compute_pre_cur_deltaY(unsigned int *pAPL_pre, unsigned int *pAPL_cur,unsigned int *pAPL_delta, unsigned int *counter_blk_pre_cur_same, int thl_temporal_dY)
{
	unsigned short idx_r = 0, idx_c = 0, idx_arr = 0;
	int num_hblk = 0, num_vblk = 0, dY_pre_cur = 0;
	
	lc_lc_global_ctrl2_RBUS blu_lc_global_ctrl2;
	
	blu_lc_global_ctrl2.regValue = IoReg_Read32(LC_LC_Global_Ctrl2_reg);

	num_hblk = blu_lc_global_ctrl2.lc_blk_hnum + 1 ;
	num_vblk = blu_lc_global_ctrl2.lc_blk_vnum + 1;


	for(idx_r=0;idx_r<(num_vblk);idx_r++){
		for(idx_c=0;idx_c<(num_hblk);idx_c++){
			
			dY_pre_cur = pAPL_pre[idx_arr]- pAPL_cur[idx_arr];		
			pAPL_delta[idx_arr] = ABS_(dY_pre_cur);

			if(pAPL_delta[idx_arr] < thl_temporal_dY){
				*counter_blk_pre_cur_same = *counter_blk_pre_cur_same+1;
			}			
				idx_arr = idx_arr + 1;			
		}
	}

}

unsigned char drvif_color_set_SLD_LCblk_inform(void)
{
	lc_lc_global_ctrl1_RBUS   lc_global_ctrl1_reg;
	color_d_vc_global_ctrl_RBUS   d_vc_global_ctrl_reg;
	lc_lc_inter_hctrl2_RBUS   lc_inter_hctrl2_reg;
	lc_lc_inter_vctrl2_RBUS   lc_inter_vctrl2_reg;
	lc_lc_global_ctrl2_RBUS   lc_global_ctrl2_reg;
	lc_lc_global_ctrl3_RBUS   lc_global_ctrl3_reg;
	lc_lc_inter_hctrl1_RBUS   lc_inter_hctrl1_reg;
	lc_lc_inter_vctrl1_RBUS   lc_inter_vctrl1_reg;
	lc_lc_global_ctrl0_RBUS   lc_global_ctrl0_reg;	


	lc_global_ctrl1_reg.regValue = IoReg_Read32(LC_LC_Global_Ctrl1_reg);
	d_vc_global_ctrl_reg.regValue = IoReg_Read32(COLOR_D_VC_Global_CTRL_reg);
	lc_inter_hctrl2_reg.regValue = IoReg_Read32(LC_LC_Inter_HCtrl2_reg);
	lc_inter_vctrl2_reg.regValue = IoReg_Read32(LC_LC_Inter_VCtrl2_reg);
	lc_global_ctrl2_reg.regValue = IoReg_Read32(LC_LC_Global_Ctrl2_reg);
	lc_global_ctrl3_reg.regValue = IoReg_Read32(LC_LC_Global_Ctrl3_reg);
	lc_inter_hctrl1_reg.regValue = IoReg_Read32(LC_LC_Inter_HCtrl1_reg);
	lc_inter_vctrl1_reg.regValue = IoReg_Read32(LC_LC_Inter_VCtrl1_reg);
	lc_global_ctrl0_reg.regValue = IoReg_Read32(LC_LC_Global_Ctrl0_reg);	
	
	lc_global_ctrl1_reg.lc_db_en = 0;// disable double buffer apply
	lc_inter_hctrl2_reg.lc_hboundary = 0;
	lc_inter_hctrl2_reg.lc_hinitphase1 = 0;
	lc_inter_hctrl2_reg.lc_hinitphase2 = 3;
	lc_inter_hctrl2_reg.lc_hinitphase3_left = 0x80;
	lc_inter_vctrl2_reg.lc_vboundary = 0;
	lc_inter_vctrl2_reg.lc_vinitphase1 = 0;
	lc_inter_vctrl2_reg.lc_vinitphase2 = 4;
	lc_inter_vctrl2_reg.lc_vinitphase3_left = 0x80;
	lc_global_ctrl2_reg.lc_hist_mode = 2;
	lc_global_ctrl2_reg.lc_tenable = 0;
	lc_global_ctrl2_reg.lc_valid = 1;
	lc_global_ctrl0_reg.lc_apl_en = 0;
	lc_global_ctrl0_reg.lc_local_sharp_en = 0;
	lc_global_ctrl0_reg.lc_tone_mapping_en = 0;
	lc_global_ctrl0_reg.lc_debugmode = 0;
	
	lc_global_ctrl2_reg.lc_blk_hnum = 47;
	lc_global_ctrl2_reg.lc_blk_vnum = 33;
	lc_global_ctrl3_reg.lc_blk_hsize = 80;
	lc_global_ctrl3_reg.lc_blk_vsize = 64;
	lc_inter_hctrl1_reg.lc_tab_hsize = 4;
	lc_inter_hctrl1_reg.lc_hfactor = 65536;
	lc_inter_vctrl1_reg.lc_tab_vsize = 4;
	lc_inter_vctrl1_reg.lc_vfactor = 82527;

	/*enable LC*/
	d_vc_global_ctrl_reg.m_vc_en		= 1;
	d_vc_global_ctrl_reg.write_enable_11 =1;
	d_vc_global_ctrl_reg.m_lc_comp_en = 1;
	d_vc_global_ctrl_reg.write_enable_12 =1;


	IoReg_Write32(LC_LC_Global_Ctrl1_reg,lc_global_ctrl1_reg.regValue);
	IoReg_Write32(COLOR_D_VC_Global_CTRL_reg,d_vc_global_ctrl_reg.regValue);
	IoReg_Write32(LC_LC_Inter_HCtrl2_reg,lc_inter_hctrl2_reg.regValue);
	IoReg_Write32(LC_LC_Inter_VCtrl2_reg,lc_inter_vctrl2_reg.regValue);
	IoReg_Write32(LC_LC_Global_Ctrl2_reg,lc_global_ctrl2_reg.regValue);
	IoReg_Write32(LC_LC_Global_Ctrl3_reg,lc_global_ctrl3_reg.regValue);
	IoReg_Write32(LC_LC_Inter_HCtrl1_reg,lc_inter_hctrl1_reg.regValue);
	IoReg_Write32(LC_LC_Inter_VCtrl1_reg,lc_inter_vctrl1_reg.regValue);
	IoReg_Write32(LC_LC_Global_Ctrl0_reg,lc_global_ctrl0_reg.regValue);

	return 1;

}


unsigned int drvif_color_SLD_set_noise_pattern(unsigned int temporal_diff,unsigned int big_temporal_diff_low,unsigned int big_temporal_diff_up, unsigned int  temporal_counter_thl)
{
	DRV_SLD_sld_temporal_noise_1 temporal_noise_1_data;
	color_sld_sld_temporal_noise_2_RBUS sld_temporal_noise_2;

	temporal_noise_1_data.temporal_diff = temporal_diff;
	temporal_noise_1_data.big_temporal_diff_low = big_temporal_diff_low;
	temporal_noise_1_data.big_temporal_diff_up = big_temporal_diff_up;
	temporal_noise_1_data.temporal_counter_thl = temporal_counter_thl;

	drvif_color_set_SLD_sld_temporal_noise((DRV_SLD_sld_temporal_noise_1*) &temporal_noise_1_data);
	
	sld_temporal_noise_2.regValue = rtd_inl(COLOR_SLD_SLD_Temporal_Noise_2_reg);
	//rtd_pr_vpq_info("temporal_counter = %d\n",sld_temporal_noise_2.temporal_counter);

	return sld_temporal_noise_2.temporal_counter;
	
}


void fwif_color_apply_NEWS_setting(void)
{

	DRV_SLD_sld_Y_curve_map sld_Y_curve_data;
	color_sld_sld_fw_control_0_RBUS	 fw_control_0;
	//color_sld_sld_fw_control_1_RBUS	 fw_control_1;


		//fw control 
			fw_control_0.regValue  					= IoReg_Read32(COLOR_SLD_SLD_FW_control_0_reg 			   );
			//fw_control_1.regValue  					= IoReg_Read32(COLOR_SLD_SLD_FW_control_1_reg 			   );

			fw_control_0.fw_idx_ctrl_step	        = 3;
			fw_control_0.fw_idx_ctrl_en 			= 1;
			IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg , fw_control_0.regValue);	

		//Y curve setting
			sld_Y_curve_data.sld_curve_seg_0 = 512;
			sld_Y_curve_data.sld_curve_seg_1 = 768;
			sld_Y_curve_data.sld_curve_seg_2 = 800;

			sld_Y_curve_data.sld_new_curve_gain_0 =64;
			sld_Y_curve_data.sld_new_curve_gain_1 =64;
			sld_Y_curve_data.sld_new_curve_gain_2 =64;
			sld_Y_curve_data.sld_new_curve_gain_3 =5;

			sld_Y_curve_data.sld_new_curve_offset_0 = 0;
			sld_Y_curve_data.sld_new_curve_offset_1 = 0;
			sld_Y_curve_data.sld_new_curve_offset_2 = 0;
			sld_Y_curve_data.sld_new_curve_offset_3 = 733;

			drvif_color_set_SLD_Y_curve_map((DRV_SLD_sld_Y_curve_map*) &sld_Y_curve_data);

}

void  fwif_color_SLD_disable_window(void)
{
	DRV_SLD_sld_control sld_control_data;

	sld_control_data.sld_window_en = 0;
		
       drvif_color_set_SLD_window_en((DRV_SLD_sld_control*) &sld_control_data);
		
}

void	fwif_color_set_SLD_LCblk_inform(void)
{
	static unsigned char flag_SLD_init_LCblk = 0; 
	
	if(flag_SLD_init_LCblk == 0){
		flag_SLD_init_LCblk = drvif_color_set_SLD_LCblk_inform();
	}

}

unsigned int drvif_color_SLD_get_noise_pattern_counter(void)
{
	color_sld_sld_temporal_noise_2_RBUS sld_temporal_noise_2;
	
	sld_temporal_noise_2.regValue = rtd_inl(COLOR_SLD_SLD_Temporal_Noise_2_reg);
	rtd_pr_vpq_info("temporal_counter = %d\n",sld_temporal_noise_2.temporal_counter);

	return sld_temporal_noise_2.temporal_counter;
	
}


void drvif_color_set_SLD_default(void)
{

	IoReg_Write32(COLOR_SLD_SLD_CONTROL_reg,0x03e00005);
	IoReg_Write32(COLOR_SLD_SLD_SIZE_reg,0x07800438);
	IoReg_Write32(COLOR_SLD_SLD_CURVE_MAP_SEG_reg,0x03000200);
	IoReg_Write32(COLOR_SLD_SLD_CURVE_MAP_GAIN_reg,0x00000000);
	IoReg_Write32(COLOR_SLD_SLD_CURVE_MAP_OFFSET_0_reg,0x00000000);
	IoReg_Write32(COLOR_SLD_SLD_CURVE_MAP_OFFSET_1_reg,0x00000000);
	IoReg_Write32(COLOR_SLD_SLD_DROP_reg,0x18040000);
	IoReg_Write32(COLOR_SLD_SLD_CNT_0_reg,0x00007b36);
	IoReg_Write32(COLOR_SLD_SLD_CNT_1_reg,0x00003053);
	IoReg_Write32(COLOR_SLD_SLD_RGB2Y_coef_1_reg,0x02590132);
	IoReg_Write32(COLOR_SLD_SLD_RGB2Y_coef_2_reg,0x00000075);
	IoReg_Write32(COLOR_SLD_SLD_NEW_CURVE_MAP_SEG_1_reg,0x00000320);
	IoReg_Write32(COLOR_SLD_SLD_NEW_CURVE_MAP_GAIN_0_reg,0x00400040);
	IoReg_Write32(COLOR_SLD_SLD_NEW_CURVE_MAP_GAIN_1_reg,0x00220040);
	IoReg_Write32(COLOR_SLD_SLD_NEW_CURVE_MAP_OFFSET_0_reg,0x00000000);
	IoReg_Write32(COLOR_SLD_SLD_NEW_CURVE_MAP_OFFSET_1_reg,0x016e0000);
	IoReg_Write32(COLOR_SLD_SLD_HPF_0_reg,0x00800000);
	IoReg_Write32(COLOR_SLD_SLD_HPF_1_reg,0x001f0001);
	IoReg_Write32(COLOR_SLD_SLD_IN_LOGO_THL_0_reg,0x00800080);
	IoReg_Write32(COLOR_SLD_SLD_IN_LOGO_THL_1_reg,0x00800080);
	IoReg_Write32(COLOR_SLD_SLD_IN_LOGO_THL_2_reg,0x00800080);
	IoReg_Write32(COLOR_SLD_SLD_IN_LOGO_THL_3_reg,0x00800080);
	IoReg_Write32(COLOR_SLD_SLD_ACC_0_reg,0x90000100);
	IoReg_Write32(COLOR_SLD_SLD_ACC_1_reg,0x00000100);
	IoReg_Write32(COLOR_SLD_SLD_NEIGHBOR_DROP_0_reg,0x03ff0000);
	IoReg_Write32(COLOR_SLD_SLD_NEIGHBOR_DROP_1_reg,0x000003ff);
	IoReg_Write32(COLOR_SLD_SLD_NEIGHBOR_NOT_DROP_reg,0x00000000);
	IoReg_Write32(COLOR_SLD_SLD_CNT_2_reg,0x00000fff);
	IoReg_Write32(COLOR_SLD_SLD_LPF_0_reg,0x00040001);
	IoReg_Write32(COLOR_SLD_SLD_LPF_1_reg,0x00040006);
	IoReg_Write32(COLOR_SLD_SLD_LPF_2_reg,0x00040001);
	IoReg_Write32(COLOR_SLD_SLD_LPF_3_reg,0x00180010);
	IoReg_Write32(COLOR_SLD_SLD_LPF_4_reg,0x00040010);
	IoReg_Write32(COLOR_SLD_SLD_LPF_5_reg,0x00180006);
	IoReg_Write32(COLOR_SLD_SLD_LPF_6_reg,0x00180024);
	IoReg_Write32(COLOR_SLD_SLD_LPF_7_reg,0x00040006);
	IoReg_Write32(COLOR_SLD_SLD_LPF_8_reg,0x00180010);
	IoReg_Write32(COLOR_SLD_SLD_LPF_9_reg,0x00040010);
	IoReg_Write32(COLOR_SLD_SLD_LPF_10_reg,0x00040001);
	IoReg_Write32(COLOR_SLD_SLD_LPF_11_reg,0x00040006);
	IoReg_Write32(COLOR_SLD_SLD_LPF_12_reg,0x00000001);
	IoReg_Write32(COLOR_SLD_SLD_LPF_DIV_reg,0x00000008);
	IoReg_Write32(COLOR_SLD_SLD_Apply_Window_0_reg,0x00000000);
	IoReg_Write32(COLOR_SLD_SLD_Apply_Window_1_reg,0x086f0eff);
	IoReg_Write32(COLOR_SLD_SLD_IN_LOGO_THL_4_reg,0x0fff0fff);
	IoReg_Write32(COLOR_SLD_SLD_IN_LOGO_THL_5_reg,0x0fff0fff);
	IoReg_Write32(COLOR_SLD_SLD_FW_read_only_0_reg,0x00000800);
	IoReg_Write32(COLOR_SLD_SLD_FW_read_only_1_reg,0x00000800);
	IoReg_Write32(COLOR_SLD_SLD_FW_read_only_2_reg,0x363a0000);
	IoReg_Write32(COLOR_SLD_SLD_FW_read_only_3_reg,0x00000005);
	IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg,0x03ff0000);
	//IoReg_Write32(COLOR_SLD_SLD_FW_control_1_reg,0x00001000);//FW_pixel_gain
	IoReg_Write32(COLOR_SLD_SLD_FW_control_2_reg,0x00000040);
	IoReg_Write32(COLOR_SLD_SLD_Drop_Slow_0_reg,0x0bb80a8c);
	IoReg_Write32(COLOR_SLD_SLD_Drop_Slow_1_reg,0x00000040);
	IoReg_Write32(COLOR_SLD_SLD_Temporal_Noise_0_reg,0x00000190);
	IoReg_Write32(COLOR_SLD_SLD_Temporal_Noise_1_reg,0x01900000);
	IoReg_Write32(COLOR_SLD_SLD_Temporal_Noise_2_reg,0x00000000);
	IoReg_Write32(COLOR_SLD_SLD_Temporal_Noise_3_reg,0x0001fa40);
	IoReg_Write32(COLOR_SLD_SLD_Idx_Protect_By_Neighbor_0_reg,0x00000009);
	IoReg_Write32(COLOR_SLD_SLD_BIST_DRF_FAIL_reg,0x00000000);
	IoReg_Write32(COLOR_SLD_SLD_BIST_FAIL_reg,0x00000000);
	IoReg_Write32(COLOR_SLD_SLD_BIST_STATUS_reg,0x00000000);
	IoReg_Write32(COLOR_SLD_SLD_BIST_CTRL_reg,0x00000000);
	IoReg_Write32(COLOR_SLD_SLD_Idx_Protect_By_Neighbor_1_reg,0x03ff03ff);
	IoReg_Write32(COLOR_SLD_SLD_Idx_not_drop_0_reg,0x000003ff);
	IoReg_Write32(COLOR_SLD_SLD_Idx_not_drop_1_reg,0x02000080);
	IoReg_Write32(COLOR_SLD_SLD_Idx_Compensate_By_Neighbor_0_reg,0x00000005);
	IoReg_Write32(COLOR_SLD_SLD_Idx_Compensate_By_Neighbor_1_reg,0x0bb80bb8);
	IoReg_Write32(COLOR_SLD_SLD_Temporal_Idx_Compensate_reg,0x000a003f);
	IoReg_Write32(COLOR_SLD_SLD_mouse_0_reg,0x00000000);
	IoReg_Write32(COLOR_SLD_SLD_mouse_1_reg,0x0004000a);
	IoReg_Write32(COLOR_SLD_SLD_mouse_2_reg,0x00000000);
	IoReg_Write32(COLOR_SLD_SLD_mouse_3_reg,0x00000300);
	IoReg_Write32(COLOR_SLD_SLD_mouse_4_reg,0x00000016);
	IoReg_Write32(COLOR_SLD_SLD_hpf_in_line_reg,0x0000017c);
	IoReg_Write32(COLOR_SLD_SLD_noise_pattern_0_reg,0x000100a0);
	IoReg_Write32(COLOR_SLD_SLD_noise_pattern_1_reg,0x000c0258);
	IoReg_Write32(COLOR_SLD_SLD_noise_pattern_2_reg,0x00d00258);
	IoReg_Write32(COLOR_SLD_SLD_IDX_MAP_SEG_reg,0x04000200);
	IoReg_Write32(COLOR_SLD_SLD_NEW_IDX_MAP_SEG_1_reg,0x00000bb8);
	IoReg_Write32(COLOR_SLD_SLD_NEW_IDX_MAP_GAIN_0_reg,0x00000000);
	IoReg_Write32(COLOR_SLD_SLD_NEW_IDX_MAP_GAIN_1_reg,0x00010061);
	IoReg_Write32(COLOR_SLD_SLD_NEW_IDX_MAP_OFFSET_0_reg,0x00000000);
	IoReg_Write32(COLOR_SLD_SLD_NEW_IDX_MAP_OFFSET_1_reg,0x0b64f9ee);
	IoReg_Write32(COLOR_SLD_SLD_histogram_0_reg,0x00085000);
	IoReg_Write32(COLOR_SLD_SLD_histogram_1_reg,0x00800000);
	IoReg_Write32(COLOR_SLD_SLD_histogram_2_reg,0x01800100);
	IoReg_Write32(COLOR_SLD_SLD_histogram_3_reg,0x02800200);
	IoReg_Write32(COLOR_SLD_SLD_histogram_4_reg,0x03800300);
	// IoReg_Write32(COLOR_SLD_SLD_histogram_ro_0_reg,0x0ea23ba3);
	// IoReg_Write32(COLOR_SLD_SLD_histogram_ro_1_reg,0x06a70c29);
	// IoReg_Write32(COLOR_SLD_SLD_histogram_ro_2_reg,0x09b40579);
	// IoReg_Write32(COLOR_SLD_SLD_histogram_ro_3_reg,0x62a52dc7);
	IoReg_Write32(COLOR_SLD_SLD_noise_pattern_3_reg,0x00001000);

	fwif_color_SLD_default_window();
       drvif_color_SLD_LPF_ur();
	drvif_color_SLD_set_noise_pattern(0,0,40,129600);

}




#endif


#if 0/*SLD, hack, elieli*/
#define SLD_Debug 0

bool drvif_set_sld_dma(void)
{
	unsigned int sld_size = 0;
	unsigned long sld_addr = 0;
	unsigned long align_value =0;

	if (drvif_color_get_HW_MEX_STATUS(MEX_SLD) != 1) {
		rtd_pr_vpq_info("HW_MEX,  MEX_SLD != 1, DMA\n");
		return false;
	}

#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
	sld_size = carvedout_buf_query_secure(CARVEDOUT_SCALER_OD, (void*) &sld_addr);
#else
	sld_size = carvedout_buf_query(CARVEDOUT_SCALER_OD,(void*) &sld_addr);
#endif
	if (sld_addr == 0 || sld_size == 0) {
		VIPrtd_pr_vpq_info("[%s %d] ERR : %x %x\n", __func__, __LINE__, sld_addr, sld_size);
                return false;
	}

#if 1
    align_value = dvr_memory_alignment((unsigned long)sld_addr, dvr_size_alignment(500000)); //96 Alignment for Bandwidth Request

    if (align_value == 0)
        BUG();
#else
        align_value = sld_addr; //origin value
#endif

        VIPprintf("aligned phy addr=%x,\n", align_value);


	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_addr_0_reg, align_value); //must be check 96 alignment	
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_addr_1_reg, align_value+0x02f00000); //Cap_sta_addr_0, must be check 96 alignment

	IoReg_Write32(OD_DMA_ODDMA_WR_Rule_check_up_reg, align_value+0x02f00000+0x2f000000  );//Cap_up_limit
	IoReg_Write32(OD_DMA_ODDMA_WR_Rule_check_low_reg, sld_addr);
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_addr_0_reg, align_value);//must be check 96 alignment	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_addr_0_reg, align_value);//must be check 96 alignment
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_addr_1_reg, align_value+0x02f00000); //Cap_sta_addr_0, must be check 96 alignment

	IoReg_Write32(OD_DMA_ODDMA_RD_Rule_check_up_reg, align_value+0x02f00000+0x2f000000  );//Disp_up_limit
	IoReg_Write32(OD_DMA_ODDMA_RD_Rule_check_low_reg, sld_addr);

	VIPprintf("[sld]drvif_set_sld_dma...end\n");

	return true;
}

// Function Description : This function is for SLD enable
//juwen, 0603, done
void SLD_k6_HAL(int Y1, int Y2, int Y3, int gain_low,unsigned char UIsel_L)
{

#ifdef VPQ_COMPILER_ERROR_ENABLE_MERLIN6
	int reg_seg0 = 256;  //fixed
	int reg_seg1 = 768;  //fixed

	int reg_gain0 = 0 ;//fixed
	int reg_gain1 = 0 ; //fixed
	int reg_gain2 = 0 ; //fixed

	int reg_offset0 = 0; //fixed
	int reg_offset1 = 0;
	int reg_offset2 = 0;

	color_sld_sld_control_RBUS color_sld_control;
	color_sld_sld_drop_RBUS color_sld_drop;

	DRV_SLD_CurveMap sld_curve_map;

	int y_out_target[4] = {0};

	color_sld_control.regValue = IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg);
	color_sld_drop.regValue = IoReg_Read32(COLOR_SLD_SLD_DROP_reg);

	color_sld_control.sld_en = 0;
	color_sld_control.sld_blend_en= 1;
	color_sld_control.sld_global_drop= 0;
	color_sld_control.sld_hpf_type= 0;
	color_sld_control.sld_hpf_thl= 255;
	color_sld_control.sld_scale_mode= 1;
	color_sld_drop.sld_cnt_thl = 0;
	color_sld_drop.sld_hpf_diff_thl = 16;
	color_sld_drop.sld_pixel_diff_thl = 16;


	IoReg_Write32(COLOR_SLD_SLD_CONTROL_reg, color_sld_control.regValue);
	IoReg_Write32(COLOR_SLD_SLD_DROP_reg, color_sld_drop.regValue);





	if(UIsel_L == 1)
	{
	        y_out_target[0] =  (0    *gain_low)/1024;
	        y_out_target[1] =  ( Y1 *gain_low)/1024;
	        y_out_target[2] =  ( Y2 *gain_low)/1024;
	        y_out_target[3] =  ( Y3 *gain_low)/1024;
	}
	else
	{
	        y_out_target[0] =  (0   *1024)/1024;
	        y_out_target[1] =  (Y1 *1024)/1024;
	        y_out_target[2] =  (Y2 *1024)/1024;
	        y_out_target[3] =  (Y3 *1024)/1024;

	}

	reg_offset0 = y_out_target[0];
	reg_gain0 =( (y_out_target[1]- reg_offset0)*64)/reg_seg0;
	reg_gain1 =( (y_out_target[2]- reg_offset0-((reg_gain0*reg_seg0)/64))*64)/(reg_seg1-reg_seg0);
	reg_gain2 =( (y_out_target[3]- reg_offset0-((reg_gain0*reg_seg0)/64)-((reg_gain1*(reg_seg1-reg_seg0))/64))*64)/(1023-reg_seg1);

	reg_offset1 =  reg_offset0 + (reg_gain0*reg_seg0)/64 - (reg_gain1*reg_seg0)/64;
	reg_offset2 =  reg_offset1 + (reg_gain1*reg_seg1)/64 - (reg_gain2*reg_seg1)/64;

 	sld_curve_map.sld_curve_seg_0 = 256;
	sld_curve_map.sld_curve_seg_1 = 768;

	if(reg_gain0 < 0)
	{
	 	sld_curve_map.sld_curve_gain_0= 256 - (short)reg_gain0;
	}
	else
	{
	 	sld_curve_map.sld_curve_gain_0= (short)reg_gain0;
	}
	if(reg_gain1 < 0)
	{
	 	sld_curve_map.sld_curve_gain_1= 256 - (short)reg_gain1;
	}
	else
	{
	 	sld_curve_map.sld_curve_gain_1= (short)reg_gain1;
	}
	if(reg_gain2 < 0)
	{
	 	sld_curve_map.sld_curve_gain_2= 256 - (short)reg_gain2;
	}
	else
	{
	 	sld_curve_map.sld_curve_gain_2= (short)reg_gain2;
	}


	if(reg_offset0 < 0)
	{
		sld_curve_map.sld_curve_offset_0= 2048 - (short)reg_offset0;
	}
	else
	{
		sld_curve_map.sld_curve_offset_0= (short)reg_offset0;
	}

	if(reg_offset1 < 0)
	{
		sld_curve_map.sld_curve_offset_1= 2048 - (short)reg_offset1;
	}
	else
	{
		sld_curve_map.sld_curve_offset_1= (short)reg_offset1;
	}

	if(reg_offset2 < 0)
	{
		sld_curve_map.sld_curve_offset_2= 2048 - (short)reg_offset2;
	}
	else
	{
		sld_curve_map.sld_curve_offset_2= (short)reg_offset2;
	}


	drvif_color_set_SLD_curve_map(&sld_curve_map);

#endif


}
/*
void drvif_color_set_SLD_4k_dma_setting()
{
	unsigned char sld_dma_return = false;
        color_sld_sld_size_RBUS color_sld_sld_size;
        od_dma_oddma_wr_ctrl_RBUS      od_dma_oddma_wr_ctrl;
        od_dma_oddma_rd_ctrl_RBUS       od_dma_oddma_rd_ctrl;
        od_dma_oddma_wr_num_bl_wrap_addr_0_RBUS        od_dma_oddma_wr_num_bl_wrap_addr_0;
        od_dma_oddma_wr_num_bl_wrap_addr_1_RBUS        od_dma_oddma_wr_num_bl_wrap_addr_1;
        od_dma_oddma_rd_num_bl_wrap_addr_0_RBUS od_dma_oddma_rd_num_bl_wrap_addr_0;
        od_dma_oddma_wr_num_bl_wrap_word_RBUS   od_dma_oddma_wr_num_bl_wrap_word;
        od_dma_oddma_rd_num_bl_wrap_addr_1_RBUS od_dma_oddma_rd_num_bl_wrap_addr_1;
        od_dma_oddma_rd_num_bl_wrap_word_RBUS    od_dma_oddma_rd_num_bl_wrap_word;
        od_dma_oddma_rd_num_bl_wrap_line_step_RBUS      od_dma_oddma_rd_num_bl_wrap_line_step;
        od_dma_oddma_wr_num_bl_wrap_line_step_RBUS     od_dma_oddma_wr_num_bl_wrap_line_step;
        od_dma_oddma_wr_num_bl_wrap_ctl_RBUS       od_dma_oddma_wr_num_bl_wrap_ctl;
        od_dma_oddma_rd_num_bl_wrap_ctl_RBUS        od_dma_oddma_rd_num_bl_wrap_ctl;
        od_dma_od_dma_ctrl_RBUS  od_dma_od_dma_ctrl;
        od_dma_od_dma_db_en_RBUS od_dma_od_dma_db_en;
	 color_sld_sld_control_RBUS color_sld_control;
	 od_dma_oddma_wr_rule_check_up_RBUS  od_dma_oddma_wr_rule_check_up;
	 od_dma_oddma_rd_rule_check_up_RBUS	 od_dma_oddma_rd_rule_check_up;
	 od_dma_oddma_pq_cmp_RBUS od_dma_oddma_pq_cmp;
	 od_dma_oddma_pq_decmp_RBUS od_dma_oddma_pq_decmp;
	od_od_ctrl_RBUS od_od_ctrl;

         od_dma_od_dma_db_en.regValue = IoReg_Read32(OD_DMA_OD_DMA_DB_EN_reg);
	 od_dma_od_dma_db_en.disp_db_en = 0;//

	 sld_dma_return = drvif_set_sld_dma();

        color_sld_sld_size.regValue = IoReg_Read32(COLOR_SLD_SLD_SIZE_reg);
        od_dma_oddma_wr_ctrl.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_Ctrl_reg);
        od_dma_oddma_rd_ctrl.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_Ctrl_reg);
        od_dma_oddma_wr_num_bl_wrap_addr_0.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_addr_0_reg);
        od_dma_oddma_wr_num_bl_wrap_addr_1.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_addr_1_reg);
        od_dma_oddma_rd_num_bl_wrap_addr_0.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_addr_0_reg);
        od_dma_oddma_rd_num_bl_wrap_addr_1.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_addr_1_reg);
        od_dma_oddma_wr_num_bl_wrap_word.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_word_reg);
        od_dma_oddma_rd_num_bl_wrap_word.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_word_reg);
        od_dma_oddma_rd_num_bl_wrap_line_step.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_line_step_reg);
        od_dma_oddma_wr_num_bl_wrap_line_step.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_line_step_reg);
        od_dma_oddma_wr_num_bl_wrap_ctl.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_ctl_reg);
        od_dma_oddma_rd_num_bl_wrap_ctl.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_ctl_reg);
        od_dma_od_dma_ctrl.regValue = IoReg_Read32(OD_DMA_OD_DMA_CTRL_reg);
	 color_sld_control.regValue = IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg);
	 od_dma_oddma_wr_rule_check_up.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_Rule_check_up_reg);
	 od_dma_oddma_rd_rule_check_up.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_Rule_check_up_reg);
	 od_dma_oddma_pq_cmp.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_reg);
	od_dma_oddma_pq_decmp.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_DECMP_reg);
	od_od_ctrl.regValue = IoReg_Read32(OD_OD_CTRL_reg);



//
	IoReg_Write32(OD_DMA_OD_DMA_DB_EN_reg,0x00000000); //db disable
	IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_reg,0x003c0438); //pq_cmp return default disable
	IoReg_Write32(OD_DMA_ODDMA_PQ_DECMP_reg,0x003c0438); //pq_dcmp return default disable
	IoReg_Write32(OD_OD_CTRL_reg,0x00000330); //od_clk en
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_rolling_reg,0x00000439); //cap rolling disable
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_rolling_reg,0x00000439); //disp rolling disable

	IoReg_Write32(COLOR_SLD_SLD_SIZE_reg,0x0F000870); //scale down width ,0x 0x1e0, height ,0x 0x10e
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_line_step_reg,0x00000260); //cap line step
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_line_step_reg,0x00000260); //disp line step
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_word_reg,0x10000260); //[28]: addr_toggle_mode_en, [26:4]line_burst_num
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_word_reg,0x10000260); //[28]: addr_toggle_mode_en, [26:4]line_burst_num
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_ctl_reg,0x210e0260); //[31:29]: addr_toggle_offset, line_num, burst_length
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_ctl_reg,0x410e0260); //[31:29]: addr_toggle_offset, line_num, burst_length
	IoReg_Write32(OD_DMA_OD_DMA_CTRL_reg,0x00000000);

	IoReg_Write32(OD_DMA_ODDMA_WR_Ctrl_reg,0x00000002); //force_all rst
	IoReg_Write32(OD_DMA_ODDMA_RD_Ctrl_reg,0x00000002); //force_all rst
	IoReg_Write32(OD_DMA_ODDMA_WR_Ctrl_reg,0x00000001); //cap_en
	IoReg_Write32(OD_DMA_ODDMA_RD_Ctrl_reg,0x00000001); //disp_en
//


        //ODDMA double buffer apply
        od_dma_od_dma_db_en.regValue = IoReg_Read32(OD_DMA_OD_DMA_DB_EN_reg);
        od_dma_od_dma_db_en.disp_db_apply = 1;
        od_dma_od_dma_db_en.cap_db_apply= 1;
        IoReg_Write32(OD_DMA_OD_DMA_DB_EN_reg,od_dma_od_dma_ctrl.regValue);



        #ifdef SLD_Debug
        VIPprintf("drvif_color_set_SLD_4k_dma_setting\n");
        #endif



}
*/

#if 0//k7 delete
void drvif_color_set_SLD_Enable(unsigned char enable)
{
	color_sld_sld_control_RBUS color_sld_control;
	color_sld_control.regValue = IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg);

	if (drvif_color_get_HW_MEX_STATUS(MEX_SLD) != 1) {
		rtd_pr_vpq_info("HW_MEX,  MEX_SLD != 1, En\n");
		return;
	}

	color_sld_control.sld_en= enable;

	#ifdef SLD_Debug
	VIPprintf("sld_valid = %d\n",color_sld_control.sld_en);
	#endif

	IoReg_Write32(COLOR_SLD_SLD_CONTROL_reg, color_sld_control.regValue);
}
//juwen, 0603, done
void drvif_color_set_SLD_control(DRV_SLD_Ctrl *ptr)
{
#ifdef VPQ_COMPILER_ERROR_ENABLE_MERLIN6
	color_sld_sld_control_RBUS color_sld_control;
	color_sld_control.regValue = IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg);

	if (drvif_color_get_HW_MEX_STATUS(MEX_SLD) != 1) {
		rtd_pr_vpq_info("HW_MEX,  MEX_SLD != 1, CTRL\n");
		return;
	}

	color_sld_control.sld_en = ptr->sld_en;
	color_sld_control.sld_blend_en= ptr->sld_blend_en;
	color_sld_control.sld_global_drop= ptr->sld_global_drop;
	color_sld_control.sld_hpf_type= ptr->sld_hpf_type;
	color_sld_control.sld_hpf_thl= ptr->sld_hpf_thl;
	color_sld_control.sld_scale_mode= ptr->sld_scale_mode;

	IoReg_Write32(COLOR_SLD_SLD_CONTROL_reg, color_sld_control.regValue);
#endif
}

//juwen, 0603
void drvif_color_set_SLD_input_size(DRV_SLD_Size *ptr)
{

	color_sld_sld_size_RBUS color_sld_size;

	color_sld_size.regValue = IoReg_Read32(COLOR_SLD_SLD_SIZE_reg);

	color_sld_size.sld_height = ptr->sld_height;
	color_sld_size.sld_width = ptr->sld_width;

	IoReg_Write32(COLOR_SLD_SLD_SIZE_reg,color_sld_size.regValue);
}

//juwen, 0603
void drvif_color_set_SLD_curve_map(DRV_SLD_CurveMap *ptr)
{
#ifdef VPQ_COMPILER_ERROR_ENABLE_MERLIN6
	color_sld_sld_curve_map_seg_RBUS color_sld_curve_map_seg;
	color_sld_sld_curve_map_gain_RBUS color_sld_curve_map_gain;
	color_sld_sld_curve_map_offset_0_RBUS color_sld_curve_map_offset_0;
	color_sld_sld_curve_map_offset_1_RBUS color_sld_curve_map_offset_1;

	color_sld_curve_map_seg.regValue = IoReg_Read32(COLOR_SLD_SLD_CURVE_MAP_SEG_reg);
	color_sld_curve_map_gain.regValue =IoReg_Read32(COLOR_SLD_SLD_CURVE_MAP_GAIN_reg);
	color_sld_curve_map_offset_0.regValue =IoReg_Read32(COLOR_SLD_SLD_CURVE_MAP_OFFSET_0_reg);
	color_sld_curve_map_offset_1.regValue =IoReg_Read32(COLOR_SLD_SLD_CURVE_MAP_OFFSET_1_reg);

	color_sld_curve_map_seg.sld_curve_seg_0 = ptr->sld_curve_seg_0;
	color_sld_curve_map_seg.sld_curve_seg_1 =  ptr->sld_curve_seg_1;

	color_sld_curve_map_gain.sld_curve_gain_0 = 	ptr->sld_curve_gain_0;
	color_sld_curve_map_gain.sld_curve_gain_1 = 	ptr->sld_curve_gain_1;
	color_sld_curve_map_gain.sld_curve_gain_2 = 	ptr->sld_curve_gain_2;

	color_sld_curve_map_offset_0.sld_curve_offset_0 = ptr->sld_curve_offset_0;
	color_sld_curve_map_offset_0.sld_curve_offset_1 = ptr->sld_curve_offset_1;

	color_sld_curve_map_offset_1.sld_curve_offset_2 = ptr->sld_curve_offset_2;

	IoReg_Write32(COLOR_SLD_SLD_CURVE_MAP_SEG_reg, color_sld_curve_map_seg.regValue);
	IoReg_Write32(COLOR_SLD_SLD_CURVE_MAP_GAIN_reg, color_sld_curve_map_gain.regValue);
	IoReg_Write32(COLOR_SLD_SLD_CURVE_MAP_OFFSET_0_reg, color_sld_curve_map_offset_0.regValue);
	IoReg_Write32(COLOR_SLD_SLD_CURVE_MAP_OFFSET_1_reg, color_sld_curve_map_offset_1.regValue);
#endif
}

//juwen, 0603
void drvif_color_set_SLD_drop(DRV_SLD_DROP *ptr)
{

	color_sld_sld_drop_RBUS color_sld_drop;

	color_sld_drop.regValue = IoReg_Read32(COLOR_SLD_SLD_DROP_reg);

	color_sld_drop.sld_cnt_thl = ptr->sld_cnt_thl;
	color_sld_drop.sld_hpf_diff_thl = ptr->sld_hpf_diff_thl;
	color_sld_drop.sld_pixel_diff_thl = ptr->sld_pixel_diff_thl;

	IoReg_Write32(COLOR_SLD_SLD_DROP_reg, color_sld_drop.regValue);
}


void drvif_color_count_read_only(unsigned short sld_logo_cnt, unsigned short sld_drop_cnt)
{

	color_sld_sld_cnt_0_RBUS color_sld_read_only_logo_cnt;
	color_sld_sld_cnt_1_RBUS color_sld_read_only_drop_cnt;

	color_sld_read_only_logo_cnt.regValue = IoReg_Read32(COLOR_SLD_SLD_CNT_0_reg);
	color_sld_read_only_drop_cnt.regValue = IoReg_Read32(COLOR_SLD_SLD_CNT_1_reg);

}

void drvif_color_set_SLD_cnt_thl(unsigned int cnt_th)
{
	color_sld_sld_drop_RBUS color_sld_drop;

	color_sld_drop.regValue = IoReg_Read32(COLOR_SLD_SLD_DROP_reg);

	color_sld_drop.sld_cnt_thl = cnt_th;

	IoReg_Write32(COLOR_SLD_SLD_DROP_reg, color_sld_drop.regValue);

}
#endif
/*
void fwif_k6_sld(int Y1, int Y2, int Y3, int gain_low,unsigned char UIsel_L, unsigned int cnt_th)
{

	drvif_color_set_SLD_cnt_thl(cnt_th);
	drvif_color_set_SLD_4k_dma_setting();
 	SLD_k6_HAL(Y1,  Y2,  Y3,  gain_low, UIsel_L);


}
*/	
#endif


/* FOR K6LP SLD(MEMC+DEMURA) FUNCTIONS */
#if 1

#if 0
/* DUMPER RELATED FUNCTIONS */
#endif

char linedata[961] = {0};	/* 120 byte + '\n'*/
static struct file* file_open(const char* path, int flags, int rights) {
	struct file* filp = NULL;
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	mm_segment_t oldfs;
	#endif
	//int err = 0;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	oldfs = get_fs();
	set_fs(KERNEL_DS);
#endif
	filp = filp_open(path, flags, rights);
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	set_fs(oldfs);
	#endif
	if(IS_ERR(filp)) {
		//err = PTR_ERR(filp);
		return NULL;
	}
	return filp;
}

static void file_close(struct file* file) {
	filp_close(file, NULL);
}

static int file_write(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) {
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	mm_segment_t oldfs;
	#endif
	int ret;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	oldfs = get_fs();
	set_fs(KERNEL_DS);
#endif
	ret = kernel_write(file, data, size, &offset);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	set_fs(oldfs);
#endif
	return ret;
}

static int file_sync(struct file* file) {
	vfs_fsync(file, 0);
	return 0;
}

#if 0
/* GLOBAL PARAMETERS */
#endif

unsigned int LD_size = 0;
unsigned char * LD_virAddr = NULL;
sld_work_struct sld_work;
unsigned char logo_flag_map_raw[192*540] = {0};
unsigned char logo_flag_map[481*271] = {0};
unsigned char logo_flag_map_buf[481*271] = {0};
short logo_demura_counter[481*271] = {0};
short demura_tbl[5][481*271];
extern VIP_DeMura_TBL DeMura_TBL;
extern DRV_DeMura_CTRL_TBL DeMura_CTRL_TBL_SW_SLD;
unsigned char is_background[10000] = {0};
unsigned short eq_table[10000][2] = {{0}};
unsigned char eq_searched[10000] = {0};
unsigned char SLD_SW_En = 0;
unsigned char MEMC_Pixel_LOGO_For_SW_SLD = 0;
unsigned char bPictureEnabled = 0;
unsigned char LSC_by_memc_logo_en = 1;
int memc_logo_to_demura_wait_time = 5; // test
int memc_logo_to_demura_drop_time = 10; // test
int memc_logo_to_demura_drop_limit = 128; // test
int memc_logo_to_demura_drop_limit_gain = 1736;	// 10 bit gain														  
int memc_logo_to_demura_ascend_speed = 20;
int memc_logo_to_demura_APL_max_th = 1040;
unsigned char circle4_flag = 0;
unsigned int memc_logo_to_demura_update_cnt = 4;
unsigned char memc_logo_read_en = 1;
unsigned char logo_to_demura_en = 1;
unsigned char memc_logo_fill_hole_en = 1;
unsigned char memc_logo_filter_en = 1;
unsigned char demura_write_en = false;
unsigned char SLD_BY_MEMC_DEMURA_LOGO_ONLY = 0;
unsigned char sld_ddr_offset_auto_get = 1;
unsigned int sld_ddr_offset = 0;

extern unsigned int blk_apl_average[ 32*18 ];
unsigned int blk_apl_maxfilter[32*18];
unsigned int blk_apl_interp[481*271];

const char DecDig[10] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

extern UINT8 v4l2_vpq_ire;

#if 0
/* DUMP FUNCTIONS */
#endif

void short_to_char(short input, char *rs)
{
	// convert a 16bits(-32768~32767) digit to char array with lenght 6
	int data = input;
	rs[0] = (data>=0)? ' ' : '-';
	if( data < 0 )
		data = -data;
	rs[1] = DecDig[(data/10000)%10];
	rs[2] = DecDig[(data/1000 )%10];
	rs[3] = DecDig[(data/100  )%10];
	rs[4] = DecDig[(data/10   )%10];
	rs[5] = DecDig[(data/1    )%10];
}

void bit_to_char(char org_byte, char *rs)
{
	UINT8 i;
	for (i = 0; i < 8; i++)
		*(rs + i) = (org_byte & (1 << (7 - i))) ? '1' : '0';
}

void dump_sld_calc_buffer_to_file(void)
{
	struct file* filp = NULL;
	unsigned long outfileOffset = 0;
	char filename[100];
	char rs[6]; // for short to digit
	char rs2[8]; // for char bitmap to bit
	static unsigned int fc = 0;
	unsigned int lineCnt = 0, dataCnt = 0;;

	fc++;

#if 1
	// dump logo buffer : logo_flag_map_raw
	sprintf(filename, "/tmp/usb/sda/sda1/LD/logo_flag_map_raw_%04d.txt", fc);
	filp = file_open(filename, O_RDWR | O_CREAT | O_APPEND, 0);
	if (filp == NULL)
	{
		rtd_pr_vpq_info("[%s] open fail\n", filename);
		return;
	}

	for(lineCnt=0; lineCnt<540; lineCnt++)
	{
		for(dataCnt=0; dataCnt<192; dataCnt++)
		{
			bit_to_char(logo_flag_map_raw[lineCnt*192+dataCnt], rs2);
			file_write(filp, outfileOffset, (unsigned char*)rs2, 8);
			outfileOffset = outfileOffset + 8;
		}
		file_write(filp, outfileOffset, (unsigned char*)"\n", 1);
		outfileOffset = outfileOffset + 1;
	}
	file_sync(filp);
	file_close(filp);
	msleep(10);
#endif

#if 1
	// dump logo buffer : logo_flag_map
	sprintf(filename, "/tmp/usb/sda/sda1/LD/logo_flag_map_%04d.txt", fc);
	filp = file_open(filename, O_RDWR | O_CREAT | O_APPEND, 0);
	if (filp == NULL)
	{
		rtd_pr_vpq_info("[%s] open fail\n", filename);
		return;
	}

	for(lineCnt=0; lineCnt<271; lineCnt++)
	{
		for(dataCnt=0; dataCnt<481; dataCnt++)
		{
			if( logo_flag_map[lineCnt*481+dataCnt]!=0 )
				file_write(filp, outfileOffset, (unsigned char*)"1", 1);
			else
				file_write(filp, outfileOffset, (unsigned char*)"0", 1);
			outfileOffset = outfileOffset + 1;
		}
		file_write(filp, outfileOffset, (unsigned char*)"\n", 1);
		outfileOffset = outfileOffset + 1;
	}
	file_sync(filp);
	file_close(filp);
	msleep(10);
#endif

#if 1
	// dump logo buffer : logo_flag_map_buf
	sprintf(filename, "/tmp/usb/sda/sda1/LD/logo_flag_map_buf_%04d.txt", fc);
	filp = file_open(filename, O_RDWR | O_CREAT | O_APPEND, 0);
	if (filp == NULL)
	{
		rtd_pr_vpq_info("[%s] open fail\n", filename);
		return;
	}

	for(lineCnt=0; lineCnt<271; lineCnt++)
	{
		for(dataCnt=0; dataCnt<481; dataCnt++)
		{
			if( logo_flag_map_buf[lineCnt*481+dataCnt]!=0 )
				file_write(filp, outfileOffset, (unsigned char*)"1", 1);
			else
				file_write(filp, outfileOffset, (unsigned char*)"0", 1);
			outfileOffset = outfileOffset + 1;
		}
		file_write(filp, outfileOffset, (unsigned char*)"\n", 1);
		outfileOffset = outfileOffset + 1;
	}
	file_sync(filp);
	file_close(filp);
	msleep(10);
#endif

#if 1
	// dump counter buffer : logo_demura_counter
	sprintf(filename, "/tmp/usb/sda/sda1/LD/logo_demura_counter_%04d.txt", fc);
	filp = file_open(filename, O_RDWR | O_CREAT | O_APPEND, 0);
	if (filp == NULL)
	{
		rtd_pr_vpq_info("[%s] open fail\n", filename);
		return;
	}

	for(lineCnt=0; lineCnt<271; lineCnt++)
	{
		for(dataCnt=0; dataCnt<481; dataCnt++)
		{
			short_to_char(logo_demura_counter[lineCnt*481+dataCnt], rs);
			file_write(filp, outfileOffset, (unsigned char*)rs, 6);
			outfileOffset = outfileOffset + 6;
			file_write(filp, outfileOffset, (unsigned char*)" ", 1);
			outfileOffset = outfileOffset + 1;
		}
		file_write(filp, outfileOffset, (unsigned char*)"\n", 1);
		outfileOffset = outfileOffset + 1;
	}
	file_sync(filp);
	file_close(filp);
	msleep(10);
#endif

#if 1
	// dump demura tbl buffer : demura_tbl_lo/md/hi
	sprintf(filename, "/tmp/usb/sda/sda1/LD/sld_demura_tbl_lo_%04d.txt", fc);
	filp = file_open(filename, O_RDWR | O_CREAT | O_APPEND, 0);
	if (filp == NULL)
	{
		rtd_pr_vpq_info("[%s] open fail\n", filename);
		return;
	}

	for(lineCnt=0; lineCnt<271; lineCnt++)
	{
		for(dataCnt=0; dataCnt<481; dataCnt++)
		{
			short_to_char(demura_tbl[0][lineCnt*481+dataCnt], rs);
			file_write(filp, outfileOffset, (unsigned char*)rs, 6);
			outfileOffset = outfileOffset + 6;
			file_write(filp, outfileOffset, (unsigned char*)" ", 1);
			outfileOffset = outfileOffset + 1;
		}
		file_write(filp, outfileOffset, (unsigned char*)"\n", 1);
		outfileOffset = outfileOffset + 1;
	}
	file_sync(filp);
	file_close(filp);
	msleep(10);

	sprintf(filename, "/tmp/usb/sda/sda1/LD/sld_demura_tbl_md_%04d.txt", fc);
	filp = file_open(filename, O_RDWR | O_CREAT | O_APPEND, 0);
	if (filp == NULL)
	{
		rtd_pr_vpq_info("[%s] open fail\n", filename);
		return;
	}

	for(lineCnt=0; lineCnt<271; lineCnt++)
	{
		for(dataCnt=0; dataCnt<481; dataCnt++)
		{
			short_to_char(demura_tbl[1][lineCnt*481+dataCnt], rs);
			file_write(filp, outfileOffset, (unsigned char*)rs, 6);
			outfileOffset = outfileOffset + 6;
			file_write(filp, outfileOffset, (unsigned char*)" ", 1);
			outfileOffset = outfileOffset + 1;
		}
		file_write(filp, outfileOffset, (unsigned char*)"\n", 1);
		outfileOffset = outfileOffset + 1;
	}
	file_sync(filp);
	file_close(filp);
	msleep(10);

	sprintf(filename, "/tmp/usb/sda/sda1/LD/sld_demura_tbl_hi_%04d.txt", fc);
	filp = file_open(filename, O_RDWR | O_CREAT | O_APPEND, 0);
	if (filp == NULL)
	{
		rtd_pr_vpq_info("[%s] open fail\n", filename);
		return;
	}

	for(lineCnt=0; lineCnt<271; lineCnt++)
	{
		for(dataCnt=0; dataCnt<481; dataCnt++)
		{
			short_to_char(demura_tbl[2][lineCnt*481+dataCnt], rs);
			file_write(filp, outfileOffset, (unsigned char*)rs, 6);
			outfileOffset = outfileOffset + 6;
			file_write(filp, outfileOffset, (unsigned char*)" ", 1);
			outfileOffset = outfileOffset + 1;
		}
		file_write(filp, outfileOffset, (unsigned char*)"\n", 1);
		outfileOffset = outfileOffset + 1;
	}
	file_sync(filp);
	file_close(filp);
	msleep(10);
#endif

}

void dump_logo_detect_to_file(unsigned char start_byte)
{
	struct file* filp = NULL;
	unsigned long outfileOffset = 0;
	unsigned long ddr_offset = start_byte;	/* de-garbage */
	char filename[100];
	static UINT32 fc = 0;
	extern INT32 filename_memc;
	extern INT32 start_offset_byte;
	unsigned int file_line_count = 0;

	linedata[960] = '\n';
	fc++;
	sprintf(filename, "/tmp/usb/sda/sda1/LD/Logo_detect_%04d_%d.txt", fc, filename_memc);

	if (LD_virAddr != NULL) {
		filp = file_open(filename, O_RDWR | O_CREAT | O_APPEND, 0);
		if (filp == NULL) {
			rtd_pr_vpq_info("(%d)open fail\n", __LINE__);
			return;
		}

		LD_virAddr = LD_virAddr+start_offset_byte;

		while(outfileOffset < (LD_size - 1920/* 1920: 10 line */)){
			unsigned int i = 0;
			while(i < 960){
				char rs[8];
				unsigned char j;
				bit_to_char(*(LD_virAddr + ddr_offset), rs);
				for (j = 0; j < 8; j++)
					linedata[i + j] = rs[j];
				ddr_offset++;
				i = i + 8;
			}
			ddr_offset = ddr_offset + (192 - 120); /* garbage */
			if(file_line_count <540){
				file_write(filp, (outfileOffset / 192) * 961 + 1, (unsigned char*)linedata, 961);
				file_line_count ++;
			}
			outfileOffset = outfileOffset + 192;/*192 byte per line*/
		}

		//file_write(filp, outfileOffset, (unsigned char*)LD_virAddr, LD_size);
		file_sync(filp);
		file_close(filp);
	} else {
		rtd_pr_vpq_info("dump fail\n");
	}

	return;
}

#if 0
/* MAIN FUNCTIONS */
#endif

extern UINT32 MEMC_Lib_GetInfoForSLD(unsigned char infoSel);



void fwif_color_SLD_apply_normal_setting(void)
{
	DRV_SLD_sld_acc_0 sld_acc_0_data;
	DRV_SLD_sld_hpf_1 sld_hpf_1;
	DRV_SLD_sld_drop_slow_0 sld_drop_slow_0;
	DRV_SLD_sld_control sld_control;
	DRV_SLD_sld_temporal_noise_1 sld_temporal_noise_1;
	DRV_SLD_sld_in_logo_thl_0 sld_in_logo_thl_0;
	DRV_SLD_sld_noise_pattern_0 sld_noise_pattern_0;

	DRV_SLD_sld_temporal_idx_compensate sld_temporal_idx_compensate;
	DRV_SLD_sld_idx_compensate_by_neighbor_0 sld_idx_compensate_by_neighbor_0;
	DRV_SLD_sld_drop sld_drop;
	DRV_SLD_sld_idx_not_drop_0 sld_idx_not_drop_0;
	DRV_SLD_sld_neighbor_not_drop sld_neighbor_not_drop;
	DRV_SLD_sld_neighbor_drop_0 sld_neighbor_drop_0;
	DRV_SLD_sld_idx_protect_by_neighbor_0 sld_idx_protect_by_neighbor_0;
	
	DRV_SLD_sld_fw_control_0 sld_fw_control_0;

	DRV_SLD_sld_lpf_0 sld_lpf_0_data;

	DRV_SLD_sld_histogram_1  sld_histogram_1;
	DRV_SLD_sld_histogram_0  sld_histogram_0;

	DRV_SLD_sld_apply_window_0 sld_apply_window_0;

	DRV_SLD_sld_idx_map_seg sld_idx_map_seg;
	DRV_SLD_sld_Y_curve_map sld_Y_curve_map;
	
//global control
	sld_control.sld_hpf_type = 0;
	sld_control.sld_add_comp_dy = 1;
	sld_control.sld_scale_mode = 1;
	sld_control.sld_disable_lpf = 0;
	sld_control.sld_disable_scale_up = 0;
	sld_control.sld_idx_count_period = 0;
	sld_control.sld_idx_input_sel = 0;
	sld_control.sld_window_en = 0;
	sld_control.sld_global_drop = 0;
	sld_control.sld_blend_en=1;
	sld_control.sld_debug_mode = 0;

	drvif_color_set_SLD_control((DRV_SLD_sld_control*) &sld_control);
/*add---------------------------------------------------------------------------------------*/
//add condition
	sld_acc_0_data.sld_idx_acc_condition_and = 1;
	sld_acc_0_data.sld_hpf_logo_add_thl = 128;
	sld_acc_0_data.sld_frame_diffy_thl = 256;
	sld_acc_0_data.sld_y_idx_thl = 256;
	sld_acc_0_data.sld_max_idx_thl = 4096;

	drvif_color_set_SLD_Idx_add_con((DRV_SLD_sld_acc_0*) &sld_acc_0_data);

//add step control
	sld_hpf_1.sld_idx_sign_0 = 0;
	sld_hpf_1.sld_idx_step_0 = 1;
	sld_hpf_1.sld_idx_sign_2 = 1;
	sld_hpf_1.sld_idx_step_2 = 15;
	
	drvif_color_set_SLD_step((DRV_SLD_sld_hpf_1*) &sld_hpf_1);

//control minus step
	sld_drop_slow_0.idx_slow_down_l      = 2700;
	sld_drop_slow_0.idx_slow_down_h     = 3000;
	sld_drop_slow_0.idx_slow_down_step = 64;

	drvif_color_set_SLD_control_minus_step((DRV_SLD_sld_drop_slow_0*) &sld_drop_slow_0);

//temporal noise
	sld_temporal_noise_1.big_temporal_diff_up = 400;
	sld_temporal_noise_1.big_temporal_diff_low = 0;
	sld_temporal_noise_1.temporal_counter_thl = 129600;//disable 
	sld_temporal_noise_1.temporal_diff = 400;

	drvif_color_set_SLD_sld_temporal_noise((DRV_SLD_sld_temporal_noise_1*) &sld_temporal_noise_1);

//8 directions 12 tab
	sld_in_logo_thl_0.sld_inter_thl_0 = 128;
	sld_in_logo_thl_0.sld_inter_thl_1 = 128;
	sld_in_logo_thl_0.sld_inter_thl_2 = 128;
	sld_in_logo_thl_0.sld_inter_thl_3 = 128;
	sld_in_logo_thl_0.sld_inter_thl_4 = 128;
	sld_in_logo_thl_0.sld_inter_thl_5 = 128;
	sld_in_logo_thl_0.sld_inter_thl_6 = 128;
	sld_in_logo_thl_0.sld_inter_thl_7 = 128;
	sld_in_logo_thl_0.sld_inter_thl_8 = 4095;
	sld_in_logo_thl_0.sld_inter_thl_9 = 4095;
	sld_in_logo_thl_0.sld_inter_thl_10 = 4095;
	sld_in_logo_thl_0.sld_inter_thl_11 = 4095;

	drvif_color_set_SLD_8dir_12tab((DRV_SLD_sld_in_logo_thl_0*) &sld_in_logo_thl_0);

//T noise settings
	sld_noise_pattern_0.check_drop_pattern_start = 4096;
	sld_noise_pattern_0.detect_tnoise_en = 0;
	sld_noise_pattern_0.diffavg = 160;
	sld_noise_pattern_0.grouph_diffmaxmin = 1;
	sld_noise_pattern_0.patternedge_hpf = 12 ;
	sld_noise_pattern_0.patternedge_y = 600;
	sld_noise_pattern_0.patternregion_hpf = 208;
	sld_noise_pattern_0.patternregion_y = 600;
	
	 drvif_color_set_SLD_LGD_Tnoise_pattern((DRV_SLD_sld_noise_pattern_0*) &sld_noise_pattern_0);

/*compensate---------------------------------------------------------------------------------------*/
//temporal compensate
	sld_temporal_idx_compensate.do_connect_seg_idx_lower_bound = 10;
	sld_temporal_idx_compensate.idx_period_check_once = 63;

	drvif_color_set_SLD_compensate_temporal((DRV_SLD_sld_temporal_idx_compensate*) &sld_temporal_idx_compensate);

//spatial compensate
	sld_idx_compensate_by_neighbor_0.idx_compensate_start_thl = 3000;
	sld_idx_compensate_by_neighbor_0.compensate_neighbor_tolerance = 3000;
	sld_idx_compensate_by_neighbor_0.idx_compensate_neighbor_counter_thl = 5;
	
	drvif_color_set_SLD_compensate_spatial((DRV_SLD_sld_idx_compensate_by_neighbor_0*) &sld_idx_compensate_by_neighbor_0);

/*drop---------------------------------------------------------------------------------------*/
	sld_drop.sld_cnt_thl = 0;
	
	sld_drop.sld_hpf_diff_thl = 128;
	sld_drop.sld_pixel_diff_thl = 96;

	sld_idx_not_drop_0.in_logo_idx_thl = 1023; 
	sld_idx_not_drop_0.hpf_diff_thl_for_logo_region = 512;
	sld_idx_not_drop_0.pixel_diff_thl_for_logo_region = 128;

	sld_neighbor_not_drop.neighbor_diffy_notdrop_thl = 0;

	sld_neighbor_drop_0.neighbor0_diffy_drop_thl = 0;
	sld_neighbor_drop_0.neighbor1_diffy_drop_thl = 1023;
	sld_neighbor_drop_0.neighbor2_diffy_drop_thl = 1023;

	sld_idx_protect_by_neighbor_0.idx_drop_protect_idx_thl = 1023;
	sld_idx_protect_by_neighbor_0.idx_drop_protect_idx_neighbor_tolerance = 1023;
	sld_idx_protect_by_neighbor_0.idx_drop_protect_counter_neighbor_sameidx = 9;

	 drvif_color_set_SLD_drop((DRV_SLD_sld_drop*) &sld_drop,(DRV_SLD_sld_idx_not_drop_0*) &sld_idx_not_drop_0, (DRV_SLD_sld_neighbor_not_drop*) &sld_neighbor_not_drop,(DRV_SLD_sld_neighbor_drop_0*) &sld_neighbor_drop_0 , (DRV_SLD_sld_idx_protect_by_neighbor_0*) &sld_idx_protect_by_neighbor_0);


/*FW control setting---------------------------------------------------------------------------------------*/

	sld_fw_control_0.fw_idx_ctrl_en = 0;
	sld_fw_control_0.fw_idx_ctrl_step = 1023;
	sld_fw_control_0.fw_idx_freeze_for_full_frame = 0;
	sld_fw_control_0.fw_pixel_gain = 4096;

	drvif_color_set_SLD_fw_control((DRV_SLD_sld_fw_control_0*) &sld_fw_control_0);

/*LPF*/

	sld_lpf_0_data.coef_0_0=1;
	sld_lpf_0_data.coef_0_1=4;
	sld_lpf_0_data.coef_0_2=6;
	sld_lpf_0_data.coef_0_3=4;
	sld_lpf_0_data.coef_0_4=1;
	
	sld_lpf_0_data.coef_1_0=4;
	sld_lpf_0_data.coef_1_1=16;
	sld_lpf_0_data.coef_1_2=24;
	sld_lpf_0_data.coef_1_3=16;
	sld_lpf_0_data.coef_1_4=4;
	
	sld_lpf_0_data.coef_2_0=6;
	sld_lpf_0_data.coef_2_1=24;
	sld_lpf_0_data.coef_2_2=36;
	sld_lpf_0_data.coef_2_3=24;
	sld_lpf_0_data.coef_2_4=6;
	
	sld_lpf_0_data.coef_3_0=4;
	sld_lpf_0_data.coef_3_1=16;
	sld_lpf_0_data.coef_3_2=24;
	sld_lpf_0_data.coef_3_3=16;
	sld_lpf_0_data.coef_3_4=4;
	
	sld_lpf_0_data.coef_4_0=1;
	sld_lpf_0_data.coef_4_1=4;
	sld_lpf_0_data.coef_4_2=6;
	sld_lpf_0_data.coef_4_3=4;
	sld_lpf_0_data.coef_4_4=1;
	
	sld_lpf_0_data.lpf_shift_bit=8;
 	drvif_color_set_SLD_lpf((DRV_SLD_sld_lpf_0*) &sld_lpf_0_data);	

/*hist---------------------------------------------------------------------------------------*/

	sld_histogram_0.hist_idx_thl = 0;
	sld_histogram_0.hist_shift_bit = 5;
	
	sld_histogram_1.y_lower_bound_hist_0 = 0;
	sld_histogram_1.y_lower_bound_hist_1 = 128;
	sld_histogram_1.y_lower_bound_hist_2 = 256;
	sld_histogram_1.y_lower_bound_hist_3 = 384;
	sld_histogram_1.y_lower_bound_hist_4 = 512;
	sld_histogram_1.y_lower_bound_hist_5 = 640;
	sld_histogram_1.y_lower_bound_hist_6 = 768;
	sld_histogram_1.y_lower_bound_hist_7 = 896;
	
	 drvif_color_set_SLD_sld_histogram((DRV_SLD_sld_histogram_1*) &sld_histogram_1, (DRV_SLD_sld_histogram_0*) &sld_histogram_0);
	
/*window---------------------------------------------------------------------------------------*/
	sld_apply_window_0.sld_window_apply_out = 0;
	sld_apply_window_0.sld_window_pos0_x = 0;
	sld_apply_window_0.sld_window_pos0_y = 0;
	sld_apply_window_0.sld_window_pos1_x = 3839;
	sld_apply_window_0.sld_window_pos1_y = 2159;

	drvif_color_set_SLD_sld_window((DRV_SLD_sld_apply_window_0*) &sld_apply_window_0);

/*idx curve---------------------------------------------------------------------------------------*/

	sld_idx_map_seg.sld_idx_seg_0 = 512;
	sld_idx_map_seg.sld_idx_seg_1 = 1024;
	sld_idx_map_seg.sld_idx_seg_2 = 3000;
	sld_idx_map_seg.sld_new_idx_gain_0 = 0 ;
	sld_idx_map_seg.sld_new_idx_gain_1 = 0;
	sld_idx_map_seg.sld_new_idx_gain_2 = 97;
	sld_idx_map_seg.sld_new_idx_gain_3 = 1;
	sld_idx_map_seg.sld_new_idx_offset_0 = 0;
	sld_idx_map_seg.sld_new_idx_offset_1 = 0;
	sld_idx_map_seg.sld_new_idx_offset_2 = 63982;
	sld_idx_map_seg.sld_new_idx_offset_3 = 2916;

 	drvif_color_set_SLD_Idx_curve_map((DRV_SLD_sld_idx_map_seg*) &sld_idx_map_seg);


/*Y curve---------------------------------------------------------------------------------------*/
	sld_Y_curve_map.sld_curve_seg_0 = 512;
	sld_Y_curve_map.sld_curve_seg_1 = 768;
	sld_Y_curve_map.sld_curve_seg_2 = 800;

	sld_Y_curve_map.sld_new_curve_gain_0 =64;
	sld_Y_curve_map.sld_new_curve_gain_1 =64;
	sld_Y_curve_map.sld_new_curve_gain_2 =64;
	sld_Y_curve_map.sld_new_curve_gain_3 =34;

	sld_Y_curve_map.sld_new_curve_offset_0 = 0;
	sld_Y_curve_map.sld_new_curve_offset_1 = 0;
	sld_Y_curve_map.sld_new_curve_offset_2 = 0;
	sld_Y_curve_map.sld_new_curve_offset_3 = 366;

	drvif_color_set_SLD_Y_curve_map((DRV_SLD_sld_Y_curve_map*) &sld_Y_curve_map);

}

void drif_color_SLD_apply_normal_setting(void)
{
	fwif_color_SLD_apply_normal_setting();
}

#if 0
void memc_information_for_SLD(void)
{
	static unsigned int frame_cnt = 0;
	unsigned char* logo_ptr = LD_virAddr;

	static unsigned char memc_sc_flag = false;
	static unsigned int  memc_sc_motion1 = 0, memc_sc_motion2 = 0;

	// LD block average
	//blu_ld_global_ctrl2_RBUS ld_global_ctrl2_reg;
	//unsigned char LD_valid, LD_type, LD_Hnum, LD_Vnum;

	// OSD mixer detect
	static unsigned char osd_measure_init = false;
	osdovl_mixer_ctrl2_RBUS osdovl_mixer_ctrl2_reg;
	osdovl_osd_db_ctrl_RBUS osdovl_osd_db_ctrl_reg;
	osdovl_measure_osd1_sta_RBUS osdovl_measure_osd1_sta_reg;
	osdovl_measure_osd1_end_RBUS osdovl_measure_osd1_end_reg;
	short osd_sta_x, osd_sta_y, osd_end_x, osd_end_y;

	//Merlin6, SLD
	unsigned int counter_blk_pre_cur_same = 0;
	//int thl_temporal_dY = 20;
	//int thl_counter_NEWS_case = 240;//tolerance num of frame
	color_sld_sld_fw_control_0_RBUS	 fw_control_0;
	

        /*memc*/
       rtd_setbits(0xb809d5fc, 0x10000000);//memc
	//WriteRegister(0x000040A0, 0, 31, 0x0);//memc
	//WriteRegister(0x000040A4, 0, 31, 0x0);//memc
		

	// dynamic calculate the offset
	if( sld_ddr_offset_auto_get == 1 )
	{
		sld_ddr_offset = IoReg_Read32(KME_DM_TOP1_KME_DM_TOP1_48_reg)&0xfff;
	}

	logo_ptr = LD_virAddr + sld_ddr_offset;

	// start to work
	if( !osd_measure_init )
	{
		osdovl_mixer_ctrl2_reg.regValue = IoReg_Read32(OSDOVL_Mixer_CTRL2_reg);
		osdovl_osd_db_ctrl_reg.regValue = IoReg_Read32(OSDOVL_OSD_Db_Ctrl_reg);

		osdovl_mixer_ctrl2_reg.measure_osd_zone_en = 1;
		osdovl_mixer_ctrl2_reg.measure_osd_zone_type = 1;
		IoReg_Write32( OSDOVL_Mixer_CTRL2_reg, osdovl_mixer_ctrl2_reg.regValue );

		osdovl_osd_db_ctrl_reg.db_load = 1;
		IoReg_Write32( OSDOVL_OSD_Db_Ctrl_reg, osdovl_osd_db_ctrl_reg.regValue );

		osd_measure_init = true;
	}

	osdovl_measure_osd1_sta_reg.regValue = IoReg_Read32(OSDOVL_measure_osd1_sta_reg);
	osdovl_measure_osd1_end_reg.regValue = IoReg_Read32(OSDOVL_measure_osd1_end_reg);
	osd_sta_x = osdovl_measure_osd1_sta_reg.x;
	osd_sta_y = osdovl_measure_osd1_sta_reg.y;
	osd_end_x = osdovl_measure_osd1_end_reg.x;
	osd_end_y = osdovl_measure_osd1_end_reg.y;

	if( osd_sta_x == 0x1fff && osd_sta_y == 0x1fff && osd_end_x == 0 && osd_end_y == 0 ) // no OSD shown
	{
		sld_work.osd_sta_blkx = -1;
		sld_work.osd_sta_blky = -1;
		sld_work.osd_end_blkx = -1;
		sld_work.osd_end_blky = -1;
	}
	else // mark osd blocks
	{
		sld_work.osd_sta_blkx = osd_sta_x / 8;
		sld_work.osd_end_blkx = (osd_end_x+7) / 8;
		sld_work.osd_sta_blky = osd_sta_y / 8;
		sld_work.osd_end_blky = (osd_end_y+7) / 8;
	}

	// motion status (read every frame)
	if( MEMC_Lib_GetInfoForSLD(1) ) // scene change flag
	{
		memc_sc_flag= true;
	}
	if( MEMC_Lib_GetInfoForSLD(4) > memc_sc_motion1 )
		memc_sc_motion1 = MEMC_Lib_GetInfoForSLD(4);
	if( MEMC_Lib_GetInfoForSLD(5) > memc_sc_motion2 )
		memc_sc_motion2 = MEMC_Lib_GetInfoForSLD(5);

	if( memc_logo_to_demura_update_cnt == 0 )
		memc_logo_to_demura_update_cnt = 1;
	if( frame_cnt % memc_logo_to_demura_update_cnt == 0 ) // read once every update_cnt frames
	{
	
		if( memc_sc_motion1 < 100 && memc_sc_motion2 < 100 ) // motion cnt low: picture pause
		{
			sld_condition.still_condition= true;
		}
		else if( memc_sc_flag && memc_sc_motion1 > 10000 && memc_sc_motion2 > 10000 ) // scene change flag + big motion: scene change
		{
			sld_condition.still_condition = false;
		}

		memc_sc_flag = false;
		memc_sc_motion1 = 0;
		memc_sc_motion2 = 0;

		// LC related info
		//get apl
		drvif_color_get_LC_APL_SLD_use(&(APL_cur[0]));

		counter_blk_pre_cur_same=0;
		LC_compute_pre_cur_deltaY(&(APL_pre[0]), &(APL_cur[0]),&(APL_delta[0]),&counter_blk_pre_cur_same,  thl_temporal_dY);
		//get sld fw information
		drvif_color_get_SLD_fw_read_only((DRV_SLD_sld_fw_read_only*) &fw_read_only_data);

		if(
			(sld_work.osd_sta_blkx > 0)
			&&(sld_work.osd_end_blkx > 0)
			&&(sld_work.osd_sta_blky > 0)
			&&(sld_work.osd_end_blky > 0)
		){//OSD condition
				rtd_pr_vpq_info("OSD condition--------------------\n");
				
				//blend_en = 0

				//fw mode : delete idx

				
		}			
		else if(sld_condition.still_condition){//apply
				rtd_pr_vpq_info("still condition--------------------\n");
				
				//fw control step until 0
				fw_control_0.regValue  		 = IoReg_Read32(COLOR_SLD_SLD_FW_control_0_reg 			   );
				fw_control_0.fw_idx_ctrl_step = 8192-4;//S(13,0)
				fw_control_0.fw_idx_ctrl_en   = 1;
				IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg , fw_control_0.regValue);
				

		}
		else if(//news condition 
			((fw_read_only_data.hpf_line_count > 3) && (fw_read_only_data.hpf_line_count < 10) && LC_detect_NEWS(&(APL_cur[0])))
			||(((frame_cnt - index_NEWS_case) < frame_cnt) && (index_NEWS_case != 0))
		)
		{

			if(((fw_read_only_data.hpf_line_count > 3) && (fw_read_only_data.hpf_line_count < 10) && LC_detect_NEWS(&(APL_cur[0])))){
				rtd_pr_vpq_info("NEWS condition--------------------\n");
				
				
				index_NEWS_case = frame_cnt;
			}else{
				rtd_pr_vpq_info("NEWS,in time condition----------------\n");
				
			}

			counter_NEWS_case = counter_NEWS_case+1;

			if(counter_NEWS_case > thl_counter_NEWS_case){//apply
				fwif_color_apply_NEWS_setting();
				fwif_color_SLD_disable_window();
				
			}
			
		}
		else{
			rtd_pr_vpq_info("normal condition----------------\n");

			sld_condition.normal_condition = 1;
			fwif_color_SLD_apply_normal_setting();
			
		}


	}

	frame_cnt++;

}
#endif

void fwif_color_compute_LC_APL_related_K7_FW(void)
{
	unsigned short idx_r = 0, idx_c = 0, idx_arr = 0, counter_UR = 0;
	int num_hblk = 0, num_vblk = 0;
	unsigned int counter_highY_blk = 0, counter_blk_pre_cur_same = 0;
	unsigned int thl_highY_frame = 750;
	color_sld_sld_fw_control_1_RBUS	 fw_control_1;
	DRV_SLD_sld_fw_read_only fw_read_only_data;
	color_sld_sld_fw_control_0_RBUS	 fw_control_0;
	lc_lc_global_ctrl2_RBUS blu_lc_global_ctrl2;
	static unsigned int  memc_sc_motion1 = 0, memc_sc_motion2 = 0;
	static unsigned char memc_sc_flag = false;

#ifdef CONFIG_MEMC_NOTSUPPORT // for code size reduce, when MEMC_BYPASS
			
#else
	if( MEMC_Lib_GetInfoForSLD(1) ) // scene change flag
	{
		memc_sc_flag= true;
	}
	if( MEMC_Lib_GetInfoForSLD(4) > memc_sc_motion1 )
		memc_sc_motion1 = MEMC_Lib_GetInfoForSLD(4);
	if( MEMC_Lib_GetInfoForSLD(5) > memc_sc_motion2 )
		memc_sc_motion2 = MEMC_Lib_GetInfoForSLD(5);
#endif

	if( memc_sc_motion1 < 100 && memc_sc_motion2 < 100 ) // motion cnt low: picture pause
	{
		sld_condition.still_condition= true;
	}
	else if( memc_sc_flag && memc_sc_motion1 > 10000 && memc_sc_motion2 > 10000 ) // scene change flag + big motion: scene change
	{
		sld_condition.still_condition = false;
	}

		
	blu_lc_global_ctrl2.regValue = IoReg_Read32(LC_LC_Global_Ctrl2_reg);

	num_hblk = blu_lc_global_ctrl2.lc_blk_hnum + 1 ;
	num_vblk = blu_lc_global_ctrl2.lc_blk_vnum + 1;

	for(idx_r=0;idx_r<(num_vblk);idx_r++){
		for(idx_c=0;idx_c <(num_hblk);idx_c++){
			APL_pre[idx_arr] = APL_cur[idx_arr];
			idx_arr = idx_arr + 1;
		}
	}

	LC_time = LC_time + 1;

	fw_control_1.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_1_reg);

#ifdef SLD_MEMC_DISPLAY
        /*memc*/
       //rtd_setbits(0xb809d5fc, 0x10000000);//memc
#endif

	if(lock_rgb_box_fw_step > 0){
		lock_rgb_box_fw_step = lock_rgb_box_fw_step - 1;
		rtd_pr_vpq_info("#############################lock_rgb_box_fw_step = %d, ",lock_rgb_box_fw_step);
		drvif_solor_set_SLD_apply_leave_RGB_box_logo(cmd_rgb_box_leave_idx_delete_step);
		
	}
	else if(lock_fw_gain > 0){
		lock_fw_gain = lock_fw_gain - 1;
		fw_control_1.fw_pixel_gain	            = CLAMP_SLD(fw_control_1.fw_pixel_gain - (4096/cmd_pixel_gain_step),4095,0);
		rtd_outl(COLOR_SLD_SLD_FW_control_1_reg , fw_control_1.regValue);	
		rtd_pr_vpq_info("lock_fw_gain = %d,fw_control_1.fw_pixel_gain = %d\n",lock_fw_gain, fw_control_1.fw_pixel_gain);
	}
	else if((LC_time % cmd_LC_period) == 0 ){
		rtd_pr_vpq_info("cur_frame_idx_global = %d\n",cur_frame_idx_global);
		
		drvif_color_get_LC_APL(&(APL_cur[0]));		
		LC_compute_pre_cur_deltaY(&(APL_pre[0]), &(APL_cur[0]),&(APL_delta[0]),&counter_blk_pre_cur_same, cmd_dY_preCur_mouseCon);//I,I,O
		drvif_color_get_SLD_fw_read_only((DRV_SLD_sld_fw_read_only*) &fw_read_only_data);

		if(
			(sld_work.osd_sta_blkx > 0)
			&&(sld_work.osd_end_blkx > 0)
			&&(sld_work.osd_sta_blky > 0)
			&&(sld_work.osd_end_blky > 0)
		){//OSD
			rtd_pr_vpq_info("reset condition----------------\n");
#ifdef SLD_MEMC_DISPLAY
			WriteRegister(0x000040A0, 0, 8, 0x1);//memc display
#endif
	 		drvif_color_set_SLD_global_drop();
			cur_frame_idx_global = 0;
			counter_box_logo_case = 0;
			index_box_logo_case = 0;
			counter_NEWS_case = 0;
			index_NEWS_case = 0;
			counter_noise_case = 0;
			index_noise_case = 0;	
			lock_fw_gain = 0;
			lock_rgb_box_fw_step = 0;
			LC_time = 0;
			flag_noise_in = 0;
			flag_rgb_box_in = 0;
			drvif_color_set_SLD_default();
			udelay(1666);
	 		drvif_color_set_SLD_disable_global_drop();

		}


		else if(//mouse condition 
			sld_condition.still_condition		
		){

			if(//R/G/B box condition 
				LC_detect_RGB_box_logo(&(APL_cur[0]),&(APL_pre[0]),cmd_thl_dC, cmd_thl_dUDL )
				|| (((cur_frame_idx_global - index_box_logo_case) < cmd_thl_error_time_box_logo_case)&&(index_box_logo_case != 0))
			){
				rtd_pr_vpq_info("debug mouse+R/G/B box condition : cur_frame_idx_global = %d,  index_box_logo_case = %d\n",cur_frame_idx_global,index_box_logo_case);

				if(LC_detect_RGB_box_logo(&(APL_cur[0]),&(APL_pre[0]),cmd_thl_dC, cmd_thl_dUDL )){
					rtd_pr_vpq_info("mouse+R/G/B box condition----------------\n");
					index_box_logo_case = cur_frame_idx_global;

				}else{
					rtd_pr_vpq_info("mouse+R/G/B box,in time condition----------------\n");
				}
			
				counter_box_logo_case = counter_box_logo_case+1;
				
				if(counter_box_logo_case > cmd_thl_counter_box_logo_case){
					LC_apply_RGB_box_logo(cmd_LC_period,cmd_thl_counter_box_logo_case,cmd_box_logo_pixel_gain,cmd_box_logo_maxGain_time);	


				}
			
			}
			else{//not R/G/B box condition 			
				rtd_pr_vpq_info("mouse condition--------------------\n");
#ifdef SLD_MEMC_DISPLAY				
				WriteRegister(0x000040A0, 0, 8, 0x2);//memc display
#endif				
				//fw control step until 0
				fw_control_0.regValue  					= IoReg_Read32(COLOR_SLD_SLD_FW_control_0_reg 			   );
				fw_control_0.fw_idx_ctrl_step	        = 8192-4;//S(13,0)
				fw_control_0.fw_idx_ctrl_en = 1;
				rtd_outl(COLOR_SLD_SLD_FW_control_0_reg , fw_control_0.regValue);	
			}
		}
		else if(//news condition 
			((fw_read_only_data.hpf_line_count > 3) && (fw_read_only_data.hpf_line_count < 10) && LC_detect_NEWS(&(APL_cur[0])))
			||(((cur_frame_idx_global - index_NEWS_case) < cmd_thl_error_time_NEWS_case) && (index_NEWS_case != 0))
		){

			if(((fw_read_only_data.hpf_line_count > 3) && (fw_read_only_data.hpf_line_count < 10) && LC_detect_NEWS(&(APL_cur[0])))){
				rtd_pr_vpq_info("NEWS condition--------------------\n");
#ifdef SLD_MEMC_DISPLAY				
				WriteRegister(0x000040A0, 0, 8, 0x3);//memc display
#endif				
				index_NEWS_case = cur_frame_idx_global;
			}else{
				rtd_pr_vpq_info("NEWS,in time condition----------------\n");
			}

			counter_NEWS_case = counter_NEWS_case+1;

			if(counter_NEWS_case > cmd_thl_counter_NEWS_case){
				fwif_color_apply_NEWS_setting();
				fwif_color_SLD_disable_window();
			}
			
		}
		else if(//R/G/B box condition 
			(
				(LC_detect_RGB_box_logo(&(APL_cur[0]),&(APL_pre[0]),cmd_thl_dC, cmd_thl_dUDL ))
				|| (((cur_frame_idx_global - index_box_logo_case) < cmd_thl_error_time_box_logo_case)&&(index_box_logo_case != 0))
			)
			||(flag_rgb_box_in == 1)
		){

			if(LC_detect_RGB_box_logo(&(APL_cur[0]),&(APL_pre[0]),cmd_thl_dC, cmd_thl_dUDL )){
				rtd_pr_vpq_info("R/G/B box condition----------------\n");
#ifdef SLD_MEMC_DISPLAY				
				WriteRegister(0x000040A0, 0, 8, 0x4);//memc display
#endif				
				index_box_logo_case = cur_frame_idx_global;

			}else{
				rtd_pr_vpq_info("R/G/B box,in time condition----------------\n");
			}
		
			counter_box_logo_case = counter_box_logo_case+1;
			
			if((counter_box_logo_case > cmd_thl_counter_box_logo_case) ||(flag_rgb_box_in == 1)){
				LC_apply_RGB_box_logo(cmd_LC_period,cmd_thl_counter_box_logo_case,cmd_box_logo_pixel_gain,cmd_box_logo_maxGain_time);
				flag_rgb_box_in = 1;
				rtd_pr_vpq_info("flag_rgb_box_in = %d --------------------\n",flag_noise_in);
				
			}
			
		}
		
		else if(//noise
			(
				(((drvif_color_SLD_get_noise_pattern_counter()) >= 129590) && LC_detect_noise(&(APL_cur[0])))
				||(((cur_frame_idx_global - index_noise_case) < cmd_thl_error_time_noise_case)&&(index_noise_case != 0))
			)
			||(flag_noise_in == 1)
		){

			if(((drvif_color_SLD_get_noise_pattern_counter()) >= 129590) && LC_detect_noise(&(APL_cur[0]))){
				rtd_pr_vpq_info("noise pattern condition--------------------\n");
#ifdef SLD_MEMC_DISPLAY				
				WriteRegister(0x000040A0, 0, 8, 0x5);//memc display
#endif				
				index_noise_case = cur_frame_idx_global;
			}else{
				rtd_pr_vpq_info("noise pattern,in time condition----------------\n");
			}

			counter_noise_case = counter_noise_case+1;

			if((counter_noise_case > cmd_thl_counter_noise_case) || (flag_noise_in == 1)){
				SLD_apply_noise_pattern(cmd_LC_period,cmd_thl_counter_noise_case);
				flag_noise_in = 1;	
				rtd_pr_vpq_info("flag_noise_in = %d --------------------\n",flag_noise_in);
			}


		}
		
		else{
			rtd_pr_vpq_info("mouse condition debug,counter_blk_pre_cur_same=%d\n", counter_blk_pre_cur_same);
			rtd_pr_vpq_info("NEWS condition debug,hpf_line_count=%d,LC_detect_NEWS=%d\n", fw_read_only_data.hpf_line_count,LC_detect_NEWS(&(APL_cur[0])));
			rtd_pr_vpq_info("box_logo condition debug, 1=%04d, 2=%04d, 3=%04d, 4=%04d, 5=%04d, 6=%04d, counter_box_logo_case = %d, (cur_frame_idx_global - index_box_logo_case) = %d\n", APL_cur[140],APL_cur[188],APL_cur[236],APL_cur[141],APL_cur[189],APL_cur[141],counter_box_logo_case,(cur_frame_idx_global - index_box_logo_case));


			if(counter_box_logo_case > 0){//leave r/g/b box
				lock_rgb_box_fw_step = cmd_box_lock_frame;// frame : lock_rgb_box_fw_step*cmd_LC_period
			}

			counter_box_logo_case = 0;
			counter_NEWS_case = 0;
			
			if(lock_fw_gain == 0){

			    drvif_color_set_SLD_default();
			
				fwif_color_SLD_add_con();//add condition add/ or	
				fwif_color_SLD_fw_read_only(cmd_lock_fw_gain_frame);//logo idx > 3000 has 1/4 area  +   spatial compensate



				
				LC_count_num_highY_block_upFrame(&(APL_cur[0]), thl_highY_frame,&counter_highY_blk);		
				if(counter_highY_blk > cmd_counter_frame_highY_blk ){//high Y block condition
					lock_fw_gain = cmd_lock_fw_gain_frame;
					rtd_pr_vpq_info("high Y block condition--------------------------------------\n");
#ifdef SLD_MEMC_DISPLAY					
					WriteRegister(0x000040A0, 0, 8, 0x6);//memc display
#endif					
				}
				else{
					//fw_control_1.fw_pixel_gain	            = 4096;
					fw_control_1.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_1_reg);	
					fw_control_1.fw_pixel_gain	            = CLAMP_SLD(fw_control_1.fw_pixel_gain + (4096/cmd_pixel_gain_step),4096,0);
					rtd_outl(COLOR_SLD_SLD_FW_control_1_reg , fw_control_1.regValue);	
					rtd_pr_vpq_info("high Y block condition debug, counter_highY_blk = %d\n",counter_highY_blk);
				}
				
		 		LC_compute_corner_UR_Y(&(APL_cur[0]),cmd_thl_counter_UR_con, &counter_UR);
				if(counter_UR > cmd_thl_counter_UR_con){//corner condition
					lock_fw_gain = cmd_lock_fw_gain_frame;
					rtd_pr_vpq_info("corner condition--------------------------------------\n");
#ifdef SLD_MEMC_DISPLAY					
					WriteRegister(0x000040A0, 0, 8, 0x7);//memc display
#endif										
				}
				else{
					fw_control_1.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_1_reg);	
					fw_control_1.fw_pixel_gain	            = CLAMP_SLD(fw_control_1.fw_pixel_gain + (4096/cmd_pixel_gain_step),4096,0);
					rtd_outl(COLOR_SLD_SLD_FW_control_1_reg , fw_control_1.regValue);	
					rtd_pr_vpq_info("corner condition debug, counter_UR = %d\n",counter_UR);
					
				}
			}   
		}

	    LC_time = 0;
	}
}

extern VIP_OPS_CTRL_Item OPS_CTRL_item;
void fwif_color_compute_APL_related_FW_New(void){
	unsigned short idx_r = 0, idx_c = 0, idx_arr = 0;
	int num_hblk = 0, num_vblk = 0, counter_blk_pre_cur_same = 0;
	int blk_cur_same_thl = 7000,blk_still_same_f = 0;
	int pixel_gain_step_decrease = 0;
	//int idx_step;
	unsigned int up_Left_counter_highY_blk = 0;
	unsigned int thl_highY_frame = 800;
	//unsigned int up_Left_counter_highY_blk_thl = 80;
	static unsigned int  memc_sc_motion1 = 0, memc_sc_motion2 = 0;
	int sld_weight=0,sld_height=0;
	int fflag = 0;
	color_sld_sld_fw_control_1_RBUS  fw_control_1;
	color_sld_sld_fw_control_0_RBUS  fw_control_0;
	color_sld_sld_curve_map_gain_RBUS curve_map_gain;
	color_sld_sld_hpf_1_RBUS color_sld_sld_hpf_1_reg;
	//color_sld_sld_acc_0_RBUS color_sld_sld_acc_0_reg;
	color_sld_sld_cnt_2_RBUS sld_cnt_2_reg;
	color_sld_sld_size_RBUS color_sld_size;

    //int memc_scene_change_flag = 0;
    //int real_scene_change_flag = 0;
    int still_image_flag = 0;
    int pixe_gain_decrease_flag = 0;
    //DRV_SLD_sld_fw_read_only fw_read_only_data;
    //DRV_SLD_sld_fw_control_2  fw_control_max_logo_tolerance_data = {0};
    DRV_SLD_sld_fw_control_0 fw_control_0_data = {0};
    //int stage1_period = 0, stage2_period = 0;
	static unsigned int cnt;

	unsigned short print_en = 0/*OPS_CTRL_item.OPS_Debug_Item.Debug_HWSW_SLD_En*/;
	unsigned short print_delay = 100/*OPS_CTRL_item.OPS_Debug_Item.Debug_Delay*/;

    color_sld_sld_control_RBUS color_sld_control;
    //color_sld_sld_hpf_1_RBUS sld_hpf_1_RBUS;
    color_sld_control.regValue = IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg);
   fw_control_0.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_0_reg);
   fw_control_1.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_1_reg);

    curve_map_gain.regValue = rtd_inl(COLOR_SLD_SLD_CURVE_MAP_GAIN_reg);
    sld_cnt_2_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_CNT_2_reg);
    color_sld_sld_hpf_1_reg.regValue = rtd_inl(COLOR_SLD_SLD_HPF_1_reg);


    cur_frame_idx_global = cur_frame_idx_global % 65535;
    cur_frame_idx_global = cur_frame_idx_global + 1;


	//if pattern gen enable bypass all FW and diable SLD
	if(0/*v4l2_vpq_ire != V4L2_EXT_VPQ_INNER_PATTERN_DISABLE*/){
		//drvif_color_set_SLD_Enable(0);
		
		fw_control_0_data.fw_idx_ctrl_step = 8192 - 4000 ; //8192-(sld_cnt_2_reg.sld_max_pre_idx/2)
		fw_control_0_data.fw_idx_ctrl_en   = 1;
		drvif_color_set_SLD_fw_control_step(&fw_control_0_data);
		if ((print_en & _BIT0) != 0) {
			if (print_delay == 0)
				print_delay = 100;
			if((cnt % print_delay) == 0) {
				rtd_pr_vpq_emerg("k7_sld_sw,bypass FW_sld\n");
			}
		}		
		goto bypass_FW_sld;
	}else{
		fw_control_0.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_0_reg);
		fw_control_0.fw_idx_ctrl_en = 0;
		IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg, fw_control_0.regValue);
	}

    
    color_sld_size.regValue = IoReg_Read32(COLOR_SLD_SLD_SIZE_reg);
    sld_weight = color_sld_size.sld_width;
    sld_height  = color_sld_size.sld_height;

    num_hblk = ceil_SLD(sld_weight, 32);
    num_vblk = ceil_SLD(sld_height, 32);
    //rtd_pr_vpq_emerg("SLD_HW Size info, sld_weight:%d, sld_height:%d\n",sld_weight,sld_height);
    //rtd_pr_vpq_info("SLD_HW APL info, num_hblk:%d, num_vblk:%d\n",num_hblk,num_vblk);

	
    //Save previous fram Block APL
    for (idx_r = 0; idx_r < (num_vblk); idx_r++) {
        for (idx_c = 0; idx_c < (num_hblk); idx_c++) {
            APL_pre_uint16[idx_arr] = APL_cur_uint16[idx_arr];
            idx_arr = idx_arr + 1;
        }
    }

    LC_time = LC_time + 1;


    /* Read SLD block APL info*/
    drvif_fwif_color_Get_SLD_APL(APL_cur_uint16, num_hblk*num_vblk);
    Compute_pre_cur_deltaY(num_hblk, num_vblk, &(APL_pre_uint16[0]), &(APL_cur_uint16[0]), &APL_avg_cur, &(APL_delta_uint16[0]), &counter_blk_pre_cur_same, cmd_box_lock_frame); //I,I,O

	if((cnt % print_delay) == 0)
		rtd_pr_vpq_info("SLD_HW counter_blk_pre_cur_same:%d\n",counter_blk_pre_cur_same);
    //APL_avg_cur = APL_avg_cur / 16;
	if( counter_blk_pre_cur_same>= blk_cur_same_thl){
		blk_still_same_f = 1;
		if((cnt % print_delay) == 0)
			rtd_pr_vpq_info("SLD_HW blk_still_same_f%d\n",blk_still_same_f);
	}
		
		
#ifndef CONFIG_MEMC_NOTSUPPORT // for code size reduce, when MEMC_BYPASS
    /*Detected scene change*/
    scene_change_flag = MEMC_Lib_GetInfoForSLD(1);

    if (MEMC_Lib_GetInfoForSLD(4) > memc_sc_motion1) {
        memc_sc_motion1 = MEMC_Lib_GetInfoForSLD(4);
    }
    if (MEMC_Lib_GetInfoForSLD(5) > memc_sc_motion2) {
        memc_sc_motion2 = MEMC_Lib_GetInfoForSLD(5);
    }
#endif   
    if ( (scene_change_flag ==1)&& (memc_sc_motion1 > 10000) && (memc_sc_motion2 > 10000)) {
        memc_scene_change_flag_global = 10;
    }
    if (memc_scene_change_flag_global > 0) {
        memc_scene_change_flag_global--;
    }

    /*Detect still image condition*/
    if( ((memc_sc_motion1 < 100) && (memc_sc_motion2 < 100)) /*|| blk_still_same_f*/ ){
		still_image_flag = 1;
    }else
    	still_image_flag = 0;

    /*Count the up left frame of highY block*/
    Count_num_highY_block_up_Left_Frame(num_hblk, &(APL_cur_uint16[0]), thl_highY_frame, &up_Left_counter_highY_blk);

    /*Detect 4 circles pattern*/
    if (circle4_flag == 1) {
        fflag = 1;
		if ((print_en & _BIT0) != 0) {
			rtd_pr_vpq_info("SLD_HW, circle4_flag=%d\n",circle4_flag);
		}
    }


    /*End of detection */


    /*When detect frame is still*/
    if(still_image_flag){
	sld_cnt_2_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_CNT_2_reg);	
	fw_control_0.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_0_reg);
	fw_control_1.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_1_reg);
	if(sld_cnt_2_reg.sld_max_pre_idx<500){
		 //Freeze_full_frame_idx
		 fw_control_0.fw_idx_freeze_for_full_frame = 1;
		 IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg, fw_control_0.regValue);

	}else{
		 //Decrease apply pixel_gain
	       fw_control_1.fw_pixel_gain = CLAMP_SLD(fw_control_1.fw_pixel_gain - (4096 / 100), 4096, 0);
	       IoReg_Write32(COLOR_SLD_SLD_FW_control_1_reg, fw_control_1.regValue);


		 //Decrease logo idx when pixel_gain<50
		 if(fw_control_1.fw_pixel_gain<50){
			if(sld_cnt_2_reg.sld_max_pre_idx>500){
				fw_control_0.fw_idx_freeze_for_full_frame = 0;
				fw_control_0.fw_idx_ctrl_step = 8192 - (sld_cnt_2_reg.sld_max_pre_idx / 15) ; 
				fw_control_0.fw_idx_ctrl_en = 1;
				IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg, fw_control_0.regValue);			
			}
			

		 }
		 pixe_gain_decrease_flag = 1;
	}
		if ((print_en & _BIT0) != 0) {
			if (print_delay == 0)
				print_delay = 100;
			if((cnt % print_delay) == 0) {
				rtd_pr_vpq_info("SLD_HW detect still image \n");
				rtd_pr_vpq_info("SLD_HW fw_pixel_gain:%d, sld_max_pre_idx:%d, pixe_gain_decrease_flag:%d \n",fw_control_1.fw_pixel_gain,sld_cnt_2_reg.sld_max_pre_idx,pixe_gain_decrease_flag);
			}
		}
    }else{
    	/*When detect frame is not still*/
		fw_control_0.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_0_reg);
		sld_cnt_2_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_CNT_2_reg);
		
	 	fw_control_0.fw_idx_freeze_for_full_frame = 0;
	 	IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg, fw_control_0.regValue);

		if(memc_scene_change_flag_global>0){
			/*scene change*/
			 fw_control_0.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_0_reg);
			if ((print_en & _BIT0) != 0) {
			 	rtd_pr_vpq_info("SLD_HW scene change, sld_max_pre_idx=%d\n",sld_cnt_2_reg.sld_max_pre_idx);
			}
			 if (sld_cnt_2_reg.sld_max_pre_idx > 2500) {
			     
			     fw_control_0.fw_idx_ctrl_step = 8192 - (sld_cnt_2_reg.sld_max_pre_idx / 500) ; 
			     fw_control_0.fw_idx_ctrl_en = 1;
			     IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg, fw_control_0.regValue);

             }else if (sld_cnt_2_reg.sld_max_pre_idx > 1800) {
			     fw_control_0.fw_idx_ctrl_step = 8192 - (sld_cnt_2_reg.sld_max_pre_idx / 600) ; 
			     fw_control_0.fw_idx_ctrl_en = 1;
			     //IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg, fw_control_0.regValue);

             }else if(fflag>0){
             		if ((print_en & _BIT0) != 0) {
					rtd_pr_vpq_info("SLD_HW, fflag=%d and detect scene change\n",fflag);
             		}
				 if(sld_cnt_2_reg.sld_max_pre_idx >600){
					 fw_control_0.fw_idx_ctrl_step = 8192 - (sld_cnt_2_reg.sld_max_pre_idx / 600) ; 
				     fw_control_0.fw_idx_ctrl_en = 1;
				     IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg, fw_control_0.regValue);
				 }

				fflag = 0;
			 }
			
		}else if(fflag>0){
			 sld_cnt_2_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_CNT_2_reg);
			 fw_control_0.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_0_reg);	
			 if ((print_en & _BIT0) != 0) {
			 	rtd_pr_vpq_info("SLD_HW, fflag=%d\n",fflag);
			 }
        	//fflag--;
			 if (sld_cnt_2_reg.sld_max_pre_idx > 2500) {
			     
			     fw_control_0.fw_idx_ctrl_step = 8192 - (sld_cnt_2_reg.sld_max_pre_idx / 400) ; 
			     fw_control_0.fw_idx_ctrl_en = 1;
			     IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg, fw_control_0.regValue);

             }else if (sld_cnt_2_reg.sld_max_pre_idx > 1800) {
			     fw_control_0.fw_idx_ctrl_step = 8192 - (sld_cnt_2_reg.sld_max_pre_idx / 600) ; 
			     fw_control_0.fw_idx_ctrl_en = 1;			
				 IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg, fw_control_0.regValue);
				 }
			 fflag = 0;
		}else if(up_Left_counter_highY_blk > cmd_counter_frame_highY_blk){ //high Y block condition
                    if (up_Left_counter_highY_blk >= 200) {
                        lock_fw_gain = 3;
                        cmd_pixel_gain_step = 3;
						pixel_gain_step_decrease = cmd_pixel_gain_step*5;
                    }
                    else if (up_Left_counter_highY_blk >= 180) {
                        lock_fw_gain = 10;
                        cmd_pixel_gain_step = 10;
						pixel_gain_step_decrease = cmd_pixel_gain_step*10;
                    }
                    else if (up_Left_counter_highY_blk >= 140) {
                        lock_fw_gain = 15;
                        cmd_pixel_gain_step = 15;
						pixel_gain_step_decrease = cmd_pixel_gain_step*15;
                    }
                    else if (up_Left_counter_highY_blk >= 120) {
                        lock_fw_gain = 20;
                        cmd_pixel_gain_step = 20;
						pixel_gain_step_decrease = cmd_pixel_gain_step*20;
                    }
                    else if (up_Left_counter_highY_blk >= 100) {
                        lock_fw_gain = 25;
                        cmd_pixel_gain_step = 25;
						pixel_gain_step_decrease = cmd_pixel_gain_step*20;
                    }
                    else {
                        lock_fw_gain = 30;
                        cmd_pixel_gain_step = 30;
						pixel_gain_step_decrease = cmd_pixel_gain_step*30;
                    }
			fw_control_1.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_1_reg);
			fw_control_1.fw_pixel_gain = CLAMP_SLD(fw_control_1.fw_pixel_gain - (4096 / pixel_gain_step_decrease), 4095, 0);	
			IoReg_Write32(COLOR_SLD_SLD_FW_control_1_reg, fw_control_1.regValue);
			pixe_gain_decrease_flag = 1;
			if ((print_en & _BIT0) != 0) {
				rtd_pr_vpq_info("SLD_HW high Y block condition, decrease piexl gain:%d,  remain pixel gain:%d\n",4096 / cmd_pixel_gain_step,fw_control_1.fw_pixel_gain);
			}
         }else if(LR_LOGO_Flag_R || LR_LOGO_Flag_L){
			if ((print_en & _BIT0) != 0) {
				rtd_pr_vpq_info("SLD_HW, LR_LOGO_Flag_L=%d, LR_LOGO_Flag_R=%d\n",LR_LOGO_Flag_L,LR_LOGO_Flag_R);
			}
			if(sld_cnt_2_reg.sld_max_pre_idx >600){
		        fw_control_0.fw_idx_ctrl_step = 8192 - (sld_cnt_2_reg.sld_max_pre_idx/500) ;
		        fw_control_0.fw_idx_ctrl_en = 1;
	 			IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg, fw_control_0.regValue);
			}

		 }else{
			//normal condition
			if((cnt % print_delay) == 0)
				rtd_pr_vpq_info("SLD_HW normal condition \n");
			fw_control_1.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_1_reg);
			if( fw_control_1.fw_pixel_gain<4096 ){
				//recover fw_pixel_gain slowly
				fw_control_1.fw_pixel_gain = CLAMP_SLD(fw_control_1.fw_pixel_gain + (4096 / 10), 4095, 0);	
				IoReg_Write32(COLOR_SLD_SLD_FW_control_1_reg, fw_control_1.regValue);
				if(fw_control_1.fw_pixel_gain==4095){
					pixe_gain_decrease_flag = 0;
				}

				fw_control_1.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_1_reg);
				//rtd_pr_vpq_info("SLD_HW pixel gain increase,  pixel gain:%d\n",fw_control_1.fw_pixel_gain);
			}
			
		   }


    }

	/*reset memc motion flag*/
	scene_change_flag = 0;
	memc_sc_motion1 = 0;
	memc_sc_motion2 = 0;
	
	/*reset */
	pixe_gain_decrease_flag = 0;
	fflag = 0;
	counter_blk_pre_cur_same = 0;
	blk_still_same_f = 0;
#if 0
    fw_control_1.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_1_reg);

    drvif_color_get_SLD_fw_read_only((DRV_SLD_sld_fw_read_only *) &fw_read_only_data); //Read max_logo_idx_counter info
    if (fw_read_only_data.sld_max_pre_idx >= 4000) {
        fw_control_max_logo_tolerance_data.max_pre_logo_tolerance = 2700;    //Set max_logo_idx tolerance
    }
    else if (fw_read_only_data.sld_max_pre_idx >= 3000) {
        fw_control_max_logo_tolerance_data.max_pre_logo_tolerance = 1700;    //Set max_logo_idx tolerance
    }
    else if (fw_read_only_data.sld_max_pre_idx >= 1800) {
        fw_control_max_logo_tolerance_data.max_pre_logo_tolerance = 500;    //Set max_logo_idx tolerance
    }
    else {
        /*fw_control_0_data.fw_idx_ctrl_step = 8192;
        fw_control_0_data.fw_idx_ctrl_en = 0;
        drvif_color_set_SLD_fw_control_step(&fw_control_0_data);*/
    }

    drvif_color_set_SLD_max_pre_logo_tolerance((DRV_SLD_sld_fw_control_2 *) &fw_control_max_logo_tolerance_data);
    drvif_color_get_SLD_fw_read_only((DRV_SLD_sld_fw_read_only *) &fw_read_only_data); //Read max_logo_idx_counter info

    //rtd_pr_vpq_info("[sld] -------max_pre_logo_idx = %d,NEAR max_pre_logo_idx_counter = %d---------\n", fw_read_only_data.sld_max_pre_idx, fw_read_only_data.near_max_pre_logo_idx_counter);
    if (fw_read_only_data.sld_max_pre_idx >= 4000 && fw_read_only_data.near_max_pre_logo_idx_counter >= 2000 ) {
        fw_control_0_data.fw_idx_ctrl_step = 8192 - (sld_cnt_2_reg.sld_max_pre_idx) ;
        fw_control_0_data.fw_idx_ctrl_en = 1;
        drvif_color_set_SLD_fw_control_step(&fw_control_0_data);
        /*#ifdef SLD_MEMC_DISPLAY
        WriteRegister(0xb809d000, 15, 15, 0x1);//display scene  change
        WriteRegister(0xb809d000, 13, 13, 0x1);//display scene  change
        #endif*/
		//rtd_pr_vpq_info("k7_sld_sw,Logo area Condition1. max_pre_idx:%d, logo area:%d,\n", fw_read_only_data.sld_max_pre_idx,fw_read_only_data.near_max_pre_logo_idx_counter >= 2000);
    }
    else if (fw_read_only_data.sld_max_pre_idx >= 3000 && fw_read_only_data.near_max_pre_logo_idx_counter >= 2200) {
        fw_control_0_data.fw_idx_ctrl_step = 8192 - (sld_cnt_2_reg.sld_max_pre_idx) ;
        fw_control_0_data.fw_idx_ctrl_en = 1;
        drvif_color_set_SLD_fw_control_step(&fw_control_0_data);
        /*#ifdef SLD_MEMC_DISPLAY
        WriteRegister(0xb809d000, 15, 15, 0x1);//display scene  change
        WriteRegister(0xb809d000, 13, 13, 0x1);//display scene  change
        #endif*/  
		//rtd_pr_vpq_info("k7_sld_sw,Logo area Condition2. max_pre_idx:%d, logo area:%d,\n", fw_read_only_data.sld_max_pre_idx,fw_read_only_data.near_max_pre_logo_idx_counter >= 2000);
    }
    else if ((fw_read_only_data.sld_max_pre_idx > 1800) && (fw_read_only_data.near_max_pre_logo_idx_counter >= 3000)) { //12960
        

        fw_control_0_data.fw_idx_ctrl_step = 8192 - (sld_cnt_2_reg.sld_max_pre_idx) ;
        fw_control_0_data.fw_idx_ctrl_en = 1;
        drvif_color_set_SLD_fw_control_step(&fw_control_0_data);
        /*#ifdef SLD_MEMC_DISPLAY
        WriteRegister(0xb809d000, 15, 15, 0x1);//display scene  change
        WriteRegister(0xb809d000, 13, 13, 0x1);//display scene  change
        #endif*/

        //fw_control_1.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_1_reg);
        //fw_control_0_data.fw_idx_ctrl_en =0;
        //rtd_outl(COLOR_SLD_SLD_FW_control_1_reg , fw_control_1.regValue);
        //rtd_pr_vpq_info("k7_sld_sw,Logo area Condition3 max_pre_idx:%d, logo area:%d,\n", fw_read_only_data.sld_max_pre_idx,fw_read_only_data.near_max_pre_logo_idx_counter >= 2000);
    }
    else if(circle4_flag!=1){
        /*#ifdef SLD_MEMC_DISPLAY
            WriteRegister(0xb809d000, 15, 15, 0x0);//display scene  change
            WriteRegister(0xb809d000, 13, 13, 0x0);//display scene  change
        #endif*/
        fw_control_0_data.fw_idx_ctrl_step = 0 ;
        fw_control_0_data.fw_idx_ctrl_en = 0;
        drvif_color_set_SLD_fw_control_step(&fw_control_0_data);
		//rtd_pr_vpq_info("k7_sld_sw,Logo area Condition4.\n");

    }

    if (LR_LOGO_Flag_R || LR_LOGO_Flag_L) {
        fw_control_1.fw_pixel_gain = CLAMP_SLD(fw_control_1.fw_pixel_gain - (4096 / 5), 4095, 0);
        shopping_ch = 1;
        shopping_ch_gain_count ++;
        fw_control_0_data.fw_idx_ctrl_step = 8192 - (sld_cnt_2_reg.sld_max_pre_idx/4) ;
        fw_control_0_data.fw_idx_ctrl_en = 1;
        drvif_color_set_SLD_fw_control_step(&fw_control_0_data);
		rtd_outl(COLOR_SLD_SLD_FW_control_1_reg, fw_control_1.regValue);

    }
    else if (shopping_ch == 1) {
        if (shopping_ch_gain_count > 0) {
            fw_control_1.fw_pixel_gain = CLAMP_SLD(fw_control_1.fw_pixel_gain + (4096 / 360), 4095, 0);
            shopping_ch_gain_count --;
        }
        else {
            shopping_ch = 0;
        }
        rtd_outl(COLOR_SLD_SLD_FW_control_1_reg, fw_control_1.regValue);
    }
#endif
	cnt++;
	bypass_FW_sld:
	
	if ((print_en & _BIT0) != 0) {
		if (print_delay == 0)
			print_delay = 100;
		if((cnt % print_delay) == 0) {
			rtd_pr_vpq_info("k7_sld_sw, fw_idx_ctrl_step:%d\n", fw_control_0_data.fw_idx_ctrl_step);
		}
	}	
	return;
	
}


void SLD_apply_condition(void)
{
	static unsigned int frame_cnt = 0;

	//Merlin6, SLD
	//unsigned int counter_blk_pre_cur_same = 0;
	//int thl_temporal_dY = 20;
	//int thl_counter_NEWS_case = 240;//tolerance num of frame
	color_sld_sld_fw_control_0_RBUS	 fw_control_0;
			
	if(sld_condition.still_condition){//apply
			rtd_pr_vpq_info("still condition--------------------\n");
			
			//fw control step until 0
			fw_control_0.regValue  		 = IoReg_Read32(COLOR_SLD_SLD_FW_control_0_reg 			   );
			fw_control_0.fw_idx_ctrl_step = 8192-4;//S(13,0)
			fw_control_0.fw_idx_ctrl_en   = 1;
			IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg , fw_control_0.regValue);	
	}
	else if(sld_condition.news_condition)
	{
			fwif_color_apply_NEWS_setting();
			fwif_color_SLD_disable_window();
	}
	else if(sld_condition.normal_condition){
			fwif_color_SLD_apply_normal_setting();
	}

	frame_cnt++;

}



static void eq_table_insert( int a, int b )
{
	int base = (a<b) ? a : b;
	int ins = (a<b) ? b : a;
	int cur_idx;
	int nxt_idx;

	if( a==b )
		return;

	cur_idx = base;
	while(1)
	{
		nxt_idx = eq_table[cur_idx][1];
		if( nxt_idx == ins ) // already in list
			return;
		else if( nxt_idx == 0 || nxt_idx > ins )
			break;
		cur_idx = nxt_idx;
	}
	if( nxt_idx == 0 ) // stopped at list end
	{
		eq_table[cur_idx][1] = ins;
		eq_table[ins][0] = cur_idx;
		eq_table[ins][1] = 0;
	}
	else // stopped in list, insert between cur_idx & nxt_idx
	{
		eq_table[cur_idx][1] = ins;
		eq_table[ins][0] = cur_idx;
		eq_table[ins][1] = 0;
		eq_table[nxt_idx][0] = ins;
	}
}

void memc_logo_to_demura_read(void)
{
#ifdef CONFIG_MEMC_NOTSUPPORT // for code size reduce, when MEMC_BYPASS
		return;
#else
	static unsigned int frame_cnt = 0;
	unsigned char* logo_ptr = LD_virAddr;

	static unsigned char memc_sc_flag = false;
	static unsigned int  memc_sc_motion1 = 0, memc_sc_motion2 = 0;

	// LD block average
	blu_ld_global_ctrl2_RBUS ld_global_ctrl2_reg;
	unsigned char LD_valid, LD_type, LD_Hnum, LD_Vnum;

	// OSD mixer detect
	static unsigned char osd_measure_init = false;
	osdovl_mixer_ctrl2_RBUS osdovl_mixer_ctrl2_reg;
	osdovl_osd_db_ctrl_RBUS osdovl_osd_db_ctrl_reg;
	osdovl_measure_osd1_sta_RBUS osdovl_measure_osd1_sta_reg;
	osdovl_measure_osd1_end_RBUS osdovl_measure_osd1_end_reg;
	short osd_sta_x, osd_sta_y, osd_end_x, osd_end_y;

	// dynamic calculate the offset
	if( sld_ddr_offset_auto_get == 1 )
	{
		sld_ddr_offset = IoReg_Read32(KME_DM_TOP1_KME_DM_TOP1_48_reg)&0xfff;
	}

	logo_ptr = LD_virAddr + sld_ddr_offset;

	// start to work
	if( !osd_measure_init )
	{
		osdovl_mixer_ctrl2_reg.regValue = IoReg_Read32(OSDOVL_Mixer_CTRL2_reg);
		osdovl_osd_db_ctrl_reg.regValue = IoReg_Read32(OSDOVL_OSD_Db_Ctrl_reg);

		osdovl_mixer_ctrl2_reg.measure_osd_zone_en = 1;
		osdovl_mixer_ctrl2_reg.measure_osd_zone_type = 1;
		IoReg_Write32( OSDOVL_Mixer_CTRL2_reg, osdovl_mixer_ctrl2_reg.regValue );

		osdovl_osd_db_ctrl_reg.db_load = 1;
		IoReg_Write32( OSDOVL_OSD_Db_Ctrl_reg, osdovl_osd_db_ctrl_reg.regValue );

		osd_measure_init = true;
	}

	osdovl_measure_osd1_sta_reg.regValue = IoReg_Read32(OSDOVL_measure_osd1_sta_reg);
	osdovl_measure_osd1_end_reg.regValue = IoReg_Read32(OSDOVL_measure_osd1_end_reg);
	osd_sta_x = osdovl_measure_osd1_sta_reg.x;
	osd_sta_y = osdovl_measure_osd1_sta_reg.y;
	osd_end_x = osdovl_measure_osd1_end_reg.x;
	osd_end_y = osdovl_measure_osd1_end_reg.y;

	if( osd_sta_x == 0x1fff && osd_sta_y == 0x1fff && osd_end_x == 0 && osd_end_y == 0 ) // no OSD shown
	{
		sld_work.osd_sta_blkx = -1;
		sld_work.osd_sta_blky = -1;
		sld_work.osd_end_blkx = -1;
		sld_work.osd_end_blky = -1;
	}
	else // mark osd blocks
	{
		sld_work.osd_sta_blkx = osd_sta_x / 8;
		sld_work.osd_end_blkx = (osd_end_x+7) / 8;
		sld_work.osd_sta_blky = osd_sta_y / 8;
		sld_work.osd_end_blky = (osd_end_y+7) / 8;
	}

	// motion status (read every frame)
	if( MEMC_Lib_GetInfoForSLD(1) ) // scene change flag
	{
		memc_sc_flag= true;
	}
	if( MEMC_Lib_GetInfoForSLD(4) > memc_sc_motion1 )
		memc_sc_motion1 = MEMC_Lib_GetInfoForSLD(4);
	if( MEMC_Lib_GetInfoForSLD(5) > memc_sc_motion2 )
		memc_sc_motion2 = MEMC_Lib_GetInfoForSLD(5);

	if( memc_logo_to_demura_update_cnt == 0 )
		memc_logo_to_demura_update_cnt = 1;
	if( frame_cnt % memc_logo_to_demura_update_cnt == 0 ) // read once every update_cnt frames
	{
		if( !sld_work.read_ok )
		{
			//time_start = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
			// read memc logo
			//time_start = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
			if( memc_logo_read_en && LD_virAddr != NULL && MEMC_Lib_GetInfoForSLD(7) )
			{
				// read whole memc logo memory
				memcpy(logo_flag_map_raw, logo_ptr, 192*540);
				/*
				for( i=0; i<270; i++ )
				{
					memcpy(logo_byte_line[0], logo_ptr, 960/8);
					logo_ptr += 192;
					memcpy(logo_byte_line[1], logo_ptr, 960/8);
					logo_ptr += 192;

					for( j=0; j<960/8; j++ )
					{
						unsigned char logo_byte1 = logo_byte_line[0][j];
						unsigned char logo_byte2 = logo_byte_line[1][j];
						unsigned char logo_bit[2][8] = {0};
						unsigned char logo_flag_block;
						for( k=0; k<8; k++ )
						{
							logo_bit[0][k] = (logo_byte1 & ( 1<<(7-k) )) ? 1 : 0;
							logo_bit[1][k] = (logo_byte2 & ( 1<<(7-k) )) ? 1 : 0;
							//logo_flag_map[i*960 + j + k] = ( (logo_byte & ( 1<<(7-k) )) != 0 );
						}
						for( k=0; k<4; k++ )
						{
							logo_flag_block = logo_bit[0][k*2] || logo_bit[0][k*2+1] || logo_bit[1][k*2] || logo_bit[1][k*2+1];
							logo_flag_map[i*481 + j*4 + k] = logo_flag_block;
						}
					}
					logo_flag_map[i*481 + 480] = logo_flag_map[i*481 + 479]; // last point repeat
				}
				for( j=0; j<=480; j++ ) // last line repeat
					logo_flag_map[270*481 + j] = logo_flag_map[269*481 + j];
				*/

				sld_work.do_counter_update = true;
				sld_work.do_reset_counter = false;
				//if( MEMC_Lib_GetInfoForSLD(0) != 0 ) // pixel logo clear signal
				//	do_counter_update = false;
				if( memc_sc_motion1 < 100 && memc_sc_motion2 < 100 ) // motion cnt low: picture pause
					sld_work.do_counter_update = false;
				else if( memc_sc_flag && memc_sc_motion1 > 10000 && memc_sc_motion2 > 10000 ) // scene change flag + big motion: scene change
				{
					sld_work.do_counter_update = true;
					sld_work.do_reset_counter = true;
				}
				sld_work.do_reset_full = false;

				// reset motion flag after used
				memc_sc_flag = false;
				memc_sc_motion1 = 0;
				memc_sc_motion2 = 0;
			}
			else
			{
				sld_work.do_counter_update = true;
				sld_work.do_reset_counter = true;
				sld_work.do_reset_full = true;
			}

			// LD related info
			ld_global_ctrl2_reg.regValue = IoReg_Read32(BLU_LD_Global_Ctrl2_reg);
			LD_valid = ld_global_ctrl2_reg.ld_valid;
			LD_type = ld_global_ctrl2_reg.ld_blk_type;
			LD_Hnum = ld_global_ctrl2_reg.ld_blk_hnum + 1;
			LD_Vnum = ld_global_ctrl2_reg.ld_blk_vnum + 1;

			if( LD_valid && LD_type == 0 && LD_Hnum == 32 && LD_Vnum == 18 )
			{
				sld_work.LD_APL_valid = drvif_color_get_LD_APL_ave( blk_apl_average );
			}
			else
			{
				sld_work.LD_APL_valid = false;
			}

			sld_work.read_ok = true;

			// trigger apply
			schedule_work(&(sld_work.sld_apply_work));

			//time_end = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
			//rtd_pr_vpq_emerg("phase1 time: %d on 90k clock\n", (time_end-time_start));

		}
		else // read period is faster than calculation time
		{
			//rtd_pr_vpq_emerg("SLD read stopped because apply is not done yet!\n");
		}

	}

	frame_cnt++;
#endif
}

sld_work_debug_struct sld_work_debug = {0};
/*static void memc_logo_to_demura_debug_log(int drop_block)
{
    unsigned short en = sld_work_debug.pr_En;
    unsigned short x = sld_work_debug.pr_point_x;
    unsigned short y = sld_work_debug.pr_point_y;
    unsigned short delay = sld_work_debug.pr_delay;
    short mark_val = sld_work_debug.mark_val;

    static unsigned int cnt = 0;
    int idx_0, idx_1, idx_2;
    int wait_time = 1;
    int drop_time = 1;
    int frm_rate = Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ);
	unsigned short h_num;
	
    if (delay == 0) {
        delay = 1;
    }

	if(DeMura_TBL.table_mode==DeMura_TBL_481x271) {
		h_num = 481;
	} else
		h_num = 241;

    if ((en & _BIT0) != 0) {
        if (cnt % delay == 0) {
            idx_2 = h_num * y + x;
            idx_1 = ((idx_2 - 1) < 0) ? (0) : (idx_2 - 1);
            idx_0 = ((idx_1 - 1) < 0) ? (0) : (idx_1 - 1);
            rtd_pr_memc_notice("sw_sld=debug_info=%d,%d,%d,%d,%d,\n", idx_0, idx_1, idx_2, x, y);
        }
    }

    if ((en & _BIT1) != 0) {
        if (cnt % delay == 0) {

            wait_time = (memc_logo_to_demura_wait_time * frm_rate) / 10;
            if (wait_time <= 0) {
                wait_time = 1;
            }
            drop_time = (memc_logo_to_demura_drop_time * frm_rate) / 10;
            if (drop_time <= 0) {
                drop_time = 1;
            }

            rtd_pr_memc_notice("sw_sld=memc_info=%d,%d,%d,%d,t=%d,%d,osd,%d,%d,%d,%d,drop=%d,\n",
                   MEMC_Lib_GetInfoForSLD(1), MEMC_Lib_GetInfoForSLD(4), MEMC_Lib_GetInfoForSLD(5),
                   MEMC_Lib_GetInfoForSLD(7), wait_time, drop_time, sld_work.osd_sta_blkx,
                   sld_work.osd_sta_blky, sld_work.osd_end_blkx, sld_work.osd_end_blky, drop_block);
        }
    }

    if ((en & _BIT2) != 0) {
        if (cnt % delay == 0) {
            idx_2 = h_num * y + x;
            idx_1 = ((idx_2 - 1) < 0) ? (0) : (idx_2 - 1);
            idx_0 = ((idx_1 - 1) < 0) ? (0) : (idx_1 - 1);
            rtd_pr_memc_notice("sw_sld=log_map=%d,%d,%d,\n", logo_flag_map[idx_0], logo_flag_map[idx_1],
                   logo_flag_map[idx_2]);
        }
    }

    if ((en & _BIT3) != 0) {
        if (cnt % delay == 0) {
            idx_2 = h_num * y + x;
            idx_1 = ((idx_2 - 1) < 0) ? (0) : (idx_2 - 1);
            idx_0 = ((idx_1 - 1) < 0) ? (0) : (idx_1 - 1);
            rtd_pr_memc_notice("sw_sld=cnt_map=%d,%d,%d,\n", logo_demura_counter[idx_0], logo_demura_counter[idx_1],
                   logo_demura_counter[idx_2]);
        }
    }

    if ((en & _BIT4) != 0) {
        if (cnt % delay == 0) {
            idx_2 = h_num * y + x;
            idx_1 = ((idx_2 - 1) < 0) ? (0) : (idx_2 - 1);
            idx_0 = ((idx_1 - 1) < 0) ? (0) : (idx_1 - 1);
            rtd_pr_memc_notice("sw_sld=apl=%d,%d,%d,\n", blk_apl_interp[idx_0], blk_apl_interp[idx_1],
                   blk_apl_interp[idx_2]);
        }
    }

    if ((en & _BIT5) != 0) {
        if (cnt % delay == 0) {
            idx_2 = h_num * y + x;
            idx_1 = ((idx_2 - 1) < 0) ? (0) : (idx_2 - 1);
            idx_0 = ((idx_1 - 1) < 0) ? (0) : (idx_1 - 1);
            rtd_pr_memc_notice("sw_sld=demura=%d,%d,%d,\n", demura_tbl[2][idx_0], demura_tbl[2][idx_1],
                   demura_tbl[2][idx_2]);
        }
    }

    if ((en & _BIT10) != 0) {
        idx_2 = h_num * y + x;
        idx_1 = ((idx_2 - 1) < 0) ? (0) : (idx_2 - 1);
        idx_0 = ((idx_1 - 1) < 0) ? (0) : (idx_1 - 1);

        demura_tbl[2][idx_0] = mark_val * (-1);
        demura_tbl[2][idx_1] = mark_val * (-1);
        demura_tbl[2][idx_2] = mark_val * (-1);

        demura_tbl[1][idx_0] = mark_val * (-1);
        demura_tbl[1][idx_1] = mark_val * (-1);
        demura_tbl[1][idx_2] = mark_val * (-1);
    }

    cnt++;
}*/

char memc_logo_to_demura_4circle_detect(unsigned char table_mode)
{
	unsigned short en = sld_work_debug.pr_En;
	unsigned short delay = sld_work_debug.pr_delay;
	static unsigned int cnt;
	unsigned int ratio = 900; // %%

	unsigned int i, j, index;
	unsigned int sum = 0, sum1 = 0;
	unsigned int total = 0;
	unsigned int th; //
	index = 0;
	sum = 0;
	sum1 = 0;
	
	if (table_mode == DeMura_TBL_481x271) {
		for (j = logo_circle4_y_sta; j <= logo_circle4_y_end; j++) {
		    for (i = logo_circle4_x_sta; i <= logo_circle4_x_end; i++) {
		        if (logo_flag_map[j * 481 + i] == logo_flag_4circle[index]) {
		            sum++;
		        }
		        index++;
		    }
		}

		total = logo_circle4_h * logo_circle4_v;
		th = (ratio * total) >> 10;
	} else {
		index = 0;
		sum = 0;
		sum1 = 0;
		for (j = logo_circle4_y_sta_271x136; j <= logo_circle4_y_end_271x136; j++) {
		    for (i = logo_circle4_x_sta_271x136; i <= logo_circle4_x_end_271x136; i++) {
		        if (logo_flag_map[j * 241 + i] == logo_flag_4circle_16_9_justscan_on_271x136[index]) {
		            sum++;
		        }
		        index++;
		    }
		}
		
		index = 0;
		sum1 = 0;
		for (j = logo_circle4_y_sta_271x136; j <= logo_circle4_y_end_271x136; j++) {
		    for (i = logo_circle4_x_sta_271x136; i <= logo_circle4_x_end_271x136; i++) {
		        if (logo_flag_map[j * 241 + i] == logo_flag_4circle_16_9_justscan_auto_271x136[index]) {
		            sum1++;
		        }
		        index++;
		    }
		}
		total = logo_circle4_h_271x136 * logo_circle4_v_271x136;
		th = (ratio * total) >> 10;

	}

	if (delay == 0) {
	    delay = 1;
	}

	if ((en & _BIT9) != 0) {
	    if (cnt % delay == 0) {
	        rtd_pr_memc_notice("sw_sld, sum=%d,%d,,th=%d,\n", sum, sum1, th);

	    }
	}

	cnt++;

	if ((sum > th) || (sum1 > th)) {
	    return 1;
	}
	else {
	    return 0;
	}

}

static void memc_logo_to_demura_apply(struct work_struct *work)
{
	int i, j, k;

	int wait_time = 1; //memc_logo_to_demura_wait_time * 120 / memc_logo_to_demura_update_cnt;
	int drop_time = 1; //memc_logo_to_demura_drop_time * 120 / memc_logo_to_demura_update_cnt;
	//int update_cnt = 1;
	int drop_buffer = 20;
	int drop_limit = memc_logo_to_demura_drop_limit; // test
	int drop_block = 0;
	int frm_rate = Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ);
	//unsigned int time_start, time_end;

	//update_cnt = (memc_logo_to_demura_update_cnt<=0)? 1 : memc_logo_to_demura_update_cnt;
	wait_time = (memc_logo_to_demura_wait_time * frm_rate) / 10;
	if( wait_time <= 0 )
		wait_time = 1;
	drop_time = (memc_logo_to_demura_drop_time * frm_rate) / 10;
	if( drop_time <= 0 )
		drop_time = 1;

	if( sld_work.read_ok )
	{
		//time_start = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);

		if( LSC_by_memc_logo_en == 0 ) // disable, demura table set to all 0, bypass all other flows
		{
			for( i=0; i<=270; i++ )
			{
				for( j=0; j<=480; j++ )
				{
					demura_tbl[0][i*481 + j] = 0;
					demura_tbl[1][i*481 + j] = 0;
					demura_tbl[2][i*481 + j] = 0;
				}
			}
		}
		else
		{
			// from raw memc logo map to 481x271 table
			for( i=0; i<270; i++ )
			{
				for( j=0; j<960/8; j++ )
				{
					unsigned char logo_byte1 = logo_flag_map_raw[(i*2  )*192 + j];
					unsigned char logo_byte2 = logo_flag_map_raw[(i*2+1)*192 + j];
					unsigned char logo_bit[2][8] = {{0}};
					unsigned char logo_flag_block;

					if( !memc_logo_read_en )
					{
						logo_byte1 = 0;
						logo_byte2 = 0;
					}

					for( k=0; k<8; k++ )
					{
						logo_bit[0][k] = (logo_byte1 & ( 1<<(7-k) )) ? 1 : 0;
						logo_bit[1][k] = (logo_byte2 & ( 1<<(7-k) )) ? 1 : 0;
						//logo_flag_map[i*960 + j + k] = ( (logo_byte & ( 1<<(7-k) )) != 0 );
					}
					for( k=0; k<4; k++ )
					{
						logo_flag_block = logo_bit[0][k*2] || logo_bit[0][k*2+1] || logo_bit[1][k*2] || logo_bit[1][k*2+1];
						logo_flag_map[i*481 + j*4 + k] = logo_flag_block;
					}
				}
				logo_flag_map[i*481 + 480] = logo_flag_map[i*481 + 479]; // last point repeat
			}
			for( j=0; j<=480; j++ ) // last line repeat
				logo_flag_map[270*481 + j] = logo_flag_map[269*481 + j];

			// memc logo map fill hole
			if( memc_logo_fill_hole_en )
			{
				//memcpy( logo_flag_map_buf, logo_flag_map, 481*271 );
				// logo_flag_map_buf: area selection
				// 0: background, 1: logo object, 2~: hole #
				int area_idx = 2;

				memset( is_background, 0, 10000 );
				memset( eq_table, 0, 10000*2*sizeof(unsigned short) );
				memset( eq_searched, 0, 10000 );
				is_background[0] = 1;

				// 1st pass: mark all 0 areas
				for( i=0; i<=270; i++ )
				{
					for( j=0; j<=480; j++ )
					{
						if( !logo_flag_map[i*481 + j] ) // not logo object
						{
							if( i==0 || j==0 ) // up/left border -> background
								logo_flag_map_buf[i*481 + j] = 0;
							else if( i==270 || j==480 ) // down/right border -> background + mark background
							{
								logo_flag_map_buf[i*481 + j] = 0;
								if( !logo_flag_map[(i-1)*481 + j] &&  logo_flag_map_buf[(i-1)*481 + j] != 0 ) // up
									is_background[ logo_flag_map_buf[(i-1)*481 + j] ] = 1;
								if( !logo_flag_map[i*481 + (j-1)] &&  logo_flag_map_buf[i*481 + (j-1)] != 0 ) // left
									is_background[ logo_flag_map_buf[i*481 + (j-1)] ] = 1;
							}
							else
							{
								// find neighbor color (4 dir)
								if( !logo_flag_map[(i-1)*481 + j] && !logo_flag_map[i*481 + (j-1)] ) // up & left
								{
									if( logo_flag_map_buf[(i-1)*481 + j] != logo_flag_map_buf[i*481 + j-1] ) // different color, add to eq table
									{
										if( logo_flag_map_buf[(i-1)*481 + j] == 0 ) // up is background
										{
											logo_flag_map_buf[i*481 + j] = logo_flag_map_buf[(i-1)*481 + j]; // use up
											is_background[ logo_flag_map_buf[i*481 + (j-1)] ] = 1;
										}
										else if( logo_flag_map_buf[i*481 + (j-1)] == 0 ) // left is background
										{
											logo_flag_map_buf[i*481 + j] = logo_flag_map_buf[i*481 + (j-1)]; // use left
											is_background[ logo_flag_map_buf[(i-1)*481 + j] ] = 1;
										}
										else
										{
											logo_flag_map_buf[i*481 + j] = logo_flag_map_buf[(i-1)*481 + j]; // use up
											eq_table_insert( logo_flag_map_buf[(i-1)*481 + j], logo_flag_map_buf[i*481 + (j-1)] );
										}
									}
									else
										logo_flag_map_buf[i*481 + j] = logo_flag_map_buf[i*481 + (j-1)]; // use left
								}
								else if( !logo_flag_map[(i-1)*481 + j] ) // up only
									logo_flag_map_buf[i*481 + j] = logo_flag_map_buf[(i-1)*481 + j]; // use up
								else if( !logo_flag_map[i*481 + (j-1)] ) // left only
									logo_flag_map_buf[i*481 + j] = logo_flag_map_buf[i*481 + (j-1)]; // use left
								else // new color index
								{
									if( area_idx >= 10000 ) // not support more than 10000 new regions
										logo_flag_map_buf[i*481 + j] = 0;
									else
									{
										logo_flag_map_buf[i*481 + j] = area_idx;
										area_idx++;
									}
								}
							}
						}
						else
							logo_flag_map_buf[i*481 + j] = 1; // logo object
					}
				}

				// check eq table for all background
				for( i=2; i<area_idx; i++ )
				{
					if( is_background[i] && !eq_searched[i] )
					{
						// trace list
						int prev = eq_table[i][0];
						int next = eq_table[i][1];

						while( prev != 0 )
						{
							is_background[ prev ] = 1;
							eq_searched[ prev ] = 1;
							prev = eq_table[prev][0];
						}
						while( next != 0 )
						{
							is_background[ next ] = 1;
							eq_searched[ next ] = 1;
							next = eq_table[next][1];
						}
					}
				}

				// 2nd pass: fill areas that are not background
				for( i=0; i<=270; i++ )
				{
					for( j=0; j<=480; j++ )
					{
						if( !logo_flag_map[i*481 + j] && !is_background[ logo_flag_map_buf[i*481 + j] ] )
						{
							logo_flag_map[i*481 + j] = 1; // fill hole
						}
					}
				}
			}

			// LD block average interpolation
			if( sld_work.LD_APL_valid )
			{
				short blk_x_l, blk_y_u;
				short blk_x_r, blk_y_d;
				short x, y;
				unsigned int apl00, apl01, apl10, apl11, intp0, intp1;
				int max_blk_avg = 0;

				// spatial max filter on block APL
				for( i=0; i<18; i++ )
				{
					for( j=0; j<32; j++ )
					{
						max_blk_avg = 0;
						for( x=-1; x<=1; x++ )
						{
							for( y=-1; y<=1; y++ )
							{
								blk_x_l = j+x;
								blk_x_l = MIN(blk_x_l, 31);
								blk_x_l = MAX(blk_x_l, 0);

								blk_y_u = i+y;
								blk_y_u = MIN(blk_y_u, 17);
								blk_y_u = MAX(blk_y_u, 0);

								if( blk_apl_average[ blk_y_u * 32 + blk_x_l ] > max_blk_avg )
									max_blk_avg = blk_apl_average[ blk_y_u * 32 + blk_x_l ];
							}
						}

						blk_apl_maxfilter[ i*32+j ] = max_blk_avg;
					}
				}

				for( i=0; i<=270; i++ )
				{
					for( j=0; j<=480; j++ )
					{
						blk_x_l = (j-7)/15;
						blk_x_l = MIN(blk_x_l, 31);
						blk_x_l = MAX(blk_x_l, 0);

						blk_x_r = (j+7)/15;
						blk_x_r = MIN(blk_x_r, 31);
						blk_x_r = MAX(blk_x_r, 0);

						blk_y_u = (i-7)/15;
						blk_y_u = MIN(blk_y_u, 17);
						blk_y_u = MAX(blk_y_u, 0);

						blk_y_d = (i+7)/15;
						blk_y_d = MIN(blk_y_d, 17);
						blk_y_d = MAX(blk_y_d, 0);

						x = (j-7) % 15;
						y = (i-7) % 15;

						apl00 = blk_apl_maxfilter[ blk_y_u * 32 + blk_x_l ];
						apl01 = blk_apl_maxfilter[ blk_y_u * 32 + blk_x_r ];
						apl10 = blk_apl_maxfilter[ blk_y_d * 32 + blk_x_l ];
						apl11 = blk_apl_maxfilter[ blk_y_d * 32 + blk_x_r ];

						intp0 = ( apl00 * (15-x) + apl01 * x + 7 ) / 15;
						intp1 = ( apl10 * (15-x) + apl11 * x + 7 ) / 15;

						blk_apl_interp[i*481 + j] = ( intp0 * (15-y) + intp1 * y + 7 ) / 15;
					}
				}
			}
			else
			{
				for( i=0; i<=270; i++ )
				{
					for( j=0; j<=480; j++ )
					{
						blk_apl_interp[i*481+j] = 0;
					}
				}
			}

			for( i=0; i<=270; i++ )
			{
				for( j=0; j<=480; j++ )
				{
					unsigned int LD_APL;
					LD_APL = blk_apl_interp[i*481+j];

					if( sld_work.do_reset_full )
					{
						logo_demura_counter[i*481 + j] = 0;
					}
					else if( sld_work.do_counter_update )
					{
						if( logo_flag_map[i*481 + j] )
						{
							logo_demura_counter[i*481 + j]++;
							if( logo_demura_counter[i*481 + j] >= wait_time + drop_time + drop_buffer )
								logo_demura_counter[i*481 + j] = wait_time + drop_time + drop_buffer;
						}
						else
						{
							if( sld_work.do_reset_counter )
							{
								logo_demura_counter[i*481 + j] = 0;
							}
							else
							{
								logo_demura_counter[i*481 + j] -= memc_logo_to_demura_ascend_speed;
								if( logo_demura_counter[i*481 + j] < 0 )
									logo_demura_counter[i*481 + j] = 0;
							}
						}
					}

					if( logo_demura_counter[i*481 + j] < wait_time )
						drop_block = 0;
					else if( logo_demura_counter[i*481 + j] > wait_time + drop_time )
						drop_block = drop_limit;
					else
						drop_block = drop_limit * (logo_demura_counter[i*481 + j] - wait_time) / drop_time;

					if( LD_APL > 940 - drop_block ) // decrease counter on too bright part to avoid luminance inversion
					{
						drop_block = 940 - LD_APL;
						if( drop_block<0 )
							drop_block = 0;
						logo_demura_counter[i*481 + j] -= memc_logo_to_demura_ascend_speed;
						if( drop_limit <= 0 && logo_demura_counter[i*481 + j] < 0 )
							logo_demura_counter[i*481 + j] = 0;
						else if( logo_demura_counter[i*481 + j] < (drop_block * drop_time / drop_limit + wait_time) )
							logo_demura_counter[i*481 + j] = (drop_block * drop_time / drop_limit + wait_time);
					}

					if( (j>=sld_work.osd_sta_blkx) && (j<=sld_work.osd_end_blkx) && (i>=sld_work.osd_sta_blky) && (i<=sld_work.osd_end_blky) )
					{
						// osd part, bypass
						demura_tbl[0][i*481 + j] = 0;
						demura_tbl[1][i*481 + j] = 0;
						demura_tbl[2][i*481 + j] = 0;
					}
					else
					{
						demura_tbl[0][i*481 + j] = 0;
						demura_tbl[1][i*481 + j] = 0 - (drop_block >> 2);
						demura_tbl[2][i*481 + j] = 0 - (drop_block >> 2);
					}
				}
			}

			// demura table filter
			if( memc_logo_filter_en )
			{
				// horizontal [1 2 1] filter
				short filter_sum = 0;
				for( i=0; i<=270; i++ )
				{
					for( j=0; j<=480; j++ )
					{
						int j_left = (j==0) ? 0 : j-1;
						int j_right = (j==480) ? 480 : j+1;
						filter_sum = (demura_tbl[2][i*481 + j_left] + demura_tbl[2][i*481 + j]*2 + demura_tbl[2][i*481 + j_right]) >> 2;

						demura_tbl[1][i*481+j] = filter_sum;
					}
				}
				memcpy( demura_tbl[2], demura_tbl[1], 481*271*sizeof(short) );

				// vertical [1 2 1] filter
				for( i=0; i<=270; i++ )
				{
					for( j=0; j<=480; j++ )
					{
						int i_up = (i==0) ? 0 : i-1;
						int i_down = (i==270) ? 270 : i+1;
						filter_sum = (demura_tbl[2][i_up*481 + j] + demura_tbl[2][i*481 + j]*2 + demura_tbl[2][i_down*481 + j]) >> 2;

						demura_tbl[1][i*481+j] = filter_sum;
					}
				}
				memcpy( demura_tbl[2], demura_tbl[1], 481*271*sizeof(short) );
			}
		}

		// check the demura table value
		for( i=0; i<=270; i++ )
		{
			for( j=1; j<=480; j++ )
			{
				int PtsDiff = demura_tbl[2][i*481+j] - demura_tbl[2][i*481+j-1];
				PtsDiff = (PtsDiff >  31)?  31 : PtsDiff;
				PtsDiff = (PtsDiff < -32)? -32 : PtsDiff;
				demura_tbl[2][i*481+j] = demura_tbl[2][i*481+j-1] + PtsDiff;
			}
		}
		memcpy( demura_tbl[1], demura_tbl[2], 481*271*sizeof(short) );

		// encode to demura table
		if( logo_to_demura_en && !demura_write_en )
		{

			fwif_color_DeMura_encode((short*)(&(demura_tbl[0][0])), DeMura_TBL_481x271, 0, 3, &DeMura_TBL);

			demura_write_en = true;
		}

		sld_work.read_ok = false;

		//time_end = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
		//rtd_pr_vpq_emerg("apply time: %d on 90k clock\n", (time_end-time_start));

	}

}

char memc_logo_to_RL_LOGO_detect(void)
{
#define L_x_sta 0
#define L_x_end 110

#define R_x_sta 370
#define R_x_end 480

#define y_sta 50
#define y_end 240

    unsigned short L_th = 14000;    //  total is 20900, (L_x_end-L_x_sta) * (y_end - y_sta);
    unsigned short R_th = 14000;    //  total is 20900, (R_x_end-R_x_sta) * (y_end - y_sta);

    //static unsigned char SLD_en = 1;
    //  unsigned char sld_en_check;
    unsigned int i, j;
	int il=0;
	int cj1=481;
    unsigned int LR_LOGO_CNT_R = 0;
    unsigned int LR_LOGO_CNT_L = 0;
    //unsigned char LR_LOGO_Flag_R = 0;
    //unsigned char LR_LOGO_Flag_L = 0;
    unsigned short en = sld_work_debug.pr_En;
    unsigned short delay = sld_work_debug.pr_delay;
    static unsigned int cnt;
    //color_temp_color_temp_contrast_0_RBUS color_temp_color_temp_contrast_0;
    //color_sld_sld_hpf_1_RBUS sld_hpf_1_RBUS;
    //color_temp_color_temp_contrast_0.regValue = IoReg_Read32(COLOR_TEMP_Color_Temp_Contrast_0_reg);
	
	if(DeMura_TBL.table_mode==DeMura_TBL_481x271)
	{
		cj1=481;
	}
	else
	// DeMura_TBL_241x136
	{
		cj1=241;
	}

    // other detect for hw sld
    LR_LOGO_CNT_R = 0;
    LR_LOGO_CNT_L = 0;
    //LR_LOGO_Flag_R = 0;
    //LR_LOGO_Flag_L = 0;

    // LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
    for (i = y_sta; i < y_end; i++) {
		il=i*cj1;
        for (j = L_x_sta; j < L_x_end; j++)
            if (logo_flag_map[il + j] == 1) {
                LR_LOGO_CNT_L++;
            }
    }
    if (LR_LOGO_CNT_L > L_th) {
        LR_LOGO_Flag_L = 1;
    }
    else {
        LR_LOGO_Flag_L = 0;
    }

    // RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR
    for (i = y_sta; i < y_end; i++) {
		il=i*cj1;
        for (j = R_x_sta; j < R_x_end; j++)
            if (logo_flag_map[il + j] == 1) {
                LR_LOGO_CNT_R++;
            }
    }
    if (LR_LOGO_CNT_R > R_th) {
        LR_LOGO_Flag_R = 1;
    }
    else {
        LR_LOGO_Flag_R = 0;
    }

    //if (color_temp_color_temp_contrast_0.contrast_g < 2000) {
        //LR_LOGO_Flag_L = 1;
        //LR_LOGO_Flag_R = 1;
        //rtd_pr_memc_notice("sw_sld,LR_LOGO_Flag_L and R is %d,%d\n", LR_LOGO_Flag_L, LR_LOGO_Flag_R);
   // }

    //if (SLD_en != sld_en_check) {
    //drvif_color_set_SLD_Enable(sld_en_check);
    ///SLD_en =  sld_en_check;
    //}

    if (delay == 0) {
        delay = 1;
    }

    if ((en & _BIT9) != 0) {
        if (cnt % delay == 0) {
            rtd_pr_memc_notice("sw_sld,L_cnt=%d,th=%d,R_cnt=%d,th=%d,,,,%d,%d,\n", 
				LR_LOGO_CNT_L, L_th, LR_LOGO_CNT_R, R_th, LR_LOGO_Flag_L, LR_LOGO_Flag_R);

        }
    }
    cnt++;

    return 0;
}

unsigned short sld_work_filter_5[81]=
{
	0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,
	0,0,2,4,4,4,2,0,0,
	0,0,4,8,8,8,4,0,0,
	0,0,4,8,8,8,4,0,0,
	0,0,4,8,8,8,4,0,0,
	0,0,2,4,4,4,2,0,0,
	0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,
};
unsigned short sld_work_filter_10[81]=
{
	1,1,1,1,1,1,1,1,1,
	1,1,2,2,2,2,2,2,1,
	1,2,2,2,2,2,2,2,1,
	1,2,2,2,2,2,2,2,1,
	1,2,2,2,2,2,2,2,1,
	1,2,2,2,2,2,2,2,1,
	1,2,2,2,2,2,2,2,1,
	1,2,2,2,2,2,2,1,1,
	1,1,1,1,1,1,1,1,1,
};

sld_work_filter_struct sld_work_filter = {
	//unsigned short filter_w[demura_filter_size*demura_filter_size];
	{
		1,1,1,1,1,1,1,1,1,
		1,1,2,2,2,2,2,2,1,
		1,2,2,2,2,2,2,2,1,
		1,2,2,2,2,2,2,2,1,
		1,2,2,2,2,2,2,2,1,
		1,2,2,2,2,2,2,2,1,
		1,2,2,2,2,2,2,2,1,
		1,2,2,2,2,2,2,1,1,
		1,1,1,1,1,1,1,1,1,

	},
	//	unsigned short filter_shift;
	7,
	//unsigned short filter_gain
	1096,
};


void memc_logo_to_demura_init(void)
{
    if (0/*strcmp(webos_strToolOption.eBackLight, "oled") == 0*/) {
        fwif_color_set_LDSetLUT(0);
        sld_work.do_counter_update = true;
        sld_work.do_reset_counter = false;
        sld_work.LD_APL_valid = false;
        sld_work.read_ok = false;
        sld_work.osd_sta_blkx = -1;
        sld_work.osd_sta_blky = -1;
        sld_work.osd_end_blkx = -1;
        sld_work.osd_end_blky = -1;
#ifdef CONFIG_SUPPORT_SLD_BY_MEMC_DEMURA_LOGO_ONLY
        SLD_BY_MEMC_DEMURA_LOGO_ONLY = 1;
#else
        memcpy(&DeMura_TBL.DeMura_CTRL_TBL, &DeMura_CTRL_TBL_SW_SLD, sizeof(DRV_DeMura_CTRL_TBL));
        DeMura_TBL.table_mode = DeMura_TBL_241x136;//DeMura_TBL_481x271;//
        SLD_BY_MEMC_DEMURA_LOGO_ONLY = 0;
#endif
        SLD_SW_En = 1;
		MEMC_Pixel_LOGO_For_SW_SLD = 1;
		// 0xff: no filter, 0:sw filter by cpu with ave filter(optimized), 1~200: SE filter, iteration times, 254:filter by cpu with normal filter
		memc_logo_filter_en = 10; 
        INIT_WORK(&(sld_work.sld_apply_work), memc_logo_to_demura_apply);
    }
    else {
        SLD_SW_En = 0;
		MEMC_Pixel_LOGO_For_SW_SLD = 0;
    }


    rtd_pr_vpq_emerg("SLD_SW, ini done, SLD_SW_En=%d,\n", SLD_SW_En);
}


int fwif_color_sld(void)
{
    //static char sld_set_DMA = 0;
    //static char sld_set_default = 0;
    color_sld_sld_control_RBUS color_sld_control;
    //static int value_dummy1802c700_31_22 = 0;

    color_sld_control.regValue = IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg);
    //color_sld_control.dummy1802c700_15_8 = 1; 
    //IoReg_Write32(COLOR_SLD_SLD_CONTROL_reg, color_sld_control.regValue); 
    //value_dummy1802c700_31_22 = drvif_color_get_SLD_Dummy();

#if 0
    /*memc*/
    rtd_setbits(0xb809d5fc, 0x10000000);//memc

    WriteRegister(0xb809d0A0, 0, s7, 0x0ff);
    WriteRegister(0xb809d0A0, 8, 15, 0xff);
    WriteRegister(0xb809d0A0, 16, 23, 0xff);
    WriteRegister(0xb809d0A0, 24, 31, 0xff);

    WriteRegister(0xb809d0A4, 0, 7, 0xff);
    WriteRegister(0xb809d0A4, 8, 15, 0xff);
    WriteRegister(0xb809d0A4, 16, 23, 0xff);
    WriteRegister(0xb809d0A4, 24, 31, 0xff);

    LC_ON = drvif_color_get_LC_ONOFF();
    if (!LC_ON) {
        drvif_color_set_SLD_global_drop();
    }
    else {
        drvif_color_set_SLD_global_NOT_drop();
    }
#endif

	#if 0
    //if(1){
    if (value_dummy1802c700_31_22 == 1) {
        //if(1){
        //if(value_dummy1802c700_31_22 >= 4){
        drvif_color_od_disable_set(0);
        drvif_color_set_SLD_4k_dma_setting_K7();
        sld_set_DMA = 1;//delete
        drvif_color_set_SLD_Enable(1);
    }

    //if(value_dummy1802c700_31_22 == 3){
    if (value_dummy1802c700_31_22 == 3) {
        //fwif_color_set_SLD_LCblk_inform();
        //fwif_color_SLD_apply_normal_setting();
        sld_demo();

        //drvif_color_set_SLD_Enable(1);
        sld_set_default = 1;//delete
    }

    if ((value_dummy1802c700_31_22 == 4) || (value_dummy1802c700_31_22 == 12)) {
        /*memc*/
        //rtd_setbits(0xb809d5fc, 0x10000000);//memc
        //WriteRegister(0xb809d0A4, 8, 15, 0xee);

        fwif_color_compute_LC_APL_related_FW();
    }
	#endif
	if(SLD_DMA_Ready){
		//fwif_color_compute_LC_APL_related_FW();
		fwif_color_compute_APL_related_FW_New();
		return 1;
	}
	else 
		return -1;
}



#if 0
void memc_logo_to_demura_gain(void)
{
	int i, j, k;
	static unsigned int frame_cnt = 0;
	unsigned int ddr_offset = 1504;	// de-garbage
	unsigned char* logo_ptr = LD_virAddr + ddr_offset;

	int wait_time = memc_logo_to_demura_wait_time * 120 / memc_logo_to_demura_update_cnt; // test
	int drop_time = memc_logo_to_demura_drop_time * 120 / memc_logo_to_demura_update_cnt; // test
	int drop_buffer = 20;
	int drop_limit = memc_logo_to_demura_drop_limit; // test
	int drop_block = 0;

	static unsigned char memc_sc_flag = false;
	static unsigned int  memc_sc_motion1 = 0, memc_sc_motion2 = 0;
	static unsigned char do_counter_update = true;
	static unsigned char do_reset_counter = false;

	//unsigned int time_start, time_end;

	// OSD mixer detect
	static unsigned char osd_measure_init = false;
	osdovl_mixer_ctrl2_RBUS osdovl_mixer_ctrl2_reg;
	osdovl_osd_db_ctrl_RBUS osdovl_osd_db_ctrl_reg;
	osdovl_measure_osd1_sta_RBUS osdovl_measure_osd1_sta_reg;
	osdovl_measure_osd1_end_RBUS osdovl_measure_osd1_end_reg;
	short osd_sta_x, osd_sta_y, osd_end_x, osd_end_y;
	short osd_sta_blkx, osd_sta_blky, osd_end_blkx, osd_end_blky;

	// LD block average
	blu_ld_global_ctrl2_RBUS ld_global_ctrl2_reg;
	unsigned char LD_valid, LD_type, LD_Hnum, LD_Vnum;
	unsigned char LD_APL_valid;

	if( !osd_measure_init )
	{
		osdovl_mixer_ctrl2_reg.regValue = IoReg_Read32(OSDOVL_Mixer_CTRL2_reg);
		osdovl_osd_db_ctrl_reg.regValue = IoReg_Read32(OSDOVL_OSD_Db_Ctrl_reg);

		osdovl_mixer_ctrl2_reg.measure_osd_zone_en = 1;
		osdovl_mixer_ctrl2_reg.measure_osd_zone_type = 1;
		IoReg_Write32( OSDOVL_Mixer_CTRL2_reg, osdovl_mixer_ctrl2_reg.regValue );

		osdovl_osd_db_ctrl_reg.db_load = 1;
		IoReg_Write32( OSDOVL_OSD_Db_Ctrl_reg, osdovl_osd_db_ctrl_reg.regValue );

		osd_measure_init = true;
	}

	osdovl_measure_osd1_sta_reg.regValue = IoReg_Read32(OSDOVL_measure_osd1_sta_reg);
	osdovl_measure_osd1_end_reg.regValue = IoReg_Read32(OSDOVL_measure_osd1_end_reg);
	osd_sta_x = osdovl_measure_osd1_sta_reg.x;
	osd_sta_y = osdovl_measure_osd1_sta_reg.y;
	osd_end_x = osdovl_measure_osd1_end_reg.x;
	osd_end_y = osdovl_measure_osd1_end_reg.y;

	if( osd_sta_x == 0x1fff && osd_sta_y == 0x1fff && osd_end_x == 0 && osd_end_y == 0 ) // no OSD shown
		osd_sta_blkx = osd_sta_blky = osd_end_blkx = osd_end_blky = -1;
	else // mark osd blocks
	{
		osd_sta_blkx = osd_sta_x / 8;
		osd_end_blkx = (osd_end_x+7) / 8;
		osd_sta_blky = osd_sta_y / 8;
		osd_end_blky = (osd_end_y+7) / 8;
	}


	if( LSC_by_memc_logo_en == 0 ) // disable, demura table set to all 0, bypass all other flows
	{
		for( i=0; i<=270; i++ )
		{
			for( j=0; j<=480; j++ )
			{
				demura_tbl_hi[i*481 + j] = 0;
				demura_tbl_md[i*481 + j] = 0;
				demura_tbl_lo[i*481 + j] = 0;
			}
		}
	}
	else
	{

		if( MEMC_Lib_GetInfoForSLD(1) ) // scene change flag
		{
			memc_sc_flag= true;
		}
		if( MEMC_Lib_GetInfoForSLD(4) > memc_sc_motion1 )
			memc_sc_motion1 = MEMC_Lib_GetInfoForSLD(4);
		if( MEMC_Lib_GetInfoForSLD(5) > memc_sc_motion2 )
			memc_sc_motion2 = MEMC_Lib_GetInfoForSLD(5);

		if( memc_logo_to_demura_update_cnt == 0 )
			memc_logo_to_demura_update_cnt = 1;
		if( frame_cnt % memc_logo_to_demura_update_cnt == 0 ) // phase 1: get logo
		{
			//time_start = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
			if( memc_logo_read_en && LD_virAddr != NULL && MEMC_Lib_GetInfoForSLD(7) )
			{

				//rtd_pr_vpq_emerg("get memc_logo from addr 0x%08x\n", (unsigned int)logo_ptr);

				for( i=0; i<270; i++ )
				{
					memcpy(logo_byte_line[0], logo_ptr, 960/8);
					logo_ptr += 192;
					memcpy(logo_byte_line[1], logo_ptr, 960/8);
					logo_ptr += 192;

					for( j=0; j<960/8; j++ )
					{
						unsigned char logo_byte1 = logo_byte_line[0][j];
						unsigned char logo_byte2 = logo_byte_line[1][j];
						unsigned char logo_bit[2][8] = {0};
						unsigned char logo_flag_block;
						for( k=0; k<8; k++ )
						{
							logo_bit[0][k] = (logo_byte1 & ( 1<<(7-k) )) ? 1 : 0;
							logo_bit[1][k] = (logo_byte2 & ( 1<<(7-k) )) ? 1 : 0;
							//logo_flag_map[i*960 + j + k] = ( (logo_byte & ( 1<<(7-k) )) != 0 );
						}
						for( k=0; k<4; k++ )
						{
							logo_flag_block = logo_bit[0][k*2] || logo_bit[0][k*2+1] || logo_bit[1][k*2] || logo_bit[1][k*2+1];
							logo_flag_map[i*481 + j*4 + k] = logo_flag_block;
						}
					}
					logo_flag_map[i*481 + 480] = logo_flag_map[i*481 + 479]; // last point repeat
				}
				for( j=0; j<=480; j++ ) // last line repeat
					logo_flag_map[270*481 + j] = logo_flag_map[269*481 + j];

				do_counter_update = true;
				do_reset_counter = false;
				//if( MEMC_Lib_GetInfoForSLD(0) != 0 ) // pixel logo clear signal
				//	do_counter_update = false;
				if( memc_sc_motion1 < 100 && memc_sc_motion2 < 100 ) // motion cnt low: picture pause
					do_counter_update = false;
				else if( memc_sc_flag && memc_sc_motion1 > 10000 && memc_sc_motion2 > 10000 ) // scene change flag + big motion: scene change
				{
					do_counter_update = true;
					do_reset_counter = true;
				}

				// fill hole
				if( memc_logo_fill_hole_en )
				{
					//memcpy( logo_flag_map_buf, logo_flag_map, 481*271 );
					// logo_flag_map_buf: area selection
					// 0: background, 1: logo object, 2~: hole #
					int area_idx = 2;

					memset( is_background, 0, 10000 );
					memset( eq_table, 0, 10000*2*sizeof(unsigned short) );
					memset( eq_searched, 0, 10000 );
					is_background[0] = 1;

					// 1st pass: mark all 0 areas
					for( i=0; i<=270; i++ )
					{
						for( j=0; j<=480; j++ )
						{
							if( !logo_flag_map[i*481 + j] ) // not logo object
							{
								if( i==0 || j==0 ) // up/left border -> background
									logo_flag_map_buf[i*481 + j] = 0;
								else if( i==270 || j==480 ) // down/right border -> background + mark background
								{
									logo_flag_map_buf[i*481 + j] = 0;
									if( !logo_flag_map[(i-1)*481 + j] &&  logo_flag_map_buf[(i-1)*481 + j] != 0 ) // up
										is_background[ logo_flag_map_buf[(i-1)*481 + j] ] = 1;
									if( !logo_flag_map[i*481 + (j-1)] &&  logo_flag_map_buf[i*481 + (j-1)] != 0 ) // left
										is_background[ logo_flag_map_buf[i*481 + (j-1)] ] = 1;
								}
								else
								{
									// find neighbor color (4 dir)
									if( !logo_flag_map[(i-1)*481 + j] && !logo_flag_map[i*481 + (j-1)] ) // up & left
									{
										if( logo_flag_map_buf[(i-1)*481 + j] != logo_flag_map_buf[i*481 + j-1] ) // different color, add to eq table
										{
											if( logo_flag_map_buf[(i-1)*481 + j] == 0 ) // up is background
											{
												logo_flag_map_buf[i*481 + j] = logo_flag_map_buf[(i-1)*481 + j]; // use up
												is_background[ logo_flag_map_buf[i*481 + (j-1)] ] = 1;
											}
											else if( logo_flag_map_buf[i*481 + (j-1)] == 0 ) // left is background
											{
												logo_flag_map_buf[i*481 + j] = logo_flag_map_buf[i*481 + (j-1)]; // use left
												is_background[ logo_flag_map_buf[(i-1)*481 + j] ] = 1;
											}
											else
											{
												logo_flag_map_buf[i*481 + j] = logo_flag_map_buf[(i-1)*481 + j]; // use up
												eq_table_insert( logo_flag_map_buf[(i-1)*481 + j], logo_flag_map_buf[i*481 + (j-1)] );
											}
										}
										else
											logo_flag_map_buf[i*481 + j] = logo_flag_map_buf[i*481 + (j-1)]; // use left
									}
									else if( !logo_flag_map[(i-1)*481 + j] ) // up only
										logo_flag_map_buf[i*481 + j] = logo_flag_map_buf[(i-1)*481 + j]; // use up
									else if( !logo_flag_map[i*481 + (j-1)] ) // left only
										logo_flag_map_buf[i*481 + j] = logo_flag_map_buf[i*481 + (j-1)]; // use left
									else // new color index
									{
										if( area_idx >= 10000 ) // not support more than 10000 new regions
											logo_flag_map_buf[i*481 + j] = 0;
										else
										{
											logo_flag_map_buf[i*481 + j] = area_idx;
											area_idx++;
										}
									}
								}
							}
							else
								logo_flag_map_buf[i*481 + j] = 1; // logo object
						}
					}

					// check eq table for all background
					for( i=2; i<area_idx; i++ )
					{
						if( is_background[i] && !eq_searched[i] )
						{
							// trace list
							int prev = eq_table[i][0];
							int next = eq_table[i][1];

							while( prev != 0 )
							{
								is_background[ prev ] = 1;
								eq_searched[ prev ] = 1;
								prev = eq_table[prev][0];
							}
							while( next != 0 )
							{
								is_background[ next ] = 1;
								eq_searched[ next ] = 1;
								next = eq_table[next][1];
							}
						}
					}

					// 2nd pass: fill areas that are not background
					for( i=0; i<=270; i++ )
					{
						for( j=0; j<=480; j++ )
						{
							if( !logo_flag_map[i*481 + j] && !is_background[ logo_flag_map_buf[i*481 + j] ] )
							{
								logo_flag_map[i*481 + j] = 1; // fill hole
							}
						}
					}
				}

			}
			else
			{
				do_counter_update = true;
				do_reset_counter = true;
				memset( logo_flag_map, 0, 481*271 );
			}
			memc_sc_flag = false; // reset SC flag after logo flag update
			memc_sc_motion1 = 0;
			memc_sc_motion2 = 0;

			//time_end = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
			//rtd_pr_vpq_emerg("phase1 time: %d on 90k clock\n", (time_end-time_start));

		}

		if( frame_cnt % memc_logo_to_demura_update_cnt == memc_logo_to_demura_update_cnt/4 ) // phase 2: get LD info
		{
			//time_start = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);

			// LD related info
			ld_global_ctrl2_reg.regValue = IoReg_Read32(BLU_LD_Global_Ctrl2_reg);
			LD_valid = ld_global_ctrl2_reg.ld_valid;
			LD_type = ld_global_ctrl2_reg.ld_blk_type;
			LD_Hnum = ld_global_ctrl2_reg.ld_blk_hnum + 1;
			LD_Vnum = ld_global_ctrl2_reg.ld_blk_vnum + 1;

			if( LD_valid && LD_type == 0 && LD_Hnum == 32 && LD_Vnum == 18 )
			{
				LD_APL_valid = drvif_color_get_LD_APL_ave( blk_apl_average );
			}
			else
			{
				LD_APL_valid = false;
			}

			if( LD_APL_valid )
			{
				short blk_x_l, blk_y_u;
				short blk_x_r, blk_y_d;
				short x, y;
				unsigned int apl00, apl01, apl10, apl11, intp0, intp1;
				for( i=0; i<=270; i++ )
				{
					for( j=0; j<=480; j++ )
					{
						blk_x_l = (j-7)/15;
						blk_x_l = MIN(blk_x_l, 31);
						blk_x_l = MAX(blk_x_l, 0);

						blk_x_r = (j+7)/15;
						blk_x_r = MIN(blk_x_r, 31);
						blk_x_r = MAX(blk_x_r, 0);

						blk_y_u = (i-7)/15;
						blk_y_u = MIN(blk_y_u, 17);
						blk_y_u = MAX(blk_y_u, 0);

						blk_y_d = (i+7)/15;
						blk_y_d = MIN(blk_y_d, 17);
						blk_y_d = MAX(blk_y_d, 0);

						x = (j-7) % 15;
						y = (i-7) % 15;

						apl00 = blk_apl_average[ blk_y_u * 32 + blk_x_l ];
						apl01 = blk_apl_average[ blk_y_u * 32 + blk_x_r ];
						apl10 = blk_apl_average[ blk_y_d * 32 + blk_x_l ];
						apl11 = blk_apl_average[ blk_y_d * 32 + blk_x_r ];

						intp0 = ( apl00 * (15-x) + apl01 * x + 7 ) / 15;
						intp1 = ( apl10 * (15-x) + apl11 * x + 7 ) / 15;

						blk_apl_interp[i*481 + j] = ( intp0 * (15-y) + intp1 * y + 7 ) / 15;
					}
				}
			}
			else
			{
				for( i=0; i<=270; i++ )
				{
					for( j=0; j<=480; j++ )
					{
						blk_apl_interp[i*481+j] = 0;
					}
				}
			}

			//time_end = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
			//rtd_pr_vpq_emerg("phase2 time: %d on 90k clock\n", (time_end-time_start));
		}

		if( frame_cnt % memc_logo_to_demura_update_cnt == memc_logo_to_demura_update_cnt*2/4 ) // phase 3: update counter
		{
			//time_start = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);

			if( logo_to_demura_en )
			{
				for( i=0; i<=270; i++ )
				{
					for( j=0; j<=480; j++ )
					{
						unsigned int LD_APL;
						LD_APL = blk_apl_interp[i*481+j];

						if( do_counter_update )
						{
							if( logo_flag_map[i*481 + j] )
							{
								logo_demura_counter[i*481 + j]++;
								if( logo_demura_counter[i*481 + j] >= wait_time + drop_time + drop_buffer )
									logo_demura_counter[i*481 + j] = wait_time + drop_time + drop_buffer;
							}
							else
							{
								if( do_reset_counter )
								{
									logo_demura_counter[i*481 + j] = 0;
								}
								else
								{
									logo_demura_counter[i*481 + j] -= memc_logo_to_demura_ascend_speed;
									if( logo_demura_counter[i*481 + j] < 0 )
										logo_demura_counter[i*481 + j] = 0;
								}
							}
						}

						drop_block = drop_limit * (logo_demura_counter[i*481 + j] - wait_time) / drop_time;
						if( LD_APL > 1024 - drop_block ) // decrease counter on too bright part to avoid luminance inversion
						{
							if( do_reset_counter )
								logo_demura_counter[i*481 + j] = 0;
							else
								logo_demura_counter[i*481 + j] -= memc_logo_to_demura_ascend_speed;
						}
						if( logo_demura_counter[i*481 + j] < 0 )
							logo_demura_counter[i*481 + j] = 0;

						if( logo_demura_counter[i*481 + j] < wait_time )
							drop_block = 0;
						else if( logo_demura_counter[i*481 + j] > wait_time + drop_time )
							drop_block = drop_limit;
						else
							drop_block = drop_limit * (logo_demura_counter[i*481 + j] - wait_time) / drop_time;
						//*/

						//drop_block = block_logo_flag[1][1] ? drop_limit : 0;

						if( (j>=osd_sta_blkx) && (j<=osd_end_blkx) && (i>=osd_sta_blky) && (i<=osd_end_blky) )
						{
							// osd part, bypass
							demura_tbl_hi[i*481 + j] = 0;
							demura_tbl_md[i*481 + j] = 0;
							demura_tbl_lo[i*481 + j] = 0;
						}
						else
						{
							demura_tbl_hi[i*481 + j] = 0 - (drop_block >> 2);
							demura_tbl_md[i*481 + j] = 0 - (drop_block >> 2);
							demura_tbl_lo[i*481 + j] = 0;
						}
					}
				}

				if( memc_logo_filter_en )
				{
					// horizontal [1 2 1] filter
					short filter_sum = 0;
					for( i=0; i<=270; i++ )
					{
						for( j=0; j<=480; j++ )
						{
							int j_left = (j==0) ? 0 : j-1;
							int j_right = (j==480) ? 480 : j+1;
							filter_sum = (demura_tbl_hi[i*481 + j_left] + demura_tbl_hi[i*481 + j]*2 + demura_tbl_hi[i*481 + j_right]) >> 2;

							demura_tbl_md[i*481+j] = filter_sum;
						}
					}
					memcpy( demura_tbl_hi, demura_tbl_md, 481*271*sizeof(short) );

					// vertical [1 2 1] filter
					for( i=0; i<=270; i++ )
					{
						for( j=0; j<=480; j++ )
						{
							int i_up = (i==0) ? 0 : i-1;
							int i_down = (i==270) ? 270 : i+1;
							filter_sum = (demura_tbl_hi[i_up*481 + j] + demura_tbl_hi[i*481 + j]*2 + demura_tbl_hi[i_down*481 + j]) >> 2;

							demura_tbl_md[i*481+j] = filter_sum;
						}
					}
					memcpy( demura_tbl_hi, demura_tbl_md, 481*271*sizeof(short) );
				}

			}
			//time_end = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
			//rtd_pr_vpq_emerg("phase3 time: %d on 90k clock\n", (time_end-time_start));
		}
	}

	if( frame_cnt % memc_logo_to_demura_update_cnt == memc_logo_to_demura_update_cnt/4*3 ) // phase 4: encode demura table
	{
		//time_start = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
		if( logo_to_demura_en )
		{

			fwif_color_DeMura_encode(demura_tbl_lo, demura_tbl_md, demura_tbl_hi, DeMura_TBL_481x271, 0, DeMura_TBL.TBL);

			demura_write_en = true;
		}
		//time_end = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
		//rtd_pr_vpq_emerg("phase4 time: %d on 90k clock\n", (time_end-time_start));
	}
	frame_cnt++;

}
#endif
#endif
