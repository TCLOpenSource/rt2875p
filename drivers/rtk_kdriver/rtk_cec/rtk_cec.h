
#ifndef _RTK_CEC_H_
#define _RTK_CEC_H_

#include <linux/clk.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <media/cec.h>
#include "rtk_cec_hw.h"


struct rtk_cec {
    struct cec_adapter	*adap;
    struct device		*dev;
    struct reset_control	*rstc;
    int			irq;
    struct rtk_cec_private priv_data;
};


#endif
