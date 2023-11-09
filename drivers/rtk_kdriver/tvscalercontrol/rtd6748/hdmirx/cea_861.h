#ifndef __CEA_861_H__
#define __CEA_861_H__

enum
{
    VIC_TB_INTERLACE = 0,
    VIC_TB_PROGRESSIVE
};

typedef enum
{
    VIC_NOT_NATIVE = 0,
    VIC_NATIVE
}VIC_PROPERTY;

typedef enum
{
    VIC_SYNC_HN_VN = 0,
    VIC_SYNC_HP_VN,
    VIC_SYNC_HN_VP,
    VIC_SYNC_HP_VP
}VIC_POLARITY_FLAG;

typedef struct
{
    unsigned char  vic;
    unsigned short hActive;
    unsigned short vActive;
    unsigned char prog;    // 0: interlace, 1: progressive
    unsigned int hFreq;
    unsigned int vFreq;
    unsigned short hTotal;
    unsigned short vTotal;
    unsigned short hSyncWidth;
    unsigned short hBackPorch;
    unsigned short vSyncWidth;
    unsigned short vBackPorch;
    unsigned char PolarityFlag;
}VIC_TIMING_INFO;

typedef struct
{
    unsigned char data[18];
}CEA_DTD_BLK;

#define DTD_H_ACT_H(x)           ((x >>4) & 0xF)
#define DTD_H_BLANKING_H(x)      (x & 0xF)
#define DTD_V_ACT_H(x)           ((x >>4) & 0xF)
#define DTD_V_BLANKING_H(x)      (x & 0xF)
#define DTD_V_SYNC_OFST_L(x)     ((x >>4) & 0xF)
#define DTD_V_SYNC_WIDTH_L(x)    (x & 0xF)
#define DTD_11_H_SYNC_OFST_H(x)  ((x >>6) & 0x3)
#define DTD_11_H_SYNC_WIDTH_H(x) ((x >>4) & 0x3)
#define DTD_11_V_SYNC_OFST_H(x)  ((x >>2) & 0x3)
#define DTD_11_V_SYNC_WIDTH_H(x) ((x) & 0x3)
#define DTD_14_H_IMG_SIZE_H(x)   ((x >>4) & 0xF)
#define DTD_14_V_IMG_SIZE_H(x)   (x & 0xF)
#define DTD_17_INTERLACED(x)     ((x>>7) & 0x1)
#define DTD_17_MODE(x)           ((x>>5) & 0x3)
#define DTD_17_TYPE(x)           ((x>>3) & 0x3)

typedef struct
{
    unsigned short pixel_clock;
    unsigned short h_active;
    unsigned short h_blank;
    unsigned short v_active;
    unsigned short v_blank;
}CEA_DTD_INFO;

//VSIF IEEE_OUI_TYPE
#define IEEE_OUI_HDMI_VSDB      0x000C03 //HDMI14B_VSIF   
#define IEEE_OUI_HF_VSDB        0xC45DD8    //HF_VSIF 
#define IEEE_OUI_AMD_VSDB       0x00001A    //OUT for AMD VSDB
#define IEEE_OUI_DOLBY_VSIF    0x00D046
#define IEEE_OUI_HDR10_PLUS    0x90848B

typedef struct
{
    unsigned char length:5;
    unsigned char tag:3;
    unsigned char oui[3];
    unsigned char B:4;
    unsigned char A:4;
    unsigned char D:4;
    unsigned char C:4;
    //==
    unsigned char dvi_dual:1;
    unsigned char rsvd:2;
    unsigned char dc_y444:1;
    unsigned char dc_30_bits:1;
    unsigned char dc_36_bits:1;
    unsigned char dc_48_bits:1;
    unsigned char support_ai:1;
    //==
    unsigned char max_tmds_clk;
    //==
    unsigned char cnc : 4;
    unsigned char rsvd2 : 1;
    unsigned char hdmi_video_present:1;
    unsigned char i_latency_present:1;
    unsigned char latency_present:1;
    //==
}CEA_HDMI_VSDB_HEAD;


