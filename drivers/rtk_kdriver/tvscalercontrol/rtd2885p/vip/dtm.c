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

#include <mach/rtk_log.h>
#define TAG_NAME "VPQ"

#undef rtd_outl
#define rtd_outl(x, y)     								IoReg_Write32(x,y)
#undef rtd_inl
#define rtd_inl(x)     									IoReg_Read32(x)


#undef VIPprintf
#undef VIPprintf
#define VIPprintf(fmt, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_DCC_DEBUG,fmt,##args)

unsigned short  dtm_rgb_table[CHMAX][TBMAX][RGBSEGMAX]={
{
	{//r
		0,	16,	32,	48,	64,	80,	96,	112,	128,	144,	160,	176,	
		192,	208,	224,	240,	256,	272,	288,	304,	320,	336,	352,	368,	
		384,	400,	416,	432,	448,	464,	480,	496,	512,	528,	544,	560,	
		576,	592,	608,	624,	640,	656,	672,	688,	704,	720,	736,	752,	
		768,	784,	800,	816,	832,	848,	864,	880,	896,	912,	928,	944,	
		960,	976,	992,	1008,	1024,	1040,	1056,	1072,	1088,	1104,	1120,	1136,	
		1152,	1168,	1184,	1200,	1216,	1232,	1248,	1264,	1280,	1296,	1312,	1328,	
		1344,	1360,	1376,	1392,	1408,	1424,	1440,	1456,	1472,	1488,	1504,	1520,	
		1536,	1552,	1568,	1584,	1600,	1616,	1632,	1648,	1664,	1680,	1696,	1712,	
		1728,	1744,	1760,	1776,	1792,	1808,	1824,	1840,	1856,	1872,	1888,	1904,	
		1920,	1936,	1952,	1968,	1984,	2000,	2016,	2032,	2048,	2064,	2080,	2096,	
		2112,	2128,	2144,	2160,	2176,	2192,	2208,	2224,	2240,	2256,	2272,	2288,	
		2304,	2320,	2336,	2352,	2368,	2384,	2400,	2416,	2432,	2448,	2464,	2480,	
		2496,	2512,	2528,	2544,	2560,	2576,	2592,	2608,	2624,	2640,	2656,	2672,	
		2688,	2704,	2720,	2736,	2752,	2768,	2784,	2800,	2816,	2832,	2848,	2864,	
		2880,	2896,	2912,	2928,	2944,	2960,	2976,	2992,	3008,	3024,	3040,	3056,	
		3072,	3088,	3104,	3120,	3136,	3152,	3168,	3184,	3200,	3216,	3232,	3248,	
		3264,	3280,	3296,	3312,	3328,	3344,	3360,	3376,	3392,	3408,	3424,	3440,	
		3456,	3472,	3488,	3504,	3520,	3536,	3552,	3568,	3584,	3600,	3616,	3632,	
		3648,	3664,	3680,	3696,	3712,	3728,	3744,	3760,	3776,	3792,	3808,	3824,	
		3840,	3856,	3872,	3888,	3904,	3920,	3936,	3952,	3968,	3984,	4000,	4016,	
		4032,	4048,	4064,	4080
	},
	{//g
		0,	16,	32,	48,	64,	80,	96,	112,	128,	144,	160,	176,	
		192,	208,	224,	240,	256,	272,	288,	304,	320,	336,	352,	368,	
		384,	400,	416,	432,	448,	464,	480,	496,	512,	528,	544,	560,	
		576,	592,	608,	624,	640,	656,	672,	688,	704,	720,	736,	752,	
		768,	784,	800,	816,	832,	848,	864,	880,	896,	912,	928,	944,	
		960,	976,	992,	1008,	1024,	1040,	1056,	1072,	1088,	1104,	1120,	1136,	
		1152,	1168,	1184,	1200,	1216,	1232,	1248,	1264,	1280,	1296,	1312,	1328,	
		1344,	1360,	1376,	1392,	1408,	1424,	1440,	1456,	1472,	1488,	1504,	1520,	
		1536,	1552,	1568,	1584,	1600,	1616,	1632,	1648,	1664,	1680,	1696,	1712,	
		1728,	1744,	1760,	1776,	1792,	1808,	1824,	1840,	1856,	1872,	1888,	1904,	
		1920,	1936,	1952,	1968,	1984,	2000,	2016,	2032,	2048,	2064,	2080,	2096,	
		2112,	2128,	2144,	2160,	2176,	2192,	2208,	2224,	2240,	2256,	2272,	2288,	
		2304,	2320,	2336,	2352,	2368,	2384,	2400,	2416,	2432,	2448,	2464,	2480,	
		2496,	2512,	2528,	2544,	2560,	2576,	2592,	2608,	2624,	2640,	2656,	2672,	
		2688,	2704,	2720,	2736,	2752,	2768,	2784,	2800,	2816,	2832,	2848,	2864,	
		2880,	2896,	2912,	2928,	2944,	2960,	2976,	2992,	3008,	3024,	3040,	3056,	
		3072,	3088,	3104,	3120,	3136,	3152,	3168,	3184,	3200,	3216,	3232,	3248,	
		3264,	3280,	3296,	3312,	3328,	3344,	3360,	3376,	3392,	3408,	3424,	3440,	
		3456,	3472,	3488,	3504,	3520,	3536,	3552,	3568,	3584,	3600,	3616,	3632,	
		3648,	3664,	3680,	3696,	3712,	3728,	3744,	3760,	3776,	3792,	3808,	3824,	
		3840,	3856,	3872,	3888,	3904,	3920,	3936,	3952,	3968,	3984,	4000,	4016,	
		4032,	4048,	4064,	4080
	},
	{//b
		0,	16,	32,	48,	64,	80,	96,	112,	128,	144,	160,	176,	
		192,	208,	224,	240,	256,	272,	288,	304,	320,	336,	352,	368,	
		384,	400,	416,	432,	448,	464,	480,	496,	512,	528,	544,	560,	
		576,	592,	608,	624,	640,	656,	672,	688,	704,	720,	736,	752,	
		768,	784,	800,	816,	832,	848,	864,	880,	896,	912,	928,	944,	
		960,	976,	992,	1008,	1024,	1040,	1056,	1072,	1088,	1104,	1120,	1136,	
		1152,	1168,	1184,	1200,	1216,	1232,	1248,	1264,	1280,	1296,	1312,	1328,	
		1344,	1360,	1376,	1392,	1408,	1424,	1440,	1456,	1472,	1488,	1504,	1520,	
		1536,	1552,	1568,	1584,	1600,	1616,	1632,	1648,	1664,	1680,	1696,	1712,	
		1728,	1744,	1760,	1776,	1792,	1808,	1824,	1840,	1856,	1872,	1888,	1904,	
		1920,	1936,	1952,	1968,	1984,	2000,	2016,	2032,	2048,	2064,	2080,	2096,	
		2112,	2128,	2144,	2160,	2176,	2192,	2208,	2224,	2240,	2256,	2272,	2288,	
		2304,	2320,	2336,	2352,	2368,	2384,	2400,	2416,	2432,	2448,	2464,	2480,	
		2496,	2512,	2528,	2544,	2560,	2576,	2592,	2608,	2624,	2640,	2656,	2672,	
		2688,	2704,	2720,	2736,	2752,	2768,	2784,	2800,	2816,	2832,	2848,	2864,	
		2880,	2896,	2912,	2928,	2944,	2960,	2976,	2992,	3008,	3024,	3040,	3056,	
		3072,	3088,	3104,	3120,	3136,	3152,	3168,	3184,	3200,	3216,	3232,	3248,	
		3264,	3280,	3296,	3312,	3328,	3344,	3360,	3376,	3392,	3408,	3424,	3440,	
		3456,	3472,	3488,	3504,	3520,	3536,	3552,	3568,	3584,	3600,	3616,	3632,	
		3648,	3664,	3680,	3696,	3712,	3728,	3744,	3760,	3776,	3792,	3808,	3824,	
		3840,	3856,	3872,	3888,	3904,	3920,	3936,	3952,	3968,	3984,	4000,	4016,	
		4032,	4048,	4064,	4080
	}
},
{
	{//r
		0,	16,	32,	48,	64,	80,	96,	112,	128,	144,	160,	176,	
		192,	208,	224,	240,	256,	272,	288,	304,	320,	336,	352,	368,	
		384,	400,	416,	432,	448,	464,	480,	496,	512,	528,	544,	560,	
		576,	592,	608,	624,	640,	656,	672,	688,	704,	720,	736,	752,	
		768,	784,	800,	816,	832,	848,	864,	880,	896,	912,	928,	944,	
		960,	976,	992,	1008,	1024,	1040,	1056,	1072,	1088,	1104,	1120,	1136,	
		1152,	1168,	1184,	1200,	1216,	1232,	1248,	1264,	1280,	1296,	1312,	1328,	
		1344,	1360,	1376,	1392,	1408,	1424,	1440,	1456,	1472,	1488,	1504,	1520,	
		1536,	1552,	1568,	1584,	1600,	1616,	1632,	1648,	1664,	1680,	1696,	1712,	
		1728,	1744,	1760,	1776,	1792,	1808,	1824,	1840,	1856,	1872,	1888,	1904,	
		1920,	1936,	1952,	1968,	1984,	2000,	2016,	2032,	2048,	2064,	2080,	2096,	
		2112,	2128,	2144,	2160,	2176,	2192,	2208,	2224,	2240,	2256,	2272,	2288,	
		2304,	2320,	2336,	2352,	2368,	2384,	2400,	2416,	2432,	2448,	2464,	2480,	
		2496,	2512,	2528,	2544,	2560,	2576,	2592,	2608,	2624,	2640,	2656,	2672,	
		2688,	2704,	2720,	2736,	2752,	2768,	2784,	2800,	2816,	2832,	2848,	2864,	
		2880,	2896,	2912,	2928,	2944,	2960,	2976,	2992,	3008,	3024,	3040,	3056,	
		3072,	3088,	3104,	3120,	3136,	3152,	3168,	3184,	3200,	3216,	3232,	3248,	
		3264,	3280,	3296,	3312,	3328,	3344,	3360,	3376,	3392,	3408,	3424,	3440,	
		3456,	3472,	3488,	3504,	3520,	3536,	3552,	3568,	3584,	3600,	3616,	3632,	
		3648,	3664,	3680,	3696,	3712,	3728,	3744,	3760,	3776,	3792,	3808,	3824,	
		3840,	3856,	3872,	3888,	3904,	3920,	3936,	3952,	3968,	3984,	4000,	4016,	
		4032,	4048,	4064,	4080
	},
	{//g
		0,	16,	32,	48,	64,	80,	96,	112,	128,	144,	160,	176,	
		192,	208,	224,	240,	256,	272,	288,	304,	320,	336,	352,	368,	
		384,	400,	416,	432,	448,	464,	480,	496,	512,	528,	544,	560,	
		576,	592,	608,	624,	640,	656,	672,	688,	704,	720,	736,	752,	
		768,	784,	800,	816,	832,	848,	864,	880,	896,	912,	928,	944,	
		960,	976,	992,	1008,	1024,	1040,	1056,	1072,	1088,	1104,	1120,	1136,	
		1152,	1168,	1184,	1200,	1216,	1232,	1248,	1264,	1280,	1296,	1312,	1328,	
		1344,	1360,	1376,	1392,	1408,	1424,	1440,	1456,	1472,	1488,	1504,	1520,	
		1536,	1552,	1568,	1584,	1600,	1616,	1632,	1648,	1664,	1680,	1696,	1712,	
		1728,	1744,	1760,	1776,	1792,	1808,	1824,	1840,	1856,	1872,	1888,	1904,	
		1920,	1936,	1952,	1968,	1984,	2000,	2016,	2032,	2048,	2064,	2080,	2096,	
		2112,	2128,	2144,	2160,	2176,	2192,	2208,	2224,	2240,	2256,	2272,	2288,	
		2304,	2320,	2336,	2352,	2368,	2384,	2400,	2416,	2432,	2448,	2464,	2480,	
		2496,	2512,	2528,	2544,	2560,	2576,	2592,	2608,	2624,	2640,	2656,	2672,	
		2688,	2704,	2720,	2736,	2752,	2768,	2784,	2800,	2816,	2832,	2848,	2864,	
		2880,	2896,	2912,	2928,	2944,	2960,	2976,	2992,	3008,	3024,	3040,	3056,	
		3072,	3088,	3104,	3120,	3136,	3152,	3168,	3184,	3200,	3216,	3232,	3248,	
		3264,	3280,	3296,	3312,	3328,	3344,	3360,	3376,	3392,	3408,	3424,	3440,	
		3456,	3472,	3488,	3504,	3520,	3536,	3552,	3568,	3584,	3600,	3616,	3632,	
		3648,	3664,	3680,	3696,	3712,	3728,	3744,	3760,	3776,	3792,	3808,	3824,	
		3840,	3856,	3872,	3888,	3904,	3920,	3936,	3952,	3968,	3984,	4000,	4016,	
		4032,	4048,	4064,	4080
	},
	{//b
		0,	16,	32,	48,	64,	80,	96,	112,	128,	144,	160,	176,	
		192,	208,	224,	240,	256,	272,	288,	304,	320,	336,	352,	368,	
		384,	400,	416,	432,	448,	464,	480,	496,	512,	528,	544,	560,	
		576,	592,	608,	624,	640,	656,	672,	688,	704,	720,	736,	752,	
		768,	784,	800,	816,	832,	848,	864,	880,	896,	912,	928,	944,	
		960,	976,	992,	1008,	1024,	1040,	1056,	1072,	1088,	1104,	1120,	1136,	
		1152,	1168,	1184,	1200,	1216,	1232,	1248,	1264,	1280,	1296,	1312,	1328,	
		1344,	1360,	1376,	1392,	1408,	1424,	1440,	1456,	1472,	1488,	1504,	1520,	
		1536,	1552,	1568,	1584,	1600,	1616,	1632,	1648,	1664,	1680,	1696,	1712,	
		1728,	1744,	1760,	1776,	1792,	1808,	1824,	1840,	1856,	1872,	1888,	1904,	
		1920,	1936,	1952,	1968,	1984,	2000,	2016,	2032,	2048,	2064,	2080,	2096,	
		2112,	2128,	2144,	2160,	2176,	2192,	2208,	2224,	2240,	2256,	2272,	2288,	
		2304,	2320,	2336,	2352,	2368,	2384,	2400,	2416,	2432,	2448,	2464,	2480,	
		2496,	2512,	2528,	2544,	2560,	2576,	2592,	2608,	2624,	2640,	2656,	2672,	
		2688,	2704,	2720,	2736,	2752,	2768,	2784,	2800,	2816,	2832,	2848,	2864,	
		2880,	2896,	2912,	2928,	2944,	2960,	2976,	2992,	3008,	3024,	3040,	3056,	
		3072,	3088,	3104,	3120,	3136,	3152,	3168,	3184,	3200,	3216,	3232,	3248,	
		3264,	3280,	3296,	3312,	3328,	3344,	3360,	3376,	3392,	3408,	3424,	3440,	
		3456,	3472,	3488,	3504,	3520,	3536,	3552,	3568,	3584,	3600,	3616,	3632,	
		3648,	3664,	3680,	3696,	3712,	3728,	3744,	3760,	3776,	3792,	3808,	3824,	
		3840,	3856,	3872,	3888,	3904,	3920,	3936,	3952,	3968,	3984,	4000,	4016,	
		4032,	4048,	4064,	4080
	}
}
};

