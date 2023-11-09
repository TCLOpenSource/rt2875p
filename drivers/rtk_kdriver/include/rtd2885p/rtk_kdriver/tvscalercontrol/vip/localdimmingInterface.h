/* 20201217 Local Dimming Interface header file*/
#ifndef __LOCALDIMMINGINTERFACE_H__
#define __LOCALDIMMINGINTERFACE_H__


#ifdef __cplusplus
extern "C" {
#endif
/* some include about scaler*/
#include <scaler/vipCommon.h>
#define DEBUG_LDINTERFACE 0
#include <base_types.h>

#ifndef LDSPI_Common
#define LDSPI_Common

typedef enum _SEND_MODE{
        VSYNC_MODE = 0,
        FREERUN_MODE_WAIT_LD,
        FREERUN_MODE_AUTO,
        TRIGGER_MODE ,
} _SEND_MODE ;

#define LDInterface_Table_Num 1

/*General Settings---From Register*/

typedef struct {
        UINT8  ld_mode;		   /*0:CMO Mode 1:LGD Mode 2:AS Mode 3:AS LED Mode*/
        UINT8  separate_block;
        UINT8  send_mode;/*change in K8: send mode:0 trigger mode;1:dvs mode;2:free run mode*/
	 UINT8  send_trigger;
        UINT8  out_data_align;		
	 UINT8  in_data_align;	
        UINT8  as_led_with_cs;     /*asled if cs output*/
        UINT8  as_led_reset;       /*cs high/low*/
        UINT8  as_force_tx;        /*0:do not care  1:force to tx mode*/
        UINT8  w_package_repeater; 
        UINT32 free_run_period;    /*when send mode is freeRun*/
	 UINT32 vsync_delay;
} DRV_LDINTERFACE_Basic_Ctrl;

typedef struct {
        UINT8 ld_spi_en;       /*ip enable*/
        UINT8 start_enable;    /*disable when change params*/
} DRV_LDINTERFACE_Enable;

typedef struct {
        UINT16 output_units;//real output unit,for register need -1
        UINT8 output_data_format;
        UINT8 tailer_units;
        UINT8 tailer_format;
        UINT8 header_units;
        UINT8 header_format;
        UINT8 delay_format; //for multi unit delay
        UINT8 multi_add; //merlin8:for RL6856-383 add multi add
        UINT16 data_sram_separate;
	 UINT8 shift_dir; 		/*0: LEFT  1:RIGHT*/
        UINT8 shift_num; 	/*shift number of bit*/	
} DRV_LDINTERFACE_Output;

typedef struct {
        UINT8 tx_sdo_inv;
        UINT8 tx_cs_inv;
        UINT8 tx_clk_inv;
        UINT8 tx_vsync_inv;
//        UINT8 tx_hsync_inv;		
} DRV_LDINTERFACE_INV;

typedef struct {
        UINT8 vrepeat_width;/*equal to H*/
        UINT8 vrepeat;
        UINT8 hrepeat;
} DRV_LDINTERFACE_Repeat;

typedef struct {
        UINT8 dv_reproduce_mode;
        UINT16 ld_spi_dh_total;
        UINT16 ld_spi_dv_total;
        UINT8 dv_total_src_sel;
        UINT16 dv_total_sync_dtg;
} DRV_LDINTERFACE_Reproduce;

typedef struct {
        UINT8 sck_spread_unit;
        UINT8 sck_spread_max;
        UINT8 sck_spread_step;
} DRV_LDINTERFACE_Spread;

typedef struct {
        UINT8 ld_spi_int1_en;/*For ld_spi_int send to dctl_int1, then send to vcpu*/
        UINT8 ld_spi_int2_en;/*For ld_spi_int send to dctl_int2, then send to scpu*/
        UINT8 tx_done_int_en;
        UINT8 sram_hw_write_done_int_en;
} DRV_LDINTERFACE_IntEn;

/*Time Info*/
typedef struct {
        UINT32 T1;
        UINT32 T1_star;
        UINT32 T2;
        UINT32 T3;
        UINT32 T4;
        UINT32 T5;
        UINT32 T6;
        UINT32 T7;
        UINT32 cs_end;
        UINT8  fMultiUnit;//0:Each Unit Delay  1:Multi Unit Delay
        //UINT8 unit;//0:ns 1:us 2:ms  fix ns
} DRV_LDINTERFACE_TimingReal;


/*APL_BPL Info*/
typedef struct {
    UINT8   apl_mode;    /*1.apl enable=>hw mode  2:disable=>sw mode*/
    UINT16  apl_average; /*use when sw mode write*/
    UINT16  apl_avg_num; /*real calcultae num*/
    UINT16  apl_spitotal;/*outputunit - header - tailer +1*/
    UINT8   bpl_mode;    /*1.bpl enable=>hw mode  2:disable=>sw mode*/
    UINT16  bpl_average; /*use when sw mode write*/
    UINT16  sram_hw_write_num_set;/*same as output numver*/

} DRV_LDINTERFACE_APLBPL;

/*MCU Driver for MBI/IWATT*/
typedef struct {
    UINT8   driver_test;

} DRV_LDINTERFACE_DRIVER;

/*MCU Driver for MBI/IWATT*/
typedef struct {
    UINT8   rx_test;

} DRV_LDINTERFACE_RXCTRL;

typedef struct {
	unsigned short info[40];
} DRV_LDSPI_HeaderInfo;

typedef struct {
	unsigned short reserved[24];
} DRV_LDSPI_Reserved;

/*LDSPI Info*/
typedef struct {
	 DRV_LDSPI_HeaderInfo	  	LDSPI_HeaderInfo;
	 UINT16 					LDvalue_init;	//initial backlight value
        DRV_LDSPI_Reserved 		Reserved;
        
} DRV_LDSPI_Info;

/*LDSPI Table INFO*/
typedef struct {
        DRV_LDINTERFACE_Basic_Ctrl   LDINTERFACE_Basic_Ctrl;
        DRV_LDINTERFACE_Output       LDINTERFACE_Output;
        DRV_LDINTERFACE_INV          LDINTERFACE_Inv;
        DRV_LDINTERFACE_Repeat       LDINTERFACE_Repeat;
        DRV_LDINTERFACE_Reproduce    LDINTERFACE_Reproduce;
        // DRV_LDINTERFACE_Spread       LDINTERFACE_Spread;
        // DRV_LDINTERFACE_IntEn        LDINTERFACE_IntEn;
        DRV_LDINTERFACE_TimingReal   LDINTERFACE_TimingReal;
        //DRV_LDINTERFACE_APLBPL       LDINTERFACE_APLBPL;
        //DRV_LDINTERFACE_DRIVER       LDINTERFACE_DRIVER;
        //DRV_LDINTERFACE_RXCTRL       LDINTERFACE_RXCTRL;
	 DRV_LDSPI_Info 				LDSPI_Info;
} DRV_LDINTERFACE_Table;
#endif

/*NOT in table*/
typedef struct {
        UINT32 send_trigger;
} DRV_LDINTERFACE_Trigger;

/*Use Calculate :NOT in table*/
typedef struct {
        UINT32 cs_hold_time;
        UINT32 sck_hold_time;
        UINT32 sck_h;
        UINT32 sck_l;
        UINT32 data_send_delay;
        UINT32 each_unit_delay;
        UINT32 multi_unit_delay;
        UINT16 vsync_d;
        UINT16 cs_end_delay_time;
} DRV_LDINTERFACE_TimingReg;

/*Read Only:Status /Debug ---NOT in Table*/
typedef struct {
        UINT16 tx_done;
        UINT16 tx_done1;
        UINT16 sram_hw_write_done;
        UINT16 cmd_error_status;
        UINT16 ld_timing_error;
        UINT16 ld_spi_timing_Error;
        UINT16 ld_spi_freerun_error;
        UINT16 apl_average;
        UINT16 bpl_result;
} DRV_LDINTERFACE_Status;

/*Sram RW Settings---NOT in Table*/
typedef struct {
        UINT16 data_rw_pointer;
        UINT16 data_rw_pointer_set ;
        UINT16 data_rw_data_port;
        UINT16 packet_in_hw_mask;
} DRV_LDINTERFACE_SRam_Debug;

/*Sram format ----NOT in table*/
/*sram: data part -->separate part: header&tailer*/
typedef struct {
        UINT16 h_region;
        UINT16 v_region;
} DRV_LDINTERFACE_SRam_SeperateFormat;

typedef struct {
	unsigned short SRAM_Position;
	unsigned short SRAM_Length;
	unsigned short *SRAM_Value;
} DRV_LD_LDSPI_INDEXSRAM_TYPE;

/**
 * @brief load all settings
 */

extern void drvif_LdInterface_Test(void);

    /**
     * @brief load all settings
     */
void drvif_set_LdInterface_Table(DRV_LDINTERFACE_Table *ptr);

    /**
     * @brief set Ldspi basic info
     */
void drvif_set_LdInterface_StartEn(UINT8 enable);
void drvif_set_LdInterface_Enable(UINT8 enable);
    /**
     * @brief when trigger mode, send trigger
     */
void drvif_set_LdInterface_SendTrigger(void);

void drvif_set_LdInterface_BasicCtrl(DRV_LDINTERFACE_Basic_Ctrl *ptr);
void drvif_set_LdInterface_LdMode(UINT8 ld_mode);

void drvif_set_LdInterface_Output(DRV_LDINTERFACE_Output *ptr);//input is real output unit
void drvif_set_LdInterface_DataUnits(UINT32 data_unit);//input is real output unit

void drvif_set_LdInterface_Inv(DRV_LDINTERFACE_INV *ptr);

void drvif_set_LdInterface_Repeat(DRV_LDINTERFACE_Repeat *ptr);

void drvif_set_LdInterface_Reproduce(DRV_LDINTERFACE_Reproduce *ptr);

void drvif_set_LdInterface_Spread(DRV_LDINTERFACE_Spread *ptr);

void drvif_set_LdInterface_IntEn(DRV_LDINTERFACE_IntEn *ptr);

void drvif_set_LdInterface_TimingReal(DRV_LDINTERFACE_TimingReal *ptr);
void drvif_set_LdInterface_TimingReg(DRV_LDINTERFACE_TimingReg *ptrReg);

void drvif_set_LdInterface_AplBpl(DRV_LDINTERFACE_APLBPL *ptr);

/*set apl_bpl table*/
void drvif_set_LdInterface_aplbplTable(UINT16 *inBplPtr);

    /**
     * @brief print current info
     */
void   drvif_get_LdInterface_Table(DRV_LDINTERFACE_Table *ptr); //cal Reg 2 N0~N9
     /**
     * @brief when trigger mode, check if trigger cleared:
         * @return 1:enable 0:disable
     */
void drvif_get_LdInterface_Output(DRV_LDINTERFACE_Output *ptrReg);
UINT8  drvif_get_LdInterface_Enable(void);
    /**
     * @brief when trigger mode, check if trigger cleared:
         * @return 1:cleared 0:unClear
     */
UINT8  drvif_get_LdInterface_CheckTriggerClear(void);

void drvif_get_LdInterface_TimingReg(DRV_LDINTERFACE_TimingReg *ptrReg);
void drvif_get_LdInterface_TimingReal(DRV_LDINTERFACE_TimingReal *ptrReal);

/*Calculate Part: Timing*/
void drvif_cal_LdInterface_TimingReg( DRV_LDINTERFACE_TimingReal *ptrReal,DRV_LDINTERFACE_TimingReg *ptrReg);//test ok
void drvif_cal_LdInterface_TimingReal(DRV_LDINTERFACE_TimingReal *ptrReal,DRV_LDINTERFACE_TimingReg *ptrReg);

/*Read Only: Status*/
/**
 * @brief get tx status:
 * @param :status info stored in *ptr
 */
void drvif_get_LdInterface_Status(DRV_LDINTERFACE_Status *ptr);

void drvif_get_LdInterface_Status(DRV_LDINTERFACE_Status *ptr);

void drvif_color_set_LDInterface_SPI_IndexSRAM_Data_Continuous(DRV_LD_LDSPI_INDEXSRAM_TYPE *LDSPI_IndexSRAM, unsigned char Wait_Tx_Done);

void drvif_color_set_LDSPI_TXDone_ISR_En(unsigned char en);

void printTableInfo(DRV_LDINTERFACE_Table *ptr);

#ifdef __cplusplus
}
#endif

#endif /* __LOCALDIMMINGINTERFACE_H__*/



