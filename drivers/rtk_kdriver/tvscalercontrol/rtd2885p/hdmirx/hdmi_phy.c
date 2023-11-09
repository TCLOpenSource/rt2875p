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
#ifndef BUILD_QUICK_SHOW
#include <linux/kthread.h> //kthread_create()¡Bkthread_run()
#include <linux/err.h> //IS_ERR()¡BPTR_ERR()
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#else
#include <no_os/spinlock.h>
#include <no_os/wait.h>
#endif

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
unsigned char factory_or_power_only_mode_bch_irq_en = 1;
unsigned char OBJ_TV_only=0;
unsigned char CLK_Wake_Up=0;	//Need to be modified

/**********************************************************************************************
*
*	Funtion Declarations
*
**********************************************************************************************/




unsigned char newbase_rxphy_measure(unsigned char port);
unsigned char newbase_rxphy_is_clock_stable(unsigned char port);
void newbase_rxphy_job(unsigned char p);
unsigned char newbase_hdmi_set_phy(unsigned char port, unsigned int b, unsigned char frl_mode,unsigned char lane);

//dfe
void newbase_hdmi_dump_dfe_para(unsigned char nport, unsigned char lane_mode);
unsigned char newbase_hdmi_dfe_record(unsigned char nport, unsigned int clk);
unsigned char newbase_hdmi_dfe_close(unsigned char nport, unsigned int clk, unsigned char lane_mode, unsigned char phy_state);
void newbase_hdmi_dfe_hi_speed(unsigned char nport, unsigned int b_clk, unsigned char lane_mode);
void newbase_hdmi_dfe_6g_long_cable_patch(unsigned char nport);
//extern unsigned char newbase_hdmi_dfe_hi_speed_close(unsigned char nport);
void newbase_hdmi_dfe_mi_speed(unsigned char nport, unsigned int b_clk, unsigned char lane_mode);
void newbasse_hdmi_dfe_mid_adapthve(unsigned char nport, unsigned int b_clk);

//Z0
void lib_hdmi_z0_calibration(void);

//eq setting
#ifdef CONFIG_POWER_SAVING_MODE
void lib_hdmi_eq_pi_power_en(unsigned char bport,unsigned char en);
#endif
void lib_hdmi_eq_bias_band_setting(unsigned char bport, unsigned int b);


//dfe setting
void lib_hdmi_dfe_power(unsigned char port, unsigned char en, unsigned char lane_mode);
void lib_hdmi_set_dfe(unsigned char nport, unsigned int b, unsigned char dfe_mode, unsigned char lane_mode);
void lib_hdmi_set_dfe_mid_band(unsigned char nport, unsigned int b_clk, int dfe_mode, unsigned char lane_mode);
void lib_hdmi_dfe_manual_set(unsigned char nport, unsigned char lane_mode);
unsigned char lib_hdmi_dfe_tap0_fix(unsigned char nport,unsigned char lane_mode);
void lib_hdmi_tap0_fix_small_swing(unsigned char nport, unsigned char lane_mode);

void lib_hdmi_dfe_second_flow(unsigned char nport, unsigned int b_clk,unsigned char lane_mode);




void lib_hdmi_dfe_config_setting(unsigned char nport,unsigned char lane_mode);



//PLL/DEMUX/CDR
void lib_hdmi_cco_band_config(unsigned char nport, unsigned int bclk, HDMI_PHY_PARAM_T *phy_param);
void lib_hdmi_cmu_pll_en(unsigned char nport, unsigned char enable);
void lib_hdmi_demux_ck_vcopll_rst(unsigned char nport, unsigned char rst);
void lib_hdmi_cdr_rst(unsigned char nport, unsigned char rst);

//Watch Dog
void lib_hdmi_wdog(unsigned char nport);
void lib_hdmi_wdg_rst(unsigned char port, unsigned char rst);

//PHY TABLE
void lib_hdmi_set_phy_table(unsigned char bport, HDMI_PHY_PARAM_T *phy_param);
unsigned char lib_hdmi_get_phy_table(unsigned char nport,unsigned int b,HDMI_PHY_PARAM_T **p_phy_table);


//foreground K offset
void lib_hdmi_fg_koffset_manual_adjust(unsigned char port, unsigned int r, unsigned int g, unsigned int b, unsigned int ck);
void lib_hdmi_fg_koffset_manual(unsigned char port);
void lib_hdmi_fg_koffset_auto(unsigned char port);
void lib_hdmi_fg_koffset_proc(unsigned char nport);
void lib_hdmi_fg_koffset_disable(unsigned char port);
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


/**********************************************************************************************
*
*	Patch for [P0][K5Lp][OQA][PRODISSUE-141] HDMI3 Black Video to get power only mode or factory mode flag 
*
**********************************************************************************************/

#ifdef CONFIG_SUPPORT_SCALER_MODULE
#else//need to check by hdmi
#if 0 //No use  
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

			//if (port <HD20_PORT)
				newbase_rxphy_tmds_job(port);  //hd21 phy
			//else
				//newbase_rxphy_job(port);  //hd20 phy

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


void newbase_set_qs_rxphy_status(unsigned char port)
{
	unsigned int clk_tmp = 0;
	if (lib_hdmi_is_clk_ready(port)) {
		clk_tmp = lib_hdmi_get_clock(port);
		newbase_hdmi2p0_check_tmds_config(port); //check scdc flag
	}

	lib_hdmi_trigger_measure_start(port);

	if (clk_tmp > VALID_MIN_CLOCK) {
		//if ((newbase_hdmi2p0_get_clock40x_flag(port) || newbase_hdmi2p0_get_scramble_flag(port)) &&
		if ((newbase_hdmi2p0_get_clock40x_flag(port)) &&
			(clk_tmp < 1422 && clk_tmp > 699)) {	//It's possible for 40x only when clock=(3G~6G)/4 //WOSQRTK-9795
			clk_tmp <<= 2;				 //HDMI2.0, change clock to clock*4
			phy_st[port].rxphy_40x = 1;
		}
		phy_st[port].clk_pre = clk_tmp;
		phy_st[port].clk_stable = 1;
		HDMI_EMG("clk = %d\n", phy_st[port].clk_pre);
		phy_st[port].clk = clk_tmp;
		phy_st[port].frl_mode = MODE_TMDS;
		phy_st[port].lane_num= HDMI_3LANE;
		phy_st[port].clk_tolerance = MAX(clk_tmp>>7, 5);
		phy_st[port].clk_debounce_count = 0;
		phy_st[port].clk_unstable_count = 0;
		hdmi_rx[port].timing_t.tmds_clk_b = phy_st[port].clk;
	}

	SET_HDMI_DETECT_EVENT(port, HDMI_DETECT_PHY_CLOCK_STABLE);
	phy_st[port].phy_proc_state = PHY_PROC_DONE;
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
		//if (port < HD20_PORT)
			newbase_rxphy_tmds_job(port);  //hd21 phy
		//else
			//newbase_rxphy_job(port);  //hd20 phy

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

/*---------------------------------------------------
 * Func : newbase_hdmi_dfe_fw_le_scan
 *
 * Desc : calibrate le with CED, to find out the best
 *        le with the minimum character error rate
 *
 * Para : nport : HDMI port number
 *
 * Retn : port : target HDMI port
 *
 * Note : 0 : le parameter doesn't change
 *        1 : le parameter changed
 *--------------------------------------------------*/
int newbase_hdmi_dfe_fw_le_scan(unsigned char port)
{
	int i;
	int le[3] = {0, 0, 0};
	int tap0[3] = {0, 0, 0};
	unsigned int ced[3] = {0, 0, 0};
	unsigned int try_ofst[6] = {-1, 1, -2, 2, -3, 3};
	int best_ced = 0xFFFFFFFF;
	int best_le = le[1];
	unsigned int ced_threshold = GET_FLOW_CFG(HDMI_FLOW_CFG_DFE, HDMI_FLOW_CFG5_FW_LE_SCAN_CED_THRESHOLD);
	unsigned int period = 60;

	for (i=0; i<3; i++)
	{
		le[i] = lib_hdmi_dfe_get_le(port,i);
		tap0[i] = lib_hdmi_dfe_get_tap0(port,i);
	}

	DFE_WARN("HDMI[p%d] newbase_hdmi_dfe_fw_le_scan. init le=%d/%d/%d, tap0=%d/%d/%d ced_threshold=%d (ced_chk_cnt=%d)\n",
		port, le[0], le[1], le[2], tap0[0], tap0[1], tap0[2], ced_threshold, phy_st[port].ced_thr_chk_cnt);

	newbase_hdmi_fw_ced_measure(port, ced, period);



	if( (ced[0] ==32767 || ced[1]  ==32767  || ced[2] ==32767 )&&((phy_st[port].ced_thr_chk_cnt<15)))
	{
		DFE_WARN("HDMI[p%d] newbase_hdmi_dfe_fw_le_scan, Abnormal CED=%d/%d/%d, Need Reset PHY\n", port, ced[0], ced[1], ced[2]);
		phy_st[port].ced_thr_chk_cnt++;

		if (newbase_hdmi_tmds_scramble_off_sync(port) && phy_st[port].ced_thr_chk_cnt>=3)
		{
			DFE_WARN("HDMI[p%d] newbase_hdmi_dfe_fw_le_scan, Abnormal CED=%d/%d/%d with TMDS off sync, skip le scan\n", port, ced[0], ced[1], ced[2]);
			return LE_NO_CHANGE;
		}

		return NEED_RESET_PHY;
	}

	if (ced[0] < ced_threshold &&
		ced[1] < ced_threshold &&
		ced[2] < ced_threshold)
	{
		DFE_WARN("HDMI[p%d] newbase_hdmi_dfe_fw_le_scan, good CED=%d/%d/%d, skip le scan\n", port, ced[0], ced[1], ced[2]);
		return LE_NO_CHANGE;
	}

	DFE_WARN("HDMI[p%d] newbase_hdmi_dfe_fw_le_scan, bad CED=%d/%d/%d, do le scan \n", port, ced[0], ced[1], ced[2]);
	newbase_hdmi_set_badcable_flag(port,1);
	for (i=0 ; i <6 ; i++)
	{
		int nle = le[1] + try_ofst[i];

		if (IS_VALID_LE(nle)==0)   // invalid LE, skip it
			continue;

		lib_hdmi_dfe_update_le(port, le[0], nle, le[2]);  // update LE paramete

		newbase_hdmi_fw_ced_measure(port, ced, period);

		DFE_WARN("HDMI[p%d] newbase_hdmi_dfe_fw_le_scan. round=%d, LE=%d/%d(%d+(%d))/%d, ced=%d/%d/%d \n",
			port, i,
			lib_hdmi_dfe_get_le(port,0),
			lib_hdmi_dfe_get_le(port,1), le[1],try_ofst[i],
			lib_hdmi_dfe_get_le(port,2),
			ced[0], ced[1], ced[2]);

		if (ced[1] < best_ced)  // Note: here we only handled the Ch1, for R/B Channel , need to take care of BR swap function
		{
			best_ced = ced[1];
			best_le = nle;
		}

		if (best_ced==0)
			break;
	}

	lib_hdmi_dfe_update_le(port, le[0], best_le, le[2]);

	DFE_WARN("HDMI[p%d] newbase_hdmi_dfe_fw_le_scan Best LE=%d->%d (CED=%d)\n", port, le[1], best_le, best_ced);

	if (best_le != le[1])
	{
		phy_st[port].dfe_t[1].le = lib_hdmi_dfe_get_le(port, 1);  // update ch1 le parameter
		newbase_hdmi_dump_dfe_para(port, phy_st[port].lane_num);
		return LE_CHANGED;
	}

	return LE_NO_CHANGE;
}
#if 0
void newbase_rxphy_job(unsigned char p)
{
	if (p < HD20_PORT) {
		HDMI_EMG("port=%d error %s need to HD20 port\n",p,__func__);
		return;
	}
	switch (phy_st[p].phy_proc_state) {
	case PHY_PROC_INIT:

		if (newbase_hdmi_set_phy(p, phy_st[p].clk,phy_st[p].frl_mode,phy_st[p].lane_num))
			newbase_rxphy_reset_setphy_proc(p);
		newbase_hdmi_init_disparity_ltp_var(p);
		newbase_hdmi_reset_meas_counter(p);
		newbase_rxphy_set_apply_clock(p,phy_st[p].clk);
		newbase_hdmi_open_err_detect(p);

		if (phy_st[p].clk <= clock_bound_45m) {
			phy_st[p].phy_proc_state= PHY_PROC_CLOSE_ADP;
		} else {
			phy_st[p].phy_proc_state= PHY_PROC_RECORD;
		}
		break;
	case PHY_PROC_RECORD:
		if (newbase_hdmi_dfe_record(p, phy_st[p].clk)) {
			phy_st[p].phy_proc_state = PHY_PROC_CLOSE_ADP;
			// go to PHY_PROC_CLOSE_ADP without taking a break;
		} else {
			break;
		}
	case PHY_PROC_CLOSE_ADP:
		if (newbase_hdmi_dfe_close(p, phy_st[p].clk,phy_st[p].lane_num,PHY_PROC_CLOSE_ADP)) {

			lib_hdmi_fg_koffset_auto(p);
			newbase_hdmi2p0_detect_config(p);

			if (GET_FLOW_CFG(HDMI_FLOW_CFG_DFE, HDMI_FLOW_CFG5_FW_LE_SCAN_ENABLE))
				phy_st[p].phy_proc_state = PHY_PROC_FW_LE_SCAN;
			else
				phy_st[p].phy_proc_state = PHY_PROC_DONE;

#if BIST_DFE_SCAN
			if (lib_hdmi_get_fw_debug_bit(DEBUG_30_DFE_SCAN_TEST) && (phy_st[p].clk > clock_bound_45m)) {
				debug_hdmi_dfe_scan(p,phy_st[p].lane_num);
			}
#endif
#if BIST_PHY_SCAN
		if (lib_hdmi_get_fw_debug_bit(DEBUG_22_PHY_SCAN_TEST))
			debug_hdmi_phy_scan(p);
#endif

		} else {
			newbase_rxphy_reset_setphy_proc(p);
		}
		break;

	case PHY_PROC_FW_LE_SCAN:
		if (GET_FLOW_CFG(HDMI_FLOW_CFG_DFE, HDMI_FLOW_CFG5_FW_LE_SCAN_ENABLE))
		{
			if(newbase_hdmi_dfe_fw_le_scan(p)==NEED_RESET_PHY)
			{
				newbase_rxphy_reset_setphy_proc(p);
			}
			else
			{
				phy_st[p].phy_proc_state = PHY_PROC_DONE;
			}

		}
		else
		{
			phy_st[p].phy_proc_state = PHY_PROC_DONE;
		}
		break;

	case PHY_PROC_DONE:
		phy_st[p].recovery = (phy_st[p].le_re_adaptive) ? 1 : 0;  // setup recovery flag is le_readaptive is done
		if (lib_hdmi_get_fw_debug_bit(DEBUG_18_PHY_DBG_MSG))
			debug_hdmi_dump_msg(p);
		#if HDMI20_PHY_ERROR_CORRECTION_EN
			lib_hdmi20_phy_error_handling(p);
		#endif
		break;




	/**********************************************************************/
	//
	//	Belowing are for recovery flow in check_mode stage.
	//
	/**********************************************************************/
	case PHY_PROC_RECOVER_6G_LONGCABLE:
		newbase_hdmi_dfe_hi_speed(p, phy_st[p].clk,phy_st[p].lane_num);
		newbase_hdmi_err_detect_add(p);
		phy_st[p].phy_proc_state = PHY_PROC_RECOVER_6G_LONGCABLE_RECORD;
		break;
	case PHY_PROC_RECOVER_6G_LONGCABLE_RECORD:
		if (newbase_hdmi_dfe_record(p, phy_st[p].clk)) {
			phy_st[p].phy_proc_state = PHY_PROC_RECOVER_6G_LONGCABLE_CLOSE_ADP;
			// go to PHY_PROC_RECOVER_HI_CLOSE_ADP;
		} else {
			break;
		}
	case PHY_PROC_RECOVER_6G_LONGCABLE_CLOSE_ADP:
		newbase_hdmi_dfe_close(p, phy_st[p].clk, phy_st[p].lane_num,PHY_PROC_RECOVER_6G_LONGCABLE_CLOSE_ADP);
		if (GET_FLOW_CFG(HDMI_FLOW_CFG_DFE, HDMI_FLOW_CFG5_FW_LE_SCAN_ENABLE))
			phy_st[p].phy_proc_state = PHY_PROC_FW_LE_SCAN;
		else
			phy_st[p].phy_proc_state = PHY_PROC_DONE;
		break;

	case PHY_PROC_RECOVER_HI:
		newbase_hdmi_dfe_hi_speed(p, phy_st[p].clk,phy_st[p].lane_num);
		newbase_hdmi_err_detect_add(p);
		phy_st[p].phy_proc_state = PHY_PROC_RECORD;
		break;

	case PHY_PROC_RECOVER_MI:
		newbase_hdmi_dfe_mi_speed(p, phy_st[p].clk,phy_st[p].lane_num);
		newbase_hdmi_err_detect_add(p);	 
		phy_st[p].phy_proc_state = PHY_PROC_RECORD;
		break;

	case PHY_PROC_RECOVER_MID_BAND_LONGCABLE:
		newbasse_hdmi_dfe_mid_adapthve(p, phy_st[p].clk);
		newbase_hdmi_err_detect_add(p);	
		if (GET_FLOW_CFG(HDMI_FLOW_CFG_DFE, HDMI_FLOW_CFG5_FW_LE_SCAN_ENABLE))
			phy_st[p].phy_proc_state = PHY_PROC_FW_LE_SCAN;
		else
			phy_st[p].phy_proc_state = PHY_PROC_DONE;
		break;

	default:
		phy_st[p].phy_proc_state = PHY_PROC_INIT;
		break;
	}

}
#endif
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

unsigned char newbase_rxphy_is_tmds_mode(unsigned char port)
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


unsigned char newbase_hdmi_set_phy(unsigned char port, unsigned int b, unsigned char frl_mode,unsigned char lane)
{  
#if 0
	unsigned char mode;
	bool b_mode_found = 0;
	HDMI_PHY_PARAM_T *p_phy_table = hdmi_phy_param;
	unsigned char tb_size = 0;

	HDMI_PRINTF("%s start[p%d]=%d frl=%d lane=%d\n", __func__, port, b,frl_mode,lane);

	if (b == 0) return 1;

#ifdef HDMI_FIX_HDMI_POWER_SAVING
	newbase_hdmi_reset_power_saving_state(port);
#endif

	lib_hdmi_mac_reset(port);

	lib_hdmi_cdr_rst(port, 1);
	lib_hdmi_demux_ck_vcopll_rst(port, 1);
	lib_hdmi_wdg_rst(port, 1);
	lib_hdmi_cmu_pll_en(port, 0);
	
	lib_hdmi_eq_bias_band_setting(port, b);

	tb_size = lib_hdmi_get_phy_table(port,b,&p_phy_table);

	if (tb_size == 0) return 1;

	for (mode=0; mode<tb_size; mode++) {
		if ((p_phy_table[mode].b_upper > b) && (p_phy_table[mode].b_lower <= b)) {
			lib_hdmi_cco_band_config(port, b, &p_phy_table[mode]);
			lib_hdmi_set_phy_table(port, &p_phy_table[mode]);
	              b_mode_found = 1;
	              break;
		}
	}
	
	if (!b_mode_found) {
	        HDMI_EMG("Not Found in PhyMode (b=%d)\n", b);
	        return 1;
	}
	

#ifdef CONFIG_POWER_SAVING_MODE
	lib_hdmi_eq_pi_power_en(port,1);
#endif

	lib_hdmi_wdg_rst(port, 0);

	lib_hdmi_wdog(port);
	udelay(5);

	lib_hdmi_cmu_pll_en(port, 1);
	udelay(1);

	lib_hdmi_demux_ck_vcopll_rst(port, 0);
	udelay(1);
	
	lib_hdmi_cdr_rst(port, 0);
	udelay(1);

	//check after conard
	lib_hdmi_dfe_config_setting(port,lane);

	lib_hdmi_fg_koffset_disable(port);

	lib_hdmi_fg_koffset_proc(port);
	if (b > clock_bound_110m) {
		lib_hdmi_set_dfe(port, b, 3, lane);
	} else if ((b > clock_bound_45m) && (b < clock_bound_110m)) {
		lib_hdmi_set_dfe_mid_band(port, b, 2, lane);
	} else {
		lib_hdmi_dfe_manual_set(port,lane);
	}

	udelay(1);
	
	if(newbase_hdcp_get_auth_mode(port) == HDCP_OFF || newbase_hdcp_get_auth_mode(port) == NO_HDCP )
	{// No AKE_INIT of HDCP22, open HDCP14 and HDCP22 at the same time
		HDMI_WARN("HDMI Port[%d] HDCP Enable after set phy!! (b=%d)\n", port, b);
		newbase_hdmi_switch_hdcp_mode(port, NO_HDCP);
		newbase_hdmi_reset_hdcp_once(port, FALSE);
	}
	else
	{
		HDMI_WARN("HDMI Port[%d] HDCP already start before set phy!! (b=%d)\n", port, b);
	}

	lib_hdmi_mac_release(port, frl_mode);
#endif	
	return 0;

}

unsigned char newbase_hdmi_dfe_record(unsigned char nport, unsigned int clk)
{
	if (lib_hdmi_dfe_is_record_enable(nport)) {
		lib_hdmi_dfe_record_enable(nport, 0);
		return TRUE; // stop recocrding
	} else {
		lib_hdmi_dfe_record_enable(nport, 1);
		return FALSE; // start recoding min/max
	}
}

unsigned char newbase_hdmi_dfe_close(unsigned char nport, unsigned int clk, unsigned char lane_mode,unsigned char phy_state)
{
	unsigned char lane;
	
	lib_hdmi_set_dfe_close(nport,lane_mode);

	 if (!lib_hdmi_clock40x_sync_status(nport)) {
                HDMI_EMG("%s: scdc sync status=0\n", __func__);
        }

	switch (phy_state) {
	case PHY_PROC_CLOSE_ADP:
		if (clk > 2900) { // > 3G
			lib_hdmi_load_le_max(nport,lane_mode);
		} else {
			lib_hdmi_dfe_examine_le_coef(nport, clk,lane_mode);
		}

		newbase_hdmi_tap2_adjust_with_le(nport,clk,lane_mode);
			
		if (newbase_hdmi_dfe_threshold_check(nport,clk,lane_mode) && (phy_st[nport].dfe_thr_chk_cnt < 15)) {
			phy_st[nport].dfe_thr_chk_cnt++;
			return FALSE;
		}

		break;
	case PHY_PROC_RECOVER_6G_LONGCABLE_CLOSE_ADP:
		newbase_hdmi_dfe_6g_long_cable_patch(nport); //check FRL mode need this patch or not ?
		break;
	default:
		break;
	}

	
	if (phy_fac_eq_st[nport].eq_mode == EQ_MANUAL_ON) {		
		newbase_hdmi_manual_eq(nport,	phy_fac_eq_st[nport].manual_eq[0] ,phy_fac_eq_st[nport].manual_eq[1] ,phy_fac_eq_st[nport].manual_eq[2]); 
	}
	
	for (lane=0; lane< lane_mode; lane++) {
		phy_st[nport].dfe_t[lane].tap0max = lib_hdmi_dfe_get_tap0max(nport, lane);
		phy_st[nport].dfe_t[lane].tap0min = lib_hdmi_dfe_get_tap0min(nport, lane);
		phy_st[nport].dfe_t[lane].vth = lib_hdmi_dfe_get_vth(nport, lane);
		phy_st[nport].dfe_t[lane].tap0 = lib_hdmi_dfe_get_tap0(nport, lane);
		phy_st[nport].dfe_t[lane].tap1 = lib_hdmi_dfe_get_tap1(nport, lane);
		phy_st[nport].dfe_t[lane].tap2 = lib_hdmi_dfe_get_tap2(nport, lane);
		phy_st[nport].dfe_t[lane].tap3 = 0;
		phy_st[nport].dfe_t[lane].tap4 = 0;
		phy_st[nport].dfe_t[lane].le = lib_hdmi_dfe_get_le(nport, lane);
		phy_st[nport].dfe_t[lane].lemax = lib_hdmi_dfe_get_lemax(nport, lane);
		phy_st[nport].dfe_t[lane].lemin = lib_hdmi_dfe_get_lemin(nport, lane);
	}

	newbase_hdmi_dump_dfe_para(nport,lane_mode);


	return TRUE;
}



void newbase_hdmi_dfe_hi_speed(unsigned char nport, unsigned int b_clk,unsigned char lane_mode)
{
#if 0
 
	unsigned char bgrc;
	HDMI_PRINTF("%s\n", __func__);

	lib_hdmi_dfe_power(nport, 1,lane_mode);

	// ENABLE  VTH & TAP0
	if (lane_mode == HDMI_4LANE)
		bgrc = LN_ALL;
	else
		bgrc = (LN_R|LN_G|LN_B);
	
	lib_hdmi_dfe_init_vth(nport, bgrc, 0x05);
	lib_hdmi_dfe_init_tap0(nport, bgrc, 0x14);

	hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_Vth),(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_Vth));
	hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_Vth),(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_Vth));
	hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_Vth),(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_Vth));
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(DFE_REG_DFE_EN_L3_reg,~(REG_dfe_adapt_en_lane3_TAP0|REG_dfe_adapt_en_lane3_Vth),(REG_dfe_adapt_en_lane3_TAP0|REG_dfe_adapt_en_lane3_Vth));

	lib_hdmi_dfe_second_flow(nport, b_clk,lane_mode);

	lib_hdmi_dfe_init_tap1(nport, bgrc, 0x0c);
	lib_hdmi_dfe_init_tap2(nport, bgrc, 0x00);
	lib_hdmi_dfe_init_le(nport, bgrc, 0x08);

	if (b_clk > 2700) {  // 3G need tap2 but below not need
		hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(REG_dfe_adapt_en_lane0_TAP1|REG_dfe_adapt_en_lane0_LE|REG_dfe_adapt_en_lane0_TAP2),(REG_dfe_adapt_en_lane0_TAP1|REG_dfe_adapt_en_lane0_LE|REG_dfe_adapt_en_lane0_TAP2));
		hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(REG_dfe_adapt_en_lane1_TAP1|REG_dfe_adapt_en_lane1_LE|REG_dfe_adapt_en_lane1_TAP2),(REG_dfe_adapt_en_lane1_TAP1|REG_dfe_adapt_en_lane1_LE|REG_dfe_adapt_en_lane1_TAP2));
		hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(REG_dfe_adapt_en_lane2_TAP1|REG_dfe_adapt_en_lane2_LE|REG_dfe_adapt_en_lane2_TAP2),(REG_dfe_adapt_en_lane2_TAP1|REG_dfe_adapt_en_lane2_LE|REG_dfe_adapt_en_lane2_TAP2));
		if (lane_mode == HDMI_4LANE)
			hdmi_mask(DFE_REG_DFE_EN_L3_reg,~(REG_dfe_adapt_en_lane3_TAP1|REG_dfe_adapt_en_lane3_LE|REG_dfe_adapt_en_lane3_TAP2),(REG_dfe_adapt_en_lane3_TAP1|REG_dfe_adapt_en_lane3_LE|REG_dfe_adapt_en_lane3_TAP2));
	} else {
		hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(REG_dfe_adapt_en_lane0_TAP1|REG_dfe_adapt_en_lane0_LE|REG_dfe_adapt_en_lane0_TAP2),(REG_dfe_adapt_en_lane0_TAP1|REG_dfe_adapt_en_lane0_LE));
		hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(REG_dfe_adapt_en_lane1_TAP1|REG_dfe_adapt_en_lane1_LE|REG_dfe_adapt_en_lane1_TAP2),(REG_dfe_adapt_en_lane1_TAP1|REG_dfe_adapt_en_lane1_LE));
		hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(REG_dfe_adapt_en_lane2_TAP1|REG_dfe_adapt_en_lane2_LE|REG_dfe_adapt_en_lane2_TAP2),(REG_dfe_adapt_en_lane2_TAP1|REG_dfe_adapt_en_lane2_LE));
		if (lane_mode == HDMI_4LANE)
			hdmi_mask(DFE_REG_DFE_EN_L3_reg,~(REG_dfe_adapt_en_lane3_TAP1|REG_dfe_adapt_en_lane3_LE|REG_dfe_adapt_en_lane3_TAP2),(REG_dfe_adapt_en_lane3_TAP1|REG_dfe_adapt_en_lane3_LE));

	}
