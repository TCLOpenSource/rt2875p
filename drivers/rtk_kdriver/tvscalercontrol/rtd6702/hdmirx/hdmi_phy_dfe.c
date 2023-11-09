#include <linux/time.h>
#include "hdmi_common.h"
#include "hdmi_phy_dfe.h"
#include "hdmi_phy.h"
#include "hdmi_reg.h"
#include "hdmi_vfe_config.h"
#include "hdmi_mac_2p1.h"

extern unsigned char OBJ_TV_only;

void lib_hdmi_set_dfe_close(unsigned char nport,unsigned char lane_mode)
{

	hdmi_mask(REG_DFE_CTRL_BA_reg,~(REG_dfe_adapt_en_TAP0|REG_dfe_adapt_en_TAP1|REG_dfe_adapt_en_TAP2|REG_dfe_adapt_en_TAP3|REG_dfe_adapt_en_TAP4),0);
	hdmi_mask(REG_DFE_CTRL_B3_reg,~(REG_dfe_adapt_en_LE),0);
	hdmi_mask(REG_DFE_CTRL_B2_reg,~(REG_dfe_adapt_en_VTH_DFE_EN|REG_dfe_adapt_en_VTH_EN),0);

	hdmi_mask(REG_DFE_CTRL_GA_reg,~(REG_dfe_adapt_en_TAP0|REG_dfe_adapt_en_TAP1|REG_dfe_adapt_en_TAP2|REG_dfe_adapt_en_TAP3|REG_dfe_adapt_en_TAP4),0);
	hdmi_mask(REG_DFE_CTRL_G3_reg,~(REG_dfe_adapt_en_LE),0);
	hdmi_mask(REG_DFE_CTRL_G2_reg,~(REG_dfe_adapt_en_VTH_DFE_EN|REG_dfe_adapt_en_VTH_EN),0);

	hdmi_mask(REG_DFE_CTRL_RA_reg,~(REG_dfe_adapt_en_TAP0|REG_dfe_adapt_en_TAP1|REG_dfe_adapt_en_TAP2|REG_dfe_adapt_en_TAP3|REG_dfe_adapt_en_TAP4),0);
	hdmi_mask(REG_DFE_CTRL_R3_reg,~(REG_dfe_adapt_en_LE),0);
	hdmi_mask(REG_DFE_CTRL_R2_reg,~(REG_dfe_adapt_en_VTH_DFE_EN|REG_dfe_adapt_en_VTH_EN),0);

	if (lane_mode == HDMI_4LANE)
	{
		hdmi_mask(REG_DFE_CTRL_CKA_reg,~(REG_dfe_adapt_en_TAP0|REG_dfe_adapt_en_TAP1|REG_dfe_adapt_en_TAP2|REG_dfe_adapt_en_TAP3|REG_dfe_adapt_en_TAP4),0);
		hdmi_mask(REG_DFE_CTRL_CK3_reg,~(REG_dfe_adapt_en_LE),0);
		hdmi_mask(REG_DFE_CTRL_CK2_reg,~(REG_dfe_adapt_en_VTH_DFE_EN|REG_dfe_adapt_en_VTH_EN),0);

	}
//Stop limit value record
	hdmi_mask(REG_DFE_CTRL_B3_reg,~(DFE_P0_REG_DFE_CTRL_B3_P0_timer_en_b_mask),DFE_P0_REG_DFE_CTRL_B3_P0_recorder_limit_en_b(0));
	hdmi_mask(REG_DFE_CTRL_G3_reg,~(DFE_P0_REG_DFE_CTRL_G3_P0_timer_en_g_mask),DFE_P0_REG_DFE_CTRL_G3_P0_recorder_limit_en_g(0));
	hdmi_mask(REG_DFE_CTRL_R3_reg,~(DFE_P0_REG_DFE_CTRL_R3_P0_timer_en_r_mask),DFE_P0_REG_DFE_CTRL_R3_P0_recorder_limit_en_r(0));
	if (lane_mode == HDMI_4LANE)
	{
		hdmi_mask(REG_DFE_CTRL_CK3_reg,~(DFE_P0_REG_DFE_CTRL_CK3_P0_timer_en_ck_mask),DFE_P0_REG_DFE_CTRL_CK3_P0_recorder_limit_en_ck(0));
	}

}

void lib_hdmi_dfe_record_enable(unsigned char nport, unsigned char onoff)
{
	hdmi_mask(REG_DFE_CTRL_B3_reg,~(DFE_P0_REG_DFE_CTRL_B3_P0_timer_en_b_mask),DFE_P0_REG_DFE_CTRL_B3_P0_recorder_limit_en_b(onoff));
	hdmi_mask(REG_DFE_CTRL_G3_reg,~(DFE_P0_REG_DFE_CTRL_G3_P0_timer_en_g_mask),DFE_P0_REG_DFE_CTRL_G3_P0_recorder_limit_en_g(onoff));
	hdmi_mask(REG_DFE_CTRL_R3_reg,~(DFE_P0_REG_DFE_CTRL_R3_P0_timer_en_r_mask),DFE_P0_REG_DFE_CTRL_R3_P0_recorder_limit_en_r(onoff));
	hdmi_mask(REG_DFE_CTRL_CK3_reg,~(DFE_P0_REG_DFE_CTRL_CK3_P0_timer_en_ck_mask),DFE_P0_REG_DFE_CTRL_CK3_P0_recorder_limit_en_ck(onoff));
}

unsigned char lib_hdmi_dfe_is_record_enable(unsigned char nport)
{
	return DFE_P0_REG_DFE_CTRL_B3_P0_get_recorder_limit_en_b(hdmi_in(REG_DFE_CTRL_B3_reg));
}

void lib_hdmi_dfe_init_vth(unsigned char nport, unsigned char bgrc, unsigned char value)
{

	if (bgrc&LN_CK) {
	hdmi_mask(REG_DFE_CTRL_CKA_reg,~(DFE_P0_REG_DFE_CTRL_CKA_P0_vthp_init_ck_4_0_mask), (DFE_P0_REG_DFE_CTRL_CKA_P0_vthp_init_ck_4_0(value)));
	hdmi_mask(REG_DFE_CTRL_CKA_reg,~(DFE_P0_REG_DFE_CTRL_CKA_P0_vthn_init_ck_4_0_mask), (DFE_P0_REG_DFE_CTRL_CKA_P0_vthn_init_ck_4_0(value)));
	hdmi_mask(REG_DFE_CTRL_CK3_reg,~(DFE_P0_REG_DFE_CTRL_CK3_P0_vth_load_in_ck_mask), (DFE_P0_REG_DFE_CTRL_CK3_P0_vth_load_in_ck(1)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_CK3_reg,~(DFE_P0_REG_DFE_CTRL_CK3_P0_vth_load_in_ck_mask), (DFE_P0_REG_DFE_CTRL_CK3_P0_vth_load_in_ck(0)));
	}
	if (bgrc&LN_R) {
	hdmi_mask(REG_DFE_CTRL_RA_reg,~(DFE_P0_REG_DFE_CTRL_RA_P0_vthp_init_r_4_0_mask), (DFE_P0_REG_DFE_CTRL_RA_P0_vthp_init_r_4_0(value)));
	hdmi_mask(REG_DFE_CTRL_RA_reg,~(DFE_P0_REG_DFE_CTRL_RA_P0_vthn_init_r_4_0_mask), (DFE_P0_REG_DFE_CTRL_RA_P0_vthn_init_r_4_0(value)));
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_R3_P0_vth_load_in_r_mask), (DFE_P0_REG_DFE_CTRL_R3_P0_vth_load_in_r(1)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_R3_P0_vth_load_in_r_mask), (DFE_P0_REG_DFE_CTRL_R3_P0_vth_load_in_r(0)));
	}
	if (bgrc&LN_G) {
	hdmi_mask(REG_DFE_CTRL_GA_reg,~(DFE_P0_REG_DFE_CTRL_GA_P0_vthp_init_g_4_0_mask), (DFE_P0_REG_DFE_CTRL_GA_P0_vthp_init_g_4_0(value)));
	hdmi_mask(REG_DFE_CTRL_GA_reg,~(DFE_P0_REG_DFE_CTRL_GA_P0_vthn_init_g_4_0_mask), (DFE_P0_REG_DFE_CTRL_GA_P0_vthn_init_g_4_0(value)));
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_G3_P0_vth_load_in_g_mask), (DFE_P0_REG_DFE_CTRL_G3_P0_vth_load_in_g(1)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_G3_P0_vth_load_in_g_mask), (DFE_P0_REG_DFE_CTRL_G3_P0_vth_load_in_g(0)));
	}
	if (bgrc&LN_B) {

	hdmi_mask(REG_DFE_CTRL_BA_reg,~(DFE_P0_REG_DFE_CTRL_BA_P0_vthp_init_b_4_0_mask), (DFE_P0_REG_DFE_CTRL_BA_P0_vthp_init_b_4_0(value)));
	hdmi_mask(REG_DFE_CTRL_BA_reg,~(DFE_P0_REG_DFE_CTRL_BA_P0_vthn_init_b_4_0_mask), (DFE_P0_REG_DFE_CTRL_BA_P0_vthn_init_b_4_0(value)));
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_B3_P0_vth_load_in_b_mask), (DFE_P0_REG_DFE_CTRL_B3_P0_vth_load_in_b(1)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_B3_P0_vth_load_in_b_mask), (DFE_P0_REG_DFE_CTRL_B3_P0_vth_load_in_b(0)));

	}

}



void lib_hdmi_dfe_init_vthp(unsigned char nport, unsigned char bgrc, unsigned char value)
{

	if (bgrc&LN_CK) {
	hdmi_mask(REG_DFE_CTRL_CKA_reg,~(DFE_P0_REG_DFE_CTRL_CKA_P0_vthp_init_ck_4_0_mask), (DFE_P0_REG_DFE_CTRL_CKA_P0_vthp_init_ck_4_0(value)));
	hdmi_mask(REG_DFE_CTRL_CK3_reg,~(DFE_P0_REG_DFE_CTRL_CK3_P0_vth_load_in_ck_mask), (DFE_P0_REG_DFE_CTRL_CK3_P0_vth_load_in_ck(1)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_CK3_reg,~(DFE_P0_REG_DFE_CTRL_CK3_P0_vth_load_in_ck_mask), (DFE_P0_REG_DFE_CTRL_CK3_P0_vth_load_in_ck(0)));
	}
	if (bgrc&LN_R) {
	hdmi_mask(REG_DFE_CTRL_RA_reg,~(DFE_P0_REG_DFE_CTRL_RA_P0_vthp_init_r_4_0_mask), (DFE_P0_REG_DFE_CTRL_RA_P0_vthp_init_r_4_0(value)));
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_R3_P0_vth_load_in_r_mask), (DFE_P0_REG_DFE_CTRL_R3_P0_vth_load_in_r(1)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_R3_P0_vth_load_in_r_mask), (DFE_P0_REG_DFE_CTRL_R3_P0_vth_load_in_r(0)));
	}
	if (bgrc&LN_G) {
	hdmi_mask(REG_DFE_CTRL_GA_reg,~(DFE_P0_REG_DFE_CTRL_GA_P0_vthp_init_g_4_0_mask), (DFE_P0_REG_DFE_CTRL_GA_P0_vthp_init_g_4_0(value)));
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_G3_P0_vth_load_in_g_mask), (DFE_P0_REG_DFE_CTRL_G3_P0_vth_load_in_g(1)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_G3_P0_vth_load_in_g_mask), (DFE_P0_REG_DFE_CTRL_G3_P0_vth_load_in_g(0)));
	}
	if (bgrc&LN_B) {

	hdmi_mask(REG_DFE_CTRL_BA_reg,~(DFE_P0_REG_DFE_CTRL_BA_P0_vthp_init_b_4_0_mask), (DFE_P0_REG_DFE_CTRL_BA_P0_vthp_init_b_4_0(value)));
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_B3_P0_vth_load_in_b_mask), (DFE_P0_REG_DFE_CTRL_B3_P0_vth_load_in_b(1)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_B3_P0_vth_load_in_b_mask), (DFE_P0_REG_DFE_CTRL_B3_P0_vth_load_in_b(0)));

	}

}


void lib_hdmi_dfe_init_vthn(unsigned char nport, unsigned char bgrc, unsigned char value)
{

	if (bgrc&LN_CK) {
	hdmi_mask(REG_DFE_CTRL_CKA_reg,~(DFE_P0_REG_DFE_CTRL_CKA_P0_vthn_init_ck_4_0_mask), (DFE_P0_REG_DFE_CTRL_CKA_P0_vthn_init_ck_4_0(value)));
	hdmi_mask(REG_DFE_CTRL_CK3_reg,~(DFE_P0_REG_DFE_CTRL_CK3_P0_vth_load_in_ck_mask), (DFE_P0_REG_DFE_CTRL_CK3_P0_vth_load_in_ck(1)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_CK3_reg,~(DFE_P0_REG_DFE_CTRL_CK3_P0_vth_load_in_ck_mask), (DFE_P0_REG_DFE_CTRL_CK3_P0_vth_load_in_ck(0)));
	}
	if (bgrc&LN_R) {
	hdmi_mask(REG_DFE_CTRL_RA_reg,~(DFE_P0_REG_DFE_CTRL_RA_P0_vthn_init_r_4_0_mask), (DFE_P0_REG_DFE_CTRL_RA_P0_vthn_init_r_4_0(value)));
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_R3_P0_vth_load_in_r_mask), (DFE_P0_REG_DFE_CTRL_R3_P0_vth_load_in_r(1)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_G3_P0_vth_load_in_g_mask), (DFE_P0_REG_DFE_CTRL_R3_P0_vth_load_in_r(0)));
	}
	if (bgrc&LN_G) {
	hdmi_mask(REG_DFE_CTRL_GA_reg,~(DFE_P0_REG_DFE_CTRL_GA_P0_vthn_init_g_4_0_mask), (DFE_P0_REG_DFE_CTRL_GA_P0_vthn_init_g_4_0(value)));
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_G3_P0_vth_load_in_g_mask), (DFE_P0_REG_DFE_CTRL_G3_P0_vth_load_in_g(1)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_G3_P0_vth_load_in_g_mask), (DFE_P0_REG_DFE_CTRL_G3_P0_vth_load_in_g(0)));
	}
	if (bgrc&LN_B) {

	hdmi_mask(REG_DFE_CTRL_BA_reg,~(DFE_P0_REG_DFE_CTRL_BA_P0_vthn_init_b_4_0_mask), (DFE_P0_REG_DFE_CTRL_BA_P0_vthn_init_b_4_0(value)));
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_B3_P0_vth_load_in_b_mask), (DFE_P0_REG_DFE_CTRL_B3_P0_vth_load_in_b(1)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_B3_P0_vth_load_in_b_mask), (DFE_P0_REG_DFE_CTRL_B3_P0_vth_load_in_b(0)));

	}

}


