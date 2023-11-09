/*
 * rtk_thermal.c - Realtek TMU (Thermal Management Unit)
 *
 *  Copyright (C) 2011 Realtek
 *  Clamp Chien <clamp.chien@realtek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#define THERMAL_DRIVER_GET_FACTORY
#include <linux/pm.h>
#include <linux/reboot.h>
#include <linux/module.h>
#include <linux/err.h>
#include <rtd_log/rtd_module_log.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/workqueue.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/export.h>
#include <linux/io.h>
#include <linux/mutex.h>
#ifdef CONFIG_RTK_KDRV_THERMAL_SENSOR_MODULE
#include <rtk_kdriver/rtk-kdrv-common.h>
#endif
#include <rtk_kdriver/rtk_thermal_sensor.h>
#include <linux/thermal.h>
#include <linux/cpufreq.h>
#include <linux/cpu_cooling.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/version.h>
#include <rbus/pll_reg_reg.h>
#include <rbus/sys_reg_reg.h>
#include <rtk_kdriver/io.h>

#if IS_ENABLED(CONFIG_SUPPORT_SCALER)
extern void Scaler_register_PQ_thermal(int t0, int t1, int t2);
#if defined(CONFIG_MEMC) && defined(CONFIG_HW_SUPPORT_MEMC)
extern void Scaler_register_MEMC_thermal(int t0, int t1, int t2);
#endif
#if IS_ENABLED(CONFIG_RTK_AI_DRV)
extern void ScalerAI_register_AI_thermal(int t0, int t1, int t2);
#endif
#endif

#ifdef CONFIG_RTK_KDRV_DYNAMIC_REFRESH_SETTING
#ifdef CONFIG_CPU_FREQ
#ifdef CONFIG_ARCH_RTK2885P
static int set_dram_refresh_rate_2885p(int temp);
#else
static int set_dram_refresh_rate(int temp);
#endif
#endif
#endif

#if 0 //disable qsort related code
/* Byte-wise swap two items of size SIZE. */
#define SWAP(a, b, size)              \
  do {                        \
      size_t __size = (size);             \
      char *__a = (a), *__b = (b);        \
      do {                    \
      char __tmp = *__a;              \
      *__a++ = *__b;              \
      *__b++ = __tmp;             \
    } while (--__size > 0);           \
    } while (0)

/* Discontinue quicksort algorithm when partition gets below this
   size.  This particular magic number was chosen to work best on a
   Sun 4/260. */
#define MAX_THRESH 4

/* Stack node declarations used to store unfulfilled partition
 * obligations. */
typedef struct {
    char *lo;
    char *hi;
} stack_node;

/* The next 5 #defines implement a very fast in-line stack
 * abstraction.  The stack needs log (total_elements) entries (we
 * could even subtract log(MAX_THRESH)).  Since total_elements has
 * type size_t, we get as upper bound for log (total_elements): bits
 * per byte (CHAR_BIT) * sizeof(size_t).  */

#define CHAR_BIT 8
#define STACK_SIZE  (CHAR_BIT * sizeof(size_t))
#define PUSH(low, high) ((top->lo = (low)), (top->hi = (high)), ++top)
#define POP(low, high)  (--top, (low = top->lo), (high = top->hi))
#define STACK_NOT_EMPTY (stack < top)


void
qsort(void *const pbase, size_t total_elems, size_t size,
      int (*cmp)(const void*, const void*))
{
    char *base_ptr = (char *)pbase;

    const size_t max_thresh = MAX_THRESH * size;

    /* Avoid lossage with unsigned arithmetic below.  */
    if (total_elems == 0) {
        return;
    }

    if (total_elems > MAX_THRESH) {
        char *lo = base_ptr;
        char *hi = &lo[size * (total_elems - 1)];
        stack_node stack[STACK_SIZE];
        stack_node *top = stack + 1;

        while (STACK_NOT_EMPTY) {
            char *left_ptr;
            char *right_ptr;

            /* Select median value from among LO, MID, and
               HI. Rearrange LO and HI so the three values
               are sorted. This lowers the probability of
               picking a pathological pivot value and
               skips a comparison for both the LEFT_PTR
               and RIGHT_PTR in the while loops. */

            char *mid = lo + size * ((hi - lo) / size >> 1);

            if ((*cmp)((void*)mid, (void*)lo) < 0)
                SWAP(mid, lo, size);
            if ((*cmp)((void*)hi, (void*)mid) < 0)
                SWAP(mid, hi, size);
            else
                goto jump_over;
            if ((*cmp)((void*)mid, (void*)lo) < 0)
                SWAP(mid, lo, size);
        jump_over:

            left_ptr = lo + size;
            right_ptr = hi - size;

            /* Here's the famous ``collapse the walls''
               section of quicksort.  Gotta like those
               tight inner loops!  They are the main
               reason that this algorithm runs much faster
               than others. */
            do {
                while ((*cmp)((void*)left_ptr, (void*)mid) < 0)
                    left_ptr += size;

                while ((*cmp)((void*)mid, (void*)right_ptr) < 0)
                    right_ptr -= size;

                if (left_ptr < right_ptr) {
                    SWAP(left_ptr, right_ptr, size);
                    if (mid == left_ptr)
                        mid = right_ptr;
                    else if (mid == right_ptr)
                        mid = left_ptr;
                    left_ptr += size;
                    right_ptr -= size;
                } else if (left_ptr == right_ptr) {
                    left_ptr += size;
                    right_ptr -= size;
                    break;
                }
            } while (left_ptr <= right_ptr);

            /* Set up pointers for next iteration.  First
               determine whether left and right partitions
               are below the threshold size.  If so,
               ignore one or both.  Otherwise, push the
               larger partition's bounds on the stack and
               continue sorting the smaller one. */

            if ((size_t) (right_ptr - lo) <= max_thresh) {
                if ((size_t) (hi - left_ptr) <= max_thresh)
                    /* Ignore both small partitions. */
                    POP(lo, hi);
                else
                    /* Ignore small left partition. */
                    lo = left_ptr;
            } else if ((size_t) (hi - left_ptr) <= max_thresh)
                /* Ignore small right partition. */
                hi = right_ptr;
            else if ((right_ptr - lo) > (hi - left_ptr)) {
                /* Push larger left partition indices. */
                PUSH(lo, right_ptr);
                lo = left_ptr;
            } else {
                /* Push larger right partition indices. */
                PUSH(left_ptr, hi);
                hi = right_ptr;
            }
        }
    }

    /* Once the BASE_PTR array is partially sorted by quicksort
       the rest is completely sorted using insertion sort, since
       this is efficient for partitions below MAX_THRESH
       size. BASE_PTR points to the beginning of the array to
       sort, and END_PTR points at the very last element in the
       array (*not* one beyond it!). */

    {
        char *end_ptr = &base_ptr[size * (total_elems - 1)];
        char *tmp_ptr = base_ptr;
        char *thresh = min(end_ptr, base_ptr + max_thresh);
        char *run_ptr;

        /* Find smallest element in first threshold and place
           it at the array's beginning.  This is the smallest
           array element, and the operation speeds up
           insertion sort's inner loop. */

        for (run_ptr = tmp_ptr + size; run_ptr <= thresh; run_ptr += size)
            if ((*cmp)((void*)run_ptr, (void*)tmp_ptr) < 0)
                tmp_ptr = run_ptr;

        if (tmp_ptr != base_ptr)
            SWAP(tmp_ptr, base_ptr, size);

        /* Insertion sort, running from left-hand-side up to
         * right-hand-side.  */

        run_ptr = base_ptr + size;
        while ((run_ptr += size) <= end_ptr) {
            tmp_ptr = run_ptr - size;
            while ((*cmp)((void*)run_ptr, (void*)tmp_ptr) < 0)
                tmp_ptr -= size;

            tmp_ptr += size;
            if (tmp_ptr != run_ptr) {
                char *trav;

                trav = run_ptr + size;
                while (--trav >= run_ptr) {
                    char c = *trav;
                    char *hi, *lo;

                    for (hi = lo = trav; (lo -= size) >= tmp_ptr; hi = lo)
                        * hi = *lo;
                    *hi = c;
                }
            }
        }
    }
}
#endif


static int rtk_thermal_value_to(int value)
{
#ifdef RTK_THERMAL_DRIVER_TEMP_MAPPING
	if(value > 40)
		return (value << 3)/10 + 8;
	else
#endif
		return value;
}

static int __maybe_unused rtk_thermal_value_back(int value)
{
#ifdef RTK_THERMAL_DRIVER_TEMP_MAPPING
	if(value > 40)
	return ((value -8)*10) >> 3;
	else
#endif
		return value;
}


typedef struct rtk_temperature_callback {
    int degree;
    void (*func)(void *data, int cur_degree, int reg_degree, char *module_name);
    void *data;
    char module_name[20];
} st_rtk_temperature_callback;

#define RTK_TEMPE_CALLBACK_COUNT 8
static st_rtk_temperature_callback temperature_func[RTK_TEMPE_CALLBACK_COUNT];
static DEFINE_MUTEX(temperature_func_mutex);

