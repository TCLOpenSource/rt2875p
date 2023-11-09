#ifndef __TLV_RING_BUFF_H__
#define __TLV_RING_BUFF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "japan4k_dmx.h"

typedef struct {
        unsigned char*  base;
        unsigned char*  limit;
        unsigned char*  rp;
        unsigned char*  wp;
} tlv_ring_buffer;

extern tlv_ring_buffer* tlv_ring_alloc(unsigned int size);
extern void tlv_ring_destroy(tlv_ring_buffer* ring);

extern int tlv_ring_get_free_space(tlv_ring_buffer* ring, unsigned char** pp_wp, unsigned int* p_size);
extern int tlv_ring_put_data(tlv_ring_buffer* ring, unsigned char* p_wp, unsigned int len);
extern int tlv_ring_get_data(tlv_ring_buffer* ring, unsigned char** pp_rp, unsigned int* p_len);
extern int tlv_ring_release_data(tlv_ring_buffer* ring, unsigned char* p_rp, unsigned int len);

extern void tlv_ring_dump_status(tlv_ring_buffer* ring);
#ifdef __cplusplus
}
#endif

#endif
