/*=============================================================================
 * Copyright (c) Realtek Semiconductor Corporation, 2006 All rights reserved.
 *============================================================================*/
/**
 * @file
 * 	This file is for the main function and some initial functions.
 *
 * @author 	$Author: sky $
 * @date 	$Date: 2013/06/05 03:26 $
 * @version 	$Revision: 0.1 $
 * @ingroup 	tve
 */

/**
 * @addtogroup tve
 * @{
 */
#ifndef __TVE_H__
#define __TVE_H__

/*-----------------------------------------------------------------------------
 * Dependency
 *----------------------------------------------------------------------------*/
#include "tvscalercontrol/vdac/vdac.h"

#ifndef __KERNEL__
extern "C" {
#endif

/*-----------------------------------------------------------------------------
 * Type definition
 *----------------------------------------------------------------------------*/
typedef struct {
	unsigned char VPS_RUN_IN_BYTE;
	unsigned char VPS_START_CODE;
	unsigned char VPS_BYTE_2;
	unsigned char VPS_BYTE_3;
	unsigned char VPS_BYTE_4;
	unsigned char VPS_BYTE_5;
	unsigned char VPS_BYTE_6;
	unsigned char VPS_BYTE_7;
	unsigned char VPS_BYTE_8;
	unsigned char VPS_BYTE_9;
	unsigned char VPS_BYTE_10;
	unsigned char VPS_BYTE_11;
	unsigned char VPS_BYTE_12;
	unsigned char VPS_BYTE_13;
	unsigned char VPS_BYTE_14;
} Struct_TVE_VPS_DATA;

typedef enum TVE_VIDEO_MODE_SEL {
	  TVE_NTSC= 0,
		TVE_NTSC_J,
		TVE_NTSC_443,
		TVE_PAL_I,
		TVE_PAL_M,
		TVE_PAL_60,
		TVE_PAL_N,
} TVE_VIDEO_MODE_SEL;

typedef enum TVE_CLOCK {
	TVE_CLOCK_DISABLE=0,
	TVE_CLOCK_ENABLE,
} TVE_CLOCK;

typedef enum TVE_AVOUT_EN {
	TVE_AVOUT_DISABLE=0,
	TVE_AVOUT_ENABLE,
} TVE_AVOUT_EN;

typedef enum TVE_AVOUT_MODE {
	 AVOUT_MODE_VD=0,
	 AVOUT_MODE_TVE=6,
} TVE_AVOUT_MODE;

typedef enum TVE_VBI_EN {
	TVE_VBI_DISABLE=0,
	TVE_VBI_ENABLE,
} TVE_VBI_EN;

typedef enum TVE_WSS_EN {
	TVE_WSS_DISABLE=0,
	TVE_WSS_ENABLE,
} TVE_WSS_EN;

typedef enum TVE_VPS_EN {
	TVE_VPS_DISABLE=0,
	TVE_VPS_ENABLE,
} TVE_VPS_EN;

typedef enum TVE_CC_MODE {
	TVE_CC_DISABLE=0,
	TVE_CC_ODD_FIELD,
	TVE_CC_EVEN_FIELD,
	TVE_CC_BOTH_FIELD,
} TVE_CC_MODE;

typedef enum TVE_VBI_TYPE {
	TVE_VBI_TT=0,
	TVE_VBI_CC,
	TVE_VBI_WSS,
	TVE_VBI_VPS
} TVE_VBI_TYPE;

/*-----------------------------------------------------------------------------
 * Macro definition
 *----------------------------------------------------------------------------*/
#ifndef HIGH_BYTE
#define HIGH_BYTE(x)	(((x)&0xFF00)>>8)
#endif

#ifndef LOW_BYTE
#define LOW_BYTE(x)	((x)&0x00FF)
#endif

#define TVE_ENABLE	(1)
#define TVE_DISABLE	(0)

/*-----------------------------------------------------------------------------
 * Interface
 *----------------------------------------------------------------------------*/
#ifdef CONFIG_HW_SUPPORT_TVE

unsigned int rtdf_tvedma_inl(unsigned int addr);
void drvif_module_tve_init(void);
void drvif_module_tve_set_mode(TVE_VIDEO_MODE_SEL mode);
void drvif_module_tve_clock_enable(TVE_CLOCK on_off);
void drvif_module_tve_AVout_MODE(VDAC_TV_SRC_SEL mode);
void drvif_module_tve_AVout_Enable(TVE_AVOUT_EN on_off);
void drvif_module_tve_vbi_setting(TVE_VBI_EN on_off);
void drvif_module_tve_vbi_memory_setting(unsigned int vbi_addr);
void drvif_module_tve_video_memory_setting(unsigned char vflip_en);
void drvif_module_tve_cc_setting(TVE_CC_MODE mode);
void drvif_module_tve_cc_data(unsigned int odd_data,unsigned int even_data);
void drvif_module_tve_vps_setting(TVE_VPS_EN on_off);
void drvif_module_tve_vps_data(Struct_TVE_VPS_DATA *VPS_DATA);
void drvif_module_tve_wss_setting(TVE_WSS_EN on_off);
void drvif_module_tve_wss_data(unsigned int data);
void drvif_module_tve_interrupt(int enable);
void drvif_module_tve_forcebg(int enable);
void drvif_module_tve_change_memory_setting(unsigned int in_width,
	unsigned char vflip_en);
void drvif_module_tve_reset(void);
void drvif_module_tve_vbi_pos(unsigned int top_val, unsigned int bottom_val);
void drvif_module_tve_wss_pos(unsigned int value);
void drvif_module_tve_vps_pos(unsigned int value);
void drvif_module_tve_vbi_tt_line_num(unsigned int line_num);
void drvif_module_tve_vbidma_trigger(unsigned char triggle_en);
void drvif_tve_resume_setting(void);
void drv_tve_set_video_pal_i(void);

#else /* else CONFIG_HW_SUPPORT_TVE */

#define rtdf_tvedma_inl(ddr) (0)
#define drvif_module_tve_init() ((void)0)
#define drvif_module_tve_set_mode(mode) ((void)0)
#define drvif_module_tve_clock_enable(on_off) ((void)0)
#define drvif_module_tve_AVout_MODE(mode) ((void)0)
#define drvif_module_tve_AVout_Enable(on_off) ((void)0)
#define drvif_module_tve_vbi_setting(on_off) ((void)0)
#define drvif_module_tve_vbi_memory_setting(vbi_addr) ((void)0)
#define drvif_module_tve_video_memory_setting(vflip_en) ((void)0)
#define drvif_module_tve_cc_setting(mode) ((void)0)
#define drvif_module_tve_cc_data(odd_data, even_data) ((void)0)
#define drvif_module_tve_vps_setting(on_off) ((void)0)
#define drvif_module_tve_vps_data(VPS_DATA) ((void)0)
#define drvif_module_tve_wss_setting(on_off) ((void)0)
#define drvif_module_tve_wss_data(data) ((void)0)
#define drvif_module_tve_interrupt(enable) ((void)0)
#define drvif_module_tve_forcebg(enable) ((void)0)
#define drvif_module_tve_change_memory_setting(in_width, vflip_en) ((void)0)
#define drvif_module_tve_reset() ((void)0)
#define drvif_module_tve_vbi_pos(top_val, bottom_val) ((void)0)
#define drvif_module_tve_wss_pos(value) ((void)0)
#define drvif_module_tve_vps_pos(value) ((void)0)
#define drvif_module_tve_vbi_tt_line_num(line_num) ((void)0)
#define drvif_module_tve_vbidma_trigger(triggle_en) ((void)0)
#define drvif_tve_resume_setting() ((void)0)
#define drv_tve_set_video_pal_i() ((void)0)

#endif /* CONFIG_HW_SUPPORT_TVE */

#ifndef __KERNEL__
}
#endif

#endif /* __TVE_H__ */
