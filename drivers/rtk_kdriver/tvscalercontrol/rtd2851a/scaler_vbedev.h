#ifndef VBE_EXPORT_H
#define VBE_EXPORT_H
#ifndef BUILD_QUICK_SHOW
#include <linux/types.h>
#include <linux/fs.h>
#include <rtd_types.h>
#endif
#include <ioctrl/scaler/vbe_cmd_id.h>
#include <ioctrl/scaler/vsc_cmd_id.h>
#include <scaler/scalerCommon.h>
//#include <linux/v4l2-ext/videodev2-ext.h>
#include <tvscalercontrol/scalerdrv/scalerdrv.h>


typedef struct{
	int  x;
	int  y;
}ORBIT_PIXEL_SHIFT_STRUCT;

typedef struct ORBIT_SZIE {
	unsigned int	nWidth;
	unsigned int	nLength;
} ORBIT_SIZE;



enum vbe_panel_orbit_mode {
    _VBE_PANEL_ORBIT_JUSTSCAN_MODE = 0,
    _VBE_PANEL_ORBIT_AUTO_MODE,
    _VBE_PANEL_ORBIT_STORE_MODE,
    _VBE_PANEL_OREBIT_MODE_MAX
};


unsigned char Get_PANEL_BL_type(void);
void Set_PANEL_BL_type(unsigned char type);
unsigned char Get_PANEL_LED_Bar_type(void);
void Set_PANEL_LED_Bar_type(unsigned char type);
unsigned char Get_PANEL_EPI_TYPE(void);
void Set_PANEL_EPI_TYPE(unsigned char type);
unsigned char Get_PANEL_INCH(void);
void Set_PANEL_INCH(unsigned char inch);
unsigned char Get_PANEL_MAKER(void);
void Set_PANEL_MAKER(unsigned char maker);
unsigned int Get_DISP_OSD_Width(void);
void Set_DISP_OSD_Width(unsigned int width);
unsigned int Get_DISP_OSD_Height(void);
void Set_DISP_OSD_Height(unsigned int height);
void Disp_Enable_isForceFreeRun(BOOLEAN isForceFreeRun);
BOOLEAN Disp_Get_isForceFreeRun(void);
void Disp_Set_framerate(unsigned short framerate);
unsigned short Disp_Get_framerate(void);
void modestate_set_fll_running_flag(unsigned char bEnable);
unsigned char modestate_get_fll_running_flag(void);
unsigned char vbe_disp_get_adaptivestream_fs_mode(void);
void scaler_disp_smooth_variable_setting(void);
void scaler_disp_smooth_variable_settingByDisp(unsigned char _channel);
void vbe_disp_first_resume(void);
void vbe_disp_resume(void);
void vbe_disp_tcon_on(void);
void vbe_disp_tcon_off(void);
void vbe_disp_tcon_verifyOnOff(char *bOnOff);
unsigned char vbe_disp_tcon_lock_self_check(void);
char vbe_disp_tcon_readStatus(void);
unsigned char vbe_disp_always_check_tcon(void);
void vbe_disp_vby1_init_flow(void);
void vbe_disp_orbit_position_update(unsigned char tableIdx);
unsigned int vbe_disp_orbit_get_max_table_index(void);
void vbe_disp_set_orbit(UINT8 bEnable, unsigned char mode);
unsigned char vbe_disp_get_orbit_mode(void);
unsigned char vbe_disp_get_orbit(void);
unsigned int vbe_disp_orbit_get_mode_table_shift_x(void);
unsigned int vbe_disp_orbit_get_mode_table_shift_y(void);
unsigned char vbe_disp_decide_PQ_power_saveing(void);
unsigned char vbe_disp_decide_memc_mux_bypass(void);
void vbe_disp_set_dynamic_memc_bypass_flag(unsigned char bOnOff);
unsigned char vbe_disp_get_dynamic_memc_bypass_flag(void);
void vbe_disp_game_mode_memc_bypass(unsigned char b_gameMode_onOff);
void vbe_disp_pwm_frequency_update(bool bEnableDB);
void vbe_disp_lane_power_off(void);
void vbe_disp_lane_power_on(void);
void vbe_disp_debug_framerate_change_msg(void);
unsigned char vbe_disp_gamemode_use_fixlastline(void);
unsigned char vbe_disp_dataFramesync_use_fixlastline(void);
void vbe_disp_set_dtgM2uzu_lowdelay_mode(unsigned char bon_off);
void vbe_disp_vcomPatternDraw( UINT16 *vcomPattern,UINT16 nSize );
void vbe_disp_set_adaptivestream_force_framerate(unsigned int framerate);
unsigned int vbe_disp_get_adaptivestream_force_framerate(void);
unsigned char vbe_disp_decide_frc2fsync_using_I2D(void);
void scaler_disp_dtg_pre_i_domain_setting(unsigned char display);
void vbe_disp_dynamic_polarity_control_init(void);
void vbe_disp_dynamic_polarity_control_set_detect_region(KADP_VIDEO_RECT_T detectRegions);
void vbe_disp_dynamic_polarity_control_set_detect_framecounts(unsigned int detectFrames);
unsigned int vbe_disp_dynamic_polarity_control_get_detect_framecounts(void);
void vbe_disp_dynamic_polarity_control_set_detect_thresholds(unsigned int threshold);
unsigned int vbe_disp_dynamic_polarity_control_get_detect_threshold(void);
void vbe_disp_dynamic_polarity_control_set_detect_enable(unsigned char bEnable);
void vbe_disp_dynamic_polarity_control_set_control_packet(unsigned char bEnable);
void vbe_disp_dynamic_polarity_control_analyze_pattern(void);
void vbe_disp_tcon_28s_toggle_set_state(unsigned char bState);
unsigned char vbe_disp_tcon_28s_toggle_get_state(void);
void vbe_disp_tcon_28s_toggle_handle_state(unsigned char bState);
unsigned char vbe_disp_tcon_28s_toggle_check_timeout(void);
void vbe_disp_tcon_28s_toggle_init(void);
void vbe_disp_set_orbit_shift_info_sharememory(SCALER_ORBIT_SHIFT_INFO scalerOrbitInfo);
unsigned char vbe_disp_orbit_set_position_justscan(int x, int y);
void vbe_disp_orbit_set_position_overscan(int x, int y);


