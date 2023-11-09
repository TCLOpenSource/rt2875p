#include "hdmi_common.h"
#include "hdmi_reg.h"
#include "hdmi_vfe_config.h"
#include "hdmi_emp.h"
#include "hdmiInternal.h"

//#define HDMI_EMP_DEBUG_EN
//#define HDMI_EMP_CHECK_DEBUG_EN  // enable EMP check debug message

#define NOT_PER_PORT  0
#define PER_PORT      1

extern HDMI_PORT_INFO_T hdmi_rx[HDMI_PORT_TOTAL_NUM];
extern spinlock_t hdmi_spin_lock;

const EMP_PARAM hdr_emp_cfg =
{
    .sync = 1,                          // receive vsem within fapa region
    .recognize_tag = 0,                 // for CTA861G HDR EMP, it should be 0 (depends on info frame type code)
    .recognize_oui = 0,                 // for CTA861G HDR EMP, it should be 0 (depends on info frame type code)
    .org_id  = EMP_ORG_ID_2_CTA_861_G,  // for CTA861G HDR EMP, it should be 2
    .data_set_tag = {0, 0},             // for CTA861G HDR EMP, this field is used to store Extended InfoFrame Type Code
    .oui = {0, 0, 0},                   // for CTA861G HDR EMP, no OUI exits
};

const EMP_PARAM cvtem_emp_cfg =
{
    .sync = 1,
    .recognize_tag = 0,
    .recognize_oui = 0,
    .org_id  = EMP_ORG_ID_1_HDMI_2p1,
    .data_set_tag = {0, 0},
    .oui = {0, 0, 0},
};


/*------------------------------------------------------------------
 * Name : default_vsem_emp_cfg
 *
 * Desc : config of VSEM by default
 *
 * Refer : This config allow us to receive all kinds of VSEM
 *         it could be also used for VSEM packet detection
 *------------------------------------------------------------------*/
const EMP_PARAM default_vsem_emp_cfg =
{
    .sync = 0,                  // receive vsem all the time...
    .recognize_tag = 0,         // do not recognize data set tag
    .recognize_oui = 0,         // do not recognize OUI
    .org_id = 0,                // for VSEM, it must be 0
    .data_set_tag = {0, 0},     // don't care
    .oui = { 0x00, 0x00, 0x00 },// don't care
};

/*------------------------------------------------------------------
 * Name : dolby_vsem_emp_cfg
 *
 * Desc : config of dolby VSEM
 *
 * Refer : Dolby Vision HDMI 2.1 transmission specification 
 *         section 5.2.1
 *------------------------------------------------------------------*/
const EMP_PARAM dolby_vsem_emp_cfg =
{
    .sync = 1,                  // receive vsem within fapa region
    .recognize_tag = 1,
    .recognize_oui = 1,
    .org_id = 0,                // for Dolby VSEM, it should be 0
    .data_set_tag = {0, 0},     // for Dolby VSEM, it should be  0
    .oui = { 0x00, 0xD0, 0x46 },//  for Dolby VSEM, it should be 0x00D046
};

/*
static void dump_mem(unsigned char* p_data, unsigned int len, const char* text)
{
    int i;
    for (i=0; i<len; i+= 16)
    {
        HDMI_EMG("%s[%d]= %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    text, i,
                    p_data[0], p_data[1], p_data[2], p_data[3],
                    p_data[4], p_data[5], p_data[6], p_data[7],
                    p_data[8], p_data[9], p_data[10], p_data[11],
                    p_data[12], p_data[13], p_data[14], p_data[15]);
        p_data+= 16;
    }
}
*/

/*---------------------------------------------------
 * Func : lib_hdmi_read_emp_sram
 *
 * Desc : read EM packet SRAM
 *
 * Para : nport : HDMI port number
 *        start_addr : start address of packet sram
 *        len   : read length
 *        pbuf  : buffer
 *        is_pp: is per port
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
void lib_hdmi_read_emp_sram(
    unsigned char           nport,
    unsigned int            start_addr,
    unsigned char           len,
    unsigned char*          pbuf,
    unsigned char           is_pp
    )
{
    unsigned int i;
    unsigned long flags;

    spin_lock_irqsave(&hdmi_spin_lock, flags);
    if(is_pp==NOT_PER_PORT)
    {
        if (hdmi_rx[nport].hdmi_2p1_en)
        {
            return; //Not support HDMI2.1
        }
        else
        {
            for (i=0; i<len; i++)
            {
                hdmi_out(HDMI_HDMI_EMAP_reg, start_addr+i);
                pbuf[i] = hdmi_in(HDMI_HDMI_EMDP_reg);
            }
        }
    }
    else
    {
        if (hdmi_rx[nport].hdmi_2p1_en)
        {
            return; //Not support HDMI2.1
        }
        else
        {
            for (i=0; i<len; i++)
            {
                hdmi_out(HDMI_HDMI_EMAP_PP_reg, start_addr+i);
                pbuf[i] = hdmi_in(HDMI_HDMI_EMDP_PP_reg);
            }
        }
    }

    spin_unlock_irqrestore(&hdmi_spin_lock, flags);
}


/*---------------------------------------------------
 * Func : lib_hdmi_emp_enable_all_mode
 *
 * Desc : enable/disable emp all mode. 
 *
 * Para : nport : HDMI port number
 *        enable : 0 : disable  (only receive data within fapa region)
 *                 1 : enable
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
void lib_hdmi_emp_enable_all_mode(
    unsigned char           nport,
    unsigned int            enable
    )
{
    if (hdmi_rx[nport].hdmi_2p1_en)
        return;
    else
        hdmi_mask(HDMI_fapa_ct_tmp_reg, ~HDMI_fapa_ct_tmp_all_mode_mask,
                (enable) ? HDMI_fapa_ct_tmp_all_mode_mask : 0);
}

//----------------------------------------------------------------------------------------
// VCTEM
//---------------------------------------------------------------------------------------

/*---------------------------------------------------
 * Func : lib_hdmi_is_em_cvtem_received
 *
 * Desc : check if CVT EM packet received
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
int lib_hdmi_is_em_cvtem_received(unsigned char nport)
{
    // for HD21 only,... do nothing
    return 0;
}


/*---------------------------------------------------
 * Func : lib_hdmi_clear_em_cvtem_received_status
 *
 * Desc : clear CVTEM packet receive status
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
int lib_hdmi_clear_em_cvtem_received_status(unsigned char nport)
{
    // for HD21 only,... do nothing
    return 0;
}


/*---------------------------------------------------
 * Func : lib_hdmi_read_em_cvtem_packet
 *
 * Desc : read CVTEM packet receive status
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
int lib_hdmi_read_em_cvtem_packet(
    unsigned char           nport,
    unsigned char           pkt_buf[EM_CVTEM_INFO_LEN]
    )
{
    // for HD21 only,... do nothing
    return 0;
}


void _dump_cvtem_info(CVTEM_INFO* p_info)
{
    int i;

    HDMI_EMP_INFO("CVTEM: hfront = %d, hsync = %d, hback=%d, hcative_bytes=%d\n",
                    p_info->hfront, p_info->hsync, p_info->hback, p_info->hcative_bytes);

    for (i=0; i<sizeof(p_info->pps); i+=16)
    {
        unsigned char* pps = &p_info->pps[i];

        HDMI_EMP_INFO("CVTEM:PPS[%d] = %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    i,
                    pps[0], pps[1], pps[2], pps[3],
                    pps[4], pps[5], pps[6], pps[7],
                    pps[8], pps[9], pps[10], pps[11],
                    pps[12], pps[13], pps[14], pps[15]);
    }
}


int _parse_cvtem_info(unsigned char pkt_buf[EM_CVTEM_INFO_LEN], CVTEM_INFO* p_info)
{
    EMP_PKT* p_emp = (EMP_PKT*) pkt_buf;

    // copy pps
    memcpy(&p_info->pps[0],   &p_emp[0].pb[7], 21);
    memcpy(&p_info->pps[21],  &p_emp[1].pb[0], 28);
    memcpy(&p_info->pps[49],  &p_emp[2].pb[0], 28);
    memcpy(&p_info->pps[77],  &p_emp[3].pb[0], 28);
    memcpy(&p_info->pps[105], &p_emp[4].pb[0], 23);

    // parse hfront
    p_info->hfront = (p_emp[4].pb[24]<<8) + p_emp[4].pb[23];
    p_info->hsync  = (p_emp[4].pb[26]<<8) + p_emp[4].pb[25];
    p_info->hback  = (p_emp[5].pb[0]<<8)  + p_emp[4].pb[27];
    p_info->hcative_bytes = (p_emp[5].pb[2]<<8)  + p_emp[2].pb[1];

    return 0;
}

//-------------------------------------------------------------------------
// SBTM (Source-Based Tone Mapping)
//-------------------------------------------------------------------------


/*---------------------------------------------------
 * Func : lib_hdmi_is_em_sbtm_received
 *
 * Desc : check if SBTM EM packet has been received
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
int lib_hdmi_is_em_sbtm_received(unsigned char nport)
{
    return HDMI_SBTM_ST_get_em_sbtm_first(hdmi_in(HDMI_SBTM_ST_reg));
}


/*---------------------------------------------------
 * Func : lib_hdmi_clear_em_sbtm_received_status
 *
 * Desc : clear SBTM packet receive status
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : successed, others : failed
 *--------------------------------------------------*/
