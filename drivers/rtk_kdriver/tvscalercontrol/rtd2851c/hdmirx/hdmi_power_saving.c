#include <mach/rtk_platform.h>
#include <mach/platform.h>
#include "hdmi_reg.h"
#include "hdmi_common.h"
#include "hdmi_vfe_config.h"

#include "hdmi_scdc.h"
#include "hdmi_reg_phy.h"   // for HDMI PHY
extern unsigned char g_hdmi_drv_socts_en;
extern HDMI_PORT_INFO_T hdmi_rx[HDMI_PORT_TOTAL_NUM];
HDMI_PS_ST hdmi_ps_st[HDMI_PORT_TOTAL_NUM];


struct semaphore sem_power_saving_ctrl;

void _init_power_saving_sem(void)   { sema_init(&sem_power_saving_ctrl, 1); }
void _lock_power_saving_sem(void)   { down(&sem_power_saving_ctrl); }
void _unlock_power_saving_sem(void) { up(&sem_power_saving_ctrl); }

unsigned int hdmi_power_saving_enable = 0;
unsigned int hdmi_power_saving_interlace_timing = 0;
unsigned int hdmi_power_saving_connected_port_if_not_hdmi_source = 1;
unsigned int hdmi_power_saving_hdmi_common = 1;
unsigned int hdmi_power_saving_video_pll = 1;
unsigned int hdmi_power_saving_sram = 1;
unsigned int hdmi_power_saving_test_mode = 0;
unsigned int hdmi_active_source = 0xF;

#define HDMI_PS_MODE_AUTO  0
#define HDMI_PS_MODE_CONTROLLED_BY_MAC  1
#define HDMI_PS_MODE_CONTROLLED_BY_FW   2

unsigned int hdmi_ps_mode = HDMI_PS_MODE_CONTROLLED_BY_FW;  // default ps mode
unsigned int hdmi_ps_phy_stable_time = 1;
unsigned int hdmi_exit_power_saving_debug = 0;

const char* _hdmi_power_saving_fsm_str(HDMI_POWER_SAVEING_FSM_T fsm)
{
    switch (fsm) {
    case PS_FSM_POWER_SAVING_OFF:       return "PS_OFF";
    case PS_FSM_MEASURE: return "PS_MEAS";
    case PS_FSM_POWER_SAVING_ON:         return "PS_ON";
    default:
    return "PS_UNKNOW";
    }
}

HDMI_PS_ST* newbase_hdmi_power_saving_get_status(unsigned char port)
{
    return (port < HDMI_PORT_TOTAL_NUM) ? &hdmi_ps_st[port] : NULL;
}

void newbase_hdmi_power_saving_init(void)
{
    _init_power_saving_sem();
    hdmi_active_source = 0xF;  // set active source
    memset(&hdmi_ps_st, 0, sizeof(hdmi_ps_st));   // reset internal PS parameter
}

void newbase_set_hdmi_power_saving_enable(void)
{
#ifdef CONFIG_RTK_KDRV_HDMI_ENABLE_DYNAMIC_POWER_SAVING
    if (g_hdmi_drv_socts_en)
    {
        hdmi_power_saving_enable = 0;
    }
    else
    {
        hdmi_power_saving_enable = 1;
    }
#endif
}

void newbase_hdmi_set_active_source(unsigned char port)
{
    _lock_power_saving_sem();
    if (hdmi_active_source != port)
        PWR_FLOW_INFO("HDMI[p%d] newbase_hdmi_set_active_source\n", port);
    hdmi_active_source = port;
    _unlock_power_saving_sem();
}

void newbase_hdmi_clr_active_source(unsigned char port)
{
    _lock_power_saving_sem();
    if (port == hdmi_active_source) {
        PWR_FLOW_INFO("HDMI[p%d] newbase_hdmi_clr_active_source\n", port);    
        hdmi_active_source = 0xF;
    }
    _unlock_power_saving_sem();
}

#define _is_active_source(port)   ((port==hdmi_active_source) ? 1 : 0)

void newbase_hdmi_reset_power_saving_state(unsigned char port)
{
    HDMI_PS_ST* p_ps = newbase_hdmi_power_saving_get_status(port);
    
    if (p_ps==NULL)
        return;

    PWR_FLOW_INFO("HDMI[p%d] reset power saving state\n", port);

    //_lock_power_saving_sem();  /* remove semaphore, will run in interrupt context */

    p_ps->ps_fsm = PS_FSM_POWER_SAVING_OFF;
    p_ps->ps_mode = HDMI_POWER_SAVEING_MODE_OFF;
    p_ps->signal_stable_cnt = 0;
    p_ps->ps_fsm_sub_state = 0;
    p_ps->ps_round = 0;

    lib_hdmi_phy_reset_power_saving_state(port);
    lib_hdmi_mac_reset_power_saving_mode(port);

    //_unlock_power_saving_sem();  /* remove semaphore, will run in interrupt context */
}

/*------------------------------------------------------------------
 * Func : newbase_hdmi_power_saving_mode_probe
 *
 * Desc : probe available power saving mode for a given HDMI port
 *
 * Para : [IN] port     : HDMI channel (0~N-1)
 *
 * Retn : Power Saving mode
 *------------------------------------------------------------------*/
HDMI_POWER_SAVEING_MODE_T newbase_hdmi_power_saving_mode_probe(unsigned char port)
{
    HDMIRX_PHY_STRUCT_T* p_phy_st = newbase_rxphy_get_status(port);
    unsigned int is_interlace = (GET_H_INTERLACE(port) != HDMI_IPCHECK_PROGRESSIVE) ? 1 : 0;

    if(p_phy_st==NULL)
        return HDMI_POWER_SAVEING_MODE_PHY_GET_STATUS_FAIL;

    if (lib_hdmi_is_hdmi_mode(port)==MODE_DVI)  // do not enter power saving if it is DVI signal
        return HDMI_POWER_SAVEING_MODE_OFF;

    if ((newbase_hdmi_get_vrr_enable(port) || newbase_hdmi_get_freesync_enable(port)|| newbase_hdmi_get_qms_enable(port)))
        return HDMI_POWER_SAVEING_MODE_OFF;

    if (is_interlace && hdmi_power_saving_interlace_timing==0) // interlace PS is not enabled
        return HDMI_POWER_SAVEING_MODE_OFF;

    //------------------------------------
    // PS mode selection
    //------------------------------------
    // HD14 :  TIMER / RONLY (depends on FW config)
    // HD20 :  RONLY
    // HD21 :  TIMER
    //------------------------------------
    if (p_phy_st->frl_mode == MODE_TMDS)
    {
        if (hdmi_rx[port].scramble_flag==0)  // HDMI 1.4
        {
            if (is_interlace)
                return HDMI_POWER_SAVEING_MODE_MAC_HW;
            else
                return (hdmi_ps_mode==HDMI_PS_MODE_CONTROLLED_BY_FW) ? HDMI_POWER_SAVEING_MODE_FW_RONLY
                                                                     : HDMI_POWER_SAVEING_MODE_MAC_HW;
        }
        else  // HDMI 2.0
        {
            if (is_interlace)
                return HDMI_POWER_SAVEING_MODE_VS_WIN_OPP;
            else
                return HDMI_POWER_SAVEING_MODE_FW_RONLY;     
        }
    }
    else
    {
        // Not sipport HDMI 2.1
        return HDMI_POWER_SAVEING_MODE_OFF;
    }
}

/*------------------------------------------------------------------
 * Func : newbase_hdmi_power_saving_check_condition
 *
 * Desc : check power saving condition of a given HDMI port
 *
 * Para : [IN] port     : HDMI channel (0~N-1)
 *
 * Retn :  0 : should not do power saving
 *         1 : ready for power saving
 *------------------------------------------------------------------*/
unsigned char newbase_hdmi_power_saving_check_condition(unsigned char port)
{
    HDMI_PS_ST* p_ps = newbase_hdmi_power_saving_get_status(port);
    HDMIRX_PHY_STRUCT_T* p_phy_st = newbase_rxphy_get_status(port);
    
    if (p_ps==NULL)
        return 0;

    // check HPD/5V/PS Enable
    if (hdmi_rx[port].cable_conn==0 || newbase_hdmi_get_hpd(port)==0 || hdmi_power_saving_enable==0)
    {
        p_ps->signal_stable_cnt = 0;
        return 0;
    }

    // check clock stable
    if (p_phy_st->frl_mode == MODE_TMDS)
    {
        if (!newbase_rxphy_is_clk_stable(port) || newbase_rxphy_get_clk(port) < VALID_MIN_CLOCK)
        {
            p_ps->signal_stable_cnt = 0;
            return 0;
        }
    }
    else
    {
        // FRL mode
#if 0    // Humm... we skip the check here because of in current version, the data align is start when setup video pll
        if (newbase_hdmi_hd21_is_data_align_lock(port)!=TRUE)
        {
            p_ps->signal_stable_cnt = 0;
            return 0;
        }
#endif
    }

    if (p_ps->signal_stable_cnt < HDMI_POWER_SAVING_MAX_STABLE_CNT)
    {
        if ((p_ps->signal_stable_cnt & 0x3F)==0)
        {
            PWR_FLOW_INFO("HDMI[p%d] FSM=%s (%d), Round=%d, power saving signal stable count...(%d) interlace=%d, dvi=%d\n",
                port, _hdmi_power_saving_fsm_str(p_ps->ps_fsm), p_ps->ps_fsm, p_ps->ps_round, p_ps->signal_stable_cnt,
                GET_H_INTERLACE(port) , (lib_hdmi_is_hdmi_mode(port)==MODE_DVI) ? 1 : 0);
        }

        p_ps->signal_stable_cnt++;
    }

    // Check Current Display Port
    if (_is_active_source(port)) 
    {
        if (hdmi_power_saving_connected_port_if_not_hdmi_source)
        {
            if (newbase_hdmi_is_dispsrc(SLR_MAIN_DISPLAY))
                return 0;   // do not power saving when HDMI is playing
        }
        else
            return 0;  // do not power saving if the port is connected
    }

    if (p_ps->ps_fsm==PS_FSM_POWER_SAVING_OFF && newbase_hdmi_power_saving_mode_probe(port)==HDMI_POWER_SAVEING_MODE_OFF)
        return 0;

    if(newbase_hdmi_is_arc_earc_working(port)) // Do not enter power saving if ARC/eARC is outputing sound.
        return 0;

    // Check wakeup condition
    if (p_ps->ps_wakeup_request)
    {
        PWR_EMG("HDMI[p%d] got wake up request = %x\n", port, p_ps->ps_wakeup_request);
        p_ps->ps_wakeup_request = 0;
        p_ps->signal_stable_cnt = 0;
        return 0;
    }

    //Check specified device
    if(newbase_hdmi_get_device_force_ps_off(port))
        return 0;

    // Check SPD info (if SPD is received already)
    if (hdmi_rx[port].spd_t.type_code && p_ps->signal_stable_cnt > HDMI_POWER_SAVING_MIN_STABLE_CNT)
        return 1;

    // Check Signal stable count
    if (p_ps->signal_stable_cnt >= HDMI_POWER_SAVING_MAX_STABLE_CNT)
        return 1;

    return 0;
}

/*------------------------------------------------------------------
 * Func : newbase_hdmi_power_saving_wakeup
 *
 * Desc : wake up a hdmi port from power saving
 *
 * Para : [IN] port     : HDMI channel (0~N-1)
 *
 * Retn :  0 : should not do power saving
 *         1 : ready for power saving
 *------------------------------------------------------------------*/
void newbase_hdmi_power_saving_wakeup(unsigned char port, unsigned char wakup_condition)
{
    HDMI_PS_ST* p_ps = newbase_hdmi_power_saving_get_status(port);

    if (p_ps==NULL)
        return ;

    p_ps->ps_wakeup_request = wakup_condition;
    PWR_FLOW_INFO("HDMI[p%d] set wake up request = %x\n", port, p_ps->ps_wakeup_request);

    if ((wakup_condition & PS_WAKEUP_IMMEDIATELY))
        newbase_hdmi_power_saving_handler(port);    // wake up immediately...
}

/*------------------------------------------------------------------
 * Func : newbase_hdmi_get_power_saving_state
 *
 * Desc : get power saving sate
 *
 * Para : [IN] port     : HDMI channel (0~N-1)
 *
 * Retn :  power saving state
 *------------------------------------------------------------------*/
HDMI_POWER_SAVEING_FSM_T newbase_hdmi_get_power_saving_state(unsigned char port)
{
    HDMI_PS_ST* p_ps = newbase_hdmi_power_saving_get_status(port);
    return (p_ps) ? p_ps->ps_fsm : PS_FSM_POWER_SAVING_OFF;
}

/*------------------------------------------------------------------
 * Func : newbase_hdmi_power_saving_send_event
 *
 * Desc : send event to HDMI RX PS controller
 *
 * Para : [IN] port     : HDMI channel (0~N-1)
 *        [IN] event    : PS event
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void newbase_hdmi_power_saving_send_event(unsigned char port, HDMI_PS_EVENT event)
{
    HDMI_PS_ST* p_ps = newbase_hdmi_power_saving_get_status(port);

    if (p_ps==NULL)
        return ;

    switch(event)
    {
    case HDMI_PS_EVENT_SIGNAL_UNSTABLE:  // signal unstable, reset signal stable count
        p_ps->signal_stable_cnt = 0;
        break;
    }
}

void newbase_hdmi_power_saving_dump_status(unsigned char port)
{
    lib_hdmi_mac_dump_power_saving_reg(port);
    lib_hdmi_phy_dump_pow_saving_reg(port);
}


void newbase_hdmi_power_saving_dump_data_status(unsigned char port)
{
    HDMI_PS_ST* p_ps = newbase_hdmi_power_saving_get_status(port);
    unsigned char RxStatus[2] = {0, 0};
    HDMI_HDCP_E hdcp_mode = HDCP_OFF;
    HDMI_HDCP_ST* p_hdcp = newbase_hdcp_get_hdcp_status(port);
    hdcp_mode = newbase_hdcp_get_auth_mode(port);
    lib_hdmi_hdcp22_get_rx_status(port, RxStatus);
    
    if (p_ps==NULL)
        return ;

    switch (hdcp_mode)
    {
    case HDCP14:
        PWR_FLOW_INFO("HDMI[p%d] FSM=%s (%d), Round=%d, RxStatus(%02x, %02x), hdcp_mode=%d, hdcp_enc=%d, phy_pwr_on=%d, bch error=%d, bch error cnt=%d, auth_count=%d, ri_cnt=%d\n",
            port, _hdmi_power_saving_fsm_str(p_ps->ps_fsm), p_ps->ps_fsm, p_ps->ps_round,
            RxStatus[0], RxStatus[1],
            hdcp_mode,
            (hdmi_rx[port].hdcp_status.hdcp_enc) ? 1 : 0,
            lib_hdmi_mac_phy_power_saving_status(port),
            hdmi_rx[port].bch_err_detect,
            hdmi_rx[port].bch_err_cnt,
            p_hdcp->hdcp14_auth_count,
            p_hdcp->hdcp14_ri_count);
        break;

    case HDCP22:
        PWR_FLOW_INFO("HDMI[p%d] FSM=%s (%d), Round=%d, RxStatus(%02x, %02x), hdcp_mode=%d, hdcp_enc=%d, phy_pwr_on=%d, bch error=%d, bch error cnt=%d, auth_count=%d\n",
            port, _hdmi_power_saving_fsm_str(p_ps->ps_fsm), p_ps->ps_fsm, p_ps->ps_round,
            RxStatus[0], RxStatus[1],
            hdcp_mode,
            (hdmi_rx[port].hdcp_status.hdcp_enc) ? 1 : 0,
            lib_hdmi_mac_phy_power_saving_status(port),
            hdmi_rx[port].bch_err_detect,
            hdmi_rx[port].bch_err_cnt,
            p_hdcp->hdcp2_auth_count);
        break;

    case NO_HDCP:
    case HDCP_OFF:
        PWR_FLOW_INFO("HDMI[p%d] FSM=%s (%d), Round=%d, RxStatus(%02x, %02x), hdcp_mode=%d, hdcp_enc=%d, phy_pwr_on=%d, bch error=%d, bch error cnt=%d\n",
            port, _hdmi_power_saving_fsm_str(p_ps->ps_fsm), p_ps->ps_fsm, p_ps->ps_round,
            RxStatus[0], RxStatus[1],
            hdcp_mode,
            (hdmi_rx[port].hdcp_status.hdcp_enc) ? 1 : 0,
            lib_hdmi_mac_phy_power_saving_status(port),
            hdmi_rx[port].bch_err_detect,
            hdmi_rx[port].bch_err_cnt);
    }
}

/*------------------------------------------------------------------
 * Func : newbase_hdmi_enter_power_saving
 *
 * Desc : put a given HDMI port to power saving state
 *
 * Para : [IN] port     : HDMI channel (0~N-1)
 *
 * Retn : 0 : success, others: failed
 *------------------------------------------------------------------*/
