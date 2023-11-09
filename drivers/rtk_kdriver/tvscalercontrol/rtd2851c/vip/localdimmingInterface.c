/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2014
  * All rights reserved.
  * ========================================================================*/
  /*================= File Description =======================================*/
  /**
   * @file
   *  This file is for FILM related functions.
   *
   * @author  $eva_sun$
   * @date    $2022.07.10$
   * @version $0.0$
   */

  /**
   * @addtogroup film
   * @{
   */

  /*============================ Module dependency	===========================*/


#include <linux/slab.h>
//#define malloc(size)        vmalloc(size, CTL_KERN)
#define malloc(size)        kmalloc(size, GFP_KERNEL)
#define free(addr)          kfree(addr)
//#define free(addr)          vfree(addr)

#include <tvscalercontrol/vip/localdimmingInterface.h>
#include "rtk_vip_logger.h"
#include <tvscalercontrol/scaler/vipinclude.h> // for typedef that used in VIP_reg_def.h
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/vip/scalerColor.h>
#include <rtk_kdriver/io.h>
#include <mach/rtk_log.h>

///*debug*/
//#include "ldspi_reg.h"
//#include "localdimmingInterface.h"


void drvif_set_LdInterface_Table(DRV_LDINTERFACE_Table *ptr)
{
    /*baisc*/
    ldspi_ld_ctrl_RBUS        ldspi_ld_ctrl_reg;
    ldspi_as_wr_ctrl_RBUS     ldspi_as_wr_ctrl_reg;
    ldspi_out_data_ctrl_RBUS  ldspi_out_data_ctrl_reg;
    ldspi_out_data_form_RBUS  ldspi_out_data_form_reg;
    ldspi_ld_spi_v_total_RBUS ldspi_ld_spi_v_total_reg;
    ldspi_ld_spi_h_total_RBUS ldspi_ld_spi_h_total_reg;
    ldspi_ld_spi_reproduce_mode_RBUS ldspi_ld_spi_reproduce_mode_reg;
    ldspi_data_sram_special_ctrl_0_RBUS  ldspi_data_sram_special_ctrl_0_reg;
    ldspi_rx_data_ctrl_RBUS   ldspi_rx_data_ctrl_reg;
    ldspi_rx_timing_ctrl_RBUS ldspi_rx_timing_ctrl_reg;
    //timing related
    ldspi_send_delay_RBUS  ldspi_send_delay_reg;
    ldspi_data_unit_delay_RBUS ldspi_data_unit_delay_reg;
    ldspi_multi_format_unit_delay_RBUS ldspi_multi_format_unit_delay_reg;
    ldspi_v_sync_duty_RBUS  ldspi_v_sync_duty_reg;
    ldspi_data_h_time_RBUS  ldspi_data_h_time_reg;
    ldspi_cs_hold_time_RBUS  ldspi_cs_hold_time_reg;
    ldspi_sck_hold_time_RBUS ldspi_sck_hold_time_reg;
    ldspi_timing_ctrl_RBUS   ldspi_timing_ctrl_reg;
    ldspi_ld_spi_v_sync_delay_RBUS ld_spi_v_sync_delay_reg;	
    //extra: timing delay	
    ldspi_ld_spi_ld_timing_fune_tune_RBUS ldspi_ld_spi_ld_timing_fune_tune_reg;  

    DRV_LDINTERFACE_TimingReg LDINTERFACE_TimingReg = {0,0,0,0,0,0,0,0,0};

    /*disable start enable*/
    drvif_set_LdInterface_StartEn(0);

    /*Read Register*/
    ldspi_ld_ctrl_reg.regValue =    IoReg_Read32(LDSPI_LD_CTRL_reg);
    ldspi_as_wr_ctrl_reg.regValue = IoReg_Read32(LDSPI_AS_WR_CTRL_reg);
    ldspi_out_data_ctrl_reg.regValue = IoReg_Read32(LDSPI_Out_data_CTRL_reg);
    ldspi_out_data_form_reg.regValue = IoReg_Read32(LDSPI_Out_data_form_reg);
    ldspi_ld_spi_v_total_reg.regValue = IoReg_Read32(LDSPI_ld_spi_v_total_reg);
    ldspi_ld_spi_h_total_reg.regValue = IoReg_Read32(LDSPI_ld_spi_h_total_reg);
    ldspi_ld_spi_reproduce_mode_reg.regValue = IoReg_Read32(LDSPI_ld_spi_reproduce_mode_reg);
    ldspi_data_sram_special_ctrl_0_reg.regValue = IoReg_Read32(LDSPI_Data_SRAM_SPECIAL_CTRL_0_reg);
    ldspi_rx_data_ctrl_reg.regValue =  IoReg_Read32(LDSPI_RX_data_CTRL_reg);
    ldspi_rx_timing_ctrl_reg.regValue = IoReg_Read32(LDSPI_Rx_timing_ctrl_reg);
    ldspi_ld_spi_ld_timing_fune_tune_reg.regValue = IoReg_Read32(LDSPI_ld_spi_LD_timing_fune_tune_reg);
    //Timing related
    ld_spi_v_sync_delay_reg.regValue = IoReg_Read32(LDSPI_ld_spi_V_sync_delay_reg);
    ldspi_send_delay_reg.regValue = IoReg_Read32(LDSPI_Send_delay_reg);
    ldspi_data_unit_delay_reg.regValue = IoReg_Read32(LDSPI_Data_unit_delay_reg);
    ldspi_multi_format_unit_delay_reg.regValue = IoReg_Read32(LDSPI_Multi_format_unit_delay_reg);
    ldspi_v_sync_duty_reg.regValue = IoReg_Read32(LDSPI_V_sync_duty_reg);
    ldspi_data_h_time_reg.regValue = IoReg_Read32(LDSPI_Data_h_time_reg);
    ldspi_cs_hold_time_reg.regValue = IoReg_Read32(LDSPI_Cs_hold_time_reg);
    ldspi_sck_hold_time_reg.regValue = IoReg_Read32(LDSPI_sck_hold_time_reg);
    ldspi_timing_ctrl_reg.regValue = IoReg_Read32(LDSPI_Timing_CTRL_reg);

    /*basic control*/
    ldspi_ld_ctrl_reg.ld_spi_en = 1;
    ldspi_ld_ctrl_reg.start_enable = 0;//disable start enable
    ldspi_ld_ctrl_reg.ld_mode   = ptr->LDINTERFACE_Basic_Ctrl.ld_mode;
    ldspi_ld_ctrl_reg.separate_two_block   = ptr->LDINTERFACE_Basic_Ctrl.separate_two_block;
    ldspi_ld_ctrl_reg.as_led_reset   = ptr->LDINTERFACE_Basic_Ctrl.as_led_reset;
    ldspi_ld_ctrl_reg.as_led_with_cs = ptr->LDINTERFACE_Basic_Ctrl.as_led_with_cs;
    ldspi_ld_ctrl_reg.out_data_align = ptr->LDINTERFACE_Basic_Ctrl.out_data_align;
    ldspi_ld_ctrl_reg.in_data_align = ptr->LDINTERFACE_Basic_Ctrl.in_data_align;
    /*basic control:send mode*/
    switch((_SEND_MODE)ptr->LDINTERFACE_Basic_Ctrl.send_mode){
        case VSYNC_MODE:
            ldspi_ld_ctrl_reg.send_follow_vsync = 1;
        break;
        case TRIGGER_MODE:
            ldspi_ld_ctrl_reg.send_follow_vsync = 0;
        break;
        default:
            ldspi_ld_ctrl_reg.send_follow_vsync = 0;
        break;
    }
    //ldspi_ld_ctrl_reg.send_trigger   = ptr->LDINTERFACE_Basic_Ctrl.send_trigger;
    /*basic control:as mode:w_package_repeater & force tx mode*/
    ldspi_as_wr_ctrl_reg.w_package_repeater = ptr->LDINTERFACE_Basic_Ctrl.w_package_repeater;
    if(ptr->LDINTERFACE_Basic_Ctrl.as_force_tx){//as force tx mode
        ldspi_as_wr_ctrl_reg.force_txorrx_mode_en = 1;
        /*force rx mode not supported after k8*/
    }
    ld_spi_v_sync_delay_reg.vsync_delay = ptr->LDINTERFACE_Basic_Ctrl.vsync_delay;

    /*Output*/
    ldspi_out_data_ctrl_reg.data_endian = 0;
    ldspi_out_data_ctrl_reg.header_format = ptr->LDINTERFACE_Output.header_format;
    ldspi_out_data_form_reg.header_units  = ptr->LDINTERFACE_Output.header_units;
    ldspi_out_data_ctrl_reg.output_data_format = ptr->LDINTERFACE_Output.output_data_format;
    ldspi_out_data_ctrl_reg.output_units  = ptr->LDINTERFACE_Output.output_units-1;
    ldspi_out_data_ctrl_reg.tailer_format = ptr->LDINTERFACE_Output.tailer_format;
    ldspi_out_data_form_reg.tailer_units  = ptr->LDINTERFACE_Output.tailer_units;
    /*DRV_LDINTERFACE_SHIFT_SWAP*/
    ldspi_data_sram_special_ctrl_0_reg.shift_op_wholesram = ptr->LDINTERFACE_Shift_Swap.shift_op;
    ldspi_data_sram_special_ctrl_0_reg.data_sram_sft_ctrl = (ptr->LDINTERFACE_Shift_Swap.shift_dir<<4)|(ptr->LDINTERFACE_Shift_Swap.shift_num);
    ldspi_as_wr_ctrl_reg.swap_op_wholesram = ptr->LDINTERFACE_Shift_Swap.swap_op;
    ldspi_as_wr_ctrl_reg.swap_of_16_bit = ptr->LDINTERFACE_Shift_Swap.swap_en;
    /*DRV_LDINTERFACE_INV*/
    ldspi_ld_ctrl_reg.tx_clk_inv = ptr->LDINTERFACE_Inv.tx_clk_inv;
    ldspi_ld_ctrl_reg.tx_cs_inv  = ptr->LDINTERFACE_Inv.tx_cs_inv;
    ldspi_ld_ctrl_reg.tx_sdo_inv = ptr->LDINTERFACE_Inv.tx_sdo_inv;
    ldspi_ld_ctrl_reg.tx_vsync_inv = ptr->LDINTERFACE_Inv.tx_vsync_inv;
    /*LDINTERFACE_Repeat*/
    ldspi_data_sram_special_ctrl_0_reg.hrepeat = ptr->LDINTERFACE_Repeat.hrepeat;
    ldspi_data_sram_special_ctrl_0_reg.vrepeat = ptr->LDINTERFACE_Repeat.vrepeat;
    ldspi_data_sram_special_ctrl_0_reg.vrepeat_width = ptr->LDINTERFACE_Repeat.vrepeat_width;
    /*LDINTERFACE_Reproduce*/
    ldspi_ld_spi_v_total_reg.dv_total_src_sel = ptr->LDINTERFACE_Reproduce.dv_total_src_sel;
    ldspi_ld_spi_v_total_reg.dv_total_sync_dtg= ptr->LDINTERFACE_Reproduce.dv_total_sync_dtg;
    ldspi_ld_spi_v_total_reg.ld_spi_dv_total  = ptr->LDINTERFACE_Reproduce.ld_spi_dv_total;
    ldspi_ld_spi_h_total_reg.ld_spi_dh_total  = ptr->LDINTERFACE_Reproduce.ld_spi_dh_total;
    ldspi_ld_spi_reproduce_mode_reg.dv_reproduce_mode = ptr->LDINTERFACE_Reproduce.dv_reproduce_mode;
    /*DRV_LDINTERFACE_TimingReal:calculate*/
    drvif_cal_LdInterface_TimingReg(&ptr->LDINTERFACE_TimingReal,&LDINTERFACE_TimingReg);
    /*DRV_LDINTERFACE_TimingReal:set register*/
    ldspi_send_delay_reg.data_send_delay = LDINTERFACE_TimingReg.data_send_delay;
    ldspi_data_unit_delay_reg.each_unit_delay = LDINTERFACE_TimingReg.each_unit_delay;
    ldspi_multi_format_unit_delay_reg.multi_unit_delay = LDINTERFACE_TimingReg.multi_unit_delay;
    ldspi_v_sync_duty_reg.vsync_d = LDINTERFACE_TimingReg.vsync_d;
    ldspi_data_h_time_reg.cs_end_delay_time = LDINTERFACE_TimingReg.cs_end_delay_time;
    ldspi_cs_hold_time_reg.cs_hold_time = LDINTERFACE_TimingReg.cs_hold_time;
    ldspi_sck_hold_time_reg.sck_hold_time = LDINTERFACE_TimingReg.sck_hold_time;
    ldspi_timing_ctrl_reg.sck_h = LDINTERFACE_TimingReg.sck_h;
    ldspi_timing_ctrl_reg.sck_l = LDINTERFACE_TimingReg.sck_h;

    //jira: RL6748-1927 ,add ld_backlight_timing_delay 0x7	
    //ldspi_ld_spi_ld_timing_fune_tune_reg.ld_backlight_timing_delay = 0x7;	
    /*write register*/
    IoReg_Write32(LDSPI_LD_CTRL_reg,   ldspi_ld_ctrl_reg.regValue);
    IoReg_Write32(LDSPI_AS_WR_CTRL_reg,ldspi_as_wr_ctrl_reg.regValue);
    IoReg_Write32(LDSPI_Out_data_CTRL_reg,ldspi_out_data_ctrl_reg.regValue);
    IoReg_Write32(LDSPI_Out_data_form_reg,ldspi_out_data_form_reg.regValue);
    IoReg_Write32(LDSPI_ld_spi_v_total_reg,ldspi_ld_spi_v_total_reg.regValue);
    IoReg_Write32(LDSPI_ld_spi_h_total_reg,ldspi_ld_spi_h_total_reg.regValue);
    IoReg_Write32(LDSPI_ld_spi_reproduce_mode_reg,   ldspi_ld_spi_reproduce_mode_reg.regValue);
    IoReg_Write32(LDSPI_Data_SRAM_SPECIAL_CTRL_0_reg,ldspi_data_sram_special_ctrl_0_reg.regValue);
    IoReg_Write32(LDSPI_RX_data_CTRL_reg,      ldspi_rx_data_ctrl_reg.regValue);
    IoReg_Write32(LDSPI_Rx_timing_ctrl_reg,    ldspi_rx_timing_ctrl_reg.regValue);
    //timing related
    IoReg_Write32(LDSPI_ld_spi_V_sync_delay_reg,ld_spi_v_sync_delay_reg.regValue);
    IoReg_Write32(LDSPI_Send_delay_reg,   ldspi_send_delay_reg.regValue);
    IoReg_Write32(LDSPI_Data_unit_delay_reg,ldspi_data_unit_delay_reg.regValue);
    IoReg_Write32(LDSPI_Multi_format_unit_delay_reg,ldspi_multi_format_unit_delay_reg.regValue);
    IoReg_Write32(LDSPI_V_sync_duty_reg,  ldspi_v_sync_duty_reg.regValue);
    IoReg_Write32(LDSPI_Data_h_time_reg,  ldspi_data_h_time_reg.regValue);
    IoReg_Write32(LDSPI_Cs_hold_time_reg, ldspi_cs_hold_time_reg.regValue);
    IoReg_Write32(LDSPI_sck_hold_time_reg,ldspi_sck_hold_time_reg.regValue);
    IoReg_Write32(LDSPI_Timing_CTRL_reg,  ldspi_timing_ctrl_reg.regValue);
    //jira: RL6748-1927 ,add ld_backlight_timing_delay 0x7	
    IoReg_Write32(LDSPI_ld_spi_LD_timing_fune_tune_reg,  ldspi_ld_spi_ld_timing_fune_tune_reg.regValue);
   
    /*after settings done,enable start enable*/
    drvif_set_LdInterface_StartEn(1);
}

