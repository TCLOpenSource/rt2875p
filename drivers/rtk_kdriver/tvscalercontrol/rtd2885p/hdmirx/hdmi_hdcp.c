#include "hdmi_common.h"

#ifndef BUILD_QUICK_SHOW
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/string.h>
#include <linux/spinlock.h>
#else
#include "no_os/spinlock.h"
#include "string.h"
#endif

#include <rtk_kdriver/rtk_hdcp_otp_util.h>
#include <rtk_kdriver/tvscalercontrol/hdcp2_2/hdcp2_interface.h>
#include <mach/platform.h>
#include <mach/rtk_platform.h>
#include "hdmi_reg.h"
#include "hdmi_hdcp.h"
#include "hdmi_vfe_config.h"
#include <hdcp/hdcp2_optee.h>

extern HDMI_PORT_INFO_T hdmi_rx[HDMI_PORT_TOTAL_NUM];
extern spinlock_t hdmi_spin_lock;

#if !IS_ENABLED(CONFIG_SUPPORT_SCALER)
#define hdcp2_load_from_flash()
#define hdcp2_get_lc128(uc_lc128)
#endif

#define HDCP_REAUTH_MESSAGE_FINISHED_THD    500
unsigned int g_hdcp_reauth_message_threshould = 32;

void newbase_hdmi_hdcp_set_reauth_msg_threshold(unsigned int threshold)
{
    if (threshold<1)
        threshold = 1;

    g_hdcp_reauth_message_threshould = threshold;

    HDCP_INFO("HDCP data off sync threshold=%d\n", g_hdcp_reauth_message_threshould);
}

unsigned int newbase_hdmi_hdcp_get_reauth_msg_threshold(void)
{
    return g_hdcp_reauth_message_threshould;
}

DEFINE_SPINLOCK(hdcp2_spin_lock);

static unsigned int hdmi_port_hdcp2_disable[HDMI_PORT_TOTAL_NUM];
static unsigned char reset_hdcp_once_flag[HDMI_PORT_TOTAL_NUM];    //record hdcp reset status. hdcp initial should be once. 

HDCP_KEY_T hdcpkey;

#define HDCP_RESET_CNT          30     //The counter for keeping HDCP 1.4 or 2.2 in no signal state, if timeout, turn on both 1.4 & 2.2
unsigned char customer_hdcp1p4= false;
unsigned char customer_hdcp2p2= false;
unsigned char* h2p2table=NULL;


const HDCP_KEY_T default_hdcpkey =
{
    { 0xb7, 0x28, 0xa9, 0x51, 0xd6 },  // BKsv
    /*HDCP key table*/
    {
    0x3a, 0x21, 0x17, 0x06, 0x72, 0x60, 0x3b, 0x6e,     // Key0
    0x0c, 0x58, 0x16, 0x4c, 0x5b, 0x3b, 0x68, 0x4b,     // Key1
    0x2f, 0x7b, 0x5d, 0x64, 0x10, 0x60, 0x71, 0x41,     // Key2
    0x6d, 0x4a, 0x1f, 0x48, 0x35, 0x78, 0x4b, 0x6a,     // Key3
    0x4b, 0x3c, 0x69, 0x2a, 0x20, 0x38, 0x55, 0x1c,     // Key4
    0x10, 0x16, 0x09, 0x5d, 0x0d, 0x07, 0x6e, 0x7a,     // Key5
    0x75, 0x1a, 0x35, 0x55, 0x7b, 0x5c, 0x26, 0x0d,     // Key6
    0x2c, 0x49, 0x1a, 0x1b, 0x3e, 0x55, 0x1f, 0x40,     // Key7
    0x11, 0x24, 0x4b, 0x4a, 0x5d, 0x56, 0x4b, 0x5c,     // Key8
    0x62, 0x70, 0x0b, 0x10, 0x52, 0x3f, 0x2b, 0x4d,     // Key9
    0x5d, 0x06, 0x78, 0x16, 0x72, 0x70, 0x2e, 0x06,     // Key10
    0x3a, 0x2c, 0x60, 0x06, 0x12, 0x6c, 0x32, 0x55,     // Key11
    0x54, 0x62, 0x38, 0x05, 0x13, 0x42, 0x49, 0x40,     // Key12
    0x61, 0x54, 0x28, 0x0c, 0x4a, 0x21, 0x7c, 0x32,     // Key13
    0x4b, 0x74, 0x04, 0x5d, 0x10, 0x69, 0x72, 0x62,     // Key14
    0x5d, 0x77, 0x55, 0x0d, 0x19, 0x79, 0x59, 0x43,     // Key15
    0x0a, 0x5f, 0x0d, 0x50, 0x58, 0x4f, 0x48, 0x42,     // Key16
    0x14, 0x64, 0x65, 0x54, 0x2f, 0x7a, 0x75, 0x04,     // Key17
    0x48, 0x35, 0x00, 0x59, 0x16, 0x0e, 0x2f, 0x4b,     // Key18
    0x4a, 0x7b, 0x04, 0x6c, 0x1a, 0x6d, 0x4b, 0x44,     // Key19
    0x50, 0x11, 0x63, 0x63, 0x47, 0x6e, 0x1a, 0x6b,     // Key20
    0x0f, 0x4c, 0x3e, 0x17, 0x66, 0x0d, 0x3b, 0x3a,     // Key21
    0x47, 0x62, 0x5a, 0x25, 0x3d, 0x0e, 0x1d, 0x55,     // Key22
    0x42, 0x0f, 0x43, 0x60, 0x3e, 0x7e, 0x56, 0x3c,     // Key23
    0x44, 0x4a, 0x44, 0x3b, 0x69, 0x17, 0x1b, 0x47,     // Key24
    0x19, 0x77, 0x2c, 0x7d, 0x72, 0x03, 0x66, 0x53,     // Key25
    0x7f, 0x5a, 0x03, 0x26, 0x17, 0x01, 0x28, 0x5e,     // Key26
    0x09, 0x1d, 0x43, 0x13, 0x35, 0x26, 0x4c, 0x15,     // Key27
    0x1e, 0x7f, 0x4d, 0x6b, 0x4d, 0x52, 0x46, 0x00,     // Key28
    0x3c, 0x38, 0x5c, 0x64, 0x6f, 0x72, 0x78, 0x11,     // Key29
    0x30, 0x65, 0x47, 0x70, 0x7a, 0x3c, 0x38, 0x69,     // Key30
    0x78, 0x26, 0x35, 0x09, 0x37, 0x14, 0x34, 0x57,     // Key31
    0x73, 0x4d, 0x28, 0x28, 0x7e, 0x5a, 0x28, 0x09,     // Key32
    0x08, 0x28, 0x6c, 0x69, 0x42, 0x43, 0x31, 0x15,     // Key33
    0x54, 0x23, 0x60, 0x1c, 0x03, 0x48, 0x7d, 0x48,     // Key34
    0x25, 0x5f, 0x2b, 0x04, 0x6c, 0x42, 0x22, 0x66,     // Key35
    0x11, 0x2a, 0x1a, 0x14, 0x29, 0x68, 0x59, 0x73,     // Key36
    0x76, 0x61, 0x16, 0x00, 0x3e, 0x11, 0x31, 0x20,     // Key37
    0x10, 0x59, 0x13, 0x44, 0x54, 0x32, 0x4f, 0x0e,     // Key38
    0x76, 0x47, 0x13, 0x6f, 0x7e, 0x24, 0x5a, 0x6a,     // Key39
    }
};