int newbase_hdmi_enter_power_saving(unsigned char port)
{
    HDMI_PS_ST* p_ps = newbase_hdmi_power_saving_get_status(port);
    HDMIRX_PHY_STRUCT_T* p_phy_st = newbase_rxphy_get_status(port);
    HDMI_POWER_SAVEING_MODE_T ps_mode = newbase_hdmi_power_saving_mode_probe(port);
    unsigned int ps_measure_stable_cnt;
    unsigned int ps_measure_vs2vs_cnt;
    unsigned int ps_measure_last_pixel2vs_count;

    if (p_ps==NULL)
        return 0;
        
    if (ps_mode == HDMI_POWER_SAVEING_MODE_OFF)
    {
        PWR_EMG("HDMI[p%d] enter power saving failed, no ps mode available\n", port);
        return -1;
    }

    newbase_hdmi_get_ps_measure_status(port, &ps_measure_vs2vs_cnt, &ps_measure_last_pixel2vs_count, &ps_measure_stable_cnt);

    //--------------------------------------------
    // Stop SCDC / CED
    //--------------------------------------------
    lib_hdmi_scdc_char_err_keep(port, 1);
    newbase_hdmi_scdc_hw_mask_enable(port, 1);

    //--------------------------------------------
    // Enter Power Saving
    //--------------------------------------------
    switch(ps_mode)
    {
    case HDMI_POWER_SAVEING_MODE_MAC_HW:
        lib_hdmi_mac_enter_power_saving_mode_hw_mac(port, p_phy_st->frl_mode,
                hdmi_rx[port].scramble_flag,
                (GET_H_INTERLACE(port) != HDMI_IPCHECK_PROGRESSIVE) ? HDMI_WIN_OPP_MODE_VS : HDMI_WIN_OPP_MODE_DE,   // Using VS mode if it is interlace
                newbase_hdcp_get_auth_mode(port),
                ps_measure_last_pixel2vs_count, ps_measure_vs2vs_cnt);
        break;

    case HDMI_POWER_SAVEING_MODE_FW_RONLY:
        lib_hdmi_mac_enter_power_saving_mode_fw_ronly(port, p_phy_st->frl_mode,
                hdmi_rx[port].scramble_flag, newbase_hdcp_get_auth_mode(port),
                ps_measure_last_pixel2vs_count, ps_measure_vs2vs_cnt);
        break;

    case HDMI_POWER_SAVEING_MODE_VS_WIN_OPP:
        lib_hdmi_mac_enter_power_saving_mode_vs_win_opp(port, p_phy_st->frl_mode,
                hdmi_rx[port].scramble_flag, newbase_hdcp_get_auth_mode(port),
                ps_measure_last_pixel2vs_count, ps_measure_vs2vs_cnt);
        break;

    case HDMI_POWER_SAVEING_MODE_PHY_GET_STATUS_FAIL:
    default:
        PWR_EMG("HDMI[p%d] enter power saving failed, get phy status fail(%d)\n", port, ps_mode);
        return -1;
    }

    //--------------------------------
    // SRAM Ligh Sleep
    //--------------------------------
    if (hdmi_power_saving_sram)
        lib_hdmi_mac_sram_light_sleep_enable(port, 1);   // enable mac light speep mode

    //--------------------------------
    // update Status
    //--------------------------------
    newbase_hdmi_power_saving_dump_status(port);
    p_ps->ps_fsm = PS_FSM_POWER_SAVING_ON;
    p_ps->ps_mode = ps_mode;
    p_ps->ps_fsm_sub_state = 0;
    return 0;
}

/*------------------------------------------------------------------
 * Func : newbase_hdmi_exit_power_saving
 *
 * Desc : make a given HDMI port out of power saving state
 *
 * Para : [IN] port     : HDMI channel (0~N-1)
 *
 * Retn : 0 : success, others: failed
 *------------------------------------------------------------------*/
int newbase_hdmi_exit_power_saving(unsigned char port)
{
    HDMI_PS_ST* p_ps = newbase_hdmi_power_saving_get_status(port);
    HDMIRX_PHY_STRUCT_T* p_phy_st = newbase_rxphy_get_status(port);

    if (p_ps==NULL)
        return 0;

    if (p_phy_st == NULL)
        return -1;

    //--------------------------------
    // Power on Video PLL
    //--------------------------------
    if (hdmi_power_saving_video_pll && _is_active_source(port))
        lib_hdmi_set_video_pll_power_up(port);

    //--------------------------------
    // SRAM Ligh Sleep Disable
    //--------------------------------
    lib_hdmi_mac_sram_light_sleep_enable(port, 0);   // disable mac sram light speep mode

    //--------------------------------------------
    // Stop Power Saving
    //--------------------------------------------
    switch(p_ps->ps_mode)
    {
    case HDMI_POWER_SAVEING_MODE_MAC_HW:
        lib_hdmi_mac_exit_power_saving_mode_hw_mac(port, p_phy_st->frl_mode,
                hdmi_rx[port].scramble_flag, newbase_hdcp_get_auth_mode(port));
        break;

    case HDMI_POWER_SAVEING_MODE_FW_RONLY:
        lib_hdmi_mac_exit_power_saving_mode_fw_ronly(port, p_phy_st->frl_mode,
                hdmi_rx[port].scramble_flag, newbase_hdcp_get_auth_mode(port));
        break;

    case HDMI_POWER_SAVEING_MODE_VS_WIN_OPP:
        lib_hdmi_mac_exit_power_saving_mode_vs_win_opp(port, p_phy_st->frl_mode,
                hdmi_rx[port].scramble_flag, newbase_hdcp_get_auth_mode(port));
        break;

    case HDMI_POWER_SAVEING_MODE_OFF:  // do nothing ...
        PWR_EMG("HDMI[p%d] exit power saving failed, HDMI_POWER_SAVEING_MODE_OFF is off ?????\n", port);
        break;

    default:
        PWR_EMG("HDMI[p%d] exit power saving failed, unknwon ps mode (%d), force reset power saving state\n", port, p_ps->ps_mode);
        lib_hdmi_phy_reset_power_saving_state(port);
        lib_hdmi_mac_reset_power_saving_mode(port);
        break;
    }

    //--------------------------------------------
    // restart scdc / ced / clear avmute
    //--------------------------------------------
    lib_hdmi_scdc_char_err_keep(port, 0);
    newbase_hdmi_scdc_hw_mask_enable(port, 0);
    lib_hdmi_clear_avmute(port);

    //--------------------------------------------
    // clear Power Saving
    //--------------------------------------------
    newbase_hdmi_power_saving_dump_status(port);
    p_ps->ps_fsm = PS_FSM_POWER_SAVING_OFF;
    p_ps->ps_mode = HDMI_POWER_SAVEING_MODE_OFF;
    p_ps->ps_fsm_sub_state = 0;

    return 0;
}

static unsigned int ps_msg_cnt = 0;
static HDMI_POWER_SAVEING_FSM_T m_last_ps_fsm[4] = {PS_FSM_POWER_SAVING_OFF, PS_FSM_POWER_SAVING_OFF, PS_FSM_POWER_SAVING_OFF, PS_FSM_POWER_SAVING_OFF};

void newbase_hdmi_power_saving_handler(unsigned char port)
{
    HDMI_PS_ST* p_ps = newbase_hdmi_power_saving_get_status(port);
    HDMIRX_PHY_STRUCT_T* p_phy_st;
    unsigned int ps_measure_stable_cnt;
    unsigned int ps_measure_vs2vs_cnt;
    unsigned int ps_measure_last_pixel2vs_count;

    if (p_ps==NULL)
        return ;

    p_phy_st = newbase_rxphy_get_status(port);
    if(p_phy_st == NULL)
    {
        PWR_EMG("power saving fail : newbase_rxphy_get_status() return null\n");
        return;
    }
    _lock_power_saving_sem();

    switch(p_ps->ps_fsm)
    {
    case PS_FSM_POWER_SAVING_OFF:

        if (newbase_hdmi_power_saving_check_condition(port))
        {
            p_ps->ps_fsm_sub_state++;

            // for functional verification only
            // when test mode is enabled, do not enter power saving immediately
            // keep print signal status for serval seconds then enter power saving state
            if (hdmi_power_saving_test_mode && p_ps->ps_fsm_sub_state < HDMI_POWER_SAVING_MIN_STABLE_CNT)
            {

                if ((p_ps->ps_fsm_sub_state & 0x3F)==0)
                    newbase_hdmi_power_saving_dump_data_status(port);
                break;
            }

            // enter power saving state
            HDMI_INFO("HDMI[p%d] FSM=%s (%d), Round=%d, Ready for power saving, start ps measure\n",
                port, _hdmi_power_saving_fsm_str(p_ps->ps_fsm), p_ps->ps_fsm, p_ps->ps_round);

            p_ps->ps_fsm = PS_FSM_MEASURE;  // enter ps measure state
            p_ps->ps_fsm_sub_state = 0;
            p_ps->ps_round++;
        }
        else
        {
            if (hdmi_power_saving_enable && (p_ps->ps_fsm_sub_state++ & 0x3F)==0 && p_ps->signal_stable_cnt)
            {
                PWR_FLOW_INFO("HDMI[p%d] FSM=%s (%d), Round=%d, wait power saving signal stable ...(%d)\n",
                    port, _hdmi_power_saving_fsm_str(p_ps->ps_fsm), p_ps->ps_fsm, p_ps->ps_round, p_ps->signal_stable_cnt);
            }
        }
        break;

    case PS_FSM_MEASURE:

        // check power saving condition
        if (newbase_hdmi_power_saving_check_condition(port)==0)
        {
            PWR_FLOW_INFO("HDMI[p%d] FSM=%s (%d), Round=%d, not ready for power saving, stop ps measure\n",
                port, _hdmi_power_saving_fsm_str(p_ps->ps_fsm), p_ps->ps_fsm, p_ps->ps_round);
            lib_hdmi_ps_measure_enable(port, 0);
            p_ps->ps_fsm = PS_FSM_POWER_SAVING_OFF;
            p_ps->ps_fsm_sub_state = 0;
            p_ps->signal_stable_cnt = 0;
            break;
        }

        newbase_hdmi_get_ps_measure_status(port, &ps_measure_vs2vs_cnt, &ps_measure_last_pixel2vs_count, &ps_measure_stable_cnt);

        // waiting for measure stable
        if (ps_measure_stable_cnt > HDMI_POWER_SAVING_MIN_PS_MEASURE_STABLE_CNT)
        {
            lib_hdmi_scdc_char_err_keep(port, 1);
            newbase_hdmi_scdc_hw_mask_enable(port, 1);

            PWR_FLOW_INFO("HDMI[p%d] FSM=%s (%d), Round=%d, ps measure stable interlace=%d, vs2vs_cnt=%08x, last_pixel2vs_count=%08x, ps_stable_cnt=%d\n",
                port, _hdmi_power_saving_fsm_str(p_ps->ps_fsm), p_ps->ps_fsm, p_ps->ps_round, GET_H_INTERLACE(port),
                ps_measure_vs2vs_cnt, ps_measure_last_pixel2vs_count, ps_measure_stable_cnt);

            if (newbase_hdmi_enter_power_saving(port)<0)
            {
                p_ps->ps_fsm = PS_FSM_POWER_SAVING_OFF;
                p_ps->ps_mode = HDMI_POWER_SAVEING_MODE_OFF;
                p_ps->ps_fsm_sub_state = 0;
                p_ps->signal_stable_cnt = 0;
            }
        }
        else
        {
            if (ps_measure_stable_cnt==0 || p_ps->ps_fsm_sub_state++ > HDMI_POWER_SAVING_MIN_STABLE_CNT)
            {
                PWR_FLOW_INFO("HDMI[p%d] FSM=%s (%d), Round=%d, wait ps measure stable timeout, stop power saving\n",
                    port, _hdmi_power_saving_fsm_str(p_ps->ps_fsm), p_ps->ps_fsm, p_ps->ps_round);

                lib_hdmi_ps_measure_enable(port, 0);
                p_ps->ps_fsm = PS_FSM_POWER_SAVING_OFF;
                p_ps->ps_mode = HDMI_POWER_SAVEING_MODE_OFF;
                p_ps->ps_fsm_sub_state = 0;
                p_ps->signal_stable_cnt = 0;
            }
            else
            {
                if ((p_ps->ps_fsm_sub_state++ & 0x1F)==0)
                {
                    PWR_FLOW_INFO("HDMI[p%d] FSM=%s (%d), Round=%d, wait ps measure stable (ps_measure_stable_cnt=%d)\n",
                        port, _hdmi_power_saving_fsm_str(p_ps->ps_fsm), p_ps->ps_fsm, p_ps->ps_round, ps_measure_stable_cnt);
                }
            }
        }
        break;

    case PS_FSM_POWER_SAVING_ON:

        if (newbase_hdmi_power_saving_check_condition(port)==0)
        {
            PWR_FLOW_INFO("HDMI[p%d] FSM=%s (%d), Round=%d, not ready for power saving, stop power saving\n",
                port, _hdmi_power_saving_fsm_str(p_ps->ps_fsm), p_ps->ps_fsm, p_ps->ps_round);

            newbase_hdmi_exit_power_saving(port);
            p_ps->signal_stable_cnt = 0;
        }
        else
        {
            if ((p_ps->ps_fsm_sub_state++ & 0x7F)==0)
                newbase_hdmi_power_saving_dump_data_status(port);

            // for power saving test, when test mode enabled, keep prink signal status for a few second
            // to check the data correctness
            if (hdmi_power_saving_test_mode && p_ps->ps_fsm_sub_state >= HDMI_POWER_SAVING_MAX_STABLE_CNT)
            {
                PWR_FLOW_INFO("HDMI[p%d] FSM=%s (%d), Round=%d, hdmi_power_saving_test_mode is on, exit leave power saving\n",
                    port, _hdmi_power_saving_fsm_str(p_ps->ps_fsm), p_ps->ps_fsm, p_ps->ps_round);

                newbase_hdmi_exit_power_saving(port);
                //p_ps->signal_stable_cnt = 0;  // do not clear signal stable count...
            }
        }
        break;

    default:
        PWR_FLOW_INFO("HDMI[p%d] FSM=%s (%d), Round=%d, unknown poser state ???\n",
            port, _hdmi_power_saving_fsm_str(p_ps->ps_fsm), p_ps->ps_fsm, p_ps->ps_round);
    }

    //--------------------------------------------------------------
    // HDMI common PS
    //--------------------------------------------------------------
    if (hdmi_power_saving_hdmi_common)
    {
        unsigned char port_active = 0;

        //-------------------------------------
        // Get current port status
        //-------------------------------------
        if (hdmi_rx[port].cable_conn &&       // port is cable connected
            newbase_hdmi_get_hpd(port) &&     // port is hpd on connected
            p_ps->ps_fsm!=PS_FSM_POWER_SAVING_ON)  // port is not power saving
        {
            port_active = 1;
        }

        if (lib_hdmi_crt_is_hdmi_common_enabled())
        {
            //-------------------------------------
            // hdmi common power down check
            //-------------------------------------
            if (hdmi_active_source==0xF || (_is_active_source(port) && port_active==0))
            {
                PWR_EMG("HDMI[p%d] hdmi common not used, power down it\n", port);
                lib_hdmi_crt_off_hdmi_common();

                if (hdmi_power_saving_video_pll && _is_active_source(port))
                    lib_hdmi_set_video_pll_power_down(port);                   // Video PLL should be shut down after HDMI common power off
            }
        }
        else
        {
            //-------------------------------------
            // hdmi common power on check
            //-------------------------------------
            if (_is_active_source(port) && port_active)
            {
                PWR_EMG("HDMI[p%d] hdmi common is used, power on it\n", port);
                lib_hdmi_crt_reset_hdmi_common();
            }
        }
    }

    //PWR Print
    if(m_last_ps_fsm[port] != p_ps->ps_fsm)
    {
        PWR_EMG("[FSM] Port [%d], PWR_FSM=%d(%s) -> %d(%s)\n",
            port, m_last_ps_fsm[port], _hdmi_power_saving_fsm_str(m_last_ps_fsm[port]), p_ps->ps_fsm, _hdmi_power_saving_fsm_str(p_ps->ps_fsm));

        m_last_ps_fsm[port] = p_ps->ps_fsm;
        ps_msg_cnt = g_pwr_print_cycle_cnt_thd;
    }
    
    if(ps_msg_cnt++ >= g_pwr_print_cycle_cnt_thd)
    {
        if(hdmi_power_saving_enable)
        {
            PWR_EMG("PWR_STS : FG_HDMI=%d, P0/P1/P2/P3(active,fsm,vs_cnt,pol,int) =  P0(%d,%s,%d,%d,%d)/P1(%d,%s,%d,%d,%d)/P2(%d,%s,%d,%d,%d)\n",
                newbase_hdmi_is_dispsrc(SLR_MAIN_DISPLAY),
                _is_active_source(0), _hdmi_power_saving_fsm_str(hdmi_ps_st[0].ps_fsm), hdmi_rx[0].video_t.vs_cnt, hdmi_rx[0].video_t.vs_pol, GET_H_INTERLACE(0),
                _is_active_source(1), _hdmi_power_saving_fsm_str(hdmi_ps_st[1].ps_fsm), hdmi_rx[1].video_t.vs_cnt, hdmi_rx[1].video_t.vs_pol, GET_H_INTERLACE(1),
                _is_active_source(2), _hdmi_power_saving_fsm_str(hdmi_ps_st[2].ps_fsm), hdmi_rx[2].video_t.vs_cnt, hdmi_rx[2].video_t.vs_pol, GET_H_INTERLACE(2));
        }

        ps_msg_cnt = 0;
    }
    _unlock_power_saving_sem();
}

//------------------------------------------------------------------
// HDMI MAC low level Power Saving API
//------------------------------------------------------------------

unsigned char lib_hdmi_mac_phy_power_saving_status(unsigned char nport)
{
    if (hdmi_rx[nport].hdmi_2p1_en)
    {
        // not support HDMI 2.1, do nothing
        return 0;
    }
    else
        return HDMI_HD20_ps_st_get_phy_power_on_flag(hdmi_in(HDMI_HD20_ps_st_reg));
}

