#include <linux/syscalls.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <rtd_log/rtd_module_log.h>
//#include <mach/io.h>
#include "hdmiPlatform.h"
#include "hdmiInternal.h"
#include "hdmi_hdcp.h"
#include "hdmi_debug.h"
#include "hdcp14_repeater/hdcp14_api.h"
#include <tvscalercontrol/hdmirx/hdcp2_optee.h>

//-------------------------------------------------------------------------
// External Variable
//-------------------------------------------------------------------------
extern HDCP_KEY_T hdcpkey;

//-------------------------------------------------------------------------
// External Function
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
// Internal Parameter
//-------------------------------------------------------------------------
static unsigned char g_topology_list[HDCP_KSV_FIFO_LENGTH];   // buffer to store KSV list, this is used to compute V'. each KSV should be MSB first
static unsigned char g_ksv_fifo[HDCP_KSV_FIFO_LENGTH];   // buffer to KSV Fifo, this is used to output KSV via I2C. each KSV should be LSB first
static unsigned short g_hdcp14_rx_info = 0;
static unsigned short g_hdcp22_rx_info = 0;
static unsigned int  hdcp_repeater_en = 0;

unsigned char hdcp2_repeater[HDMI_PORT_TOTAL_NUM] = {0};
unsigned short hdcp2_rxinfo[HDMI_PORT_TOTAL_NUM] = {0};
unsigned char hdcp2_content_type[HDMI_PORT_TOTAL_NUM] = {0};

//-------------------------------------------------------------------------
// HDCP14 Cipher function
//-------------------------------------------------------------------------

bool __hdcp14_compute_m(
    unsigned char       encrypted_bkeys[320],
    unsigned char       aksv[5],
    unsigned char       repeater,
    unsigned char       an[8],
    unsigned char       m[8],
    unsigned char       r0[2]
    )
{
    unsigned char km[7];
    unsigned char ks[7];

    // compute km
    if (optee_hdcp14_compute_km(encrypted_bkeys, aksv, km) != true)
        return false;

    HDCP_FLOW_PRINTF("__hdcp14_compute_m, km = %x, %x, %x, %x, %x, %x, %x\n",
        km[0], km[1], km[2], km[3], km[4], km[5], km[6]);

    // compute m and r0
    return hdcp14_block_cipher(km, (repeater) ? 1 : 0, an,  ks, m, r0);
}


