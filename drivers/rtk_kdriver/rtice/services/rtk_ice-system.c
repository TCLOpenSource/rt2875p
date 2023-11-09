#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/gfp.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <mach/platform.h>
#include <rtk_sb2_callback.h>
#include <linux/version.h>
#include "panelConfigParameter.h"
#include "rtk_ice-system.h"
#if IS_ENABLED(CONFIG_RTK_KDRV_PWM)
#include "rtk_pwm_rtice.h"
#endif
#include <linux/string.h>
#include <rtd_log/rtd_module_log.h>

//#define CONFIG_SUPPORT_AUDIO_VX_DAP
//#define CONFIG_SUPPORT_RW_FS

#ifdef CONFIG_SUPPORT_AUDIO_VX_DAP
#include "rtkaudio_func.h"
#endif

#ifdef CONFIG_RTK_KDRV_AUDIO
#define CONFIG_RTICE_SUPPORT_AUDIO_EQBass
#endif

#ifdef CONFIG_RTICE_SUPPORT_AUDIO_EQBass
#include "AudioRPCBaseDS_data.h"
extern int get_EQ_BASS_audio_data_path_status(ENUM_AUDIO_EQ_BASS_MODE mode, int *bOnOff);
extern int get_EQ_BASS_enable(ENUM_AUDIO_EQ_BASS_MODE mode, int band_idx, int *bOnOff);
extern int get_EQ_BASS_parameter(ENUM_AUDIO_EQ_BASS_MODE mode, int band_idx, int *fc, int *bw, int *gain);
extern int set_EQ_BASS_Audio_Data_Path_Enable(ENUM_AUDIO_EQ_BASS_MODE mode, int bOnOff);
extern int set_EQ_BASS_enable(ENUM_AUDIO_EQ_BASS_MODE mode, int band_idx, int bOnOff);
extern int set_EQ_BASS_parameter(ENUM_AUDIO_EQ_BASS_MODE mode, int band_idx, ENUM_AUDIO_EQ_BASS_FILTER_TYPE filter_type, int fc, int bw, int gain);
#endif

#if IS_ENABLED(CONFIG_REALTEK_PCBMGR)
#include "mach/pcbMgr.h"
#define PCB_NAME_BUFFER_SIZE 1024
#define FS_OPT_BUFFER_SIZE 2046

int get_pcb_names(char *tmp)
{
	char const *patternsplit = "\x2C";
	char *buf, *token, *pch;
	char pattern[16];
	char *names;
	int ret;
	
	if(tmp==NULL)
	{
	    RTICE_DEBUG("%s tmp is null\n",__func__);
            return -1;
        }

	names = (char *)kmalloc(PCB_NAME_BUFFER_SIZE, GFP_ATOMIC);
	if(names==NULL) {
	    RTICE_DEBUG("%s alloc for names failed\n",__func__);
	    return -1;
	}
		
	memset(names, 0, PCB_NAME_BUFFER_SIZE);
	buf = platform_info.pcb_enum_parameter;
	if(buf==NULL) {
	    RTICE_DEBUG("%s buf is null\n",__func__);
	    kfree(names);
	    return -1;
	}

	RTICE_DEBUG("[%s] pcb len = %d, get buf strlen = %d \n\r", __func__,\
		     	strlen(platform_info.pcb_enum_parameter), strlen(buf));
	sprintf(pattern, "pcb_enum=");
	while ((pch = strsep(&buf, patternsplit))) {
		if(pch) {
			token = strstr(pch, pattern);
			if (token!=NULL) {
				if (strlen(names)){
					snprintf(names+strlen(names), PCB_NAME_BUFFER_SIZE, ",%s", token + strlen(pattern));
				}
				else{
					sprintf(names, "%s", token + strlen(pattern));
				}
			}
		}
	}
	ret = sprintf(tmp, "%s", names);
	RTICE_DEBUG("[%s] -------ret = %d \n\r", __func__, ret);
	kfree(names);
	return ret;
}
#endif	/*  */
#ifdef CONFIG_SUPPORT_RW_FS
/*File operation cmd methods defined here*/
static struct file *gfp = 0;
/**Close file that been opened by 'openFile'**/
static int closeFile(void)
{
	if(gfp != 0 && !IS_ERR(gfp))
		filp_close(gfp,0);
	gfp = 0;
	return 0;
}
/**Open file specified by pFilename and returns file size saved at pSize while read successfully**/
static int openFile(const char*pFilename,unsigned int*pSize,int fWrite)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	mm_segment_t oldFs;
#endif

	*pSize = 0;
	/*try close file previous opened*/
	closeFile();
	/*set fs*/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	oldFs = get_fs();
	set_fs(KERNEL_DS);
