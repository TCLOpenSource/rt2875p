#include <core/event_log_core.h>

/*
######################################################################################
# INCLUDE EVENT PARSE LIBRARY
######################################################################################
*/
#include <fw/kernel/module_string_list.h>
#include <fw/quickshow/module_string_list.h>
#include <fw/android/module_string_list.h>
#include <sample/module_string_list.h>
/*
######################################################################################
# MEMORY DEFINE
######################################################################################
*/
struct fw_filter_format fw_filter_data[FW_MAX];
unsigned int log_memory[INT_LOG_SIZE];
unsigned int time_memory[INT_LOG_SIZE/4];
unsigned int output_buf_log[INT_LOG_SIZE];
unsigned int eventlog_time_unit = 1;
unsigned int memory_count = 0;
/*
######################################################################################
# MODULE STRING DEFINE
######################################################################################
*/
struct event_fw_s event_fw[]={
    {EVENT_FW_ANDROID, "ANDROID", event_android_module},
	{EVENT_FW_SAMPLE, "SAMPLE", event_sample_module},
    {EVENT_FW_QUICKSHOW, "QUICKSHOW", quickshow_module},
    {EVENT_FW_KERNEL, "KERNEL", kernel_module},
	{EVENT_LIMIT_MAX,NULL,NULL},
};
/*
######################################################################################
# FUNCTION DEFINE
######################################################################################
*/
/*
	little/big endian transform
*/
unsigned int eventlog_endian_32_swap(unsigned int x)
{

	return (x>>24) |
		   ((x<<8) & 0x00FF0000) |
		   ((x>>8) & 0x0000FF00) |
		   (x<<24);
}

int GET_FW_ID(unsigned int DW1)
{
	struct event_format event_format;
	int i = 0;

	event_format.fw_type = (DW1 & 0x00f00000)>>20;

	for( i = 0; EVENT_LIMIT_MAX != event_fw[i].fw_id; i++)
	{
		if( event_fw[i].fw_id == event_format.fw_type)
			return i;
	}
	event_err_print("parse fw error, fw_type = %x\n", event_format.fw_type);
	return -1;
}

int GET_MODULE_ID(unsigned int DW1, int fw_id)
{
	struct event_format event_format;
	int i = 0;

	event_format.module = (DW1 & 0x0000ff00)>>8;


	for( i = 0;EVENT_LIMIT_MAX != event_fw[fw_id].module[i].module_id; i++)
	{
		if( event_fw[fw_id].module[i].module_id == event_format.module)
			return i;
	}
	event_err_print("parse module error, module = %x\n", event_format.module);
	return -1;
}

int GET_EVENTTYPE_ID(unsigned int DW1, int fw_id, int module_id)
{
	struct event_format event_format;
	int i = 0;

	event_format.event_type = (DW1 & 0x000f0000)>>16;

	for( i = 0; EVENT_LIMIT_MAX != event_fw[fw_id].module[module_id].eventtype[i].eventtype_id; i++)
	{
		if( event_fw[fw_id].module[module_id].eventtype[i].eventtype_id == event_format.event_type)
			return i;
	}
	event_err_print("parse type error, event_type = %x\n", event_format.event_type);
	return -1;
}

int GET_EVENT_ID(unsigned int DW1,unsigned int fw_id,unsigned int module_id,unsigned int eventtype_id)
{
	struct event_format event_format;
	int i = 0;

	event_format.event = DW1 & 0x000000ff;

	for( i = 0; EVENT_LIMIT_MAX != event_fw[fw_id].module[module_id].eventtype[eventtype_id].event[i].event_id; i++)
	{
		if( event_fw[fw_id].module[module_id].eventtype[eventtype_id].event[i].event_id == event_format.event)
			return i;
	}
	event_err_print("parse event error, event = %x\n", event_format.event);

	return -1;
}


