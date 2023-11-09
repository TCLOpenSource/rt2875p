/******************************************************************************
 *
 *   Copyright(c) 2014 Realtek Semiconductor Corp. All rights reserved.
 *
 *   @author danielwei@realtek.com
 *
 *****************************************************************************/

#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/poll.h>
#include <asm/cacheflush.h>
#include <linux/proc_fs.h>
#include <linux/i2c.h>

#include "comm.h"
#include "tuner.h"
#include "demod.h"
#include "demod_rtk_all.h"
#include "demod_rtk_common.h"
#include "DemodMgr.h"
#include "rtk_demod_khal_transform_common.h"

#include "khal_common.h"
#include "khal_demod_common.h"

#include "khal_demod_vqi.h"
#include "khal_demod_dvb.h"
#include "khal_demod_isdbt.h"


int KHAL_DEMOD_VQI_DVB_ISDBT_Initialize(void)
{
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_VQI_DVB_ISDBT_Initialize @@@\n");

       KHAL_DEMOD_VQI_Initialize();
       KHAL_DEMOD_DVB_Initialize();
       KHAL_DEMOD_VQI_ISDBT_Initialize();

	return API_OK;

}
EXPORT_SYMBOL(KHAL_DEMOD_VQI_DVB_ISDBT_Initialize);



