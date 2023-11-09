#ifndef _RTK_VDEC_SVP_H_
#define _RTK_VDEC_SVP_H_

#if IS_ENABLED(CONFIG_RTK_KDRV_TEE)

#define TYPE_SVP_PROTECT_CPB 0
#define TYPE_SVP_PROTECT_COMEM 1
#define TYPE_SVP_PROTECT_IDMDOMAIN 2
#define TYPE_SVP_PROTECT_HDMI 3
#define TYPE_SVP_PROTECT_VBM 4
#define TYPE_SVP_PROTECT_GPU 5


#define TYPE_SVP_PROTECT_DYMANIC_CMA_CLEAR  	10
#define TYPE_SVP_PROTECT_DYMAINIC_CMA_SET   	11
#define TYPE_SVP_PROTECT_DYMAINIC_CMA_CANCEL  	12

int rtkvdec_svp_enable_protection (unsigned int addr, unsigned int size, unsigned int type);

int rtkvdec_svp_disable_protection (unsigned int addr, unsigned int size, unsigned int type);

int rtkvdec_svp_early_init(void);

#define rtkvdec_svp_enable_cpb_protection(addr,size) rtkvdec_svp_enable_protection((addr), (size), TYPE_SVP_PROTECT_CPB)

#define rtkvdec_svp_disable_cpb_protection(addr,size) rtkvdec_svp_disable_protection((addr), (size), TYPE_SVP_PROTECT_CPB)

#define rtkvdec_svp_enable_hdmi_protection(addr,size) rtkvdec_svp_enable_protection((addr), (size), TYPE_SVP_PROTECT_HDMI)

#define rtkvdec_svp_disable_hdmi_protection(addr,size) rtkvdec_svp_disable_protection((addr), (size), TYPE_SVP_PROTECT_HDMI)

#define rtkvdec_svp_enable_cma_protection(addr, size)  rtkvdec_svp_dymanic_cma_protection(TYPE_SVP_DYMAINIC_CMA_SET, addr, size)

#define rtkvdec_svp_disable_cma_protection(addr, size)  rtkvdec_svp_dymanic_cma_protection(TYPE_SVP_DYMAINIC_CMA_CANCEL, addr, size)

#define rtkvdec_svp_clear_cma_protection()  rtkvdec_svp_dymanic_cma_protection(TYPE_SVP_DYMANIC_CMA_TYPE_CLEAR, 0UL,0UL)

#define rtkvdec_svp_enable_gpu_protection(addr, size)  rtkvdec_svp_enable_protection((addr), (size), TYPE_SVP_PROTECT_GPU)

#define rtkvdec_svp_disable_gpu_protection(addr, size)  rtkvdec_svp_disable_protection((addr), (size), TYPE_SVP_PROTECT_GPU)

int rtkvdec_svp_laycout_sync_tee(void);

int svp_get_display_is_enabled(void);

#endif

#endif
