#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include "atsc3_dmx.h"
#include <rbus/tp_reg.h>
#include <tp/tp_dbg.h>
#include <tp/tp_drv_global.h>
#include <tp/tp_reg_ctrl.h>
#include "oif_lib.h"
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <asm/cacheflush.h>
#include <linux/delay.h>
#include <rbus/timer_reg.h>
#include <rbus/pll_reg_reg.h>

#include <linux/uaccess.h>

#define malloc(size)        kmalloc(size, GFP_KERNEL)
#define free(addr)          kfree(addr)

extern struct  file *rec_oif_fd;

/*======================================================
 * Func  : _dmx_thread_func
 *
 * Desc  : demux thread
 *
 * Param :
 *
 * Retn  :
 *======================================================*/
UINT32 _dmx_thread_func_run_cnt = 0;
static int _dmx_thread_func(void *data)
{
        struct atsc3_dmx* p_this = (struct atsc3_dmx*) data;
        wait_queue_head_t timeout_wq;
        init_waitqueue_head(&timeout_wq);

        set_freezable();

        while( !kthread_should_stop()) {
                _dmx_thread_func_run_cnt++;
                p_this->ops.run(p_this);
                if (freezing(current)) {
                        try_to_freeze();
                }
                //wait_event_interruptible_timeout(timeout_wq, 0, 1);
                usleep_range(1000, 1000);
        }

        p_this->flags &= ~(DMX_FLAG_THREAD_STOP | DMX_FLAG_THREAD_START);
        return 0;
}

/*======================================================
 * Func  : atsc3_dmx_start
 *
 * Desc  : start atsc3 demux
 *
 * Param : handle of atsc3 dmx
 *
 * Retn  : 0 : successed, oethers failed
 *======================================================*/
int atsc3_dmx_start(struct atsc3_dmx* p_this)
{
        ATSC3_DMX_TRACE();

        if (p_this == NULL /*|| p_this->p_in==NULL*/) {
                ATSC3_DMX_WARN("Enable ATSC 3.0 demux failed, invalid parameters\n");
                return -1;
        }

        // start thread here
        if (p_this->flags & DMX_FLAG_THREAD_START)
                return 0;

        p_this->flags &= ~DMX_FLAG_THREAD_STOP;
        p_this->flags |=  DMX_FLAG_THREAD_START;

#if 0
        if (pthread_create(&p_this->thread_id, NULL, _dmx_thread_func, p_this)) {
                p_this->flags &= ~DMX_FLAG_THREAD_START;
                return -1;
        }
#endif
        p_this->thread_id = kthread_run(_dmx_thread_func, p_this, "_dmx_thread_func");  /* no need to pass parameter into thread function */

        return 0;
}



/*======================================================
 * Func  : atsc3_dmx_stop
 *
 * Desc  : stop atsc3 demux
 *
 * Param : handle of atsc3 dmx
 *
 * Retn  : 0 : successed, oethers failed
 *======================================================*/
int atsc3_dmx_stop(struct atsc3_dmx* p_this)
{
        ATSC3_DMX_TRACE();

        if (p_this == NULL) {
                ATSC3_DMX_WARN("Enable ATSC 3.0 demux failed, invalid parameters\n");
                return -1;
        }

        // stop thread here
        p_this->flags |= DMX_FLAG_THREAD_STOP;
        //pthread_cancel(p_this->thread_id);
        if(p_this->thread_id != 0) {
                kthread_stop(p_this->thread_id);
                p_this->thread_id = 0;
        }

        return 0;
}

#define ATSC30_BBP_USE_HW
unsigned long d_ring_rp_old = 0;
unsigned long i_ring_rp_old = 0;


//unsigned long call_time_old;
//unsigned long process_time_old;
//unsigned long call_time;
//unsigned long process_time;

/*======================================================
 * Func  : atsc3_dmx_run
 *
 * Desc  :
 *
 * Param : handle of atsc3 dmx
 *
 * Retn  : 0 : successed, oethers failed
 *======================================================*/
