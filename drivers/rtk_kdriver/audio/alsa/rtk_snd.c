/*
 *  Mars soundcard
 *  Copyright (c) by Jaroslav Kysela <perex@suse.cz>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/wait.h>
#include <linux/moduleparam.h>
#include <sound/core.h>
#include <sound/control.h>
#include <sound/pcm_params.h>
#include <sound/rawmidi.h>
#include <sound/initval.h>
#include <linux/mutex.h>
#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */
#include <linux/syscalls.h> /* needed for the _IOW etc stuff used later */
#include <linux/mpage.h>
#include <linux/dcache.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <sound/asound.h>
#include "AudioInbandAPI.h"
#include <rtk_snd.h>
#include <asm/cacheflush.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <rtk_kdriver/rtkaudio.h>
#include <rbus/audio_reg.h>
#include <linux/platform_device.h>
#include <rtd_log/rtd_module_log.h>

#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
#include <rtk_kdriver/RPCDriver.h>
#endif

#define RPC_DVR_MALLOC_UNCACHED_SIZE (4096) //can't edit this size

MODULE_AUTHOR("EJ Hsu <ejhsu@realtek.com.tw>");
MODULE_DESCRIPTION("Mars soundcard");
MODULE_LICENSE("GPL");
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(5, 11, 0))
MODULE_SUPPORTED_DEVICE("{{ALSA,Mars soundcard}}");
#endif

int snd_open_count;		/* playback open count */
int snd_open_ai_count;	/* capture open count */
int alsa_agent;
extern void rtk_flush_range (const void *, const void *);

/* #define USE_DECODER */
#define MAX_PCM_SUBSTREAMS 8
#define MAX_MIDI_DEVICES 2
/* for capture feature */
#define MAX_AI_DEVICES 2
#define ENDIAN_CHANGE(x)	((((x)&0xff000000)>>24)|\
				(((x)&0x00ff0000)>>8)|\
				(((x)&0x0000ff00)<<8)|\
				(((x)&0x000000ff)<<24))

/* defaults */
#define MAX_BUFFER_SIZE (256*1024)
/* period size in bytes */
#define MIN_PERIOD_SIZE (64)
#define MAX_PERIOD_SIZE (0x40000)

#define USE_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S32_LE)

#ifdef CONFIG_CUSTOMER_TV006
#define USE_RATE (SNDRV_PCM_RATE_44100|SNDRV_PCM_RATE_48000)
#define USE_RATE_MIN 44100
#else
#define USE_RATE (SNDRV_PCM_RATE_16000|SNDRV_PCM_RATE_32000| \
                  SNDRV_PCM_RATE_44100|SNDRV_PCM_RATE_48000)
#define USE_RATE_MIN 16000
#endif

#define USE_RATE_MAX 48000

#ifndef USE_CHANNELS_MIN
#define USE_CHANNELS_MIN 2
#endif
#ifndef USE_CHANNELS_MAX
#define USE_CHANNELS_MAX 8
#endif
#ifndef USE_PERIODS_MIN
#define USE_PERIODS_MIN 2
#endif
#ifndef USE_PERIODS_MAX
#define USE_PERIODS_MAX 1024
#endif

#define USE_FIXED_AO_PINID

#ifdef CONFIG_ARM64
#define ALIGN4 (0xFFFFFFFFFFFFFFFCLL)
#else
#define ALIGN4 (0xFFFFFFFC)
#endif

#ifdef CONFIG_CUSTOMER_TV001
#ifndef RPC_ACPU_NOT_READY
#define RPC_ACPU_NOT_READY -3
#endif
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* EJ: device configuration */
static int index[SNDRV_CARDS] = SNDRV_DEFAULT_IDX;	/* Index 0-MAX */
static char *id[SNDRV_CARDS] = SNDRV_DEFAULT_STR;	/* ID for this card */
#ifndef CONFIG_CUSTOMER_TV006
static int enable[SNDRV_CARDS] = {1, 1, [2 ... (SNDRV_CARDS - 1)] = 0};
#else
static int enable[SNDRV_CARDS] = {1, [1 ... (SNDRV_CARDS - 1)] = 0};
#endif
static int pcm_devs[SNDRV_CARDS] = {[0 ... (SNDRV_CARDS - 1)] = MAX_PCM_DEVICES};
static int pcm_substreams[SNDRV_CARDS] = {[0 ... (SNDRV_CARDS - 1)] = MAX_PCM_SUBSTREAMS};
static int min_period_size = MIN_PERIOD_SIZE;

#define CARD0_CAP_DEV_NUM 5

#ifdef USE_FIXED_AO_PINID
static int used_ao_pin[MAX_PCM_DEVICES];
static int flush_error[MAX_PCM_DEVICES];
static int pause_error[MAX_PCM_DEVICES];
static int close_error[MAX_PCM_DEVICES];
static int release_error[MAX_PCM_DEVICES];
#endif

module_param_array(index, int, NULL, 0444);
MODULE_PARM_DESC(index, "Index value for mars soundcard.");
module_param_array(id, charp, NULL, 0444);
MODULE_PARM_DESC(id, "ID string for mars soundcard.");
module_param_array(enable, int, NULL, 0444);
MODULE_PARM_DESC(enable, "Enable this mars soundcard.");
module_param_array(pcm_devs, int, NULL, 0444);
MODULE_PARM_DESC(pcm_devs, "PCM devices # (0-7) for mars driver.");
module_param_array(pcm_substreams, int, NULL, 0444);
MODULE_PARM_DESC(pcm_substreams, "PCM substreams # (1-16) for mars driver.");
/* module_param_array(midi_devs, int, NULL, 0444); */
/* MODULE_PARM_DESC(midi_devs, "MIDI devices # (0-2) for mars driver."); */
module_param(min_period_size, int, 0444);

enum AUDIO_PATH {
	AUDIO_PATH_NONE = 0,
	AUDIO_PATH_DECODER_AO,
	AUDIO_PATH_AO,
	AUDIO_PATH_AO_BYPASS
};

enum RTK_SND_FLUSH_STATE {
	RTK_SND_FLUSH_STATE_NONE = 0,
	RTK_SND_FLUSH_STATE_WAIT,
	RTK_SND_FLUSH_STATE_FINISH
};

#define CONFIG_PCM_USING_HRTIMER 1

struct rtk_snd_pcm {
	struct snd_card_mars *card;
	spinlock_t *pcm_lock;
#if defined (CONFIG_PCM_USING_HRTIMER)
	struct hrtimer timer;
#else
	struct timer_list timer;
#endif
	int running;

	enum RTK_SND_FLUSH_STATE flush_state;
	unsigned int pcm_buffer_head;
	unsigned int pcm_size;		/* buffer sizze */
	unsigned int pcm_count;		/* period length */
	unsigned int pcm_jiffie;	/* bytes per one jiffie */
	int pcm_irq_pos;			/* IRQ position */
	unsigned int pcm_buf_pos;	/* position in buffer */
	unsigned int remain_sample;

	snd_pcm_access_t access;	/* access mode */
	snd_pcm_format_t format;	/* SNDRV_PCM_FORMAT_* */
	unsigned int rate;		/* rate in Hz */
	unsigned int channels;		/* channels */
	snd_pcm_uframes_t period_size;	/* period size */
	unsigned int periods;		/* periods */
	snd_pcm_uframes_t buffer_size;	/* buffer size */
	unsigned int sample_bits;

	snd_pcm_uframes_t buffer_byte_size;	/* buffer size */

	unsigned int period_jiffies;
	unsigned int wp;
	unsigned int rp_real;
	unsigned int out_sample_width;
	int output_frame_bytes;
	int output_sample_bytes;
	int input_sample_bytes;

	struct RBUF_HEADER_ARM ring_bak[8];
	/* ALSA buffer control */
	snd_pcm_uframes_t appl_ptr;
	/* buffer control */
	unsigned int ring_init;
	unsigned int extend_to_32be_ratio;
	/* realtek hw control */
	int ao_agent;
	int ao_pin_id;
	int dec_agent;
	int dec_pin_id;
	int agent_id;
	int pin_id;
	int volume;
	int volume_change;
	enum AUDIO_PATH audiopath;
	struct page *page;
	RINGBUFFER_HEADER *ring; /* realtek hw ring buffer control */
	RINGBUFFER_HEADER *inband; /* realtek hw ring buffer control */

	RINGBUFFER_HEADER hw_ring[8];
	RINGBUFFER_HEADER hw_inband_ring[8];
	unsigned int hw_inband_data[256];

	RINGBUFFER_HEADER *ao_in_ring;
	RINGBUFFER_HEADER ao_in_ring_instance[8];

	unsigned int *ao_in_ring_p[8];
	struct snd_pcm_substream *substream;

	void *vir_addr;
	unsigned long phy_addr; /* for UNCAC_BASE */

	long total_data_wb;	/* total data write in byte */
	long pre_time_ms; /* last time that updata wp in millisecond */
	long current_time_ms; /* current time in millisecond */
	unsigned long pre_writePtr;

	long pre_no_datatime_ms; /* last time that no data time in millisecond */
	long current_no_datatime_time_ms; /* current no data time in millisecond */

	long max_level; /* max data in ring buffer */
	long min_level; /* min data in ring buffer */

	int hwptr_error_times; /* error handle for ptr */
};

/* pcm capture stucture */
struct rtk_snd_cap_pcm {
	struct snd_card_mars *card;
	spinlock_t *pcm_lock;
	struct timer_list timer;
	int running;

	enum RTK_SND_FLUSH_STATE flush_state;
	unsigned int pcm_buffer_head;
	unsigned int pcm_size;		/* buffer sizze */
	unsigned int pcm_count;		/* period length */
	unsigned int pcm_jiffie;	/* bytes per one jiffie */
	int pcm_irq_pos;			/* IRQ position */
	unsigned int pcm_buf_pos;	/* position in buffer */
	unsigned int remain_sample;

	snd_pcm_access_t access;	/* access mode */
	snd_pcm_format_t format;	/* SNDRV_PCM_FORMAT_* */
	unsigned int rate;		/* rate in Hz */
	unsigned int channels;		/* channels */
	snd_pcm_uframes_t period_size;	/* period size */
	unsigned int periods;		/* periods */
	snd_pcm_uframes_t buffer_size;	/* buffer size */
	unsigned int sample_bits;

	snd_pcm_uframes_t buffer_byte_size;	/* buffer size */

	unsigned int period_jiffies;
	unsigned int out_sample_width;
	int input_sample_bytes;

	int rd_idx;

	/* ALSA buffer control */
	snd_pcm_uframes_t appl_ptr;

	/* for AIO_CONFIG */
	enum ENUM_AUDIO_IPT_SRC path_src;
	enum ENUM_AUDIO_BBADC_SRC bbadc_mux_in;
	enum ENUM_AUDIO_I2SI_SRC i2si_mux_in;
	enum ENUM_AUDIO_SPDIFI_SRC spdifi_mux_in;

	unsigned int ring_init;
	unsigned int extend_to_32be_ratio;
	/* realtek hw control */
	int ao_cap_agent;
	int ao_cap_pin;
	int ai_cap_agent;
	int ai_cap_pin;
	int volume;
	int volume_change;
	/* Android AI format */
	enum AUDIO_FORMAT_OF_AI_SEND_TO_ALSA ai_format;
	struct page *page;
	RINGBUFFER_HEADER *ring; /* realtek hw ring buffer control */
    /*int ring_offset;          //offset of struct header and ring*/
    /*int ring2_offset;*/
    RINGBUFFER_HEADER *inband; /* realtek hw ring buffer control */

	RINGBUFFER_HEADER hw_ring[8];
	RINGBUFFER_HEADER hw_inband_ring[8];
	unsigned int hw_inband_data[256];

	RINGBUFFER_HEADER *hw_ai_ring;
	RINGBUFFER_HEADER hw_ai_ring_instance[8];

	struct snd_pcm_substream *substream;

	void *ring_p;				/* for vir capture address */
	dma_addr_t ring_phy_addr;	/* for phy capture address */

	void *vir_addr;
	unsigned long phy_addr; /* for UNCAC_BASE */

	/* last time that updata wp in millisecond */
	long last_time_ms;
	/* current time in millisecond */
	long current_time_ms;
};

static struct platform_device *devices[SNDRV_CARDS];

static int rtk_snd_flush(struct snd_pcm_substream *substream);
static int rtk_snd_pause(struct snd_pcm_substream *substream);
static int rtk_snd_close(struct snd_pcm_substream *substream);
static int rtk_snd_playback_close(struct snd_pcm_substream *substream);
static int rtk_snd_create_decoder_agent(struct snd_pcm_substream *substream);
static int rtk_snd_init_connect_decoder_ao(struct snd_pcm_substream *substream);
static void rtk_snd_init_decoder_ring(struct snd_pcm_substream *substream);
static int rtk_snd_init_decoder_info(struct snd_pcm_substream *substream);
static int rtk_snd_set_ao_flashpin_volume(struct snd_pcm_substream *substream);
static void rtk_snd_resume(struct snd_pcm_substream *substream);
static int rtk_snd_init_ao_ring(struct snd_pcm_substream *substream);
static int rtk_snd_ao_info(struct snd_pcm_substream *substream);
static int rtk_snd_init(struct snd_card *card);
static int rtk_snd_open(struct snd_pcm_substream *substream);
static void rtk_snd_playback_volume_work(struct work_struct *work);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))

#if defined (CONFIG_PCM_USING_HRTIMER)
static void rtk_snd_pcm_freerun_timer_function(unsigned long data);
#else
static void rtk_snd_pcm_freerun_timer_function(struct timer_list *t);
#endif
static void rtk_snd_pcm_capture_timer_function(struct timer_list *t);

#else
static void rtk_snd_pcm_freerun_timer_function(unsigned long data);
static void rtk_snd_pcm_capture_timer_function(unsigned long data);
#endif
static void rtk_snd_fmt_convert_to_S16LE(struct snd_pcm_substream *substream,
	snd_pcm_uframes_t wp_next, snd_pcm_uframes_t wp, unsigned int adv_min);


#if defined (CONFIG_PCM_USING_HRTIMER)
static ktime_t dpcm_kt;
static enum hrtimer_restart dpcm_hrtimer_hander(struct hrtimer *timer)
{
	struct rtk_snd_pcm *dpcm = container_of(timer, struct rtk_snd_pcm, timer);

	rtk_snd_pcm_freerun_timer_function((unsigned long)dpcm);
	return HRTIMER_NORESTART;
}

static void dpcm_timer_init(struct rtk_snd_pcm *dpcm)
{
	/* rtk_snd.c:2879: rtk_snd_pcm_timer_start()::
		mod_timer(&dpcm->timer, jiffies);
	*/
	hrtimer_init(&dpcm->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	dpcm->timer.function = dpcm_hrtimer_hander;
}

static void dpcm_timer_start(struct rtk_snd_pcm *dpcm)
{
	unsigned int msec = dpcm->period_jiffies;
	int sec = msec / 1000;
	msec = msec % 1000;

	dpcm_kt = ktime_set(sec, msec * 1000000);
	hrtimer_start(&dpcm->timer, dpcm_kt, HRTIMER_MODE_REL);
}

static void dpcm_timer_stop(struct rtk_snd_pcm *dpcm, bool block)
{
	if (block) {
		hrtimer_cancel(&dpcm->timer);
	} else {
		hrtimer_try_to_cancel(&dpcm->timer);
	}
}
#endif

bool is_far_field(int dev)
{
	return ((dev == DMIC_DEVICE) || (dev == REFERENCE_DEVICE));
}

bool is_aaduio_device(int dev)
{
	return ((dev == AAUDIO_DEVICE) || (dev == AAUDIO_DEVICE_2));
}

typedef struct {
	RINGBUFFER_HEADER *ring2; /* realtek hw ring2 buffer control */
	RINGBUFFER_HEADER hw_ring2[8];
	unsigned int *hw_ai_ring_data2[8];
	void *ring_p2;	/* for capture address */
    unsigned int buffer_byte_size;
    int voice_init_done;
} ringbuf;

ringbuf *airing = NULL;

static int rtk_snd_set_ao_config(struct snd_pcm_substream *substream){
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct rtk_snd_cap_pcm *dpcm = runtime->private_data;
    long ret = S_OK;
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));

    audioConfig.msg_id = AUDIO_CONFIG_CMD_AO_OUTPUT_CONFIG;
    audioConfig.value[0] = runtime->channels;
    audioConfig.value[1] = runtime->rate;
    audioConfig.value[2] = dpcm->ao_cap_pin;
    audioConfig.value[3] = 0;
    ret = rtkaudio_send_audio_config(&audioConfig);

    return ret;
}

int create_voice_ring(struct device *dev)
{
    int i = 0;
    if(airing != NULL)
        return 0;
    airing = dvr_malloc_specific(sizeof(ringbuf), GFP_DCU1);
    memset(airing, 0, sizeof(ringbuf));
    airing -> voice_init_done = FALSE;

    /* frames per buffer * sample_bytes */
    airing->buffer_byte_size = 5 * 16000 * 6 * 2; /* 5s for 16k 6ch 16bit */
    airing->ring2 = (RINGBUFFER_HEADER *) ((unsigned long) airing->hw_ring2);
    /* allocate ring buffer */
    airing->ring_p2 = dvr_malloc_specific(airing->buffer_byte_size, GFP_DCU1);

    if ((void*)airing->ring_p2 == NULL) {
        rtd_pr_adsp_err("[ALSA] %d\n",__LINE__);
        return -ENOMEM;
    }

    for (i = 0; i < 1; i++) {
        airing->hw_ai_ring_data2[i] = (unsigned int *) (dvr_to_phys(airing->ring_p2) + i * airing->buffer_byte_size);
        if (airing->hw_ai_ring_data2[i] == NULL)
            return -ENOMEM;

        airing->ring2[i].beginAddr = htonl((unsigned long) airing->hw_ai_ring_data2[i]);

        airing->ring2[i].bufferID = htonl(RINGBUFFER_STREAM);
        airing->ring2[i].size = htonl((ulong) airing->buffer_byte_size);
        airing->ring2[i].numOfReadPtr = htonl(1);
        airing->ring2[i].writePtr = airing->ring2[i].beginAddr;
        airing->ring2[i].readPtr[0] = airing->ring2[i].beginAddr;
        airing->ring2[i].readPtr[1] = airing->ring2[i].beginAddr;
        airing->ring2[i].readPtr[2] = airing->ring2[i].beginAddr;
        airing->ring2[i].readPtr[3] = airing->ring2[i].beginAddr;
    }

    rtk_flush_range(airing, ((unsigned char*) airing) + sizeof(ringbuf));
    outer_flush_range((phys_addr_t) airing, (phys_addr_t)((unsigned long) airing) + sizeof(ringbuf)) ;
    return 0;
}

void set_voice_initialized(void)
{
    if(airing != NULL) {
        airing->voice_init_done = TRUE;
    }
    return;
}

int check_voice_ready(void)
{
    if(airing == NULL) {
        return FALSE;
    }
    return airing->voice_init_done;
}

static void update_hw_delay(struct snd_pcm_substream *substream)
{
	long value, wp, rp, length, delay;
	struct snd_pcm_runtime *runtime = substream->runtime;

	value = (long) rtd_inl(AUDIO_ADMA_RCONF_pbr_reg);
	wp = value & 0x0000efff;
	rp = (value & 0xefff0000) >> 16;

	value = (long) rtd_inl(AUDIO_ADMA_WRCONF_pbr_reg);
	length = (value & 0xefff0000) >> 16;

	delay = wp - rp;
	if (delay < 0) {
		delay += length;
	}

	runtime->delay = delay * 64 / 4;
	/*
	if (runtime->rate != 0) {
		rtd_pr_adsp_info("ALSA hw delay: %08x(%d samples, %d ms)\n", delay_90k, runtime->delay, runtime->delay*1000/runtime->rate);
	}
	*/
}

static int audio_send_rpc_command(int opt,
	unsigned long command, unsigned long param1,
	unsigned long param2, unsigned long param2_LE,
	unsigned long *retvalue)
{
	int ret, count;
	RPCRES_LONG *audio_ret;
	ret = 0;
	count = 0;

#ifdef CONFIG_CUSTOMER_TV001
	do {
		ret = send_rpc_command(opt, command, param1, param2, retvalue);

		if (ret == RPC_FAIL) {
			rtd_pr_adsp_err("[ALSA] RPC to ACPU fail!!\n");
			return -1;
		}

		if (ret == RPC_OK)
			break;

		// RPC_ACPU_NOT_READY
		msleep(100);
		count++;
	} while (count <= 100);

	if (ret == RPC_ACPU_NOT_READY) {
		rtd_pr_adsp_err("[ALSA] wait ACPU ready timeout!!!\n");
		return -1;
	}
#else
	if (send_rpc_command(opt, command, param1, param2, retvalue))
		ret = -1;
#endif

	audio_ret = (RPCRES_LONG *)param2_LE;

	if (command == ENUM_KERNEL_RPC_CHECK_READY ||
	command == ENUM_KERNEL_RPC_PRIVATEINFO ||
	command == ENUM_KERNEL_RPC_GET_MUTE_N_VOLUME) {
		if (*retvalue != S_OK) {
			rtd_pr_adsp_err("[ALSA] RPC S_OK fail\n");
			rtd_pr_adsp_err("[ALSA] retvalue %lx\n", *retvalue);
			ret = -1;
		}
	} else {
		if (*retvalue != S_OK || ntohl(audio_ret->result) != S_OK) {
			rtd_pr_adsp_err("[ALSA] RPC S_OK fail\n");
			rtd_pr_adsp_err("[ALSA] retvalue %lx, result %x, command %lx\n",
				*retvalue, ntohl(audio_ret->result), command);
			ret = -1;
		}
	}

	return ret;
}

#if 1
static int rtk_snd_create_decoder_agent(struct snd_pcm_substream *substream)
{
	struct AUDIO_RPC_INSTANCE *info, *info_audio;
	RPCRES_LONG *res, *res_audio;
	unsigned long ret;
	int result;

	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long phy_addr = 0;

	struct snd_pcm_runtime *runtime;
	struct rtk_snd_pcm *dpcm;

	if (substream == NULL)
		return -1;

	runtime = substream->runtime;
	dpcm = runtime->private_data;

	vir_addr = dvr_malloc_uncached_specific(RPC_DVR_MALLOC_UNCACHED_SIZE, GFP_DCU1, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[ALSA] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = (unsigned long) dvr_to_phys(vir_addr);

	/* set parameter to 0, for snapshot */
	memset(unvir_addr, 0, RPC_DVR_MALLOC_UNCACHED_SIZE);

	info = unvir_addr;
	info_audio = (void *)(struct AUDIO_RPC_INSTANCE *)((ulong) phy_addr);

	res = (void *)(RPCRES_LONG *) ((((unsigned long)unvir_addr + sizeof(struct AUDIO_RPC_INSTANCE) + 8) & ALIGN4));
	res_audio = (void *)(RPCRES_LONG *) (((((unsigned long) phy_addr) + sizeof(struct AUDIO_RPC_INSTANCE) + 8) & ALIGN4));

	/* create decoder */
	info->type = htonl(AUDIO_DECODER);
	info->instanceID = htonl(-1);

	if (audio_send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_CREATE_AGENT,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		(unsigned long) res, &ret)) {
		rtd_pr_adsp_err("[ALSA] %s %d RPC fail\n", __func__, __LINE__);
		dvr_free(vir_addr);
		return -1;
	}

	dpcm->dec_agent = dpcm->agent_id = ntohl(res->data);
	dpcm->dec_pin_id = dpcm->pin_id = BASE_BS_IN;
	result = dpcm->agent_id;
	rtd_pr_adsp_debug("[ALSA] Create decode instance %d\n", dpcm->agent_id);

	dvr_free(vir_addr);
	return result;
}

static int RPC_TOAGENT_INITRINGBUFFER_HEADER_SVC(struct snd_card *card,
	struct RPC_RBUF_HEADER *header)
{
	struct RPC_RBUF_HEADER *hd, *hd_audio;
	RPCRES_LONG *res, *res_audio;
	unsigned long ret;
	int i;

	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long phy_addr = 0;

	vir_addr = dvr_malloc_uncached_specific(RPC_DVR_MALLOC_UNCACHED_SIZE, GFP_DCU1, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[ALSA] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = (unsigned long) dvr_to_phys(vir_addr);

	/* set parameter to 0, for snapshot */
	memset(unvir_addr, 0, RPC_DVR_MALLOC_UNCACHED_SIZE);

	hd = unvir_addr;
	hd_audio = (void *)(struct RPC_RBUF_HEADER *)((ulong) phy_addr);

	res = (void *)(RPCRES_LONG *) ((((unsigned long) unvir_addr + sizeof(struct RPC_RBUF_HEADER) + 8) & ALIGN4));
	res_audio = (void *)(RPCRES_LONG *) (((((unsigned long) phy_addr) + sizeof(struct RPC_RBUF_HEADER) + 8) & ALIGN4));

	hd->instanceID = htonl(header->instanceID);
	hd->pin_id = htonl(header->pin_id);
	hd->rd_idx = htonl(header->rd_idx);
	hd->listsize = htonl(header->listsize);

	for (i = 0; i < 8; i++) {
		hd->rbuf_list[i] = htonl((ulong) header->rbuf_list[i]);
	}

	if (audio_send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_INIT_RINGBUF,
		(unsigned long) hd_audio,
		(unsigned long) res_audio,
		(unsigned long) res, &ret)) {
		rtd_pr_adsp_err("[ALSA] %d RPC fail\n", __LINE__);
		dvr_free(vir_addr);
		return -1;
	}

	dvr_free(vir_addr);
	return 0;
}

static int RPC_TOAGENT_CONNECT_SVC(struct snd_card *card,
	struct AUDIO_RPC_CONNECTION *pconnection)
{
	struct AUDIO_RPC_CONNECTION *info, *info_audio;
	RPCRES_LONG *res, *res_audio;
	unsigned long ret;

	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long phy_addr = 0;

	vir_addr = dvr_malloc_uncached_specific(RPC_DVR_MALLOC_UNCACHED_SIZE, GFP_DCU1, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[ALSA] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = (unsigned long) dvr_to_phys(vir_addr);

	/* set parameter to 0, for snapshot */
	memset(unvir_addr, 0, RPC_DVR_MALLOC_UNCACHED_SIZE);

	info = unvir_addr;
	info_audio = (void *) (struct AUDIO_RPC_CONNECTION *)((ulong) phy_addr);

	res = (void *)(RPCRES_LONG *) ((((unsigned long)unvir_addr + sizeof(struct AUDIO_RPC_CONNECTION) + 8) & ALIGN4));
	res_audio = (void *)(RPCRES_LONG *) (((((unsigned long) phy_addr) + sizeof(struct AUDIO_RPC_CONNECTION) + 8) & ALIGN4));

	info->srcInstanceID = htonl(pconnection->srcInstanceID);
	info->srcPinID = htonl(pconnection->srcPinID);
	info->desInstanceID = htonl(pconnection->desInstanceID);
	info->desPinID = htonl(pconnection->desPinID);

	if (audio_send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_CONNECT,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		(unsigned long) res, &ret)) {
		dvr_free(vir_addr);
		rtd_pr_adsp_err("[ALSA] %d RPC fail\n", __LINE__);
		return -1;
	}

	dvr_free(vir_addr);
	return 0;
}

static int RPC_TOAGENT_PAUSE_SVC(struct snd_card *card, int *inst_id)
{
	int *info, *info_audio;
	RPCRES_LONG *res, *res_audio;
	unsigned long ret;

	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long phy_addr = 0;

	vir_addr = dvr_malloc_uncached_specific(RPC_DVR_MALLOC_UNCACHED_SIZE, GFP_DCU1, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[ALSA] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = (unsigned long) dvr_to_phys(vir_addr);

	/* set parameter to 0, for snapshot */
	memset(unvir_addr, 0, RPC_DVR_MALLOC_UNCACHED_SIZE);

	info = unvir_addr;
	info_audio = (void *)(int *)((ulong) phy_addr);

	res = (void *)(RPCRES_LONG *) ((((unsigned long) unvir_addr + sizeof(int) + 8) & ALIGN4));
	res_audio = (void *)(RPCRES_LONG *) (((((unsigned long) phy_addr) + sizeof(int) + 8) & ALIGN4));

	*info = htonl(*inst_id);

	if (audio_send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_PAUSE,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		(unsigned long) res, &ret)) {
		rtd_pr_adsp_err("[ALSA] %d RPC fail\n", __LINE__);
		dvr_free(vir_addr);
		return -1;
	}

	dvr_free(vir_addr);
	return 0;
}

