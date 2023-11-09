#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/pageremap.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/string.h>

#include <base_types.h>
#include <tp/tp_def.h>
#include <tp/tp_drv_api.h>

#define CFG_RTK_CMA_MAP 0

#include <linux/kernel.h>
#include <linux/tee.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/uuid.h>
#include <linux/tee_drv.h>
#include <linux/uuid.h>
#include <rbus/timer_reg.h>
#include <tp/tp_dbg.h>
#include <tp/tp_ta.h>

#define TEE_NUM_PARAM 4

static const uuid_t tp_uuid = 
	UUID_INIT(0xcec1de78, 0x5d23, 0x4cfc,
		0x97, 0x9e, 0x80, 0xbf, 0x70, 0x11, 0xf7, 0xcb);

#define TP_CMD_WRITE_REGISTER           0
#define TP_CMD_READ_REGISTER            1
#define TP_CMD_SET_CW                   2
#define TP_CMD_SET_ECP                  3
#define TP_CMD_GET_SECTION_DATA         4
#define TP_CMD_TEST_01                  5
#define TP_CMD_TEST_02                  6

struct optee_ta tp_ta;

struct tee_shm *tp_shm_buffer = NULL;
int tp_shm_len = 0;
phys_addr_t tp_shm_pa = 0;
char *tp_shm_va = 0;
int g_tpta_init_cnt = 0;

static int optee_tp_match(struct tee_ioctl_version_data *data, const void *vers)
{
        return 1;
}


/* 0: success, -N: failure (N: value) */
int optee_tp_init(void)
{
        int ret = 0, rc = 0;
        struct tee_ioctl_open_session_arg arg;
        struct tee_ioctl_version_data vers = {
                .impl_id = TEE_IMPL_ID_OPTEE,
                .impl_caps = TEE_OPTEE_CAP_TZ,
                .gen_caps = TEE_GEN_CAP_GP,
        };

        g_tpta_init_cnt++;
        if(tp_ta.session != 0) {
                return 0;
        }

        memset(&tp_ta, 0, sizeof(tp_ta));
        TP_DBG("TP call tee_client_open_context 1\n");
        tp_ta.ctx = tee_client_open_context(NULL, optee_tp_match, NULL, &vers);
        TP_DBG("TP call tee_client_open_context 2\n");
        if(tp_ta.ctx == NULL) {
                TP_ERROR("optee_tp: no ta context\n");
                ret = -EINVAL;
                goto err;
        }

        memset(&arg, 0, sizeof(arg));
        memcpy(arg.uuid, tp_uuid.b, TEE_IOCTL_UUID_LEN);
        arg.clnt_login = TEE_IOCTL_LOGIN_PUBLIC;
        TP_DBG("arg uuid %pUl \n", arg.uuid);

        rc = tee_client_open_session(tp_ta.ctx, &arg, NULL);
        if(rc) {
                TP_ERROR("optee_tp: open_session failed ret %x arg %x", rc, arg.ret);
                ret = -EINVAL;
                goto err;
        }
        if (arg.ret) {
                ret = -EINVAL;
                goto err;
        }

        tp_ta.session = arg.session;

        TP_DBG("open_session ok\n");

        // alloc share memory
        tp_shm_len = 256;
        tp_shm_buffer = tee_shm_alloc(tp_ta.ctx, tp_shm_len, TEE_SHM_MAPPED);
        if (tp_shm_buffer != NULL) {
                // get share memory virtual addr for data accessing
                tp_shm_va = (char*)tee_shm_get_va(tp_shm_buffer, 0);
                rc = tee_shm_get_pa(tp_shm_buffer, 0, &tp_shm_pa);
        } else {
                TP_ERROR("optee_tp: no shm_buffer\n");
        }
        return 0;

err:
        if (tp_ta.session) {
                tee_client_close_session(tp_ta.ctx, tp_ta.session);
                TP_ERROR("optee_tp: open failed close session \n");
                tp_ta.session = 0;
        }
        if (tp_ta.ctx) {
                tee_client_close_context(tp_ta.ctx);
                TP_ERROR("optee_tp: open failed close context\n");
                tp_ta.ctx = NULL;
        }
        TP_ERROR("open_session fail\n");

        TP_DBG("%s %d\n", __func__, __LINE__);

        return ret;
}


