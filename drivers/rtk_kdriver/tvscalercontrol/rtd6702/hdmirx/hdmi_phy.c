/*=============================================================
 * Copyright (c)      Realtek Semiconductor Corporation, 2016
 *
 * All rights reserved.
 *
 *============================================================*/

/*======================= Description ============================
 *
 * file: 		hdmiPhy.c
 *
 * author: 	Iron Man, Tony Stark
 * date:
 * version: 	3.0
 *
 *============================================================*/

/*========================Header Files============================*/
#include <linux/kthread.h> //kthread_create()¡Bkthread_run()
#include <linux/err.h> //IS_ERR()¡BPTR_ERR()
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <mach/platform.h>
#include "hdmi_common.h"
#include "hdmi_dsc.h"
#include "hdmi_phy.h"
#include "hdmi_phy_dfe.h"
#include "hdmi_hdcp.h"
#include "hdmi_scdc.h"
#include "hdmi_reg.h"
#include "hdmi_vfe_config.h"
#include "hdmi_debug.h"

extern HDMI_PORT_INFO_T hdmi_rx[HDMI_PORT_TOTAL_NUM];
extern wait_queue_head_t hdmi_thd_wait_qu;
/**********************************************************************************************
*
*	Marco or Definitions
*
**********************************************************************************************/

#ifndef IRQ_HANDLED
#define IRQ_HANDLED  1
#define IRQ_NONE     0
#endif

#define CLOCK_TOLERANCE(port)		(phy_st[port].clk_tolerance)
//#define CLK_DEBOUNCE_CNT_THD		7       // Set 2, it will easy to set phy in background and influence others.
#define CLK_DEBOUNCE_SHORT_CNT_THD		1       // to prevent Tx HDCP handshake too fast
//#define NOCLK_DEBOUNCE_CNT		8       // count for confirm it's no clock, (QD980>5, has sudden no clock and clear scdc)
#define NOCLK_DEBOUNCE_CNT		13	// count for confirm it's no clock, (Panasonic DMR-UBZ1 >13, has sudden no clock and clear scdc)
#define CLK_UNSTABLE_CNT		2       // count for clock unstable change

#define TMDS_CHAR_ERR_PERIOD		4
#define TMDS_CHAR_ERR_THREAD		8
#define TMDS_CHAR_ERR_MAX_LOOP 15  //(4*1/23hz * 1000) / 15ms (one checkmode time) = 11.5 --> 15

#define TMDS_BIT_ERR_PERIOD		16
#define TMDS_BIT_ERR_THREAD		32
#define TMDS_BIT_ERR_MAX_LOOP          50      //(16 * 1/23hz *1000) / 15ms (one check mode time) =  46.3 --> 50

#define MAX_ERR_DETECT_TIMES	10
#define TMDS_BIT_ERR_SIGNAL_BAD_THREAD	1000

#define PHY_Z0        			0x11
#define HDMI_EQ_LE_NOT_ZERO_ENABLE
#define CLK_OVER_300M			1

#define HPD_R1K_SEL	0x1 //HPD_R1K_SEL = 0x0~0x7 (0.85Kohm~1.0Kohm)

#define HPD_EARC_R1K_SEL 0x6//HPD_R1K_SEL = 1.0Kohm for EACR


#ifndef MAX
    #define MAX(a,b)                (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
    #define MIN(a,b)                (((a) < (b)) ? (a) : (b))
#endif




/**********************************************************************************************
*
*	Const Declarations
*
**********************************************************************************************/
#ifndef UINT8
typedef unsigned char __UINT8;
#define UINT8 __UINT8
#endif
//typedef char INT8;
#ifndef UINT32
typedef unsigned int __UINT32;
#define UINT32 __UINT32
#endif
#ifndef INT32
typedef int __INT32;
#define INT32 __INT32
#endif
#ifndef UINT16
typedef unsigned short __UINT16;
#define UINT16 __UINT16
#endif
#ifndef INT16
typedef short __INT16;
#define INT16 __INT16
#endif


typedef struct {
	// factory mode
	unsigned char eq_mode;  // 0: auto adaptive  1: manual mode
	unsigned char manual_eq[4];
}HDMIRX_PHY_FACTORY_T;


#if BIST_PHY_SCAN
typedef enum {
	PHY_INIT_SCAN,
	PHY_ICP_SCAN,
	PHY_KP_SCAN,
	PHY_END_SCAN,
}HDMIRX_PHY_SCAN_T;
#endif


typedef enum {
	EQ_MANUAL_OFF,
	EQ_MANUAL_ON,
}HDMIRX_PHY_EQ_MODE;


/**********************************************************************************************
*
*	Variables
*
**********************************************************************************************/
unsigned int clock_bound_3g;
unsigned int clock_bound_1p5g;
unsigned int clock_bound_110m;
unsigned int clock_bound_45m;

HDMIRX_PHY_STRUCT_T phy_st[HDMI_PORT_TOTAL_NUM];

unsigned int phy_isr_en[HDMI_PORT_TOTAL_NUM];

HDMIRX_PHY_FACTORY_T phy_fac_eq_st[HDMI_PORT_TOTAL_NUM];


#if BIST_PHY_SCAN
HDMIRX_PHY_SCAN_T  phy_scan_state = PHY_INIT_SCAN;
#endif

unsigned char factory_or_power_only_mode_en=0;
unsigned char OBJ_TV_only=0;

/**********************************************************************************************
*
*	Funtion Declarations
*
**********************************************************************************************/




unsigned char newbase_rxphy_measure(unsigned char port);
unsigned char newbase_rxphy_is_clock_stable(unsigned char port);



//PHY TABLE


//foreground K offset
void lib_hdmi_single_end(unsigned char nport, unsigned char channel, unsigned char p_off, unsigned char n_off);



//char & bit error detect

unsigned char lib_hdmi_char_err_get_error(unsigned char nport, unsigned int *r_err, unsigned int *g_err, unsigned int *b_err);
unsigned char lib_hdmi_bit_err_det_done(unsigned char nport);


//clock detect
void lib_hdmi_trigger_measurement(unsigned char nport);
void lib_hdmi_trigger_measure_start(unsigned char nport);
 void lib_hdmi_trigger_measure_stop(unsigned char nport);
