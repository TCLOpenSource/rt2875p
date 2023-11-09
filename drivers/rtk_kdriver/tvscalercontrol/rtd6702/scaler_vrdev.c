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
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <uapi/linux/const.h>
#include <linux/mm.h>
#include <rtd_log/rtd_module_log.h>
#include <mach/platform.h>
#include <linux/fs.h>/*for dump buffer to usb file*/
#include <linux/hrtimer.h>/*for wait vsync hr sleep*/
#include <linux/version.h>
#include <rtk_kdriver/RPCDriver.h>
#include <rtk_dc_mt.h>

#include <rbus/dc2h_dma_reg.h>
#include <rbus/dc2h_vi_reg.h>
#include <rbus/dc2h_scaledown_reg.h>
#include <rbus/dc2h_rgb2yuv_reg.h>
#include <rbus/ppoverlay_reg.h>
#include <rbus/mdomain_disp_reg.h>
#include <rbus/scaleup_reg.h>
#include <rbus/yuv2rgb_reg.h>
#include <rbus/timer_reg.h>
#include <rbus/two_step_uzu_reg.h>
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
#include <tvscalercontrol/scalerdrv/zoom_smoothtoggle_lib.h>
#include <scaler/vipRPCCommon.h>
#include <tvscalercontrol/vip/ultrazoom.h>

#include "scaler_vrdev.h"
#include "scaler_vscdev.h"
#include "scaler_vpqmemcdev.h"

/* local variable define */
struct semaphore VR_Semaphore;
static struct semaphore VRBlock_Semaphore;

#define _ALIGN(val, align) (((val) + ((align) - 1)) & ~((align) - 1))
#define __4KPAGE  0x1000
/*4k and 96 align*/
#define __12KPAGE  0x3000

unsigned int VR_CAPTURE_BUFFER_UNITSIZE;

VR_CAPTURE_CTRL_T CaptureCtrl_VR;

static dev_t vivr_devno = 0;//vr device number
static struct cdev vivr_cdev;


/*vr init work width and length*/
#define VR_CAP_FRAME_DEFAULT_WIDTH	1920
#define VR_CAP_FRAME_DEFAULT_LENGTH	1080

#define VR_CAP_FRAME_WIDTH_2K1k		1920
#define VR_CAP_FRAME_HEIGHT_2K1k	1080
#define VR_CAP_FRAME_WIDTH_4K2K		3840
#define VR_CAP_FRAME_HEIGHT_4K2K	2160
#define VR_CAP_FRAME_WIDTH_8k4k     7680
#define VR_CAP_FRAME_HEIGHT_8k4k    4320
#define VR_CAP_ORBIT_WIDTH_8K4K     7808
#define VR_CAP_ORBIT_HEIGHT_8K4K	4384
#define VR_CAP_ORBIT_WIDTH_4K2K     3904
#define VR_CAP_ORBIT_HEIGHT_4K2K	2192


#define VR_REPEAT_IDX_INVALID  0xFF
#define VR_FPS_OUTPUT 	(60)
#define MAX_CAP_BUF_NUMS 5

#define ABNORMAL_DVS_FIRST_HAPPEN 1
#define WAIT_DVS_STABLE_COUNT 8

KADP_VR_VIDEO_FRAME_OUTPUT_DEVICE_STATE_INFO_T VFODStateVR = {FALSE,FALSE,TRUE,1};

static KADP_VR_DUMP_LOCATION_TYPE_T VRDumpLocation = KADP_VR_MAX_LOCATION;
static VR_CUR_CAPTURE_INFO curCapInfo;
static unsigned char VrFunction = FALSE;
static unsigned char VrEnableFRCMode = TRUE;

static unsigned int VrBufferNum = 1;
static unsigned char VrSwBufferMode = FALSE;

static unsigned int vr_cap_frame_max_width = VR_CAP_FRAME_WIDTH_2K1k; 
static unsigned int vr_cap_frame_max_height = VR_CAP_FRAME_HEIGHT_2K1k;

static unsigned long VrAllocatedBufferStartAdress[5] = {0};

static unsigned int abnormal_dvs_cnt = 0;
static unsigned int wait_dvs_stable_cnt = 0;
 
static VR_CAP_FMT VR_Pixel_Format = VR_CAP_RGB888;
static unsigned int uiTargetVrFps = 0;

static unsigned char vr_dqbuf_ioctl_fail_flag = FALSE;
static unsigned char IndexOfFreezedVideoFrameBufferVr = 0;
static unsigned char IndexWhenVrBlocked = VR_REPEAT_IDX_INVALID;
static unsigned int vr_capture_Width = VR_CAP_FRAME_DEFAULT_WIDTH;
static unsigned int vr_capture_Height = VR_CAP_FRAME_DEFAULT_LENGTH;
static unsigned int vr_capture_location = KADP_VR_DISPLAY_OUTPUT;
static unsigned char VRBlockStatus = FALSE;

/*------declear extern functions----------*/
extern unsigned char force_disp_s1_s2_same_clk(void);/* get d domain go two pixel mode? */
extern unsigned char get_vsc_mutestatus(void);
extern unsigned char drvif_scaler_get_abnormal_dvs_long_flag(void);
extern unsigned char drvif_scaler_get_abnormal_dvs_short_flag(void);
extern unsigned char get_MEMC_bypass_status_refer_platform_model(void);
extern int MEMC_Get_DMA_malloc_address(UINT8 status, UINT32 *DMA_release_start, UINT32 *DMA_release_size);
extern void MEMC_Set_malloc_address(UINT8 status);
extern unsigned char get_vdec_securestatus(void);
extern unsigned char get_vt_function(void);

/*------declear internal functions----------*/
unsigned char Capture_BufferMemInit_VR(unsigned int buffernum);
void Capture_BufferMemDeInit_VR(unsigned int buffernum);
unsigned char HAL_VR_Finalize(void);
void drvif_vr_set_DC2H_CaptureConfig(VR_CUR_CAPTURE_INFO capInfo);
unsigned char vr_is_4K2K_capture(void);
void vr_update_cap_buffer_size_by_AP(unsigned int usr_width, unsigned int usr_height);
void vr_reset_dc2h_hw_setting(void);
unsigned char HAL_VR_GetVideoFrameBufferIndex(KADP_VR_VIDEO_WINDOW_TYPE_T videoWindowID, unsigned int *pIndexOfCurrentVideoFrameBuffer);
void vr_set_memc_mute(bool enable);
void vr_set_DC2H_cap_boundaryaddr(unsigned int VrBufferNum);
unsigned long vr_bufAddr_align(unsigned int val);

unsigned char do_vr_capture_streamon(void);
unsigned char do_vr_reqbufs(unsigned int buf_cnt);
unsigned char do_vr_streamoff(void);
unsigned char do_vr_dqbuf(unsigned int *pdqbuf_Index);

void vr_enable_dc2h(unsigned char state);
unsigned char vr_get_dc2h_capture_state(void);
unsigned int vr_sort_boundary_addr_min_index(void);
unsigned int vr_sort_boundary_addr_max_index(void);
unsigned char vr_dump_data_to_file(unsigned int idx, unsigned int dump_w,unsigned int dump_h);


#define ENABLE_VR_TEST_CASE_CONFIG
#ifdef ENABLE_VR_TEST_CASE_CONFIG
typedef enum _VR_TEST_CASE{
	VR_TEST_CASE_DISABLE = 0,
	VR_TEST_SINGLE_CAPTURE_CASE,
	VR_TEST_ACR_CASE,
	VR_TEST_VR360_CASE,
}VR_TEST_CASE_T;

VR_TEST_CASE_T vr_test_state = VR_TEST_CASE_DISABLE;
void set_vr_test_case(VR_TEST_CASE_T enable)
{
	vr_test_state = enable;
}
VR_TEST_CASE_T get_vr_test_case(void)
{
	return vr_test_state;
}

void handle_vr_test_case(void);
#endif

unsigned char get_vr_function(void)
{
	return VrFunction;
}

void set_vr_function(unsigned char value)
{
	VrFunction = value;
}

unsigned char get_vr_EnableFRCMode(void)
{
	return VrEnableFRCMode;
}

void set_vr_EnableFRCMode(unsigned char value)
{
	VrEnableFRCMode = value;
}

void vr_set_dqbuf_ioctl_fail_flag(unsigned char flg)
{
	vr_dqbuf_ioctl_fail_flag = flg;
}

unsigned char vr_get_dqbuf_ioctl_fail_flag(void)
{
	return vr_dqbuf_ioctl_fail_flag;
}

unsigned int get_VrBufferNum(void)
{
	return VrBufferNum;
}

void set_VrBufferNum(unsigned int value)
{
	VrBufferNum = value;
}

unsigned char get_VrSwBufferMode(void)
{
	return VrSwBufferMode;
}

void set_VrSwBufferMode(unsigned char value)
{
	VrSwBufferMode = value;
}

void set_VR_Pixel_Format(VR_CAP_FMT value)
{
	VR_Pixel_Format = value;
}

VR_CAP_FMT get_VR_Pixel_Format(void)
{
	return VR_Pixel_Format;
}

unsigned int get_VR_framerateDivide(void)
{
	return VFODStateVR.framerateDivide;
}
void set_VR_framerateDivide(unsigned int value)
{
	VFODStateVR.framerateDivide = value;
}

void set_vr_target_fps(unsigned int val)
{
    uiTargetVrFps = val;
}
unsigned int get_vr_target_fps(void)
{
    return uiTargetVrFps;
}

void set_vr_blockStatus(unsigned char val)
{
    VRBlockStatus = val;
}

unsigned char get_vr_blockStatus(void)
{
	return VRBlockStatus;
}

/*
D-domain Capture to Handler input data select, from :
0 : No input (data all 0)
1 : Select data from main_uzu output
2 .Select data from yuv2rgb output
3 : Select data from Sub-Title Mixer output
4 : Select data from OSD Mixer output 
5 : Select data from Dither output 
6 : Select data from Sub UZU output
7: Select data from Memc mux output
8. select data from VD output
*/

typedef enum _DC2H_IN_SEL
{
    _NO_INPUT= 0,
    _MAIN_UZU,
    _MAIN_YUV2RGB,
    _TWO_SECOND_UZU,  //2nd_uzu = 3
    _OSD_MIXER,
    _Dither_Output,  
    _Sub_UZU_Output,
    _Memc_mux_Output,
    _Memc_Mux_Input, //8
     _THIRD_UZU,//3rd_uzu=9
    _VD_OUTPUT
}DC2H_IN_SEL;

typedef struct
{
    unsigned int reserved:18 ;
    unsigned int y_pic_no:7;
    unsigned int c_pic_no:7;
} DC2H_YC_PIC_NO ;


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
} DC2H_HANDLER;


DC2H_HANDLER *pdc2h_Vrhdl = NULL;
static DC2H_HANDLER dc2h_Vrhdl;

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
#define _RGB2YUV_Yo_odd			    0x0a
#define _RGB2YUV_Y_gain				0x0b
#define _RGB2YUV_sel_RGB			0x0c
#define _RGB2YUV_set_Yin_offset		0x0d
#define _RGB2YUV_set_UV_out_offset	0x0e
#define _RGB2YUV_sel_UV_off			0x0f
#define _RGB2YUV_Matrix_bypass		0x10
#define _RGB2YUV_Enable_Y_gain		0x11

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


static short tScale_Down_Coef_2tap[] = {
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    31,  95, 159, 223, 287, 351, 415, 479,
    543, 607, 671, 735, 799, 863, 927, 991,
};

static short tScale_Down_Coef_Blur[] = {
	//2,   3,   5,   7,  10,  15,  20,  28,  38,  49,  64,  81, 101, 124, 150, 178,
	//209, 242, 277, 314, 351, 389, 426, 462, 496, 529, 556, 582, 602, 618, 629, 635,
	213,  217,  221,  222,  224,  229,  232,  234,
	239,  242,  245,  249,  252,  255,  259,  262,
	265,  266,  267,  269,  271,  272,  273,  275,
	276,  277,  278,  280,  281,  281,  282,  284,
};

static short tScale_Down_Coef_Mid[] = {
	//-2,  0,   1,   2,   5,   9,  15,  22,  32,  45,  60,  77,  98, 122, 149, 179,
	//211, 245, 281, 318, 356, 394, 431, 468, 502, 533, 561, 586, 606, 620, 630, 636,
	131,  125,  124,  126,  130,  135,  142,  151,
	161,  172,  183,  195,  208,  221,  234,  245,
	260,  275,  290,  304,  318,  330,  341,  351,
	361,  369,  376,  381,  386,  389,  390,  388,
};


static short tScale_Down_Coef_Sharp[] = {
    -2,   0,   1,   3,   6,  10,  15,  22,  32,  43,  58,  75,  95, 119, 145, 174,
    206, 240, 276, 314, 353, 391, 430, 467, 503, 536, 565, 590, 612, 628, 639, 646,
};


static  short *tScaleDown_COEF_TAB[] =
{
    tScale_Down_Coef_Blur,     tScale_Down_Coef_Mid,     tScale_Down_Coef_Sharp,     tScale_Down_Coef_2tap
};

static void dc2h_wait_porch(void)
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
		rtd_pr_vr_debug("[UZU DEN]timeout error!!!\n");
	}
}

void vr_enable_dc2h(unsigned char state)
{
	curCapInfo.enable = state; 
	drvif_vr_set_DC2H_CaptureConfig(curCapInfo);
}

unsigned char vr_get_dc2h_capture_state(void)
{	
	if ((get_VR_Pixel_Format() == VR_CAP_NV12) || (get_VR_Pixel_Format() == VR_CAP_NV16)||(get_VR_Pixel_Format() == VR_CAP_NV21))
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

unsigned char drvif_vr_dc2h_swmode_inforpc(unsigned int onoff,unsigned int buffernumber,unsigned int width,unsigned int length){

	DC2H_SWMODE_STRUCT_T *swmode_infoptr;
	//unsigned int ulCount = 0;
	int ret;
	rtd_pr_vr_notice("fun:%s\n",__FUNCTION__);
	swmode_infoptr = (DC2H_SWMODE_STRUCT_T *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_DC2H_SWMODE_ENABLE);
	//ulCount = sizeof(DC2H_SWMODE_STRUCT_T) / sizeof(unsigned int);

	swmode_infoptr->SwModeEnable = onoff;
	swmode_infoptr->buffernumber = buffernumber;
	swmode_infoptr->cap_format = (UINT32)get_VR_Pixel_Format();
	swmode_infoptr->cap_width = vr_cap_frame_max_width;
	swmode_infoptr->cap_length = vr_cap_frame_max_height;
	swmode_infoptr->YbufferSize = vr_bufAddr_align(vr_cap_frame_max_width*vr_cap_frame_max_height);
	if(Get_PANEL_VFLIP_ENABLE())
	{
		swmode_infoptr->cap_buffer[0] = ((UINT32)CaptureCtrl_VR.cap_buffer[0].phyaddr + (vr_cap_frame_max_width*vr_cap_frame_max_height - vr_cap_frame_max_width));
		swmode_infoptr->cap_buffer[1] = ((UINT32)CaptureCtrl_VR.cap_buffer[1].phyaddr + (vr_cap_frame_max_width*vr_cap_frame_max_height - vr_cap_frame_max_width));
		swmode_infoptr->cap_buffer[2] = ((UINT32)CaptureCtrl_VR.cap_buffer[2].phyaddr + (vr_cap_frame_max_width*vr_cap_frame_max_height - vr_cap_frame_max_width));
		swmode_infoptr->cap_buffer[3] = ((UINT32)CaptureCtrl_VR.cap_buffer[3].phyaddr + (vr_cap_frame_max_width*vr_cap_frame_max_height - vr_cap_frame_max_width));
		swmode_infoptr->cap_buffer[4] = ((UINT32)CaptureCtrl_VR.cap_buffer[4].phyaddr + (vr_cap_frame_max_width*vr_cap_frame_max_height - vr_cap_frame_max_width));
	}
	else
	{
		swmode_infoptr->cap_buffer[0] = (UINT32)CaptureCtrl_VR.cap_buffer[0].phyaddr;
		swmode_infoptr->cap_buffer[1] = (UINT32)CaptureCtrl_VR.cap_buffer[1].phyaddr;
		swmode_infoptr->cap_buffer[2] = (UINT32)CaptureCtrl_VR.cap_buffer[2].phyaddr;
		swmode_infoptr->cap_buffer[3] = (UINT32)CaptureCtrl_VR.cap_buffer[3].phyaddr;
		swmode_infoptr->cap_buffer[4] = (UINT32)CaptureCtrl_VR.cap_buffer[4].phyaddr;
	}

	rtd_pr_vr_debug("[VR]SwModeEnable = %d\n",swmode_infoptr->SwModeEnable);
	rtd_pr_vr_debug("[VR]buffernumber = %d\n",swmode_infoptr->buffernumber);
	rtd_pr_vr_debug("[VR]cap_format = %d\n",swmode_infoptr->cap_format);
	rtd_pr_vr_debug("[VR]cap_width = %d\n",swmode_infoptr->cap_width);
	rtd_pr_vr_debug("[VR]cap_length = %d\n",swmode_infoptr->cap_length);
	rtd_pr_vr_debug("[VR]YbufferSize = %d\n",swmode_infoptr->YbufferSize);
	rtd_pr_vr_debug("[VR]cap_buffer[0] = 0x%x\n",swmode_infoptr->cap_buffer[0]);
	rtd_pr_vr_debug("[VR]cap_buffer[1] = 0x%x\n",swmode_infoptr->cap_buffer[1]);
	rtd_pr_vr_debug("[VR]cap_buffer[2] = 0x%x\n",swmode_infoptr->cap_buffer[2]);
	rtd_pr_vr_debug("[VR]cap_buffer[3] = 0x%x\n",swmode_infoptr->cap_buffer[3]);
	rtd_pr_vr_debug("[VR]cap_buffer[4] = 0x%x\n",swmode_infoptr->cap_buffer[4]);
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
		rtd_pr_vr_emerg("[VR][dc2h]ret=%d, SCALERIOC_SET_DC2H_SWMODE_ENABLE RPC fail !!!\n", ret);
		return FALSE;
	}
	return TRUE;
}

void drvif_vr_color_colorspaceyuv2rgbtransfer(DC2H_IN_SEL dc2h_in_sel)
{
	unsigned short *table_index = 0;
	
	dc2h_rgb2yuv_dc2h_rgb2yuv_ctrl_RBUS  dc2h_rgb2yuv_ctrl_reg;
	dc2h_rgb2yuv_dc2h_tab1_m11_m12_RBUS dc2h_tab1_m11_m12_reg;
	dc2h_rgb2yuv_dc2h_tab1_m13_m21_RBUS dc2h_tab1_m13_m21_reg;
	dc2h_rgb2yuv_dc2h_tab1_m22_m23_RBUS dc2h_tab1_m22_m23_reg;
	dc2h_rgb2yuv_dc2h_tab1_m31_m32_RBUS dc2h_tab1_m31_m32_reg;
	dc2h_rgb2yuv_dc2h_tab1_m33_y_gain_RBUS dc2h_tab1_m33_y_gain_reg;
	dc2h_rgb2yuv_dc2h_tab1_yo_RBUS dc2h_tab1_yo_reg;
	
	if((get_VR_Pixel_Format() == VR_CAP_NV12)||(get_VR_Pixel_Format() == VR_CAP_NV16)||(get_VR_Pixel_Format() == VR_CAP_NV21))
		table_index = tYUV2RGB_COEF_709_RGB_16_235;
	else
		table_index = tYUV2RGB_COEF_709_RGB_0_255;

	//main  all tab-1
	dc2h_tab1_m11_m12_reg.m11 = table_index [_RGB2YUV_m11];
	dc2h_tab1_m11_m12_reg.m12 = table_index [_RGB2YUV_m12];
	//printf("dc2h_tab1_m11_m12_reg.regValue = %x\n", dc2h_tab1_m11_m12_reg.regValue);
	rtd_outl(DC2H_RGB2YUV_DC2H_Tab1_M11_M12_reg, dc2h_tab1_m11_m12_reg.regValue);

	dc2h_tab1_m13_m21_reg.m21 = table_index [_RGB2YUV_m21];
	dc2h_tab1_m13_m21_reg.m13 = table_index [_RGB2YUV_m13];
	//printf("dc2h_tab1_m13_m21_reg.regValue = %x\n", dc2h_tab1_m13_m21_reg.regValue);
	rtd_outl(DC2H_RGB2YUV_DC2H_Tab1_M13_M21_reg, dc2h_tab1_m13_m21_reg.regValue);

	dc2h_tab1_m22_m23_reg.m22 = table_index [_RGB2YUV_m22];
	dc2h_tab1_m22_m23_reg.m23 = table_index [_RGB2YUV_m23];
	//printf("dc2h_tab1_m22_m23_reg.regValue = %x\n", dc2h_tab1_m22_m23_reg.regValue);
	rtd_outl(DC2H_RGB2YUV_DC2H_Tab1_M22_M23_reg, dc2h_tab1_m22_m23_reg.regValue);

	dc2h_tab1_m31_m32_reg.m31 = table_index [_RGB2YUV_m31];
	dc2h_tab1_m31_m32_reg.m32 = table_index [_RGB2YUV_m32];
	//printf("dc2h_tab1_m31_m32_reg.regValue = %x\n", dc2h_tab1_m31_m32_reg.regValue);
	rtd_outl(DC2H_RGB2YUV_DC2H_Tab1_M31_M32_reg, dc2h_tab1_m31_m32_reg.regValue);

	dc2h_tab1_m33_y_gain_reg.m33 = table_index [_RGB2YUV_m33];
	dc2h_tab1_m33_y_gain_reg.yo_gain= table_index [_RGB2YUV_Y_gain];
	//printf("dc2h_tab1_m33_y_gain_reg.regValue = %x\n", dc2h_tab1_m33_y_gain_reg.regValue);
	rtd_outl(DC2H_RGB2YUV_DC2H_Tab1_M33_Y_Gain_reg, dc2h_tab1_m33_y_gain_reg.regValue);

	dc2h_tab1_yo_reg.yo_even= table_index [_RGB2YUV_Yo_even];
	dc2h_tab1_yo_reg.yo_odd= table_index [_RGB2YUV_Yo_odd];
	//printf("dc2h_tab1_yo_reg.regValue = %x\n", dc2h_tab1_yo_reg.regValue);
	rtd_outl(DC2H_RGB2YUV_DC2H_Tab1_Yo_reg, dc2h_tab1_yo_reg.regValue);

	dc2h_rgb2yuv_ctrl_reg.regValue = rtd_inl(DC2H_RGB2YUV_DC2H_RGB2YUV_CTRL_reg);
	dc2h_rgb2yuv_ctrl_reg.sel_rgb= table_index [_RGB2YUV_sel_RGB];
	dc2h_rgb2yuv_ctrl_reg.set_r_in_offset= table_index [_RGB2YUV_set_Yin_offset];
	dc2h_rgb2yuv_ctrl_reg.set_uv_out_offset= table_index [_RGB2YUV_set_UV_out_offset];
	dc2h_rgb2yuv_ctrl_reg.sel_uv_off= table_index [_RGB2YUV_sel_UV_off];
	dc2h_rgb2yuv_ctrl_reg.matrix_bypass= table_index [_RGB2YUV_Matrix_bypass];
	dc2h_rgb2yuv_ctrl_reg.sel_y_gain= table_index [_RGB2YUV_Enable_Y_gain];
	dc2h_rgb2yuv_ctrl_reg.dc2h_in_sel = dc2h_in_sel;
	
	if((get_VR_Pixel_Format() == VR_CAP_NV12)||(get_VR_Pixel_Format() == VR_CAP_NV16)||(get_VR_Pixel_Format() == VR_CAP_NV21))
	{
		if((dc2h_in_sel == _MAIN_UZU) || (dc2h_in_sel == _TWO_SECOND_UZU) || (dc2h_in_sel == _THIRD_UZU))  //YUV format
		{
			dc2h_rgb2yuv_ctrl_reg.en_rgb2yuv = 0;
		}else{
			dc2h_rgb2yuv_ctrl_reg.en_rgb2yuv = 1; //Enable RGB to YUV conversion (YUV out)
		}
	} 
	else
	{
		if((dc2h_in_sel == _MAIN_UZU) || (dc2h_in_sel == _TWO_SECOND_UZU) || (dc2h_in_sel == _THIRD_UZU))
		{
			dc2h_rgb2yuv_ctrl_reg.en_rgb2yuv = 2; //Enable YUV to RGB conversion (GBR out)
		}else{
			dc2h_rgb2yuv_ctrl_reg.en_rgb2yuv = 0;
		}
	}
	rtd_outl(DC2H_RGB2YUV_DC2H_RGB2YUV_CTRL_reg, dc2h_rgb2yuv_ctrl_reg.regValue);

}

