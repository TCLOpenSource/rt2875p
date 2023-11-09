#ifndef __HDMI_EMP_H__
#define __HDMI_EMP_H__


#define EM_CVTEM_INFO_OFST              0
#define EM_VTEM_INFO_OFST               192
#define EM_VSEM_INFO_OFST               224
#define RSV0_EM_OFST                    256
#define RSV1_EM_OFST                    288
#define EM_SBTM_INFO_OFST               320

#define EM_CVTEM_INFO_LEN               192
#define EM_VTEM_INFO_LEN                32
#define EM_VSEM_INFO_LEN                32
#define RSV0_EM_LEN                    32
#define RSV1_EM_LEN                    32
#define EM_SBTM_INFO_LEN                32

#define EM_HDR_EMP_CNT_OFST             0
#define EM_HDR_EMP_INFO_OFST            4
#define MAX_EM_HDR_INFO_LEN             736

#define NO_HDR_EMP_PKT_CNT_MAX         10
#define NO_VTEM_PKT_CNT_MAX         10
#define NO_CVTEM_PKT_CNT_MAX         10
#define NO_SBTM_PKT_CNT_MAX         10
#define NO_RSV0_PKT_CNT_MAX         10
#define NO_RSV1_PKT_CNT_MAX         10

#include "hdmi_common.h"

////////////////////////////////////////////////////////////////

typedef struct 
{
    unsigned char   type;
    unsigned char   reserved:6;
    unsigned char   last:1;
    unsigned char   first:1;
    unsigned char   seq_idx;
    unsigned char   pb[28];
    unsigned char   reserved_2; // for 32 bytes alignment   
}EMP_PKT;

typedef struct 
{
    unsigned char   pps[128];
    unsigned short  hfront;
    unsigned short  hsync;
    unsigned short  hback;
    unsigned short  hcative_bytes;
}CVTEM_INFO;

typedef struct 
{
    unsigned char sync:1;           // when this flag is set, data will be received only in FAPA region
    unsigned char recognize_tag:1;  // when this flag is set, data_set_tag will be checked
    unsigned char recognize_oui:1;  // when this flag is set, oui will be checked
    unsigned char reserved:5;
    unsigned char org_id;
    unsigned char data_set_tag[2];
    unsigned char oui[3];
}EMP_PARAM;

typedef struct 
{
    unsigned char   type;
    unsigned char   reserved:6;
    unsigned char   last:1;
    unsigned char   first:1;
    unsigned char   seq_idx;
    // PB0
    unsigned char   rsv_pb0:1;
    unsigned char   sync:1;
    unsigned char   vfr:1;
    unsigned char   afr:1;
    unsigned char   ds_type:2;
    unsigned char   end:1;
    #ifndef UT_flag
    unsigned char   new:1;
    #endif
    // PB1
    unsigned char   rsv_pb1;
    // PB2
    unsigned char   orig_id;
    // PB3-4
    unsigned char   data_set_tag[2];
    // PB5-6
    unsigned char   data_set_length[2];
    // MD0-2
    unsigned char   oui[3];

}VSEMP_PKT;

#define EMP_ORG_ID_0_VSEM       0
#define EMP_ORG_ID_1_HDMI_2p1   1
#define EMP_ORG_ID_2_CTA_861_G  2
#define EMP_ORG_ID_3_VESA       3

////////////////////////////////////////////////////////////////
// HDR 
extern int lib_hdm_hdr_emp_sram_sel(unsigned char nport, unsigned char hd21);
extern int lib_hdmi_set_em_hdr_config(unsigned char nport, const EMP_PARAM* p_emp_cfg);
extern int lib_hdmi_is_em_hdr_received(unsigned char nport);
extern int lib_hdmi_clear_em_hdr_received_status(unsigned char nport);
extern int lib_hdmi_read_em_hdr_packet(unsigned char port, unsigned char pkt_buf[MAX_EM_HDR_INFO_LEN]);

// CVTEM : Compressed Video Transport Metadata 
extern int lib_hdmi_is_em_cvtem_received(unsigned char nport);
extern int lib_hdmi_clear_em_cvtem_received_status(unsigned char nport);
extern int lib_hdmi_read_em_cvtem_packet(unsigned char port, unsigned char pkt_buf[EM_CVTEM_INFO_LEN]);

