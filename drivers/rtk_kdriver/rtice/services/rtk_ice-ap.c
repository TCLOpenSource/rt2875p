#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/moduleparam.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/gfp.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/module.h>
#ifdef CONFIG_SUPPORT_SCALER
#include <tvscalercontrol/scaler/scalerthread_rtice2ap.h>
static ap_data_param rtice_param;
#endif
#include "rtk_ice-ap.h"
#include <rtd_log/rtd_module_log.h>

/*default size of single package*/
#define PACKAGE_SIZE    2046
/*flag while write data finished*/
static char recv_finish_flag = 0;
/*size of ap data contains data type*/
unsigned int rtice_data_size = 0;
/*data buffer*/
unsigned char *rtice_ap_buffer = NULL;

/*------------------------------------------------------------------
 * Func : rtice_ap_command_handler
 *
 * Desc : proc ap command
 *
 * Parm : cmd_data : command data
 *        command_data_len  : command data len
 *
 * Retn :  < 0    : error
 * others : length of response data
	Read AP ID SIZE value:
		op_code(0xF0) command_data_len id[4] size[2] rtice_checksum
	Read data package num:
		op_code(0xF1) 0x1 package_num rtice_checksum
	Read data finished:
		op_code(0xF2) 0 rtice_checksum
	Write AP ID SIZE value:
		op_code(0xF3) command_data_len id[4] size[2] rtice_checksum
	Write AP num package data:
		op_code(0xF4) command_data_len package_num data rtice_checksum
	Write AP data status:
		op_cod(0xF5) 0 rtice_checksum
 *------------------------------------------------------------------*/