int atsc3_dmx_run(struct atsc3_dmx* p_this)
{
#ifdef ATSC30_BBP_USE_HW
        int i;
        unsigned char oif_header[OIF_HDR_SIZE];
        unsigned char timeinfo[9];
        //unsigned long ori_rp;
        memset(timeinfo, 0x00, 9);
        ATSC3_DMX_TRACE();

        //call_time = READ_REG32(TIMER_SCPU_CLK90K_LO_reg) - call_time_old;
        //call_time_old = READ_REG32(TIMER_SCPU_CLK90K_LO_reg);
        //process_time_old = READ_REG32(TIMER_SCPU_CLK90K_LO_reg);
        //ori_rp = READ_REG32(TP_TP_ATSC_I_RP_reg);

        while(READ_REG32(TP_TP_ATSC_I_RP_reg) != READ_REG32(TP_TP_ATSC_I_WP_reg)) {
                unsigned char *rp_temp_virtual;
                unsigned char *dring_temp_virual;
                unsigned long dring_temp_phy;
                unsigned long oif_length;
                i_ring_rp_old = READ_REG32(TP_TP_ATSC_I_RP_reg);

                rp_temp_virtual = (unsigned char*)((i_ring_rp_old - pTp_drv->tp_atsc[0].iring_phyAddr) + pTp_drv->tp_atsc[0].iring_cachedaddr);
                dmac_flush_range( (char*)rp_temp_virtual, (char*)rp_temp_virtual + 12 - 1);
                outer_flush_range((UADDRESS)i_ring_rp_old, (UADDRESS)(i_ring_rp_old + 12 - 1));

                dring_temp_phy = (unsigned long)(rp_temp_virtual[8] << 24 | rp_temp_virtual[9] << 16 | rp_temp_virtual[10] << 8 | rp_temp_virtual[11]);
                dring_temp_virual = (unsigned char*)(((unsigned long)dring_temp_phy - pTp_drv->tp_atsc[0].dring_phyAddr) + pTp_drv->tp_atsc[0].dring_cachedaddr);
                oif_length = rp_temp_virtual[4] << 8 | rp_temp_virtual[5];

                if((dring_temp_virual + oif_length) < (pTp_drv->tp_atsc[0].dring_cachedaddr + pTp_drv->tp_atsc[0].dring_size)) {
                        dmac_flush_range( dring_temp_virual, dring_temp_virual + oif_length - 1);
                        outer_flush_range((UADDRESS)dring_temp_phy, (UADDRESS)(dring_temp_phy + oif_length - 1));
                        p_this->bbp_plp = dring_temp_virual[4] & 0x3f;
                        if(rec_oif_fd != NULL) {
#if IS_ENABLED(CONFIG_RTK_FEATURE_FOR_GKI)
                                kernel_write(rec_oif_fd, dring_temp_virual, oif_length, &rec_oif_fd->f_pos);
#endif
                        }

                        if(dring_temp_virual[8] != 0) {
                                memcpy(timeinfo, &dring_temp_virual[8], 9);
                                memset(&dring_temp_virual[8], 0x00, 9);
                        }
                        p_this->cb.callback(p_this, p_this->bbp_plp, dring_temp_virual + OIF_HDR_SIZE, dring_temp_virual[4], oif_length - OIF_HDR_SIZE, dring_temp_virual[7], &dring_temp_virual[8]);
                } else {
                        if( (dring_temp_virual + OIF_HDR_SIZE) < (pTp_drv->tp_atsc[0].dring_cachedaddr + pTp_drv->tp_atsc[0].dring_size)) {
                                unsigned long first_copy_length;
                                unsigned long second_copy_length;

                                first_copy_length = (pTp_drv->tp_atsc[0].dring_cachedaddr + pTp_drv->tp_atsc[0].dring_size) - (dring_temp_virual + OIF_HDR_SIZE);
                                second_copy_length = (oif_length - OIF_HDR_SIZE) - first_copy_length;

                                dmac_flush_range( dring_temp_virual, dring_temp_virual + OIF_HDR_SIZE + first_copy_length - 1);
                                outer_flush_range((UADDRESS)dring_temp_phy, (UADDRESS)(dring_temp_phy + OIF_HDR_SIZE + first_copy_length - 1));
                                memcpy(p_this->bbp_buffer, dring_temp_virual + OIF_HDR_SIZE, first_copy_length);

                                dmac_flush_range( pTp_drv->tp_atsc[0].dring_cachedaddr, pTp_drv->tp_atsc[0].dring_cachedaddr + second_copy_length - 1);
                                outer_flush_range((UADDRESS)pTp_drv->tp_atsc[0].dring_phyAddr, (UADDRESS)(pTp_drv->tp_atsc[0].dring_phyAddr + second_copy_length - 1));
                                memcpy(p_this->bbp_buffer + first_copy_length, pTp_drv->tp_atsc[0].dring_cachedaddr, second_copy_length);

                                if(rec_oif_fd != NULL) {
#if IS_ENABLED(CONFIG_RTK_FEATURE_FOR_GKI)
                                        kernel_write(rec_oif_fd, dring_temp_virual - OIF_HDR_SIZE, OIF_HDR_SIZE, &rec_oif_fd->f_pos);
                                        kernel_write(rec_oif_fd, p_this->bbp_buffer, oif_length - OIF_HDR_SIZE, &rec_oif_fd->f_pos);
#endif
                                }

                                if(dring_temp_virual[8] != 0) {
                                        memcpy(timeinfo, &dring_temp_virual[8], 9);
                                        memset(&dring_temp_virual[8], 0x00, 9);
                                }
                                p_this->bbp_plp = dring_temp_virual[4] & 0x3f;
                                p_this->cb.callback(p_this, p_this->bbp_plp, p_this->bbp_buffer, dring_temp_virual[4], oif_length - OIF_HDR_SIZE, dring_temp_virual[7], &dring_temp_virual[8]);
                        } else {
#if 0
                                unsigned char bbp_cc;
                                if( (dring_temp_virual + 7) >= (pTp_drv->tp_atsc[0].dring_nonCachedaddr + pTp_drv->tp_atsc[0].dring_size))
                                        bbp_cc = *(dring_temp_virual + 7 - pTp_drv->tp_atsc[0].dring_size);
                                else
                                        bbp_cc = dring_temp_virual[7];
#else
                                dmac_flush_range( dring_temp_virual, pTp_drv->tp_atsc[0].dring_cachedaddr + pTp_drv->tp_atsc[0].dring_size - 1);
                                outer_flush_range((UADDRESS)dring_temp_phy, (UADDRESS)(pTp_drv->tp_atsc[0].dring_phyAddr + pTp_drv->tp_atsc[0].dring_size - 1));
                                dmac_flush_range( pTp_drv->tp_atsc[0].dring_cachedaddr, (dring_temp_virual + oif_length - pTp_drv->tp_atsc[0].dring_size) - 1);
                                outer_flush_range((UADDRESS)pTp_drv->tp_atsc[0].dring_phyAddr, (UADDRESS)(dring_temp_phy + oif_length - pTp_drv->tp_atsc[0].dring_size - 1));

                                for(i = 0; i < OIF_HDR_SIZE; i++) {
                                        if( (dring_temp_virual + i) >= (pTp_drv->tp_atsc[0].dring_cachedaddr + pTp_drv->tp_atsc[0].dring_size)) {
                                                oif_header[i] = *(dring_temp_virual + i - pTp_drv->tp_atsc[0].dring_size);
                                        } else {
                                                oif_header[i] = dring_temp_virual[i];
                                        }
                                }
#endif

                                dring_temp_virual = (dring_temp_virual + OIF_HDR_SIZE) - pTp_drv->tp_atsc[0].dring_size;
                                p_this->bbp_plp = oif_header[4] & 0x3f;

                                if(rec_oif_fd != NULL) {
#if IS_ENABLED(CONFIG_RTK_FEATURE_FOR_GKI)
                                        kernel_write(rec_oif_fd, oif_header, OIF_HDR_SIZE, &rec_oif_fd->f_pos);
                                        kernel_write(rec_oif_fd, dring_temp_virual, oif_length - OIF_HDR_SIZE, &rec_oif_fd->f_pos);
#endif
                                }
                                if(oif_header[8] != 0) {
                                        memcpy(timeinfo, &oif_header[8], 9);
                                        memset(&oif_header[8], 0x00, 9);
                                }
                                p_this->cb.callback(p_this, p_this->bbp_plp, dring_temp_virual, oif_header[4], oif_length - OIF_HDR_SIZE, oif_header[7], &oif_header[8]);
                        }
                }
                WRITE_REG32(TP_TP_ATSC_D_RP_reg, dring_temp_phy);

                i_ring_rp_old = i_ring_rp_old + 0x0c;

                if(i_ring_rp_old >= (pTp_drv->tp_atsc[0].iring_phyAddr + pTp_drv->tp_atsc[0].iring_size))
                        i_ring_rp_old = pTp_drv->tp_atsc[0].iring_phyAddr;

                WRITE_REG32(TP_TP_ATSC_I_RP_reg, i_ring_rp_old);

        }
        if(timeinfo[0] != 0) {
                //process_time = READ_REG32(TIMER_SCPU_CLK90K_LO_reg) - process_time_old;
                p_this->cb.callback(p_this, 0, 0, 0, 0, 0, timeinfo);
        }

#else
        struct atsc3_ring* p_in = p_this->p_in;
        unsigned char* p_rp;
        unsigned long len;
        unsigned long flags;
        static int sync_byte_len = 0;

        ATSC3_DMX_TRACE();

        while (p_in->ops.read_data(p_in, &p_rp, &len, &flags) == 0 && len) {
                ATSC3_DMX_DBG("read data from ring (%p) = { %02x.%02x.%02x.%02x....%02x.%02x.%02x.%02x }\n",
                              p_rp,
                              p_rp[0], p_rp[1], p_rp[2], p_rp[3],
                              p_rp[len - 4], p_rp[len - 3], p_rp[len - 2], p_rp[len - 1]);

                if (p_this->bbp_excepted_zize == 0) {
                        // fid header header....
                        unsigned char* p_oif = oif_find_starter(p_rp, len, sync_byte_len);
                        unsigned int   data_len;

                        if (p_oif == NULL) {
                                p_in->ops.release_data(p_in, p_rp, len);
                                return 0;
                        }

                        if (p_oif > p_rp) {
                                // release spare data...
                                data_len = (unsigned int)(p_oif - p_rp);
                                p_in->ops.release_data(p_in, p_rp, data_len);
                                len  -= data_len;
                                p_rp  = p_oif;
                        }

                        ATSC3_DMX_DBG("Got OIF hdr(%p) = { %02x.%02x.%02x.%02x_%02x.%02x.%02x.%02x }\n",
                                      p_rp,
                                      p_rp[0], p_rp[1], p_rp[2], p_rp[3],
                                      p_rp[4], p_rp[5], p_rp[6], p_rp[7]);

                        if (len < OIF_HDR_SIZE) { // data less than 8 bytes
                                p_this->bbp_excepted_zize = OIF_HDR_SIZE;
                                p_this->bbp_buff_zize     = len;            // minimum size...
                                memcpy(p_this->bbp_buffer, p_rp, len);      // store temp buffer
                                p_in->ops.release_data(p_in, p_rp, len);    // release data
                                continue;
                        } else {
                                // a complete packet header
                                p_this->bbp_plp = p_rp[4] & 0x1F;
                                p_this->bbp_excepted_zize = OIF_HDR_SIZE + (p_rp[5] << 8) + (p_rp[6]);
                                if(p_this->bbp_cc != p_rp[7]) {
                                        ATSC3_DMX_INFO( "OIF cc not discontinue want=0x%x real=0x%x\n", p_this->bbp_cc, p_rp[7]);
                                        p_this->bbp_cc = p_rp[7];
                                }
                                p_this->bbp_cc++;

                                if(p_this->bbp_excepted_zize > 8192) {
                                        p_this->bbp_excepted_zize = 0;
                                        p_this->bbp_buff_zize = 0;
                                        p_in->ops.release_data(p_in, p_rp, 1);
                                        continue;
                                }

                                if (len >= p_this->bbp_excepted_zize) {
                                        ATSC3_DMX_DBG("complete data block, just deliver it\n");
                                        memcpy(p_this->bbp_buffer, p_rp, p_this->bbp_excepted_zize);

                                        if ( (*(p_this->bbp_buffer + 4) & 0xc0) != 0xc0) {
                                                // a complete data block : dispatch data directly
#ifdef ATSC3_DMX_DELIVER_OIF_HEADER
                                                p_this->cb.callback(p_this, p_this->bbp_plp, p_this->bbp_buffer, p_this->bbp_excepted_zize, p_this->bbp_buffer[7]);
#else
                                                p_this->cb.callback(p_this, p_this->bbp_plp, p_this->bbp_buffer + OIF_HDR_SIZE, p_this->bbp_excepted_zize - OIF_HDR_SIZE, p_this->bbp_buffer[7]);
#endif
                                        }

                                        p_in->ops.release_data(p_in, p_rp, p_this->bbp_excepted_zize);
                                        p_this->bbp_excepted_zize = 0;
                                        p_this->bbp_buff_zize = 0;
                                        continue;
                                } else {
                                        // part of block... : store it in the internal buffer
                                        memcpy(p_this->bbp_buffer, p_rp, len);
                                        p_this->bbp_buff_zize = len;
                                        p_in->ops.release_data(p_in, p_rp, len);

                                        ATSC3_DMX_DBG("part of data block, collect it, buff_size=%u, excepted size=%u\n",
                                                      p_this->bbp_buff_zize, p_this->bbp_excepted_zize);
                                        continue;
                                }
                        }
                } else if (p_this->bbp_excepted_zize == OIF_HDR_SIZE) {
                        // collecting header....
                        int data_len = OIF_HDR_SIZE - p_this->bbp_buff_zize;

                        ATSC3_DMX_DBG("collect data header, len = %u, buff_size=%u, excepted size=%u\n",
                                      len, p_this->bbp_buff_zize, p_this->bbp_excepted_zize);


                        if (data_len > len)
                                data_len = len;

                        memcpy(p_this->bbp_buffer, p_rp, data_len);
                        p_this->bbp_buff_zize += data_len;
                        p_in->ops.release_data(p_in, p_rp, data_len);

                        if (p_this->bbp_buff_zize == p_this->bbp_excepted_zize) {
                                // parse header
                                p_this->bbp_plp = p_this->bbp_buffer[4] & 0x1F;
                                p_this->bbp_excepted_zize += (p_this->bbp_buffer[5] << 8) + (p_this->bbp_buffer[6]);
                                if(p_this->bbp_cc != p_rp[7]) {
                                        ATSC3_DMX_INFO( "OIF cc not discontinue want=0x%x real=0x%x\n", p_this->bbp_cc, p_rp[7]);
                                        p_this->bbp_cc = p_rp[7];
                                }
                                p_this->bbp_cc++;

                                if(p_this->bbp_excepted_zize > 8192) {
                                        p_this->bbp_excepted_zize = 0;
                                        p_this->bbp_buff_zize = 0;
                                }
                                if(p_this->bbp_excepted_zize == OIF_HDR_SIZE) {
                                        p_this->bbp_excepted_zize = 0;
                                        p_this->bbp_buff_zize = 0;
                                }
                                p_this->bbp_cc  = p_this->bbp_buffer[7];
                                continue;
                        }
                } else {
                        // collecting data
                        int data_len = p_this->bbp_excepted_zize - p_this->bbp_buff_zize;       // remain size
                        ATSC3_DMX_DBG("collect data block, len = %u, buff_size=%u, excepted size=%u\n",
                                      len, p_this->bbp_buff_zize, p_this->bbp_excepted_zize);


                        if (data_len > len)
                                data_len = len;

                        memcpy(p_this->bbp_buffer + p_this->bbp_buff_zize, p_rp, data_len);
                        p_in->ops.release_data(p_in, p_rp, data_len);

                        p_this->bbp_buff_zize += data_len;

                        if (p_this->bbp_buff_zize >= p_this->bbp_excepted_zize) {
                                // complete block
                                if ( (*(p_this->bbp_buffer + 4) & 0xc0) != 0xc0) {
#ifdef ATSC3_DMX_DELIVER_OIF_HEADER
                                        p_this->cb.callback(p_this, p_this->bbp_plp, p_this->bbp_buffer, p_this->bbp_excepted_zize, p_this->bbp_buffer[7]);
#else
                                        p_this->cb.callback(p_this, p_this->bbp_plp, p_this->bbp_buffer + OIF_HDR_SIZE, p_this->bbp_excepted_zize - OIF_HDR_SIZE, p_this->bbp_buffer[7]);
#endif
                                        p_this->bbp_excepted_zize = 0;
                                        p_this->bbp_buff_zize = 0;
                                } else {
                                        p_this->bbp_excepted_zize = 0;
                                        p_this->bbp_buff_zize = 0;
                                }
                        }
                }
        }
#endif
        return 0;
}