#endif
	/*try open file*/
	if(fWrite)
		gfp = filp_open(pFilename,O_WRONLY,0);
	else
		gfp = filp_open(pFilename,O_RDONLY,0);
	if(gfp == 0 || IS_ERR(gfp)){
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
		set_fs(oldFs);
	#endif
		return PTR_ERR(gfp);
	}
	*pSize = gfp->f_path.dentry->d_inode->i_size;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	set_fs(oldFs);
#endif
	return 0;
}
/**Read file data at address specified by 'offset'.And total read size specified by 'size'.
	The data are saved in the buffer whose address is 'pOut'.
	Notice that it must called after 'openFile' method otherwise returns false.
**/
static int readFileData(unsigned int offset,unsigned int size,char*pOut)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	mm_segment_t oldFs;
#endif
	unsigned int filesize = 0;
	loff_t pos = offset;

	if(size > FS_OPT_BUFFER_SIZE)
		return -RTICE_FS_OPT_ERR_WRONG_SIZE;
	if(gfp == 0 || IS_ERR(gfp))
		return -RTICE_FS_OPT_ERR_OPEN;
	filesize = gfp->f_path.dentry->d_inode->i_size;
	if(offset + size > filesize)
		return -RTICE_FS_OPT_ERR_WRONG_SIZE;
	/*read data*/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	oldFs = get_fs();
	set_fs(KERNEL_DS);
#endif
	kernel_read(gfp,pOut,size, &pos);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	set_fs(oldFs);
#endif
	return size;
}
/**Write data to file at address specified by 'offset'.pData and size indicate data for writting.
	Notice that it must called after 'openFile' method otherwise returns false.
**/
static int writeFileData(unsigned int offset,unsigned int size,char*pData)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	mm_segment_t oldFs;
#endif
	unsigned int filesize = 0;
	loff_t pos = offset;

	if(size > FS_OPT_BUFFER_SIZE)
		return -RTICE_FS_OPT_ERR_WRONG_SIZE;
	if(gfp == 0 || IS_ERR(gfp))
		return -RTICE_FS_OPT_ERR_OPEN;
	filesize = gfp->f_path.dentry->d_inode->i_size;
	if(offset + size > filesize)
		return -RTICE_FS_OPT_ERR_WRONG_SIZE;
	/*write data*/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	oldFs = get_fs();
	set_fs(KERNEL_DS);
#endif
	kernel_write(gfp,pData,size, &pos);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	set_fs(oldFs);
#endif
	return 0;
}
#endif

