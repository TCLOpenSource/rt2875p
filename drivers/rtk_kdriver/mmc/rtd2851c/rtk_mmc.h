/*************************************************************************
	> File Name: rtk_mmc.h
	> Author: bennychen
	> Mail: bennytschen@realtek.com
	> Created Time: Wed, 07 Apr 2021 10:00:51 +0800
 ************************************************************************/

#ifndef _RTK_MMC_H
#define _RTK_MMC_H

/* need to check relative path */
#include "../../mmc/core/queue.h"
#include <linux/mmc/ioctl.h>

/* EXT_CSD_SLEEP_NOTIFICATION may be defined in linux/mmc/mmc.h */
#ifndef EXT_CSD_SLEEP_NOTIFICATION
#define EXT_CSD_SLEEP_NOTIFICATION  4   /* add for mmc5.0  */
#endif

/* MMC_CAP2_RTK_SKIP_DOWNSPEED only be used by RTK.
   This option number may be defined by native in future */
#define MMC_CAP2_RTK_SKIP_DOWNSPEED (1 << 31)   /* skip down speed hack */

/* hack on mmc/core/block.c */
struct mmc_blk_data {
	struct device	*parent;
	struct gendisk	*disk;
	struct mmc_queue queue;
	struct list_head part;
	struct list_head rpmbs;

	unsigned int	flags;
#define MMC_BLK_CMD23	(1 << 0)	/* Can do SET_BLOCK_COUNT for multiblock */
#define MMC_BLK_REL_WR	(1 << 1)	/* MMC Reliable write support */

	unsigned int	usage;
	unsigned int	read_only;
	unsigned int	part_type;
	unsigned int	reset_done;
#define MMC_BLK_READ		BIT(0)
#define MMC_BLK_WRITE		BIT(1)
#define MMC_BLK_DISCARD		BIT(2)
#define MMC_BLK_SECDISCARD	BIT(3)
#define MMC_BLK_CQE_RECOVERY	BIT(4)

	/*
	 * Only set in main mmc_blk_data associated
	 * with mmc_card with dev_set_drvdata, and keeps
	 * track of the current selected device partition.
	 */
	unsigned int	part_curr;
	struct device_attribute force_ro;
	struct device_attribute power_ro_lock;
	int	area_type;

	/* debugfs files (only in main mmc_blk_data) */
	struct dentry *status_dentry;
	struct dentry *ext_csd_dentry;
};

struct mmc_blk_ioc_data {
	struct mmc_ioc_cmd ic;
	unsigned char *buf;
	u64 buf_bytes;
	struct mmc_rpmb_data *rpmb;
};

/* RTK ioctl */
#define RTK_FLAG_SHT (16) //The flags of struct mmc_command is from bit0 to bit10. Therefore, We use bit16 to bit31.
#define GET_RTK_IOCTL_FLAG(val) \
    ((val & 0xffff0000)>>RTK_FLAG_SHT)
#define GET_CMD_IOCTL_FLAG(val) \
    (val & 0x0000FFFF)

#define RTK_IOCTL_ERASE         (0x01)
#define RTK_IOCTL_OPEN          (0x02)
#define RTK_IOCTL_CLOSE         (0x03)
#define RTK_IOCTL_PON_SHORT     (0x04)
#define RTK_IOCTL_PON_LONG      (0x05)
#define RTK_IOCTL_SMART_REPORT  (0x06)
#define RTK_IOCTL_GET_CID       (0x07)
#define RTK_IOCTL_GET_CSD       (0x08)

#define IOCTL_GET_CID   1
#define IOCTL_GET_CSD   2
#define IOCTL_GET_ECSD  3

int mmc_rtk_ioctl_cmd(
        struct mmc_blk_data *md,
        struct mmc_blk_ioc_data *idata );

/* RTK tuning */
#define MMC_RPHASE50_TUNING 99

extern bool mmc_tuning_50_done(void);
extern bool mmc_tuning_400_done(void);
extern void mmc_execute_tuning_50(struct mmc_host *);
extern void mmc_execute_tuning_400(struct mmc_host *);
extern int rtkmmc_send_tuning(struct mmc_host *host, u32 opcode, int *cmd_error);
extern void set_emmc_pin_mux(void);

#endif /* #ifndef _RTK_MMC_H */

