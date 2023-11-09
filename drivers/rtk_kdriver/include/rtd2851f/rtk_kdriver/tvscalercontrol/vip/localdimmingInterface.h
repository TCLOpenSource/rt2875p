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

typedef enum _SHIFT_DIR{
        LEFT = 0,
        RIGHT,
} _SHIFT_DIR ;

#define LDInterface_Table_Num 1
/*General Settings---From Register*/

typedef struct {
        UINT8  ld_mode;		   /*0:CMO Mode 1:LGD Mode 2:AS Mode 3:AS LED Mode*/
        UINT8  separate_two_block;
        UINT8  send_mode;/*change in K8: send mode:0 trigger mode;1:dvs mode;2:free run mode*/
	 UINT8  send_trigger;
        UINT8  out_data_align;	
	 UINT8   in_data_align;
        UINT8  as_led_with_cs;     /*asled if cs output*/
        UINT8  as_led_reset;       /*cs high/low*/
        UINT8  as_force_tx;        /*0:do not care  1:force to tx mode*/
        UINT8  w_package_repeater;
	 UINT32 vsync_delay;				// add by cathy
	 UINT32 ld_backlight_timing_delay;	//add by cathy, since eva write it in drver function, ask eva/hawaii if it is not neccessary
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
        UINT16 data_sram_separate; 	// 
        UINT16 ld_total_data_num;	//LD will total send number of data to ld_spi
} DRV_LDINTERFACE_Output;


typedef struct {
        UINT8 rx_units;
        UINT8 rx_data_format;  /*0: 8bit 1:16bit*/
        UINT8 time_out_period;
        UINT16 rx_data_shift;  /*rbus clk period, delay should be smaller than clock period*/
        UINT8 read_after_n_byte;
        UINT8 rw_cmd_byte;
        UINT8 sram_rwcmd_bit;
        UINT8 forceTxorRx_mode_en;
        UINT8 forceTxorRx_mode;
        UINT8 rw_bit_send_sel;
        UINT8 rw_cmd_sel;
        UINT8 rwcmd;

} DRV_LDINTERFACE_Read;


typedef struct {
        UINT8 shift_op;  /*0: only shift data part 1:shift whole sram*/
        UINT8 shift_dir; /*0: LEFT  1:RIGHT*/
        UINT8 shift_num; /*shift number of bit*/
        UINT8 swap_op;   /*0: only swap data part 1:swap whole sram*/
        UINT8 swap_en;   /*0: normal output       1: swap 2 bytes of the 16-bit data*/
} DRV_LDINTERFACE_SHIFT_SWAP;

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
        UINT8 spi_intven;/*For ld_spi_int send to dctl_int1, then send to vcpu*/
        UINT8 spi_intsen;/*For ld_spi_int send to dctl_int2, then send to scpu*/
        UINT8 rx_done_inten;
        UINT8 tx_done_inten;
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

typedef struct {
	
	unsigned short info[40];
} DRV_LDSPI_HeaderInfo;

typedef struct {
	unsigned short reserved[24];
} DRV_LDSPI_Reserved;

/*LDSPI Info*/
typedef struct {
	 DRV_LDSPI_HeaderInfo	  LDSPI_HeaderInfo;
	 UINT16 				LDvalue_init;	//initial backlight value
        DRV_LDSPI_Reserved Reserved;
        
} DRV_LDSPI_Info;

/*LDSPI Table INFO*/
typedef struct {
        DRV_LDINTERFACE_Basic_Ctrl   LDINTERFACE_Basic_Ctrl;
        DRV_LDINTERFACE_Output       LDINTERFACE_Output;
        DRV_LDINTERFACE_SHIFT_SWAP   LDINTERFACE_Shift_Swap;
        DRV_LDINTERFACE_INV          LDINTERFACE_Inv;
        DRV_LDINTERFACE_Repeat       LDINTERFACE_Repeat;
        DRV_LDINTERFACE_Reproduce    LDINTERFACE_Reproduce;
        DRV_LDINTERFACE_TimingReal   LDINTERFACE_TimingReal;
	 DRV_LDSPI_Info 				LDSPI_Info;
} DRV_LDINTERFACE_Table;