int lib_hdmi_clear_em_sbtm_received_status(unsigned char nport)
{
    if (hdmi_rx[nport].hdmi_2p1_en)
    {
        return 0;
    }
    else
    {
        // write 1 clear first/last flag of emp
        hdmi_mask(HDMI_SBTM_ST_reg, ~(HDMI_SBTM_ST_em_sbtm_first_mask | HDMI_SBTM_ST_em_sbtm_last_mask), (HDMI_SBTM_ST_em_sbtm_first_mask | HDMI_SBTM_ST_em_sbtm_last_mask));

        hdmi_mask(HDMI_HDMI_SBTM_EMC_reg, ~HDMI_HDMI_SBTM_EMC_sbtm_clr_mask, HDMI_HDMI_SBTM_EMC_sbtm_clr_mask);
        hdmi_mask(HDMI_HDMI_SBTM_EMC_reg, ~HDMI_HDMI_SBTM_EMC_sbtm_clr_mask, 0);
    }

    return 0;
}


/*---------------------------------------------------
 * Func : lib_hdmi_read_em_sbtm_packet
 *
 * Desc : read SBTM packet receive status
 *
 * Para : nport : HDMI port number
 *
 * Retn : > 0 : number of byte received, <0 : failed
 *--------------------------------------------------*/
int lib_hdmi_read_em_sbtm_packet(
    unsigned char           nport,
    unsigned char           pkt_buf[EM_SBTM_INFO_LEN]
    )
{
    if (pkt_buf==NULL || !lib_hdmi_is_em_sbtm_received(nport))
        return -1;

    lib_hdmi_read_emp_sram(nport, EM_SBTM_INFO_OFST + EM_SBTM_INFO_LEN-4, 4, pkt_buf, NOT_PER_PORT);  // HB0~2 + BCH

    if (pkt_buf[3]!=0) {
        HDMI_WARN("SBTM BCH Error= %02x\n", pkt_buf[3]);
    }

    lib_hdmi_read_emp_sram(nport, EM_SBTM_INFO_OFST, EM_SBTM_INFO_LEN - 4, pkt_buf+3, NOT_PER_PORT);  // PB0~PB27

    lib_hdmi_clear_em_sbtm_received_status(nport);

    HDMI_EMP_DBG("SBTM= %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \n",
                pkt_buf[0], pkt_buf[1],
                pkt_buf[2], pkt_buf[3],
                pkt_buf[4], pkt_buf[5],
                pkt_buf[6], pkt_buf[7],
                pkt_buf[8], pkt_buf[9],
                pkt_buf[10], pkt_buf[11],
                pkt_buf[12], pkt_buf[13]);

    return 0;
}


//-------------------------------------------------------------------------
// RSV0 ()
//-------------------------------------------------------------------------


/*---------------------------------------------------
 * Func : lib_hdmi_is_em_rsv0_received
 *
 * Desc : check if RSV0 EM packet has been received
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
int lib_hdmi_is_em_rsv0_received(unsigned char nport)
{
    return HDMI_HD20_RSV0_FOR_EM_ST_get_em_rsv0_first(hdmi_in(HDMI_HD20_RSV0_FOR_EM_ST_reg));
}


/*---------------------------------------------------
 * Func : lib_hdmi_clear_em_rsv0_received_status
 *
 * Desc : clear RSV0 packet receive status
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : successed, others : failed
 *--------------------------------------------------*/
int lib_hdmi_clear_em_rsv0_received_status(unsigned char nport)
{
    if (hdmi_rx[nport].hdmi_2p1_en)
    {
        return 0;
    }
    else
    {
        // write 1 clear first/last flag of emp
        hdmi_mask(HDMI_HD20_RSV0_FOR_EM_ST_reg, ~(HDMI_HD20_RSV0_FOR_EM_ST_em_rsv0_first_mask | HDMI_HD20_RSV0_FOR_EM_ST_em_rsv0_last_mask), (HDMI_HD20_RSV0_FOR_EM_ST_em_rsv0_first_mask | HDMI_HD20_RSV0_FOR_EM_ST_em_rsv0_last_mask));

        hdmi_mask(HDMI_HD20_RSV0_EM_CT_reg, ~HDMI_HD20_RSV0_EM_CT_rsv0_em_clr_mask, HDMI_HD20_RSV0_EM_CT_rsv0_em_clr_mask);
        hdmi_mask(HDMI_HD20_RSV0_EM_CT_reg, ~HDMI_HD20_RSV0_EM_CT_rsv0_em_clr_mask, 0);
    }

    return 0;
}


/*---------------------------------------------------
 * Func : lib_hdmi_read_em_rsv0_packet
 *
 * Desc : read RSV0 packet receive status
 *
 * Para : nport : HDMI port number
 *
 * Retn : > 0 : number of byte received, <0 : failed
 *--------------------------------------------------*/
int lib_hdmi_read_em_rsv0_packet(
    unsigned char           nport,
    unsigned char           pkt_buf[RSV0_EM_LEN]
    )
{
    if (pkt_buf==NULL || !lib_hdmi_is_em_rsv0_received(nport))
        return -1;

    lib_hdmi_read_emp_sram(nport, RSV0_EM_OFST + RSV0_EM_LEN-4, 4, pkt_buf, NOT_PER_PORT);  // HB0~2 + BCH

    if (pkt_buf[3]!=0) {
        HDMI_WARN("RSV0 BCH Error= %02x\n", pkt_buf[3]);
    }

    lib_hdmi_read_emp_sram(nport, RSV0_EM_OFST, RSV0_EM_LEN - 4, pkt_buf+3, NOT_PER_PORT);  // PB0~PB27

    lib_hdmi_clear_em_rsv0_received_status(nport);

    HDMI_EMP_DBG("RSV0= %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \n",
                pkt_buf[0], pkt_buf[1],
                pkt_buf[2], pkt_buf[3],
                pkt_buf[4], pkt_buf[5],
                pkt_buf[6], pkt_buf[7],
                pkt_buf[8], pkt_buf[9],
                pkt_buf[10], pkt_buf[11],
                pkt_buf[12], pkt_buf[13]);

    return 0;
}


//-------------------------------------------------------------------------
// RSV1 ()
//-------------------------------------------------------------------------


/*---------------------------------------------------
 * Func : lib_hdmi_is_em_rsv1_received
 *
 * Desc : check if RSV1 EM packet has been received
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
int lib_hdmi_is_em_rsv1_received(unsigned char nport)
{
    return HDMI_HD20_RSV1_FOR_EM_ST_get_em_rsv1_first(hdmi_in(HDMI_HD20_RSV1_FOR_EM_ST_reg));
}


/*---------------------------------------------------
 * Func : lib_hdmi_clear_em_rsv1_received_status
 *
 * Desc : clear RSV1 packet receive status
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : successed, others : failed
 *--------------------------------------------------*/
int lib_hdmi_clear_em_rsv1_received_status(unsigned char nport)
{
    if (hdmi_rx[nport].hdmi_2p1_en)
    {
        return 0;
    }
    else
    {
        // write 1 clear first/last flag of emp
        hdmi_mask(HDMI_HD20_RSV1_FOR_EM_ST_reg, ~(HDMI_HD20_RSV1_FOR_EM_ST_em_rsv1_first_mask | HDMI_HD20_RSV1_FOR_EM_ST_em_rsv1_last_mask), (HDMI_HD20_RSV1_FOR_EM_ST_em_rsv1_first_mask | HDMI_HD20_RSV1_FOR_EM_ST_em_rsv1_last_mask));

        hdmi_mask(HDMI_HD20_RSV1_EM_CT_reg, ~HDMI_HD20_RSV1_EM_CT_rsv1_em_clr_mask, HDMI_HD20_RSV1_EM_CT_rsv1_em_clr_mask);
        hdmi_mask(HDMI_HD20_RSV1_EM_CT_reg, ~HDMI_HD20_RSV1_EM_CT_rsv1_em_clr_mask, 0);
    }

    return 0;
}


/*---------------------------------------------------
 * Func : lib_hdmi_read_em_rsv1_packet
 *
 * Desc : read RSV1 packet receive status
 *
 * Para : nport : HDMI port number
 *
 * Retn : > 0 : number of byte received, <0 : failed
 *--------------------------------------------------*/
int lib_hdmi_read_em_rsv1_packet(
    unsigned char           nport,
    unsigned char           pkt_buf[RSV1_EM_LEN]
    )
{
    if (pkt_buf==NULL || !lib_hdmi_is_em_rsv1_received(nport))
        return -1;

    lib_hdmi_read_emp_sram(nport, RSV1_EM_OFST + RSV1_EM_LEN-4, 4, pkt_buf, NOT_PER_PORT);  // HB0~2 + BCH

    if (pkt_buf[3]!=0) {
        HDMI_WARN("RSV1 BCH Error= %02x\n", pkt_buf[3]);
    }

    lib_hdmi_read_emp_sram(nport, RSV1_EM_OFST, RSV1_EM_LEN - 4, pkt_buf+3, NOT_PER_PORT);  // PB0~PB27

    lib_hdmi_clear_em_rsv1_received_status(nport);

    HDMI_EMP_DBG("RSV1= %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \n",
                pkt_buf[0], pkt_buf[1],
                pkt_buf[2], pkt_buf[3],
                pkt_buf[4], pkt_buf[5],
                pkt_buf[6], pkt_buf[7],
                pkt_buf[8], pkt_buf[9],
                pkt_buf[10], pkt_buf[11],
                pkt_buf[12], pkt_buf[13]);

    return 0;
}