unsigned int eventlog_parser_printf(unsigned int DW1,unsigned int DW2, unsigned int DW3 ,unsigned int DW4, unsigned int time_unit)
{

	int fw_id = 0;
	int module_id = 0;
	int eventtype_id = 0;
	int event_id = 0;

	struct event_format event_format;

	event_format.syncbyte = (DW1 & 0xff000000)>>24;
	event_format.event_value = DW2;
	event_format.module_reserved = DW3;
	/* time stamp */
	event_format.timestamp = DW4;

	//event_warn_print(" DW1 = %08x\n", DW1);

	if(event_format.syncbyte != EVENT_SYNCBYTE)
	{
		event_err_print("syncbyte error");
		event_err_print("DW1 = %08x\n", DW1);
		return EVENT_ERROR;
	}

	fw_id = GET_FW_ID(DW1);

	if(-1 == fw_id)
	{
		event_err_print("DW1 = %08x\n", DW1);
		return EVENT_ERROR;
	}

	module_id = GET_MODULE_ID(DW1, fw_id);

	if(-1 == module_id)
	{
		event_err_print("DW1 = %08x\n", DW1);
		return EVENT_ERROR;
	}

	eventtype_id = GET_EVENTTYPE_ID(DW1, fw_id, module_id);

	if(-1 == eventtype_id)
	{
		event_err_print("DW1 = %08x\n", DW1);
		return EVENT_ERROR;
	}

	event_id = GET_EVENT_ID(DW1, fw_id, module_id, eventtype_id);

	if(-1 == event_id)
	{
		event_err_print("DW1 = %08x\n", DW1);
		return EVENT_ERROR;
	}

	if (time_unit)
		event_alert_print("[%ld.%d%d%d%d%d%d][%s][%s][%s][%s],%x,%x \n",
					(event_format.timestamp/1000000),
					((event_format.timestamp%1000000)/100000),((event_format.timestamp%100000)/10000),
					((event_format.timestamp%10000)/1000),((event_format.timestamp%1000)/100),
					((event_format.timestamp%100)/10),((event_format.timestamp%10)),
					event_fw[fw_id].name, event_fw[fw_id].module[module_id].name,
             	    event_fw[fw_id].module[module_id].eventtype[eventtype_id].name,
             	    event_fw[fw_id].module[module_id].eventtype[eventtype_id].event[event_id].name,
					event_format.event_value,
					event_format.module_reserved);
	else
		event_alert_print("[%ld.%d%d%d000][%s][%s][%s][%s],%x,%x \n",
					(event_format.timestamp/1000),((event_format.timestamp%1000)/100),
					((event_format.timestamp%100)/10),((event_format.timestamp%10)),
					event_fw[fw_id].name, event_fw[fw_id].module[module_id].name,
             	    event_fw[fw_id].module[module_id].eventtype[eventtype_id].name,
             	    event_fw[fw_id].module[module_id].eventtype[eventtype_id].event[event_id].name,
					event_format.event_value,
					event_format.module_reserved);

	return EVENT_OK;
}

void eventlog_parser_printf_all(void)
{
	int DW1 = 0;

	while  ((DW1+3) < memory_count)
	{
		eventlog_parser_printf(log_memory[DW1], log_memory[DW1+1], log_memory[DW1+2], log_memory[DW1+3], time_memory[DW1/4]);
		DW1 = DW1+4;
	}

}

