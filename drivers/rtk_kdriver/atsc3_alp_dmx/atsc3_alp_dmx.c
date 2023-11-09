#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/pageremap.h>
#include <linux/syscalls.h>
#include <linux/delay.h>     // for msleep

#include <tp/tp_reg_ctrl.h>
#include "atsc3_alp_lib.h"
#include "atsc3_alp_dmx.h"

#define atsc3_alp_dmx_malloc(size)        kmalloc(size, GFP_KERNEL)
#define atsc3_alp_dmx_free(addr)          kfree(addr)

//#define USE_ALP_LIKE

/*======================================================
 * Func  : _atsc3_alp_dmx_thread_func
 *
 * Desc  : demux thread
 *
 * Param :
 *
 * Retn  :
 *======================================================*/
static int _atsc3_alp_dmx_thread_func(void *data)
{
        struct atsc3_alp_dmx* p_this = (struct atsc3_alp_dmx*) data;
        wait_queue_head_t timeout_wq;
        init_waitqueue_head(&timeout_wq);

        while((p_this->flags & DMX_FLAG_THREAD_STOP) == 0) {
                p_this->ops.run(p_this);
                //usleep(1000);
                wait_event_interruptible_timeout(timeout_wq, 0, 1);
        }

        p_this->flags &= ~(DMX_FLAG_THREAD_STOP | DMX_FLAG_THREAD_START);
        return 0;
}

/*======================================================
 * Func  : atsc3_alp_dmx_start
 *
 * Desc  : start atsc3_alp demux
 *
 * Param : handle of atsc3_alp dmx
 *
 * Retn  : 0 : successed, oethers failed
 *======================================================*/
int atsc3_alp_dmx_start(struct atsc3_alp_dmx* p_this)
{
        ATSC3_ALP_DMX_TRACE();

        if (p_this == NULL) {
                ATSC3_ALP_DMX_WARN("Enable ATSC3 ALP demux failed, invalid parameters\n");
                return -1;
        }

        // start thread here
        if (p_this->flags & DMX_FLAG_THREAD_START)
                return 0;

        p_this->flags &= ~DMX_FLAG_THREAD_STOP;
        p_this->flags |=  DMX_FLAG_THREAD_START;

        p_this->thread_id = kthread_run(_atsc3_alp_dmx_thread_func, p_this, "_atsc3_alp_dmx_thread_func");  /* no need to pass parameter into thread function */

        return 0;
}



/*======================================================
 * Func  : atsc3_alp_dmx_stop
 *
 * Desc  : stop atsc3_alp demux
 *
 * Param : handle of atsc3_alp dmx
 *
 * Retn  : 0 : successed, oethers failed
 *======================================================*/
int atsc3_alp_dmx_stop(struct atsc3_alp_dmx* p_this)
{
        ATSC3_ALP_DMX_TRACE();

        if (p_this == NULL) {
                ATSC3_ALP_DMX_WARN("Enable atsc3_alp demux failed, invalid parameters\n");
                return -1;
        }

        // stop thread here
        p_this->flags |= DMX_FLAG_THREAD_STOP;
        kthread_stop(p_this->thread_id);

        return 0;
}

/*======================================================
 * Func  : atsc3_alp_dmx_run
 *
 * Desc  :
 *
 * Param : handle of atsc3_alp dmx
 *
 * Retn  : 0 : successed, oethers failed
 *======================================================*/