unsigned char lib_hdmi_is_clk_ready(unsigned char nport);


unsigned char lib_hdmi_clock40x_sync_status(unsigned char nport);

#if 0
/**********************************************************************************************
*
*	Patch for [P0][K5Lp][OQA][PRODISSUE-141] HDMI3 Black Video to get power only mode or factory mode flag 
*
**********************************************************************************************/

static int __init setup_power_only(char *str)
{
	factory_or_power_only_mode_en = 1;
	return 0;
}

static int __init setup_factory_only(char *str)
{
	factory_or_power_only_mode_en = 1;
	return 0;
}


__setup("pwrOnly", setup_power_only);
__setup("factory", setup_factory_only);

static int __init setup_OBJ_TV_only(char *str)
{
	OBJ_TV_only = 1;
	return 0;
}
__setup("TOOL_OBJ", setup_OBJ_TV_only);
#endif


/**********************************************************************************************
*
*	Function Body
*
**********************************************************************************************/



void newbase_rxphy_isr_set(unsigned char port, unsigned char en)
{
	phy_isr_en[port] = en;
}

void newbase_rxphy_init_struct(unsigned char port)
{
	memset(&phy_st[port], 0, sizeof(HDMIRX_PHY_STRUCT_T));
}

void newbase_hdmi_rxphy_handler(unsigned char port)
{

	unsigned char frl_mode;

	if (lib_hdmi_get_fw_debug_bit(DEBUG_27_BYPASS_PHY_ISR)) {
		return ;
	}

	frl_mode = newbase_hdmi_scdc_get_frl_mode(port);
	newbase_rxphy_set_frl_mode(port,frl_mode);

	if (frl_mode== MODE_TMDS)//hdmi_rx[port].hdmi_2p1_en==0)
	{
		//--------------------------------------------
		// for HDMI 2.0
		// check TMDS lock status
		//--------------------------------------------
		if ( newbase_rxphy_tmds_measure(port)) {
			newbase_hdmi_hdcp14_set_aksv_intr(port, 0);

				newbase_rxphy_tmds_job(port);  //hd21 phy

			newbase_hdmi_hdcp14_set_aksv_intr(port, 1);
		}
	}
	else
	{
		if (newbase_rxphy_frl_measure(port)) {
			if(GET_FRL_LT_FSM(port)>=LT_FSM_LTSP_PASS)
			{
				if (newbase_rxphy_get_setphy_done(port)) {
					newbase_rxphy_frl_job(port);
				}
			}
		}

		//--------------------------------------------
		// for HDMI 2.1....
		// TODO : check the FRL is working under LTS_P state
		//        monitor clock lane and check is the clock always stable
		//        for a while
		//--------------------------------------------
	}
}



int newbase_rxphy_isr(void)
{
	unsigned char i;

	for (i=0; i<HDMI_PORT_TOTAL_NUM; i++) 
	{
		if (!phy_isr_en[i])
			continue;

		if((GET_FLOW_CFG(HDMI_FLOW_CFG_HPD, HDMI_FLOW_CFG1_DISABLE_PORT_FAST_SWITCH) == HDMI_PCB_FORCE_ENABLE) && (i != newbase_hdmi_get_current_display_port()))
			continue;

		newbase_hdmi_rxphy_handler(i);

		newbase_hdcp_handler(i);

		newbase_hdmi_video_monitor(i);

		newbase_hdmi_error_handler(i);

		newbase_hdmi_check_connection_state(i);

		newbase_hdmi_check_sram_edid_state(i);

		newbase_hdmi_scdc_handler(i);  // handle SCDC event

		newbase_hdmi_hd21_dsc_handler(i);

		newbase_hdmi_power_saving_handler(i);  // handle Power Saving
	}

	return IRQ_HANDLED;

}

unsigned char newbase_rxphy_set_phy_for_aksv(unsigned char port)
{
	unsigned char dest_state = PHY_PROC_CLOSE_ADP; //PHY_PROC_DONE;
	unsigned char monitor_cnt = 8;

	if (phy_st[port].phy_proc_state >= dest_state) {
		HDCP_FLOW_PRINTF("[p%d] phy already init (%d)\n", port, phy_st[port].phy_proc_state);
		return TRUE;
	}

	if (newbase_rxphy_is_clk_stable(port))
	{
		HDCP_FLOW_PRINTF("[p%d] clk %d\n", port, phy_st[port].clk);
	}
	else if (newbase_rxphy_get_clk_pre(port) > VALID_MIN_CLOCK)
	{
		phy_st[port].clk = newbase_rxphy_get_clk_pre(port);
		HDCP_FLOW_PRINTF("[p%d] use pre clk %d\n", port,  phy_st[port].clk);
	}
	else
	{
		HDCP_FLOW_PRINTF("[p%d] no clk\n", port);
		return FALSE;
	}

	//newbase_rxphy_set_frl_mode(port, newbase_hdmi_scdc_get_frl_mode(port));
	if (newbase_rxphy_get_frl_mode(port) != MODE_TMDS) {
		HDCP_FLOW_PRINTF("[p%d] frl mode = %s,  must be TMDS\n", port, _hdmi_hd21_frl_mode_str(newbase_rxphy_get_frl_mode(port)));
		return FALSE;
	}

	while (phy_st[port].phy_proc_state < dest_state && monitor_cnt) {
		HDCP_FLOW_PRINTF("[p%d] current phy state = %d, target state = %d\n", port, phy_st[port].phy_proc_state, dest_state);
		newbase_rxphy_tmds_job(port);  //hd21 phy

		if (--monitor_cnt == 0)
			HDMI_EMG("[p%d] set phy fail for HDCP14 aksv intr. (%d/%d)\n", port, phy_st[port].phy_proc_state, dest_state);
	}

	return TRUE;
}

