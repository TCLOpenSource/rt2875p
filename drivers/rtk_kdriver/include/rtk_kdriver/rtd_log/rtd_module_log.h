#include <rtd_log/rtd_module_tag.h>
#include <rtd_log/rtd_os_print.h>



/*----------------------------------------
 * print api for AVD
 *---------------------------------------*/
#define rtd_pr_avd_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_AVD, fmt, ##__VA_ARGS__)
#define rtd_pr_avd_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_AVD, fmt, ##__VA_ARGS__)
#define rtd_pr_avd_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_AVD, fmt, ##__VA_ARGS__)
#define rtd_pr_avd_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_AVD, fmt, ##__VA_ARGS__)
#define rtd_pr_avd_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_AVD, fmt, ##__VA_ARGS__)
#define rtd_pr_avd_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_AVD, fmt, ##__VA_ARGS__)
#define rtd_pr_avd_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_AVD, fmt, ##__VA_ARGS__)
#define rtd_pr_avd_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_AVD, fmt, ##__VA_ARGS__)
#define rtd_pr_avd_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_AVD, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for ADC
 *---------------------------------------*/
#define rtd_pr_adc_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_ADC, fmt, ##__VA_ARGS__)
#define rtd_pr_adc_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_ADC, fmt, ##__VA_ARGS__)
#define rtd_pr_adc_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_ADC, fmt, ##__VA_ARGS__)
#define rtd_pr_adc_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_ADC, fmt, ##__VA_ARGS__)
#define rtd_pr_adc_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_ADC, fmt, ##__VA_ARGS__)
#define rtd_pr_adc_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_ADC, fmt, ##__VA_ARGS__)
#define rtd_pr_adc_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_ADC, fmt, ##__VA_ARGS__)
#define rtd_pr_adc_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_ADC, fmt, ##__VA_ARGS__)
#define rtd_pr_adc_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_ADC, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for VDC
 *---------------------------------------*/
#define rtd_pr_vdc_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_VDC, fmt, ##__VA_ARGS__)
#define rtd_pr_vdc_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_VDC, fmt, ##__VA_ARGS__)
#define rtd_pr_vdc_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_VDC, fmt, ##__VA_ARGS__)
#define rtd_pr_vdc_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_VDC, fmt, ##__VA_ARGS__)
#define rtd_pr_vdc_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_VDC, fmt, ##__VA_ARGS__)
#define rtd_pr_vdc_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_VDC, fmt, ##__VA_ARGS__)
#define rtd_pr_vdc_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_VDC, fmt, ##__VA_ARGS__)
#define rtd_pr_vdc_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_VDC, fmt, ##__VA_ARGS__)
#define rtd_pr_vdc_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_VDC, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for VBI
 *---------------------------------------*/
#define rtd_pr_vbi_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_VBI, fmt, ##__VA_ARGS__)
#define rtd_pr_vbi_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_VBI, fmt, ##__VA_ARGS__)
#define rtd_pr_vbi_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_VBI, fmt, ##__VA_ARGS__)
#define rtd_pr_vbi_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_VBI, fmt, ##__VA_ARGS__)
#define rtd_pr_vbi_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_VBI, fmt, ##__VA_ARGS__)
#define rtd_pr_vbi_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_VBI, fmt, ##__VA_ARGS__)
#define rtd_pr_vbi_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_VBI, fmt, ##__VA_ARGS__)
#define rtd_pr_vbi_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_VBI, fmt, ##__VA_ARGS__)
#define rtd_pr_vbi_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_VBI, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for ONMS
 *---------------------------------------*/
#define rtd_pr_onms_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_ONMS, fmt, ##__VA_ARGS__)
#define rtd_pr_onms_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_ONMS, fmt, ##__VA_ARGS__)
#define rtd_pr_onms_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_ONMS, fmt, ##__VA_ARGS__)
#define rtd_pr_onms_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_ONMS, fmt, ##__VA_ARGS__)
#define rtd_pr_onms_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_ONMS, fmt, ##__VA_ARGS__)
#define rtd_pr_onms_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_ONMS, fmt, ##__VA_ARGS__)
#define rtd_pr_onms_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_ONMS, fmt, ##__VA_ARGS__)
#define rtd_pr_onms_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_ONMS, fmt, ##__VA_ARGS__)
#define rtd_pr_onms_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_ONMS, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for OFFMS
 *---------------------------------------*/
#define rtd_pr_offms_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_OFFMS, fmt, ##__VA_ARGS__)
#define rtd_pr_offms_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_OFFMS, fmt, ##__VA_ARGS__)
#define rtd_pr_offms_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_OFFMS, fmt, ##__VA_ARGS__)
#define rtd_pr_offms_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_OFFMS, fmt, ##__VA_ARGS__)
#define rtd_pr_offms_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_OFFMS, fmt, ##__VA_ARGS__)
#define rtd_pr_offms_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_OFFMS, fmt, ##__VA_ARGS__)
#define rtd_pr_offms_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_OFFMS, fmt, ##__VA_ARGS__)
#define rtd_pr_offms_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_OFFMS, fmt, ##__VA_ARGS__)
#define rtd_pr_offms_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_OFFMS, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for MEMC
 *---------------------------------------*/
#define rtd_pr_memc_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_MEMC, fmt, ##__VA_ARGS__)
#define rtd_pr_memc_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_MEMC, fmt, ##__VA_ARGS__)
#define rtd_pr_memc_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_MEMC, fmt, ##__VA_ARGS__)
#define rtd_pr_memc_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_MEMC, fmt, ##__VA_ARGS__)
#define rtd_pr_memc_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_MEMC, fmt, ##__VA_ARGS__)
#define rtd_pr_memc_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_MEMC, fmt, ##__VA_ARGS__)
#define rtd_pr_memc_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_MEMC, fmt, ##__VA_ARGS__)
#define rtd_pr_memc_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_MEMC, fmt, ##__VA_ARGS__)
#define rtd_pr_memc_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_MEMC, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for MEMC_LIB
 *---------------------------------------*/
#define rtd_pr_memc_lib_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_MEMC_LIB, fmt, ##__VA_ARGS__)
#define rtd_pr_memc_lib_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_MEMC_LIB, fmt, ##__VA_ARGS__)
#define rtd_pr_memc_lib_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_MEMC_LIB, fmt, ##__VA_ARGS__)
#define rtd_pr_memc_lib_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_MEMC_LIB, fmt, ##__VA_ARGS__)
#define rtd_pr_memc_lib_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_MEMC_LIB, fmt, ##__VA_ARGS__)
#define rtd_pr_memc_lib_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_MEMC_LIB, fmt, ##__VA_ARGS__)
#define rtd_pr_memc_lib_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_MEMC_LIB, fmt, ##__VA_ARGS__)
#define rtd_pr_memc_lib_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_MEMC_LIB, fmt, ##__VA_ARGS__)
#define rtd_pr_memc_lib_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_MEMC_LIB, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for NEW_GAME_MODE
 *---------------------------------------*/
#define rtd_pr_new_game_mode_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_NEW_GAME_MODE, fmt, ##__VA_ARGS__)
#define rtd_pr_new_game_mode_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_NEW_GAME_MODE, fmt, ##__VA_ARGS__)
#define rtd_pr_new_game_mode_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_NEW_GAME_MODE, fmt, ##__VA_ARGS__)
#define rtd_pr_new_game_mode_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_NEW_GAME_MODE, fmt, ##__VA_ARGS__)
#define rtd_pr_new_game_mode_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_NEW_GAME_MODE, fmt, ##__VA_ARGS__)
#define rtd_pr_new_game_mode_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_NEW_GAME_MODE, fmt, ##__VA_ARGS__)
#define rtd_pr_new_game_mode_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_NEW_GAME_MODE, fmt, ##__VA_ARGS__)
#define rtd_pr_new_game_mode_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_NEW_GAME_MODE, fmt, ##__VA_ARGS__)
#define rtd_pr_new_game_mode_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_NEW_GAME_MODE, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for SCALER_MEMORY
 *---------------------------------------*/
#define rtd_pr_scaler_memory_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_SCALER_MEMORY, fmt, ##__VA_ARGS__)
#define rtd_pr_scaler_memory_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_SCALER_MEMORY, fmt, ##__VA_ARGS__)
#define rtd_pr_scaler_memory_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_SCALER_MEMORY, fmt, ##__VA_ARGS__)
#define rtd_pr_scaler_memory_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_SCALER_MEMORY, fmt, ##__VA_ARGS__)
#define rtd_pr_scaler_memory_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_SCALER_MEMORY, fmt, ##__VA_ARGS__)
#define rtd_pr_scaler_memory_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_SCALER_MEMORY, fmt, ##__VA_ARGS__)
#define rtd_pr_scaler_memory_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_SCALER_MEMORY, fmt, ##__VA_ARGS__)
#define rtd_pr_scaler_memory_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_SCALER_MEMORY, fmt, ##__VA_ARGS__)
#define rtd_pr_scaler_memory_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_SCALER_MEMORY, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for VBE
 *---------------------------------------*/
#define rtd_pr_vbe_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_VBE, fmt, ##__VA_ARGS__)
#define rtd_pr_vbe_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_VBE, fmt, ##__VA_ARGS__)
#define rtd_pr_vbe_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_VBE, fmt, ##__VA_ARGS__)
#define rtd_pr_vbe_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_VBE, fmt, ##__VA_ARGS__)
#define rtd_pr_vbe_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_VBE, fmt, ##__VA_ARGS__)
#define rtd_pr_vbe_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_VBE, fmt, ##__VA_ARGS__)
#define rtd_pr_vbe_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_VBE, fmt, ##__VA_ARGS__)
#define rtd_pr_vbe_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_VBE, fmt, ##__VA_ARGS__)
#define rtd_pr_vbe_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_VBE, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for VPQ
 *---------------------------------------*/
#define rtd_pr_vpq_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_VPQ, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_VPQ, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_VPQ, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_VPQ, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_VPQ, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_VPQ, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_VPQ, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_VPQ, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_VPQ, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for VPQ_GSR
 *---------------------------------------*/
#define rtd_pr_vpq_gsr_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_VPQ_GSR, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_gsr_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_VPQ_GSR, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_gsr_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_VPQ_GSR, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_gsr_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_VPQ_GSR, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_gsr_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_VPQ_GSR, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_gsr_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_VPQ_GSR, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_gsr_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_VPQ_GSR, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_gsr_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_VPQ_GSR, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_gsr_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_VPQ_GSR, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for HDMITX
 *---------------------------------------*/
#define rtd_pr_hdmitx_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_HDMITX, fmt, ##__VA_ARGS__)
#define rtd_pr_hdmitx_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_HDMITX, fmt, ##__VA_ARGS__)
#define rtd_pr_hdmitx_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_HDMITX, fmt, ##__VA_ARGS__)
#define rtd_pr_hdmitx_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_HDMITX, fmt, ##__VA_ARGS__)
#define rtd_pr_hdmitx_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_HDMITX, fmt, ##__VA_ARGS__)
#define rtd_pr_hdmitx_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_HDMITX, fmt, ##__VA_ARGS__)
#define rtd_pr_hdmitx_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_HDMITX, fmt, ##__VA_ARGS__)
#define rtd_pr_hdmitx_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_HDMITX, fmt, ##__VA_ARGS__)
#define rtd_pr_hdmitx_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_HDMITX, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for VFE
 *---------------------------------------*/
#define rtd_pr_vfe_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_VFE, fmt, ##__VA_ARGS__)
#define rtd_pr_vfe_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_VFE, fmt, ##__VA_ARGS__)
#define rtd_pr_vfe_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_VFE, fmt, ##__VA_ARGS__)
#define rtd_pr_vfe_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_VFE, fmt, ##__VA_ARGS__)
#define rtd_pr_vfe_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_VFE, fmt, ##__VA_ARGS__)
#define rtd_pr_vfe_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_VFE, fmt, ##__VA_ARGS__)
#define rtd_pr_vfe_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_VFE, fmt, ##__VA_ARGS__)
#define rtd_pr_vfe_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_VFE, fmt, ##__VA_ARGS__)
#define rtd_pr_vfe_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_VFE, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for VSC
 *---------------------------------------*/
#define rtd_pr_vsc_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_VSC, fmt, ##__VA_ARGS__)
#define rtd_pr_vsc_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_VSC, fmt, ##__VA_ARGS__)
#define rtd_pr_vsc_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_VSC, fmt, ##__VA_ARGS__)
#define rtd_pr_vsc_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_VSC, fmt, ##__VA_ARGS__)
#define rtd_pr_vsc_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_VSC, fmt, ##__VA_ARGS__)
#define rtd_pr_vsc_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_VSC, fmt, ##__VA_ARGS__)
#define rtd_pr_vsc_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_VSC, fmt, ##__VA_ARGS__)
#define rtd_pr_vsc_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_VSC, fmt, ##__VA_ARGS__)
#define rtd_pr_vsc_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_VSC, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for I3DDMA
 *---------------------------------------*/
#define rtd_pr_i3ddma_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_I3DDMA, fmt, ##__VA_ARGS__)
#define rtd_pr_i3ddma_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_I3DDMA, fmt, ##__VA_ARGS__)
#define rtd_pr_i3ddma_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_I3DDMA, fmt, ##__VA_ARGS__)
#define rtd_pr_i3ddma_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_I3DDMA, fmt, ##__VA_ARGS__)
#define rtd_pr_i3ddma_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_I3DDMA, fmt, ##__VA_ARGS__)
#define rtd_pr_i3ddma_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_I3DDMA, fmt, ##__VA_ARGS__)
#define rtd_pr_i3ddma_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_I3DDMA, fmt, ##__VA_ARGS__)
#define rtd_pr_i3ddma_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_I3DDMA, fmt, ##__VA_ARGS__)
#define rtd_pr_i3ddma_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_I3DDMA, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for IOREG
 *---------------------------------------*/
