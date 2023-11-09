#define CFG_RTK_CMA_MAP 0

#include <linux/kernel.h>
#include <linux/tee.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/tee_drv.h>
#include <linux/uuid.h>
#include <linux/slab.h>
#include <linux/debugfs.h>
#include <tvscalercontrol/hdcp2_2/hdcp2_messages.h>
#include <hdcp/hdcp2_optee.h>
#include <rtd_log/rtd_module_log.h>

static const uuid_t hdcp2_uuid =
            UUID_INIT(0x32f62de0, 0x131d, 0x4b51,
                      0xad, 0x51, 0x7a, 0xa5, 0x8c, 0xe4, 0x34, 0xf4);
#if 0
#define HDCP2_CMD_LOAD_KEY              0
#define HDCP2_CMD_HANDLE_MESSAGE        1
#define HDCP2_CMD_POLLING_MESSAGE       2
#endif
#define HDCP14_CMD_GENERATE_KM          0x66

#define TEE_NUM_PARAM 4
#define TEE_ALLOCATOR_DESC_LENGTH 32
#define TEE_TA_COUNT_MAX 10
#define TEEC_MEM_INPUT   0x00000001
#define TEEC_MEM_OUTPUT  0x00000002

typedef struct {
    uint32_t uuid_tL;
    uint32_t stack_size;
    uint32_t code_size;
} ta_minfo;

struct malloc_stats {
    char desc[TEE_ALLOCATOR_DESC_LENGTH];
    uint32_t allocated;               /* Bytes currently allocated */
    uint32_t max_allocated;           /* Tracks max value of allocated */
    uint32_t size;                    /* Total size for this allocator */
    uint32_t num_alloc_fail;          /* Number of failed alloc requests */
    uint32_t biggest_alloc_fail;      /* Size of biggest failed alloc */
    uint32_t biggest_alloc_fail_used; /* Alloc bytes when above occurred */
    uint32_t cur_alloc_ta_count;
    uint32_t max_alloc_ta_count;
    ta_minfo ta_minfo_cur[TEE_TA_COUNT_MAX];
    ta_minfo ta_minfo_max[TEE_TA_COUNT_MAX];
};

#if 0
#ifdef CONFIG_OPTEE_V3
//move to Tee_drv.h
#else //#ifdef CONFIG_OPTEE_V3
struct teec_uuid {
    uint32_t timeLow;
    uint16_t timeMid;
    uint16_t timeHiAndVersion;
    uint8_t clockSeqAndNode[8];
};
#endif //#ifdef CONFIG_OPTEE_V3
#endif

struct optee_ta {
    struct tee_context *ctx;
    __u32 session;
};

static struct optee_ta hdcp2_ta;

static int optee_hdcp2_match(struct tee_ioctl_version_data *data, const void *vers)
{
    return 1;
}

extern void optee_hdcp2_force_depends_on_rtktee(void);
/* 0: success, -N: failure (N: value) */
int optee_hdcp2_init(void)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_open_session_arg arg;
    struct tee_ioctl_version_data vers = {
        .impl_id = TEE_IMPL_ID_OPTEE,
        .impl_caps = TEE_OPTEE_CAP_TZ,
        .gen_caps = TEE_GEN_CAP_GP,
    };

    optee_hdcp2_force_depends_on_rtktee();

    memset(&hdcp2_ta, 0, sizeof(hdcp2_ta));
    hdcp2_ta.ctx = tee_client_open_context(NULL, optee_hdcp2_match, NULL, &vers);
    if (hdcp2_ta.ctx == NULL) {
        rtd_pr_hdcp_emerg("optee_hdcp2: no ta context\n");
        ret = -EINVAL;
        goto err;
    }

    memset(&arg, 0, sizeof(arg));
    memcpy(arg.uuid, hdcp2_uuid.b, TEE_IOCTL_UUID_LEN);
    arg.clnt_login = TEE_IOCTL_LOGIN_PUBLIC;
    rtd_pr_hdcp_debug("arg uuid %pUl \n", arg.uuid);

    rc = tee_client_open_session(hdcp2_ta.ctx, &arg, NULL);

    if (rc) {
        rtd_pr_hdcp_emerg("optee_hdcp2: open_session failed ret %x arg %x", rc, arg.ret);
        ret = -EINVAL;
        goto err;
    }
    if (arg.ret) {
        ret = -EINVAL;
        goto err;
    }

    hdcp2_ta.session = arg.session;

    rtd_pr_hdcp_debug("open_session ok\n");
    return 0;

err:
    if (hdcp2_ta.session) {
        tee_client_close_session(hdcp2_ta.ctx, hdcp2_ta.session);
        rtd_pr_hdcp_emerg("optee_hdcp2: open failed close session \n");
        hdcp2_ta.session = 0;
    }
    if (hdcp2_ta.ctx) {
        tee_client_close_context(hdcp2_ta.ctx);
        rtd_pr_hdcp_emerg("optee_hdcp2: open failed close context\n");
        hdcp2_ta.ctx = NULL;
    }
    rtd_pr_hdcp_emerg("open_session fail\n");

    return ret;
}
EXPORT_SYMBOL(optee_hdcp2_init);

void optee_hdcp2_deinit(void)
{
    if (hdcp2_ta.session) {
        tee_client_close_session(hdcp2_ta.ctx, hdcp2_ta.session);
        hdcp2_ta.session = 0;
    }

    if (hdcp2_ta.ctx) {
        tee_client_close_context(hdcp2_ta.ctx);
        hdcp2_ta.ctx = NULL;
    }
}
EXPORT_SYMBOL(optee_hdcp2_deinit);

/* 0: success, -N: failure (N: value) */
static int optee_hdcp2_handle(HDMI_DP_HDCP2_MODE_T hdmi_dp, unsigned char id, unsigned char nport, unsigned char *send_buf, unsigned int send_size, unsigned char *rev_buf, unsigned int rev_size)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;
    struct tee_shm *shm_send_buffer = NULL;
    unsigned int shm_send_len = 0;
    phys_addr_t shm_send_pa = 0;
    void *shm_send_va = 0;
    struct tee_shm *shm_rev_buffer = NULL;
    unsigned int shm_rev_len = 0;
    phys_addr_t shm_rev_pa = 0;
    void *shm_rev_va = 0;

    rtd_pr_hdcp_debug("get from optee\n");

    if (hdcp2_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp2: no ta context\n");
        ret = -EINVAL;
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_emerg("%s kcalloc param fail\n", __FUNCTION__);
        ret = -ENOMEM;
        goto out;
    }

    memset(&arg, 0, sizeof(arg));
    arg.func = id;
    arg.session = hdcp2_ta.session;
    arg.num_params = TEE_NUM_PARAM;
    if (send_buf && (send_size != 0)) {
        // alloc send share memory
        shm_send_len = send_size;
        shm_send_buffer = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len, TEE_SHM_MAPPED);
        if (shm_send_buffer == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va = tee_shm_get_va(shm_send_buffer, 0);
        if (shm_send_va == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer, 0, &shm_send_pa);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


        memcpy(shm_send_va, send_buf, send_size);
    }
    //rtd_pr_hdcp_debug("shm_buffer=%p, shm_len=%d, va=%p, pa=%x\n", (void *)shm_buffer, shm_len, shm_va, shm_pa);
    if (rev_buf && (rev_size != 0)) {
        // alloc receive share memory
        shm_rev_len = rev_size;
        shm_rev_buffer = tee_shm_alloc(hdcp2_ta.ctx, shm_rev_len, TEE_SHM_MAPPED);
        if (shm_rev_buffer == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_rev_va = tee_shm_get_va(shm_rev_buffer, 0);
        if (shm_rev_va == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_rev_buffer, 0, &shm_rev_pa);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }
    }
    /**
     *  optee user ta meminfo
     */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
    param[0].u.value.a = nport;//3; // user ta pool
    param[0].u.value.b = hdmi_dp;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[1].u.memref.shm = shm_send_buffer;
    param[1].u.memref.size = shm_send_len;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
    param[2].u.memref.shm = shm_rev_buffer;
    param[2].u.memref.size = shm_rev_len;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

    rc = tee_client_invoke_func(hdcp2_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp2: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }

    if (rev_buf) {
        memcpy(rev_buf, (unsigned char *)shm_rev_va, rev_size);
    }



out:
    if (param) {
        kfree(param);
    }

    if (shm_send_buffer) {
        tee_shm_free(shm_send_buffer);
    }

    if (shm_rev_buffer) {
        tee_shm_free(shm_rev_buffer);
    }

    if (ret) {
        return ret;
    }
    else {
        return 0;
    }
}

static int optee_hdcp2_widi_WriteKeySet_handle(unsigned char id, unsigned char *data, unsigned int size)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;
    struct tee_shm *shm_send_buffer = NULL;
    unsigned int shm_send_len = 0;
    phys_addr_t shm_send_pa = 0;
    void *shm_send_va = 0;

    rtd_pr_hdcp_debug("get from optee\n");

    if (hdcp2_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp2: no ta context\n");
        ret = -EINVAL;
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_emerg("%s kcalloc param fail\n", __FUNCTION__);
        ret = -ENOMEM;
        goto out;
    }

    memset(&arg, 0, sizeof(arg));
    arg.func = id;
    arg.session = hdcp2_ta.session;
    arg.num_params = TEE_NUM_PARAM;
    if (data && (size != 0)) {
        // alloc send share memory
        shm_send_len = size;
        shm_send_buffer = tee_shm_alloc(hdcp2_ta.ctx, size, TEE_SHM_MAPPED);
        if (shm_send_buffer == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va = tee_shm_get_va(shm_send_buffer, 0);
        if (shm_send_va == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer, 0, &shm_send_pa);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


        memcpy(shm_send_va, data, size);
    }
    //rtd_pr_hdcp_debug("shm_buffer=%p, shm_len=%d, va=%p, pa=%x\n", (void *)shm_buffer, shm_len, shm_va, shm_pa);

    /**
     *  optee user ta meminfo
     */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[0].u.memref.shm = shm_send_buffer;
    param[0].u.memref.size = shm_send_len;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

    rc = tee_client_invoke_func(hdcp2_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp2: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }

out:
    if (param) {
        kfree(param);
    }

    if (shm_send_buffer) {
        tee_shm_free(shm_send_buffer);
    }

    if (ret) {
        return ret;
    }
    else {
        return 0;
    }
}

