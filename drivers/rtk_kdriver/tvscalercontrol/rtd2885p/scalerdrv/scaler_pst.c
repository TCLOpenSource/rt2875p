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
#include <rbus/mdomain_disp_reg.h>
#include <rbus/scaleup_reg.h>
#include <rbus/ppoverlay_reg.h>
#include <rbus/two_step_uzu_reg.h>
#include <rbus/lc_reg.h>
#include <rbus/srnn_reg.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/scaler/scalerstruct.h>
#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerCommon.h>
	#include <scaler/scalerDrvCommon.h>
#else
	#include <scalercommon/scalerCommon.h>
	#include <scalercommon/scalerDrvCommon.h>
#endif
#include <tvscalercontrol/scalerdrv/scalermemory.h>

#include <tvscalercontrol/scalerdrv/scaler_pst.h>
#include <tvscalercontrol/scalerdrv/scaler_i2rnd.h>
#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <tvscalercontrol/vip/ultrazoom.h>
#include "tvscalercontrol/scalerdrv/scalerdisplay.h"
#include <tvscalercontrol/scalerdrv/zoom_smoothtoggle.h>
#include "scaler_vscdev.h"
#include <tvscalercontrol/scalerdrv/scalerdrv.h>
#include <rtd_log/rtd_module_log.h>

//temp add

/***********************************REGISTER RANGE*****************************************************/
//main IMD pst range from vodma to mtg
//30 parts
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_VODMA.h>//1 part
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_vgip.h>//1 part
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_color_mb_peaking.h>//1 part
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_De_xcxl.h>//1 part
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_Dfilter.h>//1 part
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_di.h>//5 part
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_Histogram.h>//1 part
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_Hsd_dither.h>//1 part
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_IDCTI.h>//1 part
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_iedge_smooth.h>//1 part
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_ipq_decontour.h>//1 part
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_Main_Dither.h>//1 part
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_Mpegnr.h>//1 part
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_nr.h>//2 part
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_Peaking.h>//1 part
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_Profile.h>//2 part
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_rgb2yuv.h>//2 part
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_ringfilter.h>//1 part
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_Scaledown.h>//2 part
#include <tvscalercontrol/scalerdrv/i2rnd/i2rnd_Mdomain_cap.h>//3 part
//num==,0->not I2run reg,1->I2run reg,2->I2Run reg,data port reg,->Group3 register
extern unsigned char vgip_0[33];
extern unsigned char color_mb_peaking_0[17];
extern unsigned char De_xcxl_0[47];
extern unsigned char Dfilter_0[2];
extern unsigned char di_0[49];
extern unsigned char di_1[355];
extern unsigned char di_2[108];
extern unsigned char di_3[46];
extern unsigned char di_4[54];
extern unsigned char Histogram_0[43];
extern unsigned char Hsd_dither_0[47];
extern unsigned char IDCTI_0[7];
extern unsigned char iedge_smooth_0[60];
extern unsigned char ipq_decontour_0[5];
extern unsigned char Main_Dither_0[21];
extern unsigned char Mpegnr_0[45];
extern unsigned char nr_0[64];
extern unsigned char nr_1[64];
extern unsigned char Peaking_0[5];
extern unsigned char Profile_0[50];
extern unsigned char Profile_1[54];
extern unsigned char rgb2yuv_0[1];
extern unsigned char rgb2yuv_1[13];
extern unsigned char ringfilter_0[5];
extern unsigned char Scaledown_0[61];
extern unsigned char Scaledown_1[32];
extern unsigned char VODMA_0[512];
extern unsigned char Mdomain_cap_0[62];
extern unsigned char Mdomain_cap_1[28];
extern unsigned char Mdomain_cap_2[5];

//mdisplay  0xb8027300 - 0xB8027608
#define MAIN_MDISP_START_REGISTER					MDOMAIN_DISP_DDR_MainPreVStart_reg
#define MAIN_MDISP_END_REGISTER					MDOMAIN_DISP_DISPM_SUB_PQ_DECMP_SAT_EN_reg
#define MAIN_MDISP_REGISTER_NUM					((MAIN_MDISP_END_REGISTER -MAIN_MDISP_START_REGISTER + 4)/4)
#define MAIN_MDISP_register_range(addr)				(((addr>=MAIN_MDISP_START_REGISTER) &&(addr <=MAIN_MDISP_END_REGISTER)) ? 1: 0 )
#define MAIN_MDISP_DDR_range(addr)					((((addr>=MAIN_MDISP_START_REGISTER) &&(addr <=MDOMAIN_DISP_Disp_main_enable_reg)) \
												|| (addr==MDOMAIN_DISP_DDR_Main_BM_DMA_reg) \
												|| ((addr>=MDOMAIN_DISP_DISPM_MAIN_PQ_DECMP_reg)&&(addr<=MDOMAIN_DISP_DISPM_MAIN_PQ_DECMP_PAIR_reg)) \
												|| ((addr>=MDOMAIN_DISP_DispMain_BoundaryAddr1_reg) &&(addr <=MDOMAIN_DISP_DispMain_BoundaryAddr2_reg)) \
												|| ((addr>=MDOMAIN_DISP_DDR_Main_R_1ST_Addr_reg) &&(addr <=MDOMAIN_DISP_DDR_Main_R_3RD_Addr_reg)) \
												|| ((addr>=MDOMAIN_DISP_DISPM_RM_reg) &&(addr <=MDOMAIN_DISP_Display_byte_channel_swap_reg))) ? 1: 0 )

//uzu  0xb8027300 - 0xB80274F8
#define MAIN_UZU_START_REGISTER				SCALEUP_D_UZU_Globle_Ctrl_reg
#define MAIN_UZU_END_REGISTER					SCALEUP_COEF_TABLE_ERR_reg
#define MAIN_UZU_REGISTER_NUM					((MAIN_UZU_END_REGISTER -MAIN_UZU_START_REGISTER + 4)/4)
#define MAIN_UZU_register_range(addr)				(((addr>=MAIN_UZU_START_REGISTER) &&(addr <=MAIN_UZU_END_REGISTER)) ? 1: 0 )
#define MAIN_UZU_DDR_range(addr)					((((addr>=MAIN_UZU_START_REGISTER) &&(addr <=SCALEUP_DM_UZU_12tap_dering_cross_reg)) \
												|| (addr==SCALEUP_DM_UZUMAIN_H_PARTIAL_DROP_reg) \
												|| ((addr==SCALEUP_DM_UZU_V8CTRL_reg)) \
												|| ((addr>=SCALEUP_DM_DIR_UZU_Ctrl_reg) &&(addr <=SCALEUP_DM_DIR_UZU_AngleCheck_reg))) ? 1: 0 )

//mtg  0xb8027300 - 0xB80274F8
#define MAIN_DTG_START_REGISTER				PPOVERLAY_Display_Timing_CTRL1_reg
#define MAIN_DTG_END_REGISTER					PPOVERLAY_SUB_Active_V_Start_End_reg
#define MAIN_DTG_REGISTER_NUM					((MAIN_DTG_END_REGISTER -MAIN_DTG_START_REGISTER + 4)/4)
#define MAIN_DTG_register_range(addr)				(((addr>=MAIN_DTG_START_REGISTER) &&(addr <=MAIN_DTG_END_REGISTER)) ? 1: 0 )
#define MAIN_DTG_DDR_range(addr)					((((addr>=PPOVERLAY_DH_DEN_Start_End_reg) &&(addr <=PPOVERLAY_MAIN_Active_V_Start_End_reg))) ? 1: 0 )

//dtg iv2dv  0xB8025C50 - 0xB8025C58
#define MAIN_DTG_IV2DV_START_REGISTER				PPOVERLAY_FS_IV_DV_Fine_Tuning2_reg
#define MAIN_DTG_IV2DV_END_REGISTER					PPOVERLAY_FS_IV_DV_Fine_Tuning5_reg
#define MAIN_DTG_IV2DV_REGISTER_NUM					((MAIN_DTG_IV2DV_END_REGISTER -MAIN_DTG_IV2DV_START_REGISTER + 4)/4)
#define MAIN_DTG_IV2DV_register_range(addr)				(((addr>=MAIN_DTG_IV2DV_START_REGISTER) &&(addr <=MAIN_DTG_IV2DV_END_REGISTER)) ? 1: 0 )
#define MAIN_DTG_IV2DV_DDR_range(addr)					((((addr>=PPOVERLAY_FS_IV_DV_Fine_Tuning2_reg) &&(addr <=PPOVERLAY_FS_IV_DV_Fine_Tuning5_reg))) ? 1: 0 )


//orbit 0xb80285d0 - 0xb80285dc
#define MAIN_ORBIT_START_REGISTER                              PPOVERLAY_uzudtg_stage1_time_borrow_reg
#define MAIN_ORBIT_END_REGISTER                                        PPOVERLAY_uzudtg_orbit_main_ctrl2_reg
#define MAIN_ORBIT_REGISTER_NUM                                        ((MAIN_ORBIT_END_REGISTER -MAIN_ORBIT_START_REGISTER + 4)/4)
#define MAIN_ORBIT_register_range(addr)                                (((addr>=MAIN_ORBIT_START_REGISTER) &&(addr <=MAIN_ORBIT_END_REGISTER)) ? 1: 0 )
#define MAIN_ORBIT_DDR_range(addr)                                     ((((addr>=PPOVERLAY_uzudtg_orbit_main_ctrl0_reg) &&(addr <=PPOVERLAY_uzudtg_orbit_main_ctrl2_reg))) ? 1: 0 )

//2stepuzu   0xb8027300 - 0xB80274F8
#define MAIN_2STEPUZU_START_REGISTER				TWO_STEP_UZU_MFLTI_Ctrl_reg
#define MAIN_2STEPUZU_END_REGISTER					TWO_STEP_UZU_ReadData_DATA_C4_SR_reg
#define MAIN_2STEPUZU_REGISTER_NUM					((MAIN_2STEPUZU_END_REGISTER -MAIN_2STEPUZU_START_REGISTER + 4)/4)
#define MAIN_2STEPUZU_register_range(addr)				(((addr>=MAIN_2STEPUZU_START_REGISTER) &&(addr <=MAIN_2STEPUZU_END_REGISTER)) ? 1: 0 )
#define MAIN_2STEPUZU_DDR_range(addr)					(((addr==TWO_STEP_UZU_SR_Ctrl_reg) \
													||((addr>=TWO_STEP_UZU_DM_TWO_STEP_SR_Ctrl_reg) &&(addr <=TWO_STEP_UZU_DM_TWO_STEP_SR_Input_Size_reg))) ? 1: 0 )

//nnsr   0xb8028c00 - 0xb8028c00
#define MAIN_NNSR_START_REGISTER				0 // SRNN_SRNN_CTRL_reg //FIXME: merlin8 compile error
#define MAIN_NNSR_END_REGISTER					0 // SRNN_SRNN_CTRL_reg //FIXME: merlin8 compile error
#define MAIN_NNSR_REGISTER_NUM					((MAIN_NNSR_END_REGISTER -MAIN_NNSR_START_REGISTER + 4)/4)
#define MAIN_NNSR_register_range(addr)				(((addr>=MAIN_NNSR_START_REGISTER) &&(addr <=MAIN_NNSR_END_REGISTER)) ? 1: 0 )
#define MAIN_NNSR_DDR_range(addr)					(((addr>=SRNN_SRNN_CTRL_reg) && (addr <=SRNN_SRNN_CTRL_reg)) ? 1: 0 )

//LC 0xB802E000 - 0xB802E1A0
#define MAIN_LC_START_REGISTER                              LC_LC_Global_Ctrl0_reg
#define MAIN_LC_END_REGISTER                                        LC_LC_Demo_window_ctrl1_reg
#define MAIN_LC_REGISTER_NUM                                        ((MAIN_LC_END_REGISTER -MAIN_LC_START_REGISTER + 4)/4)
#define MAIN_LC_register_range(addr)                                (((addr>=LC_LC_Global_Ctrl0_reg) &&(addr <=LC_LC_diff_ctrl0_reg)) ? 1: 0 )
#define MAIN_LC_DDR_range(addr)                                     ((((addr>=LC_LC_Global_Ctrl0_reg) &&(addr <=LC_LC_Global_Ctrl0_reg)) \
																		|| ((addr>=LC_LC_Global_Ctrl2_reg) &&(addr <=LC_LC_Global_Ctrl3_reg)) \
																		|| ((addr>=LC_LC_PIP_Overlay_APL_Ctrl0_reg) &&(addr <=LC_LC_PIP_Overlay_APL_Ctrl1_reg)) \
																		|| ((addr>=LC_LC_ToneMapping_Yavg_Yin_blend_reg) &&(addr <=LC_LC_Inter_VCtrl1_reg)) \
																		|| ((addr>=LC_LC_ToneMapping_Curve_sel_0_reg) &&(addr <=LC_LC_ToneMapping_Yavg_Yin_blend_factor_1_reg)) \
																		|| ((addr>=LC_LC_Local_Shpnr_1st_gain_8_reg) &&(addr <=LC_LC_PQmask_Shp_Gain_reg)) \
																		|| ((addr>=LC_LC_Local_Shpnr_2nd_gain_8_reg) &&(addr <=LC_LC_diff_ctrl0_reg))) ? 1: 0 )


/***********************************livezoom sub REGISTER RANGE start*****************************************************/
//sub m domain display 0xb8027400 - 0xB8027688
#define SUB_MDISP_START_REGISTER				MDOMAIN_DISP_DDR_SubPreVStart_reg
#define SUB_MDISP_END_REGISTER					MDOMAIN_DISP_DISPM_SUB_PQ_DECMP_SAT_EN_reg
#define SUB_MDISP_REGISTER_NUM					((SUB_MDISP_END_REGISTER -SUB_MDISP_START_REGISTER + 4)/4)
#define SUB_MDISP_register_range(addr)				(((addr>=SUB_MDISP_START_REGISTER) &&(addr <=SUB_MDISP_END_REGISTER)) ? 1: 0 )
#define SUB_MDISP_DDR_range(addr)					((((addr>=SUB_MDISP_START_REGISTER) &&(addr <=MDOMAIN_DISP_Disp_sub_enable_reg)) \
												|| (addr==MDOMAIN_DISP_Sub_disp_blocksel_option_reg) \
												|| ((addr>=MDOMAIN_DISP_DISPM_MAIN_PQ_DECMP_reg)&&(addr<=MDOMAIN_DISP_DISPM_SUB_PQ_DECMP_SAT_EN_reg)) \
												|| ((addr>=MDOMAIN_DISP_DDR_SubMPEn_reg)&&(addr<=MDOMAIN_DISP_DDR_SubBankLimitEndCol_reg)) \
												|| ((addr>=MDOMAIN_DISP_DispSub_BoundaryAddr1_reg) &&(addr <=MDOMAIN_DISP_DispSub_BoundaryAddr2_reg)) \
												|| ((addr>=MDOMAIN_DISP_DDR_Sub_BM_DMA_reg) &&(addr <=MDOMAIN_DISP_DDR_Sub_BM_V_OFFSET_reg)) \
												|| ((addr>=MDOMAIN_DISP_sub_block_read_addcmd_transform_reg) &&(addr <=MDOMAIN_DISP_sub_block_read_c_line_step_reg)) \
												|| (addr ==MDOMAIN_DISP_Display_sub_byte_channel_swap_reg)) ? 1: 0 )

//sub uzu
#define SUB_UZU_START_REGISTER					SCALEUP_DS_UZU_Ctrl_reg
#define SUB_UZU_END_REGISTER					SCALEUP_DS_UZU_PATGEN_FRAME_TOGGLE3_reg
#define SUB_UZU_REGISTER_NUM					((SUB_UZU_END_REGISTER -SUB_UZU_START_REGISTER + 4)/4)
#define SUB_UZU_register_range(addr)				(((addr>=SUB_UZU_START_REGISTER) &&(addr <=SUB_UZU_END_REGISTER)) ? 1: 0 )
#define SUB_UZU_DDR_range(addr)					((((addr>=SUB_UZU_START_REGISTER) &&(addr <=SCALEUP_DS_UZU_Input_Size_reg)) \
												|| ((addr>=SCALEUP_DS_UZUSUB_H_PARTIAL_DROP_reg) &&(addr <=SCALEUP_DS_UZUSUB_V_PARTIAL_DROP_reg))) ? 1: 0 )

