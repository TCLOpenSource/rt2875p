#ifndef SCALER_VERIFY_H
#define SCALER_VERIFY_H

#include <linux/types.h>
#include <rtd_types.h>

/**
 * Define 3D Output Freq.
 */
typedef enum
{
	scaler_verify_mode_off = 0,
	scaler_capture_verify_mode,
	scaler_smoothtoggle_verifymode,
	scaler_game_mode_verify_mode,
} scaler_verify_mode;

void set_verify_mode(unsigned char verifymode);//set verify mode. verify_mode_off means disable
unsigned char get_verify_mode(void);//get verify mode. verify_mode_off means disable
unsigned char check_verify_mode_enable(void);//check current is verify mode enable or not

void set_scaler_verify_mode(unsigned char verifymode);//set verify mode. verify_mode_off means disable
unsigned char get_scaler_verify_mode(void);//get verify mode. verify_mode_off means disable
unsigned char check_verify_mode_enable(void);//check current is verify mode enable or not
void init_scaler_verifymode_semaphore(void);//initial scaler verify mode ctrl related semaphore
void create_scaler_verify_tsk(void);//creat scaler verify task

unsigned int get_adc_ptg_table_size(void);//get current verify_PTG_TABLE size
ModeTableType *get_adc_ptg_table_from_index(unsigned short index);//get the table address form verify_PTG_TABLE
unsigned char iptg_driver_setting(unsigned short ptg_timing);//set adc PTG driver and adc structure


/*Verify mode function*/
unsigned int run_capture_verify_mode(void);//run capture verify mode

#endif
