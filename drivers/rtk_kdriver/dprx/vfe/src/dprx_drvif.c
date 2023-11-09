/*=============================================================
 * Copyright (c)      Realtek Semiconductor Corporation, 2016
 *
 * All rights reserved.
 *
 *============================================================*/

/*======================= Description ============================
 * file:    dprx_drvif.c
 * author:  Edwiin_Lai
 * date:    2022/07/04
 * version: 1.0
 *============================================================*/
#include <dprx_vfe.h>
#include "rtk_kdriver/dprx/dprxfun.h"
#include <ioctrl/scaler/vfe_cmd_id.h>
/**********************************************************************************************
*
*   Generic API
*
**********************************************************************************************/


/*------------------------------------------------------
 * Func : drvif_Dprx_GetRawTimingInfo
 *
 * Desc : get current DP raw timing info of current DP port
 *
 * Parm : p_timing_info: pointer of DPRX_TIMING_INFO_T variable from caller
 *
 * Retn : current DP port timing info
 *------------------------------------------------------*/
DPRX_bool drvif_Dprx_GetRawTimingInfo(DPRX_TIMING_INFO_T* p_timing_info)
{
    if ((p_timing_info==NULL)||(g_connected_dprx==NULL))
        return 0;

    memset(p_timing_info, 0, sizeof(DPRX_TIMING_INFO_T));

    if (dprx_adp_get_video_timing(g_connected_dprx, p_timing_info)<0)
        return DPRX_DRV_HW_ERR;

    return 1;
}
EXPORT_SYMBOL(drvif_Dprx_GetRawTimingInfo);


/*------------------------------------------------------
 * Func : drvif_Dprx_GetColorSpace
 *
 * Desc : Get Color space info of current DPRX port
 *
 * Parm : N/A
 *
 * Retn : DP_COLOR_SPACE_RGB
 *        DP_COLOR_SPACE_YUV444
 *        DP_COLOR_SPACE_YUV422
 *        DP_COLOR_SPACE_YUV420
 *------------------------------------------------------*/
DP_COLOR_SPACE_E drvif_Dprx_GetColorSpace(void)
{
    DPRX_TIMING_INFO_T timing_info;

    if (drvif_Dprx_GetRawTimingInfo(&timing_info)==1)
        return timing_info.color_space;

    return DP_COLOR_SPACE_RGB;
}
EXPORT_SYMBOL(drvif_Dprx_GetColorSpace);


/*------------------------------------------------------
 * Func : drvif_Dprx_GetColorDepth
 *
 * Desc : Get Color depth info of current DPRX port
 *
 * Parm : N/A
 *
 * Retn : DP_COLOR_DEPTH_8B
 *        DP_COLOR_DEPTH_10B
 *        DP_COLOR_DEPTH_12B
 *        DP_COLOR_DEPTH_16B
 *------------------------------------------------------*/
DP_COLOR_DEPTH_E drvif_Dprx_GetColorDepth(void)
{
    DPRX_TIMING_INFO_T timing_info;

    if (drvif_Dprx_GetRawTimingInfo(&timing_info)==1)
        return timing_info.color_depth;

    return DP_COLOR_DEPTH_8B;
}
EXPORT_SYMBOL(drvif_Dprx_GetColorDepth);


/*------------------------------------------------------
 * Func : drvif_Dprx_GetInputType
 *
 * Desc : Get Cable Type of current DPRX port
 *
 * Parm : N/A
 *
 * Retn : DP_TYPE_USB_TYPE_C
 *        DP_TYPE_DP
 *------------------------------------------------------*/
DP_TYPE drvif_Dprx_GetInputType(void)
{
    DP_PORT_CAPABILITY cap;

    if (g_connected_dprx && dprx_adp_get_capability(g_connected_dprx, &cap)==0 && cap.type==DP_TYPE_USB_TYPE_C)
        return DP_TYPE_USB_TYPE_C;

    return DP_TYPE_DP;
}

EXPORT_SYMBOL(drvif_Dprx_GetInputType);


/*------------------------------------------------------
 * Func : drvif_Dprx_CheckMode
 *
 * Desc : Check Dprx status of current DPRX port. this function is
 *        called by VSC periodically to check DPRX status
 *
 * Parm : N/A
 *
 * Retn : 1  : DPRX status check is OK
 *        0 : some error detected
 *------------------------------------------------------*/
DPRX_bool drvif_Dprx_CheckMode(void)
{
    if (g_connected_dprx==NULL)
        return 0;

    return (dprx_adp_source_check(g_connected_dprx)==0) ? 1 : 0;
}

EXPORT_SYMBOL(drvif_Dprx_CheckMode);


/*------------------------------------------------------
 * Func : drvif_Dprx_GetDrmInfoFrame
 *
 * Desc : Get DRM Info Frame of currenct DPRX port
 *
 * Parm : pDrmInfo: pointer of DPRX_DRM_INFO_T variable from caller
 *
 * Retn : 1  : successed
 *        0 : failed
 *------------------------------------------------------*/