#if 0 //disable qsort related code
int compare(const void *a, const void *b)//這函式是 qsort 所需的比較函式
{
    st_rtk_temperature_callback *c = (st_rtk_temperature_callback *)a;
    st_rtk_temperature_callback *d = (st_rtk_temperature_callback *)b;
    if(c->degree < d->degree) {
        return -1;   //傳回 -1 代表 a < b
    } else if (c->degree == d->degree) {
        return 0;   //傳回   0 代表 a = b
    } else return 1;                        //傳回  1 代表 a>b
}
#endif

#if 0 //no use
int unregister_temperature_callback(int degree)
{
    int i;
    if(degree > 250 || degree < -250)
        return -1;
    for(i = 0; i < RTK_TEMPE_CALLBACK_COUNT; i++) {
        /* search same degree handler and override it */
        if(degree == temperature_func[i].degree) {
            temperature_func[i].func = NULL;
            temperature_func[i].data = NULL;
            temperature_func[i].degree = -255;
            memset(temperature_func[i].module_name, 0, sizeof(temperature_func[i].module_name));
//            qsort((void *)temperature_func, RTK_TEMPE_CALLBACK_COUNT, sizeof(st_rtk_temperature_callback), compare);
            return 0;
        }
    }
    return -1;
}
#endif



/* return -1 if wrong parameter
              -2 if callback function full
              0  if successfully
 */
int register_temperature_callback(int degree, void *fn, void* data, char *module_name)
{
    const int mnlen = sizeof(temperature_func[0].module_name) - 1;
    int i;
	if(degree > 250 || degree < -250 || fn == NULL || module_name == NULL)
		return -1;

	mutex_lock(&temperature_func_mutex);
    for(i = 0; i < RTK_TEMPE_CALLBACK_COUNT; i++) {
        /* search same degree handler and override it */
        if((degree == temperature_func[i].degree) &&
           (strcmp(temperature_func[i].module_name, module_name) == 0)) {
            temperature_func[i].func = fn;
            temperature_func[i].data = data;
            rtd_pr_thermal_info("re-register:%d %s %d\n", i, module_name, degree);
//            qsort((void *)temperature_func, RTK_TEMPE_CALLBACK_COUNT, sizeof(st_rtk_temperature_callback), compare);
			mutex_unlock(&temperature_func_mutex);
            return 0;
        }
    }

    for(i = 0; i < RTK_TEMPE_CALLBACK_COUNT; i++) {
        /* select to assign null element */
        if(temperature_func[i].func == NULL) {
            temperature_func[i].func = fn;
            temperature_func[i].data = data;
            temperature_func[i].degree = degree;
            strncpy(temperature_func[i].module_name, module_name, mnlen);
            if (strlen(module_name) > mnlen)
                temperature_func[i].module_name[mnlen] = 0;
//            qsort((void *)temperature_func, RTK_TEMPE_CALLBACK_COUNT, sizeof(st_rtk_temperature_callback), compare);
            rtd_pr_thermal_info("register:%d %s %d\n", i, module_name, degree);
			mutex_unlock(&temperature_func_mutex);
            return 0;
        }
    }

	rtd_pr_thermal_err("Error : register_temperature_callback fail(%d)\n", RTK_TEMPE_CALLBACK_COUNT);
	mutex_unlock(&temperature_func_mutex);
    return -2;
}

EXPORT_SYMBOL(register_temperature_callback);

void callback_fn(void *data, int cur_degree , int reg_degree, char *module_name)
{
    rtd_pr_thermal_info("callback current degree :%d reg_degree:%d data:%x %s\n", cur_degree, reg_degree, (data) ? * ((int *)data) : -1, module_name);
}

#if IS_ENABLED(CONFIG_RTK_KDRV_CPU_FREQUENCY)
#ifdef RTK_THERMAL_BRINGUP_VERIFY
int thermal_protect_disable = 1;
#else
int thermal_protect_disable = 0;
#endif
extern int ref_thermal_protect_disable;
#ifdef CONFIG_RTK_KDRV_THERMAL_SENSOR
static int __init rtk_thermal_protect_disable(char *str)
{
	thermal_protect_disable = simple_strtol(str, NULL, 0);
	ref_thermal_protect_disable = thermal_protect_disable;
	rtd_pr_thermal_err("Warning: no_thermal_protect=(%d)\n", thermal_protect_disable);
	return 1;
}
__setup("no_thermal_protect=",rtk_thermal_protect_disable);
#else
void rtk_parse_thermal_protect_disable(void)
{

	char strings[4];
	char *str = strings;

	if(rtk_parse_commandline_equal("no_thermal_protect", str, sizeof(strings)) == 0)
	{
		rtd_pr_thermal_err("Error : can't get no_thermal_protect from bootargs\n");
		return;
	}
	thermal_protect_disable = simple_strtol(str, NULL, 0);
	ref_thermal_protect_disable = thermal_protect_disable;
	rtd_pr_thermal_err("Warning: no_thermal_protect=(%d)\n", thermal_protect_disable);
	return;
}
#endif

#ifdef RTK_THERMAL_BRINGUP_VERIFY
static bool thermal_shutdown_disable = 1;
#else
static bool thermal_shutdown_disable = 0;
#endif
#ifdef CONFIG_RTK_KDRV_THERMAL_SENSOR
static int __init thermal_set_shutdown_disable(char *str)
{
	thermal_shutdown_disable = 1;
	rtd_pr_thermal_err("Warning: stop thermal shutdown\n");
	return 1;
}
__setup("no_thermal_shutdown",thermal_set_shutdown_disable);
#else
void rtk_parse_thermal_set_shutdown_disable(void)
{
	if(rtk_parse_commandline("no_thermal_shutdown"))
	{
		thermal_shutdown_disable = 1;
		rtd_pr_thermal_err("Warning: stop thermal shutdown\n");
		return;
	}
}
#endif

#define RTK_THERMAL_CALLBACK_INTERVAL_MS	1000
#define RTK_THERMAL_SHUTDOWN_SKIP_PRINT_INTERVAL_MS	1000
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
static void exec_callback_hot(struct thermal_zone_device *thermal)
{
	static unsigned long lasttime = 0;
	int degree = rtk_thermal_value_back(thermal->temperature / 1000);
	int i;

	if(thermal_protect_disable == 1)
		return;

	if(lasttime && time_before(jiffies, lasttime + msecs_to_jiffies(RTK_THERMAL_CALLBACK_INTERVAL_MS)))
		return;

	for(i = 0; i < RTK_TEMPE_CALLBACK_COUNT; i++)
	{
		if(temperature_func[i].func != NULL && temperature_func[i].degree != -255)
		{
			if(degree >= temperature_func[i].degree)
			{
				if((thermal_protect_disable > 1) && (thermal_protect_disable == i+2))
					rtd_pr_thermal_err("(%d) skip thermal module: %s\n", i, temperature_func[i].module_name);
				else
				{
					rtd_pr_thermal_info("(%d) cur:%d module_name:%s\n", i, degree,temperature_func[i].module_name);
					temperature_func[i].func(temperature_func[i].data, degree, temperature_func[i].degree, temperature_func[i].module_name);
				}
			}
		}
	}

	lasttime = jiffies;
	return;
}
static void exec_callback_critical(struct thermal_zone_device *thermal)
{
	static unsigned long last_shutdown_time = 0;
	int degree = rtk_thermal_value_back(thermal->temperature / 1000);

	if(thermal_shutdown_disable == 1)
	{
		if(last_shutdown_time && time_before(jiffies, last_shutdown_time + msecs_to_jiffies(RTK_THERMAL_SHUTDOWN_SKIP_PRINT_INTERVAL_MS)))
			return;

		rtd_pr_thermal_emerg("critical temperature reached(%d C), but skip shutdown !!\n",degree);
		last_shutdown_time = jiffies;
	}
	else
	{
		//force to shutdown system
		if(pm_power_off)
		{
			rtd_pr_thermal_emerg("critical temperature reached(%d C),shutting down\n", degree);
			kernel_power_off();
			while(1); //wait here for shutdown
		}
		else
			panic("critical temperature reached(%d C),but don't have platform power off callback !!\n", degree);
	}
	return;
}
#else
static int exec_callback_fn(struct thermal_zone_device *thermal, int trip, enum thermal_trip_type type)
{
	int i;
	int degree = thermal->temperature / 1000;
	degree = rtk_thermal_value_back(degree);

	if(degree > 250 || degree < -250)
		return -1;

	if(type == THERMAL_TRIP_HOT)
	{
		static unsigned long lasttime = 0;

		if(thermal_protect_disable == 1)
			return -1;

		if(lasttime && time_before(jiffies, lasttime + msecs_to_jiffies(RTK_THERMAL_CALLBACK_INTERVAL_MS)))
			return 0;

		for(i = 0; i < RTK_TEMPE_CALLBACK_COUNT; i++)
		{
			if(temperature_func[i].func != NULL && temperature_func[i].degree != -255)
			{
				if(degree >= temperature_func[i].degree)
				{
					if((thermal_protect_disable > 1) && (thermal_protect_disable == i+2))
						rtd_pr_thermal_err("(%d) skip thermal module: %s\n", i, temperature_func[i].module_name);
					else
					{
						rtd_pr_thermal_info("(%d) cur:%d module_name:%s\n", i, degree,temperature_func[i].module_name);
						temperature_func[i].func(temperature_func[i].data, degree, temperature_func[i].degree, temperature_func[i].module_name);
					}
				}
			}
        }

		lasttime = jiffies;
        }
	else if(type == THERMAL_TRIP_CRITICAL)
	{
		static unsigned long last_shutdown_time = 0;

		if(thermal_shutdown_disable == 1)
		{
			if(last_shutdown_time && time_before(jiffies, last_shutdown_time + msecs_to_jiffies(RTK_THERMAL_SHUTDOWN_SKIP_PRINT_INTERVAL_MS)))
				return 0;

			rtd_pr_thermal_emerg("critical temperature reached(%d C), but skip shutdown !!\n",degree);
			last_shutdown_time = jiffies;
		}
		else
		{
			//force to shutdown system
			if(pm_power_off)
			{
				rtd_pr_thermal_emerg("critical temperature reached(%d C),shutting down\n", degree);
				kernel_power_off();
				while(1); //wait here for shutdown
			}
			else
				panic("critical temperature reached(%d C),but don't have platform power off callback !!\n", degree);
		}
    }

    return 0;
}
#endif	//(LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
#endif	//CONFIG_CPU_FREQ

