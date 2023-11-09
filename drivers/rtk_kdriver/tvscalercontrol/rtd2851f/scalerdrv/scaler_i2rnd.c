/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2006
  * All rights reserved.
  * ========================================================================*/

/*================= File Description =======================================*/
/**
 * @file
 * 	This file is for scaler related functions.
 *
 * @author 	$Author$
 * @date		$Date$
 * @version	$Revision$
 * @ingroup	scaler
 */

/**
 * @addtogroup scaler
 * @{
 */

/*============================ Module dependency  ===========================*/
//#include <unistd.h>			// 'close()'
//#include <fcntl.h>				// 'open()'
//#include <stdio.h>
//#include <string.h>
//#include <sys/ioctl.h>
//#include <time.h>
//#include <sys/mman.h>		// mmap
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
#include <rtk_kdriver/RPCDriver.h>
#include <mach/system.h>
#include <mach/timex.h>

#include <linux/pageremap.h>
#include <uapi/linux/const.h>
#include <linux/mm.h>
#include <linux/dma-mapping.h>/*DMA*/
#if defined(CONFIG_I2RND_ENABLE) || defined(CONFIG_PST_ENABLE)
#include <rbus/pst_i2rnd_reg.h>
#endif
#include <rbus/vodma_reg.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/scaler/scalerstruct.h>

#include <tvscalercontrol/scalerdrv/scaler_i2rnd.h>
#include <tvscalercontrol/scalerdrv/scalerdrv.h>


#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerCommon.h>
	#include <scaler/scalerDrvCommon.h>
#else
	#include <scalercommon/scalerCommon.h>
#include <scalercommon/scalerDrvCommon.h>
#endif

#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <scaler_vscdev.h>

//#include <rbus/sensio_reg.h>
#include <rbus/vgip_reg.h>
#include <rbus/main_dither_reg.h>
#include <rbus/rgb2yuv_reg.h>
//#include <rbus/peaking_reg.h> //remove from merlin4
#include <rbus/peaking_reg.h>
#include <rbus/histogram_reg.h>
#include <rbus/profile_reg.h>
#include <rbus/ringfilter_reg.h>
#include <rbus/hsd_dither_reg.h>
#include <rbus/mpegnr_reg.h>
#include <rbus/di_reg.h>
#include <rbus/nr_reg.h>
#include <rbus/iedge_smooth_reg.h>
#include <rbus/scaledown_reg.h>
#include <rbus/i_bist_bisr_reg.h>
#include <rbus/mdomain_cap_reg.h>
#include <rbus/mdomain_vi_main_reg.h>
//#include <rbus/de_xcxl_reg.h>
#include <rbus/tc_reg.h>
#include <rbus/color_mb_peaking_reg.h>
#include <rbus/ipq_decontour_reg.h>
#include <rbus/mdomain_disp_reg.h>


#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_vgip.h>
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_color_mb_peaking.h>
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_De_xcxl.h>
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_Dfilter.h>
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_di.h>
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_Histogram.h>
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_Hsd_dither.h>
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_IDCTI.h>
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_iedge_smooth.h>
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_ipq_decontour.h>
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_Main_Dither.h>
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_Mpegnr.h>
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_nr.h>
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_Peaking.h>
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_Profile.h>
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_rgb2yuv.h>
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_ringfilter.h>
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_Scaledown.h>
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_VODMA.h>
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_Mdomain_cap.h>
#include <tvscalercontrol/scalerdrv/scaler_pst.h>

//num==,0->not I2run reg,1->I2run reg,2->I2Run reg,data port reg,->Group3 register
unsigned char vgip_0[33]={0,0,1,1,1,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,0};
unsigned char color_mb_peaking_0[17]={1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1};
unsigned char De_xcxl_0[47]={1,1,1,1,1,0,0,1,0,1,1,1,1,1,1,1,1,1,1,0,0,1,1,0,0,1,1,1,0,1,\
     0,0,0,0,0,0,0,0,0,0,1,1,1,0,1,1,1};
unsigned char Dfilter_0[2]={1,1};
unsigned char di_0[49]={1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,\
     0,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,0,1};
unsigned char di_1[355]={1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,0,0,0,0,1,1,1,0,1,0,0,0,1,1,0,\
     0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,1,0,1,\
     0,0,1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,1,\
     1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,0,1,1,1,0,1,1,1,1,1,1,1,\
     1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,0,1,0,0,0,0,0,0,0,0,0,1,0,1,1,\
     1,1,1,1,1,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,1,0,1,0,0,0,0,0,0,\
     0,0,0,0,1,1,1,1,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     1,0,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,\
     1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,\
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned char di_2[108]={1,1,1,1,1,1,1,1,0,0,0,0,1,1,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,\
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,1,\
     1,1,0,0,1,1,0,1,0,1,1,1,1,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,1,1,\
     1,1,1,1,0,1,1,1,1,0,0,0,0,0,1,1,1,1};
unsigned char di_3[46]={1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,1,1,1,1,1,1,3,\
     0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1};
unsigned char di_4[54]={1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

unsigned char di_5[12]={1,1,1,1,1,1,1,1,1,1,1,1};
unsigned char di_6[3]={1,1,1};


unsigned char Histogram_0[43]={1,1,1,0,0,0,0,0,1,1,1,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,\
     0,0,0,0,0,0,0,0,0,0,1,1,1};
unsigned char Hsd_dither_0[47]={1,1,1,1,1,1,1,1,1,0,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,\
     1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0};
unsigned char IDCTI_0[7]={1,1,1,1,1,1,1};
unsigned char iedge_smooth_0[60]={1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,0,1,0,0,1,1,1,1,1,1,1,1,0,1,\
     3,1,3,0,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1};
unsigned char ipq_decontour_0[5]={3,1,1,1,1};
unsigned char Main_Dither_0[21]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
unsigned char Mpegnr_0[45]={1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,1,1,0,0,1,1,0,0,0,0,0,0,1};
unsigned char nr_0[64]={1,1,1,1,1,1,1,0,0,1,0,0,0,0,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,\
     1,1,1,1,1,0,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,\
     0,0,0,0};
unsigned char nr_1[64]={0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,\
     1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,3,1,1,1,1,1,1,1,1,0,0,0,0,0,0,\
     0,0,0,0};
unsigned char Peaking_0[5]={1,1,1,1,1}; //remove from merlin4
unsigned char Profile_0[50]={1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,0,0,0,0,0,1,0,0,0,1,1,1,1,0,1,0,0,0};
unsigned char Profile_1[54]={1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
unsigned char rgb2yuv_0[1]={1};
unsigned char rgb2yuv_1[13]={1,1,1,1,1,1,1,1,1,1,1,1,0};
unsigned char ringfilter_0[5]={1,1,1,1,1};
unsigned char Scaledown_0[61]={1,1,1,1,1,1,1,1,0,0,1,1,0,0,0,1,1,0,0,1,0,0,1,1,0,0,0,0,0,1,\
     0,1,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,1,\
     1};
unsigned char Scaledown_1[32]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,\
     1,1};
unsigned char VODMA_0[512]={1,3,1,1,1,0,0,0,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,\
     3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,0,0,0,0,0,0,1,1,1,1,0,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,\
     1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,\
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0};

unsigned char VODMA2_0[512]={1,3,1,1,1,0,0,0,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,\
     3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,0,0,0,0,0,0,1,1,1,1,0,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,\
     1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,\
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     0,0};

unsigned char Mdomain_cap_0[62]={1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
     1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,\
     0,0};
unsigned char Mdomain_cap_1[28]={1,1,1,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1};
unsigned char Mdomain_cap_2[5]={1,1,1,1,1};

unsigned char HDR_VTOP_0[23]={1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1};
unsigned char HDR_VTOP_1[5]={1,1,1,1,1};


#define  DI_REGISTER_NUM_ALL  (DI_REGISTER_NUM_0+ DI_REGISTER_NUM_1+  DI_REGISTER_NUM_2 + DI_REGISTER_NUM_3 + DI_REGISTER_NUM_4 + DI_REGISTER_NUM_5 + DI_REGISTER_NUM_6)
#define PROFILE_REGISTER_NUM_ALL (PROFILE_REGISTER_NUM_0 + PROFILE_REGISTER_NUM_1)
#define RGB2YUV_REGISTER_NUM_ALL (RGB2YUV_REGISTER_NUM_0 + RGB2YUV_REGISTER_NUM_1)
#define NR_REGISTER_NUM_ALL (NR_REGISTER_NUM_0 + NR_REGISTER_NUM_1)
#define SCALEDOWN_REGISTER_NUM_ALL (SCALEDOWN_REGISTER_NUM_0 + SCALEDOWN_REGISTER_NUM_1)
#define MDOMAIN_CAP_REGISTER_NUM_ALL (MDOMAIN_CAP_REGISTER_NUM_0 + MDOMAIN_CAP_REGISTER_NUM_1 + MDOMAIN_CAP_REGISTER_NUM_2)

#define I2RND_REGISTER_NUMBER (VODMA_REGISTER_NUM_0 + VGIP_REGISTER_NUM_0 + COLOR_MB_PEAKING_REGISTER_NUM_0 + \
							   DE_XCXL_REGISTER_NUM_0 + DFILTER_REGISTER_NUM_0 + DI_REGISTER_NUM_ALL + HISTOGRAM_REGISTER_NUM_0 + \
							   HSD_DITHER_REGISTER_NUM_0 + IDCTI_REGISTER_NUM_0 + IEDGE_SMOOTH_REGISTER_NUM_0 + IPQ_DECONTOUR_REGISTER_NUM_0 + \
							   MAIN_DITHER_REGISTER_NUM_0 + MPEGNR_REGISTER_NUM_0 + NR_REGISTER_NUM_ALL + PEAKING_REGISTER_NUM_0 + PROFILE_REGISTER_NUM_ALL + \
							   RGB2YUV_REGISTER_NUM_ALL + RINGFILTER_REGISTER_NUM_0 + SCALEDOWN_REGISTER_NUM_ALL + MDOMAIN_CAP_REGISTER_NUM_ALL + MAIN_TOP_CTL_REGISTER_NUM_0 + MAIN_TOP_CTL_REGISTER_NUM_1)

#define I2RND_REGISTER_SIZE  8
#define REGISTER_VALUE_SIZE  4

#define I2RND_DMA_LENGTH 24 //for 96-bytes align unit 64 bits, 24*8 = 192 bytes //16
#define I2RND_DMA_WATERLEVEL 96

#define I2RND_BLOCK_SIZE ((I2RND_REGISTER_NUMBER + ((I2RND_REGISTER_NUMBER%2) ? 1 : 0)) / 2 * 16) //queue size(total block size) = (regisrer num/2) * 128bits = (regisrer num/2) * 16bytes
//#define I2RND_QUEUE_SIZE (I2RND_BLOCK_SIZE*(I2RND_CMD_NUMBER+1)) //queue size(total block size) = (regisrer num/2) * 128bits = (regisrer num/2) * 16bytes

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif


//i2rnd perind define
#define I2RND_VO_CLOCK_180MHZ 180 //for under 2k1k interlace
#define I2RND_VO_CLOCK_360MHZ 360 //for under 2k1k
#define I2RND_VO_CLOCK_600MHZ 600 //for 4k2k
#define I2RND_VO_CLOCK_594MHZ 594 //for 4k2k
#define I2RND_VO_FIX_H_TOTAL_4K2K 4020
#define I2RND_VO_FIX_V_TOTAL_4K2K 2210
#define I2RND_VO_FIX_V_TOTAL_4K1K 1130
#define I2RND_VO_FIX_H_TOTAL_2K1K 2100
#define I2RND_VO_FIX_V_TOTAL_2K1K_P 1130
#define I2RND_VO_FIX_V_TOTAL_2K1K_I 590
#define VO_FIXED_4K2K_HTOTAL 4400
#define VO_FIXED_4K2K_VTOTAL 2250
#define I2RND_XTAL_CLOCK 27000000 //crystal clock: 27MHz
#define I2RND_TMASTER_TIME 40 //writing time of each register is about 40ns

#define I2RND_ENABLE_ADDR_SIZE 4
#define I2RND_WRITE_CMD_ADDR_SIZE 4
#define I2RND_READ_CMD_ADDR_SIZE 4
#define I2RND_PST_POINT_COUNTER_SIZE 4
#define I2RND_FORCE_WRITE_CMD_SIZE 4
#define I2RND_S1_WRITE_CMD_ADDR_SIZE 4
#define I2RND_S1_READ_CMD_ADDR_SIZE 4
#define I2RND_S1_PST_POINT_COUNTER_SIZE 4
#define I2RND_S0_PST_CHECK_STAGE_SIZE 4
#define I2RND_INFO_SIZE (I2RND_ENABLE_ADDR_SIZE + I2RND_WRITE_CMD_ADDR_SIZE + I2RND_READ_CMD_ADDR_SIZE + I2RND_PST_POINT_COUNTER_SIZE + I2RND_FORCE_WRITE_CMD_SIZE + I2RND_S1_WRITE_CMD_ADDR_SIZE + I2RND_S1_READ_CMD_ADDR_SIZE + I2RND_S1_PST_POINT_COUNTER_SIZE + I2RND_S0_PST_CHECK_STAGE_SIZE)

//i2run sub size condition
#define I2RUN_SUB_SIZE_CONDITION_WIDTH 1920
#define I2RUN_SUB_SIZE_CONDITION_LENGTH 1080

/* Swap bytes in 32 bit value.  */

#define __bswap_constant_32(x) \
  ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) | \
   (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))

#define __bswap_32(x) __bswap_constant_32 (x)

/*===================================  Types ================================*/
//typedef unsigned char UINT8;
//typedef unsigned short UINT16;
//typedef unsigned int UINT32;

/*================================== Variables ==============================*/
//b05_i2rnd
/*20170908, pinyen add CONFIG_I2RND_B05_ENABLE to fix compiler error*/
#ifdef CONFIG_I2RND_B05_ENABLE
static I2RND_START_ADDR i2rnd_b05_st_addr;
static I2RND_STEP_SIZE i2rnd_b05_step_size;
static I2RND_CMD_BUF i2rnd_b05_cmd_buf;
static I2RND_SCPU_VIR_START_ADDR i2rnd_b05_scpu_vir_addr;
static unsigned int i2run_b05_addr = 0;
static unsigned long b05_cache_addr = 0;
//static unsigned int i2run_b05_test = 0;
static unsigned int i2run_b05_queue_size = 0;
unsigned int i2run_b05_allocate_size = 0;
#endif
//static unsigned int vo_fix_htotal_main_4k2k = I2RND_VO_FIX_H_TOTAL_4K2K;
//static unsigned int vo_fix_vtotal_main_4k2k = I2RND_VO_FIX_V_TOTAL_4K2K;

/*20170908, pinyen add CONFIG_I2RND_ENABLE to fix compiler error*/
#ifdef CONFIG_I2RND_ENABLE
//i2rnd
static I2RND_START_ADDR i2rnd_st_addr;
static I2RND_STEP_SIZE i2rnd_step_size;
static I2RND_CMD_BUF i2rnd_cmd_buf;
static I2RND_SCPU_VIR_START_ADDR i2rnd_scpu_vir_addr;

static unsigned int i2rnd_s0_addr_unalign = 0;
static unsigned int i2rnd_s1_addr_unalign = 0;

//control
//static unsigned char i2rnd_enable = FALSE;
static unsigned char i2rnd_display = I2RND_TABLE_OFF;
static unsigned int i2run_queue_size = 0;
//static unsigned int i2run_blk_size = 0;
//static unsigned int i2rnd_final_queue_size = 0; //after align size

//i2rnd period
static unsigned int vo_clock_main = I2RND_VO_CLOCK_360MHZ;
static unsigned int vo_clock_sub = I2RND_VO_CLOCK_360MHZ;
static unsigned int vo_fix_htotal_main = I2RND_VO_FIX_H_TOTAL_2K1K;
static unsigned int vo_fix_htotal_sub = I2RND_VO_FIX_H_TOTAL_2K1K;
static unsigned int vo_fix_vtotal_main = I2RND_VO_FIX_V_TOTAL_2K1K_P;
static unsigned int vo_fix_vtotal_sub = I2RND_VO_FIX_V_TOTAL_2K1K_P;
static unsigned int vo_fix_hload_main = I2RND_VO_FIX_H_TOTAL_2K1K;
static unsigned int vo_fix_hload_sub = I2RND_VO_FIX_H_TOTAL_2K1K;
static unsigned int vo_frame_rate_main = 60;
static unsigned int vo_frame_rate_sub = 60;

static unsigned int i2rnd_period_update_main = 0;
static unsigned int i2rnd_period_update_sub = 0;

//I2RND semaphore
extern struct semaphore I2RND_Semaphore;
extern struct semaphore I2RND_display_Semaphore;
extern unsigned char vsc_i2rnd_sub_stage;
extern unsigned char vsc_i2rnd_dualvo_sub_stage;

SLR_VOINFO i2rnd_main_voinfo;
SLR_VOINFO i2rnd_sub_voinfo;
SLR_VOINFO i2rnd_empty_voinfo;
unsigned char vo_timing_change_for_adaptive = 0;

/*================================== extern ==============================*/
//extern unsigned short Scaler_DispGetInputInfoByDisp(unsigned char channel, SLR_INPUT_INFO infoList);
extern DEBUG_SCALER_VSC_STRUCT DbgSclrFlgTkr;
extern void fw_scalerip_set_di_size_change_hw_detect(unsigned char enable);
extern void drvif_color_rtnr_progressive_check(unsigned char interlace_flag, unsigned char channel, unsigned char db_apply);
extern void rtk_hal_vsc_pst_enable(unsigned char enable);
extern unsigned char Get_PANEL_VFLIP_ENABLE(void);

/*================================== Function  ==============================*/

void Scaler_I2rnd_handler(void)
{
	//enable i2rnd
	if(!Scaler_I2rnd_get_force_cmd()){//if force cmd buf, do not enable i2r function
		drvif_scaler_i2rnd_top_ctrl(Scaler_I2rnd_get_timing_enable());
	}

	//toggle DI size let DI address apply @Crixus 20160802
	i2rnd_di_size_toggle();
}