/*------------------------------------------------------------------
 * Func : lib_hdmi_mac_enter_power_saving_mode_hw_mac
 *
 * Desc : let HDMI mac to start power saving
 *        HDMI mode : 1.4 : OK
 *                    2.0 : not  (only valid on Rlane only mode)
 *                    2.1 : OK
 *        Winopp mode:
 *                  HDMI_WIN_OPP_MODE_VS : OK
 *                  HDMI_WIN_OPP_MODE_DE : OK
 *
 * Para : [IN] port      : HDMI channel (0~N-1)
 *        [IN] frl_mode  : 0 : TMDS, others: FRL
 *        [IN] scrambled : 0 : not scrambled, others: scrambled (HDMI2.0)
 *        [IN] win_opp_mode : 0 : VS mode, others: DE mode
 *        [IN] hdcp_mode : NO_HDCP / HDCP14 / HDCP22
 *        [IN] last_pixel2vs_tmds_cnt : time between DE failing to VS rising measured by PS measure
 *        [IN] vs2vs_cnt : vs period measured by PS measure
 *
 * Retn :  N/A
 *------------------------------------------------------------------*/
void lib_hdmi_mac_enter_power_saving_mode_hw_mac(
    unsigned char        nport,
    unsigned char        frl_mode,
    unsigned char        scrambled,
    HDMI_WIN_OPP_MODE_T  win_opp_mode,
    unsigned char        hdcp_mode,
    unsigned int         last_pixel2vs_tmds_cnt,
    unsigned int         vs2vs_cnt
    )
{
    unsigned int timer_cnt_end = vs2vs_cnt * 9 / 10;  // about 8 ms for 60Hz
    unsigned int hdmi_mode = lib_hdmi_is_hdmi_mode(nport);  // hw hdmi/dvi mode    

    PWR_EMG("HDMI[p%d] enter power saveing (HW-MAC) (hdmi_mode(Current/SR_reg)=%d/%d, frl_mode=%d, scrambled=%d, win_opp_mode=%d, hdcp_mode=%d, last_pixel2vs_tmds_cnt=%d, vs2vs_cnt=%d, timer_cnt_end=%d)\n",
            nport, hdmi_mode, lib_hdmi_get_hdmi_mode_reg(nport), frl_mode, scrambled, win_opp_mode, hdcp_mode, last_pixel2vs_tmds_cnt, vs2vs_cnt, timer_cnt_end);

    if (scrambled)
    {
        PWR_EMG("HDMI[p%d] enter power saveing (HW-MAC) failed, not support HDMI 2.0 signal\n", nport);
        return;
    }

    // Gating Data output first to prevent incorrect data to be sent to the data island
    if (frl_mode == MODE_TMDS)
        hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_mac_out_en_mask, 0); 
    else {
        // Not support HDMI 2.1, do nothing...
    }
        
    lib_hdmi_hdcp22_enable_reauth(nport, 0);

    lib_hdmi_set_hdirq_en(nport, 0); // disable hdirq before set to fw mode.
    hdmi_mask(HDMI_HDMI_SCR_reg, ~(HDMI_HDMI_SCR_mode_mask | HDMI_HDMI_SCR_msmode_mask),    //enable manual HDMI/DVI mode : need to use manual mode to prevent unstable data during PS
                                   HDMI_HDMI_SCR_mode_mask | HDMI_HDMI_SCR_msmode((hdmi_mode==MODE_HDMI) ? 1 : 0));

    lib_hdmi_set_colorspace_manual(nport, lib_hdmi_get_colorspace(nport));

    // set hdcp mode
    if (frl_mode == MODE_TMDS)
    {
        lib_hdmi_phy_enter_power_saving_mode(nport, HDMI_14, lib_hdmi_tmds_get_br_swap(nport));
        hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_timer_cnt_end_mask, HDMI_HD20_ps_ct_timer_cnt_end(timer_cnt_end));

        if (win_opp_mode==HDMI_WIN_OPP_MODE_VS)
        {   // using VS mode
            hdmi_out(HDMI_HDCP_CR2_reg, HDMI_HDCP_CR2_win_opp_mode(0));
            hdmi_out(HDMI_WIN_OPP_CTRL_reg, HDMI_WIN_OPP_CTRL_ctrl_r_only_sel(0) | HDMI_WIN_OPP_CTRL_opp_vs_debounce_en(1) | HDMI_WIN_OPP_CTRL_opp_vs_debounce_num(0x200));
        }
        else
        {   // using DE mode
            hdmi_out(HDMI_HDCP_CR2_reg, HDMI_HDCP_CR2_win_opp_mode(1) | HDMI_HDCP_CR2_enc_window_de2vs(last_pixel2vs_tmds_cnt));
            hdmi_out(HDMI_WIN_OPP_CTRL_reg, HDMI_WIN_OPP_CTRL_ctrl_r_only_sel(1) | HDMI_WIN_OPP_CTRL_opp_vs_debounce_en(0) | HDMI_WIN_OPP_CTRL_opp_vs_debounce_num(0x200));
        }

        hdmi_mask(HDMI_HDCP_HOA_reg, ~HDMI_HDCP_HOA_enc_method_mask, HDMI_HDCP_HOA_enc_method(1));
    }
    else
    {
        // not support HDMI 2.1, do nothing
    }

    lib_hdmi_bch_error_irq_en(nport, 0); // disable bch error irq

    // Set Power Saving mode by HDCP mode
    if (frl_mode == MODE_TMDS)
    {
        switch(hdcp_mode)
        {
        case NO_HDCP:
            hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_hdcp_mode_mask, HDMI_HD20_ps_ct_hdcp_mode(0));
            break;
        case HDCP14:
            hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_hdcp_mode_mask, HDMI_HD20_ps_ct_hdcp_mode(1));
            break;
        case HDCP22:
            hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_hdcp_mode_mask, HDMI_HD20_ps_ct_hdcp_mode(2));
            break;
        }
    }
    else
    {
	// Not sipport HDMI 2.1... ignore it
    }

    // Start Power Saving mode
    if (frl_mode == MODE_TMDS)
        hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_ps_mode_mask, HDMI_HD20_ps_ct_ps_mode(1));
    else
    {
        // not support HDMI 2.1, do nothing
    }
}


void lib_hdmi_mac_exit_power_saving_mode_hw_mac(unsigned char nport, unsigned char frl_mode, unsigned char scrambled, unsigned char hdcp_mode)
{
    int i;
    int bch_err_cnt = 0;

    PWR_EMG("HDMI[p%d] exit power saveing (HW-MAC) (hdmi_mode=%d, frl_mode=%d, scrambled=%d, hdcp_mode=%d)\n",
            nport, lib_hdmi_is_hdmi_mode(nport), frl_mode, scrambled, hdcp_mode);

    //--------------------------------------
    // Stop PS
    //--------------------------------------
    if (frl_mode  == MODE_TMDS)
    {
        hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_ps_mode_mask, HDMI_HD20_ps_ct_ps_mode(0));
        lib_hdmi_phy_exit_power_saving_mode(nport, HDMI_14, lib_hdmi_tmds_get_br_swap(nport));
    }
    else
    {
        // not support HDMI 2.1, do nothing
    }

    udelay(100);  // Important!!! wait for phy clock stable (some status bit can not work properly before phy clock stable)

    //--------------------------------------
    // wait TMDS CH align
    //--------------------------------------
    if (frl_mode == MODE_TMDS)
    {
        unsigned int timeout = 1000;

        hdmi_mask(HDMI_CH_SR_reg, ~HDMI_CH_SR_rgb_de_align_flag_mask, HDMI_CH_SR_rgb_de_align_flag_mask);

        while(HDMI_CH_SR_get_rgb_de_align_flag(hdmi_in(HDMI_CH_SR_reg))==0 && timeout--)
            udelay(100);

        PWR_FLOW_INFO("HDMI[p%d] rgb_de_align detected 1111 (%08x=%08x)\n", nport, HDMI_CH_SR_reg, hdmi_in(HDMI_CH_SR_reg));

        hdmi_mask(HDMI_CH_SR_reg, ~HDMI_CH_SR_rgb_de_align_flag_mask, HDMI_CH_SR_rgb_de_align_flag_mask);

        timeout = 100;

        while(HDMI_CH_SR_get_rgb_de_align_flag(hdmi_in(HDMI_CH_SR_reg))==0 && timeout--)
            udelay(100);

        PWR_FLOW_INFO("HDMI[p%d] rgb_de_align detected 2222 (%08x=%08x)\n", nport, HDMI_CH_SR_reg, hdmi_in(HDMI_CH_SR_reg));
    }

    //--------------------------------------
    // Dump Error status before
    //--------------------------------------
    if (hdmi_exit_power_saving_debug)
    {
        for(i=0; i<10; i++)
        {
            unsigned char RxStatus[2];

            lib_hdmi_hdcp22_get_rx_status(nport, RxStatus);

            PWR_FLOW_INFO("HDMI[p%d] before mac out, RxStatus(%02x, %02x), bch error=%d/%d, enc_toggle=%d\n",
                nport, RxStatus[0], RxStatus[1],
                lib_hdmi_get_bch_1bit_error(nport),
                lib_hdmi_get_bch_1bit_error(nport),
                lib_hdmi_hdcp_get_enc_toggle(nport));

            if (lib_hdmi_hdcp_get_enc_toggle(nport))
                lib_hdmi_hdcp_clear_enc_toggle(nport);

            msleep(10);
        }
    }

    if (frl_mode == MODE_TMDS)
    {
        //--------------------------------------
        // wait Win Opp detect (max at least 2 frame)
        //--------------------------------------
        unsigned int timeout = 1000;          // Important!!! the max wait time should at least 2 frame
        unsigned int win_opp_det_cnt = 0;          // Important!!! the max wait time should at least 2 frame

        hdmi_mask(HDMI_HD20_ps_st_reg, ~HDMI_HD20_ps_st_win_opp_detect_mask, HDMI_HD20_ps_st_win_opp_detect_mask);

        while(win_opp_det_cnt < 2 && timeout--)
        {
            if (HDMI_HD20_ps_st_get_win_opp_detect(hdmi_in(HDMI_HD20_ps_st_reg)))
            {
                win_opp_det_cnt++;
                hdmi_mask(HDMI_HD20_ps_st_reg, ~HDMI_HD20_ps_st_win_opp_detect_mask, HDMI_HD20_ps_st_win_opp_detect_mask);  // clear win opp detect
            }

            udelay(100);
        }
        PWR_FLOW_INFO("HDMI[p%d] Win opp detected cnt=%d, time=%d\n", nport, win_opp_det_cnt, 1000 - timeout);

        //--------------------------------------
        // Enable MAC output
        //--------------------------------------
        hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_mac_out_en_mask, HDMI_HD20_ps_ct_mac_out_en_mask);        // enable macout
    }
    else
    {
        // not support HDMI 2.1, do nothing
    }

    lib_hdmi_hdcp_set_win_opp_mode(nport, 0);   // change win opp mode to VS mode
    //msleep(50); // wait at least one frame
    lib_hdmi_hdcp22_enable_reauth(nport, 1);
    lib_hdmi_set_colorspace_auto(nport);        // change color space detection back to HW auto mode

    //--------------------------------------
    // Dump Error status after
    //--------------------------------------
    if (hdmi_exit_power_saving_debug || (frl_mode != MODE_TMDS))
    {
        for(i=0; i<30; i++)
        {
            unsigned char RxStatus[2];

            if (lib_hdmi_get_bch_2bit_error(nport))
                bch_err_cnt++;

            lib_hdmi_hdcp22_get_rx_status(nport, RxStatus);

            PWR_FLOW_INFO("HDMI[p%d] after mac out, RxStatus(%02x, %02x), bch error(1bit/2bit)=%d/%d, bch_err_cnt=%d, enc_toggle=%d\n",
                nport, RxStatus[0], RxStatus[1],
                lib_hdmi_get_bch_1bit_error(nport),
                lib_hdmi_get_bch_2bit_error(nport),
                bch_err_cnt,
                lib_hdmi_hdcp_get_enc_toggle(nport));

            if (lib_hdmi_get_bch_1bit_error(nport))
                lib_hdmi_bch_1bit_error_clr(nport);   // clear bch 1bit error

            if (lib_hdmi_get_bch_2bit_error(nport))
                lib_hdmi_bch_2bit_error_clr(nport);   // clear bch 2bit error

            if (lib_hdmi_hdcp_get_enc_toggle(nport))
                lib_hdmi_hdcp_clear_enc_toggle(nport);

            msleep(10);
        }
    }
}

/*------------------------------------------------------------------
 * Func : lib_hdmi_mac_enter_power_saving_mode_fw_ronly
 *
 * Desc : let HDMI mac to start power saving (FW R-Only)
 *        HDMI mode : 1.4 : OK
 *                    2.0 : OK
 *                    2.1 : not OK (need 4 lane both on)
 *
 * Para : [IN] port      : HDMI channel (0~N-1)
 *        [IN] frl_mode  : 0 : TMDS, others: FRL
 *        [IN] scrambled : 0 : not scrambled, others: scrambled (HDMI2.0)
 *        [IN] hdcp_mode : NO_HDCP / HDCP14 / HDCP22
 *        [IN] last_pixel2vs_tmds_cnt : time between DE failing to VS rising measured by PS measure
 *        [IN] vs2vs_cnt : vs period measured by PS measure
 *
 * Retn :  N/A
 *------------------------------------------------------------------*/
void lib_hdmi_mac_enter_power_saving_mode_fw_ronly(
    unsigned char       nport,
    unsigned char       frl_mode,
    unsigned char       scrambled,
    unsigned char       hdcp_mode,
    unsigned int        last_pixel2vs_tmds_cnt,
    unsigned int        vs2vs_cnt
    )
{
    unsigned int timer_cnt_end = vs2vs_cnt * 9 / 10;  // about 8 ms for 60Hz
    unsigned int hdmi_mode = lib_hdmi_is_hdmi_mode(nport);  // hw hdmi/dvi mode

    PWR_EMG("HDMI[p%d] enter power saveing (FW R-Only) (hdmi_mode(Current/SR_reg)=%d/%d, frl_mode=%d, scrambled=%d, hdcp_mode=%d, last_pixel2vs_tmds_cnt=%d, vs2vs_cnt=%d, timer_cnt_end=%d)\n",
            nport, hdmi_mode, lib_hdmi_get_hdmi_mode_reg(nport), frl_mode, scrambled, hdcp_mode, last_pixel2vs_tmds_cnt, vs2vs_cnt, timer_cnt_end);

    if (frl_mode!=MODE_TMDS) {
        PWR_EMG("HDMI[p%d] enter power saveing (FW R-Only) failed, not support HDMI 2.1 signal...\n", nport);
        return ;
    }

    // Gating Data output first to prevent incorrect data to be sent to the data island
    hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_mac_out_en_mask, 0);

    lib_hdmi_hdcp22_enable_reauth(nport, 0);

    lib_hdmi_set_hdirq_en(nport, 0); // disable hdirq before set to fw mode.
    hdmi_mask(HDMI_HDMI_SCR_reg, ~(HDMI_HDMI_SCR_mode_mask | HDMI_HDMI_SCR_msmode_mask),    //enable manual HDMI/DVI mode : need to use manual mode to prevent unstable data during PS
                                   HDMI_HDMI_SCR_mode_mask | HDMI_HDMI_SCR_msmode((hdmi_mode==MODE_HDMI) ? 1 : 0));

    lib_hdmi_set_colorspace_manual(nport, lib_hdmi_get_colorspace(nport));

    // set hdcp mode
    hdmi_out(HDMI_HDCP_CR2_reg, HDMI_HDCP_CR2_win_opp_mode(1) | HDMI_HDCP_CR2_enc_window_de2vs(last_pixel2vs_tmds_cnt));
    hdmi_out(HDMI_WIN_OPP_CTRL_reg, HDMI_WIN_OPP_CTRL_ctrl_r_only_sel(1) | HDMI_WIN_OPP_CTRL_opp_vs_debounce_en(0) | HDMI_WIN_OPP_CTRL_opp_vs_debounce_num(0x200));

    lib_hdmi_bch_error_irq_en(nport, 0); // disable bch error irq

    // Set Power Saving mode by HDCP mode
    switch(hdcp_mode)
    {
    case NO_HDCP:
        hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_hdcp_mode_mask, HDMI_HD20_ps_ct_hdcp_mode(0));
        break;
    case HDCP14:
        hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_hdcp_mode_mask, HDMI_HD20_ps_ct_hdcp_mode(1));
        break;
    case HDCP22:
        hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_hdcp_mode_mask, HDMI_HD20_ps_ct_hdcp_mode(2));
        break;
    }

    // Start Power Saving mode
    hdmi_mask(HDMI_HDMI_VCR_reg, ~HDMI_HDMI_VCR_bg_gck_mask, HDMI_HDMI_VCR_bg_gck(1));
    lib_hdmi_phy_enter_power_saving_mode(nport, HDMI_20, lib_hdmi_tmds_get_br_swap(nport));   // for HD20 signal, phy should be setting after mac ready
}

