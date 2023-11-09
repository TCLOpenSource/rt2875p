#ifndef __HDMI_INFO_PACKET_H__
#define __HDMI_INFO_PACKET_H__

////////////////////////////////////////////////////////////////
// Contro config
////////////////////////////////////////////////////////////////
#define NO_DRM_INFO_MAX         6
#define NO_AVI_INFO_MAX         8
#define NO_VSI_INFO_MAX         6    // 14B and HF VSIF
#define NO_DVS_INFO_MAX         6
#define NO_HDR10PVS_INFO_MAX         6
#define NO_SPD_INFO_MAX         25  // 40ms * 25 = 1000 ms
#define NO_AUDIO_INFO_MAX       6
#define NO_RSV0_INFO_MAX       6
#define NO_RSV2_INFO_MAX       6


////////////////////////////////////////////////////////////////
// SRAM Offset
////////////////////////////////////////////////////////////////
#define AVI_INFO_OFST       0
#define AUDIO_INFO_OFST     17
#define ACP_OFST            26
#define ISRC1_OFST          31
#define ISRC2_OFST          50
#define MPEG_INFO_OFST      69
#define RSV0_OFST           78
#define RSV1_OFST           109
#define RSV2_OFST           140
#define RSV3_OFST           171
#define VS_INFO_OFST        202
#define GM_OFST             221
#define FVS_INFO_OFST       252
#define DRM_INFO_OFST       264
#define SPD_INFO_OFST       294
#define VS_INFO_CNT_OFST    323
#define AVI_INFO_CNT_OFST   335
#define DVS_INFO_OFST       349
#define HDR10PVS_INFO_OFST  380

#define AVI_INFO_LEN        17
#define AUDIO_INFO_LEN      9
#define ACP_LEN             5
#define ISRC1_LEN           19
#define ISRC2_LEN           19
#define MPEG_INFO_LEN       9
#define RSV_LEN             31
#define VS_INFO_LEN         19
#define GM_LEN              31
#define FVS_INFO_LEN        12
#define DRM_INFO_LEN        30
#define SPD_INFO_LEN        29
#define VS_INFO_CNT_LEN     12
#define AVI_INFO_CNT_LEN    14
#define DVS_INFO_LEN        31
#define HDR10PVS_INFO_LEN   31
#define RESERVED_LEN        31
#define HDMI_INFOF_MAX_LEN  31

#define PACKET_BCH_2BIT_ERROR_MASK  0x1F
#define PACKET_CHECKSUM_ERROR       0x20

//Follow CTA861_G
//Infoframe Packet Type
#define TYPE_CODE_VS_PACKET                             0x81
#define TYPE_CODE_AVI_PACKET                            0x82
#define TYPE_CODE_SPD_PACKET                            0x83
#define TYPE_CODE_AUDIO_PACKET                          0x84
#define TYPE_CODE_MPEG_SRC_PACKET                       0x85
#define TYPE_CODE_NTSC_VBI_PACKET                       0x86
#define TYPE_CODE_DRM_PACKET                            0x87
#define TYPE_CODE_EMP_PACKET                            0x7F

//Packet Type
#define TYPE_CODE_NULL_PACKET                           0x00
#define TYPE_CODE_AUDIO_CLOCK_REGENERATION_PACKET       0x01    //Contain N and CTS
#define TYPE_CODE_AUDIO_SAMPLE_PACKET                   0x02    //Audio Sample (02) (L-PCM and IEC 61937 compressed formats)
#define TYPE_CODE_GENERAL_CONTROL_PACKET                0x03    //GCP
#define TYPE_CODE_AUDIO_CONTENT_POTECTION_PACKET        0x04    //ACP
#define TYPE_CODE_ISRC_PACKET                           0x05    //International Standard Recording Code 1
#define TYPE_CODE_ISRC2_PACKET                          0x06    //International Standard Recording Code 2
#define TYPE_CODE_OB_AUDIO_SAMPLE_PACKET                0x07    //One Bit Audio Sample Packet
#define TYPE_CODE_DST_AUDIO_PACKET                      0x08    //DST (Compressed DSD)
#define TYPE_CODE_HBR_SAMPLE_PACKET                     0x09    //HBR sample
#define TYPE_CODE_GAMUT_METADATA_PACKET                 0x0A    //Gamut metadata
#define TYPE_CODE_3D_AUDIO_SAMPLE_PACKET                0x0B    // 3D Audio Sample Packet for L-PCM only
#define TYPE_CODE_OB3D_AUDIO_SAMPLE_PACKET              0x0C    //One Bit 3D Audio Sample Packet
#define TYPE_CODE_AUDIO_METADATA_PACKET                 0x0D    //Audio Metadata Packet
#define TYPE_CODE_MS_AUDIO_SAMPLE_PACKET                0x0E    //HDMI 2.1 Multi-Stream Audio Sample Packet
#define TYPE_CODE_OBMS_AUDIO_SAMPLE_PACKET              0x0F    //HDMI 2.1 One Bit Multi-Stream Audio Sample Packet