#endif
}

void newbase_hdmi_dfe_6g_long_cable_patch(unsigned char nport)
{
 
	signed char tap2;
	unsigned char lemax;

	tap2 = lib_hdmi_dfe_get_tap2(nport, 1);
	lib_hdmi_dfe_init_tap2(nport, LN_G, MIN(tap2+6, 10));

	lemax = lib_hdmi_dfe_get_lemax(nport, 1);
	lemax += lib_hdmi_dfe_get_tap1max(nport, 1);
	lib_hdmi_dfe_init_tap1(nport, LN_G, MIN(lemax+4, 63));

	HDMI_EMG("%s end\n", __func__);
	
}

#if 0
unsigned char newbase_hdmi_dfe_hi_speed_close(unsigned char nport)
{
	unsigned char lane;

	lib_hdmi_set_dfe_close(nport);

	newbase_hdmi_dfe_6g_long_cable_patch(nport);

	for (lane=0; lane<3; lane++) {
		phy_st[nport].dfe_t[lane].tap0max = lib_hdmi_dfe_get_tap0max(nport, lane);
		phy_st[nport].dfe_t[lane].tap0min = lib_hdmi_dfe_get_tap0min(nport, lane);
		phy_st[nport].dfe_t[lane].vth = lib_hdmi_dfe_get_vth(nport, lane);
		phy_st[nport].dfe_t[lane].tap0 = lib_hdmi_dfe_get_tap0(nport, lane);
		phy_st[nport].dfe_t[lane].tap1 = lib_hdmi_dfe_get_tap1(nport, lane);
		phy_st[nport].dfe_t[lane].tap2 = lib_hdmi_dfe_get_tap2(nport, lane);
		phy_st[nport].dfe_t[lane].le = lib_hdmi_dfe_get_le(nport, lane);
		phy_st[nport].dfe_t[lane].lemax = lib_hdmi_dfe_get_lemax(nport, lane);
		phy_st[nport].dfe_t[lane].lemin = lib_hdmi_dfe_get_lemin(nport, lane);
	}

	newbase_hdmi_dump_dfe_para(nport);

	lib_hdmi_dfe_power(nport, 0);

	return TRUE;
}
#endif

void newbase_hdmi_dfe_mi_speed(unsigned char nport, unsigned int b_clk,unsigned char lane_mode)
{
#if  0
	HDMI_PRINTF("%s\n", __func__);

	lib_hdmi_dfe_power(nport, 1,lane_mode);

	// ENABLE  VTH & TAP0
	hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_Vth),(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_Vth));
	hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_Vth),(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_Vth));
	hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_Vth),(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_Vth));
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(DFE_REG_DFE_EN_L3_reg,~(REG_dfe_adapt_en_lane3_TAP0|REG_dfe_adapt_en_lane3_Vth),(REG_dfe_adapt_en_lane3_TAP0|REG_dfe_adapt_en_lane3_Vth));

	udelay(1);

	lib_hdmi_dfe_second_flow(nport, b_clk,lane_mode);
//	lib_hdmi_fg_koffset_proc(nport, lane_mode);
	
//	lib_hdmi_tap0_fix_small_swing(nport,lane_mode);
//	lib_hdmi_fg_koffset_proc(nport, lane_mode);

	if (b_clk > 2700) {  // 3G need tap2 but below not need
		hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(REG_dfe_adapt_en_lane0_TAP1|REG_dfe_adapt_en_lane0_LE|REG_dfe_adapt_en_lane0_TAP2),(REG_dfe_adapt_en_lane0_TAP1|REG_dfe_adapt_en_lane0_LE|REG_dfe_adapt_en_lane0_TAP2));
		hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(REG_dfe_adapt_en_lane1_TAP1|REG_dfe_adapt_en_lane1_LE|REG_dfe_adapt_en_lane1_TAP2),(REG_dfe_adapt_en_lane1_TAP1|REG_dfe_adapt_en_lane1_LE|REG_dfe_adapt_en_lane1_TAP2));
		hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(REG_dfe_adapt_en_lane2_TAP1|REG_dfe_adapt_en_lane2_LE|REG_dfe_adapt_en_lane2_TAP2),(REG_dfe_adapt_en_lane2_TAP1|REG_dfe_adapt_en_lane2_LE|REG_dfe_adapt_en_lane2_TAP2));
		if (lane_mode == HDMI_4LANE)
			hdmi_mask(DFE_REG_DFE_EN_L3_reg,~(REG_dfe_adapt_en_lane3_TAP1|REG_dfe_adapt_en_lane3_LE|REG_dfe_adapt_en_lane3_TAP2),(REG_dfe_adapt_en_lane3_TAP1|REG_dfe_adapt_en_lane3_LE|REG_dfe_adapt_en_lane3_TAP2));


	} else {
		hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(REG_dfe_adapt_en_lane0_TAP1|REG_dfe_adapt_en_lane0_LE|REG_dfe_adapt_en_lane0_TAP2),(REG_dfe_adapt_en_lane0_TAP1|REG_dfe_adapt_en_lane0_LE));
		hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(REG_dfe_adapt_en_lane1_TAP1|REG_dfe_adapt_en_lane1_LE|REG_dfe_adapt_en_lane1_TAP2),(REG_dfe_adapt_en_lane1_TAP1|REG_dfe_adapt_en_lane1_LE));
		hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(REG_dfe_adapt_en_lane2_TAP1|REG_dfe_adapt_en_lane2_LE|REG_dfe_adapt_en_lane2_TAP2),(REG_dfe_adapt_en_lane2_TAP1|REG_dfe_adapt_en_lane2_LE));
		if (lane_mode == HDMI_4LANE)
			hdmi_mask(DFE_REG_DFE_EN_L3_reg,~(REG_dfe_adapt_en_lane3_TAP1|REG_dfe_adapt_en_lane3_LE|REG_dfe_adapt_en_lane3_TAP2),(REG_dfe_adapt_en_lane3_TAP1|REG_dfe_adapt_en_lane3_LE));

	}

#endif
	
}

void newbasse_hdmi_dfe_mid_adapthve(unsigned char nport, unsigned int b_clk)
{
#if 0
	unsigned char le0;

//	HDMI_PRINTF("%s\n", __func__);
	le0 = lib_hdmi_dfe_get_le(nport,0);

	if (le0 >= 8)
		return;
//	le1 = lib_hdmi_dfe_get_le(nport,1);
//	le2 = lib_hdmi_dfe_get_le(nport,2);

	lib_hdmi_dfe_init_le(nport, (LN_R|LN_G|LN_B), (le0 + 2));

	HDMI_PRINTF("le=%x\n", (le0 + 2));
#endif
}


