/*
 * Realtek Audio driver debug utility
 *
 * Copyright (c) 2014 Ivan <ivanlee@realtek.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License.
 *
 * History:
 *
 * 2014-09-23:  Ivan: first version
 */

#include <generated/autoconf.h>
#include <linux/string.h>

#include <rtk_kdriver/rtkaudio.h>
#include <rtk_kdriver/RPCDriver.h>
#include <linux/pageremap.h>

#include <rtk_kdriver/rtkaudio_debug.h>
#include "audio_inc.h"

extern int drv_debug;

void rtkaudio_send_string(const char* pattern, int length)
{
	ulong ret = S_OK;
	ulong vir_addr;
	ulong phy_addr;
	char *command = NULL;

	vir_addr = (ulong)dvr_malloc_uncached_specific(length+1, GFP_DCU1, (void **)&command);

	if (!vir_addr) {
		rtd_pr_adsp_err("[%s %d]alloc memory fail\n", __func__, __LINE__);
		return;
	}
	phy_addr = (ulong)dvr_to_phys((void *)vir_addr);
	memset((void *)command, '\0', length+1);
	memcpy((void *)command, (void *)pattern, length);

    if (drv_debug)
        rtd_pr_adsp_debug("rtkaudio: send(%s), length(%d)\n", command, length);
	if (send_rpc_command(RPC_AUDIO, RPC_ACPU_DEBUG_COMMAND, phy_addr,
		length, &ret)) {
		rtd_pr_adsp_err("rtkaudio: send(%s) fail %d\n", pattern, (int)ret);
	}

	dvr_free((void *) vir_addr);
	return;
}