/*------------------------------------------------------------------
 * Func : __hdcp14_compute_v
 *
 * Desc : compute hdcp 1.4 V'
 *
 * Para : [IN] ksv_list : KSV list (MSB first)
 *        [IN] list_len : size of ksv list (in bytes)
 *        [IN] bstatus  : bstatus (MSB first)
 *        [IN] m        : m key (MSB first)
 *        [OUT] v       : V' (MSB first)
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
bool __hdcp14_compute_v(
    unsigned char*      ksv_list,
    unsigned char       list_len,
    unsigned char       bstatus[2],
    unsigned char       m[8],
    unsigned char       v[20])
{
    return hdcp14_compute_v(ksv_list, list_len, bstatus, m, v);
}

/*------------------------------------------------------------------
 * Func : _hdmi_hdcp14_get_m
 *
 * Desc : check hdcp 1.4 repeater is enabled or not
 *
 * Para : [IN] port : enable hdcp repeater
 *        [IN] m[8] : 64 bits secreate key
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
bool _hdmi_hdcp14_get_m(unsigned char port, unsigned char m[8])
{
    bool ret = false;
    unsigned char aksv[5];
    unsigned char an[8];
    unsigned char r[2];

    HDCP_FLOW_PRINTF("HDMI[p%d] hdcp key = %x, %x,%x, %x,%x, %x, %x\n",
            port,
            hdcpkey.Key[0], hdcpkey.Key[1], hdcpkey.Key[2],
            hdcpkey.Key[3], hdcpkey.Key[4], hdcpkey.Key[5],
            hdcpkey.Key[6]);

    if (newbase_hdmi_hdcp14_read_aksv_ex(port, aksv, 1) != 1)
        return false;

    newbase_hdmi_hdcp14_read_an_ex(port, an, 1);

    HDCP_FLOW_PRINTF("HDMI[p%d] _hdmi_hdcp14_get_m, an = %x, %x,%x, %x,%x, %x,%x, %x, repeater=%d\n",
        port, an[0], an[1], an[2], an[3], an[4], an[5], an[6], an[7],
        (lib_hdmi_hdcp_port_read(port, 0x40) & (1<<6)) ?  1 : 0);

    if (lib_hdmi_hdcp_port_read(port, 0x40) & (1<<6))
        ret = __hdcp14_compute_m(hdcpkey.Key, aksv, 1, an, m, r);
    else
        ret = __hdcp14_compute_m(hdcpkey.Key, aksv, 0, an, m, r);

    HDCP_FLOW_PRINTF("HDMI[p%d] _hdmi_hdcp14_get_m, compute r0' = %x, %x, m=%x, %x,%x, %x,%x, %x,%x, %x\n",
                port, r[0], r[1], m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7]);

    return ret;
}

/*------------------------------------------------------------------
 * Func : _hdmi_hdcp14_get_v
 *
 * Desc : check hdcp 1.4 repeater is enabled or not
 *
 * Para : [IN] port : enable hdcp repeater
 *        [IN] enable : enable/disable hdcp repeater
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
static bool _hdmi_hdcp14_get_v(
    unsigned char           port,
    unsigned char*          ksv_list,
    unsigned int            list_len,
    unsigned char*          v
    )
{
    bool ret = false;
    unsigned char m[8];
    unsigned char bstatus[2];

    ret = _hdmi_hdcp14_get_m(port, m);
    if(ret != true)
        return ret;

    newbase_hdmi_hdcp14_read_bstatus_ex(port, bstatus, 1);  // read bstatus in big endian mode

    ret = hdcp14_compute_v(ksv_list, list_len, bstatus, m, v);
    return ret;
}


//-------------------------------------------------------------------------
// Main Function
//-------------------------------------------------------------------------

/*------------------------------------------------------------------
 * Func : newbase_hdmi_hdcp_repeater_enable
 *
 * Desc : enable / disable hdcp repeater
 *
 * Para : [IN] enable : enable/disable hdcp repeater
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void newbase_hdmi_hdcp_repeater_enable(unsigned char enable)
{
    unsigned char port = 0;
    unsigned char reset_hdcp_fsm = 0;

    if (enable)
    {
        if (hdcp_repeater_en==0)
        {
            hdcp_repeater_en = 1;
            reset_hdcp_fsm = 1;
        }

#if HDMI_FIX_RL6672_3031_HDCP14_KSV_FIFO
        lib_hdmi_hdcp_set_fifo_mode(port, 1, 0);       // set HDCP fifo to auto share between HDCP14 and HDCP22 when repeater is enabled
#endif // HDMI_FIX_RL6672_3031_HDCP14_KSV_FIFO
    }
    else
    {
        if (hdcp_repeater_en)
        {
            hdcp_repeater_en = 0;
            reset_hdcp_fsm = 1;
        }

#if HDMI_FIX_RL6672_3031_HDCP14_KSV_FIFO
        lib_hdmi_hdcp_set_fifo_mode(port, 0, 1);       // set HDCP fifo to HDCP22 when repeater is disabled
#endif // HDMI_FIX_RL6672_3031_HDCP14_KSV_FIFO
    }

    //----------------------------------
    // Reset HDCP fsm for each port
    //----------------------------------
    if (reset_hdcp_fsm)
    {
        for (port=0; port<HDMI_PORT_TOTAL_NUM; port++)
        {
            HDMI_HDCP_ST* p_hdcp = newbase_hdcp_get_hdcp_status(port);

            if (p_hdcp==NULL)
                continue;
            newbase_hdmi_hdcp_reset_fsm(port);
        }
    }
}

/*------------------------------------------------------------------
 * Func : newbase_hdmi_is_hdcp_repeater_enable
 *
 * Desc : check hdcp repeater is enabled or not
 *
 * Para : N/A
 *
 * Retn :  0 : disabled, others : enabled
 *------------------------------------------------------------------*/
