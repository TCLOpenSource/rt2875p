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

#include <rbus/pst_i2rnd_reg.h>
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
#include <rbus/idcti_reg.h>
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
#include <rbus/de_xcxl_reg.h>
#include <rbus/tc_reg.h>
#include <rbus/color_mb_peaking_reg.h>
#include <rbus/ipq_decontour_reg.h>
#include <rbus/dfilter_reg.h>
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
#ifndef UINT8
typedef unsigned char  __UINT8;
#define UINT8 __UINT8
#endif
#ifndef UINT16
typedef unsigned short __UINT16;
#define UINT16 __UINT16
#endif
#ifndef UINT32
typedef unsigned int   __UINT32;
#define UINT32 __UINT32
#endif


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


unsigned char Scaler_I2rnd_get_Index(void)
{
	unsigned char ret_idx = 0;


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
	IoReg_Write32(VODMA_vodma_i2rnd_s_period_load_reg,IoReg_Read32(VODMA_vodma_i2rnd_m_period_load_reg));
	IoReg_Write32(VODMA_vodma_i2rnd_s_period_reg,IoReg_Read32(VODMA_vodma_i2rnd_m_period_reg));
	//set s1 flag fifo
	IoReg_Write32(VODMA_vodma_i2rnd_s_flag_reg,IoReg_Read32(VODMA_vodma_i2rnd_m_flag_reg));

	//set M-cap2 address
	i2rnd_pre_display = Scaler_I2rnd_get_display();
	Scaler_I2rnd_set_display(I2RND_SUB_S1_TABLE);
	i2rnd_default_allocate_mcap2();
	Scaler_I2rnd_set_display(i2rnd_pre_display);
	//pre-set DI start & boundary
	//i2rnd_default_allocate_DI2();
	//Set M-domain 3-buffer
	IoReg_SetBits(MDOMAIN_CAP_DDR_In1subCtrl_reg,_BIT6|_BIT8);

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




