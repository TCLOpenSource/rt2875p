#ifndef __SCALER_VPQLEDDEV_H
#define  __SCALER_VPQLEDDEV_H

#include <scaler/vipCommon.h>
#include <scaler/scalerDrvCommon.h>

typedef enum {
	PQ_LED_DEV_NOTHING = 0,
	PQ_LED_DEV_INIT_DONE,
	PQ_LED_DEV_UNINIT,
} PQ_LED_DEV_STATUS;

typedef enum {
     LOCALDIMMING_DEMOTYPE_LINEDEMO = 0,
     LOCALDIMMING_DEMOTYPE_LEFTRIGHT,
     LOCALDIMMING_DEMOTYPE_TOPBOTTOM,
     LOCALDIMMING_DEMOTYPE_SIZE_MAX
} LOCALDIMMNG_DEMO_TYPE_T;

typedef struct {
	unsigned short SRAM_Position;
	unsigned short SRAM_Length;
	unsigned short SRAM_Value[1024];	
} VPQLED_LD_LDSPI_DATASRAM_TYPE;


void vpqled_do_resume(void);

unsigned char vpq_led_ioctl_get_stop_run(unsigned int cmd);
unsigned char vpq_led_ioctl_get_stop_run_by_idx(unsigned char cmd_idx);
void vpq_led_ioctl_set_stop_run_by_idx(unsigned char cmd_idx, unsigned char stop);

void vpqled_HAL_VPQ_LED_LDCtrlSPI(unsigned char *LDCtrlSPI);
unsigned char vpqled_get_LD_GetAPL_TV006(UINT16* BLValue);
void vpqled_HAL_VPQ_LED_LDCtrlDemoMode(LOCALDIMMNG_DEMO_TYPE_T bType, BOOLEAN bCtrl);

//variable set get function
unsigned char Get_Var_PQ_LED_Dev_Status(void);
void Set_Var_PQ_LED_Dev_Status(unsigned char value);
void Set_Val_vpq_vpq_led_LDEnable(unsigned char value);

#endif
