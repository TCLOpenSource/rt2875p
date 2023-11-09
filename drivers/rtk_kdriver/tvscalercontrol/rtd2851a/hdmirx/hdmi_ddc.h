#ifndef __HDMI_DDC_H__
#define __HDMI_DDC_H__

#include <rtk_kdriver/rtk_ddc_lib.h>

extern void lib_hdmiddc_crt_on(unsigned char ch);
extern void lib_hdmiddc_sync_reset(unsigned char port);
extern unsigned char lib_hdmiddc_write(unsigned char port, unsigned char* EDID, unsigned int len);
extern unsigned char lib_hdmiddc_read(unsigned char port, unsigned char *edid_tbl, unsigned int len);
extern void lib_hdmiddc_enable(unsigned char port, unsigned char enable);
extern void lib_hdmiddc_set_sda_delay(unsigned char port, unsigned char delay_sel);
extern void lib_hdmiddc_sda2gpio(unsigned char port);
extern void lib_hdmiddc_gpio2sda(unsigned char port);
extern void lib_hdmiddc_sda_dir_out(unsigned char port, unsigned char dir_o);
extern void lib_hdmiddc_sync_reset(unsigned char port);

#endif //__HDMI_DDC_H__
