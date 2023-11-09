/******************************************************************************
 *
 *   Copyright(c) 2021. Realtek Semiconductor Corp. All rights reserved.
 *
 *   @author timo.wu@realtek.com
 *
 *****************************************************************************/
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/miscdevice.h>
#include <linux/kthread.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/pageremap.h>
#include <linux/string.h>
#include <linux/freezer.h>

#include <linux/uaccess.h>
#include <asm/cacheflush.h>

#include <mach/platform.h>
#include <rtk_kdriver/io.h>
#include <rtk_kdriver/rtk_crt.h>
#include "rbus/wov_reg.h"
#include <rtk_kdriver/rtk_vad_def.h>

/******************************************************************************
                     VAD only works on WOV DMIC path
*****************************************************************************/

extern struct semaphore rtkvad_Semaphore;
static unsigned char gVadPitchMode = RTK_VAD_PITCH_HIGH_SENSITIVITY;

void rtk_vad_pitch_det_init(void)
{
    VAD_INFO("\033[1;32;33m" "rtk vad: pitch_det_init" "\033[m");
    
    down(&rtkvad_Semaphore);
    
    CRT_CLK_OnOff(WOV, CLK_ON, NULL);
    
    rtd_maskl(WOV_wov_vad_1_reg, ~(WOV_wov_vad_1_vad_in_sel_mask), WOV_wov_vad_1_vad_in_sel(0x1));//3'b001: ad0_out_r (lg 2ch dmic use ad0)

    rtd_outl(WOV_stby_tcon_1_reg, 0xd54deb00);// 2.25Mhz
    rtd_outl(WOV_stby_ad0_1_reg, (WOV_stby_ad0_1_adc0_ad_gain_l(0x64)|WOV_stby_ad0_1_adc0_ad_gain_r(0x64)));// dmic0 for MMD201_65DB dmic
    rtd_outl(WOV_stby_ad1_1_reg, (WOV_stby_ad1_1_adc1_ad_gain_l(0x64)|WOV_stby_ad1_1_adc1_ad_gain_r(0x64)));// dmic1 for MMD201_65DB dmic
    mdelay(50);
    rtd_maskl(WOV_stby_tcon_1_reg, ~(WOV_stby_tcon_1_dmic_0_en_mask), WOV_stby_tcon_1_dmic_0_en(0x1));
    
    rtd_maskl(WOV_wov_ctrl_1_reg, ~(WOV_wov_ctrl_1_reg_wov_ch_sel_mask|WOV_wov_ctrl_1_reg_wov_ch_mode_mask|WOV_wov_ctrl_1_reg_wov_sram_wr_en_mask|
        WOV_wov_ctrl_1_reg_wov_sram_rd_en_mask|WOV_wov_ctrl_1_reg_wov_func_en_mask), (WOV_wov_ctrl_1_reg_wov_ch_sel(0x0)|WOV_wov_ctrl_1_reg_wov_ch_mode(0x2)|
        WOV_wov_ctrl_1_reg_wov_sram_wr_en(0x0)|WOV_wov_ctrl_1_reg_wov_sram_rd_en(0x0)|WOV_wov_ctrl_1_reg_wov_func_en(0x0)));//bit[4:3]=2, bit[5:6]=0 (00: ad0 l +r)

    up(&rtkvad_Semaphore);
}
EXPORT_SYMBOL(rtk_vad_pitch_det_init);


void rtk_vad_set_pitch_det_mode(RTK_VAD_PITCH_MODE vad_pitch_mode)
{
    VAD_INFO("\033[1;32;33m" "rtk vad: set_pitch_det_mode %u\n" "\033[m", vad_pitch_mode);
    
    down(&rtkvad_Semaphore);
    
    gVadPitchMode = vad_pitch_mode;
    
    up(&rtkvad_Semaphore);
}
EXPORT_SYMBOL(rtk_vad_set_pitch_det_mode);


