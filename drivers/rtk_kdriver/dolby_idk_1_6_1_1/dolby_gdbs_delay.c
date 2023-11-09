#include <linux/list.h>
#include <linux/timer.h>
#include <linux/spinlock.h>
#include <linux/kernel.h>
#include <linux/slab.h>

#include <rtk_pwm.h>
#include <rtk_kdriver/io.h>
#include <rbus/timer_reg.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/scaler/scalerstruct.h>
#include <tvscalercontrol/scaler/scalercolorlib.h>

#include "dolby_gdbs_delay.h"
#include <rtd_log/rtd_module_log.h>

static struct dolby_vision_gdbs_delay_table gdbs_delay_table;

struct timer_work {
	struct list_head list;
	struct timer_list timer;
	spinlock_t lock;
	int32_t val;
	uint32_t start_time;
	uint32_t expect_delay;
};

const int max_timer_work_size = 32;
atomic_t timer_work_number;

struct queue {
	struct list_head list;
	spinlock_t lock;
};

struct queue free_queue, active_queue;

static void set_backlight_duty(struct timer_list *timer)
{
	struct timer_work *work = container_of(timer, struct timer_work, timer);
	int32_t val = work->val;
	unsigned long flags;

	uint32_t time_start = work->start_time;
	uint32_t time_now;

	extern unsigned char vdo0_connect;

	spin_lock_irqsave(&active_queue.lock, flags);
	list_del(&work->list);
	spin_unlock_irqrestore(&active_queue.lock, flags);

	if (val >= 0
	&& ( (get_OTT_HDR_mode() == HDR_DOLBY_COMPOSER && vdo0_connect)
	|| get_HDMI_HDR_mode() == HDR_DOLBY_HDMI
	)
)
#ifdef CONFIG_DOLBY_BACKLIGHT_CTRL_ADIM_PDIM
		set_dolby_duty_by_adim_pdim(val);
#else
		rtk_pwm_backlight_set_duty(val);
#endif

	spin_lock_irqsave(&free_queue.lock, flags);
	list_add_tail(&work->list, &free_queue.list);
	spin_unlock_irqrestore(&free_queue.lock, flags);

	time_now = rtd_inl(TIMER_VCPU_CLK90K_LO_reg);
	rtd_pr_hdr_debug("[DolbyVision][%s:%d] delay %03u (%03d) msec pwm %03d (%02d)\n",
					__func__, __LINE__,
					(time_now - time_start) / 90,
					work->expect_delay,
					val,
					atomic_read(&timer_work_number));
}

static struct timer_work *get_free_timer_work(void)
{
	unsigned long flags;
	struct timer_work *work = NULL;

	spin_lock_irqsave(&free_queue.lock, flags);

	if (list_empty(&free_queue.list)) {
		spin_unlock_irqrestore(&free_queue.lock, flags);

		if (atomic_read(&timer_work_number) < max_timer_work_size)
			work = kzalloc(sizeof(struct timer_work), GFP_ATOMIC);
		if (work == NULL)
			goto get_free_timer_work_alloc_memory_error;

		atomic_inc(&timer_work_number);
		timer_setup(&work->timer, set_backlight_duty, 0);
		INIT_LIST_HEAD(&work->list);
	} else {
		struct list_head *next  = free_queue.list.next;

		work = list_entry(next, struct timer_work, list);

		list_del(&work->list);
		spin_unlock_irqrestore(&free_queue.lock, flags);
	}

get_free_timer_work_alloc_memory_error:

	return work;
}

int set_global_dimming_duty_by_timer(int32_t val, int32_t msec)
{
	int ret = -1;

	struct timer_work *work = get_free_timer_work();

	if (work) {
		struct timer_list *timer = &work->timer;
		unsigned long flags;

		work->val = val;
		work->start_time = rtd_inl(TIMER_VCPU_CLK90K_LO_reg);
		work->expect_delay = msec;

		spin_lock_irqsave(&free_queue.lock, flags);
		list_add_tail(&work->list, &active_queue.list);
		spin_unlock_irqrestore(&free_queue.lock, flags);

		mod_timer(timer, jiffies + msecs_to_jiffies(msec));

		ret = 0;
	}

	return ret;
}