static int RPC_TOAGENT_RUN_SVC(struct snd_card *card, int *inst_id)
{
	int *info, *info_audio;
	RPCRES_LONG *res, *res_audio;
	unsigned long ret;

	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long phy_addr = 0;

	vir_addr = dvr_malloc_uncached_specific(RPC_DVR_MALLOC_UNCACHED_SIZE, GFP_DCU1, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[ALSA] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = (unsigned long) dvr_to_phys(vir_addr);

	/* set parameter to 0, for snapshot */
	memset(unvir_addr, 0, RPC_DVR_MALLOC_UNCACHED_SIZE);

	info = unvir_addr;
	info_audio = (void *)(int *)((ulong) phy_addr);

	res = (void *)(RPCRES_LONG *) ((((unsigned long)unvir_addr + sizeof(int) + 8) & ALIGN4));
	res_audio = (void *)(RPCRES_LONG *) (((((unsigned long) phy_addr) + sizeof(int) + 8) & ALIGN4));

	*info = htonl(*inst_id);

	if (audio_send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_RUN,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		(unsigned long) res, &ret)) {
		rtd_pr_adsp_err("[ALSA] %s %d RPC fail\n", __func__, __LINE__);
		dvr_free(vir_addr);
		return -1;
	}

	dvr_free(vir_addr);
	return 0;
}

static int RPC_TOAGENT_FLUSH_SVC(struct snd_card *card,
	struct AUDIO_RPC_SENDIO *sendio)
{
	struct AUDIO_RPC_SENDIO *info, *info_audio;
	RPCRES_LONG *res, *res_audio;
	unsigned long ret;

	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long phy_addr = 0;

	vir_addr = dvr_malloc_uncached_specific(RPC_DVR_MALLOC_UNCACHED_SIZE, GFP_DCU1, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[ALSA] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = (unsigned long) dvr_to_phys(vir_addr);

	/* set parameter to 0, for snapshot */
	memset(unvir_addr, 0, RPC_DVR_MALLOC_UNCACHED_SIZE);

	info = unvir_addr;
	info_audio = (void *)(struct AUDIO_RPC_SENDIO *)((ulong) phy_addr);

	res = (void *)(RPCRES_LONG *) ((((unsigned long)unvir_addr + sizeof(struct AUDIO_RPC_SENDIO) + 8) & ALIGN4));
	res_audio = (void *)(RPCRES_LONG *) (((((unsigned long) phy_addr) + sizeof(struct AUDIO_RPC_SENDIO) + 8) & ALIGN4));

	rtd_pr_adsp_debug("[ALSA] %s %ld %d\n", __func__, sendio->instanceID, sendio->pinID);

	info->instanceID = htonl(sendio->instanceID);
	info->pinID = htonl(sendio->pinID);

	if (audio_send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_FLUSH,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		(unsigned long) res, &ret)) {
		rtd_pr_adsp_err("[ALSA] %s %d RPC fail\n", __func__, __LINE__);
		dvr_free(vir_addr);
		return -1;
	}

	dvr_free(vir_addr);
	return 0;
}

int RPC_TOAGENT_STOP_SVC(struct snd_card *card, int *instanceID)
{
	int *info, *info_audio;
	RPCRES_LONG *res, *res_audio;
	unsigned long ret;

	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long phy_addr = 0;

	vir_addr = dvr_malloc_uncached_specific(RPC_DVR_MALLOC_UNCACHED_SIZE, GFP_DCU1, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[ALSA] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = (unsigned long) dvr_to_phys((void *) vir_addr);

	/* set parameter to 0, for snapshot */
	memset(unvir_addr, 0, RPC_DVR_MALLOC_UNCACHED_SIZE);

	info = unvir_addr;
	info_audio = (void *)(int *)((ulong) phy_addr);

	res = (void *)(RPCRES_LONG *) ((((unsigned long) unvir_addr + sizeof(int) + 8) & ALIGN4));
	res_audio = (void *)(RPCRES_LONG *) (((((unsigned long) phy_addr) + sizeof(int) + 8) & ALIGN4));

	*info = htonl(*instanceID);

	if (audio_send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_STOP,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		(unsigned long) res, &ret)) {
		rtd_pr_adsp_err("[ALSA] %s %d RPC fail\n", __func__, __LINE__);
		dvr_free(vir_addr);
		return -1;
	}

	dvr_free(vir_addr);
	return 0;
}

int RPC_TOAGENT_DESTROY_SVC(struct snd_card *card, int *instanceID)
{
	int *info, *info_audio;
	RPCRES_LONG *res, *res_audio;
	unsigned long ret;

	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long phy_addr = 0;

	vir_addr = dvr_malloc_uncached_specific(RPC_DVR_MALLOC_UNCACHED_SIZE, GFP_DCU1, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[ALSA] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = (unsigned long) dvr_to_phys(vir_addr);

	/* set parameter to 0, for snapshot */
	memset(unvir_addr, 0, RPC_DVR_MALLOC_UNCACHED_SIZE);

	info = unvir_addr;
	info_audio = (void *)(int *)((ulong) phy_addr);

	res = (void *)(RPCRES_LONG *) ((((unsigned long)unvir_addr + sizeof(int) + 8) & ALIGN4));
	res_audio = (void *)(RPCRES_LONG *) (((((unsigned long) phy_addr) + sizeof(int) + 8) & ALIGN4));

	*info = htonl(*instanceID);

	if (audio_send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_DESTROY,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		(unsigned long) res, &ret)) {
		rtd_pr_adsp_err("[ALSA] %s %d RPC fail\n", __func__, __LINE__);
		dvr_free(vir_addr);
		return -1;
	}

	dvr_free(vir_addr);
	return 0;
}

static int hw_ring_write(RINGBUFFER_HEADER *ring, void *data, int len)
{
	RINGBUFFER_HEADER tmp;
	unsigned char *ptr = data;

    if (alsa_debug)
        rtd_pr_adsp_debug("[ALSA] %s Enter\n", __func__);

	tmp.beginAddr = ntohl(ring->beginAddr);
	tmp.writePtr = ntohl(ring->writePtr);
	tmp.readPtr[0] = ntohl(ring->readPtr[0]);
	tmp.size = ntohl(ring->size);

	if (tmp.beginAddr + tmp.size <= tmp.writePtr + len) {
		memcpy((unsigned char *) phys_to_virt(ntohl(ring->writePtr)), ptr, (tmp.beginAddr + tmp.size - tmp.writePtr));
		ptr += (tmp.beginAddr + tmp.size - tmp.writePtr);
		len -= (tmp.beginAddr + tmp.size - tmp.writePtr);
		if (len != 0) {
			memcpy((unsigned char *) phys_to_virt(ntohl(ring->beginAddr)), ptr, len);
			tmp.writePtr = tmp.beginAddr + len;
		} else {
			tmp.writePtr = tmp.beginAddr;
		}
	} else {
		memcpy((unsigned char *) phys_to_virt(ntohl(ring->writePtr)), ptr, len);
        if (alsa_debug)
            rtd_pr_adsp_debug("[ALSA] ntohl(ring->writePtr) = %lx\n", (unsigned long) phys_to_virt(ntohl(ring->writePtr)));
		tmp.writePtr = tmp.writePtr + len;
	}

	ring->writePtr = htonl(tmp.writePtr);
    if (alsa_debug)
        rtd_pr_adsp_debug("[ALSA] %s Exit\n", __func__);
	return len;
}

int RPC_TOAGENT_INBAND_EOS_SVC(struct snd_pcm_substream *substream)
{
	AUDIO_DEC_EOS cmd;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_pcm *dpcm = runtime->private_data;

	cmd.header.type = htonl(AUDIO_DEC_INBAND_CMD_TYPE_EOS);
	cmd.header.size = htonl(sizeof(AUDIO_DEC_EOS));
	cmd.EOSID = 0;
	cmd.wPtr = htonl((ulong) dpcm->ring_bak[0].writePtr);

	hw_ring_write(dpcm->inband, &cmd, sizeof(AUDIO_DEC_EOS));
	return 0;
}

int RPC_TOAGENT_EOS_SVC(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_pcm *dpcm = runtime->private_data;

	struct AUDIO_RPC_SENDPIN_LONG *info, *info_audio;
	RPCRES_LONG *res, *res_audio;
	unsigned long ret;

	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long phy_addr = 0;

	vir_addr = dvr_malloc_uncached_specific(RPC_DVR_MALLOC_UNCACHED_SIZE, GFP_DCU1, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[ALSA] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = (unsigned long) dvr_to_phys(vir_addr);

	/* set parameter to 0, for snapshot */
	memset(unvir_addr, 0, RPC_DVR_MALLOC_UNCACHED_SIZE);

	info = unvir_addr;
	info_audio = (void *)(struct AUDIO_RPC_SENDPIN_LONG *)((ulong) phy_addr);

	res = (void *)(RPCRES_LONG *) ((((unsigned long)unvir_addr + sizeof(struct AUDIO_RPC_SENDPIN_LONG) + 8) & ALIGN4));
	res_audio = (void *)(RPCRES_LONG *) (((((unsigned long) phy_addr) + sizeof(struct AUDIO_RPC_SENDPIN_LONG) + 8) & ALIGN4));

	info->instanceID = htonl(dpcm->ao_agent);
	info->pinID = htonl(dpcm->ao_pin_id);
	info->data = htonl((ulong) dpcm->ring_bak[0].writePtr);

	if (audio_send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_EOS,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		(unsigned long) res, &ret)) {
		rtd_pr_adsp_err("[ALSA] %s %d RPC fail\n", __func__, __LINE__);
		dvr_free(vir_addr);
		return -1;
	}

	dvr_free(vir_addr);
	return 0;
}

static int rtk_snd_init_decoder_info(struct snd_pcm_substream *substream)
{
	AUDIO_DEC_NEW_FORMAT cmd;
	struct AUDIO_RPC_SENDIO sendio;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_pcm *dpcm = runtime->private_data;

	int temp = dpcm->ao_agent|dpcm->ao_pin_id;
	RPC_TOAGENT_PAUSE_SVC(dpcm->card->card, &temp); /* AO pause */

	temp = dpcm->dec_agent;
	RPC_TOAGENT_PAUSE_SVC(dpcm->card->card, &temp); /* Decoder pause */

	sendio.instanceID = dpcm->dec_agent;
	sendio.pinID = dpcm->dec_pin_id;
	RPC_TOAGENT_FLUSH_SVC(dpcm->card->card, &sendio);

	cmd.audioType = htonl(AUDIO_LPCM_DECODER_TYPE);
	cmd.header.type = htonl(AUDIO_DEC_INBAMD_CMD_TYPE_NEW_FORMAT);
	cmd.header.size = htonl(sizeof(AUDIO_DEC_NEW_FORMAT));
	cmd.privateInfo[0] = htonl(runtime->channels);
	cmd.privateInfo[1] = htonl(runtime->sample_bits);
	cmd.privateInfo[2] = htonl(runtime->rate);
	cmd.privateInfo[3] = htonl(0);
	cmd.privateInfo[4] = htonl(0);
	cmd.privateInfo[5] = htonl(0);
	cmd.privateInfo[6] = htonl(0);
	cmd.privateInfo[7] = htonl(0);

	switch (runtime->format) {
	case SNDRV_PCM_FORMAT_S16_BE:
	case SNDRV_PCM_FORMAT_U16_BE:
	case SNDRV_PCM_FORMAT_S24_BE:
	case SNDRV_PCM_FORMAT_U24_BE:
	case SNDRV_PCM_FORMAT_S32_BE:
	case SNDRV_PCM_FORMAT_U32_BE:
	case SNDRV_PCM_FORMAT_S24_3BE:
	case SNDRV_PCM_FORMAT_U24_3BE:
	case SNDRV_PCM_FORMAT_S20_3BE:
	case SNDRV_PCM_FORMAT_U20_3BE:
	case SNDRV_PCM_FORMAT_S18_3BE:
	case SNDRV_PCM_FORMAT_U18_3BE:
	case SNDRV_PCM_FORMAT_FLOAT_BE:
	case SNDRV_PCM_FORMAT_FLOAT64_BE:
	case SNDRV_PCM_FORMAT_IEC958_SUBFRAME_BE:
		cmd.privateInfo[7] = htonl(AUDIO_BIG_ENDIAN);
		break;
	case SNDRV_PCM_FORMAT_S8:
	case SNDRV_PCM_FORMAT_U8:
	case SNDRV_PCM_FORMAT_S16_LE:
	case SNDRV_PCM_FORMAT_U16_LE:
	case SNDRV_PCM_FORMAT_S24_LE:
	case SNDRV_PCM_FORMAT_U24_LE:
	case SNDRV_PCM_FORMAT_S32_LE:
	case SNDRV_PCM_FORMAT_U32_LE:
	case SNDRV_PCM_FORMAT_S24_3LE:
	case SNDRV_PCM_FORMAT_U24_3LE:
	case SNDRV_PCM_FORMAT_S20_3LE:
	case SNDRV_PCM_FORMAT_U20_3LE:
	case SNDRV_PCM_FORMAT_S18_3LE:
	case SNDRV_PCM_FORMAT_U18_3LE:
	case SNDRV_PCM_FORMAT_FLOAT_LE:
	case SNDRV_PCM_FORMAT_FLOAT64_LE:
	case SNDRV_PCM_FORMAT_IEC958_SUBFRAME_LE:
	case SNDRV_PCM_FORMAT_MU_LAW:
	case SNDRV_PCM_FORMAT_A_LAW:
	case SNDRV_PCM_FORMAT_IMA_ADPCM:
	case SNDRV_PCM_FORMAT_MPEG:
	case SNDRV_PCM_FORMAT_GSM:
	case SNDRV_PCM_FORMAT_SPECIAL:
	default:
		cmd.privateInfo[7] = htonl(AUDIO_LITTLE_ENDIAN);
		break;
	}
	cmd.wPtr = dpcm->ring[0].beginAddr;

	hw_ring_write(dpcm->inband, &cmd, sizeof(AUDIO_DEC_NEW_FORMAT));
	return 0;
}

static int rtk_snd_decoder_run(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_pcm *dpcm = runtime->private_data;
	int result = 0;

	int temp = dpcm->ao_agent|dpcm->ao_pin_id;

	if (dpcm->audiopath == AUDIO_PATH_DECODER_AO)
		result = RPC_TOAGENT_RUN_SVC(dpcm->card->card, &dpcm->agent_id);

	if (result < 0)
		return result;

	result = RPC_TOAGENT_RUN_SVC(dpcm->card->card, &temp);

	return result;
}

static void rtk_snd_init_decoder_ring(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_pcm *dpcm = runtime->private_data;
	struct RPC_RBUF_HEADER ringbuf_header;

    	spin_lock(dpcm->pcm_lock);
	dpcm->pcm_buf_pos = 0;
    	spin_unlock(dpcm->pcm_lock);
	/* scprioren add for UNCAC_BASE */
	dpcm->ring = dpcm->hw_ring;
	dpcm->inband = dpcm->hw_inband_ring;

	/* set bitstream ring buffer */
	dpcm->buffer_byte_size = frames_to_bytes(runtime, runtime->buffer_size);

	dpcm->ring[0].beginAddr = htonl((ulong) runtime->dma_addr);
	dpcm->ring[0].bufferID = htonl(RINGBUFFER_STREAM);
	dpcm->ring[0].size = htonl((ulong) dpcm->buffer_byte_size);
	dpcm->ring[0].writePtr = dpcm->ring[0].beginAddr;
	dpcm->ring[0].readPtr[0] = dpcm->ring[0].beginAddr;
	dpcm->ring[0].readPtr[1] = dpcm->ring[0].beginAddr;
	dpcm->ring[0].readPtr[2] = dpcm->ring[0].beginAddr;
	dpcm->ring[0].readPtr[3] = dpcm->ring[0].beginAddr;
	dpcm->ring[0].numOfReadPtr = htonl(1);

	dpcm->ring_bak[0].beginAddr = (unsigned long) runtime->dma_addr;
	dpcm->ring_bak[0].size = dpcm->buffer_byte_size;
	dpcm->ring_bak[0].writePtr = dpcm->ring_bak[0].beginAddr;
	dpcm->ring_bak[0].readPtr[0] = dpcm->ring_bak[0].beginAddr;
	dpcm->ring_bak[0].readPtr[1] = dpcm->ring_bak[0].beginAddr;
	dpcm->ring_bak[0].readPtr[2] = dpcm->ring_bak[0].beginAddr;
	dpcm->ring_bak[0].readPtr[3] = dpcm->ring_bak[0].beginAddr;
	rtd_pr_adsp_debug("[ALSA] ring[0].readPtr[0] = %lx, ring_bak[0].writePtr = %lx\n",
		dpcm->ring[0].readPtr[0], dpcm->ring_bak[0].writePtr);
	rtd_pr_adsp_debug("[ALSA] decoder input ring buffer %lx %lx\n",
		dpcm->ring_bak[0].size, dpcm->ring_bak[0].beginAddr);

	ringbuf_header.instanceID = dpcm->agent_id;
	ringbuf_header.pin_id = BASE_BS_IN;
	ringbuf_header.rbuf_list[0] = (ulong) (&dpcm->ring[0]);
	ringbuf_header.rd_idx = 0;
	ringbuf_header.listsize = 1;

	RPC_TOAGENT_INITRINGBUFFER_HEADER_SVC(dpcm->card->card, &ringbuf_header);

	/* setup inband ring buffer */
	rtd_pr_adsp_debug("[ALSA] hw_inband_data = %lx\n", (unsigned long) dpcm->hw_inband_data);
	dpcm->inband[0].beginAddr = htonl((unsigned long) virt_to_phys(dpcm->hw_inband_data));
	rtd_pr_adsp_debug("[ALSA] inband[0].beginAddr = %x\n", dpcm->inband[0].beginAddr);
	dpcm->inband[0].size = htonl(sizeof(dpcm->hw_inband_data));
	dpcm->inband[0].readPtr[0] = dpcm->inband[0].beginAddr;
	dpcm->inband[0].writePtr = dpcm->inband[0].beginAddr;
	dpcm->inband[0].numOfReadPtr = htonl(1);

	ringbuf_header.instanceID = dpcm->agent_id;
	ringbuf_header.pin_id = INBAND_QUEUE;
	ringbuf_header.rbuf_list[0] = (ulong) (&dpcm->inband[0]);
	ringbuf_header.rd_idx = 0;
	ringbuf_header.listsize = 1;

	RPC_TOAGENT_INITRINGBUFFER_HEADER_SVC(dpcm->card->card, &ringbuf_header);
	rtd_pr_adsp_debug("[ALSA] ring[0].readPtr[0] = %lx\n", dpcm->ring[0].readPtr[0]);
}

static int rtk_snd_init_connect_decoder_ao(struct snd_pcm_substream *substream)
{
	int i;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_pcm *dpcm = runtime->private_data;
	int ring_size;
	struct RPC_RBUF_HEADER ringbuf_header;
	struct AUDIO_RPC_CONNECTION connection;

	dpcm->ao_in_ring = (void *)(RINGBUFFER_HEADER *)
		((unsigned long) dpcm->ao_in_ring_instance);

	rtd_pr_adsp_debug("[ALSA] ao_in_ring = %lx\n", (unsigned long) dpcm->ao_in_ring);

	ring_size = 8 * 1024;
	rtd_pr_adsp_debug("[ALSA] internal ring size per channel : %d bytes\n", ring_size);

	for (i = 0; i < runtime->channels; i++) {
		dpcm->ao_in_ring_p[i] =	dvr_malloc_specific(8 * 1024, GFP_DCU1);
		if (dpcm->ao_in_ring_p[i] == NULL)
			return -ENOMEM;
		rtd_pr_adsp_debug("[ALSA] ao_in_ring_p[%d] = %lx\n", i, (unsigned long) dvr_to_phys(dpcm->ao_in_ring_p[i]));

		dpcm->ao_in_ring[i].beginAddr = htonl((unsigned long) dvr_to_phys(dpcm->ao_in_ring_p[i]));
		dpcm->ao_in_ring[i].size = htonl((ring_size));
		dpcm->ao_in_ring[i].readPtr[0] = dpcm->ao_in_ring[i].beginAddr;
		dpcm->ao_in_ring[i].readPtr[1] = dpcm->ao_in_ring[i].beginAddr;
		dpcm->ao_in_ring[i].readPtr[2] = dpcm->ao_in_ring[i].beginAddr;
		dpcm->ao_in_ring[i].readPtr[3] = dpcm->ao_in_ring[i].beginAddr;
		dpcm->ao_in_ring[i].writePtr = dpcm->ao_in_ring[i].beginAddr;
		dpcm->ao_in_ring[i].numOfReadPtr = htonl(1);
	}

	ringbuf_header.instanceID = dpcm->agent_id;
	ringbuf_header.pin_id = PCM_OUT_RTK;
	ringbuf_header.rbuf_list[0] = (unsigned long) &dpcm->ao_in_ring[0];
	ringbuf_header.rbuf_list[1] = (unsigned long) &dpcm->ao_in_ring[1];
	ringbuf_header.rd_idx = -1;
	ringbuf_header.listsize = runtime->channels;

	RPC_TOAGENT_INITRINGBUFFER_HEADER_SVC(dpcm->card->card, &ringbuf_header);

	ringbuf_header.instanceID = dpcm->ao_agent;
	ringbuf_header.pin_id = dpcm->ao_pin_id;
	ringbuf_header.rd_idx = 0;
	ringbuf_header.rbuf_list[0] = (unsigned long) &dpcm->ao_in_ring[0];
	ringbuf_header.rbuf_list[1] = (unsigned long) &dpcm->ao_in_ring[1];
	ringbuf_header.listsize = runtime->channels;

	RPC_TOAGENT_INITRINGBUFFER_HEADER_SVC(dpcm->card->card, &ringbuf_header);

	connection.desInstanceID = dpcm->ao_agent;
	connection.srcInstanceID = dpcm->agent_id;

	connection.srcPinID = PCM_OUT_RTK;
	connection.desPinID = dpcm->ao_pin_id;
	RPC_TOAGENT_CONNECT_SVC(dpcm->card->card, &connection);

	return 0;
}
#endif

static int rtk_snd_flush(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_pcm *dpcm = runtime->private_data;
	struct AUDIO_RPC_SENDIO sendio;
	int ret;

	rtd_pr_adsp_debug("[ALSA] %s %d\n", __func__, __LINE__);

	sendio.instanceID = dpcm->agent_id;
	sendio.pinID = dpcm->ao_pin_id;

	ret = RPC_TOAGENT_FLUSH_SVC(dpcm->card->card, &sendio);
	/* end of hw buffer flushing */

	dpcm->flush_state = RTK_SND_FLUSH_STATE_FINISH;

	if (ret < 0)
		return -1;

	return 0;
}

static int rtk_snd_pause(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime;
	struct rtk_snd_pcm *dpcm;
	int temp, ret;

	runtime = substream->runtime;
	dpcm = runtime->private_data;

	rtd_pr_adsp_debug("[ALSA] %s %d\n", __func__, __LINE__);

	temp = dpcm->ao_agent|dpcm->ao_pin_id;
	ret = RPC_TOAGENT_PAUSE_SVC(dpcm->card->card, &temp);

	if (ret < 0)
		return -1;

	return 0;
}

static char *snd_access_mode[SNDRV_PCM_ACCESS_LAST+1] = {
	"SND_PCM_ACCESS_MMAP_INTERLEAVED",
	/** mmap access with simple non interleaved channels */
	"SND_PCM_ACCESS_MMAP_NONINTERLEAVED",
	/** mmap access with complex placement */
	"SND_PCM_ACCESS_MMAP_COMPLEX",
	/** snd_pcm_readi/snd_pcm_writei access */
	"SND_PCM_ACCESS_RW_INTERLEAVED",
	/** snd_pcm_readn/snd_pcm_writen access */
	"SND_PCM_ACCESS_RW_NONINTERLEAVED",
};

static int rtk_snd_init_ao_ring(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_pcm *dpcm = runtime->private_data;
	struct rtk_snd_pcm *dpcm_audio = (struct rtk_snd_pcm *) dpcm->phy_addr;

	struct RPC_RBUF_HEADER *info, *info_audio;
	RPCRES_LONG *res, *res_audio;
	unsigned long retval;

	int i, ret = 0;
	unsigned long count, phy_addr_ring;

	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long phy_addr = 0;

	rtd_pr_adsp_debug("[ALSA] %s\n", __func__);
    	spin_lock(dpcm->pcm_lock);
	dpcm->pcm_buf_pos = 0;
    	spin_unlock(dpcm->pcm_lock);
	dpcm->ring = (RINGBUFFER_HEADER *)((unsigned long) dpcm->hw_ring);
	dpcm->ao_in_ring = dpcm->ring;
	dpcm->buffer_byte_size = runtime->buffer_size * frames_to_bytes(runtime, 1);

	rtd_pr_adsp_debug("[ALSA] buffer_byte_size %ld\n", dpcm->buffer_byte_size);

	for (i = 0; i < 1; i++) {
		dpcm->ring[i].beginAddr = htonl((unsigned long) runtime->dma_addr);
		dpcm->ring_bak[i].beginAddr = (unsigned long) runtime->dma_addr;

		dpcm->ring[i].bufferID = htonl(RINGBUFFER_STREAM);
		dpcm->ring[i].size = htonl(dpcm->buffer_byte_size);
		dpcm->ring[i].writePtr = dpcm->ring[i].beginAddr;
		dpcm->ring[i].readPtr[0] = dpcm->ring[i].beginAddr;
		dpcm->ring[i].readPtr[1] = dpcm->ring[i].beginAddr;
		dpcm->ring[i].readPtr[2] = dpcm->ring[i].beginAddr;
		dpcm->ring[i].readPtr[3] = dpcm->ring[i].beginAddr;

		dpcm->ring_bak[i].size = dpcm->buffer_byte_size;
		dpcm->ring_bak[i].writePtr = dpcm->ring_bak[i].beginAddr;
		dpcm->ring_bak[i].readPtr[0] = dpcm->ring_bak[i].beginAddr;
		dpcm->ring_bak[i].readPtr[1] = dpcm->ring_bak[i].beginAddr;
		dpcm->ring_bak[i].readPtr[2] = dpcm->ring_bak[i].beginAddr;
		dpcm->ring_bak[i].readPtr[3] = dpcm->ring_bak[i].beginAddr;
	}

	vir_addr = dvr_malloc_uncached_specific(RPC_DVR_MALLOC_UNCACHED_SIZE, GFP_DCU1, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[ALSA] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = (unsigned long) dvr_to_phys(vir_addr);

	/* set parameter to 0, for snapshot */
	memset(unvir_addr, 0, RPC_DVR_MALLOC_UNCACHED_SIZE);

	info = unvir_addr;
	info_audio = (void *)(struct RPC_RBUF_HEADER *)((ulong) phy_addr);

	res = (void *)(RPCRES_LONG *) ((((unsigned long)unvir_addr + sizeof(struct RPC_RBUF_HEADER) + 8) & ALIGN4));
	res_audio = (void *)(RPCRES_LONG *) (((((unsigned long) phy_addr) + sizeof(struct RPC_RBUF_HEADER) + 8) & ALIGN4));

	info->instanceID = htonl(dpcm->ao_agent);
	info->pin_id = htonl(dpcm->ao_pin_id);
	info->rd_idx = htonl(0);
	info->listsize = htonl(1);

	for (i = 0; i < 1; i++) {
		count = (unsigned long)&dpcm->ring[i] - (unsigned long)dpcm;
		phy_addr_ring = (unsigned long) dpcm_audio + count;
		info->rbuf_list[i] = htonl((unsigned long) phy_addr_ring);
	}

	if (audio_send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_INIT_RINGBUF,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		(unsigned long) res, &retval)) {
		rtd_pr_adsp_err("[ALSA] %s %d RPC fail\n", __func__, __LINE__);
		ret = -1;
		goto exit;
	}

	rtd_pr_adsp_debug("[ALSA] RING INITIALIZED\n");
	rtd_pr_adsp_debug("[ALSA] channel = %d\n", runtime->channels);
	rtd_pr_adsp_debug("[ALSA] frames_to_bytes = %zd\n", frames_to_bytes(runtime, 1));
	rtd_pr_adsp_debug("[ALSA] samples_to_bytes = %zd\n", samples_to_bytes(runtime, 1));
	rtd_pr_adsp_debug("[ALSA] buffer size = %d\n", (int) runtime->buffer_size);
	rtd_pr_adsp_debug("[ALSA] ACCESS MODE=%s", snd_access_mode[runtime->access]);
	for (i = 0; i < runtime->channels; i++) {
		rtd_pr_adsp_debug("[ALSA] buf_header[%d].magic = %x\n",
			i, ntohl(dpcm->ring[i].magic));
		rtd_pr_adsp_debug("[ALSA] snd_rtk_buffer_header[%d].beginAddr = %x\n",
			i, ntohl(dpcm->ring[i].beginAddr));
		rtd_pr_adsp_debug("[ALSA] snd_rtk_buffer_header[%d].size = %x\n",
			i, ntohl(dpcm->ring[i].size));
		rtd_pr_adsp_debug("[ALSA] snd_rtk_buffer_header[%d].readPtr[0] = %x\n",
			i, ntohl(dpcm->ring[i].readPtr[0]));
		rtd_pr_adsp_debug("[ALSA] snd_rtk_buffer_header[%d].readPtr[1] = %x\n",
			i, ntohl(dpcm->ring[i].readPtr[1]));
		rtd_pr_adsp_debug("[ALSA] snd_rtk_buffer_header[%d].writePtr = %x\n",
			i, ntohl(dpcm->ring[i].writePtr));
	}

exit:
	dvr_free(vir_addr);
	return ret;
}