/**
    Get/Set VX parameters in audio.
    the 'index' see AUDIO_PARAM_TYPE_T
**/
#ifdef CONFIG_RTK_KDRV_AUDIO
#endif
static int rticeAudioParam(ENUM_RTICE_AUDIO_PARAM_GROUP group,int index,int*v,int size)
{
	int ret = -RTICE_AUDIO_VX_UNSUPPORT;
	switch(group){
	case apVX:
	case apDAP:
	{
	#ifdef CONFIG_SUPPORT_AUDIO_VX_DAP
		if(getAudioParam((AUDIO_PARAM_TYPE_T)index,v,size) < 0)
			ret = -RTICE_AUDIO_VX_FAILED;
		else
			ret = size;
	#endif
	}
	break;
	case apEQBass:
	{
	#ifdef CONFIG_RTICE_SUPPORT_AUDIO_EQBass
		ENUM_AUDIO_EQBassTreble_FILTER_PARAM paramType = (index >> 24) & 0xff;
		int mode = (index >> 16) & 0xff;
		//int filter = (index >> 8) & 0xff;
		int band = index & 0xff;

		ret = -RTICE_AUDIO_EQ_UNSUPPORT_PARAM;
		switch(paramType){
		case aefModuleEn:
		{
			ret = get_EQ_BASS_audio_data_path_status(mode,v);
			if(ret < 0){
				ret = -RTICE_AUDIO_EQ_AUDIO_DRIVER_ERR;
			}else{
				v[0] *= 1000;
				ret = 4;
			}
		}
		break;
    	case aefFilterEn:
		{
			ret = get_EQ_BASS_enable(mode, band, v);
			if(ret < 0){
				ret = -RTICE_AUDIO_EQ_AUDIO_DRIVER_ERR;
			}else{
				v[0] *= 1000;
				ret = 4;
			}	
		}		
		break;
    	case aefFilterFc:
    	case aefFilterBw:
    	case aefFilterGain:
		{
			int fc = 0,bw = 0,gain = 0;
			ret = get_EQ_BASS_parameter(mode, band, &fc, &bw, &gain);
			if(ret < 0){
				ret = -RTICE_AUDIO_EQ_AUDIO_DRIVER_ERR;
			}else{
				if(paramType == aefFilterFc)
					v[0] = fc;
				else if(paramType == aefFilterBw)
					v[0] = bw;
				else
					v[0] = gain; 
				ret = 4;
			}
		}
		break;
		default:;
		}
	#endif
	}
	break;
	default:;
	}
	return ret;
}

static int setRticeAudioParam(ENUM_RTICE_AUDIO_PARAM_GROUP group,int index,int* v,int size)
{
	int ret = -RTICE_AUDIO_VX_UNSUPPORT;
	switch(group){
	case apVX:
	case apDAP:
	{
	#ifdef CONFIG_SUPPORT_AUDIO_VX_DAP
		if(setAudioParam((AUDIO_PARAM_TYPE_T)index,v,size) < 0)
			ret = -RTICE_AUDIO_VX_FAILED;
		else
			ret = 0;
	#endif
	}
	break;
	case apEQBass:
	{
	#ifdef CONFIG_RTICE_SUPPORT_AUDIO_EQBass
		ENUM_AUDIO_EQBassTreble_FILTER_PARAM paramType = (index >> 24) & 0xff;
		int mode = (index >> 16) & 0xff;
		int filter = (index >> 8) & 0xff;
		int band = index & 0xff;

		ret = -RTICE_AUDIO_EQ_UNSUPPORT_PARAM;
		switch(paramType){
		case aefModuleEn:
		{
			ret = set_EQ_BASS_Audio_Data_Path_Enable(mode,v[0] / 1000);
			if(ret < 0)
				ret = -RTICE_AUDIO_EQ_AUDIO_DRIVER_ERR;
			else
				ret = 0;
		}
		break;
    	case aefFilterEn:
		{
			ret = set_EQ_BASS_enable(mode, band, v[0] / 1000);
			if(ret < 0)
				ret = -RTICE_AUDIO_EQ_AUDIO_DRIVER_ERR;
			else
				ret = 0;	
		}		
		break;
    	case aefFilterFc:
    	case aefFilterBw:
    	case aefFilterGain:
		{
			int fc = 0,bw = 0,gain = 0;
			ret = get_EQ_BASS_parameter(mode, band, &fc, &bw, &gain);
			if(ret < 0){
				ret = -RTICE_AUDIO_EQ_AUDIO_DRIVER_ERR;
			}else{
				if(paramType == aefFilterFc)
					fc = v[0];
				else if(paramType == aefFilterBw)
					bw = v[0];
				else
					gain = v[0];
				ret = set_EQ_BASS_parameter(mode, band, filter, fc, bw, gain);
				if(ret < 0)
					ret = -RTICE_AUDIO_EQ_AUDIO_DRIVER_ERR;
				else
					ret = 0;
			}
		}
		break;
		default:;
		}
	#endif
	}
	break;
	default:;
	}
	return ret;
}