//sub DTG
#define SUB_DTG_START_REGISTER					PPOVERLAY_SUB_DEN_H_Start_Width_reg
#define SUB_DTG_END_REGISTER					PPOVERLAY_SUB_circle_center_reg
#define SUB_DTG_REGISTER_NUM					((SUB_DTG_END_REGISTER - SUB_DTG_START_REGISTER + 4)/4)
#define SUB_DTG_register_range(addr)				(((addr>=SUB_DTG_START_REGISTER) &&(addr <=SUB_DTG_END_REGISTER)) ? 1: 0 )
#define SUB_DTG_DDR_range(addr)					((((addr>=SUB_DTG_START_REGISTER) &&(addr <=SUB_DTG_END_REGISTER))) ? 1: 0 )

//orbit
#define SUB_ORBIT_START_REGISTER                               PPOVERLAY_uzudtg_orbit_sub_ctrl3_reg
#define SUB_ORBIT_END_REGISTER                                 PPOVERLAY_uzudtg_orbit_sub_ctrl1_reg
#define SUB_ORBIT_REGISTER_NUM                                 ((SUB_ORBIT_END_REGISTER -SUB_ORBIT_START_REGISTER + 4)/4)
#define SUB_ORBIT_register_range(addr)                         (((addr>=SUB_ORBIT_START_REGISTER) &&(addr <=SUB_ORBIT_END_REGISTER)) ? 1: 0 )
#define SUB_ORBIT_DDR_range(addr)                                      ((((addr>=PPOVERLAY_uzudtg_orbit_sub_ctrl3_reg) &&(addr <=PPOVERLAY_uzudtg_orbit_sub_ctrl1_reg))) ? 1: 0 )

/***********************************livezoom sub REGISTER RANGE ended*****************************************************/

/*================================== PST COMMAND==============================*/
#define PST_I_M_REGISTER_NUMBER (VGIP_REGISTER_NUM_0 + COLOR_MB_PEAKING_REGISTER_NUM_0 + DE_XCXL_REGISTER_NUM_0 + DFILTER_REGISTER_NUM_0 + DI_REGISTER_NUM_0+DI_REGISTER_NUM_1+DI_REGISTER_NUM_2+DI_REGISTER_NUM_3+DI_REGISTER_NUM_4\
								+ HISTOGRAM_REGISTER_NUM_0 + HSD_DITHER_REGISTER_NUM_0 + IDCTI_REGISTER_NUM_0 + IEDGE_SMOOTH_REGISTER_NUM_0 + IPQ_DECONTOUR_REGISTER_NUM_0 + MAIN_DITHER_REGISTER_NUM_0 + MPEGNR_REGISTER_NUM_0\
								+ NR_REGISTER_NUM_0 + NR_REGISTER_NUM_1 + PEAKING_REGISTER_NUM_0 + PROFILE_REGISTER_NUM_0 + PROFILE_REGISTER_NUM_1 + RGB2YUV_REGISTER_NUM_0 + RGB2YUV_REGISTER_NUM_1 + RINGFILTER_REGISTER_NUM_0\
								+ SCALEDOWN_REGISTER_NUM_0 + SCALEDOWN_REGISTER_NUM_1 + VODMA_REGISTER_NUM_0 + MDOMAIN_CAP_REGISTER_NUM_0 + MDOMAIN_CAP_REGISTER_NUM_1 + MDOMAIN_CAP_REGISTER_NUM_2)
#define PST_I_S_REGISTER_NUMBER 500//default value nobody use
#define PST_DISPM_REGISTER_NUMBER (MAIN_MDISP_REGISTER_NUM + MAIN_UZU_REGISTER_NUM + MAIN_DTG_REGISTER_NUM + MAIN_2STEPUZU_REGISTER_NUM + MAIN_ORBIT_REGISTER_NUM + MAIN_LC_REGISTER_NUM + MAIN_DTG_IV2DV_REGISTER_NUM + MAIN_NNSR_REGISTER_NUM)
#define PST_DISPS_REGISTER_NUMBER 500//default value nobody use
#define PST_GDMA_REGISTER_NUMBER 500//default value nobody use

#define PST_DISPM_BLOCK_SIZE ((PST_DISPM_REGISTER_NUMBER + ((PST_DISPM_REGISTER_NUMBER%2) ? 1 : 0)) / 2 * 16) //queue size(total block size) = (regisrer num/2) * 128bits = (regisrer num/2) * 16bytes
#define PST_IDOMM_BLOCK_SIZE ((PST_I_M_REGISTER_NUMBER + ((PST_I_M_REGISTER_NUMBER%2) ? 1 : 0)) / 2 * 16) //queue size(total block size) = (regisrer num/2) * 128bits = (regisrer num/2) * 16bytes


#define PST_DMA_LENGTH 24 //for 96-bytes align unit 64 bits, 24*8 = 192 bytes //16
#define PST_DMA_WATERLEVEL 96

#define PST_REGISTER_DDR_SIZE  8
#define REGISTER_VALUE_SIZE  4

#define PST_ENABLE_ADDR_SIZE 4
#define PST_WRITE_CMD_ADDR_SIZE 4
#define PST_READ_CMD_ADDR_SIZE 4
#define PST_BUF_DELAY_CNT_SIZE (4*(PST_CMD_NUMBER+1))
#define PST_INFO_SIZE (PST_ENABLE_ADDR_SIZE + PST_WRITE_CMD_ADDR_SIZE + PST_READ_CMD_ADDR_SIZE)
//PST

#define _BIT(x)       (1UL <<x)

/*================================== Variables ==============================*/



PST_DISPM_SCPU_VIR_START_ADDR pst_dispm_scpu_vir_addr;
PST_IMD_SCPU_VIR_START_ADDR pst_imd_scpu_vir_addr;


#ifdef CONFIG_PST_ENABLE
static PST_START_ADDR pst_st_addr;
#ifdef CONFIG_ARM64
static unsigned long pst_main_md_addr_unalign = 0;
static unsigned long pst_sub_md_addr_unalign = 0;
static unsigned long pst_main_idomain_addr_unalign = 0;
static unsigned long pst_sub_idomain_addr_unalign = 0;
#else
static unsigned int pst_main_md_addr_unalign = 0;
static unsigned int pst_sub_md_addr_unalign = 0;
static unsigned int pst_main_idomain_addr_unalign = 0;
static unsigned int pst_sub_idomain_addr_unalign = 0;
#endif

static unsigned int pst_queue_size = 0;
static unsigned int idom_pst_queue_size = 0;

static PST_STEP_SIZE pst_step_size;

SCALER_PST_SUSPEND_RESUME_T scaler_pst_suspend_resume;

//static PST_CMD_BUF pst_cmd_buf;

/***********************************PST FUNCTION*****************************************************/
unsigned char drvif_scaler_pst_get_mode_enable(_PST_MODE_T mode)
{
	return pst_st_addr.pst_mode & (_BIT(mode));
}

void drvif_scaler_pst_set_mode(_PST_MODE_T mode)
{
	pst_st_addr.pst_mode |= (_BIT(mode));
}

/*========================================started these apis are used by ddomain pst started========================================*/
unsigned char Scaler_main_md_pst_get_enable(void)
{
	if (drvif_scaler_pst_get_mode_enable(_MAIN_MD)) {
		if (pst_dispm_scpu_vir_addr.pst_enable_vir_addr) {
			return (*(volatile unsigned char *)(pst_dispm_scpu_vir_addr.pst_enable_vir_addr));
		} else {
			return 0;
		}
	} else {
		return 0;
	}
}
void Scaler_main_md_pst_set_enable(unsigned char enable)
{
	if (pst_dispm_scpu_vir_addr.pst_enable_vir_addr) {
		*(volatile unsigned char *)(pst_dispm_scpu_vir_addr.pst_enable_vir_addr) = enable;
	}
}

unsigned char Scaler_pst_main_md_get_read_cmd_num(void)
{
	if (pst_dispm_scpu_vir_addr.pst_dispm_read_cmd_vir_addr) {
		return (*(volatile unsigned char *)(pst_dispm_scpu_vir_addr.pst_dispm_read_cmd_vir_addr));
	} else {
		return 0;
	}
}
void Scaler_pst_main_md_set_read_cmd_num(unsigned char num)
{
	if (pst_dispm_scpu_vir_addr.pst_dispm_read_cmd_vir_addr) {
		*(volatile unsigned char *)(pst_dispm_scpu_vir_addr.pst_dispm_read_cmd_vir_addr) = num;
	}
}
unsigned char Scaler_pst_main_md_get_write_cmd_num(void)
{
	if (pst_dispm_scpu_vir_addr.pst_dispm_write_cmd_vir_addr) {
		return (*(volatile unsigned char *)(pst_dispm_scpu_vir_addr.pst_dispm_write_cmd_vir_addr));
	} else {
		return 0;
	}
}

void Scaler_pst_main_md_set_write_cmd_num(void)
{
	if (pst_dispm_scpu_vir_addr.pst_dispm_write_cmd_vir_addr) {
		unsigned char cmd_num = (*(volatile unsigned char *)(pst_dispm_scpu_vir_addr.pst_dispm_write_cmd_vir_addr));
		if ((cmd_num + 1 )> PST_CMD_NUMBER) {
			cmd_num = 0;
		} else {
			cmd_num = cmd_num + 1;
		}
		*(volatile unsigned char *)(pst_dispm_scpu_vir_addr.pst_dispm_write_cmd_vir_addr) = cmd_num;
	}
}
void Scaler_pst_main_md_write_default_cmd_num(void)
{
	if (pst_dispm_scpu_vir_addr.pst_dispm_write_cmd_vir_addr) {
		*(volatile unsigned char *)(pst_dispm_scpu_vir_addr.pst_dispm_write_cmd_vir_addr) = 0;
	}
}

//sub pst driver
unsigned char Scaler_sub_md_pst_get_enable(void)
{
	if (pst_dispm_scpu_vir_addr.pst_disps_enable_vir_addr) {
		return (*(volatile unsigned char *)(pst_dispm_scpu_vir_addr.pst_disps_enable_vir_addr));
	} else {
		return 0;
	}
}
void Scaler_sub_md_pst_set_enable(unsigned char enable)
{
	if (pst_dispm_scpu_vir_addr.pst_disps_enable_vir_addr) {
		*(volatile unsigned char *)(pst_dispm_scpu_vir_addr.pst_disps_enable_vir_addr) = enable;
	}
}

unsigned char Scaler_pst_sub_md_get_read_cmd_num(void)
{
	if (pst_dispm_scpu_vir_addr.pst_disps_read_cmd_vir_addr) {
		return (*(volatile unsigned char *)(pst_dispm_scpu_vir_addr.pst_disps_read_cmd_vir_addr));
	} else {
		return 0;
	}
}
void Scaler_pst_sub_md_set_read_cmd_num(unsigned char enable)
{
	if (pst_dispm_scpu_vir_addr.pst_disps_read_cmd_vir_addr) {
		*(volatile unsigned char *)(pst_dispm_scpu_vir_addr.pst_disps_read_cmd_vir_addr) = enable;
	}
}
unsigned char Scaler_pst_sub_md_get_write_cmd_num(void)
{
	if (pst_dispm_scpu_vir_addr.pst_disps_write_cmd_vir_addr) {
		return (*(volatile unsigned char *)(pst_dispm_scpu_vir_addr.pst_disps_write_cmd_vir_addr));
	} else {
		return 0;
	}
}
void Scaler_pst_sub_md_set_write_cmd_num(void)
{
	if (pst_dispm_scpu_vir_addr.pst_disps_write_cmd_vir_addr) {
		//Eric@20170822
		unsigned char cmd_num = (*(volatile unsigned char *)(pst_dispm_scpu_vir_addr.pst_disps_write_cmd_vir_addr));
		if ((cmd_num + 1 )> PST_CMD_NUMBER) {
			cmd_num = 0;
		} else {
			cmd_num = cmd_num + 1;
		}
		*(volatile unsigned char *)(pst_dispm_scpu_vir_addr.pst_disps_write_cmd_vir_addr) = cmd_num;
	}
}
void Scaler_pst_sub_md_write_default_cmd_num(void)
{
	if (pst_dispm_scpu_vir_addr.pst_disps_write_cmd_vir_addr) {
		*(volatile unsigned char *)(pst_dispm_scpu_vir_addr.pst_disps_write_cmd_vir_addr) = 0;
	}
}

/*========================================ended these apis are used by ddomain pst ended==========================================*/

unsigned char Scaler_main_imd_pst_get_enable(void)
{
	if (pst_imd_scpu_vir_addr.pst_enable_vir_addr&&pst_imd_scpu_vir_addr.idom_pst_enable_vir_addr) {
		return ((*(volatile unsigned char *)(pst_imd_scpu_vir_addr.pst_enable_vir_addr))&&(*(volatile unsigned char *)(pst_imd_scpu_vir_addr.idom_pst_enable_vir_addr)));
	} else {
		return 0;
	}
}
void Scaler_main_imd_pst_set_enable(unsigned char enable)
{
	/*need open i domain and d domain enable switch*/
	if (pst_imd_scpu_vir_addr.pst_enable_vir_addr) {
		*(volatile unsigned char *)(pst_imd_scpu_vir_addr.pst_enable_vir_addr) = enable;
	}
	rtd_pr_smt_emerg("Scaler_main_imd_pst_set_enable:pst_imd_scpu_vir_addr.pst_enable_vir_addr =%lx\n",pst_imd_scpu_vir_addr.pst_enable_vir_addr);
	rtd_pr_smt_emerg("Scaler_main_imd_pst_set_enable:*(volatile unsigned char *)(pst_imd_scpu_vir_addr.pst_enable_vir_addr) =%d\n",*(volatile unsigned char *)(pst_imd_scpu_vir_addr.pst_enable_vir_addr));


	if (pst_imd_scpu_vir_addr.idom_pst_enable_vir_addr) {
		*(volatile unsigned char *)(pst_imd_scpu_vir_addr.idom_pst_enable_vir_addr) = enable;
	}
	rtd_pr_smt_emerg("Scaler_main_imd_pst_set_enable:pst_imd_scpu_vir_addr.idom_pst_enable_vir_addr =%lx\n",pst_imd_scpu_vir_addr.idom_pst_enable_vir_addr);
	rtd_pr_smt_emerg("Scaler_main_imd_pst_set_enable:*(volatile unsigned char *)(pst_imd_scpu_vir_addr.idom_pst_enable_vir_addr) =%d\n",*(volatile unsigned char *)(pst_imd_scpu_vir_addr.idom_pst_enable_vir_addr));

}

unsigned int Scaler_s0_pst_cmd_buffer_write_offset_addr(unsigned int addr)
{
	unsigned int curr_addr= 0;
	curr_addr = addr + Scaler_pst_main_md_get_write_cmd_num() * pst_st_addr.dispm_cmd_buffer_size;
	return curr_addr;
}
unsigned int Scaler_s0_pst_cmd_buffer_read_offset_addr(unsigned int addr)
{
	unsigned int curr_addr= 0;
	curr_addr = addr + Scaler_pst_main_md_get_read_cmd_num() * pst_st_addr.dispm_cmd_buffer_size;
	return curr_addr;
}

unsigned int Scaler_s1_pst_cmd_buffer_write_offset_addr(unsigned int addr)
{
	unsigned int curr_addr= 0;
	curr_addr = addr + Scaler_pst_sub_md_get_write_cmd_num() * pst_st_addr.dispm_cmd_buffer_size;
	return curr_addr;
	}