void drvif_set_LdInterface_Enable(UINT8 enable)
{
    ldspi_ld_ctrl_RBUS  ldspi_ld_ctrl_reg;
    ldspi_ld_ctrl_reg.regValue =    IoReg_Read32(LDSPI_LD_CTRL_reg);
    ldspi_ld_ctrl_reg.ld_spi_en    = 1;
    //ldspi_ld_ctrl_reg.start_enable = enable;
    IoReg_Write32(LDSPI_LD_CTRL_reg,   ldspi_ld_ctrl_reg.regValue);
}

void drvif_set_LdInterface_StartEn(UINT8 enable)
{
    ldspi_ld_ctrl_RBUS  ldspi_ld_ctrl_reg;
    ldspi_ld_ctrl_reg.regValue =    IoReg_Read32(LDSPI_LD_CTRL_reg);
    ldspi_ld_ctrl_reg.start_enable = enable;
    IoReg_Write32(LDSPI_LD_CTRL_reg,   ldspi_ld_ctrl_reg.regValue);
}

void drvif_set_LdInterface_SendTrigger()
{
    ldspi_ld_ctrl_RBUS  ldspi_ld_ctrl_reg;
    ldspi_ld_ctrl_reg.regValue =    IoReg_Read32(LDSPI_LD_CTRL_reg);
    ldspi_ld_ctrl_reg.send_trigger    = 1;
    IoReg_Write32(LDSPI_LD_CTRL_reg,   ldspi_ld_ctrl_reg.regValue);
}

void drvif_cal_LdInterface_TimingReg(DRV_LDINTERFACE_TimingReal *ptrReal, DRV_LDINTERFACE_TimingReg *ptrReg)
{
    /*Timing Real 2 Register*/
    UINT32 tmpVal;
    UINT32 regVal;
    const UINT32 baseUnit = 4;

    /*sck_l*/
    tmpVal = 10*(ptrReal->T3/baseUnit) -10 + 5;
    regVal = (UINT32)tmpVal/10;
    ptrReg->sck_l = regVal;
    /*vsync_d*/
    tmpVal = 10*ptrReal->T5/baseUnit/(128)-10 + 5;
    regVal = (UINT32)tmpVal/10;
    ptrReg->vsync_d = regVal;
    /*data_send_delay*/
    tmpVal = 10*(ptrReal->T6/baseUnit) - 100 + 5;
    regVal = (UINT32)tmpVal/10;
    ptrReg->data_send_delay = regVal;
    /*sck_h*/
    tmpVal = 10*ptrReal->T2/baseUnit - 10*ptrReg->sck_l - 20 + 5 ;
    regVal = (UINT32)tmpVal/10;
    ptrReg->sck_h = regVal;
    /*multi_unit_delay & each_unit_delay*/
    tmpVal = 10*(ptrReal->T4/baseUnit - ptrReg->sck_l - ptrReg->sck_h) + 5;
    regVal = (UINT32)tmpVal/10;
    if(ptrReal->fMultiUnit){
        ptrReg->multi_unit_delay = regVal;
        ptrReg->each_unit_delay = 0;
    }else{
        ptrReg->each_unit_delay = regVal;
        ptrReg->multi_unit_delay = 0;
    }
    /*cs_hold_time*/
    tmpVal = 10*(ptrReal->T1)/baseUnit - 10*ptrReg->data_send_delay - 100 + 5;
    regVal = (UINT32)tmpVal/10;
    ptrReg->cs_hold_time = regVal;
    /*sck_hold_time*/
    tmpVal = 10*(ptrReal->T7)/baseUnit - 10*ptrReg->sck_l + 5;
    regVal = (UINT32)tmpVal/10;
    ptrReg->sck_hold_time = regVal;
    /*cs_end_delay_time*/
    tmpVal = 10*(ptrReal->cs_end)/baseUnit + 5;
    regVal = (UINT32)tmpVal/10;
    ptrReg->cs_end_delay_time = regVal;
}


