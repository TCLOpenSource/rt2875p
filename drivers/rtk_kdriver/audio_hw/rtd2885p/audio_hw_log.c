#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/syscalls.h> 
#include <linux/file.h> 
#include <linux/fs.h>
#include <linux/fcntl.h> 
#include <linux/uaccess.h> 
#include <linux/time.h>
#include <linux/version.h>

#include "audio_hw_log.h"

#define ALOG_CHK_FILE_DEF		("/usr/local/etc/audio_lib.txt")
#define ALOG_CHK_FILE_WEBOS		("/home/audio_lib.txt")

static bool is_alog_chk_init = false;
static bool is_alog_chk_enable = false;

static int is_file_exist(char *filename)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_AUDIO_HW)
	struct file* fd = NULL;
#else
	int fd = -1;
#endif
	
	mm_segment_t old_fs;

	if (!filename)
		return 0;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

#if IS_ENABLED(CONFIG_RTK_KDRV_AUDIO_HW)
	fd = filp_open(filename, O_RDONLY, 0);

	if(fd)	filp_close(fd, NULL);

	set_fs(old_fs);
	return (fd ? 1 : 0);
#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	fd = ksys_open(filename, O_RDONLY, 0);
#else
	fd = sys_open(filename, O_RDONLY, 0);	
#endif

	if (fd >= 0)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))		
		ksys_close(fd);
#else
		sys_close(fd);	
#endif	

	set_fs(old_fs);

	return (fd >= 0 ? 1 : 0);
#endif
}

bool alog_chk_enable(void)
{
	if (!is_alog_chk_init) {
		is_alog_chk_enable = false;
		if (is_file_exist(ALOG_CHK_FILE_DEF) ||
		    is_file_exist(ALOG_CHK_FILE_WEBOS)) {
			is_alog_chk_enable = true;
		}
		is_alog_chk_init = true;
	}
	return is_alog_chk_enable;
}

struct timeval alog_chk_gettimeofday(void)
{
	struct timeval t;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	struct timespec64 t64;

	ktime_get_real_ts64(&t64);
	t.tv_sec = t64.tv_sec;
	t.tv_usec = t64.tv_nsec / 1000;
#else
	do_gettimeofday(&t);
#endif

	return t;
}