unsigned int Scaler_s1_pst_cmd_buffer_read_offset_addr(unsigned int addr)
{
	unsigned int curr_addr= 0;
	curr_addr = addr + Scaler_pst_sub_md_get_read_cmd_num() * pst_st_addr.dispm_cmd_buffer_size;
	return curr_addr;
}

unsigned int Scaler_PST_IP_register_get_cur_write_addr(unsigned int addr,unsigned char display)
{
	unsigned int cur_addr =0;
	unsigned int register_offset_addr =0;

	if (display == I2RND_MAIN_S0_TABLE) {
			if (MAIN_MDISP_register_range(addr)) {
				if (MAIN_MDISP_DDR_range(addr)) {
					if (pst_dispm_scpu_vir_addr.s0_mdisp_st_vir_addr) {
						register_offset_addr = ((addr-MAIN_MDISP_START_REGISTER) /REGISTER_VALUE_SIZE) *PST_REGISTER_DDR_SIZE;
						cur_addr =  Scaler_s0_pst_cmd_buffer_write_offset_addr(pst_dispm_scpu_vir_addr.s0_mdisp_st_vir_addr) + register_offset_addr;
					}
				}
			} else if (MAIN_UZU_register_range(addr)) {
				if (MAIN_UZU_DDR_range(addr)) {
					if (pst_dispm_scpu_vir_addr.s0_uzu_st_vir_addr) {
						register_offset_addr = ((addr-MAIN_UZU_START_REGISTER) /REGISTER_VALUE_SIZE) *PST_REGISTER_DDR_SIZE;
						cur_addr =  Scaler_s0_pst_cmd_buffer_write_offset_addr(pst_dispm_scpu_vir_addr.s0_uzu_st_vir_addr) + register_offset_addr;
					}
				}
			}  else if (MAIN_DTG_register_range(addr)) {
				if (MAIN_DTG_DDR_range(addr)) {
					if (pst_dispm_scpu_vir_addr.s0_dtg_st_vir_addr) {
						register_offset_addr = ((addr-MAIN_DTG_START_REGISTER) /REGISTER_VALUE_SIZE) *PST_REGISTER_DDR_SIZE;
						cur_addr =  Scaler_s0_pst_cmd_buffer_write_offset_addr(pst_dispm_scpu_vir_addr.s0_dtg_st_vir_addr) + register_offset_addr;
					}
				}
			}   else if (MAIN_2STEPUZU_register_range(addr)) {
				if (MAIN_2STEPUZU_DDR_range(addr)) {
					if (pst_dispm_scpu_vir_addr.s0_2stepuzu_st_vir_addr) {
						register_offset_addr = ((addr-MAIN_2STEPUZU_START_REGISTER) /REGISTER_VALUE_SIZE) *PST_REGISTER_DDR_SIZE;
						cur_addr =  Scaler_s0_pst_cmd_buffer_write_offset_addr(pst_dispm_scpu_vir_addr.s0_2stepuzu_st_vir_addr) + register_offset_addr;
					}
				}
			}
	}
	else if (display == I2RND_SUB_S1_TABLE) {
			if (SUB_MDISP_register_range(addr)) {
				if (SUB_MDISP_DDR_range(addr)) {
					if (pst_dispm_scpu_vir_addr.s1_mdisp_st_vir_addr) {
						register_offset_addr = ((addr-SUB_MDISP_START_REGISTER) /REGISTER_VALUE_SIZE) *PST_REGISTER_DDR_SIZE;
						cur_addr =  Scaler_s1_pst_cmd_buffer_write_offset_addr(pst_dispm_scpu_vir_addr.s1_mdisp_st_vir_addr) + register_offset_addr;
					}
				}
			} else if (SUB_UZU_register_range(addr)) {
				if (SUB_UZU_DDR_range(addr)) {
					if (pst_dispm_scpu_vir_addr.s1_uzu_st_vir_addr) {
						register_offset_addr = ((addr-SUB_UZU_START_REGISTER) /REGISTER_VALUE_SIZE) *PST_REGISTER_DDR_SIZE;
						cur_addr =  Scaler_s1_pst_cmd_buffer_write_offset_addr(pst_dispm_scpu_vir_addr.s1_uzu_st_vir_addr) + register_offset_addr;
					}
				}
			}  else if (SUB_DTG_register_range(addr)) {
				if (SUB_DTG_DDR_range(addr)) {
					if (pst_dispm_scpu_vir_addr.s1_dtg_st_vir_addr) {
						register_offset_addr = ((addr-SUB_DTG_START_REGISTER) /REGISTER_VALUE_SIZE) *PST_REGISTER_DDR_SIZE;
						cur_addr =  Scaler_s1_pst_cmd_buffer_write_offset_addr(pst_dispm_scpu_vir_addr.s1_dtg_st_vir_addr) + register_offset_addr;
					}
				}
			}
	}
	return cur_addr;
}

unsigned int Scaler_PST_IP_register_get_cur_read_addr(unsigned int addr,unsigned char display)
{
	unsigned int cur_addr =0;
	unsigned int register_offset_addr =0;

	if (display == I2RND_MAIN_S0_TABLE) {
			if (MAIN_MDISP_register_range(addr)) {
				if (MAIN_MDISP_DDR_range(addr)) {
					if (pst_dispm_scpu_vir_addr.s0_mdisp_st_vir_addr) {
						register_offset_addr = ((addr-MAIN_MDISP_START_REGISTER) /REGISTER_VALUE_SIZE) *PST_REGISTER_DDR_SIZE;
						cur_addr =  Scaler_s0_pst_cmd_buffer_read_offset_addr(pst_dispm_scpu_vir_addr.s0_mdisp_st_vir_addr) + register_offset_addr;
					}
				}
			} else if (MAIN_UZU_register_range(addr)) {
				if (MAIN_UZU_DDR_range(addr)) {
					if (pst_dispm_scpu_vir_addr.s0_uzu_st_vir_addr) {
						register_offset_addr = ((addr-MAIN_UZU_START_REGISTER) /REGISTER_VALUE_SIZE) *PST_REGISTER_DDR_SIZE;
						cur_addr =  Scaler_s0_pst_cmd_buffer_read_offset_addr(pst_dispm_scpu_vir_addr.s0_uzu_st_vir_addr) + register_offset_addr;
					}
				}
			}  else if (MAIN_DTG_register_range(addr)) {
				if (MAIN_DTG_DDR_range(addr)) {
					if (pst_dispm_scpu_vir_addr.s0_dtg_st_vir_addr) {
						register_offset_addr = ((addr-MAIN_DTG_START_REGISTER) /REGISTER_VALUE_SIZE) *PST_REGISTER_DDR_SIZE;
						cur_addr =  Scaler_s0_pst_cmd_buffer_read_offset_addr(pst_dispm_scpu_vir_addr.s0_dtg_st_vir_addr) + register_offset_addr;
					}
				}
			}
		//}
	}
	else if (display == I2RND_SUB_S1_TABLE) {
			if (SUB_MDISP_register_range(addr)) {
				if (SUB_MDISP_DDR_range(addr)) {
					if (pst_dispm_scpu_vir_addr.s1_mdisp_st_vir_addr) {
						register_offset_addr = ((addr-SUB_MDISP_START_REGISTER) /REGISTER_VALUE_SIZE) *PST_REGISTER_DDR_SIZE;
						cur_addr =  Scaler_s1_pst_cmd_buffer_read_offset_addr(pst_dispm_scpu_vir_addr.s1_mdisp_st_vir_addr) + register_offset_addr;
				}
			}
			} else if (SUB_UZU_register_range(addr)) {
				if (SUB_UZU_DDR_range(addr)) {
					if (pst_dispm_scpu_vir_addr.s1_uzu_st_vir_addr) {
						register_offset_addr = ((addr-SUB_UZU_START_REGISTER) /REGISTER_VALUE_SIZE) *PST_REGISTER_DDR_SIZE;
						cur_addr =  Scaler_s1_pst_cmd_buffer_read_offset_addr(pst_dispm_scpu_vir_addr.s1_uzu_st_vir_addr) + register_offset_addr;
					}
				}
			}  else if (SUB_DTG_register_range(addr)) {
				if (SUB_DTG_DDR_range(addr)) {
					if (pst_dispm_scpu_vir_addr.s1_dtg_st_vir_addr) {
						register_offset_addr = ((addr-SUB_DTG_START_REGISTER) /REGISTER_VALUE_SIZE) *PST_REGISTER_DDR_SIZE;
						cur_addr =  Scaler_s1_pst_cmd_buffer_read_offset_addr(pst_dispm_scpu_vir_addr.s1_dtg_st_vir_addr) + register_offset_addr;
					}
		}
	}
		//}
	}
	return cur_addr;
}

void Scaler_pst_set_write_point(_PST_MODE_T  mode)
{
	if (mode == _MAIN_MD) {
		pst_i2rnd_dma_wrt_pnt2_RBUS wrt_pnt2_reg;
		wrt_pnt2_reg.regValue = rtd_inl(PST_I2RND_DMA_WRT_PNT2_reg);
		wrt_pnt2_reg.pst_dispm_write_pnt = Scaler_pst_main_md_get_write_cmd_num();
		rtd_outl(PST_I2RND_DMA_WRT_PNT2_reg, wrt_pnt2_reg.regValue);
	}
	else if (mode == _SUB_MD) {
		pst_i2rnd_dma_wrt_pnt2_RBUS wrt_pnt2_reg;
		wrt_pnt2_reg.regValue = rtd_inl(PST_I2RND_DMA_WRT_PNT2_reg);
		wrt_pnt2_reg.pst_disps_write_pnt = Scaler_pst_sub_md_get_write_cmd_num();
		rtd_outl(PST_I2RND_DMA_WRT_PNT2_reg, wrt_pnt2_reg.regValue);
	}
}
unsigned char Scaler_pst_get_write_point(_PST_MODE_T  mode)
{
		pst_i2rnd_dma_wrt_pnt2_RBUS wrt_pnt2_reg;
		wrt_pnt2_reg.regValue = rtd_inl(PST_I2RND_DMA_WRT_PNT2_reg);

	if (mode == _MAIN_MD) {
		return wrt_pnt2_reg.pst_dispm_write_pnt ;
	}
	else if (mode == _SUB_MD) {
		return wrt_pnt2_reg.pst_disps_write_pnt ;
	}
	else {
		return 0;
	}
}
unsigned char Scaler_pst_get_read_point(_PST_MODE_T mode)
{
	pst_i2rnd_dma_read_pnt2_RBUS read_pnt2_reg;
	read_pnt2_reg.regValue = rtd_inl(PST_I2RND_DMA_READ_PNT2_reg);

	if (mode == _MAIN_MD) {
		return read_pnt2_reg.pst_dispm_read_pnt;
	}
	else if (mode == _SUB_MD) {
		return read_pnt2_reg.pst_disps_read_pnt;
	}
	else {
		return 0;
	}
}

void Scaler_pst_buffer_copy(_PST_MODE_T mode)
{
	unsigned char i=0;
	unsigned char j=0;
	unsigned long curr_pst_md_write_buffer_addr = 0;

	if(mode == _MAIN_MD){
		j = Scaler_pst_get_write_point(_MAIN_MD);
	    //rtd_pr_smt_emerg("~~~~~~pst write buffer(%d)\n",j);

		//Eric@0310 if cmd_num != 0 must copy to default buffer first
		if(j != 0) {
	            curr_pst_md_write_buffer_addr=pst_dispm_scpu_vir_addr.s0_mdisp_st_vir_addr+j*pst_st_addr.dispm_cmd_buffer_size;
	            memcpy((unsigned char *)pst_dispm_scpu_vir_addr.s0_mdisp_st_vir_addr, (unsigned char *)curr_pst_md_write_buffer_addr, sizeof(unsigned char) * pst_st_addr.dispm_cmd_buffer_size);
	    }
		for (i=1;i<(PST_CMD_NUMBER+1);i++) {
			if (mode == _MAIN_MD) {
					curr_pst_md_write_buffer_addr=pst_dispm_scpu_vir_addr.s0_mdisp_st_vir_addr+i* pst_st_addr.dispm_cmd_buffer_size;
				memcpy((unsigned char *)curr_pst_md_write_buffer_addr, (unsigned char *)pst_dispm_scpu_vir_addr.s0_mdisp_st_vir_addr, sizeof(unsigned char) *  pst_st_addr.dispm_cmd_buffer_size);
			}
		}
	}
	else if(mode == _SUB_MD){
		j = Scaler_pst_get_write_point(_SUB_MD);
		//Eric@0310 if cmd_num != 0 must copy to default buffer first
		if(j != 0) {
	            curr_pst_md_write_buffer_addr = pst_dispm_scpu_vir_addr.s1_mdisp_st_vir_addr + j * pst_st_addr.dispm_cmd_buffer_size;
	            memcpy((unsigned char *)pst_dispm_scpu_vir_addr.s1_mdisp_st_vir_addr, (unsigned char *)curr_pst_md_write_buffer_addr, sizeof(unsigned char) * pst_st_addr.dispm_cmd_buffer_size);
					}
		for (i=1;i<(PST_CMD_NUMBER+1);i++) {
			if (mode == _SUB_MD) {
				curr_pst_md_write_buffer_addr = pst_dispm_scpu_vir_addr.s1_mdisp_st_vir_addr + i * pst_st_addr.dispm_cmd_buffer_size;
				memcpy((unsigned char *)curr_pst_md_write_buffer_addr, (unsigned char *)pst_dispm_scpu_vir_addr.s1_mdisp_st_vir_addr, sizeof(unsigned char) *  pst_st_addr.dispm_cmd_buffer_size);
			}
		}
	}
}

#define IMD_PST_HW_INDEX_UPDATE_LINE 0x1EA
void scaler_imd_pst_enable_interrupt(void)
{
	ppoverlay_dtg_lc_RBUS dtg_lc_reg;
	ppoverlay_dtg_ie_RBUS dtg_ie_reg;
	dtg_lc_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_LC_reg);
	dtg_ie_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_ie_reg);

	if(!((dtg_ie_reg.dtg_ie)&&(dtg_ie_reg.dtg_vlc_ie)&&(dtg_lc_reg.dtg_vlcen)&&(dtg_lc_reg.dtg_vlc_mode)&&(dtg_lc_reg.dtg_vlc_src_sel == 1)
		&&(dtg_lc_reg.dtg_vln == IMD_PST_HW_INDEX_UPDATE_LINE)))
		drvif_scaler_ddomain_DTG_line_compare_irq(1, 1, 1, IMD_PST_HW_INDEX_UPDATE_LINE);

}