void lib_hdmi_mac_exit_power_saving_mode_fw_ronly(unsigned char nport, unsigned char frl_mode, unsigned char scrambled, unsigned char hdcp_mode)
{
    int i;
    int bch_err_cnt = 0;
    unsigned int timeout;
    unsigned int win_opp_det_cnt = 0;

    PWR_EMG("HDMI[p%d] exit power saveing (FW R-Only) (hdmi_mode=%d, frl_mode=%d, scrambled=%d, hdcp_mode=%d)\n",
            nport, lib_hdmi_is_hdmi_mode(nport), frl_mode, scrambled, hdcp_mode);

    lib_hdmi_phy_exit_power_saving_mode(nport, HDMI_20, lib_hdmi_tmds_get_br_swap(nport));

    //--------------------------------------
    // Disable BG clock Gating
    //--------------------------------------
    hdmi_mask(HDMI_HDMI_VCR_reg, ~HDMI_HDMI_VCR_bg_gck_mask, 0);

    //--------------------------------------
    // wait TMDS CH align
    //--------------------------------------
    timeout = 100;

    hdmi_mask(HDMI_CH_SR_reg, ~HDMI_CH_SR_rgb_de_align_flag_mask, HDMI_CH_SR_rgb_de_align_flag_mask);

    while(HDMI_CH_SR_get_rgb_de_align_flag(hdmi_in(HDMI_CH_SR_reg))==0 && timeout--)
        udelay(100);

    PWR_FLOW_INFO("HDMI[p%d] rgb_de_align detected 1111 (%08x=%08x)\n", nport, HDMI_CH_SR_reg, hdmi_in(HDMI_CH_SR_reg));

    hdmi_mask(HDMI_CH_SR_reg, ~HDMI_CH_SR_rgb_de_align_flag_mask, HDMI_CH_SR_rgb_de_align_flag_mask);

    timeout = 100;

    while(HDMI_CH_SR_get_rgb_de_align_flag(hdmi_in(HDMI_CH_SR_reg))==0 && timeout--)
        udelay(100);

    PWR_FLOW_INFO("HDMI[p%d] rgb_de_align detected 2222 (%08x=%08x)\n", nport, HDMI_CH_SR_reg, hdmi_in(HDMI_CH_SR_reg));

    //--------------------------------------
    // Dump Error status before macout
    //--------------------------------------
    if (hdmi_exit_power_saving_debug)
    {
        for(i=0; i<10; i++)
        {
            unsigned char RxStatus[2];

            lib_hdmi_hdcp22_get_rx_status(nport, RxStatus);

            PWR_FLOW_INFO("HDMI[p%d] before mac out, RxStatus(%02x, %02x), bch error=%d/%d, enc_toggle=%d\n",
                nport, RxStatus[0], RxStatus[1],
                lib_hdmi_get_bch_1bit_error(nport),
                lib_hdmi_get_bch_1bit_error(nport),
                lib_hdmi_hdcp_get_enc_toggle(nport));

            if (lib_hdmi_hdcp_get_enc_toggle(nport))
                lib_hdmi_hdcp_clear_enc_toggle(nport);

            msleep(10);
        }
    }

    //--------------------------------------
    // wait Win Opp detect (wait for 2 frame)
    //--------------------------------------
    timeout = 1000;
    win_opp_det_cnt = 0;
    hdmi_mask(HDMI_HD20_ps_st_reg, ~HDMI_HD20_ps_st_win_opp_detect_mask, HDMI_HD20_ps_st_win_opp_detect_mask);

    while(win_opp_det_cnt < 2 && timeout--)
    {
        if (HDMI_HD20_ps_st_get_win_opp_detect(hdmi_in(HDMI_HD20_ps_st_reg)))
        {
            win_opp_det_cnt++;
            hdmi_mask(HDMI_HD20_ps_st_reg, ~HDMI_HD20_ps_st_win_opp_detect_mask, HDMI_HD20_ps_st_win_opp_detect_mask);  // clear win opp detect
        }

        udelay(100);
    }
    PWR_FLOW_INFO("HDMI[p%d] Win opp detected cnt=%d, time=%d\n", nport, win_opp_det_cnt, 1000 - timeout);

    //--------------------------------------
    // Enable MAC output
    //--------------------------------------
    hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_mac_out_en_mask, HDMI_HD20_ps_ct_mac_out_en_mask);        // enable macout

    //--------------------------------------
    // reset MISC setting
    //--------------------------------------
    lib_hdmi_hdcp_set_win_opp_mode(nport, 0);   // change win opp mode to VS mode
    //msleep(50); // wait at least one frame
    lib_hdmi_hdcp22_enable_reauth(nport, 1);
    lib_hdmi_set_colorspace_auto(nport);        // change color space detection back to HW auto mode

    //--------------------------------------
    // Dump Error status after macout
    //--------------------------------------
    if (hdmi_exit_power_saving_debug)
    {
        for(i=0; i<30; i++)
        {
            unsigned char RxStatus[2];

            if (lib_hdmi_get_bch_2bit_error(nport))
                bch_err_cnt++;

            lib_hdmi_hdcp22_get_rx_status(nport, RxStatus);

            PWR_FLOW_INFO("HDMI[p%d] after mac out, RxStatus(%02x, %02x), bch error(1bit/2bit)=%d/%d, bch_err_cnt=%d, enc_toggle=%d\n",
                nport, RxStatus[0], RxStatus[1],
                lib_hdmi_get_bch_1bit_error(nport),
                lib_hdmi_get_bch_2bit_error(nport),
                bch_err_cnt,
                lib_hdmi_hdcp_get_enc_toggle(nport));

            if (lib_hdmi_get_bch_1bit_error(nport))
                lib_hdmi_bch_1bit_error_clr(nport);   // clear bch 1bit error

            if (lib_hdmi_get_bch_2bit_error(nport))
                lib_hdmi_bch_2bit_error_clr(nport);   // clear bch 2bit error

            if (lib_hdmi_hdcp_get_enc_toggle(nport))
                lib_hdmi_hdcp_clear_enc_toggle(nport);

            msleep(10);
        }
    }
}

/*------------------------------------------------------------------
 * Func : lib_hdmi_mac_enter_power_saving_mode_vs_win_opp
 *
 * Desc : let HDMI mac to start power saving (HDMI2.0 VS Win OPP mode)
 *        Support : HDMI 1.4 : not OK
 *                  HDMI 2.0 : OK
 *                  HDMI 2.1 : not OK
 *
 * Para : [IN] port      : HDMI channel (0~N-1)
 *        [IN] frl_mode  : 0 : TMDS, others: FRL
 *        [IN] scrambled : 0 : not scrambled, others: scrambled (HDMI2.0)
 *        [IN] hdcp_mode : NO_HDCP / HDCP14 / HDCP22
 *        [IN] last_pixel2vs_tmds_cnt : time between DE failing to VS rising measured by PS measure
 *        [IN] vs2vs_cnt : vs period measured by PS measure
 *
 * Retn :  N/A
 *------------------------------------------------------------------*/
void lib_hdmi_mac_enter_power_saving_mode_vs_win_opp(
    unsigned char       nport,
    unsigned char       frl_mode,
    unsigned char       scrambled,
    unsigned char       hdcp_mode,
    unsigned int        last_pixel2vs_tmds_cnt,
    unsigned int        vs2vs_cnt
    )
{
    unsigned int timer_cnt_end = vs2vs_cnt * 9 / 10;  // about 8 ms for 60Hz
    unsigned int hdmi_mode = lib_hdmi_is_hdmi_mode(nport);  // hw hdmi/dvi mode
    unsigned int ps_pwroff_cnt = 0;
    unsigned int timeout;

    PWR_EMG("HDMI[p%d] enter power saveing vs_win_opp (hdmi_mode(Current/SR_reg)=%d/%d, frl_mode=%d, scrambled=%d, hdcp_mode=%d, last_pixel2vs_tmds_cnt=%d, vs2vs_cnt=%d, timer_cnt_end=%d)\n",
            nport, hdmi_mode, lib_hdmi_get_hdmi_mode_reg(nport), frl_mode, scrambled, hdcp_mode, last_pixel2vs_tmds_cnt, vs2vs_cnt, timer_cnt_end);

    if (scrambled==0 || frl_mode)
    {
        PWR_EMG("HDMI[p%d] enter power saveing (vs_win_opp) failed, HDMI mode is not HDMI 2.0 ...\n", nport);
        return;
    }

    // Gating Data output first to prevent incorrect data to be sent to the data island
    hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_mac_out_en_mask, 0);

    lib_hdmi_hdcp22_enable_reauth(nport, 0);

    lib_hdmi_set_hdirq_en(nport, 0); // disable hdirq before set to fw mode.
    hdmi_mask(HDMI_HDMI_SCR_reg, ~(HDMI_HDMI_SCR_mode_mask | HDMI_HDMI_SCR_msmode_mask),    //enable manual HDMI/DVI mode : need to use manual mode to prevent unstable data during PS
                                   HDMI_HDMI_SCR_mode_mask | HDMI_HDMI_SCR_msmode((hdmi_mode==MODE_HDMI) ? 1 : 0));

    lib_hdmi_set_colorspace_manual(nport, lib_hdmi_get_colorspace(nport));

    hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_timer_cnt_end_mask, HDMI_HD20_ps_ct_timer_cnt_end(timer_cnt_end));
    hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_ps_bg_mode_mask, HDMI_HD20_ps_ct_ps_bg_mode(1));

    // set hdcp mode
    hdmi_out(HDMI_HDCP_CR2_reg, HDMI_HDCP_CR2_win_opp_mode(0) | // win opp search from VS
                                HDMI_HDCP_CR2_enc_window_de2vs(last_pixel2vs_tmds_cnt));

    hdmi_out(HDMI_WIN_OPP_CTRL_reg, HDMI_WIN_OPP_CTRL_ctrl_r_only_sel(1) |
                                    HDMI_WIN_OPP_CTRL_opp_vs_debounce_en(1) | // enable VS debounce
                                    HDMI_WIN_OPP_CTRL_opp_vs_debounce_num(0x200));

    if (lib_hdmi_tmds_get_br_swap(nport))
    {
        hdmi_mask(HDMI_SC_ALIGN_B_reg, ~HDMI_SC_ALIGN_B_en_mask, 0);
        hdmi_mask(HDMI_SC_ALIGN_R_reg, ~HDMI_SC_ALIGN_R_en_mask, HDMI_SC_ALIGN_R_en_mask);  // where R is R lane
    }
    else
    {
        hdmi_mask(HDMI_SC_ALIGN_B_reg, ~HDMI_SC_ALIGN_B_en_mask, HDMI_SC_ALIGN_B_en_mask);  // where B is R lane
        hdmi_mask(HDMI_SC_ALIGN_R_reg, ~HDMI_SC_ALIGN_R_en_mask, 0);
    }

    hdmi_mask(HDMI_SCR_CTR_PER_LFSR_reg, ~HDMI_SCR_CTR_PER_LFSR_br_vs_win_opp_en_mask, HDMI_SCR_CTR_PER_LFSR_br_vs_win_opp_en(1));
    hdmi_mask(HDMI_CH_CR_reg,~(1<<25), 1<<25);   // AFIFO Align mode

    lib_hdmi_bch_error_irq_en(nport, 0); // disable bch error irq

    // Set Power Saving mode by HDCP mode
    switch(hdcp_mode)
    {
    case NO_HDCP:
        hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_hdcp_mode_mask, HDMI_HD20_ps_ct_hdcp_mode(0));
        break;
    case HDCP14:
        hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_hdcp_mode_mask, HDMI_HD20_ps_ct_hdcp_mode(1));
        break;
    case HDCP22:
        hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_hdcp_mode_mask, HDMI_HD20_ps_ct_hdcp_mode(2));
        break;
    }

    //-------------------------------------------------------
    // wait hdcp frame key update complete (at least 2 frame)
    //
    // purpose: make sure ...
    // 1) the PS parameters are updated to double buffer
    // 2) and the PS won't be enabled during frame key update (might cause problem)
    //
    // Note: ps_pwroff flag will be set after hdcp frame key update
    //       when hdcp mode is 0, this flag will be set when win_opp is detected
    //-------------------------------------------------------
    timeout = 1000;
    ps_pwroff_cnt = 0;
    hdmi_mask(HDMI_HD20_ps_st_reg, ~HDMI_HD20_ps_st_pwroff_mask, HDMI_HD20_ps_st_pwroff_mask);   // when key update complete, ps pwroff flag will be set

    while(ps_pwroff_cnt < 2 && timeout--)
    {
        if (HDMI_HD20_ps_st_get_pwroff(hdmi_in(HDMI_HD20_ps_st_reg)))
        {
            ps_pwroff_cnt++;
            hdmi_mask(HDMI_HD20_ps_st_reg, ~HDMI_HD20_ps_st_pwroff_mask, HDMI_HD20_ps_st_pwroff_mask);  // clear ps pwroff flag
        }

        udelay(100);
    }
    PWR_FLOW_INFO("HDMI[p%d] ps power off detected cnt=%d, time=%d\n", nport, ps_pwroff_cnt, 1000 - timeout);

    // Start Power Saving mode
    hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_ps_mode_mask, HDMI_HD20_ps_ct_ps_mode(1));
    lib_hdmi_phy_enter_power_saving_mode(nport, HDMI_20_VS_WIN_OPP, lib_hdmi_tmds_get_br_swap(nport));   // for HD20 signal, phy should be setting after mac ready

#if 0   // for Power saving status debug
    hdmi_out(HDMI_SCR_CTR_PER_LFSR_reg, hdmi_in(HDMI_SCR_CTR_PER_LFSR_reg));
    hdmi_out(HDMI_PHY_FIFO_SR0_reg, hdmi_in(HDMI_PHY_FIFO_SR0_reg));

    for (timer_cnt_end=0; timer_cnt_end<128; timer_cnt_end++)
    {
        unsigned int val = hdmi_in(HDMI_PHY_FIFO_SR0_reg);
        unsigned int val2 = hdmi_in(HDMI_SCR_CTR_PER_LFSR_reg);

        hdmi_out(HDMI_SCR_CTR_PER_LFSR_reg, hdmi_in(HDMI_SCR_CTR_PER_LFSR_reg));
        hdmi_out(HDMI_PHY_FIFO_SR0_reg, hdmi_in(HDMI_PHY_FIFO_SR0_reg));

        PWR_EMG("22222 HDMI[p%d] HDMI_PHY_FIFO_SR0_reg=%08x, HDMI_SCR_CTR_PER_LFSR_reg=%08x\n",
            nport, val, val2);
        udelay(1000);
    }
#endif
}

/*------------------------------------------------------------------
 * Func : lib_hdmi_mac_exit_power_saving_mode_vs_win_opp
 *
 * Desc : let HDMI mac to leave power saving (HDMI2.0 VS Win OPP mode)
 *
 * Para : [IN] port      : HDMI channel (0~N-1)
 *        [IN] frl_mode  : 0 : TMDS, others: FRL
 *        [IN] scrambled : 0 : not scrambled, others: scrambled (HDMI2.0)
 *        [IN] hdcp_mode : NO_HDCP / HDCP14 / HDCP22
 *
 * Retn :  N/A
 *------------------------------------------------------------------*/
