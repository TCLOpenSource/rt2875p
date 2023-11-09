#include <rtk_kdriver/rtk_pwm.h>
#include "rtk_pwm-reg.h"
#include "rtk_pwm_crt.h"
#include "rtk_pwm_func.h"
#include "rtk_pwm_rtice.h"
#include <tvscalercontrol/panel/panelapi.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <mach/rtk_platform.h>
#include <mach/platform.h>

#define GET_EPK_VERSION_PATH        "etc/starfish-release"

int rtk_pwm_set_freq(R_CHIP_T *pchip2, int h_total, int v_total, int ifold, bool bEnableDB);
int rtk_pwm_get_freq(R_CHIP_T *pchip2,unsigned short M, unsigned short N);


extern void rtd_part_outl(unsigned int reg_addr, unsigned int endBit, unsigned int startBit, unsigned int value);
#ifdef CONFIG_CUSTOMER_TV006
extern webos_strInfo_t webos_strToolOption;

static struct file* pwm_open_file(const char* path, int flags, int rights)
{
    struct file* filp = NULL;
    mm_segment_t oldfs;
    int err = 0;
    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open(path, flags, rights);
    set_fs(oldfs);
    if(IS_ERR(filp)) {
        err = PTR_ERR(filp);
        return NULL;
    }
    return filp;
}

static int pwm_read_file(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size)
{
    mm_segment_t oldfs;
    int ret;
    oldfs = get_fs();
    set_fs(get_ds());
    ret = vfs_read(file, data, size, &offset);
    set_fs(oldfs);
    return ret;
}

static void pwm_file_close(struct file* file)
{
    filp_close(file, NULL);
}
#endif