static int rtk_snd_init_ai_ring(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_cap_pcm *dpcm = runtime->private_data;
	struct rtk_snd_pcm *dpcm_audio = (struct rtk_snd_pcm *) dpcm->phy_addr;

	struct RPC_RBUF_HEADER *info, *info_audio;
	RPCRES_LONG *res, *res_audio;
	unsigned long retval;
	int create_up_ring_ret = 0;

	int ret = 0;
	int i, ch;
	bool voice_enable = (substream->pcm->device == 2) ? TRUE : FALSE;
	unsigned long count, phy_addr_ring;

	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long phy_addr = 0;

	i = 0;
	if (is_far_field(substream->pcm->device)) {
		ch = 1;
	} else {
		ch = runtime->channels;
	}

	/* frames per buffer * sample_bytes */
	dpcm->buffer_byte_size = runtime->buffer_size * dpcm->input_sample_bytes;
	dpcm->ring_p = dvr_malloc_specific(ch * dpcm->buffer_byte_size, GFP_DCU1);

	if ((void*) dpcm->ring_p == NULL) {
		rtd_pr_adsp_err("[ALSA] %d error\n",__LINE__);
		return -ENOMEM;
	}
	dpcm->ring_phy_addr = dvr_to_phys(dpcm->ring_p);

	if (is_far_field(substream->pcm->device)) {
		if (dpcm->card->far_field_ring_audio == NULL) {
			void *far_field_unvir_addr = NULL;

			dpcm->card->far_field_ring_audio_cached = (RINGBUFFER_HEADER *) dvr_malloc_uncached_specific(RPC_DVR_MALLOC_UNCACHED_SIZE, GFP_DCU1, &far_field_unvir_addr);

			if (!dpcm->card->far_field_ring_audio_cached) {
				rtd_pr_adsp_err("[ALSA] %d alloc memory fail\n", __LINE__);
				return -ENOMEM;
			}

			dpcm->card->far_field_ring_audio = (RINGBUFFER_HEADER *) far_field_unvir_addr;
			dpcm->card->far_field_ring_audio_phy = dvr_to_phys(dpcm->card->far_field_ring_audio_cached);
		}

		dpcm->ring = (RINGBUFFER_HEADER *)(dpcm->card->far_field_ring_audio);

		/* uncache address for Audio FW */
		dpcm->card->far_field_ring_audio->beginAddr = htonl((unsigned long) (dpcm->ring_phy_addr + i * dpcm->buffer_byte_size));

		dpcm->card->far_field_ring_audio->bufferID = htonl(RINGBUFFER_STREAM);
		dpcm->card->far_field_ring_audio->size = htonl((ulong) dpcm->buffer_byte_size);
		dpcm->card->far_field_ring_audio->numOfReadPtr = htonl(1);
		dpcm->card->far_field_ring_audio->writePtr = dpcm->card->far_field_ring_audio->beginAddr;
		dpcm->card->far_field_ring_audio->readPtr[0] = dpcm->card->far_field_ring_audio->beginAddr;
		dpcm->card->far_field_ring_audio->readPtr[1] = dpcm->card->far_field_ring_audio->beginAddr;
		dpcm->card->far_field_ring_audio->readPtr[2] = dpcm->card->far_field_ring_audio->beginAddr;
		dpcm->card->far_field_ring_audio->readPtr[3] = dpcm->card->far_field_ring_audio->beginAddr;

		dpcm->card->far_field_vir = dpcm->ring_p;
		dpcm->rd_idx = dpcm->card->reference_count;
	} else {
		dpcm->ring = (RINGBUFFER_HEADER *)((unsigned long) dpcm->hw_ring);

		for (i = 0; i < ch; i++) {
			/* uncache address for Audio FW */
			dpcm->ring[i].beginAddr = htonl((unsigned long) (dpcm->ring_phy_addr + i * dpcm->buffer_byte_size));

			dpcm->ring[i].bufferID = htonl(RINGBUFFER_STREAM);
			dpcm->ring[i].size = htonl((ulong) dpcm->buffer_byte_size);
			dpcm->ring[i].numOfReadPtr = htonl(1);
			dpcm->ring[i].writePtr = dpcm->ring[i].beginAddr;
			dpcm->ring[i].readPtr[0] = dpcm->ring[i].beginAddr;
			dpcm->ring[i].readPtr[1] = dpcm->ring[i].beginAddr;
			dpcm->ring[i].readPtr[2] = dpcm->ring[i].beginAddr;
			dpcm->ring[i].readPtr[3] = dpcm->ring[i].beginAddr;
		}

		dpcm->rd_idx = 0;
	}

	if (voice_enable) {
		create_up_ring_ret = create_voice_ring(dpcm->card->card->dev);
		if(create_up_ring_ret != 0) {
			rtd_pr_adsp_err("[ALSA] create up ring fail\n");
			return -EINVAL;
		}
		info_up_down_ring((unsigned long)dpcm->hw_ring, (unsigned long) airing->hw_ring2);
	}

	vir_addr = dvr_malloc_uncached_specific(RPC_DVR_MALLOC_UNCACHED_SIZE, GFP_DCU1, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[ALSA] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = (unsigned long) dvr_to_phys(vir_addr);

	/* set parameter to 0, for snapshot */
	memset(unvir_addr, 0, RPC_DVR_MALLOC_UNCACHED_SIZE);

	info = unvir_addr;
	info_audio = (void *)(struct RPC_RBUF_HEADER *)((ulong) phy_addr);

	res = (void *)(RPCRES_LONG *) ((((unsigned long)unvir_addr + sizeof(struct RPC_RBUF_HEADER) + 8) & ALIGN4));
	res_audio = (void *)(RPCRES_LONG *) (((((unsigned long) phy_addr) + sizeof(struct RPC_RBUF_HEADER) + 8) & ALIGN4));

	info->instanceID = htonl(dpcm->ai_cap_agent);
	info->pin_id = htonl(dpcm->ai_cap_pin);
	info->rd_idx = htonl(-1);
	if (voice_enable || is_far_field(substream->pcm->device)) {
		info->listsize = htonl(1);
	} else {
		info->listsize = htonl(runtime->channels);
	}

	if (voice_enable) {
		for (i = 0; i < 1; i++) {
			info->rbuf_list[i] = htonl((unsigned long) virt_to_phys(&airing->ring2[i]));
		}
	} else if (is_far_field(substream->pcm->device)) {
		info->rbuf_list[0] = htonl((unsigned long) dpcm->card->far_field_ring_audio_phy);
	} else {
		for (i = 0; i < runtime->channels; i++) {
			count = (unsigned long)&dpcm->ring[i] - (unsigned long) dpcm;
			phy_addr_ring = (unsigned long) dpcm_audio + count;
			info->rbuf_list[i] = htonl((unsigned long) phy_addr_ring);
		}
	}

	if (voice_enable && check_voice_ready())
		goto exit;

	if (audio_send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_INIT_RINGBUF,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		(unsigned long) res, &retval)) {
		rtd_pr_adsp_err("[ALSA] %s %d RPC fail\n", __func__, __LINE__);
		ret = -1;
		goto exit;
	}

	rtd_pr_adsp_debug("[ALSA] ALSA RING INITIALIZED\n");
	rtd_pr_adsp_debug("[ALSA] channel = %d\n", ch);
	rtd_pr_adsp_debug("[ALSA] frames_to_bytes = %zd\n", frames_to_bytes(runtime, 1));
	rtd_pr_adsp_debug("[ALSA] samples_to_bytes = %zd\n", samples_to_bytes(runtime, 1));
	rtd_pr_adsp_debug("[ALSA] buffer size = %d\n", (int) runtime->buffer_size);
	rtd_pr_adsp_debug("[ALSA] ACCESS MODE=%s", snd_access_mode[runtime->access]);
	for (i = 0; i < ch; i++) {
		rtd_pr_adsp_debug("[ALSA] snd_rtk_buffer_header[%d].beginAddr = %x\n",
			i, ntohl(dpcm->ring[i].beginAddr));
		rtd_pr_adsp_debug("[ALSA] snd_rtk_buffer_header[%d].size = %x\n",
			i, ntohl(dpcm->ring[i].size));
		rtd_pr_adsp_debug("[ALSA] snd_rtk_buffer_header[%d].readPtr[0] = %x\n",
			i, ntohl(dpcm->ring[i].readPtr[0]));
		rtd_pr_adsp_debug("[ALSA] snd_rtk_buffer_header[%d].writePtr = %x\n",
			i, ntohl(dpcm->ring[i].writePtr));
	}

exit:
	dvr_free(vir_addr);
	return ret;
}

/* init AO capture ring */
static int rtk_snd_init_ao_capture_ring(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_cap_pcm *dpcm = runtime->private_data;
	struct rtk_snd_pcm *dpcm_audio = (struct rtk_snd_pcm *) dpcm->phy_addr;

	struct RPC_RBUF_HEADER *info, *info_audio;
	RPCRES_LONG *res, *res_audio;
	unsigned long retval;

	int ret = 0;
	int i;
	unsigned long count, phy_addr_ring;

	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long phy_addr = 0;

	/* allocate ring buffer */
	dpcm->ring = (RINGBUFFER_HEADER *)((unsigned long) dpcm->hw_ring);
	dpcm->buffer_byte_size = runtime->buffer_size * dpcm->input_sample_bytes;
	dpcm->ring_p = dvr_malloc_specific(runtime->channels * dpcm->buffer_byte_size, GFP_DCU1);
    if ((void*) dpcm->ring_p == NULL) {
        rtd_pr_adsp_err("[ALSA] %d\n",__LINE__);
        return -ENOMEM;
    }
    dpcm->ring_phy_addr = dvr_to_phys(dpcm->ring_p);

	for (i = 0; i < runtime->channels; i++) {
		dpcm->ring[i].beginAddr = htonl((unsigned long) (dpcm->ring_phy_addr + i * dpcm->buffer_byte_size));

		dpcm->ring[i].bufferID = htonl(RINGBUFFER_STREAM);
		dpcm->ring[i].size = htonl((ulong) dpcm->buffer_byte_size);
		dpcm->ring[i].numOfReadPtr = htonl(1);
		dpcm->ring[i].writePtr = dpcm->ring[i].beginAddr;
		dpcm->ring[i].readPtr[0] = dpcm->ring[i].beginAddr;
		dpcm->ring[i].readPtr[1] = dpcm->ring[i].beginAddr;
		dpcm->ring[i].readPtr[2] = dpcm->ring[i].beginAddr;
		dpcm->ring[i].readPtr[3] = dpcm->ring[i].beginAddr;
	}

	vir_addr = dvr_malloc_uncached_specific(RPC_DVR_MALLOC_UNCACHED_SIZE, GFP_DCU1, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[ALSA] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = (unsigned long) dvr_to_phys(vir_addr);

	/* set parameter to 0, for snapshot */
	memset(unvir_addr, 0, RPC_DVR_MALLOC_UNCACHED_SIZE);

	info = unvir_addr;
	info_audio = (void *)(struct RPC_RBUF_HEADER *)((ulong) phy_addr);

	res = (void *)(RPCRES_LONG *)
		((((unsigned long)unvir_addr + sizeof(struct RPC_RBUF_HEADER) + 8) & ALIGN4));
	res_audio = (void *)(RPCRES_LONG *) (((((unsigned long) phy_addr) + sizeof(struct RPC_RBUF_HEADER) + 8) & ALIGN4));

	info->instanceID = htonl(dpcm->ao_cap_agent);
	info->pin_id = htonl(dpcm->ao_cap_pin);
	info->rd_idx = htonl(-1);
	info->listsize = htonl(runtime->channels);

	for (i = 0; i < runtime->channels; i++) {
		count = (unsigned long)&dpcm->ring[i] - (unsigned long)dpcm;
		phy_addr_ring = (unsigned long) dpcm_audio + count;
		info->rbuf_list[i] = htonl((unsigned long) phy_addr_ring);
	}

	if (audio_send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_INIT_RINGBUF,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		(unsigned long) res, &retval)) {
		rtd_pr_adsp_err("[ALSA] %s %d RPC fail\n", __func__, __LINE__);
		ret = -1;
		goto exit;
	}

#if 0
	rtd_pr_adsp_debug("[ALSA] ALSA RING INITIALIZED\n");
	rtd_pr_adsp_debug("[ALSA] channel = %d\n", runtime->channels);
	rtd_pr_adsp_debug("[ALSA] frames_to_bytes = %d\n", frames_to_bytes(runtime, 1));
	rtd_pr_adsp_debug("[ALSA] samples_to_bytes = %d\n", samples_to_bytes(runtime, 1));
	rtd_pr_adsp_debug("[ALSA] buffer size = %d\n", (int) runtime->buffer_size);
	rtd_pr_adsp_debug("[ALSA] ACCESS MODE=%s", snd_access_mode[runtime->access]);
	for (i = 0; i < runtime->channels; i++) {
		rtd_pr_adsp_debug("[ALSA] snd_rtk_buffer_header[%d].beginAddr = %x\n",
			i, ntohl(dpcm->ring[i].beginAddr));
		rtd_pr_adsp_debug("[ALSA] snd_rtk_buffer_header[%d].size = %x\n",
			i, ntohl(dpcm->ring[i].size));
		rtd_pr_adsp_debug("[ALSA] snd_rtk_buffer_header[%d].readPtr[0] = %x\n",
			i, ntohl(dpcm->ring[i].readPtr[0]));
		rtd_pr_adsp_debug("[ALSA] snd_rtk_buffer_header[%d].writePtr = %x\n",
			i, ntohl(dpcm->ring[i].writePtr));
	}
#endif

exit:
	dvr_free(vir_addr);
	return ret;
}

static void rtk_snd_resume(struct snd_pcm_substream *substream)
{
	unsigned int temp;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_pcm *dpcm = runtime->private_data;

	rtd_pr_adsp_debug("[ALSA] %s\n", __func__);

	if (dpcm->audiopath == AUDIO_PATH_DECODER_AO) {
		rtk_snd_decoder_run(substream);
	} else {
		rtd_pr_adsp_debug("[ALSA] Flash Pin =%d\n", dpcm->ao_pin_id);
		temp = (dpcm->ao_agent | dpcm->ao_pin_id);
		RPC_TOAGENT_RUN_SVC(dpcm->card->card, &temp);
	}
}

static int rtk_snd_set_volume(struct snd_card_mars *mars)
{
	unsigned long ret;
	AUDIO_RPC_PRIVATEINFO_PARAMETERS *cmd, *cmd_audio;
	AUDIO_RPC_PRIVATEINFO_RETURNVAL *res, *res_audio;
	int instanceID, pin_id, i;

	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long phy_addr = 0;

	vir_addr = dvr_malloc_uncached_specific(RPC_DVR_MALLOC_UNCACHED_SIZE, GFP_DCU1, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[ALSA] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = (unsigned long) dvr_to_phys(vir_addr);

	/* set parameter to 0, for snapshot */
	memset(unvir_addr, 0, RPC_DVR_MALLOC_UNCACHED_SIZE);

	cmd = unvir_addr;
	res = (void *)(AUDIO_RPC_PRIVATEINFO_RETURNVAL *) ((((unsigned long)unvir_addr + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS) + 8) & ALIGN4));

	cmd_audio = (void *)(AUDIO_RPC_PRIVATEINFO_PARAMETERS *)((ulong) phy_addr);
	res_audio = (void *)(AUDIO_RPC_PRIVATEINFO_RETURNVAL *) (((((ulong) phy_addr) + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS)+8) & ALIGN4));

	for (i = 0; i < MIXER_ADDR_MAX; i++) {
		if ((mars->ao_pin_id[i] != 0) && (alsa_agent != -1) &&
			(mars->ao_flash_change[i] != 0)) {

			memset(cmd, 0, sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS));
			instanceID = alsa_agent;
			cmd->instanceID = htonl(instanceID);
			cmd->type = htonl(ENUM_PRIVATEINFO_AUDIO_CONTROL_FLASH_VOLUME);
			pin_id = mars->ao_pin_id[i];
			cmd->privateInfo[0] = htonl(pin_id);
			cmd->privateInfo[1] = htonl(mars->ao_flash_volume[i]);

            if (alsa_debug)
                rtd_pr_adsp_debug("[ALSA] volume = %d, pin_id = %d\n", mars->ao_flash_volume[i], pin_id);

			if (audio_send_rpc_command(RPC_AUDIO,
				ENUM_KERNEL_RPC_PRIVATEINFO,
				(unsigned long) cmd_audio,
				(unsigned long) res_audio,
				(unsigned long) res, &ret)) {
				rtd_pr_adsp_err("[ALSA] %s %d RPC fail\n", __func__, __LINE__);
				dvr_free(vir_addr);
				return -1;
			}

			mars->ao_flash_change[i] = 0;
		} else {
            if (alsa_debug)
                rtd_pr_adsp_debug("[ALSA] no running AO flash pin_id, don't set volume\n");
		}
	}

	dvr_free(vir_addr);
	return 0;
}

static void rtk_snd_playback_volume_work(struct work_struct *work)
{
	struct snd_card_mars *mars = container_of(work,
		struct snd_card_mars, work_volume);
	rtk_snd_set_volume(mars);
}

static int rtk_snd_set_ao_flashpin_volume(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_pcm *dpcm = runtime->private_data;
	AUDIO_RPC_PRIVATEINFO_PARAMETERS *cmd , *cmd_audio;
	AUDIO_RPC_PRIVATEINFO_RETURNVAL *res, *res_audio;
	int instanceID, pin_id;
	unsigned long ret;

	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long phy_addr = 0;

	vir_addr = dvr_malloc_uncached_specific(RPC_DVR_MALLOC_UNCACHED_SIZE, GFP_DCU1, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[ALSA] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = (unsigned long) dvr_to_phys(vir_addr);

	/* set parameter to 0, for snapshot */
	memset(unvir_addr, 0, RPC_DVR_MALLOC_UNCACHED_SIZE);

	cmd = unvir_addr;
	res = (void *)(AUDIO_RPC_PRIVATEINFO_RETURNVAL *) ((((unsigned long)unvir_addr + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS) + 8) & ALIGN4));

	cmd_audio = (void *)(AUDIO_RPC_PRIVATEINFO_PARAMETERS *)((ulong) phy_addr);
	res_audio = (void *)(AUDIO_RPC_PRIVATEINFO_RETURNVAL *) (((((ulong) phy_addr) + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS) + 8) & ALIGN4));

	memset(cmd, 0, sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS));
	instanceID = alsa_agent;
	cmd->instanceID = htonl(instanceID);
	cmd->type = htonl(ENUM_PRIVATEINFO_AUDIO_CONTROL_FLASH_VOLUME);
	pin_id = dpcm->ao_pin_id;
	cmd->privateInfo[0] = htonl(pin_id);
	cmd->privateInfo[1] = htonl(dpcm->volume);

    if (alsa_debug)
        rtd_pr_adsp_debug("[ALSA] dpcm->volume= %d\n", dpcm->volume);
	dpcm->volume_change = 0;

	if (audio_send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_PRIVATEINFO,
		(unsigned long) cmd_audio,
		(unsigned long) res_audio,
		(unsigned long) res, &ret)) {
		rtd_pr_adsp_err("[ALSA] %s %d RPC fail\n", __func__, __LINE__);
		dvr_free(vir_addr);
		return -1;
	}

	dvr_free(vir_addr);
	return pin_id;
}

#ifdef USE_FIXED_AO_PINID
static int rtk_snd_get_used_pin_id(struct snd_card *card)
{
	unsigned long ret;
	AUDIO_RPC_PRIVATEINFO_PARAMETERS *cmd, *cmd_audio;
	AUDIO_RPC_PRIVATEINFO_RETURNVAL *res, *res_audio;
	int instanceID, pin_map;

	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long phy_addr = 0;

	vir_addr = dvr_malloc_uncached_specific(RPC_DVR_MALLOC_UNCACHED_SIZE, GFP_DCU1, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[ALSA] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = (unsigned long) dvr_to_phys(vir_addr);

	/* set parameter to 0, for snapshot */
	memset(unvir_addr, 0, RPC_DVR_MALLOC_UNCACHED_SIZE);

	cmd = unvir_addr;
	res = (void *)(AUDIO_RPC_PRIVATEINFO_RETURNVAL *) ((((unsigned long)unvir_addr + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS) + 8) & ALIGN4));

	cmd_audio = (void *)(AUDIO_RPC_PRIVATEINFO_PARAMETERS *)((ulong) phy_addr);
	res_audio = (void *)(AUDIO_RPC_PRIVATEINFO_RETURNVAL *) (((((ulong) phy_addr) + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS) + 8) & ALIGN4));

	memset(cmd, 0, sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS));
	instanceID = alsa_agent;
	cmd->instanceID = htonl(instanceID);
	cmd->type = htonl(ENUM_PRIVATEINFO_AUDIO_QUERY_FLASH_PIN);

	if (audio_send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_PRIVATEINFO,
		(unsigned long)cmd_audio,
		(unsigned long) res_audio,
		(unsigned long) res, &ret)) {
		rtd_pr_adsp_err("[ALSA] %s %d RPC fail\n", __func__, __LINE__);
		dvr_free(vir_addr);
		return -1;
	}

	pin_map = ntohl(res->privateInfo[0]);

	dvr_free(vir_addr);
	return pin_map;
}

static int rtk_snd_set_flashpin_id(struct snd_card *card, int pin)
{
	unsigned long ret;
	AUDIO_RPC_PRIVATEINFO_PARAMETERS *cmd, *cmd_audio;
	AUDIO_RPC_PRIVATEINFO_RETURNVAL *res, *res_audio;
	int instanceID;

	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long phy_addr = 0;

	vir_addr = dvr_malloc_uncached_specific(RPC_DVR_MALLOC_UNCACHED_SIZE, GFP_DCU1, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[ALSA] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = (unsigned long) dvr_to_phys(vir_addr);

	/* set parameter to 0, for snapshot */
	memset(unvir_addr, 0, RPC_DVR_MALLOC_UNCACHED_SIZE);

	cmd = unvir_addr;
	res = (void *)(AUDIO_RPC_PRIVATEINFO_RETURNVAL *)((((unsigned long)unvir_addr + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS) + 8) & ALIGN4));

	cmd_audio = (void *)(AUDIO_RPC_PRIVATEINFO_PARAMETERS *) ((ulong) phy_addr);
	res_audio = (void *)(AUDIO_RPC_PRIVATEINFO_RETURNVAL *) (((((ulong) phy_addr) + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS)+8) & ALIGN4));

	memset(cmd, 0, sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS));
	instanceID = alsa_agent;
	cmd->instanceID = htonl(instanceID);
	cmd->type = htonl(ENUM_PRIVATEINFO_AUDIO_SET_FLASH_PIN);
	cmd->privateInfo[0] = htonl(pin);

	if (audio_send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_PRIVATEINFO,
		(unsigned long) cmd_audio,
		(unsigned long) res_audio,
		(unsigned long) res, &ret)) {
		rtd_pr_adsp_err("[ALSA] %s %d RPC fail\n", __func__, __LINE__);
		dvr_free(vir_addr);
		return -1;
	}

	dvr_free(vir_addr);
	return 0;
}
#else
static int rtk_snd_query_flashpin_id(struct snd_card *card)
{
	unsigned long ret;
	AUDIO_RPC_PRIVATEINFO_PARAMETERS *cmd, *cmd_audio;
	AUDIO_RPC_PRIVATEINFO_RETURNVAL *res, *res_audio;
	int instanceID, pin_id;

	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long phy_addr = 0;

	vir_addr = dvr_malloc_uncached_specific(RPC_DVR_MALLOC_UNCACHED_SIZE, GFP_DCU1, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[ALSA] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = (unsigned long) dvr_to_phys(vir_addr);

	/* set parameter to 0, for snapshot */
	memset(unvir_addr, 0, RPC_DVR_MALLOC_UNCACHED_SIZE);

	cmd = unvir_addr;
	res = (void *)(AUDIO_RPC_PRIVATEINFO_RETURNVAL *) ((((unsigned long)unvir_addr + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS) + 8) & ALIGN4));

	cmd_audio = (void *)(AUDIO_RPC_PRIVATEINFO_PARAMETERS *)((ulong) phy_addr);
	res_audio = (void *)(AUDIO_RPC_PRIVATEINFO_RETURNVAL *) ((((ulong)(phy_addr) + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS)+8) & ALIGN4));

	memset(cmd, 0, sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS));
	instanceID = alsa_agent;
	cmd->instanceID = htonl(instanceID);
	cmd->type = htonl(ENUM_PRIVATEINFO_AUDIO_GET_FLASH_PIN);
	cmd->privateInfo[0] = 0xFF;
	cmd->privateInfo[1] = 0xFF;
	cmd->privateInfo[2] = 0xFF;
	cmd->privateInfo[3] = 0xFF;
	cmd->privateInfo[4] = 0xFF;
	cmd->privateInfo[5] = 0xFF;

	if (audio_send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_PRIVATEINFO,
		(unsigned long)cmd_audio,
		(unsigned long) res_audio,
		(unsigned long) res, &ret)) {
		rtd_pr_adsp_err("[ALSA] %s %d RPC fail\n", __func__, __LINE__);
		dvr_free(vir_addr);
		return -1;
	}

	pin_id = ntohl(cmd->privateInfo[0]);

	dvr_free(vir_addr);

	if (pin_id < FLASH_AUDIO_PIN_1 || pin_id > FLASH_AUDIO_PIN_8)
		return -1;

	return pin_id;
}
#endif

static int rtk_snd_release_flashpin_id(struct snd_card *card, int pin_id)
{
	unsigned long ret;
	AUDIO_RPC_PRIVATEINFO_PARAMETERS *cmd , *cmd_audio;
	AUDIO_RPC_PRIVATEINFO_RETURNVAL *res, *res_audio;
	int instanceID;

	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long phy_addr = 0;

	if (pin_id < FLASH_AUDIO_PIN_1 || pin_id > FLASH_AUDIO_PIN_8) {
		return -1;
	}

	vir_addr = dvr_malloc_uncached_specific(RPC_DVR_MALLOC_UNCACHED_SIZE, GFP_DCU1, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[ALSA] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = (unsigned long) dvr_to_phys(vir_addr);

	/* set parameter to 0, for snapshot */
	memset(unvir_addr, 0, RPC_DVR_MALLOC_UNCACHED_SIZE);

	cmd = unvir_addr;
	res = (void *)(AUDIO_RPC_PRIVATEINFO_RETURNVAL *) ((((unsigned long)unvir_addr + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS) + 8) & ALIGN4));

	cmd_audio = (void *)(AUDIO_RPC_PRIVATEINFO_PARAMETERS *) ((ulong) phy_addr);
	res_audio = (void *)(AUDIO_RPC_PRIVATEINFO_RETURNVAL *) (((((ulong) phy_addr) + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS) + 8) & ALIGN4));

	memset(cmd, 0, sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS));
	instanceID = alsa_agent;
	cmd->instanceID = htonl(instanceID);
	cmd->type = htonl(ENUM_PRIVATEINFO_AUDIO_RELEASE_FLASH_PIN);
	cmd->privateInfo[0] = htonl(pin_id);
	cmd->privateInfo[1] = 0xFF;
	cmd->privateInfo[2] = 0xFF;
	cmd->privateInfo[3] = 0xFF;
	cmd->privateInfo[4] = 0xFF;
	cmd->privateInfo[5] = 0xFF;

	if (audio_send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_PRIVATEINFO,
		(unsigned long) cmd_audio,
		(unsigned long) res_audio,
		(unsigned long) res, &ret)) {
		rtd_pr_adsp_err("[ALSA] %s %d RPC fail\n", __func__, __LINE__);
		dvr_free(vir_addr);
		return -1;
	}

	dvr_free(vir_addr);
	return 0;
}

