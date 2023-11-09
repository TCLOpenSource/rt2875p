/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2006
  * All rights reserved.
  * ========================================================================*/

/*================= File Description =======================================*/
/**
 * @file
 * 	This file is for FILM related functions.
 *
 * @author 	$Author$
 * @date 	$Date$
 * @version 	$Revision$
 */

/**
 * @addtogroup film
 * @{
 */

/*============================ Module dependency  ===========================*/
/*#include <unistd.h>			*/ /* 'close()'*/
/*#include <fcntl.h>			*/ /* 'open()'*/
/*#include <stdio.h>*/
/*#include <sys/ioctl.h>*/
#include <linux/string.h>
//#include <mach/io.h>

#include <rbus/sb2_reg.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/scaler/scalerstruct.h>
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <tvscalercontrol/vip/color.h>
#include <tvscalercontrol/vip/dtm.h>
#include <tvscalercontrol/vip/peaking.h>
#include <tvscalercontrol/vip/dcc.h>

#include <tvscalercontrol/scaler/scalerlib.h>
//#include <rbus/scaler/rbusPpOverlayReg.h>
#include <rbus/ppoverlay_reg.h>
#include <rtd_log/rtd_module_log.h>
#include "rtk_vip_logger.h"


#undef rtd_outl
#define rtd_outl(x, y)     								IoReg_Write32(x,y)
#undef rtd_inl
#define rtd_inl(x)     									IoReg_Read32(x)


#undef VIPprintf
#undef VIPprintf
#define VIPprintf(fmt, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_DCC_DEBUG,fmt,##args)

unsigned char drvif_color_dtm_write_Table(int  dtm_table[TBMAX][RGBSEGMAX])
{
    color_dtm_dm_dtm_rw_tab_ctrl_RBUS color_dtm_dm_dtm_rw_tab_ctrl_reg;
    color_dtm_dm_dtm_tab_wr_data_RBUS color_dtm_dm_dtm_tab_wr_data_reg;

    int timeout = 100000;
    int i,tb_idx;
    color_dtm_dm_dtm_rw_tab_ctrl_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_RW_TAB_CTRL_reg);
    color_dtm_dm_dtm_tab_wr_data_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_TAB_WR_DATA_reg);
    //write curve r
    if(color_dtm_dm_dtm_rw_tab_ctrl_reg.table_sel== 3)
    {
        for(i = 0;i < 128;i++)
        {
            color_dtm_dm_dtm_tab_wr_data_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_TAB_WR_DATA_reg);
            color_dtm_dm_dtm_tab_wr_data_reg.wr_tab_data0 = dtm_table[0][i*2];
            color_dtm_dm_dtm_tab_wr_data_reg.wr_tab_data1 = dtm_table[0][i*2 + 1];
            rtd_outl(COLOR_DTM_DM_DTM_TAB_WR_DATA_reg,color_dtm_dm_dtm_tab_wr_data_reg.regValue);
            color_dtm_dm_dtm_rw_tab_ctrl_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_RW_TAB_CTRL_reg);
            color_dtm_dm_dtm_rw_tab_ctrl_reg.wr_tab_en = 1;
            color_dtm_dm_dtm_rw_tab_ctrl_reg.table_sel = 3;
            color_dtm_dm_dtm_rw_tab_ctrl_reg.rw_tab_addr = i;
            rtd_outl(COLOR_DTM_DM_DTM_RW_TAB_CTRL_reg,color_dtm_dm_dtm_rw_tab_ctrl_reg.regValue);
            do
            {
                color_dtm_dm_dtm_rw_tab_ctrl_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_RW_TAB_CTRL_reg);
                timeout --;
                if (timeout <= 0) 
                {
                    VIPprintf(" timeout!! DTM  Init. Write DTM wr tab en Fault!!\n");
                    return FALSE;
                    //break;
                }               
            }while(color_dtm_dm_dtm_rw_tab_ctrl_reg.wr_tab_en);
        }
    }
    else
    {
        for( tb_idx = 0;tb_idx < 3;tb_idx ++)
        {
            for( i = 0;i < 128;i++)
            {
                color_dtm_dm_dtm_tab_wr_data_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_TAB_WR_DATA_reg);
                color_dtm_dm_dtm_tab_wr_data_reg.wr_tab_data0 = dtm_table[tb_idx][i*2];
                color_dtm_dm_dtm_tab_wr_data_reg.wr_tab_data1 = dtm_table[tb_idx][i*2 + 1];
                rtd_outl(COLOR_DTM_DM_DTM_TAB_WR_DATA_reg,color_dtm_dm_dtm_tab_wr_data_reg.regValue);
                color_dtm_dm_dtm_rw_tab_ctrl_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_RW_TAB_CTRL_reg);
                color_dtm_dm_dtm_rw_tab_ctrl_reg.wr_tab_en = 1;
                color_dtm_dm_dtm_rw_tab_ctrl_reg.table_sel = tb_idx;
                color_dtm_dm_dtm_rw_tab_ctrl_reg.rw_tab_addr = i;
                rtd_outl(COLOR_DTM_DM_DTM_RW_TAB_CTRL_reg,color_dtm_dm_dtm_rw_tab_ctrl_reg.regValue);
                do
                {
                    color_dtm_dm_dtm_rw_tab_ctrl_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_RW_TAB_CTRL_reg);
                    timeout --;
                    if (timeout <= 0) 
                    {
                        VIPprintf(" timeout!! DTM  Init. Write DTM wr tab en Fault!!\n");
                        return FALSE;
                        //break;
                    }                
                }while(color_dtm_dm_dtm_rw_tab_ctrl_reg.wr_tab_en);
            }
        }
    }
    return TRUE;
}