void drvif_vr_color_ultrazoom_set_dc2h_scale_down(DC2H_HANDLER *pdc2h_Vrhdl)
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
	if (pdc2h_Vrhdl->in_size.nWidth > pdc2h_Vrhdl->out_size.nWidth)
	{
		// o-------calculate scaledown ratio to select one of different bandwith filters.--------o
		if ( pdc2h_Vrhdl->out_size.nWidth == 0 ) {
		rtd_pr_vr_debug("output width = 0 !!!\n");
		SDRatio = 0;
		} else {
		SDRatio = (pdc2h_Vrhdl->in_size.nWidth*TMPMUL) / pdc2h_Vrhdl->out_size.nWidth;
		}

		//UltraZoom_Printf("CSW SDRatio number=%d\n",SDRatio);

		if(SDRatio <= ((TMPMUL*3)/2))  //<1.5 sharp, wider bw
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
	if (pdc2h_Vrhdl->in_size.nLength > pdc2h_Vrhdl->out_size.nLength)
	{
		// o-------calculate scaledown ratio to select one of different bandwith filters.--------o
		//jeffrey 961231
		if ( pdc2h_Vrhdl->out_size.nLength == 0 ) {
		SDRatio = 0;
		} else {
		SDRatio = (pdc2h_Vrhdl->in_size.nLength*TMPMUL) /pdc2h_Vrhdl->out_size.nLength;
		}

		//UltraZoom_Printf("CSW SDRatio number=%d\n",SDRatio);

		if(SDRatio <= ((TMPMUL*3)/2))  //<1.5 sharp, wider bw
			SDFilter = 2;
		else if(SDRatio <= (TMPMUL*2) && SDRatio > ((TMPMUL*3)/2) )  // Mid
			SDFilter = 1;
		else    // blurest, narrow bw
			SDFilter = 0;

		rtd_pr_vr_debug("filter number=%d\n",SDFilter);

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
	if (pdc2h_Vrhdl->in_size.nWidth > pdc2h_Vrhdl->out_size.nWidth)
	{    // H scale-down
		Hini = 0x80;//0xff;//0x78;
		dc2h_hsd_initial_value_reg.hor_ini = Hini;
		dc2h_hsd_initial_value_reg.hor_ini_int= 1;
		a = 5;

		if(pdc2h_Vrhdl->panorama)
		{// CSW+ 0960830 Non-linear SD
			S1 = (pdc2h_Vrhdl->out_size.nWidth) >> 2;
			S2 = (pdc2h_Vrhdl->out_size.nWidth) - S1*2;
			//=vv==old from TONY, if no problem, use this=vv==
			nFactor = (unsigned int)(((pdc2h_Vrhdl->in_size.nWidth)<<21) - ((unsigned int)Hini<<13));
			//=vv==Thur debugged=vv==
			//nFactor = ((((unsigned long long)(ptInSize->nWidth))*(ptOutSize->nWidth-1))<<21) / ptOutSize->nWidth;
			//===================
			nFactor = nFactor /((2*a+1)*S1 + (a+1)*S2 - a) * a;
			if(S1 != 0)
				D = nFactor / a / S1;
			nFactor = SHR((nFactor + 1), 1); //rounding
			D = SHR(D + 1, 1); //rounding
			rtd_pr_vr_debug("\nPANORAMA2 TEST 111\n");
			rtd_pr_vr_debug("nPANORAMA2 Factor = %d\n",nFactor);
			rtd_pr_vr_debug("PANORAMA2 S1=%d, S2=%d, D = %d\n",S1, S2, D);

			if(nFactor < 1048576) {
				rtd_pr_vr_debug("PANORAMA2 Can't do nonlinear SD \n");
				//nFactor = (unsigned int)(((ptInSize->nWidth)<<21) - ((unsigned int)Hini<<13)) / (ptOutSize->nWidth-1);
				if(pdc2h_Vrhdl->out_size.nWidth != 0)
					nFactor = (unsigned int)(((pdc2h_Vrhdl->in_size.nWidth)<<21)) / (pdc2h_Vrhdl->out_size.nWidth);
				nFactor = SHR(nFactor + 1, 1); //rounding
				rtd_pr_vr_debug("PANORAMA2 nFactor=%x, input Wid=%d, Out Wid=%d\n",nFactor, pdc2h_Vrhdl->in_size.nWidth, pdc2h_Vrhdl->out_size.nWidth);
				S1 = 0;
				S2 = pdc2h_Vrhdl->out_size.nWidth;
				D = 0;
			}
        	}else{
			if ( pdc2h_Vrhdl->out_size.nWidth == 0 ) {
				rtd_pr_vr_debug("output width = 0 !!!\n");
				nFactor = 0;
			}else if(pdc2h_Vrhdl->in_size.nWidth>4095){
				//nFactor = (unsigned int)((((pdc2h_Vrhdl->in_size.nWidth-1)<<19) / (pdc2h_Vrhdl->out_size.nWidth-1))<<1);
				nFactor = (unsigned int)((((pdc2h_Vrhdl->in_size.nWidth)<<19) / (pdc2h_Vrhdl->out_size.nWidth))<<1);
			}else if(pdc2h_Vrhdl->in_size.nWidth>2047){
				//nFactor = (unsigned int)(((pdc2h_Vrhdl->in_size.nWidth-1)<<20) / (pdc2h_Vrhdl->out_size.nWidth-1));
				nFactor = (unsigned int)(((pdc2h_Vrhdl->in_size.nWidth)<<20) / (pdc2h_Vrhdl->out_size.nWidth));
			}else {
				//nFactor = (unsigned int)(((pdc2h_Vrhdl->in_size.nWidth-1)<<21) / (pdc2h_Vrhdl->out_size.nWidth-1));
				nFactor = (unsigned int)(((pdc2h_Vrhdl->in_size.nWidth)<<21) / (pdc2h_Vrhdl->out_size.nWidth));
				nFactor = SHR(nFactor + 1, 1); //rounding
			}

			rtd_pr_vr_debug("nFactor=%d, input Wid=%d, Out Wid=%d\n",nFactor, pdc2h_Vrhdl->in_size.nWidth, pdc2h_Vrhdl->out_size.nWidth);
			S1 = 0;
			S2 = pdc2h_Vrhdl->out_size.nWidth;
			D = 0;
		}
	}else {
		nFactor = 0x100000;
		S1 = 0;
		S2 = pdc2h_Vrhdl->out_size.nWidth;
		D = 0;
	}

	dc2h_hsd_scale_hor_factor_reg.hor_fac = nFactor;
	//dc2h_hsd_hor_segment_reg.nl_seg1 = S1;
	dc2h_hsd_hor_segment_reg.nl_seg2 = S2;
	//dc2h_hsd_hor_delta1_reg.nl_d1 = D;
	//UltraZoom_Printf("reg_seg1=%x, reg_Seg_all=%x\n",dc2h_hsd_hor_segment_reg.nl_seg1, dc2h_hsd_hor_segment_reg.regValue);
	rtd_pr_vr_debug("nFactor=%x, input_Wid=%d, Output_Wid=%d\n",nFactor, pdc2h_Vrhdl->in_size.nWidth, pdc2h_Vrhdl->out_size.nWidth);

	//================V Scale-down=================
	if (pdc2h_Vrhdl->in_size.nLength > pdc2h_Vrhdl->out_size.nLength) {    // V scale-down
		Vini = 0xff;//0x78;
		dc2h_vsd_initial_value_reg.ver_ini = Vini;
		if ( pdc2h_Vrhdl->out_size.nLength == 0 ) {
			rtd_pr_vr_debug("output length = 0 !!!\n");
			nFactor = 0;
		} else {
			//frank@1018 sync with pacific to solve 3d vide 1920X2205 scaler down fail
			//nFactor = (unsigned int)(((ptInSize->nLength)<<21) / (ptOutSize->nLength));
			//nFactor = (unsigned int)(((unsigned long long)(pdc2h_Vrhdl->in_size.nLength)<<21) / (pdc2h_Vrhdl->out_size.nLength));
			tmpLength = ((unsigned long long)(pdc2h_Vrhdl->in_size.nLength)<<21);
			do_div(tmpLength, (pdc2h_Vrhdl->out_size.nLength));
			nFactor = tmpLength;
		}
		rtd_pr_vr_debug("Ch1 Ver: CAP=%d, Disp=%d, factor=%d\n", pdc2h_Vrhdl->in_size.nLength, pdc2h_Vrhdl->out_size.nLength, nFactor);
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


void drvif_vr_color_ultrazoom_config_dc2h_scaling_down(DC2H_HANDLER *pdc2h_Vrhdl)
{
	// Scale down setup for Channel1
	dc2h_scaledown_dc2h_hsd_ctrl0_RBUS dc2h_hsd_ctrl0_reg;
	dc2h_scaledown_dc2h_vsd_ctrl0_RBUS dc2h_vsd_ctrl0_reg;
	dc2h_scaledown_dc2h_444to422_ctrl_RBUS dc2h_scaledown_dc2h_444to422_ctrl_reg;
	
	dc2h_scaledown_dc2h_444to422_ctrl_reg.regValue = rtd_inl(DC2H_SCALEDOWN_DC2H_444to422_CTRL_reg);
	//if(pdc2h_Vrhdl->dc2h_in_sel==_MAIN_UZU)/*mux uzu*/
		//dc2h_scaledown_dc2h_444to422_ctrl_reg.cbcr_swap_en =1;
	//else
		dc2h_scaledown_dc2h_444to422_ctrl_reg.cbcr_swap_en =0;
	rtd_outl(DC2H_SCALEDOWN_DC2H_444to422_CTRL_reg, dc2h_scaledown_dc2h_444to422_ctrl_reg.regValue);  //used record freeze buffer
	

	dc2h_hsd_ctrl0_reg.regValue = rtd_inl(DC2H_SCALEDOWN_DC2H_HSD_Ctrl0_reg);
	dc2h_hsd_ctrl0_reg.h_zoom_en = (pdc2h_Vrhdl->in_size.nWidth > pdc2h_Vrhdl->out_size.nWidth);
	rtd_outl(DC2H_SCALEDOWN_DC2H_HSD_Ctrl0_reg, dc2h_hsd_ctrl0_reg.regValue);

	dc2h_vsd_ctrl0_reg.regValue = rtd_inl(DC2H_SCALEDOWN_DC2H_VSD_Ctrl0_reg);
	if(pdc2h_Vrhdl->in_size.nLength > pdc2h_Vrhdl->out_size.nLength){
		dc2h_vsd_ctrl0_reg.v_zoom_en = 1;
		dc2h_vsd_ctrl0_reg.buffer_mode = 2;
	}else{
		dc2h_vsd_ctrl0_reg.v_zoom_en = 0;
		dc2h_vsd_ctrl0_reg.buffer_mode = 0;
	}
	if((get_VR_Pixel_Format() == VR_CAP_NV12)||(get_VR_Pixel_Format() == VR_CAP_NV16)||(get_VR_Pixel_Format() == VR_CAP_NV21)){
		dc2h_vsd_ctrl0_reg.sort_fmt = 1;//mark2 alway 2pixel,no drop,cap use 444
	} else {
		dc2h_vsd_ctrl0_reg.sort_fmt = 1;
	}
	rtd_outl(DC2H_SCALEDOWN_DC2H_VSD_Ctrl0_reg, dc2h_vsd_ctrl0_reg.regValue);

	drvif_vr_color_ultrazoom_set_dc2h_scale_down(pdc2h_Vrhdl);
}

void drvif_vr_dc2h_input_overscan_config(unsigned int x, unsigned int y, unsigned int w, unsigned int h)
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

	//dc2h_rgb2yuv_dc2h_3dmaskto2d_ctrl_Reg.dc2h_3dmaskto2d_3dformat = 4;/*overscan*/
	dc2h_rgb2yuv_dc2h_3dmaskto2d_ctrl_Reg.dc2h_3dmaskto2d_en = 1;
	rtd_outl(DC2H_RGB2YUV_DC2H_3DMaskTo2D_Ctrl_reg, dc2h_rgb2yuv_dc2h_3dmaskto2d_ctrl_Reg.regValue);
	
}

void drvif_vr_vi_config(unsigned int nWidth, unsigned int nLength, unsigned int data_format)
{
#define LINE_STEP_MAX 0x1FFFFFFF

	unsigned int LineSize;
	unsigned int width_16bytes_align = 0;  //128bits align
//	unsigned int width96align;
    dc2h_vi_dc2h_v1_output_fmt_RBUS dc2h_v1_output_fmt_reg;
    dc2h_vi_dc2h_cwvh1_RBUS dc2h_cwvh1_reg;
    dc2h_vi_dc2h_cwvh1_2_RBUS dc2h_cwvh1_2_reg;
	dc2h_vi_dc2h_dmactl_RBUS vi_dmactl_reg;
	dc2h_vi_dc2h_vi_addcmd_transform_RBUS dc2h_vi_dc2h_vi_addcmd_transform_reg;
	dc2h_dma_dc2h_seq_interrupt_enable_RBUS dc2h_dma_dc2h_seq_interrupt_enable_Reg;
	dc2h_vi_dc2h_y1buf_RBUS dc2h_vi_dc2h_y1buf_Reg;
	dc2h_vi_dc2h_c1buf_RBUS dc2h_vi_dc2h_c1buf_Reg;
	dc2h_vi_dc2h_vi_c_line_step_RBUS dc2h_vi_dc2h_vi_c_line_step_Reg;
	
    dc2h_v1_output_fmt_reg.regValue = rtd_inl(DC2H_VI_DC2H_V1_OUTPUT_FMT_reg);
	dc2h_v1_output_fmt_reg.data_format = data_format;/*0:420	1:422*/
	if(get_VR_Pixel_Format() == VR_CAP_NV12 || get_VR_Pixel_Format() == VR_CAP_NV16)
	{
		dc2h_v1_output_fmt_reg.swap_1byte = 0;
		dc2h_v1_output_fmt_reg.swap_2byte = 0;
		dc2h_v1_output_fmt_reg.swap_4byte = 0;
		dc2h_v1_output_fmt_reg.swap_8byte = 0;

	}else if(get_VR_Pixel_Format() == VR_CAP_NV21){
		dc2h_v1_output_fmt_reg.swap_1byte = 1;
		dc2h_v1_output_fmt_reg.swap_2byte = 0;
		dc2h_v1_output_fmt_reg.swap_4byte = 0;
		dc2h_v1_output_fmt_reg.swap_8byte = 0;
	}
    rtd_outl(DC2H_VI_DC2H_V1_OUTPUT_FMT_reg, dc2h_v1_output_fmt_reg.regValue);
	
	width_16bytes_align = drvif_memory_get_data_align(nWidth,16);
	
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
	/*if((TRUE == force_enable_two_step_uzu()) && (vr_is_4K2K_capture() == TRUE))
		LineSize = VR_CAP_FRAME_WIDTH_4K2K * 8 / 64;
	else*/
	LineSize = width_16bytes_align * 8 / 64;
	if(LineSize % 2)
		LineSize += 1;
	
	if(Get_PANEL_VFLIP_ENABLE())
		LineSize = (LINE_STEP_MAX - LineSize + 1); //2's complement of line step,means nagative line step
	
	dc2h_vi_dc2h_vi_c_line_step_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_c_line_step_reg);
	dc2h_vi_dc2h_vi_c_line_step_Reg.c_line_step = LineSize;
	rtd_outl(DC2H_VI_DC2H_vi_c_line_step_reg, dc2h_vi_dc2h_vi_c_line_step_Reg.regValue);

	dc2h_vi_dc2h_vi_addcmd_transform_reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_addcmd_transform_reg);
	dc2h_vi_dc2h_vi_addcmd_transform_reg.line_step = LineSize;
	dc2h_vi_dc2h_vi_addcmd_transform_reg.vi_addcmd_trans_en = 1;
	rtd_outl(DC2H_VI_DC2H_vi_addcmd_transform_reg, dc2h_vi_dc2h_vi_addcmd_transform_reg.regValue);

	if (get_VrSwBufferMode()) {
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


	if (get_VrSwBufferMode()) {
		rtd_outl(DC2H_DMA_dc2h_seq_Interrupt_Enable_reg, dc2h_dma_dc2h_seq_interrupt_enable_Reg.regValue);
	}	
}

void drvif_vr_dc2h_seq_config(DC2H_HANDLER *pdc2h_Vrhdl)
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
	vi_dmactl_reg.seq_blk_sel = 0;
	rtd_outl(DC2H_VI_DC2H_DMACTL_reg, vi_dmactl_reg.regValue);

	dc2h_seq_mode_ctrl1_reg.regValue = rtd_inl(DC2H_DMA_dc2h_Seq_mode_CTRL1_reg);
	dc2h_seq_mode_ctrl1_reg.cap_en = 0;
	rtd_outl(DC2H_DMA_dc2h_Seq_mode_CTRL1_reg, dc2h_seq_mode_ctrl1_reg.regValue);

	if (pdc2h_Vrhdl->dc2h_en == FALSE)
		return;

	dc2h_seq_mode_ctrl2_reg.regValue = rtd_inl(DC2H_DMA_dc2h_Seq_mode_CTRL2_reg);
	dc2h_seq_mode_ctrl2_reg.hact = pdc2h_Vrhdl->out_size.nWidth;
	dc2h_seq_mode_ctrl2_reg.vact = pdc2h_Vrhdl->out_size.nLength;
	rtd_outl(DC2H_DMA_dc2h_Seq_mode_CTRL2_reg, dc2h_seq_mode_ctrl2_reg.regValue);

	//num = (pdc2h_Vrhdl->out_size.nWidth - CAP_REM) / CAP_LEN;
	//rem = ((pdc2h_Vrhdl->out_size.nWidth - CAP_REM) % CAP_LEN) + CAP_REM;

	if(get_VR_Pixel_Format() == VR_CAP_ARGB8888 || get_VR_Pixel_Format() == VR_CAP_RGBA8888 || get_VR_Pixel_Format() == VR_CAP_ABGR8888) {
		width96align = drvif_memory_get_data_align(pdc2h_Vrhdl->out_size.nWidth*4, 96);
	} else {
		width96align = drvif_memory_get_data_align(pdc2h_Vrhdl->out_size.nWidth*3, 96);
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
	if(get_VrBufferNum() == 1) {
		dc2h_cap_ctl0_reg.triple_buf_en = 0;
		dc2h_cap_ctl0_reg.auto_block_sel_en = 0;
		
	} else {
		if (get_VrSwBufferMode()) {
			dc2h_cap_ctl0_reg.triple_buf_en = 0;
			dc2h_cap_ctl0_reg.auto_block_sel_en = 0;
		} else {
			dc2h_cap_ctl0_reg.triple_buf_en = 0;
			dc2h_cap_ctl0_reg.auto_block_sel_en = 1;
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
	if(get_VR_Pixel_Format() == VR_CAP_ARGB8888)
		dc2h_seq_byte_channel_swap_reg.dma_channel_swap = 0;
	else
		dc2h_seq_byte_channel_swap_reg.dma_channel_swap = 1;
	rtd_outl(DC2H_DMA_dc2h_seq_byte_channel_swap_reg, dc2h_seq_byte_channel_swap_reg.regValue);

	dc2h_seq_mode_ctrl1_reg.regValue = rtd_inl(DC2H_DMA_dc2h_Seq_mode_CTRL1_reg);
	dc2h_seq_mode_ctrl1_reg.cap_en = 1;

	if((get_VR_Pixel_Format() == VR_CAP_ABGR8888) || (get_VR_Pixel_Format() == VR_CAP_ARGB8888) || (get_VR_Pixel_Format() == VR_CAP_RGBA8888)){
		dc2h_seq_mode_ctrl1_reg.argb_dummy_data = 0xFF;
		dc2h_seq_mode_ctrl1_reg.rgb_to_argb_en = 1;
		dc2h_seq_mode_ctrl1_reg.argb_swap = 1;
        if(get_VR_Pixel_Format() == VR_CAP_RGBA8888)
            dc2h_seq_mode_ctrl1_reg.argb_swap = 0;
	}
	else
		dc2h_seq_mode_ctrl1_reg.rgb_to_argb_en = 0;

	if (get_VrSwBufferMode()) {
		dc2h_dma_dc2h_seq_interrupt_enable_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_seq_Interrupt_Enable_reg);
		dc2h_dma_dc2h_seq_interrupt_enable_Reg.vs_rising_ie = 1;
	}
	dc2h_wait_porch();
	rtd_outl(DC2H_DMA_dc2h_Seq_mode_CTRL1_reg, dc2h_seq_mode_ctrl1_reg.regValue);
	if (get_VrSwBufferMode()) {
		rtd_outl(DC2H_DMA_dc2h_seq_Interrupt_Enable_reg, dc2h_dma_dc2h_seq_interrupt_enable_Reg.regValue);
	}
	return;
}

void set_DC2H_rgb2yuv_ctrl_reg_refer_dump_location_vr(KADP_VR_DUMP_LOCATION_TYPE_T in_position, SIZE *in_size, unsigned char *dc2h_in_sel)
{
    //ppoverlay_uzudtg_control1_RBUS ppoverlay_uzudtg_control1_reg;
    //dc2h_rgb2yuv_dc2h_rgb2yuv_ctrl_RBUS  dc2h_rgb2yuv_ctrl_reg;
    two_step_uzu_dm_two_step_sr_ctrl_RBUS two_step_uzu_dm_two_step_sr_ctrl_reg;
    ppoverlay_main_den_h_start_end_RBUS main_den_h_start_end_Reg;
    ppoverlay_main_den_v_start_end_RBUS main_den_v_start_end_Reg;
    ppoverlay_main_active_h_start_end_RBUS main_active_h_start_end_Reg;
    ppoverlay_main_active_v_start_end_RBUS main_active_v_start_end_Reg;

    main_den_h_start_end_Reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_DEN_H_Start_End_reg);
    main_den_v_start_end_Reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_DEN_V_Start_End_reg);
    main_active_h_start_end_Reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_Active_H_Start_End_reg);
    main_active_v_start_end_Reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_Active_V_Start_End_reg);
    //ppoverlay_uzudtg_control1_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_control1_reg);
    
    /*dc2h_rgb2yuv_ctrl_reg.regValue = IoReg_Read32(DC2H_RGB2YUV_DC2H_RGB2YUV_CTRL_reg); //9c00
    dc2h_rgb2yuv_ctrl_reg.dc2h_drop_en = 0; //reserved,mark2 dc2h always 2pixel
    rtd_outl(DC2H_RGB2YUV_DC2H_RGB2YUV_CTRL_reg, dc2h_rgb2yuv_ctrl_reg.regValue);*/
    
    if(in_position == KADP_VR_SCALER_OUTPUT)
    {
        /*========mark2 ecn=======*/
        if(Get_DISPLAY_PANEL_OLED_TYPE() == FALSE)
        {
            in_size->nWidth = (main_active_h_start_end_Reg.mh_act_end - main_active_h_start_end_Reg.mh_act_sta);            
            in_size->nLength = (main_active_v_start_end_Reg.mv_act_end - main_active_v_start_end_Reg.mv_act_sta);
        }
        else
        {
            if(get_panel_res() == PANEL_RES_8K)//orbit 8k 60
            {
                in_size->nWidth = VR_CAP_ORBIT_WIDTH_8K4K;
                in_size->nLength = VR_CAP_ORBIT_HEIGHT_8K4K;
            }
            else//orbit 4k 120
            {
                in_size->nWidth = VR_CAP_ORBIT_WIDTH_4K2K;
                in_size->nLength = VR_CAP_ORBIT_HEIGHT_4K2K;
            }
        }
        two_step_uzu_dm_two_step_sr_ctrl_reg.regValue = IoReg_Read32(TWO_STEP_UZU_DM_TWO_STEP_SR_Ctrl_reg);
        /*if(ppoverlay_uzudtg_control1_reg.third_uzu_mode)//3rd uzu on
        {
            *dc2h_in_sel = _THIRD_UZU;              
        }
        else if(two_step_uzu_dm_two_step_sr_ctrl_reg.h_zoom_en || two_step_uzu_dm_two_step_sr_ctrl_reg.v_zoom_en)//2nd uzu on
        {
            *dc2h_in_sel = _TWO_SECOND_UZU;
        }
        else
        {
            *dc2h_in_sel = _MAIN_UZU;
        }*/
        *dc2h_in_sel = _THIRD_UZU;      

        /*if((*dc2h_in_sel == _MAIN_UZU || *dc2h_in_sel == _TWO_SECOND_UZU) && (in_size->nWidth % 2))
        {
            in_size->nWidth = (in_size->nWidth - 1);
        }
        else */if((*dc2h_in_sel == _THIRD_UZU) && (in_size->nWidth % 2))
        {
            in_size->nWidth = (in_size->nWidth + 1);
        }

    }
    else if(in_position == KADP_VR_DISPLAY_OUTPUT)
    {
        in_size->nWidth = (main_den_h_start_end_Reg.mh_den_end - main_den_h_start_end_Reg.mh_den_sta);
        in_size->nLength = (main_den_v_start_end_Reg.mv_den_end - main_den_v_start_end_Reg.mv_den_sta);

        *dc2h_in_sel = _Memc_Mux_Input;
    }
    else if(in_position == KADP_VR_OSDVIDEO_OUTPUT)
    {
        in_size->nWidth = (main_den_h_start_end_Reg.mh_den_end - main_den_h_start_end_Reg.mh_den_sta);
        in_size->nLength = (main_den_v_start_end_Reg.mv_den_end - main_den_v_start_end_Reg.mv_den_sta);

        *dc2h_in_sel = _OSD_MIXER;  
    }   
    else
    {
        rtd_pr_vr_emerg("[error]invalid cap position line:%d\n",__LINE__);
    }
}

unsigned char vr_judge_dc2h_inout_size_valid(KADP_VR_DUMP_LOCATION_TYPE_T location, unsigned int capW, unsigned int capH)
{
	unsigned int dc2h_inputW = 0;
	unsigned int dc2h_inputH = 0;
	ppoverlay_uzudtg_orbit_main_ctrl0_RBUS ppoverlay_uzudtg_orbit_main_ctrl0_reg;
	ppoverlay_uzudtg_control1_RBUS ppoverlay_uzudtg_control1_reg;
	ppoverlay_uzudtg_orbit_main_ctrl0_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_orbit_main_ctrl0_reg);
	ppoverlay_uzudtg_control1_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_control1_reg);
	
	if(location == KADP_VR_SCALER_INPUT)
	{	
		rtd_pr_vr_notice("dc2h source capture for extension\n");
		return FALSE;
	}
	else if (location == KADP_VR_SCALER_OUTPUT)
	{
		ppoverlay_main_active_h_start_end_RBUS main_active_h_start_end_Reg;
		ppoverlay_main_active_v_start_end_RBUS main_active_v_start_end_Reg;
		main_active_h_start_end_Reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_Active_H_Start_End_reg);
		main_active_v_start_end_Reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_Active_V_Start_End_reg);
		
		dc2h_inputW = (main_active_h_start_end_Reg.mh_act_end - main_active_h_start_end_Reg.mh_act_sta);
		dc2h_inputH = (main_active_v_start_end_Reg.mv_act_end - main_active_v_start_end_Reg.mv_act_sta);	
	}
	else
	{
		ppoverlay_main_den_h_start_end_RBUS main_den_h_start_end_Reg;
		ppoverlay_main_den_v_start_end_RBUS main_den_v_start_end_Reg;
		main_den_h_start_end_Reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_DEN_H_Start_End_reg);
		main_den_v_start_end_Reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_DEN_V_Start_End_reg);
		
		dc2h_inputW = (main_den_h_start_end_Reg.mh_den_end - main_den_h_start_end_Reg.mh_den_sta);
		dc2h_inputH = (main_den_v_start_end_Reg.mv_den_end - main_den_v_start_end_Reg.mv_den_sta);
	}

	rtd_pr_vr_notice("dc2h input size(%d,%d)\n",dc2h_inputW, dc2h_inputH);
	if(dc2h_inputW < capW || dc2h_inputH < capH)
	{
		rtd_pr_vr_emerg("invalid,dc2h out size larger than input size\n");
		return FALSE;
	}
	if(ppoverlay_uzudtg_orbit_main_ctrl0_reg.orbit_m_overscan_en && location == KADP_VR_SCALER_OUTPUT)
	{
		/*constraints: when orbit on, capture uzu original video data, dc2h input 7808*4384, dc2h output max 3840*2160*/
		if(capW > VR_CAP_FRAME_WIDTH_4K2K || capH > VR_CAP_FRAME_HEIGHT_4K2K)
		{
			rtd_pr_vr_notice("orbit on,dc2h capture uzu,max output 4k\n");
			return FALSE;
		}
	}

	return TRUE;
	
}