unsigned char drvif_color_dtm_write_One_Table(unsigned short dtm_One_table[RGBSEGMAX])
{
	color_dtm_dm_dtm_rw_tab_ctrl_RBUS color_dtm_dm_dtm_rw_tab_ctrl_reg;
	color_dtm_dm_dtm_tab_wr_data_RBUS color_dtm_dm_dtm_tab_wr_data_reg;
	color_dtm_dm_dtm_tab_wr_data_sub_RBUS color_dtm_dm_dtm_tab_wr_data_sub_reg;
	int timeout = 100000;
	int i;

	color_dtm_dm_dtm_rw_tab_ctrl_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_RW_TAB_CTRL_reg);
	color_dtm_dm_dtm_tab_wr_data_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_TAB_WR_DATA_reg);
	color_dtm_dm_dtm_tab_wr_data_sub_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_TAB_WR_DATA_SUB_reg);
	//write one curve curve 

	for(i = 0;i < 128;i++)
	{
		color_dtm_dm_dtm_tab_wr_data_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_TAB_WR_DATA_reg);
		color_dtm_dm_dtm_tab_wr_data_reg.wr_tab_data0 = dtm_One_table[i*2];
		color_dtm_dm_dtm_tab_wr_data_reg.wr_tab_data1 = dtm_One_table[i*2 + 1];

		//printk(" one DTM0[%d]=%d \n",i*2,dtm_one_table[i*2]);
		//printk(" one DTM1[%d]=%d \n",i*2+1,dtm_one_table[i*2 + 1]);

		rtd_outl(COLOR_DTM_DM_DTM_TAB_WR_DATA_reg,color_dtm_dm_dtm_tab_wr_data_reg.regValue);

		color_dtm_dm_dtm_tab_wr_data_sub_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_TAB_WR_DATA_SUB_reg);
		color_dtm_dm_dtm_tab_wr_data_sub_reg.wr_tab_data2 = dtm_One_table[i*2];
		color_dtm_dm_dtm_tab_wr_data_sub_reg.wr_tab_data3 = dtm_One_table[i*2 + 1];
		rtd_outl(COLOR_DTM_DM_DTM_TAB_WR_DATA_SUB_reg,color_dtm_dm_dtm_tab_wr_data_sub_reg.regValue);	
		
		color_dtm_dm_dtm_rw_tab_ctrl_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_RW_TAB_CTRL_reg);
		color_dtm_dm_dtm_rw_tab_ctrl_reg.wr_tab_en = 1;
		color_dtm_dm_dtm_rw_tab_ctrl_reg.rd_tab_en = 0;

		color_dtm_dm_dtm_rw_tab_ctrl_reg.table_sel = 3; //sel=3 ,r=g=b
		color_dtm_dm_dtm_rw_tab_ctrl_reg.rw_tab_addr = i;
		rtd_outl(COLOR_DTM_DM_DTM_RW_TAB_CTRL_reg,color_dtm_dm_dtm_rw_tab_ctrl_reg.regValue);

		do
		{
			color_dtm_dm_dtm_rw_tab_ctrl_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_RW_TAB_CTRL_reg);
			timeout --;
			if (timeout <= 0) 
			{
				rtd_printk(KERN_EMERG, TAG_NAME, "timeout!! DTM  Init. Write DTM wr tab en Fault!!\n");
				return FALSE;
			}               
		}while(color_dtm_dm_dtm_rw_tab_ctrl_reg.wr_tab_en);


	}

	return TRUE;
}

