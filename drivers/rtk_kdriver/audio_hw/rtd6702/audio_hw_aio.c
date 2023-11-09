#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
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
#include <linux/delay.h>		/* msleep/mdelay */
#include <linux/string.h>		/* memcpy */
#include <linux/freezer.h>
#include <linux/timer.h>
#include <linux/version.h>
#include <rtk_kdriver/tvscalercontrol/hdmirx/hdmifun.h>
#include <rtk_kdriver/tvscalercontrol/hdmirx/hdmi_vfe.h>
#include <rtk_kdriver/rtk_semaphore.h>
/*
 * eARC registers
 */
#include <rbus/earcmisc_reg.h>
#include <rbus/earc_reg.h>


/*
 * rbus registers
 */
#include <rbus/audio_reg.h>

#include "audio_hw_port.h"
#include "audio_hw_atv.h"
#include "audio_hw_aio.h"
#include "audio_hw_app.h"

#define __ALOG_SUBTAG "aio"

//#define debug_io
#define EARC_RESET_FSM
#define AUTO_READ_CAP


static uint32_t	m_ampmute[AIO_AMPMUTE_UNKNOWN] = {0};

static MUTE_AMP Audio_MuteAmpFunc = NULL;
static MUTE_SPDIF_OUT Audio_MuteSpdifOutFunc = NULL;
static MUTE_AV_OUT Audio_MuteAVOutFunc = NULL;
static AIO_PATH_CFG curr_path_cfg[2] = {
	{
		AIO_PATH_ID_PB,
		{ AIO_PATH_SRC_UNKNOWN, AIO_PATH_SRC_UNKNOWN, AIO_PATH_SRC_UNKNOWN, AIO_PATH_SRC_UNKNOWN},
	},
	{
		AIO_PATH_ID_TS,
		{ AIO_PATH_SRC_UNKNOWN, AIO_PATH_SRC_UNKNOWN, AIO_PATH_SRC_UNKNOWN, AIO_PATH_SRC_UNKNOWN},
	}
};
#ifndef TV001_BOARD
static uint32_t aio_peak_mag_tab[AIO_PEAK_MAG_TAB_SIZE] = {
	0x00007FFF,                 //   0.00 (dB)
	(uint32_t)(0.944060876286 * 0x00008000), (uint32_t)(0.891250938134 * 0x00008000), (uint32_t)(0.841395141645 * 0x00008000), (uint32_t)(0.794328234724 * 0x00008000),	// - 2.00 (dB)
	(uint32_t)(0.749894209332 * 0x00008000), (uint32_t)(0.707945784384 * 0x00008000), (uint32_t)(0.668343917569 * 0x00008000), (uint32_t)(0.630957344480 * 0x00008000),	// - 4.00 (dB)
	(uint32_t)(0.595662143529 * 0x00008000), (uint32_t)(0.562341325190 * 0x00008000), (uint32_t)(0.530884444231 * 0x00008000), (uint32_t)(0.501187233627 * 0x00008000),	// - 6.00 (dB)
	(uint32_t)(0.473151258961 * 0x00008000), (uint32_t)(0.446683592151 * 0x00008000), (uint32_t)(0.421696503429 * 0x00008000), (uint32_t)(0.398107170553 * 0x00008000),	// - 8.00 (dB)
	(uint32_t)(0.375837404288 * 0x00008000), (uint32_t)(0.354813389234 * 0x00008000), (uint32_t)(0.334965439158 * 0x00008000), (uint32_t)(0.316227766017 * 0x00008000),	// - 10.00 (dB)
	(uint32_t)(0.298538261892 * 0x00008000), (uint32_t)(0.281838293126 * 0x00008000), (uint32_t)(0.266072505980 * 0x00008000), (uint32_t)(0.251188643151 * 0x00008000),	// - 12.00 (dB)
	(uint32_t)(0.237137370566 * 0x00008000), (uint32_t)(0.223872113857 * 0x00008000), (uint32_t)(0.211348903984 * 0x00008000), (uint32_t)(0.199526231497 * 0x00008000),	// - 14.00 (dB)
	(uint32_t)(0.188364908949 * 0x00008000), (uint32_t)(0.177827941004 * 0x00008000), (uint32_t)(0.167880401812 * 0x00008000), (uint32_t)(0.158489319246 * 0x00008000),	// - 16.00 (dB)
	(uint32_t)(0.149623565609 * 0x00008000), (uint32_t)(0.141253754462 * 0x00008000), (uint32_t)(0.133352143216 * 0x00008000), (uint32_t)(0.125892541179 * 0x00008000),	// - 18.00 (dB)
	(uint32_t)(0.118850222744 * 0x00008000), (uint32_t)(0.112201845430 * 0x00008000), (uint32_t)(0.105925372518 * 0x00008000), (uint32_t)(0.100000000000 * 0x00008000),	// - 20.00 (dB)
	(uint32_t)(0.094406087629 * 0x00008000), (uint32_t)(0.089125093813 * 0x00008000), (uint32_t)(0.084139514165 * 0x00008000), (uint32_t)(0.079432823472 * 0x00008000),	// - 22.00 (dB)
	(uint32_t)(0.074989420933 * 0x00008000), (uint32_t)(0.070794578438 * 0x00008000), (uint32_t)(0.066834391757 * 0x00008000), (uint32_t)(0.063095734448 * 0x00008000),	// - 24.00 (dB)
	(uint32_t)(0.059566214353 * 0x00008000), (uint32_t)(0.056234132519 * 0x00008000), (uint32_t)(0.053088444423 * 0x00008000), (uint32_t)(0.050118723363 * 0x00008000),	// - 26.00 (dB)
	(uint32_t)(0.047315125896 * 0x00008000), (uint32_t)(0.044668359215 * 0x00008000), (uint32_t)(0.042169650343 * 0x00008000), (uint32_t)(0.039810717055 * 0x00008000),	// - 28.00 (dB)
	(uint32_t)(0.037583740429 * 0x00008000), (uint32_t)(0.035481338923 * 0x00008000), (uint32_t)(0.033496543916 * 0x00008000), (uint32_t)(0.031622776602 * 0x00008000),	// - 30.00 (dB)
	(uint32_t)(0.029853826189 * 0x00008000), (uint32_t)(0.028183829313 * 0x00008000), (uint32_t)(0.026607250598 * 0x00008000), (uint32_t)(0.025118864315 * 0x00008000),	// - 32.00 (dB)
	(uint32_t)(0.023713737057 * 0x00008000), (uint32_t)(0.022387211386 * 0x00008000), (uint32_t)(0.021134890398 * 0x00008000), (uint32_t)(0.019952623150 * 0x00008000),	// - 34.00 (dB)
	(uint32_t)(0.018836490895 * 0x00008000), (uint32_t)(0.017782794100 * 0x00008000), (uint32_t)(0.016788040181 * 0x00008000), (uint32_t)(0.015848931925 * 0x00008000),	// - 36.00 (dB)
	(uint32_t)(0.014962356561 * 0x00008000), (uint32_t)(0.014125375446 * 0x00008000), (uint32_t)(0.013335214322 * 0x00008000), (uint32_t)(0.012589254118 * 0x00008000),	// - 38.00 (dB)
	(uint32_t)(0.011885022274 * 0x00008000), (uint32_t)(0.011220184543 * 0x00008000), (uint32_t)(0.010592537252 * 0x00008000), (uint32_t)(0.010000000000 * 0x00008000),	// - 40.00 (dB)
	(uint32_t)(0.009440608763 * 0x00008000), (uint32_t)(0.008912509381 * 0x00008000), (uint32_t)(0.008413951416 * 0x00008000), (uint32_t)(0.007943282347 * 0x00008000),	// - 42.00 (dB)
	(uint32_t)(0.007498942093 * 0x00008000), (uint32_t)(0.007079457844 * 0x00008000), (uint32_t)(0.006683439176 * 0x00008000), (uint32_t)(0.006309573445 * 0x00008000),	// - 44.00 (dB)
	(uint32_t)(0.005956621435 * 0x00008000), (uint32_t)(0.005623413252 * 0x00008000), (uint32_t)(0.005308844442 * 0x00008000), (uint32_t)(0.005011872336 * 0x00008000),	// - 46.00 (dB)
	(uint32_t)(0.004731512590 * 0x00008000), (uint32_t)(0.004466835922 * 0x00008000), (uint32_t)(0.004216965034 * 0x00008000), (uint32_t)(0.003981071706 * 0x00008000),	// - 48.00 (dB)
	(uint32_t)(0.003758374043 * 0x00008000), (uint32_t)(0.003548133892 * 0x00008000), (uint32_t)(0.003349654392 * 0x00008000), (uint32_t)(0.003162277660 * 0x00008000),	// - 50.00 (dB)
	(uint32_t)(0.002985382619 * 0x00008000), (uint32_t)(0.002818382931 * 0x00008000), (uint32_t)(0.002660725060 * 0x00008000), (uint32_t)(0.002511886432 * 0x00008000),	// - 52.00 (dB)
	(uint32_t)(0.002371373706 * 0x00008000), (uint32_t)(0.002238721139 * 0x00008000), (uint32_t)(0.002113489040 * 0x00008000), (uint32_t)(0.001995262315 * 0x00008000),	// - 54.00 (dB)
	(uint32_t)(0.001883649089 * 0x00008000), (uint32_t)(0.001778279410 * 0x00008000), (uint32_t)(0.001678804018 * 0x00008000), (uint32_t)(0.001584893192 * 0x00008000),	// - 56.00 (dB)
	(uint32_t)(0.001496235656 * 0x00008000), (uint32_t)(0.001412537545 * 0x00008000), (uint32_t)(0.001333521432 * 0x00008000), (uint32_t)(0.001258925412 * 0x00008000),	// - 58.00 (dB)
	(uint32_t)(0.001188502227 * 0x00008000), (uint32_t)(0.001122018454 * 0x00008000), (uint32_t)(0.001059253725 * 0x00008000), (uint32_t)(0.001000000000 * 0x00008000),	// - 60.00 (dB)
};
#endif
/**
 * * !!!!!!!!!!!! DO NOT REMOVE static declaration !!!!!!!!!!!!!
 * !!!!! this function scope is LIMITED within this file !!!!!
 *
 * Select Headphone output source.
 *
 * @param <src>	{ AIO_HP_MUTE_ALL | AIO_HP_ADCFE | AIO_HP_PB_DAC / AIO_HP_TS_DAC}
 * @return	{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
static int32_t Audio_AioSetOptSrc_Headphone(AIO_HP_SRC mix_src);

/* 
 * aio thread
 */
static DECLARE_WAIT_QUEUE_HEAD(audio_aio_wait);
static struct task_struct *audio_aio_thread = NULL;
static int AioThread(void *arg);
static bool AioThreadState = false;

/* 
 * eARC by RyanLan
 */
#ifdef SUPPORT_EARC
/* Timing*/
// the value of comma_off/on can't change, 0x4 is (7.904-dynamic_comma_len)ms ~ (12+dynamic_comma_len)ms
#define EARC_TIMING_COMMA_ON_MIN		0x6
#define EARC_TIMING_COMMA_ON_MAX		0xE
#define EARC_TIMING_COMMA_OFF_MIN		0x6
#define EARC_TIMING_COMMA_OFF_MAX		0xE
#define EARC_TIMING_TX_TIMEOUT 			0x5 // 500ms
#define EARC_TIMING_TX_CONNEARC_VALID	0x3 // 1ms
#define EARC_TIMING_LOST_HEARTBEAT 		0x1f // 125+10 ms
#define EARC_TIMING_HEARTBEAT 	0x5 // 50ms, sent heartbeat per 50ms
#define EARC_TIMING_HEARTBEAT_DISC 	0xf // 60ms, sent heartbeat per 60ms
#define EARC_READ_NACK_NUM 		0x3
#define EARC_READ_UECC_NUM  		0x3
#define EARC_HDMI_PORT_NUM		0
#define EARC_HDMI_HPD_DELAY		(950*1000) //950ms
#define EARC_HEARTBEAT_TO                 6
static DEFINE_SEMAPHORE(audio_sem_aio);
static DEFINE_SEMAPHORE(audio_sem_earc_transaction);

static bool g_EarcThreadState = false;
EARC_STATE_T g_Earc_State = {0};
unsigned char g_CapabilityData[256] = {0};
static unsigned char g_Latency = 0;
static int32_t g_IrqFlag = 0;
static int32_t g_eARCReadData = 0;
static int32_t g_EarcDebugMode = 0;
static struct task_struct *audio_hw_earc_thread = NULL;
unsigned int RunCapFlag = 0;
unsigned int ReadCapResult = 0;
static unsigned char PreHpdState = 0;
static unsigned char Pre5vState = 0;
static unsigned char g_EarcTimerInitDone = 0;
static unsigned int g_EarcInit = 0;
struct timer_list Audio_EarcTimerList;
static unsigned int g_EarcEn = 0;
static unsigned int g_Disc2_count = 0;
unsigned int g_heartbeart_count=0, g_heartbeart_count_pre=0,g_heartbeart_to=EARC_HEARTBEAT_TO;
unsigned int g_lostheartbeart=0;
char g_EarcPort = 0;
bool	HB_CADENCE_CHANGE = false;
extern unsigned char g_cap_update,g_latency_update;

void EarcTask(void);
static DECLARE_WAIT_QUEUE_HEAD(audio_earc_wait);
static int32_t AudioEarcRead(unsigned char *Data);
static int EarcThread(void *arg);
static void AudioEarcInitPow(bool En);
static void AudioEarcInitCmdc(void);
static void Audio_HW_Earc_ClrReadIrq(void);
static void Audio_HW_Earc_ClrWriteIrq(void);
static void AudioHw_Earc_Reset(void);
static void AudioEarcARCMode(void);
int32_t Audio_HW_Earc_Arc_Switch(AIO_AUDIO_EARC_ARC_SWITCH_T Mode);
void Audio_HW_Earc_Reset_irq(void);
#ifdef AUTO_READ_CAP
static int32_t  AutoReadCap(unsigned char *Data);
#endif
#endif

#ifndef TV001_BOARD
static uint32_t AioPeakMagToDecibel(uint32_t mag);
#endif

unsigned int AIO_ReadRegister(unsigned int addr)
{
	return IoReg_Read32(addr);
}

void AIO_WriteRegister(unsigned int addr, unsigned int value)
{
	IoReg_Write32(addr, value);
}

int32_t Audio_AioGetPathSrc(AIO_PATH_CFG *p_path_cfg)
{
	if (p_path_cfg == NULL)
		return AIO_ERROR;

	if (p_path_cfg->id == AIO_PATH_ID_PB) {
		memcpy(p_path_cfg, &curr_path_cfg[AIO_PATH_ID_PB], sizeof(AIO_PATH_CFG));
	} else {
		memcpy(p_path_cfg, &curr_path_cfg[AIO_PATH_ID_TS], sizeof(AIO_PATH_CFG));
	}

	return AIO_OK;
}

void ao_SetI2sOutclockSrc(AIO_PATH_ID id)
{
	audio_global_ctrl_2_RBUS global_ctrl_2;

	global_ctrl_2.regValue =  AIO_ReadRegister(AUDIO_global_ctrl_2_reg);
	global_ctrl_2.sel_clk_i2s_out = id;
	AIO_WriteRegister(AUDIO_global_ctrl_2_reg,  global_ctrl_2.regValue);
}

void ao_SetSpdifOutclockSrc(AIO_PATH_ID id)
{
#ifdef RTD289X
	audio_global_ctrl_0_RBUS global_ctrl_0;
	global_ctrl_0.regValue =  AIO_ReadRegister(AUDIO_global_ctrl_0_reg);
	global_ctrl_0.sel_clk_spdif_out = id;
	AIO_WriteRegister(AUDIO_global_ctrl_0_reg,  global_ctrl_0.regValue);
#else //new IC after RTD289X
	audio_global_ctrl_2_RBUS global_ctrl_2;
	global_ctrl_2.regValue =  AIO_ReadRegister(AUDIO_global_ctrl_2_reg);
	global_ctrl_2.sel_clk_spdif_out = id;
	AIO_WriteRegister(AUDIO_global_ctrl_2_reg,  global_ctrl_2.regValue);
#endif
}

/** yychao:
 *    1. We control these registers in audio F/W now.
 *       Thus, do NOT use following APIs.
 *    2. to switch SPDIF-out pcm/raw mode,
 *       please use SetAudioSpdifOutputMode() in AudioUtil.cpp
 */