ORBIT_PIXEL_SHIFT_STRUCT Get_Orbit_Shift_Data(void);
void Set_Orbit_Shift_Data(ORBIT_PIXEL_SHIFT_STRUCT orbit_shift_data);
void vbe_disp_orbit_vo_overscan(void);
unsigned char vbe_disp_orbit_get_vo_overscan(void);
void vbe_disp_orbit_set_vo_overscan(unsigned char bEnable);
void vbe_disp_orbit_set_vo_overscan_state(unsigned char bEnable);
void vbe_disp_orbit_frc_overscan(void);
unsigned char vbe_disp_orbit_get_frc_overscan(void);
void vbe_disp_orbit_set_frc_overscan(unsigned char bEnable);
unsigned char vbe_disp_orbit_get_osd_overscan(void);
void vbe_disp_orbit_set_osd_overscan(unsigned char bEnable);
void vbe_disp_orbit_OSD_overscan(void);



/*declaration for VSC use*/
int HAL_VBE_AVE_Initialize(void);
int HAL_VBE_AVE_Open(void);
int HAL_VBE_AVE_Connect(KADP_VBE_AVE_INPUT_INDEX AVEInput);
int HAL_VBE_AVE_Disconnect(void);
void Scaler_disp_setting(unsigned char display);
void HAL_VBE_DISP_SetFrameRate(BOOLEAN isForceFreeRun,UINT16 frameRates);
void HAL_VBE_DISP_TCON_Initialize(void);
void HAL_VBE_DISP_TCON_Uninitialize(void);
void HAL_VBE_DISP_TCON_EnableColorTemp(bool bOnOff);
void HAL_VBE_DISP_TCON_EnableDGA(bool bOnOff);
void HAL_VBE_DISP_TCON_EnableDither(bool bOnOff);
void HAL_VBE_DISP_TCON_SetDitherMode(KADP_DISP_TCON_DITHER_T mode);
void HAL_VBE_DISP_TCON_SetGammaTable(UINT32 *pRedGammaTable, UINT32 *pGreenGammaTable, UINT32 *pBlueGammaTable);
void HAL_VBE_DISP_TCON_EnableTCon(bool bOnOff);
void HAL_VBE_DISP_TCON_H_Reverse(UINT8 u8Mode);
void HAL_VBE_DISP_TCON_SetClock(bool bOnOff);
void HAL_VBE_DISP_TCON_WriteRegister(UINT32 u32Address, UINT32 u32Data);
void HAL_VBE_DISP_TCON_ReadRegister(UINT32 u32Address, UINT32 *pData);
void HAL_VBE_DISP_TCON_SetLvdsFormat(KADP_VBE_DISP_TCON_LVDS_TYPE_T type);
void HAL_VBE_DISP_TCON_Debug(void);
void HAL_VBE_DISP_OLED_SetOrbit(BOOLEAN bEnable, unsigned char mode);
void HAL_VBE_DISP_SetBOEMode(UINT8 *pFwData, UINT32 size);
void HAL_VBE_DISP_GetBOEVersion(KADP_DISP_BOE_VERSION_T *pstRomVer, KADP_DISP_BOE_VERSION_T *pstFwVer);
#ifdef CONFIG_SCALER_ENABLE_V4L2
void HAL_VBE_DISP_SetMLEMode(enum v4l2_ext_vbe_mplus_mode index);
#endif
void HAL_VBE_DISP_SetInnerPattern(UINT8 bOnOff, VBE_DISP_INNER_PTG_BLOCK_T block, VBE_DISP_INNER_PTG_TYPE_T type);
void HAL_VBE_DISP_GetPanelSize (KADP_DISP_PANEL_SIZE_T *panel_size);
void HAL_VBE_SetDGA4CH (UINT32 *pRedGammaTable, UINT32 *pGreenGammaTable, UINT32 *pBlueGammaTable, UINT32 *pWhiteGammaTable, UINT16 nTableSize);
void HAL_VBE_DISP_SetOLEDLSR (VBE_DISP_LSR_ADJUST_MODE_T nStep, UINT32 *LSRTbl);
void HAL_VBE_DISP_SetOLEDLSR_Tunning (VBE_DISP_LSR_TUNNING_VAL_T *stLSRdata);
void HAL_VBE_DISP_SetOLEDCPC_Tunning (VBE_DISP_CPC_TUNNING_VAL_T *stCPCdata);
void HAL_VBE_DISP_SetOLEDGSR (UINT32 *GSRTbl);
void HAL_VBE_DISP_Set_Panel_Bit_Mode(UINT8 bitMode);
void HAL_VBE_DISP_Set_Panel_Ti_Mode(UINT8 table);
void HAL_VBE_DISP_Set_Panel_Swap_Mode(UINT8 bSwap);
void HAL_VBE_DISP_Set_Panel_SFG_Dual_Mode(UINT8 dualMode);
void HAL_VBE_DISP_Get_Panel_Ti_Mode(UINT8* table);
void HAL_VBE_DISP_Get_Panel_Bit_Mode(UINT8* bitMode);
void HAL_VBE_DISP_Get_Panel_SFG_Dual_Mode(UINT8* dualMode);

