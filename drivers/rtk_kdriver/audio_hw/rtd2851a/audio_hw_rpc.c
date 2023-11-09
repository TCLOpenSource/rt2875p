#include <linux/version.h>
#include <linux/kconfig.h>
#include <linux/compiler.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/stat.h>			/* permission */
#include <linux/fs.h>			/* fs */
#include <linux/errno.h>		/* error codes */
#include <linux/types.h>		/* size_t */
#include <linux/fcntl.h>		/* O_ACCMODE */
#include <linux/uaccess.h>		/* copy_*_user */
#include <linux/semaphore.h>		/* semaphore */
#include <linux/kthread.h>		/* kernel thread */
#include <linux/freezer.h>		/* set freezable */
#include <linux/wait.h>			/* wait event */
#include <linux/jiffies.h>		/* jiffies */
#include <linux/delay.h>

/*
 * rbus registers
 */
#include <rtk_kdriver/RPCDriver.h>

#include "audio_hw_port.h"
#include "audio_hw_driver.h"
#include "audio_hw_atv.h"
#include "audio_hw_aio.h"
#include "audio_hw_app.h"
#include "audio_hw_rpc.h"
#include "AudioInbandAPI.h"

#define __ALOG_SUBTAG "rpc"

#define AIN_VOL_0DB             0
#define AIN_VOL_STEP_MIN        1
#define AIN_VOL_STEP_1DB        2 // 0.5dB per step
#define AIN_MAX_VOL             ((30)*AIN_VOL_STEP_1DB)
#define AIN_MIN_VOL             (-70*AIN_VOL_STEP_1DB)
#define AIN_GAIN_TO_INDEX(x)    (140 + x)
#define S_OK		0x10000000

struct RBUF_HEADER {
	unsigned int magic;   /* Magic number */
	unsigned int begin_addr;
	unsigned int size;

	/* RINGBUFFER_TYPE, choose a type from RINGBUFFER_TYPE */
	unsigned int buf_id;

	unsigned int wr_ptr;
	unsigned int num_rd_ptr;
	unsigned int reserve2;  /* Reserve for Red Zone */
	unsigned int reserve3;  /* Reserve for Red Zone */

	unsigned int rd_ptr[4];

	int file_offset;
	int req_file_offset;
	int file_size;

	/* Can't be sought if data is streamed by HTTP */
	int b_seekable;
};

struct RBUF_HEADER_ARM {
	unsigned long magic;
	unsigned long begin_addr;
	unsigned long size;

	unsigned long buf_id;

	unsigned long wr_ptr;
	unsigned long num_rd_ptr;
	unsigned long reserve2;
	unsigned long reserve3;

	unsigned long rd_ptr[4];

	long file_offset;
	long req_file_offset;
	long file_size;

	long b_seekable;
};

struct RPCRES {
	int result;
	int data;
};

//For setting ATV clock RPC(defined in rtk_snd.c)
#ifndef RPC_ACPU_NOT_READY
#define RPC_ACPU_NOT_READY	-3
#endif

#ifdef CONFIG_ARM64
#define ALIGN4 (0xFFFFFFFFFFFFFFFCLL)
#else
#define ALIGN4 (0xFFFFFFFC)
#endif

enum MIXER_ADDR {
	MIXER_ADDR_0,
	MIXER_ADDR_1,
	MIXER_ADDR_2,
	MIXER_ADDR_3,
	MIXER_ADDR_4,
	MIXER_ADDR_5,
	MIXER_ADDR_6,
	MIXER_ADDR_7,
	MIXER_ADDR_MAX
};

struct snd_card_mars {
	struct snd_card *card;
	spinlock_t mixer_lock;
	struct work_struct work_volume;
	struct mutex rpc_lock;
	int ao_pin_id[MIXER_ADDR_MAX];
	int ao_flash_volume[MIXER_ADDR_MAX];
	int ao_flash_change[MIXER_ADDR_MAX];
};

enum AUDIO_SW_PATH {
	AUDIO_PATH_DECODER_AO = 0,
	AUDIO_PATH_AO,
	AUDIO_PATH_AO_BYPASS
};

enum RTK_SND_FLUSH_STATE {
	RTK_SND_FLUSH_STATE_NONE = 0,
	RTK_SND_FLUSH_STATE_WAIT,
	RTK_SND_FLUSH_STATE_FINISH
};