unsigned char drvif_color_dtm_get_curret_Table(int  dtm_table[TBMAX][RGBSEGMAX])
{
    int data0,data1;
    int timeout = 100000;
    int i,tb_idx;
    color_dtm_dm_dtm_rw_tab_ctrl_RBUS color_dtm_dm_dtm_rw_tab_ctrl_reg;
    color_dtm_dm_dtm_tab_rd_data_RBUS color_dtm_dm_dtm_tab_rd_data_reg;
    //read curve r
    for(tb_idx = 0;tb_idx < 3;tb_idx ++)
    {
        for(i = 0;i < 128;i++)
        {
            color_dtm_dm_dtm_rw_tab_ctrl_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_RW_TAB_CTRL_reg);
            color_dtm_dm_dtm_rw_tab_ctrl_reg.rd_tab_en = 1;
            color_dtm_dm_dtm_rw_tab_ctrl_reg.table_sel = tb_idx;
            color_dtm_dm_dtm_rw_tab_ctrl_reg.rw_tab_addr = i;
            rtd_outl(COLOR_DTM_DM_DTM_RW_TAB_CTRL_reg,color_dtm_dm_dtm_rw_tab_ctrl_reg.regValue);
            do
            {
                color_dtm_dm_dtm_rw_tab_ctrl_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_RW_TAB_CTRL_reg);
                timeout --;
                if (timeout <= 0) 
                {
                    VIPprintf(" timeout!! Read  DTM wr tab en Fault!!\n");
                    return FALSE;
                    //break;
                }                
            }while(color_dtm_dm_dtm_rw_tab_ctrl_reg.wr_tab_en);
            color_dtm_dm_dtm_tab_rd_data_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_TAB_RD_DATA_reg);
            data0 = color_dtm_dm_dtm_tab_rd_data_reg.rd_tab_data0;
            data1 = color_dtm_dm_dtm_tab_rd_data_reg.rd_tab_data1;
            dtm_table[tb_idx][2*i] = data0;
            dtm_table[tb_idx][2*i+1] = data1;
        }
    }
    return TRUE;
}