void drvif_scaler_pst_initial(_PST_MODE_T mode)
{
	//unsigned int block_mode = 0;
	//share memory allocation
	unsigned long cache_addr;
	unsigned int blk_size_mod = 0;
	unsigned int pst_blk_size = 0;
	unsigned int pst_allocate_size = 0;

	unsigned long idom_cache_addr;
	unsigned int idom_blk_size_mod = 0;
	unsigned int idom_pst_blk_size = 0;
	unsigned int idom_pst_allocate_size = 0;
	#ifdef CONFIG_ARM64
	unsigned long align_addr = 0, query_addr = 0;
	#else
	unsigned int align_addr = 0, query_addr = 0;
	#endif

	//align total queue size
	if ((mode == _MAIN_MD) || (mode == _SUB_MD)) {
		pst_blk_size = PST_DISPM_BLOCK_SIZE;
		blk_size_mod = pst_blk_size % (PST_DMA_LENGTH*8);//No remain so need align length size length*64bits
		if(blk_size_mod != 0){
			pst_blk_size = pst_blk_size + ((PST_DMA_LENGTH*8) - blk_size_mod);
		}

		//merlin4 memory do 96 bytes align @Crixus 20180321
		pst_blk_size = dvr_size_alignment(pst_blk_size);

		pst_queue_size = pst_blk_size*(PST_CMD_NUMBER+1);
		pst_allocate_size =pst_queue_size+PST_INFO_SIZE;
		rtd_pr_smt_emerg("[%s]pst_blk_size=%d,pst_queue_size=%d,pst_allocate_size=%d\n",__FUNCTION__,pst_blk_size,pst_queue_size,pst_allocate_size);

	} else if ((mode == _MAIN_VO_IMD) || (mode == _SUB_VO_IMD)){
		idom_pst_blk_size = PST_IDOMM_BLOCK_SIZE;
		idom_blk_size_mod = idom_pst_blk_size % (PST_DMA_LENGTH*8);//No remain so need align length size length*64bits
		if(idom_blk_size_mod != 0){
			idom_pst_blk_size = idom_pst_blk_size + ((PST_DMA_LENGTH*8) - idom_blk_size_mod);
		}

		//merlin4 memory do 96 bytes align @Crixus 20180321
		idom_pst_blk_size = dvr_size_alignment(idom_pst_blk_size);

		idom_pst_queue_size = idom_pst_blk_size*(PST_CMD_NUMBER+1);
		idom_pst_allocate_size =idom_pst_queue_size+PST_INFO_SIZE+PST_BUF_DELAY_CNT_SIZE;
		rtd_pr_smt_emerg("[%s]idom_pst_queue_size=%d,idom_pst_blk_size=%d,idom_pst_allocate_size=%d\n",__FUNCTION__,idom_pst_blk_size,idom_pst_queue_size,idom_pst_allocate_size);

		pst_blk_size = PST_DISPM_BLOCK_SIZE;
		blk_size_mod = pst_blk_size % (PST_DMA_LENGTH*8);//No remain so need align length size length*64bits
		if(blk_size_mod != 0){
			pst_blk_size = pst_blk_size + ((PST_DMA_LENGTH*8) - blk_size_mod);
		}

		//merlin4 memory do 96 bytes align @Crixus 20180321
		pst_blk_size = dvr_size_alignment(pst_blk_size);

		pst_queue_size = pst_blk_size*(PST_CMD_NUMBER+1);
		pst_allocate_size =pst_queue_size+PST_INFO_SIZE;
		rtd_pr_smt_emerg("[%s]pst_blk_size=%d,pst_queue_size=%d,pst_allocate_size=%d\n",__FUNCTION__,pst_blk_size,pst_queue_size,pst_allocate_size);

	}


	//alloc different mode memory
	if (mode == _MAIN_MD) {
		/*allocate d domain pst buffer*/
		if (pst_st_addr.dispm_st_addr == 0) {
			//merlin4 memory do 96 bytes align @Crixus 20180321
			pst_allocate_size = dvr_size_alignment_cma(pst_allocate_size);//size do 96-bytes align first
			cache_addr = (unsigned long)dvr_malloc_uncached_specific(pst_allocate_size, GFP_DCU1_FIRST, (void*) & (pst_dispm_scpu_vir_addr.s0_mdisp_st_vir_addr));
			pst_main_md_addr_unalign = cache_addr;//save unalign address for free memory.
			#ifdef CONFIG_ARM64
			query_addr = (unsigned long)dvr_to_phys((void*)cache_addr);
			align_addr = dvr_memory_alignment((unsigned long)query_addr, pst_allocate_size);
			if (align_addr == (unsigned long)NULL)
        			BUG();
			#else
			query_addr = (unsigned int)dvr_to_phys((void*)cache_addr);
			align_addr = dvr_memory_alignment((unsigned long)query_addr, pst_allocate_size);
			if (align_addr == (unsigned int)NULL)
        			BUG();
			#endif
			//do 96bytes align make DMA speedup
			pst_st_addr.dispm_st_addr = align_addr;
			pst_st_addr.pst_enable_addr = pst_st_addr.dispm_st_addr + pst_queue_size;
			pst_st_addr.write_dispm_cmd_num_addr = pst_st_addr.pst_enable_addr + PST_ENABLE_ADDR_SIZE;
			pst_st_addr.read_dispm_cmd_num_addr = pst_st_addr.write_dispm_cmd_num_addr + PST_WRITE_CMD_ADDR_SIZE;
		}
		if (pst_st_addr.dispm_st_addr == 0) {
			rtd_pr_smt_debug("[PST]malloc pst buffer fail .......................................\n");

			rtd_pr_smt_debug("[PST]dispm_st_addr = %x\n", pst_st_addr.dispm_st_addr);

			return;
		} else {
			memset((unsigned char *)pst_dispm_scpu_vir_addr.s0_mdisp_st_vir_addr, 0, sizeof(unsigned char) * pst_allocate_size);
		}
		pst_st_addr.dispm_cmd_buffer_size = pst_blk_size;
		pst_st_addr.pst_mode |= (_BIT(_MAIN_MD));

		rtd_pr_smt_emerg("[%s][pst_dispm_scpu_vir_addr.s0_mdisp_st_vir_addr=%lx,pst_st_addr.dispm_st_addr=%x\n",__FUNCTION__, pst_dispm_scpu_vir_addr.s0_mdisp_st_vir_addr,pst_st_addr.dispm_st_addr);
		rtd_pr_smt_emerg("[%s][pst_st_addr.pst_enable_addr=%x,pst_st_addr.pst_mode=%d\n",__FUNCTION__, pst_st_addr.pst_enable_addr,pst_st_addr.pst_mode );

		Scaler_pst_setbuffer(_MAIN_MD);
	} else if (mode == _SUB_MD) {
		if (pst_st_addr.disps_st_addr == 0) {
			//disps memory allocate
			pst_allocate_size = dvr_size_alignment_cma(pst_allocate_size);//size do 96-bytes align first
			cache_addr= (unsigned long)dvr_malloc_uncached_specific(pst_allocate_size, GFP_DCU1_FIRST, (void*) & (pst_dispm_scpu_vir_addr.s1_mdisp_st_vir_addr));
			pst_sub_md_addr_unalign = cache_addr;//save unalign address for free memory.
			#ifdef CONFIG_ARM64
			query_addr = (unsigned long)dvr_to_phys((void*)cache_addr);
			align_addr = dvr_memory_alignment((unsigned long)query_addr, pst_allocate_size);
			if (align_addr == (unsigned long)NULL)
        			BUG();
			#else
			query_addr = (unsigned int)dvr_to_phys((void*)cache_addr);
			align_addr = dvr_memory_alignment((unsigned long)query_addr, pst_allocate_size);
			if (align_addr == (unsigned int)NULL)
        			BUG();
			#endif
			//do 96bytes align make DMA speedup
			pst_st_addr.disps_st_addr = align_addr;
			pst_st_addr.pst_sub_enable_addr = pst_st_addr.disps_st_addr + pst_queue_size;
			pst_st_addr.write_disps_cmd_num_addr = pst_st_addr.pst_sub_enable_addr + PST_ENABLE_ADDR_SIZE;
			pst_st_addr.read_disps_cmd_num_addr = pst_st_addr.write_disps_cmd_num_addr + PST_WRITE_CMD_ADDR_SIZE;
		}
		if (pst_st_addr.disps_st_addr == 0) {
			rtd_pr_smt_debug("[PST]malloc pst buffer fail .......................................\n");
			rtd_pr_smt_debug("[PST]disps_st_addr = %x\n", pst_st_addr.disps_st_addr);

			return;
		}else {
			memset((unsigned char *)pst_dispm_scpu_vir_addr.s1_mdisp_st_vir_addr, 0, sizeof(unsigned char) * pst_allocate_size);
		}
		pst_st_addr.pst_mode |= (_BIT(_SUB_MD));

		rtd_pr_smt_emerg("[%s]pst_dispm_scpu_vir_addr.s1_mdisp_st_vir_addr=%lx, pst_st_addr.dispm_st_addr=%x\n",__FUNCTION__, pst_dispm_scpu_vir_addr.s1_mdisp_st_vir_addr, pst_st_addr.disps_st_addr);
		rtd_pr_smt_emerg("[%s]pst_st_addr.pst_sub_enable_addr=%x,pst_st_addr.pst_mode=%d\n",__FUNCTION__, pst_st_addr.pst_sub_enable_addr, pst_st_addr.pst_mode );

		Scaler_pst_setbuffer(_SUB_MD);
	} else if (mode == _MAIN_VO_IMD) {
		/*allocate i domain pst buffer*/
		if (pst_st_addr.i_m_st_addr == 0) {
			idom_pst_allocate_size = dvr_size_alignment_cma(idom_pst_allocate_size);//size do 96-bytes align first
			idom_cache_addr =  (unsigned long)dvr_malloc_uncached_specific(idom_pst_allocate_size, GFP_DCU1_LIMIT, (void*) & (pst_imd_scpu_vir_addr.s0_vo_st_vir_addr));
			pst_main_idomain_addr_unalign = idom_cache_addr;//save unalign address for free memory.
			#ifdef CONFIG_ARM64
			query_addr = (unsigned long)dvr_to_phys((void*)idom_cache_addr);
			align_addr = dvr_memory_alignment((unsigned long)query_addr, idom_pst_allocate_size);
			if (align_addr == (unsigned long)NULL)
        			BUG();
			#else
			query_addr = (unsigned int)dvr_to_phys((void*)idom_cache_addr);
			align_addr = dvr_memory_alignment((unsigned long)query_addr, idom_pst_allocate_size);
			if (align_addr == (unsigned int)NULL)
        			BUG();
			#endif
			//do 96bytes align make DMA speedup
			pst_st_addr.i_m_st_addr = align_addr;
			pst_st_addr.idom_pst_enable_addr = pst_st_addr.i_m_st_addr + idom_pst_queue_size;
			pst_st_addr.write_idom_cmd_num_addr = pst_st_addr.idom_pst_enable_addr + PST_ENABLE_ADDR_SIZE;
			pst_st_addr.read_idom_cmd_num_addr = pst_st_addr.write_idom_cmd_num_addr + PST_WRITE_CMD_ADDR_SIZE;
			pst_st_addr.main_mdom_buf_delay_cnt_addr = pst_st_addr.read_idom_cmd_num_addr + PST_READ_CMD_ADDR_SIZE;
		}
		/*allocate d domain pst buffer*/
		if (pst_st_addr.dispm_st_addr == 0) {
			pst_allocate_size = dvr_size_alignment_cma(pst_allocate_size);//size do 96-bytes align first
			cache_addr =  (unsigned long)dvr_malloc_uncached_specific(pst_allocate_size, GFP_DCU1_LIMIT, (void*) & (pst_imd_scpu_vir_addr.s0_mdisp_st_vir_addr));
			pst_main_md_addr_unalign = cache_addr;//save unalign address for free memory.
			#ifdef CONFIG_ARM64
			query_addr = (unsigned long)dvr_to_phys((void*)cache_addr);
			align_addr = dvr_memory_alignment((unsigned long)query_addr, pst_allocate_size);
			if (align_addr == (unsigned long)NULL)
        			BUG();
			#else
			query_addr = (unsigned int)dvr_to_phys((void*)cache_addr);
			align_addr = dvr_memory_alignment((unsigned long)query_addr, pst_allocate_size);
			if (align_addr == (unsigned int)NULL)
        			BUG();
			#endif
			//do 96bytes align make DMA speedup
			pst_st_addr.dispm_st_addr = align_addr;
			pst_st_addr.pst_enable_addr = pst_st_addr.dispm_st_addr + pst_queue_size;
			pst_st_addr.write_dispm_cmd_num_addr = pst_st_addr.pst_enable_addr + PST_ENABLE_ADDR_SIZE;
			pst_st_addr.read_dispm_cmd_num_addr = pst_st_addr.write_dispm_cmd_num_addr + PST_WRITE_CMD_ADDR_SIZE;
		}
		//else{
			//d-domain pst buffer already allocated, sync the virtual address @Crixus 20180308
			//pst_imd_scpu_vir_addr.s0_mdisp_st_vir_addr = pst_dispm_scpu_vir_addr.s0_mdisp_st_vir_addr;
		//}
		if ((pst_st_addr.dispm_st_addr == 0) ||(pst_st_addr.i_m_st_addr== 0)) {
			rtd_pr_smt_emerg("[PST]malloc pst buffer fail .......................................\n");

			rtd_pr_smt_emerg("[PST]i_m_st_addr = %x\n", pst_st_addr.i_m_st_addr);
			rtd_pr_smt_emerg("[PST]dispm_st_addr = %x\n", pst_st_addr.dispm_st_addr);

			return;
		} else {
			memset((unsigned char *)pst_imd_scpu_vir_addr.s0_vo_st_vir_addr, 0, sizeof(unsigned char) * idom_pst_allocate_size);
			memset((unsigned char *)pst_imd_scpu_vir_addr.s0_mdisp_st_vir_addr, 0, sizeof(unsigned char) * pst_allocate_size);
		}
		pst_st_addr.dispm_cmd_buffer_size = pst_blk_size;
		pst_st_addr.idom_cmd_buffer_size = idom_pst_blk_size;
		pst_st_addr.pst_mode |= (_BIT(_MAIN_VO_IMD));

		rtd_pr_smt_emerg("[%s][pst_dispm_scpu_vir_addr.s0_vo_st_vir_addr=%lx,pst_st_addr.i_m_st_addr(physical addr)=%x\n",__FUNCTION__, pst_imd_scpu_vir_addr.s0_vo_st_vir_addr,pst_st_addr.i_m_st_addr);
		rtd_pr_smt_emerg("[%s][pst_dispm_scpu_vir_addr.s0_mdisp_st_vir_addr=%lx,pst_st_addr.dispm_st_addr(physical addr)=%x\n",__FUNCTION__, pst_imd_scpu_vir_addr.s0_mdisp_st_vir_addr,pst_st_addr.dispm_st_addr);
		rtd_pr_smt_emerg("[%s][pst_st_addr.idom_pst_enable_addr=%x,pst_st_addr.pst_enable_addr=%x\n",__FUNCTION__, pst_st_addr.idom_pst_enable_addr, pst_st_addr.pst_enable_addr);
		rtd_pr_smt_emerg("[%s][pst_st_addr.pst_mode=%d\n",__FUNCTION__, pst_st_addr.pst_mode);
		Scaler_pst_setbuffer(_MAIN_VO_IMD);
	} else if (mode == _SUB_VO_IMD) {
		/*allocate sub i domain pst buffer*/
		if (pst_st_addr.i_s_st_addr == 0) {
			idom_pst_allocate_size = dvr_size_alignment_cma(idom_pst_allocate_size);//size do 96-bytes align first
			idom_cache_addr =  (unsigned long)dvr_malloc_uncached_specific(idom_pst_allocate_size, GFP_DCU1_LIMIT, (void*) & (pst_imd_scpu_vir_addr.s1_vo_st_vir_addr));
			pst_sub_idomain_addr_unalign = idom_cache_addr;//save unalign address for free memory.
			#ifdef CONFIG_ARM64
			query_addr = (unsigned long)dvr_to_phys((void*)idom_cache_addr);
			align_addr = dvr_memory_alignment((unsigned long)query_addr, idom_pst_allocate_size);
			if (align_addr == (unsigned long)NULL)
        			BUG();
			#else
			query_addr = (unsigned int)dvr_to_phys((void*)idom_cache_addr);
			align_addr = dvr_memory_alignment((unsigned long)query_addr, idom_pst_allocate_size);
			if (align_addr == (unsigned int)NULL)
        			BUG();
			#endif
			//do 96bytes align make DMA speedup
			pst_st_addr.i_s_st_addr = align_addr;
			pst_st_addr.idos_pst_enable_addr = pst_st_addr.i_s_st_addr + idom_pst_queue_size;
			pst_st_addr.write_idos_cmd_num_addr = pst_st_addr.idos_pst_enable_addr + PST_ENABLE_ADDR_SIZE;
			pst_st_addr.read_idos_cmd_num_addr = pst_st_addr.write_idos_cmd_num_addr + PST_WRITE_CMD_ADDR_SIZE;
			pst_st_addr.sub_mdom_buf_delay_cnt_addr = pst_st_addr.read_idos_cmd_num_addr + PST_READ_CMD_ADDR_SIZE;
		}
		/*allocate d domain pst buffer*/
		if (pst_st_addr.disps_st_addr == 0) {
			pst_allocate_size = dvr_size_alignment_cma(pst_allocate_size);//size do 96-bytes align first
			cache_addr =  (unsigned long)dvr_malloc_uncached_specific(pst_allocate_size, GFP_DCU1_LIMIT, (void*) & (pst_imd_scpu_vir_addr.s1_mdisp_st_vir_addr));
			pst_sub_md_addr_unalign = cache_addr;//save unalign address for free memory.
			#ifdef CONFIG_ARM64
			query_addr = (unsigned long)dvr_to_phys((void*)cache_addr);
			align_addr = dvr_memory_alignment((unsigned long)query_addr, pst_allocate_size);
			if (align_addr == (unsigned long)NULL)
        			BUG();
			#else
			query_addr = (unsigned int)dvr_to_phys((void*)cache_addr);
			align_addr = dvr_memory_alignment((unsigned long)query_addr, pst_allocate_size);
			if (align_addr == (unsigned int)NULL)
        			BUG();
			#endif
			//do 96bytes align make DMA speedup
			pst_st_addr.disps_st_addr = align_addr;
			pst_st_addr.pst_sub_enable_addr = pst_st_addr.disps_st_addr + pst_queue_size;
			pst_st_addr.write_disps_cmd_num_addr = pst_st_addr.pst_sub_enable_addr + PST_ENABLE_ADDR_SIZE;
			pst_st_addr.read_disps_cmd_num_addr = pst_st_addr.write_disps_cmd_num_addr + PST_WRITE_CMD_ADDR_SIZE;
		}
		//else{
			//d-domain pst buffer already allocated, sync the virtual address @Crixus 20180308
			//pst_imd_scpu_vir_addr.s0_mdisp_st_vir_addr = pst_dispm_scpu_vir_addr.s0_mdisp_st_vir_addr;
		//}
		if ((pst_st_addr.disps_st_addr == 0) ||(pst_st_addr.i_s_st_addr== 0)) {
			rtd_pr_smt_emerg("[PST]malloc pst buffer fail .......................................\n");

			rtd_pr_smt_emerg("[PST]i_s_st_addr = %x\n", pst_st_addr.i_s_st_addr);
			rtd_pr_smt_emerg("[PST]disps_st_addr = %x\n", pst_st_addr.disps_st_addr);

			return;
		} else {
			memset((unsigned char *)pst_imd_scpu_vir_addr.s1_vo_st_vir_addr, 0, sizeof(unsigned char) * idom_pst_allocate_size);
			memset((unsigned char *)pst_imd_scpu_vir_addr.s1_mdisp_st_vir_addr, 0, sizeof(unsigned char) * pst_allocate_size);
		}
		pst_st_addr.dispm_cmd_buffer_size = pst_blk_size;
		pst_st_addr.idom_cmd_buffer_size = idom_pst_blk_size;
		pst_st_addr.pst_mode |= (_BIT(_SUB_VO_IMD));

		rtd_pr_smt_emerg("[%s][pst_disps_scpu_vir_addr.s1_vo_st_vir_addr=%lx,pst_st_addr.i_s_st_addr(physical addr)=%x\n",__FUNCTION__, pst_imd_scpu_vir_addr.s1_vo_st_vir_addr,pst_st_addr.i_s_st_addr);
		rtd_pr_smt_emerg("[%s][pst_disps_scpu_vir_addr.s1_mdisp_st_vir_addr=%lx,pst_st_addr.disps_st_addr(physical addr)=%x\n",__FUNCTION__, pst_imd_scpu_vir_addr.s1_mdisp_st_vir_addr,pst_st_addr.disps_st_addr);
		rtd_pr_smt_emerg("[%s][pst_st_addr.idos_pst_enable_addr=%x,pst_st_addr.pst_enable_addr=%x\n",__FUNCTION__, pst_st_addr.idos_pst_enable_addr, pst_st_addr.pst_enable_addr);
		rtd_pr_smt_emerg("[%s][pst_st_addr.pst_mode=%d\n",__FUNCTION__, pst_st_addr.pst_mode);

		Scaler_pst_setbuffer(_SUB_VO_IMD);
	} else {
		if (pst_st_addr.gdma_st_addr == 0) {
		//	pst_st_addr.gdma_st_addr =  (unsigned int)dvr_malloc_uncached_specific(pst_allocate_size, GFP_DCU1_FIRST, (void*) & (pst_dispm_scpu_vir_addr.s0_mdisp_st_vir_addr));
		}
		if (pst_st_addr.gdma_st_addr == 0) {
			rtd_pr_smt_debug("[PST]malloc pst buffer fail .......................................\n");
			rtd_pr_smt_debug("[PST]gdma_st_addr = %x\n", pst_st_addr.gdma_st_addr);

			return;
		}
		pst_st_addr.pst_mode = _DEFAULT_OFF;
	}

	//PST dma setting
	drvif_scaler_pst_dma_setting(PST_CMD_NUMBER, pst_blk_size, idom_pst_blk_size, mode);

#ifdef ENABLE_IMD_PST_SMOOTHTOGGLE
		if (mode == _MAIN_VO_IMD) {
			//Scaler_main_imd_pst_set_enable(_ENABLE);
			//drvif_scaler_pst_top_ctrl(_ENABLE, _MAIN_VO_IMD);
			//drvif_scaler_ddomain_DTG_line_compare_irq(1, 1, 1, 0x1EA);
			scaler_imd_pst_enable_interrupt();
		}
#else
		if (mode == _MAIN_VO_IMD) {
			Scaler_main_imd_pst_set_enable(_ENABLE);
			drvif_scaler_pst_top_ctrl(_ENABLE, _MAIN_VO_IMD);
			drvif_scaler_ddomain_DTG_line_compare_irq(1, 1, 1, 0x1EA);
		}
#endif

}
void drvif_scaler_pst_uninitial(_PST_MODE_T mode)
 {
 	//disable pst
	//drvif_scaler_pst_top_ctrl(_DISABLE, mode);
	drvif_scaler_pst_set_mode(_DEFAULT_OFF);
	drvif_scaler_ddomain_DTG_line_compare_irq(0, 0, 0, 0);
 //free memory
	if (mode == _MAIN_MD) {
		Scaler_main_md_pst_set_enable(_DISABLE);
		return;//FIX ME,below code will crash
#if 0
		if((pst_st_addr.dispm_st_addr != 0) && (pst_dispm_scpu_vir_addr.s0_mdisp_st_vir_addr != 0)){
 			dvr_free((void *)pst_main_md_addr_unalign);
 			pst_st_addr.dispm_st_addr = 0;
			pst_dispm_scpu_vir_addr.s0_mdisp_st_vir_addr = 0;
			pst_dispm_scpu_vir_addr.pst_enable_vir_addr = 0;
			pst_dispm_scpu_vir_addr.pst_dispm_write_cmd_vir_addr = 0;
			pst_dispm_scpu_vir_addr.pst_dispm_read_cmd_vir_addr = 0;
 		}
#endif
	} else if (mode == _MAIN_VO_IMD) {
		drvif_scaler_pst_top_ctrl(_DISABLE, _MAIN_VO_IMD);
		Scaler_main_imd_pst_set_enable(_DISABLE);
		return;//FIX ME,below code will crash
#if 0
		if((pst_st_addr.dispm_st_addr != 0) && (pst_imd_scpu_vir_addr.s0_mdisp_st_vir_addr != 0)){
 			dvr_free((void *)pst_main_md_addr_unalign);
 			pst_st_addr.dispm_st_addr = 0;
			pst_imd_scpu_vir_addr.s0_mdisp_st_vir_addr = 0;
			pst_imd_scpu_vir_addr.pst_enable_vir_addr = 0;
			pst_imd_scpu_vir_addr.pst_dispm_write_cmd_vir_addr = 0;
			pst_imd_scpu_vir_addr.pst_dispm_read_cmd_vir_addr = 0;
 		}

		if((pst_st_addr.i_m_st_addr != 0) && (pst_imd_scpu_vir_addr.s0_vo_st_vir_addr != 0)){
 			dvr_free((void *)pst_main_idomain_addr_unalign);
 			pst_st_addr.i_m_st_addr = 0;
			pst_imd_scpu_vir_addr.s0_vo_st_vir_addr = 0;
			pst_imd_scpu_vir_addr.idom_pst_enable_vir_addr = 0;
			pst_imd_scpu_vir_addr.pst_idom_write_cmd_vir_addr = 0;
			pst_imd_scpu_vir_addr.pst_idom_read_cmd_vir_addr = 0;
 		}
#endif
	}
 	pst_st_addr.pst_mode = _DEFAULT_OFF;
}
void drvif_scaler_pst_top_ctrl(bool enable, _PST_MODE_T mode)
{
	pst_i2rnd_top_ctrl_RBUS   top_ctrl;
	pst_i2rnd_top_ctrl_special_RBUS top_ctrl_special;
	//pst_i2rnd_dispm_repeat_ctrl_RBUS pst_i2rnd_dispm_repeat_ctrl_reg;
	top_ctrl.regValue = rtd_inl(PST_I2RND_TOP_CTRL_reg);
	
	top_ctrl_special.regValue = rtd_inl(PST_I2RND_TOP_CTRL_SPECIAL_reg);
	top_ctrl_special.psttodispd_sep_en = 1; //Separate main_disp and sub_disp,avoid main apply effect sub
	rtd_outl(PST_I2RND_TOP_CTRL_SPECIAL_reg, top_ctrl_special);
	//pst_i2rnd_dispm_repeat_ctrl_reg.regValue = rtd_inl(PST_I2RND_DISPM_REPEAT_CTRL_reg);
	if (enable) {
		if (mode == _MAIN_MD) {
			top_ctrl.dispm_pst_en = 1;
			top_ctrl.d_main_from_vgip_m = 0;//default 0:vsync start from vodma1 1:vsync start from main vgip
		} else if (mode == _MAIN_VO_IMD) {
			//top_ctrl.dispm_f_delay = 2;
			top_ctrl.i_main_from_vgip_m = 0;//default 0:vsync start from vodma1 1:vsync start from main vgip
			top_ctrl.d_main_from_vgip_m = 0;//default 0:vsync start from vodma1 1:vsync start from main vgip
			top_ctrl.i_main_pst_en = 1;
			top_ctrl.dispm_pst_en = 1;
			//pst_i2rnd_dispm_repeat_ctrl_reg.dispm_repeat_en = 1;
		}  else if (mode == _SUB_MD) {
			top_ctrl.d_sub_from_vgip_m = 0;//default 0:vsync start from vodma1 1:vsync start from main vgip
			top_ctrl.disps_pst_en = 1;
		}  else if (mode == _SUB_VO_IMD) {
			top_ctrl.i_sub_pst_en = 1;
			top_ctrl.disps_pst_en = 1;
		}  else {
			top_ctrl.gdma_pst_en = 1;
		}
	} else {
		top_ctrl.i_main_pst_en = 0;
		top_ctrl.i_sub_pst_en = 0;
		if ((mode == _MAIN_MD) || (mode == _MAIN_VO_IMD)) {
			top_ctrl.dispm_pst_en = 0;
		}
		else if (mode == _SUB_MD) {
			top_ctrl.disps_pst_en = 0;
		}
		top_ctrl.gdma_pst_en = 0;
		//pst_i2rnd_dispm_repeat_ctrl_reg.dispm_repeat_en = 0;
	}
	//IoReg_Write32(PST_I2RND_DISPM_REPEAT_CTRL_reg, pst_i2rnd_dispm_repeat_ctrl_reg.regValue);
	IoReg_Write32(PST_I2RND_TOP_CTRL_reg, top_ctrl.regValue);
}