int atsc3_alp_dmx_run(struct atsc3_alp_dmx* p_this)
{
        unsigned char *p_rp, *p_rp_ori;
        UINT32 len, len_ori;

        static int sync_byte_len = 3;

        ATSC3_ALP_DMX_TRACE();

        while (atsc3_alp_file_ring_read_data(&p_rp, &len) == 0 && len) {
                p_rp_ori = p_rp;
                len_ori  = len;
                while(len > 0) {
                        ATSC3_ALP_DMX_DBG("read data from ring (%p) = { %02x.%02x.%02x.%02x....%02x.%02x.%02x.%02x }\n",
                                          p_rp,
                                          p_rp[0], p_rp[1], p_rp[2], p_rp[3],
                                          p_rp[len - 4], p_rp[len - 3], p_rp[len - 2], p_rp[len - 1]);

                        if (p_this->tsp_excepted_size == 0) {
                                // fid header header....
                                unsigned char* p_alp;
                                unsigned int   data_len;

                                if( (p_rp[1] == 0x00 || p_rp[1] == 0x40) && p_rp[2] == 0x00) {
                                        p_alp = p_rp;
                                } else {
                                        p_alp = atsc3_alp_find_starter(p_rp, len, sync_byte_len);
                                }

                                if (p_alp == NULL) {
                                        ATSC3_ALP_RELEASE_DATA(len);
                                        ATSC3_ALP_DMX_ERROR("alp re-sync 1 can not find sync byte p_rp=%p len=%d\n", p_rp, len);
                                        continue;
                                }

                                if (p_alp > p_rp) {
                                        // release spare data...
                                        data_len = (unsigned int)(p_alp - p_rp);
                                        ATSC3_ALP_RELEASE_DATA(data_len);
                                        ATSC3_ALP_DMX_ERROR("tlv re-sync 2 p_rp=%p len=%d data_len=%d\n", p_rp, len, data_len);
                                }

                                if (len < ATSC3_ALP_HDR_SIZE) { // data less than 4 bytes
                                        p_this->tsp_excepted_size = ATSC3_ALP_HDR_SIZE;
                                        memcpy(p_this->tsp_buffer, p_rp, len);      // store temp buffer
                                        p_this->tsp_buff_size     = len;            // minimum size...
                                        ATSC3_ALP_RELEASE_DATA(len);
                                } else {
                                        ATSC3_ALP_DMX_DBG("complete data block, just deliver it\n");
                                        // a complete data block : dispatch data directly

#ifdef USE_ALP_LIKE
                                        memcpy(p_this->alp_buffer + p_this->alp_buff_zize, p_rp, ATSC3_ALP_HDR_SIZE);
                                        p_this->alp_buff_zize = p_this->alp_buff_zize + ATSC3_ALP_HDR_SIZE;

                                        if(p_this->alp_buff_zize >= ATSC3_ALP_HDR_SIZE * 40) {
                                                p_this->cb.callback(p_this, 0x00, p_this->alp_buffer, 0, p_this->alp_buff_zize , 0x00, 0x00);
                                                p_this->alp_buff_zize = 0;
                                        }
#else
                                        if(p_rp[1] == 0x00 && p_rp[2] == 0x00 && ( (p_rp[3] & 0x20) == 0x20)) {
                                                if(p_this->alp_buff_zize != 0) {
                                                        unsigned char tsp_offset = 5 + p_rp[4];
                                                        memcpy(p_this->alp_buffer + p_this->alp_buff_zize, &p_rp[tsp_offset], ATSC3_ALP_HDR_SIZE - tsp_offset);
                                                        p_this->alp_buff_zize = p_this->alp_buff_zize + ATSC3_ALP_HDR_SIZE - tsp_offset;
                                                        p_this->cb.callback(p_this, p_rp[0], p_this->alp_buffer, 0, p_this->alp_buff_zize , 0x00, 0x00);
                                                        p_this->alp_buff_zize = 0;
                                                }
                                        } else if(p_rp[1] == 0x00 && p_rp[2] == 0x00 && ( (p_rp[3] & 0x20) == 0x00)) {
                                                if(p_this->alp_buff_zize != 0) {
                                                        memcpy(p_this->alp_buffer + p_this->alp_buff_zize, &p_rp[4], ATSC3_ALP_HDR_SIZE - 4);
                                                        p_this->alp_buff_zize = p_this->alp_buff_zize + (ATSC3_ALP_HDR_SIZE - 4);
                                                }
                                        } else if(p_rp[1] == 0x40 && p_rp[2] == 0x00 && ( (p_rp[3] & 0x20) == 0x00)) {
                                                if(p_this->alp_buff_zize != 0) {
                                                        p_this->cb.callback(p_this, p_rp[0], p_this->alp_buffer, 0, p_this->alp_buff_zize , 0x00, 0x00);
                                                }
                                                p_this->alp_buff_zize = 0;
                                                memcpy(p_this->alp_buffer + p_this->alp_buff_zize, &p_rp[4], ATSC3_ALP_HDR_SIZE - 4);
                                                p_this->alp_buff_zize = p_this->alp_buff_zize + (ATSC3_ALP_HDR_SIZE - 4);
                                        } else {
                                                p_this->alp_buff_zize = 0;
                                        }
#endif
                                        //ATSC3_ALP_DMX_ERROR("r#%d %d\n", __LINE__, ATSC3_ALP_HDR_SIZE);
                                        ATSC3_ALP_RELEASE_DATA(ATSC3_ALP_HDR_SIZE);
                                        p_this->tsp_excepted_size = 0;
                                        //ATSC3_ALP_DMX_ERROR("t_e_z =0 #%d\n", __LINE__);
                                        p_this->tsp_buff_size = 0;
                                }
                        } else if (p_this->tsp_excepted_size == ATSC3_ALP_HDR_SIZE) {
                                // collecting header....
                                if ( (len + p_this->tsp_buff_size) < ATSC3_ALP_HDR_SIZE) { // data less than 4 bytes
                                        memcpy(p_this->tsp_buffer + p_this->tsp_buff_size, p_rp, len);          // store temp buffer
                                        p_this->tsp_buff_size = p_this->tsp_buff_size + len;                        // minimum size...
                                        ATSC3_ALP_RELEASE_DATA(len);
                                } else {
                                        unsigned char process_size = ATSC3_ALP_HDR_SIZE - p_this->tsp_buff_size;
                                        memcpy(p_this->tsp_buffer + p_this->tsp_buff_size, p_rp, process_size);         // store temp buffer
                                        ATSC3_ALP_RELEASE_DATA(process_size);
#ifdef USE_ALP_LIKE
                                        memcpy(p_this->alp_buffer + p_this->alp_buff_zize, p_this->tsp_buffer, ATSC3_ALP_HDR_SIZE);
                                        p_this->alp_buff_zize = p_this->alp_buff_zize + ATSC3_ALP_HDR_SIZE;

                                        if(p_this->alp_buff_zize >= ATSC3_ALP_HDR_SIZE * 40) {
                                                p_this->cb.callback(p_this, 0x00, p_this->alp_buffer, 0, p_this->alp_buff_zize , 0x00, 0x00);
                                                p_this->alp_buff_zize = 0;
                                        }

                                        p_this->tsp_excepted_size = 0;
                                        p_this->tsp_buff_size = 0;
#else
                                        p_this->tsp_excepted_size = 0;
                                        p_this->tsp_buff_size = 0;

                                        if(p_this->tsp_buffer[1] == 0x00 && p_this->tsp_buffer[2] == 0x00 && ( (p_this->tsp_buffer[3] & 0x20) == 0x20)) {
                                                unsigned char tsp_offset = 5 + p_this->tsp_buffer[4];
                                                memcpy(p_this->alp_buffer + p_this->alp_buff_zize, &p_this->tsp_buffer[tsp_offset], ATSC3_ALP_HDR_SIZE - tsp_offset);
                                                p_this->alp_buff_zize = p_this->alp_buff_zize + ATSC3_ALP_HDR_SIZE - tsp_offset;
                                                p_this->cb.callback(p_this, p_this->tsp_buffer[0], p_this->alp_buffer, 0, p_this->alp_buff_zize , 0x00, 0x00);
                                                p_this->alp_buff_zize = 0;
                                        } else if(p_this->tsp_buffer[1] == 0x00 && p_this->tsp_buffer[2] == 0x00 && ( (p_this->tsp_buffer[3] & 0x20) == 0x00)) {
                                                memcpy(p_this->alp_buffer + p_this->alp_buff_zize, &p_this->tsp_buffer[4], ATSC3_ALP_HDR_SIZE - 4);
                                                p_this->alp_buff_zize = p_this->alp_buff_zize + (ATSC3_ALP_HDR_SIZE - 4);
                                        } else if(p_this->tsp_buffer[1] == 0x40 && p_this->tsp_buffer[2] == 0x00 && ( (p_this->tsp_buffer[3] & 0x20) == 0x00)) {
                                                p_this->alp_buff_zize = 0;
                                                memcpy(p_this->alp_buffer + p_this->alp_buff_zize, &p_this->tsp_buffer[4], ATSC3_ALP_HDR_SIZE - 4);
                                                p_this->alp_buff_zize = p_this->alp_buff_zize + (ATSC3_ALP_HDR_SIZE - 4);
                                        } else {
                                                p_this->alp_buff_zize = 0;
                                        }
#endif
                                }
                        }
                }
                atsc3_alp_file_ring_release_data(p_rp_ori, len_ori);
        }

        return 0;
}