unsigned char newbase_hdmi_is_hdcp_repeater_enable(void)
{
    return hdcp_repeater_en;
}


/*------------------------------------------------------------------
 * Func : newbase_hdmi_hdcp_repeater_handler
 *
 * Desc : check hdcp 1.4 repeater is enabled or not
 *
 * Para : [IN] port : enable hdcp repeater
 *        [IN] enable : enable/disable hdcp repeater
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void newbase_hdmi_hdcp_repeater_send_event(unsigned char port, unsigned int event)
{
    HDMI_HDCP_ST* p_hdcp = newbase_hdcp_get_hdcp_status(port);
    unsigned char b_caps;

    if (p_hdcp==NULL)
        return ;

    switch(event)
    {
    case HDCP_REPEATER_EVENT_UPDATE_AKSV:
        p_hdcp->fsm = HDCP_REPEATER_FSM_IDEL;  // force reauth
        p_hdcp->status = AKSV_READY;
        newbase_hdmi_hdcp14_read_bcaps(port, &b_caps);
        newbase_hdmi_hdcp14_write_bcaps(port, b_caps & ~(1<<5)); // clear KSV fifo ready
        break;

    case HDCP_REPEATER_EVENT_UPDATE_AKE_INIT:   // restart HDCP22

        if (newbase_hdmi_is_hdcp_repeater_enable())
        {
            rtk_hdcp2_set_repeater(port, 1);
            rtk_hdcp2_set_rxinfo_ridlist(port, g_hdcp22_rx_info, g_topology_list);
        }
        else
        {
            rtk_hdcp2_set_repeater(port, 0);
            rtk_hdcp2_set_rxinfo_ridlist(port, g_hdcp22_rx_info, g_topology_list);
        }
        break;

    case HDCP_REPEATER_EVENT_HDCP2_EKS:
        p_hdcp->fsm = HDCP_REPEATER_FSM_IDEL;  // force reauth
        p_hdcp->status = EKS_READY;
        break;

    default:
        break;
    }
}

const char* _hdcp_repeater_fsm_str(unsigned int fsm)
{
    switch(fsm)
    {
    case HDCP_REPEATER_FSM_IDEL: return "IDEL";
    case HDCP_REPEATER_FSM_HDCP14_AKSV: return "HDCP14_AKSV";
    case HDCP_REPEATER_FSM_WAIT_KSV_FIFO_READY: return "WAIT_KSV_FIFO_READY";
    case HDCP_REPEATER_FSM_WAIT_HDCP_KSV_FIFO_READ: return "WAIT_HDCP_KSV_FIFO_READ";
    case HDCP_REPEATER_FSM_CHECK_STATUS: return "Check Status";
    }
    return "Unknow";
}

/*------------------------------------------------------------------
 * Func : newbase_hdmi_hdcp14_repeater_fsm
 *
 * Desc : FSM of HDCP 1.4 repeater
 *
 * Para : [IN] port : enable hdcp repeater
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void newbase_hdmi_hdcp14_repeater_fsm(unsigned char port)
{
    HDMI_HDCP_ST* p_hdcp = newbase_hdcp_get_hdcp_status(port);
    unsigned int device_count = BSTATUS_GET_DEVICE_COUNT(g_hdcp14_rx_info);

    if (p_hdcp==NULL)
        return ;

    switch(p_hdcp->fsm)
    {
    case HDCP_REPEATER_FSM_IDEL:
        // initial state. wait hdcp 1.4 start
        if ((p_hdcp->status & AKSV_READY))
        {
            p_hdcp->fsm = HDCP_REPEATER_FSM_HDCP14_AKSV;
        }
        break;

    case HDCP_REPEATER_FSM_HDCP14_AKSV:

        if ((p_hdcp->status & AKSV_READY))
        {
            // compute M0'
            _hdmi_hdcp14_get_m(port, p_hdcp->m);

            HDCP_FLOW_PRINTF("HDMI[p%d] compute m0' = %x, %x, %x, %x, %x, %x, %x, %x\n",
                port,
                p_hdcp->m[0],
                p_hdcp->m[1],
                p_hdcp->m[2],
                p_hdcp->m[3],
                p_hdcp->m[4],
                p_hdcp->m[5],
                p_hdcp->m[6],
                p_hdcp->m[7]);

            p_hdcp->fsm = HDCP_REPEATER_FSM_WAIT_KSV_FIFO_READY;
        }
        else
            p_hdcp->fsm = HDCP_REPEATER_FSM_IDEL;
        break;

    case HDCP_REPEATER_FSM_WAIT_KSV_FIFO_READY:

        if ((p_hdcp->status & AKSV_READY)==0)
        {
            p_hdcp->fsm = HDCP_REPEATER_FSM_IDEL;
        }
        else if (device_count)
        {
            unsigned char v_prime[20];
            unsigned char b_status[2];
            unsigned char b_caps;

            HDCP_INFO("HDMI[p%d] downstream KSV fifo ready\n", port);

            //---------------------------------------------
            // Update KSV FIFO
            //---------------------------------------------
            newbase_hdmi_hdcp14_write_ksv_fifo(port, g_ksv_fifo, (HDCP_KSV_INFO_SIZE * device_count));

            //---------------------------------------------
            // Update HDCP BStatus
            //---------------------------------------------
            b_status[0] = (g_hdcp14_rx_info) & 0xFF;
            b_status[1] = (g_hdcp14_rx_info>>8) & 0xFF;
            newbase_hdmi_hdcp14_write_bstatus(port, b_status);

            //---------------------------------------------
            // compute V'
            //---------------------------------------------
            _hdmi_hdcp14_get_v(port, g_topology_list, device_count *HDCP_KSV_INFO_SIZE, v_prime);

            HDCP_FLOW_PRINTF("HDMI[p%d] compute V' = %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x\n",
                        port,
                        v_prime[0],  v_prime[1],  v_prime[2],  v_prime[3],
                        v_prime[4],  v_prime[5],  v_prime[6],  v_prime[7],
                        v_prime[8],  v_prime[9],  v_prime[10], v_prime[11],
                        v_prime[12], v_prime[13], v_prime[14], v_prime[15],
                        v_prime[16], v_prime[17], v_prime[18], v_prime[19]);

            newbase_hdmi_hdcp14_write_v_prime(port, v_prime);

            //---------------------------------------------
            // Update HDCP Bcaps
            //---------------------------------------------
            newbase_hdmi_hdcp14_read_bcaps(port, &b_caps);
            newbase_hdmi_hdcp14_write_bcaps(port, (b_caps | (1<<5))); // setup KSV fifo ready

            //---------------------------------------------
            // update fsm
            //---------------------------------------------
            p_hdcp->fsm = HDCP_REPEATER_FSM_CHECK_STATUS;
            p_hdcp->status |= KSV_FIFO_READY;
        }
        break;

    case HDCP_REPEATER_FSM_CHECK_STATUS:

        // check HDCP status
        if ((p_hdcp->status & AKSV_READY)==0 || device_count==0)
        {
            p_hdcp->fsm = HDCP_REPEATER_FSM_IDEL;
        }
        break;

    default:
        p_hdcp->fsm = HDCP_REPEATER_FSM_IDEL;
    }
}

extern unsigned char g_TX_msg_buf[HDMI_PORT_TOTAL_NUM][540];
#ifdef CONFIG_OPTEE_HDCP2
extern unsigned char g_RX_msg_buf[HDMI_PORT_TOTAL_NUM][540] ; //rx->tx msg buf
#endif

void lib_hdmi_hdcp22_set_ready(unsigned char nport, unsigned short wLen);


void rtk_hdcp2_set_repeater(unsigned char nport, unsigned char repeater)
{
	hdcp2_repeater[nport] = repeater;
	optee_hdcp2_repeater_set_enable(HDCP2_CMD_REPEATER_SET_ENABLE, nport,repeater);
}
unsigned char rtk_hdcp2_get_repeater(unsigned char nport)
{
	return hdcp2_repeater[nport];
}
void rtk_hdcp2_set_rxinfo_ridlist(unsigned char nport, unsigned short rxinfo, unsigned char *ridlist)
{
	unsigned char DeviceCount;
	unsigned char Depth;
	unsigned char DevicesExceeded;
	unsigned char DepthExceeded;
	unsigned char temp_rxinfo[2];
	if(ridlist == NULL)
	{
		rtd_pr_hdmi_emerg("ridlist is NULL\n");
		return;
	}
	temp_rxinfo[0] = (rxinfo >> 8) & 0xff;
	temp_rxinfo[1] = rxinfo & 0xff;
	DeviceCount = ((temp_rxinfo[0]&0x1)<<4)|((temp_rxinfo[1]>>4)&0xff);;
	Depth = (temp_rxinfo[0]>>1)&0x7;
	DevicesExceeded = (temp_rxinfo[1]>>3)&0x1;
	DepthExceeded = (temp_rxinfo[1]>>2)&0x1;
	if(DeviceCount > 32 || Depth > 4 || DevicesExceeded || DepthExceeded)
	{
		rtd_pr_hdmi_emerg("hdcp2 repeater rxinfo parse fail,DeviceCount:%d,Depth:%d,DevicesExceeded:%d,DepthExceeded:%d \n",DeviceCount,Depth,DevicesExceeded,DepthExceeded);
		DeviceCount = 0;
	}
	rtd_pr_hdmi_emerg("hdcp2 repeater rxinfo parse ok,DeviceCount:%d,Depth:%d,DevicesExceeded:%d,DepthExceeded:%d \n",DeviceCount,Depth,DevicesExceeded,DepthExceeded);
	hdcp2_rxinfo[nport] = rxinfo;
	optee_hdcp2_repeater_set_rxinfo_ridlist(HDCP2_CMD_REPEATER_SET_RXINFO_RIDLIST, nport,rxinfo,ridlist,DeviceCount*5);
}

unsigned short rtk_hdcp2_get_rxinfo(unsigned char nport)
{
	return hdcp2_rxinfo[nport];
}

void rtk_hdcp2_set_content_type(unsigned char nport, unsigned char type)
{
	hdcp2_content_type[nport] = type;
}
unsigned char rtk_hdcp2_get_content_type(unsigned char nport)
{
	return hdcp2_content_type[nport];
}
/*------------------------------------------------------------------
 * Func : newbase_hdmi_hdcp22_repeater_fsm
 *
 * Desc : FSM of HDCP 2.2 repeater
 *
 * Para : [IN] port : enable hdcp repeater
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void newbase_hdmi_hdcp22_repeater_fsm(unsigned char port)
{
    HDMI_HDCP_ST* p_hdcp = newbase_hdcp_get_hdcp_status(port);

    if ((p_hdcp->status & EKS_READY) && rtk_hdcp2_get_repeater(port) && RX_INFO_GET_DEVICE_COUNT(g_hdcp22_rx_info))
    {
        unsigned char device_count;
        unsigned char depth;
        unsigned char devices_exceeded;
        unsigned char depth_exceeded;
        unsigned char temp_rxinfo[2];

        temp_rxinfo[0] = (rtk_hdcp2_get_rxinfo(port) >> 8) & 0xff;
        temp_rxinfo[1] = rtk_hdcp2_get_rxinfo(port) & 0xff;

        device_count = ((temp_rxinfo[0]&0x1)<<4)|((temp_rxinfo[1]>>4)&0xff);
        depth = (temp_rxinfo[0]>>1)&0x7;
        devices_exceeded = (temp_rxinfo[1]>>3)&0x1;
        depth_exceeded = (temp_rxinfo[1]>>2)&0x1;

        HDCP_WARN("g_hdcp22_rx_info=%04x (%04x) hdcp22 state=%x\n", g_hdcp22_rx_info, rtk_hdcp2_get_rxinfo(port), lib_hdmi_hdcp22_get_state(port));

        if (device_count > 32 || depth > 4 || devices_exceeded || depth_exceeded)
        {
            HDMI_WARN("hdcp2 repeater rxinfo parse fail, device_count:%d, depth:%d, devices_exceeded:%d, depth_exceeded:%d \n",
                device_count, depth, devices_exceeded, depth_exceeded);
            device_count = 0;
        }

        optee_hdcp2_main(HDCP2_CMD_POLLING_MESSAGE, port, g_TX_msg_buf[port], 17, g_RX_msg_buf[port], 22 + device_count*5);

        //send out message
        newbase_hdmi_hdcp22_write_data_to_tx(port, (unsigned char*)g_RX_msg_buf[port], 22+device_count*5);
        lib_hdmi_hdcp22_set_ready(port, 22+device_count*5);  // setup ready flag
        p_hdcp->status = 0;
    }
}


/*------------------------------------------------------------------
 * Func : newbase_hdmi_hdcp_repeater_handler
 *
 * Desc : check hdcp 1.4 repeater is enabled or not
 *
 * Para : [IN] port : enable hdcp repeater
 *        [IN] enable : enable/disable hdcp repeater
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void newbase_hdmi_hdcp_repeater_handler(unsigned char port)
{
    // check the HDCP status of the upstream connection
    if (lib_hdmi_hdcp22_is_onoff(port)==0 && lib_hdmi_hdcp14_is_onoff(port))
        newbase_hdmi_hdcp14_repeater_fsm(port);
    else if (lib_hdmi_hdcp22_is_onoff(port) && lib_hdmi_hdcp14_is_onoff(port)==0)
        newbase_hdmi_hdcp22_repeater_fsm(port);
}



/*------------------------------------------------------------------
 * Func : newbase_hdmi_hdcp_repeater_update_downstream_topology
 *
 * Desc : update down stream hdcp ksv list. this function will be called
 *        as downstream hdcp status updated. when ksv updated, the HDMI
 *        FW will update bstatus of each HDMI port
 *
 * Para : [IN] hdcp_status : [0] for hdcp1.4 b_status, [1] hdcp 2.2 rx_ifno
 *        [IN] p_ksv_list : ksv list
 *        [IN] ksv_len    : length of ksv list
 *
 * Retn : 0: success, -1 : failed
 *------------------------------------------------------------------*/