int rwPanelTool(unsigned char functionId, char* pParam, int paramLen, char* pOutBuffer)
{
    R_CHIP_T *pchip2 = NULL;
    int ret = 0;
    int i = 0;
    int m_PWM_index = 0;
    int pwm_freq = 0;
    int pwm_duty = 0;
    int DVS_freq = 0;
    int v_total = 0;
    int is_MISC_PWM = 0;
    #ifdef CONFIG_CUSTOMER_TV006
    int string_counter = 0;
    int is_K6HP_platform = 0;
    struct file* filp = NULL;
    char buffer[100] = {0};
    #endif

    PWM_INFO("%s functionId: %x paramLen: %d\n", __func__, functionId, paramLen);

    for(i = 0; i < paramLen; i++) {
        PWM_INFO("pParam[%d] = 0x%x\n", i, pParam[i]);
    }
    if ( ((pParam[0] >> 7) & 0x1) != 1 ) {
        is_MISC_PWM = 1;
    }
    PWM_INFO("%s is_MISC_PWM = %d\n", __func__, is_MISC_PWM);

    m_PWM_index = (pParam[0] & 0x7F);

    if(is_MISC_PWM) {
        pchip2 = rtk_pwm_chip_get_by_index(m_PWM_index, PWM_MISC);
        PWM_INFO("%s PWM_MISC m_PWM_index = %d\n", __func__, m_PWM_index);
    }
    else {
        pchip2 = rtk_pwm_chip_get_by_index(m_PWM_index, PWM_ISO);
        PWM_INFO("%s PWM_ISO m_PWM_index = %d\n", __func__, m_PWM_index);
    }
    if(pchip2 == NULL) {
        PWM_ERR("PWM chip NOT FOUND !!! (m_PWM_index = %d)\n", m_PWM_index);
        return 0;
    }

    PWM_INFO("%s %s\n", __func__, pchip2->nodename);
    PWM_INFO("%s pchip2->rtk_enable: %d\n", __func__, pchip2->rtk_enable);
    PWM_INFO("%s pchip2->rtk_duty: %d\n", __func__, pchip2->rtk_duty);
    PWM_INFO("%s pchip2->rtk_duty_max: %d\n", __func__, pchip2->rtk_duty_max);
    PWM_INFO("%s pchip2->rtk_totalcnt: %d\n", __func__, pchip2->rtk_totalcnt);
    PWM_INFO("%s pchip2->rtk_freq: %d\n", __func__, pchip2->rtk_freq);
    PWM_INFO("%s pchip2->rtk_enable: %d\n", __func__, pchip2->rtk_enable);
    PWM_INFO("%s pchip2->rtk_polarity: %d\n", __func__, pchip2->rtk_polarity);
    PWM_INFO("%s rtd_inl(PPOVERLAY_DV_total_reg): %x\n", __func__, rtd_inl(PPOVERLAY_DV_total_reg));

    switch (functionId) {
        case RTK_PWM_RTICE_GET_PWM:
            PWM_INFO("%s RTK_PWM_RTICE_GET_PWM, functionId=%d \n", __func__, functionId);
            //Freq
            pOutBuffer[ret++] = (pchip2->rtk_freq >> 24) & 0xFF;
            pOutBuffer[ret++] = (pchip2->rtk_freq >> 16) & 0xFF;
            pOutBuffer[ret++] = (pchip2->rtk_freq >>  8) & 0xFF;
            pOutBuffer[ret++] = (pchip2->rtk_freq >>  0) & 0xFF;
            // duty
            pOutBuffer[ret++] = (pchip2->rtk_duty >>  8) & 0xFF;
            pOutBuffer[ret++] = (pchip2->rtk_duty >>  0) & 0xFF;
            // polarity
            pOutBuffer[ret++] = pchip2->rtk_polarity;
            // register addr: MISC_PWM_PWM_DB0_CTRL_reg
            pOutBuffer[ret++] = (MISC_PWM_PWM_DB0_CTRL_reg >> 24) & 0xFF;
            pOutBuffer[ret++] = (MISC_PWM_PWM_DB0_CTRL_reg >> 16) & 0xFF;
            pOutBuffer[ret++] = (MISC_PWM_PWM_DB0_CTRL_reg >>  8) & 0xFF;
            pOutBuffer[ret++] = (MISC_PWM_PWM_DB0_CTRL_reg >>  0) & 0xFF;
            // register data: MISC_PWM_PWM_DB0_CTRL_reg
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM_DB0_CTRL_reg) >> 24) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM_DB0_CTRL_reg) >> 16) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM_DB0_CTRL_reg) >>  8) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM_DB0_CTRL_reg) >>  0) & 0xFF;
            // register addr: MISC_PWM_PWM_DB1_CTRL_reg
            pOutBuffer[ret++] = (MISC_PWM_PWM_DB1_CTRL_reg >> 24) & 0xFF;
            pOutBuffer[ret++] = (MISC_PWM_PWM_DB1_CTRL_reg >> 16) & 0xFF;
            pOutBuffer[ret++] = (MISC_PWM_PWM_DB1_CTRL_reg >>  8) & 0xFF;
            pOutBuffer[ret++] = (MISC_PWM_PWM_DB1_CTRL_reg >>  0) & 0xFF;
            // register data: MISC_PWM_PWM_DB1_CTRL_reg
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM_DB1_CTRL_reg) >> 24) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM_DB1_CTRL_reg) >> 16) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM_DB1_CTRL_reg) >>  8) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM_DB1_CTRL_reg) >>  0) & 0xFF;



            // register addr: MISC_PWM_PWM0_CTRL_reg
            pOutBuffer[ret++] = (MISC_PWM_PWM0_CTRL_reg >> 24) & 0xFF;
            pOutBuffer[ret++] = (MISC_PWM_PWM0_CTRL_reg >> 16) & 0xFF;
            pOutBuffer[ret++] = (MISC_PWM_PWM0_CTRL_reg >>  8) & 0xFF;
            pOutBuffer[ret++] = (MISC_PWM_PWM0_CTRL_reg >>  0) & 0xFF;
            // register data: MISC_PWM_PWM0_CTRL_reg
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM0_CTRL_reg) >> 24) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM0_CTRL_reg) >> 16) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM0_CTRL_reg) >>  8) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM0_CTRL_reg) >>  0) & 0xFF;
            // register addr: MISC_PWM_PWM0_Timing_CTRL_reg
            pOutBuffer[ret++] = (MISC_PWM_PWM0_Timing_CTRL_reg >> 24) & 0xFF;
            pOutBuffer[ret++] = (MISC_PWM_PWM0_Timing_CTRL_reg >> 16) & 0xFF;
            pOutBuffer[ret++] = (MISC_PWM_PWM0_Timing_CTRL_reg >>  8) & 0xFF;
            pOutBuffer[ret++] = (MISC_PWM_PWM0_Timing_CTRL_reg >>  0) & 0xFF;
            // register data: MISC_PWM_PWM0_Timing_CTRL_reg
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM0_Timing_CTRL_reg) >> 24) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM0_Timing_CTRL_reg) >> 16) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM0_Timing_CTRL_reg) >>  8) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM0_Timing_CTRL_reg) >>  0) & 0xFF;
            // register addr: MISC_PWM_PWM0_DUTY_SET_reg
            pOutBuffer[ret++] = (MISC_PWM_PWM0_DUTY_SET_reg >> 24) & 0xFF;
            pOutBuffer[ret++] = (MISC_PWM_PWM0_DUTY_SET_reg >> 16) & 0xFF;
            pOutBuffer[ret++] = (MISC_PWM_PWM0_DUTY_SET_reg >>  8) & 0xFF;
            pOutBuffer[ret++] = (MISC_PWM_PWM0_DUTY_SET_reg >>  0) & 0xFF;
            // register data: MISC_PWM_PWM0_DUTY_SET_reg
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM0_DUTY_SET_reg) >> 24) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM0_DUTY_SET_reg) >> 16) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM0_DUTY_SET_reg) >>  8) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM0_DUTY_SET_reg) >>  0) & 0xFF;
            // register addr: MISC_PWM_PWM0_DVS_PERIOD_reg
            pOutBuffer[ret++] = (MISC_PWM_PWM0_DVS_PERIOD_reg >> 24) & 0xFF;
            pOutBuffer[ret++] = (MISC_PWM_PWM0_DVS_PERIOD_reg >> 16) & 0xFF;
            pOutBuffer[ret++] = (MISC_PWM_PWM0_DVS_PERIOD_reg >>  8) & 0xFF;
            pOutBuffer[ret++] = (MISC_PWM_PWM0_DVS_PERIOD_reg >>  0) & 0xFF;
            // register data: MISC_PWM_PWM0_DVS_PERIOD_reg
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM0_DVS_PERIOD_reg) >> 24) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM0_DVS_PERIOD_reg) >> 16) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM0_DVS_PERIOD_reg) >>  8) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM0_DVS_PERIOD_reg) >>  0) & 0xFF;



            // register addr: MISC_PWM_PWM1_CTRL_reg
            pOutBuffer[ret++] = (MISC_PWM_PWM1_CTRL_reg >> 24) & 0xFF;
            pOutBuffer[ret++] = (MISC_PWM_PWM1_CTRL_reg >> 16) & 0xFF;
            pOutBuffer[ret++] = (MISC_PWM_PWM1_CTRL_reg >>  8) & 0xFF;
            pOutBuffer[ret++] = (MISC_PWM_PWM1_CTRL_reg >>  0) & 0xFF;
            // register data: MISC_PWM_PWM1_CTRL_reg
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM1_CTRL_reg) >> 24) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM1_CTRL_reg) >> 16) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM1_CTRL_reg) >>  8) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM1_CTRL_reg) >>  0) & 0xFF;
            // register addr: MISC_PWM_PWM1_Timing_CTRL_reg
            pOutBuffer[ret++] = (MISC_PWM_PWM1_Timing_CTRL_reg >> 24) & 0xFF;
            pOutBuffer[ret++] = (MISC_PWM_PWM1_Timing_CTRL_reg >> 16) & 0xFF;
            pOutBuffer[ret++] = (MISC_PWM_PWM1_Timing_CTRL_reg >>  8) & 0xFF;
            pOutBuffer[ret++] = (MISC_PWM_PWM1_Timing_CTRL_reg >>  0) & 0xFF;
            // register data: MISC_PWM_PWM1_Timing_CTRL_reg
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM1_Timing_CTRL_reg) >> 24) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM1_Timing_CTRL_reg) >> 16) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM1_Timing_CTRL_reg) >>  8) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM1_Timing_CTRL_reg) >>  0) & 0xFF;
            // register addr: MISC_PWM_PWM1_DUTY_SET_reg
            pOutBuffer[ret++] = (MISC_PWM_PWM1_DUTY_SET_reg >> 24) & 0xFF;
            pOutBuffer[ret++] = (MISC_PWM_PWM1_DUTY_SET_reg >> 16) & 0xFF;
            pOutBuffer[ret++] = (MISC_PWM_PWM1_DUTY_SET_reg >>  8) & 0xFF;
            pOutBuffer[ret++] = (MISC_PWM_PWM1_DUTY_SET_reg >>  0) & 0xFF;
            // register data: MISC_PWM_PWM1_DUTY_SET_reg
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM1_DUTY_SET_reg) >> 24) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM1_DUTY_SET_reg) >> 16) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM1_DUTY_SET_reg) >>  8) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM1_DUTY_SET_reg) >>  0) & 0xFF;
            // register addr: MISC_PWM_PWM1_DVS_PERIOD_reg
            pOutBuffer[ret++] = (MISC_PWM_PWM1_DVS_PERIOD_reg >> 24) & 0xFF;
            pOutBuffer[ret++] = (MISC_PWM_PWM1_DVS_PERIOD_reg >> 16) & 0xFF;
            pOutBuffer[ret++] = (MISC_PWM_PWM1_DVS_PERIOD_reg >>  8) & 0xFF;
            pOutBuffer[ret++] = (MISC_PWM_PWM1_DVS_PERIOD_reg >>  0) & 0xFF;
            // register data: MISC_PWM_PWM1_DVS_PERIOD_reg
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM1_DVS_PERIOD_reg) >> 24) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM1_DVS_PERIOD_reg) >> 16) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM1_DVS_PERIOD_reg) >>  8) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(MISC_PWM_PWM1_DVS_PERIOD_reg) >>  0) & 0xFF;
            break;
        case RTK_PWM_RTICE_GET_DVS:
            PWM_INFO("%s RTK_PWM_RTICE_GET_DVS, functionId=%d \n", __func__, functionId);
            v_total = rtd_inl(PPOVERLAY_DV_total_reg);
