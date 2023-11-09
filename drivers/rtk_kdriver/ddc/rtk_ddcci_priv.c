#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <rtk_kdriver/io.h>
#include <linux/interrupt.h>
#include "rtk_ddc_dbg.h"
#include "rtk_ddcci_priv.h"

#ifdef CONFIG_ARCH_RTK2851C
#include "rtk_ddcci_config-rtk2851c.h"
#endif

#ifdef CONFIG_ARCH_RTK2851F
#include "rtk_ddcci_config-rtk2851f.h"
#endif

#ifdef CONFIG_ARCH_RTK2885P
#include "rtk_ddcci_config-rtk2885p.h"
#endif

#include <linux/kthread.h>

#ifdef CONFIG_RTK_KDRV_DDCCI_HID
//DDCHID=================================================================
typedef int (*DDC_HID_CALLBACK)(unsigned int seq_num, unsigned char *buf, unsigned int len);
static DDC_HID_CALLBACK g_ddc_hid_callback = NULL;
bool rtk_ddcci_register_hid_callback(DDC_HID_CALLBACK callback)
{
	if(!g_ddc_hid_callback)
		g_ddc_hid_callback = callback;
	return true;
}
EXPORT_SYMBOL(rtk_ddcci_register_hid_callback);

bool rtk_ddcci_unregister_hid_callback(DDC_HID_CALLBACK callback)
{
	if(g_ddc_hid_callback == callback)
		g_ddc_hid_callback = NULL;
	return true;
}
EXPORT_SYMBOL(rtk_ddcci_unregister_hid_callback);

void rtk_ddcci_tx_fifo_reset(unsigned char id);
void rtk_ddcci_rw_stop_clear(unsigned char id);
int _rtk_ddcci_write_data_to_tx_fifo(unsigned char id, unsigned char* tx_buff, unsigned char tx_buff_len);

static bool rtk_ddcci_hid_process(unsigned int id, unsigned char *buf, unsigned int len)
{
	if(buf[0] == 0xA5 && (buf[1] & 0xF0) == 0x50) {
		unsigned char response[10] = {0};
		unsigned int response_len;
		if(!g_ddc_hid_callback || 
			g_ddc_hid_callback(buf[1] & 0x7, response, 10) <= 0) {
			response[0] = ((buf[1] & 0x7) << 1);
			response_len = 10;
		} else {
			response_len = 10;
		}
		rtk_ddcci_tx_fifo_reset(id);
		if(response_len != 0)
			_rtk_ddcci_write_data_to_tx_fifo(id, response, response_len);
		return true;
	} else {
		return false;
	}
	
}

#endif
//DDCCI=====================================================================================================

unsigned char g_ddcci_connect_flag = 0;

int _rtk_ddcci_write_data_to_tx_fifo(unsigned char id, unsigned char* tx_buff, unsigned char tx_buff_len)
{
    const srtk_ddcci_reg_map *preg_map = rtk_ddcci_chip[id].reg_remap;
    int index;

    if(rtk_ddcci_chip[id].reg_remap == NULL) {
        RTK_DDC_INFO("[write_data_to_tx_fifo], Id = %d doesn't exist or not creating.\n", id);
        return -EINVAL;
    }

    //RTK_DDC_INFO("[ddcci_tx_fifo]%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,\n", tx_buff[0],tx_buff[1],tx_buff[2],tx_buff[3],tx_buff[4],tx_buff[5],tx_buff[6],tx_buff[7], tx_buff[8],tx_buff[9],tx_buff[10],tx_buff[11],tx_buff[12],tx_buff[13],tx_buff[14],tx_buff[15]);	

    for (index = 0; index < tx_buff_len; index++) {
        rtd_outl(preg_map->DDC_DDC_SFDR, DDC_DDC_SFDR_CIIDDP(tx_buff[index]));
    }

    return tx_buff_len;
}