//Infoframe Packet Expired Time
#define VSIF_EXPIRED_TIME 120
#define GET_QMS_FSM(port) (m_qms_status[port].qms_fsm)
#define SET_QMS_FSM(port, fsm)	do { if (m_qms_status[port].qms_fsm !=fsm) { rtd_pr_hdmi_warn("[FSM][QMS] Port:%d, QMS_FSM=%d(%s) -> %d(%s), currnet_M_const=%d, vfreq=%d, TFR=%d\n", port, m_qms_status[port].qms_fsm, _hdmi_qms_fsm_str(m_qms_status[port].qms_fsm), fsm, _hdmi_qms_fsm_str(fsm), m_qms_status[port].current_m_const, m_qms_status[port].current_vfreq, m_qms_status[port].next_tfr); m_qms_status[port].qms_fsm = fsm;  }  }while(0)

typedef enum {
    INFOFRAME_READ_SUCCESS = 0,
    FVS_READ_NOT_READY = -0x1,
    FVS_READ_CHECKSUM_ERROR = 0x1,
    DVS_READ_NOT_READY = -0x2,
    DVS_READ_CHECKSUM_ERROR = 0x2,
    HDR10PVS_READ_NOT_READY = -0x4,
    HDR10PVS_READ_CHECKSUM_ERROR = 0x4,
    VSI_READ_NOT_READY = -0x8,
    VSI_READ_CHECKSUM_ERROR = 0x8,
    AVI_READ_NOT_READY = -0x10,
    AVI_READ_CHECKSUM_ERROR = 0x10,
    SPD_READ_NOT_READY = -0x20,
    SPD_READ_CHECKSUM_ERROR = 0x20,
    AUDIO_READ_NOT_READY = -0x40,
    AUDIO_READ_CHECKSUM_ERROR = 0x40,
    DRM_READ_NOT_READY = -0x80,
    DRM_READ_CHECKSUM_ERROR = 0x80,
    RESERVED_READ_NOT_READY = -0x100,
    RESERVED_READ_CHECKSUM_ERROR = 0x100,
} INFOFRAME_READ_RESULT;

typedef enum
{
    FREESYNC_INFO_INVALID = 0,
    FREESYNC_INFO_VERSION1,
    FREESYNC_INFO_VERSION2
}FREESYNC_INFO_VERSION_TYPE;

typedef enum
{
    INFO_TYPE_AVI,
    INFO_TYPE_SPD,
    INFO_TYPE_DVS,
    INFO_TYPE_VSI,
    INFO_TYPE_HDR10PVSI,
    INFO_TYPE_DRM,
    INFO_TYPE_AUDIO,
    INFO_TYPE_EMP,
    //INFO_TYPE_RSV0,    //TYPE_CODE_AUDIO_SAMPLE_PACKET. Currently for AUDIO thread using.
    INFO_TYPE_RSV0,    //For Audio Content Protection Packet (ACP)
    INFO_TYPE_RSV1,    //TYPE_CODE_HBR_SAMPLE_PACKET. Currently for AUDIO thread using.
    INFO_TYPE_RSV2,    //Reserve for HDR10+ VSIF.
    INFO_TYPE_RSV3,    //Reserved
    INFO_TYPE_NUM
}INFO_PACKET_TYPE;     //Design follow SOC.