unsigned char Scaler_I2rnd_get_enable(void)
{
	if (i2rnd_scpu_vir_addr.i2rnd_enable_vir_addr) {
		return (*(volatile unsigned char *)(i2rnd_scpu_vir_addr.i2rnd_enable_vir_addr));
	} else {
		return 0;
	}
}
void Scaler_I2rnd_set_enable(unsigned char enable)
{
	if (i2rnd_scpu_vir_addr.i2rnd_enable_vir_addr) {
		*(volatile unsigned char *)(i2rnd_scpu_vir_addr.i2rnd_enable_vir_addr) = enable;
	}
}
unsigned char Scaler_I2rnd_get_read_cmd_num(unsigned char display_index)
{
	//Update s1 cmd buffer for sub pst @Crixus 20171031
	if(display_index == I2RND_MAIN_S0_TABLE){
		if (i2rnd_scpu_vir_addr.i2rnd_read_cmd_vir_addr)
			return (*(volatile unsigned char *)(i2rnd_scpu_vir_addr.i2rnd_read_cmd_vir_addr));
		else
			return 0;
	}
	else if(display_index == I2RND_SUB_S1_TABLE){
		if (i2rnd_scpu_vir_addr.i2rnd_s1_read_cmd_vir_addr)
			return (*(volatile unsigned char *)(i2rnd_scpu_vir_addr.i2rnd_s1_read_cmd_vir_addr));
		else
			return 0;
	}
	else{
		return 0;
	}
}
void Scaler_I2rnd_set_read_cmd_num(unsigned char enable)
{
	if (i2rnd_scpu_vir_addr.i2rnd_read_cmd_vir_addr) {
		*(volatile unsigned char *)(i2rnd_scpu_vir_addr.i2rnd_read_cmd_vir_addr) = enable;
	}
	if (i2rnd_scpu_vir_addr.i2rnd_s1_read_cmd_vir_addr) {
		*(volatile unsigned char *)(i2rnd_scpu_vir_addr.i2rnd_s1_read_cmd_vir_addr) = enable;
	}
}
unsigned char Scaler_I2rnd_get_write_cmd_num(unsigned char display_index)
{
	//Update s1 cmd buffer for sub pst @Crixus 20171031
	if(display_index == I2RND_MAIN_S0_TABLE){
		if (i2rnd_scpu_vir_addr.i2rnd_write_cmd_vir_addr)
			return (*(volatile unsigned char *)(i2rnd_scpu_vir_addr.i2rnd_write_cmd_vir_addr));
		else
			return 0;
	}
	else if(display_index == I2RND_SUB_S1_TABLE){
		if (i2rnd_scpu_vir_addr.i2rnd_s1_write_cmd_vir_addr)
			return (*(volatile unsigned char *)(i2rnd_scpu_vir_addr.i2rnd_s1_write_cmd_vir_addr));
		else
			return 0;
	}
	else{
		return 0;
	}
}
void Scaler_I2rnd_set_write_cmd_num(void)
{
	unsigned char cmd_num;
	if(Scaler_I2rnd_get_display() == I2RND_MAIN_S0_TABLE){
		if (i2rnd_scpu_vir_addr.i2rnd_write_cmd_vir_addr) {
			Scaler_i2rnd_buffer_copy(I2RND_MAIN_S0_TABLE);
#if 1
			cmd_num = (*(volatile unsigned char *)(i2rnd_scpu_vir_addr.i2rnd_write_cmd_vir_addr));
			if ((cmd_num + 1 )> I2RND_CMD_NUMBER) {
				cmd_num = 0;
			} else {
				cmd_num = cmd_num + 1;
			}
			Scaler_i2rnd_buffer_copy(I2RND_MAIN_S0_TABLE);
			*(volatile unsigned char *)(i2rnd_scpu_vir_addr.i2rnd_write_cmd_vir_addr) = cmd_num;
#else

			*(volatile unsigned char *)(i2rnd_scpu_vir_addr.i2rnd_write_cmd_vir_addr) = 0;
#endif
 		}
	}
	else if(Scaler_I2rnd_get_display() == I2RND_SUB_S1_TABLE){
		if (i2rnd_scpu_vir_addr.i2rnd_s1_write_cmd_vir_addr) {
			Scaler_i2rnd_buffer_copy(I2RND_SUB_S1_TABLE);
#if 1
			cmd_num = (*(volatile unsigned char *)(i2rnd_scpu_vir_addr.i2rnd_s1_write_cmd_vir_addr));
			if ((cmd_num + 1 )> I2RND_CMD_NUMBER) {
				cmd_num = 0;
			} else {
				cmd_num = cmd_num + 1;
			}
			Scaler_i2rnd_buffer_copy(I2RND_SUB_S1_TABLE);
			*(volatile unsigned char *)(i2rnd_scpu_vir_addr.i2rnd_s1_write_cmd_vir_addr) = cmd_num;
#else

			*(volatile unsigned char *)(i2rnd_scpu_vir_addr.i2rnd_write_cmd_vir_addr) = 0;
#endif
 		}
	}
}
void Scaler_i2rnd_write_default_cmd_num(void)
{
	if (i2rnd_scpu_vir_addr.i2rnd_write_cmd_vir_addr) {
		Scaler_i2rnd_buffer_copy(I2RND_MAIN_S0_TABLE);
		*(volatile unsigned char *)(i2rnd_scpu_vir_addr.i2rnd_write_cmd_vir_addr) = 0;
	}
	if (i2rnd_scpu_vir_addr.i2rnd_s1_write_cmd_vir_addr) {
		Scaler_i2rnd_buffer_copy(I2RND_SUB_S1_TABLE);
		*(volatile unsigned char *)(i2rnd_scpu_vir_addr.i2rnd_s1_write_cmd_vir_addr) = 0;
	}
}
unsigned int Scaler_i2rnd_cmd_buffer_write_offset_addr(unsigned int addr)
{
	unsigned int curr_addr= 0;
	curr_addr = addr + Scaler_I2rnd_get_write_cmd_num(Scaler_I2rnd_get_display()) * i2rnd_st_addr.cmd_buffer_size;
	return curr_addr;
}
unsigned int Scaler_i2rnd_cmd_buffer_read_offset_addr(unsigned int addr)
{
	unsigned int curr_addr= 0;
	curr_addr = addr + Scaler_I2rnd_get_read_cmd_num(Scaler_I2rnd_get_display()) * i2rnd_st_addr.cmd_buffer_size;
	return curr_addr;
}

unsigned char Scaler_I2rnd_get_display(void)
{
	return i2rnd_display;
}
void Scaler_I2rnd_set_display(unsigned char display)
{
	down(&I2RND_display_Semaphore);
	rtd_pr_vbe_emerg("[Eric][KR] Scaler_I2rnd_set_display = %d\n",display);
	i2rnd_display = display;
	up(&I2RND_display_Semaphore);

}
void i2rnd_save_voinfo(SLR_VOINFO *VoInfo, unsigned char display)
{
	if (display == SLR_SUB_DISPLAY) {
		memcpy(&i2rnd_sub_voinfo, VoInfo, sizeof(SLR_VOINFO));
	} else {
		memcpy(&i2rnd_main_voinfo, VoInfo, sizeof(SLR_VOINFO));
	}
}

/***********************************I2RND**************************************************/
/*
1.allocate i2run memory about cmd buf queue+SCPU/VCPU comulate info(I2run enable or not)
2.send RPC info to VCPU and recored each ip start access address
3.Calculate the I2run DMA setting
*/
void drvif_scaler_i2rnd_initial(void)
{
	unsigned long cache_addr;
	unsigned int blk_size_mod = 0;
	unsigned int i2run_blk_size = 0;
	unsigned int i2run_allocate_size = 0;
	unsigned int i2run_s1_allocate_size = 0;
	unsigned int align_addr = 0, query_addr = 0;
	//default period setting
	vodma_vodma_i2rnd_m_period_load_RBUS  vo_i2rnd_m_p_load;
	vodma_vodma_i2rnd_m_period_RBUS  vo_i2rnd_m_p;
	vodma_vodma_i2rnd_s_period_load_RBUS  vo_i2rnd_s_p_load;
	vodma_vodma_i2rnd_s_period_RBUS  vo_i2rnd_s_p;
	vodma_vodma_reg_db_ctrl2_RBUS vodma_vodma_reg_db_ctrl2;
	vodma_vodma_i2rnd_m_period_load_dbg_RBUS vodma_vodma_i2rnd_m_period_load_dbg;
	vodma_vodma_i2rnd_s_period_load_dbg_RBUS vodma_vodma_i2rnd_s_period_load_dbg;

	//align total queue size
	i2run_blk_size = I2RND_BLOCK_SIZE;
	blk_size_mod = i2run_blk_size % (I2RND_DMA_LENGTH*8);//No remain so need align length size length*64bits
	if(blk_size_mod != 0){
		i2run_blk_size = i2run_blk_size + ((I2RND_DMA_LENGTH*8) - blk_size_mod);
	}

	//merlin4 memory do 96 bytes align @Crixus 20180321
	i2run_blk_size = dvr_size_alignment(i2run_blk_size);

	i2run_queue_size = i2run_blk_size*(I2RND_CMD_NUMBER+1);
	i2run_allocate_size =i2run_queue_size+I2RND_INFO_SIZE;
	rtd_pr_vbe_emerg("[%s][crixus]i2rnd_final_queue_size = %d\n", __FUNCTION__, i2run_queue_size);

	//share memory allocation
	if (i2rnd_scpu_vir_addr.s0_vo_st_vir_addr == 0) {
		//merlin4 memory do 96 bytes align @Crixus 20180321
		i2run_allocate_size = dvr_size_alignment_cma(i2run_allocate_size);//size do 96-bytes align first
		cache_addr = (unsigned long)dvr_malloc_uncached_specific(i2run_allocate_size, GFP_DCU1,(void*)&(i2rnd_scpu_vir_addr.s0_vo_st_vir_addr));
		i2rnd_s0_addr_unalign = cache_addr; //save the unalign address for free memory.
		query_addr = (unsigned int)dvr_to_phys((void*)cache_addr);
		align_addr = dvr_memory_alignment((unsigned long)query_addr, i2run_allocate_size);
		if (align_addr == (unsigned int)NULL)
        		BUG();

		i2rnd_st_addr.s0_vo_st_addr = align_addr;
		i2rnd_st_addr.i2rnd_enable_addr = i2rnd_st_addr.s0_vo_st_addr + i2run_queue_size;
		i2rnd_st_addr.write_cmd_num_addr = i2rnd_st_addr.i2rnd_enable_addr + I2RND_ENABLE_ADDR_SIZE;
		i2rnd_st_addr.read_cmd_num_addr = i2rnd_st_addr.write_cmd_num_addr + I2RND_WRITE_CMD_ADDR_SIZE;
		memset((unsigned char *)i2rnd_scpu_vir_addr.s0_vo_st_vir_addr, 0, sizeof(unsigned char) * i2run_allocate_size);
	}
	if (i2rnd_scpu_vir_addr.s1_vo_st_vir_addr == 0) {
		//merlin4 memory do 96 bytes align @Crixus 20180321
		i2run_s1_allocate_size = (i2run_queue_size + I2RND_S1_WRITE_CMD_ADDR_SIZE + I2RND_S1_READ_CMD_ADDR_SIZE);
		i2run_s1_allocate_size = dvr_size_alignment_cma(i2run_s1_allocate_size);//size do 96-bytes align first

		cache_addr = (unsigned long)dvr_malloc_uncached_specific(i2run_s1_allocate_size, GFP_DCU1,(void*)&(i2rnd_scpu_vir_addr.s1_vo_st_vir_addr));
		i2rnd_s1_addr_unalign = cache_addr; //save the unalign address for free memory.
		//i2rnd_st_addr.s1_vo_st_addr = (unsigned int)dvr_to_phys((void*)cache_addr);
		query_addr = (unsigned int)dvr_to_phys((void*)cache_addr);
		align_addr = dvr_memory_alignment((unsigned long)query_addr, i2run_s1_allocate_size);
		if (align_addr == (unsigned int)NULL)
        		BUG();

		//do 96bytes align make DMA speedup
		i2rnd_st_addr.s1_vo_st_addr = align_addr;
		i2rnd_st_addr.s1_write_cmd_num_addr = i2rnd_st_addr.s1_vo_st_addr + i2run_queue_size;
		i2rnd_st_addr.s1_read_cmd_num_addr = i2rnd_st_addr.s1_write_cmd_num_addr + I2RND_S1_WRITE_CMD_ADDR_SIZE;
		memset((unsigned char *)i2rnd_scpu_vir_addr.s1_vo_st_vir_addr, 0, sizeof(unsigned char) * (i2run_queue_size + I2RND_S1_WRITE_CMD_ADDR_SIZE + I2RND_S1_READ_CMD_ADDR_SIZE));
	}
	i2rnd_st_addr.cmd_buffer_size = i2run_blk_size;

	rtd_pr_vbe_emerg("[%s][crixus]i2rnd_st_addr.s0_vo_st_addr = %x\n", __FUNCTION__, i2rnd_st_addr.s0_vo_st_addr);
	rtd_pr_vbe_emerg("[%s][crixus]i2rnd_st_addr.s1_vo_st_addr = %x\n", __FUNCTION__, i2rnd_st_addr.s1_vo_st_addr);
	rtd_pr_vbe_emerg("[%s][crixus]i2rnd_st_addr.i2rnd_enable_addr = %x\n", __FUNCTION__, i2rnd_st_addr.i2rnd_enable_addr);

	if((i2rnd_st_addr.s0_vo_st_addr == 0) || (i2rnd_st_addr.s1_vo_st_addr == 0)){
		rtd_pr_vbe_emerg("[I2RND]malloc i2rnd buffer fail .......................................\n");
		rtd_pr_vbe_emerg("[I2RND]main_i2rnd_dma_start_addr = %x\n", i2rnd_st_addr.s0_vo_st_addr);
		rtd_pr_vbe_emerg("[I2RND]sub_i2rnd_dma_start_addr = %x\n", i2rnd_st_addr.s1_vo_st_addr);
		return;
	}
	Scaler_i2rnd_setbuffer();
	//i2rnd dma setting
	drvif_scaler_i2rnd_dma_setting(I2RND_CMD_NUMBER, i2run_blk_size);

	//disable double buffer
	vodma_vodma_reg_db_ctrl2.regValue = rtd_inl(VODMA_VODMA_REG_DB_CTRL2_reg);
	vodma_vodma_reg_db_ctrl2.i2rnd_db_en = 0;
    	IoReg_Write32(VODMA_VODMA_REG_DB_CTRL2_reg, vodma_vodma_reg_db_ctrl2.regValue);

	//VODMA i2rnd period min & enable xtal clock
	vo_i2rnd_m_p_load.regValue = rtd_inl(VODMA_vodma_i2rnd_m_period_load_reg);
	vo_i2rnd_m_p_load.i2rnd_m_period_load = 2200 * 5;//s0_period;//300;
	vo_i2rnd_m_p_load.i2rnd_period_clk_src = 1;//reference xtal clock in mac5p @Crixus 20170104
	IoReg_Write32(VODMA_vodma_i2rnd_m_period_load_reg, vo_i2rnd_m_p_load.regValue);

	//VODMA i2rnd period
	vo_i2rnd_m_p.regValue = rtd_inl(VODMA_vodma_i2rnd_m_period_reg);
	vo_i2rnd_m_p.i2rnd_m_period = 2200 * 1090;//s0_period - 300;
	IoReg_Write32(VODMA_vodma_i2rnd_m_period_reg, vo_i2rnd_m_p.regValue);

	//VODMA i2rnd period max
	vodma_vodma_i2rnd_m_period_load_dbg.regValue = rtd_inl(VODMA_vodma_i2rnd_m_period_load_dbg_reg);
	vodma_vodma_i2rnd_m_period_load_dbg.i2rnd_m_period_load_max = 2200 * 35;
	vodma_vodma_i2rnd_m_period_load_dbg.i2rnd_period_load_max_dbg_en = 1; //default enable m_period load max measure @Crixus 20170504
	vodma_vodma_i2rnd_m_period_load_dbg.i2rnd_period_load_max_measure_en = 1;
	IoReg_Write32(VODMA_vodma_i2rnd_m_period_load_dbg_reg, vodma_vodma_i2rnd_m_period_load_dbg.regValue);
	//sub period min
	vo_i2rnd_s_p_load.regValue = rtd_inl(VODMA_vodma_i2rnd_s_period_load_reg);
	vo_i2rnd_s_p_load.i2rnd_s_period_load = 2200 * 5;
	IoReg_Write32(VODMA_vodma_i2rnd_s_period_load_reg, vo_i2rnd_s_p_load.regValue);

	//sub period load
	vo_i2rnd_s_p.regValue = rtd_inl(VODMA_vodma_i2rnd_s_period_reg);
	vo_i2rnd_s_p.i2rnd_s_period = 2200 * 1090;
	IoReg_Write32(VODMA_vodma_i2rnd_s_period_reg, vo_i2rnd_s_p.regValue);

	//sub period max
	vodma_vodma_i2rnd_s_period_load_dbg.regValue = rtd_inl(VODMA_vodma_i2rnd_s_period_load_dbg_reg);
	vodma_vodma_i2rnd_s_period_load_dbg.i2rnd_s_period_load_max = 2200 * 35;
	IoReg_Write32(VODMA_vodma_i2rnd_s_period_load_dbg_reg, vodma_vodma_i2rnd_s_period_load_dbg.regValue);
//	i2rnd_scpu_vir_addr.i2rnd_pst_write_pnt_counter = 0;
}

void drvif_scaler_i2rnd_uninitial(void)
{

	//disable i2rnd
	drvif_scaler_i2rnd_top_ctrl(_DISABLE);

	//disable DI hw detect
	fw_scalerip_set_di_size_change_hw_detect(_DISABLE);
	return;//FIX ME,below code will crash
	//free memory
	if((i2rnd_st_addr.s0_vo_st_addr != 0) && (i2rnd_scpu_vir_addr.s0_vo_st_vir_addr != 0)){
		dvr_free((void *)i2rnd_s0_addr_unalign);
		i2rnd_st_addr.s0_vo_st_addr = 0;
		i2rnd_scpu_vir_addr.s0_vo_st_vir_addr = 0;

	}

	if((i2rnd_st_addr.s1_vo_st_addr != 0) && (i2rnd_scpu_vir_addr.s1_vo_st_vir_addr != 0)){
		dvr_free((void *)i2rnd_s1_addr_unalign);
		i2rnd_st_addr.s1_vo_st_addr = 0;
		i2rnd_scpu_vir_addr.s1_vo_st_vir_addr = 0;
	}
}

