#include <mach/rtk_platform.h>
#include <mach/platform.h>
#include "hdmi_common.h"
#include "hdmi_vfe_config.h"

extern HDMI_PORT_INFO_T hdmi_rx[HDMI_PORT_TOTAL_NUM];


struct semaphore sem_power_saving_ctrl;

void _init_power_saving_sem(void)   { sema_init(&sem_power_saving_ctrl, 1); }
void _lock_power_saving_sem(void)   { down(&sem_power_saving_ctrl); }
void _unlock_power_saving_sem(void) { up(&sem_power_saving_ctrl); }

unsigned int hdmi_power_saving_enable = 0;
unsigned int hdmi_power_saving_connected_port_if_not_hdmi_source = 1;
unsigned int hdmi_power_saving_test_mode = 0;
unsigned int hdmi_active_source = 0xF;
unsigned int ic_version = 0;

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

void newbase_hdmi_power_saving_init(void)
{
    _init_power_saving_sem();
    hdmi_active_source = 0xF;  // set active source
    ic_version = get_ic_version();
#ifdef CONFIG_RTK_KDRV_HDMI_ENABLE_DYNAMIC_POWER_SAVING
    hdmi_power_saving_enable = 1;
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
    PWR_FLOW_INFO("HDMI[p%d] reset power saving state\n", port);

    _lock_power_saving_sem();

    hdmi_rx[port].ps_fsm = PS_FSM_POWER_SAVING_OFF;
    hdmi_rx[port].signal_stable_cnt = 0;
    hdmi_rx[port].ps_fsm_sub_state = 0;
    hdmi_rx[port].ps_round = 0;

    lib_hdmi_phy_reset_power_saving_state(port);
    lib_hdmi_reset_power_saving_mode(port);

    _unlock_power_saving_sem();
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
    if (lib_hdmi_is_hdmi_mode(port)==MODE_DVI)  // do not enter power saving if it is DVI signal
        return HDMI_POWER_SAVEING_MODE_OFF;

    if ((newbase_hdmi_get_vrr_enable(port) || newbase_hdmi_get_freesync_enable(port)))
        return HDMI_POWER_SAVEING_MODE_OFF;

    //FIXME, wait code sync
    return HDMI_POWER_SAVEING_MODE_MAC_HW;
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
    unsigned char frl_mode = newbase_rxphy_get_frl_mode(port);	
    BOOLEAN is_run_arc = FALSE;
    BOOLEAN is_run_earc = FALSE;

    // check HPD/5V/PS Enable
    if (hdmi_rx[port].cable_conn==0 || newbase_hdmi_get_hpd(port)==0 || hdmi_power_saving_enable==0)
    {
        hdmi_rx[port].signal_stable_cnt = 0;
        return 0;
    }

    // check clock stable
    if (frl_mode == MODE_TMDS)
    {
        if (!newbase_rxphy_is_clk_stable(port) || newbase_rxphy_get_clk(port) < VALID_MIN_CLOCK)
        {
            hdmi_rx[port].signal_stable_cnt = 0;
            return 0;
        }
    }
    else
    {
        // FRL mode
#if 0    // Humm... we skip the check here because of in current version, the data align is start when setup video pll
        if (newbase_hdmi_hd21_is_data_align_lock(port)!=TRUE)
        {
            hdmi_rx[port].signal_stable_cnt = 0;
            return 0;
        }
#endif
    }

    if (hdmi_rx[port].signal_stable_cnt < HDMI_POWER_SAVING_MAX_STABLE_CNT)
    {
        if ((hdmi_rx[port].signal_stable_cnt & 0x3F)==0)
        {
            PWR_FLOW_INFO("HDMI[p%d] FSM=%s (%d), Round=%d, power saving signal stable count...(%d) interlace=%d, dvi=%d\n",
                port, _hdmi_power_saving_fsm_str(hdmi_rx[port].ps_fsm), hdmi_rx[port].ps_fsm, hdmi_rx[port].ps_round, hdmi_rx[port].signal_stable_cnt,
                GET_H_INTERLACE(port) , (lib_hdmi_is_hdmi_mode(port)==MODE_DVI) ? 1 : 0);
        }

        hdmi_rx[port].signal_stable_cnt++;
    }

    // Check Current Display Port
    if (_is_active_source(port)) 
    {
        if (hdmi_power_saving_connected_port_if_not_hdmi_source)
        {
            if (newbase_hdmi_is_dispsrc(0))
                return 0;   // do not power saving when HDMI is playing
        }
        else
            return 0;  // do not power saving if the port is connected
    }

    if (hdmi_rx[port].ps_fsm==PS_FSM_POWER_SAVING_OFF && (GET_H_INTERLACE(port) != HDMI_IPCHECK_PROGRESSIVE))    // do not enter power saving if it is interlace timing or NO sync
        return 0;

    if (hdmi_rx[port].ps_fsm==PS_FSM_POWER_SAVING_OFF && newbase_hdmi_power_saving_mode_probe(port)==HDMI_POWER_SAVEING_MODE_OFF)
        return 0;

    if (ic_version==0 && hdmi_rx[port].ps_fsm==PS_FSM_POWER_SAVING_OFF && (hdmi_rx[port].video_t.vs_pol_stable < 10 || hdmi_rx[port].video_t.vs_pol==0))  // do not enter power saving if it is negative VS polarity with ic VERSION_A
        return 0;

    if((newbase_hdmi_get_vrr_enable(port) |newbase_hdmi_get_freesync_enable(port)))
        return 0;

    // Check ARC/eARC alive
    GET_AUDIO_ARC_EN(&is_run_arc);
    GET_AUDIO_EARC_EN(&is_run_earc);
    if(((newbase_hdmi_get_arc_port_index() == port) || (newbase_hdmi_get_earc_port_index() == port)) &&
            ((is_run_arc == TRUE) || (is_run_earc == TRUE))) // Do not enter power saving if ARC/eARC is outputing sound.
        return 0;

    // Check wakeup condition
    if (hdmi_rx[port].ps_wakeup_request)
    {
        PWR_EMG("HDMI[p%d] got wake up request = %x\n", port, hdmi_rx[port].ps_wakeup_request);
        hdmi_rx[port].ps_wakeup_request = 0;
        hdmi_rx[port].signal_stable_cnt = 0;
        return 0;
    }

    //Check specified device
    if(newbase_hdmi_get_device_force_ps_off(port))
        return 0;

    // Check SPD info (if SPD is received already)
    if (hdmi_rx[port].spd_t.type_code && hdmi_rx[port].signal_stable_cnt > HDMI_POWER_SAVING_MIN_STABLE_CNT)
        return 1;

    // Check Signal stable count
    if (hdmi_rx[port].signal_stable_cnt >= HDMI_POWER_SAVING_MAX_STABLE_CNT)
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
    hdmi_rx[port].ps_wakeup_request = wakup_condition;
    PWR_FLOW_INFO("HDMI[p%d] set wake up request = %x\n", port, hdmi_rx[port].ps_wakeup_request);

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
    return hdmi_rx[port].ps_fsm;
}