//-------------------------------------------------------------------------
// VTEM (Video Timing Extended Metadata)
//-------------------------------------------------------------------------


/*---------------------------------------------------
 * Func : lib_hdmi_is_em_vtem_received
 *
 * Desc : check if VRR EM packet has been received
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
int lib_hdmi_is_em_vtem_received(unsigned char nport)
{
    return HDMI_VRREM_ST_get_em_vrr_first(hdmi_in(HDMI_VRREM_ST_reg));
}


/*---------------------------------------------------
 * Func : lib_hdmi_clear_em_vtem_received_status
 *
 * Desc : clear VRR packet receive status
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : successed, others : failed
 *--------------------------------------------------*/
int lib_hdmi_clear_em_vtem_received_status(unsigned char nport)
{
    if (hdmi_rx[nport].hdmi_2p1_en)
    {
        return 0;
    }
    else
    {
        // write 1 clear first/last flag of emp
        hdmi_mask(HDMI_VRREM_ST_reg, ~(HDMI_VRREM_ST_em_vrr_first_mask | HDMI_VRREM_ST_em_vrr_last_mask), (HDMI_VRREM_ST_em_vrr_first_mask | HDMI_VRREM_ST_em_vrr_last_mask));

        hdmi_mask(HDMI_HDMI_VRR_EMC_reg, ~HDMI_HDMI_VRR_EMC_vrr_clr_mask, HDMI_HDMI_VRR_EMC_vrr_clr_mask);
        hdmi_mask(HDMI_HDMI_VRR_EMC_reg, ~HDMI_HDMI_VRR_EMC_vrr_clr_mask, 0);
    }

    return 0;
}


/*---------------------------------------------------
 * Func : lib_hdmi_read_em_vtem_packet
 *
 * Desc : read VRR packet receive status
 *
 * Para : nport : HDMI port number
 *
 * Retn : > 0 : number of byte received, <0 : failed
 *--------------------------------------------------*/
int lib_hdmi_read_em_vtem_packet(
    unsigned char           nport,
    unsigned char           pkt_buf[EM_VTEM_INFO_LEN]
    )
{
    unsigned char sram_buf[EM_VTEM_INFO_LEN];
    memset(sram_buf, 0, EM_VTEM_INFO_LEN);

    if (pkt_buf==NULL || !lib_hdmi_is_em_vtem_received(nport))
        return -1;

    lib_hdmi_read_emp_sram(nport, EM_VTEM_INFO_OFST + EM_VTEM_INFO_LEN-4, 4, sram_buf, PER_PORT);  // HB0~2 + BCH

    if (sram_buf[3]!=0) {
        HDMI_WARN("VTEM BCH Error= %02x\n", sram_buf[3]);
    }

    lib_hdmi_read_emp_sram(nport, EM_VTEM_INFO_OFST, EM_VTEM_INFO_LEN - 4, sram_buf+3, PER_PORT);  // PB0~PB27

    lib_hdmi_clear_em_vtem_received_status(nport);

    memcpy(pkt_buf, sram_buf, EM_VTEM_INFO_LEN);

    HDMI_EMP_DBG("VTEM= %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \n",
                pkt_buf[0], pkt_buf[1],
                pkt_buf[2], pkt_buf[3],
                pkt_buf[4], pkt_buf[5],
                pkt_buf[6], pkt_buf[7],
                pkt_buf[8], pkt_buf[9],
                pkt_buf[10], pkt_buf[11],
                pkt_buf[12], pkt_buf[13]);

    return 0;
}

//-------------------------------------------------------------------------
// VSEM
//-------------------------------------------------------------------------


/*---------------------------------------------------
 * Func : lib_hdmi_set_em_vsem_type
 *
 * Desc : set oui of vendor specific em packet
 *
 * Para : nport : HDMI port number
 *        oui   : 24 bits orgination ID
 *        data_set_tag : 16 bits data set tag
 *        sync  : sync of EMP 
 *           0  : non sync (able to receive all the time)
 *           1  : sync mode (only received in FAPA region)
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
int lib_hdmi_set_em_vsem_type(
    unsigned char           nport,
    const EMP_PARAM*        p_emp_cfg
    )
{
    if (p_emp_cfg==NULL)
        return -1;

    if (hdmi_rx[nport].hdmi_2p1_en)
    {
        // Do nothing, not support HDMI 2.1
        return -1;
    }
    else
    {
        hdmi_out(HDMI_HDMI_VSEM_EMC_reg,  HDMI_HDMI_VSEM_EMC_set_tag_msb(p_emp_cfg->data_set_tag[1])|
                                          HDMI_HDMI_VSEM_EMC_set_tag_lsb(p_emp_cfg->data_set_tag[0])|
                                          HDMI_HDMI_VSEM_EMC_recognize_tag_en(p_emp_cfg->recognize_tag));

        hdmi_out(HDMI_HDMI_VSEM_EMC2_reg, HDMI_HDMI_VSEM_EMC2_sync(p_emp_cfg->sync)|
                                          HDMI_HDMI_VSEM_EMC2_recognize_oui_en(p_emp_cfg->recognize_oui)|
                                          HDMI_HDMI_VSEM_EMC2_oui_1st(p_emp_cfg->oui[0])|
                                          HDMI_HDMI_VSEM_EMC2_oui_2nd(p_emp_cfg->oui[1])|
                                          HDMI_HDMI_VSEM_EMC2_oui_3rd(p_emp_cfg->oui[2]));
    }

    return 0;
}


/*---------------------------------------------------
 * Func : lib_hdmi_is_em_vsem_received
 *
 * Desc : check if Vendor Specific EM packet has been received
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
int lib_hdmi_is_em_vsem_received(unsigned char nport)
{
    unsigned int vsemst_reg = 0;

    if (hdmi_rx[nport].hdmi_2p1_en)
    {
        return 0;  // not support HDMI 2.1
    }
    else
    {
        vsemst_reg = HDMI_VSEM_ST_reg;

        if (HDMI_VSEM_ST_get_em_vsem_no(hdmi_in(vsemst_reg)))  // No vsem
        {
            hdmi_out(vsemst_reg, HDMI_VSEM_ST_em_vsem_no_mask);
            return 0;
        }

        if (!HDMI_VSEM_ST_get_em_vsem_first(hdmi_in(vsemst_reg)))  // No First Packet
            return 0;

        if (!HDMI_VSEM_ST_get_em_vsem_last(hdmi_in(vsemst_reg)))  // No Last Packet
            return 0;
    }

    return 1;
}


/*---------------------------------------------------
 * Func : lib_hdmi_clear_em_vsem_received_status
 *
 * Desc : check if Vendor Specific EM packet has been received
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : suc
 *--------------------------------------------------*/
int lib_hdmi_clear_em_vsem_received_status(unsigned char nport)
{
    hdmi_mask(HDMI_HDMI_VSEM_EMC_reg, ~HDMI_HDMI_VSEM_EMC_vsem_clr_mask, HDMI_HDMI_VSEM_EMC_vsem_clr_mask);
    hdmi_mask(HDMI_HDMI_VSEM_EMC_reg, ~HDMI_HDMI_VSEM_EMC_vsem_clr_mask, 0);
    return 0;
}


/*---------------------------------------------------
 * Func : lib_hdmi_read_em_vsem_packet
 *
 * Desc : read VRR packet receive status
 *
 * Para : nport : HDMI port number
 *
 * Retn : > 0 : number of byte received, <0 : failed
 *--------------------------------------------------*/
int lib_hdmi_read_em_vsem_packet(
    unsigned char           nport,
    unsigned char           pkt_buf[EM_VSEM_INFO_LEN]
    )
{
    if (pkt_buf==NULL || !lib_hdmi_is_em_vsem_received(nport))
        return -1;

    lib_hdmi_read_emp_sram(nport, EM_VSEM_INFO_OFST + EM_VSEM_INFO_LEN-4, 4, pkt_buf, NOT_PER_PORT);  // HB0~2 + BCH

    if (pkt_buf[3]!=0) {
        HDMI_WARN("VSEM BCH Error= %02x\n", pkt_buf[3]);
    }

    // Note: HDMI 2.0 MAC has it own SRAM to restore VSEM, but uses the same SRAM address as HD21 MAC's.
    // such that we use the same Offset address for both HD20 and HD21
    lib_hdmi_read_emp_sram(nport, EM_VSEM_INFO_OFST, EM_VSEM_INFO_LEN - 4, pkt_buf+3, NOT_PER_PORT);  // PB0~PB27

    lib_hdmi_clear_em_vsem_received_status(nport);

    HDMI_EMP_DBG("VSEM= %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \n",
                pkt_buf[0], pkt_buf[1],
                pkt_buf[2], pkt_buf[3],
                pkt_buf[4], pkt_buf[5],
                pkt_buf[6], pkt_buf[7],
                pkt_buf[8], pkt_buf[9],
                pkt_buf[10], pkt_buf[11],
                pkt_buf[12], pkt_buf[13]);
    
    return 0;
}


//-------------------------------------------------------------------------
// HDR
//-------------------------------------------------------------------------


/*---------------------------------------------------
 * Func : lib_hdm_hdr_emp_sram_sel
 *
 * Desc : share sram for HDMI2.0 or HDMI2.1
 *
 * Para : nport : HDMI port number
 *        hd21  : 0 : share SRAM for HDMI 2.0
 *                1 : share SRAM for HDMI 2.1
 *
 * Retn : 0 : success
 *--------------------------------------------------*/