unsigned char drvif_color_dtm_get_curret_Hist(int  dtm_hist[HistMax])
{
    color_dtm_dm_dtm_hist_ctrl_RBUS color_dtm_dm_dtm_hist_ctrl_reg;
    color_dtm_dm_dtm_hist_flag_RBUS color_dtm_dm_dtm_hist_flag_reg;
    color_dtm_dtm_hist_count_0_RBUS color_dtm_dtm_hist_count_0_reg;
    color_dtm_dtm_hist_count_1_RBUS color_dtm_dtm_hist_count_1_reg;
    color_dtm_dtm_hist_count_2_RBUS color_dtm_dtm_hist_count_2_reg;
    color_dtm_dtm_hist_count_3_RBUS color_dtm_dtm_hist_count_3_reg;
    color_dtm_dtm_hist_count_4_RBUS color_dtm_dtm_hist_count_4_reg;
    color_dtm_dtm_hist_count_5_RBUS color_dtm_dtm_hist_count_5_reg;
    color_dtm_dtm_hist_count_6_RBUS color_dtm_dtm_hist_count_6_reg;
    color_dtm_dtm_hist_count_7_RBUS color_dtm_dtm_hist_count_7_reg;
    color_dtm_dtm_hist_count_8_RBUS color_dtm_dtm_hist_count_8_reg;
    color_dtm_dtm_hist_count_9_RBUS color_dtm_dtm_hist_count_9_reg;
    color_dtm_dtm_hist_count_a_RBUS color_dtm_dtm_hist_count_a_reg;
    color_dtm_dtm_hist_count_b_RBUS color_dtm_dtm_hist_count_b_reg;
    color_dtm_dtm_hist_count_c_RBUS color_dtm_dtm_hist_count_c_reg;
    color_dtm_dtm_hist_count_d_RBUS color_dtm_dtm_hist_count_d_reg;
    color_dtm_dtm_hist_count_e_RBUS color_dtm_dtm_hist_count_e_reg;
    color_dtm_dtm_hist_count_f_RBUS color_dtm_dtm_hist_count_f_reg;
    int i =0;
    int timeout = 100000;
    

    color_dtm_dm_dtm_hist_flag_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_HIST_FLAG_reg);
    color_dtm_dm_dtm_hist_flag_reg.hist_done_flag = 0;
    rtd_outl(COLOR_DTM_DM_DTM_HIST_FLAG_reg,color_dtm_dm_dtm_hist_flag_reg.regValue);
    
    color_dtm_dm_dtm_hist_ctrl_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_HIST_CTRL_reg);
    color_dtm_dm_dtm_hist_ctrl_reg.dtm_hist_en = 1;
    rtd_outl(COLOR_DTM_DM_DTM_HIST_CTRL_reg,color_dtm_dm_dtm_hist_ctrl_reg.regValue);

    do
    {
        color_dtm_dm_dtm_hist_flag_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_HIST_FLAG_reg);
        timeout --;
        if (timeout <= 0) 
        {
            VIPprintf(" timeout!! Read  hist done flag Fault!!\n");
            return FALSE;
            //break;
        }        
    }while(color_dtm_dm_dtm_hist_flag_reg.hist_done_flag==0);

    if(color_dtm_dm_dtm_hist_ctrl_reg.dtm_hist_en)
    {
        for(i = 0;i < 8; i++)
        {
            timeout = 100000;
            color_dtm_dm_dtm_hist_ctrl_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_HIST_CTRL_reg);
            color_dtm_dm_dtm_hist_ctrl_reg.hist_addr_ctrl = i;
            color_dtm_dm_dtm_hist_ctrl_reg.hist_read = 1;
            rtd_outl(COLOR_DTM_DM_DTM_HIST_CTRL_reg,color_dtm_dm_dtm_hist_ctrl_reg.regValue);

            do
            {
                color_dtm_dm_dtm_hist_ctrl_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_HIST_CTRL_reg);
                if (timeout <= 0) 
                {
                    VIPprintf(" timeout!! Read  hist read flag Fault!!\n");
                    return FALSE;
                    //break;
                }              
            }while(color_dtm_dm_dtm_hist_ctrl_reg.hist_read);

            color_dtm_dtm_hist_count_0_reg.regValue= rtd_inl(COLOR_DTM_DTM_HIST_COUNT_0_reg);
            dtm_hist[i*16] = color_dtm_dtm_hist_count_0_reg.dtm_hist_cnt0;
            color_dtm_dtm_hist_count_1_reg.regValue = rtd_inl(COLOR_DTM_DTM_HIST_COUNT_1_reg);
            dtm_hist[i*16 + 1] = color_dtm_dtm_hist_count_1_reg.dtm_hist_cnt1;
            color_dtm_dtm_hist_count_2_reg.regValue= rtd_inl(COLOR_DTM_DTM_HIST_COUNT_2_reg);
            dtm_hist[i*16 + 2] = color_dtm_dtm_hist_count_2_reg.dtm_hist_cnt2;
            color_dtm_dtm_hist_count_3_reg.regValue = rtd_inl(COLOR_DTM_DTM_HIST_COUNT_3_reg);
            dtm_hist[i*16 + 3] = color_dtm_dtm_hist_count_3_reg.dtm_hist_cnt3;
            color_dtm_dtm_hist_count_4_reg.regValue = rtd_inl(COLOR_DTM_DTM_HIST_COUNT_4_reg);
            dtm_hist[i*16 + 4] = color_dtm_dtm_hist_count_4_reg.dtm_hist_cnt4;
            color_dtm_dtm_hist_count_5_reg.regValue = rtd_inl(COLOR_DTM_DTM_HIST_COUNT_5_reg);
            dtm_hist[i*16 + 5] = color_dtm_dtm_hist_count_5_reg.dtm_hist_cnt5;
            color_dtm_dtm_hist_count_6_reg.regValue = rtd_inl(COLOR_DTM_DTM_HIST_COUNT_6_reg);
            dtm_hist[i*16 + 6] = color_dtm_dtm_hist_count_6_reg.dtm_hist_cnt6;
            color_dtm_dtm_hist_count_7_reg.regValue = rtd_inl(COLOR_DTM_DTM_HIST_COUNT_7_reg);
            dtm_hist[i*16 + 7] = color_dtm_dtm_hist_count_7_reg.dtm_hist_cnt7;
            color_dtm_dtm_hist_count_8_reg.regValue = rtd_inl(COLOR_DTM_DTM_HIST_COUNT_8_reg);
            dtm_hist[i*16 + 8] = color_dtm_dtm_hist_count_8_reg.dtm_hist_cnt8;
            color_dtm_dtm_hist_count_9_reg.regValue = rtd_inl(COLOR_DTM_DTM_HIST_COUNT_9_reg);
            dtm_hist[i*16 + 9] = color_dtm_dtm_hist_count_9_reg.dtm_hist_cnt9;
            color_dtm_dtm_hist_count_a_reg.regValue = rtd_inl(COLOR_DTM_DTM_HIST_COUNT_A_reg);
            dtm_hist[i*16 + 10] = color_dtm_dtm_hist_count_a_reg.dtm_hist_cnta;
            color_dtm_dtm_hist_count_b_reg.regValue = rtd_inl(COLOR_DTM_DTM_HIST_COUNT_B_reg);
            dtm_hist[i*16 + 11] = color_dtm_dtm_hist_count_b_reg.dtm_hist_cntb;
            color_dtm_dtm_hist_count_c_reg.regValue = rtd_inl(COLOR_DTM_DTM_HIST_COUNT_C_reg);
            dtm_hist[i*16 + 12] = color_dtm_dtm_hist_count_c_reg.dtm_hist_cntc;
            color_dtm_dtm_hist_count_d_reg.regValue = rtd_inl(COLOR_DTM_DTM_HIST_COUNT_D_reg);
            dtm_hist[i*16 + 13] = color_dtm_dtm_hist_count_d_reg.dtm_hist_cntd;
            color_dtm_dtm_hist_count_e_reg.regValue = rtd_inl(COLOR_DTM_DTM_HIST_COUNT_E_reg);
            dtm_hist[i*16 + 14] = color_dtm_dtm_hist_count_e_reg.dtm_hist_cnte;
            color_dtm_dtm_hist_count_f_reg.regValue = rtd_inl(COLOR_DTM_DTM_HIST_COUNT_F_reg);
            dtm_hist[i*16 + 15] = color_dtm_dtm_hist_count_f_reg.dtm_hist_cntf;            
            
        }
    }
    return TRUE;
}