static int rtk_snd_ao_info(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_pcm *dpcm = runtime->private_data;
	unsigned long ret;
	AUDIO_RPC_PRIVATEINFO_PARAMETERS *cmd, *cmd_audio;
	AUDIO_RPC_PRIVATEINFO_RETURNVAL *res, *res_audio;
	int instanceID;

	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long phy_addr = 0;

	int temp = dpcm->ao_agent | dpcm->ao_pin_id;
	RPC_TOAGENT_PAUSE_SVC(dpcm->card->card, &temp); /* AO pause */

	vir_addr = dvr_malloc_uncached_specific(RPC_DVR_MALLOC_UNCACHED_SIZE, GFP_DCU1, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[ALSA] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = (unsigned long) dvr_to_phys(vir_addr);

	/* set parameter to 0, for snapshot */
	memset(unvir_addr, 0, RPC_DVR_MALLOC_UNCACHED_SIZE);

	cmd = unvir_addr;
	res = (void *)(AUDIO_RPC_PRIVATEINFO_RETURNVAL *) ((((unsigned long)unvir_addr + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS) + 8) & ALIGN4));

	cmd_audio = (void *)(AUDIO_RPC_PRIVATEINFO_PARAMETERS *) ((ulong) phy_addr);
	res_audio = (void *)(AUDIO_RPC_PRIVATEINFO_RETURNVAL *) (((((ulong) phy_addr) + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS) + 8) & ALIGN4));

	memset(cmd, 0, sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS));
	instanceID = dpcm->ao_agent | dpcm->ao_pin_id;
	cmd->instanceID = htonl(instanceID);
	cmd->type = htonl(ENUM_PRIVATEINFO_AUDIO_CONFIG_CMD_BS_INFO);
	cmd->privateInfo[0] = htonl(runtime->channels); /* channel number */

	rtd_pr_adsp_debug("[ALSA] format %d\n", runtime->format);
	rtd_pr_adsp_debug("[ALSA] channels %d\n", runtime->channels);

	if ((dpcm->audiopath == AUDIO_PATH_AO_BYPASS) ||
		(dpcm->audiopath == AUDIO_PATH_AO)) {
		/*
		0	32 bit big endian
		1	24 bit big endian
		2	16 bit big endian
		3	24 bit little endian
		4	16 bit little endian
		5	8 bit
		*/
		switch (runtime->format) {
		case SNDRV_PCM_FORMAT_S32_BE:
			cmd->privateInfo[1] = htonl(0);
			break;
		case SNDRV_PCM_FORMAT_S24_BE:
			cmd->privateInfo[1] = htonl(1);
			break;
		case SNDRV_PCM_FORMAT_S24_LE:
			cmd->privateInfo[1] = htonl(3);
			break;
		case SNDRV_PCM_FORMAT_S16_BE:
		case SNDRV_PCM_FORMAT_S16_LE:
			cmd->privateInfo[1] = htonl(0x84);
			break;
		case SNDRV_PCM_FORMAT_S8:
			cmd->privateInfo[1] = htonl(5);
			break;
		default:
			cmd->privateInfo[1] = htonl(0);
			break;
		}
	} else {
		cmd->privateInfo[1] = htonl(0);
	}

	rtd_pr_adsp_debug("[ALSA] privateInfo[1] %d\n", htonl(cmd->privateInfo[1]));
	rtd_pr_adsp_debug("[ALSA] rate %d\n", runtime->rate);

	cmd->privateInfo[2] = htonl(runtime->rate);
	cmd->privateInfo[4] = htonl(dpcm->ao_pin_id);

	/* channel mapping,
	ALSA driver only supports case 1 and case 2 now */
	switch (runtime->channels) {
	case 1:
		cmd->privateInfo[5] = htonl(AUDIO_LEFT_FRONT_INDEX);
		cmd->privateInfo[6] = htonl(0);
		cmd->privateInfo[7] = htonl(0);
		cmd->privateInfo[8] = htonl(0);
		cmd->privateInfo[9] = htonl(0);
		cmd->privateInfo[10] = htonl(0);
		cmd->privateInfo[11] = htonl(0);
		cmd->privateInfo[12] = htonl(0);
		break;
	case 2:
		cmd->privateInfo[5] = htonl(AUDIO_LEFT_FRONT_INDEX);
		cmd->privateInfo[6] = htonl(AUDIO_RIGHT_FRONT_INDEX);
		cmd->privateInfo[7] = htonl(0);
		cmd->privateInfo[8] = htonl(0);
		cmd->privateInfo[9] = htonl(0);
		cmd->privateInfo[10] = htonl(0);
		cmd->privateInfo[11] = htonl(0);
		cmd->privateInfo[12] = htonl(0);
		break;
	case 3:
		cmd->privateInfo[5] = htonl(AUDIO_LEFT_FRONT_INDEX);
		cmd->privateInfo[6] = htonl(AUDIO_RIGHT_FRONT_INDEX);
		cmd->privateInfo[7] = htonl(AUDIO_LFE_INDEX);
		cmd->privateInfo[8] = htonl(0);
		cmd->privateInfo[9] = htonl(0);
		cmd->privateInfo[10] = htonl(0);
		cmd->privateInfo[11] = htonl(0);
		cmd->privateInfo[12] = htonl(0);
		break;
	case 4:
		cmd->privateInfo[5] = htonl(AUDIO_LEFT_FRONT_INDEX);
		cmd->privateInfo[6] = htonl(AUDIO_RIGHT_FRONT_INDEX);
		cmd->privateInfo[7] = htonl(AUDIO_LEFT_SURROUND_REAR_INDEX);
		cmd->privateInfo[8] = htonl(AUDIO_RIGHT_SURROUND_REAR_INDEX);
		cmd->privateInfo[9] = htonl(0);
		cmd->privateInfo[10] = htonl(0);
		cmd->privateInfo[11] = htonl(0);
		cmd->privateInfo[12] = htonl(0);
		break;
	case 5:
		cmd->privateInfo[5] = htonl(AUDIO_LEFT_FRONT_INDEX);
		cmd->privateInfo[6] = htonl(AUDIO_RIGHT_FRONT_INDEX);
		cmd->privateInfo[7] = htonl(AUDIO_LEFT_SURROUND_REAR_INDEX);
		cmd->privateInfo[8] = htonl(AUDIO_RIGHT_SURROUND_REAR_INDEX);
		cmd->privateInfo[9] = htonl(AUDIO_CENTER_FRONT_INDEX);
		cmd->privateInfo[10] = htonl(0);
		cmd->privateInfo[11] = htonl(0);
		cmd->privateInfo[12] = htonl(0);
		break;
	case 6:
		cmd->privateInfo[5] = htonl(AUDIO_LEFT_FRONT_INDEX);
		cmd->privateInfo[6] = htonl(AUDIO_RIGHT_FRONT_INDEX);
		cmd->privateInfo[7] = htonl(AUDIO_LEFT_SURROUND_REAR_INDEX);
		cmd->privateInfo[8] = htonl(AUDIO_RIGHT_SURROUND_REAR_INDEX);
		cmd->privateInfo[9] = htonl(AUDIO_CENTER_FRONT_INDEX);
		cmd->privateInfo[10] = htonl(AUDIO_LFE_INDEX);
		cmd->privateInfo[11] = htonl(0);
		cmd->privateInfo[12] = htonl(0);
		break;
	case 7:
		cmd->privateInfo[5] = htonl(AUDIO_LEFT_FRONT_INDEX);
		cmd->privateInfo[6] = htonl(AUDIO_RIGHT_FRONT_INDEX);
		cmd->privateInfo[7] = htonl(AUDIO_LEFT_SURROUND_REAR_INDEX);
		cmd->privateInfo[8] = htonl(AUDIO_RIGHT_SURROUND_REAR_INDEX);
		cmd->privateInfo[9] = htonl(AUDIO_CENTER_FRONT_INDEX);
		cmd->privateInfo[10] = htonl(AUDIO_LFE_INDEX);
		cmd->privateInfo[11] = htonl(AUDIO_CENTER_SURROUND_REAR_INDEX);
		cmd->privateInfo[12] = htonl(0);
		break;
	case 8:
		cmd->privateInfo[5] = htonl(AUDIO_LEFT_FRONT_INDEX);
		cmd->privateInfo[6] = htonl(AUDIO_RIGHT_FRONT_INDEX);
		cmd->privateInfo[7] = htonl(AUDIO_LEFT_SURROUND_REAR_INDEX);
		cmd->privateInfo[8] = htonl(AUDIO_RIGHT_SURROUND_REAR_INDEX);
		cmd->privateInfo[9] = htonl(AUDIO_CENTER_FRONT_INDEX);
		cmd->privateInfo[10] = htonl(AUDIO_LFE_INDEX);
		cmd->privateInfo[11] = htonl(AUDIO_LEFT_OUTSIDE_FRONT_INDEX);
		cmd->privateInfo[12] = htonl(AUDIO_RIGHT_OUTSIDE_FRONT_INDEX);
		break;
	default:
		cmd->privateInfo[5] = htonl(AUDIO_LEFT_FRONT_INDEX);
		cmd->privateInfo[6] = htonl(AUDIO_RIGHT_FRONT_INDEX);
		cmd->privateInfo[7] = htonl(0);
		cmd->privateInfo[8] = htonl(0);
		cmd->privateInfo[9] = htonl(0);
		cmd->privateInfo[10] = htonl(0);
		cmd->privateInfo[11] = htonl(0);
		cmd->privateInfo[12] = htonl(0);
		break;
	}

	if (audio_send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_PRIVATEINFO,
		(unsigned long) cmd_audio,
		(unsigned long) res_audio,
		(unsigned long) res, &ret)) {
		rtd_pr_adsp_err("[ALSA] %s %d RPC fail\n", __func__, __LINE__);
		dvr_free(vir_addr);
		return -1;
	}

	dvr_free(vir_addr);
	return 0;
}

int RPC_TOAGENT_CHECK_AO_READY(struct snd_card *card)
{
	unsigned int *info, *info_audio;
	RPCRES_LONG *res, *res_audio;
	unsigned long ret;

	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long phy_addr = 0;

	vir_addr = dvr_malloc_uncached_specific(RPC_DVR_MALLOC_UNCACHED_SIZE, GFP_DCU1, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[ALSA] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = (unsigned long) dvr_to_phys(vir_addr);

	/* set parameter to 0, for snapshot */
	memset(unvir_addr, 0, RPC_DVR_MALLOC_UNCACHED_SIZE);

	info = unvir_addr;
	info_audio = (void *)(unsigned long *)((unsigned long) phy_addr);

	res = (void *)(RPCRES_LONG *) ((((unsigned long)unvir_addr + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS) + 8) & ALIGN4));
	res_audio = (void *)(RPCRES_LONG *) (((((unsigned long) phy_addr) + sizeof(RPCRES_LONG) + 8) & ALIGN4));

	if (audio_send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_CHECK_READY,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		(unsigned long) res, &ret)) {
		rtd_pr_adsp_err("[ALSA] %s %d RPC fail\n", __func__, __LINE__);
		dvr_free(vir_addr);
		return -1;
	}

	if (info == 0) {
		dvr_free(vir_addr);
		return -1;
	}

	dvr_free(vir_addr);
	return 0;
}

static int rtk_snd_check_ao_ready(struct snd_card *card)
{
	DEFINE_WAIT(wait);
	wait_queue_head_t q;	/* for blocking read */
	long remain_time;
	rtd_pr_adsp_debug("[ALSA] %s %d enter\n", __func__, __LINE__);

	/* Initialize wait queue... */
	init_waitqueue_head(&q);
	do {
		if (RPC_TOAGENT_CHECK_AO_READY(card) != 0) {
            if (alsa_debug)
                rtd_pr_adsp_debug("[ALSA] wait AO READY\n");
		} else {
            if (alsa_debug)
                rtd_pr_adsp_debug("[ALSA] AO ready .....\n");
			return 0;
		}

		prepare_to_wait(&q, &wait, TASK_INTERRUPTIBLE);
		remain_time = schedule_timeout(HZ);
		finish_wait(&q, &wait);
	} while (remain_time == 0);

	return 1;
}

static int rtk_snd_create_ao_agent(struct snd_card *card)
{
	struct AUDIO_RPC_INSTANCE *info, *info_audio;
	RPCRES_LONG *res, *res_audio;
	unsigned long ret;

	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long phy_addr = 0;

	vir_addr = dvr_malloc_uncached_specific(RPC_DVR_MALLOC_UNCACHED_SIZE, GFP_DCU1, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[ALSA] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = (unsigned long) dvr_to_phys(vir_addr);

	/* set parameter to 0, for snapshot */
	memset(unvir_addr, 0, RPC_DVR_MALLOC_UNCACHED_SIZE);

	info = unvir_addr;
	info_audio = (void *)(struct AUDIO_RPC_INSTANCE *)((ulong) phy_addr);

	res = (void *)(RPCRES_LONG *) ((((unsigned long)unvir_addr + sizeof(struct AUDIO_RPC_INSTANCE) + 8) & ALIGN4));
	res_audio = (void *)(RPCRES_LONG *) (((((unsigned long) phy_addr) + sizeof(struct AUDIO_RPC_INSTANCE) + 8) & ALIGN4));

	info->instanceID = htonl(0);
	info->type = htonl(AUDIO_OUT);
	rtd_pr_adsp_debug("[ALSA] %s %d\n", __func__, __LINE__);

	if (audio_send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_CREATE_AGENT,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		(unsigned long) res, &ret)) {
		rtd_pr_adsp_err("[ALSA] %s %d RPC fail\n", __func__, __LINE__);
		dvr_free(vir_addr);
		return -1;
	}

	alsa_agent = ntohl(res->data);

	dvr_free(vir_addr);
	rtd_pr_adsp_debug("[ALSA] AO agent ID = %d\n", alsa_agent);
	return 0;
}

/* create AI agent to get Instance ID for capture */
static int rtk_snd_create_ai_agent(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_cap_pcm *dpcm = runtime->private_data;
	struct AUDIO_RPC_INSTANCE *info, *info_audio;
	RPCRES_LONG *res, *res_audio;
	unsigned long ret;

	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long phy_addr = 0;

	if (snd_open_ai_count >= MAX_AI_DEVICES) {
		rtd_pr_adsp_err("[ALSA] too more capture open %d\n", __LINE__);
		return -1;
	}

	vir_addr = dvr_malloc_uncached_specific(RPC_DVR_MALLOC_UNCACHED_SIZE, GFP_DCU1, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[ALSA] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = (unsigned long) dvr_to_phys(vir_addr);

	/* set parameter to 0, for snapshot */
	memset(unvir_addr, 0, RPC_DVR_MALLOC_UNCACHED_SIZE);

	info = unvir_addr;
	info_audio = (void *)(struct AUDIO_RPC_INSTANCE *)((ulong) phy_addr);

	res = (void *)(RPCRES_LONG *) ((((unsigned long) unvir_addr + sizeof(struct AUDIO_RPC_INSTANCE) + 8) & ALIGN4));
	res_audio = (void *)(RPCRES_LONG *) (((((unsigned long) phy_addr) + sizeof(struct AUDIO_RPC_INSTANCE) + 8) &	ALIGN4));

	info->instanceID = htonl(-1);
	info->type = htonl(AUDIO_IN);

	rtd_pr_adsp_debug("[ALSA] %s %d\n", __func__, __LINE__);

	if (audio_send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_CREATE_AGENT,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		(unsigned long) res, &ret)) {
		rtd_pr_adsp_err("[ALSA] %s %d RPC fail\n", __func__, __LINE__);
		dvr_free(vir_addr);
		return -1;
	}

	dpcm->ai_cap_agent = ntohl(res->data);
	dpcm->ao_cap_agent =  -1;

	dvr_free(vir_addr);
	rtd_pr_adsp_debug("[ALSA] AI agent ID = %d\n", dpcm->ai_cap_agent);
	snd_open_ai_count++;
	return 0;
}

/* create AO agent to get Instance ID for capture */
static int rtk_snd_create_ao_capture_agent(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_cap_pcm *dpcm = runtime->private_data;
	struct AUDIO_RPC_INSTANCE *info, *info_audio;
	RPCRES_LONG *res, *res_audio;
	unsigned long ret;

	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long phy_addr = 0;

	if (snd_open_ai_count >= MAX_AI_DEVICES) {
		rtd_pr_adsp_err("[ALSA] capture busy %d\n", __LINE__);
		return -1;
	}

	if (rtk_snd_check_ao_ready(dpcm->card->card)) {
		rtd_pr_adsp_err("[ALSA] ao not ready\n");
		return -1;
	}

	vir_addr = dvr_malloc_uncached_specific(RPC_DVR_MALLOC_UNCACHED_SIZE, GFP_DCU1, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[ALSA] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = (unsigned long) dvr_to_phys(vir_addr);

	/* set parameter to 0, for snapshot */
	memset(unvir_addr, 0, RPC_DVR_MALLOC_UNCACHED_SIZE);

	info = unvir_addr;
	info_audio = (void *)(struct AUDIO_RPC_INSTANCE *)((ulong) phy_addr);

	res = (void *)(RPCRES_LONG *) ((((unsigned long)unvir_addr + sizeof(struct AUDIO_RPC_INSTANCE) + 8) & ALIGN4));
	res_audio = (void *)(RPCRES_LONG *) (((((unsigned long) phy_addr) + sizeof(struct AUDIO_RPC_INSTANCE) + 8) & ALIGN4));

	info->instanceID = htonl(0);
	info->type = htonl(AUDIO_OUT);

	rtd_pr_adsp_debug("[ALSA] [%s %d]\n", __func__, __LINE__);

	if (audio_send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_CREATE_AGENT,
		(unsigned long) info_audio,
		(unsigned long) res_audio,
		(unsigned long) res, &ret)) {
		rtd_pr_adsp_err("[ALSA] %s %d RPC fail\n", __func__, __LINE__);
		dvr_free(vir_addr);
		return -1;
	}

	dpcm->ao_cap_agent = ntohl(res->data);
	dpcm->ai_cap_agent = -1;

	dvr_free(vir_addr);
	rtd_pr_adsp_debug("[ALSA] AO capture agent ID = %d\n", dpcm->ao_cap_agent);
	snd_open_ai_count++;
	return 0;
}

static int rtk_snd_init(struct snd_card *card)
{
	static int init = 1;
	rtd_pr_adsp_debug("[ALSA] %s %d\n", __func__, __LINE__);

	if (init == 0)
		return 0;

	rtd_pr_adsp_debug("[ALSA] %s %d\n", __func__, __LINE__);

	if (rtk_snd_check_ao_ready(card))
		return -1;

	rtk_snd_create_ao_agent(card);
	init = 0;
	return 0;
}

static int rtk_snd_open(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_pcm *dpcm = runtime->private_data;
	int pin, ret;
#ifdef USE_FIXED_AO_PINID
	int pinmap[8], i, error_handle, dev_num;

	error_handle = 0;
	dev_num = substream->pcm->device;
#endif

	pin = 0;
	ret = 0;

	dpcm->ao_pin_id = 0;

	if (snd_open_count >= MAX_PCM_SUBSTREAMS) {
		rtd_pr_adsp_err("[ALSA] too more open %d\n", MAX_PCM_SUBSTREAMS);
		ret = -1;
		goto exit;
	}

#ifdef USE_FIXED_AO_PINID
	pin = rtk_snd_get_used_pin_id(dpcm->card->card);
	rtd_pr_adsp_debug("[ALSA] FW used pinmap %x\n", pin);

	for(i = 0; i < 8; i++) {
		pinmap[i] = pin % 2;
		pin = pin / 2;
	}

	if(used_ao_pin[dev_num] == 0 && pinmap[dev_num] == 1) {
		dpcm->ao_pin_id = FLASH_AUDIO_PIN_1 + dev_num;
		used_ao_pin[dev_num] = 1;
		if (rtk_snd_set_flashpin_id(dpcm->card->card, dpcm->ao_pin_id)) {
			rtd_pr_adsp_err("[ALSA] set flashpin fail\n");
			ret = -1;
			goto exit;
		}
		rtd_pr_adsp_debug("[ALSA] use fixed pin %d\n", dev_num);
	} else {
		rtd_pr_adsp_err("[ALSA] device %d already opened\n", dev_num);
		error_handle = 1;
	}

	if (error_handle == 1) {
		if (used_ao_pin[dev_num] == 0) {
			rtd_pr_adsp_err("[ALSA] error handle for pin release\n");
			dpcm->ao_pin_id = FLASH_AUDIO_PIN_1 + dev_num;
			used_ao_pin[dev_num] = 1;

			/* error handle for preclose */
			if (flush_error[dev_num] > 0) {
				rtk_snd_flush(substream);
				flush_error[dev_num] = 0;
			}
			if (pause_error[dev_num] > 0) {
				rtk_snd_pause(substream);
				pause_error[dev_num] = 0;
			}
			if (close_error[dev_num] > 0) {
				rtk_snd_close(substream);
				close_error[dev_num] = 0;
			}
			if (release_error[dev_num] > 0) {
				rtk_snd_release_flashpin_id(dpcm->card->card, dpcm->ao_pin_id);
				release_error[dev_num] = 0;
			}

			if (rtk_snd_set_flashpin_id(dpcm->card->card, dpcm->ao_pin_id)) {
				rtd_pr_adsp_err("[ALSA] set flashpin fail\n");
				ret = -1;
				goto exit;
			}
			rtd_pr_adsp_debug("[ALSA] use fixed pin\n");
		} else {
			rtd_pr_adsp_err("[ALSA] need check status\n");

			for (i = 0; i < 8; i++) {
				rtd_pr_adsp_err("[ALSA] pinmap[%d] = %d, used_ao_pin[%d] = %d\n",
					i, pinmap[i], i, used_ao_pin[i]);
			}

			ret = -1;
			goto exit;
		}
	}

#else
	dpcm->ao_pin_id = rtk_snd_query_flashpin_id(dpcm->card->card);
	rtd_pr_adsp_debug("[ALSA] use query pin\n");
	if (dpcm->ao_pin_id < 0) {
		rtd_pr_adsp_err("[ALSA] can't get flash pin");
		ret = -1;
		goto exit;
	}
#endif

	dpcm->ring_init = 0;
	dpcm->volume_change = 0;
	snd_open_count++;

	rtd_pr_adsp_debug("[ALSA] Audio AgentID = %d pin_id = %d\n",
		dpcm->ao_agent, dpcm->ao_pin_id);

exit:
	return ret;
}

static int rtk_snd_close(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime;
	struct rtk_snd_pcm *dpcm;
	struct RPC_RBUF_HEADER buf;
	int ret = 0;
	int result = 0;
	int i;
	int temp;

	rtd_pr_adsp_debug("[ALSA] %s %d\n", __func__, __LINE__);
	runtime = substream->runtime;
	dpcm = runtime->private_data;

	/* re-initialize ring buffer with null ring */
	buf.instanceID = dpcm->ao_agent;
	buf.pin_id = dpcm->ao_pin_id;
	buf.rd_idx = 0;
	buf.listsize = 0;

	for (i = 0; i < 8; i++)
		buf.rbuf_list[i] = 0;

	/*stop AO agent*/
	temp = dpcm->ao_agent | dpcm->ao_pin_id;
	result = RPC_TOAGENT_STOP_SVC(dpcm->card->card, &temp);

	/* stop decoder */
	if (dpcm->audiopath == AUDIO_PATH_DECODER_AO)
		RPC_TOAGENT_STOP_SVC(dpcm->card->card, &dpcm->agent_id);

	/* initial ring buffer NULL */
	ret = RPC_TOAGENT_INITRINGBUFFER_HEADER_SVC(dpcm->card->card, &buf);

	/* destroy decoder instance if exist */
	if (dpcm->audiopath == AUDIO_PATH_DECODER_AO)
		ret = RPC_TOAGENT_DESTROY_SVC(dpcm->card->card, &dpcm->agent_id);

	return ret;
}

static int rtk_snd_ai_hw_close(struct snd_pcm_substream *substream)
{
	unsigned int instanceID;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_cap_pcm *dpcm = runtime->private_data;
	struct RPC_RBUF_HEADER buf;
	int ret = 0;
	int i, do_uninit = 1;
	bool voice_enable = (substream->pcm->device) == 2 ? TRUE : FALSE;

	if (voice_enable && check_voice_ready()) {
		suspend_voice();
		return 0;
	}

	if (is_far_field(substream->pcm->device)) {
		mutex_lock(&dpcm->card->far_field_lock);
		if (dpcm->card->reference_count == 2) {
			/* other device still used */
			do_uninit = 0;
		} else if (dpcm->card->reference_count == 1) {
			do_uninit = 1;
		}
		mutex_unlock(&dpcm->card->far_field_lock);
	} else {
		do_uninit = 1;
	}

	if (do_uninit) {
		instanceID = ((dpcm->ai_cap_agent&0xFFFFFFF0)|(dpcm->ai_cap_pin&0xF));

		/* stop AI */
		RPC_TOAGENT_STOP_SVC(dpcm->card->card, &instanceID);
		rtd_pr_adsp_debug("[ALSA] AI stop success\n");

		/* re-initialize ring buffer with null ring */
		buf.instanceID = dpcm->ai_cap_agent;
		buf.pin_id = dpcm->ai_cap_pin;
		buf.rd_idx = 0;
		buf.listsize = 0;

		for (i = 0; i < 8; i++)
			buf.rbuf_list[i] = 0;

		/* initial ring buffer NULL */
    	ret = RPC_TOAGENT_INITRINGBUFFER_HEADER_SVC(dpcm->card->card, &buf);
    	rtd_pr_adsp_debug("[ALSA] AI init ring success\n");
    }

	return 0;
}

static int rtk_snd_ao_capture_hw_close(struct snd_pcm_substream *substream)
{
	unsigned int instanceID;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_cap_pcm *dpcm = runtime->private_data;
	struct RPC_RBUF_HEADER buf;
	int ret = 0;
	int i;

	instanceID = ((dpcm->ao_cap_agent&0xFFFFFFF0)|(dpcm->ao_cap_pin&0xF));

	/* re-initialize ring buffer with null ring */
	buf.instanceID = dpcm->ao_cap_agent;
	buf.pin_id = dpcm->ao_cap_pin;
	buf.rd_idx = 0;
	buf.listsize = 0;

	for (i = 0; i < 8; i++)
		buf.rbuf_list[i] = 0;

	/* initial ring buffer NULL */
	ret = RPC_TOAGENT_INITRINGBUFFER_HEADER_SVC(dpcm->card->card, &buf);
	rtd_pr_adsp_debug("[ALSA] AO capture init ring success\n");

	return 0;
}

static void rtk_snd_pcm_timer_start(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_pcm *dpcm = runtime->private_data;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	struct timespec64 t;
#else
	struct timeval t;
#endif

    if (alsa_debug)
        rtd_pr_adsp_debug("[ALSA] %s %d\n", __func__, __LINE__);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	ktime_get_real_ts64(&t);
	dpcm->pre_time_ms = t.tv_sec*1000 + t.tv_nsec/1000000;
#else
	do_gettimeofday(&t);
	dpcm->pre_time_ms = t.tv_sec*1000 + t.tv_usec/1000;
#endif

	dpcm->pre_no_datatime_ms = 0;
	dpcm->pre_writePtr = ntohl(dpcm->ring[0].beginAddr);
	dpcm->max_level = 0;
	dpcm->min_level = dpcm->buffer_byte_size;
	dpcm->total_data_wb = 0;

	spin_lock(dpcm->pcm_lock);
	dpcm->running = 1;
	spin_unlock(dpcm->pcm_lock);

#if defined(CONFIG_PCM_USING_HRTIMER)
	dpcm->period_jiffies = 5; /* in ms */
	dpcm_timer_start(dpcm);
#else
	dpcm->period_jiffies = 1; /* in timer tick */
	mod_timer(&dpcm->timer, jiffies);
#endif
}

static void rtk_snd_pcm_timer_stop(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_pcm *dpcm = runtime->private_data;

    if (alsa_debug)
        rtd_pr_adsp_debug("[ALSA] %s %d\n", __func__, __LINE__);

	spin_lock(dpcm->pcm_lock);
	dpcm->running = 0;
	spin_unlock(dpcm->pcm_lock);

#if defined(CONFIG_PCM_USING_HRTIMER)
	dpcm_timer_stop(dpcm, false);
#else
	del_timer(&dpcm->timer);
    if (alsa_debug)
        rtd_pr_adsp_debug("[ALSA] del timer stop\n");
#endif
}