void rtk_ddcci_lib_init(unsigned char id)
{
    const srtk_ddcci_reg_map *preg_map = rtk_ddcci_chip[id].reg_remap;

    RTK_DDC_INFO("[rtk_ddcci_lib_init] id=%d",id);
    if(rtk_ddcci_chip[id].reg_remap == NULL) {
        RTK_DDC_INFO("[rtk_ddcci_lib_init], Id = %d doesn't exist or not creating.\n", id);
        return;
    }
    g_ddcci_connect_flag = id;

    // init DDC FIFO
    rtd_maskl(preg_map->DDC_DDC_SFCR, ~(DDC_DDC_SFCR_DOFFPT_RESET_MASK), DDC_DDC_SFCR_DOFFPT_RESET(1));
    rtd_maskl(preg_map->DDC_DDC_SFCR, ~(DDC_DDC_SFCR_DIFFPT_RESET_MASK), DDC_DDC_SFCR_DIFFPT_RESET(1));

    rtd_maskl(preg_map->DDC_DDC_SFCR, ~(DDC_DDC_SFCR_DOFFPT_RESET_MASK), DDC_DDC_SFCR_DOFFPT_RESET(0));
    rtd_maskl(preg_map->DDC_DDC_SFCR, ~(DDC_DDC_SFCR_DIFFPT_RESET_MASK), DDC_DDC_SFCR_DIFFPT_RESET(0));

    //DDC1_DDC_SFCR: [3]Special mode [2:1]Buf Reset [0]Enable
    rtd_maskl(preg_map->DDC_DDC_SFCR, ~(DDC_DDC_SFCR_CIID_EN_MASK), DDC_DDC_SFCR_CIID_EN(1));

    //DDC1_DDC_SFSAR: Set CIID Slave Address(here:0x6e)
    rtd_maskl(preg_map->DDC_DDC_SFSAR, ~(DDC_DDC_SFSAR_CIID_ADDRESS_MASK), DDC_DDC_SFSAR_CIID_ADDRESS(0x37));
#ifndef CONFIG_RTK_KDRV_DDCCI_USING_POLLING
    //enable DDCCI Interrupt
    rtd_maskl(preg_map->DDC_DDC_SFIR, ~(DDC_DDC_SFIR_CIPPT_IE_MASK), DDC_DDC_SFIR_CIPPT_IE(1));
#endif
}

// Data In
int rtk_ddcci_data_in_is_emply(unsigned char id)
{
    const srtk_ddcci_reg_map *preg_map = rtk_ddcci_chip[id].reg_remap;
    int ret = 1;

    rtd_maskl(preg_map->DDC_DDC_SFSR, ~DDC_DDC_SFSR_FFDINE_ST_MASK, DDC_DDC_SFSR_FFDINE_ST(1));
    ret = DDC_DDC_SFSR_GET_FFDINE_ST(rtd_inl(preg_map->DDC_DDC_SFSR));

    return ret;
}

void rtk_ddcci_read_rx_fifo_data(unsigned char id, unsigned char* data_buff, int buff_len)
{
    int index = 0;

    const srtk_ddcci_reg_map *preg_map = rtk_ddcci_chip[id].reg_remap;

    for(index = 0; index < buff_len; index++)
    {
        data_buff[index] = (unsigned char)DDC_DDC_SFDR_GET_CIIDDP(rtd_inl(preg_map->DDC_DDC_SFDR));
    }
    //RTK_DDC_INFO("[ddcci_rx_fifo] len=%d. %2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,\n", buff_len,data_buff[0],data_buff[1],data_buff[2],data_buff[3],data_buff[4],data_buff[5],data_buff[6],data_buff[7],data_buff[8],data_buff[9],data_buff[10],data_buff[11],data_buff[12],data_buff[13],data_buff[14],data_buff[15]);		
}

int rtk_ddcci_get_rx_fifo_length(unsigned char id)
{
    const srtk_ddcci_reg_map *preg_map = rtk_ddcci_chip[id].reg_remap;
    int buff_len = 0;

    buff_len = DDC_DDC_SFIBLR_GET_FFDI_DLEN(rtd_inl(preg_map->DDC_DDC_SFIBLR));

    return buff_len;
}