void optee_tp_deinit(void)
{
        if(--g_tpta_init_cnt != 0)
                return ;
        mdelay(10);

        if (tp_shm_buffer){
                tee_shm_free(tp_shm_buffer);
                tp_shm_buffer = NULL;
        }

        if (tp_ta.session) {
                tee_client_close_session(tp_ta.ctx, tp_ta.session);
                tp_ta.session = 0;
        }

        if (tp_ta.ctx) {
                tee_client_close_context(tp_ta.ctx);
                tp_ta.ctx = NULL;
        }
}

#if 0
/* 0: success, -N: failure (N: value) */
static int optee_tp_run(void)
{
        int ret = 0, rc = 0;
        struct tee_ioctl_invoke_arg arg;
        struct tee_param *param = NULL;
        struct tee_shm *shm_buffer = NULL;
        int shm_len = 0;
        phys_addr_t shm_pa = 0;
        void *shm_va = 0;
        UINT32 pbuff_phy = 0;
        UINT8 *pbuff;
        UINT8 *pbuff_noncache;

        TP_ERROR("get from optee\n");

        if(tp_ta.ctx == NULL) {
                TP_ERROR("optee_meminfo: no ta context\n");
                ret = -EINVAL;
                goto out;
        }

        param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);

        memset(&arg, 0, sizeof(arg));
        arg.func = TP_CMD_WRITE_REGISTER;
        arg.session = tp_ta.session;
        arg.num_params = TEE_NUM_PARAM;

        // alloc share memory
        shm_len = 256;
        shm_buffer = tee_shm_alloc(tp_ta.ctx, shm_len, TEE_SHM_MAPPED);
        if (shm_buffer == NULL) {
                TP_ERROR("optee_tp: no shm_buffer\n");
                ret = -ENOMEM;
                goto out;
        }

        // get share memory virtual addr for data accessing
        shm_va = tee_shm_get_va(shm_buffer, 0);
        if (shm_va == NULL) {
                ret = -ENOMEM;
                goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_buffer, 0, &shm_pa);
        if (rc) {
                ret = -ENOMEM;
                goto out;
        }

        TP_ERROR("shm_buffer=%p, shm_len=%d, va=%p, pa=%llx\n", (void *)shm_buffer, shm_len, shm_va, shm_pa);
        pbuff = (UINT8 *)dvr_malloc_uncached_specific(4096, GFP_DCU1_FIRST, (void **)&pbuff_noncache);

        if (NULL != pbuff) {
                /* check phy secBuff */
                pbuff_phy = dvr_to_phys((void*)pbuff);
                if ( pbuff_phy == 0 ) {
                        TP_ERROR( "error, virt addr to phy addr fail\n" );
                        return TPK_NOT_ENOUGH_RESOURCE;

                }
        }
        pbuff_noncache[0] = 'R';
        pbuff_noncache[1] = 'E';
        pbuff_noncache[2] = 'E';
        pbuff_noncache[3] = 0x00;

        memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
        param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
        param[0].u.value.a = 3; // functionID
        param[0].u.value.b = 0;
        param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
        param[1].u.memref.shm = shm_buffer;
        param[1].u.memref.size = shm_len;
        param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
        param[2].u.value.a = pbuff_phy; // physical
        param[2].u.value.b = 4096;// Size
        param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

        //TP_ERROR("dvr memory %s\n", pbuff_noncache);
        rc = tee_client_invoke_func(tp_ta.ctx, &arg, param);
        if (rc || arg.ret) {
                TP_ERROR("optee_meminfo: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
                ret = -EINVAL;
                goto out;
        }

        //TP_ERROR("%s %d share memory = %s\n",__func__, __LINE__, (char*)shm_va);
        //TP_ERROR("dvr memory = %s\n", pbuff_noncache);
        dvr_free(pbuff);
out:
        if (param)
                kfree(param);

        if (shm_buffer)
                tee_shm_free(shm_buffer);

        if (ret)
                return ret;
        else
                return 0;
}
#endif

