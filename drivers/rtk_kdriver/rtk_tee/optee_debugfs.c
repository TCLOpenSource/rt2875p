#include <linux/version.h>
#include <rtd_log/rtd_module_log.h>
#include <linux/tee.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/tee_drv.h>
#include <linux/slab.h>
#include <linux/debugfs.h>
#include <mach/rtk_platform.h>
#include <linux/proc_fs.h>

#include <linux/notifier.h>
 #include<linux/reboot.h>

#include "optee_debugfs.h"
#include "rtk_optee_smc.h"
#include "optee_private.h"

static const uuid_t rtk_stats_uuid =
	UUID_INIT(0xd96a5b40, 0xe2c7, 0xb1af,
							0x87, 0x94, 0x10, 0x02, 0xa5, 0xd5, 0xc6, 0x1b);

static const uuid_t rtk_svp_uuid =
	UUID_INIT(0x15c66c21, 0x1275, 0x4040,
							0xaa, 0x61, 0xfb, 0x1e, 0xe4, 0x56, 0xda, 0xca);

#define STATS_CMD_PAGER_STATS		0
#define STATS_CMD_ALLOC_STATS		1
#define STATS_CMD_MEMLEAK_STATS		2
#define STATS_CMD_GET_CARVEDOUT_ADDR 10

#define TEE_NUM_PARAM 4
#define TEE_ALLOCATOR_DESC_LENGTH 32
#define TEE_TA_COUNT_MAX 10
#define TEEC_MEM_INPUT   0x00000001
#define TEEC_MEM_OUTPUT  0x00000002

#if 0
static struct dentry *optee_debugfs_root = NULL;
static struct dentry *meminfo_dentry = NULL;
static struct dentry *tameminfo_dentry = NULL;
#endif
// struct semaphore Mprison_Semaphore;

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

struct optee_ta {
    struct tee_context *ctx;
    __u32 session;
};

static struct optee_ta meminfo_ta;

static int optee_meminfo_match(struct tee_ioctl_version_data *data, const void *vers)
{
	return 1;
}

void rtk_optee_smccc_smc(unsigned long a0, unsigned long a1,
			    unsigned long a2, unsigned long a3,
			    unsigned long a4, unsigned long a5,
			    unsigned long a6, unsigned long a7,
			    struct arm_smccc_res *res)
{
	arm_smccc_smc(a0, a1, a2, a3, a4, a5, a6, a7, res);
}

/* 0: success, -N: failure (N: value) */
static int optee_meminfo_init(const uuid_t* uuid_in)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_open_session_arg arg;
    struct tee_ioctl_version_data vers = {
        .impl_id = TEE_IMPL_ID_OPTEE,
        .impl_caps = TEE_OPTEE_CAP_TZ,
        .gen_caps = TEE_GEN_CAP_GP,
    };

    memset(&meminfo_ta, 0, sizeof(meminfo_ta));
    meminfo_ta.ctx = tee_client_open_context(NULL, optee_meminfo_match, NULL, &vers);
    if(meminfo_ta.ctx == NULL) {
		rtd_pr_rtk_tee_err("optee_meminfo: no ta context\n");
		ret = -EINVAL;
		goto err;
	}

    memset(&arg, 0, sizeof(arg));
    memcpy(arg.uuid, uuid_in->b, TEE_IOCTL_UUID_LEN);
    arg.clnt_login = TEE_IOCTL_LOGIN_PUBLIC;
    rtd_pr_rtk_tee_debug("arg uuid %pUl \n", arg.uuid);

    rc = tee_client_open_session(meminfo_ta.ctx, &arg, NULL);
    if(rc){
        rtd_pr_rtk_tee_err("optee_meminfo: open_session failed ret %x arg %x", rc, arg.ret);
		ret = -EINVAL;
        goto err;
    }
    if (arg.ret) {
 		ret = -EINVAL;
 		goto err;
 	}

    meminfo_ta.session = arg.session;

    rtd_pr_rtk_tee_debug("open_session ok\n");
    return 0;

