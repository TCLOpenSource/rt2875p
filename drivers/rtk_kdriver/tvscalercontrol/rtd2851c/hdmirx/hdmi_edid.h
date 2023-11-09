#ifndef __HDMI_EDID_H__
#define __HDMI_EDID_H__


#define MAX_EDID_LENGTH 512 //Byte number

// EDID parser offset
#define EDID_OFFSET_ASCII_CODED_MONITOR_NAME    0x71
#define EDID_ASCII_CODED_MONITOR_NAME_LEN    13

typedef enum
{
    HDMI_EDID_CAPABILITY_UNKNOW = 0,
    HDMI_EDID_CAPABILITY_1_4_14B_VSIF = (0x01<<0),    // BIT0, without HF-VSDB,
    HDMI_EDID_CAPABILITY_2_0_HF_VSIF = (0x01<<1),    //BIT1, with HF-VSDB
    HDMI_EDID_CAPABILITY_SCDC_PRESENT = (0x01<<2),    //BIT1, with HF-VSDB
    HDMI_EDID_CAPABILITY_2_1_FRL_RATE_3L_3G = (0x01<<3),    //BIT2,  with HF-VSDB and FRL Rate >0
    HDMI_EDID_CAPABILITY_2_1_FRL_RATE_3L_6G = (0x01<<4),    //BIT2,  with HF-VSDB and FRL Rate >0
    HDMI_EDID_CAPABILITY_2_1_FRL_RATE_4L_6G = (0x01<<5),    //BIT2,  with HF-VSDB and FRL Rate >0
    HDMI_EDID_CAPABILITY_2_1_FRL_RATE_4L_8G = (0x01<<6),    //BIT2,  with HF-VSDB and FRL Rate >0
    HDMI_EDID_CAPABILITY_2_1_FRL_RATE_4L_10G = (0x01<<7),    //BIT2,  with HF-VSDB and FRL Rate >0
    HDMI_EDID_CAPABILITY_2_1_FRL_RATE_4L_12G = (0x01<<8),    //BIT2,  with HF-VSDB and FRL Rate >0
    HDMI_EDID_CAPABILITY_VRR = (0x01<<9),    //BIT2,  with HF-VSDB and FRL Rate >0
    HDMI_EDID_CAPABILITY_ALLM = (0x01<<10),    //BIT2,  with HF-VSDB and FRL Rate >0
    HDMI_EDID_CAPABILITY_FVA = (0x01<<11),    //BIT2,  with HF-VSDB and FRL Rate >0
    HDMI_EDID_CAPABILITY_DSC = (0x01<<12),    //BIT2,  with HF-VSDB and FRL Rate >0
    HDMI_EDID_CAPABILITY_DOLBY_VISION = (0x01<<13),    //BIT2,  with HF-VSDB and FRL Rate >0
    HDMI_EDID_CAPABILITY_HDR10_PLUS = (0x01<<14),    //BIT12,  with HDR10+
    HDMI_EDID_CAPABILITY_DUAL_VIEW = (0x01<<15),    //BIT12,  with HDR10+
    HDMI_EDID_CAPABILITY_3D_OSD_DISPARITY= (0x01<<16),    //BIT12,  with HDR10+
    HDMI_EDID_CAPABILITY_SUPPORT_4K120= (0x01<<17),
    HDMI_EDID_CAPABILITY_SUPPORT_8K= (0x01<<18),
    HDMI_EDID_CAPABILITY_AMD_FREESYNC= (0x01<<19),
    HDMI_EDID_CAPABILITY_HDR10= (0x01<<20),
    HDMI_EDID_CAPABILITY_HLG= (0x01<<21),
    HDMI_EDID_CAPABILITY_YUV420= (0x01<<22),
    HDMI_EDID_CAPABILITY_ADOBERGB= (0x01<<23),
    HDMI_EDID_CAPABILITY_BT2020= (0x01<<24),
    HDMI_EDID_CAPABILITY_DCIP3= (0x01<<25),
    HDMI_EDID_CAPABILITY_CINEMA_VRR = (0x01<<26),
    HDMI_EDID_CAPABILITY_TMDS_3G = (0x01<<27),
    HDMI_EDID_CAPABILITY_TMDS_6G = (0x01<<28),
    HDMI_EDID_CAPABILITY_512_EDID = (0x01<<29),
    HDMI_EDID_CAPABILITY_DOLBY_VISION_HDMI_21 = (0x01<<30),
    HDMI_EDID_CAPABILITY_DOLBY_VISION_LOW_LATENCY = (0x01<<31),
}HDMI_EDID_CAPABILITY;

