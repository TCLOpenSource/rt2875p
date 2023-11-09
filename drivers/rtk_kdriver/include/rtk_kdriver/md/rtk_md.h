#ifndef __RTK_MD_H__
#define __RTK_MD_H__


/*************Internel API for itself to call**********/
extern __init int rtk_md_init(struct device *md_device, int irq_num);
extern void rtk_md_uninit(int irq_num);
extern int rtk_md_memcpy(uint32_t dst_phys_addr,
                     uint32_t src_phys_addr, uint32_t length, int32_t op, uint32_t is_wait);
extern int rtk_md_do_cmds(uint8_t *pbyCommandBuffer, uint32_t lCommandLength);
extern int rtk_md_wait_hw_stopped(uint64_t timeout);
extern int rtk_md_get_inst_count(uint64_t *p_u64InstCnt);
extern int rtk_md_get_issued_inst_count(uint64_t *p_u64IssuedInstCnt);


#ifdef CONFIG_PM
extern int rtk_md_poweroff(void);
extern int rtk_md_suspend_noirq(void);
extern int rtk_md_resume_noirq(void);
extern int rtk_md_resume_early(void);
extern int rtk_md_restore(void);
extern int rtk_md_freeze_noirq(void);
#endif

/*************The MD Global APIs for other modules in kernel to call**********/
extern bool smd_waitComplete(uint64_t handle,
                             uint64_t timeout, uint64_t *hw_cnt);
extern bool smd_checkFinish(uint64_t handle);
extern void smd_checkComplete(void);
extern uint64_t smd_memcpy(uint32_t addr_dst,
                           uint32_t addr_src, int32_t len);
extern int smd_memfill(uint32_t addrdst, int32_t len, uint32_t val);

/**************************FDMA releated******************************************/
// fdma_max_xfer
#define FDMA_W_XFER_LEN_128  (0)
#define FDMA_W_XFER_LEN_256  (1)
#define FDMA_W_XFER_LEN_512  (2)
#define FDMA_W_XFER_DMA_LEN  (3)

#define FDMA_R_XFER_LEN_384  (0)
#define FDMA_R_XFER_LEN_768  (1)
#define FDMA_R_XFER_LEN_1536 (2)
#define FDMA_R_XFER_DMA_LEN  (3)

// fdma_dir
#define FDMA_FLASH_2_DDR (0)
#define FDMA_DDR_2_FLASH (1)

extern int smd_fdma_copy(uint32_t flash_address, unsigned char *buf,
              uint32_t len, uint32_t fdma_max_xfer, uint32_t fdma_dir);
#endif