void lib_hdmi_hdcp_init(unsigned char nport)
{
    // Disable HDCP and clear HDCP address
    //hdmi_out(HDMI_HDCP_CR_reg, (HDMI_HDCP_CR_maddf_mask|HDMI_HDCP_CR_dkapde_mask));
    hdmi_mask(HDMI_HDCP_CR_reg, ~(HDMI_HDCP_CR_hdcp_clk_switch_auto_mask), HDMI_HDCP_CR_hdcp_clk_switch_auto_mask);

    hdmi_mask(HDMI_HDCP_PCR_reg, ~(HDMI_HDCP_PCR_enc_tog_mask), 0);

#ifdef HDMI_FIX_RL6583_1265
    // [RL6583-1265] foce dvi enc mode to 1, HW keep running if ctrl3 is lost
    hdmi_mask(HDMI_STB_HDCP_PCR_reg, ~HDMI_STB_HDCP_PCR_dvi_enc_mode_mask, HDMI_HDCP_PCR_dvi_enc_mode(1));
    hdmi_mask(HDMI_HDCP_PCR_reg, ~HDMI_HDCP_PCR_dvi_enc_mode_mask, HDMI_HDCP_PCR_dvi_enc_mode(1));    
#endif

#if 1 /*defined (ENABLE_HDMI_1_1_SUPPORT)*/                     /* must set in hdcp cts1.4 */
    lib_hdmi_hdcp_port_write(nport, 0x40, HDCP_11_HDMI);        //set OESS for DVI
    lib_hdmi_hdcp_port_write(nport, 0x40, HDCP_11_HDMI|_BIT6);  //Merlin2: bug: default repeater on
    lib_hdmi_hdcp_port_write(nport, 0x43, 0x0);                 //Merlin2: bug: default repeater on
    lib_hdmi_hdcp_port_write(nport, 0x40, HDCP_11_HDMI);        //Merlin2: bug: default repeater on
#else
    Hdmi_HdcpPortWrite(0x40,HDCP_10_DVI);                       //set OESS for DVI
#endif
    newbase_hdmi_reset_hdcp_once(nport, TRUE);    //initial hdcp had reset HDCP once
    memset(hdmi_port_hdcp2_disable, GET_FLOW_CFG(HDMI_FLOW_CFG_GENERAL, HDMI_FLOW_CFG0_HDCP22_DISABLE), sizeof(hdmi_port_hdcp2_disable));
    lib_hdmi_hdcp_port_write(nport, 0x50, (GET_FLOW_CFG(HDMI_FLOW_CFG_GENERAL, HDMI_FLOW_CFG0_HDCP22_DISABLE)) ? 0x00: 0x04);    //support hdcp2.2

    hdmi_mask(HDMI_HDCP_CR_reg, ~(HDMI_HDCP_CR_namfe_mask), (HDMI_HDCP_CR_namfe_mask)); //for New_AC_Mode Enable,fix simplay bug   
}


void lib_hdmi_hdcp14_write_hdcpkey(unsigned char nport, unsigned char bksv[5], unsigned char hdcp_key[320])
{
    unsigned int i;       

    // Disable HDCP and clear HDCP address

    hdmi_mask(HDMI_STB_HDCP_CR_reg,
        ~(HDMI_STB_HDCP_CR_hdcp_en_mask|
          HDMI_STB_HDCP_CR_dkapde_mask|
          HDMI_STB_HDCP_CR_maddf_mask),
          0);

    hdmi_mask(HDMI_HDCP_CR_reg,
        ~(HDMI_HDCP_CR_ivspm_mask|
          HDMI_HDCP_CR_invvs_mask|
          HDMI_HDCP_CR_ivsp_mask|
          HDMI_HDCP_CR_rpt_mask|
          HDMI_HDCP_CR_namfe_mask), 
          0);
    
    udelay(10);   // <-- do not remove it, or you will get data error

    lib_hdmi_hdcp_port_write_conti(nport, 0x00, bksv, 5);   //set KSV 40 bits

    hdmi_mask(HDMI_STB_HDCP_CR_reg,
        ~(HDMI_STB_HDCP_CR_hdcp_en_mask|
          HDMI_STB_HDCP_CR_dkapde_mask|
          HDMI_STB_HDCP_CR_maddf_mask),
          HDMI_STB_HDCP_CR_maddf_mask|
          HDMI_STB_HDCP_CR_dkapde_mask);

    udelay(10);   // <-- do not remove it, or you will get data error

    /*Write device private key*/
    for(i=0; i<320; i++) {
#ifdef CONFIG_HDCP_KEY_PRINT
        if (i% 8 == 0) HDMI_PRINTF("\n");
        HDMI_PRINTF("%x ", hdcp_key[i]);
#endif //#ifdef CONFIG_HDCP_KEY_PRINT

        hdmi_out(HDMI_STB_HDCP_DKAP_reg, hdcp_key[i]);
    }

#ifdef CONFIG_HDCP_KEY_PRINT
    HDMI_PRINTF("\n =================================================== =");

    hdmi_mask(HDMI_HDCP_CR_reg,
    ~(HDMI_HDCP_CR_hdcp_en_mask|HDMI_HDCP_CR_dkapde_mask|HDMI_HDCP_CR_maddf_mask|HDMI_HDCP_CR_ivspm_mask|
    HDMI_HDCP_CR_invvs_mask|HDMI_HDCP_CR_ivsp_mask|HDMI_HDCP_CR_rpt_mask|HDMI_HDCP_CR_namfe_mask)
    ,HDMI_HDCP_CR_maddf_mask|HDMI_HDCP_CR_dkapde_mask);

    HDMI_PRINTF("\n i=%d  HDMI BKSV Flag = %x, %x ,%x ,%x, %x \n",
           i ,
           lib_hdmi_hdcp_port_read(nport, 0x00),
           lib_hdmi_hdcp_port_read(nport, 0x01),
           lib_hdmi_hdcp_port_read(nport, 0x02),
           lib_hdmi_hdcp_port_read(nport, 0x03),
           lib_hdmi_hdcp_port_read(nport, 0x04));
    HDMI_PRINTF("HDCP key 320 byte read ");
    for(i=0; i<320; i++) {
        if (i% 16 == 0) HDMI_PRINTF("\n");
        HDMI_PRINTF("%x ", hdmi_in(HDMI_STB_HDCP_DKAP_reg));
    }

#endif //#ifdef CONFIG_HDCP_KEY_PRINT


    hdmi_mask(HDMI_HDCP_CR_reg,
            ~(HDMI_HDCP_CR_ivspm_mask|
              HDMI_HDCP_CR_invvs_mask|
              HDMI_HDCP_CR_ivsp_mask|
              HDMI_HDCP_CR_rpt_mask|
              HDMI_HDCP_CR_namfe_mask), 
              0x00);
    
    hdmi_mask(HDMI_STB_HDCP_CR_reg,
            ~(HDMI_STB_HDCP_CR_hdcp_en_mask|
              HDMI_STB_HDCP_CR_dkapde_mask|
              HDMI_STB_HDCP_CR_maddf_mask), 0x00);    

    hdmi_mask(HDMI_HDCP_CR_reg, ~(HDMI_HDCP_CR_namfe_mask), (HDMI_HDCP_CR_namfe_mask)); //for New_AC_Mode Enable,fix simplay bug

}

void lib_hdmi_hdcp14_read_hdcpkey(unsigned char nport, unsigned char bksv[5], unsigned char hdcp_key[320])
{
    unsigned int i;
    unsigned int reg_bak;

    reg_bak = hdmi_in(HDMI_STB_HDCP_CR_reg);
    // Disable HDCP and clear HDCP address

    hdmi_mask(HDMI_HDCP_CR_reg,
            ~(HDMI_HDCP_CR_ivspm_mask|
              HDMI_HDCP_CR_invvs_mask|
              HDMI_HDCP_CR_ivsp_mask|
              HDMI_HDCP_CR_rpt_mask|
              HDMI_HDCP_CR_namfe_mask), 0x00);
    
    hdmi_mask(HDMI_STB_HDCP_CR_reg,
            ~(HDMI_STB_HDCP_CR_hdcp_en_mask|
              HDMI_STB_HDCP_CR_dkapde_mask|
              HDMI_STB_HDCP_CR_maddf_mask), 0x00);

    udelay(10);   // <-- do not remove it, or you will get data error

    hdmi_mask(HDMI_STB_HDCP_CR_reg,
            ~(HDMI_STB_HDCP_CR_hdcp_en_mask|
              HDMI_STB_HDCP_CR_dkapde_mask|
              HDMI_STB_HDCP_CR_maddf_mask),
              HDMI_STB_HDCP_CR_maddf_mask|
              HDMI_STB_HDCP_CR_dkapde_mask);

    udelay(10);   // <-- do not remove it, or you will get data error

    for (i=0; i<5; i++)
        bksv[i] = lib_hdmi_hdcp_port_read(nport, i);

    for(i=0; i<320; i++) {
        hdcp_key[i] = hdmi_in(HDMI_STB_HDCP_DKAP_reg);        
        udelay(1);
    }

#if defined(CONFIG_HDCP_KEY_PRINT)
    HDMI_PRINTF("\nHDCP BKSV Flag = %x, %x ,%x ,%x, %x \n",
                bksv[0],
                bksv[1],
                bksv[2],
                bksv[3],
                bksv[4]);
    HDMI_PRINTF("HDCP key 320 byte read=\n");
    for(i=0; i<320; i++) {
        if (i% 16 == 0) HDMI_PRINTF("\n");
        HDMI_PRINTF("%x ",  hdcp_key[i]);
    }
#endif //#ifdef CONFIG_HDCP_KEY_PRINT

    hdmi_out(HDMI_STB_HDCP_CR_reg, reg_bak);
}


