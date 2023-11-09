#ifndef __ARCH_ARM_RTK_CLOCK_H
#define __ARCH_ARM_RTK_CLOCK_H

#include <linux/cpufreq.h>
#include <linux/kref.h>
#include <linux/list.h>
#include <linux/platform_device.h>

#define CONFIG_RTK_SUPPORT_CPUFREQ_OTP
#define CONFIG_RTK_SUPPORT_BOOST_MODE
#define CONFIG_RTK_SUPPORT_BOOST_MODE_TIMEOUT //depends on CONFIG_RTK_SUPPORT_BOOST_MODE

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

struct clk *__rtk_clk_get(struct device *dev, const char *id);
unsigned long __rtk_clk_get_rate(struct clk *clk);
int __rtk_clk_set_rate(struct clk *clk, unsigned long rate);
int __rtk_clk_set_rate_ex(struct clk *clk, unsigned long rate, int algo_id);
//long __rtk_clk_round_rate(struct clk *clk, unsigned long rate);

int __rtk_init_cpufreq_table(struct cpufreq_frequency_table **table);
void __rtk_free_cpufreq_table(struct cpufreq_frequency_table **table);
void rtk_set_voltage(int freq);
int rtk_get_current_voltage(void);

#ifdef CPUFREQ_DRIVER_GET_FACTORY
#define CPU_LINE_MAX  10
#define CPU_CONTENT_MAX  8
typedef enum
{
        CPU_NAME_KEY_E = 0,
        CPU_ENABLE_KEY_E,
        CPU_0_KEY_E,
        CPU_1_KEY_E,
        CPU_2_KEY_E,
        CPU_3_KEY_E,
        CPU_4_KEY_E,
        CPU_MAX_KEY_E,
}CPU_KEY_E;

typedef struct
{
        char module_name_content[CPU_CONTENT_MAX];
        int content[CPU_MAX_KEY_E-1];
}CPU_CONTENT_T;

typedef struct
{
        unsigned int magic_num;
        int check_sum;
        bool valid;
        char reserve1;
        char reserve2;
        char reserve3;
        CPU_CONTENT_T cpu_content[CPU_LINE_MAX];
}CPU_RESOURC_T;
#endif


#endif				/* __ASM_MIPS_CLOCK_H */
