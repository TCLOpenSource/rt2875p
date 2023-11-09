#if IS_ENABLED(CONFIG_RTK_KDRV_TEE)

#define CFG_RTK_CMA_MAP 0

#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/semaphore.h>
#include <linux/tee.h>
#include <linux/uuid.h>
#include <linux/tee_drv.h>
#include <rtk_kdriver/rtk_vdec_svp.h>
#include <linux/mm.h>
#include <linux/pageremap.h>
#include <linux/vmalloc.h>
#include <linux/highmem.h>
#include <asm/cacheflush.h>
#include <mach/rtk_platform.h>
#include <rtd_log/rtd_module_log.h>



////////////////////////////////////////////////////////////////////////////////
#define TEE_NUM_PARAM 4

////////////////////////////////////////////////////////////////////////////////

//#define TA_RTK_SVP_UUID { 0xae9c26f2, 0x363f, 0x11e8,    { 0xb4, 0x67, 0x0e,
// 0xd5, 0xf8, 0x9f, 0x71, 0x8b} }
static const uuid_t svp_uuid =
	UUID_INIT(0x15c66c21, 0x1275, 0x4040,
							0xaa, 0x61, 0xfb, 0x1e, 0xe4, 0x56, 0xda, 0xca);

#define MEMPRT_CMD_SET_HDMI_PROTECT 200
#define MEMPRT_CMD_CANCEL_HDMI_PROTECT 201

#define MEMPRT_CMD_SET_GPU_PROTECT 202
#define MEMPRT_CMD_CANCEL_GPU_PROTECT 203

#define MEMPRT_CMD_OpenContentProtectedBuffer 1000
#define MEMPRT_CMD_CloseContentProtectedBuffer 1001

#define MEMPRT_CMD_OpenDisplayProtectedCOMEM 2001
#define MEMPRT_CMD_CloseDisplayProtectedCOMEM 2002

#define MEMPRT_CMD_OpenDisplayProtectedBufferScaler 3000
#define MEMPRT_CMD_CloseDisplayProtectedBufferScaler 3001

#define MEMPRT_CMD_OpenDisplayProtectedVBM 3002
#define MEMPRT_CMD_CloseDisplayProtectedVBM 3003

#define MEMPRT_CMD_Set_Display_Dynamic_protection 3010

#define MEMPRT_CMD_SetMemLayoutIndex                       3333

// extern struct semaphore Mprison_Semaphore;
struct semaphore Mprison_Semaphore = __SEMAPHORE_INITIALIZER(Mprison_Semaphore, 1);
struct optee_ta {
	struct tee_context *ctx;
	__u32 session;
};

static struct optee_ta svp_ta = {NULL, 0};
static int svp_display_is_enabled = 0;

#define MAX_VBM_REGION_NUM  24
#define MAX_VBM_CHANNEL   8 
#define VBM_REGION_MAGIC  0xafedcda5
typedef struct
{
	int start;
	int enable;
	int max_size;
}vbm_region_growup_t;


static vbm_region_growup_t vbm_region_growup[MAX_VBM_CHANNEL];

#define VBM_REGION_IS_ENABLE(_c)   (vbm_region_growup[_c].enable == VBM_REGION_MAGIC)
#define VBM_REGION_IS_DISABLE(_c)   (vbm_region_growup[_c].enable != VBM_REGION_MAGIC)
#define VBM_REGION_IS_EXIST(_c, start)   ((vbm_region_growup[_c].enable == VBM_REGION_MAGIC) && (vbm_region_growup[_c].start == start))