void lib_hdmi_mac_exit_power_saving_mode_vs_win_opp(
    unsigned char       nport,
    unsigned char       frl_mode,
    unsigned char       scrambled,
    unsigned char       hdcp_mode
    )
{
    int i;
    int bch_err_cnt = 0;
    unsigned int timeout;
    unsigned int win_opp_det_cnt = 0;

    PWR_EMG("HDMI[p%d] exit power saveing vs_win_opp (hdmi_mode=%d, frl_mode=%d, scrambled=%d, hdcp_mode=%d)\n",
            nport, lib_hdmi_is_hdmi_mode(nport), frl_mode, scrambled, hdcp_mode);

    hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_ps_mode_mask, HDMI_HD20_ps_ct_ps_mode(0));   // PS mode = 0
    lib_hdmi_phy_exit_power_saving_mode(nport, HDMI_20_VS_WIN_OPP, lib_hdmi_tmds_get_br_swap(nport));    

    //--------------------------------------
    // wait Win Opp detect (wait for 2 frame)
    //--------------------------------------
    timeout = 1000; 
    win_opp_det_cnt = 0;
    hdmi_mask(HDMI_HD20_ps_st_reg, ~HDMI_HD20_ps_st_win_opp_detect_mask, HDMI_HD20_ps_st_win_opp_detect_mask);

    while(win_opp_det_cnt < 2 && timeout--)
    {
        if (HDMI_HD20_ps_st_get_win_opp_detect(hdmi_in(HDMI_HD20_ps_st_reg)))
        {
            win_opp_det_cnt++;
            hdmi_mask(HDMI_HD20_ps_st_reg, ~HDMI_HD20_ps_st_win_opp_detect_mask, HDMI_HD20_ps_st_win_opp_detect_mask);  // clear win opp detect
        }

        udelay(100);
    }
    PWR_FLOW_INFO("HDMI[p%d] Win opp detected cnt=%d, time=%d\n", nport, win_opp_det_cnt, 1000 - timeout);

    //--------------------------------------
    // reset ps_bg mode register
    //--------------------------------------
    hdmi_mask(HDMI_CH_CR_reg,~(1<<25), 0);   // AFIFO Align mode

    hdmi_mask(HDMI_SCR_CTR_PER_LFSR_reg, ~HDMI_SCR_CTR_PER_LFSR_br_vs_win_opp_en_mask, 0);

    hdmi_mask(HDMI_SC_ALIGN_B_reg, ~HDMI_SC_ALIGN_B_en_mask, 0);
    hdmi_mask(HDMI_SC_ALIGN_R_reg, ~HDMI_SC_ALIGN_R_en_mask, 0);

    hdmi_out(HDMI_WIN_OPP_CTRL_reg, HDMI_WIN_OPP_CTRL_ctrl_r_only_sel(0) |
                                    HDMI_WIN_OPP_CTRL_opp_vs_debounce_en(0) | // enable VS debounce
                                    HDMI_WIN_OPP_CTRL_opp_vs_debounce_num(0x200));

    hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_ps_bg_mode_mask, 0); // ps_bg = 0

    //--------------------------------------
    // Enable MAC output
    //--------------------------------------
    hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_mac_out_en_mask, HDMI_HD20_ps_ct_mac_out_en_mask);        // enable macout

    //--------------------------------------
    // reset MISC setting
    //--------------------------------------
    lib_hdmi_hdcp_set_win_opp_mode(nport, 0);   // change win opp mode to VS mode
    lib_hdmi_hdcp22_enable_reauth(nport, 1);
    lib_hdmi_set_colorspace_auto(nport);        // change color space detection back to HW auto mode

    //--------------------------------------
    // Dump Error status after
    //--------------------------------------
    if (hdmi_exit_power_saving_debug)
    {
        for(i=0; i<30; i++)
        {
            unsigned char RxStatus[2];

            if (lib_hdmi_get_bch_2bit_error(nport))
                bch_err_cnt++;

            lib_hdmi_hdcp22_get_rx_status(nport, RxStatus);

            PWR_FLOW_INFO("HDMI[p%d] after mac out, RxStatus(%02x, %02x), bch error(1bit/2bit)=%d/%d, bch_err_cnt=%d, enc_toggle=%d\n",
                nport, RxStatus[0], RxStatus[1],
                lib_hdmi_get_bch_1bit_error(nport),
                lib_hdmi_get_bch_2bit_error(nport),
                bch_err_cnt,
                lib_hdmi_hdcp_get_enc_toggle(nport));

            if (lib_hdmi_get_bch_1bit_error(nport))
                lib_hdmi_bch_1bit_error_clr(nport);   // clear bch 1bit error

            if (lib_hdmi_get_bch_2bit_error(nport))
                lib_hdmi_bch_2bit_error_clr(nport);   // clear bch 2bit error

            if (lib_hdmi_hdcp_get_enc_toggle(nport))
                lib_hdmi_hdcp_clear_enc_toggle(nport);

            msleep(10);
        }
    }
}
void lib_hdmi_mac_reset_power_saving_mode(unsigned char nport)
{
    // reset HD20 mac ps state
    hdmi_mask(HDMI_HDMI_VCR_reg, ~HDMI_HDMI_VCR_bg_gck_mask, 0); // disable bg clock gating (HD20)
    hdmi_out(HDMI_HD20_ps_ct_reg, HDMI_HD20_ps_ct_mac_out_en_mask);  // enable mac output

    // reset ps_bg mode register
    hdmi_mask(HDMI_CH_CR_reg,~(1<<25), 0);   // AFIFO Align mode

    hdmi_mask(HDMI_SCR_CTR_PER_LFSR_reg, ~HDMI_SCR_CTR_PER_LFSR_br_vs_win_opp_en_mask, 0);

    hdmi_mask(HDMI_SC_ALIGN_B_reg, ~HDMI_SC_ALIGN_B_en_mask, 0);
    hdmi_mask(HDMI_SC_ALIGN_R_reg, ~HDMI_SC_ALIGN_R_en_mask, 0);

    hdmi_out(HDMI_WIN_OPP_CTRL_reg, HDMI_WIN_OPP_CTRL_ctrl_r_only_sel(0) |
                                    HDMI_WIN_OPP_CTRL_opp_vs_debounce_en(0) | // enable VS debounce
                                    HDMI_WIN_OPP_CTRL_opp_vs_debounce_num(0x200));

    hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_ps_bg_mode_mask, 0); // ps_bg = 0

    // HDCP related
    hdmi_out(HDMI_HDCP_CR2_reg, 0);
    hdmi_out(HDMI_HDCP_HOA_reg, 0);

    // SRAM Ligh Sleep
    lib_hdmi_mac_sram_light_sleep_enable(nport, 0);   // disable mac sram light speep mode
}

void lib_hdmi_mac_dump_power_saving_reg(unsigned char nport)
{
    if (hdmi_rx[nport].hdmi_2p1_en)
    {
        // Do nothing ... not support HDMI 2.1
    }
    else
    {
        PWR_FLOW_INFO("HDMI[p%d] mac setting : HD20_ps_ct=%08x (ps_mode=%d, hdcp_mode=%d, imer_cnt_end=%d, mac_out=%d, ps_bg_mode=%d), HD21_ps_st=%08x (phy_pwr_on=%d)\n",
            nport,
            hdmi_in(HDMI_HD20_ps_ct_reg),
            HDMI_HD20_ps_ct_get_ps_mode(hdmi_in(HDMI_HD20_ps_ct_reg)),
            HDMI_HD20_ps_ct_get_hdcp_mode(hdmi_in(HDMI_HD20_ps_ct_reg)),
            HDMI_HD20_ps_ct_get_timer_cnt_end(hdmi_in(HDMI_HD20_ps_ct_reg)),
            HDMI_HD20_ps_ct_get_mac_out_en(hdmi_in(HDMI_HD20_ps_ct_reg)),
            HDMI_HD20_ps_ct_get_ps_bg_mode(hdmi_in(HDMI_HD20_ps_ct_reg)),
            hdmi_in(HDMI_HD20_ps_st_reg),
            HDMI_HD20_ps_st_get_phy_power_on_flag(hdmi_in(HDMI_HD20_ps_st_reg)));
    }

    PWR_FLOW_INFO("HDMI[p%d] mac setting : TMDS_PWDCTL=%08x, iccaf=%d, TMDS_OUTCTL=%08x, aoe=%d\n",
        nport,
        hdmi_in(HDMI_TMDS_PWDCTL_reg),
        HDMI_TMDS_PWDCTL_get_iccaf(hdmi_in(HDMI_TMDS_PWDCTL_reg)),
        hdmi_in(HDMI_TMDS_OUTCTL_reg),
        HDMI_TMDS_OUTCTL_get_aoe(hdmi_in(HDMI_TMDS_OUTCTL_reg)));

    PWR_FLOW_INFO("HDMI[p%d] mac setting : HDMI_HDCP_CR2=%08x, win_opp_mode=%d, enc_window_de2vs=%08x\n",
        nport,
        hdmi_in(HDMI_HDCP_CR2_reg),
        HDMI_HDCP_CR2_get_win_opp_mode(hdmi_in(HDMI_HDCP_CR2_reg)),
        HDMI_HDCP_CR2_get_enc_window_de2vs(hdmi_in(HDMI_HDCP_CR2_reg)));

    PWR_FLOW_INFO("HDMI[p%d] mac setting : WIN_OPP_CTRL=%08x, r_only_sel=%d, vs_debounce_en=%d, vs_debounce_num=0x%x\n",
        nport,
        hdmi_in(HDMI_WIN_OPP_CTRL_reg),
        HDMI_WIN_OPP_CTRL_get_ctrl_r_only_sel(hdmi_in(HDMI_WIN_OPP_CTRL_reg)),
        HDMI_WIN_OPP_CTRL_get_opp_vs_debounce_en(hdmi_in(HDMI_WIN_OPP_CTRL_reg)),
        HDMI_WIN_OPP_CTRL_get_opp_vs_debounce_num(hdmi_in(HDMI_WIN_OPP_CTRL_reg)));
}

//------------------------------------------------------------------------------
// Power Saving Control
//------------------------------------------------------------------------------

unsigned int hdmi_bypass_phy_ps_mode = 0;

void lib_hdmi_phy_reset_power_saving_state(unsigned char nport)
{
    PWR_EMG("HDMI[p%d] phy reset power saveing state\n", nport);

    if (hdmi_bypass_phy_ps_mode)
    {
        PWR_EMG("hdmi_bypass_phy_ps_mode is enabled, do nothing...\n");
        return ;
    }

    switch(nport)
    {
    case 0:
        // reset APHY
        hdmi_mask(GDIRX_HDMI20_PHY_P0_P0_R2_reg, ~GDIRX_HDMI20_PHY_P0_P0_R2_reg_p0_r_pow_saving_switch_enable_mask, GDIRX_HDMI20_PHY_P0_P0_R2_reg_p0_r_pow_saving_switch_enable_mask); //fast switch circuit enable
        hdmi_mask(GDIRX_HDMI20_PHY_P0_P0_G2_reg, ~GDIRX_HDMI20_PHY_P0_P0_G2_reg_p0_g_pow_saving_switch_enable_mask, GDIRX_HDMI20_PHY_P0_P0_G2_reg_p0_g_pow_saving_switch_enable_mask); //fast switch circuit enable
        hdmi_mask(GDIRX_HDMI20_PHY_P0_P0_B2_reg, ~GDIRX_HDMI20_PHY_P0_P0_B2_reg_p0_b_pow_saving_switch_enable_mask, GDIRX_HDMI20_PHY_P0_P0_B2_reg_p0_b_pow_saving_switch_enable_mask); //fast switch circuit enable

//        hdmi_mask(HDMIRX_2P0_PHY_P0_B4_reg, ~0x1, 1); //fast switch circuit enable
//        hdmi_mask(HDMIRX_2P0_PHY_P0_CK4_reg, ~(0x3<<2), (0x3<<2)); //fast switch circuit enable

        // DPHY
        // reset HD20 DPHY
        hdmi_mask(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_save_bypass_mask),GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_save_bypass_mask);    //reg_p0_hd14_pow_save_bypass bypass power saving (HDMI-1.4)
        hdmi_mask(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_on_manual_en_mask), 0x0);        //reg_p0_hd14_pow_on_manual_en power on cntrol by MAC  (HDMI-1.4)
        hdmi_mask(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_on_manual_mask), GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_on_manual_mask);    //reg_p0_hd14_pow_on_manual manual power setting = On (HDMI-1.4)

        // release reset
        hdmi_mask(GDIRX_HDMI20_PHY_P0_DCDR_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_r_dig_rst_n_mask), GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_r_dig_rst_n_mask);  //P0_R_DIG_RST_N fast switch reset p0 r
        hdmi_mask(GDIRX_HDMI20_PHY_P0_DCDR_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_g_dig_rst_n_mask), GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_g_dig_rst_n_mask);  //P0_G_DIG_RST_N fast switch reset p0 g
        hdmi_mask(GDIRX_HDMI20_PHY_P0_DCDR_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_b_dig_rst_n_mask), GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_b_dig_rst_n_mask);  //P0_B_DIG_RST_N fast switch reset p0 b

        hdmi_mask(GDIRX_HDMI20_PHY_P0_DCDR_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_r_cdr_rst_n_mask), GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_r_cdr_rst_n_mask);  //P0_R_CDR_RST_N  fast switch reset p0 r        
        hdmi_mask(GDIRX_HDMI20_PHY_P0_DCDR_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_g_cdr_rst_n_mask),GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_g_cdr_rst_n_mask);  //P0_G_CDR_RST_N fast switch reset p0 g
        hdmi_mask(GDIRX_HDMI20_PHY_P0_DCDR_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_b_cdr_rst_n_mask), GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_b_cdr_rst_n_mask);  //P0_B_CDR_RST_N fast switch reset p0 b
        break;

    case 1:
        //PHY Setting
        //REG_TOP_IN_2[0]  0: select HDMI 1.4 fast switch signal  1: select HDMI 2.1 fast switch signal
        hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(1<<8), 0);
        //REG_TOP_IN_8<1>=1'b1
        hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(1<<9), (0<<9));//]port-0 fast switch signal delay enable bit
        hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(1<<9), (1<<9));
        //REG_TOP_IN_8<4:2>=1'b100
        hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(0x7<<10), (0x4<<10));

        //REG_P#_ACDR_*_8<1>=1'b1
        //REG_P#_ACDR_*_8<2>=1'b0 a 1
        //0x1800DA90[7][6]
        hdmi_mask(HDMIRX_PHY_r0_reg, ~(0x1<<6), 0);    //fast switch circuit disable
        hdmi_mask(HDMIRX_PHY_r0_reg, ~(0x1<<7), 0);    //reset delay cell dff

        // 0x1800DA70[7][6]
        hdmi_mask(HDMIRX_PHY_g0_reg, ~(0x1<<6), 0);    //fast switch circuit disable
        hdmi_mask(HDMIRX_PHY_g0_reg, ~(0x1<<7), 0);    //reset delay cell dff

        // 0x1800DA50[7][6]
        hdmi_mask(HDMIRX_PHY_b0_reg, ~(0x1<<6), 0);    //fast switch circuit disable
        hdmi_mask(HDMIRX_PHY_b0_reg, ~(0x1<<7), 0);    //reset delay cell dff

        // 0x1800DA30[7][6]
        hdmi_mask(HDMIRX_PHY_ck0_reg, ~(0x1<<6), 0);    //fast switch circuit disable
        hdmi_mask(HDMIRX_PHY_ck0_reg, ~(0x1<<7), 0);    //reset delay cell dff

        // DPHY
        // reset HD21 DPHY
	hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<11), 1<<11);   // Power on reg_p0_pow_save_bypass_ck by FW
	hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<10), 1<<10);   // Power on reg_p0_pow_save_bypass_r by FW
	hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<9), 1<<9);     // Power on reg_p0_pow_save_bypass_g by FW
	hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<8), 1<<8);    // Power onreg_p0_pow_save_bypass_b by FW
	hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<2), 0);       // reg_p0_pow_on_manual For MAC power saving, 1 : power on  0 : power off
	hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<7), 0<<7);   // reg_p0_pow_on_manual_ck_en
	hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<6), 0<<6);   // reg_p0_pow_on_manual_r_en
	hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<5), 0<<5);     //  reg_p0_pow_on_manual_g_en
	hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<4), 0<<4);    //  reg_p0_pow_on_manual_b_en    
       hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<3), 0<<3);    //  reg_p0_pow_on_manual_aphy_en Enable power save manual mode for APHY           

	//hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0xF<<4), (0xF<<4));   //[7]~[4]ck,r,g,b  manual power setting = On 
	hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<7), 1<<7);   // reg_p0_pow_on_manual_ck_en
	hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<6), 1<<6);   // reg_p0_pow_on_manual_r_en
	hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<5), 1<<5);     //  reg_p0_pow_on_manual_g_en
	hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<4), 1<<4);    //  reg_p0_pow_on_manual_b_en
       hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<3), 1<<3);    //  reg_p0_pow_on_manual_aphy_en Enable power save manual mode for APHY           

        // release reset
        hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<0), (1<<0));  // fast switch release reset p0 b Reset_n for all digital CDR of B lane
        hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<1), (1<<1));  // fast switch release reset p0 g Reset_n for all digital CDR of G lane
        hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<2), (1<<2));  // fast switch release reset p0 r Reset_n for all digital CDR of R lane

        hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<4), (1<<4));  // fast switch release reset p0 b Reset_n for digital CDR of B lane
        hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<5), (1<<5));  // fast switch release reset p0 g Reset_n for digital CDR of G lane
        hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<6), (1<<6));  // fast switch release reset p0 r Reset_n for digital CDR of R lane

        break;

    case 2:
        //PHY Setting
        //REG_TOP_IN_2[0]  0: select HDMI 1.4 fast switch signal  1: select HDMI 2.1 fast switch signal
        hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(1<<8), 0);
        //REG_TOP_IN_8<1>=1'b1
        hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(1<<9), (0<<9));//]port-0 fast switch signal delay enable bit
        hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(1<<9), (1<<9));
        //REG_TOP_IN_8<4:2>=1'b100
        hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(0x7<<10), (0x4<<10));

        //REG_P#_ACDR_*_8<1>=1'b1
        //REG_P#_ACDR_*_8<2>=1'b0 a 1
        //0x1800DA90[7][6]
        hdmi_mask(HDMIRX_PHY_r0_reg, ~(0x1<<6), 0);    //fast switch circuit disable
        hdmi_mask(HDMIRX_PHY_r0_reg, ~(0x1<<7), 0);    //reset delay cell dff

        // 0x1800DA70[7][6]
        hdmi_mask(HDMIRX_PHY_g0_reg, ~(0x1<<6), 0);    //fast switch circuit disable
        hdmi_mask(HDMIRX_PHY_g0_reg, ~(0x1<<7), 0);    //reset delay cell dff

        // 0x1800DA50[7][6]
        hdmi_mask(HDMIRX_PHY_b0_reg, ~(0x1<<6), 0);    //fast switch circuit disable
        hdmi_mask(HDMIRX_PHY_b0_reg, ~(0x1<<7), 0);    //reset delay cell dff

        // 0x1800DA30[7][6]
        hdmi_mask(HDMIRX_PHY_ck0_reg, ~(0x1<<6), 0);    //fast switch circuit disable
        hdmi_mask(HDMIRX_PHY_ck0_reg, ~(0x1<<7), 0);    //reset delay cell dff

        // DPHY
        // reset HD21 DPHY
	hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<11), 1<<11);   // Power on reg_p0_pow_save_bypass_ck by FW
	hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<10), 1<<10);   // Power on reg_p0_pow_save_bypass_r by FW
	hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<9), 1<<9);     // Power on reg_p0_pow_save_bypass_g by FW
	hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<8), 1<<8);    // Power onreg_p0_pow_save_bypass_b by FW
	hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<2), 0);       // reg_p0_pow_on_manual For MAC power saving, 1 : power on  0 : power off
	hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<7), 0<<7);   // reg_p0_pow_on_manual_ck_en
	hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<6), 0<<6);   // reg_p0_pow_on_manual_r_en
	hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<5), 0<<5);     //  reg_p0_pow_on_manual_g_en
	hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<4), 0<<4);    //  reg_p0_pow_on_manual_b_en    
       hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<3), 0<<3);    //  reg_p0_pow_on_manual_aphy_en Enable power save manual mode for APHY           

	//hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0xF<<4), (0xF<<4));   //[7]~[4]ck,r,g,b  manual power setting = On 
	hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<7), 1<<7);   // reg_p0_pow_on_manual_ck_en
	hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<6), 1<<6);   // reg_p0_pow_on_manual_r_en
	hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<5), 1<<5);     //  reg_p0_pow_on_manual_g_en
	hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<4), 1<<4);    //  reg_p0_pow_on_manual_b_en
       hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<3), 1<<3);    //  reg_p0_pow_on_manual_aphy_en Enable power save manual mode for APHY           

        // release reset
        hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<0), (1<<0));  // fast switch release reset p0 b Reset_n for all digital CDR of B lane
        hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<1), (1<<1));  // fast switch release reset p0 g Reset_n for all digital CDR of G lane
        hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<2), (1<<2));  // fast switch release reset p0 r Reset_n for all digital CDR of R lane

        hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<4), (1<<4));  // fast switch release reset p0 b Reset_n for digital CDR of B lane
        hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<5), (1<<5));  // fast switch release reset p0 g Reset_n for digital CDR of G lane
        hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<6), (1<<6));  // fast switch release reset p0 r Reset_n for digital CDR of R lane
        break;


    }
}

