#ifndef __RTK_PQ_TV002_API_H__
#define __RTK_PQ_TV002_API_H__

/*---------------------------------------------------------------------------*
 *
 *
 *      Description:
 *      Prefix:
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 *      External Header Files [#include <filename.h>]
 *---------------------------------------------------------------------------*/

#include <rtk_pq_cmn_util_lib_types.h>

/*---------------------------------------------------------------------------*
 *      Internal Header Files [#include "filename.h" ]
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 *      Global Macros/Constants/Typedefs
 *---------------------------------------------------------------------------*/

#define RTK_LUMA_HISTOGRAM_MAZSIZE              128
#define RTK_SAT_HISTOGRAM_MAZSIZE               32
#define RTK_HUE_HISTOGRAM_MAZSIZE               32
#define RTK_LD_METER_APL_MAZSIZE                16
#define RTK_DCON_LUMA_CURVE_MAZSIZE             129
#define RTK_GLOBAL_SAT_BY_SAT_MAZSIZE           17

/*---------------------------------------------------------------------------*
 *      Local Macros/Constants/Typedefs
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 *      Struct
 *---------------------------------------------------------------------------*/

/* Set APIs */
typedef struct {
    UINT16  u2Apl;
    UINT32  u4MSR_YHIST[RTK_LUMA_HISTOGRAM_MAZSIZE];
    UINT32  u4MSR_SHIST[RTK_SAT_HISTOGRAM_MAZSIZE];
    UINT32  u4MSR_HHIST[RTK_HUE_HISTOGRAM_MAZSIZE];
    UINT8   u1SC_YMEAN_RATIO;
    UINT8   u1SC_YDEV_RATIO;
    UINT8   u1SC_MOTION_RATIO;
} RTK_TV002_PQLIB_SCL_METER_T;

typedef struct {
    UINT16  u2LdMaxRgb;
    UINT16  u2LdApl[RTK_LD_METER_APL_MAZSIZE];
} RTK_TV002_PQLIB_LD_METER_T;

typedef struct {
	UINT8	u1NoiseLevel;
	UINT8	u1BNoiseLevel;
} RTK_TV002_PQLIB_NOISE_LVL_T;

typedef struct {
	UINT8	u1bitshift_a0;
	UINT8	u1bitshift_b0;
	UINT8	u1h_filter_switch_0;
	UINT8	u1h_filter_band0_div;
	INT16	i2h_filter_band0_coef[21];
	UINT8	u1bitshift_a1;
	UINT8	u1bitshift_b1;
	UINT8	u1h_filter_switch_1;
	UINT8	u1h_filter_band1_div;
	INT8	i1h_filter_band1_coef[15];
	UINT8	u1bitshift_a2;
	UINT8	u1bitshift_b2;
	UINT8	u1h_filter_switch_2;
	UINT8	u1h_filter_band2_div;
	INT8	i1h_filter_band2_coef[15];
	UINT8	u1bitshift_a3;
	UINT8	u1bitshift_b3;
	UINT8	u1h_filter_switch_3;
	UINT8	u1h_filter_band3_div;
	INT8	i1h_filter_band3_coef[15];
} RTK_TV002_PQLIB_4BAND_HFILTER_T;

typedef struct {
	UINT16	u2h_filter_h_start;
	UINT16	u2h_filter_h_size;
	UINT16	u2h_filter_v_start;
	UINT16	u2h_filter_v_size;
} RTK_TV002_PQLIB_4BAND_FILTERRANGE_T;

typedef struct {
	UINT32	u3a0_hist[16];
	UINT32	u3b0_hist[16];
	UINT32	u3a1_hist[16];
	UINT32	u3b1_hist[16];
	UINT32	u3a2_hist[16];
	UINT32	u3b2_hist[16];
	UINT32	u3a3_hist[16];
	UINT32	u3b3_hist[16];
} RTK_TV002_PQLIB_FREQ_DET_T;

/* Get APIs */
typedef struct {
    UINT8   u1SeqId;
    UINT16  setLumaCurve[RTK_DCON_LUMA_CURVE_MAZSIZE];
    UINT8   u1UpofExt;
    UINT8   u1UbofExt;
} RTK_TV002_PQLIB_DCON_OUT_T;

typedef struct {
    UINT8   u1SeqId;
    UINT16  u2GlobalSatBySat[RTK_GLOBAL_SAT_BY_SAT_MAZSIZE];
} RTK_TV002_PQLIB_DCOL_OUT_T;

typedef struct {
    UINT8   u1SeqId;
    UINT16  u2DbcAplCtl;
    UINT16  u2DbcMaxCtl;
    UINT16  u2DbcBkCtl;
    UINT16  u2DbcBk2Ctl;
    UINT16  u2DbcGfxGain;
    UINT8   u2Dbc4PsBlCtl;
} RTK_TV002_PQLIB_DBC_OUT_T;

typedef struct {
    UINT8   u1SeqId;
    UINT16  u2LdPixelGain;
    UINT16  u2HeatBlSave;
} RTK_TV002_PQLIB_LD_OUT_T;

typedef struct {
	UINT8	u1SeqId;
	UINT8	u1DS_FRQ_LEVEL;
} RTK_TV002_PQLIB_DSHP_OUT_T;

typedef struct {
	UINT8	u1OsdRatio;
} RTK_TV002_PQLIB_OSD_INFO_T;

/*---------------------------------------------------------------------------*
 *      Enum
 *---------------------------------------------------------------------------*/

typedef enum {
	E_RTK_TV002_PQLIB_VGIP_ISR_STATUS_DISABLE,
	E_RTK_TV002_PQLIB_VGIP_ISR_STATUS_ENABLE,
	E_RTK_TV002_PQLIB_VGIP_ISR_STATUS_UNKNOWN,
} E_RTK_TV002_PQLIB_VGIP_ISR_STATUS;

/*---------------------------------------------------------------------------*
 *      Struct
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 *      Global Variables
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 *      Local Variables
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 *      Global Function Prototypes
 *---------------------------------------------------------------------------*/
void RTK_TV002_PQLib_GetHardwareReg(UINT32 u4PhysicalAddress, UINT32 u4RegValue);
UINT32 RTK_TV002_PQLib_SetHardwareReg (UINT32 u4PhysicalAddress);
void RTK_TV002_PQLib_Set4BandHFilter(RTK_TV002_PQLIB_4BAND_HFILTER_T *tDShpHFilter);
void RTK_TV002_PQLib_Set4BandFilterRange(RTK_TV002_PQLIB_4BAND_FILTERRANGE_T *tDshpFilterRng);
/*---------------------------------------------------------------------------*
 *      Local Function Prototypes
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 *      Global Function
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 *      Local Function
 *---------------------------------------------------------------------------*/

#endif /* __RTK_PQ_TV002_API_H__ */