void drvif_scaler_pst_dma_setting(unsigned int cmd_num, unsigned int blk_size, unsigned int idom_blk_size,_PST_MODE_T mode)
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
	pst_i2rnd_dma_num2_RBUS dma_num2;
	pst_i2rnd_dma_num3_RBUS	dma_num3;
	pst_i2rnd_dma_num4_RBUS	dma_num4;

	//cmd number
	dma_buf_num.regValue = rtd_inl(PST_I2RND_DMA_BUF_NUM_reg);
	dma_buf_num.dma_buffer_num = cmd_num;
	IoReg_Write32(PST_I2RND_DMA_BUF_NUM_reg, dma_buf_num.regValue);

	///////////////////////////////////////////
	// WATER/NUM/LENGTH
	///////////////////////////////////////////
	dma_ctrl.regValue = rtd_inl(PST_I2RND_DMA_CTRL_reg);
	dma_ctrl.dma_wtlvl = PST_DMA_WATERLEVEL;
	dma_ctrl.dma_len = PST_DMA_LENGTH;
	IoReg_Write32(PST_I2RND_DMA_CTRL_reg, dma_ctrl.regValue);

	//start address
	if (mode == _MAIN_MD) {
		pst_step_size.dispm_blk_size = blk_size;
		IoReg_Write32(PST_I2RND_DMA_ADDR6_reg, pst_st_addr.dispm_st_addr);
		IoReg_Write32(PST_I2RND_DMA_STEP6_reg, pst_step_size.dispm_blk_size);

		dma_num3.regValue = rtd_inl(PST_I2RND_DMA_NUM3_reg);
		dma_num3.dma_num_pst_dispm = pst_step_size.dispm_blk_size/8/dma_ctrl.dma_len;
    	IoReg_Write32(PST_I2RND_DMA_NUM3_reg, dma_num3.regValue);

	} else if (mode == _MAIN_VO_IMD) {
		pst_step_size.i_m_blk_size = idom_blk_size;
		pst_step_size.dispm_blk_size = blk_size;
		{
			IoReg_Write32(PST_I2RND_DMA_ADDR4_reg, pst_st_addr.i_m_st_addr);
			IoReg_Write32(PST_I2RND_DMA_STEP4_reg, pst_step_size.i_m_blk_size);
		//dma number
			dma_num2.regValue = rtd_inl(PST_I2RND_DMA_NUM2_reg);
			dma_num2.dma_num_pst_i_main = pst_step_size.i_m_blk_size/8/dma_ctrl.dma_len;
			IoReg_Write32(PST_I2RND_DMA_NUM2_reg, dma_num2.regValue);
		}
		IoReg_Write32(PST_I2RND_DMA_ADDR6_reg, pst_st_addr.dispm_st_addr);
		IoReg_Write32(PST_I2RND_DMA_STEP6_reg, pst_step_size.dispm_blk_size);

		dma_num3.regValue = rtd_inl(PST_I2RND_DMA_NUM3_reg);
		dma_num3.dma_num_pst_dispm = pst_step_size.dispm_blk_size/8/dma_ctrl.dma_len;
    	IoReg_Write32(PST_I2RND_DMA_NUM3_reg, dma_num3.regValue);

	} else if (mode == _SUB_MD) {
		pst_step_size.disps_blk_size = blk_size;

		IoReg_Write32(PST_I2RND_DMA_ADDR7_reg, pst_st_addr.disps_st_addr);
		IoReg_Write32(PST_I2RND_DMA_STEP7_reg, pst_step_size.disps_blk_size);

		dma_num3.regValue = rtd_inl(PST_I2RND_DMA_NUM3_reg);
		dma_num3.dma_num_pst_disps = pst_step_size.disps_blk_size/8/dma_ctrl.dma_len;
		IoReg_Write32(PST_I2RND_DMA_NUM3_reg, dma_num3.regValue);

	} else if (mode == _SUB_VO_IMD) {
		pst_step_size.i_s_blk_size = idom_blk_size;
		pst_step_size.disps_blk_size = blk_size;

		IoReg_Write32(PST_I2RND_DMA_ADDR5_reg, pst_st_addr.i_s_st_addr);
		IoReg_Write32(PST_I2RND_DMA_STEP5_reg, pst_step_size.i_s_blk_size);
		IoReg_Write32(PST_I2RND_DMA_ADDR7_reg, pst_st_addr.disps_st_addr);
		IoReg_Write32(PST_I2RND_DMA_STEP7_reg, pst_step_size.disps_blk_size);

		  //dma number
		dma_num2.regValue = rtd_inl(PST_I2RND_DMA_NUM2_reg);
		dma_num2.dma_num_pst_i_sub = pst_step_size.i_s_blk_size/8/dma_ctrl.dma_len;
		IoReg_Write32(PST_I2RND_DMA_NUM2_reg, dma_num2.regValue);

		dma_num3.regValue = rtd_inl(PST_I2RND_DMA_NUM3_reg);
		dma_num3.dma_num_pst_disps = pst_step_size.disps_blk_size/8/dma_ctrl.dma_len;
		IoReg_Write32(PST_I2RND_DMA_NUM3_reg, dma_num3.regValue);

	} else {
		pst_step_size.gdma_blk_size = blk_size;
		IoReg_Write32(PST_I2RND_DMA_ADDR8_reg, pst_st_addr.gdma_st_addr);
		IoReg_Write32(PST_I2RND_DMA_STEP8_reg, pst_step_size.gdma_blk_size);

		dma_num4.regValue = rtd_inl(PST_I2RND_DMA_NUM4_reg);
		dma_num4.dma_num_pst_gdma = pst_step_size.gdma_blk_size/8/dma_ctrl.dma_len;
		IoReg_Write32(PST_I2RND_DMA_NUM4_reg, dma_num4.regValue);
	}
