#include "hdmi_common.h"

#ifndef BUILD_QUICK_SHOW
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/string.h>
#endif

#include <rbus/audio_reg.h>
#include <mach/platform.h>
#include "hdmi_reg.h"
#include "hdmi_audio.h"
#include "hdmi_info_packet.h"
#include "hdmi_vfe_config.h"
#ifndef BUILD_QUICK_SHOW
#include <rtk_kdriver/rtkaudio.h>
#include <rtk_kdriver/quick_show/quick_show.h>
#endif
#include "hdmi_mac_2p1.h"
#include "hdmi_table.h"
/**********************************************************************************************
*
*   Variables
*
**********************************************************************************************/
extern HDMI_PORT_INFO_T hdmi_rx[HDMI_PORT_TOTAL_NUM];
extern unsigned char current_port;
#ifndef BUILD_QUICK_SHOW
extern wait_queue_head_t hdmi_thd_wait_qu;
#endif

static unsigned char audio_thread_run[HDMI_PORT_TOTAL_NUM];
static bool audio_thread_flag = false;
static struct task_struct *audio_task;

static int hdmi_audio_debug_print[HDMI_PORT_TOTAL_NUM];    // for debug print
static HDMI_AUDIO_FSM_T audio_print_last_fsm[HDMI_PORT_TOTAL_NUM];    // for debug print

// the manual HBR registers have been removed, so we use FW_FUNC_reg bit 11:10 to instead
#define HDMI_FW_FUNC_hbr_audio_mode_manual_mask  (0x1<<11)               // 0 : auto mode, 1: FW mode
#define HDMI_FW_FUNC_hbr_audio_mode_manual(data) ((data & 0x1)<<11)
#define HDMI_FW_FUNC_hbr_audio_mode_fw_mask      (0x1<<10)               // 0 : non hbr, 1 : hbr
#define HDMI_FW_FUNC_hbr_audio_mode_fw(data)     ((data & 0x1)<<10)
#define HDMI_FW_FUNC_get_hbr_audio_mode_fw(data) ((data >>10) & 0x1)

#define HDMI_NO_AUDIO_COUNT_THRESHOLD       3

/**********************************************************************************************
*
*   Audio Thread
*
**********************************************************************************************/

#ifndef BUILD_QUICK_SHOW

static int hdmi_audio_thread(void* arg)
{
    while (!kthread_should_stop() && audio_thread_flag == true)
    {
        newbase_hdmi_audio_detect();
        wait_event_interruptible_hrtimeout(hdmi_thd_wait_qu,(!audio_thread_flag),ktime_set(0,20*NSEC_PER_MSEC));
    }

    return 0;
}

void newbase_hdmi_audio_thread_enable(unsigned char create)
{
    int err = 0;
    int qs_portnum = 0;
    unsigned char nport = 0;

    if (create)
    {
        if (audio_thread_flag!=true)
        {
            audio_task = kthread_create(hdmi_audio_thread, NULL, "audio_task");

            if (IS_ERR(audio_task))
            {
                err = PTR_ERR(audio_task);
                audio_task = NULL;

                audio_thread_flag = false;
                HDMI_EMG("[AUDIO] [newbase_hdmi_audio_thread_enable] , Unable to start kernel thread (err_id = %d), audio\n",err);
                return ;
            }

            audio_thread_flag = true;

            qs_portnum = get_QS_portnum();

            if(is_QS_hdmi_enable() && (!(qs_portnum < HDMI_PORT0 || qs_portnum >= HDMI_PORT_TOTAL_NUM)) \
                && (hdmi_vfe_get_hdmi_port((unsigned char)qs_portnum, &nport) == 0))
                newbase_hdmi_audio_set_qs_status(nport);
            else
                newbase_hdmi_audio_reset_status();
            wake_up_process(audio_task);

            HDMI_PRINTF("[AUDIO] [newbase_hdmi_audio_thread_enable] hdmi audio thread started\n");
        }
    }
    else
    {
        if (audio_thread_flag == true)
        {
            audio_thread_flag = false;
            wake_up(&hdmi_thd_wait_qu);
            if(audio_task)
            {
                if (kthread_stop(audio_task)==0)
                {
                    HDMI_PRINTF("[AUDIO] hdmi audio thread stopped\n");
                    audio_task = NULL;
                }
                else
                    HDMI_EMG("[AUDIO] hdmi audio thread stopped error\n");
            }
            else
                HDMI_EMG("[AUDIO] hdmi audio thread stopped already\n");
        }
    }
}

#endif
unsigned char lib_hdmi_audio_get_thread_run(unsigned char nport)
{
    return audio_thread_run[nport];
}

/**********************************************************************************************
*
*   Internal Functin
*
**********************************************************************************************/

const char* _hdmi_audio_fsm_str(unsigned int fsm)
{
    switch (fsm) {
    case AUDIO_FSM_AUDIO_START:          return "START";
    case AUDIO_FSM_FREQ_DETECT:          return "DETECT";
    case AUDIO_FSM_AUDIO_WAIT_TO_START:  return "WAIT TO START";
    case AUDIO_FSM_AUDIO_START_OUT:      return "START OUT";
    case AUDIO_FSM_AUDIO_WAIT_PLL_READY: return "WAIT PLL READY";
    case AUDIO_FSM_AUDIO_CHECK:          return "CHECK";
    default:                             return "UNKNOW";
    }
}

