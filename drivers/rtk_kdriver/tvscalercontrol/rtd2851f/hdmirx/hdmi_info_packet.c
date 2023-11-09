#include "hdmi_common.h"
#include "hdmi_reg.h"
#include "hdmi_vfe_config.h"
#include "hdmi_info_packet.h"
#include "cea_861.h"
#include "hdmi_hdr.h" // for newbase_hdmi_dolby_vision_enable()

extern HDMI_PORT_INFO_T hdmi_rx[HDMI_PORT_TOTAL_NUM];
extern spinlock_t hdmi_spin_lock;

struct semaphore sem_info_pack;

void  _init_info_pack_sem(void)      { sema_init(&sem_info_pack, 1); }
void  _lock_info_pack_sem(void)      { down(&sem_info_pack); }
void  _unlock_info_pack_sem(void)    { up(&sem_info_pack); }

#ifdef CONFIG_SUPPORT_DOLBY_VSIF
struct semaphore sem_dolby_vsif;
extern DOLBY_HDMI_VSIF_T pre_dolby_hdmi_vsif_mode;

void  _init_dolby_vsif_sem(void)      { sema_init(&sem_dolby_vsif, 1); }
void  _lock_dolby_vsif_sem(void)      { down(&sem_dolby_vsif); }
void  _unlock_dolby_vsif_sem(void)    { up(&sem_dolby_vsif); }
#endif

//-------------------------------------------------------------------------
// Common Part
//-------------------------------------------------------------------------
static INFO_PACKET_CONFIG info_packet_config_array[INFO_TYPE_NUM] =
{
    {INFO_TYPE_AVI, TRUE, TYPE_CODE_AVI_PACKET, MAIN_FSM_HDMI_MEASURE, NO_AVI_INFO_MAX},
    {INFO_TYPE_SPD, TRUE, TYPE_CODE_SPD_PACKET, MAIN_FSM_HDMI_WAIT_SYNC, NO_SPD_INFO_MAX},
    {INFO_TYPE_DVS, TRUE, TYPE_CODE_VS_PACKET, MAIN_FSM_HDMI_MEASURE, NO_DVS_INFO_MAX},
    {INFO_TYPE_VSI, TRUE, TYPE_CODE_VS_PACKET, MAIN_FSM_HDMI_MEASURE, NO_VSI_INFO_MAX},
    {INFO_TYPE_HDR10PVSI, TRUE, TYPE_CODE_VS_PACKET, MAIN_FSM_HDMI_MEASURE, NO_HDR10PVS_INFO_MAX},
    {INFO_TYPE_DRM, TRUE, TYPE_CODE_DRM_PACKET, MAIN_FSM_HDMI_MEASURE, NO_DRM_INFO_MAX},
    {INFO_TYPE_AUDIO, TRUE, TYPE_CODE_AUDIO_PACKET, MAIN_FSM_HDMI_MEASURE, NO_AUDIO_INFO_MAX},
    {INFO_TYPE_EMP, TRUE, TYPE_CODE_EMP_PACKET, MAIN_FSM_HDMI_WAIT_SYNC, NO_VTEM_PKT_CNT_MAX},
    {INFO_TYPE_RSV0, TRUE, TYPE_CODE_AUDIO_CONTENT_POTECTION_PACKET, MAIN_FSM_HDMI_MEASURE, NO_RSV0_INFO_MAX},    //For Audio Content Protection Packet (ACP)
    {INFO_TYPE_RSV1, FALSE, TYPE_CODE_HBR_SAMPLE_PACKET, MAIN_FSM_HDMI_MEASURE, NO_RSV2_INFO_MAX}, //Reserved
    {INFO_TYPE_RSV2, FALSE, TYPE_CODE_VS_PACKET, MAIN_FSM_HDMI_MEASURE, NO_RSV2_INFO_MAX},  //Reserved
    {INFO_TYPE_RSV3, FALSE, TYPE_CODE_GENERAL_CONTROL_PACKET,MAIN_FSM_HDMI_MEASURE, NO_RSV2_INFO_MAX},   //For sysfs rsv command debugging
    {INFO_TYPE_UNDEFINED, FALSE, TYPE_CODE_GENERAL_CONTROL_PACKET,MAIN_FSM_HDMI_MEASURE}   //For UT default case
};

static RESERVED_PACKET_DATA reserved_data_temp[RESERVED_PACKET_NUM][HDMI_PORT_TOTAL_NUM];
static unsigned int hdmi_amd_freesync_debug_print[HDMI_PORT_TOTAL_NUM];    // for debug print

static HDMI_QMS_STATUS_T m_qms_status[4];
/*---------------------------------------------------
 * Func : lib_hdmi_read_packet_sram
 *
 * Desc : read packet SRAM
 *
 * Para : nport : HDMI port number
 *        start_addr : start address of packet sram
 *        len   : read length
 *        pbuf  : buffer
 *
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_read_packet_sram(
    unsigned char           nport,
    unsigned int            start_addr,
    unsigned char           len,
    unsigned char*          pbuf
    )
{
    unsigned char i;
    unsigned long flags = 0;

    spin_lock_irqsave(&hdmi_spin_lock, flags);  // add spinlock to prevent racing

    for (i=0; i<len; i++)
    {
        hdmi_out(HDMI_HDMI_PSAP_reg, start_addr+i);
        pbuf[i] = hdmi_in(HDMI_HDMI_PSDP_reg);
    }

    spin_unlock_irqrestore(&hdmi_spin_lock, flags);
}

#ifndef UT_flag
/*---------------------------------------------------
 * Func : newbase_hdmi_infoframe_checksum_verification
 *
 * Desc : check vaildity of the infoframe packet by embeded checksum
 *
 * Para : type_code : Infoframe Packet Type defined in CTA-861
 *        len : inforframe packet length without type_code
 *        pkt_buf : packet content
 *
 * Retn : 0: checksum fail, 1: checksum pass
 *--------------------------------------------------*/
unsigned char newbase_hdmi_infoframe_checksum_verification(unsigned char type_code, unsigned int len, unsigned char *pkt_buf)
{
    unsigned int pkt_sum = type_code;
    unsigned int idx = 0;

    INFOFRAME_FLOW_PRINTF("%s type=0x%x, len=%d\n", __func__, type_code, len);

    if (len > HDMI_INFOF_MAX_LEN) {
        HDMI_WARN("infoframe type (%d) invalid length (%d), trim to max length %d", type_code, len, HDMI_INFOF_MAX_LEN);
        len = HDMI_INFOF_MAX_LEN;
    }

    for(idx = 0; idx < len; idx++) {
        pkt_sum += pkt_buf[idx];
    }

    if (pkt_sum & 0xff || (debugging_flow_print_flag & HDMI_FLOW_PRPINT_INFOFRAME))
    {
        // verify fail, dump raw data.
        int idx = 0, n = 0, size = len * 3;
        int end = size - 1;
        char data[HDMI_INFOF_MAX_LEN*3], *ptr; // dump format: ff ff ff ..., need 3bytes for each value

        memset(data, 0, size);
        ptr = data;

        for (idx = 0; idx < len; idx++) {
            n = scnprintf(ptr, size, "%02x ", pkt_buf[idx]);
            ptr += n;
            size -= n;
        }
        data[end] = '\0';

        HDMI_WARN("infoframe checksum %s, type(%02x) data(%s)\n", ((pkt_sum & 0xff) ? "FAIL" : "PASS"), type_code, data);
    }

    return !(pkt_sum & 0xff);
}

/*---------------------------------------------------
 * Func : lib_hdmi_infoframe_packet_read
 *
 * Desc : read packet SRAM and check the checksum result
 *
 * Para : 
 *        type_code : which infoframe is read
 *        nport : HDMI port number
 *        start_addr : start address of packet sram
 *        len   : read length
 *        pbuf  : buffer
 *
 * Retn : 0 : checksum failed, 1 : success
 *--------------------------------------------------*/
int lib_hdmi_infoframe_packet_read(
    unsigned char           type_code,
    unsigned char           nport,
    unsigned int            start_addr,
    unsigned char           len,
    unsigned char*          pbuf
    )
{
    lib_hdmi_read_packet_sram(nport, start_addr, len, pbuf);
    return newbase_hdmi_infoframe_checksum_verification(type_code, len, pbuf);
}

//-------------------------------------------------------------------------
// FVS
//-------------------------------------------------------------------------


/*---------------------------------------------------
 * Func : lib_hdmi_is_fvs_received
 *
 * Desc : check if FVS packet received
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
int lib_hdmi_is_fvs_received(unsigned char nport)
{
    return HDMI_HDMI_GPVS_get_fvsps(hdmi_in(HDMI_HDMI_GPVS_reg));
}


/*---------------------------------------------------
 * Func : lib_hdmi_clear_fvs_received_status
 *
 * Desc : clear FVS packet receive status
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
int lib_hdmi_clear_fvs_received_status(unsigned char nport)
{
    hdmi_out(HDMI_HDMI_GPVS_reg, HDMI_HDMI_GPVS_fvsps_mask);
    return 0;
}


/*---------------------------------------------------
 * Func : lib_hdmi_read_fvs_info_packet
 *
 * Desc : read HDMI formum VSIF info packet
 *
 * Para : nport : HDMI port number
 *        pkt_buf : avi output buffer
 *
 * Retn : N/A
 *--------------------------------------------------*/
INFOFRAME_READ_RESULT lib_hdmi_read_fvs_info_packet(
    unsigned char               port,
    unsigned char               *pkt_buf
    )
{
    int checksum_pass;
    if (!lib_hdmi_is_fvs_received(port) || pkt_buf==NULL)
        return FVS_READ_NOT_READY;
        
    lib_hdmi_clear_fvs_received_status(port);
    checksum_pass = lib_hdmi_infoframe_packet_read(TYPE_CODE_VS_PACKET, port, FVS_INFO_OFST + 1, FVS_INFO_LEN - 1, pkt_buf+1);
    pkt_buf[0] = TYPE_CODE_VS_PACKET;
    if (checksum_pass)
        return INFOFRAME_READ_SUCCESS;
    else return FVS_READ_CHECKSUM_ERROR;
}

/*---------------------------------------------------
 * Func : lib_hdmi_read_fvs_allm_mode
 *
 * Desc : read ALLM from HDMI forum VSIF info packet
 *
 * Para : nport : HDMI port number
 *        p_allm : allm output buffer
 *
 * Retn : N/A
 *--------------------------------------------------*/
unsigned char lib_hdmi_read_fvs_allm_mode(unsigned char nport)
{
    return HDMI_HDMI_FVS_get_allm_mode(hdmi_in(HDMI_HDMI_FVS_reg));
}

//-------------------------------------------------------------------------
// DVS - Dolby Vendor Specific Info frame
//-------------------------------------------------------------------------


/*---------------------------------------------------
 * Func : lib_hdmi_is_dvs_received
 *
 * Desc : check if Dolby Vendor Specific packet received
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
int lib_hdmi_is_dvs_received(unsigned char nport)
{
    return HDMI_HDMI_GPVS_get_dvsps(hdmi_in(HDMI_HDMI_GPVS_reg));
}


/*---------------------------------------------------
 * Func : lib_hdmi_clear_dvs_received_status
 *
 * Desc : clear Dolby Vendor Specific packet receive status
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
int lib_hdmi_clear_dvs_received_status(unsigned char nport)
{
    hdmi_out(HDMI_HDMI_GPVS_reg, HDMI_HDMI_GPVS_dvsps_mask);
    return 0;
}


/*---------------------------------------------------
 * Func : lib_hdmi_read_dvs_info_packet
 *
 * Desc : read Dolby Vendor Specific info packet
 *
 * Para : nport : HDMI port number
 *        pkt_buf : avi output buffer
 *
 * Retn : N/A
 *--------------------------------------------------*/
INFOFRAME_READ_RESULT lib_hdmi_read_dvs_info_packet(
    unsigned char               port,
    unsigned char               *pkt_buf
    )
{
    int checksum_pass;
    if (!lib_hdmi_is_dvs_received(port) || pkt_buf==NULL)
        return DVS_READ_NOT_READY;
        
    lib_hdmi_clear_dvs_received_status(port);
    checksum_pass = lib_hdmi_infoframe_packet_read(TYPE_CODE_VS_PACKET, port, DVS_INFO_OFST + 1, DVS_INFO_LEN - 1, pkt_buf+1);
    
    pkt_buf[0] = TYPE_CODE_VS_PACKET;
    if (checksum_pass) return INFOFRAME_READ_SUCCESS;
    else return DVS_READ_CHECKSUM_ERROR;
}



//-------------------------------------------------------------------------
// HDR10+ VS - HDR10+ Vendor Specific Info frame
//-------------------------------------------------------------------------


/*---------------------------------------------------
 * Func : lib_hdmi_is_hdr10pvs_received
 *
 * Desc : check if HDR10+ Vendor Specific packet received
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
int lib_hdmi_is_hdr10pvs_received(unsigned char nport)
{
    return HDMI_HDMI_GPVS_get_hdr10pvsps(hdmi_in(HDMI_HDMI_GPVS_reg));
}


/*---------------------------------------------------
 * Func : lib_hdmi_clear_hdr10pvs_received_status
 *
 * Desc : clear HDR10+ Vendor Specific packet receive status
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
int lib_hdmi_clear_hdr10pvs_received_status(unsigned char nport)
{
    hdmi_out(HDMI_HDMI_GPVS_reg, HDMI_HDMI_GPVS_hdr10pvsps_mask);
    return 0;
}


/*---------------------------------------------------
 * Func : lib_hdmi_read_hdr10pvs_info_packet
 *
 * Desc : read HDR10+ Vendor Specific info packet
 *
 * Para : nport : HDMI port number
 *        pkt_buf : avi output buffer
 *
 * Retn : N/A
 *--------------------------------------------------*/
