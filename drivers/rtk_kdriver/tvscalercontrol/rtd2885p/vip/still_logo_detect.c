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
 * @date    $Date$
 * @version     $Revision$
 */

/*============================ Module dependency  ===========================*/
#include <linux/fs.h>
#include <linux/workqueue.h>
#include <linux/string.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <linux/kernel.h>
#include "tvscalercontrol/vip/still_logo_detect.h"
#include <tvscalercontrol/scaler/vipinclude.h> // for typedef that used in VIP_reg_def.h
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <rtd_log/rtd_module_log.h>

#ifdef CONFIG_SCALER_ENABLE_V4L2
#include <linux/v4l2-ext/videodev2-ext.h>
#endif
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
#include <scaler/vipRPCCommon.h>
#include "vgip_isr/scalerVIP.h"

#include <rtk_kadp_se.h>
#include <rbus/se_reg.h>
#include "tvscalercontrol/vip/GSR2.h"

#undef VIPprintf
#define VIPprintf(fmt, args...) VIPprintfPrivate(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_GIBI_OD_DEBUG,fmt,##args)

#define SLD_Debug 0
#define SLD_MEMC_DISPLAY
#define Local_Gain_Tbl_NUM 1
#ifndef ABS_
    #define ABS_(x)   ((x) >= 0 ? (x) : -(x))
#endif

#define SLD_ABS(x,y)  ((x > y) ? (x-y) : (y-x))
#define SLD_CLIP(min, max, val) ((val < min) ? min : (val > max ? max : val))



#define ceil_SLD(a,b) (a/b+(a%b)!=0)


//unsigned int APL_pre[LC_MAX_H_NUM * LC_MAX_V_NUM] = {0};
//unsigned int APL_cur[LC_MAX_H_NUM * LC_MAX_V_NUM] = {0};
//unsigned int APL_delta[LC_MAX_H_NUM * LC_MAX_V_NUM] = {0};
#define SLD_BLK_APL_MAX (241*136)		// max for 4k. 
unsigned int APL_pre[SLD_BLK_APL_MAX] = {0};
unsigned int APL_cur[SLD_BLK_APL_MAX] = {0};
unsigned int APL_delta[SLD_BLK_APL_MAX] = {0};

extern VIP_OPS_CTRL_Item OPS_CTRL_item;

unsigned short APL_cur_uint16[SLD_BLK_APL_MAX] = {0};


//Add by kuan 2020.05.26
unsigned int scene_change_flag = 0;
int scene_change_speed_up_count_ = 0;
int APL_avg_cur = 0, APL_avg_pre = 0;
unsigned int memc_scene_change_flag_global = 0, apl_scene_change_flag_global = 0;
unsigned int still_image_count = 0 ;
int pre_memc_logo_to_demura_drop_limit = 0;
int scene_change_drop = 0; 

unsigned int Pre_apl_hi_block_count = 0;
unsigned int apl_hi_block_count = 0;
unsigned int realsc_count = 0, fflag = 0;
unsigned int four_circle_pattern_count = 0;
unsigned int scen_count_ = 0;
unsigned four_circle_pattern_flag = 0;
DRV_sld_condition_struct sld_condition;
DRV_SLD_sld_fw_read_only fw_read_only_data;
unsigned char LR_LOGO_Flag_R = 0;
unsigned char LR_LOGO_Flag_L = 0;
unsigned char shopping_ch = 0;
unsigned int shopping_ch_gain_count = 0 ;

int thl_temporal_dY = 0;
int thl_counter_NEWS_case = 0;
int SLD_cmd_2 = 0;

//#ifdef CONFIG_ARM64 //ARM32 compatible
// for register dump
#include <tvscalercontrol/io/ioregdrv.h>
#undef rtd_outl
#define rtd_outl(x, y)                                  IoReg_Write32(x,y)
#undef rtd_inl
#define rtd_inl(x)                                      IoReg_Read32(x)
#undef rtd_maskl
#define rtd_maskl(x, y, z)                              IoReg_Mask32(x,y,z)
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
unsigned short lock_fw_gain = 0, lock_rgb_box_fw_step = 0, lock_max_logo_counter_gain = 0;
unsigned short flag_news = 0;
unsigned int LC_time = 0;


int cmd_dY_preCur_mouseCon = 1300;
int cmd_LC_period = 5;
int cmd_thl_counter_UR_con = 0;
int cmd_counter_frame_highY_blk = 150; //total : 60x10
int cmd_lock_fw_gain_frame = 30; //300/5
int cmd_pixel_gain_step = 40;
int cmd_thl_counter_blk_pre_cur_same_mouseCon = 1790; //total frame still block: 1800
int cmd_thl_error_time_box_logo_case = 0;
int cmd_thl_counter_box_logo_case = 0;
int cmd_thl_error_time_NEWS_case = 60 * 2;
int cmd_thl_counter_NEWS_case = 60 * 5;
int cmd_thl_error_time_noise_case = 0;
int cmd_thl_counter_noise_case = 0;
int cmd_thl_dC = 0;
int cmd_thl_dUDL = 0;
int cmd_rgb_box_leave_idx_delete_step = 0;
int cmd_box_logo_pixel_gain = 0;
int cmd_box_logo_maxGain_time = 0;
int cmd_box_lock_frame = 3; //full frame dY




int global_frame = 0;
int stage_1_flag = 0, stage_2_flag = 0; //A function stage flag
unsigned char SLD_DMA_Ready = 0;

#define logo_circle4_x_sta  388 // 393  // 398
#define logo_circle4_x_end  470// 465   // 473
#define logo_circle4_y_sta  8 // 18 // 13
#define logo_circle4_y_end  46 // 36    // 32

#define logo_circle4_apply_x_sta  393   // 398
#define logo_circle4_apply_x_end  465   // 473
#define logo_circle4_apply_y_sta  18    // 13
#define logo_circle4_apply_y_end  36    // 32

#define logo_circle4_apply_x_sta_271x136  0 //195   // 398
#define logo_circle4_apply_x_end_271x136  240 //237   // 473
#define logo_circle4_apply_y_sta_271x136  5 //6    // 13
#define logo_circle4_apply_y_end_271x136  16 // 18    // 32

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

/*
void drvif_color_set_SLD_cnt_thl(unsigned int cnt_th)
{
    color_sld_sld_drop_RBUS color_sld_drop;

    color_sld_drop.regValue = IoReg_Read32(COLOR_SLD_SLD_DROP_reg);

    color_sld_drop.sld_cnt_thl = cnt_th;

    IoReg_Write32(COLOR_SLD_SLD_DROP_reg, color_sld_drop.regValue);

}
*/
//#define SLD_BLK_APL_MAX (120*68)		// max for 4k.
static unsigned short SLD_BLK_APL[SLD_BLK_APL_MAX];	
static VIP_SLD_DMA_CTRL SLD_APL_DMA_CTRL[2] = {0};
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
    unsigned int sld_addr = 0;
    unsigned int align_value = 0;







    sld_size = carvedout_buf_query(CARVEDOUT_SCALER_OD, (void *) &sld_addr);
    if (sld_addr == 0 || sld_size == 0) {
        VIPprintf("[%s %d] ERR : %x %x\n", __func__, __LINE__, sld_addr, sld_size);
        return false;
    }

    if (drvif_color_get_HW_MEX_STATUS(MEX_OD) != 1) {
        rtd_pr_vpq_info("HW_MEX,  MEX_OD != 1, PQC\n");
        return false;
    }



#ifdef CONFIG_BW_96B_ALIGNED
    //align_value = dvr_memory_alignment((unsigned long)sld_addr, dvr_size_alignment(500000)); //96 Alignment for Bandwidth Request
    align_value = dvr_memory_alignment((unsigned long)sld_addr, dvr_size_alignment(2400000)); //96 Alignment for Bandwidth Request
    if (align_value == 0) {
        BUG();
    }
#else
    align_value = sld_addr; //origin value
#endif
    VIPprintf("aligned phy addr=%x,\n", align_value);
    rtd_pr_vpq_info("[sld], aligned phy addr=%x,\n", align_value);


    IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_addr_0_reg, align_value); //must be check 96 alignment
    //IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_addr_1_reg, align_value+0x02f00000); //Cap_sta_addr_0, must be check 96 alignment
    IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_addr_1_reg, align_value + 0x50040); //Cap_sta_addr_0, must be check 96 alignment

    IoReg_Write32(OD_DMA_ODDMA_WR_Rule_check_up_reg, align_value + 0x02f00000 + 0x2f000000); //Cap_up_limit
    IoReg_Write32(OD_DMA_ODDMA_WR_Rule_check_low_reg, sld_addr);
    IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_addr_0_reg, align_value);//must be check 96 alignment IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_addr_0_reg, align_value);//must be check 96 alignment
    //IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_addr_1_reg, align_value+0x02f00000); //Cap_sta_addr_0, must be check 96 alignment
    IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_addr_1_reg, align_value + 0x50040); //Cap_sta_addr_0, must be check 96 alignment


    IoReg_Write32(OD_DMA_ODDMA_RD_Rule_check_up_reg, align_value + 0x02f00000 + 0x2f000000); //Disp_up_limit
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
    //od_dma_oddma_rd_rule_check_up_RBUS    od_dma_oddma_rd_rule_check_up;
    od_dma_oddma_pq_cmp_RBUS od_dma_oddma_pq_cmp;
    od_dma_oddma_pq_decmp_RBUS od_dma_oddma_pq_decmp;
    od_od_ctrl_RBUS od_od_ctrl;
    od_dma_oddma_wr_num_bl_wrap_rolling_RBUS od_dma_oddma_wr_num_bl_wrap_rolling;
    od_dma_oddma_rd_num_bl_wrap_rolling_RBUS od_dma_oddma_rd_num_bl_wrap_rolling;

    od_dma_od_dma_db_en.regValue = IoReg_Read32(OD_DMA_OD_DMA_DB_EN_reg);
    od_dma_od_dma_db_en.cap_db_en = 0;
    od_dma_od_dma_db_en.disp_db_en = 0;
    od_dma_od_dma_db_en.smooth_tog_en = 0;
    IoReg_Write32(OD_DMA_OD_DMA_DB_EN_reg, 0x00000000); //db disable

    //sld_dma_return = drvif_set_sld_dma();

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
    od_od_ctrl.regValue = IoReg_Read32(OD_OD_CTRL_reg);

    //IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_reg,0x003c0438); //pq_cmp return default disable, k7 remove
    //IoReg_Write32(OD_DMA_ODDMA_PQ_DECMP_reg,0x003c0438); //pq_dcmp return default disable, k7 remove

    od_od_ctrl.oddma_clk_sel = 0x1;
    IoReg_Write32(OD_OD_CTRL_reg, 0x00000330); //od_clk en

    //IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_rolling_reg,0x00000439); //cap rolling disable, k7 remove
    //IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_rolling_reg,0x00000439); //disp rolling disable, k7 remove

    color_sld_sld_size.sld_width = 3840;
    color_sld_sld_size.sld_height = 2160;
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

#ifdef SLD_Debug
    VIPprintf("drvif_color_set_SLD_4k_dma_setting_K7\n");
#endif

}

char drvif_color_set_SLD_CTRL(unsigned int phy_addr, unsigned int buff_size, unsigned char scale_mode)
{
	//unsigned char sld_dma_return = false;
	color_sld_sld_size_RBUS color_sld_sld_size;
	od_dma_oddma_wr_ctrl_RBUS	   od_dma_oddma_wr_ctrl;
	od_dma_oddma_rd_ctrl_RBUS		od_dma_oddma_rd_ctrl;
	//od_dma_oddma_wr_num_bl_wrap_addr_0_RBUS		 od_dma_oddma_wr_num_bl_wrap_addr_0;
	//od_dma_oddma_wr_num_bl_wrap_addr_1_RBUS		 od_dma_oddma_wr_num_bl_wrap_addr_1;
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

	if (drvif_color_get_HW_MEX_STATUS(MEX_SLD) != 1) {
		rtd_pr_vpq_info("HW_MEX,  MEX_SLD != 1, SLD CTRL return\n");
		return -1;
	}

	od_dma_od_dma_db_en.regValue = IoReg_Read32(OD_DMA_OD_DMA_DB_EN_reg);
	od_dma_od_dma_db_en.cap_db_en = 0;
	od_dma_od_dma_db_en.disp_db_en = 0;
	od_dma_od_dma_db_en.smooth_tog_en = 0;
	IoReg_Write32(OD_DMA_OD_DMA_DB_EN_reg, 0x00000000); //db disable

	//sld_dma_return = drvif_set_sld_dma();

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
	od_od_ctrl.regValue = IoReg_Read32(OD_OD_CTRL_reg);

	//IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_reg,0x003c0438); //pq_cmp return default disable, k7 remove
	//IoReg_Write32(OD_DMA_ODDMA_PQ_DECMP_reg,0x003c0438); //pq_dcmp return default disable, k7 remove

	od_od_ctrl.oddma_clk_sel = 0x1;
	od_od_ctrl.oddma_clk_en = 0x1;
#ifdef _VIP_Mer7_Compile_Error__
	od_od_ctrl.od_scaling_filter = 3;
#endif
	IoReg_Write32(OD_OD_CTRL_reg, od_od_ctrl.regValue); //od_clk en

	//IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_rolling_reg,0x00000439); //cap rolling disable, k7 remove
	//IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_rolling_reg,0x00000439); //disp rolling disable, k7 remove
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

void drvif_color_od_disable_set(unsigned char od_enable)
{
    int timeoutcnt = 0x032500;
    od_od_ctrl_RBUS od_od_ctrl_reg;
    od_od_ctrl1_RBUS od_od_ctrl1_reg;

    od_od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
    od_od_ctrl1_reg.regValue = IoReg_Read32(OD_OD_CTRL1_reg);

    if (drvif_color_get_HW_MEX_STATUS(MEX_OD) != 1) {
        rtd_pr_vpq_info("HW_MEX,  MEX_OD != 1, od_enable force 0,\n");
        od_enable = 0;
    }

    /* OD function db on */
    IoReg_SetBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_od_en_dbf_en_mask); //_BIT24

    /* OD function db read sel: reg */
    IoReg_ClearBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_db_read_level_mask); //_BIT25

    /* OD enable */
    od_od_ctrl_reg.od_en = od_enable;
    IoReg_Write32(OD_OD_CTRL_reg, od_od_ctrl_reg.regValue);

    /* db apply */
    IoReg_SetBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_db_apply_mask); //_BIT26

    /* check */
    while (drvif_color_od_enable_get() != od_enable && (timeoutcnt--) > 0) {
        //rtd_pr_vpq_emerg("%s, wait db apply od_enable=%d, timeout cnt=%d", __func__, od_enable, timeoutcnt);
    }
    if (timeoutcnt == 0) {
        od_od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
        rtd_pr_vpq_emerg("%s, time out!, timeout cnt = %d, od_ctrl_reg.regValue=%x,\n", __func__, timeoutcnt, od_od_ctrl_reg.od_en);
    }
    else {
        rtd_pr_vpq_emerg("%s, timeout cnt = %d, set od_enable %d done\n", __func__, timeoutcnt, od_enable);
    }

    /* OD function db off */
    IoReg_ClearBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_od_en_dbf_en_mask); //_BIT24

    /* OD function db read sel: temp */
    IoReg_SetBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_db_read_level_mask); //_BIT25
}

void drvif_color_set_SLD_Enable(unsigned char enable)
{
    color_sld_sld_control_RBUS color_sld_control;
    color_sld_control.regValue = IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg);

    color_sld_control.sld_en = enable;

	if ((drvif_color_get_HW_MEX_STATUS(MEX_SLD) != 1) && (color_sld_control.sld_en == 1)) {
		rtd_pr_vpq_info("HW_MEX,  MEX_SLD != 1, SLD En CTRL set to 0\n");
		color_sld_control.sld_en = 0;
	}


#ifdef SLD_Debug
    VIPprintf("sld_valid = %d\n", color_sld_control.sld_en);
#endif

    IoReg_Write32(COLOR_SLD_SLD_CONTROL_reg, color_sld_control.regValue);
}

void drvif_color_set_SLD_global_NOT_drop(void)
{

    color_sld_sld_control_RBUS color_sld_control;
    color_sld_control.regValue = IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg);

    color_sld_control.sld_global_drop                       = 0;

    IoReg_Write32(COLOR_SLD_SLD_CONTROL_reg, color_sld_control.regValue);
}

void drvif_color_set_SLD_global_drop(void)
{

    color_sld_sld_control_RBUS color_sld_control;
    color_sld_control.regValue = IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg);

    color_sld_control.sld_global_drop                       = 1;

    IoReg_Write32(COLOR_SLD_SLD_CONTROL_reg, color_sld_control.regValue);
}

void drvif_color_set_SLD_disable_global_drop(void)
{

    color_sld_sld_control_RBUS color_sld_control;
    color_sld_control.regValue = IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg);

    color_sld_control.sld_global_drop                       = 0;

    IoReg_Write32(COLOR_SLD_SLD_CONTROL_reg, color_sld_control.regValue);
}


void drvif_color_set_SLD_control(DRV_SLD_sld_control *ptr)
{

    color_sld_sld_control_RBUS color_sld_control;
	color_sld_sld_scale_up_init_RBUS color_sld_sld_scale_up_init_reg;
	
	color_sld_control.regValue = IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg);
	color_sld_sld_scale_up_init_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_DILATION_CTRL_reg);
	
    color_sld_control.sld_add_comp_dy                           = ptr->sld_add_comp_dy;
    color_sld_control.sld_scale_mode                        = ptr->sld_scale_mode;
    color_sld_control.sld_disable_scale_up              = ptr->sld_disable_scale_up;
    color_sld_control.sld_disable_lpf                       = ptr->sld_disable_lpf;
    color_sld_control.sld_idx_count_period              = ptr->sld_idx_count_period;
    color_sld_control.sld_idx_input_sel                 = ptr->sld_idx_input_sel;
    color_sld_control.sld_window_en                     = ptr->sld_window_en;
    color_sld_control.sld_hpf_type                          = ptr->sld_hpf_type;
    color_sld_control.sld_global_drop                       = ptr->sld_global_drop;
    color_sld_control.sld_blend_en                          = ptr->sld_blend_en;
    //color_sld_control.sld_debug_mode                        = ptr->sld_debug_mode;
	//color_sld_control.sld_en = ptr->sld_en;
	color_sld_sld_scale_up_init_reg.sld_su_hinit			= ptr->sld_su_hinit;
	color_sld_sld_scale_up_init_reg.sld_su_vinit			= ptr->sld_su_vinit;

    IoReg_Write32(COLOR_SLD_SLD_CONTROL_reg, color_sld_control.regValue);
	IoReg_Write32(COLOR_SLD_SLD_DILATION_CTRL_reg, color_sld_sld_scale_up_init_reg.regValue);
}

void drvif_color_set_SLD_window_en(DRV_SLD_sld_control *ptr)
{

    color_sld_sld_control_RBUS color_sld_control;
    color_sld_control.regValue = IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg);

    color_sld_control.sld_window_en                     = ptr->sld_window_en;

    IoReg_Write32(COLOR_SLD_SLD_CONTROL_reg, color_sld_control.regValue);
}


#if 0
void drvif_color_set_SLD_size(DRV_SLD_sld_size *ptr)
{

    color_sld_sld_size_RBUS color_sld_size;
    color_sld_size.regValue = IoReg_Read32(COLOR_SLD_SLD_SIZE_reg);

    color_sld_size.sld_width                            = ptr->sld_width;
    color_sld_size.sld_height                       = ptr->sld_height;

    IoReg_Write32(COLOR_SLD_SLD_SIZE_reg, color_sld_size.regValue);
}

void drvif_color_set_SLD_rgb2y_coef(DRV_SLD_sld_rgb2y_coef *ptr)
{

    color_sld_sld_rgb2y_coef_1_RBUS color_rgb2y_coef_1;
    color_sld_sld_rgb2y_coef_2_RBUS color_rgb2y_coef_2;
    color_rgb2y_coef_1.regValue = IoReg_Read32(COLOR_SLD_SLD_RGB2Y_coef_1_reg);
    color_rgb2y_coef_2.regValue = IoReg_Read32(COLOR_SLD_SLD_RGB2Y_coef_2_reg);

    color_rgb2y_coef_1.coef_r                           = ptr->coef_r;
    color_rgb2y_coef_1.coef_g                           = ptr->coef_g;
    color_rgb2y_coef_2.coef_b                           = ptr->coef_b;

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

    sld_Y_curve_map.regValue            = IoReg_Read32(COLOR_SLD_SLD_CURVE_MAP_SEG_reg);
    sld_new_curve_map_seg_1.regValue    = IoReg_Read32(COLOR_SLD_SLD_NEW_CURVE_MAP_SEG_1_reg);
    sld_new_curve_map_gain_0.regValue   = IoReg_Read32(COLOR_SLD_SLD_NEW_CURVE_MAP_GAIN_0_reg);
    sld_new_curve_map_gain_1.regValue   = IoReg_Read32(COLOR_SLD_SLD_NEW_CURVE_MAP_GAIN_1_reg);
    sld_new_curve_map_offset_0.regValue = IoReg_Read32(COLOR_SLD_SLD_NEW_CURVE_MAP_OFFSET_0_reg);
    sld_new_curve_map_offset_1.regValue = IoReg_Read32(COLOR_SLD_SLD_NEW_CURVE_MAP_OFFSET_1_reg);

    sld_Y_curve_map.sld_curve_seg_0                         = ptr->sld_curve_seg_0;
    sld_Y_curve_map.sld_curve_seg_1                         = ptr->sld_curve_seg_1;
    sld_new_curve_map_seg_1.sld_curve_seg_2                 = ptr->sld_curve_seg_2;

    sld_new_curve_map_gain_0.sld_new_curve_gain_0           = ptr->sld_new_curve_gain_0;
    sld_new_curve_map_gain_0.sld_new_curve_gain_1           = ptr->sld_new_curve_gain_1;
    sld_new_curve_map_gain_1.sld_new_curve_gain_2           = ptr->sld_new_curve_gain_2;
    sld_new_curve_map_gain_1.sld_new_curve_gain_3           = ptr->sld_new_curve_gain_3;


    sld_new_curve_map_offset_0.sld_new_curve_offset_0       = ptr->sld_new_curve_offset_0;
    sld_new_curve_map_offset_0.sld_new_curve_offset_1       = ptr->sld_new_curve_offset_1;
    sld_new_curve_map_offset_1.sld_new_curve_offset_2       = ptr->sld_new_curve_offset_2;
    sld_new_curve_map_offset_1.sld_new_curve_offset_3       = ptr->sld_new_curve_offset_3;


    IoReg_Write32(COLOR_SLD_SLD_CURVE_MAP_SEG_reg, sld_Y_curve_map.regValue);
    IoReg_Write32(COLOR_SLD_SLD_NEW_CURVE_MAP_SEG_1_reg, sld_new_curve_map_seg_1.regValue);
    IoReg_Write32(COLOR_SLD_SLD_NEW_CURVE_MAP_GAIN_0_reg, sld_new_curve_map_gain_0.regValue);
    IoReg_Write32(COLOR_SLD_SLD_NEW_CURVE_MAP_GAIN_1_reg, sld_new_curve_map_gain_1.regValue);
    IoReg_Write32(COLOR_SLD_SLD_NEW_CURVE_MAP_OFFSET_0_reg, sld_new_curve_map_offset_0.regValue);
    IoReg_Write32(COLOR_SLD_SLD_NEW_CURVE_MAP_OFFSET_1_reg, sld_new_curve_map_offset_1.regValue);

/*
    IoReg_Write32(COLOR_SLD_SLD_CURVE_MAP_SEG_reg, sld_Y_curve_map.regValue);
    IoReg_Write32(COLOR_SLD_SLD_NEW_CURVE_MAP_SEG_1_reg, sld_new_curve_map_seg_1.regValue);
    IoReg_Write32(COLOR_SLD_SLD_NEW_CURVE_MAP_GAIN_0_reg, sld_new_curve_map_gain_0.regValue);
    IoReg_Write32(COLOR_SLD_SLD_NEW_CURVE_MAP_GAIN_1_reg, sld_new_curve_map_gain_1.regValue);
    IoReg_Write32(COLOR_SLD_SLD_NEW_CURVE_MAP_OFFSET_0_reg, sld_new_curve_map_offset_0.regValue);
    IoReg_Write32(COLOR_SLD_SLD_NEW_CURVE_MAP_OFFSET_1_reg, sld_new_curve_map_offset_1.regValue);
*/
}



//yvonne add for SLD 20220419
short fw_pixel_gain_TBL_XY[2][2] = {
	// X axis:
	{102, 128},
	// y axis:
	{3071, 4095},
};

// yvonne add for SLD 20220419
void fwif_drvif_color_set_SLD_Y_curve_map_bySLD_dropGain(int drop_gain_from_v4l2)
{
	unsigned short print_en = OPS_CTRL_item.OPS_Debug_Item.Debug_HWSW_SLD_En;
	//unsigned short print_delay = OPS_CTRL_item.OPS_Debug_Item.Debug_Delay;
	//DRV_SLD_sld_Y_curve_map sld_Y_curve_map;
	DRV_SLD_sld_fw_control_0 sld_fw_control_0;
	unsigned short x0, x1, y0, y1;
	int temp;

	x0 = fw_pixel_gain_TBL_XY[0][0];
	x1 = fw_pixel_gain_TBL_XY[0][1];
	y0 = fw_pixel_gain_TBL_XY[1][0];
	y1 = fw_pixel_gain_TBL_XY[1][1];
	

	if (drop_gain_from_v4l2 <= 0) {
		sld_fw_control_0.fw_pixel_gain = 0;
	} 
	else if (drop_gain_from_v4l2 >= 128) {
		sld_fw_control_0.fw_pixel_gain = 4095;
	} 
	else if (drop_gain_from_v4l2 <= x0) {
		temp = (x0==0)?(1):(x0);
		sld_fw_control_0.fw_pixel_gain = y0 * drop_gain_from_v4l2 / temp;
	} else if (drop_gain_from_v4l2 <= x1) {
		temp = ((x1 - x0) == 0)?(1):(x1 - x0);
		sld_fw_control_0.fw_pixel_gain = y0 + (y1-y0)*(drop_gain_from_v4l2-x0)/temp;
	} else if (drop_gain_from_v4l2 > x1) {
		temp = ((128 - x1) == 0)?(1):(128 - x1);
		sld_fw_control_0.fw_pixel_gain = y1 + (4095-y1)*(drop_gain_from_v4l2-x1)/temp;
	}
	else 
		sld_fw_control_0.fw_pixel_gain = 0;
	
	drvif_color_set_SLD_fw_gain_control(sld_fw_control_0.fw_pixel_gain);

	#if 0
	if(drop_gain_from_v4l2==0){	
		sld_Y_curve_map.sld_new_curve_gain_0 = 64;
		sld_Y_curve_map.sld_new_curve_gain_1 = 64;
		sld_Y_curve_map.sld_new_curve_gain_2 = 64;
		sld_Y_curve_map.sld_new_curve_gain_3 = 64;

		sld_Y_curve_map.sld_new_curve_offset_0 = 0;
		sld_Y_curve_map.sld_new_curve_offset_1 = 0;
		sld_Y_curve_map.sld_new_curve_offset_2 = 0;
		sld_Y_curve_map.sld_new_curve_offset_3 = 0;			
	} else {
		sld_Y_curve_map.sld_new_curve_gain_0 = 64;
		sld_Y_curve_map.sld_new_curve_gain_1 = 64;
		sld_Y_curve_map.sld_new_curve_gain_2 = 64;
		sld_Y_curve_map.sld_new_curve_gain_3 = 40;
		
		sld_Y_curve_map.sld_new_curve_offset_0 = 0;
		sld_Y_curve_map.sld_new_curve_offset_1 = 0;
		sld_Y_curve_map.sld_new_curve_offset_2 = 0;
		sld_Y_curve_map.sld_new_curve_offset_3 = 298;
	}
	drvif_color_set_SLD_Y_curve_map_wo_seg((DRV_SLD_sld_Y_curve_map *) &sld_Y_curve_map);
	#endif
	if ((print_en & _BIT0) != 0){
		//rtd_pr_vpq_info("SLD_HW, memc_logo_to_demura_drop_limit=%d\n",drop_gain_from_v4l2);

	}

}


void drvif_color_set_SLD_Idx_curve_map(DRV_SLD_sld_idx_map_seg *ptr)
{

    color_sld_sld_idx_map_seg_RBUS            sld_idx_map_seg_;
    color_sld_sld_new_idx_map_seg_1_RBUS      sld_new_curve_map_seg_1;
    color_sld_sld_new_idx_map_gain_0_RBUS     sld_new_curve_map_gain_0;
    color_sld_sld_new_idx_map_gain_1_RBUS     sld_new_curve_map_gain_1;
    color_sld_sld_new_idx_map_offset_0_RBUS   sld_new_curve_map_offset_0;
    color_sld_sld_new_idx_map_offset_1_RBUS   sld_new_curve_map_offset_1;

    sld_idx_map_seg_.regValue           = IoReg_Read32(COLOR_SLD_SLD_IDX_MAP_SEG_reg);
    sld_new_curve_map_seg_1.regValue    = IoReg_Read32(COLOR_SLD_SLD_NEW_IDX_MAP_SEG_1_reg);
    sld_new_curve_map_gain_0.regValue   = IoReg_Read32(COLOR_SLD_SLD_NEW_IDX_MAP_GAIN_0_reg);
    sld_new_curve_map_gain_1.regValue   = IoReg_Read32(COLOR_SLD_SLD_NEW_IDX_MAP_GAIN_1_reg);
    sld_new_curve_map_offset_0.regValue = IoReg_Read32(COLOR_SLD_SLD_NEW_IDX_MAP_OFFSET_0_reg);
    sld_new_curve_map_offset_1.regValue = IoReg_Read32(COLOR_SLD_SLD_NEW_IDX_MAP_OFFSET_1_reg);

    sld_idx_map_seg_.sld_idx_seg_0                          = ptr->sld_idx_seg_0;
    sld_idx_map_seg_.sld_idx_seg_1                          = ptr->sld_idx_seg_1;
    sld_new_curve_map_seg_1.sld_idx_seg_2                   = ptr->sld_idx_seg_2;

    sld_new_curve_map_gain_0.sld_new_idx_gain_0         = ptr->sld_new_idx_gain_0;
    sld_new_curve_map_gain_0.sld_new_idx_gain_1         = ptr->sld_new_idx_gain_1;
    sld_new_curve_map_gain_1.sld_new_idx_gain_2         = ptr->sld_new_idx_gain_2;
    sld_new_curve_map_gain_1.sld_new_idx_gain_3         = ptr->sld_new_idx_gain_3;


    sld_new_curve_map_offset_0.sld_new_idx_offset_0     = ptr->sld_new_idx_offset_0;
    sld_new_curve_map_offset_0.sld_new_idx_offset_1     = ptr->sld_new_idx_offset_1;
    sld_new_curve_map_offset_1.sld_new_idx_offset_2     = ptr->sld_new_idx_offset_2;
    sld_new_curve_map_offset_1.sld_new_idx_offset_3     = ptr->sld_new_idx_offset_3;

    IoReg_Write32(COLOR_SLD_SLD_IDX_MAP_SEG_reg, sld_idx_map_seg_.regValue);
    IoReg_Write32(COLOR_SLD_SLD_NEW_IDX_MAP_SEG_1_reg, sld_new_curve_map_seg_1.regValue);
    IoReg_Write32(COLOR_SLD_SLD_NEW_IDX_MAP_GAIN_0_reg, sld_new_curve_map_gain_0.regValue);
    IoReg_Write32(COLOR_SLD_SLD_NEW_IDX_MAP_GAIN_1_reg, sld_new_curve_map_gain_1.regValue);
    IoReg_Write32(COLOR_SLD_SLD_NEW_IDX_MAP_OFFSET_0_reg, sld_new_curve_map_offset_0.regValue);
    IoReg_Write32(COLOR_SLD_SLD_NEW_IDX_MAP_OFFSET_1_reg, sld_new_curve_map_offset_1.regValue);

/*
    IoReg_Write32(COLOR_SLD_SLD_CURVE_MAP_SEG_reg, sld_idx_map_seg_.regValue);
    IoReg_Write32(COLOR_SLD_SLD_NEW_CURVE_MAP_SEG_1_reg, sld_new_curve_map_seg_1.regValue);
    IoReg_Write32(COLOR_SLD_SLD_NEW_CURVE_MAP_GAIN_0_reg, sld_new_curve_map_gain_0.regValue);
    IoReg_Write32(COLOR_SLD_SLD_NEW_CURVE_MAP_GAIN_1_reg, sld_new_curve_map_gain_1.regValue);
    IoReg_Write32(COLOR_SLD_SLD_NEW_CURVE_MAP_OFFSET_0_reg, sld_new_curve_map_offset_0.regValue);
    IoReg_Write32(COLOR_SLD_SLD_NEW_CURVE_MAP_OFFSET_1_reg, sld_new_curve_map_offset_1.regValue);
*/
}


void drvif_color_set_SLD_Idx_add_con(DRV_SLD_sld_acc_0 *ptr)
{

    color_sld_sld_acc_0_RBUS            sld_sld_acc_0;
    color_sld_sld_hpf_0_RBUS      sld_sld_hpf_0;
    color_sld_sld_acc_1_RBUS     sld_acc_1;

    sld_sld_acc_0.regValue          = IoReg_Read32(COLOR_SLD_SLD_ACC_0_reg);
    sld_sld_hpf_0.regValue  = IoReg_Read32(COLOR_SLD_SLD_HPF_0_reg);
    sld_acc_1.regValue  = IoReg_Read32(COLOR_SLD_SLD_ACC_1_reg);

    sld_sld_acc_0.sld_idx_acc_condition_and                         = ptr->sld_idx_acc_condition_and;
    sld_sld_acc_0.sld_max_idx_thl                                   = ptr->sld_max_idx_thl;
    sld_sld_acc_0.sld_frame_diffy_thl                               = ptr->sld_frame_diffy_thl;
    sld_sld_hpf_0.sld_hpf_logo_add_thl                              = ptr->sld_hpf_logo_add_thl;
	sld_sld_hpf_0.sld_hpf_gain_type									= ptr->sld_hpf_gain_type;
	sld_acc_1.sld_y_idx_thl                                         = ptr->sld_y_idx_thl;

    IoReg_Write32(COLOR_SLD_SLD_ACC_0_reg, sld_sld_acc_0.regValue);
    IoReg_Write32(COLOR_SLD_SLD_HPF_0_reg, sld_sld_hpf_0.regValue);
    IoReg_Write32(COLOR_SLD_SLD_ACC_1_reg, sld_acc_1.regValue);

}

void drvif_color_set_SLD_8dir_12tab(DRV_SLD_sld_in_logo_thl_0 *ptr)
{

    color_sld_sld_in_logo_thl_0_RBUS   sld_in_logo_thl_0;
    color_sld_sld_in_logo_thl_1_RBUS   sld_in_logo_thl_1;
    color_sld_sld_in_logo_thl_2_RBUS   sld_in_logo_thl_2;
    color_sld_sld_in_logo_thl_3_RBUS   sld_in_logo_thl_3;
    color_sld_sld_in_logo_thl_4_RBUS   sld_in_logo_thl_4;
    color_sld_sld_in_logo_thl_5_RBUS   sld_in_logo_thl_5;

    sld_in_logo_thl_0.regValue          = IoReg_Read32(COLOR_SLD_SLD_IN_LOGO_THL_0_reg);
    sld_in_logo_thl_1.regValue          = IoReg_Read32(COLOR_SLD_SLD_IN_LOGO_THL_1_reg);
    sld_in_logo_thl_2.regValue          = IoReg_Read32(COLOR_SLD_SLD_IN_LOGO_THL_2_reg);
    sld_in_logo_thl_3.regValue          = IoReg_Read32(COLOR_SLD_SLD_IN_LOGO_THL_3_reg);
    sld_in_logo_thl_4.regValue          = IoReg_Read32(COLOR_SLD_SLD_IN_LOGO_THL_4_reg);
    sld_in_logo_thl_5.regValue          = IoReg_Read32(COLOR_SLD_SLD_IN_LOGO_THL_5_reg);

    sld_in_logo_thl_0.sld_inter_thl_0                           = ptr->sld_inter_thl_0;
    sld_in_logo_thl_0.sld_inter_thl_1                           = ptr->sld_inter_thl_1;
    sld_in_logo_thl_1.sld_inter_thl_2                           = ptr->sld_inter_thl_2;
    sld_in_logo_thl_1.sld_inter_thl_3                           = ptr->sld_inter_thl_3;
    sld_in_logo_thl_2.sld_inter_thl_4                           = ptr->sld_inter_thl_4;
    sld_in_logo_thl_2.sld_inter_thl_5                           = ptr->sld_inter_thl_5;
    sld_in_logo_thl_3.sld_inter_thl_6                           = ptr->sld_inter_thl_6;
    sld_in_logo_thl_3.sld_inter_thl_7                           = ptr->sld_inter_thl_7;
    sld_in_logo_thl_4.sld_inter_thl_8                           = ptr->sld_inter_thl_8;
    sld_in_logo_thl_4.sld_inter_thl_9                           = ptr->sld_inter_thl_9;
    sld_in_logo_thl_5.sld_inter_thl_10                          = ptr->sld_inter_thl_10;
    sld_in_logo_thl_5.sld_inter_thl_11                          = ptr->sld_inter_thl_11;



    IoReg_Write32(COLOR_SLD_SLD_IN_LOGO_THL_0_reg, sld_in_logo_thl_0.regValue);
    IoReg_Write32(COLOR_SLD_SLD_IN_LOGO_THL_1_reg, sld_in_logo_thl_1.regValue);
    IoReg_Write32(COLOR_SLD_SLD_IN_LOGO_THL_2_reg, sld_in_logo_thl_2.regValue);
    IoReg_Write32(COLOR_SLD_SLD_IN_LOGO_THL_3_reg, sld_in_logo_thl_3.regValue);
    IoReg_Write32(COLOR_SLD_SLD_IN_LOGO_THL_4_reg, sld_in_logo_thl_4.regValue);
    IoReg_Write32(COLOR_SLD_SLD_IN_LOGO_THL_5_reg, sld_in_logo_thl_5.regValue);

}


void drvif_color_set_SLD_lpf(DRV_SLD_sld_lpf_0 *ptr)
{

    color_sld_sld_lpf_0_RBUS     sld_lpf_0;
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

    sld_lpf_0.regValue              = IoReg_Read32(COLOR_SLD_SLD_LPF_0_reg);
    sld_lpf_1.regValue              = IoReg_Read32(COLOR_SLD_SLD_LPF_1_reg);
    sld_lpf_2.regValue              = IoReg_Read32(COLOR_SLD_SLD_LPF_2_reg);
    sld_lpf_3.regValue              = IoReg_Read32(COLOR_SLD_SLD_LPF_3_reg);
    sld_lpf_4.regValue              = IoReg_Read32(COLOR_SLD_SLD_LPF_4_reg);
    sld_lpf_5.regValue              = IoReg_Read32(COLOR_SLD_SLD_LPF_5_reg);
    sld_lpf_6.regValue              = IoReg_Read32(COLOR_SLD_SLD_LPF_6_reg);
    sld_lpf_7.regValue              = IoReg_Read32(COLOR_SLD_SLD_LPF_7_reg);
    sld_lpf_8.regValue              = IoReg_Read32(COLOR_SLD_SLD_LPF_8_reg);
    sld_lpf_9.regValue              = IoReg_Read32(COLOR_SLD_SLD_LPF_9_reg);
    sld_lpf_10.regValue             = IoReg_Read32(COLOR_SLD_SLD_LPF_10_reg);
    sld_lpf_11.regValue             = IoReg_Read32(COLOR_SLD_SLD_LPF_11_reg);
    sld_lpf_12.regValue             = IoReg_Read32(COLOR_SLD_SLD_LPF_12_reg);
    sld_lpf_div.regValue            = IoReg_Read32(COLOR_SLD_SLD_LPF_DIV_reg);

    sld_lpf_0.coef_0_0                          = ptr->coef_0_0;
    sld_lpf_0.coef_0_1                          = ptr->coef_0_1;

    sld_lpf_1.coef_0_2                          = ptr->coef_0_2;
    sld_lpf_1.coef_0_3                          = ptr->coef_0_3;

    sld_lpf_2.coef_0_4                          = ptr->coef_0_4;
    sld_lpf_2.coef_1_0                          = ptr->coef_1_0;

    sld_lpf_3.coef_1_1                          = ptr->coef_1_1;
    sld_lpf_3.coef_1_2                          = ptr->coef_1_2;

    sld_lpf_4.coef_1_3                          = ptr->coef_1_3;
    sld_lpf_4.coef_1_4                          = ptr->coef_1_4;

    sld_lpf_5.coef_2_0                          = ptr->coef_2_0;
    sld_lpf_5.coef_2_1                          = ptr->coef_2_1;

    sld_lpf_6.coef_2_2                          = ptr->coef_2_2;
    sld_lpf_6.coef_2_3                          = ptr->coef_2_3;

    sld_lpf_7.coef_2_4                          = ptr->coef_2_4;
    sld_lpf_7.coef_3_0                          = ptr->coef_3_0;

    sld_lpf_8.coef_3_1                          = ptr->coef_3_1;
    sld_lpf_8.coef_3_2                          = ptr->coef_3_2;

    sld_lpf_9.coef_3_3                          = ptr->coef_3_3;
    sld_lpf_9.coef_3_4                          = ptr->coef_3_4;

    sld_lpf_10.coef_4_0                         = ptr->coef_4_0;
    sld_lpf_10.coef_4_1                         = ptr->coef_4_1;

    sld_lpf_11.coef_4_2                         = ptr->coef_4_2;
    sld_lpf_11.coef_4_3                         = ptr->coef_4_3;

    sld_lpf_12.coef_4_4                         = ptr->coef_4_4;

    sld_lpf_div.lpf_shift_bit                           = ptr->lpf_shift_bit;


    IoReg_Write32(COLOR_SLD_SLD_LPF_0_reg, sld_lpf_0.regValue);
    IoReg_Write32(COLOR_SLD_SLD_LPF_1_reg, sld_lpf_1.regValue);
    IoReg_Write32(COLOR_SLD_SLD_LPF_2_reg, sld_lpf_2.regValue);
    IoReg_Write32(COLOR_SLD_SLD_LPF_3_reg, sld_lpf_3.regValue);
    IoReg_Write32(COLOR_SLD_SLD_LPF_4_reg, sld_lpf_4.regValue);
    IoReg_Write32(COLOR_SLD_SLD_LPF_5_reg, sld_lpf_5.regValue);
    IoReg_Write32(COLOR_SLD_SLD_LPF_6_reg, sld_lpf_6.regValue);
    IoReg_Write32(COLOR_SLD_SLD_LPF_7_reg, sld_lpf_7.regValue);
    IoReg_Write32(COLOR_SLD_SLD_LPF_8_reg, sld_lpf_8.regValue);
    IoReg_Write32(COLOR_SLD_SLD_LPF_9_reg, sld_lpf_9.regValue);
    IoReg_Write32(COLOR_SLD_SLD_LPF_10_reg, sld_lpf_10.regValue);
    IoReg_Write32(COLOR_SLD_SLD_LPF_11_reg, sld_lpf_11.regValue);
    IoReg_Write32(COLOR_SLD_SLD_LPF_12_reg, sld_lpf_12.regValue);
    IoReg_Write32(COLOR_SLD_SLD_LPF_DIV_reg, sld_lpf_div.regValue);

}

void drvif_color_set_SLD_step(DRV_SLD_sld_hpf_1 *ptr)
{

    color_sld_sld_hpf_1_RBUS     sld_hpf_1;

    sld_hpf_1.regValue  = IoReg_Read32(COLOR_SLD_SLD_HPF_1_reg);

    sld_hpf_1.sld_idx_sign_0    = ptr->sld_idx_sign_0;
    sld_hpf_1.sld_idx_step_0    = ptr->sld_idx_step_0;
    sld_hpf_1.sld_idx_sign_2    = ptr->sld_idx_sign_2;
    sld_hpf_1.sld_idx_step_2    = ptr->sld_idx_step_2;


    IoReg_Write32(COLOR_SLD_SLD_HPF_1_reg, sld_hpf_1.regValue);

}

void drvif_color_set_SLD_drop(DRV_SLD_sld_drop *ptr, DRV_SLD_sld_idx_not_drop_0 *ptr_in_logo, DRV_SLD_sld_neighbor_not_drop *ptr_n_notDrop, DRV_SLD_sld_neighbor_drop_0 *ptr_nDrop, DRV_SLD_sld_idx_protect_by_neighbor_0 *ptr_dropProtect)
{

    color_sld_sld_drop_RBUS  sld_drop;
    color_sld_sld_idx_not_drop_0_RBUS idx_not_drop_0;
    color_sld_sld_idx_not_drop_1_RBUS idx_not_drop_1;
    color_sld_sld_neighbor_not_drop_RBUS neighbor_not_drop;
    color_sld_sld_neighbor_drop_0_RBUS neighbor_drop_0 ;
    color_sld_sld_neighbor_drop_1_RBUS neighbor_drop_1 ;
    color_sld_sld_idx_protect_by_neighbor_0_RBUS protect_by_neighbor;
    color_sld_sld_idx_protect_by_neighbor_1_RBUS protect_by_neighbor_1;

    sld_drop.regValue                   = IoReg_Read32(COLOR_SLD_SLD_DROP_reg);
    idx_not_drop_0.regValue             = IoReg_Read32(COLOR_SLD_SLD_Idx_not_drop_0_reg);
    idx_not_drop_1.regValue             = IoReg_Read32(COLOR_SLD_SLD_Idx_not_drop_1_reg);
    neighbor_not_drop.regValue          = IoReg_Read32(COLOR_SLD_SLD_NEIGHBOR_NOT_DROP_reg);
    neighbor_drop_0.regValue            = IoReg_Read32(COLOR_SLD_SLD_NEIGHBOR_DROP_0_reg);
    neighbor_drop_1.regValue            = IoReg_Read32(COLOR_SLD_SLD_NEIGHBOR_DROP_1_reg);
    protect_by_neighbor.regValue            = IoReg_Read32(COLOR_SLD_SLD_Idx_Protect_By_Neighbor_0_reg);
    protect_by_neighbor_1.regValue              = IoReg_Read32(COLOR_SLD_SLD_Idx_Protect_By_Neighbor_1_reg);

    sld_drop.sld_cnt_thl        = ptr->sld_cnt_thl;
    sld_drop.sld_hpf_diff_thl   = ptr->sld_hpf_diff_thl;
    sld_drop.sld_pixel_diff_thl = ptr->sld_pixel_diff_thl;

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



    IoReg_Write32(COLOR_SLD_SLD_DROP_reg, sld_drop.regValue);
    IoReg_Write32(COLOR_SLD_SLD_Idx_not_drop_0_reg, idx_not_drop_0.regValue);
    IoReg_Write32(COLOR_SLD_SLD_Idx_not_drop_1_reg, idx_not_drop_1.regValue);
    IoReg_Write32(COLOR_SLD_SLD_NEIGHBOR_NOT_DROP_reg, neighbor_not_drop.regValue);
    IoReg_Write32(COLOR_SLD_SLD_NEIGHBOR_DROP_0_reg, neighbor_drop_0.regValue);
    IoReg_Write32(COLOR_SLD_SLD_NEIGHBOR_DROP_1_reg, neighbor_drop_1.regValue);
    IoReg_Write32(COLOR_SLD_SLD_Idx_Protect_By_Neighbor_0_reg, protect_by_neighbor.regValue);
    IoReg_Write32(COLOR_SLD_SLD_Idx_Protect_By_Neighbor_1_reg, protect_by_neighbor_1.regValue);

}


void drvif_color_set_SLD_compensate_spatial(DRV_SLD_sld_idx_compensate_by_neighbor_0 *ptr)
{

    color_sld_sld_idx_compensate_by_neighbor_0_RBUS  compensate_by_neighbor_0;
    color_sld_sld_idx_compensate_by_neighbor_1_RBUS compensate_by_neighbor_1;

    compensate_by_neighbor_0.regValue                   = IoReg_Read32(COLOR_SLD_SLD_Idx_Compensate_By_Neighbor_0_reg);
    compensate_by_neighbor_1.regValue                   = IoReg_Read32(COLOR_SLD_SLD_Idx_Compensate_By_Neighbor_1_reg);

    compensate_by_neighbor_1.idx_compensate_start_thl               = ptr->idx_compensate_start_thl;
    compensate_by_neighbor_1.compensate_neighbor_tolerance          = ptr->compensate_neighbor_tolerance;
    compensate_by_neighbor_0.idx_compensate_neighbor_counter_thl    = ptr->idx_compensate_neighbor_counter_thl;



    IoReg_Write32(COLOR_SLD_SLD_Idx_Compensate_By_Neighbor_0_reg, compensate_by_neighbor_0.regValue);
    IoReg_Write32(COLOR_SLD_SLD_Idx_Compensate_By_Neighbor_1_reg, compensate_by_neighbor_1.regValue);

}

void drvif_color_set_SLD_compensate_temporal(DRV_SLD_sld_temporal_idx_compensate *ptr)
{

    color_sld_sld_temporal_idx_compensate_RBUS   temporal_idx_compensate;

    temporal_idx_compensate.regValue                    = IoReg_Read32(COLOR_SLD_SLD_Temporal_Idx_Compensate_reg);

    temporal_idx_compensate.idx_period_check_once               = ptr->idx_period_check_once;
    temporal_idx_compensate.do_connect_seg_idx_lower_bound          = ptr->do_connect_seg_idx_lower_bound;



    IoReg_Write32(COLOR_SLD_SLD_Temporal_Idx_Compensate_reg, temporal_idx_compensate.regValue);

}

void drvif_color_set_SLD_control_minus_step(DRV_SLD_sld_drop_slow_0 *ptr)
{

    color_sld_sld_drop_slow_0_RBUS   sld_drop_slow_0;
    color_sld_sld_drop_slow_1_RBUS   sld_drop_slow_1;

    sld_drop_slow_0.regValue                    = IoReg_Read32(COLOR_SLD_SLD_Drop_Slow_0_reg);
    sld_drop_slow_1.regValue                    = IoReg_Read32(COLOR_SLD_SLD_Drop_Slow_1_reg);

    sld_drop_slow_0.idx_slow_down_l             = ptr->idx_slow_down_l;
    sld_drop_slow_0.idx_slow_down_h             = ptr->idx_slow_down_h;
    sld_drop_slow_1.idx_slow_down_step          = ptr->idx_slow_down_step;



    IoReg_Write32(COLOR_SLD_SLD_Drop_Slow_0_reg, sld_drop_slow_0.regValue);
    IoReg_Write32(COLOR_SLD_SLD_Drop_Slow_1_reg, sld_drop_slow_1.regValue);

}
void drvif_color_set_SLD_fw_control_step(DRV_SLD_sld_fw_control_0 *ptr)
{

    color_sld_sld_fw_control_0_RBUS  fw_control_0;
    color_sld_sld_fw_control_1_RBUS  fw_control_1;

    fw_control_0.regValue                   = IoReg_Read32(COLOR_SLD_SLD_FW_control_0_reg);
    fw_control_1.regValue                   = IoReg_Read32(COLOR_SLD_SLD_FW_control_1_reg);

    fw_control_0.fw_idx_ctrl_step               = ptr->fw_idx_ctrl_step;
    //fw_control_0.fw_idx_freeze_for_full_frame             = ptr->fw_idx_freeze_for_full_frame;
    fw_control_0.fw_idx_ctrl_en             = ptr->fw_idx_ctrl_en;
    //fw_control_1.fw_pixel_gain                = ptr->fw_pixel_gain;



    IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg, fw_control_0.regValue);
    IoReg_Write32(COLOR_SLD_SLD_FW_control_1_reg, fw_control_1.regValue);

}




void drvif_color_set_SLD_fw_control(DRV_SLD_sld_fw_control_0 *ptr)
{

    color_sld_sld_fw_control_0_RBUS  fw_control_0;
    color_sld_sld_fw_control_1_RBUS  fw_control_1;

    fw_control_0.regValue                   = IoReg_Read32(COLOR_SLD_SLD_FW_control_0_reg);
    fw_control_1.regValue                   = IoReg_Read32(COLOR_SLD_SLD_FW_control_1_reg);

    fw_control_0.fw_idx_ctrl_step               = ptr->fw_idx_ctrl_step;
    fw_control_0.fw_idx_freeze_for_full_frame               = ptr->fw_idx_freeze_for_full_frame;
    fw_control_0.fw_idx_ctrl_en             = ptr->fw_idx_ctrl_en;
    fw_control_1.fw_pixel_gain              = ptr->fw_pixel_gain;



    IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg, fw_control_0.regValue);
    IoReg_Write32(COLOR_SLD_SLD_FW_control_1_reg, fw_control_1.regValue);

}


// yvonne add for SLD 20220419
void drvif_color_set_SLD_fw_gain_control(unsigned short gain)
{
    color_sld_sld_fw_control_1_RBUS  fw_control_1;
    fw_control_1.regValue                   = IoReg_Read32(COLOR_SLD_SLD_FW_control_1_reg);
    fw_control_1.fw_pixel_gain              = gain;
    IoReg_Write32(COLOR_SLD_SLD_FW_control_1_reg, fw_control_1.regValue);
}


void drvif_color_set_SLD_sld_temporal_noise(DRV_SLD_sld_temporal_noise_1 *ptr)
{

    color_sld_sld_temporal_noise_0_RBUS  temporal_noise_0;
    color_sld_sld_temporal_noise_1_RBUS  temporal_noise_1;
    color_sld_sld_temporal_noise_3_RBUS  temporal_noise_3;

    temporal_noise_0.regValue                   = IoReg_Read32(COLOR_SLD_SLD_Temporal_Noise_0_reg);
    temporal_noise_1.regValue                   = IoReg_Read32(COLOR_SLD_SLD_Temporal_Noise_1_reg);
    temporal_noise_3.regValue                   = IoReg_Read32(COLOR_SLD_SLD_Temporal_Noise_3_reg);

    temporal_noise_0.temporal_diff              = ptr->temporal_diff;
    temporal_noise_1.big_temporal_diff_low              = ptr->big_temporal_diff_low;
    temporal_noise_1.big_temporal_diff_up               = ptr->big_temporal_diff_up;
    temporal_noise_3.temporal_counter_thl               = ptr->temporal_counter_thl;



    IoReg_Write32(COLOR_SLD_SLD_Temporal_Noise_0_reg, temporal_noise_0.regValue);
    IoReg_Write32(COLOR_SLD_SLD_Temporal_Noise_1_reg, temporal_noise_1.regValue);
    IoReg_Write32(COLOR_SLD_SLD_Temporal_Noise_3_reg, temporal_noise_3.regValue);

}

void drvif_color_set_SLD_sld_window(DRV_SLD_sld_apply_window_0 *ptr)
{

    color_sld_sld_control_RBUS color_sld_control;

    color_sld_sld_apply_window_0_RBUS    apply_window_0;
    color_sld_sld_apply_window_1_RBUS    apply_window_1;

    color_sld_control.regValue = IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg);
    apply_window_0.regValue                     = IoReg_Read32(COLOR_SLD_SLD_Apply_Window_0_reg);
    apply_window_1.regValue                     = IoReg_Read32(COLOR_SLD_SLD_Apply_Window_1_reg);

    //color_sld_control.sld_window_en               = 0;
    apply_window_0.sld_window_apply_out             = ptr->sld_window_apply_out;
    apply_window_0.sld_window_pos0_y                = ptr->sld_window_pos0_y;
    apply_window_0.sld_window_pos0_x                = ptr->sld_window_pos0_x;
    apply_window_1.sld_window_pos1_y                = ptr->sld_window_pos1_y;
    apply_window_1.sld_window_pos1_x                = ptr->sld_window_pos1_x;


    IoReg_Write32(COLOR_SLD_SLD_Apply_Window_0_reg, apply_window_0.regValue);
    IoReg_Write32(COLOR_SLD_SLD_Apply_Window_1_reg, apply_window_1.regValue);
    IoReg_Write32(COLOR_SLD_SLD_CONTROL_reg, color_sld_control.regValue);




}
void drvif_color_set_SLD_sld_histogram(DRV_SLD_sld_histogram_1 *ptr, DRV_SLD_sld_histogram_0 *ptr_0)
{

    color_sld_sld_histogram_0_RBUS   histogram_0;
    color_sld_sld_histogram_1_RBUS   histogram_1;
    color_sld_sld_histogram_2_RBUS   histogram_2;
    color_sld_sld_histogram_3_RBUS   histogram_3;
    color_sld_sld_histogram_4_RBUS   histogram_4;

    histogram_0.regValue                    = IoReg_Read32(COLOR_SLD_SLD_histogram_0_reg);
    histogram_1.regValue                    = IoReg_Read32(COLOR_SLD_SLD_histogram_1_reg);
    histogram_2.regValue                    = IoReg_Read32(COLOR_SLD_SLD_histogram_2_reg);
    histogram_3.regValue                    = IoReg_Read32(COLOR_SLD_SLD_histogram_3_reg);
    histogram_4.regValue                    = IoReg_Read32(COLOR_SLD_SLD_histogram_4_reg);

    histogram_0.hist_idx_thl                = ptr_0->hist_idx_thl;
    histogram_0.hist_shift_bit              = ptr_0->hist_shift_bit;
    histogram_1.y_lower_bound_hist_0                = ptr->y_lower_bound_hist_0;
    histogram_1.y_lower_bound_hist_1                = ptr->y_lower_bound_hist_1;
    histogram_2.y_lower_bound_hist_2                = ptr->y_lower_bound_hist_2;
    histogram_2.y_lower_bound_hist_3                = ptr->y_lower_bound_hist_3;
    histogram_3.y_lower_bound_hist_4                = ptr->y_lower_bound_hist_4;
    histogram_3.y_lower_bound_hist_5                = ptr->y_lower_bound_hist_5;
    histogram_4.y_lower_bound_hist_6                = ptr->y_lower_bound_hist_6;
    histogram_4.y_lower_bound_hist_7                = ptr->y_lower_bound_hist_7;

    IoReg_Write32(COLOR_SLD_SLD_histogram_0_reg, histogram_0.regValue);
    IoReg_Write32(COLOR_SLD_SLD_histogram_1_reg, histogram_1.regValue);
    IoReg_Write32(COLOR_SLD_SLD_histogram_2_reg, histogram_2.regValue);
    IoReg_Write32(COLOR_SLD_SLD_histogram_3_reg, histogram_3.regValue);
    IoReg_Write32(COLOR_SLD_SLD_histogram_4_reg, histogram_4.regValue);

}

void drvif_color_set_SLD_LGD_Tnoise_pattern(DRV_SLD_sld_noise_pattern_0 *ptr)
{
    color_sld_sld_noise_pattern_1_RBUS   noise_pattern_1;
    color_sld_sld_noise_pattern_2_RBUS   noise_pattern_2;
    color_sld_sld_noise_pattern_3_RBUS   noise_pattern_3;
    color_sld_sld_noise_pattern_0_RBUS   noise_pattern_0;

    noise_pattern_0.regValue                    = IoReg_Read32(COLOR_SLD_SLD_noise_pattern_0_reg);
    noise_pattern_1.regValue                    = IoReg_Read32(COLOR_SLD_SLD_noise_pattern_1_reg);
    noise_pattern_2.regValue                    = IoReg_Read32(COLOR_SLD_SLD_noise_pattern_2_reg);
    noise_pattern_3.regValue                    = IoReg_Read32(COLOR_SLD_SLD_noise_pattern_3_reg);

    noise_pattern_0.detect_tnoise_en              = ptr->detect_tnoise_en;
    noise_pattern_0.grouph_diffmaxmin             = ptr->grouph_diffmaxmin;
    noise_pattern_0.diffavg                     = ptr->diffavg;
    noise_pattern_3.check_drop_pattern_start                = ptr->check_drop_pattern_start;
    noise_pattern_1.patternedge_y               = ptr->patternedge_y;
    noise_pattern_1.patternedge_hpf             = ptr->patternedge_hpf;
    noise_pattern_2.patternregion_hpf               = ptr->patternregion_hpf;
    noise_pattern_2.patternregion_y             = ptr->patternregion_y;


    IoReg_Write32(COLOR_SLD_SLD_noise_pattern_0_reg, noise_pattern_0.regValue);
    IoReg_Write32(COLOR_SLD_SLD_noise_pattern_1_reg, noise_pattern_1.regValue);
    IoReg_Write32(COLOR_SLD_SLD_noise_pattern_2_reg, noise_pattern_2.regValue);
    IoReg_Write32(COLOR_SLD_SLD_noise_pattern_3_reg, noise_pattern_3.regValue);

}


void drvif_color_set_SLD_max_pre_logo_tolerance(DRV_SLD_sld_fw_control_2 *ptr)
{

    color_sld_sld_fw_control_2_RBUS  fw_control_2;

    fw_control_2.regValue                   = IoReg_Read32(COLOR_SLD_SLD_FW_control_2_reg);

    fw_control_2.max_pre_logo_tolerance             = ptr->max_pre_logo_tolerance;

    IoReg_Write32(COLOR_SLD_SLD_FW_control_2_reg, fw_control_2.regValue);

}

void drvif_color_set_SLD_Dilation_Type(DRV_SLD_sld_dilation_ctrl *ptr)
{
    color_sld_sld_dilation_ctrl_RBUS color_sld_sld_dilation_ctrl_reg;
    color_sld_sld_dilation_ctrl_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_DILATION_CTRL_reg);

    color_sld_sld_dilation_ctrl_reg.sld_dilat_type = ptr->sld_dilat_type;

    IoReg_Write32(COLOR_SLD_SLD_DILATION_CTRL_reg, color_sld_sld_dilation_ctrl_reg.regValue);
}

void drvif_color_set_SLD_8_Dir_fillhole_thl(DRV_SLD_sld_8_dir_fill_hole_thl *ptr)
{
    color_sld_sld_in_logo_thl_0_RBUS color_sld_sld_in_logo_thl_0_reg;

    color_sld_sld_in_logo_thl_0_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_IN_LOGO_THL_0_reg);

    color_sld_sld_in_logo_thl_0_reg.sld_8_dir_fill_hole_thl = ptr->sld_8_dir_fill_hole_thl;

    IoReg_Write32(COLOR_SLD_SLD_IN_LOGO_THL_0_reg, color_sld_sld_in_logo_thl_0_reg.regValue);
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
    fw_control_0_data.fw_idx_ctrl_step = 8192 - cmd_rgb_box_leave_idx_delete_step; //step : -128
    fw_control_0_data.fw_idx_freeze_for_full_frame = 0;//step : -128
    fw_control_0_data.fw_pixel_gain = 4096;

    rtd_pr_vpq_info("sld_max_pre_idx = %d################\n", sld_max_pre_idx);

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

int drvif_color_get_SLD_Dummy(void)
{
    int dummy_b802c700_value = 0;
    color_sld_sld_control_RBUS color_sld_control;
    color_sld_control.regValue = IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg);
#ifdef _VIP_Mer7_Compile_Error__
    dummy_b802c700_value = color_sld_control.dummy1802c700_31_22;
#endif
    return dummy_b802c700_value;

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

    sld_lpf_0_data.coef_0_0 = 1;
    sld_lpf_0_data.coef_0_1 = 6;
    sld_lpf_0_data.coef_0_2 = 6;
    sld_lpf_0_data.coef_0_3 = 6;
    sld_lpf_0_data.coef_0_4 = 1;

    sld_lpf_0_data.coef_1_0 = 6;
    sld_lpf_0_data.coef_1_1 = 36;
    sld_lpf_0_data.coef_1_2 = 36;
    sld_lpf_0_data.coef_1_3 = 36;
    sld_lpf_0_data.coef_1_4 = 6;

    sld_lpf_0_data.coef_2_0 = 6;
    sld_lpf_0_data.coef_2_1 = 36;
    sld_lpf_0_data.coef_2_2 = 36;
    sld_lpf_0_data.coef_2_3 = 24;
    sld_lpf_0_data.coef_2_4 = 6;

    sld_lpf_0_data.coef_3_0 = 6;
    sld_lpf_0_data.coef_3_1 = 36;
    sld_lpf_0_data.coef_3_2 = 36;
    sld_lpf_0_data.coef_3_3 = 24;
    sld_lpf_0_data.coef_3_4 = 6;

    sld_lpf_0_data.coef_4_0 = 6;
    sld_lpf_0_data.coef_4_1 = 24;
    sld_lpf_0_data.coef_4_2 = 36;
    sld_lpf_0_data.coef_4_3 = 24;
    sld_lpf_0_data.coef_4_4 = 6;

    sld_lpf_0_data.lpf_shift_bit = 9;
    drvif_color_set_SLD_lpf((DRV_SLD_sld_lpf_0 *) &sld_lpf_0_data);
}


//get///////////////////////////////
void drvif_color_get_SLD_max_pre_idx(unsigned int *sld_max_pre_idx)
{

    color_sld_sld_cnt_2_RBUS     sld_cnt_2;

    sld_cnt_2.regValue                          = IoReg_Read32(COLOR_SLD_SLD_CNT_2_reg);

    *sld_max_pre_idx                        = sld_cnt_2.sld_max_pre_idx;

}



void drvif_color_get_SLD_fw_read_only(DRV_SLD_sld_fw_read_only *ptr)
{

    color_sld_sld_fw_read_only_0_RBUS    fw_read_only_0;
    color_sld_sld_fw_read_only_1_RBUS    fw_read_only_1;
    color_sld_sld_fw_read_only_2_RBUS    fw_read_only_2;
    color_sld_sld_fw_read_only_3_RBUS    fw_read_only_3;
    color_sld_sld_cnt_0_RBUS     sld_cnt_0;
    color_sld_sld_cnt_1_RBUS     sld_cnt_1;
    color_sld_sld_cnt_2_RBUS     sld_cnt_2;

    fw_read_only_0.regValue                     = IoReg_Read32(COLOR_SLD_SLD_FW_read_only_0_reg);
    fw_read_only_1.regValue                     = IoReg_Read32(COLOR_SLD_SLD_FW_read_only_1_reg);
    fw_read_only_2.regValue                     = IoReg_Read32(COLOR_SLD_SLD_FW_read_only_2_reg);
    fw_read_only_3.regValue                     = IoReg_Read32(COLOR_SLD_SLD_FW_read_only_3_reg);
    sld_cnt_0.regValue                          = IoReg_Read32(COLOR_SLD_SLD_CNT_0_reg);
    sld_cnt_1.regValue                          = IoReg_Read32(COLOR_SLD_SLD_CNT_1_reg);
    sld_cnt_2.regValue                          = IoReg_Read32(COLOR_SLD_SLD_CNT_2_reg);

    ptr->max_pre_logo_idx_counter             = fw_read_only_0.max_pre_logo_idx_counter  ;
    ptr->near_max_pre_logo_idx_counter        = fw_read_only_1.near_max_pre_logo_idx_counter;
    ptr->hpf_line_count                       = fw_read_only_3.hpf_line_count;
    ptr->blocknum_with_alpha_noise            = fw_read_only_2.blocknum_with_alpha_noise;
    ptr->sld_logo_cnt                           = sld_cnt_0.sld_logo_cnt;
    ptr->sld_drop_cnt                           = sld_cnt_1.sld_drop_cnt;
    ptr->sld_max_pre_idx                        = sld_cnt_2.sld_max_pre_idx;





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
	
	color_sld_sld_wdma_db_en_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_WDMA_DB_EN_reg);
	if (color_sld_sld_wdma_db_en_reg.cap_db_apply == 1) {
		rtd_pr_vpq_emerg("SLD_HW, APL cap_db_apply=1\n");
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
	
	pVa = (unsigned char*)SLD_DMA_CTRL->pVir_addr_align;
	idx_byte = 0;
	for (i=0;i<read_3_cnt;i++) {
		SLD_BLK_APL[i*SLD_APL_num_per_32b+0] = ((((unsigned short)pVa[idx_byte])&0x3f)<<4) + ((((unsigned short)pVa[idx_byte+1])&0xf0)>>4);
		SLD_BLK_APL[i*SLD_APL_num_per_32b+1] = ((((unsigned short)pVa[idx_byte+1])&0x0f)<<6) + ((((unsigned short)pVa[idx_byte+2])&0xfc)>>2);
		SLD_BLK_APL[i*SLD_APL_num_per_32b+2] = ((((unsigned short)pVa[idx_byte+2])&0x03)<<8) + (((unsigned short)pVa[idx_byte+3])&0xff);
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

void drvif_SLD_Block_APL_transform(int* pinarry , int* poutarry,int blk_hnum, int blk_vnum){
	int i;
	int read_cnt = blk_hnum*blk_vnum*10/32;
	for(i=0;i<read_cnt;i++){
		poutarry[i+0] = (pinarry[i]);
		poutarry[i+1] = (pinarry[i]);
		poutarry[i+2] = (pinarry[i]);
		poutarry[i+3] = (pinarry[i]);
	}
}

void drvif_SLD_Block_APL_DMA_OFF(void)
{
	color_sld_sld_wdma_db_en_RBUS color_sld_sld_wdma_db_en_reg;
	color_sld_sld_wdma_wr_ctrl_RBUS color_sld_sld_wdma_wr_ctrl_reg;

	if (SYS_REG_SYS_CLKEN1_get_clken_dispd(IoReg_Read32(SYS_REG_SYS_CLKEN1_reg)) == 0) {
		rtd_pr_vpq_info("SLD_HW, drvif_SLD_Block_APL_DMA_OFF, dispd is off\n");
		return;
	}	

	color_sld_sld_wdma_db_en_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_WDMA_DB_EN_reg);
	color_sld_sld_wdma_db_en_reg.cap_db_en = 0;
	IoReg_Write32(COLOR_SLD_SLD_WDMA_DB_EN_reg,color_sld_sld_wdma_db_en_reg.regValue);

	color_sld_sld_wdma_wr_ctrl_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_WDMA_WR_Ctrl_reg);
	color_sld_sld_wdma_wr_ctrl_reg.cap_en = 0;
	IoReg_Write32(COLOR_SLD_SLD_WDMA_WR_Ctrl_reg,color_sld_sld_wdma_wr_ctrl_reg.regValue);


	//SLD_DMA_APL_Ready_CNT = 0;
	rtd_pr_vpq_info("SLD_HW, execute drvif_SLD_Block_APL_DMA_OFF\n");
}

void drvif_SLD_Block_APL_DMA_ON(void)
{
	color_sld_sld_wdma_db_en_RBUS color_sld_sld_wdma_db_en_reg;
	color_sld_sld_wdma_wr_ctrl_RBUS color_sld_sld_wdma_wr_ctrl_reg;

	if (SYS_REG_SYS_CLKEN1_get_clken_dispd(IoReg_Read32(SYS_REG_SYS_CLKEN1_reg)) == 0) {
		rtd_pr_vpq_info("SLD_HW, drvif_SLD_Block_APL_DMA_ON, dispd is off\n");
		return;
	}	


	color_sld_sld_wdma_db_en_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_WDMA_DB_EN_reg);
	color_sld_sld_wdma_db_en_reg.cap_db_en = 1;
	IoReg_Write32(COLOR_SLD_SLD_WDMA_DB_EN_reg,color_sld_sld_wdma_db_en_reg.regValue);

	color_sld_sld_wdma_wr_ctrl_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_WDMA_WR_Ctrl_reg);
	color_sld_sld_wdma_wr_ctrl_reg.cap_en = 0;
	IoReg_Write32(COLOR_SLD_SLD_WDMA_WR_Ctrl_reg,color_sld_sld_wdma_wr_ctrl_reg.regValue);

	color_sld_sld_wdma_db_en_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_WDMA_DB_EN_reg);
	color_sld_sld_wdma_db_en_reg.cap_db_apply = 1;
	IoReg_Write32(COLOR_SLD_SLD_WDMA_DB_EN_reg,color_sld_sld_wdma_db_en_reg.regValue);
	rtd_pr_vpq_info("SLD_HW, always cap off\n");
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
	color_sld_sld_dma_wr_num_bl_wrap_addr_up_bound_RBUS  color_sld_sld_dma_wr_num_bl_wrap_addr_up_bound_reg;
	color_sld_sld_dma_wr_num_bl_wrap_addr_low_bound_RBUS  color_sld_sld_dma_wr_num_bl_wrap_addr_low_bound_reg;
	int sld_width,sld_height;
	int Block_APL_buf_size = 40*1024;
	
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
	color_sld_sld_dma_wr_num_bl_wrap_ctl_reg.cap_frame_128_num= ((sld_width/32)*10*((sld_height+31)/32) + 119 )/120;
	color_sld_sld_dma_wr_num_bl_wrap_ctl_reg.cap_burst_len = 1;
	color_sld_sld_wdma_wr_ctrl_reg.cap_en = 0;
	color_sld_sld_wdma_wr_ctrl_reg.cap_block_sel = 0;		// start from addr 0

	IoReg_Write32(COLOR_SLD_SLD_WDMA_Status_reg,color_sld_sld_wdma_status_reg.regValue);
	IoReg_Write32(COLOR_SLD_SLD_WDMA_WR_Ctrl_reg,color_sld_sld_wdma_wr_ctrl_reg.regValue);
	IoReg_Write32(COLOR_SLD_SLD_DMA_WR_num_bl_wrap_addr_0_reg,color_sld_sld_dma_wr_num_bl_wrap_addr_0_reg.regValue);
	IoReg_Write32(COLOR_SLD_SLD_DMA_WR_num_bl_wrap_addr_1_reg,color_sld_sld_dma_wr_num_bl_wrap_addr_1_reg.regValue);
	IoReg_Write32(COLOR_SLD_SLD_DMA_WR_num_bl_wrap_ctl_reg,color_sld_sld_dma_wr_num_bl_wrap_ctl_reg.regValue);

	rtd_pr_vpq_info("Block APL WDMA init setting: cap_frame_128_num:%d, cap_burst_len:%d", color_sld_sld_dma_wr_num_bl_wrap_ctl_reg.cap_frame_128_num,color_sld_sld_dma_wr_num_bl_wrap_ctl_reg.cap_burst_len);

	/*Set WDMA lower and upper bound*/
	color_sld_sld_dma_wr_num_bl_wrap_addr_up_bound_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_DMA_WR_num_bl_wrap_addr_up_bound_reg);
	color_sld_sld_dma_wr_num_bl_wrap_addr_low_bound_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_DMA_WR_num_bl_wrap_addr_low_bound_reg);
	color_sld_sld_dma_wr_num_bl_wrap_addr_low_bound_reg.cap_addr_low_bound = p_addr0>>4;
	color_sld_sld_dma_wr_num_bl_wrap_addr_up_bound_reg.cap_addr_up_bound = (p_addr0>>4)+(Block_APL_buf_size>>4);
	IoReg_Write32(COLOR_SLD_SLD_DMA_WR_num_bl_wrap_addr_up_bound_reg,color_sld_sld_dma_wr_num_bl_wrap_addr_up_bound_reg.regValue);
	IoReg_Write32(COLOR_SLD_SLD_DMA_WR_num_bl_wrap_addr_low_bound_reg,color_sld_sld_dma_wr_num_bl_wrap_addr_low_bound_reg.regValue);

	color_sld_sld_wdma_db_en_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_WDMA_DB_EN_reg);
	color_sld_sld_wdma_db_en_reg.cap_db_apply =1;
	IoReg_Write32(COLOR_SLD_SLD_WDMA_DB_EN_reg,color_sld_sld_wdma_db_en_reg.regValue);
}

#if 0

void drvif_color_set_SLD_hpf_in_line_thl(DRV_SLD_sld_hpf_in_line *ptr)
{

    color_sld_sld_hpf_in_line_RBUS   hpf_in_line;

    hpf_in_line.regValue                    = IoReg_Read32(COLOR_SLD_SLD_hpf_in_line_reg);

    hpf_in_line.hpf_in_line_thl             = ptr->hpf_in_line_thl;



    IoReg_Write32(COLOR_SLD_SLD_hpf_in_line_reg, hpf_in_line.regValue);

}


void drvif_color_get_histogram_bin(DRV_SLD_sld_histogram_ro_0 *ptr)
{

    color_sld_sld_histogram_ro_0_RBUS    histogram_ro_0;
    color_sld_sld_histogram_ro_1_RBUS    histogram_ro_1;
    color_sld_sld_histogram_ro_2_RBUS    histogram_ro_2;
    color_sld_sld_histogram_ro_3_RBUS    histogram_ro_3;

    histogram_ro_0.regValue                     = IoReg_Read32(COLOR_SLD_SLD_histogram_ro_0_reg);
    histogram_ro_1.regValue                     = IoReg_Read32(COLOR_SLD_SLD_histogram_ro_1_reg);
    histogram_ro_2.regValue                     = IoReg_Read32(COLOR_SLD_SLD_histogram_ro_2_reg);
    histogram_ro_3.regValue                     = IoReg_Read32(COLOR_SLD_SLD_histogram_ro_3_reg);

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


int CLAMP_SLD(int value, int max, int min)
{

    if (value > max) {
        return (max);
    }
    else if (value < min) {
        return (min);
    }

    return (value);

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
    //APL_4 = pAPL_cur[1293];
    //APL_3 = pAPL_cur[1292];
    //APL_2 = pAPL_cur[1291];
    //APL_1 = pAPL_cur[1290];

    APL_4 = pAPL_cur[1372];
    APL_3 = pAPL_cur[1373];
    APL_2 = pAPL_cur[1374];
    APL_1 = pAPL_cur[1375];

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
    if (
                (ABS_(abs1) < 16)
                && (ABS_(abs2) < 16)
                && (ABS_(abs3) < 16)
    ) {
        //rtd_pr_vpq_info("return 1\n");

        return 1;
    }
    else {
        //rtd_pr_vpq_info("return 0\n");

        return 0;
    }

}


unsigned short LC_detect_RGB_box_logo(unsigned int *pAPL_cur, unsigned int *pAPL_pre, int cmd_thl_dC, int cmd_thl_dUDL)
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

    d_C = C_APL - C_APL_pre;
    d_U = U_APL - U_APL_pre;
    d_D = D_APL - D_APL_pre;
    d_L = L_APL - L_APL_pre;

    //condition pre current
    if (
                (ABS_(d_C) < cmd_thl_dC)
                && (ABS_(d_U) < cmd_thl_dUDL)
                && (ABS_(d_D) < cmd_thl_dUDL)
                && (ABS_(d_L) < cmd_thl_dUDL)
    ) {
        flag_pre_current = 1;
    }
    else {
        flag_pre_current = 0;
    }


    //condition_neighbor_brighter
    if (U_APL >= C_APL) {
        counter_neighbor_brighter += 1;
    }
    if (D_APL >= C_APL) {
        counter_neighbor_brighter += 1;
    }
    if (L_APL >= C_APL) {
        counter_neighbor_brighter += 1;
    }
    if (UL_APL >= L_APL) {
        counter_neighbor_brighter += 1;
    }
    if (DL_APL >= L_APL) {
        counter_neighbor_brighter += 1;
    }

    condition_neighbor_brighter = (counter_neighbor_brighter >= 4);

    //condition_neighbor_darker
    if (U_APL <= C_APL) {
        counter_neighbor_darker += 1;
    }
    if (D_APL <= C_APL) {
        counter_neighbor_darker += 1;
    }
    if (L_APL <= C_APL) {
        counter_neighbor_darker += 1;
    }
    if (UL_APL <= L_APL) {
        counter_neighbor_darker += 1;
    }
    if (DL_APL <= L_APL) {
        counter_neighbor_darker += 1;
    }

    condition_neighbor_darker = (counter_neighbor_darker >= 4);

    //neighbor_dY_condition
    d_U_D = U_APL - D_APL;
    d_L_D = L_APL - D_APL;
    d_L_U = L_APL - U_APL;

    neighbor_dY_condition = ((ABS_(d_U_D) <= 300)
                             && (ABS_(d_L_D) <= 300)
                             && (ABS_(d_L_U) <= 300)
                            );

    //condition_U_D_L
    if (ABS_(d_U_D) <= 50) {
        counter_neighbor_UDL += 1;
    }
    if (ABS_(d_L_D) <= 100) {
        counter_neighbor_UDL += 1;
    }
    if (ABS_(d_L_U) <= 100) {
        counter_neighbor_UDL += 1;
    }

    strick_neighbor_dY_condition = (counter_neighbor_UDL >= 2);

    //Y condition
    //if(C_APL <= 600){
    if (C_APL <= 1023) {
        center_Y_condition = 1;
    }
    else {center_Y_condition = 0;}

    //condition counter
    if (condition_neighbor_brighter) {
        condition_counter += 1;
    }
    if (condition_neighbor_darker) {
        condition_counter += 1;
    }
    if (neighbor_dY_condition) {
        condition_counter += 1;
    }
    if (strick_neighbor_dY_condition) {
        condition_counter += 1;
    }
    if (center_Y_condition) {
        condition_counter += 1;
    }


    if ((condition_counter >= 4) && flag_pre_current) {
        //rtd_pr_vpq_info("box_logo\n");
        return 1;
    }
    else {
        //rtd_pr_vpq_info("not box_logo, 1=%04d, 2=%04d, 3=%04d, 4=%04d, 5=%04d, 6=%04d\n",UL_APL,L_APL,DL_APL,U_APL,C_APL,D_APL);
        return 0;
    }


}



unsigned short LC_detect_noise(unsigned int *pAPL_cur)
{
    unsigned int APL_UL = 0, APL_UR = 0, APL_DL = 0, APL_DR = 0, APL_ULL = 0, APL_DLL = 0;
    unsigned short flag_noise_pattern = 0, flag_t_noise_pattern = 0;
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

    if (
                (APL_UL >= 990)
                && (APL_UR >= 990)
                && (APL_DL >= 990)
                && (APL_DR >= 990)
                && (APL_UL - APL_ULL >= 400)
                && (APL_DL - APL_DLL >= 400)
    ) {
        flag_noise_pattern = 1;

    }

    if (
                ((APL_UL >= 700) && (APL_UL <= 800))
                && ((APL_UR >= 700) && (APL_UR <= 800))
                && ((APL_DL >= 700) && (APL_DL <= 800))
                && ((APL_DR >= 700) && (APL_DR <= 800))
                && (APL_UL - APL_ULL >= 300)
                && (APL_DL - APL_DLL >= 300)
    ) {
        flag_t_noise_pattern = 1;

    }

    //full frame apl < 512


    blu_lc_global_ctrl2.regValue = rtd_inl(LC_LC_Global_Ctrl2_reg);

    num_hblk = blu_lc_global_ctrl2.lc_blk_hnum + 1 ;
    num_vblk = blu_lc_global_ctrl2.lc_blk_vnum + 1;

    idx_arr = 0;
    counter_lower512Y_blk = 0;
    for (idx_r = 0; idx_r < (num_vblk); idx_r++) {
        for (idx_c = 0; idx_c < (num_hblk); idx_c++) {
            if (pAPL_cur[idx_arr] <= 512) {
                counter_lower512Y_blk = counter_lower512Y_blk + 1;
            }
            idx_arr = idx_arr + 1;
        }
    }


    rtd_pr_vpq_info("counter_lower512Y_blk,APL_ULL,APL_UL,APL_UR,APL_DLL,APL_DL,APL_DR----%d,%d,%d,%d,%d,%d,%d\n", counter_lower512Y_blk, APL_ULL, APL_UL, APL_UR, APL_DLL, APL_DL, APL_DR);


    if (
                (
                            (
                                        ((flag_noise_pattern == 1) && (flag_t_noise_pattern != 1))
                                        || ((flag_noise_pattern != 1) && (flag_t_noise_pattern == 1))
                            )
                            && (pAPL_cur[139] <= 800)
                            && (pAPL_cur[140] <= 800)
                            && (pAPL_cur[141] <= 800)

                )
                && (counter_lower512Y_blk >= 1550)

    ) {
        //rtd_pr_vpq_info("return 1\n");

        return 1;
    }
    else {
        //rtd_pr_vpq_info("return 0\n");

        return 0;
    }

}

void  LC_apply_RGB_box_logo(unsigned int cmd_LC_period, unsigned short cmd_thl_counter_box_logo_case, unsigned short cmd_box_logo_pixel_gain, unsigned short cmd_box_logo_maxGain_time)
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
    fw_control_0_data.fw_idx_ctrl_en = 1;
    fw_control_0_data.fw_idx_ctrl_step = 4095;

    //fw control gain

    //fw_control_0_data.fw_pixel_gain=8192<<3;//debug use
    //fw_control_0_data.fw_pixel_gain=CLAMP_SLD(((8192)* cmd_LC_period * (counter_box_logo_case - cmd_thl_counter_box_logo_case))/(13*60),8192,0) ;
    if (counter_box_logo_case > cmd_thl_counter_box_logo_case) {
        fw_control_0_data.fw_pixel_gain = CLAMP_SLD(((cmd_box_logo_pixel_gain) * cmd_LC_period * (counter_box_logo_case - cmd_thl_counter_box_logo_case)) / cmd_box_logo_maxGain_time, cmd_box_logo_pixel_gain, 0) ;
    }
    else {
        fw_control_0_data.fw_pixel_gain = 0;
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


void LC_compute_corner_UR_Y(unsigned int *pAPL_cur, unsigned short  cmd_thl_counter_UR_con, unsigned short *counter_UR)
{
    unsigned short idx_r = 0, idx_c = 0, idx_arr = 0;
    int num_hblk = 0, num_vblk = 0;

    lc_lc_global_ctrl2_RBUS blu_lc_global_ctrl2;

    blu_lc_global_ctrl2.regValue = rtd_inl(LC_LC_Global_Ctrl2_reg);

    num_hblk = blu_lc_global_ctrl2.lc_blk_hnum + 1 ;
    num_vblk = blu_lc_global_ctrl2.lc_blk_vnum + 1;


    for (idx_r = 0; idx_r < (num_vblk); idx_r++) {
        for (idx_c = 0; idx_c < (num_hblk); idx_c++) {

            if ((idx_c >= (num_hblk - 1 - 14))
                    && (idx_r <= (7))
                    && (pAPL_cur[idx_arr] >= 700)
               ) { //UR corner high
                *counter_UR = *counter_UR + 1;
            }
            idx_arr = idx_arr + 1;

        }
    }

}



void  SLD_apply_noise_pattern(unsigned int cmd_LC_period, unsigned short cmd_thl_counter_noise_case)
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
    fw_control_0_data.fw_idx_ctrl_en = 1;
    fw_control_0_data.fw_idx_ctrl_step = 4095;

    //fw control gain

    //fw_control_0_data.fw_pixel_gain=8192<<3;//debug use
    fw_control_0_data.fw_pixel_gain = CLAMP_SLD(((4096) * cmd_LC_period * (counter_noise_case - cmd_thl_counter_noise_case)) / (13 * 60), 4096, 0) ;


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


#if 0
void fwif_color_SLD_fw_read_only(unsigned short  cmd_lock_fw_gain_frame)
{

    DRV_SLD_sld_fw_read_only fw_read_only_data;
    DRV_SLD_sld_idx_compensate_by_neighbor_0 idx_compensate_by_neighbor_data;
    DRV_SLD_sld_fw_control_2  fw_control_max_logo_tolerance_data;
    //DRV_SLD_sld_Y_curve_map sld_Y_curve_data;
    //color_sld_sld_fw_control_0_RBUS    fw_control_0;
    //color_sld_sld_fw_control_1_RBUS    fw_control_1;



    drvif_color_get_SLD_fw_read_only((DRV_SLD_sld_fw_read_only *) &fw_read_only_data);

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
    drvif_color_set_SLD_max_pre_logo_tolerance((DRV_SLD_sld_fw_control_2 *) &fw_control_max_logo_tolerance_data);

    drvif_color_get_SLD_fw_read_only((DRV_SLD_sld_fw_read_only *) &fw_read_only_data);

    if (
                0
                && (fw_read_only_data.near_max_pre_logo_idx_counter > (129600 / 2))
                && (fw_read_only_data.sld_max_pre_idx >= 3000)

    ) { //logo idx : 4095 ~ 2500
        rtd_pr_vpq_info("logo idx > 3000 has 1/4 area condition----------------\n");
        /*
        fw_control_0.regValue                   = rtd_inl(COLOR_SLD_SLD_FW_control_0_reg               );
        fw_control_1.regValue                   = rtd_inl(COLOR_SLD_SLD_FW_control_1_reg               );

        rtd_outl(COLOR_SLD_SLD_FW_control_0_reg , fw_control_0.regValue);

        fw_control_1.fw_pixel_gain              = 4096>>4;
        rtd_outl(COLOR_SLD_SLD_FW_control_1_reg , fw_control_1.regValue);
        */
        lock_fw_gain = cmd_lock_fw_gain_frame;

    }

    //spatial compensate--------------------------------------------------------------------------------------------------------
    idx_compensate_by_neighbor_data.compensate_neighbor_tolerance = 1023;
    idx_compensate_by_neighbor_data.idx_compensate_start_thl = 1023;
    if (fw_read_only_data.near_max_pre_logo_idx_counter < (129600 / 15)) {
        idx_compensate_by_neighbor_data.idx_compensate_neighbor_counter_thl = 3;
        rtd_pr_vpq_info("spatial compensate condition = 3----------------\n");

    }
    else {
        idx_compensate_by_neighbor_data.idx_compensate_neighbor_counter_thl = 9;
        //rtd_pr_vpq_info("spatial compensate condition = 9----------------\n");
    }
    drvif_color_set_SLD_compensate_spatial((DRV_SLD_sld_idx_compensate_by_neighbor_0 *) &idx_compensate_by_neighbor_data);


}


void fwif_color_SLD_add_con(void)
{
    DRV_SLD_sld_fw_read_only fw_read_only_data;
    DRV_SLD_sld_acc_0 SLD_Idx_add_con_data;

    drvif_color_get_SLD_fw_read_only((DRV_SLD_sld_fw_read_only *) &fw_read_only_data);

    SLD_Idx_add_con_data.sld_hpf_logo_add_thl = 128;
    SLD_Idx_add_con_data.sld_y_idx_thl = 256;
    SLD_Idx_add_con_data.sld_frame_diffy_thl = 256;
    SLD_Idx_add_con_data.sld_max_idx_thl = 4095;

    if (
                (fw_read_only_data.max_pre_logo_idx_counter < 256)
                && (cur_frame_idx_global % 2 == 0)
    ) {
        SLD_Idx_add_con_data.sld_idx_acc_condition_and = 0;
        drvif_color_set_SLD_Idx_add_con((DRV_SLD_sld_acc_0 *) &SLD_Idx_add_con_data);
        //rtd_pr_vpq_info("add_con = %d\n",SLD_Idx_add_con_data.sld_idx_acc_condition_and);
    }
    else {
        SLD_Idx_add_con_data.sld_idx_acc_condition_and = 1;
        drvif_color_set_SLD_Idx_add_con((DRV_SLD_sld_acc_0 *) &SLD_Idx_add_con_data);
        //rtd_pr_vpq_info("add_con = %d\n",SLD_Idx_add_con_data.sld_idx_acc_condition_and);
    }

}

void LC_count_num_highY_block_upFrame(unsigned int *pAPL_cur, unsigned int thl_highY_frame, unsigned int *counter_highY_blk)
{
    unsigned short idx_r = 0, idx_c = 0, idx_arr = 0, start_x = 0;
    int num_hblk = 0;

    lc_lc_global_ctrl2_RBUS blu_lc_global_ctrl2;

    blu_lc_global_ctrl2.regValue = rtd_inl(LC_LC_Global_Ctrl2_reg);

    num_hblk = blu_lc_global_ctrl2.lc_blk_hnum + 1 ;
    //num_vblk = blu_lc_global_ctrl2.lc_blk_vnum + 1;

    start_x  = 2 * (num_hblk / 3);
    *counter_highY_blk = 0;
    for (idx_r = 0; idx_r < 10; idx_r++) {
        for (idx_c = start_x; idx_c < (num_hblk); idx_c++) {
            if (pAPL_cur[idx_arr] > thl_highY_frame) {
                *counter_highY_blk = *counter_highY_blk + 1;
            }
        }
    }

}
#endif

// yvonne add for SLD 20220419
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


// yvonne add for SLD 20220419
void Count_num_highY_block_up_Left_Frame_Uint_APL(int num_hblk, int num_vblk, unsigned int *pAPL_cur, unsigned int thl_highY_frame, unsigned int *counter_highY_blk)
{
    unsigned short idx_r = 0, idx_c = 0, start_x = 0;
    int h_num = 0,v_num=0;


    start_x  = num_hblk * 2 / 3; //start from 2/3
    h_num = num_hblk - start_x; //remain 1/3
    v_num = num_vblk/5; /*ver dir get 1/5 block*/
    *counter_highY_blk = 0;
    //apl_hi_block_count = 0;
    for (idx_r = 0; idx_r < v_num; idx_r++) {
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


void LC_count_num_highY_block_up_Left_Frame(unsigned int *pAPL_cur, unsigned int thl_highY_frame, unsigned int *counter_highY_blk)
{
    unsigned short idx_r = 0, idx_c = 0, start_x = 0;
    int num_hblk = 0, h_num = 0;

    lc_lc_global_ctrl2_RBUS blu_lc_global_ctrl2;

    blu_lc_global_ctrl2.regValue = rtd_inl(LC_LC_Global_Ctrl2_reg);

    num_hblk = blu_lc_global_ctrl2.lc_blk_hnum + 1 ;
    //num_vblk = blu_lc_global_ctrl2.lc_blk_vnum + 1;

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

int LC_count_num_four_circle_patterns(unsigned int *pAPL_cur, unsigned int thl_highY_frame)
{
    unsigned short idx_r = 0, idx_c = 0, start_x = 0;
    int num_hblk = 0, h_num = 0;
    int a = 0, b = 0;
    lc_lc_global_ctrl2_RBUS blu_lc_global_ctrl2;

    blu_lc_global_ctrl2.regValue = rtd_inl(LC_LC_Global_Ctrl2_reg);

    num_hblk = blu_lc_global_ctrl2.lc_blk_hnum + 1 ;
    //num_vblk = blu_lc_global_ctrl2.lc_blk_vnum + 1;

    start_x  = num_hblk * 2 / 3; //start from 2/3
    h_num = num_hblk - start_x; //remain 1/3
    apl_hi_block_count = 0;
    for (idx_r = 0; idx_r < 8; idx_r++) {
        for (idx_c = 39; idx_c < (num_hblk); idx_c++) {
            //
            if (pAPL_cur[idx_r * h_num + idx_c] >= 1800) {
                apl_hi_block_count++;
                a++;
            }
            if (pAPL_cur[idx_r * h_num + idx_c] >= 1200) {
                b++;
            }
        }
    }
    if (a >= 2 && a <= 16) {
        return 1;

    }
    else {
        return 0;
    }

}

void LC_compute_pre_cur_deltaY(unsigned int *pAPL_pre, unsigned int *pAPL_cur, unsigned int *APL_avg_cur, unsigned int *pAPL_delta, unsigned int *counter_blk_pre_cur_same, int thl_temporal_dY)
{
    unsigned short idx_r = 0, idx_c = 0, idx_arr = 0;
    int num_hblk = 0, num_vblk = 0, dY_pre_cur = 0;

    lc_lc_global_ctrl2_RBUS blu_lc_global_ctrl2;

    blu_lc_global_ctrl2.regValue = IoReg_Read32(LC_LC_Global_Ctrl2_reg);

    num_hblk = blu_lc_global_ctrl2.lc_blk_hnum + 1 ;
    num_vblk = blu_lc_global_ctrl2.lc_blk_vnum + 1;

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
    d_vc_global_ctrl_reg.m_vc_en        = 1;
    d_vc_global_ctrl_reg.write_enable_11 = 1;
    d_vc_global_ctrl_reg.m_lc_comp_en = 1;
    d_vc_global_ctrl_reg.write_enable_12 = 1;


    IoReg_Write32(LC_LC_Global_Ctrl1_reg, lc_global_ctrl1_reg.regValue);
    IoReg_Write32(COLOR_D_VC_Global_CTRL_reg, d_vc_global_ctrl_reg.regValue);
    IoReg_Write32(LC_LC_Inter_HCtrl2_reg, lc_inter_hctrl2_reg.regValue);
    IoReg_Write32(LC_LC_Inter_VCtrl2_reg, lc_inter_vctrl2_reg.regValue);
    IoReg_Write32(LC_LC_Global_Ctrl2_reg, lc_global_ctrl2_reg.regValue);
    IoReg_Write32(LC_LC_Global_Ctrl3_reg, lc_global_ctrl3_reg.regValue);
    IoReg_Write32(LC_LC_Inter_HCtrl1_reg, lc_inter_hctrl1_reg.regValue);
    IoReg_Write32(LC_LC_Inter_VCtrl1_reg, lc_inter_vctrl1_reg.regValue);
    IoReg_Write32(LC_LC_Global_Ctrl0_reg, lc_global_ctrl0_reg.regValue);

    return 1;

}


unsigned int drvif_color_SLD_set_noise_pattern(unsigned int temporal_diff, unsigned int big_temporal_diff_low, unsigned int big_temporal_diff_up, unsigned int  temporal_counter_thl)
{
    DRV_SLD_sld_temporal_noise_1 temporal_noise_1_data;
    color_sld_sld_temporal_noise_2_RBUS sld_temporal_noise_2;

    temporal_noise_1_data.temporal_diff = temporal_diff;
    temporal_noise_1_data.big_temporal_diff_low = big_temporal_diff_low;
    temporal_noise_1_data.big_temporal_diff_up = big_temporal_diff_up;
    temporal_noise_1_data.temporal_counter_thl = temporal_counter_thl;

    drvif_color_set_SLD_sld_temporal_noise((DRV_SLD_sld_temporal_noise_1 *) &temporal_noise_1_data);

    sld_temporal_noise_2.regValue = rtd_inl(COLOR_SLD_SLD_Temporal_Noise_2_reg);
    //rtd_pr_vpq_info("temporal_counter = %d\n",sld_temporal_noise_2.temporal_counter);

    return sld_temporal_noise_2.temporal_counter;

}


void fwif_color_apply_NEWS_setting(void)
{

    DRV_SLD_sld_Y_curve_map sld_Y_curve_data;
    color_sld_sld_fw_control_0_RBUS  fw_control_0;
    //color_sld_sld_fw_control_1_RBUS    fw_control_1;


    //fw control
    fw_control_0.regValue                   = IoReg_Read32(COLOR_SLD_SLD_FW_control_0_reg);
    //fw_control_1.regValue                     = IoReg_Read32(COLOR_SLD_SLD_FW_control_1_reg              );

    fw_control_0.fw_idx_ctrl_step           = 3;
    fw_control_0.fw_idx_ctrl_en             = 1;
    IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg, fw_control_0.regValue);

    //Y curve setting
    sld_Y_curve_data.sld_curve_seg_0 = 512;
    sld_Y_curve_data.sld_curve_seg_1 = 768;
    sld_Y_curve_data.sld_curve_seg_2 = 800;

    sld_Y_curve_data.sld_new_curve_gain_0 = 64;
    sld_Y_curve_data.sld_new_curve_gain_1 = 64;
    sld_Y_curve_data.sld_new_curve_gain_2 = 64;
    sld_Y_curve_data.sld_new_curve_gain_3 = 5;

    sld_Y_curve_data.sld_new_curve_offset_0 = 0;
    sld_Y_curve_data.sld_new_curve_offset_1 = 0;
    sld_Y_curve_data.sld_new_curve_offset_2 = 0;
    sld_Y_curve_data.sld_new_curve_offset_3 = 733;

    drvif_color_set_SLD_Y_curve_map((DRV_SLD_sld_Y_curve_map *) &sld_Y_curve_data);

}

void  fwif_color_SLD_disable_window(void)
{
    DRV_SLD_sld_control sld_control_data;

    sld_control_data.sld_window_en = 0;

    drvif_color_set_SLD_window_en((DRV_SLD_sld_control *) &sld_control_data);

}

void    fwif_color_set_SLD_LCblk_inform(void)
{
    static unsigned char flag_SLD_init_LCblk = 0;

    if (flag_SLD_init_LCblk == 0) {
        flag_SLD_init_LCblk = drvif_color_set_SLD_LCblk_inform();
    }

}

unsigned int drvif_color_SLD_get_noise_pattern_counter(void)
{
    color_sld_sld_temporal_noise_2_RBUS sld_temporal_noise_2;

    sld_temporal_noise_2.regValue = rtd_inl(COLOR_SLD_SLD_Temporal_Noise_2_reg);
    rtd_pr_vpq_info("temporal_counter = %d\n", sld_temporal_noise_2.temporal_counter);

    return sld_temporal_noise_2.temporal_counter;

}

unsigned char drvif_color_Get_SLD_swap_location(void)
{
	ppoverlay_display_timing_ctrl2_RBUS ppoverlay_display_timing_ctrl2_reg;
	demura_demura_control_RBUS demura_demura_control_reg;
	unsigned char isAfter_OSD;
	
	ppoverlay_display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	demura_demura_control_reg.regValue = IoReg_Read32(DEMURA_DEMURA_CONTROL_reg);

	if (demura_demura_control_reg.demura_location_swap2 == 1 && ppoverlay_display_timing_ctrl2_reg.sld_after_osd == 0)
		isAfter_OSD = 0;
	else
		isAfter_OSD = 1;
	return isAfter_OSD;
}

void drvif_color_set_SLD_swap_location(unsigned char after_osd_en){
	ppoverlay_display_timing_ctrl2_RBUS ppoverlay_display_timing_ctrl2_reg;
	demura_demura_control_RBUS demura_demura_control_reg;

	/*Swap HW SLD location*/
	ppoverlay_display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	ppoverlay_display_timing_ctrl2_reg.sld_after_osd = after_osd_en ;
	IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, ppoverlay_display_timing_ctrl2_reg.regValue);

	/*Swap demura location*/
	demura_demura_control_reg.regValue = IoReg_Read32(DEMURA_DEMURA_CONTROL_reg);
	if(after_osd_en){
		demura_demura_control_reg.demura_location_swap2 = 0;
		demura_demura_control_reg.demura_location_swap = 0;
	}else{
		demura_demura_control_reg.demura_location_swap2 = 1;
	}
	IoReg_Write32(DEMURA_DEMURA_CONTROL_reg, demura_demura_control_reg.regValue);
}


#endif


#if 0/*SLD, hack, elieli*/
#define SLD_Debug 0

bool drvif_set_sld_dma(void)
{
    unsigned int sld_size = 0;
    unsigned int sld_addr = 0;
    unsigned int align_value = 0;

    if (drvif_color_get_HW_MEX_STATUS(MEX_SLD) != 1) {
        rtd_pr_vpq_info("HW_MEX,  MEX_SLD != 1, DMA\n");
        return false;
    }

    sld_size = carvedout_buf_query(CARVEDOUT_SCALER_OD, (void *) &sld_addr);
    if (sld_addr == 0 || sld_size == 0) {
        VIPrtd_pr_vpq_info("[%s %d] ERR : %x %x\n", __func__, __LINE__, sld_addr, sld_size);
        return false;
    }

#if 1
    align_value = dvr_memory_alignment((unsigned long)sld_addr, dvr_size_alignment(500000)); //96 Alignment for Bandwidth Request

    if (align_value == 0) {
        BUG();
    }
#else
    align_value = sld_addr; //origin value
#endif

    VIPprintf("aligned phy addr=%x,\n", align_value);


    IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_addr_0_reg, align_value); //must be check 96 alignment
    IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_addr_1_reg, align_value + 0x02f00000); //Cap_sta_addr_0, must be check 96 alignment

    IoReg_Write32(OD_DMA_ODDMA_WR_Rule_check_up_reg, align_value + 0x02f00000 + 0x2f000000); //Cap_up_limit
    IoReg_Write32(OD_DMA_ODDMA_WR_Rule_check_low_reg, sld_addr);
    IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_addr_0_reg, align_value);//must be check 96 alignment IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_addr_0_reg, align_value);//must be check 96 alignment
    IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_addr_1_reg, align_value + 0x02f00000); //Cap_sta_addr_0, must be check 96 alignment

    IoReg_Write32(OD_DMA_ODDMA_RD_Rule_check_up_reg, align_value + 0x02f00000 + 0x2f000000); //Disp_up_limit
    IoReg_Write32(OD_DMA_ODDMA_RD_Rule_check_low_reg, sld_addr);

    VIPprintf("[sld]drvif_set_sld_dma...end\n");

    return true;
}

// Function Description : This function is for SLD enable
//juwen, 0603, done
void SLD_k6_HAL(int Y1, int Y2, int Y3, int gain_low, unsigned char UIsel_L)
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
    color_sld_control.sld_blend_en = 1;
    color_sld_control.sld_global_drop = 0;
    color_sld_control.sld_hpf_type = 0;
    color_sld_control.sld_hpf_thl = 255;
    color_sld_control.sld_scale_mode = 1;
    color_sld_drop.sld_cnt_thl = 0;
    color_sld_drop.sld_hpf_diff_thl = 16;
    color_sld_drop.sld_pixel_diff_thl = 16;


    IoReg_Write32(COLOR_SLD_SLD_CONTROL_reg, color_sld_control.regValue);
    IoReg_Write32(COLOR_SLD_SLD_DROP_reg, color_sld_drop.regValue);





    if (UIsel_L == 1) {
        y_out_target[0] = (0    * gain_low) / 1024;
        y_out_target[1] = (Y1 * gain_low) / 1024;
        y_out_target[2] = (Y2 * gain_low) / 1024;
        y_out_target[3] = (Y3 * gain_low) / 1024;
    }
    else {
        y_out_target[0] = (0   * 1024) / 1024;
        y_out_target[1] = (Y1 * 1024) / 1024;
        y_out_target[2] = (Y2 * 1024) / 1024;
        y_out_target[3] = (Y3 * 1024) / 1024;

    }

    reg_offset0 = y_out_target[0];
    reg_gain0 = ((y_out_target[1] - reg_offset0) * 64) / reg_seg0;
    reg_gain1 = ((y_out_target[2] - reg_offset0 - ((reg_gain0 * reg_seg0) / 64)) * 64) / (reg_seg1 - reg_seg0);
    reg_gain2 = ((y_out_target[3] - reg_offset0 - ((reg_gain0 * reg_seg0) / 64) - ((reg_gain1 * (reg_seg1 - reg_seg0)) / 64)) * 64) / (1023 - reg_seg1);

    reg_offset1 =  reg_offset0 + (reg_gain0 * reg_seg0) / 64 - (reg_gain1 * reg_seg0) / 64;
    reg_offset2 =  reg_offset1 + (reg_gain1 * reg_seg1) / 64 - (reg_gain2 * reg_seg1) / 64;

    sld_curve_map.sld_curve_seg_0 = 256;
    sld_curve_map.sld_curve_seg_1 = 768;

    if (reg_gain0 < 0) {
        sld_curve_map.sld_curve_gain_0 = 256 - (short)reg_gain0;
    }
    else {
        sld_curve_map.sld_curve_gain_0 = (short)reg_gain0;
    }
    if (reg_gain1 < 0) {
        sld_curve_map.sld_curve_gain_1 = 256 - (short)reg_gain1;
    }
    else {
        sld_curve_map.sld_curve_gain_1 = (short)reg_gain1;
    }
    if (reg_gain2 < 0) {
        sld_curve_map.sld_curve_gain_2 = 256 - (short)reg_gain2;
    }
    else {
        sld_curve_map.sld_curve_gain_2 = (short)reg_gain2;
    }


    if (reg_offset0 < 0) {
        sld_curve_map.sld_curve_offset_0 = 2048 - (short)reg_offset0;
    }
    else {
        sld_curve_map.sld_curve_offset_0 = (short)reg_offset0;
    }

    if (reg_offset1 < 0) {
        sld_curve_map.sld_curve_offset_1 = 2048 - (short)reg_offset1;
    }
    else {
        sld_curve_map.sld_curve_offset_1 = (short)reg_offset1;
    }

    if (reg_offset2 < 0) {
        sld_curve_map.sld_curve_offset_2 = 2048 - (short)reg_offset2;
    }
    else {
        sld_curve_map.sld_curve_offset_2 = (short)reg_offset2;
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
     od_dma_oddma_rd_rule_check_up_RBUS  od_dma_oddma_rd_rule_check_up;
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

    color_sld_control.sld_en = enable;

#ifdef SLD_Debug
    VIPprintf("sld_valid = %d\n", color_sld_control.sld_en);
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
    color_sld_control.sld_blend_en = ptr->sld_blend_en;
    color_sld_control.sld_global_drop = ptr->sld_global_drop;
    color_sld_control.sld_hpf_type = ptr->sld_hpf_type;
    color_sld_control.sld_hpf_thl = ptr->sld_hpf_thl;
    color_sld_control.sld_scale_mode = ptr->sld_scale_mode;

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

    IoReg_Write32(COLOR_SLD_SLD_SIZE_reg, color_sld_size.regValue);
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
    color_sld_curve_map_gain.regValue = IoReg_Read32(COLOR_SLD_SLD_CURVE_MAP_GAIN_reg);
    color_sld_curve_map_offset_0.regValue = IoReg_Read32(COLOR_SLD_SLD_CURVE_MAP_OFFSET_0_reg);
    color_sld_curve_map_offset_1.regValue = IoReg_Read32(COLOR_SLD_SLD_CURVE_MAP_OFFSET_1_reg);

    color_sld_curve_map_seg.sld_curve_seg_0 = ptr->sld_curve_seg_0;
    color_sld_curve_map_seg.sld_curve_seg_1 =  ptr->sld_curve_seg_1;

    color_sld_curve_map_gain.sld_curve_gain_0 =     ptr->sld_curve_gain_0;
    color_sld_curve_map_gain.sld_curve_gain_1 =     ptr->sld_curve_gain_1;
    color_sld_curve_map_gain.sld_curve_gain_2 =     ptr->sld_curve_gain_2;

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

#if 0
    /* DUMPER RELATED FUNCTIONS */
#endif

char linedata[961] = {0};   /* 120 byte + '\n'*/
#if 0
static struct file *file_open(const char *path, int flags, int rights)
{
    struct file *filp = NULL;
    mm_segment_t oldfs;
    int err = 0;

    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open(path, flags, rights);
    set_fs(oldfs);
    if (IS_ERR(filp)) {
        err = PTR_ERR(filp);
        return NULL;
    }
    return filp;
}

static void file_close(struct file *file)
{
    filp_close(file, NULL);
}

static int file_write(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size)
{
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_write(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
}

static int file_sync(struct file *file)
{
    vfs_fsync(file, 0);
    return 0;
}
#endif
#if 0
    /* GLOBAL PARAMETERS */
#endif

unsigned int LD_size = 0;
unsigned char *LD_virAddr = NULL;
sld_work_struct sld_work;
unsigned char logo_flag_map_raw[192 * 540] = {0};
unsigned char logo_flag_map[481 * 271] = {0};
unsigned char logo_flag_map_no_fill_hole[481 * 271] = {0};
unsigned char logo_flag_map_buf[481 * 271] = {0};
short logo_demura_counter[481 * 271] = {0};
short demura_tbl[5][481 * 271];
unsigned char fill_hole_th = 4;
extern VIP_DeMura_TBL DeMura_TBL;
extern DRV_DeMura_CTRL_TBL DeMura_CTRL_TBL_SW_SLD;
unsigned char is_background[10000] = {0};
unsigned short eq_table[10000][2] = {0};
unsigned char eq_searched[10000] = {0};
unsigned char SLD_SW_En = 0;
unsigned char MEMC_Pixel_LOGO_For_SW_SLD = 0;
unsigned char bPictureEnabled = 0;
unsigned char LSC_by_memc_logo_en = 1;
unsigned char ONOFFwithHWSLD = 1;
int memc_logo_to_demura_wait_time = 5; // test
int memc_logo_to_demura_drop_time = 10; // test
int memc_logo_to_demura_drop_limit = 128; // test
int memc_logo_to_demura_drop_limit_gain = 384;	// 10 bit gain 
int memc_logo_to_demura_drop_limit_gain_SP = 1030;	// 10 bit gain 
int memc_logo_to_demura_ascend_speed = 20;
int memc_logo_to_demura_APL_max_th = 1040;
unsigned char circle4_flag = 0;
unsigned int memc_logo_to_demura_update_cnt = 4;
unsigned char memc_logo_read_en = 1;
unsigned char logo_to_demura_en = 1;
unsigned char memc_logo_fill_hole_en = 1;
unsigned char memc_logo_filter_en = 0;
unsigned char memc_logo_filter_SLD_APL_SE_en = 0; 
unsigned char demura_write_en = false;
unsigned char SLD_BY_MEMC_DEMURA_LOGO_ONLY = 0;
unsigned char Gloabal_SLD_En = 1;
unsigned char sld_ddr_offset_auto_get = 1;
unsigned int sld_ddr_offset = 0;
unsigned char memc_logo_LD_APL_check = 3;
extern unsigned int blk_apl_average[ 32 * 18 ];
unsigned int blk_apl_maxfilter[32 * 18];
unsigned int blk_apl_interp[481 * 271];
unsigned int blk_apl_interp_pre[481 * 271] = {0};
unsigned short SLD_APL_reset_cnt_th = 350; 
unsigned short blk_apl_SLD[481 * 271]; 



const char DecDig[10] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

extern UINT8 v4l2_vpq_ire;

#if 0
    /* DUMP FUNCTIONS */
#endif

void short_to_char(short input, char *rs)
{
    // convert a 16bits(-32768~32767) digit to char array with lenght 6
    int data = input;
    rs[0] = (data >= 0) ? ' ' : '-';
    if (data < 0) {
        data = -data;
    }
    rs[1] = DecDig[(data / 10000) % 10];
    rs[2] = DecDig[(data / 1000) % 10];
    rs[3] = DecDig[(data / 100) % 10];
    rs[4] = DecDig[(data / 10) % 10];
    rs[5] = DecDig[(data / 1) % 10];
}

void bit_to_char(char org_byte, char *rs)
{
    UINT8 i;
    for (i = 0; i < 8; i++) {
        *(rs + i) = (org_byte & (1 << (7 - i))) ? '1' : '0';
    }
}
#if 0
void dump_sld_calc_buffer_to_file(void)
{
    struct file *filp = NULL;
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
    if (filp == NULL) {
        rtd_pr_vpq_info("[%s] open fail\n", filename);
        return;
    }

    for (lineCnt = 0; lineCnt < 540; lineCnt++) {
        for (dataCnt = 0; dataCnt < 192; dataCnt++) {
            bit_to_char(logo_flag_map_raw[lineCnt * 192 + dataCnt], rs2);
            file_write(filp, outfileOffset, (unsigned char *)rs2, 8);
            outfileOffset = outfileOffset + 8;
        }
        file_write(filp, outfileOffset, (unsigned char *)"\n", 1);
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
    if (filp == NULL) {
        rtd_pr_vpq_info("[%s] open fail\n", filename);
        return;
    }

    for (lineCnt = 0; lineCnt < 271; lineCnt++) {
        for (dataCnt = 0; dataCnt < 481; dataCnt++) {
            if (logo_flag_map[lineCnt * 481 + dataCnt] != 0) {
                file_write(filp, outfileOffset, (unsigned char *)"1", 1);
            }
            else {
                file_write(filp, outfileOffset, (unsigned char *)"0", 1);
            }
            outfileOffset = outfileOffset + 1;
        }
        file_write(filp, outfileOffset, (unsigned char *)"\n", 1);
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
    if (filp == NULL) {
        rtd_pr_vpq_info("[%s] open fail\n", filename);
        return;
    }

    for (lineCnt = 0; lineCnt < 271; lineCnt++) {
        for (dataCnt = 0; dataCnt < 481; dataCnt++) {
            if (logo_flag_map_buf[lineCnt * 481 + dataCnt] != 0) {
                file_write(filp, outfileOffset, (unsigned char *)"1", 1);
            }
            else {
                file_write(filp, outfileOffset, (unsigned char *)"0", 1);
            }
            outfileOffset = outfileOffset + 1;
        }
        file_write(filp, outfileOffset, (unsigned char *)"\n", 1);
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
    if (filp == NULL) {
        rtd_pr_vpq_info("[%s] open fail\n", filename);
        return;
    }

    for (lineCnt = 0; lineCnt < 271; lineCnt++) {
        for (dataCnt = 0; dataCnt < 481; dataCnt++) {
            short_to_char(logo_demura_counter[lineCnt * 481 + dataCnt], rs);
            file_write(filp, outfileOffset, (unsigned char *)rs, 6);
            outfileOffset = outfileOffset + 6;
            file_write(filp, outfileOffset, (unsigned char *)" ", 1);
            outfileOffset = outfileOffset + 1;
        }
        file_write(filp, outfileOffset, (unsigned char *)"\n", 1);
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
    if (filp == NULL) {
        rtd_pr_vpq_info("[%s] open fail\n", filename);
        return;
    }

    for (lineCnt = 0; lineCnt < 271; lineCnt++) {
        for (dataCnt = 0; dataCnt < 481; dataCnt++) {
            short_to_char(demura_tbl[0][lineCnt * 481 + dataCnt], rs);
            file_write(filp, outfileOffset, (unsigned char *)rs, 6);
            outfileOffset = outfileOffset + 6;
            file_write(filp, outfileOffset, (unsigned char *)" ", 1);
            outfileOffset = outfileOffset + 1;
        }
        file_write(filp, outfileOffset, (unsigned char *)"\n", 1);
        outfileOffset = outfileOffset + 1;
    }
    file_sync(filp);
    file_close(filp);
    msleep(10);

    sprintf(filename, "/tmp/usb/sda/sda1/LD/sld_demura_tbl_md_%04d.txt", fc);
    filp = file_open(filename, O_RDWR | O_CREAT | O_APPEND, 0);
    if (filp == NULL) {
        rtd_pr_vpq_info("[%s] open fail\n", filename);
        return;
    }

    for (lineCnt = 0; lineCnt < 271; lineCnt++) {
        for (dataCnt = 0; dataCnt < 481; dataCnt++) {
            short_to_char(demura_tbl[1][lineCnt * 481 + dataCnt], rs);
            file_write(filp, outfileOffset, (unsigned char *)rs, 6);
            outfileOffset = outfileOffset + 6;
            file_write(filp, outfileOffset, (unsigned char *)" ", 1);
            outfileOffset = outfileOffset + 1;
        }
        file_write(filp, outfileOffset, (unsigned char *)"\n", 1);
        outfileOffset = outfileOffset + 1;
    }
    file_sync(filp);
    file_close(filp);
    msleep(10);

    sprintf(filename, "/tmp/usb/sda/sda1/LD/sld_demura_tbl_hi_%04d.txt", fc);
    filp = file_open(filename, O_RDWR | O_CREAT | O_APPEND, 0);
    if (filp == NULL) {
        rtd_pr_vpq_info("[%s] open fail\n", filename);
        return;
    }

    for (lineCnt = 0; lineCnt < 271; lineCnt++) {
        for (dataCnt = 0; dataCnt < 481; dataCnt++) {
            short_to_char(demura_tbl[2][lineCnt * 481 + dataCnt], rs);
            file_write(filp, outfileOffset, (unsigned char *)rs, 6);
            outfileOffset = outfileOffset + 6;
            file_write(filp, outfileOffset, (unsigned char *)" ", 1);
            outfileOffset = outfileOffset + 1;
        }
        file_write(filp, outfileOffset, (unsigned char *)"\n", 1);
        outfileOffset = outfileOffset + 1;
    }
    file_sync(filp);
    file_close(filp);
    msleep(10);
#endif

}

void dump_logo_detect_to_file(unsigned char start_byte)
{
    struct file *filp = NULL;
    unsigned long outfileOffset = 0;
    unsigned long ddr_offset = start_byte;  /* de-garbage */
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

        LD_virAddr = LD_virAddr + start_offset_byte;

        while (outfileOffset < (LD_size - 1920/* 1920: 10 line */)) {
            unsigned int i = 0;
            while (i < 960) {
                char rs[8];
                unsigned char j;
                bit_to_char(*(LD_virAddr + ddr_offset), rs);
                for (j = 0; j < 8; j++) {
                    linedata[i + j] = rs[j];
                }
                ddr_offset++;
                i = i + 8;
            }
            ddr_offset = ddr_offset + (192 - 120); /* garbage */
            if (file_line_count < 540) {
                file_write(filp, (outfileOffset / 192) * 961 + 1, (unsigned char *)linedata, 961);
                file_line_count ++;
            }
            outfileOffset = outfileOffset + 192;/*192 byte per line*/
        }

        //file_write(filp, outfileOffset, (unsigned char*)LD_virAddr, LD_size);
        file_sync(filp);
        file_close(filp);
    }
    else {
        rtd_pr_vpq_info("dump fail\n");
    }

    return;
}
#endif
#if 0
    /* MAIN FUNCTIONS */
#endif

extern UINT32 MEMC_Lib_GetInfoForSLD(unsigned char infoSel);

void fwif_color_SLD_apply_normal_setting(void)
{
	//ppoverlay_display_timing_ctrl2_RBUS ppoverlay_display_timing_ctrl2_reg;
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
    DRV_SLD_sld_fw_control_2  fw_control_max_logo_tolerance_data;
	DRV_SLD_sld_dilation_ctrl sld_dilation_ctrl;
	DRV_SLD_sld_8_dir_fill_hole_thl sld_8_dir_fill_hole_thl;	
	//ppoverlay_display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	//ppoverlay_display_timing_ctrl2_reg.sld_after_osd = 0 ;
	//IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2, ppoverlay_display_timing_ctrl2_reg.regValue);
	//IoReg_SetBits(PPOVERLAY_Display_Timing_CTRL2, PPOVERLAY_Display_Timing_CTRL2_sld_after_osd_mask);
	//IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2,PPOVERLAY_Display_Timing_CTRL2_sld_after_osd(1)  );
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
    sld_control.sld_blend_en = 1;
    sld_control.sld_debug_mode = 0;

    //drvif_color_set_SLD_control((DRV_SLD_sld_control *) &sld_control); //kuan set in scalerColor.c
    /*add---------------------------------------------------------------------------------------*/
    //add condition
    sld_acc_0_data.sld_idx_acc_condition_and = 1;
    sld_acc_0_data.sld_hpf_logo_add_thl = 128;
    sld_acc_0_data.sld_frame_diffy_thl = 256;
    sld_acc_0_data.sld_y_idx_thl = 256;
    sld_acc_0_data.sld_max_idx_thl = 4096;
	sld_acc_0_data.sld_hpf_gain_type = 3;
    drvif_color_set_SLD_Idx_add_con((DRV_SLD_sld_acc_0 *) &sld_acc_0_data);

    //add step control
    sld_hpf_1.sld_idx_sign_0 = 0;
    sld_hpf_1.sld_idx_step_0 = 1;
    sld_hpf_1.sld_idx_sign_2 = 1;
    sld_hpf_1.sld_idx_step_2 = 6;

    drvif_color_set_SLD_step((DRV_SLD_sld_hpf_1 *) &sld_hpf_1); //kuan set in scalerColor.c

	/*idx dilation type*/
	sld_dilation_ctrl.sld_dilat_type = 1;
	drvif_color_set_SLD_Dilation_Type((DRV_SLD_sld_dilation_ctrl*) &sld_dilation_ctrl);

    //control minus step
    sld_drop_slow_0.idx_slow_down_l      = 1500;
    sld_drop_slow_0.idx_slow_down_h     = 4095;
    sld_drop_slow_0.idx_slow_down_step = 64;

    drvif_color_set_SLD_control_minus_step((DRV_SLD_sld_drop_slow_0 *) &sld_drop_slow_0);

    //temporal noise
    sld_temporal_noise_1.big_temporal_diff_up = 400;
    sld_temporal_noise_1.big_temporal_diff_low = 0;
    sld_temporal_noise_1.temporal_counter_thl = 129600;//disable
    sld_temporal_noise_1.temporal_diff = 400;

    drvif_color_set_SLD_sld_temporal_noise((DRV_SLD_sld_temporal_noise_1 *) &sld_temporal_noise_1);

    //8 directions 12 tab
    sld_in_logo_thl_0.sld_inter_thl_0 = 64;
    sld_in_logo_thl_0.sld_inter_thl_1 = 64;
    sld_in_logo_thl_0.sld_inter_thl_2 = 64;
    sld_in_logo_thl_0.sld_inter_thl_3 = 64;
    sld_in_logo_thl_0.sld_inter_thl_4 = 64;
    sld_in_logo_thl_0.sld_inter_thl_5 = 64;
    sld_in_logo_thl_0.sld_inter_thl_6 = 64;
    sld_in_logo_thl_0.sld_inter_thl_7 = 64;
    sld_in_logo_thl_0.sld_inter_thl_8 = 64;
    sld_in_logo_thl_0.sld_inter_thl_9 = 64;
    sld_in_logo_thl_0.sld_inter_thl_10 = 64;
    sld_in_logo_thl_0.sld_inter_thl_11 = 64;

    drvif_color_set_SLD_8dir_12tab((DRV_SLD_sld_in_logo_thl_0 *) &sld_in_logo_thl_0);

	/*8 direction fill hole thl*/
	sld_8_dir_fill_hole_thl.sld_8_dir_fill_hole_thl = 7;
	drvif_color_set_SLD_8_Dir_fillhole_thl((DRV_SLD_sld_8_dir_fill_hole_thl*) &sld_8_dir_fill_hole_thl);

    //T noise settings
    sld_noise_pattern_0.check_drop_pattern_start = 4096;
    sld_noise_pattern_0.detect_tnoise_en = 0;
    sld_noise_pattern_0.diffavg = 160;
    sld_noise_pattern_0.grouph_diffmaxmin = 1;
    sld_noise_pattern_0.patternedge_hpf = 12 ;
    sld_noise_pattern_0.patternedge_y = 600;
    sld_noise_pattern_0.patternregion_hpf = 208;
    sld_noise_pattern_0.patternregion_y = 600;

    drvif_color_set_SLD_LGD_Tnoise_pattern((DRV_SLD_sld_noise_pattern_0 *) &sld_noise_pattern_0);

    /*compensate---------------------------------------------------------------------------------------*/
    //temporal compensate
    sld_temporal_idx_compensate.do_connect_seg_idx_lower_bound = 10;
    sld_temporal_idx_compensate.idx_period_check_once = 31;

    drvif_color_set_SLD_compensate_temporal((DRV_SLD_sld_temporal_idx_compensate *) &sld_temporal_idx_compensate);

    //spatial compensate
    sld_idx_compensate_by_neighbor_0.idx_compensate_start_thl = 1800;
    sld_idx_compensate_by_neighbor_0.compensate_neighbor_tolerance = 500;
    sld_idx_compensate_by_neighbor_0.idx_compensate_neighbor_counter_thl = 9;

    drvif_color_set_SLD_compensate_spatial((DRV_SLD_sld_idx_compensate_by_neighbor_0 *) &sld_idx_compensate_by_neighbor_0);

    /*drop---------------------------------------------------------------------------------------*/
    sld_drop.sld_cnt_thl = 0;

    sld_drop.sld_hpf_diff_thl = 128;
    sld_drop.sld_pixel_diff_thl = 96;

    sld_idx_not_drop_0.in_logo_idx_thl = 1023;
    sld_idx_not_drop_0.hpf_diff_thl_for_logo_region = 512;
    sld_idx_not_drop_0.pixel_diff_thl_for_logo_region = 80;

    sld_neighbor_not_drop.neighbor_diffy_notdrop_thl = 0;

    sld_neighbor_drop_0.neighbor0_diffy_drop_thl = 0;
    sld_neighbor_drop_0.neighbor1_diffy_drop_thl = 1023;
    sld_neighbor_drop_0.neighbor2_diffy_drop_thl = 1023;

    sld_idx_protect_by_neighbor_0.idx_drop_protect_idx_thl = 1023;
    sld_idx_protect_by_neighbor_0.idx_drop_protect_idx_neighbor_tolerance = 1023;
    sld_idx_protect_by_neighbor_0.idx_drop_protect_counter_neighbor_sameidx = 9;

    drvif_color_set_SLD_drop((DRV_SLD_sld_drop *) &sld_drop, (DRV_SLD_sld_idx_not_drop_0 *) &sld_idx_not_drop_0, (DRV_SLD_sld_neighbor_not_drop *) &sld_neighbor_not_drop, (DRV_SLD_sld_neighbor_drop_0 *) &sld_neighbor_drop_0, (DRV_SLD_sld_idx_protect_by_neighbor_0 *) &sld_idx_protect_by_neighbor_0);


    /*FW control setting---------------------------------------------------------------------------------------*/

    sld_fw_control_0.fw_idx_ctrl_en = 0;
    sld_fw_control_0.fw_idx_ctrl_step = 1023;
    sld_fw_control_0.fw_idx_freeze_for_full_frame = 0;
    sld_fw_control_0.fw_pixel_gain = 4096;

    drvif_color_set_SLD_fw_control((DRV_SLD_sld_fw_control_0 *) &sld_fw_control_0);

    /*LPF*/

    sld_lpf_0_data.coef_0_0 = 1;
    sld_lpf_0_data.coef_0_1 = 4;
    sld_lpf_0_data.coef_0_2 = 6;
    sld_lpf_0_data.coef_0_3 = 4;
    sld_lpf_0_data.coef_0_4 = 1;

    sld_lpf_0_data.coef_1_0 = 4;
    sld_lpf_0_data.coef_1_1 = 16;
    sld_lpf_0_data.coef_1_2 = 24;
    sld_lpf_0_data.coef_1_3 = 16;
    sld_lpf_0_data.coef_1_4 = 4;

    sld_lpf_0_data.coef_2_0 = 6;
    sld_lpf_0_data.coef_2_1 = 24;
    sld_lpf_0_data.coef_2_2 = 36;
    sld_lpf_0_data.coef_2_3 = 24;
    sld_lpf_0_data.coef_2_4 = 6;

    sld_lpf_0_data.coef_3_0 = 4;
    sld_lpf_0_data.coef_3_1 = 16;
    sld_lpf_0_data.coef_3_2 = 24;
    sld_lpf_0_data.coef_3_3 = 16;
    sld_lpf_0_data.coef_3_4 = 4;

    sld_lpf_0_data.coef_4_0 = 1;
    sld_lpf_0_data.coef_4_1 = 4;
    sld_lpf_0_data.coef_4_2 = 6;
    sld_lpf_0_data.coef_4_3 = 4;
    sld_lpf_0_data.coef_4_4 = 1;

    sld_lpf_0_data.lpf_shift_bit = 8;
    drvif_color_set_SLD_lpf((DRV_SLD_sld_lpf_0 *) &sld_lpf_0_data);

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

    drvif_color_set_SLD_sld_histogram((DRV_SLD_sld_histogram_1 *) &sld_histogram_1, (DRV_SLD_sld_histogram_0 *) &sld_histogram_0);

    /*window---------------------------------------------------------------------------------------*/
    sld_apply_window_0.sld_window_apply_out = 0;
    sld_apply_window_0.sld_window_pos0_x = 0;
    sld_apply_window_0.sld_window_pos0_y = 0;
    sld_apply_window_0.sld_window_pos1_x = 3839;
    sld_apply_window_0.sld_window_pos1_y = 2159;

    drvif_color_set_SLD_sld_window((DRV_SLD_sld_apply_window_0 *) &sld_apply_window_0);

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
	
    /*sld_idx_map_seg.sld_idx_seg_0 = 512; 
    sld_idx_map_seg.sld_idx_seg_1 = 1600;
    sld_idx_map_seg.sld_idx_seg_2 = 1800;
    sld_idx_map_seg.sld_new_idx_gain_0 = 0 ;
    sld_idx_map_seg.sld_new_idx_gain_1 = 0;
    sld_idx_map_seg.sld_new_idx_gain_2 = 115;
    sld_idx_map_seg.sld_new_idx_gain_3 = 104;
    sld_idx_map_seg.sld_new_idx_offset_0 = 0;
    sld_idx_map_seg.sld_new_idx_offset_1 = 0;
    sld_idx_map_seg.sld_new_idx_offset_2 = -2880;
    sld_idx_map_seg.sld_new_idx_offset_3 = -2569;*/

    drvif_color_set_SLD_Idx_curve_map((DRV_SLD_sld_idx_map_seg *) &sld_idx_map_seg);


    /*Y curve---------------------------------------------------------------------------------------*/
    /*sld_Y_curve_map.sld_curve_seg_0 = 512;
    sld_Y_curve_map.sld_curve_seg_1 = 768;
    sld_Y_curve_map.sld_curve_seg_2 = 800;

    sld_Y_curve_map.sld_new_curve_gain_0 = 64;
    sld_Y_curve_map.sld_new_curve_gain_1 = 64;
    sld_Y_curve_map.sld_new_curve_gain_2 = 64;
    sld_Y_curve_map.sld_new_curve_gain_3 = 34;

    sld_Y_curve_map.sld_new_curve_offset_0 = 0;
    sld_Y_curve_map.sld_new_curve_offset_1 = 0;
    sld_Y_curve_map.sld_new_curve_offset_2 = 0;
    sld_Y_curve_map.sld_new_curve_offset_3 = 366;
	*/
	
	sld_Y_curve_map.sld_curve_seg_0 = 512;
	sld_Y_curve_map.sld_curve_seg_1 = 768;
	sld_Y_curve_map.sld_curve_seg_2 = 800;

	sld_Y_curve_map.sld_new_curve_gain_0 = 64;
	sld_Y_curve_map.sld_new_curve_gain_1 = 64;
	sld_Y_curve_map.sld_new_curve_gain_2 = 64;
	sld_Y_curve_map.sld_new_curve_gain_3 = 40;//49;

	sld_Y_curve_map.sld_new_curve_offset_0 = 0;
	sld_Y_curve_map.sld_new_curve_offset_1 = 0;
	sld_Y_curve_map.sld_new_curve_offset_2 = 0;
	sld_Y_curve_map.sld_new_curve_offset_3 = 298;//183;
    drvif_color_set_SLD_Y_curve_map((DRV_SLD_sld_Y_curve_map *) &sld_Y_curve_map);
    

	fw_control_max_logo_tolerance_data.max_pre_logo_tolerance = 1800; 
	
	drvif_color_set_SLD_max_pre_logo_tolerance((DRV_SLD_sld_fw_control_2 *) &fw_control_max_logo_tolerance_data);
}


void drif_color_SLD_apply_normal_setting(void)
{
	fwif_color_SLD_apply_normal_setting();
}

void get_osd_pos(void)
{
    //static unsigned int frame_cnt = 0;
    //unsigned char* logo_ptr = LD_virAddr;

    //static unsigned char memc_sc_flag = false;
    //static unsigned int  memc_sc_motion1 = 0, memc_sc_motion2 = 0;

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


    // start to work
    if (!osd_measure_init) {
        osdovl_mixer_ctrl2_reg.regValue = IoReg_Read32(OSDOVL_Mixer_CTRL2_reg);
        osdovl_osd_db_ctrl_reg.regValue = IoReg_Read32(OSDOVL_OSD_Db_Ctrl_reg);

        osdovl_mixer_ctrl2_reg.measure_osd_zone_en = 1;
        osdovl_mixer_ctrl2_reg.measure_osd_zone_type = 1;
        IoReg_Write32(OSDOVL_Mixer_CTRL2_reg, osdovl_mixer_ctrl2_reg.regValue);

        osdovl_osd_db_ctrl_reg.db_load = 1;
        IoReg_Write32(OSDOVL_OSD_Db_Ctrl_reg, osdovl_osd_db_ctrl_reg.regValue);

        osd_measure_init = true;
    }

    osdovl_measure_osd1_sta_reg.regValue = IoReg_Read32(OSDOVL_measure_osd1_sta_reg);
    osdovl_measure_osd1_end_reg.regValue = IoReg_Read32(OSDOVL_measure_osd1_end_reg);
    osd_sta_x = osdovl_measure_osd1_sta_reg.x;
    osd_sta_y = osdovl_measure_osd1_sta_reg.y;
    osd_end_x = osdovl_measure_osd1_end_reg.x;
    osd_end_y = osdovl_measure_osd1_end_reg.y;

    if (osd_sta_x == 0x1fff && osd_sta_y == 0x1fff && osd_end_x == 0 && osd_end_y == 0) { // no OSD shown
        sld_work.osd_sta_blkx = -1;
        sld_work.osd_sta_blky = -1;
        sld_work.osd_end_blkx = -1;
        sld_work.osd_end_blky = -1;
    }
    else { // mark osd blocks
        sld_work.osd_sta_blkx = osd_sta_x / 8;
        sld_work.osd_end_blkx = (osd_end_x + 7) / 8;
        sld_work.osd_sta_blky = osd_sta_y / 8;
        sld_work.osd_end_blky = (osd_end_y + 7) / 8;
    }




    //frame_cnt++;

}

#if 0
void fwif_color_compute_LC_APL_related_K7_FW_LC(void)
{
    unsigned short idx_r = 0, idx_c = 0, idx_arr = 0;
    int num_hblk = 0, num_vblk = 0, counter_blk_pre_cur_same = 0;
    unsigned int counter_highY_blk = 0;
    unsigned int thl_highY_frame = 750;
    color_sld_sld_fw_control_1_RBUS  fw_control_1;
    DRV_SLD_sld_fw_read_only fw_read_only_data;
    color_sld_sld_fw_control_0_RBUS  fw_control_0;
    lc_lc_global_ctrl2_RBUS blu_lc_global_ctrl2;

    cur_frame_idx_global = cur_frame_idx_global % 65535;
    cur_frame_idx_global = cur_frame_idx_global + 1;

#ifdef SLD_MEMC_DISPLAY
    /*memc*/
    rtd_setbits(0xb809d5fc, 0x10000000);//memc

#ifdef SLD_MEMC_DISPLAY
    WriteRegister(0xb809d0A4, 24, 31, lock_fw_gain);//memc display
#endif

    //WriteRegister(0xb809d0A0, 0, 7, (LC_time%255));
    //WriteRegister(0xb809d0A0, 8, 15, 0xff);
    //WriteRegister(0xb809d0A0, 16, 23, 0xff);
    //WriteRegister(0xb809d0A0, 24, 31, 0xff);

    //WriteRegister(0xb809d0A4, 0, 7, 0xff);
    //WriteRegister(0xb809d0A4, 8, 15, 0xff);
    //WriteRegister(0xb809d0A4, 16, 23, 0xff);
    //WriteRegister(0xb809d0A4, 24, 31, 0xff);
#endif

    blu_lc_global_ctrl2.regValue = IoReg_Read32(LC_LC_Global_Ctrl2_reg);

    num_hblk = blu_lc_global_ctrl2.lc_blk_hnum + 1 ;
    num_vblk = blu_lc_global_ctrl2.lc_blk_vnum + 1;

    for (idx_r = 0; idx_r < (num_vblk); idx_r++) {
        for (idx_c = 0; idx_c < (num_hblk); idx_c++) {
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
    if (0) {
        //if(flag_news > 16){
        flag_news = flag_news - 16;
        //fw control step until 0
        fw_control_0.regValue               = IoReg_Read32(COLOR_SLD_SLD_FW_control_0_reg);
        fw_control_0.fw_idx_ctrl_step          = 8192 - 256; //S(13,0)
        fw_control_0.fw_idx_ctrl_en = 1;
        rtd_outl(COLOR_SLD_SLD_FW_control_0_reg, fw_control_0.regValue);
    }
    else if (lock_fw_gain > 0) {
        lock_fw_gain = lock_fw_gain - 1;
        fw_control_1.fw_pixel_gain              = CLAMP_SLD(fw_control_1.fw_pixel_gain - (4096 / cmd_pixel_gain_step), 4095, 0);
        rtd_outl(COLOR_SLD_SLD_FW_control_1_reg, fw_control_1.regValue);
        rtd_pr_vpq_info("[sld] lock_fw_gain = %d,fw_control_1.fw_pixel_gain = %d\n", lock_fw_gain, fw_control_1.fw_pixel_gain);
    }
    /*
    else if(//news condition
        ((fw_read_only_data.hpf_line_count > 0) && (fw_read_only_data.hpf_line_count < 271) && LC_detect_NEWS(&(APL_cur[0])))
        ||(((cur_frame_idx_global - index_NEWS_case) < cmd_thl_error_time_NEWS_case) && (index_NEWS_case != 0))
    )
    {
    #ifdef SLD_MEMC_DISPLAY
            WriteRegister(0xb809d0A0, 0, 7, 0x11);//news condition
    #endif

        if(((fw_read_only_data.hpf_line_count > 0) && (fw_read_only_data.hpf_line_count < 271) && LC_detect_NEWS(&(APL_cur[0])))){
            rtd_pr_vpq_info("[sld] NEWS condition--------------------\n");
            index_NEWS_case = cur_frame_idx_global;
        }else{
            rtd_pr_vpq_info("[sld] NEWS,in time condition----------------\n");
        }

        counter_NEWS_case = counter_NEWS_case+1;

        if(counter_NEWS_case > cmd_thl_counter_NEWS_case){
            fwif_color_apply_NEWS_setting();
            fwif_color_SLD_disable_window();
            flag_news = 255;
        }

    }
    */

    else if ((LC_time % cmd_LC_period) == 0) {
        rtd_pr_vpq_info("[sld] cur_frame_idx_global = %d\n", cur_frame_idx_global);

        drvif_color_get_LC_APL(&(APL_cur[0]));
        LC_compute_pre_cur_deltaY(&(APL_pre[0]), &(APL_cur[0]), &APL_avg_cur, &(APL_delta[0]), &counter_blk_pre_cur_same, cmd_box_lock_frame); //I,I,O
        drvif_color_get_SLD_fw_read_only((DRV_SLD_sld_fw_read_only *) &fw_read_only_data);

        if (lock_fw_gain == 0) {

            //drvif_color_set_SLD_default();
            //fwif_color_SLD_add_con();//add condition add/ or
            //fwif_color_SLD_fw_read_only(cmd_lock_fw_gain_frame);//logo idx > 3000 has 1/4 area  +   spatial compensate

            thl_highY_frame = cmd_dY_preCur_mouseCon;
            LC_count_num_highY_block_upFrame(&(APL_cur[0]), thl_highY_frame, &counter_highY_blk);

#ifdef SLD_MEMC_DISPLAY
            if (counter_highY_blk > 255) {
                counter_highY_blk = 255;
            }
            WriteRegister(0xb809d0A4, 0, 7, counter_highY_blk);//memc display
#endif

            //still-------------------------------------------------------------------------------------------------
            if (counter_blk_pre_cur_same > cmd_thl_counter_blk_pre_cur_same_mouseCon) { //still

                //fw control step until 0
                fw_control_0.regValue  = IoReg_Read32(COLOR_SLD_SLD_FW_control_0_reg);
                fw_control_0.fw_idx_ctrl_step = 8192 - 64; //S(13,0)
                fw_control_0.fw_idx_ctrl_en   = 1;
                IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg, fw_control_0.regValue);
                WriteRegister(0xb809d0A0, 8, 15, 0x11);//memc display
            }
            else { //not still
                //fw control step until 0
                fw_control_0.regValue  = IoReg_Read32(COLOR_SLD_SLD_FW_control_0_reg);
                //fw_control_0.fw_idx_ctrl_step = 8192-64;//S(13,0)
                fw_control_0.fw_idx_ctrl_en   = 0;
                IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg, fw_control_0.regValue);
                WriteRegister(0xb809d0A0, 8, 15, 0x00);//memc display

                //ice decrease Y-------------------------------------------------------------------------------------------
                if (counter_highY_blk > cmd_counter_frame_highY_blk) { //high Y block condition
                    lock_fw_gain = cmd_lock_fw_gain_frame;
#ifdef SLD_MEMC_DISPLAY
                    WriteRegister(0xb809d0A4, 8, 15, 0x11);//memc display
#endif
                }
                else {
                    //fw_control_1.fw_pixel_gain                = 4096;
                    fw_control_1.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_1_reg);
                    fw_control_1.fw_pixel_gain              = CLAMP_SLD(fw_control_1.fw_pixel_gain + (4096 / cmd_pixel_gain_step), 4096, 0);
                    rtd_outl(COLOR_SLD_SLD_FW_control_1_reg, fw_control_1.regValue);
                    rtd_pr_vpq_info("[sld] high Y block condition debug, counter_highY_blk = %d\n", counter_highY_blk);
#ifdef SLD_MEMC_DISPLAY
                    WriteRegister(0xb809d0A4, 8, 15, 0x00);//memc display
#endif

                }





            }


        }

        LC_time = 0;
    }
}
#endif
void fwif_color_logo_add_accl(int *frame, int *stage_1_flag, int *stage_2_flag,
                              unsigned int stage1_period, unsigned int stage2_period, int idx_step)
{
    //DRV_SLD_sld_acc_0 fw_sld_acc_0;
    //DRV_SLD_sld_hpf_1 fw_sld_add_step_ctrl;

    color_sld_sld_hpf_1_RBUS color_sld_sld_hpf_1_reg;
    color_sld_sld_acc_0_RBUS color_sld_sld_acc_0_reg;

    color_sld_sld_hpf_1_reg.regValue = rtd_inl(COLOR_SLD_SLD_HPF_1_reg);
    color_sld_sld_acc_0_reg.regValue = rtd_inl(COLOR_SLD_SLD_ACC_0_reg);



    if (!((*frame % stage1_period) == 0) && (*stage_1_flag == 0)) { //stage 1
        //Do stage1, adjust the logo add edge conditions
        //fw_sld_acc_0.sld_max_idx_thl = 3900;
        //fw_sld_add_step_ctrl.sld_idx_step_0 = idx_step;
        color_sld_sld_acc_0_reg.sld_max_idx_thl = 3900;
        color_sld_sld_hpf_1_reg.sld_idx_step_0 = idx_step;


    }
    else if (!((*frame % stage2_period) == 0) && (*stage_1_flag == 1)) { //stage 2
        //Do stage2, adjust the logo add step to control speed
        //fw_sld_acc_0.sld_max_idx_thl = 4096;
        //fw_sld_add_step_ctrl.sld_idx_step_0 = idx_step;
        color_sld_sld_acc_0_reg.sld_max_idx_thl = 4096;
        color_sld_sld_hpf_1_reg.sld_idx_step_0 = idx_step;

    }
    else {
        //Default setting
    }


    if (((*frame) % stage1_period) == 0 && (*stage_1_flag == 0) && ((*frame) >= stage1_period)) {
        //fw_sld_acc_0.sld_max_idx_thl = 4096;
        color_sld_sld_acc_0_reg.sld_max_idx_thl = 4096;


        *stage_1_flag = 1; //stage_1 is done
        *frame = 0;        //reset frame
    }
    else if (((*frame) % stage2_period) == 0 && (*stage_1_flag == 1) && ((*frame) >= stage2_period)) {
        //fw_sld_add_step_ctrl.sld_idx_step_0 = 0;

        color_sld_sld_hpf_1_reg.sld_idx_step_0 = 0;
        *stage_1_flag = 0;  //reset stage_1
        *stage_2_flag = 1; //stage_2 is done
        *frame = 0;        //reset frame
    }
    rtd_outl(COLOR_SLD_SLD_HPF_1_reg, color_sld_sld_hpf_1_reg.regValue);
    rtd_outl(COLOR_SLD_SLD_ACC_0_reg, color_sld_sld_acc_0_reg.regValue);


    //rtd_pr_vpq_info("[sld]Stage_1_flag:%d, Stage_2_flag:%d \n",*stage_1_flag,*stage_2_flag);
    //rtd_pr_vpq_info("[sld]sld_max_idx_thl:%d, sld_idx_step_0:%d \n",fw_sld_acc_0.sld_max_idx_thl,fw_sld_add_step_ctrl.sld_idx_step_0);

}
							  
void fwif_color_compute_LC_APL_related_FW(void)
	{

    unsigned short idx_r = 0, idx_c = 0, idx_arr = 0;
    int num_hblk = 0, num_vblk = 0, counter_blk_pre_cur_same = 0;
    //int idx_step;
    unsigned int up_Left_counter_highY_blk = 0;
    unsigned int thl_highY_frame = 1600;
    unsigned int up_Left_counter_highY_blk_thl = 80;
    static unsigned int  memc_sc_motion1 = 0, memc_sc_motion2 = 0;
    color_sld_sld_fw_control_1_RBUS  fw_control_1;
    color_sld_sld_fw_control_0_RBUS  fw_control_0;
    lc_lc_global_ctrl2_RBUS blu_lc_global_ctrl2;
    color_sld_sld_curve_map_gain_RBUS curve_map_gain;
    color_sld_sld_hpf_1_RBUS color_sld_sld_hpf_1_reg;
    color_sld_sld_acc_0_RBUS color_sld_sld_acc_0_reg;
    color_sld_sld_cnt_2_RBUS sld_cnt_2_reg;
    //int dcc_scene_change_flag = 0;
    int memc_scene_change_flag = 0;
    int real_scene_change_flag = 0;
	int fflag = 0;
    //unsigned char KKKK = 0;
    DRV_SLD_sld_fw_read_only fw_read_only_data;
    DRV_SLD_sld_fw_control_2  fw_control_max_logo_tolerance_data = {0};
    DRV_SLD_sld_fw_control_0 fw_control_0_data = {0};
    //int stage1_period = 0, stage2_period = 0;
	static unsigned int cnt;
    //int value_dummy1802c700_31_22;
    //_RPC_clues *RPC_SmartPic_clue = NULL;
    //_clues *SmartPic_clue = NULL;
	unsigned short print_en = OPS_CTRL_item.OPS_Debug_Item.Debug_HWSW_SLD_En;
	unsigned short print_delay = OPS_CTRL_item.OPS_Debug_Item.Debug_Delay;

    color_sld_sld_control_RBUS color_sld_control;
    //color_sld_sld_hpf_1_RBUS sld_hpf_1_RBUS;
    color_sld_control.regValue = IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg);


    curve_map_gain.regValue = rtd_inl(COLOR_SLD_SLD_CURVE_MAP_GAIN_reg);
    sld_cnt_2_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_CNT_2_reg);
    color_sld_sld_hpf_1_reg.regValue = rtd_inl(COLOR_SLD_SLD_HPF_1_reg);




    //value_dummy1802c700_31_22 = drvif_color_get_SLD_Dummy();
    cur_frame_idx_global = cur_frame_idx_global % 65535;
    cur_frame_idx_global = cur_frame_idx_global + 1;

    //RPC_SmartPic_clue = scaler_GetShare_Memory_RPC_SmartPic_Clue_Struct();



#ifdef SLD_MEMC_DISPLAY
    /*memc*/
    //rtd_setbits(0xb809d5fc, 0x10000000);//memc
    //WriteRegister(0xb809d0A0, 0, 3, 0x0A);//
#ifdef SLD_MEMC_DISPLAY
    //WriteRegister(0xb809d0A4, 24, 31, lock_fw_gain);//memc display
#endif

    blu_lc_global_ctrl2.regValue = IoReg_Read32(LC_LC_Global_Ctrl2_reg);

    num_hblk = blu_lc_global_ctrl2.lc_blk_hnum + 1 ;
    num_vblk = blu_lc_global_ctrl2.lc_blk_vnum + 1;

	//if pattern gen enable bypass all FW and diable SLD
#ifdef CONFIG_SCALER_ENABLE_V4L2
	if(v4l2_vpq_ire != V4L2_EXT_VPQ_INNER_PATTERN_DISABLE){
#else
	if(0){
#endif
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


    //Save previous fram Block APL
    for (idx_r = 0; idx_r < (num_vblk); idx_r++) {
        for (idx_c = 0; idx_c < (num_hblk); idx_c++) {
            APL_pre[idx_arr] = APL_cur[idx_arr];
            idx_arr = idx_arr + 1;
        }
    }

    LC_time = LC_time + 1;
    global_frame = global_frame + 1;


    // Read LC block APL info
    drvif_color_get_LC_APL(&(APL_cur[0]));
    LC_compute_pre_cur_deltaY(&(APL_pre[0]), &(APL_cur[0]), &APL_avg_cur, &(APL_delta[0]), &counter_blk_pre_cur_same, cmd_box_lock_frame); //I,I,O

    APL_avg_cur = APL_avg_cur / 16;
    //curve_map_gain.dummy1802c70c_7_0 = APL_avg_cur ;
    //rtd_pr_vpq_info("[sld] Cur Average APL:%d, Pre Average APL:%d\n",APL_avg_cur,APL_avg_pre);
    //rtd_outl(COLOR_SLD_SLD_CURVE_MAP_GAIN_reg,curve_map_gain.regValue);

	#ifndef CONFIG_MEMC_NOTSUPPORT
    scene_change_flag = MEMC_Lib_GetInfoForSLD(1);

    if (MEMC_Lib_GetInfoForSLD(4) > memc_sc_motion1) {
        memc_sc_motion1 = MEMC_Lib_GetInfoForSLD(4);
    }
    if (MEMC_Lib_GetInfoForSLD(5) > memc_sc_motion2) {
        memc_sc_motion2 = MEMC_Lib_GetInfoForSLD(5);
    }
	#endif
    //dcc scene change
    //SmartPic_clue = fwif_color_Get_SmartPic_clue();
    //RPC_SmartPic_clue = (_RPC_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_SMARTPIC_CLUS);
    //dcc_scene_change_flag = SmartPic_clue->SceneChange_by_diff_mean_Dev | RPC_SmartPic_clue->SceneChange;


    //************Detected scene change*******************//
    memc_scene_change_flag = scene_change_flag & (memc_sc_motion1 > 10000) & (memc_sc_motion2 > 10000);

    if (memc_scene_change_flag) {
        memc_scene_change_flag_global = 10;
    }
    if (memc_scene_change_flag_global > 0) {
        memc_scene_change_flag_global--;
    }

    if (ABS_(APL_avg_cur - APL_avg_pre) >= 5) {
        apl_scene_change_flag_global = 10;
    }
    if (apl_scene_change_flag_global > 0) {
        apl_scene_change_flag_global--;
    }

    if ((memc_scene_change_flag_global > 0) && (apl_scene_change_flag_global > 0)) {
        real_scene_change_flag = 1;
        realsc_count = 10;
        memc_scene_change_flag_global = 0;
        apl_scene_change_flag_global = 0;
    }
    else {
        real_scene_change_flag = 0;
    }

    LC_count_num_highY_block_up_Left_Frame(&(APL_cur[0]), thl_highY_frame, &up_Left_counter_highY_blk);
    //KKKK = LC_count_num_four_circle_patterns(&(APL_cur[0]), thl_highY_frame);
    //KKKK = memc_logo_to_demura_4circle_detect(void);
    if (realsc_count > 0) {
        realsc_count--;
    }
    //if ( (apl_hi_block_count> Pre_apl_hi_block_count) && ((apl_hi_block_count>=8) && (apl_hi_block_count<=10)) )
    //if ( (apl_hi_block_count>=3) && (apl_hi_block_count<=6) )
    if (circle4_flag == 1) {
        fflag = 24;
		//rtd_pr_vpq_info("k7_sld_sw,circle4_flag=%d\n",circle4_flag);
    }
    if (fflag > 0) {
        fflag--;
    }
    curve_map_gain.regValue = rtd_inl(COLOR_SLD_SLD_CURVE_MAP_GAIN_reg);
    curve_map_gain.dummy1802c70c_7_0 = apl_hi_block_count;
    rtd_outl(COLOR_SLD_SLD_CURVE_MAP_GAIN_reg, curve_map_gain.dummy1802c70c_7_0);
    //rtd_pr_vpq_info("[sld] apl_hi_block_count = %d\n",apl_hi_block_count);
    Pre_apl_hi_block_count = apl_hi_block_count;

    scen_count_++;



    if (lock_fw_gain > 0) {
        lock_fw_gain = lock_fw_gain - 1;

        if (up_Left_counter_highY_blk >= 200) {
            fw_control_1.fw_pixel_gain = CLAMP_SLD(fw_control_1.fw_pixel_gain - (4096 / cmd_pixel_gain_step), 4095, 0);
        }
        else if (up_Left_counter_highY_blk >= 180) {
            fw_control_1.fw_pixel_gain = CLAMP_SLD(fw_control_1.fw_pixel_gain - (4096 / cmd_pixel_gain_step), 4095, 0);
        }
        else if (up_Left_counter_highY_blk >= 140) {
            fw_control_1.fw_pixel_gain = CLAMP_SLD(fw_control_1.fw_pixel_gain - (4096 / cmd_pixel_gain_step), 4095, 0);
        }
        else if (up_Left_counter_highY_blk >= 120) {
            fw_control_1.fw_pixel_gain = CLAMP_SLD(fw_control_1.fw_pixel_gain - (4096 / cmd_pixel_gain_step), 4095, 0);
        }
        else if (up_Left_counter_highY_blk >= 100) {
            fw_control_1.fw_pixel_gain = CLAMP_SLD(fw_control_1.fw_pixel_gain - (4096 / cmd_pixel_gain_step), 4095, 0);
        }
        else {
            fw_control_1.fw_pixel_gain = CLAMP_SLD(fw_control_1.fw_pixel_gain - (4096 / cmd_pixel_gain_step), 4095, 0);
        }

        rtd_outl(COLOR_SLD_SLD_FW_control_1_reg, fw_control_1.regValue);
		//rtd_pr_vpq_info("k7_sld_sw,lock_fw_gain = %d,fw_control_1.fw_pixel_gain = %d\n", lock_fw_gain, fw_control_1.fw_pixel_gain );
        //rtd_pr_vpq_info("[sld] lock_fw_gain = %d,fw_control_1.fw_pixel_gain = %d\n", lock_fw_gain, fw_control_1.fw_pixel_gain);
    }
    else if ((LC_time % 3) == 0) {
        // Read LC block APL info
        //drvif_color_get_LC_APL(&(APL_cur[0]));
        //LC_compute_pre_cur_deltaY(&(APL_pre[0]), &(APL_cur[0]),&(APL_delta[0]),&counter_blk_pre_cur_same, cmd_box_lock_frame);//I,I,O
        drvif_color_get_SLD_fw_read_only((DRV_SLD_sld_fw_read_only *) &fw_read_only_data);

        /*fw_control_0.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_0_reg);
        fw_control_0.fw_idx_ctrl_en = 0;
        rtd_outl(COLOR_SLD_SLD_FW_control_0_reg , fw_control_0.regValue);*/
        if (lock_fw_gain == 0) {
            //thl_highY_frame = cmd_dY_preCur_mouseCon;
            LC_count_num_highY_block_up_Left_Frame(&(APL_cur[0]), thl_highY_frame, &up_Left_counter_highY_blk);
            //rtd_pr_vpq_info("[sld] up_Left_counter_highY_blk = %d\n",up_Left_counter_highY_blk);

            if (up_Left_counter_highY_blk > 255) {
                up_Left_counter_highY_blk = 255;
            }

            //still-------------------------------------------------------------------------------------------------
            if (still_image_count == 5) {
                still_image_count--;
                /*
                fw_control_0.regValue  = IoReg_Read32(COLOR_SLD_SLD_FW_control_0_reg  );
                fw_control_0.fw_idx_ctrl_step = 8192-16;//S(13,0)
                fw_control_0.fw_idx_ctrl_en   = 1;
                IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg , fw_control_0.regValue);
                //WriteRegister(0xb809d0A0, 8, 15, 0x11);//memc display*/
                fw_control_1.fw_pixel_gain = CLAMP_SLD(fw_control_1.fw_pixel_gain - (4096 / 25), 4096, 0);
                rtd_outl(COLOR_SLD_SLD_FW_control_1_reg, fw_control_1.regValue);
				if ((print_en & _BIT0) != 0) {
					if (print_delay == 0)
						print_delay = 100;
					if((cnt % print_delay) == 0) {
						rtd_pr_vpq_info("k7_sld_sw ==========Still Image condition============= \n");
					}
				}
            }
            else if (still_image_count > 0) {
                still_image_count--;
            }


            if (counter_blk_pre_cur_same > cmd_thl_counter_blk_pre_cur_same_mouseCon) { //still
                if (still_image_count == 0) {
                    still_image_count = 5;
                }

            }
            else if (counter_blk_pre_cur_same <= cmd_thl_counter_blk_pre_cur_same_mouseCon) { //not still
                fw_control_0.regValue  = IoReg_Read32(COLOR_SLD_SLD_FW_control_0_reg);
                fw_control_0.fw_idx_ctrl_en   = 0;
                IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg, fw_control_0.regValue);
                //ice decrease Y-------------------------------------------------------------------------------------------
                if (up_Left_counter_highY_blk > up_Left_counter_highY_blk_thl) { //high Y block condition
                    if (up_Left_counter_highY_blk >= 200) {
                        lock_fw_gain = 3;
                        cmd_pixel_gain_step = lock_fw_gain;
                    }
                    else if (up_Left_counter_highY_blk >= 180) {
                        lock_fw_gain = 10;
                        cmd_pixel_gain_step = lock_fw_gain;
                    }
                    else if (up_Left_counter_highY_blk >= 140) {
                        lock_fw_gain = 15;
                        cmd_pixel_gain_step = lock_fw_gain;
                    }
                    else if (up_Left_counter_highY_blk >= 120) {
                        lock_fw_gain = 20;
                        cmd_pixel_gain_step = lock_fw_gain;
                    }
                    else if (up_Left_counter_highY_blk >= 100) {
                        lock_fw_gain = 25;
                        cmd_pixel_gain_step = lock_fw_gain;
                    }
                    else {
                        lock_fw_gain = 30;
                        cmd_pixel_gain_step = lock_fw_gain;
                    }
                }
                else if ((real_scene_change_flag &&realsc_count>0) || (fflag > 0 )) { //scene chagne condition
                    if (sld_cnt_2_reg.sld_max_pre_idx > 2500) {
                        //if(scen_count_>6 )
                        fw_control_0_data.fw_idx_ctrl_step = 8192 - (sld_cnt_2_reg.sld_max_pre_idx / 3) ; //
                        //else{
                        //use origin ctrl step
                        //}
                        //scen_count_ = 0; //reset


                        //drop logo index
                        fw_control_0_data.fw_idx_ctrl_en   = 1;
                        drvif_color_set_SLD_fw_control_step(&fw_control_0_data);
                        scene_change_speed_up_count_ = 15; //no speed up when scene chagnge
                    }
                    else if (sld_cnt_2_reg.sld_max_pre_idx > 1800) {
                        //if(scen_count_>6 )
                        fw_control_0_data.fw_idx_ctrl_step = 8192 - (sld_cnt_2_reg.sld_max_pre_idx / 4) ; //
                        //else{
                        //use origin ctrl step
                        //}
                        //scen_count_ = 0; //reset


                        //drop logo index
                        fw_control_0_data.fw_idx_ctrl_en   = 1;
                        drvif_color_set_SLD_fw_control_step(&fw_control_0_data);
                        scene_change_speed_up_count_ = 15; //no speed up when scene chagnge

                    }

                    //if detect 4 circle patterns, drop more
                    if (fflag > 0) {
                        //KKKK = 0;
                        fw_control_0_data.fw_idx_ctrl_step = 8192 - 4000 ; //8192-2048
                        fw_control_0_data.fw_idx_ctrl_en   = 1;
                        drvif_color_set_SLD_fw_control_step(&fw_control_0_data);

                        four_circle_pattern_count = 30;
						if ((print_en & _BIT0) != 0) {
							if (print_delay == 0)
								print_delay = 100;
							if((cnt % print_delay) == 0) {
								rtd_pr_vpq_info("k7_sld_sw,Detect 4 circle patterns and clear idx\n");
							}
						}
					}
                    else {
                        four_circle_pattern_count = 0;
                    }



                    real_scene_change_flag = 0; //reset real_scene_change_flag

#ifdef SLD_MEMC_DISPLAY
                    //WriteRegister(0xb809d0A0, 4, 7, 0x01);//display scene  change
                    //WriteRegister(0xb809d000, 15, 15, 0x1);//display scene    change
                    //WriteRegister(0xb809d000, 13, 13, 0x1);//display scene    change
#endif
                    //rtd_pr_vpq_info("sld_scene_change, +++++++++++++++Detected Scene Change+++++++++++++++++\n");
                    //rtd_pr_vpq_info("sld_scene_change, sld_max_pre_idx:%d\n", sld_cnt_2_reg.sld_max_pre_idx);
                    //rtd_pr_vpq_info("sld_scene_change, fw_idx_ctrl_step:%d\n", fw_control_0_data.fw_idx_ctrl_step);
                    //rtd_pr_vpq_info("sld_scene_change, scene_change_count:%d\n", scene_change_speed_up_count_);
                }
                else {
#ifdef SLD_MEMC_DISPLAY
                    //WriteRegister(0xb809d0A0, 4, 7, 0x00);//display scene  change
                    //WriteRegister(0xb809d000, 15, 15, 0x0);//display scene    change
                    //WriteRegister(0xb809d000, 13, 13, 0x0);//display scene    change

#endif
                    //rtd_outl(COLOR_SLD_SLD_FW_control_1_reg , fw_control_1.regValue);
                    //fw_control_1.fw_pixel_gain    = CLAMP_SLD(fw_control_1.fw_pixel_gain + (4096/scene_change_count),4096,0);
                    fw_control_1.fw_pixel_gain = 4096;



                    color_sld_sld_hpf_1_reg.regValue = rtd_inl(COLOR_SLD_SLD_HPF_1_reg);
                    color_sld_sld_acc_0_reg.regValue = rtd_inl(COLOR_SLD_SLD_ACC_0_reg);

                    if (four_circle_pattern_count > 0) {
                        four_circle_pattern_count --;
                        color_sld_sld_hpf_1_reg.regValue = rtd_inl(COLOR_SLD_SLD_HPF_1_reg);
                        color_sld_sld_acc_0_reg.regValue = rtd_inl(COLOR_SLD_SLD_ACC_0_reg);
                        //color_sld_sld_acc_0_reg.sld_max_idx_thl = 4096;
                        color_sld_sld_hpf_1_reg.sld_idx_step_0 = 0;
                        rtd_outl(COLOR_SLD_SLD_HPF_1_reg, color_sld_sld_hpf_1_reg.regValue);
                        rtd_outl(COLOR_SLD_SLD_ACC_0_reg, color_sld_sld_acc_0_reg.regValue);

                        //Slowely decrease apply gain
                        fw_control_1.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_1_reg);
                        fw_control_1.fw_pixel_gain = CLAMP_SLD(fw_control_1.fw_pixel_gain - (4096 / 15), 256, 0);
                        rtd_outl(COLOR_SLD_SLD_FW_control_1_reg, fw_control_1.regValue);

                        four_circle_pattern_flag = four_circle_pattern_count;

						if ((print_en & _BIT0) != 0) {
							if (print_delay == 0)
								print_delay = 100;
							if((cnt % print_delay) == 0) {
								rtd_pr_vpq_info("k7_sld_sw, Detect 4 patterns and decrease apply gain\n");
							}
						}

#ifdef SLD_MEMC_DISPLAY
                        //WriteRegister(0xb8099100, 15, 15, 0x1);//display scene    change
                        //WriteRegister(0xb8099100, 13, 13, 0x1);//display scene    change
#endif


                    }
                    else if (scene_change_speed_up_count_ > 0) {
                        scene_change_speed_up_count_--;
                        color_sld_sld_hpf_1_reg.regValue = rtd_inl(COLOR_SLD_SLD_HPF_1_reg);
                        //color_sld_sld_acc_0_reg.regValue = rtd_inl(COLOR_SLD_SLD_ACC_0_reg);

                        //color_sld_sld_acc_0_reg.sld_max_idx_thl = 3990;
                        color_sld_sld_hpf_1_reg.sld_idx_step_0 = 2;
                        rtd_outl(COLOR_SLD_SLD_HPF_1_reg, color_sld_sld_hpf_1_reg.regValue);
                        //rtd_outl(COLOR_SLD_SLD_ACC_0_reg,color_sld_sld_acc_0_reg.regValue);
#ifdef SLD_MEMC_DISPLAY
                        //WriteRegister(0xb8099100, 15, 15, 0x0);//display scene    change
                        //WriteRegister(0xb8099100, 13, 13, 0x0);//display scene    change
#endif
                    }
                    else {
#ifdef SLD_MEMC_DISPLAY
                        //WriteRegister(0xb8099100, 15, 15, 0x0);//display scene    change
                        //WriteRegister(0xb8099100, 13, 13, 0x0);//display scene    change
#endif
                        if (four_circle_pattern_flag > 0) { //slowely increase gain for 4 circle pattern
                            four_circle_pattern_flag -- ;
                            color_sld_sld_hpf_1_reg.regValue = rtd_inl(COLOR_SLD_SLD_HPF_1_reg);
                            color_sld_sld_acc_0_reg.regValue = rtd_inl(COLOR_SLD_SLD_ACC_0_reg);

                            color_sld_sld_hpf_1_reg.sld_idx_step_0 = 0;
                            //color_sld_sld_acc_0_reg.sld_max_idx_thl = 4096;


                            rtd_outl(COLOR_SLD_SLD_HPF_1_reg, color_sld_sld_hpf_1_reg.regValue);
                            rtd_outl(COLOR_SLD_SLD_ACC_0_reg, color_sld_sld_acc_0_reg.regValue);

                            //Slowely increase apply gain
                            fw_control_1.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_1_reg);
                            fw_control_1.fw_pixel_gain = CLAMP_SLD(fw_control_1.fw_pixel_gain + (4096 / 30), 256, 0);
                            rtd_outl(COLOR_SLD_SLD_FW_control_1_reg, fw_control_1.regValue);
                        }
                        else { //normal scene change speed up
                            color_sld_sld_hpf_1_reg.regValue = rtd_inl(COLOR_SLD_SLD_HPF_1_reg);
                            color_sld_sld_acc_0_reg.regValue = rtd_inl(COLOR_SLD_SLD_ACC_0_reg);
                            fw_control_1.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_1_reg);

                            color_sld_sld_hpf_1_reg.sld_idx_step_0 = 0;
                            //color_sld_sld_acc_0_reg.sld_max_idx_thl = 4096;
                            fw_control_1.fw_pixel_gain = CLAMP_SLD(fw_control_1.fw_pixel_gain + (4096 / cmd_lock_fw_gain_frame), 4096, 0);

                            rtd_outl(COLOR_SLD_SLD_HPF_1_reg, color_sld_sld_hpf_1_reg.regValue);
                            rtd_outl(COLOR_SLD_SLD_ACC_0_reg, color_sld_sld_acc_0_reg.regValue);
                            rtd_outl(COLOR_SLD_SLD_FW_control_1_reg, fw_control_1.regValue);
                        }

                    }


                    fw_control_0_data.fw_idx_ctrl_en = 0;
                    drvif_color_set_SLD_fw_control_step(&fw_control_0_data);
                }
                //reset memc scene change motion counter
                scene_change_flag = 0;
                memc_sc_motion1 = 0;
                memc_sc_motion2 = 0;


            }

        }

        LC_time = 0;//reset LC_time
    }
    APL_avg_pre = APL_avg_cur;

    fw_control_1.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_1_reg);
    //rtd_pr_vpq_info("[sld], LOGO_R:%d, LOGO_L:%d\n", LR_LOGO_CNT_R, LR_LOGO_CNT_L);
    //if logo area size > thl, then reduce the gain

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


    //use_a_function
    /*
        //if logo area size > thl, then reduce the gain
        drvif_color_get_SLD_fw_read_only((DRV_SLD_sld_fw_read_only*) &fw_read_only_data);//Read max_logo_idx_counter info
        if(fw_read_only_data.sld_max_pre_idx >= 4000)
            fw_control_max_logo_tolerance_data.max_pre_logo_tolerance = 1000; //Set max_logo_idx tolerance
        else if(fw_read_only_data.sld_max_pre_idx >= 3000)
            fw_control_max_logo_tolerance_data.max_pre_logo_tolerance = 700; //Set max_logo_idx tolerance
        else if(fw_read_only_data.sld_max_pre_idx >= 2000)
            fw_control_max_logo_tolerance_data.max_pre_logo_tolerance = 500; //Set max_logo_idx tolerance
        drvif_color_set_SLD_max_pre_logo_tolerance((DRV_SLD_sld_fw_control_2*) &fw_control_max_logo_tolerance_data);


        if((fw_read_only_data.sld_max_pre_idx >= 2000)&&(fw_read_only_data.near_max_pre_logo_idx_counter > (9000))) //12960
        {
            WriteRegister(0xb809d0A0, 8, 11, 0x0A);//display logo index too much
            rtd_pr_vpq_info("[sld] -------NEAR max_pre_logo_idx_counter = %d, so drop the index---------\n",fw_read_only_data.near_max_pre_logo_idx_counter);
            fw_control_1.fw_pixel_gain = 4096/10*1.5;
        }else
        {
            WriteRegister(0xb809d0A0, 8, 11, 0x0F);//display  logo index normal
            //rtd_pr_vpq_info("[sld] -------NEAR max_pre_logo_idx_counter = %d, so drop the index---------\n",fw_read_only_data.near_max_pre_logo_idx_counter);
            fw_control_1.fw_pixel_gain = 4096;
        }
        rtd_outl(COLOR_SLD_SLD_FW_control_1_reg , fw_control_1.regValue);

    */
	bypass_FW_sld:
	cnt++;
	if ((print_en & _BIT0) != 0) {
		if (print_delay == 0)
			print_delay = 100;
		if((cnt % print_delay) == 0) {
			rtd_pr_vpq_info("k7_sld_sw, fw_idx_ctrl_step:%d\n", fw_control_0_data.fw_idx_ctrl_step);
		}
	}	
	return;
	}


// yvonne add for SLD 20220419
unsigned char fwif_color_get_SLD_get_block_APL_mode(void)
{
	unsigned char mode;

	mode = 0;	// 0 = LD block APL, 1= SLD block APL

	return mode;
}


// yvonne add for SLD 20220419
void fwif_color_compute_APL_related_FW_New(void)
{
	//unsigned short idx_r = 0, idx_c = 0, idx_arr = 0;
	int num_hblk = 0, num_vblk = 0; // counter_blk_pre_cur_same = 0;
	//int blk_cur_same_thl = 7000,blk_still_same_f = 0;
	int pixel_gain_step_decrease = 0;
	//int use_SLD_APL = 0; 
	//int idx_step;
	unsigned int up_Left_counter_highY_blk = 0;
	unsigned int thl_highY_frame =500; // 800
	//unsigned int up_Left_counter_highY_blk_thl = 80;
	static unsigned int  memc_sc_motion1 = 0, memc_sc_motion2 = 0;
	int sld_weight=0,sld_height=0;
	//int fflag = 0;
	color_sld_sld_fw_control_1_RBUS  fw_control_1;
	color_sld_sld_fw_control_0_RBUS  fw_control_0;
	color_sld_sld_curve_map_gain_RBUS curve_map_gain;
	color_sld_sld_hpf_1_RBUS color_sld_sld_hpf_1_reg;
	//color_sld_sld_acc_0_RBUS color_sld_sld_acc_0_reg;
	color_sld_sld_cnt_2_RBUS sld_cnt_2_reg;
	color_sld_sld_size_RBUS color_sld_size;
	int total_highY_block = 0;
    //int memc_scene_change_flag = 0;
    //int real_scene_change_flag = 0;
    int still_image_flag = 0;
    int pixe_gain_decrease_flag = 0;
    int near_max_logo_tmp = 0, max_logo_temp = 0; 
    DRV_SLD_sld_fw_read_only fw_read_only_data;
    //DRV_SLD_sld_fw_control_2  fw_control_max_logo_tolerance_data = {0};
    DRV_SLD_sld_fw_control_0 fw_control_0_data = {0};
    //int stage1_period = 0, stage2_period = 0;
	static unsigned int cnt;
	unsigned short max_logo_protect_th = 150;
	//int near_max_idx_counter = 0;
	unsigned short print_en = OPS_CTRL_item.OPS_Debug_Item.Debug_HWSW_SLD_En;
	unsigned short print_delay = OPS_CTRL_item.OPS_Debug_Item.Debug_Delay;

    color_sld_sld_control_RBUS color_sld_control;
	//DRV_SLD_sld_Y_curve_map sld_Y_curve_map;
    //color_sld_sld_hpf_1_RBUS sld_hpf_1_RBUS;
    color_sld_control.regValue = IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg);
    fw_control_0.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_0_reg);
    fw_control_1.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_1_reg);

    curve_map_gain.regValue = rtd_inl(COLOR_SLD_SLD_CURVE_MAP_GAIN_reg);
    sld_cnt_2_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_CNT_2_reg);
    color_sld_sld_hpf_1_reg.regValue = rtd_inl(COLOR_SLD_SLD_HPF_1_reg);


    //cur_frame_idx_global = cur_frame_idx_global % 65535;
    //cur_frame_idx_global = cur_frame_idx_global + 1;
	
	/*disable blend_en*/
	//fw_control_0.fw_idx_freeze_for_full_frame = 1;
	//IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg, fw_control_0.regValue);
	//goto bypass_FW_sld;
	/*freeze all the time*/
	
	//if pattern gen enable bypass all FW and diable SLD
#ifdef CONFIG_SCALER_ENABLE_V4L2
	//if(v4l2_vpq_ire != V4L2_EXT_VPQ_INNER_PATTERN_DISABLE){
#else
	if(0){
#endif
		//drvif_color_set_SLD_Enable(0);
		
		fw_control_0_data.fw_idx_ctrl_step = 8192 - 4000 ; //8192-(sld_cnt_2_reg.sld_max_pre_idx/2)
		fw_control_0_data.fw_idx_ctrl_en   = 1;
		drvif_color_set_SLD_fw_control_step(&fw_control_0_data);
		if ((print_en & _BIT0) != 0) {
			if (print_delay == 0)
				print_delay = 100;
			if((cnt % print_delay) == 0) {
				rtd_pr_vpq_emerg("sld_sw,bypass FW_sld\n");
			}
		}		
		goto bypass_FW_sld;
	}else{
		fw_control_0.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_0_reg);
		fw_control_0.fw_idx_ctrl_en = 0;
		IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg, fw_control_0.regValue);
	}

	/*Detect not in activate then reset idx*/	
	if (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE) {
		color_sld_control.sld_global_drop = 1;
		if ((print_en & _BIT0) != 0){
			if((cnt % print_delay) == 0)
				rtd_pr_vpq_emerg("SLD_HW detect not in activate, reset logo idx\n");	
		
		IoReg_Write32(COLOR_SLD_SLD_CONTROL_reg,color_sld_control.regValue);
	}else{
		color_sld_control.sld_global_drop = 0;
		IoReg_Write32(COLOR_SLD_SLD_CONTROL_reg,color_sld_control.regValue);
		}
	}
		
	
    color_sld_size.regValue = IoReg_Read32(COLOR_SLD_SLD_SIZE_reg);
    sld_weight = color_sld_size.sld_width;
    sld_height  = color_sld_size.sld_height;

	if(fwif_color_get_SLD_get_block_APL_mode() == 1){
		num_hblk = ceil_SLD(sld_weight, 32);
		num_vblk = ceil_SLD(sld_height, 32);
		total_highY_block = (num_hblk/3)*(num_vblk/6);
		cmd_counter_frame_highY_blk = total_highY_block*85/100;/*thl=0.85*total block*/
	}else{
		num_hblk = 241; 
		num_vblk = 136;
		total_highY_block = (num_hblk/3)*(num_vblk/5);
		cmd_counter_frame_highY_blk = total_highY_block*85/100;/*thl=0.85*total block*/
	}

    //rtd_pr_vpq_emerg("SLD_HW Size info, sld_weight:%d, sld_height:%d\n",sld_weight,sld_height);
    //rtd_pr_vpq_info("SLD_HW APL info, num_hblk:%d, num_vblk:%d\n",num_hblk,num_vblk);

	
    //Save previous fram Block APL
    /*for (idx_r = 0; idx_r < (num_vblk); idx_r++) {
        for (idx_c = 0; idx_c < (num_hblk); idx_c++) {
            APL_pre_uint16[idx_arr] = APL_cur_uint16[idx_arr];
            idx_arr = idx_arr + 1;
        }
    }*/



	/* Read SLD block APL info*/
	if(fwif_color_get_SLD_get_block_APL_mode() == 1)
		drvif_fwif_color_Get_SLD_APL(APL_cur_uint16, num_hblk*num_vblk);
	else{
		if (sld_work.LD_APL_valid && memc_logo_LD_APL_check == 1){
			memcpy(APL_cur, blk_apl_interp, sizeof(int)*num_hblk*num_vblk);
		}		
	}


    /*
    Compute_pre_cur_deltaY(num_hblk, num_vblk, &(APL_pre_uint16[0]), &(APL_cur_uint16[0]), &APL_avg_cur, &(APL_delta_uint16[0]), &counter_blk_pre_cur_same, cmd_box_lock_frame); //I,I,O
	if ((print_en & _BIT0) != 0){
		if((cnt % print_delay) == 0)
			rtd_pr_vpq_info("SLD_HW counter_blk_pre_cur_same:%d\n",counter_blk_pre_cur_same);
	}

	
    //APL_avg_cur = APL_avg_cur / 16;
    	if( counter_blk_pre_cur_same>= blk_cur_same_thl){
		blk_still_same_f = 1;
		if ((print_en & _BIT0) != 0){
			if((cnt % print_delay) == 0)
				rtd_pr_vpq_info("SLD_HW blk_still_same_f%d\n",blk_still_same_f);
		}
	}
    */	
    
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
    }else {
    	still_image_flag = 0;
    }
	
    /*Count the up left frame of highY block*/
	if(fwif_color_get_SLD_get_block_APL_mode() == 1){
		Count_num_highY_block_up_Left_Frame(num_hblk, &(APL_cur_uint16[0]), thl_highY_frame, &up_Left_counter_highY_blk);
	} else{
		Count_num_highY_block_up_Left_Frame_Uint_APL(num_hblk, num_vblk, &(APL_cur[0]), thl_highY_frame, &up_Left_counter_highY_blk);
		if (((print_en & _BIT0) != 0) &&((cnt % print_delay) == 0) ){
			rtd_pr_vpq_emerg("SLD_HW,  up_Left_counter_highY_blk:%d \n",up_Left_counter_highY_blk);
		}

		
	}

    /*Detect 4 circles pattern*/
    if (circle4_flag != 0) {
	  if (((print_en & _BIT0) != 0) &&((cnt % print_delay) == 0) ){
		//rtd_pr_vpq_info("SLD_HW, circle4_flag=%d\n",circle4_flag);
	  }
    }
	//near_max_idx_counter = drvif_color_get_logo_num(1800);
	drvif_color_get_SLD_fw_read_only(&fw_read_only_data);

	near_max_logo_tmp = fw_read_only_data.near_max_pre_logo_idx_counter;
	max_logo_temp = fw_read_only_data.max_pre_logo_idx_counter;	
	
	if(((print_en & _BIT0) != 0) &&((cnt % print_delay) == 0) ){
		rtd_pr_vpq_info("SLD_HW_FW_r, max_logo_temp:%d,  near_max_idx_counter_tmp=%d\n",max_logo_temp,near_max_logo_tmp);

	}

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
    }
	else{
    	/*When detect frame is not still*/
		//fwif_color_SLD_apply_normal_setting();
		fw_control_0.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_0_reg);
		sld_cnt_2_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_CNT_2_reg);
		
	 	fw_control_0.fw_idx_freeze_for_full_frame = 0;
	 	IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg, fw_control_0.regValue);

		if(memc_scene_change_flag_global>0){
			/*scene change*/
			fw_control_0.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_0_reg);
			if ((print_en & _BIT0) != 0){
				 rtd_pr_vpq_info("SLD_HW scene change, sld_max_pre_idx=%d\n",sld_cnt_2_reg.sld_max_pre_idx);
			}
			
			if(scene_change_drop>0){
				scene_change_drop--;
				fw_control_0.fw_idx_ctrl_step = 8192 - (sld_cnt_2_reg.sld_max_pre_idx/60) ;//(sld_cnt_2_reg.sld_max_pre_idx / 500) ; 
				fw_control_0.fw_idx_ctrl_en = 1;
				IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg, fw_control_0.regValue);				
			}
			else if (sld_cnt_2_reg.sld_max_pre_idx >= 3000/*500*/) {
			     scene_change_drop = 10;
		    }
			else if (sld_cnt_2_reg.sld_max_pre_idx > 1800) {
			     fw_control_0.fw_idx_ctrl_step = 8192 - (sld_cnt_2_reg.sld_max_pre_idx / 600) ; 
			     fw_control_0.fw_idx_ctrl_en = 1;
					     //IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg, fw_control_0.regValue);
            }
			else if(circle4_flag){
            	if((print_en & _BIT0) != 0){
						rtd_pr_vpq_info("SLD_HW circle4_flag=%d and detect scene change\n",circle4_flag);	
				}
				if(sld_cnt_2_reg.sld_max_pre_idx>600){
					fw_control_0.fw_idx_ctrl_step = 8192 - 100;//(sld_cnt_2_reg.sld_max_pre_idx / 600) ;
					fw_control_0.fw_idx_ctrl_en = 1;
					IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg, fw_control_0.regValue);
				}

			}
				
		}
		else if(circle4_flag){
			sld_cnt_2_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_CNT_2_reg);
			fw_control_0.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_0_reg);

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
			if ((print_en & _BIT0) != 0){
				rtd_pr_vpq_info("SLD_HW, circle4_flag=%d, reduce idx:%d\n",circle4_flag,fw_control_0.fw_idx_ctrl_step);
			}			
		}
		else if(up_Left_counter_highY_blk > cmd_counter_frame_highY_blk){ //high Y block condition
                    if (up_Left_counter_highY_blk >= total_highY_block*9/10) {
                        lock_fw_gain = 3;
                        cmd_pixel_gain_step = 3;
						pixel_gain_step_decrease = cmd_pixel_gain_step*5;
                    }
                    else if (up_Left_counter_highY_blk >= total_highY_block*7/10) {
                        lock_fw_gain = 10;
                        cmd_pixel_gain_step = 10;
						pixel_gain_step_decrease = cmd_pixel_gain_step*10;
                    }
                    else if (up_Left_counter_highY_blk >= total_highY_block*5/10) {
                        lock_fw_gain = 15;
                        cmd_pixel_gain_step = 15;
						pixel_gain_step_decrease = cmd_pixel_gain_step*15;
                    }
			/*
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
                    }*/
			fw_control_1.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_1_reg);
			fw_control_1.fw_pixel_gain = CLAMP_SLD(fw_control_1.fw_pixel_gain - (4096 / pixel_gain_step_decrease), 4095, 0);	
			IoReg_Write32(COLOR_SLD_SLD_FW_control_1_reg, fw_control_1.regValue);
			pixe_gain_decrease_flag = 1;
			if (((print_en & _BIT0) != 0) &&((cnt % print_delay) == 0) ){
				rtd_pr_vpq_info("SLD_HW high Y block condition, decrease piexl gain:%d,  remain pixel gain:%d\n",4096 / cmd_pixel_gain_step,fw_control_1.fw_pixel_gain);
			}
        }
		else if(LR_LOGO_Flag_R || LR_LOGO_Flag_L){
         	if (((print_en & _BIT0) != 0) &&((cnt % print_delay) == 0) ){
         		rtd_pr_vpq_info("SLD_HW, LR_LOGO_Flag_L=%d, LR_LOGO_Flag_R=%d\n",LR_LOGO_Flag_L,LR_LOGO_Flag_R);
         	}
			if(sld_cnt_2_reg.sld_max_pre_idx >600){
		        fw_control_0.fw_idx_ctrl_step = 8192 - (sld_cnt_2_reg.sld_max_pre_idx/500) ;
		        fw_control_0.fw_idx_ctrl_en = 1;
	 			IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg, fw_control_0.regValue);
			}

		}
		else if(near_max_logo_tmp>=((480*270*max_logo_protect_th)>>10)){
			//fw_control_0.fw_idx_ctrl_step = 8192 - (sld_cnt_2_reg.sld_max_pre_idx / 400) ;
		 	fw_control_0.fw_idx_ctrl_step = 8192 - 50 ;
		 	fw_control_0.fw_idx_ctrl_en = 1;
		 	IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg, fw_control_0.regValue);
			if (((print_en & _BIT0) != 0) &&((cnt % print_delay) == 0) ){
				rtd_pr_vpq_info("SLD_HW detect big logo, near_max_logo_tmp:%d\n",near_max_logo_tmp);
			}
		}else{
			//normal condition
			if ((print_en & _BIT0) != 0){
				if((cnt % print_delay) == 0)
					rtd_pr_vpq_info("SLD_HW normal condition \n");
			}
			fw_control_1.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_1_reg);
			fw_control_0.fw_idx_ctrl_en = 0;
		 	IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg, fw_control_0.regValue);
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

    /*End of detection */
	if(memc_logo_to_demura_drop_limit==0)
		fwif_drvif_color_set_SLD_Y_curve_map_bySLD_dropGain(memc_logo_to_demura_drop_limit);
	else if(pre_memc_logo_to_demura_drop_limit!=memc_logo_to_demura_drop_limit){
		fwif_drvif_color_set_SLD_Y_curve_map_bySLD_dropGain(memc_logo_to_demura_drop_limit);
		pre_memc_logo_to_demura_drop_limit = memc_logo_to_demura_drop_limit;

	}

	/*reset memc motion flag*/
	scene_change_flag = 0;
	memc_sc_motion1 = 0;
	memc_sc_motion2 = 0;
	
	/*reset */
	pixe_gain_decrease_flag = 0;
	//fflag = 0;
	//counter_blk_pre_cur_same = 0;
	//blk_still_same_f = 0;
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


	
#if 0
void fwif_color_compute_LC_APL_related_K7_FW(void)
{
    unsigned short idx_r = 0, idx_c = 0, idx_arr = 0, counter_UR = 0;
    int num_hblk = 0, num_vblk = 0;
    unsigned int counter_highY_blk = 0, counter_blk_pre_cur_same = 0;
    unsigned int thl_highY_frame = 750;
    color_sld_sld_fw_control_1_RBUS  fw_control_1;
    DRV_SLD_sld_fw_read_only fw_read_only_data;
    color_sld_sld_fw_control_0_RBUS  fw_control_0;
    lc_lc_global_ctrl2_RBUS blu_lc_global_ctrl2;
    static unsigned int  memc_sc_motion1 = 0, memc_sc_motion2 = 0;
    static unsigned char memc_sc_flag = false;


    if (MEMC_Lib_GetInfoForSLD(1)) { // scene change flag
        memc_sc_flag = true;
    }
    if (MEMC_Lib_GetInfoForSLD(4) > memc_sc_motion1) {
        memc_sc_motion1 = MEMC_Lib_GetInfoForSLD(4);
    }
    if (MEMC_Lib_GetInfoForSLD(5) > memc_sc_motion2) {
        memc_sc_motion2 = MEMC_Lib_GetInfoForSLD(5);
    }

    get_osd_pos();



    cur_frame_idx_global = cur_frame_idx_global % 65535;
    cur_frame_idx_global = cur_frame_idx_global + 1;

#ifdef SLD_MEMC_DISPLAY
    /*memc*/
    rtd_setbits(0xb809d5fc, 0x10000000);//memc
#endif

    if (memc_sc_motion1 < 100 && memc_sc_motion2 < 100) { // motion cnt low: picture pause
        sld_condition.still_condition = true;

#ifdef SLD_MEMC_DISPLAY
        //WriteRegister(0xb809d0A0, 8, 15, 0x01);//memc still
#endif
    }
    else if (memc_sc_flag && memc_sc_motion1 > 10000 && memc_sc_motion2 > 10000) { // scene change flag + big motion: scene change
        sld_condition.still_condition = false;

#ifdef SLD_MEMC_DISPLAY
        //WriteRegister(0xb809d0A0, 8, 15, 0x02);//memc not still
#endif
    }


    blu_lc_global_ctrl2.regValue = IoReg_Read32(LC_LC_Global_Ctrl2_reg);

    num_hblk = blu_lc_global_ctrl2.lc_blk_hnum + 1 ;
    num_vblk = blu_lc_global_ctrl2.lc_blk_vnum + 1;

    for (idx_r = 0; idx_r < (num_vblk); idx_r++) {
        for (idx_c = 0; idx_c < (num_hblk); idx_c++) {
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

    if (lock_rgb_box_fw_step > 0) {
        lock_rgb_box_fw_step = lock_rgb_box_fw_step - 1;
        rtd_pr_vpq_info("#############################lock_rgb_box_fw_step = %d, ", lock_rgb_box_fw_step);
        drvif_solor_set_SLD_apply_leave_RGB_box_logo(cmd_rgb_box_leave_idx_delete_step);

    }
    else if (lock_fw_gain > 0) {
        lock_fw_gain = lock_fw_gain - 1;
        fw_control_1.fw_pixel_gain              = CLAMP_SLD(fw_control_1.fw_pixel_gain - (4096 / cmd_pixel_gain_step), 4095, 0);
        rtd_outl(COLOR_SLD_SLD_FW_control_1_reg, fw_control_1.regValue);
        rtd_pr_vpq_info("lock_fw_gain = %d,fw_control_1.fw_pixel_gain = %d\n", lock_fw_gain, fw_control_1.fw_pixel_gain);
    }
    else if ((LC_time % cmd_LC_period) == 0) {
        rtd_pr_vpq_info("cur_frame_idx_global = %d\n", cur_frame_idx_global);

        drvif_color_get_LC_APL(&(APL_cur[0]));
        LC_compute_pre_cur_deltaY(&(APL_pre[0]), &(APL_cur[0]), &APL_avg_cur, &(APL_delta[0]), &counter_blk_pre_cur_same, cmd_dY_preCur_mouseCon); //I,I,O
        drvif_color_get_SLD_fw_read_only((DRV_SLD_sld_fw_read_only *) &fw_read_only_data);

        if (
                    (sld_work.osd_sta_blkx > 0)
                    && (sld_work.osd_end_blkx > 0)
                    && (sld_work.osd_sta_blky > 0)
                    && (sld_work.osd_end_blky > 0)
        ) { //OSD
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


        else if ( //mouse condition
                    sld_condition.still_condition
        ) {


            if ( //R/G/B box condition
                        LC_detect_RGB_box_logo(&(APL_cur[0]), &(APL_pre[0]), cmd_thl_dC, cmd_thl_dUDL)
                        || (((cur_frame_idx_global - index_box_logo_case) < cmd_thl_error_time_box_logo_case) && (index_box_logo_case != 0))
            ) {
                rtd_pr_vpq_info("debug mouse+R/G/B box condition : cur_frame_idx_global = %d,  index_box_logo_case = %d\n", cur_frame_idx_global, index_box_logo_case);

                if (LC_detect_RGB_box_logo(&(APL_cur[0]), &(APL_pre[0]), cmd_thl_dC, cmd_thl_dUDL)) {
                    rtd_pr_vpq_info("mouse+R/G/B box condition----------------\n");
                    index_box_logo_case = cur_frame_idx_global;

                }
                else {
                    rtd_pr_vpq_info("mouse+R/G/B box,in time condition----------------\n");
                }

                counter_box_logo_case = counter_box_logo_case + 1;

                if (counter_box_logo_case > cmd_thl_counter_box_logo_case) {
                    LC_apply_RGB_box_logo(cmd_LC_period, cmd_thl_counter_box_logo_case, cmd_box_logo_pixel_gain, cmd_box_logo_maxGain_time);


                }

            }
            else { //not R/G/B box condition
                rtd_pr_vpq_info("mouse condition--------------------\n");
#ifdef SLD_MEMC_DISPLAY
                WriteRegister(0x000040A0, 0, 8, 0x2);//memc display
#endif
                //fw control step until 0
                fw_control_0.regValue                   = IoReg_Read32(COLOR_SLD_SLD_FW_control_0_reg);
                fw_control_0.fw_idx_ctrl_step           = 8192 - 4; //S(13,0)
                fw_control_0.fw_idx_ctrl_en = 1;
                rtd_outl(COLOR_SLD_SLD_FW_control_0_reg, fw_control_0.regValue);
            }
        }
        else if ( //news condition
                    ((fw_read_only_data.hpf_line_count > 3) && (fw_read_only_data.hpf_line_count < 10) && LC_detect_NEWS(&(APL_cur[0])))
                    || (((cur_frame_idx_global - index_NEWS_case) < cmd_thl_error_time_NEWS_case) && (index_NEWS_case != 0))
        ) {

            if (((fw_read_only_data.hpf_line_count > 3) && (fw_read_only_data.hpf_line_count < 10) && LC_detect_NEWS(&(APL_cur[0])))) {
                rtd_pr_vpq_info("NEWS condition--------------------\n");
#ifdef SLD_MEMC_DISPLAY
                WriteRegister(0x000040A0, 0, 8, 0x3);//memc display
#endif
                index_NEWS_case = cur_frame_idx_global;
            }
            else {
                rtd_pr_vpq_info("NEWS,in time condition----------------\n");
            }

            counter_NEWS_case = counter_NEWS_case + 1;

            if (counter_NEWS_case > cmd_thl_counter_NEWS_case) {
                fwif_color_apply_NEWS_setting();
                fwif_color_SLD_disable_window();
            }

        }
        else if ( //R/G/B box condition
                    (
                                (LC_detect_RGB_box_logo(&(APL_cur[0]), &(APL_pre[0]), cmd_thl_dC, cmd_thl_dUDL))
                                || (((cur_frame_idx_global - index_box_logo_case) < cmd_thl_error_time_box_logo_case) && (index_box_logo_case != 0))
                    )
                    || (flag_rgb_box_in == 1)
        ) {

            if (LC_detect_RGB_box_logo(&(APL_cur[0]), &(APL_pre[0]), cmd_thl_dC, cmd_thl_dUDL)) {
                rtd_pr_vpq_info("R/G/B box condition----------------\n");
#ifdef SLD_MEMC_DISPLAY
                WriteRegister(0x000040A0, 0, 8, 0x4);//memc display
#endif
                index_box_logo_case = cur_frame_idx_global;

            }
            else {
                rtd_pr_vpq_info("R/G/B box,in time condition----------------\n");
            }

            counter_box_logo_case = counter_box_logo_case + 1;

            if ((counter_box_logo_case > cmd_thl_counter_box_logo_case) || (flag_rgb_box_in == 1)) {
                LC_apply_RGB_box_logo(cmd_LC_period, cmd_thl_counter_box_logo_case, cmd_box_logo_pixel_gain, cmd_box_logo_maxGain_time);
                flag_rgb_box_in = 1;
                rtd_pr_vpq_info("flag_rgb_box_in = %d --------------------\n", flag_noise_in);

            }

        }

        else if ( //noise
                    (
                                (((drvif_color_SLD_get_noise_pattern_counter()) >= 129590) && LC_detect_noise(&(APL_cur[0])))
                                || (((cur_frame_idx_global - index_noise_case) < cmd_thl_error_time_noise_case) && (index_noise_case != 0))
                    )
                    || (flag_noise_in == 1)
        ) {

            if (((drvif_color_SLD_get_noise_pattern_counter()) >= 129590) && LC_detect_noise(&(APL_cur[0]))) {
                rtd_pr_vpq_info("noise pattern condition--------------------\n");
#ifdef SLD_MEMC_DISPLAY
                WriteRegister(0x000040A0, 0, 8, 0x5);//memc display
#endif
                index_noise_case = cur_frame_idx_global;
            }
            else {
                rtd_pr_vpq_info("noise pattern,in time condition----------------\n");
            }

            counter_noise_case = counter_noise_case + 1;

            if ((counter_noise_case > cmd_thl_counter_noise_case) || (flag_noise_in == 1)) {
                SLD_apply_noise_pattern(cmd_LC_period, cmd_thl_counter_noise_case);
                flag_noise_in = 1;
                rtd_pr_vpq_info("flag_noise_in = %d --------------------\n", flag_noise_in);
            }


        }

        else {
            rtd_pr_vpq_info("mouse condition debug,counter_blk_pre_cur_same=%d\n", counter_blk_pre_cur_same);
            rtd_pr_vpq_info("NEWS condition debug,hpf_line_count=%d,LC_detect_NEWS=%d\n", fw_read_only_data.hpf_line_count, LC_detect_NEWS(&(APL_cur[0])));
            rtd_pr_vpq_info("box_logo condition debug, 1=%04d, 2=%04d, 3=%04d, 4=%04d, 5=%04d, 6=%04d, counter_box_logo_case = %d, (cur_frame_idx_global - index_box_logo_case) = %d\n", APL_cur[140], APL_cur[188], APL_cur[236], APL_cur[141], APL_cur[189], APL_cur[141], counter_box_logo_case, (cur_frame_idx_global - index_box_logo_case));


            if (counter_box_logo_case > 0) { //leave r/g/b box
                lock_rgb_box_fw_step = cmd_box_lock_frame;// frame : lock_rgb_box_fw_step*cmd_LC_period
            }

            counter_box_logo_case = 0;
            counter_NEWS_case = 0;

            if (lock_fw_gain == 0) {

                drvif_color_set_SLD_default();

                fwif_color_SLD_add_con();//add condition add/ or
                fwif_color_SLD_fw_read_only(cmd_lock_fw_gain_frame);//logo idx > 3000 has 1/4 area  +   spatial compensate


                LC_count_num_highY_block_upFrame(&(APL_cur[0]), thl_highY_frame, &counter_highY_blk);
                if (counter_highY_blk > cmd_counter_frame_highY_blk) { //high Y block condition
                    lock_fw_gain = cmd_lock_fw_gain_frame;
                    rtd_pr_vpq_info("high Y block condition--------------------------------------\n");
#ifdef SLD_MEMC_DISPLAY
                    WriteRegister(0x000040A0, 0, 8, 0x6);//memc display
#endif
                }
                else {
                    //fw_control_1.fw_pixel_gain                = 4096;
                    fw_control_1.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_1_reg);
                    fw_control_1.fw_pixel_gain              = CLAMP_SLD(fw_control_1.fw_pixel_gain + (4096 / cmd_pixel_gain_step), 4096, 0);
                    rtd_outl(COLOR_SLD_SLD_FW_control_1_reg, fw_control_1.regValue);
                    rtd_pr_vpq_info("high Y block condition debug, counter_highY_blk = %d\n", counter_highY_blk);
                }

                LC_compute_corner_UR_Y(&(APL_cur[0]), cmd_thl_counter_UR_con, &counter_UR);
                if (counter_UR > cmd_thl_counter_UR_con) { //corner condition
                    lock_fw_gain = cmd_lock_fw_gain_frame;
                    rtd_pr_vpq_info("corner condition--------------------------------------\n");
#ifdef SLD_MEMC_DISPLAY
                    WriteRegister(0x000040A0, 0, 8, 0x7);//memc display
#endif
                }
                else {
                    fw_control_1.regValue = IoReg_Read32(COLOR_SLD_SLD_FW_control_1_reg);
                    fw_control_1.fw_pixel_gain              = CLAMP_SLD(fw_control_1.fw_pixel_gain + (4096 / cmd_pixel_gain_step), 4096, 0);
                    rtd_outl(COLOR_SLD_SLD_FW_control_1_reg, fw_control_1.regValue);
                    rtd_pr_vpq_info("corner condition debug, counter_UR = %d\n", counter_UR);

                }
            }
        }

        LC_time = 0;
    }
}
#endif



void SLD_apply_condition(void)
{
    static unsigned int frame_cnt = 0;

    //Merlin6, SLD
    //unsigned int counter_blk_pre_cur_same = 0;
    //int thl_temporal_dY = 20;
    //int thl_counter_NEWS_case = 240;//tolerance num of frame
    color_sld_sld_fw_control_0_RBUS  fw_control_0;

    if (sld_condition.still_condition) { //apply
        rtd_pr_vpq_info("still condition--------------------\n");

        //fw control step until 0
        fw_control_0.regValue        = IoReg_Read32(COLOR_SLD_SLD_FW_control_0_reg);
        fw_control_0.fw_idx_ctrl_step = 8192 - 4; //S(13,0)
        fw_control_0.fw_idx_ctrl_en   = 1;
        IoReg_Write32(COLOR_SLD_SLD_FW_control_0_reg, fw_control_0.regValue);
    }
    else if (sld_condition.news_condition) {
        fwif_color_apply_NEWS_setting();
        fwif_color_SLD_disable_window();
    }
    else if (sld_condition.normal_condition) {
        fwif_color_SLD_apply_normal_setting();
    }

    frame_cnt++;

}



static void eq_table_insert(int a, int b)
{
    int base = (a < b) ? a : b;
    int ins = (a < b) ? b : a;
    int cur_idx;
    int nxt_idx;

    if (a == b) {
        return;
    }

    cur_idx = base;
    while (1) {
        nxt_idx = eq_table[cur_idx][1];
        if (nxt_idx == ins) { // already in list
            return;
        }
        else if (nxt_idx == 0 || nxt_idx > ins) {
            break;
        }
        cur_idx = nxt_idx;
    }
    if (nxt_idx == 0) { // stopped at list end
        eq_table[cur_idx][1] = ins;
        eq_table[ins][0] = cur_idx;
        eq_table[ins][1] = 0;
    }
    else { // stopped in list, insert between cur_idx & nxt_idx
        eq_table[cur_idx][1] = ins;
        eq_table[ins][0] = cur_idx;
        eq_table[ins][1] = 0;
        eq_table[nxt_idx][0] = ins;
    }
}

// debug use, only use it in rtice tool
//#include <rbus/mc_reg.h>
//#include <rbus/mc2_reg.h>
//#include <rbus/lbmc_reg.h>
void drvif_color_set_SLD_Hybrid_debug_logo_show_rtice(unsigned char mode)
{	
#ifdef CONFIG_MEMC_NOTSUPPORT // for code size reduce, when MEMC_BYPASS
		return;
#else
    color_sld_sld_control_RBUS color_sld_control;
	mc_mc_dc_RBUS mc_mc_dc_reg;
	mc2_mc2_20_RBUS mc2_mc2_20_reg;
	lbmc_lbmc_88_RBUS lbmc_lbmc_88_reg;
	
    color_sld_control.regValue = IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg);
	mc_mc_dc_reg.regValue = IoReg_Read32(MC_MC_DC_reg);
	mc2_mc2_20_reg.regValue = IoReg_Read32(MC2_MC2_20_reg);
	lbmc_lbmc_88_reg.regValue = IoReg_Read32(LBMC_LBMC_88_reg);
	

    if (mode == 1) {
		color_sld_control.sld_debug_mode = 1;
		mc_mc_dc_reg.mc_show_sel = 0;
		mc_mc_dc_reg.mc_show_pt_mode = 5;
		mc2_mc2_20_reg.mc_logo_merge_type = 0;
		lbmc_lbmc_88_reg.lbmc_logo_merge_type = 0;		
    } else if (mode == 2) {
		color_sld_control.sld_debug_mode = 0;
		mc_mc_dc_reg.mc_show_sel = 3;
		mc_mc_dc_reg.mc_show_pt_mode = 5;
		mc2_mc2_20_reg.mc_logo_merge_type = 3;
		lbmc_lbmc_88_reg.lbmc_logo_merge_type = 3;
    } else {
		color_sld_control.sld_debug_mode = 0;
		mc_mc_dc_reg.mc_show_sel = 0;
		mc_mc_dc_reg.mc_show_pt_mode = 5;
		if (MEMC_Pixel_LOGO_For_SW_SLD == 0)
			mc2_mc2_20_reg.mc_logo_merge_type = 0x0;
		else
			mc2_mc2_20_reg.mc_logo_merge_type = 0xA;
		lbmc_lbmc_88_reg.lbmc_logo_merge_type = 0;
    }

    IoReg_Write32(COLOR_SLD_SLD_CONTROL_reg, color_sld_control.regValue);
	IoReg_Write32(MC_MC_DC_reg, mc_mc_dc_reg.regValue);
	IoReg_Write32(MC2_MC2_20_reg, mc2_mc2_20_reg.regValue);
	IoReg_Write32(LBMC_LBMC_88_reg, lbmc_lbmc_88_reg.regValue);
#endif
}


//static unsigned char memc_sc_flag = false;
//static unsigned int  memc_sc_motion1 = 0, memc_sc_motion2 = 0;

static unsigned char memc_sc_flag_debug = false;
static unsigned int  memc_sc_motion1_debug = 0, memc_sc_motion2_debug = 0;
static unsigned char memc_flag_ok_debug = false;

void memc_logo_to_demura_read(void)
{
#ifdef CONFIG_MEMC_NOTSUPPORT // for code size reduce, when MEMC_BYPASS
	return;
#else
    static unsigned int frame_cnt = 0;
    unsigned char *logo_ptr = LD_virAddr;

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
    if (sld_ddr_offset_auto_get == 1) {
        sld_ddr_offset = IoReg_Read32(KME_DM_TOP1_KME_DM_TOP1_48_reg) & 0xfff;
    }

    logo_ptr = LD_virAddr + sld_ddr_offset;

    // start to work
    if (!osd_measure_init) {
        osdovl_mixer_ctrl2_reg.regValue = IoReg_Read32(OSDOVL_Mixer_CTRL2_reg);
        osdovl_osd_db_ctrl_reg.regValue = IoReg_Read32(OSDOVL_OSD_Db_Ctrl_reg);

        osdovl_mixer_ctrl2_reg.measure_osd_zone_en = 1;
        osdovl_mixer_ctrl2_reg.measure_osd_zone_type = 1;
        IoReg_Write32(OSDOVL_Mixer_CTRL2_reg, osdovl_mixer_ctrl2_reg.regValue);

        osdovl_osd_db_ctrl_reg.db_load = 1;
        IoReg_Write32(OSDOVL_OSD_Db_Ctrl_reg, osdovl_osd_db_ctrl_reg.regValue);

        osd_measure_init = true;
    }

    osdovl_measure_osd1_sta_reg.regValue = IoReg_Read32(OSDOVL_measure_osd1_sta_reg);
    osdovl_measure_osd1_end_reg.regValue = IoReg_Read32(OSDOVL_measure_osd1_end_reg);
    osd_sta_x = osdovl_measure_osd1_sta_reg.x;
    osd_sta_y = osdovl_measure_osd1_sta_reg.y;
    osd_end_x = osdovl_measure_osd1_end_reg.x;
    osd_end_y = osdovl_measure_osd1_end_reg.y;

    if (osd_sta_x == 0x1fff && osd_sta_y == 0x1fff && osd_end_x == 0 && osd_end_y == 0) { // no OSD shown
        sld_work.osd_sta_blkx = -1;
        sld_work.osd_sta_blky = -1;
        sld_work.osd_end_blkx = -1;
        sld_work.osd_end_blky = -1;
    }
    else { // mark osd blocks
        sld_work.osd_sta_blkx = osd_sta_x / 8;
        sld_work.osd_end_blkx = (osd_end_x + 7) / 8;
        sld_work.osd_sta_blky = osd_sta_y / 8;
        sld_work.osd_end_blky = (osd_end_y + 7) / 8;
    }

    // motion status (read every frame)
    if (MEMC_Lib_GetInfoForSLD(1)) { // scene change flag
        memc_sc_flag = true;
    }
    if (MEMC_Lib_GetInfoForSLD(4) > memc_sc_motion1) {
        memc_sc_motion1 = MEMC_Lib_GetInfoForSLD(4);
    }
    if (MEMC_Lib_GetInfoForSLD(5) > memc_sc_motion2) {
        memc_sc_motion2 = MEMC_Lib_GetInfoForSLD(5);
    }

    if (memc_logo_to_demura_update_cnt == 0) {
        memc_logo_to_demura_update_cnt = 1;
    }

    if (frame_cnt % memc_logo_to_demura_update_cnt == 0) { // read once every update_cnt frames
        if (!sld_work.read_ok) {
            //time_start = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
            // read memc logo
            //time_start = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
            if (memc_logo_read_en && LD_virAddr != NULL && MEMC_Lib_GetInfoForSLD(7)) {
                // read whole memc logo memory
                memcpy(logo_flag_map_raw, logo_ptr, 192 * 540);
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
                //  do_counter_update = false;
                if (memc_sc_motion1 < 100 && memc_sc_motion2 < 100) { // motion cnt low: picture pause
                    sld_work.do_counter_update = false;
                }
                else if (memc_sc_flag && memc_sc_motion1 > 10000 && memc_sc_motion2 > 10000) { // scene change flag + big motion: scene change
                    sld_work.do_counter_update = true;
                    sld_work.do_reset_counter = true;
                }
                sld_work.do_reset_full = false;

				memc_sc_flag_debug = memc_sc_flag;
				memc_sc_motion1_debug = memc_sc_motion1; 
				memc_sc_motion2_debug = memc_sc_motion2;
				memc_flag_ok_debug = MEMC_Lib_GetInfoForSLD(7);

                // reset motion flag after used
                memc_sc_flag = false;
                memc_sc_motion1 = 0;
                memc_sc_motion2 = 0;
            }
            else {
                sld_work.do_counter_update = true;
                sld_work.do_reset_counter = true;
                sld_work.do_reset_full = true;

				memc_sc_flag_debug = memc_sc_flag;
				memc_sc_motion1_debug = memc_sc_motion1; 
				memc_sc_motion2_debug = memc_sc_motion2;
				memc_flag_ok_debug = MEMC_Lib_GetInfoForSLD(7);
            }
            if (SLD_BY_MEMC_DEMURA_LOGO_ONLY == 0) {
                // LD related info
                ld_global_ctrl2_reg.regValue = IoReg_Read32(BLU_LD_Global_Ctrl2_reg);
                LD_valid = ld_global_ctrl2_reg.ld_valid;
                LD_type = ld_global_ctrl2_reg.ld_blk_type;
                LD_Hnum = ld_global_ctrl2_reg.ld_blk_hnum + 1;
                LD_Vnum = ld_global_ctrl2_reg.ld_blk_vnum + 1;

                if (LD_valid && LD_type == 0 && LD_Hnum == 32 && LD_Vnum == 18 && memc_logo_LD_APL_check == 1) {
                    sld_work.LD_APL_valid = drvif_color_get_LD_APL_ave(blk_apl_average);
                }
                else {
                    sld_work.LD_APL_valid = false;
                }
            }
            else {
                sld_work.LD_APL_valid = false;
            }

            sld_work.read_ok = true;

            // trigger apply
            schedule_work(&(sld_work.sld_apply_work));
            //time_end = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
            //rtd_pr_vpq_emerg("phase1 time: %d on 90k clock\n", (time_end-time_start));

        }
        else { // read period is faster than calculation time
            //rtd_pr_vpq_emerg("SLD read stopped because apply is not done yet!\n");
        }

    }

    frame_cnt++;
#endif
}

sld_work_debug_struct sld_work_debug = {0};
static void memc_logo_to_demura_debug_log(int drop_block)
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
            printk("sw_sld=debug_info=%d,%d,%d,%d,%d,\n", idx_0, idx_1, idx_2, x, y);
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

            printk("sw_sld=memc_info=%d,%d,%d,%d,t=%d,%d,osd,%d,%d,%d,%d,drop=%d,\n",
                   memc_sc_flag_debug, memc_sc_motion1_debug, memc_sc_motion2_debug,
                   memc_flag_ok_debug, wait_time, drop_time, sld_work.osd_sta_blkx,
                   sld_work.osd_sta_blky, sld_work.osd_end_blkx, sld_work.osd_end_blky, drop_block);
        }
    }

    if ((en & _BIT2) != 0) {
        if (cnt % delay == 0) {
            idx_2 = h_num * y + x;
            idx_1 = ((idx_2 - 1) < 0) ? (0) : (idx_2 - 1);
            idx_0 = ((idx_1 - 1) < 0) ? (0) : (idx_1 - 1);
            printk("sw_sld=log_map[noFH:FH]=%d:%d,%d:%d,%d:%d,\n", 
				logo_flag_map_no_fill_hole[idx_0], logo_flag_map[idx_0], logo_flag_map_no_fill_hole[idx_1], logo_flag_map[idx_1],
                   logo_flag_map_no_fill_hole[idx_2], logo_flag_map[idx_2]);
        }
    }

    if ((en & _BIT3) != 0) {
        if (cnt % delay == 0) {
            idx_2 = h_num * y + x;
            idx_1 = ((idx_2 - 1) < 0) ? (0) : (idx_2 - 1);
            idx_0 = ((idx_1 - 1) < 0) ? (0) : (idx_1 - 1);
            printk("sw_sld=cnt_map=%d,%d,%d,\n", logo_demura_counter[idx_0], logo_demura_counter[idx_1],
                   logo_demura_counter[idx_2]);
        }
    }

    if ((en & _BIT4) != 0) {
        if (cnt % delay == 0) {
            idx_2 = h_num * y + x;
            idx_1 = ((idx_2 - 1) < 0) ? (0) : (idx_2 - 1);
            idx_0 = ((idx_1 - 1) < 0) ? (0) : (idx_1 - 1);
            printk("sw_sld=apl=%d,%d,%d,\n", blk_apl_interp[idx_0], blk_apl_interp[idx_1],
                   blk_apl_interp[idx_2]);
        }
    }

    if ((en & _BIT5) != 0) {
        if (cnt % delay == 0) {
            idx_2 = h_num * y + x;
            idx_1 = ((idx_2 - 1) < 0) ? (0) : (idx_2 - 1);
            idx_0 = ((idx_1 - 1) < 0) ? (0) : (idx_1 - 1);
            printk("sw_sld=demura=%d,%d,%d,\n", demura_tbl[2][idx_0], demura_tbl[2][idx_1],
                   demura_tbl[2][idx_2]);
        }
    }

    if ((en & _BIT6) != 0) {
        if (cnt % delay == 0) {
            idx_2 = h_num * y + x;
            idx_1 = ((idx_2 - 1) < 0) ? (0) : (idx_2 - 1);
            idx_0 = ((idx_1 - 1) < 0) ? (0) : (idx_1 - 1);
            printk("sw_sld=blkapl=%d:%d,%d:%d,%d:%d,diff=%d,\n", blk_apl_interp_pre[idx_0], blk_apl_interp[idx_0],
                   blk_apl_interp_pre[idx_1], blk_apl_interp[idx_1], blk_apl_interp_pre[idx_2],
                   blk_apl_interp[idx_2], SLD_ABS(blk_apl_interp_pre[idx_1], blk_apl_interp[idx_1]));
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
}


char memc_logo_to_demura_4circle_detect(void)
{
    unsigned short en = sld_work_debug.pr_En;
    unsigned short delay = sld_work_debug.pr_delay;
    static unsigned int cnt;
    unsigned int ratio = 820; // %%

    unsigned int i, j, index;
    unsigned int sum = 0;
    unsigned int total = 0;
    unsigned int th; //
    index = 0;
    sum = 0;
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


    if (delay == 0) {
        delay = 1;
    }

    if ((en & _BIT9) != 0) {
        if (cnt % delay == 0) {
            printk("sw_sld, sum=%d,th=%d,\n", sum, th);

        }
    }

    cnt++;

    if (sum > th) {
        return 1;
    }
    else {
        return 0;
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
        //printk("sw_sld,LR_LOGO_Flag_L and R is %d,%d\n", LR_LOGO_Flag_L, LR_LOGO_Flag_R);
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
            printk("sw_sld,L_cnt=%d,th=%d,R_cnt=%d,th=%d,,,,%d,%d,\n", 
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

#if IS_ENABLED(CONFIG_RTK_KDRV_SE)
static int memc_logo_to_demura_Filter_WaitSEDone(void) 
{
	int SEProcCount = 0;
	int Ret = 0;
	int SEProcCount_max = 5;
	while((IoReg_Read32(SE_IDLE_0_reg)&(0x01)) != 0x01) {
		if( SEProcCount >= SEProcCount_max ) {
			Ret = false;
			break;
		}
		usleep_range(4000, 5000);
		SEProcCount++;
	}
	if( SEProcCount >= SEProcCount_max ) {
		rtd_pr_vpq_emerg("[%s] SEProcCount = %d\n", __FUNCTION__, SEProcCount);
		Ret = -1;
	}
	return Ret;
}
#endif

short demura_apply[3][241*136];
extern unsigned long get_query_start_address(unsigned char idx);
char memc_logo_to_demura_Filter_241x136_YMode_By_SE(unsigned char filter_num)
{
	char ret = 0;
#if IS_ENABLED(CONFIG_RTK_KDRV_SE)
	KGAL_SURFACE_INFO_T SrcSurf; 
	KGAL_SURFACE_INFO_T DstSurf;

	unsigned short i, j, ii, jj;
	unsigned short width, height;
	unsigned short width_se, height_se;
	unsigned int phyAddress_Dst;
	unsigned int phyAddress_Src;
	unsigned char *virAddress_Dst;
	unsigned char *virAddress_Src;

	unsigned int base_addr;
	unsigned int *vir_addr;
	unsigned int base_addr_aligned = 0, sizeDemuraTbl_aligned;
	unsigned int  *vir_addr_aligned = NULL;
#ifdef CONFIG_ARM64 //ARM32 compatible
	unsigned long va_temp;
#else
	unsigned int va_temp;
#endif
	//unsigned char aligned_val = 0;
	//unsigned int sizeDemuraTbl = sizeof(unsigned char)*VIP_Demura_Encode_TBL_ROW*VIP_Demura_Encode_TBL_COL;
	unsigned int size_offset;
	unsigned int sizeDemuraTbl;
	unsigned int Src_Dst_addr_offset;
	unsigned char i_addr;
	unsigned char end_buff_addr = 0xff;	// 0xff = init val
	int SE_done_flag;
#if 1	// odd
	// coef and type 
	unsigned int HorCoef[4] = {	 0, 1368,	 0, 1368 }; // SEHorCoef : Spatial filter SE horizontal coefficient
	unsigned int VerCoef[4] = {	 0, 1368,	 0, 1368 }; // SEVerCoef : Spatial filter SE vertical coefficient
	unsigned char HorFIRType = 1;							// HorOddType : SE hor FIR type ( 0:even 1:odd )
	unsigned char VerFIRType = 1;							// VerOddType : SE ver FIR type ( 0:even 1:odd )
#else	// even
	// coef and type 
	unsigned int HorCoef[4] = {	 1368,	 0, 1368, 0 }; // SEHorCoef : Spatial filter SE horizontal coefficient
	unsigned int VerCoef[4] = {	 1368,	 0, 1368, 0 }; // SEVerCoef : Spatial filter SE vertical coefficient
	unsigned char HorFIRType = 0;							// HorOddType : SE hor FIR type ( 0:even 1:odd )
	unsigned char VerFIRType = 0;							// VerOddType : SE ver FIR type ( 0:even 1:odd )
#endif

	if ((DeMura_TBL.table_mode == DeMura_TBL_241x136) && (DeMura_TBL.DeMura_CTRL_TBL.demura_rgb_table_seperate == 0)) {
		width = 241;
		height = 136;
		width_se = 240;		// system will kernel panic if width_se = 241 or 242, why?
		height_se = 136;
		size_offset = 10 * 1024; // avoid trash other memory while size do 4k and 96B aligned
		sizeDemuraTbl = 200 * 1024 - size_offset; // 200k for SE filter, only support 241x136 y mode since memory is not enough in Demura
		Src_Dst_addr_offset = 100 * 1024;

		base_addr = get_query_start_address(QUERY_IDX_VIP_DeMura) + (800 * 1024);	// use last 200kB
		base_addr = drvif_memory_get_data_align(base_addr, (1 << 12));
		sizeDemuraTbl = drvif_memory_get_data_align(sizeDemuraTbl, (1 << 12));
		Src_Dst_addr_offset = drvif_memory_get_data_align(Src_Dst_addr_offset, (1 << 12));
		
		if (base_addr == 0) {
				rtd_pr_vpq_emerg("[%s] addr = NULL, DeMura_TBL.table_mode=%d, \n", __func__, DeMura_TBL.table_mode);
				ret = -1;
		} else {
	
			vir_addr = dvr_remap_uncached_memory(base_addr, sizeDemuraTbl, __builtin_return_address(0));
			sizeDemuraTbl_aligned = sizeDemuraTbl;
			base_addr_aligned = base_addr;
#ifdef CONFIG_ARM64 //ARM32 compatible
			va_temp = (unsigned long)vir_addr;
#else
			va_temp = (unsigned int)vir_addr;
#endif
			vir_addr_aligned = (unsigned int*)va_temp;
			//aligned_val = 0;

#ifdef CONFIG_BW_96B_ALIGNED
			sizeDemuraTbl_aligned = dvr_size_alignment(sizeDemuraTbl);
			base_addr_aligned = (unsigned int)dvr_memory_alignment((unsigned long)base_addr, sizeDemuraTbl_aligned);
#ifdef CONFIG_ARM64 //ARM32 compatible
			va_temp = (unsigned long)vir_addr;
#else //CONFIG_ARM64
			va_temp = (unsigned int)vir_addr;
#endif //CONFIG_ARM64
			vir_addr_aligned = (unsigned int*)dvr_memory_alignment((unsigned long)va_temp, sizeDemuraTbl_aligned);
			//aligned_val = 6;
#endif

			Src_Dst_addr_offset = dvr_size_alignment(Src_Dst_addr_offset);

			if ((vir_addr_aligned == NULL) || (vir_addr_aligned == NULL)) {
				rtd_pr_vpq_emerg("[%s] va addr = %lx, %lx  \n", __func__, (unsigned long)vir_addr_aligned, (unsigned long)vir_addr);
				ret = -1;

			}
			else {
				end_buff_addr = 0xff;
				for (i_addr = 0;i_addr<filter_num;i_addr++) {
					// switch address to phyAddress_Src and phyAddress_Dsr
					if ((end_buff_addr == 0) || (end_buff_addr == 0xff)) {
						phyAddress_Src = base_addr_aligned;
						phyAddress_Dst = base_addr_aligned + Src_Dst_addr_offset;

						virAddress_Src = (unsigned char*)vir_addr_aligned;
						virAddress_Dst = (unsigned char*)(((unsigned char*)vir_addr_aligned) + Src_Dst_addr_offset);
						// get ini table from demura tbl
						if (end_buff_addr == 0xff) {
							for (i=0;i<height_se;i++) {
								ii = (i >= height)?(height-1):(i);
								for (j=0;j<width_se;j++) {
									jj = (j >= width)?(width-1):(j);
									virAddress_Src[i*width_se+j] = demura_tbl[2][ii*width+jj] * (-1);	// demura offset is neg
								}
							}
						}
						end_buff_addr = 1;
					} else {
						phyAddress_Src = base_addr_aligned + Src_Dst_addr_offset;
						phyAddress_Dst = base_addr_aligned;
						
						virAddress_Src = (unsigned char*)(((unsigned char*)vir_addr_aligned) + Src_Dst_addr_offset);
						virAddress_Dst = (unsigned char*)vir_addr_aligned;
						end_buff_addr = 0;
					}
				
#if 0
					vas = virAddress_Src;
					printk("vSrc,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vas[242*0+0],vas[242*0+1],vas[242*0+2],vas[242*0+3],vas[242*0+4],vas[242*0+5],vas[242*0+6],vas[242*0+7],vas[242*0+8]);
					printk("vSrc,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vas[242*1+0],vas[242*1+1],vas[242*1+2],vas[242*1+3],vas[242*1+4],vas[242*1+5],vas[242*1+6],vas[242*1+7],vas[242*1+8]);
					printk("vSrc,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vas[242*2+0],vas[242*2+1],vas[242*2+2],vas[242*2+3],vas[242*2+4],vas[242*2+5],vas[242*2+6],vas[242*2+7],vas[242*2+8]);
					printk("vSrc,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vas[242*3+0],vas[242*3+1],vas[242*3+2],vas[242*3+3],vas[242*3+4],vas[242*3+5],vas[242*3+6],vas[242*3+7],vas[242*3+8]);
					printk("vSrc,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vas[242*4+0],vas[242*4+1],vas[242*4+2],vas[242*4+3],vas[242*4+4],vas[242*4+5],vas[242*4+6],vas[242*4+7],vas[242*4+8]);
					printk("vSrc,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vas[242*5+0],vas[242*5+1],vas[242*5+2],vas[242*5+3],vas[242*5+4],vas[242*5+5],vas[242*5+6],vas[242*5+7],vas[242*5+8]);
					printk("vSrc,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vas[242*6+0],vas[242*6+1],vas[242*6+2],vas[242*6+3],vas[242*6+4],vas[242*6+5],vas[242*6+6],vas[242*6+7],vas[242*6+8]);
					printk("vSrc,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vas[242*7+0],vas[242*7+1],vas[242*7+2],vas[242*7+3],vas[242*7+4],vas[242*7+5],vas[242*7+6],vas[242*7+7],vas[242*7+8]);
					printk("vSrc,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vas[242*8+0],vas[242*8+1],vas[242*8+2],vas[242*8+3],vas[242*8+4],vas[242*8+5],vas[242*8+6],vas[242*8+7],vas[242*8+8]);
#endif

					//memcpy(virAddress_Src, demura_apply[2], 241 * 136 * sizeof(short));					

					SrcSurf.physicalAddress = phyAddress_Src;
					SrcSurf.pixelFormat = KGAL_PIXEL_FORMAT_NV12_Y;

					SrcSurf.pitch = 240*8/8; // bytes per line,
					SrcSurf.width = 240;
					SrcSurf.height = 136;

					SrcSurf.bpp = 8;
					
					DstSurf.physicalAddress = phyAddress_Dst;
					DstSurf.pixelFormat = KGAL_PIXEL_FORMAT_NV12_Y;

					DstSurf.pitch = 240*8/8; // bytes per line,
					DstSurf.width = 240;
					DstSurf.height = 136;

					DstSurf.bpp = 8;

					KGAL_FWSLD_Stretch_Coef(&SrcSurf, &DstSurf,&HorCoef[0], HorFIRType,&VerCoef[0], VerFIRType);

					SE_done_flag = memc_logo_to_demura_Filter_WaitSEDone();
					if (SE_done_flag != 0) {
						ret = -1;
						break;
					}
#if 0
					vad = virAddress_Dst;					
					printk("vDst,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vad[242*0+0],vad[242*0+1],vad[242*0+2],vad[242*0+3],vad[242*0+4],vad[242*0+5],vad[242*0+6],vad[242*0+7],vad[242*0+8]);
					printk("vDst,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vad[242*1+0],vad[242*1+1],vad[242*1+2],vad[242*1+3],vad[242*1+4],vad[242*1+5],vad[242*1+6],vad[242*1+7],vad[242*1+8]);
					printk("vDst,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vad[242*2+0],vad[242*2+1],vad[242*2+2],vad[242*2+3],vad[242*2+4],vad[242*2+5],vad[242*2+6],vad[242*2+7],vad[242*2+8]);
					printk("vDst,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vad[242*3+0],vad[242*3+1],vad[242*3+2],vad[242*3+3],vad[242*3+4],vad[242*3+5],vad[242*3+6],vad[242*3+7],vad[242*3+8]);
					printk("vDst,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vad[242*4+0],vad[242*4+1],vad[242*4+2],vad[242*4+3],vad[242*4+4],vad[242*4+5],vad[242*4+6],vad[242*4+7],vad[242*4+8]);
					printk("vDst,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vad[242*5+0],vad[242*5+1],vad[242*5+2],vad[242*5+3],vad[242*5+4],vad[242*5+5],vad[242*5+6],vad[242*5+7],vad[242*5+8]);
					printk("vDst,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vad[242*6+0],vad[242*6+1],vad[242*6+2],vad[242*6+3],vad[242*6+4],vad[242*6+5],vad[242*6+6],vad[242*6+7],vad[242*6+8]);
					printk("vDst,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vad[242*7+0],vad[242*7+1],vad[242*7+2],vad[242*7+3],vad[242*7+4],vad[242*7+5],vad[242*7+6],vad[242*7+7],vad[242*7+8]);
					printk("vDst,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vad[242*8+0],vad[242*8+1],vad[242*8+2],vad[242*8+3],vad[242*8+4],vad[242*8+5],vad[242*8+6],vad[242*8+7],vad[242*8+8]);
#endif
				}

				if (SE_done_flag == 0) {
					for (i=0;i<height;i++) {
						ii = (i >= height_se)?(height_se-1):(i);
						for (j=0;j<width;j++) {						   
							jj = (j >= width_se)?(width_se-1):(j);
							demura_tbl[2][i*width+j] = virAddress_Dst[ii*width_se+jj] * (-1);
						}
					}			   
					memcpy(demura_tbl[1], demura_tbl[2], 241 * 136 * sizeof(short));    
					ret = 0;
				} else {
					rtd_pr_vpq_emerg("[%s]can't get SE done \n", __func__);
					ret = -1;				
				}

				dvr_unmap_memory((void *)vir_addr, sizeDemuraTbl);
			}
		} 
	}
	else {
			rtd_pr_vpq_emerg("[%s] demura mode not support \n", __func__);
			ret = 1;
	}

	// debug, check se coef
#if 0
	// 0x680 -> 0xA70, total is 64
	/*SE20INFO_REG_HCOEF1_32 = 0x680,
	SE20INFO_REG_HCOEF1_32_2 = 0x780,
	SE20INFO_REG_VCOEF1_32 = 0x880,
	SE20INFO_REG_VCOEF1_32_2 = 0x980,*/
	for (i=0;i<64;i++) {
		base_addr = (0xb800C000 + (0x680/*SE20INFO_REG_HCOEF1_32*/ + i * 0x10));
		if (i<32)
			printk("SE20_S1_HCOEFF%d[%x]=%x,\n", i, base_addr, IoReg_Read32(base_addr));
		else
			printk("SE20_S1_VCOEFF%d[%x]=%x,\n", i-32, base_addr, IoReg_Read32(base_addr));
	}
#endif		
#endif
	return ret;
}

// yvonne add for SLD 20220419
char memc_logo_to_demura_Filter_241x136_SLD_APL_By_SE(unsigned char filter_num)
{
#if 0
	KGAL_SURFACE_INFO_T SrcSurf; 
	KGAL_SURFACE_INFO_T DstSurf;

	char ret = 0;
	unsigned short i, j, ii, jj;
	unsigned short width, height;
	unsigned short width_se, height_se;
	unsigned int phyAddress_Dst;
	unsigned int phyAddress_Src;
	unsigned char *virAddress_Dst;
	unsigned char *virAddress_Src;

	unsigned int base_addr;
	unsigned int *vir_addr;
	unsigned int base_addr_aligned = 0, sizeDemuraTbl_aligned;
	unsigned int  *vir_addr_aligned = NULL;
#ifdef CONFIG_ARM64 //ARM32 compatible
	unsigned long va_temp;
#else
	unsigned int va_temp;
#endif
	unsigned char aligned_val = 0;
	//unsigned int sizeDemuraTbl = sizeof(unsigned char)*VIP_Demura_Encode_TBL_ROW*VIP_Demura_Encode_TBL_COL;
	unsigned int size_offset;
	unsigned int sizeDemuraTbl;
	unsigned int Src_Dst_addr_offset;
	unsigned char i_addr;
	unsigned char end_buff_addr = 0xff;	// 0xff = init val
	int SE_done_flag;
#if 1	// odd
	// coef and type 
	unsigned int HorCoef[4] = {	 0, 1368,	 0, 1368 }; // SEHorCoef : Spatial filter SE horizontal coefficient
	unsigned int VerCoef[4] = {	 0, 1368,	 0, 1368 }; // SEVerCoef : Spatial filter SE vertical coefficient
	unsigned char HorFIRType = 1;							// HorOddType : SE hor FIR type ( 0:even 1:odd )
	unsigned char VerFIRType = 1;							// VerOddType : SE ver FIR type ( 0:even 1:odd )
#else	// even
	// coef and type 
	unsigned int HorCoef[4] = {	 1368,	 0, 1368, 0 }; // SEHorCoef : Spatial filter SE horizontal coefficient
	unsigned int VerCoef[4] = {	 1368,	 0, 1368, 0 }; // SEVerCoef : Spatial filter SE vertical coefficient
	unsigned char HorFIRType = 0;							// HorOddType : SE hor FIR type ( 0:even 1:odd )
	unsigned char VerFIRType = 0;							// VerOddType : SE ver FIR type ( 0:even 1:odd )
#endif

	if ((DeMura_TBL.table_mode == DeMura_TBL_241x136) && (DeMura_TBL.DeMura_CTRL_TBL.demura_rgb_table_seperate == 0)) {
		width = 241;
		height = 136;
		width_se = 240;		// system will kernel panic if width_se = 241 or 242, why?
		height_se = 136;
		size_offset = 10 * 1024; // avoid trash other memory while size do 4k and 96B aligned
		sizeDemuraTbl = 200 * 1024 - size_offset; // 200k for SE filter, only support 241x136 y mode since memory is not enough in Demura
		Src_Dst_addr_offset = 100 * 1024;

		base_addr = get_query_start_address(QUERY_IDX_VIP_DeMura) + (800 * 1024);	// use last 200kB
		base_addr = drvif_memory_get_data_align(base_addr, (1 << 12));
		sizeDemuraTbl = drvif_memory_get_data_align(sizeDemuraTbl, (1 << 12));
		Src_Dst_addr_offset = drvif_memory_get_data_align(Src_Dst_addr_offset, (1 << 12));
		
		if (base_addr == 0) {
				rtd_pr_vpq_emerg("[%s] addr = NULL, DeMura_TBL.table_mode=%d, \n", __func__, DeMura_TBL.table_mode);
				ret = -1;
		} else {
	
			vir_addr = dvr_remap_uncached_memory(base_addr, sizeDemuraTbl, __builtin_return_address(0));
			sizeDemuraTbl_aligned = sizeDemuraTbl;
			base_addr_aligned = base_addr;
#ifdef CONFIG_ARM64 //ARM32 compatible
			va_temp = (unsigned long)vir_addr;
#else
			va_temp = (unsigned int)vir_addr;
#endif
			vir_addr_aligned = (unsigned int*)va_temp;
			aligned_val = 0;

#ifdef CONFIG_BW_96B_ALIGNED
			sizeDemuraTbl_aligned = dvr_size_alignment(sizeDemuraTbl);
			base_addr_aligned = (unsigned int)dvr_memory_alignment((unsigned long)base_addr, sizeDemuraTbl_aligned);
#ifdef CONFIG_ARM64 //ARM32 compatible
			va_temp = (unsigned long)vir_addr;
#else //CONFIG_ARM64
			va_temp = (unsigned int)vir_addr;
#endif //CONFIG_ARM64
			vir_addr_aligned = (unsigned int*)dvr_memory_alignment((unsigned long)va_temp, sizeDemuraTbl_aligned);
			aligned_val = 6;
#endif

			Src_Dst_addr_offset = dvr_size_alignment(Src_Dst_addr_offset);

			if ((vir_addr_aligned == NULL) || (vir_addr_aligned == NULL)) {
				rtd_pr_vpq_emerg("[%s] va addr = %lx, %lx  \n", __func__, (unsigned long)vir_addr_aligned, (unsigned long)vir_addr);
				ret = -1;

			}
			else {
				end_buff_addr = 0xff;
				for (i_addr = 0;i_addr<filter_num;i_addr++) {
					// switch address to phyAddress_Src and phyAddress_Dsr
					if ((end_buff_addr == 0) || (end_buff_addr == 0xff)) {
						phyAddress_Src = base_addr_aligned;
						phyAddress_Dst = base_addr_aligned + Src_Dst_addr_offset;

						virAddress_Src = (unsigned char*)vir_addr_aligned;
						virAddress_Dst = (unsigned char*)(((unsigned char*)vir_addr_aligned) + Src_Dst_addr_offset);
						// get ini table from demura tbl
						if (end_buff_addr == 0xff) {
							for (i=0;i<height_se;i++) {
								ii = (i >= height)?(height-1):(i);
								for (j=0;j<width_se;j++) {
									jj = (j >= width)?(width-1):(j);
									virAddress_Src[i*width_se+j] = blk_apl_interp[0];
								}
							}
						}
						end_buff_addr = 1;
					} else {
						phyAddress_Src = base_addr_aligned + Src_Dst_addr_offset;
						phyAddress_Dst = base_addr_aligned;
						
						virAddress_Src = (unsigned char*)(((unsigned char*)vir_addr_aligned) + Src_Dst_addr_offset);
						virAddress_Dst = (unsigned char*)vir_addr_aligned;
						end_buff_addr = 0;
					}
				
#if 0
					vas = virAddress_Src;
					printk("vSrc,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vas[242*0+0],vas[242*0+1],vas[242*0+2],vas[242*0+3],vas[242*0+4],vas[242*0+5],vas[242*0+6],vas[242*0+7],vas[242*0+8]);
					printk("vSrc,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vas[242*1+0],vas[242*1+1],vas[242*1+2],vas[242*1+3],vas[242*1+4],vas[242*1+5],vas[242*1+6],vas[242*1+7],vas[242*1+8]);
					printk("vSrc,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vas[242*2+0],vas[242*2+1],vas[242*2+2],vas[242*2+3],vas[242*2+4],vas[242*2+5],vas[242*2+6],vas[242*2+7],vas[242*2+8]);
					printk("vSrc,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vas[242*3+0],vas[242*3+1],vas[242*3+2],vas[242*3+3],vas[242*3+4],vas[242*3+5],vas[242*3+6],vas[242*3+7],vas[242*3+8]);
					printk("vSrc,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vas[242*4+0],vas[242*4+1],vas[242*4+2],vas[242*4+3],vas[242*4+4],vas[242*4+5],vas[242*4+6],vas[242*4+7],vas[242*4+8]);
					printk("vSrc,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vas[242*5+0],vas[242*5+1],vas[242*5+2],vas[242*5+3],vas[242*5+4],vas[242*5+5],vas[242*5+6],vas[242*5+7],vas[242*5+8]);
					printk("vSrc,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vas[242*6+0],vas[242*6+1],vas[242*6+2],vas[242*6+3],vas[242*6+4],vas[242*6+5],vas[242*6+6],vas[242*6+7],vas[242*6+8]);
					printk("vSrc,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vas[242*7+0],vas[242*7+1],vas[242*7+2],vas[242*7+3],vas[242*7+4],vas[242*7+5],vas[242*7+6],vas[242*7+7],vas[242*7+8]);
					printk("vSrc,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vas[242*8+0],vas[242*8+1],vas[242*8+2],vas[242*8+3],vas[242*8+4],vas[242*8+5],vas[242*8+6],vas[242*8+7],vas[242*8+8]);
#endif

					//memcpy(virAddress_Src, demura_apply[2], 241 * 136 * sizeof(short));					

					SrcSurf.physicalAddress = phyAddress_Src;
					SrcSurf.pixelFormat = KGAL_PIXEL_FORMAT_NV12_Y;

					SrcSurf.pitch = 240*8/8; // bytes per line,
					SrcSurf.width = 240;
					SrcSurf.height = 136;

					SrcSurf.bpp = 8;
					
					DstSurf.physicalAddress = phyAddress_Dst;
					DstSurf.pixelFormat = KGAL_PIXEL_FORMAT_NV12_Y;

					DstSurf.pitch = 240*8/8; // bytes per line,
					DstSurf.width = 240;
					DstSurf.height = 136;

					DstSurf.bpp = 8;

					KGAL_FWSLD_Stretch_Coef(&SrcSurf, &DstSurf,&HorCoef[0], HorFIRType,&VerCoef[0], VerFIRType);

					SE_done_flag = memc_logo_to_demura_Filter_WaitSEDone();
					if (SE_done_flag != 0) {
						ret = -1;
						break;
					}
#if 0
					vad = virAddress_Dst;					
					printk("vDst,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vad[242*0+0],vad[242*0+1],vad[242*0+2],vad[242*0+3],vad[242*0+4],vad[242*0+5],vad[242*0+6],vad[242*0+7],vad[242*0+8]);
					printk("vDst,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vad[242*1+0],vad[242*1+1],vad[242*1+2],vad[242*1+3],vad[242*1+4],vad[242*1+5],vad[242*1+6],vad[242*1+7],vad[242*1+8]);
					printk("vDst,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vad[242*2+0],vad[242*2+1],vad[242*2+2],vad[242*2+3],vad[242*2+4],vad[242*2+5],vad[242*2+6],vad[242*2+7],vad[242*2+8]);
					printk("vDst,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vad[242*3+0],vad[242*3+1],vad[242*3+2],vad[242*3+3],vad[242*3+4],vad[242*3+5],vad[242*3+6],vad[242*3+7],vad[242*3+8]);
					printk("vDst,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vad[242*4+0],vad[242*4+1],vad[242*4+2],vad[242*4+3],vad[242*4+4],vad[242*4+5],vad[242*4+6],vad[242*4+7],vad[242*4+8]);
					printk("vDst,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vad[242*5+0],vad[242*5+1],vad[242*5+2],vad[242*5+3],vad[242*5+4],vad[242*5+5],vad[242*5+6],vad[242*5+7],vad[242*5+8]);
					printk("vDst,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vad[242*6+0],vad[242*6+1],vad[242*6+2],vad[242*6+3],vad[242*6+4],vad[242*6+5],vad[242*6+6],vad[242*6+7],vad[242*6+8]);
					printk("vDst,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vad[242*7+0],vad[242*7+1],vad[242*7+2],vad[242*7+3],vad[242*7+4],vad[242*7+5],vad[242*7+6],vad[242*7+7],vad[242*7+8]);
					printk("vDst,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,\n",vad[242*8+0],vad[242*8+1],vad[242*8+2],vad[242*8+3],vad[242*8+4],vad[242*8+5],vad[242*8+6],vad[242*8+7],vad[242*8+8]);
#endif
				}

				if (SE_done_flag == 0) {
					for (i=0;i<height;i++) {
						ii = (i >= height_se)?(height_se-1):(i);
						for (j=0;j<width;j++) {						   
							jj = (j >= width_se)?(width_se-1):(j);
							blk_apl_interp[i*width+j] = virAddress_Dst[ii*width_se+jj];
						}
					}			   
					//memcpy(demura_tbl[1], demura_tbl[2], 241 * 136 * sizeof(short));    
					ret = 0;
				} else {
					rtd_pr_vpq_emerg("[%s]can't get SE done \n", __func__);
					ret = -1;				
				}

				dvr_unmap_memory((void *)vir_addr, sizeDemuraTbl);
			}
		} 
	}
	else {
			rtd_pr_vpq_emerg("[%s] demura mode not support \n", __func__);
			ret = 1;
	}

	// debug, check se coef
#if 0
	// 0x680 -> 0xA70, total is 64
	/*SE20INFO_REG_HCOEF1_32 = 0x680,
	SE20INFO_REG_HCOEF1_32_2 = 0x780,
	SE20INFO_REG_VCOEF1_32 = 0x880,
	SE20INFO_REG_VCOEF1_32_2 = 0x980,*/
	for (i=0;i<64;i++) {
		base_addr = (0xb800C000 + (0x680/*SE20INFO_REG_HCOEF1_32*/ + i * 0x10));
		if (i<32)
			printk("SE20_S1_HCOEFF%d[%x]=%x,\n", i, base_addr, IoReg_Read32(base_addr));
		else
			printk("SE20_S1_VCOEFF%d[%x]=%x,\n", i-32, base_addr, IoReg_Read32(base_addr));
	}
#endif		
	return ret;
#else
	return 0;
#endif
}


#define ID_4cir_deBounce_th 1
#define sp_gain_blending_max 32
unsigned int circle4_cnt = 0;
int filter_2D_sum_col[demura_filter_size];
static int coutner_procees_time_max;
static void memc_logo_to_demura_apply(struct work_struct *work)
{
	extern unsigned short LC_APL[LC_GSR_REPORT_H_BLKS*LC_GSR_REPORT_V_BLKS];
	int filter_sum_APL;
	unsigned char filter_2d_remove;
    unsigned char offbyhwsld = 1;
    int i, j, k;
	int il = 0;// i * cj1;
	int ilj = 0;// il + j;
	int ci0=270;
	int ci1=271;
	int cj0=480;
	int cj1=481;
	unsigned int SLD_BLK_idx;
	unsigned short SLD_BLK_h, SLD_BLK_v;
	unsigned int sld_blk_i, sld_blk_j;
	short idx_x, idx_y;
	short dmr_x, dmr_y;
	short dmr_yy[demura_filter_size];
	short dmr_xx[demura_filter_size] = {0, 0, 0, 0, 0, 1, 2, 3, 4};
    unsigned int cnt_temp;
    //static unsigned char  pre_circle4_flag = 0;
    int wait_time = 1; //memc_logo_to_demura_wait_time * 120 / memc_logo_to_demura_update_cnt;
    int drop_time = 1; //memc_logo_to_demura_drop_time * 120 / memc_logo_to_demura_update_cnt;
    //int update_cnt = 1;
    int drop_buffer = 20;
    static unsigned char memc_logo_to_demura_drop_limit_CNT = 0;
    int drop_block = 0;
	int drop_block_APL = 0;
	int demura_cnt_APL = 0;
    int frm_rate = Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ);
    //unsigned int time_start, time_end;
	unsigned long flags = 0;
	int debug_no_interrupt=0;
	int debug_log_time=0;
	unsigned int circle_apply_x0, circle_apply_x1, circle_apply_y0, circle_apply_y1;
	static unsigned short memc_logo_to_demura_drop_limit_pre = 0xffff;
	int HPhase = 0, VPhase = 0; // 2^12 factor + 2^8 bledning
	int HPos = 0, VPos = 0;
	int HBldCoef = 0, VBldCoef = 0;
	int APLBlkH1 = 0, APLBlkH2 = 0, APLBlkV1 = 0, APLBlkV2 = 0;
	int APLBlk1 = 0, APLBlk2 = 0, APLBlk3 = 0, APLBlk4 = 0;
	int HBldVal1 = 0, HBldVal2 = 0;
	unsigned int HBlks = 0, VBlks = 0;
	
	color_sld_sld_cnt_0_RBUS reg_color_sld_sld_cnt_0_reg;
	color_sld_sld_control_RBUS color_sld_sld_control_reg;
	
	int drop_limit, drop_limit_sp, drop_limit_sp_temp;
	//static unsigned char circlr4_status_pre = 0xff;
	static short sp_gain_blending = 0;
    lc_lc_global_ctrl2_RBUS blu_lc_global_ctrl2;
	color_d_vc_global_ctrl_RBUS color_d_vc_global_ctrl;

	color_d_vc_global_ctrl.regValue = IoReg_Read32(COLOR_D_VC_Global_CTRL_reg);
	blu_lc_global_ctrl2.regValue = IoReg_Read32(LC_LC_Global_Ctrl2_reg);
	HBlks = blu_lc_global_ctrl2.lc_blk_hnum+1;
	VBlks = blu_lc_global_ctrl2.lc_blk_vnum+1;

	
	if (memc_logo_to_demura_drop_limit_pre == 0xffff)
		memc_logo_to_demura_drop_limit_pre = memc_logo_to_demura_drop_limit;

	if (memc_logo_to_demura_drop_limit_CNT >= 2) {
		if (memc_logo_to_demura_drop_limit_pre > memc_logo_to_demura_drop_limit)
			memc_logo_to_demura_drop_limit_pre--;
		else if (memc_logo_to_demura_drop_limit_pre < memc_logo_to_demura_drop_limit)
			memc_logo_to_demura_drop_limit_pre++;
		else
			memc_logo_to_demura_drop_limit_pre = memc_logo_to_demura_drop_limit;
		memc_logo_to_demura_drop_limit_CNT = 0;
	}
	memc_logo_to_demura_drop_limit_CNT++;
	
	drop_limit = memc_logo_to_demura_drop_limit_pre; // test
	drop_limit = (drop_limit * memc_logo_to_demura_drop_limit_gain)>>10;

	//drop_limit_sp = (memc_logo_to_demura_drop_limit_pre * memc_logo_to_demura_drop_limit_gain_SP)>>10;
	if (circle4_flag != 0) {
		sp_gain_blending = ((sp_gain_blending+1)>sp_gain_blending_max)?(sp_gain_blending_max):(sp_gain_blending+1);
	} else {
		sp_gain_blending = ((sp_gain_blending-1)<0)?(0):(sp_gain_blending-1);
	}
	drop_limit_sp_temp = (memc_logo_to_demura_drop_limit_pre * memc_logo_to_demura_drop_limit_gain_SP)>>10;
	drop_limit_sp = ((sp_gain_blending*drop_limit_sp_temp) + ((sp_gain_blending_max-sp_gain_blending)*drop_limit))>>5;

    reg_color_sld_sld_cnt_0_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_CNT_0_reg);
	color_sld_sld_control_reg.regValue = IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg);
#ifdef _VIP_Mer7_Compile_Error__
	debug_log_time = reg_color_sld_sld_cnt_0_reg.dummy1802c71c_31_17&1;
	debug_no_interrupt = (reg_color_sld_sld_cnt_0_reg.dummy1802c71c_31_17&2)?1:0;
#endif
	debug_log_time = color_sld_sld_control_reg.dummy1802c700_25_22&1;
	debug_no_interrupt = (color_sld_sld_control_reg.dummy1802c700_25_22&2)?1:0;
	if(DeMura_TBL.table_mode==DeMura_TBL_481x271)
	{
		ci0=270;
		ci1=271;
		cj0=480;
		cj1=481;
		
		circle_apply_x0 = logo_circle4_x_sta; 
		circle_apply_x1 = logo_circle4_x_end; 
		circle_apply_y0 = logo_circle4_y_sta; 
		circle_apply_y1 = logo_circle4_y_end; 

		memcpy(sld_work_filter.filter_w,sld_work_filter_10,sizeof(sld_work_filter_10));
		sld_work_filter.filter_gain = 1096;
	}
	else
	// DeMura_TBL_241x136
	{
		ci0=135;
		ci1=136;
		cj0=240;
		cj1=241;

		SLD_BLK_h = 120;
		SLD_BLK_v = 68;

		circle_apply_x0 = logo_circle4_apply_x_sta_271x136; 
		circle_apply_x1 = logo_circle4_apply_x_end_271x136; 
		circle_apply_y0 = logo_circle4_apply_y_sta_271x136; 
		circle_apply_y1 = logo_circle4_apply_y_end_271x136; 
		
		memcpy(sld_work_filter.filter_w,sld_work_filter_5,sizeof(sld_work_filter_5));
		sld_work_filter.filter_gain = 1096;// 3288;
	}
	
	HPhase = 1048576*HBlks/cj0;
	VPhase = 1048576*VBlks/ci0;
	
	if(debug_no_interrupt)
	{
		local_irq_save(flags);
	}
	if(debug_log_time)
	{
		rtd_pr_vpq_emerg("memc_logo_to_demura_apply start\n");
	}
	// rtd_pr_vpq_emerg("memc_logo_to_demura_apply, memc_logo_to_demura_APL_max_th=%d,\n", memc_logo_to_demura_APL_max_th);
	// rtd_pr_vpq_emerg("memc_logo_to_demura_apply, drop_time=%d,\n", drop_time);
	// rtd_pr_vpq_emerg("memc_logo_to_demura_apply, drop_limit=%d,\n", drop_limit);
	// rtd_pr_vpq_emerg("memc_logo_to_demura_apply, wait_time=%d,\n", wait_time);

    //update_cnt = (memc_logo_to_demura_update_cnt <= 0) ? 1 : memc_logo_to_demura_update_cnt;
    wait_time = (memc_logo_to_demura_wait_time * frm_rate) / 10;
    if (wait_time <= 0) {
        wait_time = 1;
    }
    drop_time = (memc_logo_to_demura_drop_time * frm_rate) / 10;
    if (drop_time <= 0) {
        drop_time = 1;
    }

	coutner_procees_time_max = wait_time + drop_time + drop_buffer;

	//rtd_pr_vpq_emerg("memc_logo_to_demura_apply, wait_time=%d,\n", wait_time);
	//rtd_pr_vpq_emerg("memc_logo_to_demura_apply, drop_time=%d,\n", drop_time);
	//rtd_pr_vpq_emerg("memc_logo_to_demura_apply, drop_limit=%d,\n", drop_limit);
	//rtd_pr_vpq_emerg("memc_logo_to_demura_apply, drop_buffer=%d,\n", drop_buffer);
	if (sld_work.read_ok) {
		
        //time_start = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
        if (ONOFFwithHWSLD == 1) {
            offbyhwsld = COLOR_SLD_SLD_CONTROL_get_sld_blend_en(IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg));
        }
        else {
            offbyhwsld = 1;
        }

        if ((LSC_by_memc_logo_en == 0) && (SLD_BY_MEMC_DEMURA_LOGO_ONLY == 0)) { // disable, demura table set to all 0, bypass all other flows
			for (i = 0; i <= 270; i++) {
                for (j = 0; j <= 480; j++) {
                    demura_tbl[0][i * 481 + j] = 0;
                    demura_tbl[1][i * 481 + j] = 0;
                    demura_tbl[2][i * 481 + j] = 0;
                }
            }

#ifdef CONFIG_SCALER_ENABLE_V4L2
        } else if (v4l2_vpq_ire != V4L2_EXT_VPQ_INNER_PATTERN_DISABLE) {
#else
		} else if(0){
#endif		
            for (i = 0; i <= 270; i++) {
                for (j = 0; j <= 480; j++) {
                    demura_tbl[0][i * 481 + j] = 0;
                    demura_tbl[1][i * 481 + j] = 0;
                    demura_tbl[2][i * 481 + j] = 0;
					logo_demura_counter[i * 481 + j] = 0;
                }
            }
			circle4_cnt = 0;
			circle4_flag = 0;
			LR_LOGO_Flag_L = 0;
			LR_LOGO_Flag_R = 0;
		} else {
			if(DeMura_TBL.table_mode==DeMura_TBL_481x271)
			{
				// from raw memc logo map to 481x271 table
				for (i = 0; i < 270; i++) {
					il = i * cj1;
					for (j = 0; j < 960 / 8; j++) {
						unsigned char logo_byte1 = logo_flag_map_raw[(i * 2) * 192 + j];
						unsigned char logo_byte2 = logo_flag_map_raw[(i * 2 + 1) * 192 + j];
						unsigned char logo_bit[2][8] = {0};
						unsigned char logo_flag_block;

						if (!memc_logo_read_en) {
							logo_byte1 = 0;
							logo_byte2 = 0;
						}

						for (k = 0; k < 8; k++) {
							logo_bit[0][k] = (logo_byte1 & (1 << (7 - k))) ? 1 : 0;
							logo_bit[1][k] = (logo_byte2 & (1 << (7 - k))) ? 1 : 0;
							//logo_flag_map[i*960 + j + k] = ( (logo_byte & ( 1<<(7-k) )) != 0 );
						}
						for (k = 0; k < 4; k++) {
							logo_flag_block = logo_bit[0][k * 2] || logo_bit[0][k * 2 + 1] || logo_bit[1][k * 2] || logo_bit[1][k * 2 + 1];
							logo_flag_map[il + j * 4 + k] = logo_flag_block;
						}
					}
					logo_flag_map[il + cj0] = logo_flag_map[il + (cj0-1)]; // last point repeat
				}
				for (j = 0; j <= cj0; j++) { // last line repeat
					logo_flag_map[ci0 * cj1 + j] = logo_flag_map[(ci0-1) * cj1 + j];
				}
			}
			else
			{
				// from raw memc logo map to 241x136 table
				for( i=0; i<ci0; i++ )
				{
					for( j=0; j<120; j++ )
					{
						unsigned char logo_byte0 = logo_flag_map_raw[(i*4  )*192 + j];
						unsigned char logo_byte1 = logo_flag_map_raw[(i*4+1)*192 + j];
						unsigned char logo_byte2 = logo_flag_map_raw[(i*4+2)*192 + j];
						unsigned char logo_byte3 = logo_flag_map_raw[(i*4+3)*192 + j];
						unsigned char logo_bit[4][2] = {0};
						unsigned char logo_flag_block;

						if( !memc_logo_read_en )
						{
							logo_byte0 = 0;
							logo_byte1 = 0;
							logo_byte2 = 0;
							logo_byte3 = 0;
						}

						logo_bit[0][0] = (logo_byte0 & 15) ? 1 : 0;
						logo_bit[1][0] = (logo_byte1 & 15) ? 1 : 0;
						logo_bit[2][0] = (logo_byte2 & 15) ? 1 : 0;
						logo_bit[3][0] = (logo_byte3 & 15) ? 1 : 0;
						logo_bit[0][1] = (logo_byte0 & 240) ? 1 : 0;
						logo_bit[1][1] = (logo_byte1 & 240) ? 1 : 0;
						logo_bit[2][1] = (logo_byte2 & 240) ? 1 : 0;
						logo_bit[3][1] = (logo_byte3 & 240) ? 1 : 0;
						//logo_flag_map[i*960 + j + k] = ( (logo_byte & ( 1<<(7-k) )) != 0 );
						for( k=0; k<2; k++ )
						{
							logo_flag_block = logo_bit[0][k] || logo_bit[1][k] || logo_bit[2][k] || logo_bit[3][k];
							logo_flag_map[i*cj1 + j*2 + k] = logo_flag_block;
						}
					}
					logo_flag_map[i*cj1 + cj0] = logo_flag_map[i*cj1 + (cj0-1)]; // last point repeat
				}
				for( j=0; j<=cj0; j++ ) // last line repeat
				{
					logo_flag_map[ci0*cj1 + j] = logo_flag_map[(ci0-1)*cj1 + j];
				}
			}

			memcpy(&logo_flag_map_no_fill_hole[0], &logo_flag_map[0], sizeof(logo_flag_map));
			
			if(debug_log_time)
			{
				rtd_pr_vpq_emerg("memc_logo_to_demura_apply b1\n");
			}
            if (SLD_BY_MEMC_DEMURA_LOGO_ONLY == 0) {
                // memc logo map fill hole
                if (memc_logo_fill_hole_en) {
                    //memcpy( logo_flag_map_buf, logo_flag_map, 481*271 );
                    // logo_flag_map_buf: area selection
                    // 0: background, 1: logo object, 2~: hole #
                    int area_idx = 2;

                    memset(is_background, 0, 10000);
                    memset(eq_table, 0, 10000 * 2 * sizeof(unsigned short));
                    memset(eq_searched, 0, 10000);
                    is_background[0] = 1;

                    // 1st pass: mark all 0 areas
                    for (i = 0; i <= ci0; i++) {
						il = i * cj1;
                        for (j = 0; j <= cj0; j++) {
							ilj = il + j;
                            if (!logo_flag_map[ilj]) { // not logo object
                                if (i == 0 || j == 0) { // up/left border -> background
                                    logo_flag_map_buf[ilj] = 0;
                                }
                                else if (i == ci0 || j == cj0) { // down/right border -> background + mark background
                                    logo_flag_map_buf[ilj] = 0;
                                    if (!logo_flag_map[ilj - cj1] &&  logo_flag_map_buf[ilj - cj1] != 0) { // up
                                        is_background[ logo_flag_map_buf[ilj - cj1] ] = 1;
                                    }
                                    if (!logo_flag_map[ilj - 1] &&  logo_flag_map_buf[ilj - 1] != 0) { // left
                                        is_background[ logo_flag_map_buf[ilj - 1] ] = 1;
                                    }
                                }
                                else {
                                    // find neighbor color (4 dir)
                                    if (!logo_flag_map[ilj - cj1] && !logo_flag_map[ilj - 1]) { // up & left
                                        if (logo_flag_map_buf[ilj - cj1] != logo_flag_map_buf[ilj - 1]) { // different color, add to eq table
                                            if (logo_flag_map_buf[ilj - cj1] == 0) { // up is background
                                                logo_flag_map_buf[ilj] = logo_flag_map_buf[ilj - cj1]; // use up
                                                is_background[ logo_flag_map_buf[ilj - 1] ] = 1;
                                            }
                                            else if (logo_flag_map_buf[ilj - 1] == 0) { // left is background
                                                logo_flag_map_buf[ilj] = logo_flag_map_buf[ilj - 1]; // use left
                                                is_background[ logo_flag_map_buf[ilj - cj1] ] = 1;
                                            }
                                            else {
                                                logo_flag_map_buf[ilj] = logo_flag_map_buf[ilj - cj1]; // use up
                                                eq_table_insert(logo_flag_map_buf[ilj - cj1], logo_flag_map_buf[ilj - 1]);
                                            }
                                        }
                                        else {
                                            logo_flag_map_buf[ilj] = logo_flag_map_buf[ilj - 1];    // use left
                                        }
                                    }
                                    else if (!logo_flag_map[ilj - cj1]) { // up only
                                        logo_flag_map_buf[ilj] = logo_flag_map_buf[ilj - cj1];    // use up
                                    }
                                    else if (!logo_flag_map[ilj - 1]) { // left only
                                        logo_flag_map_buf[ilj] = logo_flag_map_buf[ilj - 1];    // use left
                                    }
                                    else { // new color index
                                        if (area_idx >= 10000) { // not support more than 10000 new regions
                                            logo_flag_map_buf[ilj] = 0;										
                                        }
                                        else {
                                            logo_flag_map_buf[ilj] = area_idx;
                                            area_idx++;
                                        }
                                    }
                                }
                            }
                            else {
                                logo_flag_map_buf[ilj] = 1;    // logo object
                            }
                        }
                    }
					if(debug_log_time)
					{
						rtd_pr_vpq_emerg("memc_logo_to_demura_apply f1\n");
					}
                    // check eq table for all background
                    for (i = 2; i < area_idx; i++) {
                        if (is_background[i] && !eq_searched[i]) {
                            // trace list
                            int prev = eq_table[i][0];
                            int next = eq_table[i][1];

                            while (prev != 0) {
                                is_background[ prev ] = 1;
                                eq_searched[ prev ] = 1;
                                prev = eq_table[prev][0];
                            }
                            while (next != 0) {
                                is_background[ next ] = 1;
                                eq_searched[ next ] = 1;
                                next = eq_table[next][1];
                            }
                        }
                    }

                    // 2nd pass: fill areas that are not background
                    for (i = 0; i <= ci0; i++) {
						il = i * cj1;
                        for (j = 0; j <= cj0; j++) {
							ilj = il + j;
                            if (!logo_flag_map[ilj] && !is_background[ logo_flag_map_buf[ilj] ]) {
                                logo_flag_map[ilj] = 1; // fill hole
                            }
                        }
                    }
					if(debug_log_time)
					{
						rtd_pr_vpq_emerg("memc_logo_to_demura_apply f2\n");
					}
					for (i = 1; i <= ci0-1; i++) {
						il = i * cj1;
                        for (j = 1; j <= cj0-1; j++) {
							ilj = il + j;
							cnt_temp = 0;
							if (logo_flag_map[ilj] == 0) {
								cnt_temp += logo_flag_map[(i-1) * cj1 + j-1];
								cnt_temp += logo_flag_map[(i-1) * cj1 + j];
								cnt_temp += logo_flag_map[(i-1) * cj1 + j+1];

								cnt_temp += logo_flag_map[ilj-1];
								//cnt_temp += logo_flag_map[ilj];
								cnt_temp += logo_flag_map[ilj+1];

								cnt_temp += logo_flag_map[(i+1) * cj1 + j-1];
								cnt_temp += logo_flag_map[(i+1) * cj1 + j];
								cnt_temp += logo_flag_map[(i+1) * cj1 + j+1];
								if (cnt_temp >= fill_hole_th)
									logo_flag_map[ilj] = 1;
							}
						}
					}
				} // end memc_logo_fill_hole_en
				if(debug_log_time)
				{
					rtd_pr_vpq_emerg("memc_logo_to_demura_apply b2\n");
				}
				
                // sp check
                //circle4_flag = DeMura_TBL.table_mode==DeMura_TBL_481x271?memc_logo_to_demura_4circle_detect():0;
           /*
#if 0  
			circle4_flag = memc_logo_to_demura_4circle_detect(DeMura_TBL.table_mode);
#else
			if (memc_logo_to_demura_4circle_detect(DeMura_TBL.table_mode, sp_gain_blending) == 1)	// debounce
				circle4_flag = ((circle4_flag+1)>=ID_4cir_deBounce_th)?(ID_4cir_deBounce_th):(circle4_flag+1);
			else
				circle4_flag = ((circle4_flag-1)<0)?(0):(circle4_flag-1);
#endif
			if (circlr4_status_pre != circle4_flag)	{
				rtd_printk(KERN_EMERG,"sw_sld", "sp_status=%d,\n", circle4_flag);
				circlr4_status_pre = circle4_flag;
			}
			*/

			//memc_logo_to_RL_LOGO_detect();
				if(debug_log_time)
				{
					rtd_pr_vpq_emerg("memc_logo_to_demura_apply b3\n");
				}
                // LD block average interpolation
#if 0                
                printk("sw_sld,  APL en=%d, mode=%d,,block size=%d,,\n", 
                	memc_logo_LD_APL_check, DeMura_TBL.table_mode, DeMura_TBL.DeMura_CTRL_TBL.demura_block_size);
#endif				
                if (memc_logo_LD_APL_check != 0) {
					if (sld_work.LD_APL_valid  && memc_logo_LD_APL_check == 1) {
	                    short blk_x_l, blk_y_u;
	                    short blk_x_r, blk_y_d;
	                    short x, y;
	                    unsigned int apl00, apl01, apl10, apl11, intp0, intp1;
	                    int max_blk_avg = 0;

	                    // spatial max filter on block APL
	                    for (i = 0; i < 18; i++) {
	                        for (j = 0; j < 32; j++) {
	                            max_blk_avg = 0;
	                            for (x = -1; x <= 1; x++) {
	                                for (y = -1; y <= 1; y++) {
	                                    blk_x_l = j + x;
	                                    blk_x_l = MIN(blk_x_l, 31);
	                                    blk_x_l = MAX(blk_x_l, 0); // comfirm blk_x_l between 0 and 31

	                                    blk_y_u = i + y;
	                                    blk_y_u = MIN(blk_y_u, 17);
	                                    blk_y_u = MAX(blk_y_u, 0); // comfirm blk_y_u between 0 and 17

	                                    if (blk_apl_average[ blk_y_u * 32 + blk_x_l ] > max_blk_avg) {
	                                        max_blk_avg = blk_apl_average[ blk_y_u * 32 + blk_x_l ];
	                                    }
	                                }
	                            }

	                            blk_apl_maxfilter[ i * 32 + j ] = max_blk_avg;
	                        }
	                    }

	                    if(DeMura_TBL.table_mode==DeMura_TBL_481x271)
	                    {
							for (i = 0; i <= ci0; i++) {
								il = i * cj1;
								for (j = 0; j <= cj0; j++) {
									blk_x_l = (j - 7) / 15; // in which block(0~31)
									blk_x_l = MIN(blk_x_l, 31);
									blk_x_l = MAX(blk_x_l, 0);

									blk_x_r = (j + 7) / 15; // in which block(0~31)
									blk_x_r = MIN(blk_x_r, 31);
									blk_x_r = MAX(blk_x_r, 0);

									blk_y_u = (i - 7) / 15; // in which block(0~31)
									blk_y_u = MIN(blk_y_u, 17);
									blk_y_u = MAX(blk_y_u, 0);

									blk_y_d = (i + 7) / 15; // in which block(0~31)
									blk_y_d = MIN(blk_y_d, 17);
									blk_y_d = MAX(blk_y_d, 0);

									x = (j - 7) % 15; // distance in block(0~14)
									y = (i - 7) % 15; // distance in block(0~14)

									apl00 = blk_apl_maxfilter[ blk_y_u * 32 + blk_x_l ];
									apl01 = blk_apl_maxfilter[ blk_y_u * 32 + blk_x_r ];
									apl10 = blk_apl_maxfilter[ blk_y_d * 32 + blk_x_l ];
									apl11 = blk_apl_maxfilter[ blk_y_d * 32 + blk_x_r ];

									intp0 = (apl00 * (15 - x) + apl01 * x + 7) / 15;
									intp1 = (apl10 * (15 - x) + apl11 * x + 7) / 15;

									blk_apl_interp[il + j] = (intp0 * (15 - y) + intp1 * y + 7) / 15;

								}
							}
	                    }
						else
	                    {
							for (i = 0; i <= ci0; i++) {
								il = i * cj1;
								for (j = 0; j <= cj0; j++) {
									blk_x_l = (j*2 - 7) / 15;
									blk_x_l = MIN(blk_x_l, 31);
									blk_x_l = MAX(blk_x_l, 0);

									blk_x_r = (j*2 + 7) / 15;
									blk_x_r = MIN(blk_x_r, 31);
									blk_x_r = MAX(blk_x_r, 0);

									blk_y_u = (i*2 - 7) / 15;
									blk_y_u = MIN(blk_y_u, 17);
									blk_y_u = MAX(blk_y_u, 0);

									blk_y_d = (i*2 + 7) / 15;
									blk_y_d = MIN(blk_y_d, 17);
									blk_y_d = MAX(blk_y_d, 0);

									x = (j*2 - 7) % 15;
									y = (i*2 - 7) % 15;

									apl00 = blk_apl_maxfilter[ blk_y_u * 32 + blk_x_l ];
									apl01 = blk_apl_maxfilter[ blk_y_u * 32 + blk_x_r ];
									apl10 = blk_apl_maxfilter[ blk_y_d * 32 + blk_x_l ];
									apl11 = blk_apl_maxfilter[ blk_y_d * 32 + blk_x_r ];

									intp0 = (apl00 * (15 - x) + apl01 * x + 7) / 15;
									intp1 = (apl10 * (15 - x) + apl11 * x + 7) / 15;

									blk_apl_interp[il + j] = (intp0 * (15 - y) + intp1 * y + 7) / 15;

								}
							}
	                    }
					}
					else if (memc_logo_LD_APL_check == 2 && DeMura_TBL.table_mode==DeMura_TBL_241x136) { // use SLD APL
						// only support "DeMura_TBL_241x136", implement SLD APL for DeMura_TBL_481x271 future?
						//drvif_fwif_color_Get_SLD_APL();
						// h and v repeat
						for (i = 0; i <= ci0; i++) {
							il = i * cj1;
							for (j = 0; j <= cj0; j++) {
								sld_blk_i = ((i>>1) >= SLD_BLK_v)?(SLD_BLK_v-1):(i>>1);
								sld_blk_j = ((j>>1) >= SLD_BLK_h)?(SLD_BLK_h-1):(j>>1);
								SLD_BLK_idx = sld_blk_i*SLD_BLK_h + sld_blk_j;
								blk_apl_SLD[il + j] = SLD_BLK_APL[SLD_BLK_idx];
#if 0								
								if (i == 13 && j == 220)
									printk("sw_sld,  blk_apl_interp[220:13]=%d,\n", blk_apl_interp[il + j]);
#endif
							}
						}		
						if (memc_logo_filter_SLD_APL_SE_en == 0) {	// use fixed average filter
							for (i = 0; i < ci1; i++) {
								il = i * cj1;
								filter_sum_APL = 0;
								for (idx_x = 0;idx_x<demura_filter_size;idx_x++) {
									dmr_x = dmr_xx[idx_x];
									filter_2D_sum_col[idx_x] = 0;
									for (idx_y = 0;idx_y<demura_filter_size;idx_y++) {
										dmr_y = i - demura_filter_size_half + idx_y;
										dmr_yy[idx_y] = (dmr_y<0)?(0):((dmr_y>=ci1)?(ci0):(dmr_y));									
										filter_2D_sum_col[idx_x] += blk_apl_SLD[dmr_yy[idx_y]*cj1+dmr_x];
									}
									filter_sum_APL += filter_2D_sum_col[idx_x];
								}
								filter_2d_remove = 0;
								
								for (j = 0; j < cj1; j++) {
									ilj = il + j;
									blk_apl_interp[ilj] = (filter_sum_APL * 809)>>16;
									//demura_tbl[1][ilj] = (demura_tbl[1][ilj]*sld_work_filter.filter_gain)>>10;
									
									// compute for next
									filter_sum_APL = filter_sum_APL - filter_2D_sum_col[filter_2d_remove];
									filter_2D_sum_col[filter_2d_remove] = 0;
									dmr_x = j + demura_filter_size_half + 1;
									dmr_x = (dmr_x>cj0)?(cj0):(dmr_x);		

									for (idx_y = 0;idx_y<demura_filter_size;idx_y++)	
										filter_2D_sum_col[filter_2d_remove] += blk_apl_SLD[dmr_yy[idx_y]*cj1+dmr_x];

									filter_sum_APL += filter_2D_sum_col[filter_2d_remove];
									
									filter_2d_remove=((filter_2d_remove+1)>=demura_filter_size)?(0):(filter_2d_remove+1);
								}
							}
						} 
						else if(memc_logo_filter_SLD_APL_SE_en >= 1 && memc_logo_filter_SLD_APL_SE_en <= 100) {
							memc_logo_to_demura_Filter_241x136_SLD_APL_By_SE(memc_logo_filter_SLD_APL_SE_en);
						}
					}
					else if (memc_logo_LD_APL_check == 3 && DeMura_TBL.table_mode==DeMura_TBL_241x136) {
						// interpoaltion from pApl(GSR APL, fix to 60x34) to 240x135 dimension
						for(i=0; i<ci0; i++)
						{
							il = i * cj1;
							for(j=0; j<cj0; j++)
							{
								HPos = j*HPhase + (128<<12);
								VPos = i*VPhase + (128<<12);

								HBldCoef = (HPos>>12) & 0xff;
								VBldCoef = (VPos>>12) & 0xff;

								APLBlkH1 = SLD_CLIP(0, HBlks-1, (HPos>>20)-1 ); //SLD_CLIP: return median value
								APLBlkH2 = SLD_CLIP(0, HBlks-1, (HPos>>20)   );
								APLBlkV1 = SLD_CLIP(0, VBlks-1, (VPos>>20)-1 );
								APLBlkV2 = SLD_CLIP(0, VBlks-1, (VPos>>20)   );

								APLBlk1 = LC_APL[APLBlkV1*HBlks+APLBlkH1];
								APLBlk2 = LC_APL[APLBlkV1*HBlks+APLBlkH2];
								APLBlk3 = LC_APL[APLBlkV2*HBlks+APLBlkH1];
								APLBlk4 = LC_APL[APLBlkV2*HBlks+APLBlkH2];

								HBldVal1 = HBldCoef*APLBlk2 + (256-HBldCoef)*APLBlk1; // topLeft and topRight
								HBldVal2 = HBldCoef*APLBlk4 + (256-HBldCoef)*APLBlk3; // botLeft and botRight
								blk_apl_interp[il + j] = ((HBldVal2*VBldCoef + (256-VBldCoef)*HBldVal1) >> 16) >> 1; // 11 bit to 10 bit

							}	
						}

					}
					else {
	                    for (i = 0; i <= ci0; i++) {
							il = i * cj1;
	                        for (j = 0; j <= cj0; j++) {
	                            blk_apl_interp[il + j] = 0;
	                        }
	                    }
                	}
                }
                else {
                    for (i = 0; i <= ci0; i++) {
						il = i * cj1;
                        for (j = 0; j <= cj0; j++) {
                            blk_apl_interp[il + j] = 0;
                        }
                    }
                } // end of memc_logo_LD_APL_check
				if(debug_log_time)
				{
					rtd_pr_vpq_emerg("memc_logo_to_demura_apply b4\n");
				}

				// Update counter & calculate gain(LD, MEMC logo map)
                for (i = 0; i <= ci0; i++) {
					il = i * cj1;
                    for (j = 0; j <= cj0; j++) {
                        unsigned int LD_APL;
						ilj = il + j;
                        LD_APL = blk_apl_interp[ilj];

                        if (sld_work.do_reset_full) {
                            logo_demura_counter[ilj] = 0;
                        }
                        else if (sld_work.do_counter_update) {
                            if (logo_flag_map[ilj]) {
                                logo_demura_counter[ilj] += memc_logo_to_demura_update_cnt;
                                if (logo_demura_counter[ilj] >= coutner_procees_time_max) {
                                    logo_demura_counter[ilj] = coutner_procees_time_max;
                                }
                            }
                            else {
                                if (sld_work.do_reset_counter) {
									if (SLD_ABS(blk_apl_interp_pre[ilj], blk_apl_interp[ilj]) > SLD_APL_reset_cnt_th)
                                    	logo_demura_counter[ilj] = 0; 
									else
										logo_demura_counter[ilj] -= memc_logo_to_demura_ascend_speed; 
	                                    if (logo_demura_counter[ilj] < 0) {
	                                        logo_demura_counter[ilj] = 0;
	                                    }	
                                }
                                else {
                                    logo_demura_counter[ilj] -= memc_logo_to_demura_ascend_speed;
                                    if (logo_demura_counter[ilj] < 0) {
                                        logo_demura_counter[ilj] = 0;
                                    }
                                }
                            }
                        }
						// update pre
						//blk_apl_interp_pre[ilj] = blk_apl_interp[ilj];	// move to bellow, for debug show


						// Gain Calculation(Gain table)
                        if (logo_demura_counter[ilj] < wait_time) {
                            drop_block = 0;
                        }
                        else if (logo_demura_counter[ilj] > wait_time + drop_time) {
                            drop_block = drop_limit;
                        }
                        else {
                            drop_block = drop_limit * (logo_demura_counter[ilj] - wait_time) / drop_time;
                        }
						// circle4 logo 
                        if ((circle4_flag != 0) && (j >= circle_apply_x0) && (j <= circle_apply_x1)
                                && (i >= circle_apply_y0) && (i <= circle_apply_y1)) {
							//if (circle4_flag != pre_circle4_flag) {
							//logo_demura_counter[i*cj1 + j] = 0;
							//drop_block = 0;
							//}
#if 1
							if (logo_demura_counter[ilj] < wait_time) {
								drop_block = 0;
							}
							else if (logo_demura_counter[ilj] > wait_time + drop_time) {
								drop_block = drop_limit_sp;
							}
							else {
								drop_block = drop_limit_sp * (logo_demura_counter[ilj] - wait_time) / drop_time;
							}	

#else
							if (logo_flag_map[ilj] == 1) {
								logo_demura_counter[ilj] = circle4_cnt;
							}
							else {
								logo_demura_counter[ilj] = 0;
							}
							
							if (logo_demura_counter[ilj] < wait_time) {
								drop_block = 0;
							}
							else if (logo_demura_counter[ilj] > wait_time + drop_time) {
								drop_block = drop_limit_sp;
								if (logo_demura_counter[ilj] >= wait_time + drop_time + drop_buffer) {
									logo_demura_counter[ilj] = wait_time + drop_time + drop_buffer;
								}
							}
							else {
								//drop_block = drop_limit * (logo_demura_counter[ilj] - wait_time) / drop_time;
								drop_block = drop_limit_sp * (logo_demura_counter[ilj] - wait_time) / drop_time;
							}
#endif							
                        }
                        else {							
                            if (LD_APL > memc_logo_to_demura_APL_max_th - drop_block) { // decrease counter on too bright part to avoid luminance inversion
#if 0	// drop level is by LD APL 1040 1 102 1
                                drop_block = memc_logo_to_demura_APL_max_th - LD_APL;
                                if (drop_block < 0) {
                                    drop_block = 0;
                                }
                                logo_demura_counter[ilj] -= memc_logo_to_demura_ascend_speed;
                                if (drop_limit <= 0 && logo_demura_counter[ilj] < 0) {
                                    logo_demura_counter[ilj] = 0;
                                }
                                else if (logo_demura_counter[ilj] < (drop_block * drop_time / drop_limit + wait_time)) {
                                    logo_demura_counter[ilj] = (drop_block * drop_time / drop_limit + wait_time);
                                }
#else	// fast return and slowly add	
								drop_block_APL = memc_logo_to_demura_APL_max_th - LD_APL;
								if (drop_block_APL < 0) {
									drop_block_APL = 0;
								}
								demura_cnt_APL = drop_block_APL * drop_time / drop_limit + wait_time; 
								if (logo_demura_counter[ilj] > demura_cnt_APL) {
									logo_demura_counter[ilj] = demura_cnt_APL;
									drop_block = drop_block_APL;
								}
#endif	
                            }	
                        }

                        if ((j >= sld_work.osd_sta_blkx) && (j <= sld_work.osd_end_blkx) && (i >= sld_work.osd_sta_blky) && (i <= sld_work.osd_end_blky)) {
                            // osd part, bypass
                            demura_tbl[0][ilj] = 0;
                            demura_tbl[1][ilj] = 0;
                            demura_tbl[2][ilj] = 0;
                        }
                        else {
                            demura_tbl[0][ilj] = 0;
                            demura_tbl[1][ilj] = 0 - (drop_block >> 2);
                            demura_tbl[2][ilj] = 0 - (drop_block >> 2);
                        }
                    }
                }
				
				if(debug_log_time)
				{
					rtd_pr_vpq_emerg("memc_logo_to_demura_apply b5\n");
				}
				
                // demura table filter
                if (memc_logo_filter_en != 0xff) {
	                if (memc_logo_filter_en == 0 || memc_logo_filter_en == 254) {
	                    // horizontal [1 2 1] filter
	                    int filter_sum = 0;
	                    /*for (i = 0; i <= ci0; i++) {
	                        for (j = 0; j <= cj0; j++) {
	                            int j_left = (j == 0) ? 0 : j - 1;
	                            int j_right = (j == cj0) ? cj0 : j + 1;
	                            filter_sum = (demura_tbl[2][i * cj1 + j_left] + demura_tbl[2][i * cj1 + j] * 2 + demura_tbl[2][i * cj1 + j_right]) >> 2;

	                            demura_tbl[1][i * cj1 + j] = filter_sum;
	                        }
	                    }
	                    memcpy(demura_tbl[2], demura_tbl[1], cj1 * 271 * sizeof(short));

	                    // vertical [1 2 1] filter
	                    for (i = 0; i <= ci0; i++) {
	                        for (j = 0; j <= cj0; j++) {
	                            int i_up = (i == 0) ? 0 : i - 1;
	                            int i_down = (i == ci0) ? ci0 : i + 1;
	                            filter_sum = (demura_tbl[2][i_up * cj1 + j] + demura_tbl[2][i * cj1 + j] * 2 + demura_tbl[2][i_down * cj1 + j]) >> 2;

	                            demura_tbl[1][i * cj1 + j] = filter_sum;
	                        }
	                    }*/
		 				if (memc_logo_filter_en == 254) {         // weighting 9x9 filter, but need 300ms to calculate at least
							for (i = 0; i < ci1; i++) {
								il = i * cj1;
								for (j = 0; j < cj1; j++) {
									filter_sum = 0;
									for (idx_y = 0; idx_y < demura_filter_size; idx_y++) {
										for (idx_x = 0; idx_x< demura_filter_size; idx_x++) {
											dmr_x = j - (demura_filter_size>>1) + idx_x;
											dmr_x = (dmr_x<0)?(0):((dmr_x>=cj1)?(cj0):(dmr_x));
											dmr_y = i - (demura_filter_size>>1) + idx_y;
											dmr_y = (dmr_y<0)?(0):((dmr_y>=ci1)?(ci0):(dmr_y));									
											filter_sum += sld_work_filter.filter_w[idx_y*demura_filter_size+idx_x]*demura_tbl[2][dmr_y*cj1+dmr_x];
										}
									}
									//filter gain =1 if val =1024, filter_shift = 8 if sum of weight is 256.
									demura_tbl[1][il + j] = (filter_sum*sld_work_filter.filter_gain)>>(sld_work_filter.filter_shift+10); //??????????
								}
							}
						} else {	// use fixed average filter
							for (i = 0; i < ci1; i++) {
								il = i * cj1;
								filter_sum = 0;
								for (idx_x = 0; idx_x < demura_filter_size; idx_x++) {
									dmr_x = dmr_xx[idx_x];
									filter_2D_sum_col[idx_x] = 0;
									for (idx_y = 0; idx_y< demura_filter_size; idx_y++) {
										dmr_y = i - demura_filter_size_half + idx_y;
										dmr_yy[idx_y] = (dmr_y<0)?(0):((dmr_y>=ci1)?(ci0):(dmr_y));									
										filter_2D_sum_col[idx_x] += demura_tbl[2][dmr_yy[idx_y]*cj1+dmr_x];
									}
									filter_sum += filter_2D_sum_col[idx_x];
								}
								filter_2d_remove = 0;
								
								for (j = 0; j < cj1; j++) {
									ilj = il + j;
									demura_tbl[1][ilj] = (filter_sum * 809)>>16;
									demura_tbl[1][ilj] = (demura_tbl[1][ilj]*sld_work_filter.filter_gain)>>10;
									
									// compute for next
									filter_sum = filter_sum - filter_2D_sum_col[filter_2d_remove];
									filter_2D_sum_col[filter_2d_remove] = 0;
									dmr_x = j + demura_filter_size_half + 1;
									dmr_x = (dmr_x>cj0)?(cj0):(dmr_x);		

									for (idx_y = 0;idx_y<demura_filter_size;idx_y++)	
										filter_2D_sum_col[filter_2d_remove] += demura_tbl[2][dmr_yy[idx_y]*cj1+dmr_x];

									filter_sum += filter_2D_sum_col[filter_2d_remove];
									
									filter_2d_remove=((filter_2d_remove+1)>=demura_filter_size)?(0):(filter_2d_remove+1);
								}
							}
						}
	                    memcpy(demura_tbl[2], demura_tbl[1], 481 * 271 * sizeof(short));
					} 
					// SE filter for 241x136 Y mode
					else if ((memc_logo_filter_en >= 1) && (memc_logo_filter_en <= 200) && (DeMura_TBL.table_mode == DeMura_TBL_241x136)) {
						memc_logo_to_demura_Filter_241x136_YMode_By_SE(memc_logo_filter_en);
					}
            	}

            }
        }
		if(debug_log_time)
		{
			rtd_pr_vpq_emerg("memc_logo_to_demura_apply b6\n");
		}

		if (Gloabal_SLD_En == 1)
			drvif_fwif_SLD2GlobalSLD_Process();		

		if (SLD_BY_MEMC_DEMURA_LOGO_ONLY == 0) {
            // check the demura table value
            for (i = 0; i <= ci0; i++) {
				il = i * cj1;
                for (j = 1; j <= cj0; j++) {
                    int PtsDiff = 0;
					ilj = il + j;
                    PtsDiff = demura_tbl[2][ilj] - demura_tbl[2][ilj - 1];
                    PtsDiff = (PtsDiff >  31) ?  31 : PtsDiff;
                    PtsDiff = (PtsDiff < -32) ? -32 : PtsDiff;
                    demura_tbl[2][ilj] = demura_tbl[2][ilj - 1] + PtsDiff;
                }
            }
            memcpy(demura_tbl[1], demura_tbl[2], 481 * 271 * sizeof(short));
			if(debug_log_time)
			{
				rtd_pr_vpq_emerg("memc_logo_to_demura_apply b7\n");
			}
            // encode to demura table
            memc_logo_to_demura_debug_log(drop_block);
			
			// update SLD block APL pre
			memcpy(&blk_apl_interp_pre[0], &blk_apl_interp[0], sizeof(blk_apl_interp));
			
            if (logo_to_demura_en && !demura_write_en) {
                if (offbyhwsld == 0) {
                    memset(&demura_tbl[1][0], 0, sizeof(short) * 481 * 271 * 2);
                }

				if (DeMura_TBL.table_mode == DeMura_TBL_481x271) {
					fwif_color_DeMura_encode((short *)(&(demura_tbl[0][0])), DeMura_TBL.table_mode, 0, 3, &DeMura_TBL);
				} else {
					for (i=0;i<136;i++) {
						memcpy(&demura_apply[0][i*241], &demura_tbl[0][i*241], sizeof(short)*241);
						memcpy(&demura_apply[1][i*241], &demura_tbl[1][i*241], sizeof(short)*241);
						memcpy(&demura_apply[2][i*241], &demura_tbl[2][i*241], sizeof(short)*241);
					}
					fwif_color_DeMura_encode((short *)(&(demura_apply[0][0])), DeMura_TBL.table_mode, 0, 3, &DeMura_TBL);
				}

                demura_write_en = true;

            }
            if (circle4_flag != 0) {
                circle4_cnt += memc_logo_to_demura_update_cnt;
				if (circle4_cnt >= wait_time + drop_time + drop_buffer) {
					circle4_cnt = wait_time + drop_time + drop_buffer;
				}
            }
            else {
                circle4_cnt = wait_time>>2;
            }
        }
        else {
            demura_write_en = false;
        }

        sld_work.read_ok = false;

        //time_end = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
        //rtd_pr_vpq_emerg("memc_logo_to_demura_apply end: %d on 90k clock\n", (time_end-time_start));
		if(debug_log_time)
		{
			rtd_pr_vpq_emerg("memc_logo_to_demura_apply end\n");
		}

    }
	if(debug_no_interrupt)
	{
		local_irq_restore(flags);
	}
}// end memc_logo_to_demura_apply
#ifdef CONFIG_SCALER_ENABLE_V4L2
extern webos_strInfo_t webos_strToolOption;
#endif
void memc_logo_to_demura_init(void)
{
	if (drvif_color_get_HW_MEX_STATUS(MEX_SLD) == 1){
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
		MEMC_Pixel_LOGO_For_SW_SLD = 2; //1 
		// 0xff: no filter, 0:sw filter by cpu with ave filter(optimized), 1~200: SE filter, iteration times, 254:filter by cpu with normal filter
		//memc_logo_filter_en = 10; 
        INIT_WORK(&(sld_work.sld_apply_work), memc_logo_to_demura_apply);
    }
    else {
        SLD_SW_En = 0;
		MEMC_Pixel_LOGO_For_SW_SLD = 0;
    }


    rtd_pr_vpq_emerg("SLD_SW, ini done, SLD_SW_En=%d,\n", SLD_SW_En);
}

unsigned int drvif_fwif_SLD2GlobalSLD_Score_Cal(unsigned int cnt)
{
	unsigned int time_max, index, scale_x;
	unsigned int x0, x1, y0, y1;
	unsigned short temp;
	VIP_SLD2GlobalSLD_CTRL *SLD2GlobalSLD_CTRL;
	unsigned int gain;
	unsigned int ret_val;
	
	SLD2GlobalSLD_CTRL = drvif_fwif_color_get_SLD2GlobalSLD_CTRL();
	
	time_max = (coutner_procees_time_max==0)?(1):(coutner_procees_time_max);

	temp = cnt * 1024 / time_max;

	index = temp >> (10-VIP_SLD2GlobalSLD_CNT_Score_Num_Shift);
	scale_x = 1024 >> VIP_SLD2GlobalSLD_CNT_Score_Num_Shift;

	if (index >= VIP_SLD2GlobalSLD_CNT_Score_Num) {
		gain = SLD2GlobalSLD_CTRL->SLD_CNT_Score[VIP_SLD2GlobalSLD_CNT_Score_Num];
	} else {
		y0 = SLD2GlobalSLD_CTRL->SLD_CNT_Score[index];
		y1 = SLD2GlobalSLD_CTRL->SLD_CNT_Score[index+1];
		x0 = index * scale_x;
		x1 = index+1 * scale_x;
	 
		gain = y0 + (((temp - x0) * (y1 - y0))>>(10-VIP_SLD2GlobalSLD_CNT_Score_Num_Shift));
	}
	
	ret_val = (gain * temp)>>10;

	return ret_val;
}

unsigned int drvif_fwif_SLD2GlobalSLD_block_num_cal(unsigned int *row_num, unsigned int *col_num)
{
	if(DeMura_TBL.table_mode==DeMura_TBL_481x271)
	{
		*col_num=271;
		*row_num=481;
	} else {
		*col_num=136;
		*row_num=241;
	}
	
	return (*col_num)*(*row_num);
}

unsigned short SLD2GlobalSLD_Tonemap_curve_target[GSR2_IDX];
unsigned short SLD2GlobalSLD_gain_by_sat_target[GSR2_IDX];
unsigned char drvif_fwif_SLD2GlobalSLD_Curve_Smooth(VIP_SLD2GlobalSLD_CTRL *SLD2GlobalSLD_CTRL, DRV_GSR2_Table *SLD2GlobalSLD_Table)
{
	extern DRV_GSR2_Table GSR2_TBL[GSR2_Table_NUM];
	unsigned short smooth_step;
	unsigned int i;
	int temp, temp2;
	unsigned char update = 1;
	
	for (i=0;i<GSR2_IDX;i++) {
		temp = GSR2_TBL[0].GSR2_tonemap_curve.gsr2_tonemap_curve[i];
		temp2 = SLD2GlobalSLD_Tonemap_curve_target[i];
		smooth_step = SLD2GlobalSLD_CTRL->smooth_step*4;
		if ((temp+smooth_step) < temp2)
			GSR2_TBL[0].GSR2_tonemap_curve.gsr2_tonemap_curve[i] = GSR2_TBL[0].GSR2_tonemap_curve.gsr2_tonemap_curve[i]+smooth_step;
		else if ((temp-smooth_step) > temp2)
			GSR2_TBL[0].GSR2_tonemap_curve.gsr2_tonemap_curve[i] = GSR2_TBL[0].GSR2_tonemap_curve.gsr2_tonemap_curve[i]-smooth_step;
		else
			GSR2_TBL[0].GSR2_tonemap_curve.gsr2_tonemap_curve[i] = SLD2GlobalSLD_Tonemap_curve_target[i];

		temp = GSR2_TBL[0].GSR2_gain_by_sat.gsr2_gain_by_sat_curve[i];
		temp2 = SLD2GlobalSLD_gain_by_sat_target[i];
		smooth_step = SLD2GlobalSLD_CTRL->smooth_step;
		if ((temp+smooth_step) < temp2)
			GSR2_TBL[0].GSR2_gain_by_sat.gsr2_gain_by_sat_curve[i] = GSR2_TBL[0].GSR2_gain_by_sat.gsr2_gain_by_sat_curve[i]+smooth_step;
		else if ((temp-smooth_step) > temp2)
			GSR2_TBL[0].GSR2_gain_by_sat.gsr2_gain_by_sat_curve[i] = GSR2_TBL[0].GSR2_gain_by_sat.gsr2_gain_by_sat_curve[i]-smooth_step;
		else
			GSR2_TBL[0].GSR2_gain_by_sat.gsr2_gain_by_sat_curve[i] = SLD2GlobalSLD_gain_by_sat_target[i];

	}
	
	GSR2_TBL[0].GSR2_global_gain = SLD2GlobalSLD_Table->GSR2_global_gain;
	GSR2_TBL[0].GSR2_gain_by_sat.gsr2_sat_mode = SLD2GlobalSLD_Table->GSR2_gain_by_sat.gsr2_sat_mode;

	return update;
}

char drvif_fwif_SLD2GlobalSLD_Process(void)
{
	static unsigned int debug_cnt = 0;
	unsigned short i;
	unsigned int row_num, col_num, total_num, temp;
	VIP_SLD2GlobalSLD_CTRL *SLD2GlobalSLD_CTRL;
	VIP_SLD2GlobalSLD_INFO *SLD2GlobalSLD_INFO;
	DRV_GSR2_Table *SLD2GlobalSLD_Table;
	unsigned short linear_scale = 256; //	table is 12 bit?
	unsigned short debug_log, debug_log_delay;
	unsigned short logo_th;
	unsigned char driver_update_flag = 0;
	
	debug_cnt++;
	
	SLD2GlobalSLD_CTRL = drvif_fwif_color_get_SLD2GlobalSLD_CTRL();
	SLD2GlobalSLD_INFO = drvif_fwif_color_get_SLD2GlobalSLD_INFO();
	SLD2GlobalSLD_Table = drvif_fwif_color_get_SLD2GlobalSLD_TBL();

	debug_log = SLD2GlobalSLD_CTRL->Debug_Mode_Log;
	debug_log_delay = SLD2GlobalSLD_CTRL->Debug_Mode_Log_Delay;
	if (debug_log_delay == 0)
		debug_log_delay = 1;

	total_num = drvif_fwif_SLD2GlobalSLD_block_num_cal(&row_num, &col_num);

	temp = 0;
	for (i=0;i<total_num;i++) {
		temp = drvif_fwif_SLD2GlobalSLD_Score_Cal(logo_demura_counter[i]) + temp;	
	}
	// ave
	SLD2GlobalSLD_INFO->Blending_Score = temp / total_num;

	logo_th = (SLD2GlobalSLD_CTRL->Logo_TH==0)?(1):(SLD2GlobalSLD_CTRL->Logo_TH);
	SLD2GlobalSLD_INFO->Blending_Score_by_th = SLD2GlobalSLD_INFO->Blending_Score * 1024 / logo_th;
	if (SLD2GlobalSLD_INFO->Blending_Score_by_th>1024)
		SLD2GlobalSLD_INFO->Blending_Score_by_th = 1024;

	if (SLD2GlobalSLD_CTRL->Debug_Blending_Score_by_th != 0xFFFF)
		SLD2GlobalSLD_INFO->Blending_Score_by_th = SLD2GlobalSLD_CTRL->Debug_Blending_Score_by_th;

	// consider SLD gain(memc_logo_to_demura_drop_limit) from AP before blending
	// memc_logo_to_demura_drop_limit = 128 => gain = 1 
	SLD2GlobalSLD_INFO->Blending_Score_Final = (memc_logo_to_demura_drop_limit * SLD2GlobalSLD_INFO->Blending_Score_by_th)>>7;
	
	// do blending
	for (i=0;i<GSR2_IDX;i++) {
		SLD2GlobalSLD_Tonemap_curve_target[i] = ((1024 - SLD2GlobalSLD_INFO->Blending_Score_Final)*(i*linear_scale) + 
			SLD2GlobalSLD_INFO->Blending_Score_Final * SLD2GlobalSLD_Table->GSR2_tonemap_curve.gsr2_tonemap_curve[i])>>10;
		SLD2GlobalSLD_gain_by_sat_target[i] = ((1024 - SLD2GlobalSLD_INFO->Blending_Score_Final)*(4096) + 
			SLD2GlobalSLD_INFO->Blending_Score_Final * SLD2GlobalSLD_Table->GSR2_gain_by_sat.gsr2_gain_by_sat_curve[i])>>10;
		if (SLD2GlobalSLD_Tonemap_curve_target[i] > 4095)
			SLD2GlobalSLD_Tonemap_curve_target[i] = 4095;
		if (SLD2GlobalSLD_gain_by_sat_target[i] > 4095)
			SLD2GlobalSLD_gain_by_sat_target[i] = 4095;
	}

	if ((debug_log&_BIT0) != 0) {
		if (debug_cnt >= debug_log_delay)	
			rtd_pr_vpq_info("gSW_SLD,Blending_ScoreSLD=%d,Blending_Score_by_th=%d,temp=%d,total_num=%d,coutner_procees_time_max=%d,TM=%d,%d,%d,%d,SC=%d,%d,%d,%d,\n", 
				SLD2GlobalSLD_INFO->Blending_Score, SLD2GlobalSLD_INFO->Blending_Score_by_th, temp, total_num, coutner_procees_time_max,
				SLD2GlobalSLD_Tonemap_curve_target[0], SLD2GlobalSLD_Tonemap_curve_target[5],
				SLD2GlobalSLD_Tonemap_curve_target[10], SLD2GlobalSLD_Tonemap_curve_target[16],
				SLD2GlobalSLD_gain_by_sat_target[0], SLD2GlobalSLD_gain_by_sat_target[5], 
				SLD2GlobalSLD_gain_by_sat_target[10], SLD2GlobalSLD_gain_by_sat_target[16]);
	}

	driver_update_flag = drvif_fwif_SLD2GlobalSLD_Curve_Smooth(SLD2GlobalSLD_CTRL, SLD2GlobalSLD_Table);
	if (driver_update_flag == 1) {
		fwif_color_Set_GSR2(0);
	}

	if (debug_cnt >= debug_log_delay)
		debug_cnt = 0;
	
	return 0;
}

int fwif_color_sld(void)
{
    color_sld_sld_control_RBUS color_sld_control;

    color_sld_control.regValue = IoReg_Read32(COLOR_SLD_SLD_CONTROL_reg);

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
    unsigned int ddr_offset = 1504; // de-garbage
    unsigned char *logo_ptr = LD_virAddr + ddr_offset;

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

    if (!osd_measure_init) {
        osdovl_mixer_ctrl2_reg.regValue = IoReg_Read32(OSDOVL_Mixer_CTRL2_reg);
        osdovl_osd_db_ctrl_reg.regValue = IoReg_Read32(OSDOVL_OSD_Db_Ctrl_reg);

        osdovl_mixer_ctrl2_reg.measure_osd_zone_en = 1;
        osdovl_mixer_ctrl2_reg.measure_osd_zone_type = 1;
        IoReg_Write32(OSDOVL_Mixer_CTRL2_reg, osdovl_mixer_ctrl2_reg.regValue);

        osdovl_osd_db_ctrl_reg.db_load = 1;
        IoReg_Write32(OSDOVL_OSD_Db_Ctrl_reg, osdovl_osd_db_ctrl_reg.regValue);

        osd_measure_init = true;
    }

    osdovl_measure_osd1_sta_reg.regValue = IoReg_Read32(OSDOVL_measure_osd1_sta_reg);
    osdovl_measure_osd1_end_reg.regValue = IoReg_Read32(OSDOVL_measure_osd1_end_reg);
    osd_sta_x = osdovl_measure_osd1_sta_reg.x;
    osd_sta_y = osdovl_measure_osd1_sta_reg.y;
    osd_end_x = osdovl_measure_osd1_end_reg.x;
    osd_end_y = osdovl_measure_osd1_end_reg.y;

    if (osd_sta_x == 0x1fff && osd_sta_y == 0x1fff && osd_end_x == 0 && osd_end_y == 0) { // no OSD shown
        osd_sta_blkx = osd_sta_blky = osd_end_blkx = osd_end_blky = -1;
    }
    else { // mark osd blocks
        osd_sta_blkx = osd_sta_x / 8;
        osd_end_blkx = (osd_end_x + 7) / 8;
        osd_sta_blky = osd_sta_y / 8;
        osd_end_blky = (osd_end_y + 7) / 8;
    }


    if (LSC_by_memc_logo_en == 0) { // disable, demura table set to all 0, bypass all other flows
        for (i = 0; i <= 270; i++) {
            for (j = 0; j <= 480; j++) {
                demura_tbl_hi[i * 481 + j] = 0;
                demura_tbl_md[i * 481 + j] = 0;
                demura_tbl_lo[i * 481 + j] = 0;
            }
        }
    }
    else {

        if (MEMC_Lib_GetInfoForSLD(1)) { // scene change flag
            memc_sc_flag = true;
        }
        if (MEMC_Lib_GetInfoForSLD(4) > memc_sc_motion1) {
            memc_sc_motion1 = MEMC_Lib_GetInfoForSLD(4);
        }
        if (MEMC_Lib_GetInfoForSLD(5) > memc_sc_motion2) {
            memc_sc_motion2 = MEMC_Lib_GetInfoForSLD(5);
        }

        if (memc_logo_to_demura_update_cnt == 0) {
            memc_logo_to_demura_update_cnt = 1;
        }
        if (frame_cnt % memc_logo_to_demura_update_cnt == 0) { // phase 1: get logo
            //time_start = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
            if (memc_logo_read_en && LD_virAddr != NULL && MEMC_Lib_GetInfoForSLD(7)) {

                //rtd_pr_vpq_emerg("get memc_logo from addr 0x%08x\n", (unsigned int)logo_ptr);

                for (i = 0; i < 270; i++) {
                    memcpy(logo_byte_line[0], logo_ptr, 960 / 8);
                    logo_ptr += 192;
                    memcpy(logo_byte_line[1], logo_ptr, 960 / 8);
                    logo_ptr += 192;

                    for (j = 0; j < 960 / 8; j++) {
                        unsigned char logo_byte1 = logo_byte_line[0][j];
                        unsigned char logo_byte2 = logo_byte_line[1][j];
                        unsigned char logo_bit[2][8] = {0};
                        unsigned char logo_flag_block;
                        for (k = 0; k < 8; k++) {
                            logo_bit[0][k] = (logo_byte1 & (1 << (7 - k))) ? 1 : 0;
                            logo_bit[1][k] = (logo_byte2 & (1 << (7 - k))) ? 1 : 0;
                            //logo_flag_map[i*960 + j + k] = ( (logo_byte & ( 1<<(7-k) )) != 0 );
                        }
                        for (k = 0; k < 4; k++) {
                            logo_flag_block = logo_bit[0][k * 2] || logo_bit[0][k * 2 + 1] || logo_bit[1][k * 2] || logo_bit[1][k * 2 + 1];
                            logo_flag_map[i * 481 + j * 4 + k] = logo_flag_block;
                        }
                    }
                    logo_flag_map[i * 481 + 480] = logo_flag_map[i * 481 + 479]; // last point repeat
                }
                for (j = 0; j <= 480; j++) { // last line repeat
                    logo_flag_map[270 * 481 + j] = logo_flag_map[269 * 481 + j];
                }

                do_counter_update = true;
                do_reset_counter = false;
                //if( MEMC_Lib_GetInfoForSLD(0) != 0 ) // pixel logo clear signal
                //  do_counter_update = false;
                if (memc_sc_motion1 < 100 && memc_sc_motion2 < 100) { // motion cnt low: picture pause
                    do_counter_update = false;
                }
                else if (memc_sc_flag && memc_sc_motion1 > 10000 && memc_sc_motion2 > 10000) { // scene change flag + big motion: scene change
                    do_counter_update = true;
                    do_reset_counter = true;
                }

                // fill hole
                if (memc_logo_fill_hole_en) {
                    //memcpy( logo_flag_map_buf, logo_flag_map, 481*271 );
                    // logo_flag_map_buf: area selection
                    // 0: background, 1: logo object, 2~: hole #
                    int area_idx = 2;

                    memset(is_background, 0, 10000);
                    memset(eq_table, 0, 10000 * 2 * sizeof(unsigned short));
                    memset(eq_searched, 0, 10000);
                    is_background[0] = 1;

                    // 1st pass: mark all 0 areas
                    for (i = 0; i <= 270; i++) {
                        for (j = 0; j <= 480; j++) {
                            if (!logo_flag_map[i * 481 + j]) { // not logo object
                                if (i == 0 || j == 0) { // up/left border -> background
                                    logo_flag_map_buf[i * 481 + j] = 0;
                                }
                                else if (i == 270 || j == 480) { // down/right border -> background + mark background
                                    logo_flag_map_buf[i * 481 + j] = 0;
                                    if (!logo_flag_map[(i - 1) * 481 + j] &&  logo_flag_map_buf[(i - 1) * 481 + j] != 0) { // up
                                        is_background[ logo_flag_map_buf[(i - 1) * 481 + j] ] = 1;
                                    }
                                    if (!logo_flag_map[i * 481 + (j - 1)] &&  logo_flag_map_buf[i * 481 + (j - 1)] != 0) { // left
                                        is_background[ logo_flag_map_buf[i * 481 + (j - 1)] ] = 1;
                                    }
                                }
                                else {
                                    // find neighbor color (4 dir)
                                    if (!logo_flag_map[(i - 1) * 481 + j] && !logo_flag_map[i * 481 + (j - 1)]) { // up & left
                                        if (logo_flag_map_buf[(i - 1) * 481 + j] != logo_flag_map_buf[i * 481 + j - 1]) { // different color, add to eq table
                                            if (logo_flag_map_buf[(i - 1) * 481 + j] == 0) { // up is background
                                                logo_flag_map_buf[i * 481 + j] = logo_flag_map_buf[(i - 1) * 481 + j]; // use up
                                                is_background[ logo_flag_map_buf[i * 481 + (j - 1)] ] = 1;
                                            }
                                            else if (logo_flag_map_buf[i * 481 + (j - 1)] == 0) { // left is background
                                                logo_flag_map_buf[i * 481 + j] = logo_flag_map_buf[i * 481 + (j - 1)]; // use left
                                                is_background[ logo_flag_map_buf[(i - 1) * 481 + j] ] = 1;
                                            }
                                            else {
                                                logo_flag_map_buf[i * 481 + j] = logo_flag_map_buf[(i - 1) * 481 + j]; // use up
                                                eq_table_insert(logo_flag_map_buf[(i - 1) * 481 + j], logo_flag_map_buf[i * 481 + (j - 1)]);
                                            }
                                        }
                                        else {
                                            logo_flag_map_buf[i * 481 + j] = logo_flag_map_buf[i * 481 + (j - 1)];    // use left
                                        }
                                    }
                                    else if (!logo_flag_map[(i - 1) * 481 + j]) { // up only
                                        logo_flag_map_buf[i * 481 + j] = logo_flag_map_buf[(i - 1) * 481 + j];    // use up
                                    }
                                    else if (!logo_flag_map[i * 481 + (j - 1)]) { // left only
                                        logo_flag_map_buf[i * 481 + j] = logo_flag_map_buf[i * 481 + (j - 1)];    // use left
                                    }
                                    else { // new color index
                                        if (area_idx >= 10000) { // not support more than 10000 new regions
                                            logo_flag_map_buf[i * 481 + j] = 0;
                                        }
                                        else {
                                            logo_flag_map_buf[i * 481 + j] = area_idx;
                                            area_idx++;
                                        }
                                    }
                                }
                            }
                            else {
                                logo_flag_map_buf[i * 481 + j] = 1;    // logo object
                            }
                        }
                    }

                    // check eq table for all background
                    for (i = 2; i < area_idx; i++) {
                        if (is_background[i] && !eq_searched[i]) {
                            // trace list
                            int prev = eq_table[i][0];
                            int next = eq_table[i][1];

                            while (prev != 0) {
                                is_background[ prev ] = 1;
                                eq_searched[ prev ] = 1;
                                prev = eq_table[prev][0];
                            }
                            while (next != 0) {
                                is_background[ next ] = 1;
                                eq_searched[ next ] = 1;
                                next = eq_table[next][1];
                            }
                        }
                    }

                    // 2nd pass: fill areas that are not background
                    for (i = 0; i <= 270; i++) {
                        for (j = 0; j <= 480; j++) {
                            if (!logo_flag_map[i * 481 + j] && !is_background[ logo_flag_map_buf[i * 481 + j] ]) {
                                logo_flag_map[i * 481 + j] = 1; // fill hole
                            }
                        }
                    }
                }

            }
            else {
                do_counter_update = true;
                do_reset_counter = true;
                memset(logo_flag_map, 0, 481 * 271);
            }
            memc_sc_flag = false; // reset SC flag after logo flag update
            memc_sc_motion1 = 0;
            memc_sc_motion2 = 0;

            //time_end = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
            //rtd_pr_vpq_emerg("phase1 time: %d on 90k clock\n", (time_end-time_start));

        }

        if (frame_cnt % memc_logo_to_demura_update_cnt == memc_logo_to_demura_update_cnt / 4) { // phase 2: get LD info
            //time_start = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);

            // LD related info
            ld_global_ctrl2_reg.regValue = IoReg_Read32(BLU_LD_Global_Ctrl2_reg);
            LD_valid = ld_global_ctrl2_reg.ld_valid;
            LD_type = ld_global_ctrl2_reg.ld_blk_type;
            LD_Hnum = ld_global_ctrl2_reg.ld_blk_hnum + 1;
            LD_Vnum = ld_global_ctrl2_reg.ld_blk_vnum + 1;

            if (LD_valid && LD_type == 0 && LD_Hnum == 32 && LD_Vnum == 18) {
                LD_APL_valid = drvif_color_get_LD_APL_ave(blk_apl_average);
            }
            else {
                LD_APL_valid = false;
            }

            if (LD_APL_valid) {
                short blk_x_l, blk_y_u;
                short blk_x_r, blk_y_d;
                short x, y;
                unsigned int apl00, apl01, apl10, apl11, intp0, intp1;
                for (i = 0; i <= 270; i++) {
                    for (j = 0; j <= 480; j++) {
                        blk_x_l = (j - 7) / 15;
                        blk_x_l = MIN(blk_x_l, 31);
                        blk_x_l = MAX(blk_x_l, 0);

                        blk_x_r = (j + 7) / 15;
                        blk_x_r = MIN(blk_x_r, 31);
                        blk_x_r = MAX(blk_x_r, 0);

                        blk_y_u = (i - 7) / 15;
                        blk_y_u = MIN(blk_y_u, 17);
                        blk_y_u = MAX(blk_y_u, 0);

                        blk_y_d = (i + 7) / 15;
                        blk_y_d = MIN(blk_y_d, 17);
                        blk_y_d = MAX(blk_y_d, 0);

                        x = (j - 7) % 15;
                        y = (i - 7) % 15;

                        apl00 = blk_apl_average[ blk_y_u * 32 + blk_x_l ];
                        apl01 = blk_apl_average[ blk_y_u * 32 + blk_x_r ];
                        apl10 = blk_apl_average[ blk_y_d * 32 + blk_x_l ];
                        apl11 = blk_apl_average[ blk_y_d * 32 + blk_x_r ];

                        intp0 = (apl00 * (15 - x) + apl01 * x + 7) / 15;
                        intp1 = (apl10 * (15 - x) + apl11 * x + 7) / 15;

                        blk_apl_interp[i * 481 + j] = (intp0 * (15 - y) + intp1 * y + 7) / 15;
                    }
                }
            }
            else {
                for (i = 0; i <= 270; i++) {
                    for (j = 0; j <= 480; j++) {
                        blk_apl_interp[i * 481 + j] = 0;
                    }
                }
            }

            //time_end = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
            //rtd_pr_vpq_emerg("phase2 time: %d on 90k clock\n", (time_end-time_start));
        }

        if (frame_cnt % memc_logo_to_demura_update_cnt == memc_logo_to_demura_update_cnt * 2 / 4) { // phase 3: update counter
            //time_start = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);

            if (logo_to_demura_en) {
                for (i = 0; i <= 270; i++) {
                    for (j = 0; j <= 480; j++) {
                        unsigned int LD_APL;
                        LD_APL = blk_apl_interp[i * 481 + j];

                        if (do_counter_update) {
                            if (logo_flag_map[i * 481 + j]) {
                                logo_demura_counter[i * 481 + j]++;
                                if (logo_demura_counter[i * 481 + j] >= wait_time + drop_time + drop_buffer) {
                                    logo_demura_counter[i * 481 + j] = wait_time + drop_time + drop_buffer;
                                }
                            }
                            else {
                                if (do_reset_counter) {
                                    logo_demura_counter[i * 481 + j] = 0;
                                }
                                else {
                                    logo_demura_counter[i * 481 + j] -= memc_logo_to_demura_ascend_speed;
                                    if (logo_demura_counter[i * 481 + j] < 0) {
                                        logo_demura_counter[i * 481 + j] = 0;
                                    }
                                }
                            }
                        }

                        drop_block = drop_limit * (logo_demura_counter[i * 481 + j] - wait_time) / drop_time;
                        if (LD_APL > 1024 - drop_block) { // decrease counter on too bright part to avoid luminance inversion
                            if (do_reset_counter) {
                                logo_demura_counter[i * 481 + j] = 0;
                            }
                            else {
                                logo_demura_counter[i * 481 + j] -= memc_logo_to_demura_ascend_speed;
                            }
                        }
                        if (logo_demura_counter[i * 481 + j] < 0) {
                            logo_demura_counter[i * 481 + j] = 0;
                        }

                        if (logo_demura_counter[i * 481 + j] < wait_time) {
                            drop_block = 0;
                        }
                        else if (logo_demura_counter[i * 481 + j] > wait_time + drop_time) {
                            drop_block = drop_limit;
                        }
                        else {
                            drop_block = drop_limit * (logo_demura_counter[i * 481 + j] - wait_time) / drop_time;
                        }
                        //*/

                        //drop_block = block_logo_flag[1][1] ? drop_limit : 0;

                        if ((j >= osd_sta_blkx) && (j <= osd_end_blkx) && (i >= osd_sta_blky) && (i <= osd_end_blky)) {
                            // osd part, bypass
                            demura_tbl_hi[i * 481 + j] = 0;
                            demura_tbl_md[i * 481 + j] = 0;
                            demura_tbl_lo[i * 481 + j] = 0;
                        }
                        else {
                            demura_tbl_hi[i * 481 + j] = 0 - (drop_block >> 2);
                            demura_tbl_md[i * 481 + j] = 0 - (drop_block >> 2);
                            demura_tbl_lo[i * 481 + j] = 0;
                        }
                    }
                }

                if (memc_logo_filter_en) {
                    // horizontal [1 2 1] filter
                    short filter_sum = 0;
                    for (i = 0; i <= 270; i++) {
                        for (j = 0; j <= 480; j++) {
                            int j_left = (j == 0) ? 0 : j - 1;
                            int j_right = (j == 480) ? 480 : j + 1;
                            filter_sum = (demura_tbl_hi[i * 481 + j_left] + demura_tbl_hi[i * 481 + j] * 2 + demura_tbl_hi[i * 481 + j_right]) >> 2;

                            demura_tbl_md[i * 481 + j] = filter_sum;
                        }
                    }
                    memcpy(demura_tbl_hi, demura_tbl_md, 481 * 271 * sizeof(short));

                    // vertical [1 2 1] filter
                    for (i = 0; i <= 270; i++) {
                        for (j = 0; j <= 480; j++) {
                            int i_up = (i == 0) ? 0 : i - 1;
                            int i_down = (i == 270) ? 270 : i + 1;
                            filter_sum = (demura_tbl_hi[i_up * 481 + j] + demura_tbl_hi[i * 481 + j] * 2 + demura_tbl_hi[i_down * 481 + j]) >> 2;

                            demura_tbl_md[i * 481 + j] = filter_sum;
                        }
                    }
                    memcpy(demura_tbl_hi, demura_tbl_md, 481 * 271 * sizeof(short));
                }

            }
            //time_end = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
            //rtd_pr_vpq_emerg("phase3 time: %d on 90k clock\n", (time_end-time_start));
        }
    }

    if (frame_cnt % memc_logo_to_demura_update_cnt == memc_logo_to_demura_update_cnt / 4 * 3) { // phase 4: encode demura table
        //time_start = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
        if (logo_to_demura_en) {

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