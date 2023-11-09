/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2013 by Chien-An Lin <colin@realtek.com.tw>
 *
 * Time initialization.
 */
#include <linux/clockchips.h>
#include <mach/platform.h>
#include <linux/export.h>
#include <rbus/stb_reg.h>
#include <rbus/drtc_reg.h>
#include <linux/init.h>
#include <rtd_log/rtd_module_log.h>       /* rtd_pr_rtc_info() */
#include <linux/fs.h>           /* everything... */
#include <linux/errno.h>        /* error codes */
#include <linux/types.h>        /* size_t */
#include <linux/slab.h>
#include <linux/time.h>
#ifndef CONFIG_ARM64
#include <asm/mach/time.h>
#endif

#if IS_ENABLED(CONFIG_RTK_KDRV_SB2)
#include <rtk_sb2_callback.h>
#endif
#include <rtk_kdriver/io.h>

DEFINE_SPINLOCK(rtc_lock);

#if IS_ENABLED(CONFIG_RTK_KDRV_TEE)
struct optee_rpc_param {
	u32	a0;
	u32	a1;
	u32	a2;
	u32	a3;
	u32	a4;
	u32	a5;
	u32	a6;
	u32	a7;
};
extern int optee_rtc_set_info_send(struct optee_rpc_param *p_rtc_set_info);
#endif

/*define*/
#define RTC_OPS_REQ_MAGIC               0xCC525443      //'R'(52)'T'(54)'C'(43)
#define RTC_OPS_SET_MMSS                0xCC00
#define RTC_OPS_SET_ALARM_MMSS          0xCC01
#define RTC_OPS_SET_AIE_ENABLE          0xCC02
#define RTC_OPS_SET_AIE_DISABLE         0xCC03

#define RTD_SHARE_MEM_BASE      0xb8060500
#define RTC_WAKEUP_FLAG         BIT(30)

#ifdef CONFIG_REALTEK_SECURE
static int rtk_rtc_cowork_with_secure_os(unsigned int         rtc_ops,
                                    unsigned int     sec,
                                    unsigned int     min,
                                    unsigned int     hour,
                                    unsigned int     day)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_TEE)
        struct optee_rpc_param rtc_set_info;

        rtc_set_info.a1 = RTC_OPS_REQ_MAGIC;
        rtc_set_info.a2 = rtc_ops;
        rtc_set_info.a3 = sec;
        rtc_set_info.a4 = min;
        rtc_set_info.a5 = hour;
        rtc_set_info.a6 = day;

        rtd_pr_rtc_notice("[%s][%d]Send cmd to optee OS.[rtc_ops = 0x%x]\n", __func__, __LINE__, rtc_ops);
        return optee_rtc_set_info_send(&rtc_set_info);
#else
        rtd_pr_rtc_err("[%s][%d] rtk_rtc_cowork_with_secure_os is not support without OPTEE OS!\n", __func__, __LINE__);
        return -1;
#endif
}
#endif

void rtk_rtc_dump_set_time(void)
{
    unsigned int min,hour,sec,day;
    unsigned int value = 0;
    rtd_pr_rtc_info("\n\n***********dump rtc device time************************");
    min = rtd_inl(DRTC_RTCMIN_reg);
    hour = rtd_inl(DRTC_RTCHR_reg);
    sec = rtd_inl(DRTC_RTCSEC_reg);
    day = rtd_inl(DRTC_RTCDATE1_reg);
    day += rtd_inl(DRTC_RTCDATE2_reg) << 8;
    rtd_pr_rtc_info("RTC_time:[day: %d] %02d:%02d:%02d\n",day,hour,min,(sec/2));

    min = rtd_inl(DRTC_ALMMIN_reg);
    hour = rtd_inl(DRTC_ALMHR_reg);
    sec = rtd_inl(DRTC_ALMHSEC_reg);
    day = rtd_inl(DRTC_ALMDATE1_reg);
    day += rtd_inl(DRTC_ALMDATE2_reg) << 8;
    rtd_pr_rtc_info("RTC_alarm:[day: %d] %02d:%02d:%02d\n",day,hour,min,(sec/2));


    value = rtd_inl(DRTC_RTCCR_reg);
    rtd_pr_rtc_info("interrupt status:(0x%08x) = 0x%08x \n", DRTC_RTCCR_reg, value);
    rtd_pr_rtc_info("rtc enable status:(0x%08x) = 0x%08x \n", DRTC_RTCEN_reg, rtd_inl(DRTC_RTCEN_reg));

    rtd_pr_rtc_info("IC version:(0x%08x) = 0x%08x \n", STB_SC_VerID_reg, rtd_inl(STB_SC_VerID_reg));
    rtd_pr_rtc_info("************dump rtc device time end***********************\n");

}