static int vbm_region_growup_unset(unsigned long start , unsigned int size)
{
	int i;
	int exist_offset = -1;

	for(i=0; i<MAX_VBM_CHANNEL; i++){
		if(VBM_REGION_IS_EXIST(i, start)){
			exist_offset = i;
		}
	}
	if(exist_offset == -1)
	{
		//BUG();
		return -1;
	}

	if(vbm_region_growup[exist_offset].max_size != size)
	{
		BUG();
	}
	vbm_region_growup[exist_offset].max_size = 0;
	vbm_region_growup[exist_offset].enable = 0;
	vbm_region_growup[exist_offset].start = 0;
	return 0;

}
static int vbm_region_growup_check(unsigned long start , unsigned int size, unsigned long * need_unmap_start, unsigned int * need_unmap_size)
{
	int i;
	int exist_offset = -1;
	int empty_offset = -1;


	if(need_unmap_start == NULL || need_unmap_size  == NULL)
	{
		return -1;
	}

	*need_unmap_start = *need_unmap_size = 0;

	//find exist
	for(i=0; i<MAX_VBM_CHANNEL; i++){
		if(VBM_REGION_IS_EXIST(i, start)){
			exist_offset = i;
			break;
		}
	}
	if(exist_offset != -1){
		if(vbm_region_growup[exist_offset].max_size < size){
			*need_unmap_start = start + vbm_region_growup[exist_offset].max_size;
			*need_unmap_size =  size - vbm_region_growup[exist_offset].max_size;
			rtd_pr_vdec_err("VBM Grow up %lx %x, %lx, %x\n", start, size, *need_unmap_start, *need_unmap_size);
			vbm_region_growup[exist_offset].max_size = size;
			return 0;
		}
		else {
			*need_unmap_start  = 0;
			return -1;
		}
	}else{
		for(i=0; i<MAX_VBM_CHANNEL; i++){
			if(VBM_REGION_IS_DISABLE(i)){
				empty_offset = i;
			}
		}
		if(empty_offset == -1){
			return  -ENOSPC;
		}else {
			*need_unmap_start = start;
			*need_unmap_size = size;
			vbm_region_growup[empty_offset].max_size  = size;
			vbm_region_growup[empty_offset].start = start;
			vbm_region_growup[empty_offset].enable = VBM_REGION_MAGIC;
			rtd_pr_vdec_err("VBM first %lx %x, %lx, %x\n", start, size, *need_unmap_start, *need_unmap_size);
			return 0;
		}


	}


}

//beacause api not export
int vmap_pages_range_noflush(unsigned long addr, unsigned long end, 
                        pgprot_t prot, struct page **pages, unsigned int page_shift);

#if 0
int rtkvdec_svp_cma_memory_free(unsigned long phys_addr,unsigned int size )
{
#ifdef CONFIG_ARM64
	struct page ** pages;
        int count, i;

	if (!pfn_valid(phys_addr >> PAGE_SHIFT))
	{
		rtd_pr_vdec_err("page removed by memblock, not exist in cma now!, no need svp_cma init");
		return 0;
	}

	//rtd_pr_vdec_err("free memory %lx, size=0x%x\n", phys_addr ,size);
        count = size >> PAGE_SHIFT;

	pages = vmalloc(count * sizeof(struct page *));
	if(pages == NULL)
        {
                rtd_pr_vdec_err("Error: svp Failed to free cma\n");
                return -1;
        }

        for(i=0; i<count; i++)
	{
		pages[i] =  pfn_to_page((phys_addr + PAGE_SIZE*i) >> PAGE_SHIFT);
	}

#ifndef CONFIG_RTK_KDRV_VDEC_MODULE
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0))
    do{ 
        unsigned long  start, end;
        start = (unsigned long)phys_to_virt(phys_addr);
        end = (unsigned long)phys_to_virt(phys_addr) +  (count<<PAGE_SHIFT);
        vmap_pages_range_noflush(start, end , PAGE_KERNEL, pages, PAGE_SHIFT);
    } while(0);            
#else        
	map_kernel_range_noflush((unsigned long)phys_to_virt(phys_addr), count<<PAGE_SHIFT, PAGE_KERNEL, pages);
#endif    
#else
	do{
		struct vm_struct area = {};
		area.addr = (void *)phys_to_virt(phys_addr);
		area.size = size;
		area.flags &= VM_NO_GUARD;
		map_vm_area(&area, PAGE_KERNEL, pages);
	} while(0);
#endif

	vfree(pages);
#endif
	return 0;

}
int rtkvdec_svp_cma_memory_init(unsigned long phys_addr,unsigned int size )
{


#ifdef CONFIG_ARM64
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0)) 
	unsigned long start, end;
