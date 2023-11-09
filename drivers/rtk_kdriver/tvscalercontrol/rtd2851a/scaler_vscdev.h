#ifndef __VSC_H
#define  __VSC_H
#include <ioctrl/scaler/vsc_cmd_id.h>
#include <ioctrl/scaler/vfe_cmd_id.h>
#ifdef BUILD_QUICK_SHOW
#include <sysdefs.h>
#else
#include <base_types.h>
#endif
#include "tvscalercontrol/scalerdrv/scalerdrv.h"
#define ENABLE_SCALER_DUMP_FRAME_INFO //for bring up

//#define CONFIG_SUPPORT_SDR_MAX_RGB
//#define NEW_FLL_SW_MODE // for verify RL6641-929 FLL SW Mode
#ifdef ENABLE_SCALER_DUMP_FRAME_INFO
  /* dump VO CRC info to file */
  #ifndef FILE_NAME_SIZE
  #define FILE_NAME_SIZE 50
  #define DUMP_ES_SIZE 2*1024*1024
  #endif
  typedef struct SCALER_MALLOC_STRUCT {
  	unsigned long Memory;
  	unsigned long PhyAddr ;
  	unsigned long VirtAddr ;
  } SCALER_MALLOC_STRUCT;

  typedef struct SCALER_DUMP_STRUCT {
  	unsigned char file_name[FILE_NAME_SIZE];    /* debug log file place & name */
  	unsigned int  mem_size; 	/* debug memory size */
  	unsigned char enable ;
  } SCALER_DUMP_STRUCT;

  typedef struct SCALER_DUMP_BUFFER_HEADER {
  	unsigned int magic;
  	unsigned int size;
  	unsigned int rd;
  	unsigned int wr;
  } SCALER_DUMP_BUFFER_HEADER;
  // -------------------------------
#endif

struct vsc_film_mode_parameter_t {
	int enable;
	int type;
	int v_freq;
};

typedef enum{
	_low_power_tsk_no_pending = 0,//not low power mode
	_low_power_tsk_request_pending,//request pending
	_low_power_tsk_pending_finish,//pending finsiah
} _LOW_POWER_TASK_PENDING_STATUS;


typedef enum{
    TSK_NAME_VSC = 0,//vsc_scaler_tsk
    TSK_NAME_GAME_MODE,//new_game_mode_tsk
    TSK_NAME_GAME_MODE_UNMUTE,//game_mode_unmute_tsk
    TSK_NAME_MUTE_OFF,//video_muteoff_tsk
    TSK_NAME_FIX_LAST_LINE,//fixlast_line_tsk
    TSK_NAME_LOCAL_DIMMING,//localDimmingDemoCtrl_tsk
    TSK_NAME_VIDEO_DEBUG,//vsc_status_debug_tsk
    TSK_NAME_MAX_DUMMY,
} _SCALER_TSK_LIST;

//VSC_INPUT_TYPE_T Get_DisplayMode_Src(unsigned char display);
#if 0
unsigned char vo_overscan_disable(VSC_INPUT_TYPE_T srctype);
unsigned char vo_overscan_adjust(VSC_INPUT_TYPE_T srctype);
#endif
void modestate_setup_idomain(StructDisplayInfo* info);
unsigned char vdo_connect(unsigned char display, unsigned char vdec_port);
unsigned char vdo_disconnect(unsigned char display, unsigned char vdec_port);
unsigned char get_ForceSmoothtoggleGo(void);
unsigned char vsc_get_adaptivestream_flag(unsigned char display);
unsigned char Get_AVD_display(unsigned char display);
#ifdef BRING_UP_K4L_TEST
void fw_scalerip_set_di_gamemode_flag(unsigned char bflag);//20170524 pinyen create new_game_mode_tsk
#endif
bool rtk_hal_vsc_GetRGB444Mode(void);
void Scaler_start_orbit_algo(unsigned char b_enable);
void Scaler_reset_orbit(void);
unsigned char Scaler_get_orbit_algo_status(void);
void fw_set_vsc_GameMode(unsigned char b_vscGameMode_OnOff);
unsigned char fw_get_vsc_GameMode(void);