#if IS_ENABLED(CONFIG_RTK_KDRV_SB2)
void rtk_rtc_sb2_timeout_debug_info(void) {
        sb2_timeout_info rtc_timeout_info;
        //check trash module
        if(!isSb2Timeout("ST PERI")) {
                return;
        }

        //get sb2 timeout information
        get_sb2_timeout_info(&rtc_timeout_info);

        //trash address not match DRTC register range
        if((rtc_timeout_info.timeout_reg < DRTC_RTCSEC) || (rtc_timeout_info.timeout_reg > DRTC_RTCCR))
        {
            return ;
        }

        rtd_pr_rtc_err("Access DRTC register timeout,Trash address:0x%lx \n",rtc_timeout_info.timeout_reg);

        //printf debug info
        rtd_pr_rtc_err("DRTC clk: (0x%08x) = 0x%08x \n",STB_ST_CLKEN2_reg,rtd_inl(STB_ST_CLKEN2_reg));

        return;
}
#endif

void rtk_rtc_alarm_aie_enable(int state)
{

#ifdef CONFIG_REALTEK_SECURE
        if(state) {
                rtk_rtc_cowork_with_secure_os(RTC_OPS_SET_AIE_ENABLE, 0, 0, 0, 0);
        } else {
                rtk_rtc_cowork_with_secure_os(RTC_OPS_SET_AIE_DISABLE, 0, 0, 0, 0);
        }
#else
        unsigned long flags;

        spin_lock_irqsave(&rtc_lock, flags);
        if(state) {
                rtd_setbits(DRTC_RTCCR_reg, DRTC_RTCCR_alarminte_mask);
        } else {
                rtd_clearbits(DRTC_RTCCR_reg, DRTC_RTCCR_rtc_alarm_int_status_mask);
                rtd_clearbits(DRTC_RTCCR_reg, DRTC_RTCCR_alarminte_mask);
        }
        spin_unlock_irqrestore(&rtc_lock, flags);
#endif
        if(state){
            //inform 8051 do rtc_wakeup
            rtd_setbits(RTD_SHARE_MEM_BASE,RTC_WAKEUP_FLAG);
            rtk_rtc_dump_set_time();
        } else {
            rtd_clearbits(RTD_SHARE_MEM_BASE,RTC_WAKEUP_FLAG);
        }
}

int rtk_rtc_alarm_aie_state(void)
{
        int ret;

        if(rtd_inl(DRTC_RTCCR_reg) & DRTC_RTCCR_alarminte_mask)
                ret = 1;
        else
                ret = 0;

        return ret;
}

void rtk_rtc_read_persistent_clock(struct timespec64 *ts)
{
        unsigned int day, hour, min, sec;
        unsigned long flags;

        spin_lock_irqsave(&rtc_lock, flags);

        sec = rtd_inl(DRTC_RTCSEC_reg) >> 1;    // One unit represents half second
        min = rtd_inl(DRTC_RTCMIN_reg);
        hour = rtd_inl(DRTC_RTCHR_reg);
        day = rtd_inl(DRTC_RTCDATE1_reg);
        day += rtd_inl(DRTC_RTCDATE2_reg) << 8;

        spin_unlock_irqrestore(&rtc_lock, flags);
        ts->tv_sec = ((day * 24 + hour) * 60 + min) * 60 + sec + mktime64(platform_info.rtc_base_date_year, 1, 1, 0, 0, 0);
        ts->tv_nsec = 0;
}