#if 0 // no use
static int init_flag = 0;
void callback_reg_test(void) //this is just for test callback
{
    int i;
    static int myint[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    if(!init_flag) {
        for(i = 0; i < RTK_TEMPE_CALLBACK_COUNT; i++) {
            temperature_func[i].degree = -255;
            temperature_func[i].func = NULL;
        }
        register_temperature_callback(70, callback_fn, (void *)&myint[0], "CPU");
        register_temperature_callback(70, callback_fn, (void *)&myint[1], "GPU");
        register_temperature_callback(77, callback_fn, (void *)&myint[3], "CPU");
        register_temperature_callback(77, callback_fn, (void *)&myint[4], "GPU");
        register_temperature_callback(80, callback_fn, (void *)&myint[5], "CPU");
        register_temperature_callback(80, callback_fn, (void *)&myint[6], "GPU");
        register_temperature_callback(85, callback_fn, (void *)&myint[7], "CPU");
        register_temperature_callback(85, callback_fn, (void *)&myint[8], "GPU");
        register_temperature_callback(90, callback_fn, (void *)&myint[9], "CPU");
        register_temperature_callback(90, callback_fn, (void *)&myint[2], "GPU");
        for(i = 0; i < RTK_TEMPE_CALLBACK_COUNT; i++) {
            rtd_pr_thermal_info("register callback temp degree:%d\n", temperature_func[i].degree);
        }
        init_flag = 0;
    }
}
#endif

/* Rtk generic registers */
/* In-kernel thermal framework related macros & definations */
#define SENSOR_NAME_LEN 16
#define MAX_TRIP_COUNT  8
#define MAX_COOLING_DEVICE 4
#define MAX_THRESHOLD_LEVS 4
#define ACTIVE_INTERVAL 50000
#define IDLE_INTERVAL 100000 //Have interrupt trigger, enlarge the idle interval
#define MCELSIUS    1000

/* CPU Zone information */
#define PANIC_ZONE      4
#define WARN_ZONE       3
#define MONITOR_ZONE    2
#define SAFE_ZONE       1

#define GET_ZONE(trip) (trip + 2)
#define GET_TRIP(zone) (zone - 2)

#define RTK_ZONE_COUNT  3
enum soc_type {
    SOC_RTK_RTD294X = 1,
    SOC_RTK_RTD298X,
    SOC_RTK_RTD299S,
};

enum calibration_type {
    TYPE_ONE_POINT_TRIMMING,
    TYPE_TWO_POINT_TRIMMING,
    TYPE_NONE,
};

struct freq_clip_table {
    unsigned int freq_clip_max;
    unsigned int temp_level;
    const struct cpumask *mask_val;
};

struct rtk_tmu_data {
    struct rtk_tmu_platform_data *pdata;
    struct resource *mem;
    void __iomem *base;
    /*      int irq; */
    enum soc_type soc;
    int irq;
    struct work_struct irq_work;
    struct mutex lock;
    /*  struct clk *clk; */
    u8 temp_error1, temp_error2;
};

struct thermal_trip_point_conf {
    int trip_val[MAX_TRIP_COUNT];
    int trip_count;
    u8 trigger_falling;
};

struct thermal_cooling_conf {
    struct freq_clip_table freq_data[MAX_TRIP_COUNT];
    int freq_clip_count;
};

#if IS_ENABLED(CONFIG_RTK_KDRV_CPU_FREQUENCY)
static irqreturn_t rtk_tmu_irq(int irq, void *id)
{
    struct rtk_tmu_data *data = id;

    disable_irq_nosync(irq);
    schedule_work(&data->irq_work);
    return IRQ_HANDLED;
}
#endif

struct thermal_sensor_conf {
    char name[SENSOR_NAME_LEN];
    int (*read_temperature) (void *data);
    int (*write_emul_temp) (void *drv_data, unsigned long temp);
    struct thermal_trip_point_conf trip_data;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    struct cpufreq_policy *policy;
#endif
    struct thermal_cooling_conf cooling_data;
    void *private_data;
};

struct rtk_thermal_zone {
    enum thermal_device_mode mode;
    struct thermal_zone_device *therm_dev;
    struct thermal_cooling_device *cool_dev[MAX_COOLING_DEVICE];
    unsigned int cool_dev_size;
    struct platform_device *rtk_dev;
    struct thermal_sensor_conf *sensor_conf;
    bool bind;
};

struct rtk_tmu_platform_data {
    u8 threshold;
    u8 threshold_falling;
    u8 trigger_levels[4];
    bool trigger_level0_en;
    bool trigger_level1_en;
    bool trigger_level2_en;
    bool trigger_level3_en;

    u8 gain;
    u8 reference_voltage;
    u8 noise_cancel_mode;
    u32 efuse_value;

    enum calibration_type cal_type;
    enum soc_type type;
    struct freq_clip_table freq_tab[4];
    unsigned int freq_tab_count;
};
#if IS_ENABLED(CONFIG_RTK_KDRV_CPU_FREQUENCY)
static struct rtk_thermal_zone *th_zone;

static void rtk_unregister_thermal(void);
static int rtk_register_thermal(struct thermal_sensor_conf *sensor_conf);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
/* Get mode callback functions for thermal zone */
static int rtk_get_mode(struct thermal_zone_device *thermal,
                        enum thermal_device_mode *mode)
{
    if (th_zone)
        *mode = th_zone->mode;
    return 0;
}
#endif

/* Set mode callback functions for thermal zone */
static int rtk_set_mode(struct thermal_zone_device *thermal,
                        enum thermal_device_mode mode)
{
    if(!th_zone) {
        rtd_pr_thermal_notice("thermal zone is null\n");
        return -1;
    }

    if (!th_zone->therm_dev) {
        rtd_pr_thermal_notice("thermal zone not registered\n");
        return 0;
    }

    mutex_lock(&th_zone->therm_dev->lock);

    if (mode == THERMAL_DEVICE_ENABLED &&
        !th_zone->sensor_conf->trip_data.trigger_falling)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
		th_zone->therm_dev->polling_delay_jiffies = msecs_to_jiffies(IDLE_INTERVAL);
#else
        th_zone->therm_dev->polling_delay = IDLE_INTERVAL;
#endif
    else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
        th_zone->therm_dev->polling_delay_jiffies = msecs_to_jiffies(IDLE_INTERVAL);
#else
        th_zone->therm_dev->polling_delay = IDLE_INTERVAL;
#endif

    mutex_unlock(&th_zone->therm_dev->lock);

    th_zone->mode = mode;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    thermal_zone_device_update(th_zone->therm_dev, THERMAL_EVENT_UNSPECIFIED);
#else
    thermal_zone_device_update(th_zone->therm_dev);
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
	rtd_pr_thermal_info("thermal polling set for duration=%d jiffies\n",
		th_zone->therm_dev->polling_delay_jiffies);
#else
    rtd_pr_thermal_info("thermal polling set for duration=%d msec\n",
            th_zone->therm_dev->polling_delay);
#endif
    return 0;
}

/* Get trip type callback functions for thermal zone */
static int rtk_get_trip_type(struct thermal_zone_device *thermal, int trip,
                             enum thermal_trip_type *type)
{
    switch (GET_ZONE(trip)) {
        case MONITOR_ZONE:
        case WARN_ZONE:
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	*type = THERMAL_TRIP_HOT;
#else
            *type = THERMAL_TRIP_ACTIVE;
#endif
            break;
        case PANIC_ZONE:
            *type = THERMAL_TRIP_CRITICAL;
            break;
        default:
            return -EINVAL;
    }
    return 0;
}

/* Get trip temperature callback functions for thermal zone */
static int rtk_get_trip_temp(struct thermal_zone_device *thermal, int trip,
                             int *temp)
{
    if (trip < GET_TRIP(MONITOR_ZONE) || trip > GET_TRIP(PANIC_ZONE))
        return -EINVAL;

    *temp = th_zone->sensor_conf->trip_data.trip_val[trip];
    /* convert the temperature into millicelsius */
    *temp = *temp * MCELSIUS;

    return 0;
}

/* Get critical temperature callback functions for thermal zone */
static int rtk_get_crit_temp(struct thermal_zone_device *thermal,
                             int *temp)
{
    int ret;
    /* Panic zone */
    ret = rtk_get_trip_temp(thermal, GET_TRIP(PANIC_ZONE), temp);
    return ret;
}

/* Bind callback functions for thermal zone */
static int rtk_bind(struct thermal_zone_device *thermal,
                    struct thermal_cooling_device *cdev)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	return 0;
#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    int ret = 0, i, tab_size;
#else
    int ret = 0, i, tab_size, level;
#endif
    struct freq_clip_table *tab_ptr, *clip_data;
    struct thermal_sensor_conf *data = th_zone->sensor_conf;

    tab_ptr = (struct freq_clip_table *)data->cooling_data.freq_data;
    tab_size = data->cooling_data.freq_clip_count;

    if (tab_ptr == NULL || tab_size == 0)
        return -EINVAL;

    /* find the cooling device registered */
    for (i = 0; i < th_zone->cool_dev_size; i++)
        if (cdev == th_zone->cool_dev[i])
            break;

    /* No matching cooling device */
    if (i == th_zone->cool_dev_size)
        return 0;

    /* Bind the thermal zone to the cpufreq cooling device */
    for (i = 0; i < tab_size; i++) {
        clip_data = (struct freq_clip_table *) & (tab_ptr[i]);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
        level = cpufreq_cooling_get_level(0, clip_data->freq_clip_max);
        rtd_pr_thermal_debug("binding level%d\n", level);
        if (level == THERMAL_CSTATE_INVALID)
            return 0;
#endif
        switch (GET_ZONE(i)) {
            case MONITOR_ZONE:
            case WARN_ZONE:
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
                if (thermal_zone_bind_cooling_device(thermal, i, cdev,
                                                     level, 0, THERMAL_WEIGHT_DEFAULT))
#else
                if (thermal_zone_bind_cooling_device(thermal, i, cdev,
                                                     THERMAL_NO_LIMIT, THERMAL_NO_LIMIT, THERMAL_WEIGHT_DEFAULT))

#endif
                {
                    rtd_pr_thermal_err("error binding cdev inst %d\n", i);
                    ret = -EINVAL;
                }
                th_zone->bind = true;
                break;
            default:
                ret = -EINVAL;
        }
    }

    return ret;
#endif
}