void lib_hdmi_dfe_init_tap0(unsigned char nport, unsigned char bgrc, unsigned char value)
{

	if (bgrc&LN_CK) {
	hdmi_mask(REG_DFE_CTRL_CK9_reg,~(DFE_P0_REG_DFE_CTRL_CK9_P0_tap0_init_ck_7_0_mask), (DFE_P0_REG_DFE_CTRL_CK9_P0_tap0_init_ck_7_0(value)));
	hdmi_mask(REG_DFE_CTRL_CK3_reg,~(DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0_mask), (DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0(LOAD_IN_INIT_TAP0)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_CK3_reg,~(DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0_mask), (DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0(0)));
	}
	if (bgrc&LN_R) {
	hdmi_mask(REG_DFE_CTRL_R9_reg,	~(DFE_P0_REG_DFE_CTRL_R9_P0_tap0_init_r_7_0_mask), (DFE_P0_REG_DFE_CTRL_R9_P0_tap0_init_r_7_0(value)));
	hdmi_mask(REG_DFE_CTRL_R3_reg,	~(DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0_mask), (DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0(LOAD_IN_INIT_TAP0)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_R3_reg,	~(DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0_mask), (DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0(0)));
	}
	if (bgrc&LN_G) {
	hdmi_mask(REG_DFE_CTRL_G9_reg,~(DFE_P0_REG_DFE_CTRL_G9_P0_tap0_init_g_7_0_mask), (DFE_P0_REG_DFE_CTRL_G9_P0_tap0_init_g_7_0(value)));
	hdmi_mask(REG_DFE_CTRL_G3_reg,~(DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0_mask), (DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0(LOAD_IN_INIT_TAP0)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_G3_reg,~(DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0_mask), (DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0(0)));
	}
	if (bgrc&LN_B) {
	hdmi_mask(REG_DFE_CTRL_B9_reg,	~(DFE_P0_REG_DFE_CTRL_B9_P0_tap0_init_b_7_0_mask), (DFE_P0_REG_DFE_CTRL_B9_P0_tap0_init_b_7_0(value)));
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0_mask), (DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0(LOAD_IN_INIT_TAP0)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0_mask), (DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0(0)));
	}

}

void lib_hdmi_dfe_init_tap1(unsigned char nport, unsigned char bgrc, unsigned char value)
{

	if (bgrc&LN_CK) {
	hdmi_mask(REG_DFE_CTRL_CK9_reg,~(DFE_P0_REG_DFE_CTRL_CK9_P0_tap1_init_ck_5_0_mask), (DFE_P0_REG_DFE_CTRL_CK9_P0_tap1_init_ck_5_0(value)));
	hdmi_mask(REG_DFE_CTRL_CK3_reg,~(DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0_mask), (DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0(LOAD_IN_INIT_TAP1)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_CK3_reg,~(DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0_mask), (DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0(0)));
	}
	if (bgrc&LN_R) {
	hdmi_mask(REG_DFE_CTRL_R9_reg,	~(DFE_P0_REG_DFE_CTRL_R9_P0_tap1_init_r_5_0_mask), (DFE_P0_REG_DFE_CTRL_R9_P0_tap1_init_r_5_0(value)));
	hdmi_mask(REG_DFE_CTRL_R3_reg,	~(DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0_mask), (DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0(LOAD_IN_INIT_TAP1)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_R3_reg,	~(DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0_mask), (DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0(0)));
	}
	if (bgrc&LN_G) {
	hdmi_mask(REG_DFE_CTRL_G9_reg,~(DFE_P0_REG_DFE_CTRL_G9_P0_tap1_init_g_5_0_mask), (DFE_P0_REG_DFE_CTRL_G9_P0_tap1_init_g_5_0(value)));
	hdmi_mask(REG_DFE_CTRL_G3_reg,~(DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0_mask), (DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0(LOAD_IN_INIT_TAP1)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_G3_reg,~(DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0_mask), (DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0(0)));
	}
	if (bgrc&LN_B) {

	hdmi_mask(REG_DFE_CTRL_B9_reg,	~(DFE_P0_REG_DFE_CTRL_B9_P0_tap1_init_b_5_0_mask), (DFE_P0_REG_DFE_CTRL_B9_P0_tap1_init_b_5_0(value)));
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0_mask), (DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0(LOAD_IN_INIT_TAP1)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0_mask), (DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0(0)));

	}

}

void lib_hdmi_dfe_init_tap2(unsigned char nport, unsigned char bgrc, signed char value)
{

	if (value < 0) {
		value = 0x20|((-value)&0x1f);
	} else {
		value = (value&0x1f);
	}

	if (bgrc&LN_CK) {
	hdmi_mask(REG_DFE_CTRL_CK9_reg,~(DFE_P0_REG_DFE_CTRL_CK9_P0_tap2_init_ck_5_0_mask), (DFE_P0_REG_DFE_CTRL_CK9_P0_tap2_init_ck_5_0(value)));
	hdmi_mask(REG_DFE_CTRL_CK3_reg,~(DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0_mask), (DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0(LOAD_IN_INIT_TAP2)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_CK3_reg,~(DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0_mask), (DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0(0)));
	}
	if (bgrc&LN_R) {
	hdmi_mask(REG_DFE_CTRL_R9_reg,	~(DFE_P0_REG_DFE_CTRL_R9_P0_tap2_init_r_5_0_mask), (DFE_P0_REG_DFE_CTRL_R9_P0_tap2_init_r_5_0(value)));
	hdmi_mask(REG_DFE_CTRL_R3_reg,	~(DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0_mask), (DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0(LOAD_IN_INIT_TAP2)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_R3_reg,	~(DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0_mask), (DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0(0)));
	}
	if (bgrc&LN_G) {
	hdmi_mask(REG_DFE_CTRL_G9_reg,~(DFE_P0_REG_DFE_CTRL_G9_P0_tap2_init_g_5_0_mask), (DFE_P0_REG_DFE_CTRL_G9_P0_tap2_init_g_5_0(value)));
	hdmi_mask(REG_DFE_CTRL_G3_reg,~(DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0_mask), (DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0(LOAD_IN_INIT_TAP2)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_G3_reg,~(DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0_mask), (DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0(0)));
	}
	if (bgrc&LN_B) {

	hdmi_mask(REG_DFE_CTRL_B9_reg,	~(DFE_P0_REG_DFE_CTRL_B9_P0_tap2_init_b_5_0_mask), (DFE_P0_REG_DFE_CTRL_B9_P0_tap2_init_b_5_0(value)));
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0_mask), (DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0(LOAD_IN_INIT_TAP2)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0_mask), (DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0(0)));

	}


}


void lib_hdmi_dfe_init_tap3(unsigned char nport, unsigned char bgrc, signed char value)
{

	if (value < 0) {
		value = 0x20|((-value)&0x1f);
	} else {
		value = (value&0x1f);
	}

	if (bgrc&LN_CK) {
	hdmi_mask(REG_DFE_CTRL_CK9_reg,~(DFE_P0_REG_DFE_CTRL_CK9_P0_tap3_init_ck_5_0_mask), (DFE_P0_REG_DFE_CTRL_CK9_P0_tap3_init_ck_5_0(value)));
	hdmi_mask(REG_DFE_CTRL_CK3_reg,~(DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0_mask), (DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0(LOAD_IN_INIT_TAP3)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_CK3_reg,~(DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0_mask), (DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0(0)));
	}
	if (bgrc&LN_R) {
	hdmi_mask(REG_DFE_CTRL_R9_reg,	~(DFE_P0_REG_DFE_CTRL_R9_P0_tap3_init_r_5_0_mask), (DFE_P0_REG_DFE_CTRL_R9_P0_tap3_init_r_5_0(value)));
	hdmi_mask(REG_DFE_CTRL_R3_reg,	~(DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0_mask), (DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0(LOAD_IN_INIT_TAP3)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_R3_reg,	~(DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0_mask), (DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0(0)));
	}
	if (bgrc&LN_G) {
	hdmi_mask(REG_DFE_CTRL_G9_reg,~(DFE_P0_REG_DFE_CTRL_G9_P0_tap3_init_g_5_0_mask), (DFE_P0_REG_DFE_CTRL_G9_P0_tap3_init_g_5_0(value)));
	hdmi_mask(REG_DFE_CTRL_G3_reg,~(DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0_mask), (DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0(LOAD_IN_INIT_TAP3)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_G3_reg,~(DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0_mask), (DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0(0)));
	}
	if (bgrc&LN_B) {

	hdmi_mask(REG_DFE_CTRL_B9_reg,	~(DFE_P0_REG_DFE_CTRL_B9_P0_tap3_init_b_5_0_mask), (DFE_P0_REG_DFE_CTRL_B9_P0_tap3_init_b_5_0(value)));
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0_mask), (DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0(LOAD_IN_INIT_TAP3)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0_mask), (DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0(0)));

	}


}



void lib_hdmi_dfe_init_tap4(unsigned char nport, unsigned char bgrc, signed char value)
{

	if (value < 0) {
		value = 0x20|((-value)&0x1f);
	} else {
		value = (value&0x1f);
	}

	if (bgrc&LN_CK) {
	hdmi_mask(REG_DFE_CTRL_CK9_reg,~(DFE_P0_REG_DFE_CTRL_CK9_P0_tap4_init_ck_5_0_mask), (DFE_P0_REG_DFE_CTRL_CK9_P0_tap4_init_ck_5_0(value)));
	hdmi_mask(REG_DFE_CTRL_CK3_reg,~(DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0_mask), (DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0(LOAD_IN_INIT_TAP4)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_CK3_reg,~(DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0_mask), (DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0(0)));
	}
	if (bgrc&LN_R) {
	hdmi_mask(REG_DFE_CTRL_R9_reg,~(DFE_P0_REG_DFE_CTRL_R9_P0_tap4_init_r_5_0_mask), (DFE_P0_REG_DFE_CTRL_R9_P0_tap4_init_r_5_0(value)));
	hdmi_mask(REG_DFE_CTRL_R3_reg,	~(DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0_mask), (DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0(LOAD_IN_INIT_TAP4)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_R3_reg,	~(DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0_mask), (DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0(0)));
	}
	if (bgrc&LN_G) {
	hdmi_mask(REG_DFE_CTRL_G9_reg,~(DFE_P0_REG_DFE_CTRL_G9_P0_tap4_init_g_5_0_mask), (DFE_P0_REG_DFE_CTRL_G9_P0_tap4_init_g_5_0(value)));
	hdmi_mask(REG_DFE_CTRL_G3_reg,~(DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0_mask), (DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0(LOAD_IN_INIT_TAP4)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_G3_reg,~(DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0_mask), (DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0(0)));
	}
	if (bgrc&LN_B) {

	hdmi_mask(REG_DFE_CTRL_B9_reg,	~(DFE_P0_REG_DFE_CTRL_B9_P0_tap4_init_b_5_0_mask), (DFE_P0_REG_DFE_CTRL_B9_P0_tap4_init_b_5_0(value)));
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0_mask), (DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0(LOAD_IN_INIT_TAP4)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0_mask), (DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0(0)));

	}



}


void lib_hdmi_dfe_init_le(unsigned char nport, unsigned char bgrc, unsigned char value)
{
	if (bgrc&LN_CK) {
		hdmi_mask(REG_DFE_CTRL_CK8_reg,~(DFE_P0_REG_DFE_CTRL_CK8_P0_leq_init_ck_4_0_mask), (DFE_P0_REG_DFE_CTRL_CK8_P0_leq_init_ck_4_0(value)));
		hdmi_mask(REG_DFE_CTRL_CK2_reg,~(DFE_P0_REG_DFE_CTRL_CK2_P0_eq_selreg_ck_mask), (DFE_P0_REG_DFE_CTRL_CK2_P0_eq_selreg_ck(1)));
		udelay(1);
		hdmi_mask(REG_DFE_CTRL_CK2_reg,~(DFE_P0_REG_DFE_CTRL_CK2_P0_eq_selreg_ck_mask), (DFE_P0_REG_DFE_CTRL_CK2_P0_eq_selreg_ck(0)));
	}

	if (bgrc&LN_R) {
		hdmi_mask(REG_DFE_CTRL_R8_reg,	~(DFE_P0_REG_DFE_CTRL_R8_P0_leq_init_r_4_0_mask), (DFE_P0_REG_DFE_CTRL_R8_P0_leq_init_r_4_0(value)));
		hdmi_mask(REG_DFE_CTRL_R2_reg,	~(DFE_P0_REG_DFE_CTRL_R2_P0_eq_selreg_r_mask), (DFE_P0_REG_DFE_CTRL_R2_P0_eq_selreg_r(1)));
		udelay(1);
		hdmi_mask(REG_DFE_CTRL_R2_reg,	~(DFE_P0_REG_DFE_CTRL_R2_P0_eq_selreg_r_mask), (DFE_P0_REG_DFE_CTRL_R2_P0_eq_selreg_r(0)));
	}
	if (bgrc&LN_G) {
		hdmi_mask(REG_DFE_CTRL_G8_reg,~(DFE_P0_REG_DFE_CTRL_G8_P0_leq_init_g_4_0_mask), (DFE_P0_REG_DFE_CTRL_G8_P0_leq_init_g_4_0(value)));
		hdmi_mask(REG_DFE_CTRL_G2_reg,~(DFE_P0_REG_DFE_CTRL_G2_P0_eq_selreg_g_mask), (DFE_P0_REG_DFE_CTRL_G2_P0_eq_selreg_g(1)));
		udelay(1);
		hdmi_mask(REG_DFE_CTRL_G2_reg,~(DFE_P0_REG_DFE_CTRL_G2_P0_eq_selreg_g_mask), (DFE_P0_REG_DFE_CTRL_G2_P0_eq_selreg_g(0)));
	}
	if (bgrc&LN_B) {
		hdmi_mask(REG_DFE_CTRL_B8_reg,	~(DFE_P0_REG_DFE_CTRL_B8_P0_leq_init_b_4_0_mask), (DFE_P0_REG_DFE_CTRL_B8_P0_leq_init_b_4_0(value)));
		//load in
		hdmi_mask(REG_DFE_CTRL_B2_reg,	~(DFE_P0_REG_DFE_CTRL_B2_P0_eq_selreg_b_mask), (DFE_P0_REG_DFE_CTRL_B2_P0_eq_selreg_b(1)));
		udelay(1);
		hdmi_mask(REG_DFE_CTRL_B2_reg,	~(DFE_P0_REG_DFE_CTRL_B2_P0_eq_selreg_b_mask), (DFE_P0_REG_DFE_CTRL_B2_P0_eq_selreg_b(0)));
	}
}

void lib_hdmi_dfe_load_in_adp(unsigned char nport, unsigned char bgrc, unsigned int option)
{
#if 0
	if (bgrc&LN_CK) {
		if (nport < HDMI20_PORT)
			hdmi_mask(DFE_REG_DFE_INIT1_L3_reg, ~(option), (option));
	}
	if (bgrc&LN_R) {
		hdmi_mask(DFE_REG_DFE_INIT1_L2_reg, ~(option), (option));
	}
	if (bgrc&LN_G) {
		hdmi_mask(DFE_REG_DFE_INIT1_L1_reg, ~(option), (option));
	}
	if (bgrc&LN_B) {
		hdmi_mask(DFE_REG_DFE_INIT1_L0_reg, ~(option), (option));
	}

	udelay(1);
	if (bgrc&LN_CK) {
		if (nport < HDMI20_PORT)
			hdmi_mask(DFE_REG_DFE_INIT1_L3_reg, ~(option), 0);
	}
	if (bgrc&LN_R) {
		hdmi_mask(DFE_REG_DFE_INIT1_L2_reg, ~(option), 0);
	}
	if (bgrc&LN_G) {
		hdmi_mask(DFE_REG_DFE_INIT1_L1_reg, ~(option), 0);
	}
	if (bgrc&LN_B) {
		hdmi_mask(DFE_REG_DFE_INIT1_L0_reg, ~(option), 0);
	}
#endif	
}

unsigned char lib_hdmi_dfe_get_vth(unsigned char nport, unsigned char lane)
{
	//Same as get Vthp
	if(lane==0)//B
	{
		hdmi_mask(REG_DFE_CTRL_B5_reg, ~DFE_P0_REG_DFE_CTRL_B5_P0_debug_sel_b_3_0_mask, DFE_P0_REG_DFE_CTRL_B5_P0_debug_sel_b_3_0(COEF_SEL_VTHP_COEF));//0x0	LIMIT SEL
		return DFE_P0_REG_DFE_CTRL_BC_P0_get_dfe_coef_data_ro_b_7_0(hdmi_in(REG_DFE_CTRL_BC_reg));
	}
	else if(lane==1)//G
	{
		hdmi_mask(REG_DFE_CTRL_G5_reg, ~DFE_P0_REG_DFE_CTRL_G5_P0_debug_sel_g_3_0_mask, DFE_P0_REG_DFE_CTRL_G5_P0_debug_sel_g_3_0(COEF_SEL_VTHP_COEF));//0x0	LIMIT SEL
		return DFE_P0_REG_DFE_CTRL_GC_P0_get_dfe_coef_data_ro_g_7_0(hdmi_in(REG_DFE_CTRL_GC_reg));
	}
	else if(lane==2)//R
	{
		hdmi_mask(REG_DFE_CTRL_R5_reg, ~DFE_P0_REG_DFE_CTRL_R5_P0_debug_sel_r_3_0_mask, DFE_P0_REG_DFE_CTRL_R5_P0_debug_sel_r_3_0(COEF_SEL_VTHP_COEF));//0x0	LIMIT SEL
		return DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_coef_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg));
	}
	else if(lane==3)//CK
	{
		hdmi_mask(REG_DFE_CTRL_CK5_reg, ~DFE_P0_REG_DFE_CTRL_CK5_P0_debug_sel_ck_3_0_mask, DFE_P0_REG_DFE_CTRL_CK5_P0_debug_sel_ck_3_0(COEF_SEL_VTHP_COEF));//0x0	LIMIT SEL
		return DFE_P0_REG_DFE_CTRL_CKC_P0_get_dfe_coef_data_ro_ck_7_0(hdmi_in(REG_DFE_CTRL_CKC_reg));
	}
	return 0;
}



unsigned char lib_hdmi_dfe_get_vthp(unsigned char nport, unsigned char lane)
{
	if(lane==0)//B
	{
		hdmi_mask(REG_DFE_CTRL_B5_reg, ~DFE_P0_REG_DFE_CTRL_B5_P0_debug_sel_b_3_0_mask, DFE_P0_REG_DFE_CTRL_B5_P0_debug_sel_b_3_0(COEF_SEL_VTHP_COEF));//0x0	LIMIT SEL
		return DFE_P0_REG_DFE_CTRL_BC_P0_get_dfe_coef_data_ro_b_7_0(hdmi_in(REG_DFE_CTRL_BC_reg));
	}
	else if(lane==1)//G
	{
		hdmi_mask(REG_DFE_CTRL_G5_reg, ~DFE_P0_REG_DFE_CTRL_G5_P0_debug_sel_g_3_0_mask, DFE_P0_REG_DFE_CTRL_G5_P0_debug_sel_g_3_0(COEF_SEL_VTHP_COEF));//0x0	LIMIT SEL
		return DFE_P0_REG_DFE_CTRL_GC_P0_get_dfe_coef_data_ro_g_7_0(hdmi_in(REG_DFE_CTRL_GC_reg));
	}
	else if(lane==2)//R
	{
		hdmi_mask(REG_DFE_CTRL_R5_reg, ~DFE_P0_REG_DFE_CTRL_R5_P0_debug_sel_r_3_0_mask, DFE_P0_REG_DFE_CTRL_R5_P0_debug_sel_r_3_0(COEF_SEL_VTHP_COEF));//0x0	LIMIT SEL
		return DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_coef_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg));
	}
	else if(lane==3)//CK
	{
		hdmi_mask(REG_DFE_CTRL_CK5_reg, ~DFE_P0_REG_DFE_CTRL_CK5_P0_debug_sel_ck_3_0_mask, DFE_P0_REG_DFE_CTRL_CK5_P0_debug_sel_ck_3_0(COEF_SEL_VTHP_COEF));//0x0	LIMIT SEL
		return DFE_P0_REG_DFE_CTRL_CKC_P0_get_dfe_coef_data_ro_ck_7_0(hdmi_in(REG_DFE_CTRL_CKC_reg));
	}
	return 0;
}

unsigned char lib_hdmi_dfe_get_vthn(unsigned char nport, unsigned char lane)
{

	if(lane==0)//B
	{
		hdmi_mask(REG_DFE_CTRL_B5_reg, ~DFE_P0_REG_DFE_CTRL_B5_P0_debug_sel_b_3_0_mask, DFE_P0_REG_DFE_CTRL_B5_P0_debug_sel_b_3_0(COEF_SEL_VTHN_COEF));//
		return DFE_P0_REG_DFE_CTRL_BC_P0_get_dfe_coef_data_ro_b_7_0(hdmi_in(REG_DFE_CTRL_BC_reg));
	}
	else if(lane==1)//G
	{
		hdmi_mask(REG_DFE_CTRL_G5_reg, ~DFE_P0_REG_DFE_CTRL_G5_P0_debug_sel_g_3_0_mask, DFE_P0_REG_DFE_CTRL_G5_P0_debug_sel_g_3_0(COEF_SEL_VTHN_COEF));//
		return DFE_P0_REG_DFE_CTRL_GC_P0_get_dfe_coef_data_ro_g_7_0(hdmi_in(REG_DFE_CTRL_GC_reg));
	}
	else if(lane==2)//R
	{
		hdmi_mask(REG_DFE_CTRL_R5_reg, ~DFE_P0_REG_DFE_CTRL_R5_P0_debug_sel_r_3_0_mask, DFE_P0_REG_DFE_CTRL_R5_P0_debug_sel_r_3_0(COEF_SEL_VTHN_COEF));//
		return DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_coef_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg));
	}
	else if(lane==3)//CK
	{
		hdmi_mask(REG_DFE_CTRL_CK5_reg, ~DFE_P0_REG_DFE_CTRL_CK5_P0_debug_sel_ck_3_0_mask, DFE_P0_REG_DFE_CTRL_CK5_P0_debug_sel_ck_3_0(COEF_SEL_VTHN_COEF));//
		return DFE_P0_REG_DFE_CTRL_CKC_P0_get_dfe_coef_data_ro_ck_7_0(hdmi_in(REG_DFE_CTRL_CKC_reg));
	}

	return 0;
}