void drvif_vr_dc2h_seqmode_conifg(unsigned char enable, int capWid, int capLen, int capSrc)
{   
    pdc2h_Vrhdl = &dc2h_Vrhdl;
    memset(pdc2h_Vrhdl, 0 , sizeof(DC2H_HANDLER));
    pdc2h_Vrhdl->out_size.nWidth = capWid;
    pdc2h_Vrhdl->out_size.nLength = capLen;
    pdc2h_Vrhdl->dc2h_en = enable;
    
    if((pdc2h_Vrhdl->out_size.nWidth) % 2)
    {   /*if dc2h uzd output is odd, uzd output repeat last point, dc2h dma pack even Width*/
        pdc2h_Vrhdl->out_size.nWidth = pdc2h_Vrhdl->out_size.nWidth + 1;
    }
        
    if(vr_judge_dc2h_inout_size_valid(capSrc,capWid,capLen) == FALSE)
    {
        rtd_pr_vr_emerg("%s,dc2h unsupport in size smaller than out size\n", __FUNCTION__);
        return;
    }
    else 
    {
        set_DC2H_rgb2yuv_ctrl_reg_refer_dump_location_vr(capSrc, &pdc2h_Vrhdl->in_size, &pdc2h_Vrhdl->dc2h_in_sel);
    }                       

    if (enable == TRUE) 
    {
        
        rtd_pr_vr_emerg("[Vt capinfo]vt cap sel=%d input w=%d; input h=%d;output w=%d,output h=%d! \n", pdc2h_Vrhdl->dc2h_in_sel,pdc2h_Vrhdl->in_size.nWidth, pdc2h_Vrhdl->in_size.nLength,pdc2h_Vrhdl->out_size.nWidth,pdc2h_Vrhdl->out_size.nLength);
        
        if(get_VrSwBufferMode())
        {
            if(FALSE == drvif_vr_dc2h_swmode_inforpc(TRUE,get_VrBufferNum(),capWid,capLen))
            {
                rtd_pr_vr_emerg("%s=%d RPC fail so not setting dc2h and return!!\n", __FUNCTION__, __LINE__);  
                return;
            }
        }
        /*overscan reserved*/
        //drvif_dc2h_input_overscan_config(0, 0, pdc2h_hdl->in_size.nWidth, pdc2h_hdl->in_size.nLength);
        drvif_vr_color_colorspaceyuv2rgbtransfer(pdc2h_Vrhdl->dc2h_in_sel);
        drvif_vr_color_ultrazoom_config_dc2h_scaling_down(pdc2h_Vrhdl);
        if((get_VR_Pixel_Format() == VR_CAP_NV12)||(get_VR_Pixel_Format() == VR_CAP_NV21))
            drvif_vr_vi_config(pdc2h_Vrhdl->out_size.nWidth,pdc2h_Vrhdl->out_size.nLength,0);
        else if (get_VR_Pixel_Format() == VR_CAP_NV16)
            drvif_vr_vi_config(pdc2h_Vrhdl->out_size.nWidth,pdc2h_Vrhdl->out_size.nLength,1);
        else
            drvif_vr_dc2h_seq_config(pdc2h_Vrhdl);
    } 
    else 
    {
        vr_reset_dc2h_hw_setting();  /* disable dc2h and reset dc2h */
        
        if (get_VrSwBufferMode())
        {
            if(FALSE == drvif_vr_dc2h_swmode_inforpc(FALSE,get_VrBufferNum(),capWid,capLen))
            {
                rtd_pr_vr_emerg("%s=%d RPC fail when close dc2h!!\n", __FUNCTION__, __LINE__); 
            }
        }       
    }
}

void vr_reset_dc2h_hw_setting(void)
{
	dc2h_dma_dc2h_seq_interrupt_enable_RBUS dc2h_dma_dc2h_seq_interrupt_enable_Reg;
	dc2h_vi_dc2h_dmactl_RBUS vi_dmactl_reg;
	dc2h_rgb2yuv_dc2h_3dmaskto2d_ctrl_RBUS dc2h_rgb2yuv_dc2h_3dmaskto2d_ctrl_Reg;
	dc2h_rgb2yuv_dc2h_rgb2yuv_ctrl_RBUS	dc2h_rgb2yuv_ctrl_reg;
	dc2h_scaledown_dc2h_hsd_ctrl0_RBUS dc2h_hsd_ctrl0_reg;
	dc2h_scaledown_dc2h_vsd_ctrl0_RBUS dc2h_vsd_ctrl0_reg;
	dc2h_dma_dc2h_seq_mode_ctrl1_RBUS dc2h_seq_mode_ctrl1_reg;

	if (get_VrSwBufferMode())
	{
		dc2h_dma_dc2h_seq_interrupt_enable_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_seq_Interrupt_Enable_reg);
		dc2h_dma_dc2h_seq_interrupt_enable_Reg.vs_rising_ie = 0;
		rtd_outl(DC2H_DMA_dc2h_seq_Interrupt_Enable_reg, dc2h_dma_dc2h_seq_interrupt_enable_Reg.regValue);
		msleep(20);
	}
	
	if ((get_VR_Pixel_Format() == VR_CAP_NV12)||(get_VR_Pixel_Format() == VR_CAP_NV16)||(get_VR_Pixel_Format() == VR_CAP_NV21))
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
	dc2h_seq_mode_ctrl1_reg.rgb_to_argb_en = 0;
	dc2h_seq_mode_ctrl1_reg.cap_en = 0;
	rtd_outl(DC2H_DMA_dc2h_Seq_mode_CTRL1_reg, dc2h_seq_mode_ctrl1_reg.regValue);
	
}

void set_vr_vfod_freezed(unsigned char boffon)
{
	rtd_pr_vr_debug("fun:%s,boffon=%d\n", __FUNCTION__, boffon);
	if (CaptureCtrl_VR.cap_buffer[0].phyaddr == 0) {
		rtd_pr_vr_emerg("[VR warning]vr no init, so no memory, return freeze action;%s=%d \n", __FUNCTION__, __LINE__);
		return;
	}
#if 0	
	if((get_VrBufferNum()==1)&&(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE)){
		rtd_pr_vr_emerg("[vr warning]%s=%d Vsc not at _MODE_STATE_ACTIVE status,return freeze action!\n", __FUNCTION__, __LINE__);
		return;			
	}
#endif

	if(((drvif_scaler_get_abnormal_dvs_long_flag()==TRUE)||(drvif_scaler_get_abnormal_dvs_short_flag()==TRUE))
		&& (get_VrBufferNum() == MAX_CAP_BUF_NUMS))
	{
		if(wait_dvs_stable_cnt != 0) /* dvs stable once */
		{
			abnormal_dvs_cnt = 0;
			wait_dvs_stable_cnt = 0;
			rtd_pr_vr_emerg("reset abnormal_dvs_cnt,wait_dvs_stable_cnt,because not wait dvs stable\n");
		}
		abnormal_dvs_cnt++;
		
		if(ABNORMAL_DVS_FIRST_HAPPEN == abnormal_dvs_cnt) /*happen abnormal_dvs stop dc2h */
		{
			rtd_pr_vr_emerg("[Warning VBE]%d Vbe at abnormal dvs status,could not do vr capture!\n", __LINE__);
			if(TRUE == vr_get_dc2h_capture_state()) /*dc2h already enable*/
			{
				vr_enable_dc2h(FALSE); /*stop dc2h,avoid memory trash,kernel panic KTASKWBS-13022*/
				rtd_pr_vr_emerg("stop dc2h done!\n");
			}
			return;
		}
		if(abnormal_dvs_cnt > MAX_CAP_BUF_NUMS)
		{
			vr_set_dqbuf_ioctl_fail_flag(TRUE);
			rtd_pr_vr_emerg("[Warning]dc2h has been disable because abnormal dvs\n");
			return;
		}
	}
	else
	{
		if(abnormal_dvs_cnt != 0) /* happen abnormal dvs before*/
		{
			wait_dvs_stable_cnt++; 
			if(wait_dvs_stable_cnt == WAIT_DVS_STABLE_COUNT) /* dvs wait stable,this function called twice in every dqubf,consecutive 4 times stable dvs, enable vr capture again */
			{
				abnormal_dvs_cnt = 0;
				wait_dvs_stable_cnt = 0;
				vr_set_dqbuf_ioctl_fail_flag(FALSE);
				if(FALSE == vr_get_dc2h_capture_state())
				{
					vr_enable_dc2h(TRUE); /* KTASKWBS-13909 dvs stable again, enable dc2h */
					rtd_pr_vr_emerg("enable dc2h done!\n");
				}
			}
		}
	}

	if(get_VrBufferNum() == 1) 
	{
		unsigned int timeoutcount = 0x4ffff;
		dc2h_vi_dc2h_vi_doublebuffer_RBUS dc2h_vi_doublebuffer_Reg;
		dc2h_vi_doublebuffer_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_doublebuffer_reg);
		dc2h_vi_doublebuffer_Reg.vi_db_en = 1; //enable double buffer
		rtd_outl(DC2H_VI_DC2H_vi_doublebuffer_reg, dc2h_vi_doublebuffer_Reg.regValue);
		if(boffon == TRUE) //freeze 
		{
			//dc2h_vi_doublebuffer_Reg.vi_db_en = 1;
			if((get_VR_Pixel_Format() == VR_CAP_NV12)||(get_VR_Pixel_Format() == VR_CAP_NV16)||(get_VR_Pixel_Format() == VR_CAP_NV21))
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
			dc2h_vi_doublebuffer_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_doublebuffer_reg);
			dc2h_vi_doublebuffer_Reg.vi_db_apply = 1;  // double buffer apply
			rtd_outl(DC2H_VI_DC2H_vi_doublebuffer_reg, dc2h_vi_doublebuffer_Reg.regValue);
			
			do{
				if(DC2H_VI_DC2H_vi_doublebuffer_get_vi_db_apply(rtd_inl(DC2H_VI_DC2H_vi_doublebuffer_reg)))
					timeoutcount--;
				else{
					break;
				}
								
			}while(timeoutcount);	

			if(timeoutcount == 0)
			{
				rtd_pr_vr_notice("fun:%s, freeze action timeout\n", __FUNCTION__);
			}
			IndexOfFreezedVideoFrameBufferVr = 0;
		}
		else
		{
			if((get_VR_Pixel_Format() == VR_CAP_NV12)||(get_VR_Pixel_Format() == VR_CAP_NV16)||(get_VR_Pixel_Format() == VR_CAP_NV21))
			{
				dc2h_vi_dc2h_dmactl_RBUS dc2h_vi_dc2h_dmactl_Reg;
				dc2h_vi_dc2h_dmactl_Reg.regValue = rtd_inl(DC2H_VI_DC2H_DMACTL_reg);
				dc2h_vi_dc2h_dmactl_Reg.dmaen1 = 1; /* enable dma with double buffer control*/ 
				rtd_outl(DC2H_VI_DC2H_DMACTL_reg,dc2h_vi_dc2h_dmactl_Reg.regValue);
			}
			else
			{				
				dc2h_dma_dc2h_seq_mode_ctrl1_RBUS dc2h_dma_dc2h_seq_mode_ctrl1_Reg;
				dc2h_dma_dc2h_seq_mode_ctrl1_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Seq_mode_CTRL1_reg);
				dc2h_dma_dc2h_seq_mode_ctrl1_Reg.cap_en = 1; 
				rtd_outl(DC2H_DMA_dc2h_Seq_mode_CTRL1_reg,dc2h_dma_dc2h_seq_mode_ctrl1_Reg.regValue);
			}
			dc2h_vi_doublebuffer_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_doublebuffer_reg);
			dc2h_vi_doublebuffer_Reg.vi_db_apply = 1;
			rtd_outl(DC2H_VI_DC2H_vi_doublebuffer_reg, dc2h_vi_doublebuffer_Reg.regValue);
			do{
				if(DC2H_VI_DC2H_vi_doublebuffer_get_vi_db_apply(rtd_inl(DC2H_VI_DC2H_vi_doublebuffer_reg)))
					timeoutcount--;
				else
					break;
							
			}while(timeoutcount);	

			if(timeoutcount == 0)
			{
				rtd_pr_vr_notice("fun:%s, unfreeze action timeout\n", __FUNCTION__);
			}
			/* wait one frame */
			timeoutcount = 0x4ffff;
			dc2h_vi_doublebuffer_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_doublebuffer_reg);
			dc2h_vi_doublebuffer_Reg.vi_db_apply = 1;
			rtd_outl(DC2H_VI_DC2H_vi_doublebuffer_reg, dc2h_vi_doublebuffer_Reg.regValue);
			do{
				if(DC2H_VI_DC2H_vi_doublebuffer_get_vi_db_apply(rtd_inl(DC2H_VI_DC2H_vi_doublebuffer_reg)))
					timeoutcount--;
				else
					break;
							
			}while(timeoutcount);	

			if(timeoutcount == 0)
			{
				rtd_pr_vr_notice("fun:%s, wait one frame timeout\n", __FUNCTION__);
			}
		}
		dc2h_vi_doublebuffer_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_doublebuffer_reg);
		dc2h_vi_doublebuffer_Reg.vi_db_en = 0; /* doubel buffer only work when buffer_num=1 */
		rtd_outl(DC2H_VI_DC2H_vi_doublebuffer_reg, dc2h_vi_doublebuffer_Reg.regValue);
		
	} else {
		if (boffon == TRUE) {
			if((get_VR_Pixel_Format() == VR_CAP_NV12)||(get_VR_Pixel_Format() == VR_CAP_NV16)||(get_VR_Pixel_Format() == VR_CAP_NV21)){
				if(get_VrBufferNum()==5) {
					dc2h_vi_dc2h_vi_ads_start_y_RBUS DC2H_VI_DC2H_vi_ads_start_y_Reg;
					dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_Reg;
					unsigned int lastcapbufferaddr = 0;
					unsigned int vflip_ystart_offset = (vr_cap_frame_max_width*vr_cap_frame_max_height - vr_cap_frame_max_width);
					DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_ads_start_y_reg);
					dc2h_cap_l3_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L3_Start_reg);

					if(Get_PANEL_VFLIP_ENABLE())
					{
						if(DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == (CaptureCtrl_VR.cap_buffer[0].phyaddr + vflip_ystart_offset)){
							lastcapbufferaddr = (CaptureCtrl_VR.cap_buffer[4].phyaddr + vflip_ystart_offset);
							IndexOfFreezedVideoFrameBufferVr = 4;
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == (CaptureCtrl_VR.cap_buffer[1].phyaddr + vflip_ystart_offset)){
							lastcapbufferaddr = (CaptureCtrl_VR.cap_buffer[0].phyaddr + vflip_ystart_offset);
							IndexOfFreezedVideoFrameBufferVr = 0;
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == (CaptureCtrl_VR.cap_buffer[2].phyaddr + vflip_ystart_offset)){
							lastcapbufferaddr = (CaptureCtrl_VR.cap_buffer[1].phyaddr + vflip_ystart_offset);
							IndexOfFreezedVideoFrameBufferVr = 1;
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == (CaptureCtrl_VR.cap_buffer[3].phyaddr + vflip_ystart_offset)){
							lastcapbufferaddr = (CaptureCtrl_VR.cap_buffer[2].phyaddr + vflip_ystart_offset);
							IndexOfFreezedVideoFrameBufferVr = 2;
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == (CaptureCtrl_VR.cap_buffer[4].phyaddr + vflip_ystart_offset)){
							lastcapbufferaddr = (CaptureCtrl_VR.cap_buffer[3].phyaddr + vflip_ystart_offset);
							IndexOfFreezedVideoFrameBufferVr = 3;
						}
					}
					else
					{
						if(DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VR.cap_buffer[0].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[4].phyaddr;
							IndexOfFreezedVideoFrameBufferVr = 4;
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VR.cap_buffer[1].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[0].phyaddr;
							IndexOfFreezedVideoFrameBufferVr = 0;
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VR.cap_buffer[2].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[1].phyaddr;
							IndexOfFreezedVideoFrameBufferVr = 1;
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VR.cap_buffer[3].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[2].phyaddr;
							IndexOfFreezedVideoFrameBufferVr = 2;
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VR.cap_buffer[4].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[3].phyaddr;
							IndexOfFreezedVideoFrameBufferVr = 3;
						}
					}
						
					/*  mark below code
					if(lastcapbufferaddr == dc2h_cap_l3_start_Reg.regValue){
						if(lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[0].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[4].phyaddr;
						}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[1].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[0].phyaddr;
						}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[2].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[1].phyaddr;
						}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[3].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[2].phyaddr;
						}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[4].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[3].phyaddr;
						}
					}
					
					if(lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[0].phyaddr){
						IndexOfFreezedVideoFrameBufferVr = 0;
					}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[1].phyaddr){
						IndexOfFreezedVideoFrameBufferVr = 1;
					}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[2].phyaddr){
						IndexOfFreezedVideoFrameBufferVr = 2;
					}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[3].phyaddr){
						IndexOfFreezedVideoFrameBufferVr = 3;
					}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[4].phyaddr){
						IndexOfFreezedVideoFrameBufferVr = 4;
					}  WOSQRTK-13247's fix return repeat index when input framerate slower than display framerate */

					rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
				} else if(get_VrBufferNum()==4) {
					dc2h_vi_dc2h_vi_ads_start_y_RBUS DC2H_VI_DC2H_vi_ads_start_y_Reg;
					dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_Reg;
					unsigned int lastcapbufferaddr = 0;
					unsigned int vflip_ystart_offset = (vr_cap_frame_max_width*vr_cap_frame_max_height - vr_cap_frame_max_width);
					DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_ads_start_y_reg);
					dc2h_cap_l3_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L3_Start_reg);
					
					if(Get_PANEL_VFLIP_ENABLE())
					{
						if(DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == (CaptureCtrl_VR.cap_buffer[0].phyaddr + vflip_ystart_offset)){
							lastcapbufferaddr = (CaptureCtrl_VR.cap_buffer[3].phyaddr + vflip_ystart_offset);
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == (CaptureCtrl_VR.cap_buffer[1].phyaddr + vflip_ystart_offset)){
							lastcapbufferaddr = (CaptureCtrl_VR.cap_buffer[0].phyaddr + vflip_ystart_offset);
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == (CaptureCtrl_VR.cap_buffer[2].phyaddr + vflip_ystart_offset)){
							lastcapbufferaddr = (CaptureCtrl_VR.cap_buffer[1].phyaddr + vflip_ystart_offset);
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == (CaptureCtrl_VR.cap_buffer[3].phyaddr + vflip_ystart_offset)){
							lastcapbufferaddr = (CaptureCtrl_VR.cap_buffer[2].phyaddr + vflip_ystart_offset);
						}

						if(lastcapbufferaddr == dc2h_cap_l3_start_Reg.regValue){
							if(lastcapbufferaddr == (CaptureCtrl_VR.cap_buffer[0].phyaddr + vflip_ystart_offset)){
								lastcapbufferaddr = (CaptureCtrl_VR.cap_buffer[3].phyaddr + vflip_ystart_offset);
							}else if (lastcapbufferaddr == (CaptureCtrl_VR.cap_buffer[1].phyaddr + vflip_ystart_offset)){
								lastcapbufferaddr = (CaptureCtrl_VR.cap_buffer[0].phyaddr + vflip_ystart_offset);
							}else if (lastcapbufferaddr == (CaptureCtrl_VR.cap_buffer[2].phyaddr + vflip_ystart_offset)){
								lastcapbufferaddr = (CaptureCtrl_VR.cap_buffer[1].phyaddr + vflip_ystart_offset);
							}else if (lastcapbufferaddr == (CaptureCtrl_VR.cap_buffer[3].phyaddr + vflip_ystart_offset)){
								lastcapbufferaddr = (CaptureCtrl_VR.cap_buffer[2].phyaddr + vflip_ystart_offset);
							}
						}

						if(lastcapbufferaddr == (CaptureCtrl_VR.cap_buffer[0].phyaddr + vflip_ystart_offset)){
							IndexOfFreezedVideoFrameBufferVr = 0;
						}else if (lastcapbufferaddr == (CaptureCtrl_VR.cap_buffer[1].phyaddr + vflip_ystart_offset)){
							IndexOfFreezedVideoFrameBufferVr = 1;
						}else if (lastcapbufferaddr == (CaptureCtrl_VR.cap_buffer[2].phyaddr + vflip_ystart_offset)){
							IndexOfFreezedVideoFrameBufferVr = 2;
						}else if (lastcapbufferaddr == (CaptureCtrl_VR.cap_buffer[3].phyaddr + vflip_ystart_offset)){
							IndexOfFreezedVideoFrameBufferVr = 3;
						}

					}
					else
					{
						if(DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VR.cap_buffer[0].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[3].phyaddr;
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VR.cap_buffer[1].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[0].phyaddr;
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VR.cap_buffer[2].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[1].phyaddr;
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VR.cap_buffer[3].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[2].phyaddr;
						}

						if(lastcapbufferaddr == dc2h_cap_l3_start_Reg.regValue){
							if(lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[0].phyaddr){
								lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[3].phyaddr;
							}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[1].phyaddr){
								lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[0].phyaddr;
							}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[2].phyaddr){
								lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[1].phyaddr;
							}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[3].phyaddr){
								lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[2].phyaddr;
							}
						}

						if(lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[0].phyaddr){
							IndexOfFreezedVideoFrameBufferVr = 0;
						}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[1].phyaddr){
							IndexOfFreezedVideoFrameBufferVr = 1;
						}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[2].phyaddr){
							IndexOfFreezedVideoFrameBufferVr = 2;
						}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[3].phyaddr){
							IndexOfFreezedVideoFrameBufferVr = 3;
						}
					}
				
					rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
				} else if(get_VrBufferNum()==3) {
					dc2h_vi_dc2h_vi_ads_start_y_RBUS DC2H_VI_DC2H_vi_ads_start_y_Reg;
					dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_Reg;
					unsigned int lastcapbufferaddr = 0;
					unsigned int vflip_ystart_offset = (vr_cap_frame_max_width*vr_cap_frame_max_height - vr_cap_frame_max_width);
					DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_ads_start_y_reg);
					dc2h_cap_l3_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L3_Start_reg);

		            if(Get_PANEL_VFLIP_ENABLE())
					{
						if(DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == (CaptureCtrl_VR.cap_buffer[0].phyaddr + vflip_ystart_offset)){
							lastcapbufferaddr = (CaptureCtrl_VR.cap_buffer[2].phyaddr + vflip_ystart_offset);
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == (CaptureCtrl_VR.cap_buffer[1].phyaddr + vflip_ystart_offset)){
							lastcapbufferaddr = (CaptureCtrl_VR.cap_buffer[0].phyaddr + vflip_ystart_offset);
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == (CaptureCtrl_VR.cap_buffer[2].phyaddr + vflip_ystart_offset)){
							lastcapbufferaddr = (CaptureCtrl_VR.cap_buffer[1].phyaddr + vflip_ystart_offset);
						}

						/*if(lastcapbufferaddr == dc2h_cap_l3_start_Reg.regValue){
							if(lastcapbufferaddr == (CaptureCtrl_VR.cap_buffer[0].phyaddr + vflip_ystart_offset)){
								lastcapbufferaddr = (CaptureCtrl_VR.cap_buffer[2].phyaddr + vflip_ystart_offset);
							}else if (lastcapbufferaddr == (CaptureCtrl_VR.cap_buffer[1].phyaddr + vflip_ystart_offset)){
								lastcapbufferaddr = (CaptureCtrl_VR.cap_buffer[0].phyaddr + vflip_ystart_offset);
							}else if (lastcapbufferaddr == (CaptureCtrl_VR.cap_buffer[2].phyaddr + vflip_ystart_offset)){
								lastcapbufferaddr = (CaptureCtrl_VR.cap_buffer[1].phyaddr + vflip_ystart_offset);
							}
						}WOSQRTK-13247's fix return repeat index when input framerate slower than display framerate */

						if(lastcapbufferaddr == (CaptureCtrl_VR.cap_buffer[0].phyaddr + vflip_ystart_offset)){
							IndexOfFreezedVideoFrameBufferVr = 0;
						}else if (lastcapbufferaddr == (CaptureCtrl_VR.cap_buffer[1].phyaddr + vflip_ystart_offset)){
							IndexOfFreezedVideoFrameBufferVr = 1;
						}else if (lastcapbufferaddr == (CaptureCtrl_VR.cap_buffer[2].phyaddr + vflip_ystart_offset)){
							IndexOfFreezedVideoFrameBufferVr = 2;
						}
					}
					else
					{
						if(DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VR.cap_buffer[0].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[2].phyaddr;
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VR.cap_buffer[1].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[0].phyaddr;
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VR.cap_buffer[2].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[1].phyaddr;
						}

						/*if(lastcapbufferaddr == dc2h_cap_l3_start_Reg.regValue){
							if(lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[0].phyaddr){
								lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[2].phyaddr;
							}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[1].phyaddr){
								lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[0].phyaddr;
							}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[2].phyaddr){
								lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[1].phyaddr;
							}
						}WOSQRTK-13247's fix return repeat index when input framerate slower than display framerate */

						if(lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[0].phyaddr){
							IndexOfFreezedVideoFrameBufferVr = 0;
						}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[1].phyaddr){
							IndexOfFreezedVideoFrameBufferVr = 1;
						}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[2].phyaddr){
							IndexOfFreezedVideoFrameBufferVr = 2;
						}
					}
		           
		           rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
		        }else if(get_VrBufferNum()==2) {
					dc2h_vi_dc2h_vi_ads_start_y_RBUS DC2H_VI_DC2H_vi_ads_start_y_Reg;
					dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_Reg;
					unsigned int lastcapbufferaddr = 0;
					unsigned int vflip_ystart_offset = (vr_cap_frame_max_width*vr_cap_frame_max_height - vr_cap_frame_max_width);
					DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_ads_start_y_reg);
					dc2h_cap_l3_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L3_Start_reg);

		            if(Get_PANEL_VFLIP_ENABLE())
					{
						if(DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == (CaptureCtrl_VR.cap_buffer[0].phyaddr + vflip_ystart_offset)){
							lastcapbufferaddr = (CaptureCtrl_VR.cap_buffer[1].phyaddr + vflip_ystart_offset);
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == (CaptureCtrl_VR.cap_buffer[1].phyaddr + vflip_ystart_offset)){
							lastcapbufferaddr = (CaptureCtrl_VR.cap_buffer[0].phyaddr + vflip_ystart_offset);
						}

						if(lastcapbufferaddr == (CaptureCtrl_VR.cap_buffer[0].phyaddr + vflip_ystart_offset)){
							IndexOfFreezedVideoFrameBufferVr = 0;
						}else if (lastcapbufferaddr == (CaptureCtrl_VR.cap_buffer[1].phyaddr + vflip_ystart_offset)){
							IndexOfFreezedVideoFrameBufferVr = 1;
						}
					}
					else
					{
						if(DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VR.cap_buffer[0].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[1].phyaddr;
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VR.cap_buffer[1].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[0].phyaddr;
						}

						if(lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[0].phyaddr){
							IndexOfFreezedVideoFrameBufferVr = 0;
						}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[1].phyaddr){
							IndexOfFreezedVideoFrameBufferVr = 1;
						}
					}
					
		           rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
		        }

			} else {
				if(get_VrBufferNum()==5) {
					dc2h_dma_dc2h_cap_l1_start_RBUS dc2h_cap_l1_start_Reg;
					dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_Reg;
					unsigned int lastcapbufferaddr = 0;
					dc2h_cap_l1_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L1_Start_db_reg);
					
					dc2h_cap_l3_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L3_Start_reg);
			
					if(dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VR.cap_buffer[0].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[4].phyaddr;
					}else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VR.cap_buffer[1].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[0].phyaddr;
					}else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VR.cap_buffer[2].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[1].phyaddr;
					}else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VR.cap_buffer[3].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[2].phyaddr;
					}else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VR.cap_buffer[4].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[3].phyaddr;
					}
			
					if(lastcapbufferaddr == dc2h_cap_l3_start_Reg.regValue){
						if(lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[0].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[4].phyaddr;
						}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[1].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[0].phyaddr;
						}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[2].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[1].phyaddr;
						}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[3].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[2].phyaddr;
						}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[4].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[3].phyaddr;
						}
					}
					if(lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[0].phyaddr){
						IndexOfFreezedVideoFrameBufferVr = 0;
					}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[1].phyaddr){
						IndexOfFreezedVideoFrameBufferVr = 1;
					}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[2].phyaddr){
						IndexOfFreezedVideoFrameBufferVr = 2;
					}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[3].phyaddr){
						IndexOfFreezedVideoFrameBufferVr = 3;
					}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[4].phyaddr){
						IndexOfFreezedVideoFrameBufferVr = 4;
					}
					rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
				} else if(get_VrBufferNum()==4) {
					dc2h_dma_dc2h_cap_l1_start_RBUS dc2h_cap_l1_start_Reg;
					dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_Reg;
					unsigned int lastcapbufferaddr = 0;

                    if(IndexWhenVrBlocked == VR_REPEAT_IDX_INVALID)
                    {
    					dc2h_cap_l1_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L1_Start_db_reg);

    					dc2h_cap_l3_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L3_Start_reg);

    					if(dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VR.cap_buffer[0].phyaddr){
    						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[3].phyaddr;
    					}else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VR.cap_buffer[1].phyaddr){
    						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[0].phyaddr;
    					}else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VR.cap_buffer[2].phyaddr){
    						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[1].phyaddr;
    					}else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VR.cap_buffer[3].phyaddr){
    						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[2].phyaddr;
    					}

    					if(lastcapbufferaddr == dc2h_cap_l3_start_Reg.regValue){
    						if(lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[0].phyaddr){
    							lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[3].phyaddr;
    						}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[1].phyaddr){
    							lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[0].phyaddr;
    						}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[2].phyaddr){
    							lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[1].phyaddr;
    						}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[3].phyaddr){
    							lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[2].phyaddr;
    						}
    					}
    					if(lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[0].phyaddr){
    						IndexOfFreezedVideoFrameBufferVr = 0;
    					}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[1].phyaddr){
    						IndexOfFreezedVideoFrameBufferVr = 1;
    					}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[2].phyaddr){
    						IndexOfFreezedVideoFrameBufferVr = 2;
    					}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[3].phyaddr){
    						IndexOfFreezedVideoFrameBufferVr = 3;
    					}
                   }
                   else
                   {
                        IndexOfFreezedVideoFrameBufferVr = IndexWhenVrBlocked;
                        rtd_pr_vr_debug("IndexWhenVrBlocked=%d\n",IndexWhenVrBlocked);
                        lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[IndexOfFreezedVideoFrameBufferVr].phyaddr;
                   }
				   rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
				} else if(get_VrBufferNum()==3) {
		           dc2h_dma_dc2h_cap_l1_start_RBUS dc2h_cap_l1_start_Reg;
		           dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_Reg;
		           unsigned int lastcapbufferaddr = 0;
		           dc2h_cap_l1_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L1_Start_db_reg);
		           
		           dc2h_cap_l3_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L3_Start_reg);

		           if(dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VR.cap_buffer[0].phyaddr){
		                   lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[2].phyaddr;
		           }else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VR.cap_buffer[1].phyaddr){
		                   lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[0].phyaddr;
		           }else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VR.cap_buffer[2].phyaddr){
		                   lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[1].phyaddr;
		           }

		           if(lastcapbufferaddr == dc2h_cap_l3_start_Reg.regValue){
		                   if(lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[0].phyaddr){
		                           lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[2].phyaddr;
		                   }else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[1].phyaddr){
		                           lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[0].phyaddr;
		                   }else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[2].phyaddr){
		                           lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[1].phyaddr;
		                   }
		           }
		           if(lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[0].phyaddr){
		                   IndexOfFreezedVideoFrameBufferVr = 0;
		           }else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[1].phyaddr){
		                   IndexOfFreezedVideoFrameBufferVr = 1;
		           }else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[2].phyaddr){
		                   IndexOfFreezedVideoFrameBufferVr = 2;
		           }       
		           rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
		        }else if(get_VrBufferNum()==2) {
		           dc2h_dma_dc2h_cap_l1_start_RBUS dc2h_cap_l1_start_Reg;
		           dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_Reg;
		           unsigned int lastcapbufferaddr = 0;
		           dc2h_cap_l1_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L1_Start_db_reg);
		           
		           dc2h_cap_l3_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L3_Start_reg);

		           if(dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VR.cap_buffer[0].phyaddr){
		                   lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[0].phyaddr;
		           }else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VR.cap_buffer[1].phyaddr){
		                   lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[1].phyaddr;
		           }
		           if(lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[0].phyaddr){
		                   IndexOfFreezedVideoFrameBufferVr = 0;
		           }else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[1].phyaddr){
		                   IndexOfFreezedVideoFrameBufferVr = 1;
		           }   
		           rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
		        }
				
			}
		}else{
			//rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,0);
		}
	}
}
unsigned char vr_is_4K2K_capture(void)
{
	if((vr_cap_frame_max_width == VR_CAP_FRAME_WIDTH_4K2K) && (vr_cap_frame_max_height == VR_CAP_FRAME_HEIGHT_4K2K))
		return TRUE;
	else
		return FALSE;
}