INFOFRAME_READ_RESULT lib_hdmi_read_hdr10pvs_info_packet(
    unsigned char               port,
    unsigned char               *pkt_buf
    )
{
    int checksum_pass;
    if (!lib_hdmi_is_hdr10pvs_received(port) || pkt_buf==NULL)
        return HDR10PVS_READ_NOT_READY;

    lib_hdmi_clear_hdr10pvs_received_status(port);
    checksum_pass = lib_hdmi_infoframe_packet_read(TYPE_CODE_VS_PACKET, port, HDR10PVS_INFO_OFST + 1, HDR10PVS_INFO_LEN - 1, pkt_buf+1);
    
    pkt_buf[0] = TYPE_CODE_VS_PACKET;
    if (checksum_pass) return INFOFRAME_READ_SUCCESS;
    else return HDR10PVS_READ_CHECKSUM_ERROR;
}


//-------------------------------------------------------------------------
// VSI
//-------------------------------------------------------------------------


/*---------------------------------------------------
 * Func : lib_hdmi_is_vsi_received
 *
 * Desc : check if VSI packet received
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
int lib_hdmi_is_vsi_received(unsigned char nport)
{
    return HDMI_HDMI_GPVS_get_vsps(hdmi_in(HDMI_HDMI_GPVS_reg));
}


/*---------------------------------------------------
 * Func : lib_hdmi_clear_vsi_received_status
 *
 * Desc : clear VSI packet receive status
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
int lib_hdmi_clear_vsi_received_status(unsigned char nport)
{
    hdmi_out(HDMI_HDMI_GPVS_reg, HDMI_HDMI_GPVS_vsps_mask);
    return 0;
}


/*---------------------------------------------------
 * Func : lib_hdmi_read_vsi_info_packet
 *
 * Desc : read vsi info packet
 *
 * Para : nport : HDMI port number
 *        pkt_buf : avi output buffer
 *
 * Retn : N/A
 *--------------------------------------------------*/
INFOFRAME_READ_RESULT lib_hdmi_read_vsi_info_packet(
    unsigned char               port,
    unsigned char               *pkt_buf
    )
{
    int checksum_pass;
    if (!lib_hdmi_is_vsi_received(port) || pkt_buf==NULL)
        return VSI_READ_NOT_READY;

    lib_hdmi_clear_vsi_received_status(port);
    
    lib_hdmi_read_packet_sram(port, VS_INFO_CNT_OFST, VS_INFO_CNT_LEN, pkt_buf + VS_INFO_LEN);  // HDMI HW will update whoe Packet, so we just copy all ot it directly
    checksum_pass = lib_hdmi_infoframe_packet_read(TYPE_CODE_VS_PACKET, port, VS_INFO_OFST + 1, VS_INFO_LEN - 1, pkt_buf + 1);

    pkt_buf[0] = TYPE_CODE_VS_PACKET;
    if (checksum_pass) return INFOFRAME_READ_SUCCESS;
    else return VSI_READ_CHECKSUM_ERROR;
}


//-------------------------------------------------------------------------
// AVI
//-------------------------------------------------------------------------


/*---------------------------------------------------
 * Func : lib_hdmi_is_avi_received
 *
 * Desc : check if AVI packet received
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
unsigned char lib_hdmi_is_avi_received(unsigned char nport)
{
    return HDMI_HDMI_GPVS_get_avips(hdmi_in(HDMI_HDMI_GPVS_reg));
}


/*---------------------------------------------------
 * Func : lib_hdmi_clear_avi_received_status
 *
 * Desc : clear AVI packet receive status
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
void lib_hdmi_clear_avi_received_status(unsigned char nport)
{
    hdmi_out(HDMI_HDMI_GPVS_reg, HDMI_HDMI_GPVS_avips_mask);
}


/*---------------------------------------------------
 * Func : lib_hdmi_read_avi_info_packet
 *
 * Desc : read avi info packet
 *
 * Para : nport : HDMI port number
 *        pkt_buf : avi output buffer
 *
 * Retn : 0 pass, others failed
 *--------------------------------------------------*/
INFOFRAME_READ_RESULT lib_hdmi_read_avi_info_packet(
    unsigned char               port,
    unsigned char               *pkt_buf,
    unsigned char*              p_bch
    )
{
    unsigned char len_extend;
    int checksum_pass = 0;
    
    if (!lib_hdmi_is_avi_received(port) || pkt_buf==NULL || p_bch==NULL)
        return AVI_READ_NOT_READY;

    lib_hdmi_clear_avi_received_status(port);

    lib_hdmi_read_packet_sram(port, AVI_INFO_OFST, 1, p_bch);
    lib_hdmi_read_packet_sram(port, AVI_INFO_OFST + 1, AVI_INFO_LEN -1, pkt_buf);


    // copy extra bytes
    if(pkt_buf[1] > 13)
    {
        len_extend = pkt_buf[1] - 13;

        if(len_extend >= AVI_INFO_CNT_LEN) {
            lib_hdmi_read_packet_sram(port, AVI_INFO_CNT_OFST, AVI_INFO_CNT_LEN, &pkt_buf[16]);
            checksum_pass = newbase_hdmi_infoframe_checksum_verification(TYPE_CODE_AVI_PACKET, AVI_INFO_LEN -1 + AVI_INFO_CNT_LEN, pkt_buf);
        }
        else {// if(len_extend < AVI_PKT_CONT_LEN)
            lib_hdmi_read_packet_sram(port, AVI_INFO_CNT_OFST, len_extend, &pkt_buf[16]);
            checksum_pass = newbase_hdmi_infoframe_checksum_verification(TYPE_CODE_AVI_PACKET, AVI_INFO_LEN -1 + len_extend, pkt_buf);
        }
    }
    else
    {
        checksum_pass = newbase_hdmi_infoframe_checksum_verification(TYPE_CODE_AVI_PACKET, AVI_INFO_LEN -1, pkt_buf);
    }

    pkt_buf[2] = pkt_buf[1];    //len
    pkt_buf[1] = pkt_buf[0];    //version
    pkt_buf[0] = TYPE_CODE_AVI_PACKET;

    if (checksum_pass) return INFOFRAME_READ_SUCCESS;
    else return AVI_READ_CHECKSUM_ERROR;
}


//-------------------------------------------------------------------------
// SPD
//-------------------------------------------------------------------------


/*---------------------------------------------------
 * Func : lib_hdmi_is_spd_received
 *
 * Desc : check if SPD packet received
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
unsigned char lib_hdmi_is_spd_received(unsigned char nport)
{
    return HDMI_HDMI_GPVS_get_spdps(hdmi_in(HDMI_HDMI_GPVS_reg));
}


/*---------------------------------------------------
 * Func : lib_hdmi_clear_spd_received_status
 *
 * Desc : clear SPD packet receive status
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
void lib_hdmi_clear_spd_received_status(unsigned char nport)
{
    hdmi_out(HDMI_HDMI_GPVS_reg, HDMI_HDMI_GPVS_spdps_mask);
}

/*---------------------------------------------------
 * Func : lib_hdmi_read_spd_info_packet
 *
 * Desc : read spd info packet
 *
 * Para : nport : HDMI port number
 *        pkt_buf : avi output buffer
 *
 * Retn : N/A
 *--------------------------------------------------*/
INFOFRAME_READ_RESULT lib_hdmi_read_spd_info_packet(
    unsigned char           port,
    unsigned char           *pkt_buf,
    unsigned char*          p_bch
    )
{
    int checksum_pass;
    if (!lib_hdmi_is_spd_received(port))
        return SPD_READ_NOT_READY;

    lib_hdmi_clear_spd_received_status(port);
    lib_hdmi_read_packet_sram(port, SPD_INFO_OFST, 1, p_bch);
    checksum_pass = lib_hdmi_infoframe_packet_read(TYPE_CODE_SPD_PACKET, port, SPD_INFO_OFST+1, SPD_INFO_LEN -1, pkt_buf);
    // skip checksum
    pkt_buf[2] = pkt_buf[1];    //len
    pkt_buf[1] = pkt_buf[0];    //version
    pkt_buf[0] = TYPE_CODE_SPD_PACKET;
    if (checksum_pass) return INFOFRAME_READ_SUCCESS;
    else return SPD_READ_CHECKSUM_ERROR;
}


//-------------------------------------------------------------------------
// AUD
//-------------------------------------------------------------------------


/*---------------------------------------------------
 * Func : lib_hdmi_is_audiosampkt_received
 *
 * Desc : check if audio sample packet received
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
unsigned char lib_hdmi_is_audiosampkt_received(unsigned char nport)
{
    return HDMI_HDMI_GPVS_get_audps(hdmi_in(HDMI_HDMI_GPVS_reg));
}


/*---------------------------------------------------
 * Func : lib_hdmi_clear_audiosampkt_received_status
 *
 * Desc : clear audio sample packet receive status
 *
 * Para : nport : HDMI port number
 *
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_clear_audiosampkt_received_status(unsigned char nport)
{
    hdmi_out(HDMI_HDMI_GPVS_reg, HDMI_HDMI_GPVS_audps_mask);
}


//-------------------------------------------------------------------------
// HBR_AUD
//-------------------------------------------------------------------------


/*---------------------------------------------------
 * Func : lib_hdmi_is_hbr_audiostreampkt_received
 *
 * Desc : check if high bitrate audio stream packet received
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
unsigned char lib_hdmi_is_hbr_audiostreampkt_received(unsigned char nport)
{
    return HDMI_HDMI_GPVS_get_hbr_audps(hdmi_in(HDMI_HDMI_GPVS_reg));
}


/*---------------------------------------------------
 * Func : lib_hdmi_clear_hbr_audiostreampkt_received_status
 *
 * Desc : clear high bitrate audio stream packet receive status
 *
 * Para : nport : HDMI port number
 *
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_clear_hbr_audiostreampkt_received_status(unsigned char nport)
{
    hdmi_out(HDMI_HDMI_GPVS_reg, HDMI_HDMI_GPVS_hbr_audps_mask);
}


//-------------------------------------------------------------------------
// Audio Packet
//-------------------------------------------------------------------------


/*---------------------------------------------------
 * Func : lib_hdmi_is_audiopkt_received
 *
 * Desc : check if audio packet received
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
unsigned char lib_hdmi_is_audiopkt_received(unsigned char nport)
{
    return HDMI_HDMI_GPVS_get_aps(hdmi_in(HDMI_HDMI_GPVS_reg));
}


/*---------------------------------------------------
 * Func : lib_hdmi_clear_audiopkt_received_status
 *
 * Desc : clear Audio packet receive status
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
void lib_hdmi_clear_audiopkt_received_status(unsigned char nport)
{
    hdmi_out(HDMI_HDMI_GPVS_reg, HDMI_HDMI_GPVS_aps_mask);
}


/*---------------------------------------------------
 * Func : lib_hdmi_read_audio_packet
 *
 * Desc : read audio info packet
 *
 * Para : nport : HDMI port number
 *        pkt_buf : avi output buffer
 *
 * Retn : N/A
 *--------------------------------------------------*/
INFOFRAME_READ_RESULT lib_hdmi_read_audio_packet(
    unsigned char           port,
    unsigned char           *pkt_buf
    )
{
    int checksum_pass;
    if (!lib_hdmi_is_audiopkt_received(port))
        return AUDIO_READ_NOT_READY;

    lib_hdmi_clear_audiopkt_received_status(port);
    checksum_pass = lib_hdmi_infoframe_packet_read(TYPE_CODE_AUDIO_PACKET, port, AUDIO_INFO_OFST+1, AUDIO_INFO_LEN - 1, pkt_buf);
    // skip checksum
    pkt_buf[2] = pkt_buf[1];    //len
    pkt_buf[1] = pkt_buf[0];    //version
    pkt_buf[0] = TYPE_CODE_AUDIO_PACKET;

    if (checksum_pass) return INFOFRAME_READ_SUCCESS;
    else return AUDIO_READ_CHECKSUM_ERROR;
}



//-------------------------------------------------------------------------
// DRM
//-------------------------------------------------------------------------


/*---------------------------------------------------
 * Func : lib_hdmi_is_drm_received
 *
 * Desc : check if drm packet received
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
unsigned char lib_hdmi_is_drm_received(unsigned char nport)
{
    return HDMI_HDMI_GPVS_get_drmps(hdmi_in(HDMI_HDMI_GPVS_reg));
}

/*---------------------------------------------------
 * Func : lib_hdmi_clear_drm_received_status
 *
 * Desc : clear DRM packet receive status
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
void lib_hdmi_clear_drm_received_status(unsigned char nport)
{
    hdmi_out(HDMI_HDMI_GPVS_reg, HDMI_HDMI_GPVS_drmps_mask);
}


/*---------------------------------------------------
 * Func : lib_hdmi_read_drm_packet
 *
 * Desc : read drm info packet
 *
 * Para : nport : HDMI port number
 *        pkt_buf : packet output
 *
 * Retn : 0 : success, -1 : failed
 *--------------------------------------------------*/
INFOFRAME_READ_RESULT lib_hdmi_read_drm_packet(
    unsigned char           port,
    unsigned char           *pkt_buf,
    unsigned char*          p_bch
    )
{
    int checksum_pass;
    if (!lib_hdmi_is_drm_received(port) || pkt_buf==NULL || p_bch==NULL)
        return DRM_READ_NOT_READY;

    lib_hdmi_clear_drm_received_status(port);
    lib_hdmi_read_packet_sram(port, DRM_INFO_OFST, 1, p_bch);

    checksum_pass = lib_hdmi_infoframe_packet_read(TYPE_CODE_DRM_PACKET, port, DRM_INFO_OFST+1, DRM_INFO_LEN-1, pkt_buf);
    // skip checksum
    pkt_buf[2] = pkt_buf[1]; // len
    pkt_buf[1] = pkt_buf[0]; // version
    pkt_buf[0] = TYPE_CODE_DRM_PACKET; // Info Frame Type

    if (checksum_pass) return INFOFRAME_READ_SUCCESS;
    else return DRM_READ_CHECKSUM_ERROR;
}
#endif // UT_flag


//-------------------------------------------------------------------------
// Reserved Packet
//-------------------------------------------------------------------------