#if 0
    ////////////////////////////////////////
    //  update current status
    ////////////////////////////////////////
    pst_st_addr.i_m_cur_addr = pst_st_addr.i_m_st_addr;
    pst_st_addr.i_s_cur_addr = pst_st_addr.i_s_st_addr;
//    pst_st_addr.dispm_cur_addr = pst_st_addr.dispm_st_addr;
    pst_st_addr.disps_cur_addr = pst_st_addr.disps_st_addr;
    pst_st_addr.gdma_cur_addr = pst_st_addr.gdma_st_addr;

    pst_cmd_buf.i_m_cur_cmd = 0;
    pst_cmd_buf.i_s_cur_cmd = 0;
    pst_cmd_buf.dispm_cur_cmd = 0;
    pst_cmd_buf.disps_cur_cmd = 0;
    pst_cmd_buf.gdma_cur_cmd = 0;
#endif
}

/*
1.send RPC the cmd queue buffer info from SCPU to VCPU
2.record I2run S0,S1 Queue each IP start register setting start point in SCPU
*/
unsigned char Scaler_pst_setbuffer(_PST_MODE_T mode)
{


	unsigned int *pulTemp = NULL;
	unsigned int i=0, ulItemCount = 0;
	PST_START_ADDR pst_addr_info={0};

	if (mode == _MAIN_MD) {
		pst_dispm_scpu_vir_addr.s0_uzu_st_vir_addr			= pst_dispm_scpu_vir_addr.s0_mdisp_st_vir_addr	+ MAIN_MDISP_REGISTER_NUM * PST_REGISTER_DDR_SIZE;
		pst_dispm_scpu_vir_addr.s0_dtg_st_vir_addr			= pst_dispm_scpu_vir_addr.s0_uzu_st_vir_addr		+ MAIN_UZU_REGISTER_NUM * PST_REGISTER_DDR_SIZE;
		pst_dispm_scpu_vir_addr.s0_dtg_iv2dv_st_vir_addr	= pst_dispm_scpu_vir_addr.s0_dtg_st_vir_addr	+ MAIN_DTG_REGISTER_NUM * PST_REGISTER_DDR_SIZE;
		pst_dispm_scpu_vir_addr.s0_orbit_st_vir_addr		= pst_dispm_scpu_vir_addr.s0_dtg_iv2dv_st_vir_addr	+ MAIN_DTG_IV2DV_REGISTER_NUM * PST_REGISTER_DDR_SIZE;
		pst_dispm_scpu_vir_addr.s0_nnsr_st_vir_addr			= pst_dispm_scpu_vir_addr.s0_orbit_st_vir_addr + MAIN_ORBIT_REGISTER_NUM * PST_REGISTER_DDR_SIZE;
		pst_dispm_scpu_vir_addr.s0_2stepuzu_st_vir_addr 	= pst_dispm_scpu_vir_addr.s0_nnsr_st_vir_addr		+ MAIN_NNSR_REGISTER_NUM * PST_REGISTER_DDR_SIZE;
		pst_dispm_scpu_vir_addr.s0_LC_st_vir_addr           = pst_dispm_scpu_vir_addr.s0_2stepuzu_st_vir_addr + MAIN_2STEPUZU_REGISTER_NUM * PST_REGISTER_DDR_SIZE;
		pst_dispm_scpu_vir_addr.pst_enable_vir_addr			= pst_dispm_scpu_vir_addr.s0_mdisp_st_vir_addr 	+ pst_queue_size;
		pst_dispm_scpu_vir_addr.pst_dispm_write_cmd_vir_addr			= pst_dispm_scpu_vir_addr.pst_enable_vir_addr 	+ PST_ENABLE_ADDR_SIZE;
		pst_dispm_scpu_vir_addr.pst_dispm_read_cmd_vir_addr			= pst_dispm_scpu_vir_addr.pst_dispm_write_cmd_vir_addr 	+ PST_WRITE_CMD_ADDR_SIZE;

		rtd_pr_smt_emerg("[pst_dispm_scpu_vir_addr]pst_enable_vir_addr=%lx\n",pst_dispm_scpu_vir_addr.pst_enable_vir_addr);
	}
	else if (mode == _SUB_MD) {
		pst_dispm_scpu_vir_addr.s1_uzu_st_vir_addr			= pst_dispm_scpu_vir_addr.s1_mdisp_st_vir_addr		+ MAIN_MDISP_REGISTER_NUM * PST_REGISTER_DDR_SIZE;
		pst_dispm_scpu_vir_addr.s1_dtg_st_vir_addr			= pst_dispm_scpu_vir_addr.s1_uzu_st_vir_addr		+ MAIN_UZU_REGISTER_NUM * PST_REGISTER_DDR_SIZE;
		pst_dispm_scpu_vir_addr.s1_orbit_st_vir_addr		= pst_dispm_scpu_vir_addr.s1_dtg_st_vir_addr		+ MAIN_DTG_REGISTER_NUM * PST_REGISTER_DDR_SIZE;
		pst_dispm_scpu_vir_addr.pst_disps_enable_vir_addr			= pst_dispm_scpu_vir_addr.s1_mdisp_st_vir_addr 	+ pst_queue_size;
		pst_dispm_scpu_vir_addr.pst_disps_write_cmd_vir_addr	= pst_dispm_scpu_vir_addr.pst_disps_enable_vir_addr 	+ PST_ENABLE_ADDR_SIZE;
		pst_dispm_scpu_vir_addr.pst_disps_read_cmd_vir_addr	= pst_dispm_scpu_vir_addr.pst_disps_write_cmd_vir_addr 	+ PST_WRITE_CMD_ADDR_SIZE;

		rtd_pr_smt_emerg("[sub pst]pst_md_end_addr=%x\n",pst_st_addr.disps_st_addr + pst_queue_size);
		rtd_pr_smt_emerg("[sub pst][pst_dispm_scpu_vir_addr]s0_uzu_st_vir_addr=%lx,s0_dtg_st_vir_addr=%lx,pst_enable_vir_addr=%lx",pst_dispm_scpu_vir_addr.s1_uzu_st_vir_addr,pst_dispm_scpu_vir_addr.s1_dtg_st_vir_addr,pst_dispm_scpu_vir_addr.pst_disps_enable_vir_addr);
	}
	else if (mode == _MAIN_VO_IMD){
		pst_imd_scpu_vir_addr.s0_vgip_st_vir_addr = pst_imd_scpu_vir_addr.s0_vo_st_vir_addr + VODMA_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_color_mb_peaking_st_vir_addr = pst_imd_scpu_vir_addr.s0_vgip_st_vir_addr + VGIP_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_dexcxl_st_vir_addr = pst_imd_scpu_vir_addr.s0_color_mb_peaking_st_vir_addr + COLOR_MB_PEAKING_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_dfilter_st_vir_addr = pst_imd_scpu_vir_addr.s0_dexcxl_st_vir_addr + DE_XCXL_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_di0_st_vir_addr = pst_imd_scpu_vir_addr.s0_dfilter_st_vir_addr + DFILTER_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_di1_st_vir_addr = pst_imd_scpu_vir_addr.s0_di0_st_vir_addr + DI_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_di2_st_vir_addr = pst_imd_scpu_vir_addr.s0_di1_st_vir_addr + DI_REGISTER_NUM_1 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_di3_st_vir_addr = pst_imd_scpu_vir_addr.s0_di2_st_vir_addr + DI_REGISTER_NUM_2 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_di4_st_vir_addr = pst_imd_scpu_vir_addr.s0_di3_st_vir_addr + DI_REGISTER_NUM_3 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_histogram_st_vir_addr = pst_imd_scpu_vir_addr.s0_di4_st_vir_addr + DI_REGISTER_NUM_4 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_hsddither_st_vir_addr = pst_imd_scpu_vir_addr.s0_histogram_st_vir_addr + HISTOGRAM_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_idcti_st_vir_addr = pst_imd_scpu_vir_addr.s0_hsddither_st_vir_addr + HSD_DITHER_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_iedgesmooth_st_vir_addr = pst_imd_scpu_vir_addr.s0_idcti_st_vir_addr + IDCTI_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_ipq_decontour_st_vir_addr = pst_imd_scpu_vir_addr.s0_iedgesmooth_st_vir_addr + IEDGE_SMOOTH_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_main_dither_st_vir_addr = pst_imd_scpu_vir_addr.s0_ipq_decontour_st_vir_addr + IPQ_DECONTOUR_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_mpegnr_st_vir_addr = pst_imd_scpu_vir_addr.s0_main_dither_st_vir_addr + MAIN_DITHER_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_nr0_st_vir_addr = pst_imd_scpu_vir_addr.s0_mpegnr_st_vir_addr + MPEGNR_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_nr1_st_vir_addr = pst_imd_scpu_vir_addr.s0_nr0_st_vir_addr + NR_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_peaking_st_vir_addr = pst_imd_scpu_vir_addr.s0_nr1_st_vir_addr + NR_REGISTER_NUM_1 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_profile0_st_vir_addr = pst_imd_scpu_vir_addr.s0_peaking_st_vir_addr + PEAKING_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_profile1_st_vir_addr = pst_imd_scpu_vir_addr.s0_profile0_st_vir_addr + PROFILE_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_rgb2yuv0_st_vir_addr = pst_imd_scpu_vir_addr.s0_profile1_st_vir_addr + PROFILE_REGISTER_NUM_1 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_rgb2yuv1_st_vir_addr = pst_imd_scpu_vir_addr.s0_rgb2yuv0_st_vir_addr + RGB2YUV_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_ringfilter_st_vir_addr = pst_imd_scpu_vir_addr.s0_rgb2yuv1_st_vir_addr + RGB2YUV_REGISTER_NUM_1 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_uzd0_st_vir_addr = pst_imd_scpu_vir_addr.s0_ringfilter_st_vir_addr + RINGFILTER_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_uzd1_st_vir_addr = pst_imd_scpu_vir_addr.s0_uzd0_st_vir_addr + SCALEDOWN_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_mcap0_st_vir_addr = pst_imd_scpu_vir_addr.s0_uzd1_st_vir_addr + SCALEDOWN_REGISTER_NUM_1 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_mcap1_st_vir_addr = pst_imd_scpu_vir_addr.s0_mcap0_st_vir_addr + MDOMAIN_CAP_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_mcap2_st_vir_addr = pst_imd_scpu_vir_addr.s0_mcap1_st_vir_addr + MDOMAIN_CAP_REGISTER_NUM_1 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.idom_pst_enable_vir_addr = pst_imd_scpu_vir_addr.s0_vo_st_vir_addr + idom_pst_queue_size;
		pst_imd_scpu_vir_addr.pst_idom_write_cmd_vir_addr = pst_imd_scpu_vir_addr.idom_pst_enable_vir_addr + PST_ENABLE_ADDR_SIZE;
		pst_imd_scpu_vir_addr.pst_idom_read_cmd_vir_addr = pst_imd_scpu_vir_addr.pst_idom_write_cmd_vir_addr + PST_WRITE_CMD_ADDR_SIZE;

		//pst_imd_scpu_vir_addr.s0_mdisp_st_vir_addr = pst_imd_scpu_vir_addr.s0_mcap2_st_vir_addr + MDOMAIN_CAP_REGISTER_NUM_2 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_uzu_st_vir_addr = pst_imd_scpu_vir_addr.s0_mdisp_st_vir_addr + MAIN_MDISP_REGISTER_NUM * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_dtg_st_vir_addr = pst_imd_scpu_vir_addr.s0_uzu_st_vir_addr + MAIN_UZU_REGISTER_NUM * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_dtg_iv2dv_st_vir_addr	= pst_imd_scpu_vir_addr.s0_dtg_st_vir_addr	+ MAIN_DTG_REGISTER_NUM * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_orbit_st_vir_addr		= pst_imd_scpu_vir_addr.s0_dtg_iv2dv_st_vir_addr	+ MAIN_DTG_IV2DV_REGISTER_NUM * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_nnsr_st_vir_addr		= pst_imd_scpu_vir_addr.s0_orbit_st_vir_addr	+ MAIN_ORBIT_REGISTER_NUM * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_2stepuzu_st_vir_addr = pst_imd_scpu_vir_addr.s0_nnsr_st_vir_addr		+ MAIN_NNSR_REGISTER_NUM * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s0_LC_st_vir_addr           = pst_imd_scpu_vir_addr.s0_2stepuzu_st_vir_addr + MAIN_2STEPUZU_REGISTER_NUM * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.pst_enable_vir_addr			= pst_imd_scpu_vir_addr.s0_mdisp_st_vir_addr + pst_queue_size;
		pst_imd_scpu_vir_addr.pst_dispm_write_cmd_vir_addr			= pst_imd_scpu_vir_addr.pst_enable_vir_addr 	+ PST_ENABLE_ADDR_SIZE;
		pst_imd_scpu_vir_addr.pst_dispm_read_cmd_vir_addr			= pst_imd_scpu_vir_addr.pst_dispm_write_cmd_vir_addr 	+ PST_WRITE_CMD_ADDR_SIZE;
		pst_imd_scpu_vir_addr.main_mdom_buf_delay_cnt_addr				= pst_imd_scpu_vir_addr.pst_dispm_read_cmd_vir_addr		+ PST_READ_CMD_ADDR_SIZE;

		rtd_pr_smt_emerg("[pst_imd_scpu_vir_addr]pst_enable_vir_addr=%lx\n",pst_imd_scpu_vir_addr.pst_enable_vir_addr);

		rtd_pr_smt_emerg("[pst_imd_scpu_vir_addr]idom_pst_enable_vir_addr=%lx\n",pst_imd_scpu_vir_addr.idom_pst_enable_vir_addr);


	}
	else if (mode == _SUB_VO_IMD){
		pst_imd_scpu_vir_addr.s1_vgip_st_vir_addr = pst_imd_scpu_vir_addr.s1_vo_st_vir_addr + VODMA_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_color_mb_peaking_st_vir_addr = pst_imd_scpu_vir_addr.s1_vgip_st_vir_addr + VGIP_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_dexcxl_st_vir_addr = pst_imd_scpu_vir_addr.s1_color_mb_peaking_st_vir_addr + COLOR_MB_PEAKING_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_dfilter_st_vir_addr = pst_imd_scpu_vir_addr.s1_dexcxl_st_vir_addr + DE_XCXL_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_di0_st_vir_addr = pst_imd_scpu_vir_addr.s1_dfilter_st_vir_addr + DFILTER_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_di1_st_vir_addr = pst_imd_scpu_vir_addr.s1_di0_st_vir_addr + DI_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_di2_st_vir_addr = pst_imd_scpu_vir_addr.s1_di1_st_vir_addr + DI_REGISTER_NUM_1 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_di3_st_vir_addr = pst_imd_scpu_vir_addr.s1_di2_st_vir_addr + DI_REGISTER_NUM_2 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_di4_st_vir_addr = pst_imd_scpu_vir_addr.s1_di3_st_vir_addr + DI_REGISTER_NUM_3 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_histogram_st_vir_addr = pst_imd_scpu_vir_addr.s1_di4_st_vir_addr + DI_REGISTER_NUM_4 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_hsddither_st_vir_addr = pst_imd_scpu_vir_addr.s1_histogram_st_vir_addr + HISTOGRAM_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_idcti_st_vir_addr = pst_imd_scpu_vir_addr.s1_hsddither_st_vir_addr + HSD_DITHER_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_iedgesmooth_st_vir_addr = pst_imd_scpu_vir_addr.s1_idcti_st_vir_addr + IDCTI_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_ipq_decontour_st_vir_addr = pst_imd_scpu_vir_addr.s1_iedgesmooth_st_vir_addr + IEDGE_SMOOTH_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_main_dither_st_vir_addr = pst_imd_scpu_vir_addr.s1_ipq_decontour_st_vir_addr + IPQ_DECONTOUR_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_mpegnr_st_vir_addr = pst_imd_scpu_vir_addr.s1_main_dither_st_vir_addr + MAIN_DITHER_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_nr0_st_vir_addr = pst_imd_scpu_vir_addr.s1_mpegnr_st_vir_addr + MPEGNR_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_nr1_st_vir_addr = pst_imd_scpu_vir_addr.s1_nr0_st_vir_addr + NR_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_peaking_st_vir_addr = pst_imd_scpu_vir_addr.s1_nr1_st_vir_addr + NR_REGISTER_NUM_1 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_profile0_st_vir_addr = pst_imd_scpu_vir_addr.s1_peaking_st_vir_addr + PEAKING_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_profile1_st_vir_addr = pst_imd_scpu_vir_addr.s1_profile0_st_vir_addr + PROFILE_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_rgb2yuv0_st_vir_addr = pst_imd_scpu_vir_addr.s1_profile1_st_vir_addr + PROFILE_REGISTER_NUM_1 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_rgb2yuv1_st_vir_addr = pst_imd_scpu_vir_addr.s1_rgb2yuv0_st_vir_addr + RGB2YUV_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_ringfilter_st_vir_addr = pst_imd_scpu_vir_addr.s1_rgb2yuv1_st_vir_addr + RGB2YUV_REGISTER_NUM_1 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_uzd0_st_vir_addr = pst_imd_scpu_vir_addr.s1_ringfilter_st_vir_addr + RINGFILTER_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_uzd1_st_vir_addr = pst_imd_scpu_vir_addr.s1_uzd0_st_vir_addr + SCALEDOWN_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_mcap0_st_vir_addr = pst_imd_scpu_vir_addr.s1_uzd1_st_vir_addr + SCALEDOWN_REGISTER_NUM_1 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_mcap1_st_vir_addr = pst_imd_scpu_vir_addr.s1_mcap0_st_vir_addr + MDOMAIN_CAP_REGISTER_NUM_0 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_mcap2_st_vir_addr = pst_imd_scpu_vir_addr.s1_mcap1_st_vir_addr + MDOMAIN_CAP_REGISTER_NUM_1 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.idos_pst_enable_vir_addr = pst_imd_scpu_vir_addr.s1_vo_st_vir_addr + idom_pst_queue_size;
		pst_imd_scpu_vir_addr.pst_idos_write_cmd_vir_addr = pst_imd_scpu_vir_addr.idos_pst_enable_vir_addr + PST_ENABLE_ADDR_SIZE;
		pst_imd_scpu_vir_addr.pst_idos_read_cmd_vir_addr = pst_imd_scpu_vir_addr.pst_idos_write_cmd_vir_addr + PST_WRITE_CMD_ADDR_SIZE;
		pst_imd_scpu_vir_addr.sub_mdom_buf_delay_cnt_addr				= pst_imd_scpu_vir_addr.pst_idos_read_cmd_vir_addr		+ PST_READ_CMD_ADDR_SIZE;

		//pst_imd_scpu_vir_addr.s0_mdisp_st_vir_addr = pst_imd_scpu_vir_addr.s0_mcap2_st_vir_addr + MDOMAIN_CAP_REGISTER_NUM_2 * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_uzu_st_vir_addr = pst_imd_scpu_vir_addr.s1_mdisp_st_vir_addr + MAIN_MDISP_REGISTER_NUM * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_dtg_st_vir_addr = pst_imd_scpu_vir_addr.s1_uzu_st_vir_addr + MAIN_UZU_REGISTER_NUM * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.s1_orbit_st_vir_addr = pst_imd_scpu_vir_addr.s1_dtg_st_vir_addr + MAIN_DTG_REGISTER_NUM * PST_REGISTER_DDR_SIZE;
		pst_imd_scpu_vir_addr.pst_sub_enable_vir_addr			= pst_imd_scpu_vir_addr.s1_mdisp_st_vir_addr + pst_queue_size;
		pst_imd_scpu_vir_addr.pst_disps_write_cmd_vir_addr			= pst_imd_scpu_vir_addr.pst_sub_enable_vir_addr + PST_ENABLE_ADDR_SIZE;
		pst_imd_scpu_vir_addr.pst_disps_read_cmd_vir_addr			= pst_imd_scpu_vir_addr.pst_disps_write_cmd_vir_addr 	+ PST_WRITE_CMD_ADDR_SIZE;

		rtd_pr_smt_emerg("[pst_imd_scpu_vir_addr]pst_sub_enable_vir_addr=%lx\n",pst_imd_scpu_vir_addr.pst_sub_enable_vir_addr);

		rtd_pr_smt_emerg("[pst_imd_scpu_vir_addr]idom_pst_enable_vir_addr=%lx\n",pst_imd_scpu_vir_addr.idos_pst_enable_vir_addr);
	}

	if (mode == _MAIN_MD) {
		if (pst_st_addr.dispm_st_addr ==0) {
			rtd_pr_smt_debug("pst  _MAIN_MD not alloc memroy\n");
			return 0;
		}
		pst_addr_info.dispm_st_addr	= pst_st_addr.dispm_st_addr;
		pst_addr_info.write_dispm_cmd_num_addr = pst_st_addr.write_dispm_cmd_num_addr;
		pst_addr_info.read_dispm_cmd_num_addr = pst_st_addr.read_dispm_cmd_num_addr;
		pst_addr_info.dispm_cur_addr = pst_st_addr.dispm_st_addr;
		pst_addr_info.pst_enable_addr = pst_st_addr.pst_enable_addr;
		pst_addr_info.dispm_cmd_buffer_size = pst_st_addr.dispm_cmd_buffer_size;
	}else if(mode == _SUB_MD) {
		if (pst_st_addr.disps_st_addr ==0) {
			rtd_pr_smt_debug("pst  _MAIN_MD not alloc memroy\n");
			return 0;
		}
		pst_addr_info.disps_st_addr				= pst_st_addr.disps_st_addr;
		pst_addr_info.write_disps_cmd_num_addr	= pst_st_addr.write_disps_cmd_num_addr;
		pst_addr_info.read_disps_cmd_num_addr	=  pst_st_addr.read_disps_cmd_num_addr;
		pst_addr_info.disps_cur_addr			= pst_st_addr.disps_st_addr;
		pst_addr_info.pst_sub_enable_addr = pst_st_addr.pst_sub_enable_addr;
	} else if (mode == _MAIN_VO_IMD) {
		if (pst_st_addr.i_m_st_addr ==0) {
			rtd_pr_smt_debug("pst  id not alloc memroy!!\n");
			return 0;
		}

		if (pst_st_addr.dispm_st_addr ==0) {
			rtd_pr_smt_debug("pst  md not alloc memroy!!\n");
			return 0;
		}

		pst_addr_info.i_m_st_addr = pst_st_addr.i_m_st_addr;
		pst_addr_info.write_idom_cmd_num_addr = pst_st_addr.write_idom_cmd_num_addr;
		pst_addr_info.read_idom_cmd_num_addr = pst_st_addr.read_idom_cmd_num_addr;
		pst_addr_info.i_m_cur_addr = pst_st_addr.i_m_cur_addr;
		pst_addr_info.idom_pst_enable_addr = pst_st_addr.idom_pst_enable_addr;
		pst_addr_info.idom_cmd_buffer_size = pst_st_addr.idom_cmd_buffer_size;
		pst_addr_info.main_mdom_buf_delay_cnt_addr = pst_st_addr.main_mdom_buf_delay_cnt_addr;

		pst_addr_info.dispm_st_addr = pst_st_addr.dispm_st_addr;
		pst_addr_info.write_dispm_cmd_num_addr = pst_st_addr.write_dispm_cmd_num_addr;
		pst_addr_info.read_dispm_cmd_num_addr = pst_st_addr.read_dispm_cmd_num_addr;
		pst_addr_info.dispm_cur_addr = pst_st_addr.dispm_cur_addr;
		pst_addr_info.pst_enable_addr = pst_st_addr.pst_enable_addr;
		pst_addr_info.dispm_cmd_buffer_size = pst_st_addr.dispm_cmd_buffer_size;
	}else if (mode == _SUB_VO_IMD) {
		if (pst_st_addr.i_s_st_addr ==0) {
			rtd_pr_smt_debug("pst sub id not alloc memroy!!\n");
			return 0;
		}

		if (pst_st_addr.disps_st_addr ==0) {
			rtd_pr_smt_debug("pst sub md not alloc memroy!!\n");
			return 0;
		}

		pst_addr_info.i_s_st_addr = pst_st_addr.i_s_st_addr;
		pst_addr_info.write_idos_cmd_num_addr = pst_st_addr.write_idos_cmd_num_addr;
		pst_addr_info.read_idos_cmd_num_addr = pst_st_addr.read_idos_cmd_num_addr;
		pst_addr_info.i_s_cur_addr = pst_st_addr.i_s_cur_addr;
		pst_addr_info.idos_pst_enable_addr = pst_st_addr.idos_pst_enable_addr;
		pst_addr_info.idom_cmd_buffer_size = pst_st_addr.idom_cmd_buffer_size;
		pst_addr_info.sub_mdom_buf_delay_cnt_addr = pst_st_addr.sub_mdom_buf_delay_cnt_addr;

		pst_addr_info.disps_st_addr = pst_st_addr.disps_st_addr;
		pst_addr_info.write_disps_cmd_num_addr = pst_st_addr.write_disps_cmd_num_addr;
		pst_addr_info.read_disps_cmd_num_addr = pst_st_addr.read_disps_cmd_num_addr;
		pst_addr_info.disps_cur_addr = pst_st_addr.disps_cur_addr;
		pst_addr_info.pst_sub_enable_addr = pst_st_addr.pst_sub_enable_addr;
		pst_addr_info.dispm_cmd_buffer_size = pst_st_addr.dispm_cmd_buffer_size;
	}

	pst_addr_info.pst_mode = mode;


	pulTemp = (unsigned int*)Scaler_GetShareMemVirAddr(SCALERIOC_PST_SET_BUFFER);
	ulItemCount = sizeof(PST_START_ADDR) /  sizeof(unsigned int);
	if (pulTemp) {
		memcpy(pulTemp, &pst_addr_info, sizeof(PST_START_ADDR));
		for (i = 0; i < ulItemCount; i++)
				pulTemp[i] = htonl(pulTemp[i]);

		if (0 != Scaler_SendRPC(SCALERIOC_PST_SET_BUFFER,0,0))
		{
			rtd_pr_smt_emerg("set buffer address for pst fail !!!\n");
			return 0;

		}
	}
	return 0;
}