void drvif_scaler_i2rnd_top_ctrl(bool i2rnd_en)
{
	vodma_vodma_i2rnd_RBUS    vo_i2rnd;
	vodma_vodma_i2rnd_st_RBUS vo_i2rnd_st;
    	pst_i2rnd_top_ctrl_RBUS   top_ctrl;
	unsigned timeout = 0x3ffff;

	if(i2rnd_en){
	    top_ctrl.regValue = rtd_inl(PST_I2RND_TOP_CTRL_reg);
	    top_ctrl.i2rnd_en = i2rnd_en;
	    IoReg_Write32(PST_I2RND_TOP_CTRL_reg, top_ctrl.regValue);
	}else{
		//Eric@20171023 wait to s0 period to make sure s0 settings already moved to register
		vo_i2rnd_st.regValue = rtd_inl(VODMA_vodma_i2rnd_st_reg);
		if(vo_i2rnd_st.i2rnd_st != 3){
			while(timeout){
				vo_i2rnd_st.regValue = rtd_inl(VODMA_vodma_i2rnd_st_reg);
				if(vo_i2rnd_st.i2rnd_st == 3)
					break;
				timeout --;
			}
			rtd_pr_vbe_emerg("[I2RND][drvif_scaler_i2rnd_top_ctrl] i2rnd_st timeout1 = %x\n",timeout);
		}
		vo_i2rnd.regValue = rtd_inl(VODMA_vodma_i2rnd_reg);
		vo_i2rnd.i2rnd_timing_mode = _1_MAIN_0_SUB;
		IoReg_Write32(VODMA_vodma_i2rnd_reg, vo_i2rnd.regValue);
		timeout = 0x3ffff;
		//Eric@20171023 wait to i2rnd_st = 0 to make status reset
		vo_i2rnd_st.regValue = rtd_inl(VODMA_vodma_i2rnd_st_reg);
		if(vo_i2rnd_st.i2rnd_st != 0){
			while(timeout){
				vo_i2rnd_st.regValue = rtd_inl(VODMA_vodma_i2rnd_st_reg);
				if(vo_i2rnd_st.i2rnd_st == 0)
					break;
				timeout --;
			}
			rtd_pr_vbe_emerg("[I2RND][drvif_scaler_i2rnd_top_ctrl] i2rnd_st timeout2 = %x\n",timeout);
		}
		vo_i2rnd.i2rnd_timing_en = 0;
		IoReg_Write32(VODMA_vodma_i2rnd_reg, vo_i2rnd.regValue);
		//Eric@20171023 check s0 rtnr progressive
		drvif_color_rtnr_progressive_check(!i2rnd_main_voinfo.progressive,SLR_MAIN_DISPLAY, 1);
		top_ctrl.regValue = rtd_inl(PST_I2RND_TOP_CTRL_reg);
		top_ctrl.i2rnd_en = i2rnd_en;
		IoReg_Write32(PST_I2RND_TOP_CTRL_reg, top_ctrl.regValue);
	}
}
#if 0
void drvif_scaler_i2rnd_vodma1_flag_fifo_prepare(void)
{
	vodma_vodma_v1_dcfg_RBUS v_v1_dcfg;
	v_v1_dcfg.dma_mode = 0;
	v_v1_dcfg.dma_auto_mode = 1;
	v_v1_dcfg.field_mode = 1;
	IoReg_Write32(VODMA_VODMA_V1_DCFG_reg, v_v1_dcfg.regValue);

	vodma_vodma_v1chroma_fmt_RBUS v_v1chroma_fmt;
	v_v1chroma_fmt.rtnr_addr_hw_mode = 0;
	IoReg_Write32(VODMA_VODMA_V1CHROMA_FMT_reg, v_v1chroma_fmt.regValue);

	vodma_vodma_quincunx_setting_RBUS vodma_quincunx_setting;
	vodma_quincunx_setting.regValue = rtd_inl(VODMA_VODMA_quincunx_setting_reg);
	vodma_quincunx_setting.quincunx_mode_src_sel = 1;
	IoReg_Write32(VODMA_VODMA_quincunx_setting_reg, vodma_quincunx_setting.regValue);
}
#endif
void drvif_scaler_i2rnd_vodma_setting(unsigned char display, unsigned char input_src)
{
	vodma_vodma_i2rnd_RBUS    vo_i2rnd;
    	vodma_vodma_i2rnd_m_flag_RBUS vo_i2rnd_m_flag;
	vodma_vodma_i2rnd_s_flag_RBUS vo_i2rnd_s_flag;
	vodma_vodma_i2rnd_m_period_load_RBUS  vo_i2rnd_m_p_load;
	vodma_vodma_i2rnd_m_period_RBUS  vo_i2rnd_m_p;
	vodma_vodma_i2rnd_s_period_load_RBUS  vo_i2rnd_s_p_load;
	vodma_vodma_i2rnd_s_period_RBUS  vo_i2rnd_s_p;
	vodma_vodma_reg_db_ctrl2_RBUS vodma_vodma_reg_db_ctrl2;
	vodma_vodma_i2rnd_m_period_load_dbg_RBUS vodma_vodma_i2rnd_m_period_load_dbg;
	vodma_vodma_i2rnd_s_period_load_dbg_RBUS vodma_vodma_i2rnd_s_period_load_dbg;
	vodma_vodma_i2rnd_dma_info_RBUS vodma_i2rnd_dma_info;

	unsigned int frame_period = 0, period_min = 0, period_max = 0, free_period = 0;
	unsigned int h_active = 0, v_active = 0, interlace = 0;
	unsigned char refine_sub_period = 0;

	//i2rnd period
	unsigned int fix_htotal = 0;
	unsigned int vo_clock = 0;
	unsigned int frame_rate = Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ);
	unsigned int fix_vtotal = 0;

	//disable double buffer
	vodma_vodma_reg_db_ctrl2.regValue = rtd_inl(VODMA_VODMA_REG_DB_CTRL2_reg);
	vodma_vodma_reg_db_ctrl2.i2rnd_db_en = 0;
    	IoReg_Write32(VODMA_VODMA_REG_DB_CTRL2_reg, vodma_vodma_reg_db_ctrl2.regValue);

	if(display == SLR_MAIN_DISPLAY){
		vo_i2rnd_m_flag.regValue = rtd_inl(VODMA_vodma_i2rnd_m_flag_reg);
		//vo_i2rnd_m_flag.i2rnd_hw_mask_repeat_frame_m_en = 1;
		vo_i2rnd_m_flag.i2rnd_m_field_fw = 0;
		vo_i2rnd_m_flag.i2rnd_m_block_free_run = 0;
		vo_i2rnd_m_flag.i2rnd_m_block_sel = 1;
		vo_i2rnd_m_flag.i2rnd_m_field_fw = 0;
		if(input_src == VSC_INPUTSRC_VDEC){//DTV use field free @Crixus 20160805
			vo_i2rnd_m_flag.i2rnd_m_field_free_run = 1;
			vo_i2rnd_m_flag.i2rnd_m_field_sel = 3;
		}
		else{
			vo_i2rnd_m_flag.i2rnd_m_field_free_run = 0;
			vo_i2rnd_m_flag.i2rnd_m_field_sel = (Get_PANEL_VFLIP_ENABLE()? 2: 1);
		}

		vo_i2rnd_m_flag.i2rnd_m_lflag_fw = 0;
		vo_i2rnd_m_flag.i2rnd_m_lflag_free_run = 0;
		vo_i2rnd_m_flag.i2rnd_m_lflag_sel = 1;
		IoReg_Write32(VODMA_vodma_i2rnd_m_flag_reg, vo_i2rnd_m_flag.regValue);
	}
	else if(display == SLR_SUB_DISPLAY){
		vo_i2rnd_s_flag.regValue = rtd_inl(VODMA_vodma_i2rnd_s_flag_reg);
		//vo_i2rnd_s_flag.i2rnd_hw_mask_repeat_frame_s_en = 1;
		vo_i2rnd_s_flag.i2rnd_s_field_fw = 0;
		vo_i2rnd_s_flag.i2rnd_s_block_free_run = 0;
		vo_i2rnd_s_flag.i2rnd_s_block_sel = 1;
		vo_i2rnd_s_flag.i2rnd_s_field_fw = 0;
		if(input_src == VSC_INPUTSRC_VDEC){//DTV use field free @Crixus 20160805
			vo_i2rnd_s_flag.i2rnd_s_field_free_run = 1;
			vo_i2rnd_s_flag.i2rnd_s_field_sel = 3;
		}
		else{
			vo_i2rnd_s_flag.i2rnd_s_field_free_run = 0;
			vo_i2rnd_s_flag.i2rnd_s_field_sel = 1;
		}
		vo_i2rnd_s_flag.i2rnd_s_lflag_fw = 0;
		vo_i2rnd_s_flag.i2rnd_s_lflag_free_run = 0;
		vo_i2rnd_s_flag.i2rnd_s_lflag_sel = 1;
		IoReg_Write32(VODMA_vodma_i2rnd_s_flag_reg, vo_i2rnd_s_flag.regValue);
	}

	//clear write point
	vodma_i2rnd_dma_info.regValue = rtd_inl(VODMA_vodma_i2rnd_dma_info_reg);
	vodma_i2rnd_dma_info.dma_info_rptr_fw_dispm = 0;
	vodma_i2rnd_dma_info.dma_info_rptr_fw_set_dispm = 1;
	vodma_i2rnd_dma_info.dma_info_wptr_fw_dispm = 0;
	vodma_i2rnd_dma_info.dma_info_wptr_fw_set_dispm= 1;
	vodma_i2rnd_dma_info.dma_info_rptr_fw_i3ddma = 0;
	vodma_i2rnd_dma_info.dma_info_rptr_fw_set_i3ddma= 1;
	vodma_i2rnd_dma_info.dma_info_wptr_fw_i3ddma = 0;
	vodma_i2rnd_dma_info.dma_info_wptr_fw_set_i3ddma= 1;
	IoReg_Write32(VODMA_vodma_i2rnd_dma_info_reg, vodma_i2rnd_dma_info.regValue);

	//enable fifo mode and timing mode 1:1
   	vo_i2rnd.regValue = rtd_inl(VODMA_vodma_i2rnd_reg);
	vo_i2rnd.i2rnd_flag_fifo_en = 1;
	vo_i2rnd.i2rnd_src_flag_inv = 0;
	vo_i2rnd.i2rnd_timing_mode = _1_MAIN_1_SUB;
	vo_i2rnd.i2rnd_timing_en = _ENABLE;
	IoReg_Write32(VODMA_vodma_i2rnd_reg, vo_i2rnd.regValue);

	if(display == SLR_MAIN_DISPLAY){
		//h_active = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPH_ACT_WID);
		//v_active = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN);
	       h_active = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPH_ACT_WID_PRE);
              v_active = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE);
		interlace = Scaler_DispGetStatus(SLR_MAIN_DISPLAY,SLR_DISP_INTERLACE);
		frame_rate = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_FREQ)/10;

		//repeat mask enable in interlace source, but DTV does not enable @Crixus 20160803
		vo_i2rnd_m_flag.regValue = rtd_inl(VODMA_vodma_i2rnd_m_flag_reg);
		if((interlace) && (input_src != VSC_INPUTSRC_VDEC))
			vo_i2rnd_m_flag.i2rnd_hw_mask_repeat_frame_m_en = 1;
		else
			vo_i2rnd_m_flag.i2rnd_hw_mask_repeat_frame_m_en = 0;
		IoReg_Write32(VODMA_vodma_i2rnd_m_flag_reg, vo_i2rnd_m_flag.regValue);

		i2rnd_period_update_main = 1;
	}else{
		h_active = Scaler_DispGetInputInfoByDisp(SLR_SUB_DISPLAY, SLR_INPUT_IPH_ACT_WID);
		v_active = Scaler_DispGetInputInfoByDisp(SLR_SUB_DISPLAY, SLR_INPUT_IPV_ACT_LEN);
		interlace = DbgSclrFlgTkr.multiview_sub_interlace_flag;

		//repeat mask enable in interlace source, but DTV does not enable @Crixus 20160803
		vo_i2rnd_s_flag.regValue = rtd_inl(VODMA_vodma_i2rnd_s_flag_reg);
		if((interlace) && (input_src != VSC_INPUTSRC_VDEC))
			vo_i2rnd_s_flag.i2rnd_hw_mask_repeat_frame_s_en = 1;
		else
			vo_i2rnd_s_flag.i2rnd_hw_mask_repeat_frame_s_en = 0;
		IoReg_Write32(VODMA_vodma_i2rnd_s_flag_reg, vo_i2rnd_s_flag.regValue);

		i2rnd_period_update_sub = 1;
	}

	rtd_pr_vbe_emerg("[crixus]h_active = %d, v_active = %d, interlace = %d\n", h_active, v_active, interlace);
	rtd_pr_vbe_emerg("[crixus]input_src = %d\n", input_src);
#if 0
	if(((frame_rate % 10) > 4) || ((frame_rate % 10) == 0) )
		frame_rate = frame_rate / 10;
	else
		frame_rate = (frame_rate / 10) + 1;
#else
	frame_rate = 60;
#endif

	if(h_active >= 3000){
		/*if(v_active >= 1000){//4k2k case
			vo_clock = I2RND_VO_CLOCK_600MHZ;
			fix_htotal = I2RND_VO_FIX_H_TOTAL_4K2K;
			fix_vtotal = I2RND_VO_FIX_V_TOTAL_4K2K;
		}else{//4k1k case
			vo_clock= I2RND_VO_CLOCK_600MHZ;
			fix_htotal = I2RND_VO_FIX_H_TOTAL_4K2K;
			fix_vtotal = I2RND_VO_FIX_V_TOTAL_4K1K;
		}*/
		//over 2k1k case, always uzd to 2k1k
		vo_clock = I2RND_VO_CLOCK_360MHZ;
		fix_htotal = I2RND_VO_FIX_H_TOTAL_2K1K;
		fix_vtotal = I2RND_VO_FIX_V_TOTAL_2K1K_P;
	}else{
		if(interlace == 0){//2k1k case
			vo_clock = I2RND_VO_CLOCK_360MHZ;
			fix_htotal = I2RND_VO_FIX_H_TOTAL_2K1K;
			fix_vtotal = I2RND_VO_FIX_V_TOTAL_2K1K_P;
		}else{//2k0.5k case
			vo_clock = I2RND_VO_CLOCK_180MHZ;
			fix_htotal = I2RND_VO_FIX_H_TOTAL_2K1K;
			fix_vtotal = I2RND_VO_FIX_V_TOTAL_2K1K_I;
		}
	}

	if(display == SLR_MAIN_DISPLAY){
		if((i2rnd_period_update_main == 1) && (i2rnd_period_update_sub == 0) && (interlace == 1)){
			vo_clock_main = vo_clock;
			vo_fix_htotal_main = fix_htotal;
			vo_fix_vtotal_main = fix_vtotal;
			vo_fix_hload_main = fix_htotal;
			vo_frame_rate_main = frame_rate;

			vo_clock_sub = vo_clock_main;
			vo_fix_htotal_sub = vo_fix_htotal_main;
			vo_fix_vtotal_sub = vo_fix_vtotal_main;
			vo_fix_hload_sub = vo_fix_hload_main;
			vo_frame_rate_sub = vo_frame_rate_main;

			refine_sub_period = 1;
		}
		else{
			vo_clock_main = vo_clock;
			vo_fix_htotal_main = fix_htotal;
			vo_fix_vtotal_main = fix_vtotal;
			vo_fix_hload_main = fix_htotal;
			vo_frame_rate_main = frame_rate;
		}
	}
	else{
		vo_clock_sub = vo_clock;
		vo_fix_htotal_sub = fix_htotal;
		vo_fix_vtotal_sub = fix_vtotal;
		vo_fix_hload_sub = fix_htotal;
		vo_frame_rate_sub = frame_rate;
	}

	if(i2rnd_period_update_main == 1){
		rtd_pr_vbe_emerg("@@@@@@@@@@@@@@@@@@[MAIN PERIOD REFINE]@@@@@@@@@@@@@@@@@@@@@\n");
		rtd_pr_vbe_emerg("[%s]main_vo_clock = %d, sub_vo_clock = %d, fix_htotal = %d, fix_vtotal = %d, h_load = %d, frame_rate = %d\n", __FUNCTION__, vo_clock_main, vo_clock_sub, vo_fix_htotal_main, vo_fix_vtotal_main, vo_fix_hload_main, vo_frame_rate_main);

		/*
			mac5p reference xtal clock @Crixus 20170104

			load_max = Tmaster * IMD_cmd * 27 / 1000 * 1.05
			load_min = load_max - 32

			free_period = 27000000/frame rate_main
			load_period = (free_period - 100) * src_ratio - load_max

			Default ratio setting: 0.5
		*/
		period_max = I2RND_TMASTER_TIME * I2RND_REGISTER_NUMBER * 27 / 1000 * 10500 / 10000;//refine new period formula @Crixus 20170504
		period_min = period_max - 32;
		rtd_pr_vbe_emerg("[%s]I2RND_TMASTER_TIME = %d, I2RND_REGISTER_NUMBER = %d\n", __FUNCTION__, I2RND_TMASTER_TIME, I2RND_REGISTER_NUMBER);
		rtd_pr_vbe_emerg("[%s]period_max = %d, period_min = %d\n", __FUNCTION__, period_max, period_min);


		free_period = I2RND_XTAL_CLOCK / vo_frame_rate_main;
		frame_period = (free_period - 100) / 2 - period_max;
		rtd_pr_vbe_emerg("[%s]free_period = %d, vo_frame_rate_main = %d\n", __FUNCTION__, free_period, vo_frame_rate_main);
		rtd_pr_vbe_emerg("[%s]period_load = %d\n", __FUNCTION__, frame_period);

		//VODMA i2rnd period min
	    vo_i2rnd_m_p_load.regValue = rtd_inl(VODMA_vodma_i2rnd_m_period_load_reg);
		vo_i2rnd_m_p_load.i2rnd_m_period_load = period_min;//htotal * period_min;//s0_period;//300;
	    IoReg_Write32(VODMA_vodma_i2rnd_m_period_load_reg, vo_i2rnd_m_p_load.regValue);

		//VODMA i2rnd period
	    vo_i2rnd_m_p.regValue = rtd_inl(VODMA_vodma_i2rnd_m_period_reg);
		vo_i2rnd_m_p.i2rnd_m_period = frame_period;//htotal * period_load;//s0_period - 300;
	    IoReg_Write32(VODMA_vodma_i2rnd_m_period_reg, vo_i2rnd_m_p.regValue);

		//VODMA i2rnd period max
		vodma_vodma_i2rnd_m_period_load_dbg.regValue = rtd_inl(VODMA_vodma_i2rnd_m_period_load_dbg_reg);
		vodma_vodma_i2rnd_m_period_load_dbg.i2rnd_m_period_load_max = period_max;//htotal * period_max;
		IoReg_Write32(VODMA_vodma_i2rnd_m_period_load_dbg_reg, vodma_vodma_i2rnd_m_period_load_dbg.regValue);
	}


	if((i2rnd_period_update_sub == 1) || (refine_sub_period == 1)){
		rtd_pr_vbe_emerg("@@@@@@@@@@@@@@@@@@[SUB PERIOD REFINE]@@@@@@@@@@@@@@@@@@@@@\n");
		rtd_pr_vbe_emerg("[%s]main vo_clock = %d, sub vo_clock = %d, fix_htotal = %d, fix_vtotal = %d, h_load = %d, frame_rate = %d\n", __FUNCTION__, vo_clock_main, vo_clock_sub, vo_fix_htotal_sub, vo_fix_vtotal_sub, vo_fix_hload_sub, vo_frame_rate_sub);

		period_max = I2RND_TMASTER_TIME * I2RND_REGISTER_NUMBER * 27 / 1000 * 10500 / 10000;
		period_min = period_max - 32;
		rtd_pr_vbe_emerg("[%s]I2RND_TMASTER_TIME = %d, I2RND_REGISTER_NUMBER = %d\n", __FUNCTION__, I2RND_TMASTER_TIME, I2RND_REGISTER_NUMBER);
		rtd_pr_vbe_emerg("[%s]period_max = %d, period_min = %d\n", __FUNCTION__, period_max, period_min);


		free_period = I2RND_XTAL_CLOCK / vo_frame_rate_sub;
		frame_period = (free_period - 100) / 2 - period_max;
		rtd_pr_vbe_emerg("[%s]free_period = %d, vo_frame_rate_main = %d\n", __FUNCTION__, free_period, vo_frame_rate_main);
		rtd_pr_vbe_emerg("[%s]period_load = %d\n", __FUNCTION__, frame_period);

		//sub period min
	    vo_i2rnd_s_p_load.regValue = rtd_inl(VODMA_vodma_i2rnd_s_period_load_reg);
		vo_i2rnd_s_p_load.i2rnd_s_period_load = period_min;//htotal * period_min;
	    IoReg_Write32(VODMA_vodma_i2rnd_s_period_load_reg, vo_i2rnd_s_p_load.regValue);

		//sub period load
	    vo_i2rnd_s_p.regValue = rtd_inl(VODMA_vodma_i2rnd_s_period_reg);
		vo_i2rnd_s_p.i2rnd_s_period = frame_period;//htotal * period_load;
	    IoReg_Write32(VODMA_vodma_i2rnd_s_period_reg, vo_i2rnd_s_p.regValue);

		//sub period max
		vodma_vodma_i2rnd_s_period_load_dbg.regValue = rtd_inl(VODMA_vodma_i2rnd_s_period_load_dbg_reg);
		vodma_vodma_i2rnd_s_period_load_dbg.i2rnd_s_period_load_max = period_max;//htotal * period_max;
		IoReg_Write32(VODMA_vodma_i2rnd_s_period_load_dbg_reg, vodma_vodma_i2rnd_s_period_load_dbg.regValue);
	}

	refine_sub_period = 0;
	//drvif_scaler_i2rnd_vodma1_flag_fifo_prepare();
}

