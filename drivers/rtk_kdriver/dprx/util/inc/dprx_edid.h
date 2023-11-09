/*=============================================================
 * File:    dprx_edid.h
 *
 * Desc:    DPRX Adapter
 *
 * AUTHOR:  kevin_wang@realtek.com
 *
 * Vresion: 0.0.1
 *
 *------------------------------------------------------------
 * Copyright (c) Realtek Semiconductor Corporation, 2021
 *
 * All rights reserved.
 *
 *============================================================*/
#ifndef __DPRX_EDID_H__
#define __DPRX_EDID_H__

#include <dprx_osal.h>
#include <dprx_types.h>
#include <dprx_dev.h>  // dev_ctx_t

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    int (*init)    (dev_ctx_t* p_ctx);
    int (*uninit)  (dev_ctx_t* p_ctx);
    int (*enable)  (dev_ctx_t* p_ctx, unsigned char enable);   // allow edid to be accessed
    int (*reset)   (dev_ctx_t* p_ctx);
    int (*set_edid)(dev_ctx_t* p_ctx, unsigned char* p_edid_data, unsigned short edid_len);
    int (*get_edid)(dev_ctx_t* p_ctx, unsigned char* p_edid_data, unsigned short edid_len);
}edid_dev_ops_t;

typedef struct
{
    dev_ctx_t       ctx;
    edid_dev_ops_t  ops;
}edid_dev_t;

//================= API of DPRX EDID ==============

extern edid_dev_t* alloc_dprx_edid_device(void);
extern void destroy_dprx_edid_device(edid_dev_t* p_dev);


extern int dprx_edid_init(edid_dev_t* p_dev);

extern int dprx_edid_enable(edid_dev_t*  p_dev, unsigned char enable);

extern int dprx_edid_reset(edid_dev_t* p_dev);

extern int dprx_edid_set_edid(edid_dev_t* p_edid, unsigned char* p_edid_data, unsigned short edid_len);

extern int dprx_edid_get_edid(edid_dev_t* p_edid, unsigned char* p_edid_data, unsigned short edid_len);

//======== DPRX EDID debug message control ========
#define DPRX_EDID_ERR(fmt, args...)         dprx_err("[EDID][ERR] " fmt, ## args)
#define DPRX_EDID_WARN(fmt, args...)        dprx_warn("[EDID][WARN] " fmt, ## args)
#define DPRX_EDID_INFO(fmt, args...)        dprx_info("[EDID][INFO] " fmt, ## args)
#define DPRX_EDID_DBG(fmt, args...)         dprx_dbg("[EDID][DEBUG] " fmt, ## args)


#ifdef __cplusplus
}
#endif

#endif // __DPRX_EDID_H__