/*======================================================
 * Func  : create_atsc3_alp_dmx
 *
 * Desc  : create a atsc3_alp demux
 *
 * Param : p_dmx_in : input buffer
 *
 * Retn  : handle of atsc3_alp dmx
 *======================================================*/
struct atsc3_alp_dmx* create_atsc3_alp_dmx(
        struct atsc3_alp_dmx_callback*            p_cb
)
{
        struct atsc3_alp_dmx* p_this;

        ATSC3_ALP_DMX_TRACE();

        p_this = (struct atsc3_alp_dmx*) atsc3_alp_dmx_malloc(sizeof(struct atsc3_alp_dmx));

        if (p_this) {
                memset(p_this, 0, sizeof(struct atsc3_alp_dmx));
                p_this->cb = *p_cb;
                p_this->ops.start = atsc3_alp_dmx_start;
                p_this->ops.stop  = atsc3_alp_dmx_stop;
                p_this->ops.run   = atsc3_alp_dmx_run;
        }

        return p_this;
}

/*======================================================
 * Func  : destroy_atsc3_alp_dmx
 *
 * Desc  :
 *
 * Param : handle of atsc3_alp dmx
 *
 * Retn  : N/A
 *======================================================*/
void destroy_atsc3_alp_dmx(struct atsc3_alp_dmx* p_this)
{
        ATSC3_ALP_DMX_TRACE();
        if (p_this)
                atsc3_alp_dmx_free(p_this);
}

