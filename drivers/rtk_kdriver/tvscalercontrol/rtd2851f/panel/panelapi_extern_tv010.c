#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <mach/io.h>
#include <generated/autoconf.h>


#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerCommon.h>
#else
#include <scalercommon/scalerCommon.h>
#endif

#include <tvscalercontrol/panel/panelapi.h>
#include <tvscalercontrol/panel/panelapi_extern_tv010.h>
//#include <tvscalercontrol/panel/panelTconCtrlAPI.h>
#include <tvscalercontrol/panel/panel.h>
#include <tvscalercontrol/io/ioregdrv.h>

#include <tvscalercontrol/hdmirx/hdmifun.h>
#include <tvscalercontrol/scaler/scalerstruct.h>

#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerDrvCommon.h>
#else
#include <scalercommon/scalerDrvCommon.h>
#endif

#include <rbus/mdomain_cap_reg.h>
//#include <rbus/rbus_DesignSpec_MISC_GPIOReg.h>
#include <rbus/timer_reg.h>
#include <rtd_log/rtd_module_log.h>



#ifdef CONFIG_PCBMGR
#include <Platform_Lib/Board/pcbMgr.h>
#endif

//USER: Vincent_Lee  DATE_2012_5_18  TODO: Add soft I2C for Gamma IC control (BUF_16821)
#ifdef OPEN_CELL_PANEL
#include <Platform_Lib/softi2c/softi2c.h>
#endif

//#include <rbus/scaler/rbusPpOverlayReg.h>
#include <rbus/ppoverlay_reg.h>
#include <rbus/ldspi_reg.h>
//#include <rbus/scaler/rbusPifReg.h>
#include <rbus/pif_reg.h>
#include <tvscalercontrol/scalerdrv/adjust.h>
#include <tvscalercontrol/scalerdrv/framesync.h>
//#include <rbus/scaler/tve_reg.h>

// osd shift
//#include <rpc/VideoRPCBaseDS_data.h>
// video shift
#include <rbus/d_2d_to_3d_reg.h>
#include <rbus/misc_reg.h>
#include <rbus/con_bri_reg.h>

//Leo Chen+
//#include <sys/ioctl.h>

//for kernel
#define Rt_Sleep(mSec)					msleep(mSec)
#define Rt_Delay(mSec)					msleep(mSec)


extern PANEL_CONFIG_PARAMETER *pPanelCOnfigParameter;


#ifdef CONFIG_CUSTOMER_TV010
unsigned char Get_PANEL_LD_Enable(void)
{
	if (pPanelCOnfigParameter!=NULL)
		return pPanelCOnfigParameter->iCONFIG_LD_Enable;

	return 0;
}

unsigned char Get_PANEL_LD_SPIDecoderMcuInPower_Enable(void)
{
	if (pPanelCOnfigParameter!=NULL)
		return pPanelCOnfigParameter->iCONFIG_LD_SPIDecoderMcuInPower_Enable;

	return 0;
}

unsigned char Get_PANEL_LD_Alogorithm_Soc(void)
{
	if (pPanelCOnfigParameter!=NULL)
		return pPanelCOnfigParameter->iCONFIG_LD_Alogorithm_Soc;

	return 0;
}

unsigned char Get_PANEL_LD_Partition_Amount(void)
{
	if (pPanelCOnfigParameter!=NULL)
		return pPanelCOnfigParameter->iCONFIG_LD_Partition_Amount;

	return 0;
}
#endif

#ifdef CONFIG_CUSTOMER_TV010
#define step 1
#define Right_thl  2//7
#define Down_thl 1// 4
#define Left_thl  -2// -7
#define Up_thl   -1// -4

