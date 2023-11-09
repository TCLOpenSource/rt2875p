/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 *
 * The OPP code in function cpu0_set_target() is reused from
 * drivers/cpufreq/omap-cpufreq.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/slab.h>

#include <rtd_log/rtd_module_log.h>
#include <rtk_kdriver/rtk_clock.h>
#include <rtk_kdriver/rtk_thermal_sensor.h>
#include <mach/platform.h>
#include <linux/version.h>
#ifdef CONFIG_RTK_LOW_POWER_MODE
#include <linux/suspend.h>
#include <linux/cpu.h>
#endif

static int rtk_cpufreq_init(struct cpufreq_policy *policy);

static struct cpufreq_frequency_table *freq_table;

static struct clk *cpu_clk;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
static int rtk_verify_speed(struct cpufreq_policy_data *policy)
#else
static int rtk_verify_speed(struct cpufreq_policy *policy)
#endif
{
	return cpufreq_frequency_table_verify(policy, freq_table);
}

static unsigned int rtk_get_speed(unsigned int cpu)
{
	return __rtk_clk_get_rate(cpu_clk);
}

int rtk_cpu_dvfs_disable = 0;
static int __init rtk_set_cpu_dvfs_disable(char *str)
{
	if (!strncmp(str, "1", 1))
		rtk_cpu_dvfs_disable = 1;
	else if(!strncmp(str, "0", 1))
		rtk_cpu_dvfs_disable = 0;
	return 1;
}
__setup("no_cpu_dvfs=", rtk_set_cpu_dvfs_disable);


#ifdef CONFIG_RTK_SUPPORT_BOOST_MODE_TIMEOUT

#define RTK_BOOST_DEFAULT_TIMEOUT_SEC (5)
#define RTK_BOOST_MAX_TIMEOUT_SEC (12 * 60)

struct workqueue_struct *rtk_boost_workqueue;
struct work_struct rtk_boost_work;
struct timer_list rtk_boost_timer;
unsigned int rtk_bootst_timeout_val = 0; /*0 unset, use default, otherwise, set by AP.*/
unsigned int rtk_bootst_timeout_max = RTK_BOOST_MAX_TIMEOUT_SEC;
unsigned int rtk_bootst_timeout_default = RTK_BOOST_DEFAULT_TIMEOUT_SEC;
unsigned long rtk_bootst_timeout_end_time = 0;

extern int init_cpu_freq(char *name, char *path);
int get_cpufre_boot_enable(void);

void rtk_boost_exit_func(struct work_struct *work)
{
	if(init_cpu_freq("0","/sys/devices/system/cpu/cpufreq/boost")!=0)
		rtd_pr_cpu_freq_err("disable boost mode fail(%s : %d)\n", __func__, __LINE__);
#if 0
	else
		rtd_pr_cpu_freq_notice("(%s)\n", __func__);
#endif
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
void rtk_boost_timer_event(struct timer_list *t)
#else
void rtk_boost_timer_event(unsigned long data)
#endif
{
	rtk_bootst_timeout_end_time = 0;
	del_timer( &rtk_boost_timer);
	queue_work(rtk_boost_workqueue, &rtk_boost_work);
}


void rtk_boost_reset_timeout_value(unsigned int val_sec)
{
	if(val_sec > rtk_bootst_timeout_max)
		val_sec = rtk_bootst_timeout_max;

	if(timer_pending(&rtk_boost_timer) == 0) // no timer
		rtk_bootst_timeout_val = val_sec;
	else
	{
		if(time_after(jiffies + HZ *val_sec , rtk_bootst_timeout_end_time)) //check extend
		{
			mod_timer(&rtk_boost_timer, jiffies + HZ *val_sec);
			rtk_bootst_timeout_end_time = jiffies + HZ * val_sec;
		}
		rtk_bootst_timeout_val = 0;
	}
}

#if 0
void boost_protect_timer_suspend(void) //when enter suspend
{
	if(get_cpufre_boot_enable() == 1)
	{
		del_timer_sync( &rtk_boost_timer);
		queue_work(rtk_boost_workqueue, &rtk_boost_work);
	}
}
#endif

void rtk_boost_setup_timer(void)
{
	//rtd_pr_cpu_freq_notice("(%s):(%d)\n", __func__, timer_pending(&rtk_boost_timer));
	//if(timer_pending(&rtk_boost_timer) == 0) // no timer
	{
		if(rtk_bootst_timeout_val == 0)
		{
			mod_timer(&rtk_boost_timer, jiffies + HZ * rtk_bootst_timeout_default);
			rtk_bootst_timeout_end_time = jiffies + HZ * rtk_bootst_timeout_default;
		}
		else
		{
			mod_timer(&rtk_boost_timer, jiffies + HZ * rtk_bootst_timeout_val);
			rtk_bootst_timeout_end_time = jiffies + HZ * rtk_bootst_timeout_val;
			rtk_bootst_timeout_val = 0; //reset timeout to unset
		}
	}
}

void rtk_boost_del_timer(void)
{
	//rtd_pr_cpu_freq_notice("(%s):(%d)\n", __func__, timer_pending(&rtk_boost_timer));
	rtk_bootst_timeout_end_time = 0;
	del_timer( &rtk_boost_timer);
}
ssize_t rtk_boost_timeout_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", rtk_bootst_timeout_val);
}