static int optee_hdcp2_widi_GET_CERTINFO2_handle(unsigned char id, unsigned char *data1, unsigned char *data2, unsigned char *data3, unsigned char *data4)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;
    struct tee_shm *shm_send_buffer1 = NULL;
    unsigned int shm_send_len1 = 0;
    phys_addr_t shm_send_pa1 = 0;
    void *shm_send_va1 = 0;
    struct tee_shm *shm_send_buffer2 = NULL;
    unsigned int shm_send_len2 = 0;
    phys_addr_t shm_send_pa2 = 0;
    void *shm_send_va2 = 0;
    struct tee_shm *shm_send_buffer3 = NULL;
    unsigned int shm_send_len3 = 0;
    phys_addr_t shm_send_pa3 = 0;
    void *shm_send_va3 = 0;
    struct tee_shm *shm_send_buffer4 = NULL;
    unsigned int shm_send_len4 = 0;
    phys_addr_t shm_send_pa4 = 0;
    void *shm_send_va4 = 0;

    rtd_pr_hdcp_debug("get from optee\n");

    if (hdcp2_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp2: no ta context\n");
        ret = -EINVAL;
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_emerg("%s kcalloc param fail\n", __FUNCTION__);
        ret = -ENOMEM;
        goto out;
    }

    memset(&arg, 0, sizeof(arg));
    arg.func = id;
    arg.session = hdcp2_ta.session;
    arg.num_params = TEE_NUM_PARAM;
    if (data1) {
        // alloc send share memory
        shm_send_len1 = RECEIVER_ID_SIZE;
        shm_send_buffer1 = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len1, TEE_SHM_MAPPED);
        if (shm_send_buffer1 == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va1 = tee_shm_get_va(shm_send_buffer1, 0);
        if (shm_send_va1 == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer1, 0, &shm_send_pa1);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


    }
    if (data2) {
        // alloc send share memory
        shm_send_len2 = KEY_PUBLIC_RX_SIZE;
        shm_send_buffer2 = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len2, TEE_SHM_MAPPED);
        if (shm_send_buffer2 == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va2 = tee_shm_get_va(shm_send_buffer2, 0);
        if (shm_send_va2 == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer2, 0, &shm_send_pa2);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


    }
    if (data3) {
        // alloc send share memory
        shm_send_len3 = RESERVED_SIZE;
        shm_send_buffer3 = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len3, TEE_SHM_MAPPED);
        if (shm_send_buffer3 == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va3 = tee_shm_get_va(shm_send_buffer3, 0);
        if (shm_send_va3 == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer3, 0, &shm_send_pa3);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


    }
    if (data4) {
        // alloc send share memory
        shm_send_len4 = DCP_LLC_SIG_SIZE;
        shm_send_buffer4 = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len4, TEE_SHM_MAPPED);
        if (shm_send_buffer4 == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va4 = tee_shm_get_va(shm_send_buffer4, 0);
        if (shm_send_va4 == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer4, 0, &shm_send_pa4);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


    }
    //rtd_pr_hdcp_debug("shm_buffer=%p, shm_len=%d, va=%p, pa=%x\n", (void *)shm_buffer, shm_len, shm_va, shm_pa);

    /**
     *  optee user ta meminfo
     */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
    param[0].u.memref.shm = shm_send_buffer1;
    param[0].u.memref.size = shm_send_len1;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
    param[1].u.memref.shm = shm_send_buffer2;
    param[1].u.memref.size = shm_send_len2;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
    param[2].u.memref.shm = shm_send_buffer3;
    param[2].u.memref.size = shm_send_len3;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
    param[3].u.memref.shm = shm_send_buffer4;
    param[4].u.memref.size = shm_send_len4;

    rc = tee_client_invoke_func(hdcp2_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp2: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }

    if (data1) {
        memcpy(data1, shm_send_va1, shm_send_len1);
    }
    if (data2) {
        memcpy(data2, shm_send_va2, shm_send_len2);
    }
    if (data3) {
        memcpy(data3, shm_send_va3, shm_send_len3);
    }
    if (data4) {
        memcpy(data4, shm_send_va4, shm_send_len4);
    }
out:
    if (param) {
        kfree(param);
    }

    if (shm_send_buffer1) {
        tee_shm_free(shm_send_buffer1);
    }
    if (shm_send_buffer2) {
        tee_shm_free(shm_send_buffer2);
    }
    if (shm_send_buffer3) {
        tee_shm_free(shm_send_buffer3);
    }
    if (shm_send_buffer4) {
        tee_shm_free(shm_send_buffer4);
    }

    if (ret) {
        return ret;
    }
    else {
        return 0;
    }
}

static int optee_hdcp2_widi_DECRYPT_RSAES_OAEP_handle(unsigned char id, unsigned char *data, unsigned int size)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;
    struct tee_shm *shm_send_buffer = NULL;
    unsigned int shm_send_len = 0;
    phys_addr_t shm_send_pa = 0;
    void *shm_send_va = 0;

    rtd_pr_hdcp_debug("get from optee\n");

    if (hdcp2_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp2: no ta context\n");
        ret = -EINVAL;
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_emerg("%s kcalloc param fail\n", __FUNCTION__);
        ret = -ENOMEM;
        goto out;
    }

    memset(&arg, 0, sizeof(arg));
    arg.func = id;
    arg.session = hdcp2_ta.session;
    arg.num_params = TEE_NUM_PARAM;
    if (data && (size != 0)) {
        // alloc send share memory
        shm_send_len = size;
        shm_send_buffer = tee_shm_alloc(hdcp2_ta.ctx, size, TEE_SHM_MAPPED);
        if (shm_send_buffer == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va = tee_shm_get_va(shm_send_buffer, 0);
        if (shm_send_va == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer, 0, &shm_send_pa);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


        memcpy(shm_send_va, data, size);
    }
    //rtd_pr_hdcp_debug("shm_buffer=%p, shm_len=%d, va=%p, pa=%x\n", (void *)shm_buffer, shm_len, shm_va, shm_pa);

    /**
     *  optee user ta meminfo
     */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[0].u.memref.shm = shm_send_buffer;
    param[0].u.memref.size = shm_send_len;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

    rc = tee_client_invoke_func(hdcp2_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp2: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }

out:
    if (param) {
        kfree(param);
    }

    if (shm_send_buffer) {
        tee_shm_free(shm_send_buffer);
    }

    if (ret) {
        return ret;
    }
    else {
        return 0;
    }
}

static int optee_hdcp2_widi_KD_KEY_DERIVATION_handle(unsigned char id, unsigned char *rtx, unsigned char *rrx, unsigned int version)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;
    struct tee_shm *shm_send_buffer1 = NULL;
    unsigned int shm_send_len1 = 0;
    phys_addr_t shm_send_pa1 = 0;
    void *shm_send_va1 = 0;
    struct tee_shm *shm_send_buffer2 = NULL;
    unsigned int shm_send_len2 = 0;
    phys_addr_t shm_send_pa2 = 0;
    void *shm_send_va2 = 0;

    rtd_pr_hdcp_debug("get from optee\n");

    if (hdcp2_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp2: no ta context\n");
        ret = -EINVAL;
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_emerg("%s kcalloc param fail\n", __FUNCTION__);
        ret = -ENOMEM;
        goto out;
    }

    memset(&arg, 0, sizeof(arg));
    arg.func = id;
    arg.session = hdcp2_ta.session;
    arg.num_params = TEE_NUM_PARAM;
    if (rtx) {
        // alloc send share memory
        shm_send_len1 = RTX_SIZE;
        shm_send_buffer1 = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len1, TEE_SHM_MAPPED);
        if (shm_send_buffer1 == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va1 = tee_shm_get_va(shm_send_buffer1, 0);
        if (shm_send_va1 == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer1, 0, &shm_send_pa1);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


        memcpy(shm_send_va1, rtx, shm_send_len1);
    }
    if (rrx) {
        // alloc send share memory
        shm_send_len2 = RRX_SIZE;
        shm_send_buffer2 = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len2, TEE_SHM_MAPPED);
        if (shm_send_buffer2 == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va2 = tee_shm_get_va(shm_send_buffer2, 0);
        if (shm_send_va2 == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer2, 0, &shm_send_pa2);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


        memcpy(shm_send_va2, rrx, shm_send_len2);
    }
    //rtd_pr_hdcp_debug("shm_buffer=%p, shm_len=%d, va=%p, pa=%x\n", (void *)shm_buffer, shm_len, shm_va, shm_pa);

    /**
     *  optee user ta meminfo
     */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
    param[0].u.value.a = version;//3; // user ta pool
    param[0].u.value.b = 0;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[1].u.memref.shm = shm_send_buffer1;
    param[1].u.memref.size = shm_send_len1;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[2].u.memref.shm = shm_send_buffer2;
    param[2].u.memref.size = shm_send_len2;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

    rc = tee_client_invoke_func(hdcp2_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp2: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }

out:
    if (param) {
        kfree(param);
    }

    if (shm_send_buffer1) {
        tee_shm_free(shm_send_buffer1);
    }
    if (shm_send_buffer2) {
        tee_shm_free(shm_send_buffer2);
    }

    if (ret) {
        return ret;
    }
    else {
        return 0;
    }
}


static int optee_hdcp2_widi_Compute_H_Prime_handle(unsigned char id, unsigned char repeater, unsigned int version, unsigned char *rtx, unsigned char *deviceOptionInfo, unsigned char *pHPrime)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;
    struct tee_shm *shm_send_buffer1 = NULL;
    unsigned int shm_send_len1 = 0;
    phys_addr_t shm_send_pa1 = 0;
    void *shm_send_va1 = 0;
    struct tee_shm *shm_send_buffer2 = NULL;
    unsigned int shm_send_len2 = 0;
    phys_addr_t shm_send_pa2 = 0;
    void *shm_send_va2 = 0;
    struct tee_shm *shm_rev_buffer = NULL;
    unsigned int shm_rev_len = 0;
    phys_addr_t shm_rev_pa = 0;
    void *shm_rev_va = 0;

    rtd_pr_hdcp_debug("get from optee\n");

    if (hdcp2_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp2: no ta context\n");
        ret = -EINVAL;
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_emerg("%s kcalloc param fail\n", __FUNCTION__);
        ret = -ENOMEM;
        goto out;
    }

    memset(&arg, 0, sizeof(arg));
    arg.func = id;
    arg.session = hdcp2_ta.session;
    arg.num_params = TEE_NUM_PARAM;
    if (rtx) {
        // alloc send share memory
        shm_send_len1 = RTX_SIZE;
        shm_send_buffer1 = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len1, TEE_SHM_MAPPED);
        if (shm_send_buffer1 == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va1 = tee_shm_get_va(shm_send_buffer1, 0);
        if (shm_send_va1 == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer1, 0, &shm_send_pa1);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


        memcpy(shm_send_va1, rtx, shm_send_len1);
    }
    if (deviceOptionInfo) {
        // alloc send share memory
        shm_send_len2 = RXCAPS_SIZE + TXCAPS_SIZE;
        shm_send_buffer2 = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len2, TEE_SHM_MAPPED);
        if (shm_send_buffer2 == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va2 = tee_shm_get_va(shm_send_buffer2, 0);
        if (shm_send_va2 == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer2, 0, &shm_send_pa2);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


        memcpy(shm_send_va2, deviceOptionInfo, shm_send_len2);
    }

    if (pHPrime) {
        // alloc receive share memory
        shm_rev_len = H_SIZE;
        shm_rev_buffer = tee_shm_alloc(hdcp2_ta.ctx, shm_rev_len, TEE_SHM_MAPPED);
        if (shm_rev_buffer == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_rev_va = tee_shm_get_va(shm_rev_buffer, 0);
        if (shm_rev_va == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_rev_buffer, 0, &shm_rev_pa);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }
    }
    //rtd_pr_hdcp_debug("shm_buffer=%p, shm_len=%d, va=%p, pa=%x\n", (void *)shm_buffer, shm_len, shm_va, shm_pa);

    /**
     *  optee user ta meminfo
     */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
    param[0].u.value.a = repeater;
    param[0].u.value.b = version;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[1].u.memref.shm = shm_send_buffer1;
    param[1].u.memref.size = shm_send_len1;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[2].u.memref.shm = shm_send_buffer2;
    param[2].u.memref.size = shm_send_len2;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
    param[3].u.memref.shm = shm_rev_buffer;
    param[3].u.memref.size = shm_rev_len;

    rc = tee_client_invoke_func(hdcp2_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp2: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }
    if (pHPrime) {
        memcpy(pHPrime, (unsigned char *)shm_rev_va, shm_rev_len);
    }