static void rtk_snd_pcm_capture_timer_start(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_cap_pcm *dpcm = runtime->private_data;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	struct timespec64 t;
#else
	struct timeval t;
#endif

	dpcm->pcm_buf_pos = 0;
	dpcm->period_jiffies = 1;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	ktime_get_real_ts64(&t);
	dpcm->last_time_ms = t.tv_sec*1000 + t.tv_nsec/1000000;
#else
	do_gettimeofday(&t);
	dpcm->last_time_ms = t.tv_sec*1000 + t.tv_usec/1000;
#endif

    if (alsa_debug)
        rtd_pr_adsp_debug("[ALSA] %s %d\n", __func__, __LINE__);

	if ((dpcm->card->reference_count == 2) && is_far_field(substream->pcm->device)) {
		dpcm->card->far_field_ring_audio->readPtr[1] = dpcm->card->far_field_ring_audio->writePtr;
		dpcm->ring[0].numOfReadPtr = htonl(2);
		rtd_pr_adsp_info("[ALSA][FAR FIELD] numOfReadPtr %x, rp[1] %x\n",
			(unsigned int) dpcm->ring[0].numOfReadPtr, dpcm->card->far_field_ring_audio->readPtr[1]);
	}

	spin_lock(dpcm->pcm_lock);
	mod_timer(&dpcm->timer, jiffies + 1);
	dpcm->running = 1;
	spin_unlock(dpcm->pcm_lock);
}

static void rtk_snd_pcm_capture_timer_stop(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_cap_pcm *dpcm = runtime->private_data;
    if (alsa_debug)
        rtd_pr_adsp_debug("[ALSA] %s %d\n", __func__, __LINE__);

	if ((dpcm->card->reference_count == 2) && is_far_field(substream->pcm->device)) {
		dpcm->ring[0].numOfReadPtr = htonl(1);
		rtd_pr_adsp_info("[ALSA][FAR FIELD] numOfReadPtr %x\n", (unsigned int) dpcm->ring[0].numOfReadPtr);
	}

	spin_lock(dpcm->pcm_lock);
	dpcm->running = 0;
	del_timer(&dpcm->timer);
	spin_unlock(dpcm->pcm_lock);
}

static int rtk_snd_playback_trigger(struct snd_pcm_substream *substream, int cmd)
{
	//rtd_pr_adsp_info("[ALSA][%s %d] %d, dev %d\n", __func__, __LINE__, cmd, substream->pcm->device);

	if (cmd == SNDRV_PCM_TRIGGER_START)
		rtk_snd_pcm_timer_start(substream);
	else if (cmd == SNDRV_PCM_TRIGGER_STOP)
		rtk_snd_pcm_timer_stop(substream);
	else
		return -EINVAL;

	return 0;
}

static int rtk_snd_capture_trigger(struct snd_pcm_substream *substream, int cmd)
{
	rtd_pr_adsp_info("[ALSA][%s %d] %d, dev %d\n", __func__, __LINE__, cmd, substream->pcm->device);

	if (cmd == SNDRV_PCM_TRIGGER_START)
		rtk_snd_pcm_capture_timer_start(substream);
	else if (cmd == SNDRV_PCM_TRIGGER_STOP)
		rtk_snd_pcm_capture_timer_stop(substream);
	else
		return -EINVAL;

	return 0;
}

static int rtk_snd_pcm_prepare(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_pcm *dpcm = runtime->private_data;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	struct timespec64 t;
#else
	struct timeval t;
#endif
	long ctms;
	long ptms;
	int dev;

	rtd_pr_adsp_info("[ALSA] [%s]\n", __func__);

	if (dpcm->ring_init == 1) {
		if (dpcm->periods != runtime->periods) {
			rtd_pr_adsp_err("[ALSA] periods different\n");
			dpcm->ring_init = 0;
		}
		if (dpcm->period_size != runtime->period_size) {
			rtd_pr_adsp_err("[ALSA] period_size different\n");
			dpcm->ring_init = 0;
		}
		if (dpcm->buffer_size != runtime->buffer_size) {
			rtd_pr_adsp_err("[ALSA] buffer_size different\n");
			dpcm->ring_init = 0;
		}
		if (dpcm->access != runtime->access) {
			rtd_pr_adsp_err("[ALSA] access different\n");
			dpcm->ring_init = 0;
		}
		if (dpcm->format != runtime->format) {
			rtd_pr_adsp_err("[ALSA] format different\n");
			dpcm->ring_init = 0;
		}
		if (dpcm->channels != runtime->channels) {
			rtd_pr_adsp_err("[ALSA] channels different\n");
			dpcm->ring_init = 0;
		}
		if (dpcm->ring_bak[0].beginAddr !=
			(unsigned long) runtime->dma_addr) {
			rtd_pr_adsp_err("[ALSA] dma_addr different\n");
			dpcm->ring_init = 0;
		}
		if (dpcm->ring_init == 0) {
			rtk_snd_flush(substream);
			rtk_snd_pause(substream);
			rtk_snd_close(substream);
		}
	}

	dpcm->pcm_jiffie = runtime->rate / HZ;
	dpcm->pcm_size = snd_pcm_lib_buffer_bytes(substream);
	dpcm->pcm_count = snd_pcm_lib_period_bytes(substream);
	dpcm->flush_state = RTK_SND_FLUSH_STATE_NONE;
	dpcm->periods = runtime->periods;
	dpcm->buffer_size = runtime->buffer_size;
	dpcm->access = runtime->access;
	dpcm->channels = runtime->channels;
	dpcm->format = runtime->format;
	dpcm->period_size = runtime->period_size;
	dpcm->sample_bits = runtime->sample_bits;
	dpcm->rate = runtime->rate;
	dpcm->remain_sample = 0;
	dpcm->running = 0;
	dpcm->appl_ptr = 0;
	spin_lock(dpcm->pcm_lock);
	dpcm->pcm_buf_pos = 0;
	spin_unlock(dpcm->pcm_lock);
	dpcm->pcm_irq_pos = 0;

	dpcm->period_jiffies = runtime->period_size * 100 / runtime->rate * 1 / 20;
	if (dpcm->period_jiffies == 0)
		dpcm->period_jiffies = 1;

	dpcm->hwptr_error_times = 0;

	switch (runtime->access) {
		case SNDRV_PCM_ACCESS_MMAP_INTERLEAVED:
			rtd_pr_adsp_debug("[ALSA] MMAP_INTERLEAVED\n");
#ifdef USE_DECODER
			/*old flow ALSA -> Decoder -> AO */
			dpcm->audiopath = AUDIO_PATH_DECODER_AO;
#else
			/* new flow ALSA -> AO */
			dpcm->audiopath = AUDIO_PATH_AO;
#endif
			break;
		case SNDRV_PCM_ACCESS_MMAP_NONINTERLEAVED:
			rtd_pr_adsp_debug("[ALSA] MMAP_NONINTERLEAVED\n");
			dpcm->audiopath = AUDIO_PATH_AO_BYPASS;
			break;
		case SNDRV_PCM_ACCESS_RW_INTERLEAVED:
			rtd_pr_adsp_debug("[ALSA] RW_INTERLEAVED\n");
#ifdef USE_DECODER
			/*old flow ALSA -> Decoder -> AO */
			dpcm->audiopath = AUDIO_PATH_DECODER_AO;
#else
			/* new flow ALSA -> AO */
			dpcm->audiopath = AUDIO_PATH_AO;
#endif
			break;
		case SNDRV_PCM_ACCESS_RW_NONINTERLEAVED:
			rtd_pr_adsp_debug("[ALSA] RW_NONINTERLEAVED\n");
			dpcm->audiopath = AUDIO_PATH_AO_BYPASS;
			break;
		default:
			rtd_pr_adsp_debug("[ALSA][%d] unsupport mode\n", __LINE__);
			return -1;
			break;
	}

	if (dpcm->audiopath == AUDIO_PATH_AO) {
		switch (runtime->format) {
#ifdef USE_DECODER
		case SNDRV_PCM_FORMAT_S32_BE:
		case SNDRV_PCM_FORMAT_S24_BE:
		case SNDRV_PCM_FORMAT_S24_LE:
		case SNDRV_PCM_FORMAT_S8:
#endif
		case SNDRV_PCM_FORMAT_S16_BE:
		case SNDRV_PCM_FORMAT_S16_LE:
			break;
		default:
			rtd_pr_adsp_debug("[ALSA] unsupport format\n");
			return -1;
			break;
		}
	}

	if (dpcm->ring_init == 1) {
		rtk_snd_pause(substream);
		spin_lock(dpcm->pcm_lock);
		dpcm->pcm_buf_pos = dpcm->appl_ptr;
		runtime->status->hw_ptr = runtime->control->appl_ptr;
		dpcm->ring_bak[0].writePtr = dpcm->ring_bak[0].beginAddr;
		dpcm->ring[0].writePtr = htonl(dpcm->ring_bak[0].writePtr);
		spin_unlock(dpcm->pcm_lock);
		rtk_snd_flush(substream);
		rtk_snd_resume(substream);

		dpcm->running = 0;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
		ktime_get_real_ts64(&t);
		ptms = t.tv_sec*1000 + t.tv_nsec/1000000;
#else
		do_gettimeofday(&t);
		ptms = t.tv_sec*1000 + t.tv_usec/1000;
#endif

		/* wait ADSP finish flush */
		while (1) {
			if (dpcm->ring_bak[0].writePtr == ntohl(dpcm->ring[0].readPtr[0])) {
				if (alsa_debug) {
					rtd_pr_adsp_debug("[ALSA] wp %lx, rp %x\n", dpcm->ring_bak[0].writePtr, ntohl(dpcm->ring[0].readPtr[0]));
				}
				break;
			}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
			ktime_get_real_ts64(&t);
			ctms = t.tv_sec*1000 + t.tv_nsec/1000000;
#else
			do_gettimeofday(&t);
			ctms = t.tv_sec*1000 + t.tv_usec/1000;
#endif
			if ((ctms - ptms) >= 100) {
				/* wait timeout prevent deadlock */
				rtd_pr_adsp_err("[ALSA] wait ADSP flush timeout!!!\n");
				break;
			}
		}

		rtd_pr_adsp_info("[ALSA] reprepare!! shared ring wp %x, rp %x\n", ntohl(dpcm->ring[0].writePtr), ntohl(dpcm->ring[0].readPtr[0]));
		return 0;
	}

	dpcm->ring_init = 1;
	dev = substream->pcm->device;

	rtd_pr_adsp_debug("[ALSA] start_threshold = %lx\n", runtime->start_threshold);
	rtd_pr_adsp_debug("[ALSA] stop_threshold = %lx\n", runtime->stop_threshold);
	rtd_pr_adsp_debug("[ALSA] avail_min = %lx\n", runtime->control->avail_min);
	rtd_pr_adsp_debug("[ALSA] buffer_size = %lx\n", runtime->buffer_size);
	rtd_pr_adsp_debug("[ALSA] period_size = %lx\n", runtime->period_size);
	rtd_pr_adsp_debug("[ALSA] sample rate = %d\n", runtime->rate);

	if (dpcm->audiopath == AUDIO_PATH_DECODER_AO) {
		dpcm->output_frame_bytes = frames_to_bytes(runtime, 1);
		dpcm->output_sample_bytes = samples_to_bytes(runtime, 1);
		dpcm->input_sample_bytes = samples_to_bytes(runtime, 1);

		rtk_snd_create_decoder_agent(substream);

		rtd_pr_adsp_debug("[ALSA] dec_agent = %d, dec_pin_id = %d\n",
			dpcm->dec_agent, dpcm->dec_pin_id);

		rtk_snd_init_decoder_ring(substream);
		if (rtk_snd_init_connect_decoder_ao(substream)) {
			rtd_pr_adsp_debug("[ALSA] %s %d\n", __func__, __LINE__);
			return -ENOMEM;
		}

		/*
		*	AO Pause
		*	Decoder Pause
		*	Decoder Flush
		*	Write Inband data
		*/
		rtk_snd_init_decoder_info(substream);
		rtk_snd_set_ao_flashpin_volume(substream);
		rtk_snd_resume(substream);

		/*
		substream->ops->silence = NULL;
		substream->ops->copy = snd_card_std_copy;
		*/

	} else if (dpcm->audiopath == AUDIO_PATH_AO_BYPASS) {

		rtd_pr_adsp_debug("[ALSA] AO BYPASS\n");
		dpcm->agent_id = dpcm->ao_agent;
		dpcm->pin_id = dpcm->ao_pin_id;
		dpcm->extend_to_32be_ratio = 0;
		dpcm->output_frame_bytes = samples_to_bytes(runtime, 1);
		dpcm->output_sample_bytes = samples_to_bytes(runtime, 1);
		dpcm->input_sample_bytes = samples_to_bytes(runtime, 1);

		/*
		substream->ops->copy = NULL;
		substream->ops->silence = NULL;
		*/

		if (rtk_snd_init_ao_ring(substream))
			return -ENOMEM;

		if (rtk_snd_ao_info(substream)) {
			rtd_pr_adsp_err("[ALSA] %s %d\n", __func__, __LINE__);
			return -ENOMEM;
		}
		rtk_snd_resume(substream);
	} else if (dpcm->audiopath == AUDIO_PATH_AO) {
		rtd_pr_adsp_debug("[ALSA] AO interleaved\n");
		dpcm->agent_id = dpcm->ao_agent;
		dpcm->pin_id = dpcm->ao_pin_id;
		dpcm->card->ao_pin_id[dev] = dpcm->ao_pin_id;
		dpcm->volume = dpcm->card->ao_flash_volume[dev];

		dpcm->output_frame_bytes = frames_to_bytes(runtime, 1);
		dpcm->output_sample_bytes = samples_to_bytes(runtime, 1);
		dpcm->input_sample_bytes = samples_to_bytes(runtime, 1);

		/*
		substream->ops->copy = NULL;
		substream->ops->silence = NULL;
		*/

		if (rtk_snd_init_ao_ring(substream)) {
			rtd_pr_adsp_err("[ALSA] %d Fail!!\n", __LINE__);
			return -ENOMEM;
		}

		if (rtk_snd_ao_info(substream)) {
			rtd_pr_adsp_err("[ALSA] %d Fail!!\n", __LINE__);
			return -ENOMEM;
		}

		rtk_snd_set_ao_flashpin_volume(substream);
		rtk_snd_resume(substream);
	}

	return 0;
}

static int rtk_snd_playback_prepare(struct snd_pcm_substream *substream)
{
	return rtk_snd_pcm_prepare(substream);
}

#ifndef CONFIG_CUSTOMER_TV006
static int get_mux_in(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_cap_pcm *dpcm = runtime->private_data;
	int ret = 0;

	switch (dpcm->path_src) {
	case AUDIO_IPT_SRC_BBADC:
		/* not used yet */
		break;
	case AUDIO_IPT_SRC_SPDIF:
		/* not used yet */
		break;
	case AUDIO_IPT_SRC_I2S_PRI_CH12:
		if (substream->pcm->device == 0) {
			dpcm->i2si_mux_in = AUDIO_I2SI_SRC_IN;
		} else if (substream->pcm->device == 1) {
			dpcm->i2si_mux_in = AUDIO_I2SI_SRC_LOOPBACK;
		}
		break;
	case AUDIO_IPT_SRC_I2S_PRI_CH34:
	case AUDIO_IPT_SRC_I2S_PRI_CH56:
	case AUDIO_IPT_SRC_I2S_PRI_CH78:
		/* not used yet */
		break;
	case AUDIO_IPT_SRC_DMIC0:
		dpcm->i2si_mux_in = AUDIO_I2SI_SRC_LOOPBACK;
		break;
	default:
		ret = -1;
		rtd_pr_adsp_err("[ALSA] unsupport path_src %d\n", dpcm->path_src);
		break;
	}

	return ret;
}
#endif

static int get_aio_config_src(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_cap_pcm *dpcm = runtime->private_data;
	int ret = 0;

#ifndef CONFIG_CUSTOMER_TV006
	rtd_pr_adsp_debug("[ALSA] capture device: %d\n", substream->pcm->device);

	switch (substream->pcm->device) {
	case 0:
	case 1:
		dpcm->path_src = AUDIO_IPT_SRC_I2S_PRI_CH12;
		break;
	case 2:
	case 3:
	case 4:
		dpcm->path_src = AUDIO_IPT_SRC_DMIC0;
		break;
	default:
		rtd_pr_adsp_err("[ALSA] unsupport capture device %d\n", substream->pcm->device);
		ret = -1;
		break;
	}

	if (ret) {
		goto exit;
	}

	ret = get_mux_in(substream);
#else
	dpcm->path_src = ENUM_AUDIO_IPT_SRC_BBADC;
#endif

exit:
	return ret;
}

static int rtk_snd_ai_switch_focus(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_cap_pcm *dpcm = runtime->private_data;
	unsigned long ret;
	AUDIO_RPC_PRIVATEINFO_PARAMETERS *cmd, *cmd_audio;
	AUDIO_RPC_PRIVATEINFO_RETURNVAL *res, *res_audio;
	int instanceID;

	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long phy_addr = 0;

	if (get_aio_config_src(substream)) {
		rtd_pr_adsp_err("[ALSA] get path src failed\n");
		return -EINVAL;
	}

	rtd_pr_adsp_debug("[ALSA] path_src: %x\n", dpcm->path_src);

	vir_addr = dvr_malloc_uncached_specific(RPC_DVR_MALLOC_UNCACHED_SIZE, GFP_DCU1, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[ALSA] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = (unsigned long) dvr_to_phys(vir_addr);

	/* set parameter to 0, for snapshot */
	memset(unvir_addr, 0, RPC_DVR_MALLOC_UNCACHED_SIZE);

	cmd = unvir_addr;
	res = (void *)(AUDIO_RPC_PRIVATEINFO_RETURNVAL *) ((((unsigned long)unvir_addr + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS) + 8) & ALIGN4));

	cmd_audio = (void *)(AUDIO_RPC_PRIVATEINFO_PARAMETERS *) ((ulong) phy_addr);
	res_audio = (void *)(AUDIO_RPC_PRIVATEINFO_RETURNVAL *) (((((ulong) phy_addr) + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS) + 8) & ALIGN4));

	memset(cmd, 0, sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS));

	instanceID = ((dpcm->ai_cap_agent & 0xFFFFFFF0) | (dpcm->ai_cap_pin & 0xF));
	cmd->instanceID = htonl(instanceID);
	cmd->type = htonl(ENUM_PRIVATEINFO_AUDIO_AI_SWITCH_FOCUS);
	cmd->privateInfo[0] = htonl(dpcm->path_src);
	cmd->privateInfo[1] = htonl(AUDIO_IPT_SRC_UNKNOWN);
	cmd->privateInfo[2] = htonl(AUDIO_IPT_SRC_UNKNOWN);
	cmd->privateInfo[3] = htonl(AUDIO_IPT_SRC_UNKNOWN);

	switch (dpcm->path_src) {
	case AUDIO_IPT_SRC_BBADC:
		rtd_pr_adsp_debug("[ALSA] bbadc_mux_in: %x\n", dpcm->bbadc_mux_in);
		cmd->privateInfo[4] = htonl(dpcm->bbadc_mux_in);
	break;
	case AUDIO_IPT_SRC_SPDIF:
		rtd_pr_adsp_debug("[ALSA] spdifi_mux_in: %x\n", dpcm->spdifi_mux_in);
		cmd->privateInfo[4] = htonl(dpcm->spdifi_mux_in);
		break;
	case AUDIO_IPT_SRC_I2S_PRI_CH12:
	case AUDIO_IPT_SRC_I2S_PRI_CH34:
	case AUDIO_IPT_SRC_I2S_PRI_CH56:
	case AUDIO_IPT_SRC_I2S_PRI_CH78:
		rtd_pr_adsp_debug("[ALSA] i2si_mux_in: %x\n", dpcm->i2si_mux_in);
		cmd->privateInfo[4] = htonl(dpcm->i2si_mux_in);
		break;
	case AUDIO_IPT_SRC_DMIC0:
		rtd_pr_adsp_debug("[ALSA][VR] i2si_mux_in: %x\n", dpcm->i2si_mux_in);
		cmd->privateInfo[4] = htonl(dpcm->i2si_mux_in);
        	break;
	case AUDIO_IPT_SRC_ATV:
		cmd->privateInfo[4] = htonl(AUDIO_IPT_SRC_UNKNOWN);
		break;
	default:
		rtd_pr_adsp_debug("[ALSA] unknown mux_in\n");
		cmd->privateInfo[4] = htonl(AUDIO_IPT_SRC_UNKNOWN);
		break;
	}

	if (audio_send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_PRIVATEINFO,
		(unsigned long) cmd_audio,
		(unsigned long) res_audio,
		(unsigned long) res, &ret)) {
		rtd_pr_adsp_err("[ALSA] %d RPC fail\n", __LINE__);
		dvr_free(vir_addr);
		return -1;
	}

	dvr_free(vir_addr);
	return 0;
}

static int rtk_snd_ai_hw_resume(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_cap_pcm *dpcm = runtime->private_data;
	unsigned int temp;

	temp = ((dpcm->ai_cap_agent&0xFFFFFFF0)|(dpcm->ai_cap_pin&0xF));

	if (RPC_TOAGENT_RUN_SVC(dpcm->card->card, &temp) < 0)
		return -1;

	return 0;
}

/* AO capture get available AO out pin */
static int rpc_toagent_get_ao_outpin(struct snd_pcm_substream *substream)
{
	unsigned long ret;
	AUDIO_RPC_PRIVATEINFO_PARAMETERS *cmd, *cmd_audio;
	AUDIO_RPC_PRIVATEINFO_RETURNVAL *res , *res_audio;
	int instanceID, pin_id;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_cap_pcm *dpcm = runtime->private_data;

	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long phy_addr = 0;

	vir_addr = dvr_malloc_uncached_specific(RPC_DVR_MALLOC_UNCACHED_SIZE, GFP_DCU1, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[ALSA] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = (unsigned long) dvr_to_phys(vir_addr);

	/* set parameter to 0, for snapshot */
	memset(unvir_addr, 0, RPC_DVR_MALLOC_UNCACHED_SIZE);

	cmd = unvir_addr;
	res = (void *)(AUDIO_RPC_PRIVATEINFO_RETURNVAL *) ((((unsigned long)unvir_addr + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS) + 8) & ALIGN4));

	cmd_audio = (void *)(AUDIO_RPC_PRIVATEINFO_PARAMETERS *)((ulong) phy_addr);
	res_audio = (void *)(AUDIO_RPC_PRIVATEINFO_RETURNVAL *) ((((ulong) phy_addr + sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS) + 8) & ALIGN4));

	memset(cmd, 0, sizeof(AUDIO_RPC_PRIVATEINFO_PARAMETERS));
	instanceID = dpcm->ao_cap_agent;
	cmd->instanceID = htonl(instanceID);
	cmd->type = htonl(ENUM_PRIVATEINFO_AUDIO_GET_AVAILABLE_AO_OUTPUT_PIN);
	cmd->privateInfo[0] = 0xFF;
	cmd->privateInfo[1] = 0xFF;
	cmd->privateInfo[2] = 0xFF;
	cmd->privateInfo[3] = 0xFF;
	cmd->privateInfo[4] = 0xFF;
	cmd->privateInfo[5] = 0xFF;

	if (audio_send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_PRIVATEINFO,
		(unsigned long)cmd_audio,
		(unsigned long)res_audio,
		(unsigned long)res, &ret)) {
		rtd_pr_adsp_err("[ALSA] %d RPC fail\n", __LINE__);
		dvr_free(vir_addr);
		return -1;
	}

	pin_id = ntohl(res->privateInfo[0]);
	dpcm->ao_cap_pin = pin_id;

	dvr_free(vir_addr);
	return 0;
}

int rtk_snd_capture_prepare_BE(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_cap_pcm *dpcm = runtime->private_data;
    bool voice_enable = (substream->pcm->device == 2) ? TRUE : FALSE;

	if (dpcm->ai_format == AUDIO_ALSA_FORMAT_24BITS_BE_PCM) {
		dpcm->input_sample_bytes = 4;
	} else if (dpcm->ai_format == AUDIO_ALSA_FORMAT_16BITS_BE_PCM) {
		if (is_far_field(substream->pcm->device)) {
			/* 6ch * 16bit */
			dpcm->input_sample_bytes = 2 * 6;
		} else {
			/* Mars 1 for AI capture, Mars 2 for AO capture */
			if (!strcmp(dpcm->card->card->longname, "Mars_1")) {
				dpcm->input_sample_bytes = 2 * runtime->channels;
			} else {
				dpcm->input_sample_bytes = 4;
			}
		}
	}

	/* Mars 1 for AI capture, Mars 2 for AO capture */
	if (!strcmp(dpcm->card->card->longname, "Mars_1")) {
		dpcm->ai_cap_pin = ALSA_OUTPIN;

		mutex_lock(&dpcm->card->far_field_lock);

		if ((dpcm->card->reference_count == 0) || !is_far_field(substream->pcm->device)) {
			rtd_pr_adsp_debug("[ALSA] prepare open AI capture\n");
			/* send rpc init ai ring */
			if (rtk_snd_init_ai_ring(substream)) {
				rtd_pr_adsp_err("[ALSA] %d fail\n", __LINE__);
				mutex_unlock(&dpcm->card->far_field_lock);
				goto prepare_fail;
			}
			if (voice_enable && check_voice_ready()) {
				mutex_unlock(&dpcm->card->far_field_lock);
				return 0;
			}
			/* send rpc switch focus to AI capture */
			if (rtk_snd_ai_switch_focus(substream)) {
				rtd_pr_adsp_err("[ALSA] %d fail\n", __LINE__);
				mutex_unlock(&dpcm->card->far_field_lock);
				goto prepare_fail;
			}
			/* send rpc run AI */
			if (rtk_snd_ai_hw_resume(substream) < 0) {
				rtd_pr_adsp_err("[ALSA] %d fail\n", __LINE__);
				mutex_unlock(&dpcm->card->far_field_lock);
				goto prepare_fail;
			}
			set_voice_initialized();

			if (is_far_field(substream->pcm->device)) {
				dpcm->card->reference_count = 1;
			}
		} else if ((dpcm->card->reference_count == 1) && is_far_field(substream->pcm->device)) {
			/* uncache address for Audio FW */
			dpcm->ring = (RINGBUFFER_HEADER *)((unsigned long) dpcm->card->far_field_ring_audio);
			dpcm->ring_p = dpcm->card->far_field_vir;
			dpcm->rd_idx = dpcm->card->reference_count;
			dpcm->card->reference_count = 2;
		}
		mutex_unlock(&dpcm->card->far_field_lock);

		return 0;
	} else {
		rtd_pr_adsp_debug("[ALSA] prepare open AO capture\n");
		/* get available ao outpin */
		rpc_toagent_get_ao_outpin(substream);

		if(rtk_snd_set_ao_config(substream) != S_OK){
			rtd_pr_adsp_err("[ALSA] %d rtk_snd_set_ao_config fail\n", __LINE__);
			goto prepare_fail;
		}

		/* send rpc init ao capture ring */
		if (rtk_snd_init_ao_capture_ring(substream)) {
			rtd_pr_adsp_err("[ALSA] %d fail\n", __LINE__);
			goto prepare_fail;
		}

		return 0;
	}

prepare_fail:
	return -ENOMEM;
}

