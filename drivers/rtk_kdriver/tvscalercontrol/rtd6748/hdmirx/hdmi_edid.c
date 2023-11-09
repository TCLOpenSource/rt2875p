/*=============================================================
 * Copyright (c)      Realtek Semiconductor Corporation, 2018
 *
 * All rights reserved.
 *
 *============================================================*/

/*======================= Description ========================== 
 * file: 	hdmi_edid.c 
 * author: 	Kevin_wang
 * date:    2018/3/13
 * version: 1.1
 * This file implements a set of functions of HDMI EDID check and 
 * modification to make sure the EDID can meets system requirement
 * (for Example, no resolutions above 2K ...)  
 *============================================================*/
#include "hdmi_common.h"
#include <linux/kernel.h>
#include <linux/slab.h>
#include "hdmi_edid.h"
#include "hdmi_ddc.h"
#include "hdmi_debug.h"
#include "hdmi_table.h"
#include "cea_861.h"
#include "hdmi_vfe_config.h"

#define HDMI_EDID_LIMIT_2K_TIMING      0x1
#define HDMI_EDID_AUDIO_NO_AC3         (0x1<<8)
#define HDMI_EDID_AUDIO_NO_DTS         (0x1<<9)

#define HDMI_EDID_FORCE_VRR_ENABLE     (0x1<<16)

#define HDMI_EDID_FORCE_MAX_FRL_RATE       (0x1<<17)
#define HDMI_EDID_FORCE_ENABLE_DSC         (0x1<<18)

#define HDMI_EDID_FORCE_ADD_VIC_TIMING     (0x1<<20)
#define HDMI_EDID_FORCE_DEL_VIC_TIMING     (0x1<<21)
#define HDMI_EDID_FORCE_DEL_ALL_VIC_TIMING     (0x1<<23)
static unsigned char HDMI_EDID_FORCE_VDB_SVD = 0;   //user input svd, use to add to or delete from vdb 4 Lane 6Gbps

#define HDMI_EDID_FORCE_AMD_FREE_SYNC_ENABLE     (0x1<<22)
#define HDMI_EDID_FORCE_AMD_FREE_SYNC_MIN_FREQ   48
#define HDMI_EDID_FORCE_AMD_FREE_SYNC_MAX_FREQ   120

#define HDMI_EDID_FORCE_FVA_ENABLE     (0x1<<24)
#define HDMI_EDID_FORCE_QMS_ENABLE     (0x1<<25)

//------------------------------------------------
// The following parameter will be replaced if HDMI_EDID_FORCE_VRR_ENABLE is set
//------------------------------------------------
#define HDMI_EDID_FORCE_MDELTA         (0)              // 0 : no MDelta, 1: has MDelta
#define HDMI_EDID_FORCE_VRR_CINEMA_VRR (1)              // 0 : Minimum Value of VRR (5 bits)
#define HDMI_EDID_FORCE_VRR_CNMVRR     (1)              // 0 : not support negative VRR, 1 : support negative VRR 
#define HDMI_EDID_FORCE_ALLM           (1)              // 0 : not support ALLM, 1 : support ALLM

#define HDMI_EDID_FORCE_VRR_MAX        (120)            // MAX Value of VRR (Hz) (10 bits) 
#define HDMI_EDID_FORCE_VRR_MIN        (20)             // Minimum Value of VRR (Hz) (6 bits)
static unsigned int m_force_edid_vrr_max = HDMI_EDID_FORCE_VRR_MAX;
static unsigned int m_force_edid_vrr_min = HDMI_EDID_FORCE_VRR_MIN;

//------------------------------------------------

const unsigned char vic_group_4k120[] = {118, 120};
const unsigned char vic_group_8k[] = {194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209};


static HDMI_EDID_STATUS_T m_edid_status[HDMI_PORT_TOTAL_NUM];

void _init_edid_capability(HDMI_EDID_CAPABILITY* flag)
{
    *flag = HDMI_EDID_CAPABILITY_UNKNOW;
}

void _set_edid_capability(unsigned char port, HDMI_EDID_CAPABILITY flag)
{
    m_edid_status[port].edid_capability |= flag;
}

unsigned char newbase_hdmi_is_support_edid_capability(unsigned char port, HDMI_EDID_CAPABILITY flag)
{
    return ((m_edid_status[port].edid_capability & flag) == flag);
}


const char* _hdmi_edid_capability_str(HDMI_EDID_CAPABILITY edid_config)
{
    switch(edid_config)
    {
    case HDMI_EDID_CAPABILITY_UNKNOW: return "UNKNOW";
    case HDMI_EDID_CAPABILITY_1_4_14B_VSIF:      return "1_4_14B_VSIF";
    case HDMI_EDID_CAPABILITY_2_0_HF_VSIF:        return "2_0_HF_VSIF";
    case HDMI_EDID_CAPABILITY_SCDC_PRESENT:        return "SCDC_PRESENT";
    case HDMI_EDID_CAPABILITY_2_1_FRL_RATE_3L_3G:     return "2_1_FRL_RATE_3L_3G";
    case HDMI_EDID_CAPABILITY_2_1_FRL_RATE_3L_6G:     return "2_1_FRL_RATE_3L_6G";
    case HDMI_EDID_CAPABILITY_2_1_FRL_RATE_4L_6G:     return "2_1_FRL_RATE_4L_6G";
    case HDMI_EDID_CAPABILITY_2_1_FRL_RATE_4L_8G:     return "2_1_FRL_RATE_4L_8G";
    case HDMI_EDID_CAPABILITY_2_1_FRL_RATE_4L_10G:      return "2_1_FRL_RATE_4L_10G";
    case HDMI_EDID_CAPABILITY_2_1_FRL_RATE_4L_12G:      return "2_1_FRL_RATE_4L_12G";
    case HDMI_EDID_CAPABILITY_VRR:   return "VRR";
    case HDMI_EDID_CAPABILITY_ALLM:       return "ALLM";
    case HDMI_EDID_CAPABILITY_FVA:     return "FVA";
    case HDMI_EDID_CAPABILITY_DSC:     return "DSC";
    case HDMI_EDID_CAPABILITY_DOLBY_VISION:        return "DOLBY_VISION";
    case HDMI_EDID_CAPABILITY_HDR10_PLUS:        return "HDR10_PLUS";
    case HDMI_EDID_CAPABILITY_DUAL_VIEW:     return "DUAL_VIEW";
    case HDMI_EDID_CAPABILITY_3D_OSD_DISPARITY:     return "3D_OSD_DISPARITY";
    case HDMI_EDID_CAPABILITY_SUPPORT_4K120:     return "SUPPORT_4K120";
    case HDMI_EDID_CAPABILITY_SUPPORT_8K:     return "SUPPORT_8K";
    case HDMI_EDID_CAPABILITY_AMD_FREESYNC:     return "AMD_FREESYNC";
    case HDMI_EDID_CAPABILITY_HDR10:     return "HDR10";
    case HDMI_EDID_CAPABILITY_HLG:     return "HLG";
    case HDMI_EDID_CAPABILITY_YUV420:     return "YUV420";
    case HDMI_EDID_CAPABILITY_ADOBERGB:     return "ADOBERGB";
    case HDMI_EDID_CAPABILITY_BT2020:     return "BT2020";
    case HDMI_EDID_CAPABILITY_DCIP3:     return "DCIP3";
    default:                    return "Unkown";
    }
}
const char* get_cea_861_audio_format_code(unsigned char fmt)
{
    switch(fmt)
    {
    case AUDIO_TYPE_REF_HEADER: return "Refer to Stream Header";
    case AUDIO_TYPE_L_PCM:      return "L-PCM";
    case AUDIO_TYPE_AC3:        return "AC3";
    case AUDIO_TYPE_MPEG_1:     return "MPEG-1";
    case AUDIO_TYPE_MP3:        return "MP3";
    case AUDIO_TYPE_MPEG_2:     return "MPEG-2";
    case AUDIO_TYPE_AAC_LC:     return "AAC LC";
    case AUDIO_TYPE_DTS:        return "DTS";
    case AUDIO_TYPE_ATRAC:      return "ATRAC";
    case AUDIO_TYPE_ONE_BIT_AUDIO:   return "One Bit Audio";
    case AUDIO_TYPE_EAC3:       return "Enhanced AC3";
    case AUDIO_TYPE_DTS_HD:     return "DTS-HD";
    case AUDIO_TYPE_MAT:        return "MAT";
    case AUDIO_TYPE_DST:        return "DST";
    case AUDIO_TYPE_WMA_PRO:    return "WMA Pro";
    case AUDIO_TYPE_EXTENSION:  return "Refert to Audio Coding Extension";
    default:                    return "Unkown";
    }
}