int lib_hdm_hdr_emp_sram_sel(
    unsigned char           nport,
    unsigned char           hd21
    )
{
    hdmi_mask(HDMI_EM_CT_reg, HDMI_em_ct_hdr_hd20_hd21_sel_mask, (hd21) ? HDMI_em_ct_hdr_hd20_hd21_sel_mask : 0);
    return 0;
}

/*---------------------------------------------------
 * Func : lib_hdmi_set_em_hdr_config
 *
 * Desc : set HDR EM packet configuration
 *
 * Para : nport : HDMI port number
 *        p_emp_cfg : emp config
 *
 * Retn : 0 : successed
 *--------------------------------------------------*/
int lib_hdmi_set_em_hdr_config(
    unsigned char           nport,
    const EMP_PARAM*        p_emp_cfg
    )
{
    if (hdmi_rx[nport].hdmi_2p1_en)
    {
        // do nothing... not support HDMI 2.1
    }
    else
    {
        hdmi_mask(HDMI_EM_CT_reg,                                                     // switch to hdmi 2.0
            ~(HDMI_em_ct_hdr_hd20_hd21_sel_mask |
              HDMI_em_ct_org_id_mask |
              HDMI_em_ct_set_tag_msb_mask |
              HDMI_em_ct_set_tag_lsb_mask),
             (HDMI_em_ct_hdr_hd20_hd21_sel(0) |
              HDMI_em_ct_org_id(p_emp_cfg->org_id) |
              HDMI_em_ct_set_tag_msb(p_emp_cfg->data_set_tag[1]) |
              HDMI_em_ct_set_tag_msb(p_emp_cfg->data_set_tag[0])));

        hdmi_out(HDMI_HDR_EM_CT2_reg,
              HDMI_HDR_EM_CT2_sync(p_emp_cfg->sync)|
              HDMI_HDR_EM_CT2_recognize_oui_en(p_emp_cfg->recognize_oui) |
              HDMI_HDR_EM_CT2_oui_3rd(p_emp_cfg->oui[2]) |
              HDMI_HDR_EM_CT2_oui_2nd(p_emp_cfg->oui[1]) |
              HDMI_HDR_EM_CT2_oui_1st(p_emp_cfg->oui[0]));
    }

    return 0;
}

/*---------------------------------------------------
 * Func : _lib_hdmi_read_hdr_emp_sram
 *
 * Desc : read HDR EM packet SRAM
 *
 * Para : nport : HDMI port number
 *        start_addr : start address of packet sram
 *        len   : read length
 *        pbuf  : buffer
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
void _lib_hdmi_read_hdr_emp_sram(
    unsigned char           nport,
    unsigned int            start_addr,
    unsigned short          len,
    unsigned char*          pbuf
    )
{
    unsigned int i;
    unsigned int block_sel;
    unsigned int* p_val;

    if ((start_addr & 0x3) || (len & 0x3) || (((unsigned long)pbuf) & 0x3))
    {
        HDMI_WARN("read hdr emp sram failed, start_addr (%d), len (%d), pbuf(%p) should be 4 bytes align\n",
            start_addr, len, pbuf);
        return;
    }

    start_addr >>= 2;
    len >>= 2;
    p_val = (unsigned int*) pbuf;

    _lock_info_pack_sem();

#if 1  // Hummm.... should we select SRAM to access here???
    block_sel = HDMI_hdr_st_get_em_hdr_done_block(hdmi_in(HDMI_HDRST_reg));
    // set access block
    hdmi_mask(HDMI_EM_CT_reg, ~HDMI_em_ct_hdr_block_sel_mask, HDMI_em_ct_hdr_block_sel(block_sel));
#endif

    for (i=0; i<len; i++)
    {
        hdmi_out(HDMI_HDRAP_reg, start_addr+i);
        *p_val++ = hdmi_in(HDMI_HDRDP_reg);
    }

    _unlock_info_pack_sem();
}


/*---------------------------------------------------
 * Func : lib_hdmi_is_em_hdr_received
 *
 * Desc : check if HDR EM packet has been received
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
int lib_hdmi_is_em_hdr_received(unsigned char nport)
{
    unsigned int hdrst_reg = 0;

    if (hdmi_rx[nport].hdmi_2p1_en)
    {
        return 0; // not support HDMI 2.1
    }
    else
    {
        hdrst_reg = HDMI_HDRST_reg;

        if (HDMI_hdr_st_get_em_hdr_no(hdmi_in(hdrst_reg)))  // No EMP
        {
            hdmi_out(hdrst_reg, HDMI_hdr_st_em_hdr_no_mask);
            return 0;
        }

        if (!HDMI_hdr_st_get_em_hdr_first(hdmi_in(hdrst_reg)))  // No First Packet
            return 0;

        if (!HDMI_hdr_st_get_em_hdr_last(hdmi_in(hdrst_reg)))  // No Last Packet
            return 0;
    }

    return 1;
}


/*---------------------------------------------------
 * Func : lib_hdmi_clear_em_hdr_received_status
 *
 * Desc : clear HDR EM packet status
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : suc
 *--------------------------------------------------*/
int lib_hdmi_clear_em_hdr_received_status(unsigned char nport)
{
    hdmi_out(HDMI_HDRST_reg, hdmi_in(HDMI_HDRST_reg));
    return 0;
}


/*---------------------------------------------------
 * Func : lib_hdmi_read_em_hdr_packet
 *
 * Desc : read eM packet receive status
 *
 * Para : nport : HDMI port number
 *
 * Retn : > 0 : number of byte received, <0 : failed
 *--------------------------------------------------*/
int lib_hdmi_read_em_hdr_packet(
    unsigned char           nport,
    unsigned char           pkt_buf[MAX_EM_HDR_INFO_LEN]
    )
{
    int i;
    unsigned int pkt_cnt = 0;
    unsigned int len = MAX_EM_HDR_INFO_LEN;

    if (pkt_buf==NULL || !lib_hdmi_is_em_hdr_received(nport))
        return -1;

    _lib_hdmi_read_hdr_emp_sram(nport, EM_HDR_EMP_CNT_OFST, 4, (unsigned char*) &pkt_cnt);  // get EMP packet counts
    len = pkt_cnt * 32;

    HDMI_EMP_DBG("pkt_cnt=%d, len=%d\n", pkt_cnt, len);

    if (len > MAX_EM_HDR_INFO_LEN)
        len = MAX_EM_HDR_INFO_LEN;

    for (i=0; i<len; i+= 32)
    {
        unsigned char* p_emp = &pkt_buf[i];
        unsigned char tmp_emp[32];

        _lib_hdmi_read_hdr_emp_sram(nport, EM_HDR_EMP_INFO_OFST+i+28, 4, p_emp);  // 28~31 : HB0~2 + BCH

        // check emp BCH error
        if (p_emp[3]!=0)
        {
            HDMI_WARN("HDMI[p%d] HDR_EMP BCH Error detected (%p)= %02x %02x %02x %02x\n",
                     nport, p_emp, p_emp[0], p_emp[1], p_emp[2], p_emp[3]);
        }

        _lib_hdmi_read_hdr_emp_sram(nport, EM_HDR_EMP_INFO_OFST+i, 28, tmp_emp);  // 0~27  : PB0~PB27
        memcpy(&p_emp[3], tmp_emp, 28);

        HDMI_EMP_DBG("HDMI[p%d] HDR_EMP[%d] Data= %02x %02x %02x %02x\n", nport, p_emp - pkt_buf, p_emp[0], p_emp[1], p_emp[2], p_emp[3]);        
    }
    lib_hdmi_clear_em_hdr_received_status(nport);
    return pkt_cnt;
}

//-------------------------------------------------------------------------
// High level API
//-------------------------------------------------------------------------
void newbase_hdmi_reset_sbtm(unsigned char port)
{
    memset(&hdmi_rx[port].sbtm_emp, 0, sizeof(hdmi_rx[port].sbtm_emp));
    lib_hdmi_clear_em_sbtm_received_status(port);
    hdmi_rx[port].sbtm_emp_cnt = 0;
}

void newbase_hdmi_reset_rsv0(unsigned char port)
{
    memset(&hdmi_rx[port].rsv0_emp, 0, sizeof(hdmi_rx[port].rsv0_emp));
    lib_hdmi_clear_em_rsv0_received_status(port);
    hdmi_rx[port].rsv0_emp_cnt = 0;
}

void newbase_hdmi_reset_rsv1(unsigned char port)
{
    memset(&hdmi_rx[port].rsv1_emp, 0, sizeof(hdmi_rx[port].rsv1_emp));
    lib_hdmi_clear_em_rsv1_received_status(port);
    hdmi_rx[port].rsv1_emp_cnt = 0;
}

void newbase_hdmi_reset_vtem(unsigned char port)
{
    memset(&hdmi_rx[port].vtem_emp, 0, sizeof(hdmi_rx[port].vtem_emp));
    lib_hdmi_clear_em_vtem_received_status(port);
    hdmi_rx[port].vtem_emp_cnt = 0;
}

void newbase_hdmi_reset_vsem(unsigned char port)
{
    memset(&hdmi_rx[port].vsem_emp, 0, sizeof(hdmi_rx[port].vsem_emp));
    lib_hdmi_clear_em_vsem_received_status(port);
    hdmi_rx[port].vsem_emp_cnt = 0;
}