/*======================================================
 * Func  : create_atsc3_dmx
 *
 * Desc  : create a atsc3 demux
 *
 * Param : p_dmx_in : input buffer
 *
 * Retn  : handle of atsc3 dmx
 *======================================================*/
struct atsc3_dmx* create_atsc3_dmx(
        struct dmx_callback*        p_cb,
        struct atsc3_ring*          p_dmx_in
)
{
        struct atsc3_dmx* p_this;

        ATSC3_DMX_TRACE();

        p_this = (struct atsc3_dmx*) malloc(sizeof(struct atsc3_dmx));

        if (p_this) {
                memset(p_this, 0, sizeof(struct atsc3_dmx));
                p_this->p_in = p_dmx_in;
                p_this->cb = *p_cb;
                p_this->ops.start = atsc3_dmx_start;
                p_this->ops.stop  = atsc3_dmx_stop;
                p_this->ops.run   = atsc3_dmx_run;
//        p_this->ops.deliver_data = atsc3_dmx_deliver_data;
        }

        return p_this;
}

/*======================================================
 * Func  : destroy_atsc3_dmx
 *
 * Desc  :
 *
 * Param : handle of atsc3 dmx
 *
 * Retn  : N/A
 *======================================================*/
void destroy_atsc3_dmx(struct atsc3_dmx* p_this)
{
        ATSC3_DMX_TRACE();
        if (p_this)
                free(p_this);
}

