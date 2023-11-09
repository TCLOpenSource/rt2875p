#ifndef _DPRX_FUN_H
#define _DPRX_FUN_H

#include "rtk_kdriver/dprx/dprx_types.h"

typedef unsigned char DPRX_bool;
/**********************************************************************************************
*
*   External Funtion Declarations
*
**********************************************************************************************/
//Timing info for scaler
extern DPRX_bool drvif_Dprx_GetRawTimingInfo(DPRX_TIMING_INFO_T* timing_info);

//Color info for PQ
extern DP_COLOR_SPACE_E drvif_Dprx_GetColorSpace(void);
extern DP_COLOR_DEPTH_E drvif_Dprx_GetColorDepth(void);

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
extern DP_TYPE drvif_Dprx_GetInputType(void);

/*------------------------------------------------------
 * Func : drvif_Dprx_CheckMode
 *
 * Desc : Check Dprx status of current DPRX port. this function is
 *        called by VSC periodically to check DPRX status
 *
 * Parm : N/A
 *
 * Retn : TRUE  : DPRX status check is OK
 *        FALSE : some error detected
 *------------------------------------------------------*/
extern DPRX_bool drvif_Dprx_CheckMode(void);

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
extern DPRX_bool drvif_Dprx_GetDrmInfoFrame(DPRX_DRM_INFO_T *pDrmInfo);

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
extern DPRX_bool drvif_Dprx_GetPixelEncodingFormat(DP_PIXEL_ENCODING_FORMAT_T *pPixelEnFormat);

typedef enum {
    DPRX_MODE_RAG_DEFAULT    = 0x0,
    DPRX_MODE_RAG_LIMIT  ,
    DPRX_MODE_RAG_FULL  ,
    DPRX_MODE_RAG_UNKNOW
} DPRX_RGB_YUV_RANGE_MODE_T;


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
DP_DRR_MODE_E drvif_Dprx_GetDrrMode(void);


//-------------------------------------------------------------------------------
// AMD Free Sync Related
//-------------------------------------------------------------------------------

/*------------------------------------------------------
 * Func : drvif_Dprx_GetAMDFreeSyncEnable
 *
 * Desc : get AMD free Sync Status
 *
 * Parm : N/A
 *
 * Retn : 0 : AMD feesync disabled, others : AMD free sync enabled
 *------------------------------------------------------*/
extern DPRX_bool drvif_Dprx_GetAMDFreeSyncEnable(void);


/*------------------------------------------------------
 * Func : drvif_Dprx_GetAMDFreeSyncStatus
 *
 * Desc : get AMD Free Sync Status
 *
 * Parm : p_info : info of AMD free sync status
 *
 * Retn : 0 : fail, 1 : success
 *------------------------------------------------------*/
extern DPRX_bool drvif_Dprx_GetAMDFreeSyncStatus(DP_AMD_FREE_SYNC_INFO_T* p_info);

//-------------------------------------------------------------------------------
// DPRX Video Watch Dog Related
//-------------------------------------------------------------------------------

/*------------------------------------------------------
 * Func : drvif_Dprx_SetVideoWatchDagEnable
 *
 * Desc : Disable/Enable Video Watch Dog Enable
 *
 * Parm : enable: Enable/Disable Video WatchDog
 *
 * Retn : 0 : fail, 1 : success
 *------------------------------------------------------*/
extern DPRX_bool drvif_Dprx_SetVideoWatchDagEnable(unsigned char enable);


#endif //_DPRX_FUN_H
