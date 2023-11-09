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
#include <linux/pm_opp.h>
#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <rtd_log/rtd_module_log.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/suspend.h>

#include <rtk_kdriver/rtkaudio.h> /*MK2BU-279*/
#include <rtk_kdriver/rtk_clock.h>
#include <rtk_kdriver/rtk_thermal_sensor.h>
#ifdef CONFIG_RTK_KDRV_CPU_FREQUENCY_MODULE
#include <rtk_kdriver/rtk-kdrv-common.h>
#endif
#include <mach/platform.h>
#include <linux/version.h>
#include <linux/pm_qos.h>

#include <linux/of.h>

static LIST_HEAD(rtk_cpufreq_info_list);

#if 0
int rtk_cpufreq_get_pll(int cpu)
{
	struct rtk_cpufreq_info *info = rtk_cpufreq_info_lookup(cpu);
	if (!info)
	{
		panic("rtk_cpufreq_info_lookup for cpu%d is not initialized.\n", cpu);
		return -EINVAL;
	}
	return info->Ppll;
}

static int rtk_cpufreq_get_R_pll(int cpu)
{

	struct device_node *np = NULL;
	int value[4];

	np = of_find_compatible_node(NULL,NULL,"arm,rtk-cpufreq");
	if (!np)
		panic("%s: failed to find cpufreq table on DT node\n",__func__);

	if(!of_property_read_u32_array(np, "freq", value,4))
	{
		if(cpu == 0)
			return value[0];
		else
			return value[1];
	}
	else
		panic("%s: failed to read opp cpufreq array\n",__func__);
}
#endif

int rtk_cpufreq_get_dsu_pll(int level)
{
	struct device_node *np = NULL;
	int value[3];

	np = of_find_compatible_node(NULL,NULL,"arm,rtk-cpufreq");
	if (!np)
		panic("%s: failed to find cpufreq table on DT node\n",__func__);

	if(!of_property_read_u32_array(np, "freq", value,3))
	{
		if(level == RTK_CPUFREQ_STATE_R)
			return value[0];
		else if(level == RTK_CPUFREQ_STATE_P)
			return value[1];
		else	//RTK_CPUFREQ_STATE_A
			return value[2];
	}
	else
		panic("%s: failed to read opp cpufreq array\n",__func__);
}


struct rtk_cpufreq_info* rtk_cpufreq_info_lookup(int cpu)
{
	struct rtk_cpufreq_info *info;
	list_for_each_entry(info, &rtk_cpufreq_info_list, list_head) {
		if (cpumask_test_cpu(cpu, &info->cpus))
			return info;
	}
	return NULL;
}

bool rtk_check_control_pll_cpu(int cpu)
{
	struct rtk_cpufreq_info *info = rtk_cpufreq_info_lookup(cpu);
	if (!info)
	{
		panic("rtk_cpufreq_info_lookup for cpu%d is not initialized.\n", cpu);
		return -EINVAL;
	}
	return info->control_pll;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
static int rtk_verify_speed(struct cpufreq_policy_data *policy)
#else
static int rtk_verify_speed(struct cpufreq_policy *policy)
#endif
{
	return cpufreq_frequency_table_verify(policy, policy->freq_table);
}

unsigned int rtk_get_speed(unsigned int cpu)
{
	struct cpufreq_policy *policy = cpufreq_cpu_get_raw(cpu);

	if (!policy || IS_ERR(policy->clk))
		panic("%s: No %s associated to cpu: %d\n", __func__, policy ? "clk" : "policy", cpu);

	return __rtk_clk_get_rate(policy->clk);
}
EXPORT_SYMBOL(rtk_get_speed);

int rtk_cpu_dvfs_disable = 0;
#ifdef CONFIG_RTK_KDRV_CPU_FREQUENCY
static int __init rtk_set_cpu_dvfs_disable(char *str)
{
	if (!strncmp(str, "1", 1))
		rtk_cpu_dvfs_disable = 1;
	else if(!strncmp(str, "0", 1))
		rtk_cpu_dvfs_disable = 0;
	return 1;
}
__setup("no_cpu_dvfs=", rtk_set_cpu_dvfs_disable);
#else
void rtk_parse_cpu_dvfs(void)
{

	char strings[4];
	char *str = strings;

	if(rtk_parse_commandline_equal("no_cpu_dvfs", str, sizeof(strings)) == 0)
	{
		rtd_pr_cpu_freq_err("Error : can't get no_cpu_dvfs from bootargs\n");
		return;
	}

	if (!strncmp(str, "1", 1))
		rtk_cpu_dvfs_disable = 1;
	else if(!strncmp(str, "0", 1))
		rtk_cpu_dvfs_disable = 0;
	return;
}
#endif

#ifdef RTK_CPUFREQ_FS_NODE_ACCESS_SUPPORT
int init_cpu_freq(char *name, char *path);
#else
static void rtk_cpufreq_set_boost_directly(bool state);
#endif

bool rtk_boost_set_performance = 0;
ssize_t rtk_boost_performance_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", rtk_boost_set_performance);
}

