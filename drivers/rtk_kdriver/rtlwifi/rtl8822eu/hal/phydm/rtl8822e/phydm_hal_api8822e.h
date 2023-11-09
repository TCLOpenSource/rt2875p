/******************************************************************************
 *
 * Copyright(c) 2007 - 2017  Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * wlanfae <wlanfae@realtek.com>
 * Realtek Corporation, No. 2, Innovation Road II, Hsinchu Science Park,
 * Hsinchu 300, Taiwan.
 *
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 *****************************************************************************/
#ifndef __INC_PHYDM_API_H_8822E__
#define __INC_PHYDM_API_H_8822E__

#if (RTL8822E_SUPPORT)
/*2023.02.15: Optimize spur eliminate API*/
#define PHY_CONFIG_VERSION_8822E "1.9.7"
/*#define CONFIG_TXAGC_DEBUG_8822E*/

#define INVALID_RF_DATA 0xffffffff
#define INVALID_TXAGC_DATA 0xff
#define L_BND_DEFAULT_8822E 0xd

#define config_phydm_read_rf_check_8822e(data) ((data) != INVALID_RF_DATA)
#define config_phydm_read_txagc_check_8822e(data) ((data) != INVALID_TXAGC_DATA)

enum agc_tab_sel_8822e {
	OFDM_2G_8822E			= 0,
	OFDM_5G_LOW_BAND_8822E		= 1,
	OFDM_5G_MID_BAND_8822E		= 2,
	OFDM_5G_HIGH_BAND_8822E		= 3,
	OFDM_2G_BTC_8822E		= 4,
	OFDM_2G_ISO_BTC_8822E		= 5,
	CCK_8822E			= 8,
	CCK_BTC_8822E			= 9
};

struct txagc_table_8822e {
	u8 ref_pow_cck[2];
	u8 ref_pow_ofdm[2];
	s8 diff_t[NUM_RATE_AC_2SS]; /*by rate differential table*/
};

struct tx_path_en_8822e {
	u8 tx_path_en_ofdm_1sts;
	u8 tx_path_en_ofdm_2sts;
	u8 tx_path_en_cck;
	boolean is_path_ctrl_by_bb_reg;
	boolean stop_path_div;
};

struct rx_path_en_8822e {
	u8 rx_path_en_ofdm;
	u8 rx_path_en_cck;
};

boolean phydm_chk_pkg_set_valid_8822e(struct dm_struct *dm,
				      u8 ver_bb, u8 ver_rf);

u32 config_phydm_read_rf_reg_8822e(struct dm_struct *dm, enum rf_path path,
				   u32 reg_addr, u32 bit_mask);

boolean config_phydm_write_rf_reg_8822e(struct dm_struct *dm, enum rf_path path,
					u32 reg_addr, u32 bit_mask, u32 data);

boolean phydm_write_txagc_1byte_8822e(struct dm_struct *dm, u32 pw_idx,
				      u8 hw_rate);

boolean config_phydm_write_txagc_ref_8822e(struct dm_struct *dm, u8 power_index,
					   enum rf_path path,
					   enum PDM_RATE_TYPE rate_type);

boolean config_phydm_write_txagc_diff_8822e(struct dm_struct *dm,
					    s8 power_index1, s8 power_index2,
					    s8 power_index3, s8 power_index4,
					    u8 hw_rate);

#ifdef CONFIG_TXAGC_DEBUG_8822E
void phydm_txagc_tab_buff_show_8822e(struct dm_struct *dm);
#endif

s8 config_phydm_read_txagc_diff_8822e(struct dm_struct *dm, u8 hw_rate);

u8 config_phydm_read_txagc_8822e(struct dm_struct *dm, enum rf_path path,
				 u8 hw_rate, enum PDM_RATE_TYPE rate_type);

void phydm_get_tx_path_en_setting_8822e(struct dm_struct *dm,
					struct tx_path_en_8822e *path);

void phydm_get_rx_path_en_setting_8822e(struct dm_struct *dm,
					struct rx_path_en_8822e *path);

void phydm_config_tx_path_8822e(struct dm_struct *dm, enum bb_path tx_path_2ss,
				enum bb_path tx_path_sel_1ss,
				enum bb_path tx_path_sel_cck);

boolean config_phydm_trx_mode_8822e(struct dm_struct *dm,
				    enum bb_path tx_path_en,
				    enum bb_path rx_path,
				    enum bb_path tx_path_sel_1ss);

void phydm_set_agc_table_8822e(struct dm_struct *dm, boolean bt_is_linked);

boolean config_phydm_switch_band_8822e(struct dm_struct *dm, u8 central_ch);

boolean config_phydm_switch_channel_8822e(struct dm_struct *dm, u8 central_ch);

boolean config_phydm_switch_bandwidth_8822e(struct dm_struct *dm, u8 pri_ch,
					    enum channel_width bw);

boolean config_phydm_switch_channel_bw_8822e(struct dm_struct *dm,
					     u8 central_ch, u8 primary_ch_idx,
					     enum channel_width bandwidth);

void phydm_set_dac_clock_8822e(struct dm_struct *dm, u8 central_ch);

void phydm_i_only_setting_8822e(struct dm_struct *dm, boolean en_i_only,
				boolean en_before_cca);

boolean phydm_1rcca_setting_8822e(struct dm_struct *dm, boolean en_1rcca);

void phydm_invld_pkt_setting_8822e(struct dm_struct *dm, boolean en_invld_pkt);

void phydm_cck_gi_bound_8822e(struct dm_struct *dm);

void phydm_ch_smooth_setting_8822e(struct dm_struct *dm, boolean en_ch_smooth);

u16 phydm_get_dis_dpd_by_rate_8822e(struct dm_struct *dm);

void phydm_set_auto_nbi_8822e(struct dm_struct *dm, boolean en_auto_nbi);

boolean config_phydm_parameter_init_8822e(struct dm_struct *dm,
					  enum odm_parameter_init type);

boolean phydm_chk_bb_state_idle_8822e(struct dm_struct *dm);

u16 phydm_get_gpio_setting_by_rfe_ctrl_8822e(struct dm_struct *dm);

#if CONFIG_POWERSAVING
boolean phydm_8822e_lps(struct dm_struct *dm, boolean enable_lps);
#endif /* #if CONFIG_POWERSAVING */

void config_phydm_set_txagc_to_hw_8822e(struct dm_struct *dm);

boolean config_phydm_write_txagc_8822e(struct dm_struct *dm, u32 power_index,
				       enum rf_path path, u8 hw_rate);

void phydm_set_txagc_by_table_8822e(struct dm_struct *dm,
				    struct txagc_table_8822e *tab);

void phydm_get_txagc_ref_and_diff_8822e(struct dm_struct *dm,
					u8 txagc_buff[2][NUM_RATE_AC_2SS],
					u16 length,
					struct txagc_table_8822e *tab);

void phydm_tx_triangular_shap_cfg_8822e(struct dm_struct *dm, boolean is_2g_ch);
#endif /* RTL8822E_SUPPORT */
#endif /*  __INC_PHYDM_API_H_8822E__ */
