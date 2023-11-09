#include <linux/init.h>

#include <io.h>
#include <linux/delay.h>

#include <rtk_kdriver/pcbMgr.h>
#include <tvscalercontrol/scaler/scalerstruct.h>

#include <mach/platform.h>
#include <mach/rtk_platform.h>


#include <tvscalercontrol/edptx/ScalerDpCommon.h>
#include <tvscalercontrol/edptx/edptx.h>
#include <tvscalercontrol/panel/panel.h>
#include <tvscalercontrol/panel/panelapi.h>

//****************************************************************************
// Macro/Definition
//****************************************************************************


//****************************************************************************
// CODE TABLES
//****************************************************************************


//****************************************************************************
// VARIABLE DECLARATIONS
//****************************************************************************
//extern unsigned char getVscInitdoneFlag(void);
EnumPanelPowerSeq targerPanelPowerSeq=0xFF;
extern int eDpTx_set_newOutPort(EnumDisplayDpTxOutputPort OutputPort);
extern int eDpTx_set_newTimingMode(edptx_timing_type timing_type);

extern unsigned char ScalerDisplayGetAsyncFifoEnable(void);
extern int UserCommonPowerPanel_Init(EnumPanelPowerSeq pwrStatus);
extern unsigned char ScalerDisplayDpTxCheckHPDStatus(EnumDisplayDpTxOutputPort outputPort);
extern EnumDisplayDpTxOutputPort eDpTx_get_outPort(void);
extern EnumDisplayDpTxOutputPort eDpTx_get_newOutPort(void);
extern edptx_timing_type eDpTx_get_currentTimingMode(void);
extern edptx_timing_type eDpTx_get_newTimingMode(void);
extern unsigned char ScalerDisplayDpTxCheckFifoErrorStatus(EnumDisplayDpTxOutputPort outputPort);

//****************************************************************************
// FUNCTION DEFINITIONS
//****************************************************************************
extern int eDpTx_set_currentTimingMode(void);
extern int eDpTx_set_outPort(void);
extern void UserCommonPowerPanelAction(EnumDisplayDpTxOutputPort out_port, EnumPanelPowerSeq enumEvent);

//****************************************************************************
// VARIABLE IMPLEMENTATION
//****************************************************************************
EnumPanelPowerSeq EDPTX_state_get_targetPanelPwrSeq(void)
{
    return targerPanelPowerSeq;
}


int EDPTX_state_set_targetPanelPwrSeq(EnumPanelPowerSeq state)
{
    if(state >= _PANEL_POWER_SEQ_CMD_NUM){
        ErrorMessageEDPTx("[EDPTX] INVALID PowerSeq[%d]@%s\n", state, __FUNCTION__);
        return -1;
    }

    if(targerPanelPowerSeq != state){
        ErrorMessageEDPTx("[EDPTX] Set targerPanelPowerSeq=%d->%d\n", targerPanelPowerSeq, state);
        targerPanelPowerSeq = state;
    }

    return 0;
}