unsigned char drvif_color_dtm_get_sub_One_Table(unsigned short *dtm_one_table)
{
    int data2,data3;
    int timeout = 100000;
    int i;
    color_dtm_dm_dtm_rw_tab_ctrl_RBUS color_dtm_dm_dtm_rw_tab_ctrl_reg;
    color_dtm_dm_dtm_tab_rd_data_sub_RBUS color_dtm_dm_dtm_tab_rd_data_sub_reg;
    //read curve r

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
        color_dtm_dm_dtm_tab_rd_data_sub_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_TAB_RD_DATA_SUB_reg);
        data2 = color_dtm_dm_dtm_tab_rd_data_sub_reg.rd_tab_data2;
        data3 = color_dtm_dm_dtm_tab_rd_data_sub_reg.rd_tab_data3;
        dtm_one_table[2*i] = data2;
        dtm_one_table[2*i+1] = data3;
    }
    return TRUE;
}

unsigned char drvif_color_dtm_write_RGB_Table(unsigned short dtm_rgb_table[CHMAX][TBMAX][RGBSEGMAX])
{
	color_dtm_dm_dtm_rw_tab_ctrl_RBUS color_dtm_dm_dtm_rw_tab_ctrl_reg;
	color_dtm_dm_dtm_tab_wr_data_RBUS color_dtm_dm_dtm_tab_wr_data_reg;
	color_dtm_dm_dtm_tab_wr_data_sub_RBUS color_dtm_dm_dtm_tab_wr_data_sub_reg;

	int timeout = 100000;
	int i,tb_idx;
	color_dtm_dm_dtm_rw_tab_ctrl_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_RW_TAB_CTRL_reg);
	color_dtm_dm_dtm_tab_wr_data_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_TAB_WR_DATA_reg);
	color_dtm_dm_dtm_tab_wr_data_sub_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_TAB_WR_DATA_SUB_reg);
	
	for( tb_idx = 0;tb_idx < 3;tb_idx ++)	//write curve r/g/b
	{
		for( i = 0;i < 128;i++)
		{
			color_dtm_dm_dtm_tab_wr_data_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_TAB_WR_DATA_reg);
			color_dtm_dm_dtm_tab_wr_data_reg.wr_tab_data0 = dtm_rgb_table[0][tb_idx][i*2];
			color_dtm_dm_dtm_tab_wr_data_reg.wr_tab_data1 = dtm_rgb_table[0][tb_idx][i*2 + 1];

			//printk(" DTM0[%d]=%d \n",i*2,dtm_rgb_table[tb_idx][i*2]);
			//printk(" DTM1[%d]=%d \n",i*2 + 1,dtm_rgb_table[tb_idx][i*2 + 1]);

			rtd_outl(COLOR_DTM_DM_DTM_TAB_WR_DATA_reg,color_dtm_dm_dtm_tab_wr_data_reg.regValue);

			color_dtm_dm_dtm_tab_wr_data_sub_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_TAB_WR_DATA_SUB_reg);
			color_dtm_dm_dtm_tab_wr_data_sub_reg.wr_tab_data2 = dtm_rgb_table[1][tb_idx][i*2];
			color_dtm_dm_dtm_tab_wr_data_sub_reg.wr_tab_data3 = dtm_rgb_table[1][tb_idx][i*2 + 1];
			rtd_outl(COLOR_DTM_DM_DTM_TAB_WR_DATA_SUB_reg,color_dtm_dm_dtm_tab_wr_data_sub_reg.regValue);
			
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
					rtd_printk(KERN_EMERG, TAG_NAME, "timeout!! DTM  Init. Write DTM wr tab en Fault!!\n");
					return FALSE;
				}                
			}while(color_dtm_dm_dtm_rw_tab_ctrl_reg.wr_tab_en);
		}

	}
	return TRUE;
}