void newbase_hdmi_reset_thr_cnt(unsigned char port)
{
	//le  abnormal counter
	phy_st[port].dfe_thr_chk_cnt = 0;
	phy_st[port].ced_thr_chk_cnt=0;
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
#if 0
	if (phy_st[port].char_err_loop == 0) {
		lib_hdmi_char_err_start(port, 0, TMDS_CHAR_ERR_MAX_LOOP);
		phy_st[port].char_err_loop++;

	} else if (phy_st[port].char_err_loop > TMDS_CHAR_ERR_MAX_LOOP) {
		phy_st[port].char_err_loop = 0;
		phy_st[port].detect_6g_longcable = 0;

		if (lib_hdmi_char_err_get_error(port, &r_err, &g_err, &b_err)) {

			if ((g_err > TMDS_CHAR_ERR_THREAD) && (r_err < 10) && (b_err < 10) && ((phy_st[port].dfe_t[1].le+phy_st[port].dfe_t[1].tap1)>10)) {
				phy_st[port].detect_6g_longcable = 1;
			}

			if ((r_err > TMDS_CHAR_ERR_THREAD) || (g_err > TMDS_CHAR_ERR_THREAD) || (b_err > TMDS_CHAR_ERR_THREAD)) {
				// Do something to save the world
				HDMI_PRINTF("[CHAR_ERR](%d, %d, %d)\n", r_err, g_err, b_err);
			}
			return 1;
		}

	} else {
		phy_st[port].char_err_loop++;
	}
#endif
	return 0;

}
unsigned char newbase_hdmi_detect_6g_longcable(unsigned char port,unsigned int r_err,unsigned int g_err,unsigned int b_err)
{
	if(phy_st[port].detect_6g_longcable_enable == 0) //char_err_loop > TMDS_CHAR_ERR_MAX_LOOP will disable 6g long cable detect
		return 0;

	if ((g_err > TMDS_CHAR_ERR_THREAD) && (r_err < 10) && (b_err < 10) && ((phy_st[port].dfe_t[1].le+phy_st[port].dfe_t[1].tap1)>10)) {
		return 1;
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

unsigned char newbase_hdmi_get_badcable_flag(unsigned char port)
{
	return phy_st[port].badcable_flag;
}

void newbase_hdmi_set_badcable_flag(unsigned char port, unsigned char enable)
{
	 phy_st[port].badcable_flag = enable;
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

void newbase_hdmi_dump_dfe_para(unsigned char nport, unsigned char lane_mode)
{
	unsigned char lane;

#if HDMI_DFE_BRSWAP
	unsigned char j;
	if (nport >= HD20_PORT) {
		for (j=0; j<lane_mode; j++) {
			
			if (j == 0)
				lane = 2;
			else if (j == 2)
				lane = 0;
			else
				lane = j;
			
			DFE_WARN("HDMI[p%d] ********************Lane%d******************\n", nport, j);
			DFE_WARN("HDMI[p%d] Lane%d Vth=0x%x\n", nport, j, phy_st[nport].dfe_t[lane].vth);
			DFE_WARN("HDMI[p%d] Lane%d Tap0=0x%x, max=0x%02x, min=0x%02x\n", nport,
				j, phy_st[nport].dfe_t[lane].tap0, phy_st[nport].dfe_t[lane].tap0max, phy_st[nport].dfe_t[lane].tap0min);

			DFE_WARN("HDMI[p%d] Lane%d Tap1=0x%02x, Tap2=%+d\n",  nport,
				j, phy_st[nport].dfe_t[lane].tap1, phy_st[nport].dfe_t[lane].tap2);

			DFE_WARN("HDMI[p%d] Lane%d LE=0x%02x, max=0x%02x, min=0x%02x\n", nport,
				j, phy_st[nport].dfe_t[lane].le, phy_st[nport].dfe_t[lane].lemax, phy_st[nport].dfe_t[lane].lemin);
		}
	} else {

		for (lane=0; lane<lane_mode; lane++) {
			DFE_WARN("HDMI[p%d] ********************Lane%d******************\n", nport,lane);
			DFE_WARN("HDMI[p%d] Lane%d Vth=0x%x\n", nport, lane, phy_st[nport].dfe_t[lane].vth);
			DFE_WARN("HDMI[p%d] Lane%d Tap0=0x%x, max=0x%02x, min=0x%02x\n", nport,
				lane, phy_st[nport].dfe_t[lane].tap0, phy_st[nport].dfe_t[lane].tap0max, phy_st[nport].dfe_t[lane].tap0min);

			DFE_WARN("HDMI[p%d] Lane%d Tap1=0x%02x, Tap2=%+d\n", nport,
				lane, phy_st[nport].dfe_t[lane].tap1, phy_st[nport].dfe_t[lane].tap2);

			DFE_WARN("HDMI[p%d] Lane%d LE=0x%02x, max=0x%02x, min=0x%02x\n", nport,
				lane, phy_st[nport].dfe_t[lane].le, phy_st[nport].dfe_t[lane].lemax, phy_st[nport].dfe_t[lane].lemin);
		}
	}

#else
	for (lane=0; lane<lane_mode; lane++) {
		DFE_WARN("HDMI[p%d] ********************Lane%d******************\n", nport, lane);
			DFE_WARN("HDMI[p%d] Lane%d Vth=0x%x\n", nport, lane, phy_st[nport].dfe_t[lane].vth);
		DFE_WARN("HDMI[p%d] Lane%d Tap0=0x%x, max=0x%02x, min=0x%02x\n", nport,
			lane, phy_st[nport].dfe_t[lane].tap0, phy_st[nport].dfe_t[lane].tap0max, phy_st[nport].dfe_t[lane].tap0min);

		DFE_WARN("HDMI[p%d] Lane%d Tap1=0x%02x, Tap2=%+d\n", nport,
			lane, phy_st[nport].dfe_t[lane].tap1, phy_st[nport].dfe_t[lane].tap2);

		DFE_WARN("HDMI[p%d] Lane%d LE=0x%02x, max=0x%02x, min=0x%02x\n", nport,
			lane, phy_st[nport].dfe_t[lane].le, phy_st[nport].dfe_t[lane].lemax, phy_st[nport].dfe_t[lane].lemin);
	}
#endif

}


#ifdef CONFIG_POWER_SAVING_MODE
void newbase_hdmi_phy_pw_saving(unsigned char port)
{	
	//if (port < HD20_PORT) {
	//	return;
	//}

	lib_hdmi_cdr_rst(port, 1);
	lib_hdmi_demux_ck_vcopll_rst(port, 1);
	lib_hdmi_eq_pi_power_en(port,0);
//	lib_hdmi_wdg_rst(port, 1);
	lib_hdmi_cmu_pll_en(port, 0);
}

#endif


unsigned char lib_hdmi_clock40x_sync_status(unsigned char nport)
{
#if 1
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
	hdmi_mask(HDMIRX_PHY_mod_regd00,~HDMIRX_PHY_HD21_P0_MOD_REGD00_reg_p0_ck_md_rstb_mask,0);
	hdmi_mask(HDMIRX_PHY_mod_regd00,~HDMIRX_PHY_HD21_P0_MOD_REGD00_reg_p0_ck_md_rstb_mask,HDMIRX_PHY_HD21_P0_MOD_REGD00_reg_p0_ck_md_rstb_mask);
	hdmi_out(HDMIRX_PHY_mod_regd01, HDMIRX_PHY_HD21_P0_MOD_REGD01_p0_ck_md_ok_ro_mask);

}

void lib_hdmi_trigger_measure_start(unsigned char nport)
{

	hdmi_mask(HDMIRX_PHY_mod_regd00,~HDMIRX_PHY_HD21_P0_MOD_REGD00_reg_p0_ck_md_rstb_mask,0);

}

void lib_hdmi_trigger_measure_stop(unsigned char nport)
{

	hdmi_mask(HDMIRX_PHY_mod_regd00,~HDMIRX_PHY_HD21_P0_MOD_REGD00_reg_p0_ck_md_rstb_mask,HDMIRX_PHY_HD21_P0_MOD_REGD00_reg_p0_ck_md_rstb_mask);


}

unsigned char lib_hdmi_is_clk_ready(unsigned char nport)
{
 
	hdmi_out(HDMIRX_PHY_mod_regd01, HDMIRX_PHY_HD21_P0_MOD_REGD01_p0_ck_md_ok_ro_mask);
	udelay(40);
	if (HDMIRX_PHY_HD21_P0_MOD_REGD01_get_p0_ck_md_ok_ro(hdmi_in(HDMIRX_PHY_mod_regd01))) {
		return TRUE;
	} else {
		return FALSE;
	}


}

unsigned int lib_hdmi_get_clock(unsigned char nport)
{   
	return HDMIRX_PHY_HD21_P0_MOD_REGD01_get_p0_ck_md_count_ro(hdmi_in(HDMIRX_PHY_mod_regd01));

}

void lib_hdmi_hysteresis_en(unsigned char en)
{

	unsigned char nport;
	if (en) 
	{
		for (nport=0; nport<HDMI_PORT_TOTAL_NUM; nport++)
		{
			if (newbase_hdmi_phy_port_status(nport) == HDMI_PHY_PORT_NOTUSED)
				continue;

			hdmi_mask(HDMIRX_PHY_ck_cmu_1_reg,~(cmu_ck_1_HYS_AMP_EN|cmu_ck_1_MD_DET_SRC|cmu_ck_1_HYS_WIN_SEL|cmu_ck_1_CKIN_SEL|cmu_ck_1_NON_HYS_AMP_EN),
			(cmu_ck_1_HYS_AMP_EN|cmu_ck_1_NON_HYS_AMP_EN|cmu_ck_1_MD_DET_SRC|cmu_ck_1_ENHANCE_BIAS_10|cmu_ck_1_CKIN_SEL));
		}
	}
	else 
	{
		for (nport=0; nport<HDMI_PORT_TOTAL_NUM; nport++)
		{
			if (newbase_hdmi_phy_port_status(nport) == HDMI_PHY_PORT_NOTUSED)
				continue;

			hdmi_mask(HDMIRX_PHY_ck_cmu_1_reg,~(cmu_ck_1_HYS_AMP_EN|cmu_ck_1_MD_DET_SRC|cmu_ck_1_HYS_WIN_SEL|cmu_ck_1_CKIN_SEL|cmu_ck_1_NON_HYS_AMP_EN),
			(cmu_ck_1_ENHANCE_BIAS_01|cmu_ck_1_CKIN_SEL|cmu_ck_1_NON_HYS_AMP_EN));
		}
	}
}

void lib_hdmi_stb_tmds_clk_wakeup_en(unsigned char en)
{
#if 0 //
	unsigned int stb_hysteresis_window_otp = rtd_inl(EFUSE_DATAO_122_reg)>>23;
	
	unsigned int otp_done = (stb_hysteresis_window_otp&0x100)>>8;
	unsigned int port0_window = (otp_done==0x1)?(stb_hysteresis_window_otp&0x03) : 0;
	unsigned int port1_window = (otp_done==0x1)?(stb_hysteresis_window_otp&0x0c)>>2 : 0;

	if (en) {
		//HDMI Z0 set (CK Lane)
		hdmi_mask(HDMIRX_2P0_PHY_TOP_IN_reg, ~(TOP_IN_Z0_N|TOP_IN_Z0_P), 0x0);
		hdmi_mask(HDMIRX_2P0_PHY_Z0POW_reg, ~(HDMIRX_2P0_PHY_Z0POW_reg_z0_z300pow_mask|HDMIRX_2P0_PHY_Z0POW_reg_z0_z0pow_ck_mask), 
			(HDMIRX_2P0_PHY_Z0POW_reg_z0_z300pow_mask|HDMIRX_2P0_PHY_Z0POW_reg_z0_z0pow_ck_mask)); //[1] Noff [0]poff :0 =>normal

		hdmi_mask(reg_ck1[0],~(p0_ck_1_port_bias),p0_ck_1_port_bias);
		hdmi_mask(reg_ck1[1],~(p0_ck_1_port_bias),p0_ck_1_port_bias);

		hdmi_mask(HDMIRX_2P0_PHY_P0_R4_reg,~(_BIT3|_BIT2|_BIT1),((port0_window<<2)|_BIT1));	
		//REG_P0_R_13<1>=1, Port0/1 power saving switch enable(RL6617)
		//port-0 CK-detect(standby mode use) hysteresis window select

		hdmi_mask(HDMIRX_2P0_PHY_Z0POW_reg,~(_BIT21|_BIT20),(port1_window<<20));	
		//port-1 CK-detect(standby mode use) hysteresis window select
		
		hdmi_mask(HDMIRX_2P0_PHY_MD_reg,~(_BIT5),0);		// REG_CK_LATCH=0, CK_LATCH=0
		hdmi_mask(HDMIRX_2P0_PHY_MD_reg,~(_BIT5),_BIT5);	 // REG_CK_LATCH=1, CK_LATCH=1

	}else {
		hdmi_mask(HDMIRX_2P0_PHY_P0_R4_reg,~(_BIT1),0);	
		//REG_P0_R_13<1>=0, Port2/3 power saving switch disable(RL6617)
	}
#endif	
}

void lib_hdmi_phy_init(void)
{
#if 0
	unsigned char i;
	//enable bandgap power
	hdmi_mask(HDMIRX_2P0_PHY_TOP_IN_reg,~TOP_IN_BIAS_POW,TOP_IN_BIAS_POW);
	//hdmi_mask(HDMIRX_2P0_PHY_TOP_IN_reg, ~(TOP_IN_REG_IBHN_TUNE), _BIT20);  //merlin5 remove
	hdmi_mask(HDMIRX_2P0_PHY_TOP_IN_reg, ~(TOP_IN_Z0_N|TOP_IN_Z0_P), 0x0);

	for (i=HDMI_PORT2; i<HDMI_PORT_TOTAL_NUM; i++) {

		if(newbase_hdmi_phy_port_status(i) == HDMI_PHY_PORT_NOTUSED) 
			continue;

		hdmi_mask(HDMIRX_2P0_PHY_TOP_IN_reg, ~(TOP_IN_CK_TX_0<<(i%HDMI_2P0_TOTAL_NUM)), (TOP_IN_CK_TX_0<<(i%HDMI_2P0_TOTAL_NUM)));

		//default hdmi mode (other :MHL3.0 mode )
		lib_hdmi_ck_md_ref_sel(i); //HDMI mode
		lib_hdmi_fg_koffset_manual_adjust(i, 0x10, 0x10, 0x10,0x10);
	}

	lib_hdmi_z0_calibration();

	for (i=HDMI_PORT2; i<HDMI_PORT_TOTAL_NUM; i++) {

		if(newbase_hdmi_phy_port_status(i) == HDMI_PHY_PORT_NOTUSED)
			continue;

		lib_hdmi_z300_sel(i, 0);

#if !HDMI_HYSTERESIS_EN //hysteresis off
		
		hdmi_mask(reg_ck1[i%HDMI_2P0_TOTAL_NUM],
			~(p0_ck_2_CMU_CKIN_SEL| p0_ck_2_ACDR_CBUS_REF| p0_ck_2_CK_MD_DET_SEL| p0_ck_2_HYS_WIN_SEL| p0_ck_2_NON_HYS_AMP_EN| HDMIRX_2P0_PHY_P0_CK1_reg_p0_ck_1_mask),
			(p0_ck_2_CMU_CKIN_SEL|p0_ck_2_ACDR_CBUS_REF| p0_ck_2_ENHANCE_BIAS_01| p0_ck_2_NON_HYS_AMP_EN|p0_ck_1_port_bias|p0_ck_1_HYS_AMP_EN));  //sel non-hysteresis amp , but hys_amp always open ( share with ac- coupple)
#else

	hdmi_mask(reg_ck1[i%HDMI_2P0_TOTAL_NUM],
			~(p0_ck_2_CMU_CKIN_SEL| p0_ck_2_ACDR_CBUS_REF| p0_ck_2_CK_MD_DET_SEL| p0_ck_2_HYS_WIN_SEL| p0_ck_2_NON_HYS_AMP_EN| HDMIRX_2P0_PHY_P0_CK1_reg_p0_ck_1_mask),
			(p0_ck_2_CMU_CKIN_SEL|p0_ck_2_ACDR_CBUS_REF|p0_ck_2_CK_MD_DET_SEL| p0_ck_2_ENHANCE_BIAS_10| p0_ck_2_NON_HYS_AMP_EN|p0_ck_1_port_bias|p0_ck_1_HYS_AMP_EN));


#endif
		
			
		//hdmi_mask(DFE_HDMI_RX_PHY_LDO_reg, ~(DFE_HDMI_RX_PHY_LDO_reg_p0_ldo_pow_mask), (0));

		hdmi_mask(reg_rgb2[i%HDMI_2P0_TOTAL_NUM][0], ~P0_b_8_POW_PR, P0_b_8_POW_PR);
		hdmi_mask(reg_rgb2[i%HDMI_2P0_TOTAL_NUM][1], ~P0_b_8_POW_PR, P0_b_8_POW_PR);
		hdmi_mask(reg_rgb2[i%HDMI_2P0_TOTAL_NUM][2], ~P0_b_8_POW_PR, P0_b_8_POW_PR);
		
	}
	
#endif
}


void lib_hdmi_z0_set(unsigned char port, unsigned char lane, unsigned char enable, unsigned char CLK_Wake_Up)
{
#if 0
	unsigned int bmask = 0;
	unsigned int Z0bmask = HDMIRX_2P0_PHY_Z0POW_reg_z0_z0pow_ck(1<<(port%HDMI_2P0_TOTAL_NUM))|HDMIRX_2P0_PHY_Z0POW_reg_z0_z0pow_b(1<<(port%HDMI_2P0_TOTAL_NUM))|
									HDMIRX_2P0_PHY_Z0POW_reg_z0_z0pow_g(1<<(port%HDMI_2P0_TOTAL_NUM))|HDMIRX_2P0_PHY_Z0POW_reg_z0_z0pow_r(1<<(port%HDMI_2P0_TOTAL_NUM));

	HDMI_INFO("Set_Z0_En_%d, CLK_wakeup_%d, Port=%d, Z0bmask=%x\n", enable, CLK_Wake_Up,port, Z0bmask);
	if ((enable == 0) && (CLK_Wake_Up == 1))
	{
		HDMI_INFO("Z0_En=%d, Lane=%d\n", enable,lane);
		enable = 1;
		lane = LN_CK;
	}

	if (port < HDMI_PORT2) {
		HDMI_INFO("2.1_Z0_En=%d, Lane=%d\n", enable,lane);
		lib_hdmi21_z0_set(port,lane,enable);
		return;
	}

	//if (enable == 0)
		hdmi_mask(HDMIRX_2P0_PHY_MD_reg, ~(REG_CK_LATCH), 0);  //latch bit clear
		
	if (lane&LN_CK) {
		bmask |= HDMIRX_2P0_PHY_Z0POW_reg_z0_z0pow_ck(1<<(port%HDMI_2P0_TOTAL_NUM));
	}
	if (lane&LN_B) {
		bmask |= HDMIRX_2P0_PHY_Z0POW_reg_z0_z0pow_b(1<<(port%HDMI_2P0_TOTAL_NUM));
	}
	if (lane&LN_G) {
		bmask |= HDMIRX_2P0_PHY_Z0POW_reg_z0_z0pow_g(1<<(port%HDMI_2P0_TOTAL_NUM));
	}
	if (lane&LN_R) {
		bmask |= HDMIRX_2P0_PHY_Z0POW_reg_z0_z0pow_r(1<<(port%HDMI_2P0_TOTAL_NUM));
	}
	
	if (!bmask) {
		return;
	}

	hdmi_mask(HDMIRX_2P0_PHY_Z0POW_reg, ~Z0bmask, enable?bmask:0);
	hdmi_mask(HDMIRX_2P0_PHY_TOP_IN_reg, ~(TOP_IN_Z0_FIX_SELECT_0 << (2*(port%HDMI_2P0_TOTAL_NUM))), enable?(TOP_IN_Z0_FIX_SELECT_10 << (2*(port%HDMI_2P0_TOTAL_NUM))):0 );
	HDMI_INFO("Z0POW_reg=%x, TOP_IN_reg=%x, bmask=%x\n", hdmi_in(HDMIRX_2P0_PHY_Z0POW_reg),hdmi_in(HDMIRX_2P0_PHY_TOP_IN_reg),bmask);

	//if (enable == 0)
		hdmi_mask(HDMIRX_2P0_PHY_MD_reg, ~(REG_CK_LATCH), REG_CK_LATCH);  //latch bit set
#endif		
}


void lib_hdmi_z300_sel(unsigned char port, unsigned char mode)  // 0: active mode 1:vcm mode
{
#if 0
	if (port == HDMI_PORT2) {
              //do nothing
        } else if (port == HDMI_PORT3) {
             // do nothing
        } else {
        	return;
        }


	if (mode) {
		hdmi_mask(HDMIRX_2P0_PHY_Z0POW_reg, ~(HDMIRX_2P0_PHY_Z0POW_reg_z0_z300_sel((1<<(port%HDMI_2P0_TOTAL_NUM)))), HDMIRX_2P0_PHY_Z0POW_reg_z0_z300_sel((1<<(port%HDMI_2P0_TOTAL_NUM))));
	} else {
		hdmi_mask(HDMIRX_2P0_PHY_Z0POW_reg, ~(HDMIRX_2P0_PHY_Z0POW_reg_z0_z300_sel((1<<(port%HDMI_2P0_TOTAL_NUM)))), 0);
	}
#endif
}

void lib_hdmi_ck_md_ref_sel(unsigned char port)
{
#if 0
	unsigned int ck_addr;
	
	if (port == HDMI_PORT2) {
               ck_addr = HDMIRX_2P0_PHY_P0_CK1_reg;
        } else if (port == HDMI_PORT3) {
              ck_addr = HDMIRX_2P0_PHY_P1_CK1_reg;		
        } else {
        	return;
        }

	hdmi_mask(ck_addr, ~p0_ck_2_CK_MD_REF_SEL, p0_ck_2_CK_MD_REF_SEL);
#endif
}



void lib_hdmi_wdg_rst(unsigned char port, unsigned char rst)
{
 #if 0
	unsigned int reg_ck3;

  	if (port == HDMI_PORT2) {
               reg_ck3 = HDMIRX_2P0_PHY_P0_CK3_reg;
        } else if (port == HDMI_PORT3) {
              reg_ck3 = HDMIRX_2P0_PHY_P1_CK3_reg;		
        } else {
        	return;
        }

	if (rst) {
		hdmi_mask(reg_ck3, ~(P0_ck_9_CMU_PFD_RSTB|_BIT3|P0_ck_9_CMU_WDRST|_BIT1), (_BIT3|P0_ck_9_CMU_WDRST));	//PFD_RSTB  reset
	} else {
		hdmi_mask(reg_ck3, ~(P0_ck_9_CMU_PFD_RSTB|_BIT3|P0_ck_9_CMU_WDRST|_BIT1), (P0_ck_9_CMU_PFD_RSTB));  //release, normal Disable WD
	}
#endif	
}


void lib_hdmi_dfe_power(unsigned char port, unsigned char en, unsigned char lane_mode)
{
#if 0
 	 unsigned int R3_addr,G3_addr,B3_addr;
	
        if (port == HDMI_PORT2) {
                R3_addr = HDMIRX_2P0_PHY_P0_R3_reg;
                G3_addr = HDMIRX_2P0_PHY_P0_G3_reg;
                B3_addr = HDMIRX_2P0_PHY_P0_B3_reg;
        } else if (port == HDMI_PORT3) {
                R3_addr = HDMIRX_2P0_PHY_P1_R3_reg;
                G3_addr = HDMIRX_2P0_PHY_P1_G3_reg;
                B3_addr = HDMIRX_2P0_PHY_P1_B3_reg;	  			
        } else {
        	return;
        }

	hdmi_mask(R3_addr, ~(P0_b_9_DFE_ADAPTION_POW_EN|P0_b_9_DFE_TAP_EN), (DFE_HDMI_RX_PHY_P0_DFE_ADAPTION_EN((en!=0))|DFE_HDMI_RX_PHY_P0_b_tap_en(0x3)));
	hdmi_mask(G3_addr, ~(P0_b_9_DFE_ADAPTION_POW_EN|P0_b_9_DFE_TAP_EN), (DFE_HDMI_RX_PHY_P0_DFE_ADAPTION_EN((en!=0))|DFE_HDMI_RX_PHY_P0_b_tap_en(0x3)));
	hdmi_mask(B3_addr, ~(P0_b_9_DFE_ADAPTION_POW_EN|P0_b_9_DFE_TAP_EN), (DFE_HDMI_RX_PHY_P0_DFE_ADAPTION_EN((en!=0))|DFE_HDMI_RX_PHY_P0_b_tap_en(0x3)));

#endif		
}

void lib_hdmi_eq_set_clock_boundary(unsigned char port)
{

        
            clock_bound_3g = TMDS_3G;
            clock_bound_1p5g = TMDS_1p5G;
            clock_bound_45m = 430;
            //clock_bound_110m = 1042;
            clock_bound_110m = 1072;            

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

#if 0
static unsigned char _check_for_factory_mode_eq_apply(unsigned long bclk)
{
    switch(factory_or_power_only_mode_en)
    {
    case 1:
        if (bclk>2700)
            return 1;         // for LGE-110HAM Protobal Genearator 4K30 timing (TID=3)
        if ((bclk>380 && bclk<385)) // for LGE-110HAM Protobal Genearator 720x480p 60 timing (TID=0) clock=383
            return 1;
        break;

    case 2:
        return 1;   // force apply factory mode eq
    }

    return 0;
}
#endif
void lib_hdmi_eq_bias_band_setting(unsigned char bport, unsigned int b)
{
#if 0
        UINT32 B1_addr,G1_addr,R1_addr;
        UINT32 B2_addr,G2_addr,R2_addr;
        UINT32 B3_addr,G3_addr,R3_addr;

        if (bport == HDMI_PORT2) {
                B1_addr = HDMIRX_2P0_PHY_P0_B1_reg;
                B2_addr = HDMIRX_2P0_PHY_P0_B2_reg;
                B3_addr = HDMIRX_2P0_PHY_P0_B3_reg;
                G1_addr = HDMIRX_2P0_PHY_P0_G1_reg;
                G2_addr = HDMIRX_2P0_PHY_P0_G2_reg;
                G3_addr = HDMIRX_2P0_PHY_P0_G3_reg;
                R1_addr = HDMIRX_2P0_PHY_P0_R1_reg;
                R2_addr = HDMIRX_2P0_PHY_P0_R2_reg;
                R3_addr = HDMIRX_2P0_PHY_P0_R3_reg;
        } else if (bport == HDMI_PORT3) {
                B1_addr = HDMIRX_2P0_PHY_P1_B1_reg;
                B2_addr = HDMIRX_2P0_PHY_P1_B2_reg;
                B3_addr = HDMIRX_2P0_PHY_P1_B3_reg;
                G1_addr = HDMIRX_2P0_PHY_P1_G1_reg;
                G2_addr = HDMIRX_2P0_PHY_P1_G2_reg;
                G3_addr = HDMIRX_2P0_PHY_P1_G3_reg;
                R1_addr = HDMIRX_2P0_PHY_P1_R1_reg;
                R2_addr = HDMIRX_2P0_PHY_P1_R2_reg;
                R3_addr = HDMIRX_2P0_PHY_P1_R3_reg;
       } else {
               return;
        }
	
	lib_hdmi_eq_set_clock_boundary(bport);

	hdmi_mask(B2_addr, ~P0_b_6_TAP0_LE_ISO, P0_b_6_TAP0_LE_ISO);
	hdmi_mask(G2_addr, ~P0_b_6_TAP0_LE_ISO, P0_b_6_TAP0_LE_ISO);
	hdmi_mask(R2_addr, ~P0_b_6_TAP0_LE_ISO, P0_b_6_TAP0_LE_ISO);

        if ((b > clock_bound_3g) || (_check_for_factory_mode_eq_apply(b)))
        { // over  3G HBR

#if CLK_OVER_300M
		if (!newbase_hdmi2p0_get_clock40x_flag(bport)) {
			hdmi_mask(B2_addr, ~(P0_b_7_TAP0_ISEL|P0_b_7_LEQ_ISEL|P0_b_6_LEQ_BIT_RATE|P0_b_6_TAP0_HBR|P0_b_5_LEQ_CURRENT_ADJ), (P0_b_7_TAP0_ISEL_400uA|P0_b_7_LEQ_ISEL_200uA|P0_b_6_LEQ_BIT_RATE_MBR|P0_b_6_TAP0_HBR|P0_b_5_LEQ_CURRENT_ADJ));		//tap0
			hdmi_mask(G2_addr, ~(P0_g_7_TAP0_ISEL|P0_g_7_LEQ_ISEL|P0_g_6_LEQ_BIT_RATE|P0_g_6_TAP0_HBR|P0_g_5_LEQ_CURRENT_ADJ), (P0_b_7_TAP0_ISEL_400uA|P0_b_7_LEQ_ISEL_200uA|P0_g_6_LEQ_BIT_RATE_MBR|P0_g_6_TAP0_HBR|P0_b_5_LEQ_CURRENT_ADJ));		//tap0
			hdmi_mask(R2_addr, ~(P0_r_7_TAP0_ISEL|P0_r_7_LEQ_ISEL|P0_r_6_LEQ_BIT_RATE|P0_r_6_TAP0_HBR|P0_r_5_LEQ_CURRENT_ADJ), (P0_b_7_TAP0_ISEL_400uA|P0_b_7_LEQ_ISEL_200uA|P0_r_6_LEQ_BIT_RATE_MBR|P0_r_6_TAP0_HBR|P0_b_5_LEQ_CURRENT_ADJ));		//tap0				
		} else {
			hdmi_mask(B2_addr, ~(P0_b_7_TAP0_ISEL|P0_b_7_LEQ_ISEL|P0_b_6_LEQ_BIT_RATE|P0_b_6_TAP0_HBR|P0_b_5_LEQ_CURRENT_ADJ), (P0_b_7_TAP0_ISEL_400uA|P0_b_7_LEQ_ISEL_400uA|P0_b_6_LEQ_BIT_RATE_HBR|P0_b_6_LEQ_BIT_RATE_HBR|P0_b_6_TAP0_HBR|P0_b_5_LEQ_CURRENT_ADJ));		//tap0
			hdmi_mask(G2_addr, ~(P0_g_7_TAP0_ISEL|P0_g_7_LEQ_ISEL|P0_g_6_LEQ_BIT_RATE|P0_g_6_TAP0_HBR|P0_g_5_LEQ_CURRENT_ADJ), (P0_b_7_TAP0_ISEL_400uA|P0_b_7_LEQ_ISEL_400uA|P0_g_6_LEQ_BIT_RATE_HBR|P0_g_6_LEQ_BIT_RATE_HBR|P0_g_6_TAP0_HBR|P0_b_5_LEQ_CURRENT_ADJ));		//tap0
			hdmi_mask(R2_addr, ~(P0_r_7_TAP0_ISEL|P0_r_7_LEQ_ISEL|P0_r_6_LEQ_BIT_RATE|P0_r_6_TAP0_HBR|P0_r_5_LEQ_CURRENT_ADJ), (P0_b_7_TAP0_ISEL_400uA|P0_b_7_LEQ_ISEL_400uA|P0_r_6_LEQ_BIT_RATE_HBR|P0_r_6_LEQ_BIT_RATE_HBR|P0_r_6_TAP0_HBR|P0_b_5_LEQ_CURRENT_ADJ));		//tap0
		}
#else
		hdmi_mask(B2_addr, ~(P0_b_7_TAP0_ISEL|P0_b_7_LEQ_ISEL|P0_b_6_LEQ_BIT_RATE|P0_b_6_TAP0_HBR|P0_b_5_LEQ_CURRENT_ADJ), (P0_b_7_TAP0_ISEL_400uA|P0_b_7_LEQ_ISEL_400uA|P0_b_6_LEQ_BIT_RATE_HBR|P0_b_6_LEQ_BIT_RATE_HBR|P0_b_6_TAP0_HBR|P0_b_5_LEQ_CURRENT_ADJ));		//tap0
		hdmi_mask(G2_addr, ~(P0_g_7_TAP0_ISEL|P0_g_7_LEQ_ISEL|P0_g_6_LEQ_BIT_RATE|P0_g_6_TAP0_HBR|P0_g_5_LEQ_CURRENT_ADJ), (P0_b_7_TAP0_ISEL_400uA|P0_b_7_LEQ_ISEL_400uA|P0_g_6_LEQ_BIT_RATE_HBR|P0_g_6_LEQ_BIT_RATE_HBR|P0_g_6_TAP0_HBR|P0_b_5_LEQ_CURRENT_ADJ));		//tap0
		hdmi_mask(R2_addr, ~(P0_r_7_TAP0_ISEL|P0_r_7_LEQ_ISEL|P0_r_6_LEQ_BIT_RATE|P0_r_6_TAP0_HBR|P0_r_5_LEQ_CURRENT_ADJ), (P0_b_7_TAP0_ISEL_400uA|P0_b_7_LEQ_ISEL_400uA|P0_r_6_LEQ_BIT_RATE_HBR|P0_r_6_LEQ_BIT_RATE_HBR|P0_r_6_TAP0_HBR|P0_b_5_LEQ_CURRENT_ADJ));		//tap0
#endif

                //need check
                //B
                hdmi_mask(B3_addr,~(P0_b_11_ACDR_RATE_SEL_HALF_RATE|P0_b_10_DFE_SUMAMP_ISEL_mask|P0_b_10_DFE_PREAMP_ISEL_mask|P0_b_10_DFE_SUMAMP_LP_MANUALOFF),(P0_b_11_ACDR_RATE_SEL_HALF_RATE|P0_b_10_DFE_SUMAMP_ISEL_400uA|P0_b_10_DFE_SUMAMP_LP_MANUALOFF));
                //G
                hdmi_mask(G3_addr,~(P0_g_11_ACDR_RATE_SEL_HALF_RATE|P0_g_10_DFE_SUMAMP_ISEL_mask|P0_g_10_DFE_PREAMP_ISEL_mask|P0_g_10_DFE_SUMAMP_LP_MANUALOFF),(P0_g_11_ACDR_RATE_SEL_HALF_RATE|P0_g_10_DFE_SUMAMP_ISEL_400uA|P0_g_10_DFE_SUMAMP_LP_MANUALOFF));
                //R
                hdmi_mask(R3_addr,~(P0_r_11_ACDR_RATE_SEL_HALF_RATE|P0_r_10_DFE_SUMAMP_ISEL_mask|P0_r_10_DFE_PREAMP_ISEL_mask|P0_r_10_DFE_SUMAMP_LP_MANUALOFF),(P0_r_11_ACDR_RATE_SEL_HALF_RATE|P0_r_10_DFE_SUMAMP_ISEL_400uA|P0_r_10_DFE_SUMAMP_LP_MANUALOFF));

#if CLK_OVER_300M
	if (!newbase_hdmi2p0_get_clock40x_flag(bport)) {
		hdmi_mask(B1_addr, ~(P1_b_2_LEQ_BIT_RATE_mask), P1_b_2_LEQ_BIT_RATE_MBR);
		hdmi_mask(G1_addr, ~(P1_g_2_LEQ_BIT_RATE_mask),P1_g_2_LEQ_BIT_RATE_MBR);
		hdmi_mask(R1_addr, ~(P1_r_2_LEQ_BIT_RATE_mask),P1_r_2_LEQ_BIT_RATE_MBR);
	} else {
		hdmi_mask(B1_addr, ~(P1_b_2_LEQ_BIT_RATE_mask), P1_b_2_LEQ_BIT_RATE_HBR);
		hdmi_mask(G1_addr, ~(P1_g_2_LEQ_BIT_RATE_mask),P1_g_2_LEQ_BIT_RATE_HBR);
		hdmi_mask(R1_addr, ~(P1_r_2_LEQ_BIT_RATE_mask),P1_r_2_LEQ_BIT_RATE_HBR);
	}
#else
                //LEQ_BIT_RATE
                hdmi_mask(B1_addr, ~(P1_b_2_LEQ_BIT_RATE_mask), P1_b_2_LEQ_BIT_RATE_HBR);
                hdmi_mask(G1_addr, ~(P1_g_2_LEQ_BIT_RATE_mask),P1_g_2_LEQ_BIT_RATE_HBR);
                hdmi_mask(R1_addr, ~(P1_r_2_LEQ_BIT_RATE_mask),P1_r_2_LEQ_BIT_RATE_HBR);
#endif



                //PI_SEL  & pr bypass
// LCB6
//                hdmi_mask(B2_addr, ~(P1_b_8_BY_PASS_PR|P0_b_6_LEQ_PASSIVE_CORNER), 0);
//                hdmi_mask(G2_addr, ~(P1_g_8_BY_PASS_PR|P0_g_6_LEQ_PASSIVE_CORNER), 0);
//                hdmi_mask(R2_addr, ~(P1_r_8_BY_PASS_PR|P0_r_6_LEQ_PASSIVE_CORNER), 0);
		hdmi_mask(B2_addr, ~(P1_b_8_BY_PASS_PR|P0_b_6_LEQ_PASSIVE_CORNER), P0_b_6_LEQ_PASSIVE_CORNER);
		hdmi_mask(G2_addr, ~(P1_g_8_BY_PASS_PR|P0_g_6_LEQ_PASSIVE_CORNER), P0_b_6_LEQ_PASSIVE_CORNER);
		hdmi_mask(R2_addr, ~(P1_r_8_BY_PASS_PR|P0_r_6_LEQ_PASSIVE_CORNER), P0_b_6_LEQ_PASSIVE_CORNER);
        }


        else if (b > clock_bound_1p5g)
        { //3//   3G ~1.5G
                /*B lane */
                // tap0_ISEL   LEQ_ISEL   EQ_bit_rate  tap_HBR
                hdmi_mask(B2_addr,~(P0_b_7_TAP0_ISEL|P0_b_7_LEQ_ISEL|P0_b_6_LEQ_BIT_RATE|P0_b_6_TAP0_HBR|P0_b_5_LEQ_CURRENT_ADJ),(P0_b_7_TAP0_ISEL_400uA|P0_b_7_LEQ_ISEL_200uA|P0_b_6_LEQ_BIT_RATE_MBR|P0_b_6_TAP0_HBR|P0_b_5_LEQ_CURRENT_ADJ_1p25X));		//tap0

                /*G lane */
                // tap0_ISEL   LEQ_ISEL   EQ_bit_rate  tap_HBR
                hdmi_mask(G2_addr,~(P0_g_7_TAP0_ISEL|P0_g_7_LEQ_ISEL|P0_g_6_LEQ_BIT_RATE|P0_g_6_TAP0_HBR|P0_g_5_LEQ_CURRENT_ADJ),(P0_g_7_TAP0_ISEL_400uA|P0_g_7_LEQ_ISEL_200uA|P0_g_6_LEQ_BIT_RATE_MBR|P0_g_6_TAP0_HBR|P0_g_5_LEQ_CURRENT_ADJ_1p25X));		//tap0


                /*R lane */
                // tap0_ISEL   LEQ_ISEL   EQ_bit_rate  tap_HBR
                hdmi_mask(R2_addr,~(P0_r_7_TAP0_ISEL|P0_r_7_LEQ_ISEL|P0_r_6_LEQ_BIT_RATE|P0_r_6_TAP0_HBR|P0_r_5_LEQ_CURRENT_ADJ),(P0_r_7_TAP0_ISEL_400uA|P0_r_7_LEQ_ISEL_200uA|P0_r_6_LEQ_BIT_RATE_MBR|P0_r_6_TAP0_HBR|P0_r_5_LEQ_CURRENT_ADJ_1p25X));		//tap0

                //B
                hdmi_mask(B3_addr,~(P0_b_11_ACDR_RATE_SEL_HALF_RATE|P0_b_10_DFE_SUMAMP_ISEL_mask|P0_b_10_DFE_PREAMP_ISEL_mask|P0_b_10_DFE_SUMAMP_LP_MANUALOFF),(P0_b_11_ACDR_RATE_SEL_HALF_RATE|P0_b_10_DFE_SUMAMP_ISEL_400uA));
                //G
                hdmi_mask(G3_addr,~(P0_g_11_ACDR_RATE_SEL_HALF_RATE|P0_g_10_DFE_SUMAMP_ISEL_mask|P0_g_10_DFE_PREAMP_ISEL_mask|P0_g_10_DFE_SUMAMP_LP_MANUALOFF),(P0_g_11_ACDR_RATE_SEL_HALF_RATE|P0_g_10_DFE_SUMAMP_ISEL_400uA));
                //R
                hdmi_mask(R3_addr,~(P0_r_11_ACDR_RATE_SEL_HALF_RATE|P0_r_10_DFE_SUMAMP_ISEL_mask|P0_r_10_DFE_PREAMP_ISEL_mask|P0_r_10_DFE_SUMAMP_LP_MANUALOFF),(P0_r_11_ACDR_RATE_SEL_HALF_RATE|P0_r_10_DFE_SUMAMP_ISEL_400uA));

                //LEQ_BIT_RATE
                hdmi_mask(B1_addr, ~(P1_b_2_LEQ_BIT_RATE_mask), (P1_b_2_LEQ_BIT_RATE_MBR));
                hdmi_mask(G1_addr, ~(P1_g_2_LEQ_BIT_RATE_mask), (P1_g_2_LEQ_BIT_RATE_MBR));
                hdmi_mask(R1_addr, ~(P1_r_2_LEQ_BIT_RATE_mask), (P1_r_2_LEQ_BIT_RATE_MBR));

                //PI_SEL  & pr bypass
                hdmi_mask(B2_addr,~(P1_b_8_BY_PASS_PR|P0_b_6_LEQ_PASSIVE_CORNER),0);
                hdmi_mask(G2_addr,~(P1_g_8_BY_PASS_PR|P0_g_6_LEQ_PASSIVE_CORNER),0);
                hdmi_mask(R2_addr,~(P1_r_8_BY_PASS_PR|P0_r_6_LEQ_PASSIVE_CORNER),0);

        }
	else 
	{ // under 1.5G
 		if (b < clock_bound_45m)
		{
	                /*B lane */
	                // tap0_ISEL   LEQ_ISEL   EQ_bit_rate  tap_HBR
	                hdmi_mask(B2_addr,~(P0_b_7_TAP0_ISEL|P0_b_7_LEQ_ISEL|P0_b_6_LEQ_BIT_RATE|P0_b_6_TAP0_HBR|P0_b_5_LEQ_CURRENT_ADJ),(P0_b_7_TAP0_ISEL_400uA|P0_b_7_LEQ_ISEL_200uA|P0_b_6_LEQ_BIT_RATE_MBR|P0_b_6_TAP0_HBR|P0_b_5_LEQ_CURRENT_ADJ_1p25X));		//tap0

	                /*G lane */
	                // tap0_ISEL   LEQ_ISEL   EQ_bit_rate  tap_HBR
	                hdmi_mask(G2_addr,~(P0_g_7_TAP0_ISEL|P0_g_7_LEQ_ISEL|P0_g_6_LEQ_BIT_RATE|P0_g_6_TAP0_HBR|P0_g_5_LEQ_CURRENT_ADJ),(P0_g_7_TAP0_ISEL_400uA|P0_g_7_LEQ_ISEL_200uA|P0_g_6_LEQ_BIT_RATE_MBR|P0_g_6_TAP0_HBR|P0_g_5_LEQ_CURRENT_ADJ_1p25X));		//tap0


	                /*R lane */
	                // tap0_ISEL   LEQ_ISEL   EQ_bit_rate  tap_HBR
	                hdmi_mask(R2_addr,~(P0_r_7_TAP0_ISEL|P0_r_7_LEQ_ISEL|P0_r_6_LEQ_BIT_RATE|P0_r_6_TAP0_HBR|P0_r_5_LEQ_CURRENT_ADJ),(P0_r_7_TAP0_ISEL_400uA|P0_r_7_LEQ_ISEL_200uA|P0_r_6_LEQ_BIT_RATE_MBR|P0_r_6_TAP0_HBR|P0_r_5_LEQ_CURRENT_ADJ_1p25X));		//tap0

	                //
	                //B
	                hdmi_mask(B3_addr,~(P0_b_11_ACDR_RATE_SEL_HALF_RATE|P0_b_10_DFE_SUMAMP_ISEL_mask|P0_b_10_DFE_PREAMP_ISEL_mask|P0_b_10_DFE_SUMAMP_LP_MANUALOFF),(P0_b_11_ACDR_RATE_SEL_HALF_RATE|P0_b_10_DFE_SUMAMP_ISEL_400uA));
	                //G
	                hdmi_mask(G3_addr,~(P0_g_11_ACDR_RATE_SEL_HALF_RATE|P0_g_10_DFE_SUMAMP_ISEL_mask|P0_g_10_DFE_PREAMP_ISEL_mask|P0_g_10_DFE_SUMAMP_LP_MANUALOFF),(P0_g_11_ACDR_RATE_SEL_HALF_RATE|P0_g_10_DFE_SUMAMP_ISEL_400uA));
	                //R
	                hdmi_mask(R3_addr,~(P0_r_11_ACDR_RATE_SEL_HALF_RATE|P0_r_10_DFE_SUMAMP_ISEL_mask|P0_r_10_DFE_PREAMP_ISEL_mask|P0_r_10_DFE_SUMAMP_LP_MANUALOFF),(P0_r_11_ACDR_RATE_SEL_HALF_RATE|P0_r_10_DFE_SUMAMP_ISEL_400uA));

	                //LEQ_BIT_RATE
	                hdmi_mask(B1_addr, ~(P1_b_2_LEQ_BIT_RATE_mask), (P1_b_2_LEQ_BIT_RATE_MBR));
	                hdmi_mask(G1_addr, ~(P1_g_2_LEQ_BIT_RATE_mask), (P1_g_2_LEQ_BIT_RATE_MBR));
	                hdmi_mask(R1_addr, ~(P1_r_2_LEQ_BIT_RATE_mask), (P1_r_2_LEQ_BIT_RATE_MBR));

	                //PI_SEL  & pr bypass
	                hdmi_mask(B2_addr,~(P1_b_8_BY_PASS_PR|P0_b_6_LEQ_PASSIVE_CORNER),0);
	                hdmi_mask(G2_addr,~(P1_g_8_BY_PASS_PR|P0_g_6_LEQ_PASSIVE_CORNER),0);
	                hdmi_mask(R2_addr,~(P1_r_8_BY_PASS_PR|P0_r_6_LEQ_PASSIVE_CORNER),0);
		}
		else
		{
			 //430~1.5G 
			if(factory_or_power_only_mode_en==0)
			{
	                /*B lane */
	                // tap0_ISEL   LEQ_ISEL   EQ_bit_rate  tap_HBR
	                hdmi_mask(B2_addr,~(P0_b_7_TAP0_ISEL|P0_b_7_LEQ_ISEL|P0_b_6_LEQ_BIT_RATE|P0_b_6_TAP0_HBR|P0_b_5_LEQ_CURRENT_ADJ),(P0_b_7_TAP0_ISEL_400uA|P0_b_7_LEQ_ISEL_200uA|P0_b_6_LEQ_BIT_RATE_MBR|P0_b_6_TAP0_HBR|P0_b_5_LEQ_CURRENT_ADJ_1p25X));		//tap0

	                /*G lane */
	                // tap0_ISEL   LEQ_ISEL   EQ_bit_rate  tap_HBR
	                hdmi_mask(G2_addr,~(P0_g_7_TAP0_ISEL|P0_g_7_LEQ_ISEL|P0_g_6_LEQ_BIT_RATE|P0_g_6_TAP0_HBR|P0_g_5_LEQ_CURRENT_ADJ),(P0_g_7_TAP0_ISEL_400uA|P0_g_7_LEQ_ISEL_200uA|P0_g_6_LEQ_BIT_RATE_MBR|P0_g_6_TAP0_HBR|P0_g_5_LEQ_CURRENT_ADJ_1p25X));		//tap0


	                /*R lane */
	                // tap0_ISEL   LEQ_ISEL   EQ_bit_rate  tap_HBR
	                hdmi_mask(R2_addr,~(P0_r_7_TAP0_ISEL|P0_r_7_LEQ_ISEL|P0_r_6_LEQ_BIT_RATE|P0_r_6_TAP0_HBR|P0_r_5_LEQ_CURRENT_ADJ),(P0_r_7_TAP0_ISEL_400uA|P0_r_7_LEQ_ISEL_200uA|P0_r_6_LEQ_BIT_RATE_MBR|P0_r_6_TAP0_HBR|P0_r_5_LEQ_CURRENT_ADJ_1p25X));		//tap0

	                //B
	                hdmi_mask(B3_addr,~(P0_b_11_ACDR_RATE_SEL_HALF_RATE|P0_b_10_DFE_SUMAMP_ISEL_mask|P0_b_10_DFE_PREAMP_ISEL_mask|P0_b_10_DFE_SUMAMP_LP_MANUALOFF),(P0_b_11_ACDR_RATE_SEL_HALF_RATE|P0_b_10_DFE_SUMAMP_ISEL_400uA));
	                //G
	                hdmi_mask(G3_addr,~(P0_g_11_ACDR_RATE_SEL_HALF_RATE|P0_g_10_DFE_SUMAMP_ISEL_mask|P0_g_10_DFE_PREAMP_ISEL_mask|P0_g_10_DFE_SUMAMP_LP_MANUALOFF),(P0_g_11_ACDR_RATE_SEL_HALF_RATE|P0_g_10_DFE_SUMAMP_ISEL_400uA));
	                //R
	                hdmi_mask(R3_addr,~(P0_r_11_ACDR_RATE_SEL_HALF_RATE|P0_r_10_DFE_SUMAMP_ISEL_mask|P0_r_10_DFE_PREAMP_ISEL_mask|P0_r_10_DFE_SUMAMP_LP_MANUALOFF),(P0_r_11_ACDR_RATE_SEL_HALF_RATE|P0_r_10_DFE_SUMAMP_ISEL_400uA));

	                //LEQ_BIT_RATE
	                hdmi_mask(B1_addr, ~(P1_b_2_LEQ_BIT_RATE_mask), (P1_b_2_LEQ_BIT_RATE_MBR));
	                hdmi_mask(G1_addr, ~(P1_g_2_LEQ_BIT_RATE_mask), (P1_g_2_LEQ_BIT_RATE_MBR));
	                hdmi_mask(R1_addr, ~(P1_r_2_LEQ_BIT_RATE_mask), (P1_r_2_LEQ_BIT_RATE_MBR));

	                //PI_SEL  & pr bypass
	                hdmi_mask(B2_addr,~(P1_b_8_BY_PASS_PR|P0_b_6_LEQ_PASSIVE_CORNER),0);
	                hdmi_mask(G2_addr,~(P1_g_8_BY_PASS_PR|P0_g_6_LEQ_PASSIVE_CORNER),0);
	                hdmi_mask(R2_addr,~(P1_r_8_BY_PASS_PR|P0_r_6_LEQ_PASSIVE_CORNER),0);
		}
		else
		{
			//Ori for LGE-4040H 720P Signal Unstable Issue
	                /*B lane */
	                // tap0_ISEL   LEQ_ISEL   EQ_bit_rate  tap_HBR
	                hdmi_mask(B2_addr,~(P0_b_7_TAP0_ISEL|P0_b_7_LEQ_ISEL|P0_b_6_LEQ_BIT_RATE|P0_b_6_TAP0_HBR|P0_b_5_LEQ_CURRENT_ADJ),(P0_b_5_LEQ_CURRENT_ADJ_1p25X|P0_b_7_TAP0_ISEL_400uA));		//tap0

	                /*G lane */
	                // tap0_ISEL   LEQ_ISEL   EQ_bit_rate  tap_HBR
	                hdmi_mask(G2_addr,~(P0_g_7_TAP0_ISEL|P0_g_7_LEQ_ISEL|P0_g_6_LEQ_BIT_RATE|P0_g_6_TAP0_HBR|P0_g_5_LEQ_CURRENT_ADJ),(P0_g_5_LEQ_CURRENT_ADJ_1p25X|P0_g_7_TAP0_ISEL_400uA));		//tap0

	                /*R lane */
	                // tap0_ISEL   LEQ_ISEL   EQ_bit_rate  tap_HBR
	                hdmi_mask(R2_addr,~(P0_r_7_TAP0_ISEL|P0_r_7_LEQ_ISEL|P0_r_6_LEQ_BIT_RATE|P0_r_6_TAP0_HBR|P0_r_5_LEQ_CURRENT_ADJ),(P0_r_5_LEQ_CURRENT_ADJ_1p25X|P0_r_7_TAP0_ISEL_400uA));		//tap0

	                //B
	                hdmi_mask(B3_addr,~(P0_b_11_ACDR_RATE_SEL_HALF_RATE|P0_b_10_DFE_SUMAMP_ISEL_mask|P0_b_10_DFE_PREAMP_ISEL_mask|P0_b_10_DFE_SUMAMP_LP_MANUALOFF),(P0_b_11_ACDR_RATE_SEL_HALF_RATE|P0_b_10_DFE_SUMAMP_ISEL_400uA));
	                //G
	                hdmi_mask(G3_addr,~(P0_g_11_ACDR_RATE_SEL_HALF_RATE|P0_g_10_DFE_SUMAMP_ISEL_mask|P0_g_10_DFE_PREAMP_ISEL_mask|P0_g_10_DFE_SUMAMP_LP_MANUALOFF),(P0_g_11_ACDR_RATE_SEL_HALF_RATE|P0_g_10_DFE_SUMAMP_ISEL_400uA));
	                //R
	                hdmi_mask(R3_addr,~(P0_r_11_ACDR_RATE_SEL_HALF_RATE|P0_r_10_DFE_SUMAMP_ISEL_mask|P0_r_10_DFE_PREAMP_ISEL_mask|P0_r_10_DFE_SUMAMP_LP_MANUALOFF),(P0_r_11_ACDR_RATE_SEL_HALF_RATE|P0_r_10_DFE_SUMAMP_ISEL_400uA));

	                //LEQ_BIT_RATE
	                /*
			hdmi_mask(B1_addr,~(P1_b_2_LEQ_BIT_RATE_mask),P1_b_2_LEQ_BIT_RATE_MBR);
			hdmi_mask(G1_addr,~(P1_g_2_LEQ_BIT_RATE_mask),P1_g_2_LEQ_BIT_RATE_MBR);
			hdmi_mask(R1_addr,~(P1_r_2_LEQ_BIT_RATE_mask),P1_r_2_LEQ_BIT_RATE_MBR);
	                 */
	                //hdmi_mask(B1_addr,~(P1_b_2_LEQ_BIT_RATE_mask),P1_b_2_LEQ_BIT_RATE_MBR);
	                
	                hdmi_mask(B1_addr, ~(P1_b_2_LEQ_BIT_RATE_mask), 0x0);
	                hdmi_mask(G1_addr, ~(P1_g_2_LEQ_BIT_RATE_mask), 0x0);
	                hdmi_mask(R1_addr, ~(P1_r_2_LEQ_BIT_RATE_mask), 0x0);
					
	                //PI_SEL  & pr bypass
	                hdmi_mask(B2_addr,~(P1_b_8_BY_PASS_PR|P0_b_6_LEQ_PASSIVE_CORNER), 0);
	                hdmi_mask(G2_addr,~(P1_g_8_BY_PASS_PR|P0_g_6_LEQ_PASSIVE_CORNER), 0);
	                hdmi_mask(R2_addr,~(P1_r_8_BY_PASS_PR|P0_r_6_LEQ_PASSIVE_CORNER), 0);
			}
		}
	}
 #endif 
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
unsigned char lib_hdmi_dfe_tap0_fix(unsigned char nport,unsigned char lane_mode)
{
#if 0
	unsigned char i, tap0max[4], tap0[4];
	unsigned char tap0_max_of_all, tap0_min_of_all;
	unsigned char bgrc;

	//read Tap0 max
	for (i=0; i<lane_mode; i++) {
		tap0max[i] = lib_hdmi_dfe_get_tap0max(nport, i);
		tap0[i] = lib_hdmi_dfe_get_tap0(nport, i);
	}


	if (lane_mode == HDMI_4LANE) {
		bgrc = LN_ALL;
		if ((tap0max[0] > 2) && (tap0max[1] > 2) && (tap0max[2] > 2) && (tap0max[3] > 2)) {
			return FALSE;
		}
		tap0_max_of_all = MAX(tap0[0],MAX(tap0[1], MAX(tap0[2], tap0[3])));
		tap0_min_of_all = MIN(tap0[0],MIN(tap0[1], MIN(tap0[2], tap0[3])));

		if ((tap0_max_of_all - tap0_min_of_all) < 8) {
			return FALSE;
		}
	} else {
		bgrc = (LN_R|LN_G|LN_B);
		if ((tap0max[0] > 2) && (tap0max[1] > 2) && (tap0max[2] > 2)) {
			return FALSE;
		}
		tap0_max_of_all = MAX(tap0[0], MAX(tap0[1], tap0[2]));
		tap0_min_of_all = MIN(tap0[0], MIN(tap0[1], tap0[2]));

		if ((tap0_max_of_all - tap0_min_of_all) < 8) {
			return FALSE;
		}
	}

	HDMI_EMG("%s\n", __func__);
	
	lib_hdmi_dfe_init_vth(nport, bgrc, 0x05);
	lib_hdmi_dfe_init_tap0(nport, bgrc, 0x14);

#if 1
//#ifdef PLAN_B
	udelay(100);
	hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_Vth), (REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_Vth));
	hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_Vth), (REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_Vth));
	hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_Vth), (REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_Vth));
	if (lane_mode == HDMI_4LANE) 
		hdmi_mask(DFE_REG_DFE_EN_L3_reg,~(REG_dfe_adapt_en_lane3_TAP0|REG_dfe_adapt_en_lane3_Vth), (REG_dfe_adapt_en_lane3_TAP0|REG_dfe_adapt_en_lane3_Vth));

	udelay(500);
	hdmi_mask(DFE_REG_DFE_READBACK_reg,~DFE_P0_REG_DFE_READBACK_P0_record_limit_en_p0_mask,DFE_P0_REG_DFE_READBACK_P0_record_limit_en_p0(1));
	udelay(500);
	hdmi_mask(DFE_REG_DFE_READBACK_reg,~DFE_P0_REG_DFE_READBACK_P0_record_limit_en_p0_mask,DFE_P0_REG_DFE_READBACK_P0_record_limit_en_p0(0));
	hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_Vth),0);
	hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_Vth),0);
	hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_Vth),0);
	if (lane_mode == HDMI_4LANE) 
		hdmi_mask(DFE_REG_DFE_EN_L3_reg,~(REG_dfe_adapt_en_lane3_TAP0|REG_dfe_adapt_en_lane3_Vth),0);


