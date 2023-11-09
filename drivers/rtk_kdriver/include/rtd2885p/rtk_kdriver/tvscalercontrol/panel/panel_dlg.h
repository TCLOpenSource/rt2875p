#ifndef _PANEL_DLG_
#define _PANEL_DLG_

#define CONFIG_DYNAMIC_PANEL_SELECT 1

typedef enum
{
    PANEL_DLG_DEFAULT,
    PANEL_DLG_4k1k120,
    //PANEL_DLG_4k2k120,
    //PANEL_DLG_4k2k144,
    PANEL_DLG_4k1k240,
    PANEL_DLG_4k1k288,
}PANEL_DLG_MODE; // if update this enum, need notify this api get_dlg_mode_str

typedef enum
{
    PANEL_HSR_OFF,
    PANEL_HSR_4k1k120,
    PANEL_HSR_4k1k240,
    PANEL_HSR_4k1k288
}PANEL_HSR_MODE;

typedef enum dlg_callback_type
{
    E_DLG_INIT,
    E_DLG_FREE_RUN,
    E_DLG_DTG_FRACTION,
    E_DLG_NOTIFY_VCPU_UPGRAD_PANEL_PARAMETER,
    E_DLG_DISABLE_OSD,
    E_DLG_ENABLE_OSD,
    E_DLG_MEMC,
    E_DLG_ENABLE_DLG_MASK,
    E_DLG_DISABLE_DLG_MASK,
    E_DLG_SET_PRINT_DLG_MESSAGE_CNT,
}DLG_CALLBACK_TYPE;

#if 0
typedef enum dpll_type
{
    DPLL_594M,    //594M
    DPLL_686_88M, //686.88M
}DPLL_TYPE;
#endif

typedef void (*DLG_CALLBACK)(DLG_CALLBACK_TYPE eType);
void vbe_dynamic_set_dlgmode(PANEL_DLG_MODE dlgMode);
void HAL_VBE_Panel_Set_DlgEnable(unsigned char dlgEnable);
unsigned char HAL_VBE_Panel_Get_DlgEnable(void);
PANEL_DLG_MODE vbe_get_panel_dlg_mode(void);
PANEL_DLG_MODE vbe_get_timing_support_dlg(PANEL_HSR_MODE hsrMode, unsigned int customerIndex);
void panel_dynamic_select_setting(PANEL_DLG_MODE dlgMode);
unsigned char vbe_check_dlg_mode_change(void);
void handle_customer_dlg_setting(PANEL_DLG_MODE dlgMode, unsigned int customerIndex);
void vbe_set_panel_dlg_mode(PANEL_DLG_MODE panelDlgMode);
void vbe_set_cur_dlg_mode(PANEL_DLG_MODE dlgMode);
PANEL_DLG_MODE vbe_get_cur_dlg_mode(void);
void panel_vby1_4k1k240_tx(void);
void panel_vby1_4k2k120_tx(void);
PANEL_DLG_MODE get_customer_dlg_mode(PANEL_HSR_MODE hsrMode, unsigned int customerIndex);
void dlg_sfg_inner_ptg(unsigned char bOnOff);
unsigned int HAL_VBE_DISP_GetOutputFrameRateByMasterDtg(void);
void printf_dlg_msg(void);
void panel_vby1_dlg_4k2K60_4k2k120(PANEL_DLG_MODE dlgMode);
unsigned char vbe_get_dlg_support_all_timing(void);
void vbe_dlg_init(DLG_CALLBACK callback);
void vbe_set_print_dlg_message_cnt(unsigned int cnt);
unsigned char vbe_get_panel_vrr_freesync_timing_is_valid(void);

#endif
