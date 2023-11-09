#include <tvscalercontrol/vip/localdimmingInterface.h>

#define DATSEP_DEFAULT  (0x400)
typedef enum _LDDATA_FORMAT{
        LD8BIT = 0,
        LD10BIT,
        LD16BIT,
        LD12BIT,
} _LDDATA_FORMAT ;
/*******************************************************************************
* Table
******************************************************************************/
DRV_LDINTERFACE_Table LDINTERFACE_Table[LDInterface_Table_Num] = {
/*table0:test*/
	{
                {1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0},/*DRV_LDINTERFACE_Basic_Ctrl:\
                ld_mode,sep_block,send_mode(trigger/dvs/freeRun),data_align,as_led_with_cs,as_led_reset,as_force_tx,w_packet_repeater,free_run_period*/
                {8, LD8BIT, 0,LD8BIT,0,LD8BIT,LD8BIT,0,DATSEP_DEFAULT, 0, 0},/*DRV_LDINTERFACE_Output: \
                outputUnit(real unit),out_format,tailer_unit,tailer_format,header_unit,header_format,multiDelay_format,multi_add,data_sram_separate, shift_dir, shift_num*/
                {0, 0, 0, 0},              /*DRV_LDINTERFACE_Inv:   sdo,cs,sck,vsync*/
                {10, 0, 0},                /*DRV_LDINTERFACE_Repeat:vrepeat_width,vrepeat,hrepeat*/
                {0, 0, 0, 0, 0},           /*DRV_LDINTERFACE_Reproduce:reproduce_mode,dhTotal,dvTotal,src_sel,dvTotal_sync_dtg,*/
                {9584, 10000, 840, 420, 1900, 30208, 4812, 5188, 0, 0},      /*DRV_LDSPI_TX_TimmingReal:\
                T1,T1_star,T2,T3,T4,T5,T6,T7,cs_end,fEachUnit*/
		   {
		       {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},0,
		       {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
		   } /*DRV_LDSPI_Info*/
	},	
};
