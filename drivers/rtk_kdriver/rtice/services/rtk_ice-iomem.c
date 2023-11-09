#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/gfp.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/memblock.h>
#include <asm/page.h>
#include <rtk_kdriver/io.h>
#include "rtk_ice-iomem.h"
#ifdef CONFIG_RKDRV_MD
#include <rtk_kdriver/md/rtk_md.h>
#endif
#include <rtd_log/rtd_module_log.h>

extern unsigned long max_low_pfn;
struct task_struct *checkOverflow_tsk = NULL;
volatile unsigned char *backupRegAddr = NULL;
volatile unsigned char regNum = 0;
volatile static unsigned int *regValues = NULL;
volatile static unsigned int  timeCounter = 0;
volatile unsigned char fThreadStop = 0x00;
extern volatile unsigned char fDebugModeEnable;

#define SAFE_FREE(x) \
if(x){ \
	kfree((void*)(x)); \
	(x) = 0; \
}


/**after merlin2 , Rbus range extend to 0xb8200000 , old SOC range is narrow **/
#define REGISTER_L_BOUNDARY 0xb8000000
#define REGISTER_U_BOUNDARY 0xb8200000

#ifndef CONFIG_PAGE_OFFSET
#ifdef PAGE_OFFSET
#define CONFIG_PAGE_OFFSET PAGE_OFFSET
#endif
#endif
#ifdef CONFIG_PAGE_OFFSET
#define MEMORY_L_BOUNDARY CONFIG_PAGE_OFFSET
#define MEMORY_U_BOUNDARY (CONFIG_PAGE_OFFSET + (PFN_DOWN(memblock_end_of_DRAM()) << PAGE_SHIFT))
#else
#define MEMORY_L_BOUNDARY 0
#define MEMORY_U_BOUNDARY 0
#endif

int Rtice_ChecAddrValid(unsigned long addr)
{
	if (addr >= REGISTER_L_BOUNDARY && addr < REGISTER_U_BOUNDARY)
		return true;

	else if (addr >= MEMORY_L_BOUNDARY && addr < MEMORY_U_BOUNDARY)
		return true;
	return false;
}


/*------------------------------------------------------------------
 * Func : rtice_checkRegisterOverflow_task
 *
 * Desc : the thread function used to check register overflow
 *
 * Parm : arg_unused - unused
 *
 * Retn : 0 - success, others fail
 *------------------------------------------------------------------*/
static int rtice_checkRegisterOverflow_task(void *arg_unused)
{
	unsigned char i;
	unsigned int RegAddr_temp;
	unsigned int * regValues_temp = NULL;
	timeCounter = 0;
	fThreadStop = 0;

	SAFE_FREE(regValues);
	regValues = (unsigned int *) kmalloc(sizeof(*regValues)*regNum, GFP_ATOMIC);
	if (!regValues){
		return (0);
	}
	memset((void*)regValues,0,sizeof(*regValues)*regNum);
	for (i = 0; i < regNum; i++){
		RegAddr_temp = (((unsigned int) backupRegAddr[i * 4]) << 24) |\
				(((unsigned int) backupRegAddr[1 + i * 4]) << 16) |\
				(((unsigned int) backupRegAddr[2 + i * 4]) << 8) |\
				backupRegAddr[3 + i * 4];
		*(regValues + i) = rtd_inl(RegAddr_temp);
	}
	regValues_temp = (unsigned int *) kmalloc(sizeof(*regValues_temp) * regNum, \
								GFP_ATOMIC);
	if (!regValues_temp)
		goto thread_end;
	memset(regValues_temp,0,sizeof(*regValues_temp) * regNum);
	do{
		msleep(50);
		for (i = 0; i < regNum; i++){
			RegAddr_temp = (((unsigned int)backupRegAddr[i*4]) << 24) |\
				(((unsigned int) backupRegAddr[1 + i * 4]) << 16) |\
				(((unsigned int) backupRegAddr[2 + i * 4]) << 8) |\
				backupRegAddr[3 + i * 4];
			*(regValues_temp + i) = rtd_inl(RegAddr_temp);
		}
		for (i = 0; i < regNum; i++){
			if ((*(regValues_temp + i) == 0xFFFFFFFF) ||\
				(*(regValues_temp + i) < *(regValues + i))){
				kfree((void *)regValues_temp);
				regValues_temp = NULL;
				goto thread_end;
			}
		}
		for (i = 0; i < regNum; i++){
			*(regValues + i) = *(regValues_temp + i);
		}
		timeCounter++;
		if (timeCounter >= 2 * 60 * 1000 / 50){
			SAFE_FREE(regValues_temp);
			SAFE_FREE(regValues);
			return (0);
		}
		if (kthread_should_stop()){
			break;
		}
	} while (1);
thread_end:
	fThreadStop = 1;
	SAFE_FREE(regValues_temp);
	return (0);
}