#define rtd_pr_ioreg_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_IOREG, fmt, ##__VA_ARGS__)
#define rtd_pr_ioreg_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_IOREG, fmt, ##__VA_ARGS__)
#define rtd_pr_ioreg_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_IOREG, fmt, ##__VA_ARGS__)
#define rtd_pr_ioreg_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_IOREG, fmt, ##__VA_ARGS__)
#define rtd_pr_ioreg_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_IOREG, fmt, ##__VA_ARGS__)
#define rtd_pr_ioreg_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_IOREG, fmt, ##__VA_ARGS__)
#define rtd_pr_ioreg_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_IOREG, fmt, ##__VA_ARGS__)
#define rtd_pr_ioreg_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_IOREG, fmt, ##__VA_ARGS__)
#define rtd_pr_ioreg_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_IOREG, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for SMT
 *---------------------------------------*/
#define rtd_pr_smt_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_SMT, fmt, ##__VA_ARGS__)
#define rtd_pr_smt_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_SMT, fmt, ##__VA_ARGS__)
#define rtd_pr_smt_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_SMT, fmt, ##__VA_ARGS__)
#define rtd_pr_smt_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_SMT, fmt, ##__VA_ARGS__)
#define rtd_pr_smt_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_SMT, fmt, ##__VA_ARGS__)
#define rtd_pr_smt_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_SMT, fmt, ##__VA_ARGS__)
#define rtd_pr_smt_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_SMT, fmt, ##__VA_ARGS__)
#define rtd_pr_smt_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_SMT, fmt, ##__VA_ARGS__)
#define rtd_pr_smt_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_SMT, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for HDR
 *---------------------------------------*/
#define rtd_pr_hdr_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_HDR, fmt, ##__VA_ARGS__)
#define rtd_pr_hdr_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_HDR, fmt, ##__VA_ARGS__)
#define rtd_pr_hdr_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_HDR, fmt, ##__VA_ARGS__)
#define rtd_pr_hdr_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_HDR, fmt, ##__VA_ARGS__)
#define rtd_pr_hdr_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_HDR, fmt, ##__VA_ARGS__)
#define rtd_pr_hdr_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_HDR, fmt, ##__VA_ARGS__)
#define rtd_pr_hdr_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_HDR, fmt, ##__VA_ARGS__)
#define rtd_pr_hdr_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_HDR, fmt, ##__VA_ARGS__)
#define rtd_pr_hdr_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_HDR, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for VR
 *---------------------------------------*/
#define rtd_pr_vr_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_VR, fmt, ##__VA_ARGS__)
#define rtd_pr_vr_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_VR, fmt, ##__VA_ARGS__)
#define rtd_pr_vr_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_VR, fmt, ##__VA_ARGS__)
#define rtd_pr_vr_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_VR, fmt, ##__VA_ARGS__)
#define rtd_pr_vr_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_VR, fmt, ##__VA_ARGS__)
#define rtd_pr_vr_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_VR, fmt, ##__VA_ARGS__)
#define rtd_pr_vr_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_VR, fmt, ##__VA_ARGS__)
#define rtd_pr_vr_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_VR, fmt, ##__VA_ARGS__)
#define rtd_pr_vr_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_VR, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for VT
 *---------------------------------------*/
#define rtd_pr_vt_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_VT, fmt, ##__VA_ARGS__)
#define rtd_pr_vt_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_VT, fmt, ##__VA_ARGS__)
#define rtd_pr_vt_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_VT, fmt, ##__VA_ARGS__)
#define rtd_pr_vt_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_VT, fmt, ##__VA_ARGS__)
#define rtd_pr_vt_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_VT, fmt, ##__VA_ARGS__)
#define rtd_pr_vt_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_VT, fmt, ##__VA_ARGS__)
#define rtd_pr_vt_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_VT, fmt, ##__VA_ARGS__)
#define rtd_pr_vt_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_VT, fmt, ##__VA_ARGS__)
#define rtd_pr_vt_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_VT, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for VO
 *---------------------------------------*/
#define rtd_pr_vo_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_VO, fmt, ##__VA_ARGS__)
#define rtd_pr_vo_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_VO, fmt, ##__VA_ARGS__)
#define rtd_pr_vo_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_VO, fmt, ##__VA_ARGS__)
#define rtd_pr_vo_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_VO, fmt, ##__VA_ARGS__)
#define rtd_pr_vo_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_VO, fmt, ##__VA_ARGS__)
#define rtd_pr_vo_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_VO, fmt, ##__VA_ARGS__)
#define rtd_pr_vo_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_VO, fmt, ##__VA_ARGS__)
#define rtd_pr_vo_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_VO, fmt, ##__VA_ARGS__)
#define rtd_pr_vo_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_VO, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for VGIP_ISR
 *---------------------------------------*/
#define rtd_pr_vgip_isr_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_VGIP_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_vgip_isr_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_VGIP_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_vgip_isr_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_VGIP_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_vgip_isr_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_VGIP_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_vgip_isr_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_VGIP_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_vgip_isr_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_VGIP_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_vgip_isr_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_VGIP_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_vgip_isr_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_VGIP_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_vgip_isr_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_VGIP_ISR, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for DDOMAIN_ISR
 *---------------------------------------*/
#define rtd_pr_ddomain_isr_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_DDOMAIN_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_ddomain_isr_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_DDOMAIN_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_ddomain_isr_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_DDOMAIN_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_ddomain_isr_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_DDOMAIN_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_ddomain_isr_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_DDOMAIN_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_ddomain_isr_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_DDOMAIN_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_ddomain_isr_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_DDOMAIN_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_ddomain_isr_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_DDOMAIN_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_ddomain_isr_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_DDOMAIN_ISR, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for PIF_ISR
 *---------------------------------------*/
#define rtd_pr_pif_isr_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_PIF_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_pif_isr_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_PIF_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_pif_isr_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_PIF_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_pif_isr_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_PIF_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_pif_isr_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_PIF_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_pif_isr_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_PIF_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_pif_isr_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_PIF_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_pif_isr_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_PIF_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_pif_isr_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_PIF_ISR, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for HIST
 *---------------------------------------*/
#define rtd_pr_hist_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_HIST, fmt, ##__VA_ARGS__)
#define rtd_pr_hist_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_HIST, fmt, ##__VA_ARGS__)
#define rtd_pr_hist_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_HIST, fmt, ##__VA_ARGS__)
#define rtd_pr_hist_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_HIST, fmt, ##__VA_ARGS__)
#define rtd_pr_hist_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_HIST, fmt, ##__VA_ARGS__)
#define rtd_pr_hist_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_HIST, fmt, ##__VA_ARGS__)
#define rtd_pr_hist_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_HIST, fmt, ##__VA_ARGS__)
#define rtd_pr_hist_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_HIST, fmt, ##__VA_ARGS__)
#define rtd_pr_hist_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_HIST, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for VPQ_AI
 *---------------------------------------*/
#define rtd_pr_vpq_ai_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_VPQ_AI, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_ai_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_VPQ_AI, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_ai_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_VPQ_AI, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_ai_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_VPQ_AI, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_ai_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_VPQ_AI, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_ai_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_VPQ_AI, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_ai_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_VPQ_AI, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_ai_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_VPQ_AI, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_ai_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_VPQ_AI, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for VPQEX
 *---------------------------------------*/
#define rtd_pr_vpqex_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_VPQEX, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqex_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_VPQEX, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqex_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_VPQEX, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqex_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_VPQEX, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqex_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_VPQEX, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqex_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_VPQEX, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqex_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_VPQEX, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqex_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_VPQEX, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqex_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_VPQEX, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for VPQUT
 *---------------------------------------*/
#define rtd_pr_vpqut_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_VPQUT, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqut_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_VPQUT, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqut_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_VPQUT, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqut_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_VPQUT, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqut_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_VPQUT, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqut_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_VPQUT, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqut_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_VPQUT, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqut_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_VPQUT, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqut_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_VPQUT, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for VPQMASK
 *---------------------------------------*/
#define rtd_pr_vpqmask_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_VPQMASK, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqmask_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_VPQMASK, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqmask_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_VPQMASK, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqmask_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_VPQMASK, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqmask_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_VPQMASK, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqmask_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_VPQMASK, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqmask_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_VPQMASK, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqmask_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_VPQMASK, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqmask_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_VPQMASK, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for VIP_RINGBUF
 *---------------------------------------*/
#define rtd_pr_vip_ringbuf_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_VIP_RINGBUF, fmt, ##__VA_ARGS__)
#define rtd_pr_vip_ringbuf_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_VIP_RINGBUF, fmt, ##__VA_ARGS__)
#define rtd_pr_vip_ringbuf_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_VIP_RINGBUF, fmt, ##__VA_ARGS__)
#define rtd_pr_vip_ringbuf_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_VIP_RINGBUF, fmt, ##__VA_ARGS__)
#define rtd_pr_vip_ringbuf_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_VIP_RINGBUF, fmt, ##__VA_ARGS__)
#define rtd_pr_vip_ringbuf_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_VIP_RINGBUF, fmt, ##__VA_ARGS__)
#define rtd_pr_vip_ringbuf_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_VIP_RINGBUF, fmt, ##__VA_ARGS__)
#define rtd_pr_vip_ringbuf_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_VIP_RINGBUF, fmt, ##__VA_ARGS__)
#define rtd_pr_vip_ringbuf_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_VIP_RINGBUF, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for VPQMASK_COLOR
 *---------------------------------------*/
#define rtd_pr_vpqmask_color_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_VPQMASK_COLOR, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqmask_color_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_VPQMASK_COLOR, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqmask_color_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_VPQMASK_COLOR, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqmask_color_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_VPQMASK_COLOR, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqmask_color_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_VPQMASK_COLOR, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqmask_color_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_VPQMASK_COLOR, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqmask_color_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_VPQMASK_COLOR, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqmask_color_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_VPQMASK_COLOR, fmt, ##__VA_ARGS__)
#define rtd_pr_vpqmask_color_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_VPQMASK_COLOR, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for SRNN
 *---------------------------------------*/
#define rtd_pr_srnn_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_SRNN, fmt, ##__VA_ARGS__)
#define rtd_pr_srnn_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_SRNN, fmt, ##__VA_ARGS__)
#define rtd_pr_srnn_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_SRNN, fmt, ##__VA_ARGS__)
#define rtd_pr_srnn_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_SRNN, fmt, ##__VA_ARGS__)
#define rtd_pr_srnn_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_SRNN, fmt, ##__VA_ARGS__)
#define rtd_pr_srnn_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_SRNN, fmt, ##__VA_ARGS__)
#define rtd_pr_srnn_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_SRNN, fmt, ##__VA_ARGS__)
#define rtd_pr_srnn_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_SRNN, fmt, ##__VA_ARGS__)
#define rtd_pr_srnn_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_SRNN, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for VPQ_OD
 *---------------------------------------*/
#define rtd_pr_vpq_od_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_VPQ_OD, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_od_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_VPQ_OD, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_od_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_VPQ_OD, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_od_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_VPQ_OD, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_od_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_VPQ_OD, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_od_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_VPQ_OD, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_od_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_VPQ_OD, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_od_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_VPQ_OD, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_od_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_VPQ_OD, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for FCIC
 *---------------------------------------*/
#define rtd_pr_fcic_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_FCIC, fmt, ##__VA_ARGS__)
#define rtd_pr_fcic_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_FCIC, fmt, ##__VA_ARGS__)
#define rtd_pr_fcic_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_FCIC, fmt, ##__VA_ARGS__)
#define rtd_pr_fcic_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_FCIC, fmt, ##__VA_ARGS__)
#define rtd_pr_fcic_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_FCIC, fmt, ##__VA_ARGS__)
#define rtd_pr_fcic_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_FCIC, fmt, ##__VA_ARGS__)
#define rtd_pr_fcic_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_FCIC, fmt, ##__VA_ARGS__)
#define rtd_pr_fcic_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_FCIC, fmt, ##__VA_ARGS__)
#define rtd_pr_fcic_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_FCIC, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for VPQ_ISR
 *---------------------------------------*/
#define rtd_pr_vpq_isr_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_VPQ_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_isr_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_VPQ_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_isr_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_VPQ_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_isr_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_VPQ_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_isr_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_VPQ_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_isr_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_VPQ_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_isr_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_VPQ_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_isr_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_VPQ_ISR, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_isr_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_VPQ_ISR, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for AI_DBG
 *---------------------------------------*/
#define rtd_pr_ai_dbg_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_AI_DBG, fmt, ##__VA_ARGS__)
#define rtd_pr_ai_dbg_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_AI_DBG, fmt, ##__VA_ARGS__)
#define rtd_pr_ai_dbg_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_AI_DBG, fmt, ##__VA_ARGS__)
#define rtd_pr_ai_dbg_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_AI_DBG, fmt, ##__VA_ARGS__)
#define rtd_pr_ai_dbg_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_AI_DBG, fmt, ##__VA_ARGS__)
#define rtd_pr_ai_dbg_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_AI_DBG, fmt, ##__VA_ARGS__)
#define rtd_pr_ai_dbg_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_AI_DBG, fmt, ##__VA_ARGS__)
#define rtd_pr_ai_dbg_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_AI_DBG, fmt, ##__VA_ARGS__)
#define rtd_pr_ai_dbg_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_AI_DBG, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for RTK_TSK
 *---------------------------------------*/
