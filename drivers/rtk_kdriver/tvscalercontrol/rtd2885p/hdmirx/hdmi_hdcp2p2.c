/*=============================================================
 * Copyright (c)      Realtek Semiconductor Corporation, 2016
 *
 * All rights reserved.
 *
 *============================================================*/

/*======================= Description ============================
 *
 * file:        hdcp_2p2.c
 *
 * author:  Scarlet Witch, Wanda Maximoff
 * date:
 * version:     3.0
 *
 *============================================================*/

/*========================Header Files============================*/
#include "hdmi_common.h"
#include "hdmi_reg.h"
#include "hdmi_hdcp.h"
#include "hdmi_vfe_config.h"

#include <tvscalercontrol/hdcp2_2/hdcp2_hal.h>
#include <tvscalercontrol/hdcp2_2/hdcp2_session.h>
#include <tvscalercontrol/hdcp2_2/hdcp2_interface.h>
#include <hdcp/hdcp2_optee.h>
#include <rbus/sys_reg_reg.h>
extern HDMI_PORT_INFO_T hdmi_rx[HDMI_PORT_TOTAL_NUM];
extern HDMI_SPECIFIC_DEVICE_E lib_hdmi_get_specific_device(unsigned char nport);
#define DEFAULT_HDCP2_MSG_TIMEOUT           3000
unsigned int g_hdmi_hdcp2_wait_msg_timeout = DEFAULT_HDCP2_MSG_TIMEOUT;

#if !IS_ENABLED(CONFIG_SUPPORT_SCALER)
static unsigned char curr_port = 0;
#define get_current_hdmi_port()         (curr_port)
#define set_current_hdmi_port(x)        do { curr_port = x; } while(0)
#define h2MessagePoll(x, y)
#define h2MessageParse(msg, len)
#define rtk_hal_hdcp2_VFE_HDMI_WriteHDCP22(data, len)
#define hdcp2_tasklet_processing()
#else
extern int rtk_hal_hdcp2_VFE_HDMI_WriteHDCP22(unsigned char *pdata, unsigned int size);
#endif
#ifndef BUILD_QUICK_SHOW
extern struct list_head hdcp2p2_list;
extern struct workqueue_struct *hdcp2p2_wq;
static void hdcp2p2_handler(struct work_struct *work);
#endif

/**********************************************************************************************
*
*   Marco or Definitions
*
**********************************************************************************************/
#define HDCP2_SEND_PAIRINGINFO_BY_IRQ_RD    0

//For HDCP2_2  TEST 20140815 cloud wu
#define HDCP_RXstatus_addr      0x70
#define IDEL                    0   //TX=>RX
#define AKE_INIT                2   //TX=>RX
#define AKE_Send_Cert           3   //RX=>TX
#define AKE_NO_Stored_Km        4   //TX=>RX
#define AKE_Stored_Km           5   //TX=>RX
#define AKE_Send_H_prime        7   //RX=>TX
#define AKE_Send_Pariring_Info  8   //RX=>TX
#define LC_Init                 9   //TX=>RX
#define LC_Send_L_prime         10  //RX=>TX
#define SKE_Send_Eks            11  //TX=>RX
//repeater
#define REP_AUTH_SEND_RXID_LIST		12 //RX=>TX
#define	REPEATERAUTH_SEND_ACK		15 //TX=>RX
#define	REPEATERAUTH_STREAM_MANAGE	16 //RX=>TX
#define	REPEATERAUTH_STREAM_READY	17 //TX=>RX




//RX send FSM
#define  RX_FSM_SEND_START      1
#define  RX_FSM_WAIT            2
#define  RX_FSM_FINISH          3


//=============================HDCP 2.2 OFFSET =========================================//
#define Write_Message_offset    0x60
#define Rxstatus_offset         0x70
#define Read_Message_offset     0x80


/**********************************************************************************************
*
*   Const Declarations
*
**********************************************************************************************/


/**********************************************************************************************
*
*   Variables
*
**********************************************************************************************/
unsigned char g_TX_msg_id[HDMI_PORT_TOTAL_NUM]; //tx->rx msg first byte,the id num
unsigned char g_RX_msg_id[HDMI_PORT_TOTAL_NUM]; //rx->tx msg first byte,the id num
unsigned char g_TX_msg_buf[HDMI_PORT_TOTAL_NUM][540] ; //tx->rx msg buf
#ifdef CONFIG_OPTEE_HDCP2
unsigned char g_RX_msg_buf[HDMI_PORT_TOTAL_NUM][540] ; //rx->tx msg buf
#endif
/**********************************************************************************************
*
*   Funtion Declarations
*
**********************************************************************************************/

/**********************************************************************************************
*
*   Function Body
*
**********************************************************************************************/

const char* _hdcp2_msg_str(unsigned char msg_id)
{
    switch(msg_id) {
    case AKE_INIT:          return "AKE_Init";
    case AKE_Send_Cert:     return "AKE_Send_Cert";
    case AKE_NO_Stored_Km:  return "AKE_NO_Stored_Km";  
    case AKE_Stored_Km:     return "AKE_Stored_Km";
    case AKE_Send_H_prime:  return "AKE_Send_H_prime";
    case AKE_Send_Pariring_Info:    return "AKE_Send_Pariring_Info";
    case LC_Init:           return "LC_Init";
    case LC_Send_L_prime:   return "LC_Send_L_prime";
    case SKE_Send_Eks:      return "SKE_Send_Eks";
    case REP_AUTH_SEND_RXID_LIST:      return "REP_AUTH_SEND_RXID_LIST";
    case REPEATERAUTH_SEND_ACK:      return "REPEATERAUTH_SEND_ACK";
    case REPEATERAUTH_STREAM_MANAGE:      return "REPEATERAUTH_STREAM_MANAGE";
    case REPEATERAUTH_STREAM_READY:      return "REPEATERAUTH_STREAM_READY";
    default:                return "Unknown";
    }
}

#ifdef CONFIG_OPTEE_HDCP2
static void spu_printBytes(char *module, unsigned char *message, unsigned int len)
{
    if(HDCP_FLOW_PRINT_FLAG){
        unsigned int i;
        char buff[128] = {0};
        HDCP_FLOW_PRINTF("%s\n", module);
        for (i = 0; i < len; i++) {
            if ((i & 0xF) == 0) {
                HDCP_FLOW_PRINTF("%s\n", buff);
                if(snprintf(buff, 128, " %04x | ", i) < 0)
                    return;
            }
            if(snprintf(buff + strlen(buff), 128 - strlen(buff), " %02x ", message[i]) < 0)
                return;
        }
        HDCP_FLOW_PRINTF("%s\n", buff);
    }
}
#endif