static int rtk_snd_capture_prepare(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_cap_pcm *dpcm = runtime->private_data;

	if (dpcm->ring_init == 1) {
		if (dpcm->periods != runtime->periods) {
			rtd_pr_adsp_err("[ALSA] periods different\n");
			dpcm->ring_init = 0;
		}
		if (dpcm->period_size != runtime->period_size) {
			rtd_pr_adsp_err("[ALSA] period_size different\n");
			dpcm->ring_init = 0;
		}
		if (dpcm->buffer_size != runtime->buffer_size) {
			rtd_pr_adsp_err("[ALSA] buffer_size different\n");
			dpcm->ring_init = 0;
		}
		if (dpcm->access != runtime->access) {
			rtd_pr_adsp_err("[ALSA] access different\n");
			dpcm->ring_init = 0;
		}
		if (dpcm->format != runtime->format) {
			rtd_pr_adsp_err("[ALSA] format different\n");
			dpcm->ring_init = 0;
		}
		if (dpcm->channels != runtime->channels) {
			rtd_pr_adsp_err("[ALSA] channels different\n");
			dpcm->ring_init = 0;
		}
		if (dpcm->ring_init == 0) {
			/* Mars 1 for AI capture, Mars 2 for AO capture */
			if (!strcmp(dpcm->card->card->longname, "Mars_1"))
				rtk_snd_ai_hw_close(substream);
			else
				rtk_snd_ao_capture_hw_close(substream);
		}
	}

	dpcm->pcm_jiffie = runtime->rate / HZ;
	dpcm->pcm_size = snd_pcm_lib_buffer_bytes(substream);
	dpcm->pcm_count = snd_pcm_lib_period_bytes(substream);
	dpcm->pcm_irq_pos = 0;
	dpcm->appl_ptr = 0;
	dpcm->flush_state = RTK_SND_FLUSH_STATE_NONE;
	dpcm->periods = runtime->periods;
	dpcm->buffer_size = runtime->buffer_size;
	dpcm->access = runtime->access;
	dpcm->channels = runtime->channels;
	dpcm->format = runtime->format;
	dpcm->period_size = runtime->period_size;
	dpcm->sample_bits = runtime->sample_bits;
	dpcm->rate = runtime->rate;
	dpcm->remain_sample = 0;

	dpcm->period_jiffies =
		runtime->period_size * 100 / runtime->rate * 1 / 20;
	if (dpcm->period_jiffies == 0)
		dpcm->period_jiffies = 1;

	rtd_pr_adsp_debug("[ALSA] period_jiffies = %d\n", dpcm->period_jiffies);

	/* check format */
	switch (runtime->access) {
	case SNDRV_PCM_ACCESS_MMAP_INTERLEAVED:
	case SNDRV_PCM_ACCESS_RW_INTERLEAVED:
		switch (runtime->format) {
		case SNDRV_PCM_FORMAT_S16_LE:
			break;
		case SNDRV_PCM_FORMAT_S24_LE:
			break;
		case SNDRV_PCM_FORMAT_S32_LE:
			if (!is_far_field(substream->pcm->device)) {
				rtd_pr_adsp_err("[ALSA] unsupport format %d, in this device %d\n", __LINE__, substream->pcm->device);
				return -1;
			}
			break;
		default:
			rtd_pr_adsp_err("[ALSA] unsupport format %d\n", __LINE__);
			return -1;
		}
		break;
	case SNDRV_PCM_ACCESS_MMAP_NONINTERLEAVED:
	case SNDRV_PCM_ACCESS_RW_NONINTERLEAVED:
	default:
		rtd_pr_adsp_err("[ALSA] unsupport access %d\n", __LINE__);
		return -1;
	}

	/* 16bit BE interleave from TS_OUTPIN */
	dpcm->ai_format = AUDIO_ALSA_FORMAT_16BITS_BE_PCM;

	if (dpcm->ring_init == 1)
		return 0;

	dpcm->ring_init = 1;
    	spin_lock(dpcm->pcm_lock);
	dpcm->running = 0;
    	spin_unlock(dpcm->pcm_lock);
	rtd_pr_adsp_debug("[ALSA] start_threshold = %lx\n", runtime->start_threshold);
	rtd_pr_adsp_debug("[ALSA] stop_threshold = %lx\n", runtime->stop_threshold);
	rtd_pr_adsp_debug("[ALSA] avail_min = %lx\n", runtime->control->avail_min);
	rtd_pr_adsp_debug("[ALSA] buffer_size = %lx\n", runtime->buffer_size);
	rtd_pr_adsp_debug("[ALSA] period_size = %lx\n", runtime->period_size);
	rtd_pr_adsp_debug("[ALSA] sample rate = %d\n", runtime->rate);

	/*
	substream->ops->silence = NULL;
	substream->ops->copy = NULL;
	*/

	/* prepare according to format */
	switch (dpcm->ai_format) {
	case AUDIO_ALSA_FORMAT_24BITS_BE_PCM:
	case AUDIO_ALSA_FORMAT_16BITS_BE_PCM:
		if (rtk_snd_capture_prepare_BE(substream)) {
			rtd_pr_adsp_err("[ALSA] %d fail\n", __LINE__);
			return -ENOMEM;
		}
		break;
	case AUDIO_ALSA_FORMAT_24BITS_LE_LPCM:
	case AUDIO_ALSA_FORMAT_16BITS_LE_LPCM:
		rtd_pr_adsp_debug("[ALSA] %d not support\n", __LINE__);
		break;
	default:
		rtd_pr_adsp_debug("[ALSA] %d unknown format\n", __LINE__);
		break;
	}

	return 0;
}

static void rtk_snd_fmt_convert_to_S16LE(struct snd_pcm_substream *substream,
	snd_pcm_uframes_t wp_next, snd_pcm_uframes_t wp, unsigned int adv_min)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_pcm *dpcm = runtime->private_data;
	int i, loop;
	char *p = NULL, temp;

	p = (char *) runtime->dma_area + (dpcm->ring_bak[0].writePtr - dpcm->ring_bak[0].beginAddr);

	switch (runtime->format) {
	case SNDRV_PCM_FORMAT_S16_BE:
		if (wp_next > wp || wp_next == 0) {
			for (i = 0; i < adv_min * runtime->channels; i++) {
				temp = *p;
				*p = *(p+1);
				p++;
				*p = temp;
				p++;
			}
		} else {
			loop = runtime->buffer_size - wp;
			for (i = 0; i < loop * runtime->channels; i++) {
				temp = *p;
				*p = *(p+1);
				p++;
				*p = temp;
				p++;
			}
			p = (char *) runtime->dma_area;
			for (i = 0; i < wp_next * runtime->channels; i++) {
				temp = *p;
				*p = *(p+1);
				p++;
				*p = temp;
				p++;
			}
		}
	break;
	case SNDRV_PCM_FORMAT_S16_LE:
	break;
	default:
		rtd_pr_adsp_err("[ALSA] not support format convert\n");
	break;
	}
}

void rtk_snd_pcm_playback_update_rp(struct rtk_snd_pcm *dpcm)
{
	struct snd_pcm_substream *substream = dpcm->substream;
	struct snd_pcm_runtime *runtime = substream->runtime;

	unsigned int rp_offset_new, rp_offset_old, rp_real, rp_base;
	unsigned int adv_min, nperiods;
	unsigned char *ptr;
	int size;

	rp_base = dpcm->ring_bak[0].beginAddr;
	rp_real = ntohl(dpcm->ring[0].readPtr[0]);
	rp_offset_old = dpcm->pcm_buf_pos % runtime->buffer_size;

	if ((dpcm->audiopath == AUDIO_PATH_DECODER_AO) || (dpcm->audiopath == AUDIO_PATH_AO)) {
		rp_offset_new = (rp_real - rp_base) / dpcm->output_frame_bytes;
		if (runtime->buffer_size != (dpcm->ring_bak[0].size / dpcm->output_frame_bytes)) {
			rtd_pr_adsp_err("[ALSA] buffer_size error\n");
		}
	} else {
		rp_offset_new = (rp_real - rp_base) / dpcm->output_sample_bytes;
		if (runtime->buffer_size != (dpcm->ring_bak[0].size / dpcm->output_sample_bytes)) {
			rtd_pr_adsp_err("[ALSA] buffer_size error\n");
		}
	}

	if (rp_offset_new < rp_offset_old) {
		adv_min = rp_offset_new + runtime->buffer_size - rp_offset_old;

		if ((runtime->access == SNDRV_PCM_ACCESS_MMAP_INTERLEAVED) && runtime->dma_area) {
			ptr = runtime->dma_area + (rp_offset_old * dpcm->output_frame_bytes);
			size = (runtime->buffer_size - rp_offset_old) * dpcm->output_frame_bytes;
			memset(ptr, 0, size);

			ptr = runtime->dma_area;
			size = rp_offset_new * dpcm->output_frame_bytes;
			if (size != 0) {
				memset(ptr, 0, size);
			}
		}
	} else {
		adv_min = rp_offset_new - rp_offset_old;

		if ((runtime->access == SNDRV_PCM_ACCESS_MMAP_INTERLEAVED) && runtime->dma_area) {
			ptr = runtime->dma_area + (rp_offset_old * dpcm->output_frame_bytes);
			size = adv_min * dpcm->output_frame_bytes;

			if (size != 0) {
				memset(ptr, 0, size);
			}
		}
	}

	nperiods = adv_min / runtime->period_size;
	adv_min = nperiods * runtime->period_size; /* update hw_ptr in period_size count */
	dpcm->pcm_buf_pos += adv_min;
	dpcm->pcm_buf_pos %= runtime->boundary;

	if (dpcm->remain_sample < adv_min)
		dpcm->remain_sample = 0;
	else
		dpcm->remain_sample = dpcm->remain_sample - adv_min;
}

void rtk_snd_pcm_playback_update_wp(struct rtk_snd_pcm *dpcm)
{
	struct snd_pcm_substream *substream = dpcm->substream;
	struct snd_pcm_runtime *runtime = substream->runtime;

	dma_addr_t phy_addr;
	snd_pcm_uframes_t wp, wp_next;
	unsigned int adv_min, avail;
	size_t offset = 0, map_size = 0;
	struct device *dev = dpcm->card->card->dev;

	wp = dpcm->appl_ptr % runtime->buffer_size;

	if (dpcm->pcm_buf_pos > runtime->control->appl_ptr) {
		avail = runtime->control->appl_ptr + runtime->boundary - dpcm->pcm_buf_pos;
	} else {
		avail = runtime->control->appl_ptr - dpcm->pcm_buf_pos;
	}

	/* buffer full */
	if (avail >= runtime->buffer_size) {
		if (runtime->control->appl_ptr < 16) {
			dpcm->appl_ptr = runtime->control->appl_ptr + runtime->boundary - 16;
		} else {
			dpcm->appl_ptr = runtime->control->appl_ptr - 16;
		}
	} else {
		dpcm->appl_ptr = runtime->control->appl_ptr;
	}

	dpcm->appl_ptr %= runtime->boundary;
	wp_next = dpcm->appl_ptr % runtime->buffer_size;

	if (wp_next != wp) {
		if (wp_next > wp) {
			dpcm->remain_sample += (wp_next - wp);
			adv_min = (wp_next - wp);
		} else {
			dpcm->remain_sample += (wp_next + runtime->buffer_size - wp);
			adv_min = (wp_next + runtime->buffer_size - wp);
		}
	} else {
		adv_min = 0;
		return ;
	}

	rtk_snd_fmt_convert_to_S16LE(substream, wp_next, wp, adv_min);

	if ((dpcm->audiopath == AUDIO_PATH_DECODER_AO) || (dpcm->audiopath == AUDIO_PATH_AO)) {
		offset = 0;
		map_size = (size_t) (runtime->buffer_size * dpcm->output_frame_bytes);

		phy_addr = dma_map_page(dev, phys_to_page(runtime->dma_addr), offset, map_size, DMA_TO_DEVICE);
		dma_unmap_page(dev, phy_addr, map_size, DMA_TO_DEVICE);

		dpcm->ring_bak[0].writePtr = dpcm->ring_bak[0].beginAddr + (dpcm->appl_ptr % runtime->buffer_size) * dpcm->output_frame_bytes;
		dpcm->ring[0].writePtr = htonl(dpcm->ring_bak[0].writePtr);
	}
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))

#if defined (CONFIG_PCM_USING_HRTIMER)
static void rtk_snd_pcm_freerun_timer_function(unsigned long data)
#else
static void rtk_snd_pcm_freerun_timer_function(struct timer_list *tl)
#endif

#else
static void rtk_snd_pcm_freerun_timer_function(unsigned long data)
#endif
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))

#if defined (CONFIG_PCM_USING_HRTIMER)
	struct rtk_snd_pcm *dpcm = (struct rtk_snd_pcm *)data;
#else
	struct rtk_snd_pcm *dpcm = from_timer(dpcm, tl, timer);
#endif

#else
	struct rtk_snd_pcm *dpcm = (struct rtk_snd_pcm *)data;
#endif
	struct snd_pcm_substream *substream = dpcm->substream;
	struct snd_pcm_runtime *runtime = substream->runtime;

	/* for ALSA data throughput */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	struct timespec64 t;
#else
	struct timeval t;
#endif
	long per_write_count;
	long valid;
	long time_diff, nodata_time_diff;
	int hwptr_diff;  /* for error handle */
	unsigned long flags_spin_lock = 0;

	if (dpcm->running == 0) {
		return;
	}

	spin_lock_irqsave(dpcm->pcm_lock, flags_spin_lock);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	ktime_get_real_ts64(&t);
	dpcm->current_time_ms = t.tv_sec*1000 + t.tv_nsec/1000000;
#else
	do_gettimeofday(&t);
	dpcm->current_time_ms = t.tv_sec*1000 + t.tv_usec/1000;
#endif
	time_diff = dpcm->current_time_ms - dpcm->pre_time_ms;

	if (runtime->dma_area == NULL) {
		goto check_status;
	}

	rtk_snd_pcm_playback_update_rp(dpcm);

	/* calculate the avail size */
	if (dpcm->appl_ptr == runtime->control->appl_ptr)
		goto check_status;

	rtk_snd_pcm_playback_update_wp(dpcm);

check_status:
	/* throughput compute */
	per_write_count = (long)((ntohl(dpcm->ring[0].writePtr)) - dpcm->pre_writePtr);

	if (per_write_count < 0)
		per_write_count += (long)dpcm->buffer_byte_size;

	dpcm->total_data_wb = dpcm->total_data_wb + per_write_count;

	valid = (long)(ntohl(dpcm->ring[0].writePtr)) - ((ntohl(dpcm->ring[0].readPtr[0])));
	if (valid < 0)
		valid += dpcm->buffer_byte_size;

	if (dpcm->max_level < valid)
		dpcm->max_level = valid;

	if (dpcm->min_level > valid)
		dpcm->min_level = valid;

	dpcm->pre_writePtr = ntohl(dpcm->ring[0].writePtr);

	if (valid == 0) {
		if (dpcm->pre_no_datatime_ms == 0) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
			dpcm->pre_no_datatime_ms = t.tv_sec*1000 + t.tv_nsec/1000000;
#else
			dpcm->pre_no_datatime_ms = t.tv_sec*1000 + t.tv_usec/1000;
#endif
		}
	} else {
		if (dpcm->pre_no_datatime_ms != 0) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
			dpcm->current_no_datatime_time_ms = t.tv_sec*1000 + t.tv_nsec/1000000;
#else
			dpcm->current_no_datatime_time_ms = t.tv_sec*1000 + t.tv_usec/1000;
#endif
			nodata_time_diff = dpcm->current_no_datatime_time_ms - dpcm->pre_no_datatime_ms;

			if (nodata_time_diff > 30) {
                if (alsa_debug)
                    rtd_pr_adsp_debug("[ALSA] shared buffer no data over than %ld ms\n", nodata_time_diff);
			}

			if (nodata_time_diff > 50) {
                if (alsa_debug)
                {
                    rtd_pr_adsp_debug("[ALSA] appl_ptr = %ld, appl_ptr = %ld\n",
                            dpcm->appl_ptr,
                            runtime->control->appl_ptr);
                    rtd_pr_adsp_debug("[ALSA] pcm_buf_pos %d, hw_ptr %ld\n",
                            dpcm->pcm_buf_pos,
                            runtime->status->hw_ptr);
                }
			}
			dpcm->pre_no_datatime_ms = 0;
		}
	}

	if (time_diff >= 3000) {
		if (dpcm->max_level != 0) {
            if (alsa_debug)
            {
                rtd_pr_adsp_debug("[ALSA] total_data_wb %ld, %ld~%ld bytes, time %ldms\n",
                        dpcm->total_data_wb,
                        dpcm->min_level,
                        dpcm->max_level,
                        time_diff);
            }
		}

        if (alsa_debug)
        {
		    rtd_pr_adsp_debug("[ALSA] writePtr = %x, readPtr = %x\n",
                    ntohl(dpcm->ring[0].writePtr),
                    ntohl(dpcm->ring[0].readPtr[0]));
            rtd_pr_adsp_debug("[ALSA] pcm_buf_pos %d, hw_ptr %ld, dma_addr %ld\n",
                    dpcm->pcm_buf_pos,
                    runtime->status->hw_ptr,
                    (unsigned long) runtime->dma_addr);
            rtd_pr_adsp_debug("[ALSA] appl_ptr = %ld, appl_ptr = %ld\n",
                    dpcm->appl_ptr, runtime->control->appl_ptr);
        }

		if (runtime->rate != 0) {
			rtd_pr_adsp_info("[ALSA] hw delay: %lu ms, dev %d, is_running %d\n",
				runtime->delay * 1000 / runtime->rate, substream->pcm->device, dpcm->running);
		}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
		ktime_get_real_ts64(&t);
		dpcm->pre_time_ms = t.tv_sec*1000 + t.tv_nsec/1000000;
#else
		do_gettimeofday(&t);
		dpcm->pre_time_ms = t.tv_sec*1000 + t.tv_usec/1000;
#endif
		dpcm->max_level = 0;
		dpcm->min_level = dpcm->buffer_byte_size;
		dpcm->total_data_wb = 0;
	}
	/* End throughput compute */

	hwptr_diff = dpcm->pcm_buf_pos - runtime->status->hw_ptr;
	if(hwptr_diff < 0) {
		hwptr_diff += runtime->boundary;
	}

	if(hwptr_diff >= runtime->buffer_size) {
		dpcm->hwptr_error_times++;

		if(dpcm->hwptr_error_times >= 10) {
			rtd_pr_adsp_debug("[ALSA] reset hwptr from %d to hw_ptr %ld\n", runtime->status->hw_ptr, dpcm->pcm_buf_pos);
			runtime->status->hw_ptr = dpcm->pcm_buf_pos;
			dpcm->hwptr_error_times = 0;
		}
	}

	if (dpcm->running == 0) {
		spin_unlock_irqrestore(dpcm->pcm_lock, flags_spin_lock);
		return;
	} else {
#if defined(CONFIG_PCM_USING_HRTIMER)
		dpcm_timer_start(dpcm);
#else
		mod_timer(&dpcm->timer, dpcm->timer.expires + 1);
#endif
	}

	if (runtime->status->state != SNDRV_PCM_STATE_DRAINING) {
		dpcm->pcm_irq_pos = dpcm->pcm_buf_pos - runtime->status->hw_ptr;
		if (dpcm->pcm_irq_pos < 0) {
			dpcm->pcm_irq_pos = dpcm->pcm_irq_pos + runtime->boundary;
		}

		if ((dpcm->pcm_irq_pos >= runtime->period_size) && !runtime->no_period_wakeup) {
#ifndef CONFIG_CUSTOMER_TV006
			update_hw_delay(substream);
#endif
			spin_unlock_irqrestore(dpcm->pcm_lock, flags_spin_lock);
			snd_pcm_period_elapsed(substream);
			return;
		}
	} else {
		switch (dpcm->flush_state) {
		case RTK_SND_FLUSH_STATE_NONE:
            if (alsa_debug)
                rtd_pr_adsp_debug("[ALSA] draining data start\n");
			dpcm->flush_state = RTK_SND_FLUSH_STATE_FINISH;
		case RTK_SND_FLUSH_STATE_WAIT:
			break;
		case RTK_SND_FLUSH_STATE_FINISH:
			if (dpcm->appl_ptr >= dpcm->pcm_buf_pos) {
				if ((dpcm->appl_ptr - dpcm->pcm_buf_pos) <= 256) {
					dpcm->pcm_buf_pos = dpcm->appl_ptr;
				}
			} else {
				if ((dpcm->appl_ptr - dpcm->pcm_buf_pos + runtime->boundary) <= 256) {
					dpcm->pcm_buf_pos = dpcm->appl_ptr;
				}
			}
			if ((dpcm->pcm_buf_pos == dpcm->appl_ptr) && !runtime->no_period_wakeup) {
                if (alsa_debug)
                    rtd_pr_adsp_debug("[ALSA] draining data done\n");
#ifndef CONFIG_CUSTOMER_TV006
				update_hw_delay(substream);
#endif
				spin_unlock_irqrestore(dpcm->pcm_lock, flags_spin_lock);
				snd_pcm_period_elapsed(substream);
				return;
			}
			break;
		}
	}

	spin_unlock_irqrestore(dpcm->pcm_lock, flags_spin_lock);
}

/* tranfer 24 bit BE to 16 bit LE */
void COPY_FUNC_24to16(int n_frame, void *src, int *ring_rp[8], int num_channels)
{
	int i, j, temp;
	short *p = (short *) src;

	for (i = 0; i < n_frame; ++i) {
		for(j = 0; j < num_channels; ++j){
			temp = *ring_rp[j];
			temp = ENDIAN_CHANGE(temp);
			*p = (short)(temp >> 9);
			p++;
			ring_rp[j]++;
		}
	}
}

/* tranfer 16 bit BE to 16 bit LE */
void COPY_FUNC_16BETo16LE(int n_frame, void *src, int *ring_rp[2], int num_channels)
{
	int i, temp;
	short *p = (short *) src;

	for (i = 0; i < n_frame; ++i) {
		temp = *ring_rp[0];
		temp = ENDIAN_CHANGE(temp);
		*p = (short)(temp&0x0000ffff);
		p++;
		*p = (short)((temp&0xffff0000)>>16);
		p++;
		ring_rp[0]++;
	}
}

/* DMIC0 is in 0, 1 ch */
void COPY_FUNC_DATA_FROM_DMIC0(int n_frame, void *src, int *ring_rp[2], int num_channels)
{
	int i, temp;
	short *p = (short *) src;

	for (i = 0; i < n_frame; ++i) {
		temp = *ring_rp[0];
		*p = (short)(temp&0x0000ffff);
		p++;
		*p = (short)((temp&0xffff0000)>>16);
		p++;
		ring_rp[0] += 3;
	}
}

/* DMIC0, DMIC1 is in 0, 1, 2, 3 ch */
void COPY_FUNC_DATA_FROM_DMIC0_DMIC1(int n_frame, void *src, int *ring_rp[2], int num_channels)
{
	int i, temp;
	short *p = (short *) src;

	for (i = 0; i < n_frame; ++i) {
		temp = *ring_rp[0];
		*p = (short)(temp&0x0000ffff);
		p++;
		*p = (short)((temp&0xffff0000)>>16);
		p++;
		ring_rp[0] += 1;
		temp = *ring_rp[0];
		*p = (short)(temp&0x0000ffff);
		p++;
		*p = (short)((temp&0xffff0000)>>16);
		p++;
		ring_rp[0] += 2;
	}
}

/* REFERENCE is in 4, 5 ch */
void COPY_FUNC_DATA_FROM_REFERENCE(int n_frame, void *src, int *ring_rp[2], int num_channels)
{
	int i, temp;
	short *p = (short *) src;

	for (i = 0; i < n_frame; ++i) {
		ring_rp[0] += 2;
		temp = *ring_rp[0];
		*p = (short)(temp&0x0000ffff);
		p++;
		*p = (short)((temp&0xffff0000)>>16);
		p++;
		ring_rp[0] += 1;
	}
}

/* tranfer 16 bit LE to 32 bit LE, DMIC0, DMIC1 is in 0, 1, 2, 3 ch */
void COPY_FUNC_DATA_FROM_DMIC0_DMIC1_32BIT(int n_frame, void *src, int *ring_rp[2], int num_channels)
{
	int i, temp;
	int *p = (int *) src;

	for (i = 0; i < n_frame; ++i) {
		temp = *ring_rp[0];
		*p = (int) ((temp & 0x0000ffff) << 16);
		p++;
		*p = (int) (temp & 0xffff0000);
		p++;
		ring_rp[0] += 1;
		temp = *ring_rp[0];
		*p = (int) ((temp & 0x0000ffff) << 16);
		p++;
		*p = (int) (temp & 0xffff0000);
		p++;
		ring_rp[0] += 2;
	}
}

/* write zero to dma_area */
#define COPY_ZERO_FUNC(n_frame, p)\
{\
	int i;\
	for (i = 0; i < n_frame; ++i) {\
		*p = (short)(0);\
		p++;\
		*p = (short)(0);\
		p++;\
	} \
}

/* 32 bit function */
/* tranfer 16 bit LE to 32 bit LE, DMIC0 is in 0, 1 ch */
void COPY_FUNC_DATA_FROM_DMIC0_32BIT(int n_frame, void *src, int *ring_rp[2], int num_channels)
{
	int i, temp;
	int *p = (int *) src;

	for (i = 0; i < n_frame; ++i) {
		temp = (int) (*ring_rp[0]);
		*p = (int) ((temp & 0x0000ffff) << 16);
		p++;
		*p = (int) (temp & 0xffff0000);
		p++;
		ring_rp[0] += 3;
	}
}

/* tranfer 16 bit LE to 32 bit LE, REFERENCE is in 4, 5 ch */
void COPY_FUNC_DATA_FROM_REFERENCE_32BIT(int n_frame, void *src, int *ring_rp[2], int num_channels)
{
	int i, temp;
	int *p = (int *) src;

	for (i = 0; i < n_frame; ++i) {
		ring_rp[0] += 2;
		temp = (int) (*ring_rp[0]);
		*p = (int) ((temp & 0x0000ffff) << 16);
		p++;
		*p = (int) (temp & 0xffff0000);
		p++;
		ring_rp[0] += 1;
	}
}

#define COPY_ZERO_FUNC_32BIT(n_frame, p)\
{\
	int i;\
	for (i = 0; i < n_frame; ++i) {\
		*p = (int)(0);\
		p++;\
		*p = (int)(0);\
		p++;\
	} \
}

/*
copy ring buf to dma buf
substream: struct of the pcm stream
nPeriodCound: number of periods that write to runtime->dma_area
write_zero:
	if write_zero = 1,
		ALSA would write zero to dma_area.
	if write_zero = 0,
		ALSA would write n periods to dma_area from AI or AO.
*/