#if IS_ENABLED(CONFIG_SUPPORT_SCALER)
            DVS_freq = pwm_get_disp_dclk()/(pwm_get_disp_htotal()*v_total);
#endif
            pOutBuffer[ret++] = (DVS_freq >> 0) & 0xFF;
            // register addr: PPOVERLAY_DV_total_reg
            pOutBuffer[ret++] = (PPOVERLAY_DV_total_reg >> 24) & 0xFF;
            pOutBuffer[ret++] = (PPOVERLAY_DV_total_reg >> 16) & 0xFF;
            pOutBuffer[ret++] = (PPOVERLAY_DV_total_reg >>  8) & 0xFF;
            pOutBuffer[ret++] = (PPOVERLAY_DV_total_reg >>  0) & 0xFF;
             // register addr: PPOVERLAY_DV_total_reg
            pOutBuffer[ret++] = (rtd_inl(PPOVERLAY_DV_total_reg) >> 24) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(PPOVERLAY_DV_total_reg) >> 16) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(PPOVERLAY_DV_total_reg) >>  8) & 0xFF;
            pOutBuffer[ret++] = (rtd_inl(PPOVERLAY_DV_total_reg) >>  0) & 0xFF;
            break;
        case RTK_PWM_RTICE_SET_PWM_DUTY:
            PWM_INFO("%s RTK_PWM_RTICE_SET_PWM_DUTY, functionId=%d \n", __func__, functionId);
            pwm_duty = pwm_duty + ((pParam[1] & 0xFF) << 8);
            pwm_duty = pwm_duty + ((pParam[2] & 0xFF) << 0);
            pwm_duty = (pwm_duty < 0)? 0: pwm_duty;
            pwm_duty = (pwm_duty >= pchip2->rtk_duty_max)? pchip2->rtk_duty_max: pwm_duty;
            pchip2->rtk_duty = pwm_duty;
            rtk_pwm_duty_w(pchip2, pchip2->rtk_duty);
            rtk_pwm_db_wb(pchip2);
            rtk_pwm_db_enable(pchip2, 0);
            rtk_pwm_db_wb(pchip2);
            break;
        case RTK_PWM_RTICE_SET_PWM_FREQ_W_DVS:
            PWM_INFO("%s RTK_PWM_RTICE_SET_PWM_FREQ_W_DVS, functionId=%d \n", __func__, functionId);
            /*{
                unsigned int frameRates = 0;
                unsigned int dvs_xtal = 0;
                dvs_xtal = PPOVERLAY_DVS_cnt_get_dvs_cnt(rtd_inl(PPOVERLAY_DVS_cnt_reg));
                frameRates = 2700000000UL / dvs_xtal;
                PWM_INFO("%s frameRates: %x\n", __func__, frameRates);
            }*/
            DVS_freq = DVS_freq + ((pParam[1] & 0xFF) << 0);
