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
#include <linux/pageremap.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <uapi/linux/const.h>
#include <linux/mm.h>
#include <rtd_log/rtd_module_log.h>
#include <linux/fs.h>/*for dump buffer to usb file*/
#include <linux/hrtimer.h>/*for wait vsync hr sleep*/
#include <linux/version.h>
#include <linux/export.h> //EXPORT_SYMBOL

#include <rtk_kdriver/RPCDriver.h>
#include <rtk_dc_mt.h>

#include <rbus/dc2h_dma_reg.h>
#include <rbus/dc2h_vi_reg.h>
#include <rbus/dc2h_scaledown_reg.h>
#include <rbus/dc2h_rgb2yuv_reg.h>
#include <rbus/ppoverlay_reg.h>
#include <rbus/mdomain_disp_reg.h>
#include <rbus/scaleup_reg.h>
#include <rbus/scaledown_reg.h>

#include <rbus/yuv2rgb_reg.h>
#include <rbus/rgb2yuv_reg.h>

#include <rbus/timer_reg.h>

#include <tvscalercontrol/vo/rtk_vo.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/panel/panelapi.h>
#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerCommon.h>
#else
#include <rtk_kdriver/scalercommon/scalerCommon.h>
#endif

#include <tvscalercontrol/scaler/scalerstruct.h>
#include <tvscalercontrol/scalerdrv/mode.h>
#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <tvscalercontrol/scalerdrv/pipmp.h>
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#include <tvscalercontrol/scalerdrv/scalerdrv.h>
#include <tvscalercontrol/scalerdrv/zoom_smoothtoggle.h>
#include <tvscalercontrol/scalerdrv/zoom_smoothtoggle_lib.h>
#include <scaler/vipRPCCommon.h>
#include <tvscalercontrol/vip/ultrazoom.h>
#include <tvscalercontrol/panel/panelapi.h>

#include "scaler_vtdev.h"
#include "scaler_vscdev.h"
#include "scaler_vpqmemcdev.h"
#include "rtk_kadp_se.h"

//#ifndef UT_flag
struct semaphore VT_Semaphore;
static struct semaphore VTBlock_Semaphore;

#define _ALIGN(val, align) (((val) + ((align) - 1)) & ~((align) - 1))
#define __4KPAGE  0x1000
/*4k and 96 align*/
#define __12KPAGE  0x3000

unsigned int VT_CAPTURE_BUFFER_UNITSIZE;

VT_CAPTURE_CTRL_T CaptureCtrl_VT;

#ifndef UT_flag
static dev_t vivt_devno = 0;//vt device number
static struct cdev vivt_cdev;
#endif


/*vt init work width and length*/
#define VT_CAP_FRAME_DEFAULT_WIDTH	1920
#define VT_CAP_FRAME_DEFAULT_LENGTH	1080

/*vt init work width and length*/
#define VT_CAP_FRAME_WIDTH_2K1k		1920
#define VT_CAP_FRAME_HEIGHT_2K1k	1080

#define VT_CAP_FRAME_WIDTH_4K2K		3840
#define VT_CAP_FRAME_HEIGHT_4K2K	2160

#define VT_SOURCE_CAP_I3DDMA_BUF	(0x0A)
#define VT_SOURCE_CAP_VDEC_BUF		(0xA0)

#define VT_FPS_OUTPUT 	(60)
#define MAX_CAP_BUF_NUM 5

#define ABNORMAL_DVS_FIRST_HAPPEN 1
#define WAIT_DVS_STABLE_COUNT 8
#define VT_SEQCAP_BURSTLEN 512 //32*128/8
#define VT_BLKCAP_BURSTLEN 512 //64*8byte

KADP_VT_VIDEO_FRAME_OUTPUT_DEVICE_STATE_INFO_T VFODState = {FALSE,FALSE,TRUE,1};

static KADP_VT_DUMP_LOCATION_TYPE_T VTDumpLocation = KADP_VT_MAX_LOCATION;
static VT_CUR_CAPTURE_INFO curCapInfo;
extern unsigned char get_vsc_mutestatus(void);
extern unsigned char drvif_scaler_get_abnormal_dvs_long_flag(void);
extern unsigned char drvif_scaler_get_abnormal_dvs_short_flag(void);
static unsigned char VtFunction = FALSE;
static unsigned char VtEnableFRCMode = TRUE;

static unsigned int VtBufferNum = MAX_CAP_BUF_NUM;
static unsigned char VtSwBufferMode = FALSE;
static unsigned char VtCaptureVDC = FALSE;

static unsigned int vt_cap_frame_max_width = VT_CAP_FRAME_WIDTH_2K1k; 
static unsigned int vt_cap_frame_max_height = VT_CAP_FRAME_HEIGHT_2K1k;

static unsigned long VtAllocatedBufferStartAdress[5] = {0};
static unsigned char vt_src_capture_enable_flag = FALSE; /*record vdec/i3ddma source capture in vt */
static int write_Index = 0;
static unsigned char vdec_buf_cap_debug = FALSE;
static unsigned char i3ddma_buf_cap_debug = FALSE;

static unsigned int abnormal_dvs_cnt = 0;
static unsigned int wait_dvs_stable_cnt = 0;
static unsigned char vt_StreamOn_flag = FALSE;

int vt_src_capture_done_flag[3] = {0,0,0}; //means 3 buffers capture done status
int vt_qbuf_flag[3] = {0,0,0};

unsigned int width_src = 0;
unsigned int height_src = 0;

static unsigned char VdecSecureStatus = FALSE;
static unsigned char DtvSecureStatus = FALSE;

/*ARGB8888_FORMAT for roku
  RGB888 for LGE*/
static VT_CAP_FMT VT_Pixel_Format = VT_CAP_RGB888;
static unsigned int uiTargetFps = 0;
extern KADP_VIDEO_RECT_T ap_main_inregion_parm;//for new input output. main path ap parameter  from new hal
extern KADP_VIDEO_RECT_T ap_main_originalInput_parm;//for new input output. main path ap input resolution parameter from new hal

#ifdef CONFIG_ENABLE_HDMI_NN
extern unsigned int g_ulNNOutWidth; 
extern unsigned int g_ulNNOutLength; 
#endif 

unsigned char dqbuf_ioctl_fail_flag = FALSE;
unsigned char IndexOfFreezedVideoFrameBuffer = 0;
volatile unsigned int vfod_capture_out_W = VT_CAP_FRAME_DEFAULT_WIDTH;
volatile unsigned int vfod_capture_out_H = VT_CAP_FRAME_DEFAULT_LENGTH;
volatile unsigned int vfod_capture_location = KADP_VT_DISPLAY_OUTPUT;

/*------include extern functions----------*/
extern unsigned char force_enable_two_step_uzu(void);/* get d domain go two pixel mode? */
unsigned char debug_dump_data_to_file(unsigned int idx, unsigned int dump_w,unsigned int dump_h);

extern void MEMC_Set_malloc_address(UINT8 status);
extern void drvif_vt_block_vr_recording(unsigned char vrEnableFlag);
extern unsigned char get_vr_function(void);
extern void drvif_vt_reconfig_vr_recording(void);
#ifdef CONFIG_ENABLE_HDMI_NN
extern int h3ddma_get_NN_read_buffer(unsigned int *a_pulYAddr, unsigned int *a_pulCAddr, unsigned long long *uzd_timestamp, unsigned int *a_pulCropYAddr, unsigned int *a_pulCropCAddr, unsigned long long *crop_timestamp);
extern void h3ddma_get_NN_output_size(unsigned int *outputWidth, unsigned int *outputLength);
#endif

unsigned char Capture_BufferMemInit_VT(unsigned int buffernum);
void Capture_BufferMemDeInit_VT(unsigned int buffernum);
unsigned char HAL_VT_Finalize(void);
void drvif_DC2H_dispD_CaptureConfig(VT_CUR_CAPTURE_INFO capInfo);
void set_cap_buffer_size_by_AP(unsigned int usr_width, unsigned int usr_height);
void reset_dc2h_hw_setting(void);
unsigned char HAL_VT_GetVideoFrameBufferIndex(KADP_VT_VIDEO_WINDOW_TYPE_T videoWindowID, unsigned int *pIndexOfCurrentVideoFrameBuffer);
void drvif_memc_outBg_ctrl(bool enable);
//void set_DC2H_rgb2yuv_ctrl_reg_refer_dump_location(KADP_VT_DUMP_LOCATION_TYPE_T in_position, SIZE *in_size, unsigned char *dc2h_in_sel);
void set_DC2H_cap_boundaryaddr(unsigned int VtBufferNum);
unsigned long vt_bufAddr_align(unsigned int val);

_RPC_system_setting_info* scaler_GetShare_Memory_RPC_system_setting_info_Struct(void);
void drvif_i3ddma_buf_SE_capture(void);
unsigned char drvif_do_se_stretch(SE_SRC_DST_info info);
unsigned char do_vt_source_capture_proc(void);
void do_vt_vdec_srcbuf_capture(void);
unsigned char do_vt_capture_streamon(void);
unsigned char do_vt_reqbufs(unsigned int buf_cnt);
unsigned char do_vt_streamoff(void);
unsigned char do_vt_dqbuf(unsigned int *pdqbuf_Index);
unsigned short Get_VFOD_FrameRate(void);
void vt_enable_dc2h(unsigned char state);
unsigned char get_dc2h_capture_state(void);
extern unsigned char get_MEMC_bypass_status_refer_platform_model(void);

#define ENABLE_VT_TEST_CASE_CONFIG
#ifdef ENABLE_VT_TEST_CASE_CONFIG
typedef enum _VT_TEST_CASE{
	VT_TEST_CASE_DISABLE = 0,
	VT_TEST_SINGLE_CAPTURE_CASE,
	VT_TEST_ACR_CASE,
	VT_TEST_VR360_CASE,
	VT_TEST_OVERSCAN_CASE,
	VT_TEST_MAX_CASE,
}VT_TEST_CASE_T;

VT_TEST_CASE_T vt_test_state = VT_TEST_CASE_DISABLE;

void set_vt_test_case(VT_TEST_CASE_T enable)
{
	vt_test_state = enable;
}
VT_TEST_CASE_T get_vt_test_case(void)
{
	return vt_test_state;
}

void handle_vt_test_case(void);
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
EXPORT_SYMBOL(get_vt_EnableFRCMode);

void set_vt_EnableFRCMode(unsigned char value)
{
	VtEnableFRCMode = value;
}

void set_dqbuf_ioctl_fail_flag(unsigned char flg)
{
	dqbuf_ioctl_fail_flag = flg;
}
unsigned char get_dqbuf_ioctl_fail_flag(void)
{
	return dqbuf_ioctl_fail_flag;
}

