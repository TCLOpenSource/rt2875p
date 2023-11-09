#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/uaccess.h>
#include <asm/cacheflush.h>
//#include <mach/RPCDriver.h>
#include <linux/pageremap.h>
/*some include about AP*/
#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <tvscalercontrol/scaler/scalertimer.h>
#include <tvscalercontrol/io/ioregdrv.h>

/*some include about VIP Driver*/
/*#include <tvscalercontrol/vip/scalerColor.h>*/
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <tvscalercontrol/vip/scalerColor_tv006.h>
#include <tvscalercontrol/vip/scalerColor_tv043.h>
#include <tvscalercontrol/vip/di_ma.h>
#include <tvscalercontrol/vip/peaking.h>
#include <tvscalercontrol/vip/icm.h>
#include <tvscalercontrol/vip/dcc.h>
#include <tvscalercontrol/vip/ultrazoom.h>
#include <tvscalercontrol/vip/color.h>
#include <tvscalercontrol/vip/xc.h>
#include <tvscalercontrol/vip/intra.h>
#include <tvscalercontrol/vip/nr.h>
#include <tvscalercontrol/vip/film.h>
#include <tvscalercontrol/vip/pq_adaptive.h>
#include <tvscalercontrol/vip/gibi_od.h>
#include <tvscalercontrol/vdc/video.h>
#include <tvscalercontrol/vip/localdimming.h>
#include <tvscalercontrol/vip/localcontrast.h>
#include <tvscalercontrol/vip/fcic.h>
#include <tvscalercontrol/vip/ST2094.h>
/*some include about scaler*/
#include <scaler/scalerDrvCommon.h>
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#include <tvscalercontrol/scalerdrv/scalermemory.h>
/*some include about rbus*/
/*some include about device driver*/
#include <tvscalercontrol/hdmirx/hdmifun.h>
//#include <tvscalercontrol/hdmirx/hdmiInternal.h>
#include <rtd_log/rtd_module_log.h>
//#include <mach/io.h>





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

//#define VIPprintf(fmt, args...)	printk(KERN_DEBUG fmt, ##args)	//20150822 roger mark for image release
#define VIPprintf(fmt, args...)
#define	vip_malloc(x)	kmalloc(x, GFP_KERNEL)
#define	vip_free(x)	kfree(x)
extern unsigned int vpq_project_id; /*pre_id:16 main_id:8 sub1_id:8 sub2_id:8, example->lg=tv006*/


void fwif_color_set_color_temp_tv043(unsigned char enable, unsigned short Red_Con, unsigned short Green_Con, unsigned short Blue_Con,
	short Red_Bri, short Green_Bri, short Blue_Bri)
{

	Red_Bri = Red_Bri - 2048;
	Green_Bri = Green_Bri - 2048;
	Blue_Bri = Blue_Bri - 2048;

	fwif_color_WaitFor_SYNC_START_UZUDTG();
	drvif_color_set_color_temp(enable, Red_Con, Green_Con, Blue_Con, Red_Bri, Green_Bri, Blue_Bri);
}

unsigned short LDDutyLimit[2] = {0,1023};
unsigned short Pre_DutyLimit[2] = {0,0};
unsigned char LDTableIdx;
unsigned short backlight_boost_lut[65] = {0, 16, 32, 48, 64, 80, 96, 112,
			128, 144, 160, 176, 192, 208, 224, 240,
			256, 272, 288, 304, 320, 336, 352, 368,
			384, 400, 416, 432, 448, 464, 480, 496,
			512, 528, 544, 560, 576, 592, 608, 624,
			640, 656, 672, 688, 704, 720, 736, 752,
			768, 784, 800, 816, 832, 848, 864, 880,
			896, 912, 928, 944, 960, 976, 992, 1008, 1023};
extern DRV_Local_Dimming_Table Local_Dimming_Table[LD_Table_NUM];


void fwif_color_set_LD_Boost_TV043(unsigned short *DutyLimit)
{

	DRV_LD_Boost data;
	int i;

	if (DutyLimit == NULL)
		return;
	
	LDDutyLimit[0] = DutyLimit[0];
	LDDutyLimit[1] = DutyLimit[1];	
	data.ld_backlightboosten= Local_Dimming_Table[LDTableIdx].LD_Boost.ld_backlightboosten;
	memcpy(&(data.ld_boost_gain_lut), &(Local_Dimming_Table[LDTableIdx].LD_Boost.ld_boost_gain_lut),sizeof(unsigned short)*65);		
	for (i=0;i<65;i++)
		data.ld_boost_curve_lut[i] = DutyLimit[0] + (Local_Dimming_Table[LDTableIdx].LD_Boost.ld_boost_curve_lut[i]* (DutyLimit[1]-DutyLimit[0]))/1023;
	drvif_color_set_LD_Boost((DRV_LD_Boost *) &data);
}