unsigned char newbase_rxphy_tmds_measure(unsigned char port)
{
unsigned int clk_tmp = 0;
unsigned char clk_stable_prev = phy_st[port].clk_stable;

#if HDMI_FRL_TRANS_DET
unsigned int lane_frl = 0;
#endif

	if (lib_hdmi_is_clk_ready(port)) {
		clk_tmp = lib_hdmi_get_clock(port);
		newbase_hdmi2p0_check_tmds_config(port); //check scdc flag
		hdmi_rx[port].raw_tmds_phy_md_clk = clk_tmp;
	}

#if HDMI_FRL_TRANS_DET
	if (lib_hdmi_lane_rate_is_ready(port)) {
		lane_frl = lib_hdmi_get_lane_rate(port);
		newbase_hdmi21_set_phy_frl_mode(port,lane_frl);
	}
#endif
	
	lib_hdmi_trigger_measure_start(port);

	if (clk_tmp > VALID_MIN_CLOCK) {

		//if ((newbase_hdmi2p0_get_clock40x_flag(port) || newbase_hdmi2p0_get_scramble_flag(port)) &&
		if ((newbase_hdmi2p0_get_clock40x_flag(port)) &&
			(clk_tmp < 1422 && clk_tmp > 699)) {	//It's possible for 40x only when clock=(3G~6G)/4 //WOSQRTK-9795
			clk_tmp <<= 2; 				 //HDMI2.0, change clock to clock*4
			phy_st[port].rxphy_40x = 1;
		}

		if (HDMI_ABS(clk_tmp, phy_st[port].clk_pre) < CLOCK_TOLERANCE(port)) {
			// ------------------------------------------
			// Get valid clock and close to previous clock
			// ------------------------------------------
			if (newbase_rxphy_is_clock_stable(port)) {
				phy_st[port].clk_stable = 1;
				phy_st[port].clk = clk_tmp;
				phy_st[port].frl_mode = MODE_TMDS;
				phy_st[port].lane_num= HDMI_3LANE;
			}
			phy_st[port].clk_unstable_count = 0;
		}
		else if (phy_st[port].clk_unstable_count < CLK_UNSTABLE_CNT) {
			// ------------------------------------------
			// Get valid clock but different with previous clock, count for debouncing
			// ------------------------------------------
			phy_st[port].clk_unstable_count++;
//			if (lib_hdmi_get_fw_debug_bit(DEBUG_14_CLK_DBG_MSG))
//				HDMI_PRINTF("unstable clk=%d, pre=%d, cur=%d, cnt=%d\n", clk_tmp, phy_st[port].clk_pre, phy_st[port].clk, phy_st[port].clk_unstable_count);
		} else {
			// ------------------------------------------
			// Get valid clock but different with previous clock
			// ------------------------------------------
			phy_st[port].clk = 0;
			phy_st[port].clk_pre = clk_tmp;
			phy_st[port].clk_tolerance = MAX(clk_tmp>>7, 5);
			phy_st[port].clk_debounce_count = 0;
			phy_st[port].clk_unstable_count = 0;
			phy_st[port].clk_stable = 0;
			phy_st[port].phy_proc_state = PHY_PROC_INIT;
		}

		phy_st[port].noclk_debounce_count = 0;

	} else {

		if (phy_st[port].noclk_debounce_count < NOCLK_DEBOUNCE_CNT) {
			// ------------------------------------------
			// NO clock, and start debouncing
			// ------------------------------------------

			phy_st[port].noclk_debounce_count++;
//			if (lib_hdmi_get_fw_debug_bit(DEBUG_14_CLK_DBG_MSG))
//				HDMI_PRINTF("noclk debounce=%d",phy_st[port].noclk_debounce_count);

		} else {
			// ------------------------------------------
			// NO clock, really!!  Then reset clk_info
			// ------------------------------------------

			if (phy_st[port].clk_pre) {
				phy_st[port].clk_pre = 0;
				phy_st[port].clk= 0;
				phy_st[port].clk_debounce_count= 0;
				phy_st[port].clk_unstable_count = 0;
				phy_st[port].clk_stable = 0;
				phy_st[port].phy_proc_state = PHY_PROC_INIT;
			}

			newbase_hdmi2p0_reset_scdc_toggle(port);
		}
	}

	lib_hdmi_trigger_measure_stop(port);

#if HDMI_FRL_TRANS_DET	
	lib_hdmi_lane_rate_detect_stop(port);
#endif
	if (clk_stable_prev != phy_st[port].clk_stable)
	{
#if HDMI_PHY_CLOCK_DYNAMIC_CNT_ENABLE
		HDMI_HDCP_E current_hdcp_mode = newbase_hdcp_get_auth_mode(port);
		if(HDCP14  == current_hdcp_mode || HDCP22 == current_hdcp_mode)
			HDCP_WARN("HDMI[p%d] clock stable change. HDCP mode (%s), clk_debounce (%x)\n", port,  _hdmi_hdcp_mode_str(current_hdcp_mode), phy_st[port].clk_debounce_count);
#endif //#if HDMI_PHY_CLOCK_DYNAMIC_CNT_ENABLE
		HDMI_EMG("clk_stable[p%d] from %d to %d clk=%d \n", port, clk_stable_prev, phy_st[port].clk_stable,phy_st[port].clk);

		lib_hdmi_misc_variable_initial(port);
		if(phy_st[port].clk_stable ==1)
		{
			SET_HDMI_DETECT_EVENT(port, HDMI_DETECT_PHY_CLOCK_STABLE);
		}
	}

	if (phy_st[port].clk_stable) {
		return TRUE;
	} else {
		return FALSE;
	}

}



/*------------------------------------------------------------------
 * Func : newbase_rxphy_is_clock_stable
 *
 * Desc : When HDCP1.4/2.2 auth from tx quickly, rxphy should quick stable refer to CLK_DEBOUNCE_SHORT_CNT_THD.
 *           NO_HDCP or HDCP_OFF are need to wait rxphy stable depend on  CLK_DEBOUNCE_CNT_THD.
 *
 * Para : [IN] port  : physical port number
 *
 * Retn : TRUE: stable; FALSE: unstable, then clk_debounce_count++.
 *------------------------------------------------------------------*/ 