/*--------------------------------------------
 * Func : _check_audio_data_block
 *
 * Desc : check if the given audio data block can meet
 *        the specified constrains
 *
 * Para : p_vsdb : (I) Vendor Specific Data Block
 *        flags  : (I) constrain flags
 *
 * Retn : 0 : the Audio DB meets the requirements
 *       -1 : the Audio DB doesn't meet the requirements
 *-------------------------------------------*/
static int _check_audio_data_block(CEA_DATA_BLOCK *p_adb, unsigned long flags)
{
    unsigned char sad_list[30];
    unsigned char sad_len = 0;
    int i;

    if((flags & HDMI_EDID_AUDIO_NO_AC3) || (flags & HDMI_EDID_AUDIO_NO_DTS))
    {
        for (i=0; i<p_adb->length && i<30; i+=3)
        {
            unsigned char* p_sad = &p_adb->data[i];
            unsigned char* n_sad = &sad_list[sad_len];
            unsigned char  valid_sad = 1;

            switch(SAD_AUDIO_FMT(p_sad[0]))
            {
            case AUDIO_TYPE_AC3:
                if ((flags & HDMI_EDID_AUDIO_NO_AC3))
                    valid_sad = 0;
                break;

           case AUDIO_TYPE_DTS:
                if ((flags & HDMI_EDID_AUDIO_NO_DTS))
                    valid_sad = 0;
                break;

            case AUDIO_TYPE_REF_HEADER:
                valid_sad = 0;
                break;

            default:
                break;
            }

            if (valid_sad) {
                memcpy(n_sad, p_sad, 3);
                sad_len+=3;
            }
        }

        if (p_adb->length == sad_len)
        {
            HDMI_EDID_DBG("check_audio_data_block, no sad changed\n");
            return 0;
        }
        else
        {
            HDMI_EDID_INFO("_check_audio_data_block, sad changed (sad_len=%d)\n", sad_len);
            p_adb->length = sad_len;
            memcpy(p_adb->data, sad_list, sad_len);
            return 1;
        }
    }
    else
    {
        return 0;
    }
}


/*--------------------------------------------
 * Func : _check_video_data_block
 *
 * Desc : check if the given video data block can meet
 *        the specified constrains
 *
 * Para : p_vsdb : (I) Vendor Specific Data Block
 *        flags  : (I) constrain flags
 *
 *    HDMI_EDID_LIMIT_2K_TIMING : resoultion
 *            should be less than 2K
 *
 * Retn : 0 : the VSDB meets the requirements
 *       -1 : the VSDB doesn't meet the requirements
 *-------------------------------------------*/
static int _check_video_data_block(CEA_DATA_BLOCK *p_vdb, unsigned long flags, CEA_DATA_BLOCK *p_new_vdb)
{
    if ((flags & HDMI_EDID_LIMIT_2K_TIMING))
    {
        unsigned char svd_list[HDMI_EDID_VDB_MAX_LEN];
        unsigned char svd_cnt = 0;
        int i;
        unsigned char is_updated = FALSE;

        //TODO: remove SVDs that above 2K

        for (i=0; i<p_vdb->length; i++)
        {
            unsigned char svd = p_vdb->data[i];
            unsigned char vic = _svd_to_vic(svd);
            const VIC_TIMING_INFO* p_vic_timing = _get_vic_timing(vic);

            if (p_vic_timing)
            {
                HDMI_EDID_INFO("check_video_data_block, i=%2d, svd=%2d (vic:%3d, hactive=%4d, vactive=%4d)\n",
                    i, svd, vic, p_vic_timing->hActive, p_vic_timing->vActive);

                if (p_vic_timing->hActive <= 2048)
                {
                    HDMI_EDID_DBG("keep svd - %d\n", svd);
                    svd_list[svd_cnt++] = svd;
                }
                else
                {
                    svd_list[svd_cnt++] = 0;
                    is_updated = TRUE;
                    HDMI_EDID_DBG("skip svd - %d, force set 0\n", svd);
                }
            }
            else {
                HDMI_EDID_DBG("skip unknown svd - %d\n", svd);
            }
        }

        if (is_updated == FALSE)
        {
            HDMI_EDID_INFO("check_video_data_block, flag=%lx, no svd changed\n",  flags);
            return 0;
        }
        else
        {
            HDMI_EDID_INFO("_check_video_data_block, flag=%lx, svd changed, Original length=%d, new length=%d\n", flags, p_vdb->length, svd_cnt);
            p_vdb->length = svd_cnt;
            memcpy(p_vdb->data, svd_list, svd_cnt);
            return 1;
        }
    }
    else if ((flags & HDMI_EDID_FORCE_ADD_VIC_TIMING) || (flags & HDMI_EDID_FORCE_DEL_VIC_TIMING) || (flags & HDMI_EDID_FORCE_DEL_ALL_VIC_TIMING))
    {
        int i=0,j=0;
        short svd_list[HDMI_EDID_VDB_MAX_LEN];
        unsigned char svd,vic,is_updated = FALSE;
        const VIC_TIMING_INFO* p_vic_timing = NULL;

        if(p_vdb->length == 0)
        {
            HDMI_EDID_INFO("check_video_data_block, EDID VDB length error!! length =%d\n", p_vdb->length);
            return 0;
        }

        memset(svd_list, -1, sizeof(short)*HDMI_EDID_VDB_MAX_LEN);

        if(flags & HDMI_EDID_FORCE_ADD_VIC_TIMING)
        {
            if(p_vdb->length == 31)
            {
                HDMI_EDID_INFO("check_video_data_block, EDID VDB length =%d , add VIC fail !!\n", p_vdb->length);
                return 0;
            }
            for (i=0; i<p_vdb->length; i++)
            {
                svd = p_vdb->data[i];
                vic = _svd_to_vic(svd);
                p_vic_timing = _get_vic_timing(vic);
                if(p_vic_timing!=NULL)
                {
                    HDMI_EDID_INFO("check_video_data_block, i=%2d, svd=%2d (vic:%3d, hactive=%4d, vactive=%4d)\n",
                        i, svd, vic, p_vic_timing->hActive, p_vic_timing->vActive);
                }
                else
                {
                    HDMI_EDID_INFO("check_video_data_block, i=%2d, svd=%2d (vic:%3d) is not in VIC Table\n",
                        i, svd, vic);
                }

                if(svd != HDMI_EDID_FORCE_VDB_SVD)
                {
                    svd_list[i] = svd;
                }
                else
                {
                    HDMI_EDID_INFO("check_video_data_block, VIC %d  already exist, add VIC fail !!\n",  _svd_to_vic(svd));
                    return 0;
                }
            }
            svd_list[i] = HDMI_EDID_FORCE_VDB_SVD;
            p_vdb->length += 1;
            is_updated = TRUE;
            HDMI_EDID_INFO("ADD_VIC=%d(SVD= %d) success, total length =%d\n",_svd_to_vic(HDMI_EDID_FORCE_VDB_SVD), HDMI_EDID_FORCE_VDB_SVD, p_vdb->length);
        }
        else if(flags & HDMI_EDID_FORCE_DEL_VIC_TIMING)
        {
            if(p_vdb->length == 1)
            {
                HDMI_EDID_INFO("check_video_data_block, EDID VDB length = %d , delete VIC fail !!\n", p_vdb->length);
                return 0;
            }
            for (i=0; i<p_vdb->length; i++)
            {
                svd = p_vdb->data[i];
                vic = _svd_to_vic(svd);
                p_vic_timing = _get_vic_timing(vic);

                if(p_vic_timing!=NULL)
                {
                    HDMI_EDID_INFO("check_video_data_block, i=%2d, svd=%2d (vic:%3d, hactive=%4d, vactive=%4d)\n",
                        i, svd, vic, p_vic_timing->hActive, p_vic_timing->vActive);
                }
                else
                {
                    HDMI_EDID_INFO("check_video_data_block, i=%2d, svd=%2d (vic:%3d) is not in VIC Table\n",
                        i, svd, vic);
                }

                if(svd != HDMI_EDID_FORCE_VDB_SVD)
                {
                    svd_list[i] = svd;
                }
                else
                {
                    is_updated = TRUE;
                    HDMI_EDID_INFO("VDB Index=%d, Delete VIC=%d(SVD=%d) success !!\n", i, _svd_to_vic(HDMI_EDID_FORCE_VDB_SVD),HDMI_EDID_FORCE_VDB_SVD);
                }

            }
        }
        else //HDMI_EDID_FORCE_DEL_ALL_VIC_TIMING
        {
            svd_list[0] = 0;
            is_updated = TRUE;
        }

        if (is_updated == FALSE)
        {
            HDMI_EDID_INFO("_check_video_data_block, flag=%lx, no svd changed\n",  flags);
            return 0;
        }
        else
        {
            if(flags & HDMI_EDID_FORCE_ADD_VIC_TIMING)
            {
                memset(p_new_vdb, 0, sizeof(CEA_DATA_BLOCK));
                memcpy(p_new_vdb, p_vdb, p_vdb->length +1);
                p_vdb = p_new_vdb;
            }
            for (i=0,j=0; i<p_vdb->length; i++)
            {
                if(svd_list[i] != -1)
                {
                    p_vdb->data[j++] = svd_list[i] ;
                }
            }
            p_vdb->length = j;
            return 1;
        }
    }

    return 0;
}