unsigned char lib_hdmi_dfe_get_tap0(unsigned char nport, unsigned char lane)
{


	if(lane==0)//B
	{
		hdmi_mask(REG_DFE_CTRL_B5_reg, ~DFE_P0_REG_DFE_CTRL_B5_P0_debug_sel_b_3_0_mask, DFE_P0_REG_DFE_CTRL_B5_P0_debug_sel_b_3_0(COEF_SEL_TAP0));//
		return DFE_P0_REG_DFE_CTRL_BC_P0_get_dfe_coef_data_ro_b_7_0(hdmi_in(REG_DFE_CTRL_BC_reg));
	}
	else if(lane==1)//G
	{
		hdmi_mask(REG_DFE_CTRL_G5_reg, ~DFE_P0_REG_DFE_CTRL_G5_P0_debug_sel_g_3_0_mask, DFE_P0_REG_DFE_CTRL_G5_P0_debug_sel_g_3_0(COEF_SEL_TAP0));//
		return DFE_P0_REG_DFE_CTRL_GC_P0_get_dfe_coef_data_ro_g_7_0(hdmi_in(REG_DFE_CTRL_GC_reg));
	}
	else if(lane==2)//R
	{
		hdmi_mask(REG_DFE_CTRL_R5_reg, ~DFE_P0_REG_DFE_CTRL_R5_P0_debug_sel_r_3_0_mask, DFE_P0_REG_DFE_CTRL_R5_P0_debug_sel_r_3_0(COEF_SEL_TAP0));//
		return DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_coef_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg));
	}
	else if(lane==3)//CK
	{
		hdmi_mask(REG_DFE_CTRL_CK5_reg, ~DFE_P0_REG_DFE_CTRL_CK5_P0_debug_sel_ck_3_0_mask, DFE_P0_REG_DFE_CTRL_CK5_P0_debug_sel_ck_3_0(COEF_SEL_TAP0));//
		return DFE_P0_REG_DFE_CTRL_CKC_P0_get_dfe_coef_data_ro_ck_7_0(hdmi_in(REG_DFE_CTRL_CKC_reg));
	}

	return 0;
}

unsigned char lib_hdmi_dfe_get_tap0max(unsigned char nport, unsigned char lane)
{

	if(lane==0)//B
	{
		hdmi_mask(REG_DFE_CTRL_B5_reg, ~DFE_P0_REG_DFE_CTRL_B5_P0_limit_sel_b_4_0_mask, DFE_P0_REG_DFE_CTRL_B5_P0_limit_sel_b_4_0(LIMIT_SEL_TAP0_MAX));//0x0	LIMIT SEL
		return DFE_P0_REG_DFE_CTRL_BC_P0_get_dfe_limit_data_ro_b_7_0(hdmi_in(REG_DFE_CTRL_BC_reg));
	}
	else if(lane==1)//G
	{
		hdmi_mask(REG_DFE_CTRL_G5_reg, ~DFE_P0_REG_DFE_CTRL_G5_P0_limit_sel_g_4_0_mask, DFE_P0_REG_DFE_CTRL_G5_P0_limit_sel_g_4_0(LIMIT_SEL_TAP0_MAX));//0x0	LIMIT SEL
		return DFE_P0_REG_DFE_CTRL_GC_P0_get_dfe_limit_data_ro_g_7_0(hdmi_in(REG_DFE_CTRL_GC_reg));
	}
	else if(lane==2)//R
	{
		hdmi_mask(REG_DFE_CTRL_R5_reg, ~DFE_P0_REG_DFE_CTRL_R5_P0_limit_sel_r_4_0_mask, DFE_P0_REG_DFE_CTRL_R5_P0_limit_sel_r_4_0(LIMIT_SEL_TAP0_MAX));//0x0	LIMIT SEL
		return DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_limit_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg));
	}
	else if(lane==3)//CK
	{
		hdmi_mask(REG_DFE_CTRL_CK5_reg, ~DFE_P0_REG_DFE_CTRL_CK5_P0_limit_sel_ck_4_0_mask, DFE_P0_REG_DFE_CTRL_CK5_P0_limit_sel_ck_4_0(LIMIT_SEL_TAP0_MAX));//0x0	LIMIT SEL
		return DFE_P0_REG_DFE_CTRL_CKC_P0_get_dfe_limit_data_ro_ck_7_0(hdmi_in(REG_DFE_CTRL_CKC_reg));
	}
	
	return 0;
}

unsigned char lib_hdmi_dfe_get_tap0min(unsigned char nport, unsigned char lane)
{
#if 0
	hdmi_mask(DFE_REG_DFE_READBACK_reg, ~DFE_P0_REG_DFE_READBACK_P0_limit_sel_p0_mask, DFE_P0_REG_DFE_READBACK_P0_limit_sel_p0(LIMIT_TAP0_MIN));
	hdmi_mask(DFE_REG_DFE_READBACK_reg, ~DFE_P0_REG_DFE_READBACK_P0_lane_sel_p0_mask, DFE_P0_REG_DFE_READBACK_P0_lane_sel_p0(lane));
	return DFE_P0_REG_DFE_READBACK_P0_get_limit_data_p0(hdmi_in(DFE_REG_DFE_READBACK_reg));
#endif
	return 0;
}


unsigned char lib_hdmi_dfe_get_tap1(unsigned char nport, unsigned char lane)
{
	if(lane==0)//B
	{
		hdmi_mask(REG_DFE_CTRL_B5_reg, ~DFE_P0_REG_DFE_CTRL_B5_P0_debug_sel_b_3_0_mask, DFE_P0_REG_DFE_CTRL_B5_P0_debug_sel_b_3_0(COEF_SEL_TAP1));//
		return DFE_P0_REG_DFE_CTRL_BC_P0_get_dfe_coef_data_ro_b_7_0(hdmi_in(REG_DFE_CTRL_BC_reg));
	}
	else if(lane==1)//G
	{
		hdmi_mask(REG_DFE_CTRL_G5_reg, ~DFE_P0_REG_DFE_CTRL_G5_P0_debug_sel_g_3_0_mask, DFE_P0_REG_DFE_CTRL_G5_P0_debug_sel_g_3_0(COEF_SEL_TAP1));//
		return DFE_P0_REG_DFE_CTRL_GC_P0_get_dfe_coef_data_ro_g_7_0(hdmi_in(REG_DFE_CTRL_GC_reg));
	}
	else if(lane==2)//R
	{
		hdmi_mask(REG_DFE_CTRL_R5_reg, ~DFE_P0_REG_DFE_CTRL_R5_P0_debug_sel_r_3_0_mask, DFE_P0_REG_DFE_CTRL_R5_P0_debug_sel_r_3_0(COEF_SEL_TAP1));//
		return DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_coef_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg));
	}
	else if(lane==3)//CK
	{
		hdmi_mask(REG_DFE_CTRL_CK5_reg, ~DFE_P0_REG_DFE_CTRL_CK5_P0_debug_sel_ck_3_0_mask, DFE_P0_REG_DFE_CTRL_CK5_P0_debug_sel_ck_3_0(COEF_SEL_TAP1));//
		return DFE_P0_REG_DFE_CTRL_CKC_P0_get_dfe_coef_data_ro_ck_7_0(hdmi_in(REG_DFE_CTRL_CKC_reg));
	}
	return 0;
}


unsigned char lib_hdmi_dfe_get_tap1max(unsigned char nport, unsigned char lane)
{
	if(lane==0)//B
	{
		hdmi_mask(REG_DFE_CTRL_B5_reg, ~DFE_P0_REG_DFE_CTRL_B5_P0_limit_sel_b_4_0_mask, DFE_P0_REG_DFE_CTRL_B5_P0_limit_sel_b_4_0(LIMIT_SEL_TAP1_MAX));//0x0	LIMIT SEL
		return DFE_P0_REG_DFE_CTRL_BC_P0_get_dfe_limit_data_ro_b_7_0(hdmi_in(REG_DFE_CTRL_BC_reg));
	}
	else if(lane==1)//G
	{
		hdmi_mask(REG_DFE_CTRL_G5_reg, ~DFE_P0_REG_DFE_CTRL_G5_P0_limit_sel_g_4_0_mask, DFE_P0_REG_DFE_CTRL_G5_P0_limit_sel_g_4_0(LIMIT_SEL_TAP1_MAX));//0x0	LIMIT SEL
		return DFE_P0_REG_DFE_CTRL_GC_P0_get_dfe_limit_data_ro_g_7_0(hdmi_in(REG_DFE_CTRL_GC_reg));
	}
	else if(lane==2)//R
	{
		hdmi_mask(REG_DFE_CTRL_R5_reg, ~DFE_P0_REG_DFE_CTRL_R5_P0_limit_sel_r_4_0_mask, DFE_P0_REG_DFE_CTRL_R5_P0_limit_sel_r_4_0(LIMIT_SEL_TAP1_MAX));//0x0	LIMIT SEL
		return DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_limit_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg));
	}
	else if(lane==3)//CK
	{
		hdmi_mask(REG_DFE_CTRL_CK5_reg, ~DFE_P0_REG_DFE_CTRL_CK5_P0_limit_sel_ck_4_0_mask, DFE_P0_REG_DFE_CTRL_CK5_P0_limit_sel_ck_4_0(LIMIT_SEL_TAP1_MAX));//0x0	LIMIT SEL
		return DFE_P0_REG_DFE_CTRL_CKC_P0_get_dfe_limit_data_ro_ck_7_0(hdmi_in(REG_DFE_CTRL_CKC_reg));
	}
	return 0;	
}