typedef struct
{
    unsigned char video_latency;
    unsigned char audio_latency;
}CEA_HDMI_VSDB_LATENCY;


typedef struct
{
    unsigned char rsvd:3;           // D2:0
    unsigned char image_size:2;     // D4:3
    unsigned char x3d_multi_present:2;  // D6:5
    unsigned char x3d_present:1;    // D7
    unsigned char hdmi_3d_len:5;    // D4:0
    unsigned char hdmi_vic_len:3;   // D7:5
}CEA_HDMI_VSDB_VIDEO_HDR;

typedef struct
{
    CEA_HDMI_VSDB_HEAD*      p_hdr;
    CEA_HDMI_VSDB_LATENCY*   p_latency;
    CEA_HDMI_VSDB_LATENCY*   p_i_latency;
    CEA_HDMI_VSDB_VIDEO_HDR* p_video;
    unsigned char*           p_hdmi_vic;
    unsigned char*           p_hdmi_3d_data;
}CEA_HDMI_VSDB_INFO;

typedef struct
{
    unsigned char length : 5;       // D4:0
    unsigned char tag    : 3;       // D7:5
    unsigned char data[31];
}CEA_DATA_BLOCK;

#define CEA_DATA_BLOCK_SIZE(p_cdb)    (p_cdb->length +1)

typedef enum {
    CEA_861_TAG_0_RESERVED         = 0x0,
    CEA_861_TAG_1_AUDIO_DATA_BLOCK = 0x1,
    CEA_861_TAG_2_VIDEO_DATA_BLOCK = 0x2,
    CEA_861_TAG_3_VENDOR_SPECIFIC_BLOCK = 0x3,
    CEA_861_TAG_4_SPEAKER_ALLOCATION_DATA_BLOCK = 0x4,
    CEA_861_TAG_5_VESA_DISPLAY_TRANSFER_CHARACTER_DATA_BLOCK = 0x5,
    CEA_861_TAG_6_RESERVED = 0x6,
    CEA_861_TAG_7_EXTENED_BLOCK = 0x7,
}CEA_861_TAG;

typedef enum {
    CEA_861_EXT_TAG_0_RESERVED         = 0x0,
    CEA_861_EXT_TAG_1_AUDIO_DATA_BLOCK = 0x1,
    CEA_861_EXT_TAG_2_VIDEO_DATA_BLOCK = 0x2,
    CEA_861_EXT_TAG_3_VENDOR_SPECIFIC_BLOCK = 0x3,
    CEA_861_EXT_TAG_4_SPEAKER_ALLOCATION_DATA_BLOCK = 0x4,
    CEA_861_EXT_TAG_5_VESA_DISPLAY_TRANSFER_CHARACTERISTIC_DATA_BLOCK = 0x5,
    CEA_861_EXT_TAG_6_RESERVED = 0x6,
    CEA_861_EXT_TAG_7_USE_EXTENDED_BLOCK = 0x7,
}CEA_861_EXT_TAG;

typedef enum {
    CEA_861_TAG07_VIDEO_CAPABILITY = 0x00,
    CEA_861_TAG07_VENDOR_SPECIFIC_VIDEO = 0x01, //For consumer VSDB using, ex: Dolby Vision VSDB, HDR10+ VSDB
    CEA_861_TAG07_VESA_DISPLAY_DEVICE = 0x02,
    CEA_861_TAG07_VESA_VIDEO_TIMING_EXTENSION = 0x03,
    CEA_861_TAG07_RESERVED_FOR_HDMI_VIDEO = 0x04,
    CEA_861_TAG07_COLORIMETRY = 0x05,
    CEA_861_TAG07_HDR_STATIC_METADATA = 0x06, //Trad-gamma SDR, Trad-gamma HDR, ST2084HDR10, HLG, 
    CEA_861_TAG07_HDR_DYNAMIC_METADATA = 0x07,
    CEA_861_TAG07_VIDEO_FORMAT_PREFERENCE = 0x0D,
    CEA_861_TAG07_YUV420_VIDEO= 0x0E,
    CEA_861_TAG07_YUV420_CAPABILITY_MAP = 0x0F,
    CEA_861_TAG07_VENDOR_SPECIFIC_AUDIO = 0x11,
    CEA_861_TAG07_ROOM_CONFIGURATION = 0x13,
    CEA_861_TAG07_SPEAKER_LOCATION = 0x14,
    CEA_861_TAG07_INFOFRAME = 0x20,
}CEA_861_TAG07_EXTENDED_TAG_CODE_DATA_BLCOK;