void lib_hdmi_hdcp14_enable(unsigned char nport, unsigned char on)
{
    hdmi_mask(HDMI_STB_HDCP_CR_reg, ~HDMI_STB_HDCP_CR_hdcp_en_mask, (on) ? HDMI_STB_HDCP_CR_hdcp_en_mask: 0);    
}

unsigned char lib_hdmi_hdcp14_is_onoff(unsigned char nport)
{
    return HDMI_STB_HDCP_CR_get_hdcp_en(hdmi_in(HDMI_STB_HDCP_CR_reg));
}

unsigned char lib_hdmi_hdcp14_check_aksv_reg(unsigned char nport)
{
    return HDMI_STB_HDCP_FLAG1_get_wr_aksv_flag(hdmi_in(HDMI_STB_HDCP_FLAG1_reg));
}

void lib_hdmi_hdcp14_clear_aksv_reg(unsigned char nport)
{
    hdmi_out(HDMI_STB_HDCP_FLAG1_reg, HDMI_STB_HDCP_FLAG1_wr_aksv_flag_mask);
}

unsigned char lib_hdmi_hdcp14_check_ri_reg(unsigned char nport)
{
    return HDMI_STB_HDCP_FLAG1_get_rd_ri_flag(hdmi_in(HDMI_STB_HDCP_FLAG1_reg));
}

void lib_hdmi_hdcp14_clear_ri_reg(unsigned char nport)
{
    hdmi_out(HDMI_STB_HDCP_FLAG1_reg, HDMI_STB_HDCP_FLAG1_rd_ri_flag_mask);
}

unsigned char lib_hdmi_hdcp14_check_akm_reg(unsigned char nport)
{
    return HDMI_STB_HDCP_FLAG1_get_akm_flag(hdmi_in(HDMI_STB_HDCP_FLAG1_reg));
}

void lib_hdmi_hdcp14_clear_akm_reg(unsigned char nport)
{
    hdmi_out(HDMI_STB_HDCP_FLAG1_reg, HDMI_STB_HDCP_FLAG1_akm_flag_mask);
}

unsigned char lib_hdmi_hdcp14_check_bksv_reg(unsigned char nport)
{
    return HDMI_STB_HDCP_FLAG1_get_rd_bksv_flag(hdmi_in(HDMI_STB_HDCP_FLAG1_reg));
}

void lib_hdmi_hdcp14_clear_bksv_reg(unsigned char nport)
{
    hdmi_out(HDMI_STB_HDCP_FLAG1_reg, HDMI_STB_HDCP_FLAG1_rd_bksv_flag_mask);
}

unsigned char lib_hdmi_hdcp14_check_adne_reg(unsigned char nport)
{    
    return HDMI_STB_HDCP_FLAG1_get_adne_flag(hdmi_in(HDMI_STB_HDCP_FLAG1_reg));
}

void lib_hdmi_hdcp14_clear_adne_reg(unsigned char nport)
{
    hdmi_out(HDMI_STB_HDCP_FLAG1_reg, HDMI_STB_HDCP_FLAG1_adne_flag_mask);
}


void lib_hdmi_hdcp14_clear_ri(unsigned char nport)
{
    hdmi_out(HDMI_STB_HDCP_CR_reg, hdmi_in(HDMI_STB_HDCP_CR_reg) | HDMI_STB_HDCP_CR_clr_ri_mask); // clear Ri
}

unsigned char lib_hdmi_hdcp14_get_aksv_wr_irq_en(unsigned char nport)
{
    return HDMI_STB_HDCP_FLAG2_get_irq_aksv_en(hdmi_in(HDMI_STB_HDCP_FLAG2_reg));
}

void lib_hdmi_hdcp14_set_aksv_wr_irq(unsigned char nport, unsigned char enable)
{
    hdmi_mask(HDMI_STB_HDCP_FLAG2_reg, ~HDMI_STB_HDCP_FLAG2_irq_aksv_en_mask, (enable) ? HDMI_STB_HDCP_FLAG2_irq_aksv_en_mask : 0);
}

unsigned char lib_hdmi_hdcp14_is_irq_pending(unsigned char nport)
{
    return HDMI_STB_HDCP_PEND_get_hdcp1p4(hdmi_in(HDMI_STB_HDCP_PEND_reg));
}

void lib_hdmi_hdcp14_clr_irq_pending(unsigned char nport)
{
    hdmi_out(HDMI_STB_HDCP_PEND_reg, HDMI_STB_HDCP_PEND_hdcp1p4_mask);
}

void lib_hdmi_hdcp_port_write_conti(unsigned char nport, unsigned char addr, const unsigned char* value, unsigned char num)
{
    unsigned char bit0_tmp;
    unsigned long flags;

    lock_hdcp_sram(flags);

    bit0_tmp = HDMI_STB_HDCP_PCR_get_apai(hdmi_in(HDMI_STB_HDCP_PCR_reg));
    hdmi_mask(HDMI_STB_HDCP_PCR_reg, ~HDMI_STB_HDCP_PCR_apai_mask, 0);

    hdmi_out(HDMI_STB_HDCP_AP_reg, addr);

    while(num--)
        hdmi_out(HDMI_STB_HDCP_DP_reg, *(value++));

    hdmi_mask(HDMI_STB_HDCP_PCR_reg, ~HDMI_STB_HDCP_PCR_apai_mask, HDMI_STB_HDCP_PCR_apai(bit0_tmp));

    unlock_hdcp_sram(flags);
}

void lib_hdmi_hdcp_port_write(unsigned char nport, unsigned char addr ,unsigned char value)
{
    unsigned long flags;
    lock_hdcp_sram(flags);
    hdmi_out(HDMI_STB_HDCP_AP_reg, addr);
    hdmi_out(HDMI_STB_HDCP_DP_reg, value);
    unlock_hdcp_sram(flags);
}

unsigned char lib_hdmi_hdcp_port_read(unsigned char nport, unsigned char addr)
{
    unsigned char val;
    unsigned long flags;
    lock_hdcp_sram(flags);
    hdmi_out(HDMI_STB_HDCP_AP_reg, addr);
    val = hdmi_in(HDMI_STB_HDCP_DP_reg) & 0xFF;
    unlock_hdcp_sram(flags);
    return val;
}

void lib_hdmi_hdcp_set_fifo_mode(unsigned char nport, unsigned char auto_mode,unsigned char fw_fifo_mode_hdcp22)
{
    unsigned int fifo_mode = 0;

    if (auto_mode)
        fifo_mode |= HDMI_STB_SCDC_PCR_fifo_switch_mode_mask;

    if (fw_fifo_mode_hdcp22==0)
        fifo_mode |= HDMI_STB_SCDC_PCR_fifo_mode_fw_mask;

    hdmi_mask(HDMI_STB_HDCP_CR_reg, ~(HDMI_STB_SCDC_PCR_fifo_mode_fw_mask | HDMI_STB_SCDC_PCR_fifo_switch_mode_mask), fifo_mode);
}

void lib_hdmi_hdcp_toggle_dkapde(unsigned char nport)
{
    hdmi_mask(HDMI_STB_HDCP_CR_reg, ~(HDMI_STB_HDCP_CR_dkapde_mask), 0);  // toggle dkape mask   
    udelay(10); 
    hdmi_mask(HDMI_STB_HDCP_CR_reg, ~(HDMI_STB_HDCP_CR_dkapde_mask), HDMI_STB_HDCP_CR_dkapde_mask);  // toggle dkape mask
    udelay(10);
    hdmi_mask(HDMI_STB_HDCP_CR_reg, ~(HDMI_STB_HDCP_CR_dkapde_mask), 0);  // toggle dkape mask        
    udelay(10);    
}