int rtk_rtc_mips_set_mmss(unsigned long nowtime)
{
#ifndef CONFIG_REALTEK_SECURE
        unsigned long flags;
#endif
        unsigned int day, hour, min, sec, hms;
        unsigned long off_sec, base_sec = mktime64(platform_info.rtc_base_date_year, 1, 1, 0, 0, 0);

        off_sec = nowtime - base_sec;
        if(base_sec > nowtime) {
                rtd_pr_rtc_err("RTC set time error! The time cannot be set to the date before year %d.\n", platform_info.rtc_base_date_year);
                return -EINVAL;
        }

        day = off_sec / (24 * 60 * 60);
        hms = off_sec % (24 * 60 * 60);
        hour = hms / 3600;
        min = (hms % 3600) / 60;
        sec = ((hms % 3600) % 60) * 2;  // One unit represents half second.

        if(day > 16383) {
                rtd_pr_rtc_err("RTC day field overflow. I am so surprised that the Realtek product has been used for more than 40 years. Is it still workable? Hahaha...\n");
                return -EINVAL;
        }

#ifdef CONFIG_REALTEK_SECURE
        if(rtk_rtc_cowork_with_secure_os(RTC_OPS_SET_MMSS, sec, min, hour, day) < 0) {
                rtd_pr_rtc_err("Set RTC time fail when cowork with Secure OS!\n");
                return -EPIPE;
        }
#else
        /* irq are locally disabled here, but I still like to use spin_lock_irqsave */
        spin_lock_irqsave(&rtc_lock, flags);
        rtd_outl(DRTC_RTCSEC_reg, sec);
        rtd_outl(DRTC_RTCMIN_reg, min);
        rtd_outl(DRTC_RTCHR_reg, hour);
        rtd_outl(DRTC_RTCDATE1_reg, day & 0x00ff);
        rtd_outl(DRTC_RTCDATE2_reg, (day & 0x3f00) >> 8);
        spin_unlock_irqrestore(&rtc_lock, flags);
#endif
        rtd_pr_rtc_notice("[%s][%d]RTC set time success\n", __func__, __LINE__);
        return 0;
}

void rtk_rtc_read_alarm_persistent_clock(struct timespec64 *ts)
{
        unsigned int day, hour, min, sec;
        unsigned long flags;

        if(rtk_rtc_alarm_aie_state()) {
                spin_lock_irqsave(&rtc_lock, flags);
                sec = rtd_inl(DRTC_ALMHSEC_reg)>> 1;    // One unit represents half second
                min = rtd_inl(DRTC_ALMMIN_reg);
                hour = rtd_inl(DRTC_ALMHR_reg);
                day = rtd_inl(DRTC_ALMDATE1_reg);
                day += rtd_inl(DRTC_ALMDATE2_reg) << 8;
                spin_unlock_irqrestore(&rtc_lock, flags);
        } else {
                sec = 0;
                min = 0;
                hour = 0;
                day = 0;
        }
        ts->tv_sec = ((day * 24 + hour) * 60 + min) * 60 + sec + mktime64(platform_info.rtc_base_date_year, 1, 1, 0, 0, 0);
        ts->tv_nsec = 0;
}

int rtk_rtc_set_alarm_mmss(unsigned long nowtime)
{
#ifndef CONFIG_REALTEK_SECURE
        unsigned long flags;
#endif
        unsigned int day, hour, min, hms, sec;
        unsigned long off_sec, base_sec = mktime64(platform_info.rtc_base_date_year, 1, 1, 0, 0, 0);

        off_sec = nowtime - base_sec;
        if(base_sec > nowtime) {
                rtd_pr_rtc_err("RTC alarm set time error! The time cannot be set to the date before year %d.\n", platform_info.rtc_base_date_year);
                return -EINVAL;
        }

        day = off_sec / (24 * 60 * 60);
        hms = off_sec % (24 * 60 * 60);
        hour = hms / 3600;
        min = (hms % 3600) / 60;
        sec = ((hms % 3600) % 60) * 2;  // One unit represents half second.

        if(day > 16383) {
                rtd_pr_rtc_err("RTC alarm day field overflow.\n");
                return -EINVAL;
        }
        rtd_pr_rtc_info("ALARM_time need to set in rtc driver:[day: %d] %02d:%02d:%02d\n",day,hour,min,sec/2);

#ifdef CONFIG_REALTEK_SECURE
        if(rtk_rtc_cowork_with_secure_os(RTC_OPS_SET_ALARM_MMSS, sec, min, hour, day) < 0) {
                rtd_pr_rtc_err("Set RTC ALARM fail when cowork with Secure OS!\n");
                return -EPIPE;
        }
#else
        /* irq are locally disabled here, but I still like to use spin_lock_irqsave */
        spin_lock_irqsave(&rtc_lock, flags);
        rtd_outl(DRTC_ALMHSEC_reg, sec);
        rtd_outl(DRTC_ALMMIN_reg, min);
        rtd_outl(DRTC_ALMHR_reg, hour);
        rtd_outl(DRTC_ALMDATE1_reg, day & 0x00ff);
        rtd_outl(DRTC_ALMDATE2_reg, (day & 0x3f00) >> 8);
        spin_unlock_irqrestore(&rtc_lock, flags);
#endif
        return 0;
}