void set_vt_StreamOn_flag(unsigned char flg)
{	
	vt_StreamOn_flag = flg;
}
unsigned char get_vt_StreamOn_flag(void)
{
	return vt_StreamOn_flag;
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

unsigned char get_vt_src_cap_status(void)
{
	return vt_src_capture_enable_flag;
}
unsigned int get_vt_capBuf_Addr(unsigned int idx)
{
    return CaptureCtrl_VT.cap_buffer[idx].phyaddr;
}

void set_vt_src_cap_status(unsigned char value)
{
	vt_src_capture_enable_flag = value;
}
void debug_vdec_cap_enable(unsigned char value)
{
	vdec_buf_cap_debug = value;
}
void debug_i3ddma_cap_enable(unsigned char value)
{
	i3ddma_buf_cap_debug = value;
}
unsigned char get_src_cap_enable_status(void)
{
	if(vdec_buf_cap_debug || i3ddma_buf_cap_debug)
		return TRUE;
	else
		return FALSE;
}
void reset_debug_src_cap_enable_status(void)
{
	vdec_buf_cap_debug = FALSE;
	i3ddma_buf_cap_debug = FALSE;
}
void set_vt_target_fps(unsigned int val)
{
    uiTargetFps = val;
}
unsigned int get_vt_target_fps(void)
{
    return uiTargetFps;
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
    _Idomain_Sdnr_input,
    _VD_OUTPUT
}DC2H_IN_SEL;

typedef struct
{
    unsigned int reserved:18;
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
} DC2H_HANDLER ;


DC2H_HANDLER *pdc2h_hdl = NULL;
static DC2H_HANDLER dc2h_hdl;

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

#ifndef UT_flag
void vt_enable_dc2h(unsigned char state)
{
	//down(&VT_Semaphore);
	curCapInfo.enable = state;
	drvif_DC2H_dispD_CaptureConfig(curCapInfo);
	//up(&VT_Semaphore);
}
#endif
unsigned char get_dc2h_capture_state(void)
{
	if ((get_VT_Pixel_Format() == VT_CAP_NV12) || (get_VT_Pixel_Format() == VT_CAP_NV16)||(get_VT_Pixel_Format() == VT_CAP_NV21))
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
	//unsigned int ulCount = 0;
	int ret;
    int i;
	rtd_pr_vt_notice("fun:%s\n",__FUNCTION__);
	swmode_infoptr = (DC2H_SWMODE_STRUCT_T *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_DC2H_SWMODE_ENABLE);
	//ulCount = sizeof(DC2H_SWMODE_STRUCT_T) / sizeof(unsigned int);

	swmode_infoptr->SwModeEnable = onoff;
	swmode_infoptr->buffernumber = buffernumber;
	swmode_infoptr->cap_format = (UINT32)get_VT_Pixel_Format();
	swmode_infoptr->cap_width = vt_cap_frame_max_width;
	swmode_infoptr->cap_length = vt_cap_frame_max_height;
	
    if(get_VT_Pixel_Format() == VT_CAP_NV12 || get_VT_Pixel_Format() == VT_CAP_NV16 || get_VT_Pixel_Format() == VT_CAP_NV21)
    {
        swmode_infoptr->YbufferSize = vt_bufAddr_align(vt_cap_frame_max_width*vt_cap_frame_max_height);
    }
    else if(get_VT_Pixel_Format() == VT_CAP_RGB888 || get_VT_Pixel_Format() == VT_CAP_ARGB8888 || get_VT_Pixel_Format() == VT_CAP_RGBA8888 || get_VT_Pixel_Format() == VT_CAP_ABGR8888)
    {
        swmode_infoptr->YbufferSize = CaptureCtrl_VT.cap_buffer[0].size;
    }
    else
    {
        rtd_pr_vt_emerg("fun:%s,unknow pixel format\n",__FUNCTION__);
        return FALSE;
    }

	if(Get_PANEL_VFLIP_ENABLE())
	{
        if((get_VT_Pixel_Format() == VT_CAP_NV12)||(get_VT_Pixel_Format() == VT_CAP_NV16)||(get_VT_Pixel_Format() == VT_CAP_NV21))
        {
    		swmode_infoptr->cap_buffer[0] = ((UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr + (vt_cap_frame_max_width*vt_cap_frame_max_height - vt_cap_frame_max_width));
    		swmode_infoptr->cap_buffer[1] = ((UINT32)CaptureCtrl_VT.cap_buffer[1].phyaddr + (vt_cap_frame_max_width*vt_cap_frame_max_height - vt_cap_frame_max_width));
    		swmode_infoptr->cap_buffer[2] = ((UINT32)CaptureCtrl_VT.cap_buffer[2].phyaddr + (vt_cap_frame_max_width*vt_cap_frame_max_height - vt_cap_frame_max_width));
    		swmode_infoptr->cap_buffer[3] = ((UINT32)CaptureCtrl_VT.cap_buffer[3].phyaddr + (vt_cap_frame_max_width*vt_cap_frame_max_height - vt_cap_frame_max_width));
    		swmode_infoptr->cap_buffer[4] = ((UINT32)CaptureCtrl_VT.cap_buffer[4].phyaddr + (vt_cap_frame_max_width*vt_cap_frame_max_height - vt_cap_frame_max_width));
        }
        else if(get_VT_Pixel_Format() == VT_CAP_RGB888)
        {
            for (i = 0; i < MAX_CAP_BUF_NUM; i++)
                swmode_infoptr->cap_buffer[i] = (((UINT32)CaptureCtrl_VT.cap_buffer[i].phyaddr) ? ((UINT32)CaptureCtrl_VT.cap_buffer[i].phyaddr + vt_cap_frame_max_width*vt_cap_frame_max_height*3 - vt_cap_frame_max_width*3) : (0));
        }
        else if(get_VT_Pixel_Format() == VT_CAP_ARGB8888 || get_VT_Pixel_Format() == VT_CAP_RGBA8888 || get_VT_Pixel_Format() == VT_CAP_ABGR8888)
        {
            for (i = 0; i < MAX_CAP_BUF_NUM; i++)
                swmode_infoptr->cap_buffer[i] = (((UINT32)CaptureCtrl_VT.cap_buffer[i].phyaddr) ? ((UINT32)CaptureCtrl_VT.cap_buffer[i].phyaddr + vt_cap_frame_max_width*vt_cap_frame_max_height*4 - vt_cap_frame_max_width*4) : (0));
        }
        else
        {
            rtd_pr_vt_notice("fun:%s,unknow pixel format\n",__FUNCTION__);
            return FALSE;
        }
	}
	else
	{
		swmode_infoptr->cap_buffer[0] = (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;
		swmode_infoptr->cap_buffer[1] = (UINT32)CaptureCtrl_VT.cap_buffer[1].phyaddr;
		swmode_infoptr->cap_buffer[2] = (UINT32)CaptureCtrl_VT.cap_buffer[2].phyaddr;
		swmode_infoptr->cap_buffer[3] = (UINT32)CaptureCtrl_VT.cap_buffer[3].phyaddr;
		swmode_infoptr->cap_buffer[4] = (UINT32)CaptureCtrl_VT.cap_buffer[4].phyaddr;
	}

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

unsigned char drvif_dc2h_se_inforpc(unsigned int capwidth,unsigned int caplength)
{
	VT_SE_CAPTURE_INFO_T *se_capture_infoptr = NULL;
	//unsigned int ulCount = 0;
	int ret;

	rtd_pr_vt_notice("fun:%s=%d\n",__FUNCTION__, __LINE__);

	curCapInfo.capWid = capwidth;
	curCapInfo.capLen = caplength;

	se_capture_infoptr = (VT_SE_CAPTURE_INFO_T *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_VT_SE_CAPTURE_ENABLE);
	//ulCount = sizeof(VT_SE_CAPTURE_INFO_T) / sizeof(unsigned int);
	memset(se_capture_infoptr, 0, sizeof(VT_SE_CAPTURE_INFO_T));
	
	se_capture_infoptr->capWid = capwidth;
	
	se_capture_infoptr->capLen = caplength;

	//change endian
	se_capture_infoptr->capWid = htonl(se_capture_infoptr->capWid);
	
	se_capture_infoptr->capLen = htonl(se_capture_infoptr->capLen);

	if (0 != (ret = Scaler_SendRPC(SCALERIOC_SET_VT_SE_CAPTURE_ENABLE,0,0)))
	{
		rtd_pr_vt_emerg("[dc2h]ret=%d, SCALERIOC_SET_DC2H_SE_CAPTURE_ENABLE RPC fail !!!\n", ret);
		return FALSE;
	}
	return TRUE;
}

unsigned char h3ddma_get_cap_enable_mask(void);
void drvif_i3ddma_buf_SE_capture(void)
{
	/* VT use i3ddma NN buffer to do stretch size required by user client */
#ifdef CONFIG_ENABLE_HDMI_NN
	SE_SRC_DST_info seinfo;
	unsigned int vt_buf_count = 0;
	
	//unsigned int s_time = 0;
	//unsigned int end_time = 100;
	
	seinfo.src_x = 0;
	seinfo.src_y = 0;
	h3ddma_get_NN_output_size(&(seinfo.src_w), &(seinfo.src_h));
	seinfo.src_fmt = KGAL_PIXEL_FORMAT_NV12;
	seinfo.src_pitch_y = seinfo.src_w;

	if(h3ddma_get_cap_enable_mask() == 0)
	{
		//rtd_pr_vt_notice("[Error] i3dma NN disable\n");
		return;
	}
	
	if(seinfo.src_w == 0 || seinfo.src_h == 0)
	{
		rtd_pr_vt_notice("[Error] i3ddma size 0\n");
		return;
	}
	rtd_pr_vt_debug("i3ddma buf size:<w,h>:<%d,%d>\n", seinfo.src_w, seinfo.src_h);
	
	// get i3ddma NN buffer address 
	if(h3ddma_get_NN_read_buffer(&(seinfo.src_phyaddr), &(seinfo.src_phyaddr_uv), NULL, NULL, NULL, NULL) < 0)
	{
		rtd_pr_vt_notice("get i3ddma buf fail\n");
		return;
	}

	rtd_pr_vt_debug("i3ddma buf addr:<y,uv>:<0x%x,0x%x>\n", seinfo.src_phyaddr, seinfo.src_phyaddr_uv);
							
	vt_buf_count = get_vt_VtBufferNum();

	seinfo.dst_x = 0;
	seinfo.dst_y = 0;
	seinfo.dst_w = vfod_capture_out_W;
	seinfo.dst_h = vfod_capture_out_H;
	
	seinfo.dst_phyaddr = 0;
	seinfo.dst_phyaddr_uv = 0;
	seinfo.dst_fmt = KGAL_PIXEL_FORMAT_NV12;

	/* ====== single buffer capture or ACR capture from i3ddma nn buffer ===========*/

	if(vt_buf_count == 1) 
	{
		write_Index = 0;
		
		if(vt_qbuf_flag[write_Index] == 0) //ap client is using the write_Index, can't write this buffer
		{
			rtd_pr_vt_notice("Ap is using index:%d,can't write\n", write_Index);
			return;
		}
		else  //loop overwrite
		{
			seinfo.dst_phyaddr = CaptureCtrl_VT.cap_buffer[write_Index].phyaddr;
			seinfo.dst_phyaddr_uv = (CaptureCtrl_VT.cap_buffer[write_Index].phyaddr + vt_bufAddr_align(vt_cap_frame_max_width*vt_cap_frame_max_height));
			if(seinfo.dst_phyaddr && seinfo.dst_phyaddr_uv)
			{
				//s_time = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg)/90;
				drvif_do_se_stretch(seinfo);
				//end_time = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg)/90;

				vt_src_capture_done_flag[write_Index] = 1; // write_Index write done flag
				//rtd_pr_vt_notice("src cap se time=%d ms\n",(end_time-s_time));
			}
		}
		
	}
	else
	{/* ====== buf count > 1 ,ex: ACR ===========*/
				/* =======loop wite to buffer ============*/
		if(vt_buf_count > 1)
		{
			write_Index = ((write_Index + 1) % vt_buf_count);
			// write_Index means to write buffer index, it is possible 0,1,2,...,(vt_buf_count-1)
			rtd_pr_vt_notice("write_Index_111:%d\n", write_Index);
			
			// check webos enable capture data to buffer write_Index, if webos is using, should re-check next buffer
			if(vt_qbuf_flag[write_Index] == 0) //ap client is using the write_Index, can't write this buffer
			{

				//buffer index re-calculate 
				write_Index = ((write_Index + 1) % vt_buf_count);
				rtd_pr_vt_notice("write_Index_222:%d\n", write_Index);
				if(vt_qbuf_flag[write_Index] == 0)
				{
					rtd_pr_vt_emerg("line:%d,error ap client using two buffers\n", __LINE__);
					return;
				}
			}

			seinfo.dst_phyaddr = CaptureCtrl_VT.cap_buffer[write_Index].phyaddr;
			seinfo.dst_phyaddr_uv = (CaptureCtrl_VT.cap_buffer[write_Index].phyaddr + vt_bufAddr_align(vt_cap_frame_max_width*vt_cap_frame_max_height));
			if(seinfo.dst_phyaddr && seinfo.dst_phyaddr_uv)
			{
				//s_time = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg)/90;
				drvif_do_se_stretch(seinfo);
				//end_time = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg)/90;

				vt_src_capture_done_flag[write_Index] = 1; // write_Index write done flag
				//rtd_pr_vt_notice("src cap se time=%d ms\n",(end_time-s_time));
			}
		}
	}
#else
	rtd_pr_vt_notice("i3ddma NN buffer not exist! error!\n");
#endif

}

unsigned char drvif_do_se_stretch(SE_SRC_DST_info info)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_SE)
	//unsigned int se_sta, se_end;
	bool status = 0;
	KGAL_SURFACE_INFO_T ssurf;
	KGAL_SURFACE_INFO_T dsurf;
	KGAL_RECT_T srect;
	KGAL_RECT_T drect;
	KGAL_BLIT_FLAGS_T sflag = KGAL_BLIT_NOFX;
	KGAL_BLIT_SETTINGS_T sblend;
	memset(&ssurf,0, sizeof(KGAL_SURFACE_INFO_T));
	memset(&dsurf,0, sizeof(KGAL_SURFACE_INFO_T));
	memset(&srect,0, sizeof(KGAL_RECT_T));
	memset(&drect,0, sizeof(KGAL_RECT_T));
	memset(&sblend,0, sizeof(KGAL_BLIT_SETTINGS_T));
	sblend.srcBlend = KGAL_BLEND_ONE;
	sblend.dstBlend = KGAL_BLEND_ZERO;

	ssurf.physicalAddress = info.src_phyaddr;
	ssurf.width = info.src_w;
	ssurf.height = info.src_h;
	ssurf.pixelFormat 		= info.src_fmt;
	srect.x = info.src_x;
	srect.y = info.src_y;
	srect.w = info.src_w;
	srect.h = info.src_h;

	dsurf.physicalAddress = info.dst_phyaddr;
	dsurf.width = info.dst_w;
	dsurf.height = info.dst_h;
	dsurf.pixelFormat 		= info.dst_fmt;
	drect.x = info.dst_x;
	drect.y = info.dst_y;
	drect.w = info.dst_w;
	drect.h = info.dst_h;

	if(info.src_fmt == KGAL_PIXEL_FORMAT_NV12)
	{
		ssurf.bpp 	= 16;
		ssurf.pitch = info.src_pitch_y;//info.src_w;
	}
	else if(info.src_fmt == KGAL_PIXEL_FORMAT_RGB888)//KGAL_PIXEL_FORMAT_YUV444
	{
		ssurf.bpp 	= 24;
		ssurf.pitch = info.src_pitch_y*3;//info.src_w*3;
		info.src_phyaddr_uv = info.dst_phyaddr_uv; //just init, no use in SE driver.
	}

	if(info.dst_fmt == KGAL_PIXEL_FORMAT_NV12)
	{
		dsurf.bpp 	= 16;
		dsurf.pitch = info.dst_w;
	}
	else if(info.dst_fmt == KGAL_PIXEL_FORMAT_RGB888)
	{
		dsurf.bpp 	= 24;
		dsurf.pitch = info.dst_w*3;
	}

	//se_sta = rtd_inl(TIMER_SCPU_CLK90K_LO_reg);
	status = KGAL_NV12_StretchBlit(&ssurf, &srect, &dsurf, &drect, &sflag, &sblend, info.src_phyaddr_uv, info.dst_phyaddr_uv);
	//se_end = rtd_inl(TIMER_SCPU_CLK90K_LO_reg);

	if(!status)
	{
		rtd_pr_vt_notice("[error]KGAL_NV12_StretchBlit run fail\n");
		return FALSE;
	}
	else
	{
		//rtd_pr_vt_notice("src(%d,%d) => dst(%d,%d)\n",ssurf.width, ssurf.height, dsurf.width, dsurf.height);
		return TRUE;
	}

#else
	return TRUE;
#endif
}

#ifndef UT_flag
unsigned char get_video_do_overscan_state(void)
{

#if 1
	if((ap_main_inregion_parm.w != ap_main_originalInput_parm.w) || (ap_main_inregion_parm.h != ap_main_originalInput_parm.h))
	{
		return TRUE;
	}
	else
		return FALSE;
#else
	return TRUE;
#endif
}

DC2H_Enable_rgb2yuv vt_colorformattransfer_enable(DC2H_IN_SEL dc2h_in_sel)
{
    DC2H_Enable_rgb2yuv dc2h_enrgb2yuv_state = DC2H_Colconv_DISABLE;

    if((get_VT_Pixel_Format() == VT_CAP_NV12)||(get_VT_Pixel_Format() == VT_CAP_NV16)||(get_VT_Pixel_Format() == VT_CAP_NV21))
    {
        if((dc2h_in_sel == _MAIN_UZU) || (dc2h_in_sel == _TWO_SECOND_UZU) || (dc2h_in_sel == _Sub_UZU_Output))  //YUV format
        {
            dc2h_enrgb2yuv_state = DC2H_Colconv_DISABLE;
        }
        else if(dc2h_in_sel == _Idomain_Sdnr_input)
        {
            if((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_COLOR_SPACE) == VODMA_COLOR_RGB) && (RGB2YUV_ICH1_RGB2YUV_CTRL_get_en_rgb2yuv(RGB2YUV_ICH1_RGB2YUV_CTRL_reg) == 0))
            {
                dc2h_enrgb2yuv_state = DC2H_Colconv_RGBtoYUV;
            }else{
                dc2h_enrgb2yuv_state = DC2H_Colconv_DISABLE;
            }
        }
        else{
            dc2h_enrgb2yuv_state = DC2H_Colconv_RGBtoYUV; //Enable RGB to YUV conversion (YUV out)
        }
    }
    else
    {
        if((dc2h_in_sel == _MAIN_UZU) || (dc2h_in_sel == _TWO_SECOND_UZU) || (dc2h_in_sel == _Sub_UZU_Output))
        {
            dc2h_enrgb2yuv_state = DC2H_Colconv_YUVtoRGB; //Enable YUV to RGB conversion (GBR out)
        }
        else if(dc2h_in_sel == _Idomain_Sdnr_input)
        {
            if((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_COLOR_SPACE) == VODMA_COLOR_RGB) && (RGB2YUV_ICH1_RGB2YUV_CTRL_get_en_rgb2yuv(RGB2YUV_ICH1_RGB2YUV_CTRL_reg) == 0))
            {
                dc2h_enrgb2yuv_state = DC2H_Colconv_DISABLE;
            }else{
                dc2h_enrgb2yuv_state = DC2H_Colconv_YUVtoRGB;
            }
        }
        else{
            dc2h_enrgb2yuv_state = DC2H_Colconv_DISABLE;
        }
    }
    rtd_pr_vt_notice("[VT]get_VT_Pixel_Format()=%x, dc2h_in_sel=%x, dc2h_en_rgb2yuv=%d\n",get_VT_Pixel_Format(), dc2h_in_sel,dc2h_enrgb2yuv_state);
    return dc2h_enrgb2yuv_state;

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

	if((get_VT_Pixel_Format() == VT_CAP_NV12)||(get_VT_Pixel_Format() == VT_CAP_NV16)||(get_VT_Pixel_Format() == VT_CAP_NV21))
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

	dc2h_rgb2yuv_ctrl_reg.en_rgb2yuv = vt_colorformattransfer_enable(dc2h_in_sel);

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

		if(SDRatio <= ((TMPMUL*3)/2))  //<1.5 sharp, wider bw
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
		Vini = 0xff;//0x78;
		dc2h_vsd_initial_value_reg.ver_ini = Vini;
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
	if((get_VT_Pixel_Format() == VT_CAP_NV12)||(get_VT_Pixel_Format() == VT_CAP_NV16)||(get_VT_Pixel_Format() == VT_CAP_NV21)){
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
	dc2h_rgb2yuv_dc2h_3dmaskto2d_ctrl_Reg.dc2h_3dmaskto2d_en = 0; /*3dmaskto2d function all reserved*/
	rtd_outl(DC2H_RGB2YUV_DC2H_3DMaskTo2D_Ctrl_reg, dc2h_rgb2yuv_dc2h_3dmaskto2d_ctrl_Reg.regValue);
}

void drvif_vi_config(unsigned int nWidth, unsigned int nLength, unsigned int data_format)
{
#define LINE_STEP_MAX 0x1FFFFFFF
#define DC2H_DMA_VI_FIFO_SIZE 1024
	unsigned int LineSize;
	unsigned int width_16bytes_align = 0;  //128bits align
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
    dc2h_vi_dc2h_y1obuff_RBUS dc2h_vi_dc2h_y1obuff_Reg;
    dc2h_vi_dc2h_c1obuff_RBUS dc2h_vi_dc2h_c1obuff_Reg;

    dc2h_vi_dc2h_y1obuff_Reg.regValue = IoReg_Read32(DC2H_VI_DC2H_Y1OBUFF_reg);
    dc2h_vi_dc2h_y1obuff_Reg.y_buf_start = 0;
    dc2h_vi_dc2h_y1obuff_Reg.y_buf_end = (DC2H_DMA_VI_FIFO_SIZE/2 - 1);
    IoReg_Write32(DC2H_VI_DC2H_Y1OBUFF_reg, dc2h_vi_dc2h_y1obuff_Reg.regValue);

    dc2h_vi_dc2h_c1obuff_Reg.regValue = IoReg_Read32(DC2H_VI_DC2H_C1OBUFF_reg);
    dc2h_vi_dc2h_c1obuff_Reg.c_buf_start = DC2H_DMA_VI_FIFO_SIZE/2;
    dc2h_vi_dc2h_c1obuff_Reg.c_buf_end = (DC2H_DMA_VI_FIFO_SIZE - 1);
    IoReg_Write32(DC2H_VI_DC2H_C1OBUFF_reg, dc2h_vi_dc2h_c1obuff_Reg.regValue);

    dc2h_v1_output_fmt_reg.regValue = rtd_inl(DC2H_VI_DC2H_V1_OUTPUT_FMT_reg);
	dc2h_v1_output_fmt_reg.data_format = data_format;/*0:420	1:422*/
	if(get_VT_Pixel_Format() == VT_CAP_NV12 || get_VT_Pixel_Format() == VT_CAP_NV16)
	{
		dc2h_v1_output_fmt_reg.swap_1byte = 0;
		dc2h_v1_output_fmt_reg.swap_2byte = 0;
		dc2h_v1_output_fmt_reg.swap_4byte = 0;
		dc2h_v1_output_fmt_reg.swap_8byte = 0;

	}else if(get_VT_Pixel_Format() == VT_CAP_NV21){
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
	/*if((TRUE == force_enable_two_step_uzu()) && (is_4K2K_capture() == TRUE))
		LineSize = VT_CAP_FRAME_WIDTH_4K2K * 8 / 64;
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

	if (get_vt_VtSwBufferMode()) {
		dc2h_dma_dc2h_seq_interrupt_enable_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_seq_Interrupt_Enable_reg);
		dc2h_dma_dc2h_seq_interrupt_enable_Reg.vs_rising_ie = 1;
	}

	dc2h_wait_porch();

	vi_dmactl_reg.regValue = rtd_inl(DC2H_VI_DC2H_DMACTL_reg);
	vi_dmactl_reg.dmaen1 = 1;
	vi_dmactl_reg.seq_blk_sel =1; //0: YV16 or RGB888 1: NV12 or NV16
	vi_dmactl_reg.bstlen = 6;//recommend 64 (8byte unit)
	rtd_outl(DC2H_VI_DC2H_DMACTL_reg, vi_dmactl_reg.regValue);

	dc2h_vi_dc2h_y1buf_Reg.regValue = rtd_inl(DC2H_VI_DC2H_Y1BUF_reg);
	dc2h_vi_dc2h_y1buf_Reg.y_water_lvl = 64; //recommend 64 (8byte unit)
	rtd_outl(DC2H_VI_DC2H_Y1BUF_reg, dc2h_vi_dc2h_y1buf_Reg.regValue);

	dc2h_vi_dc2h_c1buf_Reg.regValue = rtd_inl(DC2H_VI_DC2H_C1BUF_reg);
	dc2h_vi_dc2h_c1buf_Reg.c_water_lvl = 64; //recommend 64 (8byte unit)
	rtd_outl(DC2H_VI_DC2H_C1BUF_reg, dc2h_vi_dc2h_c1buf_Reg.regValue);


	if (get_vt_VtSwBufferMode()) {
		rtd_outl(DC2H_DMA_dc2h_seq_Interrupt_Enable_reg, dc2h_dma_dc2h_seq_interrupt_enable_Reg.regValue);
	}
}

void drvif_dc2h_seq_config(DC2H_HANDLER *pdc2h_hdl)
{
#define LINE_STEP_MAX 0x1FFFFFFF

	unsigned int num, rem;
	unsigned int lineunits, len;
	unsigned int seq_vflip_lineunits;//Vflip enable for Seq_Mode, use the vi_line_step
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
    dc2h_vi_dc2h_vi_addcmd_transform_RBUS dc2h_vi_dc2h_vi_addcmd_transform_reg;

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

	if(get_VT_Pixel_Format() == VT_CAP_ARGB8888 || get_VT_Pixel_Format() == VT_CAP_RGBA8888 || get_VT_Pixel_Format() == VT_CAP_ABGR8888) {
		width96align = drvif_memory_get_data_align(pdc2h_hdl->out_size.nWidth*4, 96);
	} else {
		width96align = drvif_memory_get_data_align(pdc2h_hdl->out_size.nWidth*3, 96);
	}
	lineunits = (width96align * 8) /128;
	seq_vflip_lineunits = (width96align * 8) /64;


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

	if((get_VT_Pixel_Format() == VT_CAP_ABGR8888) || (get_VT_Pixel_Format() == VT_CAP_ARGB8888) || (get_VT_Pixel_Format() == VT_CAP_RGBA8888)){
		dc2h_seq_mode_ctrl1_reg.argb_dummy_data = 0xFF;
		dc2h_seq_mode_ctrl1_reg.rgb_to_argb_en = 1;
		dc2h_seq_mode_ctrl1_reg.argb_swap = 1;
        if(get_VT_Pixel_Format() == VT_CAP_RGBA8888)
            dc2h_seq_mode_ctrl1_reg.argb_swap = 0;
	}
	else
		dc2h_seq_mode_ctrl1_reg.rgb_to_argb_en = 0;
    
    if(Get_PANEL_VFLIP_ENABLE())
    {
        dc2h_seq_mode_ctrl1_reg.vflip_en = 1;//seq_mode vflip,use vi_line_step @8029D74
        dc2h_vi_dc2h_vi_addcmd_transform_reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_addcmd_transform_reg);
        dc2h_vi_dc2h_vi_addcmd_transform_reg.line_step = (LINE_STEP_MAX - seq_vflip_lineunits + 1); //2's complement of line step,means nagative line step
        rtd_outl(DC2H_VI_DC2H_vi_addcmd_transform_reg, dc2h_vi_dc2h_vi_addcmd_transform_reg.regValue);
    }
    else
    {
        dc2h_seq_mode_ctrl1_reg.vflip_en = 0;
    }
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

unsigned char do_vt_source_capture_proc(void)
{
	/* if video has done overscan in scaler output 
	 * 
	 * source type:
	 * DTV/CP/Playback ==> vdec NN buffer ==> SE stretch
	 * HDMI/AV/ATV/Component ==> i3ddma NN buffer ==> SE stretch
	 *
	*/
	VSC_INPUT_TYPE_T srctype = VSC_INPUTSRC_MAXN;
	unsigned char vt_src_cap_status = FALSE;
	
	    /* first: stop dc2h */
	if(get_dc2h_capture_state() == TRUE)
	{		
		vt_enable_dc2h(FALSE); // stop dc2h
	}

	//rtd_pr_vt_notice("fun:%s\n", __FUNCTION__);
	srctype = Get_DisplayMode_Src(SLR_MAIN_DISPLAY);
	//rtd_pr_vt_notice("source type:%d\n",srctype);
	
	if((get_VT_Pixel_Format() == VT_CAP_NV12) || (get_VT_Pixel_Format() == VT_CAP_NV16)||(get_VT_Pixel_Format() == VT_CAP_NV21))
	{
						
		if(((srctype == VSC_INPUTSRC_HDMI) && (get_hdmi_4k_hfr_mode() == HDMI_NON_4K120))
		|| (srctype == VSC_INPUTSRC_ADC) || (srctype == VSC_INPUTSRC_AVD))
		{
			
			vt_src_cap_status = VT_SOURCE_CAP_I3DDMA_BUF;
			//rtd_pr_vt_notice("enable i3ddma src capture flag\n");
		}
		else if((srctype == VSC_INPUTSRC_VDEC) || (srctype == VSC_INPUTSRC_JPEG))
		{
			/* DTV/CP/Playback ==> vdec NN buffer ==> SE stretch  */
			vt_src_cap_status = VT_SOURCE_CAP_VDEC_BUF;
			//rtd_pr_vt_notice(" enable vdec src capture flag\n");
		}
		else
		{
			vt_src_cap_status = FALSE;
			rtd_pr_vt_notice("unsupport source type capture\n");
			return FALSE;
		}

		set_vt_src_cap_status(vt_src_cap_status);
		return TRUE;
	}

	rtd_pr_vt_notice("[fail] do_vt_source_capture_proc\n");
	return FALSE;
}

void reset_vt_src_cap_flags(void)
{
	int i;
	for(i = 0; i < 3; i++)
	{
		vt_src_capture_done_flag[i] = 0;
		vt_qbuf_flag[i] = 0;
	}
}
void do_vt_qbuf(unsigned int index)
{
	if((vfod_capture_location == KADP_VT_SCALER_OUTPUT) && (get_video_do_overscan_state() == TRUE)
			&& (get_src_cap_enable_status() == TRUE))
	{
		//current only ACR comes in
		//rtd_pr_vt_notice("[VT]qbuf:index:%d\n", index);
		vt_qbuf_flag[index] = 1; //ap enable buffer index, that vt can write this buffer
	}
}

unsigned int sort_boundary_addr_max_index(void)
{
    unsigned int i,index = 0;
    unsigned int bufnum = get_vt_VtBufferNum();
    unsigned int max = CaptureCtrl_VT.cap_buffer[0].phyaddr;

   if(CaptureCtrl_VT.cap_buffer[0].phyaddr == 0 || bufnum == 1)
    {
        index = 0;
    }
    else
    {
        for( i = 1; i < bufnum; i++)
        {
            if(CaptureCtrl_VT.cap_buffer[i].phyaddr > max)
            {
                max = CaptureCtrl_VT.cap_buffer[i].phyaddr;
                index = i;
            }
        }
    }
    return index;
}

unsigned int sort_boundary_addr_min_index(void)
{
    unsigned int i,index = 0;
    unsigned int bufnum = get_vt_VtBufferNum();
    unsigned int min = CaptureCtrl_VT.cap_buffer[0].phyaddr;

    if(CaptureCtrl_VT.cap_buffer[0].phyaddr == 0 || bufnum == 1)
    {
        index = 0;
    }
    else
    {
        for( i = 1; i < bufnum; i++)
        {
            if(CaptureCtrl_VT.cap_buffer[i].phyaddr < min)
            {
                min = CaptureCtrl_VT.cap_buffer[i].phyaddr;
                index = i;
            }
        }
    }
    return index;
}


void set_dc2h_capture_sdnr_in_clock(unsigned char enable)
{
    scaledown_cts_fifo_gap_RBUS scaledown_cts_fifo_gap_reg;
    scaledown_cts_fifo_gap_reg.regValue = rtd_inl(SCALEDOWN_CTS_FIFO_GAP_reg);
    if(enable == TRUE)
    {
        scaledown_cts_fifo_gap_reg.s1_r_dc2h_clk_en = 1;
    }
    else
    {
        scaledown_cts_fifo_gap_reg.s1_r_dc2h_clk_en = 0;
    }
    rtd_outl(SCALEDOWN_CTS_FIFO_GAP_reg, scaledown_cts_fifo_gap_reg.regValue);
}

void do_vt_vdec_srcbuf_capture(void)
{
	int vt_buf_count = 0;

	//unsigned int s_time = 0;
	//unsigned int end_time = 100;

	_RPC_system_setting_info* RPC_SysInfo = NULL;

	unsigned char vdec_rdPtr = 0;
	unsigned char vedc_status = 0;

	unsigned int in_addr_y_tmp = 0, in_addr_uv_tmp = 0;
	unsigned int in_w_tmp = 0, in_h_tmp = 0, in_pitch_y_tmp = 0;

	SE_SRC_DST_info seinfo;

	//bool status = 0;

	vt_buf_count = get_vt_VtBufferNum();

	RPC_SysInfo = scaler_GetShare_Memory_RPC_system_setting_info_Struct();

	if(RPC_SysInfo == NULL)
	{
		rtd_pr_vt_emerg("[fail]RPC sys info NULL\n");
		return;
	}

	rtd_pr_vt_debug("func:%s\n", __FUNCTION__);

	if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC)
	{
		vdec_rdPtr = RPC_SysInfo->SCPU_ISRIINFO_TO_VCPU.rdPtr;
		vedc_status = RPC_SysInfo->SCPU_ISRIINFO_TO_VCPU.pic[vdec_rdPtr].status;
		in_addr_y_tmp = RPC_SysInfo->SCPU_ISRIINFO_TO_VCPU.pic[vdec_rdPtr].SeqBufAddr_Curr;
		in_addr_uv_tmp = RPC_SysInfo->SCPU_ISRIINFO_TO_VCPU.pic[vdec_rdPtr].SeqBufAddr_Curr_UV;

		in_w_tmp = RPC_SysInfo->SCPU_ISRIINFO_TO_VCPU.pic[vdec_rdPtr].Width;
		in_h_tmp = RPC_SysInfo->SCPU_ISRIINFO_TO_VCPU.pic[vdec_rdPtr].Height;
		in_pitch_y_tmp = RPC_SysInfo->SCPU_ISRIINFO_TO_VCPU.pic[vdec_rdPtr].pitch_y;

		seinfo.src_x = 0;
		seinfo.src_y = 0;
		seinfo.src_w = Scaler_ChangeUINT32Endian(in_w_tmp);
		seinfo.src_h = Scaler_ChangeUINT32Endian(in_h_tmp);
		seinfo.src_pitch_y = Scaler_ChangeUINT32Endian(in_pitch_y_tmp);
		//info.src_pitch_c = Scaler_ChangeUINT32Endian(in_pitch_c_tmp);
		seinfo.src_phyaddr = Scaler_ChangeUINT32Endian(in_addr_y_tmp);
		seinfo.src_phyaddr_uv = Scaler_ChangeUINT32Endian(in_addr_uv_tmp);
		seinfo.src_fmt = KGAL_PIXEL_FORMAT_NV12;
	
		//rtd_pr_vt_notice("[vdec buf info] src_w,src_h,src_phyaddr,src_phyaddr_uv:[%d,%d,%x,%x]\n", seinfo.src_w,seinfo.src_h,seinfo.src_phyaddr,seinfo.src_phyaddr_uv);
		//rtd_pr_vt_notice("src_pitch_y=%d\n", seinfo.src_pitch_y);
		
		seinfo.dst_x = 0;
		seinfo.dst_y = 0;
		seinfo.dst_w = vfod_capture_out_W;
		seinfo.dst_h = vfod_capture_out_H;

		seinfo.dst_phyaddr = 0;
		seinfo.dst_phyaddr_uv = 0;
		seinfo.dst_fmt = KGAL_PIXEL_FORMAT_NV12;

		if(vedc_status == 1) //detect Status
		{
			/* ====== single buffer capture or ACR capture from i3ddma nn buffer ===========*/
			if(vt_buf_count == 1) 
			{
				write_Index = 0;
				if(vt_qbuf_flag[write_Index] == 0) //ap client is using the write_Index, can't write this buffer
				{
					rtd_pr_vt_notice("Ap is using index:%d,can't write\n", write_Index);
					return;
				}
				else
				{
					seinfo.dst_phyaddr = CaptureCtrl_VT.cap_buffer[write_Index].phyaddr;
					seinfo.dst_phyaddr_uv = (CaptureCtrl_VT.cap_buffer[write_Index].phyaddr + vt_bufAddr_align(vt_cap_frame_max_width*vt_cap_frame_max_height));
					if(seinfo.dst_phyaddr && seinfo.dst_phyaddr_uv)
					{
						//s_time = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg)/90;
						drvif_do_se_stretch(seinfo);
						//end_time = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg)/90;

						vt_src_capture_done_flag[write_Index] = 1; // write_Index write done flag
						//rtd_pr_vt_notice("src cap se time=%d ms\n",(end_time-s_time));
					}
					
				}
				
				/* [debug]for dump  vdec buf */
				//debug_dump_data_to_file(src_phyaddr, src_w, src_h);
			}		
			else
			{	/* ====== buf count > 1 ,ex:ACR ===========*/
							/* =======loop wite to buffer ============*/
				if(vt_buf_count > 1)
				{
					write_Index = ((write_Index + 1) % vt_buf_count);
					// write_Index means to write buffer index, it is possible 0,1,2,...,(vt_buf_count-1)
					//rtd_pr_vt_notice("write_Index_111:%d\n", write_Index);

					// check webos enable capture data to buffer write_Index, if webos is using, should re-check next buffer
					if(vt_qbuf_flag[write_Index] == 0) //ap client is using the write_Index, can't write this buffer
					{
						
						//buffer index re-calculate 
						write_Index = ((write_Index + 1) % vt_buf_count);
						//rtd_pr_vt_notice("write_Index_222:%d\n", write_Index);
						if(vt_qbuf_flag[write_Index] == 0)
						{
							rtd_pr_vt_emerg("line:%d,error ap client using two buffers\n", __LINE__);
							return;
						}
					}

					seinfo.dst_phyaddr = CaptureCtrl_VT.cap_buffer[write_Index].phyaddr;
					seinfo.dst_phyaddr_uv = (CaptureCtrl_VT.cap_buffer[write_Index].phyaddr + vt_bufAddr_align(vt_cap_frame_max_width*vt_cap_frame_max_height));
					if(seinfo.dst_phyaddr && seinfo.dst_phyaddr_uv)
					{
						//s_time = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg)/90;
						drvif_do_se_stretch(seinfo);
						//end_time = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg)/90;
						
						vt_src_capture_done_flag[write_Index] = 1; // write_Index write done flag
						//rtd_pr_vt_notice("src cap se time=%d ms\n",(end_time-s_time));
					}

				}

			}
		}
	}
}

//according to the dumplocation (in_position)
// calculate the dc2h input size (in_size) and dc2h in sel bit value (dc2h_in_sel)


void set_DC2H_rgb2yuv_ctrl_reg_refer_dump_location(KADP_VT_DUMP_LOCATION_TYPE_T in_position, SIZE *in_size, unsigned char *dc2h_in_sel)
{
	ppoverlay_uzudtg_control1_RBUS ppoverlay_uzudtg_control1_reg;
	dc2h_rgb2yuv_dc2h_rgb2yuv_ctrl_RBUS  dc2h_rgb2yuv_ctrl_reg;

	ppoverlay_main_den_h_start_end_RBUS main_den_h_start_end_Reg;
	ppoverlay_main_den_v_start_end_RBUS main_den_v_start_end_Reg;
	ppoverlay_main_active_h_start_end_RBUS main_active_h_start_end_Reg;
	ppoverlay_main_active_v_start_end_RBUS main_active_v_start_end_Reg;

	main_den_h_start_end_Reg.regValue = rtd_inl(PPOVERLAY_MAIN_DEN_H_Start_End_reg);
	main_den_v_start_end_Reg.regValue = rtd_inl(PPOVERLAY_MAIN_DEN_V_Start_End_reg);
	main_active_h_start_end_Reg.regValue = rtd_inl(PPOVERLAY_MAIN_Active_H_Start_End_reg);
	main_active_v_start_end_Reg.regValue = rtd_inl(PPOVERLAY_MAIN_Active_V_Start_End_reg);

	ppoverlay_uzudtg_control1_reg.regValue = rtd_inl(PPOVERLAY_uzudtg_control1_reg);
	dc2h_rgb2yuv_ctrl_reg.regValue = rtd_inl(DC2H_RGB2YUV_DC2H_RGB2YUV_CTRL_reg); //9c00

	dc2h_rgb2yuv_ctrl_reg.dc2h_drop_en = 0; //2pto1p , reset value
	rtd_outl(DC2H_RGB2YUV_DC2H_RGB2YUV_CTRL_reg, dc2h_rgb2yuv_ctrl_reg.regValue);

	if(in_position == KADP_VT_SCALER_OUTPUT)
	{
		*dc2h_in_sel = _MAIN_UZU;

        in_size->nWidth = (main_active_h_start_end_Reg.mh_act_end - main_active_h_start_end_Reg.mh_act_sta);
        in_size->nLength = (main_active_v_start_end_Reg.mv_act_end - main_active_v_start_end_Reg.mv_act_sta);
	}
	else if(in_position == KADP_VT_DISPLAY_OUTPUT)
	{
        *dc2h_in_sel = _Memc_Mux_Input;
        in_size->nWidth = (main_den_h_start_end_Reg.mh_den_end - main_den_h_start_end_Reg.mh_den_sta);
        in_size->nLength = (main_den_v_start_end_Reg.mv_den_end - main_den_v_start_end_Reg.mv_den_sta);
		
	}
	else if(in_position == KADP_VT_OSDVIDEO_OUTPUT)
	{
        *dc2h_in_sel = _OSD_MIXER;
        in_size->nWidth = (main_den_h_start_end_Reg.mh_den_end - main_den_h_start_end_Reg.mh_den_sta);
        in_size->nLength = (main_den_v_start_end_Reg.mv_den_end - main_den_v_start_end_Reg.mv_den_sta);
	}
    else if(in_position == KADP_VT_SCALER_INPUT)
    {
        *dc2h_in_sel = _Idomain_Sdnr_input;
        in_size->nWidth = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_DI_WID);
        in_size->nLength = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_DI_LEN);

        set_dc2h_capture_sdnr_in_clock(TRUE);

    }
    else
    {
        rtd_pr_vt_emerg("[error]invalid cap position line:%d\n",__LINE__);
    }