out:
    if (param) {
        kfree(param);
    }

    if (shm_send_buffer1) {
        tee_shm_free(shm_send_buffer1);
    }
    if (shm_send_buffer2) {
        tee_shm_free(shm_send_buffer2);
    }
    if (shm_rev_buffer) {
        tee_shm_free(shm_rev_buffer);
    }

    if (ret) {
        return ret;
    }
    else {
        return 0;
    }
}

static int optee_hdcp2_widi_Compute_L_Prime_handle(unsigned char id, unsigned int version, unsigned char *rn, unsigned char *rrx, unsigned char *pLPrime)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;
    struct tee_shm *shm_send_buffer1 = NULL;
    unsigned int shm_send_len1 = 0;
    phys_addr_t shm_send_pa1 = 0;
    void *shm_send_va1 = 0;
    struct tee_shm *shm_send_buffer2 = NULL;
    unsigned int shm_send_len2 = 0;
    phys_addr_t shm_send_pa2 = 0;
    void *shm_send_va2 = 0;
    struct tee_shm *shm_rev_buffer = NULL;
    unsigned int shm_rev_len = 0;
    phys_addr_t shm_rev_pa = 0;
    void *shm_rev_va = 0;

    rtd_pr_hdcp_debug("get from optee\n");

    if (hdcp2_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp2: no ta context\n");
        ret = -EINVAL;
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_emerg("%s kcalloc param fail\n", __FUNCTION__);
        ret = -ENOMEM;
        goto out;
    }

    memset(&arg, 0, sizeof(arg));
    arg.func = id;
    arg.session = hdcp2_ta.session;
    arg.num_params = TEE_NUM_PARAM;
    if (rn) {
        // alloc send share memory
        shm_send_len1 = RN_SIZE;
        shm_send_buffer1 = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len1, TEE_SHM_MAPPED);
        if (shm_send_buffer1 == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va1 = tee_shm_get_va(shm_send_buffer1, 0);
        if (shm_send_va1 == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer1, 0, &shm_send_pa1);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


        memcpy(shm_send_va1, rn, shm_send_len1);
    }
    if (rrx) {
        // alloc send share memory
        shm_send_len2 = RRX_SIZE;
        shm_send_buffer2 = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len2, TEE_SHM_MAPPED);
        if (shm_send_buffer2 == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va2 = tee_shm_get_va(shm_send_buffer2, 0);
        if (shm_send_va2 == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer2, 0, &shm_send_pa2);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


        memcpy(shm_send_va2, rrx, shm_send_len2);
    }

    if (pLPrime) {
        // alloc receive share memory
        shm_rev_len = L_SIZE;
        shm_rev_buffer = tee_shm_alloc(hdcp2_ta.ctx, shm_rev_len, TEE_SHM_MAPPED);
        if (shm_rev_buffer == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_rev_va = tee_shm_get_va(shm_rev_buffer, 0);
        if (shm_rev_va == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_rev_buffer, 0, &shm_rev_pa);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }
    }
    //rtd_pr_hdcp_debug("shm_buffer=%p, shm_len=%d, va=%p, pa=%x\n", (void *)shm_buffer, shm_len, shm_va, shm_pa);

    /**
     *  optee user ta meminfo
     */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
    param[0].u.value.a = version;
    param[0].u.value.b = 0;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[1].u.memref.shm = shm_send_buffer1;
    param[1].u.memref.size = shm_send_len1;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[2].u.memref.shm = shm_send_buffer2;
    param[2].u.memref.size = shm_send_len2;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
    param[3].u.memref.shm = shm_rev_buffer;
    param[3].u.memref.size = shm_rev_len;

    rc = tee_client_invoke_func(hdcp2_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp2: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }
    if (pLPrime) {
        memcpy(pLPrime, (unsigned char *)shm_rev_va, shm_rev_len);
    }

out:
    if (param) {
        kfree(param);
    }

    if (shm_send_buffer1) {
        tee_shm_free(shm_send_buffer1);
    }
    if (shm_send_buffer2) {
        tee_shm_free(shm_send_buffer2);
    }
    if (shm_rev_buffer) {
        tee_shm_free(shm_rev_buffer);
    }

    if (ret) {
        return ret;
    }
    else {
        return 0;
    }
}

static int optee_hdcp2_widi_Compute_KH_handle(unsigned char id)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;

    rtd_pr_hdcp_debug("get from optee\n");

    if (hdcp2_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp2: no ta context\n");
        ret = -EINVAL;
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_emerg("%s kcalloc param fail\n", __FUNCTION__);
        ret = -ENOMEM;
        goto out;
    }

    memset(&arg, 0, sizeof(arg));
    arg.func = id;
    arg.session = hdcp2_ta.session;
    arg.num_params = TEE_NUM_PARAM;

    /**
     *  optee user ta meminfo
     */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

    rc = tee_client_invoke_func(hdcp2_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp2: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }

out:
    if (param) {
        kfree(param);
    }

    if (ret) {
        return ret;
    }
    else {
        return 0;
    }
}

static int optee_hdcp2_widi_Encrypt_Km_using_Kh_handle(unsigned char id, unsigned char *m, unsigned char *ekhkm)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;
    struct tee_shm *shm_send_buffer = NULL;
    unsigned int shm_send_len = 0;
    phys_addr_t shm_send_pa = 0;
    void *shm_send_va = 0;
    struct tee_shm *shm_rev_buffer = NULL;
    unsigned int shm_rev_len = 0;
    phys_addr_t shm_rev_pa = 0;
    void *shm_rev_va = 0;

    rtd_pr_hdcp_debug("get from optee\n");

    if (hdcp2_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp2: no ta context\n");
        ret = -EINVAL;
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_emerg("%s kcalloc param fail\n", __FUNCTION__);
        ret = -ENOMEM;
        goto out;
    }

    memset(&arg, 0, sizeof(arg));
    arg.func = id;
    arg.session = hdcp2_ta.session;
    arg.num_params = TEE_NUM_PARAM;
    if (m) {
        // alloc send share memory
        shm_send_len = M_SIZE;
        shm_send_buffer = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len, TEE_SHM_MAPPED);
        if (shm_send_buffer == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va = tee_shm_get_va(shm_send_buffer, 0);
        if (shm_send_va == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer, 0, &shm_send_pa);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


        memcpy(shm_send_va, m, shm_send_len);
    }
    //rtd_pr_hdcp_debug("shm_buffer=%p, shm_len=%d, va=%p, pa=%x\n", (void *)shm_buffer, shm_len, shm_va, shm_pa);
    if (ekhkm) {
        // alloc receive share memory
        shm_rev_len = EKHKM_SIZE;
        shm_rev_buffer = tee_shm_alloc(hdcp2_ta.ctx, shm_rev_len, TEE_SHM_MAPPED);
        if (shm_rev_buffer == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_rev_va = tee_shm_get_va(shm_rev_buffer, 0);
        if (shm_rev_va == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_rev_buffer, 0, &shm_rev_pa);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }
    }
    /**
     *  optee user ta meminfo
     */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[0].u.memref.shm = shm_send_buffer;
    param[0].u.memref.size = shm_send_len;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
    param[1].u.memref.shm = shm_rev_buffer;
    param[1].u.memref.size = shm_rev_len;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

    rc = tee_client_invoke_func(hdcp2_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp2: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }

    if (ekhkm) {
        memcpy(ekhkm, (unsigned char *)shm_rev_va, shm_rev_len);
    }



out:
    if (param) {
        kfree(param);
    }

    if (shm_send_buffer) {
        tee_shm_free(shm_send_buffer);
    }

    if (shm_rev_buffer) {
        tee_shm_free(shm_rev_buffer);
    }

    if (ret) {
        return ret;
    }
    else {
        return 0;
    }
}

static int optee_hdcp2_widi_Decrypt_Km_using_Kh_handle(unsigned char id, unsigned char *m, unsigned char *ekhkm)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;
    struct tee_shm *shm_send_buffer1 = NULL;
    unsigned int shm_send_len1 = 0;
    phys_addr_t shm_send_pa1 = 0;
    void *shm_send_va1 = 0;
    struct tee_shm *shm_send_buffer2 = NULL;
    unsigned int shm_send_len2 = 0;
    phys_addr_t shm_send_pa2 = 0;
    void *shm_send_va2 = 0;

    rtd_pr_hdcp_debug("get from optee\n");

    if (hdcp2_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp2: no ta context\n");
        ret = -EINVAL;
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_emerg("%s kcalloc param fail\n", __FUNCTION__);
        ret = -ENOMEM;
        goto out;
    }

    memset(&arg, 0, sizeof(arg));
    arg.func = id;
    arg.session = hdcp2_ta.session;
    arg.num_params = TEE_NUM_PARAM;
    if (m) {
        // alloc send share memory
        shm_send_len1 = M_SIZE;
        shm_send_buffer1 = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len1, TEE_SHM_MAPPED);
        if (shm_send_buffer1 == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va1 = tee_shm_get_va(shm_send_buffer1, 0);
        if (shm_send_va1 == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer1, 0, &shm_send_pa1);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


        memcpy(shm_send_va1, m, shm_send_len1);
    }
    if (ekhkm) {
        // alloc send share memory
        shm_send_len2 = EKHKM_SIZE;
        shm_send_buffer2 = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len2, TEE_SHM_MAPPED);
        if (shm_send_buffer2 == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va2 = tee_shm_get_va(shm_send_buffer2, 0);
        if (shm_send_va2 == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer2, 0, &shm_send_pa2);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


        memcpy(shm_send_va2, ekhkm, shm_send_len2);
    }
    //rtd_pr_hdcp_debug("shm_buffer=%p, shm_len=%d, va=%p, pa=%x\n", (void *)shm_buffer, shm_len, shm_va, shm_pa);

    /**
     *  optee user ta meminfo
     */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[0].u.memref.shm = shm_send_buffer1;
    param[0].u.memref.size = shm_send_len1;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[1].u.memref.shm = shm_send_buffer2;
    param[1].u.memref.size = shm_send_len2;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

    rc = tee_client_invoke_func(hdcp2_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp2: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }

out:
    if (param) {
        kfree(param);
    }

    if (shm_send_buffer1) {
        tee_shm_free(shm_send_buffer1);
    }
    if (shm_send_buffer2) {
        tee_shm_free(shm_send_buffer2);
    }

    if (ret) {
        return ret;
    }
    else {
        return 0;
    }
}