/* 0: success, -N: failure (N: value) */
static int optee_tp_set_cw(void)
{
        int ret = 0, rc = 0;
        struct tee_ioctl_invoke_arg arg;
        struct tee_param *param = NULL;
        struct tee_shm *shm_buffer = NULL;
        int shm_len = 0;
        char *shm_va;

        if(tp_ta.ctx == NULL || g_tpta_init_cnt == 0) {
                TP_ERROR("optee_meminfo: no ta context\n");
                ret = -EINVAL;
                goto out;
        }

        param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
        if(param == NULL) {
            TP_ERROR("optee_tp_set_cw malloc param fail\n");
            ret = -EINVAL;
            goto out;
        }
        memset(&arg, 0, sizeof(arg));
        arg.func = TP_CMD_SET_CW;
        arg.session = tp_ta.session;
        arg.num_params = TEE_NUM_PARAM;

        shm_len = 256;
        shm_buffer = tee_shm_alloc(tp_ta.ctx, shm_len, TEE_SHM_MAPPED);
        if (shm_buffer == NULL) {
                TP_ERROR("optee_tp: no shm_buffer\n");
                ret = -ENOMEM;
                goto out;
        }

        // get share memory virtual addr for data accessing

        shm_va = (char*)tee_shm_get_va(shm_buffer, 0);
        if (shm_va == NULL) {
                ret = -ENOMEM;
                goto out;
        }

        shm_va[0] = 0x30;
        shm_va[1] = 0x31;
        shm_va[2] = 0x32;

        memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
        param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
        param[0].u.value.a = 0; // ChannelA
        param[0].u.value.b = 0; // TP_SETCW_KEYID_EVEN_KEY
        param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
        param[1].u.value.a = 10; // TPK_DESCRAMBLE_ALGO_AES_128_CBC_OFB
        param[1].u.value.b = 0;
        param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
        param[2].u.memref.shm = shm_buffer;
        param[2].u.memref.size = 32;
        param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
        param[3].u.memref.shm = shm_buffer;
        param[3].u.memref.size = 32;

        rc = tee_client_invoke_func(tp_ta.ctx, &arg, param);
        if (rc || arg.ret) {
                TP_ERROR("[%s] optee_meminfo: invoke failed ret %x arg.ret %x\n", __func__, rc, arg.ret);
                ret = -EINVAL;
                goto out;
        }

out:
        if (shm_buffer)
                tee_shm_free(shm_buffer);

        if (param)
                kfree(param);

        if (ret)
                return ret;
        else
                return 0;
}

int optee_tp_write_register(UINT32 addr, UINT32 val)
{
        int ret = 0, rc = 0;
        struct tee_ioctl_invoke_arg arg;
        struct tee_param param[TEE_NUM_PARAM];

        if(tp_ta.session == 0x00 || g_tpta_init_cnt == 0) {
                //TP_ERROR("optee_meminfo: no ta context\n");
                ret = -EINVAL;
                rtd_outl(addr, val);
                goto out;
        }

        memset(&arg, 0, sizeof(arg));
        arg.func = TP_CMD_WRITE_REGISTER;
        arg.session = tp_ta.session;
        arg.num_params = TEE_NUM_PARAM;

        memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
        param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
        param[0].u.value.a = addr;
        param[0].u.value.b = val;
        param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
        param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
        param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

        rc = tee_client_invoke_func(tp_ta.ctx, &arg, param);
        if (rc || arg.ret) {
                TP_ERROR("[%s] optee_meminfo: invoke failed ret %x arg.ret %x\n", __func__, rc, arg.ret);
                ret = -EINVAL;
                goto out;
        }

out:
        if (ret)
                return ret;
        else
                return 0;
}