typedef struct
{
    INFO_PACKET_TYPE type_name;
    unsigned char enable;
    unsigned char type_code;
    HDMI_MAIN_FSM_T detect_start_state;
    unsigned char no_pkt_thd;
}INFO_PACKET_CONFIG;

typedef struct
{
    unsigned char pkt_buf[RESERVED_LEN];  //Currently only use for reserved3, 20190109
    unsigned int pkt_received_cnt;
}RESERVED_PACKET_DATA;

#define RESERVED_PACKET_NUM    4   //define by IC spec.
////////////////////////////////////////////////////////////////

// Common
extern void lib_hdmi_read_packet_sram(unsigned char nport, unsigned int start_addr, unsigned char len, unsigned char *pbuf);

// FVS
extern int lib_hdmi_is_fvs_received(unsigned char nport);
extern int lib_hdmi_clear_fvs_received_status(unsigned char nport);
extern INFOFRAME_READ_RESULT lib_hdmi_read_fvs_info_packet(unsigned char port, unsigned char *pkt_buf);
extern unsigned char lib_hdmi_read_fvs_allm_mode(unsigned char nport);

// DVS
extern int lib_hdmi_is_dvs_received(unsigned char nport);
extern int lib_hdmi_clear_dvs_received_status(unsigned char nport);
extern INFOFRAME_READ_RESULT lib_hdmi_read_dvs_info_packet(unsigned char port, unsigned char *pkt_buf);

// HDR10+ VS
extern int lib_hdmi_is_hdr10pvs_received(unsigned char nport);
extern int lib_hdmi_clear_hdr10pvs_received_status(unsigned char nport);
extern INFOFRAME_READ_RESULT lib_hdmi_read_hdr10pvs_info_packet(unsigned char port, unsigned char *pkt_buf);

// VS
extern int lib_hdmi_is_vsi_received(unsigned char nport);
extern int lib_hdmi_clear_vsi_received_status(unsigned char nport);
extern INFOFRAME_READ_RESULT lib_hdmi_read_vsi_info_packet(unsigned char port, unsigned char *pkt_buf);

// AVI
extern unsigned char lib_hdmi_is_avi_received(unsigned char nport);
extern void lib_hdmi_clear_avi_received_status(unsigned char nport);
extern INFOFRAME_READ_RESULT  lib_hdmi_read_avi_info_packet(unsigned char port, unsigned char *pkt_buf, unsigned char* p_bch);

// SPD
extern unsigned char lib_hdmi_is_spd_received(unsigned char nport);
extern void lib_hdmi_clear_spd_received_status(unsigned char nport);
extern INFOFRAME_READ_RESULT  lib_hdmi_read_spd_info_packet(unsigned char nport, unsigned char *pkt_buf, unsigned char* p_bch);

// AUD
extern unsigned char lib_hdmi_is_audiosampkt_received(unsigned char nport);
extern void lib_hdmi_clear_audiosampkt_received_status(unsigned char nport);

// HBR AUD
extern unsigned char lib_hdmi_is_hbr_audiostreampkt_received(unsigned char nport);
extern void lib_hdmi_clear_hbr_audiostreampkt_received_status(unsigned char nport);

// Audio
extern unsigned char lib_hdmi_is_audiopkt_received(unsigned char nport);
extern void lib_hdmi_clear_audiopkt_received_status(unsigned char nport);
extern INFOFRAME_READ_RESULT lib_hdmi_read_audio_packet(unsigned char port, unsigned char *pkt_buf);

// DRM
extern unsigned char lib_hdmi_is_drm_received(unsigned char nport);
extern void lib_hdmi_clear_drm_received_status(unsigned char nport);
extern INFOFRAME_READ_RESULT lib_hdmi_read_drm_packet(unsigned char port, unsigned char *pkt_buf, unsigned char* p_bch);