#endif

#endif
	return TRUE;
}
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


void lib_hdmi_tap0_fix_small_swing(unsigned char nport, unsigned char lane_mode)
{
#if 0
	unsigned char bgrc;
	HDMI_EMG("%s\n", __func__);
	if (lane_mode == HDMI_4LANE)
		bgrc = LN_ALL;
	else
		bgrc = (LN_R|LN_G|LN_B);
	
	lib_hdmi_dfe_init_vth(nport, bgrc, 0x05);
	lib_hdmi_dfe_init_tap0(nport, bgrc, 0x14);
	udelay(100);
	hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_Vth), (REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_Vth));
	hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_Vth), (REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_Vth));
	hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_Vth), (REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_Vth));
	if (lane_mode == HDMI_4LANE) 
		hdmi_mask(DFE_REG_DFE_EN_L3_reg,~(REG_dfe_adapt_en_lane3_TAP0|REG_dfe_adapt_en_lane3_Vth), (REG_dfe_adapt_en_lane3_TAP0|REG_dfe_adapt_en_lane3_Vth));
	udelay(500);
	hdmi_mask(DFE_REG_DFE_READBACK_reg,~DFE_P0_REG_DFE_READBACK_P0_record_limit_en_p0_mask,DFE_P0_REG_DFE_READBACK_P0_record_limit_en_p0(1));
	udelay(500);
	hdmi_mask(DFE_REG_DFE_READBACK_reg,~DFE_P0_REG_DFE_READBACK_P0_record_limit_en_p0_mask,DFE_P0_REG_DFE_READBACK_P0_record_limit_en_p0(0));
	hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_Vth),0);
	hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_Vth),0);
	hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_Vth),0);
	if (lane_mode == HDMI_4LANE) 
		hdmi_mask(DFE_REG_DFE_EN_L3_reg,~(REG_dfe_adapt_en_lane3_TAP0|REG_dfe_adapt_en_lane3_Vth),0);