#endif
  
	if (!pfn_valid(phys_addr >> PAGE_SHIFT))
	{
		rtd_pr_vdec_err("page removed by memblock, not exist in cma now!, no need svp_cma free");
		return 0;
	}

	//rtd_pr_vdec_err("init memory %lx, size=0x%x\n", phys_addr ,size);
	dmac_flush_range(phys_to_virt(phys_addr), phys_to_virt(phys_addr)+size);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0))
       start = (unsigned long)phys_to_virt(phys_addr);
       end = start + size;
       vunmap_range(start , end );
#else
	unmap_kernel_range((unsigned long)phys_to_virt(phys_addr), size);
#endif /*LINUX_VERSION_CODE*/ 

#else
        struct page *page, *tpage;
        void *va;
        int i, count, pfn_base;

        page = pfn_to_page(phys_addr >> PAGE_SHIFT);

        count = size >> PAGE_SHIFT;
        if(!PageHighMem(page))
        {
                        dmac_flush_range(phys_to_virt(phys_addr), phys_to_virt(phys_addr)+size);
                        unmap_kernel_range_noflush((unsigned long)phys_to_virt(phys_addr), size);
        }
        else
        {
                pfn_base= page_to_pfn(page);
                for(i=0; i<count; i++)
                {
                        tpage = pfn_to_page(pfn_base+i);
                        va = page_address(tpage);
                        if(va){
                                rtd_pr_vdec_emerg("[PLI-MALLOC] before flush pfn=%d %d  %d, va=%p, ph=%lx",pfn_base, i, count, va, page_to_pfn(tpage) << PAGE_SHIFT);
                                dmac_flush_range((void *)va, (void *)(va+PAGE_SIZE));
                                flush_tlb_kernel_range((unsigned long)va, (unsigned long)(va+PAGE_SIZE));
                        }
                }
                kmap_flush_unused();
                dmac_flush_range(phys_addr, phys_addr + size);

                //outer_flush_range(phys_addr, phys_addr + size);
        }
#endif

        return 0;
}
#endif





#if defined(CONFIG_RTK_KDRV_VDEC_MODULE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0))

/*
#define MEMORY_HOTHUG_ENABLE
#define REMOVE_MAPPING(_pa, _size)   do{\
                                        if(remove_memory_subsection(_pa, _size)){\
                                                rtd_pr_vdec_err("error: remove_memory_subsection %lx %x\n", _pa ,_size); \
                                                panic("svp error\n");\
                                        }\
                                       }while(0)
 
//#define RESTORE_MAPPING(_pa, _size)  do{\
                                         if(add_memory_subsection(0, _pa, _size)){\
                                                 rtd_pr_vdec_err("error: add_memory_subsection %lx %x\n", _pa ,_size); \
                                                 panic("svp error\n");\
                                         }\
                                      }while(0)  
*/

#if 1
#define REMOVE_MAPPING(_pa, _size)    rtk_set_memory_valid((unsigned long)phys_to_virt(_pa), _size, 0) 
#define RESTORE_MAPPING(_pa, _size)   rtk_set_memory_valid((unsigned long)phys_to_virt(_pa), _size, 1)  
#else 

#define REMOVE_MAPPING(_pa, _size)    vunmap_range((unsigned long)phys_to_virt(_pa), (unsigned long)phys_to_virt(_pa)+_size) 
#define RESTORE_MAPPING(_pa, _size)   vmap_pages_range_noflush((unsigned long)phys_to_virt(_pa), (unsigned long)phys_to_virt(_pa)+_size , PAGE_KERNEL, pages, PAGE_SHIFT);

#endif

#elif defined(CONFIG_RTK_KDRV_VDEC_MODULE) && (LINUX_VERSION_CODE < KERNEL_VERSION(5, 13, 0))

#define REMOVE_MAPPING(_pa, _size)   unmap_kernel_range((unsigned long)phys_to_virt(_pa), _size) 
#define RESTORE_MAPPING(_pa, _size)   do{\
                struct vm_struct area = {};\
                area.addr = (void *)phys_to_virt(phys_addr);\
                area.size = size;\
                area.flags &= VM_NO_GUARD;\
                map_vm_area(&area, PAGE_KERNEL, pages);\
        } while(0);


