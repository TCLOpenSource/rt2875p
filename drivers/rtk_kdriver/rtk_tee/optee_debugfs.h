#ifndef _OPTEE_DEBUGFS_H
#define _OPTEE_DEBUGFS_H

struct tee_context;

struct tee_shm_pool_stats{
	int private_mgr_allocated;
	int private_mgr_total;
	int private_mgr_aligned_unit;
	int private_mgr_max_allocated;
	int dmabuf_mgr_allocated;
	int dmabuf_mgr_total;
	int dmabuf_mgr_aligned_unit;
	int dmabuf_mgr_max_allocated;
};

#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
int optee_carvedout_query(void);
#endif

extern int rtk_tee_shm_pool_get_stats(struct tee_shm_pool_stats * pool_stats_out);
#endif