/*---------------------------------------------------
 * Func : lib_hdmi_set_rsv_packet_type
 *
 * Desc : set rsv packet type (without OUI check)
 *
 * Para : nport : HDMI port number
 *        index : rsv packet index
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
int lib_hdmi_set_rsv_packet_type(
    unsigned char           nport,
    unsigned char           index,
    unsigned char           pkt_type
    )
{
    // for old API
    return lib_hdmi_set_rsv_packet_type_ex(nport, index, pkt_type, 0, 0);
}


/*---------------------------------------------------
 * Func : lib_hdmi_set_rsv_packet_type_ex
 *
 * Desc : set rsv packet type (with OUI check)
 *
 * Para : nport     : HDMI port number
 *        index     : rsv packet index
 *        pkt_type  : type of packet
 *        check_oui : check oui or not (only valid f
 *        oui_1     : 1st oui of the packet
 *
 * Retn : 0 : successed, -1 : failed
 *--------------------------------------------------*/
int lib_hdmi_set_rsv_packet_type_ex(
    unsigned char           nport,
    unsigned char           index,
    unsigned char           pkt_type,
    unsigned char           check_oui,
    unsigned char           oui_1
    )
{
    if (index >3)
        return -1;

    check_oui = (pkt_type==TYPE_CODE_VS_PACKET && check_oui) ? 1 : 0;

    if (check_oui)
    {
        check_oui = 1;

        if (pkt_type!=TYPE_CODE_VS_PACKET)
        {
            HDMI_WARN("set reserved packet type with OUI check failed, packet type (%02x) is not VSI, force disable OUI check\n", pkt_type);
            check_oui = 0;
            oui_1 = 0;
        }
    }

    // Packet Type
    hdmi_mask(HDMI_HDMI_PTRSV1_reg, ~(HDMI_HDMI_PTRSV1_pt0_mask<<(index<<3)), pkt_type<<(index<<3));

    // OUI pattern
    hdmi_mask(HDMI_HDMI_PTRSV2_reg, ~(HDMI_HDMI_PTRSV2_pt0_oui_1st_mask<<(index<<3)),
                                     (HDMI_HDMI_PTRSV2_pt0_oui_1st(oui_1)<<(index<<3)));

    // clear 1st OUI pattern
    hdmi_mask(HDMI_HDMI_PTRSV3_reg, ~(HDMI_HDMI_PTRSV3_pt0_recognize_oui_en_mask<<(index)),
                                     (HDMI_HDMI_PTRSV3_pt0_recognize_oui_en(check_oui)<<(index)));
    return 0;
}

#ifndef UT_flag
/*---------------------------------------------------
 * Func : lib_hdmi_is_rsv_packet_received
 *
 * Desc : check if rsv packet received
 *
 * Para : nport : HDMI port number
 *        index : rsv packet index
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
int lib_hdmi_is_rsv_packet_received(
    unsigned char           nport,
    unsigned char           index
    )
{
    switch(index) {
    case 0: return HDMI_HDMI_GPVS_get_rsv0ps(hdmi_in(HDMI_HDMI_GPVS_reg));
    case 1: return HDMI_HDMI_GPVS_get_rsv1ps(hdmi_in(HDMI_HDMI_GPVS_reg));
    case 2: return HDMI_HDMI_GPVS_get_rsv2ps(hdmi_in(HDMI_HDMI_GPVS_reg));
    case 3: return HDMI_HDMI_GPVS_get_rsv3ps(hdmi_in(HDMI_HDMI_GPVS_reg));
    default: break;
    }

    return 0;
}


/*---------------------------------------------------
 * Func : lib_hdmi_clear_rsv_packet_status
 *
 * Desc : read reserved info packet
 *
 * Para : nport : HDMI port number
 *        index : rsv packet index
 *
 * Retn : N/A
 *--------------------------------------------------*/
int lib_hdmi_clear_rsv_packet_status(
    unsigned char           nport,
    unsigned char           index
    )
{
    switch(index) {
    case 0: hdmi_out(HDMI_HDMI_GPVS_reg, HDMI_HDMI_GPVS_rsv0ps_mask); break;
    case 1: hdmi_out(HDMI_HDMI_GPVS_reg, HDMI_HDMI_GPVS_rsv1ps_mask); break;
    case 2: hdmi_out(HDMI_HDMI_GPVS_reg, HDMI_HDMI_GPVS_rsv2ps_mask); break;
    case 3: hdmi_out(HDMI_HDMI_GPVS_reg, HDMI_HDMI_GPVS_rsv3ps_mask); break;
    default:
        return -1;
    } 

    return 0;
}


/*---------------------------------------------------
 * Func : lib_hdmi_read_reserved_packet
 *
 * Desc : read reserved packet
 *
 * Para : nport : HDMI port number
 *        index : rsv packet index
 *        pkt_buf : packet output
 *
 * Retn : N/A
 *--------------------------------------------------*/
INFOFRAME_READ_RESULT lib_hdmi_read_reserved_packet(
    unsigned char           port,
    unsigned char           index,
    unsigned char*          pkt_buf,
    unsigned char           len
    )
{
    if (pkt_buf==NULL || !lib_hdmi_is_rsv_packet_received(port, index)) 
        return RESERVED_READ_NOT_READY;

    if (len > 31)
        len = 31;

    switch(index)
    {
    case 0:  lib_hdmi_read_packet_sram(port, RSV0_OFST, len, pkt_buf); break;
    case 1:  lib_hdmi_read_packet_sram(port, RSV1_OFST, len, pkt_buf); break;
    case 2:  lib_hdmi_read_packet_sram(port, RSV2_OFST, len, pkt_buf); break;
    case 3:  lib_hdmi_read_packet_sram(port, RSV3_OFST, len, pkt_buf); break;
    default:
        return -1;
    }

    lib_hdmi_clear_rsv_packet_status(port, index);

    return INFOFRAME_READ_SUCCESS; //no need to check checksum on reserved packet because 
}





/*--------------------------------------------------------------------
 * High level API
 *-------------------------------------------------------------------*/

const char *drm_eotf_name[] = {
	"Traditional gamma - SDR",
	"Traditional gamma - HDR",
	"SMPTE ST 2084",
	"HLG",
	"Reserved",
	"Unknow"
};

const char* _hdmi_drm_eotf_str(unsigned char eotf_type)
{
    switch (eotf_type)
    {
    case 0:
         return "Traditional gamma - SDR";
    case 1:
         return "Traditional gamma - HDR";
    case 2:
         return "SMPTE ST 2084";
    case 3:
         return "HLG";
    case 4:
    case 5:
    case 6:
    case 7: 
        return "Reserved";
    default:
        return "UNKNOW";
    }
}

/*---------------------------------------------------
 * Func : newbase_hdmi_infoframe_rx_init
 *
 * Desc : init info packet rx
 *
 * Para : NA
 *
 * Retn : NA
 *--------------------------------------------------*/
void newbase_hdmi_infoframe_rx_init(void)
{
    _init_info_pack_sem();
#ifdef CONFIG_SUPPORT_DOLBY_VSIF
    _init_dolby_vsif_sem();
#endif
    memset(reserved_data_temp, 0, sizeof(reserved_data_temp));
}

HDMI_QMS_STATUS_T* newbase_hdmi_get_qms_status(unsigned char port)
{
    if(port<4)
        return &m_qms_status[port];
    else
    {
        HDMI_EMG("[newbase_hdmi_get_qms_status] Invalid port index=%d\n", port);
        return NULL;
    }
}

void newbase_hdmi_reset_vsi_infoframe(unsigned char port)
{
    memset(&hdmi_rx[port].vsi_t, 0, sizeof(HDMI_VSI_T));
    memset(&hdmi_rx[port].dvsi_t, 0, sizeof(HDMI_VSI_T));
    memset(&hdmi_rx[port].hdr10pvsi_t, 0, sizeof(HDMI_VSI_T));

    hdmi_rx[port].no_vsi_cnt = 0;
    hdmi_rx[port].no_dvs_cnt = 0;
    hdmi_rx[port].no_hdr10pvsi_cnt = 0;

    /* set DOLBY_HDMI_VSIF_DISABLE if no vsif packet received */
    newbase_hdmi_dolby_vision_enable(port, DOLBY_HDMI_VSIF_DISABLE);
}


/*---------------------------------------------------
 * Func : newbase_hdmi_reset_all_infoframe
 *
 * Desc : clear all info frame that already received
 *
 * Para : nport : HDMI port number
 *
 * Retn : N/A
 *--------------------------------------------------*/
void newbase_hdmi_reset_all_infoframe(unsigned char port, unsigned int flags)
{
    HDMI_INFO("HDMI[p%d] reset all info frame (skip_spd=%d)\n", port, (flags & NO_RESET_SPD));
    // VSI
    newbase_hdmi_reset_vsi_infoframe(port);

    lib_hdmi_clear_fvs_received_status(port);
    lib_hdmi_clear_vsi_received_status(port);
    lib_hdmi_clear_hdr10pvs_received_status(port);

    // AVI
    memset(&hdmi_rx[port].avi_t, 0, sizeof(HDMI_AVI_T));
    hdmi_rx[port].no_avi_cnt = 0;
    lib_hdmi_clear_avi_received_status(port);

    // SPD
    if ((flags & NO_RESET_SPD)==0)
    {
        memset(&hdmi_rx[port].spd_t, 0, sizeof(HDMI_SPD_T));
        hdmi_rx[port].no_spd_cnt = 0;
        lib_hdmi_clear_spd_received_status(port);
    }

    // AUDIO
    memset(&hdmi_rx[port].audiopkt_t, 0, sizeof(HDMI_AUDIO_T));
    hdmi_rx[port].no_audiopkt_cnt = 0;
    lib_hdmi_clear_audiopkt_received_status(port);

    // DRM
    memset(&hdmi_rx[port].drm_t, 0, sizeof(HDMI_DRM_T));
    hdmi_rx[port].no_drm_cnt = 0;
    lib_hdmi_clear_drm_received_status(port);
}

unsigned char newbase_hdmi_check_infoframe_config(unsigned char port, INFO_PACKET_TYPE info_type)
{//TRUE: Enable, FALSE: Disable
    int i = 0;
    unsigned char select_index = 0xFF;
    for(i = 0; i< INFO_TYPE_NUM; i ++)
    {
        if(info_packet_config_array[i].type_name == info_type)
        {
            select_index = i;
            break;
        }
    }
    if(select_index >= INFO_TYPE_NUM)
    {
        HDMI_EMG("[newbase_hdmi_check_infoframe_config] Error input infoframe type: %d, select_index: %d\n", info_type, select_index);
        return FALSE;
    }
    if (newbase_hdmi_get_5v_state(port)==FALSE || GET_H_MODE(port) != MODE_HDMI)
    {
        return FALSE;
    }

    //Change infoframe config with special condition.
    switch(info_packet_config_array[select_index].type_name)
    {
        case INFO_TYPE_EMP:
            if (GET_FLOW_CFG(HDMI_FLOW_CFG_GENERAL, HDMI_FLOW_CFG0_SUPPORT_DSC))
                info_packet_config_array[select_index].detect_start_state = MAIN_FSM_HDMI_WAIT_SYNC;
            else
                info_packet_config_array[select_index].detect_start_state = MAIN_FSM_HDMI_MEASURE;

            break;
        default:
            break;
    }

    if(FALSE == info_packet_config_array[select_index].enable)
    {
        return FALSE;
    }

    if(GET_H_VIDEO_FSM(port) < info_packet_config_array[select_index].detect_start_state)
    {
        return FALSE;
    }

    return TRUE;

}
#endif //UT_flag

INFOFRAME_READ_RESULT newbase_hdmi_update_avi_info(unsigned char port, unsigned char *pkt_buf, INFO_PACKET_CONFIG *config)
{
    INFOFRAME_READ_RESULT result = 0;
    unsigned char bch_error = 0;

    if ((result = lib_hdmi_read_avi_info_packet(port, pkt_buf, &bch_error)) == INFOFRAME_READ_SUCCESS)
    {
        bch_error &= PACKET_BCH_2BIT_ERROR_MASK;
        if (bch_error)
        {
            memset(&hdmi_rx[port].avi_t, 0, sizeof(HDMI_AVI_T));  // clear AVI info

            if ((hdmi_rx[port].err_avi_cnt++ & 0x7)==0x1)     // print debug message to notify bch error
            {                  
                HDMI_EMG("read avi with bch error (%02x), skip it cnt=%d, (payload: %02x %02x %02x %02x %02x %02x %02x %02x ...)\n", 
                    bch_error, hdmi_rx[port].err_avi_cnt,
                    pkt_buf[0], pkt_buf[1], pkt_buf[2], pkt_buf[3],
                    pkt_buf[4], pkt_buf[5], pkt_buf[6], pkt_buf[7]);
            }
        }
        else {
            memcpy(&hdmi_rx[port].avi_t, pkt_buf, sizeof(HDMI_AVI_T));
            hdmi_rx[port].err_avi_cnt = 0;
        }

        hdmi_rx[port].no_avi_cnt = 0;
        hdmi_rx[port].pkt_cnt++;
    }
    else
    {
        if (++hdmi_rx[port].no_avi_cnt > config->no_pkt_thd)
        {
            memset(&hdmi_rx[port].avi_t, 0, sizeof(HDMI_AVI_T));
            hdmi_rx[port].no_avi_cnt = 0;
        }
    }

    return result;
}


INFOFRAME_READ_RESULT newbase_hdmi_update_dvs_info(unsigned char port, unsigned char *pkt_buf, INFO_PACKET_CONFIG *config)
{
    INFOFRAME_READ_RESULT result = 0;

    if ((result = lib_hdmi_read_dvs_info_packet(port, pkt_buf))==INFOFRAME_READ_SUCCESS)
    {
        memcpy(&hdmi_rx[port].dvsi_t, pkt_buf, sizeof(HDMI_VSI_T));
        hdmi_rx[port].pkt_cnt++;
        hdmi_rx[port].no_dvs_cnt = 0;

        newbase_hdmi_dolby_vision_enable(port, DOLBY_HDMI_VSIF_STD);
    }
    else
    {
        if (++hdmi_rx[port].no_dvs_cnt > config->no_pkt_thd)
        {
            memset(&hdmi_rx[port].dvsi_t, 0, sizeof(HDMI_VSI_T));
        }
    }

    return result;
}


