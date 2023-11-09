#ifndef _LINUX_RTKAUDIO_DEBUG_H
#define _LINUX_RTKAUDIO_DEBUG_H


#define RTD_PRINT_AUDIO_EANBLE

#ifdef RTD_PRINT_AUDIO_EANBLE
#include <rtd_log/rtd_module_log.h>
#endif

enum DUMP_ID {
	DUMP_STOP = 0,
	DUMP_LOG = 1,
	DUMP_PP_FOCUS,
	DUMP_AO_PCM,
	DUMP_AO_RAW
};

int rtkaudio_dump_enable(unsigned int db_command);
void rtkaudio_dump_disable(unsigned int db_command);
void rtkaudio_send_string(const char* pattern, int length);
#endif