void newbase_hdmi_reset_hdr_emp(unsigned char port)
{
    memset(&hdmi_rx[port].hdr_emp, 0, sizeof(hdmi_rx[port].hdr_emp));
    lib_hdmi_clear_em_hdr_received_status(port);
    hdmi_rx[port].hdr_emp_cnt = 0;
}

void newbase_hdmi_reset_cvtem(unsigned char port)
{
    //HDMI_EMG("[newbase_hdmi_reset_cvtem port:%d, hdmi_rx[port].no_cvtem_emp_cnt=%d\n", port,hdmi_rx[port].no_cvtem_emp_cnt );
    memset(&hdmi_rx[port].cvtem_emp, 0, sizeof(hdmi_rx[port].cvtem_emp));
    lib_hdmi_clear_em_cvtem_received_status(port);
    hdmi_rx[port].cvtem_emp_cnt = 0;
}

void newbase_hdmi_reset_dolby_vsem(unsigned char port)
{
    memset(&hdmi_rx[port].dolby_vsem, 0, sizeof(hdmi_rx[port].dolby_vsem));
    lib_hdmi_clear_em_hdr_received_status(port); //used hdr emp sram for dolby vsem
    hdmi_rx[port].dolby_vsem_emp_cnt = 0;
}

/*---------------------------------------------------
 * Func : newbase_hdmi_reset_all_emp
 *
 * Desc : clear all EM packets
 *
 * Para : nport : HDMI port number
 *
 * Retn : > 0 : number of byte received, <0 : failed
 *--------------------------------------------------*/
void newbase_hdmi_reset_all_emp(unsigned char port)
{
    newbase_hdmi_reset_vtem(port);
    newbase_hdmi_reset_hdr_emp(port);
    newbase_hdmi_reset_vsem(port);
    newbase_hdmi_reset_cvtem(port);
    newbase_hdmi_reset_dolby_vsem(port);
    newbase_hdmi_reset_sbtm(port);
    newbase_hdmi_reset_rsv0(port);
    newbase_hdmi_reset_rsv1(port);
}

int _hdmi_check_emp_correctness(unsigned char port, unsigned char* p_emp_buff, int pkt_cnt)
{
    int i;

    if (p_emp_buff==NULL || pkt_cnt<=0)
    {
        EMP_CHK_WARN("HDMI[p%d] _hdmi_check_emp_correctness failed, invlid p_emp_buff=%p, pkt_cnt=%d\n", port,  p_emp_buff, pkt_cnt);
        return -1;
    }

    for (i=0; i<pkt_cnt ; i++)
    {
        EMP_PKT* p_emp = (EMP_PKT*) p_emp_buff;

        EMP_CHK_DBG("HDMI[p%d] EMP[%d/%d] HB : first=%d, last=%d, seq=%d { %02x %02x %02x}\n",
                port, i, pkt_cnt, p_emp->first, p_emp->last, p_emp->seq_idx,
                p_emp_buff[0], p_emp_buff[1], p_emp_buff[2]);

        if (i!= p_emp->seq_idx) {
            EMP_CHK_WARN("HDMI[p%d] _hdmi_check_emp_correctness failed, sequence mismatch=%d/%d\n", port, i, p_emp->seq_idx);
            return -1;  // sequence mismatch
        }

        if (i==0 && p_emp->first==0) {
            EMP_CHK_WARN("HDMI[p%d] _hdmi_check_emp_correctness failed, 1st packet but first is %d\n", port, p_emp->first);
            return -1;
        }

        if (i==(pkt_cnt-1) && p_emp->last==0) {
            EMP_CHK_WARN("HDMI[p%d] _hdmi_check_emp_correctness failed, pkt%d is the last packet but last is %d\n", port, i, p_emp->first);
            return -1;
        }

        if ((i!=0 && i!=(pkt_cnt-1)) && (p_emp->first || p_emp->last)) {
            EMP_CHK_WARN("HDMI[p%d] _hdmi_check_emp_correctness failed, pkt%d not first or last packet, but first is %d and last is %d\n",
                port, i, p_emp->first, p_emp->last);
            return -1;
        }

        p_emp_buff+= 32;
    }

    return 0;
}

/*---------------------------------------------------
 * Func : newbase_hdmi_update_emp
 *
 * Desc : update EM packets
 *
 * Para : nport : HDMI port number
 *
 * Retn : > 0 : number of byte received, <0 : failed
 *--------------------------------------------------*/
