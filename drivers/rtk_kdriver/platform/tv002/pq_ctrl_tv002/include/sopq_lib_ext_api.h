#ifndef __SOPQ_LIB_EXT_API_H__
#define __SOPQ_LIB_EXT_API_H__

/*---------------------------------------------------------------------------*
 *      Description: This is wrapper to Platform base types
 *      Prefix:
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 *      External Header Files [#include <filename.h>]
 *---------------------------------------------------------------------------*/
#include "rtk_pq_tv002_api.h"
#include "rtk_vip_logger.h"
/*---------------------------------------------------------------------------*
 *      Internal Header Files [#include "filename.h" ]
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 *      Global Macros/Constants/Typedefs
 *---------------------------------------------------------------------------*/
#define SOPQLIB_LOG_DEBUG_VGIP(fmt, args...)	VIPprintfPrivateCount(KERN_INFO, VIP_LOGMODULE_VGIP_IP31_interface7_DEBUG, log_print, fmt, ##args)
#define SOPQLIB_LOG_DEBUG_VIP(fmt, args...)	VIPprintfPrivateCount(KERN_INFO, VIP_LOGMODULE_VIP_IP31_interface7_DEBUG, log_print, fmt, ##args)

/*---------------------------------------------------------------------------*
 *      Local Macros/Constants/Typedefs
 *---------------------------------------------------------------------------*/
typedef RTK_TV002_PQLIB_SCL_METER_T			SONYPQLIB_SCL_METER_T;
typedef RTK_TV002_PQLIB_LD_METER_T   		SONYPQLIB_LD_METER_T;
typedef RTK_TV002_PQLIB_NOISE_LVL_T 	    SONYPQLIB_NOISE_LVL_T;
typedef RTK_TV002_PQLIB_4BAND_HFILTER_T 	SONYPQLIB_4BAND_HFILTER_T;
typedef RTK_TV002_PQLIB_4BAND_FILTERRANGE_T	SONYPQLIB_4BAND_FILTERRANGE_T;
typedef RTK_TV002_PQLIB_OSD_INFO_T	        SONYPQLIB_OSD_INFO_T;
typedef RTK_TV002_PQLIB_FREQ_DET_T	        SONYPQLIB_FREQ_DET_T;

typedef RTK_TV002_PQLIB_DCON_OUT_T			SONYPQLIB_DCON_OUT_T;
typedef RTK_TV002_PQLIB_DCOL_OUT_T			SONYPQLIB_DCOL_OUT_T;
typedef RTK_TV002_PQLIB_DBC_OUT_T			SONYPQLIB_DBC_OUT_T;
typedef RTK_TV002_PQLIB_LD_OUT_T			SONYPQLIB_LD_OUT_T;
typedef RTK_TV002_PQLIB_DSHP_OUT_T			SONYPQLIB_DSHP_OUT_T;
/*---------------------------------------------------------------------------*
 *      Struct
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 *      Enum
 *---------------------------------------------------------------------------*/

//Please do conversion for RTK Enum to SonyPQ Enum!!!
typedef enum {
    E_SONYPQLIB_VGIP_ISR_STATUS_DISABLE = 0,
    E_SONYPQLIB_VGIP_ISR_STATUS_ENABLE,
    E_SONYPQLIB_VGIP_ISR_STATUS_UNKNOWN,
} E_SONYPQLIB_VGIP_ISR_STATUS;

/*---------------------------------------------------------------------------*
 *      Struct
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 *      Global Variables
 *---------------------------------------------------------------------------*/
static unsigned int log_print = 0;

/*---------------------------------------------------------------------------*
 *      Local Variables
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 *      Global Function Prototypes
 *---------------------------------------------------------------------------*/

/* Run APIs */
void SonyPQLib_RunDConCalc( UINT8 u1SeqId );
void SonyPQLib_RunDColCalc( UINT8 u1SeqId );
void SonyPQLib_RunDBCCalc( UINT8 u1SeqId );
void SonyPQLib_RunLDCalc( UINT8 u1SeqId );
INT32 SonyPQLib_RunDSHPCalc( UINT8 u1SeqId );

/* Set APIs */
INT32 SonyPQLib_SetScalerMeter( UINT8 u1SeqId, SONYPQLIB_SCL_METER_T* pSclMeter );
INT32 SonyPQLib_SetLdMeter( UINT8 u1SeqId, SONYPQLIB_LD_METER_T* pLdMeter );
INT32 SonyPQLib_SetGeneralFunc( UINT16 u2Type, UINT8* pData, UINT8 u1Size );
INT32 SonyPQLib_SetShareMemoryArea( UINT8* pData, UINT32 u4MaximumSize );
INT32 SonyPQLib_SetVgipIsrStatus( E_SONYPQLIB_VGIP_ISR_STATUS eVgipIsrStatus );
INT32 SonyPQLib_SetNoiseLevel( UINT8 u1SeqId, SONYPQLIB_NOISE_LVL_T* tDShpNoise );
INT32 SonyPQLib_SetOsdInfo( UINT8 u1SeqId, SONYPQLIB_OSD_INFO_T* pOsdInfo );
INT32 SonyPQLib_SetFreqDet( UINT8 u1SeqId, SONYPQLIB_FREQ_DET_T* tFreqDet );

/* Get APIs */
INT32 SonyPQLib_GetDConOut( SONYPQLIB_DCON_OUT_T* pDconOut );
INT32 SonyPQLib_GetDColOut( SONYPQLIB_DCOL_OUT_T* pDcolOut );
INT32 SonyPQLib_GetDBCOut( SONYPQLIB_DBC_OUT_T* pDbcOut );
INT32 SonyPQLib_GetLDOut( SONYPQLIB_LD_OUT_T* pLdOut );
INT32 SonyPQLib_GetDShpOut( SONYPQLIB_DSHP_OUT_T* pDshpOut );
INT32 SonyPQLib_GetGeneralFunc( UINT16 u2Type, UINT8* pData, UINT8 u1Size );

/*---------------------------------------------------------------------------*
 *      Local Function Prototypes
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 *      Global Function
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 *      Local Function
 *---------------------------------------------------------------------------*/

#endif /* __SOPQ_LIB_EXT_API_H__ */