void lib_hdmi_hdcp22_debug(unsigned char nport)
{
    /*
    hdmi_mask(0xb800d028,~(_BIT4|_BIT5|_BIT6|_BIT7|_BIT8|_BIT9), (0x2d)<<4);         // sel  hdmi rx debug
    hdmi_mask(0xb800d1ac,~(_BIT4|_BIT5|_BIT6|_BIT7), (0x1)<<4);      // sel  hdmi_MHL top debug sel mhl hdmi dfe cbus debug
    hdmi_mask(0xb80008c0,~(_BIT0|_BIT1|_BIT2|_BIT3|_BIT4|_BIT5|_BIT6|_BIT7), (0x6));      // test pin mux
    hdmi_mask(0xb80008c0,~(_BIT0|_BIT1|_BIT2|_BIT3|_BIT4|_BIT5|_BIT6|_BIT7), (0x6));      // test pin mux

    //pin mux
    hdmi_mask(0xb8000800,~(_BIT28|_BIT29|_BIT30|_BIT31), (0xe)<<28);
    hdmi_mask(0xb800081c,~(_BIT28|_BIT29|_BIT30|_BIT31), (0xe)<<28);
    hdmi_mask(0xb800081c,~(_BIT12|_BIT13|_BIT14|_BIT15), (0xe)<<12);
#ifndef ENABLE_FUNCTIONAL_TEST
    //0xb8060208 used by MT board GPIO, do not touch it
    hdmi_mask(0xb8060208,~(_BIT20|_BIT21|_BIT22|_BIT23), (0xe)<<20);
#endif
    hdmi_mask(0xb8000814,~(_BIT7|_BIT6|_BIT5|_BIT4), (0xe)<<4);
    hdmi_mask(0xb8000818,~(_BIT15|_BIT14|_BIT13|_BIT12), (0xe)<<12);
    */
}

char lib_hdmi_hdcp22_get_write_msg(unsigned char nport)
{
    if (HDMI_STB_HDCP_2p2_SR0_get_irq_wr_msg_done(hdmi_in(HDMI_STB_HDCP_2p2_SR0_reg)))
        return 1;
    else
        return 0;
}

char lib_hdmi_hdcp22_get_read_msg(unsigned char nport)
{
    if (HDMI_STB_HDCP_2p2_SR0_get_irq_rd_msg_done(hdmi_in(HDMI_STB_HDCP_2p2_SR0_reg)))
        return 1;
    else
        return 0;
}

void lib_hdmi_hdcp22_set_rd_msg_done_irq(unsigned char nport, unsigned char enable)
{
    hdmi_mask(HDMI_STB_HDCP_2p2_SR1_reg,~(HDMI_STB_HDCP_2p2_SR1_irq_rd_msg_done_en_mask), HDMI_STB_HDCP_2p2_SR1_irq_rd_msg_done_en(enable));
}

void lib_hdmi_hdcp22_set_wr_msg_done_irq_en(unsigned char nport, unsigned char enable)
{
    hdmi_mask(HDMI_STB_HDCP_2p2_SR1_reg,~(HDMI_STB_HDCP_2p2_SR1_irq_wr_msg_done_en_mask), HDMI_STB_HDCP_2p2_SR1_irq_wr_msg_done_en(enable));
}

unsigned char lib_hdmi_hdcp22_is_wr_msg_done_irq_en(unsigned char nport)
{
    return HDMI_STB_HDCP_2p2_SR1_get_irq_wr_msg_done_en(hdmi_in(HDMI_STB_HDCP_2p2_SR1_reg));
}

void lib_hdmi_hdcp22_clear_read_status(unsigned char nport)
{
    hdmi_out(HDMI_STB_HDCP_2p2_SR0_reg,HDMI_STB_HDCP_2p2_SR0_irq_rd_msg_done_mask);
}

void lib_hdmi_hdcp22_hw_cipher_setting(unsigned char nport, unsigned char *lc128)
{
    //initial ks value
    //lc 128
    hdmi_mask(HDMI_HDCP_2p2_lc0_reg,~HDMI_HDCP_2p2_lc0_lc_mask, (lc128[15]|(lc128[14]<<8)|(lc128[13]<<16)|(lc128[12]<<24)));
    hdmi_mask(HDMI_HDCP_2p2_lc1_reg,~HDMI_HDCP_2p2_lc1_lc_mask, (lc128[11]|(lc128[10]<<8)|(lc128[9]<<16)|(lc128[8]<<24)));
    hdmi_mask(HDMI_HDCP_2p2_lc2_reg,~HDMI_HDCP_2p2_lc2_lc_mask, (lc128[7]|(lc128[6]<<8)|(lc128[5]<<16)|(lc128[4]<<24)));
    hdmi_mask(HDMI_HDCP_2p2_lc3_reg,~HDMI_HDCP_2p2_lc3_lc_mask, (lc128[3]|(lc128[2]<<8)|(lc128[1]<<16)|(lc128[0]<<24)));
}

int lib_hdmi_hdcp22_set_ks(unsigned char nport, unsigned char* bKs)
{
    int i = 0;

    if (HDMI_STB_HDCP_2p2_SR0_get_state(hdmi_in(HDMI_STB_HDCP_2p2_SR0_reg))==0x10)
    {
        HDCP_WARN("HDMI[p%d] Something Wrong !!! Set HDCP22 Ks under Authen State is not allowed (STB_HDCP_2p2_CR=%08x, STB_HDCP_2p2_SR0=%08x)\n",
            nport, hdmi_in(HDMI_STB_HDCP_2p2_CR_reg), hdmi_in(HDMI_STB_HDCP_2p2_SR0_reg));
    }

    //initial ks value
    hdmi_mask(HDMI_HDCP_2p2_ks0_reg,~HDMI_HDCP_2p2_ks0_ks_mask, ((*(bKs+15))|((*(bKs+14))<<8)|((*(bKs+13))<<16)|((*(bKs+12))<<24)));
    hdmi_mask(HDMI_HDCP_2p2_ks1_reg,~HDMI_HDCP_2p2_ks1_ks_mask, ((*(bKs+11))|((*(bKs+10))<<8)|((*(bKs+9))<<16)|((*(bKs+8))<<24)));
    hdmi_mask(HDMI_HDCP_2p2_ks2_reg,~HDMI_HDCP_2p2_ks2_ks_mask, ((*(bKs+7))|((*(bKs+6))<<8)|((*(bKs+5))<<16)|((*(bKs+4))<<24)));
    hdmi_mask(HDMI_HDCP_2p2_ks3_reg,~HDMI_HDCP_2p2_ks3_ks_mask, ((*(bKs+3))|((*(bKs+2))<<8)|((*(bKs+1))<<16)|((*bKs)<<24)));

    hdmi_mask(HDMI_STB_HDCP_2p2_CR_reg,~HDMI_STB_HDCP_2p2_CR_ks_done_mask, HDMI_STB_HDCP_2p2_CR_ks_done_mask);

    // Check KS done flag : KS done flag will be cleared by HW automatically after update
    while(HDMI_STB_HDCP_2p2_CR_get_ks_done(hdmi_in(HDMI_STB_HDCP_2p2_CR_reg)) && i++<100)
    {
        if ((i & 0xF)==1) {
            HDCP_FLOW_PRINTF("[HDCP2.2]KS_Setting bKs wait Ks done (STB_HDCP_2p2_CR=%08x, STB_HDCP_2p2_SR0=%08x) cnt=%d\n", hdmi_in(HDMI_STB_HDCP_2p2_CR_reg), hdmi_in(HDMI_STB_HDCP_2p2_SR0_reg), i);
        }
        udelay(10);
    }

    HDCP_FLOW_PRINTF("[HDMIp%d] KS_Setting bKs = 0x%x , 0x%x ,0x%x ,0x%x ,0x%x ,0x%x ,0x%x ,0x%x (poll_cnt=%d)\n",
        nport, (*bKs),*(bKs+1),*(bKs+2),*(bKs+3),*(bKs+4),*(bKs+5),*(bKs+6),*(bKs+7), i);

    // Error handling when set Ks failed : clear ks done flag and report error
    if (HDMI_STB_HDCP_2p2_CR_get_ks_done(hdmi_in(HDMI_STB_HDCP_2p2_CR_reg)))
    {
        HDCP_WARN("[HDMIp%d] set KS failed - wait ks done timeout (%d)(STB_HDCP_2p2_CR=%08x, STB_HDCP_2p2_SR0=%08x)!!!!!!\n",
            nport, i, hdmi_in(HDMI_STB_HDCP_2p2_CR_reg), hdmi_in(HDMI_STB_HDCP_2p2_SR0_reg));
        hdmi_mask(HDMI_STB_HDCP_2p2_CR_reg,~HDMI_STB_HDCP_2p2_CR_ks_done_mask, 0);
        return -1;
    }

    return 0;
}