#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0))

#if-0
#define REMOVE_MAPPING(_pa, _size)    vunmap_range((unsigned long)phys_to_virt(_pa), (unsigned long)phys_to_virt(_pa)+_size) 
#define RESTORE_MAPPING(_pa, _size)   vmap_pages_range_noflush((unsigned long)phys_to_virt(_pa), (unsigned long)phys_to_virt(_pa)+_size , PAGE_KERNEL, pages, PAGE_SHIFT);
#else 

#define REMOVE_MAPPING(_pa, _size)    rtk_set_memory_valid((unsigned long)phys_to_virt(_pa), _size, 0) 
#define RESTORE_MAPPING(_pa, _size)   rtk_set_memory_valid((unsigned long)phys_to_virt(_pa), _size, 1)  


#endif

#elif (LINUX_VERSION_CODE < KERNEL_VERSION(5, 13, 0))

#define REMOVE_MAPPING(_pa, _size)   unmap_kernel_range((unsigned long)phys_to_virt(_pa), _size) 
#define RESTORE_MAPPING(_pa, _size)  map_kernel_range_noflush((unsigned long)phys_to_virt(phys_addr), count<<PAGE_SHIFT, PAGE_KERNEL, pages);

#endif




int rtkvdec_svp_cma_memory_init(unsigned long phys_addr,unsigned int size )
{
#ifdef CONFIG_ARM64
        if (!pfn_valid(phys_addr >> PAGE_SHIFT))
        {
                rtd_pr_vdec_err("page removed by memblock, not exist in cma now!, no need svp_cma free");
                return 0;
        }

        //rtd_pr_vdec_err("remove %lx, %x\n", phys_addr, size);
        dmac_flush_range(phys_to_virt(phys_addr), phys_to_virt(phys_addr)+size);
        outer_flush_range(phys_addr, phys_addr + size);
        REMOVE_MAPPING(phys_addr, size);
        return 0;
#endif
}

int rtkvdec_svp_cma_memory_free(unsigned long phys_addr,unsigned int size )
{

#ifdef CONFIG_ARM64

#ifdef MEMORY_HOTHUG_ENABLE
        //rtd_pr_vdec_err("restore %lx, %x\n", phys_addr, size);
        RESTORE_MAPPING(phys_addr, size);
        return 0;
#else   
        struct page ** pages;
        int count, i;


        if (!pfn_valid(phys_addr >> PAGE_SHIFT))
        {
                rtd_pr_vdec_err("page removed by memblock, not exist in cma now!, no need svp_cma init");
                return 0;
        }

        //rtd_pr_vdec_err("free memory %lx, size=0x%x\n", phys_addr ,size);
        count = size >> PAGE_SHIFT;

        pages = vmalloc(count * sizeof(struct page *));
        if(pages == NULL)
        {
                rtd_pr_vdec_err("Error: svp Failed to free cma\n");
                return -1;
        }

        for(i=0; i<count; i++)
        {
                pages[i] =  pfn_to_page((phys_addr + PAGE_SIZE*i) >> PAGE_SHIFT);
        }
        RESTORE_MAPPING(phys_addr, size);
        vfree(pages);
        return 0;
#endif
#endif /*ARM64*/

}



static int optee_svp_match(struct tee_ioctl_version_data *data,
                           const void *vers) {
	return 1;
}
static __maybe_unused void svp_set_enabled(int enableStatus);