/*======================================================
 * Func  : file_ring_read_data
 *
 * Desc  :
 *
 * Param :
 *
 * Retn  : N/A
 *======================================================*/
int file_ring_read_data(
        struct atsc3_ring*      ring,
        unsigned char**         pp_rp,
        unsigned long*          p_len,
        unsigned long*          p_flags
)
{
        struct file_ring_ctx* p_ctx = (struct file_ring_ctx*) ring->p_priv;

        *pp_rp = p_ctx->rp;

        // check size
        if (p_ctx->wp >= p_ctx->rp) {
                *p_len = p_ctx->wp - p_ctx->rp;
                *p_flags = 0;
        } else {
                *p_len = p_ctx->limit - p_ctx->rp;
                *p_flags = RING_FLAG_REACH_LIMIT;
        }

        //file_ring_dbg("read data: rp=%p, len=%d\n", *pp_rp, *p_len);

        return 0;

}


/*======================================================
 * Func  : file_ring_release_data
 *
 * Desc  :
 *
 * Param :
 *
 * Retn  : N/A
 *======================================================*/
int file_ring_release_data(
        struct atsc3_ring*      ring,
        unsigned char*          p_rp,
        unsigned long           len
)
{
        struct file_ring_ctx* p_ctx = (struct file_ring_ctx*) ring->p_priv;

        //file_ring_dbg("release data : p_rp=%p/%p, len=%d\n", p_rp, p_ctx->rp, len);

        p_ctx->rp += len;
        if (p_ctx->rp >= p_ctx->limit)
                p_ctx->rp = p_ctx->base;

        return 0;
}

