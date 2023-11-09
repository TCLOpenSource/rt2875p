#include <linux/kernel.h>
#include <linux/tee.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/tee_drv.h>
#include <linux/uuid.h>
#include <linux/slab.h>
#include <linux/debugfs.h>
#include <tvscalercontrol/hdmirx/hdcp14_optee.h>
#include <rtd_log/rtd_module_log.h>

static const uuid_t hdcp14_uuid =
            UUID_INIT(0x465f85e2, 0x58d8, 0x4ec4,
                      0xbd, 0x46, 0xee, 0xb7, 0x06, 0x25, 0xe0, 0x4c);

#define TEE_NUM_PARAM 4


struct optee_ta {
    struct tee_context *ctx;
    __u32 session;
};

static struct optee_ta hdcp14_ta;

static int optee_hdcp14_match(struct tee_ioctl_version_data *data, const void *vers)
{
    return 1;
}


/* 0: success, -N: failure (N: value) */
int optee_hdcp14_init(void)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_open_session_arg arg;
    struct tee_ioctl_version_data vers = {
        .impl_id = TEE_IMPL_ID_OPTEE,
        .impl_caps = TEE_OPTEE_CAP_TZ,
        .gen_caps = TEE_GEN_CAP_GP,
    };

    memset(&hdcp14_ta, 0, sizeof(hdcp14_ta));
    hdcp14_ta.ctx = tee_client_open_context(NULL, optee_hdcp14_match, NULL, &vers);
    if (hdcp14_ta.ctx == NULL) {
        rtd_pr_hdcp_emerg("optee_hdcp14: no ta context\n");
        ret = -EINVAL;
        goto err;
    }

    memset(&arg, 0, sizeof(arg));
    memcpy(arg.uuid, hdcp14_uuid.b, TEE_IOCTL_UUID_LEN);
    arg.clnt_login = TEE_IOCTL_LOGIN_PUBLIC;
    rtd_pr_hdcp_debug("arg uuid %pUl \n", arg.uuid);

    rc = tee_client_open_session(hdcp14_ta.ctx, &arg, NULL);

    if (rc) {
        rtd_pr_hdcp_emerg("optee_hdcp14: open_session failed ret %x arg %x", rc, arg.ret);
        ret = -EINVAL;
        goto err;
    }
    if (arg.ret) {
        ret = -EINVAL;
        goto err;
    }

    hdcp14_ta.session = arg.session;

    rtd_pr_hdcp_debug("open_session ok\n");
    return 0;

err:
    if (hdcp14_ta.session) {
        tee_client_close_session(hdcp14_ta.ctx, hdcp14_ta.session);
        rtd_pr_hdcp_emerg("optee_hdcp14: open failed close session \n");
        hdcp14_ta.session = 0;
    }
    if (hdcp14_ta.ctx) {
        tee_client_close_context(hdcp14_ta.ctx);
        rtd_pr_hdcp_emerg("optee_hdcp14: open failed close context\n");
        hdcp14_ta.ctx = NULL;
    }
    rtd_pr_hdcp_emerg("open_session fail\n");

    return ret;
}

void optee_hdcp14_deinit(void)
{
    if (hdcp14_ta.session) {
        tee_client_close_session(hdcp14_ta.ctx, hdcp14_ta.session);
        hdcp14_ta.session = 0;
    }

    if (hdcp14_ta.ctx) {
        tee_client_close_context(hdcp14_ta.ctx);
        hdcp14_ta.ctx = NULL;
    }
}

int optee_hdcp14_get_key_handle(unsigned char *bksv, unsigned int bksv_len, unsigned char *bkey, unsigned int bkey_len, struct tee_param *param)
{
    int ret = 0,  rc = 0;
    struct tee_shm *shm_rev_buffer1 = NULL;
    //phys_addr_t shm_rev_pa1 = 0;
    void *shm_rev_va1 = 0;
    struct tee_shm *shm_rev_buffer2 = NULL;
    //phys_addr_t shm_rev_pa2 = 0;
    void *shm_rev_va2 = 0;
    struct tee_ioctl_invoke_arg arg;

    memset(&arg, 0, sizeof(arg));
    arg.func = HDCP14_CMD_GET_BKSV;
    arg.session = hdcp14_ta.session;
    arg.num_params = TEE_NUM_PARAM;

    // alloc receive share memory
    shm_rev_buffer1 = tee_shm_alloc(hdcp14_ta.ctx, bksv_len, TEE_SHM_MAPPED);
    if (shm_rev_buffer1 == NULL) {
        rtd_pr_hdcp_emerg("optee_hdcp1.4: no shm_buffer\n");
        ret = -ENOMEM;
        goto out;
    }

    // get share memory virtual addr for data accessing
    shm_rev_va1 = tee_shm_get_va(shm_rev_buffer1, 0);
    if (shm_rev_va1 == NULL) {
        ret = -ENOMEM;
        goto out;
    }
#if 0
    // get share memory physial addr for tee param
    rc = tee_shm_get_pa(shm_rev_buffer1, 0, &shm_rev_pa1);
    if (rc) {
        ret = -ENOMEM;
        goto out;
    }
#endif
    // alloc receive share memory
    shm_rev_buffer2 = tee_shm_alloc(hdcp14_ta.ctx, bkey_len, TEE_SHM_MAPPED);
    if (shm_rev_buffer2 == NULL) {
        rtd_pr_hdcp_emerg("optee_hdcp1.4: no shm_buffer\n");
        ret = -ENOMEM;
        goto out;
    }

    // get share memory virtual addr for data accessing
    shm_rev_va2 = tee_shm_get_va(shm_rev_buffer2, 0);
    if (shm_rev_va2 == NULL) {
        ret = -ENOMEM;
        goto out;
    }
#if 0
    // get share memory physial addr for tee param
    rc = tee_shm_get_pa(shm_rev_buffer2, 0, &shm_rev_pa2);
    if (rc) {
        ret = -ENOMEM;
        goto out;
    }
#endif
    /**
     *  optee user ta meminfo
     */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
    param[0].u.memref.shm = shm_rev_buffer1;
    param[0].u.memref.size = bksv_len;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
    param[1].u.memref.shm = shm_rev_buffer2;
    param[1].u.memref.size = bkey_len;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

    rc = tee_client_invoke_func(hdcp14_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp2: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }

    memcpy(bksv, (unsigned char *)shm_rev_va1, bksv_len);
    memcpy(bkey, (unsigned char *)shm_rev_va2, bkey_len);

out:
    if (shm_rev_buffer1) {
        tee_shm_free(shm_rev_buffer1);
    }

    if (shm_rev_buffer2) {
        tee_shm_free(shm_rev_buffer2);
    }
    return ret;
}

