#ifndef __ATSC3_DMX_H__
#define __ATSC3_DMX_H__

//#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

//#pragma pack(1)
#include <rtd_log/rtd_module_log.h>

struct dmx_callback {
        void (*callback)(void* p_ctx, unsigned char plp, unsigned char* p_data, unsigned char type, unsigned int len, unsigned char cc, unsigned char *pTimeinfo);
};

#define RING_FLAG_REACH_LIMIT       0x1

struct atsc3_ring {
        void* p_priv;

        struct {
                // for read
                int (*read_data)(struct atsc3_ring* ring, unsigned char** pp_rp, unsigned long* p_len, unsigned long* p_flags);
                int (*release_data)(struct atsc3_ring* ring, unsigned char* p_rp, unsigned long len);

                // for write
                int (*get_free_space)(struct atsc3_ring* ring, unsigned char** pp_wp, unsigned long* p_len, unsigned long* p_flags);
                int (*put_data)(struct atsc3_ring* ring, unsigned char* p_wp, unsigned long len);
        } ops;
};

struct atsc3_dmx {
        unsigned long   plp_mask[256 / 32];     // plp mask bit
        unsigned char   bbp_buffer[8192 * 2];   // tmep buffer for multi segment buffer
        unsigned long   bbp_buff_zize;
        unsigned long   bbp_excepted_zize;
        unsigned char   bbp_plp;
        unsigned char   bbp_cc;
        unsigned char   tmode;
        unsigned int    tinfo_l;
        unsigned int    tinfo_h;

        unsigned char   tp_id;

        //pthread_t       thread_id;
        //HANDLE       thread_id;
        struct task_struct *thread_id;
        unsigned long   flags;

#define DMX_FLAG_THREAD_START    0x2
#define DMX_FLAG_THREAD_STOP     0x1

        struct atsc3_ring*  p_in;          // input ring buffer
        struct dmx_callback cb;            // dmx callback

        struct {
                int (*start)(struct atsc3_dmx* dmx);
                int (*stop)(struct atsc3_dmx* dmx);
                int (*run)(struct atsc3_dmx* dmx);
        } ops;
};

struct file_ring_ctx {
        unsigned char* base;
        unsigned char* limit;
        unsigned char* rp;
        unsigned char* wp;
};

///////////////////////////////////////////////////////
extern struct atsc3_dmx* create_atsc3_dmx(struct dmx_callback* cb, struct atsc3_ring* p_dmx_in);
extern void destroy_atsc3_dmx(struct atsc3_dmx* dmx);

int file_ring_read_data(struct atsc3_ring* ring, unsigned char** pp_rp, unsigned long* p_len, unsigned long* p_flags);
int file_ring_release_data(struct atsc3_ring* ring, unsigned char* p_rp, unsigned long len);
int file_ring_get_free_space(struct atsc3_ring* ring, unsigned char** pp_wp, unsigned long* p_len, unsigned long* p_flags);
int file_ring_put_data(struct atsc3_ring* ring, unsigned char* p_wp, unsigned long len);
struct atsc3_ring* alloc_file_ring(unsigned int buffer_size);
void destroy_file_ring(struct atsc3_ring* ring);
///////////////////////////////////////////////////////

#ifdef ATSC3_DMX_TRACE_EN
#define ATSC3_DMX_TRACE()                rtd_pr_atsc3_warn(fmt, ## args)
#else
#define ATSC3_DMX_TRACE()
#endif

#define ATSC3_DMX_DBG_EN
#ifdef ATSC3_DMX_DBG_EN

#define ATSC3_DMX_DBG(fmt, args...)
#define ATSC3_DMX_INFO(fmt, args...)      rtd_pr_atsc3_info(fmt, ## args)
#define ATSC3_DMX_WARN(fmt, args...)      rtd_pr_atsc3_warn(fmt, ## args)

#else

#define ATSC3_DMX_DBG(fmt, ...)
#define ATSC3_DMX_INFO(fmt, ...)
#define ATSC3_DMX_WARN(fmt, ...)
#endif

#ifdef __cplusplus
}
#endif

#endif