#if IS_ENABLED(CONFIG_SUPPORT_SCALER)
            v_total = pwm_get_disp_dclk()/(pwm_get_disp_htotal()*DVS_freq);
#endif
            PWM_INFO("%s DVS_freq: %d\n", __func__, DVS_freq);
            PWM_INFO("%s v_total: %x\n", __func__, v_total);
#if IS_ENABLED(CONFIG_SUPPORT_SCALER)
            rtk_pwm_set_freq(pchip2, pwm_get_disp_htotal(), (v_total-1)*100, 2, 0);
#endif
            break;
        case RTK_PWM_RTICE_SET_PWM_FREQ_W_DATA:
            PWM_INFO("%s RTK_PWM_RTICE_SET_PWM_FREQ_W_DATA, functionId=%d \n", __func__, functionId);
            pwm_freq = pwm_freq + ((pParam[1] & 0xFF) << 24);
            pwm_freq = pwm_freq + ((pParam[2] & 0xFF) << 16);
            pwm_freq = pwm_freq + ((pParam[3] & 0xFF) <<  8);
            pwm_freq = pwm_freq + ((pParam[4] & 0xFF) <<  0);
            pchip2->rtk_freq = pwm_freq / 4;
            PWM_INFO("%s pwm_freq: %x\n", __func__, pwm_freq);
            PWM_INFO("%s pchip2->rtk_freq: %d\n", __func__, pchip2->rtk_freq);
            //pchip2->rtk_freq = 100; //400Hz
            //pchip2->rtk_freq = 10; // 40Hz
            pchip2->rtk_freq_100times = pchip2->rtk_freq * 100;
            pchip2->rtk_freq_changed = true;
            pchip2->rtk_clk_sel = CLK_27MHZ;
            rtk_pwm_freq_w(pchip2);
            break;
        case RTK_PWM_RTICE_SET_DVS:
            PWM_INFO("%s RTK_PWM_RTICE_SET_DVS, functionId=%d \n", __func__, functionId);
            DVS_freq = DVS_freq + ((pParam[1] & 0xFF) << 0);
