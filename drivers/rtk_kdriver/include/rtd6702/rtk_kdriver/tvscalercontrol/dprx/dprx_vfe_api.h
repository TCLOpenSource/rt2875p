#ifndef __DPRX_VFE_API_H__
#define __DPRX_VFE_API_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    DPRX_DRV_NO_ERR = 0,
    DPRX_DRV_STATE_ERR,
    DPRX_DRV_ARG_ERR,
    DPRX_DRV_HW_ERR,
}DPRX_DRV_ERR;


/*------------------------------------------------
 * Func : vfe_dprx_drv_init
 *
 * Desc : init DPRX driver. allocate all
 *        resources will be created.
 *
 * Para : N/A
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_init(void);


/*------------------------------------------------
 * Func : vfe_dprx_drv_uninit
 *
 * Desc : uninit DPRX driver. all resources
 *        will be released.
 *
 * Para : N/A
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_uninit(void);


/*------------------------------------------------
 * Func : vfe_dprx_drv_open
 *
 * Desc : open DPRX driver. should be called before
 *        any operations
 *
 * Para : N/A
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_open(void);


/*------------------------------------------------
 * Func : vfe_dprx_drv_close
 *
 * Desc : close DPRX driver.
 *
 * Para : N/A
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_close(void);


/*------------------------------------------------
 * Func : vfe_dprx_drv_connect
 *
 * Desc : close DPRX driver.
 *
 * Para : ch : DP channel 0~3
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_connect(unsigned char ch);


/*------------------------------------------------
 * Func : vfe_dprx_drv_disconnect
 *
 * Desc : disconnect a given DPRX frontend.
 *        after disalbed, DPRX frontend will be decode.
 *
 * Para : ch : DP channel 0~3
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_disconnect(unsigned char port);


#ifdef __cplusplus
}
#endif


#endif // __DPRX_VFE_API_H__
