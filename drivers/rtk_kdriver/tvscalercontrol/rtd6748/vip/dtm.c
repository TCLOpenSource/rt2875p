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
                color_dtm_dm_dtm_rw_tab_ctrl_reg.table_sel = 0;
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
            color_dtm_dm_dtm_rw_tab_ctrl_reg.table_sel = 0;
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