HDMI_EDID_CAPABILITY newbase_hdmi_check_edid_vdb(CEA_DATA_BLOCK *p_vdb, HDMI_EDID_CAPABILITY current_flag)
{
    int i = 0;
    unsigned char table_4k120_len = sizeof(vic_group_4k120)/sizeof(unsigned char);
    unsigned char table_8k_len = sizeof(vic_group_8k)/sizeof(unsigned char);

    for (i=0; i<p_vdb->length; i++)
    {
        unsigned char svd = p_vdb->data[i];
        unsigned char vic = _svd_to_vic(svd);
        unsigned char j = 0;

        //check 4k120
        for(j =0; j < table_4k120_len; j++)
        {
            if (vic == vic_group_4k120[j])
            {
                current_flag |= HDMI_EDID_CAPABILITY_SUPPORT_4K120;
                break;
            }
        }

        //check 4k120
        for(j =0; j < table_8k_len; j++)
        {
            if (vic == vic_group_8k[j])
            {
                current_flag |= HDMI_EDID_CAPABILITY_SUPPORT_8K;
                break;
            }
        }
    }

    return current_flag;
}
/*--------------------------------------------
 * Func : _check_hdmi_vsdb
 *
 * Desc : check if the given HDMI VSDB can meet the specified
 *        constrains
 *
 * Para : p_vsdb : (I) Vendor Specific Data Block
 *        flags  : (I) constrain flags
 *
 *    HDMI_EDID_LIMIT_2K_TIMING : resoultion
 *            should be less than 2K
 *
 * Retn : 0 : the VSDB meets the requirements
 *       -1 : the VSDB doesn't meet the requirements
 *-------------------------------------------*/
static int _check_hdmi_vsdb(CEA_DATA_BLOCK *p_vsdb, unsigned long flags, CEA_DATA_BLOCK *p_new_vsdb)
{
    //HDMI_EDID_INFO("_check_hdmi_vsdb (flags=%08lx)\n", flags);
            
    if ((flags & HDMI_EDID_LIMIT_2K_TIMING))
    {
        CEA_HDMI_VSDB_INFO info;

        memset(p_new_vsdb, 0, sizeof(CEA_DATA_BLOCK));
        memcpy(p_new_vsdb, p_vsdb, p_vsdb->length +1);
        p_vsdb = p_new_vsdb;       // change to new VSDB
 
        //TODO: Remove HDMI_VIC List of the VSDB, beacause all of them are above 2K

        if (_parse_vsdb_info(p_vsdb, &info)<0)
        {
            HDMI_EDID_EMG("check VSDB failed, parse vsdb failed \n");
            return 0; // do not modify it
        }

        //_dump_vsdb_info(&info);

        if (info.p_hdmi_vic)
        {
            unsigned char hdmi_vic_len = info.p_video->hdmi_vic_len;

            HDMI_EDID_INFO("hdmi vic detected (len=%d), remove it for 2K limitation \n",
                hdmi_vic_len);

            if (info.p_hdmi_3d_data)
                memcpy(info.p_hdmi_vic, info.p_hdmi_3d_data, info.p_video->hdmi_3d_len); // copy 3D data

            info.p_hdmi_vic = NULL;
            info.p_video->hdmi_vic_len = 0;
            info.p_hdr->length -= hdmi_vic_len;

            //_dump_vsdb_info(&info);
            return 1;
        }
    }

    return 0;
}


/*--------------------------------------------
 * Func : _check_hf_vsdb
 *
 * Desc : check if the given HF VSDB can meet the specified
 *        constrains
 *
 * Para : p_vsdb : (I) Vendor Specific Data Block
 *        flags  : (I) constrain flags
 *
 *    HDMI_EDID_FORCE_VRR_ENABLE : resoultion
 *            should be less than 2K
 *
 * Retn : 0 : the VSDB meets the requirements
 *        1 : the VSDB doesn't meet the requirements and has been modified
 *-------------------------------------------*/
static int _check_hf_vsdb(CEA_DATA_BLOCK *p_vsdb, unsigned long flags, CEA_DATA_BLOCK *p_new_vsdb)
{
    //HDMI_EDID_INFO("[_check_hf_vsdb] (flags=%08lx)\n", flags);

    if(p_vsdb == NULL)
    {
        HDMI_EDID_EMG("[_check_hf_vsdb] p_vsdb NULL Pointer!!\n");
        return 0;
    }

    if ((flags & (HDMI_EDID_FORCE_VRR_ENABLE | HDMI_EDID_FORCE_MAX_FRL_RATE | HDMI_EDID_FORCE_ENABLE_DSC | HDMI_EDID_FORCE_FVA_ENABLE | HDMI_EDID_FORCE_QMS_ENABLE)))
    {
        memset(p_new_vsdb, 0, sizeof(CEA_DATA_BLOCK));
        memcpy(p_new_vsdb, p_vsdb, p_vsdb->length +1);
        p_vsdb = p_new_vsdb;       // change to new VSDB

        if ((flags & HDMI_EDID_FORCE_MAX_FRL_RATE))
        {
            p_vsdb->data[6] &= 0x0F;
            p_vsdb->data[6] |= (GET_FLOW_CFG(HDMI_FLOW_CFG_GENERAL, HDMI_FLOW_CFG0_MAX_FRL_MODE ))<<4;
        }

        if ((flags & HDMI_EDID_FORCE_VRR_ENABLE))
        {
            if (p_vsdb->length < 10)
                p_vsdb->length = 10;   // expand the length to contain VRR info.

            p_vsdb->data[7] &= ~0x3E;

            if (HDMI_EDID_FORCE_MDELTA)
                p_vsdb->data[7] |= 1<<5; 

            if (HDMI_EDID_FORCE_VRR_CINEMA_VRR)
                p_vsdb->data[7] |= 1<<4; 

            if (HDMI_EDID_FORCE_VRR_CNMVRR)
                p_vsdb->data[7] |= 1<<3; 

            if (HDMI_EDID_FORCE_ALLM)
                p_vsdb->data[7] |= 1<<1; 

            p_vsdb->data[8]  = m_force_edid_vrr_min;
            p_vsdb->data[8] |= ((m_force_edid_vrr_max>>8) & 0x3)<<6;
            p_vsdb->data[9]  = m_force_edid_vrr_max & 0xFF;
        }

        if ((flags & HDMI_EDID_FORCE_FVA_ENABLE))
        {
            p_vsdb->data[7] |= 1<<2;
        }

        if ((flags & HDMI_EDID_FORCE_QMS_ENABLE))
        {
            p_vsdb->data[7] |= 1<<6; // QMS_ENABLE
            p_vsdb->data[7] |= 1<<4; // CINEMA_VRR
        }

        if ((flags & HDMI_EDID_FORCE_ENABLE_DSC))
        {
            if (p_vsdb->length < 13)
                p_vsdb->length = 13;

            p_vsdb->data[10]  = DSC_1P2 | DSC_NATIVE_420(1) | DSC_ALL_BPP(0) | DSC_16BPC(0) | DSC_12BPC(1) | DSC_10BPC(1);
            p_vsdb->data[11]  = DSC_MAX_FRL_RATE(GET_FLOW_CFG(HDMI_FLOW_CFG_GENERAL, HDMI_FLOW_CFG0_MAX_FRL_MODE )) |
                                DSC_MAX_SLICE(3);
            p_vsdb->data[12]  = DSC_TOTAL_CHUNK_BYTES(17);  // 8K DSC_TotalChuckKBytes = 36864/1024-1 = 35 (where 36864 is the max size of DSCD RC fifo), 4K120 should divide 2
        }

        return 1;
    }

    return 0;
}