int rtk_rtc_ip_init(void)
{
        if (!STB_ST_CLKEN2_get_clken_rtc(rtd_inl(STB_ST_CLKEN2_reg))) {
                // enable DRTC clock
                rtd_outl(STB_ST_CLKEN2_reg, STB_ST_CLKEN2_clken_rtc(1) | STB_ST_CLKEN2_get_write_data(0));
                rtd_outl(STB_ST_SRST2_reg, STB_ST_SRST2_rstn_rtc(1) | STB_ST_SRST2_get_write_data(0));
                rtd_outl(STB_ST_SRST2_reg, STB_ST_SRST2_rstn_rtc(1) | STB_ST_SRST2_get_write_data(1));
                rtd_outl(STB_ST_CLKEN2_reg, STB_ST_CLKEN2_clken_rtc(1) | STB_ST_CLKEN2_get_write_data(1));

#ifndef CONFIG_REALTEK_SECURE
                // reset rtc alarm time info
                rtd_outl(DRTC_ALMHSEC_reg, 0x0);
                rtd_outl(DRTC_ALMMIN_reg, 0x0);
                rtd_outl(DRTC_ALMHR_reg, 0x0);
                rtd_outl(DRTC_ALMDATE1_reg, 0x0);
                rtd_outl(DRTC_ALMDATE2_reg, 0x0);

                // reset rtc time info
                rtd_outl(DRTC_RTCSEC_reg, 0x0); // sec
                rtd_outl(DRTC_RTCMIN_reg, 0x0); // min
                rtd_outl(DRTC_RTCHR_reg, 0x0); // hour
                rtd_outl(DRTC_RTCDATE1_reg, 0x0); // date LSB
                rtd_outl(DRTC_RTCDATE2_reg, 0x0); // date MSB

                //reset rtc alarm
                rtd_clearbits(DRTC_RTCCR_reg, DRTC_RTCCR_alarminte_mask);
                rtd_setbits(DRTC_RTCCR_reg, DRTC_RTCCR_rtc_alarm_int_status_mask);

                // enable RTC
                rtd_maskl(DRTC_RTCEN_reg, ~DRTC_RTCEN_rtcen_mask, 0x5A);
                rtd_pr_rtc_notice("(0x%x) = 0x%x,   (0x%x) = 0x%x\n", DRTC_RTCEN_reg, rtd_inl(DRTC_RTCEN_reg),DRTC_RTCCR_reg, rtd_inl(DRTC_RTCCR_reg));
                rtd_pr_rtc_notice("DRTC clock  enable...done \n");

#endif
        }

        if((platform_info.rtc_base_date_year > 2005) || (platform_info.rtc_base_date_year < 1985)) {
            //rtd_pr_rtc_notice(" Base year of RTC error. Reset to year 2005.\n");
            platform_info.rtc_base_date_year = 2005;
        }

        return 0;
}

int rtk_rtc_early_init(void)
{
        rtk_rtc_ip_init();
#ifndef CONFIG_ARM64
        register_persistent_clock(NULL, (clock_access_fn)rtk_rtc_read_persistent_clock);
#endif
        return 0;
}

//disable rtc alarm & clear rtc alarm interrupt pending bit.
int rtk_alarm_init(void)
{
	rtk_rtc_alarm_aie_enable(0);
	return 0;
}

//late_initcall(rtk_alarm_init);
EXPORT_SYMBOL(rtk_alarm_init);
EXPORT_SYMBOL(rtk_rtc_early_init);
EXPORT_SYMBOL(rtk_rtc_ip_init);
EXPORT_SYMBOL(rtk_rtc_read_persistent_clock);
EXPORT_SYMBOL(rtk_rtc_mips_set_mmss);
EXPORT_SYMBOL(rtk_rtc_read_alarm_persistent_clock);
EXPORT_SYMBOL(rtk_rtc_set_alarm_mmss);
EXPORT_SYMBOL(rtk_rtc_alarm_aie_enable);
EXPORT_SYMBOL(rtk_rtc_alarm_aie_state);