int optee_tp_read_register(UINT32 addr, UINT32 *val)
{
        int ret = 0, rc = 0;
        struct tee_ioctl_invoke_arg arg;
        struct tee_param param[TEE_NUM_PARAM];

        if(tp_ta.session == 0x00 || g_tpta_init_cnt == 0) {
                //TP_ERROR("optee_meminfo: no ta context\n");
                ret = -EINVAL;
                *val = rtd_inl(addr);
                goto out;
        }

        memset(&arg, 0, sizeof(arg));
        arg.func = TP_CMD_READ_REGISTER;
        arg.session = tp_ta.session;
        arg.num_params = TEE_NUM_PARAM;

        memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
        param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INOUT;
        param[0].u.value.a = addr;
        param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
        param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
        param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

        rc = tee_client_invoke_func(tp_ta.ctx, &arg, param);
        if (rc || arg.ret) {
                TP_ERROR("[%s] optee_meminfo: invoke failed ret %x arg.ret %x\n", __func__, rc, arg.ret);
                ret = -EINVAL;
                goto out;
        }
        *val = param[0].u.value.b;

out:
        if (ret)
                return ret;
        else
                return 0;
}

int optee_tp_get_section_data(UINT32 addr, UINT8 size)
{
        int ret = 0, rc = 0;
        struct tee_ioctl_invoke_arg arg;
        struct tee_param param[TEE_NUM_PARAM];

        if(tp_ta.ctx == NULL || g_tpta_init_cnt == 0) {
                //TP_ERROR("optee_meminfo: no ta context\n");
                ret = -EINVAL;
                goto out;
        }

        memset(&arg, 0, sizeof(arg));
        arg.func = TP_CMD_GET_SECTION_DATA;
        arg.session = tp_ta.session;
        arg.num_params = TEE_NUM_PARAM;

        memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
        param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
        param[0].u.value.a = addr;
        param[0].u.value.b = size;
        param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
        param[1].u.memref.shm = tp_shm_buffer;
        param[1].u.memref.size = size;
        param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
        param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

        rc = tee_client_invoke_func(tp_ta.ctx, &arg, param);
        if (rc || arg.ret) {
                TP_ERROR("[%s] optee_meminfo: invoke failed ret %x arg.ret %x\n", __func__, rc, arg.ret);
                ret = -EINVAL;
                goto out;
        }
out:
        if (ret)
                return ret;
        else
                return 0;
}

int call_tp_ta(void)
{
        UINT32 time_90k_01;
        UINT32 time_90k_02;
        UINT32 time_90k_03;
        UINT32 time_90k_04;

        time_90k_01 = READ_REG32(TIMER_SCPU_CLK90K_LO_reg);
        if (optee_tp_init())
                return 0;
        time_90k_02 = READ_REG32(TIMER_SCPU_CLK90K_LO_reg);
        //if (optee_tp_run())
        //      return 0;
        if (optee_tp_set_cw())
                return 0;
        time_90k_03 = READ_REG32(TIMER_SCPU_CLK90K_LO_reg);
        optee_tp_deinit();
        time_90k_04 = READ_REG32(TIMER_SCPU_CLK90K_LO_reg);

        TP_ERROR("time_90k_01 = %d time_90k_02 = %d time_90k_03 = %d time_90k_04 = %d \n", time_90k_01, time_90k_02, time_90k_03, time_90k_04);

        return 0;
}