void lib_hdmi_hdcp22_riv_setting(unsigned char nport, unsigned char* bRiv)
{
    hdmi_mask(HDMI_HDCP_2p2_riv0_reg,~HDMI_HDCP_2p2_riv0_riv_mask, (*(bRiv+7)|(*(bRiv+6)<<8)|(*(bRiv+5)<<16)|(*(bRiv+4)<<24)));
    hdmi_mask(HDMI_HDCP_2p2_riv1_reg,~HDMI_HDCP_2p2_riv1_riv_mask, (*(bRiv+3)|(*(bRiv+2)<<8)|(*(bRiv+1)<<16)|((*bRiv)<<24)));
    HDCP_FLOW_PRINTF("[HDCP2.2]Riv_Setting bRiv = 0x%x , 0x%x ,0x%x ,0x%x ,0x%x ,0x%x ,0x%x ,0x%x , \n",(*bRiv),*(bRiv+1),*(bRiv+2),*(bRiv+3),*(bRiv+4),*(bRiv+5),*(bRiv+6),*(bRiv+7));
}

void lib_hdmi_hdcp22_init(unsigned char nport,unsigned char *uc_lc128)
{
    HDCP_FLOW_PRINTF("[HDCP2.2] lib_hdcp22_init\n");
    // enable hdcp 2.2 HW
    //hdmi_mask(HDMI_HDCP_2p2_CR_reg,~HDMI_HDCP_2p2_CR_hdcp_2p2_en_mask, 0);
    hdmi_mask(HDMI_STB_HDCP_2p2_SR1_reg,~HDMI_STB_HDCP_2p2_SR1_irq_wr_msg_done_en_mask, HDMI_STB_HDCP_2p2_SR1_irq_wr_msg_done_en_mask);
    // open write & read msg irq for flag
    //hdmi_mask(HDCP_2p2_SR1_reg,~(HDCP_2p2_SR1_irq_rd_msg_done_en_mask|HDCP_2p2_SR1_irq_wr_msg_done_en_mask), (0));

    if (uc_lc128)
        lib_hdmi_hdcp22_hw_cipher_setting(nport, uc_lc128);

    //hdmi_mask(INTERRUPT_INT_CTRL_VCPU_VADDR, ~(_BIT2), 0);  // stop disp I interrupt

    //HDCP_Debug();

    //2106.08.08 willychou add HDCP2p2
    //STD flow : hdcp2_init_module =>vfe_suspend_std  =>vfe_resume_std =>vfe_hdmi_drv_init => vfe_hdmi_drv_open => vfe_hdmi_drv_connect
//              hdmi_mask(SYS_REG_INT_CTRL_MEMC_reg,~(SYS_REG_INT_CTRL_MEMC_hdmi_irq_scpu_routing_en_mask),SYS_REG_INT_CTRL_MEMC_hdmi_irq_scpu_routing_en_mask);
    rtd_outl(SYS_REG_INT_CTRL_MEMC_reg, (SYS_REG_INT_CTRL_MEMC_hdmi_irq_scpu_routing_en_mask|SYS_REG_INT_CTRL_MEMC_write_data_mask));
}

void lib_hdmi_hdcp22_write_data_to_tx(unsigned char nport, unsigned char* bSendData, unsigned short wLen)
{
    unsigned short i;
    unsigned long  flags;

    lock_hdcp_sram(flags);

    for(i=0;i<wLen;i++) {
        hdmi_out(HDMI_STB_HDCP_MSAP_reg,i);
        hdmi_out(HDMI_STB_HDCP_MSDP_reg,*(bSendData+i));
    }

    unlock_hdcp_sram(flags);

    lib_hdmi_hdcp22_update_message_size(nport, wLen);
    HDCP_FLOW_PRINTF("[HDCP2.2] Hdmi_HDCP2_2_Write_Data_to_TX!!!! wlen =%d   date 0 =0x%x ,date1 = 0x%x , date2 =0x%x \n",wLen,*(bSendData),*(bSendData+1),*(bSendData+2));
}

void lib_hdmi_hdcp22_enable_reauth(unsigned char nport, unsigned char enable)
{
    hdmi_mask(HDMI_STB_HDCP_2p2_CR_reg,~HDMI_STB_HDCP_2p2_CR_enable_reauth_mask, (enable) ? HDMI_STB_HDCP_2p2_CR_enable_reauth_mask : 0);
}

unsigned char lib_hdmi_hdcp22_get_enable_reauth(unsigned char nport)
{
    return HDMI_STB_HDCP_2p2_CR_get_enable_reauth(hdmi_in(HDMI_STB_HDCP_2p2_CR_reg));
}

void lib_hdmi_hdcp22_update_message_size(unsigned char nport, unsigned short wLen)
{
    HDMI_HDCP_ST* p_hdcp = newbase_hdcp_get_hdcp_status(nport);
    unsigned long  flags;

    if (newbase_hdmi_phy_port_status(nport)==HDMI_PHY_PORT_NOTUSED || p_hdcp==NULL)
        return;

    lock_hdcp_sram(flags);

    hdmi_out(HDMI_STB_HDCP_AP_reg, HDCP_RXstatus_addr);
    hdmi_out(HDMI_STB_HDCP_DP_reg, (wLen&0xFF));
    hdmi_out(HDMI_STB_HDCP_AP_reg, HDCP_RXstatus_addr+1);

    if (p_hdcp->hdcp2_reauth)
        hdmi_out(HDMI_STB_HDCP_DP_reg, 0x8 | ((wLen>>8)&0x3));
    else
        hdmi_out(HDMI_STB_HDCP_DP_reg, ((wLen>>8)&0x3));

    unlock_hdcp_sram(flags);
}

void lib_hdmi_hdcp22_set_ready(unsigned char nport, unsigned short wLen)
{
    HDMI_HDCP_ST* p_hdcp = newbase_hdcp_get_hdcp_status(nport);
    unsigned long  flags;

    if (newbase_hdmi_phy_port_status(nport)==HDMI_PHY_PORT_NOTUSED || p_hdcp==NULL)
        return;

    lock_hdcp_sram(flags);

    hdmi_out(HDMI_STB_HDCP_AP_reg, HDCP_RXstatus_addr);
    hdmi_out(HDMI_STB_HDCP_DP_reg, (wLen&0xFF));
    hdmi_out(HDMI_STB_HDCP_AP_reg, HDCP_RXstatus_addr+1);
    hdmi_out(HDMI_STB_HDCP_DP_reg, 0x4 | ((wLen>>8)&0x3));

    unlock_hdcp_sram(flags);
}