unsigned char lib_hdmi_dfe_get_tap1min(unsigned char nport, unsigned char lane)
{
	if(lane==0)//B
	{
		hdmi_mask(REG_DFE_CTRL_B5_reg, ~DFE_P0_REG_DFE_CTRL_B5_P0_limit_sel_b_4_0_mask, DFE_P0_REG_DFE_CTRL_B5_P0_limit_sel_b_4_0(LIMIT_SEL_TAP1_MIN));//0x0	LIMIT SEL
		return DFE_P0_REG_DFE_CTRL_BC_P0_get_dfe_limit_data_ro_b_7_0(hdmi_in(REG_DFE_CTRL_BC_reg));
	}
	else if(lane==1)//G
	{
		hdmi_mask(REG_DFE_CTRL_G5_reg, ~DFE_P0_REG_DFE_CTRL_G5_P0_limit_sel_g_4_0_mask, DFE_P0_REG_DFE_CTRL_G5_P0_limit_sel_g_4_0(LIMIT_SEL_TAP1_MIN));//0x0	LIMIT SEL
		return DFE_P0_REG_DFE_CTRL_GC_P0_get_dfe_limit_data_ro_g_7_0(hdmi_in(REG_DFE_CTRL_GC_reg));
	}
	else if(lane==2)//R
	{
		hdmi_mask(REG_DFE_CTRL_R5_reg, ~DFE_P0_REG_DFE_CTRL_R5_P0_limit_sel_r_4_0_mask, DFE_P0_REG_DFE_CTRL_R5_P0_limit_sel_r_4_0(LIMIT_SEL_TAP1_MIN));//0x0	LIMIT SEL
		return DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_limit_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg));
	}
	else if(lane==3)//CK
	{
		hdmi_mask(REG_DFE_CTRL_CK5_reg, ~DFE_P0_REG_DFE_CTRL_CK5_P0_limit_sel_ck_4_0_mask, DFE_P0_REG_DFE_CTRL_CK5_P0_limit_sel_ck_4_0(LIMIT_SEL_TAP1_MIN));//0x0	LIMIT SEL
		return DFE_P0_REG_DFE_CTRL_CKC_P0_get_dfe_limit_data_ro_ck_7_0(hdmi_in(REG_DFE_CTRL_CKC_reg));
	}

	return 0;	
}


char lib_hdmi_dfe_get_tap2(unsigned char nport, unsigned char lane)
{
	if(lane==0)//B
	{
		hdmi_mask(REG_DFE_CTRL_B5_reg, ~DFE_P0_REG_DFE_CTRL_B5_P0_debug_sel_b_3_0_mask, DFE_P0_REG_DFE_CTRL_B5_P0_debug_sel_b_3_0(COEF_SEL_TAP2));//
		return get_TAP2_coef_sign(hdmi_in(REG_DFE_CTRL_BC_reg))? \
				-get_TAP2_coef(DFE_P0_REG_DFE_CTRL_BC_P0_get_dfe_coef_data_ro_b_7_0(hdmi_in(REG_DFE_CTRL_BC_reg))):
				get_TAP2_coef(DFE_P0_REG_DFE_CTRL_BC_P0_get_dfe_coef_data_ro_b_7_0(hdmi_in(REG_DFE_CTRL_BC_reg)));
	}
	else if(lane==1)//G
	{
		hdmi_mask(REG_DFE_CTRL_G5_reg, ~DFE_P0_REG_DFE_CTRL_G5_P0_debug_sel_g_3_0_mask, DFE_P0_REG_DFE_CTRL_G5_P0_debug_sel_g_3_0(COEF_SEL_TAP2));//
		return get_TAP2_coef_sign(hdmi_in(REG_DFE_CTRL_GC_reg))? \
				-get_TAP2_coef(DFE_P0_REG_DFE_CTRL_GC_P0_get_dfe_coef_data_ro_g_7_0(hdmi_in(REG_DFE_CTRL_GC_reg))):
				get_TAP2_coef(DFE_P0_REG_DFE_CTRL_GC_P0_get_dfe_coef_data_ro_g_7_0(hdmi_in(REG_DFE_CTRL_GC_reg)));

	}
	else if(lane==2)//R
	{
		hdmi_mask(REG_DFE_CTRL_R5_reg, ~DFE_P0_REG_DFE_CTRL_R5_P0_debug_sel_r_3_0_mask, DFE_P0_REG_DFE_CTRL_R5_P0_debug_sel_r_3_0(COEF_SEL_TAP2));//
		return get_TAP2_coef_sign(hdmi_in(REG_DFE_CTRL_RC_reg))? \
				-get_TAP2_coef(DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_coef_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg))):
				get_TAP2_coef(DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_coef_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg)));

	}
	else if(lane==3)//CK
	{
		hdmi_mask(REG_DFE_CTRL_CK5_reg, ~DFE_P0_REG_DFE_CTRL_CK5_P0_debug_sel_ck_3_0_mask, DFE_P0_REG_DFE_CTRL_CK5_P0_debug_sel_ck_3_0(COEF_SEL_TAP2));//
		return get_TAP2_coef_sign(hdmi_in(REG_DFE_CTRL_CKC_reg))? \
				-get_TAP2_coef(DFE_P0_REG_DFE_CTRL_CKC_P0_get_dfe_coef_data_ro_ck_7_0(hdmi_in(REG_DFE_CTRL_CKC_reg))):
				get_TAP2_coef(DFE_P0_REG_DFE_CTRL_CKC_P0_get_dfe_coef_data_ro_ck_7_0(hdmi_in(REG_DFE_CTRL_CKC_reg)));

	}

	return 0;
}



char lib_hdmi_dfe_get_tap2max(unsigned char nport, unsigned char lane)
{
	if(lane==0)//B
	{
		hdmi_mask(REG_DFE_CTRL_B5_reg, ~DFE_P0_REG_DFE_CTRL_B5_P0_limit_sel_b_4_0_mask, DFE_P0_REG_DFE_CTRL_B5_P0_limit_sel_b_4_0(LIMIT_SEL_TAP2_MAX));//0x0	LIMIT SEL
		return get_TAP2_coef_sign(hdmi_in(REG_DFE_CTRL_BC_reg))? \
				-get_TAP2_coef(DFE_P0_REG_DFE_CTRL_BC_P0_get_dfe_limit_data_ro_b_7_0(hdmi_in(REG_DFE_CTRL_BC_reg))):
				get_TAP2_coef(DFE_P0_REG_DFE_CTRL_BC_P0_get_dfe_limit_data_ro_b_7_0(hdmi_in(REG_DFE_CTRL_BC_reg)));
	}
	else if(lane==1)//G
	{
		hdmi_mask(REG_DFE_CTRL_G5_reg, ~DFE_P0_REG_DFE_CTRL_G5_P0_limit_sel_g_4_0_mask, DFE_P0_REG_DFE_CTRL_G5_P0_limit_sel_g_4_0(LIMIT_SEL_TAP2_MAX));//0x0	LIMIT SEL
		return get_TAP2_coef_sign(hdmi_in(REG_DFE_CTRL_GC_reg))? \
				-get_TAP2_coef(DFE_P0_REG_DFE_CTRL_GC_P0_get_dfe_limit_data_ro_g_7_0(hdmi_in(REG_DFE_CTRL_GC_reg))):
				get_TAP2_coef(DFE_P0_REG_DFE_CTRL_GC_P0_get_dfe_limit_data_ro_g_7_0(hdmi_in(REG_DFE_CTRL_GC_reg)));
	}
	else if(lane==2)//R
	{
		hdmi_mask(REG_DFE_CTRL_R5_reg, ~DFE_P0_REG_DFE_CTRL_R5_P0_limit_sel_r_4_0_mask, DFE_P0_REG_DFE_CTRL_R5_P0_limit_sel_r_4_0(LIMIT_SEL_TAP2_MAX));//0x0	LIMIT SEL
		return get_TAP2_coef_sign(hdmi_in(REG_DFE_CTRL_RC_reg))? \
				-get_TAP2_coef(DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_limit_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg))):
				get_TAP2_coef(DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_limit_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg)));
	}
	else if(lane==3)//CK
	{
		hdmi_mask(REG_DFE_CTRL_CK5_reg, ~DFE_P0_REG_DFE_CTRL_CK5_P0_limit_sel_ck_4_0_mask, DFE_P0_REG_DFE_CTRL_CK5_P0_limit_sel_ck_4_0(LIMIT_SEL_TAP2_MAX));//0x0	LIMIT SEL
		return get_TAP2_coef_sign(hdmi_in(REG_DFE_CTRL_RC_reg))? \
				-get_TAP2_coef(DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_limit_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg))):
				get_TAP2_coef(DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_limit_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg)));
	}

	return 0;	
}

char lib_hdmi_dfe_get_tap2min(unsigned char nport, unsigned char lane)
{
	if(lane==0)//B
	{
		hdmi_mask(REG_DFE_CTRL_B5_reg, ~DFE_P0_REG_DFE_CTRL_B5_P0_limit_sel_b_4_0_mask, DFE_P0_REG_DFE_CTRL_B5_P0_limit_sel_b_4_0(LIMIT_SEL_TAP2_MIN));//0x0	LIMIT SEL
		return get_TAP2_coef_sign(hdmi_in(REG_DFE_CTRL_BC_reg))? \
				-get_TAP2_coef(DFE_P0_REG_DFE_CTRL_BC_P0_get_dfe_limit_data_ro_b_7_0(hdmi_in(REG_DFE_CTRL_BC_reg))):
				get_TAP2_coef(DFE_P0_REG_DFE_CTRL_BC_P0_get_dfe_limit_data_ro_b_7_0(hdmi_in(REG_DFE_CTRL_BC_reg)));
	}
	else if(lane==1)//G
	{
		hdmi_mask(REG_DFE_CTRL_G5_reg, ~DFE_P0_REG_DFE_CTRL_G5_P0_limit_sel_g_4_0_mask, DFE_P0_REG_DFE_CTRL_G5_P0_limit_sel_g_4_0(LIMIT_SEL_TAP2_MIN));//0x0	LIMIT SEL
		return get_TAP2_coef_sign(hdmi_in(REG_DFE_CTRL_GC_reg))? \
				-get_TAP2_coef(DFE_P0_REG_DFE_CTRL_GC_P0_get_dfe_limit_data_ro_g_7_0(hdmi_in(REG_DFE_CTRL_GC_reg))):
				get_TAP2_coef(DFE_P0_REG_DFE_CTRL_GC_P0_get_dfe_limit_data_ro_g_7_0(hdmi_in(REG_DFE_CTRL_GC_reg)));
	}
	else if(lane==2)//R
	{
		hdmi_mask(REG_DFE_CTRL_R5_reg, ~DFE_P0_REG_DFE_CTRL_R5_P0_limit_sel_r_4_0_mask, DFE_P0_REG_DFE_CTRL_R5_P0_limit_sel_r_4_0(LIMIT_SEL_TAP2_MIN));//0x0	LIMIT SEL
		return get_TAP2_coef_sign(hdmi_in(REG_DFE_CTRL_RC_reg))? \
				-get_TAP2_coef(DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_limit_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg))):
				get_TAP2_coef(DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_limit_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg)));
	}
	else if(lane==3)//CK
	{
		hdmi_mask(REG_DFE_CTRL_CK5_reg, ~DFE_P0_REG_DFE_CTRL_CK5_P0_limit_sel_ck_4_0_mask, DFE_P0_REG_DFE_CTRL_CK5_P0_limit_sel_ck_4_0(LIMIT_SEL_TAP2_MIN));//0x0	LIMIT SEL
		return get_TAP2_coef_sign(hdmi_in(REG_DFE_CTRL_RC_reg))? \
				-get_TAP2_coef(DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_limit_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg))):
				get_TAP2_coef(DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_limit_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg)));
	}

	return 0;	
}


char lib_hdmi_dfe_get_tap3(unsigned char nport, unsigned char lane)
{
	if(lane==0)//B
	{
		hdmi_mask(REG_DFE_CTRL_B5_reg, ~DFE_P0_REG_DFE_CTRL_B5_P0_debug_sel_b_3_0_mask, DFE_P0_REG_DFE_CTRL_B5_P0_debug_sel_b_3_0(COEF_SEL_TAP3));//
		return get_TAP3_coef_sign(hdmi_in(REG_DFE_CTRL_BC_reg))? \
			-get_TAP3_coef(DFE_P0_REG_DFE_CTRL_BC_P0_get_dfe_coef_data_ro_b_7_0(hdmi_in(REG_DFE_CTRL_BC_reg))):
			get_TAP3_coef(DFE_P0_REG_DFE_CTRL_BC_P0_get_dfe_coef_data_ro_b_7_0(hdmi_in(REG_DFE_CTRL_BC_reg)));
	
	}
	else if(lane==1)//G
	{
		hdmi_mask(REG_DFE_CTRL_G5_reg, ~DFE_P0_REG_DFE_CTRL_G5_P0_debug_sel_g_3_0_mask, DFE_P0_REG_DFE_CTRL_G5_P0_debug_sel_g_3_0(COEF_SEL_TAP3));//
		return get_TAP3_coef_sign(hdmi_in(REG_DFE_CTRL_GC_reg))? \
			-get_TAP3_coef(DFE_P0_REG_DFE_CTRL_GC_P0_get_dfe_coef_data_ro_g_7_0(hdmi_in(REG_DFE_CTRL_GC_reg))):
			get_TAP3_coef(DFE_P0_REG_DFE_CTRL_GC_P0_get_dfe_coef_data_ro_g_7_0(hdmi_in(REG_DFE_CTRL_GC_reg)));		
	}
	else if(lane==2)//R
	{
		hdmi_mask(REG_DFE_CTRL_R5_reg, ~DFE_P0_REG_DFE_CTRL_R5_P0_debug_sel_r_3_0_mask, DFE_P0_REG_DFE_CTRL_R5_P0_debug_sel_r_3_0(COEF_SEL_TAP3));//
		return get_TAP3_coef_sign(hdmi_in(REG_DFE_CTRL_RC_reg))? \
			-get_TAP3_coef(DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_coef_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg))):
			get_TAP3_coef(DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_coef_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg)));
	}
	else if(lane==3)//CK
	{
		hdmi_mask(REG_DFE_CTRL_CK5_reg, ~DFE_P0_REG_DFE_CTRL_CK5_P0_debug_sel_ck_3_0_mask, DFE_P0_REG_DFE_CTRL_CK5_P0_debug_sel_ck_3_0(COEF_SEL_TAP3));//
		return get_TAP3_coef_sign(hdmi_in(REG_DFE_CTRL_CKC_reg))? \
			-get_TAP3_coef(DFE_P0_REG_DFE_CTRL_CKC_P0_get_dfe_coef_data_ro_ck_7_0(hdmi_in(REG_DFE_CTRL_CKC_reg))):
			get_TAP3_coef(DFE_P0_REG_DFE_CTRL_CKC_P0_get_dfe_coef_data_ro_ck_7_0(hdmi_in(REG_DFE_CTRL_CKC_reg)));
	}

	return 0;
}