void lib_hdmi_phy_enter_power_saving_mode(unsigned char nport, HDMI_MODE mode, unsigned char br_swap)
{
    unsigned int hd21_mac = 0;
    PWR_EMG("HDMI[p%d] phy enter power saveing mode (mode=%d, br_swap=%d) \n", nport, mode, br_swap);

    if (hdmi_bypass_phy_ps_mode)
    {
        PWR_EMG("hdmi_bypass_phy_ps_mode is enabled, do nothing...\n");
        return ;
    }

    switch(mode)
    {
    case HDMI_20:
        switch(nport)
        {
        case 0:

		hdmi_mask(GDIRX_HDMI20_PHY_P0_TOP_IN_0_reg, ~(GDIRX_HDMI20_PHY_P0_TOP_IN_0_reg_fast_sw_div_sel_mask),GDIRX_HDMI20_PHY_P0_TOP_IN_0_get_reg_fast_sw_div_sel(0x3));  // power saving signal delay select. Fxtal/8 /16 /1024 /2048 =>REG_FAST_SW_DIV_SEL
		hdmi_mask(GDIRX_HDMI20_PHY_P0_TOP_IN_0_reg, ~(GDIRX_HDMI20_PHY_P0_TOP_IN_0_reg_fast_sw_div_en_mask),GDIRX_HDMI20_PHY_P0_TOP_IN_0_reg_fast_sw_div_en_mask);  // power saving signal delay enable =>REG_FAST_SW_DIV_EN

            if (br_swap)
            {
                // Analog
	        hdmi_mask(GDIRX_HDMI20_PHY_P0_P0_R2_reg, ~GDIRX_HDMI20_PHY_P0_P0_R2_reg_p0_r_pow_saving_switch_enable_mask, GDIRX_HDMI20_PHY_P0_P0_R2_reg_p0_r_pow_saving_switch_enable_mask); //fast switch circuit enable
	        hdmi_mask(GDIRX_HDMI20_PHY_P0_P0_G2_reg, ~GDIRX_HDMI20_PHY_P0_P0_G2_reg_p0_g_pow_saving_switch_enable_mask, GDIRX_HDMI20_PHY_P0_P0_G2_reg_p0_g_pow_saving_switch_enable_mask); //fast switch circuit enable
	        hdmi_mask(GDIRX_HDMI20_PHY_P0_P0_B2_reg, ~GDIRX_HDMI20_PHY_P0_P0_B2_reg_p0_b_pow_saving_switch_enable_mask, 0); //fast switch circuit enable

                // Digital
	        hdmi_mask(GDIRX_HDMI20_PHY_P0_DCDR_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_r_dig_rst_n_mask), 0);  //P0_R_DIG_RST_N fast switch reset p0 r
	        hdmi_mask(GDIRX_HDMI20_PHY_P0_DCDR_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_g_dig_rst_n_mask), 0);  //P0_G_DIG_RST_N fast switch reset p0 g
	        hdmi_mask(GDIRX_HDMI20_PHY_P0_DCDR_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_r_cdr_rst_n_mask), 0);  //P0_R_CDR_RST_N  fast switch reset p0 r        
	        hdmi_mask(GDIRX_HDMI20_PHY_P0_DCDR_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_g_cdr_rst_n_mask),0);  //P0_G_CDR_RST_N fast switch reset p0 g
            }
            else
            {
                // Analog
	        hdmi_mask(GDIRX_HDMI20_PHY_P0_P0_R2_reg, ~GDIRX_HDMI20_PHY_P0_P0_R2_reg_p0_r_pow_saving_switch_enable_mask, 0); //fast switch circuit enable
	        hdmi_mask(GDIRX_HDMI20_PHY_P0_P0_G2_reg, ~GDIRX_HDMI20_PHY_P0_P0_G2_reg_p0_g_pow_saving_switch_enable_mask, GDIRX_HDMI20_PHY_P0_P0_G2_reg_p0_g_pow_saving_switch_enable_mask); //fast switch circuit enable
	        hdmi_mask(GDIRX_HDMI20_PHY_P0_P0_B2_reg, ~GDIRX_HDMI20_PHY_P0_P0_B2_reg_p0_b_pow_saving_switch_enable_mask, GDIRX_HDMI20_PHY_P0_P0_B2_reg_p0_b_pow_saving_switch_enable_mask); //fast switch circuit enable

                // Digital
	        hdmi_mask(GDIRX_HDMI20_PHY_P0_DCDR_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_g_dig_rst_n_mask), 0);  //P0_G_DIG_RST_N fast switch reset p0 g
	        hdmi_mask(GDIRX_HDMI20_PHY_P0_DCDR_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_b_dig_rst_n_mask), 0);  //P0_B_DIG_RST_N fast switch reset p0 b
	        hdmi_mask(GDIRX_HDMI20_PHY_P0_DCDR_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_g_cdr_rst_n_mask),0);  //P0_G_CDR_RST_N fast switch reset p0 g
	        hdmi_mask(GDIRX_HDMI20_PHY_P0_DCDR_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_b_cdr_rst_n_mask), 0);  //P0_B_CDR_RST_N fast switch reset p0 b
				
            }
            
            // POWR OFF DPHY
		hdmi_mask(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_save_bypass_mask),GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_save_bypass_mask);    //reg_p0_hd14_pow_save_bypass bypass power saving (HDMI-1.4)
		hdmi_mask(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_save_bypass_r_only_mask),GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_save_bypass_r_only_mask);    //Power on reg_p0_hd14_pow_save_bypass_r_only by FW
		hdmi_mask(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_save_bypass_g_only_mask),GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_save_bypass_g_only_mask);    //Power on reg_p0_hd14_pow_save_bypass_g_only by FW
		hdmi_mask(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_save_bypass_b_only_mask),GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_save_bypass_b_only_mask);    //Power onreg_p0_hd14_pow_save_bypass_b_only by FW
		hdmi_mask(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_on_manual_en_mask), GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_on_manual_en_mask);        //Power on by FW
		hdmi_mask(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_on_manual_mask), 0);          //reg_p0_hd14_pow_on_manual manual power=OFF
			
            break;

        case 1:
           //REG_TOP_IN_2[0]  0: select HDMI 1.4 fast switch signal  1: select HDMI 2.1 fast switch signal
           hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(1<<8), 0);
           //REG_TOP_IN_2[0]=1'b1
           hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(1<<9), (0<<9));//]port-0 fast switch signal delay enable bit
           hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(1<<9), (1<<9));
           //REG_TOP_IN_2[0]<4:2>=1'b100
            hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(0x7<<10), (0x4<<10));
		
            //APHY Setting
	     //REG_TOP_IN_2[0]  0: select HDMI 1.4 fast switch signal  1: select HDMI 2.1 fast switch signal
	     hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(1<<8), 0);

            //REG_TOP_IN_2<1>=1'b1
            hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(1<<9), (0<<9));//]port-0 fast switch signal delay enable bit
            hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(1<<9), (1<<9));//]port-0 fast switch signal delay enable bit

            //REG_TOP_IN_2<4:2>=1'b100
            hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(0x7<<10), (0x4<<10));

            // POWER OFF FLOW HD20
            //0x1800DA90
            hdmi_mask(HDMIRX_PHY_r0_reg, ~(0x1<<6), 0);    // R Lane : fast switch circuit disable

            // 0x1800DA30
            hdmi_mask(HDMIRX_PHY_ck0_reg, ~(0x1<<6), 0);    // CK Lane : fast switch circuit disable

            // 0x1800DA70
            hdmi_mask(HDMIRX_PHY_g0_reg, ~(0x1<<6), 0);    //fast switch circuit disable
            hdmi_mask(HDMIRX_PHY_g0_reg, ~(0x1<<7), 0);    //reset delay cell dff
            hdmi_mask(HDMIRX_PHY_g0_reg, ~(0x1<<7), 1<<7);//Must set before [6]
            hdmi_mask(HDMIRX_PHY_g0_reg, ~(0x1<<6), 1<<6);//fast switch circuit enable

            // 0x1800DA68 [31:24] REG_P0_ACDR_B_8
            hdmi_mask(HDMIRX_PHY_b0_reg, ~(0x1<<6), 0);    //fast switch circuit disable
            hdmi_mask(HDMIRX_PHY_b0_reg, ~(0x1<<7), 0);    //reset delay cell dff
            hdmi_mask(HDMIRX_PHY_b0_reg, ~(0x1<<7), 1<<7);//Must set before [6]
            hdmi_mask(HDMIRX_PHY_b0_reg, ~(0x1<<6), 1<<6);//fast switch circuit enable

            // Digital Phy Setting
            hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<1), 0);   //[1]Power save signal from MAC 0: 10 bits (TMDS)  1: 18 bits(FRL)
            hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<0), 0);  // fast switch  reset p0 b Reset_n for all digital CDR of B lane
            hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<1), 0);  // fast switch reset p0 g Reset_n for all digital CDR of G lane
            hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<4), 0);  // fast switch  reset p0 b Reset_n for digital CDR of B lane
            hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<5), 0);  // fast switch  reset p0 g Reset_n for digital CDR of G lane

           hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<1), 0);   //[1]Power save signal from MAC 0: 10 bits (TMDS)  1: 18 bits(FRL)

            // POWR OFF
            hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<11), 1<<11);   // Power on reg_p0_pow_save_bypass_ck by FW
            hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<10), 1<<10);   // Power on reg_p0_pow_save_bypass_r by FW
            hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<9), 1<<9);     // Power on reg_p0_pow_save_bypass_g by FW
            hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<8), 1<<8);    // Power onreg_p0_pow_save_bypass_b by FW
            
           // hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<14), 1<<14);  //pow_on_manual_en Power on by FW
            hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<7), 1<<7);   // reg_p0_pow_on_manual_ck_en
            hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<6), 1<<6);   // reg_p0_pow_on_manual_r_en
            hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<5), 1<<5);     //  reg_p0_pow_on_manual_g_en
            hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<4), 1<<4);    //  reg_p0_pow_on_manual_b_en
            hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<3), 1<<3);    //  reg_p0_pow_on_manual_aphy_en Enable power save manual mode for APHY           
            hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<2), 0);      // Power save manual mode control signal manul_power_on=OFF (should be default value)

            break;

        case 2:  // waiting for DIC to realease
           //REG_TOP_IN_2[0]  0: select HDMI 1.4 fast switch signal  1: select HDMI 2.1 fast switch signal
           hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(1<<8), 0);
           //REG_TOP_IN_2[0]=1'b1
           hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(1<<9), (0<<9));//]port-0 fast switch signal delay enable bit
           hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(1<<9), (1<<9));
           //REG_TOP_IN_2[0]<4:2>=1'b100
            hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(0x7<<10), (0x4<<10));
		
            //APHY Setting
	     //REG_TOP_IN_2[0]  0: select HDMI 1.4 fast switch signal  1: select HDMI 2.1 fast switch signal
	     hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(1<<8), 0);

            //REG_TOP_IN_2<1>=1'b1
            hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(1<<9), (0<<9));//]port-0 fast switch signal delay enable bit
            hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(1<<9), (1<<9));//]port-0 fast switch signal delay enable bit

            //REG_TOP_IN_2<4:2>=1'b100
            hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(0x7<<10), (0x4<<10));

            // POWER OFF FLOW HD20
            //0x1800DA90
            hdmi_mask(HDMIRX_PHY_r0_reg, ~(0x1<<6), 0);    // R Lane : fast switch circuit disable

            // 0x1800DA30
            hdmi_mask(HDMIRX_PHY_ck0_reg, ~(0x1<<6), 0);    // CK Lane : fast switch circuit disable

            // 0x1800DA70
            hdmi_mask(HDMIRX_PHY_g0_reg, ~(0x1<<6), 0);    //fast switch circuit disable
            hdmi_mask(HDMIRX_PHY_g0_reg, ~(0x1<<7), 0);    //reset delay cell dff
            hdmi_mask(HDMIRX_PHY_g0_reg, ~(0x1<<7), 1<<7);//Must set before [6]
            hdmi_mask(HDMIRX_PHY_g0_reg, ~(0x1<<6), 1<<6);//fast switch circuit enable

            // 0x1800DA68 [31:24] REG_P0_ACDR_B_8
            hdmi_mask(HDMIRX_PHY_b0_reg, ~(0x1<<6), 0);    //fast switch circuit disable
            hdmi_mask(HDMIRX_PHY_b0_reg, ~(0x1<<7), 0);    //reset delay cell dff
            hdmi_mask(HDMIRX_PHY_b0_reg, ~(0x1<<7), 1<<7);//Must set before [6]
            hdmi_mask(HDMIRX_PHY_b0_reg, ~(0x1<<6), 1<<6);//fast switch circuit enable

            // Digital Phy Setting
            hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<1), 0);   //[1]Power save signal from MAC 0: 10 bits (TMDS)  1: 18 bits(FRL)
            hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<0), 0);  // fast switch  reset p0 b Reset_n for all digital CDR of B lane
            hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<1), 0);  // fast switch reset p0 g Reset_n for all digital CDR of G lane
            hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<4), 0);  // fast switch  reset p0 b Reset_n for digital CDR of B lane
            hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<5), 0);  // fast switch  reset p0 g Reset_n for digital CDR of G lane

           hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<1), 0);   //[1]Power save signal from MAC 0: 10 bits (TMDS)  1: 18 bits(FRL)

            // POWR OFF
            hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<11), 1<<11);   // Power on reg_p0_pow_save_bypass_ck by FW
            hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<10), 1<<10);   // Power on reg_p0_pow_save_bypass_r by FW
            hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<9), 1<<9);     // Power on reg_p0_pow_save_bypass_g by FW
            hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<8), 1<<8);    // Power onreg_p0_pow_save_bypass_b by FW
            
           // hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<14), 1<<14);  //pow_on_manual_en Power on by FW
            hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<7), 1<<7);   // reg_p0_pow_on_manual_ck_en
            hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<6), 1<<6);   // reg_p0_pow_on_manual_r_en
            hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<5), 1<<5);     //  reg_p0_pow_on_manual_g_en
            hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<4), 1<<4);    //  reg_p0_pow_on_manual_b_en
            hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<3), 1<<3);    //  reg_p0_pow_on_manual_aphy_en Enable power save manual mode for APHY           
            hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<2), 0);      // Power save manual mode control signal manul_power_on=OFF (should be default value)
            break;
        }

        break;

    //////////////////////////////////////////////////////////////////////////////////////////
    //Logic b lane on/off, g lane always off, r/ck lane always on		
    case HDMI_20_VS_WIN_OPP:
		//FIXME, please compare with rtd6748
        break;
    ///////////////////////////////////////////////////////////////////////////////////////////
    case HDMI_14:
    case HDMI_21:

        switch(nport)
        {
        case 0:

            //hdmi_mask(HDMIRX_2P0_PHY_P0_R4_reg, ~0x1, 1); //fast switch circuit enable
            //hdmi_mask(HDMIRX_2P0_PHY_P0_G4_reg, ~0x1, 1); //fast switch circuit enable
            //hdmi_mask(HDMIRX_2P0_PHY_P0_B4_reg, ~0x1, 1); //fast switch circuit enable
	        hdmi_mask(GDIRX_HDMI20_PHY_P0_P0_R2_reg, ~GDIRX_HDMI20_PHY_P0_P0_R2_reg_p0_r_pow_saving_switch_enable_mask, GDIRX_HDMI20_PHY_P0_P0_R2_reg_p0_r_pow_saving_switch_enable_mask); //fast switch circuit enable
	        hdmi_mask(GDIRX_HDMI20_PHY_P0_P0_G2_reg, ~GDIRX_HDMI20_PHY_P0_P0_G2_reg_p0_g_pow_saving_switch_enable_mask, GDIRX_HDMI20_PHY_P0_P0_G2_reg_p0_g_pow_saving_switch_enable_mask); //fast switch circuit enable
	        hdmi_mask(GDIRX_HDMI20_PHY_P0_P0_B2_reg, ~GDIRX_HDMI20_PHY_P0_P0_B2_reg_p0_b_pow_saving_switch_enable_mask, GDIRX_HDMI20_PHY_P0_P0_B2_reg_p0_b_pow_saving_switch_enable_mask); //fast switch circuit enable

            if (mode==HDMI_14) 
            {
	          // POWR OFF DPHY
		   hdmi_mask(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_save_bypass_mask),GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_save_bypass_mask);    //reg_p0_hd14_pow_save_bypass bypass power saving (HDMI-1.4)
		   hdmi_mask(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_on_manual_en_mask), 0);        //reg_p0_hd14_pow_on_manual_en power on cntrol by MAC  (HDMI-1.4)
		   hdmi_mask(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_on_manual_mask), 0);    //reg_p0_hd14_pow_on_manual manual power setting = On (HDMI-1.4)
            }
            break;

        case 1:
            hd21_mac = (hdmi_in(HDMIRX_PHY_top_in_0_reg) & (1<<8)) ? 1 : 0;

            if ((mode==HDMI_21 && !hd21_mac) || (mode!=HDMI_21 && hd21_mac) ||
                (hdmi_in(HDMIRX_PHY_r0_reg) & (1<<6))==0 ||
                (hdmi_in(HDMIRX_PHY_g0_reg) & (1<<6))==0 ||
                (hdmi_in(HDMIRX_PHY_b0_reg) & (1<<6))==0)
            {
                //PHY Setting
                //REG_TOP_IN_2[0]  0:
                hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(1<<8), (mode==HDMI_21) ? (1<<8) : 0);
	        //REG_TOP_IN_8<1>=1'b1
	        hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(1<<9), (0<<9));//]port-0 fast switch signal delay enable bit
	        hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(1<<9), (1<<9));
				
                //REG_TOP_IN_8<4:2>=1'b100
                hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(0x7<<10), (0x4<<10));

                //REG_P#_ACDR_*_8<1>=1'b1
                //REG_P#_ACDR_*_8<2>=1'b0 a 1
                //0x1800DAA8 [31:24] REG_P0_ACDR_R_8
               hdmi_mask(HDMIRX_PHY_r0_reg, ~(0x1<<6), 0);    //fast switch circuit disable
               hdmi_mask(HDMIRX_PHY_r0_reg, ~(0x1<<7), 0);    //reset delay cell dff
               hdmi_mask(HDMIRX_PHY_r0_reg, ~(0x1<<7), 1<<7);//Must set before [6]
               udelay(100);            
               hdmi_mask(HDMIRX_PHY_r0_reg, ~(0x1<<6), 1<<6);//fast switch circuit enable

                // 0x1800DA88 [31:24] REG_P0_ACDR_G_8
               hdmi_mask(HDMIRX_PHY_g0_reg, ~(0x1<<6), 0);    //fast switch circuit disable
               hdmi_mask(HDMIRX_PHY_g0_reg, ~(0x1<<7), 0);    //reset delay cell dff
               hdmi_mask(HDMIRX_PHY_g0_reg, ~(0x1<<7), 1<<7);//Must set before [6]
               udelay(100);            
               hdmi_mask(HDMIRX_PHY_g0_reg, ~(0x1<<6), 1<<6);//fast switch circuit enable

                // 0x1800DA68 [31:24] REG_P0_ACDR_B_8
               hdmi_mask(HDMIRX_PHY_b0_reg, ~(0x1<<6), 0);    //fast switch circuit disable
               hdmi_mask(HDMIRX_PHY_b0_reg, ~(0x1<<7), 0);    //reset delay cell dff
               hdmi_mask(HDMIRX_PHY_b0_reg, ~(0x1<<7), 1<<7);//Must set before [6]
               udelay(100);            
               hdmi_mask(HDMIRX_PHY_b0_reg, ~(0x1<<6), 1<<6);//fast switch circuit enable

                // 0x1800DA48 [31:24] REG_P0_ACDR_CK_8
               hdmi_mask(HDMIRX_PHY_ck0_reg, ~(0x1<<6), 0);    //fast switch circuit disable
               hdmi_mask(HDMIRX_PHY_ck0_reg, ~(0x1<<7), 0);    //reset delay cell dff
               hdmi_mask(HDMIRX_PHY_ck0_reg, ~(0x1<<7), 1<<7);//Must set before [6]
               udelay(100);            
               hdmi_mask(HDMIRX_PHY_ck0_reg, ~(0x1<<6), 1<<6);//fast switch circuit enable
            }

 
            if (mode==HDMI_21)
            {
                hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<1), 1<<1);   //[1]Power save signal from MAC 0: 10 bits (TMDS)  1: 18 bits(FRL)

                //hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<29), 1<<29);   // manual power on = ON  (HDMI-2.1)
                hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<2), 1<<2);      // Power save manual mode control signal manul_power_on=ON(should be default value)
                //hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<30), 0);       // power on cntrol by MAC (HDMI-2.1)
               hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<7), 0<<7);   // reg_p0_pow_on_manual_ck_en
               hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<6), 0<<6);   // reg_p0_pow_on_manual_r_en
               hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<5), 0<<5);     //  reg_p0_pow_on_manual_g_en
               hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<4), 0<<4);    //  reg_p0_pow_on_manual_b_en
               hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<3), 0<<3);    //  reg_p0_pow_on_manual_aphy_en Enable power save manual mode for APHY           


                //hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<31), 0);       // disable fast switch bypass (HDMI-2.1)
	         hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<11), 0);   // Power on reg_p0_pow_save_bypass_ck by MAC
	         hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<10), 0);   // Power on reg_p0_pow_save_bypass_r by MAC
	         hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<9), 0);     // Power on reg_p0_pow_save_bypass_g by MAC
	         hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<8), 0);    // Power onreg_p0_pow_save_bypass_b by MAC
            }
            else
            {
               hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<1), 0);   //[1]Power save signal from MAC 0: 10 bits (TMDS)  1: 18 bits(FRL)
               // hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<13), 1<<13);   // manual power on = ON (HDMI-1.4)
                hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<2), 0x1<<2);      // Power save manual mode control signal manul_power_on=ON (should be default value)

                //hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<14), 0);       // power on cntrol by MAC  (HDMI-1.4)
                hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<7), 0<<7);   // reg_p0_pow_on_manual_ck_en
                hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<6), 0<<6);   // reg_p0_pow_on_manual_r_en
                hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<5), 0<<5);     //  reg_p0_pow_on_manual_g_en
                hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<4), 0<<4);    //  reg_p0_pow_on_manual_b_en           
                hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<3), 0<<3);    //  reg_p0_pow_on_manual_aphy_en Enable power save manual mode for APHY           

                //hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<15), 0);       // disable fast switch bypass (HDMI-1.4)
                hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<10), 0);   // Power on reg_p0_pow_save_bypass_r by MAC
                hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<9), 0);     // Power on reg_p0_pow_save_bypass_g by MAC
                hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<8), 0);    // Power onreg_p0_pow_save_bypass_b by MAC
            }

            break;

        case 2:
            hd21_mac = (hdmi_in(HDMIRX_PHY_top_in_0_reg) & (1<<8)) ? 1 : 0;

            if ((mode==HDMI_21 && !hd21_mac) || (mode!=HDMI_21 && hd21_mac) ||
                (hdmi_in(HDMIRX_PHY_r0_reg) & (1<<6))==0 ||
                (hdmi_in(HDMIRX_PHY_g0_reg) & (1<<6))==0 ||
                (hdmi_in(HDMIRX_PHY_b0_reg) & (1<<6))==0)
            {
                //PHY Setting
                //REG_TOP_IN_2[0]  0:
                hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(1<<8), (mode==HDMI_21) ? (1<<8) : 0);
	        //REG_TOP_IN_8<1>=1'b1
	        hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(1<<9), (0<<9));//]port-0 fast switch signal delay enable bit
	        hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(1<<9), (1<<9));
				
                //REG_TOP_IN_8<4:2>=1'b100
                hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(0x7<<10), (0x4<<10));

                //REG_P#_ACDR_*_8<1>=1'b1
                //REG_P#_ACDR_*_8<2>=1'b0 a 1
                //0x1800DAA8 [31:24] REG_P0_ACDR_R_8
               hdmi_mask(HDMIRX_PHY_r0_reg, ~(0x1<<6), 0);    //fast switch circuit disable
               hdmi_mask(HDMIRX_PHY_r0_reg, ~(0x1<<7), 0);    //reset delay cell dff
               hdmi_mask(HDMIRX_PHY_r0_reg, ~(0x1<<7), 1<<7);//Must set before [6]
               udelay(100);            
               hdmi_mask(HDMIRX_PHY_r0_reg, ~(0x1<<6), 1<<6);//fast switch circuit enable

                // 0x1800DA88 [31:24] REG_P0_ACDR_G_8
               hdmi_mask(HDMIRX_PHY_g0_reg, ~(0x1<<6), 0);    //fast switch circuit disable
               hdmi_mask(HDMIRX_PHY_g0_reg, ~(0x1<<7), 0);    //reset delay cell dff
               hdmi_mask(HDMIRX_PHY_g0_reg, ~(0x1<<7), 1<<7);//Must set before [6]
               udelay(100);            
               hdmi_mask(HDMIRX_PHY_g0_reg, ~(0x1<<6), 1<<6);//fast switch circuit enable

                // 0x1800DA68 [31:24] REG_P0_ACDR_B_8
               hdmi_mask(HDMIRX_PHY_b0_reg, ~(0x1<<6), 0);    //fast switch circuit disable
               hdmi_mask(HDMIRX_PHY_b0_reg, ~(0x1<<7), 0);    //reset delay cell dff
               hdmi_mask(HDMIRX_PHY_b0_reg, ~(0x1<<7), 1<<7);//Must set before [6]
               udelay(100);            
               hdmi_mask(HDMIRX_PHY_b0_reg, ~(0x1<<6), 1<<6);//fast switch circuit enable

                // 0x1800DA48 [31:24] REG_P0_ACDR_CK_8
               hdmi_mask(HDMIRX_PHY_ck0_reg, ~(0x1<<6), 0);    //fast switch circuit disable
               hdmi_mask(HDMIRX_PHY_ck0_reg, ~(0x1<<7), 0);    //reset delay cell dff
               hdmi_mask(HDMIRX_PHY_ck0_reg, ~(0x1<<7), 1<<7);//Must set before [6]
               udelay(100);            
               hdmi_mask(HDMIRX_PHY_ck0_reg, ~(0x1<<6), 1<<6);//fast switch circuit enable
            }

 
            if (mode==HDMI_21)
            {
                hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<1), 1<<1);   //[1]Power save signal from MAC 0: 10 bits (TMDS)  1: 18 bits(FRL)

                //hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<29), 1<<29);   // manual power on = ON  (HDMI-2.1)
                hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<2), 1<<2);      // Power save manual mode control signal manul_power_on=ON(should be default value)
                //hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<30), 0);       // power on cntrol by MAC (HDMI-2.1)
               hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<7), 0<<7);   // reg_p0_pow_on_manual_ck_en
               hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<6), 0<<6);   // reg_p0_pow_on_manual_r_en
               hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<5), 0<<5);     //  reg_p0_pow_on_manual_g_en
               hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<4), 0<<4);    //  reg_p0_pow_on_manual_b_en
               hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<3), 0<<3);    //  reg_p0_pow_on_manual_aphy_en Enable power save manual mode for APHY           


                //hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<31), 0);       // disable fast switch bypass (HDMI-2.1)
	         hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<11), 0);   // Power on reg_p0_pow_save_bypass_ck by MAC
	         hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<10), 0);   // Power on reg_p0_pow_save_bypass_r by MAC
	         hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<9), 0);     // Power on reg_p0_pow_save_bypass_g by MAC
	         hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<8), 0);    // Power onreg_p0_pow_save_bypass_b by MAC
            }
            else
            {
               hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<1), 0);   //[1]Power save signal from MAC 0: 10 bits (TMDS)  1: 18 bits(FRL)
               // hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<13), 1<<13);   // manual power on = ON (HDMI-1.4)
                hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<2), 0x1<<2);      // Power save manual mode control signal manul_power_on=ON (should be default value)

                //hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<14), 0);       // power on cntrol by MAC  (HDMI-1.4)
                hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<7), 0<<7);   // reg_p0_pow_on_manual_ck_en
                hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<6), 0<<6);   // reg_p0_pow_on_manual_r_en
                hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<5), 0<<5);     //  reg_p0_pow_on_manual_g_en
                hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<4), 0<<4);    //  reg_p0_pow_on_manual_b_en           
                hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<3), 0<<3);    //  reg_p0_pow_on_manual_aphy_en Enable power save manual mode for APHY           

                //hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<15), 0);       // disable fast switch bypass (HDMI-1.4)
                hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<10), 0);   // Power on reg_p0_pow_save_bypass_r by MAC
                hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<9), 0);     // Power on reg_p0_pow_save_bypass_g by MAC
                hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<8), 0);    // Power onreg_p0_pow_save_bypass_b by MAC
            }

            break;

        }
        break;
    }
}