unsigned int lib_hdmi_hdcp22_get_state(unsigned char nport)
{
    return HDMI_STB_HDCP_2p2_SR0_get_state(hdmi_in(HDMI_STB_HDCP_2p2_SR0_reg));
}

void lib_hdmi_hdcp22_set_state(unsigned char nport, unsigned int new_state)
{
    HDMI_HDCP_ST* p_hdcp = newbase_hdcp_get_hdcp_status(nport);
    unsigned int state;
    if (newbase_hdmi_phy_port_status(nport)==HDMI_PHY_PORT_NOTUSED || p_hdcp==NULL)
        return;

    state = lib_hdmi_hdcp22_get_state(nport);

    if (state !=new_state)
    {
        hdmi_mask(HDMI_STB_HDCP_2p2_CR_reg, ~(HDMI_STB_HDCP_2p2_CR_switch_state_mask | HDMI_STB_HDCP_2p2_CR_apply_state_mask),
                                              HDMI_STB_HDCP_2p2_CR_switch_state(new_state) | HDMI_STB_HDCP_2p2_CR_apply_state_mask);
        udelay(10);
        hdmi_mask(HDMI_STB_HDCP_2p2_CR_reg, ~(HDMI_STB_HDCP_2p2_CR_switch_state_mask | HDMI_STB_HDCP_2p2_CR_apply_state_mask), 0);
    }
    HDCP_WARN("HDMI[p%d] HDCP22 State Changed %x -> %x (real=%x)\n", nport, state, new_state, lib_hdmi_hdcp22_get_state(nport));
}
void lib_hdmi_hdcp22_get_rx_status(unsigned char nport, unsigned char RxStatus[2])
{
    unsigned long  flags;
    RxStatus[0] = 0;
    RxStatus[1] = 0;

    if (newbase_hdmi_phy_port_status(nport)==HDMI_PHY_PORT_NOTUSED)
        return;

    lock_hdcp_sram(flags);
    hdmi_out(HDMI_STB_HDCP_AP_reg, HDCP_RXstatus_addr);

    RxStatus[0] = hdmi_in(HDMI_STB_HDCP_DP_reg);

    hdmi_out(HDMI_STB_HDCP_AP_reg, HDCP_RXstatus_addr+1);
    RxStatus[1] = hdmi_in(HDMI_STB_HDCP_DP_reg);

    unlock_hdcp_sram(flags);
}

void lib_hdmi_hdcp22_wait_tx_read_finish(unsigned char nport)
{
    while(!lib_hdmi_hdcp22_get_read_msg(nport));

    hdmi_out(HDMI_STB_HDCP_2p2_SR0_reg,HDMI_STB_HDCP_2p2_SR0_irq_rd_msg_done_mask);
    HDCP_FLOW_PRINTF("[HDCP2.2] TX read finish !!!!!     \n" );

}

void lib_hdmi_hdcp22_aes_engine_sel(unsigned char nport, unsigned char hd21)
{
    HDCP_INFO("HDMI[p%d] HDCP22 AES Engine Sel =%s\n", nport, (hd21) ? "HD21" : "HD20");
    hdmi_mask(HDMI_HDCP_2p2_CR_reg,~(HDMI_HDCP_2p2_CR_aes_engine_sel_mask), (hd21) ? HDMI_HDCP_2p2_CR_aes_engine_sel_mask : 0);  //select AES engine target
}

void lib_hdmi_hdcp22_force_cipher(unsigned char nport)
{
    hdmi_mask(HDMI_STB_HDCP_2p2_CR_reg,~(HDMI_STB_HDCP_2p2_CR_switch_state_mask), HDMI_STB_HDCP_2p2_CR_switch_state(0x10));  //set authenticated
    hdmi_mask(HDMI_STB_HDCP_2p2_CR_reg,~(HDMI_STB_HDCP_2p2_CR_apply_state_mask), HDMI_STB_HDCP_2p2_CR_apply_state_mask);
}

#ifdef CONFIG_OPTEE_HDCP2

void spu_print(unsigned char *message, int length)
{
    int i;
    for (i = 0; i < length; i++)
        rtd_pr_hdcp_emerg("%x ", message[i]);
    rtd_pr_hdcp_emerg("\n");
}


