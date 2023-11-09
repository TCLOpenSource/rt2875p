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

#include "../halbb_precomp.h"
#ifdef BB_8852A_CAV_SUPPORT

void halbb_cfg_rf_reg_8852a(struct bb_info *bb, u32 addr, u32 data,
			     enum rf_path rf_path, u32 reg_addr)
{
}

void halbb_cfg_rf_radio_a_8852a(struct bb_info *bb, u32 addr, u32 data)
{
}

void halbb_cfg_rf_radio_b_8852a(struct bb_info *bb, u32 addr, u32 data)
{
}

void halbb_cfg_bb_phy_8852a(struct bb_info *bb, u32 addr, u32 data,
			    enum phl_phy_idx phy_idx)
{
#ifdef HALBB_DBCC_SUPPORT
	u32 ofst = 0;
#endif

	if (addr == 0xfe) {
		halbb_delay_ms(bb, 50);
		BB_DBG(bb, DBG_INIT, "Delay 50 ms\n");
	} else if (addr == 0xfd) {
		halbb_delay_ms(bb, 5);
		BB_DBG(bb, DBG_INIT, "Delay 5 ms\n");
	} else if (addr == 0xfc) {
		halbb_delay_ms(bb, 1);
		BB_DBG(bb, DBG_INIT, "Delay 1 ms\n");
	} else if (addr == 0xfb) {
		halbb_delay_us(bb, 50);
		BB_DBG(bb, DBG_INIT, "Delay 50 us\n");
	} else if (addr == 0xfa) {
		halbb_delay_us(bb, 5);
		BB_DBG(bb, DBG_INIT, "Delay 5 us\n");
	} else if (addr == 0xf9) {
		halbb_delay_us(bb, 1);
		BB_DBG(bb, DBG_INIT, "Delay 1 us\n");
	} else {
		#ifdef HALBB_DBCC_SUPPORT
		if ((bb->hal_com->dbcc_en || bb->bb_dbg_i.cr_dbg_mode_en) &&
		    phy_idx == HW_PHY_1) {
			ofst = halbb_phy0_to_phy1_ofst(bb, addr);
			if (ofst == 0)
				return;
			addr += ofst;
		} else {
			phy_idx = HW_PHY_0;
		}
		BB_DBG(bb, DBG_INIT, "[REG][%d]0x%04X = 0x%08X\n", phy_idx, addr, data);
		#else
		BB_DBG(bb, DBG_INIT, "[REG]0x%04X = 0x%08X\n", addr, data);
		#endif

		halbb_set_reg(bb, addr, MASKDWORD, data);
	}
}

#endif
