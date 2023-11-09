#ifndef __ALP_LIB_H__
#define __ALP_LIB_H__

#include "alp_def.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
        unsigned char  data_len;         // 187 : TSP header is included in data field, 184 : tsp header is stored in tsp_hdr field
        unsigned char  tsp_hdr[3];       // only valid when data len is 184 bytes
        unsigned char* data;
} alp_tsp_info;


typedef struct {
        unsigned char packet_type;
        unsigned int  packet_size;
        union {
                // packet type == ALP_PACKET_TYPE_ID_MPEG_2_TS
                struct {
                        unsigned char hdm;
                        unsigned char dpm;
                        unsigned char dpn;
                        unsigned char n_tsp;         // number of tsp : up to 16
                        alp_tsp_info  tsp_list[16];   // tsp info list
                } mepg2_ts;
        };
} alp_info;


//////////////////////////////////////////////
extern int alp_parse_alp(unsigned char* alp, unsigned int alp_data_len, alp_info* p_info);
extern const char* alp_packet_type_str(unsigned char type);
extern int alp_get_alp_size(unsigned char* alp, unsigned int alp_len);

//////////////////////////////////////////////

struct alp_reader {
        void* p_priv;       // private data
        int (*read_alp)(struct alp_reader* p_this, unsigned char* buff, unsigned int buff_sz);
};

extern struct alp_reader* alloc_alp_file_reader(const char* file_path);
extern void destroy_alp_file_reader(struct alp_reader*  p_this);

//////////////////////////////////////////////
#include "alp_mpeg2_ts.h"


#ifdef __cplusplus
}
#endif

#endif