int optee_hdcp14_get_key(unsigned char *bksv, unsigned int bksv_len, unsigned char *bkey, unsigned int bkey_len)
{
    int rc = 0;
    struct tee_param *param = NULL;

    if (optee_hdcp14_init()) {
        rtd_pr_hdcp_emerg("%s-%d: optee_hdcp14_init failed!!\n", __FUNCTION__, __LINE__);
        return -EINVAL;
    }

    rtd_pr_hdcp_debug("get from optee\n");
    if (hdcp14_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp14: no ta context\n");
        goto out;
    }

    if((bksv == NULL) || (bkey == NULL)){
        rtd_pr_hdcp_err("optee_hdcp14: input param is NULL\n");
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_err("kcalloc: param failed\n");
        goto out;
    }

	rc = optee_hdcp14_get_key_handle(bksv, bksv_len, bkey, bkey_len, param);
out:
	if (param) {
        kfree(param);
    }
    optee_hdcp14_deinit();
    return rc;
}



void optee_hdcp14_dp_aksv_ksel_index(unsigned char key_sel_idx)
{
    int rc = 0;

    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;

    if (optee_hdcp14_init()) {
        rtd_pr_hdcp_emerg("%s-%d: optee_hdcp14_init failed!!\n", __FUNCTION__, __LINE__);
        return;
    }

    rtd_pr_hdcp_debug("get from optee\n");
    if (hdcp14_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp14: no ta context\n");
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_err("kcalloc: param failed\n");
        goto out;
    }

    memset(&arg, 0, sizeof(arg));
    arg.func = HDCP14_CMD_DP_AKSV_KSEL_INDEX;
    arg.session = hdcp14_ta.session;
    arg.num_params = TEE_NUM_PARAM;

    /**
    *  optee user ta meminfo
    */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
    param[0].u.value.a = key_sel_idx;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

    rc = tee_client_invoke_func(hdcp14_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp14:optee_hdcp14_dp_aksv_ksel_index invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        goto out;
    }

out:
    if (param) {
        kfree(param);
    }
    optee_hdcp14_deinit();
    return;
}

int optee_hdcp14_load_key_handle(HDMI_DP_HDCP_MODE_T hdmi_dp, unsigned char id, unsigned char nport, unsigned char load_mode)
{
    int ret = 0, rc = 0;

    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;

    rtd_pr_hdcp_debug("get from optee\n");
    if (hdcp14_ta.ctx == NULL) {
        rtd_pr_hdcp_err("optee_hdcp14: no ta context\n");
        ret = -EINVAL;
        goto out;
    }

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
    if (param == NULL) {
        rtd_pr_hdcp_err("kcalloc: param failed\n");
        ret = -EINVAL;
        goto out;
    }

    memset(&arg, 0, sizeof(arg));
    arg.func = id;
    arg.session = hdcp14_ta.session;
    arg.num_params = TEE_NUM_PARAM;

    /**
    *  optee user ta meminfo
    */
    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
    param[0].u.value.a = nport;//3; // user ta pool
    param[0].u.value.b = hdmi_dp;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
    param[1].u.value.a = load_mode;//1:write to register,  0:only read key to global vaule
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

    rc = tee_client_invoke_func(hdcp14_ta.ctx, &arg, param);
    if (rc || arg.ret) {
        rtd_pr_hdcp_emerg("optee_hdcp14: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
        ret = -EINVAL;
        goto out;
    }

out:
    if (param) {
        kfree(param);
    }

    return ret;
}

//load mode: 1:write to register,  0:only read key to global vaule
void optee_hdcp14_load_key(HDMI_DP_HDCP_MODE_T hdmi_dp, unsigned char load_mode)
{
    if (optee_hdcp14_init()) {
        rtd_pr_hdcp_emerg("%s-%d: optee_hdcp14_init failed!!\n", __FUNCTION__, __LINE__);
        return;
    }

    if (optee_hdcp14_load_key_handle(hdmi_dp, HDCP14_CMD_LOAD_KEY, 0, load_mode) == 0) {
        rtd_pr_hdcp_emerg("optee_hdcp14_load_key passed, hdmi_dp_mode:%d\n", hdmi_dp);
    }
    else {
        rtd_pr_hdcp_emerg("optee_hdcp14_load_key failed, hdmi_dp_mode:%d\n", hdmi_dp);
    }


    optee_hdcp14_deinit();
}
