#include <linux/kconfig.h>
#include <linux/compiler.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/errno.h>		/* error codes */
#include <linux/types.h>		/* size_t */
#include <linux/delay.h>
#include <linux/device.h>		/* device */
#include <linux/platform_device.h>	/* platform device */
#include <linux/pm.h>			/* power management */

#include <rbus/tv_sb1_ana_reg.h>
#include <rbus/pll27x_reg_reg.h>
#include <rbus/audio_reg.h>
#include <rbus/sys_reg_reg.h>

#include "audio_hw_port.h"
#include "audio_hw_driver.h"
#include "audio_hw_earc_pm.h"
#include "audio_hw_atv.h"
#include "audio_hw_aio.h"
#include "audio_hw_app.h"

#define __ALOG_SUBTAG "pm"

struct audio_hw_earc_pm {
	struct device *dev;
};

void audio_earc_suspend(struct audio_hw_earc_pm *pm);
void audio_earc_resume(struct audio_hw_earc_pm *pm);

struct audio_hw_earc_pm *audio_hw_earc_pm_create(struct device *dev)
{
	struct audio_hw_earc_pm *pm = NULL;

	if (!dev) {
		alog_err("invalid device\n");
		pm = (struct audio_hw_earc_pm *)ERR_PTR(-ENXIO);
		goto exit_error;
	}

	pm = devm_kzalloc(dev, sizeof(struct audio_hw_earc_pm), GFP_KERNEL);
	if (!pm) {
		alog_err("can't allocate pm\n");
		pm = ERR_PTR(-ENOMEM);
		goto exit_error;
	}

	/* keep device */
	pm->dev = dev;

	return pm;

exit_error:
	return pm;
}

void audio_hw_earc_pm_destroy(struct audio_hw_earc_pm *pm)
{
	if (pm)
		pm->dev = NULL;
}

int audio_hw_earc_pm_suspend(struct audio_hw_earc_pm *pm)
{
	struct audio_hw_earc_drvdata *drvdata;

	if (!pm || !pm->dev) {
		alog_err("invalid pm or dev\n");
		return -EINVAL;
	}

	drvdata = dev_get_drvdata(pm->dev);

	/* do suspend below here */
	audio_earc_suspend(pm);

	return 0;
}

int audio_hw_earc_pm_resume(struct audio_hw_earc_pm *pm)
{
	struct audio_hw_earc_drvdata *drvdata;

	if (!pm || !pm->dev) {
		alog_err("invalid pm or dev\n");
		return -EINVAL;
	}

	drvdata = dev_get_drvdata(pm->dev);

	/* do resume below here */
	audio_earc_resume(pm);

	return 0;
}

void audio_earc_suspend(struct audio_hw_earc_pm *pm)
{

}

void audio_earc_resume(struct audio_hw_earc_pm *pm)
{
	Audio_HW_Earc_SetAPHY();
}