static int optee_hdcp2_widi_Decrypt_EKS_handle(unsigned char id, unsigned int version, unsigned char *rtx, unsigned char *rrx, unsigned char *rn, unsigned char *eks)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;
    struct tee_shm *shm_send_buffer1 = NULL;
    unsigned int shm_send_len1 = 0;
    phys_addr_t shm_send_pa1 = 0;
    void *shm_send_va1 = 0;
    struct tee_shm *shm_send_buffer2 = NULL;
    unsigned int shm_send_len2 = 0;
    phys_addr_t shm_send_pa2 = 0;
    void *shm_send_va2 = 0;
    struct tee_shm *shm_send_buffer3 = NULL;
    unsigned int shm_send_len3 = 0;
    phys_addr_t shm_send_pa3 = 0;
    void *shm_send_va3 = 0;

    rtd_pr_hdcp_debug("get from optee\n");

    if (hdcp2_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp2: no ta context\n");
        ret = -EINVAL;
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_emerg("%s kcalloc param fail\n", __FUNCTION__);
        ret = -ENOMEM;
        goto out;
    }

    memset(&arg, 0, sizeof(arg));
    arg.func = id;
    arg.session = hdcp2_ta.session;
    arg.num_params = TEE_NUM_PARAM;
    if (rtx && rrx) {
        // alloc send share memory
        shm_send_len1 = RTX_SIZE + RRX_SIZE;
        shm_send_buffer1 = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len1, TEE_SHM_MAPPED);
        if (shm_send_buffer1 == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va1 = tee_shm_get_va(shm_send_buffer1, 0);
        if (shm_send_va1 == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer1, 0, &shm_send_pa1);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


        memcpy(shm_send_va1, rtx, RTX_SIZE);
        memcpy(shm_send_va1 + RTX_SIZE, rrx, RRX_SIZE);
    }
    if (rn) {
        // alloc send share memory
        shm_send_len2 = RN_SIZE;
        shm_send_buffer2 = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len2, TEE_SHM_MAPPED);
        if (shm_send_buffer2 == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va2 = tee_shm_get_va(shm_send_buffer2, 0);
        if (shm_send_va2 == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer2, 0, &shm_send_pa2);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


        memcpy(shm_send_va2, rn, shm_send_len2);
    }

    if (eks) {
        // alloc send share memory
        shm_send_len3 = EDKEYKS_SIZE;
        shm_send_buffer3 = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len3, TEE_SHM_MAPPED);
        if (shm_send_buffer3 == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va3 = tee_shm_get_va(shm_send_buffer3, 0);
        if (shm_send_va3 == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer3, 0, &shm_send_pa3);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


        memcpy(shm_send_va3, eks, shm_send_len3);
    }
    //rtd_pr_hdcp_debug("shm_buffer=%p, shm_len=%d, va=%p, pa=%x\n", (void *)shm_buffer, shm_len, shm_va, shm_pa);

    /**
     *  optee user ta meminfo
     */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
    param[0].u.value.a = version;
    param[0].u.value.b = 0;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[1].u.memref.shm = shm_send_buffer1;
    param[1].u.memref.size = shm_send_len1;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[2].u.memref.shm = shm_send_buffer2;
    param[2].u.memref.size = shm_send_len2;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[3].u.memref.shm = shm_send_buffer3;
    param[3].u.memref.size = shm_send_len3;

    rc = tee_client_invoke_func(hdcp2_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp2: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }

out:
    if (param) {
        kfree(param);
    }

    if (shm_send_buffer1) {
        tee_shm_free(shm_send_buffer1);
    }
    if (shm_send_buffer2) {
        tee_shm_free(shm_send_buffer2);
    }
    if (shm_send_buffer3) {
        tee_shm_free(shm_send_buffer3);
    }

    if (ret) {
        return ret;
    }
    else {
        return 0;
    }
}

static int optee_hdcp2_widi_XOR_Ks_with_LC128_handle(unsigned char id)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;

    rtd_pr_hdcp_debug("get from optee\n");

    if (hdcp2_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp2: no ta context\n");
        ret = -EINVAL;
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_emerg("%s kcalloc param fail\n", __FUNCTION__);
        ret = -ENOMEM;
        goto out;
    }

    memset(&arg, 0, sizeof(arg));
    arg.func = id;
    arg.session = hdcp2_ta.session;
    arg.num_params = TEE_NUM_PARAM;

    /**
     *  optee user ta meminfo
     */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

    rc = tee_client_invoke_func(hdcp2_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp2: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }

out:
    if (param) {
        kfree(param);
    }

    if (ret) {
        return ret;
    }
    else {
        return 0;
    }
}

static int optee_hdcp2_widi_Generate_Km_handle(unsigned char id, unsigned char *db, unsigned char *dbmask, unsigned char *maskedDB)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;
    struct tee_shm *shm_send_buffer1 = NULL;
    unsigned int shm_send_len1 = 0;
    phys_addr_t shm_send_pa1 = 0;
    void *shm_send_va1 = 0;
    struct tee_shm *shm_send_buffer2 = NULL;
    unsigned int shm_send_len2 = 0;
    phys_addr_t shm_send_pa2 = 0;
    void *shm_send_va2 = 0;
    struct tee_shm *shm_rev_buffer = NULL;
    unsigned int shm_rev_len = 0;
    phys_addr_t shm_rev_pa = 0;
    void *shm_rev_va = 0;

    rtd_pr_hdcp_debug("get from optee\n");

    if (hdcp2_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp2: no ta context\n");
        ret = -EINVAL;
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_emerg("%s kcalloc param fail\n", __FUNCTION__);
        ret = -ENOMEM;
        goto out;
    }

    memset(&arg, 0, sizeof(arg));
    arg.func = id;
    arg.session = hdcp2_ta.session;
    arg.num_params = TEE_NUM_PARAM;
    if (db) {
        // alloc send share memory
        shm_send_len1 = DB_SIZE;
        shm_send_buffer1 = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len1, TEE_SHM_MAPPED);
        if (shm_send_buffer1 == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va1 = tee_shm_get_va(shm_send_buffer1, 0);
        if (shm_send_va1 == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer1, 0, &shm_send_pa1);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


        memcpy(shm_send_va1, db, shm_send_len1);
    }
    if (dbmask) {
        // alloc send share memory
        shm_send_len2 = DB_SIZE;
        shm_send_buffer2 = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len2, TEE_SHM_MAPPED);
        if (shm_send_buffer2 == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va2 = tee_shm_get_va(shm_send_buffer2, 0);
        if (shm_send_va2 == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer2, 0, &shm_send_pa2);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


        memcpy(shm_send_va2, dbmask, shm_send_len2);
    }

    if (maskedDB) {
        // alloc receive share memory
        shm_rev_len = DB_SIZE;
        shm_rev_buffer = tee_shm_alloc(hdcp2_ta.ctx, shm_rev_len, TEE_SHM_MAPPED);
        if (shm_rev_buffer == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_rev_va = tee_shm_get_va(shm_rev_buffer, 0);
        if (shm_rev_va == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_rev_buffer, 0, &shm_rev_pa);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }
    }
    //rtd_pr_hdcp_debug("shm_buffer=%p, shm_len=%d, va=%p, pa=%x\n", (void *)shm_buffer, shm_len, shm_va, shm_pa);

    /**
     *  optee user ta meminfo
     */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[0].u.memref.shm = shm_send_buffer1;
    param[0].u.memref.size = shm_send_len1;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[1].u.memref.shm = shm_send_buffer2;
    param[1].u.memref.size = shm_send_len2;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
    param[2].u.memref.shm = shm_rev_buffer;
    param[2].u.memref.size = shm_rev_len;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

    rc = tee_client_invoke_func(hdcp2_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp2: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }
    if (maskedDB) {
        memcpy(maskedDB, (unsigned char *)shm_rev_va, shm_rev_len);
    }

out:
    if (param) {
        kfree(param);
    }

    if (shm_send_buffer1) {
        tee_shm_free(shm_send_buffer1);
    }
    if (shm_send_buffer2) {
        tee_shm_free(shm_send_buffer2);
    }
    if (shm_rev_buffer) {
        tee_shm_free(shm_rev_buffer);
    }

    if (ret) {
        return ret;
    }
    else {
        return 0;
    }
}

static int optee_hdcp2_widi_Generate_KS_handle(unsigned char id, unsigned int version, unsigned char *rtx, unsigned char *rrx, unsigned char *rn, unsigned char *eks)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;
    struct tee_shm *shm_send_buffer1 = NULL;
    unsigned int shm_send_len1 = 0;
    phys_addr_t shm_send_pa1 = 0;
    void *shm_send_va1 = 0;
    struct tee_shm *shm_send_buffer2 = NULL;
    unsigned int shm_send_len2 = 0;
    phys_addr_t shm_send_pa2 = 0;
    void *shm_send_va2 = 0;
    struct tee_shm *shm_rev_buffer = NULL;
    unsigned int shm_rev_len = 0;
    phys_addr_t shm_rev_pa = 0;
    void *shm_rev_va = 0;


    rtd_pr_hdcp_debug("get from optee\n");

    if (hdcp2_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp2: no ta context\n");
        ret = -EINVAL;
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_emerg("%s kcalloc param fail\n", __FUNCTION__);
        ret = -ENOMEM;
        goto out;
    }

    memset(&arg, 0, sizeof(arg));
    arg.func = id;
    arg.session = hdcp2_ta.session;
    arg.num_params = TEE_NUM_PARAM;
    if (rtx && rrx) {
        // alloc send share memory
        shm_send_len1 = RTX_SIZE + RRX_SIZE;
        shm_send_buffer1 = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len1, TEE_SHM_MAPPED);
        if (shm_send_buffer1 == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va1 = tee_shm_get_va(shm_send_buffer1, 0);
        if (shm_send_va1 == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer1, 0, &shm_send_pa1);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


        memcpy(shm_send_va1, rtx, RTX_SIZE);
        memcpy(shm_send_va1 + RTX_SIZE, rrx, RRX_SIZE);
    }
    if (rn) {
        // alloc send share memory
        shm_send_len2 = RN_SIZE;
        shm_send_buffer2 = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len2, TEE_SHM_MAPPED);
        if (shm_send_buffer2 == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va2 = tee_shm_get_va(shm_send_buffer2, 0);
        if (shm_send_va2 == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer2, 0, &shm_send_pa2);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


        memcpy(shm_send_va2, rn, shm_send_len2);
    }

    if (eks) {
        // alloc receive share memory
        shm_rev_len = EDKEYKS_SIZE;
        shm_rev_buffer = tee_shm_alloc(hdcp2_ta.ctx, shm_rev_len, TEE_SHM_MAPPED);
        if (shm_rev_buffer == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_rev_va = tee_shm_get_va(shm_rev_buffer, 0);
        if (shm_rev_va == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_rev_buffer, 0, &shm_rev_pa);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }
    }
    //rtd_pr_hdcp_debug("shm_buffer=%p, shm_len=%d, va=%p, pa=%x\n", (void *)shm_buffer, shm_len, shm_va, shm_pa);

    /**
     *  optee user ta meminfo
     */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
    param[0].u.value.a = version;
    param[0].u.value.b = 0;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[1].u.memref.shm = shm_send_buffer1;
    param[1].u.memref.size = shm_send_len1;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[2].u.memref.shm = shm_send_buffer2;
    param[2].u.memref.size = shm_send_len2;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
    param[3].u.memref.shm = shm_rev_buffer;
    param[3].u.memref.size = shm_rev_len;

    rc = tee_client_invoke_func(hdcp2_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp2: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }

    if (eks) {
        memcpy(eks, (unsigned char *)shm_rev_va, shm_rev_len);
    }

out:
    if (param) {
        kfree(param);
    }

    if (shm_send_buffer1) {
        tee_shm_free(shm_send_buffer1);
    }
    if (shm_send_buffer2) {
        tee_shm_free(shm_send_buffer2);
    }
    if (shm_rev_buffer) {
        tee_shm_free(shm_rev_buffer);
    }

    if (ret) {
        return ret;
    }
    else {
        return 0;
    }
}