//before scaler input have hsd,so mac8p in_size not morethan 3840
    if(in_size->nWidth > VT_CAP_FRAME_WIDTH_4K2K || in_size->nLength > VT_CAP_FRAME_HEIGHT_4K2K){
        pdc2h_hdl->out_size.nWidth = VT_CAP_FRAME_WIDTH_2K1k;
        pdc2h_hdl->out_size.nLength = VT_CAP_FRAME_HEIGHT_2K1k;
        rtd_pr_vt_notice("[warning] Input Source Hact/Vact over limitation!!!!\n");
    }
}

void drvif_dc2h_seqmode_conifg(unsigned char enable, int capWid, int capLen, int capSrc)
{
	pdc2h_hdl = &dc2h_hdl;
	memset(pdc2h_hdl, 0 , sizeof(DC2H_HANDLER));
	pdc2h_hdl->out_size.nWidth = capWid;
	pdc2h_hdl->out_size.nLength = capLen;
	pdc2h_hdl->dc2h_en = enable;

    if (enable == TRUE)
	{
		set_DC2H_rgb2yuv_ctrl_reg_refer_dump_location(capSrc, &pdc2h_hdl->in_size, &pdc2h_hdl->dc2h_in_sel);
		rtd_pr_vt_emerg("[Vt capinfo]vt cap sel=%d input w=%d; input h=%d;output w=%d,output h=%d! \n", pdc2h_hdl->dc2h_in_sel,pdc2h_hdl->in_size.nWidth, pdc2h_hdl->in_size.nLength,pdc2h_hdl->out_size.nWidth,pdc2h_hdl->out_size.nLength);

		if(get_vt_VtSwBufferMode())
		{
			if(FALSE == drvif_dc2h_swmode_inforpc(TRUE,get_vt_VtBufferNum(),capWid,capLen))
			{
				rtd_pr_vt_emerg("%s=%d RPC fail so not setting dc2h and return!!\n", __FUNCTION__, __LINE__);
				return;
			}
		}
#ifdef ENABLE_VT_TEST_CASE_CONFIG
        if(get_vt_test_case() == VT_TEST_OVERSCAN_CASE)
        {
             pdc2h_hdl->in_size.nWidth = pdc2h_hdl->out_size.nWidth;
             pdc2h_hdl->in_size.nLength = pdc2h_hdl->out_size.nLength;
        }
#endif
		drvif_dc2h_input_overscan_config(0, 0, pdc2h_hdl->in_size.nWidth, pdc2h_hdl->in_size.nLength);

		drvif_color_colorspaceyuv2rgbtransfer(pdc2h_hdl->dc2h_in_sel);
		drvif_color_ultrazoom_config_dc2h_scaling_down(pdc2h_hdl);
		if((get_VT_Pixel_Format() == VT_CAP_NV12)||(get_VT_Pixel_Format() == VT_CAP_NV21))
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

	if ((get_VT_Pixel_Format() == VT_CAP_NV12)||(get_VT_Pixel_Format() == VT_CAP_NV16)||(get_VT_Pixel_Format() == VT_CAP_NV21))
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

	set_dc2h_capture_sdnr_in_clock(FALSE);

}

void set_vfod_freezed(unsigned char boffon)
{
	rtd_pr_vt_debug("fun:%s,boffon=%d\n", __FUNCTION__, boffon);
	if (CaptureCtrl_VT.cap_buffer[0].phyaddr == 0) {
		rtd_pr_vt_emerg("[Vt warning]vt no init, so no memory, return freeze action;%s=%d \n", __FUNCTION__, __LINE__);
		return;
	}

	if(((drvif_scaler_get_abnormal_dvs_long_flag()==TRUE)||(drvif_scaler_get_abnormal_dvs_short_flag()==TRUE))
		&& (get_vt_VtBufferNum() == MAX_CAP_BUF_NUM))
	{
		if(wait_dvs_stable_cnt != 0) /* dvs stable once */
		{
			abnormal_dvs_cnt = 0;
			wait_dvs_stable_cnt = 0;
			rtd_pr_vt_emerg("reset abnormal_dvs_cnt,wait_dvs_stable_cnt,because not wait dvs stable\n");
		}
		abnormal_dvs_cnt++;
		
		if(ABNORMAL_DVS_FIRST_HAPPEN == abnormal_dvs_cnt) /*happen abnormal_dvs stop dc2h */
		{
			rtd_pr_vt_emerg("[Warning VBE]%d Vbe at abnormal dvs status,could not do vt capture!\n", __LINE__);
			if(TRUE == get_dc2h_capture_state()) /*dc2h already enable*/
			{
				vt_enable_dc2h(FALSE); /*stop dc2h,avoid memory trash,kernel panic KTASKWBS-13022*/
				rtd_pr_vt_emerg("stop dc2h done!\n");
			}
			return;
		}
		if(abnormal_dvs_cnt > MAX_CAP_BUF_NUM)
		{
			set_dqbuf_ioctl_fail_flag(TRUE);
			rtd_pr_vt_emerg("[Warning]dc2h has been disable because abnormal dvs\n");
			return;
		}
	}
	else
	{
		if(abnormal_dvs_cnt != 0) /* happen abnormal dvs before*/
		{
			wait_dvs_stable_cnt++; 
			if(wait_dvs_stable_cnt == WAIT_DVS_STABLE_COUNT) /* dvs wait stable,this function called twice in every dqubf,consecutive 4 times stable dvs, enable VT capture again */
			{
				abnormal_dvs_cnt = 0;
				wait_dvs_stable_cnt = 0;
				set_dqbuf_ioctl_fail_flag(FALSE);
				if(FALSE == get_dc2h_capture_state())
				{
					vt_enable_dc2h(TRUE); /* KTASKWBS-13909 dvs stable again, enable dc2h */
					rtd_pr_vt_emerg("enable dc2h done!\n");
				}
			}
		}
	}

	if(get_vt_VtBufferNum() == 1)
	{
		unsigned int timeoutcount = 100;
		dc2h_vi_dc2h_vi_doublebuffer_RBUS dc2h_vi_doublebuffer_Reg;
		dc2h_vi_doublebuffer_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_doublebuffer_reg);
		dc2h_vi_doublebuffer_Reg.vi_db_en = 1; //enable double buffer
		rtd_outl(DC2H_VI_DC2H_vi_doublebuffer_reg, dc2h_vi_doublebuffer_Reg.regValue);
		if(boffon == TRUE) //freeze
		{
			dc2h_vi_doublebuffer_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_doublebuffer_reg);
			dc2h_vi_doublebuffer_Reg.vi_db_apply = 1;
			rtd_outl(DC2H_VI_DC2H_vi_doublebuffer_reg, dc2h_vi_doublebuffer_Reg.regValue);

			do{
				if(DC2H_VI_DC2H_vi_doublebuffer_get_vi_db_apply(rtd_inl(DC2H_VI_DC2H_vi_doublebuffer_reg))){
					timeoutcount--;
					msleep(0);
				}
				else{
					break;
				}

			}while(timeoutcount);

			if(timeoutcount == 0)
			{
				rtd_pr_vt_notice("fun:%s, freeze before action timeout\n", __FUNCTION__);
			}

			//dc2h_vi_doublebuffer_Reg.vi_db_en = 1;
			if((get_VT_Pixel_Format() == VT_CAP_NV12)||(get_VT_Pixel_Format() == VT_CAP_NV16)||(get_VT_Pixel_Format() == VT_CAP_NV21))
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

			timeoutcount = 100;
			do{
				if(DC2H_VI_DC2H_vi_doublebuffer_get_vi_db_apply(rtd_inl(DC2H_VI_DC2H_vi_doublebuffer_reg))){
					timeoutcount--;
					msleep(0);
				}
				else{
					break;
				}
			}while(timeoutcount);

			if(timeoutcount == 0)
			{
				rtd_pr_vt_notice("fun:%s, freeze action timeout\n", __FUNCTION__);
			}
			IndexOfFreezedVideoFrameBuffer = 0;
		}
		else
		{
			if((get_VT_Pixel_Format() == VT_CAP_NV12)||(get_VT_Pixel_Format() == VT_CAP_NV16)||(get_VT_Pixel_Format() == VT_CAP_NV21))
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
				if(DC2H_VI_DC2H_vi_doublebuffer_get_vi_db_apply(rtd_inl(DC2H_VI_DC2H_vi_doublebuffer_reg))){
					timeoutcount--;
					msleep(0);
					}
				else
					break;
			}while(timeoutcount);

			if(timeoutcount == 0)
			{
				rtd_pr_vt_notice("fun:%s, unfreeze action timeout\n", __FUNCTION__);
			}
			/* wait one frame */
			timeoutcount = 100;
			dc2h_vi_doublebuffer_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_doublebuffer_reg);
			dc2h_vi_doublebuffer_Reg.vi_db_apply = 1;
			rtd_outl(DC2H_VI_DC2H_vi_doublebuffer_reg, dc2h_vi_doublebuffer_Reg.regValue);
			do{
				if(DC2H_VI_DC2H_vi_doublebuffer_get_vi_db_apply(rtd_inl(DC2H_VI_DC2H_vi_doublebuffer_reg))){
					timeoutcount--;
					msleep(0);
				}
				else
					break;
			}while(timeoutcount);

			if(timeoutcount == 0)
			{
				rtd_pr_vt_notice("fun:%s, wait one frame timeout\n", __FUNCTION__);
			}
		}
		dc2h_vi_doublebuffer_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_doublebuffer_reg);
		dc2h_vi_doublebuffer_Reg.vi_db_en = 0; /* doubel buffer only work when buffer_num=1 */
		rtd_outl(DC2H_VI_DC2H_vi_doublebuffer_reg, dc2h_vi_doublebuffer_Reg.regValue);

	} else {
		if (boffon == TRUE) {
			if((get_VT_Pixel_Format() == VT_CAP_NV12)||(get_VT_Pixel_Format() == VT_CAP_NV16)||(get_VT_Pixel_Format() == VT_CAP_NV21)){
				if(get_vt_VtBufferNum()==5) {
					dc2h_vi_dc2h_vi_ads_start_y_RBUS DC2H_VI_DC2H_vi_ads_start_y_Reg;
					dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_Reg;
                    static unsigned int last_called_time = 0;
                    unsigned int called_time = 0;
					unsigned int lastcapbufferaddr = 0;
                    unsigned short int dc2h_output_fps = 1000 / Get_VFOD_FrameRate();
					unsigned int vflip_ystart_offset = (vt_cap_frame_max_width*vt_cap_frame_max_height - vt_cap_frame_max_width);
					DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_ads_start_y_reg);
					dc2h_cap_l3_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L3_Start_reg);

                    called_time = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg)/90; //ms

					if(Get_PANEL_VFLIP_ENABLE())
					{
						if(DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == (CaptureCtrl_VT.cap_buffer[0].phyaddr + vflip_ystart_offset)){
							lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[4].phyaddr + vflip_ystart_offset);
							IndexOfFreezedVideoFrameBuffer = 4;
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == (CaptureCtrl_VT.cap_buffer[1].phyaddr + vflip_ystart_offset)){
							lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[0].phyaddr + vflip_ystart_offset);
							IndexOfFreezedVideoFrameBuffer = 0;
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == (CaptureCtrl_VT.cap_buffer[2].phyaddr + vflip_ystart_offset)){
							lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[1].phyaddr + vflip_ystart_offset);
							IndexOfFreezedVideoFrameBuffer = 1;
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == (CaptureCtrl_VT.cap_buffer[3].phyaddr + vflip_ystart_offset)){
							lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[2].phyaddr + vflip_ystart_offset);
							IndexOfFreezedVideoFrameBuffer = 2;
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == (CaptureCtrl_VT.cap_buffer[4].phyaddr + vflip_ystart_offset)){
							lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[3].phyaddr + vflip_ystart_offset);
							IndexOfFreezedVideoFrameBuffer = 3;
						}
					}
					else
					{
						if(DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[0].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[4].phyaddr;
							IndexOfFreezedVideoFrameBuffer = 4;
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[1].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[0].phyaddr;
							IndexOfFreezedVideoFrameBuffer = 0;
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[2].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[1].phyaddr;
							IndexOfFreezedVideoFrameBuffer = 1;
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[3].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[2].phyaddr;
							IndexOfFreezedVideoFrameBuffer = 2;
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[4].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[3].phyaddr;
							IndexOfFreezedVideoFrameBuffer = 3;
						}

                        if(((called_time - last_called_time) < dc2h_output_fps ))
                        {
                            lastcapbufferaddr = dc2h_cap_l3_start_Reg.regValue;
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
                        }
                        else if(((called_time - last_called_time) >= dc2h_output_fps *3 ) && (lastcapbufferaddr == dc2h_cap_l3_start_Reg.regValue))
                        {
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
    					}
                    }

                    last_called_time = called_time;

					rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
				} else if(get_vt_VtBufferNum()==4) {
					dc2h_vi_dc2h_vi_ads_start_y_RBUS DC2H_VI_DC2H_vi_ads_start_y_Reg;
					dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_Reg;
					unsigned int lastcapbufferaddr = 0;
					unsigned int vflip_ystart_offset = (vt_cap_frame_max_width*vt_cap_frame_max_height - vt_cap_frame_max_width);
					DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_ads_start_y_reg);
					dc2h_cap_l3_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L3_Start_reg);
					
					if(Get_PANEL_VFLIP_ENABLE())
					{
						if(DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == (CaptureCtrl_VT.cap_buffer[0].phyaddr + vflip_ystart_offset)){
							lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[3].phyaddr + vflip_ystart_offset);
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == (CaptureCtrl_VT.cap_buffer[1].phyaddr + vflip_ystart_offset)){
							lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[0].phyaddr + vflip_ystart_offset);
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == (CaptureCtrl_VT.cap_buffer[2].phyaddr + vflip_ystart_offset)){
							lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[1].phyaddr + vflip_ystart_offset);
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == (CaptureCtrl_VT.cap_buffer[3].phyaddr + vflip_ystart_offset)){
							lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[2].phyaddr + vflip_ystart_offset);
						}

						if(lastcapbufferaddr == dc2h_cap_l3_start_Reg.regValue){
							if(lastcapbufferaddr == (CaptureCtrl_VT.cap_buffer[0].phyaddr + vflip_ystart_offset)){
								lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[3].phyaddr + vflip_ystart_offset);
							}else if (lastcapbufferaddr == (CaptureCtrl_VT.cap_buffer[1].phyaddr + vflip_ystart_offset)){
								lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[0].phyaddr + vflip_ystart_offset);
							}else if (lastcapbufferaddr == (CaptureCtrl_VT.cap_buffer[2].phyaddr + vflip_ystart_offset)){
								lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[1].phyaddr + vflip_ystart_offset);
							}else if (lastcapbufferaddr == (CaptureCtrl_VT.cap_buffer[3].phyaddr + vflip_ystart_offset)){
								lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[2].phyaddr + vflip_ystart_offset);
							}
						}

						if(lastcapbufferaddr == (CaptureCtrl_VT.cap_buffer[0].phyaddr + vflip_ystart_offset)){
							IndexOfFreezedVideoFrameBuffer = 0;
						}else if (lastcapbufferaddr == (CaptureCtrl_VT.cap_buffer[1].phyaddr + vflip_ystart_offset)){
							IndexOfFreezedVideoFrameBuffer = 1;
						}else if (lastcapbufferaddr == (CaptureCtrl_VT.cap_buffer[2].phyaddr + vflip_ystart_offset)){
							IndexOfFreezedVideoFrameBuffer = 2;
						}else if (lastcapbufferaddr == (CaptureCtrl_VT.cap_buffer[3].phyaddr + vflip_ystart_offset)){
							IndexOfFreezedVideoFrameBuffer = 3;
						}

					}
					else
					{
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
					}
				
					rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
				} else if(get_vt_VtBufferNum()==3) {
					dc2h_vi_dc2h_vi_ads_start_y_RBUS DC2H_VI_DC2H_vi_ads_start_y_Reg;
					dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_Reg;
					unsigned int lastcapbufferaddr = 0;
					unsigned int vflip_ystart_offset = (vt_cap_frame_max_width*vt_cap_frame_max_height - vt_cap_frame_max_width);
					DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_ads_start_y_reg);
					dc2h_cap_l3_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L3_Start_reg);

		            if(Get_PANEL_VFLIP_ENABLE())
					{
						if(DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == (CaptureCtrl_VT.cap_buffer[0].phyaddr + vflip_ystart_offset)){
							lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[2].phyaddr + vflip_ystart_offset);
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == (CaptureCtrl_VT.cap_buffer[1].phyaddr + vflip_ystart_offset)){
							lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[0].phyaddr + vflip_ystart_offset);
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == (CaptureCtrl_VT.cap_buffer[2].phyaddr + vflip_ystart_offset)){
							lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[1].phyaddr + vflip_ystart_offset);
						}

						/*if(lastcapbufferaddr == dc2h_cap_l3_start_Reg.regValue){
							if(lastcapbufferaddr == (CaptureCtrl_VT.cap_buffer[0].phyaddr + vflip_ystart_offset)){
								lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[2].phyaddr + vflip_ystart_offset);
							}else if (lastcapbufferaddr == (CaptureCtrl_VT.cap_buffer[1].phyaddr + vflip_ystart_offset)){
								lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[0].phyaddr + vflip_ystart_offset);
							}else if (lastcapbufferaddr == (CaptureCtrl_VT.cap_buffer[2].phyaddr + vflip_ystart_offset)){
								lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[1].phyaddr + vflip_ystart_offset);
							}
						}WOSQRTK-13247's fix return repeat index when input framerate slower than display framerate */

						if(lastcapbufferaddr == (CaptureCtrl_VT.cap_buffer[0].phyaddr + vflip_ystart_offset)){
							IndexOfFreezedVideoFrameBuffer = 0;
						}else if (lastcapbufferaddr == (CaptureCtrl_VT.cap_buffer[1].phyaddr + vflip_ystart_offset)){
							IndexOfFreezedVideoFrameBuffer = 1;
						}else if (lastcapbufferaddr == (CaptureCtrl_VT.cap_buffer[2].phyaddr + vflip_ystart_offset)){
							IndexOfFreezedVideoFrameBuffer = 2;
						}
					}
					else
					{
						if(DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[0].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[2].phyaddr;
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[1].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[0].phyaddr;
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[2].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[1].phyaddr;
						}

						/*if(lastcapbufferaddr == dc2h_cap_l3_start_Reg.regValue){
							if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr){
								lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[2].phyaddr;
							}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr){
								lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[0].phyaddr;
							}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[2].phyaddr){
								lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[1].phyaddr;
							}
						}WOSQRTK-13247's fix return repeat index when input framerate slower than display framerate */

						if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr){
							IndexOfFreezedVideoFrameBuffer = 0;
						}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr){
							IndexOfFreezedVideoFrameBuffer = 1;
						}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[2].phyaddr){
							IndexOfFreezedVideoFrameBuffer = 2;
						}
					}
		           
		           rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
		        }else if(get_vt_VtBufferNum()==2) {
					dc2h_vi_dc2h_vi_ads_start_y_RBUS DC2H_VI_DC2H_vi_ads_start_y_Reg;
					dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_Reg;
					unsigned int lastcapbufferaddr = 0;
					unsigned int vflip_ystart_offset = (vt_cap_frame_max_width*vt_cap_frame_max_height - vt_cap_frame_max_width);
					DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_ads_start_y_reg);
					dc2h_cap_l3_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L3_Start_reg);

		            if(Get_PANEL_VFLIP_ENABLE())
					{
						if(DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == (CaptureCtrl_VT.cap_buffer[0].phyaddr + vflip_ystart_offset)){
							lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[1].phyaddr + vflip_ystart_offset);
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == (CaptureCtrl_VT.cap_buffer[1].phyaddr + vflip_ystart_offset)){
							lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[0].phyaddr + vflip_ystart_offset);
						}

						if(lastcapbufferaddr == (CaptureCtrl_VT.cap_buffer[0].phyaddr + vflip_ystart_offset)){
							IndexOfFreezedVideoFrameBuffer = 0;
						}else if (lastcapbufferaddr == (CaptureCtrl_VT.cap_buffer[1].phyaddr + vflip_ystart_offset)){
							IndexOfFreezedVideoFrameBuffer = 1;
						}
					}
					else
					{
						if(DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[0].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[1].phyaddr;
						}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[1].phyaddr){
							lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[0].phyaddr;
						}

						if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr){
							IndexOfFreezedVideoFrameBuffer = 0;
						}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr){
							IndexOfFreezedVideoFrameBuffer = 1;
						}
					}
					
		           rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
		        }

			} 
            else if((get_VT_Pixel_Format() == VT_CAP_RGB888) || (get_VT_Pixel_Format() == VT_CAP_ARGB8888) || (get_VT_Pixel_Format() == VT_CAP_RGBA8888) || (get_VT_Pixel_Format() == VT_CAP_ABGR8888)){
				if(get_vt_VtBufferNum()==5) {
					dc2h_dma_dc2h_cap_l1_start_RBUS dc2h_cap_l1_start_Reg;
					dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_Reg;
					unsigned int lastcapbufferaddr = 0;
                    unsigned int seq_vflip_offset = (get_VT_Pixel_Format() == VT_CAP_RGB888) ? (vt_cap_frame_max_width*vt_cap_frame_max_height*3 - vt_cap_frame_max_width*3) : (vt_cap_frame_max_width*vt_cap_frame_max_height*4 - vt_cap_frame_max_width*4);
					dc2h_cap_l1_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L1_Start_db_reg);
					
					dc2h_cap_l3_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L3_Start_reg);

                    if(Get_PANEL_VFLIP_ENABLE())
                    {
                        if(dc2h_cap_l1_start_Reg.regValue == (CaptureCtrl_VT.cap_buffer[0].phyaddr + seq_vflip_offset)){
                            lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[4].phyaddr + seq_vflip_offset);
                        }else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[1].phyaddr + seq_vflip_offset){
                            lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[0].phyaddr + seq_vflip_offset);
                        }else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[2].phyaddr + seq_vflip_offset){
                            lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[1].phyaddr + seq_vflip_offset);
                        }else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[3].phyaddr + seq_vflip_offset){
                            lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[2].phyaddr + seq_vflip_offset);
                        }else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[4].phyaddr + seq_vflip_offset){
                            lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[3].phyaddr + seq_vflip_offset);
                        }

                        if(lastcapbufferaddr == dc2h_cap_l3_start_Reg.regValue){
                            if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr + seq_vflip_offset){
                                lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[4].phyaddr + seq_vflip_offset);
                            }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr + seq_vflip_offset){
                            	lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[0].phyaddr + seq_vflip_offset);
                            }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[2].phyaddr + seq_vflip_offset){
                            	lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[1].phyaddr + seq_vflip_offset);
                            }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[3].phyaddr + seq_vflip_offset){
                            	lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[2].phyaddr + seq_vflip_offset);
                            }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[4].phyaddr + seq_vflip_offset){
                            	lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[3].phyaddr + seq_vflip_offset);
                            }
					    }

                        if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr + seq_vflip_offset){
                        	IndexOfFreezedVideoFrameBuffer = 0;
                        }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr + seq_vflip_offset){
                        	IndexOfFreezedVideoFrameBuffer = 1;
                        }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[2].phyaddr + seq_vflip_offset){
                        	IndexOfFreezedVideoFrameBuffer = 2;
                        }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[3].phyaddr + seq_vflip_offset){
                        	IndexOfFreezedVideoFrameBuffer = 3;
                        }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[4].phyaddr + seq_vflip_offset){
                        	IndexOfFreezedVideoFrameBuffer = 4;
                        }
                    }
                    else
                    {
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
                    }

					rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
				}
                else if(get_vt_VtBufferNum()==4) {
					dc2h_dma_dc2h_cap_l1_start_RBUS dc2h_cap_l1_start_Reg;
					dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_Reg;
					unsigned int lastcapbufferaddr = 0;
                    unsigned int seq_vflip_offset = (get_VT_Pixel_Format() == VT_CAP_RGB888) ? (vt_cap_frame_max_width*vt_cap_frame_max_height*3 - vt_cap_frame_max_width*3) : (vt_cap_frame_max_width*vt_cap_frame_max_height*4 - vt_cap_frame_max_width*4);
					dc2h_cap_l1_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L1_Start_db_reg);

					dc2h_cap_l3_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L3_Start_reg);

                    if(Get_PANEL_VFLIP_ENABLE())
                    {
                        if(dc2h_cap_l1_start_Reg.regValue == (CaptureCtrl_VT.cap_buffer[0].phyaddr + seq_vflip_offset)){
                            lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[3].phyaddr + seq_vflip_offset);
                        }else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[1].phyaddr + seq_vflip_offset){
                            lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[0].phyaddr + seq_vflip_offset);
                        }else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[2].phyaddr + seq_vflip_offset){
                            lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[1].phyaddr + seq_vflip_offset);
                        }else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[3].phyaddr + seq_vflip_offset){
                            lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[2].phyaddr + seq_vflip_offset);
                        }

                        if(lastcapbufferaddr == dc2h_cap_l3_start_Reg.regValue){
                            if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr + seq_vflip_offset){
                                lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[3].phyaddr + seq_vflip_offset);
                            }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr + seq_vflip_offset){
                            	lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[0].phyaddr + seq_vflip_offset);
                            }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[2].phyaddr + seq_vflip_offset){
                            	lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[1].phyaddr + seq_vflip_offset);
                            }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[3].phyaddr + seq_vflip_offset){
                            	lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[2].phyaddr + seq_vflip_offset);
                            }
					    }

                        if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr + seq_vflip_offset){
                        	IndexOfFreezedVideoFrameBuffer = 0;
                        }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr + seq_vflip_offset){
                        	IndexOfFreezedVideoFrameBuffer = 1;
                        }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[2].phyaddr + seq_vflip_offset){
                        	IndexOfFreezedVideoFrameBuffer = 2;
                        }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[3].phyaddr + seq_vflip_offset){
                        	IndexOfFreezedVideoFrameBuffer = 3;
                        }
                    }
                    else
                    {
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
                    }

					rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
				}
                else if(get_vt_VtBufferNum()==3) {
					dc2h_dma_dc2h_cap_l1_start_RBUS dc2h_cap_l1_start_Reg;
					dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_Reg;
					unsigned int lastcapbufferaddr = 0;
                    unsigned int seq_vflip_offset = (get_VT_Pixel_Format() == VT_CAP_RGB888) ? (vt_cap_frame_max_width*vt_cap_frame_max_height*3 - vt_cap_frame_max_width*3) : (vt_cap_frame_max_width*vt_cap_frame_max_height*4 - vt_cap_frame_max_width*4);
					dc2h_cap_l1_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L1_Start_db_reg);

					dc2h_cap_l3_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L3_Start_reg);

                    if(Get_PANEL_VFLIP_ENABLE())
                    {
                        if(dc2h_cap_l1_start_Reg.regValue == (CaptureCtrl_VT.cap_buffer[0].phyaddr + seq_vflip_offset)){
                            lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[2].phyaddr + seq_vflip_offset);
                        }else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[1].phyaddr + seq_vflip_offset){
                            lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[0].phyaddr + seq_vflip_offset);
                        }else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[2].phyaddr + seq_vflip_offset){
                            lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[1].phyaddr + seq_vflip_offset);
                        }

                        if(lastcapbufferaddr == dc2h_cap_l3_start_Reg.regValue){
                            if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr + seq_vflip_offset){
                                lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[2].phyaddr + seq_vflip_offset);
                            }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr + seq_vflip_offset){
                            	lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[0].phyaddr + seq_vflip_offset);
                            }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[2].phyaddr + seq_vflip_offset){
                            	lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[1].phyaddr + seq_vflip_offset);
                            }
					    }

                        if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr + seq_vflip_offset){
                        	IndexOfFreezedVideoFrameBuffer = 0;
                        }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr + seq_vflip_offset){
                        	IndexOfFreezedVideoFrameBuffer = 1;
                        }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[2].phyaddr + seq_vflip_offset){
                        	IndexOfFreezedVideoFrameBuffer = 2;
                        }
                    }
                    else
                    {
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
                    }

					rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
				}
                else if(get_vt_VtBufferNum()==2) {
					dc2h_dma_dc2h_cap_l1_start_RBUS dc2h_cap_l1_start_Reg;
					dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_Reg;
					unsigned int lastcapbufferaddr = 0;
                    unsigned int seq_vflip_offset = (get_VT_Pixel_Format() == VT_CAP_RGB888) ? (vt_cap_frame_max_width*vt_cap_frame_max_height*3 - vt_cap_frame_max_width*3) : (vt_cap_frame_max_width*vt_cap_frame_max_height*4 - vt_cap_frame_max_width*4);
					dc2h_cap_l1_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L1_Start_db_reg);

					dc2h_cap_l3_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L3_Start_reg);

                    if(Get_PANEL_VFLIP_ENABLE())
                    {
                        if(dc2h_cap_l1_start_Reg.regValue == (CaptureCtrl_VT.cap_buffer[0].phyaddr + seq_vflip_offset)){
                            lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[1].phyaddr + seq_vflip_offset);
                        }else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[1].phyaddr + seq_vflip_offset){
                            lastcapbufferaddr = (CaptureCtrl_VT.cap_buffer[0].phyaddr + seq_vflip_offset);
                        }

                        if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr + seq_vflip_offset){
                        	IndexOfFreezedVideoFrameBuffer = 0;
                        }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr + seq_vflip_offset){
                        	IndexOfFreezedVideoFrameBuffer = 1;
                        }
                    }
                    else
                    {
                        if(dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[0].phyaddr){
                            lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[1].phyaddr;
                        }else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[1].phyaddr){
                            lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[0].phyaddr;
                        }

                        if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr){
                        	IndexOfFreezedVideoFrameBuffer = 0;
                        }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr){
                        	IndexOfFreezedVideoFrameBuffer = 1;
                        }
                    }

					rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
				}
			}
            else
                rtd_pr_vt_notice("fun:%s,unknow pixel format\n",__FUNCTION__);
		}
        else{
			//rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,0);
		}
	}
}
#endif