const char* _hdmi_audio_fmt_str(unsigned char fmt)
{
    switch (fmt) {    
    case AUDIO_FORMAT_DVI:          return "DVI";
    case AUDIO_FORMAT_NO_AUDIO:     return "NO_AUDIO";
    case AUDIO_FORMAT_PCM:          return "LPCM";
    case AUDIO_FORMAT_AC3:          return "AC3";
    case AUDIO_FORMAT_DTS:          return "DTS";
    case AUDIO_FORMAT_AAC:          return "AAC";
    case AUDIO_FORMAT_DEFAULT:      return "Default";
    case AUDIO_FORMAT_MPEG:         return "MPEG";
    case AUDIO_FORMAT_DTS_HD_MA:    return "DTS_HDMA";
    case AUDIO_FORMAT_DTS_EXPRESS:  return "DTS_EXPRESS";
    case AUDIO_FORMAT_DTS_CD:       return "DTS_CD";
    case AUDIO_FORMAT_EAC3:         return "EAC3";
    case AUDIO_FORMAT_EAC3_ATMOS:   return "EAC3_ATMOS";
    case AUDIO_FORMAT_MAT:          return "MAT";
    case AUDIO_FORMAT_MAT_ATMOS:    return "MAT_ATMOS";
    case AUDIO_FORMAT_TRUEHD:       return "TRUEHD";
    case AUDIO_FORMAT_TRUEHD_ATMOS: return "TRUEHD-ATMOS";
    default:                        return "UNKNOW";
    }
}

void lib_hdmi_audio_set_thread_run(unsigned char nport, unsigned char uc_run)
{
//  HDMI_PRINTF("lib_hdmi_audio_set_thread_run[%d]=%d\n", nport, uc_run);
    audio_thread_run[nport] = uc_run;
}


void newbase_hdmi_audio_var_init(unsigned char nport)
{
    memset(&hdmi_rx[nport].audio_t, 0, sizeof(hdmi_rx[nport].audio_t));
}


unsigned int _hdmi_audio_freq_correction(unsigned int freq)
{
    unsigned char is_correct = FALSE;
    unsigned char i = 0;
    for (i=0; i<g_audio_freq_table_size; i++)
    {
        if (HDMI_ABS(freq, g_audio_freq_table[i].freq) <= g_audio_freq_table[i].freq_tol)
        {
            if (freq !=  g_audio_freq_table[i].freq)
            {
                ;//HDMI_EMG("[HDMI][OFMS] Corrected acr freq=%d to %d\n", freq, audio_freq_table[i].freq);
            }
            freq = g_audio_freq_table[i].freq;

            is_correct = TRUE;
            break;
        }
    }

    if(is_correct == FALSE)
    {
        HDMI_EMG("non standard audio sampling frequency %d Hz detected\n", freq);
    }

    return freq;
}


/*---------------------------------------------------
 * Func : _compute_acr_frequency
 *
 * Desc : Get Audio Frequency from CTS & N
 *
 * Para : n : ACR N value
 *        cts : ACR CTS value
 *        b : TMDS clock index
 *        frl mode : current FRL mode
 *
 * Retn : 0 : no valid audio frequency,
 *        others : ACR frequency
 *--------------------------------------------------*/
unsigned int _compute_acr_frequency(unsigned int n, unsigned int cts, unsigned int b, HDMI_FRL_MODE_T frl_mode)
{
    unsigned int acr_freq = 0;

    //HDMI_EMG("(cts=%d)(n=%d)(b=%d)(frl_mode=%d)\n",cts, n, b, frl_mode);   // cts will

    if (cts == 0 || n == 0 || b == 0)
        return 0;

    if(frl_mode == MODE_TMDS)
    {
        // 128fs = 1024/b * fx * N / CTS  =>  fs = (1024 * fx *N)/(128 * b * CTS) = (8 * fx *N)/(b*CTS)
        // calculate freq in 0.1kHz unit
        // freq = (unsigned long)8 * 2 * 10000 * HDMI_RTD_XTAL/ cts * n / ((unsigned long)b * 1000);
        acr_freq = ((((270000 * b)/256)/128) * n) / (cts);
        acr_freq *= 100;
    }
    else
    {
        unsigned int clock = 0;
        unsigned int temp_freq = 0;
        unsigned char current_frl_rate = newbase_hdmi_hd21_get_frl_rate(frl_mode);
        clock = 1000000*current_frl_rate/18; // 1000000 = 1000000000/1000;  divide 1000 first to avoid overflow.

        temp_freq = (((clock/128)*n ) / (cts))*1000;
        acr_freq = temp_freq;
    }

    return acr_freq;
}


/*---------------------------------------------------
 * Func : newbase_hdmi_audio_update_acr
 *
 * Desc : update ACR freuency
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : ACR freq not update, 1 : ACR updated
 *--------------------------------------------------*/
int newbase_hdmi_audio_update_acr(unsigned char nport, HDMI_FRL_MODE_T frl_mode)
{
    int acr_updated = 0;
    unsigned int cts, n , b;
    unsigned int acr_freq = 0;
    HDMI_AUDIO_ST* p_this = &hdmi_rx[nport].audio_t;
    MEASURE_TIMING_T* p_timing = &hdmi_rx[nport].timing_t;
    unsigned char hbr_event =  (p_this->hbr_mode != lib_hdmi_audio_get_hbr_manual_mode(nport));

    if (lib_hdmi_audio_pop_n_cts_done(nport) || hbr_event)
    {
        cts = lib_hdmi_audio_get_cts(nport);
        n   = lib_hdmi_audio_get_n(nport);
        b   = p_timing->tmds_clk_b;

        lib_hdmi_audio_pop_n_cts(nport);    // restart n & CTS pop up

        if (p_this->acr_info.n != n ||
            p_this->acr_info.cts != cts ||
            p_this->acr_info.b != b ||
            hbr_event)
        {
            acr_freq = _compute_acr_frequency(n, cts, b, frl_mode);

            acr_freq = _hdmi_audio_freq_correction(acr_freq);    // correct audio freq (to reduce small variation)

            if (lib_hdmi_audio_get_hbr_manual_mode(nport))
            {
                if (acr_freq != 192000) {  //764K / 4
                    HDMI_WARN("[AUDIO][newbase_hdmi_audio_update_acr] (Port:%d, frl_mode:%d(%s)) WARNING!!! invaild acr freq %d, HBR need to equal 192K\n", nport, frl_mode, _hdmi_hd21_frl_mode_str(frl_mode), acr_freq);
                    acr_freq = 192000;
                }
            }

            if (hbr_event)
            {
                AUDIO_FLOW_PRINTF("ACR updated (Port:%d) HBR mode changed (%d -> %d)\n", nport, p_this->hbr_mode, lib_hdmi_audio_get_hbr_manual_mode(nport));
                p_this->hbr_mode = lib_hdmi_audio_get_hbr_manual_mode(nport);
            }

            if (p_this->acr_info.acr_freq != acr_freq)
                acr_updated = 1;

            p_this->acr_info.n = n;
            p_this->acr_info.cts = cts;
            p_this->acr_info.b = b;
            p_this->acr_info.acr_freq = acr_freq;

            if (acr_updated)
            {
                AUDIO_FLOW_PRINTF("ACR updated(Port:%d, frl_mode:%d(%s)) : cts=%d, n=%d, b=%d, ARC freq=%d\n",
                    nport,
                    frl_mode,
                    _hdmi_hd21_frl_mode_str(frl_mode),
                    p_this->acr_info.cts,
                    p_this->acr_info.n,
                    p_this->acr_info.b,
                    p_this->acr_info.acr_freq);
            }
        }
    }

    return acr_updated;
}