void lib_hdmi_hdcp22_get_msg_command_optee(unsigned char nport)
{
    HDMI_HDCP_ST* p_hdcp = newbase_hdcp_get_hdcp_status(nport);
    unsigned int excepted_timeout = 0;
    unsigned char RxStatus[2];
    unsigned char bLen = 0, i = 0;    
    unsigned int rLen = 0;
    
    if (lib_hdmi_hdcp22_get_write_msg(nport))  //GET TX mesage
    {
        hdmi_out(HDMI_STB_HDCP_2p2_SR0_reg,HDMI_STB_HDCP_2p2_SR0_irq_wr_msg_done_mask);

        hdmi_out(HDMI_STB_HDCP_MSAP_reg,0);
        g_TX_msg_id[nport] = hdmi_in(HDMI_STB_HDCP_MSDP_reg);
        g_TX_msg_buf[nport][0]= g_TX_msg_id[nport];
        HDCP_FLOW_PRINTF("[HDCP2.2] bmsg_id[%d]  =%x (%s) \n",
            nport, g_TX_msg_id[nport], _hdcp2_msg_str(g_TX_msg_id[nport]));

        //g_Rx_Send_State[nport] =  RX_FSM_SEND_START;
        switch (g_TX_msg_id[nport])
        {
        case AKE_INIT:
            newbase_hdmi_hdcp_repeater_send_event(nport, HDCP_REPEATER_EVENT_UPDATE_AKE_INIT);  // notify hdcp repeater that the AKE_INIT is updated

            g_RX_msg_id[nport] = AKE_Send_Cert ;
            bLen = 11;
            rLen = 534;
            newbase_hdmi_switch_hdcp_mode(nport, HDCP22);
            p_hdcp->hdcp2_reauth = 0;
            p_hdcp->hdcp2_state = HDCP2_AKE_INIT;
            p_hdcp->hdcp2_auth_count++;
            p_hdcp->hdcp14_auth_count = 0;
            newbase_hdcp_init_timer(nport);
#if (HDCP2_SEND_PAIRINGINFO_BY_IRQ_RD)
            lib_hdmi_hdcp22_set_rd_msg_done_irq(nport, 0);
#endif
            excepted_timeout = g_hdmi_hdcp2_wait_msg_timeout;
            HDCP_INFO("HDMI[p%d] HDCP2 receives AKE_Init, authen start (round=%d)(STB_HDCP_2p2_CR=%08x, STB_HDCP_2p2_SR0=%08x)\n",
                nport, p_hdcp->hdcp2_auth_count, hdmi_in(HDMI_STB_HDCP_2p2_CR_reg), hdmi_in(HDMI_STB_HDCP_2p2_SR0_reg));

            if (HDMI_STB_HDCP_2p2_CR_get_ks_done(hdmi_in(HDMI_STB_HDCP_2p2_CR_reg)))
            {
                // Note: HDCP22 engine won't able to update Ks when KS done is pending
                // the KS done is normally caused by set Eks while HDCP FSM under authentication state
                // so we force clear this flag when AKE init is detected.
                HDCP_WARN("HDMI[p%d] got AKE_Init, with KS done flag is set !!!! clear ks_done flag to recover\n", nport);
                hdmi_mask(HDMI_STB_HDCP_2p2_CR_reg,~HDMI_STB_HDCP_2p2_CR_ks_done_mask, 0);
            }

            lib_hdmi_hdcp22_aes_engine_sel(nport,(hdmi_rx[nport].hdmi_2p1_en) ? 1 : 0);
            break;

        case AKE_NO_Stored_Km:
            g_RX_msg_id[nport] =  AKE_Send_H_prime;
            bLen =128;
            rLen = 33;
            excepted_timeout = g_hdmi_hdcp2_wait_msg_timeout;
            p_hdcp->hdcp2_state = HDCP2_NO_STOREE_KM;
            HDCP_INFO("HDMI[p%d] HDCP2 receives no stored km (round=%d)(STB_HDCP_2p2_CR=%08x, STB_HDCP_2p2_SR0=%08x)\n",
                nport, p_hdcp->hdcp2_auth_count, hdmi_in(HDMI_STB_HDCP_2p2_CR_reg), hdmi_in(HDMI_STB_HDCP_2p2_SR0_reg));
            break;

        case AKE_Stored_Km:
            g_RX_msg_id[nport] =  AKE_Send_H_prime;
            bLen =32;
            rLen = 33;
            excepted_timeout = g_hdmi_hdcp2_wait_msg_timeout;
            p_hdcp->hdcp2_state = HDCP2_STOREE_KM;
            HDCP_INFO("HDMI[p%d] HDCP2 receives stored km (round=%d)(STB_HDCP_2p2_CR=%08x, STB_HDCP_2p2_SR0=%08x)\n",
                nport, p_hdcp->hdcp2_auth_count, hdmi_in(HDMI_STB_HDCP_2p2_CR_reg), hdmi_in(HDMI_STB_HDCP_2p2_SR0_reg));
            break;

        case LC_Init:
            g_RX_msg_id[nport] =  LC_Send_L_prime;
            bLen =8;
            rLen = 33;
            HDCP_INFO("HDMI[p%d] HDCP2 receives LC_Init (round=%d)(STB_HDCP_2p2_CR=%08x, STB_HDCP_2p2_SR0=%08x)\n",
                nport, p_hdcp->hdcp2_auth_count, hdmi_in(HDMI_STB_HDCP_2p2_CR_reg), hdmi_in(HDMI_STB_HDCP_2p2_SR0_reg));
            excepted_timeout = g_hdmi_hdcp2_wait_msg_timeout;
            break;

        case SKE_Send_Eks:
            bLen =24;
            rLen = 16;
            excepted_timeout = 0;
            p_hdcp->hdcp2_state = HDCP2_SEND_EKS;
            lib_hdmi_hdcp22_get_rx_status(nport, RxStatus);
            HDCP_INFO("HDMI[p%d] HDCP2 receives Eks, authen successed (round=%d) (STB_HDCP_2p2_CR=%08x, STB_HDCP_2p2_SR0=%08x), Status(%02x, %02x) Bit Error(%d,%d)\n",
                nport, p_hdcp->hdcp2_auth_count, hdmi_in(HDMI_STB_HDCP_2p2_CR_reg), hdmi_in(HDMI_STB_HDCP_2p2_SR0_reg),
                RxStatus[0], RxStatus[1],
                hdmi_rx[nport].bch_1bit_err_cnt,
                hdmi_rx[nport].bch_err_cnt);
            break;
        case REPEATERAUTH_SEND_ACK:
            bLen =16;
            rLen = 1;
            HDCP_INFO("HDMI[p%d] HDCP2 receives repeaterauth send ack (round=%d)\n",
                nport, p_hdcp->hdcp2_auth_count);
            break;
        case REPEATERAUTH_STREAM_MANAGE:
            bLen =5;
            rLen = 33;
            HDCP_INFO("HDMI[p%d] HDCP2 receives repeaterauth stream manage (round=%d)\n",
                nport, p_hdcp->hdcp2_auth_count);
            break;
        default:
            HDCP_WARN("HDMI[p%d] HDCP2 receives illegal msg (%02x)(STB_HDCP_2p2_CR=%08x, STB_HDCP_2p2_SR0=%08x)\n", nport, g_TX_msg_id[nport], hdmi_in(HDMI_STB_HDCP_2p2_CR_reg), hdmi_in(HDMI_STB_HDCP_2p2_SR0_reg));
            return;
        }

        for (i = 1; i <= bLen; i++)
            g_TX_msg_buf[nport][i] = hdmi_in(HDMI_STB_HDCP_MSDP_reg);


        // for HW Cipher setting
        if (g_TX_msg_id[nport] == SKE_Send_Eks) 
            lib_hdmi_hdcp22_riv_setting(nport, (&(g_TX_msg_buf[nport][17])));  // set Riv address      

        //re-compute bLen
        if(g_TX_msg_id[nport] == REPEATERAUTH_STREAM_MANAGE)
        {
            unsigned short k;
            k = ((g_TX_msg_buf[nport][bLen-1]<<8)) | ((g_TX_msg_buf[nport][bLen]<<0) & 0xff);
            bLen += 2*k;

            for (i = 6; i <= bLen; i++)
                g_TX_msg_buf[nport][i] = hdmi_in(HDMI_STB_HDCP_MSDP_reg);

            if (k == 1)
                rtk_hdcp2_set_content_type(nport, g_TX_msg_buf[nport][bLen]);
            else
                HDCP_WARN("content stream k!=1\n");
        }

        optee_hdcp2_main(HDMI_HDCP2_MODE, HDCP2_CMD_HANDLE_MESSAGE, nport, g_TX_msg_buf[nport], bLen+1, g_RX_msg_buf[nport], rLen);

        if(g_TX_msg_id[nport] == SKE_Send_Eks)
        {
            lib_hdmi_hdcp22_set_ks(nport,g_RX_msg_buf[nport]);
            udelay(100);
            lib_hdmi_hdcp22_set_state(nport, HDCP2_STATE_AUTHENTICATED);  // set to authentication state
        }
        else if(g_TX_msg_id[nport] ==  AKE_NO_Stored_Km)
        {
            lib_hdmi_hdcp22_clear_read_status(nport);

#if HDCP2_SEND_PAIRINGINFO_BY_IRQ_RD
            lib_hdmi_hdcp22_set_rd_msg_done_irq(nport, 1);
#endif

            HDCP_FLOW_PRINTF("Flag =%x  \n",(lib_hdmi_hdcp22_get_read_msg(nport)));
            
            newbase_hdmi_hdcp22_write_data_to_tx(nport, (unsigned char*)g_RX_msg_buf[nport], rLen);            
        }
        else if(g_TX_msg_id[nport] ==  REPEATERAUTH_SEND_ACK)
        {
            if(!g_RX_msg_buf[nport][0]) //repeaterauth_send_ack vprime compare fail,reauth hdcp2
                newbase_hdmi_hdcp22_set_reauth(nport);
        }
        else
        {
            newbase_hdmi_hdcp22_write_data_to_tx(nport, (unsigned char*)g_RX_msg_buf[nport], rLen);  
        }
    }
    spu_printBytes("HDCP2 tx msg", g_TX_msg_buf[nport], bLen+1);
    spu_printBytes("HDCP2 rx msg", g_RX_msg_buf[nport], rLen);
    if (excepted_timeout) {
        HDCP_FLOW_PRINTF("[HDCP2.2] excepted timeout = %d ms\n", excepted_timeout);
        p_hdcp->hdcp2_wait_mesage_timout = jiffies + msecs_to_jiffies(excepted_timeout);
    }
    else
        p_hdcp->hdcp2_wait_mesage_timout = 0;
}