#define rtd_pr_rtk_tsk_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_RTK_TSK, fmt, ##__VA_ARGS__)
#define rtd_pr_rtk_tsk_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_RTK_TSK, fmt, ##__VA_ARGS__)
#define rtd_pr_rtk_tsk_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_RTK_TSK, fmt, ##__VA_ARGS__)
#define rtd_pr_rtk_tsk_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_RTK_TSK, fmt, ##__VA_ARGS__)
#define rtd_pr_rtk_tsk_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_RTK_TSK, fmt, ##__VA_ARGS__)
#define rtd_pr_rtk_tsk_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_RTK_TSK, fmt, ##__VA_ARGS__)
#define rtd_pr_rtk_tsk_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_RTK_TSK, fmt, ##__VA_ARGS__)
#define rtd_pr_rtk_tsk_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_RTK_TSK, fmt, ##__VA_ARGS__)
#define rtd_pr_rtk_tsk_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_RTK_TSK, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for SE
 *---------------------------------------*/
#define rtd_pr_se_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_SE, fmt, ##__VA_ARGS__)
#define rtd_pr_se_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_SE, fmt, ##__VA_ARGS__)
#define rtd_pr_se_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_SE, fmt, ##__VA_ARGS__)
#define rtd_pr_se_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_SE, fmt, ##__VA_ARGS__)
#define rtd_pr_se_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_SE, fmt, ##__VA_ARGS__)
#define rtd_pr_se_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_SE, fmt, ##__VA_ARGS__)
#define rtd_pr_se_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_SE, fmt, ##__VA_ARGS__)
#define rtd_pr_se_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_SE, fmt, ##__VA_ARGS__)
#define rtd_pr_se_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_SE, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for GDMA
 *---------------------------------------*/
#define rtd_pr_gdma_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_GDMA, fmt, ##__VA_ARGS__)
#define rtd_pr_gdma_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_GDMA, fmt, ##__VA_ARGS__)
#define rtd_pr_gdma_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_GDMA, fmt, ##__VA_ARGS__)
#define rtd_pr_gdma_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_GDMA, fmt, ##__VA_ARGS__)
#define rtd_pr_gdma_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_GDMA, fmt, ##__VA_ARGS__)
#define rtd_pr_gdma_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_GDMA, fmt, ##__VA_ARGS__)
#define rtd_pr_gdma_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_GDMA, fmt, ##__VA_ARGS__)
#define rtd_pr_gdma_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_GDMA, fmt, ##__VA_ARGS__)
#define rtd_pr_gdma_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_GDMA, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for DPRX
 *---------------------------------------*/
#define rtd_pr_dprx_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_DPRX, fmt, ##__VA_ARGS__)
#define rtd_pr_dprx_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_DPRX, fmt, ##__VA_ARGS__)
#define rtd_pr_dprx_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_DPRX, fmt, ##__VA_ARGS__)
#define rtd_pr_dprx_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_DPRX, fmt, ##__VA_ARGS__)
#define rtd_pr_dprx_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_DPRX, fmt, ##__VA_ARGS__)
#define rtd_pr_dprx_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_DPRX, fmt, ##__VA_ARGS__)
#define rtd_pr_dprx_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_DPRX, fmt, ##__VA_ARGS__)
#define rtd_pr_dprx_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_DPRX, fmt, ##__VA_ARGS__)
#define rtd_pr_dprx_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_DPRX, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for HDCP
 *---------------------------------------*/
#define rtd_pr_hdcp_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_HDCP, fmt, ##__VA_ARGS__)
#define rtd_pr_hdcp_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_HDCP, fmt, ##__VA_ARGS__)
#define rtd_pr_hdcp_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_HDCP, fmt, ##__VA_ARGS__)
#define rtd_pr_hdcp_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_HDCP, fmt, ##__VA_ARGS__)
#define rtd_pr_hdcp_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_HDCP, fmt, ##__VA_ARGS__)
#define rtd_pr_hdcp_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_HDCP, fmt, ##__VA_ARGS__)
#define rtd_pr_hdcp_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_HDCP, fmt, ##__VA_ARGS__)
#define rtd_pr_hdcp_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_HDCP, fmt, ##__VA_ARGS__)
#define rtd_pr_hdcp_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_HDCP, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for DELIVERY
 *---------------------------------------*/
#define rtd_pr_delivery_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_DELIVERY, fmt, ##__VA_ARGS__)
#define rtd_pr_delivery_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_DELIVERY, fmt, ##__VA_ARGS__)
#define rtd_pr_delivery_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_DELIVERY, fmt, ##__VA_ARGS__)
#define rtd_pr_delivery_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_DELIVERY, fmt, ##__VA_ARGS__)
#define rtd_pr_delivery_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_DELIVERY, fmt, ##__VA_ARGS__)
#define rtd_pr_delivery_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_DELIVERY, fmt, ##__VA_ARGS__)
#define rtd_pr_delivery_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_DELIVERY, fmt, ##__VA_ARGS__)
#define rtd_pr_delivery_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_DELIVERY, fmt, ##__VA_ARGS__)
#define rtd_pr_delivery_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_DELIVERY, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for SE_FILM
 *---------------------------------------*/
#define rtd_pr_se_film_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_SE_FILM, fmt, ##__VA_ARGS__)
#define rtd_pr_se_film_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_SE_FILM, fmt, ##__VA_ARGS__)
#define rtd_pr_se_film_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_SE_FILM, fmt, ##__VA_ARGS__)
#define rtd_pr_se_film_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_SE_FILM, fmt, ##__VA_ARGS__)
#define rtd_pr_se_film_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_SE_FILM, fmt, ##__VA_ARGS__)
#define rtd_pr_se_film_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_SE_FILM, fmt, ##__VA_ARGS__)
#define rtd_pr_se_film_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_SE_FILM, fmt, ##__VA_ARGS__)
#define rtd_pr_se_film_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_SE_FILM, fmt, ##__VA_ARGS__)
#define rtd_pr_se_film_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_SE_FILM, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for DPQ
 *---------------------------------------*/
#define rtd_pr_dpq_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_DPQ, fmt, ##__VA_ARGS__)
#define rtd_pr_dpq_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_DPQ, fmt, ##__VA_ARGS__)
#define rtd_pr_dpq_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_DPQ, fmt, ##__VA_ARGS__)
#define rtd_pr_dpq_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_DPQ, fmt, ##__VA_ARGS__)
#define rtd_pr_dpq_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_DPQ, fmt, ##__VA_ARGS__)
#define rtd_pr_dpq_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_DPQ, fmt, ##__VA_ARGS__)
#define rtd_pr_dpq_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_DPQ, fmt, ##__VA_ARGS__)
#define rtd_pr_dpq_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_DPQ, fmt, ##__VA_ARGS__)
#define rtd_pr_dpq_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_DPQ, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for PQ
 *---------------------------------------*/
#define rtd_pr_pq_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_PQ, fmt, ##__VA_ARGS__)
#define rtd_pr_pq_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_PQ, fmt, ##__VA_ARGS__)
#define rtd_pr_pq_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_PQ, fmt, ##__VA_ARGS__)
#define rtd_pr_pq_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_PQ, fmt, ##__VA_ARGS__)
#define rtd_pr_pq_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_PQ, fmt, ##__VA_ARGS__)
#define rtd_pr_pq_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_PQ, fmt, ##__VA_ARGS__)
#define rtd_pr_pq_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_PQ, fmt, ##__VA_ARGS__)
#define rtd_pr_pq_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_PQ, fmt, ##__VA_ARGS__)
#define rtd_pr_pq_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_PQ, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for VPQ_AI
 *---------------------------------------*/
#define rtd_pr_vpq_ai_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_VPQ_AI, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_ai_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_VPQ_AI, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_ai_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_VPQ_AI, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_ai_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_VPQ_AI, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_ai_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_VPQ_AI, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_ai_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_VPQ_AI, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_ai_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_VPQ_AI, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_ai_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_VPQ_AI, fmt, ##__VA_ARGS__)
#define rtd_pr_vpq_ai_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_VPQ_AI, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for EMMC
 *---------------------------------------*/
#define rtd_pr_emmc_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_EMMC, fmt, ##__VA_ARGS__)
#define rtd_pr_emmc_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_EMMC, fmt, ##__VA_ARGS__)
#define rtd_pr_emmc_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_EMMC, fmt, ##__VA_ARGS__)
#define rtd_pr_emmc_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_EMMC, fmt, ##__VA_ARGS__)
#define rtd_pr_emmc_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_EMMC, fmt, ##__VA_ARGS__)
#define rtd_pr_emmc_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_EMMC, fmt, ##__VA_ARGS__)
#define rtd_pr_emmc_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_EMMC, fmt, ##__VA_ARGS__)
#define rtd_pr_emmc_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_EMMC, fmt, ##__VA_ARGS__)
#define rtd_pr_emmc_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_EMMC, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for SDIO
 *---------------------------------------*/
#define rtd_pr_sdio_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_SDIO, fmt, ##__VA_ARGS__)
#define rtd_pr_sdio_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_SDIO, fmt, ##__VA_ARGS__)
#define rtd_pr_sdio_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_SDIO, fmt, ##__VA_ARGS__)
#define rtd_pr_sdio_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_SDIO, fmt, ##__VA_ARGS__)
#define rtd_pr_sdio_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_SDIO, fmt, ##__VA_ARGS__)
#define rtd_pr_sdio_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_SDIO, fmt, ##__VA_ARGS__)
#define rtd_pr_sdio_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_SDIO, fmt, ##__VA_ARGS__)
#define rtd_pr_sdio_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_SDIO, fmt, ##__VA_ARGS__)
#define rtd_pr_sdio_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_SDIO, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for SPI
 *---------------------------------------*/
#define rtd_pr_spi_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_SPI, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_SPI, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_SPI, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_SPI, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_SPI, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_SPI, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_SPI, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_SPI, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_SPI, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for SPI_MASTER
 *---------------------------------------*/
#define rtd_pr_spi_master_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_SPI_MASTER, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_master_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_SPI_MASTER, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_master_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_SPI_MASTER, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_master_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_SPI_MASTER, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_master_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_SPI_MASTER, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_master_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_SPI_MASTER, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_master_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_SPI_MASTER, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_master_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_SPI_MASTER, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_master_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_SPI_MASTER, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for USB_BB
 *---------------------------------------*/
#define rtd_pr_usb_bb_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_USB_BB, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_bb_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_USB_BB, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_bb_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_USB_BB, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_bb_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_USB_BB, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_bb_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_USB_BB, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_bb_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_USB_BB, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_bb_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_USB_BB, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_bb_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_USB_BB, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_bb_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_USB_BB, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for USB_HUB
 *---------------------------------------*/
#define rtd_pr_usb_hub_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_USB_HUB, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_hub_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_USB_HUB, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_hub_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_USB_HUB, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_hub_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_USB_HUB, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_hub_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_USB_HUB, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_hub_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_USB_HUB, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_hub_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_USB_HUB, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_hub_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_USB_HUB, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_hub_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_USB_HUB, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for PCIE
 *---------------------------------------*/
#define rtd_pr_pcie_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_PCIE, fmt, ##__VA_ARGS__)
#define rtd_pr_pcie_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_PCIE, fmt, ##__VA_ARGS__)
#define rtd_pr_pcie_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_PCIE, fmt, ##__VA_ARGS__)
#define rtd_pr_pcie_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_PCIE, fmt, ##__VA_ARGS__)
#define rtd_pr_pcie_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_PCIE, fmt, ##__VA_ARGS__)
#define rtd_pr_pcie_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_PCIE, fmt, ##__VA_ARGS__)
#define rtd_pr_pcie_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_PCIE, fmt, ##__VA_ARGS__)
#define rtd_pr_pcie_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_PCIE, fmt, ##__VA_ARGS__)
#define rtd_pr_pcie_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_PCIE, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for ETH
 *---------------------------------------*/
#define rtd_pr_eth_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_ETH, fmt, ##__VA_ARGS__)
#define rtd_pr_eth_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_ETH, fmt, ##__VA_ARGS__)
#define rtd_pr_eth_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_ETH, fmt, ##__VA_ARGS__)
#define rtd_pr_eth_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_ETH, fmt, ##__VA_ARGS__)
#define rtd_pr_eth_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_ETH, fmt, ##__VA_ARGS__)
#define rtd_pr_eth_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_ETH, fmt, ##__VA_ARGS__)
#define rtd_pr_eth_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_ETH, fmt, ##__VA_ARGS__)
#define rtd_pr_eth_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_ETH, fmt, ##__VA_ARGS__)
#define rtd_pr_eth_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_ETH, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for HDMI
 *---------------------------------------*/
#define rtd_pr_hdmi_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_HDMI, fmt, ##__VA_ARGS__)
#define rtd_pr_hdmi_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_HDMI, fmt, ##__VA_ARGS__)
#define rtd_pr_hdmi_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_HDMI, fmt, ##__VA_ARGS__)
#define rtd_pr_hdmi_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_HDMI, fmt, ##__VA_ARGS__)
#define rtd_pr_hdmi_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_HDMI, fmt, ##__VA_ARGS__)
#define rtd_pr_hdmi_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_HDMI, fmt, ##__VA_ARGS__)
#define rtd_pr_hdmi_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_HDMI, fmt, ##__VA_ARGS__)
#define rtd_pr_hdmi_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_HDMI, fmt, ##__VA_ARGS__)
#define rtd_pr_hdmi_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_HDMI, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for AMP
 *---------------------------------------*/
#define rtd_pr_amp_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_AMP, fmt, ##__VA_ARGS__)
#define rtd_pr_amp_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_AMP, fmt, ##__VA_ARGS__)
#define rtd_pr_amp_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_AMP, fmt, ##__VA_ARGS__)
#define rtd_pr_amp_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_AMP, fmt, ##__VA_ARGS__)
#define rtd_pr_amp_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_AMP, fmt, ##__VA_ARGS__)
#define rtd_pr_amp_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_AMP, fmt, ##__VA_ARGS__)
#define rtd_pr_amp_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_AMP, fmt, ##__VA_ARGS__)
#define rtd_pr_amp_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_AMP, fmt, ##__VA_ARGS__)
#define rtd_pr_amp_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_AMP, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for AUDIO_HW
 *---------------------------------------*/