INFOFRAME_READ_RESULT newbase_hdmi_update_vsi_info(unsigned char port, unsigned char *pkt_buf, INFO_PACKET_CONFIG *config)
{
    INFOFRAME_READ_RESULT result = 0;
    unsigned long not_receive_time = 0;
    unsigned long receive_time_diff = 0;
    const unsigned long ULONG_MAX_VALUE = 0-1;

    if ((result = lib_hdmi_read_vsi_info_packet(port, pkt_buf)) == INFOFRAME_READ_SUCCESS)
    {
        hdmi_rx[port].receive_vsif_time = hdmi_get_system_time_ms();
        memcpy(&hdmi_rx[port].vsi_t, pkt_buf, sizeof(HDMI_VSI_T));
        hdmi_rx[port].pkt_cnt++;
        hdmi_rx[port].no_vsi_cnt  = 0;
    
        newbase_hdmi_dolby_vision_enable(port, DOLBY_HDMI_h14B_VSIF);
    }
    else if ((result = lib_hdmi_read_fvs_info_packet(port, pkt_buf))==INFOFRAME_READ_SUCCESS)
    {
        hdmi_rx[port].receive_vsif_time = hdmi_get_system_time_ms();
        memcpy(&hdmi_rx[port].vsi_t, pkt_buf, sizeof(HDMI_VSI_T));
        hdmi_rx[port].no_vsi_cnt  = 0;
    }
    else
    {
        not_receive_time = hdmi_get_system_time_ms();
        if(not_receive_time >= hdmi_rx[port].receive_vsif_time)
        {
            receive_time_diff = not_receive_time - hdmi_rx[port].receive_vsif_time;
        }
        else
        {
            receive_time_diff = (ULONG_MAX_VALUE - hdmi_rx[port].receive_vsif_time) + not_receive_time + 1;
        }
        if ((++hdmi_rx[port].no_vsi_cnt > config->no_pkt_thd)||(receive_time_diff >= VSIF_EXPIRED_TIME))
        {
            memset(&hdmi_rx[port].vsi_t, 0, sizeof(HDMI_VSI_T));
        }
    }

    return result;
}

INFOFRAME_READ_RESULT newbase_hdmi_update_hdr10pvs_info(unsigned char port, unsigned char *pkt_buf, INFO_PACKET_CONFIG *config)
{
    INFOFRAME_READ_RESULT result = 0;

    if ((result = lib_hdmi_read_hdr10pvs_info_packet(port, pkt_buf)) == INFOFRAME_READ_SUCCESS)
    {
        memcpy(&hdmi_rx[port].hdr10pvsi_t, pkt_buf, sizeof(HDMI_VSI_T));
        hdmi_rx[port].no_hdr10pvsi_cnt = 0;
        hdmi_rx[port].pkt_cnt++;
        hdmi_rx[port].hdr10pvsi_t.VSIF_TypeCode  = config->type_code;
    }
    else
    {
        if (++hdmi_rx[port].no_hdr10pvsi_cnt > config->no_pkt_thd)
        {
            memset(&hdmi_rx[port].hdr10pvsi_t, 0, sizeof(HDMI_VSI_T));    //TCL2851M-2952, Amazon STB output HDR10+ and Dolby VSIF at same time.
        }
    }


    return result;
}

INFOFRAME_READ_RESULT newbase_hdmi_update_spd_info(unsigned char port, unsigned char *pkt_buf, INFO_PACKET_CONFIG *config)
{
    INFOFRAME_READ_RESULT result = 0;
    unsigned char bch_error = 0;

    if ((result = lib_hdmi_read_spd_info_packet(port, pkt_buf, &bch_error)) == INFOFRAME_READ_SUCCESS)
    {
        HDMI_SPD_T* p_spd = (HDMI_SPD_T*) pkt_buf;
        INFOFRAME_FLOW_PRINTF("read spd (bch_err=%02x), ver=%d, len=%d, (payload: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x ...)\n", 
                    bch_error, p_spd->ver, p_spd->len,
                    pkt_buf[0], pkt_buf[1], pkt_buf[2], pkt_buf[3],
                    pkt_buf[4], pkt_buf[5], pkt_buf[6], pkt_buf[7],
                    pkt_buf[8], pkt_buf[9], pkt_buf[10], pkt_buf[11],
                    pkt_buf[12], pkt_buf[13], pkt_buf[14], pkt_buf[15]);

        bch_error &= PACKET_BCH_2BIT_ERROR_MASK;  // SPD has no checksum, just check bit error

        if (bch_error || p_spd->ver > 2 || p_spd->len > 28)  // check version and length (MSPG sometimes send incorrect SPD info)
        {
            if ((hdmi_rx[port].err_spd_cnt++ & 0x7)==0x1)     // print debug message to notify bch error
            {
                HDMI_EMG("read invalid spd info (bch_err=%02x), checksum_error=%02x, ver=%d, len=%d, skip it, cnt=%d, (payload: %02x %02x %02x %02x %02x %02x %02x %02x ...)\n",
                    bch_error,  result, p_spd->ver, p_spd->len, hdmi_rx[port].err_spd_cnt,
                    pkt_buf[0], pkt_buf[1], pkt_buf[2], pkt_buf[3],
                    pkt_buf[4], pkt_buf[5], pkt_buf[6], pkt_buf[7]);
            }
        }
        else
        {
            memcpy(&hdmi_rx[port].spd_t, pkt_buf, sizeof(HDMI_SPD_T));
            hdmi_rx[port].err_spd_cnt = 0;
        }

        hdmi_rx[port].no_spd_cnt = 0;
        hdmi_rx[port].pkt_cnt++;
    }
    else
    {
        if (++hdmi_rx[port].no_spd_cnt > config->no_pkt_thd)
        {
            INFOFRAME_FLOW_PRINTF("[p%d] no SPD for %d rounds, clear SPD structure\n", port, hdmi_rx[port].no_spd_cnt);
            memset(&hdmi_rx[port].spd_t, 0, sizeof(HDMI_SPD_T));  // clear AVI info;
            hdmi_rx[port].no_spd_cnt = 0;
        }
    }

    return result;
}

INFOFRAME_READ_RESULT newbase_hdmi_update_drm_info(unsigned char port, unsigned char *pkt_buf)
{
    INFOFRAME_READ_RESULT result = 0;
    unsigned char bch_error = 0;

    if ((result = lib_hdmi_read_drm_packet(port, pkt_buf, &bch_error)) == INFOFRAME_READ_SUCCESS)
    {
        HDMI_DRM_T* p_drm = (HDMI_DRM_T*) pkt_buf;
        bch_error &= PACKET_BCH_2BIT_ERROR_MASK;
        if(bch_error || p_drm->len > 28)  // check length
        {
            memset(&hdmi_rx[port].drm_t, 0, sizeof(HDMI_DRM_T));  // clear DRM info
            if ((hdmi_rx[port].err_drm_cnt++ & 0x7)==0x1)     // print debug message to notify bch error
            {
                HDMI_EMG("read invalid drm info (bch_err=%02x), ver=%d, len=%d, skip it, cnt=%d, (payload: %02x %02x %02x %02x %02x %02x %02x %02x ...)\n",
                    bch_error,  p_drm->ver, p_drm->len, hdmi_rx[port].err_drm_cnt,
                    pkt_buf[0], pkt_buf[1], pkt_buf[2], pkt_buf[3],
                    pkt_buf[4], pkt_buf[5], pkt_buf[6], pkt_buf[7]);
            }
        }
        else
        {
            ((unsigned char*)(&hdmi_rx[port].drm_t))[0] = pkt_buf[0]; // Info Frame Type
            ((unsigned char*)(&hdmi_rx[port].drm_t))[1] = pkt_buf[1]; // version
            ((unsigned char*)(&hdmi_rx[port].drm_t))[2] = pkt_buf[2]; // len
            hdmi_rx[port].drm_t.eEOTFtype  = pkt_buf[3];
            hdmi_rx[port].drm_t.eMeta_Desc = pkt_buf[4];
            hdmi_rx[port].drm_t.display_primaries_x0 = pkt_buf[5]|(pkt_buf[6]<<8);
            hdmi_rx[port].drm_t.display_primaries_y0 = pkt_buf[7]|(pkt_buf[8]<<8);
            hdmi_rx[port].drm_t.display_primaries_x1 = pkt_buf[9]|(pkt_buf[10]<<8);
            hdmi_rx[port].drm_t.display_primaries_y1 = pkt_buf[11]|(pkt_buf[12]<<8);
            hdmi_rx[port].drm_t.display_primaries_x2 = pkt_buf[13]|(pkt_buf[14]<<8);
            hdmi_rx[port].drm_t.display_primaries_y2 = pkt_buf[15]|(pkt_buf[16]<<8);
            hdmi_rx[port].drm_t.white_point_x = pkt_buf[17]|(pkt_buf[18]<<8);
            hdmi_rx[port].drm_t.white_point_y = pkt_buf[19]|(pkt_buf[20]<<8);
            hdmi_rx[port].drm_t.max_display_mastering_luminance = pkt_buf[21]|(pkt_buf[22]<<8);
            hdmi_rx[port].drm_t.min_display_mastering_luminance = pkt_buf[23]|(pkt_buf[24]<<8);
            hdmi_rx[port].drm_t.maximum_content_light_level = pkt_buf[25]|(pkt_buf[26]<<8);
            hdmi_rx[port].drm_t.maximum_frame_average_light_level = pkt_buf[27]|(pkt_buf[28]<<8);
            hdmi_rx[port].no_drm_cnt = 0;
            hdmi_rx[port].pkt_cnt++;
            hdmi_rx[port].err_drm_cnt = 0;
        }
    }
    else
    {
        if (++hdmi_rx[port].no_drm_cnt > NO_DRM_INFO_MAX) {
            memset(&hdmi_rx[port].drm_t, 0, sizeof(HDMI_DRM_T));
            hdmi_rx[port].no_drm_cnt = 0;
        }
    }

    return result;
}


INFOFRAME_READ_RESULT newbase_hdmi_update_audio_info(unsigned char port, unsigned char *pkt_buf)
{
    INFOFRAME_READ_RESULT result = 0;

    if ((result = lib_hdmi_read_audio_packet(port, pkt_buf)) == INFOFRAME_READ_SUCCESS)
    {
        memcpy(&hdmi_rx[port].audiopkt_t, pkt_buf, 13);
        hdmi_rx[port].no_audiopkt_cnt = 0;
        hdmi_rx[port].pkt_cnt++;
    }
    else
    {
        if (++hdmi_rx[port].no_audiopkt_cnt > NO_AUDIO_INFO_MAX)
        {
            memset(&hdmi_rx[port].audiopkt_t, 0, sizeof(HDMI_AUDIO_T));
            hdmi_rx[port].no_audiopkt_cnt = 0;
        }
    }

    return result;
}


INFOFRAME_READ_RESULT newbase_hdmi_update_RSV0_info(unsigned char port, unsigned char *pkt_buf, INFO_PACKET_CONFIG *config)
{
    INFOFRAME_READ_RESULT result = RESERVED_READ_NOT_READY;

    lib_hdmi_set_rsv_packet_type(port, 0, config->type_code);
    if (lib_hdmi_is_rsv_packet_received(port, 0)) {
        result = lib_hdmi_read_reserved_packet(port, 0, pkt_buf, 31);

        memcpy(&hdmi_rx[port].acp_t, pkt_buf, sizeof(HDMI_ACP_T));
        hdmi_rx[port].acp_t.ACP_Packet_Type  = TYPE_CODE_AUDIO_CONTENT_POTECTION_PACKET;
        hdmi_rx[port].no_rsv0_cnt = 0;
    }
    else {
        if (++hdmi_rx[port].no_rsv0_cnt > NO_RSV0_INFO_MAX) {
            memset(&hdmi_rx[port].acp_t, 0, sizeof(HDMI_ACP_T));
            hdmi_rx[port].no_rsv0_cnt = 0;
        }
    }

    return result;
}

INFOFRAME_READ_RESULT newbase_hdmi_update_RSV2_info(unsigned char port, unsigned char *pkt_buf, INFO_PACKET_CONFIG *config)
{
    INFOFRAME_READ_RESULT result = 0;

    lib_hdmi_set_rsv_packet_type(port, 2, config->type_code);
    if ((result = lib_hdmi_read_reserved_packet(port, 2, pkt_buf, 31)) == INFOFRAME_READ_SUCCESS)
    {
        unsigned int ieeeoui_type = 0;
    
        ieeeoui_type = (pkt_buf[4]) | (pkt_buf[5]<<8) | (pkt_buf[6]<<16);
    
        //only for HDR10+
        if(ieeeoui_type ==IEEE_OUI_HDR10_PLUS)
        {
            memcpy(&hdmi_rx[port].hdr10pvsi_t, pkt_buf, sizeof(HDMI_VSI_T));
            hdmi_rx[port].hdr10pvsi_t.VSIF_TypeCode  = TYPE_CODE_VS_PACKET;
        }
        hdmi_rx[port].pkt_cnt++;
    }
    else
    {
        if (++hdmi_rx[port].no_rsv2_cnt > NO_RSV2_INFO_MAX)
        {
            memset(&hdmi_rx[port].hdr10pvsi_t, 0, sizeof(HDMI_VSI_T));
            hdmi_rx[port].no_rsv2_cnt = 0;
        }
    }

    return result;
}