static void rtk_snd_capture_copy(struct snd_pcm_substream *substream,
	long n_period, int write_zero)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_cap_pcm *dpcm = runtime->private_data;
	snd_pcm_uframes_t n_frame = n_period * runtime->period_size;
	snd_pcm_uframes_t dma_wp = dpcm->pcm_buf_pos % runtime->buffer_size;
	snd_pcm_uframes_t wrap_size = runtime->buffer_size - dma_wp;
	int *ring_rp[8];
	long ring_limit[8];
	int i, loop0, loop1, p_frame_bytes, j;
	void *p = NULL;
	int *ip = NULL;
	short *sp = NULL;
	int is_32bits = (runtime->format == SNDRV_PCM_FORMAT_S32_LE);
	void (*COPY_API)(int n_frame, void *p, int *ring_rp[8], int num_channels);

	if (is_32bits) {
		p_frame_bytes = LPCM_2CH_32BITS;
	} else {
		p_frame_bytes = runtime->channels * 2;
	}

	if ((substream->pcm->device == DMIC_DEVICE) && (runtime->channels == 4)) {
		p_frame_bytes = p_frame_bytes * 2;
	}

	if (is_far_field(substream->pcm->device)) {
		if (is_32bits) {
			if (substream->pcm->device == DMIC_DEVICE) {
				if (runtime->channels == 4) {
					COPY_API = COPY_FUNC_DATA_FROM_DMIC0_DMIC1_32BIT;
				} else {
					COPY_API = COPY_FUNC_DATA_FROM_DMIC0_32BIT;
				}
			} else {
				COPY_API = COPY_FUNC_DATA_FROM_REFERENCE_32BIT;
			}
		} else {
			if (substream->pcm->device == DMIC_DEVICE) {
				if (runtime->channels == 4) {
					COPY_API = COPY_FUNC_DATA_FROM_DMIC0_DMIC1;
				} else {
					COPY_API = COPY_FUNC_DATA_FROM_DMIC0;
				}
			} else {
				COPY_API = COPY_FUNC_DATA_FROM_REFERENCE;
			}
		}

		if (write_zero == 0) {
			ring_rp[0] = (int *) (dpcm->ring_p + ntohl(dpcm->ring[0].readPtr[dpcm->rd_idx]) - ntohl(dpcm->ring[0].beginAddr));
			ring_limit[0] = (long) (dpcm->ring_p + ntohl(dpcm->ring[0].size));

			dmac_flush_range(dpcm->ring_p, (void *) ring_limit[0]);
			outer_inv_range(dpcm->ring_p, ring_limit[0]);
		}

		if (n_frame > wrap_size) {
			p = (void *) (runtime->dma_area + p_frame_bytes * dma_wp);
			if (write_zero == 0) {
				if ((long)(ring_rp[0] + (wrap_size * 3)) > ring_limit[0]) {
					loop0 = (ring_limit[0] - (long)ring_rp[0]) / dpcm->input_sample_bytes;
					loop1 = wrap_size - loop0;
					COPY_API(loop0, p, ring_rp, runtime->channels);

					ring_rp[0] = (int *) dpcm->ring_p;
					p = (void *) (runtime->dma_area + p_frame_bytes * (dma_wp + loop0));
					COPY_API(loop1, p, ring_rp, runtime->channels);
				} else {
					COPY_API(wrap_size, p, ring_rp, runtime->channels);
				}
			} else {
				if (is_32bits) {
					ip = (int *) p;
					COPY_ZERO_FUNC_32BIT(n_frame, ip);
				} else {
					sp = (short *) p;
					COPY_ZERO_FUNC(n_frame, sp);
				}
			}

			p = (void *) (runtime->dma_area);
			n_frame -= wrap_size;
			if (write_zero == 0) {
				if ((long)(ring_rp[0] + (n_frame * 3)) > ring_limit[0]) {
					loop0 = (ring_limit[0] - (long)ring_rp[0]) / dpcm->input_sample_bytes;
					loop1 = n_frame - loop0;
					COPY_API(loop0, p, ring_rp, runtime->channels);

					ring_rp[0] = (int *) dpcm->ring_p;
					p = (void *) (runtime->dma_area + p_frame_bytes * loop0);
					COPY_API(loop1, p, ring_rp, runtime->channels);
				} else {
					COPY_API(n_frame, p, ring_rp, runtime->channels);
				}
			} else {
				if (is_32bits) {
					ip = (int *) p;
					COPY_ZERO_FUNC_32BIT(n_frame, ip);
				} else {
					sp = (short *) p;
					COPY_ZERO_FUNC(n_frame, sp);
				}
			}
		} else {
			p = (void *)(runtime->dma_area + p_frame_bytes * dma_wp);
			if (write_zero == 0) {
				if ((long)(ring_rp[0] + n_frame * 3) > ring_limit[0]) {
					loop0 = (ring_limit[0] - (long)ring_rp[0]) / dpcm->input_sample_bytes;
					loop1 = n_frame - loop0;
					COPY_API(loop0, p, ring_rp, runtime->channels);

					ring_rp[0] = (int *) dpcm->ring_p;
					p = (void *) (runtime->dma_area + p_frame_bytes * (dma_wp + loop0));
					COPY_API(loop1, p, ring_rp, runtime->channels);
				} else {
					COPY_API(n_frame, p, ring_rp, runtime->channels);
				}
			} else {
				if (is_32bits) {
					ip = (int *) p;
					COPY_ZERO_FUNC_32BIT(n_frame, ip);
				} else {
					sp = (short *) p;
					COPY_ZERO_FUNC(n_frame, sp);
				}
			}
		}
	} else {
		COPY_API = COPY_FUNC_16BETo16LE;

		if (dpcm->ao_cap_agent != -1) {
			COPY_API = COPY_FUNC_24to16;
		}

		if (write_zero == 0) {
			for (i = 0; i < runtime->channels; ++i) {
				ring_rp[i] = (int *) (phys_to_virt(ntohl(dpcm->ring[i].beginAddr)) + ntohl(dpcm->ring[i].readPtr[0]) - ntohl(dpcm->ring[i].beginAddr));
				ring_limit[i] = (long) (phys_to_virt(ntohl(dpcm->ring[i].beginAddr)) + ntohl(dpcm->ring[i].size));

				dmac_flush_range(phys_to_virt(ntohl(dpcm->ring[i].beginAddr)), (void *) ring_limit[i]);
				outer_inv_range(phys_to_virt(ntohl(dpcm->ring[i].beginAddr)), ring_limit[i]);
			}
		}

		if (n_frame > wrap_size) {
			p = (void *)(runtime->dma_area + p_frame_bytes * dma_wp);
			if (write_zero == 0) {
				if ((long)(ring_rp[0] + wrap_size) > ring_limit[0]) {
					loop0 = (ring_limit[0] - (long)ring_rp[0]) >> 2;
					loop1 = wrap_size - loop0;
					COPY_API(loop0, p, ring_rp, runtime->channels);

					for(j=0; j < runtime->channels; ++j){
						ring_rp[j] = (int *) phys_to_virt(ntohl(dpcm->ring[j].beginAddr));
					}
					p = (void *) (runtime->dma_area + p_frame_bytes * (dma_wp + loop0));
					COPY_API(loop1, p, ring_rp, runtime->channels);
				} else {
					COPY_API(wrap_size, p, ring_rp, runtime->channels);
				}
			} else {
				sp = (short *) p;
				COPY_ZERO_FUNC(n_frame, sp);
			}

			p = (void *)(runtime->dma_area);
			n_frame -= wrap_size;
			if (write_zero == 0) {
				if ((long)(ring_rp[0] + n_frame) > ring_limit[0]) {
					loop0 = (ring_limit[0] - (long)ring_rp[0]) >> 2;
					loop1 = n_frame - loop0;
					COPY_API(loop0, p, ring_rp, runtime->channels);

					for(j=0; j < runtime->channels; ++j){
						ring_rp[j] = (int *) phys_to_virt(ntohl(dpcm->ring[j].beginAddr));
					}
					p = (void *) (runtime->dma_area + p_frame_bytes * loop0);
					COPY_API(loop1, p, ring_rp, runtime->channels);
				} else {
					COPY_API(n_frame, p, ring_rp, runtime->channels);
				}
			} else {
				sp = (short *) p;
				COPY_ZERO_FUNC(n_frame, sp);
			}
		} else {
			p = (void *)(runtime->dma_area + p_frame_bytes * dma_wp);
			if (write_zero == 0) {
				if ((long)(ring_rp[0] + n_frame) > ring_limit[0]) {
					loop0 = (ring_limit[0] - (long)ring_rp[0]) >> 2;
					loop1 = n_frame - loop0;
					COPY_API(loop0, p, ring_rp, runtime->channels);

					for(j=0; j < runtime->channels; ++j){
						ring_rp[j] = (int *) phys_to_virt(ntohl(dpcm->ring[j].beginAddr));
					}
					p = (void *) (runtime->dma_area + p_frame_bytes * (dma_wp + loop0));
					COPY_API(loop1, p, ring_rp, runtime->channels);
				} else {
					COPY_API(n_frame, p, ring_rp, runtime->channels);
				}
			} else {
				sp = (short *) p;
				COPY_ZERO_FUNC(n_frame, sp);
			}
		}
	}
}

static long ring_valid_data(long ring_base, long ring_limit,
	long ring_rp, long ring_wp)
{
	if (ring_wp >= ring_rp)
		return ring_wp - ring_rp;
	else
		return (ring_limit - ring_base) - (ring_rp - ring_wp);
}

static long rtk_snd_get_ring_data(RINGBUFFER_HEADER *p_ring_be, int rd_idx)
{
	long base, limit, rp, wp, data_size;
	base = ntohl(p_ring_be->beginAddr);
	limit = ntohl(p_ring_be->beginAddr) + ntohl(p_ring_be->size);
	wp = (long)(ntohl(p_ring_be->writePtr));
	rp = (long)(ntohl(p_ring_be->readPtr[rd_idx]));
	data_size = ring_valid_data(base, limit, rp, wp);
	return data_size;
}

/* 24BE AI 2ch input */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
static void rtk_snd_pcm_capture_timer_function(struct timer_list *tl)
#else
static void rtk_snd_pcm_capture_timer_function(unsigned long data)
#endif
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	struct rtk_snd_cap_pcm *dpcm = from_timer(dpcm, tl, timer);
#else
	struct rtk_snd_cap_pcm *dpcm = (struct rtk_snd_cap_pcm *)data;
#endif
	struct snd_pcm_substream *substream = dpcm->substream;
	struct snd_pcm_runtime *runtime = substream->runtime;

	snd_pcm_uframes_t n_data_frame;
	long n_data_size;
	unsigned int n_period = 0;
	int i;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	struct timespec64 t;
#else
	struct timeval t;
#endif
	int ch;

	spin_lock(dpcm->pcm_lock);
	if (is_far_field(substream->pcm->device)) {
		if ((dpcm->rd_idx == 1) && is_far_field(substream->pcm->device) && (dpcm->ring[0].numOfReadPtr == htonl(1))) {
			rtd_pr_adsp_info("[ALSA][FAR FIELD] change rd_idx to 0, sync rp[1] %x to rp[0] %x\n");
			dpcm->card->far_field_ring_audio->readPtr[0] = dpcm->card->far_field_ring_audio->readPtr[1];
			dpcm->rd_idx = 0;
		}

		ch = 1;
		n_data_size = rtk_snd_get_ring_data(dpcm->ring, dpcm->rd_idx);
	} else {
		ch = runtime->channels;
		n_data_size = rtk_snd_get_ring_data(dpcm->ring, 0);
	}

	/* get AI outring data size */
	n_data_frame = n_data_size / dpcm->input_sample_bytes;

	if (dpcm->running) {
		mod_timer(&dpcm->timer, dpcm->period_jiffies + jiffies);
	}

	if (n_data_frame >= runtime->period_size) {
		n_period = n_data_frame / runtime->period_size;
		if (n_period == runtime->periods)
			n_period--;

		/* copy data from ai ring to dma */
		rtk_snd_capture_copy(substream, n_period, 0);

		/* update rp */
		dpcm->pcm_buf_pos += n_period * runtime->period_size;

        if (alsa_debug)
            rtd_pr_adsp_debug("[ALSA] pcm_buf_pos= %x\n", dpcm->pcm_buf_pos);

		if (is_far_field(substream->pcm->device)) {
			for (i = 0; i < ch; i++) {
				dpcm->ring[i].readPtr[dpcm->rd_idx] =
					htonl((ulong) ntohl(dpcm->ring[i].beginAddr) + (dpcm->pcm_buf_pos % runtime->buffer_size) * dpcm->input_sample_bytes);
			}
		} else {
			for (i = 0; i < ch; i++) {
				dpcm->ring[i].readPtr[0] =
					htonl((ulong) ntohl(dpcm->ring[i].beginAddr) + (dpcm->pcm_buf_pos % runtime->buffer_size) * dpcm->input_sample_bytes);
			}
		}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
		ktime_get_real_ts64(&t);
		dpcm->last_time_ms = t.tv_sec*1000 + t.tv_nsec/1000000;
#else
		do_gettimeofday(&t);
		dpcm->last_time_ms = t.tv_sec*1000 + t.tv_usec/1000;
#endif

		/* update ALSA runtime->hw_ptr */
		spin_unlock(dpcm->pcm_lock);
		snd_pcm_period_elapsed(substream);
		return;
	} else {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
		ktime_get_real_ts64(&t);
		dpcm->current_time_ms = t.tv_sec*1000 + t.tv_nsec/1000000;
#else
		do_gettimeofday(&t);
		dpcm->current_time_ms = t.tv_sec*1000 + t.tv_usec/1000;
#endif
		if ((dpcm->current_time_ms - dpcm->last_time_ms) >= 5000) {
            if (alsa_debug)
                rtd_pr_adsp_debug("[ALSA] no data in %ld millisecond\n", dpcm->current_time_ms - dpcm->last_time_ms);
			dpcm->last_time_ms = dpcm->current_time_ms;

			/* write zero data to dma_area
			when FW doesn't have data in 5 seconds */
			rtk_snd_capture_copy(substream, 1, 1);

			/* update rp */
			dpcm->pcm_buf_pos += runtime->period_size;
            if (alsa_debug)
                rtd_pr_adsp_debug("[ALSA] pcm_buf_pos = %x\n", dpcm->pcm_buf_pos);

			/* update ALSA runtime->status->hw_ptr */
			spin_unlock(dpcm->pcm_lock);
			snd_pcm_period_elapsed(substream);
			return;
		}
	}

	spin_unlock(dpcm->pcm_lock);
}

/* calculate the target DMA-buffer position to be written/read */
void *rtk_snd_get_dma_ptr(struct snd_pcm_runtime *runtime,
			   int channel, unsigned long hwoff)
{
	return runtime->dma_area + hwoff +
		channel * (runtime->dma_bytes / runtime->channels);
}

int rtk_snd_playback_copy_user(struct snd_pcm_substream *substream,
			      int channel, unsigned long hwoff,
			      void *buf, unsigned long bytes)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_pcm *dpcm = runtime->private_data;
	unsigned long flags;

	if (copy_from_user(rtk_snd_get_dma_ptr(substream->runtime, channel, hwoff),
			   (void __user *)buf, bytes)) {
		return -EFAULT;
	}

	spin_lock_irqsave(dpcm->pcm_lock, flags);
	rtk_snd_pcm_playback_update_rp(dpcm);

	if (dpcm->appl_ptr != runtime->control->appl_ptr) {
		rtk_snd_pcm_playback_update_wp(dpcm);
	}
	spin_unlock_irqrestore(dpcm->pcm_lock, flags);

	return 0;
}

static snd_pcm_uframes_t rtk_snd_playback_pointer(
	struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_pcm *dpcm = runtime->private_data;
	snd_pcm_uframes_t pos;
	unsigned long flags;

	spin_lock_irqsave(dpcm->pcm_lock, flags);

	rtk_snd_pcm_playback_update_rp(dpcm);
	if (dpcm->appl_ptr != runtime->control->appl_ptr) {
		rtk_snd_pcm_playback_update_wp(dpcm);
	}

	if (!runtime->no_period_wakeup) {
		update_hw_delay(substream);
	}
	pos = (snd_pcm_uframes_t) dpcm->pcm_buf_pos % runtime->buffer_size;
	spin_unlock_irqrestore(dpcm->pcm_lock, flags);
	return pos;
}

static snd_pcm_uframes_t rtk_snd_capture_pointer(
	struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_cap_pcm *dpcm = runtime->private_data;
	snd_pcm_uframes_t pos;
	unsigned long flags;
	spin_lock_irqsave(dpcm->pcm_lock, flags);
	pos = (snd_pcm_uframes_t) dpcm->pcm_buf_pos % runtime->buffer_size;
	spin_unlock_irqrestore(dpcm->pcm_lock, flags);
	return pos;
}

static struct snd_pcm_hardware rtk_snd_playback = {
	.info =	(SNDRV_PCM_INFO_MMAP |
			SNDRV_PCM_INFO_INTERLEAVED |
			SNDRV_PCM_INFO_NONINTERLEAVED |
			SNDRV_PCM_INFO_MMAP_VALID |
			SNDRV_PCM_INFO_NO_PERIOD_WAKEUP),
	.formats = (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S16_BE),
	.rates = USE_RATE,
	.rate_min = USE_RATE_MIN,
	.rate_max =	USE_RATE_MAX,
	.channels_min =	1,
	.channels_max =	8,
	.buffer_bytes_max =	MAX_BUFFER_SIZE,
	.period_bytes_min =	MIN_PERIOD_SIZE,
	.period_bytes_max =	MAX_PERIOD_SIZE,
	.periods_min = USE_PERIODS_MIN,
	.periods_max = USE_PERIODS_MAX,
	.fifo_size = 32,
};

static struct snd_pcm_hardware rtk_snd_capture = {
	.info =	SNDRV_PCM_INFO_INTERLEAVED |
			SNDRV_PCM_INFO_NONINTERLEAVED,
	.formats = USE_FORMATS,
	.rates = (SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_44100 | SNDRV_PCM_RATE_48000),
	.rate_min =	16000,
	.rate_max =	48000,
	.channels_min =	USE_CHANNELS_MIN,
	.channels_max =	USE_CHANNELS_MAX,
	.buffer_bytes_max =	MAX_BUFFER_SIZE,
	.period_bytes_min =	MIN_PERIOD_SIZE,
	.period_bytes_max =	MAX_PERIOD_SIZE,
	.periods_min = USE_PERIODS_MIN,
	.periods_max = USE_PERIODS_MAX,
	.fifo_size = 32,
};

static void rtk_snd_runtime_free(struct snd_pcm_runtime *runtime)
{
	struct rtk_snd_pcm *dpcm = runtime->private_data;
	rtd_pr_adsp_debug("[ALSA] %s\n", __func__);

	if (dpcm == NULL) {
		return;
	}

	dvr_free(dpcm->vir_addr);
	runtime->private_data = NULL;
}

static void snd_card_capture_runtime_free(struct snd_pcm_runtime *runtime)
{
	struct snd_card *card;
	struct rtk_snd_cap_pcm *dpcm = runtime->private_data;
	struct snd_pcm_substream *substream;
	int do_free = 1;

	if (dpcm == NULL) {
		return;
	}

    substream = dpcm->substream;
	card = dpcm->card->card;

	if (dpcm->pcm_lock != NULL) {
		kfree(dpcm->pcm_lock);
		dpcm->pcm_lock = NULL;
	}

	if (is_far_field(substream->pcm->device)) {
		mutex_lock(&dpcm->card->far_field_lock);
		if (dpcm->card->reference_count == 2) {
			/* other device still used */
			dpcm->card->reference_count = 1;
			do_free = 0;
		} else if (dpcm->card->reference_count == 1) {
			do_free = 1;
			dpcm->card->reference_count = 0;
		}
		mutex_unlock(&dpcm->card->far_field_lock);
	} else {
		do_free = 1;
	}

	if (dpcm->ring_p && do_free) {
		dvr_free(dpcm->ring_p);
		dpcm->card->far_field_vir = NULL;
	}

	dvr_free(dpcm->vir_addr);
	runtime->private_data = NULL;
}

static int rtk_snd_playback_mmap(struct snd_pcm_substream *substream, struct vm_area_struct *area)
{
	int ret = -ENOMEM;
	struct snd_pcm_runtime *runtime = NULL;
	struct snd_card_mars *mars = NULL;

	rtd_pr_adsp_info("[ALSA][%s]\n", __func__);

	if ((!substream) || (!substream->runtime)) {
		rtd_pr_adsp_err("[ALSA][%s] status wrong\n", __func__);
		return -EINVAL;
	}

	if (is_aaduio_device(substream->pcm->device)) {
		runtime = substream->runtime;
		mars = (struct snd_card_mars *) substream->pcm->card->private_data;

		ret = rtkaudio_ioctl_dmabuf_mmap(mars->snd_dmafd_handle[substream->pcm->device], area);
	}

	return ret;
}

int rtk_snd_dmabuf_allocate_pages(struct snd_pcm_substream *substream, size_t size)
{
	struct snd_dma_buffer *dmab = NULL;
	struct snd_pcm_runtime *runtime = NULL;
	struct snd_card_mars *mars = NULL;

	if ((!substream) || (!substream->runtime)) {
		rtd_pr_adsp_err("[ALSA][%s] status wrong\n", __func__);
		return -EINVAL;
	}
	runtime = substream->runtime;
	mars = (struct snd_card_mars *) substream->pcm->card->private_data;

	if (snd_BUG_ON(substream->dma_buffer.dev.type == SNDRV_DMA_TYPE_UNKNOWN)) {
		rtd_pr_adsp_err("[ALSA][%s] dev type is UNKNOWN\n", __func__);
		return -EINVAL;
	}

	dmab = kzalloc(sizeof(*dmab), GFP_KERNEL);
	if (!dmab) {
		rtd_pr_adsp_err("[ALSA][%s] dma buffer allocate failed\n", __func__);
		return -ENOMEM;
	}
	dmab->dev = substream->dma_buffer.dev;

	mars->snd_dmafd_handle[substream->pcm->device] = rtkaudio_dmabuf_fd_allocate(size);
	if (mars->snd_dmafd_handle[substream->pcm->device] < 0) {
		kfree(dmab);
		rtd_pr_adsp_err("[ALSA][%s] get dmabuf fd failed\n", __func__);
		return -ENOMEM;
	}

	dmab->area = rtkaudio_dmabuf_get_vaddr(mars->snd_dmafd_handle[substream->pcm->device]);
	dmab->addr = dvr_to_phys(dmab->area);

	rtd_pr_adsp_info("[ALSA][%s] get dmabuf fd %d, v %lx, p %lx\n",
		__func__, mars->snd_dmafd_handle[substream->pcm->device], (unsigned long) dmab->area, dmab->addr);

	snd_pcm_set_runtime_buffer(substream, dmab);
	runtime->dma_bytes = size;

	return 0;
}

int rtk_snd_allocate_pages(struct snd_pcm_substream *substream, size_t size) {
	struct snd_dma_buffer *dmab = NULL;
	struct snd_pcm_runtime *runtime = NULL;

	if ((!substream) || (!substream->runtime)) {
		rtd_pr_adsp_err("[ALSA][%s] status wrong\n", __func__);
		return -EINVAL;
	}
	runtime = substream->runtime;

	if (snd_BUG_ON(substream->dma_buffer.dev.type == SNDRV_DMA_TYPE_UNKNOWN)) {
		rtd_pr_adsp_err("[ALSA][%s] dev type is UNKNOWN\n", __func__);
		return -EINVAL;
	}

	dmab = kzalloc(sizeof(*dmab), GFP_KERNEL);
	if (!dmab) {
		rtd_pr_adsp_err("[ALSA][%s] dma buffer allocate failed\n", __func__);
		return -ENOMEM;
	}
	dmab->dev = substream->dma_buffer.dev;

	dmab->area = dvr_malloc_specific(size, GFP_DCU1);
	if (!dmab->area) {
		rtd_pr_adsp_err("snd-malloc: SNDRV_RTK_DMA_TYPE failed\n");
        	kfree(dmab);
		return -ENOMEM;
	} else {
		dmab->addr = dvr_to_phys(dmab->area);
	}

	snd_pcm_set_runtime_buffer(substream, dmab);
	runtime->dma_bytes = size;

	return 0;
}

static int rtk_snd_playback_hw_params(
	struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *hw_params)
{
	int err = 0;
	unsigned int bits;
	int dev;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_pcm *dpcm = runtime->private_data;
    rtd_pr_adsp_info("[ALSA] [%s] size %x\n", __func__, params_buffer_bytes(hw_params));

	if (rtk_snd_open(substream) < 0)
		return -ENOMEM;

	if (is_aaduio_device(substream->pcm->device)) {
		err = rtk_snd_dmabuf_allocate_pages(substream, params_buffer_bytes(hw_params));
		if (err) {
			rtd_pr_adsp_err("[ALSA][%s] dmabuf allocate failed %x\n", __func__, params_buffer_bytes(hw_params));
			return -ENOMEM;
		}
	} else {
		if (substream->dma_buffer.dev.type == SNDRV_RTK_DMA_TYPE) {
			err = rtk_snd_allocate_pages(substream, params_buffer_bytes(hw_params));
		} else {
			err = snd_pcm_lib_malloc_pages(substream, params_buffer_bytes(hw_params));
		}

		if (err) {
			rtd_pr_adsp_err("[ALSA][%s] allocate failed %x\n", __func__, params_buffer_bytes(hw_params));
			return -ENOMEM;
		}
	}

	rtd_pr_adsp_info("[ALSA] acs %d, fmt %d, ch %d, rate %d\n",
		params_access(hw_params),
		params_format(hw_params),
		params_channels(hw_params),
		params_rate(hw_params));

	rtd_pr_adsp_info("[ALSA] ps %d, p %d, bs %d, bits %d\n",
		params_period_size(hw_params),
		params_periods(hw_params),
		params_buffer_size(hw_params),
		snd_pcm_format_physical_width(params_format(hw_params)));

	/* add for usbmic delay */
#if 1

	runtime->access = params_access(hw_params);
	runtime->format = params_format(hw_params);
	runtime->subformat = params_subformat(hw_params);
	runtime->channels = params_channels(hw_params);
	runtime->rate = params_rate(hw_params);
	runtime->period_size = params_period_size(hw_params);
	runtime->periods = params_periods(hw_params);
	runtime->buffer_size = params_buffer_size(hw_params);

	bits = snd_pcm_format_physical_width(runtime->format);
	runtime->sample_bits = bits;
	bits *= runtime->channels;
	runtime->frame_bits = bits;

	if (dpcm->ring_init == 1) {
		if (dpcm->periods != runtime->periods) {
			rtd_pr_adsp_err("[ALSA] periods different\n");
			dpcm->ring_init = 0;
		}
		if (dpcm->period_size != runtime->period_size) {
			rtd_pr_adsp_err("[ALSA] period_size different\n");
			dpcm->ring_init = 0;
		}
		if (dpcm->buffer_size != runtime->buffer_size) {
			rtd_pr_adsp_err("[ALSA] buffer_size different\n");
			dpcm->ring_init = 0;
		}
		if (dpcm->access != runtime->access) {
			rtd_pr_adsp_err("[ALSA] access different\n");
			dpcm->ring_init = 0;
		}
		if (dpcm->format != runtime->format) {
			rtd_pr_adsp_err("[ALSA] format different\n");
			dpcm->ring_init = 0;
		}
		if (dpcm->channels != runtime->channels) {
			rtd_pr_adsp_err("[ALSA] channels different\n");
			dpcm->ring_init = 0;
		}
		if (dpcm->ring_bak[0].beginAddr != (unsigned long) runtime->dma_addr) {
			rtd_pr_adsp_err("[ALSA] dma_addr different\n");
			dpcm->ring_init = 0;
		}
		if (dpcm->ring_init == 0) {
			rtk_snd_flush(substream);
			rtk_snd_pause(substream);
			rtk_snd_close(substream);
		}
	}

	dpcm->pcm_jiffie = runtime->rate / HZ;
	dpcm->pcm_size = snd_pcm_lib_buffer_bytes(substream);
	dpcm->pcm_count = snd_pcm_lib_period_bytes(substream);
	dpcm->flush_state = RTK_SND_FLUSH_STATE_NONE;
	dpcm->periods = runtime->periods;
	dpcm->buffer_size = runtime->buffer_size;
	dpcm->access = runtime->access;
	dpcm->channels = runtime->channels;
	dpcm->format = runtime->format;
	dpcm->period_size = runtime->period_size;
	dpcm->sample_bits = runtime->sample_bits;
	dpcm->rate = runtime->rate;
	dpcm->remain_sample = 0;
	dpcm->running = 0;
	dpcm->pcm_buf_pos = 0;
	dpcm->pcm_irq_pos = 0;
	dpcm->appl_ptr = 0;
	dpcm->ring_init = 1;

	dpcm->period_jiffies =
		runtime->period_size * 100 / runtime->rate * 1 / 20;
	if (dpcm->period_jiffies == 0)
		dpcm->period_jiffies = 1;

	dpcm->hwptr_error_times = 0;

	switch (runtime->access) {
		case SNDRV_PCM_ACCESS_MMAP_INTERLEAVED:
			rtd_pr_adsp_debug("[ALSA] MMAP_INTERLEAVED\n");
#ifdef USE_DECODER
			/*old flow ALSA -> Decoder -> AO */
			dpcm->audiopath = AUDIO_PATH_DECODER_AO;
#else
			/* new flow ALSA -> AO */
			dpcm->audiopath = AUDIO_PATH_AO;
#endif
			break;
		case SNDRV_PCM_ACCESS_MMAP_NONINTERLEAVED:
			rtd_pr_adsp_debug("[ALSA] MMAP_NONINTERLEAVED\n");
			dpcm->audiopath = AUDIO_PATH_AO_BYPASS;
			break;
		case SNDRV_PCM_ACCESS_RW_INTERLEAVED:
			rtd_pr_adsp_debug("[ALSA] RW_INTERLEAVED\n");
#ifdef USE_DECODER
			/*old flow ALSA -> Decoder -> AO */
			dpcm->audiopath = AUDIO_PATH_DECODER_AO;
#else
			/* new flow ALSA -> AO */
			dpcm->audiopath = AUDIO_PATH_AO;
#endif
			break;
		case SNDRV_PCM_ACCESS_RW_NONINTERLEAVED:
			rtd_pr_adsp_debug("[ALSA] RW_NONINTERLEAVED\n");
			dpcm->audiopath = AUDIO_PATH_AO_BYPASS;
			break;
		default:
			rtd_pr_adsp_debug("[ALSA][%d] unsupport mode\n", __LINE__);
			return -1;
			break;
	}

	if (dpcm->audiopath == AUDIO_PATH_AO) {
		switch (runtime->format) {
#ifdef USE_DECODER
		case SNDRV_PCM_FORMAT_S32_BE:
		case SNDRV_PCM_FORMAT_S24_BE:
		case SNDRV_PCM_FORMAT_S24_LE:
		case SNDRV_PCM_FORMAT_S8:
#endif
		case SNDRV_PCM_FORMAT_S16_BE:
		case SNDRV_PCM_FORMAT_S16_LE:
			break;
		default:
			rtd_pr_adsp_debug("[ALSA] unsupport format\n");
			return -1;
			break;
		}
	}

	dev = substream->pcm->device;

	rtd_pr_adsp_debug("[ALSA] start_threshold = %lx\n", runtime->start_threshold);
	rtd_pr_adsp_debug("[ALSA] stop_threshold = %lx\n", runtime->stop_threshold);
	rtd_pr_adsp_debug("[ALSA] avail_min = %lx\n", runtime->control->avail_min);
	rtd_pr_adsp_debug("[ALSA] buffer_size = %lx\n", runtime->buffer_size);
	rtd_pr_adsp_debug("[ALSA] period_size = %lx\n", runtime->period_size);
	rtd_pr_adsp_debug("[ALSA] sample rate = %d\n", runtime->rate);

	if (dpcm->audiopath == AUDIO_PATH_DECODER_AO) {
		dpcm->output_frame_bytes = frames_to_bytes(runtime, 1);
		dpcm->output_sample_bytes = samples_to_bytes(runtime, 1);
		dpcm->input_sample_bytes = samples_to_bytes(runtime, 1);

		rtk_snd_create_decoder_agent(substream);

		rtd_pr_adsp_debug("[ALSA] dec_agent = %d, dec_pin_id = %d\n",
			dpcm->dec_agent, dpcm->dec_pin_id);

		rtk_snd_init_decoder_ring(substream);
		if (rtk_snd_init_connect_decoder_ao(substream)) {
			rtd_pr_adsp_debug("[ALSA] %s %d\n", __func__, __LINE__);
			return -ENOMEM;
		}

		/*
		*	AO Pause
		*	Decoder Pause
		*	Decoder Flush
		*	Write Inband data
		*/
		rtk_snd_init_decoder_info(substream);
		rtk_snd_set_ao_flashpin_volume(substream);
		rtk_snd_resume(substream);

		/*
		substream->ops->silence = NULL;
		substream->ops->copy = snd_card_std_copy;
		*/

	} else if (dpcm->audiopath == AUDIO_PATH_AO_BYPASS) {

		rtd_pr_adsp_debug("[ALSA] AO BYPASS\n");
		dpcm->agent_id = dpcm->ao_agent;
		dpcm->pin_id = dpcm->ao_pin_id;
		dpcm->extend_to_32be_ratio = 0;
		dpcm->output_frame_bytes = samples_to_bytes(runtime, 1);
		dpcm->output_sample_bytes = samples_to_bytes(runtime, 1);
		dpcm->input_sample_bytes = samples_to_bytes(runtime, 1);

		/*
		substream->ops->copy = NULL;
		substream->ops->silence = NULL;
		*/

		if (rtk_snd_init_ao_ring(substream))
			return -ENOMEM;

		if (rtk_snd_ao_info(substream)) {
			rtd_pr_adsp_err("[ALSA] %s %d\n", __func__, __LINE__);
			return -ENOMEM;
		}
		rtk_snd_resume(substream);
	} else if (dpcm->audiopath == AUDIO_PATH_AO) {
		rtd_pr_adsp_debug("[ALSA] AO interleaved\n");
		dpcm->agent_id = dpcm->ao_agent;
		dpcm->pin_id = dpcm->ao_pin_id;
		dpcm->card->ao_pin_id[dev] = dpcm->ao_pin_id;
		dpcm->volume = dpcm->card->ao_flash_volume[dev];

		dpcm->output_frame_bytes = frames_to_bytes(runtime, 1);
		dpcm->output_sample_bytes = samples_to_bytes(runtime, 1);
		dpcm->input_sample_bytes = samples_to_bytes(runtime, 1);

		/*
		substream->ops->copy = NULL;
		substream->ops->silence = NULL;
		*/

		if (rtk_snd_init_ao_ring(substream)) {
			rtd_pr_adsp_err("[ALSA] %d Fail!!\n", __LINE__);
			return -ENOMEM;
		}

		if (rtk_snd_ao_info(substream)) {
			rtd_pr_adsp_err("[ALSA] %d Fail!!\n", __LINE__);
			return -ENOMEM;
		}

		rtk_snd_set_ao_flashpin_volume(substream);
		rtk_snd_resume(substream);
	}

#endif

	return err;
}