int eventlog_parser_file(unsigned int DW1,unsigned int DW2, unsigned int DW3 ,unsigned int DW4, unsigned int time_unit, char * file, unsigned int file_size)
{

	int fw_id = 0;
	int module_id = 0;
	int eventtype_id = 0;
	int event_id = 0;
	char string[BUF_SIZE] = {0};
	int ret = 0;
	unsigned int file_index;

	struct event_format event_format;

	event_format.syncbyte = (DW1 & 0xff000000)>>24;
	event_format.event_value = DW2;
	event_format.module_reserved = DW3;
	/* time stamp */
	event_format.timestamp = DW4;

	if(event_format.syncbyte != EVENT_SYNCBYTE)
	{
		event_err_print("DW1 = %08x\n", DW1);
		event_err_print("syncbyte error");
		return EVENT_ERROR;
	}

	fw_id = GET_FW_ID(DW1);

	if(-1 == fw_id)
	{
		event_err_print("DW1 = %08x\n", DW1);
		return EVENT_ERROR;
	}

	module_id = GET_MODULE_ID(DW1, fw_id);

	if(-1 == module_id)
	{
		event_err_print("DW1 = %08x\n", DW1);
		return EVENT_ERROR;
	}

	eventtype_id = GET_EVENTTYPE_ID(DW1, fw_id, module_id);

	if(-1 == eventtype_id)
	{
		event_err_print(" DW1 = %08x\n", DW1);
		return EVENT_ERROR;
	}

	event_id = GET_EVENT_ID(DW1, fw_id, module_id, eventtype_id);

	if(-1 == event_id)
	{
		event_err_print("DW1 = %08x\n", DW1);
		return EVENT_ERROR;
	}

	if (time_unit)
		ret = snprintf(string, BUF_SIZE, "[%ld.%d%d%d%d%d%d][%s][%s][%s][%s],%x,%x \n",
					(event_format.timestamp/1000000),
					((event_format.timestamp%1000000)/100000),((event_format.timestamp%100000)/10000),
					((event_format.timestamp%10000)/1000),((event_format.timestamp%1000)/100),
					((event_format.timestamp%100)/10),((event_format.timestamp%10)),
					event_fw[fw_id].name, event_fw[fw_id].module[module_id].name,
             	    event_fw[fw_id].module[module_id].eventtype[eventtype_id].name,
             	    event_fw[fw_id].module[module_id].eventtype[eventtype_id].event[event_id].name,
					event_format.event_value,
					event_format.module_reserved);
	else
		ret = snprintf(string, BUF_SIZE, "[%ld.%d%d%d000][%s][%s][%s][%s],%x,%x \n",
					(event_format.timestamp/1000),((event_format.timestamp%1000)/100),
					((event_format.timestamp%100)/10),((event_format.timestamp%10)),
					event_fw[fw_id].name, event_fw[fw_id].module[module_id].name,
             	    event_fw[fw_id].module[module_id].eventtype[eventtype_id].name,
             	    event_fw[fw_id].module[module_id].eventtype[eventtype_id].event[event_id].name,
					event_format.event_value,
					event_format.module_reserved);

	/* calculate file size*/
	file_index = ret + strlen(file);

	if(file_index > file_size)
		return -1;

	strcat(file, string);

	return file_index;
}

unsigned int eventlog_parser_for_buf(unsigned int DW1,char *buf)
{

	int fw_id = 0;
	int module_id = 0;
	int eventtype_id = 0;
	int event_id = 0;

	struct event_format event_format;

	event_format.syncbyte = (DW1 & 0xff000000)>>24;


	if(event_format.syncbyte != EVENT_SYNCBYTE)
	{
		event_err_print("syncbyte error, syncbyte = %x\n", event_format.syncbyte);
		return EVENT_ERROR;
	}

	fw_id = GET_FW_ID(DW1);

	if(-1 == fw_id)
	{
		event_err_print("fw type error\n");
		return EVENT_ERROR;
	}

	module_id = GET_MODULE_ID(DW1, fw_id);

	if(-1 == module_id)
	{
		event_err_print("module type error\n");
		return EVENT_ERROR;
	}

	eventtype_id = GET_EVENTTYPE_ID(DW1, fw_id, module_id);

	if(-1 == eventtype_id)
	{
		event_err_print("event type error\n");
		return EVENT_ERROR;
	}

	event_id = GET_EVENT_ID(DW1, fw_id, module_id, eventtype_id);

	if(-1 == event_id)
	{
		event_err_print("event error\n");
		return EVENT_ERROR;
	}

	snprintf(buf, BUF_SIZE,"[%s]", event_fw[fw_id].module[module_id].eventtype[eventtype_id].event[event_id].name);



	return EVENT_OK;
}