static int rtkvdec_svp_init(void) {
	int ret = -1, rc = 0;
	struct tee_param param[TEE_NUM_PARAM];
	struct tee_ioctl_open_session_arg arg;
	struct tee_ioctl_version_data vers = {
	    .impl_id = TEE_IMPL_ID_OPTEE,
	    .impl_caps = TEE_OPTEE_CAP_TZ,
	    .gen_caps = TEE_GEN_CAP_GP,
	};

	memset(&svp_ta, 0, sizeof(svp_ta));
	down(&Mprison_Semaphore);
	svp_ta.ctx = tee_client_open_context(NULL, optee_svp_match, NULL, &vers);
	up(&Mprison_Semaphore);
	if (svp_ta.ctx == NULL) {
		rtd_pr_vdec_err("rtkvdec_svp: no ta context\n");
		ret = -EINVAL;
		goto err;
	}

	memset(&arg, 0, sizeof(arg));
	memcpy(arg.uuid, svp_uuid.b, TEE_IOCTL_UUID_LEN);
	arg.clnt_login = TEE_IOCTL_LOGIN_PUBLIC;
	arg.num_params = TEE_NUM_PARAM;
	rtd_pr_vdec_debug("arg uuid %pUl \n", arg.uuid);

	memset(&param[0], 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
	param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
	param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
	param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
	param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

	rc = tee_client_open_session(svp_ta.ctx, &arg, &param[0]);
	if (rc) {
		rtd_pr_vdec_err("rtkvdec_svp: open_session failed ret %x arg %x", rc, arg.ret);
		ret = -EINVAL;
		goto err;
	}
	if (arg.ret) {
		ret = -EINVAL;
		goto err;
	}
	ret = 0;
	svp_ta.session = arg.session;

	rtd_pr_vdec_info("rtkvdec_svp: open_session ok\n");
	return ret;

err:
	if (svp_ta.session) {
		down(&Mprison_Semaphore);
		tee_client_close_session(svp_ta.ctx, svp_ta.session);
		svp_ta.session = 0;
		up(&Mprison_Semaphore);
		rtd_pr_vdec_err("rtkvdec_svp: open failed close session \n");
	}
	if (svp_ta.ctx) {
		tee_client_close_context(svp_ta.ctx);
		rtd_pr_vdec_err("rtkvdec_svp: open failed close context\n");
		svp_ta.ctx = NULL;
	}
	rtd_pr_vdec_err("open_session fail\n");

	return ret;
}


typedef struct
{
    char name[20];
    unsigned int  mem_layout_index;
    unsigned int  svp_base;
    unsigned int  svp_size;
    unsigned int  tzram_base;
    unsigned int  tzram_size;
}svp_mem_layout_t;

int rtkvdec_svp_laycout_sync_tee()
{
	unsigned long vaddr = 0, tzaddr ;
	unsigned int  vsize = 0 , tzsize;
	int ret, rc;
	svp_mem_layout_t layout = {};
  	struct tee_param param[TEE_NUM_PARAM];
  	struct tee_ioctl_invoke_arg arg;

        struct tee_shm *shm_buffer = NULL;
        int shm_len = 0;
        //phys_addr_t shm_pa = 0;
        void *shm_va = 0;

	vsize = (unsigned int)carvedout_buf_query(CARVEDOUT_VDEC_RINGBUF, (void *)&vaddr);
	tzsize = (unsigned int)carvedout_buf_query(CARVEDOUT_K_OS, (void *)&tzaddr);

	memset(&layout, 0x0, sizeof(layout));
	layout.svp_base = vaddr;
	layout.svp_size = vsize;
	layout.tzram_base = tzaddr;
	layout.tzram_size = tzsize;
	layout.mem_layout_index = carvedout_buf_get_layout_idx();

	ret = rtkvdec_svp_init();
	if(ret != 0) {
		rtd_pr_vdec_err("rtkvdec_svp: no ta context\n");
		goto out;
	}

	rtd_pr_vdec_err("index=%d svp_base=%x, svp_size=%x, tzbase=%x tzsize=%x\n", layout.mem_layout_index,
			layout.svp_base, layout.svp_size, layout.tzram_base, layout.tzram_size);

	// alloc share memory
	shm_len = sizeof(svp_mem_layout_t);
	shm_buffer = tee_shm_alloc(svp_ta.ctx, shm_len, TEE_SHM_MAPPED | TEE_SHM_DMA_BUF);
	if (shm_buffer == NULL) {
		rtd_pr_vdec_err("%s %d svp layut: no shm_buffer\n", __func__, __LINE__);
		ret = -ENOMEM;
		goto out;
	}
	// get share memory virtual addr for data accessing
	shm_va = tee_shm_get_va(shm_buffer, 0);
	if (shm_va == NULL) {
		rtd_pr_vdec_err("%s %d get va fail\n", __func__, __LINE__);
		ret = -ENOMEM;
		goto out;
	}

        memset(&arg, 0, sizeof(arg));
        arg.func = MEMPRT_CMD_SetMemLayoutIndex;
        arg.session = svp_ta.session;
        arg.num_params = TEE_NUM_PARAM;


	memcpy((unsigned int *)shm_va, &layout, shm_len);

	memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
	param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
	param[0].u.memref.shm = shm_buffer;
	param[0].u.memref.size = shm_len;
	param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
	param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
	param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

	rc = tee_client_invoke_func(svp_ta.ctx, &arg, param);
	if (rc || arg.ret) {
		rtd_pr_vdec_err("optee_meminfo carvedout query: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
		ret = -EINVAL;
		if(arg.ret == 0xFFFF0009)
		{
			panic("\e[1;31m CARVEDOUT_K_OS size not sync!!, please check optee_os_prebuilt!\e[0m");
		}
		goto out;
	}

out:
	if (shm_buffer)
	{
		tee_shm_free(shm_buffer);
	}
	return 0;
}


int rtkvdec_svp_early_init(void)
{       
//2851c has not IB, no need call rtk_rmm_update_dcib_config  
#if 0
        int size;
        void *buf;
        int ret;
        
        size = sizeof(ib_region_aligned_info_t)*SW_IB_REGIONS_NUM;
        buf = vmalloc(size);
        if(buf == NULL)
        {       
                rtd_pr_vdec_err("[SVP]: failed vamlloc for dcib config\n");
                return 0;
        }               
        ret = rtkvdec_get_dcib_config(buf, size);
        if(!ret){
                rtk_rmm_update_dcib_config((ib_region_aligned_info_t *)buf, SW_IB_REGIONS_NUM);
        }
        else {  
                rtd_pr_vdec_err("[SVP]: Update to rmm dcib config Failed\n");
                return -1;
        }
        vfree(buf);
#endif
        return 0;
}

int rtkvdec_svp_dymanic_cma_protection(int type, uint64_t start, uint64_t size)
{
	int ret = -1, rc = 0;
	struct tee_ioctl_invoke_arg arg;
	struct tee_param param[TEE_NUM_PARAM];
	if (svp_ta.ctx == NULL && svp_ta.session == 0) {
		ret = rtkvdec_svp_init();
		if (ret != 0) {
			rtd_pr_vdec_err("rtkvdec_svp: no ta context\n");
			goto out;
		}
	}

	memset(&arg, 0, sizeof(arg));
	arg.func = MEMPRT_CMD_Set_Display_Dynamic_protection;
	arg.session = svp_ta.session;
	arg.num_params = TEE_NUM_PARAM;
	memset(&param[0], 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
	param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;

	switch(type)
	{
		case TYPE_SVP_PROTECT_DYMANIC_CMA_CLEAR:
			param[0].u.value.b = 1;
			break;
		case TYPE_SVP_PROTECT_DYMAINIC_CMA_SET:
			param[0].u.value.a = 1;
			param[1].u.value.a = (start>>32)& 0xFFFFFFFF;
			param[1].u.value.b = (start)& 0xFFFFFFFF;
			param[2].u.value.a = (start>>32)& 0xFFFFFFFF;
			param[2].u.value.b = (start)& 0xFFFFFFFF;
			break;
		case TYPE_SVP_PROTECT_DYMAINIC_CMA_CANCEL:
			param[0].u.value.a = 0;
			param[1].u.value.a = (start>>32)& 0xFFFFFFFF;
			param[1].u.value.b = (start)& 0xFFFFFFFF;
			break;

	}
	down(&Mprison_Semaphore);
	rc = tee_client_invoke_func(svp_ta.ctx, &arg, &param[0]);
	up(&Mprison_Semaphore);
	if (rc || arg.ret) {
		rtd_pr_vdec_err("rtkvdev_svp: invoke failed ret %x arg.ret %x/%x\n", rc, arg.ret,
		       arg.ret_origin);
		ret = -EINVAL;
		goto out;
	} else
		ret = 0;
	rtd_pr_vdec_info("rtkvdec_svp_dymanic_cma_protection: open type=%d\n", type);
out:
		return ret;
}

int rtkvdec_svp_enable_protection(unsigned int addr, unsigned int size,
                                  unsigned int type) {
	int ret = -1, rc = 0;
	struct tee_ioctl_invoke_arg arg;
	struct tee_param param[TEE_NUM_PARAM];
	unsigned long vbm_start;
	unsigned int  vbm_size;
	int growup_ret = 0;

	if ( svp_ta.ctx == NULL && svp_ta.session == 0 ) {
		ret = rtkvdec_svp_init();
		if (ret != 0) {
			rtd_pr_vdec_err("rtkvdec_svp: no ta context\n");
			goto out;
		}
	}

	memset(&arg, 0, sizeof(arg));
	if ( type == TYPE_SVP_PROTECT_CPB )
		arg.func = MEMPRT_CMD_OpenContentProtectedBuffer;
	else if ( type == TYPE_SVP_PROTECT_COMEM ) {
		arg.func = MEMPRT_CMD_OpenDisplayProtectedCOMEM;
		goto out;
#ifdef CONFIG_RTK_KDRV_SVP_HDMI_PROTECTION
	} else if ( type  == TYPE_SVP_PROTECT_HDMI ) {
		arg.func = MEMPRT_CMD_SET_HDMI_PROTECT;
		rtkvdec_svp_cma_memory_init((unsigned long) addr, size);
#endif

	} else if ( type == TYPE_SVP_PROTECT_GPU ) {
		arg.func = MEMPRT_CMD_SET_GPU_PROTECT;
		rtkvdec_svp_cma_memory_init((unsigned long) addr, size);
	} else if ( type == TYPE_SVP_PROTECT_VBM ) {
		growup_ret = vbm_region_growup_check((unsigned long) addr, size, &vbm_start, &vbm_size);
		if ( growup_ret == -ENOSPC ) {
			rtd_pr_vdec_err("vbm region no space\n");
			goto out;
		}

		if ( !growup_ret )
		{
			if ( vbm_start != 0 && vbm_size != 0 ) {
				rtkvdec_svp_cma_memory_init(vbm_start, vbm_size);
			}
		}
		arg.func = MEMPRT_CMD_OpenDisplayProtectedVBM;
	} else {
		rtd_pr_vdec_err("rtkvdev_svp enable get incorrect type %x\n", type);
		goto out;
	}
	arg.session = svp_ta.session;
	arg.num_params = TEE_NUM_PARAM;

	memset(&param[0], 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
	param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[0].u.value.a = addr;
	param[0].u.value.b = size;
	param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
	param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
	param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

	down(&Mprison_Semaphore);
	rc = tee_client_invoke_func(svp_ta.ctx, &arg, &param[0]);
	up(&Mprison_Semaphore);
	if (rc || arg.ret) {
		rtd_pr_vdec_err("rtkvdev_svp: invoke failed ret %x arg.ret %x/%x\n", rc, arg.ret,
				arg.ret_origin);
		ret = -EINVAL;
		goto out2;
	} else
		ret = 0;
	rtd_pr_vdec_info("rtkvdev_svp: open type=%d\n", type);
	return ret;

out2:
	if(type == TYPE_SVP_PROTECT_VBM){
		vbm_region_growup_unset((unsigned long)addr, size);
		rtkvdec_svp_cma_memory_free((unsigned long)addr, size);
	}
	if( type == TYPE_SVP_PROTECT_HDMI){
		rtkvdec_svp_cma_memory_free((unsigned long)addr, size);
	}
	if( type == TYPE_SVP_PROTECT_GPU){
		rtkvdec_svp_cma_memory_free((unsigned long)addr, size);
	}
	
out:
	return ret;
}
EXPORT_SYMBOL(rtkvdec_svp_enable_protection);

int rtkvdec_svp_disable_protection(unsigned int addr, unsigned int size,
				   unsigned int type) {
	int ret = -1, rc = 0;
	struct tee_ioctl_invoke_arg arg;
	struct tee_param param[TEE_NUM_PARAM];

	if (svp_ta.ctx == NULL && svp_ta.session == 0) {
		ret = rtkvdec_svp_init();
		if (ret != 0) {
			rtd_pr_vdec_err("rtkvdec_svp: no ta context (%d)\n", __LINE__);
			goto out;
		}
	}

	if(type == TYPE_SVP_PROTECT_VBM)
	{
		if(vbm_region_growup_unset((unsigned long)addr, size) ==-1){
			rtd_pr_vdec_err("[VBM] region %x-%x disable protection before\n", addr, addr+size);
			return 0;
		}
		
	}
	memset(&arg, 0, sizeof(arg));
	if (type == TYPE_SVP_PROTECT_CPB)
		arg.func = MEMPRT_CMD_CloseContentProtectedBuffer;
	else if (type == TYPE_SVP_PROTECT_COMEM) {
		arg.func = MEMPRT_CMD_CloseDisplayProtectedCOMEM;
		goto out;
#ifdef CONFIG_RTK_KDRV_SVP_HDMI_PROTECTION
	} else if (type == TYPE_SVP_PROTECT_HDMI) {
		arg.func = MEMPRT_CMD_CANCEL_HDMI_PROTECT;
#endif
    } else if (type == TYPE_SVP_PROTECT_GPU) {
		arg.func = MEMPRT_CMD_CANCEL_GPU_PROTECT;
    } else if (type == TYPE_SVP_PROTECT_VBM) {
       arg.func = MEMPRT_CMD_CloseDisplayProtectedVBM;
    } else {
            rtd_pr_vdec_err("rtkvdev_svp disable get incorrect type %x\n", type);
            goto out;
    }

	arg.session = svp_ta.session;
	arg.num_params = TEE_NUM_PARAM;

	memset(&param[0], 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
	param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[0].u.value.a = addr;
	param[0].u.value.b = size;
	param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
	param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
	param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

	down(&Mprison_Semaphore);
	rc = tee_client_invoke_func(svp_ta.ctx, &arg, &param[0]);
	up(&Mprison_Semaphore);
	if (rc || arg.ret) {
		rtd_pr_vdec_err("rtkvdev_svp-close: invoke failed ret %x arg.ret %x/%x\n", rc, arg.ret,
				arg.ret_origin);
		ret = -EINVAL;
		goto out;
	} else
		ret = 0;
	rtd_pr_vdec_info("rtkvdev_svp: close type=%d\n", type);
#ifdef CONFIG_RTK_KDRV_SVP_HDMI_PROTECTION
	if(type == TYPE_SVP_PROTECT_HDMI )
		rtkvdec_svp_cma_memory_free( (unsigned long )addr, size );
#endif
	
	if(type == TYPE_SVP_PROTECT_GPU){
		rtkvdec_svp_cma_memory_free( (unsigned long )addr, size );
	}

	if(type == TYPE_SVP_PROTECT_VBM){
		vbm_region_growup_unset((unsigned long)addr, size);
		rtkvdec_svp_cma_memory_free((unsigned long)addr, size);
	}

out:
	return ret;
}

EXPORT_SYMBOL(rtkvdec_svp_disable_protection);

static __maybe_unused void svp_set_enabled(int enableStatus) {
	svp_display_is_enabled = enableStatus;
}

int __maybe_unused svp_get_display_is_enabled(void) {
	rtd_pr_vdec_debug("rtkvdev_svp: svp(scaler type)_is_enable=%x\n",
			  svp_display_is_enabled);
	return svp_display_is_enabled;
}

#else // CONFIG_RTK_KDRV_TEE

int rtkvdec_svp_enable_cpb_protection(unsigned int addr, unsigned int size) {
	return -1;
}

int rtkvdec_svp_disable_cpb_protection(unsigned int addr, unsigned int size) {
	return -1;
}

int svp_get_display_is_enabled(void) { return 0; }

#endif // CONFIG_RTK_KDRV_TEE
