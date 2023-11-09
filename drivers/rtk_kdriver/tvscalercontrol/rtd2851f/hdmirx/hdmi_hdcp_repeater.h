#ifndef __HDMI_HDCP_REPEATER_H__
#define __HDMI_HDCP_REPEATER_H__

//-----------------------------------------------------------------
// HDCP Repeater related
//-----------------------------------------------------------------
#define HDCP_KSV_FIFO_I2C_SLAVE_ADDR    0x74
#define HDCP_REG_43_KSV_FIFO            0x43
#define HDCP_KSV_INFO_SIZE              0x5
#define HDCP_MAX_DEVICE_CNT             128
#define HDCP_KSV_FIFO_LENGTH            (HDCP_MAX_DEVICE_CNT * HDCP_KSV_INFO_SIZE)

#define HDCP_REPEATER_EVENT_UPDATE_HDCP14_KEY           0x1   // Notify HDCP 14 KEY is ready
#define HDCP_REPEATER_EVENT_UPDATE_AKSV                 0x2   // Notify HDCP REPEATER that AKSV is received
#define HDCP_REPEATER_EVENT_UPDATE_KSV_FIFO             0x4   // Notify HDCP REPEATER that KSV FIFO is changed
#define HDCP_REPEATER_EVENT_UPDATE_AKE_INIT             0x8   // Notify HDCP AKE_INIT is updated
#define HDCP_REPEATER_EVENT_HDCP2_EKS                   0x10  // Notify HDCP HDCP2_EKS is updated

#define HDCP_REPEATER_FSM_IDEL                  0
#define HDCP_REPEATER_FSM_HDCP14_AKSV           1
#define HDCP_REPEATER_FSM_WAIT_KSV_FIFO_READY   2
#define HDCP_REPEATER_FSM_KSV_FIFO_READY        3
#define HDCP_REPEATER_FSM_WAIT_HDCP_KSV_FIFO_READ  4  // for I2C KSV FIFO Complete
#define HDCP_REPEATER_FSM_CHECK_STATUS          5

//-----------------------------------------------------------------
// HDCP1 B_STATUS
//-----------------------------------------------------------------
#define BSTATUS_HDMI_MODE(x)                ((x & 0x1)<<12)
#define BSTATUS_MAX_CASCADE_EXCEEDED(x)     ((x & 0x1)<<11)
#define BSTATUS_DEPTH(x)                    ((x & 0x7)<<8)
#define BSTATUS_MAX_DEVS_EXCEEDED(x)        ((x & 0x1)<<7)
#define BSTATUS_DEVICE_COUNT(x)             ((x & 0x7F))

#define BSTATUS_GET_MAX_CASCADE_EXCEEDED(x) ((x >>11) & 0x1)
#define BSTATUS_GET_DEPTH(x)                ((x >>8) & 0x7)
#define BSTATUS_GET_MAX_DEVS_EXCEEDED(x)    ((x >>7) & 0x1)
#define BSTATUS_GET_DEVICE_COUNT(x)         ((x & 0x7F))

//-----------------------------------------------------------------
// HDCP2 RX INFO
//-----------------------------------------------------------------
#define RX_INFO_DEPTH(x)                    ((x & 0x7)<<9)
#define RX_INFO_DEVICE_COUNT(x)             ((x & 0x1F)<<4)
#define RX_INFO_MAX_DEVS_EXCEEDED           (1<<3)
#define RX_INFO_MAX_CASCADE_EXCEDED         (1<<2)
#define RX_INFO_HDCP2_0_REPEATER_DOWNSTREAM (1<<1)
#define RX_INFO_HDCP1_DEVICE_DOWNSTREAM     (1)

#define RX_INFO_GET_DEPTH(x)                       ((x >>9 )& 0x7)
#define RX_INFO_GET_DEVICE_COUNT(x)                ((x >>4) & 0x1F)
#define RX_INFO_GET_MAX_DEVS_EXCEEDED(x)           ((x>>3) & 0x1)
#define RX_INFO_GET_MAX_CASCADE_EXCEDED(x)         ((x>>2) & 0x1)
#define RX_INFO_GET_HDCP2_0_REPEATER_DOWNSTREAM(x) ((x>>1) & 0x1)
#define RX_INFO_GET_HDCP1_DEVICE_DOWNSTREAM(x)     (x & 0x1)

//-----------------------------------------------------------------
// HDCP Repeater related
//-----------------------------------------------------------------

#ifdef CONFIG_RTK_KDRV_HDMI_HDCP_REPEATER_ENABLE
extern void newbase_hdmi_hdcp_repeater_enable(unsigned char enable);
extern unsigned char newbase_hdmi_is_hdcp_repeater_enable(void);
extern void newbase_hdmi_hdcp_repeater_handler(unsigned char port);
extern void newbase_hdmi_hdcp_repeater_send_event(unsigned char port, unsigned int event);
extern int  newbase_hdmi_hdcp_repeater_update_downstream_topology(unsigned short rx_status[2], unsigned char* p_topology_list, unsigned int topology_len);
extern void rtk_hdcp2_set_repeater(unsigned char nport, unsigned char repeater);
extern unsigned char rtk_hdcp2_get_repeater(unsigned char nport);
extern void rtk_hdcp2_set_rxinfo_ridlist(unsigned char nport, unsigned short rxinfo, unsigned char *ridlist);
extern unsigned short rtk_hdcp2_get_rxinfo(unsigned char nport);
extern void rtk_hdcp2_set_content_type(unsigned char nport, unsigned char type);
extern unsigned char rtk_hdcp2_get_content_type(unsigned char nport);

#else
#define newbase_hdmi_hdcp_repeater_enable(enable)
#define newbase_hdmi_is_hdcp_repeater_enable(port)        (0)
#define newbase_hdmi_hdcp_repeater_handler(port)
#define newbase_hdmi_hdcp_repeater_send_event(port, event)
#define newbase_hdmi_hdcp_repeater_update_downstream_topology(hdcp_status, p_topology_list, topology_len)  (0)
#define rtk_hdcp2_set_repeater(port, repreater)
#define rtk_hdcp2_get_repeater(port)                (0)
#define rtk_hdcp2_set_rxinfo_ridlist(port, rxinfo,ridlist)
#define rtk_hdcp2_get_rxinfo(port)                  (0)
#define rtk_hdcp2_set_content_type(port, type)
#define rtk_hdcp2_get_content_type(port)                  (0)
#endif // CONFIG_RTK_KDRV_HDMI_HDCP_REPEATER_ENABLE

#endif //__HDMI_HDCP_REPEATER_H__