ssize_t rtk_boost_timeout_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int ret;
	unsigned int value;
	ret = sscanf(buf, "%u", &value);
	if (ret != 1) {
		return -EINVAL;
	}
	else
		rtk_boost_reset_timeout_value(value);
	return count;
}
ssize_t rtk_boost_max_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", rtk_bootst_timeout_max);
}
#if 0
ssize_t rtk_boost_max_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int ret;
	static int set = 0;

	if(set == 0 )
	{
		ret = sscanf(buf, "%u", &rtk_bootst_timeout_max);
		if (ret != 1)
			return -EINVAL;
		else
			set = 1;
	}
	return count;
}
ssize_t rtk_boost_default_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int ret;
	static int set = 0;

	if(set == 0 )
	{
		ret = sscanf(buf, "%u", &rtk_bootst_timeout_default);
		if (ret != 1)
			return -EINVAL;
		else
			set = 1;
	}
	return count;
}
#endif
ssize_t rtk_boost_default_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", rtk_bootst_timeout_default);
}

#else //CONFIG_RTK_SUPPORT_BOOST_MODE_TIMEOUT
unsigned int rtk_bootst_timeout_val = 0;
unsigned int rtk_bootst_timeout_max = 0;
unsigned int rtk_bootst_timeout_default = 0;
ssize_t rtk_boost_timeout_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", rtk_bootst_timeout_val);
}

ssize_t rtk_boost_timeout_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int ret;
	ret = sscanf(buf, "%u", &rtk_bootst_timeout_val);
	if (ret != 1)
		return -EINVAL;

	return count;
}
ssize_t rtk_boost_max_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", rtk_bootst_timeout_max);
}
#if 0
ssize_t rtk_boost_max_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int ret;
	ret = sscanf(buf, "%u", &rtk_bootst_timeout_max);
	if (ret != 1)
		return -EINVAL;

	return count;
}

ssize_t rtk_boost_default_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int ret;
	ret = sscanf(buf, "%u", &rtk_bootst_timeout_default);
	if (ret != 1)
		return -EINVAL;

	return count;
}
#endif
ssize_t rtk_boost_default_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", rtk_bootst_timeout_default);
}



#endif //CONFIG_RTK_SUPPORT_BOOST_MODE_TIMEOUT