void lib_hdmi_hdcp22_send_msg_command_optee(unsigned char nport)
{
#if (!HDCP2_SEND_PAIRINGINFO_BY_IRQ_RD)
    unsigned int current_ms =  hdmi_get_system_time_ms();
#endif

    //if(Get_Hdmi_hdcp_2_2_Read_Message())
    {
#if (HDCP2_SEND_PAIRINGINFO_BY_IRQ_RD)
        if ((g_RX_msg_id[nport] == AKE_Send_H_prime)&&(g_TX_msg_id[nport] ==  AKE_NO_Stored_Km) && lib_hdmi_hdcp22_get_read_msg(nport))
#else
        if ((g_RX_msg_id[nport] == AKE_Send_H_prime)&&(g_TX_msg_id[nport] ==  AKE_NO_Stored_Km ))
#endif
        {
            optee_hdcp2_main(HDMI_HDCP2_MODE, HDCP2_CMD_POLLING_MESSAGE, nport, g_TX_msg_buf[nport], 17, g_RX_msg_buf[nport], 17);

#if (!HDCP2_SEND_PAIRINGINFO_BY_IRQ_RD)
            //wait h prime TX read finish
            while(HDMI_ABS(hdmi_get_system_time_ms(), current_ms)  <= 1000)
            {
                usleep_range(20*1000, 20*1000);
                if(lib_hdmi_hdcp22_get_read_msg(nport))
                    break;
            }

            if (HDMI_ABS(hdmi_get_system_time_ms(), current_ms) > 1000)
                HDMI_WARN("[HDCP2.2][ERR] Tx read hprime timeout\n");
#endif

            newbase_hdmi_hdcp22_write_data_to_tx(nport, (unsigned char*)g_RX_msg_buf[nport], 17);
            //g_Rx_Send_State[nport] = RX_FSM_SEND_START;
            g_RX_msg_id[nport] = AKE_Send_Pariring_Info;
        }
        else if(rtk_hdcp2_get_repeater(nport)&&(g_TX_msg_id[nport] ==  SKE_Send_Eks ))
        {
            newbase_hdmi_hdcp_repeater_send_event(nport, HDCP_REPEATER_EVENT_HDCP2_EKS);  // notify hdcp repeater that the EKS is updated
        }
        else
        {
            //g_Rx_Send_State[nport] = RX_FSM_FINISH ;
        }
        hdmi_out(HDMI_STB_HDCP_2p2_SR0_reg,HDMI_STB_HDCP_2p2_SR0_irq_rd_msg_done_mask);
        HDCP_FLOW_PRINTF("[HDCP2.2] Tx Read finish !!!!!     \n" );
    }
}


#define lib_hdmi_hdcp22_get_msg_command(nport)  lib_hdmi_hdcp22_get_msg_command_optee(nport)
#define lib_hdmi_hdcp22_send_msg_command(nport)  lib_hdmi_hdcp22_send_msg_command_optee(nport)

#else

void lib_hdmi_hdcp22_get_msg_command(unsigned char nport)
{
    HDMI_HDCP_ST* p_hdcp = newbase_hdcp_get_hdcp_status(nport);
    unsigned char bLen, i;
    unsigned int excepted_timeout = 0;
    
    if (lib_hdmi_hdcp22_get_write_msg(nport))  //GET TX mesage
    {
        hdmi_out(HDMI_STB_HDCP_2p2_SR0_reg,HDMI_STB_HDCP_2p2_SR0_irq_wr_msg_done_mask);

        hdmi_out(HDMI_STB_HDCP_MSAP_reg,0);
        g_TX_msg_id[nport] = hdmi_in(HDMI_STB_HDCP_MSDP_reg);
        g_TX_msg_buf[nport][0]= g_TX_msg_id[nport];
        HDCP_FLOW_PRINTF("[HDCP2.2] bmsg_id[%d] =%x (%s) \n",
            nport, g_TX_msg_id[nport], _hdcp2_msg_str(g_TX_msg_id[nport]));

        switch (g_TX_msg_id[nport])
        {
        case AKE_INIT:
            g_RX_msg_id[nport] =  AKE_Send_Cert ;//AKE_Send_Cert ;
            bLen = 11;
#ifndef CONFIG_HDCP2_USE_DEFAULT_HDCP_KEY
            //spu_GetHdmiKey(hdmi_key_data);
            //rtk_hal_hdcp2_VFE_HDMI_WriteHDCP22(hdmi_key_data,HDMI_HDCP2_KEY_LENGTH);
#else
            hdcp2_load_from_flash();
#endif

            newbase_hdmi_switch_hdcp_mode(nport, HDCP22);
            newbase_hdmi_power_saving_wakeup(nport, PS_WAKEUP_HDCP_REQUEST);
            p_hdcp->hdcp2_reauth = 0;
            p_hdcp->hdcp2_state = HDCP2_AKE_INIT;
            p_hdcp->hdcp2_auth_count++;
            p_hdcp->hdcp14_auth_count=0;
            newbase_hdcp_init_timer(nport);
#if (HDCP2_SEND_PAIRINGINFO_BY_IRQ_RD)
            lib_hdmi_hdcp22_set_rd_msg_done_irq(nport, 0);
#endif
            excepted_timeout = g_hdmi_hdcp2_wait_msg_timeout;
            HDCP_INFO("HDMI[p%d] HDCP2 receives AKE_Init, authen start (round=%d)\n",
                nport, p_hdcp->hdcp2_auth_count);

            lib_hdmi_hdcp22_aes_engine_sel(nport,(hdmi_rx[nport].hdmi_2p1_en) ? 1 : 0);
            break;

        case AKE_NO_Stored_Km:
            g_RX_msg_id[nport] =  AKE_Send_H_prime;
            bLen =128;
            excepted_timeout = g_hdmi_hdcp2_wait_msg_timeout;
            p_hdcp->hdcp2_state = HDCP2_NO_STOREE_KM;
            HDCP_INFO("HDMI[p%d] HDCP2 receives No stored km (round=%d)\n",
                nport, p_hdcp->hdcp2_auth_count);
            break;

        case AKE_Stored_Km:
            g_RX_msg_id[nport] =  AKE_Send_H_prime;
            bLen =32;
            excepted_timeout = g_hdmi_hdcp2_wait_msg_timeout;
            p_hdcp->hdcp2_state = HDCP2_STOREE_KM;
            HDCP_INFO("HDMI[p%d] HDCP2 receives stored km (round=%d)\n",
                nport, p_hdcp->hdcp2_auth_count);
            break;

        case LC_Init:
            g_RX_msg_id[nport] =  LC_Send_L_prime;//LC_Send_L_prime ;
            HDCP_INFO("HDMI[p%d] HDCP2 receives LC_Init (round=%d)\n",
                nport, p_hdcp->hdcp2_auth_count);
            bLen =8;
            excepted_timeout = g_hdmi_hdcp2_wait_msg_timeout;
            break;

        case SKE_Send_Eks:
            bLen =24;
            p_hdcp->hdcp2_state = HDCP2_SEND_EKS;
            HDCP_INFO("HDMI[p%d] HDCP2 receives Eks, authen successed (round=%d)\n",
                nport, p_hdcp->hdcp2_auth_count);
            break;
            
        default:
            HDCP_WARN("HDMI[p%d] HDCP2 receives illegal msg (%02x)\n", nport, g_TX_msg_id[nport]);
            return;
        }

        for (i = 1; i <= bLen; i++)
            g_TX_msg_buf[nport][i] = hdmi_in(HDMI_STB_HDCP_MSDP_reg);
            
        // for HW Cipher setting
        if (g_TX_msg_id[nport] == SKE_Send_Eks) 
            lib_hdmi_hdcp22_riv_setting(nport, (&(g_TX_msg_buf[nport][17])));  // set Riv address

        //hdcp2.2 sw parse the message
        h2MessageParse(g_TX_msg_buf[nport], bLen);
    }
    if (excepted_timeout) {
        HDCP_FLOW_PRINTF("[HDCP2.2] excepted timeout = %d ms\n", excepted_timeout);
        p_hdcp->hdcp2_wait_mesage_timout = jiffies + msecs_to_jiffies(excepted_timeout);
    }
    else
        p_hdcp->hdcp2_wait_mesage_timout = 0;
}