HDMI_EDID_CAPABILITY newbase_hdmi_check_edid_dolby_vsvdb(CEA_DATA_BLOCK *p_vsdb, HDMI_EDID_CAPABILITY current_flag)
{
    if(p_vsdb == NULL)
    {
        HDMI_EDID_EMG("[newbase_hdmi_check_edid_hf_vsdb] p_vsdb NULL Pointer!!\n");
        return HDMI_EDID_CAPABILITY_UNKNOW;
    }

    if ( (p_vsdb->data[5] & 0xE0) >> 5 == 2) {
        current_flag |= HDMI_EDID_CAPABILITY_DOLBY_VISION_HDMI_21;
    } else {
        current_flag |= HDMI_EDID_CAPABILITY_DOLBY_VISION;
        if ((p_vsdb->data[8] & 3) == 1)
            current_flag |= HDMI_EDID_CAPABILITY_DOLBY_VISION_LOW_LATENCY;
    }
    return current_flag;
}

HDMI_EDID_CAPABILITY newbase_hdmi_check_edid_hdmi_vsdb(CEA_DATA_BLOCK *p_vsdb, HDMI_EDID_CAPABILITY current_flag)
{
    if(p_vsdb == NULL)
    {
        HDMI_EDID_EMG("[newbase_hdmi_check_edid_hf_vsdb] p_vsdb NULL Pointer!!\n");
        return HDMI_EDID_CAPABILITY_UNKNOW;
    }

    if (p_vsdb->data[7] == 0) {
        current_flag |= HDMI_EDID_CAPABILITY_TMDS_3G;
        current_flag |= HDMI_EDID_CAPABILITY_TMDS_6G;
    }
    return current_flag;
}


HDMI_EDID_CAPABILITY newbase_hdmi_check_edid_hf_vsdb(CEA_DATA_BLOCK *p_vsdb, HDMI_EDID_CAPABILITY current_flag)
{
    unsigned char max_frl_rate = 0;
    unsigned char is_allm = 0;
    unsigned char is_fva = 0;
    unsigned char is_scdc = 0;
    unsigned short vrr_max = 0;
    unsigned char is_dual_view = 0;
    unsigned char is_3d_osd_disparity = 0;
    unsigned char dsc_total_chunk = 0;

    if(p_vsdb == NULL)
    {
        HDMI_EDID_EMG("[newbase_hdmi_check_edid_hf_vsdb] p_vsdb NULL Pointer!!\n");
        return HDMI_EDID_CAPABILITY_UNKNOW;
    }

    //--------------------------------
    // HDMI2.1 Max FRL Rate
    //--------------------------------
    max_frl_rate = (p_vsdb->data[6] &0xF0)>>4;
    switch(max_frl_rate)
    {
    case 1:
        current_flag |= HDMI_EDID_CAPABILITY_2_1_FRL_RATE_3L_3G;
        break;
    case 2:
        current_flag |= HDMI_EDID_CAPABILITY_2_1_FRL_RATE_3L_6G;
        break;
    case 3:
        current_flag |= HDMI_EDID_CAPABILITY_2_1_FRL_RATE_4L_6G;
        break;
    case 4:
        current_flag |= HDMI_EDID_CAPABILITY_2_1_FRL_RATE_4L_8G;
        break;
    case 5:
        current_flag |= HDMI_EDID_CAPABILITY_2_1_FRL_RATE_4L_10G;
        break;
    case 6:
        current_flag |= HDMI_EDID_CAPABILITY_2_1_FRL_RATE_4L_12G;
        break;
    case 0:
    default:
        break;
    }

    //--------------------------------
    // ALLM
    //--------------------------------
    is_allm =  (p_vsdb->data[7] &_BIT1)>>1;
    if(is_allm)
        current_flag |= HDMI_EDID_CAPABILITY_ALLM;

    //--------------------------------
    // FVA
    //--------------------------------
    is_fva =  (p_vsdb->data[7] &_BIT2)>>2;
    if(is_fva)
        current_flag |= HDMI_EDID_CAPABILITY_FVA;

    //--------------------------------
    // SCDC_present
    //--------------------------------
    is_scdc = (p_vsdb->data[5] &_BIT7)>>7;
    if(is_scdc)
        current_flag |= HDMI_EDID_CAPABILITY_SCDC_PRESENT;

    //--------------------------------
    // Dual_View
    //--------------------------------
    is_dual_view = (p_vsdb->data[5] &_BIT1)>>1;
    if(is_dual_view)
        current_flag |= HDMI_EDID_CAPABILITY_DUAL_VIEW;

    //--------------------------------
    // 3D OSD Disparity
    //--------------------------------
    is_3d_osd_disparity = (p_vsdb->data[5] &_BIT0);
    if(is_3d_osd_disparity)
        current_flag |= HDMI_EDID_CAPABILITY_3D_OSD_DISPARITY;

    //--------------------------------
    // CINEMA VRR
    //--------------------------------
    if((p_vsdb->length>=9) && (p_vsdb->data[8] & 0x10))
    {
        current_flag |= HDMI_EDID_CAPABILITY_CINEMA_VRR;
    }

    //--------------------------------
    // VRR
    //--------------------------------
    if((p_vsdb->length>=10) && (p_vsdb->data[9]>0))
    {
        vrr_max = ((p_vsdb->data[8]&0xC0)<<2) | p_vsdb->data[9];
        if(vrr_max > 0)
            current_flag |= HDMI_EDID_CAPABILITY_VRR;
    }

    //--------------------------------
    // DSC
    //--------------------------------
    if((p_vsdb->length>=13) && (p_vsdb->data[9]>0))
    {
        dsc_total_chunk = p_vsdb->data[12] & 0x3F;
        if(dsc_total_chunk > 0)
            current_flag |= HDMI_EDID_CAPABILITY_DSC;
    }

    HDMI_EDID_INFO("[EDID] Check EDID HF-VSIF: is_scdc=%d, max_frl_rate=%d, is_allm=%d, is_fva=%d, vrr_max=%d, is_dual_view=%d, is_3d_osd_disparity=%d, dsc_total_chunk=%d\n",
        is_scdc, max_frl_rate, is_allm, is_fva, vrr_max, is_dual_view, is_3d_osd_disparity, dsc_total_chunk);

    return current_flag;
}
/*--------------------------------------------
 * Func : _check_vsdb
 *
 * Desc : check if the given VSDB can meet the specified
 *        constrains
 *
 * Para : p_vsdb : (I) Vendor Specific Data Block
 *        flags  : (I) constrain flags
 *
 *    HDMI_EDID_LIMIT_2K_TIMING : resoultion
 *            should be less than 2K
 *
 * Retn : 0 : the VSDB meets the requirements
 *           1 : the VSDB doesn't meet the requirements
 *-------------------------------------------*/
static int _check_vsdb(unsigned char port, CEA_DATA_BLOCK *p_vsdb, unsigned long flags, CEA_DATA_BLOCK *p_new_vsdb)
{
    int result = 0;
    if (p_vsdb->tag==0x3 && p_vsdb->length >= 3)
    {
        unsigned int oui = (p_vsdb->data[2]<<16) + (p_vsdb->data[1]<<8) + (p_vsdb->data[0]);
        #if 0    //EDID debug using
        HDMI_EDID_INFO("[_check_vsdb port:%d] (flags=%08lx, p_vsdb->tag=%d, p_vsdb->length =%d, OUI=0x%06x)\n", 
            port, flags, p_vsdb->tag, p_vsdb->length, oui);
        #endif 
        switch(oui)
        {
        case IEEE_OUI_HF_VSDB:
            result = _check_hf_vsdb(p_vsdb, flags, p_new_vsdb);
            break;
        case IEEE_OUI_HDMI_VSDB:
            result = _check_hdmi_vsdb(p_vsdb, flags, p_new_vsdb);
            break;
        }
    }

    return result;
}



/*--------------------------------------------
 * Func : _check_dtd
 *
 * Desc : check if the given DTD can meet the specified
 *        constrains
 *
 * Para : p_dtd : (I) DTD Block
 *        flags : (I) constrain flags
 *
 *    HDMI_EDID_LIMIT_2K_TIMING : resoultion
 *            should be less than 2K
 *
 * Retn : 0 : the DTD meets the requirements
 *       -1 : the DTD doesn't meet the requirements
 *-------------------------------------------*/
static int _check_dtd(CEA_DTD_BLK* p_dtd, unsigned long flags)
{
    if ((flags & HDMI_EDID_LIMIT_2K_TIMING))
    {
        CEA_DTD_INFO info;

        _parse_dtd_info(p_dtd, &info);

        if (info.h_active > 2048)
            return -1;
    }

    return 0;
}