static int rtk_set_target(struct cpufreq_policy *policy,
			   unsigned int target_freq, unsigned int relation)
{
	struct cpufreq_freqs freqs;
	unsigned long freq_kHz;
	unsigned int index;
	int i;
	int ret;

	/*if(is_platform_reset_freq())
	{
		__rtk_free_cpufreq_table(&freq_table);
		__rtk_init_cpufreq_table(&freq_table);
		rtk_cpufreq_init(policy);
		clear_platform_reset_freq();
	}*/

	if(rtk_cpu_dvfs_disable == 1)
		return 0;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
        index = ret = cpufreq_frequency_table_target(policy, target_freq,
                                            relation);
	if (ret < 0) {
#else
	ret = cpufreq_frequency_table_target(policy, freq_table, target_freq,
					     relation, &index);
	if (ret) {
#endif
		rtd_pr_cpu_freq_err("failed to match target freqency %d: %d\n",
		       target_freq, ret);
		return ret;
	}

	freq_kHz = freq_table[index].frequency;

	freqs.new = freq_kHz;
	freqs.old = __rtk_clk_get_rate(cpu_clk);

	if (freqs.old == freqs.new)
		return 0;


	cpufreq_freq_transition_begin(policy, &freqs);

	for (i = 0; freq_table[i].frequency != CPUFREQ_TABLE_END; i++) {
		if (freq_table[i].frequency == CPUFREQ_ENTRY_INVALID)
			continue;

		if (freqs.new == freq_table[i].frequency)
			break;
	}

	if (freqs.new != freq_table[i].frequency)
		return -ENOTSUPP;
	ret = __rtk_clk_set_rate(cpu_clk, freqs.new);
	if (ret) {
		rtd_pr_cpu_freq_err("failed to set clock rate: %d\n", ret);
		return ret;
	}

	cpufreq_freq_transition_end(policy, &freqs,0);

#ifdef CONFIG_RTK_SUPPORT_BOOST_MODE_TIMEOUT
	if(get_cpufre_boot_enable() == 1)
		rtk_boost_setup_timer();
	else
		rtk_boost_del_timer();
#endif


#if 0
	for_each_online_cpu(cpu) {
		freqs.cpu = cpu;
		cpufreq_notify_transition(policy, &freqs, CPUFREQ_POSTCHANGE);
	}
#endif
	return 0;
}
extern unsigned int rtk_get_boot_freq(void);
static int rtk_cpufreq_init(struct cpufreq_policy *policy)
{
	int ret;

//	if (policy->cpu != 0)
//		return -EINVAL;


	ret = cpufreq_frequency_table_cpuinfo(policy, freq_table);

	if (ret) {
		rtd_pr_cpu_freq_err("invalid frequency table: %d\n", ret);
		return ret;
	}

	cpu_clk->rate = policy->min; // use policy min to set as init value
	policy->cpuinfo.transition_latency = 10000;
	policy->cur = __rtk_clk_get_rate(cpu_clk);

	/*
	 * The driver only supports the SMP configuartion where all processors
	 * share the clock and voltage and clock.  Use cpufreq affected_cpus
	 * interface to have all CPUs scaled together.
	 */
	policy->shared_type = CPUFREQ_SHARED_TYPE_ANY;
	cpumask_setall(policy->cpus);
	cpufreq_generic_init(policy, freq_table, policy->cpuinfo.transition_latency );

#ifdef CONFIG_RTK_SUPPORT_BOOST_MODE
	if (policy_has_boost_freq(policy))
		cpufreq_enable_boost_support();
#endif

	//cpufreq_frequency_table_get_attr(freq_table, policy->cpu);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	cpufreq_table_validate_and_sort(policy);
#else
	cpufreq_table_validate_and_show(policy, freq_table);
#endif
	ret = cpufreq_frequency_table_target(policy, rtk_get_boot_freq(), CPUFREQ_RELATION_H);
	if(ret > 0)
	{
		policy->max = policy->freq_table[ret].frequency;
		rtd_pr_cpu_freq_info("cpufreq: set max %d\n", policy->max);
	}

	rtd_pr_cpu_freq_info("Rtk cpu frequency driver init successfully\n");

	return 0;
}

static int rtk_cpufreq_exit(struct cpufreq_policy *policy)
{
	//cpufreq_frequency_table_put_attr(policy->cpu);

	return 0;
}

static struct freq_attr *rtk_cpufreq_attr[] = {
	&cpufreq_freq_attr_scaling_available_freqs,
	NULL,
};

static struct cpufreq_driver rtk_cpufreq_driver = {
	.flags = CPUFREQ_STICKY,
	.verify = rtk_verify_speed,
	.target = rtk_set_target,
	.get = rtk_get_speed,
	.init = rtk_cpufreq_init,
	.exit = rtk_cpufreq_exit,
	.name = "realtek_cpufreq",
	.attr = rtk_cpufreq_attr,
};

#ifdef CONFIG_RTK_SUPPORT_BOOST_MODE_TIMEOUT
int get_cpufre_boot_enable(void)
{
	return rtk_cpufreq_driver.boost_enabled;
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
static int rtk_cpufreq_notify(struct notifier_block *self, unsigned long action, void *data)
{
	int ret;
	struct cpufreq_policy *policy = data;

	if(action == CPUFREQ_CREATE_POLICY)
	{
		ret = cpufreq_frequency_table_target(policy, rtk_get_boot_freq(), CPUFREQ_RELATION_H);
		if(ret > 0)
		{
			policy->cpuinfo.max_freq = policy->max = policy->freq_table[ret].frequency;
			rtd_pr_cpu_freq_info("new cpufreq: set max %d\n", policy->max);
		}
	}
	return 0;
}

static struct notifier_block rtk_cpufreq_notifier = {
	.notifier_call = rtk_cpufreq_notify,
};
#endif

#ifdef CONFIG_RTK_LOW_POWER_MODE
#include <rtk_kdriver/rtk-kdrv-common.h>
static int rtk_cpufreq_lpm_current_lpm_state = PM_LPM_POWER_MODE_UNKNOW;
static cpumask_var_t downed_cpus;
static void rtk_cpufreq_enter_uniprocessor(void)
{
	int cpu;
	int err;

	if (!cpumask_available(downed_cpus) &&
	    !alloc_cpumask_var(&downed_cpus, GFP_KERNEL)) {
		rtd_pr_cpu_freq_notice("[LPM][CPU] Failed to allocate mask\n");
		goto out;
	}

	get_online_cpus();
	cpumask_copy(downed_cpus, cpu_online_mask);
	cpumask_clear_cpu(cpumask_first(cpu_online_mask), downed_cpus);
	if (num_online_cpus() > 1)
		rtd_pr_cpu_freq_notice("[LPM][CPU] Disabling non-boot CPUs...\n");
	put_online_cpus();

	for_each_cpu(cpu, downed_cpus) {
		err = cpu_down(cpu);
		if (!err)
			rtd_pr_cpu_freq_info("[LPM][CPU] CPU%d is down.\n", cpu);
		else
			rtd_pr_cpu_freq_err("[LPM][CPU] Error taking CPU%d down: %d\n", cpu, err);
	}
out:
	if (num_online_cpus() > 1)
		pr_warning("[LPM][CPU] multiple CPUs still online, may miss events.\n");
}


static void rtk_cpufreq_leave_uniprocessor(void)
{
	int cpu;
	int err;

	if (!cpumask_available(downed_cpus) || cpumask_weight(downed_cpus) == 0)
		return;
	rtd_pr_cpu_freq_notice("[LPM][CPU] Re-enabling CPUs...\n");
	for_each_cpu(cpu, downed_cpus) {
		err = cpu_up(cpu);
		if (!err)
			rtd_pr_cpu_freq_info("[LPM][CPU] enabled CPU%d.\n", cpu);
		else
			rtd_pr_cpu_freq_err("[LPM][CPU] cannot re-enable CPU%d: %d\n", cpu, err);
	}

}

static int rtk_cpufreq_lpm_notifier_fn(struct notifier_block *nb, unsigned long action, void *data)
{
	static int rtk_cpufreq_lpm_cpu_off = 0;
	rtk_cpufreq_lpm_current_lpm_state = action;
	rtd_pr_cpu_freq_err("[LPM][CPU] notifier(%ld)\n", action);

	if(rtk_cpufreq_lpm_current_lpm_state == PM_LPM_POWER_MODE_S2)
	{
		rtk_cpufreq_lpm_cpu_off = 1;
		rtk_cpufreq_enter_uniprocessor();
	}
	else if(rtk_cpufreq_lpm_current_lpm_state == PM_LPM_POWER_MODE_S0)
	{
		if(rtk_cpufreq_lpm_cpu_off == 1)
		{
			rtk_cpufreq_lpm_cpu_off = 0;
			rtk_cpufreq_leave_uniprocessor();
		}
	}

	return 0;
}

static struct notifier_block rtk_cpufreq_lpm_notifier = {
        .notifier_call = rtk_cpufreq_lpm_notifier_fn,
};
#endif //CONFIG_RTK_LOW_POWER_MODE


static int rtk_cpufreq_driver_init(void)
{
	int ret;

	cpu_clk = __rtk_clk_get(NULL, "cpu_clk");
	if (IS_ERR(cpu_clk)) {
		ret = PTR_ERR(cpu_clk);
		rtd_pr_cpu_freq_err("failed to get rtk clock: %d\n", ret);
		goto out_put_node;
	}

//	cpu_clk->rate = rtk_get_boot_freq(); //if the bootfreq not in the freq table, crash on show_time_in_state()

	ret = __rtk_init_cpufreq_table(&freq_table);
	if (ret) {
		rtd_pr_cpu_freq_err("failed to init cpufreq table: %d\n", ret);
		goto out_put_node;
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	cpufreq_register_notifier(&rtk_cpufreq_notifier, CPUFREQ_POLICY_NOTIFIER);
#endif

	ret = cpufreq_register_driver(&rtk_cpufreq_driver);
	if (ret) {
		rtd_pr_cpu_freq_err("failed register driver: %d\n", ret);
		goto out_free_table;
	}

#ifdef CONFIG_RTK_SUPPORT_BOOST_MODE_TIMEOUT
	rtk_boost_workqueue = alloc_workqueue("rtk_cpufreq_boost", 0, 0);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	timer_setup(&rtk_boost_timer, rtk_boost_timer_event, 0);
#else
	setup_timer(&rtk_boost_timer, rtk_boost_timer_event, 0);
#endif
	INIT_WORK(&rtk_boost_work, rtk_boost_exit_func);
#endif

#ifdef CONFIG_RTK_LOW_POWER_MODE
		register_lpm_notifier(&rtk_cpufreq_lpm_notifier);
#endif
	return 0;

out_free_table:
out_put_node:
	__rtk_free_cpufreq_table(&freq_table);
	return ret;
}

fs_initcall(rtk_cpufreq_driver_init);

MODULE_AUTHOR("davidwang <davidwang@realtek.com>");
MODULE_DESCRIPTION("realtek cpufreq driver");
MODULE_LICENSE("GPL");