static int optee_hdcp2_widi_SHA_handle(unsigned char id, unsigned char *send_buf, unsigned int send_size, unsigned char *rev_buf, unsigned int rev_size)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;
    struct tee_shm *shm_send_buffer = NULL;
    unsigned int shm_send_len = 0;
    phys_addr_t shm_send_pa = 0;
    void *shm_send_va = 0;
    struct tee_shm *shm_rev_buffer = NULL;
    unsigned int shm_rev_len = 0;
    phys_addr_t shm_rev_pa = 0;
    void *shm_rev_va = 0;

    rtd_pr_hdcp_debug("get from optee\n");

    if (hdcp2_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp2: no ta context\n");
        ret = -EINVAL;
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_emerg("%s kcalloc param fail\n", __FUNCTION__);
        ret = -ENOMEM;
        goto out;
    }

    memset(&arg, 0, sizeof(arg));
    arg.func = id;
    arg.session = hdcp2_ta.session;
    arg.num_params = TEE_NUM_PARAM;
    if (send_buf && (send_size != 0)) {
        // alloc send share memory
        shm_send_len = send_size;
        shm_send_buffer = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len, TEE_SHM_MAPPED);
        if (shm_send_buffer == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va = tee_shm_get_va(shm_send_buffer, 0);
        if (shm_send_va == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer, 0, &shm_send_pa);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


        memcpy(shm_send_va, send_buf, send_size);
    }
    //rtd_pr_hdcp_debug("shm_buffer=%p, shm_len=%d, va=%p, pa=%x\n", (void *)shm_buffer, shm_len, shm_va, shm_pa);
    if (rev_buf && (rev_size != 0)) {
        // alloc receive share memory
        shm_rev_len = rev_size;
        shm_rev_buffer = tee_shm_alloc(hdcp2_ta.ctx, shm_rev_len, TEE_SHM_MAPPED);
        if (shm_rev_buffer == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_rev_va = tee_shm_get_va(shm_rev_buffer, 0);
        if (shm_rev_va == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_rev_buffer, 0, &shm_rev_pa);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }
    }
    /**
     *  optee user ta meminfo
     */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[0].u.memref.shm = shm_send_buffer;
    param[0].u.memref.size = shm_send_len;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
    param[1].u.memref.shm = shm_rev_buffer;
    param[1].u.memref.size = shm_rev_len;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

    rc = tee_client_invoke_func(hdcp2_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp2: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }

    if (rev_buf) {
        memcpy(rev_buf, (unsigned char *)shm_rev_va, rev_size);
    }



out:
    if (param) {
        kfree(param);
    }

    if (shm_send_buffer) {
        tee_shm_free(shm_send_buffer);
    }

    if (shm_rev_buffer) {
        tee_shm_free(shm_rev_buffer);
    }

    if (ret) {
        return ret;
    }
    else {
        return 0;
    }
}

static int optee_hdcp2_widi_HMAC_SHA256_handle(unsigned char id, unsigned char *in_buf, unsigned int in_size, unsigned char *key_buf, unsigned int key_size, unsigned char *out_buf, unsigned int out_size)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;
    struct tee_shm *shm_send_buffer1 = NULL;
    unsigned int shm_send_len1 = 0;
    phys_addr_t shm_send_pa1 = 0;
    void *shm_send_va1 = 0;
    struct tee_shm *shm_send_buffer2 = NULL;
    unsigned int shm_send_len2 = 0;
    phys_addr_t shm_send_pa2 = 0;
    void *shm_send_va2 = 0;
    struct tee_shm *shm_rev_buffer = NULL;
    unsigned int shm_rev_len = 0;
    phys_addr_t shm_rev_pa = 0;
    void *shm_rev_va = 0;

    rtd_pr_hdcp_debug("get from optee\n");

    if (hdcp2_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp2: no ta context\n");
        ret = -EINVAL;
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_emerg("%s kcalloc param fail\n", __FUNCTION__);
        ret = -ENOMEM;
        goto out;
    }

    memset(&arg, 0, sizeof(arg));
    arg.func = id;
    arg.session = hdcp2_ta.session;
    arg.num_params = TEE_NUM_PARAM;
    if (in_buf && (in_size != 0)) {
        // alloc send share memory
        shm_send_len1 = in_size;
        shm_send_buffer1 = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len1, TEE_SHM_MAPPED);
        if (shm_send_buffer1 == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va1 = tee_shm_get_va(shm_send_buffer1, 0);
        if (shm_send_va1 == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer1, 0, &shm_send_pa1);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


        memcpy(shm_send_va1, in_buf, in_size);
    }

    if (key_buf && (key_size != 0)) {
        // alloc send share memory
        shm_send_len2 = key_size;
        shm_send_buffer2 = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len2, TEE_SHM_MAPPED);
        if (shm_send_buffer2 == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va2 = tee_shm_get_va(shm_send_buffer2, 0);
        if (shm_send_va2 == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer2, 0, &shm_send_pa2);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


        memcpy(shm_send_va2, key_buf, key_size);
    }
    //rtd_pr_hdcp_debug("shm_buffer=%p, shm_len=%d, va=%p, pa=%x\n", (void *)shm_buffer, shm_len, shm_va, shm_pa);
    if (out_buf && (out_size != 0)) {
        // alloc receive share memory
        shm_rev_len = out_size;
        shm_rev_buffer = tee_shm_alloc(hdcp2_ta.ctx, shm_rev_len, TEE_SHM_MAPPED);
        if (shm_rev_buffer == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_rev_va = tee_shm_get_va(shm_rev_buffer, 0);
        if (shm_rev_va == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_rev_buffer, 0, &shm_rev_pa);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }
    }
    /**
     *  optee user ta meminfo
     */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[0].u.memref.shm = shm_send_buffer1;
    param[0].u.memref.size = shm_send_len1;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[1].u.memref.shm = shm_send_buffer2;
    param[1].u.memref.size = shm_send_len2;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
    param[2].u.memref.shm = shm_rev_buffer;
    param[2].u.memref.size = shm_rev_len;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

    rc = tee_client_invoke_func(hdcp2_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp2: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }

    if (out_buf) {
        memcpy(out_buf, (unsigned char *)shm_rev_va, out_size);
    }



out:
    if (param) {
        kfree(param);
    }

    if (shm_send_buffer1) {
        tee_shm_free(shm_send_buffer1);
    }
    if (shm_send_buffer2) {
        tee_shm_free(shm_send_buffer2);
    }

    if (shm_rev_buffer) {
        tee_shm_free(shm_rev_buffer);
    }

    if (ret) {
        return ret;
    }
    else {
        return 0;
    }
}

static int optee_hdcp2_widi_AES_CTR_handle(unsigned char id, unsigned char *in_buf, unsigned int in_size, unsigned char *key_buf, unsigned int key_size, unsigned char *counter_buf, unsigned int counter_size, unsigned char *out_buf, unsigned int out_size)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;
    struct tee_shm *shm_send_buffer1 = NULL;
    unsigned int shm_send_len1 = 0;
    phys_addr_t shm_send_pa1 = 0;
    void *shm_send_va1 = 0;
    struct tee_shm *shm_send_buffer2 = NULL;
    unsigned int shm_send_len2 = 0;
    phys_addr_t shm_send_pa2 = 0;
    void *shm_send_va2 = 0;
    struct tee_shm *shm_send_buffer3 = NULL;
    unsigned int shm_send_len3 = 0;
    phys_addr_t shm_send_pa3 = 0;
    void *shm_send_va3 = 0;
    struct tee_shm *shm_rev_buffer = NULL;
    unsigned int shm_rev_len = 0;
    phys_addr_t shm_rev_pa = 0;
    void *shm_rev_va = 0;

    rtd_pr_hdcp_debug("get from optee\n");

    if (hdcp2_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp2: no ta context\n");
        ret = -EINVAL;
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_emerg("%s kcalloc param fail\n", __FUNCTION__);
        ret = -ENOMEM;
        goto out;
    }

    memset(&arg, 0, sizeof(arg));
    arg.func = id;
    arg.session = hdcp2_ta.session;
    arg.num_params = TEE_NUM_PARAM;
    if (in_buf && (in_size != 0)) {
        // alloc send share memory
        shm_send_len1 = in_size;
        shm_send_buffer1 = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len1, TEE_SHM_MAPPED);
        if (shm_send_buffer1 == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va1 = tee_shm_get_va(shm_send_buffer1, 0);
        if (shm_send_va1 == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer1, 0, &shm_send_pa1);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


        memcpy(shm_send_va1, in_buf, in_size);
    }

    if (key_buf && (key_size != 0)) {
        // alloc send share memory
        shm_send_len2 = key_size;
        shm_send_buffer2 = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len2, TEE_SHM_MAPPED);
        if (shm_send_buffer2 == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va2 = tee_shm_get_va(shm_send_buffer2, 0);
        if (shm_send_va2 == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer2, 0, &shm_send_pa2);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


        memcpy(shm_send_va2, key_buf, key_size);
    }

    if (counter_buf && (counter_size != 0)) {
        // alloc send share memory
        shm_send_len3 = counter_size;
        shm_send_buffer3 = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len3, TEE_SHM_MAPPED);
        if (shm_send_buffer3 == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va3 = tee_shm_get_va(shm_send_buffer3, 0);
        if (shm_send_va3 == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer3, 0, &shm_send_pa3);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


        memcpy(shm_send_va3, counter_buf, counter_size);
    }
    //rtd_pr_hdcp_debug("shm_buffer=%p, shm_len=%d, va=%p, pa=%x\n", (void *)shm_buffer, shm_len, shm_va, shm_pa);
    if (out_buf && (out_size != 0)) {
        // alloc receive share memory
        shm_rev_len = out_size;
        shm_rev_buffer = tee_shm_alloc(hdcp2_ta.ctx, shm_rev_len, TEE_SHM_MAPPED);
        if (shm_rev_buffer == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_rev_va = tee_shm_get_va(shm_rev_buffer, 0);
        if (shm_rev_va == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_rev_buffer, 0, &shm_rev_pa);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }
    }
    /**
     *  optee user ta meminfo
     */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[0].u.memref.shm = shm_send_buffer1;
    param[0].u.memref.size = shm_send_len1;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[1].u.memref.shm = shm_send_buffer2;
    param[1].u.memref.size = shm_send_len2;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[2].u.memref.shm = shm_send_buffer3;
    param[2].u.memref.size = shm_send_len3;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
    param[3].u.memref.shm = shm_rev_buffer;
    param[3].u.memref.size = shm_rev_len;

    rc = tee_client_invoke_func(hdcp2_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp2: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }

    if (out_buf) {
        memcpy(out_buf, (unsigned char *)shm_rev_va, out_size);
    }



out:
    if (param) {
        kfree(param);
    }

    if (shm_send_buffer1) {
        tee_shm_free(shm_send_buffer1);
    }
    if (shm_send_buffer2) {
        tee_shm_free(shm_send_buffer2);
    }

    if (shm_send_buffer3) {
        tee_shm_free(shm_send_buffer3);
    }
    if (shm_rev_buffer) {
        tee_shm_free(shm_rev_buffer);
    }

    if (ret) {
        return ret;
    }
    else {
        return 0;
    }
}