void lib_hdmi_phy_exit_power_saving_mode(unsigned char nport, HDMI_MODE mode, unsigned char br_swap)
{
    PWR_EMG("HDMI[p%d] phy exit power saveing mode (mode=%d) Bryan\n", nport, mode);

    if (hdmi_bypass_phy_ps_mode)
    {
        PWR_EMG("hdmi_bypass_phy_ps_mode is enabled, do nothing...\n");
        return ;
    }

    switch(mode)
    {
    case HDMI_20:

        switch(nport)
        {
        case 0:
		// DPHY ON
		hdmi_mask(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_save_bypass_mask),GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_save_bypass_mask);    //reg_p0_hd14_pow_save_bypass bypass power saving (HDMI-1.4)
		hdmi_mask(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_save_bypass_r_only_mask),GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_save_bypass_r_only_mask);    //Power on reg_p0_hd14_pow_save_bypass_r_only by FW
		hdmi_mask(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_save_bypass_g_only_mask),GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_save_bypass_g_only_mask);    //Power on reg_p0_hd14_pow_save_bypass_g_only by FW
		hdmi_mask(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_save_bypass_b_only_mask),GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_save_bypass_b_only_mask);    //Power onreg_p0_hd14_pow_save_bypass_b_only by FW

		hdmi_mask(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_on_manual_en_mask), GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_on_manual_en_mask);        //Power on by FW
		hdmi_mask(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_on_manual_mask), GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_on_manual_mask);    //reg_p0_hd14_pow_on_manual manual power setting = On (HDMI-1.4)
		udelay(10);
		hdmi_mask(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_on_manual_en_mask), 0);          //reg_p0_hd14_pow_on_manual_en Power on by MAC
		hdmi_mask(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_on_manual_mask), 0);          //reg_p0_hd14_pow_on_manual manual power=OFF

            // DPHY RESET
            if (br_swap)
            {
	        hdmi_mask(GDIRX_HDMI20_PHY_P0_DCDR_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_r_dig_rst_n_mask), GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_r_dig_rst_n_mask);  //P0_R_DIG_RST_N fast switch reset p0 r
	        hdmi_mask(GDIRX_HDMI20_PHY_P0_DCDR_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_g_dig_rst_n_mask), GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_g_dig_rst_n_mask);  //P0_G_DIG_RST_N fast switch reset p0 g
	        hdmi_mask(GDIRX_HDMI20_PHY_P0_DCDR_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_r_cdr_rst_n_mask), GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_r_cdr_rst_n_mask);  //P0_R_CDR_RST_N  fast switch reset p0 r        
	        hdmi_mask(GDIRX_HDMI20_PHY_P0_DCDR_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_g_cdr_rst_n_mask),GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_g_cdr_rst_n_mask);  //P0_G_CDR_RST_N fast switch reset p0 g

            }
            else
            {
	        hdmi_mask(GDIRX_HDMI20_PHY_P0_DCDR_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_g_dig_rst_n_mask), GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_g_dig_rst_n_mask);  //P0_G_DIG_RST_N fast switch reset p0 g
	        hdmi_mask(GDIRX_HDMI20_PHY_P0_DCDR_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_b_dig_rst_n_mask), GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_b_dig_rst_n_mask);  //P0_B_DIG_RST_N fast switch reset p0 b
	        hdmi_mask(GDIRX_HDMI20_PHY_P0_DCDR_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_g_cdr_rst_n_mask),GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_g_cdr_rst_n_mask);  //P0_G_CDR_RST_N fast switch reset p0 g
	        hdmi_mask(GDIRX_HDMI20_PHY_P0_DCDR_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_b_cdr_rst_n_mask), GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_b_cdr_rst_n_mask);  //P0_B_CDR_RST_N fast switch reset p0 b

            }
            break;

        case 1:
 //From Mark2
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<1), 0);   //[1]Power save signal from MAC 0: 10 bits (TMDS)  1: 18 bits(FRL)

		// DPHY
		// reset HD21 DPHY
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<11), 1<<11);   // Power on reg_p0_pow_save_bypass_ck by FW
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<10), 1<<10);   // Power on reg_p0_pow_save_bypass_r by FW
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<9), 1<<9);     // Power on reg_p0_pow_save_bypass_g by FW
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<8), 1<<8);    // Power onreg_p0_pow_save_bypass_b by FW

		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<2), 0);       // reg_p0_pow_on_manual For MAC power saving, 1 : power on  0 : power off
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<7), 0<<7);   // reg_p0_pow_on_manual_ck_en
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<6), 0<<6);   // reg_p0_pow_on_manual_r_en
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<5), 0<<5);     //  reg_p0_pow_on_manual_g_en
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<4), 0<<4);    //  reg_p0_pow_on_manual_b_en    
	       hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<3), 0<<3);    //  reg_p0_pow_on_manual_aphy_en Enable power save manual mode for APHY           
		
		//hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0xF<<4), (0xF<<4));   //[7]~[4]ck,r,g,b  manual power setting = On 
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<7), 1<<7);   // reg_p0_pow_on_manual_ck_en
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<6), 1<<6);   // reg_p0_pow_on_manual_r_en
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<5), 1<<5);     //  reg_p0_pow_on_manual_g_en
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<4), 1<<4);    //  reg_p0_pow_on_manual_b_en
	       hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<3), 1<<3);    //  reg_p0_pow_on_manual_aphy_en Enable power save manual mode for APHY           

		// release reset
		hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<0), (1<<0));  // fast switch release reset p0 b Reset_n for all digital CDR of B lane
		hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<1), (1<<1));  // fast switch release reset p0 g Reset_n for all digital CDR of G lane
		hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<2), (1<<2));  // fast switch release reset p0 r Reset_n for all digital CDR of R lane

		hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<4), (1<<4));  // fast switch release reset p0 b Reset_n for digital CDR of B lane
		hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<5), (1<<5));  // fast switch release reset p0 g Reset_n for digital CDR of G lane
		hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<6), (1<<6));  // fast switch release reset p0 r Reset_n for digital CDR of R lane

		// DPHY ON
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<10), 1<<10);   // Power on reg_p0_pow_save_bypass_r by FW
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<9), 1<<9);     // Power on reg_p0_pow_save_bypass_g by FW
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<8), 1<<8);    // Power onreg_p0_pow_save_bypass_b by FW

		//hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<14), 1<<14);    // Power on by FW
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<7), 1<<7);   // reg_p0_pow_on_manual_ck_en
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<6), 1<<6);   // reg_p0_pow_on_manual_r_en
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<5), 1<<5);     //  reg_p0_pow_on_manual_g_en
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<4), 1<<4);    //  reg_p0_pow_on_manual_b_en


		//hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<13), (1<<13));  // manual power=ON
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<2), 0x1<<2);      // Power save manual mode control signal manul_power_on=ON (should be default value)
		udelay(10);
		//hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<14), 0);        // Power on control by MAC
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<7), 0<<7);   // reg_p0_pow_on_manual_ck_en
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<6), 0<<6);   // reg_p0_pow_on_manual_r_en
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<5), 0<<5);     //  reg_p0_pow_on_manual_g_en
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<4), 0<<4);    //  reg_p0_pow_on_manual_b_en           
	       hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<3), 0<<3);    //  reg_p0_pow_on_manual_aphy_en Enable power save manual mode for APHY           

		//hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<13), 0);        // manual power=OFF
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<2), 0x0<<2);      // Power save manual mode control signal manul_power_on=OFF (should be default value)

		// DPHY RESET
		hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<0), (1<<0));  // fast switch release reset p0 b Reset_n for all digital CDR of B lane
		hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<1), (1<<1));  // fast switch release reset p0 g Reset_n for all digital CDR of G lane
		hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<4), (1<<4));  // fast switch release reset p0 b Reset_n for digital CDR of B lane
		hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<5), (1<<5));  // fast switch release reset p0 g Reset_n for digital CDR of G lane

            break;

        case 2: // HDMI20 P0
 //From Mark2
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<1), 0);   //[1]Power save signal from MAC 0: 10 bits (TMDS)  1: 18 bits(FRL)

		// DPHY
		// reset HD21 DPHY
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<11), 1<<11);   // Power on reg_p0_pow_save_bypass_ck by FW
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<10), 1<<10);   // Power on reg_p0_pow_save_bypass_r by FW
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<9), 1<<9);     // Power on reg_p0_pow_save_bypass_g by FW
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<8), 1<<8);    // Power onreg_p0_pow_save_bypass_b by FW

		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<2), 0);       // reg_p0_pow_on_manual For MAC power saving, 1 : power on  0 : power off
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<7), 0<<7);   // reg_p0_pow_on_manual_ck_en
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<6), 0<<6);   // reg_p0_pow_on_manual_r_en
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<5), 0<<5);     //  reg_p0_pow_on_manual_g_en
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<4), 0<<4);    //  reg_p0_pow_on_manual_b_en    
	       hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<3), 0<<3);    //  reg_p0_pow_on_manual_aphy_en Enable power save manual mode for APHY           
		
		//hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0xF<<4), (0xF<<4));   //[7]~[4]ck,r,g,b  manual power setting = On 
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<7), 1<<7);   // reg_p0_pow_on_manual_ck_en
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<6), 1<<6);   // reg_p0_pow_on_manual_r_en
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<5), 1<<5);     //  reg_p0_pow_on_manual_g_en
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<4), 1<<4);    //  reg_p0_pow_on_manual_b_en
	       hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<3), 1<<3);    //  reg_p0_pow_on_manual_aphy_en Enable power save manual mode for APHY           

		// release reset
		hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<0), (1<<0));  // fast switch release reset p0 b Reset_n for all digital CDR of B lane
		hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<1), (1<<1));  // fast switch release reset p0 g Reset_n for all digital CDR of G lane
		hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<2), (1<<2));  // fast switch release reset p0 r Reset_n for all digital CDR of R lane

		hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<4), (1<<4));  // fast switch release reset p0 b Reset_n for digital CDR of B lane
		hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<5), (1<<5));  // fast switch release reset p0 g Reset_n for digital CDR of G lane
		hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<6), (1<<6));  // fast switch release reset p0 r Reset_n for digital CDR of R lane

		// DPHY ON
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<10), 1<<10);   // Power on reg_p0_pow_save_bypass_r by FW
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<9), 1<<9);     // Power on reg_p0_pow_save_bypass_g by FW
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<8), 1<<8);    // Power onreg_p0_pow_save_bypass_b by FW

		//hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<14), 1<<14);    // Power on by FW
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<7), 1<<7);   // reg_p0_pow_on_manual_ck_en
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<6), 1<<6);   // reg_p0_pow_on_manual_r_en
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<5), 1<<5);     //  reg_p0_pow_on_manual_g_en
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<4), 1<<4);    //  reg_p0_pow_on_manual_b_en


		//hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<13), (1<<13));  // manual power=ON
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<2), 0x1<<2);      // Power save manual mode control signal manul_power_on=ON (should be default value)
		udelay(10);
		//hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<14), 0);        // Power on control by MAC
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<7), 0<<7);   // reg_p0_pow_on_manual_ck_en
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<6), 0<<6);   // reg_p0_pow_on_manual_r_en
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<5), 0<<5);     //  reg_p0_pow_on_manual_g_en
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<4), 0<<4);    //  reg_p0_pow_on_manual_b_en           
	       hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<3), 0<<3);    //  reg_p0_pow_on_manual_aphy_en Enable power save manual mode for APHY           

		//hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<13), 0);        // manual power=OFF
		hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<2), 0x0<<2);      // Power save manual mode control signal manul_power_on=OFF (should be default value)

		// DPHY RESET
		hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<0), (1<<0));  // fast switch release reset p0 b Reset_n for all digital CDR of B lane
		hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<1), (1<<1));  // fast switch release reset p0 g Reset_n for all digital CDR of G lane
		hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<4), (1<<4));  // fast switch release reset p0 b Reset_n for digital CDR of B lane
		hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(0x1<<5), (1<<5));  // fast switch release reset p0 g Reset_n for digital CDR of G lane
            break;

        }

        break;

    ////////////////////////////////////////////////////////////////////////////////////////
    case HDMI_20_VS_WIN_OPP:
		//FIXME, please compare with rtd6748
        break;	
    ///////////////////////////////////////////////////////////////////////////////////////////
    case HDMI_14:
    case HDMI_21:
        switch(nport)
        {
        case 0:
            // DPHY ON
            hdmi_mask(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_save_bypass_mask), GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_save_bypass_mask);      //reg_p0_hd14_pow_save_bypass enable power saving byass (should be default value)
            hdmi_mask(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_on_manual_en_mask),0);      //reg_p0_hd14_pow_on_manual_en Power on by FW
            hdmi_mask(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg, ~(GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_on_manual_mask), GDIRX_HDMI20_PHY_P0_PSAVE_REG0_reg_p0_hd14_pow_on_manual_mask);    //reg_p0_hd14_pow_on_manual manual power=ON


            break;

        case 1:
		if ((hdmi_in(HDMIRX_PHY_top_in_0_reg) & (1<<8)))  // HDMI 2.1
		{
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<1), 1);   //[1]Power save signal from MAC 0: 10 bits (TMDS)  1: 18 bits(FRL)
			//hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<29), 1<<29);    // manual power on = ON  (HDMI-2.1)
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<2), 0x1<<2);      // Power save manual mode control signal manul_power_on=ON (should be default value)

			//hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<30), 0);        // power on cntrol by MAC (HDMI-2.1)
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<7), 0<<7);   // reg_p0_pow_on_manual_ck_en
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<6), 0<<6);   // reg_p0_pow_on_manual_r_en
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<5), 0<<5);     //  reg_p0_pow_on_manual_g_en
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<4), 0<<4);    //  reg_p0_pow_on_manual_b_en   
	              hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<3), 0<<3);    //  reg_p0_pow_on_manual_aphy_en Enable power save manual mode for APHY           

			//hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<31), 0);        // disable fast switch bypass (HDMI-2.1)
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<11), 0);   // Power on reg_p0_pow_save_bypass_ck by MAC
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<10), 0);   // Power on reg_p0_pow_save_bypass_r by MAC
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<9), 0);     // Power on reg_p0_pow_save_bypass_g by MAC
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<8), 0);    // Power onreg_p0_pow_save_bypass_b by MAC
		}
		else
		{
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<1), 0);   //[1]Power save signal from MAC 0: 10 bits (TMDS)  1: 18 bits(FRL)
			// hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<13), 1<<13);    // manual power on = ON  (HDMI-1.4)
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<2), 0x1<<2);      // Power save manual mode control signal manul_power_on=ON (should be default value)

			//hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<14), 0);        // Power on control by MAC   (HDMI-1.4)
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<7), 0<<7);   // reg_p0_pow_on_manual_ck_en
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<6), 0<<6);   // reg_p0_pow_on_manual_r_en
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<5), 0<<5);     //  reg_p0_pow_on_manual_g_en
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<4), 0<<4);    //  reg_p0_pow_on_manual_b_en                   
	              hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<3), 0<<3);    //  reg_p0_pow_on_manual_aphy_en Enable power save manual mode for APHY           

			//hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<15), 0);        // enable power saving byass  (HDMI-1.4)
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<10), 0);   // Power on reg_p0_pow_save_bypass_r by MAC
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<9), 0);     // Power on reg_p0_pow_save_bypass_g by MAC
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<8), 0);    // Power onreg_p0_pow_save_bypass_b by MAC
		}
            break;

        case 2: // HDMI20 P0
		if ((hdmi_in(HDMIRX_PHY_top_in_0_reg) & (1<<8)))  // HDMI 2.1
		{
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<1), 1);   //[1]Power save signal from MAC 0: 10 bits (TMDS)  1: 18 bits(FRL)
			//hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<29), 1<<29);    // manual power on = ON  (HDMI-2.1)
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<2), 0x1<<2);      // Power save manual mode control signal manul_power_on=ON (should be default value)

			//hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<30), 0);        // power on cntrol by MAC (HDMI-2.1)
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<7), 0<<7);   // reg_p0_pow_on_manual_ck_en
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<6), 0<<6);   // reg_p0_pow_on_manual_r_en
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<5), 0<<5);     //  reg_p0_pow_on_manual_g_en
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<4), 0<<4);    //  reg_p0_pow_on_manual_b_en   
	              hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<3), 0<<3);    //  reg_p0_pow_on_manual_aphy_en Enable power save manual mode for APHY           

			//hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<31), 0);        // disable fast switch bypass (HDMI-2.1)
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<11), 0);   // Power on reg_p0_pow_save_bypass_ck by MAC
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<10), 0);   // Power on reg_p0_pow_save_bypass_r by MAC
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<9), 0);     // Power on reg_p0_pow_save_bypass_g by MAC
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<8), 0);    // Power onreg_p0_pow_save_bypass_b by MAC
		}
		else
		{
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<1), 0);   //[1]Power save signal from MAC 0: 10 bits (TMDS)  1: 18 bits(FRL)
			// hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<13), 1<<13);    // manual power on = ON  (HDMI-1.4)
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<2), 0x1<<2);      // Power save manual mode control signal manul_power_on=ON (should be default value)

			//hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<14), 0);        // Power on control by MAC   (HDMI-1.4)
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<7), 0<<7);   // reg_p0_pow_on_manual_ck_en
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<6), 0<<6);   // reg_p0_pow_on_manual_r_en
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<5), 0<<5);     //  reg_p0_pow_on_manual_g_en
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<4), 0<<4);    //  reg_p0_pow_on_manual_b_en                   
	              hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<3), 0<<3);    //  reg_p0_pow_on_manual_aphy_en Enable power save manual mode for APHY           

			//hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD32_reg, ~(0x1<<15), 0);        // enable power saving byass  (HDMI-1.4)
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<10), 0);   // Power on reg_p0_pow_save_bypass_r by MAC
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<9), 0);     // Power on reg_p0_pow_save_bypass_g by MAC
			hdmi_mask(HDMIRX_PHY_psave_regd00, ~(0x1<<8), 0);    // Power onreg_p0_pow_save_bypass_b by MAC
		}

            break;
        }
    }
	//From Mark2
	if (nport > HDMI_PORT0) {
		//0x1800DAA8 [31:24] REG_P0_ACDR_R_8
		hdmi_mask(HDMIRX_PHY_r0_reg, ~(0x1<<6), 0);    //fast switch circuit disable
		// 0x1800DA88 [31:24] REG_P0_ACDR_G_8
		hdmi_mask(HDMIRX_PHY_g0_reg, ~(0x1<<6), 0);    //fast switch circuit disable
		// 0x1800DA68 [31:24] REG_P0_ACDR_B_8
		hdmi_mask(HDMIRX_PHY_b0_reg, ~(0x1<<6), 0);    //fast switch circuit disable
		// 0x1800DA48 [31:24] REG_P0_ACDR_CK_8
		hdmi_mask(HDMIRX_PHY_ck0_reg, ~(0x1<<6), 0);    //fast switch circuit disable
	}
	
}

