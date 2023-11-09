#ifndef VBE_EXPORT_H
#define VBE_EXPORT_H

#include <linux/types.h>
#include <rtd_types.h>
#include <ioctrl/scaler/vbe_cmd_id.h>
#include <ioctrl/scaler/vsc_cmd_id.h>
#include <scaler/scalerCommon.h>
//#include <linux/v4l2-ext/videodev2-ext.h>
#include <tvscalercontrol/panel/panel.h>
#include <tvscalercontrol/panel/panelapi.h>

#define FRAME_RATE_24HZ_LOW     (2350)
#define FRAME_RATE_24HZ_UP      (2450)

#define FRAME_RATE_25HZ_LOW     (2450)
#define FRAME_RATE_25HZ_UP      (2550)

#define FRAME_RATE_30HZ_LOW     (2950)
#define FRAME_RATE_30HZ_UP      (3050)

#define FRAME_RATE_48HZ_LOW     (4750)
#define FRAME_RATE_48HZ_UP      (4850)

#define FRAME_RATE_50HZ_LOW     (4950)
#define FRAME_RATE_50HZ_UP      (5050)

#define FRAME_RATE_60HZ_LOW     (5950)
#define FRAME_RATE_60HZ_UP      (6050)

#define FRAME_RATE_100HZ_LOW    (9950)
#define FRAME_RATE_100HZ_UP     (10050)

#define FRAME_RATE_120HZ_LOW    (11950)
#define FRAME_RATE_120HZ_UP     (12050)

#define FRAME_RATE_144HZ_LOW    (14350)
#define FRAME_RATE_144HZ_UP     (14450)

#define FRAME_RATE_165HZ_LOW    (16450)
#define FRAME_RATE_165HZ_UP     (16550)

#define FRAME_RATE_23_9HZ       (2390)
#define FRAME_RATE_23_97HZ      (2397)
#define FRAME_RATE_29_9HZ       (2990)
#define FRAME_RATE_29_97HZ      (2997)
#define FRAME_RATE_59_9HZ       (5990)
#define FRAME_RATE_59_94HZ      (5994)
#define FRAME_RATE_60HZ         (6000)
#define FRAME_RATE_24HZ         (2400)
#define FRAME_RATE_25HZ         (2500)
#define FRAME_RATE_50HZ         (5000)
#define FRAME_RATE_48HZ         (4800)
#define FRAME_RATE_30HZ         (3000)
#define FRAME_RATE_100HZ        (10000)
#define FRAME_RATE_120HZ        (12000)
#define FRAME_RATE_144HZ        (14400)
#define FRAME_RATE_165HZ        (16500)

#define VTOTAL_4K_60HZ          (2250)
#define HTOTAL_4K_60HZ          (4400)
#define VTOTAL_4K_50HZ          (2700)
#define VTOTAL_4K_48HZ          (2250)

#define DCLK_VALUE              (594 * 1000000)

#define VBE_ERROR               (1)
#define VBE_SUCCESS             (0)

#define REFRESH_RATE_120HZ      (120)
#define REFRESH_RATE_60HZ       (60)
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
unsigned int scaler_disp_smooth_get_memc_protect_vtotal_By_100(void);
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
void vbe_disp_set_vo_tracking_d_panel_type(unsigned char b_paneltype);
unsigned char vbe_disp_get_vo_tracking_d_panel_type(void);


void vbe_disp_set_sfg_dh_total(unsigned int u32_htotal);
unsigned int vbe_disp_get_sfg_dh_total(void);
void vbe_disp_set_sfg_pixel_mode(unsigned int u32_pixelMode);
unsigned int vbe_disp_get_sfg_pixel_mode(void);
void vbe_disp_set_vrr_average_den_update(unsigned int u32_update);
unsigned int vbe_disp_get_vrr_average_den_update(void);
void vbe_disp_set_vrr_average_den_mode(unsigned int u32_mode);
unsigned int vbe_disp_get_vrr_average_den_mode(void);

