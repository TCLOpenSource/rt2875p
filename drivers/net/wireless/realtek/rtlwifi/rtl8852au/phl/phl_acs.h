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
#ifndef _PHL_ACS_H_
#define _PHL_ACS_H_

/* environment monitor time should be smaller than scan duration time */
#define ACS_ENV_MNTR_TIME(time) ((time) * 8 / 10)

struct phl_acs_parm {
	u8 idx; /* idx of phl_acs_chnl_tbl */
	u16 monitor_time;
	bool nhm_include_cca;
};

struct phl_acs_info {
	u8 clm_ratio[MAX_ACS_INFO];
	u8 nhm_pwr[MAX_ACS_INFO];
	u8 nhm_rpt[MAX_ACS_INFO][NHM_RPT_NUM];
};

void phl_acs_mntr_trigger(struct phl_info_t *phl_info, struct phl_acs_parm *parm);
void phl_acs_mntr_result(struct phl_info_t *phl_info, struct phl_acs_parm *parm);
enum rtw_phl_status phl_acs_info_init(struct phl_info_t *phl_info);
void phl_acs_info_deinit(struct phl_info_t *phl_info);

#endif /*_PHL_ACS_H_*/