void lib_hdmi_phy_dump_pow_saving_reg(unsigned char nport)
{
#if 0
    switch(nport)
    {
    case 0:
        PWR_EMG("HDMI[p%d] aphy power saving setting (%08x=%08x, b0=%d / %08x=%08x b0=%d , %08x=%08x  b0=%d \n",
            nport,
            (HDMIRX_2P0_PHY_P0_R4_reg),
            (hdmi_in(HDMIRX_2P0_PHY_P0_R4_reg)), 
            (hdmi_in(HDMIRX_2P0_PHY_P0_R4_reg)) & 1,
            (HDMIRX_2P0_PHY_P0_G4_reg),
            (hdmi_in(HDMIRX_2P0_PHY_P0_G4_reg)),
            (hdmi_in(HDMIRX_2P0_PHY_P0_G4_reg)) & 1,
            (HDMIRX_2P0_PHY_P0_B4_reg),
            (hdmi_in(HDMIRX_2P0_PHY_P0_B4_reg)),
            (hdmi_in(HDMIRX_2P0_PHY_P0_B4_reg)) & 1);
            
        PWR_EMG("HDMI[p%d] dphy power saving setting (%08x=%08x, b13=%d, b14=%d, b15=%d)\n",
            nport,
            (0xB800DD30),
            (hdmi_in(0xB800DD30)),
            (hdmi_in(0xB800DD30)>>13) & 1,
            (hdmi_in(0xB800DD30)>>14) & 1,
            (hdmi_in(0xB800DD30)>>15) & 1);      
        break;
        
    case 1:
        PWR_EMG("HDMI[p%d] aphy power saving setting (%08x=%08x, b0=%d / %08x=%08x b0=%d , %08x=%08x  b0=%d \n",
            nport,
            (HDMIRX_2P0_PHY_P1_R4_reg),
            (hdmi_in(HDMIRX_2P0_PHY_P1_R4_reg)), 
            (hdmi_in(HDMIRX_2P0_PHY_P1_R4_reg)) & 1,
            (HDMIRX_2P0_PHY_P1_G4_reg),
            (hdmi_in(HDMIRX_2P0_PHY_P1_G4_reg)),
            (hdmi_in(HDMIRX_2P0_PHY_P1_G4_reg)) & 1,
            (HDMIRX_2P0_PHY_P1_B4_reg),
            (hdmi_in(HDMIRX_2P0_PHY_P1_B4_reg)),
            (hdmi_in(HDMIRX_2P0_PHY_P1_B4_reg)) & 1);
            
        PWR_EMG("HDMI[p%d] dphy power saving setting (%08x=%08x, b13=%d, b14=%d, b15=%d)\n",
            nport,
            (0xB800DD30),
            (hdmi_in(0xB800DD30)),
            (hdmi_in(0xB800DD30)>>5) & 1,
            (hdmi_in(0xB800DD30)>>6) & 1,
            (hdmi_in(0xB800DD30)>>7) & 1);                  
        break;

    case 2:
        PWR_EMG("HDMI[p%d] aphy power saving setting (%08x=%08x, b0=%d / %08x=%08x b0=%d , %08x=%08x  b0=%d \n",
            nport,
            (HDMIRX_2P0_PHY_P2_R4_reg),
            (hdmi_in(HDMIRX_2P0_PHY_P2_R4_reg)), 
            (hdmi_in(HDMIRX_2P0_PHY_P2_R4_reg)) & 1,
            (HDMIRX_2P0_PHY_P2_G4_reg),
            (hdmi_in(HDMIRX_2P0_PHY_P2_G4_reg)),
            (hdmi_in(HDMIRX_2P0_PHY_P2_G4_reg)) & 1,
            (HDMIRX_2P0_PHY_P2_B4_reg),
            (hdmi_in(HDMIRX_2P0_PHY_P2_B4_reg)),
            (hdmi_in(HDMIRX_2P0_PHY_P2_B4_reg)) & 1);
            
        PWR_EMG("HDMI[p%d] dphy power saving setting (%08x=%08x, b13=%d, b14=%d, b15=%d)\n",
            nport,
            (0xB800DD30),
            (hdmi_in(0xB800DD30)),
            (hdmi_in(0xB800DD30)>>21) & 1,
            (hdmi_in(0xB800DD30)>>22) & 1,
            (hdmi_in(0xB800DD30)>>23) & 1);            
        break;

    }
#endif	
}