void eventlog_show_define(void)
{
	int fw_id = 0;
	int module_id = 0;
	int eventtype_id = 0;
	int event_id = 0;

	for ( fw_id = 0; EVENT_LIMIT_MAX != event_fw[fw_id].fw_id; fw_id++)
	{
		event_alert_print("fw_id = %x, name = %s\n", event_fw[fw_id].fw_id, event_fw[fw_id].name);

		for ( module_id = 0; EVENT_LIMIT_MAX != event_fw[fw_id].module[module_id].module_id; module_id++)
		{
			event_alert_print("module_id = %x, name = %s\n", event_fw[fw_id].module[module_id].module_id,
								event_fw[fw_id].module[module_id].name);

			for( eventtype_id = 0; EVENT_LIMIT_MAX != event_fw[fw_id].module[module_id].eventtype[eventtype_id].eventtype_id; eventtype_id++)
			{
				event_alert_print("eventtype_id = %x, name = %s\n",
								event_fw[fw_id].module[module_id].eventtype[eventtype_id].eventtype_id,
								event_fw[fw_id].module[module_id].eventtype[eventtype_id].name);

				for( event_id = 0; EVENT_LIMIT_MAX != event_fw[fw_id].module[module_id].eventtype[eventtype_id].event[event_id].event_id; event_id++)
				{
					event_alert_print("event_id = %x, name = %s\n",
									event_fw[fw_id].module[module_id].eventtype[eventtype_id].event[event_id].event_id,
								    event_fw[fw_id].module[module_id].eventtype[eventtype_id].event[event_id].name);
				}
			}
		}
	}
}