// RSV Packets
extern int lib_hdmi_set_rsv_packet_type(unsigned char nport, unsigned char index, unsigned char pkt_type);
extern int lib_hdmi_set_rsv_packet_type_ex(unsigned char nport, unsigned char index, unsigned char pkt_type, unsigned char check_oui, unsigned char oui_1);
extern int lib_hdmi_is_rsv_packet_received(unsigned char nport, unsigned char index);
extern int lib_hdmi_clear_rsv_packet_status(unsigned char nport, unsigned char index);
extern INFOFRAME_READ_RESULT lib_hdmi_read_reserved_packet(unsigned char nport, unsigned char index, unsigned char* pkt_buf, unsigned char len);

#define lib_hdmi_is_rsv_packet0_received(nport)         lib_hdmi_is_rsv_packet_received(nport, 0)
#define lib_hdmi_is_rsv_packet1_received(nport)         lib_hdmi_is_rsv_packet_received(nport, 1)
#define lib_hdmi_is_rsv_packet2_received(nport)         lib_hdmi_is_rsv_packet_received(nport, 2)
#define lib_hdmi_is_rsv_packet3_received(nport)         lib_hdmi_is_rsv_packet_received(nport, 3)

#define lib_hdmi_clear_rsv_packet0_status(nport)        lib_hdmi_clear_rsv_packet_status(nport, 0)
#define lib_hdmi_clear_rsv_packet1_status(nport)        lib_hdmi_clear_rsv_packet_status(nport, 1)
#define lib_hdmi_clear_rsv_packet2_status(nport)        lib_hdmi_clear_rsv_packet_status(nport, 2)
#define lib_hdmi_clear_rsv_packet3_status(nport)        lib_hdmi_clear_rsv_packet_status(nport, 3)

#define lib_hdmi_read_rsv0_packet(nport, buf, len)      lib_hdmi_read_reserved_packet(nport, 0, buf, len)
#define lib_hdmi_read_rsv1_packet(nport, buf, len)      lib_hdmi_read_reserved_packet(nport, 1, buf, len)
#define lib_hdmi_read_rsv2_packet(nport, buf, len)      lib_hdmi_read_reserved_packet(nport, 2, buf, len)
#define lib_hdmi_read_rsv3_packet(nport, buf, len)      lib_hdmi_read_reserved_packet(nport, 3, buf, len)


////////////////////////////////////////////////////////////////
// High level API
////////////////////////////////////////////////////////////////
extern void _lock_info_pack_sem(void);
extern void _unlock_info_pack_sem(void);

extern const char* _hdmi_drm_eotf_str(unsigned char eotf_type);

extern void newbase_hdmi_infoframe_rx_init(void);
extern unsigned char newbase_hdmi_infoframe_checksum_verification(unsigned char type_code, unsigned int len, unsigned char *pkt_buf);
extern int lib_hdmi_infoframe_packet_read(unsigned char type_code, unsigned char nport, unsigned int start_addr, unsigned char len, unsigned char* pbuf);
extern void newbase_hdmi_infoframe_rx_enable(unsigned char on);
extern void newbase_hdmi_infoframe_update(void);
extern unsigned char hdmi_get_infoframe_thread_stop(void);
extern void hdmi_set_infoframe_thread_stop(unsigned char ucStop);
extern void hdmi_set_infoframe_thread_stop_nonseamphore(unsigned char ucStop);