#ifdef SMOOTH_TOGGLE_DATAFRCFS_TEST
unsigned char vsc_decide_timingframesync(unsigned char display);
#endif
unsigned char Get_AVD_display(unsigned char display);
unsigned char Get_Factory_SelfDiagnosis_Mode(void);
void Set_Factory_SelfDiagnosis_Mode(unsigned char mode);
void Scaler_Dolby_HDR_reset(void);
unsigned char uc4_get_main_dispwin_value(unsigned short *x, unsigned short *y, unsigned short *w, unsigned short *h);//WOSQRTK-7731
void scaler_i2rnd_run_main(void);
void rtk_hal_vsc_i2rnd_enable(unsigned char enable);
unsigned short Scaler_CalAVD27MWidth(SCALER_DISP_CHANNEL display,unsigned short a_usInput);
unsigned char rtk_hal_vsc_i2rnd_b05_enable(unsigned char enable);
void set_frc_style_input_fast_than_display(bool enable);
unsigned char get_frc_style_input_fast_than_display(void);
void scaler_save_main_info(void);
void scaler_update_struct_info(unsigned char display);
unsigned char rtk_hal_vsc_set_localDimmingCtrlDemoMode(unsigned char bType, unsigned char bCtrl);
extern unsigned char (*get_new_game_mode_condition)(void);
void scaler_vsc_set_vr360_block_mdomain_doublebuffer(UINT8 bEnable);
UINT8 scaler_vsc_get_vr360_block_mdomain_doublebuffer(void);
void scaler_vsc_vr360_enter_datafs_proc(void);
void scaler_vsc_vr360_enter_datafrc_proc(void);
struct vsc_film_mode_parameter_t vsc_set_film_mode_param(int film_mode_enable, int film_mode_type, int v_freq);
void scaler_vsc_set_adaptive_pst_lowdelay_mode(UINT8 bOnOff);
UINT8 scaler_vsc_get_adaptive_pst_lowdelay_mode(void);
void scaler_vsc_set_force_pst_lowdelay_mode(UINT8 bOnOff);
UINT8 scaler_vsc_get_force_pst_lowdelay_mode(void);
void Scaler_ForceUpdate_Callback_Delay_Info_by_InputOutput_Region(KADP_VIDEO_RECT_T  OutputRegion);
void Scaler_Fulfill_Callback_Delay_Info(KADP_SCALER_WIN_CALLBACK_DELAY_INFO *pCallBack_info);
void Scaler_Reset_ForceUpdate_Callback_Delay_Info(void);
struct vsc_film_mode_parameter_t get_film_mode_parameter(void);
struct semaphore* get_gamemode_check_semaphore(void);
void game_mode_set_ignore_cmd_check(unsigned char enable);
unsigned char Get_Val_game_mode_ignore_cmd_check(void);
unsigned char Scaler_get_vdec_2k120hz(void);
unsigned char get_new_game_mode_vdec_memc_bypass_condition(void);
void scaler_I2D_pwm_frequency_update(void);
unsigned char scaler_get_I2D_pwm_frequency_update_in_isr(void);
unsigned char scaler_get_I2D_tracking_in_realcinema(void);
void Convert_Timing_To_fs_Depend_datafs(unsigned char display);
void scaler_overscan_vertical_size(unsigned char display,unsigned int progressive,unsigned int source_height,unsigned int *height,unsigned int *v_start);

void scaler_set_VRR_pwm_frequency_update_in_isr(unsigned char bEnable);
unsigned char scaler_get_VRR_pwm_frequency_update_in_isr(void);
void scaler_pwm_frequency_update_by_dvs(void);
unsigned judge_scaler_break_case(unsigned char display);
unsigned int scaler_get_I2D_tracking_phase_error_threadhold(void);
unsigned int scaler_get_idomain_vfreq_pre(void);
unsigned int rtk_scaler_get_lowdelay_value(void);
unsigned int rtk_scaler_calc_current_video_path_latency(void);
void scaler_Setdualdecoder_notchange(unsigned char flag);
void  rtk_hal_vsc_Setdualdecoder_run(unsigned char flag);
unsigned char rtk_hal_vsc_Getdualdecoder_run(void);
unsigned int rtk_hal_vsc_scaler_check_video_delay(VIDEO_WID_T wid, unsigned int *current_video_delay);
unsigned char rtk_hal_vsc_SetInputRegion(KADP_VIDEO_WID_T wid, KADP_VIDEO_RECT_T  inregion);
unsigned char rtk_hal_vsc_SetOutputRegion(VIDEO_WID_T wid, KADP_VIDEO_RECT_T outregion, unsigned short Wide, unsigned short High);
unsigned char rtk_hal_vsc_GetInputRegion(KADP_VIDEO_WID_T wid, KADP_VIDEO_RECT_T * pinregion);
unsigned char rtk_hal_vsc_GetOutputRegion(KADP_VIDEO_WID_T wid, KADP_VIDEO_RECT_T * poutregion);
unsigned char rtk_vsc_setFilmMode(bool value);
unsigned char rtk_hal_vsc_Getdualdecoder(void);
void scaler_set_full_gatting_rpc(unsigned int enable);
unsigned char rtk_hal_vsc_SetInputRegion_OutputRegion(KADP_VIDEO_WID_T wid, KADP_VSC_ROTATE_T rotate_type, KADP_VIDEO_RECT_T  inregion,
	KADP_VIDEO_RECT_T originalInput, KADP_VIDEO_RECT_T outregion, unsigned char null_input, unsigned char null_output);