#define rtd_pr_audio_hw_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_AUDIO_HW, fmt, ##__VA_ARGS__)
#define rtd_pr_audio_hw_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_AUDIO_HW, fmt, ##__VA_ARGS__)
#define rtd_pr_audio_hw_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_AUDIO_HW, fmt, ##__VA_ARGS__)
#define rtd_pr_audio_hw_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_AUDIO_HW, fmt, ##__VA_ARGS__)
#define rtd_pr_audio_hw_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_AUDIO_HW, fmt, ##__VA_ARGS__)
#define rtd_pr_audio_hw_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_AUDIO_HW, fmt, ##__VA_ARGS__)
#define rtd_pr_audio_hw_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_AUDIO_HW, fmt, ##__VA_ARGS__)
#define rtd_pr_audio_hw_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_AUDIO_HW, fmt, ##__VA_ARGS__)
#define rtd_pr_audio_hw_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_AUDIO_HW, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for ADSP
 *---------------------------------------*/
#define rtd_pr_adsp_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_ADSP, fmt, ##__VA_ARGS__)
#define rtd_pr_adsp_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_ADSP, fmt, ##__VA_ARGS__)
#define rtd_pr_adsp_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_ADSP, fmt, ##__VA_ARGS__)
#define rtd_pr_adsp_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_ADSP, fmt, ##__VA_ARGS__)
#define rtd_pr_adsp_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_ADSP, fmt, ##__VA_ARGS__)
#define rtd_pr_adsp_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_ADSP, fmt, ##__VA_ARGS__)
#define rtd_pr_adsp_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_ADSP, fmt, ##__VA_ARGS__)
#define rtd_pr_adsp_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_ADSP, fmt, ##__VA_ARGS__)
#define rtd_pr_adsp_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_ADSP, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for TP
 *---------------------------------------*/
#define rtd_pr_tp_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_TP, fmt, ##__VA_ARGS__)
#define rtd_pr_tp_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_TP, fmt, ##__VA_ARGS__)
#define rtd_pr_tp_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_TP, fmt, ##__VA_ARGS__)
#define rtd_pr_tp_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_TP, fmt, ##__VA_ARGS__)
#define rtd_pr_tp_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_TP, fmt, ##__VA_ARGS__)
#define rtd_pr_tp_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_TP, fmt, ##__VA_ARGS__)
#define rtd_pr_tp_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_TP, fmt, ##__VA_ARGS__)
#define rtd_pr_tp_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_TP, fmt, ##__VA_ARGS__)
#define rtd_pr_tp_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_TP, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for DEMUX
 *---------------------------------------*/
#define rtd_pr_demux_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_DEMUX, fmt, ##__VA_ARGS__)
#define rtd_pr_demux_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_DEMUX, fmt, ##__VA_ARGS__)
#define rtd_pr_demux_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_DEMUX, fmt, ##__VA_ARGS__)
#define rtd_pr_demux_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_DEMUX, fmt, ##__VA_ARGS__)
#define rtd_pr_demux_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_DEMUX, fmt, ##__VA_ARGS__)
#define rtd_pr_demux_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_DEMUX, fmt, ##__VA_ARGS__)
#define rtd_pr_demux_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_DEMUX, fmt, ##__VA_ARGS__)
#define rtd_pr_demux_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_DEMUX, fmt, ##__VA_ARGS__)
#define rtd_pr_demux_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_DEMUX, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for ATSC3
 *---------------------------------------*/
#define rtd_pr_atsc3_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_ATSC3, fmt, ##__VA_ARGS__)
#define rtd_pr_atsc3_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_ATSC3, fmt, ##__VA_ARGS__)
#define rtd_pr_atsc3_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_ATSC3, fmt, ##__VA_ARGS__)
#define rtd_pr_atsc3_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_ATSC3, fmt, ##__VA_ARGS__)
#define rtd_pr_atsc3_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_ATSC3, fmt, ##__VA_ARGS__)
#define rtd_pr_atsc3_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_ATSC3, fmt, ##__VA_ARGS__)
#define rtd_pr_atsc3_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_ATSC3, fmt, ##__VA_ARGS__)
#define rtd_pr_atsc3_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_ATSC3, fmt, ##__VA_ARGS__)
#define rtd_pr_atsc3_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_ATSC3, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for ATSC3_ALP
 *---------------------------------------*/
#define rtd_pr_atsc3alp_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_ATSC3ALP, fmt, ##__VA_ARGS__)
#define rtd_pr_atsc3alp_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_ATSC3ALP, fmt, ##__VA_ARGS__)
#define rtd_pr_atsc3alp_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_ATSC3ALP, fmt, ##__VA_ARGS__)
#define rtd_pr_atsc3alp_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_ATSC3ALP, fmt, ##__VA_ARGS__)
#define rtd_pr_atsc3alp_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_ATSC3ALP, fmt, ##__VA_ARGS__)
#define rtd_pr_atsc3alp_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_ATSC3ALP, fmt, ##__VA_ARGS__)
#define rtd_pr_atsc3alp_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_ATSC3ALP, fmt, ##__VA_ARGS__)
#define rtd_pr_atsc3alp_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_ATSC3ALP, fmt, ##__VA_ARGS__)
#define rtd_pr_atsc3alp_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_ATSC3ALP, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for JAPAN4K DEMUX
 *---------------------------------------*/
#define rtd_pr_japan4k_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_JAPAN4K, fmt, ##__VA_ARGS__)
#define rtd_pr_japan4k_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_JAPAN4K, fmt, ##__VA_ARGS__)
#define rtd_pr_japan4k_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_JAPAN4K, fmt, ##__VA_ARGS__)
#define rtd_pr_japan4k_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_JAPAN4K, fmt, ##__VA_ARGS__)
#define rtd_pr_japan4k_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_JAPAN4K, fmt, ##__VA_ARGS__)
#define rtd_pr_japan4k_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_JAPAN4K, fmt, ##__VA_ARGS__)
#define rtd_pr_japan4k_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_JAPAN4K, fmt, ##__VA_ARGS__)
#define rtd_pr_japan4k_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_JAPAN4K, fmt, ##__VA_ARGS__)
#define rtd_pr_japan4k_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_JAPAN4K, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for DEMOD
 *---------------------------------------*/
#define rtd_pr_demod_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_DEMOD, fmt, ##__VA_ARGS__)
#define rtd_pr_demod_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_DEMOD, fmt, ##__VA_ARGS__)
#define rtd_pr_demod_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_DEMOD, fmt, ##__VA_ARGS__)
#define rtd_pr_demod_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_DEMOD, fmt, ##__VA_ARGS__)
#define rtd_pr_demod_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_DEMOD, fmt, ##__VA_ARGS__)
#define rtd_pr_demod_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_DEMOD, fmt, ##__VA_ARGS__)
#define rtd_pr_demod_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_DEMOD, fmt, ##__VA_ARGS__)
#define rtd_pr_demod_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_DEMOD, fmt, ##__VA_ARGS__)
#define rtd_pr_demod_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_DEMOD, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for DISEQC
 *---------------------------------------*/
#define rtd_pr_diseqc_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_DISEQC, fmt, ##__VA_ARGS__)
#define rtd_pr_diseqc_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_DISEQC, fmt, ##__VA_ARGS__)
#define rtd_pr_diseqc_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_DISEQC, fmt, ##__VA_ARGS__)
#define rtd_pr_diseqc_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_DISEQC, fmt, ##__VA_ARGS__)
#define rtd_pr_diseqc_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_DISEQC, fmt, ##__VA_ARGS__)
#define rtd_pr_diseqc_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_DISEQC, fmt, ##__VA_ARGS__)
#define rtd_pr_diseqc_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_DISEQC, fmt, ##__VA_ARGS__)
#define rtd_pr_diseqc_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_DISEQC, fmt, ##__VA_ARGS__)
#define rtd_pr_diseqc_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_DISEQC, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for LNB_UART
 *---------------------------------------*/
#define rtd_pr_lnb_uart_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_LNB_UART, fmt, ##__VA_ARGS__)
#define rtd_pr_lnb_uart_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_LNB_UART, fmt, ##__VA_ARGS__)
#define rtd_pr_lnb_uart_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_LNB_UART, fmt, ##__VA_ARGS__)
#define rtd_pr_lnb_uart_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_LNB_UART, fmt, ##__VA_ARGS__)
#define rtd_pr_lnb_uart_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_LNB_UART, fmt, ##__VA_ARGS__)
#define rtd_pr_lnb_uart_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_LNB_UART, fmt, ##__VA_ARGS__)
#define rtd_pr_lnb_uart_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_LNB_UART, fmt, ##__VA_ARGS__)
#define rtd_pr_lnb_uart_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_LNB_UART, fmt, ##__VA_ARGS__)
#define rtd_pr_lnb_uart_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_LNB_UART, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for LNB_GPIO
 *---------------------------------------*/
#define rtd_pr_lnb_gpio_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_LNB_GPIO, fmt, ##__VA_ARGS__)
#define rtd_pr_lnb_gpio_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_LNB_GPIO, fmt, ##__VA_ARGS__)
#define rtd_pr_lnb_gpio_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_LNB_GPIO, fmt, ##__VA_ARGS__)
#define rtd_pr_lnb_gpio_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_LNB_GPIO, fmt, ##__VA_ARGS__)
#define rtd_pr_lnb_gpio_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_LNB_GPIO, fmt, ##__VA_ARGS__)
#define rtd_pr_lnb_gpio_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_LNB_GPIO, fmt, ##__VA_ARGS__)
#define rtd_pr_lnb_gpio_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_LNB_GPIO, fmt, ##__VA_ARGS__)
#define rtd_pr_lnb_gpio_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_LNB_GPIO, fmt, ##__VA_ARGS__)
#define rtd_pr_lnb_gpio_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_LNB_GPIO, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for MD
 *---------------------------------------*/
#define rtd_pr_md_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_MD, fmt, ##__VA_ARGS__)
#define rtd_pr_md_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_MD, fmt, ##__VA_ARGS__)
#define rtd_pr_md_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_MD, fmt, ##__VA_ARGS__)
#define rtd_pr_md_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_MD, fmt, ##__VA_ARGS__)
#define rtd_pr_md_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_MD, fmt, ##__VA_ARGS__)
#define rtd_pr_md_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_MD, fmt, ##__VA_ARGS__)
#define rtd_pr_md_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_MD, fmt, ##__VA_ARGS__)
#define rtd_pr_md_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_MD, fmt, ##__VA_ARGS__)
#define rtd_pr_md_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_MD, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for CW
 *---------------------------------------*/
#define rtd_pr_cw_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_CW, fmt, ##__VA_ARGS__)
#define rtd_pr_cw_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_CW, fmt, ##__VA_ARGS__)
#define rtd_pr_cw_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_CW, fmt, ##__VA_ARGS__)
#define rtd_pr_cw_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_CW, fmt, ##__VA_ARGS__)
#define rtd_pr_cw_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_CW, fmt, ##__VA_ARGS__)
#define rtd_pr_cw_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_CW, fmt, ##__VA_ARGS__)
#define rtd_pr_cw_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_CW, fmt, ##__VA_ARGS__)
#define rtd_pr_cw_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_CW, fmt, ##__VA_ARGS__)
#define rtd_pr_cw_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_CW, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for MCP
 *---------------------------------------*/
#define rtd_pr_mcp_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_MCP, fmt, ##__VA_ARGS__)
#define rtd_pr_mcp_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_MCP, fmt, ##__VA_ARGS__)
#define rtd_pr_mcp_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_MCP, fmt, ##__VA_ARGS__)
#define rtd_pr_mcp_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_MCP, fmt, ##__VA_ARGS__)
#define rtd_pr_mcp_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_MCP, fmt, ##__VA_ARGS__)
#define rtd_pr_mcp_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_MCP, fmt, ##__VA_ARGS__)
#define rtd_pr_mcp_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_MCP, fmt, ##__VA_ARGS__)
#define rtd_pr_mcp_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_MCP, fmt, ##__VA_ARGS__)
#define rtd_pr_mcp_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_MCP, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for OTP
 *---------------------------------------*/
#define rtd_pr_otp_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_OTP, fmt, ##__VA_ARGS__)
#define rtd_pr_otp_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_OTP, fmt, ##__VA_ARGS__)
#define rtd_pr_otp_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_OTP, fmt, ##__VA_ARGS__)
#define rtd_pr_otp_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_OTP, fmt, ##__VA_ARGS__)
#define rtd_pr_otp_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_OTP, fmt, ##__VA_ARGS__)
#define rtd_pr_otp_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_OTP, fmt, ##__VA_ARGS__)
#define rtd_pr_otp_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_OTP, fmt, ##__VA_ARGS__)
#define rtd_pr_otp_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_OTP, fmt, ##__VA_ARGS__)
#define rtd_pr_otp_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_OTP, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for RCP
 *---------------------------------------*/
#define rtd_pr_rcp_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_RCP, fmt, ##__VA_ARGS__)
#define rtd_pr_rcp_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_RCP, fmt, ##__VA_ARGS__)
#define rtd_pr_rcp_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_RCP, fmt, ##__VA_ARGS__)
#define rtd_pr_rcp_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_RCP, fmt, ##__VA_ARGS__)
#define rtd_pr_rcp_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_RCP, fmt, ##__VA_ARGS__)
#define rtd_pr_rcp_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_RCP, fmt, ##__VA_ARGS__)
#define rtd_pr_rcp_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_RCP, fmt, ##__VA_ARGS__)
#define rtd_pr_rcp_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_RCP, fmt, ##__VA_ARGS__)
#define rtd_pr_rcp_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_RCP, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for RNG
 *---------------------------------------*/