typedef enum {
    CEA_861_EOTF_TRANDITIONAL_GAMMA_SDR_RANGE = 0x1,
    CEA_861_EOTF_TRANDITIONAL_GAMMA_HDR_RANGE = (0x1<<1),
    CEA_861_EOTF_SMPTE_ST_2084 = (0x1<<2), //HDR10
    CEA_861_EOTF_HLG = (0x1<<3),
    CEA_861_EOTF_RESERVED,
}CEA_861_SUPPORTED_EOTF_TYPE;

typedef struct
{
    unsigned char tag;
    unsigned char revision;
    unsigned char dtd_ofst;
    unsigned char n_dtd:4;          // D3:0
    unsigned char YCbCr422:1;       // D4
    unsigned char YCbCr444:1;       // D5
    unsigned char audio:1;          // D6
    unsigned char undescan:1;       // D7
}CEA_EXT_HDR;

typedef struct
{
    CEA_EXT_HDR     hdr;            // CEA Header
    unsigned char   n_cdb;          // number of CDB
    CEA_DATA_BLOCK* p_cdb[32];      // CDB List
    CEA_DTD_BLK*    p_dtd[6];       // DTD List
    unsigned char   checksum;       // ChecjS

}CEA_EXT_BLK_INFO;

#define SAD_AUDIO_FMT(x)    ((x>>3) & 0xF)
#define SAD_MAX_CH(x)       (x & 0x7)
#define SAD2_PROFILE(x)     (x & 0x7)

#define AUDIO_TYPE_REF_HEADER       0
#define AUDIO_TYPE_L_PCM            1
#define AUDIO_TYPE_AC3              2
#define AUDIO_TYPE_MPEG_1           3
#define AUDIO_TYPE_MP3              4
#define AUDIO_TYPE_MPEG_2           5
#define AUDIO_TYPE_AAC_LC           6
#define AUDIO_TYPE_DTS              7
#define AUDIO_TYPE_ATRAC            8
#define AUDIO_TYPE_ONE_BIT_AUDIO    9
#define AUDIO_TYPE_EAC3             10
#define AUDIO_TYPE_DTS_HD           11
#define AUDIO_TYPE_MAT              12
#define AUDIO_TYPE_DST              13
#define AUDIO_TYPE_WMA_PRO          14
#define AUDIO_TYPE_EXTENSION        15

////////////////////////////////////////////////////////////////////////////

extern unsigned char _hdmi_vic_to_vic(unsigned char hdmi_vic);
extern unsigned char _svd_to_vic(unsigned char svd);
extern unsigned char _vic_to_svd(unsigned char vic, VIC_PROPERTY property);
extern const VIC_TIMING_INFO* _get_vic_timing(unsigned char vic);
extern const VIC_TIMING_INFO* _search_vic_timing(unsigned short h_active, unsigned short v_active, unsigned int v_freq, unsigned short h_total);
extern unsigned char _compute_checksum(unsigned char* p_data, unsigned char len);


extern int  _parse_cea_ext_blk_info(unsigned char* p_edid, CEA_EXT_BLK_INFO* p_info);
extern int  _parse_vsdb_info(CEA_DATA_BLOCK *cea, CEA_HDMI_VSDB_INFO* p_info);
extern int  _parse_dtd_info(CEA_DTD_BLK* p_dtd, CEA_DTD_INFO* p_info);

extern void _dump_cea_ext_blk_info(CEA_EXT_BLK_INFO* p_info);
extern void _dump_vsdb_info(CEA_HDMI_VSDB_INFO* p_info);

extern int  _gen_hdmi_edid(CEA_EXT_BLK_INFO* p_info, unsigned char edid_out[128]);

#endif // __CEA_861_H__