unsigned char newbase_hdmi_audio_detect(void)
{
    unsigned char i;
    unsigned char port = current_port;
    HDMIRX_PHY_STRUCT_T* p_phy_st;
    p_phy_st = newbase_rxphy_get_status(port);

    if (lib_hdmi_get_fw_debug_bit(DEBUG_29_BYPASS_AUDIO_DET))
        return FALSE;

    if(_FALSE == lib_hdmi_audio_get_thread_run(port))
        return FALSE;

    if (((GET_H_VIDEO_FSM(port) != MAIN_FSM_HDMI_DISPLAY_ON) &&
        (GET_H_VIDEO_FSM(port) != MAIN_FSM_HDMI_VIDEO_READY)) ||
        (lib_hdmi_get_avmute(port) && !lib_hdmi_get_bch_2bit_error(port)) ||
        (hdmi_rx[port].timing_t.tmds_clk_b < VALID_MIN_CLOCK) ||
        (newbase_hdmi_get_power_saving_state(port)==PS_FSM_POWER_SAVING_ON))
    {
        lib_hdmi_audio_output(port, 0);
        SET_H_AUDIO_FSM(port, AUDIO_FSM_AUDIO_START);
        return FALSE;
    }

    switch (GET_H_AUDIO_FSM(port))
    {
    case AUDIO_FSM_AUDIO_START:
        for (i=0; i<HDMI_PORT_TOTAL_NUM; i++)
        {
            SET_H_AUDIO_FSM(i, AUDIO_FSM_AUDIO_START);
            lib_hdmi_audio_init(i);
        }
        
        lib_hdmi_audio_output(port, 0);
        lib_hdmi_audio_set_hbr_manual_mode(port, 0, 0);
        lib_hdmi_audio_source_select(port, AUDIO_SRC_NORMAL_AUDIO);
        hdmi_rx[port].audio_t.sr_mismatch_cnt = 0;
        hdmi_rx[port].audio_t.coding_type = AUDIO_FORMAT_NO_AUDIO;
        hdmi_rx[port].audio_t.channel_mode = 0;
        hdmi_rx[port].audio_t.channel_num = 0;
        // reset audio sample count
        hdmi_rx[port].audio_t.audio_sample_cnt = 0;
        hdmi_rx[port].audio_t.hbr_audio_sample_cnt = 0;
        hdmi_rx[port].audio_t.no_audio_cnt = 0;
        hdmi_rx[port].audio_t.wait_acr_cnt = 0;
        // reset ACR info
        memset(&hdmi_rx[port].audio_t.acr_info, 0, sizeof(hdmi_rx[port].audio_t.acr_info));

#if AUDIO_HBR_DETECT
        if (!newbase_hdmi_audio_wait_2_samples(port)) {
            break;
        }
#else
        if (!newbase_hdmi_audio_wait_sample(port)) {
            //HDMI_PRINTF("No audio sample(p%d)\n", port);
            break; //need to wait for audio sample packet
        }
#endif
        SET_H_AUDIO_FSM(port, AUDIO_FSM_AUDIO_START_OUT);        
#ifndef BUILD_QUICK_SHOW // quick show do not break
        break;
#endif
        
    case AUDIO_FSM_AUDIO_START_OUT:

        if (lib_hdmi_get_avmute(port) && !lib_hdmi_get_bch_2bit_error(port))
        {
            // Humm... do we have to stop audio here ???
            lib_hdmi_audio_output(port, 0);
            SET_H_AUDIO_FSM(port, AUDIO_FSM_AUDIO_START);
            HDMI_PRINTF("Audio Output Disable cause by AVMCR output disable\n");
        }
        else
        {
            if (newbase_hdmi_audio_update_acr(port, p_phy_st->frl_mode))
            {
                AUDIO_FLOW_PRINTF("got audio info and update ACR - %d\n", hdmi_rx[port].audio_t.acr_info.acr_freq);
                SET_AUDIO_ACR_FREQ(hdmi_rx[port].audio_t.acr_info.acr_freq);
            }
            else
            {
                hdmi_rx[port].audio_t.wait_acr_cnt ++;
                break; // no ACR, Waiting for update ACR
            }

            AUDIO_FLOW_PRINTF("AUDIO_FSM_AUDIO_START_OUT[%d]\n", port);
            
            if (lib_hdmi_audio_get_hbr_manual_mode(port))
                lib_hdmi_audio_source_select(port, AUDIO_SRC_HBR_AUDIO);       // change audio output mode to hbr
            
            SET_H_AUDIO_FSM(port, AUDIO_FSM_AUDIO_CHECK);
        }
#ifndef BUILD_QUICK_SHOW
        break;
#else
        // quick show do not break and enable audio output
        lib_hdmi_audio_output(port, 1);
#endif
        
    case AUDIO_FSM_AUDIO_CHECK:
        
        if (!newbase_hdmi_is_display_ready())
        {
            if (lib_hdmi_audio_is_output(port)) {
                AUDIO_FLOW_PRINTF("Audio Output Disable due to hdmi is not ready to display\n");
                lib_hdmi_audio_output(port, 0);  // disable audio, keep audio thread running
            }
        }
        else
        {            
            if (!lib_hdmi_audio_is_output(port)) {
                AUDIO_FLOW_PRINTF("Audio Output Enable due to hdmi is ready to display\n");
                lib_hdmi_audio_output(port, 1);
            }
        }

        if (lib_hdmi_get_avmute(port) && !lib_hdmi_get_bch_2bit_error(port))
        {
            // Humm... do we have to stop audio here ???
            lib_hdmi_audio_output(port, 0);
            SET_H_AUDIO_FSM(port, AUDIO_FSM_AUDIO_START);
            AUDIO_FLOW_PRINTF("Audio Output Disable cause by AVMCR output disable\n");
        }
        else 
        {
            hdmi_rx[port].audio_t.no_audio_cnt ++;

            // monitor audio statsus
            if (newbase_hdmi_is_audiosampkt_received(port)) {
                hdmi_rx[port].audio_t.audio_sample_cnt++;
                if (lib_hdmi_audio_get_hbr_manual_mode(port)==0)  // reset no audio count when normal audio is received
                    hdmi_rx[port].audio_t.no_audio_cnt = 0;
            }

            if (newbase_hdmi_is_hbr_audiostreampkt_received(port)) {
                hdmi_rx[port].audio_t.hbr_audio_sample_cnt++;
                if (lib_hdmi_audio_get_hbr_manual_mode(port)) // reset no audio count when HBR audio is received
                    hdmi_rx[port].audio_t.no_audio_cnt = 0;
            }

            if (hdmi_rx[port].audio_t.no_audio_cnt > HDMI_NO_AUDIO_COUNT_THRESHOLD)
            {
                AUDIO_FLOW_PRINTF("no Audio sample packet detect reset audio flow\n");
                SET_H_AUDIO_FSM(port, AUDIO_FSM_AUDIO_START);
                hdmi_rx[port].audio_t.no_audio_cnt = 0;
                break;
            }

            if (lib_hdmi_audio_is_fifo_overflow(port)) {
                hdmi_rx[port].audio_t.fifo_ov++;
                lib_hdmi_audio_wclr_fifo_overflow(port);
                hdmi_audio_debug_print[port] = g_audio_print_cycle_thd;  // force print warning message
            }

            if (hdmi_rx[port].audio_t.coding_type != lib_hdmi_audio_get_coding_type(port))
            {
                hdmi_rx[port].audio_t.coding_type = lib_hdmi_audio_get_coding_type(port);
                hdmi_audio_debug_print[port] = g_audio_print_cycle_thd;  // force print warning message
            }

            if (hdmi_rx[port].audio_t.channel_mode != lib_hdmi_audio_get_channel_mode(port)) 
            {
                hdmi_rx[port].audio_t.channel_mode = lib_hdmi_audio_get_channel_mode(port);
                hdmi_audio_debug_print[port] = g_audio_print_cycle_thd;  // force print warning message
            }

            if (hdmi_rx[port].audio_t.channel_num != lib_hdmi_audio_get_channel_num(port)) 
            {
                hdmi_rx[port].audio_t.channel_num = lib_hdmi_audio_get_channel_num(port);
            }

            if (newbase_hdmi_audio_update_acr(port, p_phy_st->frl_mode))
            {
                SET_AUDIO_ACR_FREQ(hdmi_rx[port].audio_t.acr_info.acr_freq);
                hdmi_audio_debug_print[port] = g_audio_print_cycle_thd;  // force print warning message
            }
        }

        break;

    default:
        SET_H_AUDIO_FSM(port, AUDIO_FSM_AUDIO_START);
        return FALSE;;
    }

    if(audio_print_last_fsm[port] != GET_H_AUDIO_FSM(port))
    {
        audio_print_last_fsm[port] = GET_H_AUDIO_FSM(port);
        hdmi_audio_debug_print[port] = g_audio_print_cycle_thd; 
    }

    if (hdmi_audio_debug_print[port]++ >= g_audio_print_cycle_thd) 
    {
        AUDIO_FLOW_PRINTF("Audio STS : FSM=%d(%s), hbr=%d/%d, cnt (s/h) = (%d/%d), fifo_ov=%d, type=%d(%s), ch mode/num =(%d/%d), acr freq/wait_cnt = (%d/%d), cs1/2 = (0x%x/0x%x)\n",
            GET_H_AUDIO_FSM(port), _hdmi_audio_fsm_str(GET_H_AUDIO_FSM(port)),
            lib_hdmi_audio_get_hbr_manual_mode(port), 
            lib_hdmi_audio_get_hbr_mode(port),
            hdmi_rx[port].audio_t.audio_sample_cnt, 
            hdmi_rx[port].audio_t.hbr_audio_sample_cnt,
            hdmi_rx[port].audio_t.fifo_ov,
            hdmi_rx[port].audio_t.coding_type, _hdmi_audio_fmt_str(hdmi_rx[port].audio_t.coding_type),
            hdmi_rx[port].audio_t.channel_mode,
            hdmi_rx[port].audio_t.channel_num,
            hdmi_rx[port].audio_t.acr_info.acr_freq,
            hdmi_rx[port].audio_t.wait_acr_cnt,
            hdmi_in(AUDIO_hdmi_to_audio_reg),
            hdmi_in(AUDIO_hdmi_to_audio_debug_reg));

        hdmi_audio_debug_print[port] = 0;
    }

    return TRUE;
}
void newbase_hdmi_audio_set_qs_status(unsigned char port)
{
    lib_hdmi_audio_output(port, 1);
    lib_hdmi_audio_set_thread_run(port, _TRUE);
    SET_H_AUDIO_FSM(port, AUDIO_FSM_AUDIO_CHECK);
}

