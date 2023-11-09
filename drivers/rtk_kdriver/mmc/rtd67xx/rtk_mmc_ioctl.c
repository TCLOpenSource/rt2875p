/*************************************************************************
	> File Name: rtk_mmc_ioctl.c
	> Author: bennychen
	> Mail: bennytschen@realtek.com
	> Created Time: Wed, 07 Apr 2021 10:15:36 +0800
 ************************************************************************/

/* need to check relative path */
#include "../../mmc/core/mmc_ops.h"
#include "../../mmc/core/queue.h"
#include "../../mmc/core/block.h"
#include "../../mmc/core/core.h"
#include "../../mmc/core/card.h"
#include "../../mmc/core/host.h"
#include "../../mmc/core/bus.h"
#include "../../mmc/core/mmc_ops.h"
#include "../../mmc/core/quirks.h"
#include "../../mmc/core/sd_ops.h"

#include "rtk_mmc.h"
#include "rtkemmc_dbg.h"

#include <linux/moduleparam.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/hdreg.h>
#include <linux/kdev_t.h>
#include <linux/blkdev.h>
#include <linux/cdev.h>
#include <linux/mutex.h>
#include <linux/scatterlist.h>
#include <linux/string_helpers.h>
#include <linux/delay.h>
#include <linux/capability.h>
#include <linux/compat.h>
#include <linux/pm_runtime.h>
#include <linux/idr.h>
#include <linux/debugfs.h>

#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/mmc/mmc.h>

#include <linux/uaccess.h>
#ifndef CONFIG_MMC_RTKEMMC_PLUS_MODULE
#include <linux/version.h>

/* defined in drivers/mmc/core/mmc_ops.h */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 13, 0))
extern int __mmc_switch(struct mmc_card *card, u8 set, u8 index, u8 value,
		unsigned int timeout_ms, unsigned char timing,
		bool use_busy_signal, bool send_status,	bool retry_crc_err);
#else
extern int __mmc_switch(struct mmc_card *card, u8 set, u8 index, u8 value,
        unsigned int timeout_ms, unsigned char timing,
        bool send_status, bool retry_crc_err, unsigned int retries);
#endif
#endif

/* duplicated from mmc_poweroff_notify in linux/drivers/mmc/core/mmc.c */
int export_mmc_poweroff_notify(struct mmc_card *card, unsigned int notify_type){
    unsigned int timeout = card->ext_csd.generic_cmd6_time;
    int err;
#ifdef CONFIG_MMC_RTKEMMC_PLUS_MODULE
    err = -1;
    EM_ERR("%s: RTK Power Off Notification not support on module mode\n",
                mmc_hostname(card->host), timeout);
#else
    /* Use EXT_CSD_POWER_OFF_SHORT as default notification type. */
    if (notify_type == EXT_CSD_POWER_OFF_LONG)
        timeout = card->ext_csd.power_off_longtime;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 13, 0))
    err = __mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
            EXT_CSD_POWER_OFF_NOTIFICATION,
            notify_type, timeout, 0, true, false, false);
#else
    err = __mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
            EXT_CSD_POWER_OFF_NOTIFICATION,
            notify_type, timeout, 0, false, false, 0);
#endif
    if (err)
        EM_ERR("%s: Power Off Notification timed out (RTK ver.), %u\n",
                mmc_hostname(card->host), timeout);

    /* Disable the power off notification after the switch operation. */
    card->ext_csd.power_off_notification = EXT_CSD_NO_POWER_NOTIFICATION;
#endif
    return err;
}

/* duplicated from mmc_can_poweroff_notify in linux/drivers/mmc/core/mmc.c */
int export_mmc_can_poweroff_notify(const struct mmc_card *card){
    return card &&
        mmc_card_mmc(card) &&
        (card->ext_csd.power_off_notification == EXT_CSD_POWER_ON);
}

/* duplicated from mmc_send_data_cmd in linux/drivers/mmc/core/block.c
int export_mmc_send_data_cmd(unsigned int hc_cmd,unsigned int cmd_arg,
        unsigned int blk_cnt,unsigned char * buffer );
        */

/* RTK ioctl */
static
int mmc_rtk_ioctl_open(
        struct mmc_blk_data *md,
        struct mmc_blk_ioc_data *idata )
{
    return 1;
}

static
int mmc_rtk_ioctl_close(
        struct mmc_blk_data *md,
        struct mmc_blk_ioc_data *idata )
{
    return 1;
}

static
int mmc_rtk_ioctl_power_off_notification(
        struct mmc_blk_data *md,
        struct mmc_blk_ioc_data *idata )
{
    struct mmc_card *card;
    u32 *buf;
    u8 *emmc_buf = NULL;
    int err;