/*orbit related */
void vbe_disp_orbit_position_update(unsigned char tableIdx,unsigned char force_update);
unsigned int vbe_disp_orbit_get_max_table_index(void);
void vbe_disp_set_orbit(UINT8 bEnable, unsigned char mode);
unsigned char vbe_disp_get_orbit_mode(void);
unsigned char vbe_disp_get_orbit(void);
unsigned int vbe_disp_orbit_get_mode_table_shift_x(void);
unsigned int vbe_disp_orbit_get_mode_table_shift_y(void);
void vbe_disp_set_orbit_shift_info_sharememory(SCALER_ORBIT_SHIFT_INFO scalerOrbitInfo);
unsigned char vbe_disp_orbit_set_position_justscan(int x, int y);
void vbe_disp_orbit_set_position_overscan(int x, int y);
ORBIT_PIXEL_SHIFT_STRUCT Get_Orbit_Shift_Data(void);
void Set_Orbit_Shift_Data(ORBIT_PIXEL_SHIFT_STRUCT orbit_shift_data);
void vbe_disp_orbit_vo_overscan(void);
void vbe_disp_orbit_sub_overscan(void);
unsigned char vbe_disp_orbit_get_sub_overscan(void);
void vbe_disp_orbit_set_sub_overscan(unsigned char bEnable);
void vbe_disp_orbit_set_forceupdate_state(unsigned char bEnable);
unsigned char vbe_disp_orbit_get_forceupdate_state(void);
void vbe_disp_set_VRR_60Hz_average_data_enable_on(unsigned char bEnable);
unsigned char vbe_disp_orbit_get_vo_overscan(void);
void vbe_disp_orbit_set_vo_overscan(unsigned char bEnable);
void vbe_disp_orbit_set_vo_overscan_state(unsigned char bEnable);
void vbe_disp_orbit_frc_overscan(void);
unsigned char vbe_disp_orbit_get_frc_overscan(void);
void vbe_disp_orbit_set_frc_overscan(unsigned char bEnable);
unsigned char vbe_disp_orbit_get_osd_overscan(void);
void vbe_disp_orbit_set_osd_overscan(unsigned char bEnable);
void vbe_disp_orbit_OSD_overscan(void);
void vbe_disp_orbit_overscan_HW_Ctrl(int shitf_x, int shift_y, int range_x, int range_y, unsigned char bEnable_DB);
void vbe_disp_orbit_justscan_HW_Ctrl(int shitf_x, int shift_y, unsigned char justscan_position, unsigned char bEnable_DB);
void vbe_disp_set_orbit_mode(UINT8 mode);
ORBIT_PIXEL_SHIFT_STRUCT vbe_disp_calculate_orbit_position(unsigned int MaxPixelShift, unsigned int MaxLineShift, unsigned int index);
ORBIT_PIXEL_SHIFT_STRUCT vbe_disp_get_orbit_range(unsigned int index);


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
void HAL_VBE_DISP_SetPanelTiMode(UINT8 tiMode);
void HAL_VBE_DISP_GetPanelTiMode(UINT8* tiMode);
void HAL_VBE_DISP_SetPanelBitMode(UINT8 bitMode);
void HAL_VBE_DISP_GetPanelBitMode(UINT8* bitMode);
void HAL_VBE_DISP_SetPanelSFGDualMode(UINT8 segNum);
void HAL_VBE_DISP_GetPanelSFGDualMode(UINT8* segNum);

void vbe_disp_set_force_60hz_panel_type(unsigned char bEnable);
unsigned char vbe_disp_get_force_60hz_panel_type(void);
void vbe_disp_config_60hz_panel_interface(unsigned char bEnable);