#if 0
/**
 * Setup I2S_OUT source.
 *
 * @param <p_cfg_i2s>	{ pointer of I2S_OUT control block }
 * @return				{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_AioCfgOptSrc_I2S(AIO_I2SO_CFG *p_cfg_i2s){
	audio_i2s_out_RBUS	I2SO_CTRL_REG;
	I2SO_CTRL_REG.regValue = AIO_ReadRegister(AUDIO_i2s_out_reg);

	if(p_cfg_i2s->update_src_only == FALSE) {
		I2SO_CTRL_REG.mclk_sel = p_cfg_i2s->mclk_sel;
	}
	I2SO_CTRL_REG.output_sel = p_cfg_i2s->src_sel;
	AIO_WriteRegister(AUDIO_i2s_out_reg, I2SO_CTRL_REG.regValue);

	if(p_cfg_i2s ->source_sel == AIO_PATH_ID_PB){
		if(p_cfg_i2s->isFixVolume)
		Audio_AioSetPcmSrc_I2S(AIO_I2S_PCM_PB_BF_AVC1);
		else
		Audio_AioSetPcmSrc_I2S(AIO_I2S_PCM_PB_BF_AVC2);

		ao_SetI2sOutclockSrc(AIO_PATH_ID_PB);
	} else {
		Audio_AioSetPcmSrc_I2S(AIO_I2S_PCM_TS_AF_DVC);
		ao_SetI2sOutclockSrc(AIO_PATH_ID_TS);
	}

	return AIO_OK;
}

/**
 * Setup SPDIF_OUT source.
 *
 * @param <p_cfg_spdif>	{ pointer of SPDIF_OUT control block }
 * @return				{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_AioCfgOptSrc_SPDIF(AIO_SPDIFO_CFG *p_cfg_spdif){
	audio_spdif_out_RBUS	spdif_out;

	spdif_out.regValue = AIO_ReadRegister(AUDIO_spdif_out_reg);
	spdif_out.output_sel = p_cfg_spdif->src_sel;
	AIO_WriteRegister(AUDIO_spdif_out_reg, spdif_out.regValue);

	if(p_cfg_spdif ->source_sel == AIO_PATH_ID_PB){
		if(p_cfg_spdif->isFixVolume)
		Audio_AioSetPcmSrc_SPDIF(AIO_SPDIF_PCM_PB_BF_AVC1); // fix-vol
		else
		Audio_AioSetPcmSrc_SPDIF(AIO_SPDIF_PCM_PB_BF_AVC2); // non-fix-vol

		ao_SetSpdifOutclockSrc(AIO_PATH_ID_PB);
	} else {
		if(p_cfg_spdif->isFixVolume)
		Audio_AioSetPcmSrc_SPDIF(AIO_SPDIF_PCM_TS_AF_DVC); // fix-vol
		else
		Audio_AioSetPcmSrc_SPDIF(AIO_SPDIF_PCM_TS_NONPCM); // non-pcm

		ao_SetSpdifOutclockSrc(AIO_PATH_ID_TS);
	}
	return AIO_OK;
}

/**
 * Select SPDIF pcm source.
 *
 * @param <src>	{ AIO_SPDIF_PCM_PB_BF_AVC1 |
		  AIO_SPDIF_PCM_PB_BF_AVC2 |
		  AIO_SPDIF_PCM_PB_AF_AVC2 |
		  AIO_SPDIF_PCM_TS_AF_DVC  |
		  AIO_SPDIF_PCM_PB_NONPCM  |
		  AIO_SPDIF_PCM_TS_NONPCM  }
 * @return		{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_AioSetPcmSrc_SPDIF(AIO_SPDIF_PCM_SRC sel){
	audio_spdif_out_RBUS	spdif_out_REG;
	spdif_out_REG.regValue = AIO_ReadRegister(AUDIO_spdif_out_reg);
	spdif_out_REG.pcm_source_sel = sel;
	AIO_WriteRegister(AUDIO_spdif_out_reg, spdif_out_REG.regValue);
	return AIO_OK;
}

/**
 * Select I2S pcm source.
 *
 * @param <src>	{ AIO_I2S_PCM_PB_BF_AVC1 |
		  AIO_I2S_PCM_PB_BF_AVC2 |
		  AIO_I2S_PCM_PB_AF_AVC2 |
		  AIO_I2S_PCM_TS_AF_DVC  }
 * @return		{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_AioSetPcmSrc_I2S(AIO_I2S_PCM_SRC sel){
	audio_i2s_out_RBUS	I2SO_CTRL_REG;
	I2SO_CTRL_REG.regValue = AIO_ReadRegister(AUDIO_i2s_out_reg);
	I2SO_CTRL_REG.pcm_source_sel = sel;
	AIO_WriteRegister(AUDIO_i2s_out_reg, I2SO_CTRL_REG.regValue);

	return AIO_OK;
}

/**
 * Select DAC0 pcm source.
 *
 * @param <src>	{ AIO_DAC0_PCM_PB_BF_AVC1 |
		  AIO_DAC0_PCM_PB_BF_AVC2 |
		  AIO_DAC0_PCM_PB_AF_AVC2 |
		  AIO_DAC0_PCM_TS_AF_DVC }
 * @return		{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_AioSetPcmSrc_DAC0(AIO_DAC0_PCM_SRC sel){
	audio_dac0_1_src_sel_RBUS     dac0_1_src_sel_REG;
	dac0_1_src_sel_REG.regValue =  AIO_ReadRegister(AUDIO_dac0_1_src_sel_reg);
	dac0_1_src_sel_REG.dac0_pcm_source_sel = sel;
	AIO_WriteRegister(AUDIO_dac0_1_src_sel_reg, dac0_1_src_sel_REG.regValue);

	return AIO_OK;
}

/**
 * Select DAC1 pcm source.
 *
 * @param <src>	{ AIO_DAC1_PCM_PB_BF_AVC1 |
		  AIO_DAC1_PCM_PB_BF_AVC2 |
		  AIO_DAC1_PCM_PB_AF_AVC2 |
		  AIO_DAC1_PCM_TS_BF_DVC  |
		  AIO_DAC1_PCM_TS_AF_DVC  }
 * @return		{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_AioSetPcmSrc_DAC1(AIO_DAC1_PCM_SRC sel){
	audio_dac0_1_src_sel_RBUS     dac0_1_src_sel_REG;
	dac0_1_src_sel_REG.regValue =  AIO_ReadRegister(AUDIO_dac0_1_src_sel_reg);
	dac0_1_src_sel_REG.dac1_pcm_source_sel = sel;
	AIO_WriteRegister(AUDIO_dac0_1_src_sel_reg, dac0_1_src_sel_REG.regValue);

	return AIO_OK;
}
#endif

/**
 * Get DAC0 pcm source.
 *
 * @param <p_dac0_sel>	{ AIO_DAC0_PCM_PB_BF_AVC1,
 *                        AIO_DAC0_PCM_PB_BF_AVC2,
 *                        AIO_DAC0_PCM_PB_AF_AVC1,
 *                        AIO_DAC0_PCM_PB_AF_AVC2,
 *                        AIO_DAC0_PCM_PB_CH23_AF_AVC2,
 *                        AIO_DAC0_PCM_PB_CH45_AF_AVC2,
 *                        AIO_DAC0_PCM_PB_CH67_AF_AVC2,
 *                        AIO_DAC0_PCM_TS_BF_DVC,
 *                        AIO_DAC0_PCM_TS_AF_DVC,
 *                        AIO_DAC0_CH01_OF_AO_HDMI_8CH_PCM,
 *                        AIO_DAC0_CH23_OF_AO_HDMI_8CH_PCM,
 *                        AIO_DAC0_CH45_OF_AO_HDMI_8CH_PCM,
 *                        AIO_DAC0_CH67_OF_AO_HDMI_8CH_PCM }
 * @return		{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_AioGetPcmSrc_DAC0(AIO_DAC0_PCM_SRC *p_dac0_sel)
{
	int ret;
	audio_dac0_1_src_sel_RBUS dac0_1_src_sel_REG;

	dac0_1_src_sel_REG.regValue = AIO_ReadRegister(AUDIO_dac0_1_src_sel_reg);
	ret = dac0_1_src_sel_REG.dac0_pcm_source_sel;
	*p_dac0_sel = (AIO_DAC0_PCM_SRC)ret;

	return AIO_OK;
}

/**
 * Get DAC1 pcm source.
 *
 * @param <p_dac1_sel>	{ AIO_DAC0_PCM_PB_BF_AVC1,
			  AIO_DAC0_PCM_PB_BF_AVC2,
			  AIO_DAC0_PCM_PB_AF_AVC1,
			  AIO_DAC0_PCM_PB_AF_AVC2,
			  AIO_DAC0_PCM_PB_CH23_AF_AVC2,
			  AIO_DAC0_PCM_PB_CH45_AF_AVC2,
			  AIO_DAC0_PCM_PB_CH67_AF_AVC2,
			  AIO_DAC0_PCM_TS_BF_DVC,
			  AIO_DAC0_PCM_TS_AF_DVC,
			  AIO_DAC0_CH01_OF_AO_HDMI_8CH_PCM,
			  AIO_DAC0_CH23_OF_AO_HDMI_8CH_PCM,
			  AIO_DAC0_CH45_OF_AO_HDMI_8CH_PCM,
			  AIO_DAC0_CH67_OF_AO_HDMI_8CH_PCM }
 * @return		{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_AioGetPcmSrc_DAC1(AIO_DAC1_PCM_SRC *p_dac1_sel)
{
	int ret;
	audio_dac0_1_src_sel_RBUS dac0_1_src_sel_REG;

	dac0_1_src_sel_REG.regValue = AIO_ReadRegister(AUDIO_dac0_1_src_sel_reg);
	ret = dac0_1_src_sel_REG.dac1_pcm_source_sel;
	*p_dac1_sel = (AIO_DAC1_PCM_SRC)ret;

	return AIO_OK;
}

/**
 * Select I2S_OUT source.
 *
 * @param <src>	{ AIO_I2SO_FIFO / AIO_I2SO_IN_SLAVE / AIO_I2SO_IN_MASTER / AIO_I2SO_IN_HDMI / AIO_I2SO_DISABLE }
 * @return		{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_AioSetOptSrc_I2S(AIO_I2SO_SRC src)
{
	audio_i2s_out_RBUS I2SO_CTRL_REG;

	I2SO_CTRL_REG.regValue = AIO_ReadRegister(AUDIO_i2s_out_reg);

	I2SO_CTRL_REG.output_sel = src;

	AIO_WriteRegister(AUDIO_i2s_out_reg, I2SO_CTRL_REG.regValue);

	return AIO_OK;
}

/**
 * Get I2S_OUT source info.
 *
 * @param <p_i2s_src>	{ AIO_I2SO_FIFO / AIO_I2SO_IN_SLAVE / AIO_I2SO_IN_MASTER / AIO_I2SO_IN_HDMI / AIO_I2SO_DISABLE }
 * @return				{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_AioGetOptSrc_I2S(AIO_I2SO_SRC *p_i2s_src)
{
	audio_i2s_out_RBUS I2SO_CTRL_REG;

	I2SO_CTRL_REG.regValue = AIO_ReadRegister(AUDIO_i2s_out_reg);

	*p_i2s_src = (AIO_I2SO_SRC)I2SO_CTRL_REG.output_sel;

	return AIO_OK;
}

/**
 * Select SPDIF_OUT source.
 *
 * @param <src>	{ AIO_SPDIFO_FIFO / AIO_SPDIFO_IN / AIO_SPDIFO_HDMI / AIO_SPDIFO_DISABLE }
 * @return		{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_AioSetOptSrc_SPDIF(AIO_SPDIFO_SRC src)
{
	audio_spdif_out_RBUS SPDIFO_CTRL_REG;

	SPDIFO_CTRL_REG.regValue = AIO_ReadRegister(AUDIO_spdif_out_reg);
	SPDIFO_CTRL_REG.output_sel = src;
	AIO_WriteRegister(AUDIO_spdif_out_reg, SPDIFO_CTRL_REG.regValue);

	return AIO_OK;
}

/**
 * Get SPDIF_OUT source info.
 *
 * @param <p_spdif_src>	{ AIO_SPDIFO_FIFO / AIO_SPDIFO_IN / AIO_SPDIFO_HDMI / AIO_SPDIFO_DISABLE }
 * @return				{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_AioGetOptSrc_SPDIF(AIO_SPDIFO_SRC *p_spdif_src)
{
	audio_spdif_out_RBUS SPDIFO_CTRL_REG;

	SPDIFO_CTRL_REG.regValue = AIO_ReadRegister(AUDIO_spdif_out_reg);

	*p_spdif_src = (AIO_SPDIFO_SRC)SPDIFO_CTRL_REG.output_sel;

	return AIO_OK;
}

/**
 * * !!!!!!!!!!!! DO NOT REMOVE static declaration !!!!!!!!!!!!!
 * !!!!! this function scope is LIMITED within this file !!!!!
 *
 * Select Headphone output source.
 * @param <src>	{ AIO_HP_MUTE_ALL, AIO_HP_PB_DAC0, AIO_HP_PB_DAC1, AIO_HP_ADCFE1, AIO_HP_ADCFE2
 * @return		{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
static int32_t Audio_AioSetOptSrc_Headphone(AIO_HP_SRC mix_src)
{
	/* !!!!!!!!!!!! DO NOT REMOVE static declaration !!!!!!!!!!!!! */
	/* !!!!! this function scope is LIMITED within this file !!!!! */

	audio_analog_aio_hpout_lineout_l_RBUS AIO_L_SEL_REG;
	audio_analog_aio_hpout_lineout_r_RBUS AIO_R_SEL_REG;

	AIO_L_SEL_REG.regValue = AIO_ReadRegister(AUDIO_analog_aio_Hpout_Lineout_L_reg);
	AIO_R_SEL_REG.regValue = AIO_ReadRegister(AUDIO_analog_aio_Hpout_Lineout_R_reg);

	AIO_L_SEL_REG.reg_bb_hpo_mutel = (~mix_src)&AIO_HP_MUTE_ALL;
	AIO_R_SEL_REG.reg_bb_hpo_muter = (~mix_src)&AIO_HP_MUTE_ALL;

	if ((mix_src&AIO_HP_MUTE_ALL) == AIO_HP_MUTE_ALL) {
		AIO_L_SEL_REG.reg_bb_hpo_mutel |= AIO_HP_MUTE_ALL;
		AIO_R_SEL_REG.reg_bb_hpo_muter |= AIO_HP_MUTE_ALL;
	}

	AIO_WriteRegister(AUDIO_analog_aio_Hpout_Lineout_L_reg, AIO_L_SEL_REG.regValue);
	AIO_WriteRegister(AUDIO_analog_aio_Hpout_Lineout_R_reg, AIO_R_SEL_REG.regValue);

	return AIO_OK;
}

/**
 * Config Headphone output
 *
 * @param <hp_cfg>	{}
 * @return		{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_AioCfgOptSrc_Headphone(AIO_HEADPHONE_CFG hp_cfg)
{
	AIO_DAC0_PCM_SRC dac0_sel;
	AIO_DAC1_PCM_SRC dac1_sel;
	int32_t dac0_is_fix_vol;
	int32_t dac1_is_fix_vol;

	Audio_AioGetPcmSrc_DAC0(&dac0_sel);
	Audio_AioGetPcmSrc_DAC1(&dac1_sel);


	if ((dac0_sel == AIO_DAC0_PCM_PB_BF_AVC1)
	    || (dac0_sel == AIO_DAC0_PCM_TS_BF_DVC)
	    || (dac0_sel == AIO_DAC0_PCM_TS_AF_DVC)) {
		dac0_is_fix_vol = TRUE;
	} else {
		dac0_is_fix_vol = FALSE;
	}

	if ((dac1_sel == AIO_DAC1_PCM_PB_BF_AVC1)
	    || (dac1_sel == AIO_DAC1_PCM_TS_BF_DVC)
	    || (dac1_sel == AIO_DAC1_PCM_TS_AF_DVC)) {
		dac1_is_fix_vol = TRUE;
	} else {
		dac1_is_fix_vol = FALSE;
	}

	if (dac0_is_fix_vol == dac1_is_fix_vol) {
		alog("[AIO][HP] Warning! FAIL to config headphone!\n");
		alog("[AIO][HP] pcm-source(%d): DAC0(%d), DAC1(%d)!\n\n", dac0_is_fix_vol, dac0_sel, dac1_sel);
		return AIO_ERROR;
	}

	if (hp_cfg.isFixVol == TRUE) {
		if (dac0_is_fix_vol == TRUE) {
			// DAC0 is fix-vol path
			Audio_AioSetOptSrc_Headphone(AIO_HP_PB_DAC0);
			alog("[AIO][HP] fix-vol path (from DAC0)!\n");
		} else {
			// DAC1 is fix-vol path
			Audio_AioSetOptSrc_Headphone(AIO_HP_PB_DAC1);
			alog("[AIO][HP] fix-vol path (from DAC1)!\n");
		}
	} else {
		if (dac0_is_fix_vol == FALSE) {
			// DAC0 is vol path
			Audio_AioSetOptSrc_Headphone(AIO_HP_PB_DAC0);
			alog("[AIO][HP] vol path (from DAC0)!\n");
		} else {
			// DAC1 is vol path
			Audio_AioSetOptSrc_Headphone(AIO_HP_PB_DAC1);
			alog("[AIO][HP] vol path (from DAC1)!\n");
		}
	}

	return AIO_OK;
}

/**
 * Auther: Wu Yi-Chiao
 * Date: 2013/6/6
 * Select AIO(Lineout) output source.
 *
 * @param <src>	{ AIO_AIO_MUTE_ALL | AIO_AIO_ADCFE | AIO_AIO_PB_DAC / AIO_AIO_TS_DAC}
 * @return		{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_AioSetOptSrc_AIO(AIO_AIO2_SRC mix_src)
{
	alog("[System][AIO] USE AioSetOptSrc_AIO ERROR, This is set at Audio FW AO init\n");
#if 0
	// Wu Yi-Chiao 2013/6/6
	// Fix: Add AIO source selection API function
	audio_analog_aio_hpout_lineout_l_RBUS analog_aio_hpout_lineout_l_REG;
	audio_analog_aio_hpout_lineout_r_RBUS analog_aio_hpout_lineout_r_REG;

	analog_aio_hpout_lineout_l_REG.regValue = AIO_ReadRegister(AUDIO_analog_aio_Hpout_Lineout_L_reg);
	analog_aio_hpout_lineout_r_REG.regValue = AIO_ReadRegister(AUDIO_analog_aio_Hpout_Lineout_R_reg);

	// not all mute
	if(mix_src & AIO_AIO_PB_DAC) {
		analog_aio_hpout_lineout_l_REG.reg_bb_aio_mutel&= ~AIO_AIO_PB_DAC;
		analog_aio_hpout_lineout_r_REG.reg_bb_aio_muter&= ~AIO_AIO_PB_DAC;
	} else {
		analog_aio_hpout_lineout_l_REG.reg_bb_aio_mutel |= AIO_AIO_PB_DAC;
		analog_aio_hpout_lineout_r_REG.reg_bb_aio_muter |= AIO_AIO_PB_DAC;
	}

	if(mix_src & AIO_AIO_ADCFE) {
		analog_aio_hpout_lineout_l_REG.reg_bb_aio_mutel&= ~AIO_AIO_ADCFE;
		analog_aio_hpout_lineout_r_REG.reg_bb_aio_muter&= ~AIO_AIO_ADCFE;
	} else {
		analog_aio_hpout_lineout_l_REG.reg_bb_aio_mutel |= AIO_AIO_ADCFE;
		analog_aio_hpout_lineout_r_REG.reg_bb_aio_muter |= AIO_AIO_ADCFE;
	}

	if(mix_src & AIO_AIO_TS_DAC) {
		analog_aio_hpout_lineout_l_REG.reg_bb_aio_mutel&= ~AIO_AIO_TS_DAC;
		analog_aio_hpout_lineout_r_REG.reg_bb_aio_muter&= ~AIO_AIO_TS_DAC;
	} else {
		analog_aio_hpout_lineout_l_REG.reg_bb_aio_mutel |= AIO_AIO_TS_DAC;
		analog_aio_hpout_lineout_r_REG.reg_bb_aio_muter |= AIO_AIO_TS_DAC;
	}

	if((mix_src & AIO_AIO_MUTE_ALL) == AIO_AIO_MUTE_ALL) {
		analog_aio_hpout_lineout_l_REG.reg_bb_aio_mutel |= AIO_AIO_MUTE_ALL;
		analog_aio_hpout_lineout_r_REG.reg_bb_aio_muter |= AIO_AIO_MUTE_ALL;
	}

	AIO_WriteRegister(AUDIO_analog_aio_Hpout_Lineout_L_reg, analog_aio_hpout_lineout_l_REG.regValue);
	AIO_WriteRegister(AUDIO_analog_aio_Hpout_Lineout_R_reg, analog_aio_hpout_lineout_r_REG.regValue);
#endif
	return AIO_OK;
}

/**
 * Auther: Wu Yi-Chiao
 * Date: 2013/6/6
 * Select  Lineout (AIO) output source.
 *
 * @param <src>	{ AIO_AIO_MUTE_ALL | AIO_CLLO_ADCFE | AIO_CLLO_PB_DAC / AIO_CLLO_TS_DAC}
 * @return		{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_AioSetOptSrc_Lineout(AIO_LO_SRC mix_src)
{
	alog("[System][AIO] USE AioSetOptSrc_Lineout ERROR, This is set at Audio FW AO init\n");
#if 0
	// Wu Yi-Chiao 2013/6/6
	// Fix: Add  Lineout (AIO) source selection API function

	Audio_AioSetOptSrc_AIO((AIO_AIO2_SRC) mix_src);
#endif
	return AIO_OK;
}

#if 0 // [MAC] obsolete
/**
 * Select PB_DAC_Mixer output.
 * HWP-Post-FIFO always output to Mixer and user can add AMIX_1 to Mixer output.
 *
 * @param <src>	{ AIO_DACMIX_AMIX_1 }
 * @return		{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_AioSetOptSrc_DACMixer(uint32_t mix_src){
	audio_analog_aio_master_hpout_lineout_RBUS HP_LINEOUT_MIX_REG;

	HP_LINEOUT_MIX_REG.regValue = AIO_ReadRegister(AUDIO_ANALOG_AIO_MASTER_HPOUT_LINEOUT_VADDR);

	if(mix_src & AIO_DACMIX_AMIX_1) {
		HP_LINEOUT_MIX_REG.reg_bb_m_master_l &= ~AIO_DACMIX_AMIX_1;
		HP_LINEOUT_MIX_REG.reg_bb_m_master_r &= ~AIO_DACMIX_AMIX_1;
	} else {
		HP_LINEOUT_MIX_REG.reg_bb_m_master_l |= AIO_DACMIX_AMIX_1;
		HP_LINEOUT_MIX_REG.reg_bb_m_master_r |= AIO_DACMIX_AMIX_1;
	}

	AIO_WriteRegister(AUDIO_ANALOG_AIO_MASTER_HPOUT_LINEOUT_VADDR, HP_LINEOUT_MIX_REG.regValue);

	return AIO_OK;
}
#endif

/**
 * Select Headphone output channel configuration.
 *
 * @param <sel>	{ AIO_OPT_CHANNEL_R_TO_L |AIO_OPT_CHANNEL_STEREO | AIO_OPT_CHANNEL_LR_SWAP | AIO_OPT_CHANNEL_L_TO_R}
 * @return		{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */

int32_t Audio_AioSetOptChannel_Headphone(AIO_OPT_CHANNEL sel)
{
#ifdef RTD289X
	// Clayton 2013/5/22
	// Add HPout COPY/SWAP function
	audio_analog_out_copy_swap_RBUS AIO_SEL_REG;

	AIO_SEL_REG.regValue = AIO_ReadRegister(AUDIO_analog_out_copy_swap_reg);

	AIO_SEL_REG.reg_bb_hpo_encopy = sel;

	AIO_WriteRegister(AUDIO_analog_out_copy_swap_reg, AIO_SEL_REG.regValue);
#endif
	return AIO_OK;
}

/**
 * Select AIO(Lineout) output channel configuration.
 *
 * @param <sel>	{ AIO_OPT_CHANNEL_R_TO_L |AIO_OPT_CHANNEL_STEREO | AIO_OPT_CHANNEL_LR_SWAP | AIO_OPT_CHANNEL_L_TO_R}
 * @return		{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_AioSetOptChannel_AIO1(AIO_OPT_CHANNEL sel)
{
#ifdef RTD289X
	// Clayton 2013/12/30
	// Add AIO1 COPY/SWAP function
	audio_analog_out_copy_swap_RBUS AIO_SEL_REG;

	AIO_SEL_REG.regValue = AIO_ReadRegister(AUDIO_analog_out_copy_swap_reg);

	AIO_SEL_REG.reg_bb_aio1_encopy = sel;

	AIO_WriteRegister(AUDIO_analog_out_copy_swap_reg, AIO_SEL_REG.regValue);
#endif
	return AIO_OK;
}

/**
 * Select AIO(Lineout) output channel configuration.
 *
 * @param <sel>	{ AIO_OPT_CHANNEL_R_TO_L |AIO_OPT_CHANNEL_STEREO | AIO_OPT_CHANNEL_LR_SWAP | AIO_OPT_CHANNEL_L_TO_R}
 * @return		{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_AioSetOptChannel_AIO2(AIO_OPT_CHANNEL sel)
{
#ifdef RTD289X
	// Clayton 2013/12/30
	// Add AIO2 COPY/SWAP function
	audio_analog_out_copy_swap_RBUS AIO_SEL_REG;

	AIO_SEL_REG.regValue = AIO_ReadRegister(AUDIO_analog_out_copy_swap_reg);

	AIO_SEL_REG.reg_bb_aio2_encopy = sel;

	AIO_WriteRegister(AUDIO_analog_out_copy_swap_reg, AIO_SEL_REG.regValue);
#endif
	return AIO_OK;
}

/**
 * Select DAC0 configuration.
 *
 * @param <sel>	{ DIO_OPT_CHANNEL_STEREO |DIO_OPT_CHANNEL_L_TO_R | DIO_OPT_CHANNEL_R_TO_L | DIO_OPT_CHANNEL_LR_SWAP}
 * @return		{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_DIOSetOptChannel_DAC0(DIO_OPT_CHANNEL sel)
{
	audio_dac0_1_src_sel_RBUS dac0_1_src_sel;

	dac0_1_src_sel.regValue = AIO_ReadRegister(AUDIO_dac0_1_src_sel_reg);

	dac0_1_src_sel.dac0_swap_sel = sel;

	AIO_WriteRegister(AUDIO_dac0_1_src_sel_reg, dac0_1_src_sel.regValue);

	return AIO_OK;
}

/**
 * Select DAC1 configuration.
 *
 * @param <sel>	{ DIO_OPT_CHANNEL_STEREO |DIO_OPT_CHANNEL_L_TO_R | DIO_OPT_CHANNEL_R_TO_L | DIO_OPT_CHANNEL_LR_SWAP}
 * @return		{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_DIOSetOptChannel_DAC1(DIO_OPT_CHANNEL sel)
{
	audio_dac0_1_src_sel_RBUS dac0_1_src_sel;

	dac0_1_src_sel.regValue = AIO_ReadRegister(AUDIO_dac0_1_src_sel_reg);

	dac0_1_src_sel.dac1_swap_sel = sel;

	AIO_WriteRegister(AUDIO_dac0_1_src_sel_reg, dac0_1_src_sel.regValue);

	return AIO_OK;
}

/**
 * Select I2S out configuration.
 *
 * @param <sel>	{ DIO_OPT_CHANNEL_STEREO |DIO_OPT_CHANNEL_L_TO_R | DIO_OPT_CHANNEL_R_TO_L | DIO_OPT_CHANNEL_LR_SWAP}
 * @param <ch_sel>	{DIO_I2S_CH_01, DIO_I2S_CH_23, DIO_I2S_CH_45, DIO_I2S_CH_67}
 * @return		{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_DIOSetOptChannel_I2S(DIO_OPT_CHANNEL sel, uint16_t ch_sel)
{
	audio_i2s_out_sel_RBUS i2s_out_sel;

	i2s_out_sel.regValue = AIO_ReadRegister(AUDIO_i2s_out_sel_reg);

	if (ch_sel&DIO_I2S_CH_01) {
		i2s_out_sel.swap_sel_ch01 = sel;
	}
#ifdef SUPPORT_8_CHANNEL_I2S_OUT
	if (ch_sel&DIO_I2S_CH_23) {
		i2s_out_sel.swap_sel_ch23 = sel;
	}

	if (ch_sel&DIO_I2S_CH_45) {
		i2s_out_sel.swap_sel_ch45 = sel;
	}

	if (ch_sel&DIO_I2S_CH_67) {
		i2s_out_sel.swap_sel_ch67 = sel;
	}
#endif
	AIO_WriteRegister(AUDIO_i2s_out_sel_reg, i2s_out_sel.regValue);

	return AIO_OK;
}

/**
 * Select SPDIF out configuration.
 *
 * @param <sel>	{ DIO_OPT_CHANNEL_STEREO |DIO_OPT_CHANNEL_L_TO_R | DIO_OPT_CHANNEL_R_TO_L | DIO_OPT_CHANNEL_LR_SWAP}
 * @return		{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_DIOSetOptChannel_SPDIF(DIO_OPT_CHANNEL sel)
{

	audio_spdif_out_RBUS spdif_out;

	spdif_out.regValue = AIO_ReadRegister(AUDIO_spdif_out_reg);

	spdif_out.swap_sel = sel;

	AIO_WriteRegister(AUDIO_spdif_out_reg, spdif_out.regValue);

	return AIO_OK;
}

/**
 * Select DAC0/DAC1/I2S out/SPDIF out configuration.
 *
 * @param <sel>	{ DIO_OPT_CHANNEL_STEREO |DIO_OPT_CHANNEL_L_TO_R | DIO_OPT_CHANNEL_R_TO_L | DIO_OPT_CHANNEL_LR_SWAP}
 * @return		{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_DIOSetOptChannel_ALL(DIO_OPT_CHANNEL sel)
{
	uint16_t i2s_channel_sel = DIO_I2S_CH_01|DIO_I2S_CH_23|DIO_I2S_CH_45|DIO_I2S_CH_67;

	Audio_DIOSetOptChannel_DAC0(sel);
	Audio_DIOSetOptChannel_DAC1(sel);
	Audio_DIOSetOptChannel_I2S(sel, i2s_channel_sel);
	Audio_DIOSetOptChannel_SPDIF(sel);

	return AIO_OK;
}

/**
 * Mute/Unmute SPDIF output.
 *
 * @param <para>	{ AIO_ENABLE / AIO_DISABLE }
 * @return			{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_AioMute_SPDIF(bool para)
{
	audio_spdif_out_RBUS SPDIFO_CTRL_REG;

	SPDIFO_CTRL_REG.regValue = AIO_ReadRegister(AUDIO_spdif_out_reg);
	SPDIFO_CTRL_REG.spdif_out_null_burst = para;
	AIO_WriteRegister(AUDIO_spdif_out_reg, SPDIFO_CTRL_REG.regValue);

	return AIO_OK;
}

/**
 * Enable/Disable Atlantic Audio Analog Modules 
 *  
 * Refer to typedef "AIO_ANA_POWER" in audio_hw_aio.h 
 * Default PW ON: bit3~14 all enable, module_idx = 0xFFFFFFFF
 * Default PW Down: bit3~14 all disable, module_idx = 0x0 
 *  
 * @param <para>		{ AIO_ENABLE / AIO_DISABLE }
 * @param <module_idx>	{ AIO_POW_XXX, refer to audio_aio.h }
 * @return				{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_AioSetAnalogPower(bool para, uint32_t module_idx)
{
	audio_analog_power_RBUS ANALOG_POW_REG;

	ANALOG_POW_REG.regValue = AIO_ReadRegister(AUDIO_analog_power_reg);
	if (para == AIO_ENABLE)
		ANALOG_POW_REG.regValue |= module_idx;
	else
		ANALOG_POW_REG.regValue &= ~module_idx;

	AIO_WriteRegister(AUDIO_analog_power_reg, ANALOG_POW_REG.regValue);

	return AIO_OK;
}

#if 0 // [MAC] obsolete
/**
 * Enable/Disable Atlantic Audio HP-Amp output
 *
 * @param <para>		{ AIO_ENABLE / AIO_DISABLE }
 * @return				{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_AioSetHeadphoneAmp(bool para){
	audio_analog_aio_master_hpout_lineout_RBUS HP_LINEOUT_MIX_REG;

	HP_LINEOUT_MIX_REG.regValue = AIO_ReadRegister(AUDIO_ANALOG_AIO_MASTER_HPOUT_LINEOUT_VADDR);
	HP_LINEOUT_MIX_REG.reg_bb_en_amp = para;
	AIO_WriteRegister(AUDIO_ANALOG_AIO_MASTER_HPOUT_LINEOUT_VADDR, HP_LINEOUT_MIX_REG.regValue);

	return AIO_OK;
}
#endif

/**
 * Configure Atlantic Audio Interface before using them.
 * This function maybe recall afer resuming from power-saving mode.
 * If the sequence of resuming from power-saving is the same with system-power-on,
 * this function can be replaced by audio_AioInit()
 *
 * @return				{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_AioConfig(void)
{
	return AIO_OK;
}

int32_t Audio_AioSemInit(void)
{
	return 0;
}

void Audio_AioSemDeInit(void)
{
	return;
}

int32_t Audio_HW_AQ_Init(ATV_CFG *p_atv_cfg)
{
	Audio_AioInit(p_atv_cfg);
	Audio_AppInit();
	return AIO_OK;
}

int32_t Audio_HW_AQ_DeInit(void)
{
	Audio_AioDeInit();
	Audio_AppDeInit();
	return AIO_OK;
}

/**
 * Init. Atlantic Audio Interface
 *
 * @return		{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_AioInit(ATV_CFG *p_atv_cfg)
{
#ifdef ENABLE_TVSERVER_STR_SWITCH
	// linmc_20160205, prevent creating another thread after resuming from STR.
	if (AioThreadState == true)
		return AIO_OK;
#endif
	//	audio_global_ctrl_RBUS		GLOBAL_CTRL_REG;
	//	audio_sys_clken2_RBUS			SYS_CLKEN2_REG;
	//Frank 20140718 Mac3 no this register
	//audio_post_peak_contorl_RBUS	POST_PEAK_CTRL_REG;
	//audio_ad_asrc_ip_RBUS	AD_ASRC_IP_REG;
	//	audio_down_sample_fifo_source_sel_RBUS	FIFO_SEL_REG;
	//	audio_pll512_fs1_RBUS			PLL_512FS1_REG;
	//	audio_pllaud_ps_RBUS			PLL_AUDPS_REG;
	//audio_peak_contorl_RBUS		PEAK_CTRL_REG;
	//audio_analog_capless_lineout_control_RBUS	ANALOG_CAPLESS_LINEOUT_CONTROL_REG;
	int32_t i;
	earcmisc_earc_ana_0_RBUS EARC_ANA_0;

	EARC_ANA_0.regValue = AIO_ReadRegister(EARCMISC_earc_ana_0_reg);
	EARC_ANA_0.pow_earc_driver = 0;
	//AIO_WriteRegister(EARCMISC_earc_ana_0_reg, EARC_ANA_0.regValue);
	//	uint32_t					flag = 0;
	//	ASE_GNL_CONFIG			ase_gnl_cfg;

	/*
	 * sram ,clk and power should be turn-on before power-on-music stage,
	 * but they may be turn-off after into power-save mode.
	 */
	/* ROS
		SYS_CLKEN2_REG.regValue = AIO_ReadRegister(SYSTEM_SYS_CLKEN2_VADDR);
		alog("[CLKEN2] = 0x%08x\n",SYS_CLKEN2_REG.regValue);
		SYS_CLKEN2_REG.clken_audio = AIO_ENABLE;
		SYS_CLKEN2_REG.clken_audio_daad_128fs = AIO_ENABLE;
		SYS_CLKEN2_REG.clken_audio_daad_256fs = AIO_ENABLE;
		SYS_CLKEN2_REG.clken_audio_pre512fs = AIO_ENABLE;
		AIO_WriteRegister(SYSTEM_SYS_CLKEN2_VADDR, SYS_CLKEN2_REG.regValue);
		alog("[CLKEN2] = 0x%08x\n",SYS_CLKEN2_REG.regValue);
	*/
	/*
		SYS_CLKEN3_REG.regValue = AIO_ReadRegister(SYSTEM_SYS_CLKEN3_VADDR);
		SYS_CLKEN3_REG.clken_audio2_512fs = AIO_ENABLE;
		SYS_CLKEN3_REG.clken_audio2_daad_128fs = AIO_ENABLE;
		SYS_CLKEN3_REG.clken_audio2_daad_256fs = AIO_ENABLE;
		AIO_WriteRegister(SYSTEM_SYS_CLKEN3_VADDR, SYS_CLKEN3_REG.regValue);
	*/
	/* ROS
		PLL_512FS1_REG.regValue = AIO_ReadRegister(SYSTEM_PLL512_FS1_VADDR);
		alog("[PLL512_1] = 0x%08x\n",PLL_512FS1_REG.regValue);
		PLL_512FS1_REG.pll512fs_rstb = 1;	// [0,1] = [reset,normal]
		PLL_512FS1_REG.pll512fs_pwdn = 0;	// [0,1] = [pow_on,pow_off]
		PLL_512FS1_REG.pll512fs_oeb  = 0;	// [0,1] = [output_enable,output_disable]
		AIO_WriteRegister(SYSTEM_PLL512_FS1_VADDR, PLL_512FS1_REG.regValue);
		alog("[PLL512_1] = 0x%08x\n",PLL_512FS1_REG.regValue);
	*/
	/* ROS
		PLL_AUDPS_REG.regValue = AIO_ReadRegister(SYSTEM_PLLAUD_PS_VADDR);
		alog("[PLLAUDPS] = 0x%08x\n",PLL_AUDPS_REG.regValue);
		PLL_AUDPS_REG.pllaud_rstb = 1;	// [0,1] = [reset,normal]
		PLL_AUDPS_REG.pllaud_oeb1 = 0;	// [0,1] = [output_enable,output_disable]
		PLL_AUDPS_REG.pllaud_oeb2 =	0;	// [0,1] = [output_enable,output_disable]
		PLL_AUDPS_REG.pllaud_pwdn = 0;	// [0,1] = [pow_on,pow_off]
		PLL_AUDPS_REG.psaud_en   = AIO_ENABLE;
		PLL_AUDPS_REG.psaud_psen = AIO_ENABLE;
		AIO_WriteRegister(SYSTEM_PLLAUD_PS_VADDR, PLL_AUDPS_REG.regValue);
		alog("[PLLAUDPS] = 0x%08x\n",PLL_AUDPS_REG.regValue);
	*/
	/* ROS enable
		GLOBAL_CTRL_REG.regValue = AIO_ReadRegister(AUDIO_GLOBAL_CTRL_VADDR);
		alog("[GLOBAL] = 0x%08x\n",GLOBAL_CTRL_REG.regValue);
		GLOBAL_CTRL_REG.adc_sram_on = AIO_ENABLE;
		GLOBAL_CTRL_REG.dac_sram_on = AIO_ENABLE;
		GLOBAL_CTRL_REG.am_sram_on  = AIO_ENABLE;
		GLOBAL_CTRL_REG.a2_eiaj_sram_on = AIO_ENABLE;
		GLOBAL_CTRL_REG.sif_adc_clk_en = AIO_ENABLE;
		AIO_WriteRegister(AUDIO_GLOBAL_CTRL_VADDR, GLOBAL_CTRL_REG.regValue);
		alog("[GLOBAL] = 0x%08x\n",GLOBAL_CTRL_REG.regValue);
	*/

	// peak detection
	//PEAK_CTRL_REG.regValue = AIO_ReadRegister(AUDIO_peak_contorl_reg);
	//PEAK_CTRL_REG.pdetect_enable = TRUE;
	//AIO_WriteRegister(AUDIO_peak_contorl_reg, PEAK_CTRL_REG.regValue);
	//Frank 20140718 Mac3 no this register
	//POST_PEAK_CTRL_REG.regValue = AIO_ReadRegister(AUDIO_POST_PEAK_CONTORL_VADDR);
	//POST_PEAK_CTRL_REG.post_detect_enable = TRUE;
	//AIO_WriteRegister(AUDIO_POST_PEAK_CONTORL_VADDR,POST_PEAK_CTRL_REG.regValue);

	// Disable ADC LPF bypass mode, let ADC LPF enable due to better performance
	/*
		// [Clayton] 2014/5/28
		// Optimize ADC performance(1) By pass LPF in ASRC (2) Add -1dB compensated gain to avoid truncation
		// [start]
		AD_ASRC_IP_REG.regValue = AIO_ReadRegister(AUDIO_AD_ASRC_IP_VADDR);
		AD_ASRC_IP_REG.ad_comp_gain = AIO_AD_COMP_GAIN_MINUS_1DB;
		AD_ASRC_IP_REG.adc_src_lpf_disable = 1; // By pass LPF in ASRC
		AIO_WriteRegister(AUDIO_AD_ASRC_IP_VADDR,AD_ASRC_IP_REG.regValue);
		// [end]
	*/
	//Audio_AioSetHeadphoneAmp(AIO_ENABLE); // [MAC] obsolete
	/* yychao- : unmute reg6910 in depop flow to avoid pop sound if unmute here
		Audio_AioSetOptSrc_Headphone(AIO_HP_PB_DAC);
		Audio_AioMute_Lineout(AIO_DISABLE);
		Audio_AioMute_Headphone(AIO_DISABLE);
	*/

	// TODO: replaced by API
	//	FIFO_SEL_REG.fifo_ch1_source = 0; // ch1,ch2,(ch1+ch2)/2,(ch1-ch2)/2
	//FIFO_SEL_REG.fifo_ch2_source = 1; // ch1,ch2,(ch1+ch2)/2,(ch1-ch2)/2

	//	FIFO_SEL_REG.regValue = AIO_ReadRegister(AUDIO_down_sample_fifo_source_sel_reg);
	//	FIFO_SEL_REG.fifo_ch12_scale = 5; // 0dB
	//	FIFO_SEL_REG.fifo_ch34_scale = 5; // 0dB
	//	FIFO_SEL_REG.fifo_ch56_scale = 5; // 0dB
	//	FIFO_SEL_REG.fifo_ch78_scale = 5; // 0dB
	//	AIO_WriteRegister(AUDIO_down_sample_fifo_source_sel_reg,FIFO_SEL_REG.regValue);

	alog("init aio driver -- start ----\n");
	alog("g_user_config = %d\n", g_user_config);
	//	m_hdmi_state     = 0;

	for (i = 0; i < AIO_AMPMUTE_UNKNOWN; i++) {
		m_ampmute[i] = 0; // init: set m_ampmute unmute and set m_ampmute_backup as APP_CH_ID_ALL
	}

	//ioctl(audio_fd, AIO_CMD_INIT_DRIVER, flag);

	alog("init aio driver -- end ------\n");

	if(g_user_config == AUDIO_USER_CONFIG_TV002)
	{
		//Audio_AmwInit(NULL);
	Audio_AppInit();
	Audio_AtvInit(p_atv_cfg);
	}

	// initialization for sound event general config.
	//ase_gnl_cfg.delay_ms		= 0;
	//ase_gnl_cfg.flow_pct_main	= 100;
	//Audio_SoundEventInit(&ase_gnl_cfg);

	/*
	 * because HDMI source will hold CPU too long,we force thread priority to highest.
	 */

	/*	audio_aio_thread_param.sched_priority = sched_get_priority_max(SCHED_RR);
	    pthread_attr_setinheritsched(&audio_aio_thread_attr, PTHREAD_EXPLICIT_SCHED);
	#ifndef DISABLE_SCHED_RR
	    pthread_attr_setschedpolicy(&audio_aio_thread_attr, SCHED_RR);
	#endif
	    pthread_attr_setschedparam(&audio_aio_thread_attr, &audio_aio_thread_param);
	*/

	if (IS_ERR_OR_NULL(audio_aio_thread)) {
		AioThreadState = false;
		audio_aio_thread = kthread_run(AioThread, NULL, "AioThread%d", 0);
		if (IS_ERR(audio_aio_thread))
			alog_err("audio_aio_thread creation failed\n");
		else
			alog_info("audio_aio_thread creation ok\n");
		}

	// testing
	//Audio_AioSetIptNoiseReduction(AIO_ENABLE, NULL); // parisfu-

	// for contorl/patch ain_element
	//Audio_ElmCreate(AUDIO_ELM_TYPE_AIN, 0, (void**)&m_ain_handle);

	return AIO_OK;
}