/* Unbind callback functions for thermal zone */
static int rtk_unbind(struct thermal_zone_device *thermal,
                      struct thermal_cooling_device *cdev)
{
    int ret = 0, i, tab_size;
    struct thermal_sensor_conf *data = th_zone->sensor_conf;

    if (th_zone->bind == false)
        return 0;

    tab_size = data->cooling_data.freq_clip_count;

    if (tab_size == 0)
        return -EINVAL;

    /* find the cooling device registered */
    for (i = 0; i < th_zone->cool_dev_size; i++)
        if (cdev == th_zone->cool_dev[i])
            break;

    /* No matching cooling device */
    if (i == th_zone->cool_dev_size)
        return 0;

    /* Bind the thermal zone to the cpufreq cooling device */
    for (i = 0; i < tab_size; i++) {
        switch (GET_ZONE(i)) {
            case MONITOR_ZONE:
            case WARN_ZONE:
                if (thermal_zone_unbind_cooling_device(thermal, i,
                                                       cdev)) {
                    rtd_pr_thermal_err("error unbinding cdev inst=%d\n", i);
                    ret = -EINVAL;
                }
                th_zone->bind = false;
                break;
            default:
                ret = -EINVAL;
        }
    }
    return ret;
}

/* Get temperature callback functions for thermal zone */
static int rtk_get_temp(struct thermal_zone_device *thermal,
                        int *temp)
{
    void *data;

    if (!th_zone->sensor_conf) {
        rtd_pr_thermal_info("Temperature sensor not initialised\n");
        return -EINVAL;
    }
    data = th_zone->sensor_conf->private_data;
    *temp = th_zone->sensor_conf->read_temperature(data);
    /* convert the temperature into millicelsius */
    *temp = *temp * MCELSIUS;
    return 0;
}

/* Get temperature callback functions for thermal zone */
static int rtk_set_emul_temp(struct thermal_zone_device *thermal,
                             int temp)
{
    void *data;
    int ret = -EINVAL;

    if (!th_zone->sensor_conf) {
        rtd_pr_thermal_info("Temperature sensor not initialised\n");
        return -EINVAL;
    }
    data = th_zone->sensor_conf->private_data;
    if (th_zone->sensor_conf->write_emul_temp)
        ret = th_zone->sensor_conf->write_emul_temp(data, temp);
    return ret;
}

/* Get the temperature trend */
static int rtk_get_trend(struct thermal_zone_device *thermal,
                         int trip, enum thermal_trend *trend)
{
    int ret;
    int trip_temp;

    ret = rtk_get_trip_temp(thermal, trip, &trip_temp);
    if (ret < 0)
        return ret;

    if (thermal->temperature >= trip_temp)
        *trend = THERMAL_TREND_RAISE_FULL;
    else
        *trend = THERMAL_TREND_DROP_FULL;

    return 0;
}

/* Operation callback functions for thermal zone */
static struct thermal_zone_device_ops rtk_dev_ops = {
    .bind = rtk_bind,
    .unbind = rtk_unbind,
    .get_temp = rtk_get_temp,
    .set_emul_temp = rtk_set_emul_temp,
    .get_trend = rtk_get_trend,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
    .get_mode = rtk_get_mode,
    .set_mode = rtk_set_mode,
#else
	.change_mode = rtk_set_mode,
#endif
    .get_trip_type = rtk_get_trip_type,
    .get_trip_temp = rtk_get_trip_temp,
    .get_crit_temp = rtk_get_crit_temp,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
	.hot = exec_callback_hot,
	.critical = exec_callback_critical,
#else
    .notify = exec_callback_fn,
#endif
};

/*
 * This function may be called from interrupt based temperature sensor
 * when threshold is changed.
 */
extern unsigned int rtk_monitor_debug;
static void rtk_report_trigger(void)
{
    unsigned int i;
    char data[10];
    char *envp[] = { data, NULL };

    if (!th_zone || !th_zone->therm_dev)
        return;
    if (th_zone->bind == false) {
        for (i = 0; i < th_zone->cool_dev_size; i++) {
            if (!th_zone->cool_dev[i])
                continue;
            rtk_bind(th_zone->therm_dev, th_zone->cool_dev[i]);
        }
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    thermal_zone_device_update(th_zone->therm_dev, THERMAL_EVENT_UNSPECIFIED);
#else
    thermal_zone_device_update(th_zone->therm_dev);
#endif

    mutex_lock(&th_zone->therm_dev->lock);
    /* Find the level for which trip happened */
    for (i = 0; i < th_zone->sensor_conf->trip_data.trip_count; i++) {
        if (th_zone->therm_dev->last_temperature <
            th_zone->sensor_conf->trip_data.trip_val[i] * MCELSIUS)
            break;
    }

    if (th_zone->mode == THERMAL_DEVICE_ENABLED &&
        !th_zone->sensor_conf->trip_data.trigger_falling) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
	if (i > 0)
		th_zone->therm_dev->polling_delay_jiffies = msecs_to_jiffies(ACTIVE_INTERVAL);
	else
		th_zone->therm_dev->polling_delay_jiffies = msecs_to_jiffies(IDLE_INTERVAL);
#else
        if (i > 0)
            th_zone->therm_dev->polling_delay = ACTIVE_INTERVAL;
        else
            th_zone->therm_dev->polling_delay = IDLE_INTERVAL;
#endif
    }

    snprintf(data, sizeof(data), "%u", i);
	//when enable debug thermal protect, discard to send uevent to userspace to avoid losts of uevnet
	if(rtk_monitor_debug < 2)
		kobject_uevent_env(&th_zone->therm_dev->device.kobj, KOBJ_CHANGE, envp);
    mutex_unlock(&th_zone->therm_dev->lock);
}

/* Register with the in-kernel thermal management */
static int rtk_register_thermal(struct thermal_sensor_conf *sensor_conf)
{
    int ret;
    struct cpumask mask_val;

    if (!sensor_conf || !sensor_conf->read_temperature) {
        rtd_pr_thermal_err("Temperature sensor not initialised\n");
        return -EINVAL;
    }

    th_zone = kzalloc(sizeof(struct rtk_thermal_zone), GFP_KERNEL);
    if (!th_zone) {
        rtd_pr_thermal_err("thermal create th_zone failed\n");
        return -ENOMEM;
    }

    th_zone->sensor_conf = sensor_conf;
    cpumask_set_cpu(0, &mask_val);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    th_zone->cool_dev[0] = cpufreq_cooling_register(sensor_conf->policy);
#else
    th_zone->cool_dev[0] = cpufreq_cooling_register(&mask_val);
#endif
    if (IS_ERR(th_zone->cool_dev[0])) {
        rtd_pr_thermal_err("Failed to register cpufreq cooling device\n");
        ret = -EINVAL;
        goto err_unregister;
    }
    th_zone->cool_dev_size++;

    th_zone->therm_dev = thermal_zone_device_register(sensor_conf->name,
                         RTK_ZONE_COUNT, 0,
                         NULL, &rtk_dev_ops,
                         NULL, 0,
			IDLE_INTERVAL);

    if (IS_ERR(th_zone->therm_dev)) {
        rtd_pr_thermal_err("Failed to register thermal zone device\n");
        ret = PTR_ERR(th_zone->therm_dev);
        goto err_unregister;
    }
    th_zone->mode = THERMAL_DEVICE_ENABLED;

/*
	For linux 5.15 thermal_zone_device_update() will call should_stop_polling()
	to check thermal_zone_device->mode is THERMAL_DEVICE_ENABLED or not.
*/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
	th_zone->therm_dev->mode = THERMAL_DEVICE_ENABLED;
#endif

    rtd_pr_thermal_info("Kernel TMU registered\n");

    return 0;

err_unregister:
    rtk_unregister_thermal();
    return ret;
}