extern unsigned char newbase_hdmi_check_infoframe_config(unsigned char port, INFO_PACKET_TYPE info_type);
extern void newbase_hdmi_reset_all_infoframe(unsigned char port, unsigned int flags);
#define NO_RESET_SPD   0x1
extern void newbase_hdmi_update_infoframe(unsigned char nport);
extern INFOFRAME_READ_RESULT newbase_hdmi_update_drm_info(unsigned char port, unsigned char *pkt_buf);
extern INFOFRAME_READ_RESULT newbase_hdmi_update_spd_info(unsigned char port, unsigned char *pkt_buf, INFO_PACKET_CONFIG *config);
extern INFOFRAME_READ_RESULT newbase_hdmi_update_hdr10pvs_info(unsigned char port, unsigned char *pkt_buf, INFO_PACKET_CONFIG *config);
extern INFOFRAME_READ_RESULT newbase_hdmi_update_vsi_info(unsigned char port, unsigned char *pkt_buf, INFO_PACKET_CONFIG *config);
extern INFOFRAME_READ_RESULT newbase_hdmi_update_dvs_info(unsigned char port, unsigned char *pkt_buf, INFO_PACKET_CONFIG *config);
extern INFOFRAME_READ_RESULT newbase_hdmi_update_avi_info(unsigned char port, unsigned char *pkt_buf, INFO_PACKET_CONFIG *config);
extern INFOFRAME_READ_RESULT newbase_hdmi_update_audio_info(unsigned char port, unsigned char *pkt_buf);
extern INFOFRAME_READ_RESULT newbase_hdmi_update_RSV0_info(unsigned char port, unsigned char *pkt_buf, INFO_PACKET_CONFIG *config);
extern INFOFRAME_READ_RESULT newbase_hdmi_update_RSV2_info(unsigned char port, unsigned char *pkt_buf, INFO_PACKET_CONFIG *config);
extern INFOFRAME_READ_RESULT newbase_hdmi_update_RSV3_info(unsigned char port, unsigned char *pkt_buf, INFO_PACKET_CONFIG *config);

extern unsigned char newbase_hdmi_get_vs_infoframe(unsigned char port, HDMI_VSI_T *p_vsi_t);
extern unsigned char newbase_hdmi_get_dvs_infoframe(unsigned char port, HDMI_VSI_T *p_vsi_t);
extern unsigned char newbase_hdmi_get_hdr10p_infoframe(unsigned char port, HDMI_VSI_T *p_vsi_t);
extern unsigned char newbase_hdmi_get_avi_infoframe(unsigned char port, HDMI_AVI_T *p_avt_t);
extern unsigned char newbase_hdmi_get_spd_infoframe(unsigned char port, HDMI_SPD_T *p_spd_t);
extern unsigned char newbase_hdmi_get_audio_infoframe(unsigned char port, HDMI_AUDIO_T *p_audio_t);
extern unsigned char newbase_hdmi_get_drm_infoframe(unsigned char port, HDMI_DRM_T *p_drm_t);
extern unsigned char newbase_hdmi_get_acp_infoframe(unsigned char port, HDMI_ACP_T *p_acp_t);
extern unsigned char newbase_hdmi_get_multi_vs_infoframe(unsigned char port, HDMI_VSI_T* p_vsi, unsigned char n_vsi);
extern unsigned char newbase_hdmi_get_fvs_allm(unsigned char port, unsigned char* allm_mode);
extern INFO_PACKET_CONFIG* newbase_hdmi_get_infopacket_config(INFO_PACKET_TYPE info_type);
extern RESERVED_PACKET_DATA* newbase_hdmi_get_reserved_data(unsigned char reserved_pkt_index, unsigned char port);
extern unsigned char newbase_hdmi_get_dolby_L11_content(unsigned char port, unsigned char *get_data);
extern unsigned char newbase_hdmi_get_vrr_enable(unsigned char port);
extern unsigned char newbase_hdmi_get_qms_enable(unsigned char port);
extern unsigned char newbase_hdmi_get_m_const(unsigned char port);
extern unsigned char newbase_hdmi_get_next_tfr(unsigned char port);
extern unsigned char newbase_hdmi_get_fva_factor(unsigned char port);
extern FREESYNC_INFO_VERSION_TYPE newbase_hdmi_check_freesync_info_version(HDMI_SPD_T *p_spd_t);
extern unsigned char newbase_hdmi_get_freesync_info(unsigned char port, HDMI_AMD_FREE_SYNC_INFO* p_info);
extern unsigned char newbase_hdmi_get_freesync_enable(unsigned char port);
extern unsigned char newbase_hdmi_is_audiosampkt_received(unsigned char nport);
extern unsigned char newbase_hdmi_is_hbr_audiostreampkt_received(unsigned char nport);
extern HDMI_QMS_STATUS_T* newbase_hdmi_get_qms_status(unsigned char port);
extern void newbase_hdmi_qms_handler(unsigned char nport);
extern HDMI_QMS_STATUS_T m_qms_status[4];

#endif //__HDMI_INFO_PACKET_H__