void rtk_ddcci_rx_fifo_reset(unsigned char id)
{
    const srtk_ddcci_reg_map *preg_map = rtk_ddcci_chip[id].reg_remap;

	rtd_maskl(preg_map->DDC_DDC_SFCR, ~(DDC_DDC_SFCR_DIFFPT_RESET_MASK), DDC_DDC_SFCR_DIFFPT_RESET(1));
	do {
		rtd_maskl(preg_map->DDC_DDC_SFCR, ~(DDC_DDC_SFCR_DIFFPT_RESET_MASK), DDC_DDC_SFCR_DIFFPT_RESET(0));
		if(!(rtd_inl(preg_map->DDC_DDC_SFCR) & DDC_DDC_SFCR_DIFFPT_RESET_MASK))
			break;
	} while(1);
}

// Data Out
int rtk_ddcci_data_out_is_emply(unsigned char id)
{
    const srtk_ddcci_reg_map *preg_map = rtk_ddcci_chip[id].reg_remap;
    int ret = 1;

    rtd_maskl(preg_map->DDC_DDC_SFSR, ~DDC_DDC_SFSR_FFDOET_ST_MASK, DDC_DDC_SFSR_FFDOET_ST(1));
    ret = DDC_DDC_SFSR_GET_FFDOET_ST(rtd_inl(preg_map->DDC_DDC_SFSR));

    return ret;
}

int rtk_ddcci_get_slave_address_RW(unsigned char id)
{
    const srtk_ddcci_reg_map *preg_map = rtk_ddcci_chip[id].reg_remap;
    int ret = 0;
    ret = DDC_DDC_SFSAR_GET_CIRWS_CURRENT(rtd_inl(preg_map->DDC_DDC_SFSAR));   // 0: write, 1: read
    return ret;
}

int rtk_ddcci_get_i2c_write_stop_status(unsigned char id)
{
    const srtk_ddcci_reg_map *preg_map = rtk_ddcci_chip[id].reg_remap;
    int ret = 0;
    int first_stop_st = 0;
    int second_stop_st = 0;

    if(!DDC_DDC_SFBSR_GET_CIRWS_1ST(rtd_inl(preg_map->DDC_DDC_SFBSR))) {
        first_stop_st = DDC_DDC_SFBSR_GET_CIPPT_1ST(rtd_inl(preg_map->DDC_DDC_SFBSR));
    }

    if(!DDC_DDC_SFBSR_GET_CIRWS_2ND(rtd_inl(preg_map->DDC_DDC_SFBSR))) {
         second_stop_st = DDC_DDC_SFBSR_GET_CIPPT_2ND(rtd_inl(preg_map->DDC_DDC_SFBSR));
    }

    if(first_stop_st || second_stop_st) {
        ret = 1;
    }

    return ret;
}

void rtk_ddcci_rw_stop_clear(unsigned char id)
{
    const srtk_ddcci_reg_map *preg_map = rtk_ddcci_chip[id].reg_remap;

    rtd_maskl(preg_map->DDC_DDC_SFBSR, ~DDC_DDC_SFBSR_CISPT_1ST_MASK, DDC_DDC_SFBSR_CISPT_1ST(1));
    rtd_maskl(preg_map->DDC_DDC_SFBSR, ~DDC_DDC_SFBSR_CIPPT_1ST_MASK, DDC_DDC_SFBSR_CIPPT_1ST(1));
    rtd_maskl(preg_map->DDC_DDC_SFBSR, ~DDC_DDC_SFBSR_CISPT_2ND_MASK, DDC_DDC_SFBSR_CISPT_2ND(1));
    rtd_maskl(preg_map->DDC_DDC_SFBSR, ~DDC_DDC_SFBSR_CIPPT_2ND_MASK, DDC_DDC_SFBSR_CIPPT_2ND(1));
}