struct rtk_snd_pcm {
	struct snd_card_mars *card;
	spinlock_t lock;
	spinlock_t *pcm_lock;
	struct timer_list *timer;
	struct tasklet_struct *elapsed_tasklet;
	int elapsed_tasklet_finish;
	int elapsed_underflow_debouce;
	struct tasklet_struct trigger_tasklet;
	struct work_struct work_resume;
	struct work_struct work_pause;
	struct work_struct work_flush;
	uint32_t empty_timeout;
	int running;

	enum RTK_SND_FLUSH_STATE flush_state;
	unsigned int pcm_buffer_head;
	unsigned int pcm_size;		/* buffer sizze */
	unsigned int pcm_count;		/* period length */
	unsigned int pcm_bps;		/* bytes per second */
	unsigned int pcm_jiffie;	/* bytes per one jiffie */
	unsigned int pcm_irq_pos;	/* IRQ position */
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

	unsigned int sample_jiffies;
	unsigned int period_jiffies;
	unsigned int wp;
	unsigned int rp_real;
	unsigned int out_sample_width;
	int output_frame_bytes;
	int input_frame_bytes;
	int output_sample_bytes;
	int input_sample_bytes;

	struct RBUF_HEADER_ARM ring_bak[8];
	/* ALSA buffer control */
	snd_pcm_uframes_t appl_ptr;
	/* buffer control */
	int freerun;
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
	enum AUDIO_SW_PATH audiopath;
	struct page *page;
	struct RBUF_HEADER *ring; /* realtek hw ring buffer control */
	struct RBUF_HEADER *inband; /* realtek hw ring buffer control */

	struct RBUF_HEADER hw_ring[8];
	struct RBUF_HEADER hw_inband_ring[8];
	unsigned int hw_inband_data[256];

	struct RBUF_HEADER *ao_in_ring;
	struct RBUF_HEADER ao_in_ring_instance[8];

	unsigned int *ao_in_ring_p[8];
	struct snd_pcm_substream *substream;

	dma_addr_t phy_addr; /* for UNCAC_BASE */

	long total_data_wb;	/* total data write in byte */
	long pre_time_ms; /* last time that updata wp in millisecond */
	long current_time_ms; /* current time in millisecond */
	unsigned long pre_wr_ptr;

	long max_level; /* max data in ring buffer */
	long min_level; /* min data in ring buffer */
};









static int audio_send_rpc_command(int opt,
	unsigned long command, unsigned long param1,
	unsigned long param2, unsigned long param2_LE,
	unsigned long *retvalue)
{
	int ret, count;
	struct RPCRES *audio_ret;
	ret = 0;
	count = 0;
	alog_debug("[ALSA] start audio_send_rpc_command\n");

#if 1
	do {
		//[Fix me] send_rpc_command
		ret = send_rpc_command(opt, command, param1, param2, retvalue);

		if (ret == RPC_FAIL) {
			alog_err("[ALSA] RPC to ACPU fail!!\n");
			return -1;
		}

		if (ret == RPC_OK)
			break;

		// RPC_ACPU_NOT_READY
		msleep(100);
		count++;
	} while (count <= 100);

	if (ret == RPC_ACPU_NOT_READY) {
		alog_err("[ALSA] wait ACPU ready timeout!!!\n");
		return -1;
	}
#else
	if (send_rpc_command(opt, command, param1, param2, retvalue))
		ret = -1;
#endif

	alog_debug("command %lx, param1 %lx, param2_LE %lx, param2 %lx, retvalue %p\n",
		command, param1, param2_LE, param2, retvalue);

	audio_ret = (struct RPCRES *)param2_LE;

	if (command == ENUM_KERNEL_RPC_CHECK_READY ||
	command == ENUM_KERNEL_RPC_PRIVATEINFO ||
	command == ENUM_KERNEL_RPC_GET_MUTE_N_VOLUME) {
		if (*retvalue != S_OK) {
			alog_err("[ALSA] RPC S_OK fail\n");
			alog_err("[ALSA] retvalue %lx\n", *retvalue);
			ret = -1;
		}
	} else {
		if (*retvalue != S_OK || ntohl(audio_ret->result) != S_OK) {
			alog_err("[ALSA] RPC S_OK fail\n");
			alog_err("[ALSA] retvalue %lx, result %x, command %lx\n",
				*retvalue, ntohl(audio_ret->result), command);
			ret = -1;
		}
	}

	return ret;
}

