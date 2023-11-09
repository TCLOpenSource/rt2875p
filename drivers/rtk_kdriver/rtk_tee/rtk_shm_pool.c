#include <rtd_log/rtd_module_log.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/tee_drv.h>

#include <linux/device.h>
#include <linux/dma-buf.h>
#include <linux/genalloc.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#include <linux/arm-smccc.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include "optee_private.h"
#include "optee_smc.h"

#include "optee_debugfs.h"

#define MAX_SYMBOL_LEN	64

#include <linux/tee_drv.h>
#include <linux/cdev.h>

#define TEE_MAX_DEV_NAME_LEN         32

#ifdef CONFIG_KPROBES
/*kprobe enabled, use kprobe to collect tee shm info*/
#define USE_KPROBE_TO_COLLECT_SHM_INFO 1
#endif

#ifdef USE_KPROBE_TO_COLLECT_SHM_INFO
static char symbol_alloc[MAX_SYMBOL_LEN] = "pool_op_gen_alloc";
static char symbol_free[MAX_SYMBOL_LEN] = "pool_op_gen_free";

static struct kprobe kp_free = {
	.symbol_name	= symbol_free,
};
#endif

struct rtk_pool_info{
	u64 addr;
	int max_allocated;
	int allocated;
	int total;
	spinlock_t shm_lock;
};

#ifdef USE_KPROBE_TO_COLLECT_SHM_INFO
/* per-instance private data */
struct rtk_kpret_shm_data {
	struct tee_shm *shm;
	struct rtk_pool_info *rtk_poolm;
	size_t size;
};

static struct kretprobe kpret_alloc = {
	.kp.symbol_name	= symbol_alloc,
	.data_size		= sizeof(struct rtk_kpret_shm_data),
	/* Probe up to 20 instances concurrently. */
	.maxactive		= 20,
};
#endif

static struct rtk_pool_info priv_poolm;
static struct rtk_pool_info dmabuf_poolm;

struct tee_shm_pool {
    struct tee_shm_pool_mgr *private_mgr;
    struct tee_shm_pool_mgr *dma_buf_mgr;
};

struct tee_device {
    char name[TEE_MAX_DEV_NAME_LEN];
    const struct tee_desc *desc;
    int id;
    unsigned int flags;

    struct device dev;
    struct cdev cdev;

    size_t num_users;
    struct completion c_no_users;
    struct mutex mutex; /* protects num_users and idr */

    struct idr idr;
    struct tee_shm_pool *pool;
};

static int rtk_optee_ctx_match(struct tee_ioctl_version_data *ver, const void *data)
{
	if (ver->impl_id == TEE_IMPL_ID_OPTEE)
		return 1;
	else
		return 0;
}

static void get_tee_poolm_addr(void)
{
	if(priv_poolm.addr == 0 || dmabuf_poolm.addr == 0)
	{
		struct tee_context *ctx = NULL;

		/* Open context with OP-TEE driver */
		ctx = tee_client_open_context(NULL, rtk_optee_ctx_match, NULL, NULL);
		if (IS_ERR(ctx) || !ctx->teedev || !ctx->teedev->pool) {
			rtd_pr_rtk_tee_err("rtk_shm_pool invoke tee_client_open_context failed\n");
			return;
		}

		dmabuf_poolm.addr = (unsigned long long)ctx->teedev->pool->dma_buf_mgr;
		priv_poolm.addr = (unsigned long long)ctx->teedev->pool->private_mgr;
		//rtd_pr_rtk_tee_info("get_tee_poolm_addr dmabuf_poolm addr=%lx", dmabuf_poolm.addr);
		//rtd_pr_rtk_tee_info("get_tee_poolm_addr priv_poolm addr=%lx", priv_poolm.addr);

		tee_client_close_context(ctx);
	}
}

#ifdef USE_KPROBE_TO_COLLECT_SHM_INFO
static int free_handler_pre(struct kprobe *p, struct pt_regs *regs)
{
	struct tee_shm *shm = (struct tee_shm *)(regs->regs[1]);
	struct rtk_pool_info *poolm = &priv_poolm;

	get_tee_poolm_addr();
	if(regs->regs[0] != poolm->addr){
		poolm = &dmabuf_poolm;
	}

	spin_lock(&poolm->shm_lock);
	poolm->allocated -= shm->size;
	spin_unlock(&poolm->shm_lock);

	if(poolm->allocated < 0){
		rtd_pr_rtk_tee_err("optee freed shared memory incorrect shm_size=%d pool_cur_size=%d < 0 pool_total=%d\n",shm->size, poolm->allocated, poolm->total);
		dump_stack();
		poolm->allocated = 0;
	}
	return 0;
}
#endif