void lib_hdmi_hdcp14_cypher_enable(unsigned char nport, unsigned char enable)
{
    hdmi_mask(HDMI_STB_HDCP_CR_reg, ~(HDMI_STB_P0_HDCP_CR_cypher_dis_mask), (enable) ? 0 : HDMI_STB_P0_HDCP_CR_cypher_dis_mask); // enable/disable cypher
}

void lib_hdmi_hdcp_set_enc_tog_sel(unsigned char nport, unsigned char mode)
{
    //hdmi_mask(HDMI_HDCP_PCR_reg, ~HDMI_HDCP_PCR_enc_tog_sel_mask, (mode ? HDMI_HDCP_PCR_enc_tog_sel_mask:0));
}

unsigned char lib_hdmi_hdcp_get_enc_toggle(unsigned char nport)
{
    return HDMI_HDCP_PCR_get_enc_tog(hdmi_in(HDMI_HDCP_PCR_reg));
}

void lib_hdmi_hdcp_clear_enc_toggle(unsigned char nport)
{
    hdmi_out(HDMI_HDCP_PCR_reg, hdmi_in(HDMI_HDCP_PCR_reg) | HDMI_HDCP_PCR_enc_tog_mask);    // write one clear
}

void lib_hdmi_hdcp_set_win_opp_mode(unsigned char nport, unsigned char mode)
{
    hdmi_mask(HDMI_HDCP_CR2_reg, ~HDMI_HDCP_CR2_win_opp_mode_mask, (mode) ? HDMI_HDCP_CR2_win_opp_mode(1) : 0);
}

void lib_hdmi_hdcp_set_win_enc_window(unsigned char nport, unsigned int enc_window)
{
    hdmi_mask(HDMI_HDCP_CR2_reg, ~HDMI_HDCP_CR2_enc_window_de2vs_mask, HDMI_HDCP_CR2_enc_window_de2vs(enc_window));
}

unsigned char lib_hdmi_hdcp_get_vs_polarity(unsigned char nport)
{
    if (HDMI_HDCP_CR_get_ivspm(hdmi_in(HDMI_HDCP_CR_reg)))
    {
        return (HDMI_HDCP_CR_get_invvs(hdmi_in(HDMI_HDCP_CR_reg))==0) ? 1 : 0;
    }
    
    return (HDMI_HDCP_CR_get_ivsp(hdmi_in(HDMI_HDCP_CR_reg))==0) ? 1 : 0;
}

void lib_hdmi_hdcp_set_i2c_addr(unsigned char nport, unsigned char i2c_addr)
{
    lib_hdmi_hdcp_port_write(nport, 0xC2, i2c_addr);    
}

/*********************************************************************************
 *
 *      hdcp_newbase
 *
 *********************************************************************************/
 
const char *str_hdcp_mode_name[] = {
	"NO_HDCP",
	"HDCP14",
	"HDCP22",
	"HDCP_OFF",
	"HDCP_MODE_ERROR"
};

const char* _hdmi_hdcp_mode_str(HDMI_HDCP_E hdcp_mode)
{
    if (hdcp_mode >= sizeof(str_hdcp_mode_name)/sizeof(char*))
        hdcp_mode =  sizeof(str_hdcp_mode_name)/sizeof(char*) - 1;  // force to HDCP_MODE_ERROR

    return str_hdcp_mode_name[hdcp_mode];
}

unsigned char newbase_hdmi_is_reset_hdcp_once(unsigned char port)
{
    //HDCP_WARN("HDMI[p%d] reset_hdcp_once_flag[port]=%d\n", port, reset_hdcp_once_flag[port]);
    return reset_hdcp_once_flag[port];
}

void newbase_hdmi_reset_hdcp_once(unsigned char port, unsigned char set_value)
{
    reset_hdcp_once_flag[port] = set_value;
    //HDCP_WARN("HDMI[p%d] newbase_hdmi_is_reset_hdcp_once=%d\n", port, set_value);

}

void newbase_hdmi_switch_hdcp_mode(unsigned char port, HDMI_HDCP_E target_hdcp_mode)
{
    HDCP_INFO("HDMI[p%d] Switch to %s (Real auth mode %s)\n", port,  _hdmi_hdcp_mode_str(target_hdcp_mode), _hdmi_hdcp_mode_str(newbase_hdcp_get_auth_mode(port)));
    
    switch(target_hdcp_mode)
    {
    case HDCP14:
        lib_hdmi_hdcp14_enable(port, 1);
        lib_hdmi_hdcp22_enable(port, 0);
        lib_hdmi_hdcp_set_win_opp_mode(port, 0);
        break;
    case HDCP22:
        lib_hdmi_hdcp22_enable(port, (newbase_hdmi_hdcp_is_hdcp2_disabled(port) ? 0 : 1));
        lib_hdmi_hdcp14_enable(port, 0);
        lib_hdmi_hdcp_set_win_opp_mode(port, 0);
        break;
    case NO_HDCP:    //Init        
        lib_hdmi_hdcp14_enable(port, 1);
        lib_hdmi_hdcp22_enable(port, (newbase_hdmi_hdcp_is_hdcp2_disabled(port) ? 0 : 1));
        lib_hdmi_hdcp_set_win_opp_mode(port, 0);
        break;
        
    case HDCP_OFF:    //Force disable HDCP
        lib_hdmi_hdcp14_enable(port, 0);
        lib_hdmi_hdcp22_enable(port, 0);            
        lib_hdmi_hdcp14_clear_ri(port);   // [RL6557-575] clear ri when no clock detected            
        lib_hdmi_hdcp_port_write(port, 0xC4, 0x0);   //Need to clear setting of ACTION_FLAG_HDCP_ENC_DIS.
        lib_hdmi_hdcp_set_win_opp_mode(port, 0);
        break;
        
    default:
        HDCP_WARN("HDMI[p%d] Error target_hdcp_mode =%d\n", port, target_hdcp_mode);
        break;
    }
}

void newbase_hdmi_hdcp_qs_init(void)
{
    if (check_vcpu_hdcp_idle()) {
        HDCP_WARN("check video idle in init");
    }
    else{
        HDCP_WARN("check video not idle in init");
    }
}

void newbase_hdmi_hdcp_init(void)
{
    unsigned char j;
	#ifndef CONFIG_OPTEE_HDCP2
    unsigned char uc_lc128[16]={0};
	#endif

#if defined(CONFIG_RTK_KDRV_HDCP_KEY_FROM_OTP)
    unsigned char hdcp14_buf[325];
    if (get_hdcp_14_otp(hdcp14_buf, 325) == 0) {
        HDCP_INFO("[HDCP] HDCP1.4 OTP-KEY  OK\n");
        memcpy(hdcpkey.BKsv , hdcp14_buf,5);
        memcpy(hdcpkey.Key , hdcp14_buf+5, 320);
    }
    else
#endif//defined(CONFIG_RTK_KDRV_HDCP_KEY_FROM_OTP)
    {
        HDCP_WARN("[ERROR] NO HDCP1.4 OTP-KEY\n");

        if(customer_hdcp1p4)
        {
            HDCP_WARN("[HDCP] Using Customer HDCP1.4 KEY\n");
        }
    }

    for (j=0; j<HDMI_PORT_TOTAL_NUM; j++)
    {
        if (newbase_hdmi_phy_port_status(j)==HDMI_PHY_PORT_NOTUSED)
            continue;

        lib_hdmi_hdcp_init(j);        
        lib_hdmi_hdcp14_write_hdcpkey(j, hdcpkey.BKsv, hdcpkey.Key);   // apply hdcp1.4 key of the given hdcp port

        newbase_hdmi_hdcp14_set_aksv_intr(j, 1);
    }

#ifndef CONFIG_OPTEE_HDCP2
        hdcp2_load_from_flash();
        hdcp2_get_lc128(uc_lc128);
#endif

    for (j=0; j<HDMI_PORT_TOTAL_NUM; j++) {
        if (newbase_hdmi_phy_port_status(j) == HDMI_PHY_PORT_NOTUSED)
            continue;

        #ifndef CONFIG_OPTEE_HDCP2
        lib_hdmi_hdcp22_init(j,uc_lc128);
        #else
        lib_hdmi_hdcp22_init(j, NULL);
        #endif
        newbase_hdmi_hdcp_enable(j, 0);
    }
}