#define rtd_pr_rng_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_RNG, fmt, ##__VA_ARGS__)
#define rtd_pr_rng_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_RNG, fmt, ##__VA_ARGS__)
#define rtd_pr_rng_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_RNG, fmt, ##__VA_ARGS__)
#define rtd_pr_rng_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_RNG, fmt, ##__VA_ARGS__)
#define rtd_pr_rng_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_RNG, fmt, ##__VA_ARGS__)
#define rtd_pr_rng_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_RNG, fmt, ##__VA_ARGS__)
#define rtd_pr_rng_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_RNG, fmt, ##__VA_ARGS__)
#define rtd_pr_rng_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_RNG, fmt, ##__VA_ARGS__)
#define rtd_pr_rng_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_RNG, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for RSA
 *---------------------------------------*/
#define rtd_pr_rsa_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_RSA, fmt, ##__VA_ARGS__)
#define rtd_pr_rsa_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_RSA, fmt, ##__VA_ARGS__)
#define rtd_pr_rsa_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_RSA, fmt, ##__VA_ARGS__)
#define rtd_pr_rsa_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_RSA, fmt, ##__VA_ARGS__)
#define rtd_pr_rsa_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_RSA, fmt, ##__VA_ARGS__)
#define rtd_pr_rsa_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_RSA, fmt, ##__VA_ARGS__)
#define rtd_pr_rsa_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_RSA, fmt, ##__VA_ARGS__)
#define rtd_pr_rsa_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_RSA, fmt, ##__VA_ARGS__)
#define rtd_pr_rsa_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_RSA, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for ARM_WARP
 *---------------------------------------*/
#define rtd_pr_arm_warp_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_ARM_WARP, fmt, ##__VA_ARGS__)
#define rtd_pr_arm_warp_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_ARM_WARP, fmt, ##__VA_ARGS__)
#define rtd_pr_arm_warp_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_ARM_WARP, fmt, ##__VA_ARGS__)
#define rtd_pr_arm_warp_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_ARM_WARP, fmt, ##__VA_ARGS__)
#define rtd_pr_arm_warp_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_ARM_WARP, fmt, ##__VA_ARGS__)
#define rtd_pr_arm_warp_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_ARM_WARP, fmt, ##__VA_ARGS__)
#define rtd_pr_arm_warp_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_ARM_WARP, fmt, ##__VA_ARGS__)
#define rtd_pr_arm_warp_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_ARM_WARP, fmt, ##__VA_ARGS__)
#define rtd_pr_arm_warp_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_ARM_WARP, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for DCMT
 *---------------------------------------*/
#define rtd_pr_dcmt_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_DCMT, fmt, ##__VA_ARGS__)
#define rtd_pr_dcmt_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_DCMT, fmt, ##__VA_ARGS__)
#define rtd_pr_dcmt_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_DCMT, fmt, ##__VA_ARGS__)
#define rtd_pr_dcmt_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_DCMT, fmt, ##__VA_ARGS__)
#define rtd_pr_dcmt_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_DCMT, fmt, ##__VA_ARGS__)
#define rtd_pr_dcmt_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_DCMT, fmt, ##__VA_ARGS__)
#define rtd_pr_dcmt_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_DCMT, fmt, ##__VA_ARGS__)
#define rtd_pr_dcmt_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_DCMT, fmt, ##__VA_ARGS__)
#define rtd_pr_dcmt_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_DCMT, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for HWM
 *---------------------------------------*/
#define rtd_pr_hwm_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_HWM, fmt, ##__VA_ARGS__)
#define rtd_pr_hwm_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_HWM, fmt, ##__VA_ARGS__)
#define rtd_pr_hwm_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_HWM, fmt, ##__VA_ARGS__)
#define rtd_pr_hwm_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_HWM, fmt, ##__VA_ARGS__)
#define rtd_pr_hwm_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_HWM, fmt, ##__VA_ARGS__)
#define rtd_pr_hwm_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_HWM, fmt, ##__VA_ARGS__)
#define rtd_pr_hwm_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_HWM, fmt, ##__VA_ARGS__)
#define rtd_pr_hwm_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_HWM, fmt, ##__VA_ARGS__)
#define rtd_pr_hwm_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_HWM, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for THERMAL
 *---------------------------------------*/
#define rtd_pr_thermal_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_THERMAL, fmt, ##__VA_ARGS__)
#define rtd_pr_thermal_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_THERMAL, fmt, ##__VA_ARGS__)
#define rtd_pr_thermal_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_THERMAL, fmt, ##__VA_ARGS__)
#define rtd_pr_thermal_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_THERMAL, fmt, ##__VA_ARGS__)
#define rtd_pr_thermal_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_THERMAL, fmt, ##__VA_ARGS__)
#define rtd_pr_thermal_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_THERMAL, fmt, ##__VA_ARGS__)
#define rtd_pr_thermal_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_THERMAL, fmt, ##__VA_ARGS__)
#define rtd_pr_thermal_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_THERMAL, fmt, ##__VA_ARGS__)
#define rtd_pr_thermal_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_THERMAL, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for SEMA
 *---------------------------------------*/
#define rtd_pr_sema_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_SEMA, fmt, ##__VA_ARGS__)
#define rtd_pr_sema_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_SEMA, fmt, ##__VA_ARGS__)
#define rtd_pr_sema_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_SEMA, fmt, ##__VA_ARGS__)
#define rtd_pr_sema_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_SEMA, fmt, ##__VA_ARGS__)
#define rtd_pr_sema_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_SEMA, fmt, ##__VA_ARGS__)
#define rtd_pr_sema_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_SEMA, fmt, ##__VA_ARGS__)
#define rtd_pr_sema_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_SEMA, fmt, ##__VA_ARGS__)
#define rtd_pr_sema_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_SEMA, fmt, ##__VA_ARGS__)
#define rtd_pr_sema_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_SEMA, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for CEC
 *---------------------------------------*/
#define rtd_pr_cec_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_CEC, fmt, ##__VA_ARGS__)
#define rtd_pr_cec_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_CEC, fmt, ##__VA_ARGS__)
#define rtd_pr_cec_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_CEC, fmt, ##__VA_ARGS__)
#define rtd_pr_cec_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_CEC, fmt, ##__VA_ARGS__)
#define rtd_pr_cec_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_CEC, fmt, ##__VA_ARGS__)
#define rtd_pr_cec_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_CEC, fmt, ##__VA_ARGS__)
#define rtd_pr_cec_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_CEC, fmt, ##__VA_ARGS__)
#define rtd_pr_cec_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_CEC, fmt, ##__VA_ARGS__)
#define rtd_pr_cec_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_CEC, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for DDC
 *---------------------------------------*/
#define rtd_pr_ddc_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_DDC, fmt, ##__VA_ARGS__)
#define rtd_pr_ddc_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_DDC, fmt, ##__VA_ARGS__)
#define rtd_pr_ddc_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_DDC, fmt, ##__VA_ARGS__)
#define rtd_pr_ddc_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_DDC, fmt, ##__VA_ARGS__)
#define rtd_pr_ddc_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_DDC, fmt, ##__VA_ARGS__)
#define rtd_pr_ddc_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_DDC, fmt, ##__VA_ARGS__)
#define rtd_pr_ddc_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_DDC, fmt, ##__VA_ARGS__)
#define rtd_pr_ddc_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_DDC, fmt, ##__VA_ARGS__)
#define rtd_pr_ddc_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_DDC, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for DDC_CI
 *---------------------------------------*/
#define rtd_pr_ddc_ci_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_DDC_CI, fmt, ##__VA_ARGS__)
#define rtd_pr_ddc_ci_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_DDC_CI, fmt, ##__VA_ARGS__)
#define rtd_pr_ddc_ci_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_DDC_CI, fmt, ##__VA_ARGS__)
#define rtd_pr_ddc_ci_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_DDC_CI, fmt, ##__VA_ARGS__)
#define rtd_pr_ddc_ci_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_DDC_CI, fmt, ##__VA_ARGS__)
#define rtd_pr_ddc_ci_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_DDC_CI, fmt, ##__VA_ARGS__)
#define rtd_pr_ddc_ci_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_DDC_CI, fmt, ##__VA_ARGS__)
#define rtd_pr_ddc_ci_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_DDC_CI, fmt, ##__VA_ARGS__)
#define rtd_pr_ddc_ci_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_DDC_CI, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for EXTCON
 *---------------------------------------*/
#define rtd_pr_extcon_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_EXTCON, fmt, ##__VA_ARGS__)
#define rtd_pr_extcon_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_EXTCON, fmt, ##__VA_ARGS__)
#define rtd_pr_extcon_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_EXTCON, fmt, ##__VA_ARGS__)
#define rtd_pr_extcon_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_EXTCON, fmt, ##__VA_ARGS__)
#define rtd_pr_extcon_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_EXTCON, fmt, ##__VA_ARGS__)
#define rtd_pr_extcon_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_EXTCON, fmt, ##__VA_ARGS__)
#define rtd_pr_extcon_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_EXTCON, fmt, ##__VA_ARGS__)
#define rtd_pr_extcon_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_EXTCON, fmt, ##__VA_ARGS__)
#define rtd_pr_extcon_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_EXTCON, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for GPIO
 *---------------------------------------*/
#define rtd_pr_gpio_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_GPIO, fmt, ##__VA_ARGS__)
#define rtd_pr_gpio_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_GPIO, fmt, ##__VA_ARGS__)
#define rtd_pr_gpio_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_GPIO, fmt, ##__VA_ARGS__)
#define rtd_pr_gpio_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_GPIO, fmt, ##__VA_ARGS__)
#define rtd_pr_gpio_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_GPIO, fmt, ##__VA_ARGS__)
#define rtd_pr_gpio_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_GPIO, fmt, ##__VA_ARGS__)
#define rtd_pr_gpio_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_GPIO, fmt, ##__VA_ARGS__)
#define rtd_pr_gpio_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_GPIO, fmt, ##__VA_ARGS__)
#define rtd_pr_gpio_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_GPIO, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for KEYPAD
 *---------------------------------------*/
#define rtd_pr_keypad_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_KEYPAD, fmt, ##__VA_ARGS__)
#define rtd_pr_keypad_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_KEYPAD, fmt, ##__VA_ARGS__)
#define rtd_pr_keypad_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_KEYPAD, fmt, ##__VA_ARGS__)
#define rtd_pr_keypad_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_KEYPAD, fmt, ##__VA_ARGS__)
#define rtd_pr_keypad_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_KEYPAD, fmt, ##__VA_ARGS__)
#define rtd_pr_keypad_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_KEYPAD, fmt, ##__VA_ARGS__)
#define rtd_pr_keypad_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_KEYPAD, fmt, ##__VA_ARGS__)
#define rtd_pr_keypad_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_KEYPAD, fmt, ##__VA_ARGS__)
#define rtd_pr_keypad_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_KEYPAD, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for LED
 *---------------------------------------*/
#define rtd_pr_led_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_LED, fmt, ##__VA_ARGS__)
#define rtd_pr_led_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_LED, fmt, ##__VA_ARGS__)
#define rtd_pr_led_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_LED, fmt, ##__VA_ARGS__)
#define rtd_pr_led_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_LED, fmt, ##__VA_ARGS__)
#define rtd_pr_led_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_LED, fmt, ##__VA_ARGS__)
#define rtd_pr_led_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_LED, fmt, ##__VA_ARGS__)
#define rtd_pr_led_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_LED, fmt, ##__VA_ARGS__)
#define rtd_pr_led_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_LED, fmt, ##__VA_ARGS__)
#define rtd_pr_led_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_LED, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for LED_SERIAL
 *---------------------------------------*/
#define rtd_pr_led_serial_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_LED_SERIAL, fmt, ##__VA_ARGS__)
#define rtd_pr_led_serial_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_LED_SERIAL, fmt, ##__VA_ARGS__)
#define rtd_pr_led_serial_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_LED_SERIAL, fmt, ##__VA_ARGS__)
#define rtd_pr_led_serial_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_LED_SERIAL, fmt, ##__VA_ARGS__)
#define rtd_pr_led_serial_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_LED_SERIAL, fmt, ##__VA_ARGS__)
#define rtd_pr_led_serial_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_LED_SERIAL, fmt, ##__VA_ARGS__)
#define rtd_pr_led_serial_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_LED_SERIAL, fmt, ##__VA_ARGS__)
#define rtd_pr_led_serial_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_LED_SERIAL, fmt, ##__VA_ARGS__)
#define rtd_pr_led_serial_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_LED_SERIAL, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for LSADC
 *---------------------------------------*/
#define rtd_pr_lsadc_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_LSADC, fmt, ##__VA_ARGS__)
#define rtd_pr_lsadc_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_LSADC, fmt, ##__VA_ARGS__)
#define rtd_pr_lsadc_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_LSADC, fmt, ##__VA_ARGS__)
#define rtd_pr_lsadc_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_LSADC, fmt, ##__VA_ARGS__)
#define rtd_pr_lsadc_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_LSADC, fmt, ##__VA_ARGS__)
#define rtd_pr_lsadc_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_LSADC, fmt, ##__VA_ARGS__)
#define rtd_pr_lsadc_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_LSADC, fmt, ##__VA_ARGS__)
#define rtd_pr_lsadc_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_LSADC, fmt, ##__VA_ARGS__)
#define rtd_pr_lsadc_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_LSADC, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for I2C
 *---------------------------------------*/
#define rtd_pr_i2c_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_I2C, fmt, ##__VA_ARGS__)
#define rtd_pr_i2c_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_I2C, fmt, ##__VA_ARGS__)
#define rtd_pr_i2c_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_I2C, fmt, ##__VA_ARGS__)
#define rtd_pr_i2c_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_I2C, fmt, ##__VA_ARGS__)
#define rtd_pr_i2c_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_I2C, fmt, ##__VA_ARGS__)
#define rtd_pr_i2c_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_I2C, fmt, ##__VA_ARGS__)
#define rtd_pr_i2c_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_I2C, fmt, ##__VA_ARGS__)
#define rtd_pr_i2c_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_I2C, fmt, ##__VA_ARGS__)
#define rtd_pr_i2c_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_I2C, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for IR
 *---------------------------------------*/