unsigned char newbase_rxphy_is_clock_stable(unsigned char port)
{
	unsigned int phy_stable_cnt = GET_FLOW_CFG(HDMI_FLOW_CFG_GENERAL, HDMI_FLOW_CFG0_PHY_STABLE_CNT_THD);

	if(newbase_hdmi_get_device_phy_stable_count(port)!=0)
	{// special device use specified stable count
		phy_stable_cnt = newbase_hdmi_get_device_phy_stable_count(port);
	}

	//HDMI_EMG("[newbase_rxphy_is_clock_stable] port=%d, phy_stable_cnt=%d, b_clk=%d\n", port, phy_stable_cnt, phy_st[port].clk_pre);
	if (phy_st[port].clk_debounce_count >= CLK_DEBOUNCE_SHORT_CNT_THD)
	{
		HDMI_HDCP_E hdcp_auth_mode = newbase_hdcp_get_auth_mode(port);
		if(NO_HDCP == hdcp_auth_mode || HDCP_OFF == hdcp_auth_mode)
		{
			if (phy_st[port].clk_debounce_count >=  phy_stable_cnt)
			{
				return TRUE;//Normal case: HDCP auth from TX is after rxphy_clock_stable
			}
			else
			{
				phy_st[port].clk_debounce_count++;
			}
		}
		else
		{
			return TRUE;//For special device case: HDCP auth from TX is near.
		}
	}
	else
	{
		phy_st[port].clk_debounce_count++;
	}

	return FALSE;
}



void newbase_rxphy_force_clear_clk_st(unsigned char port)
{
	// case 1: clk=1080p50->1080p60, timing changed, but clock is the same, then force to re-set phy
	// case 2: clk=4k2kp60->1080p60, timing changed, but clock is the same, then force to re-set phy (also need to clear SCDC)
	//
#if BIST_DFE_SCAN
	if (lib_hdmi_get_fw_debug_bit(DEBUG_30_DFE_SCAN_TEST)) return;
#endif
	newbase_rxphy_init_struct(port);
}

unsigned int newbase_rxphy_get_clk(unsigned char port)
{
	return phy_st[port].clk;
}
unsigned int newbase_rxphy_get_clk_pre(unsigned char port)
{
	return phy_st[port].clk_pre;
}

unsigned char newbase_rxphy_is_clk_stable(unsigned char port)
{
	//if frl mode no check clk, fix me
	if (hdmi_rx[port].hdmi_2p1_en) {
		return 1;
	}
	return phy_st[port].clk_stable;
}

void newbase_rxphy_set_apply_clock(unsigned char port,unsigned int b)
{
	phy_st[port].apply_clk = b;
}

unsigned int newbase_rxphy_get_apply_clock(unsigned char port)
{
	return phy_st[port].apply_clk;
}

unsigned char newbase_rxphy_is_tmds_mode(unsigned port)
{
	if (phy_st[port].frl_mode == MODE_TMDS)
		return 1;
	else 
		return 0;

}

unsigned char newbase_rxphy_get_setphy_done(unsigned char port)
{

	return (phy_st[port].phy_proc_state == PHY_PROC_DONE);
}

void newbase_rxphy_reset_setphy_proc(unsigned char port)
{
	phy_st[port].clk_pre = 0;
	phy_st[port].clk= 0;
	phy_st[port].clk_debounce_count= 0;
	phy_st[port].clk_unstable_count = 0;
	phy_st[port].clk_stable = 0;

	//for frl mode
	phy_st[port].frl_mode = 0;
	phy_st[port].pre_frl_mode = 0;
	phy_st[port].phy_proc_state = PHY_PROC_INIT;
	SET_HDMI_DETECT_EVENT(port, HDMI_DETECT_PHY_RESET);
	HDMI_EMG("[newbase_rxphy_reset_setphy_proc port:%d]\n", port);
}

void newbase_hdmi_dfe_recovery_6g_long_cable(unsigned char port)
{
	phy_st[port].phy_proc_state = PHY_PROC_RECOVER_6G_LONGCABLE;
	phy_st[port].recovery = 1;
}

void newbase_hdmi_dfe_recovery_mi_speed(unsigned char port)
{
	phy_st[port].phy_proc_state = PHY_PROC_RECOVER_MI;
	phy_st[port].recovery = 1;
}


void newbase_hdmi_dfe_recovery_hi_speed(unsigned char port)
{
	phy_st[port].phy_proc_state = PHY_PROC_RECOVER_HI;
	phy_st[port].recovery = 1;
}


void newbase_hdmi_dfe_recovery_midband_long_cable(unsigned char port)
{
	phy_st[port].phy_proc_state = PHY_PROC_RECOVER_MID_BAND_LONGCABLE;
	phy_st[port].recovery = 1;

}

void newbase_hdmi_set_factory_or_power_only_mode_enable(unsigned char enable)
{
	factory_or_power_only_mode_en = enable;
}

unsigned char newbase_hdmi_get_factory_or_power_only_mode_enable(void)
{
	return factory_or_power_only_mode_en;
}

void newbase_hdmi_set_eq_mode(unsigned char nport, unsigned char mode)
{
	phy_fac_eq_st[nport].eq_mode = mode;

	if (phy_fac_eq_st[nport].eq_mode == EQ_MANUAL_OFF) {
		phy_st[nport].clk_pre = 0;
		phy_st[nport].clk= 0;
		phy_st[nport].clk_debounce_count= 0;
		phy_st[nport].clk_unstable_count = 0;
		phy_st[nport].clk_stable = 0;
		phy_st[nport].phy_proc_state = PHY_PROC_INIT;
	}

}


void newbase_hdmi_manual_eq_ch(unsigned char nport,unsigned char ch,unsigned char eq_ch)
{
//	unsigned char bgrc;
//	unsigned char mode;

#if HDMI_DFE_BRSWAP
	if (nport == HDMI_PORT3)  {
		if (ch == 0) {
			phy_fac_eq_st[nport].manual_eq[2] = eq_ch;
		}
		else if (ch == 1) {
			phy_fac_eq_st[nport].manual_eq[1] = eq_ch;
		}
		else if (ch == 2) {
			phy_fac_eq_st[nport].manual_eq[0] = eq_ch;

		} else {
			return;
		}

	} else {

	if (ch == 0) {
		phy_fac_eq_st[nport].manual_eq[0] = eq_ch;
	}
	else if (ch == 1) {
		phy_fac_eq_st[nport].manual_eq[1] = eq_ch;
	}
	else if (ch == 2) {
		phy_fac_eq_st[nport].manual_eq[2] = eq_ch;

	} else {
		return;
	}
	}

#else
	if (ch == 0) {
		phy_fac_eq_st[nport].manual_eq[0] = eq_ch;
	}
	else if (ch == 1) {
		phy_fac_eq_st[nport].manual_eq[1] = eq_ch;
	}
	else if (ch == 2) {
		phy_fac_eq_st[nport].manual_eq[2] = eq_ch;

	} else {
		return;
	}
#endif

	phy_st[nport].clk_pre = 0;
	phy_st[nport].clk= 0;
	phy_st[nport].clk_debounce_count= 0;
	phy_st[nport].clk_unstable_count = 0;
	phy_st[nport].clk_stable = 0;
	phy_st[nport].phy_proc_state = PHY_PROC_INIT;
	

}