#endif
}


void lib_hdmi_dfe_second_flow(unsigned char nport, unsigned int b_clk, unsigned char lane_mode)
{
#if 0
	unsigned char i, tap0max[4];

	udelay(100);
	hdmi_mask(DFE_REG_DFE_READBACK_reg,~DFE_P0_REG_DFE_READBACK_P0_record_limit_en_p0_mask,DFE_P0_REG_DFE_READBACK_P0_record_limit_en_p0(1));
	udelay(500);
	hdmi_mask(DFE_REG_DFE_READBACK_reg,~DFE_P0_REG_DFE_READBACK_P0_record_limit_en_p0_mask,DFE_P0_REG_DFE_READBACK_P0_record_limit_en_p0(0));
	hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_Vth),0);
	hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_Vth),0);
	hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_Vth),0);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(DFE_REG_DFE_EN_L3_reg,~(REG_dfe_adapt_en_lane3_TAP0|REG_dfe_adapt_en_lane3_Vth),0);


//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	if (b_clk > 5000) {
		lib_hdmi_dfe_tap0_fix(nport,lane_mode);
	}
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	//read Tap0 max
	for (i=0; i<lane_mode; i++) {
		tap0max[i] = lib_hdmi_dfe_get_tap0max(nport, i);
		tap0max[i] = (tap0max[i]>2)?(tap0max[i]-2):0;
	}

	// LOAD TAP0 max -2  & Vth 5
	hdmi_mask(DFE_REG_DFE_INIT0_L0_reg,~(DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0_mask),(DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0(tap0max[0]))|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0(0x5));
	hdmi_mask(DFE_REG_DFE_INIT0_L1_reg,~(DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0_mask),(DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0(tap0max[1]))|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0(0x5));
	hdmi_mask(DFE_REG_DFE_INIT0_L2_reg,~(DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0_mask),(DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0(tap0max[2]))|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0(0x5));
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(DFE_REG_DFE_INIT0_L3_reg,~(DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0_mask),(DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0(tap0max[3]))|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0(0x5));


	//load initial data
	hdmi_mask(DFE_REG_DFE_INIT1_L0_reg,~(LOAD_IN_INIT_TAP0|LOAD_IN_INIT_VTH),(LOAD_IN_INIT_TAP0|LOAD_IN_INIT_VTH));
	hdmi_mask(DFE_REG_DFE_INIT1_L1_reg,~(LOAD_IN_INIT_TAP0|LOAD_IN_INIT_VTH),(LOAD_IN_INIT_TAP0|LOAD_IN_INIT_VTH));
	hdmi_mask(DFE_REG_DFE_INIT1_L2_reg,~(LOAD_IN_INIT_TAP0|LOAD_IN_INIT_VTH),(LOAD_IN_INIT_TAP0|LOAD_IN_INIT_VTH));
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(DFE_REG_DFE_INIT1_L3_reg,~(LOAD_IN_INIT_TAP0|LOAD_IN_INIT_VTH),(LOAD_IN_INIT_TAP0|LOAD_IN_INIT_VTH));
	udelay(1);

	hdmi_mask(DFE_REG_DFE_INIT1_L0_reg,~(LOAD_IN_INIT_TAP0|LOAD_IN_INIT_VTH),0);
	hdmi_mask(DFE_REG_DFE_INIT1_L1_reg,~(LOAD_IN_INIT_TAP0|LOAD_IN_INIT_VTH),0);
	hdmi_mask(DFE_REG_DFE_INIT1_L2_reg,~(LOAD_IN_INIT_TAP0|LOAD_IN_INIT_VTH),0);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(DFE_REG_DFE_INIT1_L3_reg,~(LOAD_IN_INIT_TAP0|LOAD_IN_INIT_VTH),0);