char lib_hdmi_dfe_get_tap3max(unsigned char nport, unsigned char lane)
{
	if(lane==0)//B
	{
		hdmi_mask(REG_DFE_CTRL_B5_reg, ~DFE_P0_REG_DFE_CTRL_B5_P0_limit_sel_b_4_0_mask, DFE_P0_REG_DFE_CTRL_B5_P0_limit_sel_b_4_0(LIMIT_SEL_TAP3_MAX));//0x0	LIMIT SEL
		return get_TAP2_coef_sign(hdmi_in(REG_DFE_CTRL_BC_reg))? \
				-get_TAP2_coef(DFE_P0_REG_DFE_CTRL_BC_P0_get_dfe_limit_data_ro_b_7_0(hdmi_in(REG_DFE_CTRL_BC_reg))):
				get_TAP2_coef(DFE_P0_REG_DFE_CTRL_BC_P0_get_dfe_limit_data_ro_b_7_0(hdmi_in(REG_DFE_CTRL_BC_reg)));
	}
	else if(lane==1)//G
	{
		hdmi_mask(REG_DFE_CTRL_G5_reg, ~DFE_P0_REG_DFE_CTRL_G5_P0_limit_sel_g_4_0_mask, DFE_P0_REG_DFE_CTRL_G5_P0_limit_sel_g_4_0(LIMIT_SEL_TAP3_MAX));//0x0	LIMIT SEL
		return get_TAP2_coef_sign(hdmi_in(REG_DFE_CTRL_GC_reg))? \
				-get_TAP2_coef(DFE_P0_REG_DFE_CTRL_GC_P0_get_dfe_limit_data_ro_g_7_0(hdmi_in(REG_DFE_CTRL_GC_reg))):
				get_TAP2_coef(DFE_P0_REG_DFE_CTRL_GC_P0_get_dfe_limit_data_ro_g_7_0(hdmi_in(REG_DFE_CTRL_GC_reg)));
	}
	else if(lane==2)//R
	{
		hdmi_mask(REG_DFE_CTRL_R5_reg, ~DFE_P0_REG_DFE_CTRL_R5_P0_limit_sel_r_4_0_mask, DFE_P0_REG_DFE_CTRL_R5_P0_limit_sel_r_4_0(LIMIT_SEL_TAP3_MAX));//0x0	LIMIT SEL
		return get_TAP2_coef_sign(hdmi_in(REG_DFE_CTRL_RC_reg))? \
				-get_TAP2_coef(DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_limit_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg))):
				get_TAP2_coef(DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_limit_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg)));
	}
	else if(lane==3)//CK
	{
		hdmi_mask(REG_DFE_CTRL_CK5_reg, ~DFE_P0_REG_DFE_CTRL_CK5_P0_limit_sel_ck_4_0_mask, DFE_P0_REG_DFE_CTRL_CK5_P0_limit_sel_ck_4_0(LIMIT_SEL_TAP3_MAX));//0x0	LIMIT SEL
		return get_TAP2_coef_sign(hdmi_in(REG_DFE_CTRL_RC_reg))? \
				-get_TAP2_coef(DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_limit_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg))):
				get_TAP2_coef(DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_limit_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg)));
	}

	return 0;	
}

char lib_hdmi_dfe_get_tap3min(unsigned char nport, unsigned char lane)
{
	if(lane==0)//B
	{
		hdmi_mask(REG_DFE_CTRL_B5_reg, ~DFE_P0_REG_DFE_CTRL_B5_P0_limit_sel_b_4_0_mask, DFE_P0_REG_DFE_CTRL_B5_P0_limit_sel_b_4_0(LIMIT_SEL_TAP3_MIN));//0x0	LIMIT SEL
		return get_TAP2_coef_sign(hdmi_in(REG_DFE_CTRL_BC_reg))? \
				-get_TAP2_coef(DFE_P0_REG_DFE_CTRL_BC_P0_get_dfe_limit_data_ro_b_7_0(hdmi_in(REG_DFE_CTRL_BC_reg))):
				get_TAP2_coef(DFE_P0_REG_DFE_CTRL_BC_P0_get_dfe_limit_data_ro_b_7_0(hdmi_in(REG_DFE_CTRL_BC_reg)));
	}
	else if(lane==1)//G
	{
		hdmi_mask(REG_DFE_CTRL_G5_reg, ~DFE_P0_REG_DFE_CTRL_G5_P0_limit_sel_g_4_0_mask, DFE_P0_REG_DFE_CTRL_G5_P0_limit_sel_g_4_0(LIMIT_SEL_TAP3_MIN));//0x0	LIMIT SEL
		return get_TAP2_coef_sign(hdmi_in(REG_DFE_CTRL_GC_reg))? \
				-get_TAP2_coef(DFE_P0_REG_DFE_CTRL_GC_P0_get_dfe_limit_data_ro_g_7_0(hdmi_in(REG_DFE_CTRL_GC_reg))):
				get_TAP2_coef(DFE_P0_REG_DFE_CTRL_GC_P0_get_dfe_limit_data_ro_g_7_0(hdmi_in(REG_DFE_CTRL_GC_reg)));
	}
	else if(lane==2)//R
	{
		hdmi_mask(REG_DFE_CTRL_R5_reg, ~DFE_P0_REG_DFE_CTRL_R5_P0_limit_sel_r_4_0_mask, DFE_P0_REG_DFE_CTRL_R5_P0_limit_sel_r_4_0(LIMIT_SEL_TAP3_MIN));//0x0	LIMIT SEL
		return get_TAP2_coef_sign(hdmi_in(REG_DFE_CTRL_RC_reg))? \
				-get_TAP2_coef(DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_limit_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg))):
				get_TAP2_coef(DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_limit_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg)));
	}
	else if(lane==3)//CK
	{
		hdmi_mask(REG_DFE_CTRL_CK5_reg, ~DFE_P0_REG_DFE_CTRL_CK5_P0_limit_sel_ck_4_0_mask, DFE_P0_REG_DFE_CTRL_CK5_P0_limit_sel_ck_4_0(LIMIT_SEL_TAP3_MIN));//0x0	LIMIT SEL
		return get_TAP2_coef_sign(hdmi_in(REG_DFE_CTRL_RC_reg))? \
				-get_TAP2_coef(DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_limit_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg))):
				get_TAP2_coef(DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_limit_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg)));
	}

	return 0;	
}



char lib_hdmi_dfe_get_tap4(unsigned char nport, unsigned char lane)
{
	if(lane==0)//B
	{
		hdmi_mask(REG_DFE_CTRL_B5_reg, ~DFE_P0_REG_DFE_CTRL_B5_P0_debug_sel_b_3_0_mask, DFE_P0_REG_DFE_CTRL_B5_P0_debug_sel_b_3_0(COEF_SEL_TAP4));//
		return get_TAP4_coef_sign(hdmi_in(REG_DFE_CTRL_BC_reg))? \
		-get_TAP4_coef(DFE_P0_REG_DFE_CTRL_BC_P0_get_dfe_coef_data_ro_b_7_0(hdmi_in(REG_DFE_CTRL_BC_reg))):
		get_TAP4_coef(DFE_P0_REG_DFE_CTRL_BC_P0_get_dfe_coef_data_ro_b_7_0(hdmi_in(REG_DFE_CTRL_BC_reg)));
	}
	else if(lane==1)//G
	{
		hdmi_mask(REG_DFE_CTRL_G5_reg, ~DFE_P0_REG_DFE_CTRL_G5_P0_debug_sel_g_3_0_mask, DFE_P0_REG_DFE_CTRL_G5_P0_debug_sel_g_3_0(COEF_SEL_TAP4));//
		return get_TAP4_coef_sign(hdmi_in(REG_DFE_CTRL_GC_reg))? \
		-get_TAP4_coef(DFE_P0_REG_DFE_CTRL_GC_P0_get_dfe_coef_data_ro_g_7_0(hdmi_in(REG_DFE_CTRL_GC_reg))):
		get_TAP4_coef(DFE_P0_REG_DFE_CTRL_GC_P0_get_dfe_coef_data_ro_g_7_0(hdmi_in(REG_DFE_CTRL_GC_reg)));
	}
	else if(lane==2)//R
	{
		hdmi_mask(REG_DFE_CTRL_R5_reg, ~DFE_P0_REG_DFE_CTRL_R5_P0_debug_sel_r_3_0_mask, DFE_P0_REG_DFE_CTRL_R5_P0_debug_sel_r_3_0(COEF_SEL_TAP4));//
		return get_TAP4_coef_sign(hdmi_in(REG_DFE_CTRL_RC_reg))? \
		-get_TAP4_coef(DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_coef_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg))):
		get_TAP4_coef(DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_coef_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg)));
	}
	else if(lane==3)//CK
	{
		hdmi_mask(REG_DFE_CTRL_CK5_reg, ~DFE_P0_REG_DFE_CTRL_CK5_P0_debug_sel_ck_3_0_mask, DFE_P0_REG_DFE_CTRL_CK5_P0_debug_sel_ck_3_0(COEF_SEL_TAP4));//
		return get_TAP4_coef_sign(hdmi_in(REG_DFE_CTRL_CKC_reg))? \
		-get_TAP4_coef(DFE_P0_REG_DFE_CTRL_CKC_P0_get_dfe_coef_data_ro_ck_7_0(hdmi_in(REG_DFE_CTRL_CKC_reg))):
		get_TAP4_coef(DFE_P0_REG_DFE_CTRL_CKC_P0_get_dfe_coef_data_ro_ck_7_0(hdmi_in(REG_DFE_CTRL_CKC_reg)));
	}
	return 0;
}



char lib_hdmi_dfe_get_tap4max(unsigned char nport, unsigned char lane)
{
	if(lane==0)//B
	{
		hdmi_mask(REG_DFE_CTRL_B5_reg, ~DFE_P0_REG_DFE_CTRL_B5_P0_limit_sel_b_4_0_mask, DFE_P0_REG_DFE_CTRL_B5_P0_limit_sel_b_4_0(LIMIT_SEL_TAP4_MAX));//0x0	LIMIT SEL
		return get_TAP2_coef_sign(hdmi_in(REG_DFE_CTRL_BC_reg))? \
				-get_TAP2_coef(DFE_P0_REG_DFE_CTRL_BC_P0_get_dfe_limit_data_ro_b_7_0(hdmi_in(REG_DFE_CTRL_BC_reg))):
				get_TAP2_coef(DFE_P0_REG_DFE_CTRL_BC_P0_get_dfe_limit_data_ro_b_7_0(hdmi_in(REG_DFE_CTRL_BC_reg)));
	}
	else if(lane==1)//G
	{
		hdmi_mask(REG_DFE_CTRL_G5_reg, ~DFE_P0_REG_DFE_CTRL_G5_P0_limit_sel_g_4_0_mask, DFE_P0_REG_DFE_CTRL_G5_P0_limit_sel_g_4_0(LIMIT_SEL_TAP4_MAX));//0x0	LIMIT SEL
		return get_TAP2_coef_sign(hdmi_in(REG_DFE_CTRL_GC_reg))? \
				-get_TAP2_coef(DFE_P0_REG_DFE_CTRL_GC_P0_get_dfe_limit_data_ro_g_7_0(hdmi_in(REG_DFE_CTRL_GC_reg))):
				get_TAP2_coef(DFE_P0_REG_DFE_CTRL_GC_P0_get_dfe_limit_data_ro_g_7_0(hdmi_in(REG_DFE_CTRL_GC_reg)));
	}
	else if(lane==2)//R
	{
		hdmi_mask(REG_DFE_CTRL_R5_reg, ~DFE_P0_REG_DFE_CTRL_R5_P0_limit_sel_r_4_0_mask, DFE_P0_REG_DFE_CTRL_R5_P0_limit_sel_r_4_0(LIMIT_SEL_TAP4_MAX));//0x0	LIMIT SEL
		return get_TAP2_coef_sign(hdmi_in(REG_DFE_CTRL_RC_reg))? \
				-get_TAP2_coef(DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_limit_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg))):
				get_TAP2_coef(DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_limit_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg)));
	}
	else if(lane==3)//CK
	{
		hdmi_mask(REG_DFE_CTRL_CK5_reg, ~DFE_P0_REG_DFE_CTRL_CK5_P0_limit_sel_ck_4_0_mask, DFE_P0_REG_DFE_CTRL_CK5_P0_limit_sel_ck_4_0(LIMIT_SEL_TAP4_MAX));//0x0	LIMIT SEL
		return get_TAP2_coef_sign(hdmi_in(REG_DFE_CTRL_RC_reg))? \
				-get_TAP2_coef(DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_limit_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg))):
				get_TAP2_coef(DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_limit_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg)));
	}

	return 0;	
}

char lib_hdmi_dfe_get_tap4min(unsigned char nport, unsigned char lane)
{
	if(lane==0)//B
	{
		hdmi_mask(REG_DFE_CTRL_B5_reg, ~DFE_P0_REG_DFE_CTRL_B5_P0_limit_sel_b_4_0_mask, DFE_P0_REG_DFE_CTRL_B5_P0_limit_sel_b_4_0(LIMIT_SEL_TAP4_MIN));//0x0	LIMIT SEL
		return get_TAP2_coef_sign(hdmi_in(REG_DFE_CTRL_BC_reg))? \
				-get_TAP2_coef(DFE_P0_REG_DFE_CTRL_BC_P0_get_dfe_limit_data_ro_b_7_0(hdmi_in(REG_DFE_CTRL_BC_reg))):
				get_TAP2_coef(DFE_P0_REG_DFE_CTRL_BC_P0_get_dfe_limit_data_ro_b_7_0(hdmi_in(REG_DFE_CTRL_BC_reg)));
	}
	else if(lane==1)//G
	{
		hdmi_mask(REG_DFE_CTRL_G5_reg, ~DFE_P0_REG_DFE_CTRL_G5_P0_limit_sel_g_4_0_mask, DFE_P0_REG_DFE_CTRL_G5_P0_limit_sel_g_4_0(LIMIT_SEL_TAP4_MIN));//0x0	LIMIT SEL
		return get_TAP2_coef_sign(hdmi_in(REG_DFE_CTRL_GC_reg))? \
				-get_TAP2_coef(DFE_P0_REG_DFE_CTRL_GC_P0_get_dfe_limit_data_ro_g_7_0(hdmi_in(REG_DFE_CTRL_GC_reg))):
				get_TAP2_coef(DFE_P0_REG_DFE_CTRL_GC_P0_get_dfe_limit_data_ro_g_7_0(hdmi_in(REG_DFE_CTRL_GC_reg)));
	}
	else if(lane==2)//R
	{
		hdmi_mask(REG_DFE_CTRL_R5_reg, ~DFE_P0_REG_DFE_CTRL_R5_P0_limit_sel_r_4_0_mask, DFE_P0_REG_DFE_CTRL_R5_P0_limit_sel_r_4_0(LIMIT_SEL_TAP4_MIN));//0x0	LIMIT SEL
		return get_TAP2_coef_sign(hdmi_in(REG_DFE_CTRL_RC_reg))? \
				-get_TAP2_coef(DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_limit_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg))):
				get_TAP2_coef(DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_limit_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg)));
	}
	else if(lane==3)//CK
	{
		hdmi_mask(REG_DFE_CTRL_CK5_reg, ~DFE_P0_REG_DFE_CTRL_CK5_P0_limit_sel_ck_4_0_mask, DFE_P0_REG_DFE_CTRL_CK5_P0_limit_sel_ck_4_0(LIMIT_SEL_TAP4_MIN));//0x0	LIMIT SEL
		return get_TAP2_coef_sign(hdmi_in(REG_DFE_CTRL_RC_reg))? \
				-get_TAP2_coef(DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_limit_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg))):
				get_TAP2_coef(DFE_P0_REG_DFE_CTRL_RC_P0_get_dfe_limit_data_ro_r_7_0(hdmi_in(REG_DFE_CTRL_RC_reg)));
	}

	return 0;	
}