#if IS_ENABLED(CONFIG_SUPPORT_SCALER)
            v_total = pwm_get_disp_dclk()/(pwm_get_disp_htotal()*DVS_freq);
#endif
            PWM_INFO("%s DVS_freq: %d\n", __func__, DVS_freq);
            PWM_INFO("%s v_total: %x\n", __func__, v_total);
            // Disable double buffer
            rtd_outl(PPOVERLAY_Double_Buffer_CTRL_reg, 0x0);
            // Set free run
            rtd_part_outl(PPOVERLAY_Display_Timing_CTRL1_reg, 28, 26, 0);
            // Set DVS freq
            rtd_outl(PPOVERLAY_DV_total_reg, v_total);
            break;
        case RTK_PWM_RTICE_STOP:
            PWM_INFO("%s RTK_PWM_RTICE_STOP, functionId=%d \n", __func__, functionId);
            // Back to origin setting
            pchip2->rtk_clk_sel = D_CLK_DIV4;
            pchip2->rtk_duty = 252;
            pchip2->rtk_freq = 120;
            pchip2->rtk_freq_100times = pchip2->rtk_freq * 100;
#if IS_ENABLED(CONFIG_SUPPORT_SCALER)
            rtk_pwm_set_scaler_source(pwm_get_disp_htotal(), (0x8c9-1)*100, 2, 0);