#endif	

}



void lib_hdmi_cmu_pll_en(unsigned char nport, unsigned char enable)
{  
#if 0
	unsigned int bit_mask;
	unsigned int reg_ck1;
	
	if (nport == HDMI_PORT2) {
		reg_ck1 = HDMIRX_2P0_PHY_P0_CK1_reg;
		bit_mask = HDMIRX_2P0_PHY_ENABLE_reg_p0_en_cmu_mask;
	} else if (nport == HDMI_PORT3) {
		reg_ck1 = HDMIRX_2P0_PHY_P1_CK1_reg;
		bit_mask = HDMIRX_2P0_PHY_ENABLE_reg_p1_en_cmu_mask;
	} else {
		return;
	}

	hdmi_mask(reg_ck1,~(p0_ck_2_CMU_CKIN_SEL),p0_ck_2_CMU_CKIN_SEL);  //tmds clk
	hdmi_mask(HDMIRX_2P0_PHY_ENABLE_reg, ~bit_mask, enable?bit_mask:0);
#endif	
}


void lib_hdmi_demux_ck_vcopll_rst(unsigned char nport, unsigned char rst)
{
 #if 0  
// rst = 1: reset
// rst = 0: normal working operation
	
	unsigned int rgb_demux;
	
	if (nport == HDMI_PORT2) {
		rgb_demux = HDMIRX_2P0_PHY_RST_p0_r_demux_rstb_mask|HDMIRX_2P0_PHY_RST_p0_g_demux_rstb_mask|HDMIRX_2P0_PHY_RST_p0_b_demux_rstb_mask;
	} else if (nport == HDMI_PORT3) {
		rgb_demux = HDMIRX_2P0_PHY_RST_p1_r_demux_rstb_mask|HDMIRX_2P0_PHY_RST_p1_g_demux_rstb_mask|HDMIRX_2P0_PHY_RST_p1_b_demux_rstb_mask;
	} else {
		return;
	}
	
	if (rst) {
		lib_hdmi_dfe_rst(nport, 1);
		hdmi_mask(HDMIRX_2P0_PHY_RST_reg, ~rgb_demux, 0);		
	} else {
		hdmi_mask(HDMIRX_2P0_PHY_RST_reg, ~rgb_demux, rgb_demux);		
		lib_hdmi_dfe_rst(nport, 0);
	}
#endif	
}



void lib_hdmi_cdr_rst(unsigned char nport, unsigned char rst)
{
#if 0
// rst = 1: reset
// rst = 0: normal working operation
	unsigned int rst_reg,dig_bit,cdr_bit;

	if (nport == HDMI_PORT2) {
		 rst_reg = HDMIRX_2P0_PHY_REGD0_reg;
		 dig_bit = HDMIRX_2P0_PHY_REGD0_p0_r_dig_rst_n_mask|HDMIRX_2P0_PHY_REGD0_p0_g_dig_rst_n_mask|HDMIRX_2P0_PHY_REGD0_p0_b_dig_rst_n_mask;
		 cdr_bit = HDMIRX_2P0_PHY_REGD0_p0_r_cdr_rst_n_mask|HDMIRX_2P0_PHY_REGD0_p0_g_cdr_rst_n_mask|HDMIRX_2P0_PHY_REGD0_p0_b_cdr_rst_n_mask;
	} else if (nport == HDMI_PORT3) {
		 rst_reg = HDMIRX_2P0_PHY_REGD5_reg;
		 dig_bit = HDMIRX_2P0_PHY_REGD5_p1_r_dig_rst_n_mask|HDMIRX_2P0_PHY_REGD5_p1_g_dig_rst_n_mask|HDMIRX_2P0_PHY_REGD5_p1_b_dig_rst_n_mask;
		 cdr_bit =HDMIRX_2P0_PHY_REGD5_p1_r_cdr_rst_n_mask|HDMIRX_2P0_PHY_REGD5_p1_g_cdr_rst_n_mask|HDMIRX_2P0_PHY_REGD5_p1_b_cdr_rst_n_mask;
	} else {
		return;
	}
	

	if (rst) {
		hdmi_mask(rst_reg, ~dig_bit, 0);
		hdmi_mask(rst_reg, ~cdr_bit, 0);

	} else {
		hdmi_mask(rst_reg, ~(dig_bit|cdr_bit), (dig_bit|cdr_bit));
	}
#endif
}

void lib_hdmi_wdog(unsigned char nport)
{
#if 0
	unsigned int reg_cmu0;
	unsigned int reg_cmu1;
	
	if (nport == HDMI_PORT2) {
		 reg_cmu0 = HDMIRX_2P0_PHY_REGD20_reg;
		 reg_cmu1 = HDMIRX_2P0_PHY_REGD21_reg;
	} else if (nport == HDMI_PORT3) {
		reg_cmu0 = HDMIRX_2P0_PHY_REGD22_reg;
		 reg_cmu1 = HDMIRX_2P0_PHY_REGD23_reg;
	} else {
		return;
	}

	hdmi_mask(reg_cmu0, ~(HDMIRX_2P0_PHY_REGD20_reg_p0_bypass_eqen_rdy_mask|HDMIRX_2P0_PHY_REGD20_reg_p0_bypass_data_rdy_mask|HDMIRX_2P0_PHY_REGD20_reg_p0_bypass_pi_mask|
		                                               HDMIRX_2P0_PHY_REGD20_reg_p0_data_rdy_time_mask|HDMIRX_2P0_PHY_REGD20_reg_p0_en_clkout_manual_mask|HDMIRX_2P0_PHY_REGD20_reg_p0_en_eqen_manual_mask|
		                                                HDMIRX_2P0_PHY_REGD20_reg_p0_en_data_manual_mask|HDMIRX_2P0_PHY_REGD20_reg_p0_eqen_rdy_time_mask|HDMIRX_2P0_PHY_REGD20_reg_p0_pll_wd_base_time_mask|
		                                                HDMIRX_2P0_PHY_REGD20_reg_p0_pll_wd_rst_wid_mask|HDMIRX_2P0_PHY_REGD20_reg_p0_pll_wd_time_rdy_mask|HDMIRX_2P0_PHY_REGD20_p0_pll_wd_ckrdy_ro_mask|
		                                                HDMIRX_2P0_PHY_REGD20_p0_wdog_rst_n_mask),
		                                                	HDMIRX_2P0_PHY_REGD20_reg_p0_bypass_clk_rdy(1)| HDMIRX_2P0_PHY_REGD20_reg_p0_bypass_eqen_rdy(1)|HDMIRX_2P0_PHY_REGD20_reg_p0_bypass_data_rdy(1)|
		                                                 HDMIRX_2P0_PHY_REGD20_reg_p0_data_rdy_time(4)| HDMIRX_2P0_PHY_REGD20_reg_p0_en_clkout_manual(1)|HDMIRX_2P0_PHY_REGD20_reg_p0_en_eqen_manual(1)|
									HDMIRX_2P0_PHY_REGD20_reg_p0_en_data_manual(1)|HDMIRX_2P0_PHY_REGD20_reg_p0_eqen_rdy_time(4)|HDMIRX_2P0_PHY_REGD20_reg_p0_pll_wd_base_time(2)|
									HDMIRX_2P0_PHY_REGD20_reg_p0_pll_wd_rst_wid(1)|HDMIRX_2P0_PHY_REGD20_reg_p0_pll_wd_time_rdy(1) |HDMIRX_2P0_PHY_REGD20_p0_pll_wd_ckrdy_ro(1)|
									HDMIRX_2P0_PHY_REGD20_p0_wdog_rst_n(1));


	hdmi_mask(reg_cmu1, ~(HDMIRX_2P0_PHY_REGD21_reg_p0_wd_sdm_en_mask),0);
	udelay(5);
	hdmi_mask(reg_cmu1, ~(HDMIRX_2P0_PHY_REGD21_reg_p0_wd_sdm_en_mask), HDMIRX_2P0_PHY_REGD21_reg_p0_wd_sdm_en_mask);	//adams modify, 20160130	//Load CMU M code
#endif
}

void lib_hdmi_dfe_config_setting(unsigned char nport,unsigned char lane_mode)
{
        lib_hdmi_mac_afifo_enable(nport,lane_mode);
}



//clock lane
void lib_hdmi_set_phy_table(unsigned char bport, HDMI_PHY_PARAM_T *phy_param)
{
 #if 0
	unsigned int wReg1, wReg2, wCk1Reg, wCk2Reg, ck_md_sel;
	unsigned int B1REG, G1REG, R1REG;
	unsigned int B2REG, G2REG, R2REG;
	unsigned int B3REG, G3REG, R3REG;

	if (bport == HDMI_PORT2) {
		wReg1 = HDMIRX_2P0_PHY_REGD21_reg;
		wReg2 = HDMIRX_2P0_PHY_REGD0_reg;
		wCk1Reg =  HDMIRX_2P0_PHY_P0_CK1_reg;
		B1REG = HDMIRX_2P0_PHY_P0_B1_reg;
		G1REG = HDMIRX_2P0_PHY_P0_G1_reg;
		R1REG = HDMIRX_2P0_PHY_P0_R1_reg;
		B2REG = HDMIRX_2P0_PHY_P0_B2_reg;
		G2REG = HDMIRX_2P0_PHY_P0_G2_reg;
		R2REG = HDMIRX_2P0_PHY_P0_R2_reg;
		B3REG = HDMIRX_2P0_PHY_P0_B3_reg;
		G3REG = HDMIRX_2P0_PHY_P0_G3_reg;
		R3REG = HDMIRX_2P0_PHY_P0_R3_reg;
		wCk2Reg = HDMIRX_2P0_PHY_P0_CK2_reg;
		ck_md_sel = HDMIRX_2P0_PHY_REGD56_reg;
	} else if (bport == HDMI_PORT3) {
		wReg1 = HDMIRX_2P0_PHY_REGD23_reg;
		wReg2 = HDMIRX_2P0_PHY_REGD5_reg;
		wCk1Reg =  HDMIRX_2P0_PHY_P1_CK1_reg;
		B1REG = HDMIRX_2P0_PHY_P1_B1_reg;
		G1REG = HDMIRX_2P0_PHY_P1_G1_reg;
		R1REG = HDMIRX_2P0_PHY_P1_R1_reg;
		B2REG = HDMIRX_2P0_PHY_P1_B2_reg;
		G2REG = HDMIRX_2P0_PHY_P1_G2_reg;
		R2REG = HDMIRX_2P0_PHY_P1_R2_reg;
		B3REG = HDMIRX_2P0_PHY_P1_B3_reg;
		G3REG = HDMIRX_2P0_PHY_P1_G3_reg;
		R3REG = HDMIRX_2P0_PHY_P1_R3_reg;
		wCk2Reg = HDMIRX_2P0_PHY_P1_CK2_reg;
		ck_md_sel = HDMIRX_2P0_PHY_REGD58_reg;

	} else {
		return;
	}

	//legacy mode

	 hdmi_mask(B1REG, ~(PIX2_RATE_SEL), phy_param->PIX2_SEL?PIX2_RATE_DIV5: PIX2_RATE_DIV10);
	 hdmi_mask(G1REG, ~(PIX2_RATE_SEL),phy_param->PIX2_SEL?PIX2_RATE_DIV5: PIX2_RATE_DIV10);
	 hdmi_mask(R1REG, ~(PIX2_RATE_SEL),phy_param->PIX2_SEL?PIX2_RATE_DIV5: PIX2_RATE_DIV10);
	
	 hdmi_mask(B3REG,~(P0_b_12_PI_CSEL|P0_b_12_PI_CURRENT), (phy_param->PI_CURRENT)<<28 |(phy_param->PI_CSEL << 24));
        hdmi_mask(G3REG,~(P0_g_12_PI_CSEL|P0_g_12_PI_CURRENT), (phy_param->PI_CURRENT)<<28 |(phy_param->PI_CSEL << 24));
        hdmi_mask(R3REG,~(P0_r_12_PI_CSEL|P0_r_12_PI_CURRENT),  (phy_param->PI_CURRENT)<<28 |(phy_param->PI_CSEL << 24));

	hdmi_mask(wReg1, ~HDMIRX_2P0_PHY_REGD21_reg_p0_n_code_mask, (phy_param->M_code)<<HDMIRX_2P0_PHY_REGD21_reg_p0_n_code_shift);		//M code set

	//N code
	hdmi_mask(wCk1Reg, ~(p0_ck_3_CMU_N_code|p0_ck_3_CMU_PREDIVN), (phy_param->N_code)<<18);		//N code set
	hdmi_mask(wCk1Reg, ~(p0_ck_3_CMU_SEL_D4), (phy_param->SEL_D4)<<16);


	//PI_DIV
	hdmi_mask(B2REG, ~P0_b_8_PR_RATE_SEL, (phy_param->PI_DIV)<<26);		//PI_DIV code set
	hdmi_mask(G2REG, ~P0_b_8_PR_RATE_SEL, (phy_param->PI_DIV)<<26);		//PI_DIV code set
	hdmi_mask(R2REG, ~P0_b_8_PR_RATE_SEL, (phy_param->PI_DIV)<<26);		//PI_DIV code set
	//DEMUX
	hdmi_mask(B2REG, ~(P0_b_8_DEMUX|P0_b_7_PI_ISEL), ((phy_param->DEMUX)<<29)|(((phy_param->PI_ISEL & 0x4)>>2 )<<24)|((phy_param->PI_ISEL & 0x3 )<<16));
	hdmi_mask(G2REG, ~(P0_b_8_DEMUX|P0_b_7_PI_ISEL), ((phy_param->DEMUX)<<29)|(((phy_param->PI_ISEL & 0x4)>>2 )<<24)|((phy_param->PI_ISEL & 0x3 )<<16));		//DEMUX code set
	hdmi_mask(R2REG, ~(P0_b_8_DEMUX|P0_b_7_PI_ISEL), ((phy_param->DEMUX)<<29)|(((phy_param->PI_ISEL & 0x4)>>2 )<<24)|((phy_param->PI_ISEL & 0x3)<<16));		//DEMUX code set

	hdmi_mask(wCk2Reg,
		~(P0_ck_5_CMU_SEL_R1_mask|P0_ck_5_CMU_SEL_PUMP_DB_mask|
		P0_ck_5_CMU_SEL_PUMP_I_mask|P0_ck_6_CMU_SEL_CS_mask|
		P0_ck_6_CMU_EN_CAP_mask | P0_ck_7_CMU_BYPASS_PI_mask|
		P0_ck_8_CMU_SEL_CP_mask|P0_ck_8_CMU_BPPSR_mask),
		((phy_param->CK_RS<<0)|(phy_param->Icp_DUL<<3)|
		(phy_param->CK_Icp<<4)|(phy_param->CK_CS<<8)|
		(phy_param->EN_CAP<<12)| P0_ck_7_CMU_BYPASS_PI_mask|
		(phy_param->CMU_SEL_CP << 20)|DFE_HDMI_RX_PHY_P0_CMU_BPPSR(phy_param->pscaler)));

#if 0	//merlin5 remove ldo
	hdmi_mask(wCk1Reg,~(p0_ck_2_LDO_EN),(p0_ck_2_LDO_EN));
#endif

	hdmi_mask(wReg2,
		~(HDMIRX_2P0_PHY_REGD5_reg_p1_rate_sel_mask|
		HDMIRX_2P0_PHY_REGD0_reg_p0_kp_mask|
		HDMIRX_2P0_PHY_REGD0_reg_p0_ki_mask|
		HDMIRX_2P0_PHY_REGD0_reg_p0_bypass_sdm_int_mask),
		(HDMIRX_2P0_PHY_REGD5_reg_p1_rate_sel(phy_param->RATE_SEL)|
		(phy_param->CDR_KP<<HDMIRX_2P0_PHY_REGD0_reg_p0_kp_shift)|
		(phy_param->CDR_KI<<HDMIRX_2P0_PHY_REGD0_reg_p0_ki_shift)|
		HDMIRX_2P0_PHY_REGD0_reg_p0_bypass_sdm_int_mask));


		hdmi_mask(ck_md_sel, ~HDMIRX_2P0_PHY_REGD56_reg_p0_ck_md_sel_mask, HDMIRX_2P0_PHY_REGD56_reg_p0_ck_md_sel(0));
#endif	

}