/*======================================================
 * Func  : file_ring_get_free_space
 *
 * Desc  :
 *
 * Param :
 *
 * Retn  : N/A
 *======================================================*/
int file_ring_get_free_space(
        struct atsc3_ring*      ring,
        unsigned char**          pp_wp,
        unsigned long*          p_len,
        unsigned long*          p_flags
)
{
        struct file_ring_ctx* p_ctx = (struct file_ring_ctx*) ring->p_priv;

        *pp_wp = p_ctx->wp;

        // check size
        if (p_ctx->wp >= p_ctx->rp) {
                *p_len = (p_ctx->limit - p_ctx->wp);
                *p_flags = 0;
        } else {
                *p_len = (p_ctx->rp - p_ctx->wp);
                *p_flags = 0;
        }

        return 0;
}

/*======================================================
 * Func  : file_ring_put_data
 *
 * Desc  :
 *
 * Param :
 *
 * Retn  : N/A
 *======================================================*/
int file_ring_put_data(
        struct atsc3_ring*      ring,
        unsigned char*          p_wp,
        unsigned long           len
)
{
        struct file_ring_ctx* p_ctx;

        if (ring == NULL) {
                ATSC3_DMX_INFO("ring_release_data failed, invalid arguments\n");
                return -1;
        }

        p_ctx = (struct file_ring_ctx*) ring->p_priv;

        if (p_wp != p_ctx->wp) {
                // humm ... should not happen
                ATSC3_DMX_INFO("ring_release_data failed, wp out off sync\n");
                return -1;
        }

        if (p_wp > p_ctx->limit || p_wp < p_ctx->base)
                return -1;

        p_ctx->wp = p_wp + len;

        if (p_ctx->wp >= p_ctx->limit)
                p_ctx->wp = p_ctx->base;

        return 0;
}