void drvif_cal_LdInterface_TimingReal(DRV_LDINTERFACE_TimingReal *ptrReal, DRV_LDINTERFACE_TimingReg *ptrReg)
{
    /*register 2 timing real*/
    UINT32 tmpVal;
    const UINT32 baseUnit = 4;
    /*T1*/
    tmpVal = ptrReg->data_send_delay + ptrReg->cs_hold_time + 10;
    ptrReal->T1 = (10*tmpVal *baseUnit + 5)/10;
    /*T1_star*/
    tmpVal = ptrReg->data_send_delay + ptrReg->sck_hold_time +ptrReg->sck_l + 10;
    ptrReal->T1_star = (10*tmpVal *baseUnit + 5)/10;
    /*T2*/
    tmpVal = ptrReg->sck_l + ptrReg->sck_h + 2;
    ptrReal->T2 = (10*tmpVal *baseUnit + 5)/10;
    /*T3*/
    tmpVal = ptrReg->sck_l + 1;
    ptrReal->T3 = (10*tmpVal *baseUnit + 5)/10;
    /*T4*/
    if(ptrReg->each_unit_delay>0){
        tmpVal = ptrReg->each_unit_delay + ptrReg->sck_l + ptrReg->sck_h;
    }else if(ptrReg->multi_unit_delay>0){
        tmpVal = ptrReg->multi_unit_delay + ptrReg->sck_l + ptrReg->sck_h;
    }else{
        tmpVal = ptrReg->sck_h + ptrReg->sck_l;
    }
    ptrReal->T4 = (10*tmpVal *baseUnit + 5)/10;
    /*T5*/
    tmpVal = (ptrReg->vsync_d + 1)*128;
    ptrReal->T5 = (10*tmpVal *baseUnit + 5)/10;
    /*T6*/
    tmpVal = ptrReg->data_send_delay + 10;
    ptrReal->T6 = (10*tmpVal *baseUnit + 5)/10;
    /*T7*/
    tmpVal = ptrReg->sck_hold_time + ptrReg->sck_l;
    ptrReal->T7 = (10*tmpVal *baseUnit + 5)/10;
    /*cs_end_delay*/
    tmpVal = ptrReg->cs_end_delay_time;
    ptrReal->cs_end = (10*tmpVal *baseUnit + 5)/10;

    /*printk(KERN_EMERG "Timing:%d %d %d %d %d %d %d %d %d %d \n",\
           (int)ptrReal->T1,\
           (int)ptrReal->T1_star,\
           (int)ptrReal->T2,\
           (int)ptrReal->T3,\
           (int)ptrReal->T4,\
           (int)ptrReal->T5,\
           (int)ptrReal->T6,\
           (int)ptrReal->T7,\
           (int)ptrReal->cs_end,\
           (int)ptrReal->fMultiUnit);
      */

}


void drvif_get_LdInterface_Table(DRV_LDINTERFACE_Table *ptr)
{
    /*baisc*/
    ldspi_ld_ctrl_RBUS        ldspi_ld_ctrl_reg;
    ldspi_as_wr_ctrl_RBUS     ldspi_as_wr_ctrl_reg;
    ldspi_out_data_ctrl_RBUS  ldspi_out_data_ctrl_reg;
    ldspi_out_data_form_RBUS  ldspi_out_data_form_reg;
    ldspi_ld_spi_v_total_RBUS ldspi_ld_spi_v_total_reg;
    ldspi_ld_spi_h_total_RBUS ldspi_ld_spi_h_total_reg;
    ldspi_ld_spi_reproduce_mode_RBUS ldspi_ld_spi_reproduce_mode_reg;
    ldspi_data_sram_special_ctrl_0_RBUS ldspi_data_sram_special_ctrl_0_reg;
    ldspi_rx_data_ctrl_RBUS   ldspi_rx_data_ctrl_reg;
    ldspi_rx_timing_ctrl_RBUS ldspi_rx_timing_ctrl_reg;
    //timing related
    ldspi_send_delay_RBUS  ldspi_send_delay_reg;
    ldspi_data_unit_delay_RBUS ldspi_data_unit_delay_reg;
    ldspi_multi_format_unit_delay_RBUS ldspi_multi_format_unit_delay_reg;
    ldspi_v_sync_duty_RBUS  ldspi_v_sync_duty_reg;
    ldspi_data_h_time_RBUS  ldspi_data_h_time_reg;
    ldspi_cs_hold_time_RBUS  ldspi_cs_hold_time_reg;
    ldspi_sck_hold_time_RBUS ldspi_sck_hold_time_reg;
    ldspi_timing_ctrl_RBUS   ldspi_timing_ctrl_reg;

    DRV_LDINTERFACE_TimingReg LDINTERFACE_TimingReg = {0,0,0,0,0,0,0,0,0};
    DRV_LDINTERFACE_TimingReg *ptrReg;

    ptrReg = &LDINTERFACE_TimingReg;
    /*Read Register*/
    ldspi_ld_ctrl_reg.regValue =    IoReg_Read32(LDSPI_LD_CTRL_reg);
    ldspi_as_wr_ctrl_reg.regValue = IoReg_Read32(LDSPI_AS_WR_CTRL_reg);
    ldspi_out_data_ctrl_reg.regValue = IoReg_Read32(LDSPI_Out_data_CTRL_reg);
    ldspi_out_data_form_reg.regValue = IoReg_Read32(LDSPI_Out_data_form_reg);
    ldspi_ld_spi_v_total_reg.regValue = IoReg_Read32(LDSPI_ld_spi_v_total_reg);
    ldspi_ld_spi_h_total_reg.regValue = IoReg_Read32(LDSPI_ld_spi_h_total_reg);
    ldspi_ld_spi_reproduce_mode_reg.regValue = IoReg_Read32(LDSPI_ld_spi_reproduce_mode_reg);
    ldspi_data_sram_special_ctrl_0_reg.regValue = IoReg_Read32(LDSPI_Data_SRAM_SPECIAL_CTRL_0_reg);
    ldspi_rx_data_ctrl_reg.regValue =  IoReg_Read32(LDSPI_RX_data_CTRL_reg);
    ldspi_rx_timing_ctrl_reg.regValue = IoReg_Read32(LDSPI_Rx_timing_ctrl_reg);
    //Timing related
    ldspi_send_delay_reg.regValue = IoReg_Read32(LDSPI_Send_delay_reg);
    ldspi_data_unit_delay_reg.regValue = IoReg_Read32(LDSPI_Data_unit_delay_reg);
    ldspi_multi_format_unit_delay_reg.regValue = IoReg_Read32(LDSPI_Multi_format_unit_delay_reg);
    ldspi_v_sync_duty_reg.regValue =  IoReg_Read32(LDSPI_V_sync_duty_reg);
    ldspi_data_h_time_reg.regValue =  IoReg_Read32(LDSPI_Data_h_time_reg);
    ldspi_cs_hold_time_reg.regValue = IoReg_Read32(LDSPI_Cs_hold_time_reg);
    ldspi_sck_hold_time_reg.regValue = IoReg_Read32(LDSPI_sck_hold_time_reg);
    ldspi_timing_ctrl_reg.regValue = IoReg_Read32(LDSPI_Timing_CTRL_reg);

    if(ldspi_as_wr_ctrl_reg.force_txorrx_mode_en){
        ptr->LDINTERFACE_Basic_Ctrl.as_force_tx = 1;
    }else{//do not care: no RX mode
        ptr->LDINTERFACE_Basic_Ctrl.as_force_tx = 0;
    }
    ptr->LDINTERFACE_Basic_Ctrl.as_led_reset = ldspi_ld_ctrl_reg.as_led_reset;
    ptr->LDINTERFACE_Basic_Ctrl.as_led_with_cs = ldspi_ld_ctrl_reg.as_led_with_cs;
    ptr->LDINTERFACE_Basic_Ctrl.out_data_align = ldspi_ld_ctrl_reg.out_data_align;
    ptr->LDINTERFACE_Basic_Ctrl.ld_mode    = ldspi_ld_ctrl_reg.ld_mode;
    if(ldspi_ld_ctrl_reg.send_follow_vsync)
        ptr->LDINTERFACE_Basic_Ctrl.send_mode = VSYNC_MODE;
    else
        ptr->LDINTERFACE_Basic_Ctrl.send_mode = TRIGGER_MODE;
    ptr->LDINTERFACE_Basic_Ctrl.separate_two_block = ldspi_ld_ctrl_reg.separate_two_block;
    ptr->LDINTERFACE_Basic_Ctrl.w_package_repeater = ldspi_as_wr_ctrl_reg.w_package_repeater;

    ptr->LDINTERFACE_Shift_Swap.shift_op = ldspi_data_sram_special_ctrl_0_reg.shift_op_wholesram;
    ptr->LDINTERFACE_Shift_Swap.shift_dir = ldspi_data_sram_special_ctrl_0_reg.data_sram_sft_ctrl>>4;
    ptr->LDINTERFACE_Shift_Swap.shift_num = ldspi_data_sram_special_ctrl_0_reg.data_sram_sft_ctrl&0x7;
    ptr->LDINTERFACE_Shift_Swap.swap_op = ldspi_as_wr_ctrl_reg.swap_op_wholesram;
    ptr->LDINTERFACE_Shift_Swap.swap_en = ldspi_as_wr_ctrl_reg.swap_of_16_bit;

    ptr->LDINTERFACE_Inv.tx_clk_inv = ldspi_ld_ctrl_reg.tx_clk_inv;
    ptr->LDINTERFACE_Inv.tx_cs_inv = ldspi_ld_ctrl_reg.tx_cs_inv;
    ptr->LDINTERFACE_Inv.tx_sdo_inv = ldspi_ld_ctrl_reg.tx_sdo_inv;
    ptr->LDINTERFACE_Inv.tx_vsync_inv = ldspi_ld_ctrl_reg.tx_vsync_inv;

    ptr->LDINTERFACE_Output.data_sram_separate = ldspi_out_data_form_reg.data_sram_separate;
    ptr->LDINTERFACE_Output.header_units = ldspi_out_data_form_reg.header_units;
    ptr->LDINTERFACE_Output.tailer_units = ldspi_out_data_form_reg.tailer_units;
    ptr->LDINTERFACE_Output.output_units = ldspi_out_data_ctrl_reg.output_units;
    ptr->LDINTERFACE_Output.header_format = ldspi_out_data_ctrl_reg.header_format;
    ptr->LDINTERFACE_Output.tailer_format = ldspi_out_data_ctrl_reg.tailer_format;
    ptr->LDINTERFACE_Output.output_data_format = ldspi_out_data_ctrl_reg.output_data_format;
    ptr->LDINTERFACE_Output.delay_format = ldspi_multi_format_unit_delay_reg.delay_format;

    ptr->LDINTERFACE_Repeat.hrepeat = ldspi_data_sram_special_ctrl_0_reg.hrepeat;
    ptr->LDINTERFACE_Repeat.vrepeat = ldspi_data_sram_special_ctrl_0_reg.vrepeat;
    ptr->LDINTERFACE_Repeat.vrepeat_width = ldspi_data_sram_special_ctrl_0_reg.vrepeat_width;

    ptr->LDINTERFACE_Reproduce.dv_reproduce_mode = ldspi_ld_spi_reproduce_mode_reg.dv_reproduce_mode;
    ptr->LDINTERFACE_Reproduce.ld_spi_dv_total = ldspi_ld_spi_v_total_reg.ld_spi_dv_total;
    ptr->LDINTERFACE_Reproduce.ld_spi_dh_total = ldspi_ld_spi_h_total_reg.ld_spi_dh_total;
    ptr->LDINTERFACE_Reproduce.dv_total_src_sel = ldspi_ld_spi_v_total_reg.dv_total_src_sel;
    ptr->LDINTERFACE_Reproduce.dv_total_sync_dtg = ldspi_ld_spi_v_total_reg.dv_total_sync_dtg;

    drvif_get_LdInterface_TimingReg(ptrReg);
    drvif_cal_LdInterface_TimingReal(&ptr->LDINTERFACE_TimingReal,ptrReg);
}