void vbe_disp_VRR_framerate_protect_check(void);
unsigned char vbe_disp_check_support_VRR_framerate_range(void);
UINT32 vbe_disp_get_VRR_timing_real_vtotal(void);
VBE_DISP_VRR_FRACTIONAL_TIMING_MODE_T vbe_disp_get_VRR_timing_fractional_mode(void);
unsigned char vbe_disp_set_VRR_fractional_framerate_mode(void);
unsigned char vbe_disp_get_VRR_timingMode_flag(void);
unsigned char vbe_disp_get_VRR_ALLM_flag(void);
unsigned char vbe_disp_get_pre_VRR_ALLM_flag(void);
void vbe_disp_set_VRR_timingMode_flag(unsigned char flag);
void vbe_disp_set_pre_VRR_ALLM_flag (unsigned char enable);
unsigned char vbe_disp_get_freesync_mode_flag(void);
void vbe_disp_set_freesync_mode_flag (unsigned char enable);
unsigned int Get_Input_vfreq_by_vsync_cnt(void);
unsigned int Get_Input_vfreq_by_HDMI_ori_vfreq(void);
UINT32 vbe_disp_get_VRR_device_max_framerate(void);
void Scaler_wait_for_input_one_frame(unsigned char display);
unsigned int vbe_get_HDMI_run_datafs_condition(void);
void Scaler_wait_for_LCM_done(void);
unsigned char vbe_disp_decide_uzuclock_mapping_vo(void);
unsigned int vbe_get_VDEC4K_run_datafs_without_gatting_condition(void);
unsigned int vbe_get_VDEC4K_run_datafs_condition(void);
unsigned int vbe_get_HDMI_run_timing_framesync_condition(void);
unsigned int vbe_get_HDMI_run_datafs_condition(void);

extern int create_timer(unsigned char id, unsigned int interval, unsigned char mode);
extern int rtk_timer_control(unsigned char id, unsigned int cmd);

long vbe_ioctl(struct file *file, unsigned int cmd,  unsigned long arg);
unsigned char Get_Val_vbe_disp_global_status(void);
void Set_Val_vbe_disp_global_status(unsigned char value);
unsigned char Get_Val_VBE_Tridfmt_Global_Status(void);
void Set_Val_VBE_Tridfmt_Global_Status(unsigned char uc_value);
unsigned int Get_Val_vbe_ioctl_cmd(void);
void Set_Val_vbe_ioctl_cmd(unsigned int cmd_id);
UINT8 Get_Val_first_boot_run_main(void);
void Set_Val_first_boot_run_main(UINT8 set_value);
void Set_Val_VRR_timingMode_flag(unsigned char flag);
unsigned int scaler_disp_smooth_get_memc_protect_vtotal(PROTECT_VTOTAL_TYPE eType);



void scaler_disp_dtg_set_align_output_timing(unsigned char bEnable);
void vbe_disp_set_stop_memc_push_flag(unsigned char bOnOff);
unsigned char vbe_disp_get_stop_memc_push_flag(void);
unsigned int scaler_disp_dtg_get_time_borrow_value(void);
void scaler_disp_dtg_set_s1ip_dleay(unsigned char bEnable_DB);
unsigned int scaler_disp_dtg_get_time_borrow_value(void);
unsigned int scaler_disp_dtg_get_s1ip_delay_value(void);
void wait_dispd_stage1_smooth_toggle_apply_done(void);
void scaler_disp_dtg_set_db_latch_line(unsigned int db_latch_line);
void vbe_disp_sfg_inner_ptg(UINT8 bOnOff, VBE_DISP_INNER_PTG_TYPE_T type);
unsigned char drvif_factory_set_lvds_current(KADP_VBE_DISP_LVDS_T *lvds_info);
unsigned char drvif_factory_get_lvds_current(KADP_VBE_DISP_LVDS_T *lvds_info);
unsigned char drvif_factory_set_pre_emphasis_current(KADP_VBE_DISP_Emphasis_T *emp_info);
unsigned char drvif_factory_get_pre_emphasis_current(KADP_VBE_DISP_Emphasis_T *emp_info);
void vbe_set_disp_timing_change_status(bool stable);
unsigned char vbe_get_disp_timing_change_status(void);

bool get_dvs_framerate(int *framerate);
bool is_vrr_or_freesync_mode(void);
void HAL_DISP_PANEL_GetPanelInfo(KADP_DISP_PANEL_INFO_T *panelInfo);

#endif