DPRX_bool drvif_Dprx_GetDrmInfoFrame(DPRX_DRM_INFO_T *pDrmInfo)
{
    if ((pDrmInfo==NULL)||(g_connected_dprx==NULL))
        return 0;

    if(dprx_adp_get_drm_info(g_connected_dprx, pDrmInfo)<0)
        return 0;
    else
        return 1;
}

EXPORT_SYMBOL(drvif_Dprx_GetDrmInfoFrame);


/*------------------------------------------------------
 * Func : drvif_Dprx_GetRgbYuvRangeMode
 *
 * Desc : Get Range of RGB/YUV of current DPRX port. this function will
 *        be called by
 *
 * Parm : N/A
 *
 * Retn : MODE_RAG_DEFAULT
 *        MODE_RAG_LIMIT
 *        MODE_RAG_LIMIT
 *------------------------------------------------------*/
DPRX_RGB_YUV_RANGE_MODE_T drvif_Dprx_GetRgbYuvRangeMode(void)
{
    //TBD
    return DPRX_MODE_RAG_DEFAULT;
}

EXPORT_SYMBOL(drvif_Dprx_GetRgbYuvRangeMode);


/*------------------------------------------------------
 * Func : drvif_Dprx_GetAviInfoFrame
 *
 * Desc : Get AVI Info Frame of currenct DPRX port
 *
 * Parm : pAviInfo : AVI info frame output (16 bytes)
 *
 * Retn : 1  : successed
 *        0 : failed
 *------------------------------------------------------*/
DPRX_bool drvif_Dprx_GetAviInfoFrame(DPRX_AVI_INFO_T *pAviInfo)
{
    if ((pAviInfo==NULL)||(g_connected_dprx==NULL))
        return 0;

    /*if(dprx_adp_get_avi_info(g_connected_dprx, pAviInfo)<0)
        return 0;
    else
        return 1;*/
    memset(pAviInfo,0,sizeof(DPRX_AVI_INFO_T));
    //TBD
    return 1;
}

EXPORT_SYMBOL(drvif_Dprx_GetAviInfoFrame);


/*------------------------------------------------------
 * Func : drvif_Dprx_GetPixelEncodingFormat
 *
 * Desc : Get pixel encoding format of currenct DPRX port
 *
 * Parm : pPixelEnFormat : pointer of pixel encoding format
 *
 * Retn : 1  : successed
 *        0 : failed
 *------------------------------------------------------*/
DPRX_bool drvif_Dprx_GetPixelEncodingFormat(DP_PIXEL_ENCODING_FORMAT_T *pPixelEnFormat)
{
    if ((pPixelEnFormat==NULL)||(g_connected_dprx==NULL))
        return 0;

    if(dprx_adp_get_pixel_encoding_info(g_connected_dprx, pPixelEnFormat)<0)
        return 0;
    else
        return 1;
}

EXPORT_SYMBOL(drvif_Dprx_GetPixelEncodingFormat);


 /*------------------------------------------------------
 * Func : drvif_Dprx_GetHDR10pVsInfoFrame
 *
 * Desc : Get HDR10+ Vendor Specific Info Frame of currenct DPRX port
 *
 * Parm : info_frame : HDR10+ Vendor specific info frame output
 *
 * Retn : 1  : successed
 *        0 : failed
 *------------------------------------------------------*/
DPRX_bool drvif_Dprx_GetHDR10pVsInfoFrame(DP_VSI_T *info_frame)
{
    if ((info_frame==NULL)||(g_connected_dprx==NULL))
        return 0;
    memset(info_frame,0,sizeof(DP_VSI_T));
    //TBD
    return 1;
}

EXPORT_SYMBOL(drvif_Dprx_GetHDR10pVsInfoFrame);


//-------------------------------------------------------------------------------
// DRR Related
//-------------------------------------------------------------------------------

/*------------------------------------------------------
 * Func : drvif_Dprx_GetDrrMode
 *
 * Desc : get drr mode
 *
 * Parm : N/A
 *
 * Retn : DP_DRR_MODE_FRR
 *        DP_DRR_MODE_AMD_FREE_SYNC
 *        DP_DRR_MODE_ADAPTIVE_SYNC
 *------------------------------------------------------*/
DP_DRR_MODE_E drvif_Dprx_GetDrrMode(void)
{
    DPRX_TIMING_INFO_T timing_info;

    if (g_connected_dprx==NULL)
        return DP_DRR_MODE_FRR;

    memset(&timing_info, 0, sizeof(DPRX_TIMING_INFO_T));

    if (dprx_adp_get_video_timing(g_connected_dprx, &timing_info)<0)
        return DP_DRR_MODE_FRR;

    return (DP_DRR_MODE_E) timing_info.drr_mode;
}
EXPORT_SYMBOL(drvif_Dprx_GetDrrMode);