#define rtd_pr_ir_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_IR, fmt, ##__VA_ARGS__)
#define rtd_pr_ir_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_IR, fmt, ##__VA_ARGS__)
#define rtd_pr_ir_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_IR, fmt, ##__VA_ARGS__)
#define rtd_pr_ir_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_IR, fmt, ##__VA_ARGS__)
#define rtd_pr_ir_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_IR, fmt, ##__VA_ARGS__)
#define rtd_pr_ir_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_IR, fmt, ##__VA_ARGS__)
#define rtd_pr_ir_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_IR, fmt, ##__VA_ARGS__)
#define rtd_pr_ir_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_IR, fmt, ##__VA_ARGS__)
#define rtd_pr_ir_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_IR, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for IRRC
 *---------------------------------------*/
#define rtd_pr_irrc_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_IRRC, fmt, ##__VA_ARGS__)
#define rtd_pr_irrc_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_IRRC, fmt, ##__VA_ARGS__)
#define rtd_pr_irrc_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_IRRC, fmt, ##__VA_ARGS__)
#define rtd_pr_irrc_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_IRRC, fmt, ##__VA_ARGS__)
#define rtd_pr_irrc_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_IRRC, fmt, ##__VA_ARGS__)
#define rtd_pr_irrc_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_IRRC, fmt, ##__VA_ARGS__)
#define rtd_pr_irrc_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_IRRC, fmt, ##__VA_ARGS__)
#define rtd_pr_irrc_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_IRRC, fmt, ##__VA_ARGS__)
#define rtd_pr_irrc_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_IRRC, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for IRTX
 *---------------------------------------*/
#define rtd_pr_irtx_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_IRTX, fmt, ##__VA_ARGS__)
#define rtd_pr_irtx_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_IRTX, fmt, ##__VA_ARGS__)
#define rtd_pr_irtx_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_IRTX, fmt, ##__VA_ARGS__)
#define rtd_pr_irtx_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_IRTX, fmt, ##__VA_ARGS__)
#define rtd_pr_irtx_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_IRTX, fmt, ##__VA_ARGS__)
#define rtd_pr_irtx_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_IRTX, fmt, ##__VA_ARGS__)
#define rtd_pr_irtx_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_IRTX, fmt, ##__VA_ARGS__)
#define rtd_pr_irtx_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_IRTX, fmt, ##__VA_ARGS__)
#define rtd_pr_irtx_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_IRTX, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for PCMCIA
 *---------------------------------------*/
#define rtd_pr_pcmcia_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_PCMCIA, fmt, ##__VA_ARGS__)
#define rtd_pr_pcmcia_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_PCMCIA, fmt, ##__VA_ARGS__)
#define rtd_pr_pcmcia_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_PCMCIA, fmt, ##__VA_ARGS__)
#define rtd_pr_pcmcia_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_PCMCIA, fmt, ##__VA_ARGS__)
#define rtd_pr_pcmcia_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_PCMCIA, fmt, ##__VA_ARGS__)
#define rtd_pr_pcmcia_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_PCMCIA, fmt, ##__VA_ARGS__)
#define rtd_pr_pcmcia_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_PCMCIA, fmt, ##__VA_ARGS__)
#define rtd_pr_pcmcia_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_PCMCIA, fmt, ##__VA_ARGS__)
#define rtd_pr_pcmcia_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_PCMCIA, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for PWM
 *---------------------------------------*/
#define rtd_pr_pwm_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_PWM, fmt, ##__VA_ARGS__)
#define rtd_pr_pwm_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_PWM, fmt, ##__VA_ARGS__)
#define rtd_pr_pwm_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_PWM, fmt, ##__VA_ARGS__)
#define rtd_pr_pwm_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_PWM, fmt, ##__VA_ARGS__)
#define rtd_pr_pwm_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_PWM, fmt, ##__VA_ARGS__)
#define rtd_pr_pwm_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_PWM, fmt, ##__VA_ARGS__)
#define rtd_pr_pwm_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_PWM, fmt, ##__VA_ARGS__)
#define rtd_pr_pwm_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_PWM, fmt, ##__VA_ARGS__)
#define rtd_pr_pwm_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_PWM, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for PWM_LD
 *---------------------------------------*/
#define rtd_pr_pwm_ld_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_PWM_LD, fmt, ##__VA_ARGS__)
#define rtd_pr_pwm_ld_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_PWM_LD, fmt, ##__VA_ARGS__)
#define rtd_pr_pwm_ld_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_PWM_LD, fmt, ##__VA_ARGS__)
#define rtd_pr_pwm_ld_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_PWM_LD, fmt, ##__VA_ARGS__)
#define rtd_pr_pwm_ld_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_PWM_LD, fmt, ##__VA_ARGS__)
#define rtd_pr_pwm_ld_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_PWM_LD, fmt, ##__VA_ARGS__)
#define rtd_pr_pwm_ld_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_PWM_LD, fmt, ##__VA_ARGS__)
#define rtd_pr_pwm_ld_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_PWM_LD, fmt, ##__VA_ARGS__)
#define rtd_pr_pwm_ld_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_PWM_LD, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for SCD
 *---------------------------------------*/
#define rtd_pr_scd_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_SCD, fmt, ##__VA_ARGS__)
#define rtd_pr_scd_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_SCD, fmt, ##__VA_ARGS__)
#define rtd_pr_scd_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_SCD, fmt, ##__VA_ARGS__)
#define rtd_pr_scd_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_SCD, fmt, ##__VA_ARGS__)
#define rtd_pr_scd_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_SCD, fmt, ##__VA_ARGS__)
#define rtd_pr_scd_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_SCD, fmt, ##__VA_ARGS__)
#define rtd_pr_scd_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_SCD, fmt, ##__VA_ARGS__)
#define rtd_pr_scd_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_SCD, fmt, ##__VA_ARGS__)
#define rtd_pr_scd_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_SCD, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for EMCU
 *---------------------------------------*/
#define rtd_pr_emcu_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_EMCU, fmt, ##__VA_ARGS__)
#define rtd_pr_emcu_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_EMCU, fmt, ##__VA_ARGS__)
#define rtd_pr_emcu_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_EMCU, fmt, ##__VA_ARGS__)
#define rtd_pr_emcu_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_EMCU, fmt, ##__VA_ARGS__)
#define rtd_pr_emcu_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_EMCU, fmt, ##__VA_ARGS__)
#define rtd_pr_emcu_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_EMCU, fmt, ##__VA_ARGS__)
#define rtd_pr_emcu_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_EMCU, fmt, ##__VA_ARGS__)
#define rtd_pr_emcu_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_EMCU, fmt, ##__VA_ARGS__)
#define rtd_pr_emcu_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_EMCU, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for MISC
 *---------------------------------------*/
#define rtd_pr_misc_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_MISC, fmt, ##__VA_ARGS__)
#define rtd_pr_misc_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_MISC, fmt, ##__VA_ARGS__)
#define rtd_pr_misc_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_MISC, fmt, ##__VA_ARGS__)
#define rtd_pr_misc_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_MISC, fmt, ##__VA_ARGS__)
#define rtd_pr_misc_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_MISC, fmt, ##__VA_ARGS__)
#define rtd_pr_misc_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_MISC, fmt, ##__VA_ARGS__)
#define rtd_pr_misc_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_MISC, fmt, ##__VA_ARGS__)
#define rtd_pr_misc_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_MISC, fmt, ##__VA_ARGS__)
#define rtd_pr_misc_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_MISC, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for MISC_INTERRUPT
 *---------------------------------------*/
#define rtd_pr_misc_interrupt_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_MISC_INTERRUPT, fmt, ##__VA_ARGS__)
#define rtd_pr_misc_interrupt_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_MISC_INTERRUPT, fmt, ##__VA_ARGS__)
#define rtd_pr_misc_interrupt_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_MISC_INTERRUPT, fmt, ##__VA_ARGS__)
#define rtd_pr_misc_interrupt_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_MISC_INTERRUPT, fmt, ##__VA_ARGS__)
#define rtd_pr_misc_interrupt_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_MISC_INTERRUPT, fmt, ##__VA_ARGS__)
#define rtd_pr_misc_interrupt_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_MISC_INTERRUPT, fmt, ##__VA_ARGS__)
#define rtd_pr_misc_interrupt_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_MISC_INTERRUPT, fmt, ##__VA_ARGS__)
#define rtd_pr_misc_interrupt_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_MISC_INTERRUPT, fmt, ##__VA_ARGS__)
#define rtd_pr_misc_interrupt_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_MISC_INTERRUPT, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for USB_DUMP
 *---------------------------------------*/
#define rtd_pr_usb_dump_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_USB_DUMP, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_dump_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_USB_DUMP, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_dump_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_USB_DUMP, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_dump_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_USB_DUMP, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_dump_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_USB_DUMP, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_dump_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_USB_DUMP, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_dump_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_USB_DUMP, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_dump_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_USB_DUMP, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_dump_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_USB_DUMP, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for RTDLOG
 *---------------------------------------*/
#define rtd_pr_rtdlog_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_RTDLOG, fmt, ##__VA_ARGS__)
#define rtd_pr_rtdlog_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_RTDLOG, fmt, ##__VA_ARGS__)
#define rtd_pr_rtdlog_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_RTDLOG, fmt, ##__VA_ARGS__)
#define rtd_pr_rtdlog_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_RTDLOG, fmt, ##__VA_ARGS__)
#define rtd_pr_rtdlog_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_RTDLOG, fmt, ##__VA_ARGS__)
#define rtd_pr_rtdlog_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_RTDLOG, fmt, ##__VA_ARGS__)
#define rtd_pr_rtdlog_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_RTDLOG, fmt, ##__VA_ARGS__)
#define rtd_pr_rtdlog_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_RTDLOG, fmt, ##__VA_ARGS__)
#define rtd_pr_rtdlog_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_RTDLOG, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for ONLINE_HELP
 *---------------------------------------*/
#define rtd_pr_online_help_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_ONLINE_HELP, fmt, ##__VA_ARGS__)
#define rtd_pr_online_help_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_ONLINE_HELP, fmt, ##__VA_ARGS__)
#define rtd_pr_online_help_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_ONLINE_HELP, fmt, ##__VA_ARGS__)
#define rtd_pr_online_help_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_ONLINE_HELP, fmt, ##__VA_ARGS__)
#define rtd_pr_online_help_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_ONLINE_HELP, fmt, ##__VA_ARGS__)
#define rtd_pr_online_help_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_ONLINE_HELP, fmt, ##__VA_ARGS__)
#define rtd_pr_online_help_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_ONLINE_HELP, fmt, ##__VA_ARGS__)
#define rtd_pr_online_help_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_ONLINE_HELP, fmt, ##__VA_ARGS__)
#define rtd_pr_online_help_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_ONLINE_HELP, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for LG_COUNTRY_TYPE
 *---------------------------------------*/
#define rtd_pr_lg_country_type_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_LG_COUNTRY_TYPE, fmt, ##__VA_ARGS__)
#define rtd_pr_lg_country_type_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_LG_COUNTRY_TYPE, fmt, ##__VA_ARGS__)
#define rtd_pr_lg_country_type_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_LG_COUNTRY_TYPE, fmt, ##__VA_ARGS__)
#define rtd_pr_lg_country_type_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_LG_COUNTRY_TYPE, fmt, ##__VA_ARGS__)
#define rtd_pr_lg_country_type_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_LG_COUNTRY_TYPE, fmt, ##__VA_ARGS__)
#define rtd_pr_lg_country_type_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_LG_COUNTRY_TYPE, fmt, ##__VA_ARGS__)
#define rtd_pr_lg_country_type_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_LG_COUNTRY_TYPE, fmt, ##__VA_ARGS__)
#define rtd_pr_lg_country_type_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_LG_COUNTRY_TYPE, fmt, ##__VA_ARGS__)
#define rtd_pr_lg_country_type_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_LG_COUNTRY_TYPE, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for CPU_FREQ
 *---------------------------------------*/
#define rtd_pr_cpu_freq_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_CPU_FREQ, fmt, ##__VA_ARGS__)
#define rtd_pr_cpu_freq_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_CPU_FREQ, fmt, ##__VA_ARGS__)
#define rtd_pr_cpu_freq_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_CPU_FREQ, fmt, ##__VA_ARGS__)
#define rtd_pr_cpu_freq_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_CPU_FREQ, fmt, ##__VA_ARGS__)
#define rtd_pr_cpu_freq_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_CPU_FREQ, fmt, ##__VA_ARGS__)
#define rtd_pr_cpu_freq_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_CPU_FREQ, fmt, ##__VA_ARGS__)
#define rtd_pr_cpu_freq_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_CPU_FREQ, fmt, ##__VA_ARGS__)
#define rtd_pr_cpu_freq_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_CPU_FREQ, fmt, ##__VA_ARGS__)
#define rtd_pr_cpu_freq_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_CPU_FREQ, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for DDR_SCAN
 *---------------------------------------*/
#define rtd_pr_ddr_scan_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_DDR_SCAN, fmt, ##__VA_ARGS__)
#define rtd_pr_ddr_scan_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_DDR_SCAN, fmt, ##__VA_ARGS__)
#define rtd_pr_ddr_scan_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_DDR_SCAN, fmt, ##__VA_ARGS__)
#define rtd_pr_ddr_scan_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_DDR_SCAN, fmt, ##__VA_ARGS__)
#define rtd_pr_ddr_scan_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_DDR_SCAN, fmt, ##__VA_ARGS__)
#define rtd_pr_ddr_scan_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_DDR_SCAN, fmt, ##__VA_ARGS__)
#define rtd_pr_ddr_scan_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_DDR_SCAN, fmt, ##__VA_ARGS__)
#define rtd_pr_ddr_scan_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_DDR_SCAN, fmt, ##__VA_ARGS__)
#define rtd_pr_ddr_scan_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_DDR_SCAN, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for ZQ
 *---------------------------------------*/