void newbase_hdmi_power_saving_dump_status(unsigned char port)
{
    lib_hdmi_dump_mac_power_saving_reg(port);
    lib_hdmi_phy_dump_pow_saving_reg(port);
}


void newbase_hdmi_power_saving_dump_data_status(unsigned char port)
{
    unsigned char RxStatus[2] = {0, 0};
    HDMI_HDCP_E hdcp_mode = HDCP_OFF;
    HDMI_HDCP_ST* p_hdcp = newbase_hdcp_get_hdcp_status(port);
    hdcp_mode = newbase_hdcp_get_auth_mode(port);
    lib_hdmi_hdcp22_get_rx_status(port, RxStatus);

    switch (hdcp_mode)
    {
    case HDCP14:
        PWR_FLOW_INFO("HDMI[p%d] FSM=%s (%d), Round=%d, RxStatus(%02x, %02x), hdcp_mode=%d, hdcp_enc=%d, phy_pwr_on=%d, bch error=%d, bch error cnt=%d, auth_count=%d, ri_cnt=%d\n",
            port, _hdmi_power_saving_fsm_str(hdmi_rx[port].ps_fsm), hdmi_rx[port].ps_fsm, hdmi_rx[port].ps_round,
            RxStatus[0], RxStatus[1],
            hdcp_mode,
            (hdmi_rx[port].hdcp_status.hdcp_enc) ? 1 : 0,
            lib_hdmi_phy_power_saving_status(port),
            hdmi_rx[port].bch_err_detect,
            hdmi_rx[port].bch_err_cnt,
            p_hdcp->hdcp14_auth_count,
            p_hdcp->hdcp14_ri_count);
        break;

    case HDCP22:
        PWR_FLOW_INFO("HDMI[p%d] FSM=%s (%d), Round=%d, RxStatus(%02x, %02x), hdcp_mode=%d, hdcp_enc=%d, phy_pwr_on=%d, bch error=%d, bch error cnt=%d, auth_count=%d\n",
            port, _hdmi_power_saving_fsm_str(hdmi_rx[port].ps_fsm), hdmi_rx[port].ps_fsm, hdmi_rx[port].ps_round,
            RxStatus[0], RxStatus[1],
            hdcp_mode,
            (hdmi_rx[port].hdcp_status.hdcp_enc) ? 1 : 0,
            lib_hdmi_phy_power_saving_status(port),
            hdmi_rx[port].bch_err_detect,
            hdmi_rx[port].bch_err_cnt,
            p_hdcp->hdcp2_auth_count);
        break;

    case NO_HDCP:
    case HDCP_OFF:
        PWR_FLOW_INFO("HDMI[p%d] FSM=%s (%d), Round=%d, RxStatus(%02x, %02x), hdcp_mode=%d, hdcp_enc=%d, phy_pwr_on=%d, bch error=%d, bch error cnt=%d\n",
            port, _hdmi_power_saving_fsm_str(hdmi_rx[port].ps_fsm), hdmi_rx[port].ps_fsm, hdmi_rx[port].ps_round,
            RxStatus[0], RxStatus[1],
            hdcp_mode,
            (hdmi_rx[port].hdcp_status.hdcp_enc) ? 1 : 0,
            lib_hdmi_phy_power_saving_status(port),
            hdmi_rx[port].bch_err_detect,
            hdmi_rx[port].bch_err_cnt);
    }
}