int rtk_tee_shm_pool_get_stats(struct tee_shm_pool_stats * pool_stats_out)
{
	struct tee_shm_pool_stats *pool_stats;
	struct gen_pool *genpool;
	size_t pool_unit;

	struct tee_shm_pool_mgr *private_mgr = (struct tee_shm_pool_mgr *)(priv_poolm.addr);
	struct tee_shm_pool_mgr *dma_buf_mgr = (struct tee_shm_pool_mgr *)(dmabuf_poolm.addr);

#ifndef USE_KPROBE_TO_COLLECT_SHM_INFO
	struct tee_context *ctx = NULL;

	ctx = tee_client_open_context(NULL, rtk_optee_ctx_match, NULL, NULL);
	if (IS_ERR(ctx) || !ctx->teedev || !ctx->teedev->pool) {
		rtd_pr_rtk_tee_err("rtk_shm_pool invoke tee_client_open_context fail\n");
		return -1;
	}
#endif

	pool_stats = kzalloc(sizeof(struct tee_shm_pool_stats), GFP_KERNEL);

	if(!pool_stats)
	{
		rtd_pr_rtk_tee_err("rtk_tee_shm_pool_get_stats pool_stats kzalloc fail");
		return -1;
	}

	if(priv_poolm.addr==0 || dmabuf_poolm.addr==0)
	{
		rtd_pr_rtk_tee_err("rtk_tee_shm_pool_get_stats poolm not inited");
		kfree(pool_stats);
		return -1;
	}

	genpool= private_mgr->private_data;
	pool_unit = 1 << genpool->min_alloc_order;
	pool_stats->private_mgr_aligned_unit = pool_unit;
	pool_stats->private_mgr_total= priv_poolm.total;

	genpool = dma_buf_mgr->private_data;
	pool_unit = 1 << genpool->min_alloc_order;
	pool_stats->dmabuf_mgr_aligned_unit = pool_unit;
	pool_stats->dmabuf_mgr_total = dmabuf_poolm.total;

	spin_lock(&priv_poolm.shm_lock);
#ifndef USE_KPROBE_TO_COLLECT_SHM_INFO
	priv_poolm.allocated = gen_pool_size(ctx->teedev->pool->private_mgr->private_data) - gen_pool_avail(ctx->teedev->pool->private_mgr->private_data);
	priv_poolm.max_allocated = 0;
#endif
	pool_stats->private_mgr_allocated = priv_poolm.allocated;
	pool_stats->private_mgr_max_allocated = priv_poolm.max_allocated;
	//rtd_pr_rtk_tee_err("rtk  pri allocated=%d max=%d",pool_stats->private_mgr_allocated,pool_stats->private_mgr_max_allocated);
	//rtd_pr_rtk_tee_err("pool pri allocated=%d",gen_pool_size(ctx->teedev->pool->private_mgr->private_data) - gen_pool_avail(ctx->teedev->pool->private_mgr->private_data));
	spin_unlock(&priv_poolm.shm_lock);

	spin_lock(&dmabuf_poolm.shm_lock);
#ifndef USE_KPROBE_TO_COLLECT_SHM_INFO
	dmabuf_poolm.allocated = gen_pool_size(ctx->teedev->pool->dma_buf_mgr->private_data) - gen_pool_avail(ctx->teedev->pool->dma_buf_mgr->private_data);
	dmabuf_poolm.max_allocated = 0;
#endif
	pool_stats->dmabuf_mgr_allocated = dmabuf_poolm.allocated;
	pool_stats->dmabuf_mgr_max_allocated = dmabuf_poolm.max_allocated;
	//rtd_pr_rtk_tee_err("rtk  dma allocated=%d max=%d",pool_stats->dmabuf_mgr_allocated,pool_stats->dmabuf_mgr_max_allocated);
	//rtd_pr_rtk_tee_err("pool dma allocated=%d",gen_pool_size(ctx->teedev->pool->dma_buf_mgr->private_data) - gen_pool_avail(ctx->teedev->pool->dma_buf_mgr->private_data));
	spin_unlock(&dmabuf_poolm.shm_lock);

	memcpy(pool_stats_out, pool_stats, sizeof(struct tee_shm_pool_stats));

	// tee_client_close_context(ctx);

	kfree(pool_stats);
	return 0;
}

#ifdef USE_KPROBE_TO_COLLECT_SHM_INFO
#define CONFIG_OPTEE_SHM_NUM_PRIV_PAGES 1
#define OPTEE_SHM_NUM_PRIV_PAGES      CONFIG_OPTEE_SHM_NUM_PRIV_PAGES