unsigned int vr_sort_boundary_addr_max_index(void)
{
	unsigned int i,index = 0;
	unsigned int bufnum = get_VrBufferNum();
	unsigned int max = CaptureCtrl_VR.cap_buffer[0].phyaddr;

    if(CaptureCtrl_VR.cap_buffer[0].phyaddr == 0 || bufnum == 1)
    {
        index = 0;
    }
    else
    {
        for( i = 1; i < bufnum; i++)
    	{
    		if(CaptureCtrl_VR.cap_buffer[i].phyaddr > max)
    		{
    			max = CaptureCtrl_VR.cap_buffer[i].phyaddr;
    			index = i;
    		}   		
    	}
    }
	
	return index;
}

unsigned int vr_sort_boundary_addr_min_index(void)
{
	unsigned int i,index = 0;
	unsigned int bufnum = get_VrBufferNum();
	unsigned int min = CaptureCtrl_VR.cap_buffer[0].phyaddr;
	
    if(CaptureCtrl_VR.cap_buffer[0].phyaddr == 0 || bufnum == 1)
    {
        index = 0;
    }
    else
    {
        for( i = 1; i < bufnum; i++)
    	{
    		if(CaptureCtrl_VR.cap_buffer[i].phyaddr < min)
    		{
    			min = CaptureCtrl_VR.cap_buffer[i].phyaddr;
    			index = i;
    		}
    	}
    }
    
	return index;
}

void vr_update_cap_buffer_size_by_AP(unsigned int usr_width, unsigned int usr_height)
{
	unsigned int width_16bytes_align = 0;
	width_16bytes_align = drvif_memory_get_data_align(usr_width,16);
	vr_cap_frame_max_width = width_16bytes_align;
	vr_cap_frame_max_height = usr_height;
	rtd_pr_vr_notice("cap buf size:(%d,%d)\n",vr_cap_frame_max_width, vr_cap_frame_max_height);
}

unsigned long vr_bufAddr_align(unsigned int val)
{
    unsigned long ret = 0;

#ifdef CONFIG_BW_96B_ALIGNED
    ret = drvif_memory_get_data_align(val, __12KPAGE); // LCM(96, 64, 4K),96b align for 3ddr
#else
    ret = drvif_memory_get_data_align(val, __4KPAGE); //64b align for gpu,4k align for mmap
    //ret = PAGE_ALIGN(ret); // 4k align for mmap
#endif

    return ret;
}

unsigned char Capture_BufferMemInit_VR(unsigned int buffernum)
{
    unsigned int size = 0;
	int i = 0;
	unsigned long allocatetotalsize = 0;
	unsigned long physicalstartaddree = 0;

	if((get_VR_Pixel_Format() == VR_CAP_ABGR8888) || (get_VR_Pixel_Format() == VR_CAP_ARGB8888) || (get_VR_Pixel_Format() == VR_CAP_RGBA8888))
		VR_CAPTURE_BUFFER_UNITSIZE = vr_bufAddr_align(vr_cap_frame_max_width*vr_cap_frame_max_height*4);
	else if((get_VR_Pixel_Format() == VR_CAP_NV12)||(get_VR_Pixel_Format() == VR_CAP_NV21))
		VR_CAPTURE_BUFFER_UNITSIZE = vr_bufAddr_align(vr_cap_frame_max_width*vr_cap_frame_max_height) + vr_bufAddr_align(vr_cap_frame_max_width*vr_cap_frame_max_height/2);
	else if(get_VR_Pixel_Format() == VR_CAP_NV16)
		VR_CAPTURE_BUFFER_UNITSIZE = vr_bufAddr_align(vr_cap_frame_max_width*vr_cap_frame_max_height) + vr_bufAddr_align(vr_cap_frame_max_width*vr_cap_frame_max_height);
	else
		VR_CAPTURE_BUFFER_UNITSIZE = vr_bufAddr_align(vr_cap_frame_max_width*vr_cap_frame_max_height*3);

	size = VR_CAPTURE_BUFFER_UNITSIZE;

	rtd_pr_vr_debug("[vr]size=0x%x\n",size);


	if(buffernum > MAX_CAP_BUF_NUMS)
	{
        rtd_pr_vr_emerg("[error] vr buffer nums over 5");
		return FALSE;
	}
	else
	{
        allocatetotalsize = size;
		for (i = 0; i < buffernum; i++)
		{
			if(CaptureCtrl_VR.cap_buffer[i].phyaddr == 0)
			{
				physicalstartaddree = pli_malloc(allocatetotalsize, GFP_DCU1_FIRST);
				if(physicalstartaddree == INVALID_VAL)
				{
					rtd_pr_vr_emerg("[error]:VR pli_malloc memory failed,index(%d)(%lx)\n", i,physicalstartaddree);
					return FALSE;
				}
				VrAllocatedBufferStartAdress[i] = physicalstartaddree;

				CaptureCtrl_VR.cap_buffer[i].cache = NULL;
				CaptureCtrl_VR.cap_buffer[i].phyaddr = vr_bufAddr_align(physicalstartaddree);
				CaptureCtrl_VR.cap_buffer[i].size = size;
				rtd_pr_vr_emerg("[VR memory allocate] buf[%d]=(%d KB), phy(%lx) \n", i, CaptureCtrl_VR.cap_buffer[i].size >> 10, (unsigned long)CaptureCtrl_VR.cap_buffer[i].phyaddr);
			}
		}
	}

	return TRUE;
}

void Capture_BufferMemDeInit_VR(unsigned int buffernum)
{
	int i;

	for (i = 0; i < buffernum; i++)
	{
		if(VrAllocatedBufferStartAdress[i])
		{
			pli_free(VrAllocatedBufferStartAdress[i]);
			VrAllocatedBufferStartAdress[i] = 0;
		}
	}

	for (i = 0; i < buffernum; i++)
	{
		if (CaptureCtrl_VR.cap_buffer[i].phyaddr)
		{
			CaptureCtrl_VR.cap_buffer[i].cache = NULL;
			CaptureCtrl_VR.cap_buffer[i].phyaddr = (unsigned long)NULL;
			CaptureCtrl_VR.cap_buffer[i].size = 0;
		}
	}

	rtd_pr_vr_debug("[memory free] Deinit done!\n");
}

void vr_set_DC2H_cap_boundaryaddr(unsigned int VrBufferNum)
{
	dc2h_dma_dc2h_cap_boundaryaddr1_RBUS dc2h_cap_boundaryaddr1_reg;  //up limit
	dc2h_dma_dc2h_cap_boundaryaddr2_RBUS dc2h_cap_boundaryaddr2_reg;  //low limit

	dc2h_dma_dc2h_cap_boundaryaddr3_RBUS dc2h_cap_boundaryaddr3_reg;
	dc2h_dma_dc2h_cap_boundaryaddr4_RBUS dc2h_cap_boundaryaddr4_reg;

	dc2h_dma_dc2h_cap_boundaryaddr5_RBUS dc2h_cap_boundaryaddr5_reg;
	dc2h_dma_dc2h_cap_boundaryaddr6_RBUS dc2h_cap_boundaryaddr6_reg;

	dc2h_dma_dc2h_cap_boundaryaddr7_RBUS dc2h_cap_boundaryaddr7_reg;
	dc2h_dma_dc2h_cap_boundaryaddr8_RBUS dc2h_cap_boundaryaddr8_reg;

	dc2h_dma_dc2h_cap_boundaryaddr9_RBUS dc2h_cap_boundaryaddr9_reg;
	dc2h_dma_dc2h_cap_boundaryaddr10_RBUS dc2h_cap_boundaryaddr10_reg;


	if(CaptureCtrl_VR.cap_buffer[0].phyaddr == 0)
	{
		rtd_pr_vr_emerg("[error]vr buffer addr invalid,%s=%d \n", __FUNCTION__, __LINE__);
	}
	else
	{
		if((get_VR_Pixel_Format() == VR_CAP_NV12) || (get_VR_Pixel_Format() == VR_CAP_NV16)||(get_VR_Pixel_Format() == VR_CAP_NV21)){
			if(5 == VrBufferNum)
			{
				dc2h_cap_boundaryaddr1_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[0].phyaddr +  (UINT32)CaptureCtrl_VR.cap_buffer[0].size;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr1_reg, dc2h_cap_boundaryaddr1_reg.regValue);
				dc2h_cap_boundaryaddr2_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[0].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr2_reg, dc2h_cap_boundaryaddr2_reg.regValue);
				
				dc2h_cap_boundaryaddr3_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[1].phyaddr +  (UINT32)CaptureCtrl_VR.cap_buffer[1].size;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr3_reg, dc2h_cap_boundaryaddr3_reg.regValue);
				dc2h_cap_boundaryaddr4_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[1].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr4_reg, dc2h_cap_boundaryaddr4_reg.regValue);
				
				dc2h_cap_boundaryaddr5_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[2].phyaddr +  (UINT32)CaptureCtrl_VR.cap_buffer[2].size;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr5_reg, dc2h_cap_boundaryaddr5_reg.regValue);
				dc2h_cap_boundaryaddr6_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[2].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr6_reg, dc2h_cap_boundaryaddr6_reg.regValue); 
				
				dc2h_cap_boundaryaddr7_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[3].phyaddr +  (UINT32)CaptureCtrl_VR.cap_buffer[3].size;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr7_reg, dc2h_cap_boundaryaddr7_reg.regValue);
				dc2h_cap_boundaryaddr8_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[3].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr8_reg, dc2h_cap_boundaryaddr8_reg.regValue);
				
				dc2h_cap_boundaryaddr9_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[4].phyaddr +  (UINT32)CaptureCtrl_VR.cap_buffer[4].size;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr9_reg, dc2h_cap_boundaryaddr9_reg.regValue);
				dc2h_cap_boundaryaddr10_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[4].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr10_reg, dc2h_cap_boundaryaddr10_reg.regValue);
			}
			else if(4 == VrBufferNum)
			{
				dc2h_cap_boundaryaddr1_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[0].phyaddr +  (UINT32)CaptureCtrl_VR.cap_buffer[0].size;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr1_reg, dc2h_cap_boundaryaddr1_reg.regValue);
				dc2h_cap_boundaryaddr2_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[0].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr2_reg, dc2h_cap_boundaryaddr2_reg.regValue);
				
				dc2h_cap_boundaryaddr3_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[1].phyaddr +  (UINT32)CaptureCtrl_VR.cap_buffer[1].size;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr3_reg, dc2h_cap_boundaryaddr3_reg.regValue);
				dc2h_cap_boundaryaddr4_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[1].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr4_reg, dc2h_cap_boundaryaddr4_reg.regValue);
				
				dc2h_cap_boundaryaddr5_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[2].phyaddr +  (UINT32)CaptureCtrl_VR.cap_buffer[2].size;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr5_reg, dc2h_cap_boundaryaddr5_reg.regValue);
				dc2h_cap_boundaryaddr6_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[2].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr6_reg, dc2h_cap_boundaryaddr6_reg.regValue); 
				
				dc2h_cap_boundaryaddr7_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[3].phyaddr +  (UINT32)CaptureCtrl_VR.cap_buffer[3].size;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr7_reg, dc2h_cap_boundaryaddr7_reg.regValue);
				dc2h_cap_boundaryaddr8_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[3].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr8_reg, dc2h_cap_boundaryaddr8_reg.regValue);
				
				dc2h_cap_boundaryaddr9_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[3].phyaddr +  (UINT32)CaptureCtrl_VR.cap_buffer[3].size;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr9_reg, dc2h_cap_boundaryaddr9_reg.regValue);
				dc2h_cap_boundaryaddr10_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[3].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr10_reg, dc2h_cap_boundaryaddr10_reg.regValue);
			}
			else if(3 == VrBufferNum)
			{
				dc2h_cap_boundaryaddr1_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[0].phyaddr +  (UINT32)CaptureCtrl_VR.cap_buffer[0].size;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr1_reg, dc2h_cap_boundaryaddr1_reg.regValue);
				dc2h_cap_boundaryaddr2_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[0].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr2_reg, dc2h_cap_boundaryaddr2_reg.regValue);
				
				dc2h_cap_boundaryaddr3_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[1].phyaddr +  (UINT32)CaptureCtrl_VR.cap_buffer[1].size;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr3_reg, dc2h_cap_boundaryaddr3_reg.regValue);
				dc2h_cap_boundaryaddr4_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[1].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr4_reg, dc2h_cap_boundaryaddr4_reg.regValue);
				
				dc2h_cap_boundaryaddr5_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[2].phyaddr +  (UINT32)CaptureCtrl_VR.cap_buffer[2].size;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr5_reg, dc2h_cap_boundaryaddr5_reg.regValue);
				dc2h_cap_boundaryaddr6_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[2].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr6_reg, dc2h_cap_boundaryaddr6_reg.regValue); 

				dc2h_cap_boundaryaddr7_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[2].phyaddr +  (UINT32)CaptureCtrl_VR.cap_buffer[2].size;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr7_reg, dc2h_cap_boundaryaddr7_reg.regValue);
				dc2h_cap_boundaryaddr8_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[2].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr8_reg, dc2h_cap_boundaryaddr8_reg.regValue);
				
				dc2h_cap_boundaryaddr9_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[2].phyaddr +  (UINT32)CaptureCtrl_VR.cap_buffer[2].size;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr9_reg, dc2h_cap_boundaryaddr9_reg.regValue);
				dc2h_cap_boundaryaddr10_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[2].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr10_reg, dc2h_cap_boundaryaddr10_reg.regValue);
				
			}
			else if(2 == VrBufferNum)
			{
				dc2h_cap_boundaryaddr1_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[0].phyaddr +  (UINT32)CaptureCtrl_VR.cap_buffer[0].size;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr1_reg, dc2h_cap_boundaryaddr1_reg.regValue);
				dc2h_cap_boundaryaddr2_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[0].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr2_reg, dc2h_cap_boundaryaddr2_reg.regValue);
				
				dc2h_cap_boundaryaddr3_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[1].phyaddr +  (UINT32)CaptureCtrl_VR.cap_buffer[1].size;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr3_reg, dc2h_cap_boundaryaddr3_reg.regValue);
				dc2h_cap_boundaryaddr4_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[1].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr4_reg, dc2h_cap_boundaryaddr4_reg.regValue);

				dc2h_cap_boundaryaddr5_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[1].phyaddr +  (UINT32)CaptureCtrl_VR.cap_buffer[1].size;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr5_reg, dc2h_cap_boundaryaddr5_reg.regValue);
				dc2h_cap_boundaryaddr6_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[1].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr6_reg, dc2h_cap_boundaryaddr6_reg.regValue); 
				
				dc2h_cap_boundaryaddr7_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[1].phyaddr +  (UINT32)CaptureCtrl_VR.cap_buffer[1].size;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr7_reg, dc2h_cap_boundaryaddr7_reg.regValue);
				dc2h_cap_boundaryaddr8_reg.regValue = (UINT32)CaptureCtrl_VR.cap_buffer[1].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr8_reg, dc2h_cap_boundaryaddr8_reg.regValue);
				
				dc2h_cap_boundaryaddr9_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[1].phyaddr +  (UINT32)CaptureCtrl_VR.cap_buffer[1].size;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr9_reg, dc2h_cap_boundaryaddr9_reg.regValue);
				dc2h_cap_boundaryaddr10_reg.regValue = (UINT32)CaptureCtrl_VR.cap_buffer[1].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr10_reg, dc2h_cap_boundaryaddr10_reg.regValue);
				
			}
			else //1 == VrBufferNum
			{
				dc2h_cap_boundaryaddr1_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[0].phyaddr +  (UINT32)CaptureCtrl_VR.cap_buffer[0].size;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr1_reg, dc2h_cap_boundaryaddr1_reg.regValue);
				dc2h_cap_boundaryaddr2_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[0].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr2_reg, dc2h_cap_boundaryaddr2_reg.regValue);

				dc2h_cap_boundaryaddr3_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[0].phyaddr +  (UINT32)CaptureCtrl_VR.cap_buffer[0].size;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr3_reg, dc2h_cap_boundaryaddr3_reg.regValue);
				dc2h_cap_boundaryaddr4_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[0].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr4_reg, dc2h_cap_boundaryaddr4_reg.regValue);

				dc2h_cap_boundaryaddr5_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[0].phyaddr +  (UINT32)CaptureCtrl_VR.cap_buffer[0].size;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr5_reg, dc2h_cap_boundaryaddr5_reg.regValue);
				dc2h_cap_boundaryaddr6_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[0].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr6_reg, dc2h_cap_boundaryaddr6_reg.regValue); 
				
				dc2h_cap_boundaryaddr7_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[0].phyaddr +  (UINT32)CaptureCtrl_VR.cap_buffer[0].size;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr7_reg, dc2h_cap_boundaryaddr7_reg.regValue);
				dc2h_cap_boundaryaddr8_reg.regValue = (UINT32)CaptureCtrl_VR.cap_buffer[0].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr8_reg, dc2h_cap_boundaryaddr8_reg.regValue);
				
				dc2h_cap_boundaryaddr9_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[0].phyaddr +  (UINT32)CaptureCtrl_VR.cap_buffer[0].size;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr9_reg, dc2h_cap_boundaryaddr9_reg.regValue);
				dc2h_cap_boundaryaddr10_reg.regValue = (UINT32)CaptureCtrl_VR.cap_buffer[0].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr10_reg, dc2h_cap_boundaryaddr10_reg.regValue);
			}
		}
		else
		{
			//Sequential capture just use 1st boundary address register
			dc2h_cap_boundaryaddr1_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[vr_sort_boundary_addr_max_index()].phyaddr +  (UINT32)CaptureCtrl_VR.cap_buffer[0].size;
			rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr1_reg, dc2h_cap_boundaryaddr1_reg.regValue);
			dc2h_cap_boundaryaddr2_reg.regValue =  (UINT32)CaptureCtrl_VR.cap_buffer[vr_sort_boundary_addr_min_index()].phyaddr;
			rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr2_reg, dc2h_cap_boundaryaddr2_reg.regValue);

		}
	}
}
void drvif_vr_set_DC2H_CaptureConfig(VR_CUR_CAPTURE_INFO capInfo)
{
	unsigned char waitbufferms = 80;
/*#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
	struct timespec64 timeout_s;
#else
	struct timespec timeout_s;
#endif*/

	if(capInfo.enable == TRUE){
		if (CaptureCtrl_VR.cap_buffer[0].phyaddr == 0) {
			rtd_pr_vr_emerg("[Vr warning]vr no init, so no memory, return setting;%s=%d \n", __FUNCTION__, __LINE__);
			return;
		}

		if ((capInfo.capWid > vr_cap_frame_max_width)||(capInfo.capLen > vr_cap_frame_max_height)) {
			rtd_pr_vr_emerg("[VR warning]drvif_vr_set_DC2H_CaptureConfig:capsize over buffer size,w=%d,output h=%d! \n", capInfo.capWid,capInfo.capLen);
			return;
		}

		if((get_VR_Pixel_Format() == VR_CAP_NV12)||(get_VR_Pixel_Format() == VR_CAP_NV16)||(get_VR_Pixel_Format() == VR_CAP_NV21))
		{		
			dc2h_vi_dc2h_vi_ads_start_y_RBUS DC2H_VI_DC2H_vi_ads_start_y_Reg;
			dc2h_vi_dc2h_vi_ads_start_c_RBUS dc2h_vi_dc2h_vi_ads_start_c_Reg;
			dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_reg;
		
			dc2h_cap_l3_start_reg.regValue= 0;
			rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg, dc2h_cap_l3_start_reg.regValue);  //used record freeze buffer

			if(Get_PANEL_VFLIP_ENABLE())
			{
				DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue = ((UINT32)CaptureCtrl_VR.cap_buffer[0].phyaddr + (vr_cap_frame_max_width*vr_cap_frame_max_height - vr_cap_frame_max_width));
				IoReg_Write32(DC2H_VI_DC2H_vi_ads_start_y_reg, DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue);

				if((get_VR_Pixel_Format() == VR_CAP_NV12) || (get_VR_Pixel_Format() == VR_CAP_NV21))
					dc2h_vi_dc2h_vi_ads_start_c_Reg.regValue = ((UINT32)CaptureCtrl_VR.cap_buffer[0].phyaddr + vr_bufAddr_align(vr_cap_frame_max_width*vr_cap_frame_max_height) + (vr_cap_frame_max_width*vr_cap_frame_max_height/2 - vr_cap_frame_max_width));
				else //NV16
					dc2h_vi_dc2h_vi_ads_start_c_Reg.regValue = ((UINT32)CaptureCtrl_VR.cap_buffer[0].phyaddr + vr_bufAddr_align(vr_cap_frame_max_width*vr_cap_frame_max_height) + (vr_cap_frame_max_width*vr_cap_frame_max_height - vr_cap_frame_max_width));
				IoReg_Write32(DC2H_VI_DC2H_vi_ads_start_c_reg, dc2h_vi_dc2h_vi_ads_start_c_Reg.regValue);
			}
			else
			{
				DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue = (UINT32)CaptureCtrl_VR.cap_buffer[0].phyaddr;
				IoReg_Write32(DC2H_VI_DC2H_vi_ads_start_y_reg, DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue);
			
				dc2h_vi_dc2h_vi_ads_start_c_Reg.regValue = (UINT32)CaptureCtrl_VR.cap_buffer[0].phyaddr + vr_bufAddr_align(vr_cap_frame_max_width*vr_cap_frame_max_height);
				IoReg_Write32(DC2H_VI_DC2H_vi_ads_start_c_reg, dc2h_vi_dc2h_vi_ads_start_c_Reg.regValue);
			}

			//set boundary address 
			vr_set_DC2H_cap_boundaryaddr(get_VrBufferNum());

		} 
		else 
		{
			dc2h_dma_dc2h_cap_l1_start_RBUS dc2h_cap_l1_start_reg;
			dc2h_dma_dc2h_cap_l2_start_RBUS dc2h_cap_l2_start_reg;
			dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_reg;

            dc2h_cap_l1_start_reg.regValue = (UINT32)CaptureCtrl_VR.cap_buffer[0].phyaddr;
            rtd_outl(DC2H_DMA_dc2h_Cap_L1_Start_reg, dc2h_cap_l1_start_reg.regValue);

            dc2h_cap_l2_start_reg.regValue = 0;
            rtd_outl(DC2H_DMA_dc2h_Cap_L2_Start_reg, dc2h_cap_l2_start_reg.regValue);

            dc2h_cap_l3_start_reg.regValue = 0;
            rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg, dc2h_cap_l3_start_reg.regValue);
            
            vr_set_DC2H_cap_boundaryaddr(get_VrBufferNum());		
		}
        
		drvif_vr_dc2h_seqmode_conifg(TRUE, capInfo.capWid, capInfo.capLen, capInfo.capSrc);

		if(get_VrBufferNum()==1){
			waitbufferms = 20;
		}

        msleep_interruptible(waitbufferms);
        
		if(get_VrBufferNum()==1) {
			if((drvif_scaler_get_abnormal_dvs_long_flag()==TRUE)||(drvif_scaler_get_abnormal_dvs_short_flag()==TRUE)){				
				rtd_pr_vr_emerg("[Warning VBE]%s=%d Vbe at abnormal dvs status,stop vr capture!\n", __FUNCTION__, __LINE__);
				/*abonrmal timing closed dc2h*/
				drvif_vr_dc2h_seqmode_conifg(FALSE, capInfo.capWid, capInfo.capLen, capInfo.capSrc);
			}
		}
	} else {
		if((get_VR_Pixel_Format() == VR_CAP_NV12)||(get_VR_Pixel_Format() == VR_CAP_NV16)||(get_VR_Pixel_Format() == VR_CAP_NV21)){		
			dc2h_scaledown_dc2h_444to422_ctrl_RBUS dc2h_scaledown_dc2h_444to422_ctrl_reg;
			
			dc2h_scaledown_dc2h_444to422_ctrl_reg.regValue = rtd_inl(DC2H_SCALEDOWN_DC2H_444to422_CTRL_reg);
			if(VRDumpLocation == KADP_VR_SCALER_OUTPUT)
				dc2h_scaledown_dc2h_444to422_ctrl_reg.cbcr_swap_en =0;
			rtd_outl(DC2H_SCALEDOWN_DC2H_444to422_CTRL_reg, dc2h_scaledown_dc2h_444to422_ctrl_reg.regValue);  //used record freeze buffer
		}
		drvif_vr_dc2h_seqmode_conifg(FALSE, capInfo.capWid, capInfo.capLen, capInfo.capSrc);
	}
}