UINT8 drvif_get_LdInterface_CheckTriggerClear()
{
    ldspi_ld_ctrl_RBUS        ldspi_ld_ctrl_reg;
    ldspi_ld_ctrl_reg.regValue =    IoReg_Read32(LDSPI_LD_CTRL_reg);
    return (UINT8)ldspi_ld_ctrl_reg.send_trigger;
}

void drvif_set_LdInterface_BasicCtrl(DRV_LDINTERFACE_Basic_Ctrl *ptr)
{ 
    /*baisc*/
    ldspi_ld_ctrl_RBUS        ldspi_ld_ctrl_reg;
    ldspi_as_wr_ctrl_RBUS     ldspi_as_wr_ctrl_reg;
    //extra: timing delay	
    ldspi_ld_spi_ld_timing_fune_tune_RBUS ldspi_ld_spi_ld_timing_fune_tune_reg;
    ldspi_ld_spi_v_sync_delay_RBUS ld_spi_v_sync_delay_reg;	
    /*disable start enable*/
    // drvif_set_LdInterface_StartEn(0); 	//this will control in fw level
    /*Read Register*/
    ldspi_ld_ctrl_reg.regValue =    IoReg_Read32(LDSPI_LD_CTRL_reg);
    ldspi_as_wr_ctrl_reg.regValue = IoReg_Read32(LDSPI_AS_WR_CTRL_reg);
    ldspi_ld_spi_ld_timing_fune_tune_reg.regValue = IoReg_Read32(LDSPI_ld_spi_LD_timing_fune_tune_reg);
    ld_spi_v_sync_delay_reg.regValue = IoReg_Read32(LDSPI_ld_spi_V_sync_delay_reg);
    /*basic control*/
    //ldspi_ld_ctrl_reg.ld_spi_en = 1;
    //ldspi_ld_ctrl_reg.start_enable = 0;//disable start enable
    ldspi_ld_ctrl_reg.ld_mode   = ptr->ld_mode;
    ldspi_ld_ctrl_reg.as_led_reset   = ptr->as_led_reset;
    ldspi_ld_ctrl_reg.as_led_with_cs = ptr->as_led_with_cs;
    ldspi_ld_ctrl_reg.separate_two_block = ptr->separate_two_block;
    ldspi_ld_ctrl_reg.out_data_align = ptr->out_data_align;
    ldspi_ld_ctrl_reg.in_data_align = ptr->in_data_align;
    ld_spi_v_sync_delay_reg.vsync_delay = ptr->vsync_delay;
    /*basic control:send mode*/
    switch((_SEND_MODE)ptr->send_mode){
        case VSYNC_MODE:
            ldspi_ld_ctrl_reg.send_follow_vsync = 1;
            break;
        case TRIGGER_MODE:
            ldspi_ld_ctrl_reg.send_follow_vsync = 0;
            break;
        default:
            ldspi_ld_ctrl_reg.send_follow_vsync = 0;
            break;
    }

    /*basic control:send trigger */
    //ldspi_ld_ctrl_reg.send_trigger    = ptr->send_trigger;
	
    /*basic control:as mode:w_package_repeater & force tx mode*/
    ldspi_as_wr_ctrl_reg.w_package_repeater = ptr->w_package_repeater;
    if(ptr->as_force_tx){//as force tx mode
        ldspi_as_wr_ctrl_reg.force_txorrx_mode_en = 1;
    }
    //jira: RL6748-1927 ,add ld_backlight_timing_delay 0x7	
    ldspi_ld_spi_ld_timing_fune_tune_reg.ld_backlight_timing_delay = ptr->ld_backlight_timing_delay;	
	
    //IoReg_Write32(LDSPI_LD_CTRL_reg,ldspi_ld_ctrl_reg.regValue);//disable start_enable
    /*write register*/
    IoReg_Write32(LDSPI_LD_CTRL_reg,   ldspi_ld_ctrl_reg.regValue);
    IoReg_Write32(LDSPI_AS_WR_CTRL_reg,ldspi_as_wr_ctrl_reg.regValue);
    IoReg_Write32(LDSPI_ld_spi_V_sync_delay_reg,  ld_spi_v_sync_delay_reg.regValue);
    //jira: RL6748-1927 ,add ld_backlight_timing_delay 0x7	
    IoReg_Write32(LDSPI_ld_spi_LD_timing_fune_tune_reg,  ldspi_ld_spi_ld_timing_fune_tune_reg.regValue);
    /*after settings done,enable start enable*/
    //drvif_set_LdInterface_StartEn(1);  //this will control in fw level
}

void drvif_set_LdInterface_LdMode(UINT8 ld_mode)
{
    ldspi_ld_ctrl_RBUS        ldspi_ld_ctrl_reg;
    /*before settings,disable start enable*/
    //drvif_set_LdInterface_StartEn(0);
    /*change settings*/
    ldspi_ld_ctrl_reg.regValue = IoReg_Read32(LDSPI_LD_CTRL_reg);
    ldspi_ld_ctrl_reg.ld_mode  = ld_mode;
    IoReg_Write32(LDSPI_LD_CTRL_reg,ldspi_ld_ctrl_reg.regValue);//enable start_enable
    /*after settings done,enable start enable*/
    //drvif_set_LdInterface_StartEn(1);
}

/* add ldspi_double_width_ctrl_reg */
void drvif_set_LdInterface_Output(DRV_LDINTERFACE_Output *ptr)
{
    ldspi_out_data_ctrl_RBUS  ldspi_out_data_ctrl_reg;
    ldspi_out_data_form_RBUS  ldspi_out_data_form_reg;
    ldspi_double_width_ctrl_RBUS ldspi_double_width_ctrl_reg;
    ldspi_multi_format_unit_delay_RBUS ldspi_multi_format_unit_delay_reg;
    /*before settings,disable start enable*/
    //drvif_set_LdInterface_StartEn(0);
    /*update output info read*/
    ldspi_out_data_ctrl_reg.regValue = IoReg_Read32(LDSPI_Out_data_CTRL_reg);
    ldspi_out_data_form_reg.regValue = IoReg_Read32(LDSPI_Out_data_form_reg);
    ldspi_double_width_ctrl_reg.regValue = IoReg_Read32(LDSPI_double_width_ctrl_reg);
    ldspi_multi_format_unit_delay_reg.regValue = IoReg_Read32(LDSPI_Multi_format_unit_delay_reg);
    /*update output info point*/
    //ldspi_out_data_ctrl_reg.data_endian = 0; // why is it here?
    ldspi_out_data_ctrl_reg.header_format = ptr->header_format;
    ldspi_out_data_form_reg.header_units  = ptr->header_units;
    ldspi_out_data_ctrl_reg.output_data_format = ptr->output_data_format;
    ldspi_out_data_ctrl_reg.output_units  = ptr->output_units-1;
    ldspi_out_data_ctrl_reg.tailer_format = ptr->tailer_format;
    ldspi_out_data_form_reg.tailer_units  = ptr->tailer_units;
    ldspi_double_width_ctrl_reg.ld_total_data_num = ptr->ld_total_data_num;
    ldspi_multi_format_unit_delay_reg.delay_format = ptr->delay_format;
    ldspi_out_data_form_reg.data_sram_separate = ptr->data_sram_separate;
	
    /*update output info write*/
    IoReg_Write32(LDSPI_Out_data_CTRL_reg,ldspi_out_data_ctrl_reg.regValue);
    IoReg_Write32(LDSPI_Out_data_form_reg,ldspi_out_data_form_reg.regValue);
    IoReg_Write32(LDSPI_double_width_ctrl_reg,ldspi_double_width_ctrl_reg.regValue);
    IoReg_Write32(LDSPI_Multi_format_unit_delay_reg,ldspi_multi_format_unit_delay_reg.regValue);
    /*after settings done,enable start enable*/
    //drvif_set_LdInterface_StartEn(1);
}

void drvif_set_LdInterface_DataUnits(UINT32 data_unit)
{
    ldspi_out_data_ctrl_RBUS  ldspi_out_data_ctrl_reg;
    /*before settings,disable start enable*/
    //drvif_set_LdInterface_StartEn(0);
    /*update output unit*/
    ldspi_out_data_ctrl_reg.regValue = IoReg_Read32(LDSPI_Out_data_CTRL_reg);
    ldspi_out_data_ctrl_reg.output_units  = data_unit-1;
    IoReg_Write32(LDSPI_Out_data_CTRL_reg,ldspi_out_data_ctrl_reg.regValue);
    /*after settings done,enable start enable*/
    //drvif_set_LdInterface_StartEn(1);
}

