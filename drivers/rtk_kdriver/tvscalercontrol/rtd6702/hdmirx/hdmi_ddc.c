/*=============================================================
 * Copyright (c)      Realtek Semiconductor Corporation, 2016
 *
 * All rights reserved.
 *
 *============================================================*/

/*======================= Description ============================
 *
 * file: 		hdmi_ddc.c
 *
 * author: 	Ant Man, Scott Lang
 * date:
 * version: 	3.0
 *
 *============================================================*/

/*========================Header Files============================*/
#include "hdmi_common.h"
#include "hdmi_ddc.h"


/**********************************************************************************************
*
*	Marco or Definitions
*
**********************************************************************************************/


/**********************************************************************************************
*
*	Const Declarations
*
**********************************************************************************************/



/**********************************************************************************************
*
*	Funtion Declarations
*
**********************************************************************************************/




/**********************************************************************************************
*
*	Function Body
*
**********************************************************************************************/

void lib_hdmiddc_crt_on(unsigned char ch)
{
    rtk_ddc_lib_crt_on(ch);
    rtk_ddc_lib_sync_reset(ch);
}



void lib_hdmiddc_sync_reset(unsigned char port)
{
    rtk_hddc_sync_reset(port);
}



unsigned char lib_hdmiddc_write(unsigned char port, unsigned char* EDID, unsigned int len)
{
    int ret;
    ret  = rtk_hddc_set_write_debounce(port, 2);
    ret |= rtk_hddc_write_edid(port, EDID, len);
    return ret == 0? TRUE:FALSE;
}


unsigned char lib_hdmiddc_read(unsigned char port, unsigned char *edid_tbl, unsigned int len)
{
    return (rtk_hddc_read_edid(port, edid_tbl, len)==0)? TRUE :FALSE;
}


void lib_hdmiddc_enable(unsigned char port, unsigned char enable)
{
    rtk_hddc_enable(port, enable);
}


void lib_hdmiddc_set_sda_delay(unsigned char port, unsigned char delay_sel)
{
    rtk_hddc_lib_set_sda_delay(port, delay_sel);
}


void lib_hdmiddc_sda2gpio(unsigned char port)
{
//#ifndef ENABLE_FUNCTIONAL_TEST
//        //MT board use GPIO, disable it for safe sake
//	hdmi_mask(pinmux_reg[port], ~(0xF0000000), 0xF<<28);
//#endif
}

void lib_hdmiddc_gpio2sda(unsigned char port)
{
//#ifndef ENABLE_FUNCTIONAL_TEST
//        //MT board use GPIO, disable it for safe sake
//	hdmi_mask(pinmux_reg[port], ~(0xF0000000), 0x5<<28);
//#endif
}

void lib_hdmiddc_sda_dir_out(unsigned char port, unsigned char dir_o)
{
//#ifndef ENABLE_FUNCTIONAL_TEST
//	if (port >= HDMI_PORT_TOTAL_NUM)
//		return;
//	hdmi_mask(GPIO_DIR_REG, ~gpio_dir_bit[port], (dir_o)?gpio_dir_bit[port]:0);
//#endif
}

void lib_hdmiddc_get_ddc_state(unsigned char port, srtk_edid_sts *edid_sts)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_DDC)
    rtk_hddc_lib_get_ddc_state(port, edid_sts);
#endif
}