unsigned char do_vr_reqbufs(unsigned int buf_cnt)
{	
	if(0 == buf_cnt) /* Release buffers by setting buf_cnt = 0*/
	{	
		if((vr_capture_location == KADP_VR_DISPLAY_OUTPUT) || (vr_capture_location == KADP_VR_SCALER_OUTPUT) 
			|| (vr_capture_location == KADP_VR_OSDVIDEO_OUTPUT))
		{			
			if(HAL_VR_Finalize() == FALSE)
			{			
				rtd_pr_vr_notice("%s,release buffers fail\n", __FUNCTION__);
				return FALSE; 
			}
			rtd_pr_vr_notice("reqbufs(0)for dc2h capture\n");
		}
		else
		{
			rtd_pr_vr_emerg("func:%s,unsupport capture_location\n", __FUNCTION__);
			return FALSE;
		}
		return TRUE;  
	}
	else
	{			
		down(&VR_Semaphore);
		
		if (Capture_BufferMemInit_VR(buf_cnt) == FALSE) 
		{
			rtd_pr_vr_emerg("init allocate memory fail;%s=%d \n", __FUNCTION__, __LINE__);
			up(&VR_Semaphore);
			return FALSE;
		}
		
		set_VrBufferNum(buf_cnt);
		if(1 == buf_cnt)
			set_VrSwBufferMode(FALSE);
		else
			set_VrSwBufferMode(TRUE);
		
		rtd_pr_vr_debug("reqbufs(%d) success\n", buf_cnt);
		up(&VR_Semaphore);
		return TRUE;
		
	}
}

unsigned char do_vr_streamoff(void)
{
	if(get_vr_function() == TRUE)
	{
		if((vr_capture_location == KADP_VR_DISPLAY_OUTPUT) || (vr_capture_location == KADP_VR_SCALER_OUTPUT) 
			|| (vr_capture_location == KADP_VR_OSDVIDEO_OUTPUT))
		{		
			if(vr_get_dc2h_capture_state() == TRUE)
			{
				vr_enable_dc2h(FALSE);
				rtd_pr_vr_notice("%s,dc2h capture", __FUNCTION__);
			}
			return TRUE;
		}
		else
		{
			rtd_pr_vr_emerg("func:%s,unsupport capture_location\n", __FUNCTION__);
			return FALSE;
		}
	}
	else
	{
		rtd_pr_vr_notice("can't do streamoff,%s=%d \n", __FUNCTION__, __LINE__);
		return FALSE;
	}
}

unsigned char do_vr_capture_streamon(void)
{
	ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;
	down(&VR_Semaphore);

	display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
	
	if (display_timing_ctrl1_reg.disp_en == 0)/*means d domain clock off, stop capture*/
	{
		rtd_pr_vr_emerg("[Warning]%s=%d D domain clock is off status,could not do vr capture!\n", __FUNCTION__, __LINE__);
		set_vr_function(TRUE);/* KTASKWBS-14031 if vr returns fail in stream on,AP will release memory,But vr finalize run condition is get_vr_function() == TRUE,so set_vr_function(TRUE)*/
		up(&VR_Semaphore);
		return FALSE;	
	}
	
	if(get_vr_function() == FALSE)
	{		
		set_vr_function(TRUE);
				
		if((vr_capture_location == KADP_VR_DISPLAY_OUTPUT) || (vr_capture_location == KADP_VR_SCALER_OUTPUT) 
			|| (vr_capture_location == KADP_VR_OSDVIDEO_OUTPUT))
		{		
			
			curCapInfo.enable =  1; //iMode;
			curCapInfo.capMode = VR_CAP_MODE_SEQ;
			
			curCapInfo.capSrc =  vr_capture_location;  //0:scaler output 1:display output 	
			curCapInfo.capWid = vr_capture_Width; //iWid;
			curCapInfo.capLen = vr_capture_Height;
			
			drvif_vr_set_DC2H_CaptureConfig(curCapInfo);
			
			VFODStateVR.bEnabled = TRUE;
			VFODStateVR.bAppliedPQ =TRUE;
			VFODStateVR.bFreezed = FALSE;
			VFODStateVR.framerateDivide = get_VR_framerateDivide();
			VRDumpLocation = KADP_VR_DISPLAY_OUTPUT;
			rtd_pr_vr_debug("%s Success\n", __FUNCTION__);

			up(&VR_Semaphore);
			return TRUE;
		}
		else
		{
			rtd_pr_vr_emerg("func:%s,unsupport capture_location\n", __FUNCTION__);
			up(&VR_Semaphore);
			return FALSE;
		}
	}
	else
	{
		rtd_pr_vr_notice("aready streamon,do nothing;%s=%d \n", __FUNCTION__, __LINE__);
		up(&VR_Semaphore);
		return TRUE;
	}
}

unsigned char HAL_VR_InitEx(unsigned int buffernum)
{
	ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;
	down(&VR_Semaphore);

	if(get_vt_function() == TRUE)
    {
        rtd_pr_vr_emerg("vt is working, don't support vr for recording\n");
        up(&VR_Semaphore);
        return FALSE;
    }

	if((buffernum<=0) || (buffernum>5)) {/*max support 5 buffer mode*/
		buffernum = 5;
	}

	rtd_pr_vr_emerg("%s=%d buffernum=%d\n", __FUNCTION__, __LINE__,buffernum);

	if ((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC)
		&&(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_STATE) ==  _MODE_STATE_ACTIVE)){
		if(get_vdec_securestatus()==TRUE){
			rtd_pr_vr_emerg("[Warning vdecsvp]%s=%d Vdec source security is TRUE,could not do vr capture!\n", __FUNCTION__, __LINE__);
			up(&VR_Semaphore);
			return FALSE;
		}
	}

	display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
	if (display_timing_ctrl1_reg.disp_en == 0)/*means d domain clock off, stop capture*/
	{
		rtd_pr_vr_emerg("[Warning VR]D domain clock is off status,could not do vr capture!\n");
		up(&VR_Semaphore);
		return FALSE;	
	}
#if 1
	if((buffernum == 1)&&(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE)){
		rtd_pr_vr_debug("[Warning vr]Vsc not at _MODE_STATE_ACTIVE status\n");
		//up(&VR_Semaphore);
		//return FALSE;			
	}
#endif

	if(((drvif_scaler_get_abnormal_dvs_long_flag()==TRUE)||(drvif_scaler_get_abnormal_dvs_short_flag()==TRUE))
		&& (get_VrBufferNum() == 5)){ 
		rtd_pr_vr_emerg("[Warning VBE] Vbe at abnormal dvs status,could not do vr capture!\n");
		up(&VR_Semaphore);
		return FALSE;			
	}
	
	if(get_vr_function() == FALSE) {		
		curCapInfo.enable = 1; //iMode;
		curCapInfo.capMode = VR_CAP_MODE_SEQ;
	
		curCapInfo.capSrc =  vr_capture_location;  //0:scaler output 1:display output		
		curCapInfo.capWid = vr_capture_Width; //iWid;
		curCapInfo.capLen = vr_capture_Height;
		
		if (Capture_BufferMemInit_VR(buffernum) == FALSE) {
			curCapInfo.enable = 0; //iMode;
			rtd_pr_vr_emerg("init allocate memory fail;%s=%d \n", __FUNCTION__, __LINE__);
			up(&VR_Semaphore);
			return FALSE;
		}
		set_vr_function(TRUE);
		set_VrBufferNum(buffernum);
		if(buffernum==1)
			set_VrSwBufferMode(FALSE);
		else
			set_VrSwBufferMode(TRUE);
		
		//drvif_vr_set_DC2H_CaptureConfig(curCapInfo);
		VFODStateVR.bEnabled = TRUE;
		VFODStateVR.bAppliedPQ = TRUE;
		VFODStateVR.bFreezed = FALSE;
		VFODStateVR.framerateDivide = get_VR_framerateDivide();
		//VRDumpLocation = KADP_VR_DISPLAY_OUTPUT;
		rtd_pr_vr_notice("init Success;%s=%d \n", __FUNCTION__, __LINE__);
		up(&VR_Semaphore);
		return TRUE;
	} else {
		rtd_pr_vr_notice("have inited,do nothing;%s=%d \n", __FUNCTION__, __LINE__);
		up(&VR_Semaphore);
		return TRUE;
	}
	
}

unsigned char HAL_VR_Init(void)
{
	rtd_pr_vr_emerg("%s=%d \n", __FUNCTION__, __LINE__);		

	if(HAL_VR_InitEx(5) == FALSE)
		return FALSE;
	else
		return TRUE;
}

unsigned char HAL_VR_Finalize(void)
{
	down(&VR_Semaphore);
	if(get_vr_function()==TRUE) {
		rtd_pr_vr_emerg("%s start\n", __FUNCTION__);
		if(vr_get_dc2h_capture_state() == TRUE)
		{
			curCapInfo.enable = 0; //iMode;
			drvif_vr_set_DC2H_CaptureConfig(curCapInfo);
		}
		
		Capture_BufferMemDeInit_VR(get_VrBufferNum());
		set_VrBufferNum(1);
		IndexOfFreezedVideoFrameBufferVr = 0;
		set_VrSwBufferMode(FALSE);
		VFODStateVR.bEnabled = FALSE;
		VFODStateVR.bAppliedPQ = TRUE;
		VFODStateVR.bFreezed = FALSE;
		VFODStateVR.framerateDivide = 1;
		VRDumpLocation = KADP_VR_MAX_LOCATION;
		set_vr_function(FALSE);
		vr_set_dqbuf_ioctl_fail_flag(FALSE);
		abnormal_dvs_cnt = 0;
		wait_dvs_stable_cnt = 0;
        set_vr_target_fps(0);
		rtd_pr_vr_emerg("%s done\n", __FUNCTION__);
		
		up(&VR_Semaphore);	
		return TRUE;
	} else {
		rtd_pr_vr_emerg("have finalized,do nothing;%s=%d \n", __FUNCTION__, __LINE__);
		up(&VR_Semaphore);
		return TRUE;
	}
}

unsigned char HAL_VR_GetDeviceCapability(KADP_VR_DEVICE_CAPABILITY_INFO_T *pDeviceCapabilityInfo)
{
	down(&VR_Semaphore);
	pDeviceCapabilityInfo->numOfVideoWindow = 1;
	up(&VR_Semaphore);
	return TRUE;
}

unsigned char HAL_VR_GetVideoFrameBufferCapability(KADP_VR_VIDEO_WINDOW_TYPE_T videoWindowID, KADP_VR_VIDEO_FRAME_BUFFER_CAPABILITY_INFO_T *pVideoFrameBufferCapabilityInfo)
{
	if(videoWindowID != KADP_VR_VIDEO_WINDOW_0) {
		rtd_pr_vr_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	down(&VR_Semaphore);
	pVideoFrameBufferCapabilityInfo ->numOfVideoFrameBuffer = get_VrBufferNum();

	if((get_VR_Pixel_Format() == VR_CAP_NV12)||(get_VR_Pixel_Format() == VR_CAP_NV16)||(get_VR_Pixel_Format() == VR_CAP_NV21)){	
		pVideoFrameBufferCapabilityInfo ->numOfPlane = KADP_VR_VIDEO_FRAME_BUFFER_PLANE_SEMI_PLANAR;
	} else {
		pVideoFrameBufferCapabilityInfo ->numOfPlane = KADP_VR_VIDEO_FRAME_BUFFER_PLANE_INTERLEAVED;//sequence buffer
	}
	up(&VR_Semaphore);
	return TRUE;
}

unsigned char HAL_VR_GetVideoFrameOutputDeviceCapability(KADP_VR_VIDEO_WINDOW_TYPE_T videoWindowID, KADP_VR_VIDEO_FRAME_OUTPUT_DEVICE_CAPABILITY_INFO_T *pVideoFrameOutputDeviceCapabilityInfo)
{
	if(videoWindowID != KADP_VR_VIDEO_WINDOW_0) {
		rtd_pr_vr_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	down(&VR_Semaphore);
	pVideoFrameOutputDeviceCapabilityInfo ->bSupportedControlEnabled = TRUE;
	pVideoFrameOutputDeviceCapabilityInfo ->bSupportedControlFreezed = TRUE;
	pVideoFrameOutputDeviceCapabilityInfo ->bSupportedControlFramerateDivide = FALSE;
	pVideoFrameOutputDeviceCapabilityInfo ->bSupportedControlPQ = FALSE;
	up(&VR_Semaphore);
	return TRUE;
}

unsigned char HAL_VR_GetVideoFrameOutputDeviceLimitation(KADP_VR_VIDEO_WINDOW_TYPE_T videoWindowID, KADP_VR_VIDEO_FRAME_OUTPUT_DEVICE_LIMITATION_INFO_T *pVideoFrameOutputDeviceLimitationInfo)
{
    if(videoWindowID != KADP_VR_VIDEO_WINDOW_0) {
		rtd_pr_vr_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	down(&VR_Semaphore);
	pVideoFrameOutputDeviceLimitationInfo->maxResolution = (KADP_VR_RECT_T){0, 0, (Get_DISP_ACT_END_HPOS() - Get_DISP_ACT_STA_HPOS()), (Get_DISP_ACT_END_VPOS() - Get_DISP_ACT_STA_VPOS())};
	pVideoFrameOutputDeviceLimitationInfo->bLeftTopAlign = FALSE;
	pVideoFrameOutputDeviceLimitationInfo->bSupportInputVideoDeInterlacing = FALSE;
	pVideoFrameOutputDeviceLimitationInfo->bSupportDisplayVideoDeInterlacing = FALSE;
	pVideoFrameOutputDeviceLimitationInfo->bSupportScaleUp = FALSE;
	pVideoFrameOutputDeviceLimitationInfo->scaleUpLimitWidth = 0;
	pVideoFrameOutputDeviceLimitationInfo->scaleUpLimitHeight = 0;
	pVideoFrameOutputDeviceLimitationInfo->bSupportScaleDown = TRUE;
	pVideoFrameOutputDeviceLimitationInfo->scaleDownLimitWidth = 60;//nfactor upto 32
	pVideoFrameOutputDeviceLimitationInfo->scaleDownLimitHeight = 34;
	up(&VR_Semaphore);
	return TRUE;
}

unsigned char HAL_VR_GetAllVideoFrameBufferProperty(KADP_VR_VIDEO_WINDOW_TYPE_T videoWindowID, KADP_VR_VIDEO_FRAME_BUFFER_PROPERTY_INFO_T *pVideoFrameBufferProperty)
{
	if(videoWindowID != KADP_VR_VIDEO_WINDOW_0) {
		rtd_pr_vr_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	down(&VR_Semaphore);
	if(get_vr_function() == TRUE) {

		if(get_VR_Pixel_Format() == VR_CAP_NV16){	
			pVideoFrameBufferProperty->pixelFormat = KADP_VR_VIDEO_FRAME_BUFFER_PIXEL_FORMAT_YUV422_SEMI_PLANAR;
			pVideoFrameBufferProperty->stride = drvif_memory_get_data_align(curCapInfo.capWid, 96);
			pVideoFrameBufferProperty->width = curCapInfo.capWid;		
		}else if((get_VR_Pixel_Format() == VR_CAP_NV12)||(get_VR_Pixel_Format() == VR_CAP_NV21)){ 
			pVideoFrameBufferProperty->pixelFormat = KADP_VR_VIDEO_FRAME_BUFFER_PIXEL_FORMAT_YUV420_SEMI_PLANAR;
			pVideoFrameBufferProperty->stride = drvif_memory_get_data_align(curCapInfo.capWid, 96);
			pVideoFrameBufferProperty->width = curCapInfo.capWid;		
		}else if((get_VR_Pixel_Format() == VR_CAP_ABGR8888) || (get_VR_Pixel_Format() == VR_CAP_ARGB8888) || (get_VR_Pixel_Format() == VR_CAP_RGBA8888)) {
			pVideoFrameBufferProperty->pixelFormat = KADP_VR_VIDEO_FRAME_BUFFER_PIXEL_FORMAT_ARGB;
			pVideoFrameBufferProperty->stride = drvif_memory_get_data_align(curCapInfo.capWid*4, 96);
			pVideoFrameBufferProperty->width = curCapInfo.capWid*4;
		} else {
			pVideoFrameBufferProperty->pixelFormat = KADP_VR_VIDEO_FRAME_BUFFER_PIXEL_FORMAT_RGB;
			pVideoFrameBufferProperty->stride = drvif_memory_get_data_align(curCapInfo.capWid*3, 96);
			pVideoFrameBufferProperty->width = curCapInfo.capWid*3;
		}

		pVideoFrameBufferProperty->height = curCapInfo.capLen;
		pVideoFrameBufferProperty->ppPhysicalAddress0 = CaptureCtrl_VR.cap_buffer[0].phyaddr;
		pVideoFrameBufferProperty->ppPhysicalAddress1 = CaptureCtrl_VR.cap_buffer[1].phyaddr;
		pVideoFrameBufferProperty->ppPhysicalAddress2 = CaptureCtrl_VR.cap_buffer[2].phyaddr;
		pVideoFrameBufferProperty->ppPhysicalAddress3 = CaptureCtrl_VR.cap_buffer[3].phyaddr;
		pVideoFrameBufferProperty->ppPhysicalAddress4 = CaptureCtrl_VR.cap_buffer[4].phyaddr;
		pVideoFrameBufferProperty->vfbbuffernumber = get_VrBufferNum();
		up(&VR_Semaphore);
		return TRUE;
	} else {
		rtd_pr_vr_notice("not Inited So return False;%s=%d\n", __FUNCTION__, __LINE__);
		up(&VR_Semaphore);
		return FALSE;
	}
}

unsigned char HAL_VR_GetVideoFrameBufferIndex(KADP_VR_VIDEO_WINDOW_TYPE_T videoWindowID, unsigned int *pIndexOfCurrentVideoFrameBuffer)
{
	if(videoWindowID != KADP_VR_VIDEO_WINDOW_0) {
		rtd_pr_vr_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	down(&VR_Semaphore);
	
	if(get_vr_function() == TRUE) 
	{
		if(VFODStateVR.bFreezed == TRUE) {
			set_vr_vfod_freezed(FALSE);
			VFODStateVR.bFreezed = FALSE;
		}
		
		if(VFODStateVR.bFreezed == FALSE) {
			set_vr_vfod_freezed(TRUE);
			VFODStateVR.bFreezed = TRUE;
		}
		
		if(VFODStateVR.bFreezed == TRUE) 
		{
			if(TRUE == vr_get_dqbuf_ioctl_fail_flag())
			{
				up(&VR_Semaphore);
				return FALSE;
			}
			
			*pIndexOfCurrentVideoFrameBuffer = IndexOfFreezedVideoFrameBufferVr;

			rtd_pr_vr_debug("dq_idx=%d\n", IndexOfFreezedVideoFrameBufferVr);
		}
	    else  // if VFOD is not freezed,just return the last captured buffer index
		{
			//rtd_pr_vr_debug("VFOD is not freezed\n");

			if((get_VR_Pixel_Format() == VR_CAP_NV12)||(get_VR_Pixel_Format() == VR_CAP_NV16)||(get_VR_Pixel_Format() == VR_CAP_NV21))
			{
				if(get_VrBufferNum()==5) 
				{
					dc2h_vi_dc2h_vi_ads_start_y_RBUS DC2H_VI_DC2H_vi_ads_start_y_Reg;
					unsigned int lastcapbufferaddr = 0;
					DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_ads_start_y_reg);
			
					if(DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VR.cap_buffer[0].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[4].phyaddr;
					}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VR.cap_buffer[1].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[0].phyaddr;
					}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VR.cap_buffer[2].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[1].phyaddr;
					}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VR.cap_buffer[3].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[2].phyaddr;
					}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VR.cap_buffer[4].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[3].phyaddr;
					}
								
					if(lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[0].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 0;
					}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[1].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 1;
					}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[2].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 2;
					}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[3].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 3;
					}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[4].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 4;
					}
					rtd_pr_vr_debug("++--index = %d\n", *pIndexOfCurrentVideoFrameBuffer);
					//rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
				} 
				else if(get_VrBufferNum()==4) 
				{
					dc2h_vi_dc2h_vi_ads_start_y_RBUS DC2H_VI_DC2H_vi_ads_start_y_Reg;
					unsigned int lastcapbufferaddr = 0;
					DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_ads_start_y_reg);

			
					if(DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VR.cap_buffer[0].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[3].phyaddr;
					}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VR.cap_buffer[1].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[0].phyaddr;
					}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VR.cap_buffer[2].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[1].phyaddr;
					}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VR.cap_buffer[3].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[2].phyaddr;
					}
			
					if(lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[0].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 0;
					}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[1].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 1;
					}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[2].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 2;
					}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[3].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 3;
					}	
					//rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
				} 
				else if(get_VrBufferNum()==3)
				{
					dc2h_vi_dc2h_vi_ads_start_y_RBUS DC2H_VI_DC2H_vi_ads_start_y_Reg;
					unsigned int lastcapbufferaddr = 0;
					DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_ads_start_y_reg);


		           if(DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VR.cap_buffer[0].phyaddr){
		                   lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[2].phyaddr;
		           }else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VR.cap_buffer[1].phyaddr){
		                   lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[0].phyaddr;
		           }else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VR.cap_buffer[2].phyaddr){
		                   lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[1].phyaddr;
		           }

		           if(lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[0].phyaddr){
		                   *pIndexOfCurrentVideoFrameBuffer = 0;
		           }else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[1].phyaddr){
		                   *pIndexOfCurrentVideoFrameBuffer = 1;
		           }else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[2].phyaddr){
		                   *pIndexOfCurrentVideoFrameBuffer = 2;
		           }       
		          // rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
		        }
				else if(get_VrBufferNum()==2) 
				{
					dc2h_vi_dc2h_vi_ads_start_y_RBUS DC2H_VI_DC2H_vi_ads_start_y_Reg;
					unsigned int lastcapbufferaddr = 0;
					DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_ads_start_y_reg);

		           if(DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VR.cap_buffer[0].phyaddr){
		                   lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[0].phyaddr;
		           }else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VR.cap_buffer[1].phyaddr){
		                   lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[1].phyaddr;
		           }

		           if(lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[0].phyaddr){
		                   *pIndexOfCurrentVideoFrameBuffer = 0;
		           }else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[1].phyaddr){
		                   *pIndexOfCurrentVideoFrameBuffer = 1;
		           }    
		           //rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
		        }

			}
			else
			{	
				if (get_VrBufferNum() == 5)
				{
					dc2h_dma_dc2h_cap_l1_start_RBUS dc2h_cap_l1_start_Reg;
					unsigned int lastcapbufferaddr = 0;
					dc2h_cap_l1_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L1_Start_db_reg);
				
					//dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_Reg;
					//dc2h_cap_l3_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L3_Start_reg);

					if(dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VR.cap_buffer[0].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[4].phyaddr;
					}else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VR.cap_buffer[1].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[0].phyaddr;
					}else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VR.cap_buffer[2].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[1].phyaddr;
					}else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VR.cap_buffer[3].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[2].phyaddr;
					}else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VR.cap_buffer[4].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[3].phyaddr;
					}
					
					if(lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[0].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 0;
					}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[1].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 1;
					}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[2].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 2;
					}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[3].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 3;
					}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[4].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 4;
					}	
				}
				 else if(get_VrBufferNum() == 4)
				 {
					dc2h_dma_dc2h_cap_l1_start_RBUS dc2h_cap_l1_start_Reg;
					unsigned int lastcapbufferaddr = 0;
					dc2h_cap_l1_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L1_Start_db_reg);
							
					if(dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VR.cap_buffer[0].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[3].phyaddr;
					}else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VR.cap_buffer[1].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[0].phyaddr;
					}else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VR.cap_buffer[2].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[1].phyaddr;
					}else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VR.cap_buffer[3].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[2].phyaddr;
					}

					if(lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[0].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 0;
					}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[1].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 1;
					}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[2].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 2;
					}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[3].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 3;
					}
				}
				else if(get_VrBufferNum() == 3)
				{
	               dc2h_dma_dc2h_cap_l1_start_RBUS dc2h_cap_l1_start_Reg;
	               unsigned int lastcapbufferaddr = 0;
	               dc2h_cap_l1_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L1_Start_db_reg);

	               if(dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VR.cap_buffer[0].phyaddr){
	                       lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[2].phyaddr;
	              }else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VR.cap_buffer[1].phyaddr){
	                       lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[0].phyaddr;
	               }else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VR.cap_buffer[2].phyaddr){
	                       lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[1].phyaddr;
	               }

	               if(lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[0].phyaddr){
	                       *pIndexOfCurrentVideoFrameBuffer = 0;
	               }else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[1].phyaddr){
	                       *pIndexOfCurrentVideoFrameBuffer = 1;
	               }else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[2].phyaddr){
	                       *pIndexOfCurrentVideoFrameBuffer = 2;
	               }
	       		}
					
			}
				
		}
		up(&VR_Semaphore);
		return TRUE;
	} 
	else 
	{
		rtd_pr_vr_notice("is not Inited So return False;%s=%d\n", __FUNCTION__, __LINE__);
		up(&VR_Semaphore);
		return FALSE;
	}
}

