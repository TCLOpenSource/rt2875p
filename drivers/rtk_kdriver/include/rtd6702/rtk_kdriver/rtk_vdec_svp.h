#ifndef _RTK_VDEC_SVP_H_
#define _RTK_VDEC_SVP_H_

#if IS_ENABLED(CONFIG_RTK_KDRV_TEE)

#define TYPE_SVP_PROTECT_CPB 0
#define TYPE_SVP_PROTECT_COMEM 1
#define TYPE_SVP_PROTECT_IDMDOMAIN 2
#define TYPE_SVP_PROTECT_HDMI 3
#define TYPE_SVP_PROTECT_VBM 4


int rtkvdec_svp_enable_protection (unsigned int addr, unsigned int size, unsigned int type);

int rtkvdec_svp_disable_protection (unsigned int addr, unsigned int size, unsigned int type);

int rtkvdec_svp_early_init(void);

#define rtkvdec_svp_enable_cpb_protection(addr,size) rtkvdec_svp_enable_protection((addr), (size), TYPE_SVP_PROTECT_CPB)

#define rtkvdec_svp_disable_cpb_protection(addr,size) rtkvdec_svp_disable_protection((addr), (size), TYPE_SVP_PROTECT_CPB)

#define rtkvdec_svp_enable_hdmi_protection(addr,size) rtkvdec_svp_enable_protection((addr), (size), TYPE_SVP_PROTECT_HDMI)

#define rtkvdec_svp_disable_hdmi_protection(addr,size) rtkvdec_svp_disable_protection((addr), (size), TYPE_SVP_PROTECT_HDMI)



#define TYPE_SVP_PROTECT_DYMAINIC_OP_SET       11
#define TYPE_SVP_PROTECT_DYMAINIC_OP_CANCEL    12

typedef enum
{
        SVP_DYNAMIC_PROTECT_I3DDMA,
        SVP_DYNAMIC_PROTECT_SUB,
        SVP_DYNAMIC_PROTECT_VBM,
    	SVP_DYNAMIC_PROTECT_MEMC,
        SVP_DYNAMIC_PROTECT_MDOMAIN,
    	SVP_DYNAMIC_PROTECT_OD,
    	SVP_DYNAMIC_PROTECT_DI_NR,
    	SVP_DYNAMIC_PROTECT_DEFAULT,
    	SVP_DYNAMIC_PROTECT_MAX,
}svp_protect_type_enum;

typedef struct
{
     union {
	struct
	{
	   uint32_t mc_entry:8;
	   uint32_t mc_type:8;
	   uint32_t type:8;
	   uint32_t op:8;
	};
	uint32_t val;
     }oper;
     int  is_from_cma;
     uint32_t key;
     uint32_t read;
     uint32_t write;
     uint64_t start;
     uint64_t size;

}svp_dynamic_desc_t;

int rtkvdec_svp_dymanic_protection( svp_dynamic_desc_t * desc);


#define SVP_FUNC_INIT(_type, name) \
static inline int rtkvdec_svp_enable_##name##_protection(unsigned int addr, unsigned int size, int is_from_cma) {\
	svp_dynamic_desc_t desc = {};\
	desc.oper.op = TYPE_SVP_PROTECT_DYMAINIC_OP_SET;\
	desc.oper.type = _type;\
	desc.is_from_cma = is_from_cma;\
	desc.start = (uint64_t)addr;\
	desc.size = (uint64_t)size;\
\
	return rtkvdec_svp_dymanic_protection(&desc);\
	\
} \
static inline int rtkvdec_svp_disable_##name##_protection(unsigned int addr, unsigned int size, int is_from_cma) {\
	svp_dynamic_desc_t desc = {};\
	desc.oper.op = TYPE_SVP_PROTECT_DYMAINIC_OP_CANCEL;\
	desc.oper.type = _type;\
	desc.is_from_cma = is_from_cma;\
	desc.start = (uint64_t)addr;\
	desc.size = (uint64_t)size;\
\
	return rtkvdec_svp_dymanic_protection(&desc);\
}\


/*
	rtkvdec_svp_enable_i3ddma_protection(addr, size, is_from_cma)
	rtkvdec_svp_disable_i3ddma_protection(addr, size, is_from_cma)
*/
SVP_FUNC_INIT(SVP_DYNAMIC_PROTECT_I3DDMA, i3ddma)
/*
	rtkvdec_svp_enable_sub_protection(addr, size, is_from_cma)
	rtkvdec_svp_disable_sub_protection(addr, size, is_from_cma)
*/
SVP_FUNC_INIT(SVP_DYNAMIC_PROTECT_SUB, sub)
/*
	rtkvdec_svp_enable_cma_protection(addr, size, is_from_cma)
	rtkvdec_svp_disable_cma_protection(addr, size, is_from_cma)
*/
SVP_FUNC_INIT(SVP_DYNAMIC_PROTECT_VBM, vbm)


int svp_get_display_is_enabled(void);

int rtkvdec_svp_laycout_sync_tee(void);

int rtkvdec_get_dcib_config(void *buf, int size);

#endif

#endif