void newbase_hdmi_hdcp_enable(unsigned char port, unsigned char enable)
{
    HDMI_HDCP_ST* p_hdcp = newbase_hdcp_get_hdcp_status(port);
    
    if (enable)
    {
#if HDMI_CLEAR_HDCP22_SRAM_PATCH
        if(newbase_hdmi_hdcp_is_hdcp2_disabled(port)==0)
        {
            HDCP_INFO("[newbase_hdmi_hdcp_reset_fsm] Port =%d, HDCP disable, then Initial  HDCP mode status to %s\n", port, _hdmi_hdcp_mode_str(HDCP22));
            newbase_hdmi_switch_hdcp_mode(port, HDCP22);
        }
        else
        {
            newbase_hdmi_switch_hdcp_mode(port, HDCP14);
        }

        if (newbase_hdmi_is_hdcp_repeater_enable())
        {
            lib_hdmi_hdcp_port_write(port, 0x40, HDCP_11_HDMI|_BIT6);
            
            #if HDMI_FIX_RL6672_3031_HDCP14_KSV_FIFO
            lib_hdmi_hdcp_set_fifo_mode(port, 1, 0);       // by default, set HDCP repeater fifo to HW auto mode
            #endif // HDMI_FIX_RL6672_3031_HDCP14_KSV_FIFO
        }
        else
        {
            lib_hdmi_hdcp_port_write(port, 0x40, HDCP_11_HDMI);

            #if HDMI_FIX_RL6672_3031_HDCP14_KSV_FIFO
            lib_hdmi_hdcp_set_fifo_mode(port, 0, 1);       // by default, fifo for HDCP22 only
            #endif // HDMI_FIX_RL6672_3031_HDCP14_KSV_FIFO
        }
#else
        newbase_hdmi_switch_hdcp_mode(port, HDCP14);
#endif
    }
    else
    {
        newbase_hdmi_switch_hdcp_mode(port, HDCP_OFF);

        // clear all temperal information here
        p_hdcp->hdcp14_auth_count = 0;
        p_hdcp->hdcp14_ri_count = 0;
        p_hdcp->hdcp2_auth_count = 0;
        p_hdcp->hdcp2_wait_mesage_timout = 0;
        p_hdcp->hdcp_iic_state_time = 0;

        newbase_hdmi_hdcp22_clear_status(port);
#ifdef HDMI_FIX_RL6583_647
        lib_hdmi_hdcp14_cypher_enable(port, 0);     // disable HDCP cypher to be reset, hdcp14 cipher will be enabled when got AKSV
#endif
        if (newbase_hdmi_is_hdcp_repeater_enable())
            lib_hdmi_hdcp_port_write(port, 0x40, HDCP_11_HDMI|_BIT6);
        else
            lib_hdmi_hdcp_port_write(port, 0x40, HDCP_11_HDMI);
    }
}


void newbase_hdmi_hdcp_reset_fsm(unsigned char nport)
{
    if (newbase_hdmi_phy_port_status(nport)==HDMI_PHY_PORT_NOTUSED)
        return;
        
    newbase_hdmi_hdcp_enable(nport, 0);
    newbase_hdmi_hdcp_enable(nport, 1);    //Set initial hdcp mode., Only enable HDCP22 before set_phy
}


void newbase_hdcp_init_timer(unsigned char port)
{
#if HDMI_CLEAR_HDCP22_SRAM_PATCH
    HDMI_HDCP_ST* p_hdcp = newbase_hdcp_get_hdcp_status(port);

    p_hdcp->hdcp_init_period = HDCP_RESET_CNT;
#endif
}

HDMI_HDCP_ST* newbase_hdcp_get_hdcp_status(unsigned char port)
{
    return &hdmi_rx[port].hdcp_status;
}

void newbase_hdcp_aksv_handler(unsigned char port)
{
	HDMI_HDCP_ST* p_hdcp = newbase_hdcp_get_hdcp_status(port);

	lib_hdmi_hdcp14_cypher_enable(port, 1);

	p_hdcp->hdcp14_aksv_rcv = 1;

	p_hdcp->hdcp14_auth_count++;
	p_hdcp->hdcp14_ri_count = 0;
	p_hdcp->hdcp2_auth_count = 0;

	newbase_hdmi_switch_hdcp_mode(port, HDCP14);

	newbase_hdmi_power_saving_wakeup(port, PS_WAKEUP_HDCP_REQUEST);

	newbase_hdcp_init_timer(port);
}