unsigned char do_vr_dqbuf(unsigned int *pdqbuf_Index)
{
	if(get_vr_function() == FALSE)
	{
		rtd_pr_vr_notice("[fail]func:%s,not stream on!!!\n", __FUNCTION__);
		*pdqbuf_Index = 0xFF;
		return FALSE;
	}
	else
	{		
		if(FALSE == HAL_VR_GetVideoFrameBufferIndex(KADP_VR_VIDEO_WINDOW_0, pdqbuf_Index))
		{
			return FALSE;
		}
		else
		{
			
#ifdef DEBUG_DUMP_VR_CAP
		    vr_dump_data_to_file(IndexOfFreezedVideoFrameBufferVr, vr_capture_Width, vr_capture_Height);
#endif
			return TRUE;
		}
	}
	
}

unsigned char HAL_VR_GetVideoFrameOutputDeviceState(KADP_VR_VIDEO_WINDOW_TYPE_T videoWindowID, KADP_VR_VIDEO_FRAME_OUTPUT_DEVICE_STATE_INFO_T *pVideoFrameOutputDeviceState)
{
	if(videoWindowID != KADP_VR_VIDEO_WINDOW_0) {
		rtd_pr_vr_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	down(&VR_Semaphore);
	*pVideoFrameOutputDeviceState = VFODStateVR;
	up(&VR_Semaphore);
	if(get_vr_function() == TRUE) {
		return TRUE;
	} else {
		return FALSE;
	}
}

unsigned char HAL_VR_SetVideoFrameOutputDeviceState(KADP_VR_VIDEO_WINDOW_TYPE_T videoWindowID, KADP_VR_VIDEO_FRAME_OUTPUT_DEVICE_STATE_FLAGS_T videoFrameOutputDeviceStateFlags, KADP_VR_VIDEO_FRAME_OUTPUT_DEVICE_STATE_INFO_T *pVideoFrameOutputDeviceState)
{
	if(videoWindowID != KADP_VR_VIDEO_WINDOW_0) {
		rtd_pr_vr_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}

	if(videoFrameOutputDeviceStateFlags == KADP_VR_VIDEO_FRAME_OUTPUT_DEVICE_STATE_NOFX) {
		rtd_pr_vr_debug("KADP_VR_VIDEO_FRAME_OUTPUT_DEVICE_STATE_NOFX is do nothing\n");
		return TRUE;
	}
	down(&VR_Semaphore);
	if(get_vr_function() == TRUE) { 
		
		if (videoFrameOutputDeviceStateFlags & KADP_VR_VIDEO_FRAME_OUTPUT_DEVICE_STATE_ENABLED) {
			if (VFODStateVR.bEnabled != pVideoFrameOutputDeviceState->bEnabled) {
				curCapInfo.enable = pVideoFrameOutputDeviceState->bEnabled;
				VFODStateVR.bEnabled = pVideoFrameOutputDeviceState->bEnabled;
				drvif_vr_set_DC2H_CaptureConfig(curCapInfo);
			}
		}

		if(videoFrameOutputDeviceStateFlags & KADP_VR_VIDEO_FRAME_OUTPUT_DEVICE_STATE_FRAMERATE_DIVIDE) {
			rtd_pr_vr_debug("KADP_VR_VIDEO_FRAME_OUTPUT_DEVICE_STATE_FRAMERATE_DIVIDE is %d\n",pVideoFrameOutputDeviceState->framerateDivide);
			VFODStateVR.framerateDivide = pVideoFrameOutputDeviceState->framerateDivide;
			
		}

		/*freeze do last*/
		if(videoFrameOutputDeviceStateFlags & KADP_VR_VIDEO_FRAME_OUTPUT_DEVICE_STATE_FREEZED) {
			if(get_vr_function() == TRUE) {
				if(pVideoFrameOutputDeviceState->bFreezed == VFODStateVR.bFreezed) {
					rtd_pr_vr_debug("KADP_VR_VIDEO_FRAME_OUTPUT_DEVICE_STATE_FREEZED is the same as you setting\n");
				} else {
					set_vr_vfod_freezed(pVideoFrameOutputDeviceState->bFreezed);
					VFODStateVR.bFreezed = pVideoFrameOutputDeviceState->bFreezed;
				}
			} else {
				rtd_pr_vr_notice("VFOD is not inited, so no freeze cmd!\n");
			}
		}
		up(&VR_Semaphore);
		return TRUE;
	} else {
		rtd_pr_vr_notice("not Inited So return False;%s=%d\n", __FUNCTION__, __LINE__);
		up(&VR_Semaphore);
		return FALSE;
	}
}

static unsigned short Get_VFOD_FrameRate(void)
{
#define XTAL_CLK (27000000)
	unsigned short framerate = 0;
	
	ppoverlay_dvs_cnt_RBUS ppoverlay_dvs_cnt_Reg;
	ppoverlay_uzudtg_dvs_cnt_RBUS ppoverlay_uzudtg_dvs_cnt_Reg;

	if(VRDumpLocation == KADP_VR_DISPLAY_OUTPUT || VRDumpLocation == KADP_VR_SCALER_OUTPUT || VRDumpLocation == KADP_VR_SCALER_INPUT)
	{
		ppoverlay_uzudtg_dvs_cnt_Reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_DVS_cnt_reg);
		framerate = (XTAL_CLK/ppoverlay_uzudtg_dvs_cnt_Reg.uzudtg_dvs_cnt);
	}
	else
	{
		ppoverlay_dvs_cnt_Reg.regValue = IoReg_Read32(PPOVERLAY_DVS_cnt_reg);
		framerate = (XTAL_CLK/ppoverlay_dvs_cnt_Reg.dvs_cnt);
	}
	
	return framerate;
}

unsigned char HAL_VR_GetVideoFrameOutputDeviceFramerate(KADP_VR_VIDEO_WINDOW_TYPE_T videoWindowID, unsigned int *pVideoFrameOutputDeviceFramerate)
{
	if(videoWindowID != KADP_VR_VIDEO_WINDOW_0) {
		rtd_pr_vr_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	down(&VR_Semaphore);
	if(get_vr_function() == TRUE) {	
		*pVideoFrameOutputDeviceFramerate = Get_VFOD_FrameRate();
		up(&VR_Semaphore);
		return TRUE;
	} else {
		*pVideoFrameOutputDeviceFramerate = 0;
		rtd_pr_vr_notice("not Inited So return pVideoFrameOutputDeviceFramerate 0;%s=%d\n", __FUNCTION__, __LINE__);
		up(&VR_Semaphore);
		return FALSE;
	}
}

unsigned char HAL_VR_GetVideoFrameOutputDeviceDumpLocation(KADP_VR_VIDEO_WINDOW_TYPE_T videoWindowID, KADP_VR_DUMP_LOCATION_TYPE_T *pDumpLocation)
{
	if(videoWindowID != KADP_VR_VIDEO_WINDOW_0) {
		rtd_pr_vr_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	down(&VR_Semaphore);
	if(get_vr_function() == TRUE) {
		*pDumpLocation = VRDumpLocation;
		up(&VR_Semaphore);
		return TRUE;
	} else {
		*pDumpLocation = VRDumpLocation;
		rtd_pr_vr_notice("[Warning]not Inited when get dump location;%s=%d\n", __FUNCTION__, __LINE__);
		up(&VR_Semaphore);
		return FALSE;
	}
}

unsigned char HAL_VR_SetVideoFrameOutputDeviceDumpLocation(KADP_VR_VIDEO_WINDOW_TYPE_T videoWindowID, KADP_VR_DUMP_LOCATION_TYPE_T dumpLocation)
{
	if(videoWindowID != KADP_VR_VIDEO_WINDOW_0) {
		rtd_pr_vr_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	down(&VR_Semaphore);
	if(get_vr_function() == TRUE)
	{
		if(VRDumpLocation == dumpLocation) {
			rtd_pr_vr_notice("VRDumpLocation is the same as you setting\n");
		} else {
			curCapInfo.enable = 0; //close vfod first then rerun
			drvif_vr_set_DC2H_CaptureConfig(curCapInfo);
			curCapInfo.enable = 1;
            
			curCapInfo.capSrc = (VR_CAP_SRC)dumpLocation;
			drvif_vr_set_DC2H_CaptureConfig(curCapInfo);
			if(KADP_VR_SCALER_OUTPUT  ==  dumpLocation)
				VFODStateVR.bAppliedPQ = FALSE;
			else
				VFODStateVR.bAppliedPQ = TRUE;

			VRDumpLocation = dumpLocation;

			if(VFODStateVR.bFreezed == TRUE) {
				set_vr_vfod_freezed(TRUE);//revert freezed status
			}
		}
		up(&VR_Semaphore);
		return TRUE;
	} else {
		rtd_pr_vr_notice("[Warning]not Inited when set dump location;%s=%d\n", __FUNCTION__, __LINE__);
		VRDumpLocation = dumpLocation;
		up(&VR_Semaphore);
		return FALSE;
	}
}

unsigned char HAL_VR_GetVideoFrameOutputDeviceOutputInfo(KADP_VR_VIDEO_WINDOW_TYPE_T videoWindowID, KADP_VR_DUMP_LOCATION_TYPE_T dumpLocation, KADP_VR_VIDEO_FRAME_OUTPUT_DEVICE_OUTPUT_INFO_T *pOutputInfo)
{
	
	ppoverlay_main_active_h_start_end_RBUS main_active_h_start_end_reg;
	ppoverlay_main_active_v_start_end_RBUS main_active_v_start_end_reg;
	ppoverlay_main_den_h_start_end_RBUS main_den_h_start_end_Reg;
	ppoverlay_main_den_v_start_end_RBUS main_den_v_start_end_Reg;
	unsigned int x,y,w,h;

	if(videoWindowID != KADP_VR_VIDEO_WINDOW_0) {
		rtd_pr_vr_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	down(&VR_Semaphore);
	//Main Active H pos
	main_active_h_start_end_reg.regValue = rtd_inl(PPOVERLAY_MAIN_Active_H_Start_End_reg);
	//Main Active V pos
	main_active_v_start_end_reg.regValue = rtd_inl(PPOVERLAY_MAIN_Active_V_Start_End_reg);

	main_den_h_start_end_Reg.regValue = rtd_inl(PPOVERLAY_MAIN_DEN_H_Start_End_reg);
	main_den_v_start_end_Reg.regValue = rtd_inl(PPOVERLAY_MAIN_DEN_V_Start_End_reg);
	
	if((get_vr_function() == TRUE)&&(dumpLocation ==VRDumpLocation )) {
		if((KADP_VR_DISPLAY_OUTPUT == dumpLocation) || (KADP_VR_OSDVIDEO_OUTPUT == dumpLocation)) {
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
		} else if(KADP_VR_SCALER_OUTPUT == dumpLocation) {
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
		up(&VR_Semaphore);
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

		rtd_pr_vr_notice("VFOD is not inited or vfod dump location not matches you want;%s=%d\n", __FUNCTION__, __LINE__);
		up(&VR_Semaphore);
		return FALSE;
	}
}

unsigned char HAL_VR_SetVideoFrameOutputDeviceOutputRegion(KADP_VR_VIDEO_WINDOW_TYPE_T videoWindowID, KADP_VR_DUMP_LOCATION_TYPE_T dumpLocation, KADP_VR_RECT_T *pOutputRegion)
{
	if (videoWindowID != KADP_VR_VIDEO_WINDOW_0) {
		rtd_pr_vr_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	down(&VR_Semaphore);
	rtd_pr_vr_notice("HAL_VR_SetVideoFrameOutputDeviceOutputRegion:output w=%d,output h=%d,dumplocation=%d\n", pOutputRegion->w,pOutputRegion->h,dumpLocation);

	if ((pOutputRegion->w > VR_CAP_FRAME_WIDTH_4K2K) || (pOutputRegion->h > VR_CAP_FRAME_HEIGHT_4K2K)) {
		rtd_pr_vr_emerg("[vr warning]HAL_VR_SetVideoFrameOutputDeviceOutputRegion:could not support max than 4k2k output w=%d,output h=%d! \n", pOutputRegion->w,pOutputRegion->h);
		up(&VR_Semaphore);
		return FALSE;
	}
	
	if(dumpLocation >= KADP_VR_MAX_LOCATION)
	{
		rtd_pr_vr_emerg("[error]%s=%d invalid capture location!\n", __FUNCTION__, __LINE__);
		up(&VR_Semaphore);
		return FALSE;
	}
	if((drvif_scaler_get_abnormal_dvs_long_flag() == TRUE) || (drvif_scaler_get_abnormal_dvs_short_flag() == TRUE)){
		rtd_pr_vr_emerg("[Warning VBE]%s=%d Vbe at abnormal dvs status,could not do vr capture!\n", __FUNCTION__, __LINE__);
		up(&VR_Semaphore);
		return FALSE;			
	}	
	
	if(get_vr_function() == TRUE) {
		if((dumpLocation==VRDumpLocation) && (pOutputRegion->w == curCapInfo.capWid) && (pOutputRegion->h == curCapInfo.capLen)) {
			rtd_pr_vr_notice("all SetVideoFrameOutputDeviceOutputRegion is the same with now\n");
		} else if((pOutputRegion->x != 0) || (pOutputRegion->y != 0)) {
			rtd_pr_vr_notice("[warning]only support output region x y is 0\n");
			up(&VR_Semaphore);
			return FALSE;
		} else if((pOutputRegion->w>(Get_DISP_DEN_END_HPOS() - Get_DISP_DEN_STA_HPOS()))||(pOutputRegion->w<60)||(pOutputRegion->h>(Get_DISP_DEN_END_VPOS() - Get_DISP_DEN_STA_VPOS()))||(pOutputRegion->h<34)) {
			rtd_pr_vr_notice("[warning]OutputRegion over limitation!!!!\n");
			up(&VR_Semaphore);
			return FALSE;
		} else {
			curCapInfo.enable = 0; //close vfod first then rerun
			drvif_vr_set_DC2H_CaptureConfig(curCapInfo);
			if((pOutputRegion->w > vr_cap_frame_max_width) || (pOutputRegion->h > vr_cap_frame_max_height))
			{
				//if target capture size large than allocate buffer size
				// 1st release allocated buffer memory, HW  capture has already disable
				
				Capture_BufferMemDeInit_VR(get_VrBufferNum());

				//2nd use new size to allocate memory
				vr_update_cap_buffer_size_by_AP(pOutputRegion->w, pOutputRegion->h);

				//re-allocate memory with size that AP set
				if(Capture_BufferMemInit_VR(get_VrBufferNum()) == FALSE) 
				{
					rtd_pr_vr_notice("init allocate memory fail,%s=%d \n", __FUNCTION__, __LINE__);
					up(&VR_Semaphore);
					return FALSE;
				}
			}

			curCapInfo.capSrc = (VR_CAP_SRC)dumpLocation;
			curCapInfo.capWid = pOutputRegion->w;
			curCapInfo.capLen = pOutputRegion->h;
			vr_update_cap_buffer_size_by_AP(curCapInfo.capWid,curCapInfo.capLen); 
			
			curCapInfo.enable = 1;
			drvif_vr_set_DC2H_CaptureConfig((VR_CUR_CAPTURE_INFO)curCapInfo);
			if(KADP_VR_SCALER_OUTPUT == dumpLocation)
				VFODStateVR.bAppliedPQ = FALSE;
			else
				VFODStateVR.bAppliedPQ = TRUE;

			VRDumpLocation = dumpLocation;

			if(VFODStateVR.bFreezed == TRUE)
			{
				set_vr_vfod_freezed(TRUE);//revert freezed status
			}
		}
		up(&VR_Semaphore);
		return TRUE;
	} else {
		rtd_pr_vr_notice("not Inited So return False;%s=%d\n", __FUNCTION__, __LINE__);
		up(&VR_Semaphore);
		return FALSE;
	}
}

unsigned char HAL_VR_WaitVsync(KADP_VR_VIDEO_WINDOW_TYPE_T videoWindowID)
{
	if(videoWindowID != KADP_VR_VIDEO_WINDOW_0) {
		rtd_pr_vr_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	if(get_vr_function() == TRUE) {
		unsigned int sleeptime;
		
		unsigned int vrframerate;
#ifndef CONFIG_SUPPORT_SCALER_MODULE
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
		struct timespec64 timeout_s;
#else
		struct timespec timeout_s;
#endif
#endif
		unsigned int waitvsyncframerate;
        vrframerate = Get_VFOD_FrameRate();/*Get_VFOD_FrameRate not return 0*/

        if(get_vr_target_fps() == 0)
        {
            if(VFODStateVR.framerateDivide != 0)
                waitvsyncframerate = vrframerate/VFODStateVR.framerateDivide;
            else
                waitvsyncframerate = vrframerate;
        }
        else
        {
            waitvsyncframerate = get_vr_target_fps();
            if(waitvsyncframerate > vrframerate)
                waitvsyncframerate = vrframerate;
        }

		sleeptime = 1000/waitvsyncframerate;

#ifdef CONFIG_SUPPORT_SCALER_MODULE
		msleep(sleeptime);  // FIXME: hrtimer_nanosleep can't be used by kernel module, please use other API to instead
#else	
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
        timeout_s = ns_to_timespec64(sleeptime * 1000 * 1000);
        hrtimer_nanosleep(timespec64_to_ktime(timeout_s),  HRTIMER_MODE_REL, CLOCK_MONOTONIC);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
        timeout_s = ns_to_timespec64(sleeptime * 1000 * 1000);
        hrtimer_nanosleep(&timeout_s,  HRTIMER_MODE_REL, CLOCK_MONOTONIC);
#else
        timeout_s = ns_to_timespec(sleeptime * 1000 * 1000);
        hrtimer_nanosleep(&timeout_s, NULL, HRTIMER_MODE_REL, CLOCK_MONOTONIC);
#endif
#endif
		return TRUE;
	} else {
		//rtd_pr_vr_notice("not Inited So return False;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}

}

unsigned char HAL_VR_GetInputVideoInfo(KADP_VR_VIDEO_WINDOW_TYPE_T videoWindowID, KADP_VR_INPUT_VIDEO_INFO_T *pInputVideoInfo)
{
	if(videoWindowID != KADP_VR_VIDEO_WINDOW_0)
	{
		rtd_pr_vr_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	down(&VR_Semaphore);
	if(get_vr_function() == TRUE) {
		pInputVideoInfo->region.x = 0;
		pInputVideoInfo->region.y = 0;
		pInputVideoInfo->region.w =Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPH_ACT_WID_PRE);
		pInputVideoInfo->region.h = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE);
		if (Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE) == TRUE)
		{
			pInputVideoInfo->type = KADP_VR_VIDEO_PROGRESSIVE;
			pInputVideoInfo->region.h = pInputVideoInfo->region.h *2;
		} else {
			pInputVideoInfo->type = KADP_VR_VIDEO_PROGRESSIVE;
		}
		pInputVideoInfo->bIs3DVideo = FALSE;
		up(&VR_Semaphore);
		return TRUE;
	} else {
		pInputVideoInfo->region.x = 0;
		pInputVideoInfo->region.y =0;
		pInputVideoInfo->region.w =0;
		pInputVideoInfo->region.h = 0;
		pInputVideoInfo->type = KADP_VR_VIDEO_PROGRESSIVE;
		pInputVideoInfo->bIs3DVideo = FALSE;
		rtd_pr_vr_notice("not Inited So return InputVideoregion 0;%s=%d\n", __FUNCTION__, __LINE__);
		up(&VR_Semaphore);
		return FALSE;
	}
}

unsigned char HAL_VR_GetOutputVideoInfo(KADP_VR_VIDEO_WINDOW_TYPE_T videoWindowID, KADP_VR_OUTPUT_VIDEO_INFO_T *pOutputVideoInfo)
{
	ppoverlay_main_active_h_start_end_RBUS main_active_h_start_end_reg;
	ppoverlay_main_active_v_start_end_RBUS main_active_v_start_end_reg;
	unsigned int x,y,w,h;
	if(videoWindowID != KADP_VR_VIDEO_WINDOW_0) {
		rtd_pr_vr_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	down(&VR_Semaphore);
	main_active_h_start_end_reg.regValue = rtd_inl(PPOVERLAY_MAIN_Active_H_Start_End_reg);
	main_active_v_start_end_reg.regValue = rtd_inl(PPOVERLAY_MAIN_Active_V_Start_End_reg);
	x = main_active_h_start_end_reg.mh_act_sta;
	y =main_active_v_start_end_reg.mv_act_sta;
	w =main_active_h_start_end_reg.mh_act_end - main_active_h_start_end_reg.mh_act_sta;
	h = main_active_v_start_end_reg.mv_act_end - main_active_v_start_end_reg.mv_act_sta;

	if(get_vr_function() == TRUE) {
		if((KADP_VR_DISPLAY_OUTPUT==VRDumpLocation)||(KADP_VR_OSDVIDEO_OUTPUT==VRDumpLocation)) {
			pOutputVideoInfo->maxRegion.x = Get_DISP_ACT_STA_HPOS();
			pOutputVideoInfo->maxRegion.y = Get_DISP_ACT_STA_VPOS();
			pOutputVideoInfo->maxRegion.w = Get_DISP_ACT_END_HPOS() - Get_DISP_ACT_STA_HPOS();
			pOutputVideoInfo->maxRegion.h = Get_DISP_ACT_END_VPOS() - Get_DISP_ACT_STA_VPOS();

			pOutputVideoInfo->activeRegion.x = x;
			pOutputVideoInfo->activeRegion.y = y;
			pOutputVideoInfo->activeRegion.w = w;
			pOutputVideoInfo->activeRegion.h = h;
			pOutputVideoInfo->type = KADP_VR_VIDEO_PROGRESSIVE;
		} else if(KADP_VR_SCALER_OUTPUT==VRDumpLocation) {
			pOutputVideoInfo->maxRegion.x = x;
			pOutputVideoInfo->maxRegion.y = y;
			pOutputVideoInfo->maxRegion.w = w;
			pOutputVideoInfo->maxRegion.h = h;

			pOutputVideoInfo->activeRegion.x = x;
			pOutputVideoInfo->activeRegion.y = y;
			pOutputVideoInfo->activeRegion.w = w;
			pOutputVideoInfo->activeRegion.h = h;

			pOutputVideoInfo->type = KADP_VR_VIDEO_PROGRESSIVE;
		}		
		up(&VR_Semaphore);
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

		pOutputVideoInfo->type = KADP_VR_VIDEO_PROGRESSIVE;
		up(&VR_Semaphore);
		rtd_pr_vr_notice("not Inited So return OutputVideoregion 0;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
}

unsigned char HAL_VR_GetVideoMuteStatus(KADP_VR_VIDEO_WINDOW_TYPE_T videoWindowID, unsigned char *pbOnOff)
{
	if(videoWindowID != KADP_VR_VIDEO_WINDOW_0) {
		rtd_pr_vr_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	down(&VR_Semaphore);
	if(get_vr_function() == TRUE) {
		*pbOnOff = get_vsc_mutestatus();
		up(&VR_Semaphore);
		return TRUE;
	} else {
		*pbOnOff = TRUE;
		rtd_pr_vr_notice("not Inited So return MuteStatus true;%s=%d\n", __FUNCTION__, __LINE__);
		up(&VR_Semaphore);
		return FALSE;
	}
}

unsigned char HAL_VR_GetVideoFrameBufferSecureVideoState(KADP_VR_VIDEO_WINDOW_TYPE_T videoWindowID, unsigned char *pIsSecureVideo)
{
	if(videoWindowID != KADP_VR_VIDEO_WINDOW_0) {
		rtd_pr_vr_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}

	if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC) {
		* pIsSecureVideo = get_vdec_securestatus();
	} else {
		* pIsSecureVideo = FALSE;
	}
	return TRUE;
}

unsigned char HAL_VR_GetVideoFrameOutputDeviceBlockState(KADP_VR_VIDEO_WINDOW_TYPE_T videoWindowID, unsigned char *pbBlockState)
{
	if(videoWindowID != KADP_VR_VIDEO_WINDOW_0) {
		rtd_pr_vr_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}

	down(&VRBlock_Semaphore);
	*pbBlockState = VRBlockStatus;
	up(&VRBlock_Semaphore);
	return TRUE;
}

unsigned char HAL_VR_SetVideoFrameOutputDeviceBlockState(KADP_VR_VIDEO_WINDOW_TYPE_T videoWindowID, unsigned char bBlockState)
{
	if(videoWindowID != KADP_VR_VIDEO_WINDOW_0) {
		rtd_pr_vr_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}

	down(&VRBlock_Semaphore);
	VRBlockStatus = bBlockState;
	up(&VRBlock_Semaphore);
	return TRUE;
}

unsigned char HAL_VR_set_Pixel_Format(VR_CAP_FMT value)
{
	down(&VR_Semaphore);
	if((value >= VR_CAP_RGB888)&&(value < VR_CAP_MAX)) {
		if (value != get_VR_Pixel_Format()) {
			rtd_pr_vr_notice("%s=%d\n", __FUNCTION__, value);
			set_VR_Pixel_Format(value);
			/*close vr, and release last pixel format buffer start*/
			curCapInfo.enable = 0; //iMode;
			drvif_vr_set_DC2H_CaptureConfig(curCapInfo);
			Capture_BufferMemDeInit_VR(get_VrBufferNum());
			/*close vr, and release last pixel format buffer end*/

			/*open vr, and allocate new pixel format buffer start*/
			//curCapInfo.enable = 1; //iMode;		
			if (Capture_BufferMemInit_VR(get_VrBufferNum()) == FALSE) {
				rtd_pr_vr_notice("init allocate memory fail when set_VR_Pixel_Format;%s=%d \n", __FUNCTION__, __LINE__);
				up(&VR_Semaphore);
				return FALSE;
			}
			//drvif_vr_set_DC2H_CaptureConfig(curCapInfo);	
			/*open vr, and allocate new pixel format buffer end*/
		}
		up(&VR_Semaphore);
		return TRUE;
	} else {
		up(&VR_Semaphore);
		return FALSE;
	}
}

void vr_set_memc_mute(bool enable)
{
	unsigned int timeoutcount = 100;
	ppoverlay_double_buffer_ctrl2_RBUS double_buffer_ctrl_reg;
	ppoverlay_memc_mux_ctrl_RBUS memc_mux_ctrl_reg;
	
	double_buffer_ctrl_reg.regValue =  IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	double_buffer_ctrl_reg.uzudtgreg_dbuf_en = 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl_reg.regValue);
	
	/* MEMC_Out_Bg_en  ctrl */
	memc_mux_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg);
	memc_mux_ctrl_reg.memc_out_bg_en = enable;
	IoReg_Write32(PPOVERLAY_MEMC_MUX_CTRL_reg, memc_mux_ctrl_reg.regValue); 

	/* set apply */
	double_buffer_ctrl_reg.regValue =  IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	double_buffer_ctrl_reg.uzudtgreg_dbuf_set = 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl_reg.regValue);

   /* wait apply */
	do{
		if(PPOVERLAY_Double_Buffer_CTRL2_get_uzudtgreg_dbuf_set(rtd_inl(PPOVERLAY_Double_Buffer_CTRL2_reg))){
			timeoutcount--;
			msleep(0);
		}
		else
			break;
					
	}while(timeoutcount);	

	if(timeoutcount == 0)
	{
		rtd_pr_vr_notice("memc_out_bg_en apply timeout\n");
	}
	
}


unsigned char HAL_VR_EnableFRCMode(unsigned char bEnableFRC)
{
	down(&VR_Semaphore);
	rtd_pr_vr_notice("+++ %s=%d bEnableFRC=%d!\n", __FUNCTION__, __LINE__,bEnableFRC);
	
	if(get_vr_EnableFRCMode() == bEnableFRC){
		rtd_pr_vr_notice("%s=%d bEnableFRC same,no need set again!\n", __FUNCTION__, __LINE__);
		up(&VR_Semaphore);
		return TRUE;
	}
	
	set_vr_EnableFRCMode(bEnableFRC);
	if(bEnableFRC == FALSE){
		if((get_film_mode_parameter().enable == _DISABLE) /*&& (Get_DISPLAY_REFRESH_RATE() != 120)*/){
			if(1 /*(Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE) > 2560) &&
				(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE) > 1440)*/){
				if(1 /*Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 490*/){
					Check_smooth_toggle_update_flag(SLR_MAIN_DISPLAY);
					vr_set_memc_mute(_ENABLE); /* memc mute black */
					//Scaler_MEMC_outMux(_ENABLE,_DISABLE); //db on, mux off
					//Scaler_MEMC_Bypass_On(_ENABLE);
					//rtd_pr_vr_notice("[VR360] memc bypass done!!\n");
					Scaler_MEMC_MEMC_CLK_OnOff(0, 0, 1);	//bypass MEMC with mute on
					rtd_pr_vr_notice("[VR360]memc mute and disable clock done!\n");
				}
				else{
					rtd_pr_vr_notice("[VR360] 4k timing enter vr_frc_mode, set mc on!!\n");
					Check_smooth_toggle_update_flag(SLR_MAIN_DISPLAY);
					Scaler_MEMC_MEMC_CLK_OnOff(0, 1, 1);	//Set MC on with mute on
					Scaler_MEMC_outMux(_ENABLE,_ENABLE);	//db on, mux on
					rtd_pr_vr_notice("[VR360] memc enable done!!\n");
				}
			}
			else if(Scaler_get_vdec_2k120hz()){
				rtd_pr_vr_notice("[VR360] 2k120Hz enter vr_frc_mode, set memc bypass!!\n");
				Scaler_MEMC_outMux(_ENABLE,_DISABLE);	//db on, mux off
				Scaler_MEMC_MEMC_CLK_OnOff(0, 0, 1);	// bypass MEMC with mute on
			}
			else{
				rtd_pr_vr_notice("[VR360] not 4k timing enter vr_frc_mode, set memc on!!\n");
				Scaler_MEMC_MEMC_CLK_OnOff(1, 1, 1);	//Set MEMC on with mute on
				Scaler_MEMC_outMux(_ENABLE,_ENABLE);	//db on, mux off
			}
		}
		//EnableFRCMode is FALSE,pls close FRC
		MEMC_Set_malloc_address(bEnableFRC);
	}
	else{
		//EnableFRCMode is TRUE,pls reopen FRC,however,webos never set HAL_VR_EnableFRCMode(TRUE), so need reopen at HAL_VR_Finalize
		MEMC_Set_malloc_address(bEnableFRC);
	}

	up(&VR_Semaphore);
	return TRUE;

}

unsigned char HAL_VR_Set_OutFps(unsigned int outfps)
{
    uiTargetVrFps = outfps;
    rtd_pr_vr_notice("ap set outfps=%d\n", uiTargetVrFps);
    return TRUE;
}

void drvif_vt_block_vr_recording(unsigned char vrEnableFlag)
{
    if(vrEnableFlag == FALSE)
       return;
    else
    {
        if(vr_get_dc2h_capture_state() == TRUE) /* hw dc2h capture enable */
        {
            dc2h_wait_porch();
            vr_enable_dc2h(FALSE);
            if(get_VrBufferNum() == 4)
            {
				dc2h_dma_dc2h_cap_l1_start_RBUS dc2h_cap_l1_start_Reg;
				dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_Reg;
				unsigned int lastcapbufferaddr = 0;
				dc2h_cap_l1_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L1_Start_db_reg);

				dc2h_cap_l3_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L3_Start_reg);

				if(dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VR.cap_buffer[0].phyaddr){
					lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[3].phyaddr;
				}else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VR.cap_buffer[1].phyaddr){
					lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[0].phyaddr;
				}else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VR.cap_buffer[2].phyaddr){
					lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[1].phyaddr;
				}else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VR.cap_buffer[3].phyaddr){
					lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[2].phyaddr;
				}

				if(lastcapbufferaddr == dc2h_cap_l3_start_Reg.regValue){
					if(lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[0].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[3].phyaddr;
					}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[1].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[0].phyaddr;
					}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[2].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[1].phyaddr;
					}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[3].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VR.cap_buffer[2].phyaddr;
					}
				}
				if(lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[0].phyaddr){
					IndexWhenVrBlocked = 0;
				}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[1].phyaddr){
					IndexWhenVrBlocked = 1;
				}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[2].phyaddr){
					IndexWhenVrBlocked = 2;
				}else if (lastcapbufferaddr == CaptureCtrl_VR.cap_buffer[3].phyaddr){
					IndexWhenVrBlocked = 3;
				}
				rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
			}
            else
            {
                rtd_pr_vr_emerg("not vr recording case\n");
            }
        }
        else
        {
            /* Vt block vr recording process with vr semaphore , set vr blocked = true */
            down(&VR_Semaphore);
            set_vr_blockStatus(TRUE);
            rtd_pr_vr_notice("Vt blocked vr recording process\n");
        }
    }
}