int newbase_hdmi_hdcp_repeater_update_downstream_topology(
    unsigned short                  rx_status[2],
    unsigned char*                  p_topology_list,
    unsigned int                    topology_len
    )
{
    unsigned char port = 0;
    unsigned char depth = 0;
    unsigned int  device_count = 0;

    // check the correctness of hdcp14/hdcp22 status
    // depth : should be the same
    // max_cascade : should be the same
    // device count : if hdcp 14 device count < 31, then hdcp14 dev count should be hdcp22 dev count
    //                hdcp14 device count should >= hdcp22 device count
    if ((BSTATUS_GET_DEPTH(rx_status[0]) != RX_INFO_GET_DEPTH(rx_status[1])) ||
        (BSTATUS_GET_DEVICE_COUNT(rx_status[0]) < RX_INFO_GET_DEVICE_COUNT(rx_status[1])) ||
        (BSTATUS_GET_DEVICE_COUNT(rx_status[0]) < 31 && (BSTATUS_GET_DEVICE_COUNT(rx_status[0]) != RX_INFO_GET_DEVICE_COUNT(rx_status[1]))) ||
        (BSTATUS_GET_MAX_CASCADE_EXCEEDED(rx_status[0]) != RX_INFO_GET_MAX_CASCADE_EXCEDED(rx_status[1]))
        )
    {
        HDCP_WARN("newbase_hdmi_hdcp_repeater_update_downstream_topology failed, hdcp status mismatch\n");

        HDCP_WARN("hdcp14_status : %04x, depth=%d, devs=%d, max_cascade=%d\n",
            rx_status[0], BSTATUS_GET_DEPTH(rx_status[0]), BSTATUS_GET_DEVICE_COUNT(rx_status[0]), BSTATUS_GET_MAX_CASCADE_EXCEEDED(rx_status[0]));

        HDCP_WARN("hdcp22_status : %04x, depth=%d, devs=%d, max_cascade=%d\n",
            rx_status[1], RX_INFO_GET_DEPTH(rx_status[1]), RX_INFO_GET_DEVICE_COUNT(rx_status[1]), RX_INFO_GET_MAX_CASCADE_EXCEDED(rx_status[1]));

        return -1;
    }

    // get device/depth information
    depth            = BSTATUS_GET_DEPTH(rx_status[0]);
    device_count     = BSTATUS_GET_DEVICE_COUNT(rx_status[0]);  // using hdcp14 device count as the device count
    g_hdcp14_rx_info = rx_status[0];     // store hdcp14 rx info
    g_hdcp22_rx_info = rx_status[1];     // store hdcp22 rx info

    // check topology data
    if ((device_count && p_topology_list==NULL) || ((device_count*5)>topology_len))
    {
        HDCP_WARN("newbase_hdmi_hdcp_repeater_update_downstream_topology failed, topology mismatch (device=%d, topology=%p, len=%d (shold be %d))\n",
            device_count, p_topology_list, topology_len, device_count*5);
        return -1;
    }

    // copy topology data
    if (p_topology_list==NULL || topology_len==0)
    {
        memset(g_ksv_fifo, 0, sizeof(g_topology_list));  // clear KSV fifo
        memset(g_topology_list, 0, sizeof(g_ksv_fifo));  // clear KSV list

        // HDCP22 repeater
        rtk_hdcp2_set_rxinfo_ridlist(port, 0, NULL);
    }
    else
    {
        int i;

        if (topology_len > sizeof(g_topology_list))
        {
            HDCP_WARN("newbase_hdmi_hdcp_repeater_update_downstream_topology failed, topology size %d > %d\n", topology_len, sizeof(g_topology_list));
            return -1;
        }

        memset(g_topology_list, 0, sizeof(g_topology_list));          // clear KSV fifo
        memset(g_ksv_fifo, 0, sizeof(g_ksv_fifo));          // clear KSV fifo
        memcpy(g_topology_list, p_topology_list, topology_len);  // copy data

        // setup ksv fifo, ksv list is big endian but ksv fifo should be little endian
        for (i=0; i<device_count; i++)
        {
            g_ksv_fifo[(i*HDCP_KSV_INFO_SIZE)]   = g_topology_list[(i*HDCP_KSV_INFO_SIZE)+4];
            g_ksv_fifo[(i*HDCP_KSV_INFO_SIZE)+1] = g_topology_list[(i*HDCP_KSV_INFO_SIZE)+3];
            g_ksv_fifo[(i*HDCP_KSV_INFO_SIZE)+2] = g_topology_list[(i*HDCP_KSV_INFO_SIZE)+2];
            g_ksv_fifo[(i*HDCP_KSV_INFO_SIZE)+3] = g_topology_list[(i*HDCP_KSV_INFO_SIZE)+1];
            g_ksv_fifo[(i*HDCP_KSV_INFO_SIZE)+4] = g_topology_list[(i*HDCP_KSV_INFO_SIZE)];
        }

        rtk_hdcp2_set_rxinfo_ridlist(port, g_hdcp22_rx_info, g_topology_list);
    }

    HDCP_WARN("newbase_hdmi_hdcp_repeater_update_downstream_topology, depth=%d, device_count=%d\n", depth, device_count);

    //----------------------------------
    // Update status of each HDMI Port
    //----------------------------------
    for (port=0; port<HDMI_PORT_TOTAL_NUM; port++)
    {
        HDMI_HDCP_ST* p_hdcp = newbase_hdcp_get_hdcp_status(port);

        if (p_hdcp==NULL)
            continue;

        if (device_count==0)
            p_hdcp->status &= ~KSV_FIFO_READY;
        else
            p_hdcp->status |= KSV_FIFO_READY;
    }

    return 0;
}