void drvif_set_LdInterface_shift_swap(DRV_LDINTERFACE_SHIFT_SWAP *ptr)
{
    ldspi_data_sram_special_ctrl_0_RBUS ldspi_data_sram_special_ctrl_0_reg;
    ldspi_as_wr_ctrl_RBUS     ldspi_as_wr_ctrl_reg;
    /*before settings,disable start enable*/
    //drvif_set_LdInterface_StartEn(0);
    /*Read Register*/
    ldspi_data_sram_special_ctrl_0_reg.regValue = IoReg_Read32(LDSPI_Data_SRAM_SPECIAL_CTRL_0_reg);
    ldspi_as_wr_ctrl_reg.regValue = IoReg_Read32(LDSPI_AS_WR_CTRL_reg);
    /*LDINTERFACE_SHIFT_SWAP*/
    ldspi_data_sram_special_ctrl_0_reg.shift_op_wholesram = ptr->shift_op;
    ldspi_data_sram_special_ctrl_0_reg.data_sram_sft_ctrl = (ptr->shift_dir<<4)|(ptr->shift_num);
    ldspi_as_wr_ctrl_reg.swap_op_wholesram = ptr->swap_op;
    ldspi_as_wr_ctrl_reg.swap_of_16_bit = ptr->swap_en;
    /*write register*/
    IoReg_Write32(LDSPI_Data_SRAM_SPECIAL_CTRL_0_reg,   ldspi_data_sram_special_ctrl_0_reg.regValue);
    IoReg_Write32(LDSPI_AS_WR_CTRL_reg,   ldspi_as_wr_ctrl_reg.regValue);
    /*after settings done,enable start enable*/
    //drvif_set_LdInterface_StartEn(1);
}

void drvif_set_LdInterface_Read(DRV_LDINTERFACE_Read *ptr)
{
    ldspi_rx_timing_ctrl_RBUS ldspi_rx_timing_ctrl_reg;
    ldspi_as_wr_ctrl_RBUS     ldspi_as_wr_ctrl_reg;
    ldspi_rx_data_ctrl_RBUS   ldspi_rx_data_ctrl_reg;
    /*before settings,disable start enable*/
    drvif_set_LdInterface_StartEn(0);
    /*Read Registers*/
    ldspi_rx_timing_ctrl_reg.regValue = IoReg_Read32(LDSPI_Rx_timing_ctrl_reg);
    ldspi_as_wr_ctrl_reg.regValue = IoReg_Read32(LDSPI_AS_WR_CTRL_reg);
    ldspi_rx_data_ctrl_reg.regValue = IoReg_Read32(LDSPI_RX_data_CTRL_reg);
    /*settings*/
    ldspi_rx_timing_ctrl_reg.rx_data_shift = ptr->rx_data_shift;
    ldspi_as_wr_ctrl_reg.read_after_n_byte = ptr->read_after_n_byte;
    ldspi_as_wr_ctrl_reg.rw_cmd_byte = ptr->rw_cmd_byte;
    ldspi_as_wr_ctrl_reg.sram_rwcmd_bit = ptr->sram_rwcmd_bit;
    ldspi_as_wr_ctrl_reg.force_txorrx_mode_en = ptr->forceTxorRx_mode_en;
    ldspi_as_wr_ctrl_reg.force_txorrx_mode = ptr->forceTxorRx_mode;
    ldspi_as_wr_ctrl_reg.rw_bit_sent_sel = ptr->rw_bit_send_sel;
    ldspi_as_wr_ctrl_reg.rw_cmd_sel = ptr->rw_cmd_sel;
    ldspi_as_wr_ctrl_reg.rwcmd = ptr->rwcmd;
    ldspi_rx_data_ctrl_reg.rx_units = ptr->rx_units;
    ldspi_rx_data_ctrl_reg.rx_data_format = ptr->rx_data_format;
    ldspi_rx_data_ctrl_reg.time_out_period = ptr->time_out_period;
    /*Write Registers*/
    IoReg_Write32(LDSPI_Rx_timing_ctrl_reg,ldspi_rx_timing_ctrl_reg.regValue);
    IoReg_Write32(LDSPI_AS_WR_CTRL_reg,ldspi_as_wr_ctrl_reg.regValue);
    IoReg_Write32(LDSPI_RX_data_CTRL_reg,ldspi_rx_data_ctrl_reg.regValue);
    /*after settings done,enable start enable*/
    drvif_set_LdInterface_StartEn(1);
}

void drvif_set_LdInterface_Inv(DRV_LDINTERFACE_INV *ptr)
{
    ldspi_ld_ctrl_RBUS        ldspi_ld_ctrl_reg;
    /*before settings,disable start enable*/
    //drvif_set_LdInterface_StartEn(0);
    /*Read Register*/
    ldspi_ld_ctrl_reg.regValue =    IoReg_Read32(LDSPI_LD_CTRL_reg);
    /*DRV_LDINTERFACE_INV*/
    ldspi_ld_ctrl_reg.tx_clk_inv = ptr->tx_clk_inv;
    ldspi_ld_ctrl_reg.tx_cs_inv  = ptr->tx_cs_inv;
    ldspi_ld_ctrl_reg.tx_sdo_inv = ptr->tx_sdo_inv;
    ldspi_ld_ctrl_reg.tx_vsync_inv = ptr->tx_vsync_inv;
    //ldspi_ld_ctrl_reg.start_enable = 0;
    /*write register*/
    IoReg_Write32(LDSPI_LD_CTRL_reg,   ldspi_ld_ctrl_reg.regValue);
    /*after settings done,enable start enable*/
    //drvif_set_LdInterface_StartEn(1);
}

void drvif_set_LdInterface_Repeat(DRV_LDINTERFACE_Repeat *ptr)
{
    ldspi_data_sram_special_ctrl_0_RBUS ldspi_data_sram_special_ctrl_0_reg;
    /*before settings,disable start enable*/
    //drvif_set_LdInterface_StartEn(0);
    /*Read Register*/
    ldspi_data_sram_special_ctrl_0_reg.regValue = IoReg_Read32(LDSPI_Data_SRAM_SPECIAL_CTRL_0_reg);
    /*LDINTERFACE_Repeat*/
    ldspi_data_sram_special_ctrl_0_reg.hrepeat = ptr->hrepeat;
    ldspi_data_sram_special_ctrl_0_reg.vrepeat = ptr->vrepeat;
    ldspi_data_sram_special_ctrl_0_reg.vrepeat_width = ptr->vrepeat_width;
    /*write register*/
    IoReg_Write32(LDSPI_Data_SRAM_SPECIAL_CTRL_0_reg,   ldspi_data_sram_special_ctrl_0_reg.regValue);
    /*after settings done,enable start enable*/
    //drvif_set_LdInterface_StartEn(1);
}


void drvif_set_LdInterface_Reproduce(DRV_LDINTERFACE_Reproduce *ptr)
{
    ldspi_ld_spi_v_total_RBUS ldspi_ld_spi_v_total_reg;
    ldspi_ld_spi_h_total_RBUS ldspi_ld_spi_h_total_reg;
    ldspi_ld_spi_reproduce_mode_RBUS ldspi_ld_spi_reproduce_mode_reg;
    /*before settings,disable start enable*/
    //drvif_set_LdInterface_StartEn(0);
    /*Read Register*/
    ldspi_ld_spi_v_total_reg.regValue = IoReg_Read32(LDSPI_ld_spi_v_total_reg);
    ldspi_ld_spi_h_total_reg.regValue = IoReg_Read32(LDSPI_ld_spi_h_total_reg);
    ldspi_ld_spi_reproduce_mode_reg.regValue = IoReg_Read32(LDSPI_ld_spi_reproduce_mode_reg);
    /*LDINTERFACE_Reproduce*/
    ldspi_ld_spi_v_total_reg.dv_total_src_sel = ptr->dv_total_src_sel;
    ldspi_ld_spi_v_total_reg.dv_total_sync_dtg= ptr->dv_total_sync_dtg;
    ldspi_ld_spi_v_total_reg.ld_spi_dv_total  = ptr->ld_spi_dv_total;
    ldspi_ld_spi_h_total_reg.ld_spi_dh_total  = ptr->ld_spi_dh_total;
    ldspi_ld_spi_reproduce_mode_reg.dv_reproduce_mode = ptr->dv_reproduce_mode;
    /*write register*/
    IoReg_Write32(LDSPI_ld_spi_v_total_reg,ldspi_ld_spi_v_total_reg.regValue);
    IoReg_Write32(LDSPI_ld_spi_h_total_reg,ldspi_ld_spi_h_total_reg.regValue);
    IoReg_Write32(LDSPI_ld_spi_reproduce_mode_reg,   ldspi_ld_spi_reproduce_mode_reg.regValue);
    /*after settings done,enable start enable*/
    //drvif_set_LdInterface_StartEn(1);
}


void drvif_set_LdInterface_IntEn(DRV_LDINTERFACE_IntEn *ptr)
{
    ldspi_rx_data_ctrl_RBUS   ldspi_rx_data_ctrl_reg;
    /*before settings,disable start enable*/
    drvif_set_LdInterface_StartEn(0);
    /*Read Register*/
    ldspi_rx_data_ctrl_reg.regValue =  IoReg_Read32(LDSPI_RX_data_CTRL_reg);
    /*LDINTERFACE_IntEn*/
    ldspi_rx_data_ctrl_reg.ld_spi_ints_en = ptr->spi_intsen;
    ldspi_rx_data_ctrl_reg.ld_spi_intv_en = ptr->spi_intven;
    ldspi_rx_data_ctrl_reg.tx_done_int_en = ptr->tx_done_inten;
    ldspi_rx_data_ctrl_reg.rx_done_int_en = ptr->rx_done_inten;
    /*write register*/
    IoReg_Write32(LDSPI_RX_data_CTRL_reg,      ldspi_rx_data_ctrl_reg.regValue);
    /*after settings done,enable start enable*/
    drvif_set_LdInterface_StartEn(1);
}


void drvif_set_LdInterface_TimingReal(DRV_LDINTERFACE_TimingReal *ptr)
{
    DRV_LDINTERFACE_TimingReg LDINTERFACE_TimingReg = {0,0,0,0,0,0,0,0,0};
    /*before settings,disable start enable*/
    //drvif_set_LdInterface_StartEn(0);

    /*DRV_LDINTERFACE_TimingReal:calculate*/
    drvif_cal_LdInterface_TimingReg(ptr,&LDINTERFACE_TimingReg);
    drvif_set_LdInterface_TimingReg(&LDINTERFACE_TimingReg);
    /*after settings done,enable start enable*/
    //drvif_set_LdInterface_StartEn(1);
}