void newbase_hdmi_manual_eq(unsigned char nport, unsigned char eq_ch0, unsigned char eq_ch1, unsigned char eq_ch2)
{
	if (phy_fac_eq_st[nport].eq_mode == EQ_MANUAL_OFF)
		return;

	phy_fac_eq_st[nport].manual_eq[0] = eq_ch0;
	phy_fac_eq_st[nport].manual_eq[1] = eq_ch1;
	phy_fac_eq_st[nport].manual_eq[2] = eq_ch2;
	
	lib_hdmi_dfe_update_le(nport, eq_ch0, eq_ch1, eq_ch2);
}


void newbase_hdmi_reset_thr_cnt(unsigned char port)
{
	//le  abnormal counter
	phy_st[port].dfe_thr_chk_cnt = 0;

}

void newbase_hdmi_open_err_detect(unsigned char port)
{
	//re-adaptive counter
	phy_st[port].error_detect_count = 0;
	
}


void newbase_hdmi_err_detect_add(unsigned char port)
{
		if (phy_st[port].error_detect_count == 0)
			HDMI_EMG("eq adjust count start\n");
		
		if (phy_st[port].error_detect_count < MAX_ERR_DETECT_TIMES) {
			++phy_st[port].error_detect_count;
			HDMI_EMG("cnt = %d\n",phy_st[port].error_detect_count);
		}

		if (phy_st[port].error_detect_count >= MAX_ERR_DETECT_TIMES)
			HDMI_EMG("eq adjust count end\n");
}

unsigned char newbase_hdmi_err_detect_stop(unsigned char port)
{
	if (phy_st[port].error_detect_count >= MAX_ERR_DETECT_TIMES)
		return 1;
	else
		return 0;
}

unsigned char newbase_hdmi_get_err_recovery(unsigned char port)
{
	return phy_st[port].recovery;
}


unsigned char newbase_hdmi_char_err_detection(unsigned char port,unsigned int *r_err, unsigned int *g_err, unsigned int *b_err)
{

	if (phy_st[port].char_err_loop == 0) {
		lib_hdmi_char_err_start(port, 0, TMDS_CHAR_ERR_PERIOD);
		phy_st[port].char_err_loop++;

	} else {
		if (lib_hdmi_char_err_get_error(port, r_err, g_err, b_err)) {
			HDMI_PORT_INFO_T* p_hdmi_rx_st = newbase_hdmi_get_rx_port_info(port);

			phy_st[port].char_err_loop = 0;
			newbase_hdmi_ced_error_cnt_accumulated(port, b_err, g_err, r_err);

			if(p_hdmi_rx_st != NULL)
			{
				p_hdmi_rx_st->fw_char_error[0] = *r_err;
				p_hdmi_rx_st->fw_char_error[1] = *g_err;
				p_hdmi_rx_st->fw_char_error[2] = *b_err;
			}
			else
			{
				HDMI_EMG("[newbase_hdmi_char_err_detection] NULL hdmi_rx, port = %d\n", port);
			}
			if ((*r_err > TMDS_CHAR_ERR_THREAD) || (*g_err > TMDS_CHAR_ERR_THREAD) || (*b_err > TMDS_CHAR_ERR_THREAD)) {
				// Do something to save the world
				HDMI_PRINTF("[CHAR_ERR](%d, %d, %d)\n", *b_err, *g_err, *r_err);
				return 1;
			}
			return 0;
		}

		if (phy_st[port].char_err_loop > TMDS_CHAR_ERR_MAX_LOOP) {
			phy_st[port].char_err_loop = 0;
			newbase_hdmi_set_6G_long_cable_enable(port , 0);//disable 6g long cable detect
			HDMI_EMG("[CHAR_ERR] Life is too short.\n");
		} else {
			phy_st[port].char_err_loop++;
		}

	}
	return 0;

}


unsigned char newbase_hdmi_get_longcable_flag(unsigned char port)
{
	return phy_st[port].longcable_flag;
}

void newbase_hdmi_set_longcable_flag(unsigned char port, unsigned char enable)
{
	 phy_st[port].longcable_flag = enable;
}



unsigned char newbase_hdmi_bit_err_detection(unsigned char port, unsigned int *r_err, unsigned int *g_err, unsigned int *b_err)
{
        unsigned char bit_err_det = 0;

	if (phy_st[port].bit_err_loop == 0)
	{
		lib_hdmi_bit_err_start(port, 0, TMDS_BIT_ERR_PERIOD);
		phy_st[port].bit_err_loop++;
	}
	else
	{
		if (lib_hdmi_bit_err_get_error(port, r_err, g_err, b_err))
		{
			HDMI_PORT_INFO_T* p_hdmi_rx_st = newbase_hdmi_get_rx_port_info(port);

			phy_st[port].bit_err_loop = 0;
			phy_st[port].bit_err_occurred = 0;
			if(p_hdmi_rx_st != NULL)
			{
				p_hdmi_rx_st->fw_bit_error[0] = *r_err;
				p_hdmi_rx_st->fw_bit_error[1] = *g_err;
				p_hdmi_rx_st->fw_bit_error[2] = *b_err;
			}
			else
			{
				HDMI_EMG("[newbase_hdmi_bit_err_detection] NULL hdmi_rx, port = %d\n", port);
			}

			if ((*r_err > TMDS_BIT_ERR_THREAD) || (*g_err > TMDS_BIT_ERR_THREAD) || (*b_err > TMDS_BIT_ERR_THREAD)) {
				// Do something to save the world
				HDMI_PRINTF("[BIT_ERR](%d, %d, %d)\n", *r_err, *g_err, *b_err);
				bit_err_det = 1;
			}


                         //USER:Lewislee DATE:2017/03/20
			//for KTASKWBS-6722, TATASKY 4K STB + HDMI Cable : Maxicom (Non standard)
			//The g_err bit error very large issue
			if((*g_err > TMDS_BIT_ERR_SIGNAL_BAD_THREAD) && (*r_err < 10) && (*b_err < 10))
			{
					phy_st[port].bit_err_occurred = 1;
					// Do something to save the world
					HDMI_PRINTF("[BIT_ERR] Signal BAD Re-Auto EQ\n");
					bit_err_det = 1;
			}

				if (bit_err_det)
					return 1;
				else
					return 0;

		}

		if (phy_st[port].bit_err_loop > TMDS_BIT_ERR_MAX_LOOP) {
			phy_st[port].bit_err_loop = 0;
			phy_st[port].bit_err_occurred = 0;
			HDMI_PRINTF("[BIT_ERR] Life is too short.\n");
		} else {
			phy_st[port].bit_err_loop++;
		}

	}

	return 0;

}