void Video_H_Shift(int x)
{

	ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;
	ppoverlay_dh_total_last_line_length_RBUS dh_total_last_line_length_reg;


	d_2d_to_3d_d_2d3d_db_control_RBUS d_2d3d_db_control_reg;
	d_2d_to_3d_d_2d3d_control_RBUS d_2d3d_control_reg;
	d_2d_to_3d_d_2d3d_tilt_operation_RBUS d_2d3d_tilt_operation_reg;
	d_2d_to_3d_d_2d3d_tilt_depth_RBUS d_2d3d_tilt_depth_reg;
	d_2d_to_3d_d_2d3d_b_border_RBUS d_2d3d_b_border_reg;

	ppoverlay_main_active_h_start_end_RBUS main_active_h_start_end_reg;
       char timeout = 0;
       ppoverlay_mp_layout_force_to_background_RBUS mp_layout_force_to_background_reg;
	con_bri_ds_color_rgb_ctrl_RBUS con_bri_ds_color_rgb_ctrl_reg;
	con_bri_ds_contrast_a_RBUS con_bri_ds_contrast_a_reg;
	ppoverlay_sub_den_h_start_width_RBUS sub_den_h_start_width_reg;
	ppoverlay_sub_den_v_start_length_RBUS sub_den_v_start_length_reg;

       mp_layout_force_to_background_reg.regValue = IoReg_Read32(PPOVERLAY_MP_Layout_Force_to_Background_reg);
	mp_layout_force_to_background_reg.s_disp_en = 1;
	IoReg_Write32(PPOVERLAY_MP_Layout_Force_to_Background_reg,mp_layout_force_to_background_reg.regValue);

	con_bri_ds_color_rgb_ctrl_reg.regValue = IoReg_Read32(CON_BRI_DS_COLOR_RGB_CTRL_reg);
	con_bri_ds_contrast_a_reg.regValue = IoReg_Read32(CON_BRI_DS_CONTRAST_A_reg);
	if(x == 0)
		con_bri_ds_color_rgb_ctrl_reg.s_con_en = 0;
	else
		con_bri_ds_color_rgb_ctrl_reg.s_con_en = 1;
	con_bri_ds_contrast_a_reg.s_con_a_r = 0;
	con_bri_ds_contrast_a_reg.s_con_a_g = 0;
	con_bri_ds_contrast_a_reg.s_con_a_b= 0;
	IoReg_Write32(CON_BRI_DS_COLOR_RGB_CTRL_reg,con_bri_ds_color_rgb_ctrl_reg.regValue);
	IoReg_Write32(CON_BRI_DS_CONTRAST_A_reg,con_bri_ds_contrast_a_reg.regValue);

	sub_den_h_start_width_reg.regValue = IoReg_Read32(PPOVERLAY_SUB_DEN_H_Start_Width_reg);	
	if(x > 0)
	{
		sub_den_h_start_width_reg.sh_den_sta = 0;
		sub_den_h_start_width_reg.sh_den_width = x+1;
	}
	else
	{
		sub_den_h_start_width_reg.sh_den_sta = 3839/2 + x;
		sub_den_h_start_width_reg.sh_den_width = 1 - x;
	}

	IoReg_Write32(PPOVERLAY_SUB_DEN_H_Start_Width_reg,sub_den_h_start_width_reg.regValue);

	sub_den_v_start_length_reg.regValue = IoReg_Read32(PPOVERLAY_SUB_DEN_V_Start_Length_reg);
	sub_den_v_start_length_reg.sv_den_sta = 0x0;
	sub_den_v_start_length_reg.sv_den_length = 0x870/2;
	IoReg_Write32(PPOVERLAY_SUB_DEN_V_Start_Length_reg,sub_den_v_start_length_reg.regValue);
#if 0 //tingguang_he fix left white line when black screen
	main_active_h_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_Active_H_Start_End_reg);
       d_2d3d_b_border_reg.regValue = IoReg_Read32(D_2D_TO_3D_D_2D3D_B_BORDER_reg);
	if(x==0)
		d_2d3d_b_border_reg.border_en = 0x0;
	else
		d_2d3d_b_border_reg.border_en = 0x1;
	if(x >= 0)
	{
		d_2d3d_b_border_reg.border_den_start = x;
		d_2d3d_b_border_reg.border_den_end = (main_active_h_start_end_reg.mh_act_end-main_active_h_start_end_reg.mh_act_sta);
	}
	else
	{
		d_2d3d_b_border_reg.border_den_start = 0;
		d_2d3d_b_border_reg.border_den_end = (main_active_h_start_end_reg.mh_act_end-main_active_h_start_end_reg.mh_act_sta + x);
	}
	IoReg_Write32(D_2D_TO_3D_D_2D3D_B_BORDER_reg,d_2d3d_b_border_reg.regValue);

	rtd_pr_vbe_info("\n border start=%d end=%d\n",d_2d3d_b_border_reg.border_den_start,d_2d3d_b_border_reg.border_den_end);