void drvif_set_LdInterface_TimingReg(DRV_LDINTERFACE_TimingReg *ptrReg)
{
    //timing related
    ldspi_send_delay_RBUS  ldspi_send_delay_reg;
    ldspi_data_unit_delay_RBUS ldspi_data_unit_delay_reg;
    ldspi_multi_format_unit_delay_RBUS ldspi_multi_format_unit_delay_reg;
    ldspi_v_sync_duty_RBUS  ldspi_v_sync_duty_reg;
    ldspi_data_h_time_RBUS  ldspi_data_h_time_reg;
    ldspi_cs_hold_time_RBUS  ldspi_cs_hold_time_reg;
    ldspi_sck_hold_time_RBUS ldspi_sck_hold_time_reg;
    ldspi_timing_ctrl_RBUS   ldspi_timing_ctrl_reg;
    /*before settings,disable start enable*/
    //drvif_set_LdInterface_StartEn(0);
    //Timing related
    ldspi_send_delay_reg.regValue = IoReg_Read32(LDSPI_Send_delay_reg);
    ldspi_data_unit_delay_reg.regValue = IoReg_Read32(LDSPI_Data_unit_delay_reg);
    ldspi_multi_format_unit_delay_reg.regValue = IoReg_Read32(LDSPI_Multi_format_unit_delay_reg);
    ldspi_v_sync_duty_reg.regValue = IoReg_Read32(LDSPI_V_sync_duty_reg);
    ldspi_data_h_time_reg.regValue = IoReg_Read32(LDSPI_Data_h_time_reg);
    ldspi_cs_hold_time_reg.regValue = IoReg_Read32(LDSPI_Cs_hold_time_reg);
    ldspi_sck_hold_time_reg.regValue = IoReg_Read32(LDSPI_sck_hold_time_reg);
    ldspi_timing_ctrl_reg.regValue = IoReg_Read32(LDSPI_Timing_CTRL_reg);
    /*DRV_LDINTERFACE_TimingReal:set register*/
    ldspi_send_delay_reg.data_send_delay = ptrReg->data_send_delay;
    ldspi_data_unit_delay_reg.each_unit_delay = ptrReg->each_unit_delay;
    ldspi_multi_format_unit_delay_reg.multi_unit_delay = ptrReg->multi_unit_delay;
    ldspi_v_sync_duty_reg.vsync_d = ptrReg->vsync_d;
    ldspi_data_h_time_reg.cs_end_delay_time = ptrReg->cs_end_delay_time;
    ldspi_cs_hold_time_reg.cs_hold_time = ptrReg->cs_hold_time;
    ldspi_sck_hold_time_reg.sck_hold_time = ptrReg->sck_hold_time;
    ldspi_timing_ctrl_reg.sck_h = ptrReg->sck_h;
    ldspi_timing_ctrl_reg.sck_l = ptrReg->sck_l;
    /*write timing related*/
    IoReg_Write32(LDSPI_Send_delay_reg,   ldspi_send_delay_reg.regValue);
    IoReg_Write32(LDSPI_Data_unit_delay_reg,ldspi_data_unit_delay_reg.regValue);
    IoReg_Write32(LDSPI_Multi_format_unit_delay_reg,ldspi_multi_format_unit_delay_reg.regValue);
    IoReg_Write32(LDSPI_V_sync_duty_reg,  ldspi_v_sync_duty_reg.regValue);
    IoReg_Write32(LDSPI_Data_h_time_reg,  ldspi_data_h_time_reg.regValue);
    IoReg_Write32(LDSPI_Cs_hold_time_reg, ldspi_cs_hold_time_reg.regValue);
    IoReg_Write32(LDSPI_sck_hold_time_reg,ldspi_sck_hold_time_reg.regValue);
    IoReg_Write32(LDSPI_Timing_CTRL_reg,  ldspi_timing_ctrl_reg.regValue);

    /*after settings done,enable start enable*/
    //drvif_set_LdInterface_StartEn(1);
}

void drvif_LdInterface_Read_Clear()
{
    /*Clear rx fifo. Do rx clear after finish get rx datas.*/
     ldspi_rx_data_ctrl_RBUS   ldspi_rx_data_ctrl_rbus;
     ldspi_wclr_ctrl_RBUS ldspi_wclr_ctrl_reg;
     /*read registers*/
     ldspi_rx_data_ctrl_rbus.regValue = IoReg_Read32(LDSPI_RX_data_CTRL_reg);
     ldspi_wclr_ctrl_reg.regValue = IoReg_Read32(LDSPI_WCLR_CTRL_reg);
     /*wclr rx_units and rx_done*/
     ldspi_rx_data_ctrl_rbus.read_data_clr = 1;
     ldspi_wclr_ctrl_reg.rx_done = 1;
     /*write registers*/
     IoReg_Write32(LDSPI_RX_data_CTRL_reg,ldspi_rx_data_ctrl_rbus.regValue);
     IoReg_Write32(LDSPI_WCLR_CTRL_reg,ldspi_wclr_ctrl_reg.regValue);
}

void drvif_LdInterface_Status_Clear()
{
    ldspi_wclr_ctrl_RBUS ldspi_wclr_ctrl_reg;
    ldspi_ld_spi_ld_timing_error_RBUS ldspi_ld_spi_ld_timing_error_reg;
    ldspi_ld_spi_ld_spi_timing_error_RBUS  ldspi_ld_spi_ld_spi_timing_error_reg;
    /*wclr*/
    ldspi_ld_spi_ld_timing_error_reg.regValue = 0;
    ldspi_ld_spi_ld_spi_timing_error_reg.regValue = 0;
    ldspi_wclr_ctrl_reg.regValue = 0xffffffff;
    ldspi_ld_spi_ld_timing_error_reg.ld_timing_error = 1;
    ldspi_ld_spi_ld_spi_timing_error_reg.ld_spi_timing_error = 1;
    /*write register*/
    IoReg_Write32(LDSPI_WCLR_CTRL_reg,ldspi_wclr_ctrl_reg.regValue);
    IoReg_Write32(LDSPI_ld_spi_LD_Timing_Error_reg,ldspi_ld_spi_ld_timing_error_reg.regValue);
    IoReg_Write32(LDSPI_ld_spi_LD_SPI_Timing_Error_reg,ldspi_ld_spi_ld_spi_timing_error_reg.regValue);
}

UINT8 drvif_get_LdInterface_Enable()
{
    ldspi_ld_ctrl_RBUS        ldspi_ld_ctrl_reg;
    /*disable start enable*/
    ldspi_ld_ctrl_reg.regValue =    IoReg_Read32(LDSPI_LD_CTRL_reg);
    if(ldspi_ld_ctrl_reg.start_enable&&ldspi_ld_ctrl_reg.ld_spi_en)
        return 1;
    else
        return 0;
}

UINT8 drvif_get_LdInterface_rxdone()
{
    ldspi_wclr_ctrl_RBUS ldspi_wclr_ctrl_reg;
    ldspi_wclr_ctrl_reg.regValue = IoReg_Read32(LDSPI_WCLR_CTRL_reg);
    return ldspi_wclr_ctrl_reg.rx_done;
}

UINT8 *drvif_get_LdInterface_RxUnits(int rxcnt)
{
    UINT8* p_rx_units;
    ldspi_rx_dataw0_RBUS ldspi_rx_dataw0_reg;
    int size = 32;
    int i;
    p_rx_units = (UINT8*)malloc(sizeof(UINT8)*size);
    memset(p_rx_units,0,32);
    i = 0;
    while(i<rxcnt){
        ldspi_rx_dataw0_reg.regValue =    IoReg_Read32(LDSPI_RX_dataW0_reg + 4*i);
        p_rx_units[i] = ldspi_rx_dataw0_reg.input_data_byte0;
        p_rx_units[i+1] = ldspi_rx_dataw0_reg.input_data_byte1;
        p_rx_units[i+2] = ldspi_rx_dataw0_reg.input_data_byte2;
        p_rx_units[i+3] = ldspi_rx_dataw0_reg.input_data_byte3;
        i += 4;
    }
    return p_rx_units;
}

void drvif_get_LdInterface_Output(DRV_LDINTERFACE_Output *ptrReg)
{
    ldspi_out_data_ctrl_RBUS  ldspi_out_data_ctrl_reg;
    ldspi_out_data_form_RBUS  ldspi_out_data_form_reg;
    ldspi_double_width_ctrl_RBUS ldspi_double_width_ctrl_reg;
    ldspi_multi_format_unit_delay_RBUS ldspi_multi_format_unit_delay_reg;
    /*grep output info read*/
    ldspi_out_data_ctrl_reg.regValue = IoReg_Read32(LDSPI_Out_data_CTRL_reg);
    ldspi_out_data_form_reg.regValue = IoReg_Read32(LDSPI_Out_data_form_reg);
    ldspi_double_width_ctrl_reg.regValue = IoReg_Read32(LDSPI_double_width_ctrl_reg);
    ldspi_multi_format_unit_delay_reg.regValue = IoReg_Read32(LDSPI_Data_unit_delay_reg);
    /*update output info point*/
    //ldspi_out_data_ctrl_reg.data_endian = 0; 
    ptrReg->header_format = ldspi_out_data_ctrl_reg.header_format;
    ptrReg->header_units = ldspi_out_data_form_reg.header_units;
    ptrReg->output_data_format = ldspi_out_data_ctrl_reg.output_data_format;
    ptrReg->output_units = ldspi_out_data_ctrl_reg.output_units+1;
    ptrReg->tailer_format = ldspi_out_data_ctrl_reg.tailer_format;
    ptrReg->tailer_units = ldspi_out_data_form_reg.tailer_units;
    ptrReg->ld_total_data_num = ldspi_double_width_ctrl_reg.ld_total_data_num;
    ptrReg->delay_format= ldspi_multi_format_unit_delay_reg.delay_format;
    ptrReg->data_sram_separate = ldspi_out_data_form_reg.data_sram_separate;
	
}