void Scaler_wait_for_input_one_frame(unsigned char display);

void vbe_disp_VRR_framerate_protect_check(void);
unsigned char vbe_disp_check_support_VRR_framerate_range(void);
UINT32 vbe_disp_get_VRR_timing_real_vtotal(void);
VBE_DISP_VRR_FRACTIONAL_TIMING_MODE_T vbe_disp_get_VRR_timing_fractional_mode(void);
unsigned char vbe_disp_set_VRR_fractional_framerate_mode(void);
unsigned char vbe_disp_get_VRR_timingMode_flag(void);
unsigned char vbe_disp_get_VRR_ALLM_flag(void);
unsigned char vbe_disp_get_pre_VRR_ALLM_flag(void);
void vbe_disp_set_pre_VRR_ALLM_flag (unsigned char enable);
unsigned char vbe_disp_get_freesync_mode_flag(void);
void vbe_disp_set_freesync_mode_flag (unsigned char enable);
void Set_Val_VRR_timingMode_flag(unsigned char flag);
unsigned int Get_Input_vfreq_by_vsync_cnt(void);
StructSrcRect vbe_orbit_get_input_size(void);
StructSrcRect vbe_orbit_get_output_size(void);
void vbe_orbit_set_input_size(StructSrcRect size);
void vbe_orbit_set_output_size(StructSrcRect size);
void Set_Val_vbe_disp_smooth_variable_setting_flag(unsigned int enable);
unsigned int Get_Val_vbe_disp_smooth_variable_setting_flag(void);
void vbe_set_HDMI_dynamic_film_mode_flag(unsigned int status);

extern int create_timer(unsigned char id, unsigned int interval, unsigned char mode);
extern int rtk_timer_control(unsigned char id, unsigned int cmd);
#ifndef BUILD_QUICK_SHOW
void Scaler_wait_for_input_one_frame(unsigned char display);
long vbe_ioctl(struct file *file, unsigned int cmd,  unsigned long arg);
#endif
void Set_Val_first_boot_run_main(UINT8 set_value);
UINT8 Get_Val_first_boot_run_main(void);
void Set_Val_vbe_ioctl_retval(UINT8 set_value);
UINT8 Get_Val_vbe_ioctl_retval(void);
void Set_Val_vbe_disp_global_status(unsigned char value);
unsigned char Get_Val_vbe_disp_global_status(void);
void scaler_disp_get_remove_multiple_from_uzudtg_fractional(unsigned int *pRemoveRatio, unsigned int *pMultipleRatio);