void newbase_hdmi_update_emp(unsigned char port)
{
    unsigned int pre_cvtem_emp_cnt = hdmi_rx[port].cvtem_emp_cnt;
    unsigned int pre_vtem_emp_cnt = hdmi_rx[port].vtem_emp_cnt;
    unsigned int pre_vsem_emp_cnt = hdmi_rx[port].vsem_emp_cnt;
    unsigned int pre_cta861_hdr_emp_cnt = hdmi_rx[port].hdr_emp_cnt;
    unsigned int pre_dolby_vsem_emp_cnt = hdmi_rx[port].dolby_vsem_emp_cnt;
    unsigned int pre_sbtm_emp_cnt = hdmi_rx[port].sbtm_emp_cnt;
    unsigned int pre_rsv0_emp_cnt = hdmi_rx[port].rsv0_emp_cnt;
    unsigned int pre_rsv1_emp_cnt = hdmi_rx[port].rsv1_emp_cnt;

    if(FALSE == newbase_hdmi_check_infoframe_config(port, INFO_TYPE_EMP))
    {
        return;
    }

    lib_hdmi_emp_enable_all_mode(port, 1);   // force use all mode to receive EMP (fapa end align next vs)

    //--------------------------------------------------
    // SBTM RX (for SBTM...)
    //--------------------------------------------------
    if (lib_hdmi_is_em_sbtm_received(port))
    {
        lib_hdmi_read_em_sbtm_packet(port, hdmi_rx[port].sbtm_emp);
        hdmi_rx[port].sbtm_emp_cnt++;
        hdmi_rx[port].no_sbtm_emp_cnt = 0;
    }
    else
    {
        if (++hdmi_rx[port].no_sbtm_emp_cnt > NO_SBTM_PKT_CNT_MAX)
        {
            newbase_hdmi_reset_sbtm(port);
            hdmi_rx[port].no_sbtm_emp_cnt = 0;
        }
    }

    //--------------------------------------------------
    // RSV0 RX (for RSV0...)
    //--------------------------------------------------
    if (lib_hdmi_is_em_rsv0_received(port))
    {
        lib_hdmi_read_em_rsv0_packet(port, hdmi_rx[port].rsv0_emp);
        hdmi_rx[port].rsv0_emp_cnt++;
        hdmi_rx[port].no_rsv0_emp_cnt = 0;
    }
    else
    {
        if (++hdmi_rx[port].no_rsv0_emp_cnt > NO_RSV0_PKT_CNT_MAX)
        {
            newbase_hdmi_reset_rsv0(port);
            hdmi_rx[port].no_rsv0_emp_cnt = 0;
        }
    }

    //--------------------------------------------------
    // RSV1 RX (for RSV1...)
    //--------------------------------------------------
    if (lib_hdmi_is_em_rsv1_received(port))
    {
        lib_hdmi_read_em_rsv1_packet(port, hdmi_rx[port].rsv1_emp);
        hdmi_rx[port].rsv1_emp_cnt++;
        hdmi_rx[port].no_rsv1_emp_cnt = 0;
    }
    else
    {
        if (++hdmi_rx[port].no_rsv1_emp_cnt > NO_RSV1_PKT_CNT_MAX)
        {
            newbase_hdmi_reset_rsv1(port);
            hdmi_rx[port].no_rsv1_emp_cnt = 0;
        }
    }

    // VTEM support per port received
    //--------------------------------------------------
    // VTEM RX (for VRR/FTA/...) per port received
    //--------------------------------------------------
    if (lib_hdmi_is_em_vtem_received(port))
    {
        lib_hdmi_read_em_vtem_packet(port, hdmi_rx[port].vtem_emp);
        hdmi_rx[port].vtem_emp_cnt++;
        hdmi_rx[port].no_vtem_emp_cnt = 0;
		#if 0//VRR packet
		HDMI_EMG("VTEM VRR,port=%d, mac_port_sel=%d, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x\n", 
			port, HDMI_PORT_SWITCH_get_port_sel(hdmi_in(HDMI_PORT_SWITCH_reg)), 
			hdmi_rx[port].vtem_emp[0], hdmi_rx[port].vtem_emp[1], hdmi_rx[port].vtem_emp[2], hdmi_rx[port].vtem_emp[3],
			hdmi_rx[port].vtem_emp[4], hdmi_rx[port].vtem_emp[5], hdmi_rx[port].vtem_emp[6], hdmi_rx[port].vtem_emp[7], hdmi_rx[port].vtem_emp[8],
			hdmi_rx[port].vtem_emp[9], hdmi_rx[port].vtem_emp[10], hdmi_rx[port].vtem_emp[11], hdmi_rx[port].vtem_emp[12]);
		#endif
    }
    else
    {
        if (++hdmi_rx[port].no_vtem_emp_cnt > NO_VTEM_PKT_CNT_MAX)
        {
            newbase_hdmi_reset_vtem(port);
            hdmi_rx[port].no_vtem_emp_cnt = 0;
        }
    }

    if ((g_infoframe_vtem_debug_print) || (((pre_vtem_emp_cnt==0) &&( hdmi_rx[port].vtem_emp_cnt>0))
        || ((pre_vtem_emp_cnt>0) &&( hdmi_rx[port].vtem_emp_cnt==0))))
    {
        HDMI_WARN("GET VTEM form %d -> %d, port=%d, data=%x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x. %x, %x, %x, %x\n",
            pre_vtem_emp_cnt, hdmi_rx[port].vtem_emp_cnt,
            port,
            hdmi_rx[port].vtem_emp[0], hdmi_rx[port].vtem_emp[1], hdmi_rx[port].vtem_emp[2], hdmi_rx[port].vtem_emp[3],
            hdmi_rx[port].vtem_emp[4], hdmi_rx[port].vtem_emp[5], hdmi_rx[port].vtem_emp[6], hdmi_rx[port].vtem_emp[7],
            hdmi_rx[port].vtem_emp[8], hdmi_rx[port].vtem_emp[9], hdmi_rx[port].vtem_emp[10], hdmi_rx[port].vtem_emp[11],
            hdmi_rx[port].vtem_emp[12], hdmi_rx[port].vtem_emp[13], hdmi_rx[port].vtem_emp[14], hdmi_rx[port].vtem_emp[15]);

    }


    // all ports share one HW port region, make sure only current port can read.
    if (port != newbase_hdmi_get_current_display_port()) {
        return;
    }

    lib_hdm_hdr_emp_sram_sel(port, hdmi_rx[port].hdmi_2p1_en); // select EMP SRAM (should we move it somewhere???)

    //--------------------------------------------------
    // CVTEM RX (for DSC)
    //--------------------------------------------------
    if (lib_hdmi_is_em_cvtem_received(port))
    {
        lib_hdmi_read_em_cvtem_packet(port, hdmi_rx[port].cvtem_emp);
        hdmi_rx[port].cvtem_emp_cnt++;
        hdmi_rx[port].no_cvtem_emp_cnt = 0;
    }
    else
    {
        if (++hdmi_rx[port].no_cvtem_emp_cnt > NO_CVTEM_PKT_CNT_MAX)
        {
            newbase_hdmi_reset_cvtem(port);
            hdmi_rx[port].no_cvtem_emp_cnt = 0;
        }
    }

    //--------------------------------------------------
    // VSEM RX
    //--------------------------------------------------
    lib_hdmi_set_em_vsem_type(port, &default_vsem_emp_cfg);    // set it to default vsem value

    if (lib_hdmi_is_em_vsem_received(port))
    {
#ifdef CONFIG_ENABLE_DOLBY_VISION_VSEM
        VSEMP_PKT* p_vsem = (VSEMP_PKT*) &hdmi_rx[port].vsem_emp;
#endif  // CONFIG_ENABLE_DOLBY_VISION_VSEM

        lib_hdmi_read_em_vsem_packet(port, hdmi_rx[port].vsem_emp);
        hdmi_rx[port].vsem_emp_cnt++;

#ifdef CONFIG_ENABLE_DOLBY_VISION_VSEM
        if (hdmi_rx[port].hdr_emp_mode != HDMI_HDR_EMP_RX_MODE_DOLBY_VISION_VSEM &&
            p_vsem->oui[0]==0x46 && p_vsem->oui[1]==0xD0 && p_vsem->oui[2]==0x00)
        {
            DOLBY_VSEM_INFO("HDMI[p%d] Dolby Vision VSEM detected (OUI=%02x%02x%02x). VSEM=HEAD(first=%d, last=%d, seq_idx=%d), swith HDR EMP to Dolby Vision VSEM mode\n",
                port, p_vsem->oui[2], p_vsem->oui[1], p_vsem->oui[0], p_vsem->first, p_vsem->last, p_vsem->seq_idx);
            hdmi_rx[port].hdr_emp_mode = HDMI_HDR_EMP_RX_MODE_DOLBY_VISION_VSEM;
            hdmi_rx[port].dolby_vsem_emp_cnt = 0;
            hdmi_rx[port].no_dolby_vsem_emp_cnt = 0;
        }
#endif //CONFIG_ENABLE_DOLBY_VISION_VSEM
    }

    //--------------------------------------------------
    // HDR EMP RX
    //--------------------------------------------------

    switch(hdmi_rx[port].hdr_emp_mode)
    {
    case HDMI_HDR_EMP_RX_MODE_DOLBY_VISION_VSEM:

        lib_hdmi_set_em_hdr_config(port, &dolby_vsem_emp_cfg);

        if (hdmi_rx[port].no_dolby_vsem_emp_cnt < 0xFF)
            hdmi_rx[port].no_dolby_vsem_emp_cnt++;  // default increase no dolby vsem cnt, it will be reset to zero if dolby vsem is received

        if (lib_hdmi_is_em_hdr_received(port))
        {
            VSEMP_PKT* p_vsem = (VSEMP_PKT*) &hdmi_rx[port].dolby_vsem;
            int pkt_cnt = lib_hdmi_read_em_hdr_packet(port, hdmi_rx[port].dolby_vsem);

            if (pkt_cnt>0 && p_vsem->oui[0]==0x46 && p_vsem->oui[1]==0xD0 && p_vsem->oui[2]==0x00)
            {
                hdmi_rx[port].dolby_vsem_emp_cnt++;
                hdmi_rx[port].no_dolby_vsem_emp_cnt = 0;

                if (_hdmi_check_emp_correctness(port, hdmi_rx[port].dolby_vsem, pkt_cnt)==0)
                {
                    p_vsem = (VSEMP_PKT*) hdmi_rx[port].dolby_vsem;

                    if ((hdmi_rx[port].dolby_vsem_emp_cnt & 0x3F)==1)
                    {
                        DOLBY_VSEM_INFO("HDMI[p%d] Get Dolby Vsem EMP (%d). pkt_cnt=%d, pb : new=%d, end=%d, ds_type=%d, afr=%d, vfr=%d, sync=%d, orig_id=%d, data_set_tag=%04x, data_set_len=%04x, oui=%06x\n",
                            port, hdmi_rx[port].dolby_vsem_emp_cnt, pkt_cnt, p_vsem->new, p_vsem->end, p_vsem->ds_type,
                            p_vsem->afr, p_vsem->vfr, p_vsem->sync, p_vsem->orig_id,
                            (p_vsem->data_set_tag[0]<<8) + p_vsem->data_set_tag[1],
                            (p_vsem->data_set_length[0]<<8) + p_vsem->data_set_length[1],
                            (p_vsem->oui[2]<<16) + (p_vsem->oui[1]<<8) + (p_vsem->oui[0]));
                    }

                    // Good Dolby VSEM Received
                }
                else
                {
                    DOLBY_VSEM_INFO("HDMI[p%d] Get Dolby Vsem failed, check emp sequence failed\n", port);
                }
            }
        }

        if (hdmi_rx[port].no_dolby_vsem_emp_cnt > 3)
        {
            DOLBY_VSEM_INFO("HDMI[p%d] wait dolby vsem timeout (%d), switch back to CTA-861G HDR emp mode\n", port, hdmi_rx[port].no_dolby_vsem_emp_cnt);
            hdmi_rx[port].hdr_emp_mode = HDMI_HDR_EMP_RX_MODE_CTA861_G_HDR_EMP;  // switch back to HDR EMP mode
            newbase_hdmi_reset_dolby_vsem(port);
        }
        break;

    case HDMI_HDR_EMP_RX_MODE_CTA861_G_HDR_EMP:
    default:

        lib_hdmi_set_em_hdr_config(port, &hdr_emp_cfg);

        if (lib_hdmi_is_em_hdr_received(port))
        {
            lib_hdmi_read_em_hdr_packet(port, hdmi_rx[port].hdr_emp);
            hdmi_rx[port].hdr_emp_cnt++;
            hdmi_rx[port].no_hdr_emp_cnt = 0;
        }
        else
        {
            if (++hdmi_rx[port].no_hdr_emp_cnt > NO_HDR_EMP_PKT_CNT_MAX)
            {
                newbase_hdmi_reset_hdr_emp(port);
                hdmi_rx[port].no_hdr_emp_cnt = 0;
            }
        }
    }

    // Print packet change
    if((g_infoframe_cvtem_debug_print) || (((pre_cvtem_emp_cnt==0) &&( hdmi_rx[port].cvtem_emp_cnt>0))
        || ((pre_cvtem_emp_cnt>0) &&( hdmi_rx[port].cvtem_emp_cnt==0))))
    {
        CVTEM_INFO info;

        HDMI_WARN("GET CVTEM from %d -> %d, port=%d, data=%x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x\n",
            pre_cvtem_emp_cnt, hdmi_rx[port].cvtem_emp_cnt,
            port,
            hdmi_rx[port].cvtem_emp[0], hdmi_rx[port].cvtem_emp[1], hdmi_rx[port].cvtem_emp[2], hdmi_rx[port].cvtem_emp[3],
            hdmi_rx[port].cvtem_emp[4], hdmi_rx[port].cvtem_emp[5], hdmi_rx[port].cvtem_emp[6], hdmi_rx[port].cvtem_emp[7],
            hdmi_rx[port].cvtem_emp[8], hdmi_rx[port].cvtem_emp[9], hdmi_rx[port].cvtem_emp[10], hdmi_rx[port].cvtem_emp[11]);

        _parse_cvtem_info(hdmi_rx[port].cvtem_emp, &info);

        _dump_cvtem_info(&info);
    }

    if ((g_infoframe_vsem_debug_print) || (((pre_vsem_emp_cnt==0) &&( hdmi_rx[port].vsem_emp_cnt>0))
        || ((pre_vsem_emp_cnt>0) &&( hdmi_rx[port].vsem_emp_cnt==0))))
    {
        HDMI_WARN("GET VSEM from %d -> %d, port=%d, data=%x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x\n",
            pre_vsem_emp_cnt, hdmi_rx[port].vsem_emp_cnt,
            port,
            hdmi_rx[port].vsem_emp[0], hdmi_rx[port].vsem_emp[1], hdmi_rx[port].vsem_emp[2], hdmi_rx[port].vsem_emp[3],
            hdmi_rx[port].vsem_emp[4], hdmi_rx[port].vsem_emp[5], hdmi_rx[port].vsem_emp[6], hdmi_rx[port].vsem_emp[7],
            hdmi_rx[port].vsem_emp[8], hdmi_rx[port].vsem_emp[9], hdmi_rx[port].vsem_emp[10], hdmi_rx[port].vsem_emp[11]);
    }

    if ((g_infoframe_hdremp_debug_print) || (((pre_cta861_hdr_emp_cnt==0) &&( hdmi_rx[port].hdr_emp_cnt>0))
        || ((pre_cta861_hdr_emp_cnt>0) &&( hdmi_rx[port].hdr_emp_cnt==0))))
    {
        HDMI_WARN("GET CTA861_HDR_EMP from %d -> %d, port=%d, data=%x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x\n",
            pre_cta861_hdr_emp_cnt, hdmi_rx[port].hdr_emp_cnt,
            port,
            hdmi_rx[port].hdr_emp[0], hdmi_rx[port].hdr_emp[1], hdmi_rx[port].hdr_emp[2], hdmi_rx[port].hdr_emp[3],
            hdmi_rx[port].hdr_emp[4], hdmi_rx[port].hdr_emp[5], hdmi_rx[port].hdr_emp[6], hdmi_rx[port].hdr_emp[7],
            hdmi_rx[port].hdr_emp[8], hdmi_rx[port].hdr_emp[9], hdmi_rx[port].hdr_emp[10], hdmi_rx[port].hdr_emp[11]);
    }

    if ((g_infoframe_vsem_debug_print) || (((pre_dolby_vsem_emp_cnt==0) &&( hdmi_rx[port].dolby_vsem_emp_cnt>0))
        || ((pre_dolby_vsem_emp_cnt>0) &&( hdmi_rx[port].dolby_vsem_emp_cnt==0))))
    {
        HDMI_WARN("GET DOLBY_VISION_VSEM from %d -> %d, port=%d, data=%x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x\n",
            pre_dolby_vsem_emp_cnt, hdmi_rx[port].dolby_vsem_emp_cnt,
            port,
            hdmi_rx[port].dolby_vsem[0], hdmi_rx[port].dolby_vsem[1], hdmi_rx[port].dolby_vsem[2], hdmi_rx[port].dolby_vsem[3],
            hdmi_rx[port].dolby_vsem[4], hdmi_rx[port].dolby_vsem[5], hdmi_rx[port].dolby_vsem[6], hdmi_rx[port].dolby_vsem[7],
            hdmi_rx[port].dolby_vsem[8], hdmi_rx[port].dolby_vsem[9], hdmi_rx[port].dolby_vsem[10], hdmi_rx[port].dolby_vsem[11]);
    }

    if ((g_infoframe_sbtm_debug_print) || (((pre_sbtm_emp_cnt==0) &&( hdmi_rx[port].sbtm_emp_cnt>0))
        || ((pre_sbtm_emp_cnt>0) &&( hdmi_rx[port].sbtm_emp_cnt==0))))
    {
        HDMI_WARN("GET SBTM from %d -> %d, port=%d, data=%x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x\n",
            pre_sbtm_emp_cnt, hdmi_rx[port].sbtm_emp_cnt,
            port,
            hdmi_rx[port].sbtm_emp[0], hdmi_rx[port].sbtm_emp[1], hdmi_rx[port].sbtm_emp[2], hdmi_rx[port].sbtm_emp[3],
            hdmi_rx[port].sbtm_emp[4], hdmi_rx[port].sbtm_emp[5], hdmi_rx[port].sbtm_emp[6], hdmi_rx[port].sbtm_emp[7],
            hdmi_rx[port].sbtm_emp[8], hdmi_rx[port].sbtm_emp[9], hdmi_rx[port].sbtm_emp[10], hdmi_rx[port].sbtm_emp[11]);
    }

    if ((g_infoframe_rsv0_debug_print) || (((pre_rsv0_emp_cnt==0) &&( hdmi_rx[port].rsv0_emp_cnt>0))
        || ((pre_rsv0_emp_cnt>0) &&( hdmi_rx[port].rsv0_emp_cnt==0))))
    {
        HDMI_WARN("GET RSV0 from %d -> %d, port=%d, data=%x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x\n",
            pre_rsv0_emp_cnt, hdmi_rx[port].rsv0_emp_cnt,
            port,
            hdmi_rx[port].rsv0_emp[0], hdmi_rx[port].rsv0_emp[1], hdmi_rx[port].rsv0_emp[2], hdmi_rx[port].rsv0_emp[3],
            hdmi_rx[port].rsv0_emp[4], hdmi_rx[port].rsv0_emp[5], hdmi_rx[port].rsv0_emp[6], hdmi_rx[port].rsv0_emp[7],
            hdmi_rx[port].rsv0_emp[8], hdmi_rx[port].rsv0_emp[9], hdmi_rx[port].rsv0_emp[10], hdmi_rx[port].rsv0_emp[11]);
    }

    if ((g_infoframe_rsv1_debug_print) || (((pre_rsv1_emp_cnt==0) &&( hdmi_rx[port].rsv1_emp_cnt>0))
        || ((pre_rsv1_emp_cnt>0) &&( hdmi_rx[port].rsv1_emp_cnt==0))))
    {
        HDMI_WARN("GET RSV1 from %d -> %d, port=%d, data=%x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x\n",
            pre_rsv1_emp_cnt, hdmi_rx[port].rsv1_emp_cnt,
            port,
            hdmi_rx[port].rsv1_emp[0], hdmi_rx[port].rsv1_emp[1], hdmi_rx[port].rsv1_emp[2], hdmi_rx[port].rsv1_emp[3],
            hdmi_rx[port].rsv1_emp[4], hdmi_rx[port].rsv1_emp[5], hdmi_rx[port].rsv1_emp[6], hdmi_rx[port].rsv1_emp[7],
            hdmi_rx[port].rsv1_emp[8], hdmi_rx[port].rsv1_emp[9], hdmi_rx[port].rsv1_emp[10], hdmi_rx[port].rsv1_emp[11]);
    }

    return;
}