typedef struct
{
   unsigned char current_edid_table[MAX_EDID_LENGTH];
   unsigned char edid_initialized;
   HDMI_EDID_CAPABILITY edid_capability;    // 0: EDID1.4 without HF-VSDB, 1: EDID2.0 with HF-VSDB
}HDMI_EDID_STATUS_T;


#define HDMI_EDID_VDB_MAX_LEN    60

///////////////////////////////////////////////////////////////////
#define DSC_1P2                   (1<<7)
#define DSC_NATIVE_420(x)         ((x & 0x1) << 6)
#define DSC_ALL_BPP(x)            ((x & 0x1) << 3)
#define DSC_16BPC(x)              ((x & 0x1) << 2)
#define DSC_12BPC(x)              ((x & 0x1) << 1)
#define DSC_10BPC(x)              ((x & 0x1) << 0)

#define DSC_MAX_FRL_RATE(x)       (((x) & 0xF)<<4)
#define DSC_MAX_SLICE(x)          (((x) & 0xF))

#define DSC_TOTAL_CHUNK_BYTES(x)  ((x) & 0x3F)

///////////////////////////////////////////////////////////////////
extern const char* _hdmi_edid_capability_str(HDMI_EDID_CAPABILITY edid_config);
extern void newbase_hdmi_edid_init(void);
extern int  newbase_hdmi_check_hdmi_edid(unsigned char port, unsigned char edid[128]);
extern void newbase_hdmi_edid_enable(unsigned char port, unsigned char enable);
extern unsigned char newbase_hdmi_read_edid(unsigned char port, unsigned char *edid, unsigned int len);
extern unsigned char newbase_hdmi_write_edid(unsigned char port, unsigned char *edid, unsigned int len);
extern unsigned char newbase_hdmi_save_edid(unsigned char port, const unsigned char *edid, const unsigned int len);
extern int newbase_hdmi_is_edid_initialized(unsigned char port);
extern void newbase_hdmi_set_edid_initialized(unsigned char port, unsigned char set_value);
extern unsigned char newbase_hdmi_edid_set_max_frl_rate(unsigned char port, unsigned char max_frl_rate);
extern unsigned char newbase_hdmi_edid_set_dsc_frl_rate(unsigned char port, unsigned char max_frl_rate);
extern unsigned char newbase_hdmi_edid_set_amd_freesync_vsdb(unsigned char port);
extern unsigned char newbase_hdmi_edid_set_vrr_min_max(unsigned char port, unsigned int vrr_min, unsigned int vrr_max);
extern unsigned char newbase_hdmi_edid_set_fva_vsdb(unsigned char port);
extern unsigned char newbase_hdmi_edid_set_qms_vsdb(unsigned char port);
extern unsigned char newbase_hdmi_edid_add_vic_to_vdb(unsigned char port, unsigned char vic);
extern unsigned char newbase_hdmi_edid_add_vic_native_to_vdb(unsigned char port, unsigned char vic);
extern unsigned char newbase_hdmi_edid_del_vic_to_vdb(unsigned char port, unsigned char vic);
extern unsigned char newbase_hdmi_edid_del_vic_native_to_vdb(unsigned char port, unsigned char vic);
extern unsigned char newbase_hdmi_edid_del_all_vic_to_vdb(unsigned char port);
extern HDMI_EDID_STATUS_T* newbase_hdmi_edid_get_status(unsigned char port);
extern unsigned char newbase_hdmi_is_support_edid_capability(unsigned char port, HDMI_EDID_CAPABILITY flag);
extern HDMI_EDID_CAPABILITY newbase_hdmi_check_edid_capability(unsigned char* raw_edid, unsigned int len);

#endif // __HDMI_EDID_H__