void Scaler_pst_buffer_initial(_PST_MODE_T mode)
{
	if(mode == _MAIN_MD){
		memset((unsigned char *)pst_dispm_scpu_vir_addr.s0_mdisp_st_vir_addr, 0 , sizeof(unsigned char) * pst_st_addr.dispm_cmd_buffer_size);
		}
	else if(mode == _SUB_MD){
		memset((unsigned char *)pst_dispm_scpu_vir_addr.s1_mdisp_st_vir_addr, 0 , sizeof(unsigned char) * pst_st_addr.dispm_cmd_buffer_size);
	}
}
#endif

#ifdef NEW_INPUT_OUTPUT_BUFFER_FLOW_CONFIG
void set_pst_active_state(unsigned char display, unsigned short state)
{//set pst active state or not

	INPUT_OUTPUT_RINGBUFFER_INFO_T *p_ring_buffer_addr = NULL;
	if(display == SLR_MAIN_DISPLAY)
	{
		p_ring_buffer_addr = (INPUT_OUTPUT_RINGBUFFER_INFO_T *)Scaler_GetShareMemVirAddr(SCALERIOC_MAIN_INPUT_OUTPUT_BUFFER_INFO);
		if(p_ring_buffer_addr)
			p_ring_buffer_addr->pst_active_state = Scaler_ChangeUINT16Endian(state);
	}
#ifdef CONFIG_DUAL_CHANNEL
	else if(display == SLR_SUB_DISPLAY)
	{
		p_ring_buffer_addr = (INPUT_OUTPUT_RINGBUFFER_INFO_T *)Scaler_GetShareMemVirAddr(SCALERIOC_SUB_INPUT_OUTPUT_BUFFER_INFO);
		if(p_ring_buffer_addr)
			p_ring_buffer_addr->pst_active_state = Scaler_ChangeUINT16Endian(state);
	}
#endif
}

void reset_pst_sharemem_buffer(unsigned char display)
{//set pst sharememocry buffer data for chnage source . Let write index = ready index
	INPUT_OUTPUT_RINGBUFFER_INFO_T *p_ring_buffer_addr = NULL;
	if(display == SLR_MAIN_DISPLAY)
	{
		p_ring_buffer_addr = (INPUT_OUTPUT_RINGBUFFER_INFO_T *)Scaler_GetShareMemVirAddr(SCALERIOC_MAIN_INPUT_OUTPUT_BUFFER_INFO);
		if(p_ring_buffer_addr)
			p_ring_buffer_addr->write_idx = p_ring_buffer_addr->read_idx;
	}
#ifdef CONFIG_DUAL_CHANNEL
	else if(display == SLR_SUB_DISPLAY)
	{
		p_ring_buffer_addr = (INPUT_OUTPUT_RINGBUFFER_INFO_T *)Scaler_GetShareMemVirAddr(SCALERIOC_SUB_INPUT_OUTPUT_BUFFER_INFO);
		if(p_ring_buffer_addr)
			p_ring_buffer_addr->write_idx = p_ring_buffer_addr->read_idx;
	}
#endif
}
#endif

unsigned char check_pst_active_state(unsigned char display)
{//Return TRUE: active state can run pst. Return False: not active state can not run pst
#ifdef NEW_INPUT_OUTPUT_BUFFER_FLOW_CONFIG
	INPUT_OUTPUT_RINGBUFFER_INFO_T *p_ring_buffer_addr = NULL;
	if(display == SLR_MAIN_DISPLAY)
	{
		p_ring_buffer_addr = (INPUT_OUTPUT_RINGBUFFER_INFO_T *)Scaler_GetShareMemVirAddr(SCALERIOC_MAIN_INPUT_OUTPUT_BUFFER_INFO);
		if(p_ring_buffer_addr && p_ring_buffer_addr->pst_active_state)
			return TRUE;
		else
			return FALSE;
	}
#ifdef CONFIG_DUAL_CHANNEL
	else if(display == SLR_SUB_DISPLAY)
	{
		p_ring_buffer_addr = (INPUT_OUTPUT_RINGBUFFER_INFO_T *)Scaler_GetShareMemVirAddr(SCALERIOC_SUB_INPUT_OUTPUT_BUFFER_INFO);
		if(p_ring_buffer_addr && p_ring_buffer_addr->pst_active_state)
			return TRUE;
		else
			return FALSE;
	}
#endif
#else
	if(Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE)
		return TRUE;
#endif
	return FALSE;
}