unsigned int event_log_save(unsigned int fw_type, unsigned int event_type, unsigned int module, unsigned int event, unsigned int event_val, unsigned int module_reserved)
{
	struct event_format event_format;

	if(fw_type >= FW_MAX || module >= MODULE_MAX || event >= EVENT_MAX || event_type >= EVENT_TYPE_MAX)
	{
		event_err_print("(fw_type module event event_type) > Specified range\n");
		return EVENT_ERROR;
	}

	/* clean event_format */
	memset(&event_format, 0, sizeof(event_format));

	event_format.syncbyte = EVENT_SYNCBYTE;
	event_format.fw_type = fw_type;
	event_format.event_type = event_type;
	event_format.module = module;
	event_format.event = event;
	event_format.event_value = event_val;
	event_format.module_reserved = module_reserved;

	if(eventlog_time_unit)
	{
		event_format.timestamp = (unsigned int) rtk_timer_misc_90k_us();
	}
	else
	{
		event_format.timestamp = (unsigned int) rtk_timer_misc_90k_ms();
	}

	if(memory_count >= (INT_LOG_SIZE-1))
	{
		memory_count=0;
	}

	time_memory[memory_count/4] = eventlog_time_unit;

	/*DW1*/
	log_memory[memory_count++] = ((event_format.syncbyte) << 24) | ((event_format.fw_type) << 20) \
									| ((event_format.event_type) << 16) | (event_format.module << 8) \
									| (event_format.event);
	/*DW2*/
	log_memory[memory_count++] = event_format.event_value;

	/*DW3*/
	log_memory[memory_count++] = event_format.module_reserved;

	/*DW4*/
	log_memory[memory_count++] = event_format.timestamp;


	/*event_err_print("%08x-%08x-%08x-%08x\n",((event_format.syncbyte) << 24) | ((event_format.fw_type) << 20) \
							| ((event_format.event_type) << 16) | (event_format.module << 8) \
							| (event_format.event), event_format.event_value, event_format.module_reserved, event_format.timestamp);*/
	return EVENT_OK;
}
/*
unsigned int event_log_save_DW1(unsigned int DW1, unsigned int event_val, unsigned int module_reserved)
{
	struct event_format event_format;
	int time_memory_size = sizeof(time_memory);
	int log_memory_size = sizeof(log_memory);


	memset(&event_format, 0, sizeof(event_format));

	event_format.event_value = event_val;
	event_format.module_reserved = module_reserved;

	if(eventlog_time_unit)
	{
		event_format.timestamp = (unsigned int) rtk_timer_misc_90k_us();
	}
	else
	{
		event_format.timestamp = (unsigned int) rtk_timer_misc_90k_ms();
	}

	time_memory[((memory_count%time_memory_size)/4)] = eventlog_time_unit;


	log_memory[(memory_count++)%log_memory_size] = DW1;

	log_memory[(memory_count++)%log_memory_size] = event_format.event_value;


	log_memory[(memory_count++)%log_memory_size] = event_format.module_reserved;


	log_memory[(memory_count++)%log_memory_size] = event_format.timestamp;


	event_err_print("%08x-%08x-%08x-%08x\n", DW1, event_format.event_value, event_format.module_reserved, event_format.timestamp);
	return EVENT_OK;
}

unsigned int event_log_save_DW1_start(unsigned int DW1)
{
	int time_memory_size = sizeof(time_memory);
	int log_memory_size = sizeof(log_memory);

	time_memory[((memory_count%time_memory_size)/4)] = eventlog_time_unit;


	log_memory[(memory_count++)%log_memory_size] = DW1;

	log_memory[(memory_count++)%log_memory_size] = 0;


	log_memory[(memory_count++)%log_memory_size] = DIFF_START;


	if(eventlog_time_unit)
	{
		log_memory[(memory_count++)%log_memory_size] = (unsigned int) rtk_timer_misc_90k_us();
	}
	else
	{
		log_memory[(memory_count++)%log_memory_size] = (unsigned int) rtk_timer_misc_90k_ms();
	}

	event_err_print("%08x-%08x-%08x-%08x\n", log_memory[(memory_count-4)%log_memory_size], log_memory[(memory_count-3)%log_memory_size],\
					log_memory[(memory_count-2)%log_memory_size], log_memory[(memory_count-1)%log_memory_size]);

	return EVENT_OK;
}

unsigned int event_log_save_DW1_end(unsigned int DW1)
{
	int time_memory_size = sizeof(time_memory);
	int log_memory_size = sizeof(log_memory);

	time_memory[((memory_count%time_memory_size)/4)] = eventlog_time_unit;

	log_memory[(memory_count++)%log_memory_size] = DW1;

	log_memory[(memory_count++)%log_memory_size] = 0;

	log_memory[(memory_count++)%log_memory_size] = DIFF_END;

	if(eventlog_time_unit)
	{
		log_memory[(memory_count++)%log_memory_size] = (unsigned int) rtk_timer_misc_90k_us();
	}
	else
	{
		log_memory[(memory_count++)%log_memory_size] = (unsigned int) rtk_timer_misc_90k_ms();
	}


	event_err_print("%08x-%08x-%08x-%08x\n", log_memory[(memory_count-4)%log_memory_size], log_memory[(memory_count-3)%log_memory_size],\
					log_memory[(memory_count-2)%log_memory_size], log_memory[(memory_count-1)%log_memory_size]);
	return EVENT_OK;
}
*/
void event_set_timeunit(bool time)
{
	eventlog_time_unit = time;
}

void event_log_clean(void)
{
	memset(log_memory, 0, sizeof(log_memory));
	memory_count = 0;
}

