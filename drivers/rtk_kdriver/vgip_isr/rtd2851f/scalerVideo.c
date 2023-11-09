/***************************************************************************
                          scalerVideo.c  -  description
                             -------------------
    begin                : Mon Dec 23 2008
    copyright           : (C) 2008 by hsliao
    email                : hsliao@realtek.com
 ***************************************************************************/
// oliver-
/*
#include <linux/version.h>  // kernel version

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
    #ifndef __KERNEL__
        #define __KERNEL__
    #endif
#endif

#include <linux/module.h>   // normal module use
#include <linux/kernel.h>   // printk()
*/

// driver header
#include <rtk_kdriver/io.h>

#include <scaler/vipRPCCommon.h>
#include <scaler/scalerCommon.h>

#include "vgip_isr/scalerVideo.h"
#include "vgip_isr/scalerVIP.h"
#include "vgip_isr/scalerDI.h"
#include "vgip_isr/scalerDCC.h"
#include "tvscalercontrol/vdc/video.h"
#include "tvscalercontrol/scaler/scalerstruct.h"

// register address header
#include <rbus/vdpq_reg.h>
#include <rbus/vdtop_reg.h>
//#include <rbus/adc_reg.h>
#include <rbus/di_reg.h>
#include <rbus/color_icm_reg.h>
#include "tvscalercontrol/scaler/scalerstruct.h"
#include <scaler/vipRPCCommon.h>
#include "rtk_vip_logger.h"

static unsigned char YC_separation_status = VDC_YCSEPARATE_DEFAULT; // YC separation status: default=0, 1D=1, 2D=2, 3D=3
static unsigned char scalerVideo_fh_set_1d = OFF;                // flag of "Fh set to 1D"
static unsigned char scalerVideo_tv_check_3d_is_running = OFF;   // flag of "TV check 3D is running."
unsigned char curr_motion_status = 0;
static unsigned char motion_cnt[10] = {0,0,0,0,0,0,0,0,0,0};                // motion counter for statistics
unsigned int vd_pre_Hist[32];

static ScalerVideo_DebugInfo ScalerVideo_Debug;
static ScalerVideo_DetectionStatus ScalerVideo_Status;
static ScalerVideo_SetBuffer ScalerVideo_Buffer;
static unsigned char AVD_Clock_en_check_flag = 1;

/* ------------ START : info from ScalerVIP.c ------------ */
extern unsigned char Corn_tone_result;
extern unsigned int sat_hist_ratio[COLOR_AutoSat_HISTOGRAM_LEVEL];
extern unsigned int sat_hist[COLOR_AutoSat_HISTOGRAM_LEVEL];
extern unsigned int sat_ratio_mean_value;
extern _clues *SmartPic_clue;
extern unsigned char DynamicOptimizeSystem[200];
extern unsigned char ID_CB_NO[20];
extern unsigned char LGTVPALBGE52;
extern unsigned char LGPTFatoryMultiC34;

extern unsigned int MA_print_count;
extern _system_setting_info *system_info_structure_table;


#define ROS_ENTER_CRITICAL()
#define ROS_EXIT_CRITICAL()

#undef ROSPrintf
#define ROSPrintf(fmt, args...)						VIPprintf(VIP_LOGLEVEL_DEBUG, VIP_LOGMODULE_VGIP_Video_DEBUG, MA_print_count, fmt,##args)
#define ROS_VideoPrintf_General(fmt, args...)		VIPprintf(VIP_LOGLEVEL_DEBUG, VIP_LOGMODULE_VGIP_Video_GeneralStatus_DEBUG, MA_print_count, fmt, ##args)
#define ROS_VideoPrintf_Setting(fmt, args...)		VIPprintf(VIP_LOGLEVEL_DEBUG, VIP_LOGMODULE_VGIP_Video_SettingStatus_DEBUG, MA_print_count, fmt, ##args)
#define ROS_VideoPrintf_SpecialFlag(fmt, args...)	VIPprintf(VIP_LOGLEVEL_DEBUG, VIP_LOGMODULE_VGIP_Video_SpecialFlag_DEBUG, MA_print_count, fmt, ##args)
#define ROS_VideoPrintf_Vector(fmt, args...)		VIPprintf(VIP_LOGLEVEL_DEBUG, VIP_LOGMODULE_VGIP_Video_VectorBinRatio_DEBUG, MA_print_count, fmt, ##args)
#define ROS_VideoPrintf_DynamicCtrl(fmt, args...)	VIPprintf(VIP_LOGLEVEL_DEBUG, VIP_LOGMODULE_VGIP_Video_DynamicControl_DEBUG, MA_print_count, fmt, ##args)
#define ROS_VideoPrintf_ErrorStatus(fmt, args...)	VIPprintf(VIP_LOGLEVEL_DEBUG, VIP_LOGMODULE_VGIP_Video_ErrorStatus_DEBUG, MA_print_count, fmt, ##args)

/* ------------ END : info from ScalerVIP.c ------------ */

const static int FREQ_OFST_TBL[_FSC_overflow][3] =
{
	/*	Ret			RgnSta	RgnEnd	*/
	{	_FSC_n5hz,	-25,	-23		},
	{	_FSC_n4hz,	-22,	-18		},
	{	_FSC_n3hz,	-17,	-13		},
	{	_FSC_n2hz,	-12,	 -8		},
	{	_FSC_n1hz,	 -7,	 -3		},
	{	_FSC_0hz,	 -2,	  2		},
	{	_FSC_p1hz,	  3,	  7		},
	{	_FSC_p2hz,	  8,	 12		},
	{	_FSC_p3hz,	 13,	 16		},
	{	_FSC_p4hz,	 17,	 21		},
	{	_FSC_p5hz,	 22,	 26		}
};

/*
0 : noise status 903c, VDTOP
1 : [SV] fid match cnt
2 : [SV] fid offset
3 : [SV] self diff small threshold
4 : [SV] offset base
5 : [SV] offset multiple
6 : [PAL ERR] accu tolerance
7 : [PAL ERR] massive palmode error multiple
8 : global diff th hifreq offset 20
9 : global diff th hifreq offset 10
10: OFFSET to global diff tolerance
11: VD CTI cadap gain
12: VD CTI lut gain tha
13: VD CTI lut gain thb
*/
static unsigned char NOISY_TBL[VD_TOP_NOISE_STATUS][14] =
{
  { 0x16,  8,  9, 45, 63, /*5*/4, 14,  3, 50, 50, /*10*/6, 2, 2, 4 },
  { 0x0b, 14,  9, 30, 25, /*5*/4, 14,  3, 50, 50, /*10*/5, 2, 2, 4 },
  { 0x07, 14,  9, 30, 25, /*5*/4, 10,  3, 50, 50, /*10*/3, 2, 2, 4 },
  { 0x04, 16,  7, 30, 20, /*5*/3,  6,  4, 35, 35, /*10*/1, 4, 3, 4 },
  { 0x00, 18,  5, 15, 20, /*5*/2,  4,  5, 15, 15, /*10*/0, 4, 4, 4 }
};

/*
0 : global_diff_tolerance
1 : [PAL ERR] palerr_hifreq_diff_large_th
2 : [PAL ERR] DC_level_change_th_base
3 : [SGHF] inter_diff_th
4 : [SGHF] global_diff_small_th
5 : [SGHF] FID_match_cnt_th
6 : [SV] FID_match_cnt_th
7 : [SV] FID_offset
8 : [SV] self_diff_small_th
9 : [SV] offset_base
10: [SV] offset_multi
11: [SGHF] low_border
12: [SGHF] th
13: [PAL ERR] DC_level_change_th_multiple
*/
static unsigned char NTSC_3D_TABLE[480] =
{   // =========AV_NTSC=========
    1,0x06,0x01,0x35,0x32,/*5*/0xa,0x12,0x5,0xf,0x14,/*10*/0x2,0x6,0x5b,0x5,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,0,// 0:still
    1,0x18,0x01,0x11,0x19,/*5*/0xf,0x12,0x5,0xf,0x14,/*10*/0x2,0x6,0x5b,0x5,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,1,// 1
    1,0x18,0x01,0x11,0x19,/*5*/0xf,0x12,0x5,0xf,0x14,/*10*/0x2,0x6,0x5b,0x5,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,2,// 2
    1,0x18,0x01, 0xe,0x19,/*5*/0xf,0x12,0x5,0xf,0x14,/*10*/0x2,0x6,0x5b,0x5,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,3,// 3
    1,0x18,0x01, 0xe,0x19,/*5*/0xf,0x12,0x5,0xf,0x14,/*10*/0x2,0x6,0x5b,0x5,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,4,// 4
    0,0x18,0x01, 0xe,0x19,/*5*/0xf,0x12,0x5,0xf,0x14,/*10*/0x2,0x6,0x5b,0x5,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,5,// 5
    0,0x18,0x01, 0xe,0x19,/*5*/0xf,0x12,0x5,0xf,0x14,/*10*/0x2,0x6,0x5b,0x2,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,6,// 6
    0,0x18,0x01, 0xe,0x19,/*5*/0xf,0x12,0x5,0xf,0x14,/*10*/0x2,0x6,0x5b,0x1,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,7,// 7:motion

    // =========TV_NTSC=========
    5,0x06,0x01,0x35,0x32,/*5*/0xa,0xc, 0x5,0xf ,0x14,/*10*/0x2,0x6,0x5b,0x5,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,0,// 0:still
    5,0x18,0x01,0x35,0x32,/*5*/0xa,0xd, 0x5,0x10,0x14,/*10*/0x2,0x6,0x5b,0x5,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,1,// 1
    5,0x18,0x01,0x11,0x19,/*5*/0xf,0xe, 0x5,0x12,0x14,/*10*/0x2,0x6,0x5b,0x5,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,2,// 2
    5,0x18,0x01, 0xe,0x19,/*5*/0xf,0xf, 0x5,0x12,0x14,/*10*/0x2,0x6,0x5b,0x5,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,3,// 3
    5,0x18,0x01, 0xe,0x19,/*5*/0xf,0x10,0x5,0x12,0x14,/*10*/0x2,0x6,0x5b,0x5,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,4,// 4
    4,0x18,0x01, 0xe,0x19,/*5*/0xf,0x11,0x5,0x12,0x14,/*10*/0x2,0x6,0x5b,0x5,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,5,// 5
    3,0x18,0x01, 0xe,0x19,/*5*/0xf,0x12,0x5,0x12,0x14,/*10*/0x2,0x6,0x5b,0x5,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,6,// 6
    2,0x18,0x01, 0xe,0x19,/*5*/0xf,0x13,0x5,0x12,0x14,/*10*/0x2,0x6,0x5b,0x5,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,7,// 7:motion
};

/*======HISTORY START======
[nick187][20140212][AV&TV][LG-Disk][Motion status=0~1] add still_gray_hifreq_low_border


======HISTORY END======*/
static unsigned char PAL_3D_TABLE[480] =
{   // =========AV_PAL=========
    1,0x00,0x0b,0x2d,0x50,/*5*/0xf,0x12,0x5,0xf,0x14,/*10*/0x2,0xf,0x32,0x5,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,0,// 0:still
    1,0x18,0x0a,0x19,0x50,/*5*/0xf,0x12,0x5,0xf,0x14,/*10*/0x2,0xf,0x32,0x5,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,1,// 1
    1,0x18,0x0a,0x19,0x50,/*5*/0xf,0x12,0x5,0xf,0x14,/*10*/0x2,0xf,0x32,0x5,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,2,// 2
    1,0x18,0x0a,0x19,0x50,/*5*/0xf,0x12,0x5,0xf,0x14,/*10*/0x2,0xf,0x32,0x5,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,3,// 3
    1,0x18,0x0a,0x19,0x50,/*5*/0xf,0x12,0x5,0xf,0x14,/*10*/0x2,0xf,0x32,0x5,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,4,// 4
    1,0x18,0x0a, 0xe,0x19,/*5*/0xf,0x12,0x5,0xf,0x14,/*10*/0x2,0xf,0x32,0x5,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,5,// 5
    1,0x18,0x0a, 0xe,0x19,/*5*/0xf,0x12,0x5,0xf,0x14,/*10*/0x2,0xf,0x32,0x5,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,6,// 6
    1,0x18,0x0a, 0xe,0x19,/*5*/0xf,0x12,0x5,0xf,0x14,/*10*/0x2,0xf,0x32,0x5,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,7,// 7:motion

    // =========TV_PAL=========
    4,0x06,0x0d,0x2d,0x50,/*5*/0xf,0xf,0x5,0x14,0x28,/*10*/0x4,0xf,0x46,0x5,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,0,// 0:still
    3,0x18,0x0a,0x19,0x50,/*5*/0xa,0xf,0x5,0x14,0x28,/*10*/0x4,0xf,0x46,0x5,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,1,// 1
    3,0x18,0x0a,0x19,0x50,/*5*/0xa,0xf,0x5,0x14,0x28,/*10*/0x4,0xf,0x46,0x5,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,2,// 2
    3,0x18,0x0a,0x19,0x50,/*5*/0xa,0xf,0x5,0x14,0x28,/*10*/0x4,0xf,0x46,0x5,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,3,// 3
    2,0x18,0x0a,0x19,0x50,/*5*/0xa,0xf,0x5,0x14,0x28,/*10*/0x4,0xf,0x46,0x5,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,4,// 4
    2,0x18,0x0a, 0xe,0x19,/*5*/0xf,0xf,0x5,0x14,0x28,/*10*/0x4,0xf,0x46,0x5,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,5,// 5
    2,0x18,0x0a, 0xe,0x19,/*5*/0xf,0xf,0x5,0x14,0x28,/*10*/0x4,0xf,0x46,0x5,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,6,// 6
    2,0x18,0x0a, 0xe,0x19,/*5*/0xf,0xf,0x5,0x14,0x28,/*10*/0x4,0xf,0x46,0x5,0,/*15*/0,0,0,0,0,/*20*/0,0,0,0,0,/*25*/0,0,0,0,7,// 7:motion
};

/***********************************************************************************************/
/**
 * Read VD setting mode status
 *
 * @param   void
 * @return  0: ntsc / 1: palm / 2: ntsc50 / 3: paln / 4: ntsc443 / 5: pal60 / 6: pali / 7: secam
 */
unsigned char scalerVideo_readMode(void)
{
    vdtop_mode_detection_status_RBUS mode_detection_status_reg;
    if (!scalerVIP_check_vdc_hw_pwr())
      return VDC_MODE_NO_SIGNAL;
    mode_detection_status_reg.regValue = (unsigned char)VDPQ_rtd_inl(VDTOP_MODE_DETECTION_STATUS_reg);
    return (unsigned char)mode_detection_status_reg.set_mode_status;
}

/**
 * H locked IRQ
 *
 * @param   void
 * @return  noise status
 */
unsigned char scalerVideo_Hlock_irq(void)
{
    vdtop_irq_status1_RBUS irq_status1_reg;
    irq_status1_reg.regValue = VDPQ_rtd_inl(VDTOP_IRQ_STATUS1_reg);
    return (unsigned char)irq_status1_reg.hlock_irq;
}

void scalerVideo_Clean_Hlock_irq(void)
{
    vdtop_irq_status1_RBUS irq_status1_reg;
    irq_status1_reg.regValue = VDPQ_rtd_inl(VDTOP_IRQ_STATUS1_reg);
    irq_status1_reg.hlock_irq = 1;
    VDPQ_rtd_outl(VDTOP_IRQ_STATUS1_reg, irq_status1_reg.regValue);
}

/**
 * V locked IRQ
 *
 * @param   void
 * @return  noise status
 */
unsigned char scalerVideo_Vlock_irq(void)
{
    vdtop_irq_status1_RBUS irq_status1_reg;
    irq_status1_reg.regValue = VDPQ_rtd_inl(VDTOP_IRQ_STATUS1_reg);
    return (unsigned char)irq_status1_reg.vlock_irq;
}

void scalerVideo_Clean_Vlock_irq(void)
{
    vdtop_irq_status1_RBUS irq_status1_reg;
    irq_status1_reg.regValue = VDPQ_rtd_inl(VDTOP_IRQ_STATUS1_reg);
    irq_status1_reg.vlock_irq = 1;
    VDPQ_rtd_outl(VDTOP_IRQ_STATUS1_reg, irq_status1_reg.regValue);
}

/**
 * Clock locked IRQ
 *
 * @param   void
 * @return  clock status
 */
unsigned char scalerVideo_Clock_irq(void)
{
    vdtop_irq_status1_RBUS irq_status1_reg;
    irq_status1_reg.regValue = VDPQ_rtd_inl(VDTOP_IRQ_STATUS1_reg);
    return (unsigned char)irq_status1_reg.chromalock_irq;
}

void scalerVideo_Clean_Clock_irq(void)
{
    vdtop_irq_status1_RBUS irq_status1_reg;
    irq_status1_reg.regValue = VDPQ_rtd_inl(VDTOP_IRQ_STATUS1_reg);
    irq_status1_reg.chromalock_irq = 1;
    VDPQ_rtd_outl(VDTOP_IRQ_STATUS1_reg, irq_status1_reg.regValue);
}

/**
 * No signal IRQ
 *
 * @param   void
 * @return  noise status
 */
unsigned char scalerVideo_Nosig_irq(void)
{
    vdtop_irq_status1_RBUS irq_status1_reg;
    irq_status1_reg.regValue = VDPQ_rtd_inl(VDTOP_IRQ_STATUS1_reg);
    return (unsigned char)irq_status1_reg.nosig_irq;
}

void scalerVideo_Clean_Nosig_irq(void)
{
    vdtop_irq_status1_RBUS irq_status1_reg;
    irq_status1_reg.regValue = VDPQ_rtd_inl(VDTOP_IRQ_STATUS1_reg);
    irq_status1_reg.nosig_irq = 1;
    VDPQ_rtd_outl(VDTOP_IRQ_STATUS1_reg, irq_status1_reg.regValue);
}

/**
 * Mode change IRQ
 *
 * @param   void
 * @return  noise status
 */
unsigned char scalerVideo_Mode_irq(void)
{
    vdtop_irq_status1_RBUS irq_status1_reg;
    irq_status1_reg.regValue = VDPQ_rtd_inl(VDTOP_IRQ_STATUS1_reg);
    return (unsigned char)irq_status1_reg.mode_irq;
}

void scalerVideo_Clean_Mode_irq(void)
{
    vdtop_irq_status1_RBUS irq_status1_reg;
    irq_status1_reg.regValue = VDPQ_rtd_inl(VDTOP_IRQ_STATUS1_reg);
    irq_status1_reg.mode_irq = 1;
    VDPQ_rtd_outl(VDTOP_IRQ_STATUS1_reg, irq_status1_reg.regValue);
}

#ifdef CONFIG_SUPPORT_SRC_ADC
void scalerVideo_Disable_DCrestore(void)
{
    adc_adc_dcrestore_ctrl_RBUS adc_dcrestore_ctrl_reg;
    adc_dcrestore_ctrl_reg.regValue = rtd_inl(ADC_ADC_DCRESTORE_CTRL_reg);
    if((adc_dcrestore_ctrl_reg.adc_dcrestore_ctrl_1_0 +
		adc_dcrestore_ctrl_reg.adc_dcrestore_ctrl_2 +
		adc_dcrestore_ctrl_reg.adc_dcrestore_ctrl_4_3 +
		adc_dcrestore_ctrl_reg.adc_dcrestore_ctrl_5 +
		adc_dcrestore_ctrl_reg.adc_dcrestore_ctrl_7_6 +
		adc_dcrestore_ctrl_reg.adc_dcrestore_ctrl_8
		) != 0)
    {
        adc_dcrestore_ctrl_reg.adc_dcrestore_ctrl_1_0 = 0;
		adc_dcrestore_ctrl_reg.adc_dcrestore_ctrl_2 = 0;
		adc_dcrestore_ctrl_reg.adc_dcrestore_ctrl_4_3 = 0;
		adc_dcrestore_ctrl_reg.adc_dcrestore_ctrl_5 = 0;
		adc_dcrestore_ctrl_reg.adc_dcrestore_ctrl_7_6 = 0;
		adc_dcrestore_ctrl_reg.adc_dcrestore_ctrl_8 = 0;

        rtd_outl(ADC_ADC_DCRESTORE_CTRL_reg, adc_dcrestore_ctrl_reg.regValue);
    }
}
#endif

unsigned char scalerVideo_MotionDecision(void)
{
    unsigned int MA_data[3];
    unsigned char ucMotion = 0;
    // Source index
    unsigned char SourceIdx = 0;


    di_im_di_si_ma_total_fm_sum_RBUS total_fm_sum;
    di_im_di_si_ma_large_fm_pixel_RBUS large_fm_pixel;
    di_im_di_si_ma_large_fm_sum_RBUS large_fm_sum;

    total_fm_sum.regValue = rtd_inl(DI_IM_DI_SI_MA_TOTAL_FM_SUM_reg);
    large_fm_pixel.regValue = rtd_inl(DI_IM_DI_SI_MA_LARGE_FM_PIXEL_reg);
    large_fm_sum.regValue = rtd_inl(DI_IM_DI_SI_MA_LARGE_FM_SUM_reg);

    MA_data[0] = total_fm_sum.total_frame_motion_sum;
    MA_data[1] = large_fm_pixel.large_frame_motion_pixel;
    MA_data[2] = large_fm_sum.large_frame_motion_sum;


    if(ScalerVideo_Status.src_is_tv == TRUE)
    {
           if(ScalerVideo_Status.VD_Mode == VDC_MODE_NTSC) {
			SourceIdx = _TV_NTSC;
	   } else if( ScalerVideo_Status.VD_Mode == VDC_MODE_PALI ) {
			SourceIdx = _TV_PAL;
           } else {
			SourceIdx = _TV_PAL;
           }
    } else {
            if(ScalerVideo_Status.VD_Mode == VDC_MODE_NTSC) {
			SourceIdx = _AV_NTSC;
	    } else if( ScalerVideo_Status.VD_Mode == VDC_MODE_PALI ) {
			SourceIdx = _AV_PAL;
            } else {
			SourceIdx = _AV_PAL;
            }
    }

            // Motion status decision
	 if( (MA_data[0]<VD_MA_TH_TABLE[SourceIdx][_ms_0][0]) &&
		(MA_data[1]<VD_MA_TH_TABLE[SourceIdx][_ms_0][1]) &&
		(MA_data[2]<VD_MA_TH_TABLE[SourceIdx][_ms_0][2]) )
            {
                ucMotion = 0;       //still
            }
    else if( (MA_data[0]<VD_MA_TH_TABLE[SourceIdx][_ms_1][0]) ||
		( (MA_data[1]<VD_MA_TH_TABLE[SourceIdx][_ms_1][1]) &&
		  (MA_data[2]<VD_MA_TH_TABLE[SourceIdx][_ms_1][2]) ) ) // VB 3 color fail
            {
                ucMotion = 1;
            }
    else if( (MA_data[0]<VD_MA_TH_TABLE[SourceIdx][_ms_2][0]) ||
		( (MA_data[1]<VD_MA_TH_TABLE[SourceIdx][_ms_2][1]) &&
		  (MA_data[2]<VD_MA_TH_TABLE[SourceIdx][_ms_2][2]) ) )
            {
                ucMotion = 2;
            }
    else if( (MA_data[0]<VD_MA_TH_TABLE[SourceIdx][_ms_3][0]) &&
		( (MA_data[1]<VD_MA_TH_TABLE[SourceIdx][_ms_3][1]) &&
		  (MA_data[2]<VD_MA_TH_TABLE[SourceIdx][_ms_3][2]) ) )
            {
                ucMotion = 3;
            }
    else if( (MA_data[0]<VD_MA_TH_TABLE[SourceIdx][_ms_4][0]) &&
		( (MA_data[1]<VD_MA_TH_TABLE[SourceIdx][_ms_4][1]) &&
		  (MA_data[2]<VD_MA_TH_TABLE[SourceIdx][_ms_4][2]) ) )
            {
                ucMotion = 4;
            }
    else if( (MA_data[0]<VD_MA_TH_TABLE[SourceIdx][_ms_5][0]) &&
		( (MA_data[1]<VD_MA_TH_TABLE[SourceIdx][_ms_5][1]) &&
		  (MA_data[2]<VD_MA_TH_TABLE[SourceIdx][_ms_5][2]) ) )
            {
                ucMotion = 5;
            }
    else if( (MA_data[0]<VD_MA_TH_TABLE[SourceIdx][_ms_6][0]) &&
		( (MA_data[1]<VD_MA_TH_TABLE[SourceIdx][_ms_6][1]) &&
		  (MA_data[2]<VD_MA_TH_TABLE[SourceIdx][_ms_6][2]) ) )
            {
                ucMotion = 6;
            }
            else
            {
                ucMotion = 7;   // motion
            }

    return ucMotion;
}

void scalerVideo_F4_global_diff_tolerance(unsigned char global_diff_tolerance)
{
    // PATTERN: Videobook_SONY-Radio-zoom
    // REASON: global_diff_tolerance=0 to see the motion diff
    vdpq_yc_3d_f4_pair_ctrl_6_RBUS YC_3D_F4_Pair_Ctrl_6_reg;
    YC_3D_F4_Pair_Ctrl_6_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Pair_Ctrl_6_reg);
    YC_3D_F4_Pair_Ctrl_6_reg.global_diff_tolerance = global_diff_tolerance;
    VDPQ_rtd_outl(VDPQ_YC_3D_F4_Pair_Ctrl_6_reg, YC_3D_F4_Pair_Ctrl_6_reg.regValue);
}

void scalerVideo_SGHF_hifreq_low_border(unsigned char still_gray_hifreq_low_border)
{
    vdpq_yc_3d_f4_pair_ctrl_5_RBUS yc_3d_f4_pair_ctrl_5_reg;
    yc_3d_f4_pair_ctrl_5_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Pair_Ctrl_5_reg);
    yc_3d_f4_pair_ctrl_5_reg.still_gray_hifreq_low_border = still_gray_hifreq_low_border;
    VDPQ_rtd_outl(VDPQ_YC_3D_F4_Pair_Ctrl_5_reg, yc_3d_f4_pair_ctrl_5_reg.regValue);
}

void scalerVideo_SGHF_still_gray_hifreq_th(unsigned char still_gray_hifreq_th)
{
    vdpq_yc_3d_f4_pair_ctrl_5_RBUS yc_3d_f4_pair_ctrl_5_reg;
    yc_3d_f4_pair_ctrl_5_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Pair_Ctrl_5_reg);
    yc_3d_f4_pair_ctrl_5_reg.still_gray_hifreq_th = still_gray_hifreq_th;
    VDPQ_rtd_outl(VDPQ_YC_3D_F4_Pair_Ctrl_5_reg, yc_3d_f4_pair_ctrl_5_reg.regValue);
}

void scalerVideo_palerr_hifreq_diff_large_th(unsigned char palerr_hifreq_diff_large_th)
{
    // PATTERN: PAL_Shibasoku TG-45_PHOTO
    // REASON: palerr_hifreq_diff_large_th=0x6 to lift up palerr_flag under still patttern
    vdpq_yc_3d_f4_pal_err_com_1_RBUS YC_3D_F4_PAL_Err_Com_1_reg;
    YC_3D_F4_PAL_Err_Com_1_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_PAL_Err_Com_1_reg);
    YC_3D_F4_PAL_Err_Com_1_reg.palerr_hifreq_diff_large_th = palerr_hifreq_diff_large_th;
    VDPQ_rtd_outl(VDPQ_YC_3D_F4_PAL_Err_Com_1_reg, YC_3D_F4_PAL_Err_Com_1_reg.regValue);
}

void scalerVideo_DC_level_change_th(unsigned char DC_level_change_th_base, unsigned char DC_level_change_th_multiple)
{
    // PATTERN: PAL_Shibasoku TG-45_PHOTO
    // REASON: DC_level_change_th_base=0xd to lift up palerr_flag under still patttern
    vdpq_yc_3d_f4_dc_compare_RBUS YC_3D_F4_DC_Compare_reg;
    YC_3D_F4_DC_Compare_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_DC_Compare_reg);
    YC_3D_F4_DC_Compare_reg.dc_level_change_th_base = DC_level_change_th_base;
    YC_3D_F4_DC_Compare_reg.dc_level_change_th_multiple = DC_level_change_th_multiple;
    VDPQ_rtd_outl(VDPQ_YC_3D_F4_DC_Compare_reg, YC_3D_F4_DC_Compare_reg.regValue);
}


void scalerVideo_SV_ctrl(unsigned char FID_match_cnt_th, unsigned char FID_offset, unsigned char self_diff_small_th, unsigned char still_vector_offset_base, unsigned char still_vector_offset_multi)
{
    // PATTERN: TV_PAL via Fluke : LG-Disk-YogaWoman
    // REASON: VD_Noise_status is too high

    vdpq_yc_3d_f4_still_vector_1_RBUS YC_3D_F4_Still_Vector_1_reg;
    vdpq_yc_3d_f4_still_vector_2_RBUS YC_3D_F4_Still_Vector_2_reg;
    vdpq_yc_3d_f4_pair_ctrl_1_RBUS YC_3D_F4_Pair_Ctrl_1_reg;


    YC_3D_F4_Still_Vector_1_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Still_Vector_1_reg);
    YC_3D_F4_Still_Vector_1_reg.sv_vector_fid_match_cnt_th = FID_match_cnt_th;
    YC_3D_F4_Still_Vector_1_reg.sv_vector_fid_offset = FID_offset;
    VDPQ_rtd_outl(VDPQ_YC_3D_F4_Still_Vector_1_reg, YC_3D_F4_Still_Vector_1_reg.regValue);

    YC_3D_F4_Still_Vector_2_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Still_Vector_2_reg);
    YC_3D_F4_Still_Vector_2_reg.sv_self_diff_small_th = self_diff_small_th;
    VDPQ_rtd_outl(VDPQ_YC_3D_F4_Still_Vector_2_reg, YC_3D_F4_Still_Vector_2_reg.regValue);

    YC_3D_F4_Pair_Ctrl_1_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Pair_Ctrl_1_reg);
    YC_3D_F4_Pair_Ctrl_1_reg.still_vector_offset_base = still_vector_offset_base;
    YC_3D_F4_Pair_Ctrl_1_reg.still_vector_offset_multiple = still_vector_offset_multi;
    VDPQ_rtd_outl(VDPQ_YC_3D_F4_Pair_Ctrl_1_reg, YC_3D_F4_Pair_Ctrl_1_reg.regValue);

}

void scalerVideo_SGHF_ctrl(unsigned char sghf_inter_diff_th, unsigned char sghf_global_diff_small_th, unsigned char sghf_FID_match_cnt_th)
{
    // PATTERN: AV_NTSC : Skyworth-VDTest from Trident
    // REASON: 3D dot-crawl

    vdpq_yc_3d_f4_still_gray_hifreq_1_RBUS YC_3D_F4_Still_Gray_HiFreq_1_reg;
    vdpq_yc_3d_f4_still_gray_hifreq_2_RBUS YC_3D_F4_Still_Gray_HiFreq_2_reg;


    YC_3D_F4_Still_Gray_HiFreq_1_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Still_Gray_HiFreq_1_reg);
    YC_3D_F4_Still_Gray_HiFreq_1_reg.sghf_inter_diff_th = sghf_inter_diff_th;
    YC_3D_F4_Still_Gray_HiFreq_1_reg.sghf_global_diff_small_th = sghf_global_diff_small_th;
    VDPQ_rtd_outl(VDPQ_YC_3D_F4_Still_Gray_HiFreq_1_reg, YC_3D_F4_Still_Gray_HiFreq_1_reg.regValue);

    YC_3D_F4_Still_Gray_HiFreq_2_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Still_Gray_HiFreq_2_reg);
    YC_3D_F4_Still_Gray_HiFreq_2_reg.sghf_vector_fid_match_cnt_th = sghf_FID_match_cnt_th;
    VDPQ_rtd_outl(VDPQ_YC_3D_F4_Still_Gray_HiFreq_2_reg, YC_3D_F4_Still_Gray_HiFreq_2_reg.regValue);

}

void scalerVideo_hifreq_offset_ctrl(unsigned char hifreq_offset_20, unsigned char hifreq_offset_10)
{
    // PATTERN: TV_PAL via Fluke : LG-Disk-YogaWoman
    // REASON: VD_Noise_status is too high
/*
    vdpq_yc_3d_f4_global_diff_th_hifreq_offset_1_RBUS YC_3D_F4_Global_Diff_Th_Hifreq_Offset_1_reg;
    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_1_reg.regValue = rtd_inl(VDPQ_YC_3D_F4_GLOBAL_DIFF_TH_HIFREQ_OFFSET_1_reg);
    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_1_reg.global_diff_th_hifreq_offset_180 = hifreq_offset_180;
    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_1_reg.global_diff_th_hifreq_offset_160 = hifreq_offset_160;
    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_1_reg.global_diff_th_hifreq_offset_140 = hifreq_offset_140;
    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_1_reg.global_diff_th_hifreq_offset_120 = hifreq_offset_120;
    scaler_rtd_outl(VDPQ_YC_3D_F4_GLOBAL_DIFF_TH_HIFREQ_OFFSET_1_reg, YC_3D_F4_Global_Diff_Th_Hifreq_Offset_1_reg.regValue);

    vdpq_yc_3d_f4_global_diff_th_hifreq_offset_2_RBUS YC_3D_F4_Global_Diff_Th_Hifreq_Offset_2_reg;
    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_2_reg.regValue = rtd_inl(VDPQ_YC_3D_F4_GLOBAL_DIFF_TH_HIFREQ_OFFSET_2_reg);
    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_2_reg.global_diff_th_hifreq_offset_100 = hifreq_offset_100;
    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_2_reg.global_diff_th_hifreq_offset_80 = hifreq_offset_80;
    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_2_reg.global_diff_th_hifreq_offset_70 = hifreq_offset_70;
    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_2_reg.global_diff_th_hifreq_offset_60 = hifreq_offset_60;
    scaler_rtd_outl(VDPQ_YC_3D_F4_GLOBAL_DIFF_TH_HIFREQ_OFFSET_2_reg, YC_3D_F4_Global_Diff_Th_Hifreq_Offset_2_reg.regValue);
*/
    vdpq_yc_3d_f4_global_diff_th_hifreq_offset_3_RBUS YC_3D_F4_Global_Diff_Th_Hifreq_Offset_3_reg;
    vdpq_yc_3d_f4_global_diff_th_hifreq_offset_4_RBUS YC_3D_F4_Global_Diff_Th_Hifreq_Offset_4_reg;


    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_3_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Global_Diff_Th_Hifreq_Offset_3_reg);
//  YC_3D_F4_Global_Diff_Th_Hifreq_Offset_3_reg.global_diff_th_hifreq_offset_50 = hifreq_offset_50;
//  YC_3D_F4_Global_Diff_Th_Hifreq_Offset_3_reg.global_diff_th_hifreq_offset_40 = hifreq_offset_40;
//  YC_3D_F4_Global_Diff_Th_Hifreq_Offset_3_reg.global_diff_th_hifreq_offset_30 = hifreq_offset_30;
    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_3_reg.global_diff_th_hifreq_offset_20 = hifreq_offset_20;
    VDPQ_rtd_outl(VDPQ_YC_3D_F4_Global_Diff_Th_Hifreq_Offset_3_reg, YC_3D_F4_Global_Diff_Th_Hifreq_Offset_3_reg.regValue);

    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_4_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Global_Diff_Th_Hifreq_Offset_4_reg);
    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_4_reg.global_diff_th_hifreq_offset_10 = hifreq_offset_10;
//  YC_3D_F4_Global_Diff_Th_Hifreq_Offset_4_reg.global_diff_th_hifreq_offset_0 = hifreq_offset_0;
    VDPQ_rtd_outl(VDPQ_YC_3D_F4_Global_Diff_Th_Hifreq_Offset_4_reg, YC_3D_F4_Global_Diff_Th_Hifreq_Offset_4_reg.regValue);


}

// 20150827, willy, api for VD statistics counter
unsigned int scalerVideo_Get_VD_MiddleWare(unsigned char Status)
{
    unsigned int Result = 0;
    switch(Status)
    {
        case VD_STATUS_HIFREQ_NOT_SMALL:
        {
            vdpq_yc_3d_f4_stat_3_RBUS yc3d_f4_stat_3_reg;
            yc3d_f4_stat_3_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_STAT_3_reg);
            Result = yc3d_f4_stat_3_reg.hifreqdiff_not_small_cnt;
            break;
        }
        case VD_STATUS_HOR_C_CHANGE:
        {
            vdpq_yc_3d_f4_stat_1_RBUS yc3d_f3_stat_1_reg;
            yc3d_f3_stat_1_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_STAT_1_reg);
            Result = yc3d_f3_stat_1_reg.hor_c_change_cnt;
            break;
        }
        case VD_STATUS_TEMPORAL_SPD_SMALL:
        {
            vdpq_yc_3d_f4_stat_2_RBUS yc3d_f3_stat_2_reg;
            yc3d_f3_stat_2_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_STAT_2_reg);
            Result = yc3d_f3_stat_2_reg.tempspd_small_cnt;
            break;
        }
        case VD_STATUS_MGHF_FLAG:
        {
            vdpq_yc_3d_f4_mghf_ctrl_1_RBUS mghf_ctrl_1_reg;
            mghf_ctrl_1_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_MGHF_Ctrl_1_reg);
            Result = mghf_ctrl_1_reg.mghf_flag;
            break;
        }
        case VD_STATUS_NOISE_VARIANCE_MEAN:
        {
            vdpq_ns_rw1_RBUS ns_rw1_reg;
            ns_rw1_reg.regValue = VDPQ_rtd_inl(VDPQ_NS_RW1_reg);
            Result = ns_rw1_reg.vmm;
            break;
        }
        case VD_STATUS_NOISE_DENSITY:
        {
            vdpq_ns_rw2_RBUS ns_rw2_reg;
            ns_rw2_reg.regValue = VDPQ_rtd_inl(VDPQ_NS_RW2_reg);
            Result = ns_rw2_reg.density;
            break;
        }
        case VD_STATUS_NOISE_LARGE_VARIANCE_MEAN:
        {
            vdpq_ns_rw2_RBUS ns_rw2_reg;
            ns_rw2_reg.regValue = VDPQ_rtd_inl(VDPQ_NS_RW2_reg);
            Result = ns_rw2_reg.meannxv;
            break;
        }
        case VD_STATUS_SMD:
        {
            Result = ScalerVideo_Status.Flag_SMD;
            break;
        }
        case VD_STATUS_SMD_STRONG:
        {
            Result = ScalerVideo_Status.Flag_VD_SMD;
            break;
        }
        case VD_STATUS_SMD_IEGSM:
        {
            Result = ScalerVideo_Status.Flag_SMD_IEGSM;
            break;
        }
        case VD_STATUS_MOVING_CAN:
        {
            Result = ScalerVideo_Status.Flag_MovingCan;
            break;
        }
        default:
        {
            Result = 0;
            break;
        }
    }
    return Result;

}

void scalerVideo_palerr_ctrl(unsigned char palerr_accu_tolerance, unsigned char massive_palerr_mul_th)
{
    // PATTERN: TV_PAL via Fluke : Circle on ColorBar
    // REASON: VD_Noise_status is too high

    vdpq_yc_3d_f4_pal_err_com_1_RBUS YC_3D_F4_PAL_Err_Com_1_reg;
    vdpq_yc_3d_f4_pal_err_com_2_RBUS YC_3D_F4_PAL_Err_Com_2_reg;

    YC_3D_F4_PAL_Err_Com_1_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_PAL_Err_Com_1_reg);
    YC_3D_F4_PAL_Err_Com_1_reg.palerr_accu_tolerance = palerr_accu_tolerance;
    VDPQ_rtd_outl(VDPQ_YC_3D_F4_PAL_Err_Com_1_reg, YC_3D_F4_PAL_Err_Com_1_reg.regValue);

    YC_3D_F4_PAL_Err_Com_2_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_PAL_Err_Com_2_reg);
    YC_3D_F4_PAL_Err_Com_2_reg.massive_palmode_error_multiple_th = massive_palerr_mul_th;
    VDPQ_rtd_outl(VDPQ_YC_3D_F4_PAL_Err_Com_2_reg, YC_3D_F4_PAL_Err_Com_2_reg.regValue);
}

/*----------------------- START : Customized functions for different projects -----------------------*/

void ADAP_TV_Noise(unsigned char src_is_tv)
{
    unsigned char FID_match_cnt_th=0, FID_offset=0, self_diff_small_th=0, still_vector_offset_base=0, still_vector_offset_multi=0,
        palerr_accu_tolerance=0, massive_palerr_mul_th=0, hifreq_offset_20=0,hifreq_offset_10=0;

    //======== only for TV, we control this under TV according to VD_noise_status but not motion status========
    if(  src_is_tv == TRUE )
    {
        if(VDPQ_rtd_inl(VDTOP_NOISE_STATUS_reg)> 0x15)
        {   // this condition was added for FieldTry signal at TSB's L3450(Mac2) @ 2014/02/25
            FID_match_cnt_th=8;
            FID_offset=9;
            self_diff_small_th=45;
            still_vector_offset_base=63;
            still_vector_offset_multi=4;

            hifreq_offset_20=50;
            hifreq_offset_10=50;

            palerr_accu_tolerance=13;
            massive_palerr_mul_th=3;
        }

	if(VDPQ_rtd_inl(VDTOP_NOISE_STATUS_reg)> 0xa)
        {
            FID_match_cnt_th=14;
            FID_offset=9;
            self_diff_small_th=30;
            still_vector_offset_base=25;
            still_vector_offset_multi=4;

            hifreq_offset_20=50;
            hifreq_offset_10=50;
            palerr_accu_tolerance=14;
            massive_palerr_mul_th=3;
        }
        else if(VDPQ_rtd_inl(VDTOP_NOISE_STATUS_reg)> 0x8)
        {
            FID_match_cnt_th=14;
            FID_offset=9;
            self_diff_small_th=30;
            still_vector_offset_base=25;
            still_vector_offset_multi=4;

            hifreq_offset_20=50;
            hifreq_offset_10=50;
            palerr_accu_tolerance=10;
            massive_palerr_mul_th=3;
        }
        else if(VDPQ_rtd_inl(VDTOP_NOISE_STATUS_reg)> 0x3)
        {
            FID_match_cnt_th=16;
            FID_offset=7;
            self_diff_small_th=30;
            still_vector_offset_base=20;
            still_vector_offset_multi=3;

            hifreq_offset_20=35;
            hifreq_offset_10=35;
            palerr_accu_tolerance=6;
            massive_palerr_mul_th=4;
        }
        else
        {
            FID_match_cnt_th=18;
            FID_offset=5;
            self_diff_small_th=15;
            still_vector_offset_base=20;
            still_vector_offset_multi=2;

            hifreq_offset_20=15;
            hifreq_offset_10=15;
            palerr_accu_tolerance=4;
            massive_palerr_mul_th=5;
        }

        scalerVideo_hifreq_offset_ctrl(hifreq_offset_20, hifreq_offset_10);
        scalerVideo_SV_ctrl(FID_match_cnt_th, FID_offset, self_diff_small_th, still_vector_offset_base, still_vector_offset_multi);
        scalerVideo_palerr_ctrl(palerr_accu_tolerance, massive_palerr_mul_th);

    }

}

unsigned char scalerVIP_Diverse_Hue_Detector(unsigned short SimilarHue_Cnt_Th_of_1000, unsigned short LowSat_HueSum_Th)
{
    unsigned char i = 0;
    unsigned short Tmp = 0;

    for(i=2; i<COLOR_HUE_HISTOGRAM_LEVEL; i++)
    {
        Tmp = ScalerVideo_Status.Hue_HistogramRatio[i]+ScalerVideo_Status.Hue_HistogramRatio[i-1]+ScalerVideo_Status.Hue_HistogramRatio[i-2];
        if( Tmp>=SimilarHue_Cnt_Th_of_1000 && ScalerVideo_Status.Hue_Sum>LowSat_HueSum_Th )
        {
            return 0;
        }
    }
    return 1;
}

void SONY_Dirty_function_for_cross_color(unsigned char format,_system_setting_info *system_info_structure_table )
{
    unsigned int printf_Flag=0,print_delay=0;

    di_im_di_si_film_motion_RBUS im_di_si_film_motion_reg;
    di_im_di_si_film_motion_h_t_RBUS im_di_si_film_motion_h_t_reg;
    di_im_di_si_film_motion_h_m_RBUS im_di_si_film_motion_h_m_reg;
    di_im_di_si_film_motion_h_b_RBUS im_di_si_film_motion_h_b_reg;
    di_im_di_si_film_motion_v_l_RBUS im_di_si_film_motion_v_l_reg;
    di_im_di_si_film_motion_v_m_RBUS im_di_si_film_motion_v_m_reg;
    di_im_di_si_film_motion_v_r_RBUS im_di_si_film_motion_v_r_reg;
    di_im_di_si_film_motion_c_RBUS im_di_si_film_motion_c_reg;
    unsigned int film_motion_rate_1 = 0, film_motion_rate_2 = 0, film_motion_rate_3 = 0, film_motion_rate_4 = 0, film_motion_rate_5 = 0, film_motion_rate_6 = 0;
    vdpq_yc_3d_f4_dc_compare_RBUS YC_3D_F4_DC_Compare_reg;


    printf_Flag = system_info_structure_table->ISR_Printf_Flag.Flag0;
    print_delay = system_info_structure_table->ISR_Printf_Flag.Delay_Time;
    im_di_si_film_motion_reg.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_reg);
    im_di_si_film_motion_h_t_reg.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_H_T_reg );
    im_di_si_film_motion_h_m_reg.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_H_M_reg);
    im_di_si_film_motion_h_b_reg.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_H_B_reg);
    im_di_si_film_motion_v_l_reg.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_V_L_reg );
    im_di_si_film_motion_v_m_reg.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_V_M_reg);
    im_di_si_film_motion_v_r_reg.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_V_R_reg );
    im_di_si_film_motion_c_reg.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_C_reg );

    film_motion_rate_1 = im_di_si_film_motion_h_t_reg.regValue * 100 / (im_di_si_film_motion_reg.regValue+1);
    film_motion_rate_2 = im_di_si_film_motion_h_m_reg.regValue * 100 / (im_di_si_film_motion_reg.regValue+1);
    film_motion_rate_3 = im_di_si_film_motion_h_b_reg.regValue * 100 / (im_di_si_film_motion_reg.regValue+1);
    film_motion_rate_4 = im_di_si_film_motion_v_l_reg.regValue * 100 / (im_di_si_film_motion_reg.regValue+1);
    film_motion_rate_5 = im_di_si_film_motion_v_m_reg.regValue * 100 / (im_di_si_film_motion_reg.regValue+1);
    film_motion_rate_6 = im_di_si_film_motion_v_r_reg.regValue * 100 / (im_di_si_film_motion_reg.regValue+1);

    YC_3D_F4_DC_Compare_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_DC_Compare_reg);



    if(format == VDC_MODE_NTSC)
    {

    }
    else
    {

        if(film_motion_rate_1 >= 0 && film_motion_rate_1 <= 23 && film_motion_rate_2 >= 53 && film_motion_rate_2 <= 59 && film_motion_rate_3 >= 20 && film_motion_rate_3 <= 41 && sat_ratio_mean_value <= 48)
        {
            YC_3D_F4_DC_Compare_reg.dc_level_change_th_multiple = 0;

        }
        else if(film_motion_rate_4 >= 0 && film_motion_rate_4 <= 1 && film_motion_rate_5 >= 24 && film_motion_rate_5 <= 42 && film_motion_rate_6 >= 58 && film_motion_rate_6 <= 74 && sat_ratio_mean_value <= 33)
        {
            YC_3D_F4_DC_Compare_reg.dc_level_change_th_multiple = 0;

        }
        else
        {
            YC_3D_F4_DC_Compare_reg.dc_level_change_th_multiple = 5;
        }

    }

   VDPQ_rtd_outl(VDPQ_YC_3D_F4_DC_Compare_reg, YC_3D_F4_DC_Compare_reg.regValue);
}

void SONY_Dirty_function_for_HV(unsigned char global_diff_tolerance,_system_setting_info *system_info_structure_table)
{
    static unsigned char HV_static = _FALSE;
    unsigned int printf_Flag=0,print_delay=0;
    unsigned int film_motion = 0;
    unsigned int ma_total_sum = 0;
    unsigned int ma_large_pixel = 0;
    unsigned int ma_large_sum = 0;

    di_im_di_si_film_motion_RBUS im_di_si_film_motion_reg;
    di_im_di_si_ma_total_fm_sum_RBUS im_di_si_ma_total_fm_sum_reg;
    di_im_di_si_ma_large_fm_pixel_RBUS im_di_si_ma_large_fm_pixel_reg;
    di_im_di_si_ma_large_fm_sum_RBUS im_di_si_ma_large_fm_sum_reg;


    printf_Flag = system_info_structure_table->ISR_Printf_Flag.Flag0;
    print_delay = system_info_structure_table->ISR_Printf_Flag.Delay_Time;


    im_di_si_film_motion_reg.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_reg);
    im_di_si_ma_total_fm_sum_reg.regValue = rtd_inl(DI_IM_DI_SI_MA_TOTAL_FM_SUM_reg );
    im_di_si_ma_large_fm_pixel_reg.regValue = rtd_inl(DI_IM_DI_SI_MA_LARGE_FM_PIXEL_reg);
    im_di_si_ma_large_fm_sum_reg.regValue = rtd_inl(DI_IM_DI_SI_MA_LARGE_FM_SUM_reg);

    film_motion = im_di_si_film_motion_reg.film_motionstatus_399_380;
    ma_total_sum = im_di_si_ma_total_fm_sum_reg.total_frame_motion_sum;
    ma_large_pixel = im_di_si_ma_large_fm_pixel_reg.large_frame_motion_pixel;
    ma_large_sum = im_di_si_ma_large_fm_sum_reg.large_frame_motion_sum;


    if(ma_total_sum >= 14500 && ma_total_sum <= 16900 && ma_large_pixel >= 600 && ma_large_pixel <= 900 && ma_large_sum >= 1200 && ma_large_sum <= 2400 && film_motion <= 7 && sat_ratio_mean_value <=31)
    {
        scalerVideo_F4_global_diff_tolerance(15);

        HV_static = _TRUE;


    }
    else
    {

       // if(ma_total_sum >= 13000 && ma_total_sum <= 16900 && ma_large_pixel >= 0 && ma_large_pixel <= 900 && ma_large_sum >= 0 && ma_large_sum <= 2400 && film_motion <= 7 && sat_ratio_mean_value <=31)
        if(ma_total_sum >= 13000 && ma_total_sum <= 16900  && ma_large_pixel <= 900&& ma_large_sum <= 2400 && film_motion <= 7 && sat_ratio_mean_value <=31)//fix coverity 147559
        {

            return;
        }
        else
        {
            scalerVideo_F4_global_diff_tolerance(global_diff_tolerance);
            HV_static = _FALSE;
        }
    }

}

void LG_AV_PAL(void)
{
    if( ScalerVideo_Status.RPC_SystemInfoTable->Project_ID == VIP_Project_ID_TV006 )
        scalerVideo_ApplyVDSetting(VDPQ_PAL_POSTPROC_MF_LG);
    else
        scalerVideo_ApplyVDSetting(VDPQ_PAL_POSTPROC_MF);

    if( scalerVideo_SAT_BinRatioSum(5, 31) > 300 && scalerVideo_SMD_BinRatioSum(0, 0) < 990 )
        scalerVideo_ApplyVDSetting(VDPQ_PAL_CHROMA_VLPF_ENABLE);

    if(scalerVIP_Diverse_Hue_Detector(900,250) == TRUE)
        scalerVideo_ApplyVDSetting(VDPQ_PAL_POST2DBPF_CDIFF);

    // PATTERN: LG-NTSC Thailand dancing woman (High Sat. Scene)
    // REASON: because of Post2DBPF's delay-bug at Post-Proc., use 3D-decoding at flat region and weaker DeXC setting
    //LG_temporarily_forPost2DBPFBug();
    if( scalerVideo_SAT_BinRatioSum(5, 31) > 180 &&
        scalerVIP_Diverse_Hue_Detector(900,250) == TRUE )
        scalerVideo_ApplyVDSetting(VDPQ_PAL_HIGH_SAT_DIVERSE_HUE);

    if( DynamicOptimizeSystem[12] == 1 &&
        ScalerVideo_Status.Flag_SMD == TRUE )
        scalerVideo_ApplyVDSetting(VDPQ_PAL_SMD_GRASS_BOAT);

    if( ScalerVideo_Status.Flag_PAL_SunFlowerRoof == TRUE )
        scalerVideo_ApplyVDSetting(VDPQ_PAL_SUN_FLOWER_ROOF);

    if( ScalerVideo_Status.RPC_SystemInfoTable->Project_ID != VIP_Project_ID_TV015 &&
        ScalerVideo_Status.Flag_PAL_DeXC == TRUE )
    {
        scalerVideo_ApplyVDSetting(VDPQ_PAL_DEXC);
        scalerVideo_ApplyVDSetting(VDPQ_PAL_TNR_XC);
    }

    if( scalerVideo_SAT_BinRatioSum(3, 31) < 10)
    {
        scalerVideo_ApplyVDSetting(VDPQ_PAL_LOW_SAT);
        if( scalerVIP_DI_MiddleWare_GetStatus(STATUS_VPAN) >= 2 )
          scalerVideo_ApplyVDSetting(VDPQ_PAL_IRON_TOWER);
    }

    if( scalerVideo_SAT_BinRatioSum(5, 31) >= 230 &&
        scalerVIP_Diverse_Hue_Detector(510, 250) == TRUE )
        scalerVideo_ApplyVDSetting(VDPQ_PAL_HIGH_SAT_DIVERSE_HUE_DOTCRAWL);

    //LG_MovingCan();
    if( ScalerVideo_Status.Flag_MovingCan)
        scalerVideo_ApplyVDSetting(VDPQ_PAL_MOVING_CAN);

     // 20150814, willy , SMD setting version2
    if( ScalerVideo_Status.Flag_VD_SMD == TRUE )
        scalerVideo_ApplyVDSetting(VDPQ_PAL_STRONG_VD_SMD);

    // 20151010, willy, DOS#151~#153, for only black and white pattern moving pattern, reduce the cross color
    if( scalerVideo_SAT_BinRatioSum(2, 31) <= 50 && scalerVIP_DI_MiddleWare_GetStatus(STATUS_MOTION) >= 22 )
        scalerVideo_ApplyVDSetting(VDPQ_PAL_LOW_SAT_REMOVE_COLOR);

    // 20151024, jimmy, for DOS Ref#4 in PAL vertical line blur
    if( ScalerVideo_Status.Flag_DOS_194 == TRUE )
        scalerVideo_ApplyVDSetting(VDPQ_PAL_DOS_REF4);

    if( ScalerVideo_Status.Flag_DOS_4 == TRUE )
        scalerVideo_ApplyVDSetting(VDPQ_PAL_DOS_CORN_FIELD);

    // 20151216, willy, DOS #40, disable dexc to avoid color blur and reduce xc on flag by C BPF
    if( scalerVIP_Diverse_Hue_Detector(800, 250) == FALSE &&          // the color is almost the same
        scalerVideo_HUE_BinRatioSum(13, 18) >= 850 &&                 // color tend to blue
        scalerVIP_DI_MiddleWare_GetStatus(STATUS_MOTION) <= 20 &&     // motion not small
        scalerVIP_DI_MiddleWare_GetStatus(STATUS_VPAN) == 0 )
        scalerVideo_ApplyVDSetting(VDPQ_PAL_XC_REDUCE_BY_C_BPF);

	//LG_Leaf_MagentaGreenCircle();
    if( DynamicOptimizeSystem[196] == 1 &&
        scalerVideo_SAT_BinRatioSum(12, 14) > 500 )
        scalerVideo_ApplyVDSetting(VDPQ_PAL_LEAF_MAGENTA_GREEN_CIRCLE);


}

void LG_AV_NTSC(void)
{
	scalerVideo_ApplyVDSetting(VDPQ_NTSC_DYNAMIC_HIGH_FREQ_LOW_SAT_Y);
	scalerVideo_ApplyVDSetting(VDPQ_NTSC_DYNAMIC_HIGH_FREQ_LOW_SAT_XC);
	scalerVideo_ApplyVDSetting(VDPQ_NTSC_DYNAMIC_DIRECTIONAL_3D);

    if( ScalerVideo_Status.RPC_SystemInfoTable->Project_ID == VIP_Project_ID_TV006 )
        scalerVideo_ApplyVDSetting(VDPQ_NTSC_POSTPROC_MF_LG);
    else
        scalerVideo_ApplyVDSetting(VDPQ_NTSC_POSTPROC_MF);

    if( scalerVideo_SAT_BinRatioSum(5, 31) > 300 &&
        scalerVideo_SMD_BinRatioSum(0, 0) < 975 &&          // to avoid chroma LPF sudden enabled when choose pattern in menu
        scalerVIP_Diverse_Hue_Detector(950, 250) == TRUE )  // to avoid chroma LPF sudden enabled when choose pattern in menu
    {
        scalerVideo_ApplyVDSetting(VDPQ_NTSC_CHROMA_VLPF_ENABLE);
    }

    if(scalerVIP_Diverse_Hue_Detector(900,250) == TRUE)
    {
        scalerVideo_ApplyVDSetting(VDPQ_NTSC_POST2DBPF_CDIFF);
    }

    // PATTERN: LG-NTSC Thailand dancing woman (High Sat. Scene)
    // REASON: because of Post2DBPF's delay-bug at Post-Proc., use 3D-decoding at flat region and weaker DeXC setting
    //LG_temporarily_forPost2DBPFBug();
    if( scalerVideo_SAT_BinRatioSum(5, 31) > 180 &&
        scalerVIP_Diverse_Hue_Detector(900,250) == TRUE )
        scalerVideo_ApplyVDSetting(VDPQ_NTSC_HIGH_SAT_DIVERSE_HUE);

    //LG_stablization_by_SMD();
    // PATTERN: SMD related pattern, [12][22][86][91]
    if( ScalerVideo_Status.Flag_SMD == TRUE )
    {
        if( ScalerVideo_Status.Flag_DOS_86_91 == FALSE &&
            DynamicOptimizeSystem[107] == 0 &&
            DynamicOptimizeSystem[12] == 0)
        {
            scalerVideo_ApplyVDSetting(VDPQ_NTSC_SMD_0);
        }
        if( scalerVideo_SAT_BinRatioSum(5, 31) < 10)
        {
            scalerVideo_ApplyVDSetting(VDPQ_NTSC_SMD_1);
            if( scalerVIP_DI_MiddleWare_GetStatus(STATUS_VPAN) >= 2 )
              scalerVideo_ApplyVDSetting(VDPQ_NTSC_SMD_IRON_TOWER);
        }
        else
        {
            // 20151006, willy, remake and combine the original condition
            if( ScalerVideo_Status.Flag_DOS_86_91 == FALSE &&
                DynamicOptimizeSystem[107] == 0 &&
                DynamicOptimizeSystem[12] == 0 )
                scalerVideo_ApplyVDSetting(VDPQ_NTSC_SMD_2);
        }
    }

    if( ScalerVideo_Status.Flag_SunFlowerRoof == TRUE )
        scalerVideo_ApplyVDSetting(VDPQ_NTSC_SUN_FLOWER_ROOF);

    //if(ScalerVideo_Status.ProjectID != VIP_Project_ID_TV015) LG_Avoid_DeXC_TNRXC_onStillLowFreq();
    // PATTERN: LG-NTSC-CG MovingColorBall, LG-NTSC-CG [131]CyclingColorBar
    // REASON: De-XC & TNR_C seriously blurs the Color-edge. But actually, only Hifreq-region need De-XC
    if( ScalerVideo_Status.RPC_SystemInfoTable->Project_ID != VIP_Project_ID_TV015 &&
       ((scalerVIP_DI_MiddleWare_GetStatus(STATUS_HIGHFREQUENCY_LEVEL) < 6 && ScalerVideo_Status.HighSatFlag_Bin_14_31) ||
        (scalerVIP_DI_MiddleWare_GetStatus(STATUS_HIGHFREQUENCY_LEVEL) >= 6 && scalerVideo_SMD_BinRatioSum(0, 0) > 975 )) )
    {
        scalerVideo_ApplyVDSetting(VDPQ_NTSC_DEXC);
        scalerVideo_ApplyVDSetting(VDPQ_NTSC_TNR_XC);
    }

    //LG_HighSat_DiverseHue_ReduceDotCrawl();
    // Target : DOS#45 #46
    // Lot of color transition, need a blur y setting to avoid dot crawl
    if( scalerVideo_SAT_BinRatioSum(5, 31) > 200 &&
        scalerVIP_Diverse_Hue_Detector(900,250) == TRUE )
        scalerVideo_ApplyVDSetting(VDPQ_NTSC_HIGH_SAT_DIVERSE_HUE_DOTCRAWL);

    //LG_FastMoving_LowFreqObject_ReduceDotCrawl();
    if( scalerVideo_SAT_BinRatioSum(5, 31) < 4 &&
        scalerVideo_SMD_BinRatioSum(0, 0) < 700 &&
        ScalerVideo_Status.MotionStatus > 3 &&
        scalerVIP_Diverse_Hue_Detector(800,250) == FALSE &&
        scalerVIP_DI_MiddleWare_GetStatus(STATUS_HIGHFREQUENCY_LEVEL) < 20 &&
        scalerVIP_DI_MiddleWare_GetStatus(STATUS_PAN) == FALSE )
        scalerVideo_ApplyVDSetting(VDPQ_NTSC_LG_FAST_MOVING_LOW_SAT);

    //LG_Leaf_MagentaGreenCircle();
    if( DynamicOptimizeSystem[196] == 1 &&
        scalerVideo_SAT_BinRatioSum(14, 16) > 500 )
        scalerVideo_ApplyVDSetting(VDPQ_NTSC_LEAF_MAGENTA_GREEN_CIRCLE);

    //LG_MovingCan();
    if( ScalerVideo_Status.Flag_MovingCan)
        scalerVideo_ApplyVDSetting(VDPQ_NTSC_MOVING_CAN);

    //LG_reduce_motionToStill_2d3d_jump();
    if( ScalerVideo_Status.Flag_DOS_44 == TRUE )
        scalerVideo_ApplyVDSetting(VDPQ_NTSC_2D3D_JUMP);

    // 20150814, willy , SMD setting version2
    if( ScalerVideo_Status.Flag_VD_SMD == TRUE )
        scalerVideo_ApplyVDSetting(VDPQ_NTSC_STRONG_VD_SMD);

    // 20151010, willy, DOS#151~#153, for only black and white pattern moving pattern, reduce the cross color
    if( scalerVideo_SAT_BinRatioSum(2, 31) <= 50 &&
        scalerVIP_DI_MiddleWare_GetStatus(STATUS_MOTION) >= 22 )
        scalerVideo_ApplyVDSetting(VDPQ_NTSC_LOW_SAT_REMOVE_COLOR);

    // 20151014, willy,DOS#4 #5
    if( scalerVIP_Diverse_Hue_Detector(480, 250) == FALSE &&
        scalerVIP_DI_MiddleWare_GetStatus(STATUS_MOTION) >= 19 &&
        scalerVIP_DI_MiddleWare_GetStatus(STATUS_PAN) == FALSE && // avoid panning
        scalerVIP_DI_MiddleWare_GetStatus(STATUS_VPAN) == FALSE &&
        scalerVideo_SAT_BinRatioSum(0,4) < 950 ) // avoid DOS #28
        scalerVideo_ApplyVDSetting(VDPQ_NTSC_MONO_HUE_AVOID_MOTION_BLUR);

    // 20151216, willy, DOS #40, disable dexc to avoid color blur and reduce xc on flag by C BPF
    if( scalerVIP_Diverse_Hue_Detector(800, 250) == FALSE &&          // the color is almost the same
        scalerVideo_HUE_BinRatioSum(13, 18) >= 850 &&                 // color tend to blue
        scalerVIP_DI_MiddleWare_GetStatus(STATUS_MOTION) <= 20 &&     // motion not small
        scalerVIP_DI_MiddleWare_GetStatus(STATUS_VPAN) == 0 )
        scalerVideo_ApplyVDSetting(VDPQ_NTSC_XC_REDUCE_BY_C_BPF);

    // 20151216, willy, DOS #41, improve the 2D solution by strong post 2D BPF
    if( scalerVIP_DI_MiddleWare_GetStatus(STATUS_HIGHFREQUENCY_LEVEL) >= 500 && // extremly high frequency
        scalerVideo_SAT_BinRatioSum(0, 1) >= 950 )
    {
        scalerVideo_ApplyVDSetting(VDPQ_NTSC_HIGH_FREQ_LOW_SAT);
    }
	// 20160107, jimmy, DOS #42, improve 2D3D switching speed and stability
	if( scalerVIP_Diverse_Hue_Detector(900,250) == TRUE &&
		scalerVIP_DI_MiddleWare_GetStatus(STATUS_HIGHFREQUENCY_LEVEL) < 3 &&
		scalerVIP_DI_MiddleWare_GetStatus(STATUS_MOTION) >= 10 &&
		sat_ratio_mean_value >= 140 )
		scalerVideo_ApplyVDSetting(VDPQ_NTSC_2D3D_SWITCH);

	// 20160108, jimmy, DOS #38, make 3D stable in static background
	if( scalerVideo_SAT_BinRatioSum(0, 1) >= 800 &&
		scalerVideo_SMD_BinRatioSum(0, 2) > 860 &&
		scalerVIP_DI_MiddleWare_GetStatus(STATUS_HIGHFREQUENCY_LEVEL) < 15 )
		scalerVideo_ApplyVDSetting(VDPQ_NTSC_LOW_SAT_STILL);

}

void LG_AV_OTHER(void)
{

    //SONY_motion_mf_en_reg(ScalerVideo_Status.MotionStatus); //do stronger MF when motion big
    if(ScalerVideo_Status.RPC_SystemInfoTable->VIP_source != VDC_MODE_PAL60)
    {
        if( ScalerVideo_Status.RPC_SystemInfoTable->Project_ID == VIP_Project_ID_TV006 )
            scalerVideo_ApplyVDSetting(VDPQ_OTHER_POSTPROC_MF_LG);
        else
            scalerVideo_ApplyVDSetting(VDPQ_OTHER_POSTPROC_MF);
    }

    if( scalerVideo_SAT_BinRatioSum(5, 31) > 300 &&
        scalerVideo_SMD_BinRatioSum(0, 0) < 990 )
        scalerVideo_ApplyVDSetting(VDPQ_OTHER_CHROMA_VLPF_ENABLE);

    if(scalerVIP_Diverse_Hue_Detector(900,250) == TRUE)
        scalerVideo_ApplyVDSetting(VDPQ_OTHER_POST2DBPF_CDIFF);

    // 20150814, willy , SMD setting version2
    if( ScalerVideo_Status.Flag_VD_SMD == TRUE )
        scalerVideo_ApplyVDSetting(VDPQ_OTHER_STRONG_VD_SMD);

    // 20151008, willy, DOS#28 flute woman, 2d3d jump edge peaking
    if( ScalerVideo_Status.Flag_DiagonalEdgeStable == TRUE )
    {
      scalerVideo_ApplyVDSetting(VDPQ_OTHER_DIA_EDGE_2D3D_JUMP);
    }

    // 20151010, willy, DOS#151 ~ #153, for only black and white pattern moving pattern, reduce the cross color
    if( scalerVideo_SAT_BinRatioSum(2, 31) <= 50 &&
        scalerVIP_DI_MiddleWare_GetStatus(STATUS_MOTION) >= 22 )
    {
      scalerVideo_ApplyVDSetting(VDPQ_OTHER_LOW_SAT_REMOVE_COLOR);
    }

}

void LG_AV_STYLE(void)
{

    //------------------------------------------------------------------------------------------------------------------------------------
    // 1. Reset all the VD setting
    // 2. Record all VD setting right after load table
    scalerVideo_ApplyVDSetting(VDPQ_FUNC_RST_ALL);
    //------------------------------------------------------------------------------------------------------------------------------------

    //------------------------------------------------------------------------------------------------------------------------------------
    // some general setting for [1] h2v_lk_en [1] post2dbpf_y_en [3] lut_sel_y [4] postp_mf cdiff_small_con_en [5] bpf_1d_sel_y
    switch(ScalerVideo_Status.VD_Mode)
    {
        case VDC_MODE_NTSC:
        {
            if( scalerVideo_SAT_BinRatioSum(5, 31) > 230 &&
                scalerVIP_Diverse_Hue_Detector(900,250) == TRUE &&
                ScalerVideo_Status.Flag_MovingCan == FALSE )
                scalerVideo_ApplyVDSetting(VDPQ_NTSC_HIGH_SAT_DOTCRAWL_0);
            else
                scalerVideo_ApplyVDSetting(VDPQ_NTSC_HIGH_SAT_DOTCRAWL_1);
            break;
        }
        case VDC_MODE_PALI:
        {
            if( scalerVideo_SAT_BinRatioSum(5, 31) > 230 &&
                scalerVIP_Diverse_Hue_Detector(900,250) == TRUE &&
                ScalerVideo_Status.Flag_MovingCan == FALSE )
                scalerVideo_ApplyVDSetting(VDPQ_PAL_HIGH_SAT_DOTCRAWL_0);
            else
                scalerVideo_ApplyVDSetting(VDPQ_PAL_HIGH_SAT_DOTCRAWL_1);
            break;
        }
        case VDC_MODE_NTSC443:
			break;
        case VDC_MODE_PAL60:
        case VDC_MODE_PALM:
        case VDC_MODE_PALN:
        {
            if( scalerVideo_SAT_BinRatioSum(5, 31) > 300 &&
                scalerVIP_Diverse_Hue_Detector(900,250) == TRUE )
                scalerVideo_ApplyVDSetting(VDPQ_OTHER_HIGH_SAT_DOTCRAWL_0);
            else
                scalerVideo_ApplyVDSetting(VDPQ_OTHER_HIGH_SAT_DOTCRAWL_1);
            break;
        }
        default:
            break;
    }
    //------------------------------------------------------------------------------------------------------------------------------------

    //------------------------------------------------------------------------------------------------------------------------------------
    // setting by motion
    scalerVideo_ApplyVDSetting(VDPQ_MOTION);
    //------------------------------------------------------------------------------------------------------------------------------------

    switch(ScalerVideo_Status.VD_Mode)
    {
        case VDC_MODE_NTSC:
        {
            LG_AV_NTSC();
            break;
        }
        case VDC_MODE_PALI:
        {
            LG_AV_PAL();
            break;
        }
        default:
        {
            LG_AV_OTHER();
            break;
        }
    }

    //------------------------------------------------------------------------------------------------------------------------------------
    // clear the VD apply buffer, willy 20151005
    scalerVideo_BufferOperation(VD_BUFFER_CLEAR_BY_ORDER);
    scalerVideo_BufferOperation(VD_BUFFER_RESET_IDX);
    //------------------------------------------------------------------------------------------------------------------------------------

    // 20151013, willy, for experiment general setting
    if( ScalerVideo_Debug.ExperimentSettingEnable == TRUE )
        scalerVideo_ApplyVDSetting(VDPQ_FUNC_AV_EXP);

}


void LG_TV_PAL(void)
{
	if(LGTVPALBGE52 == 1)
		scalerVideo_ApplyVDSetting(VDPQ_PAL_TNR_XC_LGTVPALBGE52);
	else
		scalerVideo_ApplyVDSetting(VDPQ_PAL_TNR_XC);

	// Dealing with ATV PAL frequency offset
	if( ScalerVideo_Status.FrequencyOffset != _FSC_0hz &&
		ScalerVideo_Status.FrequencyOffset != _FSC_n1hz &&
		ScalerVideo_Status.FrequencyOffset != _FSC_p1hz &&
		ScalerVideo_Status.NoiseStatus <= 3 )
	{
		scalerVideo_ApplyVDSetting(VDPQ_ATV_PAL_FREQ_OFFSET_SMALL);
	}

	if( ScalerVideo_Status.FrequencyOffset == _FSC_overflow && ScalerVideo_Status.NoiseStatus <= 3 )
	{
		scalerVideo_ApplyVDSetting(VDPQ_ATV_PAL_FREQ_OFFSET_LARGE);
	}


}


void LG_TV_NTSC(void)
{
	/*if( ScalerVideo_Status.NoiseStatus > 4 )
	{
		scalerVideo_ApplyVDSetting(VDPQ_ATV_NTSC_NOISY_COLOR_EDGE1);
	}

	if( ScalerVideo_Status.NoiseStatus > 6 )
	{
		scalerVideo_ApplyVDSetting(VDPQ_ATV_NTSC_NOISY_COLOR_EDGE2);
	}*/
}


void LG_TV_OTHER(void)
{

}


void LG_TV_STYLE(void)
{
    // Created by Willy, 2050909
    // This function will fine tune the ATV PQ

    //------------------------------------------------------------------------------------------------------------------------------------
    // 1. Reset all the VD setting, 2. Record all VD setting right after load table
    scalerVideo_ApplyVDSetting(VDPQ_FUNC_RST_ALL);
    //------------------------------------------------------------------------------------------------------------------------------------

    //------------------------------------------------------------------------------------------------------------------------------------
    // some general setting for [1] h2v_lk_en [1] post2dbpf_y_en [3] lut_sel_y [4] postp_mf cdiff_small_con_en [5] bpf_1d_sel_y
    switch(ScalerVideo_Status.VD_Mode)
    {
        case VDC_MODE_NTSC:
        {
            if( scalerVideo_SAT_BinRatioSum(5, 31) > 230 &&
                scalerVIP_Diverse_Hue_Detector(900,250) == TRUE &&
                ScalerVideo_Status.Flag_MovingCan == FALSE )
                scalerVideo_ApplyVDSetting(VDPQ_ATV_NTSC_HIGH_SAT_DOTCRAWL_0);
            else
                scalerVideo_ApplyVDSetting(VDPQ_ATV_NTSC_HIGH_SAT_DOTCRAWL_1);
            break;
        }
        case VDC_MODE_PALI:
        {
            if( scalerVideo_SAT_BinRatioSum(5, 31) > 230 &&
                scalerVIP_Diverse_Hue_Detector(900,250) == TRUE &&
                ScalerVideo_Status.Flag_MovingCan == FALSE )
                scalerVideo_ApplyVDSetting(VDPQ_ATV_PAL_HIGH_SAT_DOTCRAWL_0);
            else
                scalerVideo_ApplyVDSetting(VDPQ_ATV_PAL_HIGH_SAT_DOTCRAWL_1);
            break;
        }
        case VDC_MODE_NTSC443:
        case VDC_MODE_PAL60:
        case VDC_MODE_PALM:
        case VDC_MODE_PALN:
        {
            if( scalerVideo_SAT_BinRatioSum(5, 31) > 300 &&
                scalerVIP_Diverse_Hue_Detector(900,250) == TRUE )
                scalerVideo_ApplyVDSetting(VDPQ_ATV_OTHER_HIGH_SAT_DOTCRAWL_0);
            else
                scalerVideo_ApplyVDSetting(VDPQ_ATV_OTHER_HIGH_SAT_DOTCRAWL_1);
            break;
        }
        default:
            break;
    }
    //------------------------------------------------------------------------------------------------------------------------------------

    //------------------------------------------------------------------------------------------------------------------------------------
    // setting by noise status and motion
    scalerVideo_ApplyVDSetting(VDPQ_MOTION);
    scalerVideo_ApplyVDSetting(VDPQ_ATV_NOISY_SIGNAL);
    //------------------------------------------------------------------------------------------------------------------------------------

    // currently skip the auto adjust in ATV
    switch(ScalerVideo_Status.VD_Mode)
    {
        case VDC_MODE_NTSC:
        {
            LG_TV_NTSC();
            break;
        }
        case VDC_MODE_PALI:
        {
            LG_TV_PAL();
            break;
        }
        default:
        {
            LG_TV_OTHER();
            break;
        }
    }

    //------------------------------------------------------------------------------------------------------------------------------------
    // clear the VD apply buffer, willy 20151005
    scalerVideo_BufferOperation(VD_BUFFER_CLEAR_BY_ORDER);
    scalerVideo_BufferOperation(VD_BUFFER_RESET_IDX);
    //------------------------------------------------------------------------------------------------------------------------------------

    // 20151013, willy, for experiment general setting
    if( ScalerVideo_Debug.ExperimentSettingEnable == TRUE )
        scalerVideo_ApplyVDSetting(VDPQ_FUNC_ATV_EXP);

}

/*----------------------- END : Customized functions for different projects -----------------------*/


void scalerVideo_YC3D_Apply(unsigned char MA_motion, unsigned char src_is_tv)
{

    // 20150923, willy, control the black level from LG DB
    //scalerVideo_Update_HUE_SAT();

    // setting by noise status (no suitable to be in table)?
    /************* Customized threhsold for different projects *************/
    scalerVideo_RegApplyCtrl(VD_REG_CTRL_ENABLE);
    scalerVideo_ApplyVDSetting(VDPQ_FUNC_INIT_VALUE);
    scalerVideo_ClearFormatChangeBitFromKernel();
    if( ScalerVideo_Status.src_is_tv == FALSE)
        LG_AV_STYLE();
    else
        LG_TV_STYLE();
    scalerVideo_ApplyVDSetting(VDPQ_FUNC_WRITE_ADAPTIVE_REG_VAL);
    scalerVideo_RegApplyCtrl(VD_REG_CTRL_DISABLE);
    /************* new 3D adaptive threhsold by New Motion-Detection from Jimmy *************/
    //scalerVideo_adaptive3D_by_motion_noise();


}


/**
 * Adjust adaptive 3D
 *
 * @param   ref_vdc: { FALSE: enter DI / TRUE: no enter DI }
 * @return  void
 */
void scalerVideo_adaptive3DAdjust(void)
{
    vdpq_vdpq_fw_used2_RBUS vdpq_vdpq_fw_used2_reg;

	// update status VD needed
	//scalerVideo_UpdateBlackLevelOffset();
	scalerVideo_UpdateSpecialTable();
	scalerVideo_UpdateSourceType();
	scalerVideo_UpdateMotionStatus();
	scalerVideo_UpdateNoiseStatus();
	if( ScalerVideo_Status.VD_Mode == VDC_MODE_PALI )
		scalerVideo_UpdateFrequencyOffset();
	scalerVideo_UpdateYCSeperationMode();
	scalerVideo_UpdateSaturation();
	scalerVideo_UpdateHueHistogram();
	scalerVideo_UpdateSMDInfo();
	scalerVideo_UpdateSpecialFlag();
	scalerVideo_CalculateNoiseStatus();
	scalerVideo_UpdateDebugInfo();

	scalerVideo_DynamicAdjustREG();

    if( ScalerVideo_Status.YCSepModeFromSystem != VDC_YCSEPARATE_3D )
    {
        scalerVideo_RegApplyCtrl(VD_REG_CTRL_ENABLE);
        vdpq_vdpq_fw_used2_reg.regValue = VDPQ_rtd_inl(VDPQ_VDPQ_FW_USED2_reg);
        scalerVideo_ApplyVDSetting(VDPQ_FUNC_INIT_VALUE);
        if( scalerVideo_BitWiseOperation(vdpq_vdpq_fw_used2_reg.regValue, 21, 21, 0, VD_BIT_GET) != 0 )
        {
            scalerVideo_ClearFormatChangeBitFromKernel();
        }
        scalerVideo_ApplyVDSetting(VDPQ_FUNC_RST_ALL);
        scalerVideo_ApplyVDSetting(VDPQ_FUNC_WRITE_ADAPTIVE_REG_VAL);
        scalerVideo_RegApplyCtrl(VD_REG_CTRL_DISABLE);
    }
    else if( ScalerVideo_Status.YCSepModeFromReg == 3 )
    {
        // 9900[2:0] = 3, force 1D, special setting, ex. ckill, NTSC443
        scalerVideo_RegApplyCtrl(VD_REG_CTRL_ENABLE);
        vdpq_vdpq_fw_used2_reg.regValue = VDPQ_rtd_inl(VDPQ_VDPQ_FW_USED2_reg);
        scalerVideo_ApplyVDSetting(VDPQ_FUNC_INIT_VALUE);
        if( scalerVideo_BitWiseOperation(vdpq_vdpq_fw_used2_reg.regValue, 21, 21, 0, VD_BIT_GET) != 0 )
        {
            scalerVideo_ClearFormatChangeBitFromKernel();
        }
        scalerVideo_ApplyVDSetting(VDPQ_FUNC_RST_ALL);
        scalerVideo_ApplyVDSetting(VDPQ_FORCE_1D_MODE);
		/*
        if((LGPTFatoryMultiC34 == 1) && (ScalerVideo_Status.VD_Mode == VDC_MODE_SECAM))
        {
            scalerVideo_ApplyVDSetting(VDPQ_PAL_TNR_XC_LGTVPALBGE52);
        }
        else if( (ScalerVideo_Status.src_is_tv == FALSE) && (ScalerVideo_Status.VD_Mode == VDC_MODE_SECAM))
        {
            scalerVideo_ApplyVDSetting(VDPQ_PAL_TNR_XC_LGTVPALBGE52);
        }
        */
        if( ScalerVideo_Status.VD_Mode == VDC_MODE_SECAM )
        {
			scalerVideo_ApplyVDSetting(VDPQ_PAL_TNR_XC_LGTVPALBGE52);
        }
        else
        {
            scalerVideo_ApplyVDSetting(VDPQ_PAL_TNR_XC);
        }
        scalerVideo_ApplyVDSetting(VDPQ_FUNC_WRITE_ADAPTIVE_REG_VAL);
        scalerVideo_RegApplyCtrl(VD_REG_CTRL_DISABLE);
    }
    else
    {
        scalerVideo_YC3D_Apply(ScalerVideo_Status.MotionStatus, ScalerVideo_Status.src_is_tv);
    }
}


/**
 * Scaler task (user space) get YC_separation_status flag from scaler driver (kernel space).
 *
 * @param   arg: pointer of VDC_YCSEPARATE_MODE
 * @return  void
 */
void scalerVideo_getYCSep(unsigned long arg)
{
    scalerDrvPrint( "enter scalerVideo_getYCSep\n" );
    //oliver-
    //put_user( YC_separation_status, (unsigned char*)arg );
    *(unsigned char*)arg = YC_separation_status;
}

/**
 * Scaler task (user space) set YC_separation_status flag into scaler driver (kernel space).
 *
 * @param   arg: pointer of VDC_YCSEPARATE_MODE
 * @return  void
 */
void scalerVideo_setYCSep(unsigned long arg)
{
	_system_setting_info *system_info_structure_table=NULL;
    scalerDrvPrint( "enter scalerVideo_setYCSep\n" );
    //oliver-

    //get_user( YC_separation_status, (unsigned char*)arg );
    YC_separation_status = *(unsigned char*)arg;

    system_info_structure_table = scaler_GetShare_Memory_system_setting_info_Struct();
    system_info_structure_table->YC_separation_status = YC_separation_status;

    // flush cache
    //dma_cache_wback_inv((unsigned int)&YC_separation_status, sizeof(unsigned char));

}

/**
 * Get scalerVideo_fh_set_1d flag from scaler driver (kernel space)
 *
 * @param   arg: pointer of BOOL
 * @return  void
 */
void scalerVideo_getFh1DFlag(unsigned long arg)
{
    scalerDrvPrint( "enter scalerVideo_getFh1DFlag\n" );
    //oliver-
    //put_user( scalerVideo_fh_set_1d, (BOOL*)arg );
    *(BOOL*)arg = scalerVideo_fh_set_1d;
}

/**
 * Scaler task (user space) get scalerVideo_tv_check_3d_is_running flag from scaler driver (kernel space).
 *
 * @param   arg: pointer of VDC_YCSEPARATE_MODE
 * @return  void
 */
void scalerVideo_getTVCheck3DFlag(unsigned long arg)
{
    scalerDrvPrint( "enter scalerVideo_getTVCheck3DFlag\n" );
    //oliver-
    //put_user( scalerVideo_tv_check_3d_is_running, (BOOL*)arg );
    *(BOOL*)arg = scalerVideo_tv_check_3d_is_running;
}

/**
 * Scaler task (user space) set scalerVideo_tv_check_3d_is_running flag into scaler driver (kernel space).
 *
 * @param   arg: pointer of VDC_YCSEPARATE_MODE
 * @return  void
 */
void scalerVideo_setTVCheck3DFlag(unsigned long arg)
{
    scalerDrvPrint( "enter scalerVideo_setTVCheck3DFlag\n" );

    // oliver+, for compile warning "scalerVideo.c:55: warning: `scalerVideo_tv_check_3d_is_running' defined but not used"
    scalerVideo_tv_check_3d_is_running = 0;
    //oliver-
    //get_user( scalerVideo_tv_check_3d_is_running, (BOOL*)arg );
    scalerVideo_tv_check_3d_is_running = *(BOOL*)arg;

    // flush cache
    //dma_cache_wback_inv((unsigned int)&scalerVideo_tv_check_3d_is_running, sizeof(BOOL));

}

/**
 * Scaler task (user space) get scalerVideo_3d_table_index from scaler driver (kernel space).
 *
 * @param   arg: pointer of user space 3D table index
 * @return  void
 */
void scalerVideo_get3DTableIndex(unsigned long arg)
{

}

/**
 * Scaler task (user space) set scalerVideo_3d_table_index into scaler driver (kernel space).
 *
 * @param   arg: pointer of user space 3D table index
 * @return  void
 */
void scalerVideo_set3DTableIndex(unsigned long arg)
{
}


// oliver-
/*
void scalerVideo_SendIRCVariable()
{
    VIP_IRC_CMD_FORMAT      tVipIrcCmd;
    int lResult = 0;

    scalerDrvPrint("scalerVideo_sendIRCVariable start\n");
    //scalerDrvPrint("YC_separation_status addr=%x\n", (unsigned int)(&YC_separation_status) );
    //scalerDrvPrint("scalerVideo_fh_set_1d addr=%x\n", (unsigned int)(&scalerVideo_fh_set_1d) );
    //scalerDrvPrint("YC_separation_status=%x, scalerVideo_fh_set_1d=%x, scalerVideo_tv_check_3d_is_running=%x\n",YC_separation_status, scalerVideo_fh_set_1d, scalerVideo_tv_check_3d_is_running);

    g_tScalerVideoIrcConf.ulYCSepAddr = ChangeUINT32Endian( (unsigned int)(&YC_separation_status));
    g_tScalerVideoIrcConf.ulFhSet1DAddr = ChangeUINT32Endian( (unsigned int)(&scalerVideo_fh_set_1d));
    g_tScalerVideoIrcConf.ulTvCheck3DAddr = ChangeUINT32Endian( (unsigned int)(&scalerVideo_tv_check_3d_is_running));
    g_tScalerVideoIrcConf.ul3DTableIndexAddr = ChangeUINT32Endian( (unsigned int)(&scalerVideo_3d_table_index));
    g_tScalerVideoIrcConf.ulMotionCntAddr = ChangeUINT32Endian( (unsigned int)(motion_cnt));

    //scalerDrvPrint("g_tScalerVideoIrcConf.ulYCSepAddr=%x\n", g_tScalerVideoIrcConf.ulYCSepAddr);
    //scalerDrvPrint("g_tScalerVideoIrcConf.ulFhSet1DAddr=%x\n", g_tScalerVideoIrcConf.ulFhSet1DAddr);

    // flush SCALER_VIDEO_IRC_CONFIG
    dma_cache_wback_inv((unsigned int)&g_tScalerVideoIrcConf, sizeof(SCALER_VIDEO_IRC_CONFIG));

    g_tVipIrcConf.ucOpCode = OPCODE_SCALER_VIDEO_VARIABLE;
    g_tVipIrcConf.ulAddress = ChangeUINT32Endian( (unsigned int)(&g_tScalerVideoIrcConf) & 0xFFFFFFF);

    // flush VIP_IRC_CONFIG
    dma_cache_wback_inv((unsigned int)&g_tVipIrcConf, sizeof(VIP_IRC_CONFIG));

    // Send scaler_disp_info address to AV cpu by IRC
    tVipIrcCmd.tComplexCmd.simpleBit = 0x0;
    tVipIrcCmd.tComplexCmd.addrPointer = ( (unsigned int)&g_tVipIrcConf) & 0xFFFFFFF;

    //scalerDrvPrint("g_tScalerVideoIrcConf addr=%x\n", ( (unsigned int)&g_tVipIrcConf) & 0xFFFFFFF );
    //scalerDrvPrint("tVipIrcCmd.tComplexCmd.addrPointer=%x\n", tVipIrcCmd.tComplexCmd.addrPointer);

    lResult = IRC_send(CNL_VIP, *((unsigned int*)&tVipIrcCmd));
}
*/


/* The following is new 3D adaptive threhsold by New Motion-Detection from Jimmy*/

unsigned char scalerVideo_GetCurMotionStatus(void)
{
    return  curr_motion_status;
}

void scalerVideo_F4_global_diff_th(unsigned char diff_th_lower_bound, unsigned char diff_th_upper_bound)
{
    vdpq_yc_3d_f4_pair_ctrl_4_RBUS yc_3d_f4_pair_ctrl_4_reg;
    yc_3d_f4_pair_ctrl_4_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Pair_Ctrl_4_reg);
    yc_3d_f4_pair_ctrl_4_reg.diff_th_lower_bound = diff_th_lower_bound;
    yc_3d_f4_pair_ctrl_4_reg.diff_th_upper_bound = diff_th_upper_bound;
    VDPQ_rtd_outl(VDPQ_YC_3D_F4_Pair_Ctrl_4_reg, yc_3d_f4_pair_ctrl_4_reg.regValue);
}

void scalerVideo_F4_DC_level_fierce_chang(unsigned char DC_level_fierce_change_th, unsigned char global_diff_offset_dc_fierce_change)
{
    vdpq_yc_3d_f4_pair_ctrl_6_RBUS yc_3d_f4_pair_ctrl_6_reg;
    yc_3d_f4_pair_ctrl_6_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Pair_Ctrl_6_reg);
    yc_3d_f4_pair_ctrl_6_reg.dc_level_fierce_change_th = DC_level_fierce_change_th;
    yc_3d_f4_pair_ctrl_6_reg.global_diff_offset_dc_fierce_change = global_diff_offset_dc_fierce_change;
    VDPQ_rtd_outl(VDPQ_YC_3D_F4_Pair_Ctrl_6_reg, yc_3d_f4_pair_ctrl_6_reg.regValue);
}

void scalerVideo_hifreq_offset_ctrl_all(unsigned char hifreq_offset_180, unsigned char hifreq_offset_160, unsigned char hifreq_offset_140,
    unsigned char hifreq_offset_120, unsigned char hifreq_offset_100, unsigned char hifreq_offset_80, unsigned char hifreq_offset_70, unsigned char hifreq_offset_60,
    unsigned char hifreq_offset_50, unsigned char hifreq_offset_40, unsigned char hifreq_offset_30, unsigned char hifreq_offset_20, unsigned char hifreq_offset_10,
    unsigned char hifreq_offset_0)
{
    vdpq_yc_3d_f4_global_diff_th_hifreq_offset_1_RBUS YC_3D_F4_Global_Diff_Th_Hifreq_Offset_1_reg;
    vdpq_yc_3d_f4_global_diff_th_hifreq_offset_2_RBUS YC_3D_F4_Global_Diff_Th_Hifreq_Offset_2_reg;
    vdpq_yc_3d_f4_global_diff_th_hifreq_offset_3_RBUS YC_3D_F4_Global_Diff_Th_Hifreq_Offset_3_reg;
    vdpq_yc_3d_f4_global_diff_th_hifreq_offset_4_RBUS YC_3D_F4_Global_Diff_Th_Hifreq_Offset_4_reg;


    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_1_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Global_Diff_Th_Hifreq_Offset_1_reg);
    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_1_reg.global_diff_th_hifreq_offset_180 = hifreq_offset_180;
    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_1_reg.global_diff_th_hifreq_offset_160 = hifreq_offset_160;
    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_1_reg.global_diff_th_hifreq_offset_140 = hifreq_offset_140;
    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_1_reg.global_diff_th_hifreq_offset_120 = hifreq_offset_120;
    VDPQ_rtd_outl(VDPQ_YC_3D_F4_Global_Diff_Th_Hifreq_Offset_1_reg, YC_3D_F4_Global_Diff_Th_Hifreq_Offset_1_reg.regValue);

    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_2_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Global_Diff_Th_Hifreq_Offset_2_reg);
    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_2_reg.global_diff_th_hifreq_offset_100 = hifreq_offset_100;
    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_2_reg.global_diff_th_hifreq_offset_80 = hifreq_offset_80;
    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_2_reg.global_diff_th_hifreq_offset_70 = hifreq_offset_70;
    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_2_reg.global_diff_th_hifreq_offset_60 = hifreq_offset_60;
    VDPQ_rtd_outl(VDPQ_YC_3D_F4_Global_Diff_Th_Hifreq_Offset_2_reg, YC_3D_F4_Global_Diff_Th_Hifreq_Offset_2_reg.regValue);

    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_3_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Global_Diff_Th_Hifreq_Offset_3_reg);
    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_3_reg.global_diff_th_hifreq_offset_50 = hifreq_offset_50;
    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_3_reg.global_diff_th_hifreq_offset_40 = hifreq_offset_40;
    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_3_reg.global_diff_th_hifreq_offset_30 = hifreq_offset_30;
    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_3_reg.global_diff_th_hifreq_offset_20 = hifreq_offset_20;
    VDPQ_rtd_outl(VDPQ_YC_3D_F4_Global_Diff_Th_Hifreq_Offset_3_reg, YC_3D_F4_Global_Diff_Th_Hifreq_Offset_3_reg.regValue);

    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_4_reg.regValue = rtd_inl(VDPQ_YC_3D_F4_Global_Diff_Th_Hifreq_Offset_4_reg);
    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_4_reg.global_diff_th_hifreq_offset_10 = hifreq_offset_10;
    YC_3D_F4_Global_Diff_Th_Hifreq_Offset_4_reg.global_diff_th_hifreq_offset_0 = hifreq_offset_0;
    VDPQ_rtd_outl(VDPQ_YC_3D_F4_Global_Diff_Th_Hifreq_Offset_4_reg, YC_3D_F4_Global_Diff_Th_Hifreq_Offset_4_reg.regValue);
}

void scalerVideo_CVBSamp_th_ctrl_all(unsigned char diff_th_multiple_200, unsigned char diff_th_multiple_180, unsigned char diff_th_multiple_160,
    unsigned char diff_th_multiple_140, unsigned char diff_th_multiple_120, unsigned char diff_th_multiple_100, unsigned char diff_th_multiple_80, unsigned char diff_th_multiple_60,
    unsigned char diff_th_multiple_40, unsigned char diff_th_multiple_20, unsigned char diff_th_multiple_10, unsigned char diff_th_multiple_0)
{
    vdpq_yc_3d_f4_pair_th_multiple_1_RBUS yc_3d_f4_pair_th_multiple_1_reg;
    vdpq_yc_3d_f4_pair_th_multiple_2_RBUS yc_3d_f4_pair_th_multiple_2_reg;
    vdpq_yc_3d_f4_pair_th_multiple_3_RBUS yc_3d_f4_pair_th_multiple_3_reg;

    yc_3d_f4_pair_th_multiple_1_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Pair_Th_Multiple_1_reg);
    yc_3d_f4_pair_th_multiple_1_reg.diff_th_multiple_200 = diff_th_multiple_200;
    yc_3d_f4_pair_th_multiple_1_reg.diff_th_multiple_180 = diff_th_multiple_180;
    yc_3d_f4_pair_th_multiple_1_reg.diff_th_multiple_160 = diff_th_multiple_160;
    yc_3d_f4_pair_th_multiple_1_reg.diff_th_multiple_140 = diff_th_multiple_140;
    VDPQ_rtd_outl(VDPQ_YC_3D_F4_Pair_Th_Multiple_1_reg, yc_3d_f4_pair_th_multiple_1_reg.regValue);

    yc_3d_f4_pair_th_multiple_2_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Pair_Th_Multiple_2_reg);
    yc_3d_f4_pair_th_multiple_2_reg.diff_th_multiple_120 = diff_th_multiple_120;
    yc_3d_f4_pair_th_multiple_2_reg.diff_th_multiple_100 = diff_th_multiple_100;
    yc_3d_f4_pair_th_multiple_2_reg.diff_th_multiple_80 = diff_th_multiple_80;
    yc_3d_f4_pair_th_multiple_2_reg.diff_th_multiple_60 = diff_th_multiple_60;

    VDPQ_rtd_outl(VDPQ_YC_3D_F4_Pair_Th_Multiple_2_reg, yc_3d_f4_pair_th_multiple_2_reg.regValue);
    yc_3d_f4_pair_th_multiple_3_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Pair_Th_Multiple_3_reg);
    yc_3d_f4_pair_th_multiple_3_reg.diff_th_multiple_40 = diff_th_multiple_40;
    yc_3d_f4_pair_th_multiple_3_reg.diff_th_multiple_20 = diff_th_multiple_20;
    yc_3d_f4_pair_th_multiple_3_reg.diff_th_multiple_10 = diff_th_multiple_10;
    yc_3d_f4_pair_th_multiple_3_reg.diff_th_multiple_0 = diff_th_multiple_0;
    VDPQ_rtd_outl(VDPQ_YC_3D_F4_Pair_Th_Multiple_3_reg, yc_3d_f4_pair_th_multiple_3_reg.regValue);

}

void scalerVideo_update_motion_count(unsigned char motion, unsigned char threshold )
{
	unsigned int i=0;
    // update the motion_cnt vector to statistic the motion information
    if( motion != 0 )
        motion_cnt[0] = 0;

    for( i=0; i<10; i++ )
    {
        if( (i != motion) && (motion_cnt[i] > 0) )
            motion_cnt[i]--;
        else if( (i == motion) && (motion_cnt[i] < threshold) )
            motion_cnt[i]++;
    }

}
unsigned char scalerVideo_array_max(unsigned char* array, unsigned char arraySize)
{
    // find the index of max value in the array
    // if there are several max value, return the largest index
    unsigned char TmpMaxValue = 0;
    unsigned char MaxValueIdx = 0;

    unsigned int i=0;
    for( i=0; i<arraySize; i++ )
    {
        if( TmpMaxValue < array[i] )
        {
            TmpMaxValue = array[i];
            MaxValueIdx = i;
        }
    }
    return MaxValueIdx;
}

unsigned int scalerVideo_BitWiseOperation(unsigned int Input, unsigned char StartIdx, unsigned char EndIdx, unsigned int Value, ScalerVideo_BitWiseOperationMode OP_Mode)
{
  // 20151012, created by willy
  // this function do bit wise operation on input value [EndIdx:StartIdx] according to the operation mode
  // VD_BIT_SET : set the [EndIdx:StaIdx] bit to Value
  // VD_BIT_CLEAR : clear the [Start] bit to Value
  unsigned int Mask = 0;
  unsigned int i=0;
  unsigned int FinalValue = 0;

  // input parameters error protection
  if( (StartIdx > EndIdx) || ( StartIdx >= 32 ) || ( EndIdx >= 32 ) ||  ( Input == 0xdeaddead ))
    return 0;

  // generate the Mask
  for(i=StartIdx;i<=EndIdx;i++)
    Mask +=  ( 1 << (i));

  switch(OP_Mode)
  {
    case VD_BIT_GET:
    {
      FinalValue = ((Input&Mask) >> StartIdx);
      break;
    }
    case VD_BIT_SET:
    {
      if((Mask>>StartIdx) < Value)
        FinalValue = 0;
      else
        FinalValue = ( (Input & (Mask^0xffffffff)) | (Value<<StartIdx));

      break;
    }
    default:
    {
      FinalValue = 0;
      break;
    }
  }

  return FinalValue;

}

int scalerVideo_HUE_BinRatioSum(int StartIdx, int EndIdx)
{
  // 20151216, created by willy
  // this function return the ratio sum of the specific range of the Hue histogram
  // Color range : RED 0~4 GREEN 5~9 BLUE 13~18
  int i=0, sum=0;

  // input parameters error protection
  if( (StartIdx < 0) || (EndIdx >= COLOR_HUE_HISTOGRAM_LEVEL) || EndIdx<StartIdx )
    return -1;

  // get the result
  for( i=StartIdx; i<=EndIdx; i++ )
    sum += ScalerVideo_Status.Hue_HistogramRatio[i];

  return sum;
}

int scalerVideo_SMD_BinRatioSum(int StartIdx, int EndIdx)
{
  // 20151010, created by willy
  // this function return the ratio sum of the specific range of the saturation histogram
  int i=0, sum=0;

  // input parameters error protection
  if( (StartIdx < 0) || (EndIdx >= DI_SMD_BIN_NUM) || EndIdx<StartIdx )
    return -1;

  // get the result
  for( i=StartIdx; i<=EndIdx; i++ )
    sum += ScalerVideo_Status.SMD_BinRatio[i];

  return sum;
}

int scalerVideo_SAT_BinRatioSum(int StartIdx, int EndIdx)
{
  // 20151010, create by willy
  // this function return the ratio sum of the specific range of the saturation histogram
  int i=0, sum=0;

  // input parameters error protection
  if( (StartIdx < 0) || (EndIdx >= COLOR_AutoSat_HISTOGRAM_LEVEL) || EndIdx<StartIdx )
    return -1;

  // get the result
  for( i=StartIdx; i<=EndIdx; i++ )
    sum += sat_hist_ratio[i];

  return sum;
}

int scalerVideo_ValueSmooth(unsigned int InputTarget, unsigned int InputCurrent, unsigned int InputStep)
{
  // Smooth the register vlaue change to avoid situation unstable
  // Step stands for the change value for each DSR
  int FinalValue = 0;
  int Target = InputTarget;
  int Current = InputCurrent;
  int Step = InputStep;
  if( Target >= Current )
  {
      if( Current+Step>=Target )
        FinalValue = Target;
      else
        FinalValue = Current+Step;
  }
  else
  {
      if( Current-Step<=Target )
        FinalValue = Target;
      else
        FinalValue = Current-Step;
  }

  if( FinalValue < 0 )
    FinalValue = 0;

  return FinalValue;
}

void scalerVideo_ClearFormatChangeBitFromKernel(void)
{
	// fwused2 bit 21
	vdpq_vdpq_fw_used2_RBUS vdpq_vdpq_fw_used2_reg;
	vdpq_vdpq_fw_used2_reg.regValue = VDPQ_rtd_inl(VDPQ_VDPQ_FW_USED2_reg);
	if(vdpq_vdpq_fw_used2_reg.regValue == 0xdeaddead )
	{
		return;
	}
	vdpq_vdpq_fw_used2_reg.regValue = scalerVideo_BitWiseOperation(vdpq_vdpq_fw_used2_reg.regValue, 21, 21, 0, VD_BIT_SET);
	VDPQ_rtd_outl(VDPQ_VDPQ_FW_USED2_reg, vdpq_vdpq_fw_used2_reg.regValue);
}

void scalerVideo_RegApplyCtrl(ScalerVideo_RegisterControlSwitch Status)
{
  switch(Status)
  {
    case VD_REG_CTRL_ENABLE:
    {
      ScalerVideo_Status.ApplyEnable = TRUE;
      break;
    }
    case VD_REG_CTRL_DISABLE:
    {
      ScalerVideo_Status.ApplyEnable = FALSE;
      break;
    }
    default:
      break;
  }
}

void scalerVideo_AddSetToBuffer(unsigned char Num, ScalerVideo_SetType Type)
{
	if( ScalerVideo_Buffer.CurIndex < VD_BUFFER_LENGTH )
	{
		ScalerVideo_Buffer.Buffer[ScalerVideo_Buffer.CurIndex].SetNum = Num;
		ScalerVideo_Buffer.Buffer[ScalerVideo_Buffer.CurIndex].SetType = Type;
		ScalerVideo_Buffer.CurIndex = ScalerVideo_Buffer.CurIndex + 1;
	}
	else
	{
		ROS_VideoPrintf_ErrorStatus("[Buffer] Error, Buffer Full!\n");
	}
}


void scalerVideo_BufferOperation(ScalerVideo_BufferOperation Op)
{
	unsigned int i = 0;
	switch(Op)
	{
		case VD_BUFFER_RESET_IDX:
		{

			ROS_VideoPrintf_Setting("-----[ Buffer index reset ]-----\n");
			ScalerVideo_Buffer.CurIndex = 0;
			break;
		}
		case VD_BUFFER_CLEAR_BY_ORDER:
		{
			if( ScalerVideo_Debug.AutoDisable_ApplyBuffer == FALSE )
			{

				ROS_VideoPrintf_Setting("-----[ Buffer clear by order STA ]-----\n");
				for(i=0; i<ScalerVideo_Buffer.CurIndex; i++)
				{
					if( ScalerVideo_Buffer.Buffer[i].SetType == VD_APPLY_SET )
						scalerVideo_ApplyVDSetting(ScalerVideo_Buffer.Buffer[i].SetNum);
					/*
					if( ScalerVideo_Buffer.Buffer[i].SetType == VD_RESET_SET )
					scalerVideo_ResetVDSetting(ScalerVideo_Buffer.Buffer[i].SetNum);
					*/
				}

				ROS_VideoPrintf_Setting("-----[ Buffer clear by order END ]-----\n");
			}
			break;
		}
		default:
		{
			ROS_VideoPrintf_ErrorStatus("-----[ Undefined buffer operation ]-----\n");
			break;
		}
	}

}

void scalerVideo_UpdateFrequencyOffset(void)
{	
	static int preCnt = 0, ptsOft = 0, frmCnt = 0;
	const unsigned int ptsPreLine = 1135; // PAL : 1135, NTSC : 910
	const unsigned int linePerFrm = 625; // PAL : 625, NTSC : 525
	const unsigned int updtPeriod = 60;
	unsigned int i, curCnt = 0;
	vdpq_dma_ctrl6_RBUS vdpq_dma_ctrl6_reg;
	vdpq_dma_ctrl6_reg.regValue = VDPQ_rtd_inl(VDPQ_DMA_CTRL6_reg);

	curCnt = (vdpq_dma_ctrl6_reg.dma_mon_hcnt + vdpq_dma_ctrl6_reg.dma_mon_vcnt*ptsPreLine);

	if( ABS(curCnt, preCnt) > ( (ptsPreLine*linePerFrm)>>4 ) )
	{
		if(curCnt > preCnt)
		{
			ptsOft = ptsOft + curCnt - ((ptsPreLine*linePerFrm) + preCnt);
		}
		else
		{
			ptsOft = ptsOft + (curCnt+(ptsPreLine*linePerFrm)) - preCnt;
		}
	}
	else
	{
		ptsOft = ptsOft + (curCnt - preCnt);
	}

	frmCnt = (frmCnt + 1) % updtPeriod;

	// Update status
	if(frmCnt == 0)
	{
		ScalerVideo_Status.FrequencyOffset = _FSC_overflow;
		for(i=0; i<_FSC_overflow; i++)
		{
			if( ptsOft >= FREQ_OFST_TBL[i][1] && ptsOft <= FREQ_OFST_TBL[i][2] )
				ScalerVideo_Status.FrequencyOffset = FREQ_OFST_TBL[i][0];
		}
		ptsOft = 0;
        preCnt = 0;
	}else{
        preCnt = curCnt;
    }
}

void scalerVideo_UpdateMotionStatus(void)
{
    // get the motion status from DI and update the motion status in ScalerVideo_Status
    // only seperated by NTSC and PAL

    unsigned char temp_motion_th = 0;
    unsigned char motion_status = 0;
    unsigned char ucModeResult = 0;
    static unsigned char motion_status_downward_cnt = 0, motion_status_upward_cnt = 0;
    static unsigned char original_motion_status = 4;
    unsigned char downward_cnt_threshold = 0, upward_cnt_threshold = 0;
    unsigned int FM = 0, FM_pre = 0, FM_next = 0;
    di_im_di_si_film_motion_RBUS im_di_si_film_motion_reg;
    di_im_di_si_film_motion_pre_RBUS im_di_si_film_motion_pre_reg;
    di_im_di_si_film_motion_next_RBUS im_di_si_film_motion_next_reg;
    static unsigned char motion_corrected = 0;
    static unsigned char SC_flag = 0;
    static unsigned char MotionDirection = 0;

    ScalerVideo_Status.VD_Mode = scalerVideo_readMode();
    motion_status =  scalerVideo_MotionDecision();

    if ( ScalerVideo_Status.VD_Mode == VDC_MODE_PALI )
    {
        ucModeResult = 1; //PALI
        temp_motion_th = 6;
    }
    else if ( (ScalerVideo_Status.VD_Mode == VDC_MODE_PALM) || (ScalerVideo_Status.VD_Mode == VDC_MODE_PALN) )
    {
        //for customer2 only, PAL-MN tv color var zipper
        ucModeResult = 2; //PAL-M, PAL-N
        temp_motion_th = 6;
    }
    else if ( ScalerVideo_Status.VD_Mode == VDC_MODE_NTSC )
    {
        ucModeResult = 3; //NTSC, =0 in dragon, =3 in columbus ???
        temp_motion_th = 6;
    }
    else
        ucModeResult = 0; //  cannot detect correctly

//-----------------------------------------------------------------------------------------------------------------------------------------
// motion status statistic update and decision
    if ( ucModeResult > 0 )
    {
        scalerVideo_update_motion_count(motion_status, temp_motion_th);
        if( motion_cnt[0] == temp_motion_th )
            curr_motion_status = 0;
        else
            curr_motion_status = scalerVideo_array_max(motion_cnt, 10);
    }

    if(motion_status>=7)
        curr_motion_status = 7;

//-----------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------
// motion correction when mode is not NTSC
    if(scalerVideo_readMode() != VDC_MODE_NTSC)
    {
        im_di_si_film_motion_reg.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_reg);
        FM = im_di_si_film_motion_reg.film_motionstatus_399_380;
        im_di_si_film_motion_pre_reg.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_PRE_reg);
        FM_pre = im_di_si_film_motion_pre_reg.film_motionstatus_559_540;
        im_di_si_film_motion_next_reg.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_NEXT_reg);
        FM_next = im_di_si_film_motion_next_reg.film_motionstatus_719_700;

        if((curr_motion_status==5)|| ((curr_motion_status==4)
            && (FM < 0x1000) && (FM_pre < 0x1000) && (FM_next < 0x1000)
            && (FM > 0x200)  && (FM_pre > 0x200)  && (FM_next > 0x200)))
            motion_corrected = curr_motion_status - 1;
        else if( (curr_motion_status >= 6) && (FM < 0x2500) && (FM_pre < 0x2000) && (FM_next < 0x2000)) //for ditsu_girl
            motion_corrected = curr_motion_status - 1;
        else
            motion_corrected = curr_motion_status;
    }
//-----------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------
// motion debounce
// motion_corrected : the motion result from register and correction

    // debounce threshold setting
    downward_cnt_threshold = 5;
    upward_cnt_threshold = 5;

    // Scene change detection
    if( (ScalerVideo_Status.RPC_SmartPicClue->SceneChange) ||
        (curr_motion_status - original_motion_status >= 5) ||
        (original_motion_status - curr_motion_status >= 5) )
        SC_flag = 1;
    else
        SC_flag = 0;

    // Debounce when there is not scene change
    // before & after scene change, we have to instantly refresh the used motion status
    if( SC_flag == 1 )
    {
        motion_status_downward_cnt = 0;
        motion_status_upward_cnt = 0;
        original_motion_status = curr_motion_status;
    }
    else
    {
        if(original_motion_status > motion_corrected) // motion to still (DOWN)
        {
            // reset the upward count
            if(MotionDirection == VD_MOTION_DIR_UP)
                motion_status_upward_cnt = 0;

            MotionDirection = VD_MOTION_DIR_DOWN;

            // if the motion keep going down for a while, decrease the motion status
            motion_status_downward_cnt++;
            if(motion_status_downward_cnt >= downward_cnt_threshold)
            {
                original_motion_status--;
                motion_status_downward_cnt = 0;
            }
        }
        else if(original_motion_status < motion_corrected) // still to motion (UP)
        {
            // reset the downward count
            if(MotionDirection == VD_MOTION_DIR_DOWN)
                motion_status_downward_cnt = 0;

            MotionDirection = VD_MOTION_DIR_UP;

            // if the motion keep going up for a while, increase the motion status
            motion_status_upward_cnt++;
            if(motion_status_upward_cnt >= upward_cnt_threshold)
            {
                original_motion_status++;
                motion_status_upward_cnt = 0;
            }
        }
        else
            original_motion_status = motion_corrected;
    }

    // clip the motion status under 7
    ScalerVideo_Status.MotionStatus = (original_motion_status>7)? 7 : original_motion_status;

//-----------------------------------------------------------------------------------------------------------------------------------------

}

void scalerVideo_UpdateSpecialTable(void)
{
    // Special structure from system
    ScalerVideo_Status.SystemInfoTable = NULL;
    ScalerVideo_Status.SystemInfoTable = scaler_GetShare_Memory_system_setting_info_Struct();

     ScalerVideo_Status.RPC_SystemInfoTable = NULL;
    ScalerVideo_Status.RPC_SystemInfoTable = scaler_GetShare_Memory_RPC_system_setting_info_Struct();

    ScalerVideo_Status.VIPTable = NULL;
    ScalerVideo_Status.VIPTable = scaler_GetShare_Memory_VIP_TABLE_Struct();

    ScalerVideo_Status.SmartPicClue = NULL;
    ScalerVideo_Status.SmartPicClue = scaler_GetShare_Memory_SmartPic_Clue_Struct();

    ScalerVideo_Status.RPC_SmartPicClue = NULL;
    ScalerVideo_Status.RPC_SmartPicClue = scaler_GetShare_Memory_RPC_SmartPic_Clue_Struct();
}

void scalerVideo_UpdateSourceType(void)
{

    if( ScalerVideo_Status.RPC_SystemInfoTable->VIP_source == VIP_QUALITY_TV_NTSC    ||
        ScalerVideo_Status.RPC_SystemInfoTable->VIP_source == VIP_QUALITY_TV_NTSC443 ||
        ScalerVideo_Status.RPC_SystemInfoTable->VIP_source == VIP_QUALITY_TV_PAL     ||
        ScalerVideo_Status.RPC_SystemInfoTable->VIP_source == VIP_QUALITY_TV_PAL_M   ||
        ScalerVideo_Status.RPC_SystemInfoTable->VIP_source == VIP_QUALITY_TV_SECAN
      )
        ScalerVideo_Status.src_is_tv = TRUE;
    else
        ScalerVideo_Status.src_is_tv = FALSE;

    ScalerVideo_Status.VD_Mode = scalerVideo_readMode();
    ScalerVideo_Status.Source = ScalerVideo_Status.RPC_SystemInfoTable->VIP_source;

}

void scalerVideo_UpdateNoiseStatus(void)
{
    static unsigned char debounce_cnt = 10;
	static unsigned char noise_debounce = 0;
    unsigned char tmp_noise = VDPQ_rtd_inl(VDTOP_NOISE_STATUS_reg);

    if( tmp_noise != noise_debounce )
    {
		debounce_cnt--;
        if( debounce_cnt == 0 )
        {
            noise_debounce = tmp_noise;
            debounce_cnt = 10;
        }
    }
    else
    {
		debounce_cnt += 5;
		if( debounce_cnt >= 10 )
			debounce_cnt = 10;
    }

    ScalerVideo_Status.NoiseStatus = noise_debounce;
}

void scalerVideo_UpdateYCSeperationMode(void)
{
    vdpq_yc_sep_control_RBUS yc_sep_control_reg;
    ScalerVideo_Status.YCSepModeFromSystem = ((ScalerVideo_Status.SystemInfoTable->YC_separation_status) & 0x0f);

    yc_sep_control_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_SEP_CONTROL_reg);
    ScalerVideo_Status.YCSepModeFromReg = yc_sep_control_reg.adaptive_mode;
}

void scalerVideo_UpdateSaturation(void)
{

    unsigned char  TmpCount;
    unsigned int i;

    for( i=0; i<COLOR_AutoSat_HISTOGRAM_LEVEL; i++ )
      ScalerVideo_Status.Sat_HistogramRatio[i] = sat_hist_ratio[i];

    // for NTSC
    TmpCount = 0;
    for( i=14; i<COLOR_AutoSat_HISTOGRAM_LEVEL; i++)
    {
        if(ScalerVideo_Status.Sat_HistogramRatio[i] > 100)
            TmpCount++;
    }
    ScalerVideo_Status.HighSatFlag_Bin_14_31 = (TmpCount>2)? 1 : 0;

    // for PAL
    TmpCount = 0;
    for( i=6; i<COLOR_AutoSat_HISTOGRAM_LEVEL; i++)
    {
        if(ScalerVideo_Status.Sat_HistogramRatio[i] > 200)
            TmpCount++;
    }
    ScalerVideo_Status.HighSatFlag_Bin_06_31 = (TmpCount>0)? 1 : 0;


}

void scalerVideo_UpdateHueHistogram(void)
{
    unsigned int i=0;
    unsigned int TmpSum = 0;
    unsigned int TmpHistogram[COLOR_HUE_HISTOGRAM_LEVEL] = {0};
    for(i=0; i<COLOR_HUE_HISTOGRAM_LEVEL; i++)
    {
        TmpHistogram[i] = rtd_inl( COLOR_ICM_HUE_COUNT_00_reg + (i<<2)) >> 1; // 20150821, willy, LG #13 overflow
        TmpSum += TmpHistogram[i];
    }

    TmpSum +=1; // to prevent divide by 0
    for(i=0;i<COLOR_HUE_HISTOGRAM_LEVEL;i++)
         ScalerVideo_Status.Hue_HistogramRatio[i] = (TmpHistogram[i]*1000/TmpSum);

    ScalerVideo_Status.Hue_Sum = TmpSum;
}

void scalerVideo_UpdateSMDInfo(void)
{
    unsigned int i = 0;
    unsigned int temp_value, bin_sum;
    unsigned short DI_SMD_SAD_His_Bin[DI_SMD_BIN_NUM]={0};
    unsigned short DI_FMV_His_Bin[DI_FMV_BIN_NUM]={0};
    unsigned short FMV_Bin_00_09 = 0;
    unsigned short FMV_Bin_11_20 = 0;


    //----------------------------------------------------------------------------------------------------------------------
    // SMD information to ScalerVideo_Status.SMD_BinRatio
    // 1. get the SMD_SAD_His_Bin[0]~[15]
    for(i=0; i<DI_SMD_BIN_NUM; i+=2)
    {
        temp_value = rtd_inl(DI_DI_SMD_SADHisBinA_reg+ ((i/2)<<2) );

        DI_SMD_SAD_His_Bin[i+1] = temp_value & (0x0000ffff); // get [15:0]
        DI_SMD_SAD_His_Bin[i] = temp_value>>16; // get [31:16]
    }

    // 2. calculate SMD_Bin_ratio (permillage)
    bin_sum = 1; // initialize, and prevent divided by 0
    for(i=0; i<DI_SMD_BIN_NUM; i++)
        bin_sum += DI_SMD_SAD_His_Bin[i];

    for(i=0; i<DI_SMD_BIN_NUM; i++)
        ScalerVideo_Status.SMD_BinRatio[i] = (DI_SMD_SAD_His_Bin[i]*1000)/bin_sum;
    //----------------------------------------------------------------------------------------------------------------------

    //----------------------------------------------------------------------------------------------------------------------
    // FMV information to ScalerVideo_Status.FMV_BinRatio
    // 1. get the FMV_bin
    for(i=0; i<=((DI_FMV_BIN_NUM/2)-2); i+=2) // get FMV_bin[0]~FMV_bin[9]
    {
        temp_value = rtd_inl(DI_DI_GST_FMVHisA_reg+ ((i/2)<<2) );

        DI_FMV_His_Bin[i+1] = temp_value & (0x0000ffff); // get [15:0]
        DI_FMV_His_Bin[i] = temp_value>>16; // get [31:16]
    }

    //PS. FMV_His10 indicates the number of still pixels. We don't get DI_FMV_His_Bin[10] because we don's need this info
    DI_FMV_His_Bin[10]=0;

    for(i=((DI_FMV_BIN_NUM/2)+2); i<DI_FMV_BIN_NUM; i+=2) // get FMV_bin[11]~FMV_bin[20]
    {
        temp_value = rtd_inl(DI_DI_GST_FMVHisA_reg+ ((i/2)<<2) );

        DI_FMV_His_Bin[i] = temp_value & (0x0000ffff); // get [15:0]
        DI_FMV_His_Bin[i-1] = temp_value>>16; // get [31:16]
    }

    // 2. calculate FMV_Bin_ratio (permillage)
    bin_sum=1; // initialize, and prevent divided by 0
    for(i=0; i<DI_FMV_BIN_NUM; i++)
        bin_sum += DI_FMV_His_Bin[i];

    for(i=0; i<DI_FMV_BIN_NUM; i++)
        ScalerVideo_Status.FMV_BinRatio[i] = (DI_FMV_His_Bin[i]*1000)/bin_sum;
    //----------------------------------------------------------------------------------------------------------------------

    //----------------------------------------------------------------------------------------------------------------------
    // Special condition for applying register setting
    for( i=0; i<9; i++ )
        FMV_Bin_00_09 += ScalerVideo_Status.FMV_BinRatio[i];

    for( i=11; i<20; i++ )
        FMV_Bin_11_20 += ScalerVideo_Status.FMV_BinRatio[i];

    ScalerVideo_Status.FMV_LRDiff = ABS(FMV_Bin_00_09, FMV_Bin_11_20);
    //----------------------------------------------------------------------------------------------------------------------

}

void scalerVideo_UpdateSpecialFlag(void)
{

    // in this function, construct the special condition flag and debounce condition
    //-------------------------------------------------------------------------------------------
    // PAL de-XC enable
    // high frequency change from 6 to 8 to avoid DOS#45 disable de-XC and make cross color
    static unsigned char PAL_deXC_Counter = 0;
    static unsigned char MovingCan_Counter = 0;
    static unsigned char LG_86_91_Counter = 0;
    static unsigned char SMD_Counter = 0;
    static unsigned char PAL_SunFlowerRoof_Counter = 0;
    static unsigned char VD_SMD_Counter = 0;
    static unsigned char Pattern44_Counter = 0;
    static unsigned char DiagonalEdgeStable_Counter = 0;
    static unsigned char LG_VIP_194_Counter = 0;



    if( ScalerVideo_Status.VD_Mode == VDC_MODE_PALI )
    {
        if( (DynamicOptimizeSystem[2] != TRUE) &&
            (DynamicOptimizeSystem[75] != TRUE) &&
            ((scalerVIP_DI_MiddleWare_GetStatus(STATUS_HIGHFREQUENCY_LEVEL) <  8/*6*/ && ScalerVideo_Status.HighSatFlag_Bin_06_31) ||
             (scalerVIP_DI_MiddleWare_GetStatus(STATUS_HIGHFREQUENCY_LEVEL) >= 8/*6*/ && scalerVideo_SMD_BinRatioSum(0, 0) > 997) )
          )
        {
            if( PAL_deXC_Counter < 15 )
                PAL_deXC_Counter++;
        }
        else
        {
            if( PAL_deXC_Counter > 0 )
                PAL_deXC_Counter--;
        }
    }
    if( PAL_deXC_Counter > 9 )
        ScalerVideo_Status.Flag_PAL_DeXC = TRUE;
    else
        ScalerVideo_Status.Flag_PAL_DeXC = FALSE;
    //-------------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------------------------
    // for LG pattern #75 moving can
    // 20150924, for black level change by LG DB, we need to reconsider the Hue/Sat related condition
    if( DynamicOptimizeSystem[75] == 1 )
    {
      if( MovingCan_Counter <= 50 )
        MovingCan_Counter = MovingCan_Counter + 1;
    }
    else
    {
      if( MovingCan_Counter >= 5 )
        MovingCan_Counter = MovingCan_Counter - 5;
    }
    if( MovingCan_Counter >= 10 )
      ScalerVideo_Status.Flag_MovingCan = TRUE;
    else
      ScalerVideo_Status.Flag_MovingCan = FALSE;
    //-------------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------------------------
    // SMD related flags
    if( ScalerVideo_Status.VD_Mode == VDC_MODE_NTSC )
    {
        // for SMD
        static unsigned char SMD_Counter = 0;

        if( scalerVideo_SMD_BinRatioSum(5, 15) < 100 &&                            // SMD is not large
            scalerVideo_SMD_BinRatioSum(0, 0) < 990 &&                             // not very very still
            scalerVideo_SAT_BinRatioSum(5, 31) < 350 &&                            // saturation is not large
            scalerVIP_DI_MiddleWare_GetStatus(STATUS_HIGHFREQUENCY_LEVEL) > 30 &&  // DI's hifreq. info.
            scalerVIP_DI_MiddleWare_GetStatus(STATUS_PAN) == 0)
        {
          SMD_Counter += 10;
          if(SMD_Counter > 100)
            SMD_Counter = 100;
        }
        else
        {
          if(SMD_Counter > 0)
            SMD_Counter--;
        }
        if( SMD_Counter > 0 )
            ScalerVideo_Status.Flag_SMD = TRUE;
        else
            ScalerVideo_Status.Flag_SMD = FALSE;

        // for LG pattern [2][4][86] [2][4][91]
        if( scalerVideo_SMD_BinRatioSum(5, 15) < 200 &&                            // SMD is not large
            scalerVideo_SMD_BinRatioSum(0, 0 ) < 985 &&                             // not very still
            scalerVideo_SAT_BinRatioSum(5, 31) > 200 &&                       // saturation is not small/*100*/
            ScalerVideo_Status.FMV_LRDiff > 200 && /*580*/                          // slow-move to right or left
            ScalerVideo_Status.MotionStatus < 3 &&
            scalerVIP_DI_MiddleWare_GetStatus(STATUS_PAN) == 0 &&
            scalerVIP_DI_MiddleWare_GetStatus(STATUS_HIGHFREQUENCY_LEVEL) < 10 &&   //DI's hifreq. info./*100*/
            scalerVIP_Diverse_Hue_Detector(500,250)/*(630,250)*/ == 0)              // to distinguish LG-NTSC_30P & 24P-ThilandWoman
        {
            LG_86_91_Counter += 10;
            if( LG_86_91_Counter > 100 )
                LG_86_91_Counter = 100;
        }
        else
        {
            if( LG_86_91_Counter > 0)
                LG_86_91_Counter--;
        }
        if( LG_86_91_Counter > 0 && DynamicOptimizeSystem[139] == 0 && DynamicOptimizeSystem[107] == 0 && DynamicOptimizeSystem[5] == 0 && DynamicOptimizeSystem[6] == 0)
            ScalerVideo_Status.Flag_DOS_86_91 = TRUE;
        else
            ScalerVideo_Status.Flag_DOS_86_91 = FALSE;

        // for LG Pattern [2][1][22] SunFlowerRoof
        if( DynamicOptimizeSystem[5] &&
            ScalerVideo_Status.MotionStatus < 5 &&
            scalerVIP_DI_MiddleWare_GetStatus(STATUS_HIGHFREQUENCY_LEVEL) > 55 &&
            scalerVIP_DI_MiddleWare_GetStatus(STATUS_PAN) == 0 )
        {
            ScalerVideo_Status.Flag_SunFlowerRoof = TRUE;
        }
        else
        {
            ScalerVideo_Status.Flag_SunFlowerRoof = FALSE;
        }

    }
    else // PAL
    {
        if( scalerVideo_SMD_BinRatioSum(5, 15) < 100 &&                             // SMD is not large
            scalerVideo_SMD_BinRatioSum(0, 0 ) < 990 &&                             // not very very still
            scalerVideo_SAT_BinRatioSum(5, 31) < 100 &&                             // saturation is not large
            ScalerVideo_Status.FMV_LRDiff > 300 &&
            scalerVIP_DI_MiddleWare_GetStatus(STATUS_HIGHFREQUENCY_LEVEL) > 80 &&   //DI's hifreq. info.
            scalerVIP_DI_MiddleWare_GetStatus(STATUS_PAN) == 0)
        {
            SMD_Counter += 10;
            if(SMD_Counter > 100)
                SMD_Counter = 100;
        }
        else
        {
            if(SMD_Counter > 0)
                SMD_Counter--;
        }
        if( SMD_Counter > 0 )
            ScalerVideo_Status.Flag_SMD = TRUE;
        else
            ScalerVideo_Status.Flag_SMD = FALSE;


        if( DynamicOptimizeSystem[5] &&
            ScalerVideo_Status.MotionStatus < 5 &&
            scalerVIP_DI_MiddleWare_GetStatus(STATUS_HIGHFREQUENCY_LEVEL) > 35 &&
            scalerVIP_DI_MiddleWare_GetStatus(STATUS_PAN) == 0 )
        {
            PAL_SunFlowerRoof_Counter += 5;
            if( PAL_SunFlowerRoof_Counter >= 30 )
              PAL_SunFlowerRoof_Counter = 30;
        }
        else
        {
            if( PAL_SunFlowerRoof_Counter >= 1 )
              PAL_SunFlowerRoof_Counter -= 1;
            else
              PAL_SunFlowerRoof_Counter = 0;
        }

        if( PAL_SunFlowerRoof_Counter > 10 )
        {
            ScalerVideo_Status.Flag_PAL_SunFlowerRoof = TRUE;
        }
        else
        {
            ScalerVideo_Status.Flag_PAL_SunFlowerRoof = FALSE;
        }

    }
    //-------------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------------------------
    // VD STRONG SMD, willy 20150819
    // target : #6, #12, #24, #86, #104(current not included)
    // exculde: #11, #44, #45, #46, #48, #49, #107
    if( scalerVideo_SMD_BinRatioSum(0, 0 ) <= 990 &&                            // not very still (from original SMD)
        scalerVideo_SMD_BinRatioSum(5, 15) < 100 &&                             // motion not huge (from original SMD)
        scalerVideo_SMD_BinRatioSum(1, 4 ) > 70 &&                              // small motion is not small
        ScalerVideo_Status.FMV_LRDiff > 40 &&                                   // exclude ref-4
        scalerVIP_DI_MiddleWare_GetStatus(STATUS_HIGHFREQUENCY_LEVEL) > 1 &&    // exclude #102 for DI to apply strong weave setting
        scalerVIP_DI_MiddleWare_GetStatus(STATUS_MOTION) <= 20 &&               // exclude #39 iron tower, avoid trembling
        scalerVIP_Diverse_Hue_Detector(380, 250) == TRUE                        // exclude #45 cloak, #46 flower dot crawl
        //scalerVIP_DI_MiddleWare_GetStatus(STATUS_PAN) == 0
      )
    {
        VD_SMD_Counter = VD_SMD_Counter + 1;
        if( VD_SMD_Counter >= 50 )
            VD_SMD_Counter = 50;
    }
    else
    {
        if( VD_SMD_Counter >= 5 )
            VD_SMD_Counter = VD_SMD_Counter - 5;
    }
    // use debounce to exclude #107, #45 Flower dot crawl(35)
    if( VD_SMD_Counter > 10 )
        ScalerVideo_Status.Flag_VD_SMD = TRUE;
    else
        ScalerVideo_Status.Flag_VD_SMD = FALSE;
    //-------------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------------------------
    // LG Shaking Woman avoid 2D 3D jump, 20150820, willy
    if( DynamicOptimizeSystem[44] == 1 )
    {
        Pattern44_Counter++;
        if( Pattern44_Counter > 5 )
            Pattern44_Counter = 5;
    }
    else
    {
        if( Pattern44_Counter > 0)
            Pattern44_Counter--;
    }

    if( Pattern44_Counter > 0)
        ScalerVideo_Status.Flag_DOS_44 = TRUE;
    else
        ScalerVideo_Status.Flag_DOS_44 = FALSE;
    //-------------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------------------------
    // For 2d3d jump peaking > try to apply strong post 2d bpf y to make edge clear
    // Target : DOS#28 arm, DOS#63 woman arm
    // Avoid : DOS#46 cloth dot crawl when turn around, DOS#40 Flag dot crawl
    if( ScalerVideo_Status.VD_Mode == VDC_MODE_NTSC &&
        scalerVideo_SMD_BinRatioSum(1, 4) >= 35 &&
        scalerVideo_SMD_BinRatioSum(1, 4) <= 90 &&
        scalerVideo_SAT_BinRatioSum(2, 4) <= 300 &&               // avoid DOS#40 flag dot crawl
        scalerVIP_DI_MiddleWare_GetStatus(STATUS_MOTION) >= 4 &&
        scalerVIP_DI_MiddleWare_GetStatus(STATUS_MOTION) <= 8 &&
        scalerVIP_Diverse_Hue_Detector(510, 250) == FALSE         // avoid DOS#46 clothes/flower dot crawl when turnning around
      )
    {
      DiagonalEdgeStable_Counter = DiagonalEdgeStable_Counter + 1;
      if( DiagonalEdgeStable_Counter >= 50 )
        DiagonalEdgeStable_Counter = 50;
    }
    else
    {
      if( DiagonalEdgeStable_Counter >= 5 )
        DiagonalEdgeStable_Counter = DiagonalEdgeStable_Counter - 5;
    }
    if(DiagonalEdgeStable_Counter >= 20 )
      ScalerVideo_Status.Flag_DiagonalEdgeStable = TRUE;
    else
      ScalerVideo_Status.Flag_DiagonalEdgeStable = FALSE;
    //-------------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------------------------
    // VD status update, 20150827, willy
    ScalerVideo_Status.Status_HighFreq_not_small = scalerVideo_Get_VD_MiddleWare(VD_STATUS_HIFREQ_NOT_SMALL);
    ScalerVideo_Status.Status_Hor_C_change       = scalerVideo_Get_VD_MiddleWare(VD_STATUS_HOR_C_CHANGE);
    ScalerVideo_Status.Status_Temporal_spd_small = scalerVideo_Get_VD_MiddleWare(VD_STATUS_TEMPORAL_SPD_SMALL);
    ScalerVideo_Status.Status_MGHF_flag          = scalerVideo_Get_VD_MiddleWare(VD_STATUS_MGHF_FLAG);

    ScalerVideo_Status.Noise_Density             = scalerVideo_Get_VD_MiddleWare(VD_STATUS_NOISE_DENSITY);
    ScalerVideo_Status.Noise_meanNxV             = scalerVideo_Get_VD_MiddleWare(VD_STATUS_NOISE_LARGE_VARIANCE_MEAN);
    ScalerVideo_Status.Noise_Vmm                 = scalerVideo_Get_VD_MiddleWare(VD_STATUS_NOISE_VARIANCE_MEAN);
    //-------------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------------------------
    // Information fo I-EGSM
    if( ScalerVideo_Status.Flag_SMD == TRUE &&
        scalerVideo_SMD_BinRatioSum(0, 0) <= 950 )
        ScalerVideo_Status.Flag_SMD_IEGSM = TRUE;
    else
        ScalerVideo_Status.Flag_SMD_IEGSM = FALSE;
    //-------------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------------------------
    // Debounce for DynamicOptimizeSystem[194] in PAL
    if( ScalerVideo_Status.VD_Mode == VDC_MODE_PALI &&
        DynamicOptimizeSystem[194] == 1
    )
    {
      LG_VIP_194_Counter += 8;
      if( LG_VIP_194_Counter >= 40 )
        LG_VIP_194_Counter = 40;
    }
    else
    {
      if( LG_VIP_194_Counter >= 1 )
        LG_VIP_194_Counter -= 1;
      else
        LG_VIP_194_Counter = 0;
    }
    if( LG_VIP_194_Counter > 10 )
      ScalerVideo_Status.Flag_DOS_194 = TRUE;
    else
      ScalerVideo_Status.Flag_DOS_194 = FALSE;
    //-------------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------------------------
    // DOS#4 and DOS#63 cannot be distinguished by only scalerVIP(HUE, Y, SAT), need DI information
    if( DynamicOptimizeSystem[4] == 1 &&
        scalerVIP_DI_MiddleWare_GetStatus(STATUS_HIGHFREQUENCY_LEVEL) > 90 )
      ScalerVideo_Status.Flag_DOS_4 = TRUE;
    else
      ScalerVideo_Status.Flag_DOS_4 = FALSE;
    //-------------------------------------------------------------------------------------------

}

void scalerVideo_DynamicAdjustREG(void)
{
    // created by willy, 20160115
    // dynamic control the register value
    int Score=1;

    scalerVideo_Dynamic_DeXC();
    Score = scalerVideo_Dynamic_Chroma_LPF();
    scalerVideo_Dynamic_Post2DBPF_Strength(Score);
    scalerVideo_Dynamic_Directional3D_FirmwareMode();
}

void scalerVideo_Dynamic_DeXC(void)
{
    // control register list
    // 1. b8019f00[4] deXC enable
    // 2. b8019f00[3:0] deXC threshold

	static int PreScore = 0;
	int Score=1;

    int MaxSum_Th1 = 150, MaxSum_Th2 = 470, MaxSum_Th3 = 650, MaxSum_Th4 = 800;
    int MaxSum_Offset1 = -10000, MaxSum_Offset2 = 90000, MaxSum_Offset3 = -10000;
    int LowBndThr = 10, SmallBinCount = 0;
    int MaxSum = 0, i = 0, MaxSumBinNum = 3;
    int SmallBin_Th1 = 15, SmallBin_Th2 = 20;
    int SmallBin_Offset1 = 0, SmallBin_Offset2 = -30000;


	ROS_VideoPrintf_DynamicCtrl("[%s]\n", __func__);

    // initial
    Score = scalerVIP_DI_MiddleWare_GetStatus(STATUS_HIGHFREQUENCY_LEVEL)*100 + ScalerVideo_Status.MotionStatus*3000;

	// no-motion pattern, doesn't need deXC
	if( scalerVIP_DI_MiddleWare_GetStatus(STATUS_MOTION) <= 1 )
	{
		Score = -1;
	}
	// no-color pattern, need deXC to avoid VD cross color
	else if( scalerVIP_DI_MiddleWare_GetStatus(STATUS_MOTION) >= 3 && scalerVideo_SAT_BinRatioSum(2, 31) <= 50 )
	{
		Score = 90000;
	}
	else
	{

		ROS_VideoPrintf_DynamicCtrl("init scroe = %d (%d*100+%d*300)\n",
				Score, scalerVIP_DI_MiddleWare_GetStatus(STATUS_HIGHFREQUENCY_LEVEL), ScalerVideo_Status.MotionStatus);

        // step1.
        for(i = MaxSumBinNum-1; i < COLOR_HUE_HISTOGRAM_LEVEL; i++)
        {
            int TmpSum = 0, j = 0;
            for(j = (i+1)-MaxSumBinNum; j <= i; j++)
                TmpSum += ScalerVideo_Status.Hue_HistogramRatio[j];
            if( MaxSum < TmpSum )
                MaxSum = TmpSum;
        }

		//               diverse HUE  ========================================= mono HUE
		// score offset  Offset1 ===== Offset1 ===== Offset2 ===== Offset3 ===== Offset3
		// MaxSum value   0      =====   Th1   =====   Th2   =====   Th3   =====   Th4

		{
			ROS_VideoPrintf_DynamicCtrl("Score before step1 = %d, MaxSum = %d\n", Score, MaxSum);
			ROS_VideoPrintf_DynamicCtrl("MaxSumTh1 = %d, MaxSumTh2 = %d\n", MaxSum_Th1, MaxSum_Th2);
		}

		if( MaxSum <= MaxSum_Th1 )
			Score = Score + MaxSum_Offset1;
		else if( MaxSum > MaxSum_Th1 && MaxSum <= MaxSum_Th2 )
			Score = Score + (((MaxSum_Offset2-MaxSum_Offset1)/(MaxSum_Th2-MaxSum_Th1)) * (MaxSum-MaxSum_Th1)) + MaxSum_Offset1;
		else if( MaxSum > MaxSum_Th2 && MaxSum <= MaxSum_Th3 )
			Score = Score + MaxSum_Offset2;
		else if( MaxSum > MaxSum_Th3 && MaxSum <= MaxSum_Th4 )
			Score = Score + (((MaxSum_Offset3-MaxSum_Offset2)/(MaxSum_Th4-MaxSum_Th3)) * (MaxSum-MaxSum_Th3)) + MaxSum_Offset2;
		else if( MaxSum > MaxSum_Th4 )
			Score = Score + MaxSum_Offset3;


        ROS_VideoPrintf_DynamicCtrl("Score after step1 = %d\n", Score);

    // HUE Step2.
    for(i = 0; i < COLOR_HUE_HISTOGRAM_LEVEL; i++)
    {
        if( ScalerVideo_Status.Hue_HistogramRatio[i] <= LowBndThr )
            SmallBinCount++;
    }


		ROS_VideoPrintf_DynamicCtrl("SmallBinCount = %d\n", SmallBinCount);

		// score offset  Offset1 ===== Offset1 ===== Offset2
		// BinTh value      0    =====   Th1   =====   Th2
		if( SmallBinCount <= SmallBin_Th1 )
			Score = Score + SmallBin_Offset1;
		else if( SmallBinCount > SmallBin_Th1 && SmallBinCount <= SmallBin_Th2 )
			Score = Score + (((SmallBin_Offset2-SmallBin_Offset1)/(SmallBin_Th2-SmallBin_Th1)) * (SmallBinCount-SmallBin_Th1) ) + SmallBin_Th1;
		else if( SmallBinCount > SmallBin_Th2 )
			Score = Score + SmallBin_Offset2;


		ROS_VideoPrintf_DynamicCtrl("Score after step2 = %d\n", Score);

    }

    // change score to register value
    if( Score < 0 )
        ScalerVideo_Status.DynamicREG.DeXC_Enable = 0;
    else
        ScalerVideo_Status.DynamicREG.DeXC_Enable = 1;

    Score = (Score<0? 0 : ( Score>90000? 90000 : Score ));

	// IIR debounce
	Score = (Score*1 + PreScore*7 + 4) >> 3;

	ROS_VideoPrintf_DynamicCtrl("Score debounce : pre = %d, final = %d\n", PreScore, Score);
	PreScore = Score;

    ScalerVideo_Status.DynamicREG.DeXC_Threshold = 15 - (Score/6000);


	ROS_VideoPrintf_DynamicCtrl("Final Enable = %d, threshold = %d\n", ScalerVideo_Status.DynamicREG.DeXC_Enable, ScalerVideo_Status.DynamicREG.DeXC_Threshold);

    // special deXC setting for pattern
    if( ScalerVideo_Status.VD_Mode == VDC_MODE_NTSC )
    {
        if( DynamicOptimizeSystem[2] == 1 ) // hanging bed
        {
            ScalerVideo_Status.DynamicREG.DeXC_Enable = 1;
            ScalerVideo_Status.DynamicREG.DeXC_Threshold = 3;
        }
        else if( DynamicOptimizeSystem[7] == 1 ) // spoon panning
        {
            ScalerVideo_Status.DynamicREG.DeXC_Enable = 1;
            ScalerVideo_Status.DynamicREG.DeXC_Threshold = 3;
        }
        else if( DynamicOptimizeSystem[11] == 1 ) // sand tower
        {
            ScalerVideo_Status.DynamicREG.DeXC_Enable = 1;
            ScalerVideo_Status.DynamicREG.DeXC_Threshold = 3;
    }
        else if( DynamicOptimizeSystem[12] == 1 ) // boat
        {
            ScalerVideo_Status.DynamicREG.DeXC_Enable = 1;
            ScalerVideo_Status.DynamicREG.DeXC_Threshold = 3;
        }
        else if( DynamicOptimizeSystem[75] == 1 ) // can panning
        {
            ScalerVideo_Status.DynamicREG.DeXC_Enable = 1;
            ScalerVideo_Status.DynamicREG.DeXC_Threshold = 12;
        }
        else if( DynamicOptimizeSystem[107] == 1 ) // home shopping
        {
            ScalerVideo_Status.DynamicREG.DeXC_Enable = 0;
            ScalerVideo_Status.DynamicREG.DeXC_Threshold = 15;
        }
    }


}

int scalerVideo_Dynamic_Chroma_LPF(void)
{
    // control register list
    // b801992c[2:0] chorma LPF bandwidth selection

	int APL_Offset = 0;
	unsigned int APL = 0;
	unsigned char APL_Status = 0;
	int HighFreqLevel = 0;
	int HighFreqOffset = 0;
	int Score =0;
	int SAT_Offset = 0;
	int MotionCoef =0;
	static int PreScore = 0;


    ROS_VideoPrintf_DynamicCtrl("[%s]\n", __func__);

    // offset according to APL (APL low, Score high)
    APL_Status = scalerVIP_colorHist_Get_APL(SLR_MAIN_DISPLAY, &APL);
    if(APL_Status == TRUE)
        APL_Offset = 3000 - ((((int)APL-300)*3000)/130);
    if(APL_Offset < 0)
        APL_Offset = 0;
    else if(APL_Offset > 3000)
        APL_Offset = 3000;


	ROS_VideoPrintf_DynamicCtrl("[CLPF] Status = %d, APL = %d, Offset = %d\n", APL_Status, APL, APL_Offset);

	// Moving super high frequency will cause cross color
	HighFreqLevel = scalerVIP_DI_MiddleWare_GetStatus(STATUS_HIGHFREQUENCY_LEVEL);
	HighFreqOffset = ((APL-80)*500)/400;
	Score = (HighFreqLevel-HighFreqOffset)*scalerVIP_DI_MiddleWare_GetStatus(STATUS_MOTION);


	ROS_VideoPrintf_DynamicCtrl("[CLPF] HighFreq = %d, Offset = %d, Base = %d\n", HighFreqLevel, HighFreqOffset, Score);

    // Offset according to saturation
    // offset 2000  |---| -3000
    // SAT    -----100-300-----
    if( scalerVideo_SAT_BinRatioSum(2, 31) <= 100 )
        SAT_Offset = 2000;
    else if( scalerVideo_SAT_BinRatioSum(2, 31) >= 300 )
        SAT_Offset = -3000;
    else
        SAT_Offset = 2000 - (((scalerVideo_SAT_BinRatioSum(2, 31)-100)*5000)/400);


	ROS_VideoPrintf_DynamicCtrl("[CLPF] SAT_Offset = %d\n", SAT_Offset);

    // Change to register value
    Score = Score + SAT_Offset + APL_Offset;
    if( Score >= 7000 )
    {
        Score = 7000;
	}
    else if( Score <= 0 )
    {
        Score = 0;
	}

	// Protect the almost still image
	MotionCoef = (scalerVIP_DI_MiddleWare_GetStatus(STATUS_MOTION) < 4)? 0 : ( scalerVIP_DI_MiddleWare_GetStatus(STATUS_MOTION)-4 )*5+32;
	Score = (Score * MotionCoef) >> 8;

	ROS_VideoPrintf_DynamicCtrl("[CLPF] Multiply %d by DI Motion\n", MotionCoef);

	// Bouncing protection
	Score = (Score*1 + PreScore*7 + 4) >> 3;
	PreScore = Score;
	ScalerVideo_Status.DynamicREG.ChromaLPF_BandWidth = ((Score+500)/1000);

	ROS_VideoPrintf_DynamicCtrl("[CLPF] Score = %d, RegisterValue = %d\n", Score, ScalerVideo_Status.DynamicREG.ChromaLPF_BandWidth);

    return Score;

}

int scalerVideo_Dynamic_Post2DBPF_Strength(int InputScore)
{
    // control register list
    // b8019db4[14:8] hor_cdiff_th_base
    // b8019db4[6:0]  ver_cdiff_th_base

	unsigned int APL = 0;
	unsigned char APL_Status = 0;
	int HighFreqLevel = 0;
	int HighFreqOffset = 0;
	int HighFreqScore = 0;
	int Score = 0;
	int MotionCoef =0;
	static int PreScore = 0;
	int RegValue = 0;


	ROS_VideoPrintf_DynamicCtrl("[%s]\n", __func__);

    // High frequency score with APL
    APL_Status = scalerVIP_colorHist_Get_APL(SLR_MAIN_DISPLAY, &APL);
    if( APL_Status == FALSE )
    {
        APL = 480;
	}

	HighFreqLevel = scalerVIP_DI_MiddleWare_GetStatus(STATUS_HIGHFREQUENCY_LEVEL);
	HighFreqOffset = ((APL-80)*560)/400;
	HighFreqScore = ((HighFreqLevel-HighFreqOffset)>>4);
	if( HighFreqScore < 5 )
		HighFreqScore = 5;

	ROS_VideoPrintf_DynamicCtrl("[POST2DBPF] HighFreq = %d, APL = %d, Offset = %d, HighFreqScore = %d\n", HighFreqLevel, APL, HighFreqOffset, HighFreqScore);

	// Generate score including saturation
	Score = (HighFreqScore*scalerVideo_SAT_BinRatioSum(0, 1))>>2;

	ROS_VideoPrintf_DynamicCtrl("[POST2DBPF] BinRatio = %d\n", scalerVideo_SAT_BinRatioSum(0, 1));

	// Protect the almost still image
	MotionCoef = (scalerVIP_DI_MiddleWare_GetStatus(STATUS_MOTION) < 4)? 0 : ( scalerVIP_DI_MiddleWare_GetStatus(STATUS_MOTION)-4 )*5;
	Score = (Score * MotionCoef) >> 8;

	ROS_VideoPrintf_DynamicCtrl("[POST2DBPF] Multiply %d by DI Motion\n", MotionCoef);

    // Bouncing
    Score = (Score*1 + PreScore*15 + 8) >> 4;
    PreScore = Score;

    RegValue = (Score/10);
    if( RegValue > 125 )
        RegValue = 125;
    else if(RegValue < 0)
        RegValue = 0;
    ScalerVideo_Status.DynamicREG.Hor_cdiff_threshold_base = RegValue;
    ScalerVideo_Status.DynamicREG.Ver_cdiff_threshold_base = RegValue;


	ROS_VideoPrintf_DynamicCtrl("[POST2DBPF] Score = %d, RegValue = %d\n", Score, RegValue);

    return Score;
}

int scalerVideo_Dynamic_Directional3D_FirmwareMode(void)
{
	// control register list
    // b8019f80[26:25] Motion
    // b8019f80[30:27] Blending Ratio

	ScalerVideo_MotoinVector MCNR_HPAN_MV, DI_HPAN_MV;
	static int Score = 0;
	static int PreScore = 0;
	int MV_Direction = 0;

	di_im_di_hmc_pan_control_RBUS di_im_di_hmc_pan_control_reg;
	di_im_new_mcnr_pan_condition_RBUS di_im_new_mcnr_pan_condition_reg;


	ROS_VideoPrintf_DynamicCtrl("[%s]\n", __func__);

	// Read PAN information from DI
	di_im_new_mcnr_pan_condition_reg.regValue = rtd_inl(DI_IM_NEW_MCNR_PAN_CONDITION_reg);
	di_im_di_hmc_pan_control_reg.regValue = rtd_inl(DI_IM_DI_HMC_PAN_CONTROL_reg);

	if( di_im_new_mcnr_pan_condition_reg.h_pan_mv_value == 15 )
		MCNR_HPAN_MV.Dir = DIR_NONE;
	else if( di_im_new_mcnr_pan_condition_reg.h_pan_mv_value > 15 )
		MCNR_HPAN_MV.Dir = DIR_TO_LEFT;
	else
		MCNR_HPAN_MV.Dir = DIR_TO_RIGHT;
	MCNR_HPAN_MV.Offset = ABS(di_im_new_mcnr_pan_condition_reg.h_pan_mv_value, 15);

	if( di_im_di_hmc_pan_control_reg.pan_me_fw_control_motion_vector == 17 )
		DI_HPAN_MV.Dir = DIR_NONE;
	else if( di_im_di_hmc_pan_control_reg.pan_me_fw_control_motion_vector > 17 )
		DI_HPAN_MV.Dir = DIR_TO_LEFT;
	else
		DI_HPAN_MV.Dir = DIR_TO_RIGHT;
	DI_HPAN_MV.Offset = ABS(di_im_di_hmc_pan_control_reg.pan_me_fw_control_motion_vector, 17);


	{
		ROS_VideoPrintf_DynamicCtrl("MCNR Register value = %d\nDI Register value = %d\n",
			di_im_new_mcnr_pan_condition_reg.h_pan_mv_value,
			di_im_di_hmc_pan_control_reg.pan_me_fw_control_motion_vector);
		ROS_VideoPrintf_DynamicCtrl("MCNR_HPAN_MV Dir = %d, Offset = %d\n", MCNR_HPAN_MV.Dir, MCNR_HPAN_MV.Offset);
		ROS_VideoPrintf_DynamicCtrl("DI_HPAN_MV Dir = %d, Offset = %d\n", DI_HPAN_MV.Dir, DI_HPAN_MV.Offset);
	}

	//-------------------------------------------------------------------------------------
	// Case 1 for NO.75
	if( DynamicOptimizeSystem[75] == 1 &&
		( DI_HPAN_MV.Dir == DIR_TO_LEFT   && DI_HPAN_MV.Offset == 4 ) &&
		( MCNR_HPAN_MV.Dir == DIR_TO_LEFT &&  MCNR_HPAN_MV.Offset == 4 ) )
	{
		Score = 8999;
		Score = ( PreScore*15 + Score ) >> 4;
		MV_Direction = 0;
	}
	else if( DynamicOptimizeSystem[75] == 1 &&
		( DI_HPAN_MV.Dir == DIR_TO_LEFT   && DI_HPAN_MV.Offset == 4 ) &&
		( MCNR_HPAN_MV.Dir == DIR_TO_LEFT &&  MCNR_HPAN_MV.Offset == 5 ) )
	{
		Score = 12999;
		Score = ( PreScore*15 + Score ) >> 4;
		MV_Direction = 0;
	}
	else if( DynamicOptimizeSystem[75] == 1 &&
		( DI_HPAN_MV.Dir == DIR_TO_LEFT   && DI_HPAN_MV.Offset == 5 ) &&
		( MCNR_HPAN_MV.Dir == DIR_TO_LEFT &&  MCNR_HPAN_MV.Offset == 4 ) )
	{
		Score = 12999;
		Score = ( PreScore*15 + Score ) >> 4;
		MV_Direction = 0;
	}
	else if( DynamicOptimizeSystem[75] == 1 &&
		( DI_HPAN_MV.Dir == DIR_TO_LEFT   && DI_HPAN_MV.Offset == 5 ) &&
		( MCNR_HPAN_MV.Dir == DIR_TO_LEFT &&  MCNR_HPAN_MV.Offset == 5 ) )
	{
		Score = 3999;
		Score = ( PreScore*15 + Score ) >> 4;
		MV_Direction = 0;
	}
	//-------------------------------------------------------------------------------------
	// Case 2 for NO.7
	else if( DynamicOptimizeSystem[7] == 1 &&
		( DI_HPAN_MV.Dir == DIR_TO_RIGHT   && DI_HPAN_MV.Offset == 1 ) &&
		( MCNR_HPAN_MV.Dir == DIR_TO_RIGHT && MCNR_HPAN_MV.Offset == 1 )  )
	{
		Score = 7999;
		Score = ( PreScore*15 + Score ) >> 4;
		MV_Direction = 2;
	}
	else if( DynamicOptimizeSystem[7] == 1 &&
		( DI_HPAN_MV.Dir == DIR_TO_RIGHT   && DI_HPAN_MV.Offset == 1 ) &&
		( MCNR_HPAN_MV.Dir == DIR_TO_RIGHT && MCNR_HPAN_MV.Offset == 2 )  )
	{
		Score = 10999;
		Score = ( PreScore*15 + Score ) >> 4;
		MV_Direction = 2;
	}
	else if( DynamicOptimizeSystem[7] == 1 &&
		( DI_HPAN_MV.Dir == DIR_TO_RIGHT   && DI_HPAN_MV.Offset == 2 ) &&
		( MCNR_HPAN_MV.Dir == DIR_TO_RIGHT && MCNR_HPAN_MV.Offset == 1 )  )
	{
		Score = 10999;
		Score = ( PreScore*15 + Score ) >> 4;
		MV_Direction = 2;
	}
	else if( DynamicOptimizeSystem[7] == 1 &&
		( DI_HPAN_MV.Dir == DIR_TO_RIGHT   && DI_HPAN_MV.Offset == 2 ) &&
		( MCNR_HPAN_MV.Dir == DIR_TO_RIGHT && MCNR_HPAN_MV.Offset == 2 )  )
	{
		Score = 3999;
		Score = ( PreScore*15 + Score ) >> 4;
		MV_Direction = 2;
	}
	else
	{
		Score = 0;
		Score = ( PreScore + Score*15 ) >> 4;
		//MV_Direction = 0;
	}

	PreScore = Score;

	if( Score <= 1000 )
		MV_Direction = 0;

	ScalerVideo_Status.DynamicREG.Directional3D_bld_ratio = Score/1000;
	ScalerVideo_Status.DynamicREG.Directional3D_dir = MV_Direction;


	ROS_VideoPrintf_DynamicCtrl("Score = %d, Bld ratio = %d, Dir = %d\n",
			Score, ScalerVideo_Status.DynamicREG.Directional3D_bld_ratio, ScalerVideo_Status.DynamicREG.Directional3D_dir);

	return 0;

}

void scalerVideo_UpdateDebugInfo(void)
{
    // Video Decoder DEBUG function register introduction
    // dummy register 8(9ffc)
    // 0~3   : Reserved
    // 4~5   : [log] delay time = value * 32
	// 6     : Reserved
    // 7     : Enable experiment VD setting
    // 8     : [log] enable the log
    // 9     : [log] VD general status
    // 10    : Reserved
    // 11    : only reset enable, for observing the reset value only
    // 12    : [log] Dynamic control register function log enable
    // 13    : Reserved
    // 14    : auto disable, disable ScalerVideo_ApplyVDSetting API
    // 15    : clear scan set record
    // 16    : disable VD Apply buffer
    // 17    : [log] special flag
    // 18    : [log] special flag related parameters
    // 19    : [log] SMD/HUE/SAT vector
    // 21    : [RESET FLAG] Used for handshake with AP to save the VD register initial value after loading table
    // 22    : [Disable set] disable specific VD set enable, will disable all the set in the specific set vector
    // 23    : [Disable set] set the set disable specific VD set number to the disable specific set vector, 255 will clear the buffer
    // 31~24 : [Disable set] disable specific VD apply set

    // dummy register 7(9ff8)
    // 9~0   : debug value 1 for debug
    // 19~10 : debug value 2 for debug (19~0 = debug value 3 for debug)
    // 22~20 : RESERVED
    // 23	 : print table control register reset value
    // 25~24 : 1D, 2D, 3D
    // 31~26 : table set number from kernel

    // for debug information
	vdpq_vdpq_fw_used1_RBUS vdpq_vdpq_fw_used1_reg; // dummy 7
	vdpq_vdpq_fw_used2_RBUS vdpq_vdpq_fw_used2_reg; // dummy 8

    static bool InitialDebugValueSet = FALSE;
    int i = 0;
    bool FoundFlag = FALSE;

	vdpq_vdpq_fw_used1_reg.regValue = VDPQ_rtd_inl(VDPQ_VDPQ_FW_USED1_reg);
	vdpq_vdpq_fw_used2_reg.regValue = VDPQ_rtd_inl(VDPQ_VDPQ_FW_USED2_reg);


    // initialize some debug dummy register value
	if( scalerVideo_BitWiseOperation(vdpq_vdpq_fw_used2_reg.regValue, 21, 21, 0, VD_BIT_GET) != 0 )
    {
        InitialDebugValueSet = FALSE; // when change source, rewrite the default value
    }
    if( InitialDebugValueSet != TRUE )
    {
		vdpq_vdpq_fw_used2_reg.regValue = scalerVideo_BitWiseOperation(vdpq_vdpq_fw_used2_reg.regValue, 4 , 5 , 3  , VD_BIT_SET);
		vdpq_vdpq_fw_used1_reg.regValue = scalerVideo_BitWiseOperation(vdpq_vdpq_fw_used1_reg.regValue, 0 , 9 , 650, VD_BIT_SET);
		vdpq_vdpq_fw_used1_reg.regValue = scalerVideo_BitWiseOperation(vdpq_vdpq_fw_used1_reg.regValue, 10, 19, 800, VD_BIT_SET);

		for(i=0; i<VD_SET_NUM; i++)
			ScalerVideo_Debug.DisableSpecificSetBuffer[i] = 0;
		ScalerVideo_Debug.DisableSpecificSetBufferIdx = 0;
        InitialDebugValueSet = TRUE;
    }

    // read debug information
	ScalerVideo_Debug.DebugValue_1 = scalerVideo_BitWiseOperation(vdpq_vdpq_fw_used1_reg.regValue, 0 , 9 , 0, VD_BIT_GET);
	ScalerVideo_Debug.DebugValue_2 = scalerVideo_BitWiseOperation(vdpq_vdpq_fw_used1_reg.regValue, 10, 19, 0, VD_BIT_GET);
	ScalerVideo_Debug.DebugValue_3 = scalerVideo_BitWiseOperation(vdpq_vdpq_fw_used1_reg.regValue, 0 , 19, 0, VD_BIT_GET);
	ScalerVideo_Debug.PrintTime					= ((scalerVideo_BitWiseOperation(vdpq_vdpq_fw_used2_reg.regValue, 4, 5, 0, VD_BIT_GET) * 32 + 15) | 0x1);
	ScalerVideo_Debug.ExperimentSettingEnable	= scalerVideo_BitWiseOperation(vdpq_vdpq_fw_used2_reg.regValue, 7 , 7 , 0, VD_BIT_GET);
	ScalerVideo_Debug.OnlyReset					= scalerVideo_BitWiseOperation(vdpq_vdpq_fw_used2_reg.regValue, 11, 11, 0, VD_BIT_GET);
	ScalerVideo_Debug.AutoDisable				= scalerVideo_BitWiseOperation(vdpq_vdpq_fw_used2_reg.regValue, 14, 14, 0, VD_BIT_GET);
	ScalerVideo_Debug.AutoDisable_ApplyBuffer	= scalerVideo_BitWiseOperation(vdpq_vdpq_fw_used2_reg.regValue, 16, 16, 0, VD_BIT_GET);
	ScalerVideo_Debug.DisableSpecificSetNum		= scalerVideo_BitWiseOperation(vdpq_vdpq_fw_used2_reg.regValue, 24, 31, 0, VD_BIT_GET);
	ScalerVideo_Debug.DisableSpecificSetEnable	= scalerVideo_BitWiseOperation(vdpq_vdpq_fw_used2_reg.regValue, 22, 22, 0, VD_BIT_GET);
	if( scalerVideo_BitWiseOperation(vdpq_vdpq_fw_used2_reg.regValue, 23, 23, 0, VD_BIT_GET) != 0 )
    {
        if( ScalerVideo_Debug.DisableSpecificSetNum == 255 )
            ScalerVideo_Debug.DisableSpecificSetBufferIdx = 0; // reset the disable specific set buffer
        else
        {

            for(i=0; i<ScalerVideo_Debug.DisableSpecificSetBufferIdx; i++)
            {
                if(ScalerVideo_Debug.DisableSpecificSetBuffer[i]==ScalerVideo_Debug.DisableSpecificSetNum)
                {
                    // move the buffer
                    int j=i;
                    for( j=i;j<ScalerVideo_Debug.DisableSpecificSetBufferIdx;j++)
                    {
                        ScalerVideo_Debug.DisableSpecificSetBuffer[j] = ScalerVideo_Debug.DisableSpecificSetBuffer[j+1];
                    }
                    ScalerVideo_Debug.DisableSpecificSetBufferIdx--;
                    FoundFlag = TRUE;
                    break;
                }
            }
            if( FoundFlag == FALSE && (unsigned int)ScalerVideo_Debug.DisableSpecificSetBufferIdx < VD_SET_NUM )
            {
                ScalerVideo_Debug.DisableSpecificSetBuffer[ScalerVideo_Debug.DisableSpecificSetBufferIdx] = ScalerVideo_Debug.DisableSpecificSetNum;
                ScalerVideo_Debug.DisableSpecificSetBufferIdx++;
            }
        }
		vdpq_vdpq_fw_used2_reg.regValue = scalerVideo_BitWiseOperation(vdpq_vdpq_fw_used2_reg.regValue, 23, 23, 0, VD_BIT_SET);
    }


	{
		// -------------------------------------------------------------------------------------------------------------------
		/* General status log */
		ROS_VideoPrintf_General("oo-----[20171011 VDPQ General Status]-----oo\n");
		ROS_VideoPrintf_General("[Status] Kernel TBL  = VDC_%dD_SET%d\n",
			scalerVideo_BitWiseOperation(vdpq_vdpq_fw_used1_reg.regValue, 24, 25, 0, VD_BIT_GET),
			scalerVideo_BitWiseOperation(vdpq_vdpq_fw_used1_reg.regValue, 26, 31, 0, VD_BIT_GET));
        ROS_VideoPrintf_General("[Status] VD_mode     = %d\n", ScalerVideo_Status.VD_Mode );
        ROS_VideoPrintf_General("[Status] Source      = %d\n", ScalerVideo_Status.RPC_SystemInfoTable->VIP_source );
        ROS_VideoPrintf_General("[Status] TV(1)/AV(0) = %d\n", ScalerVideo_Status.src_is_tv );
        ROS_VideoPrintf_General("[Status] YCSep(reg)  = %d\n", ScalerVideo_Status.YCSepModeFromReg );
        ROS_VideoPrintf_General("[Status] YCSep(sys)  = %d\n", ScalerVideo_Status.YCSepModeFromSystem );
        ROS_VideoPrintf_General("------------------------------\n" );
        ROS_VideoPrintf_General("[Noise] Noise_Density  = %d\n", ScalerVideo_Status.Noise_Density);
        ROS_VideoPrintf_General("[Noise] Noise_MeanNxV  = %d\n", ScalerVideo_Status.Noise_meanNxV);
        ROS_VideoPrintf_General("[Noise] Noise_Vmm      = %d\n", ScalerVideo_Status.Noise_Vmm);
        ROS_VideoPrintf_General("[Noise] Noise          = %d\n", ScalerVideo_Status.NoiseStatus);
        ROS_VideoPrintf_General("[Noise] VIP Noise      = %d\n", scalerVIP_Get_Noise_Level());
        ROS_VideoPrintf_General("[Noise][DI] Noise(S/T) = %d / %d\n",
            scalerVIP_DI_MiddleWare_GetStatus(STATUS_CONTENT_SPATIAL_NOISE_LEVEL),
            scalerVIP_DI_MiddleWare_GetStatus(STATUS_CONTENT_TEMPORAL_NOISE_LEVEL));
		ROS_VideoPrintf_General("------------------------------\n" );
		ROS_VideoPrintf_General("[Status] Motion      = %d\n", ScalerVideo_Status.MotionStatus );
		ROS_VideoPrintf_General("[Status] FreqOffset  = %d\n", ScalerVideo_Status.FrequencyOffset);
		ROS_VideoPrintf_General("[Status] RTNR_MAD_count_Y2_avg_ratio = %d\n", SmartPic_clue->RTNR_MAD_count_Y2_avg_ratio);
		ROS_VideoPrintf_General("------------------------------\n" );
		// -------------------------------------------------------------------------------------------------------------------

		// -------------------------------------------------------------------------------------------------------------------
		/* Applying setting log */
		ROS_VideoPrintf_Setting("[DisSet] Disable set = %d\n", ScalerVideo_Debug.DisableSpecificSetNum);
        ROS_VideoPrintf_Setting("[DisSet] BufferIdx   = %d\n", ScalerVideo_Debug.DisableSpecificSetBufferIdx);
        ROS_VideoPrintf_Setting("[DisSet] Buffer = ");
        for(i=0;i<ScalerVideo_Debug.DisableSpecificSetBufferIdx;i++)
        {
            ROS_VideoPrintf_Setting("%d,", ScalerVideo_Debug.DisableSpecificSetBuffer[i]);
        }
        ROS_VideoPrintf_Setting("\n------------------------------\n");
		// -------------------------------------------------------------------------------------------------------------------

		// -------------------------------------------------------------------------------------------------------------------
		/* Special flag and related parameters */
        ROS_VideoPrintf_SpecialFlag("[Flag] MovingCan        = %d\n", ScalerVideo_Status.Flag_MovingCan);
        ROS_VideoPrintf_SpecialFlag("[Flag] DOS_86_91        = %d\n", ScalerVideo_Status.Flag_DOS_86_91);
        ROS_VideoPrintf_SpecialFlag("[Flag] DOS_44           = %d\n", ScalerVideo_Status.Flag_DOS_44);
        ROS_VideoPrintf_SpecialFlag("[Flag] DiaEdge Stable   = %d\n", ScalerVideo_Status.Flag_DiagonalEdgeStable);
        if( ScalerVideo_Status.RPC_SystemInfoTable->VIP_source == VIP_QUALITY_TV_NTSC ||
            ScalerVideo_Status.RPC_SystemInfoTable->VIP_source == VIP_QUALITY_CVBS_NTSC )
          ROS_VideoPrintf_SpecialFlag("[Flag] SunFlowerRoof(N) = %d\n", ScalerVideo_Status.Flag_SunFlowerRoof);
        if( ScalerVideo_Status.RPC_SystemInfoTable->VIP_source == VIP_QUALITY_TV_PAL ||
            ScalerVideo_Status.RPC_SystemInfoTable->VIP_source == VIP_QUALITY_CVBS_PAL )
        {
          ROS_VideoPrintf_SpecialFlag("[Flag] DOS_194           = %d\n", ScalerVideo_Status.Flag_DOS_194);
          ROS_VideoPrintf_SpecialFlag("[Flag] SunFlowerRoof(P)  = %d\n", ScalerVideo_Status.Flag_PAL_SunFlowerRoof);
          ROS_VideoPrintf_SpecialFlag("[Flag] PAL_DeXC          = %d\n", ScalerVideo_Status.Flag_PAL_DeXC);
        }
        ROS_VideoPrintf_SpecialFlag("[Flag] SMD              = %d\n", ScalerVideo_Status.Flag_SMD);
        ROS_VideoPrintf_SpecialFlag("[Flag] HighSat_06_31    = %d\n", ScalerVideo_Status.HighSatFlag_Bin_06_31);
        ROS_VideoPrintf_SpecialFlag("[Flag] HighSat_14_31    = %d\n", ScalerVideo_Status.HighSatFlag_Bin_14_31);
        ROS_VideoPrintf_SpecialFlag("[Flag] VD Strong SMD    = %d\n", ScalerVideo_Status.Flag_VD_SMD);
        ROS_VideoPrintf_SpecialFlag("------------------------------\n" );
        ROS_VideoPrintf_SpecialFlag("[Value] Motion              = %d\n", ScalerVideo_Status.MotionStatus );
        ROS_VideoPrintf_SpecialFlag("[Value][DI] FMV LRDiff      = %d\n", ScalerVideo_Status.FMV_LRDiff);
        ROS_VideoPrintf_SpecialFlag("[Value][DI] VERTICAL_PAN    = %d\n", scalerVIP_DI_MiddleWare_GetStatus(STATUS_VPAN));
        ROS_VideoPrintf_SpecialFlag("[Value][DI] HORIZONTAL_PAN  = %d\n", scalerVIP_DI_MiddleWare_GetStatus(STATUS_PAN));
        ROS_VideoPrintf_SpecialFlag("[Value][DI] HIFreq_Level    = %d\n", scalerVIP_DI_MiddleWare_GetStatus(STATUS_HIGHFREQUENCY_LEVEL));
        ROS_VideoPrintf_SpecialFlag("[Value][DI] Motion          = %d\n", scalerVIP_DI_MiddleWare_GetStatus(STATUS_MOTION));
        ROS_VideoPrintf_SpecialFlag("[Value] Hue_Detector_%d_%d  = %d\n",
          ScalerVideo_Debug.DebugValue_1,
          ScalerVideo_Debug.DebugValue_2,
          scalerVIP_Diverse_Hue_Detector(ScalerVideo_Debug.DebugValue_1, ScalerVideo_Debug.DebugValue_2));
        ROS_VideoPrintf_SpecialFlag("[Value] HUE_RatioSum_%d_%d   = %d\n",
          ScalerVideo_Debug.DebugValue_1,
          ScalerVideo_Debug.DebugValue_2,
          scalerVideo_HUE_BinRatioSum(ScalerVideo_Debug.DebugValue_1, ScalerVideo_Debug.DebugValue_2));
        ROS_VideoPrintf_SpecialFlag("[Value] SMD_RatioSum_%d_%d   = %d\n",
          ScalerVideo_Debug.DebugValue_1,
          ScalerVideo_Debug.DebugValue_2,
          scalerVideo_SMD_BinRatioSum(ScalerVideo_Debug.DebugValue_1, ScalerVideo_Debug.DebugValue_2));
        ROS_VideoPrintf_SpecialFlag("[Value] Sat_RatioSum_%d_%d   = %d\n",
          ScalerVideo_Debug.DebugValue_1,
          ScalerVideo_Debug.DebugValue_2,
          scalerVideo_SAT_BinRatioSum(ScalerVideo_Debug.DebugValue_1, ScalerVideo_Debug.DebugValue_2));
		ROS_VideoPrintf_SpecialFlag("------------------------------\n");
		// -------------------------------------------------------------------------------------------------------------------

		// -------------------------------------------------------------------------------------------------------------------
		/* Vector bin ration information */
        ROS_VideoPrintf_Vector("------------------------------ SMD\n");
        for(i=0; i<DI_SMD_BIN_NUM;i++)
        {
            ROS_VideoPrintf_Vector("%d = %d\t", i, ScalerVideo_Status.SMD_BinRatio[i]);
            if((i+1)%4==0)
                ROS_VideoPrintf_Vector("\n");
        }
        ROS_VideoPrintf_Vector("\n------------------------------ HUE Sum = %d\n", ScalerVideo_Status.Hue_Sum);
        for(i=0; i<COLOR_HUE_HISTOGRAM_LEVEL;i++)
        {
            ROS_VideoPrintf_Vector("%d = %d\t", i, ScalerVideo_Status.Hue_HistogramRatio[i]);
            if((i+1)%4==0)
                ROS_VideoPrintf_Vector("\n");
        }

        ROS_VideoPrintf_Vector("\n------------------------------ SATURATION Histogram\n" );
        for(i=0; i<COLOR_AutoSat_HISTOGRAM_LEVEL;i++)
        {
            ROS_VideoPrintf_Vector("%d = %d\t", i, sat_hist[i]);
            if((i+1)%4==0)
                ROS_VideoPrintf_Vector("\n");
        }
        ROS_VideoPrintf_Vector("\n------------------------------ SATURATION Ratio\n" );
        for(i=0; i<COLOR_AutoSat_HISTOGRAM_LEVEL;i++)
        {
            ROS_VideoPrintf_Vector("%d = %d\t", i, ScalerVideo_Status.Sat_HistogramRatio[i]);
            if((i+1)%4==0)
                ROS_VideoPrintf_Vector("\n");
        }
		// -------------------------------------------------------------------------------------------------------------------

    }

    // write back register value
    VDPQ_rtd_outl(VDPQ_VDPQ_FW_USED2_reg, vdpq_vdpq_fw_used2_reg.regValue);
    VDPQ_rtd_outl(VDPQ_VDPQ_FW_USED1_reg, vdpq_vdpq_fw_used1_reg.regValue);

}
unsigned char scalerVideo_ApplyVDSetting(unsigned int SetNumber)
{
    // for all auto tuning registers, revert to normal setting first
    static vdpq_yc_bw_ctrl_RBUS ORI_vdpq_yc_bw_ctrl_reg;
    static vdpq_bpf_bw_sel_y_RBUS ORI_vdpq_bpf_bw_sel_y_reg;
    static vdpq_bpf_bw_sel_c_RBUS ORI_vdpq_bpf_bw_sel_c_reg;
    static vdpq_adap_bpf_c_th1_RBUS ORI_vdpq_adap_bpf_c_th1_reg;
    static vdpq_alpha_2d_mod_enable_RBUS ORI_vdpq_alpha_2d_mod_enable_reg;
    static vdpq_yc_3d_f4_pair_ctrl_4_RBUS ORI_vdpq_yc_3d_f4_pair_ctrl_4_reg;
    static vdpq_yc_3d_f4_still_vector_1_RBUS ORI_vdpq_yc_3d_f4_still_vector_1_reg;
    static vdpq_yc_3d_f4_still_vector_2_RBUS ORI_vdpq_yc_3d_f4_still_vector_2_reg;
    static vdpq_yc_3d_f4_pair_ctrl_1_RBUS ORI_vdpq_yc_3d_f4_pair_ctrl_1_reg;
    static vdpq_h2v_y_noise_thr_RBUS ORI_vdpq_h2v_y_noise_thr_reg;
    static vdpq_yc_3d_f4_still_gray_hifreq_1_RBUS ORI_vdpq_yc_3d_f4_still_gray_hifreq_1_reg;
    static vdpq_yc_3d_f4_still_gray_hifreq_2_RBUS ORI_vdpq_yc_3d_f4_still_gray_hifreq_2_reg;
    static vdpq_postp_mf_compensation_ctrl1_RBUS ORI_vdpq_postp_mf_compensation_ctrl1_reg;
    static vdpq_yc_3d_f4_pair_ctrl_6_RBUS ORI_vdpq_yc_3d_f4_pair_ctrl_6_reg;
    static vdpq_yc_3d_f4_pair_ctrl_3_RBUS ORI_vdpq_yc_3d_f4_pair_ctrl_3_reg;
    static vdpq_yc2d_postlp_ctrl_RBUS ORI_vdpq_yc2d_postlp_ctrl_reg;
    static vdpq_yc_3d_f4_pair_ctrl_5_RBUS ORI_vdpq_yc_3d_f4_pair_ctrl_5_reg;
    static vdpq_adap_bpf_y_th3_RBUS ORI_vdpq_adap_bpf_y_th3_reg;
    static vdpq_yc2d_postlp_grad_1_RBUS ORI_vdpq_yc2d_postlp_grad_1_reg;
    static vdpq_comb_filter_threshold1_RBUS ORI_vdpq_comb_filter_threshold1_reg;
    static vdpq_cdiff_adap_bpf_th2_RBUS ORI_vdpq_cdiff_adap_bpf_th2_reg;

    static vdpq_bypass_bpf_ctrl1_RBUS ORI_vdpq_bypass_bpf_ctrl1_reg;
    static vdpq_bypass_bpf_ctrl2_RBUS ORI_vdpq_bypass_bpf_ctrl2_reg;

	// PAL Error
	static vdpq_yc_3d_f4_pal_err_com_1_RBUS ORI_vdpq_yc_3d_f4_pal_err_com_1_reg;
	static vdpq_yc_3d_f4_pal_err_com_2_RBUS ORI_vdpq_yc_3d_f4_pal_err_com_2_reg;
	static vdpq_yc_3d_f4_pal_err_com_3_RBUS ORI_vdpq_yc_3d_f4_pal_err_com_3_reg;

    static vdpq_yc_3d_f4_dc_compare_RBUS ORI_vdpq_yc_3d_f4_dc_compare_reg;
    static vdpq_yc_3d_f4_global_diff_th_hifreq_offset_3_RBUS ORI_vdpq_yc_3d_f4_global_diff_th_hifreq_offset_3_reg;
    static vdpq_yc_3d_f4_global_diff_th_hifreq_offset_4_RBUS ORI_vdpq_yc_3d_f4_global_diff_th_hifreq_offset_4_reg;
    static vdpq_blend_1d_thr_RBUS ORI_vdpq_blend_1d_thr_reg;
    static vdpq_yc2d_postlp_flat_1_RBUS ORI_vdpq_yc2d_postlp_flat_1_reg;
    static vdpq_yc2d_postlp_flat_2_RBUS ORI_vdpq_yc2d_postlp_flat_2_reg;
    static vdpq_yc2d_postlp_flat_3_RBUS ORI_vdpq_yc2d_postlp_flat_3_reg;
    static vdpq_h2v_blend_ratio_RBUS ORI_vdpq_h2v_blend_ratio_reg;
	// Directional 3D
	static vdpq_directional_3d_RBUS ORI_vdpq_directional_3d_reg;
	// 2D 3D Alpha
	static vdpq_yc_3d_f4_spa_cnt_cvbs_amp_RBUS ORI_vdpq_yc_3d_f4_spa_cnt_cvbs_amp_reg;
	static vdpq_alpha_blending_ctrl1_RBUS ORI_vdpq_alpha_blending_ctrl1_reg;
	static vdpq_alpha_blending_ctrl2_RBUS ORI_vdpq_alpha_blending_ctrl2_reg;
	static vdpq_alpha_blending_ctrl3_RBUS ORI_vdpq_alpha_blending_ctrl3_reg;
	static vdpq_alpha_blending_ctrl4_RBUS ORI_vdpq_alpha_blending_ctrl4_reg;
	// For fix 3D
	static vdpq_yc_sep_mode_ctrl_RBUS ORI_vdpq_yc_sep_mode_ctrl_reg;
    // XNR
    static vdpq_xnr_ctrl_RBUS ORI_vdpq_xnr_ctrl_reg;
    static vdpq_xnr_thr_RBUS ORI_vdpq_xnr_thr_reg;
    // MVLine
    static vdpq_yc_3d_f4_mghf_ctrl_1_RBUS ORI_vdpq_yc_3d_f4_mghf_ctrl_1_reg;
    // V2V
    static vdpq_h2v_noise_y_add_dc_RBUS ORI_vdpq_h2v_noise_y_add_dc_reg;
    // VMC
    static vdpq_vmc_ctrl_RBUS ORI_vdpq_vmc_ctrl_reg;
    static vdpq_diamc_cvbs_amp_th_RBUS ORI_vdpq_diamc_cvbs_amp_th_reg;
    // Post2DBPF DBWline
    static vdpq_yc_post2dbpf_dbwline_1_RBUS ORI_vdpq_yc_post2dbpf_dbwline_1_reg;
    // post process
    static vdpq_postp_3d2dalpha_blend_ctrl_1_RBUS ORI_vdpq_postp_3d2dalpha_blend_ctrl_1_reg;
    static vdpq_postp_3d2dalpha_blend_ctrl_2_RBUS ORI_vdpq_postp_3d2dalpha_blend_ctrl_2_reg;
    static vdpq_de_xc_ctrl_RBUS ORI_vdpq_de_xc_ctrl_reg;
    // post 2d bpf y
    static vdpq_yc_post2dbpf_ctrl_RBUS ORI_vdpq_yc_post2dbpf_ctrl_reg;
    static vdpq_yc_post2dbpf_thbase_RBUS ORI_vdpq_yc_post2dbpf_thbase_reg;
    static vdpq_yc_post2dbpf_adapth_1_RBUS ORI_vdpq_yc_post2dbpf_adapth_1_reg;
    static vdpq_yc_post2dbpf_adapth_13_RBUS ORI_vdpq_yc_post2dbpf_adapth_13_reg;
    // SMD
    static vdpq_yc_3d_f4_smd_en_RBUS ORI_vdpq_yc_3d_f4_smd_en_reg;
    static vdpq_yc_3d_f4_smd_th1_RBUS ORI_vdpq_yc_3d_f4_smd_th1_reg;
    static vdpq_yc_3d_f4_smd_th2_RBUS ORI_vdpq_yc_3d_f4_smd_th2_reg;
    static vdpq_yc_3d_f4_pair_smd_th1_RBUS ORI_vdpq_yc_3d_f4_pair_smd_th1_reg;
	// HV edge
	static vdpq_yc_3d_f4_hv_edge_1_RBUS ORI_vdpq_yc_3d_f4_hv_edge_1_reg;
	// VD CTI
	static vdpq_cadap_gain_ctrl_RBUS ORI_vdpq_cadap_gain_ctrl_reg;
	static vdpq_cadap_lut_gain_th_RBUS ORI_vdpq_cadap_lut_gain_th_reg;
    // non-VD registers
    static di_im_di_tnr_xc_control_RBUS ORI_di_im_di_tnr_xc_control_reg;
    static di_im_di_control_RBUS ORI_di_im_di_control_reg;


    // The registers that will be controled by auto adjust flow
    static vdpq_yc_bw_ctrl_RBUS vdpq_yc_bw_ctrl_reg;
    static vdpq_bpf_bw_sel_y_RBUS vdpq_bpf_bw_sel_y_reg;
    static vdpq_bpf_bw_sel_c_RBUS vdpq_bpf_bw_sel_c_reg;
    static vdpq_adap_bpf_c_th1_RBUS vdpq_adap_bpf_c_th1_reg;
    static vdpq_alpha_2d_mod_enable_RBUS vdpq_alpha_2d_mod_enable_reg;
    static vdpq_yc_3d_f4_pair_ctrl_4_RBUS vdpq_yc_3d_f4_pair_ctrl_4_reg;
    static vdpq_yc_3d_f4_still_vector_1_RBUS vdpq_yc_3d_f4_still_vector_1_reg;
    static vdpq_yc_3d_f4_still_vector_2_RBUS vdpq_yc_3d_f4_still_vector_2_reg;
    static vdpq_yc_3d_f4_pair_ctrl_1_RBUS vdpq_yc_3d_f4_pair_ctrl_1_reg;
    static vdpq_h2v_y_noise_thr_RBUS vdpq_h2v_y_noise_thr_reg;
    static vdpq_yc_3d_f4_still_gray_hifreq_1_RBUS vdpq_yc_3d_f4_still_gray_hifreq_1_reg;
    static vdpq_yc_3d_f4_still_gray_hifreq_2_RBUS vdpq_yc_3d_f4_still_gray_hifreq_2_reg;
    static vdpq_postp_mf_compensation_ctrl1_RBUS vdpq_postp_mf_compensation_ctrl1_reg;
    static vdpq_yc_3d_f4_pair_ctrl_6_RBUS vdpq_yc_3d_f4_pair_ctrl_6_reg;
    static vdpq_yc_3d_f4_pair_ctrl_3_RBUS vdpq_yc_3d_f4_pair_ctrl_3_reg;
    static vdpq_yc2d_postlp_ctrl_RBUS vdpq_yc2d_postlp_ctrl_reg;
    static vdpq_yc_3d_f4_pair_ctrl_5_RBUS vdpq_yc_3d_f4_pair_ctrl_5_reg;
    static vdpq_adap_bpf_y_th3_RBUS vdpq_adap_bpf_y_th3_reg;
    static vdpq_yc2d_postlp_grad_1_RBUS vdpq_yc2d_postlp_grad_1_reg;
    static vdpq_comb_filter_threshold1_RBUS vdpq_comb_filter_threshold1_reg;
    static vdpq_cdiff_adap_bpf_th2_RBUS vdpq_cdiff_adap_bpf_th2_reg;
    static vdpq_bypass_bpf_ctrl1_RBUS vdpq_bypass_bpf_ctrl1_reg;
    static vdpq_bypass_bpf_ctrl2_RBUS vdpq_bypass_bpf_ctrl2_reg;

	// PAL Error
	static vdpq_yc_3d_f4_pal_err_com_1_RBUS vdpq_yc_3d_f4_pal_err_com_1_reg;
	static vdpq_yc_3d_f4_pal_err_com_2_RBUS vdpq_yc_3d_f4_pal_err_com_2_reg;
	static vdpq_yc_3d_f4_pal_err_com_3_RBUS vdpq_yc_3d_f4_pal_err_com_3_reg;

    static vdpq_yc_3d_f4_dc_compare_RBUS vdpq_yc_3d_f4_dc_compare_reg;
    static vdpq_yc_3d_f4_global_diff_th_hifreq_offset_3_RBUS vdpq_yc_3d_f4_global_diff_th_hifreq_offset_3_reg;
    static vdpq_yc_3d_f4_global_diff_th_hifreq_offset_4_RBUS vdpq_yc_3d_f4_global_diff_th_hifreq_offset_4_reg;
    static vdpq_blend_1d_thr_RBUS vdpq_blend_1d_thr_reg;
    static vdpq_yc2d_postlp_flat_1_RBUS vdpq_yc2d_postlp_flat_1_reg;
    static vdpq_yc2d_postlp_flat_2_RBUS vdpq_yc2d_postlp_flat_2_reg;
    static vdpq_yc2d_postlp_flat_3_RBUS vdpq_yc2d_postlp_flat_3_reg;
    static vdpq_h2v_blend_ratio_RBUS vdpq_h2v_blend_ratio_reg;
	// Directional 3D
	static vdpq_directional_3d_RBUS vdpq_directional_3d_reg;
	// 2D 3D Alpha
	static vdpq_yc_3d_f4_spa_cnt_cvbs_amp_RBUS vdpq_yc_3d_f4_spa_cnt_cvbs_amp_reg;
	static vdpq_alpha_blending_ctrl1_RBUS vdpq_alpha_blending_ctrl1_reg;
	static vdpq_alpha_blending_ctrl2_RBUS vdpq_alpha_blending_ctrl2_reg;
	static vdpq_alpha_blending_ctrl3_RBUS vdpq_alpha_blending_ctrl3_reg;
	static vdpq_alpha_blending_ctrl4_RBUS vdpq_alpha_blending_ctrl4_reg;
	// For fix 3D
	static vdpq_yc_sep_mode_ctrl_RBUS vdpq_yc_sep_mode_ctrl_reg;
    // XNR
    static vdpq_xnr_ctrl_RBUS vdpq_xnr_ctrl_reg;
    static vdpq_xnr_thr_RBUS vdpq_xnr_thr_reg;
    // MVLine
    static vdpq_yc_3d_f4_mghf_ctrl_1_RBUS vdpq_yc_3d_f4_mghf_ctrl_1_reg;
    // V2V
    static vdpq_h2v_noise_y_add_dc_RBUS vdpq_h2v_noise_y_add_dc_reg;
    // VMC
    static vdpq_vmc_ctrl_RBUS vdpq_vmc_ctrl_reg;
    static vdpq_diamc_cvbs_amp_th_RBUS vdpq_diamc_cvbs_amp_th_reg;
    // Post2DBPF DBWline
    static vdpq_yc_post2dbpf_dbwline_1_RBUS vdpq_yc_post2dbpf_dbwline_1_reg;
    // post process
    static vdpq_postp_3d2dalpha_blend_ctrl_1_RBUS vdpq_postp_3d2dalpha_blend_ctrl_1_reg;
    static vdpq_postp_3d2dalpha_blend_ctrl_2_RBUS vdpq_postp_3d2dalpha_blend_ctrl_2_reg;
    static vdpq_de_xc_ctrl_RBUS vdpq_de_xc_ctrl_reg;
    // post 2d bpf y
    static vdpq_yc_post2dbpf_ctrl_RBUS vdpq_yc_post2dbpf_ctrl_reg;
    static vdpq_yc_post2dbpf_thbase_RBUS vdpq_yc_post2dbpf_thbase_reg;
    static vdpq_yc_post2dbpf_adapth_1_RBUS vdpq_yc_post2dbpf_adapth_1_reg;
    static vdpq_yc_post2dbpf_adapth_13_RBUS vdpq_yc_post2dbpf_adapth_13_reg;
    // SMD
    static vdpq_yc_3d_f4_smd_en_RBUS vdpq_yc_3d_f4_smd_en_reg;
    static vdpq_yc_3d_f4_smd_th1_RBUS vdpq_yc_3d_f4_smd_th1_reg;
    static vdpq_yc_3d_f4_smd_th2_RBUS vdpq_yc_3d_f4_smd_th2_reg;
    static vdpq_yc_3d_f4_pair_smd_th1_RBUS vdpq_yc_3d_f4_pair_smd_th1_reg;
	// HV edge
	static vdpq_yc_3d_f4_hv_edge_1_RBUS vdpq_yc_3d_f4_hv_edge_1_reg;
	// VD CTI
	static vdpq_cadap_gain_ctrl_RBUS vdpq_cadap_gain_ctrl_reg;
	static vdpq_cadap_lut_gain_th_RBUS vdpq_cadap_lut_gain_th_reg;
    // non-VD registers
    static di_im_di_tnr_xc_control_RBUS di_im_di_tnr_xc_control_reg;
    static di_im_di_control_RBUS di_im_di_control_reg;
    di_im_di_control_RBUS di_control_reg_Prtoect_3AEnable;


    di_db_reg_ctl_RBUS di_db_reg_ctl_Reg;

    //-------------------------------------------------------------------------------------
    // REGISTER INIT VALUE
    vdpq_vdpq_fw_used2_RBUS vdpq_vdpq_fw_used2_reg;
    vdpq_vdpq_fw_used2_reg.regValue = VDPQ_rtd_inl(VDPQ_VDPQ_FW_USED2_reg);
    if( scalerVideo_BitWiseOperation(vdpq_vdpq_fw_used2_reg.regValue, 21, 21, 0, VD_BIT_GET) != 0 )
    {
        ORI_vdpq_yc_bw_ctrl_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_BW_CTRL_reg);
        ORI_vdpq_bpf_bw_sel_y_reg.regValue = VDPQ_rtd_inl(VDPQ_BPF_BW_SEL_Y_reg);
        ORI_vdpq_bpf_bw_sel_c_reg.regValue = VDPQ_rtd_inl(VDPQ_BPF_BW_SEL_C_reg);
        ORI_vdpq_adap_bpf_c_th1_reg.regValue = VDPQ_rtd_inl(VDPQ_ADAP_BPF_C_TH1_reg);
        ORI_vdpq_alpha_2d_mod_enable_reg.regValue = VDPQ_rtd_inl(VDPQ_ALPHA_2D_MOD_ENABLE_reg);
        ORI_vdpq_yc_3d_f4_pair_ctrl_4_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Pair_Ctrl_4_reg);
        ORI_vdpq_yc_3d_f4_still_vector_1_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Still_Vector_1_reg);
        ORI_vdpq_yc_3d_f4_still_vector_2_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Still_Vector_2_reg);
        ORI_vdpq_yc_3d_f4_pair_ctrl_1_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Pair_Ctrl_1_reg);
        ORI_vdpq_h2v_y_noise_thr_reg.regValue = VDPQ_rtd_inl(VDPQ_H2V_Y_NOISE_THR_reg);
        ORI_vdpq_yc_3d_f4_still_gray_hifreq_1_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Still_Gray_HiFreq_1_reg);
        ORI_vdpq_yc_3d_f4_still_gray_hifreq_2_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Still_Gray_HiFreq_2_reg);
        ORI_vdpq_postp_mf_compensation_ctrl1_reg.regValue = VDPQ_rtd_inl(VDPQ_POSTP_MF_COMPENSATION_CTRL1_reg);
        ORI_vdpq_yc_3d_f4_pair_ctrl_6_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Pair_Ctrl_6_reg);
        ORI_vdpq_yc_3d_f4_pair_ctrl_3_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Pair_Ctrl_3_reg);
        ORI_vdpq_yc2d_postlp_ctrl_reg.regValue = VDPQ_rtd_inl(VDPQ_yc2d_postlp_ctrl_reg);
        ORI_vdpq_yc_3d_f4_pair_ctrl_5_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Pair_Ctrl_5_reg);
        ORI_vdpq_adap_bpf_y_th3_reg.regValue = VDPQ_rtd_inl(VDPQ_ADAP_BPF_Y_TH3_reg);
        ORI_vdpq_yc2d_postlp_grad_1_reg.regValue = VDPQ_rtd_inl(VDPQ_yc2d_postlp_grad_1_reg);
        ORI_vdpq_comb_filter_threshold1_reg.regValue = VDPQ_rtd_inl(VDPQ_COMB_FILTER_THRESHOLD1_reg);
        ORI_vdpq_cdiff_adap_bpf_th2_reg.regValue = VDPQ_rtd_inl(VDPQ_CDIFF_ADAP_BPF_TH2_reg);
        ORI_vdpq_bypass_bpf_ctrl1_reg.regValue = VDPQ_rtd_inl(VDPQ_BYPASS_BPF_CTRL1_reg);
        ORI_vdpq_bypass_bpf_ctrl2_reg.regValue = VDPQ_rtd_inl(VDPQ_BYPASS_BPF_CTRL2_reg);
        ORI_vdpq_yc_3d_f4_spa_cnt_cvbs_amp_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Spa_Cnt_CVBS_Amp_reg);
		ORI_vdpq_alpha_blending_ctrl1_reg.regValue = VDPQ_rtd_inl(VDPQ_ALPHA_BLENDING_CTRL1_reg);
		ORI_vdpq_alpha_blending_ctrl2_reg.regValue = VDPQ_rtd_inl(VDPQ_ALPHA_BLENDING_CTRL2_reg);
		ORI_vdpq_alpha_blending_ctrl3_reg.regValue = VDPQ_rtd_inl(VDPQ_ALPHA_BLENDING_CTRL3_reg);
		ORI_vdpq_alpha_blending_ctrl4_reg.regValue = VDPQ_rtd_inl(VDPQ_ALPHA_BLENDING_CTRL4_reg);
        ORI_vdpq_xnr_ctrl_reg.regValue = VDPQ_rtd_inl(VDPQ_XNR_CTRL_reg);
        ORI_vdpq_xnr_thr_reg.regValue = VDPQ_rtd_inl(VDPQ_XNR_THR_reg);
        ORI_vdpq_yc_3d_f4_mghf_ctrl_1_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_MGHF_Ctrl_1_reg);
        ORI_vdpq_h2v_noise_y_add_dc_reg.regValue = VDPQ_rtd_inl(VDPQ_H2V_NOISE_Y_ADD_DC_reg);
        ORI_vdpq_vmc_ctrl_reg.regValue = VDPQ_rtd_inl(VDPQ_VMC_CTRL_reg);
        ORI_vdpq_diamc_cvbs_amp_th_reg.regValue = VDPQ_rtd_inl(VDPQ_DIAMC_CVBS_AMP_TH_reg);
        ORI_vdpq_yc_post2dbpf_dbwline_1_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_Post2DBPF_DBWline_1_reg);
        ORI_vdpq_postp_3d2dalpha_blend_ctrl_1_reg.regValue = VDPQ_rtd_inl(VDPQ_POSTP_3D2DALPHA_BLEND_CTRL_1_reg);
        ORI_vdpq_postp_3d2dalpha_blend_ctrl_2_reg.regValue = VDPQ_rtd_inl(VDPQ_POSTP_3D2DALPHA_BLEND_CTRL_2_reg);
        ORI_vdpq_de_xc_ctrl_reg.regValue = VDPQ_rtd_inl(VDPQ_DE_XC_CTRL_reg);
        ORI_vdpq_yc_post2dbpf_ctrl_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_Post2DBPF_Ctrl_reg);
        ORI_vdpq_yc_post2dbpf_thbase_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_Post2DBPF_ThBase_reg);
        ORI_vdpq_yc_post2dbpf_adapth_1_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_Post2DBPF_AdapTh_1_reg);
        ORI_vdpq_yc_post2dbpf_adapth_13_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_Post2DBPF_AdapTh_13_reg);
        ORI_vdpq_yc_3d_f4_smd_en_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_SMD_EN_reg);
        ORI_vdpq_yc_3d_f4_smd_th1_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_SMD_TH1_reg);
        ORI_vdpq_yc_3d_f4_smd_th2_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_SMD_TH2_reg);
        ORI_vdpq_yc_3d_f4_pair_smd_th1_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Pair_SMD_TH1_reg);
        ORI_vdpq_yc_3d_f4_pal_err_com_1_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_PAL_Err_Com_1_reg);
        ORI_vdpq_yc_3d_f4_dc_compare_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_DC_Compare_reg);
        ORI_vdpq_yc_3d_f4_pal_err_com_2_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_PAL_Err_Com_2_reg);
        ORI_vdpq_yc_3d_f4_global_diff_th_hifreq_offset_3_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Global_Diff_Th_Hifreq_Offset_3_reg);
        ORI_vdpq_yc_3d_f4_global_diff_th_hifreq_offset_4_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Global_Diff_Th_Hifreq_Offset_4_reg);
        ORI_vdpq_blend_1d_thr_reg.regValue = VDPQ_rtd_inl(VDPQ_BLEND_1D_THR_reg);
        ORI_vdpq_yc2d_postlp_flat_1_reg.regValue = VDPQ_rtd_inl(VDPQ_yc2d_postlp_flat_1_reg);
        ORI_vdpq_yc2d_postlp_flat_2_reg.regValue = VDPQ_rtd_inl(VDPQ_yc2d_postlp_flat_2_reg);
        ORI_vdpq_yc2d_postlp_flat_3_reg.regValue = VDPQ_rtd_inl(VDPQ_yc2d_postlp_flat_3_reg);
        ORI_vdpq_yc_3d_f4_hv_edge_1_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_HV_Edge_1_reg);
        ORI_vdpq_h2v_blend_ratio_reg.regValue = VDPQ_rtd_inl(VDPQ_H2V_BLEND_RATIO_reg);
		ORI_vdpq_directional_3d_reg.regValue = VDPQ_rtd_inl(VDPQ_DIRECTIONAL_3D_reg);
		ORI_vdpq_yc_sep_mode_ctrl_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_SEP_MODE_CTRL_reg);
		ORI_vdpq_yc_3d_f4_pal_err_com_3_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_PAL_Err_Com_3_reg);
		ORI_vdpq_cadap_gain_ctrl_reg.regValue = VDPQ_rtd_inl(VDPQ_CADAP_GAIN_CTRL_reg);
		ORI_vdpq_cadap_lut_gain_th_reg.regValue = VDPQ_rtd_inl(VDPQ_CADAP_LUT_GAIN_TH_reg);
        // non-VD register
        ORI_di_im_di_tnr_xc_control_reg.regValue = rtd_inl(DI_IM_DI_TNR_XC_CONTROL_reg);
        ORI_di_im_di_control_reg.regValue = rtd_inl(DI_IM_DI_CONTROL_reg);
        return TRUE;
    }
    //-------------------------------------------------------------------------------------



    if( ScalerVideo_Debug.OnlyReset == TRUE && SetNumber != VDPQ_FUNC_RST_ALL && SetNumber != VDPQ_FUNC_WRITE_ADAPTIVE_REG_VAL )
        return FALSE;

    if( ScalerVideo_Status.ApplyEnable == FALSE )
    {
      scalerVideo_AddSetToBuffer(SetNumber, VD_APPLY_SET);
      return TRUE;
    }

    if( ScalerVideo_Debug.AutoDisable == TRUE )
        return FALSE;

    // For disable specific VD setting
    if( ScalerVideo_Debug.DisableSpecificSetEnable == TRUE )
    {
        unsigned int i=0;
        for(i=0;i<ScalerVideo_Debug.DisableSpecificSetBufferIdx;i++)
            if( ScalerVideo_Debug.DisableSpecificSetBuffer[i] == SetNumber )
              return FALSE;
    }

    // Debug log
    {
        if( (SetNumber >= FUNCTIONAL_SETTING_STA) && (SetNumber <= FUNCTIONAL_SETTING_END) )
        { ROS_VideoPrintf_Setting("[VDPQ] Apply %d = (FUNCTIONAL) %d \n", SetNumber, SetNumber - FUNCTIONAL_SETTING_STA ); }
        else if( (SetNumber >= NON_VD_REG_SETTING_STA) && (SetNumber <= NON_VD_REG_SETTING_END) )
        { ROS_VideoPrintf_Setting("[VDPQ] Apply %d = (NON VD REG) %d \n", SetNumber, SetNumber - NON_VD_REG_SETTING_STA ); }
        else if( (SetNumber >= VIP_DYNAMIC_OPTIMIZE_SYSTEM_SETTING_STA) && (SetNumber <= VIP_DYNAMIC_OPTIMIZE_SYSTEM_SETTING_END) )
        { ROS_VideoPrintf_Setting("[VDPQ] Apply %d = (SPECIAL) %d \n", SetNumber, SetNumber - VIP_DYNAMIC_OPTIMIZE_SYSTEM_SETTING_STA ); }
        else if( SetNumber >= TV_OTHER_SETTING_STA && SetNumber <= TV_OTHER_SETTING_END )
        { ROS_VideoPrintf_Setting("[VDPQ] Apply %d = (TV OTHER) %d \n", SetNumber, SetNumber - TV_OTHER_SETTING_STA ); }
        else if( SetNumber >= TV_PAL_SETTING_STA && SetNumber <= TV_PAL_SETTING_END )
        { ROS_VideoPrintf_Setting("[VDPQ] Apply %d = (TV PAL) %d \n", SetNumber, SetNumber - TV_PAL_SETTING_STA ); }
        else if( SetNumber >= TV_NTSC_SETTING_STA && SetNumber <= TV_NTSC_SETTING_END )
        { ROS_VideoPrintf_Setting("[VDPQ] Apply %d = (TV NTSC) %d \n", SetNumber, SetNumber - TV_NTSC_SETTING_STA ); }
        else if( SetNumber >= TV_COMMON_SETTING_STA && SetNumber <= TV_COMMON_SETTING_END )
        { ROS_VideoPrintf_Setting("[VDPQ] Apply %d = (TV COMMON) %d \n", SetNumber, SetNumber - TV_COMMON_SETTING_STA ); }
        else if( SetNumber >= AV_OTHER_SETTING_STA && SetNumber <= AV_OTHER_SETTING_END )
        { ROS_VideoPrintf_Setting("[VDPQ] Apply %d = (AV OTHER) %d \n", SetNumber, SetNumber - AV_OTHER_SETTING_STA ); }
        else if( SetNumber >= AV_PAL_SETTING_STA && SetNumber <= AV_PAL_SETTING_END )
        { ROS_VideoPrintf_Setting("[VDPQ] Apply %d = (AV PAL) %d \n", SetNumber, SetNumber - AV_PAL_SETTING_STA ); }
        else if( SetNumber >= AV_NTSC_SETTING_STA && SetNumber <= AV_NTSC_SETTING_END )
        { ROS_VideoPrintf_Setting("[VDPQ] Apply %d = (AV NTSC) %d \n", SetNumber, SetNumber - AV_NTSC_SETTING_STA ); }
        else if(SetNumber <= AV_COMMON_SETTING_END )//fix coverity 147489
        { ROS_VideoPrintf_Setting("[VDPQ] Apply %d = (AV COMMON) %d \n", SetNumber, SetNumber - AV_COMMON_SETTING_STA ); }
    }

    switch(SetNumber)
    {
        /*====================================================================================================*/
        /*======================================= SOME FUNCTIONAL CASE =======================================*/
        case VDPQ_FUNC_RST_ALL:
        {
            //-------------------------------------------------------------------------------------------------
            // read the register value
            vdpq_yc_bw_ctrl_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_BW_CTRL_reg);
            vdpq_bpf_bw_sel_y_reg.regValue = VDPQ_rtd_inl(VDPQ_BPF_BW_SEL_Y_reg);
            vdpq_bpf_bw_sel_c_reg.regValue = VDPQ_rtd_inl(VDPQ_BPF_BW_SEL_C_reg);
            vdpq_adap_bpf_c_th1_reg.regValue = VDPQ_rtd_inl(VDPQ_ADAP_BPF_C_TH1_reg);
            vdpq_alpha_2d_mod_enable_reg.regValue = VDPQ_rtd_inl(VDPQ_ALPHA_2D_MOD_ENABLE_reg);
            vdpq_yc_3d_f4_pair_ctrl_4_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Pair_Ctrl_4_reg);
            vdpq_yc_3d_f4_still_vector_1_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Still_Vector_1_reg);
            vdpq_yc_3d_f4_still_vector_2_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Still_Vector_2_reg);
            vdpq_yc_3d_f4_pair_ctrl_1_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Pair_Ctrl_1_reg);
            vdpq_h2v_y_noise_thr_reg.regValue = VDPQ_rtd_inl(VDPQ_H2V_Y_NOISE_THR_reg);
            vdpq_yc_3d_f4_still_gray_hifreq_1_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Still_Gray_HiFreq_1_reg);
            vdpq_yc_3d_f4_still_gray_hifreq_2_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Still_Gray_HiFreq_2_reg);
            vdpq_postp_mf_compensation_ctrl1_reg.regValue = VDPQ_rtd_inl(VDPQ_POSTP_MF_COMPENSATION_CTRL1_reg);
            vdpq_yc_3d_f4_pair_ctrl_6_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Pair_Ctrl_6_reg);
            vdpq_yc_3d_f4_pair_ctrl_3_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Pair_Ctrl_3_reg);
            vdpq_yc2d_postlp_ctrl_reg.regValue = VDPQ_rtd_inl(VDPQ_yc2d_postlp_ctrl_reg);
            vdpq_yc_3d_f4_pair_ctrl_5_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Pair_Ctrl_5_reg);
            vdpq_adap_bpf_y_th3_reg.regValue = VDPQ_rtd_inl(VDPQ_ADAP_BPF_Y_TH3_reg);
            vdpq_yc2d_postlp_grad_1_reg.regValue = VDPQ_rtd_inl(VDPQ_yc2d_postlp_grad_1_reg);
            vdpq_comb_filter_threshold1_reg.regValue = VDPQ_rtd_inl(VDPQ_COMB_FILTER_THRESHOLD1_reg);
            vdpq_cdiff_adap_bpf_th2_reg.regValue = VDPQ_rtd_inl(VDPQ_CDIFF_ADAP_BPF_TH2_reg);
            vdpq_bypass_bpf_ctrl1_reg.regValue = VDPQ_rtd_inl(VDPQ_BYPASS_BPF_CTRL1_reg);
            vdpq_bypass_bpf_ctrl2_reg.regValue = VDPQ_rtd_inl(VDPQ_BYPASS_BPF_CTRL2_reg);
            vdpq_yc_3d_f4_spa_cnt_cvbs_amp_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Spa_Cnt_CVBS_Amp_reg);
            vdpq_xnr_ctrl_reg.regValue = VDPQ_rtd_inl(VDPQ_XNR_CTRL_reg);
            vdpq_xnr_thr_reg.regValue = VDPQ_rtd_inl(VDPQ_XNR_THR_reg);
            vdpq_yc_3d_f4_mghf_ctrl_1_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_MGHF_Ctrl_1_reg);
            vdpq_h2v_noise_y_add_dc_reg.regValue = VDPQ_rtd_inl(VDPQ_H2V_NOISE_Y_ADD_DC_reg);
            vdpq_vmc_ctrl_reg.regValue = VDPQ_rtd_inl(VDPQ_VMC_CTRL_reg);
            vdpq_diamc_cvbs_amp_th_reg.regValue = VDPQ_rtd_inl(VDPQ_DIAMC_CVBS_AMP_TH_reg);
            vdpq_yc_post2dbpf_dbwline_1_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_Post2DBPF_DBWline_1_reg);
            vdpq_postp_3d2dalpha_blend_ctrl_1_reg.regValue = VDPQ_rtd_inl(VDPQ_POSTP_3D2DALPHA_BLEND_CTRL_1_reg);
            vdpq_postp_3d2dalpha_blend_ctrl_2_reg.regValue = VDPQ_rtd_inl(VDPQ_POSTP_3D2DALPHA_BLEND_CTRL_2_reg);
            vdpq_de_xc_ctrl_reg.regValue = VDPQ_rtd_inl(VDPQ_DE_XC_CTRL_reg);
            vdpq_yc_post2dbpf_ctrl_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_Post2DBPF_Ctrl_reg);
            vdpq_yc_post2dbpf_thbase_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_Post2DBPF_ThBase_reg);
            vdpq_yc_post2dbpf_adapth_1_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_Post2DBPF_AdapTh_1_reg);
            vdpq_yc_post2dbpf_adapth_13_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_Post2DBPF_AdapTh_13_reg);
            vdpq_yc_3d_f4_smd_en_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_SMD_EN_reg);
            vdpq_yc_3d_f4_smd_th1_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_SMD_TH1_reg);
            vdpq_yc_3d_f4_smd_th2_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_SMD_TH2_reg);
            vdpq_yc_3d_f4_pair_smd_th1_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Pair_SMD_TH1_reg);
            vdpq_yc_3d_f4_pal_err_com_1_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_PAL_Err_Com_1_reg);
            vdpq_yc_3d_f4_dc_compare_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_DC_Compare_reg);
            vdpq_yc_3d_f4_pal_err_com_2_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_PAL_Err_Com_2_reg);
            vdpq_yc_3d_f4_global_diff_th_hifreq_offset_3_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Global_Diff_Th_Hifreq_Offset_3_reg);
            vdpq_yc_3d_f4_global_diff_th_hifreq_offset_4_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_Global_Diff_Th_Hifreq_Offset_4_reg);
            vdpq_blend_1d_thr_reg.regValue = VDPQ_rtd_inl(VDPQ_BLEND_1D_THR_reg);
            vdpq_yc2d_postlp_flat_1_reg.regValue = VDPQ_rtd_inl(VDPQ_yc2d_postlp_flat_1_reg);
            vdpq_yc2d_postlp_flat_2_reg.regValue = VDPQ_rtd_inl(VDPQ_yc2d_postlp_flat_2_reg);
            vdpq_yc2d_postlp_flat_3_reg.regValue = VDPQ_rtd_inl(VDPQ_yc2d_postlp_flat_3_reg);
            vdpq_yc_3d_f4_hv_edge_1_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_HV_Edge_1_reg);
            vdpq_h2v_blend_ratio_reg.regValue = VDPQ_rtd_inl(VDPQ_H2V_BLEND_RATIO_reg);
			vdpq_directional_3d_reg.regValue = VDPQ_rtd_inl(VDPQ_DIRECTIONAL_3D_reg);
			vdpq_yc_sep_mode_ctrl_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_SEP_MODE_CTRL_reg);
			vdpq_yc_3d_f4_pal_err_com_3_reg.regValue = VDPQ_rtd_inl(VDPQ_YC_3D_F4_PAL_Err_Com_3_reg);
			vdpq_cadap_gain_ctrl_reg.regValue = VDPQ_rtd_inl(VDPQ_CADAP_GAIN_CTRL_reg);
			vdpq_cadap_lut_gain_th_reg.regValue = VDPQ_rtd_inl(VDPQ_CADAP_LUT_GAIN_TH_reg);
			vdpq_alpha_blending_ctrl1_reg.regValue = VDPQ_rtd_inl(VDPQ_ALPHA_BLENDING_CTRL1_reg); // willy
			vdpq_alpha_blending_ctrl2_reg.regValue = VDPQ_rtd_inl(VDPQ_ALPHA_BLENDING_CTRL2_reg);
			vdpq_alpha_blending_ctrl3_reg.regValue = VDPQ_rtd_inl(VDPQ_ALPHA_BLENDING_CTRL3_reg);
		vdpq_alpha_blending_ctrl4_reg.regValue = VDPQ_rtd_inl(VDPQ_ALPHA_BLENDING_CTRL4_reg);
            // non-VD register
            di_im_di_tnr_xc_control_reg.regValue = rtd_inl(DI_IM_DI_TNR_XC_CONTROL_reg);
            di_im_di_control_reg.regValue = rtd_inl(DI_IM_DI_CONTROL_reg);
            //-------------------------------------------------------------------------------------------------

            //-------------------------------------------------------------------------------------------------
            // Reset the auto control register value to the default value first
            vdpq_yc_bw_ctrl_reg.chroma_bw_lo = ORI_vdpq_yc_bw_ctrl_reg.chroma_bw_lo;
            vdpq_yc_bw_ctrl_reg.chroma_vlpf_en = ORI_vdpq_yc_bw_ctrl_reg.chroma_vlpf_en;
            vdpq_bpf_bw_sel_y_reg.wide_bpf_sel_y = ORI_vdpq_bpf_bw_sel_y_reg.wide_bpf_sel_y;
            vdpq_bpf_bw_sel_y_reg.mid_bpf_sel_y = ORI_vdpq_bpf_bw_sel_y_reg.mid_bpf_sel_y;
            vdpq_bpf_bw_sel_y_reg.narrow_bpf_sel_y = ORI_vdpq_bpf_bw_sel_y_reg.narrow_bpf_sel_y;
			vdpq_bpf_bw_sel_c_reg.wide_bpf_sel_c = ORI_vdpq_bpf_bw_sel_c_reg.wide_bpf_sel_c;
            vdpq_bpf_bw_sel_c_reg.narrow_bpf_sel_c = ORI_vdpq_bpf_bw_sel_c_reg.narrow_bpf_sel_c;
            vdpq_bpf_bw_sel_c_reg.mid_bpf_sel_c = ORI_vdpq_bpf_bw_sel_c_reg.mid_bpf_sel_c;
            vdpq_adap_bpf_c_th1_reg.h2v_lk_en = ORI_vdpq_adap_bpf_c_th1_reg.h2v_lk_en;
            vdpq_adap_bpf_c_th1_reg.h2v_mid_sel_y = ORI_vdpq_adap_bpf_c_th1_reg.h2v_mid_sel_y;
            vdpq_adap_bpf_c_th1_reg.v2v_lk_y_en = ORI_vdpq_adap_bpf_c_th1_reg.v2v_lk_y_en;
            vdpq_alpha_2d_mod_enable_reg.h2v_coring_thr = ORI_vdpq_alpha_2d_mod_enable_reg.h2v_coring_thr;
            vdpq_alpha_2d_mod_enable_reg.lut_sel_y = ORI_vdpq_alpha_2d_mod_enable_reg.lut_sel_y;
            vdpq_alpha_2d_mod_enable_reg.force_halpha_en = ORI_vdpq_alpha_2d_mod_enable_reg.force_halpha_en;
            vdpq_yc_3d_f4_pair_ctrl_4_reg.diff_th_upper_bound = ORI_vdpq_yc_3d_f4_pair_ctrl_4_reg.diff_th_upper_bound;
            vdpq_yc_3d_f4_pair_ctrl_4_reg.diff_th_lower_bound = ORI_vdpq_yc_3d_f4_pair_ctrl_4_reg.diff_th_lower_bound;
            vdpq_yc_3d_f4_still_vector_1_reg.sv_vector_fid_match_cnt_th = ORI_vdpq_yc_3d_f4_still_vector_1_reg.sv_vector_fid_match_cnt_th;
            vdpq_yc_3d_f4_still_vector_1_reg.sv_vector_fid_offset = ORI_vdpq_yc_3d_f4_still_vector_1_reg.sv_vector_fid_offset;
            vdpq_yc_3d_f4_still_vector_2_reg.sv_self_diff_small_th = ORI_vdpq_yc_3d_f4_still_vector_2_reg.sv_self_diff_small_th;
            vdpq_yc_3d_f4_pair_ctrl_1_reg.still_vector_offset_base = ORI_vdpq_yc_3d_f4_pair_ctrl_1_reg.still_vector_offset_base;
            vdpq_yc_3d_f4_pair_ctrl_1_reg.still_vector_offset_multiple = ORI_vdpq_yc_3d_f4_pair_ctrl_1_reg.still_vector_offset_multiple;
            vdpq_h2v_y_noise_thr_reg.h2v_y_noise_thr = ORI_vdpq_h2v_y_noise_thr_reg.h2v_y_noise_thr;
            vdpq_yc_3d_f4_still_gray_hifreq_1_reg.sghf_longtime_en = ORI_vdpq_yc_3d_f4_still_gray_hifreq_1_reg.sghf_longtime_en;
            vdpq_yc_3d_f4_still_gray_hifreq_1_reg.sghf_inter_diff_th = ORI_vdpq_yc_3d_f4_still_gray_hifreq_1_reg.sghf_inter_diff_th;
            vdpq_yc_3d_f4_still_gray_hifreq_1_reg.sghf_global_diff_small_th = ORI_vdpq_yc_3d_f4_still_gray_hifreq_1_reg.sghf_global_diff_small_th;
            vdpq_yc_3d_f4_still_gray_hifreq_2_reg.sghf_vector_fid_match_cnt_th = ORI_vdpq_yc_3d_f4_still_gray_hifreq_2_reg.sghf_vector_fid_match_cnt_th;
            vdpq_yc_3d_f4_still_gray_hifreq_2_reg.sghf_vector_large_th = ORI_vdpq_yc_3d_f4_still_gray_hifreq_2_reg.sghf_vector_large_th;
            vdpq_postp_mf_compensation_ctrl1_reg.mf_en = ORI_vdpq_postp_mf_compensation_ctrl1_reg.mf_en;
            vdpq_postp_mf_compensation_ctrl1_reg.cdiff_small_con_en = ORI_vdpq_postp_mf_compensation_ctrl1_reg.cdiff_small_con_en;
            vdpq_postp_mf_compensation_ctrl1_reg.ma0_cnt_th = ORI_vdpq_postp_mf_compensation_ctrl1_reg.ma0_cnt_th;
            vdpq_yc_3d_f4_pair_ctrl_6_reg.global_diff_tolerance = ORI_vdpq_yc_3d_f4_pair_ctrl_6_reg.global_diff_tolerance;
			vdpq_yc_3d_f4_pair_ctrl_6_reg.dc_level_fierce_change_th = ORI_vdpq_yc_3d_f4_pair_ctrl_6_reg.dc_level_fierce_change_th;
			vdpq_yc_3d_f4_pair_ctrl_6_reg.hdy_small_th = ORI_vdpq_yc_3d_f4_pair_ctrl_6_reg.hdy_small_th;
			vdpq_yc_3d_f4_pair_ctrl_3_reg.pair_diff_sel_th = ORI_vdpq_yc_3d_f4_pair_ctrl_3_reg.pair_diff_sel_th;
            vdpq_yc2d_postlp_ctrl_reg.postlpf_grad_en = ORI_vdpq_yc2d_postlp_ctrl_reg.postlpf_grad_en;
            vdpq_yc2d_postlp_ctrl_reg.postlpf_en = ORI_vdpq_yc2d_postlp_ctrl_reg.postlpf_en;
            vdpq_yc_3d_f4_pair_ctrl_5_reg.still_gray_hifreq_th = ORI_vdpq_yc_3d_f4_pair_ctrl_5_reg.still_gray_hifreq_th;
            vdpq_yc_3d_f4_pair_ctrl_5_reg.still_gray_hifreq_low_border = ORI_vdpq_yc_3d_f4_pair_ctrl_5_reg.still_gray_hifreq_low_border;
            vdpq_adap_bpf_y_th3_reg.malpha_c_en = ORI_vdpq_adap_bpf_y_th3_reg.malpha_c_en;
            vdpq_adap_bpf_y_th3_reg.malpha_c = ORI_vdpq_adap_bpf_y_th3_reg.malpha_c;
            vdpq_adap_bpf_y_th3_reg.malpha_y_en = ORI_vdpq_adap_bpf_y_th3_reg.malpha_y;
            vdpq_adap_bpf_y_th3_reg.malpha_y = ORI_vdpq_adap_bpf_y_th3_reg.malpha_y;
            vdpq_yc2d_postlp_grad_1_reg.larger_cnt_th_0 = ORI_vdpq_yc2d_postlp_grad_1_reg.larger_cnt_th_0;
            vdpq_comb_filter_threshold1_reg.noise_ntsc_c = ORI_vdpq_comb_filter_threshold1_reg.noise_ntsc_c;
            vdpq_cdiff_adap_bpf_th2_reg.cdiff_bpf_small_amp_wei = ORI_vdpq_cdiff_adap_bpf_th2_reg.cdiff_bpf_small_amp_wei;
            vdpq_cdiff_adap_bpf_th2_reg.cdiff_bpf_small_offset = ORI_vdpq_cdiff_adap_bpf_th2_reg.cdiff_bpf_small_offset;
            vdpq_cdiff_adap_bpf_th2_reg.cdiff_bpf_large_amp_wei = ORI_vdpq_cdiff_adap_bpf_th2_reg.cdiff_bpf_large_amp_wei;
            vdpq_cdiff_adap_bpf_th2_reg.cdiff_bpf_large_offset = ORI_vdpq_cdiff_adap_bpf_th2_reg.cdiff_bpf_large_offset;
            vdpq_cdiff_adap_bpf_th2_reg.cdiff_bpf_vspd_small_th = ORI_vdpq_cdiff_adap_bpf_th2_reg.cdiff_bpf_vspd_small_th;
            vdpq_bypass_bpf_ctrl2_reg.newvedge_vdiff_th = ORI_vdpq_bypass_bpf_ctrl2_reg.newvedge_vdiff_th;
			vdpq_yc_3d_f4_pal_err_com_3_reg.f4f0_still_en = ORI_vdpq_yc_3d_f4_pal_err_com_3_reg.f4f0_still_en;

			// Directional 3D
			vdpq_directional_3d_reg.dir3d_fw_mode_dalpha = ORI_vdpq_directional_3d_reg.dir3d_fw_mode_dalpha;
			vdpq_directional_3d_reg.dir3d_fw_mode_motion = ORI_vdpq_directional_3d_reg.dir3d_fw_mode_motion;

			// 2D 3D Alpha
            vdpq_yc_3d_f4_spa_cnt_cvbs_amp_reg.f4_3d_spatial_cnt_th_1 = ORI_vdpq_yc_3d_f4_spa_cnt_cvbs_amp_reg.f4_3d_spatial_cnt_th_1;
            vdpq_yc_3d_f4_spa_cnt_cvbs_amp_reg.f4_3d_spatial_cnt_th_2 = ORI_vdpq_yc_3d_f4_spa_cnt_cvbs_amp_reg.f4_3d_spatial_cnt_th_2;
            vdpq_yc_3d_f4_spa_cnt_cvbs_amp_reg.f4_3d_spatial_cnt_th_3 = ORI_vdpq_yc_3d_f4_spa_cnt_cvbs_amp_reg.f4_3d_spatial_cnt_th_3;
            vdpq_yc_3d_f4_spa_cnt_cvbs_amp_reg.f4_3d_spatial_cnt_th_4 = ORI_vdpq_yc_3d_f4_spa_cnt_cvbs_amp_reg.f4_3d_spatial_cnt_th_4;
            vdpq_yc_3d_f4_spa_cnt_cvbs_amp_reg.f4_3d_spatial_cnt_th_5 = ORI_vdpq_yc_3d_f4_spa_cnt_cvbs_amp_reg.f4_3d_spatial_cnt_th_5;

			vdpq_alpha_blending_ctrl1_reg.alpha2d3d_bld_weight_1 = ORI_vdpq_alpha_blending_ctrl1_reg.alpha2d3d_bld_weight_1;
			vdpq_alpha_blending_ctrl1_reg.alpha2d3d_bld_weight_2 = ORI_vdpq_alpha_blending_ctrl1_reg.alpha2d3d_bld_weight_2;
			vdpq_alpha_blending_ctrl1_reg.alpha2d3d_bld_weight_3 = ORI_vdpq_alpha_blending_ctrl1_reg.alpha2d3d_bld_weight_3;
			vdpq_alpha_blending_ctrl1_reg.alpha2d3d_bld_weight_4 = ORI_vdpq_alpha_blending_ctrl1_reg.alpha2d3d_bld_weight_4;
			vdpq_alpha_blending_ctrl2_reg.alpha2d3d_bld_weight_5 = ORI_vdpq_alpha_blending_ctrl2_reg.alpha2d3d_bld_weight_5;
			vdpq_alpha_blending_ctrl2_reg.alpha2d3d_bld_weight_6 = ORI_vdpq_alpha_blending_ctrl2_reg.alpha2d3d_bld_weight_6;
			vdpq_alpha_blending_ctrl2_reg.alpha2d3d_bld_weight_7 = ORI_vdpq_alpha_blending_ctrl2_reg.alpha2d3d_bld_weight_7;
			vdpq_alpha_blending_ctrl2_reg.alpha2d3d_bld_weight_8 = ORI_vdpq_alpha_blending_ctrl2_reg.alpha2d3d_bld_weight_8;
			vdpq_alpha_blending_ctrl3_reg.alpha2d3d_bld_weight_9 = ORI_vdpq_alpha_blending_ctrl3_reg.alpha2d3d_bld_weight_9;
			vdpq_alpha_blending_ctrl3_reg.alpha2d3d_bld_weight_10 = ORI_vdpq_alpha_blending_ctrl3_reg.alpha2d3d_bld_weight_10;
			vdpq_alpha_blending_ctrl3_reg.alpha2d3d_bld_weight_11 = ORI_vdpq_alpha_blending_ctrl3_reg.alpha2d3d_bld_weight_11;
			vdpq_alpha_blending_ctrl3_reg.alpha2d3d_bld_weight_12 = ORI_vdpq_alpha_blending_ctrl3_reg.alpha2d3d_bld_weight_12;
			vdpq_alpha_blending_ctrl4_reg.alpha2d3d_bld_weight_13 = ORI_vdpq_alpha_blending_ctrl4_reg.alpha2d3d_bld_weight_13;
			vdpq_alpha_blending_ctrl4_reg.alpha2d3d_bld_weight_14 = ORI_vdpq_alpha_blending_ctrl4_reg.alpha2d3d_bld_weight_14;
			vdpq_alpha_blending_ctrl4_reg.alpha2d3d_bld_weight_15 = ORI_vdpq_alpha_blending_ctrl4_reg.alpha2d3d_bld_weight_15;

			// Fix 3D
			vdpq_yc_sep_mode_ctrl_reg.yc_sep_mode = ORI_vdpq_yc_sep_mode_ctrl_reg.yc_sep_mode;

            vdpq_yc_3d_f4_pal_err_com_1_reg.palerr_accu_tolerance = ORI_vdpq_yc_3d_f4_pal_err_com_1_reg.palerr_accu_tolerance;
            vdpq_yc_3d_f4_pal_err_com_1_reg.palerr_hifreq_diff_large_th = ORI_vdpq_yc_3d_f4_pal_err_com_1_reg.palerr_hifreq_diff_large_th;
            vdpq_yc_3d_f4_dc_compare_reg.dc_level_change_th_base = ORI_vdpq_yc_3d_f4_dc_compare_reg.dc_level_change_th_base;
            vdpq_yc_3d_f4_dc_compare_reg.dc_level_change_th_multiple = ORI_vdpq_yc_3d_f4_dc_compare_reg.dc_level_change_th_multiple;
            vdpq_yc_3d_f4_pal_err_com_2_reg.massive_palmode_error_multiple_th = ORI_vdpq_yc_3d_f4_pal_err_com_2_reg.massive_palmode_error_multiple_th;
            vdpq_yc_3d_f4_global_diff_th_hifreq_offset_3_reg.global_diff_th_hifreq_offset_20 = ORI_vdpq_yc_3d_f4_global_diff_th_hifreq_offset_3_reg.global_diff_th_hifreq_offset_20;
            vdpq_yc_3d_f4_global_diff_th_hifreq_offset_4_reg.global_diff_th_hifreq_offset_10 = ORI_vdpq_yc_3d_f4_global_diff_th_hifreq_offset_4_reg.global_diff_th_hifreq_offset_10;
            vdpq_blend_1d_thr_reg.bpf_1d_sel_y = ORI_vdpq_blend_1d_thr_reg.bpf_1d_sel_y;
            vdpq_h2v_blend_ratio_reg.h2v_blendratio = ORI_vdpq_h2v_blend_ratio_reg.h2v_blendratio;
            // XNR
            vdpq_xnr_ctrl_reg.xnr_lpf_sel = ORI_vdpq_xnr_ctrl_reg.xnr_lpf_sel;
            vdpq_xnr_ctrl_reg.xnr_vlpf_sel = ORI_vdpq_xnr_ctrl_reg.xnr_vlpf_sel;
            vdpq_xnr_thr_reg.xnr_csum_thr = ORI_vdpq_xnr_thr_reg.xnr_csum_thr;
            vdpq_xnr_thr_reg.xnr_cdiff_thr = ORI_vdpq_xnr_thr_reg.xnr_cdiff_thr;
            // MVLine
            vdpq_yc_3d_f4_mghf_ctrl_1_reg.moving_vline_flat_ver_cvbs_th = ORI_vdpq_yc_3d_f4_mghf_ctrl_1_reg.moving_vline_flat_ver_cvbs_th;
            vdpq_yc_3d_f4_mghf_ctrl_1_reg.moving_vline_ver_dc_level_fierce_change_th = ORI_vdpq_yc_3d_f4_mghf_ctrl_1_reg.moving_vline_ver_dc_level_fierce_change_th;
            // V2V
            vdpq_h2v_noise_y_add_dc_reg.v2v_lut_sel_y = ORI_vdpq_h2v_noise_y_add_dc_reg.v2v_lut_sel_y;
            // VMC
            vdpq_vmc_ctrl_reg.ver_dc_still_th = ORI_vdpq_vmc_ctrl_reg.ver_dc_still_th;
            vdpq_vmc_ctrl_reg.ver_cvbsamp_still_th = ORI_vdpq_vmc_ctrl_reg.ver_cvbsamp_still_th;
            vdpq_vmc_ctrl_reg.amp_low_bnd = ORI_vdpq_vmc_ctrl_reg.amp_low_bnd;
            vdpq_diamc_cvbs_amp_th_reg.vmc_widehspd_small_th = ORI_vdpq_diamc_cvbs_amp_th_reg.vmc_widehspd_small_th;
            // Post2DBPF DBWline
            vdpq_yc_post2dbpf_dbwline_1_reg.dbwline_en = ORI_vdpq_yc_post2dbpf_dbwline_1_reg.dbwline_en;
            // post process
            vdpq_postp_3d2dalpha_blend_ctrl_1_reg.xnr_alpha_2d_weight = ORI_vdpq_postp_3d2dalpha_blend_ctrl_1_reg.xnr_alpha_2d_weight;
            vdpq_postp_3d2dalpha_blend_ctrl_1_reg.xnr_alpha_bld_weight = ORI_vdpq_postp_3d2dalpha_blend_ctrl_1_reg.xnr_alpha_bld_weight;
            vdpq_de_xc_ctrl_reg.de_xc_en = ORI_vdpq_de_xc_ctrl_reg.de_xc_en;
            vdpq_de_xc_ctrl_reg.de_xc_thr = ORI_vdpq_de_xc_ctrl_reg.de_xc_thr;
            // post 2d bpf y
            vdpq_yc_post2dbpf_ctrl_reg.weight_2d_y = ORI_vdpq_yc_post2dbpf_ctrl_reg.weight_2d_y;
            vdpq_yc_post2dbpf_ctrl_reg.weight_bld_y = ORI_vdpq_yc_post2dbpf_ctrl_reg.weight_bld_y;
            vdpq_yc_post2dbpf_ctrl_reg.weight_2d_c = ORI_vdpq_yc_post2dbpf_ctrl_reg.weight_2d_c;
            vdpq_yc_post2dbpf_ctrl_reg.weight_bld_c = ORI_vdpq_yc_post2dbpf_ctrl_reg.weight_bld_c;
            vdpq_yc_post2dbpf_ctrl_reg.greenish_detected_en = ORI_vdpq_yc_post2dbpf_ctrl_reg.greenish_detected_en;
            vdpq_yc_post2dbpf_ctrl_reg.mghf_narrowbpf_en = ORI_vdpq_yc_post2dbpf_ctrl_reg.mghf_narrowbpf_en;
            vdpq_yc_post2dbpf_thbase_reg.hor_cdiff_th_base = ORI_vdpq_yc_post2dbpf_thbase_reg.hor_cdiff_th_base;
            vdpq_yc_post2dbpf_thbase_reg.ver_cdiff_th_base = ORI_vdpq_yc_post2dbpf_thbase_reg.ver_cdiff_th_base;
            vdpq_yc_post2dbpf_adapth_1_reg.bpfed_c_diff_small_th = ORI_vdpq_yc_post2dbpf_adapth_1_reg.bpfed_c_diff_small_th;
            vdpq_yc_post2dbpf_adapth_13_reg.ignore_th_y = ORI_vdpq_yc_post2dbpf_adapth_13_reg.ignore_th_y;
            // SMD
            vdpq_yc_3d_f4_smd_en_reg.smd_en = ORI_vdpq_yc_3d_f4_smd_en_reg.smd_en;
            vdpq_yc_3d_f4_smd_en_reg.smd_hor_00_move_enable = ORI_vdpq_yc_3d_f4_smd_en_reg.smd_hor_00_move_enable;
            vdpq_yc_3d_f4_smd_en_reg.smd_ver_00_move_enable = ORI_vdpq_yc_3d_f4_smd_en_reg.smd_ver_00_move_enable;
            vdpq_yc_3d_f4_smd_en_reg.smd_hor_06_move_enable = ORI_vdpq_yc_3d_f4_smd_en_reg.smd_hor_06_move_enable;
            vdpq_yc_3d_f4_smd_en_reg.smd_ver_06_move_enable = ORI_vdpq_yc_3d_f4_smd_en_reg.smd_ver_06_move_enable;
            vdpq_yc_3d_f4_smd_en_reg.smd_hor_08_move_enable = ORI_vdpq_yc_3d_f4_smd_en_reg.smd_hor_08_move_enable;
            vdpq_yc_3d_f4_smd_en_reg.smd_ver_08_move_enable = ORI_vdpq_yc_3d_f4_smd_en_reg.smd_ver_08_move_enable;
            vdpq_yc_3d_f4_smd_en_reg.smd_hor_10_move_enable = ORI_vdpq_yc_3d_f4_smd_en_reg.smd_hor_10_move_enable;
            vdpq_yc_3d_f4_smd_en_reg.smd_ver_10_move_enable = ORI_vdpq_yc_3d_f4_smd_en_reg.smd_ver_10_move_enable;
            vdpq_yc_3d_f4_smd_th1_reg.smd_hor_spatial_temporal_traily_diff = ORI_vdpq_yc_3d_f4_smd_th1_reg.smd_hor_spatial_temporal_traily_diff;
            vdpq_yc_3d_f4_smd_th1_reg.smd_ver_spatial_temporal_traily_diff = ORI_vdpq_yc_3d_f4_smd_th1_reg.smd_ver_spatial_temporal_traily_diff;
            vdpq_yc_3d_f4_smd_th1_reg.smd_horizontal_sad_tolerance = ORI_vdpq_yc_3d_f4_smd_th1_reg.smd_horizontal_sad_tolerance;
            vdpq_yc_3d_f4_smd_th1_reg.smd_vertical_sad_tolerance = ORI_vdpq_yc_3d_f4_smd_th1_reg.smd_vertical_sad_tolerance;
            vdpq_yc_3d_f4_smd_th2_reg.smd_min_sad_horizontal = ORI_vdpq_yc_3d_f4_smd_th2_reg.smd_min_sad_horizontal;
            vdpq_yc_3d_f4_smd_th2_reg.smd_min_sad_vetical = ORI_vdpq_yc_3d_f4_smd_th2_reg.smd_min_sad_vetical;
            vdpq_yc_3d_f4_pair_smd_th1_reg.smd_highfreq_diff_th = ORI_vdpq_yc_3d_f4_pair_smd_th1_reg.smd_highfreq_diff_th;
            vdpq_yc_3d_f4_pair_smd_th1_reg.smd_2d_small_global_diff_offset = ORI_vdpq_yc_3d_f4_pair_smd_th1_reg.smd_2d_small_global_diff_offset;
            // 2D post LPF
            vdpq_yc2d_postlp_flat_1_reg.low_amp_th_0 = ORI_vdpq_yc2d_postlp_flat_1_reg.low_amp_th_0;
            vdpq_yc2d_postlp_flat_1_reg.low_amp_th_1 = ORI_vdpq_yc2d_postlp_flat_1_reg.low_amp_th_1;
            vdpq_yc2d_postlp_flat_1_reg.low_amp_th_2 = ORI_vdpq_yc2d_postlp_flat_1_reg.low_amp_th_2;
            vdpq_yc2d_postlp_flat_1_reg.low_amp_th_3 = ORI_vdpq_yc2d_postlp_flat_1_reg.low_amp_th_3;
            vdpq_yc2d_postlp_flat_2_reg.low_amp_th_4 = ORI_vdpq_yc2d_postlp_flat_2_reg.low_amp_th_4;
            vdpq_yc2d_postlp_flat_2_reg.hdiff_th_0 = ORI_vdpq_yc2d_postlp_flat_2_reg.hdiff_th_0;
            vdpq_yc2d_postlp_flat_2_reg.hdiff_th_1 = ORI_vdpq_yc2d_postlp_flat_2_reg.hdiff_th_1;
            vdpq_yc2d_postlp_flat_2_reg.hdiff_th_2 = ORI_vdpq_yc2d_postlp_flat_2_reg.hdiff_th_2;
            vdpq_yc2d_postlp_flat_3_reg.hdiff_th_3 = ORI_vdpq_yc2d_postlp_flat_3_reg.hdiff_th_3;
            vdpq_yc2d_postlp_flat_3_reg.hdiff_th_4 = ORI_vdpq_yc2d_postlp_flat_3_reg.hdiff_th_4;
			// HV edge
			vdpq_yc_3d_f4_hv_edge_1_reg.hv_diff_tolerance = ORI_vdpq_yc_3d_f4_hv_edge_1_reg.hv_diff_tolerance;
			// VD CTI
			vdpq_cadap_gain_ctrl_reg.cadap_gain = ORI_vdpq_cadap_gain_ctrl_reg.cadap_gain;
			vdpq_cadap_lut_gain_th_reg.cadap_lut_gain_tha = ORI_vdpq_cadap_lut_gain_th_reg.cadap_lut_gain_tha;
			vdpq_cadap_lut_gain_th_reg.cadap_lut_gain_thb = ORI_vdpq_cadap_lut_gain_th_reg.cadap_lut_gain_thb;
            // non-VD registers
            di_im_di_tnr_xc_control_reg.cp_temporal_xc_en = ORI_di_im_di_tnr_xc_control_reg.cp_temporal_xc_en;
            di_im_di_control_reg.cp_temporalenable = ORI_di_im_di_control_reg.cp_temporalenable;
	    di_im_di_control_reg.write_enable_6 = 1;
            //-------------------------------------------------------------------------------------------------
            ScalerVideo_Status.isRegValueInit = TRUE;
            break;
        }
        case VDPQ_FUNC_WRITE_ADAPTIVE_REG_VAL:
        {
            if( ScalerVideo_Status.isRegValueInit == FALSE )
                break;
            // Write back to register
            VDPQ_rtd_outl(VDPQ_YC_BW_CTRL_reg, vdpq_yc_bw_ctrl_reg.regValue);
            VDPQ_rtd_outl(VDPQ_BPF_BW_SEL_Y_reg, vdpq_bpf_bw_sel_y_reg.regValue);
            VDPQ_rtd_outl(VDPQ_BPF_BW_SEL_C_reg, vdpq_bpf_bw_sel_c_reg.regValue);
            VDPQ_rtd_outl(VDPQ_ADAP_BPF_C_TH1_reg, vdpq_adap_bpf_c_th1_reg.regValue);
            VDPQ_rtd_outl(VDPQ_ALPHA_2D_MOD_ENABLE_reg, vdpq_alpha_2d_mod_enable_reg.regValue);
            VDPQ_rtd_outl(VDPQ_YC_3D_F4_Pair_Ctrl_4_reg, vdpq_yc_3d_f4_pair_ctrl_4_reg.regValue);
            VDPQ_rtd_outl(VDPQ_YC_3D_F4_Still_Vector_1_reg, vdpq_yc_3d_f4_still_vector_1_reg.regValue);
            VDPQ_rtd_outl(VDPQ_YC_3D_F4_Still_Vector_2_reg, vdpq_yc_3d_f4_still_vector_2_reg.regValue);
            VDPQ_rtd_outl(VDPQ_YC_3D_F4_Pair_Ctrl_1_reg, vdpq_yc_3d_f4_pair_ctrl_1_reg.regValue);
            VDPQ_rtd_outl(VDPQ_H2V_Y_NOISE_THR_reg, vdpq_h2v_y_noise_thr_reg.regValue);
            VDPQ_rtd_outl(VDPQ_YC_3D_F4_Still_Gray_HiFreq_1_reg, vdpq_yc_3d_f4_still_gray_hifreq_1_reg.regValue);
            VDPQ_rtd_outl(VDPQ_YC_3D_F4_Still_Gray_HiFreq_2_reg, vdpq_yc_3d_f4_still_gray_hifreq_2_reg.regValue);
            VDPQ_rtd_outl(VDPQ_POSTP_MF_COMPENSATION_CTRL1_reg, vdpq_postp_mf_compensation_ctrl1_reg.regValue);
            VDPQ_rtd_outl(VDPQ_YC_3D_F4_Pair_Ctrl_6_reg, vdpq_yc_3d_f4_pair_ctrl_6_reg.regValue);
            VDPQ_rtd_outl(VDPQ_YC_3D_F4_Pair_Ctrl_3_reg, vdpq_yc_3d_f4_pair_ctrl_3_reg.regValue);
            VDPQ_rtd_outl(VDPQ_yc2d_postlp_ctrl_reg, vdpq_yc2d_postlp_ctrl_reg.regValue);
            VDPQ_rtd_outl(VDPQ_YC_3D_F4_Pair_Ctrl_5_reg, vdpq_yc_3d_f4_pair_ctrl_5_reg.regValue);
            VDPQ_rtd_outl(VDPQ_ADAP_BPF_Y_TH3_reg, vdpq_adap_bpf_y_th3_reg.regValue);
            VDPQ_rtd_outl(VDPQ_yc2d_postlp_grad_1_reg, vdpq_yc2d_postlp_grad_1_reg.regValue);
            VDPQ_rtd_outl(VDPQ_COMB_FILTER_THRESHOLD1_reg, vdpq_comb_filter_threshold1_reg.regValue);
            VDPQ_rtd_outl(VDPQ_CDIFF_ADAP_BPF_TH2_reg, vdpq_cdiff_adap_bpf_th2_reg.regValue);
            VDPQ_rtd_outl(VDPQ_BYPASS_BPF_CTRL1_reg, vdpq_bypass_bpf_ctrl1_reg.regValue);
            VDPQ_rtd_outl(VDPQ_BYPASS_BPF_CTRL2_reg, vdpq_bypass_bpf_ctrl2_reg.regValue);
            VDPQ_rtd_outl(VDPQ_YC_3D_F4_Spa_Cnt_CVBS_Amp_reg, vdpq_yc_3d_f4_spa_cnt_cvbs_amp_reg.regValue);
            VDPQ_rtd_outl(VDPQ_XNR_CTRL_reg, vdpq_xnr_ctrl_reg.regValue);
            VDPQ_rtd_outl(VDPQ_XNR_THR_reg, vdpq_xnr_thr_reg.regValue);
            VDPQ_rtd_outl(VDPQ_YC_3D_F4_MGHF_Ctrl_1_reg, vdpq_yc_3d_f4_mghf_ctrl_1_reg.regValue);
            VDPQ_rtd_outl(VDPQ_H2V_NOISE_Y_ADD_DC_reg, vdpq_h2v_noise_y_add_dc_reg.regValue);
            VDPQ_rtd_outl(VDPQ_VMC_CTRL_reg, vdpq_vmc_ctrl_reg.regValue);
            VDPQ_rtd_outl(VDPQ_DIAMC_CVBS_AMP_TH_reg, vdpq_diamc_cvbs_amp_th_reg.regValue);
            VDPQ_rtd_outl(VDPQ_YC_Post2DBPF_DBWline_1_reg, vdpq_yc_post2dbpf_dbwline_1_reg.regValue);
            VDPQ_rtd_outl(VDPQ_POSTP_3D2DALPHA_BLEND_CTRL_1_reg, vdpq_postp_3d2dalpha_blend_ctrl_1_reg.regValue);
            VDPQ_rtd_outl(VDPQ_POSTP_3D2DALPHA_BLEND_CTRL_2_reg, vdpq_postp_3d2dalpha_blend_ctrl_2_reg.regValue);
            VDPQ_rtd_outl(VDPQ_DE_XC_CTRL_reg, vdpq_de_xc_ctrl_reg.regValue);
            VDPQ_rtd_outl(VDPQ_YC_Post2DBPF_Ctrl_reg, vdpq_yc_post2dbpf_ctrl_reg.regValue);
            VDPQ_rtd_outl(VDPQ_YC_Post2DBPF_ThBase_reg, vdpq_yc_post2dbpf_thbase_reg.regValue);
            VDPQ_rtd_outl(VDPQ_YC_Post2DBPF_AdapTh_1_reg, vdpq_yc_post2dbpf_adapth_1_reg.regValue);
            VDPQ_rtd_outl(VDPQ_YC_Post2DBPF_AdapTh_13_reg, vdpq_yc_post2dbpf_adapth_13_reg.regValue);
            VDPQ_rtd_outl(VDPQ_YC_3D_F4_SMD_EN_reg, vdpq_yc_3d_f4_smd_en_reg.regValue);
            VDPQ_rtd_outl(VDPQ_YC_3D_F4_SMD_TH1_reg, vdpq_yc_3d_f4_smd_th1_reg.regValue);
            VDPQ_rtd_outl(VDPQ_YC_3D_F4_SMD_TH2_reg, vdpq_yc_3d_f4_smd_th2_reg.regValue);
            VDPQ_rtd_outl(VDPQ_YC_3D_F4_Pair_SMD_TH1_reg, vdpq_yc_3d_f4_pair_smd_th1_reg.regValue);
            VDPQ_rtd_outl(VDPQ_YC_3D_F4_PAL_Err_Com_1_reg, vdpq_yc_3d_f4_pal_err_com_1_reg.regValue);
            VDPQ_rtd_outl(VDPQ_YC_3D_F4_DC_Compare_reg, vdpq_yc_3d_f4_dc_compare_reg.regValue);
            VDPQ_rtd_outl(VDPQ_YC_3D_F4_PAL_Err_Com_2_reg, vdpq_yc_3d_f4_pal_err_com_2_reg.regValue);
            VDPQ_rtd_outl(VDPQ_YC_3D_F4_Global_Diff_Th_Hifreq_Offset_3_reg, vdpq_yc_3d_f4_global_diff_th_hifreq_offset_3_reg.regValue);
            VDPQ_rtd_outl(VDPQ_YC_3D_F4_Global_Diff_Th_Hifreq_Offset_4_reg, vdpq_yc_3d_f4_global_diff_th_hifreq_offset_4_reg.regValue);
            VDPQ_rtd_outl(VDPQ_BLEND_1D_THR_reg, vdpq_blend_1d_thr_reg.regValue);
            VDPQ_rtd_outl(VDPQ_yc2d_postlp_flat_1_reg, vdpq_yc2d_postlp_flat_1_reg.regValue);
            VDPQ_rtd_outl(VDPQ_yc2d_postlp_flat_2_reg, vdpq_yc2d_postlp_flat_2_reg.regValue);
            VDPQ_rtd_outl(VDPQ_yc2d_postlp_flat_3_reg, vdpq_yc2d_postlp_flat_3_reg.regValue);
            VDPQ_rtd_outl(VDPQ_YC_3D_F4_HV_Edge_1_reg, vdpq_yc_3d_f4_hv_edge_1_reg.regValue);
            VDPQ_rtd_outl(VDPQ_H2V_BLEND_RATIO_reg, vdpq_h2v_blend_ratio_reg.regValue);
			VDPQ_rtd_outl(VDPQ_ALPHA_BLENDING_CTRL1_reg, vdpq_alpha_blending_ctrl1_reg.regValue);
			VDPQ_rtd_outl(VDPQ_ALPHA_BLENDING_CTRL2_reg, vdpq_alpha_blending_ctrl2_reg.regValue);
			VDPQ_rtd_outl(VDPQ_ALPHA_BLENDING_CTRL3_reg, vdpq_alpha_blending_ctrl3_reg.regValue);
			VDPQ_rtd_outl(VDPQ_ALPHA_BLENDING_CTRL4_reg, vdpq_alpha_blending_ctrl4_reg.regValue);
			VDPQ_rtd_outl(VDPQ_DIRECTIONAL_3D_reg, vdpq_directional_3d_reg.regValue);
			VDPQ_rtd_outl(VDPQ_YC_SEP_MODE_CTRL_reg, vdpq_yc_sep_mode_ctrl_reg.regValue);
			VDPQ_rtd_outl(VDPQ_YC_3D_F4_PAL_Err_Com_3_reg, vdpq_yc_3d_f4_pal_err_com_3_reg.regValue);
			VDPQ_rtd_outl(VDPQ_CADAP_GAIN_CTRL_reg, vdpq_cadap_gain_ctrl_reg.regValue);
			VDPQ_rtd_outl(VDPQ_CADAP_LUT_GAIN_TH_reg, vdpq_cadap_lut_gain_th_reg.regValue);
            // non-VD register
            rtd_outl(DI_IM_DI_TNR_XC_CONTROL_reg, di_im_di_tnr_xc_control_reg.regValue);

			// protect di 3Abit : reduce r/w time .
			di_control_reg_Prtoect_3AEnable.regValue= rtd_inl(DI_IM_DI_CONTROL_reg);
			di_im_di_control_reg.ma_3aenable = di_control_reg_Prtoect_3AEnable.ma_3aenable;
			di_im_di_control_reg.write_enable_1 = 1;
			rtd_outl(DI_IM_DI_CONTROL_reg, di_im_di_control_reg.regValue);

			//20160825 roger, add DB apply
			di_db_reg_ctl_Reg.regValue = IoReg_Read32(DI_db_reg_ctl_reg);
			di_db_reg_ctl_Reg.db_apply = 1;
			IoReg_Write32(DI_db_reg_ctl_reg, di_db_reg_ctl_Reg.regValue);
            break;
        }
        case VDPQ_FUNC_INIT_VALUE:
        {
            break;
        }
        case VDPQ_FUNC_AV_EXP:
        {
            break;
        }
        case VDPQ_FUNC_ATV_EXP:
        {
            break;
        }
        /*==================================== END OF SOME FUNCTIONAL CASE ====================================*/
        /*=====================================================================================================*/
        /*==================================== PQ ADJUSTMENT CASE START =======================================*/
        case VDPQ_MOTION:
        {
            unsigned char* table_index_temp;
            unsigned char  table_sel = (ScalerVideo_Status.src_is_tv == TRUE)? 1 : 0;
            unsigned char  motion_status = 0;

            // value changed by motion
            unsigned char global_diff_tolerance = 0;
            unsigned char palerr_hifreq_diff_large_th = 0;
            unsigned char DC_level_change_th_base = 0;
            unsigned char DC_level_change_th_multiple = 0;
            unsigned char FID_match_cnt_th = 0;
            unsigned char FID_offset = 0;
            unsigned char self_diff_small_th = 0;
            unsigned char still_vector_offset_base = 0;
            unsigned char still_vector_offset_multi = 0;
            unsigned char sghf_inter_diff_th = 0;
            unsigned char sghf_global_diff_small_th = 0;
            unsigned char sghf_FID_match_cnt_th = 0;
            unsigned char still_gray_hifreq_low_border = 0;
            unsigned char still_gray_hifreq_th = 0;


            if(ScalerVideo_Status.VD_Mode == VDC_MODE_NTSC) // NTSC
            {
                table_index_temp = NTSC_3D_TABLE +  // NTSC or PAL
                                   VD_3D_TABLE_SIZE*VD_MOTION_STEP*table_sel + // AV or TV
                                   VD_3D_TABLE_SIZE*ScalerVideo_Status.MotionStatus;   // motion_status
            }
            else    //PAL
            {
                table_index_temp = PAL_3D_TABLE + // NTSC or PAL
                                   VD_3D_TABLE_SIZE*VD_MOTION_STEP*table_sel + // AV or TV
                                   VD_3D_TABLE_SIZE*ScalerVideo_Status.MotionStatus; // motion_status
            }

            global_diff_tolerance       = *(table_index_temp+0);
            palerr_hifreq_diff_large_th = *(table_index_temp+1);
            DC_level_change_th_base     = *(table_index_temp+2);
            sghf_inter_diff_th          = *(table_index_temp+3);
            sghf_global_diff_small_th   = *(table_index_temp+4);
            sghf_FID_match_cnt_th       = *(table_index_temp+5);
            FID_match_cnt_th            = *(table_index_temp+6);
            FID_offset                  = *(table_index_temp+7);
            self_diff_small_th          = *(table_index_temp+8);
            still_vector_offset_base    = *(table_index_temp+9);
            still_vector_offset_multi   = *(table_index_temp+10);
            still_gray_hifreq_low_border= *(table_index_temp+11);
            still_gray_hifreq_th        = *(table_index_temp+12);
            DC_level_change_th_multiple = *(table_index_temp+13);
            motion_status               = *(table_index_temp+29);


            vdpq_yc_3d_f4_still_gray_hifreq_1_reg.sghf_inter_diff_th = sghf_inter_diff_th;
            vdpq_yc_3d_f4_still_gray_hifreq_1_reg.sghf_global_diff_small_th = sghf_global_diff_small_th;
            vdpq_yc_3d_f4_still_gray_hifreq_2_reg.sghf_vector_fid_match_cnt_th = sghf_FID_match_cnt_th;
            vdpq_yc_3d_f4_still_vector_1_reg.sv_vector_fid_match_cnt_th = FID_match_cnt_th;
            vdpq_yc_3d_f4_still_vector_1_reg.sv_vector_fid_offset = FID_offset;
            vdpq_yc_3d_f4_still_vector_2_reg.sv_self_diff_small_th = self_diff_small_th;
            vdpq_yc_3d_f4_pair_ctrl_1_reg.still_vector_offset_base = still_vector_offset_base;
            vdpq_yc_3d_f4_pair_ctrl_1_reg.still_vector_offset_multiple = still_vector_offset_multi;
            vdpq_yc_3d_f4_pal_err_com_1_reg.palerr_hifreq_diff_large_th = palerr_hifreq_diff_large_th;
            vdpq_yc_3d_f4_dc_compare_reg.dc_level_change_th_base = DC_level_change_th_base;
            vdpq_yc_3d_f4_dc_compare_reg.dc_level_change_th_multiple = DC_level_change_th_multiple;
            vdpq_yc_3d_f4_pair_ctrl_6_reg.global_diff_tolerance = global_diff_tolerance;
            vdpq_yc_3d_f4_pair_ctrl_5_reg.still_gray_hifreq_low_border = still_gray_hifreq_low_border;
            vdpq_yc_3d_f4_pair_ctrl_5_reg.still_gray_hifreq_th = still_gray_hifreq_th;
            break;
        }
        case VDPQ_FORCE_1D_MODE:
        {
            vdpq_yc_post2dbpf_ctrl_reg.weight_2d_y = 0;
            vdpq_yc_post2dbpf_ctrl_reg.weight_bld_y = 0;
            vdpq_yc_post2dbpf_ctrl_reg.weight_2d_c = 0;
            vdpq_yc_post2dbpf_ctrl_reg.weight_bld_c = 0;
            vdpq_yc_post2dbpf_ctrl_reg.greenish_detected_en = 0;
            vdpq_yc_post2dbpf_ctrl_reg.mghf_narrowbpf_en = 0;

            // 20150825, willy, issue removed from Merlin
            /*
            vdtop_hsync_pulse_start_RBUS vdtop_hsync_pulse_start_reg;
            vdtop_hsync_pulse_start_reg.regValue = rtd_inl(VDTOP_HSYNC_PULSE_START_reg);
            vdtop_hsync_pulse_start_reg.hsync_start = 0x05;
            rtd_outl(VDTOP_HSYNC_PULSE_START_reg, vdtop_hsync_pulse_start_reg.regValue);
            */
            break;
        }
        case VDPQ_ATV_NOISY_SIGNAL:
        {
            unsigned int NoisyLevel = 0;
            for( NoisyLevel=0; NoisyLevel<VD_TOP_NOISE_STATUS; NoisyLevel++ )
            {
              if( ScalerVideo_Status.NoiseStatus >= NOISY_TBL[NoisyLevel][0] )
                break;
            }

			if (NoisyLevel >= VD_TOP_NOISE_STATUS)
				NoisyLevel = VD_TOP_NOISE_STATUS-1;

            // still vector
            vdpq_yc_3d_f4_still_vector_1_reg.sv_vector_fid_match_cnt_th = NOISY_TBL[NoisyLevel][1];
            vdpq_yc_3d_f4_still_vector_1_reg.sv_vector_fid_offset = NOISY_TBL[NoisyLevel][2];
            vdpq_yc_3d_f4_still_vector_2_reg.sv_self_diff_small_th = NOISY_TBL[NoisyLevel][3];
            vdpq_yc_3d_f4_pair_ctrl_1_reg.still_vector_offset_base = NOISY_TBL[NoisyLevel][4];
            vdpq_yc_3d_f4_pair_ctrl_1_reg.still_vector_offset_multiple = NOISY_TBL[NoisyLevel][5];
            // pal error
            vdpq_yc_3d_f4_pal_err_com_1_reg.palerr_accu_tolerance = NOISY_TBL[NoisyLevel][6];
            vdpq_yc_3d_f4_pal_err_com_2_reg.massive_palmode_error_multiple_th = NOISY_TBL[NoisyLevel][7];
            // high frequency offset 10 & 20
            vdpq_yc_3d_f4_global_diff_th_hifreq_offset_3_reg.global_diff_th_hifreq_offset_20 = NOISY_TBL[NoisyLevel][8];
            vdpq_yc_3d_f4_global_diff_th_hifreq_offset_4_reg.global_diff_th_hifreq_offset_10 = NOISY_TBL[NoisyLevel][9];
			// global diff tolerence "offset"
			vdpq_yc_3d_f4_pair_ctrl_6_reg.global_diff_tolerance += NOISY_TBL[NoisyLevel][10];
			// VD CTI
			vdpq_cadap_gain_ctrl_reg.cadap_gain = NOISY_TBL[NoisyLevel][11];
			vdpq_cadap_lut_gain_th_reg.cadap_lut_gain_tha = NOISY_TBL[NoisyLevel][12];
			vdpq_cadap_lut_gain_th_reg.cadap_lut_gain_thb = NOISY_TBL[NoisyLevel][13];
            break;
        }
        case VDPQ_PAL_POSTPROC_MF: // willy 20151123
        {
            unsigned char Motion = ScalerVideo_Status.MotionStatus;
            unsigned char ma0_cnt_th_table[8] = {0, 2, 5, 9, 14, 20, 26, 31};

            // avoid out of range
            if(Motion >= 8)
                Motion = 7;

            vdpq_postp_mf_compensation_ctrl1_reg.ma0_cnt_th = ma0_cnt_th_table[Motion];
			// 20150601 willy, for LG-45,46, grapse
			if( vdpq_postp_mf_compensation_ctrl1_reg.ma0_cnt_th > 4 )
				vdpq_postp_mf_compensation_ctrl1_reg.ma0_cnt_th = 4;

            if(Motion <= 0)
                vdpq_postp_mf_compensation_ctrl1_reg.mf_en = 0;
            else
                vdpq_postp_mf_compensation_ctrl1_reg.mf_en = 0; // 1, HW data path bug fix

            break;
        }
        case VDPQ_PAL_POSTPROC_MF_LG: // willy 20151123
        {
            unsigned char Motion = ScalerVideo_Status.MotionStatus;
            unsigned char ma0_cnt_th_table[8] = {0, 2, 5, 9, 14, 20, 26, 31};

            // avoid out of range
            if(Motion >= 8)
                Motion = 7;

            vdpq_postp_mf_compensation_ctrl1_reg.ma0_cnt_th = ma0_cnt_th_table[Motion];
            // 20150601 willy, for LG-45,46, grapse
            if( vdpq_postp_mf_compensation_ctrl1_reg.ma0_cnt_th > 4 )
                vdpq_postp_mf_compensation_ctrl1_reg.ma0_cnt_th = 4;

            if(Motion <= 0)
				vdpq_postp_mf_compensation_ctrl1_reg.mf_en = 0;
            else
                vdpq_postp_mf_compensation_ctrl1_reg.mf_en = 0; // 1, HW data path bug fix

            break;
        }
        case VDPQ_NTSC_POSTPROC_MF: // willy 20151123
        {
            unsigned char Motion = ScalerVideo_Status.MotionStatus;
            unsigned char ma0_cnt_th_table[8] = {0, 2, 5, 9, 14, 20, 26, 31};

            // avoid out of range
            if(Motion >= 8)
                Motion = 7;

            vdpq_postp_mf_compensation_ctrl1_reg.ma0_cnt_th = ma0_cnt_th_table[Motion];
			// 20150601 willy, for LG-45,46, grapse
			if( vdpq_postp_mf_compensation_ctrl1_reg.ma0_cnt_th > 4 )
				vdpq_postp_mf_compensation_ctrl1_reg.ma0_cnt_th = 4;

            if(Motion <= 0)
				vdpq_postp_mf_compensation_ctrl1_reg.mf_en = 0;
            else
                vdpq_postp_mf_compensation_ctrl1_reg.mf_en = 0; // 1, HW data path bug fix

            break;
        }
        case VDPQ_NTSC_POSTPROC_MF_LG: // willy 20151123
        {
            unsigned char Motion = ScalerVideo_Status.MotionStatus;
            unsigned char ma0_cnt_th_table[8] = {0, 2, 5, 9, 14, 20, 26, 31};

            // avoid out of range
            if(Motion >= 8)
                Motion = 7;

            vdpq_postp_mf_compensation_ctrl1_reg.ma0_cnt_th = ma0_cnt_th_table[Motion];
            // 20150601 willy, for LG-45,46, grapse
            if( vdpq_postp_mf_compensation_ctrl1_reg.ma0_cnt_th > 4 )
                vdpq_postp_mf_compensation_ctrl1_reg.ma0_cnt_th = 4;

            if(Motion <= 0)
				vdpq_postp_mf_compensation_ctrl1_reg.mf_en = 0;
            else
                vdpq_postp_mf_compensation_ctrl1_reg.mf_en = 0; // 1, HW data path bug fix

            break;
        }
        case VDPQ_OTHER_POSTPROC_MF:
        {
            unsigned char Motion = ScalerVideo_Status.MotionStatus;
            unsigned char ma0_cnt_th_table[8] = {0, 2, 5, 9, 14, 20, 26, 31};

            // avoid out of range
            if(Motion >= 8)
                Motion = 7;

            vdpq_postp_mf_compensation_ctrl1_reg.ma0_cnt_th = ma0_cnt_th_table[Motion];
			// 20150601 willy, for LG-45,46, grapse
			if( vdpq_postp_mf_compensation_ctrl1_reg.ma0_cnt_th > 4 )
				vdpq_postp_mf_compensation_ctrl1_reg.ma0_cnt_th = 4;

            if(Motion <= 0)
				vdpq_postp_mf_compensation_ctrl1_reg.mf_en = 0;
            else
                vdpq_postp_mf_compensation_ctrl1_reg.mf_en = 0; // 1, HW data path bug fix

            break;
        }
        case VDPQ_OTHER_POSTPROC_MF_LG:
        {
            unsigned char Motion = ScalerVideo_Status.MotionStatus;
            unsigned char ma0_cnt_th_table[8] = {0, 2, 5, 9, 14, 20, 26, 31};

            // avoid out of range
            if(Motion >= 8)
                Motion = 7;

            vdpq_postp_mf_compensation_ctrl1_reg.ma0_cnt_th = ma0_cnt_th_table[Motion];
            // 20150601 willy, for LG-45,46, grapse
            if( vdpq_postp_mf_compensation_ctrl1_reg.ma0_cnt_th > 4 )
                vdpq_postp_mf_compensation_ctrl1_reg.ma0_cnt_th = 4;

            if(Motion <= 0)
				vdpq_postp_mf_compensation_ctrl1_reg.mf_en = 0;
            else
                vdpq_postp_mf_compensation_ctrl1_reg.mf_en = 0; // 1, HW data path bug fix

            break;
        }
        case VDPQ_PAL_CHROMA_VLPF_ENABLE: // willy 20151123
        {
            vdpq_yc_bw_ctrl_reg.chroma_vlpf_en = 1;
            break;
        }
        case VDPQ_NTSC_CHROMA_VLPF_ENABLE:
        {
            vdpq_yc_bw_ctrl_reg.chroma_vlpf_en = 1;
            break;
        }
        case VDPQ_OTHER_CHROMA_VLPF_ENABLE: // willy 20151123
        {
            vdpq_yc_bw_ctrl_reg.chroma_vlpf_en = 1;
            break;
        }
        case VDPQ_PAL_POST2DBPF_CDIFF: // willy 20151123
        {
            vdpq_yc_post2dbpf_adapth_1_reg.bpfed_c_diff_small_th = 8;
            break;
        }
        case VDPQ_NTSC_POST2DBPF_CDIFF: // willy 20151123
        {
            vdpq_yc_post2dbpf_adapth_1_reg.bpfed_c_diff_small_th = 8;
            break;
        }
        case VDPQ_OTHER_POST2DBPF_CDIFF:
        {
            vdpq_yc_post2dbpf_adapth_1_reg.bpfed_c_diff_small_th = 8;
            break;
        }
        case VDPQ_NTSC_HIGH_SAT_DOTCRAWL_0: // willy, 20151123
        {
            // h2v_lk_en
            vdpq_adap_bpf_c_th1_reg.h2v_lk_en= 0;
            // post 2d bpf y enable and adjust the threshold, 20150818, willy, improve LG 44 2D result for 2D/3D flick
            // 20151117 jimmy, replace post2dbpf enable by blending coeff change
            //vdpq_yc_post2dbpf_ctrl_reg.post2dbpf_y_en = 1;
            vdpq_yc_post2dbpf_ctrl_reg.weight_2d_y = 0;
            vdpq_yc_post2dbpf_ctrl_reg.weight_bld_y = 0;
            //h2v lut
            vdpq_alpha_2d_mod_enable_reg.lut_sel_y = 3;
            //post MF
            vdpq_postp_mf_compensation_ctrl1_reg.cdiff_small_con_en = 0;
            // 1D BPF
            vdpq_blend_1d_thr_reg.bpf_1d_sel_y = 0; // 1D solution is too blur, use narrow BPF, 20151116, willy
            break;
        }
        case VDPQ_PAL_HIGH_SAT_DOTCRAWL_0: // willy 20151123
        {
            // h2v_lk_en
            vdpq_adap_bpf_c_th1_reg.h2v_lk_en= 0;
            // post 2d bpf y enable and adjust the threshold, 20150818, willy, improve LG 44 2D result for 2D/3D flick
            // 20151117 jimmy, replace post2dbpf enable by blending coeff change
            //YC_Post2DBPF_Ctrl_reg.post2dbpf_y_en = 1;
            vdpq_yc_post2dbpf_ctrl_reg.weight_2d_y = 0;
            vdpq_yc_post2dbpf_ctrl_reg.weight_bld_y = 0;
            //h2v lut
            vdpq_alpha_2d_mod_enable_reg.lut_sel_y = 3;
            //post MF
            vdpq_postp_mf_compensation_ctrl1_reg.cdiff_small_con_en = 0;
            // 1D BPF
            //BLEND_1D_THR_reg.bpf_1d_sel_y = 2;
            vdpq_blend_1d_thr_reg.bpf_1d_sel_y = 0; // 1D solution is too blur, use narrow BPF, 20151116, willy
            break;
        }
        case VDPQ_NTSC_HIGH_SAT_DOTCRAWL_1: // willy 20151123
        {
            // h2v_lk_en
            vdpq_adap_bpf_c_th1_reg.h2v_lk_en= 0;
            // post2dbpf_y_en
            // 20151117 jimmy, replace post2dbpf enable by blending coeff change
            //YC_Post2DBPF_Ctrl_reg.post2dbpf_y_en = 1;
            vdpq_yc_post2dbpf_ctrl_reg.weight_2d_y = 2;
            vdpq_yc_post2dbpf_ctrl_reg.weight_bld_y = 2;
            //h2v lut
            vdpq_alpha_2d_mod_enable_reg.lut_sel_y = 0;
            //post MF
            vdpq_postp_mf_compensation_ctrl1_reg.cdiff_small_con_en = 1;
            // 1D BPF
            vdpq_blend_1d_thr_reg.bpf_1d_sel_y = 0;
            break;
        }
        case VDPQ_PAL_HIGH_SAT_DOTCRAWL_1: // willy 20151123
        {
            // h2v_lk_en
            vdpq_adap_bpf_c_th1_reg.h2v_lk_en= 0;
            // post2dbpf_y_en
            // 20151117 jimmy, replace post2dbpf enable by blending coeff change
            //YC_Post2DBPF_Ctrl_reg.post2dbpf_y_en = 1;
            vdpq_yc_post2dbpf_ctrl_reg.weight_2d_y = 2;
            vdpq_yc_post2dbpf_ctrl_reg.weight_bld_y = 2;
            //h2v lut
            vdpq_alpha_2d_mod_enable_reg.lut_sel_y = 0;
            //post MF
            vdpq_postp_mf_compensation_ctrl1_reg.cdiff_small_con_en = 1;
            // 1D BPF
            vdpq_blend_1d_thr_reg.bpf_1d_sel_y = 0;
            break;
        }
        case VDPQ_OTHER_HIGH_SAT_DOTCRAWL_0:
        {
            // h2v_lk_en
            vdpq_adap_bpf_c_th1_reg.h2v_lk_en= 0;
            // post2dbpf_y_en
            // 20151117 jimmy, replace post2dbpf enable by blending coeff change
            //YC_Post2DBPF_Ctrl_reg.post2dbpf_y_en = 0; // post band pass can't in mac2, elieli
            vdpq_yc_post2dbpf_ctrl_reg.weight_2d_y = 0;
            vdpq_yc_post2dbpf_ctrl_reg.weight_bld_y = 0;
            //h2v lut
            vdpq_alpha_2d_mod_enable_reg.lut_sel_y = 3;
            //post MF
            vdpq_postp_mf_compensation_ctrl1_reg.cdiff_small_con_en = 0;
            // 1D BPF
            vdpq_blend_1d_thr_reg.bpf_1d_sel_y = 2;
            break;
        }
        case VDPQ_OTHER_HIGH_SAT_DOTCRAWL_1:
        {
            // h2v_lk_en
            vdpq_adap_bpf_c_th1_reg.h2v_lk_en= 0;
            // post2dbpf_y_en
            // 20151117 jimmy, replace post2dbpf enable by blending coeff change
            //YC_Post2DBPF_Ctrl_reg.post2dbpf_y_en = 0; // post band pass can't in mac2, elieli
            vdpq_yc_post2dbpf_ctrl_reg.weight_2d_y = 0;
            vdpq_yc_post2dbpf_ctrl_reg.weight_bld_y = 0;
            //h2v lut
            vdpq_alpha_2d_mod_enable_reg.lut_sel_y = 0;
            //post MF
            vdpq_postp_mf_compensation_ctrl1_reg.cdiff_small_con_en = 1;
            // 1D BPF
            vdpq_blend_1d_thr_reg.bpf_1d_sel_y = 0;
            break;
        }
        case VDPQ_NTSC_VIP_DOS_107:
        {
            vdpq_bpf_bw_sel_y_reg.mid_bpf_sel_y = 5;    // 20151214, willy, LG logo dot-crawl reduce
            vdpq_bpf_bw_sel_y_reg.narrow_bpf_sel_y = 0; // 20151115, willy, avoid second horizontal scroll text characters blur and stick together

            //-----------------------------------------------------------------------------
            // 20150924, willy, vertical scrolling text stable
            vdpq_yc_3d_f4_smd_en_reg.smd_ver_06_move_enable = 1;
            vdpq_yc_3d_f4_smd_en_reg.smd_ver_08_move_enable = 1;
            vdpq_yc_3d_f4_smd_th1_reg.smd_ver_spatial_temporal_traily_diff = 20;
            vdpq_yc_3d_f4_smd_en_reg.smd_hor_06_move_enable = 1;
            vdpq_yc_3d_f4_smd_en_reg.smd_hor_08_move_enable = 1;
            vdpq_yc_3d_f4_smd_th1_reg.smd_hor_spatial_temporal_traily_diff = 20;
            vdpq_yc_3d_f4_smd_th1_reg.smd_horizontal_sad_tolerance = 0;
            vdpq_yc_3d_f4_smd_th1_reg.smd_vertical_sad_tolerance = 30;
            vdpq_yc_3d_f4_smd_th2_reg.smd_min_sad_horizontal = 0;
            vdpq_yc_3d_f4_smd_th2_reg.smd_min_sad_vetical = 80;
            vdpq_yc_3d_f4_pair_smd_th1_reg.smd_highfreq_diff_th = 50;
            //-----------------------------------------------------------------------------

			// 20161207, willy, Vertical scrolling text
			vdpq_yc_3d_f4_pair_ctrl_4_reg.diff_th_upper_bound = 120;

            // 20151213, willy, reduce the dot crawl by enabling the 2D post LPF
            vdpq_yc2d_postlp_ctrl_reg.postlpf_en = 1;
            vdpq_yc_post2dbpf_ctrl_reg.weight_2d_y = 0;
            vdpq_yc_post2dbpf_ctrl_reg.weight_bld_y = 0;

            // 20151007, willy, reduce horizontal scrolling text dot crawl
            vdpq_xnr_ctrl_reg.xnr_lpf_sel = 0;

            // 20151214, willy, adjust the 2D bpf selection to distinguish the LG logo and scrolling text
            vdpq_cdiff_adap_bpf_th2_reg.cdiff_bpf_large_amp_wei = 5;
            vdpq_cdiff_adap_bpf_th2_reg.cdiff_bpf_large_offset = 0;

			// 20160121, willy, change c BPF and reduce deXC threshold to avoid color boundary blur
			// use post 2D BPF C and deXC to avoid cross color at first yellow horizontal scrolling text
			vdpq_de_xc_ctrl_reg.de_xc_en = 1;
			vdpq_de_xc_ctrl_reg.de_xc_thr = 15;
			vdpq_bpf_bw_sel_c_reg.mid_bpf_sel_c = 5;
			vdpq_yc_post2dbpf_thbase_reg.hor_cdiff_th_base = 90;
			vdpq_yc_post2dbpf_thbase_reg.ver_cdiff_th_base = 60;
			vdpq_yc_post2dbpf_adapth_1_reg.bpfed_c_diff_small_th = 30;

            // 20150122, willy, adjust the 2d/3d alpha blending ratio to make vertical scrolling text into 3D
            vdpq_yc_3d_f4_spa_cnt_cvbs_amp_reg.f4_3d_spatial_cnt_th_1 = 1;
            vdpq_yc_3d_f4_spa_cnt_cvbs_amp_reg.f4_3d_spatial_cnt_th_2 = 4;

			// 20160705, willy, blending ratio change control register
			vdpq_alpha_blending_ctrl1_reg.alpha2d3d_bld_weight_1 = 0;
			vdpq_alpha_blending_ctrl1_reg.alpha2d3d_bld_weight_2 = 4;
			vdpq_alpha_blending_ctrl1_reg.alpha2d3d_bld_weight_3 = 4;
			vdpq_alpha_blending_ctrl1_reg.alpha2d3d_bld_weight_4 = 4;
			vdpq_alpha_blending_ctrl2_reg.alpha2d3d_bld_weight_5 = 8;
			vdpq_alpha_blending_ctrl2_reg.alpha2d3d_bld_weight_6 = 8;
			vdpq_alpha_blending_ctrl2_reg.alpha2d3d_bld_weight_7 = 8;
			vdpq_alpha_blending_ctrl2_reg.alpha2d3d_bld_weight_8 = 8;
			vdpq_alpha_blending_ctrl3_reg.alpha2d3d_bld_weight_9 = 8;
			vdpq_alpha_blending_ctrl3_reg.alpha2d3d_bld_weight_10 = 12;
			vdpq_alpha_blending_ctrl3_reg.alpha2d3d_bld_weight_11 = 12;
			vdpq_alpha_blending_ctrl3_reg.alpha2d3d_bld_weight_12 = 14;
			vdpq_alpha_blending_ctrl4_reg.alpha2d3d_bld_weight_13 = 14;
			vdpq_alpha_blending_ctrl4_reg.alpha2d3d_bld_weight_14 = 16;
			vdpq_alpha_blending_ctrl4_reg.alpha2d3d_bld_weight_15 = 16;
            break;
        }
        case VDPQ_NTSC_HIGH_SAT_DIVERSE_HUE:
        {
            vdpq_yc_3d_f4_pair_ctrl_6_reg.global_diff_tolerance = 3;
            break;
        }
        case VDPQ_PAL_HIGH_SAT_DIVERSE_HUE:
        {
            vdpq_de_xc_ctrl_reg.de_xc_thr = 10;
            break;
        }
        case VDPQ_NTSC_VIP_DOS_2:
        {
			vdpq_yc_sep_mode_ctrl_reg.yc_sep_mode = 2;

            // chroma
            vdpq_yc_bw_ctrl_reg.chroma_bw_lo = 7;
            vdpq_yc_bw_ctrl_reg.chroma_vlpf_en = 1;
            // 20150126, willy, improve 2D solution for explaination
            vdpq_bpf_bw_sel_y_reg.wide_bpf_sel_y = 0;
            vdpq_bpf_bw_sel_y_reg.mid_bpf_sel_y = 0;
            vdpq_bpf_bw_sel_y_reg.narrow_bpf_sel_y = 0;
            break;
        }
        case VDPQ_NTSC_DOS_2_TNR_XC:
        {
            // TNRXC on
            di_im_di_tnr_xc_control_reg.cp_temporal_xc_en = 1;
            // temporalenable must follow cp_temporal_xc_en
            di_im_di_control_reg.cp_temporalenable = di_im_di_tnr_xc_control_reg.cp_temporal_xc_en;
	    di_im_di_control_reg.write_enable_6 =1;
            break;
        }
        case VDPQ_PAL_VIP_DOS_2:
        {
            // narrow 2D BPF
            vdpq_bpf_bw_sel_y_reg.narrow_bpf_sel_y = 0;
            // 20151024 jimmy, for hanging bed in PAL
            // DBWline enable for PAL hanging bed
            vdpq_yc_post2dbpf_dbwline_1_reg.dbwline_en = 1;
            // xnr off
            vdpq_xnr_ctrl_reg.xnr_lpf_sel = 0;
            // Make lines go into 2D, weaken SGHF
            // still_gray_hifreq_th = 15
            vdpq_yc_3d_f4_pair_ctrl_5_reg.still_gray_hifreq_th = 15;

            // strong post 2d bpf y setting, willy 20151113
            vdpq_yc_post2dbpf_thbase_reg.hor_cdiff_th_base = 120;
            vdpq_yc_post2dbpf_thbase_reg.ver_cdiff_th_base = 120;
            vdpq_yc_post2dbpf_adapth_13_reg.ignore_th_y = 0;

            // disable V2V Y for diagonal line stable
            vdpq_adap_bpf_c_th1_reg.v2v_lk_y_en = 0;

            // 20160124, willy, make small movement region take more 3D solution to stablize the shaking
            vdpq_yc_3d_f4_spa_cnt_cvbs_amp_reg.f4_3d_spatial_cnt_th_1 = 0;
            vdpq_yc_3d_f4_spa_cnt_cvbs_amp_reg.f4_3d_spatial_cnt_th_2 = 0;


			// 20160705, willy, blending ratio change control register
			vdpq_alpha_blending_ctrl1_reg.alpha2d3d_bld_weight_1  = 0;
			vdpq_alpha_blending_ctrl1_reg.alpha2d3d_bld_weight_2  = 0;
			vdpq_alpha_blending_ctrl1_reg.alpha2d3d_bld_weight_3  = 0;
			vdpq_alpha_blending_ctrl1_reg.alpha2d3d_bld_weight_4  = 0;
			vdpq_alpha_blending_ctrl2_reg.alpha2d3d_bld_weight_5  = 0;
			vdpq_alpha_blending_ctrl2_reg.alpha2d3d_bld_weight_6  = 0;
			vdpq_alpha_blending_ctrl2_reg.alpha2d3d_bld_weight_7  = 0;
			vdpq_alpha_blending_ctrl2_reg.alpha2d3d_bld_weight_8  = 0;
			vdpq_alpha_blending_ctrl3_reg.alpha2d3d_bld_weight_9  = 12;
			vdpq_alpha_blending_ctrl3_reg.alpha2d3d_bld_weight_10 = 12;
			vdpq_alpha_blending_ctrl3_reg.alpha2d3d_bld_weight_11 = 14;
			vdpq_alpha_blending_ctrl3_reg.alpha2d3d_bld_weight_12 = 14;
			vdpq_alpha_blending_ctrl4_reg.alpha2d3d_bld_weight_13 = 16;
			vdpq_alpha_blending_ctrl4_reg.alpha2d3d_bld_weight_14 = 16;
			vdpq_alpha_blending_ctrl4_reg.alpha2d3d_bld_weight_15 = 16;
            break;
        }
        case VDPQ_PAL_DOS_2_TNR_XC:
        {
            // TNRXC on
            di_im_di_tnr_xc_control_reg.cp_temporal_xc_en = 1;
            // temporalenable must follow cp_temporal_xc_en
            di_im_di_control_reg.cp_temporalenable = di_im_di_tnr_xc_control_reg.cp_temporal_xc_en;
	    di_im_di_control_reg.write_enable_6 =1;
            break;
        }
        case VDPQ_PAL_LOW_SAT:
        {
            // 2D Y Post BPF
            vdpq_bpf_bw_sel_y_reg.narrow_bpf_sel_y = 0;
            vdpq_bpf_bw_sel_y_reg.mid_bpf_sel_y = 10;
            break;
        }
        case VDPQ_NTSC_HIGH_SAT_DIVERSE_HUE_DOTCRAWL:
        {
            vdpq_h2v_y_noise_thr_reg.h2v_y_noise_thr = 0xf;// 0x17;
            break;
        }
        case VDPQ_PAL_HIGH_SAT_DIVERSE_HUE_DOTCRAWL:
        {
            vdpq_bpf_bw_sel_y_reg.narrow_bpf_sel_y = 5;
            vdpq_postp_3d2dalpha_blend_ctrl_1_reg.xnr_alpha_2d_weight = 4;
            vdpq_postp_3d2dalpha_blend_ctrl_1_reg.xnr_alpha_bld_weight = 2;
            vdpq_xnr_ctrl_reg.xnr_lpf_sel = 3;
            vdpq_xnr_ctrl_reg.xnr_vlpf_sel = 3;
            break;
        }
        case VDPQ_NTSC_LG_FAST_MOVING_LOW_SAT:
        {
            vdpq_bpf_bw_sel_y_reg.narrow_bpf_sel_y = 5;
            // 20151117 jimmy, replace post2dbpf enable by blending coeff change
            vdpq_yc_post2dbpf_ctrl_reg.weight_2d_y = 0;
            vdpq_yc_post2dbpf_ctrl_reg.weight_bld_y = 0;
            break;
        }
        case VDPQ_NTSC_LEAF_MAGENTA_GREEN_CIRCLE:
        {
            // diff_th_upper_bound=90
            vdpq_yc_3d_f4_pair_ctrl_4_reg.diff_th_upper_bound = 90;
            // still vector
            vdpq_yc_3d_f4_still_vector_1_reg.sv_vector_fid_match_cnt_th = 7;
            vdpq_yc_3d_f4_still_vector_1_reg.sv_vector_fid_offset = 5;
            vdpq_yc_3d_f4_still_vector_2_reg.sv_self_diff_small_th = 40;
            vdpq_yc_3d_f4_pair_ctrl_1_reg.still_vector_offset_base = 63;
            vdpq_yc_3d_f4_pair_ctrl_1_reg.still_vector_offset_multiple = 7;
            vdpq_adap_bpf_c_th1_reg.h2v_lk_en = 1;
            vdpq_adap_bpf_c_th1_reg.h2v_mid_sel_y = 1;
            vdpq_bpf_bw_sel_c_reg.narrow_bpf_sel_c = 2;
            // avoid color blur
            vdpq_de_xc_ctrl_reg.de_xc_en = 0;
            vdpq_yc_bw_ctrl_reg.chroma_vlpf_en = 0;
            break;
        }
        case VDPQ_PAL_LEAF_MAGENTA_GREEN_CIRCLE:
        {
            // 20160113, jimmy, LG leaf #42 reduce dot-crawl
            // h2v_lk_en on
            vdpq_adap_bpf_c_th1_reg.h2v_lk_en= 1;
            vdpq_adap_bpf_c_th1_reg.h2v_mid_sel_y = 1; // 20160122 willy
            // bpf_y narrow/mid/wide = 5/2/5
            vdpq_bpf_bw_sel_y_reg.mid_bpf_sel_y = 2;
            vdpq_bpf_bw_sel_y_reg.narrow_bpf_sel_y = 5;
            // bpf_c narrow/mid/wide = 4/5/5
            vdpq_bpf_bw_sel_c_reg.narrow_bpf_sel_c = 5;
            vdpq_bpf_bw_sel_c_reg.mid_bpf_sel_c = 5;

            // de_xc & chroma_vlpf off
            vdpq_de_xc_ctrl_reg.de_xc_en = 0;
            vdpq_yc_bw_ctrl_reg.chroma_vlpf_en = 0;

            // xnr 2D weight = 4
            vdpq_postp_3d2dalpha_blend_ctrl_1_reg.xnr_alpha_2d_weight = 4;

            break;
        }
        case VDPQ_NTSC_MOVING_CAN:
        {
            vdpq_bpf_bw_sel_y_reg.narrow_bpf_sel_y = 5;
            // change the xnr lpf to stable the 2D solution, 20151007, willy
            vdpq_xnr_ctrl_reg.xnr_lpf_sel = 3;
            // 20151210, reduce dot-crawl by xNR and enhance edge by DLTI
            vdpq_alpha_2d_mod_enable_reg.force_halpha_en = 1; //stablize the diagonal-edge of the yellow mustard can
            vdpq_yc_3d_f4_smd_en_reg.smd_en = 0;
            // 20150923 willy, disable for less dot crawl
            // 20151117 jimmy, replace post2dbpf enable by blending coeff change
            //vdpq_yc_post2dbpf_ctrl_reg.post2dbpf_y_en = 0;
            vdpq_yc_post2dbpf_ctrl_reg.weight_2d_y = 0;
            vdpq_yc_post2dbpf_ctrl_reg.weight_bld_y = 0;
            vdpq_yc_post2dbpf_adapth_1_reg.bpfed_c_diff_small_th = 3;
            // More 2D to avoid 2D/3D flick, and blur by XNR, 20151109 willy
            vdpq_yc_3d_f4_pair_ctrl_4_reg.diff_th_upper_bound = 6;
            vdpq_yc_3d_f4_pair_ctrl_4_reg.diff_th_lower_bound = 1;
            // 20151210, willy, supress the cross color
            vdpq_de_xc_ctrl_reg.de_xc_en = 1;
            vdpq_yc_bw_ctrl_reg.chroma_vlpf_en = 1;
            // 20151007, willy, reduce the dot crawl at ramen
            vdpq_h2v_y_noise_thr_reg.h2v_y_noise_thr = 15;
            // 20151007, willy, solve the yellow mustard characters sudden blur issue
            vdpq_yc_3d_f4_still_vector_1_reg.sv_vector_fid_match_cnt_th = 24;
            // 20151217, willy, because the default mid C bpf is change to 0, set to 5 to avoid edge color blur
            vdpq_bpf_bw_sel_c_reg.mid_bpf_sel_c = 5;

            // 20160105, jimmy, weaken dexc to improve color blur
            vdpq_de_xc_ctrl_reg.de_xc_thr = 12;

            /*
            // 20150924, willy, 2d post lpf cause blur edge and color, disabled by willy
            vdpq_yc2d_postlp_ctrl_reg.postlpf_en = 0;
            vdpq_yc2d_postlp_flat_1_reg.low_amp_th_0 = 12;
            vdpq_yc2d_postlp_flat_1_reg.low_amp_th_1 = 14;
            vdpq_yc2d_postlp_flat_1_reg.low_amp_th_2 = 17;
            vdpq_yc2d_postlp_flat_1_reg.low_amp_th_3 = 22;
            vdpq_yc2d_postlp_flat_2_reg.low_amp_th_4 = 25;
            vdpq_yc2d_postlp_flat_2_reg.hdiff_th_0 = 3;
            vdpq_yc2d_postlp_flat_2_reg.hdiff_th_1 = 10;
            vdpq_yc2d_postlp_flat_2_reg.hdiff_th_2 = 14;
            vdpq_yc2d_postlp_flat_3_reg.hdiff_th_3 = 18;
            vdpq_yc2d_postlp_flat_3_reg.hdiff_th_4 = 21;
            vdpq_yc2d_postlp_flat_3_reg.vdiff_th_0 = 1;
            vdpq_yc2d_postlp_flat_3_reg.vdiff_th_1 = 1;
            vdpq_yc2d_postlp_flat_4_reg.vdiff_th_2 = 2;
            vdpq_yc2d_postlp_flat_4_reg.vdiff_th_3 = 6;
            vdpq_yc2d_postlp_flat_4_reg.vdiff_th_4 = 8;
            */
            break;
        }
        case VDPQ_PAL_MOVING_CAN:
        {
            // change from 4 to 5 for the word clearity on tomto can "POMODORI", 20151115, willy
            vdpq_bpf_bw_sel_y_reg.narrow_bpf_sel_y = 5;
            vdpq_yc_post2dbpf_adapth_1_reg.bpfed_c_diff_small_th = 3;
            vdpq_adap_bpf_c_th1_reg.h2v_lk_en = 1;
            vdpq_adap_bpf_c_th1_reg.v2v_lk_y_en = 0; // to stablize the line on coke can, 20151116, willy
            vdpq_comb_filter_threshold1_reg.noise_ntsc_c = 90;
            // ---------------------------------------------------------------------------
            // Setting for Horizontal PAN
            // More 2D to avoid 2D/3D flick, and blur by XNR, 20151109 willy
            vdpq_yc_3d_f4_pair_ctrl_4_reg.diff_th_upper_bound = 6;
            vdpq_yc_3d_f4_pair_ctrl_4_reg.diff_th_lower_bound = 1;
            vdpq_xnr_thr_reg.xnr_cdiff_thr = 0;
            vdpq_xnr_thr_reg.xnr_csum_thr = 0;
            // ---------------------------------------------------------------------------
            vdpq_yc_bw_ctrl_reg.chroma_bw_lo = 1; // cannot apply too strong, make color boundary blur, 20151117, willy
            // ---------------------------------------------------------------------------
            // 2D post LPF along the diagonal edge => yellow mustard, 20151116, willy
            vdpq_yc2d_postlp_ctrl_reg.postlpf_grad_en = 1;
            // adjust the threshold to avoid the blur on SOJU can edge
            vdpq_yc2d_postlp_grad_1_reg.larger_cnt_th_0 = 5;
            // ---------------------------------------------------------------------------
            // 20151217, willy, because the default mid C bpf is change to 0, set to 5 to avoid edge color blur
            vdpq_bpf_bw_sel_c_reg.mid_bpf_sel_c = 4;

			// 20160112, jimmy, weaken dexc to improve color blur
            vdpq_de_xc_ctrl_reg.de_xc_thr = 9;
            break;
        }
        case VDPQ_NTSC_DEXC:
        {
            //vdpq_yc_3d_f4_pair_ctrl_6_reg.global_diff_tolerance = 3;
            vdpq_de_xc_ctrl_reg.de_xc_en = 0;
            break;
        }
        case VDPQ_NTSC_TNR_XC:
        {
            // DI TNR_XC
            di_im_di_tnr_xc_control_reg.cp_temporal_xc_en = 0;
            // 20150805 roger, temporalenable must follow cp_temporal_xc_en
            di_im_di_control_reg.cp_temporalenable = di_im_di_tnr_xc_control_reg.cp_temporal_xc_en;
	    di_im_di_control_reg.write_enable_6=1;
            break;
        }
        case VDPQ_PAL_DEXC:
        {
            vdpq_de_xc_ctrl_reg.de_xc_en = 0;
            break;
        }
        case VDPQ_PAL_TNR_XC:
        {
            di_im_di_tnr_xc_control_reg.cp_temporal_xc_en = 0;

            // 20150805 roger, temporalenable must follow cp_temporal_xc_en
            di_im_di_control_reg.cp_temporalenable = di_im_di_tnr_xc_control_reg.cp_temporal_xc_en;
	    di_im_di_control_reg.write_enable_6=1;
            break;
        }
		case VDPQ_PAL_TNR_XC_LGTVPALBGE52:
		{
			di_im_di_tnr_xc_control_reg.cp_temporal_xc_en = 1;

			// 20150805 roger, temporalenable must follow cp_temporal_xc_en
			di_im_di_control_reg.cp_temporalenable = di_im_di_tnr_xc_control_reg.cp_temporal_xc_en;
			di_im_di_control_reg.write_enable_6=1;
			break;
		}
        case VDPQ_NTSC_2D3D_JUMP:
        {
            // 20151008, willy, improve 2d performance to avoid 2d 3d jump
            vdpq_yc_post2dbpf_thbase_reg.hor_cdiff_th_base = 120;

            /*
            vdpq_yc_3d_f4_still_vector_1_reg.sv_vector_large_cnt_th = 20;
            vdpq_yc_3d_f4_still_vector_1_reg.sv_vector_large_th = 25;
            */
            break;
        }
        case VDPQ_NTSC_VIP_DOS_12:
        {
            // 1. diff_th_upper_bound=110 from 0x38
            vdpq_yc_3d_f4_pair_ctrl_4_reg.diff_th_upper_bound = 110;
            //---------------------------------------------------------------------------
            // setting copied from STRONG_VD_SMD, willy 20151113
            vdpq_yc_3d_f4_smd_en_reg.smd_ver_00_move_enable = 1;
            vdpq_yc_3d_f4_smd_en_reg.smd_ver_06_move_enable = 1;
            vdpq_yc_3d_f4_smd_en_reg.smd_ver_08_move_enable = 1;
            vdpq_yc_3d_f4_smd_th1_reg.smd_ver_spatial_temporal_traily_diff = 0;
            vdpq_yc_3d_f4_smd_en_reg.smd_hor_00_move_enable = 1;
            vdpq_yc_3d_f4_smd_en_reg.smd_hor_06_move_enable = 1;
            vdpq_yc_3d_f4_smd_en_reg.smd_hor_08_move_enable = 1;
            vdpq_yc_3d_f4_smd_th1_reg.smd_hor_spatial_temporal_traily_diff = 0;

            //---------------------------------------------------------------------------
            // supress the cross color for strong 3D setting in slow motion scene, willy 20151113
            vdpq_yc_bw_ctrl_reg.chroma_bw_lo = 7;
            vdpq_yc_bw_ctrl_reg.chroma_vlpf_en = 1;
            break;
        }
        case VDPQ_PAL_VIP_DOS_12:
        {
            // 1. diff_th_upper_bound=110 from 0x38
            vdpq_yc_3d_f4_pair_ctrl_4_reg.diff_th_upper_bound = 110;
            vdpq_bpf_bw_sel_y_reg.narrow_bpf_sel_y = 0;
            vdpq_xnr_ctrl_reg.xnr_lpf_sel = 0;
            // supress the cross color for strong 3D setting in slow motion scene, willy 20151113
            vdpq_yc_bw_ctrl_reg.chroma_bw_lo = 7;
            vdpq_yc_bw_ctrl_reg.chroma_vlpf_en = 1;
            break;
        }
        case VDPQ_NTSC_SMD_0:
        {
            vdpq_bpf_bw_sel_y_reg.narrow_bpf_sel_y = 0;
            break;
        }
        case VDPQ_NTSC_SMD_1:
        {
            vdpq_yc_3d_f4_still_gray_hifreq_1_reg.sghf_inter_diff_th = 60;
            vdpq_yc_3d_f4_still_gray_hifreq_1_reg.sghf_global_diff_small_th = 100;
            vdpq_yc_3d_f4_still_gray_hifreq_2_reg.sghf_vector_fid_match_cnt_th = 8;
            break;
        }
        case VDPQ_NTSC_SMD_IRON_TOWER:
        {
            // special setting for DOS#39, 20151113, willy
            vdpq_yc_3d_f4_still_gray_hifreq_1_reg.sghf_global_diff_small_th = 80;
            vdpq_yc_3d_f4_still_gray_hifreq_1_reg.sghf_longtime_en = 1;
            vdpq_yc_3d_f4_still_gray_hifreq_2_reg.sghf_vector_large_th = 12;
            break;
        }
        case VDPQ_PAL_IRON_TOWER:
        {
            // to stable the iron tower vertical line
            vdpq_adap_bpf_c_th1_reg.v2v_lk_y_en = 0;
            vdpq_yc_3d_f4_pair_ctrl_4_reg.diff_th_upper_bound = 5;
            vdpq_yc_post2dbpf_thbase_reg.hor_cdiff_th_base = 120;
            break;
        }
        case VDPQ_PAL_SMD_GRASS_BOAT: // willy 20151123
        {
            // 20151029, re-applied for PAL case, jimmy
            vdpq_yc_3d_f4_still_vector_1_reg.sv_vector_fid_match_cnt_th = 9;
            vdpq_yc_3d_f4_still_vector_1_reg.sv_vector_fid_offset = 5;
            vdpq_yc_3d_f4_still_vector_2_reg.sv_self_diff_small_th = 70;
            vdpq_yc_3d_f4_pair_ctrl_1_reg.still_vector_offset_base = 60;
            vdpq_yc_3d_f4_pair_ctrl_1_reg.still_vector_offset_multiple = 7;
            // make SGHF_th stronger too, jimmy 20151029
            vdpq_yc_3d_f4_pair_ctrl_5_reg.still_gray_hifreq_th = 90;
            break;
        }
        case VDPQ_NTSC_SMD_2:
        {
            vdpq_yc_3d_f4_still_vector_1_reg.sv_vector_fid_match_cnt_th = 9;
            vdpq_yc_3d_f4_still_vector_1_reg.sv_vector_fid_offset = 5;
            vdpq_yc_3d_f4_still_vector_2_reg.sv_self_diff_small_th = 40;
            vdpq_yc_3d_f4_pair_ctrl_1_reg.still_vector_offset_base = 60;
            vdpq_yc_3d_f4_pair_ctrl_1_reg.still_vector_offset_multiple = 7;
            break;
        }
        case VDPQ_NTSC_SUN_FLOWER_ROOF:
        {
            vdpq_yc_3d_f4_still_gray_hifreq_1_reg.sghf_inter_diff_th = 110;
            vdpq_yc_3d_f4_still_gray_hifreq_1_reg.sghf_global_diff_small_th = 185;
            vdpq_yc_3d_f4_still_gray_hifreq_2_reg.sghf_vector_fid_match_cnt_th = 8;
            vdpq_yc_3d_f4_pair_ctrl_5_reg.still_gray_hifreq_th = 108;
            break;
        }
        case VDPQ_PAL_SUN_FLOWER_ROOF:
        {
            vdpq_yc_3d_f4_still_gray_hifreq_1_reg.sghf_inter_diff_th = 110;
            vdpq_yc_3d_f4_still_gray_hifreq_1_reg.sghf_global_diff_small_th = 185;
            vdpq_yc_3d_f4_still_gray_hifreq_2_reg.sghf_vector_fid_match_cnt_th = 8;
            vdpq_yc_3d_f4_pair_ctrl_5_reg.still_gray_hifreq_th = 108;
            break;
        }
        case VDPQ_NTSC_VIP_DOS_7:
        {
            /*
            // Setting from scalerVIP.c
            // fix 3D by adjusting the alpha count threshold, 20151005, willy
            vdpq_yc_3d_f4_spa_cnt_cvbs_amp_reg.f4_3d_spatial_cnt_th_5 = 0; // fix 3D
            // to reduce the cross color
            vdpq_yc_bw_ctrl_reg.chroma_vlpf_en = 1;
            vdpq_yc_bw_ctrl_reg.chroma_bw_lo = 7;
            vdpq_de_xc_ctrl_reg.de_xc_en = 1;
            */
            // h2v_lk_en
            vdpq_adap_bpf_c_th1_reg.h2v_lk_en = 1;
            vdpq_yc_3d_f4_pair_ctrl_4_reg.diff_th_upper_bound = 10;
            break;
        }
        case VDPQ_PAL_VIP_DOS_7:
        {
            vdpq_yc_bw_ctrl_reg.chroma_vlpf_en = 1;
            // ---------------------------------------------------------------------------
            // HPAN Scene, more 2D to avoid 2D/3D flick, and blur by XNR, 20151109 willy
            vdpq_yc_3d_f4_pair_ctrl_4_reg.diff_th_upper_bound = 6;
            vdpq_yc_3d_f4_pair_ctrl_4_reg.diff_th_lower_bound = 1;
            vdpq_yc_3d_f4_pair_ctrl_6_reg.dc_level_fierce_change_th = 1;
            vdpq_xnr_thr_reg.xnr_cdiff_thr = 0;
            vdpq_xnr_thr_reg.xnr_csum_thr = 0;
            // ---------------------------------------------------------------------------
            // make the left bot marker clear
            vdpq_bpf_bw_sel_y_reg.mid_bpf_sel_y = 0;
            // 20151218, willy, disable the 2d post lpf to avoid blur
            vdpq_yc2d_postlp_ctrl_reg.postlpf_en = 0;
            break;
        }
        case VDPQ_NTSC_VIP_DOS_1:
        {
            // Setting from scalerVIP.c
            vdpq_adap_bpf_y_th3_reg.malpha_y_en = 1;
            vdpq_adap_bpf_y_th3_reg.malpha_y = 1;
            break;
        }
        case VDPQ_NTSC_VIP_DOS_123:
        {
            vdpq_yc_bw_ctrl_reg.chroma_vlpf_en = 0;
            vdpq_yc_3d_f4_pair_ctrl_6_reg.global_diff_tolerance = 5;
            break;
        }
        case VDPQ_PAL_VIP_DOS_123:
        {
            // 20151202 jimmy, avoid the color blur
            vdpq_yc_bw_ctrl_reg.chroma_vlpf_en = 0;
            // 20151225, willy, eliminate dot-crawl by making all the region into 3D by SMD and global diff tolerance
            vdpq_yc_3d_f4_pair_ctrl_6_reg.global_diff_tolerance = 15;

            //------------------------------------------------------------------------
            // with original filter coef. [1710, 1710, 424, 1732, 196]
            vdpq_yc_3d_f4_smd_en_reg.smd_en = 1;
            vdpq_yc_3d_f4_smd_en_reg.smd_ver_00_move_enable = 1;
            vdpq_yc_3d_f4_smd_th1_reg.smd_ver_spatial_temporal_traily_diff = 0;
            vdpq_yc_3d_f4_smd_en_reg.smd_hor_00_move_enable = 1;
            vdpq_yc_3d_f4_smd_th1_reg.smd_hor_spatial_temporal_traily_diff = 0;
            vdpq_yc_3d_f4_pair_smd_th1_reg.smd_2d_small_global_diff_offset = 1024;
            //------------------------------------------------------------------------
            break;
        }
        case VDPQ_NTSC_VIP_DOS_139:
        {
            // ---------------------------------------------------------------------------
            // 20151209, willy, this setting can avoid fix 3D, but dot crawl appears when FW write register
            // improve 2D solution
            vdpq_bpf_bw_sel_y_reg.narrow_bpf_sel_y = 0;
            vdpq_bpf_bw_sel_y_reg.mid_bpf_sel_y = 0;
            vdpq_bpf_bw_sel_y_reg.wide_bpf_sel_y = 0;
            vdpq_yc_3d_f4_pair_ctrl_6_reg.global_diff_tolerance = 15;
            vdpq_yc_post2dbpf_thbase_reg.hor_cdiff_th_base = 120;
            vdpq_yc_post2dbpf_thbase_reg.ver_cdiff_th_base = 120;
            vdpq_bypass_bpf_ctrl2_reg.newvedge_vdiff_th = 0;
            // tend to do 3D
            vdpq_yc_3d_f4_smd_en_reg.smd_ver_00_move_enable = 1;
            vdpq_yc_3d_f4_smd_en_reg.smd_ver_06_move_enable = 1;
            vdpq_yc_3d_f4_smd_en_reg.smd_ver_08_move_enable = 1;
            vdpq_yc_3d_f4_smd_en_reg.smd_ver_10_move_enable = 1;
            vdpq_yc_3d_f4_smd_th1_reg.smd_ver_spatial_temporal_traily_diff = 0;
            vdpq_yc_3d_f4_smd_en_reg.smd_hor_00_move_enable = 1;
            vdpq_yc_3d_f4_smd_en_reg.smd_hor_06_move_enable = 1;
            vdpq_yc_3d_f4_smd_en_reg.smd_hor_08_move_enable = 1;
            vdpq_yc_3d_f4_smd_en_reg.smd_hor_10_move_enable = 1;
            vdpq_yc_3d_f4_smd_th1_reg.smd_hor_spatial_temporal_traily_diff = 0;
            vdpq_yc_3d_f4_smd_th1_reg.smd_horizontal_sad_tolerance = 50;
            vdpq_yc_3d_f4_smd_th1_reg.smd_vertical_sad_tolerance = 50;
            vdpq_yc_3d_f4_smd_th2_reg.smd_min_sad_horizontal = 50;
            vdpq_yc_3d_f4_smd_th2_reg.smd_min_sad_vetical = 50;
            // ---------------------------------------------------------------------------
            break;
        }
        case VDPQ_PAL_VIP_DOS_139:
        {
            // 20151209, willy, tuning the threshold to make it 3D
            vdpq_yc_3d_f4_pair_ctrl_6_reg.global_diff_tolerance = 15;
            vdpq_yc_3d_f4_pair_ctrl_3_reg.pair_diff_sel_th = 0;
            /*
            // 20151209, willy, when change back to menu, will see the 3D output status, NG
            // fix 3D by adjusting the alpha count threshold, 20151005, willy
            vdpq_yc_3d_f4_spa_cnt_cvbs_amp_reg.f4_3d_spatial_cnt_th_5 = 0; // fix 3D
            */
            break;
        }
        case VDPQ_PAL_VIP_DOS_11:
        {
            vdpq_yc_3d_f4_pair_ctrl_6_reg.global_diff_tolerance = 15;
            break;
        }
        case VDPQ_NTSC_VIP_DOS_11:
        {
            // global diff tolerence = 5
            vdpq_yc_3d_f4_pair_ctrl_6_reg.global_diff_tolerance = 15;
            // fix the c 2D direction to avoid cross color
            vdpq_adap_bpf_y_th3_reg.malpha_c_en = 1;
            vdpq_adap_bpf_y_th3_reg.malpha_c = 1;
            break;
        }
        case VDPQ_NTSC_VIP_DOS_28:
        {
            // narrow bpf = 0
            vdpq_bpf_bw_sel_y_reg.narrow_bpf_sel_y = 0;
            // fix the c 2D direction to avoid cross color, willy 20151224
            vdpq_adap_bpf_y_th3_reg.malpha_c_en = 1;
            vdpq_adap_bpf_y_th3_reg.malpha_c = 1;

            // avoid cross color on the flute, willy 20160119
            /*
            vdpq_yc_bw_ctrl_reg.chroma_bw_lo = 7;
            vdpq_yc_bw_ctrl_reg.chroma_vlpf_en = 1;
            */
            break;
        }
        case VDPQ_PAL_VIP_DOS_28:
        {
            // mid bpf = 0, 20160105, jimmy
            //vdpq_bpf_bw_sel_y_reg.mid_bpf_sel_y = 0;

            // make finger part go in narrow bpf, 20160106, jimmy
            vdpq_cdiff_adap_bpf_th2_reg.cdiff_bpf_small_amp_wei = 0;
            break;
        }
        case VDPQ_NTSC_VIP_DOS_63:
        {
            // 20151203, jimmy, DOS#63 yoga woman
            // narrow bpf = 0 to make arm edge clear
            vdpq_bpf_bw_sel_y_reg.narrow_bpf_sel_y = 0;
            // CDiff_BPF_small_offset = 15 to keep cloth in mid (avoid xc)
            vdpq_cdiff_adap_bpf_th2_reg.cdiff_bpf_small_offset = 15;
            vdpq_adap_bpf_y_th3_reg.malpha_c_en = 1;
            vdpq_adap_bpf_y_th3_reg.malpha_c = 1;
            // ---------------------------------------------------------------------------
            // 20151210, willy, make the arm edge get into 2D and improve the 2d solution
            //vdpq_yc_3d_f4_pair_ctrl_6_reg.global_diff_tolerance = 1;
            // h2v_lk_en
            vdpq_adap_bpf_c_th1_reg.h2v_lk_en= 1;
            vdpq_adap_bpf_c_th1_reg.h2v_mid_sel_y = 1;
            // ---------------------------------------------------------------------------
            // ---------------------------------------------------------------------------
            // 20151210, eliminate the cross color by vlpf, rather than de-xc
            vdpq_de_xc_ctrl_reg.de_xc_en = 0;
            vdpq_yc_bw_ctrl_reg.chroma_vlpf_en = 1;
            // ---------------------------------------------------------------------------

            // 20160109, jimmy, back to more 3D setting for eyebrow and cloth clear
            vdpq_yc_3d_f4_pair_ctrl_6_reg.global_diff_tolerance = 15;
            vdpq_yc_3d_f4_pair_ctrl_6_reg.dc_level_fierce_change_th = 63;

            // 20160109, jimmy, eyebrow 2D clear by post2DBPF
            vdpq_yc_post2dbpf_ctrl_reg.weight_2d_y = 2;
            vdpq_yc_post2dbpf_ctrl_reg.weight_bld_y = 2;
            vdpq_yc_post2dbpf_adapth_13_reg.ignore_th_y = 0;
            vdpq_yc_post2dbpf_thbase_reg.hor_cdiff_th_base = 50;
            vdpq_yc_post2dbpf_thbase_reg.ver_cdiff_th_base = 50;
            break;
        }
        case VDPQ_PAL_VIP_DOS_63:
        {
            // make finger part go in narrow bpf, 20160106, jimmy
            vdpq_cdiff_adap_bpf_th2_reg.cdiff_bpf_small_amp_wei = 0;

            // turn off dexc, 20160106, jimmy
            vdpq_de_xc_ctrl_reg.de_xc_en = 0;

            break;
        }
        case VDPQ_PAL_STRONG_VD_SMD:
        {
            vdpq_yc_3d_f4_smd_en_reg.smd_ver_00_move_enable = 1;
            vdpq_yc_3d_f4_smd_en_reg.smd_ver_06_move_enable = 1;
            vdpq_yc_3d_f4_smd_en_reg.smd_ver_08_move_enable = 1;
            vdpq_yc_3d_f4_smd_th1_reg.smd_ver_spatial_temporal_traily_diff = 0;
            vdpq_yc_3d_f4_smd_en_reg.smd_hor_00_move_enable = 1;
            vdpq_yc_3d_f4_smd_en_reg.smd_hor_06_move_enable = 1;
            vdpq_yc_3d_f4_smd_en_reg.smd_hor_08_move_enable = 1;
            vdpq_yc_3d_f4_smd_th1_reg.smd_hor_spatial_temporal_traily_diff = 0;

            break;
        }
        case VDPQ_NTSC_STRONG_VD_SMD:
        {
            vdpq_yc_3d_f4_smd_en_reg.smd_ver_00_move_enable = 1;
            vdpq_yc_3d_f4_smd_en_reg.smd_ver_06_move_enable = 1;
            vdpq_yc_3d_f4_smd_en_reg.smd_ver_08_move_enable = 1;
            vdpq_yc_3d_f4_smd_th1_reg.smd_ver_spatial_temporal_traily_diff = 0;
            vdpq_yc_3d_f4_smd_en_reg.smd_hor_00_move_enable = 1;
            vdpq_yc_3d_f4_smd_en_reg.smd_hor_06_move_enable = 1;
            vdpq_yc_3d_f4_smd_en_reg.smd_hor_08_move_enable = 1;
            vdpq_yc_3d_f4_smd_th1_reg.smd_hor_spatial_temporal_traily_diff = 0;

            break;
        }
        case VDPQ_OTHER_STRONG_VD_SMD:
        {
            vdpq_yc_3d_f4_smd_en_reg.smd_ver_00_move_enable = 1;
            vdpq_yc_3d_f4_smd_en_reg.smd_ver_06_move_enable = 1;
            vdpq_yc_3d_f4_smd_en_reg.smd_ver_08_move_enable = 1;
            vdpq_yc_3d_f4_smd_th1_reg.smd_ver_spatial_temporal_traily_diff = 0;
            vdpq_yc_3d_f4_smd_en_reg.smd_hor_00_move_enable = 1;
            vdpq_yc_3d_f4_smd_en_reg.smd_hor_06_move_enable = 1;
            vdpq_yc_3d_f4_smd_en_reg.smd_hor_08_move_enable = 1;
            vdpq_yc_3d_f4_smd_th1_reg.smd_hor_spatial_temporal_traily_diff = 0;
            break;
        }
        case VDPQ_ATV_NTSC_HIGH_SAT_DOTCRAWL_0: // willy, 20151123
        {
            // 20151217, willy, sync from AV setting
            // h2v_lk_en
            vdpq_adap_bpf_c_th1_reg.h2v_lk_en= 0;
            // post 2d bpf y enable and adjust the threshold, 20150818, willy, improve LG 44 2D result for 2D/3D flick
            vdpq_yc_post2dbpf_ctrl_reg.weight_2d_y = 0;
            vdpq_yc_post2dbpf_ctrl_reg.weight_bld_y = 0;
            //h2v lut
            vdpq_alpha_2d_mod_enable_reg.lut_sel_y = 3;
            //post MF
            vdpq_postp_mf_compensation_ctrl1_reg.cdiff_small_con_en = 0;
            // 1D BPF
            vdpq_blend_1d_thr_reg.bpf_1d_sel_y = 3; // 20151217, willy, sync with kernel default value to avoid tmp status when channel switch
            break;
        }
        case VDPQ_ATV_PAL_HIGH_SAT_DOTCRAWL_0: // willy 20151123
        {
            // h2v_lk_en
            vdpq_adap_bpf_c_th1_reg.h2v_lk_en= 0;
            // post 2d bpf y enable and adjust the threshold, 20150818, willy, improve LG 44 2D result for 2D/3D flick
            vdpq_yc_post2dbpf_ctrl_reg.weight_2d_y = 0;
            vdpq_yc_post2dbpf_ctrl_reg.weight_bld_y = 0;
            //h2v lut
            vdpq_alpha_2d_mod_enable_reg.lut_sel_y = 3;
            //post MF
            vdpq_postp_mf_compensation_ctrl1_reg.cdiff_small_con_en = 0;
            // 1D BPF
            //BLEND_1D_THR_reg.bpf_1d_sel_y = 2;
            vdpq_blend_1d_thr_reg.bpf_1d_sel_y = 3; // 20151217, willy, sync with kernel default value to avoid tmp status when channel switch
            break;
        }
        case VDPQ_ATV_OTHER_HIGH_SAT_DOTCRAWL_0:
        {
            // h2v_lk_en
            vdpq_adap_bpf_c_th1_reg.h2v_lk_en= 0;
            // post 2d bpf y disabled for PALM/N color bar, cannot get into 3D at Y high freq and cause dot crawl, 20150909, willy
            vdpq_yc_post2dbpf_ctrl_reg.weight_2d_y = 0;
            vdpq_yc_post2dbpf_ctrl_reg.weight_bld_y = 0;
            //h2v lut
            vdpq_alpha_2d_mod_enable_reg.lut_sel_y = 3;
            //post MF
            vdpq_postp_mf_compensation_ctrl1_reg.cdiff_small_con_en = 0;
            // 1D BPF
            vdpq_blend_1d_thr_reg.bpf_1d_sel_y = 2;
            break;
        }
        case VDPQ_ATV_NTSC_HIGH_SAT_DOTCRAWL_1: // willy 20151123
        {
            // h2v_lk_en
            vdpq_adap_bpf_c_th1_reg.h2v_lk_en= 0;
            // post2dbpf_y_en
            vdpq_yc_post2dbpf_ctrl_reg.weight_2d_y = 2;
            vdpq_yc_post2dbpf_ctrl_reg.weight_bld_y = 2;
            //h2v lut
            vdpq_alpha_2d_mod_enable_reg.lut_sel_y = 0;
            //post MF
            vdpq_postp_mf_compensation_ctrl1_reg.cdiff_small_con_en = 1;
            // 1D BPF
            vdpq_blend_1d_thr_reg.bpf_1d_sel_y = 3; // 20151217, willy, sync with kernel default value to avoid tmp status when channel switch
            break;
        }
        case VDPQ_ATV_PAL_HIGH_SAT_DOTCRAWL_1: // willy 20151123
        {
            // h2v_lk_en
            vdpq_adap_bpf_c_th1_reg.h2v_lk_en= 0;
            // post2dbpf_y_en
            vdpq_yc_post2dbpf_ctrl_reg.weight_2d_y = 2;
            vdpq_yc_post2dbpf_ctrl_reg.weight_bld_y = 2;
            //h2v lut
            vdpq_alpha_2d_mod_enable_reg.lut_sel_y = 0;
            //post MF
            vdpq_postp_mf_compensation_ctrl1_reg.cdiff_small_con_en = 1;
            // 1D BPF
            vdpq_blend_1d_thr_reg.bpf_1d_sel_y = 3; // 20151217, willy, sync with kernel default value to avoid tmp status when channel switch
            break;
        }
        case VDPQ_ATV_OTHER_HIGH_SAT_DOTCRAWL_1:
        {
            // h2v_lk_en
            vdpq_adap_bpf_c_th1_reg.h2v_lk_en= 0;
            // post2dbpf_y_en
            vdpq_yc_post2dbpf_ctrl_reg.weight_2d_y = 0;
            vdpq_yc_post2dbpf_ctrl_reg.weight_bld_y = 0;
            //h2v lut
            vdpq_alpha_2d_mod_enable_reg.lut_sel_y = 0;
            //post MF
            vdpq_postp_mf_compensation_ctrl1_reg.cdiff_small_con_en = 1;
            // 1D BPF
            vdpq_blend_1d_thr_reg.bpf_1d_sel_y = 3; // 20151217, willy, sync with kernel default value to avoid tmp status when channel switch
            break;
        }
        case VDPQ_OTHER_DIA_EDGE_2D3D_JUMP:
        {
            // 20151008, willy, LG #29 woman arm 2d/3d edge peaking jump (because 2d edge is blur)
            vdpq_yc_post2dbpf_thbase_reg.hor_cdiff_th_base = 90;
            break;
        }
        case VDPQ_PAL_LOW_SAT_REMOVE_COLOR:
        {
            // 20151010, willy, DOS#151 ~ DOS#153 black and white character moving cross color
            // strong de-XC need to avoid DOS#131 ~ DOS#136 color edge blur
            vdpq_de_xc_ctrl_reg.de_xc_en = 1;
            vdpq_yc_bw_ctrl_reg.chroma_bw_lo = 7;
            vdpq_yc_bw_ctrl_reg.chroma_vlpf_en = 1;
            break;
        }
        case VDPQ_NTSC_LOW_SAT_REMOVE_COLOR:
        {
            // 20151010, willy, DOS#151 ~ DOS#153 black and white character moving cross color
            // strong de-XC need to avoid DOS#131 ~ DOS#136 color edge blur
            vdpq_de_xc_ctrl_reg.de_xc_en = 1;
            vdpq_yc_bw_ctrl_reg.chroma_bw_lo = 7;
            vdpq_yc_bw_ctrl_reg.chroma_vlpf_en = 1;
            break;
        }
        case VDPQ_OTHER_LOW_SAT_REMOVE_COLOR:
        {
            // 20151010, willy, DOS#151 ~ DOS#153 black and white character moving cross color
            // strong de-XC need to avoid DOS#131 ~ DOS#136 color edge blur
            vdpq_de_xc_ctrl_reg.de_xc_en = 1;
            vdpq_yc_bw_ctrl_reg.chroma_bw_lo = 7;
            vdpq_yc_bw_ctrl_reg.chroma_vlpf_en = 1;
            break;
        }
        case VDPQ_NTSC_MONO_HUE_AVOID_MOTION_BLUR:
        {
            /*
            // 20151014, willy, avoid 2d motion blur by adjust the horizontal LPF in xNR
            vdpq_xnr_ctrl_reg.xnr_lpf_sel = 0;
            // 20151014, willy, apply post 2d bpf y to enhance the 2d performance
            vdpq_yc_post2dbpf_ctrl_reg.weight_2d_y = 3;
            vdpq_yc_post2dbpf_thbase_reg.hor_cdiff_th_base = 35;
            vdpq_yc_post2dbpf_adapth_1_reg.bpfed_c_diff_small_th = 15;
            vdpq_yc_post2dbpf_adapth_2_reg.bpfed_c_diff_large_hor_offset = 25;
            vdpq_yc_post2dbpf_adapth_2_reg.bpfed_c_diff_small_hor_offset = 5;
            vdpq_yc_post2dbpf_adapth_8_reg.hdiff_much_larger_ratio = 10;
            vdpq_yc_post2dbpf_adapth_8_reg.hdiff_large_th = 50;
            vdpq_yc_post2dbpf_adapth_8_reg.hdiff_vdiff_diff_larger_th = 30;
            vdpq_yc_post2dbpf_adapth_9_reg.hdiff_much_larger_hor_offset = 40;
            */
            // 20151130, jimmy, dexc off
            vdpq_de_xc_ctrl_reg.de_xc_en = 0;
            break;
        }
        case VDPQ_PAL_DOS_REF4:
        {
            // 20151022 jimmy, for Ref#4 MVline in PAL
            // global_diff_tolerance = 1 for dot-crawl
            vdpq_yc_3d_f4_pair_ctrl_6_reg.global_diff_tolerance = 1;
            // Make MVLine 2D
            vdpq_yc_3d_f4_mghf_ctrl_1_reg.moving_vline_flat_ver_cvbs_th = 15;
            vdpq_yc_3d_f4_mghf_ctrl_1_reg.moving_vline_ver_dc_level_fierce_change_th = 10;
            // h2v_lk_en = 0
            vdpq_adap_bpf_c_th1_reg.h2v_lk_en= 0;
            // lut_sel_y = 1(old NTSC)
            vdpq_alpha_2d_mod_enable_reg.lut_sel_y = 1;
            // h2v_y_noise_thr = 10
            vdpq_h2v_y_noise_thr_reg.h2v_y_noise_thr = 10;
            // new v2v lut, v2v_noise_y_add_dc = 15
            vdpq_h2v_noise_y_add_dc_reg.v2v_lut_sel_y = 1;
            // 2D VMC
            vdpq_vmc_ctrl_reg.ver_dc_still_th = 50;
            vdpq_vmc_ctrl_reg.ver_cvbsamp_still_th = 30;
            vdpq_vmc_ctrl_reg.amp_low_bnd = 20;
            vdpq_diamc_cvbs_amp_th_reg.vmc_widehspd_small_th = 15;
            // disable all post filters to make the lines clean
            // xnr off
            vdpq_xnr_ctrl_reg.xnr_lpf_sel = 0;
            // dexc off
            vdpq_de_xc_ctrl_reg.de_xc_en = 0;
            // chroma vlpf off
            vdpq_yc_bw_ctrl_reg.chroma_vlpf_en = 0;
            break;
        }
        case VDPQ_PAL_VIP_DOS_107:
        {
            //-----------------------------------------------------------------------------
            // Avoid first horizontal scrolling text vertical line stick together
            vdpq_alpha_2d_mod_enable_reg.h2v_coring_thr = 1;
            /*
            // 20151212, willy, tend to 1D solution to avoid first horizontal scrolling text dot crawl at the upper white line
            vdpq_h2v_y_noise_thr_reg.h2v_y_noise_thr = 30;
            */
            // 20151213, willy, LG logo dot crawl and horizontal scrolling text clearity
            vdpq_cdiff_adap_bpf_th2_reg.cdiff_bpf_small_amp_wei = 0;
            vdpq_cdiff_adap_bpf_th2_reg.cdiff_bpf_vspd_small_th = 60;
            vdpq_postp_3d2dalpha_blend_ctrl_1_reg.xnr_alpha_2d_weight = 2;
            //-----------------------------------------------------------------------------

            //-----------------------------------------------------------------------------
            // 20151212, willy, use to make vertical scrolling text into 3D to stablize
            vdpq_yc_3d_f4_pair_ctrl_4_reg.diff_th_upper_bound = 60;
            vdpq_yc_3d_f4_pair_ctrl_6_reg.global_diff_tolerance = 5;
            /*
            vdpq_yc_3d_f4_smd_en_reg.smd_en = 1;
            vdpq_yc_3d_f4_smd_en_reg.smd_00_move_enable = 1;
            vdpq_yc_3d_f4_smd_en_reg.smd_02_move_enable = 1;
            vdpq_yc_3d_f4_smd_en_reg.smd_04_move_enable = 1;
            vdpq_yc_3d_f4_smd_en_reg.smd_06_move_enable = 1;
            vdpq_yc_3d_f4_smd_en_reg.smd_08_move_enable = 1;
            vdpq_yc_3d_f4_smd_th1_reg.smd_horizontal_sad_tolerance = 0;
            vdpq_yc_3d_f4_smd_th1_reg.smd_vertical_sad_tolerance = 30;
            vdpq_yc_3d_f4_smd_th1_reg.smd_spatial_temporal_traily_diff = 0;
            vdpq_yc_3d_f4_smd_th2_reg.smd_min_sad_horizontal = 0;
            vdpq_yc_3d_f4_smd_th2_reg.smd_min_sad_vetical = 50;
            vdpq_yc_3d_f4_pair_smd_th1_reg.smd_highfreq_diff_2_th = 45;
            */
            //-----------------------------------------------------------------------------

            //-----------------------------------------------------------------------------
            // 20151217, willy, make the vertical scrolling text more clear by post 2D BPF
            vdpq_yc_post2dbpf_ctrl_reg.weight_2d_y = 2;
            vdpq_yc_post2dbpf_thbase_reg.hor_cdiff_th_base= 70;
            vdpq_yc_post2dbpf_thbase_reg.ver_cdiff_th_base= 70;
            //-----------------------------------------------------------------------------

            // 20151218, willy, 2D post LPF to reduce the vertical scrolling text dot noise between lin
            vdpq_yc2d_postlp_flat_1_reg.low_amp_th_0 = 11;
            vdpq_yc2d_postlp_flat_1_reg.low_amp_th_1 = 12;
            vdpq_yc2d_postlp_flat_1_reg.low_amp_th_2 = 14;
            vdpq_yc2d_postlp_flat_1_reg.low_amp_th_3 = 15;
            vdpq_yc2d_postlp_flat_2_reg.low_amp_th_4 = 17;
            vdpq_yc2d_postlp_flat_2_reg.hdiff_th_0 = 15;
            vdpq_yc2d_postlp_flat_2_reg.hdiff_th_1 = 16;
            vdpq_yc2d_postlp_flat_2_reg.hdiff_th_2 = 18;
            vdpq_yc2d_postlp_flat_3_reg.hdiff_th_3 = 21;
            vdpq_yc2d_postlp_flat_3_reg.hdiff_th_4 = 24;

            // 20160111, jimmy, HV edge tolerence to reduce vertical scrolling text dot noise between lines caused by 3D
            vdpq_yc_3d_f4_hv_edge_1_reg.hv_diff_tolerance = 10;

            // 20150122, willy, adjust the 2d/3d alpha blending ratio to make vertical scrolling text into 3D
            vdpq_yc_3d_f4_spa_cnt_cvbs_amp_reg.f4_3d_spatial_cnt_th_1 = 1;
            vdpq_yc_3d_f4_spa_cnt_cvbs_amp_reg.f4_3d_spatial_cnt_th_2 = 4;

			// 20160705, willy, blending ratio change control register
			vdpq_alpha_blending_ctrl1_reg.alpha2d3d_bld_weight_1 = 4;
			vdpq_alpha_blending_ctrl1_reg.alpha2d3d_bld_weight_2 = 4;
			vdpq_alpha_blending_ctrl1_reg.alpha2d3d_bld_weight_3 = 4;
			vdpq_alpha_blending_ctrl1_reg.alpha2d3d_bld_weight_4 = 8;
			vdpq_alpha_blending_ctrl2_reg.alpha2d3d_bld_weight_5 = 8;
			vdpq_alpha_blending_ctrl2_reg.alpha2d3d_bld_weight_6 = 8;
			vdpq_alpha_blending_ctrl2_reg.alpha2d3d_bld_weight_7 = 8;
			vdpq_alpha_blending_ctrl2_reg.alpha2d3d_bld_weight_8 = 8;
			vdpq_alpha_blending_ctrl3_reg.alpha2d3d_bld_weight_9 = 12;
			vdpq_alpha_blending_ctrl3_reg.alpha2d3d_bld_weight_10 = 12;
			vdpq_alpha_blending_ctrl3_reg.alpha2d3d_bld_weight_11 = 14;
			vdpq_alpha_blending_ctrl3_reg.alpha2d3d_bld_weight_12 = 14;
			vdpq_alpha_blending_ctrl4_reg.alpha2d3d_bld_weight_13 = 16;
			vdpq_alpha_blending_ctrl4_reg.alpha2d3d_bld_weight_14 = 16;
			vdpq_alpha_blending_ctrl4_reg.alpha2d3d_bld_weight_15 = 16;

            break;
        }
        case VDPQ_PAL_DOS_CORN_FIELD:
        {
            // Use global diff tolerance to simulate the SMD effect
            vdpq_yc_3d_f4_pair_ctrl_6_reg.global_diff_tolerance = 10;
            // force Y to 2D solution
            vdpq_adap_bpf_y_th3_reg.malpha_y_en = 1;
            vdpq_adap_bpf_y_th3_reg.malpha_y = 0;
            // disable xNR HLPF and enable xNR VLPF to avoid too much dot-crawl
            vdpq_xnr_ctrl_reg.xnr_vlpf_sel = 3;
            vdpq_xnr_ctrl_reg.xnr_lpf_sel = 0;
            // disable dexc, 20151201, jimmy
            vdpq_de_xc_ctrl_reg.de_xc_en = 0;
            break;
        }
        case VDPQ_PAL_XC_REDUCE_BY_C_BPF:
        {
            // disable dexc to avoid edge color blur, 20151216, willy
            vdpq_de_xc_ctrl_reg.de_xc_en = 0;
            vdpq_cdiff_adap_bpf_th2_reg.cdiff_bpf_small_amp_wei = 4;
            break;
        }
        case VDPQ_NTSC_XC_REDUCE_BY_C_BPF:
        {
            // disable dexc to avoid edge color blur, 20151216, willy
            vdpq_de_xc_ctrl_reg.de_xc_en = 0;
            // Avoid flag edge color blur, 20160108, jimmy
            vdpq_bpf_bw_sel_c_reg.mid_bpf_sel_c = 1;
            break;
        }
        case VDPQ_NTSC_HIGH_FREQ_LOW_SAT:
        {
            // enhance the 2D solution
            vdpq_yc_post2dbpf_thbase_reg.hor_cdiff_th_base = 120;
            vdpq_yc_post2dbpf_thbase_reg.ver_cdiff_th_base = 120;
            // avoid to blur the 2D solution
            vdpq_xnr_ctrl_reg.xnr_lpf_sel = 0;
            break;
        }
        case VDPQ_NTSC_VIP_DOS_134:
        {
            // disable dexc to avoid edge color blur, 20160105, jimmy
            vdpq_de_xc_ctrl_reg.de_xc_en = 0;
            break;
        }
        case VDPQ_PAL_VIP_DOS_134:
        {
            // disable dexc to avoid edge color blur, 20160105, jimmy
            vdpq_de_xc_ctrl_reg.de_xc_en = 0;
            break;
        }
        case VDPQ_NTSC_2D3D_SWITCH:
        {
            // 20160107, jimmy, target #42, improve 2D3D switching speed and stability
            vdpq_yc_3d_f4_pair_ctrl_6_reg.hdy_small_th = 0;
			// 20161101, willy, change filter setting for supress the cross color
			vdpq_bpf_bw_sel_c_reg.mid_bpf_sel_c = 0;
			vdpq_bpf_bw_sel_c_reg.wide_bpf_sel_c = 0;
            break;
        }
        case VDPQ_NTSC_LOW_SAT_STILL:
        {
            // 20160108, jimmy, target #38, make 3D stable in static background
            vdpq_yc_3d_f4_pair_ctrl_6_reg.global_diff_tolerance = 5;
            break;
        }
        case VDPQ_NTSC_VIP_DOS_LEAF_72:
        {
            // LG leaf #72, color scrolling text

            // 20160114, jimmy, 1st setting trial
            // h2v_lk_en on
            //vdpq_adap_bpf_c_th1_reg.h2v_lk_en= 1;
            // bpf_y 5/5/5
            vdpq_bpf_bw_sel_y_reg.narrow_bpf_sel_y = 5;
            // bpf_c 5/2/2
            vdpq_bpf_bw_sel_c_reg.narrow_bpf_sel_c = 2;
            vdpq_bpf_bw_sel_c_reg.mid_bpf_sel_c = 2;
            // de_xc weaker
            vdpq_de_xc_ctrl_reg.de_xc_thr = 10;
            // xnr 2D weight = 4
            vdpq_postp_3d2dalpha_blend_ctrl_1_reg.xnr_alpha_2d_weight = 4;

            // 20160123, willy, tend to 1D solution
            vdpq_h2v_y_noise_thr_reg.h2v_y_noise_thr = 100;

            // 20160123, willy, surpress the cross color by post 2D BPF C
            vdpq_yc_post2dbpf_thbase_reg.hor_cdiff_th_base = 80;
            vdpq_yc_post2dbpf_thbase_reg.ver_cdiff_th_base = 80;

            break;
        }
        case VDPQ_PAL_VIP_DOS_LEAF_74:
        {
            // 20160122 willy, tend to 1D solution to avoid character teeth, this issue is similar with monoscope line teeth by 2D solution
            vdpq_h2v_blend_ratio_reg.h2v_blendratio = 6;
            vdpq_alpha_2d_mod_enable_reg.h2v_coring_thr = 4;

            // 20160122 willy, use weaker C BPF and disable deXC to avoid color boundary blur
            vdpq_bpf_bw_sel_c_reg.mid_bpf_sel_c = 4;
            vdpq_blend_1d_thr_reg.bpf_1d_sel_y = 3;
            vdpq_de_xc_ctrl_reg.de_xc_en = 0;
            vdpq_de_xc_ctrl_reg.de_xc_thr = 15;
            // 20150122 willy, disable force malpha to avoid ghost noise around the character
            vdpq_alpha_2d_mod_enable_reg.force_halpha_en = 0;

            // 20160125, willy, adjust the Y narrow BPF range and setting to avoid sixth scrolling text "4" edge blur
            vdpq_bpf_bw_sel_y_reg.narrow_bpf_sel_y = 0;
            vdpq_cdiff_adap_bpf_th2_reg.cdiff_bpf_large_amp_wei = 3;
            vdpq_cdiff_adap_bpf_th2_reg.cdiff_bpf_vspd_small_th = 60;
            break;
        }
		case VDPQ_ATV_PAL_FREQ_OFFSET_SMALL:
		{
			vdpq_yc_3d_f4_pal_err_com_3_reg.f4f0_still_en = 1;
			break;
		}
		case VDPQ_ATV_PAL_FREQ_OFFSET_LARGE:
		{
			vdpq_yc_3d_f4_smd_en_reg.smd_en = 0;
			vdpq_yc_3d_f4_pair_ctrl_4_reg.diff_th_upper_bound = 6;
			vdpq_yc_3d_f4_pair_ctrl_6_reg.global_diff_tolerance = 0;
			vdpq_bpf_bw_sel_y_reg.mid_bpf_sel_y = 2;
			break;
		}
		case VDPQ_ATV_NTSC_NOISY_COLOR_EDGE1:
		{
			vdpq_cadap_gain_ctrl_reg.cadap_gain = 4;
			vdpq_cadap_lut_gain_th_reg.cadap_lut_gain_tha = 2;
			vdpq_cadap_lut_gain_th_reg.cadap_lut_gain_thb = 4;
			break;
		}
		case VDPQ_ATV_NTSC_NOISY_COLOR_EDGE2:
		{
			vdpq_cadap_gain_ctrl_reg.cadap_gain = 2;
			vdpq_cadap_lut_gain_th_reg.cadap_lut_gain_tha = 2;
			vdpq_cadap_lut_gain_th_reg.cadap_lut_gain_thb = 4;
			break;
		}
		// ================================= DYNAMIC ADJUST REGISTER  ================================= //
        case VDPQ_NTSC_DYNAMIC_HIGH_FREQ_LOW_SAT_Y:
        {
            vdpq_yc_post2dbpf_thbase_reg.hor_cdiff_th_base = ScalerVideo_Status.DynamicREG.Hor_cdiff_threshold_base;
            vdpq_yc_post2dbpf_thbase_reg.ver_cdiff_th_base = ScalerVideo_Status.DynamicREG.Ver_cdiff_threshold_base;
            break;
        }
        case VDPQ_NTSC_DYNAMIC_HIGH_FREQ_LOW_SAT_XC:
        {
            vdpq_yc_bw_ctrl_reg.chroma_bw_lo = ScalerVideo_Status.DynamicREG.ChromaLPF_BandWidth;
            break;
        }
		case VDPQ_NTSC_DYNAMIC_DIRECTIONAL_3D:
		{
			vdpq_directional_3d_reg.dir3d_fw_mode_dalpha = ScalerVideo_Status.DynamicREG.Directional3D_bld_ratio;
			vdpq_directional_3d_reg.dir3d_fw_mode_motion = ScalerVideo_Status.DynamicREG.Directional3D_dir;
			break;
		}
        /*================================= END OF PQ ADJUSTMENT CASE START ====================================*/
        /*======================================================================================================*/
        default:
        {

            {
                ROS_VideoPrintf_Setting("[VDPQ] Apply register set %d undefined!\n", SetNumber);
            }
            return FALSE;
        }
    }

    return TRUE;
}

void scalerVideo_CalculateNoiseStatus(void)
{
    // willy 20150617, noise status division by FW in ISR

    // Setting the hsync start and end position
    vdtop_hsync_gate_start_RBUS vdtop_hsync_gate_start_reg;
    vdtop_hsync_gate_end_RBUS vdtop_hsync_gate_end_reg;
    vdpq_ns_gate_RBUS vdpq_ns_gate_reg;
    vdpq_ns_r1_RBUS vdpq_ns_r1;
    vdpq_ns_r2_RBUS vdpq_ns_r2;
    vdpq_ns_r3_RBUS vdpq_ns_r3;
    vdpq_ns_rw1_RBUS vdpq_ns_rw1;
    vdpq_ns_rw2_RBUS vdpq_ns_rw2;

    unsigned int SumVm;
    unsigned int CountV;
    unsigned int SumV;
    unsigned int FieldCount = 1;

	vdtop_hsync_gate_start_reg.regValue = VDPQ_rtd_inl(VDTOP_HSYNC_GATE_START_reg);
	vdtop_hsync_gate_end_reg.regValue = VDPQ_rtd_inl(VDTOP_HSYNC_GATE_END_reg);
	vdpq_ns_gate_reg.regValue = VDPQ_rtd_inl(VDPQ_NS_GATE_reg);
	vdpq_ns_gate_reg.pq_noisestatus_hsync_start = vdtop_hsync_gate_start_reg.hsync_gate_start;
	vdpq_ns_gate_reg.pq_noisestatus_hsync_end = vdtop_hsync_gate_end_reg.hsync_gate_end;
	VDPQ_rtd_outl(VDPQ_NS_GATE_reg, vdpq_ns_gate_reg.regValue);

    // read calculation value from register
    vdpq_ns_r1.regValue = VDPQ_rtd_inl(VDPQ_NS_R1_reg);
    vdpq_ns_r2.regValue = VDPQ_rtd_inl(VDPQ_NS_R2_reg);
    vdpq_ns_r3.regValue = VDPQ_rtd_inl(VDPQ_NS_R3_reg);


	SumVm = vdpq_ns_r1.sum_vm;
	CountV = vdpq_ns_r2.countv;
	SumV = vdpq_ns_r3.sumv;



	if( ScalerVideo_Status.VD_Mode == VDC_MODE_NTSC ||
		ScalerVideo_Status.VD_Mode == VDC_MODE_PALM ||
		ScalerVideo_Status.VD_Mode == VDC_MODE_NTSC443 ||
		ScalerVideo_Status.VD_Mode == VDC_MODE_PAL60 )
        FieldCount = 64*240; // 525 line system, NTSC, palm, NTSC443, pal60
	else if( ScalerVideo_Status.VD_Mode == VDC_MODE_NTSC50 ||
			 ScalerVideo_Status.VD_Mode == VDC_MODE_PALN ||
			 ScalerVideo_Status.VD_Mode == VDC_MODE_PALI ||
			 ScalerVideo_Status.VD_Mode == VDC_MODE_SECAM )
        FieldCount = 64*288; // 625 line system, other
    else
        FieldCount = 1;

    // read the original value and update
    vdpq_ns_rw1.regValue = VDPQ_rtd_inl(VDPQ_NS_RW1_reg);
    vdpq_ns_rw1.vmm = ( SumVm + (FieldCount >> 1) )/FieldCount;
    vdpq_ns_rw2.regValue = VDPQ_rtd_inl(VDPQ_NS_RW2_reg);
    vdpq_ns_rw2.density = ( CountV + (FieldCount >> 1) )/FieldCount;
    vdpq_ns_rw2.meannxv = ( SumV + (CountV >> 1) )/(CountV+1);

    // write back the result
    VDPQ_rtd_outl(VDPQ_NS_RW1_reg, vdpq_ns_rw1.regValue);
    VDPQ_rtd_outl(VDPQ_NS_RW2_reg, vdpq_ns_rw2.regValue);

}

inline void vdc_hw_semaphore_lock(void)
{
        /* 0xb801a624 */
	unsigned int print_cnt = 0;

        while (!rtd_inl(0xB801A634)) {
                //pr_err("#@# %s wait SB2_HD_SEM_NEW_1_reg ready\n", moduleName);
                //pr_err("#@# last %s used SB2_HD_SEM_NEW_1_reg semaphore\n", lastModuleName);
                if( print_cnt == 0 ) {
    	        	ROSPrintf("[VD W/R Reg] vdc_hw_semaphore_lock : %x\n\n",rtd_inl(0xB801A634));
			print_cnt = 1;
		}
               // VP_Delay(2);//~ 22 us
                //udelay(20);
        }
}

inline void vdc_hw_semaphore_unlock(void)
{
        /* 0xb801a624 */
        rtd_outl(0xB801A634,0);
	//VP_Delay(1);//~ 11 us
        //udelay(10);
}

extern int rtd_crt_swsem_trylock(int bit);
extern void rtd_crt_swsem_lock(int bit);
extern void rtd_crt_swsem_unlock(int bit);
#define PLL_27X3_LOCKBIT      BIT(0)
#define AVDIN_IFD_LOCKBIT     BIT(4)


#define  RTD_SWSEM_27x3_TRYLOCK() rtd_crt_swsem_trylock(PLL_27X3_LOCKBIT)
#define  RTD_SWSEM_27x3_LOCK()    rtd_crt_swsem_lock(PLL_27X3_LOCKBIT)
#define  RTD_SWSEM_27x3_UNLOCK()  rtd_crt_swsem_unlock(PLL_27X3_LOCKBIT)

#define  RTD_SWSEM_AVDIN_TRYLOCK() rtd_crt_swsem_trylock(AVDIN_IFD_LOCKBIT)
#define  RTD_SWSEM_AVDIN_LOCK()    rtd_crt_swsem_lock(AVDIN_IFD_LOCKBIT)
#define  RTD_SWSEM_AVDIN_UNLOCK()  rtd_crt_swsem_unlock(AVDIN_IFD_LOCKBIT)

#define AVD_LOCK() \
				do{ \
					RTD_SWSEM_AVDIN_LOCK(); \
				}while(0)

#define AVD_UNLOCK() \
				do{ \
					RTD_SWSEM_AVDIN_UNLOCK(); \
				}while(0)





unsigned char AVD_Clock_Flag_detection(void)
{
	unsigned char rstn_vdec2_flag = 1, clken_atvin_vd_flag = 1;
	unsigned char IFD_freq_change_flag = 1;
	//unsigned int print_cnt = 0;

	rstn_vdec2_flag = ((rtd_inl(0xb8000104)>>21)&0x01) ; /* b800_0104[21] , for VD SW reset , ,  0xb8000104[21]=1*/
	clken_atvin_vd_flag = ((rtd_inl(0xb8000118)>>27)&0x01); /* b800_0118[27], for VD clock reset ,  0xb8000118[27]=1*/
	//AVD_Reset_Enable_flag = ((rtd_inl(0xB801A604))&0x01); /*B801A604 [0], for AVD reset start, 0xB801A604[0]=1*/
	IFD_freq_change_flag = ((rtd_inl(0xB801A604)>>4)&0x01);/*B801A604 [4], for IFD_freq_change start, */


	//if( (( rstn_vdec2_flag == 1 ) && ( clken_atvin_vd_flag == 1 )) && (IFD_freq_change_flag == 0) ) {
	if( (( rstn_vdec2_flag == 1 ) && ( clken_atvin_vd_flag == 1 ))) {
		AVD_Clock_en_check_flag = 1;
		return TRUE ;
	} else {
		if(AVD_Clock_en_check_flag == 1){
			if( IFD_freq_change_flag == 1 ) {
				//VP_Delay(90*6); // wait 6 ms
				//msleep(6);
			} else {
				//VP_Delay(90*2.5); // wait 2.5 ms
				//msleep(3);
			}
		}
		rstn_vdec2_flag = ((rtd_inl(0xb8000104)>>21)&0x01) ; /* b800_0104[21] , for VD SW reset , ,  0xb8000104[21]=1*/
		clken_atvin_vd_flag = ((rtd_inl(0xb8000118)>>27)&0x01); /* b800_0118[27], for VD clock reset ,  0xb8000118[27]=1*/
		IFD_freq_change_flag = ((rtd_inl(0xB801A604)>>4)&0x01);/*B801A604 [4], for IFD_freq_change start, */

		//if( (( rstn_vdec2_flag == 1 ) && ( clken_atvin_vd_flag == 1 )) && (IFD_freq_change_flag == 0) ) {
		if( ( rstn_vdec2_flag == 1 ) && ( clken_atvin_vd_flag == 1 )) {
			AVD_Clock_en_check_flag = 1;
			return TRUE ;
		} else {
			AVD_Clock_en_check_flag = 0;
			return FALSE;
		}
	}
}


/* for reset video decoder*/
unsigned int VDPQ_rtd_inl(unsigned int offset)
{
	unsigned int val = 0;

	val = rtd_inl(offset);


	return val;
}

/* for reset video decoder*/
void VDPQ_rtd_outl(unsigned int offset, unsigned int val)
{

	rtd_outl(offset,val);

}

