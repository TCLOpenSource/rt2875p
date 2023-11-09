#ifndef __RTK_PWM_RTICE_H__
#define __RTK_PWM_RTICE_H__

typedef enum
{
    RTK_PWM_RTICE_GET_PWM = 0,
    RTK_PWM_RTICE_GET_DVS,
    RTK_PWM_RTICE_SET_PWM_DUTY,
    RTK_PWM_RTICE_SET_PWM_FREQ_W_DVS,
    RTK_PWM_RTICE_SET_PWM_FREQ_W_DATA,
    RTK_PWM_RTICE_SET_DVS,
    RTK_PWM_RTICE_STOP,
    RTK_PWM_RTICE_GET_PCB_PANEL,
} RTK_PWM_RTICE_FUN_ID;

int rwPanelTool(unsigned char functionId, char* pParam, int paramLen, char* pOutBuffer);

#endif
