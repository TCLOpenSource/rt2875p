/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2010 by Chien-An Lin <colin@realtek.com.tw>
 *
 * RTC driver for Realtek series CPUs.
 */
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/rtc.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <rtd_log/rtd_module_log.h>       /* rtd_pr_rtc_info() */
#if IS_ENABLED(CONFIG_RTK_KDRV_SB2)
#include <rtk_sb2_callback.h>
#endif

extern  int rtk_rtc_early_init(void);
extern  int rtk_alarm_init(void);
extern  int rtk_rtc_mips_set_mmss(unsigned long nowtime);
extern void rtk_rtc_read_persistent_clock(struct timespec64 *ts);
extern void rtk_rtc_read_alarm_persistent_clock(struct timespec64 *ts);
extern  int rtk_rtc_set_alarm_mmss(unsigned long nowtime);
extern void rtk_rtc_alarm_aie_enable(int state);
extern  int rtk_rtc_alarm_aie_state(void);

#define RTC_RTK_DEV_ID      0
#define RTC_RTK_NAME        "rtc-rtk"


/*rtk rtc io command*/
#define RTC_WKALM_ON   _IO('p', 0x20)
#define RTC_WKALM_OFF  _IO('p', 0x21)

/*g_is_walarm_off = 1 means disable rtc alarm wakeup funciton.*/
int g_rtc_is_walarm_off = 0;

static struct platform_device *rtc_pdev;

struct timer_list rtk_rtc_timer;
struct timespec64 *p_ts1;

static int rtk_rtc_read_time(struct device *dev, struct rtc_time *time)
{
	struct timespec64 ts;

	rtk_rtc_read_persistent_clock(&ts);
	rtc_time64_to_tm(ts.tv_sec, time);

	return 0;
}

static int rtk_rtc_set_time(struct device *dev, struct rtc_time *time)
{
	unsigned long cur_sec;

	cur_sec = mktime64(time->tm_year + 1900, time->tm_mon + 1, time->tm_mday,
			time->tm_hour, time->tm_min, time->tm_sec);
	rtk_rtc_mips_set_mmss(cur_sec);

	return 0;
}

static int rtk_rtc_read_alarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	struct timespec64 ts;

	rtk_rtc_read_alarm_persistent_clock(&ts);
	rtc_time64_to_tm(ts.tv_sec, &alrm->time);
	alrm->enabled = rtk_rtc_alarm_aie_state();

	return 0;
}

static int rtk_rtc_set_alarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	unsigned long cur_sec;

 	rtk_rtc_alarm_aie_enable(0);

	if(g_rtc_is_walarm_off){
		rtd_pr_rtc_info("[%s]rtc alarm wakeup function was disabled!!!\n", __func__);
		return 0;
	}

	cur_sec = mktime64(alrm->time.tm_year + 1900, alrm->time.tm_mon + 1, alrm->time.tm_mday,
			alrm->time.tm_hour, alrm->time.tm_min, alrm->time.tm_sec);
	rtk_rtc_set_alarm_mmss(cur_sec);
	if(alrm->enabled)
		rtk_rtc_alarm_aie_enable(1);

	return 0;
}

static int rtk_rtc_ioctl(struct device *dev, unsigned int cmd,
	unsigned long arg)
{
	switch (cmd) {
	case RTC_AIE_ON:
		rtk_rtc_alarm_aie_enable(1);
		break;
	case RTC_AIE_OFF:
		rtk_rtc_alarm_aie_enable(0);
		break;
	case RTC_WKALM_ON:
		g_rtc_is_walarm_off = 0;
		break;
	case RTC_WKALM_OFF:
		g_rtc_is_walarm_off = 1;
		break;
	default:
		return -ENOIOCTLCMD;
	}
	return 0;
}

static int rtk_rtc_alarm_irq_enable(struct device *dev, unsigned int enable)
{
    if(enable) {
        rtk_rtc_alarm_aie_enable(1);
    }
    else {
        rtk_rtc_alarm_aie_enable(0);
    }
    return 0;
}

static const struct rtc_class_ops rtk_rtc_ops = {
	.read_time	= rtk_rtc_read_time,
	.set_time	= rtk_rtc_set_time,
	.read_alarm	= rtk_rtc_read_alarm,
	.set_alarm	= rtk_rtc_set_alarm,
	.ioctl		= rtk_rtc_ioctl,
    .alarm_irq_enable = rtk_rtc_alarm_irq_enable,
};

void check_rtc_timerfunc(struct timer_list * data)
{
	struct timespec64 ts2;

	//read_persistent_clock64(&ts2);
	rtk_rtc_read_persistent_clock(&ts2);
	if(p_ts1->tv_sec == ts2.tv_sec) {
		rtd_pr_rtc_info("RTK rtc cannot work.\n");
	} else {
		rtd_pr_rtc_info("RTK rtc can work.\n");
	}

	kfree(p_ts1);
}

#if IS_ENABLED(CONFIG_RTK_KDRV_SB2)
//the real rtk_rtc_sb2_timeout_debug_info() funciton was defined in rtk_rtc.c.
void __weak rtk_rtc_sb2_timeout_debug_info(void)
{
	return ;
}

