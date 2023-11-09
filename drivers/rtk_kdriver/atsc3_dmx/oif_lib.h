#ifndef __OIF_LIB_H__
#define __OIF_LIB_H__

#include "alp_def.h"

#ifdef __cplusplus
extern "C" {
#endif

//#pragma pack(1)

#define OIF_STARTER_VAL     0x5A5A5A5A
#define OIF_HDR_SIZE        17
#define SYNC_BYTE_LEN       4
#define MAX_OIF_SIZE        8192*2 + OIF_HDR_SIZE

#define OIF_GET_MSEC(tinfo_l)              ((tinfo_l>>22) & 0x3FF)
#define OIF_GET_USEC(tinfo_l)              ((tinfo_l>>12) & 0x3FF)
#define OIF_GET_NSEC(tinfo_l)              ((tinfo_l>>2)  & 0x3FF)

typedef struct {
        unsigned int   starter;

#if 1
        unsigned int   plp_id: 6;
        unsigned int   err: 1;
        unsigned int   err_en: 1;
        unsigned int   length : 16;
        unsigned int   cc : 8;
#else
        unsigned int   cc : 8;
        unsigned int   length : 16;
        unsigned int   plp_id: 6;
        unsigned int   err: 1;
        unsigned int   err_en: 1;
#endif
        unsigned char  tmode;
        unsigned int   tinfo_l;
        unsigned int   tinfo_h;
} OIF_HDR;

extern int oif_encapsulate_bbf( unsigned char*      bbf,
                                unsigned short      bbf_sz,
                                unsigned char*      out,
                                unsigned long       out_len,
                                unsigned char       plp,
                                unsigned char*      p_cc,
                                unsigned char       tmode,
                                unsigned int        tinfo_l,
                                unsigned int        tinfo_h,
                                unsigned char       err);

extern int oif_parse_sync_byte( unsigned char*      oif_in,
                                unsigned short      oif_sb_sz,
                                unsigned char*      oif_out,
                                unsigned long       sb_len);
extern int oif_generate_sync_byte( unsigned char*   oif_in,
                                   unsigned short   oif_sz,
                                   unsigned char*   oif_out);
unsigned char* oif_find_starter(unsigned char*      oif,
                                unsigned long       oif_len,
                                unsigned long       sb_len);
#define OIF_STARTER(oif)    (*(unsigned int*)oif)
#define OIF_PLP(oif)        (oif[4] & 0x3F)
#define OIF_ERROR(oif)      ((oif[4]>>6) & 0x3)
#define OIF_LENGTH(oif)     ((oif[5]<<8) + oif[6])
#define OIF_CC(oif)         (oif[7])


#ifdef __cplusplus
}
#endif

#endif