static int rtk_snd_capture_hw_params(
	struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *hw_params)
{
	int err = 0;
    if (alsa_debug)
        rtd_pr_adsp_debug("[ALSA] [%s] size %x\n", __func__, params_buffer_bytes(hw_params));

	if (substream->pcm->device == DMIC_DEVICE) {
		if ((params_channels(hw_params) != 2) && (params_channels(hw_params) != 4)) {
			rtd_pr_adsp_err("[ALSA] [%s] wrong channel number! dev %d, chnum %d\n", __func__, substream->pcm->device, params_channels(hw_params));
			err = -ENOSYS;
			goto exit;
		}
	} else {
		if (params_channels(hw_params) > 8) {
			rtd_pr_adsp_err("[ALSA] [%s] wrong channel number! dev %d, chnum %d\n", __func__, substream->pcm->device, params_channels(hw_params));
			err = -ENOSYS;
			goto exit;
		}
	}

	if (substream->dma_buffer.dev.type == SNDRV_RTK_DMA_TYPE) {
		err = rtk_snd_allocate_pages(substream, params_buffer_bytes(hw_params));
	} else {
		err = snd_pcm_lib_malloc_pages(substream, params_buffer_bytes(hw_params));
	}

exit:
	return err;
}

static int rtk_snd_dmabuf_free_pages(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = NULL;

	if ((!substream) || (!substream->runtime)) {
		return -EINVAL;
	}

	runtime = substream->runtime;
	if (runtime->dma_area == NULL)
		return 0;
	if (runtime->dma_buffer_p != &substream->dma_buffer) {
		/* it's a newly allocated buffer.  release it now. */
		kfree(runtime->dma_buffer_p);
	}
	snd_pcm_set_runtime_buffer(substream, NULL);

	return 0;
}

static int rtk_snd_free_pages(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = NULL;
	struct snd_dma_buffer *dmab = NULL;

	if ((!substream) || (!substream->runtime)) {
		return -EINVAL;
	}
	runtime = substream->runtime;
	if (runtime->dma_area == NULL)
		return 0;
	if (runtime->dma_buffer_p != &substream->dma_buffer) {
		/* it's a newly allocated buffer.  release it now. */
		dmab = runtime->dma_buffer_p;
		dvr_free(dmab->area);
		kfree(runtime->dma_buffer_p);
	}
	snd_pcm_set_runtime_buffer(substream, NULL);

	return 0;
}

static int rtk_snd_playback_hw_free(struct snd_pcm_substream *substream)
{
	int ret;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_pcm *dpcm = runtime->private_data;

    if (alsa_debug)
        rtd_pr_adsp_debug("[ALSA] [%s]\n", __func__);

	ret = 0;

	if (dpcm == NULL || dpcm->ring_init == 0) {
		if (dpcm && dpcm->ao_pin_id != 0) {
			ret = rtk_snd_release_flashpin_id(dpcm->card->card, dpcm->ao_pin_id);
#ifdef USE_FIXED_AO_PINID
			if (ret < 0) {
				release_error[substream->pcm->device] = dpcm->ao_pin_id;
			}

			dpcm->ao_pin_id = 0;
			used_ao_pin[substream->pcm->device] = 0;
			rtd_pr_adsp_debug("[ALSA] reset used_ao_pin[%d] %d\n",
				substream->pcm->device, used_ao_pin[substream->pcm->device]);
#endif
		}
		goto exit;
	}
	ret = rtk_snd_flush(substream);

#ifdef USE_FIXED_AO_PINID
	if (ret < 0)
		flush_error[substream->pcm->device] = 1;
#endif

	ret = rtk_snd_pause(substream);

#ifdef USE_FIXED_AO_PINID
	if (ret < 0)
		pause_error[substream->pcm->device] = 1;
#endif

	ret = rtk_snd_close(substream);

#ifdef USE_FIXED_AO_PINID
	if (ret < 0)
		close_error[substream->pcm->device] = 1;
#endif

	ret = rtk_snd_release_flashpin_id(dpcm->card->card, dpcm->ao_pin_id);
	dpcm->card->ao_pin_id[substream->pcm->device] = 0;
#ifdef USE_FIXED_AO_PINID
	if (ret < 0)
		release_error[substream->pcm->device] = dpcm->ao_pin_id;

	dpcm->ao_pin_id = 0;
	used_ao_pin[substream->pcm->device] = 0;
#endif

exit:

	if (is_aaduio_device(substream->pcm->device)) {
		ret = rtk_snd_dmabuf_free_pages(substream);
	} else {
		if (substream->dma_buffer.dev.type == SNDRV_RTK_DMA_TYPE) {
			ret = rtk_snd_free_pages(substream);
		} else {
			ret = snd_pcm_lib_free_pages(substream);
		}
	}

	return ret;
}

static int rtk_snd_capture_hw_free(struct snd_pcm_substream *substream)
{
	int ret = 0;
	rtd_pr_adsp_debug("[ALSA] [%s]\n", __func__);

	if (substream->dma_buffer.dev.type == SNDRV_RTK_DMA_TYPE) {
		ret = rtk_snd_free_pages(substream);
	} else {
		ret = snd_pcm_lib_free_pages(substream);
	}

	return ret;
}

static int rtk_snd_playback_open(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct snd_card_mars *mars;
	struct rtk_snd_pcm *dpcm;

	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long phy_addr = 0;

	rtd_pr_adsp_debug("[ALSA] [%s]\n", __func__);

	mars = (struct snd_card_mars *)(substream->pcm->card->private_data);

	vir_addr = dvr_malloc_uncached_specific(sizeof(struct rtk_snd_pcm), GFP_DCU1, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[ALSA] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = (unsigned long) dvr_to_phys(vir_addr);

	/* set parameter to 0, for snapshot */
	memset(unvir_addr, 0, sizeof(struct rtk_snd_pcm));

	dpcm = unvir_addr;
	dpcm->phy_addr = phy_addr;
	dpcm->vir_addr = vir_addr;
	rtd_pr_adsp_debug("[ALSA] dpcm address = %lx\n", (unsigned long) dpcm);

	if (rtk_snd_init(mars->card)) {
		dvr_free(vir_addr);
		return -ENOMEDIUM;
	}

	dpcm->ao_agent = alsa_agent;
	dpcm->pcm_lock = kmalloc(sizeof(spinlock_t), GFP_KERNEL);

	if (dpcm->pcm_lock == NULL) {
		rtd_pr_adsp_err("[ALSA] malloc mem fail\n");
		dvr_free(vir_addr);
		return -ENOMEM;
	}

	spin_lock_init(dpcm->pcm_lock);

	runtime->private_data = dpcm;
	runtime->private_free = rtk_snd_runtime_free;

	memcpy(&runtime->hw,&rtk_snd_playback, sizeof(struct snd_pcm_hardware));
	runtime->hw.period_bytes_min = min_period_size;
	dpcm->substream = substream;
	dpcm->card = mars;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
#if defined (CONFIG_PCM_USING_HRTIMER)
	dpcm_timer_init(dpcm);
#else
	timer_setup(&dpcm->timer, rtk_snd_pcm_freerun_timer_function, 0);
#endif
#else
#if defined (CONFIG_PCM_USING_HRTIMER)
	dpcm_timer_init(dpcm);
#else
	setup_timer(&dpcm->timer, rtk_snd_pcm_freerun_timer_function, (unsigned long) dpcm);
#endif
#endif

	dpcm->ring_init = 0;

	snd_pcm_hw_constraint_minmax(runtime,SNDRV_PCM_HW_PARAM_BUFFER_TIME, 20000, 1000000);

	rtd_pr_adsp_info("[ALSA] open playback device %d\n", substream->pcm->device);
	return 0;
}

static int rtk_snd_capture_open(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_cap_pcm *dpcm;
	struct snd_card_mars *mars;
	int err;

	void *vir_addr = NULL;
	void *unvir_addr = NULL;
	unsigned long phy_addr = 0;

	rtd_pr_adsp_debug("[ALSA] [%s]\n", __func__);

	mars = (struct snd_card_mars *)(substream->pcm->card->private_data);

	/* allocate memory for card instance */
	vir_addr = dvr_malloc_uncached_specific(sizeof(struct rtk_snd_cap_pcm), GFP_DCU1, &unvir_addr);
	if (!vir_addr) {
		rtd_pr_adsp_err("[ALSA] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}
	phy_addr = (unsigned long) dvr_to_phys(vir_addr);

	/* set parameter to 0, for snapshot */
	memset(unvir_addr, 0, sizeof(struct rtk_snd_cap_pcm));

	dpcm = unvir_addr;
	dpcm->phy_addr = phy_addr;
	dpcm->vir_addr = vir_addr;

	dpcm->pcm_lock = kmalloc(sizeof(spinlock_t), GFP_KERNEL);

	if (dpcm->pcm_lock == NULL) {
		rtd_pr_adsp_err("[ALSA] malloc mem fail\n");
		dvr_free(vir_addr);
		return -ENOMEM;
	}

	rtd_pr_adsp_debug("[ALSA] %d\n", __LINE__);
	spin_lock_init(dpcm->pcm_lock);

	/* set pcm capture hardware consraint */
	memcpy(&runtime->hw, &rtk_snd_capture, sizeof(struct snd_pcm_hardware));

	dpcm->substream = substream;
	runtime->private_data = dpcm;
	runtime->private_free = snd_card_capture_runtime_free;

	dpcm->card = mars;

	/* Mars 1 for AI capture, Mars 2 for AO capture */
	if (!strcmp(dpcm->card->card->longname, "Mars_1")) {
		if (rtk_snd_create_ai_agent(substream) < 0) {
			err = -ENOMEM;
			goto fail;
		}
	} else {
		if (rtk_snd_create_ao_capture_agent(substream) < 0) {
			err = -ENOMEM;
			goto fail;
		}
	}

	/* init capture timer */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	timer_setup(&dpcm->timer, rtk_snd_pcm_capture_timer_function, 0);
#else
	setup_timer(&dpcm->timer, rtk_snd_pcm_capture_timer_function, (unsigned long) dpcm);
#endif

	dpcm->ring_init = 0;

//#ifdef CONFIG_ENABLE_WOV_SUPPORT
#if 0
	wov_init();
#endif

	rtd_pr_adsp_info("[ALSA] open a realtek pcm capture device %d success\n", substream->pcm->device);
	return 0;

fail:
	rtd_pr_adsp_err("[ALSA] open a realtek pcm capture fail\n");

	if (dpcm->pcm_lock != NULL) {
		kfree(dpcm->pcm_lock);
		dpcm->pcm_lock = NULL;
	}

	dpcm->substream = NULL;
	dpcm->card = NULL;
	runtime->private_data = NULL;
	runtime->private_free = NULL;

	dvr_free(vir_addr);

	return err;
}

static int rtk_snd_playback_close(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_pcm *dpcm = runtime->private_data;
	int i;
	rtd_pr_adsp_info("[ALSA] [%s]\n", __func__);

#if defined (CONFIG_PCM_USING_HRTIMER)
	dpcm_timer_stop(dpcm, true);
#else
	del_timer_sync(&dpcm->timer);
#endif

	if (dpcm->pcm_lock != NULL) {
		kfree(dpcm->pcm_lock);
	}

	for (i = 0; i < runtime->channels; i++) {
		if (dpcm->ao_in_ring_p[i]) {
			if (dpcm->audiopath == AUDIO_PATH_DECODER_AO) {
				dvr_free(dpcm->ao_in_ring_p[i]);
			} else {
				rtd_pr_adsp_err("[ALSA] dpcm->ao_in_ring_p[%d] %x\n", i, (unsigned long)dpcm->ao_in_ring_p[i]);
			}
		}
	}

	snd_open_count--;
	return 0;
}

static int rtk_snd_capture_close(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rtk_snd_cap_pcm *dpcm = runtime->private_data;

	rtd_pr_adsp_info("[ALSA] close a realtek pcm capture channel\n");

	del_timer_sync(&dpcm->timer);

	/* Mars 1 for AI capture, Mars 2 for AO capture */
	if (!strcmp(dpcm->card->card->longname, "Mars_1")) {
		rtd_pr_adsp_debug("[ALSA] close AI capture channel\n");
        rtk_snd_ai_hw_close(substream);
	} else {
		rtd_pr_adsp_debug("[ALSA] close AO capture channel\n");
		rtk_snd_ao_capture_hw_close(substream);
	}

	snd_open_ai_count--;
	return 0;
}

static int rtk_snd_playback_ioctl(struct snd_pcm_substream *substream,
	unsigned int cmd, void *arg)
{
	switch (cmd) {
	default:
		return snd_pcm_lib_ioctl(substream, cmd, arg);
		break;

	}

	return 0;
}

static int rtk_snd_capture_ioctl(struct snd_pcm_substream *substream,
	unsigned int cmd, void *arg)
{
	switch (cmd) {
	default:
		return snd_pcm_lib_ioctl(substream, cmd, arg);
		break;
	}

	return 0;
}

static struct snd_pcm_ops rtk_snd_playback_ops = {
	.open =	rtk_snd_playback_open,
	.close = rtk_snd_playback_close,
	.ioctl = rtk_snd_playback_ioctl,
	.hw_params = rtk_snd_playback_hw_params,
	.hw_free = rtk_snd_playback_hw_free,
	.prepare = rtk_snd_playback_prepare,
	.trigger = rtk_snd_playback_trigger,
	.pointer = rtk_snd_playback_pointer,
	.copy_user = rtk_snd_playback_copy_user,
	.mmap    = rtk_snd_playback_mmap,
};

static struct snd_pcm_ops rtk_snd_capture_ops = {
	.open =	rtk_snd_capture_open,
	.close = rtk_snd_capture_close,
	.ioctl = rtk_snd_capture_ioctl,
	.hw_params = rtk_snd_capture_hw_params,
	.hw_free = rtk_snd_capture_hw_free,
	.prepare = rtk_snd_capture_prepare,
	.trigger = rtk_snd_capture_trigger,
	.pointer = rtk_snd_capture_pointer,
};

static int rtk_snd_pcm(struct snd_card_mars *mars,
	int device, int substreams, int card)
{
	struct snd_pcm *pcm = NULL;
	int err;
	struct snd_pcm_substream *p;
	int i;

	if (card == 0 && device < CARD0_CAP_DEV_NUM) {
        if (alsa_debug)
            rtd_pr_adsp_debug("[ALSA] support capture deivce C%dD%d\n", card, device);
		err = snd_pcm_new(mars->card, "Mars PCM", device, substreams, MAX_AI_DEVICES, &pcm);
		if (err < 0)
			return err;
	} else if (card == 0 && device >= CARD0_CAP_DEV_NUM) {
        if (alsa_debug)
            rtd_pr_adsp_debug("[ALSA] not support capture device C%dD%d\n", card, device);
		err = snd_pcm_new(mars->card, "Mars PCM", device, substreams, 0, &pcm);
		if (err < 0)
			return err;
	} else if (card == 1 && device == 0) {
        if (alsa_debug)
            rtd_pr_adsp_debug("[ALSA] support capture device C%dD%d\n", card, device);
		err = snd_pcm_new(mars->card, "Mars PCM", device, 0, MAX_AI_DEVICES, &pcm);
		if (err < 0)
			return err;
	} else {
		rtd_pr_adsp_info("[ALSA] not support all device C%dD%d\n", card, device);
	}

	if (pcm) {
		if (card == 0) {
			snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK, &rtk_snd_playback_ops);
		}

		if ((device < CARD0_CAP_DEV_NUM && card == 0) || (card == 1 && device == 0)) {
			snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_CAPTURE, &rtk_snd_capture_ops);
		}

		/* set init flashpin volume to 0dB*/
		for (i = 0; i < MIXER_ADDR_MAX; i++)
			mars->ao_flash_volume[i] = 389;

		pcm->private_data = mars;
		pcm->info_flags = 0;
		strncpy(pcm->name, "Mars PCM", sizeof(pcm->name));

		/* set playback device */
		if (card == 0) {
			p = pcm->streams[0].substream;
			for (i = 0; i < substreams; i++) {
				p->dma_buffer.dev.dev =	snd_dma_continuous_data(GFP_KERNEL);
				p->dma_buffer.dev.type = SNDRV_RTK_DMA_TYPE;
				p = p->next;
			}
		}

		/* set capture device */
		if ((device < CARD0_CAP_DEV_NUM && card == 0) || (card == 1 && device == 0)) {
	        p = pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream;
			for (i = 0; i < MAX_AI_DEVICES; i++) {
				p->dma_buffer.dev.dev =	snd_dma_continuous_data(GFP_KERNEL);
				p->dma_buffer.dev.type = SNDRV_RTK_DMA_TYPE;
				p = p->next;
			}
		}
	}

	return 0;
}

#define MARS_VOLUME(xname, xindex, addr)\
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER,\
	.name = xname,\
	.index = xindex,\
	.info = snd_mars_volume_info,\
	.get = snd_mars_volume_get,\
	.put = snd_mars_volume_put,\
	.private_value = addr }

static int snd_mars_volume_info(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_info *uinfo)
{
    if (alsa_debug)
        rtd_pr_adsp_debug("[ALSA] %s\n", __func__);
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 100;
	uinfo->value.integer.max = 509;
	return 0;
}

static int snd_mars_volume_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_card_mars *mars = snd_kcontrol_chip(kcontrol);
	unsigned long flags;
	int addr = kcontrol->private_value;
    if (alsa_debug)
        rtd_pr_adsp_debug("[ALSA] %s\n", __func__);
	spin_lock_irqsave(&mars->mixer_lock, flags);
	ucontrol->value.integer.value[0] = mars->ao_flash_volume[addr];
	spin_unlock_irqrestore(&mars->mixer_lock, flags);
	return 0;
}

static int snd_mars_volume_put(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_card_mars *mars = snd_kcontrol_chip(kcontrol);
	unsigned long flags;
	int addr, volume;
    if (alsa_debug)
        rtd_pr_adsp_debug("[ALSA] %s\n", __func__);

	addr = kcontrol->private_value;
	volume = ucontrol->value.integer.value[0];

	/* check volume gain is correct */
	if (volume < 100)
		volume = 100;
	if (volume > 509)
		volume = 509;

    if (alsa_debug)
        rtd_pr_adsp_debug("[ALSA] volume(%d) %d\n", addr, volume);

	if ((addr >= MIXER_ADDR_0) && (addr <= MIXER_ADDR_7)) {
	spin_lock_irqsave(&mars->mixer_lock, flags);
	mars->ao_flash_change[addr] = (mars->ao_flash_volume[addr] != volume);
	mars->ao_flash_volume[addr] = volume;
	spin_unlock_irqrestore(&mars->mixer_lock, flags);
		schedule_work(&mars->work_volume);
	return mars->ao_flash_change[addr];
	}

	return 0;
}

static struct snd_kcontrol_new snd_mars_controls[] = {
	MARS_VOLUME("AMIXER0", 0, MIXER_ADDR_0),
	MARS_VOLUME("AMIXER1", 0, MIXER_ADDR_1),
	MARS_VOLUME("AMIXER2", 0, MIXER_ADDR_2),
	MARS_VOLUME("AMIXER3", 0, MIXER_ADDR_3),
	MARS_VOLUME("AMIXER4", 0, MIXER_ADDR_4),
	MARS_VOLUME("AMIXER5", 0, MIXER_ADDR_5),
	MARS_VOLUME("AMIXER6", 0, MIXER_ADDR_6),
	MARS_VOLUME("AMIXER7", 0, MIXER_ADDR_7),
};

static int rtk_snd_new_mixer(struct snd_card_mars *mars)
{
	struct snd_card *card = mars->card;
	unsigned int idx;
	int err;

	spin_lock_init(&mars->mixer_lock);
	INIT_WORK(&mars->work_volume, rtk_snd_playback_volume_work);

	strncpy(card->mixername, "Mars Mixer", sizeof(card->mixername));

	for (idx = 0; idx < ARRAY_SIZE(snd_mars_controls); idx++) {
		err = snd_ctl_add(card,
			snd_ctl_new1(&snd_mars_controls[idx], mars));
		if (err < 0)
			return err;
	}
	return 0;
}

static int rtk_snd_probe(struct platform_device *devptr)
{
	struct snd_card *card;
	struct snd_card_mars *mars;
	int idx, err;
	int dev, i;

	dev = devptr->id;

	if (!enable[dev])
		return -ENODEV;

	err = snd_card_new(&devptr->dev, index[dev], id[dev],
			THIS_MODULE, sizeof(struct snd_card_mars), &card);
	if (err < 0)
		return err;
	mars = (struct snd_card_mars *)card->private_data;
	mars->card = card;

	mutex_init(&mars->far_field_lock);
	mars->far_field_ring_audio = NULL;
	mars->far_field_ring_audio_cached = NULL;
	mars->far_field_vir = NULL;
	mars->reference_count = 0;

	for (i = 0; i < MAX_PCM_DEVICES; i++) {
		mars->snd_dmafd_handle[i] = -1;
	}

	for (idx = 0; idx < MAX_PCM_DEVICES && idx < pcm_devs[dev]; idx++) {
		if (pcm_substreams[dev] < 1)
			pcm_substreams[dev] = 1;
		if (pcm_substreams[dev] > MAX_PCM_SUBSTREAMS)
			pcm_substreams[dev] = MAX_PCM_SUBSTREAMS;

		err = rtk_snd_pcm(mars, idx, pcm_substreams[dev], dev);

		if (err < 0) {
			rtd_pr_adsp_err("init pcm device failed %d\n", dev);
			goto __nodev;
		}

		if ((idx == 0) && (dev == 0)) {
			err = rtk_snd_create_hwdep(mars, idx);

			if (err < 0) {
				rtd_pr_adsp_err("init hwdep device failed %d\n", dev);
				goto __nodev;
			}
		}
	}

	err = rtk_snd_new_mixer(mars);
	if (err < 0)
		goto __nodev;

	strncpy(card->driver, "Mars", sizeof(card->driver));
	strncpy(card->shortname, "Mars", sizeof(card->shortname));
	snprintf(card->longname, sizeof(card->longname), "Mars_%i", dev + 1);

	err = snd_card_register(card);
	if (err == 0) {
		platform_set_drvdata(devptr, card);
		return 0;
	}

__nodev:
	snd_card_free(platform_get_drvdata(devptr));
	return err;
}

static int rtk_snd_remove(struct platform_device *devptr)
{
	snd_card_free(platform_get_drvdata(devptr));
	platform_set_drvdata(devptr, NULL);
	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int rtk_snd_pm_suspend(struct device *dev)
{
    rtd_pr_adsp_debug("[ALSA] rtk_snd_pm_suspend!!!\n");
	return 0;
}

static int rtk_snd_pm_resume(struct device *dev)
{
	rtd_pr_adsp_debug("[ALSA] rtk_snd_pm_resume!!!\n");
	return 0;
}

static SIMPLE_DEV_PM_OPS(rtk_snd_pm, rtk_snd_pm_suspend, rtk_snd_pm_resume);
#define RTK_SND_OPS	&rtk_snd_pm
#else
#define RTK_SND_OPS	NULL
#endif

#define RTK_SND_DRIVER	"rtk_snd"

static struct platform_driver rtk_snd_driver = {
	.probe		= rtk_snd_probe,
	.remove		= rtk_snd_remove,
	.driver		= {
		.name	= RTK_SND_DRIVER,
		.owner	= THIS_MODULE,
		.pm	= RTK_SND_OPS,
	},
};

static int alsa_card_mars_init(void)
{
	int dev, cards, err;
	int ret = 0;
	struct platform_device *device;

#ifdef USE_FIXED_AO_PINID
	int i;

	for (i = 0; i < MAX_PCM_DEVICES; i++) {
		used_ao_pin[i] = 0;
		flush_error[i] = 0;
		pause_error[i] = 0;
		close_error[i] = 0;
		release_error[i] = 0;
	}
#endif

	snd_open_count = 0;
	snd_open_ai_count = 0;
	alsa_agent = -1;

	err = platform_driver_register(&rtk_snd_driver);
	if (err < 0)
		return err;

	for (dev = cards = 0; dev < SNDRV_CARDS && enable[dev]; dev++) {
		device = platform_device_register_simple(RTK_SND_DRIVER,
							 dev, NULL, 0);

		if (IS_ERR(device))
			continue;
		if (!platform_get_drvdata(device)) {
			platform_device_unregister(device);
			continue;
		}
		devices[dev] = device;

		cards++;
	}

	if (!cards)
		ret = -ENODEV;

	return ret;
}

static void __exit alsa_card_mars_exit(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(devices); ++i)
		platform_device_unregister(devices[i]);
	platform_driver_unregister(&rtk_snd_driver);
}

late_initcall(alsa_card_mars_init);
module_exit(alsa_card_mars_exit);