//-------------------------------------------------------------------------------
// AMD Free Sync Related
//-------------------------------------------------------------------------------


/*------------------------------------------------------
 * Func : drvif_Dprx_GetAMDFreeSyncEnable
 *
 * Desc : get AMD free sync status
 *
 * Parm : N/A
 *
 * Retn : 1 : DPRX status check is OK
 *        0 : some error detected
 *------------------------------------------------------*/
DPRX_bool drvif_Dprx_GetAMDFreeSyncEnable(void)
{
    return (drvif_Dprx_GetDrrMode()==DP_DRR_MODE_AMD_FREE_SYNC) ? 1 : 0;
}
EXPORT_SYMBOL(drvif_Dprx_GetAMDFreeSyncEnable);


/*------------------------------------------------------
 * Func : drvif_Dprx_GetAMDFreeSyncStatus
 *
 * Desc : get AMD Free Sync Status
 *
 * Parm : p_info : info of AMD free sync status
 *
 * Retn : 0 : fail, 1 : success
 *------------------------------------------------------*/
DPRX_bool drvif_Dprx_GetAMDFreeSyncStatus(DP_AMD_FREE_SYNC_INFO_T* p_info)
{
    DP_SDP_DATA_T sdp;

    if (p_info==NULL || g_connected_dprx==NULL)
        return 0;

    memset(p_info, 0, sizeof(DP_AMD_FREE_SYNC_INFO_T));

    if (dprx_adp_get_sdp_data(g_connected_dprx, DP_SDP_TYPE_SPD, &sdp)<0) {
        DP_VFE_INFO("drvif_Dprx_GetAMDFreeSyncStatus failed \n");
        return 0;
    }

    // PB0 : Check Sum
    // PB1-3 : AMD OUI
    // PB4-5 : reserved
    // PB6 : b
    // PB7 : Min Refresh Rate (Hz)
    // PB8 : Max Refresh Rate (Hz)
    // c7 1a 00 00 _ 00 00 01 3c _ 3c 00 00 00 _ 00 00 00 00

    DP_VFE_INFO("SPD : hb={%02x %02x %02x %02x} pb={%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x}\n",
        sdp.hb[0], sdp.hb[1], sdp.hb[2], sdp.hb[3],
        sdp.pb[0], sdp.pb[1], sdp.pb[2], sdp.pb[3],
        sdp.pb[4], sdp.pb[5], sdp.pb[6], sdp.pb[7],
        sdp.pb[8], sdp.pb[9]);

    if (sdp.hb[0]== 0x00 && sdp.hb[1]== 0x83 &&
        sdp.hb[2]== 0x1B && sdp.hb[3]== 0x08 &&
        sdp.pb[1]==0x1A && sdp.pb[2]==0x00 && sdp.pb[3]==0x00)
    {
        p_info->version = 2;
        p_info->freesync_supported = ((sdp.pb[6] >>0) & 0x1);
        p_info->freesync_enabled   = ((sdp.pb[6] >>1) & 0x1);
        p_info->freesync_activate  = ((sdp.pb[6] >>2) & 0x1);
        p_info->native_color_space_active  = ((sdp.pb[6] >>3) & 0x1);
        p_info->brightness_control_active  = ((sdp.pb[6] >>4) & 0x1);
        p_info->local_diming_disalbe  = ((sdp.pb[6] >>5) & 0x1);
        p_info->min_refresh_rate   = sdp.pb[7];
        p_info->max_refresh_rate   = sdp.pb[8];

        p_info->srgb_eotf_active   = ((sdp.pb[9] >>0) & 0x1);
        p_info->bt709_eotf_active  = ((sdp.pb[9] >>1) & 0x1);
        p_info->gamma_2p2_eotf_active  = ((sdp.pb[9] >>2) & 0x1);
        p_info->gamma_2p6_eotf_active  = ((sdp.pb[9] >>3) & 0x1);
        p_info->gamma_2p6_eotf_active  = ((sdp.pb[9] >>5) & 0x1);
        p_info->brightness_control  = sdp.pb[10];
        return 1;
    }

    return 0;
}

EXPORT_SYMBOL(drvif_Dprx_GetAMDFreeSyncStatus);

/*------------------------------------------------------
 * Func : drvif_Dprx_SetVideoWatchDagEnable
 *
 * Desc : Disable/Enable Video Watch Dog Enable
 *
 * Parm : enable: Enable/Disable Video WatchDog
 *
 * Retn : 0 : fail, 1 : success
 *------------------------------------------------------*/
DPRX_bool drvif_Dprx_SetVideoWatchDagEnable(unsigned char enable)
{
    if (g_connected_dprx==NULL)
        return 0;

    if (dprx_adp_set_ext_ctrl(g_connected_dprx, (ADP_EXCTL_ID)DP_EXCTL_WATCH_DOG_ENABLE, enable)<0)
        return 0;
    return 1;
}
EXPORT_SYMBOL(drvif_Dprx_SetVideoWatchDagEnable);