static int optee_hdcp2_widi_GET_PROTECTED_DECRYPTIONKEY_handle(unsigned char id, unsigned char *data, unsigned int size)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;
    struct tee_shm *shm_rev_buffer = NULL;
    unsigned int shm_rev_len = 0;
    phys_addr_t shm_rev_pa = 0;
    void *shm_rev_va = 0;

    rtd_pr_hdcp_debug("get from optee\n");

    if (hdcp2_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp2: no ta context\n");
        ret = -EINVAL;
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_emerg("%s kcalloc param fail\n", __FUNCTION__);
        ret = -ENOMEM;
        goto out;
    }

    memset(&arg, 0, sizeof(arg));
    arg.func = id;
    arg.session = hdcp2_ta.session;
    arg.num_params = TEE_NUM_PARAM;
    if (data && (size != 0)) {
        // alloc receive share memory
        shm_rev_len = size;
        shm_rev_buffer = tee_shm_alloc(hdcp2_ta.ctx, shm_rev_len, TEE_SHM_MAPPED);
        if (shm_rev_buffer == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_rev_va = tee_shm_get_va(shm_rev_buffer, 0);
        if (shm_rev_va == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_rev_buffer, 0, &shm_rev_pa);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }
    }
    //rtd_pr_hdcp_debug("shm_buffer=%p, shm_len=%d, va=%p, pa=%x\n", (void *)shm_buffer, shm_len, shm_va, shm_pa);

    /**
     *  optee user ta meminfo
     */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
    param[0].u.memref.shm = shm_rev_buffer;
    param[0].u.memref.size = shm_rev_len;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

    rc = tee_client_invoke_func(hdcp2_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp2: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }

    if (data) {
        memcpy(data, (unsigned char *)shm_rev_va, size);
    }

out:
    if (param) {
        kfree(param);
    }

    if (shm_rev_buffer) {
        tee_shm_free(shm_rev_buffer);
    }

    if (ret) {
        return ret;
    }
    else {
        return 0;
    }
}

static int optee_hdcp2_widi_SET_PROTECTED_DECRYPTIONKEY_handle(unsigned char id, unsigned char *data, unsigned int size)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;
    struct tee_shm *shm_send_buffer = NULL;
    unsigned int shm_send_len = 0;
    phys_addr_t shm_send_pa = 0;
    void *shm_send_va = 0;

    rtd_pr_hdcp_debug("get from optee\n");

    if (hdcp2_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp2: no ta context\n");
        ret = -EINVAL;
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_emerg("%s kcalloc param fail\n", __FUNCTION__);
        ret = -ENOMEM;
        goto out;
    }

    memset(&arg, 0, sizeof(arg));
    arg.func = id;
    arg.session = hdcp2_ta.session;
    arg.num_params = TEE_NUM_PARAM;
    if (data && (size != 0)) {
        // alloc send share memory
        shm_send_len = size;
        shm_send_buffer = tee_shm_alloc(hdcp2_ta.ctx, size, TEE_SHM_MAPPED);
        if (shm_send_buffer == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va = tee_shm_get_va(shm_send_buffer, 0);
        if (shm_send_va == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer, 0, &shm_send_pa);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


        memcpy(shm_send_va, data, size);
    }
    //rtd_pr_hdcp_debug("shm_buffer=%p, shm_len=%d, va=%p, pa=%x\n", (void *)shm_buffer, shm_len, shm_va, shm_pa);

    /**
     *  optee user ta meminfo
     */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[0].u.memref.shm = shm_send_buffer;
    param[0].u.memref.size = shm_send_len;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

    rc = tee_client_invoke_func(hdcp2_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp2: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }

out:
    if (param) {
        kfree(param);
    }

    if (shm_send_buffer) {
        tee_shm_free(shm_send_buffer);
    }

    if (ret) {
        return ret;
    }
    else {
        return 0;
    }
}

static int optee_hdcp2_widi_UseTestVector_handle(unsigned char id, unsigned int argc, unsigned char *data, unsigned int size)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;
    struct tee_shm *shm_send_buffer = NULL;
    unsigned int shm_send_len = 0;
    phys_addr_t shm_send_pa = 0;
    void *shm_send_va = 0;

    rtd_pr_hdcp_debug("get from optee\n");

    if (hdcp2_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp2: no ta context\n");
        ret = -EINVAL;
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_emerg("%s kcalloc param fail\n", __FUNCTION__);
        ret = -ENOMEM;
        goto out;
    }

    memset(&arg, 0, sizeof(arg));
    arg.func = id;
    arg.session = hdcp2_ta.session;
    arg.num_params = TEE_NUM_PARAM;
    if (data && (size != 0)) {
        // alloc send share memory
        shm_send_len = size;
        shm_send_buffer = tee_shm_alloc(hdcp2_ta.ctx, size, TEE_SHM_MAPPED);
        if (shm_send_buffer == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va = tee_shm_get_va(shm_send_buffer, 0);
        if (shm_send_va == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer, 0, &shm_send_pa);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


        memcpy(shm_send_va, data, size);
    }
    //rtd_pr_hdcp_debug("shm_buffer=%p, shm_len=%d, va=%p, pa=%x\n", (void *)shm_buffer, shm_len, shm_va, shm_pa);

    /**
     *  optee user ta meminfo
     */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
    param[0].u.value.a = argc;
    param[0].u.value.b = 0;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[1].u.memref.shm = shm_send_buffer;
    param[1].u.memref.size = shm_send_len;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

    rc = tee_client_invoke_func(hdcp2_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp2: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }

out:
    if (param) {
        kfree(param);
    }

    if (shm_send_buffer) {
        tee_shm_free(shm_send_buffer);
    }

    if (ret) {
        return ret;
    }
    else {
        return 0;
    }
}

static int optee_hdcp2_widi_Encrypt_RSAES_OAEP_handle(unsigned char id, unsigned char *kpubrx, unsigned char *ekpub_km)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;
    struct tee_shm *shm_send_buffer = NULL;
    unsigned int shm_send_len = 0;
    phys_addr_t shm_send_pa = 0;
    void *shm_send_va = 0;
    struct tee_shm *shm_rev_buffer = NULL;
    unsigned int shm_rev_len = 0;
    phys_addr_t shm_rev_pa = 0;
    void *shm_rev_va = 0;

    rtd_pr_hdcp_debug("get from optee\n");

    if (hdcp2_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp2: no ta context\n");
        ret = -EINVAL;
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_emerg("%s kcalloc param fail\n", __FUNCTION__);
        ret = -ENOMEM;
        goto out;
    }

    memset(&arg, 0, sizeof(arg));
    arg.func = id;
    arg.session = hdcp2_ta.session;
    arg.num_params = TEE_NUM_PARAM;
    if (kpubrx) {
        // alloc send share memory
        shm_send_len = KPUBRX_SIZE;
        shm_send_buffer = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len, TEE_SHM_MAPPED);
        if (shm_send_buffer == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va = tee_shm_get_va(shm_send_buffer, 0);
        if (shm_send_va == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer, 0, &shm_send_pa);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


        memcpy(shm_send_va, kpubrx, shm_send_len);
    }
    //rtd_pr_hdcp_debug("shm_buffer=%p, shm_len=%d, va=%p, pa=%x\n", (void *)shm_buffer, shm_len, shm_va, shm_pa);
    if (ekpub_km) {
        // alloc receive share memory
        shm_rev_len = EKPUBKM_SIZE;
        shm_rev_buffer = tee_shm_alloc(hdcp2_ta.ctx, shm_rev_len, TEE_SHM_MAPPED);
        if (shm_rev_buffer == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_rev_va = tee_shm_get_va(shm_rev_buffer, 0);
        if (shm_rev_va == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_rev_buffer, 0, &shm_rev_pa);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }
    }
    /**
     *  optee user ta meminfo
     */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[0].u.memref.shm = shm_send_buffer;
    param[0].u.memref.size = shm_send_len;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
    param[1].u.memref.shm = shm_rev_buffer;
    param[1].u.memref.size = shm_rev_len;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

    rc = tee_client_invoke_func(hdcp2_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp2: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }

    if (ekpub_km) {
        memcpy(ekpub_km, (unsigned char *)shm_rev_va, shm_rev_len);
    }



out:
    if (param) {
        kfree(param);
    }

    if (shm_send_buffer) {
        tee_shm_free(shm_send_buffer);
    }

    if (shm_rev_buffer) {
        tee_shm_free(shm_rev_buffer);
    }

    if (ret) {
        return ret;
    }
    else {
        return 0;
    }
}

static int optee_hdcp2_widi_Get_KsXorLc128_handle(unsigned char id, unsigned char *data, unsigned int size)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;
    struct tee_shm *shm_rev_buffer = NULL;
    unsigned int shm_rev_len = 0;
    phys_addr_t shm_rev_pa = 0;
    void *shm_rev_va = 0;

    rtd_pr_hdcp_debug("get from optee\n");

    if (hdcp2_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp2: no ta context\n");
        ret = -EINVAL;
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_emerg("%s kcalloc param fail\n", __FUNCTION__);
        ret = -ENOMEM;
        goto out;
    }

    memset(&arg, 0, sizeof(arg));
    arg.func = id;
    arg.session = hdcp2_ta.session;
    arg.num_params = TEE_NUM_PARAM;

    //rtd_pr_hdcp_debug("shm_buffer=%p, shm_len=%d, va=%p, pa=%x\n", (void *)shm_buffer, shm_len, shm_va, shm_pa);
    if (data) {
        // alloc receive share memory
        shm_rev_len = size;
        shm_rev_buffer = tee_shm_alloc(hdcp2_ta.ctx, shm_rev_len, TEE_SHM_MAPPED);
        if (shm_rev_buffer == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_rev_va = tee_shm_get_va(shm_rev_buffer, 0);
        if (shm_rev_va == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_rev_buffer, 0, &shm_rev_pa);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }
    }
    /**
     *  optee user ta meminfo
     */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
    param[0].u.memref.shm = shm_rev_buffer;
    param[0].u.memref.size = shm_rev_len;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

    rc = tee_client_invoke_func(hdcp2_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp2: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }

    if (data) {
        memcpy(data, (unsigned char *)shm_rev_va, shm_rev_len);
    }



out:
    if (param) {
        kfree(param);
    }

    if (shm_rev_buffer) {
        tee_shm_free(shm_rev_buffer);
    }

    if (ret) {
        return ret;
    }
    else {
        return 0;
    }
}