err:
 	if (meminfo_ta.session) {
 		tee_client_close_session(meminfo_ta.ctx, meminfo_ta.session);
        rtd_pr_rtk_tee_err("optee_meminfo: open failed close session \n");
 		meminfo_ta.session = 0;
 	}
 	if (meminfo_ta.ctx) {
 		tee_client_close_context(meminfo_ta.ctx);
        rtd_pr_rtk_tee_err("optee_meminfo: open failed close context\n");
 		meminfo_ta.ctx = NULL;
 	}
    rtd_pr_rtk_tee_err("open_session fail\n");

    return ret;
}

static void optee_meminfo_deinit(void)
{
 	if (meminfo_ta.session) {
 		tee_client_close_session(meminfo_ta.ctx, meminfo_ta.session);
		meminfo_ta.session = 0;
	}

 	if (meminfo_ta.ctx) {
 		tee_client_close_context(meminfo_ta.ctx);
 		meminfo_ta.ctx = NULL;
 	}
}

/* 0: success, -N: failure (N: value) */
static int optee_meminfo_dump(struct malloc_stats *kernel_stats, struct malloc_stats *user_stats)
{
    int ret = 0, rc = 0;
    struct tee_ioctl_invoke_arg arg;
    struct tee_param *param = NULL;
    struct tee_shm *shm_buffer = NULL;
    int shm_len = 0;
	phys_addr_t shm_pa = 0;
	void *shm_va = 0;

    rtd_pr_rtk_tee_debug("get from optee\n");

    if(meminfo_ta.ctx == NULL) {
		rtd_pr_rtk_tee_err("optee_meminfo: no ta context\n");
		ret = -EINVAL;
		goto out;
	}

    param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);

    memset(&arg, 0, sizeof(arg));
    arg.func = STATS_CMD_ALLOC_STATS;
    arg.session = meminfo_ta.session;
    arg.num_params = TEE_NUM_PARAM;

	// alloc share memory
    shm_len = sizeof(struct malloc_stats);
    shm_buffer = tee_shm_alloc(meminfo_ta.ctx, shm_len, TEE_SHM_MAPPED);
    if (shm_buffer == NULL) {
        rtd_pr_rtk_tee_err("optee_meminfo: no shm_buffer\n");
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

	rtd_pr_rtk_tee_debug("shm_buffer=%p, shm_len=%d, va=%p, pa=%x\n", (void *)shm_buffer, shm_len, shm_va, (unsigned int)shm_pa);
	if(user_stats){
        /**
         *  optee user ta meminfo
         */
		memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
		param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
		param[0].u.value.a = 3; // user ta pool
		param[0].u.value.b = 0;
		param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
		param[1].u.memref.shm = shm_buffer;
		param[1].u.memref.size = shm_len;
		param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
		param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

		rc = tee_client_invoke_func(meminfo_ta.ctx, &arg, param);
		if (rc || arg.ret) {
			rtd_pr_rtk_tee_err("optee_meminfo: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
			ret = -EINVAL;
		goto out;
	}

        memcpy(user_stats, (struct malloc_stats *)shm_va, sizeof(struct malloc_stats));

	}

	if(kernel_stats){
		/**
		*  optee kernel heap info
		*/
		memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
		param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
		param[0].u.value.a = 1; // kernel pool
		param[0].u.value.b = 0;
		param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
		param[1].u.memref.shm = shm_buffer;
		param[1].u.memref.size = shm_len;
		param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
		param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

		rc = tee_client_invoke_func(meminfo_ta.ctx, &arg, param);
		if (rc || arg.ret) {
			rtd_pr_rtk_tee_err("optee_meminfo: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
			ret = -EINVAL;
			goto out;
		}

		memcpy(kernel_stats, (struct malloc_stats *)shm_va, sizeof(struct malloc_stats));

	}

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

typedef enum display_protect_id
{
    DISP_CARVEDOUT_ID_MEMC = 0,
    DISP_CARVEDOUT_ID_MDOMAIN,
    DISP_CARVEDOUT_ID_OD,
    DISP_CARVEDOUT_ID_DI_NR,
    DISP_CARVEDOUT_NUM
}display_protect_id;

#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
int optee_carvedout_query(void)
{
	struct tee_ioctl_invoke_arg arg;
	struct tee_param *param = NULL;
	int ret = 0, rc = 0;
	unsigned int svp_module[DISP_CARVEDOUT_NUM][2] = {0};

	struct tee_shm *shm_buffer = NULL;
	int shm_len = 0;
	phys_addr_t shm_pa = 0;
	void *shm_va = 0;

	// sema_init(&Mprison_Semaphore, 1);
	if (optee_meminfo_init(&rtk_svp_uuid))
		goto out;

	if(meminfo_ta.ctx == NULL) {
		rtd_pr_rtk_tee_err("%s %d: no ta context\n", __func__, __LINE__);
		ret = -EINVAL;
		goto out;
	}

	memset(&arg, 0, sizeof(arg));
	arg.func = STATS_CMD_GET_CARVEDOUT_ADDR;
	arg.session = meminfo_ta.session;
	arg.num_params = TEE_NUM_PARAM;
	param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);

	svp_module[DISP_CARVEDOUT_ID_MEMC][1] = carvedout_buf_query_secure(CARVEDOUT_SCALER_MEMC, (void **)&svp_module[DISP_CARVEDOUT_ID_MEMC][0]);
	svp_module[DISP_CARVEDOUT_ID_MDOMAIN][1] = carvedout_buf_query_secure(CARVEDOUT_SCALER_MDOMAIN, (void **)&svp_module[DISP_CARVEDOUT_ID_MDOMAIN][0]);
	svp_module[DISP_CARVEDOUT_ID_OD][1] = carvedout_buf_query_secure(CARVEDOUT_SCALER_OD, (void **)&svp_module[DISP_CARVEDOUT_ID_OD][0]);
	svp_module[DISP_CARVEDOUT_ID_DI_NR][1] = carvedout_buf_query_secure(CARVEDOUT_SCALER_DI_NR, (void **)&svp_module[DISP_CARVEDOUT_ID_DI_NR][0]);

    // alloc share memory
    shm_len = sizeof(unsigned int) * 2 * DISP_CARVEDOUT_NUM;
    shm_buffer = tee_shm_alloc(meminfo_ta.ctx, shm_len, TEE_SHM_MAPPED | TEE_SHM_DMA_BUF);
    if (shm_buffer == NULL) {
        rtd_pr_rtk_tee_err("%s %d carvedout_query: no shm_buffer\n", __func__, __LINE__);
        ret = -ENOMEM;
        goto out;
    }

    // get share memory virtual addr for data accessing
    shm_va = tee_shm_get_va(shm_buffer, 0);
    if (shm_va == NULL) {
        rtd_pr_rtk_tee_err("%s %d get va fail\n", __func__, __LINE__);
        ret = -ENOMEM;
        goto out;
    }

    // get share memory physial addr for tee param
    rc = tee_shm_get_pa(shm_buffer, 0, &shm_pa);
    if (rc) {
        rtd_pr_rtk_tee_err("%s %d get pa fail\n", __func__, __LINE__);
        ret = -ENOMEM;
        goto out;
    }
    memcpy((unsigned int *)shm_va, svp_module, shm_len);

    memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);
    param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[0].u.memref.shm = shm_buffer;
    param[0].u.memref.size = shm_len;
    param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
    param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

	rc = tee_client_invoke_func(meminfo_ta.ctx, &arg, param);
	if (rc || arg.ret) {
		rtd_pr_rtk_tee_err("optee_meminfo carvedout query: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
		ret = -EINVAL;
		goto out;
	}
	rtd_pr_rtk_tee_err("optee_meminfo carvedout query: invoke success ret %x arg.ret %x\n", rc, arg.ret);

out:
    if (shm_buffer)
    {
        tee_shm_free(shm_buffer);
    }
	if (param)
    {
		kfree(param);
    }
	optee_meminfo_deinit();

    return ret;
}
#endif
static int optee_meminfo_show(struct seq_file *m, void *v)
{
	struct malloc_stats kernel_stats;
	struct malloc_stats user_stats;
	struct malloc_stats *stat_buffer;
	struct tee_shm_pool_stats pool_stats;
	int idx = 0;

	if (optee_meminfo_init(&rtk_stats_uuid)){
		rtd_pr_rtk_tee_err("optee_meminfo_init fail");
		return 0;
	}
	if (optee_meminfo_dump(&kernel_stats, &user_stats)){
		rtd_pr_rtk_tee_err("optee_meminfo_dump fail");
		return 0;
	}

	if(rtk_tee_shm_pool_get_stats(&pool_stats)){
		rtd_pr_rtk_tee_err("No tee_shm_pool_stat\n");
		return 0;
	}

	optee_meminfo_deinit();

	/**
	 *  optee kernel heap info
	 */
	stat_buffer = &kernel_stats;

	seq_printf(m,
		"========= OPTEE Kernel Heap ==========\n"
		"allocated:                    %8d\n"
		"max_allocated:                %8d\n"
		"total_size:                   %8d\n"
		"free:                         %8d\n"
		"num_alloc_fail:               %8d\n"
		"biggest_alloc_fail:           %8d\n"
		"biggest_alloc_fail_used:      %8d\n"
		,
		//stat_buffer->desc,
		(int)stat_buffer->allocated,
		stat_buffer->max_allocated,
		stat_buffer->size,
		stat_buffer->size - stat_buffer->allocated,
		stat_buffer->num_alloc_fail,
		stat_buffer->biggest_alloc_fail,
		stat_buffer->biggest_alloc_fail_used
	);


	/**
	 *  optee user ta meminfo
	 */
	stat_buffer = &user_stats;

	seq_printf(m,
		"========= OPTEE TA RAM Secure ========\n"
		"allocated:                    %8d\n"
		"max_allocated:                %8d\n"
		"total_size:                   %8d\n"
		"free:                         %8d\n"
        "the number of cur ta:         %8d\n"
        "the number of max ta:         %8d\n"
		,
		(int)stat_buffer->allocated,
		stat_buffer->max_allocated,
		stat_buffer->size,
		stat_buffer->size - stat_buffer->allocated,
        stat_buffer->cur_alloc_ta_count,
        stat_buffer->max_alloc_ta_count
	    );

    if(stat_buffer->cur_alloc_ta_count > 0) {
        for(idx = 0; idx < stat_buffer->cur_alloc_ta_count; idx++) {
            seq_printf(m,
            "|-cur_TA idx:                 %8d\n"
            "| cur_TA uuid_time_low:       %08x\n"
            "| cur_TA stack_size:          %8d\n"
            "|-cur_TA code_size:           %8d\n"
            ,
            idx + 1,
            stat_buffer->ta_minfo_cur[idx].uuid_tL,
            stat_buffer->ta_minfo_cur[idx].stack_size,
            stat_buffer->ta_minfo_cur[idx].code_size
            );
        }
    }
    if(stat_buffer->max_alloc_ta_count > 0) {
        for(idx = 0; idx < stat_buffer->max_alloc_ta_count; idx++) {
           seq_printf(m,
           "|-max_TA idx:                 %8d\n"
           "| max_TA uuid_time_low        %08x\n"
           "| max_TA stack_size:          %8d\n"
           "|-max_TA code_size:           %8d\n"
           ,
           idx + 1,
           stat_buffer->ta_minfo_max[idx].uuid_tL,
           stat_buffer->ta_minfo_max[idx].stack_size,
           stat_buffer->ta_minfo_max[idx].code_size
           );
        }
    }
	/**
	*  optee shared memory (OPTEE SHM MEM) info
	*/
	seq_printf(m,
		"========= OPTEE Shared Memory ========\n"
		"-major pool for user      (unit %4dB)\n"
		" major total_size             %8d\n"
		" major allocated              %8d\n"
		" major max_allocated          %8d\n"
		" major free                   %8d\n"
		"-private pool for kernel  (unit %4dB)\n"
		" private total_size           %8d\n"
		" private allocated            %8d\n"
		" private max_allocated        %8d\n"
		" private free                 %8d\n"
		,
		pool_stats.dmabuf_mgr_aligned_unit,
		pool_stats.dmabuf_mgr_total,
		pool_stats.dmabuf_mgr_allocated,
		pool_stats.dmabuf_mgr_max_allocated,
		pool_stats.dmabuf_mgr_total - pool_stats.dmabuf_mgr_allocated,
		pool_stats.private_mgr_aligned_unit,
		pool_stats.private_mgr_total,
		pool_stats.private_mgr_allocated,
		pool_stats.private_mgr_max_allocated,
		pool_stats.private_mgr_total - pool_stats.private_mgr_allocated
	);

	return 0;
}

static int optee_tameminfo_show(struct seq_file *m, void *v)
{
	struct malloc_stats user_stats;
	struct malloc_stats *stat_buffer;
	int idx = 0;

	if (optee_meminfo_init(&rtk_stats_uuid))
		return 0;
    /* only show user ta info */
	if (optee_meminfo_dump(NULL, &user_stats))
		return 0;
	optee_meminfo_deinit();

	/**
	 *  optee user ta meminfo
	 */
	stat_buffer = &user_stats;
	seq_printf(m,
		"========= OPTEE TA RAM Secure ========\n"
		"allocated:                    %8d\n"
		"max_allocated:                %8d\n"
		"total_size:                   %8d\n"
		"free:                         %8d\n"
		"the number of cur ta:         %8d\n"
		"the number of max ta:         %8d\n"
		,
		(int)stat_buffer->allocated,
		stat_buffer->max_allocated,
		stat_buffer->size,
		stat_buffer->size - stat_buffer->allocated,
		stat_buffer->cur_alloc_ta_count,
		stat_buffer->max_alloc_ta_count
		);
	if(stat_buffer->cur_alloc_ta_count > 0)
        	for(idx = 0; idx < stat_buffer->cur_alloc_ta_count ; idx ++)
            		seq_printf(m,
                	"|-cur_TA idx:                 %8d\n"
                	"| cur_TA uuid_time_low:       %08x\n"
                	"| cur_TA stack_size:          %8d\n"
                	"|-cur_TA code_size:           %8d\n"
                	,
                	idx + 1,
                	stat_buffer->ta_minfo_cur[idx].uuid_tL,
                	stat_buffer->ta_minfo_cur[idx].stack_size,
                	stat_buffer->ta_minfo_cur[idx].code_size
                	);
	if(stat_buffer->max_alloc_ta_count > 0)
        	for(idx = 0; idx < stat_buffer->max_alloc_ta_count ; idx ++)
            	seq_printf(m,
                	"|-max_TA idx:                 %8d\n"
                	"| max_TA uuid_time_low        %08x\n"
                	"| max_TA stack_size:          %8d\n"
                	"|-max_TA code_size:           %8d\n"
                	,
                	idx + 1,
                	stat_buffer->ta_minfo_max[idx].uuid_tL,
                	stat_buffer->ta_minfo_max[idx].stack_size,
                	stat_buffer->ta_minfo_max[idx].code_size
                	);

	return 0;
}
static int optee_meminfo_open(struct inode *inode, struct file *file)
{
	return single_open(file, optee_meminfo_show, NULL);
}
static int optee_tameminfo_open(struct inode *inode, struct file *file)
{
	return single_open(file, optee_tameminfo_show, NULL);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
static const struct proc_ops optee_meminfo_fops = {
	.proc_open           = optee_meminfo_open,
	.proc_read           = seq_read,
	.proc_lseek         = seq_lseek,
	.proc_release        = single_release,
};
#else
static const struct file_operations optee_meminfo_fops = {
	.open           = optee_meminfo_open,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = single_release,
};
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
static const struct proc_ops optee_tameminfo_fops = {
	.proc_open           = optee_tameminfo_open,
	.proc_read           = seq_read,
	.proc_lseek         = seq_lseek,
	.proc_release        = single_release,
};
#else
static const struct file_operations optee_tameminfo_fops = {
	.open           = optee_tameminfo_open,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = single_release,
};
#endif

char optee_git_version[13] = {"not surport"};
bool optee_msg_api_get_optee_gitversion(optee_invoke_fn *invoke_fn)
{
	union {
		struct arm_smccc_res smccc;
		struct optee_smc_calls_get_git_version_result result;
	} res;

	memset(&res, 0, sizeof(res.smccc));
	invoke_fn(OPTEE_SMC_GET_GIT_VERSION, 0, 0, 0, 0, 0, 0, 0, &(res.smccc));

	if(res.result.a0 == OPTEE_SMC_RETURN_OK)
	{
		memset(optee_git_version, 0, 13);
		memcpy(optee_git_version, res.result.git_version, 12);
		rtd_pr_rtk_tee_info("optee git version: %s\n", optee_git_version);
		return true;
	}
	else
	{
		rtd_pr_rtk_tee_info("get optee git version fail\n");
		return false;
	}
}

static bool optee_msg_api_set_cw_write_en(optee_invoke_fn *invoke_fn)
{
	struct arm_smccc_res res;

	invoke_fn(OPTEE_SMC_SET_TP_CW_WEN, 0xCCCC4357, 0, 0, 0, 0, 0, 0, &res);

	if(res.a0 == OPTEE_SMC_RETURN_OK)
		return true;
	return false;
}

static bool optee_msg_api_set_shutdown(optee_invoke_fn *invoke_fn)
{
	struct arm_smccc_res res;

	invoke_fn(OPTEE_SMC_SHUTDOWN, 0x444f574e/*DOWN*/, 0, 0, 0, 0, 0, 0, &res);

	if(res.a0 == OPTEE_SMC_RETURN_OK)
		return true;
	return false;
}

static int rtktee_shutdown(struct notifier_block *self, unsigned long event, void *data)
{
	optee_invoke_fn *invoke_fn = rtk_optee_smccc_smc;
	rtd_pr_rtk_tee_info("send shutdown flag to tee");
	optee_msg_api_set_shutdown(invoke_fn);
	return NOTIFY_OK;
}

static struct notifier_block rtktee_reboot_notifier = {
	.notifier_call = rtktee_shutdown,
};

#if 1 /* device node in /proc/rtktee */
struct proc_dir_entry *rtktee_dir;

extern int rtk_tee_shm_collect_init(void);
extern void rtk_tee_shm_collect_exit(void);
static int optee_rtk_debugfs_init(void)
{
	optee_invoke_fn *invoke_fn = rtk_optee_smccc_smc;

	if (optee_msg_api_get_optee_gitversion(invoke_fn))
		rtd_pr_rtk_tee_warn("get optee git version success\n");
	else
		rtd_pr_rtk_tee_warn("get optee git version error\n");

	// RTK_FIXME_64 : added this block
	if (!optee_msg_api_set_cw_write_en(invoke_fn)) {
		rtd_pr_rtk_tee_info("set cw write enable error\n");
	}

	rtktee_dir = proc_mkdir("rtktee", NULL);

	proc_create("tee_info", 0, rtktee_dir, &optee_meminfo_fops);
	proc_create("ta_info", 0, rtktee_dir, &optee_tameminfo_fops);

// RTK_FIXME_64 : added
#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
	optee_carvedout_query();
#endif
	rtk_tee_shm_collect_init();
	register_reboot_notifier(&rtktee_reboot_notifier);

	rtd_pr_rtk_tee_err("RTK tee debugfs install success");

    return 0;
}

static void optee_rtk_debugfs_exit(void)
{
	remove_proc_entry("tee_info", rtktee_dir);
	remove_proc_entry("ta_info", rtktee_dir);
	remove_proc_entry("rtktee", NULL);

	rtk_tee_shm_collect_exit();
	unregister_reboot_notifier(&rtktee_reboot_notifier);
}

module_init(optee_rtk_debugfs_init);
module_exit(optee_rtk_debugfs_exit);
#endif

MODULE_AUTHOR("RTK");
MODULE_DESCRIPTION("RTK OP-TEE driver");
MODULE_VERSION("1.0");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_SOFTDEP("pre: tee optee");