/* Un-Register with the in-kernel thermal management */
static void rtk_unregister_thermal(void)
{
    int i;

    if (!th_zone)
        return;

    if (th_zone->therm_dev)
        thermal_zone_device_unregister(th_zone->therm_dev);

    for (i = 0; i < th_zone->cool_dev_size; i++) {
        if (th_zone->cool_dev[i])
            cpufreq_cooling_unregister(th_zone->cool_dev[i]);
    }

    kfree(th_zone);
    rtd_pr_thermal_info("Kernel TMU unregistered\n");
}
#endif

#if IS_ENABLED(CONFIG_RTK_KDRV_CPU_FREQUENCY)
static int rtk_tmu_initialize(struct platform_device *pdev)
{

    int ret = 0;
#ifdef CONFIG_RTK_KDRV_DYNAMIC_REFRESH_SETTING
	int temp = 0;
#endif

#ifdef CONFIG_RTK_KDRV_ENABLE_THERMAL_CALLBACK
    callback_reg_test();
#endif
    init_rtk_thermal_sensor();
#ifdef CONFIG_RTK_KDRV_DYNAMIC_REFRESH_SETTING
	temp = rtk_get_thermal_value_ext();

#ifdef CONFIG_ARCH_RTK2885P
	set_dram_refresh_rate_2885p(temp);
#else
	set_dram_refresh_rate(temp);
#endif

#endif
    rtk_thermal_trigger_high(85); //default trigger value, write high first
    rtk_thermal_trigger_low(75);
    return ret;
}

static void rtk_tmu_control(struct platform_device *pdev, bool on)
{
    struct rtk_tmu_data *data = platform_get_drvdata(pdev);

    mutex_lock(&data->lock);
    if (on) {
        rtk_thermal_sensor_enable(true);
    } else {
        rtk_thermal_sensor_enable(false);

    }
    mutex_unlock(&data->lock);
}


static int rtk_tmu_read(void *data)
{
    int temp;
    temp = rtk_get_thermal_value();
    /*rtd_pr_thermal_info("sensor value:%d %s\n", temp, __func__);*/
    return rtk_thermal_value_to(temp);
}

static int rtk_tmu_set_emulation(void *drv_data, unsigned long temp)
{
    return 0;
}


#ifdef CONFIG_RTK_KDRV_DYNAMIC_REFRESH_SETTING
/* Ref [ML7BU-1048] for ddr dynamic refresh rate control */

#include <rbus/dc_mc_reg.h>

#if !(defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2851F) || defined(CONFIG_ARCH_RTK2885P))
#include <rbus/dc_mc2_reg.h>
#endif

#ifdef CONFIG_ARCH_RTK2885P
#include <rbus/dc_mc3_reg.h>
static bool is_enable_dynamic_refresh_set1(void)
{
	unsigned int dc_mc_mc_dmy_fw15_val;
	dc_mc_mc_dmy_fw15_val = rtd_inl(DC_MC_MC_DMY_FW15_reg);
	if (dc_mc_mc_dmy_fw15_val & 0x1)
		return 1;
	else
		return 0;
}

static bool is_enable_dynamic_refresh_set2(void)
{
	unsigned int dc_mc_mc_dmy_fw15_val;
	dc_mc_mc_dmy_fw15_val = rtd_inl(DC_MC_MC_DMY_FW15_reg);
	if (dc_mc_mc_dmy_fw15_val & 0x2)
		return 1;
	else
		return 0;
}
static int set_dram_refresh_rate_2885p(int temp)
{


#define TEMP_DEBOUNCE				3

#define TEMP_TO_KGD(t)      (t - 15)
#define KGD_L1_DEFAULT            82
#define KGD_L2_DEFAULT            92
#define KGD_L1_REF_SETTING    0x289E015B
#define KGD_L2_REF_SETTING    0x144F015B
#define KGD_L3_REF_SETTING    0x0A28015B


#define TEMP_TO_DDR(t)      (t - 30)
#define DDR_L1_DEFAULT            82
#define DDR_L2_DEFAULT            92
#define DDR_L1_REF_SETTING    0x289E015B
#define DDR_L2_REF_SETTING    0x144F015B
#define DDR_L3_REF_SETTING    0x0A28015B


    static int KGD_L1_UP = KGD_L1_DEFAULT;
    static int KGD_L2_UP = KGD_L2_DEFAULT;
    static int DDR_L1_UP = DDR_L1_DEFAULT;
    static int DDR_L2_UP = DDR_L2_DEFAULT;

	int tj, l1_setting, l2_setting, l3_setting;
    unsigned int cur_val;
    bool change;

	//set 1 : KGD
	if(is_enable_dynamic_refresh_set1() == 1)
	{
		tj = TEMP_TO_KGD(temp);
		change = false;
		l1_setting = KGD_L1_REF_SETTING;
		l2_setting = KGD_L2_REF_SETTING;
		l3_setting = KGD_L3_REF_SETTING;
		cur_val = rtd_inl(DC_MC_MC_DPAR_REF_0_reg);

	    if (tj <= KGD_L1_UP) {
	        if (cur_val != l1_setting) {
	            rtd_outl(DC_MC_MC_DPAR_REF_0_reg, l1_setting);
	            KGD_L1_UP = KGD_L1_DEFAULT;
	            KGD_L2_UP = KGD_L2_DEFAULT;
	            change = true;
	        }
	    } else if (tj <= KGD_L2_UP) {
	        if (cur_val != l2_setting) {
	            rtd_outl(DC_MC_MC_DPAR_REF_0_reg, l2_setting);
	            KGD_L1_UP = KGD_L1_DEFAULT - TEMP_DEBOUNCE;
	            KGD_L2_UP = KGD_L2_DEFAULT;
	            change = true;
	        }
	    } else{
	        if (cur_val != l3_setting) {
	            rtd_outl(DC_MC_MC_DPAR_REF_0_reg, l3_setting);
	            KGD_L1_UP = KGD_L1_DEFAULT;
	            KGD_L2_UP = KGD_L2_DEFAULT - TEMP_DEBOUNCE;
	            change = true;
	        }
	    }

	    if (change)
	        rtd_pr_thermal_info("%s: [tmu] temp:%d, kgd:%d. 0x%x=0x%08x\n", __func__,
	                temp, tj, DC_MC_MC_DPAR_REF_0, rtd_inl(DC_MC_MC_DPAR_REF_0_reg));
	}

	//set 2 : DDR
	if(is_enable_dynamic_refresh_set2() == 1)
	{
		tj = TEMP_TO_DDR(temp);
		change = false;
		l1_setting = DDR_L1_REF_SETTING;
		l2_setting = DDR_L2_REF_SETTING;
		l3_setting = DDR_L3_REF_SETTING;
		cur_val = rtd_inl(DC_MC3_MC_DPAR_REF_0_reg);

	    if (tj <= DDR_L1_UP) {
	        if (cur_val != l1_setting) {
	            rtd_outl(DC_MC3_MC_DPAR_REF_0_reg, l1_setting);
	            DDR_L1_UP = DDR_L1_DEFAULT;
	            DDR_L2_UP = DDR_L2_DEFAULT;
	            change = true;
	        }
	    } else if (tj <= DDR_L2_UP) {
	        if (cur_val != l2_setting) {
	            rtd_outl(DC_MC3_MC_DPAR_REF_0_reg, l2_setting);
	            DDR_L1_UP = DDR_L1_DEFAULT - TEMP_DEBOUNCE;
	            DDR_L2_UP = DDR_L2_DEFAULT;
	            change = true;
	        }
	    } else{
	        if (cur_val != l3_setting) {
	            rtd_outl(DC_MC3_MC_DPAR_REF_0_reg, l3_setting);
	            DDR_L1_UP = DDR_L1_DEFAULT;
	            DDR_L2_UP = DDR_L2_DEFAULT - TEMP_DEBOUNCE;
	            change = true;
	        }
	    }

	    if (change)
	        rtd_pr_thermal_info("%s: [tmu] temp:%d, ddr:%d. 0x%x=0x%08x\n", __func__,
	                temp, tj, DC_MC3_MC_DPAR_REF_0, rtd_inl(DC_MC3_MC_DPAR_REF_0_reg));
	}

    return 0;
}