void newbase_hdmi_audio_close(unsigned char port)
{
    lib_hdmi_audio_output(port, 0);
    lib_hdmi_audio_set_thread_run(port, _FALSE);
    SET_H_AUDIO_FSM(port, AUDIO_FSM_AUDIO_START);
}

void newbase_hdmi_audio_reset_status(void)
{
    unsigned char i;
    for (i=0; i<HDMI_PORT_TOTAL_NUM; i++)
        SET_H_AUDIO_FSM(i, AUDIO_FSM_AUDIO_START);
}

unsigned char newbase_hdmi_audio_get_current_status(void)
{
    return GET_H_AUDIO_FSM(current_port);
}

unsigned char newbase_hdmi_audio_wait_sample(unsigned char port)
{   //Audio Sample (L-PCM and IEC 61937 compressed formats)
    unsigned char i;
    lib_hdmi_clear_audiosampkt_received_status(port);

    for (i=0; i<5; i++)
    {
        if (newbase_hdmi_is_audiosampkt_received(port)==1) {  // got audio sample
            lib_hdmi_audio_set_hbr_manual_mode(port, 1, 0);
            return TRUE;
        }
        msleep(10);
    }

    return FALSE;
}


unsigned char newbase_hdmi_audio_wait_2_samples(unsigned char port)
{   // Audio Sample (02) (L-PCM and IEC 61937 compressed formats)
    //HBR sample (09)
    unsigned char i;
    lib_hdmi_clear_audiosampkt_received_status(port);
    lib_hdmi_clear_hbr_audiostreampkt_received_status(port);
    for (i=0; i<5; i++) 
    {
        if (newbase_hdmi_is_hbr_audiostreampkt_received(port)==1 || lib_hdmi_audio_get_hbr_mode(port))
        {
            lib_hdmi_audio_set_hbr_manual_mode(port, 1, 1);
            AUDIO_FLOW_PRINTF("Got HBR packet(p%d)\n", port);
            return TRUE;
        }
        msleep(10);
    }
    if (newbase_hdmi_is_audiosampkt_received(port)==1)
    {
        lib_hdmi_audio_set_hbr_manual_mode(port, 1, 0);  //follow up Merlin-1 setting, review this later
        return TRUE;
    }

    return FALSE;
}