void set_force_i3ddma_enable(UINT8 enable);
void set_force_hdmi_hdr_flow_enable(UINT8 enable);
unsigned char rtk_hal_vsc_SetWinBlank(VIDEO_WID_T wid, unsigned char bonoff, KADP_VIDEO_DDI_WIN_COLOR_T color);
void modestate_decide_rtnr(void);
void trigger_mute_off(unsigned char display, unsigned char no_delay);
void Set_vsc_input_src_info(unsigned char display,KADP_VSC_OUTPUT_MODE_T vsc_output_mode, KADP_VSC_INPUT_SRC_INFO_T source_info);
extern void (*decide_rotate_flag)(unsigned char);
extern void (*reset_rotate_mode)(unsigned char);
extern unsigned char (*Scaler_get_data_framesync)(unsigned char display);
extern unsigned char (*get_displayinfo_proc)(unsigned char, VSC_INPUT_TYPE_T);
extern unsigned char (*Setup_IMD)(StructDisplayInfo*);
extern unsigned char (*scaler_decide_smoothtoggle_flow)(unsigned char display, KADP_VSC_INPUT_TYPE_T inputType);
void run_scaler_fail_handle(unsigned char display, KADP_VSC_INPUT_TYPE_T inputType);
unsigned char wait_vo_avsync_filmmode_done(unsigned char display);
unsigned char scaler_check_smoothtoggle_again(unsigned char display, KADP_VSC_INPUT_TYPE_T inputType);
void scaler_run_smoothtoggle_proc(unsigned char display, KADP_VSC_INPUT_TYPE_T inputType);
void scaler_set_online_measure(unsigned char display, KADP_VSC_INPUT_TYPE_T inputType);
unsigned char Get_Val_new_voinfo_flag(void);
void Set_Val_new_voinfo_flag(unsigned char enable);
unsigned char Get_Val_wait_avsync_do_flag(void);
void Set_Val_wait_avsync_do_flag(unsigned char enable);
extern unsigned char (*wait_atv_stable_for_bg)(unsigned char);
void set_current_scaler_source_num(unsigned char display);
void increase_source_connect_verify_num(unsigned char display);
unsigned char rtk_hal_vsc_initialize(void);
unsigned char rtk_hal_vsc_uninitialize(void);
unsigned char rtk_hal_vsc_open(VIDEO_WID_T wid);
unsigned char rtk_hal_vsc_close(VIDEO_WID_T wid);
unsigned char rtk_hal_vsc_Connect(VIDEO_WID_T wid, KADP_VSC_INPUT_SRC_INFO_T inputSrcInfo, KADP_VSC_OUTPUT_MODE_T outputMode);
unsigned char rtk_hal_vsc_Disconnect(VIDEO_WID_T wid, KADP_VSC_INPUT_SRC_INFO_T inputSrcInfo, KADP_VSC_OUTPUT_MODE_T outputMode);
StructSrcRect get_main_dispwin(void);
StructSrcRect Get_Val_main_dispwin_pre(void);
void Set_Val_main_dispwin_pre(StructSrcRect m_disp);
void Set_Val_OutpuLocalDimmingPatMode(unsigned char value);
void update_force_rerun_scaler_condition(unsigned char srctype, unsigned char display);
void Set_Val_sync_memc_ready(unsigned char value);
extern unsigned int (*Get_Val_smooth_toggle_game_mode_timeout_trigger_flag)(void);
void Set_Val_smooth_toggle_game_mode_timeout_trigger_flag(unsigned int flag);
extern void (*wait_memc_ready)(void);
extern long (*Scaler_Send_VoDisconnect_VOMDA)(KADP_VO_VSC_SRC_TYPE_T type, unsigned char wid_port);
unsigned char scaler_Getdualdecoder_notchange(void);
unsigned char Get_Val_OutpuLocalDimmingPatMode(void);
void Set_Val_OutpuLocalDimmingPatMode(unsigned char mode);
DIRECT_VO_FRAME_ORIENTATION Get_Val_ap_roate_mode(void);
void Set_Val_ap_rotate_mode(DIRECT_VO_FRAME_ORIENTATION mode);
void set_shiftxy_main_inputpre(StructSrcRect input_size );
void set_shiftxy_main_dispwinpre(StructSrcRect out_size );
void Set_Val_cur_main_vsc_src_num(unsigned char val);
void Set_Val_cur_sub_vsc_src_num(unsigned char val);
void Set_Val_source_connect_verify_num(unsigned char dispaly, unsigned char val);
void Set_Val_rotatedisconnecten(unsigned int value);
unsigned int Get_Val_rotatedisconnecten(void);
unsigned char rtk_output_connect(unsigned char display);
unsigned char Get_Val_vdo0_connect(void);
unsigned char Get_Val_vdo1_connect(void);
void Set_Val_vdo0_connect(unsigned char);
void Set_Val_vdo1_connect(unsigned char);
#ifdef CONFIG_DUAL_CHANNEL
StructSrcRect Get_Val_sub_dispwin(void);
void Set_Val_sub_dispwin_pre(StructSrcRect m_disp);
void Set_Val_sub_input_pre(StructSrcRect m_disp);
StructSrcRect Get_Val_sub_input_size(void);
void Set_Val_sub_input_size(StructSrcRect m_disp);
#endif
UINT8 get_read_buffer_first_flag(unsigned char display);
void set_read_buffer_first_flag(unsigned char display, UINT8 flag);
unsigned char scaler_get_I2D_previous_frameIdx(void);
void scaler_set_I2D_previous_frameIdx(unsigned char frameIdx);
void Set_Val_checkStableCnt(UINT8 stableCnt);
UINT8 Get_Val_checkStableCnt(void);
unsigned char Get_Val_scaler_DumpCRC_Config_enable(void);
void Set_Val_scaler_DumpCRC_Config_enable(unsigned char enable);
void scaler_set_I2D_tracking_in_slow_mode(unsigned char bEnable);
unsigned char scaler_get_I2D_tracking_in_slow_mode(void);
void scaler_set_I2D_tracking_in_realcinema(unsigned char bEnable);
unsigned char scaler_get_I2D_tracking_in_realcinema(void);
extern unsigned char (*get_force_i3ddma_enable)(unsigned char);
extern unsigned char (*rtk_run_scaler)(unsigned char display, KADP_VSC_INPUT_TYPE_T inputType, KADP_VSC_OUTPUT_MODE_T outputMode);
extern unsigned char (*init_source_proc)(unsigned char display, VSC_INPUT_TYPE_T sourcetype, unsigned char sourceport);
unsigned int gamemode_unmute_datafs_wait_golden_vsync_done(unsigned int timeout_count);
unsigned int Get_Val_smooth_toggle_game_mode_check_cnt(void);
void Set_Val_smooth_toggle_game_mode_check_cnt(unsigned int value);
unsigned int Get_Val_st_game_mode_cmd_pre(void);
void Set_Val_st_game_mode_cmd_pre(unsigned int value);
unsigned int Get_Val_game_mode_cmd_check(void);
void Set_Val_game_mode_cmd_check(unsigned int value);
KADP_VSC_HDR_TYPE_T Get_Val_main_vsc_hdrtype(void);
bool Get_Val_VSCModuleInitDone(void);
void rtk_hal_QS_vscInit(void);
unsigned char Get_Val_bQsmode(void);
void Set_Val_bQsmode(unsigned char val);
extern void (* QS_VSCInit_SyncConnectInfo)(void);
extern void (*Scaler_SetFreeze)(unsigned char channel, unsigned char mode);
void game_mode_unmute_trigger_mute_off(void);
extern void (*reset_hdr_mode)(unsigned char display);
extern void (*setup_imd_hdr)(unsigned int hdrtype);
extern void (*p_Set_Val_Tsk_Status)(unsigned char, unsigned char);
extern unsigned char (*p_Get_Val_Tsk_Status)(unsigned char);
extern unsigned char (*Get_game_mode_dynamic_flag)(void);
bool IsPcModeTiming(vfe_hdmi_timing_info_t *info, vfe_hdmi_avi_t* aviInfo);
unsigned char scaler_borrowmemory_from_i3ddma_for_mdomain(void);
unsigned char Get_Val_qs_game_flag(void);
void check_to_run_rtk_video_pm_rumtime_resume_driver(unsigned char CallByDevice);
#endif