#else
static bool is_enable_dynamic_refresh(void)
{
	unsigned int dc_mc_mc_dmy_fw15_val;
	dc_mc_mc_dmy_fw15_val = rtd_inl(DC_MC_MC_DMY_FW15_reg);
	if (dc_mc_mc_dmy_fw15_val & 0x1)
		return 1;
	else
		return 0;
}
static bool is_DDR4(void)
{
	dc_mc_mc_sys_0_RBUS dc_mc_mc_sys_0;
#if !(defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2851F) || defined(CONFIG_ARCH_RTK2885P))
	dc_mc2_mc_sys_0_RBUS dc_mc2_mc_sys_0;
#endif

	dc_mc_mc_sys_0.regValue = rtd_inl(DC_MC_MC_SYS_0_reg);
#if !(defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2851F) || defined(CONFIG_ARCH_RTK2885P))
	dc_mc2_mc_sys_0.regValue = rtd_inl(DC_MC2_MC_SYS_0_reg);
#endif

#if !(defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2851F) || defined(CONFIG_ARCH_RTK2885P))
	if ((dc_mc_mc_sys_0.ddr4_en == 1) || (dc_mc2_mc_sys_0.ddr4_en == 1))
#else
	if (dc_mc_mc_sys_0.ddr4_en == 1)
#endif
		return 1;
	else
		return 0;
}

#if !(defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2851F) || defined(CONFIG_ARCH_RTK2885P))
static bool is_2MC(void)
{
	dc_mc_mc_sys_2_RBUS dc_mc_mc_sys2_0;
	dc_mc_mc_sys2_0.regValue = rtd_inl(DC_MC_MC_SYS_2_reg);
	if (dc_mc_mc_sys2_0.mcx2_en == 1)
		return 1;
	else
		return 0;

}
#endif

static int set_dram_refresh_rate(int temp)
{
#define KGD_L1_DEFAULT            82
#define KGD_L2_DEFAULT            92
#define KGD_L3_DEFAULT           102
#define TEMP_TO_KGD(t)      (t - 12)
#define TEMP_DEBOUNCE				3

#if defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2851F)  /*ref: MAC8PBU-487*/
#define L1_REF_SETTING_DDR4    0x289E01d3
#define L2_REF_SETTING_DDR4    0x144F01d3
#define L3_REF_SETTING_DDR4    0x0A2801d3
#else
#define L1_REF_SETTING_DDR4    0x289E015B
#define L2_REF_SETTING_DDR4    0x144F015B
#define L3_REF_SETTING_DDR4    0x0A28015B
#endif

#define L1_REF_SETTING_DDR3    0x207F0116
#define L2_REF_SETTING_DDR3    0x10400116
#define L3_REF_SETTING_DDR3    0x08200116

    static int KGD_L1_UP = KGD_L1_DEFAULT;
    static int KGD_L2_UP = KGD_L2_DEFAULT;
    static int KGD_L3_UP = KGD_L3_DEFAULT;
    int kgd_tj = TEMP_TO_KGD(temp);
    int l1_setting, l2_setting, l3_setting;
    unsigned int cur_val = 0;
    bool change = false;

	if(is_enable_dynamic_refresh() == 0)
		return 0;

    if (is_DDR4()) {
        l1_setting = L1_REF_SETTING_DDR4;
        l2_setting = L2_REF_SETTING_DDR4;
        l3_setting = L3_REF_SETTING_DDR4;
    } else {
        l1_setting = L1_REF_SETTING_DDR3;
        l2_setting = L2_REF_SETTING_DDR3;
        l3_setting = L3_REF_SETTING_DDR3;
    }

    cur_val = rtd_inl(DC_MC_MC_DPAR_REF_0_reg);

    if (kgd_tj <= KGD_L1_UP) {
        if (cur_val != l1_setting) {
            rtd_outl(DC_MC_MC_DPAR_REF_0_reg, l1_setting);
#if !(defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2851F) || defined(CONFIG_ARCH_RTK2885P))
			if(is_2MC())
				rtd_outl(DC_MC2_MC_DPAR_REF_0_reg, l1_setting);
#endif
            KGD_L1_UP = KGD_L1_DEFAULT;
            KGD_L2_UP = KGD_L2_DEFAULT;
            KGD_L3_UP = KGD_L3_DEFAULT;
            change = true;
        }
    } else if (kgd_tj <= KGD_L2_UP) {
        if (cur_val != l2_setting) {
            rtd_outl(DC_MC_MC_DPAR_REF_0_reg, l2_setting);
#if !(defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2851F) || defined(CONFIG_ARCH_RTK2885P))
			if(is_2MC())
				rtd_outl(DC_MC2_MC_DPAR_REF_0_reg, l2_setting);
#endif
            KGD_L1_UP = KGD_L1_DEFAULT - TEMP_DEBOUNCE;
            KGD_L2_UP = KGD_L2_DEFAULT;
            KGD_L3_UP = KGD_L3_DEFAULT;
            change = true;
        }
    } else{
        if (cur_val != l3_setting) {
            rtd_outl(DC_MC_MC_DPAR_REF_0_reg, l3_setting);
#if !(defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2851F) || defined(CONFIG_ARCH_RTK2885P))
			if(is_2MC())
				rtd_outl(DC_MC2_MC_DPAR_REF_0_reg, l3_setting);
#endif
            KGD_L1_UP = KGD_L1_DEFAULT;
            KGD_L2_UP = KGD_L2_DEFAULT - TEMP_DEBOUNCE;
            KGD_L3_UP = KGD_L3_DEFAULT;
            change = true;
        }
    }

    if (change)
        rtd_pr_thermal_info("%s: [tmu] temp:%d, kgd:%d. 0x%x=0x%08x\n", __func__,
                temp, kgd_tj, DC_MC_MC_DPAR_REF_0, rtd_inl(DC_MC_MC_DPAR_REF_0_reg));
    return 0;
}
#endif	//CONFIG_ARCH_RTK2885P
#endif	//CONFIG_RTK_KDRV_DYNAMIC_REFRESH_SETTING


static void rtk_tmu_work(struct work_struct *work)
{
    struct rtk_tmu_data *data = container_of(work,
                                struct rtk_tmu_data, irq_work);
    //unsigned int reg = rtk_thermal_int_status();
    int temp = rtk_get_thermal_value_ext();
    unsigned int reg = rtk_thermal_int_status();

#ifdef CONFIG_RTK_KDRV_DYNAMIC_REFRESH_SETTING
#ifdef CONFIG_ARCH_RTK2885P
	set_dram_refresh_rate_2885p(temp);
#else
	set_dram_refresh_rate(temp);
#endif
#endif

    rtk_thermal_trigger_high(temp + 1);
    rtk_thermal_trigger_low(temp - 1);
    rtd_pr_thermal_debug("rtk_tmu_work:%x %d\n", reg, temp);
    rtk_report_trigger();
    enable_irq(data->irq);
}

/*static irqreturn_t rtk_tmu_irq(int irq, void *id)
{
}*/
static struct thermal_sensor_conf rtk_sensor_conf = {
    .name = "rtk-thermal",
    .read_temperature = rtk_tmu_read,
    .write_emul_temp = rtk_tmu_set_emulation,
};
#endif

static struct rtk_tmu_platform_data rtk_rtd299s_default_tmu_data = {
    .threshold = 75,
    .trigger_levels[0] = 5,
    .trigger_levels[1] = 13,
    .trigger_levels[2] = 45,
    .trigger_level0_en = 1,
    .trigger_level1_en = 1,
    .trigger_level2_en = 1,
    .trigger_level3_en = 0,
    .gain = 15,
    .reference_voltage = 7,
    .cal_type = TYPE_ONE_POINT_TRIMMING,
    .freq_tab[0] = {
        .freq_clip_max = 1150 * 1000,
        .temp_level = 85,
    },
    .freq_tab[1] = {
        .freq_clip_max = 700 * 1000,
        .temp_level = 170,
    },
    .freq_tab_count = 2,
    .type = SOC_RTK_RTD299S,
};


#define RTD299S_TMU_DRV_DATA (&rtk_rtd299s_default_tmu_data)
struct rtk_tmu_data rtk_ops = {
    .pdata = &rtk_rtd299s_default_tmu_data,
    .soc = SOC_RTK_RTD299S,
};


#ifdef CONFIG_OF
static const struct of_device_id rtk_tmu_match[] = {
    {
        .compatible = "rtk_rtk299s-tmu",
        .data = (void *)&rtk_ops,
    },
    {},
};

MODULE_DEVICE_TABLE(of, rtk_tmu_match);
#endif

static struct platform_device_id rtk_tmu_driver_ids[] = {
    {
        .name = "rtk-rtd299s-tmu",
        .driver_data = (kernel_ulong_t) RTD299S_TMU_DRV_DATA,
    },
    {},
};

MODULE_DEVICE_TABLE(platform, rtk_tmu_driver_ids);