/*--------------------------------------------
 * Func : _hdmi_get_amd_vsdb_entry
 *
 * Desc : 
 *
 * Para : p_info  : (I) HDMI info
 *
 * Retn : index  block of AMD VSDB
 *-------------------------------------------*/
int _hdmi_get_amd_vsdb_entry(CEA_EXT_BLK_INFO* p_info)
{
    int i;
    
    for (i=0; i< p_info->n_cdb; i++)
    {
        CEA_DATA_BLOCK* p_cdb = p_info->p_cdb[i];
        
        if (p_cdb->tag==CEA_861_TAG_3_VENDOR_SPECIFIC_BLOCK)
        {
            unsigned int oui = (p_cdb->data[2]<<16) + (p_cdb->data[1]<<8) + (p_cdb->data[0]);

            if (oui == IEEE_OUI_AMD_VSDB)
                return i;
        }
    }
    return -1;         
}

/*--------------------------------------------
 * Func : _hdmi_check_hdmi_edid
 *
 * Desc : check the correctness of hdmi edid.
 *        If there are some limitation are specified
 *        (for Example: limit EDID to 2K resolution)
 *        the FW will modify EDID to meet the constrain
 *        this limitation is specified by kernel config
 *
 *        for example, config CONFIG_REALTEK_2K_MODEL_ENABLED
 *        will eanble the check flag HDMI_EDID_LIMIT_2K_TIMING
 *        to modify EDID to meet 2K limitation
 *
 * Para : edid  : (I) HDMI EDID
 *
 * Retn : 0 : success, -1 : failed
 *-------------------------------------------*/
int _hdmi_check_hdmi_edid(unsigned char port, unsigned char* p_edid,  unsigned long check_flags)
{
    CEA_EXT_BLK_INFO info;
    CEA_DATA_BLOCK   new_amd_vsdb;
    CEA_DATA_BLOCK   new_ceadb;
    int i=0;
    int edid_modified = 0;

    HDMI_EDID_DBG("[_hdmi_check_hdmi_edid port: %d], (%p)\n", port, p_edid);

    if (_compute_checksum(p_edid, 128)!=0)
    {
        HDMI_EDID_EMG("[_hdmi_check_hdmi_edid port: %d] failed, checksum failed\n", port);
        return -1;
    }

    if (_parse_cea_ext_blk_info(p_edid, &info)<0)
    {
        HDMI_EDID_EMG("[_hdmi_check_hdmi_edid port: %d] failed, parse hdmi edid failed\n", port);
        return -1;
    }

    //_dump_cea_ext_blk_info(&info);

    // check CDB
    for (i=0; i< info.n_cdb; i++)
    {
        //HDMI_EDID_INFO("check CDB[%d/%d], tag=%d\n", i, info.n_cdb, info.p_cdb[i]->tag);

        switch(info.p_cdb[i]->tag)
        {
        case CEA_861_TAG_1_AUDIO_DATA_BLOCK:
            if (_check_audio_data_block(info.p_cdb[i], check_flags))
            {
                edid_modified++;
            }
            break;

        case CEA_861_TAG_2_VIDEO_DATA_BLOCK:
            if (_check_video_data_block(info.p_cdb[i], check_flags, &new_ceadb))
            {
                if(check_flags & HDMI_EDID_FORCE_ADD_VIC_TIMING)
                {
                    info.p_cdb[i] = &new_ceadb;
                }
                edid_modified++;
            }
            break;

        case CEA_861_TAG_3_VENDOR_SPECIFIC_BLOCK:
            if (_check_vsdb(port, info.p_cdb[i], check_flags, &new_ceadb)) // for all VSDB, including HDMI VSDB/HF VSDB ...
            {
                info.p_cdb[i] = &new_ceadb;  // modified, use new vsdb to instead
                HDMI_EDID_EMG("case VSDB : info.p_cdb[%d]->length = %d",i,info.p_cdb[i]->length);
                edid_modified++;
            }
            break;

        default:
            break;
        }
    }

    if ((check_flags & HDMI_EDID_FORCE_AMD_FREE_SYNC_ENABLE))
    {
        int cdb_id = _hdmi_get_amd_vsdb_entry(&info);

        // init AMD VSDB : here we use AMD VSDB V1
        new_amd_vsdb.tag     = CEA_861_TAG_3_VENDOR_SPECIFIC_BLOCK;
        new_amd_vsdb.length  = 8;
        new_amd_vsdb.data[0] = (IEEE_OUI_AMD_VSDB) & 0xFF;              // OUI P0 (0x1A)
        new_amd_vsdb.data[1] = (IEEE_OUI_AMD_VSDB>>8) & 0xFF;           // OUI P1 (0x00)
        new_amd_vsdb.data[2] = (IEEE_OUI_AMD_VSDB>>16) & 0xFF;          // OUI P2 (0x00)
        new_amd_vsdb.data[3] = 0x1;                                     // PB0 : vsersion
        new_amd_vsdb.data[4] = 0x1;                                     // PB1 : free sync support
        new_amd_vsdb.data[5] = HDMI_EDID_FORCE_AMD_FREE_SYNC_MIN_FREQ;  // PB2 : min refresh rate
        new_amd_vsdb.data[6] = HDMI_EDID_FORCE_AMD_FREE_SYNC_MAX_FREQ;  // PB3 : max refresh rate
        new_amd_vsdb.data[7] = 0x00;                                    // PB4 : MCCS VCP Code

        // find vsdb
        if (cdb_id<0 && info.n_cdb < 32)   // AMD VSDB absent
            cdb_id = info.n_cdb++;

        // modify vsdb
        if (cdb_id>=0)
        {
            info.p_cdb[cdb_id] = &new_amd_vsdb;  // attached AMD VSDB to the end of cdb
            edid_modified++;
            HDMI_EDID_EMG("[_hdmi_check_hdmi_edid port: %d] insert AMD VSDB success - block id=%d, min_freq=%d, max_freq=%d\n",
                port, cdb_id, HDMI_EDID_FORCE_AMD_FREE_SYNC_MIN_FREQ, HDMI_EDID_FORCE_AMD_FREE_SYNC_MAX_FREQ);
        }
        else
        {
            HDMI_EDID_EMG("[_hdmi_check_hdmi_edid port: %d] insert AMD VSDB failed - out of CEA data block\n", port);
        }
    }

    // check DTD
    for (i=0; i< info.hdr.n_dtd; i++)
    {
        HDMI_EDID_DBG("check DTD[%d/%d]\n", i, info.hdr.n_dtd);

        if (_check_dtd(info.p_dtd[i], check_flags)<0)
        {
            info.p_dtd[i] = NULL;
            edid_modified++;
        }
    }

    if (edid_modified) {
        HDMI_EDID_INFO("[_hdmi_check_hdmi_edid port:%d], hdmi edid(%p) has been modified\n", port, p_edid);
        _gen_hdmi_edid(&info, p_edid);
    }

    return 0;
}

/*--------------------------------------------
 * Func : newbase_hdmi_check_edid_capability
 *
 * Desc : check the correctness of hdmi edid.
 *        If there are some limitation are specified
 *
 * Para : edid  : (I) HDMI EDID, len: edid length
 *
 * Retn : HDMI_EDID_CAPABILITY edid capability
 *-------------------------------------------*/