void eventlog_printf(unsigned int DW1,unsigned int DW2, unsigned int DW3 ,unsigned int DW4)
{
	struct event_format event_format;

	event_format.syncbyte = (DW1 & 0xff000000)>>24;
	event_format.fw_type = (DW1 & 0x00f00000)>>20;
	event_format.module = (DW1 & 0x0000ff00)>>8;
	event_format.event_type = (DW1 & 0x000f0000)>>16;
	event_format.event = DW1 & 0x000000ff;
	event_format.event_value = DW2;
	event_format.module_reserved = DW3;
	/* time stamp */
	event_format.timestamp = DW4;

	event_alert_print("%08x-%08x-%08x-%08x\n",((event_format.syncbyte) << 24) | ((event_format.fw_type) << 20) \
							| ((event_format.event_type) << 16) | (event_format.module << 8) \
							| (event_format.event), event_format.event_value, event_format.module_reserved, event_format.timestamp);
}

void eventlog_printf_all(void)
{
	int DW1 = 0;

	while  ((DW1+3) < memory_count)
	{
		eventlog_printf(log_memory[DW1], log_memory[DW1+1], log_memory[DW1+2], log_memory[DW1+3]);
		DW1 = DW1+4;
	}
}

unsigned int eventlog_event_diff(unsigned int fw_type, unsigned int event_type, unsigned int module, unsigned int event)
{
	int cum = 0;
	int flag = 0;
	unsigned int time1 = 0;
	unsigned int time2 = 0;
	unsigned int DW1 = 0;
	struct event_format event_format;
	char buf[BUF_SIZE]={0};
	int ret = 1;

	if(memory_count == 0){

		event_alert_print("memory_count = 0\n");
		return EVENT_ERROR;
	}

	event_format.syncbyte = EVENT_SYNCBYTE;
	event_format.fw_type = fw_type;
	event_format.event_type = event_type;
	event_format.module = module;
	event_format.event = event;

	DW1 = ((event_format.syncbyte) << 24) | ((event_format.fw_type) << 20) \
									| ((event_format.event_type) << 16) | (event_format.module << 8) \
									| (event_format.event);



	ret = eventlog_parser_for_buf(DW1,buf);

	while(cum <= (memory_count-3))
	{
		if(log_memory[cum] == DW1)
		{
			if(0 != flag)
			{
				time2 = log_memory[cum+3];
				if(EVENT_ERROR == ret)
				{
					event_alert_print("%x diff = %d\n", DW1, time2 - time1);
				}
				else
				{
					event_alert_print("%s diff = %d\n", buf, time2 - time1);
				}
				time1 = time2;
				flag ++;
			}
			else
			{
				time1 = log_memory[cum+3];
				flag ++;
			}
		}
		cum = cum+4;
	}

	if(flag == 0)
	{
		if(EVENT_ERROR == ret)
		{
			event_alert_print("no find: %x\n", DW1);
		}
		else
		{
			event_alert_print("no find: %s\n", buf);
		}
	}
	if(flag == 1)
	{
		if(EVENT_ERROR == ret)
		{
			event_alert_print("Only one: %x\n", DW1);
		}
		else
		{
			event_alert_print("Only one: %s\n", buf);
		}
	}



	return 1;
}

unsigned int eventlog_event_two_diff(unsigned int DW11, unsigned int DW12)
{
	int cum = 0;
	unsigned int time1 = 0;
	unsigned int time2 = 0;
	char buf1[BUF_SIZE]={0};
	char buf2[BUF_SIZE]={0};
	int ret1 = 1;
	int ret2 = 1;

	if(memory_count == 0){

		event_alert_print("memory_count = 0\n");
		return EVENT_ERROR;
	}

	ret1 = eventlog_parser_for_buf(DW11,buf1);
	ret2 = eventlog_parser_for_buf(DW12,buf2);

	while(cum <= (memory_count-3))
	{
		if(log_memory[cum] == DW11)
		{
			time1 = log_memory[cum+3];
			cum = cum+4;
			continue;
		}

		if(log_memory[cum] == DW12)
		{
			time2 = log_memory[cum+3];
			cum = cum+4;
			continue;
		}

		if( time1 != 0 && time2 != 0)
			break;
		cum = cum+4;
	}

	if(time1 == 0 || time2 == 0)
	{
		event_err_print("no find\n");
		return EVENT_ERROR;
	}

	if(EVENT_ERROR == ret1 || EVENT_ERROR == ret2)
	{
		event_alert_print("[%x] [%x] time diff = %d ms\n", DW11, DW12, ((time2>time1)?(time2-time1):(time1-time2)));
	}
	else
	{
		event_alert_print("%s %s time diff = %d ms\n", buf1, buf2, ((time2>time1)?(time2-time1):(time1-time2)));
	}

	return EVENT_OK;
}