int EDPTX_state_handler(void *data)
{
    //unsigned int waitCnt;
    EnumPanelPowerSeq curPanelPowerSeq=0xFF;
    EnumPanelPowerSeq initPowerSeq=_PANEL_OFF;
    unsigned int dbgLoopCnt=0;
    unsigned char preOutPort=0, preTargetPwrSeq=0, preTimeMode=0;
    unsigned char timeMode;
    unsigned char outPort;

    InfoMessageEDPTx("[EDPTX] Handler start...@%s\n", __FUNCTION__);

    // wait VSC init done
#if 0
    waitCnt = 0;
    while(getVscInitdoneFlag() == 0) {
        mdelay(50);
        if((waitCnt++ % 10) == 0){
            NoteMessageEDPTx("[EDPTX] Wait VSC Init[%d]...\n", waitCnt);
        }
    }
#endif
    // panel init
    if((Get_DISPLAY_PANEL_TYPE() == P_EDP_4K2K) || (Get_DISPLAY_PANEL_TYPE() == P_EDP_4K2K_2PORT)){
        outPort = Get_DISPLAY_PANEL_CUSTOM_INDEX() < P_EDP_P2_4K2K_60_594M? _DPTX_TX0_PORT: _DPTX_TX2_PORT;
        timeMode = Get_DISPLAY_PANEL_CUSTOM_INDEX() - (outPort==_DPTX_TX2_PORT? P_EDP_P2_4K2K_60_594M: P_EDP_P0_4K2K_60_594M);

        //ROSPrintfInternal("[EDPTX] panel Type/Index%d/%d\n", Get_DISPLAY_PANEL_TYPE(), Get_DISPLAY_PANEL_CUSTOM_INDEX());
        //ROSPrintfInternal("[EDPTX] TimeMode/En=%d/%d@Port[%d]\n", timeMode, ScalerDisplayGetAsyncFifoEnable(), outPort);

        // eDP TX status int (port2, timing 4k2kp60@594M)
        eDpTx_set_newOutPort(outPort);
        eDpTx_set_newTimingMode(timeMode);

        if(ScalerDisplayGetAsyncFifoEnable()){
            eDpTx_set_currentTimingMode();
            eDpTx_set_outPort();
            initPowerSeq = _PANEL_ON;
            curPanelPowerSeq = _PANEL_ON;//let flow can enter check fifo status
            targerPanelPowerSeq = _PANEL_ON;
        }
    }

    UserCommonPowerPanel_Init(initPowerSeq);

    // state check & process
    while(1){
        if((targerPanelPowerSeq != curPanelPowerSeq)
                || ((eDpTx_get_currentTimingMode() != eDpTx_get_newTimingMode()) && (curPanelPowerSeq == _PANEL_ON))
                || ((eDpTx_get_outPort() != eDpTx_get_newOutPort()) && (curPanelPowerSeq == _PANEL_ON)))
        {
            if((eDpTx_get_newTimingMode() < EDPTX_TIMING_NUM) && (eDpTx_get_newOutPort() < _DISPLAY_DP_TX_PORT_MAX)){
                // panel state change to PANEL_OFF before change to _PANEL_ON state
                if((curPanelPowerSeq == _PANEL_ON)|| ScalerDisplayGetAsyncFifoEnable()){
                    UserCommonPowerPanelAction(preOutPort, _PANEL_OFF);
                    if(targerPanelPowerSeq == _PANEL_ON) // delay for panel state change
                        mdelay(3000);
                }
                eDpTx_set_currentTimingMode();
                eDpTx_set_outPort();
                UserCommonPowerPanelAction(eDpTx_get_outPort(), targerPanelPowerSeq);
                curPanelPowerSeq = targerPanelPowerSeq;
            }
        }

        mdelay(10);

        timeMode = eDpTx_get_currentTimingMode();
        outPort = eDpTx_get_outPort();

        // check HPD status
#if(_PANEL_DPTX_HPD_DETECT == _ON)
        ScalerDisplayDpTxCheckHPDStatus(outPort);
#endif
        // check eDp TX FIFO error status
        if((curPanelPowerSeq == _PANEL_ON)||(curPanelPowerSeq == _BACKLIGHT_ON))
        {            
            if(GET_PANEL_DISPLAY_DP_TX_PORT_0() == _ON)
            {
                ScalerDisplayDpTxCheckFifoErrorStatus(_DISPLAY_DP_TX_PORT_0);
            }
            
            if(GET_PANEL_DISPLAY_DP_TX_PORT_1() == _ON)
            {
                ScalerDisplayDpTxCheckFifoErrorStatus(_DISPLAY_DP_TX_PORT_1);
            }
            
            if(GET_PANEL_DISPLAY_DP_TX_PORT_2() == _ON)
            {
                ScalerDisplayDpTxCheckFifoErrorStatus(_DISPLAY_DP_TX_PORT_2);
            }
            
            if(GET_PANEL_DISPLAY_DP_TX_PORT_3() == _ON)
            {
                ScalerDisplayDpTxCheckFifoErrorStatus(_DISPLAY_DP_TX_PORT_3);
            }

        }
        if((preOutPort != outPort)|| (preTargetPwrSeq != targerPanelPowerSeq)|| (preTimeMode != timeMode)||(++dbgLoopCnt % 500 == 0)){
            if((preTargetPwrSeq != targerPanelPowerSeq)|| (preTimeMode != timeMode))
                ErrorMessageEDPTx("[EDPTX][%d]Port[%d], PwrSeq=%d/%d, timeMode=%d\n", dbgLoopCnt, outPort, targerPanelPowerSeq, curPanelPowerSeq, timeMode);
            else
                InfoMessageEDPTx("[EDPTX][%d]Port[%d], PwrSeq=%d/%d, timeMode=%d\n", dbgLoopCnt, outPort, targerPanelPowerSeq, curPanelPowerSeq, timeMode);
            preTargetPwrSeq = targerPanelPowerSeq;
            preOutPort = outPort;
            preTimeMode = timeMode;
        }
    }

    DebugMessageEDPTx("[EDPTX] Done!!%s\n", __FUNCTION__);
    return 0;
}