void lib_hdmi_cco_band_config(unsigned char nport, unsigned int bclk, HDMI_PHY_PARAM_T *phy_param)
{
#if 0
	HDMI_PHY_PARAM2_T *phy_param2 = &hdmi_phy_param2[0];
	unsigned char i = 0, tabsize = 0;
	unsigned int Fvco = 0;
	unsigned int tmds_b = bclk;
	unsigned int reg_ck2_addr;
	
	if (nport == HDMI_PORT2) {
		reg_ck2_addr = HDMIRX_2P0_PHY_P0_CK2_reg;
		
	} else if (nport == HDMI_PORT3) {
		reg_ck2_addr = HDMIRX_2P0_PHY_P1_CK2_reg;
	}else {
		return;
	}
	
	//Fvco=[(Fin x prescaler x SEL_D4 x M_Code) ¡Ò N_Code ]
	//Fin=[(b+1)x27¡Ò256]¡A


	if (newbase_hdmi2p0_get_clock40x_flag(nport)) {
		if (((bclk >> 2) < 1422) && ((bclk >> 2) > 699))
			tmds_b = (bclk >> 2);
	}

	//cco band
	Fvco = (tmds_b+1) * 27 *  (phy_param->pscaler? 1:2) * (1<< phy_param->SEL_D4) * (phy_param->M_code + 4);	
	Fvco = (Fvco / (1<< phy_param->N_code) ) >> 8;


	tabsize = sizeof(hdmi_phy_param2)/sizeof(HDMI_PHY_PARAM2_T);

	for (i = 0 ; i < tabsize ; ++i) {
		if ( (Fvco >= hdmi_phy_param2[i].fvco_lower)  &&  (Fvco < phy_param2[i].fvco_upper)) {
				break;
		}
	}

	if (i >= tabsize) {
	        HDMI_EMG("Not Found in PHY_PARAM2 (Fvco=%d)\n", Fvco);
		 return;
	}

	phy_param2 = &hdmi_phy_param2[i];

	//setting register in lib_hdmi_set_phy_table
	//phy_param->CMU_KVCO = phy_param2 ->CMU_KVCO;  //merlin5 remove
	phy_param->EN_CAP = phy_param2->EN_CAP;
	phy_param->PI_ISEL = phy_param2->PI_ISEL;
	phy_param->PI_CSEL = phy_param2->PI_CSEL;

	hdmi_mask(reg_ck2_addr, ~(P0_ck_8_ENC_mask), DFE_HDMI_RX_PHY_P0_ENC(phy_param2->CCO_BAND_SEL));
#endif
}


void lib_hdmi_fg_koffset_manual_adjust(unsigned char port, unsigned int r, unsigned int g, unsigned int b, unsigned int ck)
{
#if 0
	unsigned int koff_b_addr;
	unsigned int koff_g_addr;
	unsigned int koff_r_addr;

	if (port == HDMI_PORT2) {
		koff_b_addr = HDMIRX_2P0_PHY_P0_B1_reg;
		koff_g_addr = HDMIRX_2P0_PHY_P0_G1_reg;
		koff_r_addr = HDMIRX_2P0_PHY_P0_R1_reg;

	} else if (port == HDMI_PORT3) {
		koff_b_addr = HDMIRX_2P0_PHY_P1_B1_reg;
		koff_g_addr = HDMIRX_2P0_PHY_P1_G1_reg;
		koff_r_addr = HDMIRX_2P0_PHY_P1_R1_reg;
	}else {
		return;
	}

	hdmi_mask(koff_r_addr, ~(P0_b_1_FORE_KOFF_ADJR), r<<16);
	hdmi_mask(koff_g_addr, ~(P0_b_1_FORE_KOFF_ADJR), g<<16);
	hdmi_mask(koff_b_addr, ~(P0_b_1_FORE_KOFF_ADJR), b<<16);

#endif
}

void lib_hdmi_fg_koffset_manual(unsigned char port)
{
#if 0
	unsigned int koff_b_addr;
	unsigned int koff_g_addr;
	unsigned int koff_r_addr;
	
	if (port == HDMI_PORT2) {
		koff_b_addr = HDMIRX_2P0_PHY_P0_B1_reg;
		koff_g_addr = HDMIRX_2P0_PHY_P0_G1_reg;
		koff_r_addr = HDMIRX_2P0_PHY_P0_R1_reg;

	} else if (port == HDMI_PORT3) {
		koff_b_addr = HDMIRX_2P0_PHY_P1_B1_reg;
		koff_g_addr = HDMIRX_2P0_PHY_P1_G1_reg;
		koff_r_addr = HDMIRX_2P0_PHY_P1_R1_reg;
	}else {
		return;
	}
	
  	hdmi_mask(koff_r_addr, ~(P0_r_4_FORE_KOFF_AUTOK), 0);
  	hdmi_mask(koff_g_addr, ~(P0_g_4_FORE_KOFF_AUTOK), 0);
	hdmi_mask(koff_b_addr, ~(P0_b_4_FORE_KOFF_AUTOK), 0);
#endif	
}

void lib_hdmi_fg_koffset_auto(unsigned char port)
{
  #if   0
#define FORE_KOFF_RANGE  3  // range maybe 0  1  2   3
	UINT8 i, k_cnt;
	UINT32 w_b_addr_control,w_g_addr_control,w_r_addr_control;
	UINT32 w_b_addr_result,w_g_addr_result,w_r_addr_result;
	volatile u_int32_t koff_r_result;
	volatile u_int32_t  koff_g_result;
	volatile u_int32_t  koff_b_result;

	if (port == HDMI_PORT2) {
		w_b_addr_control =HDMIRX_2P0_PHY_P0_B1_reg;
		w_g_addr_control = HDMIRX_2P0_PHY_P0_G1_reg;
		w_r_addr_control = HDMIRX_2P0_PHY_P0_R1_reg;
		w_b_addr_result = HDMIRX_2P0_PHY_P0_B4_reg;
		w_g_addr_result = HDMIRX_2P0_PHY_P0_G4_reg;
		w_r_addr_result = HDMIRX_2P0_PHY_P0_R4_reg;
	} else if (port == HDMI_PORT3) {
		w_b_addr_control = HDMIRX_2P0_PHY_P1_B1_reg;
		w_g_addr_control = HDMIRX_2P0_PHY_P1_G1_reg;
		w_r_addr_control = HDMIRX_2P0_PHY_P1_R1_reg;
		w_b_addr_result = HDMIRX_2P0_PHY_P1_B4_reg;
		w_g_addr_result = HDMIRX_2P0_PHY_P1_G4_reg;
		w_r_addr_result = HDMIRX_2P0_PHY_P1_R4_reg;
	} else {
		return;
	}


	// initial circuit power
	hdmi_mask(w_b_addr_control, ~(P0_b_3_FORE_EN_FORE_KOFF|P0_b_4_FORE_KOFF_AUTOK|P0_b_4_FORE_POW_FORE_KOFF),(P0_b_3_FORE_EN_FORE_KOFF|P0_b_4_FORE_KOFF_AUTOK));
	hdmi_mask(w_g_addr_control, ~(P0_b_3_FORE_EN_FORE_KOFF|P0_b_4_FORE_KOFF_AUTOK|P0_b_4_FORE_POW_FORE_KOFF),(P0_b_3_FORE_EN_FORE_KOFF|P0_b_4_FORE_KOFF_AUTOK));
	hdmi_mask(w_r_addr_control, ~(P0_b_3_FORE_EN_FORE_KOFF|P0_b_4_FORE_KOFF_AUTOK|P0_b_4_FORE_POW_FORE_KOFF),(P0_b_3_FORE_EN_FORE_KOFF|P0_b_4_FORE_KOFF_AUTOK));

	// cut input off
	hdmi_mask(w_b_addr_control, ~(P0_b_1_inputoff),P0_b_1_inputoff);
	hdmi_mask(w_g_addr_control, ~(P0_b_1_inputoff),P0_b_1_inputoff);
	hdmi_mask(w_r_addr_control, ~(P0_b_1_inputoff),P0_b_1_inputoff);


	//for b lane
	for (i=0; i<FORE_KOFF_RANGE+1; i++) {
		hdmi_mask(w_b_addr_control, ~(P0_b_4_FORE_POW_FORE_KOFF),0);

		//set k off range
		hdmi_mask(w_b_addr_control, ~(P0_b_1_FORE_KOFF_RANGE),i<<21);

		// start cal
		hdmi_mask(w_b_addr_control, ~(P0_b_4_FORE_POW_FORE_KOFF),P0_b_4_FORE_POW_FORE_KOFF);

		//wait finish cal
		k_cnt = 100;
		while (!((hdmi_in(w_b_addr_result))&HDMIRX_2P0_PHY_P0_B4_reg_p0_b_koffok_mask) && --k_cnt) {
			udelay(10);
		}

		if (!k_cnt) {
			HDMI_EMG("[ERROR] %s: func err(B)\n", __func__);
		}

		koff_b_result = HDMIRX_2P0_PHY_P0_B4_get_reg_p0_b_koff_sel(hdmi_in(w_b_addr_result));

		if (!(hdmi_in(w_b_addr_result) & HDMIRX_2P0_PHY_P0_B4_reg_p0_b_koff_bound_mask)) {
			//HDMI_PRINTF("[HDMI] R channel foreground K Off result   %x  range =%x \n",koff_b_result,i);
			break;
		} else {

			if (i == FORE_KOFF_RANGE)
				HDMI_EMG("%s: out of B range (value=%d)\n", __func__,i);

		}
	}


	//for g lane
	for (i=0; i<FORE_KOFF_RANGE+1; i++) {
		hdmi_mask(w_g_addr_control, ~(P0_b_4_FORE_POW_FORE_KOFF),0);

		//set k off range
		hdmi_mask(w_g_addr_control, ~(P0_b_1_FORE_KOFF_RANGE),i<<21);

		// start cal
		hdmi_mask(w_g_addr_control, ~(P0_b_4_FORE_POW_FORE_KOFF),P0_b_4_FORE_POW_FORE_KOFF);

		//wait finish cal
		k_cnt = 100;
		while (!((hdmi_in(w_g_addr_result))&HDMIRX_2P0_PHY_P0_B4_reg_p0_b_koffok_mask) && --k_cnt) {
			udelay(10);
		}

		if (!k_cnt) {
			HDMI_EMG("[ERROR] %s: func err(G)\n", __func__);
		}

		koff_g_result = HDMIRX_2P0_PHY_P0_B4_get_reg_p0_b_koff_sel(hdmi_in(w_g_addr_result));

		if (!(hdmi_in(w_g_addr_result) & HDMIRX_2P0_PHY_P0_B4_reg_p0_b_koff_bound_mask)) {
			break;
		} else {
			if (i == FORE_KOFF_RANGE)
				HDMI_EMG("%s: out of G range (value=%d)\n", __func__,i);

		}
	}


	//for r lane
	for (i=0; i<FORE_KOFF_RANGE+1; i++) {
		hdmi_mask(w_r_addr_control, ~(P0_b_4_FORE_POW_FORE_KOFF),0);

		//set k off range
		hdmi_mask(w_r_addr_control, ~(P0_b_1_FORE_KOFF_RANGE),i<<21);

		// start cal
		hdmi_mask(w_r_addr_control, ~(P0_b_4_FORE_POW_FORE_KOFF),P0_b_4_FORE_POW_FORE_KOFF);

		//wait finish cal
		k_cnt = 100;
		while(!((hdmi_in(w_r_addr_result))&HDMIRX_2P0_PHY_P0_B4_reg_p0_b_koffok_mask) && --k_cnt) {
			udelay(10);
		}

		if (!k_cnt) {
			HDMI_EMG("[ERROR] %s: func err(R)\n", __func__);
		}

		koff_r_result = HDMIRX_2P0_PHY_P0_B4_get_reg_p0_b_koff_sel(hdmi_in(w_r_addr_result));

		if (!(hdmi_in(w_r_addr_result) & HDMIRX_2P0_PHY_P0_B4_reg_p0_b_koff_bound_mask)) {
			break;
		} else {
			if (i == FORE_KOFF_RANGE)
				HDMI_EMG("%s: out of R range (value=%d)\n", __func__,i);

		}
	}

	// cut input on
	hdmi_mask(w_b_addr_control, ~(P0_b_1_inputoff),0);
	hdmi_mask(w_g_addr_control, ~(P0_b_1_inputoff),0);
	hdmi_mask(w_r_addr_control, ~(P0_b_1_inputoff),0);
	
	HDMI_PRINTF("FG KOFFSET=(b=%d,g=%d,r=%d)\n", koff_b_result,koff_g_result,koff_r_result);
#endif
}

void lib_hdmi_fg_koffset_proc(unsigned char nport)
{
#if 0
	lib_hdmi_fg_koffset_auto(nport);
	lib_hdmi_cdr_rst(nport, 1);
	udelay(5);
	lib_hdmi_cdr_rst(nport, 0);
	udelay(50);
#endif	
}

void lib_hdmi_fg_koffset_disable(unsigned char port)
{
#if 0
	UINT32 w_b_addr_control,w_g_addr_control,w_r_addr_control;

	if (port == HDMI_PORT2) {
		w_b_addr_control = HDMIRX_2P0_PHY_P0_B1_reg;
		w_g_addr_control = HDMIRX_2P0_PHY_P0_G1_reg;
		w_r_addr_control = HDMIRX_2P0_PHY_P0_R1_reg;
	} else if (port == HDMI_PORT3) {
		w_b_addr_control = HDMIRX_2P0_PHY_P1_B1_reg;
		w_g_addr_control = HDMIRX_2P0_PHY_P1_G1_reg;
		w_r_addr_control = HDMIRX_2P0_PHY_P1_R1_reg;
	} else {
		return;
	}

	hdmi_mask(w_b_addr_control, ~(P0_b_3_FORE_EN_FORE_KOFF|P0_b_4_FORE_POW_FORE_KOFF|P0_b_4_FORE_KOFF_AUTOK), 0);
	hdmi_mask(w_g_addr_control, ~(P0_b_3_FORE_EN_FORE_KOFF|P0_b_4_FORE_POW_FORE_KOFF|P0_b_4_FORE_KOFF_AUTOK), 0);
	hdmi_mask(w_r_addr_control, ~(P0_b_3_FORE_EN_FORE_KOFF|P0_b_4_FORE_POW_FORE_KOFF|P0_b_4_FORE_KOFF_AUTOK), 0);
#endif	
}

 bool AP_SET_HDMI_ext_trace_z0=false; //
 unsigned char ext_trace_z0=5;
void lib_ap_set_hdmi_ext_trace_z0(unsigned int z0)
{
	HDMI_PRINTF("%s set z0=%x\n", __func__,z0);
	HDMI_PRINTF("AP_SET_HDMI_ext_trace_z0==TRUE\n");
	AP_SET_HDMI_ext_trace_z0=TRUE;
	if (z0>5)
	z0=5;
	ext_trace_z0=z0;
	//lib_hdmi_z0_calibration();
	lib_hdmi21_z0_calibration();
}
	
//0xB80381E0
//B80381E0[31:0]: z0 value 4 port
//port0_z0=B80381E0[4:0] port0_done=B80381E0[5]
//port1_z0=B80381E0[10:6] port1_done=B80381E0[11]
//port2_z0=B80381E0[16:12] port2_done=B80381E0[17]
//port3_z0=B80381E0[22:18] port3_done=B80381E0[23]

void lib_hdmi_z0_calibration(void)
{
#if 0
	 #define Z0_DIFF_ERR		0x0
        unsigned int z0_otp_value = rtd_inl(EFUSE_DATAO_120_reg);
	 unsigned char z0_otp_done;
        unsigned char z0_2,z0_3;    //z0 for 4 port
	 unsigned char z0_low_bound_2,z0_low_bound_3;     //  85 -- 100 -- 115  +-15%
	 unsigned char z0_high_bound_2,z0_high_bound_3;    //high boundary z0 for all port
	 unsigned char ext_trace_z0_2,ext_trace_z0_3; //external z0 for 3 port
	 unsigned char soc_z0 = PHY_Z0;
	 unsigned char target_z0_2,target_z0_3;     // target_z0 = soc_z0 + pcb_trace_z0  (target z0 for all port)
	 unsigned long long val = 0;    

	 ext_trace_z0_2 = ext_trace_z0_3 = (0x5 - Z0_DIFF_ERR);  // 1 step 1.5 ohm  5.4 ohm = 4 step
	 z0_otp_done = ((z0_otp_value & 0x00000020) >> 5) | ((z0_otp_value & 0x00000800) >> 10) | ((z0_otp_value & 0x00020000) >> 15) | ((z0_otp_value & 0x00800000) >> 20);
	 
	  if (AP_SET_HDMI_ext_trace_z0==TRUE) {
		ext_trace_z0_2 = ext_trace_z0- Z0_DIFF_ERR;
		ext_trace_z0_3 = ext_trace_z0- Z0_DIFF_ERR;
	  }else  if (pcb_mgr_get_enum_info_byname("HDMI_EXT_Z0_CFG", &val)==0) {
		ext_trace_z0_2 = ((val & 0x00FF0000) >> 16) - Z0_DIFF_ERR;
		ext_trace_z0_3 = ((val & 0xFF000000) >> 24) - Z0_DIFF_ERR;
	  } else {
			HDMI_PRINTF("NOT GET  EXTERNAL PCB TRACE Z0\n");
	  }
	  
	HDMI_PRINTF("soc_z0=%x p2_ext_trace_z0=%x p3_ext_trace_z0=%x\n",soc_z0,ext_trace_z0_2,ext_trace_z0_3);
	

	 target_z0_2 = soc_z0 + ext_trace_z0_2;
	 target_z0_3 = soc_z0 + ext_trace_z0_3;

	 z0_high_bound_2 = target_z0_2 + 10;
	 z0_high_bound_3 = target_z0_3 + 10;
	 
	 if (target_z0_2 > 6)
	 	z0_low_bound_2 = target_z0_2 - 6;
	 else {
		z0_low_bound_2 = 0;
	 }

	  if (target_z0_3 > 6)
	 	z0_low_bound_3 = target_z0_3 - 6;
	 else {
		z0_low_bound_3 = 0;
	 }

	HDMI_PRINTF("Z0 OTP done_bit=%x value=%x\n",z0_otp_done,z0_otp_value);

	
        if (z0_otp_done & 0x4) {
                z0_2 = (z0_otp_value & 0x0001F000) >> 12;
		  z0_2 = z0_2 + ext_trace_z0_2;
		  if ((z0_2 < z0_low_bound_2) || (z0_2 > z0_high_bound_2)) {
			  z0_2 = target_z0_2;
			   HDMI_PRINTF("port2 z0 otp over boundary\n");
		  }
        }
        else
                z0_2 = target_z0_2;

        if (z0_otp_done & 0x8) {
                z0_3 = (z0_otp_value & 0x007C0000) >> 18;
		  z0_3 = z0_3 + ext_trace_z0_3;
		 if ((z0_3 < z0_low_bound_3) || (z0_3 > z0_high_bound_3)) {
			z0_3 = target_z0_3;
			 HDMI_PRINTF("port3 z0 otp over boundary\n");
		 }
        }
        else
                z0_3 = target_z0_3;

	HDMI_PRINTF("z0_2=%x z0_3=%x\n",z0_2,z0_3);


        hdmi_mask(HDMIRX_2P0_PHY_Z0K_reg, ~(HDMIRX_2P0_PHY_Z0K_reg_z0_adjr_0_mask|HDMIRX_2P0_PHY_Z0K_reg_z0_adjr_1_mask),
                (HDMIRX_2P0_PHY_Z0K_reg_z0_adjr_0(z0_2)|HDMIRX_2P0_PHY_Z0K_reg_z0_adjr_1(z0_3)));
#endif
	
}