void set_cap_buffer_size_by_AP(unsigned int usr_width, unsigned int usr_height)
{
	unsigned int width_16bytes_align = 0;
	width_16bytes_align = drvif_memory_get_data_align(usr_width,16);
	vt_cap_frame_max_width = width_16bytes_align;
	vt_cap_frame_max_height = usr_height;
	rtd_pr_vt_notice("cap buf size:(%d,%d)\n",vt_cap_frame_max_width, vt_cap_frame_max_height);
}

unsigned long vt_bufAddr_align(unsigned int val)
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

unsigned int get_vt_capBuf_Size(void)
{
    if((get_VT_Pixel_Format() == VT_CAP_ABGR8888) || (get_VT_Pixel_Format() == VT_CAP_ARGB8888) || (get_VT_Pixel_Format() == VT_CAP_RGBA8888))
        VT_CAPTURE_BUFFER_UNITSIZE = vt_bufAddr_align(vt_cap_frame_max_width*vt_cap_frame_max_height*4);
    else if((get_VT_Pixel_Format() == VT_CAP_NV12)||(get_VT_Pixel_Format() == VT_CAP_NV21))
        VT_CAPTURE_BUFFER_UNITSIZE = vt_bufAddr_align((vt_cap_frame_max_width*vt_cap_frame_max_height))+vt_bufAddr_align((vt_cap_frame_max_width*vt_cap_frame_max_height/2));
    else if(get_VT_Pixel_Format() == VT_CAP_NV16)
        VT_CAPTURE_BUFFER_UNITSIZE = vt_bufAddr_align(vt_cap_frame_max_width*vt_cap_frame_max_height)+vt_bufAddr_align(vt_cap_frame_max_width*vt_cap_frame_max_height);
    else
        VT_CAPTURE_BUFFER_UNITSIZE = vt_bufAddr_align(vt_cap_frame_max_width*vt_cap_frame_max_height*3);

    return VT_CAPTURE_BUFFER_UNITSIZE;
}


unsigned char Capture_BufferMemInit_VT(unsigned int buffernum)
{
    unsigned int size = 0;
    int i;
    unsigned long allocatetotalsize = 0, carvedout_size = 0;
    unsigned long physicalstartaddree = 0;
    //unsigned long align96phisicaladdr = 0;
    unsigned long DMA_release_start = 0;
    unsigned long DMA_release_size = 0;

    size = get_vt_capBuf_Size();

    rtd_pr_vt_debug("[VT]buf size=0x%x\n",size);

    if(get_vt_EnableFRCMode() == TRUE)
    {
        DMA_release_start = 0;
    }
    else
    {// co-buffer with memc

        allocatetotalsize = size * buffernum;
        if(get_MEMC_bypass_status_refer_platform_model() == FALSE)
        {//non-bypass memc
#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
            DMA_release_size = (unsigned int)carvedout_buf_query_secure(CARVEDOUT_SCALER_MEMC, (void *)&DMA_release_start);
#else
            DMA_release_size = (unsigned int)carvedout_buf_query(CARVEDOUT_SCALER_MEMC, (void *)&DMA_release_start);
#endif
            if((DMA_release_start == 0) || (allocatetotalsize > DMA_release_size))
            {
                rtd_pr_vt_emerg("[error]:query carvedout memc memory failed (%lx/%lx)\n", DMA_release_start, DMA_release_size);
                return FALSE;
            }
        }
        else
        {//if memc bypass.
#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
            carvedout_size = (unsigned int)carvedout_buf_query_secure(CARVEDOUT_SCALER_MEMC, (void *)&DMA_release_start);
#else
            carvedout_size = (unsigned int)carvedout_buf_query(CARVEDOUT_SCALER_MEMC, (void *)&DMA_release_start);
#endif
            if((carvedout_size == 0) || (allocatetotalsize > carvedout_size))
            {
                rtd_pr_vt_emerg("[warn]query carvedout memc memory failed (%lx/%lx)\n", DMA_release_start, carvedout_size);
                DMA_release_start = 0; //allocate memory from cma
            }
        }
    }

    if(DMA_release_start != 0)
    { /* all capture buffer form memc */
        physicalstartaddree = DMA_release_start;

        for (i = 0; i < buffernum; i++)
        {
            if(CaptureCtrl_VT.cap_buffer[i].phyaddr == 0)
            {
                CaptureCtrl_VT.cap_buffer[i].cache = NULL;
                CaptureCtrl_VT.cap_buffer[i].phyaddr = vt_bufAddr_align((i == 0) ? (physicalstartaddree) : (CaptureCtrl_VT.cap_buffer[i-1].phyaddr + size));
                CaptureCtrl_VT.cap_buffer[i].size = size;
                rtd_pr_vt_notice("[memory allocate1] buf[%d]=(%d KB), phy(%lx) \n", i, CaptureCtrl_VT.cap_buffer[i].size >> 10, (unsigned long)CaptureCtrl_VT.cap_buffer[i].phyaddr);
            }
        }
    }
    else
    {
        allocatetotalsize = size * buffernum;
        carvedout_size = 0;//(unsigned int)carvedout_buf_query(CARVEDOUT_VT, (void *)&physicalstartaddree);

        if ((carvedout_size == 0) || (physicalstartaddree == 0) || (allocatetotalsize > carvedout_size))
        {
            rtd_pr_vt_notice("VT pli(%lx/%lx)\n", physicalstartaddree, carvedout_size);

            for(i = 0; i < buffernum; i++)
            {
                physicalstartaddree = pli_malloc(size, GFP_DCU2_FIRST);
                if(physicalstartaddree == INVALID_VAL)
                {
                    rtd_pr_vt_emerg("[error]retry pli malloc fail(%lx)\n", physicalstartaddree);
                    return FALSE;
                }
                else
                {
                    VtAllocatedBufferStartAdress[i] = physicalstartaddree;
                    if(CaptureCtrl_VT.cap_buffer[i].phyaddr == 0)
                    {
                        CaptureCtrl_VT.cap_buffer[i].cache = NULL;
                        CaptureCtrl_VT.cap_buffer[i].phyaddr = vt_bufAddr_align(physicalstartaddree);
                        CaptureCtrl_VT.cap_buffer[i].size = size;
                        rtd_pr_vt_notice("[memory allocate2] buf[%d]=(%d KB), phy(%lx) \n", i, CaptureCtrl_VT.cap_buffer[i].size >> 10, (unsigned long)CaptureCtrl_VT.cap_buffer[i].phyaddr);
                    }
                }
            }
        }
        else
        {
            for (i = 0; i < buffernum; i++)
            {
                if(CaptureCtrl_VT.cap_buffer[i].phyaddr == 0)
                {
                    CaptureCtrl_VT.cap_buffer[i].cache = NULL;
                    CaptureCtrl_VT.cap_buffer[i].phyaddr = vt_bufAddr_align((i == 0) ? (physicalstartaddree) : (CaptureCtrl_VT.cap_buffer[i-1].phyaddr + size));
                    CaptureCtrl_VT.cap_buffer[i].size = size;
                    rtd_pr_vt_notice("[memory allocate3] buf[%d]=(%d KB), phy(%lx) \n", i, CaptureCtrl_VT.cap_buffer[i].size >> 10, (unsigned long)CaptureCtrl_VT.cap_buffer[i].phyaddr);
                }
            }
        }
    }

    return TRUE;

}