#ifndef BUILD_QUICK_SHOW
void newbase_hdcp_handler(unsigned char port)
{
    HDMI_HDCP_ST* p_hdcp = newbase_hdcp_get_hdcp_status(port);

    unsigned char hdcp_0x15 = 0;
    unsigned char hdcp_0x40 = 0;
    if (p_hdcp==NULL)
        return ;

    //-----------------------------
    // HDCP 1.4
    //-----------------------------
    if (GET_FLOW_CFG(HDMI_FLOW_CFG_GENERAL, HDMI_FLOW_CFG0_DISABLE_AKSV_INTERRUPT))
    {
        if (lib_hdmi_hdcp14_check_aksv_reg(port))
        {
            HDCP_FLOW_PRINTF("HDMI[p%d] Interrupt disabled. polled aksv flag\n", port);

            newbase_hdcp_aksv_handler(port);

            lib_hdmi_hdcp14_clear_aksv_reg(port);
        }
    }

    if (p_hdcp->hdcp14_aksv_rcv)
    {
        //Tx write Aksv
        HDCP_INFO("HDMI[p%d] got AKSV, start auth (round=%d, ri_cnt=%d->0)\n", port, 
            p_hdcp->hdcp14_auth_count, 
            p_hdcp->hdcp14_ri_count);

        p_hdcp->tx_aksv[0] = lib_hdmi_hdcp_port_read(port, 0x10);
        p_hdcp->tx_aksv[1] = lib_hdmi_hdcp_port_read(port, 0x11);
        p_hdcp->tx_aksv[2] = lib_hdmi_hdcp_port_read(port, 0x12);
        p_hdcp->tx_aksv[3] = lib_hdmi_hdcp_port_read(port, 0x13);
        p_hdcp->tx_aksv[4] = lib_hdmi_hdcp_port_read(port, 0x14);

        HDCP_FLOW_PRINTF("HDMI[p%d] AKSV Flag = %x, %x ,%x ,%x, %x\n",
            port,
            p_hdcp->tx_aksv[0], 
            p_hdcp->tx_aksv[1], 
            p_hdcp->tx_aksv[2], 
            p_hdcp->tx_aksv[3], 
            p_hdcp->tx_aksv[4]);
            
        newbase_hdmi_hdcp_repeater_send_event(port, HDCP_REPEATER_EVENT_UPDATE_AKSV);  // notify hdcp repeater that the AKSV list is updated

        /* clear aksv write done flag */
        newbase_hdmi_hdcp14_set_aksv_intr(port, 0);
        p_hdcp->hdcp14_aksv_rcv = 0;
        newbase_hdmi_hdcp14_set_aksv_intr(port, 1);

        hdcp_0x40 = lib_hdmi_hdcp_port_read(port, 0x40);
        hdcp_0x15 = lib_hdmi_hdcp_port_read(port, 0x15);
        HDCP_INFO("[HDMI][p%d] Bcaps=%x, Ainfo=%x\n",port,hdcp_0x40, hdcp_0x15);


        HDCP_FLOW_PRINTF("\nHDMI An = %x  %x %x  %x  %x %x %x  %x\n",
            lib_hdmi_hdcp_port_read(port, 0x18),
            lib_hdmi_hdcp_port_read(port, 0x19),
            lib_hdmi_hdcp_port_read(port, 0x1a),
            lib_hdmi_hdcp_port_read(port, 0x1b),
            lib_hdmi_hdcp_port_read(port, 0x1c),
            lib_hdmi_hdcp_port_read(port, 0x1d),
            lib_hdmi_hdcp_port_read(port, 0x1e),
            lib_hdmi_hdcp_port_read(port, 0x1f));
    }
    
    if (lib_hdmi_hdcp14_check_ri_reg(port))
    {
        unsigned char ri0 = 0, ri1 = 0;
        unsigned char Pj  =0;
        lib_hdmi_hdcp14_clear_ri_reg(port);
        p_hdcp->hdcp14_ri_count++;

        ri0 = lib_hdmi_hdcp_port_read(port, 0x8);
        ri1 = lib_hdmi_hdcp_port_read(port, 0x9);
        Pj  = lib_hdmi_hdcp_port_read(port, 0xA);
        
        HDCP_FLOW_PRINTF("HDMI[p%d] Ri(%d) = %02x, %02x Pj = %02x\n", port,
            p_hdcp->hdcp14_ri_count,
            ri0,
            ri1,
            Pj);

        if((ri0 == 0x00) && (ri1 == 0x00))
        {
            HDMI_EMG("HDCP14 Ri calculate fail!!\n");
        }
    }

    //---------------------------------------------
    // for HDCP2
    //---------------------------------------------

    if (lib_hdmi_hdcp22_is_onoff(port) && lib_hdmi_hdcp22_is_wr_msg_done_irq_en(port))
    {
        unsigned long flags;
        unsigned long hdcp2_recover = 0;

        spin_lock_irqsave(&hdmi_spin_lock, flags);  // make sure no irq handeler is running

        if (lib_hdmi_is_hdcp22_event_pending(port) && lib_hdmi_is_irq_pending(port)==0)
        {
            newbase_hdmi_hdcp22_msg_handler(port);
            hdcp2_recover++;
        }

        spin_unlock_irqrestore(&hdmi_spin_lock, flags);

        if (hdcp2_recover) {
            HDCP_WARN("HDMI[%d] HDCP2 event pending but irq pending is not set, force process it\n", port);
        }
    }

  #if HDMI_CLEAR_HDCP22_SRAM_PATCH
    // No HDCP1.4 and No clock for a while, then reset to HDCP2.2
    if (newbase_rxphy_get_clk(port) < VALID_MIN_CLOCK)
    {
        if (p_hdcp->hdcp_init_period)
        {
            p_hdcp->hdcp_init_period--;
        }
        else
        {
            if (!lib_hdmi_hdcp22_is_onoff(port) && (newbase_hdmi_is_reset_hdcp_once(port) == FALSE))
            {
                HDCP_INFO("HDMI[p%d] No clock, reset to Initial Setting\n", port);
                newbase_hdmi_hdcp_reset_fsm(port);
                newbase_hdmi_reset_hdcp_once(port, TRUE);
            }
        }
    }
    else 
    {
        unsigned char RxStatus[2];

        if(newbase_hdcp_get_auth_mode(port) != HDCP22)
            p_hdcp->hdcp2_wait_mesage_timout = 0;


        if(p_hdcp->hdcp_reauth_cnt <= HDCP_REAUTH_MESSAGE_FINISHED_THD)
        {//Print reauth status
            lib_hdmi_hdcp22_get_rx_status(port, RxStatus);

            if ((RxStatus[1] & 0x8))
            {
                if ((p_hdcp->hdcp_reauth_cnt++ % g_hdcp_reauth_message_threshould)==0) // limit reauth debug message output
                {
                    HDCP_INFO("HDMI[p%d] %s RxReauth is set Status(%02x, %02x) BCHErr(%d,%d), Enc=%d, AVmute=%d, Cnt=%d\n",
                        port, _hdmi_hdcp_mode_str(newbase_hdcp_get_auth_mode(port)), RxStatus[0], RxStatus[1],
                        (hdmi_rx[port].bch_err_detect) ? 1 : 0,
                        (hdmi_rx[port].bch_err_cnt_continue),
                        (p_hdcp->hdcp_enc) ? 1 : 0,
                        lib_hdmi_get_avmute(port),
                        p_hdcp->hdcp_reauth_cnt);
                }
            }
            else
            {
                if (p_hdcp->hdcp_reauth_cnt)
                {
                    HDCP_INFO("HDMI[p%d] %s RxReauth is unset Status(%02x, %02x) BitErr(%d,%d), Enc=%d, AVmute=%d, Cnt=%d\n",
                        port, _hdmi_hdcp_mode_str(newbase_hdcp_get_auth_mode(port)), RxStatus[0], RxStatus[1],
                        (hdmi_rx[port].bch_err_detect) ? 1 : 0,
                        (hdmi_rx[port].bch_err_cnt_continue),
                        (p_hdcp->hdcp_enc) ? 1 : 0,
                        lib_hdmi_get_avmute(port),
                        p_hdcp->hdcp_reauth_cnt);
                }

                p_hdcp->hdcp_reauth_cnt = 0;   // reset reauth message count
            }
        }

        if (p_hdcp->hdcp2_wait_mesage_timout &&
            time_after(jiffies, p_hdcp->hdcp2_wait_mesage_timout))
        {
            HDCP_WARN("HDMI[p%d] hdcp2_wait_mesage_timout, force reauth\n", port);
            newbase_hdmi_hdcp22_set_reauth(port);
            p_hdcp->hdcp2_wait_mesage_timout = 0;
        }
        newbase_hdmi_hdcp22_check_iic_status(port);
    }
  #endif
    newbase_hdmi_hdcp_repeater_handler(port);
}
#endif

HDMI_HDCP_E newbase_hdcp_get_auth_mode(unsigned char port)
{
    HDMI_HDCP_E running_hdcp_mode = HDCP_OFF;
    HDMI_HDCP_ST* p_hdcp = newbase_hdcp_get_hdcp_status(port);
    
    if (lib_hdmi_hdcp22_is_onoff(port)==0 && lib_hdmi_hdcp14_is_onoff(port)==0)
        running_hdcp_mode =  HDCP_OFF;
    else if ((p_hdcp->hdcp2_auth_count > 0) && (p_hdcp->hdcp14_auth_count ==0))
        running_hdcp_mode = HDCP22;
    else if ((p_hdcp->hdcp2_auth_count == 0) && (p_hdcp->hdcp14_auth_count > 0))
        running_hdcp_mode = HDCP14;
    else if ((p_hdcp->hdcp2_auth_count == 0) && (p_hdcp->hdcp14_auth_count == 0))
        running_hdcp_mode = NO_HDCP;
    else
    {
        HDMI_EMG("[newbase_hdcp_get_auth_mode] HDCP1.4 and 2.2 auth duration very close, port = %d (hdcp22_en=%d (cnt=%d), hdcp14_en=%d (cnt=%d))\n", 
            port, 
            lib_hdmi_hdcp22_is_onoff(port), p_hdcp->hdcp2_auth_count, 
            lib_hdmi_hdcp14_is_onoff(port), p_hdcp->hdcp14_auth_count);
        running_hdcp_mode =  HDCP_OFF;
    }
    //HDMI_INFO("Get running auth HDCP port = %d, auth mode = %d, \n", port, running_hdcp_mode);
    return running_hdcp_mode;
}

#ifndef BUILD_QUICK_SHOW
//------------------------------------------------------------------
//  HDCP 1.4 related
//------------------------------------------------------------------

/*------------------------------------------------------------------
 * Func : newbase_hdmi_hdcp14_write_hdcp_key
 *
 * Desc : read hdcp1.4 key
 *
 * Para : [IN] bksv  : bksv
 *        [IN] hdcp_key  : 320 bytes hdcp key
 *        [IN] mode  : control mode
 *           APPLY_KEY_IMMEDATELY : apply it to hdcp hw
 *
 * Retn : N/A
 *------------------------------------------------------------------*/ 
void newbase_hdmi_hdcp14_write_hdcp_key(
    unsigned char         bksv[5], 
    unsigned char         hdcp_key[320], 
    unsigned char         mode
    )
{                    
    memcpy(hdcpkey.BKsv, bksv, 5);
    memcpy(hdcpkey.Key,  hdcp_key, 320);
    
    if (mode==APPLY_KEY_IMMEDATELY)
    {
        unsigned char i;
        
        for (i=0; i<HDMI_PORT_TOTAL_NUM; i++)
            lib_hdmi_hdcp14_write_hdcpkey(i, bksv, hdcp_key);         
    }
}

/*------------------------------------------------------------------
 * Func : newbase_hdmi_hdcp14_read_hdcp_key
 *
 * Desc : read hdcp1.4 key
 *
 * Para : [IN] bksv  : bksv
 *        [IN] bksv  : bksv
 *
 * Retn : N/A
 *------------------------------------------------------------------*/ 
void newbase_hdmi_hdcp14_read_hdcp_key(
    unsigned char           bksv[5],
    unsigned char           hdcp_key[320]
    )
{
    lib_hdmi_hdcp14_read_hdcpkey(0, bksv, hdcp_key);
}