/*------------------------------------------------------------------
 * Func : rtice_iomem_command_handler
 *
 * Desc : proc iomem command
 *
 * Parm : cmd_data : command data
 *        command_data_len  : command data len
 *
 * Retn :  < 0    : error
 *         others : length of response data
 *------------------------------------------------------------------*/
int rtice_iomem_command_handler(\
				unsigned char opcode, unsigned char *cmd_data,\
				unsigned int command_data_len,\
				unsigned char *response_data,\
				unsigned int response_buff_size )
{
	unsigned int AcceptedAddr;
	unsigned int GetData;
	unsigned char n_reg;
	int ret = 0;
	int i;

	switch (opcode){
	case RTICE_IOMEM_CMD_NORMAL_READ:
		if (command_data_len != 5)
			return -RTICE_ERR_INVALID_PARAM;
		ret = 0;
 		AcceptedAddr = B4TL(cmd_data[0], cmd_data[1], \
				     	cmd_data[2], cmd_data[3]);
		for (i = 0; i < cmd_data[4]; i++){
			if (!Rtice_ChecAddrValid(AcceptedAddr)){
#ifdef CONFIG_RKDRV_MD
				smd_memcpy(&GetData, AcceptedAddr, 4);
#else
				return -RTICE_IOMEM_ERR_INVALID_ADDRESS;
#endif
			}
			else{
				 /* this is for rbus timeout handler with respect to rtice tool Power-table function,
				  * because rbus timeout handler take special treatment for RTICE tool, with folloing protocol:
				  * 1. first, RTICE tool issues RTICE_SYS_CMD_SET_BUSERR_IGNORE_ADDR.
				  * 2. here to denote "rtice_iomem_read" global label for rbus timeout handler referenceing.
				  * 3. both above asserted, the rbus timeout will be ignored and return from exception.
				  */
#if !IS_ENABLED(CONFIG_RTK_KDRV_SB2)
				GetData = rtd_inl(AcceptedAddr);
#else //#if !defined(CONFIG_RTK_KDRV_SB2)
				{
					/* io.h:33: rtd_inl()::
					 static inline unsigned int rtd_inl(unsigned long addr)
					 {
						if (addr < (RBUS_BASE_VIRT_OLD+RBUS_BASE_SIZE) && addr >= RBUS_BASE_VIRT_OLD)
						return __raw_readl((void __iomem *)(addr-RBUS_BASE_VIRT_OLD+RBUS_BASE_VIRT));
						else
						return __raw_readl((void __iomem *)addr);
					 }
					*/
					unsigned long addr = (unsigned long)AcceptedAddr;
					unsigned long flag;

					if (addr < (RBUS_BASE_VIRT_OLD+RBUS_BASE_SIZE) && addr >= RBUS_BASE_VIRT_OLD)
						addr = (addr-RBUS_BASE_VIRT_OLD+(unsigned long)RBUS_BASE_VIRT);

					local_irq_save(flag);
					asm volatile (
							".global rtice_iomem_read ;"
							"rtice_iomem_read: ldr %w0, [%1] ;"
							"esb ; "
							/* "isb; dsb sy;" */
							: "=r"(GetData)
							: "r"(addr));
					local_irq_restore(flag);
				}
#endif //#if !defined(CONFIG_RTK_KDRV_SB2)
			}
			LTB4(GetData, response_data[0], response_data[1],\
					response_data[2], response_data[3]);
			AcceptedAddr += 4;
			response_data += 4;
			ret += 4;
		}
	break;
	case RTICE_IOMEM_CMD_NORMAL_WRITE:
		if (command_data_len != 8)
			return -RTICE_ERR_INVALID_PARAM;
		AcceptedAddr = B4TL(cmd_data[0], cmd_data[1], \
						cmd_data[2], cmd_data[3]);
		GetData = B4TL(cmd_data[4], cmd_data[5], \
						cmd_data[6], cmd_data[7]);
		if (!Rtice_ChecAddrValid(AcceptedAddr)){
#ifdef CONFIG_RKDRV_MD
			smd_memcpy(AcceptedAddr, &GetData, 4);
#else
			return -RTICE_IOMEM_ERR_INVALID_ADDRESS;
#endif
		}
        else{
			rtd_outl(AcceptedAddr, GetData);
		}
	break;
	case RTICE_IOMEM_CMD_DATA_PORT_WRITE:
		if (command_data_len < 6)
			return -RTICE_ERR_INVALID_PARAM;
		if (command_data_len != 6 + cmd_data[0])
			return -RTICE_ERR_INVALID_PARAM;
		AcceptedAddr = B4TL(cmd_data[1], cmd_data[2], \
						cmd_data[3], cmd_data[4]);
		if (((cmd_data[0] + 1) % 4))
			return -RTICE_IOMEM_ERR_INVALID_LENGTH;
		for (i = 0; i < cmd_data[0] + 1; i += 4){
			GetData = B4TL(cmd_data[5 + i], cmd_data[6 + i],\
					cmd_data[7 + i], cmd_data[8 + i]);

			if (!Rtice_ChecAddrValid(AcceptedAddr)){
#ifdef CONFIG_RKDRV_MD
				smd_memcpy(AcceptedAddr, &GetData, 4);
#else
				return -RTICE_IOMEM_ERR_INVALID_ADDRESS;
#endif
			}
			else{
				rtd_outl(AcceptedAddr, GetData);
			}
		}
	break;
	case RTICE_IOMEM_CMD_DATA_PORT_READ:
		if (command_data_len != 5)
			return -RTICE_ERR_INVALID_PARAM;
		AcceptedAddr = B4TL(cmd_data[1], cmd_data[2], \
						cmd_data[3], cmd_data[4]);
		if (!Rtice_ChecAddrValid(AcceptedAddr))
			return -RTICE_IOMEM_ERR_INVALID_ADDRESS;
		if (((cmd_data[0] + 1) % 4))
			return -RTICE_IOMEM_ERR_INVALID_LENGTH;
		for (i = 0; i < cmd_data[0] + 1; i += 4){
			if (!Rtice_ChecAddrValid(AcceptedAddr)){
#ifdef CONFIG_RKDRV_MD
				smd_memcpy(&GetData, AcceptedAddr, 4);
#else
				return -RTICE_IOMEM_ERR_INVALID_ADDRESS;
#endif
			}
			else{
				GetData = rtd_inl(AcceptedAddr);
			}
			LTB4(GetData, response_data[0], response_data[1],\
					response_data[2], response_data[3]);
			ret += 4;
			response_data += 4;
		}
	break;
	case RTICE_IOMEM_CMD_MULTI_READ:
		if (command_data_len < 5)
			return -RTICE_ERR_INVALID_PARAM;
		if (command_data_len != 1 + (4 * cmd_data[0]))
			return -RTICE_ERR_INVALID_PARAM;
		n_reg = cmd_data[0];
		cmd_data++;
		for (i = 0; i < n_reg; i++){
			AcceptedAddr = B4TL(cmd_data[0], cmd_data[1], \
						cmd_data[2], cmd_data[3]);
			if (!Rtice_ChecAddrValid(AcceptedAddr)){
#ifdef CONFIG_RKDRV_MD
				smd_memcpy(&GetData, AcceptedAddr, 4);
#else
				return -RTICE_IOMEM_ERR_INVALID_ADDRESS;
#endif
			}
			else{
				GetData = rtd_inl(AcceptedAddr);
			}
			LTB4(GetData, response_data[0], response_data[1],\
					response_data[2], response_data[3]);
			ret += 4;
			response_data += 4;
			cmd_data += 4;
		}
	break;
	case RTICE_IOMEM_CMD_MULTI_WRITE:
		if (command_data_len < 9)
			return -RTICE_ERR_INVALID_PARAM;
		if (command_data_len != 1 + (8 * cmd_data[0]))
			return -RTICE_ERR_INVALID_PARAM;
		n_reg = cmd_data[0];
		cmd_data++;
		for (i = 0; i < n_reg; i++){
			AcceptedAddr = B4TL(cmd_data[0], cmd_data[1], \
						cmd_data[2],cmd_data[3]);
			GetData = B4TL(cmd_data[4], cmd_data[5], \
						cmd_data[6],cmd_data[7]);
			cmd_data += 8;

			if (!Rtice_ChecAddrValid(AcceptedAddr)){
#ifdef CONFIG_RKDRV_MD
				smd_memcpy(AcceptedAddr, &GetData, 4);
#else
				return -RTICE_IOMEM_ERR_INVALID_ADDRESS;
#endif
			}
			else{
				rtd_outl(AcceptedAddr, GetData);
			}
		}
	break;
	case RTICE_IOMEM_CMD_START_BUS_MONITOR:
		if (command_data_len < 5)
			return -RTICE_ERR_INVALID_PARAM;
		if (command_data_len != 1 + (4 * cmd_data[0]))
			return -RTICE_ERR_INVALID_PARAM;
		regNum = cmd_data[0];
		if (backupRegAddr)
			return -RTICE_IOMEM_ERR_BUS_MONITOR_IS_RUNNING;
		SAFE_FREE(backupRegAddr);
		backupRegAddr = \
			(unsigned char *) kmalloc(sizeof(*backupRegAddr)*(regNum * 4),\
								GFP_ATOMIC);
		if (backupRegAddr == NULL)
			return \
		-RTICE_IOMEM_ERR_CREATE_BUS_MONITOR_REGISTER_TABLE_FAILED;
		for (i = 0; i < (regNum * 4); i++){
			if (i % 4 == 0){
				AcceptedAddr = B4TL(cmd_data[i + 1],\
							cmd_data[i + 2],\
							cmd_data[i + 3], \
							cmd_data[i + 4]);
				if (!Rtice_ChecAddrValid(AcceptedAddr)){
					SAFE_FREE(backupRegAddr);
 					return \
					-RTICE_IOMEM_ERR_INVALID_ADDRESS;
				}
			}
			*(backupRegAddr + i) = *(cmd_data + i + 1);
		}
		if (!checkOverflow_tsk)
			checkOverflow_tsk = kthread_create(\
					rtice_checkRegisterOverflow_task,\
					NULL, "checkOverflow");
		if (IS_ERR(checkOverflow_tsk)){
			rtd_pr_rtice_info("Unable to create thread checkOverflow.\n");
			checkOverflow_tsk = NULL;
			SAFE_FREE(backupRegAddr);
			return \
			-RTICE_IOMEM_ERR_CREATE_BUS_MONITOR_THREAD_FAILED;
		}
		wake_up_process(checkOverflow_tsk);
	break;
	case RTICE_IOMEM_CMD_STOP_BUS_MONITOR:
		if (command_data_len)
			return -RTICE_ERR_INVALID_PARAM;
		if (!backupRegAddr){
			return -RTICE_IOMEM_ERR_BUS_MONITOR_FAILED;
		}
		if (!checkOverflow_tsk){
			SAFE_FREE(backupRegAddr);
			return -RTICE_IOMEM_ERR_BUS_MONITOR_FAILED;
		}
		if (!regValues || !timeCounter){
			SAFE_FREE(backupRegAddr);
			SAFE_FREE(regValues);
			fThreadStop = 1;
			checkOverflow_tsk = NULL;
			return -RTICE_IOMEM_ERR_BUS_MONITOR_FAILED;
		}
		if (timeCounter >= 2 * 60 * 1000 / 50){
			SAFE_FREE(backupRegAddr);
			SAFE_FREE(regValues);
			fThreadStop = 1;
			checkOverflow_tsk = NULL;
			return -RTICE_IOMEM_ERR_CREATE_BUS_MONITOR_TIMEOUT;
		}
		if (fThreadStop)
			goto data_putchar;
		kthread_stop(checkOverflow_tsk);
		while (!fThreadStop)
			msleep(5);
	data_putchar:
		checkOverflow_tsk = NULL;
		SAFE_FREE(backupRegAddr);
		for (i = 0; i < regNum; i++){
			LTB4(*(regValues + i), \
				response_data[0],\
				response_data[1], \
				response_data[2],\
				response_data[3]);
			ret += 4;
			response_data += 4;
		}
		SAFE_FREE(regValues);
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
int rtice_iomem_command_probe(unsigned char op_code)
{
	return 1;
}

rtice_command_handler_t iomem_cmd_handler[] =  \
{
	{
		RTICE_CMD_GROUP_ID_IOMEM,
		rtice_iomem_command_probe,
		rtice_iomem_command_handler
	},
};

/*------------------------------------------------------------------
 * Func : rtice_iomem_tool_init
 *
 * Desc : register rtice io/mem tool
 *
 * Parm : N/A
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
int rtice_iomem_tool_init(void)
{
	rtice_register_command_handler(iomem_cmd_handler,\
		sizeof(iomem_cmd_handler) /sizeof(rtice_command_handler_t));
	return 0;
}

/*------------------------------------------------------------------
 * Func : rtice_iomem_tool_exit
 *
 * Desc : exit rtice io/mem tool
 *
 * Parm : N/A
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
void rtice_iomem_tool_exit(void)
{
	rtice_unregister_command_handler(iomem_cmd_handler,\
		sizeof(iomem_cmd_handler) / sizeof(rtice_command_handler_t));
	rtd_pr_rtice_info("rtice_unregister_command_handler() has been executed.\n");
	/*free some buffer*/
	SAFE_FREE(backupRegAddr);
	SAFE_FREE(regValues);
}

