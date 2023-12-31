/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/
#ifndef _HALRF_API_H_
#define _HALRF_API_H_
/*@--------------------------[Define] ---------------------------------------*/
#define HALRF_ABS(a,b) ((a>b) ? (a-b) : (b-a))
/*@--------------------------[Enum]------------------------------------------*/
enum phlrf_lna_set {
	PHLRF_LNA_DISABLE = 0,
	PHLRF_LNA_ENABLE = 1,
};

enum halrf_rfk_type {
	RF_BTC_IQK		= 0,
	RF_BTC_LCK		= 1,
	RF_BTC_DPK		= 2,
	RF_BTC_TXGAPK		= 3,
	RF_BTC_DACK		= 4,
	RF_BTC_RXDCK		= 5,
	RF_BTC_TSSI		= 6
};

enum halrf_rfk_process {
	RFK_STOP		= 0,
	RFK_START		= 1,
	RFK_ONESHOT_START	= 2,
	RFK_ONESHOT_STOP	= 3
};

/*@--------------------------[Structure]-------------------------------------*/
 
/*@--------------------------[Prptotype]-------------------------------------*/
struct rf_info;

u32 phlrf_psd_log2base(struct rf_info *rf, u32 val);

void phlrf_rf_lna_setting(struct rf_info *rf, enum phlrf_lna_set type);

void halrf_bkp(struct rf_info *rf, u32 *bp_reg, u32 *bp, u32 reg_num);

void halrf_bkprf(struct rf_info *rf, u32 *bp_reg, u32 bp[][4], u32 reg_num, u32 path_num);

void halrf_reload_bkp(struct rf_info *rf, u32 *bp_reg, u32 *bp, u32 reg_num);
	
void halrf_reload_bkprf(struct rf_info *rf,
		       u32 *bp_reg,
		       u32 bp[][4],
		       u32 reg_num,
		       u8 path_num);

u8 halrf_kpath(struct rf_info *rf, enum phl_phy_idx phy_idx);

void halrf_tmac_tx_pause(struct rf_info *rf, enum phl_band_idx band_idx, bool pause);

void halrf_trigger_thermal(struct rf_info *rf);

u8 halrf_only_get_thermal(struct rf_info *rf, enum rf_path path);

void halrf_thermal_period(struct rf_info *rf);

void halrf_btc_rfk_ntfy(struct rf_info *rf, u8 phy_map, enum halrf_rfk_type type,
			enum halrf_rfk_process process);
void halrf_fcs_init(struct rf_info *rf);
void halrf_fast_chl_sw_backup(struct rf_info *rf, u8 chl_index, u8 t_index);
void halrf_fast_chl_sw_reload(struct rf_info *rf, u8 chl_index, u8 t_index);

/*FW Offload*/
void halrf_wreg_fw(struct rf_info *rf, u32 addr, u32 mask, u32 val);
void halrf_wrf_fw(struct rf_info *rf,
			  enum rf_path path, u32 addr, u32 mask, u32 val);
void halrf_wmac_fw(struct rf_info *rf, enum phl_phy_idx phy,
			u32 addr, u32 mask, u32 val);
void halrf_write_fw_final(struct rf_info *rf);
void  halrf_quick_check_rf(void *rf_void);
#endif