/*---------------------------------------------------
 * Func : newbase_hdmi_get_sbtm_emp
 *
 * Desc : get SBTM EM packets
 *
 * Para : nport : HDMI port number
 *
 * Retn : > 0 : number of byte received, <0 : failed
 *--------------------------------------------------*/
int newbase_hdmi_get_sbtm_emp(
    unsigned char           port,
    unsigned char           pkt_buf[EM_SBTM_INFO_LEN]
    )
{
    if (GET_H_VIDEO_FSM(port) < MAIN_FSM_HDMI_MEASURE ||
        GET_H_MODE(port) != MODE_HDMI ||
        hdmi_rx[port].sbtm_emp[0] != 0x7F)
        return 0;

    memcpy(pkt_buf, &hdmi_rx[port].sbtm_emp, EM_SBTM_INFO_LEN);
    return EM_SBTM_INFO_LEN;
}


/*---------------------------------------------------
 * Func : newbase_hdmi_get_sbtm_info
 *
 * Desc : parse sbtm packet
 *
 * Para : nport : HDMI port number
 *
 * Retn : TRUE: success, FALSE : failed
 *--------------------------------------------------*/
unsigned char newbase_hdmi_get_sbtm_info(unsigned char port, HDMI_EM_SBTM_T *p_emp_sbtm_t)
{
    unsigned char pkt_buf[EM_SBTM_INFO_LEN] = {0};
    if(newbase_hdmi_get_sbtm_emp(port, pkt_buf)>0)
    {
        p_emp_sbtm_t->SBTM_ver = (hdmi_rx[port].sbtm_emp[10] & (_BIT3 | _BIT2 | _BIT1 | _BIT0));
        p_emp_sbtm_t->SBTM_mode = (hdmi_rx[port].sbtm_emp[11] & (_BIT1 | _BIT0));
        p_emp_sbtm_t->SBTM_type = (hdmi_rx[port].sbtm_emp[11] & (_BIT3 | _BIT2))>>2;
        p_emp_sbtm_t->GRDM_min = (hdmi_rx[port].sbtm_emp[11] & (_BIT5 | _BIT4))>>4;
        p_emp_sbtm_t->GRDM_lum = (hdmi_rx[port].sbtm_emp[11] & (_BIT7 | _BIT6))>>6;
        p_emp_sbtm_t->FrmPBLimitInt = ((hdmi_rx[port].sbtm_emp[12]& (_BIT5 | _BIT4 | _BIT3 | _BIT2 | _BIT1 | _BIT0))<<8) | hdmi_rx[port].sbtm_emp[13];
        return TRUE;
    }
    else
    {
        HDMI_EMP_DBG("newbase_hdmi_get_sbtm_info get sbtm fail!! port = %d \n", port);
        return FALSE;
    }

    return FALSE;
}


