#include <linux/init.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <rbus/stb_reg.h>

#include "rtd_types.h"
#include <tvscalercontrol/hdmitx/hdmitx.h>
#include <tvscalercontrol/hdmitx/hdmitx_lib.h>

int tx_timing_change(int argc, char **argv){
    int select = 0;
    //char userinput = '\0';
    long result = -1;

    showTimingList();

    if(result == -1) {
        ErrorMessageHDMITx("[TX_TIMING] command: SYSTEM2_RPC_SCPU_HDMITX_SHOWLIST, RPC  fail ...\n");
        return -1;
    }
    else
        NoteMessageHDMITx("[TX_TIMING] command: SYSTEM2_RPC_SCPU_HDMITX_SHOWLIST, RPC  success ...\n");

    #if 0 // [MARK2] FIX-ME
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
    #endif

    if( /*select < TX_TIMING_NUM &&*/ select >=0 )
    {

        setInputTimingType(select, 0);
    }
    else
    {
        ErrorMessageHDMITx("\n [ERR] wrong input! \r\n");
        return -1;
    }

    return 0;
}


int tx_timing_init(void) {

    int timing_type = 1;
    NoteMessageHDMITx("\r\n*******************tx_timing_init*******************\r\n");

    switch (0/*CONFIG_HDMITX_OUTPUT_DEFAULT_TIMING*/)
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

    while((getVscInitdoneFlag() == 0) && (ScalerHdmiTxGetTxRunFlag() == 0)
#ifdef _MARK2_ZEBU_BRING_UP_LOCAL_RUN // [MARK2] FIX-ME -- only for local verify
        && (kernel_finished() == 0)
#endif
        )
    {
        ROSTimeDly(50);
    } //while()

    setInputTimingType(timing_type, 0);

    return 0;
}