static int optee_hdcp2_widi_DATA_DECRYPT_handle(unsigned char id, unsigned char *counter, unsigned char *input, unsigned char *output, int size)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;
    struct tee_shm *shm_send_buffer1 = NULL;
    unsigned int shm_send_len1 = 0;
    phys_addr_t shm_send_pa1 = 0;
    void *shm_send_va1 = 0;
    struct tee_shm *shm_send_buffer2 = NULL;
    unsigned int shm_send_len2 = 0;
    phys_addr_t shm_send_pa2 = 0;
    void *shm_send_va2 = 0;
    struct tee_shm *shm_rev_buffer = NULL;
    unsigned int shm_rev_len = 0;
    phys_addr_t shm_rev_pa = 0;
    void *shm_rev_va = 0;

    rtd_pr_hdcp_debug("get from optee\n");

    if (hdcp2_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp2: no ta context\n");
        ret = -EINVAL;
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_emerg("%s kcalloc param fail\n", __FUNCTION__);
        ret = -ENOMEM;
        goto out;
    }

    memset(&arg, 0, sizeof(arg));
    arg.func = id;
    arg.session = hdcp2_ta.session;
    arg.num_params = TEE_NUM_PARAM;
    if (counter) {
        // alloc send share memory
        shm_send_len1 = 16;
        shm_send_buffer1 = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len1, (TEE_SHM_MAPPED | TEE_SHM_DMA_BUF));
        if (shm_send_buffer1 == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va1 = tee_shm_get_va(shm_send_buffer1, 0);
        if (shm_send_va1 == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer1, 0, &shm_send_pa1);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


        memcpy(shm_send_va1, counter, 16);
    }

    if (input && (size != 0)) {
        // alloc send share memory
        shm_send_len2 = size;
        shm_send_buffer2 = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len2, (TEE_SHM_MAPPED | TEE_SHM_DMA_BUF));
        if (shm_send_buffer2 == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va2 = tee_shm_get_va(shm_send_buffer2, 0);
        if (shm_send_va2 == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer2, 0, &shm_send_pa2);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }


        memcpy(shm_send_va2, input, size);
    }

    if (output && (size != 0)) {
        // alloc receive share memory
        shm_rev_len = size;
        shm_rev_buffer = tee_shm_alloc(hdcp2_ta.ctx, shm_rev_len, (TEE_SHM_MAPPED | TEE_SHM_DMA_BUF));
        if (shm_rev_buffer == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_rev_va = tee_shm_get_va(shm_rev_buffer, 0);
        if (shm_rev_va == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_rev_buffer, 0, &shm_rev_pa);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }
    }
    /**
     *  optee user ta meminfo
     */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[0].u.memref.shm = shm_send_buffer1;
    param[0].u.memref.size = shm_send_len1;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[1].u.memref.shm = shm_send_buffer2;
    param[1].u.memref.size = shm_send_len2;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
    param[2].u.memref.shm = shm_rev_buffer;
    param[2].u.memref.size = shm_rev_len;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

    rc = tee_client_invoke_func(hdcp2_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp2: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }

    if (output) {
        memcpy(output, (unsigned char *)shm_rev_va, size);
    }



out:
    if (param) {
        kfree(param);
    }

    if (shm_send_buffer1) {
        tee_shm_free(shm_send_buffer1);
    }
    if (shm_send_buffer2) {
        tee_shm_free(shm_send_buffer2);
    }

    if (shm_rev_buffer) {
        tee_shm_free(shm_rev_buffer);
    }

    if (ret) {
        return ret;
    }
    else {
        return 0;
    }
}

static int optee_hdcp2_widi_Select_StoredKm_handle(unsigned char id, int index)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;

    rtd_pr_hdcp_debug("get from optee\n");

    if (hdcp2_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp2: no ta context\n");
        ret = -EINVAL;
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_emerg("%s kcalloc param fail\n", __FUNCTION__);
        ret = -ENOMEM;
        goto out;
    }

    memset(&arg, 0, sizeof(arg));
    arg.func = id;
    arg.session = hdcp2_ta.session;
    arg.num_params = TEE_NUM_PARAM;

    /**
     *  optee user ta meminfo
     */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
    param[0].u.value.a = index;
    param[0].u.value.b = 0;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

    rc = tee_client_invoke_func(hdcp2_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp2: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }

out:
    if (param) {
        kfree(param);
    }

    if (ret) {
        return ret;
    }
    else {
        return 0;
    }
}

static int optee_hdcp2_widi_Write_StoredKm_handle(unsigned char id, int index)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;

    rtd_pr_hdcp_debug("get from optee\n");

    if (hdcp2_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp2: no ta context\n");
        ret = -EINVAL;
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_emerg("%s kcalloc param fail\n", __FUNCTION__);
        ret = -ENOMEM;
        goto out;
    }

    memset(&arg, 0, sizeof(arg));
    arg.func = id;
    arg.session = hdcp2_ta.session;
    arg.num_params = TEE_NUM_PARAM;

    /**
     *  optee user ta meminfo
     */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
    param[0].u.value.a = index;
    param[0].u.value.b = 0;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

    rc = tee_client_invoke_func(hdcp2_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp2: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }

out:
    if (param) {
        kfree(param);
    }

    if (ret) {
        return ret;
    }
    else {
        return 0;
    }
}

static int optee_hdcp2_repeater_set_enable_handle(unsigned char id, unsigned char nport, unsigned char enable)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;

    rtd_pr_hdcp_debug("get from optee\n");

    if (hdcp2_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp2: no ta context\n");
        ret = -EINVAL;
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_emerg("%s kcalloc param fail\n", __FUNCTION__);
        ret = -ENOMEM;
        goto out;
    }

    memset(&arg, 0, sizeof(arg));
    arg.func = id;
    arg.session = hdcp2_ta.session;
    arg.num_params = TEE_NUM_PARAM;

    /**
     *  optee user ta meminfo
     */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
    param[0].u.value.a = nport;
    param[0].u.value.b = enable;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

    rc = tee_client_invoke_func(hdcp2_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp2: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }

out:
    if (param) {
        kfree(param);
        param = NULL;
    }

    if (ret) {
        return ret;
    }
    else {
        return 0;
    }
}

static int optee_hdcp2_repeater_set_rxinfo_ridlist_handle(unsigned char id, unsigned char nport, unsigned short rxinfo, unsigned char *ridlist, unsigned int size)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;
    struct tee_shm *shm_send_buffer = NULL;
    unsigned int shm_send_len = 0;
    phys_addr_t shm_send_pa = 0;
    void *shm_send_va = 0;
    unsigned int max_list_size = 160; //32*5

    rtd_pr_hdcp_debug("get from optee\n");

    if (hdcp2_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp2: no ta context\n");
        ret = -EINVAL;
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_emerg("%s kcalloc param fail\n", __FUNCTION__);
        ret = -ENOMEM;
        goto out;
    }

    memset(&arg, 0, sizeof(arg));
    arg.func = id;
    arg.session = hdcp2_ta.session;
    arg.num_params = TEE_NUM_PARAM;

    // alloc send share memory
    shm_send_len = max_list_size;
    shm_send_buffer = tee_shm_alloc(hdcp2_ta.ctx, max_list_size, TEE_SHM_MAPPED);
    if (shm_send_buffer == NULL) {
        rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
        ret = -ENOMEM;
        goto out;
    }

    // get share memory virtual addr for data accessing
    shm_send_va = tee_shm_get_va(shm_send_buffer, 0);
    if (shm_send_va == NULL) {
        ret = -ENOMEM;
        goto out;
    }

    // get share memory physial addr for tee param
    rc = tee_shm_get_pa(shm_send_buffer, 0, &shm_send_pa);
    if (rc) {
        ret = -ENOMEM;
        goto out;
    }

    memset(shm_send_va, 0, shm_send_len);

    if (ridlist && (size != 0)) {
        memcpy(shm_send_va, ridlist, size);
    }

    //rtd_pr_hdcp_debug("shm_buffer=%p, shm_len=%d, va=%p, pa=%x\n", (void *)shm_buffer, shm_len, shm_va, shm_pa);

    /**
     *  optee user ta meminfo
     */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
    param[0].u.value.a = nport;
    param[0].u.value.b = rxinfo;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[1].u.memref.shm = shm_send_buffer;
    param[1].u.memref.size = shm_send_len;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

    rc = tee_client_invoke_func(hdcp2_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp2: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }

out:
    if (param) {
        kfree(param);
        param = NULL;
    }

    if (shm_send_buffer) {
        tee_shm_free(shm_send_buffer);
        shm_send_buffer = NULL;
    }

    if (ret) {
        return ret;
    }
    else {
        return 0;
    }
}


static int optee_hdcp2_widi_compute_km(unsigned char id, unsigned char encrypted_bkeys[320], unsigned char aksv[5], unsigned char km[7])
{
    int ret = 0, rc = 0;
    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;
    struct tee_shm *shm_send_buffer = NULL;
    unsigned int shm_send_len = 0;
    phys_addr_t shm_send_pa = 0;
    void *shm_send_va = 0;
    struct tee_shm *shm_rev_buffer = NULL;
    unsigned int shm_rev_len = 0;
    phys_addr_t shm_rev_pa = 0;
    void *shm_rev_va = 0;

    rtd_pr_hdcp_debug("get from optee\n");

    if (hdcp2_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp2: no ta context\n");
        ret = -EINVAL;
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_emerg("%s kcalloc param fail\n", __FUNCTION__);
        ret = -ENOMEM;
        goto out;
    }
    memset(&arg, 0, sizeof(arg));
    arg.func = id;
    arg.session = hdcp2_ta.session;
    arg.num_params = TEE_NUM_PARAM;
    if (encrypted_bkeys && aksv) {
        // alloc send share memory
        shm_send_len = 325;
        shm_send_buffer = tee_shm_alloc(hdcp2_ta.ctx, shm_send_len, TEE_SHM_MAPPED);
        if (shm_send_buffer == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_send_va = tee_shm_get_va(shm_send_buffer, 0);
        if (shm_send_va == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_send_buffer, 0, &shm_send_pa);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }
        memcpy(shm_send_va, encrypted_bkeys, 320);
        memcpy(shm_send_va + 320, aksv, 5);
    }
    //rtd_pr_hdcp_debug("shm_buffer=%p, shm_len=%d, va=%p, pa=%x\n", (void *)shm_buffer, shm_len, shm_va, shm_pa);
    if (km) {
        // alloc receive share memory
        shm_rev_len = 7;
        shm_rev_buffer = tee_shm_alloc(hdcp2_ta.ctx, shm_rev_len, TEE_SHM_MAPPED);
        if (shm_rev_buffer == NULL) {
            rtd_pr_hdcp_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_rev_va = tee_shm_get_va(shm_rev_buffer, 0);
        if (shm_rev_va == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_rev_buffer, 0, &shm_rev_pa);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }
    }
    /**
     *  optee user ta meminfo
     */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[0].u.memref.shm = shm_send_buffer;
    param[0].u.memref.size = shm_send_len;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
    param[1].u.memref.shm = shm_rev_buffer;
    param[1].u.memref.size = shm_rev_len;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

    rc = tee_client_invoke_func(hdcp2_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp2: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }

    if (km) {
        memcpy(km, (unsigned char *)shm_rev_va, shm_rev_len);
    }



out:
    if (param) {
        kfree(param);
    }

    if (shm_send_buffer) {
        tee_shm_free(shm_send_buffer);
    }

    if (shm_rev_buffer) {
        tee_shm_free(shm_rev_buffer);
    }

    if (ret) {
        return ret;
    }
    else {
        return 0;
    }
}

void optee_hdcp2_main(HDMI_DP_HDCP2_MODE_T hdmi_dp, unsigned char id, unsigned char nport, unsigned char *send_buf, unsigned int send_size, unsigned char *rev_buf, unsigned int rev_size)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_handle(hdmi_dp, id, nport, send_buf, send_size, rev_buf, rev_size)) {
        return;
    }

#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_main);

void optee_hdcp2_wid_WriteKeySet(unsigned char id, unsigned char *data, unsigned int size)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_widi_WriteKeySet_handle(id, data, size)) {
        return;
    }
#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_wid_WriteKeySet);


void optee_hdcp2_widi_GET_CERTINFO2(unsigned char id, unsigned char *data1, unsigned char *data2, unsigned char *data3, unsigned char *data4)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_widi_GET_CERTINFO2_handle(id, data1, data2, data3, data4)) {
        return;
    }
#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_widi_GET_CERTINFO2);

void optee_hdcp2_widi_DECRYPT_RSAES_OAEP(unsigned char id, unsigned char *data, unsigned int size)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_widi_DECRYPT_RSAES_OAEP_handle(id, data, size)) {
        return;
    }
#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_widi_DECRYPT_RSAES_OAEP);

