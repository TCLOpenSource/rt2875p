/*****************************************************************************
 *
 *   Realtek Delivery driver
 *
 *   Copyright(c) 2019 Realtek Semiconductor Corp. All rights reserved.
 *
 *****************************************************************************/
#include <generated/autoconf.h>
#include <linux/uaccess.h>
#include <asm/cacheflush.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pageremap.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/string.h>
#include "rtkdelivery.h"
#include "rtkdelivery_export.h"
#include "rtkdelivery_debug.h"
#include <rtk_kdriver/RPCDriver.h>   /* register_kernel_rpc, send_rpc_command */

#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
#include <VideoRPC_System.h>
#else
#include <rpc/VideoRPC_System.h>
#endif

#include <tp/tp_drv_global.h>

static void IOCTL_ShutDownModule(void)
{
	int i;
	if (delivery_device->pChArray) {
		for (i = 0; i < delivery_device->chNum; i++) {
			delivery_channel *pCh = &delivery_device->pChArray[i];
			if (pCh->state != DELIVERY_STATE_STOP) {
				pCh->state = DELIVERY_STATE_STOP;
				pCh->thread_task = 0;
			}

			if (pCh->isInit) {

				/* hw tp */
				RHAL_TPStreamControl(pCh->tunerHandle, TP_STREAM_STOP);
				//RHAL_TPUninit(pCh->tunerHandle);
				mutex_destroy(&pCh->mutex);
			}
		}
		mutex_destroy(&delivery_device->mutex);
		kfree(delivery_device->pChArray);
		delivery_device->pChArray = NULL;
	}

}

int IOCTL_Delivery_InitModule ()
{
	int i, ret;
	delivery_function_call(CHANNEL_UNKNOWN);

	/* don't init module twice. */
	if (delivery_device->pChArray)
		return 0;

	create_tp_controller();

	RHAL_GetHardwareInfo(&delivery_device->tpInfo);

	delivery_device->chNum         = delivery_device->tpInfo.tp_num;

	delivery_device->pChArray = kmalloc(PAGE_ALIGN(sizeof(delivery_channel)*delivery_device->chNum), GFP_KERNEL);
	if (!(delivery_device->pChArray)) {
		goto demux_init_fail;
	}
	memset(delivery_device->pChArray, 0, PAGE_ALIGN(sizeof(delivery_channel)*delivery_device->chNum));
	///////////////////////////////////////////////////////////////////////////////////
	mutex_init(&delivery_device->mutex);

	/* pcr recovery */
	for (i = 0; i < delivery_device->chNum; i++) {
		delivery_channel *pCh = &delivery_device->pChArray[i];

		mutex_init(&pCh->mutex);

		/* hw tp */
		RHAL_TPInit(pCh->tunerHandle);

		pCh->state = DELIVERY_STATE_RUNNING;
		pCh->isInit  = 1;

	}

	delivery_device->isInit = 1;
	return 0;

demux_init_fail:
	ret = -ENOMEM;
	IOCTL_ShutDownModule();
	delivery_err(CHANNEL_UNKNOWN, "DEMUX init Fail @#@#!!!!!!!!!!@#@#\n");
	BUG();
	return ret;
}
int IOCTL_Delivery_UnInitModule(void)
{
	delivery_function_call(CHANNEL_UNKNOWN);

	if (delivery_device->pChArray) {
		IOCTL_ShutDownModule();
	}

	delivery_device->isInit = 0;
	return 0;
}

int IOCTL_Delivery_STDResetInfoPoll(void)
{

	delivery_function_call(CHANNEL_UNKNOWN);

	return 0;
}