int set_global_dimming_duty(int32_t val, int32_t msec)
{
    const int threshold_using_timer = 5;

    int ret;

    { //debug area
        static const int DEBUG_DUMP = 1000;
        static int debug_dump_count = 0;
        if(!debug_dump_count)
            pr_notice("[Dolby][%s-%d] delay - value: %d, msec: %d, threshold: %d\n", __func__, __LINE__, val, msec, threshold_using_timer);
        debug_dump_count++;
        debug_dump_count %= DEBUG_DUMP;
    }
    if (msec < threshold_using_timer) {
#ifdef CONFIG_DOLBY_BACKLIGHT_CTRL_ADIM_PDIM
        ret = set_dolby_duty_by_adim_pdim(val);
#else
        ret = rtk_pwm_backlight_set_duty(val);
#endif
    } else {
        ret = set_global_dimming_duty_by_timer(val, msec);
    }

    return ret;
}

void initial_timer_work(void)
{
	INIT_LIST_HEAD(&free_queue.list);
	INIT_LIST_HEAD(&active_queue.list);

	spin_lock_init(&free_queue.lock);
	spin_lock_init(&active_queue.lock);
}

void del_timer_work(void)
{
	return;
}

void set_gdbs_delay_matrix_table(struct dolby_vision_gdbs_delay_data table) {
	gdbs_delay_table.delay_hdmi[FRAME_RATE_24] = table.delay_hdmi_fps_24;
	gdbs_delay_table.delay_hdmi[FRAME_RATE_30] = table.delay_hdmi_fps_30;
	gdbs_delay_table.delay_hdmi[FRAME_RATE_50] = table.delay_hdmi_fps_50;
	gdbs_delay_table.delay_hdmi[FRAME_RATE_60] = table.delay_hdmi_fps_60;

	gdbs_delay_table.delay_ott_fhd[FRAME_RATE_24] = table.delay_ott_fhd_fps_24;
	gdbs_delay_table.delay_ott_fhd[FRAME_RATE_30] = table.delay_ott_fhd_fps_30;
	gdbs_delay_table.delay_ott_fhd[FRAME_RATE_50] = table.delay_ott_fhd_fps_50;
	gdbs_delay_table.delay_ott_fhd[FRAME_RATE_60] = table.delay_ott_fhd_fps_60;

	gdbs_delay_table.delay_ott_uhd[FRAME_RATE_24] = table.delay_ott_uhd_fps_24;
	gdbs_delay_table.delay_ott_uhd[FRAME_RATE_30] = table.delay_ott_uhd_fps_30;
	gdbs_delay_table.delay_ott_uhd[FRAME_RATE_50] = table.delay_ott_uhd_fps_50;
	gdbs_delay_table.delay_ott_uhd[FRAME_RATE_60] = table.delay_ott_uhd_fps_60;
}

static enum FRAME_RATE get_current_frame_rate(void)
{
	unsigned int freq = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_FREQ) / 10;

	enum FRAME_RATE frame_rate;

	if (freq < 27) {
		frame_rate = FRAME_RATE_24;
	} else if (freq < 40) {
		frame_rate = FRAME_RATE_24;
	} else if (freq < 55) {
		frame_rate = FRAME_RATE_60;
	} else {
		frame_rate = FRAME_RATE_60;
	}

	return frame_rate;
}

static int32_t get_video_delay(void)
{
	uint32_t video_delay = 0;
	enum FRAME_RATE frame_rate = get_current_frame_rate();

	if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)
		video_delay = gdbs_delay_table.delay_hdmi[frame_rate];
	else
		video_delay = gdbs_delay_table.delay_ott_uhd[frame_rate];

	return video_delay;
}

static bool flag_enable_dolby_vision_pwm_function = true;

void enabe_doolby_vision_pwm_func(bool enable)
{
	flag_enable_dolby_vision_pwm_function = enable;
}

void dolby_set_pwm_duty(int32_t duty)
{
	int32_t video_delay = get_video_delay();

	if (flag_enable_dolby_vision_pwm_function)
		set_global_dimming_duty(duty, video_delay);
}