void rtk_vad_pitch_det_start(void)
{
    VAD_INFO("\033[1;32;33m" "rtk vad: pitch_det_start" "\033[m");
    
    down(&rtkvad_Semaphore);
    
    if(gVadPitchMode == RTK_VAD_PITCH_LOW_SENSITIVITY)
    {
        VAD_INFO("\033[1;32;33m" "RTK_VAD_PITCH_LOW_SENSITIVITY Mode" "\033[m");
        rtd_maskl(WOV_mm_vad_energy6_reg, ~(WOV_mm_vad_energy6_pitch_det_thr_st_valid_mask), WOV_mm_vad_energy6_pitch_det_thr_st_valid(0x8));
    }
    else if(gVadPitchMode == RTK_VAD_PITCH_MID_SENSITIVITY)
    {
        VAD_INFO("\033[1;32;33m" "RTK_VAD_PITCH_MID_SENSITIVITY Mode" "\033[m");
        rtd_maskl(WOV_mm_vad_energy6_reg, ~(WOV_mm_vad_energy6_pitch_det_thr_st_valid_mask), WOV_mm_vad_energy6_pitch_det_thr_st_valid(0x4));
    }
    else //gVadPitchMode == RTK_VAD_PITCH_HIGH_SENSITIVITY
    {
        VAD_INFO("\033[1;32;33m" "RTK_VAD_PITCH_HIGH_SENSITIVITY Mode" "\033[m");
        rtd_maskl(WOV_mm_vad_energy6_reg, ~(WOV_mm_vad_energy6_pitch_det_thr_st_valid_mask), WOV_mm_vad_energy6_pitch_det_thr_st_valid(0x0));
    }

    rtd_maskl(WOV_stby_tcon_1_reg, ~(WOV_stby_tcon_1_pitch_det_en_mask), WOV_stby_tcon_1_pitch_det_en(0x1));
    
    //clear once
    VAD_INFO("\033[1;32;33m" "clear once" "\033[m");
    rtd_maskl(WOV_mm_vad_energy7_reg, ~(WOV_mm_vad_energy7_pitch_det_clr_mask), WOV_mm_vad_energy7_pitch_det_clr(0x1));
    udelay(10);
    rtd_maskl(WOV_mm_vad_energy7_reg, ~(WOV_mm_vad_energy7_pitch_det_clr_mask), WOV_mm_vad_energy7_pitch_det_clr(0x0));
    
    up(&rtkvad_Semaphore);
}
EXPORT_SYMBOL(rtk_vad_pitch_det_start);

void rtk_vad_pitch_det_stop(void)
{
    VAD_INFO("\033[1;32;33m" "rtk vad: pitch_det_stop" "\033[m");
    
    down(&rtkvad_Semaphore);
    
    rtd_maskl(WOV_stby_tcon_1_reg, ~(WOV_stby_tcon_1_pitch_det_en_mask), WOV_stby_tcon_1_pitch_det_en(0x0));
    
    up(&rtkvad_Semaphore);
}
EXPORT_SYMBOL(rtk_vad_pitch_det_stop);


void rtk_vad_get_pitch_det_result(unsigned char *pitch_det)
{
    unsigned char pitch_val = 0;
    
    down(&rtkvad_Semaphore);
    
    pitch_val = WOV_mm_vad_energy4_get_pitch_det_flag_hold(rtd_inl(WOV_mm_vad_energy4_reg));
    *pitch_det = pitch_val;
    
    up(&rtkvad_Semaphore);
    
    VAD_INFO("\033[1;32;32m" "rtk vad: get_pitch_det_result = %u\n" "\033[m", *pitch_det);
}
EXPORT_SYMBOL(rtk_vad_get_pitch_det_result);


void rtk_vad_reset_pitch_det_result(void)
{
    VAD_INFO("\033[1;32;31m" "rtk vad: reset_pitch_det_result\n" "\033[m");
    
    down(&rtkvad_Semaphore);
    
    rtd_maskl(WOV_mm_vad_energy7_reg, ~(WOV_mm_vad_energy7_pitch_det_clr_mask), WOV_mm_vad_energy7_pitch_det_clr(0x1));
    udelay(10);
    rtd_maskl(WOV_mm_vad_energy7_reg, ~(WOV_mm_vad_energy7_pitch_det_clr_mask), WOV_mm_vad_energy7_pitch_det_clr(0x0));
    
    up(&rtkvad_Semaphore);
}
EXPORT_SYMBOL(rtk_vad_reset_pitch_det_result);
