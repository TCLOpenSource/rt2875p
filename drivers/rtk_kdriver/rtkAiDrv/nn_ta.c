#include <linux/delay.h>
#include <linux/tee.h>
#include <linux/uuid.h>
#include <linux/tee_drv.h>

static const uuid_t nn_uuid =
    UUID_INIT(0x7d41609a, 0x60f5, 0x455a,
        0xad, 0x88, 0x71, 0xcf, 0x83, 0x8f, 0x76, 0xc0);

struct nn_ta {
    struct tee_context *ctx;
    __u32 session;
};
static DEFINE_MUTEX(ta_mutex);
static uint8_t nonsecure_locked = 0;
static uint32_t lock_refCnt = 0;

#define NN_CMD_UPDATE_HW_LOCK_STATUS    0


static int nn_ta_match(struct tee_ioctl_version_data *data, const void *vers)
{
    return 1;
}

static void nn_ta_deinit(struct nn_ta *ta)
{
    if (ta->session) {
        tee_client_close_session(ta->ctx, ta->session);
        ta->session = 0;
    }
    if (ta->ctx) {
        tee_client_close_context(ta->ctx);
        ta->ctx = NULL;
    }
}

static int nn_ta_init(struct nn_ta *ta)
{
    int rc = 0;
    struct tee_ioctl_open_session_arg arg = {};
    struct tee_ioctl_version_data vers = {
        .impl_id = TEE_IMPL_ID_OPTEE,
        .impl_caps = TEE_OPTEE_CAP_TZ,
        .gen_caps = TEE_GEN_CAP_GP,
    };


    if(ta->session != 0) {
        return 0;
    }

    ta->ctx = tee_client_open_context(NULL, nn_ta_match, NULL, &vers);
    if(ta->ctx == NULL) {
        printk(KERN_EMERG "[AI] open tee/context failed\n");
        goto err;
    }


    memcpy(arg.uuid, nn_uuid.b, TEE_IOCTL_UUID_LEN);
    arg.clnt_login = TEE_IOCTL_LOGIN_PUBLIC;
    rc = tee_client_open_session(ta->ctx, &arg, NULL);
    if(rc || arg.ret) {
        printk(KERN_EMERG "[AI] open tee/session failed. rc=0x%x, arg=0x%x\n", rc, arg.ret);
        goto err;
    }

    ta->session = arg.session;
    return 0;

err:
    nn_ta_deinit(ta);
    printk(KERN_EMERG "[AI] nn ta: open failed\n");
    return -EINVAL;
}

static int nn_ta_update_hw_status(bool lock)
{
    int rc = 0;
    struct tee_ioctl_invoke_arg arg = {};
    struct tee_param param[4] = {};
    struct nn_ta ta = {};
    
    if(nn_ta_init(&ta) != 0)
        return -EINVAL;


    arg.func = NN_CMD_UPDATE_HW_LOCK_STATUS;
    arg.session = ta.session;
    arg.num_params = 4;
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
    param[0].u.value.a = lock;
    param[0].u.value.b = 0; //non-secure

    rc = tee_client_invoke_func(ta.ctx, &arg, param);
    nn_ta_deinit(&ta);
    if (rc || arg.ret)
    {
        printk(KERN_EMERG "[AI] invoke func failed\n");
        return -EINVAL;
    }
    
    nonsecure_locked = lock;
    return 0;
}

extern int scalerAI_enableLowPowerMode(int enable);
int nn_ta_lock_npu(bool lock)
{
    int ret = 0;

    mutex_lock(&ta_mutex);
    if(nonsecure_locked)
    {
        if(lock)
        {
            lock_refCnt++;
        }
        else if(--lock_refCnt == 0)
        {
            scalerAI_enableLowPowerMode(1);
            ret = nn_ta_update_hw_status(lock);
        }
    }
    else if(lock)
    {
        int retry = 10;
        do {
            ret = nn_ta_update_hw_status(lock);
            if(ret == 0)
            {
                lock_refCnt = 1;
                scalerAI_enableLowPowerMode(0);
                break;
            }
            msleep(10);
        } while (ret < 0 && --retry > 0);
    }

      
    printk(KERN_EMERG "[AI] lock_refCnt=%d\n", lock_refCnt);
    printk(KERN_EMERG "[AI] %s secure mode\n", nonsecure_locked ? "lock": "unlock");
    mutex_unlock(&ta_mutex);
    return ret;
}
EXPORT_SYMBOL(nn_ta_lock_npu);
