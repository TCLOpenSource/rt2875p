#ifndef _LINUX_RTKAUDIO_H
#define _LINUX_RTKAUDIO_H

#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/string.h>

#include <rtk_kdriver/rtkaudio_debug.h>
#include "ioctrl/audio/audio_cmd_id.h"
#include <rtd_log/rtd_module_log.h>

#define RTKAUDIO_MAJOR             0
#define RTKAUDIO_MINOR 0

#define RTKAUDIO_SUSPEND           100
#define RTKAUDIO_RESUME            101
#define RTKAUDIO_RESET_PREPARE     102
#define RTKAUDIO_RESET_DONE        103

#define REMOTE_MALLOC_DISABLE	0
#define REMOTE_MALLOC_ENABLE	1

#define RPC_DVR_MALLOC_UNCACHED_SIZE (4096)//can't edit this size

#ifdef CONFIG_HIBERNATION
extern int in_suspend;
#endif

#define RTKAUDIO_PROC_DIR   "rtkaudio"
#define RTKAUDIO_PROC_ENTRY "debuginfo"

int register_rtkaudio_notifier(struct notifier_block *nb);
int unregister_rtkaudio_notifier(struct notifier_block *nb);
void halt_acpu(void);
long rtkaudio_send_hdmi_fs(long hdmi_fs);
void RHAL_AUDIO_HDMI_GetAudioReturnChannel(bool *onoff);
void RHAL_AUDIO_HDMI_GetEnhancedAudioReturnChannel(bool *onoff);

/* Record memory use*/
struct buffer_info {
	struct list_head buffer_list;

	/* virtual address */
	unsigned long vir_addr;

	/* physical address */
	unsigned long phy_addr;

	/* request size by audio DSP */
	unsigned long request_size;

	/* kernel malloc size */
	unsigned long malloc_size;
	pid_t         task_pid;
};

/* Store Open list */
struct rtkaudio_open_list {
	struct list_head open_list;
	pid_t pid;
	struct gst_info info;
};

#define refc_info_size 8
struct refclock_info {
	unsigned int phy_addr;
	int pid;
	int port;
};

/* Store Gst RefClock list */
struct gst_refc_list {
	int size;
	int index;
	struct refclock_info info[refc_info_size];
};

/* command queue between voice_ap and rtkaudio */
#define CMDQ_NUM (10)
typedef enum {
    CMD_INIT_DOWNRING = 0,
    CMD_SUSPEND,
    CMD_CLOSE,
    CMD_INIT_UPRING,
    CMD_SPK_VOL,    //notify speaker volume
    CMD_TRI_STS,    //notify trigger status
} State;

typedef struct {
    int cmd_type;
    int cmd[7];
} cmd_packet;

typedef struct {
    int r_index;
    int w_index;
    int num;
    int error;
} queue_header;

typedef struct {
    queue_header header;
    cmd_packet *pkt_array_kernel;   //virtual
    cmd_packet *pkt_array_user;     //physical
} cmd_queue;

int cmd_queue_create(int num);
/*end*/

typedef struct {
	unsigned int  msg_id;
	unsigned int value[15];
} AUDIO_CONFIG_COMMAND_RTKAUDIO;

typedef enum {
	LR_DIGITAL_VOLUME_CONTROL_STATUS = 0,
	CSW_DIGITAL_VOLUME_CONTROL_STATUS = 1,
	LSRS_DIGITAL_VOLUME_CONTROL_STATUS = 2,
	LRRR_DIGITAL_VOLUME_CONTROL_STATUS = 3,
	GST_CLOCK = 4,
	AMIXER_STATUS = 5,
	DEBUG_REGISTER = 6
} AUDIO_REGISTER_TYPE;

typedef struct
{
	AUDIO_REGISTER_TYPE reg_type;
	int reg_value;	/* register value */
	int data;   	/* value for data write */
} AUDIO_REGISTER_ACCESS_T;

typedef enum
{
	MODE_AUTO_DETECT,
	MODE_DOLBY_SURROUND,
	MODE_STEREO,
	MODE_ARIB
} RTKAUDIO_DMX_MODE;

/* defined in kernel/printk.c */
extern int console_printk[4];

/* defined in rtkaudio_driver.c*/
extern int alsa_debug;

#define rtkaudio_must_print(fmt, ...)	\
{\
	int prev_printk_level = console_printk[0];\
	if (prev_printk_level == 0)\
		console_printk[0] = 1;\
	rtd_pr_adsp_emerg(fmt, ##__VA_ARGS__);\
	console_printk[0] = prev_printk_level;\
}

void info_up_down_ring(unsigned int down_header_phy,unsigned int up_header_phy);
void suspend_voice(void);
void rtkaudio_dap_onoff(int enable);
long rtkaudio_send_audio_config(AUDIO_CONFIG_COMMAND_RTKAUDIO * cmd);
void rtkaudio_runtime_put(void);
void rtkaudio_runtime_get(void);

/***************dmabuf define*****************/
/*
	rtkaudio_dmabuf_fd_allocate
	get a dmabuf fd, the buffer is from CMA.
	size: buffer size
 */
int rtkaudio_dmabuf_fd_allocate(int size);

/*
	rtkaudio_dmabuf_get_vaddr
	get the virtual address from dmabuf fd handle.
	fd: dmabuf handle.

	physical address could use dvr_to_phys to get.
 */
void* rtkaudio_dmabuf_get_vaddr(int fd);

/*
	rtkaudio_ioctl_dmabuf_mmap
	mmap address from dmabuf fd handle.
	fd: dmabuf handle.
	area: vm area structure
 */
int rtkaudio_ioctl_dmabuf_mmap(int fd, struct vm_area_struct *area);

#endif