INT32 RHAL_TP_TA_set_mem_prison(UINT32 startAddr, UINT32 endAddr, TP_TA_MEMORY_PRISON_TYPE_T memoryType, UINT8 isEnable)
{
        int ret = 0, rc = 0;
        struct tee_ioctl_invoke_arg arg;
        struct tee_param param[TEE_NUM_PARAM];

        if(tp_ta.ctx == NULL || g_tpta_init_cnt == 0) {
                TP_ERROR("[RHAL_TP_TA_set_mem_prison] optee_meminfo: no ta context\n");
                ret = -EINVAL;
                goto out;
        }

        memset(&arg, 0, sizeof(arg));
        arg.func = TP_CMD_SET_ECP;
        arg.session = tp_ta.session;
        arg.num_params = TEE_NUM_PARAM;

        memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
        param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
        param[0].u.value.a = startAddr;
        param[0].u.value.b = endAddr;
        param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
        param[1].u.value.a = memoryType;
        param[1].u.value.b = isEnable;
        param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
        param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

        rc = tee_client_invoke_func(tp_ta.ctx, &arg, param);
        if (rc || arg.ret) {
                TP_ERROR("[%s] optee_meminfo: invoke failed ret %x arg.ret %x\n", __func__, rc, arg.ret);
                ret = -EINVAL;
                goto out;
        }
out:
        if (ret)
                return ret;
        else
                return 0;
        return 0;
}

unsigned char *test_buffer = 0;
unsigned char *test_buffer_noncache = 0;

int optee_tp_test_01(UINT32 para1, UINT32 para2)
{
        int ret = 0, rc = 0;
        struct tee_ioctl_invoke_arg arg;
        struct tee_param param[TEE_NUM_PARAM];
        UINT32 physical;

        para1 = 0;
        para2 = 0;

        if(tp_ta.ctx == NULL) {
                //TP_ERROR("optee_meminfo: no ta context\n");
                ret = -EINVAL;
                goto out;
        }

        memset(&arg, 0, sizeof(arg));
        arg.func = TP_CMD_TEST_01;
        arg.session = tp_ta.session;
        arg.num_params = TEE_NUM_PARAM;

        //test_buffer = kmalloc(4*1024, GFP_KERNEL);
        test_buffer = (UINT8 *)dvr_malloc_uncached_specific(4096, GFP_DCU1_FIRST, (void **)&test_buffer_noncache);


        physical = dvr_to_phys(test_buffer);

        memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
        param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
        param[0].u.value.a = physical;
        param[0].u.value.b = 4 * 1024;
        param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
        param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
        param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

        rc = tee_client_invoke_func(tp_ta.ctx, &arg, param);
        if (rc || arg.ret) {
                TP_ERROR("optee_meminfo: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
                ret = -EINVAL;
                goto out;
        }

out:
        if (ret)
                return ret;
        else
                return 0;
}

unsigned char last_value;
int optee_tp_test_02(UINT32 para1, UINT32 para2)
{
        int ret = 0, rc = 0;
        struct tee_ioctl_invoke_arg arg;
        struct tee_param param[TEE_NUM_PARAM];

        para1 = 0;
        para2 = 0;

        if(tp_ta.ctx == NULL) {
                //TP_ERROR("optee_meminfo: no ta context\n");
                ret = -EINVAL;
                goto out;
        }

        memset(&arg, 0, sizeof(arg));
        arg.func = TP_CMD_TEST_02;
        arg.session = tp_ta.session;
        arg.num_params = TEE_NUM_PARAM;

        memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
        param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
        param[0].u.value.a = 1;
        param[0].u.value.b = 1;
        param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
        param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
        param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

        rc = tee_client_invoke_func(tp_ta.ctx, &arg, param);
        if (rc || arg.ret) {
                TP_ERROR("optee_meminfo: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
                ret = -EINVAL;
                goto out;
        }

        if(last_value != *test_buffer_noncache) {
                TP_ERROR("last_value=0x%x data=0x%x\n", last_value, *test_buffer_noncache);
                last_value = *test_buffer_noncache;
        }
        last_value++;

out:
        if (ret)
                return ret;
        else
                return 0;
}

