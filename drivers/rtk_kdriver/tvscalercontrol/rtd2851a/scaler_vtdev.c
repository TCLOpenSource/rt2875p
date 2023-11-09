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
#include <linux/pageremap.h>
#include <uapi/linux/const.h>
#include <linux/mm.h>
#include <rtd_log/rtd_module_log.h>
#include <linux/fs.h>/*for dump buffer to usb file*/
#include <linux/hrtimer.h>/*for wait vsync hr sleep*/
#include <linux/version.h>
#include <rtk_kdriver/RPCDriver.h>
#include <rtk_dc_mt.h>
 
#ifdef CONFIG_HW_SUPPORT_DC2H 
#include <rbus/dc2h_dma_reg.h>
#include <rbus/dc2h_vi_reg.h>
#include <rbus/dc2h_scaledown_reg.h>
#include <rbus/dc2h_rgb2yuv_reg.h>
#include "rtk_kadp_se.h"
#else
#include <rbus/dma_vgip_reg.h>
#include <rbus/h3ddma_rgb2yuv_dither_4xxto4xx_reg.h>
#include <rbus/h3ddma_reg.h>
#include <rbus/h3ddma_hsd_reg.h>
//#include <rbus/h3ddma_vi_reg.h>
#include <rbus/vgip_reg.h>
#include <rbus/sub_vgip_reg.h>
#include <rbus/mdomain_cap_reg.h>
#include <rbus/mdomain_vi_sub_reg.h>
#include <rbus/iedge_smooth_reg.h>
#include <rbus/rgb2yuv_reg.h>
#include <tvscalercontrol/vip/color.h>

#endif

#include <rbus/ppoverlay_reg.h>
#include <rbus/mdomain_disp_reg.h>
#include <rbus/mdomain_cap_reg.h>
#include <rbus/scaledown_reg.h>
#include <rbus/scaleup_reg.h>
#include <rbus/yuv2rgb_reg.h>

#include <tvscalercontrol/vip/ultrazoom.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/panel/panelapi.h>
#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerCommon.h>
#else
#include <rtk_kdriver/scalercommon/scalerCommon.h>
#endif
#include <tvscalercontrol/scaler/scalerstruct.h>
#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <tvscalercontrol/scalerdrv/pipmp.h>
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#include <tvscalercontrol/scalerdrv/scalerdrv.h>
#include <scaler/vipRPCCommon.h>

#include "scaler_vtdev.h"
#include "scaler_vscdev.h"
#include "scaler_vpqmemcdev.h"


#define _ALIGN(val, align) (((val) + ((align) - 1)) & ~((align) - 1))
#define __4KPAGE  0x1000
/*4k and 96 align*/
#define __12KPAGE  0x3000
#define FMT_NV16 	1
#define FMT_NV12 	0

#ifndef FALSE
#define FALSE   0
#endif

#ifndef TRUE
#define TRUE    1
#endif

/*vt init work width and length*/
#define VT_CAP_FRAME_DEFAULT_WIDTH	1920
#define VT_CAP_FRAME_DEFAULT_LENGTH	1080

#define VT_SUPPORT_MAX_CAP_BUF_NUM	5
/*vt init work width and length*/
#define VT_CAP_FRAME_WIDTH_2K1k		1920
#define VT_CAP_FRAME_HEIGHT_2K1k	1080

#define VT_CAP_FRAME_WIDTH_4K2K		3840
#define VT_CAP_FRAME_HEIGHT_4K2K	2160

#define VR360_BORROW_12M_FROM_CMA (0xC00000)

#define CAP_LEN 32
#define CAP_REM 12

#define _RGB2YUV_m11				0x00
#define _RGB2YUV_m12				0x01
#define _RGB2YUV_m13				0x02
#define _RGB2YUV_m21				0x03
#define _RGB2YUV_m22				0x04
#define _RGB2YUV_m23				0x05
#define _RGB2YUV_m31				0x06
#define _RGB2YUV_m32				0x07
#define _RGB2YUV_m33				0x08
#define _RGB2YUV_Yo_even			0x09
#define _RGB2YUV_Yo_odd			0x0a
#define _RGB2YUV_Y_gain				0x0b
#define _RGB2YUV_sel_RGB			0x0c
#define _RGB2YUV_set_Yin_offset		0x0d
#define _RGB2YUV_set_UV_out_offset	0x0e
#define _RGB2YUV_sel_UV_off			0x0f
#define _RGB2YUV_Matrix_bypass		0x10
#define _RGB2YUV_Enable_Y_gain		0x11

unsigned int VT_CAPTURE_BUFFER_UNITSIZE;

VT_CAPTURE_CTRL_T CaptureCtrl_VT;

#ifndef UT_flag
static dev_t vivt_devno = 0;//vt device number
static struct cdev vivt_cdev;
struct semaphore VT_Semaphore;
static struct semaphore VTBlock_Semaphore;

#endif

//static KADP_VT_VIDEO_FRAME_OUTPUT_DEVICE_STATE_INFO_T VFODState = {FALSE,FALSE,TRUE,1};
KADP_VT_VIDEO_FRAME_OUTPUT_DEVICE_STATE_INFO_T VFODState = {FALSE,FALSE,TRUE,1};

extern void h3ddma_set_capture_enable(unsigned char chanel,unsigned char enable);
extern unsigned char get_vsc_mutestatus(void);
extern unsigned char drvif_scaler_get_abnormal_dvs_long_flag(void);
extern unsigned char drvif_scaler_get_abnormal_dvs_short_flag(void);
static unsigned char VtFunction = FALSE;
static unsigned char VtEnableFRCMode = TRUE;

static unsigned int VtBufferNum = 3;
static unsigned char VtSwBufferMode = FALSE;
static unsigned char VtCaptureVDC = FALSE;

static unsigned int vt_cap_frame_max_width = VT_CAP_FRAME_WIDTH_2K1k; 
static unsigned int vt_cap_frame_max_height = VT_CAP_FRAME_HEIGHT_2K1k;

static unsigned long VtAllocatedBufferStartAdress[5] = {0};
#ifndef UT_flag
static VT_CUR_CAPTURE_INFO curCapInfo;
static KADP_VT_DUMP_LOCATION_TYPE_T VTDumpLocation = KADP_VT_MAX_LOCATION;
static unsigned char VdecSecureStatus = FALSE;
static unsigned char DtvSecureStatus = FALSE;
static unsigned int VtFreezeRegAddr = 0;
#ifndef CONFIG_HW_SUPPORT_DC2H 
	static unsigned char VT_vactend_ie = FALSE;
#endif
#endif

#ifdef CONFIG_HW_SUPPORT_DC2H 
	DC2H_HANDLER *pdc2h_hdl = NULL;
	static DC2H_HANDLER dc2h_hdl;
#endif
/*ARGB8888_FORMAT for roku
  RGB888 for LGE*/
/**
RGB888 for DDTS
*/
static VT_CAP_FMT VT_Pixel_Format = VT_CAP_RGB888;
//static unsigned char VtBlockCapFlag = FALSE;

#ifdef CONFIG_HW_SUPPORT_DC2H

static unsigned int DC2H_InputWidthSta;
static unsigned int DC2H_InputLengthSta;
static unsigned int DC2H_InputWidth;
static unsigned int DC2H_InputLength;
#endif

extern KADP_VIDEO_RECT_T ap_main_inregion_parm;//for new input output. main path ap parameter  from new hal
extern KADP_VIDEO_RECT_T ap_main_originalInput_parm;//for new input output. main path ap input resolution parameter from new hal
extern unsigned short tRGB2YUV_COEF[RGB2YUV_COEF_MATRIX_MODE_Max][RGB2YUV_COEF_MATRIX_ITEMS_Max];

#ifdef CONFIG_ENABLE_HDMI_NN
extern unsigned int g_ulNNOutWidth; 
extern unsigned int g_ulNNOutLength; 
#endif 

unsigned char IndexOfFreezedVideoFrameBuffer = 0;
//static unsigned int VtFreezeRegAddr = 0;

volatile unsigned int vfod_capture_out_W = VT_CAP_FRAME_DEFAULT_WIDTH;
volatile unsigned int vfod_capture_out_H = VT_CAP_FRAME_DEFAULT_LENGTH;
volatile unsigned int vfod_capture_location = KADP_VT_DISPLAY_OUTPUT;

void set_vfod_freezed_for_dc2h(unsigned char boffon);
void set_vfod_freezed_for_i3ddma_sub_capture(unsigned char boffon);
unsigned char get_dc2h_capture_state(void);
void reset_dc2h_hw_setting(void);
unsigned char HAL_VT_EnableFRCMode(unsigned char bEnableFRC);
unsigned char Capture_BufferMemInit_VT(unsigned int buffernum);
void Capture_BufferMemDeInit_VT(unsigned int buffernum);
unsigned char get_i3ddma_idlestatus(void);
void i3ddma_cap1cap2_vi_VtCap_freeze(unsigned char enable);
void i3ddma_cap0_seq_VtCap_freeze(unsigned char enable);
void sub_mdomain_vi_VtCap_freeze(unsigned char enable);
void sub_mdomain_seq_VtCap_freeze(unsigned char enable);
void vt_i3ddma_vi_capture_enable(unsigned int enable);
int vt_dump_data_to_file(unsigned int bufferindex, unsigned int size);

#ifdef CONFIG_HW_SUPPORT_DC2H 

//extern void MEMC_Set_malloc_address(UINT8 status);
void drvif_DC2H_dispD_CaptureConfig(VT_CUR_CAPTURE_INFO capInfo);
unsigned char is_4K2K_capture(void);
void set_cap_buffer_size_by_AP(unsigned int usr_width, unsigned int usr_height);
//void reset_dc2h_hw_setting(void);

_RPC_system_setting_info* scaler_GetShare_Memory_RPC_system_setting_info_Struct(void);
unsigned char drvif_vdec_NN_SE_proc(void);
unsigned char drvif_i3ddma_NN_SE_proc(void);
void drvif_do_se_stretch(unsigned int src_w, unsigned int src_h, unsigned int src_addr_y, unsigned int src_addr_uv, KGAL_PIXEL_FORMAT_T src_fmt);
void start_stop_dc2h(unsigned char state);
unsigned char get_dc2h_capture_state(void);
unsigned int sort_boundary_addr_min_index(void);
unsigned int sort_boundary_addr_max_index(void);

extern void dcmt_monitor_dc2h_range(int index, unsigned int addr, unsigned int size);
extern void dcmt_clear_monitor(unsigned int entry);
extern int vt_dump_data_to_file(unsigned int bufferindex, unsigned int size);

#ifdef CONFIG_ENABLE_HDMI_NN
extern unsigned int h3ddma_get_NN_read_buffer(unsigned int *a_pulYAddr, unsigned int *a_pulCAddr);
#endif

#endif
unsigned char get_vt_function(void)
{
	return VtFunction;
}

void set_vt_function(unsigned char value)
{
	VtFunction = value;
}

unsigned char get_vt_EnableFRCMode(void)
{
	return VtEnableFRCMode;
}

void set_vt_EnableFRCMode(unsigned char value)
{
	VtEnableFRCMode = value;
}


unsigned int get_vt_VtBufferNum(void)
{
	return VtBufferNum;
}

void set_vt_VtBufferNum(unsigned int value)
{
	VtBufferNum = value;
}

unsigned char get_vt_VtSwBufferMode(void)
{
	return VtSwBufferMode;
}

void set_vt_VtSwBufferMode(unsigned char value)
{
	VtSwBufferMode = value;
}

unsigned char get_vt_VtCaptureVDC(void)
{
	return VtCaptureVDC;
}

void set_vt_VtCaptureVDC(unsigned char value)
{
	VtCaptureVDC = value;
}

void set_VT_Pixel_Format(VT_CAP_FMT value)
{
	VT_Pixel_Format = value;
}

VT_CAP_FMT get_VT_Pixel_Format(void)
{
	return VT_Pixel_Format;
}
unsigned int get_framerateDivide(void)
{
	return VFODState.framerateDivide;
}
void set_framerateDivide(unsigned int value)
{
	VFODState.framerateDivide = value;
}

unsigned int get_cap_buffer_Width(void)
{
	return vt_cap_frame_max_width;
}
unsigned int get_cap_buffer_Height(void)
{
	return vt_cap_frame_max_height;
}

unsigned int get_vt_capBuf_Addr(unsigned int idx)
{
    return CaptureCtrl_VT.cap_buffer[idx].phyaddr;
}

typedef struct
{
    SIZE in_size;
    SIZE out_size;
    DC2H_YC_PIC_NO yc_pic_no;
    //unsigned char current_idx;
    unsigned char dc2h_init;
    unsigned char dc2h_en;
    unsigned char dc2h_in_sel;
    unsigned char panorama;
    unsigned char set_bit;
    unsigned char done_bit;
} DC2H_HANDLER ;

#ifdef CONFIG_HW_SUPPORT_DC2H 

/*YUV to RBG table for RGB888 and ARGB888 capture format*/
static unsigned short tYUV2RGB_COEF_709_RGB_0_255[] =
{

// CCIR 709 RGB
        0x129f,   // m11
        0x0000,  // m12
        0x1660,  // m13

        0x129f,   // m21 >> 2 0x01D5 up bits at AT Name: cychen2 , Date: 2010/3/25
        0x7a80,   // m22 >> 1
        0x74a0,   // m23 >> 2
        0x129f,   // m31 >> 2
        0x1c50,   // m32 >> 2 0x0195  up bits at AT Name: cychen2 , Date: 2010/3/25
        0x0000,   // m33 >> 1

        0x0000,          // Yo_even
        0x0000,          // Yo_odd
        0x0000,          // Y_gain

        0x0000,  // sel_RGB
        0x0001,  // sel_Yin_offset
        0x0000,  // sel_UV_out_offset
        0x0000,  // sel_UV_off
        0x0000,  // Matrix_bypass
        0x0000,  // Enable_Y_gain

};

#endif

#ifdef CONFIG_HW_SUPPORT_DC2H 

/*RGB to YUV table for nv12 nv16 capture format*/
static unsigned short tYUV2RGB_COEF_709_RGB_16_235[] =
{
    // CCIR 709 RGB
            0x04c8,   // m11
            0x0964,   // m12
            0x01d2,   // m13
#if 0 /*uv channel could swap*/            
			/*nv12 uv need swap*/
            0x0800,   // m31 >> 2
            0x794d,   // m32 >> 2 0x0195  up bits at AT Name: cychen2 , Date: 2010/3/25
            0x7eb2,   // m33 >> 1

            0x7d4d,   // m21 >> 2 0x01D5 up bits at AT Name: cychen2 , Date: 2010/3/25
            0x7ab2,   // m22 >> 1
            0x0800,   // m23 >> 2
#else
			0x7d4d,   // m21 >> 2 0x01D5 up bits at AT Name: cychen2 , Date: 2010/3/25
			0x7ab2,   // m22 >> 1
			0x0800,   // m23 >> 2
			0x0800,   // m31 >> 2
			0x794d,   // m32 >> 2 0x0195  up bits at AT Name: cychen2 , Date: 2010/3/25
			0x7eb2,   // m33 >> 1
#endif
            0x0000,//0x07e0,          // Yo_even
            0x0000,//0x07e0,          // Yo_odd
            0x0129,//0x0110,          // Y_gain

            0x0001,  // sel_RGB
            0x0001,//0x0000,  // sel_Yin_offset
            0x0001,  // sel_UV_out_offset
            0x0000,  // sel_UV_off
            0x0000,  // Matrix_bypass
            0x0001,  // Enable_Y_gain

};
#endif
#ifndef UT_flag
static short tScale_Down_Coef_Blur[] = {
	2,   3,   5,   7,  10,  15,  20,  28,  38,  49,  64,  81, 101, 124, 150, 178,
	209, 242, 277, 314, 351, 389, 426, 462, 496, 529, 556, 582, 602, 618, 629, 635,
	//213,  217,  221,  222,  224,  229,  232,  234,
	//239,  242,  245,  249,  252,  255,  259,  262,
	//265,  266,  267,  269,  271,  272,  273,  275,
	//276,  277,  278,  280,  281,  281,  282,  284,
};

static short tScale_Down_Coef_Mid[] = {
	-2,  0,   1,   2,   5,   9,  15,  22,  32,  45,  60,  77,  98, 122, 149, 179,
	211, 245, 281, 318, 356, 394, 431, 468, 502, 533, 561, 586, 606, 620, 630, 636,
	//131,  125,  124,  126,  130,  135,  142,  151,
	//161,  172,  183,  195,  208,  221,  234,  245,
	//260,  275,  290,  304,  318,  330,  341,  351,
	//361,  369,  376,  381,  386,  389,  390,  388,
};

static short tScale_Down_Coef_Sharp[] = {
    -2,   0,   1,   3,   6,  10,  15,  22,  32,  43,  58,  75,  95, 119, 145, 174,
    206, 240, 276, 314, 353, 391, 430, 467, 503, 536, 565, 590, 612, 628, 639, 646,
};

static short tScale_Down_Coef_2tap[] = {
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    31,  95, 159, 223, 287, 351, 415, 479,
    543, 607, 671, 735, 799, 863, 927, 991,
};

static  short tScale_Down_Coef_repeat[] = {
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
};

static  short *tScaleDown_COEF_TAB[] =
{
    tScale_Down_Coef_Blur,
	tScale_Down_Coef_Mid,
	tScale_Down_Coef_Sharp,
	tScale_Down_Coef_2tap,
	tScale_Down_Coef_repeat
};
#endif

#ifndef UT_flag

#ifdef CONFIG_HW_SUPPORT_DC2H 

void dc2h_wait_porch(void)
{
	unsigned int uzulinecntA =0;
	unsigned int uzulinecntB =0;

	unsigned int timeoutcount = 0x3ffff;
	ppoverlay_dv_den_start_end_RBUS dv_den_start_end_reg;
	ppoverlay_new_meas1_linecnt_real_RBUS new_meas1_linecnt_real_Reg;

	dv_den_start_end_reg.regValue = rtd_inl(PPOVERLAY_DV_DEN_Start_End_reg);
	do {
		new_meas1_linecnt_real_Reg.regValue = rtd_inl(PPOVERLAY_new_meas1_linecnt_real_reg);
		uzulinecntA = new_meas1_linecnt_real_Reg.uzudtg_line_cnt_rt;
		new_meas1_linecnt_real_Reg.regValue = rtd_inl(PPOVERLAY_new_meas1_linecnt_real_reg);
		uzulinecntB = new_meas1_linecnt_real_Reg.uzudtg_line_cnt_rt;
		if((uzulinecntA == uzulinecntB)&&((uzulinecntA < (dv_den_start_end_reg.dv_den_sta))||(uzulinecntA > (dv_den_start_end_reg.dv_den_end)))) {
			break;
		}
		timeoutcount--;
	} while(timeoutcount);

	if(timeoutcount == 0){
		rtd_pr_vt_debug("[UZU DEN]timeout error!!!\n");
	}

}
void start_stop_dc2h(unsigned char state)
{
	down(&VT_Semaphore);
	curCapInfo.enable = state; 
	drvif_DC2H_dispD_CaptureConfig(curCapInfo);
	up(&VT_Semaphore);
}
unsigned char get_dc2h_capture_state(void)
{	
	if ((get_VT_Pixel_Format() == VT_CAP_NV12) || (get_VT_Pixel_Format() == VT_CAP_NV16))
	{
		if(DC2H_VI_DC2H_DMACTL_get_dmaen1(rtd_inl(DC2H_VI_DC2H_DMACTL_reg)) == 1)		
			return TRUE;
		else
			return FALSE;
		
	}
	else
	{
		if(DC2H_DMA_dc2h_Seq_mode_CTRL1_get_cap_en(rtd_inl(DC2H_DMA_dc2h_Seq_mode_CTRL1_reg)) == DC2H_DMA_dc2h_Seq_mode_CTRL1_cap_en_mask)
			return TRUE;
		else
			return FALSE;
	}

}

unsigned char drvif_dc2h_swmode_inforpc(unsigned int onoff,unsigned int buffernumber,unsigned int width,unsigned int length){

	DC2H_SWMODE_STRUCT_T *swmode_infoptr;
	unsigned int ulCount = 0;
	int ret;
	rtd_pr_vt_notice("fun:%s\n",__FUNCTION__);
	swmode_infoptr = (DC2H_SWMODE_STRUCT_T *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_DC2H_SWMODE_ENABLE);
	ulCount = sizeof(DC2H_SWMODE_STRUCT_T) / sizeof(unsigned int);

	swmode_infoptr->SwModeEnable = onoff;
	swmode_infoptr->buffernumber = buffernumber;
	swmode_infoptr->cap_format = (UINT32)get_VT_Pixel_Format();
	swmode_infoptr->cap_width = width;
	swmode_infoptr->cap_length = length;
	swmode_infoptr->YbufferSize = _ALIGN(vt_cap_frame_max_width*vt_cap_frame_max_height,__12KPAGE);
	swmode_infoptr->cap_buffer[0] = (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;
	swmode_infoptr->cap_buffer[1] = (UINT32)CaptureCtrl_VT.cap_buffer[1].phyaddr;
	swmode_infoptr->cap_buffer[2] = (UINT32)CaptureCtrl_VT.cap_buffer[2].phyaddr;
	swmode_infoptr->cap_buffer[3] = (UINT32)CaptureCtrl_VT.cap_buffer[3].phyaddr;
	swmode_infoptr->cap_buffer[4] = (UINT32)CaptureCtrl_VT.cap_buffer[4].phyaddr;

	
	rtd_pr_vt_debug("[VT]SwModeEnable = %d\n",swmode_infoptr->SwModeEnable);
	rtd_pr_vt_debug("[VT]buffernumber = %d\n",swmode_infoptr->buffernumber);
	rtd_pr_vt_debug("[VT]cap_format = %d\n",swmode_infoptr->cap_format);
	rtd_pr_vt_debug("[VT]cap_width = %d\n",swmode_infoptr->cap_width);
	rtd_pr_vt_debug("[VT]cap_length = %d\n",swmode_infoptr->cap_length);
	rtd_pr_vt_debug("[VT]YbufferSize = %d\n",swmode_infoptr->YbufferSize);
	rtd_pr_vt_debug("[VT]cap_buffer[0] = 0x%x\n",swmode_infoptr->cap_buffer[0]);
	rtd_pr_vt_debug("[VT]cap_buffer[1] = 0x%x\n",swmode_infoptr->cap_buffer[1]);
	rtd_pr_vt_debug("[VT]cap_buffer[2] = 0x%x\n",swmode_infoptr->cap_buffer[2]);
	rtd_pr_vt_debug("[VT]cap_buffer[3] = 0x%x\n",swmode_infoptr->cap_buffer[3]);
	rtd_pr_vt_debug("[VT]cap_buffer[4] = 0x%x\n",swmode_infoptr->cap_buffer[4]);
	//change endian
	swmode_infoptr->SwModeEnable = htonl(swmode_infoptr->SwModeEnable);
	swmode_infoptr->buffernumber = htonl(swmode_infoptr->buffernumber);
	swmode_infoptr->cap_format = htonl(swmode_infoptr->cap_format);
	swmode_infoptr->cap_width = htonl(swmode_infoptr->cap_width);
	swmode_infoptr->cap_length = htonl(swmode_infoptr->cap_length);
	swmode_infoptr->YbufferSize = htonl(swmode_infoptr->YbufferSize);
	swmode_infoptr->cap_buffer[0] = htonl(swmode_infoptr->cap_buffer[0]);
	swmode_infoptr->cap_buffer[1] = htonl(swmode_infoptr->cap_buffer[1]);
	swmode_infoptr->cap_buffer[2] = htonl(swmode_infoptr->cap_buffer[2]);
	swmode_infoptr->cap_buffer[3] = htonl(swmode_infoptr->cap_buffer[3]);
	swmode_infoptr->cap_buffer[4] = htonl(swmode_infoptr->cap_buffer[4]);

	
	if (0 != (ret = Scaler_SendRPC(SCALERIOC_SET_DC2H_SWMODE_ENABLE,0,0))){
		rtd_pr_vt_emerg("[dc2h]ret=%d, SCALERIOC_SET_DC2H_SWMODE_ENABLE RPC fail !!!\n", ret);
		return FALSE;
	}
	return TRUE;
}

unsigned char drvif_i3ddma_NN_SE_proc(void)
{
	/* VT use i3ddma NN buffer to do stretch size required by user client */
#ifdef CONFIG_ENABLE_HDMI_NN

	unsigned int SE_Addr_Cur_Y = 0;
	unsigned int SE_width = 0;    //src_w
	unsigned int SE_height = 0;   // src_h
	unsigned int SE_Addr_Cur_UV = 0;
	
	rtd_pr_vt_notice("fun:%s\n",__FUNCTION__);	
	
	SE_width = g_ulNNOutWidth;
	SE_height = g_ulNNOutLength;

	rtd_pr_vt_notice("i3ddma NN buffer size:<w,h>:<%d,%d>\n", SE_width, SE_height);
	// get i3ddma NN buffer address 
	if(h3ddma_get_NN_read_buffer(&SE_Addr_Cur_Y, &SE_Addr_Cur_UV) < 0)
	{
		rtd_pr_vt_notice("get i3ddma NN buffer fail\n");
		return FALSE;
	}
	rtd_pr_vt_notice("i3ddma NN buffer addr:<y,uv>:<0x%x,0x%x>\n", SE_Addr_Cur_Y, SE_Addr_Cur_UV);
							
		
	drvif_do_se_stretch(SE_width, SE_height, SE_Addr_Cur_Y, SE_Addr_Cur_UV, KGAL_PIXEL_FORMAT_NV12);
			
	return TRUE;
#else
	
	rtd_pr_vt_notice("i3ddma NN buffer not exist! error\n");
	return FALSE;
#endif

}

unsigned char drvif_vdec_NN_SE_proc(void)
{
	unsigned char SE_rdPtr = 0;
	unsigned char SE_status = 0;
	unsigned int SE_Addr_Cur_tmp = 0;
	unsigned int SE_width_tmp = 0;
	unsigned int SE_height_tmp = 0;
	unsigned int SE_Addr_Cur = 0;
	unsigned int SE_width = 0;
	unsigned int SE_height = 0;

	unsigned int SE_Addr_Cur_UV_tmp = 0;
	unsigned int SE_Addr_Cur_UV = 0;
	
	_RPC_system_setting_info* RPC_system_info_structure_table = NULL;

	RPC_system_info_structure_table = scaler_GetShare_Memory_RPC_system_setting_info_Struct();

	if((RPC_system_info_structure_table == NULL))
	{
		rtd_pr_vt_emerg(" Mem4 = %p\n",RPC_system_info_structure_table);
		return FALSE;
	}
	
	SE_rdPtr = RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.rdPtr;
	SE_status = RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.pic[SE_rdPtr].status;
	SE_Addr_Cur_tmp = RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.pic[SE_rdPtr].SeqBufAddr_Curr;
	SE_width_tmp = RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.pic[SE_rdPtr].Width;
	SE_height_tmp = RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.pic[SE_rdPtr].Height;

	SE_Addr_Cur = Scaler_ChangeUINT32Endian(SE_Addr_Cur_tmp);
	SE_width = Scaler_ChangeUINT32Endian(SE_width_tmp);
	SE_height = Scaler_ChangeUINT32Endian(SE_height_tmp);

	if(SE_status == 1)
	{		
		SE_Addr_Cur_UV_tmp = RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.pic[SE_rdPtr].SeqBufAddr_Curr_UV;
		SE_Addr_Cur_UV = Scaler_ChangeUINT32Endian(SE_Addr_Cur_UV_tmp);
		
		//if(SE_rdPtr == 0)
		//{
			//NN_SE_Proc(SE_width, SE_height, SE_Addr_Cur, SE_Addr_Cur_UV, KGAL_PIXEL_FORMAT_NV12);	
			drvif_do_se_stretch(SE_width, SE_height, SE_Addr_Cur, SE_Addr_Cur_UV, KGAL_PIXEL_FORMAT_NV12);
		//}
	}	
	return TRUE;
	
}
void drvif_do_se_stretch(unsigned int src_w, unsigned int src_h, unsigned int src_addr_y, unsigned int src_addr_uv, KGAL_PIXEL_FORMAT_T src_fmt)
{
	unsigned int src_c_addr = 0;
	unsigned int dst_c_addr = 0;
	KGAL_PIXEL_FORMAT_T dst_fmt = KGAL_PIXEL_FORMAT_MAX;
	
	bool status=0;
	KGAL_SURFACE_INFO_T ssurf;
	KGAL_SURFACE_INFO_T dsurf;
	KGAL_RECT_T srect;
	KGAL_RECT_T drect;
	//KGAL_BLIT_FLAGS_T sflag = KGAL_BLIT_NOFX;
	KGAL_BLIT_SETTINGS_T sblend;
	
	rtd_pr_vt_notice("fun:%s=%d\n",__FUNCTION__, __LINE__);
	
	memset(&ssurf,0, sizeof(KGAL_SURFACE_INFO_T));
	memset(&dsurf,0, sizeof(KGAL_SURFACE_INFO_T));
	memset(&srect,0, sizeof(KGAL_RECT_T));
	memset(&drect,0, sizeof(KGAL_RECT_T));
	memset(&sblend,0, sizeof(KGAL_BLIT_SETTINGS_T));
	sblend.srcBlend = KGAL_BLEND_ONE;
	sblend.dstBlend = KGAL_BLEND_ZERO;
	dst_fmt = src_fmt;
	
//---------------------------------------------------------------------------------------------------------------	
	ssurf.physicalAddress = src_addr_y;
	ssurf.width 		  = src_w;
	ssurf.height		  = src_h;
	ssurf.bpp			  = 16;
	ssurf.pitch			  = src_w;
	ssurf.pixelFormat 	  = src_fmt;
	srect.x 			  = 0;
	srect.y               = 0;
	srect.w               = src_w;
	srect.h               = src_h;
	src_c_addr			  = src_addr_uv;

//-----------------------------------------------------------------------------------------------------------------
	dsurf.physicalAddress = CaptureCtrl_VT.cap_buffer[0].phyaddr;
	dsurf.width           = vfod_capture_out_W;
	dsurf.height          = vfod_capture_out_H;
	dsurf.bpp             = 16;
	dsurf.pitch           = vfod_capture_out_W;
	dsurf.pixelFormat     = dst_fmt;
	drect.x               = 0;
	drect.y               = 0;
	drect.w               = vfod_capture_out_W;
	drect.h               = vfod_capture_out_H;
	dst_c_addr            = (CaptureCtrl_VT.cap_buffer[0].phyaddr + _ALIGN(vt_cap_frame_max_width*vt_cap_frame_max_height,__12KPAGE));
//-------------------------------------------------------------------------------------------------

#ifdef CONFIG_HW_SUPPORT_DC2H 
//#FixMe, 20190912	
	status = KGAL_NV12_StretchBlit(&ssurf, &srect, &dsurf, &drect, &sflag, &sblend, src_c_addr, dst_c_addr);
#endif

	if(!status)
	{
		rtd_pr_vt_notice("[error]KGAL_NV12_StretchBlit run fail\n");
		
	}
	else
	{
		rtd_pr_vt_notice("src(%d,%d) => dst(%d,%d)\n",ssurf.width, ssurf.height, dsurf.width, dsurf.height);
	}
}

unsigned char get_video_do_overscan_state(void)
{
	/*if(Scaler_DispGetInputInfo(SLR_INPUT_SDNRH_ACT_STA_PRE) != Scaler_DispGetInputInfo(SLR_INPUT_SDNRH_ACT_STA) ||
	Scaler_DispGetInputInfo(SLR_INPUT_SDNRV_ACT_STA_PRE) != Scaler_DispGetInputInfo(SLR_INPUT_SDNRV_ACT_STA) ||
	Scaler_DispGetInputInfo(SLR_INPUT_SDNRH_ACT_WID_PRE) != Scaler_DispGetInputInfo(SLR_INPUT_SDNRH_ACT_WID) ||
	Scaler_DispGetInputInfo(SLR_INPUT_SDNRV_ACT_STA_PRE) != Scaler_DispGetInputInfo(SLR_INPUT_SDNRV_ACT_LEN) )*/
#if 0
	if((ap_main_inregion_parm.w != ap_main_originalInput_parm.w) || (ap_main_inregion_parm.h != ap_main_originalInput_parm.h))
	{
		return TRUE;
	}
	else
		return FALSE;
#else
	return FALSE;
#endif
}

void drvif_color_colorspaceyuv2rgbtransfer(DC2H_IN_SEL dc2h_in_sel)
{
	unsigned short *table_index = 0;
	
	dc2h_rgb2yuv_dc2h_rgb2yuv_ctrl_RBUS  dc2h_rgb2yuv_ctrl_reg;
	dc2h_rgb2yuv_dc2h_tab1_m11_m12_RBUS dc2h_tab1_m11_m12_reg;
	dc2h_rgb2yuv_dc2h_tab1_m13_m21_RBUS dc2h_tab1_m13_m21_reg;
	dc2h_rgb2yuv_dc2h_tab1_m22_m23_RBUS dc2h_tab1_m22_m23_reg;
	dc2h_rgb2yuv_dc2h_tab1_m31_m32_RBUS dc2h_tab1_m31_m32_reg;
	dc2h_rgb2yuv_dc2h_tab1_m33_y_gain_RBUS dc2h_tab1_m33_y_gain_reg;
	dc2h_rgb2yuv_dc2h_tab1_yo_RBUS dc2h_tab1_yo_reg;
	
	if((get_VT_Pixel_Format() == VT_CAP_NV12)||(get_VT_Pixel_Format() == VT_CAP_NV16))
		table_index = tYUV2RGB_COEF_709_RGB_16_235;
	else
		table_index = tYUV2RGB_COEF_709_RGB_0_255;

	//main  all tab-1
	dc2h_tab1_m11_m12_reg.m11 = table_index [_RGB2YUV_m11];
	dc2h_tab1_m11_m12_reg.m12 = table_index [_RGB2YUV_m12];
	//rtd_pr_vt_info("dc2h_tab1_m11_m12_reg.regValue = %x\n", dc2h_tab1_m11_m12_reg.regValue);
	rtd_outl(DC2H_RGB2YUV_DC2H_Tab1_M11_M12_reg, dc2h_tab1_m11_m12_reg.regValue);

	dc2h_tab1_m13_m21_reg.m21 = table_index [_RGB2YUV_m21];
	dc2h_tab1_m13_m21_reg.m13 = table_index [_RGB2YUV_m13];
	//rtd_pr_vt_info("dc2h_tab1_m13_m21_reg.regValue = %x\n", dc2h_tab1_m13_m21_reg.regValue);
	rtd_outl(DC2H_RGB2YUV_DC2H_Tab1_M13_M21_reg, dc2h_tab1_m13_m21_reg.regValue);

	dc2h_tab1_m22_m23_reg.m22 = table_index [_RGB2YUV_m22];
	dc2h_tab1_m22_m23_reg.m23 = table_index [_RGB2YUV_m23];
	//rtd_pr_vt_info("dc2h_tab1_m22_m23_reg.regValue = %x\n", dc2h_tab1_m22_m23_reg.regValue);
	rtd_outl(DC2H_RGB2YUV_DC2H_Tab1_M22_M23_reg, dc2h_tab1_m22_m23_reg.regValue);

	dc2h_tab1_m31_m32_reg.m31 = table_index [_RGB2YUV_m31];
	dc2h_tab1_m31_m32_reg.m32 = table_index [_RGB2YUV_m32];
	//rtd_pr_vt_info("dc2h_tab1_m31_m32_reg.regValue = %x\n", dc2h_tab1_m31_m32_reg.regValue);
	rtd_outl(DC2H_RGB2YUV_DC2H_Tab1_M31_M32_reg, dc2h_tab1_m31_m32_reg.regValue);

	dc2h_tab1_m33_y_gain_reg.m33 = table_index [_RGB2YUV_m33];
	dc2h_tab1_m33_y_gain_reg.yo_gain= table_index [_RGB2YUV_Y_gain];
	//rtd_pr_vt_info("dc2h_tab1_m33_y_gain_reg.regValue = %x\n", dc2h_tab1_m33_y_gain_reg.regValue);
	rtd_outl(DC2H_RGB2YUV_DC2H_Tab1_M33_Y_Gain_reg, dc2h_tab1_m33_y_gain_reg.regValue);

	dc2h_tab1_yo_reg.yo_even= table_index [_RGB2YUV_Yo_even];
	dc2h_tab1_yo_reg.yo_odd= table_index [_RGB2YUV_Yo_odd];
	//rtd_pr_vt_info("dc2h_tab1_yo_reg.regValue = %x\n", dc2h_tab1_yo_reg.regValue);
	rtd_outl(DC2H_RGB2YUV_DC2H_Tab1_Yo_reg, dc2h_tab1_yo_reg.regValue);

	dc2h_rgb2yuv_ctrl_reg.regValue = rtd_inl(DC2H_RGB2YUV_DC2H_RGB2YUV_CTRL_reg);
	dc2h_rgb2yuv_ctrl_reg.sel_rgb= table_index [_RGB2YUV_sel_RGB];
	dc2h_rgb2yuv_ctrl_reg.set_r_in_offset= table_index [_RGB2YUV_set_Yin_offset];
	dc2h_rgb2yuv_ctrl_reg.set_uv_out_offset= table_index [_RGB2YUV_set_UV_out_offset];
	dc2h_rgb2yuv_ctrl_reg.sel_uv_off= table_index [_RGB2YUV_sel_UV_off];
	dc2h_rgb2yuv_ctrl_reg.matrix_bypass= table_index [_RGB2YUV_Matrix_bypass];
	dc2h_rgb2yuv_ctrl_reg.sel_y_gain= table_index [_RGB2YUV_Enable_Y_gain];
	dc2h_rgb2yuv_ctrl_reg.dc2h_in_sel = dc2h_in_sel;
	
	if((get_VT_Pixel_Format() == VT_CAP_NV12)||(get_VT_Pixel_Format() == VT_CAP_NV16))
	{
		if((dc2h_in_sel == _MAIN_UZU) || (dc2h_in_sel == _TWO_SECOND_UZU))  //YUV format
		{
			dc2h_rgb2yuv_ctrl_reg.en_rgb2yuv = 0;
		}else{
			dc2h_rgb2yuv_ctrl_reg.en_rgb2yuv = 1; //Enable RGB to YUV conversion (YUV out)
		}
	} 
	else
	{
		if((dc2h_in_sel == _MAIN_UZU) || (dc2h_in_sel == _TWO_SECOND_UZU))
		{
			dc2h_rgb2yuv_ctrl_reg.en_rgb2yuv = 2; //Enable YUV to RGB conversion (GBR out)
		}else{
			dc2h_rgb2yuv_ctrl_reg.en_rgb2yuv = 0;
		}
	}
	rtd_outl(DC2H_RGB2YUV_DC2H_RGB2YUV_CTRL_reg, dc2h_rgb2yuv_ctrl_reg.regValue);

}

void drvif_color_ultrazoom_set_dc2h_scale_down(DC2H_HANDLER *pdc2h_hdl)
{
	unsigned int SDRatio;
	unsigned int SDFilter=0;
	unsigned int tmp_data;
	short *coef_pt;
	unsigned int i;
	unsigned long long tmpLength = 0;
	unsigned int nFactor;
	unsigned int D = 0;
	unsigned char Hini, Vini, a;
	unsigned short S1,S2;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	dc2h_scaledown_dc2h_hsd_ctrl0_RBUS        dc2h_hsd_ctrl0_reg;
	dc2h_scaledown_dc2h_vsd_ctrl0_RBUS                      dc2h_vsd_ctrl0_reg;
	dc2h_scaledown_dc2h_hsd_scale_hor_factor_RBUS  dc2h_hsd_scale_hor_factor_reg;
	dc2h_scaledown_dc2h_vsd_scale_ver_factor_RBUS  dc2h_vsd_scale_ver_factor_reg;
	dc2h_scaledown_dc2h_hsd_hor_segment_RBUS    dc2h_hsd_hor_segment_reg;
	//dc2h_scaledown_dc2h_hsd_hor_delta1_RBUS      dc2h_hsd_hor_delta1_reg;
	dc2h_scaledown_dc2h_hsd_initial_value_RBUS      dc2h_hsd_initial_value_reg;
	dc2h_scaledown_dc2h_vsd_initial_value_RBUS      dc2h_vsd_initial_value_reg;

	dc2h_hsd_ctrl0_reg.regValue    = rtd_inl(DC2H_SCALEDOWN_DC2H_HSD_Ctrl0_reg);
	dc2h_vsd_ctrl0_reg.regValue    = rtd_inl(DC2H_SCALEDOWN_DC2H_VSD_Ctrl0_reg);
	dc2h_hsd_scale_hor_factor_reg.regValue  = rtd_inl(DC2H_SCALEDOWN_DC2H_HSD_Scale_Hor_Factor_reg);
	dc2h_vsd_scale_ver_factor_reg.regValue  = rtd_inl(DC2H_SCALEDOWN_DC2H_VSD_Scale_Ver_Factor_reg);
	dc2h_hsd_hor_segment_reg.regValue = rtd_inl(DC2H_SCALEDOWN_DC2H_HSD_Hor_Segment_reg);
	//dc2h_hsd_hor_delta1_reg.regValue  = rtd_inl(DC2H_SCALEDOWN_DC2H_HSD_Hor_Delta1_reg);
	dc2h_hsd_initial_value_reg.regValue  = rtd_inl(DC2H_SCALEDOWN_DC2H_HSD_Initial_Value_reg);
	dc2h_vsd_initial_value_reg.regValue  = rtd_inl(DC2H_SCALEDOWN_DC2H_VSD_Initial_Value_reg);

	#define TMPMUL  (16)

	//o============ H scale-down=============o
	if (pdc2h_hdl->in_size.nWidth > pdc2h_hdl->out_size.nWidth)
	{
		// o-------calculate scaledown ratio to select one of different bandwith filters.--------o
		if ( pdc2h_hdl->out_size.nWidth == 0 ) {
		rtd_pr_vt_debug("output width = 0 !!!\n");
		SDRatio = 0;
		} else {
		SDRatio = (pdc2h_hdl->in_size.nWidth*TMPMUL) / pdc2h_hdl->out_size.nWidth;
		}

		//UltraZoom_Printf("CSW SDRatio number=%d\n",SDRatio);

		if(SDRatio <= ((TMPMUL*3)/2))
			SDFilter = 2;
		else if(SDRatio <= (TMPMUL*2) && SDRatio > ((TMPMUL*3)/2) )  // Mid
			SDFilter = 1;
		else    // blurest, narrow bw
			SDFilter = 0;

		//o---------------- fill coefficients into access port--------------------o
		coef_pt = tScaleDown_COEF_TAB[SDFilter];  

		dc2h_hsd_ctrl0_reg.h_y_table_sel = 0;  // TAB1
		dc2h_hsd_ctrl0_reg.h_c_table_sel = 0;  // TAB1

		for (i=0; i<16; i++)
		{
			tmp_data = ((unsigned int)(*coef_pt++)<<16);
			tmp_data += (unsigned int)(*coef_pt++); 
			rtd_outl(DC2H_SCALEDOWN_DC2H_UZD_FIR_Coef_Tab1_C0_reg + i*4, tmp_data);
		}
	}

	//o============ V scale-down=============o
	if (pdc2h_hdl->in_size.nLength > pdc2h_hdl->out_size.nLength)
	{
		// o-------calculate scaledown ratio to select one of different bandwith filters.--------o
		//jeffrey 961231
		if ( pdc2h_hdl->out_size.nLength == 0 ) {
		SDRatio = 0;
		} else {
		SDRatio = (pdc2h_hdl->in_size.nLength*TMPMUL) /pdc2h_hdl->out_size.nLength;
		}

		//UltraZoom_Printf("CSW SDRatio number=%d\n",SDRatio);

		if(SDRatio <= ((TMPMUL*3)/2)) 
			SDFilter = 2;
		else if(SDRatio <= (TMPMUL*2) && SDRatio > ((TMPMUL*3)/2) )  // Mid
			SDFilter = 1;
		else    // blurest, narrow bw
			SDFilter = 0;

		rtd_pr_vt_debug("filter number=%d\n",SDFilter);

		//o---------------- fill coefficients into access port--------------------o
		 coef_pt = tScaleDown_COEF_TAB[SDFilter]; 
		//coef_pt = &(ScaleDown_COEF_TAB.FIR_Coef_Table[SD_V_Coeff_Sel[SDFilter]][0]);

		dc2h_vsd_ctrl0_reg.v_y_table_sel = 1;  // TAB2
		dc2h_vsd_ctrl0_reg.v_c_table_sel = 1;  // TAB2

		for (i=0; i<16; i++)
		{
			tmp_data = ((unsigned int)(*coef_pt++)<<16);
			tmp_data += (unsigned int)(*coef_pt++);
			rtd_outl(DC2H_SCALEDOWN_DC2H_UZD_FIR_Coef_Tab2_C0_reg + i*4, tmp_data);
		}
	} else {
		//no need  V scale-down, use bypass filter
	}

	// Decide Output data format for scale down
	//============H Scale-down============
	if (pdc2h_hdl->in_size.nWidth > pdc2h_hdl->out_size.nWidth)
	{    // H scale-down
		Hini = 0x80;//0xff;//0x78;
		dc2h_hsd_initial_value_reg.hor_ini = Hini;
		dc2h_hsd_initial_value_reg.hor_ini_int= 1;
		a = 5;

		if(pdc2h_hdl->panorama)
		{// CSW+ 0960830 Non-linear SD
			S1 = (pdc2h_hdl->out_size.nWidth) >> 2;
			S2 = (pdc2h_hdl->out_size.nWidth) - S1*2;
			//=vv==old from TONY, if no problem, use this=vv==
			nFactor = (unsigned int)(((pdc2h_hdl->in_size.nWidth)<<21) - ((unsigned int)Hini<<13));
			//=vv==Thur debugged=vv==
			//nFactor = ((((unsigned long long)(ptInSize->nWidth))*(ptOutSize->nWidth-1))<<21) / ptOutSize->nWidth;
			//===================
			nFactor = nFactor /((2*a+1)*S1 + (a+1)*S2 - a) * a;
			if(S1 != 0)
				D = nFactor / a / S1;
			nFactor = SHR((nFactor + 1), 1); //rounding
			D = SHR(D + 1, 1); //rounding
			rtd_pr_vt_debug("\nPANORAMA2 TEST 111\n");
			rtd_pr_vt_debug("nPANORAMA2 Factor = %d\n",nFactor);
			rtd_pr_vt_debug("PANORAMA2 S1=%d, S2=%d, D = %d\n",S1, S2, D);

			if(nFactor < 1048576) {
				rtd_pr_vt_debug("PANORAMA2 Can't do nonlinear SD \n");
				//nFactor = (unsigned int)(((ptInSize->nWidth)<<21) - ((unsigned int)Hini<<13)) / (ptOutSize->nWidth-1);
				if(pdc2h_hdl->out_size.nWidth != 0)
					nFactor = (unsigned int)(((pdc2h_hdl->in_size.nWidth)<<21)) / (pdc2h_hdl->out_size.nWidth);
				nFactor = SHR(nFactor + 1, 1); //rounding
				rtd_pr_vt_debug("PANORAMA2 nFactor=%x, input Wid=%d, Out Wid=%d\n",nFactor, pdc2h_hdl->in_size.nWidth, pdc2h_hdl->out_size.nWidth);
				S1 = 0;
				S2 = pdc2h_hdl->out_size.nWidth;
				D = 0;
			}
        	}else{
			if ( pdc2h_hdl->out_size.nWidth == 0 ) {
				rtd_pr_vt_debug("output width = 0 !!!\n");
				nFactor = 0;
			}else if(pdc2h_hdl->in_size.nWidth>4095){
				//nFactor = (unsigned int)((((pdc2h_hdl->in_size.nWidth-1)<<19) / (pdc2h_hdl->out_size.nWidth-1))<<1);
				nFactor = (unsigned int)((((pdc2h_hdl->in_size.nWidth)<<19) / (pdc2h_hdl->out_size.nWidth))<<1);
			}else if(pdc2h_hdl->in_size.nWidth>2047){
				//nFactor = (unsigned int)(((pdc2h_hdl->in_size.nWidth-1)<<20) / (pdc2h_hdl->out_size.nWidth-1));
				nFactor = (unsigned int)(((pdc2h_hdl->in_size.nWidth)<<20) / (pdc2h_hdl->out_size.nWidth));
			}else {
				//nFactor = (unsigned int)(((pdc2h_hdl->in_size.nWidth-1)<<21) / (pdc2h_hdl->out_size.nWidth-1));
				nFactor = (unsigned int)(((pdc2h_hdl->in_size.nWidth)<<21) / (pdc2h_hdl->out_size.nWidth));
				nFactor = SHR(nFactor + 1, 1); //rounding
			}

			rtd_pr_vt_debug("nFactor=%d, input Wid=%d, Out Wid=%d\n",nFactor, pdc2h_hdl->in_size.nWidth, pdc2h_hdl->out_size.nWidth);
			S1 = 0;
			S2 = pdc2h_hdl->out_size.nWidth;
			D = 0;
		}
	}else {
		nFactor = 0x100000;
		S1 = 0;
		S2 = pdc2h_hdl->out_size.nWidth;
		D = 0;
	}

	dc2h_hsd_scale_hor_factor_reg.hor_fac = nFactor;
	//dc2h_hsd_hor_segment_reg.nl_seg1 = S1;
	dc2h_hsd_hor_segment_reg.nl_seg2 = S2;
	//dc2h_hsd_hor_delta1_reg.nl_d1 = D;
	//UltraZoom_Printf("reg_seg1=%x, reg_Seg_all=%x\n",dc2h_hsd_hor_segment_reg.nl_seg1, dc2h_hsd_hor_segment_reg.regValue);
	rtd_pr_vt_debug("nFactor=%x, input_Wid=%d, Output_Wid=%d\n",nFactor, pdc2h_hdl->in_size.nWidth, pdc2h_hdl->out_size.nWidth);

	//================V Scale-down=================
	if (pdc2h_hdl->in_size.nLength > pdc2h_hdl->out_size.nLength) {    // V scale-down
		Vini = 0x80;// actual 0x180;
		dc2h_vsd_initial_value_reg.ver_ini = Vini;
		// ???_reg.ver_ini_int = 1;
		if ( pdc2h_hdl->out_size.nLength == 0 ) {
			rtd_pr_vt_debug("output length = 0 !!!\n");
			nFactor = 0;
		} else {
			//frank@1018 sync with pacific to solve 3d vide 1920X2205 scaler down fail
			//nFactor = (unsigned int)(((ptInSize->nLength)<<21) / (ptOutSize->nLength));
			//nFactor = (unsigned int)(((unsigned long long)(pdc2h_hdl->in_size.nLength)<<21) / (pdc2h_hdl->out_size.nLength));
			tmpLength = ((unsigned long long)(pdc2h_hdl->in_size.nLength)<<21);
			do_div(tmpLength, (pdc2h_hdl->out_size.nLength));
			nFactor = tmpLength;
		}
		rtd_pr_vt_debug("Ch1 Ver: CAP=%d, Disp=%d, factor=%d\n", pdc2h_hdl->in_size.nLength, pdc2h_hdl->out_size.nLength, nFactor);
		nFactor = SHR(nFactor + 1, 1); //rounding
	} else {
		nFactor = 0x100000;
	}

	dc2h_vsd_scale_ver_factor_reg.ver_fac = nFactor;

	rtd_outl(DC2H_SCALEDOWN_DC2H_HSD_Ctrl0_reg, dc2h_hsd_ctrl0_reg.regValue);
	rtd_outl(DC2H_SCALEDOWN_DC2H_VSD_Ctrl0_reg, dc2h_vsd_ctrl0_reg.regValue);
	rtd_outl(DC2H_SCALEDOWN_DC2H_HSD_Scale_Hor_Factor_reg, dc2h_hsd_scale_hor_factor_reg.regValue);
	rtd_outl(DC2H_SCALEDOWN_DC2H_VSD_Scale_Ver_Factor_reg, dc2h_vsd_scale_ver_factor_reg.regValue);
	rtd_outl(DC2H_SCALEDOWN_DC2H_HSD_Hor_Segment_reg, dc2h_hsd_hor_segment_reg.regValue);
	//rtd_outl(DC2H_SCALEDOWN_DC2H_HSD_Hor_Delta1_reg, dc2h_hsd_hor_delta1_reg.regValue);
	rtd_outl(DC2H_SCALEDOWN_DC2H_HSD_Initial_Value_reg, dc2h_hsd_initial_value_reg.regValue);
	rtd_outl(DC2H_SCALEDOWN_DC2H_VSD_Initial_Value_reg, dc2h_vsd_initial_value_reg.regValue);
}


void drvif_color_ultrazoom_config_dc2h_scaling_down(DC2H_HANDLER *pdc2h_hdl)
{
	// Scale down setup for Channel1
	dc2h_scaledown_dc2h_hsd_ctrl0_RBUS dc2h_hsd_ctrl0_reg;
	dc2h_scaledown_dc2h_vsd_ctrl0_RBUS dc2h_vsd_ctrl0_reg;
	dc2h_scaledown_dc2h_444to422_ctrl_RBUS dc2h_scaledown_dc2h_444to422_ctrl_reg;
	
	dc2h_scaledown_dc2h_444to422_ctrl_reg.regValue = rtd_inl(DC2H_SCALEDOWN_DC2H_444to422_CTRL_reg);
	//if(pdc2h_hdl->dc2h_in_sel==_MAIN_UZU)/*mux uzu*/
		//dc2h_scaledown_dc2h_444to422_ctrl_reg.cbcr_swap_en =1;
	//else
		dc2h_scaledown_dc2h_444to422_ctrl_reg.cbcr_swap_en =0;
	rtd_outl(DC2H_SCALEDOWN_DC2H_444to422_CTRL_reg, dc2h_scaledown_dc2h_444to422_ctrl_reg.regValue);  //used record freeze buffer
	

	dc2h_hsd_ctrl0_reg.regValue = rtd_inl(DC2H_SCALEDOWN_DC2H_HSD_Ctrl0_reg);
	dc2h_hsd_ctrl0_reg.h_zoom_en = (pdc2h_hdl->in_size.nWidth > pdc2h_hdl->out_size.nWidth);
	rtd_outl(DC2H_SCALEDOWN_DC2H_HSD_Ctrl0_reg, dc2h_hsd_ctrl0_reg.regValue);

	dc2h_vsd_ctrl0_reg.regValue = rtd_inl(DC2H_SCALEDOWN_DC2H_VSD_Ctrl0_reg);
	if(pdc2h_hdl->in_size.nLength > pdc2h_hdl->out_size.nLength){
		dc2h_vsd_ctrl0_reg.v_zoom_en = 1;
		dc2h_vsd_ctrl0_reg.buffer_mode = 2;
	}else{
		dc2h_vsd_ctrl0_reg.v_zoom_en = 0;
		dc2h_vsd_ctrl0_reg.buffer_mode = 0;
	}
	if((get_VT_Pixel_Format() == VT_CAP_NV12)||(get_VT_Pixel_Format() == VT_CAP_NV16)){
		dc2h_vsd_ctrl0_reg.sort_fmt = 0;
	} else {
		dc2h_vsd_ctrl0_reg.sort_fmt = 1;
	}
	rtd_outl(DC2H_SCALEDOWN_DC2H_VSD_Ctrl0_reg, dc2h_vsd_ctrl0_reg.regValue);

	drvif_color_ultrazoom_set_dc2h_scale_down(pdc2h_hdl);
}

void drvif_dc2h_input_overscan_config(unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
	dc2h_rgb2yuv_dc2h_3dmaskto2d_ctrl_RBUS dc2h_rgb2yuv_dc2h_3dmaskto2d_ctrl_Reg;
	dc2h_rgb2yuv_dc2h_overscan_ctrl0_RBUS dc2h_rgb2yuv_dc2h_overscan_ctrl0_Reg;
	dc2h_rgb2yuv_dc2h_3dmaskto2d_ctrl1_RBUS dc2h_rgb2yuv_dc2h_3dmaskto2d_ctrl1_Reg;

	dc2h_rgb2yuv_dc2h_3dmaskto2d_ctrl_Reg.regValue = rtd_inl(DC2H_RGB2YUV_DC2H_3DMaskTo2D_Ctrl_reg);
	dc2h_rgb2yuv_dc2h_overscan_ctrl0_Reg.regValue = rtd_inl(DC2H_RGB2YUV_DC2H_OVERSCAN_Ctrl0_reg);
	dc2h_rgb2yuv_dc2h_3dmaskto2d_ctrl1_Reg.regValue = rtd_inl(DC2H_RGB2YUV_DC2H_3DMaskTo2D_Ctrl1_reg);

	dc2h_rgb2yuv_dc2h_overscan_ctrl0_Reg.dc2h_overscan_left = x+1;
	dc2h_rgb2yuv_dc2h_overscan_ctrl0_Reg.dc2h_overscan_right = x+w;
	rtd_outl(DC2H_RGB2YUV_DC2H_OVERSCAN_Ctrl0_reg, dc2h_rgb2yuv_dc2h_overscan_ctrl0_Reg.regValue);

	dc2h_rgb2yuv_dc2h_3dmaskto2d_ctrl1_Reg.dc2h_overscan_top = y;
	dc2h_rgb2yuv_dc2h_3dmaskto2d_ctrl1_Reg.dc2h_overscan_bottom = y+h-1;
	rtd_outl(DC2H_RGB2YUV_DC2H_3DMaskTo2D_Ctrl1_reg, dc2h_rgb2yuv_dc2h_3dmaskto2d_ctrl1_Reg.regValue);

	dc2h_rgb2yuv_dc2h_3dmaskto2d_ctrl_Reg.dc2h_3dmaskto2d_3dformat = 4;/*overscan*/
	dc2h_rgb2yuv_dc2h_3dmaskto2d_ctrl_Reg.dc2h_3dmaskto2d_en = 1;
	rtd_outl(DC2H_RGB2YUV_DC2H_3DMaskTo2D_Ctrl_reg, dc2h_rgb2yuv_dc2h_3dmaskto2d_ctrl_Reg.regValue);
	
}

void drvif_vi_config(unsigned int nWidth, unsigned int nLength, unsigned int data_format)
{
	unsigned int LineSize;
//	unsigned int width96align;
    dc2h_vi_dc2h_v1_output_fmt_RBUS dc2h_v1_output_fmt_reg;//DC2H_RGB2YUV_DC2H_V1_OUTPUT_FMT_VADDR
    dc2h_vi_dc2h_cwvh1_RBUS dc2h_cwvh1_reg;//w,h //DC2H_RGB2YUV_DC2H_CWVH1_VADDR
    dc2h_vi_dc2h_cwvh1_2_RBUS dc2h_cwvh1_2_reg;
	dc2h_vi_dc2h_dmactl_RBUS vi_dmactl_reg;//dmaen	//I3DDMA_VI_IDMA_DMACTL_VADDR
	dc2h_vi_dc2h_vi_addcmd_transform_RBUS dc2h_vi_dc2h_vi_addcmd_transform_reg;
	dc2h_dma_dc2h_seq_interrupt_enable_RBUS dc2h_dma_dc2h_seq_interrupt_enable_Reg;
	dc2h_vi_dc2h_y1buf_RBUS dc2h_vi_dc2h_y1buf_Reg;
	dc2h_vi_dc2h_c1buf_RBUS dc2h_vi_dc2h_c1buf_Reg;
	dc2h_vi_dc2h_vi_c_line_step_RBUS dc2h_vi_dc2h_vi_c_line_step_Reg;
	
    dc2h_v1_output_fmt_reg.regValue = rtd_inl(DC2H_VI_DC2H_V1_OUTPUT_FMT_reg);
	dc2h_v1_output_fmt_reg.data_format = data_format;/*0:420	1:422*/
	dc2h_v1_output_fmt_reg.swap_1byte = 0;
	dc2h_v1_output_fmt_reg.swap_2byte = 0;
	dc2h_v1_output_fmt_reg.swap_4byte = 0;
	dc2h_v1_output_fmt_reg.swap_8byte = 0;
    rtd_outl(DC2H_VI_DC2H_V1_OUTPUT_FMT_reg, dc2h_v1_output_fmt_reg.regValue);

    dc2h_cwvh1_reg.regValue = rtd_inl(DC2H_VI_DC2H_CWVH1_reg);
    dc2h_cwvh1_reg.hsize = nWidth;
    dc2h_cwvh1_reg.vsize = nLength;
    rtd_outl(DC2H_VI_DC2H_CWVH1_reg, dc2h_cwvh1_reg.regValue);

   	dc2h_cwvh1_2_reg.regValue = rtd_inl(DC2H_VI_DC2H_CWVH1_2_reg);
    dc2h_cwvh1_2_reg.vsize_c = nLength >> (data_format == 1? 0: 1);
    rtd_outl(DC2H_VI_DC2H_CWVH1_2_reg, dc2h_cwvh1_2_reg.regValue);

	  /*width96align = drvif_memory_get_data_align(nWidth, 96);
	  LineSize = drvif_memory_get_data_align(width96align*8, 64);
      LineSize = (unsigned int)SHR(LineSize, 6);      // (pixels of per line * 8 / 32), unit: 32bits */
	  
	LineSize = nWidth * 8 / 64;
	if(LineSize % 2)
		LineSize += 1;
	
	dc2h_vi_dc2h_vi_c_line_step_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_c_line_step_reg);
	dc2h_vi_dc2h_vi_c_line_step_Reg.c_line_step = LineSize;
	rtd_outl(DC2H_VI_DC2H_vi_c_line_step_reg, dc2h_vi_dc2h_vi_c_line_step_Reg.regValue);

	dc2h_vi_dc2h_vi_addcmd_transform_reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_addcmd_transform_reg);
	dc2h_vi_dc2h_vi_addcmd_transform_reg.line_step = LineSize;
	dc2h_vi_dc2h_vi_addcmd_transform_reg.vi_addcmd_trans_en = 1;
	rtd_outl(DC2H_VI_DC2H_vi_addcmd_transform_reg, dc2h_vi_dc2h_vi_addcmd_transform_reg.regValue);

	if (get_vt_VtSwBufferMode()) {
		dc2h_dma_dc2h_seq_interrupt_enable_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_seq_Interrupt_Enable_reg);
		dc2h_dma_dc2h_seq_interrupt_enable_Reg.vs_rising_ie = 1;
	}		

	dc2h_wait_porch();	
	
	vi_dmactl_reg.regValue = rtd_inl(DC2H_VI_DC2H_DMACTL_reg);
	vi_dmactl_reg.dmaen1 = 1;
	vi_dmactl_reg.seq_blk_sel =1; //0: YV16 or RGB888 1: NV12 or NV16
	vi_dmactl_reg.bstlen = 7;//merlin4 run block mode 96B align
	rtd_outl(DC2H_VI_DC2H_DMACTL_reg, vi_dmactl_reg.regValue);

	dc2h_vi_dc2h_y1buf_Reg.regValue = rtd_inl(DC2H_VI_DC2H_Y1BUF_reg);
	dc2h_vi_dc2h_y1buf_Reg.y_water_lvl = 120; //burst write 120x64bits
	rtd_outl(DC2H_VI_DC2H_Y1BUF_reg, dc2h_vi_dc2h_y1buf_Reg.regValue);

	dc2h_vi_dc2h_c1buf_Reg.regValue = rtd_inl(DC2H_VI_DC2H_C1BUF_reg);
	dc2h_vi_dc2h_c1buf_Reg.c_water_lvl = 120; //burst write 120x64bits
	rtd_outl(DC2H_VI_DC2H_C1BUF_reg, dc2h_vi_dc2h_c1buf_Reg.regValue);


	if (get_vt_VtSwBufferMode()) {
		rtd_outl(DC2H_DMA_dc2h_seq_Interrupt_Enable_reg, dc2h_dma_dc2h_seq_interrupt_enable_Reg.regValue);
	}	
}

void drvif_dc2h_seq_config(DC2H_HANDLER *pdc2h_hdl)
{
	unsigned int num, rem;
	unsigned int lineunits, len;
	unsigned int width96align;
	dc2h_dma_dc2h_seq_mode_ctrl1_RBUS dc2h_seq_mode_ctrl1_reg;
	dc2h_dma_dc2h_seq_mode_ctrl2_RBUS dc2h_seq_mode_ctrl2_reg;
	dc2h_dma_dc2h_seq_interrupt_enable_RBUS dc2h_dma_dc2h_seq_interrupt_enable_Reg;
	dc2h_dma_dc2h_cap_ctl0_RBUS dc2h_cap_ctl0_reg;
	dc2h_dma_dc2h_cap_ctl1_RBUS dc2h_cap_ctl1_reg;
	//dc2h_cap_l1_start_RBUS dc2h_cap_l1_start_reg;
	//dc2h_cap_l2_start_RBUS dc2h_cap_l2_start_reg;
	//dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_reg;
	dc2h_dma_dc2h_seq_byte_channel_swap_RBUS dc2h_seq_byte_channel_swap_reg;

	dc2h_vi_dc2h_dmactl_RBUS vi_dmactl_reg;//dmaen
	vi_dmactl_reg.regValue = rtd_inl(DC2H_VI_DC2H_DMACTL_reg);
	//vi_dmactl_reg.dmaen1 = 1;
	vi_dmactl_reg.seq_blk_sel =0;
	rtd_outl(DC2H_VI_DC2H_DMACTL_reg, vi_dmactl_reg.regValue);

	dc2h_seq_mode_ctrl1_reg.regValue = rtd_inl(DC2H_DMA_dc2h_Seq_mode_CTRL1_reg);
	dc2h_seq_mode_ctrl1_reg.cap_en = 0;
	rtd_outl(DC2H_DMA_dc2h_Seq_mode_CTRL1_reg, dc2h_seq_mode_ctrl1_reg.regValue);

	if (pdc2h_hdl->dc2h_en == FALSE)
		return;

	dc2h_seq_mode_ctrl2_reg.regValue = rtd_inl(DC2H_DMA_dc2h_Seq_mode_CTRL2_reg);
	dc2h_seq_mode_ctrl2_reg.hact = pdc2h_hdl->out_size.nWidth;
	dc2h_seq_mode_ctrl2_reg.vact = pdc2h_hdl->out_size.nLength;
	rtd_outl(DC2H_DMA_dc2h_Seq_mode_CTRL2_reg, dc2h_seq_mode_ctrl2_reg.regValue);

	//num = (pdc2h_hdl->out_size.nWidth - CAP_REM) / CAP_LEN;
	//rem = ((pdc2h_hdl->out_size.nWidth - CAP_REM) % CAP_LEN) + CAP_REM;

	if(get_VT_Pixel_Format() == VT_CAP_ARGB8888) {
		width96align = drvif_memory_get_data_align(pdc2h_hdl->out_size.nWidth*4, 96);
	} else {
		width96align = drvif_memory_get_data_align(pdc2h_hdl->out_size.nWidth*3, 96);		
	}
	lineunits = (width96align * 8) /128;

	len = 32;
	num = (unsigned int)(lineunits / len);
	rem = (unsigned int)(lineunits % len);

	if (rem == 0) { // remainder is not allowed to be zero
		rem = len;
		num -= 1;
	}

	dc2h_cap_ctl0_reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_CTL0_reg);
	dc2h_cap_ctl0_reg.bit_sel = 0;
	dc2h_cap_ctl0_reg.pixel_encoding = 0;
	//dc2h_cap_ctl0_reg.triple_buf_en = 1;
	if(get_vt_VtBufferNum() == 1) {
		dc2h_cap_ctl0_reg.triple_buf_en = 0;
		dc2h_cap_ctl0_reg.auto_block_sel_en = 0;
		//dc2h_cap_ctl0_reg.cap_block_fw = 0;
	} else {
		if(get_vt_VtCaptureVDC()) {
			dc2h_cap_ctl0_reg.triple_buf_en = 1;
			dc2h_cap_ctl0_reg.auto_block_sel_en = 1;
		} else {
			if (get_vt_VtSwBufferMode()) {
				dc2h_cap_ctl0_reg.triple_buf_en = 0;
				dc2h_cap_ctl0_reg.auto_block_sel_en = 0;
			} else {
				dc2h_cap_ctl0_reg.triple_buf_en = 0;
				dc2h_cap_ctl0_reg.auto_block_sel_en = 1;
			}
		}
	}
	dc2h_cap_ctl0_reg.cap_write_len = CAP_LEN;
	dc2h_cap_ctl0_reg.cap_write_rem = rem;
	rtd_outl(DC2H_DMA_dc2h_Cap_CTL0_reg, dc2h_cap_ctl0_reg.regValue);

	dc2h_cap_ctl1_reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_CTL1_reg);
	dc2h_cap_ctl1_reg.cap_water_lv =CAP_LEN ;
	dc2h_cap_ctl1_reg.cap_write_num = num;
	rtd_outl(DC2H_DMA_dc2h_Cap_CTL1_reg, dc2h_cap_ctl1_reg.regValue);

	//dc2h_seq_mode_ctrl1_reg.regValue = rtd_inl(DC2H_DMA_dc2h_Seq_mode_CTRL1_reg);
	//dc2h_seq_mode_ctrl1_reg.cap_en = 1;
	//rtd_outl(DC2H_DMA_dc2h_Seq_mode_CTRL1_reg, dc2h_seq_mode_ctrl1_reg.regValue);

	dc2h_seq_byte_channel_swap_reg.regValue = rtd_inl(DC2H_DMA_dc2h_seq_byte_channel_swap_reg);
	dc2h_seq_byte_channel_swap_reg.dma_1byte_swap = 1;
	dc2h_seq_byte_channel_swap_reg.dma_2byte_swap = 1;
	dc2h_seq_byte_channel_swap_reg.dma_4byte_swap = 1;
	dc2h_seq_byte_channel_swap_reg.dma_8byte_swap = 1;
	if(get_VT_Pixel_Format() == VT_CAP_ARGB8888)
		dc2h_seq_byte_channel_swap_reg.dma_channel_swap = 0;
	else
		dc2h_seq_byte_channel_swap_reg.dma_channel_swap = 1;
	rtd_outl(DC2H_DMA_dc2h_seq_byte_channel_swap_reg, dc2h_seq_byte_channel_swap_reg.regValue);

	dc2h_seq_mode_ctrl1_reg.regValue = rtd_inl(DC2H_DMA_dc2h_Seq_mode_CTRL1_reg);
	dc2h_seq_mode_ctrl1_reg.cap_en = 1;

	if((get_VT_Pixel_Format() == VT_CAP_ABGR8888)||(get_VT_Pixel_Format() == VT_CAP_ARGB8888)){
		dc2h_seq_mode_ctrl1_reg.argb_dummy_data = 0xFF;
		dc2h_seq_mode_ctrl1_reg.rgb_to_argb_en = 1;
		dc2h_seq_mode_ctrl1_reg.argb_swap = 1;
	}
	else
		dc2h_seq_mode_ctrl1_reg.rgb_to_argb_en = 0;

	if (get_vt_VtSwBufferMode()) {
		dc2h_dma_dc2h_seq_interrupt_enable_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_seq_Interrupt_Enable_reg);
		dc2h_dma_dc2h_seq_interrupt_enable_Reg.vs_rising_ie = 1;
	}
	dc2h_wait_porch();
	rtd_outl(DC2H_DMA_dc2h_Seq_mode_CTRL1_reg, dc2h_seq_mode_ctrl1_reg.regValue);
	if (get_vt_VtSwBufferMode()) {
		rtd_outl(DC2H_DMA_dc2h_seq_Interrupt_Enable_reg, dc2h_dma_dc2h_seq_interrupt_enable_Reg.regValue);
	}
	return;
}

unsigned char vt_source_capture_use_SE(KADP_VT_DUMP_LOCATION_TYPE_T dumplocation)
{
	/* if video has done overscan in scaler output 
	 * 
	 * source type:
	 * DTV/CP/Playback ==> vdec NN buffer ==> SE stretch
	 * HDMI/AV/ATV/Component ==> i3ddma NN buffer ==> SE stretch
	 *
	*/
	VSC_INPUT_TYPE_T srctype = VSC_INPUTSRC_MAXN;
	
	    /* first: stop dc2h */
	if(get_dc2h_capture_state() == TRUE)
	{		
		start_stop_dc2h(FALSE); // stop dc2h
	}
	
	if((dumplocation == KADP_VT_SCALER_OUTPUT) && (get_video_do_overscan_state()))
	{
	/*if dumplocation=scaler output and video path has do overscan,
		so for geting no overscan data,will use SE capture from i3ddma or vdec */
		//capSrc = KADP_VT_SCALER_INPUT;
		rtd_pr_vt_notice("i3ddma/vdec NN buffer to SE flow\n");
		srctype = Get_DisplayMode_Src(SLR_MAIN_DISPLAY);
		rtd_pr_vt_notice("source type:%d\n",srctype);
		
		if((get_VT_Pixel_Format() == VT_CAP_NV12) || (get_VT_Pixel_Format() == VT_CAP_NV16))
		{
						
			if(((srctype == VSC_INPUTSRC_HDMI) && (get_hdmi_4k_hfr_mode() == HDMI_NON_4K120))
			|| (srctype == VSC_INPUTSRC_ADC) || (srctype == VSC_INPUTSRC_AVD))
			{
				/* HDMI/AV/ATV/Component ==> i3ddma NN buffer ==> SE stretch */
				if(drvif_i3ddma_NN_SE_proc() == FALSE)
				{
					rtd_pr_vt_notice("drvif_i3ddma_NN_SE_proc result fail\n");
					return FALSE;
				}
				
				return TRUE;
			}
			else if((srctype == VSC_INPUTSRC_VDEC) || (srctype == VSC_INPUTSRC_JPEG))
			{
				/* DTV/CP/Playback ==> vdec NN buffer ==> SE stretch  */
				if(drvif_vdec_NN_SE_proc() == FALSE)
				{
					rtd_pr_vt_notice("drvif_vdec_NN_SE_proc result fail\n");
					return FALSE;
				}

				return TRUE;
			}
			else
			{
				rtd_pr_vt_notice("unsupport source type capture\n");
				return FALSE;
			}

		}
	
	}

	return FALSE;
}

void drvif_DC2H_dispD_CaptureConfig(VT_CUR_CAPTURE_INFO capInfo)
{
	unsigned char waitbufferms = 80;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
	struct timespec64 timeout_s;
#else
	struct timespec timeout_s;
#endif	
#if 0//trash debug
	if((capInfo.enable==1)){
		rtd_pr_vt_emerg("%s=%d memset 000000000000000000000!!\n", __FUNCTION__, __LINE__);	
		memset((unsigned char *)VtAllocatedBufferStartAdress, 0 , (VT_CAP_FRAME_MAX_WIDTH*VT_CAP_FRAME_MAX_LENGTH+VT_CAP_FRAME_MAX_WIDTH*VT_CAP_FRAME_MAX_LENGTH+_ALIGN(VT_CAP_FRAME_MAX_WIDTH*VT_CAP_FRAME_MAX_LENGTH,__12KPAGE)));
	}
#endif
	if(capInfo.enable == TRUE){
		if (CaptureCtrl_VT.cap_buffer[0].phyaddr == 0) {
			rtd_pr_vt_emerg("[Vt warning]vt no init, so no memory, return setting;%s=%d \n", __FUNCTION__, __LINE__);
			return;
		}

		if ((capInfo.capWid > vt_cap_frame_max_width)||(capInfo.capLen > vt_cap_frame_max_height)) {
			rtd_pr_vt_emerg("[VT warning]drvif_DC2H_dispD_CaptureConfig:could not support max than 2k1k output w=%d,output h=%d! \n", capInfo.capWid,capInfo.capLen);
			return;
		}	

		if((get_VT_Pixel_Format() == VT_CAP_NV12)||(get_VT_Pixel_Format() == VT_CAP_NV16))
		{		
			dc2h_vi_dc2h_vi_ads_start_y_RBUS DC2H_VI_DC2H_vi_ads_start_y_Reg;
			dc2h_vi_dc2h_vi_ads_start_c_RBUS dc2h_vi_dc2h_vi_ads_start_c_Reg;
			dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_reg;
		
			dc2h_dma_dc2h_cap_boundaryaddr1_RBUS dc2h_cap_boundaryaddr1_reg;
			dc2h_dma_dc2h_cap_boundaryaddr2_RBUS dc2h_cap_boundaryaddr2_reg;
			unsigned int vt_use_buffer_num = 0;

			vt_use_buffer_num = get_vt_VtBufferNum();
			dc2h_cap_l3_start_reg.regValue= 0;
			rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg, dc2h_cap_l3_start_reg.regValue);  //used record freeze buffer

			DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue = (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;
			IoReg_Write32(DC2H_VI_DC2H_vi_ads_start_y_reg, DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue);
			
			dc2h_vi_dc2h_vi_ads_start_c_Reg.regValue = (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr + _ALIGN(vt_cap_frame_max_width*vt_cap_frame_max_height,__12KPAGE);
			IoReg_Write32(DC2H_VI_DC2H_vi_ads_start_c_reg, dc2h_vi_dc2h_vi_ads_start_c_Reg.regValue);
			//rtd_pr_vt_emerg("nv12 or nv16 %s=%d \n", __FUNCTION__, __LINE__);

#ifndef CONFIG_MEMC_BYPASS	//no-bypass memc 

			if((vt_use_buffer_num == 5))
			{
				unsigned int up = sort_boundary_addr_max_index();
				unsigned int low =  sort_boundary_addr_min_index();
				
				dc2h_cap_boundaryaddr1_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[up].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[up].size;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr1_reg, dc2h_cap_boundaryaddr1_reg.regValue);

				dc2h_cap_boundaryaddr2_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[low].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr2_reg, dc2h_cap_boundaryaddr2_reg.regValue);
			}
			else
			{
				
				dc2h_cap_boundaryaddr1_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[vt_use_buffer_num-1].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[vt_use_buffer_num-1].size;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr1_reg, dc2h_cap_boundaryaddr1_reg.regValue);
				
				dc2h_cap_boundaryaddr2_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr2_reg, dc2h_cap_boundaryaddr2_reg.regValue);
			}
#else	// bypass memc	
		
			dc2h_cap_boundaryaddr1_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[vt_use_buffer_num-1].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[vt_use_buffer_num-1].size;
			rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr1_reg, dc2h_cap_boundaryaddr1_reg.regValue);
			
			dc2h_cap_boundaryaddr2_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;
			rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr2_reg, dc2h_cap_boundaryaddr2_reg.regValue);
#endif			
		} 
		else 
		{
			dc2h_dma_dc2h_cap_l1_start_RBUS dc2h_cap_l1_start_reg;
			dc2h_dma_dc2h_cap_l2_start_RBUS dc2h_cap_l2_start_reg;
			dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_reg;

			dc2h_dma_dc2h_cap_boundaryaddr1_RBUS dc2h_cap_boundaryaddr1_reg;
			dc2h_dma_dc2h_cap_boundaryaddr2_RBUS dc2h_cap_boundaryaddr2_reg;

			if (get_vt_VtCaptureVDC()){
				dc2h_cap_l1_start_reg.regValue= (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_L1_Start_reg, dc2h_cap_l1_start_reg.regValue);  // block0, DDR buffer L1 start address

				dc2h_cap_l2_start_reg.regValue= (UINT32)CaptureCtrl_VT.cap_buffer[1].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_L2_Start_reg, dc2h_cap_l2_start_reg.regValue);  // block0, DDR buffer L2 start address

				dc2h_cap_l3_start_reg.regValue= (UINT32)CaptureCtrl_VT.cap_buffer[2].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg, dc2h_cap_l3_start_reg.regValue);  // block0, DDR buffer L3 start address

				dc2h_cap_boundaryaddr1_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[2].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[2].size;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr1_reg, dc2h_cap_boundaryaddr1_reg.regValue);

				dc2h_cap_boundaryaddr2_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr2_reg, dc2h_cap_boundaryaddr2_reg.regValue);			
			} else {
				if(get_vt_VtBufferNum()==5) {
					dc2h_cap_l1_start_reg.regValue= (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;
					rtd_outl(DC2H_DMA_dc2h_Cap_L1_Start_reg, dc2h_cap_l1_start_reg.regValue);  // block0, DDR buffer L1 start address

					dc2h_cap_l2_start_reg.regValue= 0;
					rtd_outl(DC2H_DMA_dc2h_Cap_L2_Start_reg, dc2h_cap_l2_start_reg.regValue);  // block0, DDR buffer L2 start address

					dc2h_cap_l3_start_reg.regValue= 0;
					rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg, dc2h_cap_l3_start_reg.regValue);  // block0, DDR buffer L3 start address

					dc2h_cap_boundaryaddr1_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[4].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[4].size;
					rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr1_reg, dc2h_cap_boundaryaddr1_reg.regValue);

					dc2h_cap_boundaryaddr2_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;
					rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr2_reg, dc2h_cap_boundaryaddr2_reg.regValue);
				} else if(get_vt_VtBufferNum()==4){
					dc2h_cap_l1_start_reg.regValue= (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;
					rtd_outl(DC2H_DMA_dc2h_Cap_L1_Start_reg, dc2h_cap_l1_start_reg.regValue);	// block0, DDR buffer L1 start address

					dc2h_cap_l2_start_reg.regValue= 0;
					rtd_outl(DC2H_DMA_dc2h_Cap_L2_Start_reg, dc2h_cap_l2_start_reg.regValue);	// block0, DDR buffer L2 start address

					dc2h_cap_l3_start_reg.regValue= 0;
					rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg, dc2h_cap_l3_start_reg.regValue);	// block0, DDR buffer L3 start address

					dc2h_cap_boundaryaddr1_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[3].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[3].size;
					rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr1_reg, dc2h_cap_boundaryaddr1_reg.regValue);

					dc2h_cap_boundaryaddr2_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;
					rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr2_reg, dc2h_cap_boundaryaddr2_reg.regValue);

				} else if(get_vt_VtBufferNum()==3){
					dc2h_cap_l1_start_reg.regValue= (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;
					rtd_outl(DC2H_DMA_dc2h_Cap_L1_Start_reg, dc2h_cap_l1_start_reg.regValue);	// block0, DDR buffer L1 start address

					dc2h_cap_l2_start_reg.regValue= 0;
					rtd_outl(DC2H_DMA_dc2h_Cap_L2_Start_reg, dc2h_cap_l2_start_reg.regValue);	// block0, DDR buffer L2 start address

					dc2h_cap_l3_start_reg.regValue= 0;
					rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg, dc2h_cap_l3_start_reg.regValue);	// block0, DDR buffer L3 start address

					dc2h_cap_boundaryaddr1_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[2].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[2].size;
					rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr1_reg, dc2h_cap_boundaryaddr1_reg.regValue);

					dc2h_cap_boundaryaddr2_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;
					rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr2_reg, dc2h_cap_boundaryaddr2_reg.regValue);

				}else if(get_vt_VtBufferNum()==2){
					dc2h_cap_l1_start_reg.regValue= (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;
					rtd_outl(DC2H_DMA_dc2h_Cap_L1_Start_reg, dc2h_cap_l1_start_reg.regValue);	// block0, DDR buffer L1 start address

					dc2h_cap_l2_start_reg.regValue= 0;
					rtd_outl(DC2H_DMA_dc2h_Cap_L2_Start_reg, dc2h_cap_l2_start_reg.regValue);	// block0, DDR buffer L2 start address

					dc2h_cap_l3_start_reg.regValue= 0;
					rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg, dc2h_cap_l3_start_reg.regValue);	// block0, DDR buffer L3 start address

					dc2h_cap_boundaryaddr1_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[1].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[1].size;
					rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr1_reg, dc2h_cap_boundaryaddr1_reg.regValue);

					dc2h_cap_boundaryaddr2_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;
					rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr2_reg, dc2h_cap_boundaryaddr2_reg.regValue);

				}else if(get_vt_VtBufferNum()==1){
					dc2h_cap_l1_start_reg.regValue= (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;
					rtd_outl(DC2H_DMA_dc2h_Cap_L1_Start_reg, dc2h_cap_l1_start_reg.regValue);	// block0, DDR buffer L1 start address

					dc2h_cap_l2_start_reg.regValue= 0;
					rtd_outl(DC2H_DMA_dc2h_Cap_L2_Start_reg, dc2h_cap_l2_start_reg.regValue);	// block0, DDR buffer L2 start address

					dc2h_cap_l3_start_reg.regValue= 0;
					rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg, dc2h_cap_l3_start_reg.regValue);	// block0, DDR buffer L3 start address

					dc2h_cap_boundaryaddr1_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[0].size;
					rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr1_reg, dc2h_cap_boundaryaddr1_reg.regValue);

					dc2h_cap_boundaryaddr2_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;
					rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr2_reg, dc2h_cap_boundaryaddr2_reg.regValue);
				}
			}
		}
		drvif_dc2h_seqmode_conifg(TRUE, capInfo.capWid, capInfo.capLen, capInfo.capSrc);

		if(get_vt_VtBufferNum()==1){
			waitbufferms = 20;
		}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
		timeout_s = ns_to_timespec64(waitbufferms * 1000 * 1000);
		hrtimer_nanosleep(&timeout_s,  HRTIMER_MODE_REL, CLOCK_MONOTONIC);
#else
		timeout_s = ns_to_timespec(waitbufferms * 1000 * 1000);
		hrtimer_nanosleep(&timeout_s, NULL, HRTIMER_MODE_REL, CLOCK_MONOTONIC);
#endif	
		if(get_vt_VtBufferNum()==1) {
			if((drvif_scaler_get_abnormal_dvs_long_flag()==TRUE)||(drvif_scaler_get_abnormal_dvs_short_flag()==TRUE)){				
				rtd_pr_vt_emerg("[Warning VBE]%s=%d Vbe at abnormal dvs status,stop vt capture!\n", __FUNCTION__, __LINE__);
				/*abonrmal timing closed dc2h*/
				drvif_dc2h_seqmode_conifg(FALSE, capInfo.capWid, capInfo.capLen, capInfo.capSrc);
			}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
			timeout_s = ns_to_timespec64(waitbufferms * 1000 * 1000);
			hrtimer_nanosleep(&timeout_s,  HRTIMER_MODE_REL, CLOCK_MONOTONIC);
#else
			timeout_s = ns_to_timespec(waitbufferms * 1000 * 1000);
			hrtimer_nanosleep(&timeout_s, NULL, HRTIMER_MODE_REL, CLOCK_MONOTONIC);
#endif				
		}
#if 0//trash debug		
		if((capInfo.capLen==1080)&&(capInfo.capWid==1920)&&(capInfo.capSrc==0)&&(capInfo.enable==1)){
			rtd_pr_vt_emerg("%s=%d dump all allocate buffer!!\n", __FUNCTION__, __LINE__); 
			vt_dump_data_to_file(0, (VT_CAP_FRAME_MAX_WIDTH*VT_CAP_FRAME_MAX_LENGTH+VT_CAP_FRAME_MAX_WIDTH*VT_CAP_FRAME_MAX_LENGTH+_ALIGN(VT_CAP_FRAME_MAX_WIDTH*VT_CAP_FRAME_MAX_LENGTH,__12KPAGE)));
		}

		if((capInfo.enable==1)){
			rtd_pr_vt_emerg("data1 = %d\n", *((unsigned char*)(VtAllocatedBufferStartAdress+(VT_CAP_FRAME_MAX_WIDTH*VT_CAP_FRAME_MAX_LENGTH+_ALIGN(VT_CAP_FRAME_MAX_WIDTH*VT_CAP_FRAME_MAX_LENGTH,__12KPAGE)+1))));
			rtd_pr_vt_emerg("data2 = %d\n", *((unsigned char*)(VtAllocatedBufferStartAdress+(VT_CAP_FRAME_MAX_WIDTH*VT_CAP_FRAME_MAX_LENGTH+_ALIGN(VT_CAP_FRAME_MAX_WIDTH*VT_CAP_FRAME_MAX_LENGTH,__12KPAGE)+2))));
			rtd_pr_vt_emerg("data3 = %d\n", *((unsigned char*)(VtAllocatedBufferStartAdress+(VT_CAP_FRAME_MAX_WIDTH*VT_CAP_FRAME_MAX_LENGTH+_ALIGN(VT_CAP_FRAME_MAX_WIDTH*VT_CAP_FRAME_MAX_LENGTH,__12KPAGE)+3))));
			rtd_pr_vt_emerg("data4 = %d\n", *((unsigned char*)(VtAllocatedBufferStartAdress+(VT_CAP_FRAME_MAX_WIDTH*VT_CAP_FRAME_MAX_LENGTH+_ALIGN(VT_CAP_FRAME_MAX_WIDTH*VT_CAP_FRAME_MAX_LENGTH,__12KPAGE)+4))));
		}
#endif
	} else {
		if((get_VT_Pixel_Format() == VT_CAP_NV12)||(get_VT_Pixel_Format() == VT_CAP_NV16)){		
			dc2h_scaledown_dc2h_444to422_ctrl_RBUS dc2h_scaledown_dc2h_444to422_ctrl_reg;
			
			dc2h_scaledown_dc2h_444to422_ctrl_reg.regValue = rtd_inl(DC2H_SCALEDOWN_DC2H_444to422_CTRL_reg);
			if(VTDumpLocation == KADP_VT_SCALER_OUTPUT)
				dc2h_scaledown_dc2h_444to422_ctrl_reg.cbcr_swap_en =0;
			rtd_outl(DC2H_SCALEDOWN_DC2H_444to422_CTRL_reg, dc2h_scaledown_dc2h_444to422_ctrl_reg.regValue);  //used record freeze buffer
		}
		drvif_dc2h_seqmode_conifg(FALSE, capInfo.capWid, capInfo.capLen, capInfo.capSrc);
	}
}

void drvif_dc2h_seqmode_conifg(unsigned char enable, int capWid, int capLen, int capSrc)
{
	ppoverlay_main_den_h_start_end_RBUS main_den_h_start_end_Reg;
	ppoverlay_main_den_v_start_end_RBUS main_den_v_start_end_Reg;
	ppoverlay_main_active_h_start_end_RBUS main_active_h_start_end_Reg;
	ppoverlay_main_active_v_start_end_RBUS main_active_v_start_end_Reg;

	ppoverlay_uzudtg_control1_RBUS ppoverlay_uzudtg_control1_reg;
	
	main_den_h_start_end_Reg.regValue = rtd_inl(PPOVERLAY_MAIN_DEN_H_Start_End_reg);
	main_den_v_start_end_Reg.regValue = rtd_inl(PPOVERLAY_MAIN_DEN_V_Start_End_reg);
	main_active_h_start_end_Reg.regValue = rtd_inl(PPOVERLAY_MAIN_Active_H_Start_End_reg);
	main_active_v_start_end_Reg.regValue = rtd_inl(PPOVERLAY_MAIN_Active_V_Start_End_reg);

	ppoverlay_uzudtg_control1_reg.regValue = rtd_inl(PPOVERLAY_uzudtg_control1_reg);

	pdc2h_hdl = &dc2h_hdl;
	memset(pdc2h_hdl, 0 , sizeof(DC2H_HANDLER));
	pdc2h_hdl->out_size.nWidth = capWid;
	pdc2h_hdl->out_size.nLength = capLen;
	pdc2h_hdl->dc2h_en = enable;					

#if 1
	
	if((capSrc == KADP_VT_SCALER_OUTPUT) && (get_video_do_overscan_state()))
	{
	/*if dumplocation=scaler output and video path has do overscan,
		so for geting no overscan data,will use SE capture from i3ddma or vdec */
		//capSrc = KADP_VT_SCALER_INPUT;
		rtd_pr_vt_notice("vt source capture flow\n");
		return;
	}

#endif
		
	if(/*(capSrc == 2)&&*/(get_vt_VtCaptureVDC())){
			pdc2h_hdl->in_size.nWidth = DC2H_InputWidth;
			pdc2h_hdl->in_size.nLength = DC2H_InputLength;
			pdc2h_hdl->dc2h_in_sel = _VD_OUTPUT;
	}
	else 
	{
		if(Get_DISPLAY_REFRESH_RATE() >= 120) //_120hz panel, ==>b802850c,two_step_uzu_en=1,uzu_4k_120_mode=1
		{
			dc2h_rgb2yuv_dc2h_rgb2yuv_ctrl_RBUS  dc2h_rgb2yuv_ctrl_reg;
			dc2h_rgb2yuv_ctrl_reg.regValue = rtd_inl(DC2H_RGB2YUV_DC2H_RGB2YUV_CTRL_reg); //9c00
			dc2h_rgb2yuv_ctrl_reg.dc2h_drop_en = 1; //only 4k_120 can use
			rtd_outl(DC2H_RGB2YUV_DC2H_RGB2YUV_CTRL_reg, dc2h_rgb2yuv_ctrl_reg.regValue);

			rtd_pr_vt_debug("120-panel\n");
			
			if(KADP_VT_SCALER_OUTPUT == capSrc)   
			{
				pdc2h_hdl->dc2h_in_sel = _TWO_SECOND_UZU; //4k_120 mode 2nd uzu on
				pdc2h_hdl->in_size.nWidth = (main_active_h_start_end_Reg.mh_act_end - main_active_h_start_end_Reg.mh_act_sta);
				if((pdc2h_hdl->in_size.nWidth % 2) != 0)
				{
					pdc2h_hdl->in_size.nWidth -= 1;
				}	
				
				if(Get_DISPLAY_REFRESH_RATE() == (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ)/10))
					pdc2h_hdl->in_size.nWidth = (pdc2h_hdl->in_size.nWidth)/2;
				
				pdc2h_hdl->in_size.nLength = main_active_v_start_end_Reg.mv_act_end - main_active_v_start_end_Reg.mv_act_sta;
			}
			else if(KADP_VT_DISPLAY_OUTPUT == capSrc)
			{
				if(is_4K2K_capture() == TRUE)
				{
					pdc2h_hdl->dc2h_in_sel = _TWO_SECOND_UZU;
					pdc2h_hdl->in_size.nWidth = (main_active_h_start_end_Reg.mh_act_end - main_active_h_start_end_Reg.mh_act_sta);
					if((pdc2h_hdl->in_size.nWidth % 2) != 0)
					{
							pdc2h_hdl->in_size.nWidth -= 1;
					}		
			
					if(Get_DISPLAY_REFRESH_RATE() == (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ)/10))
							pdc2h_hdl->in_size.nWidth = (pdc2h_hdl->in_size.nWidth)/2;
			
					pdc2h_hdl->in_size.nLength = main_active_v_start_end_Reg.mv_act_end - main_active_v_start_end_Reg.mv_act_sta;
				}
				else
				{
					pdc2h_hdl->dc2h_in_sel = _Memc_mux_Output;
					pdc2h_hdl->in_size.nWidth = (main_den_h_start_end_Reg.mh_den_end - main_den_h_start_end_Reg.mh_den_sta)/2;
					pdc2h_hdl->in_size.nLength = main_den_v_start_end_Reg.mv_den_end - main_den_v_start_end_Reg.mv_den_sta; 			   
				}
			}
			else if(KADP_VT_OSDVIDEO_OUTPUT == capSrc)
			{
				pdc2h_hdl->dc2h_in_sel = _OSD_MIXER;		
				pdc2h_hdl->in_size.nWidth = (main_den_h_start_end_Reg.mh_den_end - main_den_h_start_end_Reg.mh_den_sta)/2;
				pdc2h_hdl->in_size.nLength = main_den_v_start_end_Reg.mv_den_end - main_den_v_start_end_Reg.mv_den_sta;
			}
			else
			{				
				rtd_pr_vt_emerg("[Vt error] unknow the capSrc line:%d\n",__LINE__);
			}
		}
		else  // non 4k_120 mode 
		{
			rtd_pr_vt_debug("non-120-panel\n");
			
			if(KADP_VT_SCALER_OUTPUT == capSrc)
			{
				pdc2h_hdl->dc2h_in_sel = _MAIN_UZU;
				if (ppoverlay_uzudtg_control1_reg.two_step_uzu_en) {
					pdc2h_hdl->in_size.nWidth = (main_active_h_start_end_Reg.mh_act_end - main_active_h_start_end_Reg.mh_act_sta)/2;			
					pdc2h_hdl->in_size.nLength = (main_active_v_start_end_Reg.mv_act_end - main_active_v_start_end_Reg.mv_act_sta)/2;
				} else {
					pdc2h_hdl->in_size.nWidth = (main_active_h_start_end_Reg.mh_act_end - main_active_h_start_end_Reg.mh_act_sta);			
					pdc2h_hdl->in_size.nLength = (main_active_v_start_end_Reg.mv_act_end - main_active_v_start_end_Reg.mv_act_sta);
				}
			}
			else if(KADP_VT_DISPLAY_OUTPUT == capSrc)
			{
				pdc2h_hdl->dc2h_in_sel = _Memc_mux_Output;
				pdc2h_hdl->in_size.nWidth = (main_den_h_start_end_Reg.mh_den_end - main_den_h_start_end_Reg.mh_den_sta);
				pdc2h_hdl->in_size.nLength = main_den_v_start_end_Reg.mv_den_end - main_den_v_start_end_Reg.mv_den_sta;
			}
			else if(KADP_VT_OSDVIDEO_OUTPUT == capSrc)
			{
				pdc2h_hdl->dc2h_in_sel = _OSD_MIXER;		
				pdc2h_hdl->in_size.nWidth = (main_den_h_start_end_Reg.mh_den_end - main_den_h_start_end_Reg.mh_den_sta);
				pdc2h_hdl->in_size.nLength = main_den_v_start_end_Reg.mv_den_end - main_den_v_start_end_Reg.mv_den_sta;
			}
			else
			{				
				rtd_pr_vt_emerg("[Vt error] unknow the capSrc line:%d\n",__LINE__);
			}
			
		}
				
	}					

	if (enable == TRUE) {
		if(get_vt_VtSwBufferMode()){
			if(FALSE == drvif_dc2h_swmode_inforpc(TRUE,get_vt_VtBufferNum(),capWid,capLen)){
				rtd_pr_vt_emerg("%s=%d RPC fail so not setting dc2h and return!!\n", __FUNCTION__, __LINE__);	
				return;
			}
		}
#if 0		
		if(get_vt_VtCaptureVDC()){
			drvif_dc2h_input_overscan_config(DC2H_InputWidthSta, DC2H_InputLengthSta, pdc2h_hdl->in_size.nWidth, pdc2h_hdl->in_size.nLength);
		} else {
			dc2h_rgb2yuv_dc2h_3dmaskto2d_ctrl_Reg.regValue = rtd_inl(DC2H_RGB2YUV_DC2H_3DMaskTo2D_Ctrl_reg);
			dc2h_rgb2yuv_dc2h_3dmaskto2d_ctrl_Reg.dc2h_3dmaskto2d_3dformat = 0;
			dc2h_rgb2yuv_dc2h_3dmaskto2d_ctrl_Reg.dc2h_3dmaskto2d_en = 0;
			rtd_outl(DC2H_RGB2YUV_DC2H_3DMaskTo2D_Ctrl_reg, dc2h_rgb2yuv_dc2h_3dmaskto2d_ctrl_Reg.regValue);
		}
#endif		
		rtd_pr_vt_emerg("[Vt capinfo]vt cap sel=%d input w= %d; input h=%d;output w=%d,output h=%d! \n", pdc2h_hdl->dc2h_in_sel,pdc2h_hdl->in_size.nWidth, pdc2h_hdl->in_size.nLength,pdc2h_hdl->out_size.nWidth,pdc2h_hdl->out_size.nLength);
	
		//if(capSrc == VT_CAP_SRC_VIDEO_NOPQ) {
			//drvif_dc2h_input_overscan_config(main_active_h_start_end_Reg.mh_act_sta, main_active_v_start_end_Reg.mv_act_sta, pdc2h_hdl->in_size.nWidth, pdc2h_hdl->in_size.nLength);
		//} else {
			drvif_dc2h_input_overscan_config(0, 0, pdc2h_hdl->in_size.nWidth, pdc2h_hdl->in_size.nLength);
		//}
		drvif_color_colorspaceyuv2rgbtransfer(pdc2h_hdl->dc2h_in_sel);
		drvif_color_ultrazoom_config_dc2h_scaling_down(pdc2h_hdl);
		if(get_VT_Pixel_Format() == VT_CAP_NV12)
			drvif_vi_config(pdc2h_hdl->out_size.nWidth,pdc2h_hdl->out_size.nLength,0);
		else if (get_VT_Pixel_Format() == VT_CAP_NV16)
			drvif_vi_config(pdc2h_hdl->out_size.nWidth,pdc2h_hdl->out_size.nLength,1);
		else
			drvif_dc2h_seq_config(pdc2h_hdl);
	} 
	else 
	{
		reset_dc2h_hw_setting();  /* disable dc2h and reset dc2h */
		
		if (get_vt_VtSwBufferMode())
		{
			if(FALSE == drvif_dc2h_swmode_inforpc(FALSE,get_vt_VtBufferNum(),capWid,capLen))
			{
				rtd_pr_vt_emerg("%s=%d RPC fail when close dc2h!!\n", __FUNCTION__, __LINE__);	
			}
		}		
	}
}

void reset_dc2h_hw_setting(void)
{
	dc2h_dma_dc2h_seq_interrupt_enable_RBUS dc2h_dma_dc2h_seq_interrupt_enable_Reg;
	dc2h_vi_dc2h_dmactl_RBUS vi_dmactl_reg;
	dc2h_rgb2yuv_dc2h_3dmaskto2d_ctrl_RBUS dc2h_rgb2yuv_dc2h_3dmaskto2d_ctrl_Reg;
	dc2h_rgb2yuv_dc2h_rgb2yuv_ctrl_RBUS	dc2h_rgb2yuv_ctrl_reg;
	dc2h_scaledown_dc2h_hsd_ctrl0_RBUS dc2h_hsd_ctrl0_reg;
	dc2h_scaledown_dc2h_vsd_ctrl0_RBUS dc2h_vsd_ctrl0_reg;
	dc2h_dma_dc2h_seq_mode_ctrl1_RBUS dc2h_seq_mode_ctrl1_reg;

	
	if (get_vt_VtSwBufferMode())
	{
		dc2h_dma_dc2h_seq_interrupt_enable_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_seq_Interrupt_Enable_reg);
		dc2h_dma_dc2h_seq_interrupt_enable_Reg.vs_rising_ie = 0;
		rtd_outl(DC2H_DMA_dc2h_seq_Interrupt_Enable_reg, dc2h_dma_dc2h_seq_interrupt_enable_Reg.regValue);
		msleep(20);
	}
	
	if ((get_VT_Pixel_Format() == VT_CAP_NV12)||(get_VT_Pixel_Format() == VT_CAP_NV16))
	{
		vi_dmactl_reg.regValue = rtd_inl(DC2H_VI_DC2H_DMACTL_reg);
		vi_dmactl_reg.dmaen1 = 0;
		rtd_outl(DC2H_VI_DC2H_DMACTL_reg, vi_dmactl_reg.regValue);		
	}

	dc2h_rgb2yuv_dc2h_3dmaskto2d_ctrl_Reg.regValue = rtd_inl(DC2H_RGB2YUV_DC2H_3DMaskTo2D_Ctrl_reg);
	dc2h_rgb2yuv_dc2h_3dmaskto2d_ctrl_Reg.dc2h_3dmaskto2d_3dformat = 0;
	dc2h_rgb2yuv_dc2h_3dmaskto2d_ctrl_Reg.dc2h_3dmaskto2d_en = 0;
	rtd_outl(DC2H_RGB2YUV_DC2H_3DMaskTo2D_Ctrl_reg, dc2h_rgb2yuv_dc2h_3dmaskto2d_ctrl_Reg.regValue);


	dc2h_rgb2yuv_ctrl_reg.regValue = rtd_inl(DC2H_RGB2YUV_DC2H_RGB2YUV_CTRL_reg);
	dc2h_rgb2yuv_ctrl_reg.dc2h_in_sel = 0;
	dc2h_rgb2yuv_ctrl_reg.en_rgb2yuv = 0;
	rtd_outl(DC2H_RGB2YUV_DC2H_RGB2YUV_CTRL_reg, dc2h_rgb2yuv_ctrl_reg.regValue);

	dc2h_hsd_ctrl0_reg.regValue = rtd_inl(DC2H_SCALEDOWN_DC2H_HSD_Ctrl0_reg);
	dc2h_hsd_ctrl0_reg.h_zoom_en = 0;
	rtd_outl(DC2H_SCALEDOWN_DC2H_HSD_Ctrl0_reg, dc2h_hsd_ctrl0_reg.regValue);

	dc2h_vsd_ctrl0_reg.regValue = rtd_inl(DC2H_SCALEDOWN_DC2H_VSD_Ctrl0_reg);
	dc2h_vsd_ctrl0_reg.v_zoom_en = 0;
	dc2h_vsd_ctrl0_reg.buffer_mode = 0;
	rtd_outl(DC2H_SCALEDOWN_DC2H_VSD_Ctrl0_reg, dc2h_vsd_ctrl0_reg.regValue);
	
	dc2h_seq_mode_ctrl1_reg.regValue = rtd_inl(DC2H_DMA_dc2h_Seq_mode_CTRL1_reg);
	if((get_VT_Pixel_Format() == VT_CAP_ABGR8888)||(get_VT_Pixel_Format() == VT_CAP_ARGB8888))
		dc2h_seq_mode_ctrl1_reg.rgb_to_argb_en = 0;
	dc2h_seq_mode_ctrl1_reg.cap_en = 0;
	rtd_outl(DC2H_DMA_dc2h_Seq_mode_CTRL1_reg, dc2h_seq_mode_ctrl1_reg.regValue);
	
}

unsigned char is_4K2K_capture(void)
{
	if((vt_cap_frame_max_width == VT_CAP_FRAME_WIDTH_2K1k) && (vt_cap_frame_max_height == VT_CAP_FRAME_HEIGHT_2K1k))
		return FALSE;
	else
		return TRUE;
}

unsigned int sort_boundary_addr_max_index(void)
{

	unsigned int i,index = 0;
	unsigned int bufnum = 5;
	unsigned int max = CaptureCtrl_VT.cap_buffer[0].phyaddr;
	
	for( i = 1; i < bufnum; i++)
	{
		if(CaptureCtrl_VT.cap_buffer[i].phyaddr > max)
		{
			max = CaptureCtrl_VT.cap_buffer[i].phyaddr;
			index = i;
		}
		
	}
	return index;
}

unsigned int sort_boundary_addr_min_index(void)
{
	unsigned int i,index = 0;
	unsigned int bufnum = 5;
	unsigned int min = CaptureCtrl_VT.cap_buffer[0].phyaddr;
	
	for( i = 1; i < bufnum; i++)
	{
		if(CaptureCtrl_VT.cap_buffer[i].phyaddr < min)
		{
			min = CaptureCtrl_VT.cap_buffer[i].phyaddr;
			index = i;
		}
		
	}
	return index;
}

void set_cap_buffer_size_by_AP(unsigned int usr_width, unsigned int usr_height)
{
	vt_cap_frame_max_width = usr_width;
	vt_cap_frame_max_height = usr_height;
#if 0	
	if((usr_width <= VT_CAP_FRAME_WIDTH_2K1k) && (usr_height <= VT_CAP_FRAME_HEIGHT_2K1k))
	{
		vt_cap_frame_max_width = VT_CAP_FRAME_WIDTH_2K1k;
		vt_cap_frame_max_height = VT_CAP_FRAME_HEIGHT_2K1k;
	}
	else
	{
		vt_cap_frame_max_width = VT_CAP_FRAME_WIDTH_4K2K;
		vt_cap_frame_max_height = VT_CAP_FRAME_HEIGHT_4K2K;
	}
#endif
	rtd_pr_vt_debug("VT capture buffer size:(%d,%d)\n",vt_cap_frame_max_width, vt_cap_frame_max_height);
}
#endif 


#ifdef CONFIG_HW_SUPPORT_DC2H 
void set_vfod_freezed_for_dc2h(unsigned char boffon)
{
	dc2h_vi_dc2h_vi_doublebuffer_RBUS dc2h_vi_doublebuffer_Reg;
	rtd_pr_vt_debug("fun:%s,boffon=%d\n", __FUNCTION__, boffon);
	
	if (CaptureCtrl_VT.cap_buffer[0].phyaddr == 0) {
		rtd_pr_vt_emerg("[Vt warning]vt no init, so no memory, return freeze action;%s=%d \n", __FUNCTION__, __LINE__);
		return;
	}
#if 0	
	if((get_vt_VtBufferNum()==1)&&(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE)){
		rtd_pr_vt_emerg("[Vt warning]%s=%d Vsc not at _MODE_STATE_ACTIVE status,return freeze action!\n", __FUNCTION__, __LINE__);
		return;			
	}
#endif

	/*if(((drvif_scaler_get_abnormal_dvs_long_flag()==TRUE)||(drvif_scaler_get_abnormal_dvs_short_flag()==TRUE))
		&& (get_vt_VtBufferNum() == 5)){
		rtd_pr_vt_emerg("[Warning VBE]%s=%d Vbe at abnormal dvs status,could not do vt capture!\n", __FUNCTION__, __LINE__);
		return;			
	}	*/


	dc2h_vi_doublebuffer_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_doublebuffer_reg);
	if(get_vt_VtBufferNum() == 1) 
	{
		unsigned int timeoutcount = 0x3ffff;
		if(boffon == TRUE) //vidioc_dqbuf  
		{
			dc2h_vi_doublebuffer_Reg.vi_db_en = 1;
			if((get_VT_Pixel_Format() == VT_CAP_NV12)||(get_VT_Pixel_Format() == VT_CAP_NV16))
			{
				dc2h_vi_dc2h_dmactl_RBUS dc2h_vi_dc2h_dmactl_Reg;
				dc2h_vi_dc2h_dmactl_Reg.regValue = rtd_inl(DC2H_VI_DC2H_DMACTL_reg);
				dc2h_vi_dc2h_dmactl_Reg.dmaen1 = 0; //double buffer 
				rtd_outl(DC2H_VI_DC2H_DMACTL_reg,dc2h_vi_dc2h_dmactl_Reg.regValue);
			}
			else
			{				
				dc2h_dma_dc2h_seq_mode_ctrl1_RBUS dc2h_dma_dc2h_seq_mode_ctrl1_Reg;
				dc2h_dma_dc2h_seq_mode_ctrl1_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Seq_mode_CTRL1_reg);
				dc2h_dma_dc2h_seq_mode_ctrl1_Reg.cap_en = 0; //double buffer
				rtd_outl(DC2H_DMA_dc2h_Seq_mode_CTRL1_reg,dc2h_dma_dc2h_seq_mode_ctrl1_Reg.regValue);
			}
			dc2h_vi_doublebuffer_Reg.vi_db_apply = 1;
			rtd_outl(DC2H_VI_DC2H_vi_doublebuffer_reg, dc2h_vi_doublebuffer_Reg.regValue);
			
			do{
				dc2h_vi_doublebuffer_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_doublebuffer_reg);
				if(DC2H_VI_DC2H_vi_doublebuffer_get_vi_db_apply(dc2h_vi_doublebuffer_Reg.regValue))
					timeoutcount--;
				else{
					IndexOfFreezedVideoFrameBuffer = 0;
					break;
				}
								
			}while(timeoutcount);	

			if(timeoutcount == 0)
			{
				rtd_pr_vt_notice("fun:%s=%d, freeze action timeout\n", __FUNCTION__, __LINE__);
			}		
		}
		else
		{
			if(IndexOfFreezedVideoFrameBuffer != 0)
			{
				rtd_pr_vt_notice("fun:%s=%d, unfreeze index error\n", __FUNCTION__, __LINE__);
				return;
			}
			dc2h_vi_doublebuffer_Reg.vi_db_en = 1;  //enable double buffer
			if((get_VT_Pixel_Format() == VT_CAP_NV12)||(get_VT_Pixel_Format() == VT_CAP_NV16))
			{
				dc2h_vi_dc2h_dmactl_RBUS dc2h_vi_dc2h_dmactl_Reg;
				dc2h_vi_dc2h_dmactl_Reg.regValue = rtd_inl(DC2H_VI_DC2H_DMACTL_reg);
				dc2h_vi_dc2h_dmactl_Reg.dmaen1 = 1; //double buffer 
				rtd_outl(DC2H_VI_DC2H_DMACTL_reg,dc2h_vi_dc2h_dmactl_Reg.regValue);
			}
			else
			{				
				dc2h_dma_dc2h_seq_mode_ctrl1_RBUS dc2h_dma_dc2h_seq_mode_ctrl1_Reg;
				dc2h_dma_dc2h_seq_mode_ctrl1_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Seq_mode_CTRL1_reg);
				dc2h_dma_dc2h_seq_mode_ctrl1_Reg.cap_en = 1; //double buffer
				rtd_outl(DC2H_DMA_dc2h_Seq_mode_CTRL1_reg,dc2h_dma_dc2h_seq_mode_ctrl1_Reg.regValue);
			}
			dc2h_vi_doublebuffer_Reg.vi_db_apply = 1;
			rtd_outl(DC2H_VI_DC2H_vi_doublebuffer_reg, dc2h_vi_doublebuffer_Reg.regValue);
			do{
				dc2h_vi_doublebuffer_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_doublebuffer_reg);
				if(DC2H_VI_DC2H_vi_doublebuffer_get_vi_db_apply(dc2h_vi_doublebuffer_Reg.regValue))
					timeoutcount--;
				else
					break;
							
			}while(timeoutcount);	

			if(timeoutcount == 0)
			{
				rtd_pr_vt_notice("fun:%s=%d, unfreeze action timeout\n", __FUNCTION__, __LINE__);
			}					
		}
		dc2h_vi_doublebuffer_Reg.vi_db_en = 0; /* doubel buffer only work when buffer_num=1 */
		rtd_outl(DC2H_VI_DC2H_vi_doublebuffer_reg, dc2h_vi_doublebuffer_Reg.regValue);
		
	} else {
		if (boffon == TRUE) {
			if((get_VT_Pixel_Format() == VT_CAP_NV12)||(get_VT_Pixel_Format() == VT_CAP_NV16)){
				if(get_vt_VtBufferNum()==5) {
					dc2h_vi_dc2h_vi_ads_start_y_RBUS DC2H_VI_DC2H_vi_ads_start_y_Reg;
					dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_Reg;
					unsigned int lastcapbufferaddr = 0;
					DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_ads_start_y_reg);
					dc2h_cap_l3_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L3_Start_reg);
			
					if(DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[0].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[4].phyaddr;
					}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[1].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[0].phyaddr;
					}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[2].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[1].phyaddr;
					}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[3].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[2].phyaddr;
					}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[4].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[3].phyaddr;
					}
			
					if(lastcapbufferaddr == dc2h_cap_l3_start_Reg.regValue){
						if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[4].phyaddr;
						}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[0].phyaddr;
						}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[2].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[1].phyaddr;
						}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[3].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[2].phyaddr;
						}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[4].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[3].phyaddr;
						}
					}
					if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr){
						IndexOfFreezedVideoFrameBuffer = 0;
					}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr){
						IndexOfFreezedVideoFrameBuffer = 1;
					}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[2].phyaddr){
						IndexOfFreezedVideoFrameBuffer = 2;
					}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[3].phyaddr){
						IndexOfFreezedVideoFrameBuffer = 3;
					}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[4].phyaddr){
						IndexOfFreezedVideoFrameBuffer = 4;
					}
					rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
				} else if(get_vt_VtBufferNum()==4) {
					dc2h_vi_dc2h_vi_ads_start_y_RBUS DC2H_VI_DC2H_vi_ads_start_y_Reg;
					dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_Reg;
					unsigned int lastcapbufferaddr = 0;
					DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_ads_start_y_reg);
					dc2h_cap_l3_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L3_Start_reg);

			
					if(DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[0].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[3].phyaddr;
					}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[1].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[0].phyaddr;
					}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[2].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[1].phyaddr;
					}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[3].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[2].phyaddr;
					}
			
					if(lastcapbufferaddr == dc2h_cap_l3_start_Reg.regValue){
						if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[3].phyaddr;
						}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[0].phyaddr;
						}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[2].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[1].phyaddr;
						}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[3].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[2].phyaddr;
						}
					}
					if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr){
						IndexOfFreezedVideoFrameBuffer = 0;
					}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr){
						IndexOfFreezedVideoFrameBuffer = 1;
					}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[2].phyaddr){
						IndexOfFreezedVideoFrameBuffer = 2;
					}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[3].phyaddr){
						IndexOfFreezedVideoFrameBuffer = 3;
					}	
					rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
				} else if(get_vt_VtBufferNum()==3) {
					dc2h_vi_dc2h_vi_ads_start_y_RBUS DC2H_VI_DC2H_vi_ads_start_y_Reg;
					dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_Reg;
					unsigned int lastcapbufferaddr = 0;
					DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_ads_start_y_reg);
					dc2h_cap_l3_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L3_Start_reg);


		           if(DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[0].phyaddr){
		                   lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[2].phyaddr;
		           }else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[1].phyaddr){
		                   lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[0].phyaddr;
		           }else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[2].phyaddr){
		                   lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[1].phyaddr;
		           }

		           if(lastcapbufferaddr == dc2h_cap_l3_start_Reg.regValue){
		                   if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr){
		                           lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[2].phyaddr;
		                   }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr){
		                           lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[0].phyaddr;
		                   }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[2].phyaddr){
		                           lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[1].phyaddr;
		                   }
		           }
		           if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr){
		                   IndexOfFreezedVideoFrameBuffer = 0;
		           }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr){
		                   IndexOfFreezedVideoFrameBuffer = 1;
		           }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[2].phyaddr){
		                   IndexOfFreezedVideoFrameBuffer = 2;
		           }       
		           rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
		        }else if(get_vt_VtBufferNum()==2) {
					dc2h_vi_dc2h_vi_ads_start_y_RBUS DC2H_VI_DC2H_vi_ads_start_y_Reg;
					dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_Reg;
					unsigned int lastcapbufferaddr = 0;
					DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_ads_start_y_reg);
					dc2h_cap_l3_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L3_Start_reg);

		           if(DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[0].phyaddr){
		                   lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[0].phyaddr;
		           }else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[1].phyaddr){
		                   lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[1].phyaddr;
		           }

		           if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr){
		                   IndexOfFreezedVideoFrameBuffer = 0;
		           }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr){
		                   IndexOfFreezedVideoFrameBuffer = 1;
		           }    
		           rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
		        }

			} else {
				if(get_vt_VtBufferNum()==5) {
					dc2h_dma_dc2h_cap_l1_start_RBUS dc2h_cap_l1_start_Reg;
					dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_Reg;
					unsigned int lastcapbufferaddr = 0;
					dc2h_cap_l1_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L1_Start_db_reg);
					
					dc2h_cap_l3_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L3_Start_reg);
			
					if(dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[0].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[4].phyaddr;
					}else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[1].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[0].phyaddr;
					}else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[2].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[1].phyaddr;
					}else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[3].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[2].phyaddr;
					}else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[4].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[3].phyaddr;
					}
			
					if(lastcapbufferaddr == dc2h_cap_l3_start_Reg.regValue){
						if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[4].phyaddr;
						}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[0].phyaddr;
						}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[2].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[1].phyaddr;
						}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[3].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[2].phyaddr;
						}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[4].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[3].phyaddr;
						}
					}
					if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr){
						IndexOfFreezedVideoFrameBuffer = 0;
					}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr){
						IndexOfFreezedVideoFrameBuffer = 1;
					}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[2].phyaddr){
						IndexOfFreezedVideoFrameBuffer = 2;
					}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[3].phyaddr){
						IndexOfFreezedVideoFrameBuffer = 3;
					}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[4].phyaddr){
						IndexOfFreezedVideoFrameBuffer = 4;
					}
					rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
				} else if(get_vt_VtBufferNum()==4) {
					dc2h_dma_dc2h_cap_l1_start_RBUS dc2h_cap_l1_start_Reg;
					dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_Reg;
					unsigned int lastcapbufferaddr = 0;
					dc2h_cap_l1_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L1_Start_db_reg);
					
					dc2h_cap_l3_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L3_Start_reg);
			
					if(dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[0].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[3].phyaddr;
					}else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[1].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[0].phyaddr;
					}else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[2].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[1].phyaddr;
					}else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[3].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[2].phyaddr;
					}
			
					if(lastcapbufferaddr == dc2h_cap_l3_start_Reg.regValue){
						if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[3].phyaddr;
						}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[0].phyaddr;
						}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[2].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[1].phyaddr;
						}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[3].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[2].phyaddr;
						}
					}
					if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr){
						IndexOfFreezedVideoFrameBuffer = 0;
					}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr){
						IndexOfFreezedVideoFrameBuffer = 1;
					}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[2].phyaddr){
						IndexOfFreezedVideoFrameBuffer = 2;
					}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[3].phyaddr){
						IndexOfFreezedVideoFrameBuffer = 3;
					}	
					rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
				} else if(get_vt_VtBufferNum()==3) {
		           dc2h_dma_dc2h_cap_l1_start_RBUS dc2h_cap_l1_start_Reg;
		           dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_Reg;
		           unsigned int lastcapbufferaddr = 0;
		           dc2h_cap_l1_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L1_Start_db_reg);
		           
		           dc2h_cap_l3_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L3_Start_reg);

		           if(dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[0].phyaddr){
		                   lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[2].phyaddr;
		           }else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[1].phyaddr){
		                   lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[0].phyaddr;
		           }else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[2].phyaddr){
		                   lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[1].phyaddr;
		           }

		           if(lastcapbufferaddr == dc2h_cap_l3_start_Reg.regValue){
		                   if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr){
		                           lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[2].phyaddr;
		                   }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr){
		                           lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[0].phyaddr;
		                   }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[2].phyaddr){
		                           lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[1].phyaddr;
		                   }
		           }
		           if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr){
		                   IndexOfFreezedVideoFrameBuffer = 0;
		           }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr){
		                   IndexOfFreezedVideoFrameBuffer = 1;
		           }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[2].phyaddr){
		                   IndexOfFreezedVideoFrameBuffer = 2;
		           }       
		           rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
		        }else if(get_vt_VtBufferNum()==2) {
		           dc2h_dma_dc2h_cap_l1_start_RBUS dc2h_cap_l1_start_Reg;
		           dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_Reg;
		           unsigned int lastcapbufferaddr = 0;
		           dc2h_cap_l1_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L1_Start_db_reg);
		           
		           dc2h_cap_l3_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L3_Start_reg);

		           if(dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[0].phyaddr){
		                   lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[0].phyaddr;
		           }else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[1].phyaddr){
		                   lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[1].phyaddr;
		           }
		           if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr){
		                   IndexOfFreezedVideoFrameBuffer = 0;
		           }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr){
		                   IndexOfFreezedVideoFrameBuffer = 1;
		           }   
		           rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
		        }
				
			}
		}else{
			//rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,0);
		}
	}
}

#else

void sub_mdomain_seq_VtCap_freeze(unsigned char enable)
{

	if(TRUE == enable)
	{
		mdomain_cap_ddr_in2status_RBUS mdomain_cap_ddr_in2status_Reg;
	
		rtd_pr_vt_notice("MDOMAIN_CAP_DDR_In2Status_reg = %x\n", IoReg_Read32(MDOMAIN_CAP_DDR_In2Status_reg));
		mdomain_cap_ddr_in2status_Reg.regValue = IoReg_Read32(MDOMAIN_CAP_DDR_In2Status_reg);
		
		if (mdomain_cap_ddr_in2status_Reg.in2_block_select == 1) {
			VtFreezeRegAddr = IoReg_Read32(MDOMAIN_CAP_DDR_In2Addr_reg);
			IoReg_Write32(MDOMAIN_CAP_DDR_In2Addr_reg,IoReg_Read32(MDOMAIN_CAP_DDR_In2_3rdAddr_reg));
			IoReg_Write32(MDOMAIN_CAP_DDR_In2_3rdAddr_reg,VtFreezeRegAddr);

		} else if (mdomain_cap_ddr_in2status_Reg.in2_block_select == 0) {
			VtFreezeRegAddr = IoReg_Read32(MDOMAIN_CAP_DDR_In2_2ndAddr_reg);
			IoReg_Write32(MDOMAIN_CAP_DDR_In2_2ndAddr_reg, IoReg_Read32(MDOMAIN_CAP_DDR_In2_3rdAddr_reg));
			IoReg_Write32(MDOMAIN_CAP_DDR_In2_3rdAddr_reg, VtFreezeRegAddr);
		} 

		if (VtFreezeRegAddr == CaptureCtrl_VT.cap_buffer[0].phyaddr)
			IndexOfFreezedVideoFrameBuffer = 0;
		else if (VtFreezeRegAddr == CaptureCtrl_VT.cap_buffer[1].phyaddr)
			IndexOfFreezedVideoFrameBuffer = 1;
		else
			IndexOfFreezedVideoFrameBuffer = 2;
		
		rtd_pr_vt_notice("IndexOfFreezedVideoFrameBuffer = %d\n", IndexOfFreezedVideoFrameBuffer);
		rtd_pr_vt_notice("MDOMAIN_CAP_DDR_In2Addr_reg = %x\n", IoReg_Read32(MDOMAIN_CAP_DDR_In2Addr_reg));
		rtd_pr_vt_notice("MDOMAIN_CAP_DDR_In2_2ndAddr_reg = %x\n", IoReg_Read32(MDOMAIN_CAP_DDR_In2_2ndAddr_reg));
		rtd_pr_vt_notice("MDOMAIN_CAP_DDR_In2_3rdAddr_reg = %x\n", IoReg_Read32(MDOMAIN_CAP_DDR_In2_3rdAddr_reg));	
	}
	else
		VtFreezeRegAddr = 0;	
}

void sub_mdomain_vi_VtCap_freeze(unsigned char enable)
{
	if(TRUE == enable)
	{ //calculate last capture done buffer,which send to ap
		if(get_vt_VtBufferNum() == 3)
		{
			mdomain_vi_sub_mdom_sub_vi_ads_start_y_RBUS mdomain_vi_sub_mdom_sub_vi_ads_start_y_reg;
		  //mdomain_vi_sub_mdom_sub_vi_ads_start_c_RBUS mdomain_vi_sub_mdom_sub_vi_ads_start_c_reg;
		  
		  unsigned int lastcapbufferaddr = 0;
		
		  mdomain_vi_sub_mdom_sub_vi_ads_start_y_reg.regValue = IoReg_Read32(MDOMAIN_VI_SUB_mdom_sub_vi_ads_start_y_reg);

		  // calculate last capture done buffer 
		  if(mdomain_vi_sub_mdom_sub_vi_ads_start_y_reg.regValue == CaptureCtrl_VT.cap_buffer[0].phyaddr)
			  lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[2].phyaddr;
		  else if(mdomain_vi_sub_mdom_sub_vi_ads_start_y_reg.regValue == CaptureCtrl_VT.cap_buffer[1].phyaddr)
			  lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[0].phyaddr;
		  else if(mdomain_vi_sub_mdom_sub_vi_ads_start_y_reg.regValue == CaptureCtrl_VT.cap_buffer[2].phyaddr)
			  lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[1].phyaddr;

		  // if lastcapbufferaddr is last freezed buffer, re-calculate
		  if(lastcapbufferaddr == VtFreezeRegAddr)
			  {
				 if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr){
						 lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[2].phyaddr;
				 }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr){
						 lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[0].phyaddr;
				 }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[2].phyaddr){
						 lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[1].phyaddr;
				 }
			 }
			
			 if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr){
					 IndexOfFreezedVideoFrameBuffer = 0;
			 }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr){
					 IndexOfFreezedVideoFrameBuffer = 1;
			 }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[2].phyaddr){
					 IndexOfFreezedVideoFrameBuffer = 2;
			 }
			VtFreezeRegAddr = lastcapbufferaddr;
		}
	}
	else
		VtFreezeRegAddr = 0;
}

void set_vfod_freezed_for_i3ddma_sub_capture(unsigned char boffon)
{
	rtd_pr_vt_notice("func:%s,%d,%d\n",__FUNCTION__,get_VT_Pixel_Format(), boffon);

	if ((get_VT_Pixel_Format() == VT_CAP_NV12) || (get_VT_Pixel_Format() == VT_CAP_NV16))
	{
		//if sub mdomain path  capture
		if(get_i3ddma_idlestatus() == FALSE)
			sub_mdomain_vi_VtCap_freeze(boffon);
		else
			i3ddma_cap1cap2_vi_VtCap_freeze(boffon);
	}
	else if ((get_VT_Pixel_Format() == VT_CAP_RGB888)||(get_VT_Pixel_Format() == VT_CAP_ARGB8888)||(get_VT_Pixel_Format() == VT_CAP_ABGR8888))
	{
		if(get_i3ddma_idlestatus() == FALSE)  //sub_mdomain_seq_VtCap_freeze
			sub_mdomain_seq_VtCap_freeze(boffon);
		else
			i3ddma_cap0_seq_VtCap_freeze(boffon);

	}
	else if (get_VT_Pixel_Format() == VT_CAP_YUV888)
	{
		if (boffon == TRUE)
		{
			mdomain_cap_ddr_in2status_RBUS mdomain_cap_ddr_in2status_Reg;
			unsigned int yuvfreezebufferaddr;

			rtd_pr_vt_notice("MDOMAIN_CAP_DDR_In2Status_reg = %x\n", IoReg_Read32(MDOMAIN_CAP_DDR_In2Status_reg));
			//msleep(16);
			//rtd_pr_vt_notice("H3DDMA_I3DDMA_enable_reg = %x\n", IoReg_Read32(H3DDMA_I3DDMA_enable_reg));
			//rtd_pr_vt_notice("h3ddma_cap_status_Reg = %x\n", IoReg_Read32(H3DDMA_Cap_Status_reg));
			mdomain_cap_ddr_in2status_Reg.regValue = rtd_inl(MDOMAIN_CAP_DDR_In2Status_reg);

			yuvfreezebufferaddr = 0;
			if (mdomain_cap_ddr_in2status_Reg.in2_block_select == 1) {
				yuvfreezebufferaddr = IoReg_Read32(MDOMAIN_CAP_DDR_In2Addr_reg);
				IoReg_Write32(MDOMAIN_CAP_DDR_In2Addr_reg,IoReg_Read32(MDOMAIN_CAP_DDR_In2_3rdAddr_reg));
				IoReg_Write32(MDOMAIN_CAP_DDR_In2_3rdAddr_reg,yuvfreezebufferaddr);

			} else if (mdomain_cap_ddr_in2status_Reg.in2_block_select == 0) {
				yuvfreezebufferaddr = IoReg_Read32(MDOMAIN_CAP_DDR_In2_2ndAddr_reg);
				IoReg_Write32(MDOMAIN_CAP_DDR_In2_2ndAddr_reg,IoReg_Read32(MDOMAIN_CAP_DDR_In2_3rdAddr_reg));
				IoReg_Write32(MDOMAIN_CAP_DDR_In2_3rdAddr_reg,yuvfreezebufferaddr);
			}

			if (yuvfreezebufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr)
				IndexOfFreezedVideoFrameBuffer = 0;
			else if (yuvfreezebufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr)
				IndexOfFreezedVideoFrameBuffer = 1;
			else
				IndexOfFreezedVideoFrameBuffer = 2;
			rtd_pr_vt_notice("IndexOfFreezedVideoFrameBuffer = %d\n", IndexOfFreezedVideoFrameBuffer);
			rtd_pr_vt_notice("MDOMAIN_CAP_DDR_In2Addr_reg = %x\n", IoReg_Read32(MDOMAIN_CAP_DDR_In2Addr_reg));
			rtd_pr_vt_notice("MDOMAIN_CAP_DDR_In2_2ndAddr_reg = %x\n", IoReg_Read32(MDOMAIN_CAP_DDR_In2_2ndAddr_reg));
			rtd_pr_vt_notice("MDOMAIN_CAP_DDR_In2_3rdAddr_reg = %x\n", IoReg_Read32(MDOMAIN_CAP_DDR_In2_3rdAddr_reg));

		}
	}

}
#endif

#ifdef CONFIG_HW_SUPPORT_DC2H
	extern int MEMC_Get_DMA_malloc_address(UINT8 status, UINT32 *DMA_release_start, UINT32 *DMA_release_size);
#endif
#endif

unsigned int get_vt_capBuf_Size(void)
{
#ifdef CONFIG_HW_SUPPORT_DC2H
    if((get_VT_Pixel_Format() == VT_CAP_ABGR8888) || (get_VT_Pixel_Format() == VT_CAP_ARGB8888))
        VT_CAPTURE_BUFFER_UNITSIZE = _ALIGN(VT_CAP_FRAME_WIDTH_2K1k*VT_CAP_FRAME_HEIGHT_2K1k*4,__12KPAGE);//mali gpu need 4k align,merlin4 ic need 96 align,so public align is 12k
    else if(get_VT_Pixel_Format() == VT_CAP_NV12)
        VT_CAPTURE_BUFFER_UNITSIZE = _ALIGN((VT_CAP_FRAME_WIDTH_2K1k*VT_CAP_FRAME_HEIGHT_2K1k),__12KPAGE)+_ALIGN((VT_CAP_FRAME_WIDTH_2K1k*VT_CAP_FRAME_HEIGHT_2K1k/2),__12KPAGE);
    else if(get_VT_Pixel_Format() == VT_CAP_NV16)
        VT_CAPTURE_BUFFER_UNITSIZE = _ALIGN(VT_CAP_FRAME_WIDTH_2K1k*VT_CAP_FRAME_HEIGHT_2K1k,__12KPAGE)+_ALIGN(VT_CAP_FRAME_WIDTH_2K1k*VT_CAP_FRAME_HEIGHT_2K1k,__12KPAGE);
    else
        VT_CAPTURE_BUFFER_UNITSIZE = _ALIGN(VT_CAP_FRAME_WIDTH_2K1k*VT_CAP_FRAME_HEIGHT_2K1k*3,__12KPAGE);
#else
    if((get_VT_Pixel_Format() == VT_CAP_ABGR8888)||(get_VT_Pixel_Format() == VT_CAP_ARGB8888))
        VT_CAPTURE_BUFFER_UNITSIZE = _ALIGN(VT_CAP_FRAME_WIDTH_4K2K*VT_CAP_FRAME_HEIGHT_4K2K*4,__4KPAGE);
    else if(get_VT_Pixel_Format() == VT_CAP_NV12)
        VT_CAPTURE_BUFFER_UNITSIZE = _ALIGN((VT_CAP_FRAME_WIDTH_4K2K*VT_CAP_FRAME_HEIGHT_4K2K + VT_CAP_FRAME_WIDTH_4K2K*VT_CAP_FRAME_HEIGHT_4K2K/2),__4KPAGE);
    else if(get_VT_Pixel_Format() == VT_CAP_NV16)
        VT_CAPTURE_BUFFER_UNITSIZE = _ALIGN(VT_CAP_FRAME_WIDTH_4K2K*VT_CAP_FRAME_HEIGHT_4K2K*2,__4KPAGE);
    else
        VT_CAPTURE_BUFFER_UNITSIZE = _ALIGN(VT_CAP_FRAME_WIDTH_4K2K*VT_CAP_FRAME_HEIGHT_4K2K*3,__4KPAGE);
#endif

    return VT_CAPTURE_BUFFER_UNITSIZE;
}

unsigned char Capture_BufferMemInit_VT(unsigned int buffernum)
{
	unsigned int size = 0;
	int i = 0;

#ifdef CONFIG_HW_SUPPORT_DC2H

	unsigned int allocatetotalsize = 0, carvedout_size = 0;
	unsigned int physicalstartaddree = 0;
	unsigned int align96phisicaladdr = 0;
	unsigned int DMA_release_start = 0;
#else
	unsigned int align96mallocphysicaladdr = 0;
	unsigned long mallocphysicaladdr = 0;
#endif

	size = get_vt_capBuf_Size();
	rtd_pr_vt_notice("[VT]size=0x%x\n",size);

#ifndef CONFIG_HW_SUPPORT_DC2H
	for (i = 0; i < buffernum; i++)
	{
		mallocphysicaladdr = pli_malloc(size, GFP_DCU2_FIRST);
		if(mallocphysicaladdr == INVALID_VAL)
		{
			rtd_pr_vt_emerg("[error]VT retry pli malloc fail(%lx)\n", mallocphysicaladdr);
			return FALSE;
		}
		else
			VtAllocatedBufferStartAdress[i] = mallocphysicaladdr;
		align96mallocphysicaladdr = _ALIGN(mallocphysicaladdr,__4KPAGE);
		CaptureCtrl_VT.cap_buffer[i].cache = NULL;
		CaptureCtrl_VT.cap_buffer[i].phyaddr = align96mallocphysicaladdr;
		CaptureCtrl_VT.cap_buffer[i].size = size;
		rtd_pr_vt_notice("[VT memory allocate] buf[%d]=(%d KB), phy(%lx) \n", i, CaptureCtrl_VT.cap_buffer[i].size >> 10, (unsigned long)CaptureCtrl_VT.cap_buffer[i].phyaddr);
	}
	return TRUE;

#else  /* else if define support hw dc2h */

#ifndef CONFIG_MEMC_BYPASS	//non-bypass memc

		unsigned int DMA_release_size;
#endif
		if(buffernum == 1 || buffernum == 3)/*Capture frame case:use cma memory*/
		{
			DMA_release_start = 0;
		} else {/*VR 360 case, borrow MEMC memory*/

#ifndef CONFIG_MEMC_BYPASS	//non-bypass memc

		if(MEMC_Get_DMA_malloc_address(TRUE, &DMA_release_start,&DMA_release_size) == 0)
			rtd_pr_vt_notice("MEMC_Get_DMA_malloc_address TRUE addr:%x size %d \n",DMA_release_start,DMA_release_size);

		if(get_vt_EnableFRCMode() == TRUE)
		{
			DMA_release_start = 0;
		}
#else
#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
		carvedout_size = (unsigned int)carvedout_buf_query_secure(CARVEDOUT_SCALER_MEMC, (void *)&DMA_release_start);
#else
		carvedout_size = (unsigned int)carvedout_buf_query(CARVEDOUT_SCALER_MEMC, (void *)&DMA_release_start);
#endif
		if((DMA_release_start == 0) || (allocatetotalsize > carvedout_size))
		{
			rtd_pr_vt_emerg("[Warning]:query VT carvedout memory failed (%x/%x)\n", DMA_release_start, carvedout_size);
			return FALSE;
		}
#endif

	}
	if(DMA_release_start != 0)
	{
#ifndef CONFIG_MEMC_BYPASS	//non-bypass memc

		physicalstartaddree = DMA_release_start;
		align96phisicaladdr = _ALIGN(physicalstartaddree,__12KPAGE);//mali gpu need start address 4k align,merlin4 ic need 96 align,so public align is 12k

		//size = size +__12KPAGE;
		// vr360 case, vt 1st buffer has allocate from memc ok (borrow 20M)
		if(CaptureCtrl_VT.cap_buffer[0].phyaddr == 0)
		{
			CaptureCtrl_VT.cap_buffer[0].cache = NULL;
			CaptureCtrl_VT.cap_buffer[0].phyaddr =  align96phisicaladdr;
			CaptureCtrl_VT.cap_buffer[0].size = size;
			rtd_pr_vt_notice("[VT memory allocate] buf[0]=(%d KB), phy(%lx) \n", CaptureCtrl_VT.cap_buffer[0].size >> 10, (unsigned long)CaptureCtrl_VT.cap_buffer[0].phyaddr);
		}
		// vr360 case, vt the 2nd~5th buffers allocate from cma
		allocatetotalsize = size + __12KPAGE;

		for(i = 1; i < buffernum; i++)
		{
			physicalstartaddree = pli_malloc(allocatetotalsize, GFP_DCU1_FIRST);
			if(physicalstartaddree == INVALID_VAL)
			{
				rtd_pr_vt_emerg("[error]:VT pli_malloc memory failed,index(%d)(%x)\n", i,physicalstartaddree);
				return FALSE;
			}
			VtAllocatedBufferStartAdress[i] = physicalstartaddree;

			align96phisicaladdr = _ALIGN(physicalstartaddree,__12KPAGE);

			if(CaptureCtrl_VT.cap_buffer[i].phyaddr == 0)
			{
				CaptureCtrl_VT.cap_buffer[i].cache = NULL;
				CaptureCtrl_VT.cap_buffer[i].phyaddr =	align96phisicaladdr;
				CaptureCtrl_VT.cap_buffer[i].size = size;
				rtd_pr_vt_emerg("[VT memory allocate] buf[%d]=(%d KB), phy(%lx) \n", i, CaptureCtrl_VT.cap_buffer[i].size >> 10, (unsigned long)CaptureCtrl_VT.cap_buffer[i].phyaddr);
			}

		}

		/* dcmt monitor for dc2h ------start--DCU1 mem9 ~ DCU1 mem13 ===> index9-13----*/

		for(i = 0; i < buffernum; i++)
		{
			dcmt_monitor_dc2h_range(9+i, CaptureCtrl_VT.cap_buffer[i].phyaddr, CaptureCtrl_VT.cap_buffer[i].size);
		}
		/* dcmt monitor for dc2h ------end------*/

#else

		physicalstartaddree = DMA_release_start;
		align96phisicaladdr = _ALIGN(physicalstartaddree,__12KPAGE);//mali gpu need start address 4k align,merlin4 ic need 96 align,so public align is 12k
		// after bypass memc, assign memory
		for (i = 0; i < buffernum; i++)
		{
			if(CaptureCtrl_VT.cap_buffer[i].phyaddr == 0)
			{
				CaptureCtrl_VT.cap_buffer[i].cache = NULL;
				//CaptureCtrl_VT.cap_buffer[i].phyaddr =  (unsigned int)virt_to_phys((void*)CaptureCtrl_VT.cap_buffer[i].cache);
				CaptureCtrl_VT.cap_buffer[i].phyaddr =  (align96phisicaladdr + i*size);
				CaptureCtrl_VT.cap_buffer[i].size = size;
				rtd_pr_vt_notice("[VT memory allocate][memc all bypass] buf[%d]=(%d KB), phy(%lx) \n", i, CaptureCtrl_VT.cap_buffer[i].size >> 10, (unsigned long)CaptureCtrl_VT.cap_buffer[i].phyaddr);
			}
		}
#endif

	}
	else
	{
		/* carvedout memory from DDR */
		if((buffernum == 1) || (buffernum == 3))
		{
			allocatetotalsize = (size * buffernum +__12KPAGE);

			//#FixMe, 20190912
			//carvedout_size = (unsigned int)carvedout_buf_query(CARVEDOUT_VT, (void *)&physicalstartaddree);

			if ((carvedout_size == 0) || (physicalstartaddree == 0) || (allocatetotalsize > carvedout_size))
			{
				rtd_pr_vt_notice("VT retry pli(%x/%x)\n", physicalstartaddree, carvedout_size);
				/*return FALSE;*/

				/* retry pli malloc */
				physicalstartaddree = pli_malloc(allocatetotalsize, GFP_DCU1_FIRST);
				if(physicalstartaddree == INVALID_VAL)
				{
					rtd_pr_vt_emerg("[error]VT retry pli malloc fail(%x)\n", physicalstartaddree);
					return FALSE;
				}
				else
					VtAllocatedBufferStartAdress[0] = physicalstartaddree;
			}

			align96phisicaladdr = _ALIGN(physicalstartaddree,__12KPAGE);
			for (i = 0; i < buffernum; i++)
			{
				if(CaptureCtrl_VT.cap_buffer[i].phyaddr == 0)
				{
					CaptureCtrl_VT.cap_buffer[i].cache = NULL;
					CaptureCtrl_VT.cap_buffer[i].phyaddr =	(align96phisicaladdr + i*size);
					CaptureCtrl_VT.cap_buffer[i].size = size;
					rtd_pr_vt_emerg("[VT memory allocate] buf[%d]=(%d KB), phy(%lx) \n", i, CaptureCtrl_VT.cap_buffer[i].size >> 10, (unsigned long)CaptureCtrl_VT.cap_buffer[i].phyaddr);
				}
			}
		}
		else
		{
			allocatetotalsize = size + __12KPAGE;
			/* vr360 case: vt 5 buffers allocate memory from cam by call pli_malloc 5 times ----start */
			for (i = 0; i < buffernum; i++)
			{
				physicalstartaddree = pli_malloc(allocatetotalsize, GFP_DCU1_FIRST);
				if(physicalstartaddree == INVALID_VAL)
				{
					rtd_pr_vt_emerg("[error]:VT pli_malloc memory failed,index(%d)(%x)\n", i,physicalstartaddree);
					return FALSE;
				}
				VtAllocatedBufferStartAdress[i] = physicalstartaddree;

				align96phisicaladdr = _ALIGN(physicalstartaddree,__12KPAGE);

				CaptureCtrl_VT.cap_buffer[i].cache = NULL;
				CaptureCtrl_VT.cap_buffer[i].phyaddr =	align96phisicaladdr;
				CaptureCtrl_VT.cap_buffer[i].size = size;
				rtd_pr_vt_emerg("[VT memory allocate][5] buf[%d]=(%d KB), phy(%lx) \n", i, CaptureCtrl_VT.cap_buffer[i].size >> 10, (unsigned long)CaptureCtrl_VT.cap_buffer[i].phyaddr);
			}
			/* vr360 case: vt 5 buffers allocate memory from cam by call pli_malloc 5 times ----end */

			/* dcmt monitor for dc2h ------start--DCU1 mem9 ~ DCU1 mem13 ===> index9-13----*/

			for(i = 0; i < buffernum; i++)
			{
				dcmt_monitor_dc2h_range(9+i, CaptureCtrl_VT.cap_buffer[i].phyaddr, CaptureCtrl_VT.cap_buffer[i].size);
			}
			/* dcmt monitor for dc2h ------end------*/
		}
	}
	return TRUE;
#endif
}

void Capture_BufferMemDeInit_VT(unsigned int buffernum)
{
	int i;

#ifdef CONFIG_HW_SUPPORT_DC2H
#ifndef CONFIG_MEMC_BYPASS	//no-bypass memc

	if(buffernum == 5)
	{
	/* dcmt clear monitor for dc2h ------start--DCU1 mem9 ~ DCU1 mem13 ===> index9-13----*/
		for(i = 0; i < buffernum; i++)
			dcmt_clear_monitor(9+i);
	}

#endif
#endif
	for (i = 0; i < buffernum; i++)
	{
		if(VtAllocatedBufferStartAdress[i])
		{
			pli_free(VtAllocatedBufferStartAdress[i]);
			VtAllocatedBufferStartAdress[i] = 0;
		}
	}

#if 0
	if(VtBlockCapFlag == TRUE)
	{
		if (CaptureCtrl_VT.cap_buffer[0].cache)
		{
			dvr_free(CaptureCtrl_VT.cap_buffer[0].cache);
			rtd_pr_vt_notice("Block capture free memory done!\n");
			VtBlockCapFlag = FALSE;
		}
	}
	else
	{
		for (i = 0; i < buffernum; i++)
		{
			if (CaptureCtrl_VT.cap_buffer[i].cache)
			{
				dvr_free(CaptureCtrl_VT.cap_buffer[i].cache);
			}
		}
	}
#endif


	for (i = 0; i < buffernum; i++)
	{
		if (CaptureCtrl_VT.cap_buffer[i].phyaddr)
		{
			CaptureCtrl_VT.cap_buffer[i].cache = NULL;
			CaptureCtrl_VT.cap_buffer[i].phyaddr = (unsigned long)NULL;
			CaptureCtrl_VT.cap_buffer[i].size = 0;
		}
		rtd_pr_vt_debug("[MemDeInit_VT]VtAllocatedBufferStartAdress[%d]= %ld\n", i, VtAllocatedBufferStartAdress[i]);
	}

	rtd_pr_vt_debug("[VT memory free] Deinit done!\n");
}

#ifndef UT_flag

#ifndef CONFIG_HW_SUPPORT_DC2H 

#define _BURSTLENGTH2  (0x78)  // 0x78 (120) for sub-channel

/* scalering down table*/
extern DRV_ScalingDown_COEF_TAB ScaleDown_COEF_TAB;
static unsigned char SD_H_Coeff_Sel[] = {SDFIR64_Blur, SDFIR64_Mid, SDFIR64_Sharp, SDFIR64_2tap}; /*the size must match large table*/
static unsigned char SD_V_Coeff_Sel[] = {SDFIR64_Blur, SDFIR64_Mid, SDFIR64_Sharp, SDFIR64_2tap}; /*the size must match large table*/

#if 0
static unsigned short tRGB2YUV_COEF_709_RGB_0_255[] =
{
    // CCIR 709 RGB
#if 1 // for 0~255
    0x0131,  // m11[10:2] U(9,8)
    0x025c,  // m12   [20:13] U(8,8)
    0x0074,  // m13 [31:23] U(9,8)
#else // for 16~235
    0x0163,  // m11
    0x02bf,   // m12
    0x0087,  // m13
#endif
    0x03D5,   // m21 >> 2 0x01D5 up bits at AT Name: cychen2 , Date: 2010/3/25
    0x0357,   // m22 >> 1
    0x0080,   // m23 >> 2
    0x0080,   // m31 >> 2
    0x0795,   // m32 >> 2 0x0195  up bits at AT Name: cychen2 , Date: 2010/3/25
    0x03D7,   // m33 >> 1
    0x0000,    // Yo_even
    0x0000,    // Yo_odd
    0x0100,    // Y_gain
    0x0001,  // sel_RGB
    0x0000,  // sel_Yin_offset
    0x0001,  // sel_UV_out_offset
    0x0000,  // sel_UV_off
    0x0000,  // Matrix_bypass
    0x0001,  // Enable_Y_gain
};
#endif

unsigned int memory_get_line_size_subvt(unsigned int LineSize, unsigned char InputFormat, unsigned char BitNum)
{

	if(InputFormat){
		if(BitNum) {		// 4:4:4 format     10bit
			LineSize = drvif_memory_get_data_align(LineSize *30, 64);
			LineSize = (unsigned int)SHR(LineSize, 6);	// (pixels of per line * 30 / 64), unit: 64bits
			rtd_pr_vt_debug("\n 444_10 bits !");
		} else {			// 4:4:4 format     8bit
			LineSize = drvif_memory_get_data_align(LineSize*24, 64);
			LineSize = (unsigned int)SHR(LineSize, 6);	// (pixels of per line * 24 / 64), unit: 64bits
			rtd_pr_vt_debug("\n 444_8 bits !");
		}
	}else{
		if(BitNum) {		// 4:2:2 format     10bit
			LineSize = drvif_memory_get_data_align(LineSize*20, 64);
			LineSize = (unsigned int)SHR(LineSize, 6);	// (pixels of per line * 20 / 64), unit: 64bits
			rtd_pr_vt_debug("\n 422_10 bits !");
		} else {			// 4:2:2 format     8bit
			LineSize = drvif_memory_get_data_align(LineSize*16, 64);
			LineSize = (unsigned int)SHR(LineSize, 6);	// (pixels of per line * 16 / 64), unit: 64bits
			rtd_pr_vt_debug("\n 422_8 bits !");
		}
	}
	rtd_pr_vt_notice("[VT]memory_get_line_size LineSize=%d\n",LineSize);
	return LineSize;
}

unsigned int memory_get_capture_size_subvt(unsigned int capWidth, unsigned int capLenth, eMemCapAccessType AccessType)
{
	unsigned int TotalSize;
	unsigned int capLen = capLenth;
	unsigned int capWid = capWidth;

	TotalSize = memory_get_line_size_subvt(capWid, 0, 0);//444;8 bit
	TotalSize = drvif_memory_get_data_align(TotalSize, 0x04);	// it must be times of 4

	if (AccessType == MEMCAPTYPE_FRAME)
		TotalSize = TotalSize * capLen;	// multiple total_lines if frame-access
	return TotalSize;
}

void memory_set_subcapture_vt(unsigned char bDouble, eMemCapAccessType Cap_type, unsigned int capWid, unsigned int capLen)
{
	unsigned int  TotalSize;
	unsigned int  Quotient;
	unsigned char  Remainder;
	unsigned int blockstep;//need to be even
	unsigned int burstLen = _BURSTLENGTH2;
	unsigned int buffer_size = 0;
	unsigned int double_buf_flag = 0, triple_buf_flag = 0;
	unsigned char Sts;
	mdomain_cap_capture2_byte_swap_RBUS capture2_byte_swap_reg;
	mdomain_cap_sub_sdnr_cxxto64_RBUS mdomain_cap_sub_sdnr_cxxto64_reg;
	mdomain_cap_ddr_in2ctrl_RBUS mdomain_cap_ddr_in2ctrl_reg;
	mdomain_cap_cap_ddr_fifostatus_RBUS cap_ddr_fifostatus_reg;

	//field polarity error check
	Sts = IoReg_ReadWord0(/*channel ? MDOMAIN_CAP_DDR_In2Status_reg : */MDOMAIN_CAP_DDR_In1Status_reg);
	if (Sts & _BIT4) {	//Field polarity Error bit
		IoReg_ClearBits(/*channel ? MDOMAIN_CAP_DDR_In2Status_reg : */MDOMAIN_CAP_DDR_In1Status_reg, _BIT4);
	}


	mdomain_cap_sub_sdnr_cxxto64_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Sub_SDNR_cxxto64_reg);
	mdomain_cap_sub_sdnr_cxxto64_reg.sub_sort_fmt = 1;//0:422,1:444
	mdomain_cap_sub_sdnr_cxxto64_reg.sub_out_bit = 0;//sub only support 8 bit
	IoReg_Write32(MDOMAIN_CAP_Sub_SDNR_cxxto64_reg, mdomain_cap_sub_sdnr_cxxto64_reg.regValue);

	TotalSize = memory_get_capture_size_subvt(capWid, capLen, Cap_type);		// calculate the memory size of capture

	if(Cap_type == MEMCAPTYPE_LINE){

		IoReg_Write32(MDOMAIN_CAP_DDR_In2LineStep_reg, (TotalSize + TotalSize%2) << 3);//bit 3 need to be 0. rbus rule
		blockstep = (TotalSize*capLen);
		blockstep = (blockstep % 2)? blockstep + 1: blockstep;
		IoReg_Write32(MDOMAIN_CAP_DDR_In2BlkStep_reg, blockstep << 3);//bit 3 need to be 0. rbus rule

	}else{
		IoReg_Write32(MDOMAIN_CAP_DDR_In2LineStep_reg, 0);
	}

	memory_division(TotalSize, burstLen, &Quotient, &Remainder);	// Do a division

	Remainder = drvif_memory_get_data_align(Remainder, 4);	// multiple of 4

	if (Remainder == 0) {	// remainder is not allowed to be zero
		Remainder = burstLen;
		Quotient -= 1;
	}

	//memory_set_capture_fifo(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL), Quotient, Remainder, burstLen);	// set capture FIFO
	IoReg_Write32(MDOMAIN_CAP_DDR_In2WTLVL_reg, ((burstLen>>1) << 16) | Quotient);
	IoReg_Write32(MDOMAIN_CAP_DDR_In2WrLen_Rem_reg, (burstLen << 8) | Remainder);

	TotalSize = SHL(Quotient * burstLen + Remainder, 3); //unit conversion. 64bits to 8bits

	if(Cap_type == MEMCAPTYPE_LINE){
		TotalSize *=capLen;

	}
	if(bDouble)
	{
		IoReg_Write32(MDOMAIN_CAP_DDR_In2_2ndAddr_reg, CaptureCtrl_VT.cap_buffer[1].phyaddr & 0x7ffffff0);
	}

	IoReg_Write32(MDOMAIN_CAP_DDR_In2Addr_reg, CaptureCtrl_VT.cap_buffer[0].phyaddr & 0x7ffffff0);
	rtd_pr_vt_debug("###################sub buffer TotalSize=%d;;\n", buffer_size);

	/*restore buffer freeze use at MDOMAIN_CAP_DDR_In2_3rdAddr_reg*/
	IoReg_Write32(MDOMAIN_CAP_DDR_In2_3rdAddr_reg, CaptureCtrl_VT.cap_buffer[2].phyaddr & 0x7ffffff0);

	IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr_BLK0_reg, (CaptureCtrl_VT.cap_buffer[0].phyaddr & 0x7ffffff0));
	IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr2_BLK0_reg, (CaptureCtrl_VT.cap_buffer[0].phyaddr+CaptureCtrl_VT.cap_buffer[0].size) & 0x7ffffff0);
	IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr_BLK1_reg, (CaptureCtrl_VT.cap_buffer[1].phyaddr & 0x7ffffff0));
	IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr2_BLK1_reg, (CaptureCtrl_VT.cap_buffer[1].phyaddr+CaptureCtrl_VT.cap_buffer[0].size) & 0x7ffffff0);

	IoReg_Mask32(MDOMAIN_CAP_DDR_In2LineNum_reg, 0xfffee000, (Cap_type<<16)|AND(capLen, 0x1FFF));

	if(bDouble){
		double_buf_flag = 1;
		triple_buf_flag = 0;
	}
	else{
		double_buf_flag = 0;
		triple_buf_flag = 0;
	}

	//Use register structure to set
	mdomain_cap_ddr_in2ctrl_reg.regValue = IoReg_Read32(MDOMAIN_CAP_DDR_In2Ctrl_reg);
	if(triple_buf_flag == 1){//triple buffer case
		mdomain_cap_ddr_in2ctrl_reg.in2_en_vs_rst = 1;
		mdomain_cap_ddr_in2ctrl_reg.in2_double_enable = 1;
		mdomain_cap_ddr_in2ctrl_reg.in2_v_flip_3buf_en = 1;
		mdomain_cap_ddr_in2ctrl_reg.in2_bufnum = 2; // 3-buf mode
		mdomain_cap_ddr_in2ctrl_reg.in2_disp2cap_distance = 1;
	}
	else if((double_buf_flag == 1) && (triple_buf_flag == 0)){//double buffer case
		mdomain_cap_ddr_in2ctrl_reg.in2_en_vs_rst = 1;
		mdomain_cap_ddr_in2ctrl_reg.in2_double_enable = 1;
		mdomain_cap_ddr_in2ctrl_reg.in2_v_flip_3buf_en = 0;
		mdomain_cap_ddr_in2ctrl_reg.in2_bufnum = 0;
		mdomain_cap_ddr_in2ctrl_reg.in2_disp2cap_distance = 0;
	}
	else{//single buffer
		mdomain_cap_ddr_in2ctrl_reg.in2_en_vs_rst = 1;
		mdomain_cap_ddr_in2ctrl_reg.in2_double_enable = 0;
		mdomain_cap_ddr_in2ctrl_reg.in2_v_flip_3buf_en = 0;
		mdomain_cap_ddr_in2ctrl_reg.in2_bufnum = 0;
		mdomain_cap_ddr_in2ctrl_reg.in2_disp2cap_distance = 0;
	}
	IoReg_Write32(MDOMAIN_CAP_DDR_In2Ctrl_reg, mdomain_cap_ddr_in2ctrl_reg.regValue);

	//frank@03132013 add below code for GPU access YUV format ++
	capture2_byte_swap_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Capture2_byte_swap_reg);
	capture2_byte_swap_reg.in2_1byte_swap = 1;
	capture2_byte_swap_reg.in2_2byte_swap = 1;
	capture2_byte_swap_reg.in2_4byte_swap = 1;
	capture2_byte_swap_reg.in2_8byte_swap = 0;
	IoReg_Write32(MDOMAIN_CAP_Capture2_byte_swap_reg, capture2_byte_swap_reg.regValue);
	//frank@03132013 add below code for GPU access YUV format --
	IoReg_SetBits(MDOMAIN_CAP_Cap_In2_enable_reg, _BIT0);

	/*wclr cap buf done status*/
	cap_ddr_fifostatus_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg);
	cap_ddr_fifostatus_reg.in1_cap_last_wr_flag = 1;
	IoReg_Write32(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg, cap_ddr_fifostatus_reg.regValue);
}

void drvif_vt_color_ultrazoom_set_scale_down(SIZE *ptInSize, SIZE *ptOutSize)
{
	unsigned char SDRatio;
	unsigned char SDFilter = 0;
	signed short *coef_pt;
	unsigned int nFactor;
	unsigned char Hini, Vini;
	unsigned long long tmpLength = 0;
	scaledown_ich2_uzd_ctrl0_RBUS				ich2_uzd_Ctrl0_REG;
	scaledown_ich2_uzd_ctrl1_RBUS				ich2_uzd_Ctrl1_REG;
	scaledown_ich2_uzd_h_output_size_RBUS		ich2_uzd_H_Output_Size_REG;
	scaledown_ich2_uzd_scale_hor_factor_RBUS 	ich2_uzd_Scale_Hor_Factor_REG;
	scaledown_ich2_uzd_scale_ver_factor_RBUS	ich2_uzd_Scale_Ver_Factor_REG;
	scaledown_ich2_uzd_initial_value_RBUS			ich2_uzd_Initial_Value_REG;



	ich2_uzd_Ctrl0_REG.regValue		= IoReg_Read32(SCALEDOWN_ICH2_UZD_Ctrl0_reg);
	ich2_uzd_Ctrl1_REG.regValue		= IoReg_Read32(SCALEDOWN_ICH2_UZD_Ctrl1_reg);
	ich2_uzd_H_Output_Size_REG.regValue = IoReg_Read32(SCALEDOWN_ICH2_UZD_H_Output_Size_reg);
	ich2_uzd_Initial_Value_REG.regValue	= IoReg_Read32(SCALEDOWN_ICH2_UZD_Initial_Value_reg);

#define TMPMUL	(16)

/*o============H scale-down=============o*/
	if (ptInSize->nWidth > ptOutSize->nWidth) {
		/*o-------calculate scaledown ratio to select one of different bandwith filters.--------o*/
#if 0
		SDRatio = ptOutSize->nWidth * TMPMUL / ptInSize->nWidth;
		if (SDRatio > (TMPMUL * 2))	
			SDFilter = 2;
		else if (SDRatio <= (TMPMUL * 2) && SDRatio > (TMPMUL * 3 / 2))/*Mid*/
			SDFilter = 1;
		else		/*blurest, narrow bw*/
			SDFilter = 0;
#endif
		if (ptOutSize->nWidth == 0) {
			rtd_pr_vt_notice("output width = 0 !!!\n");
			SDRatio = 0;
		} else {
			SDRatio = (ptInSize->nWidth*TMPMUL) / ptOutSize->nWidth;
		}

		/*rtd_pr_vt_info("CSW SDRatio number =%d\n", SDRatio);*/

		if (SDRatio <= ((TMPMUL * 3)/2))	
			SDFilter = 2;
		else if (SDRatio <= (TMPMUL * 2) && SDRatio > ((TMPMUL * 3)/2))	/*Mid*/
			SDFilter = 1;
		else		/*blurest, narrow bw*/
			SDFilter = 0;

		/*o---------------- fill coefficients into access port--------------------o*/
		/*coef_pt = tScaleDown_COEF_TAB[SD_H_Coeff_Sel[SDFilter]];*/
		coef_pt = &(ScaleDown_COEF_TAB.FIR_Coef_Table[SD_H_Coeff_Sel[SDFilter]][0]);
		/*DC Value Check*/
		//if (SCALING_LOG) {
			//for(i=0; i<8; i++) {
				//if((*(coef_pt+i)+*(coef_pt+i+16)+*(coef_pt+31-i)+*(coef_pt+15-i)) != 1024)
					//rtd_pr_vt_notice("[Warnning] [%s-%s-%d] DC Not Balance!, i=%d\n", __FILE__, __func__, __LINE__, i);
			//}
		//}

		ich2_uzd_Ctrl1_REG.h_c_filter = (SDFilter+1)%3;
		ich2_uzd_Ctrl1_REG.h_y_filter = ich2_uzd_Ctrl1_REG.h_c_filter;

	}
/*o============V scale-down=============o*/
	if (ptInSize->nLength > ptOutSize->nLength) {
		/*o-------calculate scaledown ratio to select one of different bandwith filters.--------o*/
#if 0
		SDRatio = ptOutSize->nLength*TMPMUL / ptInSize->nLength;
		if (SDRatio > (TMPMUL * 2))	
			SDFilter = 2;
		else if (SDRatio <= (TMPMUL * 2) && SDRatio > (TMPMUL * 3 / 2))	/*Mid*/
			SDFilter = 1;
		else		/*blurest, narrow bw*/
			SDFilter = 0;
#endif
		 /*jeffrey 961231*/
		if (ptOutSize->nLength == 0) {
			SDRatio = 0;
		} else {
			SDRatio = (ptInSize->nLength*TMPMUL) / ptOutSize->nLength;
		}

		/*rtd_pr_vt_info("CSW SDRatio number =%d\n", SDRatio);*/

		if (SDRatio <= ((TMPMUL * 3) / 2))	
			SDFilter = 2;
		else if (SDRatio <= (TMPMUL * 2) && SDRatio > ((TMPMUL * 3) / 2))	/*Mid*/
			SDFilter = 1;
		else		/*blurest, narrow bw*/
			SDFilter = 0;


		rtd_pr_vt_notice("filter number =%d\n", SDFilter);


		/*o---------------- fill coefficients into access port--------------------o*/
		/*coef_pt = tScaleDown_COEF_TAB[SD_V_Coeff_Sel[SDFilter]];*/
		coef_pt = &(ScaleDown_COEF_TAB.FIR_Coef_Table[SD_V_Coeff_Sel[SDFilter]][0]);
		/*DC Value Check*/
		//if (SCALING_LOG) {
			//for(i=0; i<8; i++) {
				//if((*(coef_pt+i)+*(coef_pt+i+16)+*(coef_pt+31-i)+*(coef_pt+15-i)) != 1024)
					//rtd_pr_vt_notice("[Warnning] [%s-%s-%d] DC Not Balance!, i=%d\n", __FILE__, __func__, __LINE__, i);
			//}
		//}
	} 

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

	/*Decide Output data format for scale down*/
	if (ptInSize->nWidth > ptOutSize->nWidth) {    /*H scale-down*/
		Hini = 0xff;/*0x78;*/
		ich2_uzd_Initial_Value_REG.hor_ini = Hini;
		{
			/*frank@0108 add for code exception*/
			if (ptOutSize->nWidth == 0) {
				rtd_pr_vt_notice("output width = 0 !!!\n");
				nFactor = 0;
			} else if(ptInSize->nWidth>4095){
				nFactor = (unsigned int)((((ptInSize->nWidth-1)<<19) / (ptOutSize->nWidth-1))<<1);
			}
			else if(ptInSize->nWidth>2047){
				nFactor = (unsigned int)(((ptInSize->nWidth-1)<<20) / (ptOutSize->nWidth-1));
			}
			else {
				nFactor = (unsigned int)((ptInSize->nWidth<<21)) / (ptOutSize->nWidth);
				nFactor = SHR(nFactor + 1, 1); //rounding
			}
		}

		ich2_uzd_Scale_Hor_Factor_REG.hor_fac = nFactor;
	} else {
		ich2_uzd_Scale_Hor_Factor_REG.hor_fac = 0x100000;
	}
	ich2_uzd_H_Output_Size_REG.h_output_size = ptOutSize->nWidth;

	if (ptInSize->nLength > ptOutSize->nLength) {    /*V scale-down*/
/*			ich2_uzd_Ctrl0_REG.buffer_mode = 0x02;*/
		Vini = 0xff;/*0x78;*/
		ich2_uzd_Initial_Value_REG.ver_ini = Vini;
		/*nFactor = (unsigned int)((ptInSize->nLength << 20) - (Vini<<12)) / (ptOutSize->nLength - 1);*/
		/*frank@0108 add for code exception*/
		if (ptOutSize->nLength == 0) {
			rtd_pr_vt_notice("output length = 0 !!!\n");
			nFactor = 0;
		} else {
			if((ptInSize->nLength>=1080) && (ptOutSize->nLength<=544)){
			//frank@1018 sync with pacific to solve 3d vide 1920X2205 scaler down fail
			//nFactor = (unsigned int)(((ptInSize->nLength-1)<<21) / (ptOutSize->nLength-1));
				//nFactor = (unsigned int)(((unsigned long long)(ptInSize->nLength-1)<<21) / (ptOutSize->nLength-1));
				tmpLength = ((unsigned long long)(ptInSize->nLength-1)<<21);
				do_div(tmpLength, (ptOutSize->nLength-1));
				nFactor = tmpLength;
			}
			else{
				//frank@1018 sync with pacific to solve 3d vide 1920X2205 scaler down fail
				//nFactor = (unsigned int)(((ptInSize->nLength)<<21) / (ptOutSize->nLength));
				//nFactor = (unsigned int)(((unsigned long long)(ptInSize->nLength)<<21) / (ptOutSize->nLength));
				tmpLength = ((unsigned long long)(ptInSize->nLength)<<21);
				do_div(tmpLength, (ptOutSize->nLength));
				nFactor = tmpLength;
			}
		}
		nFactor = SHR(nFactor + 1, 1); /*rounding*/
		ich2_uzd_Scale_Ver_Factor_REG.ver_fac = nFactor;
	} else {
/*			ich2_uzd_Ctrl0_REG.buffer_mode = 0;*/
		ich2_uzd_Scale_Ver_Factor_REG.ver_fac = 0x100000;
	}

/*	ich2_uzd_Ctrl0_REG.output_fmt = 1;*/
/*	CLR_422_Fr_SD();*/

	IoReg_Write32(SCALEDOWN_ICH2_UZD_Ctrl0_reg, ich2_uzd_Ctrl0_REG.regValue);
	IoReg_Write32(SCALEDOWN_ICH2_UZD_Ctrl1_reg, ich2_uzd_Ctrl1_REG.regValue);
	IoReg_Write32(SCALEDOWN_ICH2_UZD_H_Output_Size_reg, ich2_uzd_H_Output_Size_REG.regValue);
	IoReg_Write32(SCALEDOWN_ICH2_UZD_Scale_Hor_Factor_reg, ich2_uzd_Scale_Hor_Factor_REG.regValue);
	IoReg_Write32(SCALEDOWN_ICH2_UZD_Scale_Ver_Factor_reg, ich2_uzd_Scale_Ver_Factor_REG.regValue);
	IoReg_Write32(SCALEDOWN_ICH2_UZD_Initial_Value_reg, ich2_uzd_Initial_Value_REG.regValue);
}

unsigned char force_enable_vtdatapath_clock(void)
{
	unsigned char need_update = FALSE;
	vgip_data_path_select_RBUS data_path_select_reg;
	data_path_select_reg.regValue = IoReg_Read32(VGIP_Data_Path_Select_reg);

	if(data_path_select_reg.uzd1_in_sel == 0)
	{
		rtd_pr_vt_notice("VT[warning] NO DI source, Capture will fail!;%s=%d \n", __FUNCTION__, __LINE__);
		return FALSE;
	}	

	if(data_path_select_reg.xcdi_clk_en == 0)
	{
		need_update = TRUE;
		data_path_select_reg.xcdi_clk_en = 1;
	}

	if(data_path_select_reg.s2_clk_en == 0)
	{
		need_update = TRUE;
		data_path_select_reg.s2_clk_en = 1;
	}

	if(data_path_select_reg.uzd2_in_sel != 1)
	{
		need_update = TRUE;
		data_path_select_reg.uzd2_in_sel = 1;
	}	

	if(need_update)
		IoReg_Write32(VGIP_Data_Path_Select_reg, data_path_select_reg.regValue);
	return TRUE;
}

void drvif_vt_check_subvgip_data_dclk(unsigned char enable)
{
	sub_vgip_vgip_chn2_ctrl_RBUS vgip_chn2_ctrl_reg;
	vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
	if(enable == TRUE)
	{
		vgip_chn2_ctrl_reg.ch2_ivrun = 1;
		vgip_chn2_ctrl_reg.ch2_in_clk_en= 1;
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_CTRL_reg, vgip_chn2_ctrl_reg.regValue);
		mdelay(2);
		rtd_pr_vt_notice("VT Enable Sub VGIP Data.Dclk!;%s=%d \n", __FUNCTION__, __LINE__);
	} else {
		vgip_chn2_ctrl_reg.ch2_ivrun = 0;
		vgip_chn2_ctrl_reg.ch2_in_clk_en= 0;
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_CTRL_reg, vgip_chn2_ctrl_reg.regValue);
		mdelay(2);
		rtd_pr_vt_notice("VT Disable Sub VGIP Data.Dclk!;%s=%d \n", __FUNCTION__, __LINE__);

	}		
}

void config_sub_vgip_for_Vt_capture(void)
{
	sub_vgip_vgip_chn2_double_buffer_ctrl_RBUS sub_vgip_vgip_chn2_double_buffer_ctrl_Reg;
	sub_vgip_vgip_chn2_ctrl_RBUS vgip_chn2_ctrl_reg;
	sub_vgip_vgip_chn2_act_hsta_width_RBUS vgip_chn2_act_hsta_width_reg;
	sub_vgip_vgip_chn2_act_vsta_length_RBUS vgip_chn2_act_vsta_length_reg;
	sub_vgip_vgip_chn2_delay_RBUS sub_vgip_vgip_chn2_delay_reg;
	sub_vgip_vgip_chn2_misc_RBUS vgip_chn2_misc_reg;
	
	vgip_data_path_select_RBUS data_path_select_reg;
	vgip_vgip_chn1_misc_RBUS vgip_chn1_misc_reg;
	vgip_vgip_chn1_ctrl_RBUS vgip_chn1_ctrl_reg;
	
	iedge_smooth_format_convert_RBUS main_sub_iedge_smooth_format_convert_reg;
	iedge_smooth_edsm_db_ctrl_RBUS iedge_smooth_edsm_db_ctrl_reg;
	//unsigned char need_update = FALSE;
	data_path_select_reg.regValue = IoReg_Read32(VGIP_Data_Path_Select_reg);
	if(data_path_select_reg.uzd1_in_sel == 1)
	{	
		data_path_select_reg.uzd2_in_sel = 1;
		data_path_select_reg.s2_clk_en = 1;
		data_path_select_reg.xcdi_clk_en = 1;
		data_path_select_reg.di_i_clk_en =1;
		vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
		//vgip_chn2_ctrl_reg.ch2_ivrun = 1;
		vgip_chn2_ctrl_reg.ch2_in_clk_en = 1;
		IoReg_Write32(VGIP_Data_Path_Select_reg, data_path_select_reg.regValue);
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_CTRL_reg, vgip_chn2_ctrl_reg.regValue);
		rtd_pr_vt_notice("go rtnr/di:SUB_VGIP_VGIP_CHN2_CTRL_reg = %x\n", IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg));	
	}	
	else if(data_path_select_reg.uzd1_in_sel == 0)
	{
		/*close sub vgip double buffer*/
		sub_vgip_vgip_chn2_double_buffer_ctrl_Reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_DOUBLE_BUFFER_CTRL_reg);
		sub_vgip_vgip_chn2_double_buffer_ctrl_Reg.ch2_db_en = 0;
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_DOUBLE_BUFFER_CTRL_reg, sub_vgip_vgip_chn2_double_buffer_ctrl_Reg.regValue);

		// Disable SRC_Clock_Enbale bit
		vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
		vgip_chn2_ctrl_reg.ch2_in_clk_en = 0;
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_CTRL_reg, vgip_chn2_ctrl_reg.regValue);

		vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg);
		
		vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
		vgip_chn2_ctrl_reg.ch2_in_sel = vgip_chn1_ctrl_reg.ch1_in_sel;
		vgip_chn2_ctrl_reg.ch2_digital_mode = vgip_chn1_ctrl_reg.ch1_digital_mode;
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_CTRL_reg, vgip_chn2_ctrl_reg.regValue);

		vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);

		//sub vgip size sync from main
		vgip_chn2_act_hsta_width_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_ACT_HSTA_Width_reg);
		if(vgip_chn2_ctrl_reg.ch2_digital_mode == 1)
			vgip_chn2_act_hsta_width_reg.ch2_ih_act_sta = 0;
		else
			vgip_chn2_act_hsta_width_reg.ch2_ih_act_sta = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPH_ACT_STA);

		vgip_chn2_act_hsta_width_reg.ch2_ih_act_wid = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPH_ACT_WID);
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_ACT_HSTA_Width_reg, vgip_chn2_act_hsta_width_reg.regValue);

		vgip_chn2_act_vsta_length_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_ACT_VSTA_Length_reg);
		if(vgip_chn2_ctrl_reg.ch2_digital_mode == 1)
			vgip_chn2_act_vsta_length_reg.ch2_iv_act_sta = 0;
		else
			vgip_chn2_act_vsta_length_reg.ch2_iv_act_sta = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_STA);
		
		vgip_chn2_act_vsta_length_reg.ch2_iv_act_len = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN);
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_ACT_VSTA_Length_reg, vgip_chn2_act_vsta_length_reg.regValue);

		//vgip ch2 prochnum
		vgip_chn1_misc_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_MISC_reg);
		vgip_chn2_misc_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_MISC_reg);

		vgip_chn2_misc_reg.ch2_hporch_num = vgip_chn1_misc_reg.ch1_hporch_num;
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_MISC_reg, vgip_chn2_misc_reg.regValue);
		
		
		sub_vgip_vgip_chn2_delay_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_DELAY_reg);
		sub_vgip_vgip_chn2_delay_reg.ch2_ihs_dly = 0;//reset 0
		sub_vgip_vgip_chn2_delay_reg.ch2_ivs_dly = 0;//reset 0
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_DELAY_reg, sub_vgip_vgip_chn2_delay_reg.regValue);

		
		vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
		vgip_chn2_ctrl_reg.ch2_field_det_en = _DISABLE;		//bit16
		vgip_chn2_ctrl_reg.ch2_field_sync_edge = _ENABLE;	//bit17
		vgip_chn2_ctrl_reg.ch2_field_inv = _DISABLE;		//bit18
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_CTRL_reg, vgip_chn2_ctrl_reg.regValue);

		// enable SRC_Clock_Enbale bit
		vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
		vgip_chn2_ctrl_reg.ch2_ivrun = 1;
		vgip_chn2_ctrl_reg.ch2_in_clk_en= 1;
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_CTRL_reg, vgip_chn2_ctrl_reg.regValue);
		mdelay(2);
		rtd_pr_vt_notice("not go rtnr/di:SUB_VGIP_VGIP_CHN2_CTRL_reg = %x\n", IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg));
	}
	
 //disable sub 422 to 444	
	iedge_smooth_edsm_db_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_EDSM_DB_CTRL_reg);
	main_sub_iedge_smooth_format_convert_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_FORMAT_CONVERT_reg);
	if(main_sub_iedge_smooth_format_convert_reg.sub_422_to_444_en)
	{
		if(iedge_smooth_edsm_db_ctrl_reg.edsm_db_en) //db on
		{
			iedge_smooth_edsm_db_ctrl_reg.edsm_db_en = 0;
			IoReg_Write32(IEDGE_SMOOTH_EDSM_DB_CTRL_reg, iedge_smooth_edsm_db_ctrl_reg.regValue);

			main_sub_iedge_smooth_format_convert_reg.sub_422_to_444_en = 0;
			IoReg_Write32(IEDGE_SMOOTH_FORMAT_CONVERT_reg, main_sub_iedge_smooth_format_convert_reg.regValue);
		}
		else
		{			
			main_sub_iedge_smooth_format_convert_reg.sub_422_to_444_en = 0;
			IoReg_Write32(IEDGE_SMOOTH_FORMAT_CONVERT_reg, main_sub_iedge_smooth_format_convert_reg.regValue);
		}
	}
	
}

void config_sub_rgb2yuv_for_Vt_capture(void)
{
	//unsigned short *table_index = NULL;
	rgb2yuv_ich2_rgb2yuv_ctrl_RBUS	ich2_rgb2yuv_ctrl_reg;
#if 0	
	rgb2yuv_tab2_m11_m12_RBUS tab2_m11_m12_reg;
	rgb2yuv_tab2_m13_m21_RBUS tab2_m13_m21_reg;
	rgb2yuv_tab2_m22_m23_RBUS tab2_m22_m23_reg;
	rgb2yuv_tab2_m31_m32_RBUS tab2_m31_m32_reg;
	rgb2yuv_tab2_m33_y_gain_RBUS tab2_m33_ygain_reg;
	rgb2yuv_tab2_yo_RBUS tab2_yo_reg;
	
	unsigned char table_num = RGB2YUV_COEF_MATRIX_MODE_Max;

	// calculate table_index
	table_num = RGB2YUV_COEF_YCC601_0_255_TO_YCC709_0_255;
	table_index = &(tRGB2YUV_COEF[table_num][0]);

	IoReg_SetBits(RGB2YUV_ICH1_RGB2YUV_CTRL_reg, RGB2YUV_ICH1_RGB2YUV_CTRL_sub_rgb2yuv_db_en_mask); // sub DB en
	ich2_RGB2YUV_CTRL_REG.en_rgb2yuv = 0;
	IoReg_Write32(RGB2YUV_ICH2_RGB2YUV_CTRL_reg, ich2_RGB2YUV_CTRL_REG.regValue);
	
	/*sub  all tab-2*/
	tab2_m11_m12_reg.m11 = table_index[_RGB2YUV_m11];
	tab2_m11_m12_reg.m12 = table_index[_RGB2YUV_m12];
	IoReg_Write32(RGB2YUV_Tab2_M11_M12_reg, tab2_m11_m12_reg.regValue);
	tab2_m13_m21_reg.m13 = table_index[_RGB2YUV_m13];
	tab2_m13_m21_reg.m21 = table_index[_RGB2YUV_m21];
	IoReg_Write32(RGB2YUV_Tab2_M13_M21_reg, tab2_m13_m21_reg.regValue);
	tab2_m22_m23_reg.m22 = table_index[_RGB2YUV_m22];
	tab2_m22_m23_reg.m23 = table_index[_RGB2YUV_m23];
	IoReg_Write32(RGB2YUV_Tab2_M22_M23_reg, tab2_m22_m23_reg.regValue);
	tab2_m31_m32_reg.m31 = table_index[_RGB2YUV_m31];
	tab2_m31_m32_reg.m32 = table_index[_RGB2YUV_m32];
	IoReg_Write32(RGB2YUV_Tab2_M31_M32_reg, tab2_m31_m32_reg.regValue);
	tab2_m33_ygain_reg.m33 = table_index[_RGB2YUV_m33];
	tab2_m33_ygain_reg.y_gain = table_index[_RGB2YUV_Y_gain];
	IoReg_Write32(RGB2YUV_Tab2_M33_Y_Gain_reg, tab2_m33_ygain_reg.regValue);
	
	tab2_yo_reg.yo_even = table_index[_RGB2YUV_Yo_even];
	tab2_yo_reg.yo_odd = table_index[_RGB2YUV_Yo_odd];
	IoReg_Write32(RGB2YUV_Tab2_Yo_reg, tab2_yo_reg.regValue);

	
	ich2_RGB2YUV_CTRL_REG.sel_rgb = table_index[_RGB2YUV_sel_RGB];
	ich2_RGB2YUV_CTRL_REG.set_r_in_offset = table_index[_RGB2YUV_set_Yin_offset];
	ich2_RGB2YUV_CTRL_REG.set_uv_out_offset = table_index[_RGB2YUV_set_UV_out_offset];
	ich2_RGB2YUV_CTRL_REG.sel_uv_off = table_index[_RGB2YUV_sel_UV_off];
	ich2_RGB2YUV_CTRL_REG.matrix_bypass = table_index[_RGB2YUV_Matrix_bypass];
	ich2_RGB2YUV_CTRL_REG.sel_y_gain = table_index[_RGB2YUV_Enable_Y_gain];
#endif

	ich2_rgb2yuv_ctrl_reg.regValue = IoReg_Read32(RGB2YUV_ICH2_RGB2YUV_CTRL_reg);	
	
	if((get_VT_Pixel_Format() == VT_CAP_NV12) || (get_VT_Pixel_Format() == VT_CAP_NV16) || (get_VT_Pixel_Format() == VT_CAP_YUV888)) //target capture format
	{
		ich2_rgb2yuv_ctrl_reg.en_rgb2yuv = 0;
	}
	else //target rgb output if use sub capture
	{
		ich2_rgb2yuv_ctrl_reg.en_rgb2yuv = 2;  //10: Enable YUV to RGB conversion (GBR out)
	}
	
	IoReg_Write32(RGB2YUV_ICH2_RGB2YUV_CTRL_reg, ich2_rgb2yuv_ctrl_reg.regValue);
}

void config_sub_uzd_for_Vt_capture(SIZE* pin_Size, SIZE* pout_Size)
{
	/*Scale down Setup for Channel 2*/
	scaledown_ich2_uzd_ctrl0_RBUS ich2_uzd_Ctrl0_REG;
	scaledown_ich2_ibuff_ctrli_buff_ctrl_RBUS ich2_ibuff_ctrl_BUFF_CTRL;	
	scaledown_ich2_sdnr_cutout_range_hor_RBUS ich1_sdnr_cutout_range_hor_reg;
	scaledown_ich2_sdnr_cutout_range_ver0_RBUS ich1_sdnr_cutout_range_ver0_reg;
	scaledown_ich2_sdnr_cutout_range_ver1_RBUS ich1_sdnr_cutout_range_ver1_reg;
	scaledown_ich2_uzd_channel_swap_RBUS scaledown_ich2_uzd_channel_swap_Reg;

	scaledown_ich2_uzd_db_ctrl_RBUS scaledown_ich2_uzd_db_ctrl_Reg;
	
	/*close sub uzd double buffer*/
	scaledown_ich2_uzd_db_ctrl_Reg.regValue = IoReg_Read32(SCALEDOWN_ICH2_UZD_DB_CTRL_reg);
	scaledown_ich2_uzd_db_ctrl_Reg.h_db_en = 0;
	scaledown_ich2_uzd_db_ctrl_Reg.v_db_en = 0;
	IoReg_Write32(SCALEDOWN_ICH2_UZD_DB_CTRL_reg, scaledown_ich2_uzd_db_ctrl_Reg.regValue);

	ich1_sdnr_cutout_range_hor_reg.regValue = IoReg_Read32(SCALEDOWN_ich2_sdnr_cutout_range_hor_reg);
	ich1_sdnr_cutout_range_hor_reg.hor_front= 0;
	ich1_sdnr_cutout_range_hor_reg.hor_back= pin_Size->nWidth+1; // chip setting need to +1
	IoReg_Write32(SCALEDOWN_ich2_sdnr_cutout_range_hor_reg, ich1_sdnr_cutout_range_hor_reg.regValue);

	ich1_sdnr_cutout_range_ver0_reg.regValue = IoReg_Read32(SCALEDOWN_ich2_sdnr_cutout_range_ver0_reg);
	ich1_sdnr_cutout_range_ver0_reg.ver_front= 0;
	ich1_sdnr_cutout_range_ver0_reg.ver_back= pin_Size->nLength+1;	// chip setting need to +1
	IoReg_Write32(SCALEDOWN_ich2_sdnr_cutout_range_ver0_reg, ich1_sdnr_cutout_range_ver0_reg.regValue);

	ich1_sdnr_cutout_range_ver1_reg.regValue = IoReg_Read32(SCALEDOWN_ich2_sdnr_cutout_range_ver1_reg);
	ich1_sdnr_cutout_range_ver1_reg.ver_mid_top=0;	// inregion.y;
	ich1_sdnr_cutout_range_ver1_reg.ver_mid_bot=0;	// inregion.y;
	IoReg_Write32(SCALEDOWN_ich2_sdnr_cutout_range_ver1_reg, ich1_sdnr_cutout_range_ver1_reg.regValue);

	scaledown_ich2_uzd_channel_swap_Reg.regValue = IoReg_Read32(SCALEDOWN_ICH2_UZD_channel_swap_reg);
	scaledown_ich2_uzd_channel_swap_Reg.rgb_ch_swap = 0;//YUV	1:VUY-YUV		
	IoReg_Write32(SCALEDOWN_ICH2_UZD_channel_swap_reg, scaledown_ich2_uzd_channel_swap_Reg.regValue);	

	ich2_uzd_Ctrl0_REG.regValue = IoReg_Read32(SCALEDOWN_ICH2_UZD_Ctrl0_reg);
	ich2_uzd_Ctrl0_REG.h_zoom_en = (pin_Size->nWidth > pout_Size->nWidth);
	IoReg_Write32(SCALEDOWN_ICH2_UZD_Ctrl0_reg, ich2_uzd_Ctrl0_REG.regValue);
	
	ich2_uzd_Ctrl0_REG.regValue = IoReg_Read32(SCALEDOWN_ICH2_UZD_Ctrl0_reg);
	ich2_uzd_Ctrl0_REG.v_zoom_en = (pin_Size->nLength > pout_Size->nLength);
	if(ich2_uzd_Ctrl0_REG.v_zoom_en)
		ich2_uzd_Ctrl0_REG.buffer_mode = 2;//do V-scale down have to use v-scale down buffer mode.
	else
		ich2_uzd_Ctrl0_REG.buffer_mode = 0;
		
	if((get_VT_Pixel_Format() == VT_CAP_NV12) || (get_VT_Pixel_Format() == VT_CAP_NV16))
		ich2_uzd_Ctrl0_REG.output_fmt = 0; 
	else
		ich2_uzd_Ctrl0_REG.output_fmt = 1; 
	ich2_uzd_Ctrl0_REG.cutout_en = 0;
	ich2_uzd_Ctrl0_REG.video_comp_en = 0;
	IoReg_Write32(SCALEDOWN_ICH2_UZD_Ctrl0_reg, ich2_uzd_Ctrl0_REG.regValue);

	ich2_ibuff_ctrl_BUFF_CTRL.regValue = IoReg_Read32(SCALEDOWN_ICH2_IBUFF_CTRLI_BUFF_CTRL_reg);
	ich2_ibuff_ctrl_BUFF_CTRL.ibuf_h_size = pout_Size->nWidth;
	ich2_ibuff_ctrl_BUFF_CTRL.ibuf_v_size = pout_Size->nLength;
	IoReg_Write32(SCALEDOWN_ICH2_IBUFF_CTRLI_BUFF_CTRL_reg, ich2_ibuff_ctrl_BUFF_CTRL.regValue);

	drvif_vt_color_ultrazoom_set_scale_down(pin_Size, pout_Size);
	
}

void drvif_sub_mdomain_blk_cap_for_Vt(unsigned int capWid, unsigned int capLen, unsigned int data_fmt)
{
	mdomain_cap_sub_sdnr_cxxto64_RBUS mdomain_cap_sub_sdnr_cxxto64_reg;
	mdomain_cap_ddr_in2ctrl_RBUS mdomain_cap_ddr_in2ctrl_reg;
	mdomain_vi_sub_mdom_sub_vi_addcmd_transform_RBUS mdomain_vi_sub_mdom_sub_vi_addcmd_transform_reg;
	mdomain_vi_sub_sub_v1_output_fmt_RBUS mdomain_vi_sub_sub_v1_output_fmt_reg;
	mdomain_vi_sub_sub_cwvh1_RBUS mdomain_vi_sub_sub_cwvh1_reg;
	mdomain_vi_sub_sub_cwvh1_2_RBUS mdomain_vi_sub_sub_cwvh1_2_reg;
	mdomain_vi_sub_mdom_sub_vi_c_line_step_RBUS mdomain_vi_sub_mdom_sub_vi_c_line_step_reg;
	mdomain_vi_sub_sub_y1buf_RBUS mdomain_vi_sub_sub_y1buf_reg;
	mdomain_vi_sub_sub_c1buf_RBUS mdomain_vi_sub_sub_c1buf_reg;
	
	mdomain_vi_sub_sub_dmactl_RBUS vi_dmactl_reg;//dmaen  //MDOM_VI_DMACTL_reg
	mdomain_vi_sub_sub_gctl_RBUS vi_gctl_reg;//vsce1 //MDOM_VI_GCTL_reg
	mdomain_vi_sub_mdom_sub_vi_ads_start_y_RBUS mdomain_vi_sub_mdom_sub_vi_ads_start_y_reg;
	mdomain_vi_sub_mdom_sub_vi_ads_start_c_RBUS mdomain_vi_sub_mdom_sub_vi_ads_start_c_reg;

	unsigned int LineSize = 0;
	mdomain_cap_cap_reg_doublbuffer_RBUS cap_reg_doublbuffer_reg;
	//disable M-cap double buffer - sub
	cap_reg_doublbuffer_reg.regValue = rtd_inl(MDOMAIN_CAP_cap_reg_doublbuffer_reg);
	cap_reg_doublbuffer_reg.cap2_db_en = 0;
	IoReg_Write32(MDOMAIN_CAP_cap_reg_doublbuffer_reg,cap_reg_doublbuffer_reg.regValue);
	
	mdomain_cap_sub_sdnr_cxxto64_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Sub_SDNR_cxxto64_reg);
	mdomain_cap_sub_sdnr_cxxto64_reg.sub_sort_fmt = 1;//0:422,1:444
	mdomain_cap_sub_sdnr_cxxto64_reg.sub_out_bit = 0;//sub only support 8 bit
	IoReg_Write32(MDOMAIN_CAP_Sub_SDNR_cxxto64_reg, mdomain_cap_sub_sdnr_cxxto64_reg.regValue);
	
	//boundary set
	IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr_BLK0_reg, (CaptureCtrl_VT.cap_buffer[0].phyaddr & 0x7ffffff0));
	IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr2_BLK0_reg, (CaptureCtrl_VT.cap_buffer[0].phyaddr + CaptureCtrl_VT.cap_buffer[0].size) & 0x7ffffff0);
	IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr_BLK1_reg, (CaptureCtrl_VT.cap_buffer[1].phyaddr & 0x7ffffff0));
	IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr2_BLK1_reg, (CaptureCtrl_VT.cap_buffer[1].phyaddr + CaptureCtrl_VT.cap_buffer[1].size) & 0x7ffffff0);

	IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr_BLK2_reg, (CaptureCtrl_VT.cap_buffer[2].phyaddr & 0x7ffffff0));
	IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr2_BLK2_reg, (CaptureCtrl_VT.cap_buffer[2].phyaddr + CaptureCtrl_VT.cap_buffer[2].size));

	mdomain_cap_ddr_in2ctrl_reg.regValue = IoReg_Read32(MDOMAIN_CAP_DDR_In2Ctrl_reg);
	//single buffer
	mdomain_cap_ddr_in2ctrl_reg.in2_en_vs_rst = 1;
	mdomain_cap_ddr_in2ctrl_reg.in2_double_enable = 0; //single buffer
	mdomain_cap_ddr_in2ctrl_reg.in2_v_flip_3buf_en = 0;
	mdomain_cap_ddr_in2ctrl_reg.in2_bufnum = 0;
	mdomain_cap_ddr_in2ctrl_reg.in2_disp2cap_distance = 0;
	
	IoReg_Write32(MDOMAIN_CAP_DDR_In2Ctrl_reg, mdomain_cap_ddr_in2ctrl_reg.regValue);

	//set_mdomain_sub_vi_cap_nv12_for_vt(capWid, capLen, 0); //420

	//sub vi register setting
    vi_dmactl_reg.regValue = IoReg_Read32(MDOMAIN_VI_SUB_SUB_DMACTL_reg);
    vi_dmactl_reg.dmaen1 = 0;
    IoReg_Write32(MDOMAIN_VI_SUB_SUB_DMACTL_reg, vi_dmactl_reg.regValue);

	mdomain_vi_sub_sub_v1_output_fmt_reg.regValue = IoReg_Read32(MDOMAIN_VI_SUB_SUB_V1_OUTPUT_FMT_reg);
	mdomain_vi_sub_sub_v1_output_fmt_reg.swap_1byte = 0;
	mdomain_vi_sub_sub_v1_output_fmt_reg.swap_2byte = 0;
	mdomain_vi_sub_sub_v1_output_fmt_reg.swap_4byte = 0;
	mdomain_vi_sub_sub_v1_output_fmt_reg.swap_8byte = 0;
	mdomain_vi_sub_sub_v1_output_fmt_reg.data_format = data_fmt;
	IoReg_Write32(MDOMAIN_VI_SUB_SUB_V1_OUTPUT_FMT_reg,mdomain_vi_sub_sub_v1_output_fmt_reg.regValue);
	
	mdomain_vi_sub_sub_cwvh1_reg.regValue = IoReg_Read32(MDOMAIN_VI_SUB_SUB_CWVH1_reg);
	mdomain_vi_sub_sub_cwvh1_reg.hsize = capWid;
	mdomain_vi_sub_sub_cwvh1_reg.vsize = capLen;
	IoReg_Write32(MDOMAIN_VI_SUB_SUB_CWVH1_reg, mdomain_vi_sub_sub_cwvh1_reg.regValue);
	
	mdomain_vi_sub_sub_cwvh1_2_reg.regValue = IoReg_Read32(MDOMAIN_VI_SUB_SUB_CWVH1_2_reg);
	mdomain_vi_sub_sub_cwvh1_2_reg.vsize_c = (capLen >> ((data_fmt == FMT_NV16) ? 0 : 1));
	IoReg_Write32(MDOMAIN_VI_SUB_SUB_CWVH1_2_reg, mdomain_vi_sub_sub_cwvh1_2_reg.regValue);

	LineSize = capWid * 8 / 64;
	if(LineSize % 2)
		LineSize += 1;
	rtd_pr_vt_notice("[VT][vi]LineSize:%d\n",LineSize);
	
	mdomain_vi_sub_mdom_sub_vi_c_line_step_reg.regValue = IoReg_Read32(MDOMAIN_VI_SUB_mdom_sub_vi_c_line_step_reg);
	mdomain_vi_sub_mdom_sub_vi_c_line_step_reg.c_line_step = LineSize;
	mdomain_vi_sub_mdom_sub_vi_c_line_step_reg.common_line_step_en = 0;
	IoReg_Write32(MDOMAIN_VI_SUB_mdom_sub_vi_c_line_step_reg, mdomain_vi_sub_mdom_sub_vi_c_line_step_reg.regValue);
	
	mdomain_vi_sub_mdom_sub_vi_addcmd_transform_reg.regValue = IoReg_Read32(MDOMAIN_VI_SUB_mdom_sub_vi_addcmd_transform_reg);
	mdomain_vi_sub_mdom_sub_vi_addcmd_transform_reg.line_step = LineSize;
	//mdomain_vi_sub_mdom_sub_vi_addcmd_transform_reg.vi_addcmd_trans_en = 1;
	IoReg_Write32(MDOMAIN_VI_SUB_mdom_sub_vi_addcmd_transform_reg, mdomain_vi_sub_mdom_sub_vi_addcmd_transform_reg.regValue);
	
	mdomain_vi_sub_mdom_sub_vi_ads_start_y_reg.regValue = CaptureCtrl_VT.cap_buffer[0].phyaddr;
	IoReg_Write32(MDOMAIN_VI_SUB_mdom_sub_vi_ads_start_y_reg, mdomain_vi_sub_mdom_sub_vi_ads_start_y_reg.regValue);

	mdomain_vi_sub_mdom_sub_vi_ads_start_c_reg.regValue = (CaptureCtrl_VT.cap_buffer[0].phyaddr + VT_CAP_FRAME_WIDTH_4K2K*VT_CAP_FRAME_HEIGHT_4K2K /*_ALIGN((VT_CAP_FRAME_WIDTH_2K1k*VT_CAP_FRAME_HEIGHT_2K1k),__4KPAGE)*/);
	IoReg_Write32(MDOMAIN_VI_SUB_mdom_sub_vi_ads_start_c_reg, mdomain_vi_sub_mdom_sub_vi_ads_start_c_reg.regValue);

	//rtd_pr_vt_notice("sub_vi_ads_start_y_reg = 0x%x\n", IoReg_Read32(MDOMAIN_VI_SUB_mdom_sub_vi_ads_start_y_reg));
	//rtd_pr_vt_notice("sub_vi_ads_start_c_reg = 0x%x\n", IoReg_Read32(MDOMAIN_VI_SUB_mdom_sub_vi_ads_start_c_reg));

	// water lvl
	mdomain_vi_sub_sub_y1buf_reg.regValue = IoReg_Read32(MDOMAIN_VI_SUB_SUB_Y1BUF_reg);
	mdomain_vi_sub_sub_y1buf_reg.y_water_lvl = 64; //120; //120*64bits
	IoReg_Write32(MDOMAIN_VI_SUB_SUB_Y1BUF_reg, mdomain_vi_sub_sub_y1buf_reg.regValue);
	
	mdomain_vi_sub_sub_c1buf_reg.regValue = IoReg_Read32(MDOMAIN_VI_SUB_SUB_C1BUF_reg);
	mdomain_vi_sub_sub_c1buf_reg.c_water_lvl = 64; //120; //120*64bits
	IoReg_Write32(MDOMAIN_VI_SUB_SUB_C1BUF_reg, mdomain_vi_sub_sub_c1buf_reg.regValue);
	
	vi_gctl_reg.regValue = IoReg_Read32(MDOMAIN_VI_SUB_SUB_GCTL_reg);
	vi_gctl_reg.vsce1 = 1;
	IoReg_Write32(MDOMAIN_VI_SUB_SUB_GCTL_reg, vi_gctl_reg.regValue);
	
	vi_dmactl_reg.regValue = IoReg_Read32(MDOMAIN_VI_SUB_SUB_DMACTL_reg);
	vi_dmactl_reg.bstlen = 6; //2^6 * 64 bits = 64*64 bits
	vi_dmactl_reg.dmaen1 = 1;
	vi_dmactl_reg.seq_blk_sel = 1; //select block mode DMA
	IoReg_Write32(MDOMAIN_VI_SUB_SUB_DMACTL_reg, vi_dmactl_reg.regValue);
	rtd_pr_vt_notice("[VT] %s\n",__FUNCTION__);
	IoReg_SetBits(MDOMAIN_CAP_Cap_In2_enable_reg, _BIT0);
}

void sub_mdomain_vi_for_Vt_cap_isr(void)
{
	//unsigned int wr_index = 0;
	mdomain_vi_sub_mdom_sub_vi_ads_start_y_RBUS mdomain_vi_sub_mdom_sub_vi_ads_start_y_reg;
	mdomain_vi_sub_mdom_sub_vi_ads_start_y_reg.regValue = IoReg_Read32(MDOMAIN_VI_SUB_mdom_sub_vi_ads_start_y_reg);

	if(mdomain_vi_sub_mdom_sub_vi_ads_start_y_reg.regValue == (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr){
	   mdomain_vi_sub_mdom_sub_vi_ads_start_y_reg.regValue = (UINT32)CaptureCtrl_VT.cap_buffer[1].phyaddr;
	}else if (mdomain_vi_sub_mdom_sub_vi_ads_start_y_reg.regValue == (UINT32)CaptureCtrl_VT.cap_buffer[1].phyaddr){
	   mdomain_vi_sub_mdom_sub_vi_ads_start_y_reg.regValue = (UINT32)CaptureCtrl_VT.cap_buffer[2].phyaddr;
	}else if (mdomain_vi_sub_mdom_sub_vi_ads_start_y_reg.regValue == (UINT32)CaptureCtrl_VT.cap_buffer[2].phyaddr){
	   mdomain_vi_sub_mdom_sub_vi_ads_start_y_reg.regValue = (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;
	}

	if(mdomain_vi_sub_mdom_sub_vi_ads_start_y_reg.regValue == VtFreezeRegAddr){
	   if(mdomain_vi_sub_mdom_sub_vi_ads_start_y_reg.regValue == (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr){
		   mdomain_vi_sub_mdom_sub_vi_ads_start_y_reg.regValue = (UINT32)CaptureCtrl_VT.cap_buffer[1].phyaddr;
	   }else if (mdomain_vi_sub_mdom_sub_vi_ads_start_y_reg.regValue == (UINT32)CaptureCtrl_VT.cap_buffer[1].phyaddr){
		   mdomain_vi_sub_mdom_sub_vi_ads_start_y_reg.regValue = (UINT32)CaptureCtrl_VT.cap_buffer[2].phyaddr;
	   }else if (mdomain_vi_sub_mdom_sub_vi_ads_start_y_reg.regValue == (UINT32)CaptureCtrl_VT.cap_buffer[2].phyaddr){
		   mdomain_vi_sub_mdom_sub_vi_ads_start_y_reg.regValue = (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;
	   }
	}
	
	IoReg_Write32(MDOMAIN_VI_SUB_mdom_sub_vi_ads_start_y_reg, mdomain_vi_sub_mdom_sub_vi_ads_start_y_reg.regValue);
	IoReg_Write32(MDOMAIN_VI_SUB_mdom_sub_vi_ads_start_c_reg, mdomain_vi_sub_mdom_sub_vi_ads_start_y_reg.regValue + VT_CAP_FRAME_WIDTH_4K2K*VT_CAP_FRAME_HEIGHT_4K2K);
}

void drvif_sub_mdomain_seq_cap_for_Vt(unsigned int capWid, unsigned int capLen, eMemCapAccessType Cap_type, unsigned char bDouble)
{	
	//memory_set_subcapture_vt(TRUE, MEMCAPTYPE_LINE, capWid, capLen);

	unsigned int  TotalSize;
	unsigned int  Quotient;
	unsigned char  Remainder;
	unsigned int blockstep;//need to be even
	unsigned int burstLen = _BURSTLENGTH2;
//	unsigned int buffer_size = 0;
	unsigned int double_buf_flag = 0, triple_buf_flag = 0;
	unsigned char Sts;
	mdomain_cap_capture2_byte_swap_RBUS capture2_byte_swap_reg;
	mdomain_cap_sub_sdnr_cxxto64_RBUS mdomain_cap_sub_sdnr_cxxto64_reg;
	mdomain_cap_ddr_in2ctrl_RBUS mdomain_cap_ddr_in2ctrl_reg;
	mdomain_cap_cap_ddr_fifostatus_RBUS cap_ddr_fifostatus_reg;

	//field polarity error check
	Sts = IoReg_ReadWord0(/*channel ? MDOMAIN_CAP_DDR_In2Status_reg : */MDOMAIN_CAP_DDR_In1Status_reg);
	if (Sts & _BIT4) {	//Field polarity Error bit
		IoReg_ClearBits(/*channel ? MDOMAIN_CAP_DDR_In2Status_reg : */MDOMAIN_CAP_DDR_In1Status_reg, _BIT4);
	}

	rtd_pr_vt_notice("func:%s\n", __FUNCTION__);
		
	mdomain_cap_sub_sdnr_cxxto64_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Sub_SDNR_cxxto64_reg);
	mdomain_cap_sub_sdnr_cxxto64_reg.sub_sort_fmt = 0;//0:422,1:444, default=1
	mdomain_cap_sub_sdnr_cxxto64_reg.sub_out_bit = 0;//sub only support 8 bit
	IoReg_Write32(MDOMAIN_CAP_Sub_SDNR_cxxto64_reg, mdomain_cap_sub_sdnr_cxxto64_reg.regValue);

	TotalSize = memory_get_capture_size_subvt(capWid, capLen, Cap_type);		// calculate the memory size of capture

	if(Cap_type == MEMCAPTYPE_LINE){

		IoReg_Write32(MDOMAIN_CAP_DDR_In2LineStep_reg, (TotalSize + TotalSize%2) << 3);//bit 3 need to be 0. rbus rule
		blockstep = (TotalSize*capLen);
		blockstep = (blockstep % 2)? blockstep + 1: blockstep;
		IoReg_Write32(MDOMAIN_CAP_DDR_In2BlkStep_reg, blockstep << 3);//bit 3 need to be 0. rbus rule

	}else{
		IoReg_Write32(MDOMAIN_CAP_DDR_In2LineStep_reg, 0);
	}

	memory_division(TotalSize, burstLen, &Quotient, &Remainder);	// Do a division

	Remainder = drvif_memory_get_data_align(Remainder, 4);	// multiple of 4

	if (Remainder == 0) {	// remainder is not allowed to be zero
		Remainder = burstLen;
		Quotient -= 1;
	}

	//memory_set_capture_fifo(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL), Quotient, Remainder, burstLen);	// set capture FIFO
	IoReg_Write32(MDOMAIN_CAP_DDR_In2WTLVL_reg, ((burstLen>>1) << 16) | Quotient);
	IoReg_Write32(MDOMAIN_CAP_DDR_In2WrLen_Rem_reg, (burstLen << 8) | Remainder);

	TotalSize = SHL(Quotient * burstLen + Remainder, 3); //unit conversion. 64bits to 8bits

	if(Cap_type == MEMCAPTYPE_LINE){
		TotalSize *=capLen;

	}
	if(bDouble)
	{
		IoReg_Write32(MDOMAIN_CAP_DDR_In2_2ndAddr_reg, CaptureCtrl_VT.cap_buffer[1].phyaddr & 0x7ffffff0);
	}

	IoReg_Write32(MDOMAIN_CAP_DDR_In2Addr_reg, CaptureCtrl_VT.cap_buffer[0].phyaddr & 0x7ffffff0);
	rtd_pr_vt_notice("[VT]###################sub buffer TotalSize=%d\n", TotalSize);

	/*restore buffer freeze use at MDOMAIN_CAP_DDR_In2_3rdAddr_reg*/
	IoReg_Write32(MDOMAIN_CAP_DDR_In2_3rdAddr_reg, CaptureCtrl_VT.cap_buffer[2].phyaddr & 0x7ffffff0);

	IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr_BLK0_reg, (CaptureCtrl_VT.cap_buffer[0].phyaddr & 0x7ffffff0));
	IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr2_BLK0_reg, (CaptureCtrl_VT.cap_buffer[0].phyaddr+CaptureCtrl_VT.cap_buffer[0].size) & 0x7ffffff0);
	IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr_BLK1_reg, (CaptureCtrl_VT.cap_buffer[1].phyaddr & 0x7ffffff0));
	IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr2_BLK1_reg, (CaptureCtrl_VT.cap_buffer[1].phyaddr+CaptureCtrl_VT.cap_buffer[1].size) & 0x7ffffff0);
	
	IoReg_Mask32(MDOMAIN_CAP_DDR_In2LineNum_reg, 0xfffee000, (Cap_type<<16)|AND(capLen, 0x1FFF));

	if(bDouble){
		double_buf_flag = 1;
		triple_buf_flag = 0;
	}
	else{
		double_buf_flag = 0;
		triple_buf_flag = 0;
	}

	//Use register structure to set
	mdomain_cap_ddr_in2ctrl_reg.regValue = IoReg_Read32(MDOMAIN_CAP_DDR_In2Ctrl_reg);
	if(triple_buf_flag == 1){//triple buffer case
		mdomain_cap_ddr_in2ctrl_reg.in2_en_vs_rst = 1;
		mdomain_cap_ddr_in2ctrl_reg.in2_double_enable = 1;
		mdomain_cap_ddr_in2ctrl_reg.in2_v_flip_3buf_en = 1;
		mdomain_cap_ddr_in2ctrl_reg.in2_bufnum = 2; // 3-buf mode
		mdomain_cap_ddr_in2ctrl_reg.in2_disp2cap_distance = 1;
	}
	else if((double_buf_flag == 1) && (triple_buf_flag == 0)){//double buffer case
		mdomain_cap_ddr_in2ctrl_reg.in2_en_vs_rst = 1;
		mdomain_cap_ddr_in2ctrl_reg.in2_double_enable = 1;
		mdomain_cap_ddr_in2ctrl_reg.in2_v_flip_3buf_en = 0;
		mdomain_cap_ddr_in2ctrl_reg.in2_bufnum = 0;
		mdomain_cap_ddr_in2ctrl_reg.in2_disp2cap_distance = 0;
	}
	else{//single buffer
		mdomain_cap_ddr_in2ctrl_reg.in2_en_vs_rst = 1;
		mdomain_cap_ddr_in2ctrl_reg.in2_double_enable = 0;
		mdomain_cap_ddr_in2ctrl_reg.in2_v_flip_3buf_en = 0;
		mdomain_cap_ddr_in2ctrl_reg.in2_bufnum = 0;
		mdomain_cap_ddr_in2ctrl_reg.in2_disp2cap_distance = 0;
	}
	IoReg_Write32(MDOMAIN_CAP_DDR_In2Ctrl_reg, mdomain_cap_ddr_in2ctrl_reg.regValue);

	//below code YUYV format ++
	capture2_byte_swap_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Capture2_byte_swap_reg);
	capture2_byte_swap_reg.in2_1byte_swap = 0;
	capture2_byte_swap_reg.in2_2byte_swap = 1;
	capture2_byte_swap_reg.in2_4byte_swap = 1;
	capture2_byte_swap_reg.in2_8byte_swap = 0;
	IoReg_Write32(MDOMAIN_CAP_Capture2_byte_swap_reg, capture2_byte_swap_reg.regValue);
	//frank@03132013 add below code for GPU access YUV format --
	IoReg_SetBits(MDOMAIN_CAP_Cap_In2_enable_reg, _BIT0);

	/*wclr cap buf done status*/
	cap_ddr_fifostatus_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg);
	cap_ddr_fifostatus_reg.in1_cap_last_wr_flag = 1;
	IoReg_Write32(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg, cap_ddr_fifostatus_reg.regValue);

}


void drvif_sub_source_VtCaptureConfig(VT_CUR_CAPTURE_INFO capInfo, unsigned char waitbufferfresh)
{
	rtd_pr_vt_notice("func:%s\n", __FUNCTION__);
	
	if(capInfo.enable == TRUE) {
		SIZE in_Size,out_Size;
		sub_vgip_vgip_chn2_double_buffer_ctrl_RBUS sub_vgip_vgip_chn2_double_buffer_ctrl_Reg;
		scaledown_ich2_uzd_db_ctrl_RBUS scaledown_ich2_uzd_db_ctrl_Reg;
		mdomain_cap_cap_reg_doublbuffer_RBUS cap_reg_doublbuffer_reg;

		in_Size.nWidth = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_DI_WID);		
		in_Size.nLength = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_DI_LEN);
		
		rtd_pr_vt_notice("VT in_Size.nWidth = %d,in_Size.nLength = %d;%s=%d \n", in_Size.nWidth,in_Size.nLength, __FUNCTION__, __LINE__);
		out_Size.nWidth = capInfo.capWid;
		out_Size.nLength = capInfo.capLen;
		rtd_pr_vt_notice("VT out_Size.nWidth = %d,out_Size.nLength = %d;%s=%d \n", out_Size.nWidth,out_Size.nLength, __FUNCTION__, __LINE__);
		
		/*close sub vgip double buffer*/
		sub_vgip_vgip_chn2_double_buffer_ctrl_Reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_DOUBLE_BUFFER_CTRL_reg);
		sub_vgip_vgip_chn2_double_buffer_ctrl_Reg.ch2_db_en = 0;
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_DOUBLE_BUFFER_CTRL_reg, sub_vgip_vgip_chn2_double_buffer_ctrl_Reg.regValue);

		/*close sub uzd double buffer*/
		scaledown_ich2_uzd_db_ctrl_Reg.regValue = IoReg_Read32(SCALEDOWN_ICH2_UZD_DB_CTRL_reg);
		scaledown_ich2_uzd_db_ctrl_Reg.h_db_en = 0;
		scaledown_ich2_uzd_db_ctrl_Reg.v_db_en = 0;
		IoReg_Write32(SCALEDOWN_ICH2_UZD_DB_CTRL_reg, scaledown_ich2_uzd_db_ctrl_Reg.regValue);

		//disable M-cap double buffer - sub
		cap_reg_doublbuffer_reg.regValue = rtd_inl(MDOMAIN_CAP_cap_reg_doublbuffer_reg);
		cap_reg_doublbuffer_reg.cap2_db_en = 0;
		rtd_outl(MDOMAIN_CAP_cap_reg_doublbuffer_reg,cap_reg_doublbuffer_reg.regValue);
		
		config_sub_vgip_for_Vt_capture();
		
		config_sub_rgb2yuv_for_Vt_capture();

		config_sub_uzd_for_Vt_capture(&in_Size, &out_Size);

		if(get_VT_Pixel_Format() == VT_CAP_NV12)
		{
			drvif_sub_mdomain_blk_cap_for_Vt(out_Size.nWidth, out_Size.nLength, FMT_NV12);
		}
		else if(get_VT_Pixel_Format() == VT_CAP_NV16)
		{
			drvif_sub_mdomain_blk_cap_for_Vt(out_Size.nWidth, out_Size.nLength, FMT_NV16);
		}
		else
		{	//sequence capture use hw double buffer mode 
			drvif_sub_mdomain_seq_cap_for_Vt(out_Size.nWidth, out_Size.nLength, MEMCAPTYPE_LINE, TRUE);
		}
	
		if(waitbufferfresh == TRUE) {
			drv_memory_wait_cap_last_write_done(SLR_SUB_DISPLAY,2,0);
		}		
	} 
	else
	{	mdomain_vi_sub_sub_dmactl_RBUS vi_dmactl_reg;//dmaen  //MDOM_VI_DMACTL_reg
		mdomain_cap_cap_in2_enable_RBUS in2_cap_en_reg;
	
		if((get_VT_Pixel_Format() == VT_CAP_NV12) || (get_VT_Pixel_Format() == VT_CAP_NV16))
		{
			vi_dmactl_reg.regValue = IoReg_Read32(MDOMAIN_VI_SUB_SUB_DMACTL_reg);
    		vi_dmactl_reg.dmaen1 = 0;
   			IoReg_Write32(MDOMAIN_VI_SUB_SUB_DMACTL_reg, vi_dmactl_reg.regValue);
		}
		else
		{
			in2_cap_en_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Cap_In2_enable_reg);
			in2_cap_en_reg.in2_cap_enable = 0;
			IoReg_Write32(MDOMAIN_CAP_Cap_In2_enable_reg, in2_cap_en_reg.regValue);
		}
		drvif_vt_check_subvgip_data_dclk(FALSE);
		//IoReg_ClearBits(MDOMAIN_CAP_Cap_In2_enable_reg, _BIT0);
		
	}
}

void drvif_I3DDMA_Set_VtCaptureDisable(void)
{
	h3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_ctrl_RBUS  i3ddma_rgb2yuv_ctrl_reg;
	h3ddma_hsd_i3ddma_hsd_ctrl0_RBUS i3ddma_hsd_ctrl0_reg;
	h3ddma_hsd_i3ddma_vsd_ctrl0_RBUS i3ddma_vsd_ctrl0_reg;
	dma_vgip_dma_vgip_ctrl_RBUS dma_vgip_chn1_ctrl_reg;
	h3ddma_cap0_cti_dma_wr_ctrl_RBUS h3ddma_cap0_cti_dma_wr_ctrl_reg;

	if((get_VT_Pixel_Format() == VT_CAP_NV12) || (get_VT_Pixel_Format() == VT_CAP_NV16))
	{
		vt_i3ddma_vi_capture_enable(0); //cap1cap2 dma disable
	}
	else
	{
		h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg);
		h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_dma_enable = 0;
		IoReg_Write32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg, h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue);
	}

	i3ddma_rgb2yuv_ctrl_reg.regValue = IoReg_Read32(H3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_CTRL_reg);
	i3ddma_rgb2yuv_ctrl_reg.en_rgb2yuv = 0;
	IoReg_Write32(H3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_CTRL_reg, i3ddma_rgb2yuv_ctrl_reg.regValue);

	i3ddma_hsd_ctrl0_reg.regValue = IoReg_Read32(H3DDMA_HSD_I3DDMA_HSD_Ctrl0_reg);
	i3ddma_hsd_ctrl0_reg.h_zoom_en = 0;
	IoReg_Write32(H3DDMA_HSD_I3DDMA_HSD_Ctrl0_reg, i3ddma_hsd_ctrl0_reg.regValue);

	i3ddma_vsd_ctrl0_reg.regValue = IoReg_Read32(H3DDMA_HSD_I3DDMA_VSD_Ctrl0_reg);
	i3ddma_vsd_ctrl0_reg.v_zoom_en = 0;
	IoReg_Write32(H3DDMA_HSD_I3DDMA_VSD_Ctrl0_reg, i3ddma_vsd_ctrl0_reg.regValue);

	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_ivrun = 0;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);	

	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_in_clk_en = 0;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);

}

void drvif_I3DDMA_Set_SeqCap0_boundary_set_VtCapture(void)
{
	//i3ddma boundary setting
	unsigned int buf_count = get_vt_VtBufferNum();
	if(buf_count == 3)
	{			
		rtd_outl(H3DDMA_CAP0_CTI_DMA_WR_Rule_check_low_reg, CaptureCtrl_VT.cap_buffer[0].phyaddr);	//cap0 low limit
		rtd_outl(H3DDMA_CAP0_CTI_DMA_WR_Rule_check_up_reg, CaptureCtrl_VT.cap_buffer[0].phyaddr + CaptureCtrl_VT.cap_buffer[0].size); //cap0 up limit

		
		rtd_outl(H3DDMA_CAP0_CTI_DMA_WR_Rule_check_low_1_reg, CaptureCtrl_VT.cap_buffer[1].phyaddr);  //cap0 low limit
		rtd_outl(H3DDMA_CAP0_CTI_DMA_WR_Rule_check_up_1_reg, CaptureCtrl_VT.cap_buffer[1].phyaddr + CaptureCtrl_VT.cap_buffer[1].size); //cap0 up limit

		
		rtd_outl(H3DDMA_CAP0_CTI_DMA_WR_Rule_check_low_2_reg, CaptureCtrl_VT.cap_buffer[2].phyaddr);  //cap0 low limit
		rtd_outl(H3DDMA_CAP0_CTI_DMA_WR_Rule_check_up_2_reg, CaptureCtrl_VT.cap_buffer[2].phyaddr + CaptureCtrl_VT.cap_buffer[2].size); //cap0 up limit
	}
	else if (buf_count == 1)
	{		
		rtd_outl(H3DDMA_CAP0_CTI_DMA_WR_Rule_check_low_reg, CaptureCtrl_VT.cap_buffer[0].phyaddr);  //cap0 low limit
		rtd_outl(H3DDMA_CAP0_CTI_DMA_WR_Rule_check_up_reg, CaptureCtrl_VT.cap_buffer[0].phyaddr + CaptureCtrl_VT.cap_buffer[0].size); //cap0 up limit
	}
}
void drvif_I3DDMA_Set_SeqCap0_Addr(void)
{
	unsigned int buf_count = get_vt_VtBufferNum();
	if(buf_count == 3)
	{
/* default HW double buffer must set H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_0_reg and H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_1_reg*/

		IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_0_reg, CaptureCtrl_VT.cap_buffer[0].phyaddr);
		IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_1_reg,  CaptureCtrl_VT.cap_buffer[1].phyaddr);	

		//3rd buffer takes turn to write
		IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_2_reg,  CaptureCtrl_VT.cap_buffer[2].phyaddr);
	}
	else if(buf_count == 1)
	{
		IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_0_reg, CaptureCtrl_VT.cap_buffer[0].phyaddr);
	}
		
}

extern unsigned char dvrif_i3ddma_compression_get_enable(void);
extern unsigned char dvrif_i3ddma_get_compression_bits(void);

void drvif_i3ddma_cap0_seq_cap_forVt(SIZE outsize)
{
#define FIFO_WIDTH   128

	int bitsPerLine; 
	int fifoElementsPerFrame; 
	int burstLen;
	//unsigned int h_act_len;

	h3ddma_cap0_cap_ctl0_RBUS h3ddma_cap0_cap_ctl0_reg;
	h3ddma_lr_separate_ctrl1_RBUS h3ddma_lr_separate_ctrl1_reg;
	h3ddma_lr_separate_ctrl2_RBUS h3ddma_lr_separate_ctrl2_reg;
	h3ddma_cap0_cti_dma_wr_ctrl_RBUS h3ddma_cap0_cti_dma_wr_ctrl_reg;
//	h3ddma_i3ddma_enable_RBUS i3ddma_enable_reg;
	h3ddma_cap0_wr_dma_pxltobus_RBUS h3ddma_cap0_wr_dma_pxltobus_reg;
	h3ddma_cap0_wr_dma_num_bl_wrap_word_RBUS  h3ddma_cap0_wr_dma_num_bl_wrap_word_reg;
	h3ddma_cap0_wr_dma_num_bl_wrap_ctl_RBUS  h3ddma_cap0_wr_dma_num_bl_wrap_ctl_reg;
	h3ddma_cap0_wr_dma_num_bl_wrap_line_step_RBUS  h3ddma_cap0_wr_dma_num_bl_wrap_line_step_reg;
	h3ddma_i3ddma_enable_RBUS i3ddma_enable_reg;
	h3ddma_i3ddma_ctrl_0_RBUS i3ddma_ctrl_0_reg;
	
	unsigned int buf_count = get_vt_VtBufferNum();
	
	h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg);
	h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_dma_enable = 0;
	IoReg_Write32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg, h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue);
	
	// IDMA LR separate control
	h3ddma_lr_separate_ctrl1_reg.regValue = IoReg_Read32(H3DDMA_LR_Separate_CTRL1_reg);
	h3ddma_lr_separate_ctrl1_reg.progressive = 1; //input is dispD
	h3ddma_lr_separate_ctrl1_reg.hdmi_3d_structure = 10; // frame 2D
	h3ddma_lr_separate_ctrl1_reg.fp_vact_space1 = 1;
	h3ddma_lr_separate_ctrl1_reg.fp_vact_space2 = 5;
	IoReg_Write32(H3DDMA_LR_Separate_CTRL1_reg, h3ddma_lr_separate_ctrl1_reg.regValue);

	h3ddma_lr_separate_ctrl2_reg.regValue = IoReg_Read32(H3DDMA_LR_Separate_CTRL2_reg);
	h3ddma_lr_separate_ctrl2_reg.hact = outsize.nWidth;
	h3ddma_lr_separate_ctrl2_reg.vact = outsize.nLength;
	IoReg_Write32(H3DDMA_LR_Separate_CTRL2_reg,h3ddma_lr_separate_ctrl2_reg.regValue);

	/*if(dvrif_i3ddma_compression_get_enable() == TRUE){
		h_act_len = timing->h_act_len;
		if((h_act_len % 32) != 0){
			//drop bits
			h_act_len = h_act_len + (32 - (h_act_len % 32));
		}
		bitsPerLine = h_act_len * dvrif_i3ddma_get_compression_bits() + 256;
	}
	else*/

	bitsPerLine = (8 * outsize.nWidth * 3); //rgb
	
	if(bitsPerLine%FIFO_WIDTH)
		fifoElementsPerFrame = (bitsPerLine/FIFO_WIDTH)+1;
	else
		fifoElementsPerFrame = (bitsPerLine/FIFO_WIDTH);
	burstLen = 32;

	h3ddma_cap0_wr_dma_num_bl_wrap_word_reg.regValue = IoReg_Read32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_word_reg);
	h3ddma_cap0_wr_dma_num_bl_wrap_word_reg.cap0_line_burst_num = fifoElementsPerFrame;
	IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_word_reg, h3ddma_cap0_wr_dma_num_bl_wrap_word_reg.regValue);

	h3ddma_cap0_wr_dma_num_bl_wrap_ctl_reg.regValue = IoReg_Read32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_ctl_reg);
	h3ddma_cap0_wr_dma_num_bl_wrap_ctl_reg.cap0_burst_len = burstLen;
	h3ddma_cap0_wr_dma_num_bl_wrap_ctl_reg.cap0_line_num = outsize.nLength;
	IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_ctl_reg, h3ddma_cap0_wr_dma_num_bl_wrap_ctl_reg.regValue);

	h3ddma_cap0_wr_dma_num_bl_wrap_line_step_reg.regValue = IoReg_Read32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_line_step_reg);
	h3ddma_cap0_wr_dma_num_bl_wrap_line_step_reg.cap0_line_step = fifoElementsPerFrame; 
	IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_line_step_reg, h3ddma_cap0_wr_dma_num_bl_wrap_line_step_reg.regValue);

	h3ddma_cap0_cap_ctl0_reg.regValue = rtd_inl(H3DDMA_CAP0_Cap_CTL0_reg);
	h3ddma_cap0_cap_ctl0_reg.cap0_triple_buf_en = 0;
	h3ddma_cap0_cap_ctl0_reg.cap0_quad_buf_en = 0;
	h3ddma_cap0_cap_ctl0_reg.cap0_freeze_en = 0;
	
	if(buf_count == 1)
		h3ddma_cap0_cap_ctl0_reg.cap0_auto_block_sel_en = 0;
	else
		h3ddma_cap0_cap_ctl0_reg.cap0_auto_block_sel_en = 1;

	h3ddma_cap0_cap_ctl0_reg.cap0_half_wr_sel = 0;
	h3ddma_cap0_cap_ctl0_reg.cap0_half_wr_en = 0;
	IoReg_Write32(H3DDMA_CAP0_Cap_CTL0_reg, h3ddma_cap0_cap_ctl0_reg.regValue);

	rtd_pr_vt_notice("H3DDMA_CAP0_Cap_CTL0_reg = %x\n", IoReg_Read32(H3DDMA_CAP0_Cap_CTL0_reg));
	
	h3ddma_cap0_wr_dma_pxltobus_reg.regValue=IoReg_Read32(H3DDMA_CAP0_WR_DMA_pxltobus_reg);
//#ifndef CONFIG_RTK_KDRV_DV_IDK_DUMP
#if 0
	h3ddma_cap0_wr_dma_pxltobus_reg.cap0_bit_sel=0;
	h3ddma_cap0_wr_dma_pxltobus_reg.cap0_pixel_encoding=0;
#else
	h3ddma_cap0_wr_dma_pxltobus_reg.cap0_bit_sel = 0; //8 bits
	h3ddma_cap0_wr_dma_pxltobus_reg.cap0_pixel_encoding = 0; //4:4:4
#endif
	IoReg_Write32(H3DDMA_CAP0_WR_DMA_pxltobus_reg, h3ddma_cap0_wr_dma_pxltobus_reg.regValue);
	
	//for capture
	h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg);
	h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_dma_1byte_swap = 1;
	h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_dma_2byte_swap = 1;
	h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_dma_4byte_swap = 1;
	h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_dma_8byte_swap = 1;
	//h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_dma_enable=1; //enable IDMA capture3 
	IoReg_Write32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg, h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue);

	drvif_I3DDMA_Set_SeqCap0_boundary_set_VtCapture();
	
	drvif_I3DDMA_Set_SeqCap0_Addr();

	//channel byte swap
	i3ddma_ctrl_0_reg.regValue = IoReg_Read32(H3DDMA_I3DDMA_ctrl_0_reg);
	i3ddma_ctrl_0_reg.cap0_channel_swap = 4; //RBG
	IoReg_Write32(H3DDMA_I3DDMA_ctrl_0_reg, i3ddma_ctrl_0_reg.regValue );
		
	// IDMA timeing gen enable	
	i3ddma_enable_reg.regValue = IoReg_Read32(H3DDMA_I3DDMA_enable_reg);
	i3ddma_enable_reg.cap0_seq_blk_sel = 0; //seq mode
	//i3ddma_enable_reg.frc_en = 0;
	i3ddma_enable_reg.lr_separate_en = 1;
	i3ddma_enable_reg.tg_en = 1;
	i3ddma_enable_reg.uzd_mux_sel = 0;
	IoReg_Write32(H3DDMA_I3DDMA_enable_reg, i3ddma_enable_reg.regValue );
	rtd_pr_vt_notice("H3DDMA_I3DDMA_enable_reg = %x\n", IoReg_Read32(H3DDMA_I3DDMA_enable_reg));

	h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg);
	h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_dma_enable = 1;
	rtd_outl(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg, h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue);
}

void vt_i3ddma_cap1cap2_vi_isr(void)
{
	if(get_VT_Pixel_Format() == VT_CAP_NV12)
	{
		if(get_vt_VtBufferNum() == 3)
		{
			//set cap1(Y) cap2(UV) capture address
			h3ddma_cap1_wr_dma_num_bl_wrap_addr_0_RBUS h3ddma_cap1_dma_addr0_reg;
			//h3ddma_cap2_wr_dma_num_bl_wrap_addr_0_RBUS h3mdda_cap2_dma_addr0_reg;
			unsigned int yBufSize = CaptureCtrl_VT.cap_buffer[0].size;
			unsigned int ucBufAddr = 0;
			unsigned int wrIdx = 0;
			
			h3ddma_cap1_dma_addr0_reg.regValue = IoReg_Read32(H3DDMA_CAP1_WR_DMA_num_bl_wrap_addr_0_reg);
			//h3mdda_cap2_dma_addr0_reg.regValue = IoReg_Read32(H3DDMA_CAP2_WR_DMA_num_bl_wrap_addr_0_reg);
			
			if(h3ddma_cap1_dma_addr0_reg.regValue == CaptureCtrl_VT.cap_buffer[0].phyaddr)
			{
				h3ddma_cap1_dma_addr0_reg.regValue = CaptureCtrl_VT.cap_buffer[1].phyaddr;
				wrIdx = 1;
			}
			else if(h3ddma_cap1_dma_addr0_reg.regValue == CaptureCtrl_VT.cap_buffer[1].phyaddr)
			{
				h3ddma_cap1_dma_addr0_reg.regValue = CaptureCtrl_VT.cap_buffer[2].phyaddr;
				wrIdx = 2;
			}
			else if(h3ddma_cap1_dma_addr0_reg.regValue == CaptureCtrl_VT.cap_buffer[2].phyaddr)
			{
				h3ddma_cap1_dma_addr0_reg.regValue = CaptureCtrl_VT.cap_buffer[0].phyaddr;
				wrIdx = 0;
			}

			if(h3ddma_cap1_dma_addr0_reg.regValue == VtFreezeRegAddr)
			{
				if(h3ddma_cap1_dma_addr0_reg.regValue == CaptureCtrl_VT.cap_buffer[0].phyaddr)
				{
					h3ddma_cap1_dma_addr0_reg.regValue = CaptureCtrl_VT.cap_buffer[1].phyaddr;
					wrIdx = 1;
				}
				else if(h3ddma_cap1_dma_addr0_reg.regValue == CaptureCtrl_VT.cap_buffer[1].phyaddr)
				{
					h3ddma_cap1_dma_addr0_reg.regValue = CaptureCtrl_VT.cap_buffer[2].phyaddr;
					wrIdx = 2;
				}
				else if(h3ddma_cap1_dma_addr0_reg.regValue == CaptureCtrl_VT.cap_buffer[2].phyaddr)
				{
					h3ddma_cap1_dma_addr0_reg.regValue = CaptureCtrl_VT.cap_buffer[0].phyaddr;
					wrIdx = 0;
				}
			}
			
			IoReg_Write32(H3DDMA_CAP1_WR_DMA_num_bl_wrap_addr_0_reg, h3ddma_cap1_dma_addr0_reg.regValue);
			ucBufAddr = _ALIGN((CaptureCtrl_VT.cap_buffer[0].phyaddr + yBufSize * 3 + (yBufSize/2) * wrIdx), __4KPAGE);
    		IoReg_Write32(H3DDMA_CAP2_WR_DMA_num_bl_wrap_addr_0_reg, ucBufAddr);
	
		}
	}
}

void i3ddma_cap0_seq_VtCap_freeze(unsigned char enable)
{
	if(TRUE == enable)
	{
		if(get_vt_VtBufferNum() == 3)
		{
			h3ddma_cap0_cap_status_RBUS h3ddma_cap0_cap_status_Reg;
			//rtd_pr_vt_notice("h3ddma_cap_status_Reg = %x\n", IoReg_Read32(H3DDMA_CAP0_Cap_Status_reg));

			h3ddma_cap0_cap_status_Reg.regValue = IoReg_Read32(H3DDMA_CAP0_Cap_Status_reg); 

			// HW double buffer,hw just use 0 and 1
			if(h3ddma_cap0_cap_status_Reg.cap0_cap_block_sel == 0) /* cap0 is accessing address in addr_0_reg*/
			{
				VtFreezeRegAddr = IoReg_Read32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_1_reg);
				IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_1_reg, IoReg_Read32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_2_reg));
				//IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_r1_reg, IoReg_Read32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_2_reg));
				IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_2_reg, VtFreezeRegAddr);
			}
			else if(h3ddma_cap0_cap_status_Reg.cap0_cap_block_sel == 1)
			{
				VtFreezeRegAddr = IoReg_Read32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_0_reg);
				IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_0_reg, IoReg_Read32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_2_reg));
				//IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_r0_reg, IoReg_Read32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_2_reg));
				IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_2_reg, VtFreezeRegAddr);
			}
			rtd_pr_vt_notice("cap0 freeze done!\n");

			if(VtFreezeRegAddr == CaptureCtrl_VT.cap_buffer[0].phyaddr)
				IndexOfFreezedVideoFrameBuffer = 0;
			else if(VtFreezeRegAddr == CaptureCtrl_VT.cap_buffer[1].phyaddr)
				IndexOfFreezedVideoFrameBuffer = 1;
			else
				IndexOfFreezedVideoFrameBuffer = 2;
				
			rtd_pr_vt_notice("cap0 freezed buffer Idx = %d\n", IndexOfFreezedVideoFrameBuffer);
			rtd_pr_vt_notice("H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_0_reg = %x\n", IoReg_Read32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_0_reg));
			rtd_pr_vt_notice("H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_1_reg = %x\n", IoReg_Read32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_1_reg));
			rtd_pr_vt_notice("H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_2_reg = %x\n", IoReg_Read32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_2_reg));
		}
		else if(get_vt_VtBufferNum() == 1)
		{
			h3ddma_cap0_cap_status_RBUS h3ddma_cap0_cap_status_Reg;
			h3ddma_cap0_cti_dma_wr_ctrl_RBUS h3ddma_cap0_cti_dma_wr_ctrl_reg;
			unsigned int timeoutcount = 100;
			h3ddma_cap0_cap_status_Reg.regValue = IoReg_Read32(H3DDMA_CAP0_Cap_Status_reg); 		
			h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg);

			h3ddma_cap0_cap_status_Reg.regValue = IoReg_Read32(H3DDMA_CAP0_Cap_Status_reg);
			h3ddma_cap0_cap_status_Reg.cap0_cap_last_wr_flag = 1;
			IoReg_Write32(H3DDMA_CAP0_Cap_Status_reg, h3ddma_cap0_cap_status_Reg.regValue);
			
			do{
				if(H3DDMA_CAP0_Cap_Status_get_cap0_cap_last_wr_flag(IoReg_Read32(H3DDMA_CAP0_Cap_Status_reg))){
					//When capture starts writing last data of one frame to DDR, this flag will go high.
					h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_dma_enable = 0; //disable capture enable
					IoReg_Write32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg, h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue);
					IndexOfFreezedVideoFrameBuffer = 0;
					break;
				}
				else
				{
					timeoutcount--;
					msleep(0);
				}
				
			}while(timeoutcount);	
				
			if(timeoutcount == 0)
			{
				rtd_pr_vt_notice("wait cap0_cap_last_wr_flag timeout\n");
			}
		}
	}
	else
	{
		if(get_vt_VtBufferNum() == 1)
		{
			h3ddma_cap0_cap_status_RBUS h3ddma_cap0_cap_status_Reg;
			h3ddma_cap0_cti_dma_wr_ctrl_RBUS h3ddma_cap0_cti_dma_wr_ctrl_reg;

			IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_0_reg, CaptureCtrl_VT.cap_buffer[0].phyaddr);
			
			h3ddma_cap0_cap_status_Reg.regValue = IoReg_Read32(H3DDMA_CAP0_Cap_Status_reg);
			h3ddma_cap0_cap_status_Reg.cap0_cap_last_wr_flag = 1; //clear first
			IoReg_Write32(H3DDMA_CAP0_Cap_Status_reg, h3ddma_cap0_cap_status_Reg.regValue);

			h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg);
			h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_dma_enable = 1; // capture enable
			IoReg_Write32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg, h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue);
		}
		else
			VtFreezeRegAddr = 0;
	}
				
}

void i3ddma_cap1cap2_vi_VtCap_freeze(unsigned char enable)
{
	rtd_pr_vt_notice("func:%s,%d\n",__FUNCTION__,enable);
	if(TRUE == enable)
	{ //calculate last capture done buffer,which send to ap
		if(get_vt_VtBufferNum() == 3)
		{
			h3ddma_cap1_wr_dma_num_bl_wrap_addr_0_RBUS h3ddma_cap1_dma_addr0_reg;
			h3ddma_cap2_wr_dma_num_bl_wrap_addr_0_RBUS h3mdda_cap2_dma_addr0_reg;
		  
		  	unsigned int lastcapbufferaddr = 0;
		
		  	h3ddma_cap1_dma_addr0_reg.regValue = IoReg_Read32(H3DDMA_CAP1_WR_DMA_num_bl_wrap_addr_0_reg);
			h3mdda_cap2_dma_addr0_reg.regValue = IoReg_Read32(H3DDMA_CAP2_WR_DMA_num_bl_wrap_addr_0_reg);

		  // calculate last capture done buffer 
		  if(h3ddma_cap1_dma_addr0_reg.regValue == CaptureCtrl_VT.cap_buffer[0].phyaddr)
			  lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[2].phyaddr;
		  else if(h3ddma_cap1_dma_addr0_reg.regValue == CaptureCtrl_VT.cap_buffer[1].phyaddr)
			  lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[0].phyaddr;
		  else if(h3ddma_cap1_dma_addr0_reg.regValue == CaptureCtrl_VT.cap_buffer[2].phyaddr)
			  lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[1].phyaddr;

		  // if lastcapbufferaddr is last freezed buffer, re-calculate
		  if(lastcapbufferaddr == VtFreezeRegAddr)
			  {
				 if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr){
						 lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[2].phyaddr;
				 }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr){
						 lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[0].phyaddr;
				 }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[2].phyaddr){
						 lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[1].phyaddr;
				 }
			 }
			
			 if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr){
					 IndexOfFreezedVideoFrameBuffer = 0;
			 }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr){
					 IndexOfFreezedVideoFrameBuffer = 1;
			 }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[2].phyaddr){
					 IndexOfFreezedVideoFrameBuffer = 2;
			 }
			 
			VtFreezeRegAddr = lastcapbufferaddr;
		}
	}
	else
		VtFreezeRegAddr = 0;
}


void vt_i3ddma_vi_capture_enable(unsigned int enable)
{
	//	h3ddma_cap0_cti_dma_wr_ctrl_RBUS h3ddma_cap0_cti_dma_wr_ctrl_reg;
	h3ddma_cap1_cti_dma_wr_ctrl_RBUS h3ddma_cap1_cti_dma_wr_ctrl_reg;
	h3ddma_cap2_cti_dma_wr_ctrl_RBUS h3ddma_cap2_cti_dma_wr_ctrl_reg;
//	h3ddma_cap3_cti_dma_wr_ctrl_RBUS h3ddma_cap3_cti_dma_wr_ctrl_reg;

#if 0
	h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg);
	h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_dma_enable=enable;
	IoReg_Write32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg, h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue);
#endif
	h3ddma_cap1_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP1_CTI_DMA_WR_Ctrl_reg);
	h3ddma_cap1_cti_dma_wr_ctrl_reg.cap1_dma_enable=enable;
	IoReg_Write32(H3DDMA_CAP1_CTI_DMA_WR_Ctrl_reg, h3ddma_cap1_cti_dma_wr_ctrl_reg.regValue);
	
	h3ddma_cap2_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP2_CTI_DMA_WR_Ctrl_reg);
	h3ddma_cap2_cti_dma_wr_ctrl_reg.cap2_dma_enable=enable;
	IoReg_Write32(H3DDMA_CAP2_CTI_DMA_WR_Ctrl_reg, h3ddma_cap2_cti_dma_wr_ctrl_reg.regValue);

#if 0
	h3ddma_cap3_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP3_CTI_DMA_WR_Ctrl_reg);
	h3ddma_cap3_cti_dma_wr_ctrl_reg.cap3_dma_enable=enable;
	IoReg_Write32(H3DDMA_CAP3_CTI_DMA_WR_Ctrl_reg, h3ddma_cap3_cti_dma_wr_ctrl_reg.regValue);	
#endif
}

void drvif_i3ddma_cap1cap2_config(unsigned int width, unsigned int height)
{
	h3ddma_i3ddma_enable_RBUS					   h3ddma_i3ddma_enable_reg;
	h3ddma_cap1_cap_ctl0_RBUS					   h3ddma_cap1_cap_ctl0_reg;
	h3ddma_cap1_wr_dma_num_bl_wrap_word_RBUS	   h3ddma_cap1_wr_dma_num_bl_wrap_word_reg;
	h3ddma_cap1_wr_dma_num_bl_wrap_ctl_RBUS 	   h3ddma_cap1_wr_dma_num_bl_wrap_ctl_reg;
	h3ddma_cap1_wr_dma_num_bl_wrap_line_step_RBUS  h3ddma_cap1_wr_dma_num_bl_wrap_line_step_reg;
	h3ddma_cap2_cap_ctl0_RBUS					   h3ddma_cap2_cap_ctl0_reg;
	h3ddma_cap2_wr_dma_num_bl_wrap_word_RBUS	   h3ddma_cap2_wr_dma_num_bl_wrap_word_reg;
	h3ddma_cap2_wr_dma_pxltobus_RBUS			   h3ddma_cap2_wr_dma_pxltobus_reg;
	h3ddma_cap2_wr_dma_num_bl_wrap_ctl_RBUS 	   h3ddma_cap2_wr_dma_num_bl_wrap_ctl_reg;
	h3ddma_cap2_wr_dma_num_bl_wrap_line_step_RBUS  h3ddma_cap2_wr_dma_num_bl_wrap_line_step_reg;
//	h3ddma_lr_separate_ctrl2_RBUS				   h3ddma_lr_separate_ctrl2_reg;
	h3ddma_lr_separate_ctrl3_RBUS				   h3ddma_lr_separate_ctrl3_reg;
//	h3ddma_lr_separate_ctrl1_RBUS				   h3ddma_lr_separate_ctrl1_reg;

	h3ddma_cap1_cti_dma_wr_ctrl_RBUS			   h3ddma_cap1_cti_dma_wr_ctrl_reg;
	h3ddma_cap2_cti_dma_wr_ctrl_RBUS			   h3ddma_cap2_cti_dma_wr_ctrl_reg;

	
	UINT32 bitsPerLine = width * 8;
	UINT32 fifoWidth = 128;
	UINT32 fifoElementsPerLine = 0;


	if (bitsPerLine % fifoWidth)
		  fifoElementsPerLine = (UINT32) (bitsPerLine/fifoWidth) + 1;
	else
		  fifoElementsPerLine = (UINT32) (bitsPerLine/fifoWidth);


	// Disable double bfffer
	IoReg_ClearBits(H3DDMA_H3DDMA_PQ_CMP_DOUBLE_BUFFER_CTRL_reg, _BIT0);

	//cap1cap2 block mode
	h3ddma_i3ddma_enable_reg.regValue = IoReg_Read32(H3DDMA_I3DDMA_enable_reg);
	h3ddma_i3ddma_enable_reg.cap1_seq_blk_sel = 1; //blk mode
	h3ddma_i3ddma_enable_reg.cap2_seq_blk_sel = 1;
	IoReg_Write32(H3DDMA_I3DDMA_enable_reg, h3ddma_i3ddma_enable_reg.regValue);

	h3ddma_cap1_cap_ctl0_reg.regValue = IoReg_Read32(H3DDMA_CAP1_Cap_CTL0_reg);
	h3ddma_cap1_cap_ctl0_reg.cap1_auto_block_sel_en = 0;
	IoReg_Write32(H3DDMA_CAP1_Cap_CTL0_reg, h3ddma_cap1_cap_ctl0_reg.regValue);
	h3ddma_cap2_cap_ctl0_reg.regValue = IoReg_Read32(H3DDMA_CAP2_Cap_CTL0_reg);
	h3ddma_cap2_cap_ctl0_reg.cap2_auto_block_sel_en = 0;//FW set memory start address
	IoReg_Write32(H3DDMA_CAP2_Cap_CTL0_reg, h3ddma_cap2_cap_ctl0_reg.regValue);

	//data_y
	h3ddma_cap1_wr_dma_num_bl_wrap_word_reg.regValue	  = IoReg_Read32(H3DDMA_CAP1_WR_DMA_num_bl_wrap_word_reg);
	h3ddma_cap1_wr_dma_num_bl_wrap_word_reg.cap1_line_burst_num = fifoElementsPerLine;
	IoReg_Write32(H3DDMA_CAP1_WR_DMA_num_bl_wrap_word_reg, h3ddma_cap1_wr_dma_num_bl_wrap_word_reg.regValue);

	h3ddma_cap1_wr_dma_num_bl_wrap_ctl_reg.regValue    = IoReg_Read32(H3DDMA_CAP1_WR_DMA_num_bl_wrap_ctl_reg);
	h3ddma_cap1_wr_dma_num_bl_wrap_ctl_reg.cap1_burst_len = 32;
	h3ddma_cap1_wr_dma_num_bl_wrap_ctl_reg.cap1_line_num = height;
	IoReg_Write32( H3DDMA_CAP1_WR_DMA_num_bl_wrap_ctl_reg, h3ddma_cap1_wr_dma_num_bl_wrap_ctl_reg.regValue);

	h3ddma_cap1_wr_dma_num_bl_wrap_line_step_reg.regValue = IoReg_Read32(H3DDMA_CAP1_WR_DMA_num_bl_wrap_line_step_reg);
	h3ddma_cap1_wr_dma_num_bl_wrap_line_step_reg.cap1_line_step = fifoElementsPerLine; 
	IoReg_Write32(H3DDMA_CAP1_WR_DMA_num_bl_wrap_line_step_reg, h3ddma_cap1_wr_dma_num_bl_wrap_line_step_reg.regValue);

	//data_c
	h3ddma_cap2_wr_dma_num_bl_wrap_word_reg.regValue	   = IoReg_Read32(H3DDMA_CAP2_WR_DMA_num_bl_wrap_word_reg);
	h3ddma_cap2_wr_dma_num_bl_wrap_word_reg.cap2_line_burst_num = fifoElementsPerLine;
	IoReg_Write32(H3DDMA_CAP2_WR_DMA_num_bl_wrap_word_reg, h3ddma_cap2_wr_dma_num_bl_wrap_word_reg.regValue);

	h3ddma_cap2_wr_dma_pxltobus_reg.regValue = IoReg_Read32(H3DDMA_CAP2_WR_DMA_pxltobus_reg);
	h3ddma_cap2_wr_dma_pxltobus_reg.cap2_pixel_encoding = 2; //4:2:0
	IoReg_Write32(H3DDMA_CAP2_WR_DMA_pxltobus_reg, h3ddma_cap2_wr_dma_pxltobus_reg.regValue);
	h3ddma_cap2_wr_dma_num_bl_wrap_ctl_reg.regValue    = IoReg_Read32(H3DDMA_CAP2_WR_DMA_num_bl_wrap_ctl_reg);
	if(h3ddma_cap2_wr_dma_pxltobus_reg.cap2_pixel_encoding == 2)
		h3ddma_cap2_wr_dma_num_bl_wrap_ctl_reg.cap2_line_num = height/2;
	else
		h3ddma_cap2_wr_dma_num_bl_wrap_ctl_reg.cap2_line_num = height;
	h3ddma_cap2_wr_dma_num_bl_wrap_ctl_reg.cap2_burst_len = 32;
	IoReg_Write32(H3DDMA_CAP2_WR_DMA_num_bl_wrap_ctl_reg, h3ddma_cap2_wr_dma_num_bl_wrap_ctl_reg.regValue);

	h3ddma_cap2_wr_dma_num_bl_wrap_line_step_reg.regValue = IoReg_Read32(H3DDMA_CAP2_WR_DMA_num_bl_wrap_line_step_reg);
	h3ddma_cap2_wr_dma_num_bl_wrap_line_step_reg.cap2_line_step = fifoElementsPerLine; 
	IoReg_Write32(H3DDMA_CAP2_WR_DMA_num_bl_wrap_line_step_reg, h3ddma_cap2_wr_dma_num_bl_wrap_line_step_reg.regValue);

	h3ddma_lr_separate_ctrl3_reg.regValue = IoReg_Read32(H3DDMA_LR_Separate_CTRL3_reg);
	h3ddma_lr_separate_ctrl3_reg.hact2 = width;
	h3ddma_lr_separate_ctrl3_reg.vact2 = height;
	IoReg_Write32(H3DDMA_LR_Separate_CTRL3_reg,h3ddma_lr_separate_ctrl3_reg.regValue);

	//byte swap
	
	h3ddma_cap1_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP1_CTI_DMA_WR_Ctrl_reg);
	h3ddma_cap1_cti_dma_wr_ctrl_reg.cap1_dma_8byte_swap = 1;
	h3ddma_cap1_cti_dma_wr_ctrl_reg.cap1_dma_4byte_swap = 1;
	h3ddma_cap1_cti_dma_wr_ctrl_reg.cap1_dma_2byte_swap = 1;
	h3ddma_cap1_cti_dma_wr_ctrl_reg.cap1_dma_1byte_swap = 1;
	IoReg_Write32(H3DDMA_CAP1_CTI_DMA_WR_Ctrl_reg,h3ddma_cap1_cti_dma_wr_ctrl_reg.regValue);

	h3ddma_cap2_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP2_CTI_DMA_WR_Ctrl_reg);
	h3ddma_cap2_cti_dma_wr_ctrl_reg.cap2_dma_8byte_swap = 1;
	h3ddma_cap2_cti_dma_wr_ctrl_reg.cap2_dma_4byte_swap = 1;
	h3ddma_cap2_cti_dma_wr_ctrl_reg.cap2_dma_2byte_swap = 1;
	h3ddma_cap2_cti_dma_wr_ctrl_reg.cap2_dma_1byte_swap = 1;
	IoReg_Write32(H3DDMA_CAP2_CTI_DMA_WR_Ctrl_reg,h3ddma_cap2_cti_dma_wr_ctrl_reg.regValue);
}

unsigned char drvif_i3ddmavi_vtconfig(unsigned int nWidth, unsigned int nLength, unsigned int data_format)
{
/* VT use i3ddma capture dispD data, if ap needs nv12 format, use i3ddma cap1,cap2 to do vi block capture */
	h3ddma_i3ddma_enable_RBUS h3ddma_i3ddma_enable_reg;
	//dma_vgip_dma_vgip_ctrl_RBUS dma_vgip_chn1_ctrl_reg;
	
	h3ddma_lr_separate_ctrl1_RBUS lr_separate_ctrl1_reg;
	h3ddma_tg_v1_end_RBUS tg_v1_end_reg;
	
	//SIZE insize,outsize;
	unsigned int size_video = 0, allocSize = 0;
	unsigned int unit_size = 0;
	unsigned long mallocphysicaladdr = 0;
	unsigned int buffnum = 0;
	unsigned int ucBufAddr = 0, uvIdx1 = 0, uvIdx2 = 0;
	unsigned int align96mallocphysicaladdr = 0;
	int i;

	buffnum = get_vt_VtBufferNum();
	unit_size = _ALIGN((VT_CAP_FRAME_WIDTH_4K2K*VT_CAP_FRAME_HEIGHT_4K2K + VT_CAP_FRAME_WIDTH_4K2K*VT_CAP_FRAME_HEIGHT_4K2K/2),__4KPAGE);
//disable dma
	vt_i3ddma_vi_capture_enable(0);

	//for nv12 re-allocate memory
	if(CaptureCtrl_VT.cap_buffer[0].phyaddr != 0)
	{
		rtd_pr_vt_notice("[VT] free memory when nv12 capture\n");
		Capture_BufferMemDeInit_VT(buffnum);
	}
	
	
	//size_video = _ALIGN((VT_CAP_FRAME_WIDTH_4K2K*VT_CAP_FRAME_HEIGHT_4K2K),__4KPAGE);
	size_video = (VT_CAP_FRAME_WIDTH_4K2K*VT_CAP_FRAME_HEIGHT_4K2K);
	
	rtd_pr_vt_notice("[VT]re-allocate memory when nv12 capture,size_y=0x%x\n", size_video);
	
	//allocSize = (size_video + size_video / 2) * buffnum;
	allocSize = (unit_size) * buffnum;

	rtd_pr_vt_notice("[VT]nv12 capture, buffer info: wid:%d,height:%d,totalsize:%x\n", VT_CAP_FRAME_WIDTH_4K2K,VT_CAP_FRAME_HEIGHT_4K2K,allocSize);

	mallocphysicaladdr = pli_malloc(allocSize, GFP_DCU2_FIRST);
	if(INVALID_VAL == mallocphysicaladdr)
	{
		rtd_pr_vt_notice("[error]:nv12 alloc VT memory failed[non-dc2h path]\n");
		return FALSE;
		
	}else{
		VtAllocatedBufferStartAdress[0] = mallocphysicaladdr;
		align96mallocphysicaladdr = _ALIGN(mallocphysicaladdr,__4KPAGE);
		for (i = 0; i < buffnum; i++) 
		{
			if(CaptureCtrl_VT.cap_buffer[i].phyaddr == 0)
			{
				CaptureCtrl_VT.cap_buffer[i].cache = NULL;
				CaptureCtrl_VT.cap_buffer[i].phyaddr =	(align96mallocphysicaladdr + i*size_video);
				CaptureCtrl_VT.cap_buffer[i].size = size_video;
				rtd_pr_vt_notice("[VT memory allocate] buf[%d]=(%d KB), phy(%lx) \n", i, CaptureCtrl_VT.cap_buffer[i].size >> 10, (unsigned long)CaptureCtrl_VT.cap_buffer[i].phyaddr);
			}
		}
	

#if 0
		for(i = 0; i < buffnum; i++)
		{
			if(CaptureCtrl_VT.cap_buffer[i].cache == NULL)
			{
				CaptureCtrl_VT.cap_buffer[i].cache = (void*)(malloccacheaddr + i*size_video);
				//CaptureCtrl_VT.cap_buffer[i].phyaddr =  (unsigned int)virt_to_phys((void*)CaptureCtrl_VT.cap_buffer[i].cache);
				CaptureCtrl_VT.cap_buffer[i].phyaddr =	(unsigned int)dvr_to_phys((void*)CaptureCtrl_VT.cap_buffer[i].cache);
				CaptureCtrl_VT.cap_buffer[i].size = size_video;
				rtd_pr_vt_notice("[VT memory allocate] buf[%d]=(%d KB), phy(%lx) \n", i, CaptureCtrl_VT.cap_buffer[i].size >> 10, (unsigned long)CaptureCtrl_VT.cap_buffer[i].phyaddr);
		   }
		}
#endif
	}

	//set cap1 (Y) , cap2 (UV) boundary address
	
	IoReg_Write32(H3DDMA_CAP1_CTI_DMA_WR_Rule_check_low_reg, CaptureCtrl_VT.cap_buffer[0].phyaddr);
	IoReg_Write32(H3DDMA_CAP1_CTI_DMA_WR_Rule_check_up_reg, CaptureCtrl_VT.cap_buffer[0].phyaddr + size_video * buffnum);
		
	IoReg_Write32(H3DDMA_CAP2_CTI_DMA_WR_Rule_check_low_reg, CaptureCtrl_VT.cap_buffer[0].phyaddr + size_video * buffnum);
	IoReg_Write32(H3DDMA_CAP2_CTI_DMA_WR_Rule_check_up_reg, CaptureCtrl_VT.cap_buffer[0].phyaddr + allocSize);

	rtd_pr_vt_notice("nv12 capture output size(%d,%d),datafmt:%d\n",nWidth, nLength, data_format);

	
	// set cap1 cap2 capture register
	drvif_i3ddma_cap1cap2_config(nWidth, nLength);

	//set cap1(Y) cap2(UV) capture address
	IoReg_Write32(H3DDMA_CAP1_WR_DMA_num_bl_wrap_addr_0_reg, CaptureCtrl_VT.cap_buffer[0].phyaddr);
	ucBufAddr = _ALIGN((CaptureCtrl_VT.cap_buffer[0].phyaddr + size_video * buffnum), __4KPAGE);
    IoReg_Write32(H3DDMA_CAP2_WR_DMA_num_bl_wrap_addr_0_reg, ucBufAddr);
	uvIdx1 = _ALIGN((CaptureCtrl_VT.cap_buffer[0].phyaddr + size_video * buffnum + (size_video/2) * 1), __4KPAGE);
	uvIdx2 = _ALIGN((CaptureCtrl_VT.cap_buffer[0].phyaddr + size_video * buffnum + size_video), __4KPAGE);
	rtd_pr_vt_notice("nv12 index=0,uv phy addr=0x%x,%x,%x\n", ucBufAddr, uvIdx1, uvIdx2);
		
	// IDMA LR separate control
	lr_separate_ctrl1_reg.regValue = IoReg_Read32(H3DDMA_LR_Separate_CTRL1_reg);
	lr_separate_ctrl1_reg.hdmi420_en = 0;
	lr_separate_ctrl1_reg.vs_inv = 0;
	lr_separate_ctrl1_reg.hs_inv = 0;
	lr_separate_ctrl1_reg.progressive = 1;
	lr_separate_ctrl1_reg.hdmi_3d_structure = 0xa; // frame 2D
	IoReg_Write32(H3DDMA_LR_Separate_CTRL1_reg, lr_separate_ctrl1_reg.regValue);

	tg_v1_end_reg.regValue = IoReg_Read32(H3DDMA_TG_v1_end_reg);
	tg_v1_end_reg.pullup_en = 0;
	tg_v1_end_reg.tg_hend1 = 2;
	tg_v1_end_reg.tg_vend1 = 2;
	IoReg_Write32(H3DDMA_TG_v1_end_reg, tg_v1_end_reg.regValue);

	// IDMA timeing gen enable
	h3ddma_i3ddma_enable_reg.regValue = IoReg_Read32(H3DDMA_I3DDMA_enable_reg);
	//h3ddma_i3ddma_enable_reg.frc_en = 0;
	h3ddma_i3ddma_enable_reg.lr_separate_en = 1; 
	h3ddma_i3ddma_enable_reg.tg_en = 1;
	h3ddma_i3ddma_enable_reg.uzd_mux_sel = 0; //cap0 bypass uzd,make sure cap1cap2 use uzd
	IoReg_Write32(H3DDMA_I3DDMA_enable_reg, h3ddma_i3ddma_enable_reg.regValue);

	/*dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	if (dma_vgip_chn1_ctrl_reg.dma_vact_end_ie == 1)
		VT_vactend_ie = TRUE;
	else
		VT_vactend_ie = FALSE;
	dma_vgip_chn1_ctrl_reg.dma_vact_end_ie = 1;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);*/
	
	//I3DDMA_ultrazoom_config_scaling_down_for_NN(&insize, &outsize, 1);
	//VtBlockCapFlag = TRUE;
	//enable dma
	vt_i3ddma_vi_capture_enable(1);
	return TRUE;
}

void I3DDMA_ultrazoom_set_scale_down_VtCapture(SIZE* in_Size, SIZE* out_Size, unsigned char panorama)
{

	unsigned char SDRatio;
	unsigned char SDFilter=0;
	unsigned int tmp_data;
	short *coef_pt;
	unsigned char i;
	unsigned int nFactor;
	unsigned char Hini, Vini;

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	h3ddma_hsd_i3ddma_vsd_ctrl0_RBUS				   i3ddma_vsd_ctrl0_reg;
	h3ddma_hsd_i3ddma_hsd_ctrl0_RBUS				i3ddma_hsd_ctrl0_reg;
	h3ddma_hsd_i3ddma_hsd_scale_hor_factor_RBUS i3ddma_hsd_scale_hor_factor_reg;
	h3ddma_hsd_i3ddma_vsd_scale_ver_factor_RBUS 	i3ddma_vsd_Scale_Ver_Factor_reg;
	h3ddma_hsd_i3ddma_hsd_hor_segment_RBUS		i3ddma_hsd_hor_segment_reg;
   // h3ddma_hsd_i3ddma_hsd_hor_delta1_RBUS 		i3ddma_hsd_hor_delta1_reg;
	h3ddma_hsd_i3ddma_hsd_initial_value_RBUS		i3ddma_hsd_initial_value_reg;
	h3ddma_hsd_i3ddma_vsd_initial_value_RBUS		i3ddma_vsd_initial_value_reg;

	i3ddma_vsd_ctrl0_reg.regValue		= IoReg_Read32(H3DDMA_HSD_I3DDMA_VSD_Ctrl0_reg);
	i3ddma_hsd_ctrl0_reg.regValue		= IoReg_Read32(H3DDMA_HSD_I3DDMA_HSD_Ctrl0_reg);
	i3ddma_hsd_scale_hor_factor_reg.regValue	= IoReg_Read32(H3DDMA_HSD_I3DDMA_HSD_Scale_Hor_Factor_reg);
	i3ddma_vsd_Scale_Ver_Factor_reg.regValue = IoReg_Read32(H3DDMA_HSD_I3DDMA_VSD_Scale_Ver_Factor_reg);
	i3ddma_hsd_hor_segment_reg.regValue = IoReg_Read32(H3DDMA_HSD_I3DDMA_HSD_Hor_Segment_reg);
   // i3ddma_hsd_hor_delta1_reg.regValue	= IoReg_Read32(H3DDMA_HSD_I3DDMA_HSD_Hor_Delta1_reg);
	i3ddma_hsd_initial_value_reg.regValue	= IoReg_Read32(H3DDMA_HSD_I3DDMA_HSD_Initial_Value_reg);
	i3ddma_vsd_initial_value_reg.regValue = IoReg_Read32(H3DDMA_HSD_I3DDMA_VSD_Initial_Value_reg);


	rtd_pr_vt_emerg("I3DDMA_ultrazoom_set_scale_down\n");
	//o============ H scale-down=============o
	if (in_Size->nWidth > out_Size->nWidth) {
		// o-------calculate scaledown ratio to select one of different bandwith filters.--------o
		if ( out_Size->nWidth == 0 ) {
			rtd_pr_vt_notice("[VT]output width = 0 !!!\n");
			SDRatio = 0;
		} else {
			SDRatio = (in_Size->nWidth*TMPMUL) / out_Size->nWidth;
		}

		//UltraZoom_Printf("CSW SDRatio number=%d\n",SDRatio);

		if(SDRatio <= ((TMPMUL*3)/2))	
			SDFilter = 2;
		else if(SDRatio <= (TMPMUL*2) && SDRatio > ((TMPMUL*3)/2) ) // Mid
			SDFilter = 1;
		else		// blurest, narrow bw
			SDFilter = 0;

		//o---------------- fill coefficients into access port--------------------o
		coef_pt = tScaleDown_COEF_TAB[SDFilter];

		i3ddma_hsd_ctrl0_reg.h_y_table_sel = 0; // TAB1
		i3ddma_hsd_ctrl0_reg.h_c_table_sel = 0; // TAB1

		for (i=0; i<16; i++)
		{
			tmp_data = ((unsigned int)(*coef_pt++)<<16);
			tmp_data += (unsigned int)(*coef_pt++);
			IoReg_Write32(H3DDMA_HSD_I3DDMA_UZD_FIR_Coef_Tab1_C0_reg+ i*4, tmp_data);
		}
	}

#if 0
	b802560c=0x00000000;
	b802592c=0x04000000;//coef0=1
	b8025948=0x00000000;//coef1=0
	b802594c=0x00000000;//coef2=0
	b8025968=0x00000000;//coef3=0

	0xb8025600[6][7]=0


	// b802592c=0x04000000;//coef0=1
	IoReg_Write32(H3DDMA_HSD_I3DDMA_UZD_FIR_Coef_Tab1_C0_reg, 0x04000000);
	// b8025948=0x00000000;//coef14=0
	IoReg_Write32(H3DDMA_HSD_I3DDMA_UZD_FIR_Coef_Tab1_C14_reg, 0x00000000);
	// b802594c=0x00000000;//coef16=0
	IoReg_Write32(H3DDMA_HSD_I3DDMA_UZD_FIR_Coef_Tab1_C16_reg, 0x00000000);
	// b8025968=0x00000000;//coef30=0
	IoReg_Write32(H3DDMA_HSD_I3DDMA_UZD_FIR_Coef_Tab1_C30_reg, 0x00000000);
#endif

	//o============ V scale-down=============o
	if (in_Size->nLength > out_Size->nLength) {
		// o-------calculate scaledown ratio to select one of different bandwith filters.--------o

		 //jeffrey 961231
		if ( out_Size->nLength == 0 ) {
			SDRatio = 0;
		} else {
			SDRatio = (in_Size->nLength*TMPMUL) / out_Size->nLength;
		}

		//UltraZoom_Printf("CSW SDRatio number=%d\n",SDRatio);

	#if 1
		SDFilter = 4;// when i3ddma pqc, i3ddma vsd can only repeat
	#else
		if(SDRatio <= ((TMPMUL*3)/2))	
			SDFilter = 2;
		else if(SDRatio <= (TMPMUL*2) && SDRatio > ((TMPMUL*3)/2) ) // Mid
			SDFilter = 1;
		else		// blurest, narrow bw
			SDFilter = 0;
	#endif

		//o---------------- fill coefficients into access port--------------------o
		coef_pt = tScaleDown_COEF_TAB[SDFilter];

		i3ddma_vsd_ctrl0_reg.v_y_table_sel = 1; // TAB2
		i3ddma_vsd_ctrl0_reg.v_c_table_sel = 1; // TAB2

		for (i=0; i<16; i++)
		{
			tmp_data = ((unsigned int)(*coef_pt++)<<16);
			tmp_data += (unsigned int)(*coef_pt++);
			IoReg_Write32(H3DDMA_HSD_I3DDMA_UZD_FIR_Coef_Tab2_C0_reg+ i*4, tmp_data);
		}

	}


//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	if (in_Size->nWidth > out_Size->nWidth) {	 // H scale-down
		Hini = 0x80;// actual phase is 0x180;
		i3ddma_hsd_initial_value_reg.hor_ini = Hini;
		i3ddma_hsd_initial_value_reg.hor_ini_int = 1;
		{
			//frank@0108 add for code exception
			if ( out_Size->nWidth == 0 ) {
				rtd_pr_vt_info("output width = 0 !!!\n");
				nFactor = 0;
			} else if(in_Size->nWidth>4095){
				//nFactor = (unsigned int)((((in_Size->nWidth-1)<<19) / (out_Size->nWidth-1))<<1);
				nFactor = (unsigned int)((((in_Size->nWidth)<<19) / (out_Size->nWidth))<<1);
			} else if(in_Size->nWidth>2047){
				//nFactor = (unsigned int)(((in_Size->nWidth-1)<<20) / (out_Size->nWidth-1));
				nFactor = (unsigned int)(((in_Size->nWidth)<<20) / (out_Size->nWidth));
			} else {
				nFactor = (unsigned int)((in_Size->nWidth<<21)) / (out_Size->nWidth);
				nFactor = SHR(nFactor + 1, 1); //rounding
			}
		}

		i3ddma_hsd_scale_hor_factor_reg.hor_fac = nFactor;
	}else {
		i3ddma_hsd_scale_hor_factor_reg.hor_fac = 0x100000;
	}
	i3ddma_hsd_hor_segment_reg.nl_seg2 = out_Size->nWidth;

	if (in_Size->nLength > out_Size->nLength) {    // V scale-down
//			ich2_uzd_Ctrl0_REG.buffer_mode = 0x02;
		Vini = 0x80;// actual 0x180;
		//i3ddma_vsd_initial_value_reg.ver_ini = Vini;
		//i3ddma_vsd_initial_value_reg.ver_ini_l = 0x80;
		i3ddma_vsd_initial_value_reg.ver_ini = Vini;
		i3ddma_vsd_Scale_Ver_Factor_reg.ver_ini_int = 1;
		//i3ddma_vsd_Scale_Ver_Factor_reg.ver_ini_int_l = 1;
		//nFactor = (unsigned int)((ptInSize->nLength << 20) - (Vini<<12)) / (ptOutSize->nLength - 1);
		//frank@0108 add for code exception
		if ( out_Size->nLength == 0 ) {
			rtd_pr_vt_info("output length = 0 !!!\n");
			nFactor = 0;
		} else {
			//nFactor = (unsigned int)((in_Size->nLength << 21)) / (out_Size->nLength);
			//modify the calculation for V-scale down @Crixus 20160804
			nFactor = (unsigned int)((in_Size->nLength * 1024 * 1024)) / (out_Size->nLength);
		}
		//nFactor = SHR(nFactor+1, 1); //rounding
		i3ddma_vsd_Scale_Ver_Factor_reg.ver_fac = nFactor;
	}else{
//			ich2_uzd_Ctrl0_REG.buffer_mode = 0;
		i3ddma_vsd_Scale_Ver_Factor_reg.ver_fac = 0x100000;
	}

//	ich2_uzd_Ctrl0_REG.output_fmt = 1;
//	CLR_422_Fr_SD();

	IoReg_Write32(H3DDMA_HSD_I3DDMA_VSD_Ctrl0_reg, i3ddma_vsd_ctrl0_reg.regValue);
	IoReg_Write32(H3DDMA_HSD_I3DDMA_HSD_Ctrl0_reg, i3ddma_hsd_ctrl0_reg.regValue);
	IoReg_Write32(H3DDMA_HSD_I3DDMA_HSD_Scale_Hor_Factor_reg, i3ddma_hsd_scale_hor_factor_reg.regValue);
	IoReg_Write32(H3DDMA_HSD_I3DDMA_VSD_Scale_Ver_Factor_reg, i3ddma_vsd_Scale_Ver_Factor_reg.regValue);
	IoReg_Write32(H3DDMA_HSD_I3DDMA_HSD_Hor_Segment_reg, i3ddma_hsd_hor_segment_reg.regValue);
	//IoReg_Write32(H3DDMA_HSD_I3DDMA_HSD_Hor_Delta1_reg, i3ddma_hsd_hor_delta1_reg.regValue);
	IoReg_Write32(H3DDMA_HSD_I3DDMA_VSD_Initial_Value_reg, i3ddma_vsd_initial_value_reg.regValue);
	IoReg_Write32(H3DDMA_HSD_I3DDMA_HSD_Initial_Value_reg, i3ddma_hsd_initial_value_reg.regValue);

	return;
}


void I3DDMA_ultrazoom_config_scaling_down_VtCapture(SIZE* in_Size, SIZE* out_Size, unsigned char panorama)
{
	
	h3ddma_hsd_i3ddma_hsd_ctrl0_RBUS i3ddma_hsd_ctrl0_reg;
	h3ddma_hsd_i3ddma_vsd_ctrl0_RBUS i3ddma_vsd_ctrl0_reg;

	i3ddma_hsd_ctrl0_reg.regValue = IoReg_Read32(H3DDMA_HSD_I3DDMA_HSD_Ctrl0_reg);
	i3ddma_hsd_ctrl0_reg.h_zoom_en = (in_Size->nWidth > out_Size->nWidth);
	IoReg_Write32(H3DDMA_HSD_I3DDMA_HSD_Ctrl0_reg, i3ddma_hsd_ctrl0_reg.regValue);

	i3ddma_vsd_ctrl0_reg.regValue = IoReg_Read32(H3DDMA_HSD_I3DDMA_VSD_Ctrl0_reg);
	i3ddma_vsd_ctrl0_reg.v_zoom_en = (in_Size->nLength > out_Size->nLength);
	if(i3ddma_vsd_ctrl0_reg.v_zoom_en)
		i3ddma_vsd_ctrl0_reg.buffer_mode = 2;//do V-scale down have to use v-scale down buffer mode.
	else
		i3ddma_vsd_ctrl0_reg.buffer_mode = 0;
	
	if(get_VT_Pixel_Format() == VT_CAP_NV12 || get_VT_Pixel_Format() == VT_CAP_NV16)
		i3ddma_vsd_ctrl0_reg.sort_fmt = 0;
	else
		i3ddma_vsd_ctrl0_reg.sort_fmt = 1;
	i3ddma_vsd_ctrl0_reg.truncationctrl = 1;
	i3ddma_vsd_ctrl0_reg.video_comp_en = 0;
	IoReg_Write32(H3DDMA_HSD_I3DDMA_VSD_Ctrl0_reg, i3ddma_vsd_ctrl0_reg.regValue);

	I3DDMA_ultrazoom_set_scale_down_VtCapture(in_Size, out_Size, panorama);

}

static unsigned short tRGB2YUV_COEF_709_RGB_0_255[] = {
	/* CCIR 709 RGB*/
	0x0368,	/* m11*/
	0x0b71,  /* m12*/
	0x0127,  /* m13*/

	0x7e1f,   /* m21 >> 2 0x01D5 up bits at AT Name: cychen2 , Date: 2010/3/25*/
	0x79b2,   /* m22 >> 1*/
	0x082f,   /* m23 >> 2*/
	0x082f,   /* m31 >> 2*/
	0x7891,   /* m32 >> 2 0x0195  up bits at AT Name: cychen2 , Date: 2010/3/25*/
	0x7f40,   /* m33 >> 1*/

	0x0000, 	/* Yo_even*/
	0x0000, 	/* Yo_odd*/
	0x0100, 	/* Y_gain*/
	0x0001,  /* sel_RGB*/
	0x0000,  /* sel_Yin_offset*/
	0x0001,  /* sel_UV_out_offset*/
	0x0000,  /* sel_UV_off*/
	0x0000,  /* Matrix_bypass*/
	0x0000,  /* Enable_Y_gain*/
};
	
void disable_422to_444_en(void)
{
	h3ddma_rgb2yuv_dither_4xxto4xx_hdmi_422to444_ctrl_RBUS i3ddma_rgb2yuv_dither_4xxto4xx_hdmi_4xxto4xx_ctrl_reg;
	i3ddma_rgb2yuv_dither_4xxto4xx_hdmi_4xxto4xx_ctrl_reg.regValue = rtd_inl(H3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_422to444_CTRL_reg);
	i3ddma_rgb2yuv_dither_4xxto4xx_hdmi_4xxto4xx_ctrl_reg.db_en = 0;
	i3ddma_rgb2yuv_dither_4xxto4xx_hdmi_4xxto4xx_ctrl_reg.en_422to444 = 0;
	IoReg_Write32(H3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_422to444_CTRL_reg, i3ddma_rgb2yuv_dither_4xxto4xx_hdmi_4xxto4xx_ctrl_reg.regValue);
}

void I3DDMA_color_colorspacergb2yuvtransfer_VtCapture(VT_DISPD_I3DDMACAP_SRC idmaDispD_in_sel)
{
	unsigned short *table_index = 0;
	h3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_ctrl_RBUS  i3ddma_rgb2yuv_ctrl_reg;

	h3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_tab_m11_m12_RBUS tab2_M11_M12_REG;
	h3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_tab_m13_m21_RBUS tab2_M13_M21_REG;
	h3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_tab_m22_m23_RBUS tab2_M22_M23_REG;
	h3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_tab_m31_m32_RBUS tab2_M31_M32_REG;
	h3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_tab_m33_ygain_RBUS tab2_M33_YGain_REG;
	h3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_tab_yo_RBUS tab2_Yo_REG;

	table_index = tRGB2YUV_COEF_709_RGB_0_255;

	
	tab2_M11_M12_REG.m11 = table_index [_RGB2YUV_m11];
	tab2_M11_M12_REG.m12 = table_index [_RGB2YUV_m12];
	IoReg_Write32(H3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_TAB_M11_M12_reg, tab2_M11_M12_REG.regValue);

	tab2_M13_M21_REG.m13 = table_index [_RGB2YUV_m13];
	tab2_M13_M21_REG.m21 = table_index [_RGB2YUV_m21];
	IoReg_Write32(H3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_TAB_M13_M21_reg, tab2_M13_M21_REG.regValue);

	tab2_M22_M23_REG.m22 = table_index [_RGB2YUV_m22];
	tab2_M22_M23_REG.m23 = table_index [_RGB2YUV_m23];
	IoReg_Write32(H3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_TAB_M22_M23_reg, tab2_M22_M23_REG.regValue);

	tab2_M31_M32_REG.m31 = table_index [_RGB2YUV_m31];
	tab2_M31_M32_REG.m32 = table_index [_RGB2YUV_m32];
	IoReg_Write32(H3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_TAB_M31_M32_reg, tab2_M31_M32_REG.regValue);

	tab2_M33_YGain_REG.m33 = table_index [_RGB2YUV_m33];
	tab2_M33_YGain_REG.y_gain= table_index [_RGB2YUV_Y_gain];
	IoReg_Write32(H3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_TAB_M33_YGain_reg, tab2_M33_YGain_REG.regValue);

	//tab2_Yo_and_Y_Gain_REG.yo_even= table_index [_RGB2YUV_Yo_even];
	//tab2_Yo_and_Y_Gain_REG.yo_odd= table_index [_RGB2YUV_Yo_odd];
	tab2_Yo_REG.yo_odd= table_index [_RGB2YUV_Yo_odd];
	IoReg_Write32(H3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_TAB_Yo_reg, tab2_Yo_REG.regValue);

	i3ddma_rgb2yuv_ctrl_reg.regValue = IoReg_Read32(H3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_CTRL_reg);
	
	i3ddma_rgb2yuv_ctrl_reg.sel_rgb= table_index [_RGB2YUV_sel_RGB];
	i3ddma_rgb2yuv_ctrl_reg.set_r_in_offset= table_index [_RGB2YUV_set_Yin_offset];
	i3ddma_rgb2yuv_ctrl_reg.set_uv_out_offset= table_index [_RGB2YUV_set_UV_out_offset];
	i3ddma_rgb2yuv_ctrl_reg.sel_uv_off= table_index [_RGB2YUV_sel_UV_off];
	i3ddma_rgb2yuv_ctrl_reg.matrix_bypass= table_index [_RGB2YUV_Matrix_bypass];
	i3ddma_rgb2yuv_ctrl_reg.sel_y_gain= table_index [_RGB2YUV_Enable_Y_gain];

	if((get_VT_Pixel_Format() == VT_CAP_NV12) || (get_VT_Pixel_Format() == VT_CAP_NV16) || (get_VT_Pixel_Format() == VT_CAP_YUV888)) //target capture format
	{
		if(idmaDispD_in_sel == I3DDMACAP_SRC_UZU || idmaDispD_in_sel == I3DDMACAP_SRC_UZU_SUB)//if current capture location already is yuv 
			i3ddma_rgb2yuv_ctrl_reg.en_rgb2yuv = 0;
		else
			i3ddma_rgb2yuv_ctrl_reg.en_rgb2yuv = 1; //Enable RGB to YUV conversion (YUV out)
	}
	else // capture position is after yuv2rgb block
	{
		i3ddma_rgb2yuv_ctrl_reg.en_rgb2yuv = 0; //Disable color conversion (GBR out)
	}

	rtd_pr_vt_notice("[VT] en_rgb2yuv=%d\n", i3ddma_rgb2yuv_ctrl_reg.en_rgb2yuv);

	IoReg_Write32(H3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_CTRL_reg, i3ddma_rgb2yuv_ctrl_reg.regValue);
	if(get_VT_Pixel_Format() == VT_CAP_RGB888)
	{
		if(idmaDispD_in_sel == I3DDMACAP_SRC_YUV2RGB || idmaDispD_in_sel == I3DDMACAP_SRC_OSD)
			disable_422to_444_en();
	}
}

unsigned char is_yuv_cap_in_non_full_screen(VT_DISPD_I3DDMACAP_SRC position)
{
	ppoverlay_main_active_h_start_end_RBUS main_active_h_start_end_Reg;
	ppoverlay_main_active_v_start_end_RBUS main_active_v_start_end_Reg;
	unsigned int main_act_wid = 0, main_act_height = 0;
	
	main_active_h_start_end_Reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_Active_H_Start_End_reg);
	main_act_wid = (main_active_h_start_end_Reg.mh_act_end - main_active_h_start_end_Reg.mh_act_sta);

	main_active_v_start_end_Reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_Active_V_Start_End_reg);
	main_act_height = (main_active_v_start_end_Reg.mv_act_end - main_active_v_start_end_Reg.mv_act_sta);
	
	if((get_VT_Pixel_Format() == VT_CAP_NV12) || (get_VT_Pixel_Format() == VT_CAP_NV16))
	{
		if((position == I3DDMACAP_SRC_UZU) && ((main_act_wid < _DISP_WID) || (main_act_height < _DISP_LEN)))
		{
			rtd_pr_vt_notice("is_yuv_cap_in_non_full_screen\n");
			return TRUE;
		}
	}
	return FALSE;
	
}

void I3DDMA_Set_Vgip_VtCapture(unsigned char src, VT_DISPD_I3DDMACAP_SRC dispD_sel, unsigned char mode)
{

	dma_vgip_dma_vgip_ctrl_RBUS dma_vgip_chn1_ctrl_reg;
	dma_vgip_dma_vgip_act_hsta_width_RBUS dma_vgip_chn1_act_hsta_width_reg;
	dma_vgip_dma_vgip_act_vsta_length_RBUS dma_vgip_chn1_act_vsta_length_reg;
	dma_vgip_dma_vgip_delay_RBUS  dma_vgip_chn1_delay_reg;
	dma_vgip_dma_vgip_cts_fifo_ctl_RBUS  dma_cts_fifo_ctl_reg;
	ppoverlay_dh_den_start_end_RBUS ppoverlay_dh_den_start_end_Reg;
	ppoverlay_dv_den_start_end_RBUS ppoverlay_dv_den_start_end_Reg;

	ppoverlay_main_active_h_start_end_RBUS main_active_h_start_end_Reg;
	ppoverlay_main_active_v_start_end_RBUS main_active_v_start_end_Reg;
	
	ppoverlay_dh_den_start_end_Reg.regValue = IoReg_Read32(PPOVERLAY_DH_DEN_Start_End_reg);
	ppoverlay_dv_den_start_end_Reg.regValue = IoReg_Read32(PPOVERLAY_DV_DEN_Start_End_reg);

	main_active_h_start_end_Reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_Active_H_Start_End_reg);
	main_active_v_start_end_Reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_Active_V_Start_End_reg);

	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_in_clk_en = 0;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);

	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_in_sel = src;
	dma_vgip_chn1_ctrl_reg.dma_digital_mode = mode;
	dma_vgip_chn1_ctrl_reg.dma_field_sync_edge = 1;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);

	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_in_clk_en = 1;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);


	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_ivrun = 1;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);


	dma_vgip_chn1_act_hsta_width_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg);
	dma_vgip_chn1_act_vsta_length_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_ACT_VSTA_Length_reg);

	if(is_yuv_cap_in_non_full_screen(dispD_sel) == FALSE){

		if(dispD_sel == I3DDMACAP_SRC_UZU || dispD_sel == I3DDMACAP_SRC_SUBTITLE || dispD_sel == I3DDMACAP_SRC_OSD ||dispD_sel == I3DDMACAP_SRC_DITHER || dispD_sel == I3DDMACAP_SRC_MEMC_MUX || dispD_sel == I3DDMACAP_SRC_YUV2RGB)
	 	{
	  		dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_sta = ppoverlay_dh_den_start_end_Reg.dh_den_sta;
	  		dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_wid = ppoverlay_dh_den_start_end_Reg.dh_den_end - ppoverlay_dh_den_start_end_Reg.dh_den_sta;
	  		dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_sta = ppoverlay_dv_den_start_end_Reg.dv_den_sta;
	  		dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_len= ppoverlay_dv_den_start_end_Reg.dv_den_end - ppoverlay_dv_den_start_end_Reg.dv_den_sta;
	 	}
	}
	else
	{
		dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
		dma_vgip_chn1_ctrl_reg.dma_digital_mode = VGIP_MODE_DIGITAL;
		IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);
		
  		dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_sta = 2;
  		dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_wid = main_active_h_start_end_Reg.mh_act_end - main_active_h_start_end_Reg.mh_act_sta;
  		dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_sta = 1; //main_active_v_start_end_Reg.mv_act_sta;
  		dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_len= main_active_v_start_end_Reg.mv_act_end - main_active_v_start_end_Reg.mv_act_sta;
	}

	IoReg_Write32(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg, dma_vgip_chn1_act_hsta_width_reg.regValue);
	IoReg_Write32(DMA_VGIP_DMA_VGIP_ACT_VSTA_Length_reg, dma_vgip_chn1_act_vsta_length_reg.regValue);

	// IHS/IVS delay
	dma_vgip_chn1_delay_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_DELAY_reg);
	dma_vgip_chn1_delay_reg.dma_ihs_dly = 0;
	dma_vgip_chn1_delay_reg.dma_ivs_dly = 0;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_DELAY_reg, dma_vgip_chn1_delay_reg.regValue);

	dma_cts_fifo_ctl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTS_FIFO_CTL_reg);
	dma_cts_fifo_ctl_reg.dma_en_ctsfifo_vsrst_vgip = 1;
	dma_cts_fifo_ctl_reg.dma_force_ctsfifo_rstn_vgip = 1;
	dma_cts_fifo_ctl_reg.dma_force_ctsfifo_rstn_sdnr = 1;
	dma_cts_fifo_ctl_reg.dma_en_ctsfifo_vsrst_sdnr = 0;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTS_FIFO_CTL_reg, dma_cts_fifo_ctl_reg.regValue);

	rtd_pr_vt_notice("DMA_VGIP_DMA_VGIP_CTRL_reg = %x\n", IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg));
	rtd_pr_vt_notice("DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg = %x\n", IoReg_Read32(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg));
	rtd_pr_vt_notice("DMA_VGIP_DMA_VGIP_ACT_VSTA_Length_reg = %x\n", IoReg_Read32(DMA_VGIP_DMA_VGIP_ACT_VSTA_Length_reg));
}

void disable_i3_comp_en(void)
{
 
    h3ddma_h3ddma_pq_cmp_double_buffer_ctrl_RBUS h3ddma_h3ddma_pq_cmp_double_buffer_ctrl_reg;
    h3ddma_h3ddma_pq_cmp_RBUS h3ddma_h3ddma_pq_cmp_reg;
   
    h3ddma_h3ddma_pq_cmp_double_buffer_ctrl_reg.regValue = IoReg_Read32(H3DDMA_H3DDMA_PQ_CMP_DOUBLE_BUFFER_CTRL_reg);
    h3ddma_h3ddma_pq_cmp_double_buffer_ctrl_reg.db_en = 0;
    IoReg_Write32(H3DDMA_H3DDMA_PQ_CMP_DOUBLE_BUFFER_CTRL_reg, h3ddma_h3ddma_pq_cmp_double_buffer_ctrl_reg.regValue);
   
    h3ddma_h3ddma_pq_cmp_reg.regValue = IoReg_Read32(H3DDMA_H3DDMA_PQ_CMP_reg);
    h3ddma_h3ddma_pq_cmp_reg.cmp_en = 0;
    IoReg_Write32(H3DDMA_H3DDMA_PQ_CMP_reg, h3ddma_h3ddma_pq_cmp_reg.regValue);
}

unsigned char drvif_I3DDMA_dispD_VtCaptureConfig(VT_CUR_CAPTURE_INFO capInfo, unsigned char waitbufferfresh)
{
	unsigned int waitbufferms = 100;//50hz 2 vsync waiting
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
	struct timespec64 timeout_s;
#else
	struct timespec timeout_s;
#endif

	if(capInfo.enable == TRUE) {
		VT_DISPD_I3DDMACAP_SRC dispD_sel = I3DDMACAP_SRC_MEMC_MUX;
		SIZE inSize,outSize;
		ppoverlay_dh_den_start_end_RBUS ppoverlay_dh_den_start_end_Reg;
		ppoverlay_dv_den_start_end_RBUS ppoverlay_dv_den_start_end_Reg;
		
		ppoverlay_main_active_h_start_end_RBUS main_active_h_start_end_Reg;
		ppoverlay_main_active_v_start_end_RBUS main_active_v_start_end_Reg;
		unsigned int timeoutselsetting;
		unsigned long flags;//for spin lock_irqsave	
		ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;
		
	
		if(KADP_VT_SCALER_OUTPUT == VTDumpLocation)
			dispD_sel = I3DDMACAP_SRC_UZU;   /*not support uzu mux for rgb888,no yuv2rgb ip, support yuv2rgb */
		else if(KADP_VT_DISPLAY_OUTPUT == VTDumpLocation)
			dispD_sel = I3DDMACAP_SRC_YUV2RGB; 
		else if(KADP_VT_OSDVIDEO_OUTPUT == VTDumpLocation)
			dispD_sel = I3DDMACAP_SRC_OSD; /*after PQ block */

		rtd_pr_vt_notice("drvif_I3DDMA_dispD_VtCaptureConfig dispD_sel:%d;%s=%d \n",dispD_sel, __FUNCTION__, __LINE__);	
		ppoverlay_dh_den_start_end_Reg.regValue = IoReg_Read32(PPOVERLAY_DH_DEN_Start_End_reg);
		ppoverlay_dv_den_start_end_Reg.regValue = IoReg_Read32(PPOVERLAY_DV_DEN_Start_End_reg);
		main_active_h_start_end_Reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_Active_H_Start_End_reg);
		main_active_v_start_end_Reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_Active_V_Start_End_reg);
			
		if(/*dispD_sel == I3DDMACAP_SRC_UZU ||*/ dispD_sel == I3DDMACAP_SRC_SUBTITLE || dispD_sel == I3DDMACAP_SRC_OSD ||dispD_sel == I3DDMACAP_SRC_DITHER || dispD_sel == I3DDMACAP_SRC_MEMC_MUX || dispD_sel == I3DDMACAP_SRC_YUV2RGB)
		{
			inSize.nWidth = ppoverlay_dh_den_start_end_Reg.dh_den_end - ppoverlay_dh_den_start_end_Reg.dh_den_sta;
			inSize.nLength = ppoverlay_dv_den_start_end_Reg.dv_den_end - ppoverlay_dv_den_start_end_Reg.dv_den_sta;
		} 
		else if(dispD_sel == I3DDMACAP_SRC_UZU)
		{		
			inSize.nWidth = main_active_h_start_end_Reg.mh_act_end - main_active_h_start_end_Reg.mh_act_sta;
			inSize.nLength = main_active_v_start_end_Reg.mv_act_end - main_active_v_start_end_Reg.mv_act_sta;
		}
		
		outSize.nWidth = capInfo.capWid;
		outSize.nLength = capInfo.capLen;
		rtd_pr_vt_notice("drvif_I3DDMA_dispD_VtCaptureConfig outSize.nWidth:%d; outSize.nLength = %d;inSize.nWidth=%d;inSize.nLength=%d \n",outSize.nWidth,outSize.nLength,inSize.nWidth,inSize.nLength);

		display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
		timeoutselsetting = 0x3ffff;
		while(display_timing_ctrl2_reg.d2i3ddma_src_sel != dispD_sel) {
			down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
			spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock		
			if(PPOVERLAY_Double_Buffer_CTRL_get_dreg_dbuf_en(IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg)))
			{
				IoReg_ClearBits(PPOVERLAY_Double_Buffer_CTRL_reg, PPOVERLAY_Double_Buffer_CTRL_dreg_dbuf_en_mask);
				// IDMA VGIP DispD input source select
				display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
				display_timing_ctrl2_reg.d2i3ddma_src_sel = dispD_sel;
				IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);
				IoReg_SetBits(PPOVERLAY_Double_Buffer_CTRL_reg, PPOVERLAY_Double_Buffer_CTRL_dreg_dbuf_en_mask);
				rtd_pr_vt_notice("double buffer on:PPOVERLAY_Display_Timing_CTRL2_reg = %x\n", IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg));
				
			}
			else
			{
				// IDMA VGIP DispD input source select
				display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
				display_timing_ctrl2_reg.d2i3ddma_src_sel = dispD_sel;
				IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);
				rtd_pr_vt_notice("double buffer close:PPOVERLAY_Display_Timing_CTRL2_reg = %x\n", IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg));			
			}
			spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
			up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
			display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
			timeoutselsetting--;
			if(timeoutselsetting == 0){
				rtd_pr_vt_notice("set i3ddma sel timeout error!!!\n");
				break;
			}
		}
		rtd_pr_vt_notice("result:PPOVERLAY_Display_Timing_CTRL2_reg = %x\n", IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg));
		disable_i3_comp_en();

		/* dma_vgip > 4dither > yuv2rgb > hsd,vsd > 422_con > i3ddma cap*/
		I3DDMA_Set_Vgip_VtCapture(VGIP_SRC_DISPD, dispD_sel, VGIP_MODE_ANALOG);

		I3DDMA_color_colorspacergb2yuvtransfer_VtCapture(dispD_sel);

		/*IDMA scaling down setup*/
		I3DDMA_ultrazoom_config_scaling_down_VtCapture(&inSize, &outSize, 1);

		if(get_VT_Pixel_Format() == VT_CAP_NV12 || get_VT_Pixel_Format() == VT_CAP_NV16)
		{
			if(drvif_i3ddmavi_vtconfig(outSize.nWidth, outSize.nLength, 0) == FALSE)
				return FALSE;
		}
		/*else if (get_VT_Pixel_Format() == VT_CAP_NV16)
			drvif_i3ddmavi_vtconfig(outSize.nWidth, outSize.nLength, 1);*/
		else 
			drvif_i3ddma_cap0_seq_cap_forVt(outSize);

		if(waitbufferfresh == TRUE) {
			rtd_pr_vt_debug("waitbufferms = %d\n", waitbufferms);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
			timeout_s = ns_to_timespec64(waitbufferms * 1000 * 1000);
			hrtimer_nanosleep(&timeout_s,  HRTIMER_MODE_REL, CLOCK_MONOTONIC);
#else
			timeout_s = ns_to_timespec(waitbufferms * 1000 * 1000);
			hrtimer_nanosleep(&timeout_s, NULL, HRTIMER_MODE_REL, CLOCK_MONOTONIC);
#endif
		}
	} 
	else {
		drvif_I3DDMA_Set_VtCaptureDisable();
	}
	return TRUE;
}

#include <rtk_kdriver/tvscalercontrol/scalerdrv/scaler_hdr_ctrl.h>
extern SCALER_HDMI_DM_STATUS Scaler_Hdmi_DM_Get_Status(void);

unsigned char get_i3ddma_idlestatus(void)
{
	VSC_INPUT_TYPE_T srctype;

	srctype = Get_DisplayMode_Src(SLR_MAIN_DISPLAY);
#if (defined(CONFIG_ENABLE_HDR10_HDMI_AUTO_DETECT)||defined(CONFIG_ENABLE_DOLBY_VISION_HDMI_AUTO_DETECT))/*mac6 ic use i3ddma capture,and hdmi hdr source need use i3ddma, so not support capture when hdmi hdr*/	
	if((srctype == VSC_INPUTSRC_HDMI) && (Scaler_Hdmi_DM_Get_Status() == SLR_HDMI_DM_CONNECT))
	{
		rtd_pr_vt_notice("VT[info] I3DDMA is used by I3ddma video source,please use sub capture!;%s=%d \n", __FUNCTION__, __LINE__);
		return FALSE;
	}
#endif
	rtd_pr_vt_notice("VT[info] I3DDMA is idle use i3ddma capture!;%s=%d \n", __FUNCTION__, __LINE__);
	return TRUE;
}

#endif
unsigned char HAL_VT_InitEx(unsigned int buffernum)
{
#ifdef CONFIG_HW_SUPPORT_DC2H 
	ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;
#endif
	
	rtd_pr_vt_emerg("%s=%d buffernum=%d\n", __FUNCTION__, __LINE__,buffernum);

	down(&VT_Semaphore);
	
#ifdef CONFIG_HW_SUPPORT_DC2H 

	if(buffernum > VT_SUPPORT_MAX_CAP_BUF_NUM) /*max support 5 buffer mode*/
	{
		rtd_pr_vt_emerg("max support 5 buffer mode\n");
		up(&VT_Semaphore);
		return FALSE;
	}
	
	if ((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC)
		&&(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_STATE) ==  _MODE_STATE_ACTIVE)){
		if(get_vdec_securestatus()==TRUE){	
			rtd_pr_vt_emerg("[Warning vdecsvp]%s=%d Vdec source security is TRUE,could not do vt capture!\n", __FUNCTION__, __LINE__);
			up(&VT_Semaphore);
			return FALSE;
		}
	}

	display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
	if (display_timing_ctrl1_reg.disp_en == 0)/*means d domain clock off, stop capture*/
	{
		rtd_pr_vt_emerg("[Warning]D domain clock is off status,could not do vt capture!\n");
		up(&VT_Semaphore);
		return FALSE;	
	}

	if(((drvif_scaler_get_abnormal_dvs_long_flag()==TRUE)||(drvif_scaler_get_abnormal_dvs_short_flag()==TRUE))
		&& (get_vt_VtBufferNum() == 5)){ /* add vr360 condition(5 buffers),avoid capture out of range */
		rtd_pr_vt_emerg("[Warning VBE][VT] Vbe at abnormal dvs status,could not do vt capture!\n");
		up(&VT_Semaphore);
		return FALSE;			
	}
#endif

	if(get_vt_function() == FALSE) 
	{		
#ifndef CONFIG_HW_SUPPORT_DC2H 
		dma_vgip_dma_vgip_ctrl_RBUS dma_vgip_chn1_ctrl_reg;
#endif

		curCapInfo.enable = 1; //iMode;
		curCapInfo.capMode = VT_CAP_MODE_SEQ;
		curCapInfo.capSrc =  vfod_capture_location;  //0:scaler output 1:display output		
		curCapInfo.capWid = vfod_capture_out_W; //iWid;
		curCapInfo.capLen = vfod_capture_out_H;
	
#ifndef CONFIG_HW_SUPPORT_DC2H 
		dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
		if (dma_vgip_chn1_ctrl_reg.dma_vact_end_ie == 1)
			VT_vactend_ie = TRUE;
		else
			VT_vactend_ie = FALSE;
#endif	

		if (Capture_BufferMemInit_VT(buffernum) == FALSE) 
		{
			curCapInfo.enable = 0; //iMode;
			rtd_pr_vt_emerg("VT init allocate memory fail;%s=%d \n", __FUNCTION__, __LINE__);
			up(&VT_Semaphore);
			return FALSE;
		}
		set_vt_function(TRUE);
		set_vt_VtBufferNum(buffernum);
		set_vt_VtCaptureVDC(FALSE);
		if(buffernum == 1)
			set_vt_VtSwBufferMode(FALSE);
		else
			set_vt_VtSwBufferMode(TRUE);
	
#ifdef CONFIG_HW_SUPPORT_DC2H 

		drvif_DC2H_dispD_CaptureConfig(curCapInfo);
#else
#if 0
		if(get_i3ddma_idlestatus() == TRUE)
		{	//DispD2 i3ddma capture
			curCapInfo.capWid = 1920; //iWid;
			curCapInfo.capLen = 1080;			
			set_VT_Pixel_Format(VT_CAP_RGB888);
			drvif_I3DDMA_dispD_VtCaptureConfig(curCapInfo,FALSE);
		} 
		else 
		{
			curCapInfo.capWid = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_DI_WID); //iWid;
			curCapInfo.capLen = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_DI_LEN);			
			set_VT_Pixel_Format(VT_CAP_YUV888);
			drvif_sub_source_VtCaptureConfig(curCapInfo,FALSE);
		}
#endif

#endif
		VFODState.bEnabled = TRUE;
		VFODState.bAppliedPQ = TRUE;
		VFODState.bFreezed = FALSE;
		VFODState.framerateDivide = get_framerateDivide();
		rtd_pr_vt_notice("VT init Success;%s=%d \n", __FUNCTION__, __LINE__);
		up(&VT_Semaphore);
		return TRUE;
	}
	else 
	{
		rtd_pr_vt_notice("VT have inited,do nothing;%s=%d \n", __FUNCTION__, __LINE__);
		up(&VT_Semaphore);
		return TRUE;
	}
}

unsigned char HAL_VT_Init(void)
{
	rtd_pr_vt_emerg("%s=%d \n", __FUNCTION__, __LINE__);		

	if(FALSE == HAL_VT_InitEx(get_vt_VtBufferNum()))
		return FALSE;
	else
		return TRUE;
}

unsigned char HAL_VT_Finalize(void)
{
	down(&VT_Semaphore);
	if(get_vt_function()==TRUE) 
	{

#ifndef CONFIG_HW_SUPPORT_DC2H 
	dma_vgip_dma_vgip_ctrl_RBUS dma_vgip_chn1_ctrl_reg;
#endif

#ifdef ENABLE_VR360_DATA_FS_FLOW
		INPUT_TIMING_INDEX index = get_current_pattern_index();
		if(index == VO_4K2K_60){
			rtd_pr_vt_emerg("[func:%s] enter datafrc proc.\n", __FUNCTION__);
			scaler_vsc_vr360_enter_datafrc_proc();
		}
#endif
		rtd_pr_vt_emerg("%s=%d  start\n", __FUNCTION__, __LINE__);
		curCapInfo.enable = 0; //iMode;

#ifdef CONFIG_HW_SUPPORT_DC2H 

		drvif_DC2H_dispD_CaptureConfig(curCapInfo);
#else
		
		if(get_i3ddma_idlestatus() == TRUE)
			drvif_I3DDMA_dispD_VtCaptureConfig(curCapInfo,TRUE);
		else
			drvif_sub_source_VtCaptureConfig(curCapInfo,TRUE);

		
		if (VT_vactend_ie == FALSE)
		{
			dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
			dma_vgip_chn1_ctrl_reg.dma_vact_end_ie = 0;
			IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);
		}
#endif
		Capture_BufferMemDeInit_VT(get_vt_VtBufferNum());
		set_vt_VtBufferNum(3);
		IndexOfFreezedVideoFrameBuffer = 0;
		VtFreezeRegAddr = 0;
		//set_VT_Pixel_Format(VT_CAP_RGB888);
		set_vt_VtSwBufferMode(FALSE);
		VFODState.bEnabled = FALSE;
		VFODState.bFreezed = FALSE;
		VFODState.bAppliedPQ = TRUE;
		VFODState.framerateDivide = 1;
		VTDumpLocation = KADP_VT_MAX_LOCATION;
		set_vt_function(FALSE);
		rtd_pr_vt_emerg("VT finalize Success;%s=%d \n", __FUNCTION__, __LINE__);

#ifdef CONFIG_HW_SUPPORT_DC2H 
		if ((get_vt_EnableFRCMode()==FALSE))
		{
			//360vr close FRC, so end 360vr need reopen frc
			//please close memc here
			//MEMC_Set_malloc_address(TRUE);  /*mac7p no this function */
			set_vt_EnableFRCMode(TRUE);
			
			rtd_pr_vt_emerg("VT finalize EnableFRCMode;%s=%d \n", __FUNCTION__, __LINE__);
			if(get_film_mode_parameter().enable == _DISABLE){
				rtd_pr_vt_notice("[VR360] exit vt_frc_mode, set mc on!!\n");
				if(0 /*(Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE) > 2560) &&
					(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE) > 1440) &&
					(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 490) &&
					(Get_DISPLAY_REFRESH_RATE() != 120)*/){
					Scaler_MEMC_MEMC_CLK_OnOff(0, 1, 1);	//4k 50/60Hz, enable MC clock with mute on
				}
				else{
					Scaler_MEMC_MEMC_CLK_OnOff(1, 1, 1);	//not 4k 50/60Hz, enable MEMC clock with mute on
				}
				Scaler_MEMC_outMux(_ENABLE,_ENABLE);	//db on, mux on
			}
			
		}
#endif
		up(&VT_Semaphore);	
		return TRUE;
	} else {
		rtd_pr_vt_emerg("VT have finalized,do nothing;%s=%d \n", __FUNCTION__, __LINE__);
		up(&VT_Semaphore);
		return TRUE;
	}

}

unsigned char HAL_VT_GetDeviceCapability(KADP_VT_DEVICE_CAPABILITY_INFO_T *pDeviceCapabilityInfo)
{
	down(&VT_Semaphore);
	pDeviceCapabilityInfo->numOfVideoWindow = 1;
	up(&VT_Semaphore);
	return TRUE;
}


unsigned char HAL_VT_GetVideoFrameBufferCapability(KADP_VT_VIDEO_WINDOW_TYPE_T videoWindowID, KADP_VT_VIDEO_FRAME_BUFFER_CAPABILITY_INFO_T *pVideoFrameBufferCapabilityInfo)
{
	if(videoWindowID != KADP_VT_VIDEO_WINDOW_0) {
		rtd_pr_vt_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	down(&VT_Semaphore);
	pVideoFrameBufferCapabilityInfo ->numOfVideoFrameBuffer = get_vt_VtBufferNum();//i3ddma triple buffer mode

	if((get_VT_Pixel_Format() == VT_CAP_NV12)||(get_VT_Pixel_Format() == VT_CAP_NV16)){	
		pVideoFrameBufferCapabilityInfo ->numOfPlane = KADP_VT_VIDEO_FRAME_BUFFER_PLANE_SEMI_PLANAR;
	} else {
		pVideoFrameBufferCapabilityInfo ->numOfPlane = KADP_VT_VIDEO_FRAME_BUFFER_PLANE_INTERLEAVED;//sequence buffer
	}
	up(&VT_Semaphore);
	return TRUE;
}

unsigned char HAL_VT_GetVideoFrameOutputDeviceCapability(KADP_VT_VIDEO_WINDOW_TYPE_T videoWindowID, KADP_VT_VIDEO_FRAME_OUTPUT_DEVICE_CAPABILITY_INFO_T *pVideoFrameOutputDeviceCapabilityInfo)
{
	if(videoWindowID != KADP_VT_VIDEO_WINDOW_0) {
		rtd_pr_vt_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	down(&VT_Semaphore);
	pVideoFrameOutputDeviceCapabilityInfo ->bSupportedControlEnabled = TRUE;
	pVideoFrameOutputDeviceCapabilityInfo ->bSupportedControlFreezed = TRUE;//VI block mode disble endma
	pVideoFrameOutputDeviceCapabilityInfo ->bSupportedControlFramerateDivide = FALSE;
	pVideoFrameOutputDeviceCapabilityInfo ->bSupportedControlPQ = FALSE;
	up(&VT_Semaphore);
	return TRUE;
}

unsigned char HAL_VT_GetVideoFrameOutputDeviceLimitation(KADP_VT_VIDEO_WINDOW_TYPE_T videoWindowID, KADP_VT_VIDEO_FRAME_OUTPUT_DEVICE_LIMITATION_INFO_T *pVideoFrameOutputDeviceLimitationInfo)
{
	if(videoWindowID != KADP_VT_VIDEO_WINDOW_0) {
		rtd_pr_vt_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	down(&VT_Semaphore);
	pVideoFrameOutputDeviceLimitationInfo->maxResolution = (KADP_VT_RECT_T){0,0,VT_CAP_FRAME_WIDTH_4K2K,VT_CAP_FRAME_HEIGHT_4K2K};
	pVideoFrameOutputDeviceLimitationInfo->bLeftTopAlign = FALSE;
	pVideoFrameOutputDeviceLimitationInfo->bSupportInputVideoDeInterlacing =FALSE;
	pVideoFrameOutputDeviceLimitationInfo->bSupportDisplayVideoDeInterlacing = FALSE;
	pVideoFrameOutputDeviceLimitationInfo->bSupportScaleUp = FALSE;
	pVideoFrameOutputDeviceLimitationInfo->scaleUpLimitWidth = 0;
	pVideoFrameOutputDeviceLimitationInfo->scaleUpLimitHeight = 0;
	pVideoFrameOutputDeviceLimitationInfo->bSupportScaleDown = TRUE;
	pVideoFrameOutputDeviceLimitationInfo->scaleDownLimitWidth = 60;//nfactor upto 32
	pVideoFrameOutputDeviceLimitationInfo->scaleDownLimitHeight = 34;
	up(&VT_Semaphore);
	return TRUE;
}

unsigned char HAL_VT_GetAllVideoFrameBufferProperty(KADP_VT_VIDEO_WINDOW_TYPE_T videoWindowID, KADP_VT_VIDEO_FRAME_BUFFER_PROPERTY_INFO_T *pVideoFrameBufferProperty)
{
	if(videoWindowID != KADP_VT_VIDEO_WINDOW_0) {
		rtd_pr_vt_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	down(&VT_Semaphore);
	if(get_vt_function() == TRUE) {
		/*rtd_pr_vt_debug("Kernel: pVideoFrameBufferProperty->ppPhysicalAddress addr =%x\n", (unsigned int)(pVideoFrameBufferProperty->ppPhysicalAddress));*/

		if(get_VT_Pixel_Format() == VT_CAP_NV16){	
			pVideoFrameBufferProperty->pixelFormat = KADP_VT_VIDEO_FRAME_BUFFER_PIXEL_FORMAT_YUV422_SEMI_PLANAR;
			pVideoFrameBufferProperty->stride = drvif_memory_get_data_align(curCapInfo.capWid, 96);
			pVideoFrameBufferProperty->width = curCapInfo.capWid;		
		}else if(get_VT_Pixel_Format() == VT_CAP_NV12){ 
			pVideoFrameBufferProperty->pixelFormat = KADP_VT_VIDEO_FRAME_BUFFER_PIXEL_FORMAT_YUV420_SEMI_PLANAR;
			pVideoFrameBufferProperty->stride = drvif_memory_get_data_align(curCapInfo.capWid, 96);
			pVideoFrameBufferProperty->width = curCapInfo.capWid;		
		}else if((get_VT_Pixel_Format() == VT_CAP_ABGR8888)||(get_VT_Pixel_Format() == VT_CAP_ARGB8888)) {
			pVideoFrameBufferProperty->pixelFormat = KADP_VT_VIDEO_FRAME_BUFFER_PIXEL_FORMAT_ARGB;
			pVideoFrameBufferProperty->stride = drvif_memory_get_data_align(curCapInfo.capWid*4, 96);
			pVideoFrameBufferProperty->width = curCapInfo.capWid*4;
		} else {
			pVideoFrameBufferProperty->pixelFormat = KADP_VT_VIDEO_FRAME_BUFFER_PIXEL_FORMAT_RGB;
			pVideoFrameBufferProperty->stride = drvif_memory_get_data_align(curCapInfo.capWid*3, 96);
			pVideoFrameBufferProperty->width = curCapInfo.capWid*3;
		}

		pVideoFrameBufferProperty->height = curCapInfo.capLen;
		pVideoFrameBufferProperty->ppPhysicalAddress0 = CaptureCtrl_VT.cap_buffer[0].phyaddr;
		pVideoFrameBufferProperty->ppPhysicalAddress1 = CaptureCtrl_VT.cap_buffer[1].phyaddr;
		pVideoFrameBufferProperty->ppPhysicalAddress2 = CaptureCtrl_VT.cap_buffer[2].phyaddr;
		pVideoFrameBufferProperty->ppPhysicalAddress3 = CaptureCtrl_VT.cap_buffer[3].phyaddr;
		pVideoFrameBufferProperty->ppPhysicalAddress4 = CaptureCtrl_VT.cap_buffer[4].phyaddr;
		pVideoFrameBufferProperty->vfbbuffernumber = get_vt_VtBufferNum();
		up(&VT_Semaphore);
		return TRUE;
	} else {
		rtd_pr_vt_notice("VT is not Inited So return False;%s=%d\n", __FUNCTION__, __LINE__);
		up(&VT_Semaphore);
		return FALSE;
	}
}

unsigned char HAL_VT_GetVideoFrameBufferIndex(KADP_VT_VIDEO_WINDOW_TYPE_T videoWindowID, unsigned int *pIndexOfCurrentVideoFrameBuffer)
{
	if(videoWindowID != KADP_VT_VIDEO_WINDOW_0) {
		rtd_pr_vt_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	down(&VT_Semaphore);

#ifdef CONFIG_HW_SUPPORT_DC2H 	
	if((vfod_capture_location == KADP_VT_SCALER_OUTPUT) && (get_video_do_overscan_state()))
	{
		vt_source_capture_use_SE(vfod_capture_location);
		*pIndexOfCurrentVideoFrameBuffer = 0;
		
		up(&VT_Semaphore);		
		return TRUE;
	}
#endif

	if(get_vt_function() == TRUE) 
	{

		if(VFODState.bFreezed == TRUE) 
		{
#ifdef CONFIG_HW_SUPPORT_DC2H 	
			set_vfod_freezed_for_dc2h(FALSE);
#else
			set_vfod_freezed_for_i3ddma_sub_capture(FALSE);
#endif
			VFODState.bFreezed = FALSE;
		}
		
		if(VFODState.bFreezed == FALSE)
		{
#ifdef CONFIG_HW_SUPPORT_DC2H 	
			set_vfod_freezed_for_dc2h(TRUE);
#else
			set_vfod_freezed_for_i3ddma_sub_capture(TRUE);
#endif
			VFODState.bFreezed = TRUE;
		}

		rtd_pr_vt_notice("VFODState.bFreezed = %d\n", VFODState.bFreezed);
		if(VFODState.bFreezed == TRUE) 
		{
			*pIndexOfCurrentVideoFrameBuffer = IndexOfFreezedVideoFrameBuffer;
			rtd_pr_vt_notice("VFOD is freezed IndexOfFreezedVideoFrameBuffer = %d\n", IndexOfFreezedVideoFrameBuffer);
		}

		up(&VT_Semaphore);
		return TRUE;
	} 
	else 
	{
		rtd_pr_vt_notice("VT is not Inited So return False;%s=%d\n", __FUNCTION__, __LINE__);
		up(&VT_Semaphore);
		return FALSE;
	}
}

unsigned char HAL_VT_GetVideoFrameOutputDeviceState(KADP_VT_VIDEO_WINDOW_TYPE_T videoWindowID, KADP_VT_VIDEO_FRAME_OUTPUT_DEVICE_STATE_INFO_T *pVideoFrameOutputDeviceState)
{
	if(videoWindowID != KADP_VT_VIDEO_WINDOW_0) {
		rtd_pr_vt_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	down(&VT_Semaphore);
	*pVideoFrameOutputDeviceState = VFODState;
	up(&VT_Semaphore);
	if(get_vt_function() == TRUE) {
		return TRUE;
	} else {
		return FALSE;
	}
}

unsigned char HAL_VT_SetVideoFrameOutputDeviceState(KADP_VT_VIDEO_WINDOW_TYPE_T videoWindowID, KADP_VT_VIDEO_FRAME_OUTPUT_DEVICE_STATE_FLAGS_T videoFrameOutputDeviceStateFlags, KADP_VT_VIDEO_FRAME_OUTPUT_DEVICE_STATE_INFO_T *pVideoFrameOutputDeviceState)
{
	if(videoWindowID != KADP_VT_VIDEO_WINDOW_0) {
		rtd_pr_vt_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}

	if(videoFrameOutputDeviceStateFlags==KADP_VT_VIDEO_FRAME_OUTPUT_DEVICE_STATE_NOFX) {
		rtd_pr_vt_debug("KADP_VT_VIDEO_FRAME_OUTPUT_DEVICE_STATE_NOFX is do nothing\n");
		return TRUE;
	}
#if 0//not support aplly pq and framerate divide
	if(videoFrameOutputDeviceStateFlags&KADP_VT_VIDEO_FRAME_OUTPUT_DEVICE_STATE_FRAMERATE_DIVIDE) {
		rtd_pr_vt_info("KADP_VT_VIDEO_FRAME_OUTPUT_DEVICE_STATE_FRAMERATE_DIVIDE is not support\n");
		return FALSE;
	}

	if(videoFrameOutputDeviceStateFlags&KADP_VT_VIDEO_FRAME_OUTPUT_DEVICE_STATE_APPLIED_PQ) {
		if(pVideoFrameOutputDeviceState->bAppliedPQ)
			curCapInfo.capSrc =  KADP_VT_CAP_SRC_SUBTITLE;
		else
			curCapInfo.capSrc =  KADP_VT_CAP_SRC_UZU;
		VFODState.bAppliedPQ = pVideoFrameOutputDeviceState->bAppliedPQ;
		if(pVideoFrameOutputDeviceState->bAppliedPQ)
			VTDumpLocation = KADP_VT_DISPLAY_OUTPUT;
		else
			VTDumpLocation = KADP_VT_SCALER_OUTPUT;
	}
#endif
	down(&VT_Semaphore);
	if(get_vt_function() == TRUE) { 
		
		if (videoFrameOutputDeviceStateFlags&KADP_VT_VIDEO_FRAME_OUTPUT_DEVICE_STATE_ENABLED) {
			if (VFODState.bEnabled != pVideoFrameOutputDeviceState->bEnabled) {
				curCapInfo.enable = pVideoFrameOutputDeviceState->bEnabled;
				VFODState.bEnabled = pVideoFrameOutputDeviceState->bEnabled;

			#ifdef CONFIG_HW_SUPPORT_DC2H 
				drvif_DC2H_dispD_CaptureConfig(curCapInfo);
			#else			
			if(get_i3ddma_idlestatus() == TRUE)
					drvif_I3DDMA_dispD_VtCaptureConfig(curCapInfo,TRUE);
				else
					drvif_sub_source_VtCaptureConfig(curCapInfo,TRUE);
			#endif
			}
		}

		if(videoFrameOutputDeviceStateFlags&KADP_VT_VIDEO_FRAME_OUTPUT_DEVICE_STATE_FRAMERATE_DIVIDE) {
			rtd_pr_vt_debug("KADP_VT_VIDEO_FRAME_OUTPUT_DEVICE_STATE_FRAMERATE_DIVIDE is %d\n",pVideoFrameOutputDeviceState->framerateDivide);
			VFODState.framerateDivide = pVideoFrameOutputDeviceState->framerateDivide;
			
		}

		/*freeze do last*/
		if(videoFrameOutputDeviceStateFlags&KADP_VT_VIDEO_FRAME_OUTPUT_DEVICE_STATE_FREEZED) {
			if(get_vt_function() == TRUE) {
				if(pVideoFrameOutputDeviceState->bFreezed == VFODState.bFreezed) {
					rtd_pr_vt_debug("KADP_VT_VIDEO_FRAME_OUTPUT_DEVICE_STATE_FREEZED is the same as you setting\n");
				} else {
				
				#ifdef CONFIG_HW_SUPPORT_DC2H
					set_vfod_freezed_for_dc2h(pVideoFrameOutputDeviceState->bFreezed);
				#else
					set_vfod_freezed_for_i3ddma_sub_capture(pVideoFrameOutputDeviceState->bFreezed);
				#endif
				
					VFODState.bFreezed = pVideoFrameOutputDeviceState->bFreezed;
				}
			} else {
				rtd_pr_vt_notice("VFOD is not inited, so no freeze cmd!\n");
			}
		}
		up(&VT_Semaphore);
		return TRUE;
	} else {
		rtd_pr_vt_notice("VT is not Inited So return False;%s=%d\n", __FUNCTION__, __LINE__);
		up(&VT_Semaphore);
		return FALSE;
	}
}

/*qiangzhou:please refer modestate_decide_dtg_m_mode api SLR_INPUT_DTG_MASTER_V_FREQ setting*/
unsigned short Get_VFOD_FrameRate(void)
{
	unsigned short framerate = 0;

	framerate = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_DTG_MASTER_V_FREQ);

	if ((framerate != 50)&&(framerate != 60)&&(framerate != 48)) {
		
		rtd_pr_vt_notice("[warning] get SLR_INPUT_DTG_MASTER_V_FREQ not right!!!\n");
		framerate = 60;
	}

	if (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) == PIXEL_MODE_2_PIXEL) {
		if(framerate == 48){/*qiangzhou: not happen case, mention here*/
			framerate = 120;
		} else {	
			framerate = framerate*2;
		}
	}
	return framerate;

}

unsigned char HAL_VT_GetVideoFrameOutputDeviceFramerate(KADP_VT_VIDEO_WINDOW_TYPE_T videoWindowID, unsigned int *pVideoFrameOutputDeviceFramerate)
{
	if(videoWindowID != KADP_VT_VIDEO_WINDOW_0) {
		rtd_pr_vt_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	down(&VT_Semaphore);
	if(get_vt_function() == TRUE) {	
		*pVideoFrameOutputDeviceFramerate = Get_VFOD_FrameRate();
		up(&VT_Semaphore);
		return TRUE;
	} else {
		*pVideoFrameOutputDeviceFramerate = 0;
		rtd_pr_vt_notice("VT is not Inited So return pVideoFrameOutputDeviceFramerate 0;%s=%d\n", __FUNCTION__, __LINE__);
		up(&VT_Semaphore);
		return FALSE;
	}
}

unsigned char HAL_VT_GetVideoFrameOutputDeviceDumpLocation(KADP_VT_VIDEO_WINDOW_TYPE_T videoWindowID, KADP_VT_DUMP_LOCATION_TYPE_T *pDumpLocation)
{
	if(videoWindowID != KADP_VT_VIDEO_WINDOW_0) {
		rtd_pr_vt_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	down(&VT_Semaphore);
	if(get_vt_function() == TRUE) {
		*pDumpLocation = VTDumpLocation;
		up(&VT_Semaphore);
		return TRUE;
	} else {
		*pDumpLocation = VTDumpLocation;
		rtd_pr_vt_notice("[Warning]VT is not Inited when get dump location;%s=%d\n", __FUNCTION__, __LINE__);
		up(&VT_Semaphore);
		return FALSE;
	}
}

unsigned char HAL_VT_SetVideoFrameOutputDeviceDumpLocation(KADP_VT_VIDEO_WINDOW_TYPE_T videoWindowID, KADP_VT_DUMP_LOCATION_TYPE_T dumpLocation)
{
	if(videoWindowID != KADP_VT_VIDEO_WINDOW_0) {
		rtd_pr_vt_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	down(&VT_Semaphore);
	if(get_vt_function() == TRUE)
	{
		if(VTDumpLocation == dumpLocation) {
			rtd_pr_vt_notice("VTDumpLocation is the same as you setting\n");
		} 
		else 
		{
			curCapInfo.enable = 0; //close vfod first then rerun
	#ifdef CONFIG_HW_SUPPORT_DC2H 
			drvif_DC2H_dispD_CaptureConfig(curCapInfo);
	#else
			
			if((get_i3ddma_idlestatus()==TRUE)	&& (curCapInfo.capSrc != KADP_VT_SCALER_INPUT) && (curCapInfo.capSrc != KADP_VT_SOURCE_OUTPUT))
				drvif_I3DDMA_dispD_VtCaptureConfig(curCapInfo,TRUE);
			else
				drvif_sub_source_VtCaptureConfig(curCapInfo,TRUE);

	#endif
			
			curCapInfo.enable = 1;
			curCapInfo.capSrc = dumpLocation;
			
			VTDumpLocation = dumpLocation;
			
	#ifdef CONFIG_HW_SUPPORT_DC2H 
			
			drvif_DC2H_dispD_CaptureConfig(curCapInfo);

	#else		
			if((get_i3ddma_idlestatus()==TRUE)	&& (curCapInfo.capSrc != KADP_VT_SCALER_INPUT) && (curCapInfo.capSrc != KADP_VT_SOURCE_OUTPUT))
				drvif_I3DDMA_dispD_VtCaptureConfig(curCapInfo,TRUE);
			else
				drvif_sub_source_VtCaptureConfig(curCapInfo,TRUE);
	#endif

	#ifdef CONFIG_HW_SUPPORT_DC2H 
			if(dumpLocation == KADP_VT_SCALER_OUTPUT)
	#else
			if((dumpLocation == KADP_VT_SCALER_OUTPUT) || (dumpLocation == KADP_VT_SOURCE_OUTPUT))
	#endif
				VFODState.bAppliedPQ = FALSE;
			else
				VFODState.bAppliedPQ = TRUE;


			if(VFODState.bFreezed == TRUE) {
				//set_vfod_freezed_for_dc2h(TRUE);//revert freezed status
			
#ifdef CONFIG_HW_SUPPORT_DC2H
				set_vfod_freezed_for_dc2h(TRUE);
#else
				set_vfod_freezed_for_i3ddma_sub_capture(TRUE);
#endif
			
			}
		}
		up(&VT_Semaphore);
		return TRUE;
	} else {
		rtd_pr_vt_notice("[Warning]VT is not Inited when set dump location;%s=%d\n", __FUNCTION__, __LINE__);
		VTDumpLocation = dumpLocation;
		up(&VT_Semaphore);
		return FALSE;
	}
}

unsigned char HAL_VT_GetVideoFrameOutputDeviceOutputInfo(KADP_VT_VIDEO_WINDOW_TYPE_T videoWindowID, KADP_VT_DUMP_LOCATION_TYPE_T dumpLocation, KADP_VT_VIDEO_FRAME_OUTPUT_DEVICE_OUTPUT_INFO_T *pOutputInfo)
{
	
	ppoverlay_main_active_h_start_end_RBUS main_active_h_start_end_reg;
	ppoverlay_main_active_v_start_end_RBUS main_active_v_start_end_reg;
	ppoverlay_main_den_h_start_end_RBUS main_den_h_start_end_Reg;
	ppoverlay_main_den_v_start_end_RBUS main_den_v_start_end_Reg;
	unsigned int x,y,w,h;

	if(videoWindowID != KADP_VT_VIDEO_WINDOW_0) {
		rtd_pr_vt_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	down(&VT_Semaphore);
	//Main Active H pos
	main_active_h_start_end_reg.regValue = rtd_inl(PPOVERLAY_MAIN_Active_H_Start_End_reg);
	//Main Active V pos
	main_active_v_start_end_reg.regValue = rtd_inl(PPOVERLAY_MAIN_Active_V_Start_End_reg);

	main_den_h_start_end_Reg.regValue = rtd_inl(PPOVERLAY_MAIN_DEN_H_Start_End_reg);
	main_den_v_start_end_Reg.regValue = rtd_inl(PPOVERLAY_MAIN_DEN_V_Start_End_reg);
	
	if((get_vt_function() == TRUE)&&(dumpLocation ==VTDumpLocation )) {
		if((KADP_VT_DISPLAY_OUTPUT==dumpLocation)||(KADP_VT_OSDVIDEO_OUTPUT==dumpLocation)) {
			pOutputInfo->maxRegion.x = 0;
			pOutputInfo->maxRegion.y =0;
			pOutputInfo->maxRegion.w = curCapInfo.capWid;
			pOutputInfo->maxRegion.h = curCapInfo.capLen;

			x = main_active_h_start_end_reg.mh_act_sta;
			y =main_active_v_start_end_reg.mv_act_sta;
			w =main_active_h_start_end_reg.mh_act_end - main_active_h_start_end_reg.mh_act_sta;
			h = main_active_v_start_end_reg.mv_act_end - main_active_v_start_end_reg.mv_act_sta;

			//scaledown:
			x = x*curCapInfo.capWid;
			x = x/(main_den_h_start_end_Reg.mh_den_end -main_den_h_start_end_Reg.mh_den_sta);

			y = y*curCapInfo.capLen;
			y = y/(main_den_v_start_end_Reg.mv_den_end - main_den_v_start_end_Reg.mv_den_sta);

			w = w*curCapInfo.capWid;
			w = w/(main_den_h_start_end_Reg.mh_den_end -main_den_h_start_end_Reg.mh_den_sta);

			h = h*curCapInfo.capLen;
			h = h/(main_den_v_start_end_Reg.mv_den_end - main_den_v_start_end_Reg.mv_den_sta);

			pOutputInfo->activeRegion.x = x;
			pOutputInfo->activeRegion.y =y;
			pOutputInfo->activeRegion.w = w;
			pOutputInfo->activeRegion.h = h;
			pOutputInfo->stride = curCapInfo.capWid;
		} else if(KADP_VT_SCALER_OUTPUT==dumpLocation) {
			pOutputInfo->maxRegion.x = 0;
			pOutputInfo->maxRegion.y =0;
			pOutputInfo->maxRegion.w = curCapInfo.capWid;
			pOutputInfo->maxRegion.h = curCapInfo.capLen;

			pOutputInfo->activeRegion.x = 0;
			pOutputInfo->activeRegion.y =0;
			pOutputInfo->activeRegion.w = curCapInfo.capWid;
			pOutputInfo->activeRegion.h = curCapInfo.capLen;

			pOutputInfo->stride = curCapInfo.capWid;
		}	
		up(&VT_Semaphore);
		return TRUE;
	} else {
		pOutputInfo->maxRegion.x = 0;
		pOutputInfo->maxRegion.y =0;
		pOutputInfo->maxRegion.w = 0;
		pOutputInfo->maxRegion.h = 0;

		pOutputInfo->activeRegion.x = 0;
		pOutputInfo->activeRegion.y =0;
		pOutputInfo->activeRegion.w =0;
		pOutputInfo->activeRegion.h = 0;

		pOutputInfo->stride = 0;

		rtd_pr_vt_notice("VFOD is not inited or vfod dump location not matches you want;%s=%d\n", __FUNCTION__, __LINE__);
		up(&VT_Semaphore);
		return FALSE;
	}
}

unsigned char HAL_VT_SetVideoFrameOutputDeviceOutputRegion(KADP_VT_VIDEO_WINDOW_TYPE_T videoWindowID, KADP_VT_DUMP_LOCATION_TYPE_T dumpLocation, KADP_VT_RECT_T *pOutputRegion)
{
	down(&VT_Semaphore);
	if (videoWindowID != KADP_VT_VIDEO_WINDOW_0) {
		rtd_pr_vt_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		up(&VT_Semaphore);
		return FALSE;
	}

	if((pOutputRegion->x!=0)||(pOutputRegion->y!=0)) 
	{
		rtd_pr_vt_notice("[warning]only support output region x y is 0\n");
		up(&VT_Semaphore);
		return FALSE;
	}
	if(dumpLocation >= KADP_VT_MAX_LOCATION)
	{
		rtd_pr_vt_emerg("[error]%s=%d invalid capture location!\n", __FUNCTION__, __LINE__);
		up(&VT_Semaphore);
		return FALSE;
	}
	if((pOutputRegion->w>(Get_DISP_DEN_END_HPOS() - Get_DISP_DEN_STA_HPOS()))||(pOutputRegion->w<60)||(pOutputRegion->h>(Get_DISP_DEN_END_VPOS() - Get_DISP_DEN_STA_VPOS()))||(pOutputRegion->h<34)) {
		rtd_pr_vt_notice("[warning]OutputRegion over limitation!!!!\n");
		up(&VT_Semaphore);
		return FALSE;
	}
	if(get_VT_Pixel_Format() == VT_CAP_RGB888)
	{
		if(dumpLocation == KADP_VT_SCALER_OUTPUT)
		{
			up(&VT_Semaphore);
			return FALSE;
		}
	}
	
	if(get_vt_function() == TRUE) {
		if((dumpLocation==VTDumpLocation)&&(pOutputRegion->w==curCapInfo.capWid)&&(pOutputRegion->h==curCapInfo.capLen)) 
		{
			rtd_pr_vt_notice("all SetVideoFrameOutputDeviceOutputRegion is the same with now\n");
		}
		else 
		{
			curCapInfo.enable = 0; //close vfod first then rerun
	#ifdef CONFIG_HW_SUPPORT_DC2H 		
			drvif_DC2H_dispD_CaptureConfig(curCapInfo);
	#else

		if(get_i3ddma_idlestatus() == TRUE)
		{
			if(drvif_I3DDMA_dispD_VtCaptureConfig(curCapInfo,TRUE) == FALSE)
			{
				up(&VT_Semaphore);
				return FALSE;
			}
		}
		else
		{
			if(get_VT_Pixel_Format() != VT_CAP_NV12)
			{
				rtd_pr_vt_notice("not nv12,sub only support nv12,return false,%s\n", __FUNCTION__);
				up(&VT_Semaphore);
				return FALSE;
			}
			drvif_sub_source_VtCaptureConfig(curCapInfo,TRUE);
		}
	#endif

			
			curCapInfo.enable = 1;

			curCapInfo.capSrc = dumpLocation;
			curCapInfo.capWid = pOutputRegion->w;
			curCapInfo.capLen = pOutputRegion->h;
			
			VTDumpLocation = dumpLocation;
	#ifdef CONFIG_HW_SUPPORT_DC2H		
		
			drvif_DC2H_dispD_CaptureConfig(curCapInfo);

	#else
			
			if(get_i3ddma_idlestatus() == TRUE)
				drvif_I3DDMA_dispD_VtCaptureConfig(curCapInfo,TRUE);
			else
				drvif_sub_source_VtCaptureConfig(curCapInfo,TRUE);

	#endif

	
	#ifdef CONFIG_HW_SUPPORT_DC2H		
			if(dumpLocation==KADP_VT_SCALER_OUTPUT)
	#else
			if((dumpLocation == KADP_VT_SCALER_OUTPUT) || (dumpLocation == KADP_VT_SOURCE_OUTPUT))
	#endif
				VFODState.bAppliedPQ = FALSE;
			else
				VFODState.bAppliedPQ = TRUE;

		}
		up(&VT_Semaphore);
		return TRUE;
	} else {
		rtd_pr_vt_notice("VT is not Inited So return False;%s=%d\n", __FUNCTION__, __LINE__);
		up(&VT_Semaphore);
		return FALSE;
	}
}

unsigned char HAL_VT_WaitVsync(KADP_VT_VIDEO_WINDOW_TYPE_T videoWindowID)
{
	if(videoWindowID != KADP_VT_VIDEO_WINDOW_0) {
		rtd_pr_vt_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	if(get_vt_function() == TRUE) {
		unsigned int sleeptime;
		
		unsigned int vtframerate;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
		struct timespec64 timeout_s;
#else
		struct timespec timeout_s;
#endif
		unsigned int waitvsyncframerate;
		vtframerate = Get_VFOD_FrameRate();/*Get_VFOD_FrameRate not return 0*/


		if(VFODState.framerateDivide !=0)
			waitvsyncframerate = vtframerate/VFODState.framerateDivide;
		else
			waitvsyncframerate = vtframerate;

		sleeptime = 1000/waitvsyncframerate;
	
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
		timeout_s = ns_to_timespec64(sleeptime * 1000 * 1000);
		hrtimer_nanosleep(&timeout_s,  HRTIMER_MODE_REL, CLOCK_MONOTONIC);
#else
		timeout_s = ns_to_timespec(sleeptime * 1000 * 1000);
		hrtimer_nanosleep(&timeout_s, NULL, HRTIMER_MODE_REL, CLOCK_MONOTONIC);
#endif
		return TRUE;
	} else {
		//rtd_pr_vt_notice("VT is not Inited So return False;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}

}

unsigned char HAL_VT_GetInputVideoInfo(KADP_VT_VIDEO_WINDOW_TYPE_T videoWindowID, KADP_VT_INPUT_VIDEO_INFO_T *pInputVideoInfo)
{
	if(videoWindowID != KADP_VT_VIDEO_WINDOW_0)
	{
		rtd_pr_vt_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	down(&VT_Semaphore);
	if(get_vt_function() == TRUE) {
		pInputVideoInfo->region.x = 0;
		pInputVideoInfo->region.y = 0;
		pInputVideoInfo->region.w =Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPH_ACT_WID_PRE);
		pInputVideoInfo->region.h = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE);
		if (Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE) == TRUE)
		{
			pInputVideoInfo->type = KADP_VT_VIDEO_PROGRESSIVE;
			pInputVideoInfo->region.h = pInputVideoInfo->region.h *2;
		} else {
			pInputVideoInfo->type = KADP_VT_VIDEO_PROGRESSIVE;
		}
		pInputVideoInfo->bIs3DVideo = FALSE;
		up(&VT_Semaphore);
		return TRUE;
	} else {
		pInputVideoInfo->region.x = 0;
		pInputVideoInfo->region.y =0;
		pInputVideoInfo->region.w =0;
		pInputVideoInfo->region.h = 0;
		pInputVideoInfo->type = KADP_VT_VIDEO_PROGRESSIVE;
		pInputVideoInfo->bIs3DVideo = FALSE;
		rtd_pr_vt_notice("VT is not Inited So return InputVideoregion 0;%s=%d\n", __FUNCTION__, __LINE__);
		up(&VT_Semaphore);
		return FALSE;
	}
}

unsigned char HAL_VT_GetOutputVideoInfo(KADP_VT_VIDEO_WINDOW_TYPE_T videoWindowID, KADP_VT_OUTPUT_VIDEO_INFO_T *pOutputVideoInfo)
{
	ppoverlay_main_active_h_start_end_RBUS main_active_h_start_end_reg;
	ppoverlay_main_active_v_start_end_RBUS main_active_v_start_end_reg;
	unsigned int x,y,w,h;
	if(videoWindowID != KADP_VT_VIDEO_WINDOW_0) {
		rtd_pr_vt_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	down(&VT_Semaphore);
	main_active_h_start_end_reg.regValue = rtd_inl(PPOVERLAY_MAIN_Active_H_Start_End_reg);
	main_active_v_start_end_reg.regValue = rtd_inl(PPOVERLAY_MAIN_Active_V_Start_End_reg);
	x = main_active_h_start_end_reg.mh_act_sta;
	y =main_active_v_start_end_reg.mv_act_sta;
	w =main_active_h_start_end_reg.mh_act_end - main_active_h_start_end_reg.mh_act_sta;
	h = main_active_v_start_end_reg.mv_act_end - main_active_v_start_end_reg.mv_act_sta;

	if(get_vt_function() == TRUE) {
		if((KADP_VT_DISPLAY_OUTPUT==VTDumpLocation)||(KADP_VT_OSDVIDEO_OUTPUT==VTDumpLocation)) {
			pOutputVideoInfo->maxRegion.x = Get_DISP_ACT_STA_HPOS();
			pOutputVideoInfo->maxRegion.y = Get_DISP_ACT_STA_VPOS();
			pOutputVideoInfo->maxRegion.w = Get_DISP_ACT_END_HPOS() - Get_DISP_ACT_STA_HPOS();
			pOutputVideoInfo->maxRegion.h = Get_DISP_ACT_END_VPOS() - Get_DISP_ACT_STA_VPOS();

			pOutputVideoInfo->activeRegion.x = x;
			pOutputVideoInfo->activeRegion.y = y;
			pOutputVideoInfo->activeRegion.w = w;
			pOutputVideoInfo->activeRegion.h = h;
			pOutputVideoInfo->type = KADP_VT_VIDEO_PROGRESSIVE;
		} else if(KADP_VT_SCALER_OUTPUT==VTDumpLocation) {
			pOutputVideoInfo->maxRegion.x = x;
			pOutputVideoInfo->maxRegion.y = y;
			pOutputVideoInfo->maxRegion.w = w;
			pOutputVideoInfo->maxRegion.h = h;

			pOutputVideoInfo->activeRegion.x = x;
			pOutputVideoInfo->activeRegion.y = y;
			pOutputVideoInfo->activeRegion.w = w;
			pOutputVideoInfo->activeRegion.h = h;

			pOutputVideoInfo->type = KADP_VT_VIDEO_PROGRESSIVE;
		}		
		up(&VT_Semaphore);
		return TRUE;
	} else {
		pOutputVideoInfo->maxRegion.x = 0;
		pOutputVideoInfo->maxRegion.y =0;
		pOutputVideoInfo->maxRegion.w = 0;
		pOutputVideoInfo->maxRegion.h = 0;

		pOutputVideoInfo->activeRegion.x = 0;
		pOutputVideoInfo->activeRegion.y =0;
		pOutputVideoInfo->activeRegion.w = 0;
		pOutputVideoInfo->activeRegion.h = 0;

		pOutputVideoInfo->type = KADP_VT_VIDEO_PROGRESSIVE;
		up(&VT_Semaphore);
		rtd_pr_vt_notice("VT is not Inited So return OutputVideoregion 0;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
}

unsigned char HAL_VT_GetVideoMuteStatus(KADP_VT_VIDEO_WINDOW_TYPE_T videoWindowID, unsigned char *pbOnOff)
{
	if(videoWindowID != KADP_VT_VIDEO_WINDOW_0) {
		rtd_pr_vt_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	down(&VT_Semaphore);
	if(get_vt_function() == TRUE) {
		*pbOnOff = get_vsc_mutestatus();
		up(&VT_Semaphore);
		return TRUE;
	} else {
		*pbOnOff = TRUE;
		rtd_pr_vt_notice("VT is not Inited So return MuteStatus true;%s=%d\n", __FUNCTION__, __LINE__);
		up(&VT_Semaphore);
		return FALSE;
	}
}

void set_vdec_securestatus(unsigned char value)
{
	if(TRUE == value)  // security status
	{
	#ifdef CONFIG_HW_SUPPORT_DC2H
		if(get_dc2h_capture_state() == TRUE)
		{
			reset_dc2h_hw_setting(); /*stop and reset dc2h */
		}
	#endif
	}
	
	VdecSecureStatus = value;
}

unsigned char get_vdec_securestatus(void)
{
	if((VdecSecureStatus == TRUE) || ((DtvSecureStatus == TRUE) && (is_DTV_flag_get() == TRUE)))
		return TRUE;  //security status
	else
		return FALSE;
}

void set_dtv_securestatus(unsigned char status)
{
	if(TRUE == status)  //dtv security status
	{	
#ifdef CONFIG_HW_SUPPORT_DC2H

		if(get_dc2h_capture_state() == TRUE)
		{
			reset_dc2h_hw_setting(); /*stop and reset dc2h */
		}
#endif
	}

	DtvSecureStatus = status;
}

unsigned char get_svp_protect_status(void)
{
	if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC)
	{
		return (get_vdec_securestatus());
	}
	else
		return FALSE;	
}
unsigned char HAL_VT_GetVideoFrameBufferSecureVideoState(KADP_VT_VIDEO_WINDOW_TYPE_T videoWindowID, unsigned char *pIsSecureVideo)
{
	if(videoWindowID != KADP_VT_VIDEO_WINDOW_0) {
		rtd_pr_vt_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}

	if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC) {
		* pIsSecureVideo = get_vdec_securestatus();
	} else {
		* pIsSecureVideo = FALSE;
	}
	return TRUE;

}


static unsigned char VTBlockStatus = FALSE;
unsigned char HAL_VT_GetVideoFrameOutputDeviceBlockState(KADP_VT_VIDEO_WINDOW_TYPE_T videoWindowID, unsigned char *pbBlockState)
{
	if(videoWindowID != KADP_VT_VIDEO_WINDOW_0) {
		rtd_pr_vt_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}

	down(&VTBlock_Semaphore);
	*pbBlockState = VTBlockStatus;
	up(&VTBlock_Semaphore);
	return TRUE;
}

unsigned char HAL_VT_SetVideoFrameOutputDeviceBlockState(KADP_VT_VIDEO_WINDOW_TYPE_T videoWindowID, unsigned char bBlockState)
{
	if(videoWindowID != KADP_VT_VIDEO_WINDOW_0) {
		rtd_pr_vt_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}

	down(&VTBlock_Semaphore);
	VTBlockStatus = bBlockState;
	up(&VTBlock_Semaphore);
	return TRUE;
}

unsigned char HAL_VT_set_Pixel_Format(VT_CAP_FMT value)
{
	down(&VT_Semaphore);
	if((value >= VT_CAP_RGB888)&&(value <= VT_CAP_NV16)) {
		if (value != get_VT_Pixel_Format()) {
			set_VT_Pixel_Format(value);
			/*close vt, and release last pixel format buffer start*/
			curCapInfo.enable = 0; //iMode;
			
#ifdef CONFIG_HW_SUPPORT_DC2H 
			drvif_DC2H_dispD_CaptureConfig(curCapInfo);
else
			
			if((get_i3ddma_idlestatus()==TRUE)	&& (curCapInfo.capSrc != KADP_VT_SCALER_INPUT) && (curCapInfo.capSrc != KADP_VT_SOURCE_OUTPUT))
				drvif_I3DDMA_dispD_VtCaptureConfig(curCapInfo,TRUE);
			else
				drvif_sub_source_VtCaptureConfig(curCapInfo,TRUE);
#endif
			
			Capture_BufferMemDeInit_VT(get_vt_VtBufferNum());
			/*close vt, and release last pixel format buffer end*/

			/*allocate new pixel format buffer start*/
			//curCapInfo.enable = 1; //iMode;		
			if (Capture_BufferMemInit_VT(get_vt_VtBufferNum()) == FALSE) {
				rtd_pr_vt_notice("VT init allocate memory fail when set_VT_Pixel_Format;%s=%d \n", __FUNCTION__, __LINE__);
				up(&VT_Semaphore);
				return FALSE;
			}
			rtd_pr_vt_notice("%s=%d,fmt=%d\n", __FUNCTION__, __LINE__, value);
			
#if 0 //just allocate memory with new format, don't enable capture
#ifdef CONFIG_HW_SUPPORT_DC2H 
			
			drvif_DC2H_dispD_CaptureConfig(curCapInfo);	
			/*open vt, and allocate new pixel format buffer end*/
#else
			
			if((get_i3ddma_idlestatus()==TRUE)	&& (curCapInfo.capSrc != KADP_VT_SCALER_INPUT) && (curCapInfo.capSrc != KADP_VT_SOURCE_OUTPUT))
				drvif_I3DDMA_dispD_VtCaptureConfig(curCapInfo,TRUE);
			else
				drvif_sub_source_VtCaptureConfig(curCapInfo,TRUE);

#endif

#endif
		}
		up(&VT_Semaphore);
		return TRUE;
	} else {
		up(&VT_Semaphore);
		return FALSE;
	}
}


unsigned char HAL_VT_EnableFRCMode(unsigned char bEnableFRC)
{
	rtd_pr_vt_notice("%s=%d bEnableFRC=%d!\n", __FUNCTION__, __LINE__,bEnableFRC);	
	down(&VT_Semaphore);

	if(get_vt_EnableFRCMode()==bEnableFRC){
		rtd_pr_vt_notice("%s=%d bEnableFRC same,no need set again!\n", __FUNCTION__, __LINE__);	
		up(&VT_Semaphore);
		return TRUE;
	}
	
	set_vt_EnableFRCMode(bEnableFRC);
	if(bEnableFRC == FALSE){
		if((get_film_mode_parameter().enable == _DISABLE) /*&& (Get_DISPLAY_REFRESH_RATE() != 120)*/){
			if(1 /*(Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE) > 2560) &&
				(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE) > 1440)*/){
				if(0/*Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 490*/){
					rtd_pr_vt_notice("[VR360] 4k 50/60Hz enter vt_frc_mode, set memc bypass!!\n");
					Scaler_MEMC_outMux(_ENABLE,_DISABLE);	//db on, mux off
					Scaler_MEMC_MEMC_CLK_OnOff(0, 0, 1);	//bypass MEMC with mute on
				}
				else{
					rtd_pr_vt_notice("[VR360] 4k timing enter vt_frc_mode, set mc on!!\n");
					Scaler_MEMC_outMux(_ENABLE,_ENABLE);	//db on, mux off
					Scaler_MEMC_MEMC_CLK_OnOff(0, 1, 1);	//Set MC on with mute on
				}
			}
			else if(Scaler_get_vdec_2k120hz()){
				rtd_pr_vt_notice("[VR360] 2k120Hz enter vt_frc_mode, set memc bypass!!\n");
				Scaler_MEMC_outMux(_ENABLE,_DISABLE);	//db on, mux off
				Scaler_MEMC_MEMC_CLK_OnOff(0, 0, 1);	// bypass MEMC with mute on
			}
			else{
				rtd_pr_vt_notice("[VR360] not 4k timing enter vt_frc_mode, set memc on!!\n");
				Scaler_MEMC_outMux(_ENABLE,_ENABLE);	//db on, mux off
				Scaler_MEMC_MEMC_CLK_OnOff(1, 1, 1);	//Set MEMC on with mute on
			}
		}
		//EnableFRCMode is FALSE,pls close FRC
		//MEMC_Set_malloc_address(bEnableFRC);
	}
	else{
		//EnableFRCMode is TRUE,pls reopen FRC,however,webos never set HAL_VT_EnableFRCMode(TRUE), so need reopen at HAL_VT_Finalize
		//MEMC_Set_malloc_address(bEnableFRC);
	}
#ifdef ENABLE_VR360_DATA_FS_FLOW
	if(bEnableFRC == FALSE){
		INPUT_TIMING_INDEX index = get_current_pattern_index();
		if(index == VO_4K2K_60){
			rtd_pr_vt_notice("[func:%s] enter data fs proc.\n", __FUNCTION__);
			scaler_vsc_vr360_enter_datafs_proc();
		}
	}
#endif
	up(&VT_Semaphore);
	return TRUE;

}

#ifdef CONFIG_HW_SUPPORT_DC2H 

/************************************start of DC2H capture VDC(ATV or CVBS) data function********************************/
void rtk_dc2h_capturevdc_config(unsigned char connect, VIDEO_RECT_T inregion, VIDEO_RECT_T outregion)
{
	if (connect == TRUE) {
		curCapInfo.enable = 1; //iMode;
		curCapInfo.capSrc = 2;  //0:scaler output 1:display output 2:VD output
		curCapInfo.capMode = VT_CAP_MODE_SEQ;
		curCapInfo.capWid = outregion.w; //iWid;
		curCapInfo.capLen = outregion.h;

		DC2H_InputWidthSta = inregion.x;
		DC2H_InputLengthSta = inregion.y;
		DC2H_InputWidth = inregion.w;
		DC2H_InputLength = inregion.h;	

		if (Capture_BufferMemInit_VT(3) == FALSE) {
			rtd_pr_vt_notice("VT init allocate memory fail;%s=%d \n", __FUNCTION__, __LINE__);
			up(&VT_Semaphore);
			return ;
		}
		set_vt_VtBufferNum(3);
		set_vt_VtSwBufferMode(FALSE);
		set_vt_VtCaptureVDC(TRUE);
		//freezebufferaddr = CaptureCtrl_VT.cap_buffer[2].phyaddr;
		drvif_DC2H_dispD_CaptureConfig(curCapInfo);
	} else {
		curCapInfo.enable = 0; //iMode;
		drvif_DC2H_dispD_CaptureConfig(curCapInfo);
		Capture_BufferMemDeInit_VT(get_vt_VtBufferNum());
		set_vt_VtBufferNum(0);
		set_vt_VtSwBufferMode(FALSE);
		set_vt_VtCaptureVDC(FALSE);
	}
}
#endif

/************************************end of DC2H capture VDC(ATV or CVBS) data function********************************/


/************************************start of VT buffer show by sub display function********************************/
#define _BURSTLENGTH2 	(0x78) // 0x78 (120) for sub-channel
#define _FIFOLENGTH2 	(0x80) //sub path FIFO
void memory_set_sub_displaywindow_DispVT(VIDEO_RECT_T s_dispwin)
{
	SCALER_DISP_CHANNEL display = SLR_SUB_DISPLAY;
	unsigned int TotalSize;
	unsigned int Quotient;
	unsigned char Remainder;
	unsigned int fifoLen = _FIFOLENGTH2;
	unsigned int burstLen = _BURSTLENGTH2;
	unsigned int MemShiftAddr = 0;
	unsigned short droppixel = 0;
	unsigned int subctrltemp = 0;
	mdomain_disp_ddr_subprevstart_RBUS mdomain_disp_ddr_subprevstart_Reg;
	mdomain_disp_display_sub_byte_channel_swap_RBUS display_sub_byte_channel_swap_reg;
	
	mdomain_disp_ddr_subprevstart_Reg.regValue = rtd_inl(MDOMAIN_DISP_DDR_SubPreVStart_reg);
	if(s_dispwin.y <= 100) {
		mdomain_disp_ddr_subprevstart_Reg.sub_pre_rd_v_start = 7;
	} else {
		mdomain_disp_ddr_subprevstart_Reg.sub_pre_rd_v_start = (s_dispwin.y-4);
	}
	rtd_outl(MDOMAIN_DISP_DDR_SubPreVStart_reg, mdomain_disp_ddr_subprevstart_Reg.regValue);


	MemGetBlockShiftAddr(display, Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_HSTA),Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_VSTA), &MemShiftAddr, &droppixel);
	TotalSize = memory_get_disp_line_size1(display, Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_WID) + droppixel);	// calculate the memory size of capture
	IoReg_Mask32(MDOMAIN_DISP_DDR_SubPixNumWTL_reg, 0xffc00000, (( ((Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_WID) +droppixel) & 0x3FFF) << 8) | ((fifoLen - (burstLen>>1))>>1)));


	memory_division(TotalSize, burstLen, &Quotient, &Remainder);	// Do a division

	// remainder is not allowed to be zero
	if (Remainder == 0) {
		Remainder = burstLen;
		Quotient -= 1;
	}


	if((dvrif_memory_compression_get_enable(SLR_SUB_DISPLAY) == TRUE) && (dvrif_memory_get_compression_mode(SLR_SUB_DISPLAY) == COMPRESSION_FRAME_MODE))
	{
		IoReg_Mask32(MDOMAIN_DISP_DDR_SubRdNumLenRem_reg, 0x00000000, (0xffff << 16) | (burstLen << 8) | burstLen);
	}
	else
	{
		IoReg_Mask32(MDOMAIN_DISP_DDR_SubRdNumLenRem_reg, 0x00000000, (Quotient << 16) | (burstLen << 8) | Remainder);
	}

	TotalSize = memory_get_capture_size(display, MEMCAPTYPE_LINE);
	/*TotalSize = Quotient * burstLen + Remainder; 	// count one line, 64bits unit
	TotalSize = drvif_memory_get_data_align(TotalSize, 4);	// times of 4*/

	rtd_outl(MDOMAIN_DISP_DDR_SubLineStep_reg,  (TotalSize + TotalSize%2) << 3);//bit 3 need to be 0. rbus rule

	TotalSize = SHL(TotalSize, 3); // unit conversion from 64bits to 8bits

	IoReg_Mask32(MDOMAIN_DISP_DDR_SubLineNum_reg, 0xfffff000, (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_LEN) & 0x0FFF));

	rtd_outl(MDOMAIN_DISP_DDR_SubAddr_reg, (CaptureCtrl_VT.cap_buffer[0].phyaddr + MemShiftAddr) & 0x7ffffff0);
	rtd_outl(MDOMAIN_DISP_DDR_Sub2ndAddr_reg, (CaptureCtrl_VT.cap_buffer[1].phyaddr + MemShiftAddr) & 0x7ffffff0);
	rtd_outl(MDOMAIN_DISP_DDR_Sub3rdAddr_reg, (CaptureCtrl_VT.cap_buffer[2].phyaddr + MemShiftAddr) & 0x7ffffff0);

	rtd_outl(MDOMAIN_DISP_DDR_SubAddrDropBits_reg,droppixel);

	subctrltemp = _BIT6;
	subctrltemp |=_BIT7;//set Mdomain display triple buffer
	//subctrltemp |= (_BIT1);
	//disp ctrl default setting
	subctrltemp |= (_BIT25 | _BIT28);
	subctrltemp |= Scaler_DispGetStatus(SLR_SUB_DISPLAY, SLR_DISP_422CAP) ? 0 : _BIT17;
	//subctrltemp |= (_BIT19);/*for VT data is RGB*/
	rtd_outl(MDOMAIN_DISP_DDR_SubCtrl_reg, subctrltemp);

	display_sub_byte_channel_swap_reg.regValue = rtd_inl(MDOMAIN_DISP_Display_sub_byte_channel_swap_reg);
	display_sub_byte_channel_swap_reg.sub_1byte_swap = 1;
	display_sub_byte_channel_swap_reg.sub_2byte_swap = 1;
	display_sub_byte_channel_swap_reg.sub_4byte_swap = 1;
	display_sub_byte_channel_swap_reg.sub_8byte_swap = 0;
	display_sub_byte_channel_swap_reg.sub_channel_swap = 4;
	rtd_outl(MDOMAIN_DISP_Display_sub_byte_channel_swap_reg, display_sub_byte_channel_swap_reg.regValue);
	rtd_outl(MDOMAIN_DISP_Disp_sub_enable_reg, _BIT0);
}

extern void magnifier_color_ultrazoom_config_scaling_up(unsigned char display);
extern void Scaler_disp_setting(unsigned char display);
/*extern void PipmpSetSubDisplayWindow(unsigned short DispHSta, unsigned short DispHEnd, unsigned short DispVSta,
		unsigned short DispVEnd, unsigned char Border);
extern void PipmpSetSubActiveWindow(unsigned short DispHSta, unsigned short DispHEnd, unsigned short DispVSta, unsigned short DispVEnd);*/

unsigned char Scaler_SubMDispWindow_VFB(bool enable)
{
	rtd_pr_vt_debug("Scaler_SubMDispWindow_VFB curCapInfo.enable==%d\n",curCapInfo.enable);
	if (curCapInfo.enable){
		SCALER_DISP_CHANNEL display = SLR_SUB_DISPLAY;
		mdomain_disp_ddr_mainsubctrl_RBUS mdomain_disp_ddr_mainsubctrl_Reg;
		ppoverlay_double_buffer_ctrl_RBUS ppoverlay_double_buffer_ctrl_Reg;
		scaleup_ds_uzu_db_ctrl_RBUS scaleup_ds_uzu_db_ctrl_Reg;
		//IoReg_SetBits(MDOMAIN_DISP_DDR_MainSubCtrl_reg, _BIT17);
		mdomain_disp_ddr_mainsubctrl_Reg.regValue = rtd_inl(MDOMAIN_DISP_DDR_MainSubCtrl_reg);
		mdomain_disp_ddr_mainsubctrl_Reg.disp2_double_enable = 0;
		rtd_outl(MDOMAIN_DISP_DDR_MainSubCtrl_reg, mdomain_disp_ddr_mainsubctrl_Reg.regValue);

		//IoReg_SetBits(PPOVERLAY_Double_Buffer_CTRL_reg, _BIT6);//Enable dtg double buffer register
		ppoverlay_double_buffer_ctrl_Reg.regValue = rtd_inl(PPOVERLAY_Double_Buffer_CTRL_reg);
		ppoverlay_double_buffer_ctrl_Reg.dsubreg_dbuf_en = 0;
		rtd_outl(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_Reg.regValue);

		//IoReg_SetBits(SCALEUP_DS_UZU_DB_CTRL_reg, _BIT2);//Enable uzu double buffer register
		scaleup_ds_uzu_db_ctrl_Reg.regValue = rtd_inl(SCALEUP_DS_UZU_DB_CTRL_reg);
		scaleup_ds_uzu_db_ctrl_Reg.db_en = 0;
		rtd_outl(SCALEUP_DS_UZU_DB_CTRL_reg, scaleup_ds_uzu_db_ctrl_Reg.regValue);	
		if(enable == TRUE) {
			unsigned short VTCapLen = curCapInfo.capLen;
			unsigned short VTCapWid = curCapInfo.capWid;
			VIDEO_RECT_T outputwin;
			yuv2rgb_d_yuv2rgb_control_RBUS yuv2rgb_d_yuv2rgb_control_Reg;
			
			outputwin.x = 0;
			outputwin.y = 0;
			outputwin.w = VTCapWid;
			outputwin.h = VTCapLen;
			

			
			Scaler_DispSetStatus(display, SLR_DISP_422CAP, FALSE);/*VT buffer is RGB888,so go 444*/
			Scaler_DispSetStatus(display, SLR_DISP_10BIT, FALSE);/*VT buffer is RGB888,so go 8bit*/
			Scaler_DispSetStatus(display, SLR_DISP_INTERLACE, FALSE);
			
			Scaler_DispSetInputInfoByDisp(display, SLR_INPUT_FRAMESYNC, FALSE);
			
			Scaler_DispSetInputInfoByDisp(display, SLR_INPUT_CAP_LEN, VTCapLen);
			Scaler_DispSetInputInfoByDisp(display, SLR_INPUT_CAP_WID, VTCapWid);

			Scaler_DispSetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_LEN, VTCapLen);
			Scaler_DispSetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_VSTA, 0);
			
			Scaler_DispSetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_WID, VTCapWid);
			Scaler_DispSetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_HSTA, 0);

			Scaler_DispSetInputInfoByDisp(display, SLR_INPUT_DISP_WID, VTCapWid);
			Scaler_DispSetInputInfoByDisp(display, SLR_INPUT_DISP_LEN, VTCapLen);

			if(Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_WID) <= Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_DISP_WID)){
				Scaler_DispSetScaleStatus(display, SLR_SCALE_H_UP, TRUE);
			}else{
				Scaler_DispSetScaleStatus(display, SLR_SCALE_H_UP, FALSE);
			}

			if(Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_LEN) <= Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_DISP_LEN))
				Scaler_DispSetScaleStatus(display, SLR_SCALE_V_UP, TRUE);
			else
				Scaler_DispSetScaleStatus(display, SLR_SCALE_V_UP, FALSE);

			memory_set_sub_displaywindow_DispVT(outputwin);
			
			/*sub uzu*/
			magnifier_color_ultrazoom_config_scaling_up(display);

			/*sub yuv2rgb*/
			yuv2rgb_d_yuv2rgb_control_Reg.regValue = rtd_inl(YUV2RGB_D_YUV2RGB_Control_reg);
			if(curCapInfo.capSrc == 1) {//memc
				yuv2rgb_d_yuv2rgb_control_Reg.yuv2rgb_overlay = 1;
				yuv2rgb_d_yuv2rgb_control_Reg.yuv2rgb_sub_en = 0;
			} else{//uzu
				yuv2rgb_d_yuv2rgb_control_Reg.yuv2rgb_overlay = 0;
				yuv2rgb_d_yuv2rgb_control_Reg.yuv2rgb_sub_en = 1;
			}
			rtd_outl(YUV2RGB_D_YUV2RGB_Control_reg, yuv2rgb_d_yuv2rgb_control_Reg.regValue);
			
			/*PipmpSetSubDisplayWindow(
				outputwin.x, outputwin.x + Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_DISP_WID) - 1,
				outputwin.y, outputwin.y + Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_DISP_LEN) - 1,
				0);*/

			/*PipmpSetSubActiveWindow(
				0, Scaler_DispGetInputInfoByDisp(SLR_SUB_DISPLAY,SLR_INPUT_DISP_WID)  - 1,
				0, Scaler_DispGetInputInfoByDisp(SLR_SUB_DISPLAY,SLR_INPUT_DISP_LEN) - 1);*/

			Scaler_disp_setting(display);

			set_display_forcebg_mask(SLR_SUB_DISPLAY, SLR_FORCE_BG_TYPE_VSC, FALSE);
			mute_control(SLR_SUB_DISPLAY, FALSE);		
		} else {
			rtd_outl(MDOMAIN_DISP_Disp_sub_enable_reg, 0x00000000);

			down(get_forcebg_semaphore());
			drvif_scalerdisplay_enable_display(SLR_SUB_DISPLAY, _DISABLE);
			up(get_forcebg_semaphore());
		}
	} else {
		return FALSE;
	}
	return TRUE;
}

/************************************end of VT buffer show by sub display function********************************/



unsigned char ColorBlock_Compare(unsigned long bufferaddr, unsigned int R, unsigned int G, unsigned int B, VIDEO_RECT_T block)
{
	
	unsigned int sumR, sumG, sumB;
	unsigned int i,j;
	unsigned char* bufferpoint;
	
	if((block.w==0)&&(block.h==0))
		return TRUE;
	
	sumR = 0;
	sumG = 0;
	sumB = 0;
	bufferpoint = (unsigned char*)bufferaddr;
	
	for(i=0;i<block.h;i++)
	{
		for(j=0;j<block.w;j++)
		{
			sumR += bufferpoint[1920*3*(i+block.y)+(block.x+j)*3];
			sumG += bufferpoint[1920*3*(i+block.y)+(block.x+j)*3+1];
			sumB += bufferpoint[1920*3*(i+block.y)+(block.x+j)*3+2];
		}
	}
	sumR = sumR/(block.w*block.h);
	sumG = sumG/(block.w*block.h);
	sumB = sumB/(block.w*block.h);
	rtd_pr_vt_emerg("@@@@@@@");
	rtd_pr_vt_emerg("R=%d;sumR=%d\n",R,sumR);
	rtd_pr_vt_emerg("G=%d;sumG=%d\n",G,sumG);
	rtd_pr_vt_emerg("B=%d;sumB=%d\n",B,sumB);
	
	if((abs(R-sumR)<80)&&(abs(G-sumG)<80)&&(abs(B-sumB)<80))
		return TRUE;
	else{
		if(abs(R-sumR)>=80)
			rtd_pr_vt_emerg("!!!!!!@@@R_Diff_value =%d\n",(int)abs(R-sumR));
		if(abs(G-sumR)>=80)
			rtd_pr_vt_emerg("!!!!!!@@@G_Diff_value =%d\n",(int)abs(G-sumR));
		if(abs(B-sumR)>=80)
			rtd_pr_vt_emerg("!!!!!!@@@B_Diff_value =%d\n",(int)abs(B-sumR));
		return FALSE;
	}
}

extern unsigned char rtk_hal_vsc_GetInputRegion(KADP_VIDEO_WID_T wid, KADP_VIDEO_RECT_T * pinregion);
unsigned char Check_colorbar_window_cal(VIDEO_RECT_T *GrayBlock, VIDEO_RECT_T *YellowBlock, VIDEO_RECT_T *LittleBlueBlock, VIDEO_RECT_T *GreenBlock, VIDEO_RECT_T *PinkBlock, VIDEO_RECT_T *RedBlock, VIDEO_RECT_T *BlueBlock, VIDEO_RECT_T *BlackBlock)
{
	unsigned int SourceWidth;
	unsigned int OverscanWidth;
	KADP_VIDEO_RECT_T inregion;
	unsigned int input_w;
	unsigned int nooverscancolorbarWidth;
	unsigned int overscancolorbarWidth;
	unsigned int colorbarnum;
	unsigned int sidecolorwidth;
	VSC_INPUT_TYPE_T srctype;
	srctype = Get_DisplayMode_Src(SLR_MAIN_DISPLAY);

	SourceWidth = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPH_ACT_WID_PRE);
	rtk_hal_vsc_GetInputRegion(KADP_VIDEO_WID_0, &inregion);
	input_w = Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IHWID);
	if(input_w > 3840){
		inregion.x = 3840*inregion.x/input_w;
		inregion.w = 3840*inregion.w/input_w;
	}

	if (srctype == VSC_INPUTSRC_AVD)
	{
		OverscanWidth = Scaler_CalAVD27MWidth(SLR_MAIN_DISPLAY,inregion.w);
	} else {
		OverscanWidth = inregion.w;
	}

	nooverscancolorbarWidth = SourceWidth/8;
	overscancolorbarWidth = 1920*nooverscancolorbarWidth/OverscanWidth;
	sidecolorwidth = (960%overscancolorbarWidth)?(960%overscancolorbarWidth):overscancolorbarWidth;
	colorbarnum =( 8 - 2*((SourceWidth - OverscanWidth) / (nooverscancolorbarWidth*2)));
	
	rtd_pr_vt_emerg("SourceWidth=%d;OverscanWidth=%d\n",SourceWidth,OverscanWidth);
	rtd_pr_vt_emerg("overscancolorbarWidth=%d;sidecolorwidth=%d\n",overscancolorbarWidth,sidecolorwidth);
	rtd_pr_vt_emerg("colorbarnum=%d\n",colorbarnum);

	if(8 == colorbarnum ){
		/*line one*/
		GreenBlock[0].x = 960 - overscancolorbarWidth/2 -5;
		GreenBlock[0].y = 175;
		GreenBlock[0].w = 10;
		GreenBlock[0].h = 10;

		PinkBlock[0].x = 960 + overscancolorbarWidth/2 -5;
		PinkBlock[0].y = 175;
		PinkBlock[0].w = 10;
		PinkBlock[0].h = 10;		

		LittleBlueBlock[0].x = 960 - 3*overscancolorbarWidth/2 -5;
		LittleBlueBlock[0].y = 175;
		LittleBlueBlock[0].w = 10;
		LittleBlueBlock[0].h = 10;

		RedBlock[0].x = 960 + 3*overscancolorbarWidth/2 -5;
		RedBlock[0].y = 175;
		RedBlock[0].w = 10;
		RedBlock[0].h = 10;

		YellowBlock[0].x = 960 - 5*overscancolorbarWidth/2 -5;
		YellowBlock[0].y = 175;
		YellowBlock[0].w = 10;
		YellowBlock[0].h = 10;

		BlueBlock[0].x = 960 + 5*overscancolorbarWidth/2 -5;
		BlueBlock[0].y = 175;
		BlueBlock[0].w = 10;
		BlueBlock[0].h = 10;

		if(sidecolorwidth >= 30){
			GrayBlock[0].x = 960 - 3*overscancolorbarWidth - sidecolorwidth/2 -5;
			GrayBlock[0].y = 175;
			GrayBlock[0].w = 10;
			GrayBlock[0].h = 10;
			
			BlackBlock[0].x = 960 + 3*overscancolorbarWidth + sidecolorwidth/2 -5;
			BlackBlock[0].y = 175;
			BlackBlock[0].w = 10;
			BlackBlock[0].h = 10;
		} else {
			GrayBlock[0].x = 0;
			GrayBlock[0].y = 0;
			GrayBlock[0].w = 0;
			GrayBlock[0].h = 0;
			
			BlackBlock[0].x = 0;
			BlackBlock[0].y = 0;
			BlackBlock[0].w = 0;
			BlackBlock[0].h = 0;
		}

		/*line two*/
		GreenBlock[1].x = 960 - overscancolorbarWidth/2 -5;
		GreenBlock[1].y = 535;
		GreenBlock[1].w = 10;
		GreenBlock[1].h = 10;

		PinkBlock[1].x = 960 + overscancolorbarWidth/2 -5;
		PinkBlock[1].y = 535;
		PinkBlock[1].w = 10;
		PinkBlock[1].h = 10;		

		LittleBlueBlock[1].x = 960 - 3*overscancolorbarWidth/2 -5;
		LittleBlueBlock[1].y = 535;
		LittleBlueBlock[1].w = 10;
		LittleBlueBlock[1].h = 10;

		RedBlock[1].x = 960 + 3*overscancolorbarWidth/2 -5;
		RedBlock[1].y = 535;
		RedBlock[1].w = 10;
		RedBlock[1].h = 10;

		YellowBlock[1].x = 960 - 5*overscancolorbarWidth/2 -5;
		YellowBlock[1].y = 535;
		YellowBlock[1].w = 10;
		YellowBlock[1].h = 10;

		BlueBlock[1].x = 960 + 5*overscancolorbarWidth/2 -5;
		BlueBlock[1].y = 535;
		BlueBlock[1].w = 10;
		BlueBlock[1].h = 10;

		if(sidecolorwidth >= 30){
			GrayBlock[1].x = 960 - 3*overscancolorbarWidth - sidecolorwidth/2 -5;
			GrayBlock[1].y = 535;
			GrayBlock[1].w = 10;
			GrayBlock[1].h = 10;
			
			BlackBlock[1].x = 960 + 3*overscancolorbarWidth + sidecolorwidth/2 -5;
			BlackBlock[1].y = 535;
			BlackBlock[1].w = 10;
			BlackBlock[1].h = 10;
		} else {
			GrayBlock[1].x = 0;
			GrayBlock[1].y = 0;
			GrayBlock[1].w = 0;
			GrayBlock[1].h = 0;
			
			BlackBlock[1].x = 0;
			BlackBlock[1].y = 0;
			BlackBlock[1].w = 0;
			BlackBlock[1].h = 0;
		}

		/*line three*/
		GreenBlock[2].x = 960 - overscancolorbarWidth/2 -5;
		GreenBlock[2].y = 895;
		GreenBlock[2].w = 10;
		GreenBlock[2].h = 10;

		PinkBlock[2].x = 960 + overscancolorbarWidth/2 -5;
		PinkBlock[2].y = 895;
		PinkBlock[2].w = 10;
		PinkBlock[2].h = 10;		

		LittleBlueBlock[2].x = 960 - 3*overscancolorbarWidth/2 -5;
		LittleBlueBlock[2].y = 895;
		LittleBlueBlock[2].w = 10;
		LittleBlueBlock[2].h = 10;

		RedBlock[2].x = 960 + 3*overscancolorbarWidth/2 -5;
		RedBlock[2].y = 895;
		RedBlock[2].w = 10;
		RedBlock[2].h = 10;

		YellowBlock[2].x = 960 - 5*overscancolorbarWidth/2 -5;
		YellowBlock[2].y = 895;
		YellowBlock[2].w = 10;
		YellowBlock[2].h = 10;

		BlueBlock[2].x = 960 + 5*overscancolorbarWidth/2 -5;
		BlueBlock[2].y = 895;
		BlueBlock[2].w = 10;
		BlueBlock[2].h = 10;

		if(sidecolorwidth >= 30){
			GrayBlock[2].x = 960 - 3*overscancolorbarWidth - sidecolorwidth/2 -5;
			GrayBlock[2].y = 895;
			GrayBlock[2].w = 10;
			GrayBlock[2].h = 10;
			
			BlackBlock[2].x = 960 + 3*overscancolorbarWidth + sidecolorwidth/2 -5;
			BlackBlock[2].y = 895;
			BlackBlock[2].w = 10;
			BlackBlock[2].h = 10;
		} else {
			GrayBlock[2].x = 0;
			GrayBlock[2].y = 0;
			GrayBlock[2].w = 0;
			GrayBlock[2].h = 0;
			
			BlackBlock[2].x = 0;
			BlackBlock[2].y = 0;
			BlackBlock[2].w = 0;
			BlackBlock[2].h = 0;
		}	
	} else if (6 == colorbarnum ){
			/*line one*/
			GreenBlock[0].x = 960 - overscancolorbarWidth/2 -5;
			GreenBlock[0].y = 175;
			GreenBlock[0].w = 10;
			GreenBlock[0].h = 10;
			
			PinkBlock[0].x = 960 + overscancolorbarWidth/2 -5;
			PinkBlock[0].y = 175;
			PinkBlock[0].w = 10;
			PinkBlock[0].h = 10;		
			
			LittleBlueBlock[0].x = 960 - 3*overscancolorbarWidth/2 -5;
			LittleBlueBlock[0].y = 175;
			LittleBlueBlock[0].w = 10;
			LittleBlueBlock[0].h = 10;
			
			RedBlock[0].x = 960 + 3*overscancolorbarWidth/2 -5;
			RedBlock[0].y = 175;
			RedBlock[0].w = 10;
			RedBlock[0].h = 10;
			
			if(sidecolorwidth >= 30){
				YellowBlock[0].x = 960 - 2*overscancolorbarWidth-sidecolorwidth/2 -5;
				YellowBlock[0].y = 175;
				YellowBlock[0].w = 10;
				YellowBlock[0].h = 10;
				
				BlueBlock[0].x = 960 + 2*overscancolorbarWidth+sidecolorwidth/2 -5;
				BlueBlock[0].y = 175;
				BlueBlock[0].w = 10;
				BlueBlock[0].h = 10;

			} else {
				YellowBlock[0].x = 0;
				YellowBlock[0].y = 0;
				YellowBlock[0].w = 0;
				YellowBlock[0].h = 0;
				
				BlueBlock[0].x = 0;
				BlueBlock[0].y = 0;
				BlueBlock[0].w = 0;
				BlueBlock[0].h = 0;

			}

			GrayBlock[0].x = 0;
			GrayBlock[0].y = 0;
			GrayBlock[0].w = 0;
			GrayBlock[0].h = 0;
			
			BlackBlock[0].x = 0;
			BlackBlock[0].y = 0;
			BlackBlock[0].w = 0;
			BlackBlock[0].h = 0;			
			/*line two*/
			GreenBlock[1].x = 960 - overscancolorbarWidth/2 -5;
			GreenBlock[1].y = 535;
			GreenBlock[1].w = 10;
			GreenBlock[1].h = 10;
			
			PinkBlock[1].x = 960 + overscancolorbarWidth/2 -5;
			PinkBlock[1].y = 535;
			PinkBlock[1].w = 10;
			PinkBlock[1].h = 10;		
			
			LittleBlueBlock[1].x = 960 - 3*overscancolorbarWidth/2 -5;
			LittleBlueBlock[1].y = 535;
			LittleBlueBlock[1].w = 10;
			LittleBlueBlock[1].h = 10;
			
			RedBlock[1].x = 960 + 3*overscancolorbarWidth/2 -5;
			RedBlock[1].y = 535;
			RedBlock[1].w = 10;
			RedBlock[1].h = 10;
					
			if(sidecolorwidth >= 30){
				YellowBlock[1].x = 960 - 2*overscancolorbarWidth-sidecolorwidth/2 -5;
				YellowBlock[1].y = 535;
				YellowBlock[1].w = 10;
				YellowBlock[1].h = 10;
				
				BlueBlock[1].x = 960 + 2*overscancolorbarWidth+sidecolorwidth/2 -5;
				BlueBlock[1].y = 535;
				BlueBlock[1].w = 10;
				BlueBlock[1].h = 10;
			} else {
				YellowBlock[1].x = 0;
				YellowBlock[1].y = 0;
				YellowBlock[1].w = 0;
				YellowBlock[1].h = 0;
				
				BlueBlock[1].x = 0;
				BlueBlock[1].y = 0;
				BlueBlock[1].w = 0;
				BlueBlock[1].h = 0;
			}
			GrayBlock[1].x = 0;
			GrayBlock[1].y = 0;
			GrayBlock[1].w = 0;
			GrayBlock[1].h = 0;
			
			BlackBlock[1].x = 0;
			BlackBlock[1].y = 0;
			BlackBlock[1].w = 0;
			BlackBlock[1].h = 0;
			
			/*line three*/
			GreenBlock[2].x = 960 - overscancolorbarWidth/2 -5;
			GreenBlock[2].y = 895;
			GreenBlock[2].w = 10;
			GreenBlock[2].h = 10;
			
			PinkBlock[2].x = 960 + overscancolorbarWidth/2 -5;
			PinkBlock[2].y = 895;
			PinkBlock[2].w = 10;
			PinkBlock[2].h = 10;		
			
			LittleBlueBlock[2].x = 960 - 3*overscancolorbarWidth/2 -5;
			LittleBlueBlock[2].y = 895;
			LittleBlueBlock[2].w = 10;
			LittleBlueBlock[2].h = 10;
			
			RedBlock[2].x = 960 + 3*overscancolorbarWidth/2 -5;
			RedBlock[2].y = 895;
			RedBlock[2].w = 10;
			RedBlock[2].h = 10;
			
			if(sidecolorwidth >= 30){
				YellowBlock[2].x = 960 - 2*overscancolorbarWidth-sidecolorwidth/2 -5;
				YellowBlock[2].y = 895;
				YellowBlock[2].w = 10;
				YellowBlock[2].h = 10;
				
				BlueBlock[2].x = 960 + 2*overscancolorbarWidth+sidecolorwidth/2 -5;
				BlueBlock[2].y = 895;
				BlueBlock[2].w = 10;
				BlueBlock[2].h = 10;

			} else {
				YellowBlock[2].x = 0;
				YellowBlock[2].y = 0;
				YellowBlock[2].w = 0;
				YellowBlock[2].h = 0;
				
				BlueBlock[2].x = 0;
				BlueBlock[2].y = 0;
				BlueBlock[2].w = 0;
				BlueBlock[2].h = 0;
			}
			GrayBlock[2].x = 0;
			GrayBlock[2].y = 0;
			GrayBlock[2].w = 0;
			GrayBlock[2].h = 0;
			
			BlackBlock[2].x = 0;
			BlackBlock[2].y = 0;
			BlackBlock[2].w = 0;
			BlackBlock[2].h = 0;
	} else if (4 == colorbarnum){
		/*line one*/
		GreenBlock[0].x = 960 - overscancolorbarWidth/2 -5;
		GreenBlock[0].y = 175;
		GreenBlock[0].w = 10;
		GreenBlock[0].h = 10;
		
		PinkBlock[0].x = 960 + overscancolorbarWidth/2 -5;
		PinkBlock[0].y = 175;
		PinkBlock[0].w = 10;
		PinkBlock[0].h = 10;		
		
		if(sidecolorwidth >= 30){
			LittleBlueBlock[0].x = 960 - overscancolorbarWidth-sidecolorwidth/2 -5;
			LittleBlueBlock[0].y = 175;
			LittleBlueBlock[0].w = 10;
			LittleBlueBlock[0].h = 10;
			
			RedBlock[0].x = 960 + overscancolorbarWidth+sidecolorwidth/2 -5;
			RedBlock[0].y = 175;
			RedBlock[0].w = 10;
			RedBlock[0].h = 10;
		} else {
			LittleBlueBlock[0].x = 0;
			LittleBlueBlock[0].y = 0;
			LittleBlueBlock[0].w = 0;
			LittleBlueBlock[0].h = 0;
			
			RedBlock[0].x = 0;
			RedBlock[0].y = 0;
			RedBlock[0].w = 0;
			RedBlock[0].h = 0;
		}
		YellowBlock[0].x = 0;
		YellowBlock[0].y = 0;
		YellowBlock[0].w = 0;
		YellowBlock[0].h = 0;
		
		BlueBlock[0].x = 0;
		BlueBlock[0].y = 0;
		BlueBlock[0].w = 0;
		BlueBlock[0].h = 0;	
		
		GrayBlock[0].x = 0;
		GrayBlock[0].y = 0;
		GrayBlock[0].w = 0;
		GrayBlock[0].h = 0;
		
		BlackBlock[0].x = 0;
		BlackBlock[0].y = 0;
		BlackBlock[0].w = 0;
		BlackBlock[0].h = 0; 		

		/*line two*/
		GreenBlock[1].x = 960 - overscancolorbarWidth/2 -5;
		GreenBlock[1].y = 535;
		GreenBlock[1].w = 10;
		GreenBlock[1].h = 10;
		
		PinkBlock[1].x = 960 + overscancolorbarWidth/2 -5;
		PinkBlock[1].y = 535;
		PinkBlock[1].w = 10;
		PinkBlock[1].h = 10;		
		
		if(sidecolorwidth >= 30){
			LittleBlueBlock[1].x = 960 - overscancolorbarWidth-sidecolorwidth/2 -5;
			LittleBlueBlock[1].y = 535;
			LittleBlueBlock[1].w = 10;
			LittleBlueBlock[1].h = 10;
			
			RedBlock[1].x = 960 + overscancolorbarWidth+sidecolorwidth/2 -5;
			RedBlock[1].y = 535;
			RedBlock[1].w = 10;
			RedBlock[1].h = 10;
		} else {
			LittleBlueBlock[1].x = 0;
			LittleBlueBlock[1].y = 0;
			LittleBlueBlock[1].w = 0;
			LittleBlueBlock[1].h = 0;
			
			RedBlock[1].x = 0;
			RedBlock[1].y = 0;
			RedBlock[1].w = 0;
			RedBlock[1].h = 0;
		}
		YellowBlock[1].x = 0;
		YellowBlock[1].y = 0;
		YellowBlock[1].w = 0;
		YellowBlock[1].h = 0;
		
		BlueBlock[1].x = 0;
		BlueBlock[1].y = 0;
		BlueBlock[1].w = 0;
		BlueBlock[1].h = 0;	
		
		GrayBlock[1].x = 0;
		GrayBlock[1].y = 0;
		GrayBlock[1].w = 0;
		GrayBlock[1].h = 0;
		
		BlackBlock[1].x = 0;
		BlackBlock[1].y = 0;
		BlackBlock[1].w = 0;
		BlackBlock[1].h = 0; 		

		/*line three*/
		GreenBlock[2].x = 960 - overscancolorbarWidth/2 -5;
		GreenBlock[2].y = 895;
		GreenBlock[2].w = 10;
		GreenBlock[2].h = 10;
		
		PinkBlock[2].x = 960 + overscancolorbarWidth/2 -5;
		PinkBlock[2].y = 895;
		PinkBlock[2].w = 10;
		PinkBlock[2].h = 10;		
		
		if(sidecolorwidth >= 30){
			LittleBlueBlock[2].x = 960 - overscancolorbarWidth-sidecolorwidth/2 -5;
			LittleBlueBlock[2].y = 895;
			LittleBlueBlock[2].w = 10;
			LittleBlueBlock[2].h = 10;
			
			RedBlock[2].x = 960 + overscancolorbarWidth+sidecolorwidth/2 -5;
			RedBlock[2].y = 895;
			RedBlock[2].w = 10;
			RedBlock[2].h = 10;
		} else {
			LittleBlueBlock[2].x = 0;
			LittleBlueBlock[2].y = 0;
			LittleBlueBlock[2].w = 0;
			LittleBlueBlock[2].h = 0;
			
			RedBlock[2].x = 0;
			RedBlock[2].y = 0;
			RedBlock[2].w = 0;
			RedBlock[2].h = 0;
		}
		YellowBlock[2].x = 0;
		YellowBlock[2].y = 0;
		YellowBlock[2].w = 0;
		YellowBlock[2].h = 0;
		
		BlueBlock[2].x = 0;
		BlueBlock[2].y = 0;
		BlueBlock[2].w = 0;
		BlueBlock[2].h = 0;	
		
		GrayBlock[2].x = 0;
		GrayBlock[2].y = 0;
		GrayBlock[2].w = 0;
		GrayBlock[2].h = 0;
		
		BlackBlock[2].x = 0;
		BlackBlock[2].y = 0;
		BlackBlock[2].w = 0;
		BlackBlock[2].h = 0; 		

	}else if ((2 == colorbarnum) || (0 == colorbarnum)) {
		/*line one*/
		GreenBlock[0].x = 475;
		GreenBlock[0].y = 175;
		GreenBlock[0].w = 10;
		GreenBlock[0].h = 10;
		
		PinkBlock[0].x = 475;
		PinkBlock[0].y = 175;
		PinkBlock[0].w = 10;
		PinkBlock[0].h = 10;		
		
		LittleBlueBlock[0].x = 0;
		LittleBlueBlock[0].y = 0;
		LittleBlueBlock[0].w = 0;
		LittleBlueBlock[0].h = 0;
		
		RedBlock[0].x = 0;
		RedBlock[0].y = 0;
		RedBlock[0].w = 0;
		RedBlock[0].h = 0;

		YellowBlock[0].x = 0;
		YellowBlock[0].y = 0;
		YellowBlock[0].w = 0;
		YellowBlock[0].h = 0;
		
		BlueBlock[0].x = 0;
		BlueBlock[0].y = 0;
		BlueBlock[0].w = 0;
		BlueBlock[0].h = 0;	
		
		GrayBlock[0].x = 0;
		GrayBlock[0].y = 0;
		GrayBlock[0].w = 0;
		GrayBlock[0].h = 0;
		
		BlackBlock[0].x = 0;
		BlackBlock[0].y = 0;
		BlackBlock[0].w = 0;
		BlackBlock[0].h = 0; 

		/*line two*/
		GreenBlock[1].x = 475;
		GreenBlock[1].y = 535;
		GreenBlock[1].w = 10;
		GreenBlock[1].h = 10;
		
		PinkBlock[1].x = 475;
		PinkBlock[1].y = 535;
		PinkBlock[1].w = 10;
		PinkBlock[1].h = 10;		
		
		LittleBlueBlock[1].x = 0;
		LittleBlueBlock[1].y = 0;
		LittleBlueBlock[1].w = 0;
		LittleBlueBlock[1].h = 0;
		
		RedBlock[1].x = 0;
		RedBlock[1].y = 0;
		RedBlock[1].w = 0;
		RedBlock[1].h = 0;

		YellowBlock[1].x = 0;
		YellowBlock[1].y = 0;
		YellowBlock[1].w = 0;
		YellowBlock[1].h = 0;
		
		BlueBlock[1].x = 0;
		BlueBlock[1].y = 0;
		BlueBlock[1].w = 0;
		BlueBlock[1].h = 0;	
		
		GrayBlock[1].x = 0;
		GrayBlock[1].y = 0;
		GrayBlock[1].w = 0;
		GrayBlock[1].h = 0;
		
		BlackBlock[1].x = 0;
		BlackBlock[1].y = 0;
		BlackBlock[1].w = 0;
		BlackBlock[1].h = 0; 

		/*line three*/
		GreenBlock[2].x = 475;
		GreenBlock[2].y = 895;
		GreenBlock[2].w = 10;
		GreenBlock[2].h = 10;
		
		PinkBlock[2].x = 475;
		PinkBlock[2].y = 895;
		PinkBlock[2].w = 10;
		PinkBlock[2].h = 10;		
		
		LittleBlueBlock[2].x = 0;
		LittleBlueBlock[2].y = 0;
		LittleBlueBlock[2].w = 0;
		LittleBlueBlock[2].h = 0;
		
		RedBlock[2].x = 0;
		RedBlock[2].y = 0;
		RedBlock[2].w = 0;
		RedBlock[2].h = 0;

		YellowBlock[2].x = 0;
		YellowBlock[2].y = 0;
		YellowBlock[2].w = 0;
		YellowBlock[2].h = 0;
		
		BlueBlock[2].x = 0;
		BlueBlock[2].y = 0;
		BlueBlock[2].w = 0;
		BlueBlock[2].h = 0;	
		
		GrayBlock[2].x = 0;
		GrayBlock[2].y = 0;
		GrayBlock[2].w = 0;
		GrayBlock[2].h = 0;
		
		BlackBlock[2].x = 0;
		BlackBlock[2].y = 0;
		BlackBlock[2].w = 0;
		BlackBlock[2].h = 0; 
	} 
	return TRUE;
}

/*colorbar type is master7800 PTG pattern 32 colorbar 75%*/
unsigned char Check_DisplayFrame_isRightColorbar(void)
{
	KADP_VT_RECT_T OutputRegion;
	KADP_VT_VIDEO_FRAME_OUTPUT_DEVICE_STATE_INFO_T VideoFrameOutputDeviceState;
	unsigned int IndexOfCurrentVideoFrameBuffer;
	unsigned long colorbarbufferaddr;

	VIDEO_RECT_T GrayBlock[3];
	VIDEO_RECT_T YellowBlock[3];
	VIDEO_RECT_T LittleBlueBlock[3];
	VIDEO_RECT_T GreenBlock[3];
	VIDEO_RECT_T PinkBlock[3];
	VIDEO_RECT_T RedBlock[3];
	VIDEO_RECT_T BlueBlock[3];
	VIDEO_RECT_T BlackBlock[3];
	
	if (HAL_VT_InitEx(5) == FALSE){
		rtd_pr_vt_emerg("KADP_VT_Init fail!%s=%d \n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	
	OutputRegion.x = 0;
	OutputRegion.y = 0;
	OutputRegion.w = 1920;
	OutputRegion.h = 1080;
	if (HAL_VT_SetVideoFrameOutputDeviceOutputRegion(KADP_VT_VIDEO_WINDOW_0, KADP_VT_DISPLAY_OUTPUT, &OutputRegion) == FALSE){
		rtd_pr_vt_emerg("KADP_VT_SetVideoFrameOutputDeviceOutputRegion fail!%s=%d \n", __FUNCTION__, __LINE__);
		HAL_VT_Finalize();
		return FALSE;
	}
	
	VideoFrameOutputDeviceState.bAppliedPQ = 0;
	VideoFrameOutputDeviceState.bEnabled = 1;
	VideoFrameOutputDeviceState.framerateDivide = 0;
	VideoFrameOutputDeviceState.bFreezed = 1;
	if (HAL_VT_SetVideoFrameOutputDeviceState(KADP_VT_VIDEO_WINDOW_0, KADP_VT_VIDEO_FRAME_OUTPUT_DEVICE_STATE_FREEZED, &VideoFrameOutputDeviceState) ==FALSE){
		rtd_pr_vt_emerg("KADP_VT_SetVideoFrameOutputDeviceState fail!%s=%d \n", __FUNCTION__, __LINE__);
		HAL_VT_Finalize();
		return FALSE;	
	}

	if (HAL_VT_GetVideoFrameBufferIndex(KADP_VT_VIDEO_WINDOW_0, &IndexOfCurrentVideoFrameBuffer) == FALSE){
		rtd_pr_vt_emerg("KADP_VT_SetVideoFrameOutputDeviceState fail!%s=%d \n", __FUNCTION__, __LINE__);
		HAL_VT_Finalize();
		return FALSE;			
	}	

	colorbarbufferaddr =(unsigned long) CaptureCtrl_VT.cap_buffer[IndexOfCurrentVideoFrameBuffer].cache;

	Check_colorbar_window_cal(GrayBlock,YellowBlock,LittleBlueBlock,GreenBlock,PinkBlock,RedBlock,BlueBlock,BlackBlock);

	rtd_pr_vt_emerg("GrayBlock[0] x=%d;y=%d\n",GrayBlock[0].x,GrayBlock[0].y);
	rtd_pr_vt_emerg("GrayBlock[1] x=%d;y=%d\n",GrayBlock[1].x,GrayBlock[1].y);
	rtd_pr_vt_emerg("GrayBlock[2] x=%d;y=%d\n",GrayBlock[2].x,GrayBlock[2].y);

	rtd_pr_vt_emerg("YellowBlock[0] x=%d;y=%d\n",YellowBlock[0].x,YellowBlock[0].y);
	rtd_pr_vt_emerg("YellowBlock[1] x=%d;y=%d\n",YellowBlock[1].x,YellowBlock[1].y);
	rtd_pr_vt_emerg("YellowBlock[2] x=%d;y=%d\n",YellowBlock[2].x,YellowBlock[2].y);

	rtd_pr_vt_emerg("LittleBlueBlock[0] x=%d;y=%d\n",LittleBlueBlock[0].x,LittleBlueBlock[0].y);
	rtd_pr_vt_emerg("LittleBlueBlock[1] x=%d;y=%d\n",LittleBlueBlock[1].x,LittleBlueBlock[1].y);
	rtd_pr_vt_emerg("LittleBlueBlock[2] x=%d;y=%d\n",LittleBlueBlock[2].x,LittleBlueBlock[2].y);

	rtd_pr_vt_emerg("GreenBlock[0] x=%d;y=%d\n",GreenBlock[0].x,GreenBlock[0].y);
	rtd_pr_vt_emerg("GreenBlock[1] x=%d;y=%d\n",GreenBlock[1].x,GreenBlock[1].y);
	rtd_pr_vt_emerg("GreenBlock[2] x=%d;y=%d\n",GreenBlock[2].x,GreenBlock[2].y);

	rtd_pr_vt_emerg("PinkBlock[0] x=%d;y=%d\n",PinkBlock[0].x,PinkBlock[0].y);
	rtd_pr_vt_emerg("PinkBlock[1] x=%d;y=%d\n",PinkBlock[1].x,PinkBlock[1].y);
	rtd_pr_vt_emerg("PinkBlock[2] x=%d;y=%d\n",PinkBlock[2].x,PinkBlock[2].y);

	rtd_pr_vt_emerg("RedBlock[0] x=%d;y=%d\n",RedBlock[0].x,RedBlock[0].y);
	rtd_pr_vt_emerg("RedBlock[1] x=%d;y=%d\n",RedBlock[1].x,RedBlock[1].y);
	rtd_pr_vt_emerg("RedBlock[2] x=%d;y=%d\n",RedBlock[2].x,RedBlock[2].y);

	rtd_pr_vt_emerg("BlueBlock[0] x=%d;y=%d\n",BlueBlock[0].x,BlueBlock[0].y);
	rtd_pr_vt_emerg("BlueBlock[1] x=%d;y=%d\n",BlueBlock[1].x,BlueBlock[1].y);
	rtd_pr_vt_emerg("BlueBlock[2] x=%d;y=%d\n",BlueBlock[2].x,BlueBlock[2].y);

	rtd_pr_vt_emerg("BlackBlock[0] x=%d;y=%d\n",BlackBlock[0].x,BlackBlock[0].y);
	rtd_pr_vt_emerg("BlackBlock[1] x=%d;y=%d\n",BlackBlock[1].x,BlackBlock[1].y);
	rtd_pr_vt_emerg("BlackBlock[2] x=%d;y=%d\n",BlackBlock[2].x,BlackBlock[2].y);

	rtd_pr_vt_emerg("Check_DisplayFrame_isRightColorbar start!%s=%d \n", __FUNCTION__, __LINE__);
	if(ColorBlock_Compare(colorbarbufferaddr,0xBE,0xBE,0xC2,GrayBlock[0]) == FALSE) 
	{
		HAL_VT_Finalize();
		return FALSE;
	}
	if(ColorBlock_Compare(colorbarbufferaddr,0xBE,0xBE,0xC2,GrayBlock[1]) == FALSE)
	{
		HAL_VT_Finalize();
		return FALSE;
	}
	if(ColorBlock_Compare(colorbarbufferaddr,0xBE,0xBE,0xC2,GrayBlock[2]) == FALSE)
	{
		HAL_VT_Finalize();
		return FALSE;
	}
	if(ColorBlock_Compare(colorbarbufferaddr,0xBC,0xBB,0,YellowBlock[0]) == FALSE)
	{
		HAL_VT_Finalize();
		return FALSE;
	}
	if(ColorBlock_Compare(colorbarbufferaddr,0xBC,0xBB,0,YellowBlock[1]) == FALSE)
	{
		HAL_VT_Finalize();
		return FALSE;
	}
	if(ColorBlock_Compare(colorbarbufferaddr,0xBC,0xBB,0,YellowBlock[2]) == FALSE)
	{
		HAL_VT_Finalize();
		return FALSE;
	}
	if(ColorBlock_Compare(colorbarbufferaddr,0,0xB7,0xB5,LittleBlueBlock[0]) == FALSE)
	{
		HAL_VT_Finalize();
		return FALSE;
	}
	if(ColorBlock_Compare(colorbarbufferaddr,0,0xB7,0xB5,LittleBlueBlock[1]) == FALSE)
	{
		HAL_VT_Finalize();
		return FALSE;
	}
	if(ColorBlock_Compare(colorbarbufferaddr,0,0xB7,0xB5,LittleBlueBlock[2]) == FALSE)
	{
		HAL_VT_Finalize();
		return FALSE;
	}
	if(ColorBlock_Compare(colorbarbufferaddr,0,0xB5,0,GreenBlock[0]) == FALSE)
	{
		HAL_VT_Finalize();
		return FALSE;
	}
	if(ColorBlock_Compare(colorbarbufferaddr,0,0xB5,0,GreenBlock[1]) == FALSE)
	{
		HAL_VT_Finalize();
		return FALSE;
	}
	if(ColorBlock_Compare(colorbarbufferaddr,0,0xB5,0,GreenBlock[2]) == FALSE)
	{
		HAL_VT_Finalize();
		return FALSE;
	}
	if(ColorBlock_Compare(colorbarbufferaddr,0xB6,0,0xB9,PinkBlock[0]) == FALSE)
	{
		HAL_VT_Finalize();
		return FALSE;
	}
	if(ColorBlock_Compare(colorbarbufferaddr,0xB6,0,0xB9,PinkBlock[1]) == FALSE)
	{
		HAL_VT_Finalize();
		return FALSE;
	}
	if(ColorBlock_Compare(colorbarbufferaddr,0xB6,0,0xB9,PinkBlock[2]) == FALSE)
	{
		HAL_VT_Finalize();
		return FALSE;
	}
	if(ColorBlock_Compare(colorbarbufferaddr,0xB8,0,0,RedBlock[0]) == FALSE)
	{
		HAL_VT_Finalize();
		return FALSE;
	}
	if(ColorBlock_Compare(colorbarbufferaddr,0xB8,0,0,RedBlock[1]) == FALSE)
	{
		HAL_VT_Finalize();
		return FALSE;
	}
	if(ColorBlock_Compare(colorbarbufferaddr,0xB8,0,0,RedBlock[2]) == FALSE)
	{
		HAL_VT_Finalize();
		return FALSE;
	}
	if(ColorBlock_Compare(colorbarbufferaddr,0,0,0xC0,BlueBlock[0]) == FALSE)
	{
		HAL_VT_Finalize();
		return FALSE;
	}
	if(ColorBlock_Compare(colorbarbufferaddr,0,0,0xC0,BlueBlock[1]) == FALSE)
	{
		HAL_VT_Finalize();
		return FALSE;
	}
	if(ColorBlock_Compare(colorbarbufferaddr,0,0,0xC0,BlueBlock[2]) == FALSE)
	{
		HAL_VT_Finalize();
		return FALSE;
	}
	if(ColorBlock_Compare(colorbarbufferaddr,0,0,0,BlackBlock[0]) == FALSE)
	{
		HAL_VT_Finalize();
		return FALSE;
	}
	if(ColorBlock_Compare(colorbarbufferaddr,0,0,0,BlackBlock[1]) == FALSE)
	{
		HAL_VT_Finalize();
		return FALSE;
	}
	if(ColorBlock_Compare(colorbarbufferaddr,0,0,0,BlackBlock[2]) == FALSE)
	{
		HAL_VT_Finalize();
		return FALSE;
	}
	HAL_VT_Finalize();
	rtd_pr_vt_emerg("Check_DisplayFrame_isRightColorbar success!%s=%d \n", __FUNCTION__, __LINE__);
	return TRUE;

}


/* =======================================VT_BUFFER_DUMP_DEBUG======================================================*/
#define VT_FILE_PATH "/tmp/VtCapture.raw"

static struct file* file_open(const char* path, int flags, int rights) {
	struct file* filp = NULL;
	mm_segment_t oldfs;
	int err = 0;

	oldfs = get_fs();
	set_fs(KERNEL_DS);
	filp = filp_open(path, flags, rights);
	set_fs(oldfs);
	if(IS_ERR(filp)) {
		err = PTR_ERR(filp);
		return NULL;
	}
	return filp;
}

static void file_close(struct file* file) {
	filp_close(file, NULL);
}

/*static int file_read(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) {
	mm_segment_t oldfs;
	int ret;

	oldfs = get_fs();
	set_fs(get_ds());

	ret = vfs_read(file, data, size, &offset);

	set_fs(oldfs);
	return ret;
}*/

static int file_write(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) {
	mm_segment_t oldfs;
	int ret;

	oldfs = get_fs();
	set_fs(KERNEL_DS);

	ret = vfs_write(file, data, size, &offset);

	set_fs(oldfs);
	return ret;
}

static int file_sync(struct file* file) {
	vfs_fsync(file, 0);
	return 0;
}

int vt_dump_data_to_file(unsigned int bufferindex, unsigned int size)
{
	struct file* filp = NULL;
	unsigned long outfileOffset = 0;
	if (CaptureCtrl_VT.cap_buffer[bufferindex].cache != NULL) {
		rtd_pr_vt_emerg("\n\n\n\n *****************  vt_dump_data_to_file start  ******%lx***************\n\n\n\n",(unsigned long)CaptureCtrl_VT.cap_buffer[bufferindex].cache);
		filp = file_open(VT_FILE_PATH, O_RDWR | O_CREAT, 0);
		if (filp == NULL) {
			rtd_pr_vt_notice("file open fail\n");
			return FALSE;
		}
		file_write(filp, outfileOffset, (unsigned char*)CaptureCtrl_VT.cap_buffer[bufferindex].cache, size);
		file_sync(filp);
		file_close(filp);
		rtd_pr_vt_emerg("\n\n\n\n *****************  vt_dump_data_to_file end  *********************\n\n\n\n");
		return TRUE;
	} else {
		rtd_pr_vt_emerg("\n\n\n\n *****************  vt_dump_data_to_file NG for null buffer address  *********************\n\n\n\n");
		return FALSE;
	}
}
/* =======================================VT_BUFFER_DUMP_DEBUG======================================================*/


unsigned int VIVT_PM_REGISTER_DATA[][2]
=
{
#ifdef CONFIG_HW_SUPPORT_DC2H 

	{DC2H_DMA_dc2h_Cap_L1_Start_reg, 0},
	{DC2H_DMA_dc2h_Cap_L2_Start_reg, 0},
	{DC2H_DMA_dc2h_Cap_L3_Start_reg, 0},

	{DC2H_RGB2YUV_DC2H_Tab1_M11_M12_reg, 0},
	{DC2H_RGB2YUV_DC2H_Tab1_M13_M21_reg, 0},
	{DC2H_RGB2YUV_DC2H_Tab1_M22_M23_reg, 0},
	{DC2H_RGB2YUV_DC2H_Tab1_M31_M32_reg, 0},
	{DC2H_RGB2YUV_DC2H_Tab1_M33_Y_Gain_reg, 0},
	{DC2H_RGB2YUV_DC2H_Tab1_Yo_reg, 0},
	{DC2H_RGB2YUV_DC2H_RGB2YUV_CTRL_reg, 0},

	{DC2H_SCALEDOWN_DC2H_HSD_Scale_Hor_Factor_reg, 0},
	{DC2H_SCALEDOWN_DC2H_VSD_Scale_Ver_Factor_reg, 0},
	{DC2H_SCALEDOWN_DC2H_HSD_Hor_Segment_reg, 0},
	/*{DC2H_SCALEDOWN_DC2H_HSD_Hor_Delta1_reg, 0},*/
	{DC2H_SCALEDOWN_DC2H_HSD_Initial_Value_reg, 0},
	{DC2H_SCALEDOWN_DC2H_VSD_Initial_Value_reg, 0},
	{DC2H_SCALEDOWN_DC2H_HSD_Ctrl0_reg, 0},
	{DC2H_SCALEDOWN_DC2H_VSD_Ctrl0_reg, 0},

	{DC2H_VI_DC2H_DMACTL_reg, 0},
	{DC2H_DMA_dc2h_Seq_mode_CTRL2_reg, 0},
	{DC2H_DMA_dc2h_Cap_CTL0_reg, 0},
	{DC2H_DMA_dc2h_Cap_CTL1_reg, 0},
	{DC2H_DMA_dc2h_seq_byte_channel_swap_reg, 0},
	{DC2H_DMA_dc2h_Seq_mode_CTRL1_reg, 0},

#endif
};

#ifdef CONFIG_PM
static int vivt_suspend (struct device *p_dev)
{
	//int i,size;
	if(HAL_VT_Finalize()==TRUE)
		rtd_pr_vt_emerg("%s=%d  (HAL_VT_Finalize()==TRUE)\n", __FUNCTION__, __LINE__);
	else
		rtd_pr_vt_emerg("%s=%d  (HAL_VT_Finalize()==FALSE)\n", __FUNCTION__, __LINE__);
#if 0
	size = (sizeof(VIVT_PM_REGISTER_DATA)/4)/2;
	for(i=0; i<size; i++)
	{
		VIVT_PM_REGISTER_DATA[i][1]=rtd_inl(VIVT_PM_REGISTER_DATA[i][0]);
	}
#endif	
	return 0;
}

static int vivt_resume (struct device *p_dev)
{
	//int i, size;
#if 0	
	size = (sizeof(VIVT_PM_REGISTER_DATA)/4)/2;
	for(i=0; i<size; i++)
	{
		rtd_outl(VIVT_PM_REGISTER_DATA[i][0],VIVT_PM_REGISTER_DATA[i][1]);
	}
#endif	
	return 0;
}
#endif


int vivt_open(struct inode *inode, struct file *filp) {
	return 0;
}

ssize_t  vivt_read(struct file *filep, char *buffer, size_t count, loff_t *offp)
{
	return 0;

}

ssize_t vivt_write(struct file *filep, const char *buffer, size_t count, loff_t *offp)
{
	return 0;
}

int vivt_release(struct inode *inode, struct file *filep)
{
	return 0;
}


long vivt_ioctl(struct file *file, unsigned int cmd,  unsigned long arg)
{
	int retval = 0;
	if (_IOC_TYPE(cmd) != VT_IOC_MAGIC || _IOC_NR(cmd) > VT_IOC_MAXNR) return -ENOTTY ;
	
	switch (cmd)
	{
		case VT_IOC_INIT:
		{
			if(HAL_VT_Init()==FALSE)
				retval = -1;
			break;
		}
		case VT_IOC_INITEX:
		{
			unsigned int buffernum;
			if(copy_from_user((void *)&buffernum, (const void __user *)arg, sizeof(unsigned int)))
			{
				retval = -EFAULT;
				rtd_pr_vt_debug("copy_to_user:scaler vt ioctl code=VT_IOC_INITEX failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VT_InitEx(buffernum)==FALSE)
					retval =  -1;					
			}
			break;
		}		
		case VT_IOC_FINALIZE:
		{
			//addr = *((unsigned int *)arg);
			if(HAL_VT_Finalize()==FALSE)
				retval = -1;
			break;
		}
		case VT_IOC_GET_DEVICE_CAPABILITY:
		{
			KADP_VT_DEVICE_CAPABILITY_INFO_T DeviceCapabilityInfo;
			if(HAL_VT_GetDeviceCapability(&DeviceCapabilityInfo)==FALSE) {
				retval = -1;
			} else {
				if(copy_to_user((void __user *)arg, (void *)&DeviceCapabilityInfo, sizeof(KADP_VT_DEVICE_CAPABILITY_INFO_T)))
				{
					retval = -EFAULT;
					rtd_pr_vt_debug("copy_to_user:scaler vt ioctl code=VT_IOC_GET_DEVICE_CAPABILITY failed!!!!!!!!!!!!!!!\n");
				}
			}

			break;
		}
		case VT_IOC_GET_VFB_CAPABILITY:
		{
			KADP_VT_VFB_CAPINFO_T vfb_capinfo;
			if(copy_from_user((void *)&vfb_capinfo, (const void __user *)arg, sizeof(KADP_VT_VFB_CAPINFO_T)))
			{
				retval = -EFAULT;
				rtd_pr_vt_debug("copy_from_user:scaler vt ioctl code=VT_IOC_GET_VFB_CAPABILITY failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VT_GetVideoFrameBufferCapability(vfb_capinfo.wid,&vfb_capinfo.vfbCapInfo)==FALSE) {
					retval = -1;
				} else {
					if(copy_to_user((void __user *)arg, (void *)&vfb_capinfo, sizeof(KADP_VT_VFB_CAPINFO_T)))
					{
						retval = -EFAULT;
						rtd_pr_vt_debug("copy_to_user:scaler vt ioctl code=VT_IOC_GET_VFB_CAPABILITY failed!!!!!!!!!!!!!!!\n");
					}
				}
			}
			break;
		}
		case VT_IOC_GET_VFOD_CAPABILITY:
		{
			KADP_VT_VFOD_CAPINFO_T vfod_capinfo;
			if(copy_from_user((void *)&vfod_capinfo, (const void __user *)arg, sizeof(KADP_VT_VFOD_CAPINFO_T)))
			{
				retval = -EFAULT;
				rtd_pr_vt_debug("copy_from_user:scaler vt ioctl code=VT_IOC_GET_VFOD_CAPABILITY failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VT_GetVideoFrameOutputDeviceCapability(vfod_capinfo.wid,&vfod_capinfo.vfodCapInfo)==FALSE) {
					retval = -1;
				} else {
					if(copy_to_user((void __user *)arg, (void *)&vfod_capinfo, sizeof(KADP_VT_VFOD_CAPINFO_T)))
					{
						retval = -EFAULT;
						rtd_pr_vt_debug("copy_to_user:scaler vt ioctl code=VT_IOC_GET_VFOD_CAPABILITY failed!!!!!!!!!!!!!!!\n");
					}
				}
			}
			break;
		}
		case VT_IOC_GET_VFOD_LIMITATION:
		{
			KADP_VT_VFOD_LIMITATIONINFO_T vfod_limitationinfo;
			if(copy_from_user((void *)&vfod_limitationinfo, (const void __user *)arg, sizeof(KADP_VT_VFOD_LIMITATIONINFO_T)))
			{
				retval = -EFAULT;
				rtd_pr_vt_debug("copy_from_user:scaler vt ioctl code=VT_IOC_GET_VFOD_LIMITATION failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VT_GetVideoFrameOutputDeviceLimitation(vfod_limitationinfo.wid,&vfod_limitationinfo.vfodlimitationInfo)==FALSE) {
					retval = -1;
				} else {
					if(copy_to_user((void __user *)arg, (void *)&vfod_limitationinfo, sizeof(KADP_VT_VFOD_LIMITATIONINFO_T)))
					{
						retval = -EFAULT;
						rtd_pr_vt_debug("copy_to_user:scaler vt ioctl code=VT_IOC_GET_VFOD_LIMITATION failed!!!!!!!!!!!!!!!\n");
					}
				}
			}
			break;
		}
		case VT_IOC_GET_ALLVFB_PROPERTY:
		{
			KADP_VT_VFB_PROPERTY_T vfb_property;
			if(copy_from_user((void *)&vfb_property, (const void __user *)arg, sizeof(KADP_VT_VFB_PROPERTY_T)))
			{
				retval = -EFAULT;
				rtd_pr_vt_debug("copy_from_user:scaler vt ioctl code=VT_IOC_GET_ALLVFB_PROPERTY failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VT_GetAllVideoFrameBufferProperty(vfb_property.wid,&vfb_property.vfbProInfo)==FALSE) {
					retval = -1;
				} else {
					if(copy_to_user((void __user *)arg, (void *)&vfb_property, sizeof(KADP_VT_VFB_PROPERTY_T)))
					{
						retval = -EFAULT;
						rtd_pr_vt_debug("copy_to_user:scaler vt ioctl code=VT_IOC_GET_ALLVFB_PROPERTY failed!!!!!!!!!!!!!!!\n");
					}
				}
			}
			break;
		}
		case VT_IOC_GET_VFB_INDEX:
		{
			KADP_VT_GET_VFBINDEX_T vfb_index;
			if(copy_from_user((void *)&vfb_index, (const void __user *)arg, sizeof(KADP_VT_GET_VFBINDEX_T)))
			{
				retval = -EFAULT;
				rtd_pr_vt_debug("copy_from_user:scaler vt ioctl code=VT_IOC_GET_VFB_INDEX failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VT_GetVideoFrameBufferIndex(vfb_index.wid,&vfb_index.IndexCurVFB)==FALSE) {
					retval = -1;
				} else {
					if(copy_to_user((void __user *)arg, (void *)&vfb_index, sizeof(KADP_VT_GET_VFBINDEX_T)))
					{
						retval = -EFAULT;
						rtd_pr_vt_debug("copy_to_user:scaler vt ioctl code=VT_IOC_GET_VFB_INDEX failed!!!!!!!!!!!!!!!\n");
					}
				}
			}
			break;
		}
		case VT_IOC_GET_VFOD_STATE:
		{
			KADP_VT_VFOD_GET_STATEINFO_T vfodgetstateinfo;
			if(copy_from_user((void *)&vfodgetstateinfo, (const void __user *)arg, sizeof(KADP_VT_VFOD_GET_STATEINFO_T)))
			{
				retval = -EFAULT;
				rtd_pr_vt_debug("copy_from_user:scaler vt ioctl code=VT_IOC_GET_VFOD_STATE failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VT_GetVideoFrameOutputDeviceState(vfodgetstateinfo.wid,&vfodgetstateinfo.VFODstate)==FALSE) {
					retval =-1;
				} else {
					if(copy_to_user((void __user *)arg, (void *)&vfodgetstateinfo, sizeof(KADP_VT_VFOD_GET_STATEINFO_T)))
					{
						retval = -EFAULT;
						rtd_pr_vt_debug("copy_to_user:scaler vt ioctl code=VT_IOC_GET_VFOD_STATE failed!!!!!!!!!!!!!!!\n");
					}
				}
			}
			break;
		}
		case VT_IOC_SET_VFOD_STATE:
		{
			KADP_VT_VFOD_SET_STATEINFO_T vfodsetstateinfo;
			if(copy_from_user((void *)&vfodsetstateinfo, (const void __user *)arg, sizeof(KADP_VT_VFOD_SET_STATEINFO_T)))
			{
				retval = -EFAULT;
				rtd_pr_vt_debug("copy_from_user:scaler vt ioctl code=VT_IOC_SET_VFOD_STATE failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VT_SetVideoFrameOutputDeviceState(vfodsetstateinfo.wid,vfodsetstateinfo.vfodStateFlag,&vfodsetstateinfo.VFODstate)==FALSE)
					retval =-1;
			}

			break;
		}
		case VT_IOC_GET_VFOD_FRAMERATE:
		{
			KADP_VT_VFOD_GET_FRAMERATE_T vfodgetfr;
			if(copy_from_user((void *)&vfodgetfr, (const void __user *)arg, sizeof(KADP_VT_VFOD_GET_FRAMERATE_T)))
			{
				retval = -EFAULT;
				rtd_pr_vt_debug("copy_from_user:scaler vt ioctl code=VT_IOC_GET_VFOD_FRAMERATE failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VT_GetVideoFrameOutputDeviceFramerate(vfodgetfr.wid,&vfodgetfr.framerate)==FALSE) {
					retval = -1;
				} else {
					if(copy_to_user((void __user *)arg, (void *)&vfodgetfr, sizeof(KADP_VT_VFOD_GET_FRAMERATE_T)))
					{
						retval = -EFAULT;
						rtd_pr_vt_debug("copy_to_user:scaler vt ioctl code=VT_IOC_GET_VFOD_FRAMERATE failed!!!!!!!!!!!!!!!\n");
					}
				}
			}

			break;
		}
		case VT_IOC_GET_VFOD_DUMPLOCATION:
		{
			KADP_VT_VFOD_GET_DUMPLOCATION_T vfodgetdumplocation;
			if(copy_from_user((void *)&vfodgetdumplocation, (const void __user *)arg, sizeof(KADP_VT_VFOD_GET_DUMPLOCATION_T)))
			{
				retval = -EFAULT;
				rtd_pr_vt_debug("copy_from_user:scaler vt ioctl code=VT_IOC_GET_VFOD_DUMPLOCATION failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VT_GetVideoFrameOutputDeviceDumpLocation(vfodgetdumplocation.wid,&vfodgetdumplocation.DumpLocation)==FALSE) {
					retval =-1;
				} else {
					if(copy_to_user((void __user *)arg, (void *)&vfodgetdumplocation, sizeof(KADP_VT_VFOD_GET_DUMPLOCATION_T)))
					{
						retval = -EFAULT;
						rtd_pr_vt_debug("copy_to_user:scaler vt ioctl code=VT_IOC_GET_VFOD_DUMPLOCATION failed!!!!!!!!!!!!!!!\n");
					}
				}
			}

			break;
		}
		case VT_IOC_SET_VFOD_DUMPLOCATION:
		{
			KADP_VT_VFOD_SET_DUMPLOCATION_T vfodsetdumplocation;
			if(copy_from_user((void *)&vfodsetdumplocation, (const void __user *)arg, sizeof(KADP_VT_VFOD_SET_DUMPLOCATION_T)))
			{
				retval = -EFAULT;
				rtd_pr_vt_debug("copy_from_user:scaler vt ioctl code=VT_IOC_SET_VFOD_DUMPLOCATION failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VT_SetVideoFrameOutputDeviceDumpLocation(vfodsetdumplocation.wid,vfodsetdumplocation.DumpLocation)==FALSE)
					retval =-1;
			}

			break;
		}
		case VT_IOC_GET_VFOD_OUTPUTINFO:
		{
			KADP_VT_VFOD_GET_OUTPUTINFO_T vfodgetoututinfo;
			if(copy_from_user((void *)&vfodgetoututinfo, (const void __user *)arg, sizeof(KADP_VT_VFOD_GET_OUTPUTINFO_T)))
			{
				retval = -EFAULT;
				rtd_pr_vt_debug("copy_from_user:scaler vt ioctl code=VT_IOC_GET_VFOD_OUTPUTINFO failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VT_GetVideoFrameOutputDeviceOutputInfo(vfodgetoututinfo.wid,vfodgetoututinfo.DumpLocation,&vfodgetoututinfo.OutputInfo)==FALSE) {
					retval =-1;
				} else {
					if(copy_to_user((void __user *)arg, (void *)&vfodgetoututinfo, sizeof(KADP_VT_VFOD_GET_OUTPUTINFO_T)))
					{
						retval = -EFAULT;
						rtd_pr_vt_debug("copy_to_user:scaler vt ioctl code=VT_IOC_GET_VFOD_OUTPUTINFO failed!!!!!!!!!!!!!!!\n");
					}
				}
			}

			break;
		}
		case VT_IOC_SET_VFOD_OUTPUTREGION:
		{
			KADP_VT_VFOD_SET_OUTPUTREGION_T vfodsetoututinfo;
			if(copy_from_user((void *)&vfodsetoututinfo, (const void __user *)arg, sizeof(KADP_VT_VFOD_SET_OUTPUTREGION_T)))
			{
				retval = -EFAULT;
				rtd_pr_vt_debug("copy_from_user:scaler vt ioctl code=VT_IOC_SET_VFOD_OUTPUTREGION failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VT_SetVideoFrameOutputDeviceOutputRegion(vfodsetoututinfo.wid,vfodsetoututinfo.DumpLocation,&vfodsetoututinfo.OutputRegion)==FALSE)
					retval =-1;
			}

			break;
		}
		case VT_IOC_WAIT_VSYNC:
		{
			KADP_VT_VIDEO_WINDOW_TYPE_T videoWindowID;
			if(copy_from_user((void *)&videoWindowID, (const void __user *)arg, sizeof(KADP_VT_VIDEO_WINDOW_TYPE_T)))
			{
				retval = -EFAULT;
				rtd_pr_vt_debug("scaler vt ioctl code=VT_IOC_WAIT_VSYNC failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VT_WaitVsync(videoWindowID)==FALSE)
					retval =-1;
			}
			break;
		}
		case VT_IOC_GET_INPUTVIDEOINFO:
		{
			KADP_VT_GET_INPUTVIDEOINFO_T inputvideoinfo;
			if(copy_from_user((void *)&inputvideoinfo, (const void __user *)arg, sizeof(KADP_VT_GET_INPUTVIDEOINFO_T)))
			{
				retval = -EFAULT;
				rtd_pr_vt_debug("copy_from_user:scaler vt ioctl code=VT_IOC_GET_INPUTVIDEOINFO failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VT_GetInputVideoInfo(inputvideoinfo.wid,&inputvideoinfo.InputVideoInfo)==FALSE) {
					retval =-1;
				} else {
					if(copy_to_user((void __user *)arg, (void *)&inputvideoinfo, sizeof(KADP_VT_GET_INPUTVIDEOINFO_T)))
					{
						retval = -EFAULT;
						rtd_pr_vt_debug("copy_to_user:scaler vt ioctl code=VT_IOC_GET_INPUTVIDEOINFO failed!!!!!!!!!!!!!!!\n");
					}
				}
			}

			break;
		}
		case VT_IOC_GET_OUTPUTVIDEOINFO:
		{
			KADP_VT_GET_OUTPUTVIDEOINFO_T onputvideoinfo;
			if(copy_from_user((void *)&onputvideoinfo, (const void __user *)arg, sizeof(KADP_VT_GET_OUTPUTVIDEOINFO_T)))
			{
				retval = -EFAULT;
				rtd_pr_vt_debug("scaler vt ioctl code=VT_IOC_GET_OUTPUTVIDEOINFO failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VT_GetOutputVideoInfo(onputvideoinfo.wid,&onputvideoinfo.OutputVideoInfo)==FALSE) {
					retval =-1;
				} else {
					if(copy_to_user((void __user *)arg, (void *)&onputvideoinfo, sizeof(KADP_VT_GET_OUTPUTVIDEOINFO_T)))
					{
						retval = -EFAULT;
						rtd_pr_vt_debug("copy_to_user:scaler vt ioctl code=VT_IOC_GET_OUTPUTVIDEOINFO failed!!!!!!!!!!!!!!!\n");
					}
				}
			}

			break;
		}
		case VT_IOC_GET_VIDEOMUTESTATUS:
		{
			KADP_VT_GET_VIDEOMUTESTATUS_T videomutestatus;
			if(copy_from_user((void *)&videomutestatus, (const void __user *)arg, sizeof(KADP_VT_GET_VIDEOMUTESTATUS_T)))
			{
				retval = -EFAULT;
				rtd_pr_vt_debug("copy_from_user:scaler vt ioctl code=VT_IOC_GET_VIDEOMUTESTATUS failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VT_GetVideoMuteStatus(videomutestatus.wid,&videomutestatus.bOnOff)==FALSE) {
					retval =-1;
				} else {
					if(copy_to_user((void __user *)arg, (void *)&videomutestatus, sizeof(KADP_VT_GET_VIDEOMUTESTATUS_T)))
					{
						retval = -EFAULT;
						rtd_pr_vt_debug("copy_to_user:scaler vt ioctl code=VT_IOC_GET_VIDEOMUTESTATUS failed!!!!!!!!!!!!!!!\n");
					}
				}
			}

			break;
		}
		case VT_IOC_GET_VFB_SVSTATE:
		{
			KADP_VT_GET_VFBSVSTATE_T vfbsvstatus;
			if(copy_from_user((void *)&vfbsvstatus, (const void __user *)arg, sizeof(KADP_VT_GET_VFBSVSTATE_T)))
			{
				retval = -EFAULT;
				rtd_pr_vt_debug("copy_from_user:scaler vt ioctl code=VT_IOC_GET_VFB_SVSTATE failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VT_GetVideoFrameBufferSecureVideoState(vfbsvstatus.wid,&vfbsvstatus.IsSecureVideo)==FALSE) {
					retval =-1;
				} else {
					if(copy_to_user((void __user *)arg, (void *)&vfbsvstatus, sizeof(KADP_VT_GET_VFBSVSTATE_T)))
					{
						retval = -EFAULT;
						rtd_pr_vt_debug("copy_to_user:scaler vt ioctl code=VT_IOC_GET_VFB_SVSTATE failed!!!!!!!!!!!!!!!\n");
					}
				}
			}
			break;
		}
		case VT_IOC_GET_VFOD_BLOCKSTATE:
		{
			KADP_VT_GET_VFODBLOCKSTATE_T vfodblockstatus;
			if(copy_from_user((void *)&vfodblockstatus, (const void __user *)arg, sizeof(KADP_VT_GET_VFODBLOCKSTATE_T)))
			{
				retval = -EFAULT;
				rtd_pr_vt_debug("scaler vt ioctl code=VT_IOC_GET_VFOD_BLOCKSTATE failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VT_GetVideoFrameOutputDeviceBlockState(vfodblockstatus.wid,&vfodblockstatus.bBlockState)==FALSE) {
					retval =-1;
				} else {
					if(copy_to_user((void __user *)arg, (void *)&vfodblockstatus, sizeof(KADP_VT_GET_VFODBLOCKSTATE_T)))
					{
						retval = -EFAULT;
						rtd_pr_vt_debug("copy_to_user:scaler vt ioctl code=VT_IOC_GET_VFOD_BLOCKSTATE failed!!!!!!!!!!!!!!!\n");
					}

				}
			}
			break;
		}
		case VT_IOC_SET_VFOD_BLOCKSTATE:
		{
			KADP_VT_SET_VFODBLOCKSTATE_T setvfodblockstatus;
			if(copy_from_user((void *)&setvfodblockstatus, (const void __user *)arg, sizeof(KADP_VT_SET_VFODBLOCKSTATE_T)))
			{
				retval = -EFAULT;
				rtd_pr_vt_debug("scaler vt ioctl code=VT_IOC_SET_VFOD_BLOCKSTATE failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VT_SetVideoFrameOutputDeviceBlockState(setvfodblockstatus.wid,setvfodblockstatus.bBlockState)==FALSE)
					retval =-1;
			}
			break;
		}
		case VT_IOC_SET_PIXEL_FORMAT:
		{	
			VT_CAP_FMT format;
			if(copy_from_user((void *)&format, (const void __user *)arg, sizeof(VT_CAP_FMT)))
			{
				retval = -EFAULT;
				rtd_pr_vt_debug("scaler vt ioctl code=VT_IOC_SET_PIXEL_FORMAT failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VT_set_Pixel_Format(format)==FALSE)
					retval =-1;
			}
			break;
		}		
		case VT_IOC_VFB_DATASHOW_ONSUBDISP:
		{	
			unsigned char vfbdatashowonsub;
			if(copy_from_user((void *)&vfbdatashowonsub, (const void __user *)arg, sizeof(unsigned char)))
			{
				retval = -EFAULT;
				rtd_pr_vt_debug("scaler vt ioctl code=VT_IOC_VFB_DATASHOW_ONSUBDISP failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(Scaler_SubMDispWindow_VFB(vfbdatashowonsub)==FALSE)
					retval =-1;
			}
			break;
		}
		case VT_IOC_EnableFRCMode:
		{
			unsigned int bEnableFrcMode;
			if(copy_from_user((void *)&bEnableFrcMode, (const void __user *)arg, sizeof(unsigned int)))
			{
				retval = -EFAULT;
				rtd_pr_vt_debug("copy_to_user:scaler vt ioctl code=VT_IOC_INITEX failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char bEnableFrc;
				bEnableFrc = (unsigned char)bEnableFrcMode;
				if(HAL_VT_EnableFRCMode(bEnableFrc)==FALSE)
					retval =  -1;
			}
			break;
		}			
		default:
			rtd_pr_vt_debug("Scaler vt disp: ioctl code = %d is invalid!!!!!!!!!!!!!!!1\n", cmd);
			break ;
		}
	return retval;


}

#if defined(CONFIG_COMPAT) && defined(CONFIG_ARM64)
long vivt_compat_ioctl(struct file *file, unsigned int cmd,  unsigned long arg)
{ 
	return vivt_ioctl(file,cmd,arg);
}
#endif

int vivt_major   = VT_MAJOR;
int vivt_minor   = 0 ;
int vivt_nr_devs = VT_NR_DEVS;

module_param(vivt_major, int, S_IRUGO);
module_param(vivt_minor, int, S_IRUGO);
module_param(vivt_nr_devs, int, S_IRUGO);


static struct class *vivt_class = NULL;
static struct platform_device *vivt_platform_devs = NULL;

struct file_operations vivt_fops= {
	.owner =    THIS_MODULE,
	.open  =    vivt_open,
	.release =  vivt_release,
	.read  =    vivt_read,
	.write = 	vivt_write,
	.unlocked_ioctl = 	vivt_ioctl,
#if defined(CONFIG_COMPAT) && defined(CONFIG_ARM64)
  	.compat_ioctl = vivt_compat_ioctl,
#endif
};

#ifdef CONFIG_PM
static const struct dev_pm_ops vivt_pm_ops =
{
	.suspend    = vivt_suspend,
	.resume     = vivt_resume,
#ifdef CONFIG_HIBERNATION
	.freeze 	= vivt_suspend,
	.thaw		= vivt_resume,
	.poweroff	= vivt_suspend,
	.restore	= vivt_resume,
#endif

};
#endif

static struct platform_driver vivt_device_driver = {
    .driver = {
        .name       = VT_DEVICE_NAME,
        .bus        = &platform_bus_type,
#ifdef CONFIG_PM
	.pm 		= &vivt_pm_ops,
#endif

    },
} ;

#if 0//removed
static char *vivt_devnode(struct device *dev, mode_t *mode)
{
	*mode =0666;
	return NULL;
}
#endif
#ifdef CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER
static void sb4_dc2h_debug_info(void)
{
	dcmt_trap_info trap_info;
	unsigned int i = 0;
	//check trash module
	if(!isDcmtTrap("TVSB4_DC2H"))  //"SB3_MD_SCPU"?trap ip,ip?????rtk_dc_mt.c??module_info[]
	{
		rtd_pr_vt_err("not TVSB4_DC2H module!\n");
		return;
	}

	get_dcmt_trap_info(&trap_info);  //get dcmt trap information (include trash_addr,module_id,rw_type)
	rtd_pr_vt_err("TVSB4_DC2H module trashed somewhere!\n");

	//dc2h address and control
	for (i = 0xb8029900; i <=0xb8029a70; i=i+4)
	{
		rtd_pr_vt_err("%x=%x\n", i, IoReg_Read32(i));
	}

	for (i = 0xb8029c00; i <=0xb8029c18; i=i+4)
	{
		rtd_pr_vt_err("%x=%x\n", i, IoReg_Read32(i));
	}

	rtd_pr_vt_err("0xb8029c20=%x\n", IoReg_Read32(0xb8029c20));

	for (i = 0xb8029c80; i <=0xb8029c88; i=i+4)
	{
		rtd_pr_vt_err("%x=%x\n", i, IoReg_Read32(i));
	}

	for (i = 0xb8029d00; i <=0xb8029d44; i=i+4)
	{
		rtd_pr_vt_err("%x=%x\n", i, IoReg_Read32(i));
	}

	rtd_pr_vt_err("0xb8029d68=%x\n", IoReg_Read32(0xb8029d68));
	rtd_pr_vt_err("0xb8029d6c=%x\n", IoReg_Read32(0xb8029d6c));

	for (i = 0xb8029d70; i <=0xb8029db8; i=i+4)
	{
		rtd_pr_vt_err("%x=%x\n", i, IoReg_Read32(i));
	}
	return;
}
#endif

DCMT_DEBUG_INFO_DECLARE(sb4_dc2h_mdscpu, sb4_dc2h_debug_info);

int vivt_init_module(void)
{
	int result;
  	int devno;
	dev_t dev = 0;

#ifdef CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER
	/******************DCMT register callback func in init flow******************/
	DCMT_DEBUG_INFO_REGISTER(sb4_dc2h_mdscpu, sb4_dc2h_debug_info);
	rtd_pr_vt_debug("DCMT_DEBUG_INFO_REGISTER(sb4_dc2h_mdscpu, sb4_dc2h_debug_info)\n");
#endif

	rtd_pr_vt_debug("\n\n\n\n *****************  vt init module  *********************\n\n\n\n");
	if (vivt_major) {
		dev = MKDEV(vivt_major, vivt_minor);
		result = register_chrdev_region(dev, vivt_nr_devs, VT_DEVICE_NAME);
	} else {
		result = alloc_chrdev_region(&dev, vivt_minor, vivt_nr_devs,VT_DEVICE_NAME);
		vivt_major = MAJOR(dev);
	}
	if (result < 0) {
		rtd_pr_vt_debug("vt: can't get major %d\n", vivt_major);
		return result;
	}

	rtd_pr_vt_debug("vt init module major number = %d\n", vivt_major);

	vivt_class = class_create(THIS_MODULE,VT_DEVICE_NAME);

	if (IS_ERR(vivt_class))
	{
		rtd_pr_vt_debug("scalevt: can not create class...\n");
		result = PTR_ERR(vivt_class);
		goto fail_class_create;
	}

	// vivt_class->devnode = (void *)vivt_devnode;

	vivt_platform_devs = platform_device_register_simple(VT_DEVICE_NAME, -1, NULL, 0);

    	if((result=platform_driver_register(&vivt_device_driver)) != 0){
		rtd_pr_vt_debug("scalevt: can not register platform driver...\n");
		result = -ENOMEM;
		goto fail_platform_driver_register;
    	}

    	devno = MKDEV(vivt_major, vivt_minor);
    	cdev_init(&vivt_cdev, &vivt_fops);
    	vivt_cdev.owner = THIS_MODULE;
   	vivt_cdev.ops = &vivt_fops;
	result = cdev_add (&vivt_cdev, devno, 1);
	if (result<0)
	{
		rtd_pr_vt_debug("scalevt: can not add character device...\n");
		goto fail_cdev_init;
	}
    	device_create(vivt_class, NULL, MKDEV(vivt_major, 0), NULL, VT_DEVICE_NAME);
	sema_init(&VT_Semaphore, 1);
	sema_init(&VTBlock_Semaphore, 1);
    return 0;	//success

fail_cdev_init:
	platform_driver_unregister(&vivt_device_driver);
fail_platform_driver_register:
	platform_device_unregister(vivt_platform_devs);
	vivt_platform_devs = NULL;
	class_destroy(vivt_class);
fail_class_create:
	vivt_class = NULL;
	unregister_chrdev_region(vivt_devno, 1);
	return result;
}



void __exit vivt_cleanup_module(void)
{
	dev_t devno = MKDEV(vivt_major, vivt_minor);
	rtd_pr_vt_debug("rtice clean module vt_major = %d\n", vivt_major);
  	device_destroy(vivt_class, MKDEV(vivt_major, 0));
  	class_destroy(vivt_class);
	vivt_class = NULL;
	cdev_del(&vivt_cdev);
   	/* device driver removal */
	if(vivt_platform_devs) {
		platform_device_unregister(vivt_platform_devs);
		vivt_platform_devs = NULL;
	}
  	platform_driver_unregister(&vivt_device_driver);
	/* cleanup_module is never called if registering failed */
	unregister_chrdev_region(devno, vivt_nr_devs);
}

module_init(vivt_init_module) ;
module_exit(vivt_cleanup_module) ;

#endif