/*------------------------------------------------------------------
 * Func : rtice_system_command_handler
 *
 * Desc : proc iomem command
 *
 * Parm : cmd_data : command data
 *        command_data_len  : command data len
 *
 * Retn :  < 0    : error
 *         others : length of response data
 *------------------------------------------------------------------*/
int rtice_system_command_handler(\
				unsigned char opcode, unsigned char *cmd_data,\
				unsigned int command_data_len,\
				unsigned char *response_data,\
				unsigned int response_buff_size )
{
	int ret = 0;
#if IS_ENABLED(CONFIG_REALTEK_PCBMGR)
    char pcbname[512];
	unsigned long long pcbvalue;
#endif	/*  */

	switch (opcode){
	case RTICE_SYS_CMD_SET_BUSERR_IGNORE_ADDR:
		if (command_data_len < 4)
			return -RTICE_ERR_INVALID_PARAM;
		{
#if IS_ENABLED(CONFIG_RTK_KDRV_SB2)
			unsigned long addr = B4TL(cmd_data[0], cmd_data[1],
						  cmd_data[2], cmd_data[3]);

			BUSERR_IGNORE_SET(addr);
#endif
		}
		break;
	case RTICE_SYS_CMD_READ_SYSTEM_INFO:
		if (command_data_len != 1)
			return -RTICE_ERR_INVALID_PARAM;
		switch (cmd_data[0]){
		case RTICE_SYSTEM_INFO_ID_CPU_ID:
			ret = sprintf(response_data,"%x",platform_info.cpu_id);
		break;
		case RTICE_SYSTEM_INFO_ID_BOARD_ID:
			ret = sprintf(response_data,"%x",\
						platform_info.board_id);
		break;
		case RTICE_SYSTEM_INFO_ID_BOOTLOADER_VERSION:
			ret = sprintf(response_data, "%s",\
					platform_info.bootloader_version);
		break;
		case RTICE_SYSTEM_INFO_ID_AUDIO_FW_VERSION:
			ret = sprintf(response_data, "Unknown");
		break;
		case RTICE_SYSTEM_INFO_ID_VIDEO_FW_VERSION:
			ret = sprintf(response_data, "Unknown");
		break;
		case RTICE_SYSTEM_INFO_ID_KERNEL_SOURCE_CODE_INFO:
			ret = sprintf(response_data, "%s",\
					platform_info.kernel_source_code_info);
		break;
		case RTICE_SYSTEM_INFO_ID_SYSTEM_PARAMETERS:
			ret = sprintf(response_data, "%s", \
					platform_info.system_parameters);
		break;
		case RTICE_SYSTEM_INFO_ID_PANEL_NAME:
		{
			PANEL_CONFIG_PARAMETER *panel_parameter = NULL;
			panel_parameter = (PANEL_CONFIG_PARAMETER*)&platform_info.panel_parameter;
			ret = snprintf(response_data,sizeof(panel_parameter->sPanelName),"%s", \
					panel_parameter->sPanelName);
		}
		break;
		default:
			ret = -RTICE_IOMEM_ERR_INVALID_SYSTEM_INFO;
		}
		if (ret <= 0)
			ret = -RTICE_IOMEM_ERR_INVALID_SYSTEM_INFO;
	break;
	case RTICE_SYS_CMD_READ_PCB_PARAMETER:
	#if IS_ENABLED(CONFIG_REALTEK_PCBMGR)
		switch (cmd_data[0]) {
		case RTICE_PCB_INFO_SEARCH:
            if(sizeof(pcbname) - 1 < command_data_len)
                snprintf(pcbname, sizeof(pcbname) - 1, "%s",cmd_data + 1);
            else
                snprintf(pcbname, command_data_len, "%s",cmd_data + 1);
			ret = pcb_mgr_get_enum_info_byname(pcbname, &pcbvalue);
			ret = sprintf(response_data, "%x",\
						(unsigned int)pcbvalue);
		break;
		case RTICE_PCB_INFO_GET_ALL_NAME:
			ret = get_pcb_names(response_data);
		break;
		default:
			ret = -RTICE_ERR_UNSUPPORTED_OP_CODE;
		};
		#else
		ret = -RTICE_ERR_UNSUPPORTED_OP_CODE;
		#endif
	break;
#ifdef CONFIG_SUPPORT_RW_FS
	case RTICE_SYS_CMD_FS_OPT:
	{
		ENUM_RTICE_FS_OPT opt = (ENUM_RTICE_FS_OPT)cmd_data[0];
		if (command_data_len < 1)
			return -RTICE_ERR_INVALID_PARAM;
		if(opt == RTICE_FS_OPT_OPEN || opt == RTICE_FS_OPT_OPEN_RW){
			unsigned int size = 0;
			
			cmd_data[command_data_len] = '\0';
			if(opt == RTICE_FS_OPT_OPEN)
				ret = openFile((char*)&cmd_data[1],&size,0);
			else
				ret = openFile((char*)&cmd_data[1],&size,1);
			if(ret == 0){
				LTB4(size,response_data[0],response_data[1],\
						response_data[2],response_data[3]);
				ret = 4;
			}
		}else if(opt == RTICE_FS_OPT_CLOSE){
			ret = closeFile();
		}else if(opt == RTICE_FS_OPT_READ){
			unsigned int offset = 0,size = 0;

			if(command_data_len != 9)
				return -RTICE_ERR_INVALID_PARAM;
			offset = B4TL(cmd_data[1],cmd_data[2],cmd_data[3],cmd_data[4]);
			size = B4TL(cmd_data[5],cmd_data[6],cmd_data[7],cmd_data[8]);
			ret = readFileData(offset,size,response_data);
		}else if(opt == RTICE_FS_OPT_WRITE){
			unsigned int offset = 0,size = 0;

			if(command_data_len < 5)
				return -RTICE_ERR_INVALID_PARAM;
			offset = B4TL(cmd_data[1],cmd_data[2],cmd_data[3],cmd_data[4]);
			size = command_data_len - 5;
			if(size)
				ret = writeFileData(offset,size,&cmd_data[5]);
		}else{
			return -RTICE_ERR_INVALID_PARAM;
		}
	}
	break;
#endif
        case RTICE_SYS_CMD_PWM_PANEL:/*r/w param about PWM-Panel*/
	{
		unsigned char funcId = 0;
		int paramLen = 0;
		
		if(command_data_len < 1)
			return -RTICE_ERR_INVALID_PARAM;
		funcId = cmd_data[0];
		paramLen = command_data_len - 1;
	#if IS_ENABLED(CONFIG_RTK_KDRV_PWM)
		ret = rwPanelTool(funcId, (char*)&cmd_data[1],paramLen, (char*)&response_data[1]);
		if(ret >= 0){
			response_data[0] = funcId;
			ret++;
		}
	#else
		ret = -RTICE_ERR_INVALID_PARAM;
	#endif
	}
	break;
	case RTICE_SYS_CMD_AUDIO_PARAM:
	{
		int dataLen = command_data_len - 6;
		int index = 0,size = 0,i = 0;
		int *v = NULL;
		int *pBuf = NULL;
		int pBuf2[16];
		ENUM_AUDIO_OPT_TYPE optType = ENUM_AUDIO_OPT_TYPE_SIZE;
		ENUM_RTICE_AUDIO_PARAM_GROUP audioGrp = ENUM_RTICE_AUDIO_PARAM_GROUP_SIZE;

		if(command_data_len < 2 || cmd_data[0] >= (int)ENUM_AUDIO_OPT_TYPE_SIZE || cmd_data[1] >= (int)ENUM_RTICE_AUDIO_PARAM_GROUP_SIZE)
			return -RTICE_ERR_INVALID_PARAM;
		optType = (ENUM_AUDIO_OPT_TYPE)cmd_data[0];
		audioGrp = (ENUM_RTICE_AUDIO_PARAM_GROUP)cmd_data[1];

		if(dataLen < 0 || dataLen % 4 != 0)
			return -RTICE_ERR_INVALID_PARAM;
		index = B4TL(cmd_data[2],cmd_data[3],cmd_data[4],cmd_data[5]);
		if(optType == otWrite){
			size = dataLen;
			if((size / 4) < 16){
				v = pBuf2;
			}else{
				pBuf = (int*)kmalloc((size / 4 + 1) * sizeof(int),GFP_ATOMIC);
				v = pBuf;
			}
			for(i = 0;i < size;i += 4)
				v[i / 4] = B4TL(cmd_data[6 + i],cmd_data[7 + i],cmd_data[8 + i],cmd_data[9 + i]);
			ret = setRticeAudioParam(audioGrp,index,v,size);
			if(ret > 0)
				ret = 0;
		}else if(optType == otRead){
			if(dataLen == 4)
				size = B4TL(cmd_data[6],cmd_data[7],cmd_data[8],cmd_data[9]);
			else
				size = 4;
			if((size / 4) < 16){
				v = pBuf2;
			}else{
				pBuf = (int*)kmalloc((size / 4 + 1) * sizeof(int),GFP_ATOMIC);
				v = pBuf;
			}
			ret = rticeAudioParam(audioGrp,index,v,size);
			for(i = 0;i < ret;i += 4)
				LTB4(v[i / 4],response_data[0 + i],response_data[1 + i],response_data[2 + i],response_data[3 + i]);
		}
		if(pBuf){
			kfree(pBuf);
			pBuf = NULL;
		}
	}
	break;
	default:
		ret = -RTICE_ERR_UNSUPPORTED_OP_CODE;
	}
	return ret;
}