void lib_hdmi_hdcp22_send_msg_command(unsigned char nport)
{
    //if(Get_Hdmi_hdcp_2_2_Read_Message())
    {
#if (HDCP2_SEND_PAIRINGINFO_BY_IRQ_RD)
        if ((g_RX_msg_id[nport] == AKE_Send_H_prime)&&(g_TX_msg_id[nport] ==  AKE_NO_Stored_Km) && lib_hdmi_hdcp22_get_read_msg(nport))
#else
        if ((g_RX_msg_id[nport] == AKE_Send_H_prime)&&(g_TX_msg_id[nport] ==  AKE_NO_Stored_Km ))
#endif
        {
            h2MessagePoll(NULL, 0);
            g_RX_msg_id[nport] = AKE_Send_Pariring_Info;
        }
        else
        {
            //g_Rx_Send_State[nport] = RX_FSM_FINISH ;
        }
        hdmi_out(HDMI_STB_HDCP_2p2_SR0_reg,HDMI_STB_HDCP_2p2_SR0_irq_rd_msg_done_mask);
        HDCP_FLOW_PRINTF("[HDCP2.2] Tx Read finish !!!!!     \n" );
    }
}

#endif


void lib_hdmi_hdcp22_enable(unsigned char nport, unsigned char enable)
{
    hdmi_mask(HDMI_STB_HDCP_2p2_CR_reg, ~HDMI_STB_HDCP_2p2_CR_hdcp_2p2_en_mask, HDMI_STB_HDCP_2p2_CR_hdcp_2p2_en((enable) ? 1 : 0));    
}

unsigned char lib_hdmi_hdcp22_is_onoff(unsigned char nport)
{
    return HDMI_STB_HDCP_2p2_CR_get_hdcp_2p2_en(hdmi_in(HDMI_STB_HDCP_2p2_CR_reg));
}

unsigned char lib_hdmi_hdcp22_get_auth_done(unsigned char nport)
{
    return (HDMI_STB_HDCP_2p2_SR0_get_state(hdmi_in(HDMI_STB_HDCP_2p2_SR0_reg))>=HDMI_HDCP_2P2_AUTHENTICATED);
}

unsigned char lib_hdmi_hdcp22_is_irq_pending(unsigned char nport)
{
    return HDMI_STB_HDCP_PEND_get_hdcp2p2(hdmi_in(HDMI_STB_HDCP_PEND_reg));
}

void lib_hdmi_hdcp22_clr_irq_pending(unsigned char nport)
{
    hdmi_out(HDMI_STB_HDCP_PEND_reg, HDMI_STB_HDCP_PEND_hdcp2p2_mask);
}

/*********************************************************************************
 *
 *      hdcp_newbase
 *
 *********************************************************************************/


unsigned char lib_hdmi_is_hdcp22_event_pending(unsigned char nport)
{
#if (HDCP2_SEND_PAIRINGINFO_BY_IRQ_RD)
    if (lib_hdmi_hdcp22_get_write_msg(nport) || lib_hdmi_hdcp22_get_read_msg(nport))
#else
    if (lib_hdmi_hdcp22_get_write_msg(nport))
#endif
    {
        return 1;
    }

    return 0;
}