#define FRAME_RATE_24HZ_LOW   (2350)
#define FRAME_RATE_24HZ_UP    (2450)

#define FRAME_RATE_25HZ_LOW   (2450)
#define FRAME_RATE_25HZ_UP    (2550)

#define FRAME_RATE_30HZ_LOW   (2950)
#define FRAME_RATE_30HZ_UP    (3050)

#define FRAME_RATE_48HZ_LOW   (4750)
#define FRAME_RATE_48HZ_UP    (4850)

#define FRAME_RATE_50HZ_LOW   (4950)
#define FRAME_RATE_50HZ_UP    (5050)

#define FRAME_RATE_60HZ_LOW   (5950)
#define FRAME_RATE_60HZ_UP    (6050)

#define FRAME_RATE_100HZ_LOW   (9950)
#define FRAME_RATE_100HZ_UP    (10050)

#define FRAME_RATE_120HZ_LOW   (11950)
#define FRAME_RATE_120HZ_UP    (12050)

#define FRAME_RATE_144HZ_LOW   (14350)
#define FRAME_RATE_144HZ_UP    (14450)

#define FRAME_RATE_165HZ_LOW   (16450)
#define FRAME_RATE_165HZ_UP    (16550)

#define FRAME_RATE_23_9HZ      (2390)
#define FRAME_RATE_23_97HZ      (2397)
#define FRAME_RATE_29_9HZ      (2990)
#define FRAME_RATE_29_97HZ      (2997)
#define FRAME_RATE_59_9HZ      (5990)
#define FRAME_RATE_59_94HZ      (5994)
#define FRAME_RATE_60HZ        (6000)
#define FRAME_RATE_24HZ        (2400)
#define FRAME_RATE_25HZ        (2500)
#define FRAME_RATE_50HZ        (5000)
#define FRAME_RATE_48HZ        (4800)
#define FRAME_RATE_30HZ        (3000)
#define FRAME_RATE_100HZ       (10000)
#define FRAME_RATE_120HZ       (12000) 
#define FRAME_RATE_144HZ       (14400) 
#define FRAME_RATE_165HZ       (16500) 

#define VTOTAL_4K_60HZ              (2250)
#define HTOTAL_4K_60HZ              (4400)
#define VTOTAL_4K_50HZ              (2700)
#define VTOTAL_4K_48HZ              (2250)


#define DCLK_VALUE             (594 * 1000000)

#define VBE_ERROR              (1)
#define VBE_SUCCESS            (0)

#define REFRESH_RATE_120HZ      (120)
#define REFRESH_RATE_60HZ        (60)


typedef enum
{
    FR_24_TYPE,        //24Hz
    FR_25_TYPE,        //25Hz
    FR_30_TYPE,        //30Hz
    FR_48_TYPE,        //48Hz
    FR_50_TYPE,        //50Hz
    FR_60_TYPE,        //60Hz
    FR_100_TYPE,       //100Hz
    FR_120_TYPE,       //120Hz
    FR_144_TYPE,       //144Hz
    FR_165_TYPE,       //165Hz
    FR_UNKNOWN_TYPE,
}FRAME_RATE_TYPE;

typedef enum
{
    VTOTAL_FOR_NORMAL,
    VTOTAL_FOR_PMW
}PROTECT_VTOTAL_TYPE;
unsigned int scaler_disp_smooth_get_memc_protect_vtotal(PROTECT_VTOTAL_TYPE eType);
unsigned char Get_Val_VBE_Tridfmt_Global_Status(void);
void Set_Val_VBE_Tridfmt_Global_Status(unsigned char uc_value);
void Set_Val_vbe_ioctl_cmd(unsigned int cmd_id);
unsigned int Get_Val_vbe_ioctl_cmd(void);
void Scaler_wait_for_frc2fs_hw_lock_done(unsigned char display);
void scaler_disp_get_remove_multiple(unsigned int *pRemoveRatio, unsigned int *pMultipleRatio);
unsigned char drvif_factory_set_lvds_current(KADP_VBE_DISP_LVDS_T *lvds_info);
unsigned char drvif_factory_get_lvds_current(KADP_VBE_DISP_LVDS_T *lvds_info);
unsigned char drvif_factory_set_pre_emphasis_current(KADP_VBE_DISP_Emphasis_T *emp_info);
unsigned char drvif_factory_get_pre_emphasis_current(KADP_VBE_DISP_Emphasis_T *emp_info);
void Scaler_wait_for_LCM_done(void);
void HAL_DISP_PANEL_GetPanelInfo(KADP_DISP_PANEL_INFO_T *panelInfo);

#endif