int32_t rtk_snd_set_atv_clock(AUDIO_RPC_SET_ATV_CLOCK *atv_clk_info)
{	
	AUDIO_RPC_SET_ATV_CLOCK *info, *info_audio;
	struct RPCRES *res, *res_audio;
	unsigned long ret;

	dma_addr_t dat;
	void *p;

	if(!audio_driver_data) {
		alog_err("audio_driver_data is NULL!!!\n");		
		return -1;
	}
	if(atv_clk_info == NULL)
	{
		alog_err("atv_clk_info is NULL\n");
		return -1;
	}

	audio_driver_data->clsdev->dma_mask = &audio_driver_data->clsdev->coherent_dma_mask;

	if(dma_set_mask(audio_driver_data->clsdev, DMA_BIT_MASK(32))) {
		alog_err("DMA not supported\n");
	}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)) && defined(CONFIG_ARM64)
	arch_setup_dma_ops(audio_driver_data->clsdev, 0, 0, NULL, false);
#endif
	p = dma_alloc_coherent(audio_driver_data->clsdev, 4096, &dat, GFP_KERNEL | __GFP_DMA);

	if (!p) {
		alog_err("[ALSA] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}

	info = p;
	info_audio = (AUDIO_RPC_SET_ATV_CLOCK *)((ulong) dat|0xa0000000);

	res = (struct RPCRES *)
		((((unsigned long)p + sizeof(AUDIO_RPC_SET_ATV_CLOCK) + 8) & ALIGN4));
	res_audio = (struct RPCRES *)
		(((((unsigned int) dat|0xa0000000) + sizeof(AUDIO_RPC_SET_ATV_CLOCK) + 8) &
			0xFFFFFFFC));

	alog_debug("[atv_clk_info] %ld %d", atv_clk_info->instanceID, atv_clk_info->atv_type);

	/* send atv clock info */
	info->instanceID = htonl(atv_clk_info->instanceID); //endian swap
	info->atv_type = htonl(atv_clk_info->atv_type);	//endian swap

	if (audio_send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_AIN_ATV_SET_CLOCK,
		(ulong) info_audio,
		(ulong) res_audio,
		(unsigned long) res, &ret)) {
		alog_err("%s %d RPC fail\n", __func__, __LINE__);
		dma_free_coherent(audio_driver_data->clsdev, 4096, p, dat);
		return -1;
	}

	dma_free_coherent(audio_driver_data->clsdev, 4096, p, dat);

	return 0;
}

int32_t rtk_snd_set_decoder_mute(bool mute, ENUM_AI_PATH path_sel)
{	
	AUDIO_CONFIG_COMMAND *vir_audioConfig, *phy_audioConfig;
	struct RPCRES *res, *res_audio;
	unsigned long ret;
	dma_addr_t dat;
	void *p;

	int ch;

	if(!audio_driver_data) {
		alog_err("audio_driver_data is NULL!!!\n");		
		return -1;
	}

	audio_driver_data->clsdev->dma_mask = &audio_driver_data->clsdev->coherent_dma_mask;

	if(dma_set_mask(audio_driver_data->clsdev, DMA_BIT_MASK(32))) {
		alog_err("DMA not supported\n");
	}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)) && defined(CONFIG_ARM64)
	arch_setup_dma_ops(audio_driver_data->clsdev, 0, 0, NULL, false);
#endif
	p = dma_alloc_coherent(audio_driver_data->clsdev, 4096, &dat, GFP_KERNEL | __GFP_DMA);

	if (!p) {
		alog_err("[ALSA] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}

	vir_audioConfig = p;
	phy_audioConfig = (AUDIO_CONFIG_COMMAND *)((ulong) dat|0xa0000000);

	res = (struct RPCRES *)
		((((unsigned long)p + sizeof(AUDIO_CONFIG_COMMAND) + 8) & ALIGN4));
	res_audio = (struct RPCRES *)
		(((((unsigned int) dat|0xa0000000) + sizeof(AUDIO_CONFIG_COMMAND) + 8) &
			0xFFFFFFFC));

	/* send sw decoder info */
    	vir_audioConfig->msgID = htonl(AUDIO_CONFIG_CMD_SET_AI_VOLUME);
    	vir_audioConfig->value[0] = htonl(true); //always enable SET_AI_VOLUME function
    	vir_audioConfig->value[1] = htonl(path_sel);

	//if mute = 1, volume = ENUM_AUDIO_DVOL_MUTE, ENUM_AUDIO_DVOL_K0p0DB AIN_VOL_0DB
	for(ch=0;ch<8;ch++)
    	{
        	vir_audioConfig->value[2 + ch] = htonl(mute?ENUM_AUDIO_DVOL_MUTE:ENUM_AUDIO_DVOL_K0p0DB);
    	}
	

	alog_debug("[atv_decoder_info] %x %x", mute, path_sel);

	if (audio_send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_ATV_MUTE,
		(ulong) phy_audioConfig,
		(ulong) res_audio,
		(unsigned long) res, &ret)) {
		alog_err("%s %d RPC fail\n", __func__, __LINE__);
		dma_free_coherent(audio_driver_data->clsdev, 4096, p, dat);
		return -1;
	}

	dma_free_coherent(audio_driver_data->clsdev, 4096, p, dat);

	return 0;
}

