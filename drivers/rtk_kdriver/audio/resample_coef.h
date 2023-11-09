#ifndef __RESAMPLE_DEF_SYSTEM_H__
#define __RESAMPLE_DEF_SYSTEM_H__

#include "audio_inc.h"

#define RESAMPLE_16BIT_TABLE  //need to define RESAMPLE_16BIT_TABLE_ASM at FW

#define SWAP_TABLE  // swap the resample tables for fw little endian

#define TAPS_NUMBER	 60

#ifndef SWAP_UINT16
#define SWAP_UINT16(x) (const short)( (((x) & 0xff) << 8) | ((unsigned short)(x) >> 8) )
#endif
#ifndef SWAP_UINT32
#define SWAP_UINT32(x) (unsigned int)((((x) >> 24)&0x000000FF) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24))
#endif


#define RESAMP_SRC_NUMBER_22050_48000	147
#define RESAMP_DST_NUMBER_22050_48000	320

#define RESAMP_SRC_NUMBER_11025_48000	147
#define RESAMP_DST_NUMBER_11025_48000	640

#define FIXED_ONE_2P30 ((unsigned int)(1<<30)-1) //(2^30-1)
#define FIXED_ONE_1P15 ((unsigned int)(1<<15)) //(2^15)
#define SW_MIN	(short)0x8000 // for value -1 in 16 bit
#define SW_MAX	(short)0x7fff // for value 1 in 16 bit

#ifdef RESAMPLE_16BIT_TABLE

#define RESAMP_COEF_UNITSIZE_IN_BYTE (2)
#else
#define RESAMP_COEF_UNITSIZE_IN_BYTE (4)

#endif

typedef struct
{
    int Taps;
    int SrcSamples;
    int DstSamples;

    void* Coefs;

    int nCoefSizeInByte;
    void* uncacheCoefAddress;
    UINT32 phyCoefAddress;
} RESAMPLE_COEF_INFO;



void CreateResample22To48Info(RESAMPLE_COEF_INFO* pCoef);
void CreateResample11To48Info(RESAMPLE_COEF_INFO* pCoef);
void DeleteResampleInfo(RESAMPLE_COEF_INFO* pCoef);
HRESULT SetResampleCoef(RESAMPLE_COEF_INFO* pCoef);



#endif // #ifndef __RESAMPLE_DEF_SYSTEM_H__