#if 0
void lib_hdmi_single_end(unsigned char nport, unsigned char channel, unsigned char p_off, unsigned char n_off)
{
// channel = 0(R), 1(G), 2(B) 3(CK)
	hdmi_mask(reg_rgb1[nport][channel], ~(P0_b_1_inputoff_p), p_off<<2);
	hdmi_mask(reg_rgb1[nport][channel], ~(P0_b_1_inputoff_n), n_off<<2);

	if (nport == HDMI_PORT3) {
		if (channel == 3) {
			hdmi_mask(DFE_HDMI_RX_PHY_P0_HD21_P0_CK1_reg, ~(P0_b_1_inputoff_p), p_off<<2);
			hdmi_mask(DFE_HDMI_RX_PHY_P0_HD21_P0_CK1_reg, ~(P0_b_1_inputoff_n), n_off<<2);
		}
	}
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
  #if 0
	#define ERR_DET_ONETIME_MODE 0
	signed char dfe_arg;
	unsigned int bit_r_err, bit_g_err, bit_b_err, ch_r_err, ch_g_err, ch_b_err;
	unsigned char mode;
	unsigned int crc = 0;
	unsigned char bgrc;
	unsigned char tmds_ch;

	lib_hdmi_set_fw_debug_bit(DEBUG_27_BYPASS_PHY_ISR, 1);
	lib_hdmi_set_fw_debug_bit(DEBUG_26_BYPASS_DETECT, 1);
	lib_hdmi_set_fw_debug_bit(DEBUG_25_BYPASS_CHECK, 1);
	HDMI_EMG("================SCAN START===================\n");

	if (lane_mode == HDMI_4LANE)
		bgrc = LN_ALL;
	else
		bgrc = (LN_R|LN_G|LN_B);

	//tap0 0~36
	for (dfe_arg=0; dfe_arg<0x25; dfe_arg++) {
		lib_hdmi_dfe_init_tap0(nport, bgrc, dfe_arg);
		//lib_hdmi_dfe_init_tap1(nport, (LN_R|LN_G|LN_B), dfe_arg);
		//lib_hdmi_dfe_init_tap2(nport, (LN_R|LN_G|LN_B), dfe_arg);
		udelay(100);

#if ERR_DET_ONETIME_MODE
		lib_hdmi_bit_err_start(nport, 0, 63);
		lib_hdmi_char_err_start(nport, 0, 63);
		lib_hdmi_clear_rgb_hv_status(nport);

		while (!lib_hdmi_bit_err_get_error(nport, &bit_r_err, &bit_g_err, &bit_b_err)) {
			msleep(20);
		}
		while (!lib_hdmi_char_err_get_error(nport, &ch_r_err, &ch_g_err, &ch_b_err)) {
			msleep(20);
		}
#else
		lib_hdmi_bit_err_start(nport, 1, 10);
		lib_hdmi_char_err_start(nport, 1, 10);
		lib_hdmi_clear_rgb_hv_status(nport);
		msleep(1500);
		lib_hdmi_bit_err_get_error(nport, &bit_r_err, &bit_g_err, &bit_b_err);
		lib_hdmi_char_err_get_error(nport, &ch_r_err, &ch_g_err, &ch_b_err);
#endif
		crc = lib_hdmi_read_crc(nport);
		tmds_ch = (lib_hdmi_read_rgb_hv_status(nport) & 0x000000F8) >> 3;

		HDMI_EMG("tap0=%02d, [CH](%05d, %05d, %05d); [BIT](%05d, %05d, %05d) PP_CRC=%x tmds_ch=%x\n",
			dfe_arg, ch_r_err, ch_g_err, ch_b_err, bit_r_err, bit_g_err, bit_b_err,crc,tmds_ch);

	}

	lib_hdmi_dfe_init_tap0(nport, (LN_B), phy_st[nport].dfe_t[0].tap0);
	lib_hdmi_dfe_init_tap0(nport, (LN_G), phy_st[nport].dfe_t[1].tap0);
	lib_hdmi_dfe_init_tap0(nport, (LN_R), phy_st[nport].dfe_t[2].tap0);

	if (lane_mode == HDMI_4LANE)
		lib_hdmi_dfe_init_tap0(nport, (LN_CK), phy_st[nport].dfe_t[3].tap0);


	mode = DFE_P0_REG_DFE_MODE_P0_get_adapt_mode_p0(hdmi_in(DFE_REG_DFE_MODE_reg));


	if (mode == 2) {
			//le 0~24
			for (dfe_arg=0; dfe_arg<25; dfe_arg++) {
			//lib_hdmi_dfe_init_tap0(nport, (LN_R|LN_G|LN_B), dfe_arg);
			//lib_hdmi_dfe_init_tap1(nport, (LN_R|LN_G|LN_B), dfe_arg);
			lib_hdmi_dfe_init_le(nport, (LN_R|LN_G|LN_B), dfe_arg);
			//lib_hdmi_dfe_init_tap2(nport, (LN_R|LN_G|LN_B), dfe_arg);
			udelay(100);

#if ERR_DET_ONETIME_MODE
			lib_hdmi_bit_err_start(nport, 0, 63);
			lib_hdmi_char_err_start(nport, 0, 63);
			lib_hdmi_clear_rgb_hv_status(nport);

			while (!lib_hdmi_bit_err_get_error(nport, &bit_r_err, &bit_g_err, &bit_b_err)) {
				msleep(20);
			}
			
			while (!lib_hdmi_char_err_get_error(nport, &ch_r_err, &ch_g_err, &ch_b_err)) {
				msleep(20);
			}
#else
			lib_hdmi_bit_err_start(nport, 1, 10);
			lib_hdmi_char_err_start(nport, 1, 10);
			lib_hdmi_clear_rgb_hv_status(nport);
			msleep(1500);
			lib_hdmi_bit_err_get_error(nport, &bit_r_err, &bit_g_err, &bit_b_err);
			lib_hdmi_char_err_get_error(nport, &ch_r_err, &ch_g_err, &ch_b_err);
#endif
			crc = lib_hdmi_read_crc(nport);
			tmds_ch = (lib_hdmi_read_rgb_hv_status(nport) & 0x000000F8) >> 3;
			
			HDMI_EMG("le=%02d, [CH](%05d, %05d, %05d); [BIT](%05d, %05d, %05d) PP_CRC=%x tmds_ch=%x\n",
				dfe_arg, ch_r_err, ch_g_err, ch_b_err, bit_r_err, bit_g_err, bit_b_err,crc,tmds_ch);

		}

		lib_hdmi_dfe_init_le(nport, (LN_B), phy_st[nport].dfe_t[0].le);
		lib_hdmi_dfe_init_le(nport, (LN_G), phy_st[nport].dfe_t[1].le);
		lib_hdmi_dfe_init_le(nport, (LN_R), phy_st[nport].dfe_t[2].le);
		if (lane_mode == HDMI_4LANE)
			lib_hdmi_dfe_init_le(nport, (LN_CK), phy_st[nport].dfe_t[3].le);
	} else {
		
		//mode 3
			for (dfe_arg=0; dfe_arg<25; dfe_arg++) {
				//lib_hdmi_dfe_init_tap0(nport, (LN_R|LN_G|LN_B), dfe_arg);
				lib_hdmi_dfe_init_tap1(nport, bgrc, dfe_arg);
				//lib_hdmi_dfe_init_tap2(nport, (LN_R|LN_G|LN_B), dfe_arg);
				udelay(100);

#if ERR_DET_ONETIME_MODE
				lib_hdmi_bit_err_start(nport, 0, 63);
				lib_hdmi_char_err_start(nport, 0, 63);
				lib_hdmi_clear_rgb_hv_status(nport);

				while (!lib_hdmi_bit_err_get_error(nport, &bit_r_err, &bit_g_err, &bit_b_err)) {
					msleep(20);
				}
				while (!lib_hdmi_char_err_get_error(nport, &ch_r_err, &ch_g_err, &ch_b_err)) {
					msleep(20);
				}
#else
				lib_hdmi_bit_err_start(nport, 1, 10);
				lib_hdmi_char_err_start(nport, 1, 10);
				lib_hdmi_clear_rgb_hv_status(nport);
				msleep(1500);
				lib_hdmi_bit_err_get_error(nport, &bit_r_err, &bit_g_err, &bit_b_err);
				lib_hdmi_char_err_get_error(nport, &ch_r_err, &ch_g_err, &ch_b_err);
#endif
				crc = lib_hdmi_read_crc(nport);
				tmds_ch = (lib_hdmi_read_rgb_hv_status(nport) & 0x000000F8) >> 3;
				HDMI_EMG("le=%02d, [CH](%05d, %05d, %05d); [BIT](%05d, %05d, %05d) PP_CRC=%x tmds_ch=%x\n",
					dfe_arg, ch_r_err, ch_g_err, ch_b_err, bit_r_err, bit_g_err, bit_b_err,crc, tmds_ch);
			}

			lib_hdmi_dfe_init_tap1(nport, (LN_B), phy_st[nport].dfe_t[0].tap1+phy_st[nport].dfe_t[0].le);
			lib_hdmi_dfe_init_tap1(nport, (LN_G), phy_st[nport].dfe_t[1].tap1+phy_st[nport].dfe_t[1].le);
			lib_hdmi_dfe_init_tap1(nport, (LN_R), phy_st[nport].dfe_t[2].tap1+phy_st[nport].dfe_t[2].le);
			if (lane_mode == HDMI_4LANE)
				lib_hdmi_dfe_init_tap1(nport, (LN_CK), phy_st[nport].dfe_t[3].tap1+phy_st[nport].dfe_t[3].le);
	}

	

	//tap2 -20~20
	for (dfe_arg=-20; dfe_arg<21; dfe_arg++) {
		//lib_hdmi_dfe_init_tap0(nport, (LN_R|LN_G|LN_B), dfe_arg);
		//lib_hdmi_dfe_init_tap1(nport, (LN_R|LN_G|LN_B), dfe_arg);
		lib_hdmi_dfe_init_tap2(nport, bgrc, dfe_arg);
		udelay(100);
#if ERR_DET_ONETIME_MODE
		lib_hdmi_bit_err_start(nport, 0, 63);
		lib_hdmi_char_err_start(nport, 0, 63);
		lib_hdmi_clear_rgb_hv_status(nport);
		while (!lib_hdmi_bit_err_get_error(nport, &bit_r_err, &bit_g_err, &bit_b_err)) {
			msleep(20);
		}
		while (!lib_hdmi_char_err_get_error(nport, &ch_r_err, &ch_g_err, &ch_b_err)) {
			msleep(20);
		}
#else
		lib_hdmi_bit_err_start(nport, 1, 10);
		lib_hdmi_char_err_start(nport, 1, 10);
		lib_hdmi_clear_rgb_hv_status(nport);
		msleep(1500);
		lib_hdmi_bit_err_get_error(nport, &bit_r_err, &bit_g_err, &bit_b_err);
		lib_hdmi_char_err_get_error(nport, &ch_r_err, &ch_g_err, &ch_b_err);
#endif

		crc = lib_hdmi_read_crc(nport);
		tmds_ch = (lib_hdmi_read_rgb_hv_status(nport) & 0x000000F8) >> 3;
		HDMI_EMG("tap2=%02d, [CH](%05d, %05d, %05d); [BIT](%05d, %05d, %05d) PP_CRC=%x tmds_ch=%x\n",
			dfe_arg, ch_r_err, ch_g_err, ch_b_err, bit_r_err, bit_g_err, bit_b_err,crc,tmds_ch);

	}

	lib_hdmi_dfe_init_tap2(nport, (LN_B), phy_st[nport].dfe_t[0].tap2);
	lib_hdmi_dfe_init_tap2(nport, (LN_G), phy_st[nport].dfe_t[1].tap2);
	lib_hdmi_dfe_init_tap2(nport, (LN_R), phy_st[nport].dfe_t[2].tap2);
	if (lane_mode == HDMI_4LANE)
		lib_hdmi_dfe_init_tap2(nport, (LN_CK), phy_st[nport].dfe_t[3].tap2);

	//disable bit error/character detect
	lib_hdmi_bit_err_start(nport, 0, 0);
	lib_hdmi_char_err_start(nport, 0, 0);

	
	lib_hdmi_set_fw_debug_bit(DEBUG_27_BYPASS_PHY_ISR, 0);
	lib_hdmi_set_fw_debug_bit(DEBUG_26_BYPASS_DETECT, 0);
	lib_hdmi_set_fw_debug_bit(DEBUG_25_BYPASS_CHECK, 0);

	HDMI_EMG("================SCAN END===================\n");

#endif
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
#if 0  // FIXME : phy register has been changed...
/*

CK_ICP  b800da54[7:4] = 0x8 ~ 0xe
KP=6	b800da90[15:8] = 0x18
KP=7    b800da90[15:8] = 0x1c
KP=8	b800da90[15:8] = 0x20
KP=9    b800da90[15:8] = 0x24
KP=10	b800da90[15:8] = 0x28
*/

	#define MAX_ICP 0xE
	#define MIN_ICP 0x8
	#define MAX_KP 0xC
	#define MIN_KP 0x4
	
	static unsigned int original_icp, original_kp;
	static unsigned int modify_icp, modify_kp;
	unsigned int wReg2,wCk2Reg;


	if (nport == HDMI_PORT0) {
		wReg2 = DFE_HDMI_RX_PHY_REGD0_reg;
		wCk2Reg = DFE_HDMI_RX_PHY_P0_CK2_reg;
	} else if (nport == HDMI_PORT1) {
		wReg2 = DFE_HDMI_RX_PHY_REGD5_reg;
		wCk2Reg = DFE_HDMI_RX_PHY_P1_CK2_reg;
	} else if (nport == HDMI_PORT2) {
		wReg2 = DFE_HDMI_RX_PHY_REGD10_reg;
		wCk2Reg = DFE_HDMI_RX_PHY_P2_CK2_reg;
	} else if (nport == HDMI_PORT3) {
		wReg2 = DFE_HDMI_RX_PHY_P0_HD21_Y1_REGD00_reg;
		wCk2Reg = DFE_HDMI_RX_PHY_P0_HD21_P0_CMU2_reg;
	} else {
		return;
	}

	msleep(5000);
	
	switch(phy_scan_state) {
		case PHY_INIT_SCAN:
			HDMI_EMG("PHY_INIT_SCAN");
			original_icp = (((hdmi_in(wCk2Reg) & P0_ck_5_CMU_SEL_PUMP_I_mask)) >> 4);
			original_kp = 	DFE_HDMI_RX_PHY_REGD0_get_reg_p0_kp(hdmi_in(wReg2));
			modify_icp = MIN_ICP;
			modify_kp = MIN_KP;
			phy_scan_state = PHY_ICP_SCAN;
			phy_st[nport].clk_pre = 0;
			phy_st[nport].clk= 0;
			phy_st[nport].clk_debounce_count= 0;
			phy_st[nport].clk_unstable_count = 0;
			phy_st[nport].clk_stable = 0;
			phy_st[nport].phy_proc_state = PHY_PROC_INIT;
			break;
		
		case PHY_ICP_SCAN:
			if ((modify_icp) > MAX_ICP) {
				hdmi_mask(wCk2Reg, P0_ck_5_CMU_SEL_PUMP_I_mask,  (original_icp << 4));
				phy_scan_state = PHY_KP_SCAN;
				
			} else {

				HDMI_EMG("PHY_ICP_SCAN");
				HDMI_EMG("ICP=%d\n",modify_icp);
				hdmi_mask(wCk2Reg, P0_ck_5_CMU_SEL_PUMP_I_mask,  (modify_icp << 4));
				modify_icp++;
			}
			break;

		case PHY_KP_SCAN:
			if ((modify_kp) > MAX_KP) {
				hdmi_mask(wReg2, DFE_HDMI_RX_PHY_REGD0_reg_p0_kp_mask,  ((original_kp << 2) << DFE_HDMI_RX_PHY_REGD0_reg_p0_kp_shift));
				phy_scan_state = PHY_END_SCAN;
				
			} else {

				HDMI_EMG("PHY_KP_SCAN");
				HDMI_EMG("KP=%d\n",modify_kp);
				hdmi_mask(wReg2, DFE_HDMI_RX_PHY_REGD0_reg_p0_kp_mask, ((modify_kp << 2) << DFE_HDMI_RX_PHY_REGD0_reg_p0_kp_shift));
				modify_kp++;
			}
			break;

		case PHY_END_SCAN:
			HDMI_EMG("PHY_END_SCAN");
			lib_hdmi_set_fw_debug_bit(DEBUG_22_PHY_SCAN_TEST,0x0);
			break;

	}
#endif
}

#endif

#ifndef BUILD_QUICK_SHOW
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
#endif

HDMIRX_PHY_STRUCT_T* newbase_rxphy_get_status(unsigned char port)
{
	return (port < HDMI_PORT_TOTAL_NUM) ? &phy_st[port] : NULL;
}


//------------------------------------------------------------------
// HPD/DET
//------------------------------------------------------------------

const unsigned int dedicate_hpd_det_reg[] =
{
    PINMUX_MAIN_ST_HPD_R1K_Ctrl0_reg,
    PINMUX_MAIN_ST_HPD_R1K_Ctrl1_reg,
    PINMUX_MAIN_ST_HPD_R1K_Ctrl2_reg,
    PINMUX_MAIN_ST_HPD_R1K_Ctrl3_reg
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
		val = PINMUX_MAIN_ST_HPD_R1K_Ctrl0_hdmirx_p0_reg_sel(HPD_EARC_R1K_SEL); //Internal R1K register control, 0.85Kohm~1.0Kohm
	else
		val = PINMUX_MAIN_ST_HPD_R1K_Ctrl0_hdmirx_p0_reg_sel(HPD_R1K_SEL); //Internal R1K register control, 0.85Kohm~1.0Kohm

	hdmi_out(dedicate_hpd_det_reg[hpd_idx], val);	//R1K_EN & PD set to 0 first, avoid set to 1 in the same time
	
	if (en)
		val |= PINMUX_MAIN_ST_HPD_R1K_Ctrl0_hdmirx_p0_r1k_en_mask;    // enable 1K resistance to HDMI 5V and disable pull down
	else
		val |= PINMUX_MAIN_ST_HPD_R1K_Ctrl0_hdmirx_p0_pd_mask;        // disable 1K resistance to HDMI 5V and enable pull down

	hdmi_out(dedicate_hpd_det_reg[hpd_idx], val);	//set R1K_EN & PD at second step
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

    if (PINMUX_MAIN_ST_HPD_R1K_Ctrl0_get_hdmirx_p0_r1k_en(hdmi_in(dedicate_hpd_det_reg[hpd_idx])))
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

    return PINMUX_MAIN_ST_HPD_R1K_Ctrl0_get_hdmirx_p0_h5vdet(hdmi_in(dedicate_hpd_det_reg[det_idx]));
}