static inline struct rtk_tmu_platform_data *rtk_get_driver_data(struct
        platform_device
        *pdev)
{
#ifdef CONFIG_OF
    if (pdev->dev.of_node) {
        const struct of_device_id *match;
        match = of_match_node(rtk_tmu_match, pdev->dev.of_node);
        if (!match)
            return NULL;
        return (struct rtk_tmu_platform_data *)match->data;
    }
#endif
    return (struct rtk_tmu_platform_data *)
           platform_get_device_id(pdev)->driver_data;
}
#if 0 //no use
extern void * __init get_platform_throttle_resource(void);
static THROTTLE_CONTENT_T *pThrottle=NULL;

static void apply_callback(char *module_name, THROTTLE_CONTENT_T *pThrottle);
extern void vfe_register_HDMI_thermal(int t0, int t1, int t2);
#endif
static void rtk_tmu_thermal_zone(int t0, int t1, int t2, int t3);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
#if IS_ENABLED(CONFIG_RTK_KDRV_CPU_FREQUENCY)
static void dump_throttle_resource_log(THROTTLE_CONTENT_T *throttle_content)
{
        int i = 0;
        int j = 0;

        for(i=0; i<THROTTLE_LINE_MAX; i++)
        {
                if(!strlen(throttle_content[i].module_name_content))
                        continue;

                rtd_pr_thermal_debug("%s ", throttle_content[i].module_name_content);

                for(j=THROTTLE_ENABLE_KEY_E; j<THROTTLE_MAX_KEY_E; j++)
                        rtd_pr_thermal_debug("%d ", throttle_content[i].content[j-1]);
                rtd_pr_thermal_debug("\n");
        }
}
#endif
#endif
static struct timer_list thermal_timer;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
static void thermal_timer_callback(struct timer_list *t)
#else
static void thermal_timer_callback(unsigned long data)
#endif
{
	int temp = rtk_get_thermal_value_ext();
	int ret;
	if(temp!=0)  //0 stands not inited
	{
		unsigned int reg = rtk_thermal_int_status();
		temp = rtk_get_thermal_value_ext();
		rtk_thermal_trigger_high(temp + 1);
		rtk_thermal_trigger_low(temp - 1);
		rtd_pr_thermal_debug("rtk_timer:%x %d\n", reg, temp);
		rtk_thermal_sensor_enable(true);
	}
	ret = mod_timer( &thermal_timer, jiffies + msecs_to_jiffies(1000) );
	if (ret)
		rtd_pr_thermal_info("Error in thermal_timer\n");
}

int init_thermal_timer( void )
{
  int ret;

  // my_timer.function, my_timer.data
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
  timer_setup(&thermal_timer, thermal_timer_callback, 0);
#else
  setup_timer( &thermal_timer, thermal_timer_callback, 0 );
#endif

  ret = mod_timer( &thermal_timer, jiffies + msecs_to_jiffies(1000) );
  if (ret)
	rtd_pr_thermal_debug("Error in thermal_timer\n");

  return 0;
}

void clean_thermal_timer( void )
{
  int ret;

  ret = del_timer_sync( &thermal_timer );
  if (ret)
	rtd_pr_thermal_debug("The timer is still in use...\n");


  return;
}

static int rtk_tmu_probe(struct platform_device *pdev)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_CPU_FREQUENCY)
    struct rtk_tmu_data *data;
    struct rtk_tmu_platform_data *pdata;    /* = pdev->dev.platform_data; */
    int ret,i;
    const struct of_device_id *match;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	struct device_node *np = pdev->dev.of_node;
	int value[4];
#endif
    match = of_match_device(rtk_tmu_match, &pdev->dev);
    if (!match)
        return -ENODEV;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    rtk_sensor_conf.policy=cpufreq_cpu_get(0);
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))

	if(of_property_read_u32_array(np, "throttle", value,4))
	{
		rtd_pr_thermal_err("%s: failed to find throttle table on DT node\n",__func__);
		value[0] = RTK_THERMAL_THROTTLE_STEP0;
		value[1] = RTK_THERMAL_THROTTLE_STEP1;
		value[2] = RTK_THERMAL_THROTTLE_STEP2;
		value[3] = RTK_THERMAL_THROTTLE_STEP3;
	}
	/*[MA7PBU-162]*/
	if(value[1] >100)
		value[1] = 100;
	rtk_tmu_thermal_zone(value[0],value[1],value[2],value[3]);
#else
//Change the init for thermal core's temperature
    pThrottle=(THROTTLE_CONTENT_T *)get_platform_throttle_resource();
    if(pThrottle)
    {
	dump_throttle_resource_log(pThrottle);
	apply_callback("core",pThrottle);
    }
#endif
    data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);

    if (!data) {
        dev_err(&pdev->dev, "Failed to allocate driver structure\n");
        return -ENOMEM;
    }
    data = (struct rtk_tmu_data *)match->data;
    memset(temperature_func, 0, sizeof(temperature_func));

    data->irq = platform_get_irq(pdev, 0);
    if (data->irq < 0) {
        dev_err(&pdev->dev, "Failed to get platform irq\n");
        return data->irq;
    }

    INIT_WORK(&data->irq_work, rtk_tmu_work);

    data->mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    data->base = devm_ioremap_resource(&pdev->dev, data->mem);
    if (IS_ERR(data->base))
        return PTR_ERR(data->base);
    pdata = data->pdata;
    ret = devm_request_irq(&pdev->dev, data->irq, rtk_tmu_irq,
                           IRQF_TRIGGER_RISING, "rtk-tmu", data);
    if (ret) {
        dev_err(&pdev->dev, "Failed to request irq: %d\n", data->irq);
        return ret;
    }

    data->soc = pdata->type;
    data->pdata = pdata;
    platform_set_drvdata(pdev, data);
    mutex_init(&data->lock);

    ret = rtk_tmu_initialize(pdev);
    if (ret) {
        dev_err(&pdev->dev, "Failed to initialize TMU\n");
        goto err_clk;
    }

    /* Register the sensor with thermal management interface */
    (&rtk_sensor_conf)->private_data = data;
    rtk_sensor_conf.trip_data.trip_count = pdata->trigger_level0_en +
                                           pdata->trigger_level1_en + pdata->trigger_level2_en +
                                           pdata->trigger_level3_en;

    for (i = 0; i < rtk_sensor_conf.trip_data.trip_count; i++)
        rtk_sensor_conf.trip_data.trip_val[i] =
            pdata->threshold + pdata->trigger_levels[i];

    rtk_sensor_conf.trip_data.trigger_falling = pdata->threshold_falling;

    rtk_sensor_conf.cooling_data.freq_clip_count = pdata->freq_tab_count;
    for (i = 0; i < pdata->freq_tab_count; i++) {
        rtk_sensor_conf.cooling_data.freq_data[i].freq_clip_max =
            pdata->freq_tab[i].freq_clip_max;
        rtk_sensor_conf.cooling_data.freq_data[i].temp_level =
            pdata->freq_tab[i].temp_level;
    }

    ret = rtk_register_thermal(&rtk_sensor_conf);
    if (ret) {
        dev_err(&pdev->dev, "Failed to register thermal interface\n");
        goto err_clk;
    }
//  rtk_thermal_trigger_high(85); //default trigger value, write high first
//  rtk_thermal_trigger_low(75);
    rtk_tmu_control(pdev, true);


    device_enable_async_suspend(&pdev->dev);
    init_thermal_timer();
	rtk_thermal_sensor_init();
	rtk_monitor_init();
	register_cpu_thermal_handler(register_temperature_callback);
#ifdef CONFIG_RTK_KDRV_THERMAL_SENSOR_MODULE
	rtk_parse_thermal_set_shutdown_disable();
	rtk_parse_thermal_protect_disable();
#endif
    return 0;
err_clk:
    platform_set_drvdata(pdev, NULL);
    /*  clk_unprepare(data->clk); */
    return ret;

#else
    init_rtk_thermal_sensor();
    device_enable_async_suspend(&pdev->dev);
	rtk_thermal_sensor_init();
	rtk_monitor_init();
    return 0;
#endif
}

static int rtk_tmu_remove(struct platform_device *pdev)
{
	clean_thermal_timer();
	rtk_monitor_disable();

	rtk_thermal_sensor_exit();
#if IS_ENABLED(CONFIG_RTK_KDRV_CPU_FREQUENCY)
    //struct rtk_tmu_data *data = platform_get_drvdata(pdev);

    rtk_tmu_control(pdev, false);

    rtk_unregister_thermal();

    /*clk_unprepare(data->clk);*/

    platform_set_drvdata(pdev, NULL);
    exit_rtk_thermal_sensor();
#else
    exit_rtk_thermal_sensor();
#endif
    return 0;
}

#ifdef CONFIG_PM
static int rtk_thermal_suspend(void)
{
    exit_rtk_thermal_sensor();
    return 0;
}

#if !IS_ENABLED(CONFIG_RTK_KDRV_CPU_FREQUENCY)
static int rtk_thermal_resume(void)
{
    init_rtk_thermal_sensor();
    return 0;
}
#endif

static int rtk_tmu_resume(struct device *dev)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_CPU_FREQUENCY)
    struct platform_device *pdev = to_platform_device(dev);
    rtk_tmu_initialize(pdev);
    rtk_tmu_control(pdev, true);
#else
     rtk_thermal_resume();
#endif
    init_thermal_timer();
	rtk_monitor_resume();
     return 0;
}

