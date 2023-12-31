#ifndef __RTK_AMP_DEVICE_ID_TABLE_H__
#define __RTK_AMP_DEVICE_ID_TABLE_H__

#define AMP_DEVICE_ID(vid ,pid)  ( (vid << 24) | (pid & 0xFFFFFF) )

typedef enum _AMP_FACTORY_BRAND
{
    RTK_ALC = 0,
    TI_TAS  = 1 ,
    ESMT_AD = 2 ,
    WELLANG = 4 ,
    MAX_BRAND ,
} AMP_FACTORY_BRAND_E;

typedef enum _AMP_DEVICE_ID
{
// RTK section
    RTK_ALC1310  = AMP_DEVICE_ID(RTK_ALC , 1310),
    RTK_ALC1311  = AMP_DEVICE_ID(RTK_ALC , 1311),
    RTK_ALC1312  = AMP_DEVICE_ID(RTK_ALC , 1312),
// TI section
    TI_TAS5707   = AMP_DEVICE_ID(TI_TAS , 5707),
    TI_TAS5711   = AMP_DEVICE_ID(TI_TAS , 5711),
    TI_TAS5751   = AMP_DEVICE_ID(TI_TAS , 5751),
    TI_TAS5805   = AMP_DEVICE_ID(TI_TAS , 5805),
// ESMT section
    ESMT_AD82010 = AMP_DEVICE_ID(ESMT_AD , 82010),
    ESMT_AD82120 = AMP_DEVICE_ID(ESMT_AD , 82120),
    ESMT_AD82088 = AMP_DEVICE_ID(ESMT_AD , 82088),
    ESMT_AD82050 = AMP_DEVICE_ID(ESMT_AD , 82050),
// WELLANG section
    WA_6819      = AMP_DEVICE_ID(WELLANG , 6819),
    WA_156819   = AMP_DEVICE_ID(WELLANG , 156819),
} AMP_DEVICE_ID_E;

#endif