#endif
	d_2d3d_control_reg.regValue = IoReg_Read32(D_2D_TO_3D_D_2D3D_Control_reg);
	d_2d3d_control_reg.render_method = 0x1;
	d_2d3d_control_reg.output_format = 0x0;
	d_2d3d_control_reg.l_frame_shift_en = 0x1;
	d_2d3d_control_reg.r_frame_shift_en = 0x1;
	d_2d3d_control_reg.tilt_depth_en = 0x1;
	if(x >= 0)
	{
		d_2d3d_control_reg.l_flag_inv = 0x1;
	}
	else
	{
		d_2d3d_control_reg.l_flag_inv = 0x0;
		x = 0 - x;
	}
       IoReg_Write32(D_2D_TO_3D_D_2D3D_Control_reg,d_2d3d_control_reg.regValue);

	d_2d3d_tilt_operation_reg.regValue = IoReg_Read32(D_2D_TO_3D_D_2D3D_Tilt_Operation_reg);
	d_2d3d_tilt_operation_reg.tilt_gain = 0xff;
	IoReg_Write32(D_2D_TO_3D_D_2D3D_Tilt_Operation_reg,d_2d3d_tilt_operation_reg.regValue);

	d_2d3d_tilt_depth_reg.regValue = IoReg_Read32(D_2D_TO_3D_D_2D3D_TILT_DEPTH_reg);
	d_2d3d_tilt_depth_reg.initial_depth= x + 1;
	IoReg_Write32(D_2D_TO_3D_D_2D3D_TILT_DEPTH_reg,d_2d3d_tilt_depth_reg.regValue);

	 d_2d3d_db_control_reg.regValue = IoReg_Read32(D_2D_TO_3D_D_2D3D_Db_Control_reg);
	 d_2d3d_db_control_reg.db_apply= 0x01;
	 IoReg_Write32(D_2D_TO_3D_D_2D3D_Db_Control_reg,d_2d3d_db_control_reg.regValue);
	 while(IoReg_Read32(D_2D_TO_3D_D_2D3D_Db_Control_reg)&_BIT29)
	 {
		timeout++;
		if(timeout >= 10)
			break;
		Rt_Sleep(10);
	 }
	 rtd_pr_vbe_info("2d3d double buffer timeout = %d\n",timeout);
	d_2d3d_db_control_reg.regValue = IoReg_Read32(D_2D_TO_3D_D_2D3D_Db_Control_reg);
	d_2d3d_db_control_reg.db_en = 0x0;
	IoReg_Write32(D_2D_TO_3D_D_2D3D_Db_Control_reg,d_2d3d_db_control_reg.regValue);
}

#define SHIFT_FACTOR 1
//#define GOLDEN_VSYC_LINE 0x9b
//#define GOLDEN_VSYC_PIXEL 0x40b

