#ifndef __ATSC3_ALP_DMX_H__
#define __ATSC3_ALP_DMX_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <rtd_log/rtd_module_log.h>

#define ATSC3_ALP_RELEASE_DATA(alp_len) \
        p_rp = p_rp + alp_len; \
        len  = len - alp_len;

//#pragma pack(1)

struct atsc3_alp_dmx_callback {
        void (*callback)(void* p_ctx, unsigned char plp, unsigned char* p_data, unsigned char type, unsigned int len, unsigned char cc, unsigned char *pTimeinfo);
};

#define DMX_FLAG_THREAD_START    0x2
#define DMX_FLAG_THREAD_STOP     0x1

struct atsc3_alp_dmx {
        unsigned char   alp_buffer[65536];   // tmep buffer for multi segment buffer
        unsigned long   alp_buff_zize;
        unsigned char   tsp_buffer[188];     // tmep buffer for multi segment buffer
        unsigned char   tsp_buff_size;       // tmep buffer for multi segment buffer
        unsigned char   tsp_excepted_size;   // tmep buffer for multi segment buffer

        unsigned char   tp_id;

        struct task_struct *thread_id;
        unsigned long   flags;
        struct atsc3_alp_dmx_callback cb;    // dmx callback

        struct {
                int (*start)(struct atsc3_alp_dmx* dmx);
                int (*stop)(struct atsc3_alp_dmx* dmx);
                int (*run)(struct atsc3_alp_dmx* dmx);
        } ops;
};

///////////////////////////////////////////////////////
struct atsc3_alp_dmx* create_atsc3_alp_dmx(struct atsc3_alp_dmx_callback* cb);
void destroy_atsc3_alp_dmx(struct atsc3_alp_dmx* dmx);

int atsc3_alp_file_ring_read_data(unsigned char** pp_rp, UINT32* p_len);
int atsc3_alp_file_ring_release_data(unsigned char* p_rp, UINT32 len);
int atsc3_alp_file_ring_get_free_space(unsigned char** pp_wp, unsigned long* p_len);
int atsc3_alp_file_ring_put_data(unsigned char* p_wp, unsigned long len);
///////////////////////////////////////////////////////

#ifdef ATSC3_ALP_DMX_TRACE_EN
#define ATSC3_ALP_DMX_TRACE()                rtd_pr_atsc3alp_info(fmt, ## args)
#else
#define ATSC3_ALP_DMX_TRACE()
#endif

#define ATSC3_ALP_DMX_DBG_EN
#ifdef ATSC3_ALP_DMX_DBG_EN

#define ATSC3_ALP_DMX_DBG(fmt, args...)
#define ATSC3_ALP_DMX_INFO(fmt, args...)       rtd_pr_atsc3alp_info(fmt, ## args)
#define ATSC3_ALP_DMX_WARN(fmt, args...)       rtd_pr_atsc3alp_warn(fmt, ## args)
#define ATSC3_ALP_DMX_ERROR(fmt, args...)      rtd_pr_atsc3alp_err(fmt, ## args)

#else

#define ATSC3_ALP_DMX_DBG(fmt, ...)
#define ATSC3_ALP_DMX_INFO(fmt, ...)
#define ATSC3_ALP_DMX_WARN(fmt, ...)
#endif

#ifdef __cplusplus
}
#endif

#endif