#define rtd_pr_zq_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_ZQ, fmt, ##__VA_ARGS__)
#define rtd_pr_zq_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_ZQ, fmt, ##__VA_ARGS__)
#define rtd_pr_zq_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_ZQ, fmt, ##__VA_ARGS__)
#define rtd_pr_zq_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_ZQ, fmt, ##__VA_ARGS__)
#define rtd_pr_zq_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_ZQ, fmt, ##__VA_ARGS__)
#define rtd_pr_zq_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_ZQ, fmt, ##__VA_ARGS__)
#define rtd_pr_zq_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_ZQ, fmt, ##__VA_ARGS__)
#define rtd_pr_zq_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_ZQ, fmt, ##__VA_ARGS__)
#define rtd_pr_zq_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_ZQ, fmt, ##__VA_ARGS__)
/*----------------------------------------
 * print api for EHCI_PLAT
 *---------------------------------------*/
#define rtd_pr_ehci_plat_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_EHCI_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_ehci_plat_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_EHCI_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_ehci_plat_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_EHCI_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_ehci_plat_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_EHCI_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_ehci_plat_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_EHCI_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_ehci_plat_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_EHCI_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_ehci_plat_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_EHCI_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_ehci_plat_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_EHCI_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_ehci_plat_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_EHCI_PLAT, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for XHCI_PLAT
 *---------------------------------------*/
#define rtd_pr_xhci_plat_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_XHCI_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_xhci_plat_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_XHCI_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_xhci_plat_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_XHCI_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_xhci_plat_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_XHCI_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_xhci_plat_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_XHCI_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_xhci_plat_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_XHCI_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_xhci_plat_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_XHCI_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_xhci_plat_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_XHCI_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_xhci_plat_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_XHCI_PLAT, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for XHCI_DRD
 *---------------------------------------*/
#define rtd_pr_xhci_drd_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_XHCI_DRD, fmt, ##__VA_ARGS__)
#define rtd_pr_xhci_drd_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_XHCI_DRD, fmt, ##__VA_ARGS__)
#define rtd_pr_xhci_drd_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_XHCI_DRD, fmt, ##__VA_ARGS__)
#define rtd_pr_xhci_drd_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_XHCI_DRD, fmt, ##__VA_ARGS__)
#define rtd_pr_xhci_drd_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_XHCI_DRD, fmt, ##__VA_ARGS__)
#define rtd_pr_xhci_drd_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_XHCI_DRD, fmt, ##__VA_ARGS__)
#define rtd_pr_xhci_drd_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_XHCI_DRD, fmt, ##__VA_ARGS__)
#define rtd_pr_xhci_drd_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_XHCI_DRD, fmt, ##__VA_ARGS__)
#define rtd_pr_xhci_drd_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_XHCI_DRD, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for OTG_PLAT
 *---------------------------------------*/
#define rtd_pr_otg_plat_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_OTG_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_otg_plat_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_OTG_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_otg_plat_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_OTG_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_otg_plat_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_OTG_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_otg_plat_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_OTG_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_otg_plat_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_OTG_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_otg_plat_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_OTG_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_otg_plat_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_OTG_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_otg_plat_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_OTG_PLAT, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for RTK_USB
 *---------------------------------------*/
#define rtd_pr_rtk_usb_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_RTK_USB, fmt, ##__VA_ARGS__)
#define rtd_pr_rtk_usb_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_RTK_USB, fmt, ##__VA_ARGS__)
#define rtd_pr_rtk_usb_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_RTK_USB, fmt, ##__VA_ARGS__)
#define rtd_pr_rtk_usb_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_RTK_USB, fmt, ##__VA_ARGS__)
#define rtd_pr_rtk_usb_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_RTK_USB, fmt, ##__VA_ARGS__)
#define rtd_pr_rtk_usb_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_RTK_USB, fmt, ##__VA_ARGS__)
#define rtd_pr_rtk_usb_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_RTK_USB, fmt, ##__VA_ARGS__)
#define rtd_pr_rtk_usb_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_RTK_USB, fmt, ##__VA_ARGS__)
#define rtd_pr_rtk_usb_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_RTK_USB, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for OHCI_PLAT
 *---------------------------------------*/
#define rtd_pr_ohci_plat_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_OHCI_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_ohci_plat_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_OHCI_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_ohci_plat_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_OHCI_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_ohci_plat_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_OHCI_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_ohci_plat_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_OHCI_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_ohci_plat_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_OHCI_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_ohci_plat_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_OHCI_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_ohci_plat_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_OHCI_PLAT, fmt, ##__VA_ARGS__)
#define rtd_pr_ohci_plat_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_OHCI_PLAT, fmt, ##__VA_ARGS__)


/*----------------------------------------
 * print api for VDEC
 *---------------------------------------*/
#define rtd_pr_vdec_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_VDEC, fmt, ##__VA_ARGS__)
#define rtd_pr_vdec_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_VDEC, fmt, ##__VA_ARGS__)
#define rtd_pr_vdec_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_VDEC, fmt, ##__VA_ARGS__)
#define rtd_pr_vdec_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_VDEC, fmt, ##__VA_ARGS__)
#define rtd_pr_vdec_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_VDEC, fmt, ##__VA_ARGS__)
#define rtd_pr_vdec_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_VDEC, fmt, ##__VA_ARGS__)
#define rtd_pr_vdec_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_VDEC, fmt, ##__VA_ARGS__)
#define rtd_pr_vdec_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_VDEC, fmt, ##__VA_ARGS__)
#define rtd_pr_vdec_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_VDEC, fmt, ##__VA_ARGS__)


/*----------------------------------------
 * print api for USB_GADGET
 *---------------------------------------*/
#define rtd_pr_usb_gadget_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_USB_GADGET, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_gadget_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_USB_GADGET, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_gadget_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_USB_GADGET, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_gadget_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_USB_GADGET, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_gadget_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_USB_GADGET, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_gadget_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_USB_GADGET, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_gadget_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_USB_GADGET, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_gadget_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_USB_GADGET, fmt, ##__VA_ARGS__)
#define rtd_pr_usb_gadget_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_USB_GADGET, fmt, ##__VA_ARGS__)


/*----------------------------------------
 * print api for RTC
 *---------------------------------------*/
#define rtd_pr_rtc_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_RTC, fmt, ##__VA_ARGS__)
#define rtd_pr_rtc_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_RTC, fmt, ##__VA_ARGS__)
#define rtd_pr_rtc_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_RTC, fmt, ##__VA_ARGS__)
#define rtd_pr_rtc_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_RTC, fmt, ##__VA_ARGS__)
#define rtd_pr_rtc_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_RTC, fmt, ##__VA_ARGS__)
#define rtd_pr_rtc_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_RTC, fmt, ##__VA_ARGS__)
#define rtd_pr_rtc_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_RTC, fmt, ##__VA_ARGS__)
#define rtd_pr_rtc_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_RTC, fmt, ##__VA_ARGS__)
#define rtd_pr_rtc_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_RTC, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for PCB_MGR
 *---------------------------------------*/
#define rtd_pr_pcb_mgr_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_PCB_MGR, fmt, ##__VA_ARGS__)
#define rtd_pr_pcb_mgr_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_PCB_MGR, fmt, ##__VA_ARGS__)
#define rtd_pr_pcb_mgr_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_PCB_MGR, fmt, ##__VA_ARGS__)
#define rtd_pr_pcb_mgr_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_PCB_MGR, fmt, ##__VA_ARGS__)
#define rtd_pr_pcb_mgr_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_PCB_MGR, fmt, ##__VA_ARGS__)
#define rtd_pr_pcb_mgr_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_PCB_MGR, fmt, ##__VA_ARGS__)
#define rtd_pr_pcb_mgr_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_PCB_MGR, fmt, ##__VA_ARGS__)
#define rtd_pr_pcb_mgr_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_PCB_MGR, fmt, ##__VA_ARGS__)
#define rtd_pr_pcb_mgr_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_PCB_MGR, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for SWITCH
 *---------------------------------------*/
#define rtd_pr_switch_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_SWITCH, fmt, ##__VA_ARGS__)
#define rtd_pr_switch_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_SWITCH, fmt, ##__VA_ARGS__)
#define rtd_pr_switch_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_SWITCH, fmt, ##__VA_ARGS__)
#define rtd_pr_switch_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_SWITCH, fmt, ##__VA_ARGS__)
#define rtd_pr_switch_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_SWITCH, fmt, ##__VA_ARGS__)
#define rtd_pr_switch_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_SWITCH, fmt, ##__VA_ARGS__)
#define rtd_pr_switch_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_SWITCH, fmt, ##__VA_ARGS__)
#define rtd_pr_switch_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_SWITCH, fmt, ##__VA_ARGS__)
#define rtd_pr_switch_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_SWITCH, fmt, ##__VA_ARGS__)


/*----------------------------------------
 * print api for RTICE
 *---------------------------------------*/
#define rtd_pr_rtice_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_RTICE, fmt, ##__VA_ARGS__)
#define rtd_pr_rtice_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_RTICE, fmt, ##__VA_ARGS__)
#define rtd_pr_rtice_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_RTICE, fmt, ##__VA_ARGS__)
#define rtd_pr_rtice_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_RTICE, fmt, ##__VA_ARGS__)
#define rtd_pr_rtice_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_RTICE, fmt, ##__VA_ARGS__)
#define rtd_pr_rtice_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_RTICE, fmt, ##__VA_ARGS__)
#define rtd_pr_rtice_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_RTICE, fmt, ##__VA_ARGS__)
#define rtd_pr_rtice_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_RTICE, fmt, ##__VA_ARGS__)
#define rtd_pr_rtice_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_RTICE, fmt, ##__VA_ARGS__)



/*----------------------------------------
 * print api for UART
 *---------------------------------------*/
#define rtd_pr_uart_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_UART, fmt, ##__VA_ARGS__)
#define rtd_pr_uart_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_UART, fmt, ##__VA_ARGS__)
#define rtd_pr_uart_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_UART, fmt, ##__VA_ARGS__)
#define rtd_pr_uart_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_UART, fmt, ##__VA_ARGS__)
#define rtd_pr_uart_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_UART, fmt, ##__VA_ARGS__)
#define rtd_pr_uart_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_UART, fmt, ##__VA_ARGS__)
#define rtd_pr_uart_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_UART, fmt, ##__VA_ARGS__)
#define rtd_pr_uart_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_UART, fmt, ##__VA_ARGS__)
#define rtd_pr_uart_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_UART, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for QUICK_SHOW
 *---------------------------------------*/
#define rtd_pr_quick_show_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_QUICK_SHOW, fmt, ##__VA_ARGS__)
#define rtd_pr_quick_show_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_QUICK_SHOW, fmt, ##__VA_ARGS__)
#define rtd_pr_quick_show_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_QUICK_SHOW, fmt, ##__VA_ARGS__)
#define rtd_pr_quick_show_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_QUICK_SHOW, fmt, ##__VA_ARGS__)
#define rtd_pr_quick_show_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_QUICK_SHOW, fmt, ##__VA_ARGS__)
#define rtd_pr_quick_show_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_QUICK_SHOW, fmt, ##__VA_ARGS__)
#define rtd_pr_quick_show_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_QUICK_SHOW, fmt, ##__VA_ARGS__)
#define rtd_pr_quick_show_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_QUICK_SHOW, fmt, ##__VA_ARGS__)
#define rtd_pr_quick_show_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_QUICK_SHOW, fmt, ##__VA_ARGS__)



/*----------------------------------------
 * print api for GDECMP
 *---------------------------------------*/
#define rtd_pr_gdecmp_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_GDECMP, fmt, ##__VA_ARGS__)
#define rtd_pr_gdecmp_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_GDECMP, fmt, ##__VA_ARGS__)
#define rtd_pr_gdecmp_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_GDECMP, fmt, ##__VA_ARGS__)
#define rtd_pr_gdecmp_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_GDECMP, fmt, ##__VA_ARGS__)
#define rtd_pr_gdecmp_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_GDECMP, fmt, ##__VA_ARGS__)
#define rtd_pr_gdecmp_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_GDECMP, fmt, ##__VA_ARGS__)
#define rtd_pr_gdecmp_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_GDECMP, fmt, ##__VA_ARGS__)
#define rtd_pr_gdecmp_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_GDECMP, fmt, ##__VA_ARGS__)
#define rtd_pr_gdecmp_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_GDECMP, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for CRM
 *---------------------------------------*/
#define rtd_pr_crm_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_CRM, fmt, ##__VA_ARGS__)
#define rtd_pr_crm_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_CRM, fmt, ##__VA_ARGS__)
#define rtd_pr_crm_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_CRM, fmt, ##__VA_ARGS__)
#define rtd_pr_crm_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_CRM, fmt, ##__VA_ARGS__)
#define rtd_pr_crm_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_CRM, fmt, ##__VA_ARGS__)
#define rtd_pr_crm_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_CRM, fmt, ##__VA_ARGS__)
#define rtd_pr_crm_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_CRM, fmt, ##__VA_ARGS__)
#define rtd_pr_crm_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_CRM, fmt, ##__VA_ARGS__)
#define rtd_pr_crm_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_CRM, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for OMX
 *---------------------------------------*/
#define rtd_pr_omx_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_OMX, fmt, ##__VA_ARGS__)
#define rtd_pr_omx_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_OMX, fmt, ##__VA_ARGS__)
#define rtd_pr_omx_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_OMX, fmt, ##__VA_ARGS__)
#define rtd_pr_omx_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_OMX, fmt, ##__VA_ARGS__)
#define rtd_pr_omx_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_OMX, fmt, ##__VA_ARGS__)
#define rtd_pr_omx_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_OMX, fmt, ##__VA_ARGS__)
#define rtd_pr_omx_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_OMX, fmt, ##__VA_ARGS__)
#define rtd_pr_omx_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_OMX, fmt, ##__VA_ARGS__)
#define rtd_pr_omx_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_OMX, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for DFT
 *---------------------------------------*/