INFOFRAME_READ_RESULT newbase_hdmi_update_RSV3_info(unsigned char port, unsigned char *pkt_buf, INFO_PACKET_CONFIG *config)
{
    INFOFRAME_READ_RESULT result = 0;

    lib_hdmi_set_rsv_packet_type(port, 3, config->type_code);
    if ((result = lib_hdmi_read_reserved_packet(port, 3, pkt_buf, RESERVED_LEN)) == INFOFRAME_READ_SUCCESS)
    {
        reserved_data_temp[3][port].pkt_received_cnt++; // count for debugging
        memcpy(&reserved_data_temp[3][port].pkt_buf, pkt_buf, sizeof(reserved_data_temp[3][port].pkt_buf));
        hdmi_rx[port].pkt_cnt++;
        if(g_infoframe_rsv3_debug_print)
        {
            HDMI_EMG("GET RSV3 INFO=%x, pkt_cnt=%d, rsv_cnt=%d, data=\n", config->type_code, hdmi_rx[port].pkt_cnt, reserved_data_temp[3][port].pkt_received_cnt);
            HDMI_EMG("[0] 0x%x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x\n",
                pkt_buf[0], pkt_buf[1], pkt_buf[2], pkt_buf[3], pkt_buf[4], pkt_buf[5], pkt_buf[6], pkt_buf[7],
                pkt_buf[8], pkt_buf[9], pkt_buf[10], pkt_buf[11], pkt_buf[12], pkt_buf[13], pkt_buf[14], pkt_buf[15]);
            HDMI_EMG("[16] 0x%x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x\n",
                pkt_buf[16], pkt_buf[17], pkt_buf[18], pkt_buf[19], pkt_buf[20], pkt_buf[21], pkt_buf[22], pkt_buf[23],
                pkt_buf[24], pkt_buf[25], pkt_buf[26], pkt_buf[27], pkt_buf[28], pkt_buf[29], pkt_buf[30]);
        }
    
    }
    else
    {
        //if (++hdmi_rx[port].no_audiopkt_cnt > NO_AUDIO_INFO_MAX)
        {
            //memset(&hdmi_rx[port].audiopkt_t, 0, sizeof(HDMI_AUDIO_T));
            //hdmi_rx[port].no_audiopkt_cnt = 0;
        }
    }

    return result;
}


void newbase_hdmi_update_infoframe(unsigned char port)
{
    unsigned char pkt_buf[31];
    unsigned char is_drm_here = hdmi_rx[port].drm_t.type_code;
    unsigned char is_avi_here = hdmi_rx[port].avi_t.type_code;
    unsigned char is_spd_here = hdmi_rx[port].spd_t.type_code;
    unsigned char is_audio_here = hdmi_rx[port].audiopkt_t.type_code;
    unsigned char is_vsi_here = hdmi_rx[port].vsi_t.VSIF_TypeCode;  // vsi has 2 version-type //861-G
    unsigned char is_dvsi_here = hdmi_rx[port].dvsi_t.VSIF_TypeCode;
    unsigned char is_hdr10pvsi_here = hdmi_rx[port].hdr10pvsi_t.VSIF_TypeCode;

    INFOFRAME_READ_RESULT total_result = 0, curr_result = 0;

    int i = 0;
    
    if (lib_hdmi_get_fw_debug_bit(DEBUG_19_BYPASS_PACKET_DET) ||
        (newbase_hdmi_get_power_saving_state(port)==PS_FSM_POWER_SAVING_ON))
        return;

    for(i = 0; i < INFO_TYPE_NUM; i++)
    {
        if(TRUE == newbase_hdmi_check_infoframe_config(port, info_packet_config_array[i].type_name))
        {
            memset(pkt_buf, 0, sizeof(pkt_buf));
            curr_result = 0;
            switch(info_packet_config_array[i].type_name)
            {
            case INFO_TYPE_AVI:
                curr_result = newbase_hdmi_update_avi_info(port, pkt_buf, info_packet_config_array+i);
                if (curr_result > 0) total_result |= curr_result;
            break;
            case INFO_TYPE_DVS: // Dolby Vision VSIF
                curr_result = newbase_hdmi_update_dvs_info(port, pkt_buf, info_packet_config_array+i);
                if (curr_result > 0) total_result |= curr_result;
            break;
            case INFO_TYPE_VSI: //HDMI14B and HDMI-Fourm VSIF
                curr_result = newbase_hdmi_update_vsi_info(port, pkt_buf, info_packet_config_array+i);
                if (curr_result > 0) total_result |= curr_result;
                break;
            case INFO_TYPE_HDR10PVSI:  //HDR10 Plus VSIF
                curr_result = newbase_hdmi_update_hdr10pvs_info(port, pkt_buf, info_packet_config_array+i);
                if (curr_result > 0) total_result |= curr_result;
                break;
            case INFO_TYPE_SPD:
                curr_result = newbase_hdmi_update_spd_info(port, pkt_buf, info_packet_config_array+i);
                if (curr_result > 0) total_result |= curr_result;
                break;
            case INFO_TYPE_DRM:
                curr_result = newbase_hdmi_update_drm_info(port, pkt_buf);
                if (curr_result > 0) total_result |= curr_result;
                break;
            case INFO_TYPE_AUDIO:
                curr_result = newbase_hdmi_update_audio_info(port, pkt_buf);
                if (curr_result > 0) total_result |= curr_result;
                break;
            case INFO_TYPE_EMP:
                newbase_hdmi_update_emp(port);
                break;
            case INFO_TYPE_RSV0:     // For Audio Content Protection Packet (ACP)
                newbase_hdmi_update_RSV0_info(port, pkt_buf, info_packet_config_array+i);
                break;
            case INFO_TYPE_RSV1:   // Currently for Audio HBR Sample Packet Using. Maintain on Audio thread,
                break;
            case INFO_TYPE_RSV2:   // Currently for Dobly VSIF Packet Using if define enable.Maintain on newbase_hdmi_update_vsi
                curr_result = newbase_hdmi_update_RSV2_info(port, pkt_buf, info_packet_config_array+i);
                if (curr_result > 0) total_result |= curr_result;
                break;
            case INFO_TYPE_RSV3:   // TBD for extended debugging using.
                curr_result = newbase_hdmi_update_RSV3_info(port, pkt_buf, info_packet_config_array+i);
                if (curr_result > 0) total_result |= curr_result;
                break;
            default:
                continue;
                break;
            }
        }
    }

    if (total_result) {
        HDMI_WARN("Infoframe read total result : %s %s %s %s %s %s %s %s %s\n", 
        (total_result & FVS_READ_CHECKSUM_ERROR)? "FVS_READ_CHECKSUM_ERROR," : "", 
        (total_result & DVS_READ_CHECKSUM_ERROR)? "DVS_READ_CHECKSUM_ERROR," : "",
        (total_result & HDR10PVS_READ_CHECKSUM_ERROR)? "HDR10PVS_READ_CHECKSUM_ERROR," : "", 
        (total_result & VSI_READ_CHECKSUM_ERROR)? "VSI_READ_CHECKSUM_ERROR,": "", 
        (total_result & AVI_READ_CHECKSUM_ERROR)? "AVI_READ_CHECKSUM_ERROR,": "",
        (total_result & SPD_READ_CHECKSUM_ERROR)? "SPD_READ_CHECKSUM_ERROR," : "", 
        (total_result & AUDIO_READ_CHECKSUM_ERROR)? "AUDIO_READ_CHECKSUM_ERROR," : "",
        (total_result & DRM_READ_CHECKSUM_ERROR)? "DRM_READ_CHECKSUM_ERROR," : "", 
        (total_result & RESERVED_READ_CHECKSUM_ERROR)? "RESERVED_READ_CHECKSUM_ERROR," : "");
    }

#ifndef UT_flag
    //If debug_print is TRUE, force to print
    if(g_infoframe_drm_debug_print &&(port!=newbase_hdmi_get_current_display_port())) is_drm_here++;
    if(g_infoframe_avi_debug_print&&(port!=newbase_hdmi_get_current_display_port()) )is_avi_here++;
    if(g_infoframe_spd_debug_print&&(port!=newbase_hdmi_get_current_display_port())) is_spd_here++;
    if(g_infoframe_audio_debug_print&&(port!=newbase_hdmi_get_current_display_port())) is_audio_here++;
    if(g_infoframe_vsi_debug_print&&(port!=newbase_hdmi_get_current_display_port())) is_vsi_here++;
    if(g_infoframe_dvs_debug_print&&(port!=newbase_hdmi_get_current_display_port())) is_dvsi_here++;
    if(g_infoframe_hdr10pvsi_debug_print&&(port!=newbase_hdmi_get_current_display_port())) is_hdr10pvsi_here++;
#endif // UT_flag

    if (is_drm_here != hdmi_rx[port].drm_t.type_code) {
        HDMI_WARN("GET DRM INFO=%x, pkt_cnt=%d, EOTF_TYPE=%d\n", hdmi_rx[port].drm_t.type_code, hdmi_rx[port].pkt_cnt, hdmi_rx[port].drm_t.eEOTFtype);
    }
    if (is_avi_here != hdmi_rx[port].avi_t.type_code) {
        HDMI_WARN("GET AVI INFO=%x, pkt_cnt=%d, VIC=%d, pixel_repeat=%d, scaling=%d, Q=%d, YQ=%d\n",
            hdmi_rx[port].avi_t.type_code, hdmi_rx[port].pkt_cnt, hdmi_rx[port].avi_t.VIC, hdmi_rx[port].avi_t.PR, hdmi_rx[port].avi_t.SC, hdmi_rx[port].avi_t.Q, hdmi_rx[port].avi_t.YQ);
    }
    if (is_spd_here != hdmi_rx[port].spd_t.type_code)
    {
        hdmi_amd_freesync_debug_print[port] = g_amd_vsdb_debug_thd;  // force print warning message
        HDMI_WARN("GET SPD INFO=%x, pkt_cnt=%d \n", hdmi_rx[port].spd_t.type_code, hdmi_rx[port].pkt_cnt);
        HDMI_WARN("[SPD] VenderName=<0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x>,(%s)\n",
            hdmi_rx[port].spd_t.VendorName[0], hdmi_rx[port].spd_t.VendorName[1], hdmi_rx[port].spd_t.VendorName[2], hdmi_rx[port].spd_t.VendorName[3],
            hdmi_rx[port].spd_t.VendorName[4], hdmi_rx[port].spd_t.VendorName[5], hdmi_rx[port].spd_t.VendorName[6], hdmi_rx[port].spd_t.VendorName[7],
            hdmi_rx[port].spd_t.VendorName);
        HDMI_WARN("[SPD] ProductDesc=<0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x>,(%s)\n",
            hdmi_rx[port].spd_t.ProductDesc[0], hdmi_rx[port].spd_t.ProductDesc[1], hdmi_rx[port].spd_t.ProductDesc[2], hdmi_rx[port].spd_t.ProductDesc[3],
            hdmi_rx[port].spd_t.ProductDesc[4], hdmi_rx[port].spd_t.ProductDesc[5], hdmi_rx[port].spd_t.ProductDesc[6], hdmi_rx[port].spd_t.ProductDesc[7],
            hdmi_rx[port].spd_t.ProductDesc[8], hdmi_rx[port].spd_t.ProductDesc[9], hdmi_rx[port].spd_t.ProductDesc[10], hdmi_rx[port].spd_t.ProductDesc[11],
            hdmi_rx[port].spd_t.ProductDesc[12], hdmi_rx[port].spd_t.ProductDesc[13], hdmi_rx[port].spd_t.ProductDesc[14], hdmi_rx[port].spd_t.ProductDesc[15],
            hdmi_rx[port].spd_t.ProductDesc);
        HDMI_WARN("[SPD] SourceInfo=%x\n", hdmi_rx[port].spd_t.SourceInfo);
    } 
    if (is_audio_here != hdmi_rx[port].audiopkt_t.type_code) {
        HDMI_WARN("GET AUD INFO=%x, pkt_cnt=%d\n", hdmi_rx[port].audiopkt_t.type_code, hdmi_rx[port].pkt_cnt);
    }
    if (is_vsi_here != hdmi_rx[port].vsi_t.VSIF_TypeCode)
    {
        HDMI_WARN("GET VSI INFO=%x, pkt_cnt=%d, VER=%x, LEN=%x, OUI=(%x,%x,%x)\n",
            hdmi_rx[port].vsi_t.VSIF_TypeCode, hdmi_rx[port].pkt_cnt,  hdmi_rx[port].vsi_t.VSIF_Version, hdmi_rx[port].vsi_t.Length, hdmi_rx[port].vsi_t.Reg_ID[0], hdmi_rx[port].vsi_t.Reg_ID[1], hdmi_rx[port].vsi_t.Reg_ID[2]);
        // 0xd8, 0x5d, 0xc4 is hdmi2.0 FVSPS                        (QD980 Forum VS)
        // 0x03, 0x0c, 0x00 is hdmi1.4 VSPS or Dolby         (QD980 LLC VS)
        // QD980 has a bug, it needs to toggle & apply twice then send correct info.
    }

    if (is_dvsi_here != hdmi_rx[port].dvsi_t.VSIF_TypeCode)
    {
        HDMI_WARN("GET DOLBY VSI INFO=%x, pkt_cnt=%d, VER=%x, LEN=%x, OUI=(%x,%x,%x)\n",
            hdmi_rx[port].dvsi_t.VSIF_TypeCode, hdmi_rx[port].pkt_cnt,  hdmi_rx[port].dvsi_t.VSIF_Version, hdmi_rx[port].dvsi_t.Length, hdmi_rx[port].dvsi_t.Reg_ID[0], hdmi_rx[port].dvsi_t.Reg_ID[1], hdmi_rx[port].dvsi_t.Reg_ID[2]);
    }
    if (is_hdr10pvsi_here != hdmi_rx[port].hdr10pvsi_t.VSIF_TypeCode)
    {
        HDMI_WARN("GET HDR10PLUS VSI INFO=%x, pkt_cnt=%d, VER=%x, LEN=%x, OUI=(%x,%x,%x)\n",
            hdmi_rx[port].hdr10pvsi_t.VSIF_TypeCode, hdmi_rx[port].pkt_cnt,  hdmi_rx[port].hdr10pvsi_t.VSIF_Version, hdmi_rx[port].hdr10pvsi_t.Length, hdmi_rx[port].hdr10pvsi_t.Reg_ID[0], hdmi_rx[port].hdr10pvsi_t.Reg_ID[1], hdmi_rx[port].hdr10pvsi_t.Reg_ID[2]);
    }

    //Debug print
    if (AMDFREESYNC_FLOW_PRINT_FLAG && (hdmi_amd_freesync_debug_print[port]++ >= g_amd_vsdb_debug_thd)) 
    {
        // Note : the checksum byte of is removed.
        unsigned char* p_pb = (unsigned char*) hdmi_rx[port].spd_t.VendorName;

        if (hdmi_rx[port].spd_t.type_code !=0 && newbase_hdmi_check_freesync_info_version(&hdmi_rx[port].spd_t) == FREESYNC_INFO_VERSION1)
        {
            AMDFREESYNC_FLOW_PRINTF("GetAMDFreeSyncStatus : Port=%d, pkt_cnt=%d, AMD SPD (V1), fs_support=%d, fs_en=%d, fs_act=%d, rate=%d~%d\n",
                port,
                hdmi_rx[port].pkt_cnt,
                (p_pb[5]) & 0x1,
                (p_pb[5]>>1) & 0x1,
                (p_pb[5]>>2) & 0x1,
                (p_pb[6]),
                (p_pb[7]));
        }
        else if (hdmi_rx[port].spd_t.type_code !=0 && newbase_hdmi_check_freesync_info_version(&hdmi_rx[port].spd_t) == FREESYNC_INFO_VERSION2)
        {
            AMDFREESYNC_FLOW_PRINTF("GetAMDFreeSyncStatus : Port=%d, pkt_cnt=%d, AMD SPD (V2), fs_support=%d, fs_en=%d, fs_act=%d, ncs_act=%d, bright_ctrl=%d, ld_disable=%d, rate=%d~%d \n",
                port,
                hdmi_rx[port].pkt_cnt,
                (p_pb[5]) & 0x1,
                (p_pb[5]>>1) & 0x1,
                (p_pb[5]>>2) & 0x1,
                (p_pb[5]>>3) & 0x1,
                (p_pb[5]>>4) & 0x1,
                (p_pb[5]>>5) & 0x1,
                (p_pb[6]),
                (p_pb[7]));
        }
        else
        {
            ;//do nothing
            #if 0 //for debugging
            unsigned char* p_pb2 = (unsigned char*) hdmi_rx[port].spd_t.ProductDesc;

            AMDFREESYNC_FLOW_PRINTF("GetAMDFreeSyncStatus : Port=%d, not AMD VSIF, SPD (Ver=%d, Len=%d, DATA={%02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x})\n",
                port, hdmi_rx[port].spd_t.ver, hdmi_rx[port].spd_t.len, p_pb[0], p_pb[1], p_pb[2], p_pb[3], p_pb[4], p_pb[5], p_pb[6], p_pb[7], p_pb2[0], p_pb2[1], p_pb2[2]);
            #endif
        }

        hdmi_amd_freesync_debug_print[port] = 0;
    }
}