void Capture_BufferMemDeInit_VT(unsigned int buffernum)
{
	int i;

	for (i = 0; i < buffernum; i++)
	{
		if(VtAllocatedBufferStartAdress[i])
		{
			pli_free(VtAllocatedBufferStartAdress[i]);
			VtAllocatedBufferStartAdress[i] = 0;
		}
	}

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
void set_DC2H_cap_boundaryaddr(unsigned int VtBufferNum)
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

	if(CaptureCtrl_VT.cap_buffer[0].phyaddr == 0)
	{
		rtd_pr_vt_emerg("[error]vt buffer addr invalid,%s=%d \n", __FUNCTION__, __LINE__);
	}
	else
	{
		if((get_VT_Pixel_Format() == VT_CAP_NV12) || (get_VT_Pixel_Format() == VT_CAP_NV16)||(get_VT_Pixel_Format() == VT_CAP_NV21)){
			if(5 == VtBufferNum)
			{
				dc2h_cap_boundaryaddr1_reg.regValue =  ((UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[0].size - VT_BLKCAP_BURSTLEN);
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr1_reg, dc2h_cap_boundaryaddr1_reg.regValue);
				dc2h_cap_boundaryaddr2_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr2_reg, dc2h_cap_boundaryaddr2_reg.regValue);
				
				dc2h_cap_boundaryaddr3_reg.regValue =  ((UINT32)CaptureCtrl_VT.cap_buffer[1].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[1].size - VT_BLKCAP_BURSTLEN);
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr3_reg, dc2h_cap_boundaryaddr3_reg.regValue);
				dc2h_cap_boundaryaddr4_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[1].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr4_reg, dc2h_cap_boundaryaddr4_reg.regValue);
				
				dc2h_cap_boundaryaddr5_reg.regValue =  ((UINT32)CaptureCtrl_VT.cap_buffer[2].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[2].size - VT_BLKCAP_BURSTLEN);
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr5_reg, dc2h_cap_boundaryaddr5_reg.regValue);
				dc2h_cap_boundaryaddr6_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[2].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr6_reg, dc2h_cap_boundaryaddr6_reg.regValue); 
				
				dc2h_cap_boundaryaddr7_reg.regValue =  ((UINT32)CaptureCtrl_VT.cap_buffer[3].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[3].size - VT_BLKCAP_BURSTLEN);
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr7_reg, dc2h_cap_boundaryaddr7_reg.regValue);
				dc2h_cap_boundaryaddr8_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[3].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr8_reg, dc2h_cap_boundaryaddr8_reg.regValue);
				
				dc2h_cap_boundaryaddr9_reg.regValue =  ((UINT32)CaptureCtrl_VT.cap_buffer[4].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[4].size - VT_BLKCAP_BURSTLEN);
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr9_reg, dc2h_cap_boundaryaddr9_reg.regValue);
				dc2h_cap_boundaryaddr10_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[4].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr10_reg, dc2h_cap_boundaryaddr10_reg.regValue);
			}
			else if(4 == VtBufferNum)
			{
				dc2h_cap_boundaryaddr1_reg.regValue =  ((UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[0].size - VT_BLKCAP_BURSTLEN);
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr1_reg, dc2h_cap_boundaryaddr1_reg.regValue);
				dc2h_cap_boundaryaddr2_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr2_reg, dc2h_cap_boundaryaddr2_reg.regValue);
				
				dc2h_cap_boundaryaddr3_reg.regValue =  ((UINT32)CaptureCtrl_VT.cap_buffer[1].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[1].size - VT_BLKCAP_BURSTLEN);
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr3_reg, dc2h_cap_boundaryaddr3_reg.regValue);
				dc2h_cap_boundaryaddr4_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[1].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr4_reg, dc2h_cap_boundaryaddr4_reg.regValue);
				
				dc2h_cap_boundaryaddr5_reg.regValue =  ((UINT32)CaptureCtrl_VT.cap_buffer[2].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[2].size - VT_BLKCAP_BURSTLEN);
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr5_reg, dc2h_cap_boundaryaddr5_reg.regValue);
				dc2h_cap_boundaryaddr6_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[2].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr6_reg, dc2h_cap_boundaryaddr6_reg.regValue); 
				
				dc2h_cap_boundaryaddr7_reg.regValue =  ((UINT32)CaptureCtrl_VT.cap_buffer[3].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[3].size - VT_BLKCAP_BURSTLEN);
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr7_reg, dc2h_cap_boundaryaddr7_reg.regValue);
				dc2h_cap_boundaryaddr8_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[3].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr8_reg, dc2h_cap_boundaryaddr8_reg.regValue);
				
				dc2h_cap_boundaryaddr9_reg.regValue =  ((UINT32)CaptureCtrl_VT.cap_buffer[3].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[3].size - VT_BLKCAP_BURSTLEN);
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr9_reg, dc2h_cap_boundaryaddr9_reg.regValue);
				dc2h_cap_boundaryaddr10_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[3].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr10_reg, dc2h_cap_boundaryaddr10_reg.regValue);
			}
			else if(3 == VtBufferNum)
			{
				dc2h_cap_boundaryaddr1_reg.regValue =  ((UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[0].size - VT_BLKCAP_BURSTLEN);
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr1_reg, dc2h_cap_boundaryaddr1_reg.regValue);
				dc2h_cap_boundaryaddr2_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr2_reg, dc2h_cap_boundaryaddr2_reg.regValue);
				
				dc2h_cap_boundaryaddr3_reg.regValue =  ((UINT32)CaptureCtrl_VT.cap_buffer[1].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[1].size - VT_BLKCAP_BURSTLEN);
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr3_reg, dc2h_cap_boundaryaddr3_reg.regValue);
				dc2h_cap_boundaryaddr4_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[1].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr4_reg, dc2h_cap_boundaryaddr4_reg.regValue);
				
				dc2h_cap_boundaryaddr5_reg.regValue =  ((UINT32)CaptureCtrl_VT.cap_buffer[2].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[2].size - VT_BLKCAP_BURSTLEN);
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr5_reg, dc2h_cap_boundaryaddr5_reg.regValue);
				dc2h_cap_boundaryaddr6_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[2].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr6_reg, dc2h_cap_boundaryaddr6_reg.regValue); 

				dc2h_cap_boundaryaddr7_reg.regValue =  ((UINT32)CaptureCtrl_VT.cap_buffer[2].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[2].size - VT_BLKCAP_BURSTLEN);
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr7_reg, dc2h_cap_boundaryaddr7_reg.regValue);
				dc2h_cap_boundaryaddr8_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[2].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr8_reg, dc2h_cap_boundaryaddr8_reg.regValue);
				
				dc2h_cap_boundaryaddr9_reg.regValue =  ((UINT32)CaptureCtrl_VT.cap_buffer[2].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[2].size - VT_BLKCAP_BURSTLEN);
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr9_reg, dc2h_cap_boundaryaddr9_reg.regValue);
				dc2h_cap_boundaryaddr10_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[2].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr10_reg, dc2h_cap_boundaryaddr10_reg.regValue);
				
			}
			else if(2 == VtBufferNum)
			{
				dc2h_cap_boundaryaddr1_reg.regValue =  ((UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[0].size - VT_BLKCAP_BURSTLEN);
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr1_reg, dc2h_cap_boundaryaddr1_reg.regValue);
				dc2h_cap_boundaryaddr2_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr2_reg, dc2h_cap_boundaryaddr2_reg.regValue);
				
				dc2h_cap_boundaryaddr3_reg.regValue =  ((UINT32)CaptureCtrl_VT.cap_buffer[1].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[1].size - VT_BLKCAP_BURSTLEN);
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr3_reg, dc2h_cap_boundaryaddr3_reg.regValue);
				dc2h_cap_boundaryaddr4_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[1].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr4_reg, dc2h_cap_boundaryaddr4_reg.regValue);

				dc2h_cap_boundaryaddr5_reg.regValue =  ((UINT32)CaptureCtrl_VT.cap_buffer[1].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[1].size - VT_BLKCAP_BURSTLEN);
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr5_reg, dc2h_cap_boundaryaddr5_reg.regValue);
				dc2h_cap_boundaryaddr6_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[1].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr6_reg, dc2h_cap_boundaryaddr6_reg.regValue); 
				
				dc2h_cap_boundaryaddr7_reg.regValue =  ((UINT32)CaptureCtrl_VT.cap_buffer[1].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[1].size - VT_BLKCAP_BURSTLEN);
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr7_reg, dc2h_cap_boundaryaddr7_reg.regValue);
				dc2h_cap_boundaryaddr8_reg.regValue = (UINT32)CaptureCtrl_VT.cap_buffer[1].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr8_reg, dc2h_cap_boundaryaddr8_reg.regValue);
				
				dc2h_cap_boundaryaddr9_reg.regValue =  ((UINT32)CaptureCtrl_VT.cap_buffer[1].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[1].size - VT_BLKCAP_BURSTLEN);
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr9_reg, dc2h_cap_boundaryaddr9_reg.regValue);
				dc2h_cap_boundaryaddr10_reg.regValue = (UINT32)CaptureCtrl_VT.cap_buffer[1].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr10_reg, dc2h_cap_boundaryaddr10_reg.regValue);
				
			}
			else //1 == VtBufferNum
			{
				dc2h_cap_boundaryaddr1_reg.regValue =  ((UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[0].size - VT_BLKCAP_BURSTLEN);
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr1_reg, dc2h_cap_boundaryaddr1_reg.regValue);
				dc2h_cap_boundaryaddr2_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr2_reg, dc2h_cap_boundaryaddr2_reg.regValue);

				dc2h_cap_boundaryaddr3_reg.regValue =  ((UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[0].size - VT_BLKCAP_BURSTLEN);
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr3_reg, dc2h_cap_boundaryaddr3_reg.regValue);
				dc2h_cap_boundaryaddr4_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr4_reg, dc2h_cap_boundaryaddr4_reg.regValue);

				dc2h_cap_boundaryaddr5_reg.regValue =  ((UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[0].size - VT_BLKCAP_BURSTLEN);
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr5_reg, dc2h_cap_boundaryaddr5_reg.regValue);
				dc2h_cap_boundaryaddr6_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr6_reg, dc2h_cap_boundaryaddr6_reg.regValue); 
				
				dc2h_cap_boundaryaddr7_reg.regValue =  ((UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[0].size - VT_BLKCAP_BURSTLEN);
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr7_reg, dc2h_cap_boundaryaddr7_reg.regValue);
				dc2h_cap_boundaryaddr8_reg.regValue = (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr8_reg, dc2h_cap_boundaryaddr8_reg.regValue);

				dc2h_cap_boundaryaddr9_reg.regValue =  ((UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[0].size - VT_BLKCAP_BURSTLEN);
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr9_reg, dc2h_cap_boundaryaddr9_reg.regValue);
				dc2h_cap_boundaryaddr10_reg.regValue = (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;
				rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr10_reg, dc2h_cap_boundaryaddr10_reg.regValue);
			}
		}
		else
		{
			//Sequential capture just use 1st boundary address register
			//dc2h_cap_boundaryaddr1_reg.regValue =  ((UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr + (UINT32)CaptureCtrl_VT.cap_buffer[0].size - VT_SEQCAP_BURSTLEN);// sub burst len
			//rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr1_reg, dc2h_cap_boundaryaddr1_reg.regValue);
			//dc2h_cap_boundaryaddr2_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;
			//rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr2_reg, dc2h_cap_boundaryaddr2_reg.regValue);*/

			dc2h_cap_boundaryaddr1_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[sort_boundary_addr_max_index()].phyaddr +  (UINT32)CaptureCtrl_VT.cap_buffer[0].size;
			rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr1_reg, dc2h_cap_boundaryaddr1_reg.regValue);
			dc2h_cap_boundaryaddr2_reg.regValue =  (UINT32)CaptureCtrl_VT.cap_buffer[sort_boundary_addr_min_index()].phyaddr;
			rtd_outl(DC2H_DMA_dc2h_Cap_BoundaryAddr2_reg, dc2h_cap_boundaryaddr2_reg.regValue);
		}
	}
}