/*======================================================
 * Func  : alloc_file_ring
 *
 * Desc  :
 *
 * Param :
 *
 * Retn  : N/A
 *======================================================*/
struct atsc3_ring* alloc_file_ring(unsigned int buffer_size)
{
        //unsigned char* buff = (unsigned char*) malloc(sizeof(struct atsc3_ring) + sizeof(struct file_ring_ctx) + buffer_size);
        //unsigned char* buff = (unsigned char*) kmalloc(sizeof(struct atsc3_ring) + sizeof(struct file_ring_ctx) + buffer_size , GFP_KERNEL);
        struct atsc3_ring*    ring;
        struct file_ring_ctx* p_ctx;
        unsigned char* buff = (unsigned char*)dvr_malloc_specific(sizeof(struct atsc3_ring) + sizeof(struct file_ring_ctx) + buffer_size, GFP_DCU1_FIRST);

        if(buff == 0) {
                ATSC3_DMX_INFO("ERROR!! alloc_file_ring dvr_malloc_specific Fail buffer_size=%d\n", buffer_size);
        }

        ring = (struct atsc3_ring*) buff;
        p_ctx = (struct file_ring_ctx*) (buff + sizeof (struct atsc3_ring));
        buff += sizeof(struct atsc3_ring) + sizeof(struct file_ring_ctx);

        if (ring) {
                ring->p_priv = p_ctx;
                p_ctx->base  = buff; //((unsigned char*) p_ctx) + sizeof(struct file_ring_ctx);
                p_ctx->limit = p_ctx->base + buffer_size;
                p_ctx->rp    = p_ctx->base;
                p_ctx->wp    = p_ctx->base;

                ring->ops.read_data  = file_ring_read_data;
                ring->ops.release_data = file_ring_release_data;
                ring->ops.get_free_space = file_ring_get_free_space;
                ring->ops.put_data = file_ring_put_data;
        }

        return ring;
}

/*======================================================
 * Func  : destroy_file_ring
 *
 * Desc  :
 *
 * Param :
 *
 * Retn  : N/A
 *======================================================*/
void destroy_file_ring(struct atsc3_ring* ring)
{
        if (ring) {
                //free(ring);
                //kfree(ring);
                dvr_free(ring);
                ATSC3_DMX_INFO("destroy_file_ring dvr_free success\n");
        }
}