unsigned char newbase_hdmi_set_6G_long_cable_enable(unsigned char port, unsigned char enable)
{
	return phy_st[port].detect_6g_longcable_enable = enable;
}

/*---------------------------------------------------
 * Func : newbase_hdmi_ced_error_cnt_accumulated
 *
 * Desc : scdc read will clear zero between offset 0x50 ~ 0x55
 *
 * Para : nport : HDMI port number
 *        err    : ced error of detet one times mode
 *
 * Retn : N/A
 *--------------------------------------------------*/
void newbase_hdmi_ced_error_cnt_accumulated(unsigned char port, unsigned int *b_err, unsigned int *g_err, unsigned int *r_err)
{
	if(0x7FFF - phy_st[port].char_error_cnt[0] < *b_err)
		phy_st[port].char_error_cnt[0] = 0x7FFF;
	else
		phy_st[port].char_error_cnt[0] += *b_err ;

	if(0x7FFF - phy_st[port].char_error_cnt[1] < *g_err)
		phy_st[port].char_error_cnt[1] = 0x7FFF;
	else
		phy_st[port].char_error_cnt[1] += *g_err ;

	if(0x7FFF - phy_st[port].char_error_cnt[2] < *r_err)
		phy_st[port].char_error_cnt[2] = 0x7FFF;
	else
		phy_st[port].char_error_cnt[2] += *r_err ;

}
void newbase_hdmi_ced_error_cnt_reset(unsigned char port)
{
	phy_st[port].char_error_cnt[0] = 0;
	phy_st[port].char_error_cnt[1] = 0;
	phy_st[port].char_error_cnt[2] = 0;
	phy_st[port].char_error_cnt[3] = 0;
}
unsigned short newbase_hdmi_get_ced_error_cnt(unsigned char port , unsigned char ch)
{
	return phy_st[port].char_error_cnt[ch];
}


unsigned char newbase_hdmi_is_bit_error_occured(unsigned char port)
{
	return phy_st[port].bit_err_occurred;
}


unsigned char lib_hdmi_clock40x_sync_status(unsigned char nport)
{
#if 0
        /* skip sync check. will run in interrupt context */
        DFE_WARN("HDMI[p%d] var_40x=%d toggle_count=%d\n", nport, newbase_hdmi2p0_get_clock40x_flag(nport) ,newbase_hdmi2p0_get_tmds_toggle_flag(nport));
        return 1;
#else
        unsigned char reg_40x = 0, var_40x = 0;

        reg_40x =  (lib_hdmi_scdc_get_tmds_config(nport) & _BIT1) >> 1;
	 var_40x =  newbase_hdmi2p0_get_clock40x_flag(nport);

	DFE_WARN("HDMI[p%d] reg_40x =%d  var_40x=%d toggle_count=%d\n", nport, reg_40x,var_40x,newbase_hdmi2p0_get_tmds_toggle_flag(nport));	
        if (reg_40x == var_40x)
                return 1;
        else {
                return 0;
        }
#endif
}


void lib_hdmi_trigger_measurement(unsigned char nport)
{
#if 1
	hdmi_mask(HDMIRX_PHY_mod_regd00,~HDMIRX_PHY_HD21_P0_MOD_REGD00_reg_p0_ck_md_rstb_mask,0);
	hdmi_mask(HDMIRX_PHY_mod_regd00,~HDMIRX_PHY_HD21_P0_MOD_REGD00_reg_p0_ck_md_rstb_mask,HDMIRX_PHY_HD21_P0_MOD_REGD00_reg_p0_ck_md_rstb_mask);
	hdmi_out(HDMIRX_PHY_mod_regd01, HDMIRX_PHY_HD21_P0_MOD_REGD01_p0_ck_md_ok_ro_mask);
#endif
}

void lib_hdmi_trigger_measure_start(unsigned char nport)
{
#if 1
	hdmi_mask(HDMIRX_PHY_mod_regd00,~HDMIRX_PHY_HD21_P0_MOD_REGD00_reg_p0_ck_md_rstb_mask,0);
#endif
}

void lib_hdmi_trigger_measure_stop(unsigned char nport)
{
#if 1 
	hdmi_mask(HDMIRX_PHY_mod_regd00,~HDMIRX_PHY_HD21_P0_MOD_REGD00_reg_p0_ck_md_rstb_mask,HDMIRX_PHY_HD21_P0_MOD_REGD00_reg_p0_ck_md_rstb_mask);
	
#endif
}

#ifdef FAKE_FW_CLOCK_DETECT
unsigned int fw_b_clock = 0;
#endif

unsigned char lib_hdmi_is_clk_ready(unsigned char nport)
{

#ifdef FAKE_FW_CLOCK_DETECT
  //  if (fw_b_clock)    // for bring up verification
   //     return TRUE;
#endif

#if 1
	hdmi_out(HDMIRX_PHY_mod_regd01, HDMIRX_PHY_HD21_P0_MOD_REGD01_p0_ck_md_ok_ro_mask);
	udelay(40);
	if (HDMIRX_PHY_HD21_P0_MOD_REGD01_get_p0_ck_md_ok_ro(hdmi_in(HDMIRX_PHY_mod_regd01))) {
		return TRUE;
	} else {
		return FALSE;
	}	
#endif
	return FALSE;
}