void drvif_DC2H_dispD_CaptureConfig(VT_CUR_CAPTURE_INFO capInfo)
{
	if(capInfo.enable == TRUE){
		if (CaptureCtrl_VT.cap_buffer[0].phyaddr == 0) {
			rtd_pr_vt_emerg("[Vt warning]vt no init, so no memory, return setting;%s=%d \n", __FUNCTION__, __LINE__);
			return;
		}

		if ((capInfo.capWid > vt_cap_frame_max_width)||(capInfo.capLen > vt_cap_frame_max_height)) {
			rtd_pr_vt_emerg("[VT warning]drvif_DC2H_dispD_CaptureConfig:could not support max than 2k1k output w=%d,output h=%d! \n", capInfo.capWid,capInfo.capLen);
			return;
		}

		if((get_VT_Pixel_Format() == VT_CAP_NV12)||(get_VT_Pixel_Format() == VT_CAP_NV16)||(get_VT_Pixel_Format() == VT_CAP_NV21))
		{
			dc2h_vi_dc2h_vi_ads_start_y_RBUS DC2H_VI_DC2H_vi_ads_start_y_Reg;
			dc2h_vi_dc2h_vi_ads_start_c_RBUS dc2h_vi_dc2h_vi_ads_start_c_Reg;
			dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_reg;

			dc2h_cap_l3_start_reg.regValue= 0;
			rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg, dc2h_cap_l3_start_reg.regValue);  //used record freeze buffer

			if(Get_PANEL_VFLIP_ENABLE())
			{
				DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue = ((UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr + (vt_cap_frame_max_width*vt_cap_frame_max_height - vt_cap_frame_max_width));
				IoReg_Write32(DC2H_VI_DC2H_vi_ads_start_y_reg, DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue);

				if((get_VT_Pixel_Format() == VT_CAP_NV12) || (get_VT_Pixel_Format() == VT_CAP_NV21))
					dc2h_vi_dc2h_vi_ads_start_c_Reg.regValue = ((UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr + vt_bufAddr_align(vt_cap_frame_max_width*vt_cap_frame_max_height) + (vt_cap_frame_max_width*vt_cap_frame_max_height/2 - vt_cap_frame_max_width));
				else //NV16
					dc2h_vi_dc2h_vi_ads_start_c_Reg.regValue = ((UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr + vt_bufAddr_align(vt_cap_frame_max_width*vt_cap_frame_max_height) + (vt_cap_frame_max_width*vt_cap_frame_max_height - vt_cap_frame_max_width));
				IoReg_Write32(DC2H_VI_DC2H_vi_ads_start_c_reg, dc2h_vi_dc2h_vi_ads_start_c_Reg.regValue);
			}
			else
			{
				DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue = (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;
				IoReg_Write32(DC2H_VI_DC2H_vi_ads_start_y_reg, DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue);

				dc2h_vi_dc2h_vi_ads_start_c_Reg.regValue = (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr + vt_bufAddr_align(vt_cap_frame_max_width*vt_cap_frame_max_height);
				IoReg_Write32(DC2H_VI_DC2H_vi_ads_start_c_reg, dc2h_vi_dc2h_vi_ads_start_c_Reg.regValue);
			}

			//set boundary address 
			set_DC2H_cap_boundaryaddr(get_vt_VtBufferNum());

		}
		else 
		{
			dc2h_dma_dc2h_cap_l1_start_RBUS dc2h_cap_l1_start_reg;
			dc2h_dma_dc2h_cap_l2_start_RBUS dc2h_cap_l2_start_reg;
			dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_reg;

            if(Get_PANEL_VFLIP_ENABLE())
            {
                if(get_VT_Pixel_Format() == VT_CAP_RGB888)
                    dc2h_cap_l1_start_reg.regValue = ((UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr + vt_cap_frame_max_width*vt_cap_frame_max_height*3 - vt_cap_frame_max_width*3);
                else if(get_VT_Pixel_Format() == VT_CAP_ARGB8888 || get_VT_Pixel_Format() == VT_CAP_RGBA8888 || get_VT_Pixel_Format() == VT_CAP_ABGR8888)
                    dc2h_cap_l1_start_reg.regValue = ((UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr + vt_cap_frame_max_width*vt_cap_frame_max_height*4 - vt_cap_frame_max_width*4);
                else
                   rtd_pr_vt_emerg("[error]unknow pixel format\n"); 
            }
            else
                dc2h_cap_l1_start_reg.regValue = (UINT32)CaptureCtrl_VT.cap_buffer[0].phyaddr;

            rtd_outl(DC2H_DMA_dc2h_Cap_L1_Start_reg, dc2h_cap_l1_start_reg.regValue);

            dc2h_cap_l2_start_reg.regValue = 0;
            rtd_outl(DC2H_DMA_dc2h_Cap_L2_Start_reg, dc2h_cap_l2_start_reg.regValue);

            dc2h_cap_l3_start_reg.regValue = 0;
            rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg, dc2h_cap_l3_start_reg.regValue);

            set_DC2H_cap_boundaryaddr(get_vt_VtBufferNum());

		}
		drvif_dc2h_seqmode_conifg(TRUE, capInfo.capWid, capInfo.capLen, capInfo.capSrc);

	} else {
		if((get_VT_Pixel_Format() == VT_CAP_NV12)||(get_VT_Pixel_Format() == VT_CAP_NV16)||(get_VT_Pixel_Format() == VT_CAP_NV21)){
			dc2h_scaledown_dc2h_444to422_ctrl_RBUS dc2h_scaledown_dc2h_444to422_ctrl_reg;

			dc2h_scaledown_dc2h_444to422_ctrl_reg.regValue = rtd_inl(DC2H_SCALEDOWN_DC2H_444to422_CTRL_reg);
			if(VTDumpLocation == KADP_VT_SCALER_OUTPUT)
				dc2h_scaledown_dc2h_444to422_ctrl_reg.cbcr_swap_en =0;
			rtd_outl(DC2H_SCALEDOWN_DC2H_444to422_CTRL_reg, dc2h_scaledown_dc2h_444to422_ctrl_reg.regValue);  //used record freeze buffer
		}
		drvif_dc2h_seqmode_conifg(FALSE, capInfo.capWid, capInfo.capLen, capInfo.capSrc);
	}
}

void vt_reqbuf0_for_src_capture(void)
{

	//rtd_pr_vt_debug("%s\n", __FUNCTION__);
	Capture_BufferMemDeInit_VT(get_vt_VtBufferNum());

	set_vt_function(FALSE);
	rtd_pr_vt_notice("%s Success;%d \n", __FUNCTION__, __LINE__);

}

unsigned char do_vt_reqbufs(unsigned int buf_cnt)
{
	/*for vt_v4l2_ioctl_reqbufs allocate capture buffer memory*/
	if(0 == buf_cnt) /* Release buffers by setting buf_cnt = 0*/
	{

		if((vfod_capture_location == KADP_VT_SCALER_OUTPUT) && (get_video_do_overscan_state()) && (get_src_cap_enable_status() == TRUE))
		{
			vt_reqbuf0_for_src_capture();
			rtd_pr_vt_notice("reqbufs(0)for src se capture\n");
		}

		else if((vfod_capture_location == KADP_VT_DISPLAY_OUTPUT) || (vfod_capture_location == KADP_VT_SCALER_OUTPUT)
			|| (vfod_capture_location == KADP_VT_OSDVIDEO_OUTPUT) || (vfod_capture_location == KADP_VT_SCALER_INPUT))
		{
			if(HAL_VT_Finalize() == FALSE)
			{
				rtd_pr_vt_notice("%s,release buffers fail\n", __FUNCTION__);
				return FALSE;
			}
			rtd_pr_vt_notice("reqbufs(0)for dc2h capture\n");
		}
		else
		{
			rtd_pr_vt_emerg("func:%s,unsupport capture_location\n", __FUNCTION__);
			return FALSE;
		}
		return TRUE;
	}
	else
	{
		down(&VT_Semaphore);

		if (Capture_BufferMemInit_VT(buf_cnt) == FALSE)
		{
			rtd_pr_vt_emerg("VT init allocate memory fail;%s=%d \n", __FUNCTION__, __LINE__);
			up(&VT_Semaphore);
			return FALSE;
		}

		set_vt_VtBufferNum(buf_cnt);
		if(1 == buf_cnt)
			set_vt_VtSwBufferMode(FALSE);
		else
			set_vt_VtSwBufferMode(TRUE);
		set_vt_VtCaptureVDC(FALSE);

		rtd_pr_vt_debug("reqbufs(%d) success\n", buf_cnt);
		up(&VT_Semaphore);
		return TRUE;
	}
}

unsigned char do_vt_streamoff(void)
{
	if(get_vt_function() == TRUE)
	{
		if((vfod_capture_location == KADP_VT_SCALER_OUTPUT) && (get_video_do_overscan_state()) && (get_src_cap_enable_status() == TRUE))
		{
			if(get_vt_src_cap_status() != FALSE)
			{
				set_vt_src_cap_status(FALSE);
				write_Index = 0;
				reset_debug_src_cap_enable_status();
				
				rtd_pr_vt_notice("%s,src se capture", __FUNCTION__);
				reset_vt_src_cap_flags();
			}
			return TRUE;
		}
		else if((vfod_capture_location == KADP_VT_DISPLAY_OUTPUT) || (vfod_capture_location == KADP_VT_SCALER_OUTPUT)
			|| (vfod_capture_location == KADP_VT_OSDVIDEO_OUTPUT) || (vfod_capture_location == KADP_VT_SCALER_INPUT))
		{
			if(get_dc2h_capture_state() == TRUE)
			{
				vt_enable_dc2h(FALSE);
				rtd_pr_vt_notice("%s,dc2h capture", __FUNCTION__);
			}
			return TRUE;
		}
		else
		{
			rtd_pr_vt_emerg("func:%s,unsupport capture_location\n", __FUNCTION__);
			return FALSE;
		}
	}
	else
	{
		rtd_pr_vt_notice("can't do streamoff,%s=%d \n", __FUNCTION__, __LINE__);
		return FALSE;
	}
}

unsigned char do_vt_capture_streamon(void)
{
	ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;
	down(&VT_Semaphore);

	display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);

	if (display_timing_ctrl1_reg.disp_en == 0)/*means d domain clock off, stop capture*/
	{
		rtd_pr_vt_emerg("[Warning Vt]%s=%d D domain clock is off status,could not do vt capture!\n", __FUNCTION__, __LINE__);
		set_vt_function(TRUE);/* KTASKWBS-14031 if VT returns fail in stream on,AP will release memory,But VT finalize run condition is get_vt_function() == TRUE,so set_vt_function(TRUE)*/
		up(&VT_Semaphore);
		return FALSE;
	}

	if(FALSE == get_vt_StreamOn_flag())
	{
		rtd_pr_vt_notice("vt_StreamOn_flag FALSE\n");
		set_vt_function(TRUE);
		up(&VT_Semaphore);
		return FALSE;
	}

	if(get_vt_function() == FALSE)
	{
		set_vt_function(TRUE);

		if((vfod_capture_location == KADP_VT_SCALER_OUTPUT) && (get_video_do_overscan_state() == TRUE)
			&& (get_src_cap_enable_status() == TRUE))
		{
			/*if dumplocation=scaler output and video path has do overscan,
				so for geting no overscan data, will use SE capture from i3ddma or vdec */

			if(do_vt_source_capture_proc() == FALSE)
			{
				rtd_pr_vt_notice("[Fail]vt source capture flow\n");
				up(&VT_Semaphore);
				return FALSE;
			}

			up(&VT_Semaphore);
			return TRUE;
		}
		else if((vfod_capture_location == KADP_VT_DISPLAY_OUTPUT) || (vfod_capture_location == KADP_VT_SCALER_OUTPUT)
			|| (vfod_capture_location == KADP_VT_OSDVIDEO_OUTPUT) || (vfod_capture_location == KADP_VT_SCALER_INPUT))
		{

			curCapInfo.enable =  1; //iMode;
			curCapInfo.capMode = VT_CAP_MODE_SEQ;

			curCapInfo.capSrc =  vfod_capture_location;  //0:scaler output 1:display output 	
			curCapInfo.capWid = vfod_capture_out_W; //iWid;
			curCapInfo.capLen = vfod_capture_out_H;

			drvif_DC2H_dispD_CaptureConfig(curCapInfo);

			VFODState.bEnabled = TRUE;
			VFODState.bAppliedPQ=TRUE;
			VFODState.bFreezed =FALSE;
			VFODState.framerateDivide = get_framerateDivide();
			VTDumpLocation = KADP_VT_DISPLAY_OUTPUT;
			rtd_pr_vt_debug("%s Success\n", __FUNCTION__);

			up(&VT_Semaphore);
			return TRUE;
		}
		else
		{
			rtd_pr_vt_emerg("func:%s,unsupport capture_location\n", __FUNCTION__);
			up(&VT_Semaphore);
			return FALSE;
		}
	}
	else
	{
		rtd_pr_vt_notice("VT aready streamon,do nothing;%s=%d \n", __FUNCTION__, __LINE__);
		up(&VT_Semaphore);
		return TRUE;
	}
}

unsigned char HAL_VT_InitEx(unsigned int buffernum)
{
	ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;
	down(&VT_Semaphore);

    drvif_vt_block_vr_recording(get_vr_function());

	if((buffernum<=0)||(buffernum>5)) {/*max support 5 buffer mode*/
		buffernum = 5;
	}

	rtd_pr_vt_emerg("%s=%d buffernum=%d\n", __FUNCTION__, __LINE__,buffernum);

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
		rtd_pr_vt_emerg("[Warning VT]D domain clock is off status,could not do vt capture!\n");
		up(&VT_Semaphore);
		return FALSE;
	}
#if 1
	if((buffernum == 1)&&(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE)){
		rtd_pr_vt_debug("[Warning VT]Vsc not at _MODE_STATE_ACTIVE status\n");
		//up(&VT_Semaphore);
		//return FALSE;
	}
#endif

	if(((drvif_scaler_get_abnormal_dvs_long_flag()==TRUE)||(drvif_scaler_get_abnormal_dvs_short_flag()==TRUE))
		&& (get_vt_VtBufferNum() == 5)){ /* add vr360 condition(5 buffers),avoid capture out of range */
		rtd_pr_vt_emerg("[Warning VBE][VT] Vbe at abnormal dvs status,could not do vt capture!\n");
		up(&VT_Semaphore);
		return FALSE;
	}
	
	if(get_vt_function()==FALSE) {
		curCapInfo.enable = 1; //iMode;
		curCapInfo.capMode = VT_CAP_MODE_SEQ;

		curCapInfo.capSrc =  vfod_capture_location;
		curCapInfo.capWid = vfod_capture_out_W;
		curCapInfo.capLen = vfod_capture_out_H;
		
		if (Capture_BufferMemInit_VT(buffernum) == FALSE) {
			curCapInfo.enable = 0; //iMode;
			rtd_pr_vt_emerg("VT init allocate memory fail;%s=%d \n", __FUNCTION__, __LINE__);
			up(&VT_Semaphore);
			return FALSE;
		}
		set_vt_function(TRUE);
		set_vt_VtBufferNum(buffernum);
		if(buffernum==1)
			set_vt_VtSwBufferMode(FALSE);
		else
			set_vt_VtSwBufferMode(TRUE);
		set_vt_VtCaptureVDC(FALSE);
		
        //drvif_DC2H_dispD_CaptureConfig(curCapInfo);

		VFODState.bEnabled = TRUE;
		VFODState.bAppliedPQ = TRUE;
		VFODState.bFreezed = FALSE;
		VFODState.framerateDivide = get_framerateDivide();
		//VTDumpLocation = KADP_VT_DISPLAY_OUTPUT;
		rtd_pr_vt_notice("VT init Success;%s=%d \n", __FUNCTION__, __LINE__);
		up(&VT_Semaphore);
		return TRUE;
	} else {
		rtd_pr_vt_notice("VT have inited,do nothing;%s=%d \n", __FUNCTION__, __LINE__);
		up(&VT_Semaphore);
		return TRUE;
	}

}

unsigned char HAL_VT_Init(void)
{
	rtd_pr_vt_emerg("%s=%d \n", __FUNCTION__, __LINE__);

	if(HAL_VT_InitEx(5)==FALSE)
		return FALSE;
	else
		return TRUE;
}

unsigned char HAL_VT_Finalize(void)
{
	down(&VT_Semaphore);
	if(get_vt_function()==TRUE) {
#ifdef ENABLE_VR360_DATA_FS_FLOW
		INPUT_TIMING_INDEX index = get_current_pattern_index();
		if(index == VO_4K2K_60){
			rtd_pr_vt_emerg("[func:%s] enter datafrc proc.\n", __FUNCTION__);
			scaler_vsc_vr360_enter_datafrc_proc();
		}
#endif
		rtd_pr_vt_emerg("%s start\n", __FUNCTION__);
		if(get_dc2h_capture_state() == TRUE)
		{
			curCapInfo.enable = 0; //iMode;
			drvif_DC2H_dispD_CaptureConfig(curCapInfo);
		}

		Capture_BufferMemDeInit_VT(get_vt_VtBufferNum());
		set_vt_VtBufferNum(5);
		IndexOfFreezedVideoFrameBuffer = 0;
		//set_VT_Pixel_Format(VT_CAP_RGB888);
		set_vt_VtSwBufferMode(FALSE);
		VFODState.bEnabled = FALSE;
		VFODState.bAppliedPQ = TRUE;
		VFODState.bFreezed = FALSE;
		VFODState.framerateDivide = 1;
		VTDumpLocation = KADP_VT_MAX_LOCATION;
		set_vt_function(FALSE);
		set_dqbuf_ioctl_fail_flag(FALSE);
		abnormal_dvs_cnt = 0;
		wait_dvs_stable_cnt = 0;
        set_vt_target_fps(0);
		rtd_pr_vt_emerg("%s done\n", __FUNCTION__);
		if ((get_vt_EnableFRCMode()==FALSE))
		{
			//360vr close FRC, so end 360vr need reopen frc
#ifndef CONFIG_MEMC_BYPASS
			//please close memc here
			MEMC_Set_malloc_address(TRUE);
#endif
			set_vt_EnableFRCMode(TRUE);
			
			rtd_pr_vt_emerg("VT finalize EnableFRCMode;%s \n", __FUNCTION__);
			if(get_film_mode_parameter().enable == _DISABLE){
				rtd_pr_vt_notice("[VR360] exit vt_frc_mode, set mc on!!\n");
				if(0 /*(Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE) > 2560) &&
					(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE) > 1440) &&
					(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 490) &&
					(Get_DISPLAY_REFRESH_RATE() < 120)*/){
					Scaler_MEMC_MEMC_CLK_OnOff(0, 1, 1);	//4k 50/60Hz, enable MC clock with mute on
				}
				else{
					if(FALSE == force_enable_two_step_uzu()) /* input 120 memc always bypass */
						Scaler_MEMC_MEMC_CLK_OnOff(1, 1, 1);	//not 4k 50/60Hz, enable MEMC clock with mute on
				}
				if(FALSE == force_enable_two_step_uzu()){
					Scaler_MEMC_outMux(_ENABLE,_ENABLE);	//db on, mux on
				}

				drvif_memc_outBg_ctrl(_DISABLE);
			}

		}

        drvif_vt_reconfig_vr_recording();
		up(&VT_Semaphore);
		return TRUE;
	} else {
		rtd_pr_vt_emerg("VT have finalized,do nothing;%s=%d \n", __FUNCTION__, __LINE__);
		up(&VT_Semaphore);
		return TRUE;
	}

}
#endif

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

	if((get_VT_Pixel_Format() == VT_CAP_NV12)||(get_VT_Pixel_Format() == VT_CAP_NV16)||(get_VT_Pixel_Format() == VT_CAP_NV21)){	
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
	pVideoFrameOutputDeviceLimitationInfo->maxResolution = (KADP_VT_RECT_T){0,0,_DISP_WID,_DISP_LEN};
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
			pVideoFrameBufferProperty->stride = vt_cap_frame_max_width; /* capture buffer width */
			pVideoFrameBufferProperty->width = curCapInfo.capWid;		
		}else if((get_VT_Pixel_Format() == VT_CAP_NV12)||(get_VT_Pixel_Format() == VT_CAP_NV21)){ 
			pVideoFrameBufferProperty->pixelFormat = KADP_VT_VIDEO_FRAME_BUFFER_PIXEL_FORMAT_YUV420_SEMI_PLANAR;
			pVideoFrameBufferProperty->stride = vt_cap_frame_max_width;
			pVideoFrameBufferProperty->width = curCapInfo.capWid;		
		}else if((get_VT_Pixel_Format() == VT_CAP_ABGR8888)||(get_VT_Pixel_Format() == VT_CAP_ARGB8888)||(get_VT_Pixel_Format() == VT_CAP_RGBA8888)) {
			pVideoFrameBufferProperty->pixelFormat = KADP_VT_VIDEO_FRAME_BUFFER_PIXEL_FORMAT_ARGB;
			pVideoFrameBufferProperty->stride =  (vt_cap_frame_max_width * 4);
			pVideoFrameBufferProperty->width = curCapInfo.capWid*4;
		} else {
			pVideoFrameBufferProperty->pixelFormat = KADP_VT_VIDEO_FRAME_BUFFER_PIXEL_FORMAT_RGB;
			pVideoFrameBufferProperty->stride =  (vt_cap_frame_max_width * 3);
			pVideoFrameBufferProperty->width = curCapInfo.capWid*3;
		}

		pVideoFrameBufferProperty->height = curCapInfo.capLen;
		pVideoFrameBufferProperty->ppPhysicalAddress0 = (unsigned int)CaptureCtrl_VT.cap_buffer[0].phyaddr;
		pVideoFrameBufferProperty->ppPhysicalAddress1 = (unsigned int)CaptureCtrl_VT.cap_buffer[1].phyaddr;
		pVideoFrameBufferProperty->ppPhysicalAddress2 = (unsigned int)CaptureCtrl_VT.cap_buffer[2].phyaddr;
		pVideoFrameBufferProperty->ppPhysicalAddress3 = (unsigned int)CaptureCtrl_VT.cap_buffer[3].phyaddr;
		pVideoFrameBufferProperty->ppPhysicalAddress4 = (unsigned int)CaptureCtrl_VT.cap_buffer[4].phyaddr;
		pVideoFrameBufferProperty->vfbbuffernumber = get_vt_VtBufferNum();
		pVideoFrameBufferProperty->vtYbufSize = vt_bufAddr_align((vt_cap_frame_max_width*vt_cap_frame_max_height));
		up(&VT_Semaphore);
		return TRUE;
	} else {
		rtd_pr_vt_notice("VT is not Inited So return False;%s=%d\n", __FUNCTION__, __LINE__);
		up(&VT_Semaphore);
		return FALSE;
	}
}

#ifndef UT_flag
unsigned char HAL_VT_GetVideoFrameBufferIndex(KADP_VT_VIDEO_WINDOW_TYPE_T videoWindowID, unsigned int *pIndexOfCurrentVideoFrameBuffer)
{
	if(videoWindowID != KADP_VT_VIDEO_WINDOW_0) {
		rtd_pr_vt_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	down(&VT_Semaphore);

	if(get_vt_function() == TRUE)
	{
		if(VFODState.bFreezed == TRUE) {
			set_vfod_freezed(FALSE);
			VFODState.bFreezed = FALSE;
		}

		if(VFODState.bFreezed == FALSE) {
			set_vfod_freezed(TRUE);
			VFODState.bFreezed = TRUE;
		}

		if(VFODState.bFreezed == TRUE)
		{
			if(TRUE == get_dqbuf_ioctl_fail_flag())
			{
				up(&VT_Semaphore);
				return FALSE;
			}

			*pIndexOfCurrentVideoFrameBuffer = IndexOfFreezedVideoFrameBuffer;

			rtd_pr_vt_debug("dq_idx=%d\n", IndexOfFreezedVideoFrameBuffer);
		}
	    else  // if VFOD is not freezed,just return the last captured buffer index
		{
			//rtd_pr_vt_notice("VFOD is not freezed\n");

			if((get_VT_Pixel_Format() == VT_CAP_NV12)||(get_VT_Pixel_Format() == VT_CAP_NV16)||(get_VT_Pixel_Format() == VT_CAP_NV21))
			{
				if(get_vt_VtBufferNum()==5) 
				{
					dc2h_vi_dc2h_vi_ads_start_y_RBUS DC2H_VI_DC2H_vi_ads_start_y_Reg;
					unsigned int lastcapbufferaddr = 0;
					DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_ads_start_y_reg);
			
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
								
					if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 0;
					}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 1;
					}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[2].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 2;
					}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[3].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 3;
					}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[4].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 4;
					}
					rtd_pr_vt_debug("++--index = %d\n", *pIndexOfCurrentVideoFrameBuffer);
					//rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
				}
				else if(get_vt_VtBufferNum()==4)
				{
					dc2h_vi_dc2h_vi_ads_start_y_RBUS DC2H_VI_DC2H_vi_ads_start_y_Reg;
					unsigned int lastcapbufferaddr = 0;
					DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_ads_start_y_reg);

					if(DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[0].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[3].phyaddr;
					}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[1].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[0].phyaddr;
					}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[2].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[1].phyaddr;
					}else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[3].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[2].phyaddr;
					}

					if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 0;
					}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 1;
					}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[2].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 2;
					}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[3].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 3;
					}
					//rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
				}
				else if(get_vt_VtBufferNum()==3)
				{
					dc2h_vi_dc2h_vi_ads_start_y_RBUS DC2H_VI_DC2H_vi_ads_start_y_Reg;
					unsigned int lastcapbufferaddr = 0;
					DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_ads_start_y_reg);

		           if(DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[0].phyaddr){
		                   lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[2].phyaddr;
		           }else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[1].phyaddr){
		                   lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[0].phyaddr;
		           }else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[2].phyaddr){
		                   lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[1].phyaddr;
		           }

		           if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr){
		                   *pIndexOfCurrentVideoFrameBuffer = 0;
		           }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr){
		                   *pIndexOfCurrentVideoFrameBuffer = 1;
		           }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[2].phyaddr){
		                   *pIndexOfCurrentVideoFrameBuffer = 2;
		           }       
		          // rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
		        }
				else if(get_vt_VtBufferNum()==2) 
				{
					dc2h_vi_dc2h_vi_ads_start_y_RBUS DC2H_VI_DC2H_vi_ads_start_y_Reg;
					unsigned int lastcapbufferaddr = 0;
					DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue = rtd_inl(DC2H_VI_DC2H_vi_ads_start_y_reg);

		           if(DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[0].phyaddr){
		                   lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[0].phyaddr;
		           }else if (DC2H_VI_DC2H_vi_ads_start_y_Reg.regValue == CaptureCtrl_VT.cap_buffer[1].phyaddr){
		                   lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[1].phyaddr;
		           }

		           if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr){
		                   *pIndexOfCurrentVideoFrameBuffer = 0;
		           }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr){
		                   *pIndexOfCurrentVideoFrameBuffer = 1;
		           }    
		           //rtd_outl(DC2H_DMA_dc2h_Cap_L3_Start_reg,lastcapbufferaddr);
		        }

			}
			else
			{	
				if (get_vt_VtBufferNum() == 5)
				{
					dc2h_dma_dc2h_cap_l1_start_RBUS dc2h_cap_l1_start_Reg;
					unsigned int lastcapbufferaddr = 0;
					dc2h_cap_l1_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L1_Start_db_reg);
				
					//dc2h_dma_dc2h_cap_l3_start_RBUS dc2h_cap_l3_start_Reg;
					//dc2h_cap_l3_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L3_Start_reg);

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
					
					if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 0;
					}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 1;
					}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[2].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 2;
					}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[3].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 3;
					}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[4].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 4;
					}	
				}
				 else if(get_vt_VtBufferNum() == 4)
				 {
					dc2h_dma_dc2h_cap_l1_start_RBUS dc2h_cap_l1_start_Reg;
					unsigned int lastcapbufferaddr = 0;
					dc2h_cap_l1_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L1_Start_db_reg);
							
					if(dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[0].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[3].phyaddr;
					}else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[1].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[0].phyaddr;
					}else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[2].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[1].phyaddr;
					}else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[3].phyaddr){
						lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[2].phyaddr;
					}

					if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 0;
					}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 1;
					}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[2].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 2;
					}else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[3].phyaddr){
						*pIndexOfCurrentVideoFrameBuffer = 3;
					}
				}
				else if(get_vt_VtBufferNum() == 3)
				{
	               dc2h_dma_dc2h_cap_l1_start_RBUS dc2h_cap_l1_start_Reg;
	               unsigned int lastcapbufferaddr = 0;
	               dc2h_cap_l1_start_Reg.regValue = rtd_inl(DC2H_DMA_dc2h_Cap_L1_Start_db_reg);

	               if(dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[0].phyaddr){
	                       lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[2].phyaddr;
	              }else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[1].phyaddr){
	                       lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[0].phyaddr;
	               }else if (dc2h_cap_l1_start_Reg.regValue == CaptureCtrl_VT.cap_buffer[2].phyaddr){
	                       lastcapbufferaddr = CaptureCtrl_VT.cap_buffer[1].phyaddr;
	               }

	               if(lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[0].phyaddr){
	                       *pIndexOfCurrentVideoFrameBuffer = 0;
	               }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[1].phyaddr){
	                       *pIndexOfCurrentVideoFrameBuffer = 1;
	               }else if (lastcapbufferaddr == CaptureCtrl_VT.cap_buffer[2].phyaddr){
	                       *pIndexOfCurrentVideoFrameBuffer = 2;
	               }
	       		}
					
			}
				
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

//#define DEBUG_DUMP_VT_CAP 1
unsigned char do_vt_dqbuf(unsigned int *pdqbuf_Index)
{

#ifdef DEBUG_DUMP_VT_CAP

	// count value for dump debug
	static unsigned int acr_dump_y_count = 0;
#endif

	if(get_vt_function() == FALSE)
	{
		rtd_pr_vt_notice("[fail]func:%s,vt not stream on!!!\n", __FUNCTION__);
		*pdqbuf_Index = 0xFF;
		return FALSE;
	}
	
	// can not use below confition in if, because vt_src_cap_status was reset to false
	//if(get_vt_src_cap_status() == VT_SOURCE_CAP_I3DDMA_BUF || get_vt_src_cap_status() == VT_SOURCE_CAP_VDEC_BUF)
	
	if((vfod_capture_location == KADP_VT_SCALER_OUTPUT) && (get_video_do_overscan_state() == TRUE)
		&& (get_src_cap_enable_status() == TRUE))
	{
		// source capture,acr case
		//if source capture not ready, return FALSE, means notice webos calls v4l2 dqbuf again.
		
		if(get_vt_VtBufferNum() == 1)
		{
			/* check the buffer index capture done ?*/
			if((vt_src_capture_done_flag[write_Index] == 1) && (write_Index == 0))
			{
				*pdqbuf_Index = write_Index;
				
				vt_qbuf_flag[write_Index] = 0;  // this buffer will used by AP client, vt can not capture data to again,until ap qbuf
				vt_src_capture_done_flag[write_Index] = 0;//to prevent ap dqbuf fast
								
#ifdef DEBUG_DUMP_VT_CAP
				/* [debug]for dump	buffer */

				debug_dump_data_to_file(write_Index, vfod_capture_out_W, vfod_capture_out_H);
#endif			
				rtd_pr_vt_notice("one buffer dqbuf done\n");
				return TRUE;
			}
			else
			{
				return FALSE;  //notice ap dqbuf again
			}			
		}
		else
		{
			unsigned int vt_src_capture_done_count = 0;
			vt_src_capture_done_count = vt_src_capture_done_flag[0]+vt_src_capture_done_flag[1]+vt_src_capture_done_flag[2];
			
			if(vt_src_capture_done_count > 1) // at least 1 buffer has capture done 
			{
				
				if(vt_src_capture_done_flag[write_Index] == 1)
				{
					*pdqbuf_Index = write_Index; //dqubf index
					vt_qbuf_flag[write_Index] = 0;  // this buffer will used by AP client, vt can not capture data to again
					
					vt_src_capture_done_flag[write_Index] = 0;

#ifdef DEBUG_DUMP_VT_CAP

					if(acr_dump_y_count < 6)
					{
						/* [debug]for dump  buffer */
				
						debug_dump_data_to_file(write_Index, vfod_capture_out_W, vfod_capture_out_H);
					}

					acr_dump_y_count++;
#endif					
					return TRUE;					
				}
				else
				{
					//if vt source nn buffer capture not ready,means vt_src_capture_done_flag[write_Index] == 0
					// then check last buffer is capture done?
					
					if(write_Index == 0)
						write_Index = (get_vt_VtBufferNum() -1); //last capture done buffer index 
					else
						write_Index = (write_Index - 1); //last capture done buffer index 

					//check this buffer already capture done
					if(vt_src_capture_done_flag[write_Index] == 1)
					{
						*pdqbuf_Index = write_Index; //dqubf index
						vt_qbuf_flag[write_Index] = 0;  // this buffer will used by AP client, vt can not capture data to again
						
						vt_src_capture_done_flag[write_Index] = 0;
						return TRUE;					
					}
					else
					{
						rtd_pr_vt_notice("dqbuf fail\n");
						return FALSE;
					}
				}
			}
			else
			{
				//wait VT source capture done,so return FALSE,make AP client try dqbuf again
				return FALSE;
			}
		}
	}
	else
	{
		//HAL_VT_GetVideoFrameBufferIndex(KADP_VT_VIDEO_WINDOW_0, pdqbuf_Index);
		if(FALSE == HAL_VT_GetVideoFrameBufferIndex(KADP_VT_VIDEO_WINDOW_0, pdqbuf_Index))
		{
			return FALSE;
		}
		else
		{

#ifdef DEBUG_DUMP_VT_CAP
		/*for dump y plane data*/

		debug_dump_data_to_file(IndexOfFreezedVideoFrameBuffer, vfod_capture_out_W, vfod_capture_out_H);
		//debug_dump_data_to_file(CaptureCtrl_VT.cap_buffer[IndexOfFreezedVideoFrameBuffer].phyaddr, vfod_capture_out_W, vfod_capture_out_H);
#endif
			return TRUE;
		}
	}
}
#endif

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