HDMI_EDID_CAPABILITY newbase_hdmi_check_edid_capability(unsigned char* raw_edid, unsigned int len)
{
    CEA_EXT_BLK_INFO info;
    int i;
    unsigned char* edid;
    HDMI_EDID_CAPABILITY current_flag = HDMI_EDID_CAPABILITY_UNKNOW;

    HDMI_EDID_DBG("[newbase_hdmi_check_edid_capability], (%p)\n", raw_edid);
    if(len < 256)
    {
        HDMI_EDID_EMG("[newbase_hdmi_check_edid_capability] It is non-HDMI EDID, len=%d\n", len);
        return HDMI_EDID_CAPABILITY_UNKNOW;
    }

    edid = raw_edid + 128;

    if (_compute_checksum(edid, len)!=0)
    {
        HDMI_EDID_EMG("[newbase_hdmi_check_edid_capability] failed, checksum failed\n");
        return HDMI_EDID_CAPABILITY_UNKNOW;
    }

    if (_parse_cea_ext_blk_info(edid, &info)<0)
    {
        HDMI_EDID_EMG("[newbase_hdmi_check_edid_capability] failed, parse hdmi edid failed\n");
        return HDMI_EDID_CAPABILITY_UNKNOW;
    }


    // check CDB
    for (i=0; i< info.n_cdb; i++)
    {
        //HDMI_EDID_INFO("[newbase_hdmi_check_edid_capability] check CDB[%d/%d], tag=%d, length=%d\n", i, info.n_cdb, info.p_cdb[i]->tag, info.p_cdb[i]->length);

        switch(info.p_cdb[i]->tag)
        {            
        case CEA_861_TAG_2_VIDEO_DATA_BLOCK:
            current_flag |= newbase_hdmi_check_edid_vdb(info.p_cdb[i], current_flag);
            break;

        case CEA_861_TAG_3_VENDOR_SPECIFIC_BLOCK:
            if (info.p_cdb[i]->length >= 3)
            {
                unsigned int oui = (info.p_cdb[i]->data[2]<<16) + (info.p_cdb[i]->data[1]<<8) + (info.p_cdb[i]->data[0]);

                HDMI_EDID_INFO("Check CDB[%d/%d] (tag=%d, length =%d, OUI=0x%06x)\n", 
                     i, (info.n_cdb-1), info.p_cdb[i]->tag, info.p_cdb[i]->length, oui);

                switch(oui)
                {
                case IEEE_OUI_HF_VSDB:
                    HDMI_EDID_INFO("SUPPORT HF_VSIF\n");
                    current_flag |= HDMI_EDID_CAPABILITY_2_0_HF_VSIF;
                    current_flag |= newbase_hdmi_check_edid_hf_vsdb(info.p_cdb[i], current_flag);
                    break;
                case IEEE_OUI_HDMI_VSDB:
                    HDMI_EDID_INFO("SUPPORT H14B_VSIF\n");
                    current_flag |= HDMI_EDID_CAPABILITY_1_4_14B_VSIF;
                    current_flag |= newbase_hdmi_check_edid_hdmi_vsdb(info.p_cdb[i], current_flag);
                    break;
                case IEEE_OUI_AMD_VSDB:
                    HDMI_EDID_INFO("SUPPORT AMD Freesync\n");
                    current_flag |= HDMI_EDID_CAPABILITY_AMD_FREESYNC;
                    break;
                default:
                    break;
                }
            }
            break;

        case CEA_861_EXT_TAG_7_USE_EXTENDED_BLOCK:
            if (info.p_cdb[i]->length >= 3)
            {
                unsigned char extension_tag = info.p_cdb[i]->data[0];
                #if 0 //for edid content debug
                HDMI_EDID_INFO("Check USE_EXTENDED_BLOCK[%d/%d] (tag=%d, extension_tag=%d, length =%d)\n", 
                    i, (info.n_cdb-1), info.p_cdb[i]->tag, extension_tag, info.p_cdb[i]->length);
                #endif

                switch(extension_tag)
                {
                case CEA_861_TAG07_VENDOR_SPECIFIC_VIDEO:
                {
                    unsigned int oui = (info.p_cdb[i]->data[3]<<16) + (info.p_cdb[i]->data[2]<<8) + (info.p_cdb[i]->data[1]);

                    HDMI_EDID_INFO("Check CDB[%d/%d] (tag=%d, extension_tag=%d, length =%d, OUI=0x%06x)\n", 
                        i, (info.n_cdb-1), info.p_cdb[i]->tag, extension_tag, info.p_cdb[i]->length, oui);

                    switch(oui)
                    {
                    case IEEE_OUI_DOLBY_VSIF:
                        HDMI_EDID_INFO("SUPPORT DOLBY_VSIF\n");
                        current_flag |= HDMI_EDID_CAPABILITY_DOLBY_VISION;
                        break;
                    case IEEE_OUI_HDR10_PLUS:
                        HDMI_EDID_INFO("SUPPORT HDR10_PLUS\n");
                        current_flag |= HDMI_EDID_CAPABILITY_HDR10_PLUS;
                        break;
                    default:
                        break;
                    }
                }
                    break;
                case CEA_861_TAG07_COLORIMETRY:
                {
                    unsigned char supported_colorimetry_db3 = (info.p_cdb[i]->data[1]);
                    unsigned char supported_colorimetry_db4= (info.p_cdb[i]->data[2]);
                    unsigned char adobergb_mask = _BIT4 | _BIT3;
                    unsigned char bt2020_mask = _BIT7| _BIT6 | _BIT5;
                    unsigned char dcip3_mask = _BIT7;

                    if((supported_colorimetry_db3 & adobergb_mask))
                    {
                        HDMI_EDID_INFO("SUPPORT ADOBERGB, db3:0x%x, db4:0x%x\n", supported_colorimetry_db3, supported_colorimetry_db4);
                        current_flag |= HDMI_EDID_CAPABILITY_ADOBERGB;
                    }

                    if((supported_colorimetry_db3 & bt2020_mask))
                    {
                        HDMI_EDID_INFO("SUPPORT BT2020, db3:0x%x, db4:0x%x\n", supported_colorimetry_db3, supported_colorimetry_db4);
                        current_flag |= HDMI_EDID_CAPABILITY_BT2020;
                    }

                    if((supported_colorimetry_db4 & dcip3_mask))
                    {
                         HDMI_EDID_INFO("SUPPORT DCI-P3, db3:0x%x, db4:0x%x\n", supported_colorimetry_db3, supported_colorimetry_db4);
                        current_flag |= HDMI_EDID_CAPABILITY_DCIP3;
                    }
                    break;
                }
                case CEA_861_TAG07_HDR_STATIC_METADATA:
                {
                    unsigned char supported_eotf = ((info.p_cdb[i]->data[1]) & 0x3F);
                    if((supported_eotf & CEA_861_EOTF_SMPTE_ST_2084) == CEA_861_EOTF_SMPTE_ST_2084)
                    {
                        HDMI_EDID_INFO("SUPPORT HDR10, supported_eotf:0x%x\n", supported_eotf);
                        current_flag |= HDMI_EDID_CAPABILITY_HDR10;
                    }
                    if((supported_eotf & CEA_861_EOTF_HLG) == CEA_861_EOTF_HLG)
                    {
                        HDMI_EDID_INFO("SUPPORT HLG, supported_eotf:0x%x\n", supported_eotf);
                        current_flag |= HDMI_EDID_CAPABILITY_HLG;
                    }
                    break;
                }
                case CEA_861_TAG07_YUV420_VIDEO:
                {
                    unsigned char vic_table_length = (info.p_cdb[i]->length>=1) ? (info.p_cdb[i]->length-1) : 0;
                    if((vic_table_length>0) && (vic_table_length<31))
                    {
                        unsigned char j =0;
                        HDMI_EDID_INFO("SUPPORT YUV420: ");

                        for(j = 0; j<vic_table_length; j++)
                        {
                            HDMI_EDID_INFO("[%2d] VIC: %d,", j, _svd_to_vic(info.p_cdb[i]->data[j+1]));
                        }
                        current_flag |= HDMI_EDID_CAPABILITY_YUV420;
                    }
                    break;
                }
                }
            }
            break;

        case CEA_861_TAG_1_AUDIO_DATA_BLOCK:
        default:
            #if 0 //debugging use
            if (info.p_cdb[i]->length >= 3)
            {
                unsigned int oui = (info.p_cdb[i]->data[2]<<16) + (info.p_cdb[i]->data[1]<<8) + (info.p_cdb[i]->data[0]);

                HDMI_EDID_INFO("Check CDB[%d/%d] (tag=%d, length =%d, OUI=0x%06x)\n", 
                     i, (info.n_cdb-1), info.p_cdb[i]->tag, info.p_cdb[i]->length, oui);
            }
            #endif
            break;
        }
    }

    // check DTD
    #if 0
    for (i=0; i< info.hdr.n_dtd; i++)
    {
        HDMI_EDID_DBG("check DTD[%d/%d]\n", i, info.hdr.n_dtd);

        if (_check_dtd(info.p_dtd[i], check_flags)<0)
        {
            info.p_dtd[i] = NULL;
        }
    }
    #endif
    return current_flag;
}
/*--------------------------------------------
 * Func : newbase_hdmi_check_hdmi_edid
 *
 * Desc : check the correctness of hdmi edid.
 *        If there are some limitation are specified
 *        (for Example: limit EDID to 2K resolution)
 *        the FW will modify EDID to meet the constrain
 *        this limitation is specified by kernel config
 *
 *        for example, config CONFIG_REALTEK_2K_MODEL_ENABLED
 *        will eanble the check flag HDMI_EDID_LIMIT_2K_TIMING
 *        to modify EDID to meet 2K limitation
 *
 * Para : edid  : (I) HDMI EDID
 *
 * Retn : 0 : success, -1 : failed
 *-------------------------------------------*/