#endif
            rtk_pwm_duty_w(pchip2, pchip2->rtk_duty);
            rtk_pwm_db_wb(pchip2);
            rtk_pwm_db_enable(pchip2, 0);
            rtk_pwm_db_wb(pchip2);
            break;
        case RTK_PWM_RTICE_GET_PCB_PANEL:
            #ifdef CONFIG_CUSTOMER_TV006
            PWM_INFO("%s RTK_PWM_RTICE_GET_PCB_PANEL, functionId=%d \n", __func__, functionId);
            // 0 => LED_CURRENT
            // 1 => 2CH_PHASE_SAME
            // 2 => 2CH_PHASE_DIFF
            pOutBuffer[ret++] = rtk_pwm_panel_mode_get();

            // get platform
            if (get_platform() == PLATFORM_K6HP) {
                PWM_INFO("%s K6HP\n", __func__);
                is_K6HP_platform = 1;
            }
            pOutBuffer[ret++] = is_K6HP_platform;

            // get EPK
            filp = pwm_open_file(GET_EPK_VERSION_PATH, O_RDONLY, 0);
            if(filp) {
                pwm_read_file(filp, 0, (void*)&buffer, sizeof(char)*100);
                pwm_file_close(filp);
            }
            else {
                PWM_ERR("PWM filp = NULL\n");
            }
            pwm_file_close(filp);
            PWM_INFO("%s %s\n", __func__, buffer);
            for(i = 0; buffer[i] != '\n'; i++) {
                pOutBuffer[ret++] = buffer[i];
                string_counter++;
                //PWM_INFO("%s buffer[%d] = %c\n", __func__, i, buffer[i]);
            }
            for(i = 0; (80 - string_counter) > 0; i++) {
                string_counter++;
                pOutBuffer[ret++] = ' ';
            }
            pOutBuffer[ret++] = '\n';

            // get panel type
            PWM_INFO("%s %s %s %s %s %s bLocalDimming = %d\n", __func__,
                                    webos_strToolOption.eBackLight,
                                    webos_strToolOption.eLEDBarType,
                                    webos_strToolOption.eModelInchType,
                                    webos_strToolOption.eModelToolType,
                                    webos_strToolOption.eModelModuleType,
                                    webos_strToolOption.bLocalDimming);
            for(i = 0; i < 8; i++) {
                pOutBuffer[ret++] = webos_strToolOption.eBackLight[i];
                //PWM_INFO("%s webos_strToolOption.eBackLight[%d] = %c\n", __func__, i, webos_strToolOption.eBackLight[i]);
            }
            for(i = 0; i < 4; i++) {
                pOutBuffer[ret++] = webos_strToolOption.eLEDBarType[i];
                //PWM_INFO("%s webos_strToolOption.eLEDBarType[%d] = %c\n", __func__, i, webos_strToolOption.eLEDBarType[i]);
            }
            for(i = 0; i < 4; i++) {
                pOutBuffer[ret++] = webos_strToolOption.eModelInchType[i];
                //PWM_INFO("%s webos_strToolOption.eModelInchType[%d] = %c\n", __func__, i, webos_strToolOption.eModelInchType[i]);
            }
            for(i = 0; i < 9; i++) {
                pOutBuffer[ret++] = webos_strToolOption.eModelToolType[i];
                //PWM_INFO("%s webos_strToolOption.eModelToolType[%d] = %c\n", __func__, i, webos_strToolOption.eModelToolType[i]);
            }
            for(i = 0; i < 6; i++) {
                pOutBuffer[ret++] = webos_strToolOption.eModelModuleType[i];
                //PWM_INFO("%s webos_strToolOption.eModelModuleType[%d] = %c\n", __func__, i, webos_strToolOption.eModelModuleType[i]);
            }
            pOutBuffer[ret++] = webos_strToolOption.bLocalDimming;
            // Debug
            //for(i = 0; i<ret; i++) {
            //    PWM_INFO("%s pOutBuffer[%d] = %c\n", __func__, i, pOutBuffer[i]);
            //}
            #endif
            break;
        default:
            break;
    }
    return ret;
}
EXPORT_SYMBOL(rwPanelTool);


