/******************************************************************************
 *
 * Copyright(c)2019 Realtek Corporation.
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
#ifndef _HAL_COM_I_H_
#define _HAL_COM_I_H_

enum rtw_hal_status rtw_hal_ppdu_sts_cfg(void *hal, u8 band_idx, bool en);
enum rtw_hal_status rtw_hal_ppdu_sts_init(void *hal, u8 band_idx,
			bool en, u8 appen_info, u8 filter);


#endif /* _HAL_COM_I_H_ */