    err = -EPERM;
    card = md->queue.card;
    buf  = (unsigned int *)idata->buf;
    EM_INFO("%s(%d)PON type is %s\n",
            __func__,__LINE__,
            (buf[0]==EXT_CSD_POWER_OFF_SHORT)?
            "EXT_CSD_POWER_OFF_SHORT":
            ((buf[0]==EXT_CSD_POWER_OFF_LONG)?
             "EXT_CSD_POWER_OFF_LONG":"Unknow PON Type"));

    if(card &&
            (buf[0] <= EXT_CSD_SLEEP_NOTIFICATION) &&
            (card->ext_csd.rev >= 6) )
    {
        if (export_mmc_can_poweroff_notify(card)){
            /* waitting to send poweroff_notify command */
            EM_INFO( "Card poweroff-notify-state is EXT_CSD_POWER_ON already.\n");
        }else{
            EM_INFO( "transfer Card poweroff-notify-state to EXT_CSD_POWER_ON.\n");
            emmc_buf = kmalloc(512, GFP_KERNEL);
            if(!emmc_buf){
                err = -ENOMEM;
                goto ERR_OUT;
            }else{
                memset(emmc_buf, 0, 512);
                //err = mmc_send_ext_csd(card, emmc_buf);
                err = mmc_get_ext_csd(card, &emmc_buf);

                if(err){
                    err = -EIO;
                    goto ERR_OUT;
                }
#if 0
                for(i=0; i<512; i++){
                    EM_ERR( "    [%03u]=%02x\n",i,*(emmc_buf+i));
                }
#endif
                if(emmc_buf[34] == EXT_CSD_POWER_ON){
                    card->ext_csd.power_off_notification = EXT_CSD_POWER_ON;

                }else{
                    err = mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
                            EXT_CSD_POWER_OFF_NOTIFICATION,
                            EXT_CSD_POWER_ON,
                            card->ext_csd.generic_cmd6_time);
                    if (err && err != -EBADMSG){
                        goto ERR_OUT;
                    }

                    /*
                     * The err can be -EBADMSG or 0,
                     * so check for success and update the flag
                     */
                    if (!err){
                        card->ext_csd.power_off_notification = EXT_CSD_POWER_ON;
                    }
                }
            }
        }
        EM_INFO( "perform poweroff-notify.\n");
        err = export_mmc_poweroff_notify(card, buf[0]);
        if(err){
            EM_ERR( "perform poweroff-notify fail.\n");
            err = -EIO;
            goto ERR_OUT;
        }
    }
ERR_OUT:
    if(emmc_buf)
        kfree(emmc_buf);

    return err;
}

#if 0
static
int mmc_rtk_ioctl_CXD(
        struct mmc_blk_data *md,
        struct mmc_blk_ioc_data *idata,
        int reg_type )
{
    struct mmc_card *card;
    int err = -EBADMSG;

    card = md->queue.card;

    if(card){
        u8 *pcid;

        if(reg_type == IOCTL_GET_CID)
            pcid = (u8 *)card->raw_cid;
        else if(reg_type == IOCTL_GET_CSD)
            pcid = (u8 *)card->raw_csd;
        else{
            WARN_ON(1);
            err = -EFAULT;
            goto ERR_OUT;
        }

        EM_INFO("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n",
                pcid[3] ,pcid[2] ,pcid[1] ,pcid[0],
                pcid[7] ,pcid[6] ,pcid[5] ,pcid[4],
                pcid[11],pcid[10],pcid[9] ,pcid[8],
                pcid[15],pcid[14],pcid[13],pcid[12]);

        //memcpy(idata->buf,card->raw_cid,16);
        memcpy(idata->buf,pcid,16);

        /*
           long_byte_swap((unsigned int*)idata->buf,4);
         */
        if (copy_to_user((void __user *)(unsigned long) idata->ic.data_ptr,
                    idata->buf, 16))
        {
            err = -EFAULT;
        }else{
            err = 0;
        }
    }
ERR_OUT:
    return err;
}
#endif


int mmc_rtk_ioctl_cmd(
        struct mmc_blk_data *md,
        struct mmc_blk_ioc_data *idata )
{
    struct mmc_card *card;

    u32 rtk_flags;
    u32 cmd_flags;
    u32 *buf;
    int err = 0;

    card = md->queue.card;

    rtk_flags = GET_RTK_IOCTL_FLAG(idata->ic.flags);    //((idata->ic.flags)& 0xffff0000)>>RTK_FLAG_SHT;
    cmd_flags = GET_CMD_IOCTL_FLAG(idata->ic.flags);    //idata->ic.flags & (0x0000FFFF);

    buf = (unsigned int *)idata->buf;