#ifndef BUILD_QUICK_SHOW
/*------------------------------------------------------------------
 * Func : newbase_hdmi_hdcp22_msg_handler
 *
 * Desc : handle hdcp2.2 event of a given hdmi port.
 *        this function will be called by scaler_hdcp2dev.c
 *        that when hw receive hdcp interrupts. (may need to refine >"<)
 *
 * Para : [IN] nport  : target hdmi port
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
unsigned char newbase_hdmi_hdcp22_msg_handler(unsigned char nport)
{
    struct hdcp2p2_work *w;
    int ret = 0;

    if (lib_hdmi_is_hdcp22_event_pending(nport) && lib_hdmi_hdcp22_is_wr_msg_done_irq_en(nport))
    {
        lib_hdmi_hdcp22_set_wr_msg_done_irq_en(nport, 0);// disalbe hdcp22 interrupt handler

#if IS_ENABLED(CONFIG_SUPPORT_SCALER)
        if(!list_empty(&hdcp2p2_list)) {
            w = list_first_entry(&hdcp2p2_list, struct hdcp2p2_work, list);
            list_del(&w->list);
        }
        else
#endif
        {
            HDCP_WARN("handle hdcp2p2 message failed, no free hdcp2p2 list entries\n");
            hdmi_out(HDMI_STB_HDCP_2p2_SR0_reg,HDMI_STB_HDCP_2p2_SR0_irq_wr_msg_done_mask); // clear write done message
            if (lib_hdmi_hdcp22_is_irq_pending(nport))	// for ML5 HDCP22 has dedicate IRQ
                lib_hdmi_hdcp22_clr_irq_pending(nport); // clear hdcp22 irq
            lib_hdmi_hdcp22_set_wr_msg_done_irq_en(nport, 1);
            return 1;
        }

        INIT_WORK(&w->work, hdcp2p2_handler);
        w->nport = nport;
#if IS_ENABLED(CONFIG_SUPPORT_SCALER)
        queue_work(hdcp2p2_wq, &w->work);
#endif
        ret = 1;
    }

    if (lib_hdmi_hdcp22_is_irq_pending(nport))  // for ML5 HDCP22 has dedicate IRQ
        lib_hdmi_hdcp22_clr_irq_pending(nport); // clear hdcp22 irq

    return ret;
}
#endif

/*------------------------------------------------------------------
 * Func : newbase_hdmi_hdcp22_write_data_to_tx
 *
 * Desc : write data to hdcp2 tx port. this function will be used by
 *        hdcp2p2 driver... (may need to refine >"<)
 *
 * Para : [IN] port  : target hdmi port
 *        [IN] bSendData  : data to be sent
 *        [IN] wLen  : message length
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void newbase_hdmi_hdcp22_write_data_to_tx(
    unsigned char               port,
    unsigned char*              bSendData,
    unsigned short              wLen
    )
{
    HDMI_HDCP_ST* p_hdcp = newbase_hdcp_get_hdcp_status(port);

    if (newbase_hdmi_hdcp_is_hdcp2_disabled(port) || p_hdcp == NULL) {
        HDCP_FLOW_PRINTF("newbase_hdmi_hdcp22_write_data_to_tx failed, HDCP22 is disabled, skip it\n");
        return;
    }

    HDCP_INFO("HDMI[p%d] HDCP2 responses %s (round=%d)\n",
        port, _hdcp2_msg_str(bSendData[0]), p_hdcp->hdcp2_auth_count);

    lib_hdmi_hdcp22_write_data_to_tx(port, bSendData, wLen);
}

void newbase_hdmi_hdcp22_check_iic_status(unsigned char port)
{
    unsigned char nport = port;
    HDMI_HDCP_ST* p_hdcp = newbase_hdcp_get_hdcp_status(port);
    unsigned int hdcp_iic_stay_one_timeout = GET_FLOW_CFG(HDMI_FLOW_CFG_TIMING_MEASURE, HDMI_FLOW_CFG7_HDCP_IIC_STAY_ONE_TIMEOUT);

    if (newbase_hdmi_phy_port_status(port)==HDMI_PHY_PORT_NOTUSED || p_hdcp==NULL)
        return;

    if (hdcp_iic_stay_one_timeout == 0 || (lib_hdmi_get_specific_device(port) != HDMI_SPECIFIC_DEVICE_SONY_BDP_S1500_S570))
        return;

    if(g_RX_msg_id[port] == AKE_Send_Cert && HDMI_STB_P0_HDCP_PCR_get_iic_st(hdmi_in(HDMI_STB_HDCP_PCR_reg)) == IIC_STATUS_READ){
        if(p_hdcp->hdcp_iic_state_time != 0 && hdmi_get_system_time_ms() - p_hdcp->hdcp_iic_state_time >= hdcp_iic_stay_one_timeout ){
            HDCP_WARN("HDMI[p%d] i2c status keep READ status too long, force reset hpd\n", port);
            newbase_hdmi_set_hpd(port, 0);
            msleep(50);
            newbase_hdmi_set_hpd(port, 1);
            p_hdcp->hdcp_iic_state_time = 0;
        }else if (p_hdcp->hdcp_iic_state_time == 0){
            p_hdcp->hdcp_iic_state_time = hdmi_get_system_time_ms();
        }
    }else{
        p_hdcp->hdcp_iic_state_time = 0;
    }
}

/*------------------------------------------------------------------
 * Func : newbase_hdmi_hdcp22_clear_status
 *
 * Desc : reset hdcp22 internal status
 *
 * Para : [IN] nport  : target hdmi port
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void newbase_hdmi_hdcp22_clear_status(unsigned char port)
{
    HDMI_HDCP_ST* p_hdcp = newbase_hdcp_get_hdcp_status(port);
    
    g_TX_msg_id[port] = NULL_MSG;
    p_hdcp->hdcp2_reauth = 0; // clear reauth flag
    p_hdcp->hdcp2_state = HDCP2_IDEL;
    p_hdcp->hdcp2_auth_count = 0;

#if (HDCP2_SEND_PAIRINGINFO_BY_IRQ_RD)
    lib_hdmi_hdcp22_set_rd_msg_done_irq(port, 0);
    lib_hdmi_hdcp22_clear_read_status(port);
#endif
}


/*------------------------------------------------------------------
 * Func : newbase_hdmi_hdcp22_clear_status
 *
 * Desc : reset hdcp22 internal status
 *
 * Para : [IN] nport  : target hdmi port
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void newbase_hdmi_hdcp22_set_reauth(unsigned char port)
{
    HDMI_HDCP_ST* p_hdcp = newbase_hdcp_get_hdcp_status(port);

    if (newbase_hdmi_phy_port_status(port)==HDMI_PHY_PORT_NOTUSED || p_hdcp==NULL)
        return;

    p_hdcp->hdcp2_reauth = 1;
    lib_hdmi_hdcp22_update_message_size(port, 0);
}
#ifndef BUILD_QUICK_SHOW
#define HDCP_IDLE 1
#if IS_ENABLED(CONFIG_SUPPORT_SCALER)
#include <rtk_kdriver/RPCDriver.h>
#else
#define send_rpc_command(opt, command, param1, param2, retvalue)    (0)
#endif

int check_vcpu_hdcp_idle(void)
{
    static int status = 0;
    if (!status) {
        unsigned long hdcp_return = 0;
        int ret;
        ret = send_rpc_command(RPC_VIDEO, RPC_VCPU_ID_0x200_CHECK_HDCP_IDLE, 
            0, 0, &hdcp_return);
        HDCP_WARN("ret = %d", ret);
        HDCP_WARN("hdcp_return = %lu", hdcp_return);
        if (ret == 0 && hdcp_return == HDCP_IDLE) {
            status = 1;
        }
    }
    return status;
}

/*------------------------------------------------------------------
 * Func : newbase_hdmi_hdcp22_set_hw_reauth
 *
 * Desc : set hdcp22 hw auto reauth
 *
 * Para : [IN] nport  : target hdmi port
 *        [IN] en  : enable/disable auto reauth
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void newbase_hdmi_hdcp22_set_hw_reauth(unsigned char en)
{
    unsigned char port = newbase_hdmi_get_current_display_port();

    HDMI_EMG("HDMI[p%d] HDCP2 set hw auto reauth %s\n", port, (en) ? "on" : "off");
    lib_hdmi_hdcp22_enable_reauth(port, en);
}

/*------------------------------------------------------------------
 * Func : newbase_hdmi_hdcp22_get_hw_reauth
 *
 * Desc : get hdcp22 hw auto reauth
 *
 * Para : [IN] nport  : target hdmi port
 *
 * Retn : 0: auto reauth disabled, 1: audo reauth enabled
 *------------------------------------------------------------------*/
unsigned char newbase_hdmi_hdcp22_get_hw_reauth(unsigned char port)
{
    return lib_hdmi_hdcp22_get_enable_reauth(port);
}

static void hdcp2p2_handler(struct work_struct *work)
{
    struct hdcp2p2_work *w = container_of(work, struct hdcp2p2_work, work);
    if (check_vcpu_hdcp_idle() == 0) {
        HDCP_WARN("video not idle, return");
        return;
    }
    set_current_hdmi_port(w->nport);
    lib_hdmi_hdcp22_get_msg_command(w->nport);
    lib_hdmi_hdcp22_send_msg_command(w->nport);
    lib_hdmi_hdcp22_set_wr_msg_done_irq_en(w->nport, 1);   // restart hdcp22 interrupt
#if IS_ENABLED(CONFIG_SUPPORT_SCALER)
    list_add_tail(&w->list, &hdcp2p2_list);
#endif
}
#endif