void drvif_color_dtm_control(DTM_CTRL_INFO dtm_para)
{
    color_dtm_dm_dtm_hist_ctrl_RBUS color_dtm_dm_dtm_hist_ctrl_reg;
    color_dtm_dm_dtm_ctrl_RBUS color_dtm_dm_dtm_ctrl_reg;
    
    color_dtm_dm_dtm_ctrl_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_CTRL_reg);
    color_dtm_dm_dtm_hist_ctrl_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_HIST_CTRL_reg);
    
    if((dtm_para.dtm_main_en == 0)&&(dtm_para.dtm_sub_en == 0)&&(dtm_para.dtm_overlay_en == 0))
        dtm_para.dtm_hist_en = 0;

    if(dtm_para.bypass_rgb2hsi == 1)
    {
        dtm_para.bypass_h_compensation = 1;
        dtm_para.bypass_s_compensation = 1;
    }
    if(dtm_para.bypass_yuv2rgb == 1)
        dtm_para.bypass_rgb2hsi = 1;
    
    color_dtm_dm_dtm_ctrl_reg.bypass_dtm_lut = dtm_para.bypass_dtm_lut;
    color_dtm_dm_dtm_ctrl_reg.bypass_h_compensation = dtm_para.bypass_h_compensation;
    color_dtm_dm_dtm_ctrl_reg.bypass_s_compensation = dtm_para.bypass_s_compensation;
    color_dtm_dm_dtm_ctrl_reg.bypass_rgb2hsi = dtm_para.bypass_rgb2hsi;
    color_dtm_dm_dtm_ctrl_reg.bypass_yuv2rgb = dtm_para.bypass_yuv2rgb;
    color_dtm_dm_dtm_ctrl_reg.dtm_hsi_mode_sel = dtm_para.dtm_hsi_mode_sel;
    color_dtm_dm_dtm_ctrl_reg.dtm_rgb_mode_sel = dtm_para.dtm_rgb_mode_sel;
    color_dtm_dm_dtm_ctrl_reg.dtm_main_en = dtm_para.dtm_main_en;
    color_dtm_dm_dtm_ctrl_reg.dtm_sub_en = dtm_para.dtm_sub_en;
    color_dtm_dm_dtm_ctrl_reg.dtm_overlay_en = dtm_para.dtm_overlay_en;

    color_dtm_dm_dtm_hist_ctrl_reg.dtm_hist_en = dtm_para.dtm_hist_en;
    color_dtm_dm_dtm_hist_ctrl_reg.dtm_hist_mode_sel = dtm_para.dtm_hist_mode_sel;
    rtd_outl(COLOR_DTM_DM_DTM_CTRL_reg,color_dtm_dm_dtm_ctrl_reg.regValue);
    rtd_outl(COLOR_DTM_DM_DTM_HIST_CTRL_reg,color_dtm_dm_dtm_hist_ctrl_reg.regValue);
    
}