    switch(rtk_flags){
        case RTK_IOCTL_OPEN:
            EM_INFO("RTK_IOCTL_OPEN\n");
            mmc_rtk_ioctl_open(md,idata);
            break;
        case RTK_IOCTL_CLOSE:
            EM_INFO("RTK_IOCTL_CLOSE\n");
            mmc_rtk_ioctl_close(md,idata);
            break;
        case RTK_IOCTL_ERASE:
            EM_INFO("RTK_IOCTL_ERASE\n");
#ifdef CONFIG_MMC_RTKEMMC_PLUS_MODULE
            EM_INFO("No support for GKI now!!!\n");
#else
            err = mmc_erase(card, buf[0], buf[1], MMC_TRIM_ARG);
#endif
            break;
        case RTK_IOCTL_PON_SHORT:
            EM_INFO("RTK_IOCTL_PON_SHORT\n");
            buf[0] = EXT_CSD_POWER_OFF_SHORT;
            mmc_rtk_ioctl_power_off_notification(md,idata);
            break;
        case RTK_IOCTL_PON_LONG:
            EM_INFO("RTK_IOCTL_PON_LONG\n");
            buf[0] = EXT_CSD_POWER_OFF_LONG;
            mmc_rtk_ioctl_power_off_notification(md,idata);
            break;
        case RTK_IOCTL_SMART_REPORT:
            EM_INFO("RTK_IOCTL_SMART_REPORT have removed to kadp layer!\n");
            break;
        case RTK_IOCTL_GET_CID:
            EM_INFO("RTK_IOCTL_GET_CID have removed to kadp layer!\n");
            break;
        case RTK_IOCTL_GET_CSD:
            EM_INFO("RTK_IOCTL_GET_CSD have removed to kadp layer\n");
            break;
        default:
            EM_INFO("vendor cmd%d no support!\n",rtk_flags);
            break;
    }

    return err;
}

/* RTK tuning process */
static bool HS50_tune = false;
static bool HS400_tune_fail = false;

bool mmc_tuning_50_done(void)
{
    return HS50_tune;
}

bool mmc_tuning_400_done(void)
{
    return !HS400_tune_fail;
}

void mmc_execute_tuning_50(struct mmc_host *host)
{
    u32 opcode;
    int err;

    /* If HS400 tuning success and HS50 has tuned. do nothing */
    if (!HS400_tune_fail && HS50_tune){
        EM_ERR("%s: rphase50 has been tuned\n", mmc_hostname(host));
        return;
    }

    if (!host->ops->execute_tuning)
        return;

    opcode = MMC_RPHASE50_TUNING;

    err = host->ops->execute_tuning(host, opcode);

    if (err)
        EM_ERR("%s: tuning execution rphase50 failed\n", mmc_hostname(host));
    else {
        EM_ERR("%s: rphase50 tuning susccess\n", mmc_hostname(host));
        HS50_tune = true;
    }

    return;
}

void mmc_execute_tuning_400(struct mmc_host *host)
{
    u32 opcode;
    int err;

    if (!host->ops->execute_tuning)
        return;

    opcode = MMC_READ_MULTIPLE_BLOCK;

    err = host->ops->execute_tuning(host, opcode);

    if (err){
        EM_ERR("%s: tuning execution 400 failed\n", mmc_hostname(host));
        /* If we can't get result of HS400 tuning at second time, We will downspeed */
        if (HS400_tune_fail){
            if(host->caps2 & MMC_CAP2_HS400_1_8V){
                EM_ALERT("%s: Disable HS400 mode\n",mmc_hostname(host));
                host->caps2 &= ~MMC_CAP2_HS400_1_8V;
                host->card->mmc_avail_type &=
                    ~(EXT_CSD_CARD_TYPE_HS400ES|EXT_CSD_CARD_TYPE_HS400);
            }
        }
        HS400_tune_fail = true;
    }
    else {
        EM_ERR("%s: ds tuning susccess\n", mmc_hostname(host));
        HS400_tune_fail = false;
    }
    return;
}

/* duplicated from drivers/mmc/core/mmc_ops.c */
static const u8 tuning_blk_pattern_4bit_clone[] = {
    0xff, 0x0f, 0xff, 0x00, 0xff, 0xcc, 0xc3, 0xcc,
    0xc3, 0x3c, 0xcc, 0xff, 0xfe, 0xff, 0xfe, 0xef,
    0xff, 0xdf, 0xff, 0xdd, 0xff, 0xfb, 0xff, 0xfb,
    0xbf, 0xff, 0x7f, 0xff, 0x77, 0xf7, 0xbd, 0xef,
    0xff, 0xf0, 0xff, 0xf0, 0x0f, 0xfc, 0xcc, 0x3c,
    0xcc, 0x33, 0xcc, 0xcf, 0xff, 0xef, 0xff, 0xee,
    0xff, 0xfd, 0xff, 0xfd, 0xdf, 0xff, 0xbf, 0xff,
    0xbb, 0xff, 0xf7, 0xff, 0xf7, 0x7f, 0x7b, 0xde,
};

