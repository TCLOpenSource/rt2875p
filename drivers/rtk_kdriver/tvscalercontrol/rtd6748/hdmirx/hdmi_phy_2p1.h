
#ifndef __HDMI_PHY_2P1_H_
#define __HDMI_PHY_2P1_H_

#define HD21_PORT_TOTAL_NUM	2

#define FRL_3G		3000
#define FRL_6G		6000
#define FRL_8G		8000
#define FRL_10G		10000
#define FRL_12G		12000

typedef enum {
	ACDR_MODE,
	DCDR_MODE,
}HDMIRX_CDR_MODE;


typedef enum{
	PS_NORMAL_MODE,
	PS_FAST_SWITCH_MODE,
}HDMIRX_PS_MODE;

extern void lib_hdmi21_z0_calibration(void);
extern void lib_hdmi21_phy_init(void);
extern void lib_hdmi21_z0_set(unsigned char port, unsigned char lane, unsigned char enable);
extern unsigned char newbase_rxphy_frl_measure(unsigned char port);
extern unsigned char newbase_rxphy_tmds_measure(unsigned char port);
extern void newbase_rxphy_frl_job(unsigned char p);
extern void newbase_rxphy_tmds_job(unsigned char p);
extern unsigned char newbase_hdmi_scdc_get_frl_mode(unsigned char port);
extern void newbase_rxphy_set_frl_mode(unsigned char port, unsigned char frl_mode);
extern unsigned char newbase_rxphy_get_frl_mode(unsigned char port);
extern void newbase_hdmi21_set_phy_frl_mode(unsigned port, unsigned char frl_mode);
extern void newbase_hdmi_frl_set_phy(unsigned char port, unsigned char frl_mode);
extern void newbase_hdmi21_phy_pw_saving(unsigned char port);
extern void lib_hdmi_hd21_ltp_phy_adaptive(unsigned char nport, unsigned char lane_num);
extern void lib_hdmi21_hysteresis_en(unsigned char en);
extern void lib_hdmi21_stb_tmds_clk_wakeup_en(unsigned char en);

#if HDMI_FRL_TRANS_DET
extern void lib_hdmi_lane_rate_detect_start(unsigned char nport);
extern void lib_hdmi_lane_rate_detect_stop(unsigned char nport);
extern int lib_hdmi_lane_rate_is_ready(unsigned char nport);
extern int lib_hdmi_get_lane_rate(unsigned char nport);
#endif

extern unsigned char factory_or_power_only_mode_en;
extern unsigned char factory_or_power_only_mode_bch_irq_en;
extern int factory_or_power_only_mode_hd21_eq_ofst;
extern unsigned int scdc_extend[4];

#endif  // __HDMI_PHY_H_