/*------------------------------------------------------------------
 * Func : newbase_hdmi_hdcp14_read_aksv
 *
 * Desc : read hdcp1.4 key (legacy mode, output in little endian format)
 *
 * Para : [IN] bksv  : bksv
 *        [IN] bksv  : bksv
 *
 * Retn : N/A
 *------------------------------------------------------------------*/ 
unsigned char newbase_hdmi_hdcp14_read_aksv(unsigned char port, unsigned char *pAKS)
{
    return newbase_hdmi_hdcp14_read_aksv_ex(port, pAKS, 0); // return in little endian mode
}

/*------------------------------------------------------------------
 * Func : newbase_hdmi_hdcp14_read_aksv
 *
 * Desc : read hdcp1.4 key (big or little endian)
 *
 * Para : [IN]  port  : bksv
 *        [OUT] pAKSV  : AKSV list
 *        [IN]  big_endian  : 0 : little, others : big endian
 *
 * Retn : N/A
 *------------------------------------------------------------------*/ 
unsigned char newbase_hdmi_hdcp14_read_aksv_ex(unsigned char port, unsigned char *pAKSV, unsigned char big_endian)
{
    unsigned char i;
    unsigned char aksv_zero[5]={0,0,0,0,0};
    HDMI_HDCP_ST* p_hdcp = newbase_hdcp_get_hdcp_status(port);
    
    
    if (lib_hdmi_hdcp22_is_onoff(port)==0 && lib_hdmi_hdcp14_is_onoff(port))
    {
        for (i=0; i<5; i++){
            pAKSV[i] = (big_endian) ? p_hdcp->tx_aksv[4-i]: p_hdcp->tx_aksv[i];
        }
    }
    else
    {
        memset(pAKSV, 0, sizeof(unsigned char)*5);
    }
    
    if(memcmp((unsigned char*)p_hdcp->tx_aksv,(unsigned char*)aksv_zero, 5) ==0)
        return 0;
    else
        return 1;
}

void newbase_hdmi_save_hdcp(const HDCP_KEY_T *key)
{
    memcpy(&hdcpkey, key, sizeof(HDCP_KEY_T));
}

void newbase_hdmi_save_customer_hdcp1p4(unsigned char* hdcp_1p4, unsigned int len )
{
    if(hdcp_1p4)//save Customer hdcp1p4 to hdcpkey
    {
        memcpy(hdcpkey.BKsv , hdcp_1p4, 5);
        memcpy(hdcpkey.Key, hdcp_1p4+5, 320);

        HDMI_EMG("Customer HDCP1p4:%d,%d,%d,%d,%d,len=%d\n",hdcpkey.Key[0],hdcpkey.Key[1],hdcpkey.Key[2],hdcpkey.Key[3],hdcpkey.Key[4],len);
        customer_hdcp1p4=TRUE;
    }
    else
        customer_hdcp1p4=FALSE;

}
void newbase_hdmi_save_customer_hdcp2p2(unsigned char* hdcp_2p2, unsigned int len )
{
#ifdef CONFIG_OPTEE_HDCP2
	unsigned char uc_lc128[16]={0};

	if(hdcp_2p2 == NULL || len < 858)
	{
		HDMI_EMG("ap burn key fail p=%lx,len=%d\n",(unsigned long)hdcp_2p2,len);
		return;
	}
	optee_hdcp2_main(HDMI_HDCP2_MODE, HDCP2_CMD_AP_BURN_KEY, 0, hdcp_2p2, len,uc_lc128,16);
#else
	unsigned char j;
	unsigned char uc_lc128[16]={0};
	
	h2p2table = kmalloc(len, GFP_KERNEL);//kernal space
	if(h2p2table)
	{
		memcpy(h2p2table, hdcp_2p2, len);
		HDMI_EMG("Customer HDCP2p2:%d,%d,%d,%d,%d,len=%d\n",h2p2table[0],h2p2table[1],h2p2table[2],h2p2table[3],h2p2table[4],len);
		customer_hdcp2p2=TRUE;
		
		hdcp2_load_from_flash();
		hdcp2_get_lc128(uc_lc128);

		for (j=0; j<HDMI_PORT_TOTAL_NUM; j++) {
			if (newbase_hdmi_phy_port_status(j) == HDMI_PHY_PORT_NOTUSED)
				continue;
			lib_hdmi_hdcp22_init(j,uc_lc128);
			newbase_hdmi_hdcp_enable(j, 0);
		}
	}
	else
		customer_hdcp2p2=FALSE;
#endif
}


unsigned char newbase_hdmi_get_customer_hdcp1p4(void)
{
    return customer_hdcp1p4;
}

unsigned char newbase_hdmi_get_customer_hdcp2p2(void)
{
    return customer_hdcp2p2;
}

unsigned char* newbase_hdmi_get_customer_hdcp2p2_table(void)
{
    return h2p2table;
}

void newbase_hdmi_clear_table(void)
{
    if (h2p2table)
        kfree(h2p2table);

    customer_hdcp1p4 = FALSE;
    customer_hdcp2p2= FALSE;
}
#endif

/*------------------------------------------------------------------
 * Func : newbase_hdmi_hdcp_disable_hdcp2
 *
 * Desc : disable hdcp2 functionality
 *
 * Para : disable : 0 : not disable hdcp2 
 *                  1 : disable hdcp2
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void newbase_hdmi_hdcp_disable_hdcp2(unsigned char port, unsigned char disable)
{
    int i;

    if (port==EVERY_HDMI_PORT)
    {
        SET_FLOW_CFG(HDMI_FLOW_CFG_GENERAL, HDMI_FLOW_CFG0_HDCP22_DISABLE, (disable>0)?1:0);
    
        HDCP_WARN("HDMI set HDCP2 disable=%d\n", disable);
    }
            
    for (i=0; i<HDMI_PORT_TOTAL_NUM; i++)
    {
        if ((port != EVERY_HDMI_PORT) && (port != i))
            continue;

        if (newbase_hdmi_phy_port_status(i)==HDMI_PHY_PORT_NOTUSED)
            continue;

        lib_hdmi_hdcp_port_write(i, 0x50, (newbase_hdmi_hdcp_is_hdcp2_disabled(port)) ? 0x0 : 0x4);
        hdmi_port_hdcp2_disable[i] = (disable) ? 1 : 0;
        newbase_hdmi_switch_hdcp_mode(i, ((disable) ? HDCP14 : newbase_hdcp_get_auth_mode(i)));
    }
}


/*------------------------------------------------------------------
 * Func : newbase_hdmi_hdcp_is_hdcp2_disabled
 *
 * Desc : check hdcp function of a given hdcp port is disabled
 *
 * Para : port    : port id
 *        disable : 0 : not disable hdcp2 
 *                  1 : disable hdcp2
 *
 * Retn : 0 : no disable hdcp2, 1 : hdcp2 disabled
 *------------------------------------------------------------------*/
int newbase_hdmi_hdcp_is_hdcp2_disabled(unsigned char port)
{      
    if (port==EVERY_HDMI_PORT)    
        return GET_FLOW_CFG(HDMI_FLOW_CFG_GENERAL, HDMI_FLOW_CFG0_HDCP22_DISABLE);  // global hdcp2 disable flag
        
    if (newbase_hdmi_phy_port_status(port)==HDMI_PHY_PORT_NOTUSED)
        return 0;
    
    return (GET_FLOW_CFG(HDMI_FLOW_CFG_GENERAL, HDMI_FLOW_CFG0_HDCP22_DISABLE) || hdmi_port_hdcp2_disable[port]) ? 1 : 0;
}


unsigned char newbase_hdmi_hdcp14_read_ri(unsigned char port,unsigned char Ri[2])
{
    unsigned char ri_zero[2]={0,0};
    memset(Ri, 0, sizeof(unsigned char)*2);

    if(lib_hdmi_hdcp22_is_onoff(port)==0 && lib_hdmi_hdcp14_is_onoff(port))
    {
        Ri[0] = lib_hdmi_hdcp_port_read(port, 0x08);
        Ri[1] = lib_hdmi_hdcp_port_read(port, 0x09);
    }

    if(memcmp((unsigned char*)Ri,(unsigned char*)ri_zero, 2) ==0)
        return 0;
    else
        return 1;
}

void newbase_hdmi_hdcp14_read_an(unsigned char port,unsigned char An[8])
{
    newbase_hdmi_hdcp14_read_an_ex(port, An, 0);
}