int rtice_ap_command_handler(\
				unsigned char opcode,
				unsigned char* cmd_data,\
				unsigned int command_data_len,\
				unsigned char* response_data,\
				unsigned int response_buff_size)
{
	int ret = 0;
	int package_num;
	unsigned char num;
	int last_package_size;
	char*read_buffer_tmp = NULL;

	switch(opcode){
	case RTICE_AP_CMD_READ_ID_SIZE:
	{
		if(command_data_len  != 16){
			ret = -RTICE_ERR_INVALID_PARAM;
			goto error_return;
		}
		rtice_data_size = B4TL(cmd_data[10],cmd_data[11],cmd_data[12],cmd_data[13]);
		if(rtice_data_size == 0){
			ret = -RTICE_ERR_INVALID_PARAM;
			goto error_return;
		}
		if(rtice_ap_buffer != NULL)
			kfree(rtice_ap_buffer);
		rtice_ap_buffer = (char *)kmalloc(rtice_data_size,GFP_KERNEL);
		if(rtice_ap_buffer == NULL)
			return -RTICE_AP_ERR_ABORT;
	#ifdef CONFIG_SUPPORT_SCALER
		rtice_param.id = (((unsigned short)cmd_data[0]<<8) + ((unsigned short)cmd_data[1]));
		rtice_param.saddr= B4TL(cmd_data[2],cmd_data[3],cmd_data[4],cmd_data[5]);
		rtice_param.eaddr= B4TL(cmd_data[6],cmd_data[7],cmd_data[8],cmd_data[9]);
		rtice_param.mode0= cmd_data[14];
		rtice_param.mode1= cmd_data[15];
		rtice_param.size = rtice_data_size - 1;
		ret = rtice_get_vip_table(rtice_param, &rtice_ap_buffer[0],&rtice_ap_buffer[1]);
		if(ret < 0)
			ret = -RTICE_AP_ERR_INVALID_ID_SIZE;
		else
			ret = 0;
	#else
		ret = -RTICE_AP_ERR_UNIMPLEMENT;
	#endif
	}
	break;
	case RTICE_AP_CMD_READ_NUM_DATA:
	{
		if (command_data_len != 1){
			ret = -RTICE_ERR_INVALID_PARAM;
			goto error_return;
		}
		if(rtice_ap_buffer == NULL){
			ret = -RTICE_AP_ERR_ABORT;
			goto error_return;
		}
		read_buffer_tmp = rtice_ap_buffer;
		num = cmd_data[0];
		last_package_size = rtice_data_size % PACKAGE_SIZE;
		if(last_package_size){
			package_num = rtice_data_size / PACKAGE_SIZE + 1;
		}else{
			package_num = rtice_data_size / PACKAGE_SIZE;
			last_package_size = PACKAGE_SIZE;
		}
		read_buffer_tmp += (PACKAGE_SIZE * (num - 1));
		response_data[0] = num;
		if((num > 0 ) && (num < package_num)){
			memcpy(&response_data[1],read_buffer_tmp,PACKAGE_SIZE);
			read_buffer_tmp += PACKAGE_SIZE;
			ret = PACKAGE_SIZE + 1;
		}else if(num == package_num){
			if(last_package_size == 0)
				last_package_size = PACKAGE_SIZE;
			memcpy(&response_data[1],read_buffer_tmp,last_package_size);
			read_buffer_tmp += last_package_size;
			ret = last_package_size + 1;
		}else{
			ret = -RTICE_AP_ERR_INVALID_PACKAGE_NUM;
			goto error_return;
		}
	}
	break;
	case RTICE_AP_CMD_READ_DATA:
	{
		unsigned int addr = 0,len = 0;

		if(command_data_len  != 8){
			ret = -RTICE_ERR_INVALID_PARAM;
			goto error_return;
		}
		addr = B4TL(cmd_data[0],cmd_data[1],cmd_data[2],cmd_data[3]);
		len = B4TL(cmd_data[4],cmd_data[5],cmd_data[6],cmd_data[7]);
		if(!rtice_ap_buffer || (addr + len) > rtice_data_size){
			ret = -RTICE_AP_ERR_READ_DATA_FAILED;
			goto error_return;
		}
		memcpy(&response_data[0],rtice_ap_buffer + addr,len);
		ret = len;
	}
	break;
	case RTICE_AP_CMD_READ_REC_FINISH:
	{
		if(rtice_ap_buffer != NULL){
			kfree(rtice_ap_buffer);
			rtice_ap_buffer = NULL;
		}
	}
	break;
	case RTICE_AP_CMD_WRITE_ID_SIZE: /*write data to ap flow*/
	{
		if(command_data_len  != 16){
			ret = -RTICE_ERR_INVALID_PARAM;
			goto error_return;
		}
		rtice_data_size = B4TL(cmd_data[10],cmd_data[11],cmd_data[12],cmd_data[13]);
		if(rtice_ap_buffer != NULL)
			kfree(rtice_ap_buffer);
		rtice_ap_buffer = (char *)kmalloc(rtice_data_size,GFP_KERNEL);
		if(rtice_ap_buffer == NULL)
			return -RTICE_AP_ERR_ABORT;
	#ifdef CONFIG_SUPPORT_SCALER
		rtice_param.id = (((unsigned short)cmd_data[0]<<8) + ((unsigned short)cmd_data[1]));
		rtice_param.saddr= B4TL(cmd_data[2],cmd_data[3],cmd_data[4],cmd_data[5]);
		rtice_param.eaddr= B4TL(cmd_data[6],cmd_data[7],cmd_data[8],cmd_data[9]);
		rtice_param.mode0= cmd_data[14];
		rtice_param.mode1= cmd_data[15];
		rtice_param.size = rtice_data_size;
	#else
		ret = -RTICE_AP_ERR_UNIMPLEMENT;
	#endif
	}
	break;
	case RTICE_AP_CMD_WRITE_NUM_DATA:
	{
		if(rtice_ap_buffer == NULL){
			ret = -RTICE_AP_ERR_ABORT;
			goto error_return;
		}
		recv_finish_flag = 0;
		read_buffer_tmp = rtice_ap_buffer;
		num = cmd_data[0];
		last_package_size = rtice_data_size%PACKAGE_SIZE;
		if(last_package_size){
			package_num = rtice_data_size/PACKAGE_SIZE +1;
		}else{
			package_num = rtice_data_size/PACKAGE_SIZE;
			last_package_size = PACKAGE_SIZE;
		}
		read_buffer_tmp += (PACKAGE_SIZE * (num - 1));
		if((num > 0 )&& (num < package_num)){
			if((command_data_len - 1) !=PACKAGE_SIZE){
				ret = -RTICE_AP_ERR_WRITE_DATA_FAILED;
				goto error_return;
			}
			memcpy(read_buffer_tmp,&cmd_data[1],PACKAGE_SIZE);
			read_buffer_tmp += PACKAGE_SIZE;
		}else if(num == package_num){
			if((command_data_len - 1) !=last_package_size){
				ret = -RTICE_AP_ERR_WRITE_DATA_FAILED;
				goto error_return;
			}
			memcpy(read_buffer_tmp,&cmd_data[1],last_package_size);
			read_buffer_tmp += last_package_size;
			/*receive last package means receive finished*/
			recv_finish_flag = 1;
		}else{
			ret = -RTICE_AP_ERR_INVALID_PACKAGE_NUM;
			goto error_return;
		}
		response_data[0] = num;
		ret = 1;
		RTICE_DEBUG("[%s]recv_finish_flag=%d\r\n",__func__,recv_finish_flag);
	}
	break;
	case RTICE_AP_CMD_WRITE_DATA:
	{
		unsigned int addr = 0,len = 0;

		if(command_data_len < 4){
			ret = -RTICE_ERR_INVALID_PARAM;
			goto error_return;
		}
		recv_finish_flag = 0;
		addr = B4TL(cmd_data[0],cmd_data[1],cmd_data[2],cmd_data[3]);
		len = command_data_len - 4;
		if(!rtice_ap_buffer || (addr + len) > rtice_data_size){
			ret = -RTICE_AP_ERR_WRITE_DATA_FAILED;
			goto error_return;
		}
		memcpy(rtice_ap_buffer + addr,&cmd_data[4],len);
		if((addr + len) == rtice_data_size)
			recv_finish_flag = 1;
		ret = 0;
	}
	break;
	case RTICE_AP_CMD_WRITE_STATUS:
	{
		if(!recv_finish_flag){
			ret = -RTICE_AP_ERR_WRITE_DATA_FAILED;
			goto error_return;
		}
	#ifdef CONFIG_SUPPORT_SCALER
		ret = rtice_set_vip_table(rtice_param,rtice_ap_buffer);
		if(ret < 0)
			ret = -RTICE_AP_ERR_INVALID_ID_SIZE;
		else
			ret = 0;
	#else
		ret = -RTICE_AP_ERR_UNIMPLEMENT;
	#endif
	}
	break;
	default:
		ret = -RTICE_ERR_UNSUPPORTED_OP_CODE;
	}
	return ret;
error_return:
	if(rtice_ap_buffer){
		kfree(rtice_ap_buffer);
		rtice_ap_buffer = NULL;
	}
	return ret;
}