#ifndef UT_flag
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
	down(&VT_Semaphore);
	if(get_vt_function() == TRUE) { 
		
		if (videoFrameOutputDeviceStateFlags&KADP_VT_VIDEO_FRAME_OUTPUT_DEVICE_STATE_ENABLED) {
			if (VFODState.bEnabled != pVideoFrameOutputDeviceState->bEnabled) {
				curCapInfo.enable = pVideoFrameOutputDeviceState->bEnabled;
				VFODState.bEnabled = pVideoFrameOutputDeviceState->bEnabled;
				drvif_DC2H_dispD_CaptureConfig(curCapInfo);
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
					set_vfod_freezed(pVideoFrameOutputDeviceState->bFreezed);
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
#endif

unsigned short Get_VFOD_FrameRate(void)
{
#define XTAL_CLK (27000000)
	unsigned short framerate = 0;

	ppoverlay_dvs_cnt_RBUS ppoverlay_dvs_cnt_Reg;
	ppoverlay_uzudtg_dvs_cnt_RBUS ppoverlay_uzudtg_dvs_cnt_Reg;

	if(VTDumpLocation == KADP_VT_DISPLAY_OUTPUT || VTDumpLocation == KADP_VT_SCALER_OUTPUT || VTDumpLocation == KADP_VT_SCALER_INPUT)
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
#ifndef UT_flag
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
		} else {
			curCapInfo.enable = 0; //close vfod first then rerun
			drvif_DC2H_dispD_CaptureConfig(curCapInfo);
			curCapInfo.enable = 1;
#if 0
			if(dumpLocation==KADP_VT_DISPLAY_OUTPUT)
				curCapInfo.capSrc =  VT_CAP_SRC_VIDEO_PQ;
			else if(dumpLocation==KADP_VT_SCALER_OUTPUT)
				curCapInfo.capSrc =  VT_CAP_SRC_VIDEO_NOPQ;
			else if(dumpLocation==KADP_VT_OSDVIDEO_OUTPUT)
				curCapInfo.capSrc =  VT_CAP_SRC_VIDEO_OSD;
#endif
			curCapInfo.capSrc = (VT_CAP_SRC)dumpLocation;
			drvif_DC2H_dispD_CaptureConfig(curCapInfo);
			if(KADP_VT_SCALER_OUTPUT  ==  dumpLocation)
				VFODState.bAppliedPQ = FALSE;
			else
				VFODState.bAppliedPQ = TRUE;

			VTDumpLocation = dumpLocation;

			if(VFODState.bFreezed == TRUE) {
				set_vfod_freezed(TRUE);//revert freezed status
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
#endif

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

#ifndef UT_flag
unsigned char HAL_VT_SetVideoFrameOutputDeviceOutputRegion(KADP_VT_VIDEO_WINDOW_TYPE_T videoWindowID, KADP_VT_DUMP_LOCATION_TYPE_T dumpLocation, KADP_VT_RECT_T *pOutputRegion)
{

	if (videoWindowID != KADP_VT_VIDEO_WINDOW_0) {
		rtd_pr_vt_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	down(&VT_Semaphore);
	rtd_pr_vt_notice("HAL_VT_SetVideoFrameOutputDeviceOutputRegion:output w=%d,output h=%d,dumplocation=%d\n", pOutputRegion->w,pOutputRegion->h,dumpLocation);

    if ((pOutputRegion->w> _DISP_WID)||(pOutputRegion->w<60)||(pOutputRegion->h> _DISP_LEN)||(pOutputRegion->h<34)) {
        rtd_pr_vt_notice("[warning]OutputRegion over limitation!!!!\n");
        up(&VT_Semaphore);
        return FALSE;
    }

    if((get_VT_Pixel_Format() == VT_CAP_ARGB8888)){
        if(pOutputRegion->w > VT_CAP_FRAME_WIDTH_2K1k || pOutputRegion->h> VT_CAP_FRAME_HEIGHT_2K1k){
            pOutputRegion->w = VT_CAP_FRAME_WIDTH_2K1k;
            pOutputRegion->h = VT_CAP_FRAME_HEIGHT_2K1k;
            rtd_pr_vt_notice("[warning]ARGB OutputRegion only support 1920*1080 capture!!!!\n");
        }
    }

	if(dumpLocation >= KADP_VT_MAX_LOCATION)
	{
		rtd_pr_vt_emerg("[error]%s=%d invalid capture location!\n", __FUNCTION__, __LINE__);
		up(&VT_Semaphore);
		return FALSE;
	}
	if((drvif_scaler_get_abnormal_dvs_long_flag()==TRUE)||(drvif_scaler_get_abnormal_dvs_short_flag()==TRUE)){
		rtd_pr_vt_emerg("[Warning VBE]%s=%d Vbe at abnormal dvs status,could not do vt capture!\n", __FUNCTION__, __LINE__);
		up(&VT_Semaphore);
		return FALSE;
	}

	if(get_vt_function() == TRUE) {
		if((dumpLocation==VTDumpLocation)&&(pOutputRegion->w==curCapInfo.capWid)&&(pOutputRegion->h==curCapInfo.capLen)) {
			rtd_pr_vt_notice("all SetVideoFrameOutputDeviceOutputRegion is the same with now\n");
		} else if((pOutputRegion->x!=0)||(pOutputRegion->y!=0)) {
			rtd_pr_vt_notice("[warning]only support output region x y is 0\n");
			up(&VT_Semaphore);
			return FALSE;
		} else {
			curCapInfo.enable = 0; //close vfod first then rerun
			drvif_DC2H_dispD_CaptureConfig(curCapInfo);
			if((pOutputRegion->w > vt_cap_frame_max_width) || (pOutputRegion->h > vt_cap_frame_max_height))
			{
				//if target capture size large than vt allocate buffer size
				// 1st release VT allocate buffer memory, HW  capture has already disable

				Capture_BufferMemDeInit_VT(get_vt_VtBufferNum());

				//2nd vt use new size to allocate memory
				set_cap_buffer_size_by_AP(pOutputRegion->w, pOutputRegion->h);

				//re-allocate memory with size that AP set
				if(Capture_BufferMemInit_VT(get_vt_VtBufferNum()) == FALSE)
				{
					rtd_pr_vt_notice("VT init allocate memory fail,%s=%d \n", __FUNCTION__, __LINE__);
					up(&VT_Semaphore);
					return FALSE;
				}
			}

			curCapInfo.capSrc = (VT_CAP_SRC)dumpLocation;
			curCapInfo.capWid = pOutputRegion->w;
			curCapInfo.capLen = pOutputRegion->h;
			set_cap_buffer_size_by_AP(curCapInfo.capWid,curCapInfo.capLen); 
			
			curCapInfo.enable = 1;
			drvif_DC2H_dispD_CaptureConfig((VT_CUR_CAPTURE_INFO)curCapInfo);
			if(KADP_VT_SCALER_OUTPUT == dumpLocation || KADP_VT_SCALER_INPUT == dumpLocation)
				VFODState.bAppliedPQ = FALSE;
			else
				VFODState.bAppliedPQ = TRUE;

			VTDumpLocation = dumpLocation;

			if(VFODState.bFreezed == TRUE)
			{
				set_vfod_freezed(TRUE);//revert freezed status
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

unsigned char HAL_VT_WaitVsync(KADP_VT_VIDEO_WINDOW_TYPE_T videoWindowID)
{
	if(videoWindowID != KADP_VT_VIDEO_WINDOW_0) {
		rtd_pr_vt_notice("videoWindowID id is not ok;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	if(get_vt_function() == TRUE) {
		unsigned int sleeptime;

		unsigned int vtframerate;
#ifndef CONFIG_SUPPORT_SCALER_MODULE
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
		struct timespec64 timeout_s;
#else
		struct timespec timeout_s;
#endif
#endif
		unsigned int waitvsyncframerate;
        vtframerate = Get_VFOD_FrameRate();/*Get_VFOD_FrameRate not return 0*/

        if(get_vt_target_fps() == 0)
        {
            if(VFODState.framerateDivide != 0)
                waitvsyncframerate = vtframerate/VFODState.framerateDivide;
            else
                waitvsyncframerate = vtframerate;
        }
        else
        {
            waitvsyncframerate = get_vt_target_fps();
            if(waitvsyncframerate > vtframerate)
                waitvsyncframerate = vtframerate;
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
		//rtd_pr_vt_notice("VT is not Inited So return False;%s=%d\n", __FUNCTION__, __LINE__);
		return FALSE;
	}

}
#endif

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

#ifndef UT_flag
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
		if(get_dc2h_capture_state() == TRUE)
		{
			reset_dc2h_hw_setting(); /*stop and reset dc2h */
		}
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
		if(get_dc2h_capture_state() == TRUE)
		{
			reset_dc2h_hw_setting(); /*stop and reset dc2h */
		}
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
			rtd_pr_vt_notice("%s=%d\n", __FUNCTION__, value);
			set_VT_Pixel_Format(value);
			/*close vt, and release last pixel format buffer start*/
			curCapInfo.enable = 0; //iMode;
			drvif_DC2H_dispD_CaptureConfig(curCapInfo);
			Capture_BufferMemDeInit_VT(get_vt_VtBufferNum());
			/*close vt, and release last pixel format buffer end*/

			/*open vt, and allocate new pixel format buffer start*/
			//curCapInfo.enable = 1; //iMode;		
			if (Capture_BufferMemInit_VT(get_vt_VtBufferNum()) == FALSE) {
				rtd_pr_vt_notice("VT init allocate memory fail when set_VT_Pixel_Format;%s=%d \n", __FUNCTION__, __LINE__);
				up(&VT_Semaphore);
				return FALSE;
			}
			//drvif_DC2H_dispD_CaptureConfig(curCapInfo);	
			/*open vt, and allocate new pixel format buffer end*/
		}
		up(&VT_Semaphore);
		return TRUE;
	} else {
		up(&VT_Semaphore);
		return FALSE;
	}
}


void drvif_memc_outBg_ctrl(bool enable)
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
		rtd_pr_vt_notice("memc_out_bg_en apply timeout\n");
	}

}

unsigned char HAL_VT_EnableFRCMode(unsigned char bEnableFRC)
{
	down(&VT_Semaphore);
	rtd_pr_vt_notice("+++ %s=%d bEnableFRC=%d!\n", __FUNCTION__, __LINE__,bEnableFRC);

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
				if(1 /*Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 490*/){
					Check_smooth_toggle_update_flag(SLR_MAIN_DISPLAY);
					drvif_memc_outBg_ctrl(_ENABLE); /* memc mute black */
					//Scaler_MEMC_outMux(_ENABLE,_DISABLE); //db on, mux off
					//Scaler_MEMC_Bypass_On(_ENABLE);
					//rtd_pr_vt_notice("[VR360] memc bypass done!!\n");
					Scaler_MEMC_MEMC_CLK_OnOff(0, 0, 1);	//bypass MEMC with mute on
					rtd_pr_vt_notice("[VR360]memc mute and disable clock done!\n");
				}
				else{
					rtd_pr_vt_notice("[VR360] 4k timing enter vt_frc_mode, set mc on!!\n");
					Check_smooth_toggle_update_flag(SLR_MAIN_DISPLAY);
					Scaler_MEMC_MEMC_CLK_OnOff(0, 1, 1);	//Set MC on with mute on
					Scaler_MEMC_outMux(_ENABLE,_ENABLE);	//db on, mux on
					rtd_pr_vt_notice("[VR360] memc enable done!!\n");
				}
			}
			else if(Scaler_get_vdec_2k120hz()){
				rtd_pr_vt_notice("[VR360] 2k120Hz enter vt_frc_mode, set memc bypass!!\n");
				Scaler_MEMC_outMux(_ENABLE,_DISABLE);	//db on, mux off
				Scaler_MEMC_MEMC_CLK_OnOff(0, 0, 1);	// bypass MEMC with mute on
			}
			else{
				rtd_pr_vt_notice("[VR360] not 4k timing enter vt_frc_mode, set memc on!!\n");
				Scaler_MEMC_MEMC_CLK_OnOff(1, 1, 1);	//Set MEMC on with mute on
				Scaler_MEMC_outMux(_ENABLE,_ENABLE);	//db on, mux off
			}
		}
		//EnableFRCMode is FALSE,pls close FRC
#ifndef CONFIG_MEMC_BYPASS
		MEMC_Set_malloc_address(bEnableFRC);
#endif
	}
	else{
		//EnableFRCMode is TRUE,pls reopen FRC,however,webos never set HAL_VT_EnableFRCMode(TRUE), so need reopen at HAL_VT_Finalize
#ifndef CONFIG_MEMC_BYPASS
		MEMC_Set_malloc_address(bEnableFRC);
#endif
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

unsigned char HAL_VT_Set_OutFps(unsigned int outfps)
{
    uiTargetFps = outfps;
    rtd_pr_vt_notice("ap set outfps=%d\n", uiTargetFps);
    return TRUE;
}


/************************************start of VT buffer show by sub display function********************************/
//#define _BURSTLENGTH2 	(0x78) // 0x78 (120) for sub-channel
//#define _FIFOLENGTH2 	(0x80) //sub path FIFO
void memory_set_sub_displaywindow_DispVT(VIDEO_RECT_T s_dispwin)
{
	mdomain_disp_ddr_sublinestep_RBUS mdomain_disp_ddr_sublinestep_reg;
	mdomain_disp_ddr_sublinenum_RBUS mdomain_disp_ddr_sublinenum_reg;
	mdomain_disp_ddr_subrdnumlenrem_RBUS mdomain_disp_ddr_subrdnumlenrem_reg;
	mdomain_disp_ddr_subpixnumwtl_RBUS mdomain_disp_ddr_subpixnumwtl_reg;
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
	mdomain_disp_ddr_subpixnumwtl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_SubPixNumWTL_reg);
	mdomain_disp_ddr_subpixnumwtl_reg.sub_pixel_num = (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_WID) +droppixel) & 0x3FFF;
	mdomain_disp_ddr_subpixnumwtl_reg.sub_wtlvl = ((fifoLen - (burstLen>>1))>>1);
	IoReg_Write32(MDOMAIN_DISP_DDR_SubPixNumWTL_reg, mdomain_disp_ddr_subpixnumwtl_reg.regValue);

	memory_division(TotalSize, burstLen, &Quotient, &Remainder);	// Do a division

	// remainder is not allowed to be zero
	if (Remainder == 0) {
		Remainder = burstLen;
		Quotient -= 1;
	}


	if((dvrif_memory_compression_get_enable(SLR_SUB_DISPLAY) == TRUE) && (dvrif_memory_get_compression_mode(SLR_SUB_DISPLAY) == COMPRESSION_FRAME_MODE))
	{
		mdomain_disp_ddr_subrdnumlenrem_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_SubRdNumLenRem_reg);
		mdomain_disp_ddr_subrdnumlenrem_reg.sub_read_num1 = 0xffff;
		mdomain_disp_ddr_subrdnumlenrem_reg.sub_read_len = burstLen;
		mdomain_disp_ddr_subrdnumlenrem_reg.sub_read_remain = burstLen;
		IoReg_Write32(MDOMAIN_DISP_DDR_SubRdNumLenRem_reg, mdomain_disp_ddr_subrdnumlenrem_reg.regValue);
	}
	else
	{
		mdomain_disp_ddr_subrdnumlenrem_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_SubRdNumLenRem_reg);
		mdomain_disp_ddr_subrdnumlenrem_reg.sub_read_num1 = Quotient;
		mdomain_disp_ddr_subrdnumlenrem_reg.sub_read_len = burstLen;
		mdomain_disp_ddr_subrdnumlenrem_reg.sub_read_remain = Remainder;
		IoReg_Write32(MDOMAIN_DISP_DDR_SubRdNumLenRem_reg, mdomain_disp_ddr_subrdnumlenrem_reg.regValue);
	}

	TotalSize = memory_get_capture_size(display, MEMCAPTYPE_LINE);
	/*TotalSize = Quotient * burstLen + Remainder; 	// count one line, 64bits unit
	TotalSize = drvif_memory_get_data_align(TotalSize, 4);	// times of 4*/

	mdomain_disp_ddr_sublinestep_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_SubLineStep_reg);
	mdomain_disp_ddr_sublinestep_reg.sub_line_step = (TotalSize + TotalSize%2);//bit 3 need to be 0. rbus rule
	IoReg_Write32(MDOMAIN_DISP_DDR_SubLineStep_reg, mdomain_disp_ddr_sublinestep_reg.regValue);

	TotalSize = SHL(TotalSize, 3); // unit conversion from 64bits to 8bits

	mdomain_disp_ddr_sublinenum_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_SubLineNum_reg);
	mdomain_disp_ddr_sublinenum_reg.sub_line_number = (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_LEN) & 0x0FFF);
	IoReg_Write32(MDOMAIN_DISP_DDR_SubLineNum_reg, mdomain_disp_ddr_sublinenum_reg.regValue);

	rtd_outl(MDOMAIN_DISP_DDR_SubAddr_reg, (CaptureCtrl_VT.cap_buffer[0].phyaddr + MemShiftAddr) &0xfffffff0);
	rtd_outl(MDOMAIN_DISP_DDR_Sub2ndAddr_reg, (CaptureCtrl_VT.cap_buffer[1].phyaddr + MemShiftAddr) &0xfffffff0);
	rtd_outl(MDOMAIN_DISP_DDR_Sub3rdAddr_reg, (CaptureCtrl_VT.cap_buffer[2].phyaddr + MemShiftAddr) &0xfffffff0);

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
			
			PipmpSetSubDisplayWindow(
				outputwin.x, outputwin.x + Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_DISP_WID) - 1,
				outputwin.y, outputwin.y + Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_DISP_LEN) - 1,
				0);

			PipmpSetSubActiveWindow(
				0, Scaler_DispGetInputInfoByDisp(SLR_SUB_DISPLAY,SLR_INPUT_DISP_WID)  - 1,
				0, Scaler_DispGetInputInfoByDisp(SLR_SUB_DISPLAY,SLR_INPUT_DISP_LEN) - 1);

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


void memory_set_displaysubwindow_DispVT(VIDEO_RECT_T s_dispwin, unsigned long phyaddr)
{
	mdomain_disp_ddr_sublinestep_RBUS mdomain_disp_ddr_sublinestep_reg;
	mdomain_disp_ddr_sublinenum_RBUS mdomain_disp_ddr_sublinenum_reg;
	mdomain_disp_ddr_subrdnumlenrem_RBUS mdomain_disp_ddr_subrdnumlenrem_reg;
	mdomain_disp_ddr_subpixnumwtl_RBUS mdomain_disp_ddr_subpixnumwtl_reg;
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

	mdomain_disp_ddr_subpixnumwtl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_SubPixNumWTL_reg);
	mdomain_disp_ddr_subpixnumwtl_reg.sub_pixel_num = (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_WID) +droppixel) & 0x3FFF;
	mdomain_disp_ddr_subpixnumwtl_reg.sub_wtlvl = ((fifoLen - (burstLen>>1))>>1);
	IoReg_Write32(MDOMAIN_DISP_DDR_SubPixNumWTL_reg, mdomain_disp_ddr_subpixnumwtl_reg.regValue);

	memory_division(TotalSize, burstLen, &Quotient, &Remainder);	// Do a division

	// remainder is not allowed to be zero
	if (Remainder == 0) {
		Remainder = burstLen;
		Quotient -= 1;
	}

	if((dvrif_memory_compression_get_enable(SLR_SUB_DISPLAY) == TRUE) && (dvrif_memory_get_compression_mode(SLR_SUB_DISPLAY) == COMPRESSION_FRAME_MODE))
	{
		mdomain_disp_ddr_subrdnumlenrem_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_SubRdNumLenRem_reg);
		mdomain_disp_ddr_subrdnumlenrem_reg.sub_read_num1 = 0xffff;
		mdomain_disp_ddr_subrdnumlenrem_reg.sub_read_len = burstLen;
		mdomain_disp_ddr_subrdnumlenrem_reg.sub_read_remain = burstLen;
		IoReg_Write32(MDOMAIN_DISP_DDR_SubRdNumLenRem_reg, mdomain_disp_ddr_subrdnumlenrem_reg.regValue);
	}
	else
	{
		mdomain_disp_ddr_subrdnumlenrem_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_SubRdNumLenRem_reg);
		mdomain_disp_ddr_subrdnumlenrem_reg.sub_read_num1 = Quotient;
		mdomain_disp_ddr_subrdnumlenrem_reg.sub_read_len = burstLen;
		mdomain_disp_ddr_subrdnumlenrem_reg.sub_read_remain = Remainder;
		IoReg_Write32(MDOMAIN_DISP_DDR_SubRdNumLenRem_reg, mdomain_disp_ddr_subrdnumlenrem_reg.regValue);
	}


	TotalSize = memory_get_capture_size(display, MEMCAPTYPE_LINE);
	/*TotalSize = Quotient * burstLen + Remainder; 	// count one line, 64bits unit
	TotalSize = drvif_memory_get_data_align(TotalSize, 4);	// times of 4*/


	mdomain_disp_ddr_sublinestep_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_SubLineStep_reg);
	mdomain_disp_ddr_sublinestep_reg.sub_line_step = (TotalSize + TotalSize%2);//bit 3 need to be 0. rbus rule
	IoReg_Write32(MDOMAIN_DISP_DDR_SubLineStep_reg, mdomain_disp_ddr_sublinestep_reg.regValue);

	TotalSize = SHL(TotalSize, 3); // unit conversion from 64bits to 8bits

	mdomain_disp_ddr_sublinenum_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_SubLineNum_reg);
	mdomain_disp_ddr_sublinenum_reg.sub_line_number = (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_LEN) & 0x0FFF);
	IoReg_Write32(MDOMAIN_DISP_DDR_SubLineNum_reg, mdomain_disp_ddr_sublinenum_reg.regValue);

#if 1
	rtd_outl(MDOMAIN_DISP_DDR_SubAddr_reg, (phyaddr + MemShiftAddr) &0xfffffff0);