void lib_hdmi_audio_init(unsigned char nport)
{
    lib_hdmi_audio_output(nport, 0);

    //audio debuggin print  variable
    memset(hdmi_audio_debug_print, 0, sizeof(hdmi_audio_debug_print));
    memset(audio_print_last_fsm, AUDIO_FSM_AUDIO_START, sizeof(audio_print_last_fsm));
}


void lib_hdmi_audio_pop_n_cts(unsigned char nport)
{
    if (hdmi_rx[nport].hdmi_2p1_en)
    {
        hdmi_mask(HD21_HDMI_ACRCR_reg, ~(HDMI21_P0_HD21_HDMI_ACRCR_pucnr_mask), (HDMI21_P0_HD21_HDMI_ACRCR_pucnr_mask));
        return ;
    }
    
    hdmi_mask(HDMI_HDMI_ACRCR_reg, ~(HDMI_HDMI_ACRCR_pucnr_mask), (HDMI_HDMI_ACRCR_pucnr_mask));
}

unsigned char lib_hdmi_audio_pop_n_cts_done(unsigned char nport)
{
    if (hdmi_rx[nport].hdmi_2p1_en)
        return ((hdmi_in(HD21_HDMI_ACRCR_reg) & (HDMI21_P0_HD21_HDMI_ACRCR_pucnr_mask))==0);
            
    return ((hdmi_in(HDMI_HDMI_ACRCR_reg) & (HDMI_HDMI_ACRCR_pucnr_mask))==0);
}

unsigned int lib_hdmi_audio_get_n(unsigned char nport)
{
    if (hdmi_rx[nport].hdmi_2p1_en)
        return HDMI21_P0_HD21_HDMI_ACRSR1_get_n(hdmi_in(HD21_HDMI_ACRSR1_reg));
            
    return HDMI_HDMI_ACRSR1_get_n(hdmi_in(HDMI_HDMI_ACRSR1_reg));
}

unsigned int lib_hdmi_audio_get_cts(unsigned char nport)
{
    if (hdmi_rx[nport].hdmi_2p1_en)
        return HDMI21_P0_HD21_HDMI_ACRSR0_get_cts(hdmi_in(HD21_HDMI_ACRSR0_reg));
        
    return HDMI_HDMI_ACRSR0_get_cts(hdmi_in(HDMI_HDMI_ACRSR0_reg));
}


void lib_hdmi_audio_cts_bound(unsigned char nport, unsigned int clk)
{
    unsigned long cts_up, cts_low;

    if (clk > 5600) { //6G timing
        cts_up = 0xfffff ;//990000;
        cts_low = 445500 *7/8 ;
    } else if (clk > 2750) { //   3G timing
        cts_up = 421875 * 9/8 ; //990000;
        cts_low = 222750 *7/8 ;
    } else if (clk > 1380) { //148M
        cts_up = 421875 * 9/8  ;//990000;
        cts_low = 140625 *7/8 ;
    } else if (clk > 660) {
        cts_up = 234375 * 9/8  ;//990000;
        cts_low = 74250 *7/8 ;
    } else {
        cts_up = 60060 * 9/8  ;//990000;
        cts_low = 25200 *7/8 ;
    }
    
    if (hdmi_rx[nport].hdmi_2p1_en) 
    {
        hdmi_mask(HD21_AUDIO_CTS_UP_BOUND_reg,~(HDMI21_P0_HD21_AUDIO_CTS_UP_BOUND_cts_up_bound_mask),HDMI21_P0_HD21_AUDIO_CTS_UP_BOUND_cts_up_bound(cts_up));
        hdmi_mask(HD21_AUDIO_CTS_LOW_BOUND_reg,~HDMI21_P0_HD21_AUDIO_CTS_LOW_BOUND_cts_low_bound_mask,HDMI21_P0_HD21_AUDIO_CTS_LOW_BOUND_cts_low_bound(cts_low));     //CTS low boundary set 20000
    }
    else
    {
        hdmi_mask(HDMI_AUDIO_CTS_UP_BOUND_reg,~(HDMI_AUDIO_CTS_UP_BOUND_cts_up_bound_mask),HDMI_AUDIO_CTS_UP_BOUND_cts_up_bound(cts_up));
        hdmi_mask(HDMI_AUDIO_CTS_LOW_BOUND_reg,~HDMI_AUDIO_CTS_LOW_BOUND_cts_low_bound_mask,HDMI_AUDIO_CTS_LOW_BOUND_cts_low_bound(cts_low));     //CTS low boundary set 20000
    }
}


