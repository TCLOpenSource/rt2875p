#ifndef _HDMI_VFE_CONFIG_H_
#define _HDMI_VFE_CONFIG_H_

#include <tvscalercontrol/hdmirx/hdmifun.h>
#include <pcbMgr.h>
#include "hdmi_table.h"

enum hdmi_rx_port_idx_e
{
       HDMI_RX_PORT_IDX_0 = 0,
       HDMI_RX_PORT_IDX_1,
       HDMI_RX_PORT_IDX_2,
       HDMI_RX_PORT_IDX_3,
       HDMI_RX_PORT_MAX_NUM,
};


int hdmi_vfe_get_rx_port_info(unsigned char port, HDMI_CHANNEL_T_PCB_INFO* p_info);
int hdmi_vfe_get_hdmi_port(unsigned char ch, unsigned char* p_port);
int hdmi_vfe_get_hdmi_channel(unsigned char port, unsigned char* p_ch);
#ifdef MODULE
void get_bootparam_systemd_unit(void);
#endif
void board_hdmi_drv_port_mapping_Initial(void);

extern unsigned int newbase_hdmi_calculate_entry_max_value(HDMI_FLOW_CFG_GROUP group_id, unsigned char entry_id);
extern int  newbase_hdmi_set_flow_cfg_entry(HDMI_FLOW_CFG_GROUP group_id, unsigned char entry_id, unsigned int set_value);
extern unsigned int newbase_hdmi_get_flow_cfg_entry(HDMI_FLOW_CFG_GROUP group_id, unsigned char entry_id);
extern const char* newbase_hdmi_get_flow_cfg_name(HDMI_FLOW_CFG_GROUP group_id, unsigned char entry_id);
extern void newbase_hdmi_flow_cfg_info_init(void);
extern unsigned char newbase_hdmi_is_socts_mode(void);

#define GET_FLOW_CFG_NAME(x, y)    newbase_hdmi_get_flow_cfg_name(x,  y)
#define GET_FLOW_CFG(x, y)    newbase_hdmi_get_flow_cfg_entry(x,  y)
#define SET_FLOW_CFG(x, y, z)    newbase_hdmi_set_flow_cfg_entry(x,  y, z)

#endif // _HDMI_VFE_CONFIG_H_