void drvif_get_LdInterface_Inv(DRV_LDINTERFACE_INV *ptrReg)
{
    ldspi_ld_ctrl_RBUS        ldspi_ld_ctrl_reg;
    /*Read Register*/
    ldspi_ld_ctrl_reg.regValue =    IoReg_Read32(LDSPI_LD_CTRL_reg);
    /*DRV_LDINTERFACE_INV*/
    ptrReg->tx_clk_inv = ldspi_ld_ctrl_reg.tx_clk_inv;
    ptrReg->tx_cs_inv = ldspi_ld_ctrl_reg.tx_cs_inv;
    ptrReg->tx_sdo_inv = ldspi_ld_ctrl_reg.tx_sdo_inv;
    ptrReg->tx_vsync_inv = ldspi_ld_ctrl_reg.tx_vsync_inv;

}

void drvif_get_LdInterface_Repeat(DRV_LDINTERFACE_Repeat *ptrReg)
{
    ldspi_data_sram_special_ctrl_0_RBUS ldspi_data_sram_special_ctrl_0_reg;

    /*Read Register*/
    ldspi_data_sram_special_ctrl_0_reg.regValue = IoReg_Read32(LDSPI_Data_SRAM_SPECIAL_CTRL_0_reg);
    /*LDINTERFACE_Repeat*/
    ptrReg->hrepeat = ldspi_data_sram_special_ctrl_0_reg.hrepeat;
    ptrReg->vrepeat = ldspi_data_sram_special_ctrl_0_reg.vrepeat;
    ptrReg->vrepeat_width = ldspi_data_sram_special_ctrl_0_reg.vrepeat_width;

}

void drvif_get_LdInterface_Reproduce(DRV_LDINTERFACE_Reproduce *ptrReg)
{
    ldspi_ld_spi_v_total_RBUS ldspi_ld_spi_v_total_reg;
    ldspi_ld_spi_h_total_RBUS ldspi_ld_spi_h_total_reg;
    ldspi_ld_spi_reproduce_mode_RBUS ldspi_ld_spi_reproduce_mode_reg;

    /*Read Register*/
    ldspi_ld_spi_v_total_reg.regValue = IoReg_Read32(LDSPI_ld_spi_v_total_reg);
    ldspi_ld_spi_h_total_reg.regValue = IoReg_Read32(LDSPI_ld_spi_h_total_reg);
    ldspi_ld_spi_reproduce_mode_reg.regValue = IoReg_Read32(LDSPI_ld_spi_reproduce_mode_reg);
	
    /*LDINTERFACE_Reproduce*/
    ptrReg->dv_total_src_sel = 			ldspi_ld_spi_v_total_reg.dv_total_src_sel;
    ptrReg->dv_total_sync_dtg = 		ldspi_ld_spi_v_total_reg.dv_total_sync_dtg;
    ptrReg->ld_spi_dv_total      = ldspi_ld_spi_v_total_reg.ld_spi_dv_total;
    ptrReg->ld_spi_dh_total   = ldspi_ld_spi_h_total_reg.ld_spi_dh_total;
    ptrReg->dv_reproduce_mode   = ldspi_ld_spi_reproduce_mode_reg.dv_reproduce_mode;
}

void drvif_get_LdInterface_TimingReg(DRV_LDINTERFACE_TimingReg *ptrReg)
{
    ldspi_send_delay_RBUS  ldspi_send_delay_reg;
    ldspi_data_unit_delay_RBUS ldspi_data_unit_delay_reg;
    ldspi_multi_format_unit_delay_RBUS ldspi_multi_format_unit_delay_reg;
    ldspi_v_sync_duty_RBUS  ldspi_v_sync_duty_reg;
    ldspi_data_h_time_RBUS  ldspi_data_h_time_reg;
    ldspi_cs_hold_time_RBUS  ldspi_cs_hold_time_reg;
    ldspi_sck_hold_time_RBUS ldspi_sck_hold_time_reg;
    ldspi_timing_ctrl_RBUS   ldspi_timing_ctrl_reg;
	
    ldspi_send_delay_reg.regValue = IoReg_Read32(LDSPI_Send_delay_reg);
    ldspi_data_unit_delay_reg.regValue = IoReg_Read32(LDSPI_Data_unit_delay_reg);
    ldspi_multi_format_unit_delay_reg.regValue = IoReg_Read32(LDSPI_Multi_format_unit_delay_reg);
    ldspi_v_sync_duty_reg.regValue = IoReg_Read32(LDSPI_V_sync_duty_reg);
    ldspi_data_h_time_reg.regValue = IoReg_Read32(LDSPI_Data_h_time_reg);
    ldspi_cs_hold_time_reg.regValue = IoReg_Read32(LDSPI_Cs_hold_time_reg);
    ldspi_sck_hold_time_reg.regValue = IoReg_Read32(LDSPI_sck_hold_time_reg);
    ldspi_timing_ctrl_reg.regValue = IoReg_Read32(LDSPI_Timing_CTRL_reg);
	
    ptrReg->cs_end_delay_time = ldspi_data_h_time_reg.cs_end_delay_time;
    ptrReg->sck_hold_time = 		ldspi_sck_hold_time_reg.sck_hold_time;
    ptrReg->cs_hold_time      = ldspi_cs_hold_time_reg.cs_hold_time;
    ptrReg->data_send_delay   = ldspi_send_delay_reg.data_send_delay;
    ptrReg->each_unit_delay   = ldspi_data_unit_delay_reg.each_unit_delay;
    ptrReg->multi_unit_delay  = ldspi_multi_format_unit_delay_reg.multi_unit_delay;
    ptrReg->sck_h             = ldspi_timing_ctrl_reg.sck_h;
    ptrReg->sck_l             = ldspi_timing_ctrl_reg.sck_l;
    ptrReg->vsync_d           = ldspi_v_sync_duty_reg.vsync_d;
}

void drvif_get_LdInterface_TimingReal(DRV_LDINTERFACE_TimingReal *ptrReal)
{
    ldspi_send_delay_RBUS  ldspi_send_delay_reg;
    ldspi_data_unit_delay_RBUS ldspi_data_unit_delay_reg;
    ldspi_multi_format_unit_delay_RBUS ldspi_multi_format_unit_delay_reg;
    ldspi_v_sync_duty_RBUS  ldspi_v_sync_duty_reg;
    ldspi_data_h_time_RBUS  ldspi_data_h_time_reg;
    ldspi_cs_hold_time_RBUS  ldspi_cs_hold_time_reg;
    ldspi_sck_hold_time_RBUS ldspi_sck_hold_time_reg;
    ldspi_timing_ctrl_RBUS   ldspi_timing_ctrl_reg;
    DRV_LDINTERFACE_TimingReg LDINTERFACE_TimingReg = {0,0,0,0,0,0,0,0,0};
	
    ldspi_send_delay_reg.regValue = IoReg_Read32(LDSPI_Send_delay_reg);
    ldspi_data_unit_delay_reg.regValue = IoReg_Read32(LDSPI_Data_unit_delay_reg);
    ldspi_multi_format_unit_delay_reg.regValue = IoReg_Read32(LDSPI_Data_unit_delay_reg);
    ldspi_v_sync_duty_reg.regValue = IoReg_Read32(LDSPI_V_sync_duty_reg);
    ldspi_data_h_time_reg.regValue = IoReg_Read32(LDSPI_Data_h_time_reg);
    ldspi_cs_hold_time_reg.regValue = IoReg_Read32(LDSPI_Cs_hold_time_reg);
    ldspi_sck_hold_time_reg.regValue = IoReg_Read32(LDSPI_sck_hold_time_reg);
    ldspi_timing_ctrl_reg.regValue = IoReg_Read32(LDSPI_Timing_CTRL_reg);	
    LDINTERFACE_TimingReg.cs_end_delay_time = ldspi_data_h_time_reg.cs_end_delay_time;
    LDINTERFACE_TimingReg.cs_hold_time      = ldspi_cs_hold_time_reg.cs_hold_time;
    LDINTERFACE_TimingReg.data_send_delay   = ldspi_send_delay_reg.data_send_delay;
    LDINTERFACE_TimingReg.each_unit_delay   = ldspi_data_unit_delay_reg.each_unit_delay;
    LDINTERFACE_TimingReg.multi_unit_delay  = ldspi_multi_format_unit_delay_reg.multi_unit_delay;
    LDINTERFACE_TimingReg.sck_h             = ldspi_timing_ctrl_reg.sck_h;
    LDINTERFACE_TimingReg.sck_l             = ldspi_timing_ctrl_reg.sck_l;
    LDINTERFACE_TimingReg.vsync_d           = ldspi_v_sync_duty_reg.vsync_d;
    drvif_cal_LdInterface_TimingReal(ptrReal,&LDINTERFACE_TimingReg);
}


void drvif_get_LdInterface_Status(DRV_LDINTERFACE_Status *ptr)
{
    ldspi_wclr_ctrl_RBUS ldspi_wclr_ctrl_reg;
    ldspi_ld_spi_ld_timing_error_RBUS ldspi_ld_spi_ld_timing_error_reg;
    ldspi_ld_spi_ld_spi_timing_error_RBUS  ldspi_ld_spi_ld_spi_timing_error_reg;

    ldspi_wclr_ctrl_reg.regValue = IoReg_Read32(LDSPI_WCLR_CTRL_reg);
    ldspi_ld_spi_ld_timing_error_reg.regValue = IoReg_Read32(LDSPI_ld_spi_LD_Timing_Error_reg);
    ldspi_ld_spi_ld_spi_timing_error_reg.regValue = IoReg_Read32(LDSPI_ld_spi_LD_SPI_Timing_Error_reg);

    ptr->rx_done             = ldspi_wclr_ctrl_reg.rx_done;
    ptr->rx_time_out_pending = ldspi_wclr_ctrl_reg.rx_time_out_pending;
    ptr->tx_done             = ldspi_wclr_ctrl_reg.tx_done;
    ptr->cmd_error_status    = ldspi_wclr_ctrl_reg.cmd_error_status;
    ptr->ld_timing_error  = ldspi_ld_spi_ld_timing_error_reg.ld_timing_error;
    ptr->ld_spi_timing_Error = ldspi_ld_spi_ld_spi_timing_error_reg.ld_spi_timing_error;
}