/**
 * DeInit. Atlantic Audio Interface
 * call this function before into power-save/power-down mode.
 *
 * @return				{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_AioDeInit(void)
{
	int32_t res;
	audio_analog_aio_hpout_lineout_l_RBUS AIO_L_SEL_REG;
	audio_analog_aio_hpout_lineout_r_RBUS AIO_R_SEL_REG;

	Audio_AtvDeInit();
	Audio_AppDeInit();

	if (!IS_ERR(audio_aio_thread)) {
		res = kthread_stop(audio_aio_thread);
		if (res) {
			alog_info("audio_aio_thread cancellation failed %d\n", res);
			audio_aio_thread = NULL;
		} else {
			alog_info("audio_aio_thread cancellation ok\n");
		}
	}

	AIO_L_SEL_REG.regValue = AIO_ReadRegister(AUDIO_analog_aio_Hpout_Lineout_L_reg);
	AIO_R_SEL_REG.regValue = AIO_ReadRegister(AUDIO_analog_aio_Hpout_Lineout_R_reg);

	AIO_L_SEL_REG.reg_bb_hpo_mutel |= AIO_HP_MUTE_ALL;
	AIO_R_SEL_REG.reg_bb_hpo_muter |= AIO_HP_MUTE_ALL;

	AIO_L_SEL_REG.reg_bb_aio1_mutel |= AIO_AIO1_MUTE_ALL;
	AIO_R_SEL_REG.reg_bb_aio1_muter |= AIO_AIO1_MUTE_ALL;
	
#if !(defined(RTD284X) || defined(RTD287X) || defined(RTD2842) || defined(RTD2874))
	AIO_L_SEL_REG.reg_bb_aio2_mutel |= AIO_AIO2_MUTE_ALL;	
	AIO_R_SEL_REG.reg_bb_aio2_muter |= AIO_AIO2_MUTE_ALL;
	
#endif

	AIO_WriteRegister(AUDIO_analog_aio_Hpout_Lineout_L_reg, AIO_L_SEL_REG.regValue); //audio mute
	AIO_WriteRegister(AUDIO_analog_aio_Hpout_Lineout_R_reg, AIO_R_SEL_REG.regValue);

	return AIO_OK;
}

#if 0 // [MAC] obsolete
/**
 * Set AIO2 is ouput
 *
 * @param <src>	{ AIO_BBADC_AIO1 | BBADC_AIO2 | ..., refer to audio_aio.h }
 * @return		{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_SetAIO2IsOutput(uint8_t Output){
	//	audio_analog_frontend_mute_RBUS	BBADC_MUTE_CTRL_REG;
	audio_analog_in_out_sel_RBUS		AIO_SEL_REG;
	uint32_t						ana_pow_flag;

	ana_pow_flag = 0;

	//BBADC_MUTE_CTRL_REG.regValue = 0x03FFFF;		// mute all input of BBADC
	AIO_SEL_REG.regValue = AIO_ReadRegister(AUDIO_ANALOG_IN_OUT_SEL_VADDR);

	if(Output == TRUE)
	AIO_SEL_REG.reg_bb_aio2_io = TRUE; //output
	else
	AIO_SEL_REG.reg_bb_aio2_io = FALSE; //output

	AIO_WriteRegister(AUDIO_ANALOG_IN_OUT_SEL_VADDR, AIO_SEL_REG.regValue);

	return AIO_OK;
}
#endif

/**
 * register Amp Mute callback function for audio library
 *
 * @param <para>		{ callback function name }
 * @ingroup lib_audio
 */
int32_t Audio_AioSetMuteAmpFunc(MUTE_AMP p_func)
{
	Audio_MuteAmpFunc = p_func;
	return AIO_OK;
}

/**
 * Determine Mute/Unmute Amp
 *
 * @param <para>		{ AIO_ENABLE/AIO_DISABLE }
 * @param <ch_id>		{ AIO_CH_ID_L/AIO_CH_ID_R/AIO_CH_ID_ALL )
 * @param <mute_by_who>	{ AIO_AMPMUTE_ID )
 * @return				{ AIO_OK or AIO_ERROR_CODE }
 * @ingroup lib_audio
 */
int32_t Audio_AioSetMute(int32_t para, uint32_t ch_id, AIO_AMPMUTE_ID mute_by_who)
{
	// TODO:
#if 0
	uint32_t i, res;

	if(mute_by_who >= AIO_AMPMUTE_UNKNOWN){
		alog("Error! Unknown AMPMUTE_ID(%d).\n", mute_by_who);
		return AIO_ERROR;
	}

	if(para == ASE_ENABLE){
		m_ampmute[mute_by_who] |= ch_id;

		res = (m_ampmute[AIO_AMPMUTE_APP] & m_ampmute[AIO_AMPMUTE_ASE])
		| m_ampmute[AIO_AMPMUTE_HEADPHONE]
		| m_ampmute[AIO_AMPMUTE_POWERSAVING];

		alog("AmpMute ");
		for(i=0;
		    i<AIO_AMPMUTE_UNKNOWN;
		    i++) {
			alog("(%d:0x%x) ",i,m_ampmute[i]);
		}
		alog("\n[AUDIO][AIO] AMP Mute CH:%x (by ID:%d, CH:0x%x)\n", res, mute_by_who, ch_id);

		if(Audio_MuteAmpFunc == NULL){
			m_ampmute_backup = res;
			alog("Warning! Audio_MuteAmpFunc is NOT registered.\n");
			return AIO_ERROR;
		} else {
			if(res != 0)
			Audio_MuteAmpFunc(AIO_ENABLE, res);
			m_ampmute_backup = res;
		}

	} else {
		m_ampmute[mute_by_who] &= ~ch_id;

		res = (m_ampmute[AIO_AMPMUTE_APP] & m_ampmute[AIO_AMPMUTE_ASE])
		| m_ampmute[AIO_AMPMUTE_HEADPHONE]
		| m_ampmute[AIO_AMPMUTE_POWERSAVING];

		alog("AmpMute ");
		for(i=0;
		    i<AIO_AMPMUTE_UNKNOWN;
		    i++) {
			alog("(%d:0x%x) ",i,m_ampmute[i]);
		}
		alog("\n[AUDIO][AIO] AMP Unmute CH:%x (by ID:%d, CH:0x%x)\n", (m_ampmute_backup - res), mute_by_who, ch_id);

		if(Audio_MuteAmpFunc == NULL){
			m_ampmute_backup = res;
			alog("Warning! Audio_MuteAmpFunc is NOT registered.\n");
			return AIO_ERROR;
		} else {
			if((m_ampmute_backup - res) != 0)
			Audio_MuteAmpFunc(AIO_DISABLE, (m_ampmute_backup - res));
			m_ampmute_backup = res;
		}
	}
#endif
	return AIO_OK;
}

/*===================== Local Subroutine ======================*/
#ifndef TV001_BOARD
static void set_peak_src_sel(AIO_PEAK_SRC src)
{
	audio_peak_contorl_RBUS	PEAK_CTRL_REG;

	PEAK_CTRL_REG.regValue = AIO_ReadRegister(AUDIO_peak_contorl_reg);
	PEAK_CTRL_REG.detect_source_sel = src;
	alog("peak set source %x\n", PEAK_CTRL_REG.detect_source_sel);
	AIO_WriteRegister(AUDIO_peak_contorl_reg, PEAK_CTRL_REG.regValue);
}

static uint32_t AioPeakMagToDecibel(uint32_t mag)
{
	uint32_t i;

	for (i = 1; i < AIO_PEAK_MAG_TAB_SIZE; i++) {
		if (mag > aio_peak_mag_tab[i])
			return i - 1;
	}

	return AIO_PEAK_MAG_TAB_SIZE - 1;
}
#endif

//[start] eq overflow test , test eq overflow process
uint32_t isEQOverflow(void)
{
	audio_system_int_en_RBUS system_int_en_RBUS;

	system_int_en_RBUS.regValue = AIO_ReadRegister(AUDIO_System_Int_En_reg);
	return system_int_en_RBUS.eq_ovf;
}

uint32_t isLoudnessOverflow(void)
{
	audio_system_int_en_RBUS system_int_en_RBUS;

	system_int_en_RBUS.regValue = AIO_ReadRegister(AUDIO_System_Int_En_reg);
	return system_int_en_RBUS.loudness_ovf;
}

//[end]
int AioThread(void *arg)
{
	alog_info("AioThread ceate...\n");

	(void)arg;  /* suppress compile warning */

	AioThreadState = true;
	
	set_freezable();

	do {
		wait_event_freezable_timeout(audio_aio_wait,
					     kthread_should_stop(),
					     usecs_to_jiffies(AIO_PEAK_DETECT_PERIOD_US));

		if (isEQOverflow() || isLoudnessOverflow()) {
			CleanEQLoudnessOverflow();
			alog("eq & loud of \n");
		}
	} while (!kthread_should_stop());

	AioThreadState = false;

	alog_info("AioThread exiting\n");

	return 0;
}

/**
 * register Spdif Out Mute callback function for audio library
 *
 * @param <para>		{ callback function name }
 * @ingroup lib_audio
 */
void Audio_AioMuteSpdifOutFuncRegister(MUTE_SPDIF_OUT p_func)
{
	Audio_MuteSpdifOutFunc = p_func;
}

int32_t Audio_AioSetSpdifOutMute(bool para, uint32_t mute_id)
{
	if (NULL == Audio_MuteSpdifOutFunc) {
		//alog("Warning! Audio_MuteSpdifOutFunc is NOT registered.\n");
		return AIO_ERROR;
	}

	Audio_MuteSpdifOutFunc(para, mute_id);
	return AIO_OK;
}

/**
 * register AV Out Mute callback function for audio library
 *
 * @param <para>		{ callback function name }
 * @ingroup lib_audio
 */
void Audio_AioMuteAVOutFuncRegister(MUTE_AV_OUT p_func)
{
	Audio_MuteAVOutFunc = p_func;
}

int32_t Audio_AioSetAVOutMute(bool para)
{
	if (NULL == Audio_MuteAVOutFunc) {
		//alog("Warning! Audio_MuteAVOutFunc is NOT registered.\n");
		return AIO_ERROR;
	}

	Audio_MuteAVOutFunc(para);
	return AIO_OK;
}

// Clayton 2013/10/18
// Call this API to compensate the HW default gain difference of ADC
#if !(defined(RTD2874))
void AioADC0GainCompensate(void)
{
	audio_ad0_asrc_ip_RBUS ad0_asrc_ip;

	ad0_asrc_ip.regValue = AIO_ReadRegister(AUDIO_ad0_asrc_ip_reg);
	ad0_asrc_ip.ad0_gain_l = AIO_AD_ASRC_GAIN_HW_Compensate;
	ad0_asrc_ip.ad0_gain_r = AIO_AD_ASRC_GAIN_HW_Compensate;
	AIO_WriteRegister(AUDIO_ad0_asrc_ip_reg, ad0_asrc_ip.regValue);
}
#endif
#ifdef SUPPORT_2_SET_ADC
void AioADC1GainCompensate(void)
{
	audio_ad1_asrc_ip_RBUS ad1_asrc_ip;

	ad1_asrc_ip.regValue = AIO_ReadRegister(AUDIO_ad1_asrc_ip_reg);
	ad1_asrc_ip.ad1_gain_l = AIO_AD_ASRC_GAIN_HW_Compensate;
	ad1_asrc_ip.ad1_gain_r = AIO_AD_ASRC_GAIN_HW_Compensate;
	AIO_WriteRegister(AUDIO_ad1_asrc_ip_reg, ad1_asrc_ip.regValue);
}
#endif

void Audio_AioADCGainCompensate(void)
{
#if !(defined(RTD2874))
	AioADC0GainCompensate();
#endif
#ifdef SUPPORT_2_SET_ADC
	AioADC1GainCompensate();
#endif
}

AUDIO_ICVERSION Audio_GetIcVersion(void)
{
	uint32_t GET_IC_VERSION;

	GET_IC_VERSION = AIO_ReadRegister(STB_SC_VerID_reg);
	alog("GET_IC_VERSION = %x\n", GET_IC_VERSION);

	if (GET_IC_VERSION == VERSION_A_REG_CODE)
		return VERSION_A;
	else
		return VERSION_B;
}

#if 0
int32_t Audio_AioSetIptNoiseReduction(bool para, AIO_NR_CFG *p_cfg_nr){
	printf("[%s] phase out .......\n",__FUNCTION__);
	return 0;
}

void Audio_AioGetNrSetting(AIO_NR_CFG* pAIO_NR_CFG){
	printf("[%s]phase out......\n",__FUNCTION__);
}

void Audio_AioSetOptAVCFactor(uint32_t factor){
	printf("[%s]phase out.....\n",__FUNCTION__);
}
int32_t Audio_AioSetOptAVC(bool para, AIO_AVC_CFG *p_cfg_opt_avc){
	printf("[%s]phase out.....\n",__FUNCTION__);
}
#endif

#ifdef SUPPORT_EARC
extern unsigned int EARC_READ_COUNT;
void EarcTask(void)
{
	//earc_earc_cmdc_req_RBUS Earc_State;
	AIO_EARC_STATE_MACHINE_T HandshakingState = AIO_EARC_STATE_IDLE1;
	earc_earc_cmdc_irq_RBUS earc_cmdc_irq;

	//Earc_State.regValue = AIO_ReadRegister(EARC_eARC_CMDC_REQ_reg);
	earc_cmdc_irq.regValue = AIO_ReadRegister(EARC_eARC_CMDC_IRQ_reg);
	#if 0
	if (g_Earc_State.ConnectState != Earc_State.disc_state)
	{
		g_Earc_State.ConnectState = Earc_State.disc_state;
	}
	#else
	Audio_HW_Earc_GetHandshakingStatus(&HandshakingState);
	#endif

	if (AioEarcGetIrqFlag() != earc_cmdc_irq.regValue)
	{
		AioEarcSetIrqFlag(earc_cmdc_irq.regValue);
		#if 0
		if (earc_cmdc_irq.cap_chng_irq == 1)
			g_Earc_State.ReadCapReady = false;
		if (earc_cmdc_irq.start_chng_irq == 1)
			g_Earc_State.ReadLatencyReady = false;
		#endif
	}
}