void rtk_ddcci_tx_fifo_reset(unsigned char id)//[chen] DDC OUT buffer
{
    const srtk_ddcci_reg_map *preg_map = rtk_ddcci_chip[id].reg_remap;

    rtd_maskl(preg_map->DDC_DDC_SFCR, ~(DDC_DDC_SFCR_DOFFPT_RESET_MASK), DDC_DDC_SFCR_DOFFPT_RESET(1));
    rtd_maskl(preg_map->DDC_DDC_SFCR, ~(DDC_DDC_SFCR_DOFFPT_RESET_MASK), DDC_DDC_SFCR_DOFFPT_RESET(0));
    do {
		rtd_maskl(preg_map->DDC_DDC_SFCR, ~(DDC_DDC_SFCR_DOFFPT_RESET_MASK), DDC_DDC_SFCR_DOFFPT_RESET(0));
		if(!(rtd_inl(preg_map->DDC_DDC_SFCR) & DDC_DDC_SFCR_DOFFPT_RESET_MASK))
			break;
    } while(1);
}


int rtk_ddcci_get_number_of_data_in_fifo_tx(unsigned char id)
{
    const srtk_ddcci_reg_map *preg_map = rtk_ddcci_chip[id].reg_remap;
    int buff_len = 0;
    buff_len = DDC_DDC_SFOBSR_GET_FFDO_DLEN(rtd_inl(preg_map->DDC_DDC_SFOBSR));
    return buff_len;
}


//DDCCI=====================================================================================================

//irq

void rtk_ddcci_isr_rx_fifo_t(unsigned char id)
{
#ifdef CONFIG_RTK_KDRV_DDCCI_USING_POLLING
    rtk_ddcci_rcv *p_current_rcv = &p_rcv;
    int buff_len;

    if (p_current_rcv->rx_en == 0 ) {
        buff_len = rtk_ddcci_get_rx_fifo_length(id);

        memset(p_current_rcv->data,0,sizeof(p_current_rcv->data));

        rtk_ddcci_read_rx_fifo_data(id, p_current_rcv->data, buff_len);
        p_current_rcv->len = buff_len;

        rtk_ddcci_rx_fifo_reset(id);
        rtk_ddcci_rw_stop_clear(id);
        p_current_rcv->rx_en = 1;
    }
#else
    rtk_ddcci_rcv *p_current_rcv = &p_rcv;
    int buff_len;
    static unsigned char recv_buff[DDC_BUF_LENGTH];
    buff_len = rtk_ddcci_get_rx_fifo_length(id);
    rtk_ddcci_read_rx_fifo_data(id, recv_buff, buff_len);
    rtk_ddcci_rx_fifo_reset(id);
#ifdef CONFIG_RTK_KDRV_DDCCI_HID    
    if(rtk_ddcci_hid_process(id, recv_buff, buff_len) == false) {
		if (p_current_rcv->rx_en == 0 ) {
			memcpy(p_current_rcv->data, recv_buff, buff_len);
        		p_current_rcv->len = buff_len;
        		p_current_rcv->rx_en = 1;
    		}
    }
#else
    if (p_current_rcv->rx_en == 0 ) {
        memcpy(p_current_rcv->data, recv_buff, buff_len);
        p_current_rcv->len = buff_len;
        p_current_rcv->rx_en = 1;
    }   
#endif
#endif
}

void rtk_ddcci_isr_tx_fifo_t(void)
{
    rtk_ddcci_xmit *p_current_xmit = &p_xmit;
    int ret;
    unsigned char id = g_ddcci_connect_flag;

    if (p_current_xmit->tx_en == 1) {
        rtk_ddcci_tx_fifo_reset(id);
        rtk_ddcci_rw_stop_clear(id);
        ret = _rtk_ddcci_write_data_to_tx_fifo(id, p_current_xmit->data, p_current_xmit->len);
        p_current_xmit->tx_en = 0;
    }
}