/*------------------------------------------------------------------
 * Func : newbase_hdmi_get_vs_infoframe
 *
 * Desc : get HDMI VSIF/ HDIM Forum VISF VSIF from a given hdmi port
 *
 * Para : [IN] port  : HDMI port
 *        [IN] p_vsi : VSIF output 
 *
 * Retn : 0 : no Dolby VSIF received
 *        1 : Dolby VSIF received
 *------------------------------------------------------------------*/
unsigned char newbase_hdmi_get_vs_infoframe(unsigned char port, HDMI_VSI_T *p_vsi_t)
{
    if(FALSE == newbase_hdmi_check_infoframe_config(port, INFO_TYPE_VSI))
    {
        return 0;
    }

    if (hdmi_rx[port].vsi_t.VSIF_TypeCode == 0) {
        return 0;
    }

    if (lib_hdmi_get_fw_debug_bit(DEBUG_21_FORCE_NO_INFOPKT)) {
        return 0;
    }

    memcpy(p_vsi_t, &hdmi_rx[port].vsi_t, sizeof(HDMI_VSI_T));
    return 1;
}


/*------------------------------------------------------------------
 * Func : newbase_hdmi_get_dvs_infoframe
 *
 * Desc : get Dolby Vision VSIF from a given hdmi port
 *
 * Para : [IN] port  : HDMI port
 *        [IN] p_vsi : VSIF output 
 *
 * Retn : 0 : no Dolby VSIF received
 *        1 : Dolby VSIF received
 *------------------------------------------------------------------*/
unsigned char newbase_hdmi_get_dvs_infoframe(unsigned char port, HDMI_VSI_T *p_vsi_t)
{
    if (FALSE == newbase_hdmi_check_infoframe_config(port, INFO_TYPE_VSI))
        return 0;

    if (hdmi_rx[port].dvsi_t.VSIF_TypeCode == 0)
        return 0;

    if (lib_hdmi_get_fw_debug_bit(DEBUG_21_FORCE_NO_INFOPKT))
        return 0;

    memcpy(p_vsi_t, &hdmi_rx[port].dvsi_t, sizeof(HDMI_VSI_T));
    return 1;
}

/*------------------------------------------------------------------
 * Func : newbase_hdmi_get_hdr10p_infoframe
 *
 * Desc : get HDR10+ VSIF from a given hdmi port
 *
 * Para : [IN] port  : HDMI port
 *        [IN] p_vsi : VSIF output 
 *
 * Retn : 0 : no HDR10+ VSIF received
 *        1 : HDR10+ VSIF received
 *------------------------------------------------------------------*/
unsigned char newbase_hdmi_get_hdr10p_infoframe(unsigned char port, HDMI_VSI_T *p_vsi_t)
{
    if (FALSE == newbase_hdmi_check_infoframe_config(port, INFO_TYPE_VSI))
        return 0;

    if (hdmi_rx[port].hdr10pvsi_t.VSIF_TypeCode == 0)
        return 0;

    if (lib_hdmi_get_fw_debug_bit(DEBUG_21_FORCE_NO_INFOPKT))
        return 0;

    memcpy(p_vsi_t, &hdmi_rx[port].hdr10pvsi_t, sizeof(HDMI_VSI_T));
    return 1;
}

/*------------------------------------------------------------------
 * Func : newbase_hdmi_get_multi_vs_infoframe
 *
 * Desc : get multi vsif from the given hdmi port
 *
 * Para : [IN] port  : HDMI port
 *        [IN] p_vsi : VSIF output list
 *        [IN] n_vsi : size of VSIF output array
 *
 * Retn : number of vsif received
 *------------------------------------------------------------------*/
unsigned char newbase_hdmi_get_multi_vs_infoframe(
    unsigned char           port, 
    HDMI_VSI_T*             p_vsi, 
    unsigned char           n_vsi
    )
{
    int vsi_cnt = 0;

    if (p_vsi==NULL)
        return 0;

    if (n_vsi && newbase_hdmi_get_vs_infoframe(port, p_vsi)) {
        p_vsi++;
        vsi_cnt++;
        n_vsi--;
    }

    if (n_vsi && newbase_hdmi_get_dvs_infoframe(port, p_vsi)) {
        p_vsi++;
        vsi_cnt++;
        n_vsi--;
    }

    if (n_vsi && newbase_hdmi_get_hdr10p_infoframe(port, p_vsi)) {
        p_vsi++;
        vsi_cnt++;
        n_vsi--;
    }
    
    return vsi_cnt;
}


unsigned char newbase_hdmi_get_avi_infoframe(unsigned char port, HDMI_AVI_T *p_avi_t)
{
    if (FALSE == newbase_hdmi_check_infoframe_config(port, INFO_TYPE_AVI))
        return 0;

    if (hdmi_rx[port].avi_t.type_code == 0)
        return 0;

    if (lib_hdmi_get_fw_debug_bit(DEBUG_21_FORCE_NO_INFOPKT))
        return 0;

    memcpy(p_avi_t, &hdmi_rx[port].avi_t, sizeof(HDMI_AVI_T));
    return 1;
}

unsigned char newbase_hdmi_get_spd_infoframe(unsigned char port, HDMI_SPD_T *p_spd_t)
{
    if (FALSE == newbase_hdmi_check_infoframe_config(port, INFO_TYPE_SPD))
        return 0;

    if (hdmi_rx[port].spd_t.type_code == 0)
        return 0;

    if (lib_hdmi_get_fw_debug_bit(DEBUG_21_FORCE_NO_INFOPKT))
        return 0;

    memcpy(p_spd_t, &hdmi_rx[port].spd_t, sizeof(HDMI_SPD_T));
    return 1;
}

unsigned char newbase_hdmi_get_audio_infoframe(unsigned char port, HDMI_AUDIO_T *p_audio_t)
{
    if (FALSE == newbase_hdmi_check_infoframe_config(port, INFO_TYPE_AUDIO))
        return 0;

    if (hdmi_rx[port].audiopkt_t.type_code == 0)
        return 0;

    if (lib_hdmi_get_fw_debug_bit(DEBUG_21_FORCE_NO_INFOPKT))
        return 0;

    memcpy(p_audio_t, &hdmi_rx[port].audiopkt_t, sizeof(HDMI_AUDIO_T));
    return 1;
}


unsigned char newbase_hdmi_get_drm_infoframe(unsigned char port, HDMI_DRM_T *p_drm_t)
{
    if (FALSE == newbase_hdmi_check_infoframe_config(port, INFO_TYPE_DRM))
        return 0;

    if (hdmi_rx[port].drm_t.type_code == 0)
        return 0;

    if (lib_hdmi_get_fw_debug_bit(DEBUG_20_FORCE_NO_DRMPKT))
        return 0;

    memcpy(p_drm_t, &hdmi_rx[port].drm_t, sizeof(HDMI_DRM_T));
    return 1;
}

unsigned char newbase_hdmi_get_acp_infoframe(unsigned char port, HDMI_ACP_T *p_acp_t)
{
    if (FALSE == newbase_hdmi_check_infoframe_config(port, INFO_TYPE_RSV0))
        return 0;

    if (hdmi_rx[port].acp_t.ACP_Packet_Type == 0)
        return 0;

    if (lib_hdmi_get_fw_debug_bit(DEBUG_21_FORCE_NO_INFOPKT))
        return 0;

    memcpy(p_acp_t, &hdmi_rx[port].acp_t, sizeof(HDMI_ACP_T));
    return 1;
}

unsigned char newbase_hdmi_get_fvs_allm(unsigned char port, unsigned char* allm_mode)
{
    unsigned char result = FALSE;
    unsigned int ieeeoui_type = 0;
    if(FALSE == newbase_hdmi_check_infoframe_config(port, INFO_TYPE_VSI))
    {
        return FALSE;
    }

    if (hdmi_rx[port].vsi_t.VSIF_TypeCode == 0)
    {
        return FALSE;
    }

    if (lib_hdmi_get_fw_debug_bit(DEBUG_21_FORCE_NO_INFOPKT))
    {
        return FALSE;
    }

    ieeeoui_type = hdmi_rx[port].vsi_t.Reg_ID[0] | (hdmi_rx[port].vsi_t.Reg_ID[1]<<8) | (hdmi_rx[port].vsi_t.Reg_ID[2]<<16);

    if(ieeeoui_type ==IEEE_OUI_HF_VSDB)
    {    
        *allm_mode = (hdmi_rx[port].vsi_t.Payload[1]&_BIT1)>>1;    
        result =  TRUE;
        //HDMI_PRINTF("newbase_hdmi_get_fvs_allm port %d Read Success \n", port);
    }
    else
    {
        result =  FALSE;
        //HDMI_PRINTF("newbase_hdmi_get_fvs_allm port %d NON HF-VSIF\n", port);
    }
    return result;
}

unsigned char newbase_hdmi_get_vrr_enable(unsigned char port)
{
    HDMI_EM_VTEM_T p_emp_vtem_t;
    p_emp_vtem_t.VRR_EN=0;
    if(newbase_hdmi_get_vtem_info(port, &p_emp_vtem_t))
    {
        return p_emp_vtem_t.VRR_EN;
    }
    else
    {
        return FALSE;
    }
}

unsigned char newbase_hdmi_get_qms_enable(unsigned char port)
{
    HDMI_EM_VTEM_T p_emp_vtem_t;
    p_emp_vtem_t.QMS_EN=0;
    if(newbase_hdmi_get_vtem_info(port, &p_emp_vtem_t))
    {
        return p_emp_vtem_t.QMS_EN;
    }
    else
    {
        return FALSE;
    }
}

unsigned char newbase_hdmi_get_m_const(unsigned char port)
{
    HDMI_EM_VTEM_T p_emp_vtem_t;
    p_emp_vtem_t.M_CONST=0;
    if(newbase_hdmi_get_vtem_info(port, &p_emp_vtem_t))
    {
        return p_emp_vtem_t.M_CONST;
    }
    else
    {
        return FALSE;
    }
}

unsigned char newbase_hdmi_get_next_tfr(unsigned char port)
{
    HDMI_EM_VTEM_T p_emp_vtem_t;
    p_emp_vtem_t.Next_TFR=0;
    if(newbase_hdmi_get_vtem_info(port, &p_emp_vtem_t))
    {
        return p_emp_vtem_t.Next_TFR;
    }
    else
    {
        return FALSE;
    }
}

unsigned char newbase_hdmi_get_fva_factor(unsigned char port)
{
    HDMI_EM_VTEM_T p_emp_vtem_t;
    p_emp_vtem_t.FVA_Factor_M1 = 0;

    if (newbase_hdmi_get_vtem_info(port, &p_emp_vtem_t))
        return p_emp_vtem_t.FVA_Factor_M1;
    else
        return 0;
}