unsigned char lib_hdmi_dfe_get_le(unsigned char nport, unsigned char lane)
{
	if(lane==0)//B
	{
		hdmi_mask(REG_DFE_CTRL_B5_reg, ~DFE_P0_REG_DFE_CTRL_B5_P0_debug_sel_b_3_0_mask, DFE_P0_REG_DFE_CTRL_B5_P0_debug_sel_b_3_0(COEF_SEL_LE));//
		return rtd_part_inl(REG_DFE_CTRL_BC_reg, 15, 10);
	}
	else if(lane==1)//G
	{
		hdmi_mask(REG_DFE_CTRL_G5_reg, ~DFE_P0_REG_DFE_CTRL_G5_P0_debug_sel_g_3_0_mask, DFE_P0_REG_DFE_CTRL_G5_P0_debug_sel_g_3_0(COEF_SEL_LE));//
		return rtd_part_inl(REG_DFE_CTRL_GC_reg, 15, 10);
	}
	else if(lane==2)//R
	{
		hdmi_mask(REG_DFE_CTRL_R5_reg, ~DFE_P0_REG_DFE_CTRL_R5_P0_debug_sel_r_3_0_mask, DFE_P0_REG_DFE_CTRL_R5_P0_debug_sel_r_3_0(COEF_SEL_LE));//
		return rtd_part_inl(REG_DFE_CTRL_RC_reg, 15, 10);
	}
	else if(lane==3)//CK
	{
		hdmi_mask(REG_DFE_CTRL_CK5_reg, ~DFE_P0_REG_DFE_CTRL_CK5_P0_debug_sel_ck_3_0_mask, DFE_P0_REG_DFE_CTRL_CK5_P0_debug_sel_ck_3_0(COEF_SEL_LE));//
		return rtd_part_inl(REG_DFE_CTRL_CKC_reg, 15, 10);
	}
	return 0;
}

unsigned char lib_hdmi_dfe_get_lemax(unsigned char nport, unsigned char lane)
{
#if 0
	hdmi_mask(DFE_REG_DFE_READBACK_reg, ~DFE_P0_REG_DFE_READBACK_P0_limit_sel_p0_mask, DFE_P0_REG_DFE_READBACK_P0_limit_sel_p0(LIMIT_LE_MAX));
	hdmi_mask(DFE_REG_DFE_READBACK_reg, ~DFE_P0_REG_DFE_READBACK_P0_lane_sel_p0_mask, DFE_P0_REG_DFE_READBACK_P0_lane_sel_p0(lane));
	return DFE_P0_REG_DFE_READBACK_P0_get_limit_data_p0(hdmi_in(DFE_REG_DFE_READBACK_reg));
#endif
	return 0;
}

unsigned char lib_hdmi_dfe_get_lemin(unsigned char nport, unsigned char lane)
{
#if 0
	hdmi_mask(DFE_REG_DFE_READBACK_reg, ~DFE_P0_REG_DFE_READBACK_P0_limit_sel_p0_mask, DFE_P0_REG_DFE_READBACK_P0_limit_sel_p0(LIMIT_LE_MIN));
	hdmi_mask(DFE_REG_DFE_READBACK_reg, ~DFE_P0_REG_DFE_READBACK_P0_lane_sel_p0_mask, DFE_P0_REG_DFE_READBACK_P0_lane_sel_p0(lane));
	return DFE_P0_REG_DFE_READBACK_P0_get_limit_data_p0(hdmi_in(DFE_REG_DFE_READBACK_reg));
#endif
	return 0;
}

void lib_hdmi_dfe_update_le(unsigned char nport, unsigned char eq_ch0, unsigned char eq_ch1, unsigned char eq_ch2)
{

	unsigned char bgrc = (LN_R|LN_G|LN_B);
	unsigned char mode = DFE_P0_REG_DFE_CTRL_RB_P0_dfe_adapt_mode_r_1_0(hdmi_in(REG_DFE_CTRL_RB_reg));

	if (mode == 2)
	{
		/*
		if (bgrc&LN_CK) {
			hdmi_mask(REG_DFE_CTRL_CK8_reg,
				~(DFE_P0_REG_DFE_CTRL_CK8_P0_leq_init_ck_4_0_mask), (DFE_P0_REG_DFE_CTRL_CK8_P0_leq_init_ck_4_0(value)));
			hdmi_mask(REG_DFE_CTRL_CK2_reg,
				~(DFE_P0_REG_DFE_CTRL_CK2_P0_eq_selreg_ck_mask), (DFE_P0_REG_DFE_CTRL_CK2_P0_eq_selreg_ck(1)));
			udelay(1);
			hdmi_mask(REG_DFE_CTRL_CK2_reg,
				~(DFE_P0_REG_DFE_CTRL_CK2_P0_eq_selreg_ck_mask), (DFE_P0_REG_DFE_CTRL_CK2_P0_eq_selreg_ck(0)));
		}
		*/
		if (bgrc&LN_R) {
			hdmi_mask(REG_DFE_CTRL_R8_reg,
				~(DFE_P0_REG_DFE_CTRL_R8_P0_leq_init_r_4_0_mask), (DFE_P0_REG_DFE_CTRL_R8_P0_leq_init_r_4_0(eq_ch2)));
			hdmi_mask(REG_DFE_CTRL_R2_reg,
				~(DFE_P0_REG_DFE_CTRL_R2_P0_eq_selreg_r_mask), (DFE_P0_REG_DFE_CTRL_R2_P0_eq_selreg_r(1)));
			udelay(1);
			hdmi_mask(REG_DFE_CTRL_R2_reg,
				~(DFE_P0_REG_DFE_CTRL_R2_P0_eq_selreg_r_mask), (DFE_P0_REG_DFE_CTRL_R2_P0_eq_selreg_r(0)));
		}
		if (bgrc&LN_G) {
			hdmi_mask(REG_DFE_CTRL_G8_reg,
				~(DFE_P0_REG_DFE_CTRL_G8_P0_leq_init_g_4_0_mask), (DFE_P0_REG_DFE_CTRL_G8_P0_leq_init_g_4_0(eq_ch1)));
			hdmi_mask(REG_DFE_CTRL_G2_reg,
				~(DFE_P0_REG_DFE_CTRL_G2_P0_eq_selreg_g_mask), (DFE_P0_REG_DFE_CTRL_G2_P0_eq_selreg_g(1)));
			udelay(1);
			hdmi_mask(REG_DFE_CTRL_G2_reg,
				~(DFE_P0_REG_DFE_CTRL_G2_P0_eq_selreg_g_mask), (DFE_P0_REG_DFE_CTRL_G2_P0_eq_selreg_g(0)));
		}
		if (bgrc&LN_B) {
			hdmi_mask(REG_DFE_CTRL_B8_reg,
				~(DFE_P0_REG_DFE_CTRL_B8_P0_leq_init_b_4_0_mask), (DFE_P0_REG_DFE_CTRL_B8_P0_leq_init_b_4_0(eq_ch0)));
			hdmi_mask(REG_DFE_CTRL_B2_reg,
				~(DFE_P0_REG_DFE_CTRL_B2_P0_eq_selreg_b_mask), (DFE_P0_REG_DFE_CTRL_B2_P0_eq_selreg_b(1)));
			udelay(1);
			hdmi_mask(REG_DFE_CTRL_B2_reg,
				~(DFE_P0_REG_DFE_CTRL_B2_P0_eq_selreg_b_mask), (DFE_P0_REG_DFE_CTRL_B2_P0_eq_selreg_b(0)));
		}


	}
	else if (mode == 3)
	{
		//mode 3
		//0~24
		/*
		if (bgrc&LN_CK) {
		hdmi_mask(REG_DFE_CTRL_CK9_reg,
				~(DFE_P0_REG_DFE_CTRL_CK9_P0_tap1_init_ck_5_0_mask), (DFE_P0_REG_DFE_CTRL_CK9_P0_tap1_init_ck_5_0(value)));
		hdmi_mask(REG_DFE_CTRL_CK3_reg,
				~(DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0_mask), (DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0(LOAD_IN_INIT_TAP1)));
		udelay(1);
		hdmi_mask(REG_DFE_CTRL_CK3_reg,
				~(DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0_mask), (DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0(0)));
		}
		*/
		if (bgrc&LN_R) {
		hdmi_mask(REG_DFE_CTRL_R9_reg,
				~(DFE_P0_REG_DFE_CTRL_R9_P0_tap1_init_r_5_0_mask), (DFE_P0_REG_DFE_CTRL_R9_P0_tap1_init_r_5_0(eq_ch2)));
		hdmi_mask(REG_DFE_CTRL_R3_reg,
				~(DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0_mask), (DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0(LOAD_IN_INIT_TAP1)));
		udelay(1);
		hdmi_mask(REG_DFE_CTRL_R3_reg,
				~(DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0_mask), (DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0(0)));
		}
		if (bgrc&LN_G) {
		hdmi_mask(REG_DFE_CTRL_G9_reg,
				~(DFE_P0_REG_DFE_CTRL_G9_P0_tap1_init_g_5_0_mask), (DFE_P0_REG_DFE_CTRL_G9_P0_tap1_init_g_5_0(eq_ch1)));
		hdmi_mask(REG_DFE_CTRL_G3_reg,
				~(DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0_mask), (DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0(LOAD_IN_INIT_TAP1)));
		udelay(1);
		hdmi_mask(REG_DFE_CTRL_G3_reg,
				~(DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0_mask), (DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0(0)));
		}
		if (bgrc&LN_B) {

		hdmi_mask(REG_DFE_CTRL_B9_reg,
				~(DFE_P0_REG_DFE_CTRL_B9_P0_tap1_init_b_5_0_mask), (DFE_P0_REG_DFE_CTRL_B9_P0_tap1_init_b_5_0(eq_ch0)));
		hdmi_mask(REG_DFE_CTRL_B3_reg,
				~(DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0_mask), (DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0(LOAD_IN_INIT_TAP1)));
		udelay(1);
		hdmi_mask(REG_DFE_CTRL_B3_reg,
				~(DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0_mask), (DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0(0)));

		}
	}
	else {
		//do nothing
	}

}

void lib_hdmi_load_le_max(unsigned char nport,unsigned char lane_mode)
{
#if 0
	unsigned char lemax[4];
	lemax[0] = lemax[1] = lemax[2] = lemax[3] = 0;

	HDMI_PRINTF("%s\n", __func__);

	//read LE max
	hdmi_mask(DFE_REG_DFE_READBACK_reg, ~DFE_P0_REG_DFE_READBACK_P0_limit_sel_p0_mask, DFE_P0_REG_DFE_READBACK_P0_limit_sel_p0(LIMIT_LE_MAX));

	hdmi_mask(DFE_REG_DFE_READBACK_reg, ~DFE_P0_REG_DFE_READBACK_P0_lane_sel_p0_mask, DFE_P0_REG_DFE_READBACK_P0_lane_sel_p0(0));
	lemax[0] = (DFE_P0_REG_DFE_READBACK_P0_get_limit_data_p0(hdmi_in(DFE_REG_DFE_READBACK_reg)));

	hdmi_mask(DFE_REG_DFE_READBACK_reg, ~DFE_P0_REG_DFE_READBACK_P0_lane_sel_p0_mask, DFE_P0_REG_DFE_READBACK_P0_lane_sel_p0(1));
	lemax[1] = (DFE_P0_REG_DFE_READBACK_P0_get_limit_data_p0(hdmi_in(DFE_REG_DFE_READBACK_reg)));

 	hdmi_mask(DFE_REG_DFE_READBACK_reg, ~DFE_P0_REG_DFE_READBACK_P0_lane_sel_p0_mask, DFE_P0_REG_DFE_READBACK_P0_lane_sel_p0(2));
	lemax[2] = (DFE_P0_REG_DFE_READBACK_P0_get_limit_data_p0(hdmi_in(DFE_REG_DFE_READBACK_reg)));

	if (lane_mode == HDMI_4LANE) {
		hdmi_mask(DFE_REG_DFE_READBACK_reg, ~DFE_P0_REG_DFE_READBACK_P0_lane_sel_p0_mask, DFE_P0_REG_DFE_READBACK_P0_lane_sel_p0(3));
		lemax[3] = (DFE_P0_REG_DFE_READBACK_P0_get_limit_data_p0(hdmi_in(DFE_REG_DFE_READBACK_reg)));
	}
		

	hdmi_mask(DFE_REG_DFE_READBACK_reg, ~DFE_P0_REG_DFE_READBACK_P0_limit_sel_p0_mask, DFE_P0_REG_DFE_READBACK_P0_limit_sel_p0(LIMIT_TAP1_MAX));

	hdmi_mask(DFE_REG_DFE_READBACK_reg, ~DFE_P0_REG_DFE_READBACK_P0_lane_sel_p0_mask, DFE_P0_REG_DFE_READBACK_P0_lane_sel_p0(0));
	lemax[0] += (DFE_P0_REG_DFE_READBACK_P0_get_limit_data_p0(hdmi_in(DFE_REG_DFE_READBACK_reg)));

	hdmi_mask(DFE_REG_DFE_READBACK_reg, ~DFE_P0_REG_DFE_READBACK_P0_lane_sel_p0_mask, DFE_P0_REG_DFE_READBACK_P0_lane_sel_p0(1));
	lemax[1] += (DFE_P0_REG_DFE_READBACK_P0_get_limit_data_p0(hdmi_in(DFE_REG_DFE_READBACK_reg)));

 	hdmi_mask(DFE_REG_DFE_READBACK_reg, ~DFE_P0_REG_DFE_READBACK_P0_lane_sel_p0_mask, DFE_P0_REG_DFE_READBACK_P0_lane_sel_p0(2));
	lemax[2] += (DFE_P0_REG_DFE_READBACK_P0_get_limit_data_p0(hdmi_in(DFE_REG_DFE_READBACK_reg)));

	if (lane_mode == HDMI_4LANE) {
		hdmi_mask(DFE_REG_DFE_READBACK_reg, ~DFE_P0_REG_DFE_READBACK_P0_lane_sel_p0_mask, DFE_P0_REG_DFE_READBACK_P0_lane_sel_p0(3));
		lemax[3] += (DFE_P0_REG_DFE_READBACK_P0_get_limit_data_p0(hdmi_in(DFE_REG_DFE_READBACK_reg)));
	}

	// LOAD LE max
	hdmi_mask(DFE_REG_DFE_INIT0_L0_reg,~(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0_mask),(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0(lemax[0])));
	hdmi_mask(DFE_REG_DFE_INIT0_L1_reg,~(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0_mask),(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0(lemax[1])));
	hdmi_mask(DFE_REG_DFE_INIT0_L2_reg,~(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0_mask),(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0(lemax[2])));
	if ((lane_mode == HDMI_4LANE) && (nport<HDMI20_PORT)) {
		hdmi_mask(DFE_REG_DFE_INIT0_L3_reg,~(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0_mask),(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0(lemax[3])));
	}

	//load initial data
	hdmi_mask(DFE_REG_DFE_INIT1_L0_reg,~(LOAD_IN_INIT_TAP1),(LOAD_IN_INIT_TAP1));
	hdmi_mask(DFE_REG_DFE_INIT1_L1_reg,~(LOAD_IN_INIT_TAP1),(LOAD_IN_INIT_TAP1));
	hdmi_mask(DFE_REG_DFE_INIT1_L2_reg,~(LOAD_IN_INIT_TAP1),(LOAD_IN_INIT_TAP1));
	if ((lane_mode == HDMI_4LANE) && (nport<HDMI20_PORT)) {
		hdmi_mask(DFE_REG_DFE_INIT1_L3_reg,~(LOAD_IN_INIT_TAP1),(LOAD_IN_INIT_TAP1));
	}

	udelay(1);
	hdmi_mask(DFE_REG_DFE_INIT1_L0_reg,~(LOAD_IN_INIT_TAP1),0);
	hdmi_mask(DFE_REG_DFE_INIT1_L1_reg,~(LOAD_IN_INIT_TAP1),0);
	hdmi_mask(DFE_REG_DFE_INIT1_L2_reg,~(LOAD_IN_INIT_TAP1),0);
	if ((lane_mode == HDMI_4LANE) && (nport<HDMI20_PORT)) {
		hdmi_mask(DFE_REG_DFE_INIT1_L3_reg,~(LOAD_IN_INIT_TAP1),0);
	}
#endif
}