void lib_hdmi_audio_n_bound(unsigned char nport, unsigned int freq)
{
    int N_up, N_low;

    if (freq>190000) { //192k
        N_up = 46592 *9/8 ;//;
        N_low = 20480 *7/8 ;
    } else if (freq >170000) { //   176.4
        N_up = 71344 * 9/8 ; //990000;
        N_low = 17836 *7/8 ;
    } else if (freq >80000) { //   96 k 88.2k
        N_up = 35672 * 9/8  ;//990000;
        N_low = 8918 *7/8 ;
    } else { //32k  44.1k  48k
        N_up = 17836 * 9/8  ;//990000;
        N_low = 3072 *7/8 ;
    }

    if (hdmi_rx[nport].hdmi_2p1_en) 
    {
        hdmi_mask(HD21_AUDIO_N_UP_BOUND_reg,~(HDMI21_P0_HD21_AUDIO_N_UP_BOUND_n_up_bound_mask),HDMI21_P0_HD21_AUDIO_N_UP_BOUND_n_up_bound(N_up));
        hdmi_mask(HD21_AUDIO_N_LOW_BOUND_reg,~HDMI21_P0_HD21_AUDIO_N_LOW_BOUND_n_low_bound_mask,HDMI21_P0_HD21_AUDIO_N_LOW_BOUND_n_low_bound(N_low));     //CTS low boundary set 20000
    }
    else
    {
        hdmi_mask(HDMI_AUDIO_N_UP_BOUND_reg,~(HDMI_AUDIO_N_UP_BOUND_n_up_bound_mask),HDMI_AUDIO_N_UP_BOUND_n_up_bound(N_up));
        hdmi_mask(HDMI_AUDIO_N_LOW_BOUND_reg,~HDMI_AUDIO_N_LOW_BOUND_n_low_bound_mask,HDMI_AUDIO_N_LOW_BOUND_n_low_bound(N_low));     //CTS low boundary set 20000
    }
    
    AUDIO_FLOW_PRINTF("Port:%d, freq = %d Audio_N_Bound H = %d , L =%d\n", nport, freq, N_up, N_low);
}


unsigned char lib_hdmi_audio_is_fifo_overflow(unsigned char nport)
{
    if (hdmi_rx[nport].hdmi_2p1_en) 
        return HDMI21_P0_HD21_HDMI_AUDSR_get_aud_fifof(hdmi_in(HD21_HDMI_AUDSR_reg));
        
    return HDMI_HDMI_AUDSR_get_aud_fifof(hdmi_in(HDMI_HDMI_AUDSR_reg));
}

unsigned char lib_hdmi_audio_get_channel_mode(unsigned char nport)
{
    if (hdmi_rx[nport].hdmi_2p1_en) 
        return HDMI21_P0_HD21_HDMI_AUDSR_get_aud_ch_mode(hdmi_in(HD21_HDMI_AUDSR_reg));
        
    return HDMI_HDMI_AUDSR_get_aud_ch_mode(hdmi_in(HDMI_HDMI_AUDSR_reg));
}

void lib_hdmi_audio_wclr_fifo_overflow(unsigned char nport)
{
    if (hdmi_rx[nport].hdmi_2p1_en) { 
        hdmi_out(HD21_HDMI_AUDSR_reg, HDMI21_P0_HD21_HDMI_AUDSR_aud_fifof_mask);
        return ;
    }
    
    hdmi_out(HDMI_HDMI_AUDSR_reg, HDMI_HDMI_AUDSR_aud_fifof_mask);
}


unsigned char lib_hdmi_audio_is_output(unsigned char nport)
{
    if (hdmi_rx[nport].hdmi_2p1_en)
        return HDMI21_P0_HD21_HDMI_AUDCR_get_aud_en(hdmi_in(HD21_HDMI_AUDCR_reg));
        
    return HDMI_HDMI_AUDCR_get_aud_en(hdmi_in(HDMI_HDMI_AUDCR_reg));
}


void lib_hdmi_audio_output(unsigned char nport, unsigned char on)
{
    if (on)
    {      
        if (!lib_hdmi_audio_is_output(nport))  
        {
            if (hdmi_rx[nport].hdmi_2p1_en)
            {
                hdmi_mask(HD21_HDMI_AUDCR_reg, ~HD21_HDMI_AUDCR_aud_en_mask, HD21_HDMI_AUDCR_aud_en(1));
                hdmi_mask(HD21_HDMI_AUDCR_reg, ~HD21_HDMI_AUDCR_flat_en_mask, HD21_HDMI_AUDCR_flat_en(1));

                hdmi_out(HD21_HDMI_AUDSR_reg, HDMI21_P0_HD21_HDMI_AUDSR_aud_fifof_mask);  // clear audio fifo over flow status
            }
            else
            {

                hdmi_mask(HDMI_HDMI_AUDCR_reg, ~HDMI_HDMI_AUDCR_aud_en_mask, HDMI_HDMI_AUDCR_aud_en(1));
                hdmi_mask(HDMI_HDMI_AUDCR_reg, ~HDMI_HDMI_AUDCR_flat_en_mask, HDMI_HDMI_AUDCR_flat_en(1));

                hdmi_out(HDMI_HDMI_AUDSR_reg, HDMI_HDMI_AUDSR_aud_fifof_mask);  // clear audio fifo over flow status
            }
            AUDIO_FLOW_PRINTF("Port:%d, on=%d, Audio output start\n", nport, on);
        }
    }
    else
    {
        if (lib_hdmi_audio_is_output(nport))  
        {
            if (hdmi_rx[nport].hdmi_2p1_en)
            {
                hdmi_mask(HD21_HDMI_AUDCR_reg, ~HD21_HDMI_AUDCR_aud_en_mask, HD21_HDMI_AUDCR_aud_en(0));
                hdmi_mask(HD21_HDMI_AUDCR_reg, ~HD21_HDMI_AUDCR_flat_en_mask, HD21_HDMI_AUDCR_flat_en(0));
                hdmi_out(HD21_HDMI_AUDSR_reg, HDMI21_P0_HD21_HDMI_AUDSR_aud_fifof_mask);  // clear audio fifo over flow status
            }
            else
            {
                hdmi_mask(HDMI_HDMI_AUDCR_reg, ~HDMI_HDMI_AUDCR_aud_en_mask, HDMI_HDMI_AUDCR_aud_en(0));
                hdmi_mask(HDMI_HDMI_AUDCR_reg, ~HDMI_HDMI_AUDCR_flat_en_mask, HDMI_HDMI_AUDCR_flat_en(0));
                hdmi_out(HDMI_HDMI_AUDSR_reg, HDMI_HDMI_AUDSR_aud_fifof_mask);  // clear audio fifo over flow status
            }
            AUDIO_FLOW_PRINTF("Port:%d, on=%d,  Audio output stopped\n", nport, on);
        }
    }    
}