SB2_DEBUG_INFO_DECLARE (sb2_rtc_callback,rtk_rtc_sb2_timeout_debug_info);
#endif
static int rtk_rtc_probe(struct platform_device *pdev)
{
	//struct resource *res;
	struct rtc_device *rtc;
	int retval;
#if 0
	if (pdev->num_resources != 1) {
		rtd_pr_rtc_err("%s(): Number of resource not equal to one.\n", __func__);
		return -EBUSY;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		rtd_pr_rtc_err("%s(): Get resource fail.\n", __func__);
		return -EBUSY;
	}

	if(!request_mem_region(res->start, resource_size(res), pdev->name)) {
		rtd_pr_rtc_err("%s: RTC registers at %llx are not free\n", pdev->name, res->start);
		return -EBUSY;
	}
#endif

	rtc = devm_rtc_allocate_device(&pdev->dev);
    // devm_rtc_device_register( &pdev->dev, pdev->name, &rtk_rtc_ops, THIS_MODULE);
	if (IS_ERR(rtc)) {
		retval = PTR_ERR(rtc);
		rtd_pr_rtc_err("%s: can't register RTC device, err %d\n", pdev->name, retval);
		goto out;
	}
    platform_set_drvdata(pdev, rtc);

    rtc->ops = &rtk_rtc_ops;
    /* M6PRTANPM-241: Disable Periodic IRQ(PIE). but rtc_irq_set_freq isn't exported in GKI kernel.
     *                Need to use ioctl with RTC_PIE_OFF for GKI version. */
	//rtc->max_user_freq = 1;
    //rtc_irq_set_freq(rtc, 1);

	p_ts1 = (struct timespec64*)kmalloc(sizeof(struct timespec64), GFP_KERNEL);
	//read_persistent_clock64(p_ts1);
    rtk_rtc_read_persistent_clock(p_ts1);
	//init_timer(&rtk_rtc_timer); // Use timer_setup instead
	//rtk_rtc_timer.function = check_rtc_timerfunc;
	timer_setup(&rtk_rtc_timer, check_rtc_timerfunc, 0);
	rtk_rtc_timer.expires = jiffies + 2*HZ;
	add_timer(&rtk_rtc_timer);

    //should call this function before rtc_device_register.
    device_init_wakeup(&pdev->dev, 1);

	rtd_pr_rtc_info("rtc: Real Time Clock of Realtek series CPUs\n");

    rtk_rtc_early_init();
    rtk_alarm_init();

#if IS_ENABLED(CONFIG_RTK_KDRV_SB2)
	SB2_DEBUG_INFO_REGISTER (sb2_rtc_callback,rtk_rtc_sb2_timeout_debug_info);
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 11, 0))
	return devm_rtc_register_device(rtc);
#else
	return rtc_register_device(rtc);
#endif

out:
	//release_mem_region(res->start, resource_size(res));
	return retval;
}

static int rtk_rtc_remove(struct platform_device *pdev)
{
//	struct rtc_device *rtc = platform_get_drvdata(pdev);
//	struct resource *res = dev_get_drvdata(&rtc->dev);

#if IS_ENABLED(CONFIG_RTK_KDRV_SB2)
	SB2_DEBUG_INFO_UNREGISTER (sb2_rtc_callback,rtk_rtc_sb2_timeout_debug_info);
#endif
	rtk_rtc_alarm_aie_enable(0);
	//rtc_device_unregister(rtc);
//	release_mem_region(res->start, resource_size(res));
	platform_set_drvdata(pdev, NULL);

	return 0;
}

#ifdef CONFIG_PM
static void rtk_rtc_clear_alarm_flag(struct device *dev)
{
	//disable alarm interrupt, and clear alarm interrupt pending bit.
	rtk_rtc_alarm_aie_enable(0);
	return ;
}

static const struct dev_pm_ops rtc_pm_ops = {
	.complete	= rtk_rtc_clear_alarm_flag,
};
#endif


/* work with hotplug and coldplug */
MODULE_ALIAS("platform:rtc-rtk");
static struct platform_driver rtk_platform_rtc_driver = {
	.probe		= rtk_rtc_probe,
	.remove		= rtk_rtc_remove,
	.driver		= {
		.name	= RTC_RTK_NAME,
		.owner	= THIS_MODULE,
#ifdef CONFIG_PM
		.pm 	= &rtc_pm_ops,
#endif
	},
};

static int __init rtk_rtc_init(void)
{
    int err;

	err = platform_driver_register(&rtk_platform_rtc_driver);
	if (err)
		return err;

	err = -ENOMEM;
	rtc_pdev = platform_device_alloc(RTC_RTK_NAME, RTC_RTK_DEV_ID);
	if (!rtc_pdev)
		goto exit_free_mem;

	err = platform_device_add(rtc_pdev);
	if (err)
		goto exit_device_del;

	return 0;

exit_device_del:
	platform_device_del(rtc_pdev);

exit_free_mem:
	platform_device_put(rtc_pdev);

	platform_driver_unregister(&rtk_platform_rtc_driver);
	return err;


}

static void __exit rtk_rtc_exit(void)
{
    platform_device_unregister(rtc_pdev);
	platform_driver_unregister(&rtk_platform_rtc_driver);
}

module_init(rtk_rtc_init);
module_exit(rtk_rtc_exit);

MODULE_AUTHOR("Colin <colin@realtek.com.tw>");
MODULE_DESCRIPTION("Realtek series RTC driver");
MODULE_LICENSE("GPL v2");