/*---------------------------------------------------
 * Func : newbase_hdmi_get_rsv0_emp
 *
 * Desc : get RSV0 EM packets
 *
 * Para : nport : HDMI port number
 *
 * Retn : > 0 : number of byte received, <0 : failed
 *--------------------------------------------------*/
int newbase_hdmi_get_rsv0_emp(
    unsigned char           port,
    unsigned char           pkt_buf[RSV0_EM_LEN]
    )
{
    if (GET_H_VIDEO_FSM(port) < MAIN_FSM_HDMI_MEASURE ||
        GET_H_MODE(port) != MODE_HDMI ||
        hdmi_rx[port].rsv0_emp[0] != 0x7F)
        return 0;

    memcpy(pkt_buf, &hdmi_rx[port].rsv0_emp, RSV0_EM_LEN);
    return RSV0_EM_LEN;
}


/*---------------------------------------------------
 * Func : newbase_hdmi_get_rsv0_info
 *
 * Desc : parse rsv0 packet
 *
 * Para : nport : HDMI port number
 *
 * Retn : TRUE: success, FALSE : failed
 *--------------------------------------------------*/
unsigned char newbase_hdmi_get_rsv0_info(unsigned char port, HDMI_EM_RSV0_T *p_emp_rsv0_t)
{
    unsigned char pkt_buf[RSV0_EM_LEN] = {0};
    if(newbase_hdmi_get_rsv0_emp(port, pkt_buf)>0)
    {
        p_emp_rsv0_t->RSV0_MD0 = hdmi_rx[port].rsv0_emp[10];
        p_emp_rsv0_t->RSV0_MD1 = hdmi_rx[port].rsv0_emp[11];
        p_emp_rsv0_t->RSV0_MD2 = hdmi_rx[port].rsv0_emp[12];
        p_emp_rsv0_t->RSV0_MD3 = hdmi_rx[port].rsv0_emp[13];
        return TRUE;
    }
    else
    {
        HDMI_EMP_DBG("newbase_hdmi_get_rsv0_info get rsv0 fail!! port = %d \n", port);
        return FALSE;
    }

    return FALSE;
}


/*---------------------------------------------------
 * Func : newbase_hdmi_get_rsv1_emp
 *
 * Desc : get RSV1 EM packets
 *
 * Para : nport : HDMI port number
 *
 * Retn : > 0 : number of byte received, <0 : failed
 *--------------------------------------------------*/
int newbase_hdmi_get_rsv1_emp(
    unsigned char           port,
    unsigned char           pkt_buf[RSV1_EM_LEN]
    )
{
    if (GET_H_VIDEO_FSM(port) < MAIN_FSM_HDMI_MEASURE ||
        GET_H_MODE(port) != MODE_HDMI ||
        hdmi_rx[port].rsv1_emp[0] != 0x7F)
        return 0;

    memcpy(pkt_buf, &hdmi_rx[port].rsv1_emp, RSV1_EM_LEN);
    return RSV1_EM_LEN;
}


/*---------------------------------------------------
 * Func : newbase_hdmi_get_rsv1_info
 *
 * Desc : parse rsv1 packet
 *
 * Para : nport : HDMI port number
 *
 * Retn : TRUE: success, FALSE : failed
 *--------------------------------------------------*/
unsigned char newbase_hdmi_get_rsv1_info(unsigned char port, HDMI_EM_RSV1_T *p_emp_rsv1_t)
{
    unsigned char pkt_buf[RSV1_EM_LEN] = {0};
    if(newbase_hdmi_get_rsv1_emp(port, pkt_buf)>0)
    {
        p_emp_rsv1_t->RSV1_MD0 = hdmi_rx[port].rsv1_emp[10];
        p_emp_rsv1_t->RSV1_MD1 = hdmi_rx[port].rsv1_emp[11];
        p_emp_rsv1_t->RSV1_MD2 = hdmi_rx[port].rsv1_emp[12];
        p_emp_rsv1_t->RSV1_MD3 = hdmi_rx[port].rsv1_emp[13];
        return TRUE;
    }
    else
    {
        HDMI_EMP_DBG("newbase_hdmi_get_rsv1_info get rsv1 fail!! port = %d \n", port);
        return FALSE;
    }

    return FALSE;
}


/*---------------------------------------------------
 * Func : newbase_hdmi_get_vtem_emp
 *
 * Desc : get vrr EM packets
 *
 * Para : nport : HDMI port number
 *
 * Retn : > 0 : number of byte received, <0 : failed
 *--------------------------------------------------*/
int newbase_hdmi_get_vtem_emp(
    unsigned char           port,
    unsigned char           pkt_buf[EM_VTEM_INFO_LEN]
    )
{
    if (GET_H_MODE(port) != MODE_HDMI)
        return 0;

    memcpy(pkt_buf, &hdmi_rx[port].vtem_emp, EM_VTEM_INFO_LEN);
    if(g_hdmi_vrr_debug_disable == HDMI_DEBUG_PACKET_FORCE_DISABLE){ // disable VRR
        pkt_buf[10] = pkt_buf[10]&0xFE;
    }else if(g_hdmi_vrr_debug_disable == HDMI_DEBUG_PACKET_FORCE_ENABLE){
        pkt_buf[10] = pkt_buf[10]|_BIT0;
    }
    return EM_VTEM_INFO_LEN;
}

/*---------------------------------------------------
 * Func : newbase_hdmi_get_vtem_info
 *
 * Desc : parse vtem packet
 *
 * Para : nport : HDMI port number
 *
 * Retn : TRUE: success, FALSE : failed
 *--------------------------------------------------*/
unsigned char newbase_hdmi_get_vtem_info(unsigned char port, HDMI_EM_VTEM_T *p_emp_vtem_t)
{
    unsigned char pkt_buf[EM_VTEM_INFO_LEN];
    memset(&pkt_buf, 0, EM_VTEM_INFO_LEN); //init
    if(newbase_hdmi_get_vtem_emp(port, pkt_buf)>0)
    {
        p_emp_vtem_t->FVA_Factor_M1 = (pkt_buf[10] & (_BIT7 | _BIT6 | _BIT5 | _BIT4))>>4;
        p_emp_vtem_t->QMS_EN= (pkt_buf[10] & (_BIT2))>>2;
        p_emp_vtem_t->M_CONST = (pkt_buf[10] & (_BIT1))>>1;
        p_emp_vtem_t->VRR_EN = (pkt_buf[10] & (_BIT0));
        p_emp_vtem_t->Base_Vfront = pkt_buf[11];
        p_emp_vtem_t->Next_TFR = (pkt_buf[12] & (_BIT7 | _BIT6 | _BIT5 | _BIT4 | _BIT3))>>3;
        p_emp_vtem_t->RB = (pkt_buf[12]& (_BIT2))>>2;
        p_emp_vtem_t->Base_Refresh_Rate = ((pkt_buf[12]& (_BIT1 | _BIT0))<<8) | pkt_buf[13];

        return TRUE;
    }
    else
    {
        HDMI_EMP_DBG("newbase_hdmi_get_vtem_info get vtem fail!! port = %d \n", port);
        return FALSE;
    }

    return FALSE;
}
 


/*---------------------------------------------------
 * Func : newbase_hdmi_get_cvtem_emp
 *
 * Desc : get vrr EM packets
 *
 * Para : nport : HDMI port number
 *
 * Retn : > 0 : number of byte received, <0 : failed
 *--------------------------------------------------*/
int newbase_hdmi_get_cvtem_emp(
    unsigned char           port,
    unsigned char           pkt_buf[EM_CVTEM_INFO_LEN]
    )
{
    if (GET_H_VIDEO_FSM(port) < MAIN_FSM_HDMI_MEASURE ||
        GET_H_MODE(port) != MODE_HDMI)
        return 0;

    memcpy(pkt_buf, &hdmi_rx[port].cvtem_emp, EM_CVTEM_INFO_LEN);
    return EM_CVTEM_INFO_LEN;
}


/*---------------------------------------------------
 * Func : newbase_hdmi_get_vsem_emp
 *
 * Desc : get VS EM packets
 *
 * Para : nport : HDMI port number
 *
 * Retn : > 0 : number of byte received, <0 : failed
 *--------------------------------------------------*/
int newbase_hdmi_get_vsem_emp(
    unsigned char           port,
    unsigned char           pkt_buf[EM_VSEM_INFO_LEN]
    )
{
    if (GET_H_VIDEO_FSM(port) < MAIN_FSM_HDMI_MEASURE ||
        GET_H_MODE(port) != MODE_HDMI)
        return 0;

    memcpy(pkt_buf, &hdmi_rx[port].vsem_emp, EM_VSEM_INFO_LEN);
    return EM_VSEM_INFO_LEN;
}


/*---------------------------------------------------
 * Func : newbase_hdmi_get_hdr_emp
 *
 * Desc : get HDR EM packets
 *
 * Para : nport : HDMI port number
 *
 * Retn : > 0 : number of byte received, <0 : failed
 *--------------------------------------------------*/
int newbase_hdmi_get_hdr_emp(
    unsigned char           port,
    unsigned char           pkt_buf[MAX_EM_HDR_INFO_LEN]
    )
{
    if (GET_H_VIDEO_FSM(port) < MAIN_FSM_HDMI_MEASURE ||
        GET_H_MODE(port) != MODE_HDMI)
        return 0;

    memcpy(pkt_buf, &hdmi_rx[port].hdr_emp, MAX_EM_HDR_INFO_LEN);
    return MAX_EM_HDR_INFO_LEN;
}