void drvif_vt_reconfig_vr_recording(void)
{
    if(get_vr_blockStatus() == TRUE)
    {
        set_vr_blockStatus(FALSE);
        up(&VR_Semaphore);
    }
    else if((IndexWhenVrBlocked != VR_REPEAT_IDX_INVALID) && (get_vr_function() == TRUE))
    {
        vr_enable_dc2h(TRUE);
        //dc2h_wait_porch();
        IndexWhenVrBlocked = VR_REPEAT_IDX_INVALID;
    }
    else
       rtd_pr_vr_notice("normal vt finalize\n");
}

/* ======================================file operations======================================================*/
#ifdef CONFIG_RTK_FEATURE_FOR_GKI
static struct file* file_open(const char* path, int flags, int rights) {
	struct file* filp = NULL;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	mm_segment_t oldfs;
	oldfs = get_fs();
	set_fs(KERNEL_DS);
#endif
	filp = filp_open(path, flags, rights);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	set_fs(oldfs);
#endif
	if(IS_ERR(filp)) {
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
	set_fs(KERNEL_DS);

#ifdef CONFIG_SUPPORT_SCALER_MODULE
	ret = kernel_read(file, data, size, &offset);
#else
	ret = vfs_read(file, data, size, &offset);
#endif

	set_fs(oldfs);
	return ret;
}*/

static int file_write(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) {
	int ret;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	mm_segment_t oldfs;
	oldfs = get_fs();
	set_fs(KERNEL_DS);
#endif

#ifdef CONFIG_SUPPORT_SCALER_MODULE
    ret = kernel_write(file, data, size, &offset);
#else
	ret = vfs_write(file, data, size, &offset);
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	set_fs(oldfs);
#endif
	return ret;
}

static int file_sync(struct file* file) {
	vfs_fsync(file, 0);
	return 0;
}
#else
static struct file* file_open(const char* path, int flags, int rights) {
	return NULL;
}

static void file_close(struct file* file) {
}

static int file_write(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) {
	return 0;
}

static int file_sync(struct file* file) {
	return 0;
}
#endif

unsigned char vr_dump_data_to_file(unsigned int idx, unsigned int dump_w,unsigned int dump_h)
{
	struct file* filp = NULL;
	unsigned long outfileOffset = 0;
	unsigned int size = VR_CAPTURE_BUFFER_UNITSIZE;
	unsigned char *pVirStartAdrr = NULL;
	static unsigned int g_ulFileCount = 0;
	unsigned int src_phy = CaptureCtrl_VR.cap_buffer[idx].phyaddr;
	char raw_dat_path[30];
	sprintf(raw_dat_path, "/data/vr_dump_%d_%d.raw", idx, g_ulFileCount);
	pVirStartAdrr = (unsigned char *)dvr_remap_uncached_memory(src_phy, size, __builtin_return_address(0));

	if((get_VR_Pixel_Format() == VR_CAP_NV12) || (get_VR_Pixel_Format() == VR_CAP_NV16) || (get_VR_Pixel_Format() == VR_CAP_NV21))
	{
		if (pVirStartAdrr != 0)
		{
			filp = file_open(raw_dat_path, O_RDWR | O_CREAT | O_APPEND, 0);
			if (filp == NULL)
			{
				rtd_pr_vr_emerg("[VR]file open Y fail\n");
				dvr_unmap_memory(pVirStartAdrr, size);
				return FALSE;
			}

			file_write(filp, outfileOffset, pVirStartAdrr, (dump_w * dump_h));//Y plane
			file_sync(filp);
			file_close(filp);

			filp = file_open(raw_dat_path, O_RDWR | O_CREAT | O_APPEND, 0);
			if (filp == NULL)
			{
				rtd_pr_vr_emerg("[VR]file open fail\n");
				dvr_unmap_memory(pVirStartAdrr, size);
				return FALSE;
			}

			if((get_VR_Pixel_Format() == VR_CAP_NV12)||(get_VR_Pixel_Format() == VR_CAP_NV21))
				file_write(filp, outfileOffset, (unsigned char *)(pVirStartAdrr+vr_bufAddr_align(vr_cap_frame_max_width*vr_cap_frame_max_height)), ((dump_w * dump_h)/2));//nv12 UV plane
			else
				file_write(filp, outfileOffset, (unsigned char *)(pVirStartAdrr+vr_bufAddr_align(vr_cap_frame_max_width*vr_cap_frame_max_height)), (dump_w * dump_h));//nv16 UV plane
			file_sync(filp);
			file_close(filp);

			rtd_pr_vr_emerg("[VR]dump_finish\n");
			g_ulFileCount++;
			dvr_unmap_memory(pVirStartAdrr, size);
			return TRUE;
		}
		else {
			rtd_pr_vr_emerg("\n\n\n\n ***************** [VR]dump_to_file NG for null buffer address	*********************\n\n\n\n");
			return FALSE;
		}
	}
	else if(get_VR_Pixel_Format() == VR_CAP_ARGB8888 || get_VR_Pixel_Format() == VR_CAP_ABGR8888 || get_VR_Pixel_Format() == VR_CAP_RGB888 || get_VR_Pixel_Format() == VR_CAP_RGBA8888)
	{
		if (pVirStartAdrr != 0)
		{
			filp = file_open(raw_dat_path, O_RDWR | O_CREAT, 0);
			if (filp == NULL)
			{
				rtd_pr_vr_emerg("[VR]file open fail\n");
				dvr_unmap_memory(pVirStartAdrr, size);
				return FALSE;
			}

			if(get_VR_Pixel_Format() == VR_CAP_RGB888)
				file_write(filp, outfileOffset, pVirStartAdrr, (dump_w * dump_h*3));
			else
				file_write(filp, outfileOffset, pVirStartAdrr, (dump_w * dump_h*4));
			file_sync(filp);
			file_close(filp);

			rtd_pr_vr_emerg("[VR]dump_finish\n");
			g_ulFileCount++;
			dvr_unmap_memory(pVirStartAdrr, size);
			return TRUE;
		}
		else {
			rtd_pr_vr_emerg("\n\n\n\n ***************** [VR]dump_to_file NG for null buffer address	*********************\n\n\n\n");
			return FALSE;
		}
	}
	else
	{
		rtd_pr_vr_emerg("[VR]invalid pixelfmt\n");
		dvr_unmap_memory(pVirStartAdrr, size);
		return TRUE;
	}
}


unsigned int VIVR_PM_REGISTER_DATA[][2]
=
{
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
};

#ifdef CONFIG_PM
static int vivr_suspend (struct device *p_dev)
{
	//int i,size;

	if(get_vr_function() == TRUE)  /*WOSQRTK-13830 tvpowerd suspend call, stop dc2h capture */
	{
		vr_enable_dc2h(FALSE);
		rtd_pr_vr_emerg("%s=%d\n", __FUNCTION__, __LINE__);
	}
	else
		rtd_pr_vr_emerg("%s=%d\n", __FUNCTION__, __LINE__);
#if 0
	size = (sizeof(VIVR_PM_REGISTER_DATA)/4)/2;
	for(i=0; i<size; i++)
	{
		VIVR_PM_REGISTER_DATA[i][1]=rtd_inl(VIVR_PM_REGISTER_DATA[i][0]);
	}
#endif	
	return 0;
}

static int vivr_resume (struct device *p_dev)
{
	//int i, size;
#if 0	
	size = (sizeof(VIVR_PM_REGISTER_DATA)/4)/2;
	for(i=0; i<size; i++)
	{
		rtd_outl(VIVR_PM_REGISTER_DATA[i][0],VIVR_PM_REGISTER_DATA[i][1]);
	}
#endif	
	return 0;
}
#endif


int vivr_open(struct inode *inode, struct file *filp) {
	return 0;
}

ssize_t  vivr_read(struct file *filep, char *buffer, size_t count, loff_t *offp)
{
	return 0;

}

ssize_t vivr_write(struct file *filep, const char *buffer, size_t count, loff_t *offp)
{
	long ret = count;
	char cmd_buf[100] = {0};
	
#ifdef ENABLE_VR_TEST_CASE_CONFIG
	unsigned int cap_idx = 0;
#endif

#ifndef CONFIG_ARM64
	rtd_pr_vr_notice("%s(): count=%d, buf=0x%08lx\n", __func__, count, (long)buffer);
#endif

	if (count >= 100)
		return -EFAULT;

	if (copy_from_user(cmd_buf, buffer, count)) {
		ret = -EFAULT;
	}

	if(count > 0) {
	   cmd_buf[count] = '\0';
	}
	
	if ((cmd_buf[0] == 'v') && (cmd_buf[1] == 'r') && (cmd_buf[2] == '-') && (cmd_buf[3] == '-') && (cmd_buf[4] == 'h')
			&& (cmd_buf[5] == 'e') && (cmd_buf[6] == 'l') && (cmd_buf[7] == 'p'))
	{	 /* echo vr--help > /dev/vivrdev */
		rtd_pr_vr_notice("[VR] capture szie:echo vrsize wid len > /dev/vivrdev\n");
		rtd_pr_vr_notice("[VR] set pixelfmt:echo vrfmt=0 > /dev/vivrdev\n");
		rtd_pr_vr_notice("[VR] vrfmt:0(RGB888),1(ARGB8888),2(RGBA8888),3(ABGR8888),4(NV12),5(NV16)\n");
		rtd_pr_vr_notice("[VR] vrtest case location buffernum, ex:echo vrtest 1 1 1 > /dev/vivrdev\n");
		rtd_pr_vr_notice("[VR] case:0(DISABLE),1(single capture),2(ACR),3(VR360)\n");
		rtd_pr_vr_notice("[VR] location:0(Scaler_Input),1(Scaler_Output),2(Display_Output),3(Blended_Output),4(Osd_Output)\n");
		rtd_pr_vr_notice("[VR] buffernum:1(single capture),3(ACR),5(VR360)\n");
		rtd_pr_vr_notice("[VR] dump raw:echo vrdump=1 > /dev/vivrdev\n");		
		rtd_pr_vr_notice("[VR] exit VR testcase: echo vrtest 0 > /dev/vivrdev\n");
	}
#ifdef ENABLE_VR_TEST_CASE_CONFIG
	else if((cmd_buf[0] == 'v') && (cmd_buf[1] == 'r') && (cmd_buf[2] == 's') && (cmd_buf[3] == 'i') && (cmd_buf[4] == 'z')
			&& (cmd_buf[5] == 'e'))
	{
		 /* echo vrsize wid len > /dev/vivrdev, ex: echo vrsize 1920 1080 > /dev/vivrdev */
		if((cmd_buf[10] <= '9') && (cmd_buf[10] >= '0'))
		{
			vr_capture_Width = ((cmd_buf[7]-0x30)*1000 + (cmd_buf[8]-0x30)*100 + (cmd_buf[9]-0x30)*10 + (cmd_buf[10]-0x30)*1);
			if((cmd_buf[15] <= '9') && (cmd_buf[15] >= '0'))
				vr_capture_Height = ((cmd_buf[12]-0x30)*1000 + (cmd_buf[13]-0x30)*100 + (cmd_buf[14]-0x30)*10 + (cmd_buf[15]-0x30)*1);
			else
				vr_capture_Height = ((cmd_buf[12]-0x30)*100 + (cmd_buf[13]-0x30)*10 + (cmd_buf[14]-0x30)*1);
		}	
		else
		{
			vr_capture_Width = ((cmd_buf[7]-0x30)*100 + (cmd_buf[8]-0x30)*10 + (cmd_buf[9]-0x30)*1);
			if((cmd_buf[14] <= '9') && (cmd_buf[14] >= '0'))
				vr_capture_Height = ((cmd_buf[11]-0x30)*1000 + (cmd_buf[12]-0x30)*100 + (cmd_buf[13]-0x30)*10 + (cmd_buf[14]-0x30)*1);
			else
				vr_capture_Height = ((cmd_buf[11]-0x30)*100 + (cmd_buf[12]-0x30)*10 + (cmd_buf[13]-0x30)*1);
		}
			
		vr_update_cap_buffer_size_by_AP(vr_capture_Width,vr_capture_Height);
	}
	else if((cmd_buf[0] == 'v') && (cmd_buf[1] == 'r') && (cmd_buf[2] == 'f') && (cmd_buf[3] == 'm') && (cmd_buf[4] == 't')
			&& (cmd_buf[5] == '='))
	{
		rtd_pr_vr_notice("[VR]current pixelfmt=%d\n", get_VR_Pixel_Format());
		
		if(((cmd_buf[6]-0x30) >= VR_CAP_RGB888) && ((cmd_buf[6]-0x30) <= VR_CAP_MAX))
		{
			if((cmd_buf[6]-0x30) != get_VR_Pixel_Format())
			{
				set_VR_Pixel_Format((VR_CAP_FMT)(cmd_buf[6]-0x30));
				rtd_pr_vr_notice("[VR]set pixelfmt=%d\n", get_VR_Pixel_Format());
			}
		}
		else
		{
			rtd_pr_vr_notice("[VR]set pixelfmt error\n");
			ret = -EFAULT;
			return ret;
		}
	}
	else if(((cmd_buf[0] == 'v') && (cmd_buf[1] == 'r') && (cmd_buf[2] == 't') && (cmd_buf[3] == 'e') && (cmd_buf[4] == 's')
			&& (cmd_buf[5] == 't')))   /* echo vrtest case location buffernum > /dev/vivrdev */
	{
		if(((cmd_buf[7]-0x30) >= VR_TEST_CASE_DISABLE) && ((cmd_buf[7]-0x30) <= VR_TEST_VR360_CASE)){
			set_vr_test_case((VR_TEST_CASE_T)(cmd_buf[7]-0x30));
		}else{
			rtd_pr_vr_notice("[VR] set test case error\n");
			ret = -EFAULT;
			return ret;
		}		

		if(get_vr_test_case() != VR_TEST_CASE_DISABLE)
		{
			if(((cmd_buf[9]-0x30) >= KADP_VR_SCALER_INPUT) && ((cmd_buf[9]-0x30) <= KADP_VR_OSD_OUTPUT)){
				vr_capture_location = (cmd_buf[9]-0x30);
				VRDumpLocation = (KADP_VR_DUMP_LOCATION_TYPE_T)vr_capture_location;
			}else{
				rtd_pr_vr_notice("[VR] set test case location error\n");
				ret = -EFAULT;
				return ret;
			}
		
			if(((cmd_buf[11]-0x30) >= 1) && ((cmd_buf[11]-0x30) <= 5)){
				set_VrBufferNum(cmd_buf[11]-0x30);
			}else{
				rtd_pr_vr_notice("[VR] set test case bufnum error\n");
				ret = -EFAULT;
				return ret;
			}
		}
	
		rtd_pr_vr_notice("[VR]%s\n", cmd_buf);

		//start test case
		handle_vr_test_case();
	}
	else if(strcmp(cmd_buf, "vrdump=1\n") == 0)
	{
		rtd_pr_vr_notice("[VR]enable vr dump buffer\n");
		//get buffer index & dump
		HAL_VR_WaitVsync(KADP_VR_VIDEO_WINDOW_0);
		if(do_vr_dqbuf(&cap_idx) == TRUE){
			vr_dump_data_to_file(cap_idx, vr_capture_Width, vr_capture_Height);
		}
		else
			rtd_pr_vr_notice("[VR]dqubf fail,please retry\n");
	}
	else
	{
		rtd_pr_vr_notice("[VR]%s\n", cmd_buf);
	}

#endif

	return ret;
}
	
	
#ifdef ENABLE_VR_TEST_CASE_CONFIG
	
void handle_vr_test_case(void)
{
	if(get_vr_test_case() == VR_TEST_CASE_DISABLE)
	{
		//exit vr capture
		do_vr_streamoff();
		do_vr_reqbufs(0);
	}
	else
	{
		if(get_vr_test_case() == VR_TEST_VR360_CASE)
		{
			HAL_VR_EnableFRCMode(FALSE);
		}
		do_vr_reqbufs(get_VrBufferNum());

		do_vr_capture_streamon();
	}
}	
#endif


int vivr_release(struct inode *inode, struct file *filep)
{
	return 0;
}


long vivr_ioctl(struct file *file, unsigned int cmd,  unsigned long arg)
{
	int retval = 0;
	if (_IOC_TYPE(cmd) != VR_IOC_MAGIC || _IOC_NR(cmd) > VR_IOC_MAXNR) return -ENOTTY ;
	
	switch (cmd)
	{
		case VR_IOC_INIT:
		{
			if(HAL_VR_Init()==FALSE)
				retval = -1;
			break;
		}
		case VR_IOC_INITEX:
		{
			unsigned int buffernum;
			if(copy_from_user((void *)&buffernum, (const void __user *)arg, sizeof(unsigned int)))
			{
				retval = -EFAULT;
				rtd_pr_vr_debug("copy_to_user:scaler vr ioctl code=VR_IOC_INITEX failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VR_InitEx(buffernum)==FALSE)
					retval =  -1;					
			}
			break;
		}		
		case VR_IOC_FINALIZE:
		{
			//addr = *((unsigned int *)arg);
			if(HAL_VR_Finalize()==FALSE)
				retval = -1;
			break;
		}
		case VR_IOC_GET_DEVICE_CAPABILITY:
		{
			KADP_VR_DEVICE_CAPABILITY_INFO_T DeviceCapabilityInfo;
			if(HAL_VR_GetDeviceCapability(&DeviceCapabilityInfo)==FALSE) {
				retval = -1;
			} else {
				if(copy_to_user((void __user *)arg, (void *)&DeviceCapabilityInfo, sizeof(KADP_VR_DEVICE_CAPABILITY_INFO_T)))
				{
					retval = -EFAULT;
					rtd_pr_vr_debug("copy_to_user:scaler vr ioctl code=VR_IOC_GET_DEVICE_CAPABILITY failed!!!!!!!!!!!!!!!\n");
				}
			}

			break;
		}
		case VR_IOC_GET_VFB_CAPABILITY:
		{
			KADP_VR_VFB_CAPINFO_T vfb_capinfo;
			if(copy_from_user((void *)&vfb_capinfo, (const void __user *)arg, sizeof(KADP_VR_VFB_CAPINFO_T)))
			{
				retval = -EFAULT;
				rtd_pr_vr_debug("copy_from_user:scaler vr ioctl code=VR_IOC_GET_VFB_CAPABILITY failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VR_GetVideoFrameBufferCapability(vfb_capinfo.wid,&vfb_capinfo.vfbCapInfo)==FALSE) {
					retval = -1;
				} else {
					if(copy_to_user((void __user *)arg, (void *)&vfb_capinfo, sizeof(KADP_VR_VFB_CAPINFO_T)))
					{
						retval = -EFAULT;
						rtd_pr_vr_debug("copy_to_user:scaler vr ioctl code=VR_IOC_GET_VFB_CAPABILITY failed!!!!!!!!!!!!!!!\n");
					}
				}
			}
			break;
		}
		case VR_IOC_GET_VFOD_CAPABILITY:
		{
			KADP_VR_VFOD_CAPINFO_T vfod_capinfo;
			if(copy_from_user((void *)&vfod_capinfo, (const void __user *)arg, sizeof(KADP_VR_VFOD_CAPINFO_T)))
			{
				retval = -EFAULT;
				rtd_pr_vr_debug("copy_from_user:scaler vr ioctl code=VR_IOC_GET_VFOD_CAPABILITY failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VR_GetVideoFrameOutputDeviceCapability(vfod_capinfo.wid,&vfod_capinfo.vfodCapInfo)==FALSE) {
					retval = -1;
				} else {
					if(copy_to_user((void __user *)arg, (void *)&vfod_capinfo, sizeof(KADP_VR_VFOD_CAPINFO_T)))
					{
						retval = -EFAULT;
						rtd_pr_vr_debug("copy_to_user:scaler vr ioctl code=VR_IOC_GET_VFOD_CAPABILITY failed!!!!!!!!!!!!!!!\n");
					}
				}
			}
			break;
		}
		case VR_IOC_GET_VFOD_LIMITATION:
		{
			KADP_VR_VFOD_LIMITATIONINFO_T vfod_limitationinfo;
			if(copy_from_user((void *)&vfod_limitationinfo, (const void __user *)arg, sizeof(KADP_VR_VFOD_LIMITATIONINFO_T)))
			{
				retval = -EFAULT;
				rtd_pr_vr_debug("copy_from_user:scaler vr ioctl code=VR_IOC_GET_VFOD_LIMITATION failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VR_GetVideoFrameOutputDeviceLimitation(vfod_limitationinfo.wid,&vfod_limitationinfo.vfodlimitationInfo)==FALSE) {
					retval = -1;
				} else {
					if(copy_to_user((void __user *)arg, (void *)&vfod_limitationinfo, sizeof(KADP_VR_VFOD_LIMITATIONINFO_T)))
					{
						retval = -EFAULT;
						rtd_pr_vr_debug("copy_to_user:scaler vr ioctl code=VR_IOC_GET_VFOD_LIMITATION failed!!!!!!!!!!!!!!!\n");
					}
				}
			}
			break;
		}
		case VR_IOC_GET_ALLVFB_PROPERTY:
		{
			KADP_VR_VFB_PROPERTY_T vfb_property;
			if(copy_from_user((void *)&vfb_property, (const void __user *)arg, sizeof(KADP_VR_VFB_PROPERTY_T)))
			{
				retval = -EFAULT;
				rtd_pr_vr_debug("copy_from_user:scaler vr ioctl code=VR_IOC_GET_ALLVFB_PROPERTY failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VR_GetAllVideoFrameBufferProperty(vfb_property.wid,&vfb_property.vfbProInfo)==FALSE) {
					retval = -1;
				} else {
					if(copy_to_user((void __user *)arg, (void *)&vfb_property, sizeof(KADP_VR_VFB_PROPERTY_T)))
					{
						retval = -EFAULT;
						rtd_pr_vr_debug("copy_to_user:scaler vr ioctl code=VR_IOC_GET_ALLVFB_PROPERTY failed!!!!!!!!!!!!!!!\n");
					}
				}
			}
			break;
		}
		case VR_IOC_GET_VFB_INDEX:
		{
			KADP_VR_GET_VFBINDEX_T vfb_index;
			if(copy_from_user((void *)&vfb_index, (const void __user *)arg, sizeof(KADP_VR_GET_VFBINDEX_T)))
			{
				retval = -EFAULT;
				rtd_pr_vr_debug("copy_from_user:scaler vr ioctl code=VR_IOC_GET_VFB_INDEX failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VR_GetVideoFrameBufferIndex(vfb_index.wid,&vfb_index.IndexCurVFB)==FALSE) {
					retval = -1;
				} else {
					if(copy_to_user((void __user *)arg, (void *)&vfb_index, sizeof(KADP_VR_GET_VFBINDEX_T)))
					{
						retval = -EFAULT;
						rtd_pr_vr_debug("copy_to_user:scaler vr ioctl code=VR_IOC_GET_VFB_INDEX failed!!!!!!!!!!!!!!!\n");
					}
				}
			}
			break;
		}
		case VR_IOC_GET_VFOD_STATE:
		{
			KADP_VR_VFOD_GET_STATEINFO_T vfodgetstateinfo;
			if(copy_from_user((void *)&vfodgetstateinfo, (const void __user *)arg, sizeof(KADP_VR_VFOD_GET_STATEINFO_T)))
			{
				retval = -EFAULT;
				rtd_pr_vr_debug("copy_from_user:scaler vr ioctl code=VR_IOC_GET_VFOD_STATE failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VR_GetVideoFrameOutputDeviceState(vfodgetstateinfo.wid,&vfodgetstateinfo.VFODstate)==FALSE) {
					retval =-1;
				} else {
					if(copy_to_user((void __user *)arg, (void *)&vfodgetstateinfo, sizeof(KADP_VR_VFOD_GET_STATEINFO_T)))
					{
						retval = -EFAULT;
						rtd_pr_vr_debug("copy_to_user:scaler vr ioctl code=VR_IOC_GET_VFOD_STATE failed!!!!!!!!!!!!!!!\n");
					}
				}
			}
			break;
		}
		case VR_IOC_SET_VFOD_STATE:
		{
			KADP_VR_VFOD_SET_STATEINFO_T vfodsetstateinfo;
			if(copy_from_user((void *)&vfodsetstateinfo, (const void __user *)arg, sizeof(KADP_VR_VFOD_SET_STATEINFO_T)))
			{
				retval = -EFAULT;
				rtd_pr_vr_debug("copy_from_user:scaler vr ioctl code=VR_IOC_SET_VFOD_STATE failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VR_SetVideoFrameOutputDeviceState(vfodsetstateinfo.wid,vfodsetstateinfo.vfodStateFlag,&vfodsetstateinfo.VFODstate)==FALSE)
					retval =-1;
			}

			break;
		}
		case VR_IOC_GET_VFOD_FRAMERATE:
		{
			KADP_VR_VFOD_GET_FRAMERATE_T vfodgetfr;
			if(copy_from_user((void *)&vfodgetfr, (const void __user *)arg, sizeof(KADP_VR_VFOD_GET_FRAMERATE_T)))
			{
				retval = -EFAULT;
				rtd_pr_vr_debug("copy_from_user:scaler vr ioctl code=VR_IOC_GET_VFOD_FRAMERATE failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VR_GetVideoFrameOutputDeviceFramerate(vfodgetfr.wid,&vfodgetfr.framerate)==FALSE) {
					retval = -1;
				} else {
					if(copy_to_user((void __user *)arg, (void *)&vfodgetfr, sizeof(KADP_VR_VFOD_GET_FRAMERATE_T)))
					{
						retval = -EFAULT;
						rtd_pr_vr_debug("copy_to_user:scaler vr ioctl code=VR_IOC_GET_VFOD_FRAMERATE failed!!!!!!!!!!!!!!!\n");
					}
				}
			}

			break;
		}
		case VR_IOC_GET_VFOD_DUMPLOCATION:
		{
			KADP_VR_VFOD_GET_DUMPLOCATION_T vfodgetdumplocation;
			if(copy_from_user((void *)&vfodgetdumplocation, (const void __user *)arg, sizeof(KADP_VR_VFOD_GET_DUMPLOCATION_T)))
			{
				retval = -EFAULT;
				rtd_pr_vr_debug("copy_from_user:scaler vr ioctl code=VR_IOC_GET_VFOD_DUMPLOCATION failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VR_GetVideoFrameOutputDeviceDumpLocation(vfodgetdumplocation.wid,&vfodgetdumplocation.DumpLocation)==FALSE) {
					retval =-1;
				} else {
					if(copy_to_user((void __user *)arg, (void *)&vfodgetdumplocation, sizeof(KADP_VR_VFOD_GET_DUMPLOCATION_T)))
					{
						retval = -EFAULT;
						rtd_pr_vr_debug("copy_to_user:scaler vr ioctl code=VR_IOC_GET_VFOD_DUMPLOCATION failed!!!!!!!!!!!!!!!\n");
					}
				}
			}

			break;
		}
		case VR_IOC_SET_VFOD_DUMPLOCATION:
		{
			KADP_VR_VFOD_SET_DUMPLOCATION_T vfodsetdumplocation;
			if(copy_from_user((void *)&vfodsetdumplocation, (const void __user *)arg, sizeof(KADP_VR_VFOD_SET_DUMPLOCATION_T)))
			{
				retval = -EFAULT;
				rtd_pr_vr_debug("copy_from_user:scaler vr ioctl code=VR_IOC_SET_VFOD_DUMPLOCATION failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VR_SetVideoFrameOutputDeviceDumpLocation(vfodsetdumplocation.wid,vfodsetdumplocation.DumpLocation)==FALSE)
					retval =-1;
			}

			break;
		}
		case VR_IOC_GET_VFOD_OUTPUTINFO:
		{
			KADP_VR_VFOD_GET_OUTPUTINFO_T vfodgetoututinfo;
			if(copy_from_user((void *)&vfodgetoututinfo, (const void __user *)arg, sizeof(KADP_VR_VFOD_GET_OUTPUTINFO_T)))
			{
				retval = -EFAULT;
				rtd_pr_vr_debug("copy_from_user:scaler vr ioctl code=VR_IOC_GET_VFOD_OUTPUTINFO failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VR_GetVideoFrameOutputDeviceOutputInfo(vfodgetoututinfo.wid,vfodgetoututinfo.DumpLocation,&vfodgetoututinfo.OutputInfo)==FALSE) {
					retval =-1;
				} else {
					if(copy_to_user((void __user *)arg, (void *)&vfodgetoututinfo, sizeof(KADP_VR_VFOD_GET_OUTPUTINFO_T)))
					{
						retval = -EFAULT;
						rtd_pr_vr_debug("copy_to_user:scaler vr ioctl code=VR_IOC_GET_VFOD_OUTPUTINFO failed!!!!!!!!!!!!!!!\n");
					}
				}
			}

			break;
		}
		case VR_IOC_SET_VFOD_OUTPUTREGION:
		{
			KADP_VR_VFOD_SET_OUTPUTREGION_T vfodsetoututinfo;
			if(copy_from_user((void *)&vfodsetoututinfo, (const void __user *)arg, sizeof(KADP_VR_VFOD_SET_OUTPUTREGION_T)))
			{
				retval = -EFAULT;
				rtd_pr_vr_debug("copy_from_user:scaler vr ioctl code=VR_IOC_SET_VFOD_OUTPUTREGION failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VR_SetVideoFrameOutputDeviceOutputRegion(vfodsetoututinfo.wid,vfodsetoututinfo.DumpLocation,&vfodsetoututinfo.OutputRegion)==FALSE)
					retval =-1;
			}

			break;
		}
		case VR_IOC_WAIT_VSYNC:
		{
			KADP_VR_VIDEO_WINDOW_TYPE_T videoWindowID;
			if(copy_from_user((void *)&videoWindowID, (const void __user *)arg, sizeof(KADP_VR_VIDEO_WINDOW_TYPE_T)))
			{
				retval = -EFAULT;
				rtd_pr_vr_debug("scaler vr ioctl code=VR_IOC_WAIT_VSYNC failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VR_WaitVsync(videoWindowID)==FALSE)
					retval =-1;
			}
			break;
		}
		case VR_IOC_GET_INPUTVIDEOINFO:
		{
			KADP_VR_GET_INPUTVIDEOINFO_T inputvideoinfo;
			if(copy_from_user((void *)&inputvideoinfo, (const void __user *)arg, sizeof(KADP_VR_GET_INPUTVIDEOINFO_T)))
			{
				retval = -EFAULT;
				rtd_pr_vr_debug("copy_from_user:scaler vr ioctl code=VR_IOC_GET_INPUTVIDEOINFO failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VR_GetInputVideoInfo(inputvideoinfo.wid,&inputvideoinfo.InputVideoInfo)==FALSE) {
					retval =-1;
				} else {
					if(copy_to_user((void __user *)arg, (void *)&inputvideoinfo, sizeof(KADP_VR_GET_INPUTVIDEOINFO_T)))
					{
						retval = -EFAULT;
						rtd_pr_vr_debug("copy_to_user:scaler vr ioctl code=VR_IOC_GET_INPUTVIDEOINFO failed!!!!!!!!!!!!!!!\n");
					}
				}
			}

			break;
		}
		case VR_IOC_GET_OUTPUTVIDEOINFO:
		{
			KADP_VR_GET_OUTPUTVIDEOINFO_T onputvideoinfo;
			if(copy_from_user((void *)&onputvideoinfo, (const void __user *)arg, sizeof(KADP_VR_GET_OUTPUTVIDEOINFO_T)))
			{
				retval = -EFAULT;
				rtd_pr_vr_debug("scaler vr ioctl code=VR_IOC_GET_OUTPUTVIDEOINFO failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VR_GetOutputVideoInfo(onputvideoinfo.wid,&onputvideoinfo.OutputVideoInfo)==FALSE) {
					retval =-1;
				} else {
					if(copy_to_user((void __user *)arg, (void *)&onputvideoinfo, sizeof(KADP_VR_GET_OUTPUTVIDEOINFO_T)))
					{
						retval = -EFAULT;
						rtd_pr_vr_debug("copy_to_user:scaler vr ioctl code=VR_IOC_GET_OUTPUTVIDEOINFO failed!!!!!!!!!!!!!!!\n");
					}
				}
			}

			break;
		}
		case VR_IOC_GET_VIDEOMUTESTATUS:
		{
			KADP_VR_GET_VIDEOMUTESTATUS_T videomutestatus;
			if(copy_from_user((void *)&videomutestatus, (const void __user *)arg, sizeof(KADP_VR_GET_VIDEOMUTESTATUS_T)))
			{
				retval = -EFAULT;
				rtd_pr_vr_debug("copy_from_user:scaler vr ioctl code=VR_IOC_GET_VIDEOMUTESTATUS failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VR_GetVideoMuteStatus(videomutestatus.wid,&videomutestatus.bOnOff)==FALSE) {
					retval =-1;
				} else {
					if(copy_to_user((void __user *)arg, (void *)&videomutestatus, sizeof(KADP_VR_GET_VIDEOMUTESTATUS_T)))
					{
						retval = -EFAULT;
						rtd_pr_vr_debug("copy_to_user:scaler vr ioctl code=VR_IOC_GET_VIDEOMUTESTATUS failed!!!!!!!!!!!!!!!\n");
					}
				}
			}

			break;
		}
		case VR_IOC_GET_VFB_SVSTATE:
		{
			KADP_VR_GET_VFBSVSTATE_T vfbsvstatus;
			if(copy_from_user((void *)&vfbsvstatus, (const void __user *)arg, sizeof(KADP_VR_GET_VFBSVSTATE_T)))
			{
				retval = -EFAULT;
				rtd_pr_vr_debug("copy_from_user:scaler vr ioctl code=VR_IOC_GET_VFB_SVSTATE failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VR_GetVideoFrameBufferSecureVideoState(vfbsvstatus.wid,&vfbsvstatus.IsSecureVideo)==FALSE) {
					retval =-1;
				} else {
					if(copy_to_user((void __user *)arg, (void *)&vfbsvstatus, sizeof(KADP_VR_GET_VFBSVSTATE_T)))
					{
						retval = -EFAULT;
						rtd_pr_vr_debug("copy_to_user:scaler vr ioctl code=VR_IOC_GET_VFB_SVSTATE failed!!!!!!!!!!!!!!!\n");
					}
				}
			}
			break;
		}
		case VR_IOC_GET_VFOD_BLOCKSTATE:
		{
			KADP_VR_GET_VFODBLOCKSTATE_T vfodblockstatus;
			if(copy_from_user((void *)&vfodblockstatus, (const void __user *)arg, sizeof(KADP_VR_GET_VFODBLOCKSTATE_T)))
			{
				retval = -EFAULT;
				rtd_pr_vr_debug("scaler vr ioctl code=VR_IOC_GET_VFOD_BLOCKSTATE failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VR_GetVideoFrameOutputDeviceBlockState(vfodblockstatus.wid,&vfodblockstatus.bBlockState)==FALSE) {
					retval =-1;
				} else {
					if(copy_to_user((void __user *)arg, (void *)&vfodblockstatus, sizeof(KADP_VR_GET_VFODBLOCKSTATE_T)))
					{
						retval = -EFAULT;
						rtd_pr_vr_debug("copy_to_user:scaler vr ioctl code=VR_IOC_GET_VFOD_BLOCKSTATE failed!!!!!!!!!!!!!!!\n");
					}

				}
			}
			break;
		}
		case VR_IOC_SET_VFOD_BLOCKSTATE:
		{
			KADP_VR_SET_VFODBLOCKSTATE_T setvfodblockstatus;
			if(copy_from_user((void *)&setvfodblockstatus, (const void __user *)arg, sizeof(KADP_VR_SET_VFODBLOCKSTATE_T)))
			{
				retval = -EFAULT;
				rtd_pr_vr_debug("scaler vr ioctl code=VR_IOC_SET_VFOD_BLOCKSTATE failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VR_SetVideoFrameOutputDeviceBlockState(setvfodblockstatus.wid,setvfodblockstatus.bBlockState)==FALSE)
					retval =-1;
			}
			break;
		}
		case VR_IOC_SET_PIXEL_FORMAT:
		{	
			VR_CAP_FMT format;
			if(copy_from_user((void *)&format, (const void __user *)arg, sizeof(VR_CAP_FMT)))
			{
				retval = -EFAULT;
				rtd_pr_vr_debug("scaler vr ioctl code=VR_IOC_SET_PIXEL_FORMAT failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VR_set_Pixel_Format(format)==FALSE)
					retval =-1;
			}
			break;
		}		
		case VR_IOC_VFB_DATASHOW_ONSUBDISP:
		{	
			unsigned char vfbdatashowonsub;
			if(copy_from_user((void *)&vfbdatashowonsub, (const void __user *)arg, sizeof(unsigned char)))
			{
				retval = -EFAULT;
				rtd_pr_vr_debug("scaler vr ioctl code=VR_IOC_VFB_DATASHOW_ONSUBDISP failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				rtd_pr_vr_emerg("vr ioctl code=VR_IOC_VFB_DATASHOW_ONSUBDISP unsupport\n");
				retval =-1;
			}
			break;
		}
		case VR_IOC_EnableFRCMode:
		{
			unsigned int bEnableFrcMode;
			if(copy_from_user((void *)&bEnableFrcMode, (const void __user *)arg, sizeof(unsigned int)))
			{
				retval = -EFAULT;
				rtd_pr_vr_debug("copy_to_user:scaler vr ioctl code=VR_IOC_INITEX failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char bEnableFrc;
				bEnableFrc = (unsigned char)bEnableFrcMode;
				if(HAL_VR_EnableFRCMode(bEnableFrc)==FALSE)
					retval = -1;
			}
			break;
		}
        case VR_IOC_SET_OUTPUT_FPS:
		{
			unsigned int tmpFps;
			if(copy_from_user((void *)&tmpFps, (const void __user *)arg, sizeof(unsigned int)))
			{
				retval = -EFAULT;
				rtd_pr_vr_debug("copy_to_user:scaler vr ioctl code=VR_IOC_SET_OUTPUT_FPS failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VR_Set_OutFps(tmpFps) == FALSE)
					retval = -1;
			}
			break;
		}
		default:
			rtd_pr_vr_debug("Scaler vr disp: ioctl code = %d is invalid!!!!!!!!!!!!!!!1\n", cmd);
			break ;
		}
	return retval;


}

#if defined(CONFIG_COMPAT) && defined(CONFIG_ARM64)
long vivr_compat_ioctl(struct file *file, unsigned int cmd,  unsigned long arg)
{ 
	return vivr_ioctl(file,cmd,arg);
}
#endif

int vivr_major   = VR_MAJOR;
int vivr_minor   = 0 ;
int vivr_nr_devs = VR_NR_DEVS;

module_param(vivr_major, int, S_IRUGO);
module_param(vivr_minor, int, S_IRUGO);
module_param(vivr_nr_devs, int, S_IRUGO);

static struct class *vivr_class = NULL;
static struct platform_device *vivr_platform_devs = NULL;

struct file_operations vivr_fops= {
	.owner =    THIS_MODULE,
	.open  =    vivr_open,
	.release =  vivr_release,
	.read  =    vivr_read,
	.write = 	vivr_write,
	.unlocked_ioctl = 	vivr_ioctl,
#if defined(CONFIG_COMPAT) && defined(CONFIG_ARM64)
  	.compat_ioctl = vivr_compat_ioctl,
#endif
};

#ifdef CONFIG_PM
static const struct dev_pm_ops vivr_pm_ops =
{
	.suspend    = vivr_suspend,
	.resume     = vivr_resume,
#ifdef CONFIG_HIBERNATION
	.freeze 	= vivr_suspend,
	.thaw		= vivr_resume,
	.poweroff	= vivr_suspend,
	.restore	= vivr_resume,
#endif

};
#endif

static struct platform_driver vivr_device_driver = {
    .driver = {
        .name       = VR_DEVICE_NAME,
        .bus        = &platform_bus_type,
#ifdef CONFIG_PM
	.pm 		= &vivr_pm_ops,
#endif

    },
} ;


static char *vivr_devnode(struct device *dev, umode_t *mode)
{
	*mode = 0666;
	return NULL;
}

#if IS_ENABLED(CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER)
static void sb4_dc2h_debug_info(void)
{
	dcmt_trap_info trap_info;
	unsigned int i = 0;
	//check trash module
	if(!isDcmtTrap("TVSB4_DC2H"))  //"SB3_MD_SCPU"?trap ip,ip?????rtk_dc_mt.c??module_info[]
	{
		rtd_pr_vr_err("not TVSB4_DC2H module!\n");
		return;
	}

	get_dcmt_trap_info(&trap_info);  //get dcmt trap information (include trash_addr,module_id,rw_type)
	rtd_pr_vr_err("TVSB4_DC2H module trashed somewhere!\n");

	//dc2h address and control
	for (i = DC2H_SCALEDOWN_DC2H_HSD_Ctrl0_reg; i <=DC2H_SCALEDOWN_DC2H_444to422_CTRL_reg; i=i+4)
	{
		rtd_pr_vr_err("%x=%x\n", i, IoReg_Read32(i));
	}

	for (i = DC2H_RGB2YUV_DC2H_RGB2YUV_CTRL_reg; i <=DC2H_RGB2YUV_DC2H_Tab1_Yo_reg; i=i+4)
	{
		rtd_pr_vr_err("%x=%x\n", i, IoReg_Read32(i));
	}

	rtd_pr_vr_err("0xb8029c20=%x\n", IoReg_Read32(DC2H_RGB2YUV_DC2H_clken_reg));

	for (i = DC2H_RGB2YUV_DC2H_3DMaskTo2D_Ctrl_reg; i <=DC2H_RGB2YUV_DC2H_3DMaskTo2D_Ctrl1_reg; i=i+4)
	{
		rtd_pr_vr_err("%x=%x\n", i, IoReg_Read32(i));
	}

	for (i = DC2H_VI_DC2H_V1_OUTPUT_FMT_reg; i <=DC2H_VI_DC2H_VI_READ_MARGIN_reg; i=i+4)
	{
		rtd_pr_vr_err("%x=%x\n", i, IoReg_Read32(i));
	}

	rtd_pr_vr_err("0xb8029d68=%x\n", IoReg_Read32(DC2H_VI_DC2H_DBG_reg));
	rtd_pr_vr_err("0xb8029d6c=%x\n", IoReg_Read32(DC2H_VI_DC2H_vi_doublebuffer_reg));

	for (i = DC2H_VI_DC2H_vi_c_line_step_reg; i <=DC2H_DMA_dc2h_seq_byte_channel_swap_reg; i=i+4)
	{
		rtd_pr_vr_err("%x=%x\n", i, IoReg_Read32(i));
	}
	return;
}
#endif

DCMT_DEBUG_INFO_DECLARE(sb4_dc2h_mdscpu, sb4_dc2h_debug_info);

int vivr_init_module(void)
{
	int result;
  	int devno;
	dev_t dev = 0;

#if IS_ENABLED(CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER)
	/******************DCMT register callback func in init flow******************/
	DCMT_DEBUG_INFO_REGISTER(sb4_dc2h_mdscpu, sb4_dc2h_debug_info);
	rtd_pr_vr_debug("DCMT_DEBUG_INFO_REGISTER(sb4_dc2h_mdscpu, sb4_dc2h_debug_info)\n");
#endif

	rtd_pr_vr_debug("\n\n\n\n *****************  vr init module  *********************\n\n\n\n");
	if (vivr_major) {
		dev = MKDEV(vivr_major, vivr_minor);
		result = register_chrdev_region(dev, vivr_nr_devs, VR_DEVICE_NAME);
	} else {
		result = alloc_chrdev_region(&dev, vivr_minor, vivr_nr_devs,VR_DEVICE_NAME);
		vivr_major = MAJOR(dev);
	}
	if (result < 0) {
		rtd_pr_vr_debug("vr: can't get major %d\n", vivr_major);
		return result;
	}

	rtd_pr_vr_debug("vr init module major number = %d\n", vivr_major);

	vivr_class = class_create(THIS_MODULE,VR_DEVICE_NAME);

	if (IS_ERR(vivr_class))
	{
		rtd_pr_vr_debug("scalevr: can not create class...\n");
		result = PTR_ERR(vivr_class);
		goto fail_class_create;
	}

	vivr_class->devnode = vivr_devnode;

	vivr_platform_devs = platform_device_register_simple(VR_DEVICE_NAME, -1, NULL, 0);

	if((result=platform_driver_register(&vivr_device_driver)) != 0){
	rtd_pr_vr_debug("scalevr: can not register platform driver...\n");
	result = -ENOMEM;
	goto fail_platform_driver_register;
	}

    devno = MKDEV(vivr_major, vivr_minor);
    cdev_init(&vivr_cdev, &vivr_fops);
    vivr_cdev.owner = THIS_MODULE;
   	vivr_cdev.ops = &vivr_fops;
	result = cdev_add (&vivr_cdev, devno, 1);
	if (result<0)
	{
		rtd_pr_vr_debug("scalevr: can not add character device...\n");
		goto fail_cdev_init;
	}
    device_create(vivr_class, NULL, MKDEV(vivr_major, 0), NULL, VR_DEVICE_NAME);
	sema_init(&VR_Semaphore, 1);
	sema_init(&VRBlock_Semaphore, 1);
	
    return 0;	//success

fail_cdev_init:
	platform_driver_unregister(&vivr_device_driver);
fail_platform_driver_register:
	platform_device_unregister(vivr_platform_devs);
	vivr_platform_devs = NULL;
	class_destroy(vivr_class);
fail_class_create:
	vivr_class = NULL;
	unregister_chrdev_region(vivr_devno, 1);
	return result;
}

void __exit vivr_cleanup_module(void)
{
	dev_t devno = MKDEV(vivr_major, vivr_minor);
	rtd_pr_vr_debug("rtice clean module vr_major = %d\n", vivr_major);
  	device_destroy(vivr_class, MKDEV(vivr_major, 0));
  	class_destroy(vivr_class);
	vivr_class = NULL;
	cdev_del(&vivr_cdev);
   	/* device driver removal */
	if(vivr_platform_devs) {
		platform_device_unregister(vivr_platform_devs);
		vivr_platform_devs = NULL;
	}
  	platform_driver_unregister(&vivr_device_driver);
	/* cleanup_module is never called if registering failed */
	unregister_chrdev_region(devno, vivr_nr_devs);
}


#ifdef CONFIG_SUPPORT_SCALER_MODULE
// the module init/exit will be moved to scaler_module.c if scaler was built as a kernel module
#else
module_init(vivr_init_module);
module_exit(vivr_cleanup_module);
#endif