void lib_hdmi_audio_source_select(unsigned char nport, unsigned char src)
{
    #define HDMI_FW_FUNC_pck_hbr_aud_gating_mask        (1<<11)
    #define HDMI_FW_FUNC_pck_aud_gating_mask            (1<<10)
    
    switch(src)
    {
    case AUDIO_SRC_NORMAL_AUDIO:
        hdmi_mask(HDMI_FW_FUNC_reg, ~(HDMI_FW_FUNC_pck_hbr_aud_gating_mask | HDMI_FW_FUNC_pck_aud_gating_mask), 
            HDMI_FW_FUNC_pck_hbr_aud_gating_mask);
        break;
    case AUDIO_SRC_HBR_AUDIO:
        hdmi_mask(HDMI_FW_FUNC_reg, ~(HDMI_FW_FUNC_pck_hbr_aud_gating_mask | HDMI_FW_FUNC_pck_aud_gating_mask), 
            HDMI_FW_FUNC_pck_aud_gating_mask);
        break;
    case AUDIO_SRC_BOTH:
        hdmi_mask(HDMI_FW_FUNC_reg, ~(HDMI_FW_FUNC_pck_hbr_aud_gating_mask | HDMI_FW_FUNC_pck_aud_gating_mask), 0);
        break;
    default:
    case AUDIO_SRC_NONE:
        hdmi_mask(HDMI_FW_FUNC_reg, ~(HDMI_FW_FUNC_pck_hbr_aud_gating_mask | HDMI_FW_FUNC_pck_aud_gating_mask), 
            HDMI_FW_FUNC_pck_hbr_aud_gating_mask | HDMI_FW_FUNC_pck_aud_gating_mask);            
        break;
    }    
}

void lib_hdmi_audio_set_hbr_manual_mode(unsigned char nport, unsigned char manu, unsigned char mode_fw)
{
    // here we use HDMI_TMDS_PWDCTL_reg bit 10/11 for HBR control
    hdmi_mask(HDMI_TMDS_PWDCTL_reg, ~(HDMI_FW_FUNC_hbr_audio_mode_manual_mask | HDMI_FW_FUNC_hbr_audio_mode_fw_mask),
                                 (HDMI_FW_FUNC_hbr_audio_mode_manual(manu)| HDMI_FW_FUNC_hbr_audio_mode_fw(mode_fw))); 

    lib_hdmi_audio_clear_hbr_status(nport);         // clear auto hbr status
}

void lib_hdmi_audio_clear_hbr_status(unsigned char nport)
{
    if (hdmi_rx[nport].hdmi_2p1_en) 
    {
        hdmi_out(HD21_High_Bit_Rate_Audio_Packet_reg, HDMI21_P0_HD21_High_Bit_Rate_Audio_Packet_hbr_audio_mode_mask);
        return ;
    }
    
    hdmi_out(HDMI_High_Bit_Rate_Audio_Packet_reg, HDMI_High_Bit_Rate_Audio_Packet_hbr_audio_mode_mask);
}

unsigned char lib_hdmi_audio_get_hbr_manual_mode(unsigned char nport)
{
    // here we use HDMI_TMDS_PWDCTL_reg bit 10/11 for HBR control
    return HDMI_FW_FUNC_get_hbr_audio_mode_fw(hdmi_in(HDMI_TMDS_PWDCTL_reg));
}

unsigned char lib_hdmi_audio_get_hbr_mode(unsigned char nport)
{
    if (hdmi_rx[nport].hdmi_2p1_en)
        return HDMI_High_Bit_Rate_Audio_Packet_get_hbr_audio_mode(hdmi_in(HD21_High_Bit_Rate_Audio_Packet_reg));
    
    return HDMI_High_Bit_Rate_Audio_Packet_get_hbr_audio_mode(hdmi_in(HDMI_High_Bit_Rate_Audio_Packet_reg));
}