void lib_hdmi_dfe_examine_le_coef(unsigned char nport, unsigned int b_clk,unsigned char lane_mode)
{
unsigned char max_tap0, min_tap0;
//unsigned char  bLan0_LE,bLan1_LE,bLan2_LE;

	//HDMI_PRINTF("%s\n", __func__);
	//pr_err ("%s(%d)-TAP0(0=%d, 1=%d, 2=%d)\n", __func__,nport , lib_hdmi_dfe_get_tap0(nport, 0), lib_hdmi_dfe_get_tap0(nport, 1), lib_hdmi_dfe_get_tap0(nport, 2));

	if (lane_mode == HDMI_3LANE) {
		max_tap0 = MAX(lib_hdmi_dfe_get_tap0(nport, 0), MAX(lib_hdmi_dfe_get_tap0(nport, 1), lib_hdmi_dfe_get_tap0(nport, 2)));
		min_tap0 = MIN(lib_hdmi_dfe_get_tap0(nport, 0), MIN(lib_hdmi_dfe_get_tap0(nport, 1), lib_hdmi_dfe_get_tap0(nport, 2)));
		HDMI_PRINTF ("%s(%d)-(max=%d, min=%d, b_clk=%d)\n",  __func__, nport ,max_tap0, min_tap0, b_clk);

		if ( (max_tap0-min_tap0 > 10) && ((b_clk > 2000) && (b_clk <2200)) ) {
			if(lib_hdmi_dfe_get_le(nport, 0) < 1){
				hdmi_mask(REG_DFE_CTRL_B9_reg,	~(DFE_P0_REG_DFE_CTRL_B9_P0_tap1_init_b_5_0_mask), (DFE_P0_REG_DFE_CTRL_B9_P0_tap1_init_b_5_0(0x1)));
				udelay(1);
				hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0_mask), (DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0(LOAD_IN_INIT_TAP1)));
				udelay(1);
				hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0_mask), (DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0(0)));

				HDMI_EMG("[RELE_L0] LE reload 0x1\n");
			}

			if(lib_hdmi_dfe_get_le(nport, 1) < 1){
				hdmi_mask(REG_DFE_CTRL_G9_reg,~(DFE_P0_REG_DFE_CTRL_G9_P0_tap1_init_g_5_0_mask), (DFE_P0_REG_DFE_CTRL_G9_P0_tap1_init_g_5_0(0x1)));
				udelay(1);
				hdmi_mask(REG_DFE_CTRL_G3_reg,~(DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0_mask), (DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0(LOAD_IN_INIT_TAP1)));
				udelay(1);
				hdmi_mask(REG_DFE_CTRL_G3_reg,~(DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0_mask), (DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0(0)));
				HDMI_EMG("[RELE_L1] LE reload 0x1\n");
			}

			if(lib_hdmi_dfe_get_le(nport, 2) < 4){
				hdmi_mask(REG_DFE_CTRL_R9_reg,	~(DFE_P0_REG_DFE_CTRL_R9_P0_tap1_init_r_5_0_mask), (DFE_P0_REG_DFE_CTRL_R9_P0_tap1_init_r_5_0(0x6)));
				udelay(1);
				hdmi_mask(REG_DFE_CTRL_R3_reg,	~(DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0_mask), (DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0(LOAD_IN_INIT_TAP1)));
				udelay(1);
				hdmi_mask(REG_DFE_CTRL_R3_reg,	~(DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0_mask), (DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0(0)));
				HDMI_EMG("[RELE_L2] LE reload 0x6\n");
			}
		}
	}
}


void newbase_hdmi_tap2_adjust_with_le(unsigned char nport, unsigned int b_clk,unsigned char lane_mode)
{
	unsigned char le_0,le_1,le_2,le_3;
	signed char tap2_0,tap2_1,tap2_2,tap2_3;
	unsigned char bgrc = 0;

	if (b_clk > 2700) {
		le_0 = lib_hdmi_dfe_get_le(nport, 0);
		le_1 = lib_hdmi_dfe_get_le(nport, 1);
		le_2 = lib_hdmi_dfe_get_le(nport, 2);
		if (lane_mode == HDMI_4LANE)
			le_3 = lib_hdmi_dfe_get_le(nport, 3);

		tap2_0 = lib_hdmi_dfe_get_tap2(nport, 0);
		tap2_1 = lib_hdmi_dfe_get_tap2(nport, 1);
		tap2_2 = lib_hdmi_dfe_get_tap2(nport, 2);
		if (lane_mode == HDMI_4LANE)
			tap2_3 = lib_hdmi_dfe_get_tap2(nport, 3);

		if (le_0 == 0)
			bgrc = bgrc | LN_B;
		if (le_1 == 0)
			bgrc = bgrc | LN_G;
		if (le_2 == 0)
			bgrc = bgrc | LN_R;

		if (lane_mode == HDMI_4LANE) {
			if(le_3 == 0)
				bgrc = bgrc | LN_CK;
		}

		if (bgrc > 0) {
			if (lane_mode == HDMI_4LANE)
				HDMI_PRINTF("original tap2=(%d,%d,%d,%d)",tap2_0,tap2_1,tap2_2,tap2_3);
			else
				HDMI_PRINTF("original tap2=(%d,%d,%d)",tap2_0,tap2_1,tap2_2);
			
			lib_hdmi_dfe_init_tap2(nport,bgrc,0);
		} else {
			HDMI_PRINTF("%s :  no adjust  \n",__func__);
		}
					
	}
}


int  newbase_hdmi_dfe_threshold_check(unsigned char nport,unsigned int b_clk,unsigned char lane_mode)
{   
	unsigned char min_le = 0;
	unsigned char max_le = 0;
	unsigned int bit_r_err, bit_g_err, bit_b_err;
	unsigned int timeout = 0;
	unsigned char mode = 0;

	if (lane_mode == HDMI_3LANE) {
		//min_tap1 = MIN(lib_hdmi_dfe_get_tap1(nport, 0), MIN(lib_hdmi_dfe_get_tap1(nport, 1), lib_hdmi_dfe_get_tap1(nport, 2)));
		min_le = MIN(lib_hdmi_dfe_get_le(nport, 0), MIN(lib_hdmi_dfe_get_le(nport, 1), lib_hdmi_dfe_get_le(nport, 2)));
		if ( min_le >= 22) {// over threshold reset phy
			HDMI_EMG("min_le=%d\n", min_le);
			return 1;
		}
			
	}

	if ((OBJ_TV_only == 1)&& (lane_mode == HDMI_3LANE)&& (b_clk > 1380) && (b_clk < 2700)) {
		min_le = MIN(lib_hdmi_dfe_get_le(nport, 0), MIN(lib_hdmi_dfe_get_le(nport, 1), lib_hdmi_dfe_get_le(nport, 2)));
		max_le = MAX(lib_hdmi_dfe_get_le(nport, 0), MAX(lib_hdmi_dfe_get_le(nport, 1), lib_hdmi_dfe_get_le(nport, 2)));
		if ((max_le - min_le) > 20) {
			HDMI_EMG("max_le=%d min_le=%d\n",max_le,min_le);
			lib_hdmi_bit_err_start(nport, 0, 32);
			while (!lib_hdmi_bit_err_get_error(nport, &bit_r_err, &bit_g_err, &bit_b_err)) {
				timeout++;
				msleep(20);
				if (timeout > 50)
					break;
			}

			mode = DFE_P0_REG_DFE_CTRL_RB_P0_dfe_adapt_mode_r_1_0(hdmi_in(REG_DFE_CTRL_RB_reg));
			HDMI_EMG(" [BIT](%05d, %05d, %05d) \n", bit_r_err, bit_g_err, bit_b_err);

			if ((bit_r_err >= 10) || (bit_g_err >= 10) || (bit_b_err >= 10)) {
				if (mode == 2) {
					lib_hdmi_dfe_init_le(nport, (LN_R|LN_G|LN_B), 0x0);
				}
				else {
					lib_hdmi_dfe_init_tap1(nport, (LN_R|LN_G|LN_B), 0x0);
				}
			}
			else
				return 1;
			
		}
	}
	
	return 0;

}



/**************************** HDMI2.1 PHY DCDR/ACDR MODE DFE ADAPT **************************/

void lib_hdmi_vth_set(unsigned char nport,unsigned char lane_mode,unsigned char value)
{
//Set vthp & vthn

	if (lane_mode == HDMI_4LANE){
		hdmi_mask(REG_DFE_CTRL_CKA_reg,~(DFE_P0_REG_DFE_CTRL_CKA_P0_vthp_init_ck_4_0_mask), (DFE_P0_REG_DFE_CTRL_CKA_P0_vthp_init_ck_4_0(value)));
		hdmi_mask(REG_DFE_CTRL_CKA_reg,~(DFE_P0_REG_DFE_CTRL_CKA_P0_vthn_init_ck_4_0_mask), (DFE_P0_REG_DFE_CTRL_CKA_P0_vthn_init_ck_4_0(value)));
		hdmi_mask(REG_DFE_CTRL_CK3_reg,~(DFE_P0_REG_DFE_CTRL_CK3_P0_vth_load_in_ck_mask), (DFE_P0_REG_DFE_CTRL_CK3_P0_vth_load_in_ck(1)));
		udelay(1);
		hdmi_mask(REG_DFE_CTRL_CK3_reg,~(DFE_P0_REG_DFE_CTRL_CK3_P0_vth_load_in_ck_mask), (DFE_P0_REG_DFE_CTRL_CK3_P0_vth_load_in_ck(0)));
	}

	hdmi_mask(REG_DFE_CTRL_RA_reg,~(DFE_P0_REG_DFE_CTRL_RA_P0_vthp_init_r_4_0_mask), (DFE_P0_REG_DFE_CTRL_RA_P0_vthp_init_r_4_0(value)));
	hdmi_mask(REG_DFE_CTRL_RA_reg,~(DFE_P0_REG_DFE_CTRL_RA_P0_vthn_init_r_4_0_mask), (DFE_P0_REG_DFE_CTRL_RA_P0_vthn_init_r_4_0(value)));
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_R3_P0_vth_load_in_r_mask), (DFE_P0_REG_DFE_CTRL_R3_P0_vth_load_in_r(1)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_R3_P0_vth_load_in_r_mask), (DFE_P0_REG_DFE_CTRL_R3_P0_vth_load_in_r(0)));

	hdmi_mask(REG_DFE_CTRL_GA_reg,~(DFE_P0_REG_DFE_CTRL_GA_P0_vthp_init_g_4_0_mask), (DFE_P0_REG_DFE_CTRL_GA_P0_vthp_init_g_4_0(value)));
	hdmi_mask(REG_DFE_CTRL_GA_reg,~(DFE_P0_REG_DFE_CTRL_GA_P0_vthn_init_g_4_0_mask), (DFE_P0_REG_DFE_CTRL_GA_P0_vthn_init_g_4_0(value)));
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_G3_P0_vth_load_in_g_mask), (DFE_P0_REG_DFE_CTRL_G3_P0_vth_load_in_g(1)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_G3_P0_vth_load_in_g_mask), (DFE_P0_REG_DFE_CTRL_G3_P0_vth_load_in_g(0)));

	hdmi_mask(REG_DFE_CTRL_BA_reg,~(DFE_P0_REG_DFE_CTRL_BA_P0_vthp_init_b_4_0_mask), (DFE_P0_REG_DFE_CTRL_BA_P0_vthp_init_b_4_0(value)));
	hdmi_mask(REG_DFE_CTRL_BA_reg,~(DFE_P0_REG_DFE_CTRL_BA_P0_vthn_init_b_4_0_mask), (DFE_P0_REG_DFE_CTRL_BA_P0_vthn_init_b_4_0(value)));
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_B3_P0_vth_load_in_b_mask), (DFE_P0_REG_DFE_CTRL_B3_P0_vth_load_in_b(1)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_B3_P0_vth_load_in_b_mask), (DFE_P0_REG_DFE_CTRL_B3_P0_vth_load_in_b(0)));
}


void lib_hdmi_load_tap0_max(unsigned char nport,unsigned char lane_mode)
{
	unsigned char i, tap0max[4];

	//read Tap0 max
	for (i=0; i<lane_mode; i++) {
		tap0max[i] = lib_hdmi_dfe_get_tap0max(nport, i);
		tap0max[i] = (tap0max[i]>2)?(tap0max[i]-2):0;
	}

	if (lane_mode == HDMI_4LANE) {
		hdmi_mask(REG_DFE_CTRL_CK9_reg,~(DFE_P0_REG_DFE_CTRL_CK9_P0_tap0_init_ck_7_0_mask), (DFE_P0_REG_DFE_CTRL_CK9_P0_tap0_init_ck_7_0(tap0max[3])));
		hdmi_mask(REG_DFE_CTRL_CK3_reg,~(DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0_mask), (DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0(LOAD_IN_INIT_TAP0)));
		udelay(1);
		hdmi_mask(REG_DFE_CTRL_CK3_reg,~(DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0_mask), (DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0(0)));
	}

	hdmi_mask(REG_DFE_CTRL_R9_reg,	~(DFE_P0_REG_DFE_CTRL_R9_P0_tap0_init_r_7_0_mask), (DFE_P0_REG_DFE_CTRL_R9_P0_tap0_init_r_7_0(tap0max[2])));
	hdmi_mask(REG_DFE_CTRL_R3_reg,	~(DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0_mask), (DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0(LOAD_IN_INIT_TAP0)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_R3_reg,	~(DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0_mask), (DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0(0)));

	hdmi_mask(REG_DFE_CTRL_G9_reg,~(DFE_P0_REG_DFE_CTRL_G9_P0_tap0_init_g_7_0_mask), (DFE_P0_REG_DFE_CTRL_G9_P0_tap0_init_g_7_0(tap0max[1])));
	hdmi_mask(REG_DFE_CTRL_G3_reg,~(DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0_mask), (DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0(LOAD_IN_INIT_TAP0)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_G3_reg,~(DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0_mask), (DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0(0)));

	hdmi_mask(REG_DFE_CTRL_B9_reg,	~(DFE_P0_REG_DFE_CTRL_B9_P0_tap0_init_b_7_0_mask), (DFE_P0_REG_DFE_CTRL_B9_P0_tap0_init_b_7_0(tap0max[0])));
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0_mask), (DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0(LOAD_IN_INIT_TAP0)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0_mask), (DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0(0)));

}