unsigned char drvif_color_dtm_get_RGB_Table(unsigned short dtm_rgb_table[CHMAX][TBMAX][RGBSEGMAX])
{
	int data0,data1,data2,data3;
	int timeout = 100000;
	int i,tb_idx;
	color_dtm_dm_dtm_rw_tab_ctrl_RBUS color_dtm_dm_dtm_rw_tab_ctrl_reg;
	color_dtm_dm_dtm_tab_rd_data_RBUS color_dtm_dm_dtm_tab_rd_data_reg;
	color_dtm_dm_dtm_tab_rd_data_sub_RBUS color_dtm_dm_dtm_tab_rd_data_sub_reg;	
	//read curve r
	for(tb_idx = 0;tb_idx < 3;tb_idx ++)
	{
		for(i = 0;i < 128;i++)
		{
			color_dtm_dm_dtm_rw_tab_ctrl_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_RW_TAB_CTRL_reg);
			color_dtm_dm_dtm_rw_tab_ctrl_reg.rd_tab_en = 1;
			color_dtm_dm_dtm_rw_tab_ctrl_reg.wr_tab_en = 0;
			color_dtm_dm_dtm_rw_tab_ctrl_reg.table_sel = tb_idx;
			color_dtm_dm_dtm_rw_tab_ctrl_reg.rw_tab_addr = i;
			
			rtd_outl(COLOR_DTM_DM_DTM_RW_TAB_CTRL_reg,color_dtm_dm_dtm_rw_tab_ctrl_reg.regValue);
			do
			{
				color_dtm_dm_dtm_rw_tab_ctrl_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_RW_TAB_CTRL_reg);
				timeout --;
				if (timeout <= 0) 
				{
					rtd_printk(KERN_EMERG, TAG_NAME, "timeout!! Read  DTM wr tab en Fault!!\n");
					return FALSE;
				}                
			}while(color_dtm_dm_dtm_rw_tab_ctrl_reg.rd_tab_en);

			color_dtm_dm_dtm_tab_rd_data_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_TAB_RD_DATA_reg);
			data0 = color_dtm_dm_dtm_tab_rd_data_reg.rd_tab_data0;
			data1 = color_dtm_dm_dtm_tab_rd_data_reg.rd_tab_data1;
			dtm_rgb_table[0][tb_idx][2*i] = data0;
			dtm_rgb_table[0][tb_idx][2*i+1] = data1;
			
			color_dtm_dm_dtm_tab_rd_data_sub_reg.regValue = rtd_inl(COLOR_DTM_DM_DTM_TAB_RD_DATA_SUB_reg);
			data2 = color_dtm_dm_dtm_tab_rd_data_sub_reg.rd_tab_data2;
			data3 = color_dtm_dm_dtm_tab_rd_data_sub_reg.rd_tab_data3;
			dtm_rgb_table[1][tb_idx][2*i] = data2;
			dtm_rgb_table[1][tb_idx][2*i+1] = data3;
		}
	}
	return TRUE;
}

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