void optee_hdcp2_widi_KD_KEY_DERIVATION(unsigned char id, unsigned char *rtx, unsigned char *rrx, unsigned int version)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_widi_KD_KEY_DERIVATION_handle(id, rtx, rrx, version)) {
        return;
    }
#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_widi_KD_KEY_DERIVATION);

void optee_hdcp2_widi_Compute_H_Prime(unsigned char id, unsigned char repeater, unsigned int version, unsigned char *rtx, unsigned char *deviceOptionInfo, unsigned char *pHPrime)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_widi_Compute_H_Prime_handle(id, repeater, version, rtx, deviceOptionInfo, pHPrime)) {
        return;
    }
#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_widi_Compute_H_Prime);

void optee_hdcp2_widi_Compute_L_Prime(unsigned char id, unsigned int version, unsigned char *rn, unsigned char *rrx, unsigned char *pLPrime)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_widi_Compute_L_Prime_handle(id, version, rn, rrx, pLPrime)) {
        return;
    }
#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_widi_Compute_L_Prime);

void optee_hdcp2_widi_Compute_KH(unsigned char id)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_widi_Compute_KH_handle(id)) {
        return;
    }
#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_widi_Compute_KH);

void optee_hdcp2_widi_Encrypt_Km_using_Kh(unsigned char id, unsigned char *m, unsigned char *ekhkm)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_widi_Encrypt_Km_using_Kh_handle(id, m, ekhkm)) {
        return;
    }
#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_widi_Encrypt_Km_using_Kh);

void optee_hdcp2_widi_Decrypt_Km_using_Kh(unsigned char id, unsigned char *m, unsigned char *ekhkm)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_widi_Decrypt_Km_using_Kh_handle(id, m, ekhkm)) {
        return;
    }
#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_widi_Decrypt_Km_using_Kh);

void optee_hdcp2_widi_Decrypt_EKS(unsigned char id, unsigned int version, unsigned char *rtx, unsigned char *rrx, unsigned char *rn, unsigned char *eks)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_widi_Decrypt_EKS_handle(id, version, rtx, rrx, rn, eks)) {
        return;
    }
#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_widi_Decrypt_EKS);

void optee_hdcp2_widi_XOR_Ks_with_LC128(unsigned char id)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_widi_XOR_Ks_with_LC128_handle(id)) {
        return;
    }
#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_widi_XOR_Ks_with_LC128);

void optee_hdcp2_widi_Generate_Km(unsigned char id, unsigned char *db, unsigned char *dbmask, unsigned char *maskedDB)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_widi_Generate_Km_handle(id, db, dbmask, maskedDB)) {
        return;
    }
#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_widi_Generate_Km);

void optee_hdcp2_widi_Generate_KS(unsigned char id, unsigned int version, unsigned char *rtx, unsigned char *rrx, unsigned char *rn, unsigned char *eks)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_widi_Generate_KS_handle(id, version, rtx, rrx, rn, eks)) {
        return;
    }
#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_widi_Generate_KS);

void optee_hdcp2_widi_SHA1(unsigned char id, unsigned char *in_ptr,  unsigned int in_len,  unsigned char *out_ptr, unsigned int out_len)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_widi_SHA_handle(id, in_ptr, in_len, out_ptr, out_len)) {
        return;
    }
#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_widi_SHA1);

void optee_hdcp2_widi_SHA256(unsigned char id, unsigned char *in_ptr,  unsigned int in_len,  unsigned char *out_ptr, unsigned int out_len)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_widi_SHA_handle(id, in_ptr, in_len, out_ptr, out_len)) {
        return;
    }
#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_widi_SHA256);

void optee_hdcp2_widi_HMAC_SHA256(unsigned char id, unsigned char *in_ptr,  unsigned int in_len, unsigned char *key_ptr, unsigned int key_len, unsigned char *out_ptr, unsigned int out_len)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_widi_HMAC_SHA256_handle(id, in_ptr, in_len, key_ptr, key_len, out_ptr, out_len)) {
        return;
    }
#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_widi_HMAC_SHA256);

void optee_hdcp2_widi_AES_CTR(unsigned char id, unsigned char *in_ptr,  unsigned int in_len, unsigned char *key_ptr, unsigned int key_len, unsigned char *counter_ptr, unsigned int counter_len, unsigned char *out_ptr, unsigned int out_len)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_widi_AES_CTR_handle(id, in_ptr, in_len, key_ptr, key_len, counter_ptr, counter_len, out_ptr, out_len)) {
        return;
    }
#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_widi_AES_CTR);

void optee_hdcp2_widi_GET_PROTECTED_DECRYPTIONKEY(unsigned char id, unsigned char *data, unsigned int len)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_widi_GET_PROTECTED_DECRYPTIONKEY_handle(id, data, len)) {
        return;
    }
#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_widi_GET_PROTECTED_DECRYPTIONKEY);

void optee_hdcp2_widi_SET_PROTECTED_DECRYPTIONKEY(unsigned char id, unsigned char *data, unsigned int len)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_widi_SET_PROTECTED_DECRYPTIONKEY_handle(id, data, len)) {
        return;
    }
#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_widi_SET_PROTECTED_DECRYPTIONKEY);


void optee_hdcp2_widi_Compute_V_Prime(unsigned char id, unsigned char *in_ptr,  unsigned int in_len,  unsigned char *out_ptr, unsigned int out_len)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_widi_SHA_handle(id, in_ptr, in_len, out_ptr, out_len)) {
        return;
    }
#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_widi_Compute_V_Prime);

void optee_hdcp2_widi_Compute_M_Prime(unsigned char id, unsigned char *in_ptr,  unsigned int in_len,  unsigned char *out_ptr, unsigned int out_len)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_widi_SHA_handle(id, in_ptr, in_len, out_ptr, out_len)) {
        return;
    }
#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_widi_Compute_M_Prime);

void optee_hdcp2_widi_UseTestVector(unsigned char id, unsigned int argc, unsigned char *testvector, unsigned int size)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_widi_UseTestVector_handle(id, argc, testvector, size)) {
        return;
    }
#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_widi_UseTestVector);


void optee_hdcp2_widi_Encrypt_RSAES_OAEP(unsigned char id, unsigned char *kpubrx, unsigned char *ekpub_km)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_widi_Encrypt_RSAES_OAEP_handle(id, kpubrx, ekpub_km)) {
        return;
    }
#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_widi_Encrypt_RSAES_OAEP);


void optee_hdcp2_widi_HMAC_SHA256_with_kd(unsigned char id, unsigned char *in_ptr,  unsigned int in_len, unsigned char *out_ptr, unsigned int out_len)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_widi_SHA_handle(id, in_ptr, in_len, out_ptr, out_len)) {
        return;
    }
#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_widi_HMAC_SHA256_with_kd);

void optee_hdcp2_widi_Get_KsXorLc128(unsigned char id, unsigned char *data, unsigned int size)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_widi_Get_KsXorLc128_handle(id, data, size)) {
        return;
    }
#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_widi_Get_KsXorLc128);

void optee_hdcp2_widi_DATA_DECRYPT(unsigned char id, unsigned char *counter, unsigned char *input, unsigned char *output, int size)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_widi_DATA_DECRYPT_handle(id, counter, input, output, size)) {
        return;
    }
#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_widi_DATA_DECRYPT);

void optee_hdcp2_widi_Select_StoredKm(unsigned char id, int index)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_widi_Select_StoredKm_handle(id, index)) {
        return;
    }
#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_widi_Select_StoredKm);

void optee_hdcp2_widi_Write_StoredKm(unsigned char id, int index)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_widi_Write_StoredKm_handle(id, index)) {
        return;
    }
#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_widi_Write_StoredKm);

void optee_hdcp2_repeater_set_enable(unsigned char id, unsigned char nport, unsigned char enable)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_repeater_set_enable_handle(id, nport, enable)) {
        return;
    }
#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_repeater_set_enable);

void optee_hdcp2_repeater_set_rxinfo_ridlist(unsigned char id, unsigned char nport, unsigned short rxinfo, unsigned char *ridlist, unsigned int size)
{
#if 0
    if (optee_hdcp2_init()) {
        return;
    }
#endif
    if (optee_hdcp2_repeater_set_rxinfo_ridlist_handle(id, nport, rxinfo, ridlist, size)) {
        return;
    }
#if 0
    optee_hdcp2_deinit();
#endif
}
EXPORT_SYMBOL(optee_hdcp2_repeater_set_rxinfo_ridlist);

bool optee_hdcp14_compute_km(unsigned char encrypted_bkeys[320], unsigned char aksv[5], unsigned char km[7])
{
    if (!encrypted_bkeys || !aksv) {
        return false;
    }

#if 0
    if (optee_hdcp2_init()) {
        return false;
    }
#endif
    if (optee_hdcp2_widi_compute_km(HDCP14_CMD_GENERATE_KM, encrypted_bkeys, aksv, km)) {
        return false;
    }
#if 0
    optee_hdcp2_deinit();
#endif
    return true;

}
EXPORT_SYMBOL(optee_hdcp14_compute_km);

void optee_hdcp2_get_receiver_id(unsigned char id, unsigned char *data, unsigned int size)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;
    struct tee_shm *shm_rev_buffer = NULL;
    unsigned int shm_rev_len = 0;
    phys_addr_t shm_rev_pa = 0;
    void *shm_rev_va = 0;

    pr_debug("get receiver_id from optee\n");

    if (hdcp2_ta.ctx == NULL) {
        pr_err("optee_hdcp2: no ta context\n");
        ret = -EINVAL;
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        pr_emerg("%s kcalloc param fail\n", __FUNCTION__);
        ret = -ENOMEM;
        goto out;
    }

    memset(&arg, 0, sizeof(arg));
    arg.func = id;
    arg.session = hdcp2_ta.session;
    arg.num_params = TEE_NUM_PARAM;

    //pr_debug("shm_buffer=%p, shm_len=%d, va=%p, pa=%x\n", (void *)shm_buffer, shm_len, shm_va, shm_pa);
    if (data) {
        // alloc receive share memory
        shm_rev_len = size;
        shm_rev_buffer = tee_shm_alloc(hdcp2_ta.ctx, shm_rev_len, TEE_SHM_MAPPED);
        if (shm_rev_buffer == NULL) {
            pr_emerg("optee_hdcp2: no shm_buffer\n");
            ret = -ENOMEM;
            goto out;
        }

        // get share memory virtual addr for data accessing
        shm_rev_va = tee_shm_get_va(shm_rev_buffer, 0);
        if (shm_rev_va == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        // get share memory physial addr for tee param
        rc = tee_shm_get_pa(shm_rev_buffer, 0, &shm_rev_pa);
        if (rc) {
            ret = -ENOMEM;
            goto out;
        }
    }
    /**
     *  optee user ta meminfo
     */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
    param[0].u.memref.shm = shm_rev_buffer;
    param[0].u.memref.size = shm_rev_len;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

    rc = tee_client_invoke_func(hdcp2_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        pr_emerg("optee_hdcp2: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }

    if (data) {
        memcpy(data, (unsigned char *)shm_rev_va, shm_rev_len);
    }

out:
    if (param) {
        kfree(param);
    }

    if (shm_rev_buffer) {
        tee_shm_free(shm_rev_buffer);
    }

    if (ret) {
        pr_emerg("optee_hdcp2: optee_hdcp2_get_receiver_id failed, ret %x\n", ret);
    }

    return;
}
EXPORT_SYMBOL(optee_hdcp2_get_receiver_id);