int EarcThread(void *arg)
{
	alog_info("[AudioHw] EarcThread ceate...\n");
	(void)arg;  /* suppress compile warning */
	g_EarcThreadState = true;
	set_freezable();
	do
	{
		wait_event_freezable_timeout(audio_earc_wait, kthread_should_stop(),
			usecs_to_jiffies(AIO_EARC_NORMAL_PERIOD_US));
		EarcTask();
	} while (!kthread_should_stop());

	g_EarcThreadState = false;
	alog_info("EarcThread exiting\n");
	return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
void Audio_EarcTimer(struct timer_list *t)
#else
void Audio_EarcTimer(unsigned long arg)
#endif
{
	AIO_EARC_STATE_MACHINE_T HandshakingState = AIO_EARC_STATE_IDLE1;
	earc_earc_cmdc_irq_RBUS earc_cmdc_irq;
	unsigned char Earc_hpd_state = 0;
	unsigned char Hdmi_5V_State = 0;
#ifdef EARC_RESET_FSM	
	earc_earc_cmdc_req_RBUS CMDC_Tx_Req;
#endif
//Update port
//	Earc_hpd_state = drvif_Hdmi_IsHpdHigh(EARC_HDMI_PORT_NUM);
//	Hdmi_5V_State = drvif_Hdmi_Is5VHigh(EARC_HDMI_PORT_NUM);
	g_Disc2_count++;
#if IS_ENABLED(CONFIG_RTK_HDMI_RX)
	Earc_hpd_state = drvif_Hdmi_IsHpdHigh(g_EarcPort);
	Hdmi_5V_State = drvif_Hdmi_Is5VHigh(g_EarcPort);
#endif

	Audio_HW_Earc_GetHandshakingStatus(&HandshakingState);
	if ((PreHpdState == 1 && Earc_hpd_state == 0) || Hdmi_5V_State == 0)
	{
		if (g_EarcInit == 1)
		{
			if (PreHpdState == 1 && Earc_hpd_state == 0)
				alog_err("hpd, AudioHw_Earc_Reset\n");
			if (Pre5vState == 1 && Hdmi_5V_State == 0)
				alog_err("5v, AudioHw_Earc_Reset\n");

			AudioHw_Earc_Reset();
            HandshakingState = AIO_EARC_STATE_IDLE1;//fix some avr ac off/on issue: toggle 5v twice very quickly
			audio_hw_usleep(6);
		}
		if((Pre5vState==1) && (Hdmi_5V_State==0))
		{
				alog_err("5V on->off, switch to earc mode\n");		
				Audio_HW_Earc_Arc_Switch(AIO_AUDIO_EARC);
		}
	}
	
	if ((PreHpdState == 0 && Earc_hpd_state == 1 && Hdmi_5V_State == 1) || (Pre5vState == 0 && Hdmi_5V_State == 1))
	{
		if (g_EarcEn == 1)
		{
			alog_err("start to earc\n");
			AudioHw_Earc_Reset();
			audio_hw_usleep(6);
			EARC_READ_COUNT = 0;

			if(HB_CADENCE_CHANGE == true)	//Initial earc handshake, for G-CTS, heartbeat timing using 60ms				
			{						
				earc_earc_cmdc_timing1_RBUS CMDC_TIMING1;
				CMDC_TIMING1.regValue = AIO_ReadRegister(EARC_eARC_CMDC_TIMING1_reg);	
				CMDC_TIMING1.heartbeat = EARC_TIMING_HEARTBEAT_DISC;
				AIO_WriteRegister(EARC_eARC_CMDC_TIMING1_reg, CMDC_TIMING1.regValue);
				HB_CADENCE_CHANGE= false;
			}
			
			Audio_HW_Earc_Start();
		}else
		{
			alog_err("5V off->on, earc ui off, switch to arc mode\n");
			AudioEarcARCMode();
		}
	}
	
	if (PreHpdState != Earc_hpd_state)
	{
		PreHpdState = Earc_hpd_state;
	}

	if (Pre5vState != Hdmi_5V_State)
	{
		Pre5vState = Hdmi_5V_State;
	}

	if (HandshakingState == AIO_EARC_STATE_EARC)
	{
		if(HB_CADENCE_CHANGE == false)			//After DISC done, heartbeat timing back to 50ms	
		{					
			earc_earc_cmdc_timing1_RBUS CMDC_TIMING1;					
			CMDC_TIMING1.regValue = AIO_ReadRegister(EARC_eARC_CMDC_TIMING1_reg);	
			CMDC_TIMING1.heartbeat = EARC_TIMING_HEARTBEAT;					
			AIO_WriteRegister(EARC_eARC_CMDC_TIMING1_reg, CMDC_TIMING1.regValue);	
			HB_CADENCE_CHANGE= true;
		}	
		earc_cmdc_irq.regValue = AIO_ReadRegister(EARC_eARC_CMDC_IRQ_reg);
		if (AioEarcGetIrqFlag() != earc_cmdc_irq.regValue)
		{
			AioEarcSetIrqFlag(earc_cmdc_irq.regValue);
			#if 0
			if (earc_cmdc_irq.cap_chng_irq == 1)
				g_Earc_State.ReadCapReady = false;
			if (earc_cmdc_irq.start_chng_irq == 1)
				g_Earc_State.ReadLatencyReady = false;
			#endif
		}
#ifdef EARC_RESET_FSM
//heartbeat error handling
		CMDC_Tx_Req.regValue = AIO_ReadRegister(EARC_eARC_CMDC_REQ_reg);			
		if(g_heartbeart_count!=g_heartbeart_count_pre)
		{
			g_heartbeart_count_pre=g_heartbeart_count;
			g_heartbeart_to=EARC_HEARTBEAT_TO;
		}else
		{
			if(CMDC_Tx_Req.fw_req==0)
		{
			if(g_heartbeart_to==0)
			{	
				CMDC_Tx_Req.regValue = AIO_ReadRegister(EARC_eARC_CMDC_REQ_reg);					
					CMDC_Tx_Req.force_trans_state= 0x1;
				AIO_WriteRegister(EARC_eARC_CMDC_REQ_reg, CMDC_Tx_Req.regValue);
					alog_err("reset earc FSM\n");

					g_heartbeart_to=EARC_HEARTBEAT_TO;
			}else
				g_heartbeart_to--;
	}
		}
	}else{
		g_heartbeart_to=EARC_HEARTBEAT_TO;
#endif
}
	mod_timer(&Audio_EarcTimerList, jiffies + HZ/100);
}

char Audio_Earc_Timer_Init(void)
{
	char cRet = 0;

	if (g_EarcTimerInitDone == 1)
	{
		alog_err("earc timer is done\n");
		return -1;
	}
	
	if (timer_pending(&Audio_EarcTimerList))
	{
		dump_stack();
		return -1;
	}

	BUG_ON(timer_pending(&Audio_EarcTimerList));
	/* timer init*/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	timer_setup(&Audio_EarcTimerList, Audio_EarcTimer, 0);
#else
	init_timer(&Audio_EarcTimerList);
	Audio_EarcTimerList.data = ((unsigned long) 0);
	Audio_EarcTimerList.function = (void *)Audio_EarcTimer;
#endif
	Audio_EarcTimerList.expires = jiffies + HZ/100;
	/*add timer*/
	add_timer(&Audio_EarcTimerList);
	g_EarcTimerInitDone = 1;
	return cRet;
}

#include <rbus/sb2_reg.h>
#define EARC_fom_ACPU_IRQ_reg 0xb801a600
//#define eARC_FROM_ACPU_bit  (4) //merlin6
#define eARC_FROM_ACPU_bit  (7) //merlin7
//#define eARC_FROM_ACPU_mask (1<<4) //merlin6
#define eARC_FROM_ACPU_mask (1<<7) //merlin7

irqreturn_t AudioHw_Earc_Isr(int irq, void *dev_id)
{
	earc_sw_cmdc_irq_event CMDC_IRQ_STATUS;

	int32_t IrqFlag = AioEarcGetIrqFlag();
        uint32_t swint;

        swint = rtd_inl(SB2_CPU_SW_INT_2_reg);
        // 2. return IRQ_NONE, if not our business.
        if (!(swint & eARC_FROM_ACPU_mask))
        	return IRQ_NONE;

        // 3. ack the SW_INT first.
	AIO_WriteRegister(SB2_CPU_SW_INT_2_reg,eARC_FROM_ACPU_mask); //clear with write_data bit 0.
       /* put your handler */
       //alog_info("%s: get interrupt swint(0x%x), cpu_time(%llx) \n", __func__, swint, sched_clock())
	   //alog_err("earc_isr=%x\n",swint);

	CMDC_IRQ_STATUS.regValue = AIO_ReadRegister(EARC_fom_ACPU_IRQ_reg);
	//alog_err("earc_isr=%x\n",CMDC_IRQ_STATUS.regValue);	   
	//if (CMDC_IRQ_STATUS.regValue > 0)
#ifdef debug_io	
	if  (CMDC_IRQ_STATUS.lost_heartbeat_irq== 1)
	{
		CMDC_IRQ_STATUS.regValue = AIO_ReadRegister(EARC_fom_ACPU_IRQ_reg);
		CMDC_IRQ_STATUS.lost_heartbeat_irq= 0;	//dummy register clear ack irq flag
		AIO_WriteRegister(EARC_fom_ACPU_IRQ_reg, CMDC_IRQ_STATUS.regValue);	

		rtd_outl(MIS_GPIO_GP4DATO_reg,0x4001);//gp137 high
		alog_err("lost heartbeat=%x\n",CMDC_IRQ_STATUS.regValue);
	}
#endif	
	if (CMDC_IRQ_STATUS.heartbeat_ack_irq != 0)
	{

		if  (CMDC_IRQ_STATUS.heartbeat_ack_irq == 1)
		{
			CMDC_IRQ_STATUS.regValue = AIO_ReadRegister(EARC_fom_ACPU_IRQ_reg);
			CMDC_IRQ_STATUS.heartbeat_ack_irq = 0;	//dummy register clear ack irq flag
			AIO_WriteRegister(EARC_fom_ACPU_IRQ_reg, CMDC_IRQ_STATUS.regValue);
#ifdef EARC_RESET_FSM			
			g_heartbeart_count++;
#endif
			IrqFlag |= AIO_EARC_HEARTBEAT_ACK_IRQ;
			if (RunCapFlag == 1)
			{
#ifndef AUTO_READ_CAP			
				earc_earc_cmdc_req_RBUS CMDC_Tx_Req;
				earc_earc_cmdc_payload_RBUS CMDC_PayLoad;
    			earc_earc_cmdc_write_RBUS eARC_CMDC_CONTROL_2;				
				CMDC_PayLoad.regValue = AIO_ReadRegister(EARC_eARC_CMDC_PAYLOAD_reg);
				CMDC_PayLoad.device_id = AIO_EARC_CAP_DEVICE_ID;
				CMDC_PayLoad.offset = AIO_EARC_CAP_OFFSET;
				AIO_WriteRegister(EARC_eARC_CMDC_PAYLOAD_reg, CMDC_PayLoad.regValue);

			    eARC_CMDC_CONTROL_2.regValue = AIO_ReadRegister(EARC_eARC_CMDC_WRITE_reg);
    			eARC_CMDC_CONTROL_2.capa_case = 1;
    			AIO_WriteRegister(EARC_eARC_CMDC_WRITE_reg, eARC_CMDC_CONTROL_2.regValue);

				CMDC_Tx_Req.regValue = AIO_ReadRegister(EARC_eARC_CMDC_REQ_reg);
				CMDC_Tx_Req.request_case = 1; //read request
				CMDC_Tx_Req.continue_status = 1;
				CMDC_Tx_Req.fw_req = 1;
				CMDC_Tx_Req.rdata_len = 0;
				AIO_WriteRegister(EARC_eARC_CMDC_REQ_reg, CMDC_Tx_Req.regValue);
				ReadCapResult = 0;
				AudioEarcRead(&g_CapabilityData[0]);
			    eARC_CMDC_CONTROL_2.regValue = AIO_ReadRegister(EARC_eARC_CMDC_WRITE_reg);
    			eARC_CMDC_CONTROL_2.capa_case = 0;
    			AIO_WriteRegister(EARC_eARC_CMDC_WRITE_reg, eARC_CMDC_CONTROL_2.regValue);				
#else				
                             RunCapFlag = 0;				
#endif
			}
		}

	//	return IRQ_HANDLED;
	}
	if (CMDC_IRQ_STATUS.dmac_frequency_irq!= 0)
	{		
		CMDC_IRQ_STATUS.regValue = AIO_ReadRegister(EARC_fom_ACPU_IRQ_reg);
		CMDC_IRQ_STATUS.dmac_frequency_irq = 0;	//dummy register clear ack irq flag
		AIO_WriteRegister(EARC_fom_ACPU_IRQ_reg, CMDC_IRQ_STATUS.regValue);
		Audio_HW_Earc_APHY_Update((AIO_FS_ID)CMDC_IRQ_STATUS.sw_data);		
	}
	return IRQ_HANDLED;
}

int32_t Audio_HW_Earc_Init_Detection(void)
{
//	Audio_Earc_Timer_Init();
#if IS_ENABLED(CONFIG_RTK_HDMI_RX)
	PreHpdState = drvif_Hdmi_IsHpdHigh(g_EarcPort);
	Pre5vState = drvif_Hdmi_Is5VHigh(g_EarcPort);
#endif
	//AudioEarcInitPow(1);
	return AIO_OK;
}

int32_t Audio_HW_Earc_Disable_Detection(void)
{
	#if 0 //don't disable earc HPD detection
	if (g_EarcTimerInitDone == 1)
	{
		del_timer_sync(&Audio_EarcTimerList);
		g_EarcTimerInitDone = 0;
	}
	#endif
	
	AudioEarcInitPow(0);
	AudioHw_Earc_Reset();
	return AIO_OK;
}

static void AudioHw_Earc_Reset(void)
{
	//int32_t IRQ_Reg = 0;
	earc_earc_cmdc_req_RBUS CMDC_Tx;
	earc_earc_cmdc_irq_en_RBUS CMDC_IRQ_EN;

	/* disable eARC IRQ */
	CMDC_IRQ_EN.regValue = AIO_ReadRegister(EARC_eARC_CMDC_IRQ_EN_reg);
	CMDC_IRQ_EN.regValue = 0;
	AIO_WriteRegister(EARC_eARC_CMDC_IRQ_EN_reg, CMDC_IRQ_EN.regValue);

	CMDC_Tx.regValue = AIO_ReadRegister(EARC_eARC_CMDC_REQ_reg);
	CMDC_Tx.force_earc_mode=0;//clear force_earc_mode from error handling
	CMDC_Tx.request_case = 0;
	CMDC_Tx.continue_status = 0;
	CMDC_Tx.fw_req = 0;
	CMDC_Tx.cmdc_tx_en = 0;
	CMDC_Tx.hdmi_hpd = 0;
	AIO_WriteRegister(EARC_eARC_CMDC_REQ_reg, CMDC_Tx.regValue);
	
	AudioEarcInitPow(0);
	AioEarcSetIrqFlag(0);
	Audio_HW_Earc_Reset_irq();
	// disable route to SCPU
	//IRQ_Reg = AIO_ReadRegister(0xbSYS_REG_INT_CTRL_SCPU_2_reg80002A4);
	//IRQ_Reg = (IRQ_Reg & (~VERIFY_BIT31)) | VERIFY_BIT0;
	//AIO_WriteRegister(SYS_REG_INT_CTRL_SCPU_2_reg, IRQ_Reg);

	g_Earc_State.ConnectState = AIO_EARC_STATE_IDLE1;
	g_Earc_State.TransactionState = AIO_AUDIO_EARC_TRAN_IDLE;
	g_Earc_State.ReadCapReady = true;
	g_Earc_State.ReadLatencyReady = true;
	g_Earc_State.RxState = AIO_AUDIO_EARC_RX_STATUS_NONE;
	g_Earc_State.CommaTimeout = 0;
	memset(&g_CapabilityData[0], 0, 256);
	g_Latency = 0;
	g_EarcInit = 0;
}

int32_t Audio_HW_Earc_Start(void)
{
	g_EarcInit = 1;
	 AIO_WriteRegister(SYS_REG_INT_CTRL_SCPU_2_reg,VERIFY_BIT31|VERIFY_BIT0); //earc int enable
	 AIO_WriteRegister(SYS_REG_INT_CTRL_SCPU_2_reg,VERIFY_BIT4|VERIFY_BIT0);	//SB2_SCPU_ROUTING_EN
	AudioEarcInitPow(1);
	AioEarcSetIrqFlag(0);	
	g_Earc_State.ConnectState = AIO_EARC_STATE_IDLE1;
	g_Earc_State.TransactionState = AIO_AUDIO_EARC_TRAN_IDLE;
	g_Earc_State.ReadCapReady = true;
	g_Earc_State.ReadLatencyReady = true;
	g_Earc_State.CommaTimeout = 0;
	//memset(&g_CapabilityData[0], 0, 256);
	g_Latency = 0;
	AudioEarcInitCmdc();
	return AIO_OK;
}

int32_t Audio_HW_Earc_Init(void)
{
	if (IS_ERR_OR_NULL(audio_hw_earc_thread))
	{
		g_EarcThreadState = false;
		audio_hw_earc_thread = kthread_run(EarcThread, NULL, "EarcThread%d", 0);
		if (IS_ERR(audio_hw_earc_thread))
		{
			alog_err("audio_hw_earc_thread creation failed\n");
			return AIO_ERROR;
		}
		else
			alog_err("audio_hw_earc_thread creation ok\n");
	}
	else alog_err("audio_hw_earc_thread err\n");

	AudioEarcInitPow(1);
	AioEarcSetIrqFlag(0);
	
	g_Earc_State.ConnectState = AIO_EARC_STATE_IDLE1;
	g_Earc_State.TransactionState = AIO_AUDIO_EARC_TRAN_IDLE;
	g_Earc_State.ReadCapReady = true;
	g_Earc_State.ReadLatencyReady = true;
	g_Earc_State.CommaTimeout = 0;
	memset(&g_CapabilityData[0], 0, 256);
	g_Latency = 0;
	AudioEarcInitCmdc();
	return AIO_OK;
}

int32_t Audio_HW_Earc_Deinit(void)
{
	int32_t IRQ_Reg = 0;
	earc_earc_cmdc_req_RBUS CMDC_Tx;
	earc_earc_cmdc_irq_en_RBUS CMDC_IRQ_EN;
	
	if (!IS_ERR_OR_NULL(audio_hw_earc_thread))
	{
		//res = kthread_stop(audio_hw_earc_thread);
		if (kthread_stop(audio_hw_earc_thread)==0)
		{
			alog_info("audio_aio_thread cancellation ok\n");
			audio_hw_earc_thread = NULL;
		}
		else
		{
			alog_info("audio_aio_thread cancellation failed\n");
		}
	}

	CMDC_Tx.regValue = AIO_ReadRegister(EARC_eARC_CMDC_REQ_reg);
	CMDC_Tx.cmdc_tx_en = 0;
	CMDC_Tx.hdmi_hpd = 0;
	AIO_WriteRegister(EARC_eARC_CMDC_REQ_reg, CMDC_Tx.regValue);

	/* disable eARC IRQ */
	CMDC_IRQ_EN.regValue = AIO_ReadRegister(EARC_eARC_CMDC_IRQ_EN_reg);
	CMDC_IRQ_EN.regValue = 0;
	AIO_WriteRegister(EARC_eARC_CMDC_IRQ_EN_reg, CMDC_IRQ_EN.regValue);
	
	AudioEarcInitPow(0);
	AioEarcSetIrqFlag(0);

	// disable route to SCPU
	IRQ_Reg = AIO_ReadRegister(SYS_REG_INT_CTRL_SCPU_2_reg);
	IRQ_Reg = (IRQ_Reg & (~VERIFY_BIT31)) | VERIFY_BIT0;//disable earc int
	IRQ_Reg = (IRQ_Reg & (~VERIFY_BIT4)) | VERIFY_BIT0; //SB2_SCPU_ROUTING
	AIO_WriteRegister(SYS_REG_INT_CTRL_SCPU_2_reg, IRQ_Reg);

	CMDC_Tx.regValue = AIO_ReadRegister(EARC_eARC_CMDC_REQ_reg);
	CMDC_Tx.cmdc_tx_en = 0;
	CMDC_Tx.hdmi_hpd = 0;
	AIO_WriteRegister(EARC_eARC_CMDC_REQ_reg, CMDC_Tx.regValue);

	g_Earc_State.ConnectState = AIO_EARC_STATE_IDLE1;
	g_Earc_State.TransactionState = AIO_AUDIO_EARC_TRAN_IDLE;
	g_Earc_State.ReadCapReady = true;
	g_Earc_State.ReadLatencyReady = true;
	g_Earc_State.RxState = AIO_AUDIO_EARC_RX_STATUS_NONE;
	g_Earc_State.CommaTimeout = 0;
	memset(&g_CapabilityData[0], 0, 256);
	g_Latency = 0;
	return AIO_OK;
}

int32_t Audio_HW_Earc_Get_Connect_State(AIO_EARC_STATE_T *State)
{
	if (g_Earc_State.ConnectState == AIO_EARC_STATE_IDLE1)
		*State = AIO_EARC_IDLE1;
	else if (g_Earc_State.ConnectState == AIO_EARC_STATE_IDLE2)
	{
		*State = AIO_EARC_EARC_FAIL;
	}
	else if (g_Earc_State.ConnectState == AIO_EARC_STATE_EARC)
		*State = AIO_EARC_EARC;
	else *State = AIO_EARC_EARC_HANKSHAKE;

	/* debug mode */
	if (AioEarcGetDebugFlag() == 1)
	{
		alog_err("[AudioHw][eARC]State = %d, g_Earc_State.ConnectState = %d\n", *State, g_Earc_State.ConnectState);
	}
	/* debug mode */
	return AIO_OK;
}

int32_t Audio_HW_Earc_Get_Rx_Status(AIO_AUDIO_EARC_RX_STATUS_T *RxState)
{
	if (g_Earc_State.ConnectState == AIO_EARC_STATE_EARC)
	{
		if ((AioEarcGetIrqFlag() & AIO_EARC_START_CHNG_IRQ) != 0)
		{
			g_Earc_State.RxState = AIO_AUDIO_EARC_RX_STATUS_STAT_CHNG;
			g_Earc_State.ReadLatencyReady = false;
			alog_debug("Latency Change detected!\n");
		}else g_Earc_State.RxState = AIO_AUDIO_EARC_RX_STATUS_NONE;

		if ((AioEarcGetIrqFlag() & AIO_EARC_CAP_CHNG_IRQ) != 0)
		{
			g_Earc_State.RxState = AIO_AUDIO_EARC_RX_STATUS_CAP_CHNG;
			if((g_Earc_State.ReadCapReady == true) && (EARC_READ_COUNT>5))
				EARC_READ_COUNT=3;//read capabilities data after delay 1 HB time for AP to read
			g_Earc_State.ReadCapReady = false;
			alog_debug("Cap Change detected!\n");
		}
		else g_Earc_State.RxState = AIO_AUDIO_EARC_RX_STATUS_NONE;

		*RxState = g_Earc_State.RxState;
	}
	else
	{
		*RxState = AIO_AUDIO_EARC_RX_STATUS_NONE;
	}

	/* debug mode */
	if (AioEarcGetDebugFlag() == 1)
	{
		alog_err("[AudioHw][eARC]RxState = %d\n", *RxState);
	}
	/* debug mode */
	return AIO_OK;
}

int32_t Audio_HW_Earc_Read_Transaction(AIO_AUDIO_EARC_CMDC_READ_T *Info)
{
	int ret = AIO_OK;
	//earc_earc_cmdc_req_RBUS CMDC_Tx_Req;
	//earc_earc_cmdc_payload_RBUS CMDC_PayLoad;
	earc_earc_cmdc_irq_RBUS CMDC_IRQ_STATUS;
#ifndef EARC_RESET_FSM		
	earc_earc_cmdc_irq_en_RBUS CMDC_IRQ_EN;
#endif	
	unsigned int read_cap_to_count=0;

	down(&audio_sem_earc_transaction);
	Audio_HW_Earc_ClrReadIrq();
	if (g_Earc_State.ConnectState == AIO_EARC_STATE_EARC && 
		g_Earc_State.TransactionState == AIO_AUDIO_EARC_TRAN_IDLE)
	{
		g_eARCReadData = 0;
		AioEarcSetIrqFlag((AioEarcGetIrqFlag() & (~AIO_EARC_ERR_READ_TRANS_IRQ) & (~AIO_EARC_RDATA_VALID_IRQ)
			& (~AIO_EARC_REQUEST_FIN_IRQ)));
		if (Info->Type == AIO_AUDIO_EARC_READ_CAP)
		{
			if (g_Earc_State.ReadCapReady == false && g_Earc_State.TransactionState == AIO_AUDIO_EARC_TRAN_READ_CAP_PROCESS)
			{
				int i = 0;
				
				for (i=1; i<=12; i++)
				{
					audio_hw_usleep(10*1000);
					if (g_Earc_State.ReadCapReady == true && g_Earc_State.TransactionState == AIO_AUDIO_EARC_TRAN_READ_CAP_FINISH)
					{
						memcpy(Info->ReadData, g_CapabilityData, 256);
						up(&audio_sem_earc_transaction);
						return AIO_OK;
					}
					else if (g_Earc_State.ReadCapReady == false && g_Earc_State.TransactionState == AIO_AUDIO_EARC_TRAN_READ_CAP_FALSE)
					{
						break;
					}
				}
			}
			else if (g_Earc_State.ReadCapReady == true)
			{
				memcpy(Info->ReadData, &g_CapabilityData[0], 256);
				up(&audio_sem_earc_transaction);
				return AIO_OK;
			}

			memset(&g_CapabilityData[0], 0, 256);
			g_Earc_State.TransactionState = AIO_AUDIO_EARC_TRAN_READ_CAP_PROCESS;
			RunCapFlag = 1;
			read_cap_to_count=0;
			while(1)
			{
				audio_hw_usleep(5000);
				if (RunCapFlag == 0)
				{
					break;
				}
				if(read_cap_to_count>100)//500ms timeout
				{
					alog_err("read cap timeout\n");
					ReadCapResult=1;
					break;
				}else
					read_cap_to_count++;				
			}
		
			RunCapFlag = 0;
#ifdef AUTO_READ_CAP		
	               if (AutoReadCap(Info->ReadData) == AIO_OK) {
				memcpy(&g_CapabilityData[0],Info->ReadData, 256);	
#else				   	
			if (ReadCapResult == 0)
			{
				memcpy(Info->ReadData, &g_CapabilityData[0], 256);
#endif					
				CMDC_IRQ_STATUS.regValue = AIO_ReadRegister(EARC_eARC_CMDC_IRQ_reg);
				if (CMDC_IRQ_STATUS.cap_chng_irq != 0)
				{
					CMDC_IRQ_STATUS.regValue = 0;
					CMDC_IRQ_STATUS.cap_chng_irq = 1;
					AIO_WriteRegister(EARC_eARC_CMDC_IRQ_reg, CMDC_IRQ_STATUS.regValue);
				}
				
				AioEarcSetIrqFlag((AioEarcGetIrqFlag() & (~AIO_EARC_CAP_CHNG_IRQ)));
				g_Earc_State.TransactionState = AIO_AUDIO_EARC_TRAN_IDLE;
				alog_err("[AudioHW][eARC] Read Cap Ok.\n");
				g_cap_update=1;
				/* debug mode */
				if (AioEarcGetDebugFlag() == 1)
				{
					int i = 0;

					alog_err("[AudioHw] Capability Data:\n");
					for (i=0; i<256; ++i)
					{
						alog_err("[%d]%x ", i, g_CapabilityData[i]);
					}
				}
				/* debug mode */
				ret =  AIO_OK;
			}
			else
			{
				g_Earc_State.TransactionState = AIO_AUDIO_EARC_TRAN_IDLE;
				ret =  AIO_ERROR;
			}
		}
		else if (Info->Type == AIO_AUDIO_EARC_READ_LATENCY)
		{
			earc_earc_cmdc_req_RBUS CMDC_Tx_Req;
			earc_earc_cmdc_payload_RBUS CMDC_PayLoad;

			if (g_Earc_State.ReadLatencyReady == true)
			{
				memcpy(Info->ReadData, &g_Latency, sizeof(unsigned char));
				up(&audio_sem_earc_transaction);
				return AIO_OK;
			}
			
			g_Earc_State.TransactionState = AIO_AUDIO_EARC_TRAN_READ_LATENCY_PROCESS;
			CMDC_PayLoad.regValue = AIO_ReadRegister(EARC_eARC_CMDC_PAYLOAD_reg);
			CMDC_PayLoad.device_id = AIO_EARC_LATENCY_DEVICE_ID;
			CMDC_PayLoad.offset = AIO_EARC_ERX_LATENCY_OFFSET;
			AIO_WriteRegister(EARC_eARC_CMDC_PAYLOAD_reg, CMDC_PayLoad.regValue);
			
			CMDC_Tx_Req.regValue = AIO_ReadRegister(EARC_eARC_CMDC_REQ_reg);
			CMDC_Tx_Req.request_case = 1; //read request
			CMDC_Tx_Req.continue_status = 0;	//Latency, need to set 0
			CMDC_Tx_Req.fw_req = 1;
			AIO_WriteRegister(EARC_eARC_CMDC_REQ_reg, CMDC_Tx_Req.regValue);
			if (AudioEarcRead(Info->ReadData) == AIO_OK)
			{
				g_Earc_State.TransactionState = AIO_AUDIO_EARC_TRAN_IDLE;
				g_latency_update=1;
				ret =  AIO_OK;
			}
			else
			{
				g_Earc_State.TransactionState = AIO_AUDIO_EARC_TRAN_IDLE;
				/* debug mode */
				if (AioEarcGetDebugFlag() == 1)
				{
					alog_err("[AudioHw][eARC]Read Latency Error\n");
				}
				/* debug mode */
				ret =  AIO_ERROR;
			}
		}
		else
		{
			alog_err("[AudioHw][eARC] unknown read-transaction type\n");
			ret =  AIO_ERROR;
		}
	}
	else
	{
		if (g_Earc_State.ConnectState != AIO_EARC_STATE_EARC)
			alog_err("[AudioHw][eARC]Err Read Transaction, not in eARC state\n");
		else if (g_Earc_State.TransactionState != AIO_AUDIO_EARC_TRAN_IDLE)
			alog_err("[AudioHw][eARC]Err Read Transaction, not in trans idle\n");
		
		ret =  AIO_ERROR;
	}

	AioEarcSetIrqFlag((AioEarcGetIrqFlag() & (~AIO_EARC_ERR_READ_TRANS_IRQ) & (~AIO_EARC_RDATA_VALID_IRQ)
		& (~AIO_EARC_REQUEST_FIN_IRQ)));
	up(&audio_sem_earc_transaction);
	return ret;
}

int32_t Audio_HW_Earc_Write_Latency(uint8_t WriteData)
{
	int32_t ret = AIO_OK;

	down(&audio_sem_earc_transaction);
	Audio_HW_Earc_ClrWriteIrq();
	if (g_Earc_State.ConnectState == AIO_EARC_STATE_EARC && 
		g_Earc_State.TransactionState == AIO_AUDIO_EARC_TRAN_IDLE)
	{
		earc_earc_cmdc_irq_RBUS CMDC_IRQ_STATUS;
		earc_earc_cmdc_irq_RBUS CMDC_IRQ_STATUS_2;
		earc_earc_cmdc_req_RBUS CMDC_Tx_Req;
		earc_earc_cmdc_payload_RBUS CMDC_PayLoad;
		earc_earc_cmdc_write_RBUS Write_Data;
	       earc_earc_cmdc_debug_RBUS CMDC_debug;	
		earc_earc_cmdc_req_RBUS CMDC_REQ;
		unsigned long flags;
		int i = 0,stop=0,stop_to=500;
		unsigned int FinIsrFlag = 0, ErrorFlag = 0;
		audio_hw_usleep(10*1000);
		AioEarcSetIrqFlag((AioEarcGetIrqFlag() & (~AIO_EARC_ERR_WRITE_TRANS_IRQ)));
		g_Earc_State.TransactionState = AIO_AUDIO_EARC_TRAN_WRITE_LATENCY_PROCESS;
		CMDC_PayLoad.regValue = AIO_ReadRegister(EARC_eARC_CMDC_PAYLOAD_reg);
		CMDC_PayLoad.device_id = AIO_EARC_LATENCY_DEVICE_ID;
		CMDC_PayLoad.offset = AIO_EARC_ERX_LATENCY_REQ_OFFSET;
		AIO_WriteRegister(EARC_eARC_CMDC_PAYLOAD_reg, CMDC_PayLoad.regValue);

		Write_Data.regValue = AIO_ReadRegister(EARC_eARC_CMDC_WRITE_reg);
		Write_Data.write_data = WriteData;
		AIO_WriteRegister(EARC_eARC_CMDC_WRITE_reg, Write_Data.regValue);
		
		CMDC_Tx_Req.regValue = AIO_ReadRegister(EARC_eARC_CMDC_REQ_reg);
		CMDC_Tx_Req.request_case = 0; //write request
		CMDC_Tx_Req.fw_req = 1;
		AIO_WriteRegister(EARC_eARC_CMDC_REQ_reg, CMDC_Tx_Req.regValue);
		local_irq_save(flags);
		//audio_hw_usleep(500);
		while(1)
		{
			audio_hw_usleep(1);
			CMDC_REQ.regValue = AIO_ReadRegister(EARC_eARC_CMDC_REQ_reg);
			if(CMDC_REQ.trans_state==0xa)
			{
				stop=1;
				break;
			}
			if((stop_to--)==0)
				break;
		}
		for (i=0; i<10; ++i)
		{

			CMDC_IRQ_STATUS.regValue = AIO_ReadRegister(EARC_eARC_CMDC_IRQ_reg);
			CMDC_IRQ_STATUS_2.regValue = 0;
			if (CMDC_IRQ_STATUS.request_fin_irq != 0)
			{
				CMDC_IRQ_STATUS_2.request_fin_irq = 1;
				FinIsrFlag = 1;
			}

			if (CMDC_IRQ_STATUS.lost_heartbeat_irq != 0)
			{
				CMDC_IRQ_STATUS_2.lost_heartbeat_irq = 1;
				ErrorFlag = 1;
			}

			if (CMDC_IRQ_STATUS.unexp_response_irq != 0)
			{
				CMDC_IRQ_STATUS_2.unexp_response_irq = 1;
				ErrorFlag = 1;
			}
			if (CMDC_IRQ_STATUS.unexp_noise_irq != 0)
			{
			   	CMDC_debug.regValue = AIO_ReadRegister(EARC_eARC_CMDC_DEBUG_reg);	
				alog_err("[AudioHW][eARC] unexp_noise_irq\n");	
				CMDC_IRQ_STATUS_2.unexp_noise_irq = 1;				
			}

			if (CMDC_IRQ_STATUS.no_response_irq != 0)
			{
				CMDC_IRQ_STATUS_2.no_response_irq = 1;
				ErrorFlag = 1;
			}

			if (CMDC_IRQ_STATUS.write_nack_irq != 0)
			{
				CMDC_IRQ_STATUS_2.write_nack_irq = 1;
				ErrorFlag = 1;
			}

			if (FinIsrFlag == 1 || ErrorFlag == 1)
			{
				break;
			}

			audio_hw_usleep(300);
		}

		local_irq_save(flags);
		AIO_WriteRegister(EARC_eARC_CMDC_IRQ_reg, CMDC_IRQ_STATUS_2.regValue);
		g_Earc_State.TransactionState = AIO_AUDIO_EARC_TRAN_IDLE;
		if (ErrorFlag == 1)
		{
			alog_err("[AudioHw][eARC]Err Write Transaction\n");
			ret = AIO_ERROR;
		}
	}
	else
	{
		if (g_Earc_State.ConnectState != AIO_EARC_STATE_EARC)
			alog_err("[AudioHw][eARC]Err Write Transaction, not in eARC state\n");
		else if (g_Earc_State.TransactionState != AIO_AUDIO_EARC_TRAN_IDLE)
			alog_err("[AudioHw][eARC]Err Write Transaction, not in trans idle\n");
		
		ret = AIO_ERROR;
	}

	AioEarcSetIrqFlag((AioEarcGetIrqFlag() & (~AIO_EARC_ERR_WRITE_TRANS_IRQ)));
	AioEarcSetIrqFlag((AioEarcGetIrqFlag() & (~AIO_EARC_REQUEST_FIN_IRQ)));
	up(&audio_sem_earc_transaction);
	return ret;
}


#ifdef AUTO_READ_CAP
static int32_t  AutoReadCap(unsigned char *Data)
{
    unsigned int i = 0, data_len = 0, ret = AIO_OK;
    unsigned int ValidFlag = 0, FinIsrFlag = 0, ErrorFlag = 0;
    earc_earc_cmdc_write_RBUS eARC_CMDC_CONTROL_2;
    earc_earc_cmdc_irq_RBUS CMDC_IRQ_STATUS, CMDC_IRQ_STATUS_2;
    earc_earc_cmdc_req_RBUS CMDC_Tx_Req;
    earc_earc_cmdc_payload_RBUS CMDC_PayLoad;
    earc_earc_cmdc_read_RBUS ReadData;
    earc_earc_cmdc_debug_RBUS CMDC_debug;	
	
    alog_err("Auto Read Start\n");

    eARC_CMDC_CONTROL_2.regValue = AIO_ReadRegister(EARC_eARC_CMDC_WRITE_reg);
    eARC_CMDC_CONTROL_2.auto_read_cap_en = 1;
    eARC_CMDC_CONTROL_2.capa_case = 1;
    eARC_CMDC_CONTROL_2.auto_stop_cap_en = 1;
    eARC_CMDC_CONTROL_2.first_len_index = 1;
    AIO_WriteRegister(EARC_eARC_CMDC_WRITE_reg, eARC_CMDC_CONTROL_2.regValue);

    CMDC_PayLoad.regValue = AIO_ReadRegister(EARC_eARC_CMDC_PAYLOAD_reg);
    CMDC_PayLoad.device_id = AIO_EARC_CAP_DEVICE_ID;
    CMDC_PayLoad.offset = AIO_EARC_CAP_OFFSET;
    AIO_WriteRegister(EARC_eARC_CMDC_PAYLOAD_reg, CMDC_PayLoad.regValue);

    CMDC_Tx_Req.regValue = AIO_ReadRegister(EARC_eARC_CMDC_REQ_reg);
    CMDC_Tx_Req.request_case = 1; //read request
    CMDC_Tx_Req.continue_status = 1;
    CMDC_Tx_Req.fw_req = 1;
    CMDC_Tx_Req.rdata_len = 0;
    AIO_WriteRegister(EARC_eARC_CMDC_REQ_reg, CMDC_Tx_Req.regValue);

    while (1) {
        CMDC_IRQ_STATUS.regValue = AIO_ReadRegister(EARC_eARC_CMDC_IRQ_reg);
        CMDC_IRQ_STATUS_2.regValue = 0;
        if (CMDC_IRQ_STATUS.rdata_valid_irq != 0) {
            CMDC_IRQ_STATUS_2.rdata_valid_irq = 1;
            ValidFlag = 1;
            AIO_WriteRegister(EARC_eARC_CMDC_IRQ_reg, CMDC_IRQ_STATUS_2.regValue);
            audio_hw_usleep(8);
            CMDC_IRQ_STATUS_2.regValue = 0;
        }

        if (CMDC_IRQ_STATUS.request_fin_irq != 0) {
            CMDC_IRQ_STATUS_2.request_fin_irq = 1;
            FinIsrFlag = 1;
        }

        if (CMDC_IRQ_STATUS.lost_heartbeat_irq != 0) {
            CMDC_IRQ_STATUS_2.lost_heartbeat_irq = 1;
            ErrorFlag = 1;
        }

	if (CMDC_IRQ_STATUS.unexp_response_irq != 0)
	{
		CMDC_IRQ_STATUS_2.unexp_response_irq = 1;
		ErrorFlag = 1;				
	}
	if (CMDC_IRQ_STATUS.unexp_noise_irq != 0)
	{
	   	CMDC_debug.regValue = AIO_ReadRegister(EARC_eARC_CMDC_DEBUG_reg);
		alog_err("[AudioHW][eARC] unexp_noise_irq\n");		
		CMDC_IRQ_STATUS_2.unexp_noise_irq = 1;		
	}

        if (CMDC_IRQ_STATUS.no_response_irq != 0) {
            CMDC_IRQ_STATUS_2.no_response_irq = 1;
            ErrorFlag = 1;
        }

        if (CMDC_IRQ_STATUS.read_uecc_irq != 0) {
            CMDC_IRQ_STATUS_2.read_uecc_irq = 1;
            ErrorFlag = 1;
        }

        AIO_WriteRegister(EARC_eARC_CMDC_IRQ_reg, CMDC_IRQ_STATUS_2.regValue);
        audio_hw_usleep(3);
        if (FinIsrFlag == 1 || ErrorFlag == 1) {
            break;
        }
    }


    if (ErrorFlag == 1) {
        alog_err("[AudioHW][eARC] Read Cap Fail. %x\n", CMDC_IRQ_STATUS_2.regValue);
        ret = AIO_ERROR;
        ReadCapResult = 1;
    }
    else if (FinIsrFlag == 1) {
        CMDC_PayLoad.regValue = AIO_ReadRegister(EARC_eARC_CMDC_PAYLOAD_reg);
        data_len = CMDC_PayLoad.current_index;
        //alog_err("data_len=%x\n",data_len);
        CMDC_PayLoad.read_index = 0;
        AIO_WriteRegister(EARC_eARC_CMDC_PAYLOAD_reg, CMDC_PayLoad.regValue);
	    data_len=256;
        audio_hw_usleep(10);
        eARC_CMDC_CONTROL_2.regValue = AIO_ReadRegister(EARC_eARC_CMDC_WRITE_reg);
        eARC_CMDC_CONTROL_2.read_en = 1;
        AIO_WriteRegister(EARC_eARC_CMDC_WRITE_reg, eARC_CMDC_CONTROL_2.regValue);
        audio_hw_usleep(10);
        for (i = 0; i < data_len; i += 4) {
            ReadData.regValue = AIO_ReadRegister(EARC_eARC_CMDC_READ_reg);
            //alog_err("cap data=%x\n",ReadData.regValue);

	        Data[i] = ((ReadData.regValue >> 24) & 0xFF);
            Data[i + 1] = ((ReadData.regValue >> 16) & 0xFF);
            Data[i + 2] = ((ReadData.regValue >> 8) & 0xFF);
            Data[i + 3] = (ReadData.regValue & 0xFF);
            //alog_info("cap data=%x\n",ReadData.regValue);

            CMDC_PayLoad.read_index++;
            AIO_WriteRegister(EARC_eARC_CMDC_PAYLOAD_reg, CMDC_PayLoad.regValue);
            audio_hw_usleep(10);
            eARC_CMDC_CONTROL_2.regValue = AIO_ReadRegister(EARC_eARC_CMDC_WRITE_reg);
            eARC_CMDC_CONTROL_2.read_en = 1;
            AIO_WriteRegister(EARC_eARC_CMDC_WRITE_reg, eARC_CMDC_CONTROL_2.regValue);
            audio_hw_usleep(10);
        }
	//check block length
		alog_err("Block1 ID=%x\n",Data[1]);	
		alog_err("Block1 datalen=%x\n",Data[2]);
		alog_err("Block2 ID=%x\n",Data[Data[2]+3]);
		alog_err("Block2 datalen=%x\n",Data[Data[2]+4]);
		alog_err("Block3 ID=%x\n",Data[(Data[2]+Data[Data[2]+4])+5]);
		alog_err("Block3 datalen=%x\n",Data[(Data[2]+Data[Data[2]+4])+6]);
	
        g_Earc_State.ReadCapReady = true;
        alog_err("Auto Read Done\n");
        ret = AIO_OK;
        ReadCapResult = 0;
        AioEarcSetIrqFlag(AioEarcGetIrqFlag() & (~AIO_EARC_CAP_CHNG_IRQ));
    }
    CMDC_Tx_Req.regValue = AIO_ReadRegister(EARC_eARC_CMDC_REQ_reg);
    CMDC_Tx_Req.fw_req = 0;
    CMDC_Tx_Req.continue_status = 0;
    AIO_WriteRegister(EARC_eARC_CMDC_REQ_reg, CMDC_Tx_Req.regValue);
    eARC_CMDC_CONTROL_2.regValue = AIO_ReadRegister(EARC_eARC_CMDC_WRITE_reg);
    eARC_CMDC_CONTROL_2.capa_case = 0;
    AIO_WriteRegister(EARC_eARC_CMDC_WRITE_reg, eARC_CMDC_CONTROL_2.regValue);
    return ret;
}

#endif

static int32_t AudioEarcRead(unsigned char *Data)
{
	#define RDATA_1BYTE_TIME 5 //us
	#define WAIT_VALID_IRQ_TIME 4 //us
	#define CMD_TIME	250 //us

	unsigned char FirstTimeRead = 0, Data_Len = 0, FinishFlag = 0, Total_Read_Count = 0;
	earc_earc_cmdc_req_RBUS CMDC_REQ;
	//earc_earc_cmdc_irq_en_RBUS CMDC_IRQ_EN;
	earc_earc_cmdc_irq_RBUS CMDC_IRQ_STATUS;
	earc_earc_cmdc_irq_RBUS CMDC_IRQ_STATUS_2;
	earc_earc_cmdc_debug_RBUS CMDC_debug;	
	int32_t ret = AIO_OK, Rdata_Index = 0;
	unsigned int ValidFlag = 0, FinIsrFlag = 0, ErrorFlag = 0;
	earc_earc_cmdc_read_RBUS ReadData;
	unsigned int count=0;
	//audio_hw_usleep(CMD_TIME);
	if (g_Earc_State.TransactionState == AIO_AUDIO_EARC_TRAN_READ_CAP_PROCESS)
	{
		unsigned long flags;
		
		FirstTimeRead = 1;
		local_irq_save(flags);
		while(1)
		{
			count++;
			if(count>1000)// 2ms timeout
			{
				ErrorFlag=1;
				alog_err("read cap data timeout\n");
			}
			CMDC_IRQ_STATUS.regValue = AIO_ReadRegister(EARC_eARC_CMDC_IRQ_reg);
			CMDC_IRQ_STATUS_2.regValue = 0;
			if (CMDC_IRQ_STATUS.rdata_valid_irq != 0)
			{
				ReadData.regValue = AIO_ReadRegister(EARC_eARC_CMDC_READ_reg);
				g_eARCReadData = ReadData.regValue;
				CMDC_IRQ_STATUS_2.rdata_valid_irq = 1;
				ValidFlag = 1;
				AIO_WriteRegister(EARC_eARC_CMDC_IRQ_reg, CMDC_IRQ_STATUS_2.regValue);
				audio_hw_usleep(8);
				CMDC_IRQ_STATUS_2.regValue = 0;
				//alog_err("count=%d\n",count);
				count=0;
			}

			if (CMDC_IRQ_STATUS.request_fin_irq != 0)
			{
				CMDC_IRQ_STATUS_2.request_fin_irq = 1;
				FinIsrFlag = 1;
				count=0;				
			}

			if (CMDC_IRQ_STATUS.lost_heartbeat_irq != 0)
			{
				CMDC_IRQ_STATUS_2.lost_heartbeat_irq = 1;
				ErrorFlag = 1;
			}

			if (CMDC_IRQ_STATUS.unexp_response_irq != 0)
			{
				CMDC_IRQ_STATUS_2.unexp_response_irq = 1;
					ErrorFlag = 1;
				}
			if (CMDC_IRQ_STATUS.unexp_noise_irq != 0)
			{			
			   	CMDC_debug.regValue = AIO_ReadRegister(EARC_eARC_CMDC_DEBUG_reg);			
				alog_err("[AudioHW][eARC] unexp_noise_irq\n");
				CMDC_IRQ_STATUS_2.unexp_noise_irq = 1;				
			}
			if (CMDC_IRQ_STATUS.no_response_irq != 0)
			{
				CMDC_IRQ_STATUS_2.no_response_irq = 1;
				ErrorFlag = 1;
			}

			if (CMDC_IRQ_STATUS.read_uecc_irq != 0)
			{
				CMDC_IRQ_STATUS_2.read_uecc_irq = 1;
				ErrorFlag = 1;
			}

			AIO_WriteRegister(EARC_eARC_CMDC_IRQ_reg, CMDC_IRQ_STATUS_2.regValue);
			audio_hw_usleep(2);
			//CMDC_IRQ_STATUS.regValue = AIO_ReadRegister(EARC_eARC_CMDC_IRQ_reg);
			if (ValidFlag == 1/* && CMDC_IRQ_STATUS.rdata_valid_irq == 0*/)
			{
				audio_hw_usleep(2);
				CMDC_REQ.regValue = AIO_ReadRegister(EARC_eARC_CMDC_REQ_reg);
				if (FirstTimeRead)
				{
					FirstTimeRead = 0;
					CMDC_REQ.rdata_len = 128;
					Data_Len = (CMDC_REQ.rdata_len/4);
				}

				Data[Rdata_Index] = ((g_eARCReadData >> 24) & 0xFF);
				Data[Rdata_Index+1] = ((g_eARCReadData >> 16) & 0xFF);
				Data[Rdata_Index+2] = ((g_eARCReadData >> 8) & 0xFF);
				Data[Rdata_Index+3] = (g_eARCReadData & 0xFF);
				Rdata_Index += 4;
				++ Total_Read_Count;
				g_eARCReadData = 0;	
				ValidFlag = 0;
				AioEarcSetIrqFlag(AioEarcGetIrqFlag() & (~AIO_EARC_RDATA_VALID_IRQ));
				if (Total_Read_Count < Data_Len)
				{
					CMDC_REQ.continue_status = 1;
					AIO_WriteRegister(EARC_eARC_CMDC_REQ_reg, CMDC_REQ.regValue);
					audio_hw_usleep(2);
				}
				else
				{
					FinishFlag = 1;
				}
			}
			else if (ErrorFlag == 1)
			{
				alog_err("[AudioHW][eARC] Read Cap Fail. %x\n", CMDC_IRQ_STATUS_2.regValue);
				CMDC_REQ.regValue = AIO_ReadRegister(EARC_eARC_CMDC_REQ_reg);
				CMDC_REQ.fw_req= 0;
				CMDC_REQ.continue_status = 0;
				//CMDC_REQ.capability_stop = 1;
				AIO_WriteRegister(EARC_eARC_CMDC_REQ_reg, CMDC_REQ.regValue);
				audio_hw_usleep(2);
				ret = AIO_ERROR;
				ReadCapResult = 1;				
				break;
			}
			else if (FinIsrFlag == 1 && ValidFlag == 0)
			{
				//alog_err("[AudioHW][eARC] Read Cap Ok.\n");
				CMDC_REQ.regValue = AIO_ReadRegister(EARC_eARC_CMDC_REQ_reg);
				CMDC_REQ.fw_req= 0;
				CMDC_REQ.continue_status = 0;
				//CMDC_REQ.capability_stop = 1;
				AIO_WriteRegister(EARC_eARC_CMDC_REQ_reg, CMDC_REQ.regValue);
				audio_hw_usleep(2);
				g_Earc_State.ReadCapReady = true;
				ret = AIO_OK;
				ReadCapResult = 0;
				AioEarcSetIrqFlag(AioEarcGetIrqFlag() & (~AIO_EARC_CAP_CHNG_IRQ));
				break;
			}
		}

		RunCapFlag = 0;
		local_irq_save(flags);
	}
	else if (g_Earc_State.TransactionState == AIO_AUDIO_EARC_TRAN_READ_LATENCY_PROCESS)
	{
		unsigned long flags;
		int i = 0;
		local_irq_save(flags);
		audio_hw_usleep(500);
		for (i=0; i<10; ++i)
		{
			CMDC_IRQ_STATUS.regValue = AIO_ReadRegister(EARC_eARC_CMDC_IRQ_reg);
			CMDC_IRQ_STATUS_2.regValue = 0;
			if (CMDC_IRQ_STATUS.request_fin_irq != 0)
			{
				CMDC_IRQ_STATUS_2.request_fin_irq = 1;
				FinIsrFlag = 1;
			}

			if (CMDC_IRQ_STATUS.lost_heartbeat_irq != 0)
			{
				CMDC_IRQ_STATUS_2.lost_heartbeat_irq = 1;
				ErrorFlag = 1;
			}

			if (CMDC_IRQ_STATUS.unexp_response_irq != 0)
			{
				CMDC_IRQ_STATUS_2.unexp_response_irq = 1;
				ErrorFlag = 1;
			}
			if (CMDC_IRQ_STATUS.unexp_noise_irq != 0)
			{				
			   	CMDC_debug.regValue = AIO_ReadRegister(EARC_eARC_CMDC_DEBUG_reg);			
				alog_err("[AudioHW][eARC] unexp_noise_irq\n");
				CMDC_IRQ_STATUS_2.unexp_noise_irq = 1;				
			}

			if (CMDC_IRQ_STATUS.no_response_irq != 0)
			{
				CMDC_IRQ_STATUS_2.no_response_irq = 1;
				ErrorFlag = 1;
			}

			if (CMDC_IRQ_STATUS.read_uecc_irq != 0)
			{
				CMDC_IRQ_STATUS_2.read_uecc_irq = 1;
				ErrorFlag = 1;
			}

			if (CMDC_IRQ_STATUS.rdata_valid_irq != 0)
			{
				ReadData.regValue = AIO_ReadRegister(EARC_eARC_CMDC_READ_reg);
				g_eARCReadData = ReadData.regValue;
				CMDC_IRQ_STATUS_2.rdata_valid_irq = 1;
				ValidFlag = 1;
			}

			if (FinIsrFlag == 1 || ErrorFlag == 1)
			{
				break;
			}

			audio_hw_usleep(300);
		}

		local_irq_save(flags);
		AIO_WriteRegister(EARC_eARC_CMDC_IRQ_reg, CMDC_IRQ_STATUS_2.regValue);
		if (FinIsrFlag == 1 && ErrorFlag != 1)
		{
			*Data = (g_eARCReadData & 0xFF);
			g_Latency = *Data;
			g_eARCReadData = 0;
			CMDC_REQ.regValue = AIO_ReadRegister(EARC_eARC_CMDC_REQ_reg);
			CMDC_REQ.fw_req= 0;
			CMDC_REQ.continue_status = 0;
			AIO_WriteRegister(EARC_eARC_CMDC_REQ_reg, CMDC_REQ.regValue);
			AioEarcSetIrqFlag(AioEarcGetIrqFlag() & (~AIO_EARC_START_CHNG_IRQ));
			g_Earc_State.ReadLatencyReady = true;
			if (CMDC_IRQ_STATUS.start_chng_irq != 0)
			{
				CMDC_IRQ_STATUS.regValue = 0;
				CMDC_IRQ_STATUS.start_chng_irq = 1;
				AIO_WriteRegister(EARC_eARC_CMDC_IRQ_reg, CMDC_IRQ_STATUS.regValue);
			}

			alog_err("[AudioHW][eARC] Read Latency Ok. %x\n", g_Latency);
			ret = AIO_OK;
		}
		else if (ErrorFlag == 1)	
		{
			*Data = (g_eARCReadData & 0xFF);
			g_eARCReadData = 0;
			CMDC_REQ.regValue = AIO_ReadRegister(EARC_eARC_CMDC_REQ_reg);
			CMDC_REQ.fw_req= 0;
			CMDC_REQ.continue_status = 0;
			AIO_WriteRegister(EARC_eARC_CMDC_REQ_reg, CMDC_REQ.regValue);
			alog_err("[AudioHW][eARC] Read Latency Error, %x\n", CMDC_IRQ_STATUS_2.regValue);
			ret = AIO_ERROR;
		}
	}

	/* disable eARC  IRQ of read */
	#if 0
	CMDC_IRQ_EN.regValue = AIO_ReadRegister(EARC_eARC_CMDC_IRQ_EN_reg);
	CMDC_IRQ_EN.regValue &= (~AIO_EARC_ERR_READ_TRANS_IRQ);
	AIO_WriteRegister(EARC_eARC_CMDC_IRQ_EN_reg, CMDC_IRQ_EN.regValue);
	#endif
	return ret;
}

void AioEarcSetIrqFlag(int32_t ErrFlag)
{
	g_IrqFlag = ErrFlag;
}

int32_t AioEarcGetIrqFlag(void)
{
	return g_IrqFlag;
}

void AioEarcSetDebugFlag(int32_t Flag)
{
	g_EarcDebugMode = Flag;
}

int32_t AioEarcGetDebugFlag(void)
{
	return g_EarcDebugMode;
}

static void AudioEarcInitPow(bool En)
{
	earcmisc_earc_ana_0_RBUS EARC_ANA_0;

	EARC_ANA_0.regValue = AIO_ReadRegister(EARCMISC_earc_ana_0_reg);
	if (En == true)
	{
		if (EARC_ANA_0.earc_data_sel != 1)
			EARC_ANA_0.earc_data_sel = 1; //earc path
		if (EARC_ANA_0.pow_earc_driver != 1)
			EARC_ANA_0.pow_earc_driver = 1;
	}
	else
	{
		if (EARC_ANA_0.earc_data_sel != 0 && Pre5vState == 1)
			EARC_ANA_0.earc_data_sel = 0; //arc path
		if (EARC_ANA_0.pow_earc_driver != 0)
			EARC_ANA_0.pow_earc_driver = 0;
	}

	AIO_WriteRegister(EARCMISC_earc_ana_0_reg, EARC_ANA_0.regValue);
}

static void AudioEarcInitCmdc(void)
{
	earc_earc_cmdc_req_RBUS CMDC_Tx_Req;
	earc_earc_cmdc_irq_en_RBUS CMDC_IRQ_EN;

	/* enable eARC IRQ */
	CMDC_IRQ_EN.regValue = AIO_ReadRegister(EARC_eARC_CMDC_IRQ_EN_reg);
	CMDC_IRQ_EN.heartbeat_ack_irq_en = 1;
#ifdef debug_io	
	CMDC_IRQ_EN.lost_heartbeat_irq_en=1;
#endif	
	AIO_WriteRegister(EARC_eARC_CMDC_IRQ_EN_reg, CMDC_IRQ_EN.regValue);

#ifdef debug_io //gpio137
	rtd_outl(PINMUX_GPIO_RIGHT_CFG_13_reg,0x0000f000);
	rtd_outl(MIS_GPIO_GP4DIR_reg,0x00004001);
	rtd_outl(MIS_GPIO_GP4DATO_reg,0x00004000);
#endif
	CMDC_Tx_Req.regValue = AIO_ReadRegister(EARC_eARC_CMDC_REQ_reg);
	CMDC_Tx_Req.cmdc_tx_en = 1;
	CMDC_Tx_Req.hdmi_hpd = 1;
	CMDC_Tx_Req.heartbeat_en = 1;
	AIO_WriteRegister(EARC_eARC_CMDC_REQ_reg, CMDC_Tx_Req.regValue);
}

int32_t Audio_HW_Earc_Arc_Switch(AIO_AUDIO_EARC_ARC_SWITCH_T Mode)
{
	earcmisc_earc_ana_0_RBUS EARC_ANA_0;
	earcmisc_earc_ana_2_RBUS EARC_ANA_2;

	EARC_ANA_0.regValue = AIO_ReadRegister(EARCMISC_earc_ana_0_reg);
	EARC_ANA_2.regValue = AIO_ReadRegister(EARCMISC_earc_ana_2_reg);
//    EARC_ANA_2.earc_test_mode = 0; //Disable ARC Internal Pattern//mask it, due to acpu already control it
	if (Mode == AIO_AUDIO_ARC)
	{
		//EARC_ANA_0.pow_earc_driver = 0;
		EARC_ANA_0.earc_data_sel = 0; //arc path
	}
	else
	{
		//EARC_ANA_0.pow_earc_driver = 1;
		EARC_ANA_0.earc_data_sel = 1; //earc path
	}

	AIO_WriteRegister(EARCMISC_earc_ana_0_reg, EARC_ANA_0.regValue);
	AIO_WriteRegister(EARCMISC_earc_ana_2_reg, EARC_ANA_2.regValue);
	return 0;
}

void Audio_HW_Earc_ClrReadIrq(void)
{
	earc_earc_cmdc_irq_RBUS CMDC_IRQ_STATUS;

	CMDC_IRQ_STATUS.regValue = 0;
	CMDC_IRQ_STATUS.regValue |= AIO_EARC_ERR_READ_TRANS_IRQ;
	AIO_WriteRegister(EARC_eARC_CMDC_IRQ_reg, CMDC_IRQ_STATUS.regValue);
}

void Audio_HW_Earc_ClrWriteIrq(void)
{
	earc_earc_cmdc_irq_RBUS CMDC_IRQ_STATUS;

	CMDC_IRQ_STATUS.regValue = 0;
	CMDC_IRQ_STATUS.regValue |= AIO_EARC_ERR_WRITE_TRANS_IRQ;
	AIO_WriteRegister(EARC_eARC_CMDC_IRQ_reg, CMDC_IRQ_STATUS.regValue);
}

char Audio_HW_Earc_PortUpdate(void)
{
#if IS_ENABLED(CONFIG_RTK_HDMI_RX)
    return drvif_Hdmi_get_pcb_earc_port_index();
#else
    return 1;
#endif
}

static void AudioEarcARCMode(void)
{
	earcmisc_earc_ana_0_RBUS EARC_ANA_0;

	EARC_ANA_0.regValue = AIO_ReadRegister(EARCMISC_earc_ana_0_reg);
	EARC_ANA_0.earc_data_sel = 0; //arc path
	EARC_ANA_0.pow_earc_driver = 0;
	AIO_WriteRegister(EARCMISC_earc_ana_0_reg, EARC_ANA_0.regValue);
	g_Earc_State.ConnectState = AIO_EARC_STATE_IDLE2;
}

void Audio_HW_Earc_APHY_Update(AIO_FS_ID freq)
{
	earcmisc_earc_ana_1_RBUS EARC_ANA_1;
	earcmisc_earc_ana_2_RBUS EARC_ANA_2;
	if(freq==AIO_FS_ID_192K || freq==AIO_FS_ID_176K )//earc_pll_1x=98M/90M, adjust APHY eyediagram parameter
	{
		EARC_ANA_2.regValue = AIO_ReadRegister(EARCMISC_earc_ana_2_reg);
		EARC_ANA_2.df_duty_sel = AIO_EARC_DUTY_FIX60FINETUNE40; 
		AIO_WriteRegister(EARCMISC_earc_ana_2_reg, EARC_ANA_2.regValue);
		
		EARC_ANA_1.regValue = AIO_ReadRegister(EARCMISC_earc_ana_1_reg);
		EARC_ANA_1.reserved_aphy &= 0x1f; //Clear 	
		EARC_ANA_1.reserved_aphy |= AIO_EARC_DUTY_DOWN_1P; 
		AIO_WriteRegister(EARCMISC_earc_ana_1_reg, EARC_ANA_1.regValue);	

		alog_debug("freq=%x, 98/90M update aphy=%x\n",freq,EARC_ANA_1.reserved_aphy);	
		
	}else
	{
		EARC_ANA_2.regValue = AIO_ReadRegister(EARCMISC_earc_ana_2_reg);
		EARC_ANA_2.df_duty_sel = AIO_EARC_DUTY_FIX4060;
		AIO_WriteRegister(EARCMISC_earc_ana_2_reg, EARC_ANA_2.regValue);
		
		EARC_ANA_1.regValue = AIO_ReadRegister(EARCMISC_earc_ana_1_reg);
		EARC_ANA_1.reserved_aphy &= 0x1f; //Clear 
		AIO_WriteRegister(EARCMISC_earc_ana_1_reg, EARC_ANA_1.regValue);	

		alog_debug("freq=%x, back to default=%x\n",freq,EARC_ANA_1.reserved_aphy);	
	}	
}

void Audio_HW_Earc_OTPCheck(void)
{
	earcmisc_earc_ana_1_RBUS EARC_ANA_1;
	earcmisc_earc_ana_2_RBUS EARC_ANA_2;
	earcmisc_earc_db_ctrl_RBUS EARC_DB_CTRL;
	EARC_OTP_T EARC_OTP;
	EARC_OTP_Z0_T EARC_OTP_Z0;	
	EARC_OTP_Z0.regValue = AIO_ReadRegister(AIO_EARC_OTP_Z0_REG);
	alog_err("EARC Z0 OTP Check=%x\n",EARC_OTP_Z0.regValue);
	if(EARC_OTP_Z0.R60_Done==1)	
	{
		alog_err("EARC OTP Found:R60 = %d\n",EARC_OTP_Z0.R60_Sel);	
		EARC_ANA_1.regValue = AIO_ReadRegister(EARCMISC_earc_ana_1_reg);
		EARC_ANA_1.r60_sel= EARC_OTP_Z0.R60_Sel;
		AIO_WriteRegister(EARCMISC_earc_ana_1_reg, EARC_ANA_1.regValue);
	}
	if(EARC_OTP_Z0.R300_Done==1)	
	{
		alog_err("EARC OTP Found:R300 = %d\n",EARC_OTP_Z0.R300_Sel);	
		EARC_ANA_1.regValue = AIO_ReadRegister(EARCMISC_earc_ana_1_reg);
		EARC_ANA_1.r300_sel= EARC_OTP_Z0.R300_Sel;
		AIO_WriteRegister(EARCMISC_earc_ana_1_reg, EARC_ANA_1.regValue);
	}	
	EARC_OTP.regValue = AIO_ReadRegister(AIO_EARC_OTP_REG);	
	alog_err("EARC OTP Check=%x\n",EARC_OTP.regValue);	
	if(EARC_OTP.Cm_Ib_Done==1)	
	{
		alog_err("EARC OTP Found:Cm_Ib = %d\n",EARC_OTP.Cm_Ib_Sel);	
		EARC_ANA_1.regValue = AIO_ReadRegister(EARCMISC_earc_ana_1_reg);
		EARC_ANA_1.cm_ib_sel = EARC_OTP.Cm_Ib_Sel;
		AIO_WriteRegister(EARCMISC_earc_ana_1_reg, EARC_ANA_1.regValue);
	}
	if(EARC_OTP.Df_Ib_Done==1)	
	{
		alog_err("EARC OTP Found:Df_Ib = %d\n",EARC_OTP.Df_Ib_Sel);	
		EARC_ANA_2.regValue = AIO_ReadRegister(EARCMISC_earc_ana_2_reg);
		EARC_ANA_2.df_ib_sel = EARC_OTP.Df_Ib_Sel;
		AIO_WriteRegister(EARCMISC_earc_ana_2_reg, EARC_ANA_2.regValue);
	}

	if(EARC_OTP.Hy_Wd_Done==1)	
	{
		alog_err("EARC OTP Found:Wd = %d\n",EARC_OTP.Hy_Wd_Sel);		
		EARC_ANA_2.regValue = AIO_ReadRegister(EARCMISC_earc_ana_2_reg);
		EARC_ANA_2.pow_earc_ldo=EARC_OTP.Hy_Wd_Sel;//eco
		AIO_WriteRegister(EARCMISC_earc_ana_2_reg, EARC_DB_CTRL.regValue);
	}
}

void Earc_Clock_Check(void)
{
//    GET_HW_SEM();
    sys_reg_sys_clken0_RBUS clken0;
    sys_reg_sys_srst0_RBUS srst;	
    _rtd_hwsem_lock(SEMA_HW_SEM_5, SEMA_HW_SEM_5_SCPU_0);	
    clken0.regValue = AIO_ReadRegister(SYS_REG_SYS_CLKEN0_reg);
    if(clken0.clken_earc==0)
    {
         clken0.regValue=0;
         clken0.clken_earc=1;
         clken0.write_data=1;
	  AIO_WriteRegister(SYS_REG_SYS_CLKEN0_reg,clken0.regValue);
	  
	  srst.regValue=0;
	  srst.rstn_earc=1;
	  srst.write_data=1;
	  AIO_WriteRegister(SYS_REG_SYS_SRST0_reg,srst.regValue);
    }
//    SET_HW_SEM();
    _rtd_hwsem_unlock(SEMA_HW_SEM_5, SEMA_HW_SEM_5_SCPU_0);
}

void Audio_HW_Earc_SetAPHY(void)
{
	earcmisc_earc_ana_0_RBUS EARC_ANA_0;
	earcmisc_earc_ana_1_RBUS EARC_ANA_1;
	earcmisc_earc_ana_2_RBUS EARC_ANA_2;
	earcmisc_earc_dphy1_RBUS DPHY1;
	earc_earc_cmdc_timing0_RBUS CMDC_TIMING0;
	earc_earc_cmdc_timing1_RBUS CMDC_TIMING1;
	earc_earc_cmdc_nack_uecc_RBUS CMDC_NACK_UECC;
	int32_t RegValue = 0;

	Earc_Clock_Check();
	alog_err("Audio_HW_Earc_SetAPHY s\n");
	EARC_ANA_0.regValue = AIO_ReadRegister(EARCMISC_earc_ana_0_reg);
	EARC_ANA_0.df_trf_sel = 0x3;
	EARC_ANA_0.earc_data_sel=1;
	AIO_WriteRegister(EARCMISC_earc_ana_0_reg, EARC_ANA_0.regValue);

	EARC_ANA_1.regValue = AIO_ReadRegister(EARCMISC_earc_ana_1_reg);
	EARC_ANA_1.r300_sel = 0x1;
	EARC_ANA_1.r60_sel = 0xa;
	EARC_ANA_1.cm_ib_sel = 0x4;
	EARC_ANA_1.r_en = 0xf;
	EARC_ANA_1.reserved_aphy|=0x1;//ARC_RES_SEL[3]
	AIO_WriteRegister(EARCMISC_earc_ana_1_reg, EARC_ANA_1.regValue);

	EARC_ANA_2.regValue = AIO_ReadRegister(EARCMISC_earc_ana_2_reg);
	EARC_ANA_2.df_ib_sel = 0x4;
	EARC_ANA_2.cm_trf_sel = 0x3;
	EARC_ANA_2.df_duty_sel = 0x7;
	EARC_ANA_2.arc_res_sel = 0x4;//ARC_RES_SEL[2:0]
	EARC_ANA_2.vol_level_cm = 0x0;
	EARC_ANA_2.ldo18v_ref_sel=0; //eco: CMRx ref, 0: from RC, 1: from OP
	EARC_ANA_2.pow_earc_ldo=1;//eco: hysteresis window, 0:78mV, 1: 55mV
	AIO_WriteRegister(EARCMISC_earc_ana_2_reg, EARC_ANA_2.regValue);
	
	DPHY1.regValue = AIO_ReadRegister(EARCMISC_earc_dphy1_reg);
	DPHY1.cm_rx_response = 0x10; //the default value is too small. orig:0x3F
	AIO_WriteRegister(EARCMISC_earc_dphy1_reg, DPHY1.regValue);
	
	RegValue = AIO_ReadRegister(EARCMISC_earc_dphy2_reg); //eco
	RegValue = ((RegValue & (~(VERIFY_BIT9 | VERIFY_BIT8))) | VERIFY_BIT8); //0xb80b8030[9:8] : cm_rx_response[7:6], bit8 =1
	RegValue = ((RegValue & (~(VERIFY_BIT19 | VERIFY_BIT18))) | VERIFY_BIT18); //0xb80b8030[19:18] : dphy rx debounce=3:5-bit, =2:4-bit,=1:3-bit,=0:no debounce
	RegValue = RegValue | VERIFY_BIT23; //b80b8030[23]=1, turn on pramble-1 patch
	AIO_WriteRegister(EARCMISC_earc_dphy2_reg, RegValue);

	CMDC_TIMING0.regValue = AIO_ReadRegister(EARC_eARC_CMDC_TIMING0_reg);
	CMDC_TIMING0.tx_conn_comma_off_min = EARC_TIMING_COMMA_OFF_MIN;
	CMDC_TIMING0.tx_conn_comma_off_max = EARC_TIMING_COMMA_OFF_MAX;
	CMDC_TIMING0.tx_timeout = EARC_TIMING_TX_TIMEOUT;
	CMDC_TIMING0.tx_connearc_valid = EARC_TIMING_TX_CONNEARC_VALID;
	CMDC_TIMING0.tx_disconn = 0x10; //6dc0[30]=1, turn on pramble-1 patch
	CMDC_TIMING0.hdmi_hpd_hb=1;
	AIO_WriteRegister(EARC_eARC_CMDC_TIMING0_reg, CMDC_TIMING0.regValue);

	CMDC_TIMING1.regValue = AIO_ReadRegister(EARC_eARC_CMDC_TIMING1_reg);
	CMDC_TIMING1.tx_conn_comma_on_min = EARC_TIMING_COMMA_ON_MIN;
	CMDC_TIMING1.tx_conn_comma_on_max = EARC_TIMING_COMMA_ON_MAX;
	CMDC_TIMING1.lost_heartbeat = EARC_TIMING_LOST_HEARTBEAT;
	CMDC_TIMING1.heartbeat = EARC_TIMING_HEARTBEAT_DISC;//EARC_TIMING_HEARTBEAT;
	CMDC_TIMING1.stat_chng_interval=AIO_EARC_CHNG_NEXT_HB;
	
	AIO_WriteRegister(EARC_eARC_CMDC_TIMING1_reg, CMDC_TIMING1.regValue);

	CMDC_NACK_UECC.regValue = AIO_ReadRegister(EARC_eARC_CMDC_NACK_UECC_reg);
	CMDC_NACK_UECC.read_nack_num = EARC_READ_NACK_NUM;
	CMDC_NACK_UECC.read_uecc_num = EARC_READ_UECC_NUM;
	AIO_WriteRegister(EARC_eARC_CMDC_NACK_UECC_reg, CMDC_NACK_UECC.regValue);
//For OTP Usage
	Audio_HW_Earc_OTPCheck();

	PreHpdState = 0;
	Pre5vState = 0;
//
	g_EarcPort=(HDMI_PCB_EARC_T)Audio_HW_Earc_PortUpdate();
//Update port
	if(g_EarcPort==HDMI_PCB_EARC_PORT0 || g_EarcPort==HDMI_PCB_EARC_PORT1 ||
		g_EarcPort==HDMI_PCB_EARC_PORT2 || g_EarcPort==HDMI_PCB_EARC_PORT3)
	{
		Audio_Earc_Timer_Init();
		alog_err("PCB Info found, earc support port=%x, Audio_HW_Earc_Timer On e\n",g_EarcPort);
		AudioHw_Earc_Reset();
		alog_err("Audio_HW_Earc_SetAPHY e\n");

	}else if(g_EarcPort==HDMI_PCB_EARC_UNKNOW)//unknown earc information, set to default port0
	{

		g_EarcPort=HDMI_PCB_EARC_PORT0;//default
		Audio_Earc_Timer_Init();
		alog_err("PCB Info not found, earc support default port=%x, Audio_HW_Earc_Timer On e\n",g_EarcPort);
		AudioHw_Earc_Reset();
		alog_err("Audio_HW_Earc_SetAPHY e\n");			
	}
	else
	{
		alog_err("PCB Info found, not support earc, Audio_HW_Earc_Timer Off e\n");	
		AudioHw_Earc_Reset();
		AudioEarcARCMode();
		alog_err("Audio_HW_Earc_ARC_Mode e\n");		
	}


	
}

void Audio_HW_Earc_GetHandshakingStatus(AIO_EARC_STATE_MACHINE_T *Status)
{
	earc_earc_cmdc_req_RBUS Earc_State;
	earc_earc_cmdc_irq_RBUS earc_cmdc_irq;
	
	earc_cmdc_irq.regValue = AIO_ReadRegister(EARC_eARC_CMDC_IRQ_reg);


	Earc_State.regValue = AIO_ReadRegister(EARC_eARC_CMDC_REQ_reg);
	if (g_Earc_State.ConnectState == AIO_EARC_STATE_EARC 
		&& (AIO_EARC_STATE_MACHINE_T)Earc_State.disc_state == AIO_EARC_STATE_DISC1)
	{
		Audio_HW_Earc_Reset_irq();
	}
	
	if (g_Earc_State.ConnectState != (AIO_EARC_STATE_MACHINE_T)Earc_State.disc_state)
	{
		alog_err("g_Earc_State.ConnectState = 0x%x, Earc_State.disc_state = 0x%x,irq=%x\n", g_Earc_State.ConnectState,
			Earc_State.disc_state,earc_cmdc_irq.regValue);
		g_Earc_State.ConnectState = Earc_State.disc_state;
	}

	if (g_Earc_State.ConnectState == AIO_EARC_STATE_IDLE2)
	{
		AudioEarcInitPow(0);
	}
	
	if (g_Earc_State.ConnectState == AIO_EARC_STATE_DISC2)
	{
		if(g_Disc2_count>100)
		{
			AudioHw_Earc_Reset();
			//g_Earc_State.ConnectState =AIO_EARC_STATE_DISC2;
			alog_err("DISC2 timeout\n");
			g_Disc2_count=0;
		}
	}else
		g_Disc2_count=0;


#ifdef debug_io //gpio137
	if (g_Earc_State.ConnectState == AIO_EARC_STATE_EARC)
		rtd_outl(MIS_GPIO_GP4DATO_reg,0x00004000);
#endif

	
	*Status = g_Earc_State.ConnectState;
}

void Audio_HW_Earc_ResetHdmiHpd(void)
{
	if (g_Earc_State.ConnectState == AIO_EARC_STATE_EARC)
	{
		earc_earc_cmdc_timing0_RBUS CMDC_TIMING0;

		down(&audio_sem_aio);
		audio_hw_usleep(200*1000);
		//set hdmi_hpd to 0
		CMDC_TIMING0.regValue = AIO_ReadRegister(EARC_eARC_CMDC_TIMING0_reg);
		CMDC_TIMING0.hdmi_hpd_hb=0; //hdmi_hpd bit = 0;
		AIO_WriteRegister(EARC_eARC_CMDC_TIMING0_reg, CMDC_TIMING0.regValue);

		audio_hw_usleep(500*1000);

		//set hdmi_hpd to 1
		CMDC_TIMING0.regValue = AIO_ReadRegister(EARC_eARC_CMDC_TIMING0_reg);
		CMDC_TIMING0.hdmi_hpd_hb=1;//hdmi_hpd bit = 1;
		AIO_WriteRegister(EARC_eARC_CMDC_TIMING0_reg, CMDC_TIMING0.regValue);
		up(&audio_sem_aio);
	}
}

void Audio_HW_Earc_Reset_irq(void)
{
	earc_earc_cmdc_irq_RBUS earc_cmdc_irq;
	earc_earc_cmdc_timing0_RBUS CMDC_TIMING0;
	
	earc_cmdc_irq.regValue = AIO_ReadRegister(EARC_eARC_CMDC_IRQ_reg);
	//earc_cmdc_irq.regValue = 0x000003ff;
	earc_cmdc_irq.regValue = 0x00000fff;
	AIO_WriteRegister(EARC_eARC_CMDC_IRQ_reg, earc_cmdc_irq.regValue);
audio_hw_usleep(3);
	CMDC_TIMING0.regValue = AIO_ReadRegister(EARC_eARC_CMDC_TIMING0_reg);
	//CMDC_TIMING0.tx_disconn = 0x20; //hdmi_hpd bit = 1
	CMDC_TIMING0.hdmi_hpd_hb=1;
	AIO_WriteRegister(EARC_eARC_CMDC_TIMING0_reg, CMDC_TIMING0.regValue);
}

void Audio_HW_Earc_En(unsigned int en)
{
	g_EarcEn = en;
}

void Audio_HW_earc_suspend(void)
{
	if (g_EarcTimerInitDone == 1)
	{
		alog_err("delete earc timer\n");
		del_timer_sync(&Audio_EarcTimerList);
		g_EarcTimerInitDone = 0;
	}

}
EXPORT_SYMBOL_GPL(Audio_HW_Earc_Get_Rx_Status);
EXPORT_SYMBOL_GPL(Audio_HW_Earc_Read_Transaction);
EXPORT_SYMBOL_GPL(Audio_HW_Earc_Write_Latency);
EXPORT_SYMBOL_GPL(Audio_HW_Earc_SetAPHY);
EXPORT_SYMBOL_GPL(Audio_HW_Earc_Init_Detection);
EXPORT_SYMBOL_GPL(AudioHw_Earc_Isr);
EXPORT_SYMBOL_GPL(Audio_HW_Earc_Disable_Detection);
EXPORT_SYMBOL_GPL(Audio_HW_Earc_ResetHdmiHpd);
EXPORT_SYMBOL_GPL(Audio_HW_Earc_En);
EXPORT_SYMBOL_GPL(AIO_ReadRegister);
EXPORT_SYMBOL_GPL(AIO_WriteRegister);
EXPORT_SYMBOL_GPL(g_Earc_State);
EXPORT_SYMBOL_GPL(Audio_HW_earc_suspend);
#endif



static void WriteMaskRegister(UINT32 address, UINT32 mask, UINT32 data)
{
	UINT32 target_value;
	target_value=rtd_inl(address);
	target_value=target_value&(~mask);
	target_value=(target_value|data);
	AIO_WriteRegister(address, target_value);
}
int32_t Audio_AQLoopBackVerify(AIO_HWP_SET_DMA_T d)
{
//AQ enable// DVC OFF
AIO_WriteRegister(AUDIO_HW_Process_Control_reg,0x0000002e);
AIO_WriteRegister(AUDIO_LR_Digital_Volume_Control_reg,0x80008000);//

//SPDIF Out
AIO_WriteRegister(AUDIO_spdif_out_reg,0x01800000);//SPDIF OUT from PB path (after AVC2)
WriteMaskRegister(AUDIO_spdif_out_cs_2_reg,0xfffffff0,0xb);//word length=24bit
WriteMaskRegister(AUDIO_spdif_out_cs_4_reg,0xfffffff0,0xb);//word length=24bit

//SPDIF In
WriteMaskRegister(AUDIO_spdif_in_reg, 0x3fffffff, 0x80000000);//internal loopback
AIO_WriteRegister(AUDIO_down_sample_fifo_source_sel_reg,0x00000002);//DSFIFO from SPDIF

//PB RDMA
AIO_WriteRegister(AUDIO_ADMA_CSR_pbr_reg,0x00000280); //2//ch, disable PB_DMA_r
AIO_WriteRegister(AUDIO_ADMA_RSADR_pbr_reg,d.RDMA_PB_ADDR_L | 8); //ch0
AIO_WriteRegister(AUDIO_ADMA_RSADR_pbr_reg,d.RDMA_PB_ADDR_R | 9); //ch1
AIO_WriteRegister(AUDIO_ADMA_WRCONF_pbr_reg,d.RDMA_Lens<<16);	// set rd_buffer_length = 0x40 (the same as audio SW use), 
WriteMaskRegister(AUDIO_ARDMA_ctrl_reg,0xFFFFFFFE, 0x00000001);      //rDMA_enable
WriteMaskRegister(AUDIO_ADMA_CSR_pbr_reg,0xffffffdf, 0x00000020);      //PB Dma_soft_rd_rst =1 reset
AIO_WriteRegister(AUDIO_ADMA_CSR_pbr_reg,0x00000290); //[11:9]2ch, [5]PB Dma_soft_rd_rst =0 normal, [4]Enable_pb_rDMA

//PB WDMA
AIO_WriteRegister(AUDIO_ADMA_CSR_pbw_reg,0x00000280); //src: from FIFO ; 2ch, disable PB_DMA_w
AIO_WriteRegister(AUDIO_ADMA_WSADR_pbw_reg,d.WDMA_PB_ADDR_L | 8); //ch0
AIO_WriteRegister(AUDIO_ADMA_WSADR_pbw_reg,d.WDMA_PB_ADDR_R | 9); //ch1
AIO_WriteRegister(AUDIO_ADMA_WRCONF_pbw_reg,d.WDMA_Lens<<16);	// set wd_buffer_length = 0x40, 
WriteMaskRegister(AUDIO_AWDMA_ctrl_reg,0xFFFFFFFE, 0x00000001);      //wDMA_enable
WriteMaskRegister(AUDIO_ADMA_CSR_pbw_reg,0xffffffdf, 0x00000020);      //PB Dma_soft_wt_rst =1 reset
AIO_WriteRegister(AUDIO_ADMA_CSR_pbw_reg,0x00000290); //[11:9]2ch, [5]PB Dma_soft_wt_rst =0 normal, [4]Enable_pb_wDMA

return 0;
}