void printTableInfo(DRV_LDINTERFACE_Table *ptr)
{
    printk(KERN_EMERG "LDSPI:PRINT Table Info\n");
    printk(KERN_EMERG "BASIC:%d %d %d %d %d %d %d %d %d %d\n",\
           (int)ptr->LDINTERFACE_Basic_Ctrl.ld_mode,\
           (int)ptr->LDINTERFACE_Basic_Ctrl.separate_two_block,\
           (int)ptr->LDINTERFACE_Basic_Ctrl.send_mode,\
           (int)ptr->LDINTERFACE_Basic_Ctrl.send_trigger,\
           (int)ptr->LDINTERFACE_Basic_Ctrl.out_data_align,\
           (int)ptr->LDINTERFACE_Basic_Ctrl.in_data_align,\
           (int)ptr->LDINTERFACE_Basic_Ctrl.as_led_with_cs,\
           (int)ptr->LDINTERFACE_Basic_Ctrl.as_led_reset,\
           (int)ptr->LDINTERFACE_Basic_Ctrl.as_force_tx,\
           (int)ptr->LDINTERFACE_Basic_Ctrl.w_package_repeater);
    printk(KERN_EMERG "Output:%d %d %d %d %d %d %d %d\n",\
           (int)ptr->LDINTERFACE_Output.output_units,\
           (int)ptr->LDINTERFACE_Output.output_data_format,\
           (int)ptr->LDINTERFACE_Output.tailer_units,\
           (int)ptr->LDINTERFACE_Output.tailer_format,\
           (int)ptr->LDINTERFACE_Output.header_units,\
           (int)ptr->LDINTERFACE_Output.header_format,\
           (int)ptr->LDINTERFACE_Output.delay_format,\
           (int)ptr->LDINTERFACE_Output.data_sram_separate);
    printk(KERN_EMERG "SHIFT_SWAP:%d %d %d %d %d\n",\
           (int)ptr->LDINTERFACE_Shift_Swap.shift_op,\
           (int)ptr->LDINTERFACE_Shift_Swap.shift_dir,\
           (int)ptr->LDINTERFACE_Shift_Swap.shift_num,\
           (int)ptr->LDINTERFACE_Shift_Swap.swap_op,\
           (int)ptr->LDINTERFACE_Shift_Swap.swap_en);
    printk(KERN_EMERG "INV:%d %d %d %d\n",\
           (int)ptr->LDINTERFACE_Inv.tx_sdo_inv,\
           (int)ptr->LDINTERFACE_Inv.tx_cs_inv,\
           (int)ptr->LDINTERFACE_Inv.tx_clk_inv,\
           (int)ptr->LDINTERFACE_Inv.tx_vsync_inv);
    printk(KERN_EMERG "Repeat:%d %d %d\n",\
           (int)ptr->LDINTERFACE_Repeat.vrepeat_width,\
           (int)ptr->LDINTERFACE_Repeat.vrepeat,\
           (int)ptr->LDINTERFACE_Repeat.hrepeat);
    printk(KERN_EMERG "Reproduce:%d %d %d %d %d\n",\
           (int)ptr->LDINTERFACE_Reproduce.dv_reproduce_mode,\
           (int)ptr->LDINTERFACE_Reproduce.ld_spi_dh_total,\
           (int)ptr->LDINTERFACE_Reproduce.ld_spi_dv_total,\
           (int)ptr->LDINTERFACE_Reproduce.dv_total_src_sel,\
           (int)ptr->LDINTERFACE_Reproduce.dv_total_sync_dtg);
    printk(KERN_EMERG "Timing:%d %d %d %d %d %d %d %d %d %d \n",\
           (int)ptr->LDINTERFACE_TimingReal.T1,\
           (int)ptr->LDINTERFACE_TimingReal.T1_star,\
           (int)ptr->LDINTERFACE_TimingReal.T2,\
           (int)ptr->LDINTERFACE_TimingReal.T3,\
           (int)ptr->LDINTERFACE_TimingReal.T4,\
           (int)ptr->LDINTERFACE_TimingReal.T5,\
           (int)ptr->LDINTERFACE_TimingReal.T6,\
           (int)ptr->LDINTERFACE_TimingReal.T7,\
           (int)ptr->LDINTERFACE_TimingReal.cs_end,\
           (int)ptr->LDINTERFACE_TimingReal.fMultiUnit);
}

void drvif_LdInterface_Test(void)
{

#if 1 // Test Set Table => Pass
    extern DRV_LDINTERFACE_Table LDINTERFACE_Table[0];
    drvif_set_LdInterface_Table(&LDINTERFACE_Table[0]);
#endif

#if 0 //Test Cal Reg2Timing Info =>pass (Timing to Reg calculate in set table, tesed)
    extern DRV_LDINTERFACE_Table LDINTERFACE_Table[0];
    DRV_LDINTERFACE_Table* ptrTable;
    DRV_LDINTERFACE_TimingReg LDINTERFACE_TimingReg1 = {0x4a9,0x4a9,0x28,0x29,0x4a9,0x10b,0,0x3a,0x2ee};
    DRV_LDINTERFACE_TimingReal LDINTERFACE_TimingReal1 = {0,0,0,0,0,0,0,0,0};
    ptrTable = &LDINTERFACE_Table[0];
    drvif_cal_LdInterface_TimingReal(&LDINTERFACE_TimingReal1,&LDINTERFACE_TimingReg1);
    ptrTable->LDINTERFACE_TimingReal = LDINTERFACE_TimingReal1;
    drvif_set_LdInterface_Table(&LDINTERFACE_Table[0]);
#endif

}


bool check_LDSPI_IndexSRAM_status_NULL(DRV_LD_LDSPI_INDEXSRAM_TYPE *LDSPI_IndexSRAM){
	/*Null Protection*/
	if (LDSPI_IndexSRAM == NULL) {
		rtd_pr_vpq_emerg("drvif_color_set_LDInterface_SPI_IndexSRAM_Data_Continuous, LDSPI_IndexSRAM is NULL\n");
		return true;
	}
	if (LDSPI_IndexSRAM->SRAM_Value == NULL) {
		rtd_pr_vpq_emerg("drvif_color_set_LDInterface_SPI_IndexSRAM_Data_Continuous, LDSPI_IndexSRAM->SRAM_Value is NULL\n");
		return true;
	}

	if ((LDSPI_IndexSRAM->SRAM_Position+LDSPI_IndexSRAM->SRAM_Length)>1072) {
		rtd_pr_vpq_emerg("drvif_color_set_LDInterface_SPI_IndexSRAM_Data_Continuous, Data too much\n");
		return true;
	}
	rtd_pr_vpq_emerg("check_LDSPI_IndexSRAM_status_NULL done\n");
	return false;
	
}

void drvif_color_set_LDInterface_SPI_IndexSRAM_Data_Continuous(DRV_LD_LDSPI_INDEXSRAM_TYPE *LDSPI_IndexSRAM, unsigned char Wait_Tx_Done)
{
	unsigned short i;

	unsigned int timeoutcnt = 0x32500;
	//unsigned int dummy;
	unsigned char tx_done;
	
	ldspi_outindex_addr_ctrl_RBUS ldspi_outindex_addr_ctrl;
	ldspi_outindex_rwport_RBUS ldspi_outindex_rwport;
	ldspi_wclr_ctrl_RBUS ldspi_wclr_ctrl;

	/*Null Protection*/
/*	if (LDSPI_IndexSRAM == NULL) {
		rtd_pr_vpq_emerg("drvif_color_set_LDInterface_SPI_IndexSRAM_Data_Continuous, LDSPI_IndexSRAM is NULL\n");
		return;
	}
	if (LDSPI_IndexSRAM->SRAM_Value == NULL) {
		rtd_pr_vpq_emerg("drvif_color_set_LDInterface_SPI_IndexSRAM_Data_Continuous, LDSPI_IndexSRAM->SRAM_Value is NULL\n");
		return;
	}

	if ((LDSPI_IndexSRAM->SRAM_Position+LDSPI_IndexSRAM->SRAM_Length)>1072) {
		rtd_pr_vpq_emerg("drvif_color_set_LDInterface_SPI_IndexSRAM_Data_Continuous, Data too much\n");
		return;
	}*/
	if (check_LDSPI_IndexSRAM_status_NULL(LDSPI_IndexSRAM)){
		return;
	}
		
	if (Wait_Tx_Done == 1) {/*Normal Usage Wait tx_done*/
	
		do {
			ldspi_wclr_ctrl.regValue = IoReg_Read32(LDSPI_WCLR_CTRL_reg);
			tx_done = ldspi_wclr_ctrl.tx_done;
			if(tx_done == 1) {
				ldspi_outindex_addr_ctrl.regValue = IoReg_Read32(LDSPI_Outindex_Addr_CTRL_reg);
				ldspi_outindex_addr_ctrl.outindex_rw_pointer_set = LDSPI_IndexSRAM->SRAM_Position;
				IoReg_Write32(LDSPI_Outindex_Addr_CTRL_reg, ldspi_outindex_addr_ctrl.regValue);
				for (i=0; i<LDSPI_IndexSRAM->SRAM_Length; i++){
					ldspi_outindex_rwport.outindex_rw_data_port = *(LDSPI_IndexSRAM->SRAM_Value+i); 
					IoReg_Write32(LDSPI_Outindex_Rwport_reg, ldspi_outindex_rwport.regValue);		
					}
				return;
			}
		} while(--timeoutcnt != 0);
		
		if(timeoutcnt == 0){
			rtd_pr_vpq_emerg("drvif_color_set_LDInterface_SPI_IndexSRAM_Data_Continuous, timeoutcnt == 0\n");
		}
	}
	else {
			ldspi_outindex_addr_ctrl.regValue = IoReg_Read32(LDSPI_Outindex_Addr_CTRL_reg);
			ldspi_outindex_addr_ctrl.outindex_rw_pointer_set = LDSPI_IndexSRAM->SRAM_Position;
			IoReg_Write32(LDSPI_Outindex_Addr_CTRL_reg, ldspi_outindex_addr_ctrl.regValue);
			for (i=0; i<LDSPI_IndexSRAM->SRAM_Length; i++){
				ldspi_outindex_rwport.outindex_rw_data_port = *(LDSPI_IndexSRAM->SRAM_Value+i); 
				IoReg_Write32(LDSPI_Outindex_Rwport_reg, ldspi_outindex_rwport.regValue);		
				}
			rtd_pr_vpq_emerg("drvif_color_set_LDInterface_SPI_IndexSRAM_Data_Continuous, done without wait_tv_done\n");
			return;
		}
}