ssize_t rtk_boost_performance_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int ret;
	unsigned int value;
	ret = sscanf(buf, "%u", &value);
	if (ret != 1) {
		return -EINVAL;
	}
	else
#ifdef CONFIG_RTK_SUPPORT_BOOST_MODE
		rtk_boost_set_performance = true;
#else
		rtk_boost_set_performance = false;
#endif
	return count;
}

#ifdef CONFIG_RTK_SUPPORT_BOOST_MODE_TIMEOUT

#define RTK_BOOST_DEFAULT_TIMEOUT_SEC (5)
#define RTK_BOOST_MAX_TIMEOUT_SEC (12 * 60)

struct workqueue_struct *rtk_boost_workqueue;
struct work_struct rtk_boost_work;
struct work_struct rtk_cpufreq_suspend_work;
struct timer_list rtk_boost_timer;
unsigned int rtk_bootst_timeout_val = 0; /*0 unset, use default, otherwise, set by AP.*/
unsigned int rtk_bootst_timeout_max = RTK_BOOST_MAX_TIMEOUT_SEC;
unsigned int rtk_bootst_timeout_default = RTK_BOOST_DEFAULT_TIMEOUT_SEC;
unsigned long rtk_bootst_timeout_end_time = 0;




void rtk_boost_exit_func(struct work_struct *work)
{
#ifdef RTK_CPUFREQ_FS_NODE_ACCESS_SUPPORT
	if(init_cpu_freq("0","/sys/devices/system/cpu/cpufreq/boost")!=0)
		rtd_pr_cpu_freq_err("disable boost mode fail(%s : %d)\n", __func__, __LINE__);
#else
	rtk_cpufreq_set_boost_directly(0);
#endif
}
#if IS_ENABLED(CONFIG_RTK_KDRV_THERMAL_SENSOR)
int ref_thermal_protect_disable = 0; //set by thermal driver
#endif
void cpu_thermal_callback(void *data, int cur_degree , int reg_degree, char *module_name);
void rtk_cpufreq_suspend_func(struct work_struct *work)
{
	//exist boost mode if need
	if(cpufreq_boost_enabled())
	{
#ifdef RTK_CPUFREQ_FS_NODE_ACCESS_SUPPORT
		if(init_cpu_freq("0","/sys/devices/system/cpu/cpufreq/boost")!=0)
			rtd_pr_cpu_freq_err("disable boost mode fail(%s : %d)\n", __func__, __LINE__);
#else
		rtk_cpufreq_set_boost_directly(0);
#endif
	}
#ifdef CONFIG_RTK_CPUFREQ_THERMAL_PROTECT
	else
	{
#if IS_ENABLED(CONFIG_RTK_KDRV_THERMAL_SENSOR)
		if(!ref_thermal_protect_disable)
			cpu_thermal_callback(0, 60, 0, 0);
#endif
	}
#endif
}
EXPORT_SYMBOL(ref_thermal_protect_disable);


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
	if(timer_pending(&rtk_boost_timer) == 0) // no timer
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
EXPORT_SYMBOL(rtk_boost_performance_show);
EXPORT_SYMBOL(rtk_boost_performance_store);
EXPORT_SYMBOL(rtk_boost_timeout_show);
EXPORT_SYMBOL(rtk_boost_timeout_store);
EXPORT_SYMBOL(rtk_boost_max_show);
EXPORT_SYMBOL(rtk_boost_default_show);