void lib_hdmi_audio_dbg_reg_dump(unsigned char nport)
{
    if (hdmi_rx[nport].hdmi_2p1_en) 
    {
        HDMI_EMG("###################Audio Register Start ###################\n");
        HDMI_EMG("ASR0=%x ASR1=%x \n",hdmi_in(HD21_HDMI_ASR0_reg),hdmi_in(HD21_HDMI_ASR1_reg));
        HDMI_EMG("ACRCR=%x ACRSR0=%x ACRSR1=%x \n",hdmi_in(HD21_HDMI_ACRCR_reg),hdmi_in(HD21_HDMI_ACRSR0_reg),hdmi_in(HDMI_HDMI_ACRSR1_reg));
        HDMI_EMG("CTS_UP_BOUND=%x  CTS_LOW_BOUND=%x \n",hdmi_in(HD21_AUDIO_CTS_UP_BOUND_reg),hdmi_in(HD21_AUDIO_CTS_LOW_BOUND_reg));
        HDMI_EMG("N_UP_BOUND=%x  N_LOW_BOUND=%x \n",hdmi_in(HD21_AUDIO_N_UP_BOUND_reg),hdmi_in(HD21_AUDIO_N_LOW_BOUND_reg));
        HDMI_EMG("INTCR=%x SR_CHG=%x \n",hdmi_in(HD21_HDMI_INTCR_reg),hdmi_in(HD21_Audio_Sample_Rate_Change_IRQ_reg));
        HDMI_EMG("###################Audio Register End###################\n");
        return ;
    }
    
    HDMI_EMG("###################Audio Register Start ###################\n");
    
    HDMI_EMG("ASR0=%x ASR1=%x \n",hdmi_in(HDMI_HDMI_ASR0_reg),hdmi_in(HDMI_HDMI_ASR1_reg));
    HDMI_EMG("ACRCR=%x ACRSR0=%x ACRSR1=%x \n",hdmi_in(HDMI_HDMI_ACRCR_reg),hdmi_in(HDMI_HDMI_ACRSR0_reg),hdmi_in(HDMI_HDMI_ACRSR1_reg));
    HDMI_EMG("CTS_UP_BOUND=%x  CTS_LOW_BOUND=%x \n",hdmi_in(HDMI_AUDIO_CTS_UP_BOUND_reg),hdmi_in(HDMI_AUDIO_CTS_LOW_BOUND_reg));
    HDMI_EMG("N_UP_BOUND=%x  N_LOW_BOUND=%x \n",hdmi_in(HDMI_AUDIO_N_UP_BOUND_reg),hdmi_in(HDMI_AUDIO_N_LOW_BOUND_reg));
    HDMI_EMG("INTCR=%x SR_CHG=%x \n",hdmi_in(HDMI_HDMI_INTCR_reg),hdmi_in(HDMI_Audio_Sample_Rate_Change_IRQ_reg));

    HDMI_EMG("###################Audio Register End###################\n");
}


int newbase_hdmi_audio_get_audio_status(unsigned char port, HDMI_AUDIO_ST* p_status)
{
    if (p_status==NULL || port >= HDMI_PORT_TOTAL_NUM)
        return -1;
    
    if (port != current_port)    
        memset(p_status, 0, sizeof(HDMI_AUDIO_ST));
    else
        *p_status = hdmi_rx[port].audio_t;
    
    return 0;
}


HDMI_AUDIO_FORMAT_T lib_hdmi_audio_get_coding_type(unsigned char nport)
{
    if (nport!=current_port)
        return AUDIO_FORMAT_NO_AUDIO;

    if (MODE_DVI == GET_H_MODE(nport))
        return AUDIO_FORMAT_DVI;

    if (AUDIO_hdmi_to_audio_cs_1_get_data_type_l(hdmi_in(AUDIO_hdmi_to_audio_cs_1_reg))==0)  // this is linear pcm data
        return AUDIO_FORMAT_PCM;
    
    // non pcm data
    switch((hdmi_in(AUDIO_hdmi_in_PaPb_val_reg) & 0x1F)) 
    {
    case 0:
        return AUDIO_FORMAT_PCM;
        
    case 1:
        return AUDIO_FORMAT_AC3;
        
    case 4:   
        return AUDIO_FORMAT_MPEG;
        
    case 5:   
        return AUDIO_FORMAT_MPEG;
        
    case 7:   
        return AUDIO_FORMAT_AAC;
        
    case 11:
    case 12:
    case 13:  
        return AUDIO_FORMAT_DTS;

    case 0x15: 
        return AUDIO_FORMAT_EAC3;
        
    case 0x11:
        return AUDIO_FORMAT_DTS_HD_MA;
        
    case 0x16: 
        return AUDIO_FORMAT_TRUEHD;
        
    default :
        return AUDIO_FORMAT_PCM;
    }
}


unsigned char lib_hdmi_audio_get_channel_num(unsigned char nport)
{
    if (nport!=current_port)
        return 0;
        
    return AUDIO_hdmi_to_audio_debug_get_hdmi_to_aud_ch_num(hdmi_in(AUDIO_hdmi_to_audio_debug_reg))+1;
}

unsigned char lib_hdmi_audio_get_channel_status_bit(unsigned char nport)
{
    unsigned int audio_cs_1 = 0;
    unsigned char channel_status_bit = 0;
    if (nport!=current_port)
        return 0;

    audio_cs_1 = hdmi_in(AUDIO_hdmi_to_audio_cs_1_reg);
    switch(AUDIO_hdmi_to_audio_cs_1_get_sampling_freq(audio_cs_1))
    {
        case 0: // 44.1k
        case 2: //48k
        case 3: // 32k
        case 8: // 88.2k
        case 9: // 768k
        case 10: // 96k
        case 12: // 176.4k
        case 14: // 192k
            channel_status_bit = AUDIO_hdmi_to_audio_cs_1_get_sampling_freq(audio_cs_1);
            break;
        default:
            channel_status_bit = ((AUDIO_hdmi_to_audio_cs_1_get_cs_lb3_31_30(audio_cs_1)<<4) | AUDIO_hdmi_to_audio_cs_1_get_sampling_freq(audio_cs_1));
            break;
    }
    return channel_status_bit;
}

unsigned char lib_hdmi_audio_get_audio_packet_layout_channel_num(unsigned char nport)
{
    if (nport!=current_port)
        return 0;

    return AUDIO_hdmi_to_audio_cs_2_get_layout_channel_num(hdmi_in(AUDIO_hdmi_to_audio_cs_2_reg));
}

unsigned char lib_hdmi_audio_get_audio_packet_layout_value(unsigned char nport)
{
    unsigned char layout_value = 0;
    if (nport!=current_port)
        return 0;

    switch(AUDIO_hdmi_to_audio_cs_2_get_layout_channel_num(hdmi_in(AUDIO_hdmi_to_audio_cs_2_reg)))
    {
        case 0x0:    // 0000b, PCM 2 channel or  compressed audio layout A
            layout_value = 0;
            break;
        case 0xE:     // 1110b, PCM 8 channel or  compressed audio layout B
            layout_value = 1;
            break;
        case 0xD:     // 1101b, PCM 16 channel
            layout_value = 2;
            break;
        case 0xC:     // 1100b, PCM 32 channel
            layout_value = 3;
            break;
        default:
            layout_value = 0;
            break;
    }

    return layout_value;
}