static unsigned int ps_msg_cnt = 0;
static HDMI_POWER_SAVEING_FSM_T m_last_ps_fsm[4] = {PS_FSM_POWER_SAVING_OFF, PS_FSM_POWER_SAVING_OFF, PS_FSM_POWER_SAVING_OFF, PS_FSM_POWER_SAVING_OFF};

void newbase_hdmi_power_saving_handler(unsigned char port)
{
    unsigned char frl_mode = newbase_rxphy_get_frl_mode(port);
    unsigned int ps_measure_stable_cnt;
    unsigned int ps_measure_vs2vs_cnt;
    unsigned int ps_measure_last_pixel2vs_count;

    _lock_power_saving_sem();

    switch(hdmi_rx[port].ps_fsm)
    {
    case PS_FSM_POWER_SAVING_OFF:

        if (newbase_hdmi_power_saving_check_condition(port))
        {
            hdmi_rx[port].ps_fsm_sub_state++;

            // for functional verification only
            // when test mode is enabled, do not enter power saving immediately
            // keep print signal status for serval seconds then enter power saving state
            if (hdmi_power_saving_test_mode && hdmi_rx[port].ps_fsm_sub_state < HDMI_POWER_SAVING_MIN_STABLE_CNT)
            {

                if ((hdmi_rx[port].ps_fsm_sub_state & 0x3F)==0)
                    newbase_hdmi_power_saving_dump_data_status(port);
                break;
            }

            // enter power saving state
            HDMI_INFO("HDMI[p%d] FSM=%s (%d), Round=%d, Ready for power saving, start ps measure\n",
                port, _hdmi_power_saving_fsm_str(hdmi_rx[port].ps_fsm), hdmi_rx[port].ps_fsm, hdmi_rx[port].ps_round);

            hdmi_rx[port].ps_fsm = PS_FSM_MEASURE;  // enter ps measure state
            hdmi_rx[port].ps_fsm_sub_state = 0;
            hdmi_rx[port].ps_round++;
        }
        else
        {
            if (hdmi_power_saving_enable && (hdmi_rx[port].ps_fsm_sub_state++ & 0x3F)==0 && hdmi_rx[port].signal_stable_cnt)
            {
                PWR_FLOW_INFO("HDMI[p%d] FSM=%s (%d), Round=%d, wait power saving signal stable ...(%d)\n",
                    port, _hdmi_power_saving_fsm_str(hdmi_rx[port].ps_fsm), hdmi_rx[port].ps_fsm, hdmi_rx[port].ps_round, hdmi_rx[port].signal_stable_cnt);
            }
        }
        break;

    case PS_FSM_MEASURE:

        // check power saving condition
        if (newbase_hdmi_power_saving_check_condition(port)==0)
        {
            PWR_FLOW_INFO("HDMI[p%d] FSM=%s (%d), Round=%d, not ready for power saving, stop ps measure\n",
                port, _hdmi_power_saving_fsm_str(hdmi_rx[port].ps_fsm), hdmi_rx[port].ps_fsm, hdmi_rx[port].ps_round);
            lib_hdmi_ps_measure_enable(port, 0);
            hdmi_rx[port].ps_fsm = PS_FSM_POWER_SAVING_OFF;
            hdmi_rx[port].ps_fsm_sub_state = 0;
            hdmi_rx[port].signal_stable_cnt = 0;
            break;
        }

        newbase_hdmi_get_ps_measure_status(port, &ps_measure_vs2vs_cnt, &ps_measure_last_pixel2vs_count, &ps_measure_stable_cnt);

        // waiting for measure stable
        if (ps_measure_stable_cnt > HDMI_POWER_SAVING_MIN_PS_MEASURE_STABLE_CNT)
        {
            PWR_FLOW_INFO("HDMI[p%d] FSM=%s (%d), Round=%d, ps measure stable vs2vs_cnt=%08x, last_pixel2vs_count=%08x, ps_stable_cnt=%d\n",
                port, _hdmi_power_saving_fsm_str(hdmi_rx[port].ps_fsm), hdmi_rx[port].ps_fsm, hdmi_rx[port].ps_round, 
                ps_measure_vs2vs_cnt, ps_measure_last_pixel2vs_count, ps_measure_stable_cnt);

            lib_hdmi_enter_power_saving_mode(port, frl_mode, hdmi_rx[port].scramble_flag, newbase_hdcp_get_auth_mode(port), ps_measure_last_pixel2vs_count, ps_measure_vs2vs_cnt);
            newbase_hdmi_power_saving_dump_status(port);
            hdmi_rx[port].ps_fsm = PS_FSM_POWER_SAVING_ON;
            hdmi_rx[port].ps_fsm_sub_state = 0;
        }
        else
        {
            if (ps_measure_stable_cnt==0 || hdmi_rx[port].ps_fsm_sub_state++ > HDMI_POWER_SAVING_MIN_STABLE_CNT)
            {
                PWR_FLOW_INFO("HDMI[p%d] FSM=%s (%d), Round=%d, wait ps measure stable timeout, stop power saving\n",
                    port, _hdmi_power_saving_fsm_str(hdmi_rx[port].ps_fsm), hdmi_rx[port].ps_fsm, hdmi_rx[port].ps_round);

                lib_hdmi_ps_measure_enable(port, 0);
                hdmi_rx[port].ps_fsm = PS_FSM_POWER_SAVING_OFF;
                hdmi_rx[port].ps_fsm_sub_state = 0;
                hdmi_rx[port].signal_stable_cnt = 0;
            }
            else
            {
                if ((hdmi_rx[port].ps_fsm_sub_state++ & 0x1F)==0)
                {
                    PWR_FLOW_INFO("HDMI[p%d] FSM=%s (%d), Round=%d, wait ps measure stable (ps_measure_stable_cnt=%d)\n",
                        port, _hdmi_power_saving_fsm_str(hdmi_rx[port].ps_fsm), hdmi_rx[port].ps_fsm, hdmi_rx[port].ps_round, ps_measure_stable_cnt);
                }
            }
        }
        break;

    case PS_FSM_POWER_SAVING_ON:

        if (newbase_hdmi_power_saving_check_condition(port)==0)
        {
            PWR_FLOW_INFO("HDMI[p%d] FSM=%s (%d), Round=%d, not ready for power saving, stop power saving\n",
                port, _hdmi_power_saving_fsm_str(hdmi_rx[port].ps_fsm), hdmi_rx[port].ps_fsm, hdmi_rx[port].ps_round);

            lib_hdmi_exit_power_saving_mode(port, frl_mode, hdmi_rx[port].scramble_flag, newbase_hdcp_get_auth_mode(port));
            hdmi_rx[port].ps_fsm = PS_FSM_POWER_SAVING_OFF;
            hdmi_rx[port].ps_fsm_sub_state = 0;
            hdmi_rx[port].signal_stable_cnt = 0;
        }
        else
        {
            if ((hdmi_rx[port].ps_fsm_sub_state++ & 0x7F)==0)
                newbase_hdmi_power_saving_dump_data_status(port);

            // for power saving test, when test mode enabled, keep prink signal status for a few second
            // to check the data correctness
            if (hdmi_power_saving_test_mode && hdmi_rx[port].ps_fsm_sub_state >= HDMI_POWER_SAVING_MAX_STABLE_CNT)
            {
                PWR_FLOW_INFO("HDMI[p%d] FSM=%s (%d), Round=%d, hdmi_power_saving_test_mode is on, exit leave power saving\n",
                    port, _hdmi_power_saving_fsm_str(hdmi_rx[port].ps_fsm), hdmi_rx[port].ps_fsm, hdmi_rx[port].ps_round);
                lib_hdmi_exit_power_saving_mode(port, frl_mode, hdmi_rx[port].scramble_flag, newbase_hdcp_get_auth_mode(port));
                newbase_hdmi_power_saving_dump_status(port);
                hdmi_rx[port].ps_fsm = PS_FSM_POWER_SAVING_OFF;
                hdmi_rx[port].ps_fsm_sub_state = 0;
                //hdmi_rx[port].signal_stable_cnt = 0;  // do not clear signal stable count...
            }
        }
        break;

    default:
        PWR_FLOW_INFO("HDMI[p%d] FSM=%s (%d), Round=%d, unknown poser state ???\n",
            port, _hdmi_power_saving_fsm_str(hdmi_rx[port].ps_fsm), hdmi_rx[port].ps_fsm, hdmi_rx[port].ps_round);
    }

    //PWR Print
    if(m_last_ps_fsm[port] != hdmi_rx[port].ps_fsm)
    {
        PWR_EMG("[FSM] Port [%d], PWR_FSM=%d(%s) -> %d(%s)\n",
            port, m_last_ps_fsm[port], _hdmi_power_saving_fsm_str(m_last_ps_fsm[port]), hdmi_rx[port].ps_fsm, _hdmi_power_saving_fsm_str(hdmi_rx[port].ps_fsm));

        m_last_ps_fsm[port] = hdmi_rx[port].ps_fsm;
        ps_msg_cnt = g_pwr_print_cycle_cnt_thd;
    }
    
    if(ps_msg_cnt++ >= g_pwr_print_cycle_cnt_thd)
    {
        if(hdmi_power_saving_enable)
        {
            PWR_FLOW_INFO("PWR_STS : FG_HDMI=%d, P0/P1/P2(active,fsm,state_cnt) =  P0(%d,%s,%d)/P1(%d,%s,%d)/P2(%d,%s,%d)\n",
                newbase_hdmi_is_dispsrc(0),
                _is_active_source(0), _hdmi_power_saving_fsm_str(hdmi_rx[0].ps_fsm), hdmi_rx[0].ps_fsm_sub_state,
                _is_active_source(1), _hdmi_power_saving_fsm_str(hdmi_rx[1].ps_fsm), hdmi_rx[1].ps_fsm_sub_state,
                _is_active_source(2), _hdmi_power_saving_fsm_str(hdmi_rx[2].ps_fsm), hdmi_rx[2].ps_fsm_sub_state);
        }

        ps_msg_cnt = 0;
    }
    _unlock_power_saving_sem();
}