static const u8 tuning_blk_pattern_8bit_clone[] = {
    0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00,
    0xff, 0xff, 0xcc, 0xcc, 0xcc, 0x33, 0xcc, 0xcc,
    0xcc, 0x33, 0x33, 0xcc, 0xcc, 0xcc, 0xff, 0xff,
    0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xee, 0xff,
    0xff, 0xff, 0xdd, 0xff, 0xff, 0xff, 0xdd, 0xdd,
    0xff, 0xff, 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb,
    0xbb, 0xff, 0xff, 0xff, 0x77, 0xff, 0xff, 0xff,
    0x77, 0x77, 0xff, 0x77, 0xbb, 0xdd, 0xee, 0xff,
    0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00,
    0x00, 0xff, 0xff, 0xcc, 0xcc, 0xcc, 0x33, 0xcc,
    0xcc, 0xcc, 0x33, 0x33, 0xcc, 0xcc, 0xcc, 0xff,
    0xff, 0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xee,
    0xff, 0xff, 0xff, 0xdd, 0xff, 0xff, 0xff, 0xdd,
    0xdd, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff, 0xff,
    0xbb, 0xbb, 0xff, 0xff, 0xff, 0x77, 0xff, 0xff,
    0xff, 0x77, 0x77, 0xff, 0x77, 0xbb, 0xdd, 0xee,
};

/* some modification for RTK use */
int rtkmmc_send_tuning(struct mmc_host *host, u32 opcode, int *cmd_error)
{
    struct mmc_request mrq = {};
    struct mmc_command cmd = {};
    struct mmc_data data = {};
    struct scatterlist sg;
    struct mmc_ios *ios = &host->ios;
    const u8 *tuning_block_pattern;
    int size, err = 0;
    u8 *data_buf;
    /* RTK add */
    bool pattern_cmp = true;

    if (ios->bus_width == MMC_BUS_WIDTH_8) {
        tuning_block_pattern = tuning_blk_pattern_8bit_clone;
        size = sizeof(tuning_blk_pattern_8bit_clone);
    } else if (ios->bus_width == MMC_BUS_WIDTH_4) {
        tuning_block_pattern = tuning_blk_pattern_4bit_clone;
        size = sizeof(tuning_blk_pattern_4bit_clone);
    } else
        return -EINVAL;

    /* RTK add */
    if(opcode != MMC_SEND_TUNING_BLOCK_HS200){
        if( (opcode == MMC_WRITE_MULTIPLE_BLOCK)
                || (opcode == MMC_READ_MULTIPLE_BLOCK) ){
            opcode -= 1;
        }
        pattern_cmp = false;
        size = 512;
    }

    data_buf = kzalloc(size, GFP_KERNEL);
    if (!data_buf)
        return -ENOMEM;

    mrq.cmd = &cmd;
    mrq.data = &data;

    cmd.opcode = opcode;
    if (opcode == MMC_WRITE_BLOCK)
        cmd.arg = 0x100;
    if (opcode == MMC_SEND_STATUS){
        /* RCA check*/
        cmd.arg = (0x1 << 16);
        mrq.data = NULL;
        cmd.flags = MMC_RSP_R1 | MMC_CMD_AC;
    }
    else {
        cmd.flags = MMC_RSP_R1 | MMC_CMD_ADTC;
        data.blksz = size;
        data.blocks = 1;
        /* data.flags = MMC_DATA_READ , but we have to do hs50,hs400 tuning */
        data.flags = (opcode == MMC_WRITE_BLOCK)? MMC_DATA_WRITE:MMC_DATA_READ;

        /*
         * According to the tuning specs, Tuning process
         * is normally shorter 40 executions of CMD19,
         * and timeout value should be shorter than 150 ms
         */
        data.timeout_ns = 150 * NSEC_PER_MSEC;

        data.sg = &sg;
        data.sg_len = 1;
        sg_init_one(&sg, data_buf, size);
    }

    mmc_wait_for_req(host, &mrq);

    if (cmd_error)
        *cmd_error = cmd.error;

    if (cmd.error) {
        err = cmd.error;
        goto out;
    }

    if (data.error) {
        err = data.error;
        goto out;
    }

    /* RTK modify */
    if(pattern_cmp)
    {
        if (memcmp(data_buf, tuning_block_pattern, size))
            err = -EIO;
    }else{
        /* not cmd21, skip partten compare  */
        //EM_ALERT("%s(%d)non cmd21, skip partten compare\n",__func__,__LINE__);
    }

out:
    kfree(data_buf);
    return err;
}