unsigned int lib_hdmi_get_clock(unsigned char nport)
{

#ifdef FAKE_FW_CLOCK_DETECT
   if (fw_b_clock)    // for bring up verification
        return fw_b_clock;
#endif

	return HDMIRX_PHY_HD21_P0_MOD_REGD01_get_p0_ck_md_count_ro(hdmi_in(HDMIRX_PHY_mod_regd01));

}

void lib_hdmi_z0_set(unsigned char nport, unsigned char lane, unsigned char enable)
{
	lib_hdmi21_z0_set(nport,lane,enable);		
}

void lib_hdmi_eq_set_clock_boundary(unsigned char port)
{

        
            clock_bound_3g = TMDS_3G;
            clock_bound_1p5g = TMDS_1p5G;
            clock_bound_45m = 430;
            clock_bound_110m = 1042;
        

        //HDMI_PRINTF("clock_bound_3g=%d\n", clock_bound_3g);

}

#ifdef CONFIG_POWER_SAVING_MODE

void lib_hdmi_eq_pi_power_en(unsigned char bport,unsigned char en)
{
#if 0
		UINT32 B2_addr,G2_addr,R2_addr;
        
        if (bport == HDMI_PORT2) {
                B2_addr = HDMIRX_2P0_PHY_P0_B2_reg;
                G2_addr = HDMIRX_2P0_PHY_P0_G2_reg;
                R2_addr = HDMIRX_2P0_PHY_P0_R2_reg;
        } else if (bport == HDMI_PORT3) {
                B2_addr = HDMIRX_2P0_PHY_P1_B2_reg;
                G2_addr = HDMIRX_2P0_PHY_P1_G2_reg;
                R2_addr = HDMIRX_2P0_PHY_P1_R2_reg;	  			
        } else {
        	return;
        }

	if (en) {

		hdmi_mask(B2_addr, ~(P0_b_5_EQ_POW), P0_b_5_EQ_POW);
		hdmi_mask(G2_addr, ~(P0_b_5_EQ_POW) , P0_b_5_EQ_POW);
		hdmi_mask(R2_addr, ~(P0_b_5_EQ_POW), P0_b_5_EQ_POW);
		udelay(1);
		hdmi_mask(B2_addr, ~(P0_b_8_POW_PR), P0_b_8_POW_PR);
		hdmi_mask(G2_addr, ~(P0_b_8_POW_PR), P0_b_8_POW_PR);
		hdmi_mask(R2_addr, ~(P0_b_8_POW_PR), P0_b_8_POW_PR);
		
	} else {
		// 4 lane all close
		hdmi_mask(B2_addr, ~(P0_b_8_POW_PR|P0_b_5_EQ_POW), 0x0);
		hdmi_mask(G2_addr, ~(P0_b_8_POW_PR|P0_b_5_EQ_POW), 0x0);
		hdmi_mask(R2_addr, ~(P0_b_8_POW_PR|P0_b_5_EQ_POW), 0x0);
	}
#endif
}
#endif


#if BIST_DFE_SCAN
//--------------------------------------------------------------------------
//
//	Debug Functions, not release.
//
//--------------------------------------------------------------------------
void debug_hdmi_dfe_scan(unsigned char nport,unsigned char lane_mode)
{

}
#endif

#if BIST_PHY_SCAN

int newbase_get_phy_scan_done(unsigned char nport)
{
	if (phy_scan_state == PHY_END_SCAN)
		return 1;
	else {
		phy_st[nport].clk_pre = 0;
		phy_st[nport].clk= 0;
		phy_st[nport].clk_debounce_count= 0;
		phy_st[nport].clk_unstable_count = 0;
		phy_st[nport].clk_stable = 0;
		phy_st[nport].phy_proc_state = PHY_PROC_INIT;
		return 0;
	}
}

void debug_hdmi_dump_msg(unsigned char nport)
{
	unsigned int bit_r_err, bit_g_err, bit_b_err, ch_r_err, ch_g_err, ch_b_err;
	//unsigned int i = 0;
	
	lib_hdmi_set_fw_debug_bit(DEBUG_26_BYPASS_DETECT, 1);
	lib_hdmi_set_fw_debug_bit(DEBUG_25_BYPASS_CHECK, 1);

	lib_hdmi_bch_1bit_error_clr(nport);
	lib_hdmi_bch_2bit_error_clr(nport);
	lib_hdmi_clear_rgb_hv_status(nport);
	lib_hdmi_bit_err_start(nport, 1, 10);
	lib_hdmi_char_err_start(nport, 1, 10);
	msleep(50);
	lib_hdmi_bit_err_get_error(nport, &bit_r_err, &bit_g_err, &bit_b_err);
	lib_hdmi_char_err_get_error(nport, &ch_r_err, &ch_g_err, &ch_b_err);
	HDMI_EMG("bch1=%d bch2=%d\n",lib_hdmi_get_bch_1bit_error(nport),lib_hdmi_get_bch_2bit_error(nport));
	HDMI_EMG("BGRHV = %x\n",(lib_hdmi_read_rgb_hv_status(nport) & 0x000000F8) >> 3);
	HDMI_EMG("[CH B G R](%05d, %05d, %05d); [BIT B G R](%05d, %05d, %05d)\n",  ch_b_err, ch_g_err, ch_r_err , bit_b_err, bit_g_err, bit_r_err);
	
	lib_hdmi_set_fw_debug_bit(DEBUG_26_BYPASS_DETECT, 0);
	lib_hdmi_set_fw_debug_bit(DEBUG_25_BYPASS_CHECK, 0);
}

void debug_hdmi_phy_scan(unsigned char nport)
{
}

#endif

//------------------------------------------------------------------------------
// HDMI PHY ISR 
//------------------------------------------------------------------------------

static bool hdmiPhyEnable = false;
static struct task_struct *hdmi_task;


