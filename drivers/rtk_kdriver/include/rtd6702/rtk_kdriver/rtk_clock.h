#ifndef __ARCH_ARM_RTK_CLOCK_H
#define __ARCH_ARM_RTK_CLOCK_H

#include <linux/cpufreq.h>
#include <linux/kref.h>
#include <linux/list.h>
#include <linux/platform_device.h>

#define CONFIG_RTK_SUPPORT_CPUFREQ_OTP
#define CONFIG_RTK_SUPPORT_BOOST_MODE
#define CONFIG_RTK_SUPPORT_BOOST_MODE_TIMEOUT //depends on CONFIG_RTK_SUPPORT_BOOST_MODE
#define CONFIG_RTK_CPUFREQ_THERMAL_PROTECT
#define CONFIG_RTK_CPUFREQ_FORCE_TO_USE_FREQ_TABLE
//#define CONFIG_RTK_SUPPORT_BOOST_PERFORMANCE_MODE //Mark2 don't have boost performance mode

#if 0// #ifdef CONFIG_RTK_FEATURE_FOR_GKI
/*
ML7BU-1651: can't use fs_access API under GKI
So, can't directly change cpufreq governor, or etner/exit boost by fs node.
*/
#define RTK_CPUFREQ_FS_NODE_ACCESS_SUPPORT 1
#endif

#if (defined CONFIG_CUSTOMER_TV006)
#define SCPU_FREQ_STEP0  700000
#define SCPU_FREQ_STEP1  900000
#define SCPU_FREQ_STEP2  900000
#define SCPU_FREQ_STEP3  1050000
#define SCPU_FREQ_STEP4  1100000
#define SCPU_VID1        42
#define SCPU_VID0        40

#else
#define SCPU_FREQ_STEP0  900000
#define SCPU_FREQ_STEP1  1000000
#define SCPU_FREQ_STEP2  1100000
#define SCPU_FREQ_STEP3  1300000
#define SCPU_FREQ_STEP4  1400000
#define SCPU_VID1        33
#define SCPU_VID0        34
#endif

#ifdef CONFIG_RTK_SUPPORT_BOOST_PERFORMANCE_MODE
#define RTK_CPUFREQ_STATE_X	3
#endif
#define RTK_CPUFREQ_STATE_A	2
#define RTK_CPUFREQ_STATE_P	1
#define RTK_CPUFREQ_STATE_R	0

struct rtk_cpufreq_info {
	struct list_head list_head;
	struct cpumask cpus;
	struct cpufreq_policy *policy;
	int freqtbl_cnt;
	int freqtbl_idx_working_min;
	int freqtbl_idx_working_max;
	int freqtbl_idx_boost;
#ifdef CONFIG_RTK_SUPPORT_BOOST_PERFORMANCE_MODE
	int freqtbl_idx_boost_perf;
#endif
	unsigned int status;
	int freq_tbl_idx;
//	unsigned int bootfreq;
	unsigned int Rpll;
	unsigned int Ppll;
#ifdef CONFIG_RTK_SUPPORT_BOOST_PERFORMANCE_MODE
	unsigned int Xpll;
#endif
	unsigned int Apll;
	unsigned int cur_pll;
	bool control_pll;
};

struct rtk_cpufreq_info* rtk_cpufreq_info_lookup(int cpu);
bool rtk_check_control_pll_cpu(int cpu);
int rtk_cpufreq_get_dsu_pll(int cpu);
void rtk_clk_set_ratio(int cpu, unsigned int pll, unsigned int rate);
void rtk_clk_set_pll(struct rtk_cpufreq_info *info, unsigned int rate, unsigned int rate2);
int rtk_cpufreq_get_pll_index(struct rtk_cpufreq_info *info);
void rtk_clk_set_dsu_pll(struct rtk_cpufreq_info *info, unsigned int state);

extern void (*cpu_wait) (void);

struct clk;

struct clk_ops {
	void (*init) (struct clk *clk);
	void (*enable) (struct clk *clk);
	void (*disable) (struct clk *clk);
	void (*recalc) (struct clk *clk);
	int (*set_rate) (struct clk *clk, unsigned long rate, int algo_id);
	long (*round_rate) (struct clk *clk, unsigned long rate);
};

struct clk {
	struct list_head node;
	const char *name;
	int id;
	struct module *owner;

	struct clk *parent;
	struct clk_ops *ops;

	struct kref kref;

	unsigned long rate;
	unsigned long flags;
};

#define CLK_ALWAYS_ENABLED	(1 << 0)
#define CLK_RATE_PROPAGATES	(1 << 1)

/* Should be defined by processor-specific code */
void arch_init_clk_ops(struct clk_ops **, int type);

int clk_init(void);

int __clk_enable(struct clk *);
void __clk_disable(struct clk *);

void clk_recalc_rate(struct clk *);

int clk_register(struct clk *);
void clk_unregister(struct clk *);

struct clk *__rtk_clk_get(int cpu);
unsigned long __rtk_clk_get_rate(struct clk *clk);
int __rtk_clk_set_rate(struct clk *clk, unsigned int rate, int cpu, int idx);
int __rtk_clk_set_rate_ex(struct clk *clk, unsigned int rate, int algo_id, int cpu, int idx);
//long __rtk_clk_round_rate(struct clk *clk, unsigned long rate);

void rtk_set_voltage(struct rtk_cpufreq_info *info, unsigned int pll, unsigned int rate);

unsigned int rtk_get_speed(unsigned int cpu);
#endif				/* __ASM_MIPS_CLOCK_H */
