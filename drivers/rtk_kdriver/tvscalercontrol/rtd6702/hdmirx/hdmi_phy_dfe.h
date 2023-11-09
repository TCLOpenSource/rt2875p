
#ifndef __HDMI_PHY_DFE_H_
#define __HDMI_PHY_DFE_H_


void lib_hdmi_set_dfe_close(unsigned char nport,unsigned char lane_mode);
void lib_hdmi_dfe_record_enable(unsigned char nport, unsigned char onoff);
unsigned char lib_hdmi_dfe_is_record_enable(unsigned char nport);
void lib_hdmi_dfe_init_vth(unsigned char nport, unsigned char bgrc, unsigned char value);
void lib_hdmi_dfe_init_tap0(unsigned char nport, unsigned char bgrc, unsigned char value);
void lib_hdmi_dfe_init_tap1(unsigned char nport, unsigned char bgrc, unsigned char value);
void lib_hdmi_dfe_init_tap2(unsigned char nport, unsigned char bgrc, signed char value);
void lib_hdmi_dfe_init_tap3(unsigned char nport, unsigned char bgrc, signed char value);
void lib_hdmi_dfe_init_tap4(unsigned char nport, unsigned char bgrc, signed char value);
void lib_hdmi_dfe_init_le(unsigned char nport, unsigned char bgrc, unsigned char value);
void lib_hdmi_dfe_load_in_adp(unsigned char nport, unsigned char bgrc, unsigned int option);
unsigned char lib_hdmi_dfe_get_vth(unsigned char nport, unsigned char lane);
unsigned char lib_hdmi_dfe_get_vthp(unsigned char nport, unsigned char lane);
unsigned char lib_hdmi_dfe_get_vthn(unsigned char nport, unsigned char lane);
unsigned char lib_hdmi_dfe_get_tap0(unsigned char nport, unsigned char lane);
unsigned char lib_hdmi_dfe_get_tap0max(unsigned char nport, unsigned char lane);
unsigned char lib_hdmi_dfe_get_tap0min(unsigned char nport, unsigned char lane);
unsigned char lib_hdmi_dfe_get_tap1(unsigned char nport, unsigned char lane);
unsigned char lib_hdmi_dfe_get_tap1max(unsigned char nport, unsigned char lane);
unsigned char lib_hdmi_dfe_get_tap1min(unsigned char nport, unsigned char lane);
char lib_hdmi_dfe_get_tap2(unsigned char nport, unsigned char lane);
char lib_hdmi_dfe_get_tap3(unsigned char nport, unsigned char lane);
char lib_hdmi_dfe_get_tap4(unsigned char nport, unsigned char lane);
char lib_hdmi_dfe_get_tap2max(unsigned char nport, unsigned char lane);
char lib_hdmi_dfe_get_tap2min(unsigned char nport, unsigned char lane);
char lib_hdmi_dfe_get_tap3max(unsigned char nport, unsigned char lane);
char lib_hdmi_dfe_get_tap3min(unsigned char nport, unsigned char lane);
char lib_hdmi_dfe_get_tap4max(unsigned char nport, unsigned char lane);
char lib_hdmi_dfe_get_tap4min(unsigned char nport, unsigned char lane);
unsigned char lib_hdmi_dfe_get_le(unsigned char nport, unsigned char lane);
unsigned char lib_hdmi_dfe_get_lemax(unsigned char nport, unsigned char lane);
unsigned char lib_hdmi_dfe_get_lemin(unsigned char nport, unsigned char lane);
void lib_hdmi_dfe_update_le(unsigned char nport, unsigned char eq_ch0, unsigned char eq_ch1, unsigned char eq_ch2);

void lib_hdmi_load_le_max(unsigned char nport,unsigned char lane_mode);
void lib_hdmi_dfe_examine_le_coef(unsigned char nport, unsigned int b_clk,unsigned char lane_mode);
void newbase_hdmi_tap2_adjust_with_le(unsigned char nport, unsigned int b_clk,unsigned char lane_mode);
int  newbase_hdmi_dfe_threshold_check(unsigned char nport,unsigned int b_clk,unsigned char lane_mode);
void lib_hdmi_dfe_rst(unsigned char nport, unsigned char rst);


//HDMI2.1 PHY DFE ADAPT
void lib_hdmi_vth_set(unsigned char nport,unsigned char lane_mode,unsigned char value);
void lib_hdmi_load_tap0_max(unsigned char nport,unsigned char lane_mode);
void lib_hdmi_load_tap1_add(unsigned char nport,unsigned char lane_mode,unsigned char value);
void lib_hdmi_load_le_add(unsigned char nport,unsigned char lane_mode,unsigned char value);
void lib_dfe_tap0_le_adp_en(unsigned char nport, unsigned char lane_mode, unsigned char en);
void lib_acdr_dfe_tap_all_adp_en(unsigned char nport,unsigned char frl_mode,unsigned char lane_mode,unsigned char en);
void lib_dfe_tap0_disable(unsigned char nport, unsigned char lane_mode);
void lib_dfe_le_disable(unsigned char nport,unsigned char lane_mode);


#endif