#else
	rtd_outl(MDOMAIN_DISP_DDR_SubAddr_reg, (CaptureCtrl_VT.cap_buffer[0].phyaddr + MemShiftAddr) & 0xfffffff0);
	rtd_outl(MDOMAIN_DISP_DDR_Sub2ndAddr_reg, (CaptureCtrl_VT.cap_buffer[1].phyaddr + MemShiftAddr) & 0xfffffff0);
	rtd_outl(MDOMAIN_DISP_DDR_Sub3rdAddr_reg, (CaptureCtrl_VT.cap_buffer[2].phyaddr + MemShiftAddr) & 0xfffffff0);
#endif

	rtd_outl(MDOMAIN_DISP_DDR_SubAddrDropBits_reg,droppixel);

//	subctrltemp = _BIT6;
//	subctrltemp |=_BIT7;//set Mdomain display triple buffer
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

unsigned char Scaler_Rowdata_SubMDispWindow(KADP_VT_ROWDATA_SHOW_ONSUBDISP_T *VT_ROWDATA_SHOW_ONSUBDISP)
{
		ppoverlay_memc_mux_ctrl_RBUS memc_mux_ctrl_reg;
		SCALER_DISP_CHANNEL display = SLR_SUB_DISPLAY;
		mdomain_disp_ddr_mainsubctrl_RBUS mdomain_disp_ddr_mainsubctrl_Reg;
		ppoverlay_double_buffer_ctrl_RBUS ppoverlay_double_buffer_ctrl_Reg;
		scaleup_ds_uzu_db_ctrl_RBUS scaleup_ds_uzu_db_ctrl_Reg;

		memc_mux_ctrl_reg.regValue = rtd_inl(PPOVERLAY_MEMC_MUX_CTRL_reg);
		memc_mux_ctrl_reg.memc_out_bg_en = 0;
		rtd_outl(PPOVERLAY_MEMC_MUX_CTRL_reg, memc_mux_ctrl_reg.regValue);
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
		if(VT_ROWDATA_SHOW_ONSUBDISP->enable == TRUE) {
			unsigned short VTCapLen = VT_ROWDATA_SHOW_ONSUBDISP->height;
			unsigned short VTCapWid = VT_ROWDATA_SHOW_ONSUBDISP->width;
			VIDEO_RECT_T outputwin;
			yuv2rgb_d_yuv2rgb_control_RBUS yuv2rgb_d_yuv2rgb_control_Reg;

			outputwin.x = VT_ROWDATA_SHOW_ONSUBDISP->OutputRegion.x;
			outputwin.y = VT_ROWDATA_SHOW_ONSUBDISP->OutputRegion.y;
			outputwin.w = VT_ROWDATA_SHOW_ONSUBDISP->OutputRegion.w;
			outputwin.h = VT_ROWDATA_SHOW_ONSUBDISP->OutputRegion.h;

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

			Scaler_DispSetInputInfoByDisp(display, SLR_INPUT_DISP_WID, outputwin.w);
			Scaler_DispSetInputInfoByDisp(display, SLR_INPUT_DISP_LEN, outputwin.h);

			if(Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_WID) <= Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_DISP_WID)){
				Scaler_DispSetScaleStatus(display, SLR_SCALE_H_UP, TRUE);
			}else{
				Scaler_DispSetScaleStatus(display, SLR_SCALE_H_UP, FALSE);
			}

			if(Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_LEN) <= Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_DISP_LEN))
				Scaler_DispSetScaleStatus(display, SLR_SCALE_V_UP, TRUE);
			else
				Scaler_DispSetScaleStatus(display, SLR_SCALE_V_UP, FALSE);

			memory_set_displaysubwindow_DispVT(outputwin, VT_ROWDATA_SHOW_ONSUBDISP->phyaddr);
			
			/*sub uzu*/
			magnifier_color_ultrazoom_config_scaling_up(display);

			/*sub yuv2rgb*/
			yuv2rgb_d_yuv2rgb_control_Reg.regValue = rtd_inl(YUV2RGB_D_YUV2RGB_Control_reg);
			yuv2rgb_d_yuv2rgb_control_Reg.yuv2rgb_overlay = 1;
			yuv2rgb_d_yuv2rgb_control_Reg.yuv2rgb_sub_en = 0;
			rtd_outl(YUV2RGB_D_YUV2RGB_Control_reg, yuv2rgb_d_yuv2rgb_control_Reg.regValue);
			
			PipmpSetSubDisplayWindow(
				outputwin.x, outputwin.x + Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_DISP_WID) - 1,
				outputwin.y, outputwin.y + Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_DISP_LEN) - 1,
				0);

			PipmpSetSubActiveWindow(
				0, Scaler_DispGetInputInfoByDisp(SLR_SUB_DISPLAY,SLR_INPUT_DISP_WID)  - 1,
				0, Scaler_DispGetInputInfoByDisp(SLR_SUB_DISPLAY,SLR_INPUT_DISP_LEN) - 1);

			Scaler_disp_setting(display);

			set_display_forcebg_mask(SLR_SUB_DISPLAY, SLR_FORCE_BG_TYPE_VSC, FALSE);
			mute_control(SLR_SUB_DISPLAY, FALSE);		
		} else {
			rtd_outl(MDOMAIN_DISP_Disp_sub_enable_reg, 0x00000000);

			down(get_forcebg_semaphore());
			drvif_scalerdisplay_enable_display(SLR_SUB_DISPLAY, _DISABLE);
			up(get_forcebg_semaphore());
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

	if ((VSC_INPUTSRC_AVD == srctype))
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

//#ifndef UT_flag
unsigned char Check_DisplayFrame_isRightColorbar(void){
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

unsigned char debug_dump_data_to_file(unsigned int idx, unsigned int dump_w,unsigned int dump_h)
{
	struct file* filp = NULL;
	unsigned long outfileOffset = 0;
	unsigned int size = VT_CAPTURE_BUFFER_UNITSIZE;
	unsigned char *pVirStartAdrr = NULL;
	static unsigned int g_ulFileCount = 0;
	unsigned int src_phy = CaptureCtrl_VT.cap_buffer[idx].phyaddr;
	char raw_dat_path[30];
	sprintf(raw_dat_path, "/tmp/vt_dump_%d_%d.raw", idx, g_ulFileCount);
	pVirStartAdrr = (unsigned char *)dvr_remap_uncached_memory(src_phy, size, __builtin_return_address(0));

	if((get_VT_Pixel_Format() == VT_CAP_NV12) || (get_VT_Pixel_Format() == VT_CAP_NV16)||(get_VT_Pixel_Format() == VT_CAP_NV21))
	{
        rtd_pr_vt_notice("[VT]y buf offset=%lx\n", vt_bufAddr_align(vt_cap_frame_max_width*vt_cap_frame_max_height));
        if (pVirStartAdrr != 0)
		{
			filp = file_open(raw_dat_path, O_RDWR | O_CREAT | O_APPEND, 0);
			if (filp == NULL)
			{
				rtd_pr_vt_emerg("[VT]file open Y fail\n");
				dvr_unmap_memory(pVirStartAdrr, size);
				return FALSE;
			}
            rtd_pr_vt_notice("[VT]dump y buf addr=%lx\n", (unsigned long)pVirStartAdrr);
			file_write(filp, outfileOffset, pVirStartAdrr, (dump_w * dump_h));//Y plane
			file_sync(filp);
			file_close(filp);

			filp = file_open(raw_dat_path, O_RDWR | O_CREAT | O_APPEND, 0);
			if (filp == NULL)
			{
				rtd_pr_vt_emerg("[VT]file open fail\n");
				dvr_unmap_memory(pVirStartAdrr, size);
				return FALSE;
			}

             rtd_pr_vt_notice("[VT]dump c buf addr=%lx\n", (unsigned long)(pVirStartAdrr + vt_bufAddr_align(vt_cap_frame_max_width*vt_cap_frame_max_height)));
			if((get_VT_Pixel_Format() == VT_CAP_NV12)||(get_VT_Pixel_Format() == VT_CAP_NV21))
				file_write(filp, outfileOffset, (unsigned char *)(pVirStartAdrr+vt_bufAddr_align(vt_cap_frame_max_width*vt_cap_frame_max_height)), ((dump_w * dump_h)/2));//nv12 UV plane
			else
				file_write(filp, outfileOffset, (unsigned char *)(pVirStartAdrr+vt_bufAddr_align(vt_cap_frame_max_width*vt_cap_frame_max_height)), (dump_w * dump_h));//nv16 UV plane
			file_sync(filp);
			file_close(filp);

			rtd_pr_vt_emerg("[VT]dump_finish\n");
			g_ulFileCount++;
			dvr_unmap_memory(pVirStartAdrr, size);
			return TRUE;
		}
		else {
			rtd_pr_vt_emerg("\n\n\n\n ***************** [VT]dump_to_file NG for null buffer address	*********************\n\n\n\n");
			return FALSE;
		}
	}
	else if(get_VT_Pixel_Format() == VT_CAP_ARGB8888 || get_VT_Pixel_Format() == VT_CAP_ABGR8888 || get_VT_Pixel_Format() == VT_CAP_RGBA8888 || get_VT_Pixel_Format() == VT_CAP_RGB888)
	{
		if (pVirStartAdrr != 0)
		{
			filp = file_open(raw_dat_path, O_RDWR | O_CREAT, 0);
			if (filp == NULL) 
			{
				rtd_pr_vt_emerg("[VT]file open fail\n");
				dvr_unmap_memory(pVirStartAdrr, size);
				return FALSE;
			}

			if(get_VT_Pixel_Format() == VT_CAP_RGB888)
				file_write(filp, outfileOffset, pVirStartAdrr, (dump_w * dump_h*3));
			else
				file_write(filp, outfileOffset, pVirStartAdrr, (dump_w * dump_h*4));
			file_sync(filp);
			file_close(filp);
			
			rtd_pr_vt_emerg("[VT]dump_finish\n");
			g_ulFileCount++;
			dvr_unmap_memory(pVirStartAdrr, size);
			return TRUE;
		}
		else {
			rtd_pr_vt_emerg("\n\n\n\n ***************** [VT]dump_to_file NG for null buffer address	*********************\n\n\n\n");
			return FALSE;
		}
	}
	else
	{
		rtd_pr_vt_emerg("[VT]invalid pixelfmt\n");
		dvr_unmap_memory(pVirStartAdrr, size);
		return TRUE;
	}

}

/* =======================================VT_BUFFER_DUMP_DEBUG======================================================*/


unsigned int VIVT_PM_REGISTER_DATA[][2]
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
#endif

#ifdef CONFIG_PM
static int vivt_suspend (struct device *p_dev)
{
	//int i,size;

	if(get_vt_function() == TRUE)  /*WOSQRTK-13830 tvpowerd suspend call, stop dc2h capture */
	{
		vt_enable_dc2h(FALSE);
		rtd_pr_vt_emerg("%s=%d\n", __FUNCTION__, __LINE__);
	}
	else
		rtd_pr_vt_emerg("%s=%d\n", __FUNCTION__, __LINE__);
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

#ifndef UT_flag
int vivt_open(struct inode *inode, struct file *filp) {
	return 0;
}

ssize_t  vivt_read(struct file *filep, char *buffer, size_t count, loff_t *offp)
{
	return 0;

}

ssize_t vivt_write(struct file *filep, const char *buffer, size_t count, loff_t *offp)
{
	long ret = count;
	char cmd_buf[100] = {0};

#ifdef ENABLE_VT_TEST_CASE_CONFIG
	unsigned int cap_idx = 0;
#endif

#ifndef CONFIG_ARM64
	rtd_pr_vt_notice("%s(): count=%d, buf=0x%08lx\n", __func__, count, (long)buffer);
#endif

	if (count >= 100)
		return -EFAULT;

	if (copy_from_user(cmd_buf, buffer, count)) {
		ret = -EFAULT;
	}

	if(count > 0) {
	   cmd_buf[count] = '\0';
	}

	if ((cmd_buf[0] == 'v') && (cmd_buf[1] == 't') && (cmd_buf[2] == '-') && (cmd_buf[3] == '-') && (cmd_buf[4] == 'h')
			&& (cmd_buf[5] == 'e') && (cmd_buf[6] == 'l') && (cmd_buf[7] == 'p'))
	{	 /* echo vt--help > /dev/vivtdev */
		rtd_pr_vt_notice("[VT] capture szie:echo vtsize wid len > /dev/vivtdev\n");
		rtd_pr_vt_notice("[VT] set pixelfmt:echo vtfmt=0 > /dev/vivtdev\n");
		rtd_pr_vt_notice("[VT] vtfmt:0(RGB888),1(ARGB8888),2(RGBA8888),3(ABGR8888),4(NV12),5(NV16),6(NV21)\n");
		rtd_pr_vt_notice("[VT] vttest case location buffernum, ex:echo vttest 1 1 1 > /dev/vivtdev\n");
		rtd_pr_vt_notice("[VT] case:0(DISABLE),1(single capture),2(ACR),3(VR360),4(overscan)\n");
		rtd_pr_vt_notice("[VT] location:0(Scaler_Input),1(Scaler_Output),2(Display_Output),3(Blended_Output),4(Osd_Output)\n");
		rtd_pr_vt_notice("[VT] buffernum:1(single capture),3(ACR),5(VR360)\n");
		rtd_pr_vt_notice("[VT] dump raw:echo vtdump=1 > /dev/vivtdev\n");
		rtd_pr_vt_notice("[VT] exit VT testcase: echo vttest 0 > /dev/vivtdev\n");
	}
#ifdef ENABLE_VT_TEST_CASE_CONFIG
	else if((cmd_buf[0] == 'v') && (cmd_buf[1] == 't') && (cmd_buf[2] == 's') && (cmd_buf[3] == 'i') && (cmd_buf[4] == 'z')
			&& (cmd_buf[5] == 'e'))
	{
		 /* echo vtsize wid len > /dev/vivtdev, ex: echo vtsize 1920 1080 > /dev/vivtdev */
		if((cmd_buf[10] <= '9') && (cmd_buf[10] >= '0'))
		{
			vfod_capture_out_W = ((cmd_buf[7]-0x30)*1000 + (cmd_buf[8]-0x30)*100 + (cmd_buf[9]-0x30)*10 + (cmd_buf[10]-0x30)*1);
			if((cmd_buf[15] <= '9') && (cmd_buf[15] >= '0'))
				vfod_capture_out_H = ((cmd_buf[12]-0x30)*1000 + (cmd_buf[13]-0x30)*100 + (cmd_buf[14]-0x30)*10 + (cmd_buf[15]-0x30)*1);
			else
				vfod_capture_out_H = ((cmd_buf[12]-0x30)*100 + (cmd_buf[13]-0x30)*10 + (cmd_buf[14]-0x30)*1);
		}	
		else
		{
			vfod_capture_out_W = ((cmd_buf[7]-0x30)*100 + (cmd_buf[8]-0x30)*10 + (cmd_buf[9]-0x30)*1);
			if((cmd_buf[14] <= '9') && (cmd_buf[14] >= '0'))
				vfod_capture_out_H = ((cmd_buf[11]-0x30)*1000 + (cmd_buf[12]-0x30)*100 + (cmd_buf[13]-0x30)*10 + (cmd_buf[14]-0x30)*1);
			else
				vfod_capture_out_H = ((cmd_buf[11]-0x30)*100 + (cmd_buf[12]-0x30)*10 + (cmd_buf[13]-0x30)*1);
		}
			
		set_cap_buffer_size_by_AP(vfod_capture_out_W,vfod_capture_out_H);
	}
	else if((cmd_buf[0] == 'v') && (cmd_buf[1] == 't') && (cmd_buf[2] == 'f') && (cmd_buf[3] == 'm') && (cmd_buf[4] == 't')
			&& (cmd_buf[5] == '='))
	{
		rtd_pr_vt_notice("[VT]current pixelfmt=%d\n", get_VT_Pixel_Format());
		
		if(((cmd_buf[6]-0x30) >= VT_CAP_RGB888) && ((cmd_buf[6]-0x30) < VT_CAP_MAX))
		{
			if((cmd_buf[6]-0x30) != get_VT_Pixel_Format())
			{
				set_VT_Pixel_Format((VT_CAP_FMT)(cmd_buf[6]-0x30));
				rtd_pr_vt_notice("[VT]set pixelfmt=%d\n", get_VT_Pixel_Format());
			}
		}
		else
		{
			rtd_pr_vt_notice("[VT]set pixelfmt error\n");
			ret = -EFAULT;
			return ret;
		}
	}
	else if(((cmd_buf[0] == 'v') && (cmd_buf[1] == 't') && (cmd_buf[2] == 't') && (cmd_buf[3] == 'e') && (cmd_buf[4] == 's')
			&& (cmd_buf[5] == 't')))   /* echo vttest case location buffernum > /dev/vivtdev */
	{
		if(((cmd_buf[7]-0x30) >= VT_TEST_CASE_DISABLE) && ((cmd_buf[7]-0x30) < VT_TEST_MAX_CASE)){
			set_vt_test_case((VT_TEST_CASE_T)(cmd_buf[7]-0x30));
		}else{
			rtd_pr_vt_notice("[VT] set test case error\n");
			ret = -EFAULT;
			return ret;
		}		

		if(get_vt_test_case() != VT_TEST_CASE_DISABLE)
		{
			if(((cmd_buf[9]-0x30) >= KADP_VT_SCALER_INPUT) && ((cmd_buf[9]-0x30) <= KADP_VT_OSD_OUTPUT)){
				vfod_capture_location = (cmd_buf[9]-0x30);
				VTDumpLocation = (KADP_VT_DUMP_LOCATION_TYPE_T)vfod_capture_location;
			}else{
				rtd_pr_vt_notice("[VT] set test case location error\n");
				ret = -EFAULT;
				return ret;
			}
		
			if(((cmd_buf[11]-0x30) >= 1) && ((cmd_buf[11]-0x30) <= 5)){
				set_vt_VtBufferNum(cmd_buf[11]-0x30);
			}else{
				rtd_pr_vt_notice("[VT] set test case bufnum error\n");
				ret = -EFAULT;
				return ret;
			}
		}
	
		rtd_pr_vt_notice("[VT]%s\n", cmd_buf);

		//start test case
		handle_vt_test_case();
	}
	else if(strcmp(cmd_buf, "vtdump=1\n") == 0)
	{
		rtd_pr_vt_notice("[VT]enable vt dump buffer\n");
		//get buffer index & dump
		HAL_VT_WaitVsync(KADP_VT_VIDEO_WINDOW_0);
		if(do_vt_dqbuf(&cap_idx) == TRUE){
			debug_dump_data_to_file(cap_idx, vfod_capture_out_W, vfod_capture_out_H);
			do_vt_qbuf(cap_idx);
		}
		else
			rtd_pr_vt_notice("[VT]dqubf fail,please retry\n"); 	
	}
	else
	{
		rtd_pr_vt_notice("[VT]%s\n", cmd_buf);
	}

#endif

	return ret;
}
	
	
#ifdef ENABLE_VT_TEST_CASE_CONFIG
	
	void handle_vt_test_case(void)
	{
		int i;
		vt_StreamOn_flag = TRUE;
		if(get_vt_test_case() == VT_TEST_CASE_DISABLE)
		{
			//exit vt capture
			do_vt_streamoff();
			do_vt_reqbufs(0);
		}
		else
		{
			if(get_vt_test_case() == VT_TEST_VR360_CASE)
			{
				HAL_VT_EnableFRCMode(FALSE);
			}
			do_vt_reqbufs(get_vt_VtBufferNum());
			for (i = 0; i < get_vt_VtBufferNum(); i++)
				do_vt_qbuf(i);
			do_vt_capture_streamon();
		}
	}
#endif

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
        case VT_IOC_SET_OUTPUT_FPS:
		{
			unsigned int tmpFps;
			if(copy_from_user((void *)&tmpFps, (const void __user *)arg, sizeof(unsigned int)))
			{
				retval = -EFAULT;
				rtd_pr_vt_debug("copy_to_user:scaler vt ioctl code=VT_IOC_SET_OUTPUT_FPS failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VT_Set_OutFps(tmpFps) == FALSE)
					retval = -1;
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
static char *vivt_devnode(struct device *dev, umode_t *mode)
{
	*mode =0666;
	return NULL;
}
#endif

#ifndef UT_flag
#if IS_ENABLED(CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER)
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
	for (i = DC2H_SCALEDOWN_DC2H_HSD_Ctrl0_reg; i <=DC2H_SCALEDOWN_DC2H_444to422_CTRL_reg; i=i+4)
	{
		rtd_pr_vt_err("%x=%x\n", i, IoReg_Read32(i));
	}

	for (i = DC2H_RGB2YUV_DC2H_RGB2YUV_CTRL_reg; i <=DC2H_RGB2YUV_DC2H_Tab1_Yo_reg; i=i+4)
	{
		rtd_pr_vt_err("%x=%x\n", i, IoReg_Read32(i));
	}

	rtd_pr_vt_err("0xb8029c20=%x\n", IoReg_Read32(DC2H_RGB2YUV_DC2H_clken_reg));

	for (i = DC2H_RGB2YUV_DC2H_3DMaskTo2D_Ctrl_reg; i <=DC2H_RGB2YUV_DC2H_3DMaskTo2D_Ctrl1_reg; i=i+4)
	{
		rtd_pr_vt_err("%x=%x\n", i, IoReg_Read32(i));
	}

	for (i = DC2H_VI_DC2H_V1_OUTPUT_FMT_reg; i <=DC2H_VI_DC2H_VI_READ_MARGIN_reg; i=i+4)
	{
		rtd_pr_vt_err("%x=%x\n", i, IoReg_Read32(i));
	}

	rtd_pr_vt_err("0xb8029d68=%x\n", IoReg_Read32(DC2H_VI_DC2H_DBG_reg));
	rtd_pr_vt_err("0xb8029d6c=%x\n", IoReg_Read32(DC2H_VI_DC2H_vi_doublebuffer_reg));

	for (i = DC2H_VI_DC2H_vi_c_line_step_reg; i <=DC2H_DMA_dc2h_seq_byte_channel_swap_reg; i=i+4)
	{
		rtd_pr_vt_err("%x=%x\n", i, IoReg_Read32(i));
	}
	return;
}
DCMT_DEBUG_INFO_DECLARE(sb4_dc2h_mdscpu, sb4_dc2h_debug_info);
#endif

int vivt_init_module(void)
{
	int result;
  	int devno;
	dev_t dev = 0;

#if IS_ENABLED(CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER)
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

	// vivt_class->devnode = vivt_devnode;

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

#ifdef CONFIG_SUPPORT_SCALER_MODULE
// the module init/exit will be moved to scaler_module.c if scaler was built as a kernel module
#else
module_init(vivt_init_module);
module_exit(vivt_cleanup_module);
#endif

#endif

// #ifndef UT_flag