#if 0
void drvif_scaler_i2rnd_cal_total_cmd_reg(void){
	unsigned int cmd_number = 0;
	unsigned int i = 0;
	unsigned int j = 0;

	i = 0;
	j = VODMA_REGISTER_NUM_0;
	for(i=0;i<j;i++){
		if(VODMA_0[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = VGIP_REGISTER_NUM_0;
	for(i=0;i<j;i++){
		if(vgip_0[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = COLOR_MB_PEAKING_REGISTER_NUM_0;
	for(i=0;i<j;i++){
		if(color_mb_peaking_0[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = DE_XCXL_REGISTER_NUM_0;
	for(i=0;i<j;i++){
		if(De_xcxl_0[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = DFILTER_REGISTER_NUM_0;
	for(i=0;i<j;i++){
		if(Dfilter_0[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = DI_REGISTER_NUM_0;
	for(i=0;i<j;i++){
		if(di_0[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = DI_REGISTER_NUM_1;
	for(i=0;i<j;i++){
		if(di_1[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = DI_REGISTER_NUM_2;
	for(i=0;i<j;i++){
		if(di_2[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = DI_REGISTER_NUM_0;
	for(i=0;i<j;i++){
		if(VODMA_0[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = DI_REGISTER_NUM_3;
	for(i=0;i<j;i++){
		if(di_3[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = DI_REGISTER_NUM_4;
	for(i=0;i<j;i++){
		if(di_4[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = DI_REGISTER_NUM_5;
	for(i=0;i<j;i++){
		if(di_5[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = DI_REGISTER_NUM_6;
	for(i=0;i<j;i++){
		if(di_6[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = HISTOGRAM_REGISTER_NUM_0;
	for(i=0;i<j;i++){
		if(Histogram_0[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = HSD_DITHER_REGISTER_NUM_0;
	for(i=0;i<j;i++){
		if(Hsd_dither_0[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = IDCTI_REGISTER_NUM_0;
	for(i=0;i<j;i++){
		if(IDCTI_0[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = IEDGE_SMOOTH_REGISTER_NUM_0;
	for(i=0;i<j;i++){
		if(iedge_smooth_0[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = IPQ_DECONTOUR_REGISTER_NUM_0;
	for(i=0;i<j;i++){
		if(ipq_decontour_0[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = MAIN_DITHER_REGISTER_NUM_0;
	for(i=0;i<j;i++){
		if(Main_Dither_0[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = MPEGNR_REGISTER_NUM_0;
	for(i=0;i<j;i++){
		if(Mpegnr_0[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = NR_REGISTER_NUM_0;
	for(i=0;i<j;i++){
		if(nr_0[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = NR_REGISTER_NUM_1;
	for(i=0;i<j;i++){
		if(nr_1[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = PEAKING_REGISTER_NUM_0;
	for(i=0;i<j;i++){
		if(Peaking_0[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = PROFILE_REGISTER_NUM_0;
	for(i=0;i<j;i++){
		if(Profile_0[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = PROFILE_REGISTER_NUM_1;
	for(i=0;i<j;i++){
		if(Profile_1[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = RGB2YUV_REGISTER_NUM_0;
	for(i=0;i<j;i++){
		if(rgb2yuv_0[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = RGB2YUV_REGISTER_NUM_1;
	for(i=0;i<j;i++){
		if(rgb2yuv_1[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = RINGFILTER_REGISTER_NUM_0;
	for(i=0;i<j;i++){
		if(ringfilter_0[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = SCALEDOWN_REGISTER_NUM_0;
	for(i=0;i<j;i++){
		if(Scaledown_0[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = SCALEDOWN_REGISTER_NUM_1;
	for(i=0;i<j;i++){
		if(Scaledown_1[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = MDOMAIN_CAP_REGISTER_NUM_0;
	for(i=0;i<j;i++){
		if(Mdomain_cap_0[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = MDOMAIN_CAP_REGISTER_NUM_1;
	for(i=0;i<j;i++){
		if(Mdomain_cap_1[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = MDOMAIN_CAP_REGISTER_NUM_2;
	for(i=0;i<j;i++){
		if(Mdomain_cap_2[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = MAIN_TOP_CTL_REGISTER_NUM_0;
	for(i=0;i<j;i++){
		if(HDR_VTOP_0[i])
			cmd_number = cmd_number + 1;
	}

	i = 0;
	j = MAIN_TOP_CTL_REGISTER_NUM_1;
	for(i=0;i<j;i++){
		if(HDR_VTOP_1[i])
			cmd_number = cmd_number + 1;
	}

	rtd_pr_vbe_emerg("[I2RND]Total cmd register number = %d\n", cmd_number);
}
#endif

void drvif_scaler_i2rnd_change_fw_mode(void)
{
	vodma_vodma_i2rnd_m_flag_RBUS vo_i2rnd_m_flag;

	vo_i2rnd_m_flag.regValue = rtd_inl(VODMA_vodma_i2rnd_m_flag_reg);
	vo_i2rnd_m_flag.i2rnd_m_field_fw = 0;
	if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC){//DTV use field free @Crixus 20160805
		vo_i2rnd_m_flag.i2rnd_m_block_free_run = 0;
		vo_i2rnd_m_flag.i2rnd_m_block_sel = 1;
	}
	else{
		vo_i2rnd_m_flag.i2rnd_m_block_free_run = 0;
		vo_i2rnd_m_flag.i2rnd_m_block_sel = 1;
	}
	vo_i2rnd_m_flag.i2rnd_m_field_fw = 0;
	if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC){//DTV use field free @Crixus 20160805
		vo_i2rnd_m_flag.i2rnd_m_field_free_run = 1;
		vo_i2rnd_m_flag.i2rnd_m_field_sel = 3;
	}
	else{
		vo_i2rnd_m_flag.i2rnd_m_field_free_run = 0;
		vo_i2rnd_m_flag.i2rnd_m_field_sel = 1;
	}
	vo_i2rnd_m_flag.i2rnd_m_lflag_fw = 0;
	if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC){//DTV use field free @Crixus 20160805
		vo_i2rnd_m_flag.i2rnd_m_lflag_free_run = 0;
		vo_i2rnd_m_flag.i2rnd_m_lflag_sel = 1;
	}
	else{
		vo_i2rnd_m_flag.i2rnd_m_lflag_free_run = 0;
		vo_i2rnd_m_flag.i2rnd_m_lflag_sel = 1;
	}
	IoReg_Write32(VODMA_vodma_i2rnd_m_flag_reg, vo_i2rnd_m_flag.regValue);

}

void drvif_scaler_i2rnd_step_size(unsigned int blk_size)
{
    /////  block size = (DMA.num*128/8)*N
    i2rnd_step_size.s0_vo_blk_size = blk_size;
    i2rnd_step_size.s1_vo_blk_size = blk_size;
}

/*
1.Calculate the I2run DMA setting num/length and blk size, Blk num
*/
void drvif_scaler_i2rnd_dma_setting(unsigned int cmd_num, unsigned int blk_size)
{

	/*
	block size = 592 * 8 = 4736 bits
	Queue buffer number = cmd_num = 16
	DMA length = 16

	1. register number = (4736 / 128 * 2) * 16 = 1184
	2. total block size = 4736 * 16 = 75776 bits = (1184 / 2) * 128bits
	3. DMA number = (block size / DMA length) / 64   (unit. 64bits)
	                       = (4736 / 16) / 64
	                       = 4
	*/
	pst_i2rnd_dma_buf_num_RBUS dma_buf_num;
	pst_i2rnd_dma_ctrl_RBUS	dma_ctrl;
	pst_i2rnd_dma_num0_RBUS	dma_num0;
	pst_i2rnd_dma_num1_RBUS	dma_num1;
/*	unsigned int blk_size_mod = 0;

	//align
	blk_size_mod = blk_size % (64 * 2);
	if(blk_size_mod != 0){
		blk_size = blk_size + (64 * 2 - blk_size_mod);
	}
*/
	rtd_pr_vbe_emerg("[crixus]cmd_num = %d, blk_size = %d\n", cmd_num, blk_size);


	drvif_scaler_i2rnd_step_size(blk_size);

	//start address
	IoReg_Write32(PST_I2RND_DMA_ADDR0_reg, i2rnd_st_addr.s0_vo_st_addr);
	IoReg_Write32(PST_I2RND_DMA_ADDR2_reg, i2rnd_st_addr.s1_vo_st_addr);
	//block step
	IoReg_Write32(PST_I2RND_DMA_STEP0_reg, i2rnd_step_size.s0_vo_blk_size);
	IoReg_Write32(PST_I2RND_DMA_STEP2_reg, i2rnd_step_size.s1_vo_blk_size);

	//dma number
	dma_buf_num.regValue = rtd_inl(PST_I2RND_DMA_BUF_NUM_reg);
	dma_buf_num.dma_buffer_num = cmd_num;
	IoReg_Write32(PST_I2RND_DMA_BUF_NUM_reg, dma_buf_num.regValue);

	///////////////////////////////////////////
	// WATER/NUM/LENGTH
	///////////////////////////////////////////
	dma_ctrl.regValue = rtd_inl(PST_I2RND_DMA_CTRL_reg);
	dma_ctrl.dma_wtlvl = I2RND_DMA_WATERLEVEL;
	dma_ctrl.dma_len = I2RND_DMA_LENGTH;
	IoReg_Write32(PST_I2RND_DMA_CTRL_reg, dma_ctrl.regValue);

	dma_num0.regValue = rtd_inl(PST_I2RND_DMA_NUM0_reg);
	dma_num0.dma_num_i2rnd_src0_vo = i2rnd_step_size.s0_vo_blk_size/8/dma_ctrl.dma_len;//burst length 64bits=8bytes
	IoReg_Write32(PST_I2RND_DMA_NUM0_reg, dma_num0.regValue);

	dma_num1.regValue = rtd_inl(PST_I2RND_DMA_NUM1_reg);
	dma_num1.dma_num_i2rnd_src1_vo = i2rnd_step_size.s1_vo_blk_size/8/dma_ctrl.dma_len;//burst length 64bits=8bytes
	IoReg_Write32(PST_I2RND_DMA_NUM1_reg, dma_num1.regValue);

	////////////////////////////////////////
	//  update current status
	////////////////////////////////////////
	i2rnd_st_addr.s0_vo_cur_addr = i2rnd_st_addr.s0_vo_st_addr;
	i2rnd_st_addr.s1_vo_cur_addr = i2rnd_st_addr.s1_vo_st_addr;
	i2rnd_cmd_buf.s0_vo_cur_cmd = 0;
	i2rnd_cmd_buf.s1_vo_cur_cmd = 0;
}

/*
1.send RPC the cmd queue buffer info from SCPU to VCPU
2.record I2run S0,S1 Queue each IP start register setting start point in SCPU
*/
unsigned char	Scaler_i2rnd_setbuffer(void)
{
	unsigned int *pulTemp = NULL;
	unsigned int i=0, ulItemCount = 0;
	I2RND_START_ADDR i2rnd_addr_info={0};
#if 0
	if (i2rnd_st_addr.s0_vo_st_vir_addr == 0) {
		i2rnd_st_addr.s0_vo_st_vir_addr= (unsigned int)dvr_malloc_specific(I2RND_QUEUE_SIZE, GFP_DCU1_FIRST);
		i2rnd_st_addr.s0_vo_st_addr = (unsigned int)dvr_to_phys((void*)i2rnd_st_addr.s0_vo_st_vir_addr);
	}
	if (i2rnd_st_addr.s1_vo_st_vir_addr == 0) {
		i2rnd_st_addr.s1_vo_st_vir_addr = (unsigned int)dvr_malloc_specific(I2RND_QUEUE_SIZE, GFP_DCU1_FIRST);
		i2rnd_st_addr.s1_vo_st_addr = (unsigned int)dvr_to_phys((void*)i2rnd_st_addr.s1_vo_st_vir_addr);
	}
#endif
	if (i2rnd_st_addr.s0_vo_st_addr ==0 || i2rnd_st_addr.s1_vo_st_addr == 0) {
		rtd_pr_vbe_debug(" i2rnd not alloc memroy\n");
		return 0;
	}
	i2rnd_addr_info.s0_vo_st_addr = i2rnd_st_addr.s0_vo_st_addr;
	i2rnd_addr_info.s1_vo_st_addr = i2rnd_st_addr.s1_vo_st_addr;
	i2rnd_addr_info.i2rnd_enable_addr= i2rnd_st_addr.i2rnd_enable_addr;
	i2rnd_addr_info.write_cmd_num_addr = i2rnd_st_addr.write_cmd_num_addr;
	i2rnd_addr_info.read_cmd_num_addr = i2rnd_st_addr.read_cmd_num_addr;
	i2rnd_addr_info.cmd_buffer_size = i2rnd_st_addr.cmd_buffer_size;
	i2rnd_addr_info.s0_vo_cur_addr =  i2rnd_st_addr.s0_vo_st_addr;
	i2rnd_addr_info.s1_vo_cur_addr =  i2rnd_st_addr.s1_vo_st_addr;
	rtd_pr_vbe_emerg("i2rnd s0_phy : %x ,s1 : %x\n",(i2rnd_st_addr.s0_vo_st_addr),(i2rnd_st_addr.s1_vo_st_addr));
	rtd_pr_vbe_emerg("i2rnd s0 _vir: %x ,s1 : %x\n",(i2rnd_scpu_vir_addr.s0_vo_st_vir_addr),(i2rnd_scpu_vir_addr.s1_vo_st_vir_addr));
	pulTemp = (unsigned int*)Scaler_GetShareMemVirAddr(SCALERIOC_I2RND_SET_BUFFER);
	ulItemCount = sizeof(I2RND_START_ADDR) /  sizeof(unsigned int);
	if (pulTemp) {
		memcpy(pulTemp, &i2rnd_addr_info, sizeof(I2RND_START_ADDR));
		for (i = 0; i < ulItemCount; i++)
				pulTemp[i] = htonl(pulTemp[i]);

		if (0 != Scaler_SendRPC(SCALERIOC_I2RND_SET_BUFFER,0,0))
		{
			rtd_pr_vbe_emerg("set buffer address for i2rnd fail !!!\n");
			return 0;

		}
	}

	i2rnd_scpu_vir_addr.s0_vgip_st_vir_addr			= i2rnd_scpu_vir_addr.s0_vo_st_vir_addr		+ VODMA_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_color_mb_peaking_st_vir_addr			= i2rnd_scpu_vir_addr.s0_vgip_st_vir_addr		+ VGIP_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_dexcxl_st_vir_addr		= i2rnd_scpu_vir_addr.s0_color_mb_peaking_st_vir_addr		+ COLOR_MB_PEAKING_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_dfilter_st_vir_addr			= i2rnd_scpu_vir_addr.s0_dexcxl_st_vir_addr	+ DE_XCXL_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_di0_st_vir_addr			= i2rnd_scpu_vir_addr.s0_dfilter_st_vir_addr		+ DFILTER_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_di1_st_vir_addr				= i2rnd_scpu_vir_addr.s0_di0_st_vir_addr		+ DI_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_di2_st_vir_addr				= i2rnd_scpu_vir_addr.s0_di1_st_vir_addr			+ DI_REGISTER_NUM_1 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_di3_st_vir_addr		= i2rnd_scpu_vir_addr.s0_di2_st_vir_addr			+ DI_REGISTER_NUM_2 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_di4_st_vir_addr			= i2rnd_scpu_vir_addr.s0_di3_st_vir_addr	+ DI_REGISTER_NUM_3 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_di5_st_vir_addr			= i2rnd_scpu_vir_addr.s0_di4_st_vir_addr	+ DI_REGISTER_NUM_4 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_di6_st_vir_addr			= i2rnd_scpu_vir_addr.s0_di5_st_vir_addr	+ DI_REGISTER_NUM_5 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_histogram_st_vir_addr			= i2rnd_scpu_vir_addr.s0_di6_st_vir_addr		+ DI_REGISTER_NUM_6 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_hsddither_st_vir_addr			= i2rnd_scpu_vir_addr.s0_histogram_st_vir_addr		+ HISTOGRAM_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_idcti_st_vir_addr			= i2rnd_scpu_vir_addr.s0_hsddither_st_vir_addr		+ HSD_DITHER_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_iedgesmooth_st_vir_addr			= i2rnd_scpu_vir_addr.s0_idcti_st_vir_addr		+ IDCTI_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_ipq_decontour_st_vir_addr			= i2rnd_scpu_vir_addr.s0_iedgesmooth_st_vir_addr		+ IEDGE_SMOOTH_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_main_dither_st_vir_addr			= i2rnd_scpu_vir_addr.s0_ipq_decontour_st_vir_addr		+ IPQ_DECONTOUR_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_mpegnr_st_vir_addr		= i2rnd_scpu_vir_addr.s0_main_dither_st_vir_addr		+ MAIN_DITHER_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_nr0_st_vir_addr				= i2rnd_scpu_vir_addr.s0_mpegnr_st_vir_addr	+ MPEGNR_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_nr1_st_vir_addr				= i2rnd_scpu_vir_addr.s0_nr0_st_vir_addr	+ NR_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_peaking_st_vir_addr				= i2rnd_scpu_vir_addr.s0_nr1_st_vir_addr			+ NR_REGISTER_NUM_1 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_profile0_st_vir_addr		= i2rnd_scpu_vir_addr.s0_peaking_st_vir_addr			+ PEAKING_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_profile1_st_vir_addr				= i2rnd_scpu_vir_addr.s0_profile0_st_vir_addr	+ PROFILE_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_rgb2yuv0_st_vir_addr		= i2rnd_scpu_vir_addr.s0_profile1_st_vir_addr			+ PROFILE_REGISTER_NUM_1 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_rgb2yuv1_st_vir_addr				= i2rnd_scpu_vir_addr.s0_rgb2yuv0_st_vir_addr	+ RGB2YUV_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_ringfilter_st_vir_addr			= i2rnd_scpu_vir_addr.s0_rgb2yuv1_st_vir_addr			+ RGB2YUV_REGISTER_NUM_1 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_uzd0_st_vir_addr			= i2rnd_scpu_vir_addr.s0_ringfilter_st_vir_addr		+ RINGFILTER_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_uzd1_st_vir_addr			= i2rnd_scpu_vir_addr.s0_uzd0_st_vir_addr		+ SCALEDOWN_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_mcap0_st_vir_addr			= i2rnd_scpu_vir_addr.s0_uzd1_st_vir_addr		+ SCALEDOWN_REGISTER_NUM_1 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_mcap1_st_vir_addr			= i2rnd_scpu_vir_addr.s0_mcap0_st_vir_addr		+ MDOMAIN_CAP_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_mcap2_st_vir_addr			= i2rnd_scpu_vir_addr.s0_mcap1_st_vir_addr		+ MDOMAIN_CAP_REGISTER_NUM_1 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_vtop0_st_vir_addr			= i2rnd_scpu_vir_addr.s0_mcap2_st_vir_addr		+ MDOMAIN_CAP_REGISTER_NUM_2 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s0_vtop1_st_vir_addr			= i2rnd_scpu_vir_addr.s0_vtop0_st_vir_addr		+ MAIN_TOP_CTL_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;

	i2rnd_scpu_vir_addr.s1_vgip_st_vir_addr			= i2rnd_scpu_vir_addr.s1_vo_st_vir_addr		+ VODMA_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_color_mb_peaking_st_vir_addr			= i2rnd_scpu_vir_addr.s1_vgip_st_vir_addr		+ VGIP_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_dexcxl_st_vir_addr		= i2rnd_scpu_vir_addr.s1_color_mb_peaking_st_vir_addr		+ COLOR_MB_PEAKING_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_dfilter_st_vir_addr			= i2rnd_scpu_vir_addr.s1_dexcxl_st_vir_addr	+ DE_XCXL_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_di0_st_vir_addr			= i2rnd_scpu_vir_addr.s1_dfilter_st_vir_addr		+ DFILTER_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_di1_st_vir_addr				= i2rnd_scpu_vir_addr.s1_di0_st_vir_addr		+ DI_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_di2_st_vir_addr				= i2rnd_scpu_vir_addr.s1_di1_st_vir_addr			+ DI_REGISTER_NUM_1 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_di3_st_vir_addr		= i2rnd_scpu_vir_addr.s1_di2_st_vir_addr			+ DI_REGISTER_NUM_2 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_di4_st_vir_addr			= i2rnd_scpu_vir_addr.s1_di3_st_vir_addr	+ DI_REGISTER_NUM_3 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_di5_st_vir_addr			= i2rnd_scpu_vir_addr.s1_di4_st_vir_addr	+ DI_REGISTER_NUM_4 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_di6_st_vir_addr			= i2rnd_scpu_vir_addr.s1_di5_st_vir_addr	+ DI_REGISTER_NUM_5 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_histogram_st_vir_addr			= i2rnd_scpu_vir_addr.s1_di6_st_vir_addr		+ DI_REGISTER_NUM_6 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_hsddither_st_vir_addr			= i2rnd_scpu_vir_addr.s1_histogram_st_vir_addr		+ HISTOGRAM_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_idcti_st_vir_addr			= i2rnd_scpu_vir_addr.s1_hsddither_st_vir_addr		+ HSD_DITHER_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_iedgesmooth_st_vir_addr			= i2rnd_scpu_vir_addr.s1_idcti_st_vir_addr		+ IDCTI_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_ipq_decontour_st_vir_addr			= i2rnd_scpu_vir_addr.s1_iedgesmooth_st_vir_addr		+ IEDGE_SMOOTH_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_main_dither_st_vir_addr			= i2rnd_scpu_vir_addr.s1_ipq_decontour_st_vir_addr		+ IPQ_DECONTOUR_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_mpegnr_st_vir_addr		= i2rnd_scpu_vir_addr.s1_main_dither_st_vir_addr		+ MAIN_DITHER_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_nr0_st_vir_addr				= i2rnd_scpu_vir_addr.s1_mpegnr_st_vir_addr	+ MPEGNR_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_nr1_st_vir_addr				= i2rnd_scpu_vir_addr.s1_nr0_st_vir_addr	+ NR_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_peaking_st_vir_addr				= i2rnd_scpu_vir_addr.s1_nr1_st_vir_addr			+ NR_REGISTER_NUM_1 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_profile0_st_vir_addr		= i2rnd_scpu_vir_addr.s1_peaking_st_vir_addr			+ PEAKING_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_profile1_st_vir_addr				= i2rnd_scpu_vir_addr.s1_profile0_st_vir_addr	+ PROFILE_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_rgb2yuv0_st_vir_addr		= i2rnd_scpu_vir_addr.s1_profile1_st_vir_addr			+ PROFILE_REGISTER_NUM_1 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_rgb2yuv1_st_vir_addr				= i2rnd_scpu_vir_addr.s1_rgb2yuv0_st_vir_addr	+ RGB2YUV_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_ringfilter_st_vir_addr			= i2rnd_scpu_vir_addr.s1_rgb2yuv1_st_vir_addr			+ RGB2YUV_REGISTER_NUM_1 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_uzd0_st_vir_addr			= i2rnd_scpu_vir_addr.s1_ringfilter_st_vir_addr		+ RINGFILTER_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_uzd1_st_vir_addr			= i2rnd_scpu_vir_addr.s1_uzd0_st_vir_addr		+ SCALEDOWN_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_mcap0_st_vir_addr			= i2rnd_scpu_vir_addr.s1_uzd1_st_vir_addr		+ SCALEDOWN_REGISTER_NUM_1 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_mcap1_st_vir_addr			= i2rnd_scpu_vir_addr.s1_mcap0_st_vir_addr		+ MDOMAIN_CAP_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_mcap2_st_vir_addr			= i2rnd_scpu_vir_addr.s1_mcap1_st_vir_addr		+ MDOMAIN_CAP_REGISTER_NUM_1 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_vtop0_st_vir_addr			= i2rnd_scpu_vir_addr.s1_mcap2_st_vir_addr		+ MDOMAIN_CAP_REGISTER_NUM_2 * I2RND_REGISTER_SIZE;
	i2rnd_scpu_vir_addr.s1_vtop1_st_vir_addr			= i2rnd_scpu_vir_addr.s1_vtop0_st_vir_addr		+ MAIN_TOP_CTL_REGISTER_NUM_0 * I2RND_REGISTER_SIZE;

	i2rnd_scpu_vir_addr.i2rnd_enable_vir_addr			= i2rnd_scpu_vir_addr.s0_vo_st_vir_addr 		+ i2run_queue_size;
	i2rnd_scpu_vir_addr.i2rnd_write_cmd_vir_addr		= i2rnd_scpu_vir_addr.i2rnd_enable_vir_addr 	+ I2RND_ENABLE_ADDR_SIZE;
	i2rnd_scpu_vir_addr.i2rnd_read_cmd_vir_addr		= i2rnd_scpu_vir_addr.i2rnd_write_cmd_vir_addr 	+ I2RND_WRITE_CMD_ADDR_SIZE;
	i2rnd_scpu_vir_addr.i2rnd_pst_pnt_counter_vir_addr		= i2rnd_scpu_vir_addr.i2rnd_read_cmd_vir_addr 	+ I2RND_READ_CMD_ADDR_SIZE;
	i2rnd_scpu_vir_addr.i2rnd_force_write_cmd_addr		= i2rnd_scpu_vir_addr.i2rnd_pst_pnt_counter_vir_addr	+ I2RND_PST_POINT_COUNTER_SIZE;
	i2rnd_scpu_vir_addr.i2rnd_s1_write_cmd_vir_addr		= i2rnd_scpu_vir_addr.i2rnd_force_write_cmd_addr	+ I2RND_FORCE_WRITE_CMD_SIZE;
	i2rnd_scpu_vir_addr.i2rnd_s1_read_cmd_vir_addr		= i2rnd_scpu_vir_addr.i2rnd_s1_write_cmd_vir_addr	+ I2RND_S1_WRITE_CMD_ADDR_SIZE;
	i2rnd_scpu_vir_addr.i2rnd_s1_pst_pnt_counter_vir_addr = i2rnd_scpu_vir_addr.i2rnd_s1_read_cmd_vir_addr + I2RND_S1_READ_CMD_ADDR_SIZE;
	i2rnd_scpu_vir_addr.i2rnd_vo1_refine_status_vir_addr = i2rnd_scpu_vir_addr.i2rnd_s1_pst_pnt_counter_vir_addr + I2RND_PST_POINT_COUNTER_SIZE;
	i2rnd_scpu_vir_addr.i2rnd_s0_idomain_done_status_vir_addr = i2rnd_scpu_vir_addr.i2rnd_vo1_refine_status_vir_addr + I2RND_PST_POINT_COUNTER_SIZE;
	rtd_pr_vbe_emerg("s0_end_addr=%x\n",i2rnd_scpu_vir_addr.s0_vo_st_vir_addr +i2run_queue_size);
	rtd_pr_vbe_emerg("s1_end_addr=%x\n",i2rnd_scpu_vir_addr.s1_vo_st_vir_addr +i2run_queue_size);
	//rtd_pr_vbe_emerg("i2rnd_enable_addr=%x->%x\n",i2rnd_scpu_vir_addr.i2rnd_enable_vir_addr,i2rnd_st_addr.i2rnd_enable_addr);
	return 0;
}

unsigned int Scaler_i2rnd_send_apvr_info(void)
{
	unsigned int *pulTemp = NULL;
	unsigned int i=0, ulItemCount = 0;

	I2RND_APVR_INFO i2rnd_apvr_info;
	i2rnd_apvr_info.i2rnd_enable = Scaler_I2rnd_get_enable();
	i2rnd_get_mcap2_address(&i2rnd_apvr_info.s1_buf_addr1, &i2rnd_apvr_info.s1_buf_addr2, &i2rnd_apvr_info.s1_buf_addr3);

	pulTemp = (unsigned int*)Scaler_GetShareMemVirAddr(SCALERIOC_I2RND_SET_APVR_INFO);
	ulItemCount = sizeof(I2RND_APVR_INFO) /  sizeof(unsigned int);
	if (pulTemp) {
		memcpy(pulTemp, &i2rnd_apvr_info, sizeof(I2RND_APVR_INFO));
		for (i = 0; i < ulItemCount; i++)
				pulTemp[i] = htonl(pulTemp[i]);

		if (0 != Scaler_SendRPC(SCALERIOC_I2RND_SET_APVR_INFO,0,0))
		{
			rtd_pr_vbe_emerg("send i2run apvr info fail !!!\n");
			return 0;

		}
	}

	return 0;
}

unsigned int Scaler_i2rnd_IP_register_get_cur_write_addr(unsigned int addr,unsigned char display)
{
	unsigned int cur_addr =0;
	unsigned int register_offset_addr =0;
	if (i2rnd_scpu_vir_addr.s0_vo_st_vir_addr && i2rnd_scpu_vir_addr.s1_vo_st_vir_addr) {
		if (VGIP_register_range_0(addr)) {
			if (VGIP_DDR_range_0(addr)) {
				register_offset_addr = ((addr-VGIP_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_vgip_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_vgip_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (VODMA_register_range_0(addr)) {
			if (VODMA_DDR_range_0(addr)) {
				register_offset_addr = ((addr-VODMA_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_vo_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_vo_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (COLOR_MB_PEAKING_register_range_0(addr)) {
			if (COLOR_MB_PEAKING_DDR_range_0(addr)) {
				register_offset_addr = ((addr-COLOR_MB_PEAKING_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_color_mb_peaking_st_vir_addr)+ register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_color_mb_peaking_st_vir_addr)+ register_offset_addr;
				}
			}
		} else if (DE_XCXL_register_range_0(addr)) {
			if (DE_XCXL_DDR_range_0(addr)) {
				register_offset_addr = ((addr-DE_XCXL_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_dexcxl_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_dexcxl_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (DFILTER_register_range_0(addr)) {
			if (DFILTER_DDR_range_0(addr)) {
				register_offset_addr = ((addr-DFILTER_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_dfilter_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_dfilter_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (DI_register_range_0(addr)) {
			if (DI_DDR_range_0(addr)) {
				register_offset_addr = ((addr-DI_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_di0_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_di0_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (DI_register_range_1(addr)) {
			if (DI_DDR_range_1(addr)) {
				register_offset_addr = ((addr-DI_START_REGISTER_1) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_di1_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_di1_st_vir_addr) + register_offset_addr;
				}
			}
		}  else if (DI_register_range_2(addr)) {
			if (DI_DDR_range_2(addr)) {
				register_offset_addr = ((addr-DI_START_REGISTER_2) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_di2_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_di2_st_vir_addr) + register_offset_addr;
				}
			}
		}  else if (DI_register_range_3(addr)) {
			if (DI_DDR_range_3(addr)) {
				register_offset_addr = ((addr-DI_START_REGISTER_3) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_di3_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_di3_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (DI_register_range_4(addr)) {
			if (DI_DDR_range_4(addr)) {
				register_offset_addr = ((addr-DI_START_REGISTER_4) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_di4_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_di4_st_vir_addr) + register_offset_addr;
				}
			}
		}  else if (DI_register_range_5(addr)) {
			if (DI_DDR_range_5(addr)) {
				register_offset_addr = ((addr-DI_START_REGISTER_5) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_di5_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_di5_st_vir_addr) + register_offset_addr;
				}
			}
		}  else if (DI_register_range_6(addr)) {
			if (DI_DDR_range_6(addr)) {
				register_offset_addr = ((addr-DI_START_REGISTER_6) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_di6_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_di6_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (HISTOGRAM_register_range_0(addr)) {
			if (HISTOGRAM_DDR_range_0(addr)) {
				register_offset_addr = ((addr-HISTOGRAM_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_histogram_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_histogram_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (HSD_DITHER_register_range_0(addr)) {
			if (HSD_DITHER_DDR_range_0(addr)) {
				register_offset_addr = ((addr-HSD_DITHER_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_hsddither_st_vir_addr)+ register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_hsddither_st_vir_addr)+ register_offset_addr;
				}
			}
		} else if (IDCTI_register_range_0(addr)) {
			if (IDCTI_DDR_range_0(addr)) {
				register_offset_addr = ((addr-IDCTI_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_idcti_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_idcti_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (IEDGE_SMOOTH_register_range_0(addr)) {
			if (IEDGE_SMOOTH_DDR_range_0(addr)) {
				register_offset_addr = ((addr-IEDGE_SMOOTH_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_iedgesmooth_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_iedgesmooth_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (IPQ_DECONTOUR_register_range_0(addr)) {
			if (IPQ_DECONTOUR_DDR_range_0(addr)) {
				register_offset_addr = ((addr-IPQ_DECONTOUR_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_ipq_decontour_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_ipq_decontour_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (MAIN_DITHER_register_range_0(addr)) {
			if (MAIN_DITHER_register_range_0(addr)) {
				register_offset_addr = ((addr-MAIN_DITHER_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_main_dither_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_main_dither_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (MPEGNR_register_range_0(addr)) {
			if (MPEGNR_DDR_range_0(addr)) {
				register_offset_addr = ((addr-MPEGNR_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr = Scaler_i2rnd_cmd_buffer_write_offset_addr( i2rnd_scpu_vir_addr.s1_mpegnr_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_mpegnr_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (NR_register_range_0(addr)) {
			if (NR_DDR_range_0(addr)) {
				register_offset_addr = ((addr-NR_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_nr0_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_nr0_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (NR_register_range_1(addr)) {
			if (NR_DDR_range_1(addr)) {
				register_offset_addr = ((addr-NR_START_REGISTER_1) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_nr1_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_nr1_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (PEAKING_register_range_0(addr)) {//remove from merlin4, just return 0
			if (PEAKING_DDR_range_0(addr)) {
				register_offset_addr = ((addr-PEAKING_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_peaking_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_peaking_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (PROFILE_register_range_0(addr)) {
			if (PROFILE_DDR_range_0(addr)) {
				register_offset_addr = ((addr-PROFILE_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_profile0_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_profile0_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (PROFILE_register_range_1(addr)) {
			if (PROFILE_DDR_range_1(addr)) {
				register_offset_addr = ((addr-PROFILE_START_REGISTER_1) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_profile1_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_profile1_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (RGB2YUV_register_range_0(addr)) {
			if (RGB2YUV_DDR_range_0(addr)) {
				register_offset_addr = ((addr-RGB2YUV_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_rgb2yuv0_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_rgb2yuv0_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (RGB2YUV_register_range_1(addr)) {
			if (RGB2YUV_DDR_range_1(addr)) {
				register_offset_addr = ((addr-RGB2YUV_START_REGISTER_1) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_rgb2yuv1_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_rgb2yuv1_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (RINGFILTER_register_range_0(addr)) {
			if (RINGFILTER_DDR_range_0(addr)) {
				register_offset_addr = ((addr-RINGFILTER_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_ringfilter_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_ringfilter_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (SCALEDOWN_register_range_0(addr)) {
			if (SCALEDOWN_DDR_range_0(addr)) {
				register_offset_addr = ((addr-SCALEDOWN_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_uzd0_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_uzd0_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (SCALEDOWN_register_range_1(addr)) {
			if (SCALEDOWN_DDR_range_1(addr)) {
				register_offset_addr = ((addr-SCALEDOWN_START_REGISTER_1) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_uzd1_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_uzd1_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (MDOMAIN_CAP_register_range_0(addr)) {
			if (MDOMAIN_CAP_DDR_range_0(addr)) {
				register_offset_addr = ((addr-MDOMAIN_CAP_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_mcap0_st_vir_addr)+ register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_mcap0_st_vir_addr)+ register_offset_addr;
				}
			}
		} else if (MDOMAIN_CAP_register_range_1(addr)) {
			if (MDOMAIN_CAP_DDR_range_1(addr)) {
				register_offset_addr = ((addr-MDOMAIN_CAP_START_REGISTER_1) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_mcap1_st_vir_addr)+ register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_mcap1_st_vir_addr)+ register_offset_addr;
				}
			}
		}  else if (MDOMAIN_CAP_register_range_2(addr)) {
			if (MDOMAIN_CAP_DDR_range_2(addr)) {
				register_offset_addr = ((addr-MDOMAIN_CAP_START_REGISTER_2) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_mcap2_st_vir_addr)+ register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_mcap2_st_vir_addr)+ register_offset_addr;
				}
			}
		}else if (MAIN_TOP_CTL_register_range_0(addr)) {
			if (MAIN_TOP_CTL_DDR_range_0(addr)) {
				register_offset_addr = ((addr-MAIN_TOP_CTL_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_vtop0_st_vir_addr)+ register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_vtop0_st_vir_addr)+ register_offset_addr;
				}
			}
		}else if (MAIN_TOP_CTL_register_range_1(addr)) {
			if (MAIN_TOP_CTL_DDR_range_1(addr)) {
				register_offset_addr = ((addr-MAIN_TOP_CTL_START_REGISTER_1) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s1_vtop1_st_vir_addr)+ register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_write_offset_addr(i2rnd_scpu_vir_addr.s0_vtop1_st_vir_addr)+ register_offset_addr;
				}
			}
		}
	}
	return cur_addr;
}
unsigned int Scaler_i2rnd_IP_register_get_cur_read_addr(unsigned int addr,unsigned char display)
{
	unsigned int cur_addr =0;
	unsigned int register_offset_addr =0;
	if (i2rnd_scpu_vir_addr.s0_vo_st_vir_addr && i2rnd_scpu_vir_addr.s1_vo_st_vir_addr) {
		if (VGIP_register_range_0(addr)) {
			if (VGIP_DDR_range_0(addr)) {
				register_offset_addr = ((addr-VGIP_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_vgip_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_vgip_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (VODMA_register_range_0(addr)) {
			if (VODMA_DDR_range_0(addr)) {
				register_offset_addr = ((addr-VODMA_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_vo_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_vo_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (COLOR_MB_PEAKING_register_range_0(addr)) {
			if (COLOR_MB_PEAKING_DDR_range_0(addr)) {
				register_offset_addr = ((addr-COLOR_MB_PEAKING_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_color_mb_peaking_st_vir_addr)+ register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_color_mb_peaking_st_vir_addr)+ register_offset_addr;
				}
			}
		} else if (DE_XCXL_register_range_0(addr)) {
			if (DE_XCXL_DDR_range_0(addr)) {
				register_offset_addr = ((addr-DE_XCXL_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_dexcxl_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_dexcxl_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (DFILTER_register_range_0(addr)) {
			if (DFILTER_DDR_range_0(addr)) {
				register_offset_addr = ((addr-DFILTER_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_dfilter_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_dfilter_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (DI_register_range_0(addr)) {
			if (DI_DDR_range_0(addr)) {
				register_offset_addr = ((addr-DI_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_di0_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_di0_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (DI_register_range_1(addr)) {
			if (DI_DDR_range_1(addr)) {
				register_offset_addr = ((addr-DI_START_REGISTER_1) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_di1_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_di1_st_vir_addr) + register_offset_addr;
				}
			}
		}  else if (DI_register_range_2(addr)) {
			if (DI_DDR_range_2(addr)) {
				register_offset_addr = ((addr-DI_START_REGISTER_2) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_di2_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_di2_st_vir_addr) + register_offset_addr;
				}
			}
		}  else if (DI_register_range_3(addr)) {
			if (DI_DDR_range_3(addr)) {
				register_offset_addr = ((addr-DI_START_REGISTER_3) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_di3_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_di3_st_vir_addr) + register_offset_addr;
				}
			}
		}  else if (DI_register_range_4(addr)) {
			if (DI_DDR_range_4(addr)) {
				register_offset_addr = ((addr-DI_START_REGISTER_4) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_di4_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_di4_st_vir_addr) + register_offset_addr;
				}
			}
		}  else if (DI_register_range_5(addr)) {
			if (DI_DDR_range_5(addr)) {
				register_offset_addr = ((addr-DI_START_REGISTER_5) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_di5_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_di5_st_vir_addr) + register_offset_addr;
				}
			}
		}  else if (DI_register_range_6(addr)) {
			if (DI_DDR_range_6(addr)) {
				register_offset_addr = ((addr-DI_START_REGISTER_6) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_di6_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_di6_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (HISTOGRAM_register_range_0(addr)) {
			if (HISTOGRAM_DDR_range_0(addr)) {
				register_offset_addr = ((addr-HISTOGRAM_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_histogram_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_histogram_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (HSD_DITHER_register_range_0(addr)) {
			if (HSD_DITHER_DDR_range_0(addr)) {
				register_offset_addr = ((addr-HSD_DITHER_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_hsddither_st_vir_addr)+ register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_hsddither_st_vir_addr)+ register_offset_addr;
				}
			}
		} else if (IDCTI_register_range_0(addr)) {
			if (IDCTI_DDR_range_0(addr)) {
				register_offset_addr = ((addr-IDCTI_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_idcti_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_idcti_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (IEDGE_SMOOTH_register_range_0(addr)) {
			if (IEDGE_SMOOTH_DDR_range_0(addr)) {
				register_offset_addr = ((addr-IEDGE_SMOOTH_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_iedgesmooth_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_iedgesmooth_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (IPQ_DECONTOUR_register_range_0(addr)) {
			if (IPQ_DECONTOUR_DDR_range_0(addr)) {
				register_offset_addr = ((addr-IPQ_DECONTOUR_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_ipq_decontour_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_ipq_decontour_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (MAIN_DITHER_register_range_0(addr)) {
			if (MAIN_DITHER_register_range_0(addr)) {
				register_offset_addr = ((addr-MAIN_DITHER_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_main_dither_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_main_dither_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (MPEGNR_register_range_0(addr)) {
			if (MPEGNR_DDR_range_0(addr)) {
				register_offset_addr = ((addr-MPEGNR_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr = Scaler_i2rnd_cmd_buffer_read_offset_addr( i2rnd_scpu_vir_addr.s1_mpegnr_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_mpegnr_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (NR_register_range_0(addr)) {
			if (NR_DDR_range_0(addr)) {
				register_offset_addr = ((addr-NR_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_nr0_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_nr0_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (NR_register_range_1(addr)) {
			if (NR_DDR_range_1(addr)) {
				register_offset_addr = ((addr-NR_START_REGISTER_1) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_nr1_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_nr1_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (PEAKING_register_range_0(addr)) { //remove from merlin4, just return 0
			if (PEAKING_DDR_range_0(addr)) {
				register_offset_addr = ((addr-PEAKING_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_peaking_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_peaking_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (PROFILE_register_range_0(addr)) {
			if (PROFILE_DDR_range_0(addr)) {
				register_offset_addr = ((addr-PROFILE_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_profile0_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_profile0_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (PROFILE_register_range_1(addr)) {
			if (PROFILE_DDR_range_1(addr)) {
				register_offset_addr = ((addr-PROFILE_START_REGISTER_1) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_profile1_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_profile1_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (RGB2YUV_register_range_0(addr)) {
			if (RGB2YUV_DDR_range_0(addr)) {
				register_offset_addr = ((addr-RGB2YUV_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_rgb2yuv0_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_rgb2yuv0_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (RGB2YUV_register_range_1(addr)) {
			if (RGB2YUV_DDR_range_1(addr)) {
				register_offset_addr = ((addr-RGB2YUV_START_REGISTER_1) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_rgb2yuv1_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_rgb2yuv1_st_vir_addr) + register_offset_addr;
				}
			}
		}  else if (RINGFILTER_register_range_0(addr)) {
			if (RINGFILTER_DDR_range_0(addr)) {
				register_offset_addr = ((addr-RINGFILTER_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_ringfilter_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_ringfilter_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (SCALEDOWN_register_range_0(addr)) {
			if (SCALEDOWN_DDR_range_0(addr)) {
				register_offset_addr = ((addr-SCALEDOWN_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_uzd0_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_uzd0_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (SCALEDOWN_register_range_1(addr)) {
			if (SCALEDOWN_DDR_range_1(addr)) {
				register_offset_addr = ((addr-SCALEDOWN_START_REGISTER_1) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_uzd1_st_vir_addr) + register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_uzd1_st_vir_addr) + register_offset_addr;
				}
			}
		} else if (MDOMAIN_CAP_register_range_0(addr)) {
			if (MDOMAIN_CAP_DDR_range_0(addr)) {
				register_offset_addr = ((addr-MDOMAIN_CAP_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_mcap0_st_vir_addr)+ register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_mcap0_st_vir_addr)+ register_offset_addr;
				}
			}
		} else if (MDOMAIN_CAP_register_range_1(addr)) {
			if (MDOMAIN_CAP_DDR_range_1(addr)) {
				register_offset_addr = ((addr-MDOMAIN_CAP_START_REGISTER_1) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_mcap1_st_vir_addr)+ register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_mcap1_st_vir_addr)+ register_offset_addr;
				}
			}
		}  else if (MDOMAIN_CAP_register_range_2(addr)) {
			if (MDOMAIN_CAP_DDR_range_2(addr)) {
				register_offset_addr = ((addr-MDOMAIN_CAP_START_REGISTER_2) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_mcap2_st_vir_addr)+ register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_mcap2_st_vir_addr)+ register_offset_addr;
				}
			}
		} else if (MAIN_TOP_CTL_register_range_0(addr)) {
			if (MAIN_TOP_CTL_DDR_range_0(addr)) {
				register_offset_addr = ((addr-MAIN_TOP_CTL_START_REGISTER_0) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_vtop0_st_vir_addr)+ register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_vtop0_st_vir_addr)+ register_offset_addr;
				}
			}
		}else if (MAIN_TOP_CTL_register_range_1(addr)) {
			if (MAIN_TOP_CTL_DDR_range_1(addr)) {
				register_offset_addr = ((addr-MAIN_TOP_CTL_START_REGISTER_1) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
				if (display == I2RND_SUB_S1_TABLE) {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s1_vtop1_st_vir_addr)+ register_offset_addr;
				} else {
					cur_addr =  Scaler_i2rnd_cmd_buffer_read_offset_addr(i2rnd_scpu_vir_addr.s0_vtop1_st_vir_addr)+ register_offset_addr;
				}
			}
		}
	}
	return cur_addr;
}

void i2rnd_debug(void){
	memcpy((unsigned char *)i2rnd_scpu_vir_addr.s0_vo_st_vir_addr, (unsigned char *)i2rnd_scpu_vir_addr.s1_vo_st_vir_addr, sizeof(unsigned char) * i2run_queue_size);
}
extern void i2rnd_default_allocate_mcap1(void);
extern void i2rnd_default_allocate_mcap2(void);
void i2rnd_default_register_setting(void){
	di_im_di_active_window_control_RBUS di_im_di_active_window_control_reg;
	/*===============s1 table=====================*/
	Scaler_I2rnd_set_display(I2RND_SUB_S1_TABLE);
#if 0//move to video fw
	//pre-allocate memory for M-cap2
	i2rnd_default_allocate_mcap2();

	//pre-set DI start & boundary
	i2rnd_default_allocate_DI2();
#endif
	//DI default size setting for sub start address sample issue @Crixus 20160725
	di_im_di_active_window_control_reg.regValue = IoReg_Read32(DI_IM_DI_ACTIVE_WINDOW_CONTROL_reg);
	di_im_di_active_window_control_reg.hsize = 0xa;
	di_im_di_active_window_control_reg.vsize = 0xa;
	IoReg_Write32(DI_IM_DI_ACTIVE_WINDOW_CONTROL_reg, di_im_di_active_window_control_reg.regValue);


	/*===============End=====================*/

	//DI default size setting for sub start address sample issue @Crixus 20160725
	//DI hw detect does not write to cmd buf
	fw_scalerip_set_di_size_change_hw_detect(_ENABLE);//enable hw detect


	//frank@0815 test
	//Scaler_I2rnd_set_display(I2RND_MAIN_S0_TABLE);
}

//a register include i2rnd and non-i2rnd function
void i2rnd_group3_s0_to_s1_register_sync(void){
	/*Please add the group 3 register here*/
#if 0
	//0xb80272c8 MDOMAIN_CAP_Capture_byte_swap_reg
	mdomain_cap_capture1_byte_swap_RBUS capture_byte_swap_reg_s0;
	mdomain_cap_capture1_byte_swap_RBUS capture_byte_swap_reg_s1;
	mdomain_disp_display_byte_channel_swap_RBUS display_byte_channel_swap_reg_s0;

	//step 1. Read value from s1 table
	Scaler_I2rnd_set_display(I2RND_SUB_S1_TABLE);
	capture_byte_swap_reg_s1.regValue = IoReg_Read32(MDOMAIN_CAP_Capture1_byte_swap_reg);
	//rtd_pr_vbe_emerg("[%s]capture_byte_swap_reg_s1.regValue = %x\n", __FUNCTION__, capture_byte_swap_reg_s1.regValue);

	//step 2. Read value from s0 table
	Scaler_I2rnd_set_display(I2RND_MAIN_S0_TABLE);
	capture_byte_swap_reg_s0.regValue = IoReg_Read32(MDOMAIN_CAP_Capture1_byte_swap_reg);
	//rtd_pr_vbe_emerg("[%s]capture_byte_swap_reg_s0.regValue = %x\n", __FUNCTION__, capture_byte_swap_reg_s0.regValue);
	display_byte_channel_swap_reg_s0.regValue = IoReg_Read32(MDOMAIN_DISP_Display_byte_channel_swap_reg);

	//step 3. Sync the non-i2rnd function from s1 to s0
	capture_byte_swap_reg_s0.in2_1byte_swap = capture_byte_swap_reg_s1.in2_1byte_swap;
	capture_byte_swap_reg_s0.in2_2byte_swap = capture_byte_swap_reg_s1.in2_2byte_swap;
	capture_byte_swap_reg_s0.in2_4byte_swap = capture_byte_swap_reg_s1.in2_4byte_swap;
	capture_byte_swap_reg_s0.in2_8byte_swap = capture_byte_swap_reg_s1.in2_8byte_swap;
	IoReg_Write32(MDOMAIN_CAP_Capture1_byte_swap_reg, capture_byte_swap_reg_s0.regValue);

	//S-disp byte swap
	display_byte_channel_swap_reg_s0.sub_1byte_swap = mdomain_disp_display_byte_channel_swap_s1_g.sub_1byte_swap;
	display_byte_channel_swap_reg_s0.sub_2byte_swap = mdomain_disp_display_byte_channel_swap_s1_g.sub_2byte_swap;
	display_byte_channel_swap_reg_s0.sub_4byte_swap = mdomain_disp_display_byte_channel_swap_s1_g.sub_4byte_swap;
	display_byte_channel_swap_reg_s0.sub_8byte_swap = mdomain_disp_display_byte_channel_swap_s1_g.sub_8byte_swap;
	IoReg_Write32(MDOMAIN_DISP_Display_byte_channel_swap_reg, display_byte_channel_swap_reg_s0.regValue);
#endif
	//step 4. Change to original table selection
	Scaler_I2rnd_set_display(I2RND_SUB_S1_TABLE);
}
void Scaler_i2rnd_set_write_point(unsigned char display_index)
{
	if (display_index == I2RND_MAIN_S0_TABLE) {
		pst_i2rnd_dma_wrt_pnt1_RBUS wrt_pnt1_reg;
		wrt_pnt1_reg.regValue = rtd_inl(PST_I2RND_DMA_WRT_PNT1_reg);
		wrt_pnt1_reg.i2rnd_src0_vo_write_pnt = Scaler_I2rnd_get_write_cmd_num(display_index);
		rtd_outl(PST_I2RND_DMA_WRT_PNT1_reg, wrt_pnt1_reg.regValue);
	} else if (display_index == I2RND_SUB_S1_TABLE) {
		pst_i2rnd_dma_wrt_pnt0_RBUS wrt_pnt0_reg;
		wrt_pnt0_reg.regValue = rtd_inl(PST_I2RND_DMA_WRT_PNT0_reg);
		wrt_pnt0_reg.i2rnd_src1_vo_write_pnt = Scaler_I2rnd_get_write_cmd_num(display_index);
		rtd_outl(PST_I2RND_DMA_WRT_PNT0_reg, wrt_pnt0_reg.regValue);
	}
}
unsigned char Scaler_i2rnd_get_write_point(unsigned char display_index)
{
	if (display_index == I2RND_MAIN_S0_TABLE) {
		pst_i2rnd_dma_wrt_pnt1_RBUS wrt_pnt1_reg;
		wrt_pnt1_reg.regValue = rtd_inl(PST_I2RND_DMA_WRT_PNT1_reg);
		return wrt_pnt1_reg.i2rnd_src0_vo_write_pnt;
	} else if (display_index == I2RND_SUB_S1_TABLE) {
		pst_i2rnd_dma_wrt_pnt0_RBUS wrt_pnt0_reg;
		wrt_pnt0_reg.regValue = rtd_inl(PST_I2RND_DMA_WRT_PNT0_reg);
		return wrt_pnt0_reg.i2rnd_src1_vo_write_pnt;
	} else {
		return 0;
	}
}
unsigned char Scaler_i2rnd_get_read_point(unsigned char display_index)
{
	if (display_index == I2RND_MAIN_S0_TABLE) {
		pst_i2rnd_dma_read_pnt1_RBUS read_pnt1_reg;
		read_pnt1_reg.regValue = rtd_inl(PST_I2RND_DMA_READ_PNT1_reg);
		return read_pnt1_reg.i2rnd_src0_vo_read_pnt;
	} else if (display_index == I2RND_SUB_S1_TABLE) {
		pst_i2rnd_dma_read_pnt0_RBUS read_pnt0_reg;
		read_pnt0_reg.regValue = rtd_inl(PST_I2RND_DMA_READ_PNT0_reg);
		return read_pnt0_reg.i2rnd_src1_vo_read_pnt;
	} else {
		return 0;
	}
}

void set_i2rnd_pst_write_pnt_counter(unsigned int cnt)
{
	if (i2rnd_scpu_vir_addr.i2rnd_pst_pnt_counter_vir_addr) {
		*(volatile unsigned char *)i2rnd_scpu_vir_addr.i2rnd_pst_pnt_counter_vir_addr = cnt;
	}
}

unsigned int get_i2rnd_pst_write_pnt_counter(void)
{
	if (i2rnd_scpu_vir_addr.i2rnd_pst_pnt_counter_vir_addr) {
		return (*(volatile unsigned char *)i2rnd_scpu_vir_addr.i2rnd_pst_pnt_counter_vir_addr);
	} else {
		return 0;
	}
}

void set_i2rnd_s0_idomain_done_status(unsigned int cnt)
{
	if (i2rnd_scpu_vir_addr.i2rnd_s0_idomain_done_status_vir_addr) {
		*(volatile unsigned char *)(i2rnd_scpu_vir_addr.i2rnd_s0_idomain_done_status_vir_addr)= cnt;
	}
}

unsigned int get_i2rnd_s0_idomain_done_status(void)
{
	if (i2rnd_scpu_vir_addr.i2rnd_s0_idomain_done_status_vir_addr) {
		return(*(volatile unsigned char *)(i2rnd_scpu_vir_addr.i2rnd_s0_idomain_done_status_vir_addr));
	} else {
		return 0;
	}
}

void i2rnd_group3_s1_to_s0_register_sync(void){
	/*Please add the group 3 register here*/
	//0xb80272c8 MDOMAIN_CAP_Capture_byte_swap_reg
	mdomain_cap_capture2_byte_swap_RBUS capture_byte_swap_reg_s0;
	mdomain_cap_capture2_byte_swap_RBUS capture_byte_swap_reg_s1;

	//step 1. Read value from s1 table
	Scaler_I2rnd_set_display(I2RND_SUB_S1_TABLE);
	capture_byte_swap_reg_s1.regValue = IoReg_Read32(MDOMAIN_CAP_Capture2_byte_swap_reg);
	//rtd_pr_vbe_emerg("[%s]capture_byte_swap_reg_s1.regValue = %x\n", __FUNCTION__, capture_byte_swap_reg_s1.regValue);

	//step 2. Read value from s0 table
	Scaler_I2rnd_set_display(I2RND_MAIN_S0_TABLE);
	capture_byte_swap_reg_s0.regValue = IoReg_Read32(MDOMAIN_CAP_Capture2_byte_swap_reg);
	//rtd_pr_vbe_emerg("[%s]capture_byte_swap_reg_s0.regValue = %x\n", __FUNCTION__, capture_byte_swap_reg_s0.regValue);


	//step 3. Sync the non-i2rnd function from s1 to s0
	capture_byte_swap_reg_s0.in2_1byte_swap = capture_byte_swap_reg_s1.in2_1byte_swap;
	capture_byte_swap_reg_s0.in2_2byte_swap = capture_byte_swap_reg_s1.in2_2byte_swap;
	capture_byte_swap_reg_s0.in2_4byte_swap = capture_byte_swap_reg_s1.in2_4byte_swap;
	capture_byte_swap_reg_s0.in2_8byte_swap = capture_byte_swap_reg_s1.in2_8byte_swap;
	IoReg_Write32(MDOMAIN_CAP_Capture2_byte_swap_reg, capture_byte_swap_reg_s0.regValue);

	//step 4. Change to original table selection
	Scaler_I2rnd_set_display(I2RND_SUB_S1_TABLE);
}

void i2rnd_di_size_toggle(void){
	di_im_di_active_window_control_RBUS di_im_di_active_window_control_reg;
	vgip_vgip_chn1_lc_RBUS vgip_vgip_chn1_lc_reg;
	vgip_vgip_chn1_act_vsta_length_RBUS vgip_vgip_chn1_act_vsta_length_reg;

	unsigned int v_start = 0;
	unsigned int timeout_cnt = 0x7ffff;
	//unsigned int line_cnt1, line_cnt2, line_cnt3;
	//unsigned int di_value1 = 0, di_value2 = 0;

	//get vgip v_start
	vgip_vgip_chn1_act_vsta_length_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_ACT_VSTA_Length_reg);
	v_start = vgip_vgip_chn1_act_vsta_length_reg.ch1_iv_act_sta;

	//DI default size setting for sub start address sample issue @Crixus 20160725
	di_im_di_active_window_control_reg.regValue = IoReg_Read32(DI_IM_DI_ACTIVE_WINDOW_CONTROL_reg);
	di_im_di_active_window_control_reg.hsize = 0xa;
	di_im_di_active_window_control_reg.vsize = 0xa;

	//wait active start to modify the cmd buf because the cmd buf only access in porch @Crixus 20160803
	//wait first Vsync
	while(timeout_cnt--){
		vgip_vgip_chn1_lc_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_LC_reg);
		if(vgip_vgip_chn1_lc_reg.ch1_line_cnt == v_start){
			//line_cnt1= vgip_vgip_chn1_lc_reg.ch1_line_cnt;
			break;
		}
	}

	//line_cnt1 = vgip_vgip_chn1_lc_reg.ch1_line_cnt;
	IoReg_Write32(DI_IM_DI_ACTIVE_WINDOW_CONTROL_reg, di_im_di_active_window_control_reg.regValue);

	//wait Vsync to make sure the cmd buf already write to register.
	//wait second Vsync because there are two Vsync which one is main, one is sub. Need to wait two Vsync. @Crixus 20160811
	timeout_cnt = 0x7ffff;
	while(timeout_cnt--){
		vgip_vgip_chn1_lc_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_LC_reg);
		if(vgip_vgip_chn1_lc_reg.ch1_line_cnt == 2){
			//line_cnt2 = vgip_vgip_chn1_lc_reg.ch1_line_cnt;
			break;
		}
	}

	timeout_cnt = 0x7ffff;
	while(timeout_cnt--){
		vgip_vgip_chn1_lc_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_LC_reg);
		if(vgip_vgip_chn1_lc_reg.ch1_line_cnt == 1){
			//line_cnt2 = vgip_vgip_chn1_lc_reg.ch1_line_cnt;
			break;
		}
	}

	di_im_di_active_window_control_reg.regValue = IoReg_Read32(DI_IM_DI_ACTIVE_WINDOW_CONTROL_reg);
	di_im_di_active_window_control_reg.hsize = Scaler_DispGetInputInfo(SLR_INPUT_DI_WID);// Width;
	di_im_di_active_window_control_reg.vsize = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN); // Height;

	//wait active start to modify the cmd buf because the cmd buf only access in porch @Crixus 20160803
	//WaitFor_IEN_START1();
	timeout_cnt = 0x7ffff;
	while(timeout_cnt--){
		vgip_vgip_chn1_lc_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_LC_reg);
		if(vgip_vgip_chn1_lc_reg.ch1_line_cnt == v_start){
		//	line_cnt3 = vgip_vgip_chn1_lc_reg.ch1_line_cnt;
			break;
		}
	}
	IoReg_Write32(DI_IM_DI_ACTIVE_WINDOW_CONTROL_reg, di_im_di_active_window_control_reg.regValue);

}

void i2rnd_send_table_idx(unsigned char index){
	int ret = 0;

	if (0 != (ret = Scaler_SendRPC(SCALERIOC_I2RND_SEND_TABLE_IDX, index, 1))){
		rtd_pr_vbe_debug("[I2RND]ret=%d, SCALERIOC_I2RND_SEND_TABLE_IDX RPC fail !!!\n", ret);
	}

}

void Scaler_i2rnd_buffer_copy(unsigned char display_idx)
{
	unsigned char i=0;
	unsigned char j=0;
	unsigned int curr_i2rnd_s0_write_buffer_addr = 0;

	if(display_idx == I2RND_MAIN_S0_TABLE){
		j = Scaler_I2rnd_get_write_cmd_num(I2RND_MAIN_S0_TABLE);
			//rtd_pr_vbe_emerg("~~~~~~i2rnd write buffer(%d)\n",j);

			if(j != 0) {
					curr_i2rnd_s0_write_buffer_addr=i2rnd_scpu_vir_addr.s0_vo_st_vir_addr+j*i2rnd_st_addr.cmd_buffer_size;
					memcpy((unsigned char *)i2rnd_scpu_vir_addr.s0_vo_st_vir_addr, (unsigned char *)curr_i2rnd_s0_write_buffer_addr, sizeof(unsigned char) * i2rnd_st_addr.cmd_buffer_size);
			}
	for (i=1;i<(I2RND_CMD_NUMBER+1);i++) {
			curr_i2rnd_s0_write_buffer_addr=i2rnd_scpu_vir_addr.s0_vo_st_vir_addr+i*i2rnd_st_addr.cmd_buffer_size;
		memcpy((unsigned char *)curr_i2rnd_s0_write_buffer_addr, (unsigned char *)i2rnd_scpu_vir_addr.s0_vo_st_vir_addr, sizeof(unsigned char) * i2rnd_st_addr.cmd_buffer_size);
		}
	}
	else if(display_idx == I2RND_SUB_S1_TABLE){
		j = Scaler_I2rnd_get_write_cmd_num(I2RND_SUB_S1_TABLE);
		//rtd_pr_vbe_emerg("~~~~~~i2rnd write buffer(%d)\n",j);

		if(j != 0) {
			curr_i2rnd_s0_write_buffer_addr=i2rnd_scpu_vir_addr.s1_vo_st_vir_addr+j*i2rnd_st_addr.cmd_buffer_size;
			memcpy((unsigned char *)i2rnd_scpu_vir_addr.s1_vo_st_vir_addr, (unsigned char *)curr_i2rnd_s0_write_buffer_addr, sizeof(unsigned char) * i2rnd_st_addr.cmd_buffer_size);
		}
		for (i=1;i<(I2RND_CMD_NUMBER+1);i++) {
			curr_i2rnd_s0_write_buffer_addr=i2rnd_scpu_vir_addr.s1_vo_st_vir_addr+i*i2rnd_st_addr.cmd_buffer_size;
			memcpy((unsigned char *)curr_i2rnd_s0_write_buffer_addr, (unsigned char *)i2rnd_scpu_vir_addr.s1_vo_st_vir_addr, sizeof(unsigned char) * i2rnd_st_addr.cmd_buffer_size);
		}
	}
}

#if 0 //no using now
void i2rnd_di_control_reg_change_valid_index(unsigned char enable, unsigned char display){
	//DI special case checking, DI disable => DI setting => DI enable @Crixus 20160725
	unsigned int addr =0;
	unsigned int cur_addr =0;
	unsigned int tmp_addr = 0;
	unsigned int register_offset_addr =0;


	rtd_pr_vbe_emerg("[%s]display = %d, enable = %d\n", __FUNCTION__, display, enable);

	addr = DI_IM_DI_CONTROL_reg;
	register_offset_addr = ((addr-MAIN_DI_START_REGISTER) /REGISTER_VALUE_SIZE) *I2RND_REGISTER_SIZE;
	//get virtual address
	if (display == I2RND_SUB_S1_TABLE) {
		cur_addr =	i2rnd_scpu_vir_addr.s1_di_st_vir_addr + register_offset_addr;
	} else {
		cur_addr =	i2rnd_scpu_vir_addr.s0_di_st_vir_addr + register_offset_addr;
	}

	//change the valid index
	if(enable == _DISABLE){
		tmp_addr =((addr>>2)&0x000fffff)|0x00000000;
	}
	else{
		tmp_addr =((addr>>2)&0x000fffff)|0x80000000;
	}

	//write to table
	cur_addr-=4;
	*(volatile unsigned int *)(cur_addr) = __bswap_32(tmp_addr);

	WaitFor_IVS1();
}
#endif

unsigned char Scaler_I2rnd_get_timing_enable(void)
{
	vodma_vodma_i2rnd_RBUS    vo_i2rnd;
	vo_i2rnd.regValue = rtd_inl(VODMA_vodma_i2rnd_reg);
	return vo_i2rnd.i2rnd_timing_en;
}

void set_i2rnd_vo1_refine_status_vir_addr(unsigned int cnt)
{
	if (i2rnd_scpu_vir_addr.i2rnd_vo1_refine_status_vir_addr) {
		*(volatile unsigned char *)(i2rnd_scpu_vir_addr.i2rnd_vo1_refine_status_vir_addr)= cnt;
	}
}

unsigned int get_i2rnd_vo1_refine_status_vir_addr(void)
{
	if (i2rnd_scpu_vir_addr.i2rnd_vo1_refine_status_vir_addr) {
		return(*(volatile unsigned char *)(i2rnd_scpu_vir_addr.i2rnd_vo1_refine_status_vir_addr));
	} else {
		return 0;
	}
}

//for sub pst
void set_i2rnd_s1_pst_write_pnt_counter(unsigned int cnt)
{
	if (i2rnd_scpu_vir_addr.i2rnd_s1_pst_pnt_counter_vir_addr) {
		*(volatile unsigned char *)(i2rnd_scpu_vir_addr.i2rnd_s1_pst_pnt_counter_vir_addr)= cnt;
	}
}

unsigned int get_i2rnd_s1_pst_write_pnt_counter(void)
{
	if (i2rnd_scpu_vir_addr.i2rnd_s1_pst_pnt_counter_vir_addr) {
		return(*(volatile unsigned char *)(i2rnd_scpu_vir_addr.i2rnd_s1_pst_pnt_counter_vir_addr));
	} else {
		return 0;
	}
}

unsigned char Scaler_I2rnd_get_force_cmd(void)
{
	if (i2rnd_scpu_vir_addr.i2rnd_force_write_cmd_addr) {
		return (*(volatile unsigned char *)(i2rnd_scpu_vir_addr.i2rnd_force_write_cmd_addr));
	} else {
		return 0;
	}
}
void Scaler_I2rnd_set_force_cmd(unsigned char enable)
{
	if (i2rnd_scpu_vir_addr.i2rnd_force_write_cmd_addr) {
		*(volatile unsigned char *)(i2rnd_scpu_vir_addr.i2rnd_force_write_cmd_addr) = enable;
	}
}

void Scaler_I2rnd_condition_check(unsigned int main_outputdisplay, unsigned int in_width, unsigned in_length){

	//rtd_pr_vbe_emerg("[%s][I2RND]in_width = %d, in_length = %d\n", __FUNCTION__, in_width, in_length);

	//if sub input size is over 2k1k, do not enable i2run
	if((in_width > I2RUN_SUB_SIZE_CONDITION_WIDTH)/*(in_length > I2RUN_SUB_SIZE_CONDITION_LENGTH)*/){
		if(main_outputdisplay == FALSE){
			//main already disconnect
			vsc_i2rnd_dualvo_sub_stage= I2RND_DUALVO_STAGE_SUB_DISABLE;
			Scaler_I2rnd_set_force_cmd(_ENABLE);
			rtk_hal_vsc_pst_enable(_DISABLE);
			drvif_scaler_i2rnd_top_ctrl(_DISABLE);
			drvif_scaler_pst_top_ctrl(_DISABLE, _MAIN_MD);
			drvif_scaler_pst_top_ctrl(_DISABLE, _SUB_MD);
			rtd_pr_vbe_emerg("[%s][I2RND]over sub size condition, set i2rnd & pst disable!\n", __FUNCTION__);
		}
		else{
			//main still connect
			vsc_i2rnd_dualvo_sub_stage= I2RND_DUALVO_STAGE_SUB_DISABLE;
			Scaler_I2rnd_set_force_cmd(_ENABLE);
			Scaler_I2rnd_set_display(I2RND_MAIN_S0_TABLE);
			drvif_scaler_i2rnd_top_ctrl(_DISABLE);
			drvif_scaler_pst_top_ctrl(_DISABLE, _MAIN_MD);
			drvif_scaler_pst_top_ctrl(_DISABLE, _SUB_MD);
			rtd_pr_vbe_emerg("[%s][I2RND]over sub size condition, main still connect, set i2rnd & pst disable!\n", __FUNCTION__);
		}
	}
}

void Scaler_I2rnd_wait_timing_enable(void){
	unsigned char timeout = 10;
	while (timeout--) {
		if(Scaler_I2rnd_get_timing_enable() == _ENABLE) {
			rtd_pr_vbe_emerg("\r\n[I2RND]I2run timing already enable done!!#####\r\n");
			break;
		}
		msleep(10);
	}
}

unsigned int scaler_i2rnd_enable_di_write_bit(unsigned int value)
{
	di_im_di_control_RBUS di_im_di_control_reg;
	di_im_di_control_reg.regValue = value;

	di_im_di_control_reg.write_enable_0 = 1;
	di_im_di_control_reg.write_enable_1 = 1;
	di_im_di_control_reg.write_enable_2 = 1;
	//di_im_di_control_reg.write_enable_3 = 1; remove from merlin4
	di_im_di_control_reg.write_enable_4 = 1;
	di_im_di_control_reg.write_enable_5 = 1;
	di_im_di_control_reg.write_enable_6 = 1;
	di_im_di_control_reg.write_enable_7 = 1;
	di_im_di_control_reg.write_enable_8 = 1;

	return di_im_di_control_reg.regValue;
}

#endif

unsigned char Scaler_I2rnd_get_Index(void)
{
	unsigned char ret_idx = 0;

#ifdef CONFIG_I2RND_ENABLE
	//add I2rnd condition @Crixus 20160716
	if((Scaler_I2rnd_get_timing_enable() == _ENABLE) && ((vsc_i2rnd_sub_stage == I2RND_STAGE_MAIN_GET_RPC) ||(vsc_i2rnd_dualvo_sub_stage == I2RND_DUALVO_STAGE_RERUN_MAIN_FOR_S1)))
		ret_idx = 1;
#endif

#ifdef CONFIG_I2RND_PQ_ENABLE
	if((Scaler_I2rnd_get_timing_enable() == _ENABLE) && (vsc_i2rnd_s1_mcap12vo_stage == I2RND_S1_MCAP12VO_STAGE_MAIN_GET_RPC))
		ret_idx = 1;
#endif

	return ret_idx;
}

unsigned int Scaler_I2rnd_get_speedup_htotal(unsigned int width)
{
	if(width> 1920)// 4k2k or 4k1k case
		return I2RND_VO_FIX_H_TOTAL_4K2K;
	else
		return I2RND_VO_FIX_H_TOTAL_2K1K;
}

#ifdef BRING_UP_I2RND
void i2rnd_bring_up_verify(void)
{
	unsigned char i2rnd_pre_display = 0;
	//copy s0 buffer to s1
	memcpy((unsigned char *)i2rnd_scpu_vir_addr.s1_vo_st_vir_addr, (unsigned char *)i2rnd_scpu_vir_addr.s0_vo_st_vir_addr, i2rnd_st_addr.cmd_buffer_size);

	//set load/period
	IoReg_Write32(0xb8005650,IoReg_Read32(0xb8005648));
	IoReg_Write32(0xb8005654,IoReg_Read32(0xb800564c));
	//set s1 flag fifo
	IoReg_Write32(0xb8005628,IoReg_Read32(0xb8005624));

	//set M-cap2 address
	i2rnd_pre_display = Scaler_I2rnd_get_display();
	Scaler_I2rnd_set_display(I2RND_SUB_S1_TABLE);
	i2rnd_default_allocate_mcap2();
	Scaler_I2rnd_set_display(i2rnd_pre_display);
	//pre-set DI start & boundary
	//i2rnd_default_allocate_DI2();
	//Set M-domain 3-buffer
	IoReg_SetBits(0xb802722c,_BIT6|_BIT8);

}
#endif

#ifdef CONFIG_I2RND_B05_ENABLE

/*=======================b05==================================*/
void drvif_scaler_i2rnd_b05_uninitial(void)
{
	 if (Scaler_I2rnd_b05_get_enable()){
			memset((unsigned char *)i2rnd_b05_scpu_vir_addr.s0_vo_st_vir_addr, 0, sizeof(unsigned char) * i2run_b05_allocate_size);
			drvif_scaler_i2rnd_b05_top_ctrl(_DISABLE);
			rtd_pr_vbe_debug("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~in_b05_uninitial\n");
			return;
   	     //free memory
	//      if((i2rnd_b05_st_addr.s0_vo_st_addr != 0) && (i2rnd_b05_scpu_vir_addr.s0_vo_st_vir_addr != 0))
	//      {       dvr_free((void *)i2rnd_b05_scpu_vir_addr.s0_vo_st_vir_addr);
	//              rtd_pr_vbe_debug("i2rnd_b05_free_memory\n");
	//              i2rnd_b05_st_addr.s0_vo_st_addr = 0;
	//              i2rnd_b05_scpu_vir_addr.s0_vo_st_vir_addr = 0;
	//              i2rnd_b05_scpu_vir_addr.i2rnd_enable_vir_addr = 0;
	//              rtd_pr_vbe_debug("i2rnd_b05_scpu_vir_addr.i2rnd_enable_vir_addr1 =%x\n",i2rnd_b05_scpu_vir_addr.i2rnd_enable_vir_addr);
	//      }
	 }
}


void drvif_scaler_i2rnd_b05_initial(void)
{
        //rtd_pr_vbe_debug("i2rnd_i2rnd_b05_scpu_vir_addr.s0_vo_st_vir_addr = %x\n", i2rnd_b05_scpu_vir_addr.s0_vo_st_vir_addr);
        //unsigned int b05_cache_addr;
        unsigned int i2run_b05_blk_size = 0;            //17*17*40 (34+1+5)= 8440
        unsigned int b05_blk_size_mod = 0;
     //   unsigned int i2run_b05_allocate_size = 0;
        //default period setting
    //    vodma_vodma_i2rnd_m_period_load_RBUS  vo_i2rnd_m_p_load;
    //    vodma_vodma_i2rnd_m_period_RBUS  vo_i2rnd_m_p;
        vodma_vodma_reg_db_ctrl2_RBUS vodma_vodma_reg_db_ctrl2;
//        vodma_vodma_i2rnd_m_period_load_dbg_RBUS vodma_vodma_i2rnd_m_period_load_dbg;

        i2run_b05_blk_size = I2RND_b05_BLOCK_SIZE;
        b05_blk_size_mod = i2run_b05_blk_size % (I2RND_DMA_LENGTH * 8);

        if(b05_blk_size_mod != 0) {
                i2run_b05_blk_size = i2run_b05_blk_size + (I2RND_DMA_LENGTH * 8 - b05_blk_size_mod);
        }

        i2run_b05_queue_size = i2run_b05_blk_size * (I2RND_B05_CMD_NUMBER + 1);
        i2run_b05_allocate_size = i2run_b05_queue_size + I2RND_INFO_SIZE;

        rtd_pr_vbe_debug("i2run_b05_blk_size = %d,i2run_b05_blk_size2=%d,i2run_b05_queue_size= %d,i2run_b05_allocate_size = %d\n", I2RND_b05_BLOCK_SIZE, i2run_b05_blk_size, i2run_b05_queue_size, i2run_b05_allocate_size);
        rtd_pr_vbe_emerg("[%s]i2run_b05_allocate_size = %d\n", __FUNCTION__, i2run_b05_allocate_size);

        //share memory allocation
        //vo_rtd_xxx_get_memory(i2run_b05_blk_size);
        rtd_pr_vbe_emerg("[%s]set_i2rnd_i2rnd_b05_scpu_vir_addr.s0_vo_st_vir_addr = %x\n", __FUNCTION__, i2rnd_b05_scpu_vir_addr.s0_vo_st_vir_addr);
        if (i2rnd_b05_scpu_vir_addr.s0_vo_st_vir_addr == 0) {
                b05_cache_addr = (unsigned long)dvr_malloc_uncached_specific(i2run_b05_allocate_size, GFP_DCU1_FIRST, (void*) & (i2rnd_b05_scpu_vir_addr.s0_vo_st_vir_addr));
                i2rnd_b05_st_addr.s0_vo_st_addr = (unsigned int)dvr_to_phys((void*)b05_cache_addr);
                i2rnd_b05_st_addr.i2rnd_enable_addr = i2rnd_b05_st_addr.s0_vo_st_addr + i2run_b05_queue_size;
                memset((unsigned char *)i2rnd_b05_scpu_vir_addr.s0_vo_st_vir_addr, 0, sizeof(unsigned char) * i2run_b05_allocate_size);
       //         rtd_pr_vbe_debug("i2rnd_b05_cache_addr=%x\n", b05_cache_addr);
               // i2run_b05_test += 1;
        }
        i2rnd_b05_st_addr.cmd_buffer_size = i2run_b05_blk_size;

        rtd_pr_vbe_debug("i2rnd_b05_cache_addr = %lx\n", b05_cache_addr);
       // rtd_pr_vbe_debug("i2rnd_i2rnd_b05_scpu_vir_addr.s0_vo_st_vir_addr = %x\n", i2rnd_b05_scpu_vir_addr.s0_vo_st_vir_addr);

        if(i2rnd_b05_st_addr.s0_vo_st_addr == 0) {
                rtd_pr_vbe_debug("[I2RND]malloc i2rnd buffer fail .......................................\n");
                rtd_pr_vbe_emerg("[%s]i2rnd_i2rnd_b05_scpu_vir_addr.s0_vo_st_vir_addr = %x\n", __FUNCTION__, i2rnd_b05_scpu_vir_addr.s0_vo_st_vir_addr);
                return;
        }

        i2rnd_b05_scpu_vir_addr.i2rnd_enable_vir_addr                   = i2rnd_b05_scpu_vir_addr.s0_vo_st_vir_addr             + i2run_b05_queue_size;
        i2rnd_b05_scpu_vir_addr.i2rnd_write_cmd_vir_addr                = i2rnd_b05_scpu_vir_addr.i2rnd_enable_vir_addr         + I2RND_ENABLE_ADDR_SIZE;
        i2rnd_b05_scpu_vir_addr.i2rnd_read_cmd_vir_addr         = i2rnd_b05_scpu_vir_addr.i2rnd_write_cmd_vir_addr      + I2RND_WRITE_CMD_ADDR_SIZE;
        rtd_pr_vbe_emerg("b05_s0_end_addr=%x\n", i2rnd_scpu_vir_addr.s0_vo_st_vir_addr + i2run_b05_allocate_size);


        //i2rnd_b05 dma setting
        drvif_scaler_i2rnd_b05_dma_setting(I2RND_B05_CMD_NUMBER, i2run_b05_blk_size);

        //disable double buffer
        vodma_vodma_reg_db_ctrl2.regValue = rtd_inl(VODMA_VODMA_REG_DB_CTRL2_reg);
        vodma_vodma_reg_db_ctrl2.i2rnd_db_en = 0;
        IoReg_Write32(VODMA_VODMA_REG_DB_CTRL2_reg, vodma_vodma_reg_db_ctrl2.regValue);

}



void drvif_scaler_i2rnd_b05_dma_setting(unsigned int cmd_num, unsigned int blk_size)
{
        pst_i2rnd_dma_buf_num_RBUS dma_buf_num;
        pst_i2rnd_dma_ctrl_RBUS     dma_ctrl;
        pst_i2rnd_dma_num0_RBUS     dma_num0;
//        pst_i2rnd_dma_num1_RBUS     dma_num1;
        rtd_pr_vbe_emerg("cmd_num = %d, blk_size = %d\n", cmd_num, blk_size);


        i2rnd_b05_step_size.s0_vo_blk_size = blk_size;


        //start address
        IoReg_Write32(PST_I2RND_DMA_ADDR0_reg, i2rnd_b05_st_addr.s0_vo_st_addr);
        //block step
        IoReg_Write32(PST_I2RND_DMA_STEP0_reg, i2rnd_b05_step_size.s0_vo_blk_size);
        //dma number
        dma_buf_num.regValue = rtd_inl(PST_I2RND_DMA_BUF_NUM_reg);
        dma_buf_num.dma_buffer_num = cmd_num;
        IoReg_Write32(PST_I2RND_DMA_BUF_NUM_reg, dma_buf_num.regValue);

        ///////////////////////////////////////////
        // WATER/NUM/LENGTH
        ///////////////////////////////////////////
        dma_ctrl.regValue = rtd_inl(PST_I2RND_DMA_CTRL_reg);
        dma_ctrl.dma_wtlvl = I2RND_B05_DMA_WATERLEVEL;
        dma_ctrl.dma_len = I2RND_B05_DMA_LENGTH;
        IoReg_Write32(PST_I2RND_DMA_CTRL_reg, dma_ctrl.regValue);

        dma_num0.regValue = rtd_inl(PST_I2RND_DMA_NUM0_reg);
        dma_num0.dma_num_i2rnd_src0_vo = i2rnd_b05_step_size.s0_vo_blk_size * 8 / 64 / dma_ctrl.dma_len;
        IoReg_Write32(PST_I2RND_DMA_NUM0_reg, dma_num0.regValue);

        ////////////////////////////////////////
        //  update current status
        ////////////////////////////////////////
        i2rnd_b05_st_addr.s0_vo_cur_addr = i2rnd_b05_st_addr.s0_vo_st_addr;
        i2rnd_b05_cmd_buf.s0_vo_cur_cmd = 0;
}


void drvif_scaler_i2rnd_b05_top_ctrl(bool i2rnd_en)
{
        vodma_vodma_i2rnd_RBUS    vo_i2rnd;
        pst_i2rnd_top_ctrl_RBUS   top_ctrl;
        if(i2rnd_en) {
                top_ctrl.regValue = rtd_inl(PST_I2RND_TOP_CTRL_reg);
                top_ctrl.i2rnd_en = i2rnd_en;
                IoReg_Write32(PST_I2RND_TOP_CTRL_reg, top_ctrl.regValue);
        } else {
                vo_i2rnd.regValue = rtd_inl(VODMA_vodma_i2rnd_reg);
                vo_i2rnd.i2rnd_timing_mode = _1_MAIN_0_SUB;
                IoReg_Write32(VODMA_vodma_i2rnd_reg, vo_i2rnd.regValue);
                vo_i2rnd.i2rnd_timing_en = 0;
                IoReg_Write32(VODMA_vodma_i2rnd_reg, vo_i2rnd.regValue);
                top_ctrl.regValue = rtd_inl(PST_I2RND_TOP_CTRL_reg);
                top_ctrl.i2rnd_en = i2rnd_en;
                IoReg_Write32(PST_I2RND_TOP_CTRL_reg, top_ctrl.regValue);
        }
}

void Scaler_I2rnd_b05_handler(void)
{
        drvif_scaler_i2rnd_b05_top_ctrl(Scaler_I2rnd_b05_get_enable());
 //       rtd_pr_vbe_debug("^^^^^^^^^^^^^_i2rnd_b05_top_ctrl_^^^^^^^^^^^^^^^^");
}


unsigned char Scaler_I2rnd_b05_get_enable(void)
{
        if (i2rnd_b05_scpu_vir_addr.i2rnd_enable_vir_addr) {
                return (*(volatile unsigned char *)(i2rnd_b05_scpu_vir_addr.i2rnd_enable_vir_addr));
        } else {
      //          rtd_pr_vbe_debug("^^^^^^^^^^^^^_i2rnd_b05_get_enable_faile_^^^^^^^^^^^^^");
                return 0;
        }
}


void Scaler_I2rnd_b05_set_enable(unsigned char enable)
{
        //rtd_pr_vbe_debug("i2rnd_b05_scpu_vir_addr.i2rnd_enable_vir_addr1 =%x,__%d\n",i2rnd_b05_scpu_vir_addr.i2rnd_enable_vir_addr,enable);
        if (i2rnd_b05_scpu_vir_addr.i2rnd_enable_vir_addr) {
                *(volatile unsigned char *)(i2rnd_b05_scpu_vir_addr.i2rnd_enable_vir_addr) = enable;
        }

}

void Scaler_i2rnd_b05_buffer_copy(void)
{
	unsigned char i=0;
	for (i=1;i<(I2RND_B05_CMD_NUMBER+1);i++) {
		unsigned int curr_i2rnd_b05_write_buffer_addr=i2rnd_scpu_vir_addr.s0_vo_st_vir_addr+i*i2rnd_st_addr.cmd_buffer_size;
		memcpy((unsigned char *)curr_i2rnd_b05_write_buffer_addr, (unsigned char *)i2rnd_scpu_vir_addr.s0_vo_st_vir_addr, sizeof(unsigned char) * i2rnd_st_addr.cmd_buffer_size);

	}

}

void drvif_scaler_i2rnd_b05_vodma_setting(unsigned char display)
{
        vodma_vodma_i2rnd_RBUS    vo_i2rnd;
        vodma_vodma_i2rnd_m_flag_RBUS vo_i2rnd_m_flag;
//        vodma_vodma_i2rnd_s_flag_RBUS vo_i2rnd_s_flag;
        vodma_vodma_i2rnd_m_period_load_RBUS  vo_i2rnd_m_p_load;
        vodma_vodma_i2rnd_m_period_RBUS  vo_i2rnd_m_p;
    //    vodma_vodma_i2rnd_s_period_load_RBUS  vo_i2rnd_s_p_load;
 //       vodma_vodma_i2rnd_s_period_RBUS  vo_i2rnd_s_p;
        vodma_vodma_reg_db_ctrl2_RBUS vodma_vodma_reg_db_ctrl2;
        vodma_vodma_i2rnd_m_period_load_dbg_RBUS vodma_vodma_i2rnd_m_period_load_dbg;
     //   vodma_vodma_i2rnd_s_period_load_dbg_RBUS vodma_vodma_i2rnd_s_period_load_dbg;
	// vodma_vodma_i2rnd_dma_info_RBUS vodma_i2rnd_dma_info;
        unsigned int period_load = 0, period_min = 0, period_max = 0;
        unsigned int h_active = 0, v_active = 0, interlace = 0;
	 unsigned int h_start = 0, v_start = 0;


        //i2rnd period
 //       unsigned int h_load = 0;
        unsigned int fix_htotal = 0;
        unsigned int vo_clock = 0;
        unsigned int frame_rate = Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ);
        unsigned int fix_vtotal = 0;
//        unsigned int v_load = 0;

	 unsigned int v_b05_load = 0;
	 unsigned int vo_fix_htotal = 0;
	 unsigned int vo_fix_vtotal = 0;

        //disable double buffer
        vodma_vodma_reg_db_ctrl2.regValue = rtd_inl(VODMA_VODMA_REG_DB_CTRL2_reg);
        vodma_vodma_reg_db_ctrl2.i2rnd_db_en = 0;
        IoReg_Write32(VODMA_VODMA_REG_DB_CTRL2_reg, vodma_vodma_reg_db_ctrl2.regValue);

        vo_i2rnd.regValue = rtd_inl(VODMA_vodma_i2rnd_reg);
        vo_i2rnd.i2rnd_flag_fifo_en = 0;
        vo_i2rnd.i2rnd_src_flag_inv = 0;
        vo_i2rnd.i2rnd_timing_mode = _1_MAIN_0_SUB;
        vo_i2rnd.i2rnd_timing_en = _ENABLE;

        //vo_i2rnd.i2rnd_timing_en = _DISABLE;
        IoReg_Write32(VODMA_vodma_i2rnd_reg, vo_i2rnd.regValue);

        vo_i2rnd_m_flag.regValue = rtd_inl(VODMA_vodma_i2rnd_m_flag_reg);
        vo_i2rnd_m_flag.i2rnd_m_field_fw = 0;
        vo_i2rnd_m_flag.i2rnd_m_block_free_run = 0;
     //   vo_i2rnd_m_flag.i2rnd_m_block_sel = 1;
        vo_i2rnd_m_flag.i2rnd_m_field_fw = 0;

        vo_i2rnd_m_flag.i2rnd_m_field_free_run = 0;
        vo_i2rnd_m_flag.i2rnd_m_field_sel = (Get_PANEL_VFLIP_ENABLE()? 2: 1);

        vo_i2rnd_m_flag.i2rnd_m_lflag_fw = 0;
        vo_i2rnd_m_flag.i2rnd_m_lflag_free_run = 0;
        vo_i2rnd_m_flag.i2rnd_m_lflag_sel = 0;

	 vo_i2rnd_m_flag.i2rnd_m_block_sel = 1;
	 vo_i2rnd_m_flag.i2rnd_hw_mask_repeat_frame_m_en = 0;
        IoReg_Write32(VODMA_vodma_i2rnd_m_flag_reg, vo_i2rnd_m_flag.regValue);


       h_active = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPH_ACT_WID_PRE);
       v_active = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE);
       interlace = Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE);
	frame_rate = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_FREQ);
	v_start = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_STA_PRE);
	h_start = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPH_ACT_STA_PRE);
        i2rnd_period_update_main = 1;
//	 rtd_pr_vbe_debug("frame_rate = %d\n",frame_rate);

        if( ((frame_rate % 10) == 0) )
                frame_rate = frame_rate / 10;
        else
                frame_rate = (frame_rate / 10) + 1;


//rtd_pr_vbe_debug("~~~~~~~~~~~~~~~~~~h_active = %d, v_active = %d, interlace = %d,frame_rate=%d\n", h_active, v_active, interlace,frame_rate);


	if(frame_rate && ((frame_rate< 51)|| (h_active > 2048))){
		vo_fix_htotal = (VO_FIXED_4K2K_HTOTAL * 95) / 100;
		vo_fix_vtotal = ((VO_FIXED_4K2K_VTOTAL*105)/100);

	}else{
		vo_fix_htotal = h_active + (h_start * 2) ;
		vo_fix_vtotal = VO_FIXED_4K2K_VTOTAL;

		}

		vo_clock = vo_fix_htotal*vo_fix_vtotal*frame_rate/1000000;
              fix_htotal = I2RND_VO_FIX_H_TOTAL_4K2K;
              fix_vtotal = I2RND_VO_FIX_V_TOTAL_4K2K;
		rtd_pr_vbe_debug("vo_clock=%d\n",vo_clock);


        if(display == SLR_MAIN_DISPLAY) {
                vo_clock_main = vo_clock;
                vo_fix_htotal_main = fix_htotal;
                vo_fix_vtotal_main = fix_vtotal;
                vo_fix_hload_main = fix_htotal;
                vo_frame_rate_main = frame_rate;
        } else {
                vo_clock_sub = vo_clock;
                vo_fix_htotal_sub = fix_htotal;
                vo_fix_vtotal_sub = fix_vtotal;
                vo_fix_hload_sub = fix_htotal;
                vo_frame_rate_sub = frame_rate;
        }

        if(i2rnd_period_update_main == 1) {
                rtd_pr_vbe_emerg("@@@@@@@@@@@@@@@@@@[MAIN PERIOD REFINE]@@@@@@@@@@@@@@@@@@@@@\n");
                rtd_pr_vbe_emerg("[%s]main_vo_clock = %d, sub_vo_clock = %d, fix_htotal = %d, fix_vtotal = %d, h_load = %d, frame_rate = %d\n", __FUNCTION__, vo_clock_main, vo_clock_sub, vo_fix_htotal_main, vo_fix_vtotal_main, vo_fix_hload_main, vo_frame_rate_main);

		v_b05_load = 1000000*vo_clock_main/frame_rate/vo_fix_htotal_main*1-vo_fix_vtotal_main;

               // rtd_pr_vbe_emerg("[%s]v_load = %d\n", __FUNCTION__, v_load / 100);
		  rtd_pr_vbe_debug("v_b05_load = %d\n",v_b05_load);
		  rtd_pr_vbe_debug("vo_fix_htotal_main = %d\n",vo_fix_htotal_main);
		  rtd_pr_vbe_debug("vo_fix_vtotal_main = %d\n",vo_fix_vtotal_main);

            //    period_min = v_load / 100 - 2048;
           //     period_load = vo_fix_htotal_main * vo_fix_vtotal_main;
           //    period_max = period_min + 1024;

		  period_min =(v_b05_load * vo_fix_htotal_main) - 2048;
                period_load = vo_fix_htotal_main * vo_fix_vtotal_main;
                period_max = period_min + 2048;

                rtd_pr_vbe_emerg("[%s]period_min = %d\n", __FUNCTION__, period_min);
                rtd_pr_vbe_emerg("[%s]period_load = %d\n", __FUNCTION__, period_load);
		  rtd_pr_vbe_emerg("[%s]period_max = %d\n", __FUNCTION__, period_max);
                //VODMA i2rnd period min
                vo_i2rnd_m_p_load.regValue = rtd_inl(VODMA_vodma_i2rnd_m_period_load_reg);
                vo_i2rnd_m_p_load.i2rnd_m_period_load = period_min;//htotal * period_min;//s0_period;//300;
                IoReg_Write32(VODMA_vodma_i2rnd_m_period_load_reg, vo_i2rnd_m_p_load.regValue);

                //VODMA i2rnd period
                vo_i2rnd_m_p.regValue = rtd_inl(VODMA_vodma_i2rnd_m_period_reg);
                vo_i2rnd_m_p.i2rnd_m_period = period_load;//htotal * period_load;//s0_period - 300;
                IoReg_Write32(VODMA_vodma_i2rnd_m_period_reg, vo_i2rnd_m_p.regValue);

                //VODMA i2rnd period max
                vodma_vodma_i2rnd_m_period_load_dbg.regValue = rtd_inl(VODMA_vodma_i2rnd_m_period_load_dbg_reg);
                vodma_vodma_i2rnd_m_period_load_dbg.i2rnd_m_period_load_max = period_max;//htotal * period_max;

                vodma_vodma_i2rnd_m_period_load_dbg.i2rnd_period_load_max_measure_en = 0;
                vodma_vodma_i2rnd_m_period_load_dbg.i2rnd_period_load_max_dbg_en = 0;

                IoReg_Write32(VODMA_vodma_i2rnd_m_period_load_dbg_reg, vodma_vodma_i2rnd_m_period_load_dbg.regValue);
        }
 // rtd_pr_vbe_debug("~~~~~~~~~~~~period_min= %d, period_load = %d, period_max = %d\n", period_min, period_load, period_max);

}


void set_b05_var(void)
{
        i2run_b05_addr = 0;

}

void vo_b05_debug(void)
{
        rtd_pr_vbe_debug("i2run_b05_addr: %x\n", i2run_b05_addr);
        rtd_pr_vbe_debug("b05_write_ddr_add: %x\n", (i2rnd_b05_scpu_vir_addr.s0_vo_st_vir_addr + i2run_b05_addr));
        rtd_pr_vbe_debug("s0_vo_st_vir_addr: %x\n", (i2rnd_b05_scpu_vir_addr.s0_vo_st_vir_addr));
       // rtd_pr_vbe_debug("i2run_b05_test: %x\n", i2run_b05_test);

}

void vo_b05_outl(unsigned int addr, unsigned int value)
{
#ifdef CONFIG_I2RND_B05_ENABLE
        //rtd_pr_vbe_debug("i2rnd_b05_scpu_vir_addr.i2rnd_enable_vir_addr =%x\n",i2rnd_b05_scpu_vir_addr.i2rnd_enable_vir_addr);
        if (Scaler_I2rnd_b05_get_enable()) {
                unsigned int curr_addr;
                curr_addr = i2rnd_b05_scpu_vir_addr.s0_vo_st_vir_addr + i2run_b05_addr;
                if (curr_addr) {
                        unsigned int tmp_addr = ((addr >> 2) & 0x000fffff) | 0x80000000;
                        curr_addr += 4;
                        *(volatile unsigned int *)(curr_addr) = __bswap_32(value);
                        curr_addr -= 4;
                        *(volatile unsigned int *)(curr_addr) = __bswap_32(tmp_addr);
                        //curr_addr+=8;
                        i2run_b05_addr += 8;

                } else {
                        (*(volatile unsigned int *)GET_MAPPED_RBUS_ADDR(addr) = value);
                }
        } else {
#endif
                (*(volatile unsigned int *)GET_MAPPED_RBUS_ADDR(addr) = value);
#ifdef CONFIG_I2RND_B05_ENABLE
        }
#endif
}



///////////////////////b05//////////////////////////////////////

#endif



/*======================== End of File =======================================*/
/**
*
* @}
*/