/* Here we use the alloc_entry_handler to collect tee momery pool status */
static int alloc_entry_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	struct rtk_kpret_shm_data *data = NULL;
	struct tee_shm_pool_mgr *pool_mgr = (struct tee_shm_pool_mgr *)(regs->regs[0]);
	struct tee_shm *shm = (struct tee_shm *)(regs->regs[1]);
	size_t size = regs->regs[2];
	struct gen_pool *genpool = pool_mgr->private_data;
	size_t s = roundup(size, 1 << genpool->min_alloc_order);

	struct rtk_pool_info *poolm = &priv_poolm;

	get_tee_poolm_addr();
	if(regs->regs[0] != poolm->addr)
		poolm = &dmabuf_poolm;

	spin_lock(&(poolm->shm_lock));
	poolm->allocated += s;//shm->size here is still zero

	if(poolm->allocated > poolm->total)
		rtd_pr_rtk_tee_err("optee allocated shared memory incorrect shm_size=%d pool_cur_alloc=%d > pool_total=%d\n", shm->size, poolm->allocated, poolm->total);

	if(poolm->allocated > poolm->max_allocated)
		poolm->max_allocated = poolm->allocated;

	spin_unlock(&(poolm->shm_lock));

	data = (struct rtk_kpret_shm_data *)ri->data;
	data->shm = shm;
	data->rtk_poolm = poolm;
	data->size = s;

	return 0;
}

/*
 * Return-probe handler: Log the return value and duration. Duration may turn
 * out to be zero consistently, depending upon the granularity of time
 * accounting on the platform.
 */
static int alloc_ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	unsigned long retval = regs_return_value(regs);
	struct rtk_kpret_shm_data *data = (struct rtk_kpret_shm_data *)ri->data;

	if(retval != 0){
		spin_lock(&(data->rtk_poolm->shm_lock));
		data->rtk_poolm->allocated -= data->size;

		if(data->rtk_poolm->allocated < 0){
			rtd_pr_rtk_tee_err("optee alloc shared memory fail shm_size=%d pool_cur_size=%d < 0 pool_total=%d\n",data->shm->size, data->rtk_poolm->allocated, data->rtk_poolm->total);
			// dump_stack();
			data->rtk_poolm->allocated = 0;
		}

		spin_unlock(&(data->rtk_poolm->shm_lock));
	}

	return 0;
}
#endif

int rtk_tee_shm_collect_init(void)
{
#ifdef USE_KPROBE_TO_COLLECT_SHM_INFO
	int ret;
#endif
	// const int priv_sz = OPTEE_SHM_NUM_PRIV_PAGES * PAGE_SIZE;
	struct tee_shm_pool_mgr *poolm = NULL;

	get_tee_poolm_addr();

	poolm = (struct tee_shm_pool_mgr *) priv_poolm.addr;
	priv_poolm.total = gen_pool_size(poolm->private_data);//priv_sz
	priv_poolm.allocated = priv_poolm.total - gen_pool_avail(poolm->private_data);
	priv_poolm.max_allocated = priv_poolm.allocated;
	spin_lock_init(&priv_poolm.shm_lock);

	poolm = (struct tee_shm_pool_mgr *) dmabuf_poolm.addr;
	dmabuf_poolm.total = gen_pool_size(poolm->private_data);
	dmabuf_poolm.allocated = dmabuf_poolm.total - gen_pool_avail(poolm->private_data);
	dmabuf_poolm.max_allocated = dmabuf_poolm.allocated;
	spin_lock_init(&dmabuf_poolm.shm_lock);

#ifdef USE_KPROBE_TO_COLLECT_SHM_INFO
	kp_free.pre_handler = free_handler_pre;
	kp_free.post_handler = NULL;

	kpret_alloc.handler = alloc_ret_handler;
	kpret_alloc.entry_handler = alloc_entry_handler;

	ret = register_kretprobe(&kpret_alloc);
	if (ret < 0) {
		rtd_pr_rtk_tee_err("register_kretprobe kpret_alloc failed, returned %d\n", ret);
		return -1;
	}

	ret = register_kprobe(&kp_free);
	if (ret < 0) {
		rtd_pr_rtk_tee_err("register_kprobe kp_free failed, returned %d\n", ret);
		return ret;
	}
#endif

	return 0;
}

void rtk_tee_shm_collect_exit(void)
{
#ifdef USE_KPROBE_TO_COLLECT_SHM_INFO
	unregister_kretprobe(&kpret_alloc);
	unregister_kprobe(&kp_free);
#endif
}

// module_init(rtk_tee_shm_collect_init);
// module_exit(rtk_tee_shm_collect_exit)
// MODULE_LICENSE("GPL");