void lib_hdmi_load_tap1_add(unsigned char nport,unsigned char lane_mode,unsigned char value)
{
	unsigned char tap1[4];
	tap1[0] = lib_hdmi_dfe_get_tap1(nport, 0);
	tap1[1] = lib_hdmi_dfe_get_tap1(nport, 1);
	tap1[2] = lib_hdmi_dfe_get_tap1(nport, 2);
	tap1[3] = lib_hdmi_dfe_get_tap1(nport, 3);

	HDMI_EMG("origin tap1=%d %d %d %d\n",tap1[0],tap1[1] ,tap1[2] ,tap1[3]);

	tap1[0] = tap1[0] + value;
	tap1[1] = tap1[1] + value;
	tap1[2] = tap1[2] + value;
	tap1[3] = tap1[3] + value;

	if (tap1[0] >= 0xf)
		tap1[0] = 0xe;
	if (tap1[1] >= 0xf)
		tap1[1] = 0xe;
	if (tap1[2] >= 0xf)
		tap1[2] = 0xe;
	if (tap1[3] >= 0xf)
		tap1[3] = 0xe;
	

	if (lane_mode == HDMI_4LANE) {
	hdmi_mask(REG_DFE_CTRL_CK9_reg,~(DFE_P0_REG_DFE_CTRL_CK9_P0_tap1_init_ck_5_0_mask), (DFE_P0_REG_DFE_CTRL_CK9_P0_tap1_init_ck_5_0(tap1[3])));
	hdmi_mask(REG_DFE_CTRL_CK3_reg,~(DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0_mask), (DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0(LOAD_IN_INIT_TAP1)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_CK3_reg,~(DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0_mask), (DFE_P0_REG_DFE_CTRL_CK3_P0_tap_load_in_ck_4_0(0)));
	}

	hdmi_mask(REG_DFE_CTRL_R9_reg,	~(DFE_P0_REG_DFE_CTRL_R9_P0_tap1_init_r_5_0_mask), (DFE_P0_REG_DFE_CTRL_R9_P0_tap1_init_r_5_0(tap1[2])));
	hdmi_mask(REG_DFE_CTRL_R3_reg,	~(DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0_mask), (DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0(LOAD_IN_INIT_TAP1)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_R3_reg,	~(DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0_mask), (DFE_P0_REG_DFE_CTRL_R3_P0_tap_load_in_r_4_0(0)));

	hdmi_mask(REG_DFE_CTRL_G9_reg,~(DFE_P0_REG_DFE_CTRL_G9_P0_tap1_init_g_5_0_mask), (DFE_P0_REG_DFE_CTRL_G9_P0_tap1_init_g_5_0(tap1[1])));
	hdmi_mask(REG_DFE_CTRL_G3_reg,~(DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0_mask), (DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0(LOAD_IN_INIT_TAP1)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_G3_reg,~(DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0_mask), (DFE_P0_REG_DFE_CTRL_G3_P0_tap_load_in_g_4_0(0)));

	hdmi_mask(REG_DFE_CTRL_B9_reg,	~(DFE_P0_REG_DFE_CTRL_B9_P0_tap1_init_b_5_0_mask), (DFE_P0_REG_DFE_CTRL_B9_P0_tap1_init_b_5_0(tap1[0])));
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0_mask), (DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0(LOAD_IN_INIT_TAP1)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_B3_reg,	~(DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0_mask), (DFE_P0_REG_DFE_CTRL_B3_P0_tap_load_in_b_4_0(0)));

}




void lib_hdmi_load_le_add(unsigned char nport,unsigned char lane_mode,unsigned char value)
{
	unsigned char le[4];
	le[0] = lib_hdmi_dfe_get_le(nport, 0);
	le[1] = lib_hdmi_dfe_get_le(nport, 1);
	le[2] = lib_hdmi_dfe_get_le(nport, 2);
	le[3] = lib_hdmi_dfe_get_le(nport, 3);

	HDMI_EMG("origin le=%d %d %d %d\n",le[0],le[1] ,le[2] ,le[3]);

	le[0] = le[0] + value;
	le[1] = le[1] + value;
	le[2] = le[2] + value;
	le[3] = le[3] + value;

	if (lane_mode == HDMI_4LANE) {
		hdmi_mask(REG_DFE_CTRL_CK8_reg,~(DFE_P0_REG_DFE_CTRL_CK8_P0_leq_init_ck_4_0_mask), (DFE_P0_REG_DFE_CTRL_CK8_P0_leq_init_ck_4_0(le[3])));
		hdmi_mask(REG_DFE_CTRL_CK2_reg,~(DFE_P0_REG_DFE_CTRL_CK2_P0_eq_selreg_ck_mask), (DFE_P0_REG_DFE_CTRL_CK2_P0_eq_selreg_ck(1)));
		udelay(1);
		hdmi_mask(REG_DFE_CTRL_CK2_reg,~(DFE_P0_REG_DFE_CTRL_CK2_P0_eq_selreg_ck_mask), (DFE_P0_REG_DFE_CTRL_CK2_P0_eq_selreg_ck(0)));
	}

	hdmi_mask(REG_DFE_CTRL_R8_reg,	~(DFE_P0_REG_DFE_CTRL_R8_P0_leq_init_r_4_0_mask), (DFE_P0_REG_DFE_CTRL_R8_P0_leq_init_r_4_0(le[2])));
	hdmi_mask(REG_DFE_CTRL_R2_reg,	~(DFE_P0_REG_DFE_CTRL_R2_P0_eq_selreg_r_mask), (DFE_P0_REG_DFE_CTRL_R2_P0_eq_selreg_r(1)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_R2_reg,	~(DFE_P0_REG_DFE_CTRL_R2_P0_eq_selreg_r_mask), (DFE_P0_REG_DFE_CTRL_R2_P0_eq_selreg_r(0)));

	hdmi_mask(REG_DFE_CTRL_G8_reg,~(DFE_P0_REG_DFE_CTRL_G8_P0_leq_init_g_4_0_mask), (DFE_P0_REG_DFE_CTRL_G8_P0_leq_init_g_4_0(le[1])));
	hdmi_mask(REG_DFE_CTRL_G2_reg,~(DFE_P0_REG_DFE_CTRL_G2_P0_eq_selreg_g_mask), (DFE_P0_REG_DFE_CTRL_G2_P0_eq_selreg_g(1)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_G2_reg,~(DFE_P0_REG_DFE_CTRL_G2_P0_eq_selreg_g_mask), (DFE_P0_REG_DFE_CTRL_G2_P0_eq_selreg_g(0)));

	hdmi_mask(REG_DFE_CTRL_B8_reg,	~(DFE_P0_REG_DFE_CTRL_B8_P0_leq_init_b_4_0_mask), (DFE_P0_REG_DFE_CTRL_B8_P0_leq_init_b_4_0(le[0])));
	hdmi_mask(REG_DFE_CTRL_B2_reg,	~(DFE_P0_REG_DFE_CTRL_B2_P0_eq_selreg_b_mask), (DFE_P0_REG_DFE_CTRL_B2_P0_eq_selreg_b(1)));
	udelay(1);
	hdmi_mask(REG_DFE_CTRL_B2_reg,	~(DFE_P0_REG_DFE_CTRL_B2_P0_eq_selreg_b_mask), (DFE_P0_REG_DFE_CTRL_B2_P0_eq_selreg_b(0)));


}


void lib_dfe_tap0_le_adp_en(unsigned char nport, unsigned char lane_mode, unsigned char en)
{
	if (en) {

		hdmi_mask(REG_DFE_CTRL_BA_reg,~(REG_dfe_adapt_en_TAP0),REG_dfe_adapt_en_TAP0);
		hdmi_mask(REG_DFE_CTRL_B3_reg,~(REG_dfe_adapt_en_LE),REG_dfe_adapt_en_LE);

		hdmi_mask(REG_DFE_CTRL_GA_reg,~(REG_dfe_adapt_en_TAP0),REG_dfe_adapt_en_TAP0);
		hdmi_mask(REG_DFE_CTRL_G3_reg,~(REG_dfe_adapt_en_LE),REG_dfe_adapt_en_LE);

		hdmi_mask(REG_DFE_CTRL_RA_reg,~(REG_dfe_adapt_en_TAP0),REG_dfe_adapt_en_TAP0);
		hdmi_mask(REG_DFE_CTRL_R3_reg,~(REG_dfe_adapt_en_LE),REG_dfe_adapt_en_LE);

		if (lane_mode == HDMI_4LANE)
		{
			hdmi_mask(REG_DFE_CTRL_CKA_reg,~(REG_dfe_adapt_en_TAP0),REG_dfe_adapt_en_TAP0);
			hdmi_mask(REG_DFE_CTRL_CK3_reg,~(REG_dfe_adapt_en_LE),REG_dfe_adapt_en_LE);
		}
	}
	else
	{
		hdmi_mask(REG_DFE_CTRL_BA_reg,~(REG_dfe_adapt_en_TAP0),0);
		hdmi_mask(REG_DFE_CTRL_B3_reg,~(REG_dfe_adapt_en_LE),0);

		hdmi_mask(REG_DFE_CTRL_GA_reg,~(REG_dfe_adapt_en_TAP0),0);
		hdmi_mask(REG_DFE_CTRL_G3_reg,~(REG_dfe_adapt_en_LE),0);

		hdmi_mask(REG_DFE_CTRL_RA_reg,~(REG_dfe_adapt_en_TAP0),0);
		hdmi_mask(REG_DFE_CTRL_R3_reg,~(REG_dfe_adapt_en_LE),0);

		if (lane_mode == HDMI_4LANE)
		{
			hdmi_mask(REG_DFE_CTRL_CKA_reg,~(REG_dfe_adapt_en_TAP0),0);
			hdmi_mask(REG_DFE_CTRL_CK3_reg,~(REG_dfe_adapt_en_LE),0);
		}

	}
}


void lib_dfe_tap0_disable(unsigned char nport, unsigned char lane_mode)
{

	hdmi_mask(REG_DFE_CTRL_BA_reg,~(REG_dfe_adapt_en_TAP0),0);
	hdmi_mask(REG_DFE_CTRL_GA_reg,~(REG_dfe_adapt_en_TAP0),0);
	hdmi_mask(REG_DFE_CTRL_RA_reg,~(REG_dfe_adapt_en_TAP0),0);
	if (lane_mode == HDMI_4LANE)
	{
		hdmi_mask(REG_DFE_CTRL_CKA_reg,~(REG_dfe_adapt_en_TAP0),0);
	}

}

void lib_dfe_le_disable(unsigned char nport,unsigned char lane_mode)
{
	hdmi_mask(REG_DFE_CTRL_B3_reg,~(REG_dfe_adapt_en_LE),0);
	hdmi_mask(REG_DFE_CTRL_G3_reg,~(REG_dfe_adapt_en_LE),0);
	hdmi_mask(REG_DFE_CTRL_R3_reg,~(REG_dfe_adapt_en_LE),0);
	if (lane_mode == HDMI_4LANE)
	{
		hdmi_mask(REG_DFE_CTRL_CK3_reg,~(REG_dfe_adapt_en_LE),0);
	}
}

void lib_acdr_dfe_tap_all_adp_en(unsigned char nport,unsigned char frl_mode,unsigned char lane_mode,unsigned char en)
{
	if (en) {
		switch(frl_mode)
		{
			case MODE_FRL_3G_3_LANE:
			case MODE_FRL_6G_3_LANE:
			case MODE_FRL_6G_4_LANE:
				hdmi_mask(REG_DFE_CTRL_BA_reg,~(REG_dfe_adapt_en_TAP0|REG_dfe_adapt_en_TAP1|REG_dfe_adapt_en_TAP2),REG_dfe_adapt_en_TAP0|REG_dfe_adapt_en_TAP1|REG_dfe_adapt_en_TAP2);
				hdmi_mask(REG_DFE_CTRL_GA_reg,~(REG_dfe_adapt_en_TAP0|REG_dfe_adapt_en_TAP1|REG_dfe_adapt_en_TAP2),REG_dfe_adapt_en_TAP0|REG_dfe_adapt_en_TAP1|REG_dfe_adapt_en_TAP2);
				hdmi_mask(REG_DFE_CTRL_RA_reg,~(REG_dfe_adapt_en_TAP0|REG_dfe_adapt_en_TAP1|REG_dfe_adapt_en_TAP2),REG_dfe_adapt_en_TAP0|REG_dfe_adapt_en_TAP1|REG_dfe_adapt_en_TAP2);
				if (lane_mode == HDMI_4LANE)
				{
					hdmi_mask(REG_DFE_CTRL_CKA_reg,~(REG_dfe_adapt_en_TAP0|REG_dfe_adapt_en_TAP1|REG_dfe_adapt_en_TAP2),REG_dfe_adapt_en_TAP0|REG_dfe_adapt_en_TAP1|REG_dfe_adapt_en_TAP2);
				}
				break;
		
			case MODE_FRL_8G_4_LANE:
			case MODE_FRL_10G_4_LANE:
			case MODE_FRL_12G_4_LANE:
				hdmi_mask(REG_DFE_CTRL_BA_reg,~(REG_dfe_adapt_en_TAP0|REG_dfe_adapt_en_TAP1|REG_dfe_adapt_en_TAP2|REG_dfe_adapt_en_TAP3|REG_dfe_adapt_en_TAP4),REG_dfe_adapt_en_TAP0|REG_dfe_adapt_en_TAP1|REG_dfe_adapt_en_TAP2|REG_dfe_adapt_en_TAP3|REG_dfe_adapt_en_TAP4);
				hdmi_mask(REG_DFE_CTRL_GA_reg,~(REG_dfe_adapt_en_TAP0|REG_dfe_adapt_en_TAP1|REG_dfe_adapt_en_TAP2|REG_dfe_adapt_en_TAP3|REG_dfe_adapt_en_TAP4),REG_dfe_adapt_en_TAP0|REG_dfe_adapt_en_TAP1|REG_dfe_adapt_en_TAP2|REG_dfe_adapt_en_TAP3|REG_dfe_adapt_en_TAP4);
				hdmi_mask(REG_DFE_CTRL_RA_reg,~(REG_dfe_adapt_en_TAP0|REG_dfe_adapt_en_TAP1|REG_dfe_adapt_en_TAP2|REG_dfe_adapt_en_TAP3|REG_dfe_adapt_en_TAP4),REG_dfe_adapt_en_TAP0|REG_dfe_adapt_en_TAP1|REG_dfe_adapt_en_TAP2|REG_dfe_adapt_en_TAP3|REG_dfe_adapt_en_TAP4);
				if (lane_mode == HDMI_4LANE)
				{
					hdmi_mask(REG_DFE_CTRL_CKA_reg,~(REG_dfe_adapt_en_TAP0|REG_dfe_adapt_en_TAP1|REG_dfe_adapt_en_TAP2),REG_dfe_adapt_en_TAP0|REG_dfe_adapt_en_TAP1|REG_dfe_adapt_en_TAP2|REG_dfe_adapt_en_TAP3|REG_dfe_adapt_en_TAP4);
				}
				break;
		}	
	}
	else {
		// DISABLE  TAP0~TAP4
		hdmi_mask(REG_DFE_CTRL_BA_reg,~(REG_dfe_adapt_en_TAP0|REG_dfe_adapt_en_TAP1|REG_dfe_adapt_en_TAP2|REG_dfe_adapt_en_TAP3|REG_dfe_adapt_en_TAP4),0);
		hdmi_mask(REG_DFE_CTRL_GA_reg,~(REG_dfe_adapt_en_TAP0|REG_dfe_adapt_en_TAP1|REG_dfe_adapt_en_TAP2|REG_dfe_adapt_en_TAP3|REG_dfe_adapt_en_TAP4),0);
		hdmi_mask(REG_DFE_CTRL_RA_reg,~(REG_dfe_adapt_en_TAP0|REG_dfe_adapt_en_TAP1|REG_dfe_adapt_en_TAP2|REG_dfe_adapt_en_TAP3|REG_dfe_adapt_en_TAP4),0);
		if (lane_mode == HDMI_4LANE)
		{
			hdmi_mask(REG_DFE_CTRL_CKA_reg,~(REG_dfe_adapt_en_TAP0|REG_dfe_adapt_en_TAP1|REG_dfe_adapt_en_TAP2),0);
		}
	}
}


void lib_hdmi_dfe_rst(unsigned char nport, unsigned char rst)
{
	#if 0 //can not reset  hd21 no tap0

	#endif

}