FREESYNC_INFO_VERSION_TYPE newbase_hdmi_check_freesync_info_version(HDMI_SPD_T *p_spd_t)
{
    unsigned int oui = 0;

    if(p_spd_t == NULL)
        return FREESYNC_INFO_INVALID;

    oui = (p_spd_t->VendorName[2]<<16) + (p_spd_t->VendorName[1]<<8) + p_spd_t->VendorName[0];// For  AMD freesync or other special spec.

    if (p_spd_t->ver==1 && p_spd_t->len == 8 && oui==IEEE_OUI_AMD_VSDB)
        return FREESYNC_INFO_VERSION1;
    else if (p_spd_t->ver==2 && (p_spd_t->len == 0xA||p_spd_t->len == 0x9) && oui==IEEE_OUI_AMD_VSDB)
        return FREESYNC_INFO_VERSION2;
    else
        return FREESYNC_INFO_INVALID;
}

unsigned char newbase_hdmi_get_freesync_info(unsigned char port, HDMI_AMD_FREE_SYNC_INFO* p_info)
{
    HDMI_SPD_T spd;

    memset(p_info, 0, sizeof(HDMI_AMD_FREE_SYNC_INFO));

    if (newbase_hdmi_get_spd_infoframe(port, &spd))
    {
        // Note : the checksum byte of is removed.
        unsigned char* p_pb = (unsigned char*) spd.VendorName;
        unsigned char* p_pb2 = (unsigned char*) spd.ProductDesc;

        if (newbase_hdmi_check_freesync_info_version(&spd) == FREESYNC_INFO_VERSION1)
        {
            p_info->version = 1;
            p_info->freesync_supported = (p_pb[5]) & 0x1;
            p_info->freesync_enabled   = (p_pb[5]>>1) & 0x1;
            p_info->freesync_activate  = (p_pb[5]>>2) & 0x1;
            p_info->min_refresh_rate   = (p_pb[6]);
            p_info->max_refresh_rate   = (p_pb[7]);

            return TRUE;
        }
        else if (newbase_hdmi_check_freesync_info_version(&spd) == FREESYNC_INFO_VERSION2)
        {
            p_info->version = 2;
            p_info->freesync_supported        = (p_pb[5])    & 0x1;
            p_info->freesync_enabled          = (p_pb[5]>>1) & 0x1;
            p_info->freesync_activate         = (p_pb[5]>>2) & 0x1;
            p_info->native_color_space_active = (p_pb[5]>>3) & 0x1;
            p_info->brightness_control_active = (p_pb[5]>>4) & 0x1;
            p_info->local_diming_disalbe      = (p_pb[5]>>5) & 0x1;
            p_info->min_refresh_rate          = (p_pb[6]);
            p_info->max_refresh_rate          = (p_pb[7]);
            p_info->srgb_eotf_active          = (p_pb2[0])    & 0x1;
            p_info->bt709_eotf_active         = (p_pb2[0]>>1) & 0x1;
            p_info->gamma_2p2_eotf_active     = (p_pb2[0]>>2) & 0x1;
            p_info->gamma_2p6_eotf_active     = (p_pb2[0]>>3) & 0x1;
            p_info->brightness_control        = (p_pb2[1]);

            return TRUE;
        }
    }

    return FALSE;
}

unsigned char newbase_hdmi_get_freesync_enable(unsigned char port)
{
    HDMI_AMD_FREE_SYNC_INFO fs_info;
    
    if (newbase_hdmi_get_freesync_info(port, &fs_info)==TRUE && (fs_info.freesync_enabled ||fs_info.freesync_activate) )
        return TRUE;

    return FALSE;
}

unsigned char newbase_hdmi_get_dolby_L11_content(unsigned char port, unsigned char *get_data)
{
    unsigned char result = FALSE;
    unsigned int ieeeoui_type = 0;
    unsigned char frl_mode = newbase_rxphy_get_frl_mode(port);

    if(FALSE == newbase_hdmi_check_infoframe_config(port, INFO_TYPE_VSI))
    {
        return FALSE;
    }

    if (hdmi_rx[port].vsi_t.VSIF_TypeCode == 0)
    {
        return FALSE;
    }

    if (lib_hdmi_get_fw_debug_bit(DEBUG_21_FORCE_NO_INFOPKT))
    {
        return FALSE;
    }

    if(frl_mode == MODE_TMDS)
    {//From VSIF
        ieeeoui_type = hdmi_rx[port].vsi_t.Reg_ID[0] | (hdmi_rx[port].vsi_t.Reg_ID[1]<<8) | (hdmi_rx[port].vsi_t.Reg_ID[2]<<16);

        if(ieeeoui_type ==IEEE_OUI_DOLBY_VSIF)
        {    
            get_data[0] = hdmi_rx[port].vsi_t.Payload[6]; // Content sub type/Content type
            get_data[1] = hdmi_rx[port].vsi_t.Payload[7]; // Intend White Point
            get_data[2] = hdmi_rx[port].vsi_t.Payload[8]; // L11_Byte2
            get_data[3] = hdmi_rx[port].vsi_t.Payload[9]; // L11_Byte3

            result =  TRUE;
        }
        else
        {
            result =  FALSE;
        }
    }
    else
    {//From EMP VSEM
        unsigned char vsem_pkt[EM_VSEM_INFO_LEN];
        unsigned char l11_present = 0;
        memset(vsem_pkt, 0, EM_VSEM_INFO_LEN);
        if(newbase_hdmi_get_vsem_emp(port, vsem_pkt)>0)
        {
            l11_present = (vsem_pkt[5]&_BIT5)>>5; // Follow DolbyVision_Metadata_Structure_for_Source_and_Sink_Connections_Specification_V1.1.pdf
            if(l11_present == 1) //support L11
            {//TBD, Follow DolbyVision_Metadata_Structure_for_Source_and_Sink_Connections_Specification_V1.1.pdf
                get_data[0] = vsem_pkt[10]; // Content sub type/Content type
                get_data[1] = vsem_pkt[11]; // Intend White Point
                get_data[2] = vsem_pkt[12]; // L11_Byte2
                get_data[3] = vsem_pkt[13]; // L11_Byte3
                result =  TRUE;
            }
            else
            {//NO L11 Data
                HDMI_INFO("[newbase_hdmi_get_dolby_L11_content port:%d] No L11 Data", port);
                result =  FALSE;
            }
        }
        else
        {
            //no emp
            result =  FALSE;
        }
    }
    return result;
}

unsigned char newbase_hdmi_check_spd_name(unsigned char port, unsigned char *vendor_name)
{
    unsigned char i;

    if (hdmi_rx[port].spd_t.type_code == 0)
        return 0;

    for (i=0; i<8; i++)
    {
        if (hdmi_rx[port].spd_t.VendorName[i] != vendor_name[i])
        {
            return 0;
        }
    }

    return 1;
}

unsigned char newbase_hdmi_check_spd_prod(unsigned char port, unsigned char *prod_desc)
{
    unsigned char i;

    if (hdmi_rx[port].spd_t.type_code == 0)
        return 0;

    for (i=0; i<16; i++)
    {
        if (hdmi_rx[port].spd_t.ProductDesc[i] != prod_desc[i])
        {
            return 0;
        }
    }

    return 1;
}


#ifndef BUILD_QUICK_SHOW
#ifndef UT_flag

/**********************************************************************************************
*
*   Info Frmae Thread 
*
**********************************************************************************************/
static bool infoframe_flag = false;
static struct task_struct *infoframe_task;
#endif
unsigned char ucInfoframe_Thread_Stop = _FALSE;
#ifndef UT_flag


/*---------------------------------------------------
 * Func : newbase_hdmi_infoframe_rx_enable
 *
 * Desc : enable / disable HDMI info packet rx.
 *        when enabled, HDMI driver creates a kernel thread
 *        for HDMI info frame receiving
 *
 * Para : on : 0 : disabled, others: neabled
 *
 * Retn : N/A
 *--------------------------------------------------*/
static int _infoframe_rx_thread(void* arg)
{
    set_freezable(); //remove thread flag PF_NOFREEZE
    while (!kthread_freezable_should_stop(NULL) && infoframe_flag)
    {
#ifdef CONFIG_SUPPORT_DOLBY_VSIF
        _lock_dolby_vsif_sem();

        if (_FALSE == hdmi_get_infoframe_thread_stop())
        {
            newbase_hdmi_infoframe_update();
        }
        _unlock_dolby_vsif_sem();
#else
        newbase_hdmi_infoframe_update();
#endif

        msleep_interruptible(40);
    }

    return 0;
}
#endif
/*---------------------------------------------------
 * Func : hdmi_get_infoframe_thread_stop
 *
 * Desc : get info frame thread status
 *
 * Para : N/A
 *
 * Retn : info frame update 
 *--------------------------------------------------*/
unsigned char hdmi_get_infoframe_thread_stop(void)
{
#ifdef CONFIG_SUPPORT_DOLBY_VSIF
    return ucInfoframe_Thread_Stop;
#endif 

    return _FALSE;  // never stop rx
}

#ifdef CONFIG_SUPPORT_DOLBY_VSIF
/*---------------------------------------------------
 * Func : hdmi_set_infoframe_thread_stop
 *
 * Desc : tempolar disable infofram rx update.
 *        this function will called by Scalar driver
 *        when it doesn't want HDMI driver to update info 
 *        frame. in this case, scalar driver will call
 *        newbase_hdmi_infoframe_update to update info frame. 
 *
 * Para : ucStop : stop hdmi info thread update
 *
 * Retn : N/A
 *
 * Note : This patch may need refine in the furture.
 *--------------------------------------------------*/
void hdmi_set_infoframe_thread_stop(unsigned char ucStop)
{
    _lock_dolby_vsif_sem();
    ucInfoframe_Thread_Stop = ucStop;
    _unlock_dolby_vsif_sem();
}
#endif

/*---------------------------------------------------
 * Func : hdmi_set_infoframe_thread_stop_nonseamphore
 *
 * Desc : as hdmi_set_infoframe_thread_stop but without 
 *        semaphore protection (calling at ISR)
 *
 * Para : ucStop : stop hdmi info thread update
 *
 * Retn : N/A
 *
 * Note : This patch may need refine in the furture.
 *--------------------------------------------------*/
void hdmi_set_infoframe_thread_stop_nonseamphore(unsigned char ucStop)// for ISR use
{
    ucInfoframe_Thread_Stop = ucStop;
}

#ifndef UT_flag

/*---------------------------------------------------
 * Func : newbase_hdmi_infoframe_rx_enable
 *
 * Desc : enable / disable HDMI info packet rx.
 *        when enabled, HDMI driver creates a kernel thread
 *        for HDMI info frame receiving
 *
 * Para : on : 0 : disabled, others: neabled
 *
 * Retn : N/A
 *--------------------------------------------------*/
void newbase_hdmi_infoframe_rx_enable(unsigned char on)
{
    int err = 0;

    if (on)
    {
        if (infoframe_flag!=true)
        {
            infoframe_task = kthread_create(_infoframe_rx_thread, NULL, "infoframe_task");

#ifdef CONFIG_SUPPORT_DOLBY_VSIF
            hdmi_set_infoframe_thread_stop(_FALSE);
#endif

            if (IS_ERR(infoframe_task))
            {
                err = PTR_ERR(infoframe_task);
                infoframe_task = NULL;

                infoframe_flag = false;
                HDMI_EMG("%s , Unable to start kernel thread (err_id = %d) infopack .\n", __func__,err);
                return ;
            }

            infoframe_flag = true;
            wake_up_process(infoframe_task);
            HDMI_PRINTF("%s hdmi infopack thread started\n" , __func__ );
        }
    }
    else
    {
        if (infoframe_flag)
        {
            if (kthread_stop(infoframe_task)==0){
                HDMI_PRINTF("hdmi info thread stopped\n");
            }else{
                HDMI_EMG("hdmi info thread stopped error\n");
            }
            infoframe_flag = false;
        }
    }
}
#endif
#endif
INFO_PACKET_CONFIG* newbase_hdmi_get_infopacket_config(INFO_PACKET_TYPE info_type)
{
    unsigned int select_index = 0xFFFF;
    unsigned int i = 0;
    for(i = 0; i< INFO_TYPE_NUM; i ++)
    {
        if(info_packet_config_array[i].type_name == info_type)
        {
            select_index = i;
            break;
        }
    }
    return (select_index != 0xFFFF) ? &info_packet_config_array[select_index] : NULL;
}

RESERVED_PACKET_DATA* newbase_hdmi_get_reserved_data(unsigned char reserved_pkt_index, unsigned char port)
{//TBD
    if(port >= HDMI_PORT_TOTAL_NUM)
        port = newbase_hdmi_get_current_display_port();

    return (reserved_pkt_index<RESERVED_PACKET_NUM) ? &reserved_data_temp[reserved_pkt_index][port] : NULL;
}