/*======================================================
 * Func  : atsc3_alp_file_ring_read_data
 *
 * Desc  :
 *
 * Param :
 *
 * Retn  : N/A
 *======================================================*/
int atsc3_alp_file_ring_read_data(
        unsigned char**         pp_rp,
        UINT32*          p_len
)
{
        int result;
        unsigned char *temp;
        result = TP_ReadData_NoNeedAligment(TP_TP2, pp_rp, p_len, 1024, TP_BUFFER_MASS_DATA);
        if(result < 0) {
                *p_len = 0;
        } else {
                temp = *pp_rp;
                *pp_rp = Tp_Buf_Phy_to_Vir(TP_TP2, TP_BUFFER_MASS_DATA, (unsigned char*)*pp_rp);
                if(*pp_rp == NULL) {
                        ATSC3_ALP_DMX_ERROR("pp_rp = 0x%p result=%d\n", temp, result);
                }
        }

        return 0;
}


/*======================================================
 * Func  : atsc3_alp_file_ring_release_data
 *
 * Desc  :
 *
 * Param :
 *
 * Retn  : N/A
 *======================================================*/
int atsc3_alp_file_ring_release_data(
        unsigned char*          p_rp,
        UINT32                  len
)
{
        p_rp = Tp_Buf_Vir_to_Phy(TP_TP2, TP_BUFFER_MASS_DATA, (unsigned char*)p_rp);
        TP_ReleaseData_NoNeedAligment(TP_TP2, p_rp, len, TP_BUFFER_MASS_DATA);
        return 0;
}

/*======================================================
 * Func  : atsc3_alp_file_ring_get_free_space
 *
 * Desc  :
 *
 * Param :
 *
 * Retn  : N/A
 *======================================================*/
int atsc3_alp_file_ring_get_free_space(
        unsigned char**          pp_wp,
        unsigned long*          p_len
)
{
        return 0;
}

/*======================================================
 * Func  : atsc3_alp_file_ring_put_data
 *
 * Desc  :
 *
 * Param :
 *
 * Retn  : N/A
 *======================================================*/
int atsc3_alp_file_ring_put_data(
        unsigned char*          p_wp,
        unsigned long           len
)
{
        return 0;
}