int32_t rtk_snd_enhance_btsc_separation(bool enable)
{	
	AUDIO_CONFIG_COMMAND *vir_audioConfig, *phy_audioConfig;
	struct RPCRES *res, *res_audio;
	unsigned long ret;
	dma_addr_t dat;
	void *p;

	if(!audio_driver_data) {
		alog_err("audio_driver_data is NULL!!!\n");
		return -1;
	}

	audio_driver_data->clsdev->dma_mask = &audio_driver_data->clsdev->coherent_dma_mask;

	if(dma_set_mask(audio_driver_data->clsdev, DMA_BIT_MASK(32))) {
		alog_err("DMA not supported\n");
	}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)) && defined(CONFIG_ARM64)
	arch_setup_dma_ops(audio_driver_data->clsdev, 0, 0, NULL, false);
#endif
	p = dma_alloc_coherent(audio_driver_data->clsdev, 4096, &dat, GFP_KERNEL | __GFP_DMA);

	if (!p) {
		alog_err("[ALSA] %d alloc memory fail\n", __LINE__);
		return -ENOMEM;
	}

	vir_audioConfig = p;
	phy_audioConfig = (AUDIO_CONFIG_COMMAND *)((ulong) dat|0xa0000000);

	res = (struct RPCRES *)
		((((unsigned long)p + sizeof(AUDIO_CONFIG_COMMAND) + 8) & ALIGN4));
	res_audio = (struct RPCRES *)
		(((((unsigned int) dat|0xa0000000) + sizeof(AUDIO_CONFIG_COMMAND) + 8) &
			0xFFFFFFFC));

	/* send btsc separation enhance config */
    	vir_audioConfig->value[0] = htonl(enable);
#if 0
    	vir_audioConfig->value[1] = htonl(10); //down 5dB if L/R diff between hi_th, low_th
    	vir_audioConfig->value[2] = htonl(10); //low_th
		vir_audioConfig->value[3] = htonl(30); //high_th
		vir_audioConfig->value[4] = htonl(10); //low_band_diff_lowth
		vir_audioConfig->value[5] = htonl(18); //lowband_diff_high_th
		vir_audioConfig->value[6] = htonl(10); //high_bnad_diff_lowth
		vir_audioConfig->value[7] = htonl(25); //high_band_diff_highth
#else
    	vir_audioConfig->value[1] = htonl(5); //down 5dB if L/R diff between hi_th, low_th
    	vir_audioConfig->value[2] = htonl(16); //low_th
	vir_audioConfig->value[3] = htonl(23); //high_th
	vir_audioConfig->value[4] = htonl(18); //low_band_diff_lowth
	vir_audioConfig->value[5] = htonl(25); //lowband_diff_high_th
	vir_audioConfig->value[6] = htonl(0); //high_bnad_diff_lowth
	vir_audioConfig->value[7] = htonl(0); //high_band_diff_highth		
#endif	
		
	alog_debug("[atv_btsc_enhance_info] %d", enable);

	if (audio_send_rpc_command(RPC_AUDIO,
		ENUM_KERNEL_RPC_BTSC_CTL,
		(ulong) phy_audioConfig,
		(ulong) res_audio,
		(unsigned long) res, &ret)) {
		alog_err("%s %d RPC fail\n", __func__, __LINE__);
		dma_free_coherent(audio_driver_data->clsdev, 4096, p, dat);
		return -1;
	}

	dma_free_coherent(audio_driver_data->clsdev, 4096, p, dat);

	return 0;
}