unsigned int eventlog_DW1_diff(unsigned int DW1)
{
	int cum = 0;
	unsigned int time1 = 0;
	unsigned int time2 = 0;
	char buf1[BUF_SIZE]={0};
	int ret1 = 1;

	if(memory_count == 0){

		event_alert_print("memory_count = 0\n");
		return EVENT_ERROR;
	}

	ret1 = eventlog_parser_for_buf(DW1,buf1);

	while(cum <= (memory_count-3))
	{
		if(log_memory[cum] == DW1 && log_memory[cum+2] == DIFF_START)
		{
			time1 = log_memory[cum+3];
		}

		if(log_memory[cum] == DW1 && log_memory[cum+2] == DIFF_END)
		{
			time2 = log_memory[cum+3];
		}

		if( time1 != 0 && time2 != 0)
			break;
		cum = cum+4;
	}

	if(EVENT_ERROR == ret1)
	{
		event_alert_print("[%x] time diff = %d ms\n", DW1, ((time2>time1)?(time2-time1):(time1-time2)));
	}
	else
	{
		event_alert_print("%s time diff = %d ms\n", buf1, ((time2>time1)?(time2-time1):(time1-time2)));
	}

	return EVENT_OK;
}

unsigned int eventlog_event_number(unsigned int fw_type, unsigned int event_type, unsigned int module, unsigned int event)
{
	int cum = 0;
	int number = 0;
	unsigned int DW1 = 0;
	struct event_format event_format;

	event_format.syncbyte = EVENT_SYNCBYTE;
	event_format.fw_type = fw_type;
	event_format.event_type = event_type;
	event_format.module = module;
	event_format.event = event;

	DW1 = ((event_format.syncbyte) << 24) | ((event_format.fw_type) << 20) \
									| ((event_format.event_type) << 16) | (event_format.module << 8) \
									| (event_format.event);


	while(cum < memory_count)
	{
		if(log_memory[cum] == DW1)
		{
			number++;
		}
		cum = cum+4;
	}

	return number;
}

void eventlog_event_printf_number(unsigned int fw_type, unsigned int event_type, unsigned int module, unsigned int event)
{
	unsigned int DW1 = 0;
	struct event_format event_format;

	event_format.syncbyte = EVENT_SYNCBYTE;
	event_format.fw_type = fw_type;
	event_format.event_type = event_type;
	event_format.module = module;
	event_format.event = event;
	if(memory_count == 0)
		event_alert_print("memory_count = 0\n");
	else
		event_alert_print("number: %d\n", DW1, eventlog_event_number(fw_type, event_type, module, event));

}


void eventlog_90k_enable(void)
{
	rtd_outl(TIMER_CLK90K_CTRL,1);
}

unsigned int eventlog_merge_DW1(unsigned int fw_type, unsigned int event_type, unsigned int module, unsigned int event)
{
	struct event_format event_format;

	event_format.syncbyte = EVENT_SYNCBYTE;
	event_format.fw_type = fw_type;
	event_format.event_type = event_type;
	event_format.module = module;
	event_format.event = event;

	return (((event_format.syncbyte) << 24) | ((event_format.fw_type) << 20) \
									| ((event_format.event_type) << 16) | (event_format.module << 8) \
									| (event_format.event));
}