static int rtk_set_target(struct cpufreq_policy *policy,
			   unsigned int target_freq, unsigned int relation)
{
	struct cpufreq_freqs freqs;
	unsigned long freq_kHz;
	unsigned int index;
	int i;
	int ret;
	struct cpufreq_frequency_table *freq_table = policy->freq_table;
	struct rtk_cpufreq_info *info = rtk_cpufreq_info_lookup(policy->cpu);

	if (!info) {
		rtd_pr_cpu_freq_err("rtk_cpufreq_info_lookup for cpu%d is not initialized.\n", policy->cpu);
		return -EINVAL;
	}

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
	freqs.old = __rtk_clk_get_rate(policy->clk);

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
	ret = __rtk_clk_set_rate(policy->clk, freqs.new, policy->cpu, i);
	if (ret) {
		rtd_pr_cpu_freq_err("failed to set clock rate: %d\n", ret);
		return ret;
	}

	info->freq_tbl_idx = i;
	cpufreq_freq_transition_end(policy, &freqs,0);

#ifdef CONFIG_RTK_SUPPORT_BOOST_MODE_TIMEOUT
	if(cpufreq_boost_enabled() == 1)
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
extern unsigned int rtk_get_boot_freq(int);
//extern void rtk_set_default_voltage(void);
static int rtk_cpufreq_init(struct cpufreq_policy *policy)
{
	int ret;

	struct device *cpu_dev = get_cpu_device(policy->cpu);
	struct cpufreq_frequency_table *freq_table;
	struct rtk_cpufreq_info *info;

	if (!cpu_dev)
	{
		panic("failed to get cpu%d device\n", policy->cpu);
		return -ENODEV;
	}

	info = rtk_cpufreq_info_lookup(policy->cpu);
	if (!info)
	{
		panic("dvfs info for cpu%d is not allocate\n", policy->cpu);
		return -EINVAL;
	}

	policy->clk = __rtk_clk_get(policy->cpu);
	policy->transition_delay_us = 1000;

	cpumask_copy(policy->cpus, &info->cpus);
	info->policy = policy;
	info->status = RTK_CPUFREQ_STATE_P;

	ret = dev_pm_opp_init_cpufreq_table(cpu_dev, &freq_table);

	if (ret)
		rtd_pr_cpu_freq_err("failed to init cpufreq table for cpu%d: %d\n", policy->cpu, ret);

	policy->freq_table = freq_table;
	policy->cpuinfo.transition_latency = 10000;

	{
		int i;
		int max_opps = dev_pm_opp_get_opp_count(cpu_dev);
		info->control_pll = ((policy->cpu) == 0) ? (0) : (1);
		for (i = 0; i < max_opps; i++)
		{
			if(freq_table[i].frequency == CPUFREQ_TABLE_END)
				break;

#ifdef CONFIG_RTK_SUPPORT_BOOST_PERFORMANCE_MODE
			if(i >= (max_opps-2))
#else
			if(i >= (max_opps-1))
#endif

				freq_table[i].flags = CPUFREQ_BOOST_FREQ;

			freq_table[i].driver_data = RTK_CPUFREQ_STATE_P;
			rtd_pr_cpu_freq_info("table[%d].freq=%dkHz, boost=%d\n", i, freq_table[i].frequency, (freq_table[i].flags & CPUFREQ_BOOST_FREQ));
		}

		info->freqtbl_cnt = max_opps;
#ifdef CONFIG_RTK_SUPPORT_BOOST_PERFORMANCE_MODE
		info->freqtbl_idx_boost_perf = max_opps-1;
		info->freqtbl_idx_boost = max_opps-2;
		freq_table[max_opps-1].driver_data = RTK_CPUFREQ_STATE_X;
		freq_table[max_opps-2].driver_data = RTK_CPUFREQ_STATE_A;
#else
		info->freqtbl_idx_boost = max_opps-1;
		freq_table[max_opps-1].driver_data = RTK_CPUFREQ_STATE_A;
#endif
		freq_table[0].driver_data = RTK_CPUFREQ_STATE_R;

		info->Rpll = freq_table[0].frequency;
		if(info->Rpll < 1000000)
			info->Rpll = 1000000;

		info->Ppll = freq_table[max_opps-2].frequency;

#ifdef CONFIG_RTK_SUPPORT_BOOST_PERFORMANCE_MODE
		info->Apll = freq_table[max_opps-2].frequency;
		info->Xpll = freq_table[max_opps-1].frequency;
#else
		info->Apll = freq_table[max_opps-1].frequency;
#endif
		info->cur_pll = freq_table[max_opps-2].frequency;

#ifdef CONFIG_RTK_SUPPORT_BOOST_PERFORMANCE_MODE
		rtd_pr_cpu_freq_info("(%d)set PLL(%d/%d/%d/%d)\n", policy->cpu, info->Rpll, info->Ppll, info->Apll, info->Xpll);
#else
		rtd_pr_cpu_freq_info("(%d)set PLL(%d/%d/%d)\n", policy->cpu, info->Rpll, info->Ppll, info->Apll);
#endif
	}

	//for first time get booting freq
	rtk_get_boot_freq(rtk_cpufreq_get_pll_index(info));
	if(info->control_pll)
		rtk_get_boot_freq(rtk_cpufreq_get_pll_index(0));

#ifdef CONFIG_RTK_SUPPORT_BOOST_MODE
	if (policy_has_boost_freq(policy))
		cpufreq_enable_boost_support();
#endif


#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
	cpufreq_table_validate_and_show(policy, policy->freq_table);

	ret = cpufreq_frequency_table_target(policy, rtk_get_boot_freq(policy->cpu), CPUFREQ_RELATION_H);
	if(ret > 0)
	{
		policy->max = policy->freq_table[ret].frequency;
		rtd_pr_cpu_freq_info("cpufreq: set max %d\n", policy->max);
	}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	dev_pm_opp_of_register_em(get_cpu_device(policy->cpu), policy->cpus);
#else
	dev_pm_opp_of_register_em(policy->cpus);
#endif

#if 0	//move to rtk_cpufreq_set_range -> rtk_set_voltage
#ifdef CONFIG_REALTEK_VOLTAGE_CTRL
	if(policy->cpu != 0)	//for non-boot cpu when resume
		rtk_set_default_voltage();
#endif
#endif
	rtd_pr_cpu_freq_info("Rtk cpu frequency driver init successfully(%d/%d)\n", policy->cpu, info->control_pll );

	return 0;
}

static int rtk_cpufreq_exit(struct cpufreq_policy *policy)
{
	struct rtk_cpufreq_info *info = rtk_cpufreq_info_lookup(policy->cpu);
	struct device *cpu_dev = get_cpu_device(policy->cpu);

	if (!info) {
		panic("rtk_cpufreq_info_lookup for cpu%d is not initialized.\n", policy->cpu);
		return -EINVAL;
	}

	if (!cpu_dev)
	{
		panic("failed to get cpu%d device\n", policy->cpu);
		return -ENODEV;
	}
	dev_pm_opp_free_cpufreq_table(cpu_dev, &policy->freq_table);
	return 0;
}

#ifdef CONFIG_RTK_SUPPORT_BOOST_MODE
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
static int rtk_cpufreq_set_boost(struct cpufreq_policy *input_policy, int state)
#else
static int rtk_cpufreq_set_boost(int state)
#endif
{
	struct rtk_cpufreq_info *info;
	struct cpufreq_policy *policy;
	int ret;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	//Linux 5.10 or later : change to per-policy call the set_boost
	if(!rtk_check_control_pll_cpu(input_policy->cpu))
		return 0;
#endif

	list_for_each_entry(info, &rtk_cpufreq_info_list, list_head)
	{
		int idx;
		policy = info->policy;

		//Setp1: get index first
		if(cpufreq_boost_enabled())	//set to boost
		{
			if(info->status == RTK_CPUFREQ_STATE_R)
				return -EINVAL;
#ifdef CONFIG_RTK_SUPPORT_BOOST_PERFORMANCE_MODE
			if(rtk_boost_set_performance)
				idx = info->freqtbl_idx_boost_perf;
			else
#endif
				idx = info->freqtbl_idx_boost;
		}
		else	//exit boost
			idx = info->freqtbl_idx_working_max;

		if(idx >= info->freqtbl_cnt)
		{
			rtd_pr_cpu_freq_err("set boot idx fail(%d)\n", idx);
			return -EINVAL;
		}

		//step2: update max
		policy->max = policy->cpuinfo.max_freq = policy->freq_table[idx].frequency;
		ret = freq_qos_update_request(policy->max_freq_req, policy->max);
		if (ret < 0)
			rtd_pr_cpu_freq_err("Failed to update policy max (%s : %d)\n", __func__, __LINE__);

		//Step3: change scaling_governor if need
		if(cpufreq_boost_enabled())
		{
#ifdef RTK_CPUFREQ_FS_NODE_ACCESS_SUPPORT
			char path[64]={'\0'};
			sprintf(path, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_governor", policy->cpu);
			if(init_cpu_freq("performance",path)!=0)
				rtd_pr_cpu_freq_err("set CPUFreq performance(%d) fail (%s : %d)\n", policy->cpu, __func__, __LINE__);
#else
			//update min freq setting to meet performance governor
			policy->min = policy->cpuinfo.min_freq = policy->max;
			ret = freq_qos_update_request(policy->min_freq_req, policy->min);
			if (ret < 0)
				rtd_pr_cpu_freq_err("Failed to update policy min (%s : %d)\n", __func__, __LINE__);
#endif
#ifdef CONFIG_RTK_SUPPORT_BOOST_PERFORMANCE_MODE
			if(rtk_boost_set_performance)
			{
				info->status = RTK_CPUFREQ_STATE_X;
			}
			else
#else
				info->status = RTK_CPUFREQ_STATE_A;
#endif

		}
		else
		{
#ifdef RTK_CPUFREQ_FS_NODE_ACCESS_SUPPORT
			char path[64]={'\0'};
			sprintf(path, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_governor", policy->cpu);
			if(init_cpu_freq("schedutil",path)!=0)
				rtd_pr_cpu_freq_err("set CPUFreq schedutil(%d) fail (%s : %d)\n", policy->cpu, __func__, __LINE__);
#else
			int idx = info->freqtbl_idx_working_min;
			//update min freq setting to meet schedutil governor
			policy->min = policy->cpuinfo.min_freq = policy->freq_table[idx].frequency;
			ret = freq_qos_update_request(policy->min_freq_req, policy->min);
			if (ret < 0)
				rtd_pr_cpu_freq_err("Failed to update policy min (%s : %d)\n", __func__, __LINE__);
#endif
			info->status = RTK_CPUFREQ_STATE_P;
		}
	}

	//reset to default off
	if(cpufreq_boost_enabled() && rtk_boost_set_performance)
		rtk_boost_set_performance = 0;

	return 0;
}
#endif

static struct freq_attr *rtk_cpufreq_attr[] = {
	&cpufreq_freq_attr_scaling_available_freqs,
	NULL,
};

static void rtk_cpufreq_set_range(struct cpufreq_policy *policy)
{
	struct rtk_cpufreq_info *info = rtk_cpufreq_info_lookup(policy->cpu);
	int ret;
	if (!info) {
		panic("rtk_cpufreq_info_lookup for cpu%d is not initialized.\n", policy->cpu);
		return;
	}

	//set max and min
	ret = info->freqtbl_idx_boost -1;
	if(ret > 0)
	{
		policy->cpuinfo.max_freq = policy->max = policy->freq_table[ret].frequency;
		rtd_pr_cpu_freq_info("new cpufreq: set max %d\n", policy->max);
	}

	info->freqtbl_idx_working_max = ret;
	info->freqtbl_idx_working_min = 1;

	policy->cpuinfo.min_freq = policy->min = policy->freq_table[1].frequency;
	rtd_pr_cpu_freq_info("new cpufreq: set min %d\n", policy->min);

#if 0 //always use the policy's freq to avoid batterystats-wo access show_time_in_state (unable to handle kernel paging request)
	if(rtk_cpu_dvfs_disable == 1)
		policy->clk->rate = rtk_get_boot_freq(policy->cpu);
	else
#endif
		policy->clk->rate = policy->min; // use policy min to set as init value

	policy->cur = __rtk_clk_get_rate(policy->clk); //set cur as policy min
	info->freq_tbl_idx = 1;

	if(rtk_cpu_dvfs_disable != 1)
	{
		//in case bootcode use different pll, first time need to setup pll to same base
		rtd_pr_cpu_freq_info("rtk_cpufreq_notify set pll(%d)\n", info->Ppll);
		rtk_clk_set_pll(info, info->Ppll, rtk_cpufreq_get_pll_index(info));

		if(rtk_check_control_pll_cpu(policy->cpu))
		{
			rtd_pr_cpu_freq_info("rtk_cpufreq_notify set dsu\n");
			rtk_clk_set_dsu_pll(info, RTK_CPUFREQ_STATE_P);
		}

		//set current to avoid only one freq case cause skip setting
		rtd_pr_cpu_freq_info("rtk_cpufreq_set_range set clkratio(%d)/(%d)/(%d)\n", policy->cpu, info->Ppll, policy->cur);
		rtk_clk_set_ratio(policy->cpu, info->Ppll, policy->cur);
#ifdef CONFIG_REALTEK_VOLTAGE_CTRL
		rtk_set_voltage(info, info->Ppll, info->Ppll);
#endif
	}
}

#ifdef CONFIG_REALTEK_VOLTAGE_CTRL
extern int rtk_cpu_dvs_enable;
extern int rtk_cpu_voltage_val;
extern int old_voltage_level;
extern void rtk_set_voltage_directly(int level);
extern void rtk_parse_cpu_dvs_control(void);
extern void rtk_parse_cpu_voltage_val(void);
#endif
static int rtk_cpufreq_resume(struct cpufreq_policy *policy)
{
	/*
	Enabling non-boot CPUs :
	cpuhp_thread_fun -> cpuhp_invoke_callback -> cpuhp_cpufreq_online -> cpufreq_online -> rtk_cpufreq_init
	without CPUFREQ_CREATE_POLICY notifier,
	Then dpm_resume -> cpufreq_resume -> cpufreq_driver->resume
	*/
#ifdef CONFIG_REALTEK_VOLTAGE_CTRL
	if(rtk_check_control_pll_cpu(policy->cpu))
		old_voltage_level = 0; //reset to SCPU_NONE
#endif
	rtk_cpufreq_set_range(policy);

#ifdef CONFIG_REALTEK_VOLTAGE_CTRL
	if(rtk_cpu_dvs_enable == 0 && rtk_cpu_voltage_val != 0)
		rtk_set_voltage_directly(rtk_cpu_voltage_val);
#endif
	return 0;
}

static int rtk_cpufreq_pm_notifier_event(struct notifier_block *this,
                           unsigned long event, void *dummy)
{
	switch (event)
	{
		case PM_SUSPEND_PREPARE:
			rtd_pr_cpu_freq_info("rtk_cpufreq_pm_notifier_event(%ld)\n", event);
			queue_work(rtk_boost_workqueue, &rtk_cpufreq_suspend_work);
			break;
		default:
			break;
	}
	return NOTIFY_DONE;
}

static struct notifier_block rtk_cpufreq_pm_notifier = {
        .notifier_call = rtk_cpufreq_pm_notifier_event,
};

static struct cpufreq_driver rtk_cpufreq_driver = {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
	.flags = CPUFREQ_STICKY,
#endif
	.verify = rtk_verify_speed,
	.target = rtk_set_target,
	.get = rtk_get_speed,
	.init = rtk_cpufreq_init,
	.exit = rtk_cpufreq_exit,
	.name = "realtek_cpufreq",
	.attr = rtk_cpufreq_attr,
#ifdef CONFIG_RTK_SUPPORT_BOOST_MODE
	.set_boost = rtk_cpufreq_set_boost,
#endif
	.resume = rtk_cpufreq_resume,
};

#ifndef RTK_CPUFREQ_FS_NODE_ACCESS_SUPPORT
static void rtk_cpufreq_set_boost_directly(bool state)
{
	struct rtk_cpufreq_info *info;
	struct cpufreq_policy *policy;
	int ret;

	if (rtk_cpufreq_driver.boost_enabled == state)
		return;

	cpus_read_lock();
	//set value first (because cpufreq_boost_trigger_state set value then call cpufreq_driver->set_boost)
	rtk_cpufreq_driver.boost_enabled = state;


	list_for_each_entry(info, &rtk_cpufreq_info_list, list_head)
	{
		int idx;
		policy = info->policy;

		//Setp1: get index first
		if(cpufreq_boost_enabled()) //set to boost
		{
			if(info->status == RTK_CPUFREQ_STATE_R)
			{
				rtk_cpufreq_driver.boost_enabled = 0;
				cpus_read_unlock();
				rtd_pr_cpu_freq_err("Error: set cpufreq boost on cpufreq R mode\n");
				return;
			}
#ifdef CONFIG_RTK_SUPPORT_BOOST_PERFORMANCE_MODE
			if(rtk_boost_set_performance)
				idx = info->freqtbl_idx_boost_perf;
			else
#endif
				idx = info->freqtbl_idx_boost;

		}
			else	//exit boost
				idx = info->freqtbl_idx_working_max;

		if(idx >= info->freqtbl_cnt)
		{
			rtk_cpufreq_driver.boost_enabled = !state;
			cpus_read_unlock();
			rtd_pr_cpu_freq_err("set boot idx fail(%d)\n", idx);
			return;
		}

		//step2: update max
		policy->max = policy->cpuinfo.max_freq = policy->freq_table[idx].frequency;
		ret = freq_qos_update_request(policy->max_freq_req, policy->max);
		if (ret < 0)
			rtd_pr_cpu_freq_err("Failed to update policy max (%s : %d)\n", __func__, __LINE__);

		//Step3: change scaling_governor if need
		if(cpufreq_boost_enabled())
		{
			//update min freq setting to meet performance governor
			policy->min = policy->cpuinfo.min_freq = policy->max;
			ret = freq_qos_update_request(policy->min_freq_req, policy->min);
			if (ret < 0)
				rtd_pr_cpu_freq_err("Failed to update policy min (%s : %d)\n", __func__, __LINE__);

#ifdef CONFIG_RTK_SUPPORT_BOOST_PERFORMANCE_MODE
			if(rtk_boost_set_performance)
			{
				info->status = RTK_CPUFREQ_STATE_X;
			}
			else
#else
				info->status = RTK_CPUFREQ_STATE_A;
#endif

		}
		else
		{
			int idx = info->freqtbl_idx_working_min;
			//update min freq setting to meet schedutil governor
			policy->min = policy->cpuinfo.min_freq = policy->freq_table[idx].frequency;
			ret = freq_qos_update_request(policy->min_freq_req, policy->min);
			if (ret < 0)
				rtd_pr_cpu_freq_err("Failed to update policy min (%s : %d)\n", __func__, __LINE__);
			info->status = RTK_CPUFREQ_STATE_P;
		}
	}
	cpus_read_unlock();

	//reset to default off
	if(cpufreq_boost_enabled() && rtk_boost_set_performance)
		rtk_boost_set_performance = 0;

	//set value again, to prevent the cpufreq_boost_trigger_state racing.
	rtk_cpufreq_driver.boost_enabled = state;
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
static int rtk_cpufreq_notify(struct notifier_block *self, unsigned long action, void *data)
{
	if(action == CPUFREQ_CREATE_POLICY)
	{
		/*
		cpufreq_online -> create policy ->  cpufreq_driver->init
		-> cpufreq_table_validate_and_sort (set policy min)
		-> blocking_notifier_call_chain(CPUFREQ_CREATE_POLICY)
		-> cpufreq_driver->get (policy->cur should not be zero)
		-> cpufreq_init_policy (cpufreq_set_policy -> verify -> cpufreq_frequency_table_verify) (use cpuinfo.max_freq to limit policy->max)
		*/
		rtk_cpufreq_set_range(data);
	}
	return 0;
}

static struct notifier_block rtk_cpufreq_notifier = {
	.notifier_call = rtk_cpufreq_notify,
};
#endif

static int rtk_cpufreq_driver_init(void)
{
	int ret, cpu;
	struct rtk_cpufreq_info *info;
	struct device *cpu_dev;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	cpufreq_register_notifier(&rtk_cpufreq_notifier, CPUFREQ_POLICY_NOTIFIER);
#endif

	for_each_possible_cpu(cpu)
	{
		info = rtk_cpufreq_info_lookup(cpu);
		if (info)
			continue;

		info = kzalloc(sizeof(*info), GFP_KERNEL);
		if (!info)
		{
			panic("fail to allocate rtk_cpufreq_info\n");
			return -ENOMEM;
		}

		cpu_dev = get_cpu_device(cpu);
		if (!cpu_dev)
			panic("failed to get cpu%d device\n", cpu);

		ret = dev_pm_opp_of_get_sharing_cpus(cpu_dev, &info->cpus);
		if (ret)
			panic("failed to get OPP-sharing information for cpu %d: %d\n", cpu, ret);

		ret = dev_pm_opp_of_cpumask_add_table(&info->cpus);
		if (ret)
			panic("no OPP table for cpu%d\n", cpu);

		list_add(&info->list_head, &rtk_cpufreq_info_list);
		rtd_pr_cpu_freq_info("rtk_cpufreq_driver_init add cpufreq info(%d)\n", cpu);
	}

	ret = cpufreq_register_driver(&rtk_cpufreq_driver);
	if (ret) {
		panic("failed register driver: %d\n", ret);
		goto register_fail;
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
	register_pm_notifier(&rtk_cpufreq_pm_notifier);
	INIT_WORK(&rtk_cpufreq_suspend_work, rtk_cpufreq_suspend_func);

#ifdef CONFIG_RTK_KDRV_CPU_FREQUENCY_MODULE
	rtk_parse_cpu_dvfs();
#ifdef CONFIG_REALTEK_VOLTAGE_CTRL
	rtk_parse_cpu_dvs_control();
	rtk_parse_cpu_voltage_val();
#endif
#endif

#ifdef CONFIG_REALTEK_VOLTAGE_CTRL
	if(rtk_cpu_dvs_enable == 0 && rtk_cpu_voltage_val != 0)
		rtk_set_voltage_directly(rtk_cpu_voltage_val);
#endif

	return 0;
register_fail:
	return ret;
}

fs_initcall(rtk_cpufreq_driver_init);


/*[ML7BU-1]*/
#if 1
#define CPUFREQ_LEVEL_0_UP_DEGREE 120
#define CPUFREQ_LEVEL_1_DOWN_DEGREE 115

#ifdef RTK_CPUFREQ_FS_NODE_ACCESS_SUPPORT
int init_cpu_freq(char *name, char *path)  //setup sysfs node value
{
	struct file *filp = NULL;
	loff_t pos = 0;
	int ret;
	char buf[20]="";

	filp = filp_open(path, O_RDWR | O_CREAT, 0);

	if (IS_ERR(filp)) {
		rtd_pr_cpu_freq_info("open failed %s %s...(%ld)\n", path,__func__, PTR_ERR(filp));
		return -1;
	}

	sprintf(buf, "%s", name);

	ret = kernel_write(filp, buf, strlen(buf), &pos);
	if (ret < 0)
		rtd_pr_cpu_freq_err("Error writing path: %s fail\n", path);

	if(filp != NULL)
		filp_close(filp, NULL);
	return 0;
}
#endif

#ifdef CONFIG_RTK_CPUFREQ_THERMAL_PROTECT
void cpu_thermal_callback(void *data, int cur_degree , int reg_degree, char *module_name)
{
	static int current_level  = 0;
	int previous_level = current_level;

	if(current_level == 0)
	{
		if(cur_degree >= CPUFREQ_LEVEL_0_UP_DEGREE)
		{
			current_level = 1;
		}
	}
	else if(current_level == 1)
	{
		if(cur_degree < CPUFREQ_LEVEL_1_DOWN_DEGREE)
		{
			current_level = 0;
		}
	}

	if(current_level != previous_level)
	{
		struct rtk_cpufreq_info *info;
		struct cpufreq_policy *policy;
		int ret, idx;
#ifdef RTK_CPUFREQ_FS_NODE_ACCESS_SUPPORT
		char path[64]={'\0'};
#endif
		if(current_level == 1)
		{
			if(cpufreq_boost_enabled())
			{
#ifdef RTK_CPUFREQ_FS_NODE_ACCESS_SUPPORT
				if(init_cpu_freq("0","/sys/devices/system/cpu/cpufreq/boost")!=0)
					rtd_pr_cpu_freq_err("disable boost mode fail(%s : %d)\n", __func__, __LINE__);
#else
				rtk_cpufreq_set_boost_directly(0);
#endif
				//maks sure set back to RTK_CPUFREQ_STATE_P first
				current_level = 0;
				return;
			}

			rtkaudio_dap_onoff(0); /*MK2BU-279*/
			list_for_each_entry(info, &rtk_cpufreq_info_list, list_head)
			{
				idx = 0;
				policy = info->policy;

				//then update min
				policy->min = policy->cpuinfo.min_freq = policy->freq_table[idx].frequency;
				ret = freq_qos_update_request(policy->min_freq_req, policy->min);
				if (ret < 0)
					rtd_pr_cpu_freq_err("Failed to update policy min (%s : %d)\n", __func__, __LINE__);

#ifdef RTK_CPUFREQ_FS_NODE_ACCESS_SUPPORT
				//set scaling_governor first
				sprintf(path, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_governor", policy->cpu);
				if(init_cpu_freq("powersave",path)!=0)
				{
					rtd_pr_cpu_freq_err("set CPUFreq powersave(%d) fail (%s : %d)\n", policy->cpu, __func__, __LINE__);

					/* For juest after resume back, android.system.suspend call thermal_pm_notify to update, but Permission denied.
						So just return for next time trigger by interrupt event.
					*/
					// update min back
					idx = info->freqtbl_idx_working_min;
					policy = info->policy;
					policy->min = policy->cpuinfo.min_freq = policy->freq_table[idx].frequency;
					ret = freq_qos_update_request(policy->min_freq_req, policy->min);
					if (ret < 0)
						rtd_pr_cpu_freq_err("Failed to update policy min (%s : %d)\n", __func__, __LINE__);

					current_level = 0;
					return;
				}
#else
				policy->max = policy->cpuinfo.max_freq = policy->min;
				ret = freq_qos_update_request(policy->max_freq_req, policy->max);
				if (ret < 0)
					rtd_pr_cpu_freq_err("Failed to update policy max (%s : %d)\n", __func__, __LINE__);
#endif
				info->status = RTK_CPUFREQ_STATE_R;

			}

		}
		else if(current_level == 0)
		{
			list_for_each_entry(info, &rtk_cpufreq_info_list, list_head)
			{

				// update min first
				idx = info->freqtbl_idx_working_min;
				policy = info->policy;
				policy->min = policy->cpuinfo.min_freq = policy->freq_table[idx].frequency;
				ret = freq_qos_update_request(policy->min_freq_req, policy->min);
				if (ret < 0)
					rtd_pr_cpu_freq_err("Failed to update policy min (%s : %d)\n", __func__, __LINE__);

#ifdef RTK_CPUFREQ_FS_NODE_ACCESS_SUPPORT
				//then change scaling_governor
				sprintf(path, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_governor", policy->cpu);
				if(init_cpu_freq("schedutil",path)!=0)
					rtd_pr_cpu_freq_err("set CPUFreq schedutil(%d) fail (%s : %d)\n", policy->cpu, __func__, __LINE__);
#else
				idx = info->freqtbl_idx_working_max;
				policy->max = policy->cpuinfo.max_freq = policy->freq_table[idx].frequency;
				ret = freq_qos_update_request(policy->max_freq_req, policy->max);
				if (ret < 0)
					rtd_pr_cpu_freq_err("Failed to update policy max (%s : %d)\n", __func__, __LINE__);
#endif
				info->status = RTK_CPUFREQ_STATE_P;
			}
			rtkaudio_dap_onoff(1); /*MK2BU-279*/
		}
	}
}
#endif


#if IS_ENABLED(CONFIG_RTK_KDRV_THERMAL_SENSOR)
int register_cpu_thermal_handler(register_temp_fun_ptr opt)
{
#ifdef CONFIG_RTK_CPUFREQ_THERMAL_PROTECT
        int ret;
	static int temp;

        rtd_pr_cpu_freq_info("%s\n",__func__);
	if ((ret = opt(CPUFREQ_LEVEL_1_DOWN_DEGREE -10, cpu_thermal_callback, &temp, "cpufreq")) < 0)
                rtd_pr_cpu_freq_info("register cpufreq thermal handler fail, ret:%d \n", ret);

#endif
        return 0;
}
EXPORT_SYMBOL(register_cpu_thermal_handler);
//late_initcall(register_cpu_thermal_handler); //move to rtk_tmu_probe, and use fptr to pass
#endif

#endif

MODULE_AUTHOR("davidwang <davidwang@realtek.com>");
MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);
MODULE_DESCRIPTION("realtek cpufreq driver");
MODULE_LICENSE("GPL");