#endif	/* end LDSPI_Common */

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

/*Read Only:Status /Debug ---NOT in Table (rtd2851f[mac8p] is one port only)*/
typedef struct {
        UINT16 rx_done;
        UINT16 rx_time_out_pending;
        UINT16 tx_done;
        UINT16 cmd_error_status;
        UINT16 ld_timing_error;
        UINT16 ld_spi_timing_Error;
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
 * @brief set Ldspi enable
 */
void drvif_set_LdInterface_Enable(UINT8 enable);
/**
 * @brief ldspi start en/disable : for change settings need disable ldspi.
 */
void drvif_set_LdInterface_StartEn(UINT8 enable);
/**
 * @brief when trigger mode, send trigger
 */
void drvif_set_LdInterface_SendTrigger(void);

/**
 * @brief set Ldspi basic info
 */
void drvif_set_LdInterface_BasicCtrl(DRV_LDINTERFACE_Basic_Ctrl *ptr);
void drvif_set_LdInterface_LdMode(UINT8 ld_mode);
void drvif_set_LdInterface_Output(DRV_LDINTERFACE_Output *ptr);//input is real output unit
void drvif_set_LdInterface_DataUnits(UINT32 data_unit);//input is real output unit
/**
 * @brief Controls about SPI read. Default is TX :froce tx mode
 */
void drvif_set_LdInterface_shift_swap(DRV_LDINTERFACE_SHIFT_SWAP *ptr);

void drvif_set_LdInterface_Read(DRV_LDINTERFACE_Read *ptr);

void drvif_set_LdInterface_Inv(DRV_LDINTERFACE_INV *ptr);

void drvif_set_LdInterface_Repeat(DRV_LDINTERFACE_Repeat *ptr);

void drvif_set_LdInterface_Reproduce(DRV_LDINTERFACE_Reproduce *ptr);

void drvif_set_LdInterface_IntEn(DRV_LDINTERFACE_IntEn *ptr);

void drvif_set_LdInterface_TimingReal(DRV_LDINTERFACE_TimingReal *ptr);
void drvif_set_LdInterface_TimingReg(DRV_LDINTERFACE_TimingReg *ptrReg);


/**
 * @brief Clear rx fifo. Do rx clear after finish get rx datas.
 * include rx_done clear
 */
void drvif_LdInterface_Read_Clear(void);
/**
 * @brief Clear status.
 */
void drvif_LdInterface_Status_Clear(void);

/**
 * @brief print current info
 */
void   drvif_get_LdInterface_Table(DRV_LDINTERFACE_Table *ptr); //cal Reg 2 N0~N9
 /**
 * @brief when trigger mode, check if trigger cleared:
     * @return 1:enable 0:disable
 */
UINT8  drvif_get_LdInterface_Enable(void);
/**
 * @brief when trigger mode, check if trigger cleared:
     * @return 1:cleared 0:unClear
 */
UINT8  drvif_get_LdInterface_CheckTriggerClear(void);
/**
 * @brief: check rx done status: for recrive rx units
     * @return 1:rx_done   0:no rx received
 */
UINT8 drvif_get_LdInterface_rxdone(void);

/**
 * @brief get rx units:
     * @return rx unit infos
 */
UINT8*  drvif_get_LdInterface_RxUnits(int rxcnt);

void drvif_get_LdInterface_Output(DRV_LDINTERFACE_Output *ptrReg);
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

void drvif_color_set_LDInterface_SPI_IndexSRAM_Data_Continuous(DRV_LD_LDSPI_INDEXSRAM_TYPE *LDSPI_IndexSRAM, unsigned char Wait_Tx_Done);

void printTableInfo(DRV_LDINTERFACE_Table *ptr);


#ifdef __cplusplus
}
#endif

#endif /* __LOCALDIMMINGINTERFACE_H__*/



