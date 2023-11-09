#ifndef __BBP_LIB_H__
#define __BBP_LIB_H__


#ifdef __cplusplus
extern "C" {
#endif

#define BBP_MAX_ALPS    64
#define BBP_BLOCK_SIZE  8192

typedef struct {
        unsigned char  mode: 1;
        unsigned char  ofi: 2;
        unsigned short pointer;

        // extension
        struct {
                unsigned char   type;
                unsigned short  len;
                unsigned char*  data;
        } ext;

        // payload
        unsigned char* payload_start;
        unsigned int   payload_size;

        // alp_start
        unsigned char  n_alps;
        struct {
                unsigned short      flags;
#define ALP_HDR     (0x1)
#define ALP_TAIL    (0x1<<1)
                unsigned short      size;
                unsigned char*      data;
        } alps[BBP_MAX_ALPS];

} bbp_info;


//////////////////////////////////////////////

extern int bbp_generate_bbp_hdr(unsigned char*      p_buff,
                                unsigned int        buff_sz,
                                unsigned char*      p_ext,
                                unsigned short      n_ext,
                                unsigned char       ext_type,
                                unsigned short      pointer);

extern int bbp_get_header_size (unsigned char*      bbp,
                                unsigned int        bbp_len);

extern int bbp_parse_bbp       (unsigned char*      bbp,
                                unsigned int        bbp_len,
                                bbp_info*           p_info);

extern void bbp_dump_bbp_info  (bbp_info*           p_info);


#ifdef __cplusplus
}
#endif

#endif