// VRR : Video Timing Extended Data Block
extern int lib_hdmi_is_em_vrr_received(unsigned char nport);
extern int lib_hdmi_clear_em_vrr_received_status(unsigned char nport);
extern int lib_hdmi_read_em_vrr_packet(unsigned char port, unsigned char pkt_buf[EM_VTEM_INFO_LEN]);
extern int _parse_cvtem_info(unsigned char pkt_buf[EM_CVTEM_INFO_LEN], CVTEM_INFO* p_info);
extern void _dump_cvtem_info(CVTEM_INFO* p_info);

// VSEM : Vendor Specific Metadata 
extern int lib_hdmi_set_em_vsem_type(unsigned char nport, const EMP_PARAM* p_emp_cfg);
extern int lib_hdmi_is_em_vsem_received(unsigned char nport);
extern int lib_hdmi_clear_em_vsem_received_status(unsigned char nport);
extern int lib_hdmi_read_em_vsem_packet(unsigned char port, unsigned char pkt_buf[EM_VSEM_INFO_LEN]);

////////////////////////////////////////////////////////////////////
extern void newbase_hdmi_update_emp(unsigned char port);
extern void newbase_hdmi_reset_vtem(unsigned char port);
extern void newbase_hdmi_reset_vsem(unsigned char port);
extern void newbase_hdmi_reset_hdr_emp(unsigned char port);
extern void newbase_hdmi_reset_cvtem(unsigned char port);
extern void newbase_hdmi_reset_dolby_vsem(unsigned char port);
extern void newbase_hdmi_reset_all_emp(unsigned char port);

#define NO_RESET_VTEM    0x1
#define NO_RESET_CVTEM   0x2
#define NO_RESET_SBTM    0x4
#define NO_RESET_RSV0    0x8
#define NO_RESET_RSV1    0x10
extern int newbase_hdmi_get_vtem_emp(unsigned char port, unsigned char pkt_buf[EM_VTEM_INFO_LEN]);
extern unsigned char newbase_hdmi_get_vtem_info(unsigned char port, HDMI_EM_VTEM_T *p_emp_vtem_t);
extern int newbase_hdmi_get_cvtem_emp(unsigned char port, unsigned char pkt_buf[EM_CVTEM_INFO_LEN]);
extern int newbase_hdmi_get_vsem_emp(unsigned char port, unsigned char pkt_buf[EM_VSEM_INFO_LEN]);
extern int newbase_hdmi_get_hdr_emp(unsigned char port, unsigned char pkt_buf[MAX_EM_HDR_INFO_LEN]);
extern int newbase_hdmi_get_sbtm_emp(unsigned char port, unsigned char pkt_buf[EM_SBTM_INFO_LEN]);
extern unsigned char newbase_hdmi_get_sbtm_info(unsigned char port, HDMI_EM_SBTM_T *p_emp_sbtm_t);
extern int newbase_hdmi_get_rsv0_emp(unsigned char port, unsigned char pkt_buf[RSV0_EM_LEN]);
extern unsigned char newbase_hdmi_get_rsv0_info(unsigned char port, HDMI_EM_RSV0_T *p_emp_rsv0_t);
extern int newbase_hdmi_get_rsv1_emp(unsigned char port, unsigned char pkt_buf[RSV1_EM_LEN]);
extern unsigned char newbase_hdmi_get_rsv1_info(unsigned char port, HDMI_EM_RSV1_T *p_emp_rsv1_t);

#ifdef  HDMI_EMP_DEBUG_EN
#define HDMI_EMP_DBG(fmt, args...)      HDMI_INFO(fmt, ## args)
#else
#define HDMI_EMP_DBG(fmt, args...)
#endif

#define HDMI_EMP_INFO(fmt, args...)      HDMI_INFO(fmt, ## args)

#ifdef  HDMI_EMP_CHECK_DEBUG_EN
#define EMP_CHK_DBG(fmt, args...)       HDMI_INFO(fmt, ## args)
#define EMP_CHK_WARN(fmt, args...)      HDMI_INFO(fmt, ## args)
#else
#define EMP_CHK_DBG(fmt, args...)
#define EMP_CHK_WARN(fmt, args...)
#endif

#ifdef  HDMI_DOLBY_VSEM_DEBUG_EN
#define DOLBY_VSEM_DBG(fmt, args...)     HDMI_INFO(fmt, ## args)
#else
#define DOLBY_VSEM_DBG(fmt, args...)
#endif

#define DOLBY_VSEM_INFO(fmt, args...)     HDMI_INFO(fmt, ## args)

#endif //__HDMI_EMP_H__