bool rtk_ddcci_pooling_t(void)
{
    bool is_isr_happen = false;
    unsigned char id =g_ddcci_connect_flag;
    const srtk_ddcci_reg_map *preg_map = rtk_ddcci_chip[id].reg_remap;

    int first_type = 0, second_type = 0;
    int first_start_st = 0, second_start_st = 0, first_stop_st = 0, second_stop_st = 0;
    int in_empty = 0, out_empty =0;
    int first_stop_flag = 0, second_stop_flag = 0;

    if(rtk_ddcci_chip[id].reg_remap == NULL) {
        return false;
    }

    first_type = DDC_DDC_SFBSR_GET_CIRWS_1ST(rtd_inl(preg_map->DDC_DDC_SFBSR));
    second_type = DDC_DDC_SFBSR_GET_CIRWS_2ND(rtd_inl(preg_map->DDC_DDC_SFBSR));
    first_start_st = DDC_DDC_SFBSR_GET_CISPT_1ST(rtd_inl(preg_map->DDC_DDC_SFBSR));
    second_start_st = DDC_DDC_SFBSR_GET_CISPT_2ND(rtd_inl(preg_map->DDC_DDC_SFBSR));
    first_stop_st = DDC_DDC_SFBSR_GET_CIPPT_1ST(rtd_inl(preg_map->DDC_DDC_SFBSR));
    second_stop_st = DDC_DDC_SFBSR_GET_CIPPT_2ND(rtd_inl(preg_map->DDC_DDC_SFBSR));
    in_empty = rtk_ddcci_data_in_is_emply(id);
    out_empty = rtk_ddcci_data_out_is_emply(id);

    if(!first_type)
        first_stop_flag = first_stop_st;

    if(!second_type)
        second_stop_flag = second_stop_st;

    //RTK_DDC_ERR("\033[1;35m[ddcci]r(%d)t(%d)en(%d)lr(%d)lt(%d),(%d)(%d)(%d)(%d),(%d)(%d)\033[m\n", in_empty,out_empty,p_rcv.rx_en,rtk_ddcci_get_rx_fifo_length(id),rtk_ddcci_get_number_of_data_in_fifo_tx(id),first_start_st,first_stop_st,second_start_st,second_stop_st,first_type,second_type);		

#ifdef CONFIG_RTK_KDRV_DDCCI_USING_POLLING
    if(!in_empty && (first_stop_flag || second_stop_flag)) {
        rtk_ddcci_isr_rx_fifo_t(id);
    }
#else
   if(rtd_inl(preg_map->DDC_DDC_SFBSR) & 0xA) {
   	rtd_outl(preg_map->DDC_DDC_SFBSR, rtd_inl(preg_map->DDC_DDC_SFBSR) | 0xA);
    	is_isr_happen = true;
   }
    if((first_stop_flag || second_stop_flag)) {
	 if(!in_empty)
		rtk_ddcci_isr_rx_fifo_t(id);
    }
#endif
    return is_isr_happen;
}


int rtk_ddcci_xmit_message(unsigned char *tx_data, unsigned char len)
{
    int res = 0;
    rtk_ddcci_xmit *p_current_xmit = &p_xmit;

    if(p_current_xmit->tx_en == 0) {
        memcpy(p_current_xmit->data, (unsigned char*) tx_data, len);
        p_current_xmit->len = len;

        p_current_xmit->tx_en = 1;
        res = 1;
    }
    return res;
}

int rtk_ddcci_rcv_message(unsigned char *rx_data, unsigned char *len)
{
    int res = 0;
    rtk_ddcci_rcv *p_current_rcv = &p_rcv;

    if (p_current_rcv->rx_en == 1) {
        if (p_current_rcv->len > DDC_BUF_LENGTH) {
            RTK_DDC_ERR("[ddcci] rcv len over max, (%d)=>(%d)\n", p_current_rcv->len, DDC_BUF_LENGTH);
            p_current_rcv->len = DDC_BUF_LENGTH;
        }
        *len = p_current_rcv->len;
        memcpy(rx_data, p_current_rcv->data, *len);
        res = 1;
        p_current_rcv->rx_en = 0;
    }
    return res;
}

int rtk_ddcci_fifo_thread(void* arg)
{
    while(!kthread_should_stop())
    {
        rtk_ddcci_pooling_t();
#ifdef CONFIG_RTK_KDRV_DDCCI_HID
        msleep(1);
#else
        msleep(15);
#endif
    }
    return 0;
}