/*---------------------------------------------------
 * Func : newbase_hdmi_is_audiosampkt_received
 *
 * Desc : check if audio sample packet received then clear audio sample packet receive status
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
unsigned char newbase_hdmi_is_audiosampkt_received(unsigned char nport)
{
    unsigned char ret=0;
    ret = lib_hdmi_is_audiosampkt_received(nport);
    lib_hdmi_clear_audiosampkt_received_status(nport);
    return ret;
}


/*---------------------------------------------------
 * Func : newbase_hdmi_is_hbr_audiostreampkt_received
 *
 * Desc : check if high bitrate audio stream packet received then clear high bitrate audio stream packet receive status
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
unsigned char newbase_hdmi_is_hbr_audiostreampkt_received(unsigned char nport)
{
    unsigned char ret=0;
    ret = lib_hdmi_is_hbr_audiostreampkt_received(nport);
    lib_hdmi_clear_hbr_audiostreampkt_received_status(nport);
    return ret;
}

#ifndef UT_flag
const char* _hdmi_qms_fsm_str(HDMI_QMS_FSM_E fsm)
{
    switch (fsm) {
    case HDMI_QMS_FSM_NOT_SUPPORT:       return "QMS_NOT_SUPPORT";
    case HDMI_QMS_FSM_VFREQ_CHANGE_START:       return "QMS_VFREQ_CHANGE_START";
    case HDMI_QMS_FSM_VFREQ_CHANGE_RUNNING: return "QMS_VFREQ_CHANGE_RUNNING";
    case HDMI_QMS_FSM_VFREQ_CHANGE_END: return "QMS_VFREQ_CHANGE_END";
    case HDMI_QMS_FSM_READY:         return "QMS_READY";
    default:
    return "QMS_FSM_UNKNOW";
    }
}


#define GET_QMS_FSM(port) (m_qms_status[port].qms_fsm)
#define SET_QMS_FSM(port, fsm)	do { if (m_qms_status[port].qms_fsm !=fsm) { rtd_pr_hdmi_warn("[FSM][QMS] Port:%d, QMS_FSM=%d(%s) -> %d(%s), currnet_M_const=%d, vfreq=%d, TFR=%d\n", port, m_qms_status[port].qms_fsm, _hdmi_qms_fsm_str(m_qms_status[port].qms_fsm), fsm, _hdmi_qms_fsm_str(fsm), m_qms_status[port].current_m_const, m_qms_status[port].current_vfreq, m_qms_status[port].next_tfr); m_qms_status[port].qms_fsm = fsm;  }  }while(0)

const int c_next_tfr_vfreq_list[HDMI_QMS_NEXT_TFR_NUM] =
{
    0,
    23976,
    24000,
    25000,
    29970,
    30000,
    47952,
    48000,
    50000,
    59940,
    60000,
    100000,
    119880,
    120000
};

void newbase_hdmi_qms_status_init(unsigned char port)
{
    m_qms_status[port].qms_fsm = HDMI_QMS_FSM_NOT_SUPPORT;
    m_qms_status[port].current_vfreq = 0;
    m_qms_status[port].pre_vfreq = 0;
    m_qms_status[port].pre_m_const = 0;
    m_qms_status[port].current_m_const = 0;
    m_qms_status[port].next_tfr = 0;
    m_qms_status[port].next_tfr_vfreq = 0;
    m_qms_status[port].pre_qms_en = 0;
    m_qms_status[port].current_qms_en = 0;
    m_qms_status[port].qms_vs_cnt = 0;
}

void newbase_hdmi_qms_handler(unsigned char port)
{
    unsigned char current_qms_en = 0;
    unsigned char current_m_const = 0;
    unsigned char current_next_tfr = 0;
    unsigned int onms_vfreq = 0;
    
    if (newbase_hdmi_get_hpd(port)==0 || newbase_hdmi_get_5v_state(port)==0)
    {
        newbase_hdmi_qms_status_init(port);
        return;
    }

    if (!newbase_rxphy_is_clk_stable(port) || (newbase_hdmi_get_power_saving_state(port)!=PS_FSM_POWER_SAVING_OFF))  // clock not stable or power saving is on ... do nothing ... 
    {
        newbase_hdmi_qms_status_init(port);
        return ;
    }

    // 1. vs_cnt check if handler is not in vgip isr 
    if(_FALSE == hdmi_get_infoframe_thread_stop())
    {
        if((hdmi_rx[port].video_t.vs_valid != 0) && (hdmi_rx[port].video_t.vs_cnt != m_qms_status[port].qms_vs_cnt))
        {//frame count update, enter new frame 
            m_qms_status[port].qms_vs_cnt = hdmi_rx[port].video_t.vs_cnt;
        }
        else
        {
            return; // vs_cnt no update
        }
    }
    else
    {
        m_qms_status[port].qms_vs_cnt++;
    }

    // 2. QMS EN check
    current_qms_en = newbase_hdmi_get_qms_enable(port);
    current_m_const = newbase_hdmi_get_m_const(port);
    current_next_tfr = newbase_hdmi_get_next_tfr(port);
    onms_vfreq  = lib_measure_calc_vfreq(hdmi_rx[port].video_t.vs2vs_cnt); // not lib_hdmims_get_onms3_vfreq, because rtd2851f and rtd2851a don't support DEmode

    if(g_qms_debug_print)
    {//for debugging
    HDMI_EMG("[QMS] port=%d,QMS_FSM:%d, qms_en=%d, m_const=(%d,%d), next_tfr=%d, thread_stop=%d, oringinal_vfreq(onms)=%d, new_vfreq(tfr)=%d, qms_vs_cnt=%d\n",
        port,
        m_qms_status[port].qms_fsm,
        current_qms_en,
        m_qms_status[port].pre_m_const,
        current_m_const,
        current_next_tfr,
        hdmi_get_infoframe_thread_stop(),
        onms_vfreq,
        c_next_tfr_vfreq_list[current_next_tfr], 
        m_qms_status[port].qms_vs_cnt);
    }
    m_qms_status[port].pre_vfreq = onms_vfreq;//Online measure always get previous frame's vfreq
    m_qms_status[port].current_qms_en = current_qms_en;
    m_qms_status[port].current_m_const = current_m_const;
    m_qms_status[port].next_tfr= current_next_tfr;
    if(m_qms_status[port].next_tfr < HDMI_QMS_NEXT_TFR_NUM)
        m_qms_status[port].next_tfr_vfreq = c_next_tfr_vfreq_list[m_qms_status[port].next_tfr];
    else
        m_qms_status[port].next_tfr_vfreq = 0;
   

    if(m_qms_status[port].pre_qms_en != current_qms_en)
    {
        HDMI_WARN("[newbase_hdmi_qms_handler port:%d] vs_cnt=%d, QMS en from %d -> %d, onms_vfreq=%d, current_m_const=%d, current_next_tfr=%d\n",
            port, m_qms_status[port].qms_vs_cnt, m_qms_status[port].pre_qms_en, current_qms_en, onms_vfreq, current_m_const, current_next_tfr);

        m_qms_status[port].pre_qms_en = current_qms_en;

        if(current_qms_en== 0)
        {
            HDMI_WARN("[QMS] port=%d,  QMS_EN=0, back to init\n", port);
            SET_QMS_FSM(port,  HDMI_QMS_FSM_NOT_SUPPORT);
            newbase_hdmi_qms_status_init(port);
            return;
        }
    }

    // 3. M_Const check
    switch(m_qms_status[port].qms_fsm)
    {
    case HDMI_QMS_FSM_NOT_SUPPORT:
        if(newbase_hdmi_get_qms_enable(port))
        {
            if(m_qms_status[port].pre_m_const ==0 && current_m_const ==1)
            {// 0 -> 1, start QMS
                m_qms_status[port].pre_m_const = m_qms_status[port].current_m_const;
                HDMI_WARN("[QMS] port=%d,  (pre, curr) = (%d,%d) in HDMI_QMS_FSM_NOT_SUPPORT, go to READY, onms_vfreq=%d, NEXT_TFR=%d\n", 
                    port, m_qms_status[port].pre_m_const, m_qms_status[port].current_m_const, onms_vfreq, current_next_tfr);

                SET_QMS_FSM(port,  HDMI_QMS_FSM_READY);
            }
            else if(m_qms_status[port].pre_m_const ==0 && current_m_const ==0)
            {// 0 -> 0, keep the same state
                m_qms_status[port].pre_m_const = current_m_const;
                m_qms_status[port].current_m_const = current_m_const;
                m_qms_status[port].pre_vfreq = onms_vfreq; 
            }
        }

        break;
    case HDMI_QMS_FSM_VFREQ_CHANGE_START:
    {
        m_qms_status[port].current_m_const = current_m_const;
        if(m_qms_status[port].pre_m_const==1 && m_qms_status[port].current_m_const==0)
        {//QMS_EN=1, const 1 -> 0,  V Freq will become unstable
            m_qms_status[port].pre_m_const = m_qms_status[port].current_m_const;
            SET_QMS_FSM(port,  HDMI_QMS_FSM_VFREQ_CHANGE_RUNNING);
        }
        else  if(m_qms_status[port].pre_m_const==1 && m_qms_status[port].current_m_const==1)
        {
            HDMI_WARN("[QMS] port=%d,  (pre, curr) = (%d,%d) in VFREQ_CHANGE_START, onms_vfreq=%d, NEXT_TFR=%d\n", 
                port, m_qms_status[port].pre_m_const, m_qms_status[port].current_m_const, onms_vfreq, current_next_tfr);
        }
    }
        break;
    case HDMI_QMS_FSM_VFREQ_CHANGE_RUNNING:
        m_qms_status[port].current_m_const = newbase_hdmi_get_m_const(port);
        if(m_qms_status[port].pre_m_const==0 && m_qms_status[port].current_m_const==0)
        {//QMS_EN=1, const 0 -> 0,  V Freq still unstable
            m_qms_status[port].pre_m_const = m_qms_status[port].current_m_const;
            SET_QMS_FSM(port,  HDMI_QMS_FSM_VFREQ_CHANGE_RUNNING);
        }
        else if(m_qms_status[port].pre_m_const==0 && m_qms_status[port].current_m_const==1)
        {//  QMS_EN=1, m_const 0->1, v freq become stable.  //TX might be ready to output stable frame rate
            m_qms_status[port].pre_m_const = m_qms_status[port].current_m_const;
            SET_QMS_FSM(port,  HDMI_QMS_FSM_VFREQ_CHANGE_END);
        }
        else
        {
            HDMI_WARN("[newbase_hdmi_qms_handler port:%d] Current QMS_FSM:%d, Unexpected current_m_const=%d, pre_m_const=%d, onms_vfreq=%d, NEXT_TFR=%d\n",
                port, m_qms_status[port].qms_fsm, m_qms_status[port].current_m_const, m_qms_status[port].pre_m_const, onms_vfreq, current_next_tfr);
        }

        break;
    case HDMI_QMS_FSM_VFREQ_CHANGE_END:
        m_qms_status[port].current_m_const = newbase_hdmi_get_m_const(port);
        if(m_qms_status[port].pre_m_const==1 && m_qms_status[port].current_m_const==1)
        { //QMS_EN=1, m_const 1->1, //Update final vfreq
            m_qms_status[port].pre_m_const = m_qms_status[port].current_m_const;
            #if 0 // real vfreq
            m_qms_status[port].current_vfreq = current_vfreq;

            #else //use vtem
            if(m_qms_status[port].next_tfr != HDMI_QMS_NEXT_TFR_NOT_ACTIVE && m_qms_status[port].next_tfr <HDMI_QMS_NEXT_TFR_NUM)
            {
                m_qms_status[port].current_vfreq = c_next_tfr_vfreq_list[m_qms_status[port].next_tfr];
            }
            else
            {
                m_qms_status[port].current_vfreq = onms_vfreq; 
            }

            #endif
            SET_QMS_FSM(port,  HDMI_QMS_FSM_READY);
        }
        else if(m_qms_status[port].current_m_const==0 &&m_qms_status[port].pre_m_const==1)
        {//  QMS_EN=1, m_const 1->0, abnormal
            m_qms_status[port].pre_m_const = m_qms_status[port].current_m_const;
            HDMI_WARN("[newbase_hdmi_qms_handler port:%d] Current QMS_FSM:%d, Unexpected current_m_const=%d, pre_m_const=%d, onms_vfreq=%d, NEXT_TFR=%d\n",
                port,  m_qms_status[port].qms_fsm, m_qms_status[port].current_m_const, m_qms_status[port].pre_m_const, onms_vfreq, current_next_tfr);
        }
        else
        {
            m_qms_status[port].pre_m_const = m_qms_status[port].current_m_const;

            HDMI_WARN("[newbase_hdmi_qms_handler port:%d] Current QMS_FSM:%d, Unexpected2 current_m_const=%d, pre_m_const=%d, onms_vfreq=%d, NEXT_TFR=%d\n",
                port,  m_qms_status[port].qms_fsm, m_qms_status[port].current_m_const, m_qms_status[port].pre_m_const, onms_vfreq, current_next_tfr);
        }

        break;
    case HDMI_QMS_FSM_READY:
        m_qms_status[port].current_m_const = newbase_hdmi_get_m_const(port);

        if(m_qms_status[port].pre_m_const==1 && m_qms_status[port].current_m_const==1)
        { //QMS_EN=1, m_const 1->1, 
            m_qms_status[port].pre_m_const = m_qms_status[port].current_m_const;
            SET_QMS_FSM(port,  HDMI_QMS_FSM_READY);
        }
        else if(m_qms_status[port].pre_m_const==1 && m_qms_status[port].current_m_const==0)
        {//  QMS_EN=1, m_const 1->0,
            m_qms_status[port].pre_m_const = m_qms_status[port].current_m_const;
            HDMI_WARN("[newbase_hdmi_qms_handler port:%d] Current QMS_FSM:%d, QMS V freq change!! current_m_const=%d, pre_m_const=%d, onms_vfreq=%d, NEXT_TFR=%d\n",
                port, m_qms_status[port].qms_fsm, m_qms_status[port].current_m_const, m_qms_status[port].pre_m_const, onms_vfreq, current_next_tfr);
            SET_QMS_FSM(port,  HDMI_QMS_FSM_VFREQ_CHANGE_RUNNING);
        }

        break;
    default:
        HDMI_EMG("[newbase_hdmi_qms_handler port:%d][QMS] Invalid qms_fsm!! \n", port);
        break;

    }
    
}
#endif // UT_flag

/*---------------------------------------------------
 * Func : newbase_hdmi_infoframe_update
 *
 * Desc : update infoframe of every hdmi port.
 *
 * Para : N/A
 *
 * Retn : N/A
 *--------------------------------------------------*/
void newbase_hdmi_infoframe_update(void)
{
    int i;
    for( i = 0 ; i < HDMI_PORT_TOTAL_NUM; ++i )
    {
        if((GET_FLOW_CFG(HDMI_FLOW_CFG_HPD, HDMI_FLOW_CFG1_DISABLE_PORT_FAST_SWITCH) == HDMI_PCB_FORCE_ENABLE) && (i != newbase_hdmi_get_current_display_port()))
            continue;

        newbase_hdmi_update_infoframe(i);         // update info frame for each port
#ifndef UT_flag
        newbase_hdmi_qms_handler(i);
#endif // UT_flag
    }
}