void newbase_hdmi_hdcp14_read_an_ex(unsigned char port,unsigned char An[8], unsigned char big_endian)
{
    memset(An, 0, sizeof(unsigned char)*8);

    if(lib_hdmi_hdcp22_is_onoff(port)==0 && lib_hdmi_hdcp14_is_onoff(port))
    {
        if (big_endian)
        {
            An[7] = lib_hdmi_hdcp_port_read(port, 0x18);
            An[6] = lib_hdmi_hdcp_port_read(port, 0x19);
            An[5] = lib_hdmi_hdcp_port_read(port, 0x1a);
            An[4] = lib_hdmi_hdcp_port_read(port, 0x1b);
            An[3] = lib_hdmi_hdcp_port_read(port, 0x1c);
            An[2] = lib_hdmi_hdcp_port_read(port, 0x1d);
            An[1] = lib_hdmi_hdcp_port_read(port, 0x1e);
            An[0] = lib_hdmi_hdcp_port_read(port, 0x1f);
        }
        else
        {
            An[0] = lib_hdmi_hdcp_port_read(port, 0x18);
            An[1] = lib_hdmi_hdcp_port_read(port, 0x19);
            An[2] = lib_hdmi_hdcp_port_read(port, 0x1a);
            An[3] = lib_hdmi_hdcp_port_read(port, 0x1b);
            An[4] = lib_hdmi_hdcp_port_read(port, 0x1c);
            An[5] = lib_hdmi_hdcp_port_read(port, 0x1d);
            An[6] = lib_hdmi_hdcp_port_read(port, 0x1e);
            An[7] = lib_hdmi_hdcp_port_read(port, 0x1f);
        }
    }
}

void newbase_hdmi_hdcp14_read_bksv(unsigned char port,unsigned char Bksv[5])
{
    memset(Bksv, 0, sizeof(unsigned char)*5);

    if(lib_hdmi_hdcp22_is_onoff(port)==0 && lib_hdmi_hdcp14_is_onoff(port))
    {
        Bksv[0] = lib_hdmi_hdcp_port_read(port, 0x0);
        Bksv[1] = lib_hdmi_hdcp_port_read(port, 0x1);
        Bksv[2] = lib_hdmi_hdcp_port_read(port, 0x2);
        Bksv[3] = lib_hdmi_hdcp_port_read(port, 0x3);
        Bksv[4] = lib_hdmi_hdcp_port_read(port, 0x4);
    }
}

void newbase_hdmi_hdcp14_read_bcaps(unsigned char port,unsigned char *pBcaps)
{
    *pBcaps = lib_hdmi_hdcp_port_read(port, 0x40);    
}

void newbase_hdmi_hdcp14_write_bcaps(unsigned char port,unsigned char bcaps)
{
    lib_hdmi_hdcp_port_write(port, 0x40, bcaps);    
}

void newbase_hdmi_hdcp14_read_bstatus(unsigned char port,unsigned char Bstatus[2])
{
    newbase_hdmi_hdcp14_read_bstatus_ex(port, Bstatus, 0);
}

void newbase_hdmi_hdcp14_read_bstatus_ex(unsigned char port,unsigned char Bstatus[2], unsigned char big_endian)
{
    memset(Bstatus, 0, sizeof(unsigned char)*2);
    if (big_endian)
    {
        Bstatus[0] = lib_hdmi_hdcp_port_read(port, 0x42);
        Bstatus[1] = lib_hdmi_hdcp_port_read(port, 0x41);
    }
    else
    {
        Bstatus[0] = lib_hdmi_hdcp_port_read(port, 0x41);
        Bstatus[1] = lib_hdmi_hdcp_port_read(port, 0x42);
    }
}

void newbase_hdmi_hdcp14_write_bstatus(unsigned char port,unsigned char Bstatus[2])
{        
    lib_hdmi_hdcp_port_write(port, 0x41, Bstatus[0]);
    lib_hdmi_hdcp_port_write(port, 0x42, Bstatus[1]);    
}

void newbase_hdmi_hdcp14_write_ksv_fifo(unsigned char port, unsigned char* p_ksv_fifo, unsigned int len)
{
#if HDMI_FIX_RL6672_3031_HDCP14_KSV_FIFO
    lib_hdmi_hdcp22_write_data_to_tx(port, p_ksv_fifo, len);  // for K7, we use HDCP22 message fifo as KSV fifo
#endif
}

void newbase_hdmi_hdcp14_write_v_prime(unsigned char port,unsigned char v_prime[20])
{        
    // H0 : LSB first
    lib_hdmi_hdcp_port_write(port, 0x20, v_prime[3]);
    lib_hdmi_hdcp_port_write(port, 0x21, v_prime[2]);
    lib_hdmi_hdcp_port_write(port, 0x22, v_prime[1]);
    lib_hdmi_hdcp_port_write(port, 0x23, v_prime[0]);
    // H1 : LSB first
    lib_hdmi_hdcp_port_write(port, 0x24, v_prime[7]);
    lib_hdmi_hdcp_port_write(port, 0x25, v_prime[6]);
    lib_hdmi_hdcp_port_write(port, 0x26, v_prime[5]);
    lib_hdmi_hdcp_port_write(port, 0x27, v_prime[4]);    
    // H2 : LSB first
    lib_hdmi_hdcp_port_write(port, 0x28, v_prime[11]);
    lib_hdmi_hdcp_port_write(port, 0x29, v_prime[10]);
    lib_hdmi_hdcp_port_write(port, 0x2A, v_prime[9]);
    lib_hdmi_hdcp_port_write(port, 0x2B, v_prime[8]);    
    // H3 : LSB first
    lib_hdmi_hdcp_port_write(port, 0x2C, v_prime[15]);
    lib_hdmi_hdcp_port_write(port, 0x2D, v_prime[14]);
    lib_hdmi_hdcp_port_write(port, 0x2E, v_prime[13]);
    lib_hdmi_hdcp_port_write(port, 0x2F, v_prime[12]);         
    // H4 : LSB first
    lib_hdmi_hdcp_port_write(port, 0x30, v_prime[19]);
    lib_hdmi_hdcp_port_write(port, 0x31, v_prime[18]);
    lib_hdmi_hdcp_port_write(port, 0x32, v_prime[17]);
    lib_hdmi_hdcp_port_write(port, 0x33, v_prime[16]);      
}

unsigned char* newbase_hdmi_get_default_hdcp14_bksv(void)
{
    unsigned char* ptr = (unsigned char* )default_hdcpkey.BKsv;
    return ptr;
}

unsigned char* newbase_hdmi_get_default_hdcp14_key(void)
{
    unsigned char* ptr = (unsigned char* )default_hdcpkey.Key;
    return ptr;
}

void newbase_hdmi_hdcp14_set_aksv_intr(unsigned char port, unsigned char en)
{
	unsigned char is_enabled = lib_hdmi_hdcp14_get_aksv_wr_irq_en(port);

	//HDCP_FLOW_PRINTF("HDMI[p%d] HDCP14 AKSV intr %d -> %d\n", port, is_enabled, en);

	if (GET_FLOW_CFG(HDMI_FLOW_CFG_GENERAL, HDMI_FLOW_CFG0_DISABLE_AKSV_INTERRUPT))
	{
		if (en != 0) {
			//HDCP_FLOW_PRINTF("HDMI[p%d] force disable aksv interrupt\n", port);
			en = 0;
		}
	}

	if (is_enabled != en) {
		lib_hdmi_hdcp14_set_aksv_wr_irq(port, en);
	}
}

int newbase_hdmi_hdcp14_interrupt_handler(unsigned char port)
{
	int ret = 0;

	if (lib_hdmi_hdcp14_get_aksv_wr_irq_en(port) && lib_hdmi_hdcp14_check_aksv_reg(port))
	{
		HDCP_FLOW_PRINTF("HDMI[p%d] HDCP14 detect AKSV intr!!!\n", port);

		newbase_hdcp_aksv_handler(port);

		newbase_rxphy_set_phy_for_aksv(port);

		lib_hdmi_hdcp14_clear_aksv_reg(port);

		ret = 1;
	}

	if (lib_hdmi_hdcp14_is_irq_pending(port))
		lib_hdmi_hdcp14_clr_irq_pending(port);  // clear hdcp14 dedicate irq

	return ret;
}
