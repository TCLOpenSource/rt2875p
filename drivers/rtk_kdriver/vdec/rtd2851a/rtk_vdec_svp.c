#if IS_ENABLED(CONFIG_RTK_KDRV_TEE)


#include "rtk_vdec_svp.h"


#define CFG_RTK_CMA_MAP 0

#include <linux/kernel.h>
#include <linux/tee.h>
#include <linux/tee_drv.h>
#include <linux/err.h>
#include <linux/mm.h>
#include <linux/pageremap.h>
#include <linux/vmalloc.h>
#include <mach/rtk_platform.h>
#include <rtd_log/rtd_module_log.h>


////////////////////////////////////////////////////////////////////////////////
#define TEE_NUM_PARAM 4

////////////////////////////////////////////////////////////////////////////////

static const uuid_t svp_uuid =
	UUID_INIT(0x15c66c21, 0x1275, 0x4040,
							0xaa, 0x61, 0xfb, 0x1e, 0xe4, 0x56, 0xda, 0xca);


#define TA_RTK_SVP_CMD_OpenContentProtectedBuffer          1000
#define TA_RTK_SVP_CMD_CloseContentProtectedBuffer         1001

#define MEMPRT_CMD_SetMemLayoutIndex                       3333

struct semaphore Mprison_Semaphore = __SEMAPHORE_INITIALIZER(Mprison_Semaphore, 1);

struct optee_ta {
    struct tee_context *ctx;
    __u32 session;
};

static struct optee_ta svp_ta;

static int optee_svp_match(struct tee_ioctl_version_data *data, const void *vers)
{
        return 1;
}

static int rtkvdec_svp_init (void)
{
  int ret = 0, rc = 0;
  struct tee_param param[TEE_NUM_PARAM];
  struct tee_ioctl_open_session_arg arg;
  struct tee_ioctl_version_data vers = {
      .impl_id = TEE_IMPL_ID_OPTEE,
      .impl_caps = TEE_OPTEE_CAP_TZ,
      .gen_caps = TEE_GEN_CAP_GP,
  };

  memset (&svp_ta, 0, sizeof(svp_ta));
  svp_ta.ctx = tee_client_open_context (NULL, optee_svp_match, NULL, &vers);
  if (svp_ta.ctx == NULL)
  {
    rtd_pr_vdec_err ("rtkvdec_svp: no ta context\n");
    ret = -EINVAL;
    goto err;
  }

  memset(&arg, 0, sizeof(arg));
  memcpy(&arg.uuid, svp_uuid.b, TEE_IOCTL_UUID_LEN);
  arg.clnt_login = TEE_IOCTL_LOGIN_PUBLIC;
  arg.num_params = TEE_NUM_PARAM;
  rtd_pr_vdec_debug("arg uuid %pUl \n", arg.uuid);

  memset(&param[0], 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
  param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
  param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
  param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
  param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;


  rc = tee_client_open_session(svp_ta.ctx, &arg, &param[0]);
  if(rc) {
      rtd_pr_vdec_err("rtkvdec_svp: open_session failed ret %x arg %x", rc, arg.ret);
      ret = -EINVAL;
      goto err;
  }
  if (arg.ret) {
    ret = -EINVAL;
    goto err;
  }

  svp_ta.session = arg.session;

  rtd_pr_vdec_debug("open_session ok\n");
  return 0;

err:
  if (svp_ta.session) {
    tee_client_close_session(svp_ta.ctx, svp_ta.session);
    rtd_pr_vdec_err("rtkvdec_svp: open failed close session \n");
    svp_ta.session = 0;
  }
  if (svp_ta.ctx) {
    tee_client_close_context(svp_ta.ctx);
    rtd_pr_vdec_err("rtkvdec_svp: open failed close context\n");
    svp_ta.ctx = NULL;
  }
  rtd_pr_vdec_err("open_session fail\n");

  return ret;
}

static void rtkvdec_svp_deinit(void)
{
  if (svp_ta.session) {
    tee_client_close_session(svp_ta.ctx, svp_ta.session);
    svp_ta.session = 0;
  }

  if (svp_ta.ctx) {
    tee_client_close_context(svp_ta.ctx);
    svp_ta.ctx = NULL;
  }
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

        rtd_pr_vdec_err("layout index=%d svp_base=%x, svp_size=%x tzbase=%x tzsize=%x\n", layout.mem_layout_index,
                        layout.svp_base, layout.svp_size, layout.tzram_base, layout.tzram_size );

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




int
rtkvdec_svp_enable_cpb_protection (unsigned int addr, unsigned int size)
{
  int ret = 0, rc = 0;
  struct tee_ioctl_invoke_arg arg;
  struct tee_param param[TEE_NUM_PARAM];

  ret = rtkvdec_svp_init();
  if(ret != 0) {
    rtd_pr_vdec_err("rtkvdec_svp: no ta context\n");
    goto out;
  }

  memset(&arg, 0, sizeof(arg));
  arg.func = TA_RTK_SVP_CMD_OpenContentProtectedBuffer;
  arg.session = svp_ta.session;
  arg.num_params = TEE_NUM_PARAM;

  memset(&param[0], 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
  param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
  param[0].u.value.a = addr;
  param[0].u.value.b = size;
  param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
  param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
  param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

  rc = tee_client_invoke_func(svp_ta.ctx, &arg, &param[0]);
  if (rc || arg.ret) {
      rtd_pr_vdec_err("svp-open: invoke failed ret %x arg.ret %x/%x\n", rc, arg.ret,
             arg.ret_origin);
      ret = -EINVAL;
      goto out;
  }

out:
  rtkvdec_svp_deinit();
  return ret;
}

int
rtkvdec_svp_disable_cpb_protection (unsigned int addr, unsigned int size)
{
  int ret = 0, rc = 0;
  struct tee_ioctl_invoke_arg arg;
  struct tee_param param[TEE_NUM_PARAM];

  ret = rtkvdec_svp_init();
  if(ret != 0) {
    rtd_pr_vdec_err("rtkvdec_svp: no ta context\n");
    goto out;
  }

  memset(&arg, 0, sizeof(arg));
  arg.func = TA_RTK_SVP_CMD_CloseContentProtectedBuffer;
  arg.session = svp_ta.session;
  arg.num_params = TEE_NUM_PARAM;

  memset(&param[0], 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
  param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
  param[0].u.value.a = addr;
  param[0].u.value.b = size;
  param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
  param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
  param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

  rc = tee_client_invoke_func(svp_ta.ctx, &arg, &param[0]);
  if (rc || arg.ret) {
      rtd_pr_vdec_err("svp-close: invoke failed ret %x arg.ret %x/%x\n", rc, arg.ret,
             arg.ret_origin);
      ret = -EINVAL;
      goto out;
  }

out:
  rtkvdec_svp_deinit();
  return ret;
}

#else // CONFIG_RTK_KDRV_TEE

int
rtkvdec_svp_enable_cpb_protection (unsigned int addr, unsigned int size)
{
  return -1;
}

int
rtkvdec_svp_disable_cpb_protection (unsigned int addr, unsigned int size)
{
  return -1;
}

#endif // CONFIG_RTK_KDRV_TEE