int newbase_hdmi_check_hdmi_edid(unsigned char port, unsigned char* p_edid)
{
    unsigned long check_flags = 0;

#if 0 //For 2k model, modify EDID.
    check_flags |= HDMI_EDID_LIMIT_2K_TIMING;
#endif
    //check_flags |= HDMI_EDID_FORCE_VRR_ENABLE;   // modify edid to support VRR (for testing only)
    //check_flags |= HDMI_EDID_AUDIO_NO_AC3;     // disable AC3 support
    //check_flags |= HDMI_EDID_FORCE_MAX_FRL_RATE;
    //check_flags |= HDMI_EDID_FORCE_ENABLE_DSC;   //Default is enabel HDMI2.1 DSC flag
    //check_flags |= HDMI_EDID_FORCE_AMD_FREE_SYNC_ENABLE;   //Default is enabel AMD Free Sync

    return _hdmi_check_hdmi_edid(port, p_edid,  check_flags);
}

int newbase_hdmi_is_edid_initialized(unsigned char port)
{
    return m_edid_status[port].edid_initialized;
}

void newbase_hdmi_set_edid_initialized(unsigned char port, unsigned char set_value)
{
    m_edid_status[port].edid_initialized = set_value;
}

unsigned char newbase_hdmi_save_edid(unsigned char port, const unsigned char *edid, const unsigned int len)
{// 1: save success, 0: save fail 
    unsigned char* p_edid_table = NULL;
    unsigned int i = 0;
    unsigned char is_need_save = FALSE;

    if (port >=HDMI_PORT_TOTAL_NUM) {
        HDMI_EDID_EMG("[newbase_hdmi_save_edid] failed, port %d doesn't exist\n" , port);
        return 0;
    }
    if(len>MAX_EDID_LENGTH)
    {
        HDMI_EDID_EMG("[newbase_hdmi_save_edid port %d] INVALID length=%d,  doesn't exist\n", port, len);
        return 0;
    }
    
    p_edid_table = m_edid_status[port].current_edid_table;
    //compare EDID
    for(i = 0; i<len; i++)
    {
        if(p_edid_table[i] == edid[i])
        {
            is_need_save = FALSE;
        }
        else
        {
            is_need_save = TRUE;
            break;
        }
    }

    if(is_need_save == FALSE)
    {
        HDMI_EDID_INFO("Save no need, The same EDID already exist, port=%d, Checksum=(%02x,%02x), len=%d\n",
            port, p_edid_table[127], p_edid_table[255], len);
    }
    else
    {
        memcpy(p_edid_table, edid, len);

        newbase_hdmi_set_edid_initialized(port, TRUE);

        HDMI_EDID_INFO("Save Customer EDID, port=%d, Checksum=(%02x,%02x), len=%d\n",
            port, p_edid_table[127], p_edid_table[255], len);
    }

    return is_need_save;
}

/*--------------------------------------------
 * Func : newbase_hdmi_read_edid
 *
 * Desc : Read EDID from DDC.
 *
 * Para : edid  : (I) HDMI EDID
 *
 * Retn : TRUE : success, FALSE : failed
 *-------------------------------------------*/
unsigned char newbase_hdmi_read_edid(unsigned char port, unsigned char *edid, unsigned int len)
{
    unsigned char result = FALSE;
    if (port >= HDMI_PORT_TOTAL_NUM)
    {
        HDMI_EDID_EMG("%s: err port=%d\n", __func__, port);
        result = FALSE;
    }
    else
    {
        unsigned char ddc_ch = 0;

        if (newbase_hdmi_ddc_channel_status(port,&ddc_ch) != 0 )
            return FALSE;

        result = lib_hdmiddc_read(ddc_ch, edid, len);

        HDMI_EDID_INFO("Read EDID port=%d, result=%d, Checksum=(%02x,%02x), len=%d, ddc_ch=%d\n",
            port, 
            result,
            edid[127], edid[255], len, ddc_ch);
    }

    return result;
}

/*--------------------------------------------
 * Func : newbase_hdmi_write_edid
 *
 * Desc : write EDID from DDC.
 *
 * Para : edid  : (I) HDMI EDID
 *
 * Retn : TRUE : success, FALSE : failed
 *-------------------------------------------*/
unsigned char newbase_hdmi_write_edid(unsigned char port, unsigned char *edid, unsigned int len)
{
    unsigned char result = FALSE;

    if (port >= HDMI_PORT_TOTAL_NUM)
    {
        HDMI_EDID_EMG("%s: err port=%d\n", __func__, port);
        result = FALSE;
    }
    else
    {
        unsigned char ddc_ch = 0;

        if (newbase_hdmi_ddc_channel_status(port,&ddc_ch) != 0 )
            return FALSE;

        result = lib_hdmiddc_write(ddc_ch, edid, len);

        m_edid_status[port].edid_capability = newbase_hdmi_check_edid_capability(edid, len);
        HDMI_EDID_INFO("Write EDID port=%d, result=%d, Checksum=(%02x,%02x),len=%d, ddc_ch=%d, capability=0x%x\n",
            port, 
            result,
            edid[127],
            edid[255], len, ddc_ch, m_edid_status[port].edid_capability);
    }

    return result;
}

void newbase_hdmi_edid_enable(unsigned char port, unsigned char enable)
{
    unsigned char ddc_ch = 0;

    if (port >= HDMI_PORT_TOTAL_NUM || newbase_hdmi_ddc_channel_status(port,&ddc_ch) != 0) {
        HDMI_EDID_EMG("[EDID] %s: err port=%d\n", __func__, port);
        return;
    }
    else
    {
        lib_hdmiddc_enable(ddc_ch, enable);
        HDMI_EDID_INFO("Enable HDMI[p%d] DDC-%d EDID Access\n", port, ddc_ch);
    }
}

void newbase_hdmi_edid_init(void)
{
    unsigned char j;
    HDMI_EDID_INFO("[newbase_hdmi_edid_init] Start !!\n");

    for (j=0; j<HDMI_PORT_TOTAL_NUM; j++)
    {
        unsigned char* p_edid_table = NULL;
        HDMI_CHANNEL_T* p_ch = newbase_hdmi_get_pcbinfo(j);

        if (p_ch == NULL || p_ch->phy_port == HDMI_PHY_PORT_NOTUSED)
        {
            HDMI_EDID_INFO("%s: init port %d failed - port not used, skip it\n", __func__, j);
            continue;
        }
        p_edid_table = m_edid_status[j].current_edid_table;

        HDMI_EDID_INFO("INIT EDID Start !! Port=%d, Checksum=(%02x,%02x), capability=0x%x\n",
            j, p_edid_table[127], p_edid_table[255], m_edid_status[j].edid_capability);

        if(!newbase_hdmi_write_edid(j, p_edid_table, 256))
        {
            HDMI_EDID_EMG("%s: init edid %d failed!\n", __func__, j);
        }
    }

    HDMI_EDID_INFO("[newbase_hdmi_edid_init] End !!\n");
}

/*--------------------------------------------
 * Func : newbase_hdmi_edid_set_max_frl_rate
 *
 * Desc : hack max frl rate to hf-vsdb of edid.
 *
 * Para :port
 *
 * Retn : TRUE : success, FALSE : failed
 *-------------------------------------------*/
unsigned char newbase_hdmi_edid_set_max_frl_rate(
    unsigned char               port, 
    unsigned char               max_frl_rate
    )
{
    unsigned char edid[256];

    newbase_hdmi_read_edid(port, edid, sizeof(edid));

    SET_FLOW_CFG(HDMI_FLOW_CFG_GENERAL, HDMI_FLOW_CFG0_MAX_FRL_MODE, max_frl_rate );

    _hdmi_check_hdmi_edid(port, &edid[128], HDMI_EDID_FORCE_MAX_FRL_RATE);

    newbase_hdmi_write_edid(port, edid, sizeof(edid)); 

    return 0;
}

/*--------------------------------------------
 * Func : newbase_hdmi_edid_set_dsc_frl_rate
 *
 * Desc : set dsc block and hack max frl rate to hf-vsdb of edid.
 *
 * Para :port
 *
 * Retn : TRUE : success, FALSE : failed
 *-------------------------------------------*/
unsigned char newbase_hdmi_edid_set_dsc_frl_rate(
    unsigned char               port, 
    unsigned char               max_frl_rate
    )
{
    unsigned char edid[256];

    newbase_hdmi_read_edid(port, edid, sizeof(edid));

    SET_FLOW_CFG(HDMI_FLOW_CFG_GENERAL, HDMI_FLOW_CFG0_MAX_FRL_MODE, max_frl_rate );

    _hdmi_check_hdmi_edid(port, &edid[128], HDMI_EDID_FORCE_ENABLE_DSC);

    newbase_hdmi_write_edid(port, edid, sizeof(edid)); 

    return 0;
}
/*--------------------------------------------
 * Func : newbase_hdmi_edid_set_amd_freesync_vsdb
 *
 * Desc :  hack amd_freesync vsdb to edid.
 *
 * Para : port
 *
 * Retn : TRUE : success, FALSE : failed
 *-------------------------------------------*/