/*------------------------------------------------------------------
 * Func : rtice_ap_command_probe
 *
 * Desc : proc ap command
 *
 * Parm : opcode
 *
 * Retn :
 *------------------------------------------------------------------*/
int rtice_ap_command_probe(unsigned char op_code)
{
	return (GET_OP_CODE_COMMAND(op_code) <= 7) ? 1 : 0;
}

rtice_command_handler_t ap_cmd_handler[] =
{
	{
		RTICE_CMD_GROUP_ID_AP,
		rtice_ap_command_probe,
		rtice_ap_command_handler
	},
};

/*------------------------------------------------------------------
 * Func : rtice_ap_tool_init
 *
 * Desc : register rtice ap tool
 *
 * Parm : N/A
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
int rtice_ap_tool_init(void)
{
	rtice_register_command_handler(ap_cmd_handler, \
		sizeof(ap_cmd_handler)/sizeof(rtice_command_handler_t));
	return 0;
}

/*------------------------------------------------------------------
 * Func : rtice_ap_tool_exit
 *
 * Desc : exit rtice ap tool
 *
 * Parm : N/A
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
void rtice_ap_tool_exit(void)
{
	rtice_unregister_command_handler(ap_cmd_handler, \
		sizeof(ap_cmd_handler)/sizeof(rtice_command_handler_t));
	rtd_pr_rtice_info("rtice_unregister_command_handler() has been executed.\n");
}