void scaler_pst_suspend(void)
{
#ifdef CONFIG_PST_ENABLE
	scaler_pst_suspend_resume.PST_I2RND_TOP_CTRL_S_R		   = IoReg_Read32(PST_I2RND_TOP_CTRL_reg);
	scaler_pst_suspend_resume.PST_I2RND_DMA_CTRL_S_R		   = IoReg_Read32(PST_I2RND_DMA_CTRL_reg);
	scaler_pst_suspend_resume.PST_I2RND_DMA_NUM0_S_R		   = IoReg_Read32(PST_I2RND_DMA_NUM0_reg);
	scaler_pst_suspend_resume.PST_I2RND_DMA_NUM1_S_R		   = IoReg_Read32(PST_I2RND_DMA_NUM1_reg);
	scaler_pst_suspend_resume.PST_I2RND_DMA_NUM2_S_R		   = IoReg_Read32(PST_I2RND_DMA_NUM2_reg);
	scaler_pst_suspend_resume.PST_I2RND_DMA_NUM3_S_R		   = IoReg_Read32(PST_I2RND_DMA_NUM3_reg);
	scaler_pst_suspend_resume.PST_I2RND_DMA_NUM4_S_R		   = IoReg_Read32(PST_I2RND_DMA_NUM4_reg);
	scaler_pst_suspend_resume.PST_I2RND_DMA_ADDR0_S_R		   = IoReg_Read32(PST_I2RND_DMA_ADDR0_reg);
	scaler_pst_suspend_resume.PST_I2RND_DMA_ADDR2_S_R		   = IoReg_Read32(PST_I2RND_DMA_ADDR2_reg);
	scaler_pst_suspend_resume.PST_I2RND_DMA_ADDR4_S_R		   = IoReg_Read32(PST_I2RND_DMA_ADDR4_reg);
	scaler_pst_suspend_resume.PST_I2RND_DMA_ADDR5_S_R		   = IoReg_Read32(PST_I2RND_DMA_ADDR5_reg);
	scaler_pst_suspend_resume.PST_I2RND_DMA_ADDR6_S_R		   = IoReg_Read32(PST_I2RND_DMA_ADDR6_reg);
	scaler_pst_suspend_resume.PST_I2RND_DMA_ADDR7_S_R		   = IoReg_Read32(PST_I2RND_DMA_ADDR7_reg);
	scaler_pst_suspend_resume.PST_I2RND_DMA_ADDR8_S_R		   = IoReg_Read32(PST_I2RND_DMA_ADDR8_reg);
	scaler_pst_suspend_resume.PST_I2RND_DMA_STEP0_S_R		   = IoReg_Read32(PST_I2RND_DMA_STEP0_reg);
	scaler_pst_suspend_resume.PST_I2RND_DMA_STEP2_S_R		   = IoReg_Read32(PST_I2RND_DMA_STEP2_reg);
	scaler_pst_suspend_resume.PST_I2RND_DMA_STEP4_S_R		   = IoReg_Read32(PST_I2RND_DMA_STEP4_reg);
	scaler_pst_suspend_resume.PST_I2RND_DMA_STEP5_S_R		   = IoReg_Read32(PST_I2RND_DMA_STEP5_reg);
	scaler_pst_suspend_resume.PST_I2RND_DMA_STEP6_S_R		   = IoReg_Read32(PST_I2RND_DMA_STEP6_reg);
	scaler_pst_suspend_resume.PST_I2RND_DMA_STEP7_S_R		   = IoReg_Read32(PST_I2RND_DMA_STEP7_reg);
	scaler_pst_suspend_resume.PST_I2RND_DMA_STEP8_S_R		   = IoReg_Read32(PST_I2RND_DMA_STEP8_reg);
	scaler_pst_suspend_resume.PST_I2RND_DMA_BUF_NUM_S_R 	   = IoReg_Read32(PST_I2RND_DMA_BUF_NUM_reg);
	scaler_pst_suspend_resume.PST_I2RND_DMA_WRT_PNT0_S_R	   = IoReg_Read32(PST_I2RND_DMA_WRT_PNT0_reg);
	scaler_pst_suspend_resume.PST_I2RND_DMA_WRT_PNT1_S_R	   = IoReg_Read32(PST_I2RND_DMA_WRT_PNT1_reg);
	scaler_pst_suspend_resume.PST_I2RND_DMA_WRT_PNT2_S_R	   = IoReg_Read32(PST_I2RND_DMA_WRT_PNT2_reg);
	scaler_pst_suspend_resume.PST_I2RND_DMA_READ_PNT0_S_R	   = IoReg_Read32(PST_I2RND_DMA_READ_PNT0_reg);
	scaler_pst_suspend_resume.PST_I2RND_DMA_READ_PNT1_S_R	   = IoReg_Read32(PST_I2RND_DMA_READ_PNT1_reg);
	scaler_pst_suspend_resume.PST_I2RND_DMA_READ_PNT2_S_R	   = IoReg_Read32(PST_I2RND_DMA_READ_PNT2_reg);
	scaler_pst_suspend_resume.PST_I2RND_DEBUG0_S_R			   = IoReg_Read32(PST_I2RND_DEBUG0_reg);
	scaler_pst_suspend_resume.PST_I2RND_DEBUG1_S_R			   = IoReg_Read32(PST_I2RND_DEBUG1_reg);
	scaler_pst_suspend_resume.PST_I2RND_DISPM_REPEAT_CTRL_S_R  = IoReg_Read32(PST_I2RND_DISPM_REPEAT_CTRL_reg);
	scaler_pst_suspend_resume.PST_I2RND_DISPM_REPEAT_DEBUG_S_R = IoReg_Read32(PST_I2RND_DISPM_REPEAT_DEBUG_reg);
	scaler_pst_suspend_resume.PST_I2RND_PST_MBIST_MODE_S_R	   = IoReg_Read32(PST_I2RND_PST_MBIST_MODE_reg);
	scaler_pst_suspend_resume.PST_I2RND_PST_MBIST_DONE_S_R	   = IoReg_Read32(PST_I2RND_PST_MBIST_DONE_reg);
	scaler_pst_suspend_resume.PST_I2RND_PST_MBIST_FAIL_S_R	   = IoReg_Read32(PST_I2RND_PST_MBIST_FAIL_reg);
	scaler_pst_suspend_resume.PST_I2RND_PST_DRF_MODE_S_R	   = IoReg_Read32(PST_I2RND_PST_DRF_MODE_reg);
	scaler_pst_suspend_resume.PST_I2RND_PST_DRF_RESUME_S_R	   = IoReg_Read32(PST_I2RND_PST_DRF_RESUME_reg);
	scaler_pst_suspend_resume.PST_I2RND_PST_DRF_DONE_S_R	   = IoReg_Read32(PST_I2RND_PST_DRF_DONE_reg);
	scaler_pst_suspend_resume.PST_I2RND_PST_DRF_PAUSE_S_R	   = IoReg_Read32(PST_I2RND_PST_DRF_PAUSE_reg);
	scaler_pst_suspend_resume.PST_I2RND_PST_MBIST_DRF_FAIL_S_R = IoReg_Read32(PST_I2RND_PST_MBIST_DRF_FAIL_reg);
	scaler_pst_suspend_resume.PST_I2RND_PST_MBIST_LS_S_R	   = IoReg_Read32(PST_I2RND_PST_MBIST_LS_reg);
	scaler_pst_suspend_resume.PST_I2RND_PST_MBIST_RM_S_R	   = IoReg_Read32(PST_I2RND_PST_MBIST_RM_reg);
#if 0 //FIXME: merlin8 compile error
	scaler_pst_suspend_resume.PST_I2RND_PST_MBIST_RME_S_R	   = IoReg_Read32(PST_I2RND_PST_MBIST_RME_reg);
#endif
#endif
}


void scaler_pst_resume(void)
{
#ifdef CONFIG_PST_ENABLE
	IoReg_Write32(PST_I2RND_TOP_CTRL_reg,			(scaler_pst_suspend_resume.PST_I2RND_TOP_CTRL_S_R & ~(_BIT3|_BIT1)));
	IoReg_Write32(PST_I2RND_DMA_CTRL_reg,			scaler_pst_suspend_resume.PST_I2RND_DMA_CTRL_S_R);
	IoReg_Write32(PST_I2RND_DMA_NUM0_reg,			scaler_pst_suspend_resume.PST_I2RND_DMA_NUM0_S_R);
	IoReg_Write32(PST_I2RND_DMA_NUM1_reg,			scaler_pst_suspend_resume.PST_I2RND_DMA_NUM1_S_R);
	IoReg_Write32(PST_I2RND_DMA_NUM2_reg,			scaler_pst_suspend_resume.PST_I2RND_DMA_NUM2_S_R);
	IoReg_Write32(PST_I2RND_DMA_NUM3_reg,			scaler_pst_suspend_resume.PST_I2RND_DMA_NUM3_S_R);
	IoReg_Write32(PST_I2RND_DMA_NUM4_reg,			scaler_pst_suspend_resume.PST_I2RND_DMA_NUM4_S_R);
	IoReg_Write32(PST_I2RND_DMA_ADDR0_reg,			scaler_pst_suspend_resume.PST_I2RND_DMA_ADDR0_S_R);
	IoReg_Write32(PST_I2RND_DMA_ADDR2_reg,			scaler_pst_suspend_resume.PST_I2RND_DMA_ADDR2_S_R);
	IoReg_Write32(PST_I2RND_DMA_ADDR4_reg,			scaler_pst_suspend_resume.PST_I2RND_DMA_ADDR4_S_R);
	IoReg_Write32(PST_I2RND_DMA_ADDR5_reg,			scaler_pst_suspend_resume.PST_I2RND_DMA_ADDR5_S_R);
	IoReg_Write32(PST_I2RND_DMA_ADDR6_reg,			scaler_pst_suspend_resume.PST_I2RND_DMA_ADDR6_S_R);
	IoReg_Write32(PST_I2RND_DMA_ADDR7_reg,			scaler_pst_suspend_resume.PST_I2RND_DMA_ADDR7_S_R);
	IoReg_Write32(PST_I2RND_DMA_ADDR8_reg,			scaler_pst_suspend_resume.PST_I2RND_DMA_ADDR8_S_R);
	IoReg_Write32(PST_I2RND_DMA_STEP0_reg,			scaler_pst_suspend_resume.PST_I2RND_DMA_STEP0_S_R);
	IoReg_Write32(PST_I2RND_DMA_STEP2_reg,			scaler_pst_suspend_resume.PST_I2RND_DMA_STEP2_S_R);
	IoReg_Write32(PST_I2RND_DMA_STEP4_reg,			scaler_pst_suspend_resume.PST_I2RND_DMA_STEP4_S_R);
	IoReg_Write32(PST_I2RND_DMA_STEP5_reg,			scaler_pst_suspend_resume.PST_I2RND_DMA_STEP5_S_R);
	IoReg_Write32(PST_I2RND_DMA_STEP6_reg,			scaler_pst_suspend_resume.PST_I2RND_DMA_STEP6_S_R);
	IoReg_Write32(PST_I2RND_DMA_STEP7_reg,			scaler_pst_suspend_resume.PST_I2RND_DMA_STEP7_S_R);
	IoReg_Write32(PST_I2RND_DMA_STEP8_reg,			scaler_pst_suspend_resume.PST_I2RND_DMA_STEP8_S_R);
	IoReg_Write32(PST_I2RND_DMA_BUF_NUM_reg,		scaler_pst_suspend_resume.PST_I2RND_DMA_BUF_NUM_S_R);
	IoReg_Write32(PST_I2RND_DMA_WRT_PNT0_reg,		scaler_pst_suspend_resume.PST_I2RND_DMA_WRT_PNT0_S_R);
	//reset write idx
	IoReg_Write32(PST_I2RND_DMA_WRT_PNT1_reg,		0);
	IoReg_Write32(PST_I2RND_DMA_WRT_PNT2_reg,		0);
	IoReg_Write32(PST_I2RND_DMA_READ_PNT0_reg,		scaler_pst_suspend_resume.PST_I2RND_DMA_READ_PNT0_S_R);
	IoReg_Write32(PST_I2RND_DMA_READ_PNT1_reg,		scaler_pst_suspend_resume.PST_I2RND_DMA_READ_PNT1_S_R);
	IoReg_Write32(PST_I2RND_DMA_READ_PNT2_reg,		scaler_pst_suspend_resume.PST_I2RND_DMA_READ_PNT2_S_R);
	IoReg_Write32(PST_I2RND_DEBUG0_reg, 			scaler_pst_suspend_resume.PST_I2RND_DEBUG0_S_R);
	IoReg_Write32(PST_I2RND_DEBUG1_reg, 			scaler_pst_suspend_resume.PST_I2RND_DEBUG1_S_R);
	IoReg_Write32(PST_I2RND_DISPM_REPEAT_CTRL_reg,	scaler_pst_suspend_resume.PST_I2RND_DISPM_REPEAT_CTRL_S_R);
	IoReg_Write32(PST_I2RND_DISPM_REPEAT_DEBUG_reg, scaler_pst_suspend_resume.PST_I2RND_DISPM_REPEAT_DEBUG_S_R);
	IoReg_Write32(PST_I2RND_PST_MBIST_MODE_reg, 	scaler_pst_suspend_resume.PST_I2RND_PST_MBIST_MODE_S_R);
	IoReg_Write32(PST_I2RND_PST_MBIST_DONE_reg, 	scaler_pst_suspend_resume.PST_I2RND_PST_MBIST_DONE_S_R);
	IoReg_Write32(PST_I2RND_PST_MBIST_FAIL_reg, 	scaler_pst_suspend_resume.PST_I2RND_PST_MBIST_FAIL_S_R);
	IoReg_Write32(PST_I2RND_PST_DRF_MODE_reg,		scaler_pst_suspend_resume.PST_I2RND_PST_DRF_MODE_S_R);
	IoReg_Write32(PST_I2RND_PST_DRF_RESUME_reg, 	scaler_pst_suspend_resume.PST_I2RND_PST_DRF_RESUME_S_R);
	IoReg_Write32(PST_I2RND_PST_DRF_DONE_reg,		scaler_pst_suspend_resume.PST_I2RND_PST_DRF_DONE_S_R);
	IoReg_Write32(PST_I2RND_PST_DRF_PAUSE_reg,		scaler_pst_suspend_resume.PST_I2RND_PST_DRF_PAUSE_S_R);
	IoReg_Write32(PST_I2RND_PST_MBIST_DRF_FAIL_reg, scaler_pst_suspend_resume.PST_I2RND_PST_MBIST_DRF_FAIL_S_R);
	IoReg_Write32(PST_I2RND_PST_MBIST_LS_reg,		scaler_pst_suspend_resume.PST_I2RND_PST_MBIST_LS_S_R);
	IoReg_Write32(PST_I2RND_PST_MBIST_RM_reg,		scaler_pst_suspend_resume.PST_I2RND_PST_MBIST_RM_S_R);
#if 0 //FIXME: merlin8 compile error
	IoReg_Write32(PST_I2RND_PST_MBIST_RME_reg,		scaler_pst_suspend_resume.PST_I2RND_PST_MBIST_RME_S_R);
#endif
#endif

}



/*======================== End of File =======================================*/
/**
*
* @}
*/