static int rtk_tmu_suspend(struct device *dev)
{
    clean_thermal_timer();
    rtk_thermal_suspend();
#if IS_ENABLED(CONFIG_RTK_KDRV_CPU_FREQUENCY)
    rtk_tmu_control(to_platform_device(dev), false);
#endif

    return 0;
}

static const struct dev_pm_ops rtk_tmu_pm_ops = {
    .suspend    = rtk_tmu_suspend,
    .resume     = rtk_tmu_resume,
#ifdef CONFIG_HIBERNATION
    .freeze     = rtk_tmu_suspend,
    .thaw       = rtk_tmu_resume,
    .poweroff   = rtk_tmu_suspend,
    .restore    = rtk_tmu_resume,
#endif
};
#endif

static struct platform_driver rtk_tmu_driver = {
    .driver = {
        .name = "rtk-tmu",
        .owner = THIS_MODULE,
#ifdef CONFIG_PM
        .pm = &rtk_tmu_pm_ops,
#endif
        .of_match_table = of_match_ptr(rtk_tmu_match),
    },
    .probe = rtk_tmu_probe,
    .remove = rtk_tmu_remove,
    .id_table = rtk_tmu_driver_ids,
};

//EXPORT_SYMBOL(register_temperature_callback);
//EXPORT_SYMBOL(unregister_temperature_callback);

#define GET_TMU_VALUE(x) (x)

static void __maybe_unused rtk_tmu_thermal_zone(int t0, int t1, int t2, int t3)
{
	if((t3<=145) && (t3 >t2) && (t2 > t1) && (t1 > t0))
	{
		rtk_rtd299s_default_tmu_data.threshold = rtk_thermal_value_to(GET_TMU_VALUE(t0));
		rtk_rtd299s_default_tmu_data.trigger_levels[0]=rtk_thermal_value_to(GET_TMU_VALUE(t1)) - rtk_rtd299s_default_tmu_data.threshold;
		rtk_rtd299s_default_tmu_data.trigger_levels[1]=rtk_thermal_value_to(GET_TMU_VALUE(t2)) - rtk_rtd299s_default_tmu_data.threshold;
		rtk_rtd299s_default_tmu_data.trigger_levels[2]=rtk_thermal_value_to(GET_TMU_VALUE(t3)) - rtk_rtd299s_default_tmu_data.threshold;
	}
}

#if 0
static unsigned short acpu_t=80;
static unsigned short vcpu_t=80;
static unsigned short bush_t=80;
static unsigned long  bush_reg=0x017B2003; //475MHz

void avh_freq_thermal_callback(void *data, int cur_degree , int reg_degree, char *module_name)
{
	if (cur_degree <= acpu_t)
	{
		rtd_part_outl(SYS_REG_SYS_DYN_SW_ACPU_reg,15,11,0x0);// acpu1 /1 //recover
		rtd_part_outl(SYS_REG_SYS_DYN_SW_ACPU_reg,8,8,0x1);// active

	}
	else if (cur_degree > acpu_t)
	{
		rtd_part_outl(SYS_REG_SYS_DYN_SW_ACPU_reg,15,11,0x10);// acpu1 /2
		rtd_part_outl(SYS_REG_SYS_DYN_SW_ACPU_reg,8,8,0x1);// active
	}

	if (cur_degree <= vcpu_t)
	{
		rtd_part_outl(SYS_REG_SYS_DYN_SW_VCPU_reg,23,19,0x00);// vcpu /1 //recover
		rtd_part_outl(SYS_REG_SYS_DYN_SW_VCPU_reg,16,16,0x1);// active

	}
	else if (cur_degree > vcpu_t)
	{
		rtd_part_outl(SYS_REG_SYS_DYN_SW_VCPU_reg,23,19,0x10);// vcpu /2
		rtd_part_outl(SYS_REG_SYS_DYN_SW_VCPU_reg,16,16,0x1);// active
	}

	if (cur_degree <= bush_t)
	{
		rtd_outl(PLL_REG_SYS_DVFS_BUSH_reg,0x50200014);// Oc_en =0
		rtd_outl(PLL_REG_SYS_PLL_BUSH2_reg,bush_reg);// recover
		rtd_outl(PLL_REG_SYS_DVFS_BUSH_reg,0x50200015);// Oc_en =1
	}
	else
	{
//		bush_reg=rtd_inl(PLL_REG_SYS_PLL_BUSH2_reg);//backup bush_reg
		rtd_outl(PLL_REG_SYS_DVFS_BUSH_reg,0x50200014);// Oc_en =0
		rtd_outl(PLL_REG_SYS_PLL_BUSH2_reg,0x01C71303);// change to 300MHz
		rtd_outl(PLL_REG_SYS_DVFS_BUSH_reg,0x50200015);// Oc_en =1
	}
}

static void rtk_tmu_register_avh_freq(int t0, int t1, int t2)
{
	acpu_t=t0;
	vcpu_t=t1;
	bush_t=t2;

	bush_reg=rtd_inl(PLL_REG_SYS_PLL_BUSH2_reg);//backup bush_reg
	if(acpu_t<=70)
		acpu_t=120;
	if(vcpu_t<=70)
		vcpu_t=120;
	if(bush_t<=70)
		bush_t=120;

	rtd_pr_thermal_info("thermal avh_freq :%d %d %d\n",t0,t1,t2);
	register_temperature_callback(70, avh_freq_thermal_callback, NULL, "PLL Slow down");
}


unsigned int get_throttle_value(char *name, THROTTLE_CONTENT_T *pThrottle)
{
	unsigned int ret=0;
	unsigned int *data;
	char *module_name;
	int i;

    	if(pThrottle) {
		for(i=0;i<THROTTLE_LINE_MAX;i++) {
			if(pThrottle->module_name_content!=NULL)
			{
				module_name=pThrottle[i].module_name_content;
				data=pThrottle[i].content;
				if(strcmp(name,module_name)==0 && (data[0]==1))
				{ //Find the result
					ret=(data[4]<<24) | (data[3]<<16) | (data[2] <<8) | data[1];
					rtd_pr_thermal_info("clamp %s %x\n",name,ret);
				}
			}
		}
	}
	return ret;
}

static __attribute__((unused)) void apply_callback(char *name, THROTTLE_CONTENT_T *pThrottle)
{
	int *data;
	char *module_name;
	int i;
	int avh_flag=0;

	if(pThrottle) {
		for(i=0;i<THROTTLE_LINE_MAX;i++) {
			if(pThrottle->module_name_content!=NULL)
			{

				module_name=pThrottle[i].module_name_content;
				data=pThrottle[i].content;

				if(strcmp(name,module_name)==0 && (data[0]==1))
				{ //Find the result
					rtd_pr_thermal_info("thermal %s: %d %d %d %d\n",module_name,data[1],data[2],data[3],data[4]);
					if(strcmp(name, "core")==0) {
						/*[ML5BU-814]*/
						if(data[2]> 100)
						{
							data[2] = 100;
							rtd_pr_thermal_info("set thermal data[2]:%d\n", data[2]);
						}
						rtk_tmu_thermal_zone(data[1],data[2],data[3],data[4]);
					}
					else if(strcmp(name,"pq")==0) {
#ifdef CONFIG_SUPPORT_SCALER
						Scaler_register_PQ_thermal(data[1],data[2],data[3]);
#endif
					}
#if defined(CONFIG_MEMC) && defined(CONFIG_HW_SUPPORT_MEMC)
					else if(strcmp(name,"memc")==0) {
						Scaler_register_MEMC_thermal(data[1],data[2],data[3]);
					}
#endif
					else if(strcmp(name,"avh")==0)
					{
						rtk_tmu_register_avh_freq(data[1], data[2], data[3]);
						avh_flag=1;
					}
					else if(strcmp(name,"hdmi")==0) {
#ifdef CONFIG_RTK_HDMI_RX
						vfe_register_HDMI_thermal(data[1], data[2], data[3]);
#endif
					}
#if IS_ENABLED(CONFIG_RTK_AI_DRV)
					else if(strcmp(name,"ai")==0) {
#if IS_ENABLED(CONFIG_RTK_KDRV_THERMAL_SENSOR)
						ScalerAI_register_AI_thermal(data[1],data[2],data[3]);
#endif
					}
#endif
					return;
				}
			}
		}
	}
	if((strcmp(name,"avh")==0) && (avh_flag==0)) //set default
	{
		rtk_tmu_register_avh_freq(80, 85, 90);
	}

}
#endif

#if 0 //no use
int rtk_tmu_appy_resource(void)
{
#if 0 //[ML5BU-814]
	apply_callback("pq",pThrottle);
	apply_callback("memc",pThrottle);
	apply_callback("avh",pThrottle);
	apply_callback("hdmi",pThrottle);
#endif
	return 0;
}


unsigned int register_gpu_degree_callback(void)
{
	return get_throttle_value("gpu",pThrottle);
}

EXPORT_SYMBOL(register_gpu_degree_callback);
#endif
module_platform_driver(rtk_tmu_driver);
//late_initcall(rtk_tmu_appy_resource);
MODULE_DESCRIPTION("RTK TMU Driver");
MODULE_AUTHOR("<clamp.chien@realtek.com.tw");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:rtk-tmu");