/*------------------------------------------------------------------
 * Func : rtice_i2c_command_probe
 *
 * Desc : proc i2c command
 *
 * Parm : opcode
 *
 * Retn :
 *------------------------------------------------------------------*/
int rtice_system_command_probe(unsigned char op_code)
{
	switch (op_code)
	{
	case RTICE_SYS_CMD_READ_SYSTEM_INFO:
	case RTICE_SYS_CMD_READ_PCB_PARAMETER:
	case RTICE_SYS_CMD_SET_BUSERR_IGNORE_ADDR:
	case RTICE_SYS_CMD_FS_OPT:
	case RTICE_SYS_CMD_PWM_PANEL:
	case RTICE_SYS_CMD_AUDIO_PARAM:
		return 1;
	}
	return 0;
}

rtice_command_handler_t system_cmd_handler[] =  \
{
	{
		RTICE_CMD_GROUP_ID_SYSTEM, 
		rtice_system_command_probe,
		rtice_system_command_handler
	},
};

/*------------------------------------------------------------------
 * Func : rtice_system_tool_init
 *
 * Desc : register rtice io/mem tool
 *
 * Parm : N/A
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
int rtice_system_tool_init(void)
{
	rtice_register_command_handler(system_cmd_handler,\
		sizeof(system_cmd_handler) /sizeof(rtice_command_handler_t));
	return 0;
}

/*------------------------------------------------------------------
 * Func : rtice_system_tool_exit
 *
 * Desc : exit rtice io/mem tool
 *
 * Parm : N/A
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
void rtice_system_tool_exit(void)
{
	rtice_unregister_command_handler(system_cmd_handler,\
		sizeof(system_cmd_handler) /sizeof(rtice_command_handler_t));
	rtd_pr_rtice_info("rtice_unregister_command_handler() has been executed.\n");
}

MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);
