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
                {1, 0, VSYNC_MODE, 0, 0, 0, 0, 1, 1, 0, 0 ,0},/*DRV_LDINTERFACE_Basic_Ctrl:\
                ld_mode, separate_two_block, send_mode(dvs/trigger), send_trigger, out_data_align, 
                in_data_align, as_led_with_cs, as_led_reset, as_force_tx, w_package_repeater, vsync_delay, ld_backlight_timing_delay*/
                {8, LD8BIT, 0,LD8BIT,0,LD8BIT,LD8BIT,DATSEP_DEFAULT, 160},/*DRV_LDINTERFACE_Output: \
                outputUnit(real unit),out_format,tailer_unit,tailer_format,header_unit,header_format,multiDelay_format,data_sram_separate, ld_total_data_num*/
                {0, LEFT, 0, 0, 0},        /*DRV_LDINTERFACE_Shift_Swap:   shift_op,shift_dir,shift_num,swap_op,swap_en*/
                {0, 0, 0, 0},              /*DRV_LDINTERFACE_Inv:   sdo,cs,sck,vsync*/
                {10, 0, 0},                /*DRV_LDINTERFACE_Repeat:vRepeat_width,vRepeat,hRepeat*/
                {0, 0, 0, 0, 0},           /*DRV_LDINTERFACE_Reproduce:reproduce_mode,dhTotal,dvTotal,src_sel,dvTotal_sync_dtg,*/
                {9584, 10000, 840, 420, 1900, 30208, 4812, 5188, 3000, 0},      /*DRV_LDSPI_TX_TimmingReal:\
                T1,T1_star,T2,T3,T4,T5,T6,T7,cs_end,fEachUnit*/
                {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, 100, 
                  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
                /* DRV_LDSPI_Info:  		header_info[40], initial_backlight_value, Reserved[24] */  
	},	
};
