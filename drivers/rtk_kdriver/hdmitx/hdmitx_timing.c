#include <init.h>
#include "ros_command.h"
#include <rtk_io.h>
#include <mm/mm.h>
#include <scaler/scaler_types.h>
#include <rpc/rpcdriver.h>
#include <tvscalercontrol/hdmitx/hdmitx.h>
#include <fs.h>



#define ErrorMessageHDMITx(fmt, ...) ROS_Printf_Module(LOGLEVEL_ERR, RTD_LOG_MODULE_SCPU1_SCALER, fmt, ##__VA_ARGS__)
#define NoteMessageHDMITx(fmt, ...) ROS_Printf_Module(LOGLEVEL_NOTICE, RTD_LOG_MODULE_SCPU1_SCALER, fmt, ##__VA_ARGS__)
#define DebugMessageHDMITx(fmt, ...)  ROS_Printf_Module(LOGLEVEL_DEBUG, RTD_LOG_MODULE_SCPU1_SCALER, fmt, ##__VA_ARGS__) // (x,y...) //

#ifdef CONFIG_RTK_KDRV_SPINAND
#ifdef CONFIG_SHELL_CMD_LINE
#ifdef CONFIG_RTK_SECURE_FS
extern int char2int(char a);

#ifndef CONFIG_OTHER_TEST_DBG_CMD_DISABLE
int tx_timing_change(int argc, char **argv){
    int select = 0;
    char userinput = '\0';
    long result = -1;
    SYSTEM_RPC_KERNEL_PARAMETER parameter;

    parameter.command = SYSTEM2_RPC_SCPU_HDMITX_SHOWLIST;
    parameter.param1 = 0;
    parameter.param2 = 0;

#if defined(CONFIG_RTK_H5X_RPC)
    result = sendKernelRPCtoSCPU1(parameter, BLOCK_MODE);
#endif

    if(result == -1) {
        ErrorMessageHDMITx("[TX_TIMING] command: SYSTEM2_RPC_SCPU_HDMITX_SHOWLIST, RPC  fail ...\n");
        return -1;
    }
    else
        NoteMessageHDMITx("[TX_TIMING] command: SYSTEM2_RPC_SCPU_HDMITX_SHOWLIST, RPC  success ...\n");


    while(1){

        userinput = shell_getc();

        if(userinput == '\r' || userinput == '\n')
            break;

        if(userinput != '\0') {
            shell_printf(1, "%c", userinput);
            select *= 10;
            select += char2int(userinput);
        }

        msleep(100);
    }

    if( /*select < TX_TIMING_NUM &&*/ select >=0 )
    {
        parameter.command = SYSTEM2_RPC_SCPU_HDMITX_TIMING;
        parameter.param1 = (unsigned long)select;
        parameter.param2 =  0;
#if defined(CONFIG_RTK_H5X_RPC)
        result = sendKernelRPCtoSCPU1(parameter, NONBLOCK_MODE);
#endif

        if(result == -1) {
            ErrorMessageHDMITx("[TX_TIMING] command: SYSTEM2_RPC_SCPU_HDMITX_TIMING, RPC  fail ...\n");
            return -1;
        }
        else
            NoteMessageHDMITx("[TX_TIMING] command: SYSTEM2_RPC_SCPU_HDMITX_TIMING, RPC  success ...\n");
    }
    else
    {
        ErrorMessageHDMITx("\n [ERR] wrong input! \r\n");
        return -1;
    }

    return 0;
}

SHELL_ADD(timing, tx_timing_change);
#endif //#ifndef CONFIG_OTHER_TEST_DBG_CMD_DISABLE
#endif
#endif
#endif

#ifdef CONFIG_ROS_H5S2
#include "rlink_slave.h"
extern RLNK_STATUS  RLink_system_check_slave_ready (void);
#endif

int tx_timing_init(void) {

    int timing_type = 1;
#if defined(CONFIG_RTK_H5X_RPC)
    long result;
#endif //#if defined(CONFIG_RTK_H5X_RPC)
    SYSTEM_RPC_KERNEL_PARAMETER parameter;
    NoteMessageHDMITx("\r\n*******************tx_timing_init*******************\r\n");

    switch (CONFIG_HDMITX_OUTPUT_DEFAULT_TIMING)
    {
        case 0:
            timing_type = TX_TIMING_HDMI20_4K2KP60_YUV444_8BIT;
            break;
        case 1:
            timing_type = TX_TIMING_HDMI21_4K2KP60_YUV444_8BIT_6G4L;
            break;
        case 2:
            timing_type = TX_TIMING_HDMI21_8K4KP60_YUV420_8BIT_8G;
            break;
        case 3:
            timing_type = TX_TIMING_HDMI20_1080P60_YUV444_8BIT;
            break;
        default:
            timing_type = TX_TIMING_HDMI21_8K4KP60_YUV420_8BIT_8G;
            break;
    }
    NoteMessageHDMITx("[TX_TIMING] func:%s,timing_type = %d!\r\n", __FUNCTION__, timing_type);

	while((*((int *)RTARC_UNCACHED_ADDRESS(SYNC_FLAG_SCPU1)) == (0xffffffff))) {
		ROSTimeDly(50);
	} //while()
#ifdef CONFIG_ROS_H5S2
    while( (RLink_system_check_slave_ready() != RLNK_OK)){
        ROSTimeDly(50);
    }//while()
#endif
    //send rpc to scpu1, to set hdmi tx input timing
    parameter.command = SYSTEM2_RPC_SCPU_HDMITX_TIMING;
    parameter.param1 = (unsigned long)timing_type;
    parameter.param2 = 0;
#if defined(CONFIG_RTK_H5X_RPC)
    result = sendKernelRPCtoSCPU1(parameter, NONBLOCK_MODE);
    if(result == -1)
        ErrorMessageHDMITx("[TX_TIMING] send SCPU1 kernel RPC  fail ...\n");
    else
        NoteMessageHDMITx("[TX_TIMING] send SCPU1 kernel RPC  success ...\n");
#else
        NoteMessageHDMITx("[TX_TIMING] NO RPC supported  ...\n");
#endif

    return 0;
}

//late_initcall_sync(tx_timing_init);
defer_initcall(tx_timing_init);