static int _hdmi_rxphy_thread(void* arg)
{
	unsigned int polling_cycle_us;
	while (!kthread_should_stop() && hdmiPhyEnable == true) 
	{
		polling_cycle_us = GET_FLOW_CFG(HDMI_FLOW_CFG_HPD, HDMI_FLOW_CFG1_DEF_PHY_THREAD_CYCLE_MS)*1000;
		if((polling_cycle_us> 0) && (polling_cycle_us <64000))
		{
			wait_event_interruptible_hrtimeout(hdmi_thd_wait_qu,(!hdmiPhyEnable),ktime_set(0,polling_cycle_us*NSEC_PER_USEC));
		}
		else
		{
			msleep(10);
		}
		newbase_rxphy_isr();
		
	}

	HDMI_PRINTF("hdmi rx_phy thread terminated\n");
	hdmi_task = NULL;
	return 0;
}

void newbase_rxphy_isr_enable(char enable)
{
	int err = 0;

	if (enable)
	{
		if (hdmiPhyEnable != true)
		{
			hdmi_task = kthread_create(_hdmi_rxphy_thread, NULL, "hdmi_task");

			if (IS_ERR(hdmi_task))
			{
				err = PTR_ERR(hdmi_task);
				hdmi_task = NULL;
				hdmiPhyEnable = false;
				HDMI_EMG("%s , Unable to start kernel thread (err_id = %d),HDMI_phy\n", __func__,err);
				return ;
			}

			hdmiPhyEnable = true;
			wake_up_process(hdmi_task);
			HDMI_PRINTF("%s hdmi phy thread started\n" , __func__ );
		}
	}
	else
	{
		if (hdmiPhyEnable == true)
		{
			hdmiPhyEnable = false;
			wake_up(&hdmi_thd_wait_qu);
			if (hdmi_task)
			{
				if (kthread_stop(hdmi_task)==0)
				{
					HDMI_PRINTF("hdmi rx_phy thread stopped\n");
					hdmi_task = NULL;
				}
				else
					HDMI_EMG("hdmi rx_phy thread stopped error\n");
			}
			else
				HDMI_EMG("hdmi rx_phy thread stopped already\n");
		}
	}
}

HDMIRX_PHY_STRUCT_T* newbase_rxphy_get_status(unsigned char port)
{
	return (port < HDMI_PORT_TOTAL_NUM) ? &phy_st[port] : NULL;
}


//------------------------------------------------------------------
// HPD/DET
//------------------------------------------------------------------

const unsigned int dedicate_hpd_det_reg[] =
{
    PINMUX_ST_HPD_R1K_Ctrl0_reg,
    PINMUX_ST_HPD_R1K_Ctrl1_reg,
    PINMUX_ST_HPD_R1K_Ctrl2_reg,
    PINMUX_ST_HPD_R1K_Ctrl3_reg
};

#define DEDICATE_HPD_DET_CNT (sizeof(dedicate_hpd_det_reg)/sizeof(unsigned int))


/*---------------------------------------------------
 * Func : lib_hdmi_set_hdmi_hpd
 *
 * Desc : enable/disable hdmi hpd. K7LP provides 4
 *        pairs of special io pads for HDMI hpd & 5v_det
 *        control.
 *
 * Para : hpd_idx : which hpd/det pad to be used
 *        en      : enable/disable hpd
 *          0 : disable,
 *          others : enable
 *
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_set_hdmi_hpd(unsigned char hpd_idx, unsigned char en)
{
	unsigned int val;
	unsigned char i=0;
	unsigned char earc_hpd_idx=2;
	
	if (hpd_idx >= DEDICATE_HPD_DET_CNT)
	{
		return;
	}
	
	for (i = 0; i < HDMI_RX_PORT_MAX_NUM; i++)
	{
		HDMI_CHANNEL_T* p_pcbinfo = newbase_hdmi_get_pcbinfo(i);

		if (p_pcbinfo==NULL)
		{
			HDMI_EMG("newbase_hdmi_get_earc_port_index null pointer, port=%d\n", i);
			break;
		}
		if (p_pcbinfo->earc_support)
		{
			earc_hpd_idx= p_pcbinfo->dedicate_hpd_det_sel;
		}
	}

	if(hpd_idx == earc_hpd_idx)
		val = hdmi_in(dedicate_hpd_det_reg[hpd_idx]) & PINMUX_ST_HPD_R1K_Ctrl0_hdmirx_p0_reg_sel(HPD_EARC_R1K_SEL); //Internal R1K register control, 0.85Kohm~1.0Kohm
	else
		val = hdmi_in(dedicate_hpd_det_reg[hpd_idx]) & PINMUX_ST_HPD_R1K_Ctrl0_hdmirx_p0_reg_sel(HPD_R1K_SEL); //Internal R1K register control, 0.85Kohm~1.0Kohm

	if (en)
		val |= PINMUX_ST_HPD_R1K_Ctrl0_hdmirx_p0_r1k_en_mask;    // enable 1K resistance to HDMI 5V and disable pull down
	else
		val |= PINMUX_ST_HPD_R1K_Ctrl0_hdmirx_p0_pd_mask;        // disable 1K resistance to HDMI 5V and enable pull down

	hdmi_out(dedicate_hpd_det_reg[hpd_idx], val);
}


/*---------------------------------------------------
 * Func : lib_hdmi_get_hdmi_hpd
 *
 * Desc : get the status of the hdmi hpd io pad
 *
 * Para : hpd_idx : which hpd/det pad to be used
 *
 * Retn : 0 : disabled, 1 : eanbled
 *--------------------------------------------------*/
unsigned char lib_hdmi_get_hdmi_hpd(unsigned char hpd_idx)
{
    if (hpd_idx >= DEDICATE_HPD_DET_CNT)
        return 0;

    if (PINMUX_ST_HPD_R1K_Ctrl0_get_hdmirx_p0_r1k_en(hdmi_in(dedicate_hpd_det_reg[hpd_idx])))
        return 1;

    return 0;
}


/*---------------------------------------------------
 * Func : lib_hdmi_get_hdmi_5v_det
 *
 * Desc : get the status of the hdmi 5v det io pad
 *
 * Para : det_idx : which det pad to be used
 *
 * Retn : 0 : 5v absent, 1 : 5v present
 *--------------------------------------------------*/
unsigned char lib_hdmi_get_hdmi_5v_det(unsigned char det_idx)
{
    if (det_idx >= DEDICATE_HPD_DET_CNT)
        return 0;

    return PINMUX_ST_HPD_R1K_Ctrl0_get_hdmirx_p0_h5vdet(hdmi_in(dedicate_hpd_det_reg[det_idx]));
}