#define rtd_pr_dft_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_DFT, fmt, ##__VA_ARGS__)
#define rtd_pr_dft_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_DFT, fmt, ##__VA_ARGS__)
#define rtd_pr_dft_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_DFT, fmt, ##__VA_ARGS__)
#define rtd_pr_dft_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_DFT, fmt, ##__VA_ARGS__)
#define rtd_pr_dft_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_DFT, fmt, ##__VA_ARGS__)
#define rtd_pr_dft_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_DFT, fmt, ##__VA_ARGS__)
#define rtd_pr_dft_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_DFT, fmt, ##__VA_ARGS__)
#define rtd_pr_dft_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_DFT, fmt, ##__VA_ARGS__)
#define rtd_pr_dft_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_DFT, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for HW_MONITOR
 *---------------------------------------*/
#define rtd_pr_hw_monitor_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_HW_MONITOR, fmt, ##__VA_ARGS__)
#define rtd_pr_hw_monitor_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_HW_MONITOR, fmt, ##__VA_ARGS__)
#define rtd_pr_hw_monitor_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_HW_MONITOR, fmt, ##__VA_ARGS__)
#define rtd_pr_hw_monitor_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_HW_MONITOR, fmt, ##__VA_ARGS__)
#define rtd_pr_hw_monitor_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_HW_MONITOR, fmt, ##__VA_ARGS__)
#define rtd_pr_hw_monitor_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_HW_MONITOR, fmt, ##__VA_ARGS__)
#define rtd_pr_hw_monitor_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_HW_MONITOR, fmt, ##__VA_ARGS__)
#define rtd_pr_hw_monitor_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_HW_MONITOR, fmt, ##__VA_ARGS__)
#define rtd_pr_hw_monitor_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_HW_MONITOR, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for MACH_RTK
 *---------------------------------------*/
#define rtd_pr_mach_rtk_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_MACH_RTK, fmt, ##__VA_ARGS__)
#define rtd_pr_mach_rtk_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_MACH_RTK, fmt, ##__VA_ARGS__)
#define rtd_pr_mach_rtk_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_MACH_RTK, fmt, ##__VA_ARGS__)
#define rtd_pr_mach_rtk_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_MACH_RTK, fmt, ##__VA_ARGS__)
#define rtd_pr_mach_rtk_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_MACH_RTK, fmt, ##__VA_ARGS__)
#define rtd_pr_mach_rtk_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_MACH_RTK, fmt, ##__VA_ARGS__)
#define rtd_pr_mach_rtk_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_MACH_RTK, fmt, ##__VA_ARGS__)
#define rtd_pr_mach_rtk_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_MACH_RTK, fmt, ##__VA_ARGS__)
#define rtd_pr_mach_rtk_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_MACH_RTK, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for POWER
 *---------------------------------------*/
#define rtd_pr_power_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_POWER, fmt, ##__VA_ARGS__)
#define rtd_pr_power_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_POWER, fmt, ##__VA_ARGS__)
#define rtd_pr_power_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_POWER, fmt, ##__VA_ARGS__)
#define rtd_pr_power_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_POWER, fmt, ##__VA_ARGS__)
#define rtd_pr_power_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_POWER, fmt, ##__VA_ARGS__)
#define rtd_pr_power_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_POWER, fmt, ##__VA_ARGS__)
#define rtd_pr_power_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_POWER, fmt, ##__VA_ARGS__)
#define rtd_pr_power_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_POWER, fmt, ##__VA_ARGS__)
#define rtd_pr_power_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_POWER, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for PLATFORM
 *---------------------------------------*/
#define rtd_pr_platform_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_PLATFORM, fmt, ##__VA_ARGS__)
#define rtd_pr_platform_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_PLATFORM, fmt, ##__VA_ARGS__)
#define rtd_pr_platform_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_PLATFORM, fmt, ##__VA_ARGS__)
#define rtd_pr_platform_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_PLATFORM, fmt, ##__VA_ARGS__)
#define rtd_pr_platform_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_PLATFORM, fmt, ##__VA_ARGS__)
#define rtd_pr_platform_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_PLATFORM, fmt, ##__VA_ARGS__)
#define rtd_pr_platform_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_PLATFORM, fmt, ##__VA_ARGS__)
#define rtd_pr_platform_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_PLATFORM, fmt, ##__VA_ARGS__)
#define rtd_pr_platform_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_PLATFORM, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for CRT
 *---------------------------------------*/
#define rtd_pr_crt_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_CRT, fmt, ##__VA_ARGS__)
#define rtd_pr_crt_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_CRT, fmt, ##__VA_ARGS__)
#define rtd_pr_crt_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_CRT, fmt, ##__VA_ARGS__)
#define rtd_pr_crt_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_CRT, fmt, ##__VA_ARGS__)
#define rtd_pr_crt_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_CRT, fmt, ##__VA_ARGS__)
#define rtd_pr_crt_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_CRT, fmt, ##__VA_ARGS__)
#define rtd_pr_crt_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_CRT, fmt, ##__VA_ARGS__)
#define rtd_pr_crt_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_CRT, fmt, ##__VA_ARGS__)
#define rtd_pr_crt_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_CRT, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for RPC
 *---------------------------------------*/
#define rtd_pr_rpc_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_RPC, fmt, ##__VA_ARGS__)
#define rtd_pr_rpc_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_RPC, fmt, ##__VA_ARGS__)
#define rtd_pr_rpc_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_RPC, fmt, ##__VA_ARGS__)
#define rtd_pr_rpc_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_RPC, fmt, ##__VA_ARGS__)
#define rtd_pr_rpc_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_RPC, fmt, ##__VA_ARGS__)
#define rtd_pr_rpc_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_RPC, fmt, ##__VA_ARGS__)
#define rtd_pr_rpc_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_RPC, fmt, ##__VA_ARGS__)
#define rtd_pr_rpc_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_RPC, fmt, ##__VA_ARGS__)
#define rtd_pr_rpc_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_RPC, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for RTK_TEE
 *---------------------------------------*/
#define rtd_pr_rtk_tee_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_RTK_TEE, fmt, ##__VA_ARGS__)
#define rtd_pr_rtk_tee_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_RTK_TEE, fmt, ##__VA_ARGS__)
#define rtd_pr_rtk_tee_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_RTK_TEE, fmt, ##__VA_ARGS__)
#define rtd_pr_rtk_tee_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_RTK_TEE, fmt, ##__VA_ARGS__)
#define rtd_pr_rtk_tee_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_RTK_TEE, fmt, ##__VA_ARGS__)
#define rtd_pr_rtk_tee_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_RTK_TEE, fmt, ##__VA_ARGS__)
#define rtd_pr_rtk_tee_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_RTK_TEE, fmt, ##__VA_ARGS__)
#define rtd_pr_rtk_tee_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_RTK_TEE, fmt, ##__VA_ARGS__)
#define rtd_pr_rtk_tee_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_RTK_TEE, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for SPI_NOR
 *---------------------------------------*/
#define rtd_pr_spi_nor_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_SPI_NOR, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_nor_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_SPI_NOR, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_nor_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_SPI_NOR, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_nor_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_SPI_NOR, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_nor_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_SPI_NOR, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_nor_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_SPI_NOR, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_nor_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_SPI_NOR, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_nor_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_SPI_NOR, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_nor_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_SPI_NOR, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for SPI_DEV
 *---------------------------------------*/
#define rtd_pr_spi_dev_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_SPI_DEV, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_dev_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_SPI_DEV, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_dev_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_SPI_DEV, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_dev_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_SPI_DEV, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_dev_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_SPI_DEV, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_dev_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_SPI_DEV, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_dev_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_SPI_DEV, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_dev_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_SPI_DEV, fmt, ##__VA_ARGS__)
#define rtd_pr_spi_dev_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_SPI_DEV, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for RMM
 *---------------------------------------*/
#define rtd_pr_rmm_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_RMM, fmt, ##__VA_ARGS__)
#define rtd_pr_rmm_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_RMM, fmt, ##__VA_ARGS__)
#define rtd_pr_rmm_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_RMM, fmt, ##__VA_ARGS__)
#define rtd_pr_rmm_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_RMM, fmt, ##__VA_ARGS__)
#define rtd_pr_rmm_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_RMM, fmt, ##__VA_ARGS__)
#define rtd_pr_rmm_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_RMM, fmt, ##__VA_ARGS__)
#define rtd_pr_rmm_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_RMM, fmt, ##__VA_ARGS__)
#define rtd_pr_rmm_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_RMM, fmt, ##__VA_ARGS__)
#define rtd_pr_rmm_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_RMM, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for QOS
 *---------------------------------------*/
#define rtd_pr_qos_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_QOS, fmt, ##__VA_ARGS__)
#define rtd_pr_qos_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_QOS, fmt, ##__VA_ARGS__)
#define rtd_pr_qos_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_QOS, fmt, ##__VA_ARGS__)
#define rtd_pr_qos_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_QOS, fmt, ##__VA_ARGS__)
#define rtd_pr_qos_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_QOS, fmt, ##__VA_ARGS__)
#define rtd_pr_qos_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_QOS, fmt, ##__VA_ARGS__)
#define rtd_pr_qos_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_QOS, fmt, ##__VA_ARGS__)
#define rtd_pr_qos_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_QOS, fmt, ##__VA_ARGS__)
#define rtd_pr_qos_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_QOS, fmt, ##__VA_ARGS__)


/*----------------------------------------
 * print api for VENUSFB
 *---------------------------------------*/
#define rtd_pr_venusfb_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_VENUSFB, fmt, ##__VA_ARGS__)
#define rtd_pr_venusfb_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_VENUSFB, fmt, ##__VA_ARGS__)
#define rtd_pr_venusfb_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_VENUSFB, fmt, ##__VA_ARGS__)
#define rtd_pr_venusfb_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_VENUSFB, fmt, ##__VA_ARGS__)
#define rtd_pr_venusfb_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_VENUSFB, fmt, ##__VA_ARGS__)
#define rtd_pr_venusfb_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_VENUSFB, fmt, ##__VA_ARGS__)
#define rtd_pr_venusfb_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_VENUSFB, fmt, ##__VA_ARGS__)
#define rtd_pr_venusfb_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_VENUSFB, fmt, ##__VA_ARGS__)
#define rtd_pr_venusfb_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_VENUSFB, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for MEASURE
 *---------------------------------------*/
#define rtd_pr_measure_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_MEASURE, fmt, ##__VA_ARGS__)
#define rtd_pr_measure_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_MEASURE, fmt, ##__VA_ARGS__)
#define rtd_pr_measure_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_MEASURE, fmt, ##__VA_ARGS__)
#define rtd_pr_measure_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_MEASURE, fmt, ##__VA_ARGS__)
#define rtd_pr_measure_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_MEASURE, fmt, ##__VA_ARGS__)
#define rtd_pr_measure_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_MEASURE, fmt, ##__VA_ARGS__)
#define rtd_pr_measure_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_MEASURE, fmt, ##__VA_ARGS__)
#define rtd_pr_measure_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_MEASURE, fmt, ##__VA_ARGS__)
#define rtd_pr_measure_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_MEASURE, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for DSCD
 *---------------------------------------*/
#define rtd_pr_dscd_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_DSCD, fmt, ##__VA_ARGS__)
#define rtd_pr_dscd_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_DSCD, fmt, ##__VA_ARGS__)
#define rtd_pr_dscd_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_DSCD, fmt, ##__VA_ARGS__)
#define rtd_pr_dscd_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_DSCD, fmt, ##__VA_ARGS__)
#define rtd_pr_dscd_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_DSCD, fmt, ##__VA_ARGS__)
#define rtd_pr_dscd_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_DSCD, fmt, ##__VA_ARGS__)
#define rtd_pr_dscd_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_DSCD, fmt, ##__VA_ARGS__)
#define rtd_pr_dscd_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_DSCD, fmt, ##__VA_ARGS__)
#define rtd_pr_dscd_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_DSCD, fmt, ##__VA_ARGS__)

/*----------------------------------------
 * print api for EVENTLOG
 *---------------------------------------*/
#define rtd_pr_eventlog_emerg(fmt, ...)              _rtd_pr_emerg(  TAG_NAME_EVENTLOG, fmt, ##__VA_ARGS__)
#define rtd_pr_eventlog_alert(fmt, ...)              _rtd_pr_alert(  TAG_NAME_EVENTLOG, fmt, ##__VA_ARGS__)
#define rtd_pr_eventlog_crit(fmt, ...)               _rtd_pr_crit(   TAG_NAME_EVENTLOG, fmt, ##__VA_ARGS__)
#define rtd_pr_eventlog_err(fmt, ...)                _rtd_pr_err(    TAG_NAME_EVENTLOG, fmt, ##__VA_ARGS__)
#define rtd_pr_eventlog_warn(fmt, ...)               _rtd_pr_warn(   TAG_NAME_EVENTLOG, fmt, ##__VA_ARGS__)
#define rtd_pr_eventlog_notice(fmt, ...)             _rtd_pr_notice( TAG_NAME_EVENTLOG, fmt, ##__VA_ARGS__)
#define rtd_pr_eventlog_info(fmt, ...)               _rtd_pr_info(   TAG_NAME_EVENTLOG, fmt, ##__VA_ARGS__)
#define rtd_pr_eventlog_debug(fmt, ...)              _rtd_pr_debug(  TAG_NAME_EVENTLOG, fmt, ##__VA_ARGS__)
#define rtd_pr_eventlog_print(level, fmt, ...)       _rtd_pr_print(level, TAG_NAME_EVENTLOG, fmt, ##__VA_ARGS__)