unsigned char newbase_hdmi_edid_set_amd_freesync_vsdb(unsigned char port)
{
    unsigned char edid[256];

    newbase_hdmi_read_edid(port, edid, sizeof(edid));

    _hdmi_check_hdmi_edid(port, &edid[128], HDMI_EDID_FORCE_AMD_FREE_SYNC_ENABLE);

    newbase_hdmi_write_edid(port, edid, sizeof(edid)); 

    return 0;
}

/*--------------------------------------------
 * Func : newbase_hdmi_edid_set_amd_freesync_vsdb
 *
 * Desc :  hack amd_freesync vsdb to edid.
 *
 * Para : port
 *
 * Retn : TRUE : success, FALSE : failed
 *-------------------------------------------*/
unsigned char newbase_hdmi_edid_set_vrr_min_max(unsigned char port, unsigned int vrr_min, unsigned int vrr_max)
{
    unsigned char edid[256];

    newbase_hdmi_read_edid(port, edid, sizeof(edid));

    m_force_edid_vrr_max = vrr_max;
    m_force_edid_vrr_min = vrr_min;
    _hdmi_check_hdmi_edid(port, &edid[128], HDMI_EDID_FORCE_VRR_ENABLE);

    newbase_hdmi_write_edid(port, edid, sizeof(edid)); 

    return 0;
}

/*--------------------------------------------
 * Func : newbase_hdmi_edid_set_fva_vsdb
 *
 * Desc :  hack fva vsdb to edid.
 *
 * Para : port
 *
 * Retn : TRUE : success, FALSE : failed
 *-------------------------------------------*/
unsigned char newbase_hdmi_edid_set_fva_vsdb(unsigned char port)
{
    unsigned char edid[256];

    newbase_hdmi_read_edid(port, edid, sizeof(edid));

    _hdmi_check_hdmi_edid(port, &edid[128], HDMI_EDID_FORCE_FVA_ENABLE);

    newbase_hdmi_write_edid(port, edid, sizeof(edid));

    return 0;
}

/*--------------------------------------------
 * Func : newbase_hdmi_edid_set_qms_vsdb
 *
 * Desc :  hack qms vsdb to edid.
 *
 * Para : port
 *
 * Retn : TRUE : success, FALSE : failed
 *-------------------------------------------*/
unsigned char newbase_hdmi_edid_set_qms_vsdb(unsigned char port)
{
    unsigned char edid[256];

    newbase_hdmi_read_edid(port, edid, sizeof(edid));

    _hdmi_check_hdmi_edid(port, &edid[128], HDMI_EDID_FORCE_QMS_ENABLE);

    newbase_hdmi_write_edid(port, edid, sizeof(edid));

    return 0;
}

/*--------------------------------------------
 * Func : newbase_hdmi_edid_add_vic_to_vdb
 *
 * Desc : Add VIC to  EDID VDB
 *
 * Para : port, vic
 *
 * Retn : TRUE : success, FALSE : failed
 *-------------------------------------------*/
unsigned char newbase_hdmi_edid_add_vic_to_vdb(
    unsigned char               port, 
    unsigned char               vic
    )
{
    unsigned char edid[256];

    if((vic>=128)&&(vic<=192))
    {
        HDMI_EDID_EMG("Add illegal VIC %d, add VIC fail !!",vic);
        return 0;
    }

    HDMI_EDID_FORCE_VDB_SVD = _vic_to_svd(vic,VIC_NOT_NATIVE);
    HDMI_EDID_EMG("enter add vic = %d(svd = %d)",vic,HDMI_EDID_FORCE_VDB_SVD);
    newbase_hdmi_read_edid(port, edid, sizeof(edid));

    _hdmi_check_hdmi_edid(port, &edid[128], HDMI_EDID_FORCE_ADD_VIC_TIMING);

    newbase_hdmi_write_edid(port, edid, sizeof(edid)); 

    return 0;
}

/*--------------------------------------------
 * Func : newbase_hdmi_edid_add_vic_native_to_vdb
 *
 * Desc : Add VIC to  EDID VDB
 *
 * Para : port, vic
 *
 * Retn : TRUE : success, FALSE : failed
 *-------------------------------------------*/
unsigned char newbase_hdmi_edid_add_vic_native_to_vdb(
    unsigned char               port,
    unsigned char               vic
    )
{
    unsigned char edid[256];

    if((vic>64)||(vic<1))
    {
        HDMI_EDID_EMG("Add illegal native VIC %d, add native VIC fail !!",vic);
        return 0;
    }

    HDMI_EDID_FORCE_VDB_SVD = _vic_to_svd(vic,VIC_NATIVE);
    HDMI_EDID_EMG("enter add vic native = %d(svd = %d)",vic,HDMI_EDID_FORCE_VDB_SVD);
    newbase_hdmi_read_edid(port, edid, sizeof(edid));

    _hdmi_check_hdmi_edid(port, &edid[128], HDMI_EDID_FORCE_ADD_VIC_TIMING);

    newbase_hdmi_write_edid(port, edid, sizeof(edid));

    return 0;
}

/*--------------------------------------------
 * Func : newbase_hdmi_edid_del_vic_native_to_vdb
 *
 * Desc : Add VIC to  EDID VDB
 *
 * Para : port, vic
 *
 * Retn : TRUE : success, FALSE : failed
 *-------------------------------------------*/
unsigned char newbase_hdmi_edid_del_vic_native_to_vdb(
    unsigned char               port,
    unsigned char               vic
    )
{
    unsigned char edid[256];

    if((vic>64)||(vic<1))
    {
        HDMI_EDID_EMG("Delete illegal native VIC %d, delete native VIC fail !!",vic);
        return 0;
    }

    HDMI_EDID_FORCE_VDB_SVD = _vic_to_svd(vic,VIC_NATIVE);
    HDMI_EDID_EMG("enter delete vic native = %d(svd = %d)",vic,HDMI_EDID_FORCE_VDB_SVD);
    newbase_hdmi_read_edid(port, edid, sizeof(edid));

    _hdmi_check_hdmi_edid(port, &edid[128], HDMI_EDID_FORCE_DEL_VIC_TIMING);

    newbase_hdmi_write_edid(port, edid, sizeof(edid));

    return 0;
}

/*--------------------------------------------
 * Func : newbase_hdmi_edid_del_vic_to_vdb
 *
 * Desc : Add VIC to  EDID VDB
 *
 * Para : port, vic
 *
 * Retn : TRUE : success, FALSE : failed
 *-------------------------------------------*/
unsigned char newbase_hdmi_edid_del_vic_to_vdb(
    unsigned char               port, 
    unsigned char               vic
    )
{
    unsigned char edid[256];

    if((vic>=128)&&(vic<=192))
    {
        HDMI_EDID_EMG("Delete illegal VIC %d, delete VIC fail !!",vic);
        return 0;
    }

    HDMI_EDID_FORCE_VDB_SVD = _vic_to_svd(vic,VIC_NOT_NATIVE);
    HDMI_EDID_EMG("enter delete vic = %d(svd = %d)",vic,HDMI_EDID_FORCE_VDB_SVD);
    newbase_hdmi_read_edid(port, edid, sizeof(edid));

    _hdmi_check_hdmi_edid(port, &edid[128], HDMI_EDID_FORCE_DEL_VIC_TIMING);

    newbase_hdmi_write_edid(port, edid, sizeof(edid));

    return 0;
}

/*--------------------------------------------
 * Func : newbase_hdmi_edid_cle_all_vic_of_vdb
 *
 * Desc : Add VIC to  EDID VDB
 *
 * Para : port, vic
 *
 * Retn : TRUE : success, FALSE : failed
 *-------------------------------------------*/
unsigned char newbase_hdmi_edid_del_all_vic_to_vdb(
    unsigned char               port
    )
{
    unsigned char edid[256];

    newbase_hdmi_read_edid(port, edid, sizeof(edid));

    _hdmi_check_hdmi_edid(port, &edid[128], HDMI_EDID_FORCE_DEL_ALL_VIC_TIMING);

    newbase_hdmi_write_edid(port, edid, sizeof(edid)); 

    return 0;
}

HDMI_EDID_STATUS_T* newbase_hdmi_edid_get_status(unsigned char port)
{
	return (port < HDMI_PORT_TOTAL_NUM) ? &m_edid_status[port] : NULL;
}

