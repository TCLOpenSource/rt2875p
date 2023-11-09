#ifndef AUDIO_HW_EARC_DRIVER_H
#define AUDIO_HW_EARC_DRIVER_H

#include <linux/types.h>		/* size_t, dev_t */
#include <linux/mutex.h>		/* mutex */
#include <linux/cdev.h>			/* char device */

struct device;
struct class;
struct audio_hw_earc_pm;

struct audio_hw_earc_drvdata {
	dev_t devt;
	struct cdev cdev;
	struct mutex mtx;
	struct class *cls;
	struct device *clsdev;
	struct audio_hw_earc_pm *pm;
};

extern struct audio_hw_earc_drvdata *audio_earc_driver_data;

#endif