#define GOLDEN_VSYC_LINE 0x4d
#define GOLDEN_VSYC_PIXEL 0x65a
void Video_V_Shift(int y)
{
	unsigned int orbit_pixel_shift, orbit_line_shift;
	ppoverlay_memcdtg_control3_RBUS memcdtg_control3_reg;
	ppoverlay_memc_mux_ctrl_RBUS memc_mux_ctrl_reg;
	ppoverlay_double_buffer_ctrl2_RBUS double_buffer_ctrl2_reg;


	//set golden vs
	/*
	double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl2_reg.regValue);
	memc_mux_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg);
	memc_mux_ctrl_reg.memcdtg_golden_vs = 1;
	IoReg_Write32(PPOVERLAY_MEMC_MUX_CTRL_reg,memc_mux_ctrl_reg.regValue);
	double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	double_buffer_ctrl2_reg.uzudtgreg_dbuf_set = 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl2_reg.regValue);
	*/

	//set pixel shift
	rtd_pr_vbe_info("orbit v postion = %d\n",y);
	orbit_line_shift  = GOLDEN_VSYC_LINE   + (0*SHIFT_FACTOR) - (y*SHIFT_FACTOR);
	orbit_pixel_shift = GOLDEN_VSYC_PIXEL  + (0*SHIFT_FACTOR) - (0*SHIFT_FACTOR);

	memcdtg_control3_reg.regValue = IoReg_Read32(PPOVERLAY_MEMCDTG_CONTROL3_reg);
	memcdtg_control3_reg.dtgm2goldenpostvs_line = orbit_line_shift;
	memcdtg_control3_reg.dtgm2goldenpostvs_pixel = orbit_pixel_shift;
	IoReg_Write32(PPOVERLAY_MEMCDTG_CONTROL3_reg, memcdtg_control3_reg.regValue);

	IoReg_SetBits(PPOVERLAY_Double_Buffer_CTRL_reg, _BIT0);
	rtd_pr_vbe_info("orbit update(line:%d)\n",orbit_line_shift);


}
#if 0
char OsdLayer_Shift(int x,int y)
{
	int gdma_fd;

	 gdma_fd = open("/dev/gdma0", O_RDONLY);
        if(gdma_fd <= 0) {
            rtd_pr_vbe_info("[GDMA]@%s, gdma fd open fail!! %d \n",__func__, gdma_fd);
            return false;
        }
	 else
	 {
	     //rtd_pr_vbe_info("[GDMA]@%s, gdma fd open success!! %d \n",__func__, gdma_fd);
	 }

	CONFIG_OSDSHIFT_STRUCT osdshift;
	osdshift.plane = (VO_GRAPHIC_PLANE)1; //VO_GRAPHIC_OSD1;
	osdshift.shift_h_enable = 1;
	osdshift.shift_v_enable = 1;
	osdshift.h_shift_pixel = x;
	osdshift.v_shift_pixel = y;
	ioctl(gdma_fd, GDMA_CONFIG_OSD_SHIFT, &osdshift);
	//rtd_pr_vbe_info("GDMA_CONFIG_OSD_SHIFT is %d\n", GDMA_CONFIG_OSD_SHIFT);
	close(gdma_fd);

	return true;
}
#endif

static int x = 0;
static int y = 0;
static char direction = 0;

void Panel_ResetScreenShiftPos(void)
{
	if((Get_DISPLAY_PANEL_CUSTOM_INDEX() != VBY_ONE_PANEL_LG_OLED_4K2K_120HZ_4S_16L) &&
		 (Get_DISPLAY_PANEL_CUSTOM_INDEX() != VBY_ONE_PANEL_LG_OLED_2K1K_120HZ_1S_4L))
       {
		rtd_pr_vbe_info("\n non support 60hz panel ! force return !\n");
		return;
       }
	rtd_pr_vbe_info("\n Reset Screen Shift Pos\n");
	x = 0;
	y = 0;
	direction = 0;
	Video_H_Shift(x);
	Video_V_Shift(y);
}

void Panel_ScreenShift(void)
{
         if((Get_DISPLAY_PANEL_CUSTOM_INDEX() != VBY_ONE_PANEL_LG_OLED_4K2K_120HZ_4S_16L) &&
		 (Get_DISPLAY_PANEL_CUSTOM_INDEX() != VBY_ONE_PANEL_LG_OLED_2K1K_120HZ_1S_4L))
         {
		rtd_pr_vbe_info("\n non support 60hz panel ! force return !\n");
		return;
         }
//	  if(getMagicPictureFlag())
//	  {
//	  	rtd_pr_vbe_info("\n enter Highlight Window demo mode ! force return !file:%s \n",__FILE__);
//		return;
//	  }
	   switch(direction)
	   {
		case 0:
			x += step;
			if(x >= step*Right_thl)
				direction = 1;
			break;

		case 1:
			y += step;
			if(y >= step*Down_thl)
				direction = 2;
			break;

		case 2:
			x -= step;
			if(x <= step*Left_thl)
				direction = 3;
			break;

		case 3:
			y -= step;
			if(y <= step*Up_thl)
				direction = 4;
			break;

		case 4:
			x += step;
			if(x >= step*Right_thl)
				direction = 5;
			break;

		case 5:
			y += step;
			if(y >= 0)
				direction = 6;
			break;

		case 6:
			x -= step;
			if(x <= 0)
				direction = 0;
			break;

		default:
			break;
	}
	//OsdLayer_Shift(x,y);
	if(Scaler_DispGetInputInfo(SLR_INPUT_STATE)==_MODE_STATE_ACTIVE)
	{
		Video_H_Shift(x);
		Video_V_Shift(y);
	}
	rtd_pr_vbe_info("\n H_pos=%d V_pos=%d\n",x,y);


}
#endif


