/*
 * Realtek MMC/SD/SDIO driver
 *
 * Authors:
 * Copyright (C) 2008-2009 Realtek Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/mbus.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include  <linux/dmapool.h>
#include <linux/scatterlist.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/mmc/host.h>
#include <asm/unaligned.h>

#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/mmc/card.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sd.h>
#include <linux/workqueue.h>
#include <linux/completion.h>
#include <linux/time.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include "mmc_reg.h"
#include "mmc_p_reg.h"
#include <rbus/misc_reg.h>
//#include <rbus/pinmux_reg.h>
#include "rtksdio.h"
#include "rtksd_ops.h"
#include <mach/platform.h>
#include <mach/pcbMgr.h>
#include <rbus/sb2_reg.h>
#include <rtk_kdriver/rtk_otp_region_api.h>
#include <rtk_kdriver/rtk-kdrv-common.h>

#include "rtk_mmc.h"

/************************************************************************
 *  Local Define
 ************************************************************************/
#define DRIVER_NAME "rtkemmc_dw"
#define BANNER      "Realtek eMMC Driver"
#define VERSION "$Id: rtkemmc.c RTK_2885P 2023-04-12 13:55 $"

#define EXT_CSD_ENH_START_ADDR          136     /* R/W, 4 bytes */
#define EXT_CSD_ENH_SIZE_MULT           140     /* R/W, 3 bytes */
#define EXT_CSD_WR_REL_SET              167     /* R/W ifHS_CTRL_REL=1 */


#ifndef CONFIG_MMC_RTKEMMC_HK_ATTR
#define CONFIG_MMC_RTKEMMC_HK_ATTR
#endif

#ifdef CONFIG_MMC_RTKEMMC_SHOUTDOWN_PROTECT
#if IS_ENABLED(CONFIG_REALTEK_PCBMGR) && IS_ENABLED(CONFIG_REALTEK_GPIO)
#define EMMC_SHOUTDOWN_PROTECT
#else
#define EMMC_SHOUTDOWN_PROTECT_FAIL
#endif
#endif

#ifdef CONFIG_MMC_RTKEMMC_DDR
#define EN_EMMC_DDR50
#endif

#if defined(CONFIG_RTKEMMC_HS200) || defined(CONFIG_RTKEMMC_HS200_MODULE)
#define EN_EMMC_HS200
#endif

#if defined(CONFIG_RTKEMMC_HS400) || defined(CONFIG_RTKEMMC_HS400_MODULE)
#define EN_EMMC_HS400
#endif

/* #define EN_SHOW_DATA */
#define cr_maskl(reg,msk,sht,val)  \
        cr_writel((cr_readl(reg) & ~(msk<<sht))|(val<<sht), reg)

#define USE_FIXED_W_PHASE

/* rtk_emmc_bus_wid control flag*/
#define EMMC_DIS_50M_TUNE   BIT(14)

#define EMMC_EN_SCAN_TOOL   BIT(15)
#define EMMC_EN_PARAM       BIT(16)
#define EMMC_DIS_SSCG       BIT(17)
#define EMMC_EN_IO_DRV      BIT(18)
#define EMMC_EN_MMC_DRV     BIT(19)

#define EMMC_FIX_DSTUNE     BIT(20)
#define EMMC_SKIP_TUNE      BIT(21)
#define EMMC_ALWAYS_CLK     BIT(22)
#define EMMC_BURST_BLK      BIT(23)

#define EMMC_RW_REG         BIT(24)
#define EMMC_RESTORE        BIT(25)
#define EMMC_RCV_FLOW       BIT(26)
#define EMMC_SHOW_CRC       BIT(27)

#define EMMC_TUNE_WPH       BIT(28)
#define EMMC_FIX_WPHASE     BIT(29)
#define EMMC_CHK_BUF        BIT(30)
#define EMMC_FIX_STRENGTH   BIT(31)

#define IDMAC_INT_CLR       (SDMMC_IDMAC_INT_AI | SDMMC_IDMAC_INT_NI | \
                 SDMMC_IDMAC_INT_CES | SDMMC_IDMAC_INT_DU | \
                 SDMMC_IDMAC_INT_FBE | SDMMC_IDMAC_INT_RI | \
                 SDMMC_IDMAC_INT_TI)

#define USE_INT_MODE (SDMMC_CMD_DAT_EXP|SDMMC_CMD_START)

#define FIX_WPHASE50           BIT(0)
#define FIX_RPHASE50           BIT(1)
#define FIX_WPHASE200          BIT(2)
#define FIX_WPHASE400          BIT(3)
#define FIX_CPHASE200          BIT(4)
#define FIX_RPHASE400          BIT(5)
#define FIX_RPHASE200          BIT(6)

/* Low speed phase (<50M) */
#define MMC_LOW_SPEED_PHASE    0x64

//#define DEBUG_BUFFER

/*
 Beware!!! Only phase below are available (from DIC's suggestion)
 WPhase : 4~13 / 20~31
 RPhase : 3~12 / 20~28
 CPhase : 4~31
 DS     : 0~28
 */
#define VAILD_DEFAULT_WPHASE_ZONE   (0x7ffefffe)    //31:0 = [0111 1111 1111 1110 1111 1111 1111 1110]

#define VAILD_WPHASE_ZONE           (0xfff03ff0)    //31:0 = [1111 1111 1111 0000 0011 1111 1111 0000]
#define VAILD_RPHASE_ZONE           (0x0ff80ff8)    //31:0 = [0000 1111 1111 1000 0000 1111 1111 1000]
#define VAILD_CPHASE_ZONE           (0xfffffff0)    //31:0 = [1111 1111 1111 1111 1111 1111 1111 0000]
#define VAILD_DSTUNE_ZONE           (0x0fffffff)    //31:0 = [0000 1111 1111 1111 1111 1111 1111 1111]
#define VAILD_WPHASE_BOUNDARY_LOW   (13)
#define VAILD_WPHASE_BOUNDARY_HIGH  (20)
#define VAILD_RPHASE_BOUNDARY_LOW   (12)
#define VAILD_RPHASE_BOUNDARY_HIGH  (20)
/* Always find middle value*/
#define VAILD_CPHASE_BOUNDARY       (32)

#define OPTIMAL_R_PHASE     (8)
#define OPTIMAL_C_PHASE     (18)
#define OPTIMAL_W_PHASE     (8)

/* default phase */
#define DEFAULT_R_PHASE     (6)
#define DEFAULT_C_PHASE     (18)
#define DEFAULT_W_PHASE     (18)

#define DEFAULT_C_PHASE_200 (18)
#define DEFAULT_W_PHASE_200 (13)
#define DEFAULT_W_PHASE_400 (9)
#define DEFAULT_DSTUNE      (9)

static u32 rtk_fix_rphase = 0;

#define SET_FIX_WPHA400(x,y)    (y = ((y & ~0x3f) | (0xc0|x)))
#define SET_FIX_WPHA200(x,y)    (y = ((y & ~(0x3f<< 8)) | ((0xc0|x)<< 8)))
#define SET_FIX_RPHA400(x)      (rtk_fix_rphase = ((rtk_fix_rphase & ~0x3f) | (0xc0|x)))
#define SET_FIX_RPHA200(x)      (rtk_fix_rphase = ((rtk_fix_rphase & ~0x3f00) | ((0xc0|x)<<8)))
#define SET_FIX_CPHA200(x,y)    (y = ((y & ~(0x3f<<16)) | ((0xc0|x)<<16)))
#define SET_FIX_STRENGTH(x,y)   (y = ((y & ~(0xf <<24)) | (x<<24)))         //24~27

#define GET_FIX_WPHA400(y)      ((y & 0xc0) == 0xc0)?(y & 0x3f):0xff
#define GET_FIX_WPHA200(y)      (((y >> 8) & 0xc0) == 0xc0)?((y >>8) & 0x3f):0xff
#define GET_FIX_RPHA400()       ((rtk_fix_rphase & 0xc0) == 0xc0)?(rtk_fix_rphase & 0x3f):0xff
#define GET_FIX_RPHA200()       ((rtk_fix_rphase & 0xc000) == 0xc000) ? ((rtk_fix_rphase & 0x3f00)>>8):0xff

#define GET_FIX_CPHA200(y)      (((y >> 16) & 0xc0) == 0xc0)?((y >>16) & 0x3f):0xff
#define GET_FIX_STRENGTH(y)     ((y >>24) & 0xf)

#define SET_FIX_RPHA50(x,y)    (y = ((y & ~0x3f) | (0xc0|x)))
#define SET_FIX_WPHA50(x,y)    (y = ((y & ~(0x3f<< 8)) | ((0xc0|x)<< 8)))
#define GET_FIX_RPHA50(y)      ((y & 0xc0) == 0xc0)?(y & 0x3f):0xff
#define GET_FIX_WPHA50(y)      (((y >> 8) & 0xc0) == 0xc0)?((y >>8) & 0x3f):0xff

#define PAD_SET_CNT (7)

/* CMD23 by SW flow or Auto stop by IP */
#define RTK_CMD23_USE
/* We check product name to determinate which read flow be used */
#define AUTO_SEND_STOP
/* Directly ISR routine */
#define RTK_EMMC_ISR_DIRECT

/* Each descriptor can transfer up to 4KB of data in chained mode */
#define DW_MCI_DESC_DATA_LENGTH     0x1000

/* Downspeed mechanism, must define which platform to enable */
#if defined (CONFIG_ARCH_RTK6702)
#define RTK_RW_DOWNSPEED
#endif

typedef u8 Phase_Arry[MAX_TUNING_STEP];

/************************************************************************
 *  Local veriable
 ************************************************************************/
static int maxfreq = 0;
static int nodma;
struct mmc_host *mmc_host_local;
#ifdef CONFIG_MMC_RTKEMMC_PLUS_MODULE
EXPORT_SYMBOL(mmc_host_local);
#endif
static u32 rtk_emmc_test_ctl =0;
static u32 rtk_emmc_log_state = 0;
static u32 rtk_fix_wphase = 0;
static unsigned long rtk_emmc_err_cnt = 0;
u32 rtk_emmc_raw_op;
#ifdef CONFIG_MMC_RTKEMMC_PLUS_MODULE
EXPORT_SYMBOL(rtk_emmc_raw_op);
#endif
static mmc_phase_t tuning_phase;
static u32 rtk_fix_50M_phase = 0;
static bool device_suspend = 0;
static bool stop_timer = 0;
static bool done_timer = 0;
static u32 fix_phase_flag = 0;
#ifdef RTK_CMD23_USE
/* We check product name to determinate which read flow be used */
static bool predefined_read = 0;
#endif
static bool is_rpmb = false;
static bool downspeed_test = false;
static u8 HS200_fail_count = 0;

// debug
static __attribute__((unused))bool HS400_fix_test = false;
static u8 rtk_emmc_debug_log = 0;
static u8 pre_cmd = 0;
static u16 int_collect = 0;
#ifdef RTK_RW_DOWNSPEED
static u8 pre_cmd_write_count = 0;
static u8 pre_cmd_read_count = 0;
static u32 pre_wcmd_arg = 0;
static u32 pre_rcmd_arg = 0;
#endif

/* for caculation about cmd finish time */
atomic64_t cmd_time_ns_old;
u32 cmd_record = 0;
u32 total_byte_record = 0;

#ifdef CONFIG_LGEMMC_PARTITION
#define MMC_ROOTFS_CHECK
#endif

#ifdef MMC_ROOTFS_CHECK
static u32 partition_info_ready = 0;
static int rootfs_part_num = -1;

struct mmc_partinfo
{
	unsigned long long	offset;					/* offset within the master MTD space              */
	unsigned long long	size;					/* partition size                                  */
	unsigned long long	ptend;					/* end position of partition                       */
};
static struct mmc_partinfo rootfs_partinfo;

extern int lgemmc_get_partnum(const char *name);
extern char *lgemmc_get_partname(int partnum);
extern unsigned int lgemmc_get_partsize(int partnum);
extern unsigned int lgemmc_get_filesize(int partnum);
extern unsigned int lgemmc_get_sw_version(int partnum);
extern unsigned long long lgemmc_get_partoffset(int partnum);
extern char *rtkcr_parse_token(const char *parsed_string, const char *token);


#endif

static const char *const dw_clk_tlb[23] = {
    "200Khz",
    "10Mhz",
    "15Mhz",
    "20Mhz",
    "25Mhz",
    "30Mhz",
    "40Mhz",
    "50Mhz",
    "60Mhz",
    "70Mhz",
    "80Mhz",
    "90Mhz",
    "100Mhz",
    "110Mhz",
    "120Mhz",
    "130Mhz",
    "140Mhz",
    "150Mhz",
    "160Mhz",
    "170Mhz",
    "180Mhz",
    "190Mhz",
    "200Mhz"
};

#ifdef CONFIG_RTKEMMC_COWORK_WITH_KCPU
// for pm
#define KCPU_EMMC_DEVICE_SUSPEND_DONE   0xED00ED00
extern unsigned int mmc_pm_sync_addr;
#endif
/************************************************************************
 *  Local Function Protal Type
 ************************************************************************/
static void rtkcr_display_version(void);
static void dw_em_request(struct mmc_host *host, struct mmc_request *mrq);
static int dw_em_get_ro(struct mmc_host *mmc);
static void dw_em_set_ios(struct mmc_host *host, struct mmc_ios *ios);
static int dw_em_execute_tuning(struct mmc_host *mmc, u32 opcode);
static int mmc_send_data_cmd(struct mmc_host * mmc, u32 hc_cmd,
                 u32 cmd_arg, u32 blk_cnt, unsigned char *buffer);

static void set_cmd_info(struct rtksd_host *sdport,struct mmc_command * cmd,
             struct sd_cmd_pkt *cmd_info, u32 opcode, u32 arg,
             u32 rsp_para);

static int dw_em_stop_transmission(struct rtksd_host *sdport);
static u32 dw_em_get_cmd_timeout(struct sd_cmd_pkt *cmd_info);

static void dw_em_set_scrip_base(dma_addr_t scrip_base);
static void show_ext_csd(u8 *ext_csd);
static void dw_em_crt_reset(struct rtksd_host *sdport,u32 sync);
static void dw_em_hw_reset(struct mmc_host *host);
static void dw_em_reset_host_IP(struct rtksd_host *sdport,u8 save_en);
static void dw_em_reset_host(struct rtksd_host *sdport,u8 save_en);
static int dw_em_reset_IP(void);
static void dw_em_set_div(struct rtksd_host *sdport,u32 em_div);
static void dw_em_IP_config(struct rtksd_host *sdport);
static int rtkem_clr_sta(struct rtksd_host *sdport);
static int dw_em_dis_sd_tune(struct rtksd_host *sdport);
static int dw_em_auto_sd_tune(struct rtksd_host *sdport);
int dw_em_prepare_hs400_tuning(struct mmc_host *host, struct mmc_ios *ios);
int dw_em_drive_strength (struct mmc_card *card,
        unsigned int max_dtr, int host_drv,
        int card_drv, int *drv_type);
static void dw_em_set_ds_delay(struct rtksd_host *sdport, int ds_delay);
static int dw_em_ds_tuning(struct mmc_host *host, u32 ctl);
static int dw_em_save_pad(struct rtksd_host *sdport,u32* pad_arry,int cnt);
static int dw_em_restore_pad(struct rtksd_host *sdport,u32* pad_arry);
static int rtkemmc_disable_int(struct rtksd_host * sdport);
static int rtkemmc_enable_int(struct rtksd_host * sdport);
#ifdef RTK_CMD23_USE
static void rtk_init_card(struct mmc_host *host, struct mmc_card *card);
#endif
#ifdef RTK_EMMC_ISR_DIRECT
static void rtkcr_req_end_tasklet(struct rtksd_host *sdport);
static void rtkem_irq_task(struct rtksd_host *sdport);
#endif

#ifdef  EN_SHOW_DATA
static void show_data(u8 *buf, unsigned int len);
#endif

static
const struct mmc_host_ops rtkemmc_ops = {
    .request        = dw_em_request,
    .get_ro         = dw_em_get_ro,
    .set_ios        = dw_em_set_ios,
#ifdef RTK_CMD23_USE
    .init_card      = rtk_init_card,
#endif
    .execute_tuning     = dw_em_execute_tuning,
    .hw_reset       = dw_em_hw_reset,
    .prepare_hs400_tuning   = dw_em_prepare_hs400_tuning,
    .hs400_downgrade  = mmc_execute_tuning_50,
    .hs400_complete   = mmc_execute_tuning_400,
    .select_drive_strength  = dw_em_drive_strength,
};

/************************************************************************
 *  external Function Protal Type
 ************************************************************************/
#ifndef CONFIG_MMC_RTKEMMC_PLUS_MODULE
extern s32 export_raw_dw_em_read(u8 *buffer, u64 address, u32 size);
extern s32 export_raw_dw_em_write(u8 *buffer, u64 address, u32 size);
extern s32 export_raw_dw_em_init(void);
extern int export_raw_emmc_partition_change(u32 part_num);
#endif
/************************************************************************
 * Function Body
 ************************************************************************/

#ifdef RTK_CMD23_USE
/*
 * Need to confirm heximal or decimal val
 * 0xFF means dont care value
 */
#define RTK_CMD23_TABLE_LEN     2
static unsigned char product_use_cmd23[RTK_CMD23_TABLE_LEN][6] = {
    /* hynix H26M41204HPR : 0x483847346192 (hex) */
    {0x48,0x38,0x47,0x34,0x61,0x92},
    /* SANDISK SDINBDG4-XX : 0x44473430FFFF (hex) */
    {0x44,0x47,0x34,0x30,0xFF,0xFF},
};

static
void rtk_init_card(struct mmc_host *host, struct mmc_card *card)
{
    int i, j, match;
    unsigned char product_name[7] = {0};

    /* Determinate CMD18/25+CMD12 or CMD23+CMD18/25 */
    product_name[0] = (unsigned char)(card->raw_cid[0] & 0xFF);
    product_name[1] = (unsigned char)(card->raw_cid[1] >> 24 & 0xFF);
    product_name[2] = (unsigned char)(card->raw_cid[1] >> 16 & 0xFF);
    product_name[3] = (unsigned char)(card->raw_cid[1] >> 8 & 0xFF);
    product_name[4] = (unsigned char)(card->raw_cid[1] & 0xFF);
    product_name[5] = (unsigned char)(card->raw_cid[2] >> 24 & 0xFF);
    EM_INFO("Product name: \"%s\" (%x%x%x%x%x%x)\n",
            product_name,
            product_name[0], product_name[1], product_name[2],
            product_name[3], product_name[4], product_name[5]);
    i = match = 0;
    for (i = 0;i < RTK_CMD23_TABLE_LEN;i++)
    {
        for (j = 0; j < 6; j++){
            if (0xFF == product_use_cmd23[i][j])
                continue;
            if (product_name[j] != product_use_cmd23[i][j])
                break;
        }
        if (j == 6){
            match = 1;
            break;
        }
    }
    predefined_read = match;
    EM_INFO("Pre-defined Read Enabled : %u\n", predefined_read);
}
#endif

bool get_mmc_tuning_50_done(struct mmc_host *host)
{
	struct rtksd_host *sdport = mmc_priv(host);
	return sdport->hs50_tuned;
}
void set_mmc_tuning_50_done(struct mmc_host *host,bool val)
{
	struct rtksd_host *sdport = mmc_priv(host);
	sdport->hs50_tuned = val;
}

bool get_mmc_tuning_200_done(struct mmc_host *host)
{
	struct rtksd_host *sdport = mmc_priv(host);
	return sdport->hs200_tuned;
}
void set_mmc_tuning_200_done(struct mmc_host *host,bool val)
{
	struct rtksd_host *sdport = mmc_priv(host);
	sdport->hs200_tuned = val;
}
bool get_mmc_tuning_400_done(struct mmc_host *host)
{
	struct rtksd_host *sdport = mmc_priv(host);
	return sdport->hs400_tuned;
}
void set_mmc_tuning_400_done(struct mmc_host *host,bool val)
{
	struct rtksd_host *sdport = mmc_priv(host);
	sdport->hs400_tuned = val;
}

static int dw_em_idmac_init(struct rtksd_host *sdport)
{
    struct idmac_desc *p = NULL;
    int i;
    u32 ring_size;

    ring_size = (EMMC_SCRIPT_BUF_SIZE) / sizeof(struct idmac_desc);
    /* Forward link the descriptor list */
    for (i = 0, p = sdport->scrip_buf;
            i < ring_size - 1;
            i++, p++) {
        p->des3 = cpu_to_le32(sdport->scrip_buf_phy +
                (sizeof(struct idmac_desc) * (i + 1)));
        p->des1 = 0;
    }

    /* Set the last descriptor as the end-of-ring descriptor */
    //p->des3 = cpu_to_le32(sdport->scrip_buf_phy);
    //p->des0 = cpu_to_le32(IDMAC_DES0_ER);

    return 0;
}

/************************************************************************
 *
 ************************************************************************/
static
int dw_em_get_occupy(struct rtksd_host *sdport)
{
    unsigned int relock = 0;
    unsigned int loop = 0;
    unsigned long flags;

#if defined(CONFIG_MMC_HW_SEMAPHORE) || defined(CONFIG_MMC_HW_SEMAPHORE_MODULE)
    /* get HW semaphore */
    while(!(cr_readl(EMMC_HW_SEMAPHORE))
     && loop < 1000)
    {
        loop++;
        EM_WARNING("wait EMMC_HW_SEMAPHORE\n");
        msleep(1);
    }
    if(loop >= 1000)
        return -1;
    loop = 0;
#endif

RE_LOCK:
    spin_lock_irqsave(&sdport->lock,flags);

    if(cr_readl(EM_SCPU_SEL) & PS_SCPU_SEL){
        relock = 1;
    }else{
        cr_writel( PS_SCPU_SEL, EM_SCPU_SEL );
    }

    spin_unlock_irqrestore(&sdport->lock, flags);
    if(relock){
        loop ++;
        relock = 0;
        msleep(1);
        if(loop > 5){
            u32 reg1,reg2;
#if defined(RTK_EMMC_33V_SEL)
            u32 volinfo;
            volinfo = 0;    //cr_readl(RTK_EMMC_33V_SEL);
#endif
            reg1 = cr_readl(RTK_EM_DW_SRST);
            reg2 = cr_readl(RTK_EM_DW_CLKEN);
            if(!(reg1 & BIT(23)) || !(reg2 & BIT(23))){
                EM_WARNING("%s(%d)RST/CLK disable[%d] DW_SRST=0x%08x DW_CLKEN=0x%08x  <<<<<<<<<\n",
                __func__,__LINE__,loop,
                cr_readl(RTK_EM_DW_SRST),
                cr_readl(RTK_EM_DW_CLKEN));

                cr_writel((RSTN_EM_DW | 0x01UL),RTK_EM_DW_SRST);
                cr_writel((CLKEN_EM_DW | 0x01U),RTK_EM_DW_CLKEN);
            }
#if defined(RTK_EMMC_33V_SEL)
            cr_writel(volinfo,RTK_EMMC_33V_SEL);
#endif
        }
        if(loop > 100){
            /* check CRT reg status here */
            EM_WARNING("%s(%d)waitting[%d] <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n",
                __func__,__LINE__,loop);
            if(loop > 1000){
#if defined(CONFIG_MMC_HW_SEMAPHORE) || defined(CONFIG_MMC_HW_SEMAPHORE_MODULE)
                /* release HW semahpore */
                rtd_outl(EMMC_HW_SEMAPHORE,    0);
#endif
                return -1;
            }
        }
        goto RE_LOCK;
    }

    return 0;
}

/************************************************************************
 *
 ************************************************************************/
static
int dw_em_release_occupy(struct rtksd_host *sdport){

    /* release scpu */
    unsigned long flags;
    int err = 0;

    if(rtk_emmc_raw_op) {
        cr_writel(0 , EM_SCPU_SEL );    /* release SCPU */
        return 0;
    }

    spin_lock_irqsave(&sdport->lock,flags);

    if(cr_readl(EM_SCPU_SEL) & PS_SCPU_SEL){
        cr_writel(0 , EM_SCPU_SEL );    /* release SCPU */
    }else{
        err = 1;
    }
    spin_unlock_irqrestore(&sdport->lock, flags);

#if defined(CONFIG_MMC_HW_SEMAPHORE) || defined(CONFIG_MMC_HW_SEMAPHORE_MODULE)
    /* release HW semahpore */
    rtd_outl(EMMC_HW_SEMAPHORE,    0);
#endif

    return 0;
}


/************************************************************************
 *
 ************************************************************************/
static
void dw_em_show_phase(void)
{
    u32 reg1,reg2,reg3,reg4,reg5;

    reg1 = (u32)cr_readl(EMMC_WRAP_half_cycle_cal_en_reg);
    reg2 = (u32)cr_readl(EMMC_WRAP_half_cycle_cal_result_reg);
    reg3 = (u32)cr_readl(EMMC_WRAP_ds_tune_ctrl_reg);
    reg4 = (u32)cr_readl(EMMC_PLLPHASE_CTRL);
    reg5 = (u32)cr_readl(EMMC_PLL_PHASE_INTERPOLATION);

    EM_ALERT("Wphase=%d Rphase=%d Cphase=%d\n",
            (reg4&PHASE_W_MASK),((reg4>>PHASE_R_SHT)&PHASE_R_MASK),((reg5>>PHASE_C_SHT)&PHASE_C_MASK));
    if(reg1 & BIT(DS_CYCLE_CAL_EN_SHIFT)){
        EM_ALERT("half-cycle calibration enable\n"
                 "    mode=%d thr=%d init=%d result=%d\n",
                 ((reg1>>DS_CAL_MODE_SHIFT)&DS_CAL_MODE_MASK),((reg1>>DS_CAL_THR_SHIFT)&DS_CAL_THR_MASK),
                 ((reg3>>DS_VALUE_SHIFT)&DS_VALUE_MASK),(reg2&DS_CAL_RES_MASK));
    }else{
        EM_ALERT("half-cycle calibration disable\n");
    }
}

/************************************************************************
 *
 ************************************************************************/
static
void dw_em_read_rsp( u32 * rsp, u32 reg_count )
{
    /* #define SHOW_RESP */

    if ( reg_count == 16 ){
        rsp[3] = cr_readl(EM_DWC_RESP0);
        rsp[2] = cr_readl(EM_DWC_RESP1);
        rsp[1] = cr_readl(EM_DWC_RESP2);
        rsp[0] = cr_readl(EM_DWC_RESP3);
#ifdef SHOW_RESP
        EM_ERR("rsp[0]=0x%08x\n", rsp[0]);
        EM_ERR("rsp[1]=0x%08x\n", rsp[1]);
        EM_ERR("rsp[2]=0x%08x\n", rsp[2]);
        EM_ERR("rsp[3]=0x%08x\n", rsp[3]);
#endif
    } else {
        rsp[0] = cr_readl(EM_DWC_RESP0);
#ifdef SHOW_RESP
        EM_ERR("rsp[0]=0x%08x\n", rsp[0]);
#endif
        /*
        When CIU sends auto-stop command,
        Response for previous command sent by host is
        still preserved in Response 1 register.
        So, record rsp[1] always.
        */
        if(reg_count == 2 ){
            rsp[1] = cr_readl(EM_DWC_RESP1);
#ifdef SHOW_RESP
            EM_ERR("rsp[1]=0x%08x\n", rsp[1]);
#endif
        }
    }
}

/************************************************************************
 *
 ************************************************************************/
static
int dw_em_chk_rsp(u32 * rsp, u32 cmd_flag, struct sd_cmd_pkt * cmd_info)
{
    int ret_err = 0;
    u32 cmd_idx = 0;

    cmd_idx = SDMMC_CMD_INDX(cmd_flag);

    if(cmd_flag & SDMMC_CMD_RESP_LONG){
        dw_em_read_rsp(rsp, 16);
        /* only resp == 6 is possible have card status */

    }else{
        if(cmd_flag & SDMMC_CMD_SEND_STOP)
            dw_em_read_rsp(rsp, 2);
        else
            dw_em_read_rsp(rsp, 6);

        if((cmd_flag & SDMMC_CMD_RESP_CRC)){
            if( (cmd_idx != MMC_GO_IRQ_STATE)){
                /*
                 Only R1/R1b should check Card_Status.
                 It can't diff R1/R1b/R5/R6/R7.
                 but only CMD40 have definded, so check it here.
                */
                if(rsp[0] & RESP_ERR_BIT){
                    ret_err = BIT(0);
                }
                if((cmd_flag & SDMMC_CMD_SEND_STOP)
                    && (rsp[1] & RESP_ERR_BIT)) {
                    ret_err |= BIT(1);
                }
            }
        }
    }
    if(ret_err){
        /* response will be check by high-level, print information just for debug. */
        mmcinfo("rsp error !!!\n");
        mmcinfo("rsp[0]=0x%08x\n", rsp[0]);
        if(cmd_flag & SDMMC_CMD_SEND_STOP){
            mmcinfo("rsp[1]=0x%08x\n", rsp[1]);
        }
    }

    return ret_err;
}

/************************************************************************
 *   0x0 :  50ohm x1
 *   0x1 :  33ohm x1.5
 *   0x2 :  66ohm x0.75
 *   0x3 : 100ohm x0.5
 *   0x4 :  40ohm x1.2
 ************************************************************************/
int dw_em_drive_strength (struct mmc_card *card,
        unsigned int max_dtr, int host_drv,
        int card_drv, int *drv_type)
{
    struct rtksd_host *sdport = NULL;
    int drv_stg;

    sdport = mmc_priv(card->host);

    /* according to Scott's advice, use 1.2(0x4) */
    if(max_dtr > 52000000){
        if(rtk_emmc_test_ctl & EMMC_FIX_STRENGTH){
            drv_stg = GET_FIX_STRENGTH(rtk_fix_wphase);
            EM_ALERT("bootcode set device strength : %d\n",drv_stg);
        }else{
            if(sdport->mid == 0x15)
                drv_stg = 0x4;
            else if(sdport->mid == 0x90)
                drv_stg = 0x1;
            else
                drv_stg = 0x4;
        }
    }else
        drv_stg = 0;

    EM_INFO("set drive strength to %d(mid=0x%x)\n",drv_stg,sdport->mid);
    return drv_stg;
}

/************************************************************************
 *
 ************************************************************************/
int dw_em_prepare_hs400_tuning(struct mmc_host *host, struct mmc_ios *ios)
{
    struct rtksd_host *sdport;

    sdport = mmc_priv(host);
    if(!(sdport->ins_event & EVENT_SKIP_DSTUNING)){
        if(sdport->mid == 0x11){    /* TSB */
                sdport->cur_ds_tune = DEFAULT_DSTUNE;
        }else{                      /* other else */
                sdport->cur_ds_tune = 9;
        }
    }else{
        sdport->cur_ds_tune = sdport->pre_ds_tune;
    }
    dw_em_set_ds_delay(sdport, sdport->cur_ds_tune);

    return 0;
}

/************************************************************************
 *
 ************************************************************************/
static
int dw_em_chk_phase_vaild(u32 phase,u32 vaild){
    if(vaild & (0x01UL<<phase)){
        return 1;
    }
    return 0;
}

/************************************************************************
 *
 ************************************************************************/
int dw_em_check_phase_fix(struct rtksd_host *sdport, bool *w_fix,bool *r_fix,bool *c_fix)
{
    *w_fix = *r_fix = *c_fix = false;
    if(sdport->cur_timing == MMC_TIMING_MMC_HS400){
        if(fix_phase_flag & FIX_WPHASE400){
            *w_fix = true;
        }
        if(fix_phase_flag & FIX_RPHASE400){
            *r_fix = true;
        }
        if(fix_phase_flag & FIX_CPHASE200){
            *c_fix = true;
        }
    }else if(sdport->cur_timing == MMC_TIMING_MMC_HS200){
        if(fix_phase_flag & FIX_WPHASE200){
            *w_fix = true;
        }
        if(fix_phase_flag & FIX_RPHASE200){
            *r_fix = true;
        }
        if(fix_phase_flag & FIX_CPHASE200){
            *c_fix = true;
        }
    }else{
        if(fix_phase_flag & FIX_WPHASE50){
            *w_fix = true;
        }
        if(fix_phase_flag & FIX_RPHASE50){
            *r_fix = true;
        }
    }
    return 0;
}

static
int dw_em_init_phase(struct rtksd_host *sdport, u32 w_phase,u32 r_phase,u32 c_phase)
{
    /*
     eMMC PLL Phase:
          setting on romcode\src\arch\reset\drivers\crt\crt.c +239

      PLL27X_REG_EMMC_PLLPHASE_CTRL_reg[ 5: 0]==> clk vs data write phase
      PLL27X_REG_EMMC_PLLPHASE_CTRL_reg[15:10]==> clk vs data read phase
      PLL27X_REG_EMMC_PLL_PHASE_INTERPOLATION_reg[15:10]==> clk vs cmd  write phase
    */
    unsigned int valid_phase_zone = 0;
    bool w_fix,r_fix,c_fix;

    mmcmsg3("w_phase=%u r_phase=%u\n",w_phase,r_phase);
    dw_em_check_phase_fix(sdport,&w_fix,&r_fix,&c_fix);
    while(w_phase != 0xff){
        if(sdport->cur_timing == MMC_TIMING_MMC_HS400){
            valid_phase_zone = VAILD_WPHASE_ZONE;
        }else if(sdport->cur_timing == MMC_TIMING_MMC_HS200){
            valid_phase_zone = VAILD_WPHASE_ZONE;
        }else{
            valid_phase_zone = VAILD_DEFAULT_WPHASE_ZONE;
        }

        if(w_fix){
            sdport->cur_w_phase = w_phase;
        }else if(dw_em_chk_phase_vaild(w_phase,valid_phase_zone)){
            sdport->cur_w_phase = w_phase;
        }else{
            EM_ALERT("The chosen W phase(%d) is unvalid !!!!!!!!!\n",w_phase);
            WARN_ON(1);
            break;
        }
        cr_maskl(EMMC_PLLPHASE_CTRL, PHASE_W_MASK, PHASE_W_SHT ,w_phase);
        break;
    }
    while(r_phase != 0xff){
        if(r_fix){
            sdport->cur_r_phase = r_phase;
        }else if(dw_em_chk_phase_vaild(r_phase,VAILD_RPHASE_ZONE)){
            sdport->cur_r_phase = r_phase;
        }else{
            EM_ALERT("The chosen R phase(%d) is unvalid !!!!!!!!!\n",r_phase);
            WARN_ON(1);
            break;
        }
        cr_maskl(EMMC_PLLPHASE_CTRL, PHASE_R_MASK, PHASE_R_SHT ,r_phase);
        break;
    }

    while(c_phase != 0xff){
        if(c_fix){
            sdport->cur_c_phase = c_phase;
        }else if(dw_em_chk_phase_vaild(c_phase,VAILD_CPHASE_ZONE)){
            sdport->cur_c_phase = c_phase;
        }else{
            EM_ALERT("The chosen C phase(%d) is unvalid !!!!!!!!!\n",c_phase);
            WARN_ON(1);
            break;
        }
        cr_maskl(EMMC_PLL_PHASE_INTERPOLATION, PHASE_C_MASK, PHASE_C_SHT ,c_phase);
        break;
    }

    mmcmsg3("PHASE_REG(0x%08x)=0x%08x\n",
        EMMC_PLLPHASE_CTRL, cr_readl(EMMC_PLLPHASE_CTRL));

    return 0;
}

/************************************************************************
 *
 ************************************************************************/
static
int dw_em_set_PLL_SSC(struct rtksd_host *sdport,
    u32 ncode ,u32 fcode ,u32 rang)
{
    int ret;
    unsigned int loop;

    ret = -1;

    EM_INFO("%s: Going to enable SSC!!!\n",DRIVER_NAME);
    cr_maskl(EMMC_PLL_SSC0, EMMC_PLL_SSC0_OC_EN_MASK, EMMC_PLL_SSC0_OC_EN_SHIFT ,0);       /* PLLEMMC_OC_EN = 0 */

    cr_maskl(EMMC_PLL_SSC3, EMMC_PLL_SSC3_NCODE_MASK, EMMC_PLL_SSC3_NCODE_SHIFT,ncode);   /* Ncode_ssc = veriable */
    cr_maskl(EMMC_PLL_SSC3, EMMC_PLL_SSC3_FCODE_MASK, EMMC_PLL_SSC3_FCODE_SHIFT,fcode);   /* Fcode_ssc = veriable */
    cr_maskl(EMMC_PLL_SSC5, EMMC_PLL_SSC5_DOT_GRAN_MASK, EMMC_PLL_SSC5_DOT_GRAN_SHIFT,4);         /* dot_gran=4 */
    cr_maskl(EMMC_PLL_SSC5, EMMC_PLL_SSC5_GRAN_EST_MASK, EMMC_PLL_SSC5_GRAN_EST_SHIFT,rang);   /* Gran_est=49166(30k) */

    cr_maskl(EMMC_PLL_SSC0, EMMC_PLL_SSC0_OC_EN_MASK, EMMC_PLL_SSC0_OC_EN_SHIFT ,1);       /* PLLEMMC_OC_EN = 1 */

    loop = 200000;
    while(loop--){  /* polling PLLEMMC_Oc_done flag */
        if(cr_readl(EMMC_PLL_SSC6) & BIT(EMMC_PLL_SSC6_OC_DONE_SHIFT)){
            break;
        }
    }

    cr_maskl(EMMC_PLL_SSC0, EMMC_PLL_SSC0_OC_EN_MASK, EMMC_PLL_SSC0_OC_EN_SHIFT ,0);       /* PLLEMMC_OC_EN = 0 */
    if(loop){
        cr_maskl(EMMC_PLL_SSC5, EMMC_PLL_SSC5_EN_SSC_MASK, EMMC_PLL_SSC5_EN_SSC_SHIFT ,1);   /* En_ssc =1 */
        ret = 0;
    }else{
        EM_ERR("SSC enable fail!!!\n");
    }

    return ret;
}

/************************************************************************
 *
 Available phase margin
    Read    3~12 / 20~28
    Write   4~13 / 20~31
    Cmd     4~31
    DS      0~27

 typedef struct{
    u32                 ctl;
    u8                  ncode_t;
    u8                  sdiv2;
    u8                  r_phase;
    u8                  w_phase;
    u8                  icp;
    u8                  pi_isel;
    u16                 ncode;
    u16                 fcode;
    u32                 range;
}  mmc_param_t;
 ************************************************************************/
mmc_param_t mmc_defaul_tparam[16]= {
/*  ctl/ncode_t/sdiv2/r_phase/w_phase/icp/pi_isel/ncode_ssc/fcode_ssc/range_ssc              */
    { 1,    40,    0,      19,   0xff,  2,     3,    0x27,    0x11E,   0x5DAA }, /* 193.5Mhz SSC: 2% 30KHz */
    { 0,    39,    0,      10,   0xff,  2,     3,       0,        0,        0 }, /* 189Mhz   */
    { 0,    37,    0,       8,   0xff,  2,     3,       0,        0,        0 }, /* 180Mhz   */
    { 0,    35,    0,       7,   0xff,  1,     3,       0,        0,        0 }, /* 171Mhz   */
    { 0,    33,    0,      23,   0xff,  1,     3,       0,        0,        0 }, /* 162Mhz   */
    { 0,    30,    0,      21,   0xff,  1,     2,       0,        0,        0 }, /* 148.5Mhz */
    { 0,    28,    0,      20,   0xff,  1,     2,       0,        0,        0 }, /* 139.5Mhz */
    { 0,    26,    0,      21,   0xff,  1,     2,       0,        0,        0 }, /* 130.5Mhz */
    { 0,    24,    0,      12,   0xff,  1,     2,       0,        0,        0 }, /* 121.5Mhz */
    { 0,    21,    0,      12,   0xff,  1,     2,       0,        0,        0 }, /* 108Mhz   */
    { 0,    41,    1,      12,   0xff,  2,     3,       0,        0,        0 }, /* 99Mhz    */
    { 0,    37,    1,      10,   0xff,  2,     3,       0,        0,        0 }, /* 90Mhz    */
    { 0,    33,    1,       7,   0xff,  1,     3,       0,        0,        0 }, /* 81Mhz    */
    { 0,    29,    1,       5,   0xff,  1,     2,       0,        0,        0 }, /* 72Mhz    */
    { 0,    23,    1,       3,   0xff,  1,     2,       0,        0,        0 }, /* 58.5Mhz  */
    { 0,    19,    1,       8,   0xff,  0,     1,       0,        0,        0 }  /* 49.5Mhz include under */
};

/************************************************************************
 *
 * PLL Clock Formula:
 *
 *   PLLEMMC_NCODE_T = 0x1800_0720[23:16]
 *   PLLSN           = 0x1800_0700[10:8] <==1(currently)
 *   Note : max PLLSN is 4
 *   PLLSDIV2        = 0x1800_0700[17]   <==0(currently)
 *   Fout = 27 *( PLLEMMC_NCODE_T +3)/ (PLLSN+2)/( PLLSDIV2+1)/2
 *        = 27 *( PLLEMMC_NCODE_T +3)/ 6
 *   PLLEMMC_NCODE_T = (((6 * Fout )/27)-3)
 *
 * if PLLSN is 4
 *   PLLEMMC_NCODE_T = (((12 * Fout )/27)-3)
 *
 *                      PLLSN=  [1]    [4]
 * 200Mhz : 0x1800_0720[23:16] = 41     85
 * 190Mhz : 0x1800_0720[23:16] = 39 81
 * 180Mhz : 0x1800_0720[23:16] = 37 77
 * 170Mhz : 0x1800_0720[23:16] = 35 72
 * 160Mhz : 0x1800_0720[23:16] = 32 68
 * 150Mhz : 0x1800_0720[23:16] = 30 63
 * 140Mhz : 0x1800_0720[23:16] = 28 59
 * 130Mhz : 0x1800_0720[23:16] = 26 54
 * 120Mhz : 0x1800_0720[23:16] = 24 50
 * 110Mhz : 0x1800_0720[23:16] = 21 45
 * 100Mhz : 0x1800_0720[23:16] = 19 41
 *  90Mhz : 0x1800_0720[23:16] = 17 37
 *  80Mhz : 0x1800_0720[23:16] = 15 32
 *  70Mhz : 0x1800_0720[23:16] = 12 28
 *  60Mhz : 0x1800_0720[23:16] = 10 23
 *  50Mhz : 0x1800_0720[23:16] =  8 19
 *
 * ICP setting :
 * under 50Mhz =>                 => ICP=? => 0x18000700[16:12]=0x00
 * 50,60,70Mhz => VCO=100,120,140 => ICP=5 => 0x18000700[16:12]=0x01
 * 80,90,100,110, 120,130,140Mhz
 *             => VCO=160,180,200,220,240,260,280
 *                                => ICP=10 => 0x18000700[16:12]=0x03
 * 150,160,170,180,190,200Mhz
 *             => VCO=300,320,340,360,380,400
 *                                => ICP=20 => 0x18000700[16:12]=0x07
 *
 ************************************************************************/
static
int dw_em_set_PLL_clk(struct rtksd_host *sdport, u32 targ_clk,u32 clk_div,u32 init)
{
    unsigned long flags;
    int err = 0;
    unsigned int pllemmc_ncode_t = 0;
    unsigned int loop = 20000;
    unsigned int icp = 0;
    unsigned int sdiv2 = 0;
    unsigned int sdiv2_old = 0;
    unsigned int pi_isel = 0;
    unsigned int r_phase = 0;
    unsigned int pll_sta = 0;
    unsigned int ncode_ssc = 0;
    unsigned int fcode_ssc = 0;
    unsigned int ssc_rang = 0;
    unsigned char idx;
    unsigned int pad_set[PAD_SET_CNT];
    u32 config1;
    u32 config2;
    u32 config3;
    u32 config4;
    u32 config5;
    u32 config6;
    u32 config7;
    mmc_param_t *tmp_param_ptr;
    unsigned int ldo_trim_info_otp = 0;
    bool ret = 0;

#define SSC_EN BIT(EMMC_PLL_SSC5_EN_SSC_SHIFT)

    if (sdport->cur_PLL == targ_clk) {
        spin_lock_irqsave(&sdport->lock, flags);
        dw_em_set_div(sdport,clk_div);
        spin_unlock_irqrestore(&sdport->lock, flags);
        mmcmsg3(" skip\n");
        return 0;
    }

    if (targ_clk >= CLK_200Mhz){
        idx = 0;
    }else if (targ_clk >= CLK_190Mhz){
        idx = 1;
    }else if (targ_clk >= CLK_180Mhz){
        idx = 2;
    }else if (targ_clk >= CLK_170Mhz){
        idx = 3;
    }else if (targ_clk >= CLK_160Mhz){
        idx = 4;
    }else if (targ_clk >= CLK_150Mhz){
        idx = 5;
    }else if (targ_clk >= CLK_140Mhz){
        idx = 6;
    }else if (targ_clk >= CLK_130Mhz){
        idx = 7;
    }else if (targ_clk >= CLK_120Mhz){
        idx = 8;
    }else if (targ_clk >= CLK_110Mhz){
        idx = 9;
    }else if (targ_clk >= CLK_100Mhz){
        idx = 10;
    }else if (targ_clk >= CLK_90Mhz){
        idx = 11;
    }else if (targ_clk >= CLK_80Mhz){
        idx = 12;
    }else if (targ_clk >= CLK_70Mhz){
        idx = 13;
    }else if (targ_clk >= CLK_60Mhz){
        idx = 14;
    }else {
        idx = 15;
    }

    if(rtk_emmc_test_ctl & EMMC_EN_PARAM){
        EM_ALERT("%s(%d)Use Bootcode pre-setting parameter\n",__func__,__LINE__);
        tmp_param_ptr = sdport->mmc_boot_param;
    }else {
        tmp_param_ptr = mmc_defaul_tparam;
    }

    pllemmc_ncode_t = tmp_param_ptr[idx].ncode_t;
    sdiv2           = tmp_param_ptr[idx].sdiv2;
    r_phase         = tmp_param_ptr[idx].r_phase;
    icp             = tmp_param_ptr[idx].icp;
    pi_isel         = tmp_param_ptr[idx].pi_isel;

    ncode_ssc = tmp_param_ptr[idx].ncode_ssc;
    fcode_ssc = tmp_param_ptr[idx].fcode_ssc;
    ssc_rang  = tmp_param_ptr[idx].range_ssc;

    /* disable SSCG func by pre-setting of bootcode */
    if(rtk_emmc_test_ctl & EMMC_DIS_SSCG){
        pll_sta &= ~SSC_EN;
    }else{
        if(tmp_param_ptr[idx].ctl & MMC_EN_SSCG)
            pll_sta |= SSC_EN;
    }

    if(rtk_emmc_test_ctl & EMMC_EN_PARAM){
        EM_ALERT("%s(%d)idx%d==================================\n",__func__,__LINE__,idx);
        EM_ALERT("    pllemmc_ncode_t=%u(0x%x)\n",pllemmc_ncode_t,pllemmc_ncode_t);
        EM_ALERT("    r_phase=%u(0x%x)\n",r_phase,r_phase);
        EM_ALERT("    icp=%u(0x%x)\n",icp,icp);

        if(pll_sta & SSC_EN){
            EM_ALERT("SSCG enable !!!\n");
            EM_ALERT("    ncode_ssc=%u(0x%x)\n",ncode_ssc,ncode_ssc);
            EM_ALERT("    fcode_ssc=%u(0x%x)\n",fcode_ssc,fcode_ssc);
            EM_ALERT("    ssc_rang=%u(0x%x)\n",ssc_rang,ssc_rang);
        }
    }

    if(!init){
        /* backup IP status first */
        config1 = cr_readl(EM_DWC_PWREN);
        config2 = cr_readl(EM_DWC_CLKDIV);
        config3 = cr_readl(EM_DWC_CTYPE);
        config4 = cr_readl(EM_DWC_UHS_REG);
        config5 = cr_readl(EM_DWC_DBADDR);
        config6 = cr_readl(EM_DWC_EMMC_DDR_REG);
        config7 = cr_readl(EM_HALF_CYCLE_CAL_EN);
        dw_em_dis_sd_tune(sdport);
    }

    if(dw_em_save_pad(sdport,pad_set,PAD_SET_CNT))
        WARN(1,"iopad array size mismatch!\n");

    spin_lock_irqsave(&sdport->lock, flags);

    /* reset IP  */
    dw_em_reset_IP();
    udelay(10);

    /* gating digital clock */
    cr_writel(CLKEN_EM_DW,RTK_EM_DW_CLKEN);

    if(clk_div){
        /*
         * if clk_div is true, meaning bus clock lower or equal 50Mhz,
         * use default phase
         */
        r_phase = DEFAULT_R_PHASE;
    }else{
        if(sdport->cur_timing == MMC_TIMING_MMC_HS400){
            if(fix_phase_flag & FIX_RPHASE400){
                r_phase = GET_FIX_RPHA400();
                EM_ALERT("%s(%d)Use Bootcode pre-setting parameter\n",__func__,__LINE__);
                EM_ALERT("Set HS400 cmd read phase: %d\n", r_phase);
            }else if(get_mmc_tuning_200_done(sdport->mmc)){
                /* If clock get from PLL directly ,use new phase setting clock. */
                r_phase = sdport->tud_r_pha200;
            }
        }else if(sdport->cur_timing ==  MMC_TIMING_MMC_HS200){
            if(fix_phase_flag & FIX_RPHASE200){
                r_phase = GET_FIX_RPHA200();
                EM_ALERT("%s(%d)Use Bootcode pre-setting parameter\n",__func__,__LINE__);
                EM_ALERT("Set HS200 read phase: %d\n", r_phase);
            }
        }else if(targ_clk == CLK_50Mhz){
            if(fix_phase_flag & FIX_RPHASE50){
                /* Manual set 50M read phase */
                r_phase = GET_FIX_RPHA50(rtk_fix_50M_phase);
                EM_ALERT("%s(%d)Use Bootcode pre-setting parameter\n",__func__,__LINE__);
                EM_ALERT("Set HS50 read phase: %d\n", r_phase);
            }else if (get_mmc_tuning_50_done(sdport->mmc)){
                r_phase = sdport->tud_r_pha50;
            }
        }
    }
    /* w_phase[15:10];r_phase [5:0];c_phase [25:20]; */
    dw_em_init_phase(sdport,0xff,r_phase,0xff);

    if(init){
        cr_maskl(EMMC_PLL_SET, EMMC_PLL_SET_ALL_RSTB_MASK, EMMC_PLL_SET_ALL_RSTB_SHFIT ,0);    /* PLLALLRSTB = 0 */
        cr_maskl(EMMC_PLL_SET, EMMC_PLL_SET_CKSSC_INV_MASK, EMMC_PLL_SET_CKSSC_INV_SHFIT ,0);   /* PLL_EMMC_CKSSC_INV = 0 */
        cr_maskl(EMMC_PLL_SSC0,EMMC_PLL_SSC0_OC_STEP_MASK, EMMC_PLL_SSC0_OC_STEP_SHIFT ,1);   /* PLLEMMC_OC_STEP_SET = 1 */
        cr_maskl(EMMC_PLL_CTRL,EMMC_PLL_CTRL_FREEZE_MASK, EMMC_PLL_CTRL_FREEZE_SHFIT ,1);  /* PLLEMMC_FREEZE */
        cr_maskl(EMMC_PLL_CTRL,EMMC_PLL_CTRL_RSVD_L_MASK, EMMC_PLL_CTRL_RSVD_L_SHFIT ,0);   /* LDO RSVD[0]=0; RSVD[0]=0x18000704[8] */
        cr_maskl(EMMC_PLL_CTRL,EMMC_PLL_CTRL_RSVD_H_MASK, EMMC_PLL_CTRL_RSVD_H_SHFIT ,1);  /* LDO RSVD[5:4]=1; RSVD[5:4]=0x18000704[13:12] */
        ret = rtk_otp_field_read(OTP_FIELD_EMMC_LDO_TRIM_INFO, (unsigned char *)&ldo_trim_info_otp, 4);
        if(!ret)
            EM_ERR("OTP_FIELD_EMMC_LDO_TRIM_INFO not exist\n");
        /* LDO RSVD[5:4]=otp OTP_FIELD_EMMC_LDO_TRIM_INFO; RSVD[5:4]=0x18000704[13:12] */
        switch(ldo_trim_info_otp){
            case 4:
                cr_maskl(EMMC_PLL_CTRL, EMMC_PLL_CTRL_RSVD_H_MASK, EMMC_PLL_CTRL_RSVD_H_SHFIT, 0);
                break;
            case 5:
                cr_maskl(EMMC_PLL_CTRL, EMMC_PLL_CTRL_RSVD_H_MASK, EMMC_PLL_CTRL_RSVD_H_SHFIT, 1);
                break;
            case 6:
                cr_maskl(EMMC_PLL_CTRL, EMMC_PLL_CTRL_RSVD_H_MASK, EMMC_PLL_CTRL_RSVD_H_SHFIT, 2);
                break;
            case 7:
                cr_maskl(EMMC_PLL_CTRL, EMMC_PLL_CTRL_RSVD_H_MASK, EMMC_PLL_CTRL_RSVD_H_SHFIT, 3);
                break;
            default:
                break;
        }
        cr_maskl(EMMC_PLL_CTRL, EMMC_PLL_CTRL_POWER_MASK, EMMC_PLL_CTRL_POWER_SHFIT ,2);   /* LDO power on, PLLEMMC power on */
        cr_maskl(EMMC_PLL_SSC2, EMMC_PLL_SSC2_EN_PI_DEBUG_MASK, EMMC_PLL_SSC2_EN_PI_DEBUG_SHIFT ,1);   /* En_pi_debug, relative PI_CUR_SEL*/
        cr_maskl(EMMC_PLL_SSC4, EMMC_PLL_SSC4_NCODE_MASK, EMMC_PLL_SSC4_NCODE_SHIFT,pllemmc_ncode_t); /* To setting PLLEMMC_NCODE_T */
        cr_maskl(EMMC_PLL_SSC5, EMMC_PLL_SSC5_EN_SSC_MASK, EMMC_PLL_SSC5_EN_SSC_SHIFT ,0);   /* En_ssc =0; disable SSC */

        udelay(5);

        cr_maskl(EMMC_PLL_SET, EMMC_PLL_SET_PLLSN_MASK, EMMC_PLL_SET_PLLSN_SHFIT ,1);    /* PLLSN = 1, */
        cr_maskl(EMMC_PLL_SET, EMMC_PLL_SET_PLLSCP_MASK, EMMC_PLL_SET_PLLSCP_SHFIT,0);    /* CP */
        cr_maskl(EMMC_PLL_SET, EMMC_PLL_SET_PLLSCS_MASK, EMMC_PLL_SET_PLLSCS_SHFIT,3);    /* CS */
        cr_maskl(EMMC_PLL_SET, EMMC_PLL_SET_PLLSSI_MASK, EMMC_PLL_SET_PLLSSI_SHFIT,icp);  /* PLL Charger Pump Current IchDpll[16:12] (PLLSSI) */
        cr_maskl(EMMC_PLL_SET, EMMC_PLL_SET_PLLSDIV2_MASK, EMMC_PLL_SET_PLLSDIV2_SHFIT,sdiv2);    /* PLLSDIV2 = 0, 27x(19+3)/3/2=99MHz */
        cr_maskl(EMMC_PLL_SSC1, EMMC_PLL_SSC1_PI_CUR_SEL_MASK, EMMC_PLL_SSC1_PI_CUR_SEL_SHIFT,pi_isel);  /* PI_CUR_SEL */
        udelay(5);

        cr_maskl(EMMC_PLL_SET, EMMC_PLL_SET_ALL_RSTB_MASK, EMMC_PLL_SET_ALL_RSTB_SHFIT ,1);    /* PLLALLRSTB = 1 */
        cr_maskl(EMMC_PLL_SSC0, EMMC_PLL_SSC0_OC_EN_MASK, EMMC_PLL_SSC0_OC_EN_SHIFT ,1);   /* PLLEMMC_OC_EN = 1 */
    }else{
        sdiv2_old = (cr_readl(EMMC_PLL_SET)>>EMMC_PLL_SET_PLLSDIV2_SHFIT) & EMMC_PLL_SET_PLLSDIV2_MASK;
        cr_maskl(EMMC_PLL_CTRL, EMMC_PLL_CTRL_FREEZE_MASK, EMMC_PLL_CTRL_FREEZE_SHFIT ,1);  /* PLLEMMC_FREEZE */
        cr_maskl(EMMC_PLL_SSC5, EMMC_PLL_SSC5_EN_SSC_MASK, EMMC_PLL_SSC5_EN_SSC_SHIFT ,0);   /* En_ssc =0; disable SSC */
        cr_maskl(EMMC_PLL_SSC0, EMMC_PLL_SSC0_OC_STEP_MASK, EMMC_PLL_SSC0_OC_STEP_SHIFT ,0x200);   /* PLLEMMC_OC_STEP_SET = 0x200 */
        cr_maskl(EMMC_PLL_SSC0, EMMC_PLL_SSC0_OC_EN_MASK, EMMC_PLL_SSC0_OC_EN_SHIFT ,0);   /* PLLEMMC_OC_EN = 0 */
        cr_maskl(EMMC_PLL_SET, EMMC_PLL_SET_PLLSN_MASK, EMMC_PLL_SET_PLLSN_SHFIT ,1);    /* PLLSN = 1, */
        cr_maskl(EMMC_PLL_SET, EMMC_PLL_SET_PLLSSI_MASK, EMMC_PLL_SET_PLLSSI_SHFIT,icp);  /* PLL Charger Pump Current IchDpll[16:12] (PLLSSI) */
        cr_maskl(EMMC_PLL_SSC1, EMMC_PLL_SSC1_PI_CUR_SEL_MASK, EMMC_PLL_SSC1_PI_CUR_SEL_SHIFT,pi_isel);  /* PI_CUR_SEL */
        if (sdiv2 > sdiv2_old) {
            /* high --> low */
            cr_maskl(EMMC_PLL_SET, EMMC_PLL_SET_PLLSDIV2_MASK, EMMC_PLL_SET_PLLSDIV2_SHFIT,sdiv2);    /* PLLSDIV2 = 0, 27x(19+3)/3/2=99MHz */
            cr_maskl(EMMC_PLL_SSC4, EMMC_PLL_SSC4_NCODE_MASK, EMMC_PLL_SSC4_NCODE_SHIFT,pllemmc_ncode_t); /* To setting PLLEMMC_NCODE_T */
        }else{
            /* low --> high */
            cr_maskl(EMMC_PLL_SSC4, EMMC_PLL_SSC4_NCODE_MASK, EMMC_PLL_SSC4_NCODE_SHIFT,pllemmc_ncode_t); /* To setting PLLEMMC_NCODE_T */
            cr_maskl(EMMC_PLL_SET, EMMC_PLL_SET_PLLSDIV2_MASK, EMMC_PLL_SET_PLLSDIV2_SHFIT,sdiv2);    /* PLLSDIV2 = 0, 27x(19+3)/3/2=99MHz */
        }
        cr_maskl(EMMC_PLL_SSC0, EMMC_PLL_SSC0_OC_EN_MASK, EMMC_PLL_SSC0_OC_EN_SHIFT ,1);   /* PLLEMMC_OC_EN = 1 */
    }

    spin_unlock_irqrestore(&sdport->lock, flags);

    while(--loop){  /* polling PLLEMMC_Oc_done flag */
        if(cr_readl(EMMC_PLL_SSC6) & BIT(EMMC_PLL_SSC6_OC_DONE_SHIFT)){
            break;
        }
        /* max waiting for 100ms = 20000*5us */
        udelay(5);
    }

    cr_maskl(EMMC_PLL_SSC0, EMMC_PLL_SSC0_OC_EN_MASK, EMMC_PLL_SSC0_OC_EN_SHIFT ,0);   /* PLLEMMC_OC_EN = 0 */
    udelay(150);
    cr_maskl(EMMC_PLL_CTRL, EMMC_PLL_CTRL_FREEZE_MASK, EMMC_PLL_CTRL_FREEZE_SHFIT ,0);  /* release PLLEMMC_FREEZE */

    if(!loop){
        err = -1;
        goto OUT;
    }

    /* To enable SSC, just care about PLL clock, no matter divider or else. */
    if(pll_sta & SSC_EN){
        dw_em_set_PLL_SSC(sdport, ncode_ssc, fcode_ssc ,ssc_rang);
    }

    sdport->cur_PLL = targ_clk;

OUT:
    spin_lock_irqsave(&sdport->lock, flags);
    /* crt sync reset */
    dw_em_crt_reset(sdport,1);

    /* reset IP  */
    dw_em_reset_IP();
    udelay(10);

    /*config IP */
    dw_em_IP_config(sdport);

    dw_em_restore_pad(sdport,pad_set);
    if(!init){
        /* restore previously IP setting */
        dw_em_set_div(sdport,clk_div);
        cr_writel(config3,EM_DWC_CTYPE);
        cr_writel(config4,EM_DWC_UHS_REG);
        cr_writel(config5,EM_DWC_DBADDR);
        cr_writel(config6,EM_DWC_EMMC_DDR_REG);
        if(config7 & BIT(DS_CYCLE_CAL_EN_SHIFT)){
            dw_em_set_ds_delay(sdport, sdport->cur_ds_tune);
        }
    }
    spin_unlock_irqrestore(&sdport->lock, flags);

    return err;

}

/************************************************************************
 *
 ************************************************************************/
#define SDMMC_CMD_VOLT_SWITCH   BIT(28)
static
void dw_em_wait_while_busy(struct rtksd_host *sdport, u32 cmd_flags)
{
    unsigned long timeout = jiffies + msecs_to_jiffies(500);

    /*
     * Databook says that before issuing a new data transfer command
     * we need to check to see if the card is busy.  Data transfer commands
     * all have SDMMC_CMD_PRV_DAT_WAIT set, so we'll key off that.
     *
     * ...also allow sending for SDMMC_CMD_VOLT_SWITCH where busy is
     * expected.
     */
    if ((cmd_flags & SDMMC_CMD_PRV_DAT_WAIT) &&
        !(cmd_flags & SDMMC_CMD_VOLT_SWITCH)) {
        while (cr_readl(EM_DWC_STATUS) & SDMMC_DAT0_BUSY) {
            if (time_after(jiffies, timeout)) {
                /* Command will fail; we'll pass error then */
                EM_ALERT("Busy; trying anyway\n");
                break;
            }
            udelay(10);
        }
    }
}

/************************************************************************
 *
 ************************************************************************/
static
void dw_em_send_cmd(struct rtksd_host *sdport, u32 cmd, u32 sd_arg)
{
    unsigned long timeout = jiffies + msecs_to_jiffies(500);
    unsigned int cmd_status = 0;
    unsigned int reginfo = 0;

    reginfo = cr_readl(EM_DWC_INTMASK); /*backup int mask */
    cr_writel(sd_arg, EM_DWC_CMDARG);
    wmb();
    dw_em_wait_while_busy(sdport, cmd);
    cr_writel(SDMMC_CMD_START | cmd, EM_DWC_CMD);

    while (time_before(jiffies, timeout)) {
        cmd_status = cr_readl(EM_DWC_CMD);
        if (!(cmd_status & SDMMC_CMD_START))
            goto OUT;
    }

    EM_ERR("Timeout sending command (cmd %#x arg %#x status %#x)\n",
           cmd, sd_arg, cmd_status);
OUT:
    cr_writel(0xffffffff, EM_DWC_RINTSTS);  /* Clear all int result. */
    cr_writel(reginfo, EM_DWC_INTMASK);     /*restore int mask */
}

/************************************************************************
 *  Notice: em_div must be even value.
 ************************************************************************/
static
void dw_em_set_div(struct rtksd_host *sdport,u32 em_div)
{
    u32 row_div;
    u32 reginfo;

    if(em_div)
        row_div = (em_div>>1);
    else
        row_div = 0;

    /* reset div every time change clock */

    /* disable clock */
    cr_writel(0, EM_DWC_CLKENA);
    cr_writel(0, EM_DWC_CLKSRC);

    /* inform CIU */
    dw_em_send_cmd(sdport, SDMMC_CMD_UPD_CLK | SDMMC_CMD_PRV_DAT_WAIT, 0);

    /* set clock to desired speed */
    cr_writel(row_div, EM_DWC_CLKDIV);

    /* inform CIU */
    dw_em_send_cmd(sdport, SDMMC_CMD_UPD_CLK | SDMMC_CMD_PRV_DAT_WAIT, 0);

    /* enable clock */
    if(rtk_emmc_test_ctl & EMMC_ALWAYS_CLK){
        EM_ALERT("%s(%d)Use Bootcode pre-setting parameter. clock always on.\n",
            __func__,__LINE__);
        cr_writel(BIT(0), EM_DWC_CLKENA);
    }else{
        /* only low power if no SDIO */
        cr_writel((BIT(16) | BIT(0)), EM_DWC_CLKENA);
    }
    /* inform CIU */
    dw_em_send_cmd(sdport, SDMMC_CMD_UPD_CLK | SDMMC_CMD_PRV_DAT_WAIT, 0);

    sdport->cur_div = em_div;

    mmcmsg2("CLK divide %u\n",(row_div<<1));

    /* low speed phase function */
    reginfo = (cr_readl(EM_CLK_OUT_DELAY) & ~0x1ff);
    if(em_div > 10){	//if digital div big than 10, enable clock delay
        reginfo |= (CLK_OUT_DELAY_EN|MMC_LOW_SPEED_PHASE);
    }
    cr_writel(reginfo, EM_CLK_OUT_DELAY);
}

/************************************************************************
 *
 ************************************************************************/
static
void dw_em_set_clk(struct rtksd_host *sdport,u32 em_clock)
{
    u32 base_clk = 0;
    u32 clk_div = 0;
    u32 seted_clk = 0;
    u32 idx = 0;
    u32 quirk_num = 0;

    quirk_num = sdport->mid;

    if(em_clock > CLK_200Mhz){
        EM_NOTICE("emmc: unknow clock target %u, force to 25Mhz\n",
               em_clock);
        em_clock = CLK_25Mhz;
    }

    if(em_clock <=CLK_400Khz ){
        /* select 250KHz */
        idx = 0;
        seted_clk = CLK_200Khz;
        base_clk = CLK_50Mhz;
        clk_div = 200;
    }else if(em_clock < CLK_15Mhz){
        /* 10Mhz */
        idx = 1;
        seted_clk = CLK_10Mhz;
        base_clk = CLK_60Mhz;
        clk_div = 6;
    }else if(em_clock < CLK_20Mhz){
        /* 15Mhz */
        idx = 2;
        seted_clk = CLK_15Mhz;
        base_clk = CLK_60Mhz;
        clk_div = 4;
    }else if(em_clock < CLK_25Mhz){
        /* 20Mhz */
        idx = 3;
        seted_clk = CLK_20Mhz;
        base_clk = CLK_80Mhz;
        clk_div = 4;
    } else if (em_clock < CLK_30Mhz) {
        /* 25Mhz */
        idx = 4;
        seted_clk = CLK_25Mhz;
        base_clk = CLK_50Mhz;
        clk_div = 2;
    } else if (em_clock < CLK_40Mhz) {
        /* 30Mhz */
        idx = 5;
        seted_clk = CLK_30Mhz;
        base_clk = CLK_60Mhz;
        clk_div = 2;
    } else if (em_clock < CLK_50Mhz) {
        /* 40Mhz */
        idx = 6;
        seted_clk = CLK_40Mhz;
        base_clk = CLK_80Mhz;
        clk_div = 2;
    } else if (em_clock < CLK_60Mhz) {
        /* 50Mhz */
        idx = 7;
        seted_clk = CLK_50Mhz;
        base_clk = CLK_50Mhz;
        clk_div = 0;
    } else if (em_clock < CLK_70Mhz) {
        /* 60Mhz */
        idx = 8;
        seted_clk = CLK_60Mhz;
        base_clk = CLK_60Mhz;
        clk_div = 0;
    } else if (em_clock < CLK_80Mhz) {
        /* 70Mhz */
        idx = 9;
        seted_clk = CLK_70Mhz;
        base_clk = CLK_70Mhz;
        clk_div = 0;
    } else if (em_clock < CLK_90Mhz) {
        /* 80Mhz */
        idx = 10;
        seted_clk = CLK_80Mhz;
        base_clk = CLK_80Mhz;
        clk_div = 0;
    } else if (em_clock < CLK_100Mhz) {
        /* 90Mhz */
        idx = 11;
        seted_clk = CLK_90Mhz;
        base_clk = CLK_90Mhz;
        clk_div = 0;
    } else if (em_clock < CLK_110Mhz) {
        /* 100Mhz */
        idx = 12;
        seted_clk = CLK_100Mhz;
        base_clk  = CLK_100Mhz;
        clk_div   = 0;
    } else if (em_clock < CLK_120Mhz) {
        /* 110Mhz */
        idx = 13;
        seted_clk = CLK_110Mhz;
        base_clk = CLK_110Mhz;
        clk_div = 0;
    } else if (em_clock < CLK_130Mhz) {
        /* 120Mhz */
        idx = 14;
        seted_clk = CLK_120Mhz;
        base_clk  = CLK_120Mhz;
        clk_div   = 0;
    } else if (em_clock < CLK_140Mhz) {
        /* 130Mhz */
        idx = 15;
        seted_clk = CLK_130Mhz;
        base_clk = CLK_130Mhz;
        clk_div = 0;
    } else if (em_clock < CLK_150Mhz) {
        /* 140Mhz */
        idx = 16;
        seted_clk = CLK_140Mhz;
        base_clk  = CLK_140Mhz;
        clk_div   = 0;
    } else if (em_clock < CLK_160Mhz) {
        /* 150Mhz */
        idx = 17;
        seted_clk = CLK_150Mhz;
        base_clk = CLK_150Mhz;
        clk_div = 0;
    } else if (em_clock < CLK_170Mhz) {
        /* 160Mhz */
        idx = 18;
        seted_clk = CLK_160Mhz;
        base_clk  = CLK_160Mhz;
        clk_div   = 0;
    } else if (em_clock < CLK_180Mhz) {
        /* 170Mhz */
        idx = 19;
        seted_clk = CLK_170Mhz;
        base_clk = CLK_170Mhz;
        clk_div = 0;
    } else if (em_clock < CLK_190Mhz) {
        /* 180Mhz */
        idx = 20;
        seted_clk = CLK_180Mhz;
        base_clk  = CLK_180Mhz;
        clk_div   = 0;
    } else if (em_clock < CLK_200Mhz) {
        /* 190Mhz */
        idx = 21;
        seted_clk = CLK_190Mhz;
        base_clk = CLK_190Mhz;
        clk_div = 0;
    }else{
        /*
        mid 0x11 ==> Toshiba checked
        mid 0x13 ==> Micron  not yet
        mid 0x15 ==> Samsung checked
        mid 0x90 ==> Hynix checked
         */
        idx = 22;
        seted_clk = CLK_200Mhz;
        base_clk  = CLK_200Mhz;
        clk_div   = 0;
        EM_NOTICE("%s: MID:0x%x eMMC detected\n",
                DRIVER_NAME,quirk_num);

    }

    if(sdport->cur_clock == seted_clk){

        /* do nothing */
    }else{
        if (sdport->int_mode)
            rtkemmc_disable_int(sdport);
        dw_em_set_PLL_clk(sdport,base_clk,clk_div,0);
#ifdef CONFIG_LGEMMC_PARTITION
        EM_NOTICE("EMMC: Clock change to %d Hz from %d Hz\n",
        seted_clk,sdport->cur_clock);
        sdport->cur_clock = seted_clk;
#else
        sdport->cur_clock = seted_clk;
        EM_NOTICE("%s: clk change to %s(%u) [0x%x=0x%08x]\n",
            DRIVER_NAME, dw_clk_tlb[idx], idx,
            EMMC_PLLPHASE_CTRL,cr_readl(EMMC_PLLPHASE_CTRL));
#endif
        if (sdport->int_mode)
            rtkemmc_enable_int(sdport);
    }

}

/************************************************************************
 *
 ************************************************************************/
static
void dw_em_clear_int_sta(u32 hold_int)
{
    if(hold_int)
        cr_writel(0x0, EM_DWC_INTMASK); /* disable int. */

    cr_writel(0xffffffff, EM_DWC_RINTSTS);  /* Clear all int result. */
}

/************************************************************************
 *
 ************************************************************************/
#if 0
static
void dw_em_enable_int_sta(unsigned int_event)
{
    cr_writel(int_event, EM_DWC_INTMASK);

    /* enable global int. */
    cr_writel(cr_readl(EM_DWC_CTRL) | SDMMC_CTRL_INT_ENABLE, EM_DWC_CTRL);
}
#endif

/************************************************************************
 *
 ************************************************************************/
static
void dw_em_clear_int_sta_IDMA(u32 hold_int)
{
    if(hold_int)
        cr_writel(0x0, EM_DWC_IDINTEN);
    /* Clear all int result. */
    cr_writel(0xffffffff, EM_DWC_IDSTS);
}

/************************************************************************
 *
 ************************************************************************/
#if 0
static
void dw_em_enable_int_sta_IDMA(unsigned int_event)
{

    cr_writel(int_event, EM_DWC_IDINTEN);

    /* enable global int. */
    cr_writel(cr_readl(EM_DWC_CTRL) | SDMMC_CTRL_INT_ENABLE, EM_DWC_CTRL);
}
#endif

/************************************************************************
 *
 ************************************************************************/
static
void dw_em_clear_int_wrap(unsigned int hold_int)
{
    if(hold_int)
        cr_writel(0x07, EM_INT_MASK);

    cr_writel(0x07,EM_INT_STATUS);
}

/************************************************************************
 *
 ************************************************************************/
#if 0
static
void dw_em_enable_int_wrap(unsigned int_event)
{
    /* Note : It's not like normal.
     * Zero enable here.
     * To reference spec if doubt.
     */
    cr_writel(~(int_event), EM_INT_MASK);
}
#endif

/************************************************************************
 * IP configure
 ************************************************************************/
static
void dw_em_IP_config(struct rtksd_host *sdport)
{
    u32 reginfo = 0;

    cr_writel(BIT(0), EM_DWC_PWREN); /* enable power of device 0. */

    /* int mask setting */
    cr_writel(reginfo, EM_DWC_INTMASK);
    cr_writel(0xffffffff, EM_DWC_RINTSTS);  /* Clear all int result. */

    /* disable IDMAC int */
    cr_writel(0, EM_DWC_IDINTEN);

    cr_writel(0xffffff40, EM_DWC_TMOUT);    /* set timeout of data and response as default. */

    cr_writel( (0x05UL<<SDMMC_MSIZE_SHT)        /* set DW_DMA_Multiple_Transaction_Size as 64(0x05) */
              |(0x3fUL<<SDMMC_RX_WMARK_SHT)     /* RX_WMARK=0x3f */
              |(0x40UL<<SDMMC_TX_WMARK_SHT) ,   /* TX_WMARK=0x40 */
                EM_DWC_FIFOTH );

    /* enable write/read threshold as 512 bytes */
    cr_writel((0x0200 << 16) | BIT(2) | BIT(0), EM_DWC_CardThrCtl);

    cr_writel( reginfo,EM_DWC_CTRL );

    cr_writel( cr_readl(EM_WARP_STATUS0)|BIT(18) ,EM_WARP_STATUS0 );
}

/************************************************************************
 *
 ************************************************************************/
static
void dw_em_set_scrip_base(dma_addr_t scrip_base)
{
    /* set Descriptor List Base Address Register */
    cr_writel( scrip_base,EM_DWC_DBADDR );
}

/************************************************************************
 * depend on
 * JESD84-B51-1.pdf chapter 6.15.9 H/W Reset Operation
 * tRSTW : RST_n pulse width; min 1[us]
 ************************************************************************/
static
void dw_em_reset_card(struct rtksd_host *sdport)
{
    u32 reginfo = 0;

    EM_INFO("hw reset\n");
    /* In MMC mode, support card number is 16 */
    reginfo = cr_readl(EM_DWC_CDETECT) & 0xFFFF;

    /* card reset should be implement here. This reg is GPIO control only.  */
    cr_writel(~((unsigned int)reginfo),EM_DWC_RST_n);
    udelay(10);
    cr_writel(reginfo,EM_DWC_RST_n);

    /* Toggle 74 clock here :
     *  The toggle clock will be send out before cmd0 by DW IP.
     *  To reference u32 dw_em_prepare_command()
     *  cmd0 set SDMMC_CMD_INIT bit.
     */
}

/************************************************************************
 *
 ************************************************************************/
static
int dw_em_reset_IP(void)
{
    int ret;
    u32 loop;

    /*
    * To reset controller/FIFO/DMA.
    * This reset is not to effect warper of RTK.
    */
    cr_writel(cr_readl(EM_DWC_CTRL)|(0x07), EM_DWC_CTRL);

    ret = -1;
    loop = 1000;
    while(loop--){
        if((cr_readl(EM_DWC_CTRL)&(0x07)) == 0){
            ret = 0;
            break;
        }
        udelay(1);
    }

    if(ret){
        EM_WARNING("%s: IP reset fail!!!\n",DRIVER_NAME);
    }
    return ret;
}

/************************************************************************

 ************************************************************************/
static
void dw_em_reset_host_IP(struct rtksd_host *sdport,u8 save_en)
{
    unsigned long flags;
    u32 config1;
    u32 config2;
    u32 config3;
    u32 config4;
    u32 config5;
    u32 config6;
    mmcmsg3("\n");

    spin_lock_irqsave(&sdport->lock,flags);
    mmcmsg3("\n");
    if(save_en){
        /* store currently IP setting */
        config1 = cr_readl(EM_DWC_PWREN);
        config2 = cr_readl(EM_DWC_CLKDIV);
        config3 = cr_readl(EM_DWC_CTYPE);
        config4 = cr_readl(EM_DWC_UHS_REG);
        config5 = cr_readl(EM_DWC_DBADDR);
        config6 = cr_readl(EM_DWC_EMMC_DDR_REG);
    }

    /* reset synopsys eMMC IP */
    if(dw_em_reset_IP()){
        /* reset all eMMC module. */
        EM_ERR("%s: IP reset fail, reset Host all!!!\n",DRIVER_NAME);
        dw_em_crt_reset(sdport,0);
    }

    udelay(10);

    dw_em_IP_config(sdport);

    if(save_en){
        /* restore previously IP setting */
        cr_writel(config1,EM_DWC_PWREN);
        dw_em_set_div(sdport,(config2<<1));
        cr_writel(config3,EM_DWC_CTYPE);
        cr_writel(config4,EM_DWC_UHS_REG);
        cr_writel(config5,EM_DWC_DBADDR);
        cr_writel(config6,EM_DWC_EMMC_DDR_REG);
    }

    sdport->rtflags |= RTKCR_FHOST_POWER;
    sdport->reset_event = 0;

    spin_unlock_irqrestore(&sdport->lock, flags);
    mmcmsg3("\n");
}

/************************************************************************
 *
 ************************************************************************/
static
int dw_em_save_pad(struct rtksd_host *sdport,u32* pad_arry,int cnt)
{
    if(cnt != PAD_SET_CNT)
        return -1;
    pad_arry[0] = cr_readl(RTK_IOPAD_CFG1);
    pad_arry[1] = cr_readl(RTK_IOPAD_CFG2);
    pad_arry[2] = cr_readl(RTK_IOPAD_CFG3);
    pad_arry[3] = cr_readl(RTK_IOPAD_SET1);
    pad_arry[4] = cr_readl(RTK_IOPAD_SET2);
    pad_arry[5] = cr_readl(RTK_IOPAD_SET3);
    pad_arry[6] = cr_readl(RTK_IOPAD_SET4);

    return 0;
}

/************************************************************************
 *
 ************************************************************************/
static
int dw_em_restore_pad(struct rtksd_host *sdport,u32* pad_arry)
{
    cr_writel(pad_arry[0] ,RTK_IOPAD_CFG1);   //rst,clk,cmd,
    cr_writel(pad_arry[1] ,RTK_IOPAD_CFG2);   //ds,d5,d3,d4,d0,d1,d2,d7
    cr_writel(pad_arry[2] ,RTK_IOPAD_CFG3);   //d6

    cr_writel(pad_arry[3] , RTK_IOPAD_SET1);   //rst/clk/cmd
    cr_writel(pad_arry[4] , RTK_IOPAD_SET2);   //ds/d5/d3
    cr_writel(pad_arry[5] , RTK_IOPAD_SET3);   //d4/d0/d1
    cr_writel(pad_arry[6] , RTK_IOPAD_SET4);   //d2/d7/d6

    return 0;
}

/************************************************************************
 eMMC Pad driving:
 DRIVE_LEVEL is means driving strength,
 As the level number is bigger, the strength is greater

 ************************************************************************/

/* Future Work : This definition may be written in each platform header */
#define DRIVE_LEVEL7      (0x3fUL)
#define DRIVE_LEVEL6_CLK_DUTY      (0x9bUL)
#define DRIVE_LEVEL6      (0x1bUL)
#define DRIVE_LEVEL5      (0x2dUL)
#define DRIVE_LEVEL4      (0x09UL)
#define DRIVE_LEVEL4_2    (0x36UL)
#define DRIVE_LEVEL3      (0x12UL)
#define DRIVE_LEVEL2      (0x24UL)
#define DRIVE_LEVEL1      (0x0UL)

#if defined(CONFIG_ARCH_RTK6702)
/*
 * Mark2 PAD driving table changed
 * Maybe change setting in future.
 */
#define IO_PAD_SET1_HS400 ((DRIVE_LEVEL4<<26)|(DRIVE_LEVEL4<<17)|(DRIVE_LEVEL4<<8))   //rst/clk/cmd
#define IO_PAD_SET2_HS400 ((DRIVE_LEVEL4<<26)|(DRIVE_LEVEL4<<17)|(DRIVE_LEVEL4<<8))   //ds/d5/d3
#define IO_PAD_SET3_HS400 ((DRIVE_LEVEL4<<26)|(DRIVE_LEVEL4<<17)|(DRIVE_LEVEL4<<8))   //d4/d0/d1
#define IO_PAD_SET4_HS400 ((DRIVE_LEVEL4<<26)|(DRIVE_LEVEL4<<17)|(DRIVE_LEVEL4<<8))   //d2/d7/d6
#define IO_PAD_SET5_HS400 0
#else
#define IO_PAD_SET1_HS400 ((DRIVE_LEVEL4<<26)|(DRIVE_LEVEL4<<17)|(DRIVE_LEVEL4<<8))   //rst/clk/cmd
#define IO_PAD_SET2_HS400 ((DRIVE_LEVEL4<<26)|(DRIVE_LEVEL4<<17)|(DRIVE_LEVEL4<<8))   //ds/d5/d3
#define IO_PAD_SET3_HS400 ((DRIVE_LEVEL4<<26)|(DRIVE_LEVEL4<<17)|(DRIVE_LEVEL4<<8))   //d4/d0/d1
#define IO_PAD_SET4_HS400 ((DRIVE_LEVEL4<<26)|(DRIVE_LEVEL4<<17)|(DRIVE_LEVEL4<<8))   //d2/d7/d6
#define IO_PAD_SET5_HS400 0
#endif

#define IO_PAD_SET1_HS200 IO_PAD_SET1_HS400
#define IO_PAD_SET2_HS200 IO_PAD_SET2_HS400
#define IO_PAD_SET3_HS200 IO_PAD_SET3_HS400
#define IO_PAD_SET4_HS200 IO_PAD_SET4_HS400
#define IO_PAD_SET5_HS200 IO_PAD_SET5_HS400

/*
 with device driving:HS50 : 0x0(x1)
 */
#define IO_PAD_SET1_HS50 ((DRIVE_LEVEL1<<26)|(DRIVE_LEVEL1<<17)|(DRIVE_LEVEL1<<8))
#define IO_PAD_SET2_HS50 ((DRIVE_LEVEL1<<26)|(DRIVE_LEVEL1<<17)|(DRIVE_LEVEL1<<8))
#define IO_PAD_SET3_HS50 ((DRIVE_LEVEL1<<26)|(DRIVE_LEVEL1<<17)|(DRIVE_LEVEL1<<8))
#define IO_PAD_SET4_HS50 ((DRIVE_LEVEL1<<26)|(DRIVE_LEVEL1<<17)|(DRIVE_LEVEL1<<8))
#define IO_PAD_SET5_HS50 ((DRIVE_LEVEL1<<26)|(DRIVE_LEVEL1<<17)|(DRIVE_LEVEL1<<8))

void dw_em_set_IO_driving(struct rtksd_host *sdport, unsigned int type_sel)
{
    //u32 mid = sdport->mid;
    u32 io_pad_setting1;
    u32 io_pad_setting2;
    u32 io_pad_setting3;
    u32 io_pad_setting4;
    u32 w_phase;
    u32 c_phase;

    /*
     Pad config:
     set emmc pull up/pull down/slew rate/smt trigger
    */
    cr_writel(0x91900000,RTK_IOPAD_CFG1);   //rst,clk,cmd,
    cr_writel(0x59999999,RTK_IOPAD_CFG2);   //ds,d5,d3,d4,d0,d1,d2,d7
    cr_writel(0x90000000,RTK_IOPAD_CFG3);   //d6

    if(type_sel == DRIVING_HS400){
        w_phase = DEFAULT_W_PHASE_400;
        c_phase = DEFAULT_C_PHASE_200;
#if defined(CONFIG_ARCH_RTK6702)
        io_pad_setting1 = (IO_PAD_SET1_HS400);
#else
        io_pad_setting1 = (IO_PAD_SET1_HS400 | (0x2UL<<14));  //clk duty cycle +5% : b80108A4[16:14]=010
#endif
        io_pad_setting2 = IO_PAD_SET2_HS400;
        io_pad_setting3 = IO_PAD_SET3_HS400;
        io_pad_setting4 = IO_PAD_SET4_HS400;

        if(rtk_emmc_test_ctl & EMMC_FIX_WPHASE){
            w_phase = GET_FIX_WPHA400(rtk_fix_wphase);
            c_phase = GET_FIX_CPHA200(rtk_fix_wphase);
            EM_ALERT("%s(%d)Bootcode pre-setting HS400 wPhase=%d cPhase=%d \n",
                __func__,__LINE__,w_phase,c_phase);
        }
    }else if(type_sel == DRIVING_HS200){
        w_phase = DEFAULT_W_PHASE_200;
        c_phase = DEFAULT_C_PHASE_200;
        io_pad_setting1 = IO_PAD_SET1_HS200;
        io_pad_setting2 = IO_PAD_SET2_HS200;
        io_pad_setting3 = IO_PAD_SET3_HS200;
        io_pad_setting4 = IO_PAD_SET4_HS200;

        if(rtk_emmc_test_ctl & EMMC_FIX_WPHASE){
            w_phase = GET_FIX_WPHA200(rtk_fix_wphase);
            c_phase = GET_FIX_CPHA200(rtk_fix_wphase);
            EM_ALERT("%s(%d)Bootcode pre-setting HS400 wPhase=%d cPhase=%d \n",
                __func__,__LINE__,w_phase,c_phase);
        }

    }else{
        w_phase = DEFAULT_W_PHASE;
        c_phase = DEFAULT_C_PHASE;
        io_pad_setting1 = IO_PAD_SET1_HS50;
        io_pad_setting2 = IO_PAD_SET2_HS50;
        io_pad_setting3 = IO_PAD_SET3_HS50;
        io_pad_setting4 = IO_PAD_SET4_HS50;

        if((type_sel == CLK_50Mhz) && (fix_phase_flag & FIX_WPHASE50)){
            w_phase = GET_FIX_WPHA50(rtk_fix_50M_phase);;
            EM_ALERT("%s(%d)Bootcode pre-setting HS50 wPhase=%d cPhase=%d \n",
                __func__,__LINE__,w_phase,c_phase);
        }
    }

    if(rtk_emmc_test_ctl & EMMC_EN_IO_DRV){
        u32 *io_drv_ptr;

        EM_ALERT("%s(%d)Use Bootcode pre-setting parameter\n",__func__,__LINE__);

        io_drv_ptr = sdport->mmc_io_drv;

        io_pad_setting1 = io_drv_ptr[0];
        io_pad_setting2 = io_drv_ptr[1];
        io_pad_setting3 = io_drv_ptr[2];
        io_pad_setting4 = io_drv_ptr[3];
    }

    /*
     eMMC Pad driving/duty cycle:
    */
    cr_writel(io_pad_setting1, RTK_IOPAD_SET1);   //rst/clk/cmd
    cr_writel(io_pad_setting2, RTK_IOPAD_SET2);   //ds/d5/d3
    cr_writel(io_pad_setting3, RTK_IOPAD_SET3);   //d4/d0/d1
    cr_writel(io_pad_setting4, RTK_IOPAD_SET4);   //d2/d7/d6

    dw_em_init_phase(sdport,w_phase,0xff,c_phase);

    if(rtk_emmc_test_ctl & EMMC_EN_IO_DRV){
        EM_ALERT("0x%08x=0x%08x 0x%08x=0x%08x 0x%08x=0x%08x 0x%08x=0x%08x\n",
            RTK_IOPAD_SET1,(u32)cr_readl(RTK_IOPAD_SET1),
            RTK_IOPAD_SET2,(u32)cr_readl(RTK_IOPAD_SET2),
            RTK_IOPAD_SET3,(u32)cr_readl(RTK_IOPAD_SET3),
            RTK_IOPAD_SET4,(u32)cr_readl(RTK_IOPAD_SET4));
    }

}

/************************************************************************
 *
 ************************************************************************/
static
void dw_em_crt_reset(struct rtksd_host *sdport,u32 sync)
{
#if defined(RTK_EMMC_33V_SEL)
    u32 volinfo;
    volinfo = 0;    //cr_readl(RTK_EMMC_33V_SEL);
#endif
    /*
      reset all eMMC module.
     */
    if(sync){
        /* crt sync reset */
        cr_writel(RSTN_EM_DW,RTK_EM_DW_SRST);
        cr_writel((CLKEN_EM_DW | 0x01U),RTK_EM_DW_CLKEN);
        udelay(10);
        cr_writel(CLKEN_EM_DW,RTK_EM_DW_CLKEN);
    }else{
        cr_writel(CLKEN_EM_DW,RTK_EM_DW_CLKEN);
        cr_writel(RSTN_EM_DW,RTK_EM_DW_SRST);
        udelay(5);
    }

    cr_writel((RSTN_EM_DW | 0x01UL),RTK_EM_DW_SRST);
    cr_writel((CLKEN_EM_DW | 0x01U),RTK_EM_DW_CLKEN);
#if defined(RTK_EMMC_33V_SEL)
    cr_writel(volinfo,RTK_EMMC_33V_SEL);
#endif
}

/************************************************************************
 *
 ************************************************************************/
static
void dw_em_reset_host(struct rtksd_host *sdport,u8 save_en)
{
    unsigned long flags;
    u32 config1;
    u32 config2;
    u32 config3;
    u32 config4;
    u32 config5;
    u32 config6;
    u32 config7;

    mmcmsg3("\n");
    spin_lock_irqsave(&sdport->lock,flags);
    if(save_en){
        mmcmsg3("\n");
        /* store currently IP setting */
        config1 = cr_readl(EM_DWC_PWREN);
        config2 = cr_readl(EM_DWC_CLKDIV);
        config3 = cr_readl(EM_DWC_CTYPE);
        config4 = cr_readl(EM_DWC_UHS_REG);
        config5 = cr_readl(EM_DWC_DBADDR);
        config6 = cr_readl(EM_DWC_EMMC_DDR_REG);
        config7 = cr_readl(EM_HALF_CYCLE_CAL_EN);
        dw_em_dis_sd_tune(sdport);
    }

    /*
      reset all eMMC module.
     */
    dw_em_crt_reset(sdport,0);

    /* reset synopsys eMMC IP */
    dw_em_IP_config(sdport);

    if(save_en){
        /* restore previously IP setting */
        cr_writel(config1,EM_DWC_PWREN);
        dw_em_set_div(sdport,(config2<<1));
        cr_writel(config3,EM_DWC_CTYPE);
        cr_writel(config4,EM_DWC_UHS_REG);
        cr_writel(config5,EM_DWC_DBADDR);
        cr_writel(config6,EM_DWC_EMMC_DDR_REG);
        if(config7 & BIT(DS_CYCLE_CAL_EN_SHIFT)){
            dw_em_set_ds_delay(sdport, sdport->cur_ds_tune);
        }
    }

    sdport->rtflags |= RTKCR_FHOST_POWER;
    sdport->reset_event = 0;

    spin_unlock_irqrestore(&sdport->lock, flags);
}

/************************************************************************
 *
 ************************************************************************/
static __attribute__((unused))
u32 dw_em_chk_card_busy(void)
{
    if(cr_readl( EM_DWC_STATUS ) & SDMMC_DAT0_BUSY)
        return 1;
    else
        return 0;

}

/************************************************************************
 * usage: force sdport->cur_ds_tune = 0 to make sure 1st tune on ds-tune.
 ************************************************************************/
static __attribute__((unused))
int dw_em_auto_sd_tune(struct rtksd_host *sdport)
{
    u32 ds_tune;
    u32 dis_auto_tune = 0;

    if(rtk_emmc_test_ctl & EMMC_FIX_DSTUNE){
        EM_ALERT("%s(%d)Use Bootcode pre-setting parameter\n",__func__,__LINE__);
        dis_auto_tune = 1;
    }
    ds_tune = sdport->cur_ds_tune;
    mmcmsg4("%s(%d)ds_tune = 0x%08x \n",__func__,__LINE__,ds_tune);

    /*
     enable DS-tune
     Enable of half-cycle calibration
     */
    cr_maskl(EM_HALF_CYCLE_CAL_EN,1,18 ,0);     /* fw_mode is trigged by rising */
    udelay(5);
    cr_maskl(EM_DS_TUNE_CTRL,0x1f,15 ,ds_tune);     /* fill result read from  EMMC_WRAP_half_cycle_cal_result_reg */

#if 0
    /* skip direct settting, use ds_tune offset to tuning */
    if(dis_auto_tune){
        cr_maskl(EM_HALF_CYCLE_CAL_EN,0x1f,3 ,ds_tune);
        cr_maskl(EM_HALF_CYCLE_CAL_EN,3,1 ,3);
        cr_maskl(EM_DS_TUNE_CTRL,1,20 ,1);
        cr_maskl(EM_HALF_CYCLE_CAL_EN,1,0 ,1);
    }else
#endif
    {
        /* pre-setting fine-tune value *** */

        if(dis_auto_tune){
            unsigned int tmp_dstune;
            tmp_dstune = sdport->mmc_dstune;

            if(tmp_dstune & 0x80){
                /* half_cycle_res_mod = 1 */
                EM_ALERT("%s(%d)---\n",__func__,__LINE__);
                cr_maskl(EM_HALF_CYCLE_CAL_EN,3,1 ,1);
            }else{
                /* half_cycle_res_mod = 2 */
                EM_ALERT("%s(%d)+++\n",__func__,__LINE__);
                cr_maskl(EM_HALF_CYCLE_CAL_EN,3,1 ,2);
            }
            tmp_dstune &= 0x1fUL;
            cr_maskl(EM_HALF_CYCLE_CAL_EN,0x1f,3 ,tmp_dstune);
            EM_ALERT("%s(%d)force offset ds_tune = %d\n",__func__,__LINE__,tmp_dstune);
        }else{
#if 0
            /* half_cycle_res_mod = 2; "1=>minus";"2=>plus" */
            if(sdport->mid == 0x15){
                /* set half_cycle_res_thr = -2 for Samsung*/
                cr_maskl(EM_HALF_CYCLE_CAL_EN,3,1 ,1);
                cr_maskl(EM_HALF_CYCLE_CAL_EN,0x1f,3 ,2);
            }else{
                /* set half_cycle_res_thr = +3 */
                cr_maskl(EM_HALF_CYCLE_CAL_EN,3,1 ,2);
                cr_maskl(EM_HALF_CYCLE_CAL_EN,0x1f,3 ,3);
            }
#endif
            cr_maskl(EM_HALF_CYCLE_CAL_EN,3,1 ,3);
            cr_maskl(EM_HALF_CYCLE_CAL_EN,0x1f,3 ,sdport->cur_ds_tune);

        }

        /* enable auto ds tune */
        cr_maskl(EM_DS_TUNE_CTRL,1,20 ,1);  /* bypass SW mode */
        cr_maskl(EM_HALF_CYCLE_CAL_EN,1,18 ,1); /*enable half cycle calibrate mode*/
        cr_maskl(EM_HALF_CYCLE_CAL_EN,1,0 ,1);
    }

    udelay(5);
    cr_maskl(EM_HALF_CYCLE_CAL_EN,1,18 ,0); /* fw_mode is trigged by rising */

    mmcmsg4("0x%08x=0x%08x 0x%08x=0x%08x\n",
        (unsigned int)EM_HALF_CYCLE_CAL_EN,(unsigned int)cr_readl(EM_HALF_CYCLE_CAL_EN),
        (unsigned int)EM_HALF_CYCLE_CAL_RESULT,(unsigned int)cr_readl(EM_HALF_CYCLE_CAL_RESULT));

    return 0;
}

/************************************************************************
 *
 ************************************************************************/
static
int dw_em_dis_sd_tune(struct rtksd_host *sdport)
{
    u32 reginfo;

    reginfo = cr_readl(EM_HALF_CYCLE_CAL_EN);

    if(reginfo & BIT(DS_CYCLE_CAL_EN_SHIFT)){
        mmcmsg4("record ds-tune value");
        /* record ds-tune value */
        sdport->cur_ds_tune = cr_readl(EM_HALF_CYCLE_CAL_RESULT) & DS_CAL_RES_MASK;

        cr_maskl(EM_HALF_CYCLE_CAL_EN, DS_CYCLE_CAL_EN_MASK, DS_CYCLE_CAL_EN_SHIFT ,0);  /* disable DS-tune */

        mmcmsg4("%s(%d)ds_tune=0x%08x\n",
            __func__,__LINE__,sdport->cur_ds_tune);
        mmcmsg4("0x%08x=0x%08x 0x%08x=0x%08x\n",
            (unsigned int)EM_HALF_CYCLE_CAL_EN,(unsigned int)cr_readl(EM_HALF_CYCLE_CAL_EN),
            (unsigned int)EM_HALF_CYCLE_CAL_RESULT,(unsigned int)cr_readl(EM_HALF_CYCLE_CAL_RESULT));
    }else{
        mmcmsg4("skip\n");
    }

    return 0;
}


/************************************************************************
 *
 ************************************************************************/
static
int dw_em_set_DDR400(struct rtksd_host *sdport, u32 ddr_mode)
{
    u32 reginfo;
    u32 ddr_targ;
    unsigned long flags;

    mmcmsg3("ddr_mode=0x%08x\n",ddr_mode);
    /* non-DDR mode, SW 1.8V */
    cr_writel(BIT(0), EM_DWC_UHS_REG);

    reginfo = cr_readl(EM_DWC_EMMC_DDR_REG);
    mmcmsg3("0x%08x = 0x%08x\n",(u32)EM_DWC_EMMC_DDR_REG,reginfo);

    spin_lock_irqsave(&sdport->lock, flags);

    if (ddr_mode) {
        ddr_targ = BIT(31);
    } else {
        ddr_targ = 0;
    }

    if ((reginfo & BIT(31)) != ddr_targ) {
        reginfo = 0;

        if (ddr_targ) {
            reginfo |= ddr_targ;
            EM_NOTICE("HS400 enable\n");
        } else {
            EM_NOTICE("HS400 disable\n");
        }
        EM_NOTICE("set phase 0x%x=0x%08x\n",EMMC_PLLPHASE_CTRL,cr_readl(EMMC_PLLPHASE_CTRL));
        cr_writel(reginfo, EM_DWC_EMMC_DDR_REG);
    }else{
        /* nothing to do */
        mmcmsg3("skip\n");
    }

    spin_unlock_irqrestore(&sdport->lock, flags);

    return 0;

}


/************************************************************************
 *  if reset IP, this reg should be recoard and restore
 ************************************************************************/
static
int dw_em_set_DDR(struct rtksd_host *sdport, u32 ddr_mode)
{
    u32 reginfo;
    u32 ddr_targ;
    unsigned long flags;

    /*
     * If DDR400 enable last time, disable it.
     */
    cr_writel(0, EM_DWC_EMMC_DDR_REG);

    reginfo = cr_readl(EM_DWC_UHS_REG);
    spin_lock_irqsave(&sdport->lock, flags);

    if (ddr_mode) {
        ddr_targ = (BIT(16) | BIT(0));
    } else {
        ddr_targ = 0;
    }

    if ((reginfo & (BIT(16)|BIT(0))) == ddr_targ) {
        /* do nothing */
    } else {
        reginfo &= ~(BIT(16) | BIT(0));

        if (ddr_targ) {
            reginfo |= ddr_targ;
            EM_INFO("DDR enable\n");
        } else {
            EM_INFO("DDR disable\n");
        }
        cr_writel(reginfo, EM_DWC_UHS_REG);
    }

    spin_unlock_irqrestore(&sdport->lock, flags);

    return 0;

}

/************************************************************************
 * if reset IP, this reg should be recoard and restore
 ************************************************************************/
static
int dw_em_set_bits(struct rtksd_host *sdport, u32 bus_width)
{
    u32 reginfo = 0;
    u32 bit_targ;

    unsigned long flags;

    reginfo = cr_readl(EM_DWC_CTYPE);

    spin_lock_irqsave(&sdport->lock, flags);

    if (bus_width == MMC_BUS_WIDTH_8) {
        if (reginfo & BIT(16)) {
            goto skip_out;
        } else {
            EM_NOTICE("%s: 8 bits mode\n",DRIVER_NAME);
            bit_targ = BIT(16);
            sdport->cur_width = 8;
        }
    } else if (bus_width == MMC_BUS_WIDTH_4) {
        if (reginfo & BIT(0)) {
            goto skip_out;
        }else{
            EM_NOTICE("%s: 4 bits mode\n",DRIVER_NAME);
            bit_targ = BIT(0);
            sdport->cur_width = 4;
        }
    }else{
        if (reginfo & (BIT(16) | BIT(0))) {
            EM_NOTICE("%s: 1 bits mode\n",DRIVER_NAME);
            bit_targ = 0;
            sdport->cur_width = 1;
        } else {
            goto skip_out;
        }
    }
    reginfo &= ~(BIT(16) | BIT(0));
    reginfo |= bit_targ;

    cr_writel( reginfo,EM_DWC_CTYPE);

skip_out:

    spin_unlock_irqrestore(&sdport->lock, flags);
    return 0;

}

/************************************************************************
 * reference .\linux-3.10.0\drivers\mmc\host\dw_mmc.c
 * static u32 dw_mci_prepare_command(struct mmc_host *mmc, struct mmc_command *cmd)
 ************************************************************************/
static
u32 dw_em_prepare_command(struct sd_cmd_pkt * cmd_info)
{
    struct mmc_command *cmd = cmd_info->cmd;
    u32 cmd_idx  = 0;
    u32 cmd_flag = 0;
    u32 rsp_typ  = 0;
    u32 cmd_typ  = 0;
    u32 wrt_typ  = 0;

    if(cmd == NULL)
        BUG_ON(1);

    cmd_idx = (u32)(cmd->opcode);
    rsp_typ = (u32)mmc_resp_type(cmd);
    cmd_typ = (u32)mmc_cmd_type(cmd);

    if(cmd->data)
        wrt_typ = (u32)cmd->data->flags;

    cmd_flag = cmd_idx;

    if (cmd_idx == MMC_STOP_TRANSMISSION)
        cmd_flag |= SDMMC_CMD_STOP;
    else{
        cmd_flag |= SDMMC_CMD_PRV_DAT_WAIT;
    }

    /* before idle cmd, send 80 clk plus first */
    if(cmd_idx == MMC_GO_IDLE_STATE)
        cmd_flag |= SDMMC_CMD_INIT;

    /* check resp type */
    if(rsp_typ & MMC_RSP_PRESENT){

        cmd_flag |= (SDMMC_CMD_RESP_EXP);

        if(rsp_typ & MMC_RSP_136)
            cmd_flag |= (SDMMC_CMD_RESP_LONG);

    }

    /*
       Cmd13 skip resp CRC check.
       reference JESD84-B50-1.pdf; 6.6.2 High-speed modes selection
       Because hard to judge what time is proper disable CRC time,
       Skip all time here
     */
    if( (rsp_typ & MMC_RSP_CRC)
     && (cmd_idx != MMC_SEND_STATUS) )
    {
        cmd_flag |= (SDMMC_CMD_RESP_CRC);
    }

    /* check cmd type ADTC? */
    if (cmd_typ == MMC_CMD_ADTC) {
        cmd_flag |= SDMMC_CMD_DAT_EXP;

        /* check data write/read */
        if(wrt_typ & MMC_DATA_WRITE)
            cmd_flag |= SDMMC_CMD_DAT_WR;
    }
#ifdef AUTO_SEND_STOP
    /* auto send stop cmd */
    if((cmd_idx == MMC_READ_MULTIPLE_BLOCK)
    || (cmd_idx == MMC_WRITE_MULTIPLE_BLOCK)) {
        cmd_flag |= SDMMC_CMD_SEND_STOP;
    }
#endif

    cmd_flag |= SDMMC_CMD_USE_HREG;
    cmd_info->cmd_flag = cmd_flag;

    return cmd_flag;
}

/************************************************************************
 * Note : If relative interrupt are disable,
          these value of regiter will be zero.
 ************************************************************************/
void show_int_sta(const char *c_func,
        const int c_line,
        struct rtksd_host *sdport,
        u32 ctl)
{
#ifdef MMC_DEBUG
#define READ_FROM_REG _BIT0
    u32 int_status;
    u32 sd_trans;
    u32 sd_status;
    u32 wrap_sta;

    if(ctl & READ_FROM_REG){
        int_status  = cr_readl(EM_DWC_RINTSTS);
        sd_trans    = cr_readl(EM_DWC_IDSTS);
        sd_status   = cr_readl(EM_DWC_STATUS);
        wrap_sta    = cr_readl(EM_INT_STATUS);
    }else{
        int_status  = sdport->int_status;
        sd_trans    = sdport->sd_trans;
        sd_status   = sdport->sd_status;
        wrap_sta    = sdport->wrap_sta;

    }
    EM_ERR( "%s(%d){\n"
        "    EM_DWC_RINTSTS = 0x%08x EM_DWC_IDSTS   = 0x%08x\n"
        "    EM_DWC_STATUS  = 0x%08x EM_INT_STATUS  = 0x%08x\n"
        "}\n",
        c_func,c_line,
        int_status,sd_trans,
        sd_status,wrap_sta);
#endif
}

/************************************************************************
 *
 ************************************************************************/
#define IDMAC_NORMAL_INT    (SDMMC_IDMAC_INT_NI|SDMMC_IDMAC_INT_RI|SDMMC_IDMAC_INT_TI)
#define IDMAC_INT_ERR       (SDMMC_IDMAC_INT_AI|SDMMC_IDMAC_INT_CES| \
                             SDMMC_IDMAC_INT_DU|SDMMC_IDMAC_INT_FBE)

#define RECORD_STATE_LEN 10
static unsigned int sta_2nd[RECORD_STATE_LEN];
static unsigned int sta_1st[RECORD_STATE_LEN];

static
int dw_em_show_record_sta(unsigned int* buf_1st,unsigned int* buf_2nd);

static
int dw_em_record_sta(
    unsigned int sta0,unsigned int sta1,
    unsigned int sta2,unsigned int sta3,
    unsigned int sta4,unsigned int sta5,
    unsigned int sta6,unsigned int sta7,
    unsigned int sta8,unsigned int sta9)
{
/*
    cmd_idx,
    cmd_arg,
    cmd_flag,
    sdport->int_status = cr_readl(EM_DWC_RINTSTS);
    sdport->sd_status  = cr_readl(EM_DWC_STATUS);
    sdport->sd_trans   = cr_readl(EM_DWC_IDSTS);
    sdport->wrap_sta   = cr_readl(EM_INT_STATUS);
    cr_readl(EM_DWC_BYTCNT),
    timeend
*/
    int i;
    for(i=0;i<RECORD_STATE_LEN;i++){
        sta_2nd[i] = sta_1st[i];
    }

    sta_1st[0]= sta0;
    sta_1st[1]= sta1;
    sta_1st[2]= sta2;
    sta_1st[3]= sta3;
    sta_1st[4]= sta4;
    sta_1st[5]= sta5;
    sta_1st[6]= sta6;
    sta_1st[7]= sta7;
    sta_1st[8]= sta8;
    sta_1st[9]= sta9;

    return 0;
}

static
int dw_em_show_record_sta(unsigned int* buf_1st,unsigned int* buf_2nd)
{
    EM_ALERT("last 2nd:\n"
        "        sta[0]=0x%08x sta[1]=0x%08x sta[2]=0x%08x sta[3]=0x%08x\n"
            "        sta[4]=0x%08x sta[5]=0x%08x sta[6]=0x%08x sta[7]=0x%08x\n"
            "        sta[8]=0x%08x sta[9]=0x%08x\n",
            buf_2nd[0],buf_2nd[1],buf_2nd[2],buf_2nd[3],
            buf_2nd[4],buf_2nd[5],buf_2nd[6],buf_2nd[7],
            buf_2nd[8],buf_2nd[9]);

    EM_ALERT("last 1st:\n"
        "        sta[0]=0x%08x sta[1]=0x%08x sta[2]=0x%08x sta[3]=0x%08x\n"
            "        sta[4]=0x%08x sta[5]=0x%08x sta[6]=0x%08x sta[7]=0x%08x\n"
            "        sta[8]=0x%08x sta[9]=0x%08x\n",
            buf_1st[0],buf_1st[1],buf_1st[2],buf_1st[3],
            buf_1st[4],buf_1st[5],buf_1st[6],buf_1st[7],
            buf_2nd[8],buf_2nd[9]);

    return 0;
}

static
int rtkem_debug_info(struct sd_cmd_pkt * cmd_info)
{
#if 0
    u32 *des_base;
    u32 i;

    EM_ALERT("\n****** BUS Status Info ******\n");

    EM_ALERT("======DC_SYS FIFO Status======\n"
         "    0x%08x=0x%08x 0x%08x=0x%08x\n",
            (u32)0xb8007064,(u32)cr_readl(0xb8007064),
            (u32)0xb8003064,(u32)cr_readl(0xb8003064));
    EM_ALERT("======IB FIFO Status======\n"
         "    0x%08x=0x%08x 0x%08x=0x%08x 0x%08x=0x%08x 0x%08x=0x%08x\n"
         "    0x%08x=0x%08x 0x%08x=0x%08x 0x%08x=0x%08x 0x%08x=0x%08x\n"
         "    0x%08x=0x%08x 0x%08x=0x%08x 0x%08x=0x%08x 0x%08x=0x%08x\n"
         "    0x%08x=0x%08x 0x%08x=0x%08x\n",
            (u32)0xb803F0F0,(u32)cr_readl(0xb803F0F0),
            (u32)0xb803F1F0,(u32)cr_readl(0xb803F1F0),
            (u32)0xb803F2F0,(u32)cr_readl(0xb803F2F0),
            (u32)0xb803F3F0,(u32)cr_readl(0xb803F3F0),
            (u32)0xb803F4F0,(u32)cr_readl(0xb803F4F0),
            (u32)0xb803F5F0,(u32)cr_readl(0xb803F5F0),
            (u32)0xb803F6F0,(u32)cr_readl(0xb803F6F0),
            (u32)0xb803F7F0,(u32)cr_readl(0xb803F7F0),
            (u32)0xb803F8F0,(u32)cr_readl(0xb803F8F0),
            (u32)0xb803F9F0,(u32)cr_readl(0xb803F9F0),
            (u32)0xb803FAF0,(u32)cr_readl(0xb803FAF0),
            (u32)0xb803FBF0,(u32)cr_readl(0xb803FBF0),
            (u32)0xb803FCF0,(u32)cr_readl(0xb803FCF0),
            (u32)0xb803FCF0,(u32)cr_readl(0xb803FCF0));
    EM_ALERT("\n****** WRAPPER setting ******\n"
         "    0x%08x=0x%08x 0x%08x=0x%08x\n",
            (u32)0xb8007008,(u32)cr_readl(0xb8007008),
            (u32)0xb80070c8,(u32)cr_readl(0xb80070c8));
    EM_ALERT("======eMMC descripter======\n");
    des_base = cmd_info->sdport->scrip_buf;
    for(i=0;i<10;i++){
        EM_ALERT("{Line%02u}[des0]=0x%08x [des1]==0x%08x [des2]==0x%08x [des3]==0x%08x\n",
            i,des_base[(i*4)+0],des_base[(i*4)+1],des_base[(i*4)+2],des_base[(i*4)+3]);
    }

    EM_ALERT("\n&&&&&& BUS Status Info &&&&&&\n");
#endif
    return 0;
}

/************************************************************************
 *
 ************************************************************************/
#define GET_DBUS_SMA(value) ((value>>16)&0x07)
#define GET_AHB_SMA(value)  (value & 0x7FFF)
static
int dw_em_TriggerXferCmd(u32 cmd_flag,struct sd_cmd_pkt * cmd_info)
{
    struct rtksd_host *sdport = cmd_info->sdport;
    int ret_err = 0;
    u32 cmd_idx = 0;
    unsigned long timeend = 0;
    unsigned long flags;
    u32 cmd_arg = 0;
    u32 warp_sta1 = 0;
    u8 int_mode = 0;
    __maybe_unused u64 time_stamp_1, time_stamp_2;
    __maybe_unused u64 cmd_time_ns_diff;

    DECLARE_COMPLETION(rtk_wait);
    sdport->task_state = 0;
    if(sdport->tmout){
        /*
         * Dont reset sdport->tmout
         * Not all command can be shared with the same timeout val
         * CMD6 timeout val is depended on MMC device
         */
        timeend = msecs_to_jiffies(sdport->tmout);
        // sdport->tmout = 0;
    }else{
        WARN_ON(1);
        timeend = msecs_to_jiffies(100);
    }

    int_collect = 0;

    sdport->int_status  = 0;
    sdport->sd_trans    = 0;
    sdport->sd_status   = 0;
    sdport->wrap_sta    = 0;

    spin_lock_irqsave(&sdport->lock,flags);

    /* Here, we use cmd_flag to get information for flow sync reason.  */
    cmd_idx = SDMMC_CMD_INDX(cmd_flag);
    cmd_arg = (u32)(cmd_info->cmd->arg);

    int_mode = sdport->int_mode;

    /* Start IDMAC running */
    cr_writel(1, EM_DWC_PLDMND);

    sdport->int_waiting = &rtk_wait;
    sdport->task_state = STATE_SENDING_DATA;
    if(int_mode){
        /* clear pending bit */
        cr_writel(0xffffffff, EM_DWC_IDSTS);
        cr_writel( 0xffffffff,EM_DWC_RINTSTS);
        cr_writel( BIT(2) | BIT(1) | BIT(0),EM_INT_STATUS);
        /*  Do nothing here */
        if (rtk_emmc_debug_log){
            if ((cmd_idx == 24 || cmd_idx == 25))
                EM_ERR("begin:previous cmd = %u, current RINTSTS=0x%x before send write command\n", pre_cmd, cr_readl(EM_DWC_RINTSTS));
        }

        if (device_suspend)
            EM_INFO( "%s: resume fail\n",DRIVER_NAME);
    } else {
        /* disable and clear int */
        dw_em_clear_int_sta(1);
        dw_em_clear_int_sta_IDMA(1);
        dw_em_clear_int_wrap(1);
        tasklet_schedule(&sdport->polling_tasklet);
    }

    if(cmd_arg==0x03a50101 ){
        /* if sanitize operation, waitting until finish. */
    }else{
        if(sdport->rtflags & RTKCR_TUNING_STATE){
            /* need to check Nac value */
            timeend = msecs_to_jiffies(150);
        }

        stop_timer = 0;
        done_timer = 0;
        mod_timer(&sdport->timer, (jiffies + timeend) );
    }
    smp_wmb();

#if 0
    if(cr_readl(EM_DWC_RINTSTS)){
        EM_ERR("cmd_idx=%02d cmd_flag=0x%08x\n",
               cmd_idx,cmd_flag);

        EM_ERR("0x%08x=0x%08x 0x%08x=0x%08x 0x%08x=0x%08x 0x%08x=0x%08x \n",
               (u32)EM_DWC_RINTSTS,cr_readl(EM_DWC_RINTSTS),
               (u32)EM_DWC_IDSTS,cr_readl(EM_DWC_IDSTS),
               (u32)EM_DWC_STATUS,cr_readl(EM_DWC_STATUS),
               (u32)EM_INT_STATUS,cr_readl(EM_INT_STATUS));
    }
#endif
    cr_writel(cmd_flag | SDMMC_CMD_START, EM_DWC_CMD);

    spin_unlock_irqrestore(&sdport->lock, flags);

    if (sdport->rtflags & RTKCR_FTIMER_CHECK){
        time_stamp_1 = ktime_get_real_ns();
    }
    wait_for_completion(&rtk_wait);

    // if done_timer == 0, timeout event triggered before tasklet completed
    if (timer_pending(&sdport->timer) || !done_timer)
        del_timer_sync(&sdport->timer);

    if (!(sdport->int_status & SDMMC_INT_ERROR)
     && !(sdport->sd_trans & IDMAC_INT_ERR))
    {
        u32 fsm = (sdport->sd_trans & (0xf<<13))>>13;
        if(fsm){
            mmcmsg2("FSM not idel(%u)",fsm);
        }
    }

    pre_cmd = cmd_idx;
    ret_err = cmd_info->cmd->error;

    if (sdport->rtflags & RTKCR_FTIMER_CHECK){
//#define SHOW_TIME_DIFF_INLINE
        time_stamp_2 = ktime_get_real_ns();
        if(time_stamp_2 > time_stamp_1 ){
            cmd_time_ns_diff = time_stamp_2-time_stamp_1;
        }else{
            cmd_time_ns_diff = (0xffffffffffffffff+time_stamp_2)-time_stamp_1;
        }
        if(cmd_time_ns_diff < 10000000000 ){
            if(cmd_time_ns_diff > atomic64_read( &cmd_time_ns_old)){
                atomic64_set( &cmd_time_ns_old,cmd_time_ns_diff);
                cmd_record = cmd_idx ;
                total_byte_record = (u32)(cmd_info->total_byte_cnt);
#ifdef SHOW_TIME_DIFF_INLINE
                EM_ALERT("update max timeout %llu (%llu-%llu)!\n",cmd_time_ns_diff,time_stamp_2,time_stamp_1);
#endif
            }
        }else{
#ifdef SHOW_TIME_DIFF_INLINE
            EM_ALERT("update max timeout %llu (%llu-%llu) big number!!!\n",cmd_time_ns_diff,time_stamp_2,time_stamp_1);
#endif
        }
    }

    if( ret_err) {
        rtk_emmc_err_cnt++;
        warp_sta1 = cr_readl(EM_WARP_STATUS1);
        if(sdport->rtflags & RTKCR_TUNING_STATE ) {
#if 0
            /* skip log display while tuning flow  */
            EM_ERR( "%s: Tuning State{mid=0x%x}\n"
                "    cmd_idx=%d RINTSTS=0x%08x\n",
                DRIVER_NAME, sdport->mid,
                cmd_idx, sdport->int_status);
#endif
        } else {
            sdport->mmc->can_retune = 1;
            if(rtk_emmc_test_ctl & EMMC_RCV_FLOW){
                dw_em_show_record_sta(sta_1st,sta_2nd);
            }
            if (cmd_flag & SDMMC_CMD_DAT_EXP)
                sdport->rtflags |= RTKCR_RECOVER_STATE;

            if(rtk_emmc_test_ctl & EMMC_EN_SCAN_TOOL){
                EM_EMERG( "%s: Recover State{mid=0x%x}\n"
                    "    cmd_idx=%d cmd_arg=0x%08x cmd_flag=0x%08x loops=0x%x\n"
                    "        RINTSTS=0x%08x  STATUS=0x%08x\n"
                    "        IDSTS=0x%08x(0x%08x)%s\n"
                    "        INT_STATUS=0x%08x  EM_DWC_BYTCNT=0x%08x\n"
                    "        WARP_STATUS0=0x%08x  WARP_STATUS1=0x%08x\n",
                    DRIVER_NAME, sdport->mid,
                    cmd_idx, cmd_arg, cmd_flag, cmd_info->timeout,
                    sdport->int_status,sdport->sd_status,
                    sdport->sd_trans,cr_readl(EM_DWC_STATUS),
                    (sdport->sd_trans & (0x1f << 13))?"[DMAC not Idle!!!]":"",
                    sdport->wrap_sta,cr_readl(EM_DWC_BYTCNT),
                    cr_readl(EM_WARP_STATUS0),warp_sta1);
            }else{
                EM_ERR( "%s: Recover State{mid=0x%x}\n"
                    "    cmd_idx=%d cmd_arg=0x%08x cmd_flag=0x%08x loops=0x%x\n"
                    "        RINTSTS=0x%08x  STATUS=0x%08x\n"
                    "        IDSTS=0x%08x(0x%08x)%s\n"
                    "        INT_STATUS=0x%08x  EM_DWC_BYTCNT=0x%08x\n"
                    "        WARP_STATUS0=0x%08x  WARP_STATUS1=0x%08x\n",
                    DRIVER_NAME, sdport->mid,
                    cmd_idx, cmd_arg, cmd_flag, cmd_info->timeout,
                    sdport->int_status,sdport->sd_status,
                    sdport->sd_trans,cr_readl(EM_DWC_STATUS),
                    (sdport->sd_trans & (0x1f << 13))?"[DMAC not Idle!!!]":"",
                    sdport->wrap_sta,cr_readl(EM_DWC_BYTCNT),
                    cr_readl(EM_WARP_STATUS0),warp_sta1);
                EM_ERR( "total byte count=0x%x waiting=%dms(%ld)\n",cr_readl(EM_DWC_BYTCNT),sdport->tmout,timeend);
                if (sdport->rtflags & RTKCR_FTIMER_CHECK){
                    EM_ERR( "timeout %llu (%llu-%llu)!\n",cmd_time_ns_diff,time_stamp_2,time_stamp_1);
                }
                dw_em_show_phase();
#ifdef RTK_RW_DOWNSPEED
                /* If bit 7 is set, maybe some signal issue occurs */
                if ((sdport->mmc->caps2 & MMC_CAP2_HS400_1_8V) && (sdport->int_status & 0x0080)){
                    if ( (cmd_idx == 25) || (cmd_idx == 24) ){
                        if (pre_wcmd_arg == cmd_arg)
                            pre_cmd_write_count++;
                        else
                            pre_cmd_write_count = 1;
                        // record write fail info
                        pre_wcmd_arg = cmd_arg;
                    }
                    if ( (cmd_idx == 18) || (cmd_idx == 17) ){
                        if (pre_rcmd_arg == cmd_arg)
                            pre_cmd_read_count++;
                        else
                            pre_cmd_read_count = 1;
                        // record read fail info
                        pre_rcmd_arg = cmd_arg;
                    }
                    if (!HS400_fix_test && ((pre_cmd_write_count>2) || (pre_cmd_read_count>2)) ){
                        EM_INFO( "%s: downspeed HS200 to prevent write bad data\n", DRIVER_NAME);
                        sdport->mmc->caps2 &= ~(MMC_CAP2_HS400_1_8V);
                        sdport->mmc->card->mmc_avail_type &= ~EXT_CSD_CARD_TYPE_HS400;
                        /* Enabel retune */
                    }
                }
#endif
            }
            mmcmsg3("0x%08x=0x%08x 0x%08x=0x%08x\n",
                (u32)EM_HALF_CYCLE_CAL_EN,(u32)cr_readl(EM_HALF_CYCLE_CAL_EN),
                (u32)EM_HALF_CYCLE_CAL_RESULT,(u32)cr_readl(EM_HALF_CYCLE_CAL_RESULT));

            rtkem_debug_info(cmd_info);

            /* if error occur at non-tuning flow, enable tuning folw */
            sdport->ins_event &= ~EVENT_SKIP_DSTUNING;
            sdport->ins_event &= ~EVENT_SKIP_PHASETUNE;
            sdport->hs400_tuned = false;
            sdport->hs200_tuned = false;
            sdport->hs50_tuned  = false;
        }

        if((cmd_flag & SDMMC_CMD_DAT_EXP)
         &&((GET_DBUS_SMA(warp_sta1)!=1) || (GET_AHB_SMA(warp_sta1)!=1)))
        {
            /* if wrap state machine not idle, reset wrap */
            if (sdport->int_mode)
                rtkemmc_disable_int(sdport);
            dw_em_reset_host(sdport,1);
            if (sdport->int_mode)
                rtkemmc_enable_int(sdport);
        }else{
            if (sdport->int_mode)
                rtkemmc_disable_int(sdport);
            dw_em_reset_host_IP(sdport,1);
            if (sdport->int_mode)
                rtkemmc_enable_int(sdport);
        }

    }else{
        if(sdport->rtflags & RTKCR_TUNING_STATE){
            if (sdport->int_mode)
                rtkemmc_disable_int(sdport);
            dw_em_reset_host_IP(sdport,1);
            if (sdport->int_mode)
                rtkemmc_enable_int(sdport);
        }

        if(cmd_flag & SDMMC_CMD_DAT_EXP){
            sdport->mmc->can_retune = 0;
            if(sdport->rtflags & RTKCR_RECOVER_STATE){
                sdport->rtflags &= ~RTKCR_RECOVER_STATE;
                EM_ALERT("recover success!\n");
            }
        }
    }
    dw_em_record_sta(cmd_idx,cmd_arg,cmd_flag,sdport->int_status,
            sdport->sd_status,sdport->sd_trans,
            sdport->wrap_sta,cr_readl(EM_DWC_BYTCNT),
            (u32)timeend,0);

    if (rtk_emmc_debug_log){
        if (cmd_idx == 24 || cmd_idx == 25){
            EM_ERR("end:Intterrupt flag=0x%x, RINTSTS=0x%x after write command\n", int_collect, cr_readl(EM_DWC_RINTSTS));
        }
    }

    return ret_err;
}


/************************************************************************
 *
 ************************************************************************/
static
int dw_em_SendCmd(struct sd_cmd_pkt *cmd_info)
{
    struct rtksd_host *sdport   = cmd_info->sdport;
    struct mmc_command *cmd     = cmd_info->cmd;
    u32 cmd_idx                 = (u32)(cmd->opcode);
    u32 cmd_arg = (u32) (cmd->arg);
    u32 *rsp                    = cmd->resp;
    u32 cmd_flag = 0;
    unsigned long flags;

    int ret_err = 0;
    int rsp_err = 0;

    if(rsp == NULL) {
        BUG_ON(1);
    }

    if(dw_em_get_occupy(sdport)){
        return cmd_info->cmd->error = -EIO;
    }

    dw_em_get_cmd_timeout(cmd_info);
    cmd_flag = dw_em_prepare_command(cmd_info);

    /* reduce log message when write operation in retry*/
    if( (sdport->rtflags & RTKCR_FOPEN_LOG) )
    {
        EM_ALERT("cmd_idx=%02d cmd_arg=0x%08x cmd_flag=0x%08x retries=%u\n",
               cmd_idx, cmd_arg, cmd_flag, cmd->retries);
        EM_ALERT("        ds_tune=%u\n",
            (cr_readl(EM_HALF_CYCLE_CAL_RESULT) & 0x1f));
    }

    if( sdport->log_state == 1 )
    {
        if (cmd_idx == 35)
            EM_WARNING("[EMMC ERASE]: start addr: 0x%x\n", cmd_arg);
        else if (cmd_idx == 36)
            EM_WARNING("[EMMC ERASE]: end addr: 0x%x\n", cmd_arg);
        else if (cmd_idx == 6)
        {
            if ((cmd_arg & 0xffffff00) == 0x3200100)
                EM_WARNING("[EMMC FLUSH]: 0x%x\n", cmd_arg);
        }
    }

    if( (cmd_idx == 6)
     && (rtk_emmc_test_ctl & EMMC_EN_MMC_DRV) ){
        u32 tmp_mmc_drv = sdport->mmc_drv;
        u32 chg_alert = 0;

        if((cmd_arg & 0xffff0fff) == 0x03b90301 ){
            /* HS400 mode */
            tmp_mmc_drv = (tmp_mmc_drv & 0x00ff0000)>>16;
            if(tmp_mmc_drv <= 4)
                chg_alert = 1;
        }else if((cmd_arg & 0xffff0fff) == 0x03b90201 ){
            /* HS200 mode */
            tmp_mmc_drv = (tmp_mmc_drv & 0x0000ff00)>>8;
            if(tmp_mmc_drv <= 4)
                chg_alert = 1;
        }
        if(chg_alert){
            EM_NOTICE("%s(%d)Use Bootcode pre-setting parameter\n",
                __func__,__LINE__);
            EM_NOTICE("%s(%d)arg=0x%08x mmc_drv=0x%08x\n",
                __func__,__LINE__,cmd_arg,tmp_mmc_drv);
            cmd_arg &= 0xffff0fff;
            cmd_arg |= (tmp_mmc_drv<<12);
            EM_NOTICE("%s(%d)arg=0x%08x mmc_drv=0x%08x\n",
                __func__,__LINE__,cmd_arg,tmp_mmc_drv);
        }
    }

    spin_lock_irqsave(&sdport->lock,flags);

    cr_writel(cmd_arg,  EM_DWC_CMDARG);
    cr_writel(0,        EM_DWC_BLKSIZ);
    cr_writel(0,        EM_DWC_BYTCNT);

    spin_unlock_irqrestore(&sdport->lock, flags);

    ret_err = dw_em_TriggerXferCmd(cmd_flag,cmd_info);

    rsp_err = dw_em_chk_rsp(rsp, cmd_flag, cmd_info);

    dw_em_release_occupy(sdport);
    if(rsp_err){
#ifdef SHOW_RESPONSE
        EM_INFO("response error alert. check it!!!\n");
#endif
    }

    if( ret_err ){
        EM_INFO( "%s: %s fail\n", DRIVER_NAME, __func__);
        if (rtk_emmc_debug_log){
            EM_ERR("rsp[0]=0x%08x\n", cr_readl(EM_DWC_RESP0));
            EM_ERR("rsp[1]=0x%08x\n", cr_readl(EM_DWC_RESP1));
            EM_ERR("rsp[2]=0x%08x\n", cr_readl(EM_DWC_RESP2));
            EM_ERR("rsp[3]=0x%08x\n", cr_readl(EM_DWC_RESP3));
        }
    } else {
        /* TO get manufacturer ID here */
        if(cmd_idx == MMC_ALL_SEND_CID ){
            sdport->mid = ((rsp[0]>>24) & 0xffU);
        }
    }

    if(cmd_idx == 23 && !rsp_err && !ret_err)
        sdport->rtflags |= RTKCR_CMD23_ALERT;

    return ret_err;
}

/************************************************************************
 *
 ************************************************************************/
static
int dw_em_Stream_Cmd(struct sd_cmd_pkt * cmd_info)
{
    struct rtksd_host *sdport = cmd_info->sdport;
    struct mmc_command *cmd    = cmd_info->cmd;
    u32  cmd_idx        = (u32)(cmd->opcode);
    u32  cmd_arg        = (u32)(cmd->arg);
    u32 *rsp            = cmd->resp;
    u32  byte_count     = (u32)(cmd_info->byte_count);
    u32  total_byte_cnt = (u32)(cmd_info->total_byte_cnt);
    u32  cmd_flag       = 0;
    unsigned long flags;

    int ret_err = 0;
    int rsp_err = 0;

    if(rsp == NULL) {
        BUG_ON(1);
    }

    dw_em_get_cmd_timeout(cmd_info);

    cmd_flag = dw_em_prepare_command(cmd_info);

    if(sdport->rtflags & RTKCR_CMD23_ALERT){
        sdport->rtflags &= ~RTKCR_CMD23_ALERT;
        cmd_flag &= ~SDMMC_CMD_SEND_STOP;
    }

    if( (sdport->rtflags & RTKCR_FOPEN_LOG)
     || (sdport->rtflags & RTKCR_RECOVER_STATE))
    {
        EM_ALERT("cmd_idx=%02d cmd_arg=0x%08x cmd_flag=0x%08x retries=%u\n"
            "            blk_siz=0x%08x total_byte_cnt=0x%08x\n",
            cmd_idx, cmd_arg, cmd_flag, cmd->retries,
            (u32) byte_count, (u32) total_byte_cnt);
        EM_ALERT("        ds_tune=%u\n",
            (cr_readl(EM_HALF_CYCLE_CAL_RESULT) & 0x1f));
    }

    if( (sdport->log_state == 1)
     && ( (cmd_idx == 18) || (cmd_idx == 25)
       || (cmd_idx == 17) || (cmd_idx == 24) ) )
    {
        EM_WARNING("[EMMC %s]: addr: 0x%x; size: 0x%x; {cmd%u}\n",
            ((cmd_idx == 18) || (cmd_idx == 17))? " Read" : "Write",
            cmd_arg, total_byte_cnt,cmd_idx);
    }

    spin_lock_irqsave(&sdport->lock,flags);

    cr_writel(cmd_arg,          EM_DWC_CMDARG);
    cr_writel(byte_count,       EM_DWC_BLKSIZ);
    cr_writel(total_byte_cnt,   EM_DWC_BYTCNT);

    spin_unlock_irqrestore(&sdport->lock, flags);

#ifdef MMC_ROOTFS_CHECK
    /* got rootfs information */
    if(!partition_info_ready){
        rootfs_part_num = lgemmc_get_partnum("rootfs");
        if(rootfs_part_num < 0){
            /* wait partition information ready */
            //EM_ALERT("%s(%d)partition not ready\n",__func__,__LINE__);
        }else{
            partition_info_ready = 1;

            rootfs_partinfo.offset = lgemmc_get_partoffset(rootfs_part_num);
            rootfs_partinfo.size   = lgemmc_get_partsize(rootfs_part_num);
            rootfs_partinfo.ptend  = (rootfs_partinfo.offset+rootfs_partinfo.size);
            EM_ALERT("%s(%d)partition%d [%s] offset:0x%llx size:0x%llx end:0x%llx\n",
                __func__,__LINE__,
                rootfs_part_num,lgemmc_get_partname(rootfs_part_num),
                rootfs_partinfo.offset,rootfs_partinfo.size,rootfs_partinfo.ptend);

            rootfs_partinfo.offset = (rootfs_partinfo.offset>>9);
            rootfs_partinfo.size   = (rootfs_partinfo.size>>9);
            rootfs_partinfo.ptend  = (rootfs_partinfo.ptend>>9);
            EM_ALERT("%s(%d)partition%d [%s] start_blk:0x%llx size_blk:0x%llx end_blk:0x%llx\n",
                __func__,__LINE__,
                rootfs_part_num,lgemmc_get_partname(rootfs_part_num),
                rootfs_partinfo.offset,rootfs_partinfo.size,rootfs_partinfo.ptend);
            if(rootfs_partinfo.size == 0){
                partition_info_ready = 0xff;
                EM_ALERT("bad partition information skip monitor\n");
            }else{
                EM_ALERT("Start rootfs monitor!!!\n");
            }
        }
    }

    if((partition_info_ready) && (partition_info_ready <10)){
        /* call BUG_ON() if write command target address of mmc block is locate at rootfs */
        if((cmd_idx == 25) || (cmd_idx == 24)){
            u32 addr_start = cmd_arg;
            u32 addr_end   = cmd_arg+(total_byte_cnt/512);

            if(((addr_start >= (u32)rootfs_partinfo.offset) && (addr_start < (u32)rootfs_partinfo.ptend))
             ||((addr_end >= (u32)rootfs_partinfo.offset) && (addr_end < (u32)rootfs_partinfo.ptend)))
            {
                EM_ALERT("%s(%d)cmd_idx=%02d cmd_arg=0x%08x-0x%08x\n",
                        __func__,__LINE__,cmd_idx, addr_start,addr_end);
                EM_ALERT("%s(%d)partition%d [%s] start_blk:0x%llx size_blk:0x%llx end_blk:0x%llx\n",
                    __func__,__LINE__,
                    rootfs_part_num,lgemmc_get_partname(rootfs_part_num),
                    rootfs_partinfo.offset,rootfs_partinfo.size,rootfs_partinfo.ptend);
                partition_info_ready++;
                WARN_ON(1);
            }
        }
    }

#endif

    ret_err = dw_em_TriggerXferCmd(cmd_flag,cmd_info);

    rsp_err = dw_em_chk_rsp(rsp, cmd_flag, cmd_info);

    if(rsp_err){
        if(cmd_flag & SDMMC_CMD_SEND_STOP){
            /* It's needed to handle error of resp at case to enable auto cmd12  */
            if(rsp_err & BIT(1))
                ret_err |= rsp_err;
        }
#ifdef SHOW_RESPONSE
        if(!(sdport->rtflags & RTKCR_TUNING_STATE))
            EM_INFO("response error alert. check it!!!\n");
#endif
    }

    if( ret_err ){
        if(!(sdport->rtflags & RTKCR_TUNING_STATE)){
            EM_INFO( "%s: %s fail\n", DRIVER_NAME, __func__);
            if (rtk_emmc_debug_log){
                EM_ERR("rsp[0]=0x%08x\n", cr_readl(EM_DWC_RESP0));
                EM_ERR("rsp[1]=0x%08x\n", cr_readl(EM_DWC_RESP1));
                EM_ERR("rsp[2]=0x%08x\n", cr_readl(EM_DWC_RESP2));
                EM_ERR("rsp[3]=0x%08x\n", cr_readl(EM_DWC_RESP3));
            }
        }
    }

    return ret_err;

}

/************************************************************************
 *
 ************************************************************************/
static
void rtk_op_complete(struct rtksd_host *sdport)
{

    if (sdport->int_waiting) {
        struct completion *waiting = sdport->int_waiting;
        sdport->int_waiting = NULL;
        complete(waiting);
    }
}

/************************************************************************
 *
 ************************************************************************/
static
void rtkem_hold_card(struct rtksd_host *sdport)
{
    /* 296x_plus eMMC pin are dedicate.
       It can't hold pin. */
    return;
}

/************************************************************************
 *
 ************************************************************************/
static
void dw_em_bus_speed_down(struct rtksd_host *sdport)
{
    u32 tmp_clk;
    if (sdport->int_mode)
        rtkemmc_disable_int(sdport);
    if(sdport->cur_clock > CLK_100Mhz){
        tmp_clk = (sdport->cur_clock) - CLK_20Mhz;
    } else if (sdport->cur_clock > CLK_50Mhz) {
        tmp_clk = CLK_50Mhz;
    } else {
        tmp_clk = sdport->cur_clock - CLK_10Mhz;
    }
    sdport->ops->set_clk(sdport,tmp_clk);
    if (sdport->int_mode)
        rtkemmc_enable_int(sdport);
}

/************************************************************************
 *
 ************************************************************************/
static
u32 dw_em_get_cmd_timeout(struct sd_cmd_pkt *cmd_info)
{
    struct rtksd_host *sdport   = cmd_info->sdport;
    struct mmc_command* cmd     = cmd_info->cmd;
    u16 block_count             = cmd_info->block_count;
    u32 tmout = 0;

    /*
     * Dont do conversion too early
     * We want to know real value user want to set.
     */
    if(cmd_info->cmd->data){
        if(block_count == 1)
            tmout = 500;
        else
            tmout = 1000+ (block_count/2);
    }else{
        if (mmc_resp_type(cmd) & MMC_RSP_BUSY) {
            /*
             if response type is R1b,
             Host should wait long time for card busy.
             */
            tmout = (cmd->busy_timeout)? cmd->busy_timeout:1000;
        } else {
            tmout = (200);
        }
    }

    cmd_info->timeout = sdport->tmout = tmout;
    return 0;
}

/************************************************************************
 *  Reference:  linux-3.10.0\drivers\mmc\host\dw_mmc.c
 *              static void dw_mci_translate_sglist(struct dw_mci *host, struct mmc_data *data,
 *  The byte_ctl is used to judege if data small then 1bolck(512 bytes).
 *  if so, It will set end flag at first loop.
 ************************************************************************/
#define SG_1ST_FLAG (0x01UL<<0)
#define SG_END_FLAG (0x01UL<<1)
#define SG_BYT_FLAG (0x01UL<<2)
#define SHOW_SCRIPT_DATA        0
u32 *dw_em_build_script(u32 *des_base,
    dma_addr_t  dma_addr,
    u32 blk_cnt, u32 *scrip_cnt, u32 ctl,
    struct sd_cmd_pkt *cmd_info)
{
    u32 *des_base2;
    dma_addr_t dma_addr2;
    dma_addr_t scrip_buf_phy;
    u32  blk_cnt2;
    u32  remain_blk_cnt;
    u32 tmp_val;
    *scrip_cnt       = 0;

    des_base2       = des_base;
    dma_addr2       = dma_addr;
    remain_blk_cnt  = blk_cnt;

    scrip_buf_phy = cmd_info->sdport->scrip_buf_phy;

    mmcmsg1("des_base2=0x%p dma_addr2=0x%08x remain_blk_cnt=0x%08x\n",
            des_base2,dma_addr2,remain_blk_cnt);

    while(remain_blk_cnt){

#if SHOW_SCRIPT_DATA
        mmcmsg1("des_base2=0x%08x", (u32)des_base2);
#endif
        /* setting des0; transfer parameter  */

        /*
                 To enable IDMAC interrupt:
         reference to Spec 3.2.3Descriptors.
         Table 3-3 Bits in IDMAC DES0 Element (32-bit Address Configuration)
         meantion about DES0 bit1
         */
        tmp_val = IDMAC_DES0_OWN | IDMAC_DES0_DIC | IDMAC_DES0_CH;
        if(ctl & SG_1ST_FLAG){
            if(remain_blk_cnt == blk_cnt)
                tmp_val |= IDMAC_DES0_FD;
            }

        if(ctl & SG_END_FLAG){
            if(remain_blk_cnt <= EMMC_MAX_SCRIPT_BLK){
                tmp_val |= IDMAC_DES0_LD;
                tmp_val &= ~IDMAC_DES0_DIC;
            }
        }

        des_base2[0] = tmp_val;

        /* setting des1; buffer size in byte */
        if(remain_blk_cnt > EMMC_MAX_SCRIPT_BLK){
            blk_cnt2 = EMMC_MAX_SCRIPT_BLK;
        }else{
            blk_cnt2 = remain_blk_cnt;
        }
        des_base2[1] = (blk_cnt2<<9);

        /* setting des2; Physical address to DMA to/from */
        des_base2[2] = (dma_addr2);

        /* setting des3; next descrpter entry */
        des_base2[3] = scrip_buf_phy+(((cmd_info->scrip_cnt)+(*scrip_cnt)+1)*16);

        if(ctl & SG_BYT_FLAG){
            des_base2[0] |= IDMAC_DES0_LD;
            des_base2[1]  = blk_cnt;
            remain_blk_cnt = 0;
#if SHOW_SCRIPT_DATA
            /*
              the value of des_base[x] and des_base2[x] is different.
              because point des_base2 is increase 4 per loop
            */
            mmcmsg1("des0=0x%08x\n", des_base2[0]);
            mmcmsg1("des1=0x%08x\n", des_base2[1]);
            mmcmsg1("des2=0x%08x\n", des_base2[2]);
            mmcmsg1("des3=0x%08x\n", des_base2[3]);
#endif
        }else{
#if SHOW_SCRIPT_DATA
            /*
              the value of des_base[x] and des_base2[x] is different.
              because point des_base2 is increase 4 per loop
              If want to see value of des_base2[x], It must be here,
              because  des_base2 will be increased later.
            */
            mmcmsg1("des0=0x%08x\n", des_base2[0]);
            mmcmsg1("des1=0x%08x\n", des_base2[1]);
            mmcmsg1("des2=0x%08x\n", des_base2[2]);
            mmcmsg1("des3=0x%08x\n", des_base2[3]);
#endif
            dma_addr2 = dma_addr2+(blk_cnt2<<9);
            remain_blk_cnt -= blk_cnt2;
            des_base2 += 4;
        }
        *scrip_cnt += 1;
    }

    mmcmsg1("scrip_cnt=%u des_base2=0x%p\n", *scrip_cnt, des_base2);

    return des_base2;
}

/************************************************************************
 *
 ************************************************************************/
static
int dw_em_sg_make_script(struct scatterlist *sg,
             int dma_nents, struct sd_cmd_pkt *cmd_info)
{
    int err = 0;
    unsigned int desc_len;
    u32 i;
    struct idmac_desc *desc_first, *desc_last, *desc;

        mmcmsg1("sg=0x%p dma_nents=%d\n",sg,dma_nents);
    cmd_info->scrip_cnt      = 0;
    cmd_info->block_count    = 0;
    cmd_info->total_byte_cnt = 0;

    desc_first = desc_last = desc = cmd_info->sdport->scrip_buf;
    for (i = 0; i < dma_nents; i++,sg++) {
        unsigned int length = sg_dma_len(sg);
        u32 mem_addr = sg_dma_address(sg);
        for ( ; length ; desc++) {
            desc_len = (length <= DW_MCI_DESC_DATA_LENGTH) ?
                length : DW_MCI_DESC_DATA_LENGTH;

            length -= desc_len;

            /*
             * Set the OWN bit and disable interrupts
             * for this descriptor
             */
            desc->des0 = cpu_to_le32(IDMAC_DES0_OWN |
                    IDMAC_DES0_DIC |
                    IDMAC_DES0_CH);

            /* Buffer length */
            IDMAC_SET_BUFFER1_SIZE(desc, desc_len);

            /* Physical address to DMA to/from */
            desc->des2 = cpu_to_le32(mem_addr);

            /* Update physical address for the next desc */
            mem_addr += desc_len;

            /* Save pointer to the last descriptor */
            desc_last = desc;
            cmd_info->scrip_cnt++;
            cmd_info->total_byte_cnt += desc_len;
        }
    }
    cmd_info->byte_count = 0x200;
    cmd_info->block_count  = cmd_info->total_byte_cnt / 512;
    if (cmd_info->total_byte_cnt & 0x1FF){
        if (cmd_info->total_byte_cnt < 512)
            cmd_info->byte_count = cmd_info->total_byte_cnt;
        cmd_info->block_count += 1;
    }

    /* Set first descriptor */
    desc_first->des0 |= cpu_to_le32(IDMAC_DES0_FD);

    /* Set last descriptor */
    desc_last->des0 &= cpu_to_le32(~IDMAC_DES0_DIC);
    desc_last->des0 |= cpu_to_le32(IDMAC_DES0_LD);

    wmb(); /* drain writebuffer */

    return err;
}

int export_em_sg_make_script(struct scatterlist *sg,
             int dma_nents, struct sd_cmd_pkt *cmd_info)
{
    return dw_em_sg_make_script(sg,dma_nents,cmd_info);
}
EXPORT_SYMBOL(export_em_sg_make_script);

/************************************************************************
 *
 ************************************************************************/
static
int dw_em_reinit_idmac_pre(struct sd_cmd_pkt *cmd_info)
{
    unsigned int ctrl = 0;

    cr_writel(SDMMC_SWR,EM_DWC_BMOD);

    if(cmd_info->sdport->int_mode){
        /* Mask out interrupts - get Tx & Rx complete only */
        //ctrl = (SDMMC_IDMAC_INT_NI|SDMMC_IDMAC_INT_RI|SDMMC_IDMAC_INT_TI);
        ctrl = 0;
    }else{
        ctrl = 0;
    }

    cr_writel(IDMAC_INT_CLR,EM_DWC_IDSTS);
    cr_writel(ctrl, EM_DWC_IDINTEN);

    dw_em_idmac_init(cmd_info->sdport);
    dw_em_set_scrip_base(cmd_info->sdport->scrip_buf_phy);

    /* Select IDMAC interface */
    ctrl = cr_readl(EM_DWC_CTRL);
    ctrl |= SDMMC_CTRL_USE_IDMAC;
    cr_writel(ctrl, EM_DWC_CTRL);
    wmb();

    /* Enable the IDMAC */
    ctrl = cr_readl(EM_DWC_BMOD);
    ctrl |= SDMMC_IDMAC_ENABLE;
    cr_writel(ctrl, EM_DWC_BMOD);
    wmb();

    /* Start it running */
    cr_writel(1, EM_DWC_PLDMND);
    wmb();

    return 0;
}


int export_em_reinit_idmac_pre(struct sd_cmd_pkt *cmd_info)
{
    return dw_em_reinit_idmac_pre(cmd_info);
}
EXPORT_SYMBOL(export_em_reinit_idmac_pre);

/************************************************************************
 *
 ************************************************************************/
static
int dw_em_reinit_idmac_post(struct sd_cmd_pkt *cmd_info)
{
    u32 temp;

    /* Disable and reset the IDMAC interface */
    temp = cr_readl(EM_DWC_CTRL);
    temp &= ~SDMMC_CTRL_USE_IDMAC;
    temp |= SDMMC_CTRL_DMA_RESET;
    cr_writel(temp, EM_DWC_CTRL);
    wmb();

    /* Stop the IDMAC running */
    temp = cr_readl(EM_DWC_BMOD);
    temp &= ~(SDMMC_IDMAC_ENABLE | SDMMC_IDMAC_FB);
    cr_writel(temp, EM_DWC_BMOD);
    wmb();

    return 0;
}

int export_em_reinit_idmac_post(struct sd_cmd_pkt *cmd_info){
    return dw_em_reinit_idmac_post(cmd_info);
}
EXPORT_SYMBOL(export_em_reinit_idmac_post);

/************************************************************************
 *  This func is used for red mmc fix pattern func to verify data.
 *  if block header is 0x2379fbeef then scan data.
 ************************************************************************/
#ifdef DEBUG_BUFFER

// This is reference function below
// lib/scatterlist.c
// static size_t sg_copy_buffer(struct scatterlist *sgl, unsigned int nents,
//			     void *buf, size_t buflen, int to_buffer)
static
void emmc_vreify_buf(struct sd_cmd_pkt *cmd_info, int to_buffer)
{
    int i,j;
    struct scatterlist *sg;
    u32* dma_addr_virt = NULL;
    u32 dma_leng;
    int miter_cnt;
    int blk_cnt;
    int show_err;
    struct mmc_host *host;
    struct sg_mapping_iter sg_miter;
    unsigned long flags;
    unsigned int sg_flags = SG_MITER_ATOMIC;

    host = cmd_info->sdport->mmc;
    sg = cmd_info->data->sg;

    if (to_buffer)
        sg_flags |= SG_MITER_FROM_SG;
    else
        sg_flags |= SG_MITER_TO_SG;

    sg_miter_start(&sg_miter, cmd_info->data->sg,cmd_info->data->sg_len, sg_flags);

    local_irq_save(flags);

    miter_cnt = 0;
    while(sg_miter_next(&sg_miter)){
        blk_cnt = 0;
        dma_addr_virt = (u32*)sg_miter.addr;
        dma_leng = sg_miter.length;
        blk_cnt = (dma_leng>>9);
        ++miter_cnt;
        for(i=0; i< blk_cnt; i++ ){
            if(dma_addr_virt[(i*128)+0] == 0x2379beef){
                show_err = 0;
                j = 0;
                for(j=1; j< 128; j++ ){
                    if((dma_addr_virt[(i*128)+j] & 0xffff0000) != 0xbeef0000){
                        EM_ALERT("error hit!!! dma_addr_phy[%x]=0x%08x, (virt_addr=0x%p, phy_addr=0x%08x)\n",
                                (i*128)+j, dma_addr_virt[(i*128)+j],
                                &(dma_addr_virt[(i*128)+j]),
                                virt_to_phys((void*)&(dma_addr_virt[(i*128)+j])));
                        show_err = 1;
                        break;
                    }
                }

                if(show_err){
                    if(i==0){
                        EM_ALERT("##(%s)## miter_cnt=%d\n",(cmd_info->data->flags & MMC_DATA_READ)?"R":"W",miter_cnt);
                        EM_ALERT("dma_addr_virt =0x%p len=0x%x blk_cnt=%d\n",dma_addr_virt,dma_leng,blk_cnt);
                    }

                    EM_ALERT("virt_addr=0x%p, phy_addr=0x%08x\n",
                                &(dma_addr_virt[(i*128)]),
                                virt_to_phys((void*)&(dma_addr_virt[(i*128)])));
                    EM_ALERT("dma_addr_phy[0]=0x%08x [1]=0x%08x [2]=0x%08x [3]=0x%08x\n",
                        dma_addr_virt[(i*128)+0],dma_addr_virt[(i*128)+1],
                        dma_addr_virt[(i*128)+2],dma_addr_virt[(i*128)+3]);
//#define SHOW_BLK_DATA
#ifdef SHOW_BLK_DATA
                    for(j=0; j< (128/8); j++ ){
                            u32 addr_sht = (i*128)+(j*8);
                            EM_ALERT("[%03x-%03x] %08x %08x %08x %08x %08x %08x %08x %08x",
                                 (j*32)+0,(j*32)+31,
                                 dma_addr_virt[addr_sht+0],dma_addr_virt[addr_sht+1],
                                 dma_addr_virt[addr_sht+2],dma_addr_virt[addr_sht+3],
                                 dma_addr_virt[addr_sht+4],dma_addr_virt[addr_sht+5],
                                 dma_addr_virt[addr_sht+6],dma_addr_virt[addr_sht+7]);
                            EM_ALERT("\n");
                    }
                    EM_ALERT("\n");
#endif
                }
            }
        }
    }

    sg_miter_stop(&sg_miter);

    local_irq_restore(flags);
}

#endif

/************************************************************************
 *
 ************************************************************************/

static
int dw_em_Stream(struct sd_cmd_pkt *cmd_info)
{
    int err=0;
    struct mmc_host *host;
    struct rtksd_host *sdport;
    struct scatterlist *sg;
    u32 dir = 0;
    int dma_nents = 0;

    host   = cmd_info->sdport->mmc;
    sdport = cmd_info->sdport;

    if(sdport->scrip_buf == NULL){
       /*
        * Scripter base is setting when srcipter buffer alloced.
        * check where is dw_em_set_scrip_base().
        */
        BUG_ON(1);
        return cmd_info->cmd->error = -ENOMEM;
    }

    if(cmd_info->data->flags & MMC_DATA_READ){
        dir = DMA_FROM_DEVICE;
    }else{
        dir = DMA_TO_DEVICE;
    }

    cmd_info->data->bytes_xfered=0;
    dma_nents = dma_map_sg( mmc_dev(host), cmd_info->data->sg,
            cmd_info->data->sg_len,  dir);
    sg = cmd_info->data->sg;

    if((dma_nents < 0) || (dma_nents > host->max_segs) ){
        WARN(1, "Unexpected scatter&gather case. dma_nents=%d\n",
                dma_nents);
        err = -ENOMEM;
        cmd_info->cmd->error = err;
        goto umap_err_out;
    }

    if(dw_em_get_occupy(sdport)){
        return cmd_info->cmd->error = -EIO;
    }

#define RE_INITIAL_IDMAC
#ifdef  RE_INITIAL_IDMAC
                /* Software reset of DMA */
    dw_em_reinit_idmac_pre(cmd_info);
#endif

    if(dw_em_sg_make_script(sg,dma_nents,cmd_info)){
        err = -ENOMEM;
        cmd_info->cmd->error = err;
        dw_em_release_occupy(sdport);
        goto umap_err_out;
    }

    if((cmd_info->scrip_cnt)>EMMC_MAX_SCRIPT_LINE ){
        WARN(1, "scrip length more then %d(%d)\n",
                EMMC_MAX_SCRIPT_LINE,cmd_info->scrip_cnt);
        err = -ENOMEM;
        cmd_info->cmd->error = err;
        dw_em_release_occupy(sdport);
        goto umap_err_out;
    }

#ifdef DEBUG_BUFFER
    if( (rtk_emmc_test_ctl & EMMC_CHK_BUF) &&
        (cmd_info->data->flags & MMC_DATA_WRITE)){
        emmc_vreify_buf(cmd_info,1);
    }
#endif

    err = dw_em_Stream_Cmd(cmd_info);

#ifdef DEBUG_BUFFER
    if( (rtk_emmc_test_ctl & EMMC_CHK_BUF) &&
        (cmd_info->data->flags & MMC_DATA_READ)){
        emmc_vreify_buf(cmd_info,0);
    }
#endif

#ifdef  RE_INITIAL_IDMAC
    dw_em_reinit_idmac_post(cmd_info);
#endif
    dw_em_release_occupy(sdport);
    if(err == 0){
        if(host->card){

#ifdef RTK_CMD23_USE
            if( !cmd_info->predefined_read
             && !(cmd_info->cmd_flag & SDMMC_CMD_SEND_STOP)
             && ( (cmd_info->cmd->opcode == 18)
               || (cmd_info->cmd->opcode == 25) ) )
#else
            if( !(cmd_info->cmd_flag & SDMMC_CMD_SEND_STOP)
             && ( (cmd_info->cmd->opcode == 18)
               || (cmd_info->cmd->opcode == 25) ) )
#endif
            {
                if(dw_em_stop_transmission(sdport)){
                    /* if stop cmd error, set stop cmd error code, and skip bytes_xfered caculation */
                    err = -ETIMEDOUT;
                    cmd_info->stop->error = err;
                    goto umap_err_out;
                }
            }
        }
        cmd_info->data->bytes_xfered = cmd_info->total_byte_cnt;
        cmd_info->data->error = 0;
    }else{
        cmd_info->data->bytes_xfered = 0;
    }

umap_err_out:
    dma_unmap_sg( mmc_dev(host), cmd_info->data->sg,
            cmd_info->data->sg_len,  dir);

    return err;
}

/************************************************************************
 *
 ************************************************************************/
void show_CRC_value(void)
{
#define CRC_REG_BASE 0xb8010854
    int i;
    if(rtk_emmc_test_ctl & EMMC_SHOW_CRC){
        for(i=0;i<2;i++){
            EM_ALERT("0x%08x=0x%08x 0x%08x=0x%08x 0x%08x=0x%08x 0x%08x=0x%08x\n",
            CRC_REG_BASE+(i*16)+0,  cr_readl(CRC_REG_BASE+(i*16)+0),
            CRC_REG_BASE+(i*16)+4,  cr_readl(CRC_REG_BASE+(i*16)+4),
            CRC_REG_BASE+(i*16)+8,  cr_readl(CRC_REG_BASE+(i*16)+8),
            CRC_REG_BASE+(i*16)+12, cr_readl(CRC_REG_BASE+(i*16)+12));
        }
    }
}

/************************************************************************
 *
 ************************************************************************/
static
void rtkem_polling_tasklet(unsigned long param)
{
    u32 cmd_flag;
    struct rtksd_host *sdport;
    struct mmc_command *cmd;
    struct sd_cmd_pkt * cmd_info;
    u32  state;
    u32  prev_state = 0;
    u8 int_mode = 0;
    u32 int_status;
    u32 sd_trans;
    u32 sd_status;
    u32 wrap_sta;
    unsigned int    wrap_sta0 = 0;
    bool err_flag = false;
    unsigned long flags;

    sdport = (struct rtksd_host *)param;

    spin_lock_irqsave(&sdport->lock, flags);

    state = sdport->task_state;
    int_mode = sdport->int_mode;

    /*
     * Note:
     *  At STATE_CMD_DONE state,
     *  the struct sd_cmd_pkt have been released.
     */
    cmd_info = sdport->cmd_info;
    if(cmd_info == NULL){
        WARN_ON(1);
        sdport->task_state = STATE_IDLE;
        spin_unlock_irqrestore(&sdport->lock, flags);
        return;
    }

    cmd = cmd_info->cmd;
    cmd_flag = cmd_info->cmd_flag;

    do {
        prev_state = state;

        switch (state) {
        case STATE_IDLE:
            break;

        case STATE_SENDING_DATA:
            int_status   = cr_readl(EM_DWC_RINTSTS);
            sd_trans     = cr_readl(EM_DWC_IDSTS);
            wrap_sta     = cr_readl(EM_INT_STATUS);
            sd_status    = cr_readl(EM_DWC_STATUS);

            if(int_mode){
                int_status = sdport->int_status;
                sd_trans   |= sdport->sd_trans;
                wrap_sta   |= sdport->wrap_sta;
            }else{
                sdport->int_status  = int_status;
                sdport->sd_trans    = sd_trans;
                sdport->wrap_sta    = wrap_sta;
            }
            sdport->sd_status   = sd_status;
            wrap_sta0           = cr_readl(EM_WARP_STATUS0);

            if((int_status & SDMMC_INT_ERROR)) {
                dw_em_clear_int_sta(1);
                dw_em_clear_int_sta_IDMA(1);
                dw_em_clear_int_wrap(1);
#define DW_EM_TIMEOUT   (SDMMC_INT_HTO|SDMMC_INT_DTO|SDMMC_INT_RTO)
#define DW_EM_RCRC_ERR  (SDMMC_INT_DCRC|SDMMC_INT_RCRC|SDMMC_INT_RESP_ERR)
#define DW_EM_RESP_ERR  (SDMMC_INT_RCRC|SDMMC_INT_RESP_ERR)

                if(int_status & DW_EM_RESP_ERR){
                    cmd_info->cmd->error = -EILSEQ;
                }else{
                    cmd_info->cmd->error = -ETIMEDOUT;
                }
                err_flag = true;
            }

            if(cmd_flag & SDMMC_CMD_DAT_EXP){
                if (sd_trans & IDMAC_INT_ERR) {
                    if(!err_flag){
                        err_flag = true;
                        cmd_info->cmd->error = -ETIMEDOUT;
                    }
                    cmd_info->data->error = -ETIMEDOUT;
                }

                if (int_status & SDMMC_INT_DATA_OVER) {
                    if (cmd_flag & SDMMC_CMD_DAT_WR)
                    {
                        /* At write case, waiting card busy even error alert */
                        if(!(sd_status & SDMMC_DAT0_BUSY)){
                            if( !(sd_trans & (0x1f << 13))
                             && !(wrap_sta0 & BIT(1)))
                            {
                                state = STATE_POST_POLLING;
                                break;
                            }else{
                                mmcinfo("wait FSM!!![0x%08x]\n",sd_trans);
                                show_int_sta(__func__,__LINE__,sdport,1);

                            }
                        }else{
                            mmcinfo("wait for card busy!!!\n");
                            show_int_sta(__func__,__LINE__,sdport,1);
                        }

                    }else{
                        if(err_flag){
                            state = STATE_DATA_ERROR;
                            break;
                        }
                        if(wrap_sta & PS_DMA_WR_DONE_S){
                            /* At read case, waitting inner DDR done.
                               Read action is writting data to DDR.
                               that is why to use PS_DMA_WR_DONE_S */
                            if( !(sd_trans & (0x1f << 13))
                             && !(wrap_sta0 & BIT(0))
                             && !(wrap_sta0 & BIT(1)) )
                            {
                                state = STATE_POST_POLLING;
                                break;
                            }else{
                                mmcinfo("wait FSM!!![0x%08x]\n",sd_trans);
                                show_int_sta(__func__,__LINE__,sdport,1);
                            }
                        }else{
                            mmcinfo("wait for DDR busy!!!(0x%08x)\n",wrap_sta);
                            show_int_sta(__func__,__LINE__,sdport,1);
                        }
                    }
                } else {
                    if(int_mode){
                        state = STATE_WAIT_STOP;
                        break;
                    }
                }
            } else {
                if (int_status & SDMMC_INT_CMD_DONE) {
                    if (mmc_resp_type(cmd) & MMC_RSP_BUSY)
                    {
                        if(!(cr_readl(EM_DWC_STATUS) & SDMMC_DAT0_BUSY)){
                            state = STATE_POST_POLLING;
                            break;
                        }else{
                            mmcinfo("wait for card busy!!! [cmd%u]\n",
                                SDMMC_CMD_INDX(cmd_flag));
                        }
                    }else{
                        state = STATE_POST_POLLING;
                        break;
                    }

                }
            }
            sdport->task_state = prev_state = state;
            tasklet_schedule(&sdport->polling_tasklet);
            break;
        case STATE_POST_POLLING:
            if(cr_readl(EM_DWC_CMD) & BIT(31)){
                sdport->task_state = prev_state = state;
                tasklet_schedule(&sdport->polling_tasklet);
            }else{
                if((cmd_flag & SDMMC_CMD_DAT_EXP)
                && (cr_readl(EM_DWC_IDSTS) & (0x1f << 13))){
                    mmcmsg4("wait FSM!!![0x%08x]\n",sd_trans);
                    sdport->task_state = prev_state = state;
                    tasklet_schedule(&sdport->polling_tasklet);
                }else{
                    show_CRC_value();
                    if(cmd->error == -EINPROGRESS)
                        cmd_info->cmd->error = 0;
                    state = STATE_DATA_COMPLETE;
                }
            }
            break;
        case STATE_DATA_ERROR:
            state = STATE_DATA_COMPLETE;
            break;
        case STATE_DATA_COMPLETE:
            prev_state = state = STATE_IDLE;
            stop_timer = 1;
#if 0
            /* disable and clear int */
            dw_em_clear_int_sta(1);
            dw_em_clear_int_sta_IDMA(1);
            dw_em_clear_int_wrap(1);
#endif
            rtk_op_complete(sdport);
            break;
        }
    } while (state != prev_state);

    sdport->task_state = state;
    spin_unlock_irqrestore(&sdport->lock, flags);

}

#ifdef RTK_EMMC_ISR_DIRECT
static
void rtkem_irq_task(struct rtksd_host *sdport)
{
    u32 cmd_flag;
    struct mmc_command *cmd;
    struct sd_cmd_pkt * cmd_info;
    u32  state;
    u32  prev_state = 0;
    u8 int_mode = 0;
    u32 int_status;
    u32 sd_trans;
    u32 sd_status;
    u32 wrap_sta;
    unsigned int    wrap_sta0 = 0;
    bool err_flag = false;

    state = sdport->task_state;
    int_mode = sdport->int_mode;

    /*
     * Note:
     *  At STATE_CMD_DONE state,
     *  the struct sd_cmd_pkt have been released.
     */
    cmd_info = sdport->cmd_info;
    if(cmd_info == NULL){
        WARN_ON(1);
        sdport->task_state = STATE_IDLE;
        return;
    }

    cmd = cmd_info->cmd;
    cmd_flag = cmd_info->cmd_flag;

    do {
        prev_state = state;

        switch (state) {
        case STATE_IDLE:
            break;

        case STATE_SENDING_DATA:
            int_status   = cr_readl(EM_DWC_RINTSTS);
            sd_trans     = cr_readl(EM_DWC_IDSTS);
            wrap_sta     = cr_readl(EM_INT_STATUS);
            sd_status    = cr_readl(EM_DWC_STATUS);

            if(int_mode){
                int_status = sdport->int_status;
                sd_trans   |= sdport->sd_trans;
                wrap_sta   |= sdport->wrap_sta;
            }else{
                sdport->int_status  = int_status;
                sdport->sd_trans    = sd_trans;
                sdport->wrap_sta    = wrap_sta;
            }
            sdport->sd_status   = sd_status;
            wrap_sta0           = cr_readl(EM_WARP_STATUS0);

            if((int_status & SDMMC_INT_ERROR)) {
                dw_em_clear_int_sta(1);
                dw_em_clear_int_sta_IDMA(1);
                dw_em_clear_int_wrap(1);
#define DW_EM_TIMEOUT   (SDMMC_INT_HTO|SDMMC_INT_DTO|SDMMC_INT_RTO)
#define DW_EM_RCRC_ERR  (SDMMC_INT_DCRC|SDMMC_INT_RCRC|SDMMC_INT_RESP_ERR)
#define DW_EM_RESP_ERR  (SDMMC_INT_RCRC|SDMMC_INT_RESP_ERR)

                if(int_status & DW_EM_RESP_ERR){
                    cmd_info->cmd->error = -EILSEQ;
                }else{
                    cmd_info->cmd->error = -ETIMEDOUT;
                }
                err_flag = true;
            }

            if(cmd_flag & SDMMC_CMD_DAT_EXP){
                if (sd_trans & IDMAC_INT_ERR) {
                    if(!err_flag){
                        err_flag = true;
                        cmd_info->cmd->error = -ETIMEDOUT;
                    }
                    cmd_info->data->error = -ETIMEDOUT;
                }

                if (int_status & SDMMC_INT_DATA_OVER) {
                    if (cmd_flag & SDMMC_CMD_DAT_WR)
                    {
                        /* At write case, waiting card busy even error alert */
                        if(!(sd_status & SDMMC_DAT0_BUSY)){
                            if( !(sd_trans & (0x1f << 13))
                             && !(wrap_sta0 & BIT(1)))
                            {
                                state = STATE_POST_POLLING;
                                break;
                            }else{
                                mmcinfo("wait FSM!!![0x%08x]\n",sd_trans);
                                show_int_sta(__func__,__LINE__,sdport,1);

                            }
                        }else{
                            mmcinfo("wait for card busy!!!\n");
                            show_int_sta(__func__,__LINE__,sdport,1);
                        }

                    }else{
                        if(err_flag){
                            state = STATE_DATA_ERROR;
                            break;
                        }
                        if(wrap_sta & PS_DMA_WR_DONE_S){
                            /* At read case, waitting inner DDR done.
                               Read action is writting data to DDR.
                               that is why to use PS_DMA_WR_DONE_S */
                            if( !(sd_trans & (0x1f << 13))
                             && !(wrap_sta0 & BIT(0))
                             && !(wrap_sta0 & BIT(1)) )
                            {
                                state = STATE_POST_POLLING;
                                break;
                            }else{
                                mmcinfo("wait FSM!!![0x%08x]\n",sd_trans);
                                show_int_sta(__func__,__LINE__,sdport,1);
                            }
                        }else{
                            mmcinfo("wait for DDR busy!!!(0x%08x)\n",wrap_sta);
                            show_int_sta(__func__,__LINE__,sdport,1);
                        }
                    }
                }else{
                    if(int_mode){
                        state = STATE_WAIT_STOP;
                        break;
                    }
                }
            } else {
                if (int_status & SDMMC_INT_CMD_DONE) {
                    if (mmc_resp_type(cmd) & MMC_RSP_BUSY)
                    {
                        if(!(cr_readl(EM_DWC_STATUS) & SDMMC_DAT0_BUSY)){
                            state = STATE_POST_POLLING;
                            break;
                        }else{
                            mmcinfo("wait for card busy!!! [cmd%u]\n",
                                (u32)SDMMC_CMD_INDX(cmd_flag));
                        }
                    }else{
                        state = STATE_POST_POLLING;
                        break;
                    }

                }
            }
            sdport->task_state = prev_state = state;
            tasklet_schedule(&sdport->polling_tasklet);
            break;
        case STATE_POST_POLLING:
            if(cr_readl(EM_DWC_CMD) & BIT(31)){
                sdport->task_state = prev_state = state;
                tasklet_schedule(&sdport->polling_tasklet);
            }else{
                if((cmd_flag & SDMMC_CMD_DAT_EXP)
                && (cr_readl(EM_DWC_IDSTS) & (0x1f << 13))){
                    mmcmsg4("wait FSM!!![0x%08x]\n",sd_trans);
                    sdport->task_state = prev_state = state;
                    tasklet_schedule(&sdport->polling_tasklet);
                }else{
                    show_CRC_value();
                    if(cmd->error == -EINPROGRESS)
                        cmd_info->cmd->error = 0;
                    state = STATE_DATA_COMPLETE;
                }
            }
            break;
        case STATE_DATA_ERROR:
            state = STATE_DATA_COMPLETE;
            break;
        case STATE_DATA_COMPLETE:
            prev_state = state = STATE_IDLE;
            stop_timer = 1;
            rtk_op_complete(sdport);
            break;
        case STATE_WAIT_STOP:
            prev_state = state = STATE_SENDING_DATA;
        }
    } while (state != prev_state);

    sdport->task_state = state;

}
#endif
/************************************************************************
 *
 ************************************************************************/
static
#ifdef RTK_EMMC_ISR_DIRECT
void rtkcr_req_end_tasklet(struct rtksd_host *sdport)
#else
void rtkcr_req_end_tasklet(unsigned long param)
#endif
{
    struct mmc_request* mrq;
    unsigned long flags;

#ifndef RTK_EMMC_ISR_DIRECT
    struct rtksd_host *sdport;
    sdport = (struct rtksd_host *)param;
#endif

    spin_lock_irqsave(&sdport->lock,flags);

    mrq = sdport->mrq;
    sdport->mrq = NULL;

    spin_unlock_irqrestore(&sdport->lock, flags);
    mmc_request_done(sdport->mmc, mrq);
}

/************************************************************************
 *
 ************************************************************************/
static
void rtksd_send_command(struct rtksd_host *sdport, struct mmc_command *cmd)
{
    struct sd_cmd_pkt *cmd_info = NULL;
    bool pre_rpmb = false;

    if ( !sdport || !cmd ){
        EM_ERR( "%s: sdport or cmd is null\n", DRIVER_NAME);
        return ;
    }

    cmd_info = sdport->cmd_info;
    cmd_info->cmd    = cmd;
    cmd_info->stop   = sdport->mrq->stop;
    cmd_info->sdport = sdport;

    if (cmd->data){
        cmd_info->data = cmd->data;
        dw_em_Stream(cmd_info);

    }else{
        dw_em_SendCmd(cmd_info);
    }

    pre_rpmb = is_rpmb;
    /* This is for mmc_fast_rw function. */
    if(cmd->opcode == MMC_SWITCH){
         if((cmd->arg & 0xffff00ff) == 0x03b30001) {
            if((cmd->arg & 0x0000ff00)==0){
                sdport->rtflags |= RTKCR_USER_PARTITION;
                is_rpmb = false;
            }else{
                if((cmd->arg & 0x0300)==0x0300)
                    is_rpmb = true;
                else
                    is_rpmb = false;
                sdport->rtflags &= ~RTKCR_USER_PARTITION;
            }
         }
    }
    /* We don't want to retune when partition changed */
    if(!cmd_info->sbc_flag)
#ifdef RTK_EMMC_ISR_DIRECT
        rtkcr_req_end_tasklet(sdport);
#else
        tasklet_schedule(&sdport->req_end_tasklet);
#endif
}

/************************************************************************
 *
 ************************************************************************/
static
void dw_em_request(struct mmc_host *host, struct mmc_request *mrq)
{
    struct rtksd_host *sdport = NULL;
    struct mmc_command *cmd = NULL;
    struct sd_cmd_pkt *cmd_info = NULL;
    unsigned long flags;

    sdport = mmc_priv(host);

    BUG_ON(!sdport);
    BUG_ON(sdport->mrq != NULL);
    BUG_ON(!(mrq->cmd));

    /*
     * The flag "rtk_emmc_raw_op" is for store log while kernel panic.
     * When this flag alert, skip all request come from system.
     */
    while(rtk_emmc_raw_op){
        EM_ERR("wait RAW func finish!!!!!!!!!!!!!!!!!!!\n");
        msleep(10);
    }

    /*
    Beware: only struct mmc_command malloc by rtk_self should be assigned to sdport->cmd.
    It's to identify which assigend by.
    */
    cmd_info = kmalloc(sizeof(struct sd_cmd_pkt), GFP_KERNEL);
    spin_lock_irqsave(&sdport->lock,flags);
    sdport->mrq = mrq;

    if(!cmd_info ){
        EM_ERR("request cmd_info memory fail\n");
        mrq->cmd->error = -ENOMEM;
        mrq->cmd->retries = 0;
        goto done;
    }
    memset(cmd_info, 0, sizeof(struct sd_cmd_pkt));

    if (!(sdport->rtflags & RTKCR_FCARD_DETECTED)){
        cmd->error = -ENOMEDIUM;
        cmd->retries = 0;
        goto done;
    }

#ifdef RTK_CMD23_USE
    cmd_info->predefined_read = predefined_read;
#endif

    if(mrq->sbc){
       /* 5.4 rpmb must use sbc control */
#ifdef RTK_CMD23_USE
        if (is_rpmb || cmd_info->predefined_read)
#else
        if (is_rpmb)
#endif
        {
            cmd_info->sbc_flag = 1;
            cmd = mrq->sbc;
            cmd->error = -EINPROGRESS;
            if (cmd->data){
                cmd->data->error = -EINPROGRESS;
            }
            sdport->cmd_info = cmd_info;
            sdport->cmd = cmd;

            spin_unlock_irqrestore(&sdport->lock, flags);
            rtksd_send_command(sdport, cmd);
            cmd_info->sbc_flag = 0;
            spin_lock_irqsave(&sdport->lock,flags);
            memset(cmd_info, 0, sizeof(struct sd_cmd_pkt));
        }
    }

    cmd = mrq->cmd;
    cmd->error = -EINPROGRESS;
    if (cmd->data){
        cmd->data->error = -EINPROGRESS;
    }
    sdport->cmd_info = cmd_info;
    sdport->cmd = cmd;

    spin_unlock_irqrestore(&sdport->lock, flags);
    rtksd_send_command(sdport, cmd);
    goto out;

done:
#ifdef RTK_EMMC_ISR_DIRECT
    spin_unlock_irqrestore(&sdport->lock, flags);
    rtkcr_req_end_tasklet(sdport);
#else
    tasklet_schedule(&sdport->req_end_tasklet);
    spin_unlock_irqrestore(&sdport->lock, flags);
#endif

out:
    if(cmd_info)
        kfree(cmd_info);
}

/************************************************************************
 *
 ************************************************************************/
static
void dw_em_hw_reset(struct mmc_host *host)
{
    struct rtksd_host *sdport;
    sdport = mmc_priv(host);

    if(dw_em_get_occupy(sdport)){
        return;
    }

    if (sdport->int_mode)
        rtkemmc_disable_int(sdport);

    rtkem_clr_sta(sdport);
    sdport->ops->reset_card(sdport);
    sdport->ops->reset_host(sdport,1);
    if (sdport->int_mode)
        rtkemmc_enable_int(sdport);

    dw_em_release_occupy(sdport);
}

/************************************************************************
 *
 ************************************************************************/
#define HS_52_DDR   (0x01)
#define HS_200_DDR  (0x01<<16)

static
void dw_em_set_ios(struct mmc_host *host, struct mmc_ios *ios)
{
    struct rtksd_host *sdport;
    u32 tmp_clock = 0;
    u32 tmp_bits = 0;
    u32 ctrl_DDR = 0;
    u32 driving = 0;

    sdport = mmc_priv(host);
    mmcmsg2("bus_mode  = %u\n",ios->bus_mode);
    mmcmsg2("clock     = %u\n",ios->clock);
    mmcmsg2("timing    = %u\n",ios->timing);
    mmcmsg2("bus_width = %u\n", ios->bus_width);

    sdport->ops->set_DDR = dw_em_set_DDR;
    sdport->cur_timing = ios->timing;
    driving = DRIVING_COMMON;

    if(dw_em_get_occupy(sdport)){
        return;
    }

    if (sdport->int_mode)
        rtkemmc_disable_int(sdport);

    if (ios->bus_mode == MMC_BUSMODE_PUSHPULL){
        if (ios->bus_width == MMC_BUS_WIDTH_8){
            tmp_bits = MMC_BUS_WIDTH_8;
        }else if (ios->bus_width == MMC_BUS_WIDTH_4){
            tmp_bits = MMC_BUS_WIDTH_4;
        }else{
            tmp_bits = MMC_BUS_WIDTH_1;
        }

        tmp_clock = ios->clock;

        if (ios->timing == MMC_TIMING_MMC_HS400) {
            dw_em_set_ds_delay(sdport, sdport->cur_ds_tune);
            ctrl_DDR = (HS_52_DDR|HS_200_DDR);
            if (tmp_clock > CLK_200Mhz) {
                tmp_clock = CLK_200Mhz;
            }
            sdport->ops->set_DDR = dw_em_set_DDR400;
            driving = DRIVING_HS400;
        } else if (ios->timing == MMC_TIMING_MMC_HS200) {
            ctrl_DDR = 0;
            if (tmp_clock > CLK_200Mhz) {
                tmp_clock = CLK_200Mhz;
            }
            driving = DRIVING_HS200;
        } else if (ios->timing == MMC_TIMING_UHS_DDR50) {
            dw_em_dis_sd_tune(sdport);
            ctrl_DDR = HS_52_DDR;
            if(tmp_clock > CLK_50Mhz){
                tmp_clock = CLK_50Mhz;
            }
        } else if ((ios->timing == MMC_TIMING_SD_HS) ||
            (ios->timing == MMC_TIMING_MMC_HS)) {
            dw_em_dis_sd_tune(sdport);
            ctrl_DDR = 0;
            if(tmp_clock > CLK_50Mhz){
                tmp_clock = CLK_50Mhz;
            }
        } else {
            dw_em_dis_sd_tune(sdport);
            ctrl_DDR = 0;
            if(tmp_clock>CLK_25Mhz){
                tmp_clock = CLK_25Mhz;
            }
        }

    } else {
        /* MMC_BUSMODE_OPENDRAIN */
        tmp_clock = CLK_200Khz;
        tmp_bits = MMC_BUS_WIDTH_1;
        ctrl_DDR = 0;

    }

    if(tmp_clock == CLK_50Mhz)
        sdport->ops->set_IO_driving(sdport, CLK_50Mhz);
    else
        sdport->ops->set_IO_driving(sdport,driving);

    sdport->ops->set_DDR(sdport, ctrl_DDR);
    sdport->ops->set_bits(sdport, tmp_bits);
    sdport->ops->set_clk(sdport,tmp_clock);
    /* eMMC Power control */
    switch (ios->power_mode) {
    case MMC_POWER_UP:
        cr_writel(BIT(0), EM_DWC_PWREN);
        break;
    case MMC_POWER_OFF:
        /* Power down slot */
        /* record ds_tune for STR flow */
        sdport->cur_ds_tune = cr_readl(EM_HALF_CYCLE_CAL_RESULT);
        cr_writel(0, EM_DWC_PWREN);
        break;
    default:
        break;
    }

    if (sdport->int_mode)
        rtkemmc_enable_int(sdport);
    dw_em_release_occupy(sdport);

}

/************************************************************************
 *
 ************************************************************************/
/************************************************************************
 *
 *  Description : To set emmc pinmux, start from 287O
 *  Parameters :
 *  Return values :
 ************************************************************************/
/************************************************************************
 *
 ************************************************************************/
static
void rtkem_chk_card_insert(struct rtksd_host *sdport)
{
    struct mmc_host *host=sdport->mmc;

    if (sdport->int_mode)
        rtkemmc_disable_int(sdport);
    /* setting HW  */

    /* enable MMC pin_mux first */
    set_emmc_pin_mux();

    /* enable PLL of eMMC */
    dw_em_set_PLL_clk(sdport,CLK_50Mhz,0,1);

    /* set emmc pin mux */
    if(sdport->ops->set_crt_muxpad)
        sdport->ops->set_crt_muxpad(sdport);

    sdport->ops->reset_host(sdport,0);
    sdport->ops->set_IO_driving(sdport,DRIVING_COMMON);
    sdport->ops->set_clk(sdport,CLK_200Khz);
    if (get_mmc_tuning_50_done(host))
        dw_em_init_phase(sdport,DEFAULT_W_PHASE,sdport->tud_r_pha50,DEFAULT_C_PHASE); /* w_phase;r_phase */
    else
        dw_em_init_phase(sdport,DEFAULT_W_PHASE,DEFAULT_R_PHASE,DEFAULT_C_PHASE); /* w_phase;r_phase */
    sdport->ops->set_bits(sdport, MMC_BUS_WIDTH_1);
    /* move reset card to dw_em_hw_reset() to support re-initial flow. */
    host->ops = &rtkemmc_ops;
    sdport->rtflags |= RTKCR_FCARD_DETECTED;

    if (sdport->int_mode)
        rtkemmc_enable_int(sdport);
}

/************************************************************************
 *
 ************************************************************************/
static
void rtksd_timeout_timer(struct timer_list *t)
{
    struct rtksd_host *sdport = NULL;
    struct mmc_command *cmd = NULL;

    u32 int_status = 0;
    u32 int_status_m = 0;
    u32 sd_trans = 0;
    u32 sd_status = 0;
    u32 wrap_sta = 0;
    unsigned long flags;

    sdport = from_timer(sdport, t, timer);
    cmd = sdport->cmd;

    if (rtk_emmc_debug_log){
        EM_ERR("%s:running\n", __func__);
    }

    spin_lock_irqsave(&sdport->lock,flags);
    if (stop_timer)
    {
        done_timer = 1;
        spin_unlock_irqrestore(&sdport->lock, flags);
        EM_INFO("%s:fast return\n", __func__);
        return;
    }

    cmd->error =  -ETIMEDOUT;
    if(cmd->data){
        cmd->data->error = -ETIMEDOUT;
    }

    if(sdport->int_waiting){

        int_status   = cr_readl(EM_DWC_RINTSTS);
        int_status_m = cr_readl(EM_DWC_MINTSTS);
        sd_trans     = cr_readl(EM_DWC_IDSTS);
        sd_status    = cr_readl(EM_DWC_STATUS);
        wrap_sta     = cr_readl(EM_INT_STATUS);
        /* disable and clear int */
        dw_em_clear_int_sta(1);
        dw_em_clear_int_sta_IDMA(1);
        dw_em_clear_int_wrap(1);

        if(sdport->int_mode){

            sdport->int_status |= ((int_status & 0xffff) | (int_status_m<<16));
            sdport->sd_trans = (sdport->sd_trans & ~(0xfU<<13))|sd_trans;
            sdport->sd_status = sd_status;
            sdport->wrap_sta |= wrap_sta;
            sdport->task_state  = STATE_DATA_COMPLETE;
        }else{
            /* must keep interrupt pending bit for tesklet polling */
            sdport->int_status  = (int_status & 0xffff) | (int_status_m<<16);
            sdport->sd_trans    = sd_trans;
            sdport->sd_status   = sd_status;
            sdport->wrap_sta    = wrap_sta;
            sdport->task_state  = STATE_DATA_ERROR;
        }

    }else{
        sdport->task_state  = STATE_DATA_ERROR;
        WARN_ON(1);
    }

    /* disable and clear int */
    dw_em_clear_int_sta(1);
    dw_em_clear_int_sta_IDMA(1);
    dw_em_clear_int_wrap(1);

    done_timer = 1;
    rtk_op_complete(sdport);
    spin_unlock_irqrestore(&sdport->lock, flags);

    if((!(sdport->rtflags & RTKCR_TUNING_STATE))
     ||(sdport->rtflags & RTKCR_FOPEN_LOG))
        EM_INFO( "%s:  card access time out!\n",DRIVER_NAME);

}

/************************************************************************
 *
 ************************************************************************/
#ifdef EMMC_SHOUTDOWN_PROTECT
static
void rtksd_gpio_isr(VENUS_GPIO_ID gid, unsigned char assert, void *dev)
{
    struct rtksd_host *sdport = dev;
    u32 reg_tmp;
    u32 bit_tmp;
    u32 reginfo;
    u32 invert;
    unsigned char assert_tmp;

    reginfo = (u32)GET_PIN_TYPE(sdport->gpio_isr_info);
    bit_tmp = (u32)GET_PIN_INDEX(sdport->gpio_isr_info);
    invert  = (u32)GET_GPIO_INVERT(sdport->gpio_isr_info);

    reg_tmp = ((bit_tmp & 0xffffffe0)>>5)<<2;
    bit_tmp = (bit_tmp & 0x1fUL);

    switch(reginfo){
        case PCB_PIN_TYPE_GPIO:
            reg_tmp += MIS_GPDATI_BASE;
            break;
        case PCB_PIN_TYPE_ISO_GPIO:
            reg_tmp += ISO_GPDATI_BASE;
            break;
        default:
        EM_INFO( "PIN group not match\n");
            WARN_ON(1);
            return;
    }

    reginfo = cr_readl(reg_tmp);

    assert_tmp =
        (unsigned char)((reginfo & (0x01UL << bit_tmp)) >> bit_tmp);

    /* In here, need to make sure the level of GPIO if high or low. */
    if(invert)
        invert = 0;
    else
        invert = (0x01UL <<bit_tmp);

    if ((reginfo & (0x01UL << bit_tmp)) == invert) {

        local_irq_disable();
        preempt_disable();

        /* disable Watchdog 0xB801B6F0 */
        /* regval = cr_readl(0xB801B6F0) & ~0xFF; */
        /* cr_writel(regval | 0xa5, 0xB801B6F0); */

        rtkem_hold_card(sdport);

        EM_NOTICE( "%s(%u)eMMC have locked on!!!\n",
            __func__,__LINE__);

        /* cr_writel(0x04000000, 0xb801B6FC); // set 2.4 sec */
        /* cr_writel(cr_readl(0xb801B6F0) & ~0xFF, 0xb801B6F0); // enable Watchdog */
        while(1);
    }
}
#endif

/************************************************************************
 *
 ************************************************************************/
static __attribute__((unused))
void dw_em_chk_shoutdown_protect(struct rtksd_host *sdport)
{
#ifdef EMMC_SHOUTDOWN_PROTECT
    /* note: AC detect pin is iso_gpio_36 on 296x*/
    u64 rtk_tmp_gpio;

    /* #define FAKE_AC_DET_PIN */
#ifdef FAKE_AC_DET_PIN
    /* for reference POWER_EN  0x101010c04 */
    if(pcb_mgr_get_enum_info_byname("POWER_EN", &rtk_tmp_gpio)==0)
#else
    if(pcb_mgr_get_enum_info_byname("PIN_AC_DETECT", &rtk_tmp_gpio)==0)
#endif
    {
        u32 gpio_group;
        u32 rtk_gpio_num;
        set_gpio_func_t gpio_func;

        EM_NOTICE("PIN_AC_DETECT got.(0x%x)\n", (u32) rtk_tmp_gpio);

        gpio_group      = GET_PIN_TYPE(rtk_tmp_gpio);
        rtk_gpio_num    = GET_PIN_INDEX(rtk_tmp_gpio);

#ifdef FAKE_AC_DET_PIN
        EM_ERR("gpio_group = %x\n", gpio_group);
        EM_ERR("rtk_gpio_num = %x\n", rtk_gpio_num);

        /* Has 296x board AC detect pin? */
        rtk_tmp_gpio    = 0x0015303;
        gpio_group      = GET_PIN_TYPE(rtk_tmp_gpio);
        rtk_gpio_num    = GET_PIN_INDEX(rtk_tmp_gpio);
        EM_ERR("\nfor test change PIN_AC_DETECT to 0x%08llx.\n",
               rtk_tmp_gpio);
        EM_ERR("gpio_group = %x\n", gpio_group);
        EM_ERR("rtk_gpio_num = %x\n", rtk_gpio_num);
#endif
        sdport->gpio_isr_info = (u32)rtk_tmp_gpio;

        switch(gpio_group){
        case PCB_PIN_TYPE_GPIO:
            EM_INFO("PIN type is PCB_PIN_TYPE_GPIO\n");
            if (rtk_gpio_num > MAX_MIS_GPIO_CNT) {
                EM_INFO
                    ("Error Pin define,Skip AC_DET function\n");
                goto AC_DET_OUT;
            }
            gpio_group = MIS_GPIO;
            gpio_func = rtkcr_set_mis_gpio;
            break;
        case PCB_PIN_TYPE_ISO_GPIO:
            EM_INFO("PIN type is PCB_PIN_TYPE_ISO_GPIO\n");
            if(iso_gpio_mux_tbl[rtk_gpio_num][0] == 0xff ||
                rtk_gpio_num > MAX_ISO_GPIO_CNT) {
                EM_INFO
                    ("Error Pin define,Skip AC_DET function\n");
                goto AC_DET_OUT;
            }
            gpio_group = ISO_GPIO;
            gpio_func = rtkcr_set_iso_gpio;
            break;
        default:
            EM_INFO("PIN group not match\n");
            goto AC_DET_OUT;

        }

        EM_INFO("PIN_AC_DETECT is %s_GPIO %u.\n",
                    gpio_group==ISO_GPIO? "ISO":"MIS",rtk_gpio_num);

        gpio_func(rtk_gpio_num,DIR_IN|GPIO_L);
        venus_gpio_set_debounce(venus_gpio_id(gpio_group, rtk_gpio_num), VENUS_GPIO_DEBOUNCE_10ms);
        venus_gpio_set_dir(venus_gpio_id(gpio_group, rtk_gpio_num), 0);
        venus_gpio_clear_isr(venus_gpio_id(gpio_group, rtk_gpio_num));
        if (venus_gpio_request_irq
            (venus_gpio_id(gpio_group, rtk_gpio_num), rtksd_gpio_isr,
             DRIVER_NAME, sdport)) {
            EM_INFO("%s: request gpio isr fail\n", DRIVER_NAME);
            goto AC_DET_OUT;
        }
        venus_gpio_set_irq_enable(venus_gpio_id
                      (gpio_group, rtk_gpio_num), 1);

    }

AC_DET_OUT:
    return;

#else //? #ifdef EMMC_SHOUTDOWN_PROTECT
    EM_NOTICE( "%s: EMMC_SHOUTDOWN_PROTECT not define\n",
            DRIVER_NAME);

#ifdef EMMC_SHOUTDOWN_PROTECT_FAIL
    EM_NOTICE("%s: Lost config CONFIG_REALTEK_PCBMGR or CONFIG_REALTEK_GPIO, Please check! \n",
            DRIVER_NAME);
#endif

#endif  //? #ifdef EMMC_SHOUTDOWN_PROTECT
}

/************************************************************************
 *
 ************************************************************************/
static
irqreturn_t dw_em_irq(int irq, void *dev)
{
    struct rtksd_host *sdport = dev;

    int irq_handled = 0;

    u32 int_status = 0;
    u32 int_status_m = 0;
    u32 sd_trans = 0;
    u32 sd_status = 0;
    u32 wrap_sta = 0;
    u32 int_mask = 0;
    u32 reg1,reg2;
    unsigned long flags;

    spin_lock_irqsave(&sdport->lock, flags);

    reg1 = cr_readl(RTK_EM_DW_SRST);
    reg2 = cr_readl(RTK_EM_DW_CLKEN);

    if( !(reg1 & RSTN_EM_DW)
     || !(reg2 & CLKEN_EM_DW) )
    {
        EM_ERR("%s: Clock has been disable, no interrupt service\n",
                DRIVER_NAME);

/* #define SHOW_CRT_CLK_INFO*/
#ifdef SHOW_CRT_CLK_INFO
        EM_ERR("%s: RTK_EM_DW_SRST=0x%08x RTK_EM_DW_CLKEN=0x%08x\n",
                DRIVER_NAME,reg1,reg2);
#endif
    }else{
        int_status   = cr_readl(EM_DWC_RINTSTS);
        int_status_m = cr_readl(EM_DWC_MINTSTS);
        sd_trans     = cr_readl(EM_DWC_IDSTS);
        sd_status    = cr_readl(EM_DWC_STATUS);
        wrap_sta     = cr_readl(EM_INT_STATUS);
        int_mask     = cr_readl(EM_DWC_INTMASK);
    }

    if(int_status_m){
        /*
         Spec 6.2.17: MISTATS = RINTSTS & INTMASK.
         But (int_status2 != int_status_m) some times, I don't know why!!
        */
        u32 int_status2 = int_status & int_mask;

        int_collect |= int_status_m;

        cr_writel(int_status_m,EM_DWC_RINTSTS);

        if(int_status2 != int_status_m){
            mmcinfo("RINTSTS=0x%08x MINTSTS=0x%08x IDSTS=0x%08x int_mask=0x%08x\n",
                    int_status,int_status_m,sd_trans,int_mask);
            int_status |= int_status_m;
        }
        irq_handled |= BIT(0);
    }

    if(sd_trans){
        cr_writel(sd_trans,EM_DWC_IDSTS);
        if(sd_trans & cr_readl(EM_DWC_IDINTEN)){
            irq_handled |= BIT(1);
        }
    }

    /* has not use wrap int func, check by pulling */
    if(wrap_sta){
        cr_writel(wrap_sta,EM_INT_STATUS);
        if(wrap_sta & (~cr_readl(EM_INT_MASK) & 0x07)){
            irq_handled |= BIT(2);
        }
    }

    if(irq_handled){
        /* disable and clear int */
        //dw_em_clear_int_sta(1);
        //dw_em_clear_int_sta_IDMA(1);
        //dw_em_clear_int_wrap(1);

        sdport->int_status  |= ((int_status & 0xffff) | (int_status_m<<16));
        sdport->sd_trans    |= (sd_trans & ~(0xfU<<13));
        sdport->wrap_sta    |= wrap_sta;
        sdport->sd_status   = sd_status;

        if(sdport->int_waiting){
#ifdef RTK_EMMC_ISR_DIRECT
            rtkem_irq_task(sdport);
#else
            tasklet_schedule(&sdport->polling_tasklet);
#endif
        }else{
            EM_ERR("\n"
                "%s: No int_waiting!!!(0x%02x)\n"
                "   INT_STATUS=0x%08x RINTSTS=0x%08x\n"
                "   IDSTS=0x%08x STATUS=0x%08x\n",
                DRIVER_NAME,irq_handled,
                wrap_sta,int_status,sd_trans,sd_status);
/*
                wrap_sta     = cr_readl(EM_INT_STATUS);
                int_status   = cr_readl(EM_DWC_RINTSTS);
                int_status_m = cr_readl(EM_DWC_MINTSTS);
                sd_trans     = cr_readl(EM_DWC_IDSTS);
                sd_status    = cr_readl(EM_DWC_STATUS);
*/

        }
    }

    spin_unlock_irqrestore(&sdport->lock, flags);

    if(irq_handled)
        return IRQ_HANDLED;
    else
        return IRQ_NONE;

}

/************************************************************************
 * check read only func
 ************************************************************************/
static
int dw_em_get_ro(struct mmc_host *mmc)
{
#if 0
    struct rtksd_host *sdport = mmc_priv(mmc);
    struct mmc_card *card = mmc->card;
#endif

    return 0;
}

/************************************************************************
 *
 ************************************************************************/
static
int rtksd_switch_user_partition(struct mmc_card *card)
{
    struct mmc_command *cmd = NULL;
    struct sd_cmd_pkt *cmd_info = NULL;
    struct rtksd_host *sdport = NULL;
    int err = 0;

    sdport = mmc_priv(card->host);

    cmd = kmalloc(sizeof(struct mmc_command), GFP_KERNEL);
    if (!cmd) {
        EM_ALERT("No mem can be allocated\n");
        err = -1;
        goto ERR;
    }
    cmd_info = kmalloc(sizeof(struct sd_cmd_pkt), GFP_KERNEL);
    if (!cmd_info) {
        EM_ALERT("No mem can be allocated\n");
        err = -1;
        goto ERR;
    }
    sdport->cmd = cmd;

    memset(cmd, 0, sizeof(struct mmc_command));
    memset(cmd_info, 0, sizeof(struct sd_cmd_pkt));

    set_cmd_info(sdport,cmd,cmd_info,
             MMC_SWITCH, 0x03b30001, (MMC_CMD_AC | MMC_RSP_R1B));
    err = dw_em_SendCmd(cmd_info);
ERR:
    if(cmd)
        kfree(cmd);
    if(cmd_info)
        kfree(cmd_info);
    if(err){
        EM_WARNING( "%s: MMC_SWITCH fail\n", DRIVER_NAME);
    }
    return err;

}

/************************************************************************
 *
 ************************************************************************/
#ifdef CONFIG_MMC_RTKEMMC_HK_ATTR

#define NORMAL_PART 0
#define BOOT1_PART  1
#define BOOT2_PART  2
#define GP1_PART    3
#define GP2_PART    4
#define GP3_PART    5
#define GP4_PART    6

#if 0
static int rtksd_switch_partition(struct rtksd_host *sdport,u8 acc_part)
{
    struct mmc_command *cmd = NULL;
    struct sd_cmd_pkt *cmd_info = NULL;
    int err;

    cmd = kmalloc(sizeof(struct mmc_command), GFP_KERNEL);
    cmd_info = kmalloc(sizeof(struct sd_cmd_pkt), GFP_KERNEL);
    sdport->cmd = cmd;

    memset(cmd, 0, sizeof(struct mmc_command));
    memset(cmd_info, 0, sizeof(struct sd_cmd_pkt));

    /* set_cmd_info() */
    cmd->opcode         = MMC_SWITCH;
    cmd->arg            = 0x03b30001 | (acc_part << 8);
    cmd->flags          = (MMC_CMD_AC | MMC_RSP_R1B);
    cmd_info->cmd       = cmd;
    cmd_info->sdport    = sdport;
    sdport->cmd_info    = cmd_info;

    err = dw_em_SendCmd(cmd_info);

    kfree(cmd);
    kfree(cmd_info);

    if(err){
        EM_WARNING( "%s: MMC_SWITCH fail\n", DRIVER_NAME);
    }
    return err;

}
#endif

/************************************************************************
 *
 ************************************************************************/
static int rtksd_switch(struct mmc_card *card,
            u8 acc_mod, u8 index, u8 value, u8 cmd_set)
{
    struct mmc_command *cmd = NULL;
    struct sd_cmd_pkt *cmd_info = NULL;
    struct rtksd_host *sdport = NULL;
    u32 arg = 0;
    int err = 0;

    sdport = mmc_priv(card->host);

    cmd = kmalloc(sizeof(struct mmc_command), GFP_KERNEL);
    if (!cmd) {
        EM_ALERT("No mem can be allocated\n");
        err = -1;
        goto ERR;
    }
    cmd_info = kmalloc(sizeof(struct sd_cmd_pkt), GFP_KERNEL);
    if (!cmd_info) {
        EM_ALERT("No mem can be allocated\n");
        err = -1;
        goto ERR;
    }
    sdport->cmd = cmd;

    memset(cmd, 0, sizeof(struct mmc_command));
    memset(cmd_info, 0, sizeof(struct sd_cmd_pkt));

    arg = (acc_mod << 24) | (index << 16) | (value << 8) | (cmd_set);

    EM_INFO("arg=0x%08x\n", arg);
    set_cmd_info(sdport,cmd,cmd_info,
             MMC_SWITCH, arg, (MMC_CMD_AC | MMC_RSP_R1B));

    err = dw_em_SendCmd(cmd_info);
ERR:
    if(cmd)
        kfree(cmd);
    if(cmd_info)
        kfree(cmd_info);

    if(err){
        EM_WARNING( "%s: MMC_SWITCH fail\n", DRIVER_NAME);
    }
    return err;

}
#endif

/************************************************************************
 *
 ************************************************************************/
static
int dw_em_stop_transmission(struct rtksd_host *sdport)
{
    struct mmc_command *cmd = NULL;
    struct sd_cmd_pkt *cmd_info = NULL;
    int err = 0;

    cmd = kmalloc(sizeof(struct mmc_command), GFP_KERNEL);
    if (!cmd) {
        EM_ALERT("No mem can be allocated\n");
        err = -1;
        goto ERR;
    }
    cmd_info = kmalloc(sizeof(struct sd_cmd_pkt), GFP_KERNEL);
    if (!cmd_info) {
        EM_ALERT("No mem can be allocated\n");
        err = -1;
        goto ERR;
    }
    sdport->cmd = cmd;

    memset(cmd, 0, sizeof(struct mmc_command));
    memset(cmd_info, 0, sizeof(struct sd_cmd_pkt));
    /*
        cmd12 : R1 for read
                R1b for write
    */

    set_cmd_info(sdport,cmd,cmd_info,
             MMC_STOP_TRANSMISSION, 0x00, (MMC_RSP_R1B | MMC_CMD_AC));
    err = dw_em_SendCmd(cmd_info);
ERR:
    if(cmd)
        kfree(cmd);
    if(cmd_info)
        kfree(cmd_info);

    if(err){
        EM_WARNING( "%s: MMC_STOP_TRANSMISSION fail\n",
            DRIVER_NAME);
    }
    return err;

}

/************************************************************************
 *
 ************************************************************************/
static
void set_cmd_info(struct rtksd_host *sdport,struct mmc_command * cmd,
          struct sd_cmd_pkt *cmd_info, u32 opcode, u32 arg,
          u32 rsp_para)
{
    //memset(cmd, 0, sizeof(struct mmc_command));
    //memset(cmd_info, 0, sizeof(struct sd_cmd_pkt));

    cmd->opcode         = opcode;
    cmd->arg            = arg;
    cmd->flags          = rsp_para;
    cmd_info->cmd       = cmd;
    cmd_info->sdport    = sdport;
    sdport->cmd_info    = cmd_info;

}

/************************************************************************
 * For reference
 *
 * 4Bits Pattern
 * [000]=ff [001]=0f [002]=ff [003]=00 [004]=ff [005]=cc [006]=c3 [007]=cc
 * [008]=c3 [009]=3c [010]=cc [011]=ff [012]=fe [013]=ff [014]=fe [015]=ef
 * [016]=ff [017]=df [018]=ff [019]=dd [020]=ff [021]=fb [022]=ff [023]=fb
 * [024]=bf [025]=ff [026]=7f [027]=ff [028]=77 [029]=f7 [030]=bd [031]=ef
 * [032]=ff [033]=f0 [034]=ff [035]=f0 [036]=0f [037]=fc [038]=cc [039]=3c
 * [040]=cc [041]=33 [042]=cc [043]=cf [044]=ff [045]=ef [046]=ff [047]=ee
 * [048]=ff [049]=fd [050]=ff [051]=fd [052]=df [053]=ff [054]=bf [055]=ff
 * [056]=bb [057]=ff [058]=f7 [059]=ff [060]=f7 [061]=7f [062]=7b [063]=de
 *
 * =======================================================================
 * 8Bits Pattern
 * [000]=ff [001]=ff [002]=00 [003]=ff [004]=ff [005]=ff [006]=00 [007]=00
 * [008]=ff [009]=ff [010]=cc [011]=cc [012]=cc [013]=33 [014]=cc [015]=cc
 * [016]=cc [017]=33 [018]=33 [019]=cc [020]=cc [021]=cc [022]=ff [023]=ff
 * [024]=ff [025]=ee [026]=ff [027]=ff [028]=ff [029]=ee [030]=ee [031]=ff
 * [032]=ff [033]=ff [034]=dd [035]=ff [036]=ff [037]=ff [038]=dd [039]=dd
 * [040]=ff [041]=ff [042]=ff [043]=bb [044]=ff [045]=ff [046]=ff [047]=bb
 * [048]=bb [049]=ff [050]=ff [051]=ff [052]=77 [053]=ff [054]=ff [055]=ff
 * [056]=77 [057]=77 [058]=ff [059]=77 [060]=bb [061]=dd [062]=ee [063]=ff
 * [064]=ff [065]=ff [066]=ff [067]=00 [068]=ff [069]=ff [070]=ff [071]=00
 * [072]=00 [073]=ff [074]=ff [075]=cc [076]=cc [077]=cc [078]=33 [079]=cc
 * [080]=cc [081]=cc [082]=33 [083]=33 [084]=cc [085]=cc [086]=cc [087]=ff
 * [088]=ff [089]=ff [090]=ee [091]=ff [092]=ff [093]=ff [094]=ee [095]=ee
 * [096]=ff [097]=ff [098]=ff [099]=dd [100]=ff [101]=ff [102]=ff [103]=dd
 * [104]=dd [105]=ff [106]=ff [107]=ff [108]=bb [109]=ff [110]=ff [111]=ff
 * [112]=bb [113]=bb [114]=ff [115]=ff [116]=ff [117]=77 [118]=ff [119]=ff
 * [120]=ff [121]=77 [122]=77 [123]=ff [124]=77 [125]=bb [126]=dd [127]=ee
 *
 ************************************************************************/
#define TUNING_BLK_CNT      1
#define TUNING_WRITE_FLAG   BIT(31)
#define TUNING_SKIP_FLAG    BIT(30)
#define TUNING_HS400        BIT(29)
#define TUNING_DIR_UP       BIT(28)
#define TUNING_SKIP_ALL     BIT(27)
#define TUNING_BLK_SHT      6
#define TUNING_CMD_MASK     (0x3fUL)            /* bit0~bit5 */
#define TUNING_BLK_MASK     (0xfUL << TUNING_BLK_SHT)   /* bit6~bit9 */
#define TUNING_TPHASE_SHT   6
#define TUNING_TPHASE_MASK  (0x3fUL << TUNING_TPHASE_SHT)   /* bit6~bit11 */
#define GET_TUNING_CMD(opcode)  (opcode & TUNING_CMD_MASK)
#define GET_TUNING_BLK(opcode)  ((opcode & TUNING_BLK_MASK) >> TUNING_BLK_SHT)
#define GET_TPHASE(opcode)  ((opcode & TUNING_TPHASE_MASK) >> TUNING_TPHASE_SHT)
#define TUNING_BLK_ADR      0x80 /* At write case, this address should be careful to erase data on eMMC */

/* #define CHECK_PATTERN_SELF */
#ifdef CHECK_PATTERN_SELF
/*
keep these struct and function for debug if needed
*/
static const unsigned char pattern_blk[64] = {
    0xff, 0x0f, 0xff, 0x00, 0xff, 0xcc, 0xc3, 0xcc,
    0xc3, 0x3c, 0xcc, 0xff, 0xfe, 0xff, 0xfe, 0xef,
    0xff, 0xdf, 0xff, 0xdd, 0xff, 0xfb, 0xff, 0xfb,
    0xbf, 0xff, 0x7f, 0xff, 0x77, 0xf7, 0xbd, 0xef,
    0xff, 0xf0, 0xff, 0xf0, 0x0f, 0xfc, 0xcc, 0x3c,
    0xcc, 0x33, 0xcc, 0xcf, 0xff, 0xef, 0xff, 0xee,
    0xff, 0xfd, 0xff, 0xfd, 0xdf, 0xff, 0xbf, 0xff,
    0xbb, 0xff, 0xf7, 0xff, 0xf7, 0x7f, 0x7b, 0xde,

};

/************************************************************************
 *
 ************************************************************************/
static int dw_em_chk_pattern(struct rtksd_host *sdport,u8 *data)
{
    int i;
    int err = 0;

    if(sdport->cur_width == 8)
    {
        unsigned char nb1 = 0;
        unsigned char nb2 = 0;

        mmcmsg2("%dbits pattern check\n",sdport->cur_width);
        for(i=0; i<64; i++){
            nb1 = (pattern_blk[i] & 0xf0) >> 4;
            nb1 = (nb1<<4)|nb1;
            nb2 = (pattern_blk[i] & 0xf);
            nb2 = (nb2<<4)|nb2;

            if((data[i*2] != nb1) || (data[(i*2)+1] != nb2)){
                mmcmsg2("nb1=0x%x nb2=0x%x \n",nb1,nb2);
                mmcmsg2("data[%x]=0x%x data[%x]=0x%x \n",
                    i,data[i*2],i+1,data[(i*2)+1]);
                err = -1;
                break;
            }
        }

    } else {
        mmcmsg2("%dbits pattern check\n",sdport->cur_width);
        for(i=0; i<64; i++){
            if(data[i] != pattern_blk[i]){
                mmcmsg2("data[%x]=0x%x pattern_blk[%x]=0x%x \n",
                    i,data[i],i,pattern_blk[i]);
                err = -1;
                break;
            }
        }
    }

    return err;
}

#endif

/************************************************************************
 *
 ************************************************************************/
static void rtkem_show_tuning_result(struct rtksd_host *sdport, u8* p,char* reason)
{
    EM_INFO("%s\n"
    "        {00-31}[%d%d%d%d %d%d%d%d %d%d%d%d %d%d%d%d% d%d%d%d %d%d%d%d %d%d%d%d %d%d%d%d]\n",
        reason,
        p[ 0],p[ 1],p[ 2],p[ 3],p[ 4],p[ 5],p[ 6],p[ 7],
        p[ 8],p[ 9],p[10],p[11],p[12],p[13],p[14],p[15],
        p[16],p[17],p[18],p[19],p[20],p[21],p[22],p[23],
        p[24],p[25],p[26],p[27],p[28],p[29],p[30],p[31]
        );
}

/************************************************************************
 *
 ************************************************************************/
static
void dw_em_cal_max_phase_range(Phase_Arry p_arry,
                u32* phase_diff,u32* min_p, u32 expected)
{
    int i,j;
    u32 tmp_range = 0;
    u32 tmp_min = 0;
    u32 max_range = 0;
    u32 max_end = 0;
    u32 middle = 0;
    pr_alert("%s(%d)expected=%d\n",__func__,__LINE__,expected);

    if(expected == 0xff){
    	for (i=0; i<MAX_TUNING_STEP; i++) {
    		if (p_arry[i]){
    			tmp_range++;
    		}else{
    			tmp_range = 0;
    			continue;
    		}

    		if (tmp_range > max_range) {
    			max_range = tmp_range;
    			max_end = i;
    		}
    	}

    	*phase_diff = max_range;
    	*min_p = (max_end-max_range+1);
    }else{
        middle = expected;
        if (p_arry[middle]){
            pr_alert("%s(%d)expected=%d match\n",__func__,__LINE__,middle);
            /* p_arry[middle] can use, search more better phase */
            for (i = middle;i >= 0;i--){
                if (p_arry[i])
                    tmp_min = i;
                else
                    break;
            }
            for (j = middle;j < MAX_TUNING_STEP;j++){
                if (p_arry[j])
                    max_end = j;
                else
                    break;
            }
            max_range = max_end - tmp_min + 1;
        }else{
            /* p_arry[middle] can't use, find left side or right side */
            max_range = tmp_range = 0;
            for (i = middle - 1;i >= 0;i--){
                if (p_arry[i])
                    tmp_range++;
                else
                    break;
            }
            for (j = middle + 1;j < MAX_TUNING_STEP;j++){
                if (p_arry[j])
                    max_range++;
                else
                    break;
            }
            if (tmp_range > max_range){
                max_range = tmp_range;
                tmp_min = middle - max_range;
            }else{
                tmp_min = middle + 1;
            }
        }

        *phase_diff = max_range;
        *min_p = tmp_min;
    }
}

/************************************************************************
 *
 ************************************************************************/
#define MIN_RANGE_READ  8
#define MIN_RANGE_WRITE 4
static int dw_em_simple_tuning(struct mmc_host *mmc, u32 ctl)
{
    struct rtksd_host *sdport;
    int i;
    u32 tmp_phase = 0;
    u32 cur_phase = 0;
    u32 vaild_phase = 0;
    u32 phase_sht = 0;
    u32 phase_mask = 0;
    u32 min_p = 0;
    u32 max_p = 0;
    u32 phase_diff = 0;
    u32 tune_cmd = 0;
    u8 w_flag = 0;
    u8 p[64] = {0};
    int cmd_error = 0;
    int err = 0;
    u32 min_range = 0;

    sdport = mmc_priv(mmc);
    mmcmsg3( "cur bus width %d\n", sdport->cur_width );

    tune_cmd = GET_TUNING_CMD(ctl);

    if(ctl & TUNING_WRITE_FLAG)
        w_flag = 1;

    if(w_flag) {
        if(ctl & TUNING_HS400){
            EM_ALERT("%s(%d)HS400 tuning \n",__func__,__LINE__);
        }else{
            EM_ALERT("%s(%d)HS200 tuning \n",__func__,__LINE__);
        }
        if (tune_cmd == 24){
            phase_sht = PHASE_W_SHT;
            phase_mask = PHASE_W_MASK;
            vaild_phase = VAILD_WPHASE_ZONE;
            min_range = MIN_RANGE_WRITE;
        } else if (tune_cmd == 13){
            phase_sht = PHASE_C_SHT;
            phase_mask = PHASE_C_MASK;
            vaild_phase = VAILD_CPHASE_ZONE;
            /* We think command phase will have larger range */
            min_range = MIN_RANGE_READ;
        }
    } else {
        phase_sht = PHASE_R_SHT;
        phase_mask = PHASE_R_MASK;
        vaild_phase = VAILD_RPHASE_ZONE;
        min_range = MIN_RANGE_READ;
    }

    /* tuining start */
    if (tune_cmd == 13){
        mmcmsg3("PHASE_REG(0x%08x)=0x%08x\n",
                EMMC_PLL_PHASE_INTERPOLATION, cr_readl(EMMC_PLL_PHASE_INTERPOLATION));
        cur_phase = (cr_readl(EMMC_PLL_PHASE_INTERPOLATION)>>phase_sht) & phase_mask;
        mmcmsg3("initial cmd phase=%u(0x%x)\n",cur_phase, cur_phase);
    } else {
        mmcmsg3("PHASE_REG(0x%08x)=0x%08x\n",
                EMMC_PLLPHASE_CTRL, cr_readl(EMMC_PLLPHASE_CTRL));
        cur_phase = (cr_readl(EMMC_PLLPHASE_CTRL)>>phase_sht) & phase_mask;
        mmcmsg3("initial %s phase=%u(0x%x)\n",(w_flag)?"write":"read",cur_phase, cur_phase);
    }
#define SCAN_LOW_TO_HIGH
#ifdef  SCAN_LOW_TO_HIGH
    for (i=0; i < MAX_TUNING_STEP; i++) {
#else
    i = MAX_TUNING_STEP;
    while(i--){
#endif
        tmp_phase = i;
        mmcmsg3( "tmp %s phase is %u(0x%x)\n",(w_flag)?"write":"read",tmp_phase,tmp_phase);

        if(!(ctl & TUNING_SKIP_FLAG)){
            if (tune_cmd == 13)
                cr_maskl(EMMC_PLL_PHASE_INTERPOLATION,phase_mask,phase_sht ,tmp_phase);
            else
                cr_maskl(EMMC_PLLPHASE_CTRL,phase_mask,phase_sht ,tmp_phase);
        }

        if(w_flag && tune_cmd != 13)
            mdelay(2);

        if(dw_em_chk_phase_vaild(i,vaild_phase)){
            sdport->rtflags |= RTKCR_TUNING_STATE;
            err = rtkmmc_send_tuning(mmc, tune_cmd, &cmd_error);
            sdport->rtflags &= ~RTKCR_TUNING_STATE;
        }else{
            err = -1;
        }

        if(cmd_error)
            mmcmsg3("tuning cmd fail!(phase %d)\n",i);

        if (!err){
            p[i] = true;
        }
    }

    if(!(ctl & TUNING_SKIP_FLAG)){
        rtkem_show_tuning_result(sdport,p,"tuning result: ");
    }

    /* find longest good phase */
    dw_em_cal_max_phase_range(p,&phase_diff,&min_p, 0xff);

    /* We have used valid zones to filter phases, so don't restrict too much */
    if (phase_diff < 3) {
        err = -EIO;
        EM_NOTICE("good range too small,roll back to original phase %d(0x%x)\n",
            cur_phase,cur_phase);

        tmp_phase = cur_phase;
        if (tune_cmd == 13)
            cr_maskl(EMMC_PLL_PHASE_INTERPOLATION,phase_mask,phase_sht ,tmp_phase);
        else{
            cr_maskl(EMMC_PLLPHASE_CTRL,phase_mask,phase_sht ,tmp_phase);
            if(!w_flag)
                set_mmc_tuning_200_done(mmc,false);
        }
    }else{
        err = 0;

        if (ctl & TUNING_SKIP_FLAG) {
            /* restore origional phase */
            tmp_phase = cur_phase;
            if (tune_cmd == 13)
                cr_maskl(EMMC_PLL_PHASE_INTERPOLATION,phase_mask,phase_sht ,tmp_phase);
            else
                cr_maskl(EMMC_PLLPHASE_CTRL,phase_mask,phase_sht ,tmp_phase);
            EM_NOTICE("skip tuning!!!,keep original phase %d(0x%x)\n",
                cur_phase,cur_phase);
        }else{
            u32 ds_tune_tmp;

            max_p = min_p + phase_diff - 1;
            if(max_p >= MAX_TUNING_STEP)
                max_p -= MAX_TUNING_STEP;

            tmp_phase = min_p + (phase_diff/2) - 1;

            if(tmp_phase >= MAX_TUNING_STEP)
                tmp_phase -= MAX_TUNING_STEP;

            if (tune_cmd == 13)
                cr_maskl(EMMC_PLL_PHASE_INTERPOLATION,phase_mask,phase_sht ,tmp_phase);
            else
                cr_maskl(EMMC_PLLPHASE_CTRL,phase_mask,phase_sht ,tmp_phase);
            ds_tune_tmp = cr_readl(EM_HALF_CYCLE_CAL_RESULT) & 0x1f;

            EM_NOTICE("%s: max_p=%u min_p=%u diff=%u\n",
                DRIVER_NAME,max_p,min_p,phase_diff);
            EM_NOTICE("============================================\n");
            if (tune_cmd == 13){
                EM_NOTICE("%s: final PHASE(%s) is %u; 0x%08x = 0x%08x\n",
                        DRIVER_NAME,"CMD",tmp_phase,
                        EMMC_PLL_PHASE_INTERPOLATION,cr_readl(EMMC_PLL_PHASE_INTERPOLATION));
            }
            else {
                EM_NOTICE("%s: final PHASE(%s) is %u; 0x%08x = 0x%08x\n",
                        DRIVER_NAME,(w_flag)?"W":"R",tmp_phase,
                        EMMC_PLLPHASE_CTRL,cr_readl(EMMC_PLLPHASE_CTRL));
                EM_NOTICE("ds_tune=%u(show only)\n",ds_tune_tmp);
            }
            EM_NOTICE("============================================\n");

            if (tune_cmd == 13){
                /* We will use fix phase setting */
            } else if(w_flag){
                if(ctl & TUNING_HS400){
                    sdport->cur_w_ph400 = tmp_phase;
                    mmcmsg3("cur_w_ph400 = 0x%08x\n",sdport->cur_w_ph400);
                }else{
                    sdport->cur_w_phase = tmp_phase;
                    mmcmsg3("cur_w_phase = 0x%08x\n",sdport->cur_w_phase);
                }
            } else {
                sdport->cur_r_phase = tmp_phase;
                sdport->pre_ds_tune = ds_tune_tmp;
                set_mmc_tuning_200_done(mmc,true);
                sdport->tud_r_pha200 = tmp_phase;

                mmcmsg3("%s(%d) ==>\n"
                    "        tud_r_pha200 =0x%08x; pre_ds_tune=0x%08x\n",
                    __func__,__LINE__,
                    sdport->tud_r_pha200,sdport->pre_ds_tune);

            }
        }
    }

    return err;
}

/************************************************************************
 *
 ************************************************************************/
static void dw_em_set_ds_delay(struct rtksd_host *sdport, int ds_delay)
{
    unsigned int tmp_dstune;
    unsigned int tmp_dsmode;
    unsigned int init_ds;

    init_ds = sdport->cur_ds_tune;
    if(rtk_emmc_test_ctl & EMMC_FIX_DSTUNE){
        EM_ALERT("%s(%d)Use Bootcode pre-setting parameter\n",
            __func__,__LINE__);

        tmp_dsmode = ((sdport->mmc_dstune) & 0xffff0000) >> 16;
        tmp_dstune = sdport->mmc_dstune & 0xff;
        if(tmp_dsmode == 3){
            /* becauce ds_value is abslute value at mode 3. fix initial ds_value */
            init_ds = tmp_dstune;
        }

        EM_ALERT("%s(%d)mode%d %s %d\n",
                __func__,__LINE__,
                tmp_dsmode,
                (tmp_dsmode==1)?"---":
                (tmp_dsmode==2)?"+++":
                (tmp_dsmode==3)?"fix delay":"",
                tmp_dstune);

    }else{
        tmp_dsmode = 3;
        tmp_dstune = ds_delay;
    }

    cr_maskl(EM_HALF_CYCLE_CAL_EN, DS_CYCLE_FW_MODE_MASK, DS_CYCLE_FW_MODE_SHIFT,0);
    udelay(5);
    cr_maskl(EM_DS_TUNE_CTRL, DS_VALUE_MASK, DS_VALUE_SHIFT, init_ds);
    cr_maskl(EM_HALF_CYCLE_CAL_EN, DS_CAL_MODE_MASK, DS_CAL_MODE_SHIFT,tmp_dsmode);
    cr_maskl(EM_HALF_CYCLE_CAL_EN, DS_CAL_THR_MASK, DS_CAL_THR_SHIFT,tmp_dstune);
    cr_maskl(EM_DS_TUNE_CTRL,1,20 ,1); // maybe unused?
    cr_maskl(EM_HALF_CYCLE_CAL_EN, DS_CYCLE_FW_MODE_MASK, DS_CYCLE_FW_MODE_SHIFT ,1);
    cr_maskl(EM_HALF_CYCLE_CAL_EN, DS_CYCLE_CAL_EN_MASK, DS_CYCLE_CAL_EN_SHIFT ,1);
    udelay(5);
    cr_maskl(EM_HALF_CYCLE_CAL_EN, DS_CYCLE_FW_MODE_MASK, DS_CYCLE_FW_MODE_SHIFT ,0);
#if 0
    EM_ALERT("%s(%d)cur_ds_tune:%d\n" ,
        __func__, __LINE__,sdport->cur_ds_tune);
    EM_ALERT("0x%08x=0x%08x 0x%08x=0x%08x 0x%08x=0x%08x 0x%08x=0x%08x 0x%08x=0x%08x\n",
        EMMC_WRAP_half_cycle_cal_en_reg,(u32)cr_readl(EMMC_WRAP_half_cycle_cal_en_reg),
        EMMC_WRAP_half_cycle_cal_result_reg,(u32)cr_readl(EMMC_WRAP_half_cycle_cal_result_reg),
        EMMC_WRAP_ds_tune_ctrl_reg,(u32)cr_readl(EMMC_WRAP_ds_tune_ctrl_reg),
        PLL27X_REG_EMMC_PLLPHASE_CTRL_reg,(u32)cr_readl(PLL27X_REG_EMMC_PLLPHASE_CTRL_reg),
        PLL27X_REG_EMMC_PLL_PHASE_INTERPOLATION_reg,(u32)cr_readl(PLL27X_REG_EMMC_PLL_PHASE_INTERPOLATION_reg));
#endif
}

/************************************************************************
 *
 ************************************************************************/
static int dw_em_50M_rphase_tuning(struct mmc_host *mmc, u32 ctl)
{
    struct rtksd_host *sdport = mmc_priv(mmc);
    u16 i=0;
    int err=0;
    u32 max_range = 0;
    u32 min_rphase50 = 0;
    u32 rphase50_middle = 0;
    Phase_Arry result = {0};
    int cmd_error = 0;
    u32 tmp_phase = 0;
    u32 ds_tune_tmp;

    for (i=0; i < 32; i++) {
        if(dw_em_chk_phase_vaild(i,VAILD_RPHASE_ZONE)){
            cr_maskl(EMMC_PLLPHASE_CTRL, PHASE_R_MASK, PHASE_R_SHT, i);
            sdport->rtflags |= RTKCR_TUNING_STATE;
            err = rtkmmc_send_tuning(mmc, MMC_READ_SINGLE_BLOCK, &cmd_error);
            sdport->rtflags &= ~RTKCR_TUNING_STATE;

            if (!err)
                result[i] = 1;
        }
    }

    rtkem_show_tuning_result(sdport,result,"50M rphase tuning result: ");

    dw_em_cal_max_phase_range(result,&max_range,&min_rphase50, OPTIMAL_R_PHASE);

    rphase50_middle = (min_rphase50 + (max_range)/2)-1;

    if (max_range < 3) {
        EM_ERR("rphase50 capable range %d is small\n", max_range);
        tmp_phase = DEFAULT_R_PHASE;
        EM_NOTICE("good range too small,roll back to default phase %d(0x%x)\n",
            tmp_phase,tmp_phase);
        cr_maskl(EMMC_PLLPHASE_CTRL, PHASE_R_MASK, PHASE_R_SHT, tmp_phase);
        sdport->cur_r_pha50 = tmp_phase;
        goto ERR;
    } else {
        cr_maskl(EMMC_PLLPHASE_CTRL, PHASE_R_MASK, PHASE_R_SHT, rphase50_middle);
        EM_ERR("set rphase50 %d\n", rphase50_middle);
        sdport->cur_r_pha50 = rphase50_middle;
    }

    sdport->tud_r_pha50 = sdport->cur_r_pha50;
    ds_tune_tmp = cr_readl(EM_HALF_CYCLE_CAL_RESULT) & 0x1f;
    sdport->pre_ds_tune = ds_tune_tmp;

    return 0;
ERR:
    return -1;
}

/************************************************************************
 *
 ************************************************************************/
static int dw_em_ds_tuning(struct mmc_host *mmc, u32 ctl)
{
    struct rtksd_host *sdport = mmc_priv(mmc);
    u16 i=0, j=0;
    int err=0;
    u32 max_range = 0;
    u32 min_ds = 0;
    u32 ds_middle = 0;
    Phase_Arry result = {0};  //always prepare 32 step for MAX_DS_TUNE
    int cmd_error = 0;

    if(ctl & TUNING_SKIP_FLAG){
        EM_ALERT("%s(%d)cur_ds_tune:%d\n" ,
            __func__, __LINE__,sdport->cur_ds_tune);
        EM_ALERT("0x%08x=0x%08x 0x%08x=0x%08x 0x%08x=0x%08x 0x%08x=0x%08x 0x%08x=0x%08x\n",
            EMMC_WRAP_half_cycle_cal_en_reg,(u32)cr_readl(EMMC_WRAP_half_cycle_cal_en_reg),
            EMMC_WRAP_half_cycle_cal_result_reg,(u32)cr_readl(EMMC_WRAP_half_cycle_cal_result_reg),
            EMMC_WRAP_ds_tune_ctrl_reg,(u32)cr_readl(EMMC_WRAP_ds_tune_ctrl_reg),
            EMMC_PLLPHASE_CTRL,(u32)cr_readl(EMMC_PLLPHASE_CTRL),
            EMMC_PLL_PHASE_INTERPOLATION,(u32)cr_readl(EMMC_PLL_PHASE_INTERPOLATION));
        return 0;
    }

    for (i=0; i < MAX_DS_TUNE; i++) {
        if(dw_em_chk_phase_vaild(i,VAILD_DSTUNE_ZONE)){
            dw_em_set_ds_delay(sdport, i);
            sdport->rtflags |= RTKCR_TUNING_STATE;
            err = rtkmmc_send_tuning(mmc, MMC_READ_SINGLE_BLOCK, &cmd_error);
            sdport->rtflags &= ~RTKCR_TUNING_STATE;

            if (!err)
                result[i] = 1;
        }
    }

    rtkem_show_tuning_result(sdport,result,"DS tuning result: ");

    for (i=0; i < MAX_DS_TUNE; i++) {
        int circum = 0;

        for (j=i; j < i+MAX_DS_TUNE; j++) {
            int convert_j =  (j >= MAX_DS_TUNE)?
                (j-MAX_DS_TUNE) : j;
            if (result[convert_j])
                circum++;
            else
                break;
        }

        if (circum > max_range) {
            max_range = circum;
            min_ds = i;
        }
    }

    ds_middle = (min_ds + (max_range)/2)-1;

    if (max_range < 3) {
        EM_ERR("ds capable range %d is small\n", max_range);
        dw_em_set_ds_delay(sdport, sdport->cur_ds_tune);
        sdport->pre_ds_tune = sdport->cur_ds_tune;
        goto ERR;
    } else {
       dw_em_set_ds_delay(sdport, ds_middle);
       sdport->pre_ds_tune = sdport->cur_ds_tune = ds_middle;
       EM_ERR("set ds %d\n", ds_middle);
    }

    return 0;
ERR:
    return -1;
}

/************************************************************************
 *
 ************************************************************************/
static int dw_em_execute_tuning(struct mmc_host *mmc, u32 opcode)
{
    struct rtksd_host *sdport;
    int err = 0;
    u32 tune_ctl = 0;
    u32 pre_ctl = 0;

    sdport = mmc_priv(mmc);

    if(sdport->ins_event & (EVENT_SKIP_PHASETUNE|EVENT_SKIP_DSTUNING))
    {
        /* if STR operation, skip tuning */
        EM_NOTICE("%s(%d) skip tuning flow!!!\n",__func__,__LINE__);
        /* if tuning prior at resume case, skip tuning  */
        pre_ctl |= TUNING_SKIP_ALL;
    } else {
        pre_ctl &= ~TUNING_SKIP_ALL;
    }

    if(rtk_emmc_test_ctl & EMMC_SKIP_TUNE){
        EM_ALERT("%s(%d)Use Bootcode pre-setting parameter. Skip tuning\n",
            __func__,__LINE__);
        pre_ctl |= TUNING_SKIP_FLAG;
    }

    if( (opcode == 21)
     || (opcode == 18) )
    {
        /*
         opcode 21 is HS200 tuning state
         opcode 18 is HS400 tuning state
         */

        /* enable DS-tune has moved to prepare_hs400_tuning() */
        tune_ctl = pre_ctl;

        if(opcode == 18 &&(rtk_emmc_test_ctl & EMMC_FIX_DSTUNE)){
            /* skip ds tuning if fix dstune */
            tune_ctl |= TUNING_SKIP_FLAG;
        }else if((opcode == 21) &&(fix_phase_flag & FIX_RPHASE200)){
            /* skip hs200 tuning if fix rphase200 */
            tune_ctl |= TUNING_SKIP_FLAG;
        }

        if(tune_ctl & TUNING_SKIP_ALL){
            if(opcode == 18){
                /* ds reload"dw_em_set_ds_delay()" at dw_em_prepare_hs400_tuning() & set at dw_em_set_ios()   */
                sdport->ins_event &= ~EVENT_SKIP_DSTUNING;
                EM_NOTICE("%s(%d) skip ds tuning flow!!!\n", __func__, __LINE__);
                EM_NOTICE("%s(%d)set ds %d\n", __func__, __LINE__, sdport->cur_ds_tune);
            }else{
                /* w_phase;r_phase */
                /* r_phase set at dw_em_set_PLL_clk() */
                sdport->ins_event &= ~EVENT_SKIP_PHASETUNE;
                EM_INFO("%s(%d)restore r_phase=%d\n",
                    __func__,__LINE__,sdport->tud_r_pha200);
            }
            err = 0;
            goto ERR_OUT;
        }

        /* read tuning first at HS200 */
        if(opcode == 18){
            tune_ctl |= 17;
            err = dw_em_ds_tuning(mmc, tune_ctl);
        }else{
            tune_ctl |= 21;
            err = dw_em_simple_tuning(mmc, tune_ctl);
        }

        if(err){
            goto ERR_OUT;
        }

        if(rtk_emmc_test_ctl & EMMC_TUNE_WPH){
             EM_ALERT("%s(%d)Write phase tuning start\n",__func__,__LINE__);
            /* write tuning  */
            tune_ctl = pre_ctl;
            tune_ctl |= TUNING_WRITE_FLAG;

            if(opcode == 18){
                tune_ctl |= TUNING_HS400;
            }
            tune_ctl &= ~(TUNING_CMD_MASK);
            tune_ctl |= 24; // tuning cmd
            err = dw_em_simple_tuning(mmc, tune_ctl);

            if(err){
                goto ERR_OUT;
            }
        }

        if(rtk_emmc_test_ctl & EMMC_TUNE_WPH){
             EM_ALERT("%s(%d)Command phase tuning start\n",__func__,__LINE__);
            /* actually, do commad tuning  */
            tune_ctl = pre_ctl;
            tune_ctl |= TUNING_WRITE_FLAG;

            if(opcode == 18){
                tune_ctl |= TUNING_HS400;
            }
            tune_ctl &= ~(TUNING_CMD_MASK);
            tune_ctl |= 13; // tuning cmd, send status
            err = dw_em_simple_tuning(mmc, tune_ctl);

            if(err){
                goto ERR_OUT;
            }
        }

    }else{
        err = -1;
    }

    /* force downspeed */
    if (downspeed_test){
        /* Downspeed HS200 -> HS50 */
        /* Make HS400 tuning fail, Downspeed to HS200 */
        if (opcode == 18)
            return (-1);
        /* Make HS200 tuning fail, Downspeed to HS50 */
        if (opcode == 21){
            err = (-1);
        }
    }

    if(!(rtk_emmc_test_ctl & EMMC_DIS_50M_TUNE)){
        if(opcode == MMC_RPHASE50_TUNING)
            err = dw_em_50M_rphase_tuning(mmc, tune_ctl);
    }
ERR_OUT:
    if(!get_mmc_tuning_200_done(mmc)){
        if (HS200_fail_count > 3){
            if (sdport->mmc->caps2 & MMC_CAP2_HS200_1_8V_SDR){
                EM_ALERT("%s: Disable HS200 mode\n",mmc_hostname(sdport->mmc));
                sdport->mmc->caps2 &= (~MMC_CAP2_HS200_1_8V_SDR);
                sdport->mmc->card->mmc_avail_type &= ~EXT_CSD_CARD_TYPE_HS200;
            }
        }
        else {
            HS200_fail_count++;
            err = 0;
        }
    } else {
        HS200_fail_count = 0;
    }
    return err;

}

/************************************************************************
 *
 ************************************************************************/
static int mmc_send_data_cmd(struct mmc_host *mmc, u32 hc_cmd,
                 u32 cmd_arg, u32 blk_cnt, unsigned char *buffer)
{
    int err = 0;
    struct rtksd_host *sdport;
    unsigned long flags;

    if(mmc == NULL)
        mmc = mmc_host_local;

    sdport = mmc_priv(mmc);

    if(!(sdport->rtflags & RTKCR_TUNING_STATE)){
        EM_NOTICE( "cmd=%u, arg=0x%08x, size=0x%08x, buf=0x%p\n",
            hc_cmd,cmd_arg,blk_cnt,buffer);
    }

    if(mmc){
        struct mmc_card *card = mmc->card;
        struct mmc_request mrq = {0};
        struct mmc_command cmd = {0};
        struct mmc_data data   = {0};
        struct scatterlist sg;

        sdport = mmc_priv(mmc);
        mmc_claim_host(mmc);
        spin_lock_irqsave(&sdport->lock,flags);

        cmd.opcode = hc_cmd;
        cmd.arg = cmd_arg;
        cmd.flags = MMC_RSP_R1 | MMC_CMD_ADTC;

        if (blk_cnt) {
            data.sg = &sg;
            data.sg_len = 1;

            if(hc_cmd == 21){
                if(sdport->cur_width == 8){
                    /* 8bits 128bytes */
                    data.blksz = 128;
                }else if(sdport->cur_width ==4){
                    /* 4bits 64bytes */
                    data.blksz = 64;
                }else{
                    mmcmsg2( "not supported parameter");
                    spin_unlock_irqrestore(&sdport->lock, flags);
                    mmc_release_host(mmc);
                    goto ERR_OUT;
                }

            }else{
                data.blksz = 512;
            }
            data.blocks = blk_cnt;

            sg_init_one(data.sg, buffer, (blk_cnt * (data.blksz)));

            if( hc_cmd == MMC_WRITE_MULTIPLE_BLOCK ||
                hc_cmd == MMC_WRITE_BLOCK) {
                data.flags = MMC_DATA_WRITE;
            } else if (hc_cmd == MMC_SEND_EXT_CSD ||
                hc_cmd == MMC_READ_MULTIPLE_BLOCK ||
                hc_cmd == MMC_READ_SINGLE_BLOCK ||
                hc_cmd == MMC_SEND_TUNING_BLOCK_HS200) {
                data.flags = MMC_DATA_READ;
            }else{
                EM_ERR( "not supported command");
                spin_unlock_irqrestore(&sdport->lock, flags);
                mmc_release_host(mmc);
                goto ERR_OUT;
            }

            if(card){
                mmc_set_data_timeout(&data, card);
            }
        }

        data.mrq = &mrq;
        cmd.mrq = &mrq;
        cmd.data = &data;

        if(sdport->rtflags & RTKCR_TUNING_STATE)
            cmd.retries = 0;
        else
            cmd.retries = 3;

CMD_RETRY:
        mrq.data = &data;
        mrq.cmd = &cmd;

        if(sdport->mrq){
            sdport->mrq = NULL;
        }

        cmd.error = 0;
        data.error = 0;

        spin_unlock_irqrestore(&sdport->lock, flags);
        mmc_wait_for_req(mmc, &mrq);
        spin_lock_irqsave(&sdport->lock,flags);

        err = cmd.error;

        if(err && cmd.retries ){
            mmcmsg2( "%s(%u)last retry %d counter.\n",
                __func__,__LINE__,cmd.retries);
            cmd.retries--;
            goto CMD_RETRY;
        }
        spin_unlock_irqrestore(&sdport->lock, flags);
        mmc_release_host(mmc);

    }else{
        err = -ENODEV;
    }

ERR_OUT:
    if(err)
        mmcmsg2( "err=%d\n", err);
    return err;
}

/************************************************************************
 *
 ************************************************************************/
__attribute__((unused)) int export_mmc_send_data_cmd(u32 hc_cmd,
                 u32 cmd_arg, u32 blk_cnt, unsigned char *buffer)
{
    return mmc_send_data_cmd(NULL,hc_cmd,cmd_arg,blk_cnt,buffer);
}
EXPORT_SYMBOL(export_mmc_send_data_cmd);

#ifdef CONFIG_MMC_RTKEMMC_HK_ATTR
/************************************************************************
 *
 ************************************************************************/
static
void show_ext_csd(u8 *ext_csd)
{
    int i;
    for (i = 0; i < 512; i += 8) {
        EM_NOTICE(
               "[%03u]=%02x [%03u]=%02x [%03u]=%02x [%03u]=%02x "
               "[%03u]=%02x [%03u]=%02x [%03u]=%02x [%03u]=%02x\n",
               i, *(ext_csd + i), i + 1, *(ext_csd + i + 1), i + 2,
               *(ext_csd + i + 2), i + 3, *(ext_csd + i + 3), i + 4,
               *(ext_csd + i + 4), i + 5, *(ext_csd + i + 5), i + 6,
               *(ext_csd + i + 6), i + 7, *(ext_csd + i + 7));
    }
    EM_NOTICE( "\n");
}

/************************************************************************
 *
 ************************************************************************/
#ifdef  EN_SHOW_DATA
static
void show_data(u8 *buf, unsigned int len)
{
    int i;
    for (i = 0; i < len; i += 8) {
        EM_NOTICE(
               "[%03u]=%02x [%03u]=%02x [%03u]=%02x [%03u]=%02x "
               "[%03u]=%02x [%03u]=%02x [%03u]=%02x [%03u]=%02x\n",
               i, *(buf + i), i + 1, *(buf + i + 1), i + 2,
               *(buf + i + 2), i + 3, *(buf + i + 3), i + 4,
               *(buf + i + 4), i + 5, *(buf + i + 5), i + 6,
               *(buf + i + 6), i + 7, *(buf + i + 7));
    }
    EM_NOTICE( "\n");
}
#endif

/************************************************************************
 *
 ************************************************************************/
#define PF2_SHOW_EXT_CSD 0x01UL
#define PF2_FULL_PARAM   0x02UL
#define PF2_SET_EXT_CSD  0x04UL

/************************************************************************
 *
 ************************************************************************/
static void hk_set_wr_rel(struct device *dev,
              size_t p_count, unsigned char *cr_param)
{
    u8 *emmc_buf;
    u8 acc_mod;
    u8 index;
    u8 value;
    u8 i;
    u8 idx_lop;
    u8 cmd_set;
    u32 param1;
    u32 param2;
    u32 param3;
    u32 buf_size;
    struct mmc_host *host;
    struct mmc_card *card = NULL;

    EM_NOTICE( "%s(%u)2013/08/15 17:30\n", __func__, __LINE__);

    if( p_count != 38){
        EM_NOTICE( "Command format:\n"
               "    echo set_wr_rel=param1,param2,param3 > emmc_hacking;\n"
               "        param1[7:0] is value in byte you want to set.\n"
               "        param2 :\n"
               "            [bit0] TO show all ext_csd.\n"
               "            [bit1] Send CMD6. if this bit is set.\n"
               "                   param1 change to 32 bits parameter of argument of AMD6.\n"
               "        param3 must be 2379beef to make sure you want to do this.\n"
               "ex:\n"
               "    echo set_wr_rel=00000001,00000000,2379beef > emmc_hacking;\n"
               "        param1=0x00000001 enable user data area write reliability\n"
               "        param2=0x00020000 do not show all ext_csd.\n"
               "        param3=2379beef make sure that want to do this.\n"
               "\n"
               "    echo set_wr_rel=03a71f01,00000002,2379beef > emmc_hacking;\n"
               "        param1=0x03a71f01 set ext_cse[0xa7], value=0x1f, cmd_set=1\n"
               "        param2=0x00000002 send CMD6. param1 is argument in 32 bits.\n"
               "        param3=2379beef make sure that want to do this.\n" );
        goto ERR_OUT;
    }

    host = dev_get_drvdata(dev);
    if(host)
        card = host->card;

    /* KWarning: checked ok by alexkh@realtek.com */
    if(card == NULL){
        EM_NOTICE( "card is not exist.\n");
        goto ERR_OUT;
    }

    rtkcr_chk_param(&param1,8,cr_param);
    rtkcr_chk_param(&param2,8,cr_param+9);
    rtkcr_chk_param(&param3,8,cr_param+18);

    EM_NOTICE( "param1=0x%x\n", param1);
    EM_NOTICE( "param2=0x%x\n", param2);
    EM_NOTICE( "param3=0x%x\n", param3);

    if(param2 & PF2_FULL_PARAM){
        EM_NOTICE( "Send CMD6 alert\n");
        acc_mod = (u8)((param1>>24)&0xff);
        index   = (u8)((param1>>16)&0xff);
        value   = (u8)((param1>> 8)&0xff);
        cmd_set = (u8)(param1&0xff);
    }else{
        EM_NOTICE( "Enable Write Reliability\n");
        acc_mod = MMC_SWITCH_MODE_WRITE_BYTE;
        index   = EXT_CSD_WR_REL_SET;
        value   = (u8)param1;
        cmd_set = 1;
    }

    EM_NOTICE( "acc_mod=0x%x; index=%u; value=0x%x; cmd_set=0x%x\n",
            acc_mod,index,value,cmd_set);

    buf_size = 512;
    emmc_buf = kmalloc(buf_size, GFP_KERNEL);
    if(!emmc_buf){
        EM_NOTICE( "emmc_buf is NULL\n");
        goto ERR_OUT;
    }

    memset(emmc_buf, 0, 512);
    mmc_send_data_cmd(NULL, MMC_SEND_EXT_CSD, 0, 1, emmc_buf);

    EM_NOTICE( "[EXT_CSD] :\n");
    if(param2){
        if(param2 & PF2_SHOW_EXT_CSD)
            show_ext_csd(emmc_buf);
        if(param2 & PF2_FULL_PARAM){
            if( index==249 || index==242 ||
                index==212 || index==136 )
                idx_lop = 4;
            else if( index==157 || index==140)
                idx_lop = 3;
            else if( index==143)
                idx_lop = 12;
            else
                idx_lop = 1;

            for(i=0; i<idx_lop; i++)
                EM_NOTICE( "    [%03u]=%02x\n", index + i,
                       *(emmc_buf + index + i));
        }

    }else{
        EM_NOTICE( "    [%03u]=%02x [%03u]=%02x\n",
               166, *(emmc_buf + 166), 167, *(emmc_buf + 167));
    }

    if(index == EXT_CSD_WR_REL_SET){
        if((*(emmc_buf+EXT_CSD_WR_REL_PARAM) & 0x05) == 0x05 ){
            if(*(emmc_buf+EXT_CSD_PARTITION_SETTING_COMPLETED) & 0x01){
                EM_NOTICE( "This chip PARTITION configuration have completed\n");
                EM_NOTICE( "  ENH_SATRT_ADDR = 0x%08x\n",
                       (*(emmc_buf + EXT_CSD_ENH_START_ADDR + 3)
                    << 24) | (*(emmc_buf +
                            EXT_CSD_ENH_START_ADDR +
                            2) << 16) | (*(emmc_buf +
                                   EXT_CSD_ENH_START_ADDR
                                   +
                                   1) << 8) |
                          (*(emmc_buf+EXT_CSD_ENH_START_ADDR)));
                EM_NOTICE( "  ENH_SIZE_MULT  = 0x%06x\n",
                       (*(emmc_buf + EXT_CSD_ENH_SIZE_MULT + 2)
                    << 16) | (*(emmc_buf +
                            EXT_CSD_ENH_SIZE_MULT +
                            1) << 8) | (*(emmc_buf +
                                  EXT_CSD_ENH_SIZE_MULT)));
                EM_NOTICE( "  PARTITION_ATTRIBUTE  = 0x%06x\n",
                       (*
                    (emmc_buf +
                     EXT_CSD_PARTITION_ATTRIBUTE)));
                EM_NOTICE( "  WR_REL_SET  = 0x%x\n",
                        (*(emmc_buf+EXT_CSD_WR_REL_SET)));
                goto FINISH_OUT;
            }

        }else{
            EM_NOTICE( "Device not support setting write reliability\n");
            EM_NOTICE( "  WR_REL_PARAM = 0x%x\n",
                      *(emmc_buf+EXT_CSD_WR_REL_PARAM));
            goto FINISH_OUT;
        }
    }

    if(param3 == 0x2379beef)
        rtksd_switch(card,acc_mod,index,value,cmd_set);
    else{
        EM_NOTICE( "param3 != 0x2379beef skip command.\n");
        goto FINISH_OUT;
    }

    memset(emmc_buf, 0, 512);
    mmc_send_data_cmd(NULL, MMC_SEND_EXT_CSD, 0, 1, emmc_buf);

    EM_NOTICE( "{F} [EXT_CSD] :\n");
    if(param2){
        if(param2 & PF2_SHOW_EXT_CSD)
            show_ext_csd(emmc_buf);
        if(param2 & PF2_FULL_PARAM){
            if( index==249 || index==242 ||
                index==212 || index==136 )
                idx_lop = 4;
            else if( index==157 || index==140)
                idx_lop = 3;
            else if( index==143)
                idx_lop = 12;
            else
                idx_lop = 1;

            for(i=0; i<idx_lop; i++)
                EM_NOTICE( "    [%03u]=%02x\n", index + i,
                       *(emmc_buf + index + i));
        }
    }else{
        EM_NOTICE( "    [%03u]=%02x [%03u]=%02x\n",
               166, *(emmc_buf + 166), 167, *(emmc_buf + 167));
    }

FINISH_OUT:
    kfree(emmc_buf);

ERR_OUT:
    return;

}

/************************************************************************
 *
 ************************************************************************/
static void hk_red_ext_csd(struct device *dev, size_t p_count,
                           unsigned char *cr_param)
{
    u8 *emmc_buf;
    u32 param1;     /* target emmc address */
    u32 param2;     /* block number */
    u32 param3;     /* 1: write; 0: read */
    u32 buf_size ;
    struct mmc_host *host;
    struct mmc_card *card = NULL;

    EM_NOTICE( "%s(%u)2013/08/15 17:30\n", __func__, __LINE__);
    host = dev_get_drvdata(dev);
    if(host)
        card = host->card;

    /* KWarning: checked ok by alexkh@realtek.com */
    if(card == NULL){
        EM_INFO( "card is not exist.\n");
        goto ERR_OUT;
    }

    rtkcr_chk_param(&param1,8,cr_param);
    rtkcr_chk_param(&param2,8,cr_param+9);
    rtkcr_chk_param(&param3,8,cr_param+18);

    EM_NOTICE( "param1=0x%x\n", param1);  /* enh_start_addr */
    EM_NOTICE( "param2=0x%x\n", param2);  /* enh_block_cnt */
    EM_NOTICE( "param3=0x%x\n", param3);

    buf_size = 512;
    emmc_buf = kmalloc(buf_size, GFP_KERNEL);
    if(!emmc_buf){
        EM_NOTICE( "emmc_buf is NULL\n");
        goto ERR_OUT;
    }
    EM_NOTICE( "emmc_buf=0x%p\n", emmc_buf);

    memset(emmc_buf, 0, 512);
    mmc_send_data_cmd(NULL, MMC_SEND_EXT_CSD, 0, 1, emmc_buf);

    EM_NOTICE( "[EXT_CSD] :\n");

    if(param1 && !(param2 & PF2_SHOW_EXT_CSD)){
        u8 i;
        u8 item_cnt = 0;
        u8 item_cnt_old = 0;
        for(i=0;i<4;i++){
            /* every byte of param1 denote one extcsd address, parser here.
             * if the field is zero, use lasted address add one
             */
            item_cnt = param1>>(i*8);
            if(!item_cnt)
                item_cnt = (item_cnt_old+1);
            EM_NOTICE( "    [%03u]=%02x\n", item_cnt,
                   *(emmc_buf + item_cnt));
            item_cnt_old = item_cnt;
        }
        EM_NOTICE( "\n");
    }else{
        show_ext_csd(emmc_buf);
    }
    kfree(emmc_buf);

ERR_OUT:
    return;
}


/************************************************************************
 *
 ************************************************************************/
static void hk_rw_reg(struct device *dev,
        size_t p_count, unsigned char *cr_param)
{
    u32 param1;
    u32 param2;
    u32 param3;

    EM_NOTICE( "%s(%u)2015/10/19 16:25\n", __func__, __LINE__);

    rtkcr_chk_param(&param1,8,cr_param);
    rtkcr_chk_param(&param2,8,cr_param+9);
    rtkcr_chk_param(&param3,8,cr_param+18);

    if(p_count == 34){
        if(param1 == 0x2379beef){
            /* write reg */
            EM_NOTICE("REG[0x%08x] = 0x%08x(before)\n",param2,cr_readl(param2));
            cr_writel(param3, param2);
            EM_NOTICE("REG[0x%08x] = 0x%08x\n",param2,cr_readl(param2));
        }else if(param3 == 0x2379beef){
            /* read reg */
            EM_NOTICE("REG[0x%08x] = 0x%08x\n",param2,cr_readl(param2));
        }
    }else{
#if 0       /* Just for remind, do not to display user guide */
        EM_NOTICE( "Command format:\n"
               "    echo rw_reg=param1,param2,param3 > emmc_hacking;\n"
               "        param1: if 0xffffffff is write; if 0x00000000 is read.\n"
               "        param2: register address.\n"
               "        param3: if write case, it's writed value, or nothing.\n"
               "  ex:\n"
               "   write case:"
               "    echo rw_reg=2379beef,b8010900,12345678 > emmc_hacking;\n"
               "   read case:"
               "    echo rw_reg=00000000,b8010900,2379beef > emmc_hacking;\n" );
#endif
    }

    return;
}

#endif

/* ========================================================
 * blk_addr    : eMMC read/write target address, block base.
 * data_size   : tarnsfer data size, block base.
 * buffer      : DMA address
 * rw_mode     : fast read or fast write
 ========================================================== */
#define FAST_READ   0x1278
#define FAST_WRITE  0x3478
extern int export_get_card_status(struct mmc_card *card, u32 *status, int retries);
static int mmc_fast_rw( u32 blk_addr,
               u32 data_size, unsigned char *buffer, u32 rw_mode)
{
    int err = 0;
    struct mmc_host * mmc = mmc_host_local;

    if(mmc){
        struct rtksd_host *sdport;
        struct mmc_card *card;
        struct mmc_request mrq = {0};
        struct mmc_command cmd = {0};
        struct mmc_data data   = {0};
        struct scatterlist sg;

        sdport = mmc_priv(mmc);
        mmc_claim_host(mmc);
        card = mmc->card;

        if((sdport->rtflags & RTKCR_USER_PARTITION)==0){
            int try_loop = 3;
            do{
                err = 0;
                /* Switch to ser partition */
                err = mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
                        EXT_CSD_PART_CONFIG, 0,
                        card->ext_csd.part_time);
            }while(try_loop-- && err );
            if(err){
                mmc_release_host(mmc);
                goto ERR_OUT;
            } else
                sdport->rtflags |= RTKCR_USER_PARTITION;
        }

        if(data_size>1){

            if(rw_mode == FAST_WRITE)
                cmd.opcode = MMC_WRITE_MULTIPLE_BLOCK;
            else
                cmd.opcode = MMC_READ_MULTIPLE_BLOCK;

        }else{

            if(rw_mode == FAST_WRITE)
                cmd.opcode = MMC_WRITE_BLOCK;
            else
                cmd.opcode = MMC_READ_SINGLE_BLOCK;
        }

        cmd.arg = blk_addr;
        cmd.flags = MMC_RSP_R1 | MMC_CMD_ADTC;

        if (data_size) {    /* date info setting */
            data.sg = &sg;
            data.sg_len = 1;
            data.blksz = 512;
            data.blocks = data_size;

            sg_init_one(data.sg, buffer, (data_size<<9));

            if(rw_mode == FAST_WRITE)
                data.flags = MMC_DATA_WRITE;
            else
                data.flags = MMC_DATA_READ;

            mmc_set_data_timeout(&data, card);
        }

        data.mrq = &mrq;
        cmd.mrq = &mrq;
        cmd.data = &data;
        cmd.retries =5;

CMD_RETRY:
        mrq.data = &data;
        mrq.cmd = &cmd;

        if(sdport->mrq){
            sdport->mrq = NULL;
        }
        cmd.error = 0;
        data.error = 0;
        mmc_wait_for_req(mmc, &mrq);
        err = cmd.error;
        if(err && cmd.retries){
            EM_INFO("%s(%u)last retry %d counter.\n",
                    __func__,__LINE__,cmd.retries);
            cmd.retries--;
            goto CMD_RETRY;
        }
        mmc_release_host(mmc);
    }else{
        err = -ENODEV;
    }

ERR_OUT:
    if(err)
        EM_INFO("err=%d\n", err);
    return err;
}

/************************************************************************
 *
 ************************************************************************/
#define MAX_XFER_BLK      0x400
static int mmc_fast_rw_loop(u32 blk_addr,
                u32 data_size, unsigned char *buffer, u32 rw_mode)
{
    int err = 0;

    u32 tmp_addr   = blk_addr;
    u32 tmp_size   = data_size;
    u32 org_size   = data_size;
    unsigned char * tmp_buf = buffer;
    u32 max_xfer_blk;
    do{
        /* max 1M bytes read/write per transfer */

        max_xfer_blk = MAX_XFER_BLK;

        if(data_size > max_xfer_blk){
            tmp_size = max_xfer_blk;
        }else{
            tmp_size = data_size;
        }

        err = mmc_fast_rw(tmp_addr,tmp_size,tmp_buf,rw_mode);

        if(err)
            break;

        if(data_size > max_xfer_blk){
            tmp_addr    += max_xfer_blk;
            data_size   -= max_xfer_blk;
            tmp_buf     += (max_xfer_blk<<9);
        }else{
            data_size = 0;
        }

    }while(data_size);

    if(err == 0)
         return org_size-data_size;
    else
        return err;
}

/************************************************************************
 *
 ************************************************************************/
int mmc_fast_read(u32 blk_addr, u32 data_size, unsigned char *buffer)
{
    int err = 0;
    err = mmc_fast_rw_loop(blk_addr,data_size,buffer,FAST_READ);
    return err;
}

EXPORT_SYMBOL(mmc_fast_read);

/************************************************************************
 *
 ************************************************************************/
int mmc_fast_write(u32 blk_addr, u32 data_size, unsigned char *buffer)
{
    int err = 0;
    if(blk_addr<0x20000){
        EM_INFO("target small then save area.\n");
        err = -1;
        return err;
    }
    err = mmc_fast_rw_loop(blk_addr,data_size,buffer,FAST_WRITE);
    return err;
}

EXPORT_SYMBOL(mmc_fast_write);

/************************************************************************
 *
 ************************************************************************/
#ifndef CONFIG_MMC_RTKEMMC_PLUS_MODULE
void test_raw_emmc_partition_change(u32 mode)
{
    EM_ALERT( "%s(%d)\n",__func__,__LINE__);
    if(export_raw_emmc_partition_change(mode)){
        EM_ALERT( "%s(%d)change partition%u fail\n",
            __func__,__LINE__,mode);
    }
}
#endif
/************************************************************************
 *
 ************************************************************************/
#ifndef CONFIG_MMC_RTKEMMC_PLUS_MODULE
void test_raw_emmc_read(u64 blk_addr, u32 blk_cnt)
{
    u8 *emmc_buf = NULL;
    int err;

    EM_ALERT( "%s(%d)\n",__func__,__LINE__);

    emmc_buf = kmalloc(blk_cnt*512, GFP_KERNEL);
    if(!emmc_buf){
        EM_ALERT( "%s(%d)malloc buffer fail!!!\n",__func__,__LINE__);
    }else{
        err = export_raw_dw_em_read(emmc_buf, (u64)(blk_addr*512),(blk_cnt*512));

        if(err){
            EM_ALERT( "%s(%d) fail!!!\n",__func__,__LINE__);
        }
        kfree(emmc_buf);
    }
}
#endif
/************************************************************************
 *
 ************************************************************************/
#ifndef CONFIG_MMC_RTKEMMC_PLUS_MODULE
void test_raw_emmc_write(u64 blk_addr, u32 blk_cnt)
{
    u8 *emmc_buf = NULL;
    int err;

    EM_ALERT( "%s(%d)\n",__func__,__LINE__);

    emmc_buf = kmalloc(blk_cnt*512, GFP_KERNEL);
    if(!emmc_buf){
        EM_ALERT( "%s(%d)malloc buffer fail!!!\n",__func__,__LINE__);
    }else{
        err = export_raw_dw_em_write(emmc_buf, (u64)(blk_addr*512),(blk_cnt*512));

        if(err){
            EM_ALERT( "%s(%d) fail!!!\n",__func__,__LINE__);
        }
        kfree(emmc_buf);
    }
}
#endif
/************************************************************************
 *
 ************************************************************************/
#define MMC_PATTERN_1 0x00ff00ff
#define MMC_PATTERN_2 0x55aa55aa
#define MMC_PATTERN_3 0xff00ff00
#define MMC_PATTERN_4 0xaa55aa55

#ifndef CONFIG_MMC_RTKEMMC_PLUS_MODULE
static void mmc_fill_pattern(char *mmc_buf,int blk_cnt)
{
    int i;
    uint *ptr;

    /*
     change to 4 bytes, it's convenient to arrange pattern.
     Four types pattern to test.
     */
     ptr = (uint*)mmc_buf;
RE_FILL:
    blk_cnt -= 1;

    for(i=0;i< 128;i++){
        if(i<32)
            ptr[i] = MMC_PATTERN_1;
        else if(i<64)
            ptr[i] = MMC_PATTERN_2;
        else if(i<96)
            ptr[i] = MMC_PATTERN_3;
        else if(i<128)
            ptr[i] = MMC_PATTERN_4;
    }
    if(blk_cnt){
        ptr += 128;
        goto RE_FILL;
    }

}
#endif
/************************************************************************
 *
 ************************************************************************/
#ifndef CONFIG_MMC_RTKEMMC_PLUS_MODULE
static int mmc_chk_pattern(char *mmc_buf,int blk_cnt)
{
    int i;
    int ret;
    uint *ptr;

    /*
     change to 4 bytes, it's convenient to arrange pattern.
     Four types pattern to test.
     */
    ret = 0;
    ptr = (uint*)mmc_buf;

NEXT_BLK_CHK:
    blk_cnt -= 1;
    for(i=0;i< 128;i++){
        if(i<32){
            if(ptr[i] != MMC_PATTERN_1){
                ret = -1;
                break;
            }
        }
        else if(i<64){
            if(ptr[i] != MMC_PATTERN_2){
                ret = -1;
                break;
            }
        }
        else if(i<96){
            if(ptr[i] != MMC_PATTERN_3){
                ret = -1;
                break;
            }
        }
        else if(i<128){
            if(ptr[i] != MMC_PATTERN_4){
                ret = -1;
                break;
            }
        }
    }

    if(blk_cnt && !ret){
        ptr += 128;
        goto NEXT_BLK_CHK;
    }

    return ret;
}
#endif
/************************************************************************
 *
 ************************************************************************/
#ifndef CONFIG_MMC_RTKEMMC_PLUS_MODULE
static void mmc_show_blk(char *mmc_buf,int blk_cnt)
{
#ifdef DEBUG_PATTERN_EN

    int i;
    uint *ptr;

    ptr = (uint*)mmc_buf;

NEXT_BLK:
    blk_cnt -= 1;
    for(i=0;i<16;i++){
        printf("0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
               ptr[(i*8)+0],ptr[(i*8)+1],ptr[(i*8)+2],ptr[(i*8)+3],
               ptr[(i*8)+4],ptr[(i*8)+5],ptr[(i*8)+6],ptr[(i*8)+7]);
    }

    if(blk_cnt){
        ptr += 128;
        goto NEXT_BLK;
    }
#endif
}
#endif
/************************************************************************
 *
 ************************************************************************/
#ifndef CONFIG_MMC_RTKEMMC_PLUS_MODULE
void test_raw_emmc_compare(u64 blk_addr, u32 blk_cnt)
{
    u8 *emmc_buf = NULL;
    int err;

    EM_ALERT( "%s(%d)\n",__func__,__LINE__);

    emmc_buf = kmalloc(blk_cnt*512, GFP_KERNEL);
    if(!emmc_buf){
        EM_ALERT( "%s(%d)malloc buffer fail!!!\n",__func__,__LINE__);
    }else{
        memset(emmc_buf, 0, blk_cnt*0x200);
        mmc_fill_pattern(emmc_buf,blk_cnt);
        mmc_show_blk(emmc_buf,blk_cnt);
        err = export_raw_dw_em_write(emmc_buf, (u64)(blk_addr*512),(blk_cnt*512));

        if(err){
            EM_ALERT( "%s(%d) fail!!!\n",__func__,__LINE__);
            goto ERR_OUT;
        }

        memset(emmc_buf, 0, blk_cnt*0x200);
        mmc_show_blk(emmc_buf,blk_cnt);
        err = export_raw_dw_em_read(emmc_buf, (u64)(blk_addr*512),(blk_cnt*512));
        if(err){
            EM_ALERT( "%s(%d) fail!!!\n",__func__,__LINE__);
            goto ERR_OUT;
        }

        mmc_show_blk(emmc_buf,blk_cnt);
        err = mmc_chk_pattern(emmc_buf,blk_cnt);
        if (err){
            EM_ALERT( "%s(%d) compare fail\n",__func__,__LINE__);
        }

ERR_OUT:
        kfree(emmc_buf);
    }
}
#endif
/************************************************************************
 * Command Sample
 * //No use
 * echo raw_func=00000000,b800084c,00001000 > /sys/devices/platform/18010800.emmc/emmc_hacking
 * //change to user partition
 * echo raw_func=00000001,00000000,00001000 > /sys/devices/platform/18010800.emmc/emmc_hacking
 * //change to boot part 1
 * echo raw_func=00000001,00000001,00001000 > /sys/devices/platform/18010800.emmc/emmc_hacking
 * //change to boot part 2
 * echo raw_func=00000001,00000002,00001000 > /sys/devices/platform/18010800.emmc/emmc_hacking
 * //read from 0x1234 total 0x100blk
 * echo raw_func=00000002,00001234,00000100 > /sys/devices/platform/18010800.emmc/emmc_hacking
 * //write to 0x740000 total 0x100blk
 * echo raw_func=00000003,00740000,00000100 > /sys/devices/platform/18010800.emmc/emmc_hacking
 * //emmc init
 * echo raw_func=00000004,b800084c,00001000 > /sys/devices/platform/18010800.emmc/emmc_hacking
 * //emmc W/R compare use raw func. write to 0x740000 total 0x10blk then read back and compare.
 * echo raw_func=00000005,00740000,00000010 > /sys/devices/platform/18010800.emmc/emmc_hacking
 *
 ************************************************************************/
#ifndef CONFIG_MMC_RTKEMMC_PLUS_MODULE
static void hk_raw_func(struct device *dev, size_t p_count,
            unsigned char *cr_param)
{
    u32 param1;     /* 1:change partition; 2:raw_read; 3:raw_write */
    u32 param2;     /* partition number or start blk address for read/write */
    u32 param3;     /* block size of read/write  */

    EM_ALERT( "%s(%u)2016/05/04 16:05\n", __func__, __LINE__);

    rtkcr_chk_param(&param1,8,cr_param);
    rtkcr_chk_param(&param2,8,cr_param+9);
    rtkcr_chk_param(&param3,8,cr_param+18);

    EM_NOTICE( "param1=0x%x\n", param1);
    EM_NOTICE( "param2=0x%x\n", param2);
    EM_NOTICE( "param3=0x%x\n", param3);

    switch(param1){
        case 1: //raw_partition
            test_raw_emmc_partition_change(param2);
            break;
        case 2: //raw_read
            test_raw_emmc_read((u64)param2, param3);
            break;
        case 3: //raw_write
            test_raw_emmc_write((u64)param2, param3);
            break;
        case 4: //raw_init
            export_raw_dw_em_init();
            break;
        case 5: //W/R compare
            test_raw_emmc_compare((u64)param2, param3);
            break;
        default:
            EM_ALERT( "No support\n");
            return;
    }
    return;
}
#endif
/************************************************************************
 * mmc device attribute
 ************************************************************************/

/************************************************************************
 *
 ************************************************************************/
#define TEST_BLK_SIZE 10
static ssize_t
cr_fast_RW_dev_show(struct device *dev, struct device_attribute *attr,
        char *buf)
{
    struct mmc_host * host = dev_get_drvdata(dev);

    EM_INFO("%s(%u)\n", __func__, __LINE__);

    if(host && host->card){
        rtksd_switch_user_partition(host->card);
    }
    return sprintf(buf, "send SWITCH command\n");
}

static ssize_t
cr_fast_RW_dev_store(struct device *dev,
                     struct device_attribute *attr,
             const char *buf, size_t count)
{

    unsigned char *cr_param = NULL;

    EM_INFO("%s(%u)\n", __func__, __LINE__);
    EM_INFO("%s\n", buf);
    EM_INFO("count=%d\n", (int)count);

    cr_param=(char *)rtkcr_parse_token(buf,"cr_param");

    if(cr_param){
        u8 *emmc_buf;
        u32 param1; /* target emmc address */
        u32 param2; /* block number */
        u32 param3; /* 1: write; 0: read */

        rtkcr_chk_param(&param1,8,cr_param);
        rtkcr_chk_param(&param2,8,cr_param+9);
        rtkcr_chk_param(&param3,8,cr_param+18);
        EM_INFO("param1=0x%x param2=0x%x param3=0x%x\n",
                param1,param2,param3);

        emmc_buf = kmalloc(512*param2, GFP_KERNEL);
        if(!emmc_buf){
            EM_INFO("emmc_buf is NULL\n");
            goto ERR_OUT;
        }

        EM_INFO("emmc_buf=0x%p\n", emmc_buf);

        if(param3 == 1){
            mmc_fast_write(param1, param2, emmc_buf );
        }else{
            mmc_fast_read(param1, param2, emmc_buf );
        }
        kfree(emmc_buf);
    }else{
        EM_INFO("have no parameter searched.\n");
    }

ERR_OUT:
    /*
    return value must be equare or big then "count"
    to finish this attribute
    */
    if(cr_param)
        kfree(cr_param);
    return count;
}

DEVICE_ATTR(cr_fast_RW, S_IRUGO|S_IWUSR|S_IWGRP,
            cr_fast_RW_dev_show,cr_fast_RW_dev_store);

/************************************************************************
 *
 ************************************************************************/
static ssize_t
em_open_log_dev_show(struct device *dev, struct device_attribute *attr,
        char *buf)
{
    return sprintf(buf,
                "key in \"echo 1 > device_node\em_open_log\" to enable log\n");
}

static ssize_t
em_open_log_dev_store(struct device *dev,
        struct device_attribute *attr,
        const char *buf, size_t count)
{
    unsigned long tmp_long;
    struct mmc_host * host = dev_get_drvdata(dev);
    struct rtksd_host *sdport = mmc_priv(host);

    if (kstrtoul(buf, 10, &tmp_long)){
        return -EINVAL;
    }else{
        if(tmp_long == 0){
            sdport->rtflags &= ~RTKCR_FOPEN_LOG;
        }else if(tmp_long == 1){
            sdport->rtflags |= RTKCR_FOPEN_LOG;
        }else{
            return -EINVAL;
        }
    }

    return count;
}

DEVICE_ATTR(em_open_log, S_IRUGO|S_IWUSR|S_IWGRP,
              em_open_log_dev_show,em_open_log_dev_store);

/************************************************************************
 *
 ************************************************************************/
#ifdef CONFIG_MMC_RTKEMMC_HK_ATTR
static ssize_t
emmc_hacking_dev_show(struct device *dev, struct device_attribute *attr,
        char *buf)
{
    return sprintf(buf,
            "Supported hacking below:\n"
            "    set_wr_rel  : enable write reliability.\n"
            "    red_ext_csd : show ext_csd.\n"
            "    set_ehuser  : enable enhance user date area.\n");
}

static ssize_t
emmc_hacking_dev_store(struct device *dev,
                     struct device_attribute *attr,
               const char *buf, size_t count)
{
    unsigned char *cr_param = NULL;

    EM_INFO( "%s(%u)2013/08/15 17:30\n", __func__, __LINE__);
    EM_INFO( "count=%d\n", (int)count);

    cr_param=(char *)rtkcr_parse_token(buf,"set_wr_rel");
    if(cr_param){
        hk_set_wr_rel(dev,count,cr_param);
        kfree(cr_param);
        goto FINISH_OUT;
    }

    cr_param=(char *)rtkcr_parse_token(buf,"red_ext_csd");
    if(cr_param){
        hk_red_ext_csd(dev,count,cr_param);
        kfree(cr_param);
        goto FINISH_OUT;
    }

    cr_param=(char *)rtkcr_parse_token(buf,"rw_reg");
    if(cr_param){
        hk_rw_reg(dev,count,cr_param);
        kfree(cr_param);
        goto FINISH_OUT;
    }
#ifndef CONFIG_MMC_RTKEMMC_PLUS_MODULE
    cr_param=(char *)rtkcr_parse_token(buf,"raw_func");
    if(cr_param){
        hk_raw_func(dev,count,cr_param);
        kfree(cr_param);
        goto FINISH_OUT;
    }
#endif
    EM_INFO("have no match command!!\n");

FINISH_OUT:
    /*
    return value must be equare or big then "count"
    to finish this attribute
    */
    return count;
}

DEVICE_ATTR(emmc_hacking, S_IRUGO|S_IWUSR|S_IWGRP,
            emmc_hacking_dev_show,emmc_hacking_dev_store);

#endif

/************************************************************************
 *
 ************************************************************************/
static ssize_t
emmc_errcnt_dev_show(struct device *dev, struct device_attribute *attr,
        char *buf)
{

    return sprintf(buf, "%s: eMMC have had %lu error since boot\n",
        DRIVER_NAME, rtk_emmc_err_cnt);
}

static ssize_t
emmc_errcnt_dev_store(struct device *dev,
                     struct device_attribute *attr,
              const char *buf, size_t count)
{
    unsigned long tmp_long;

    if (kstrtoul(buf, 10, &tmp_long)){
        return -EINVAL;
    }else{
        if(tmp_long == 0){
            rtk_emmc_err_cnt = 0;
            EM_INFO("%s(%u)reset error count to %u.\n",
                __func__, __LINE__,(unsigned int)rtk_emmc_err_cnt);
        }else{
            EM_INFO("%s(%u)No thing to do.\n",
                __func__, __LINE__);
        }
    }

    return count;
}

DEVICE_ATTR(emmc_errcnt, S_IRUGO|S_IWUSR|S_IWGRP,
            emmc_errcnt_dev_show, emmc_errcnt_dev_store);

static ssize_t
phase_tune_dev_show(struct device *dev, struct device_attribute *attr,
        char *buf)
{

    dw_em_show_phase();

    return sprintf(buf, "%s: display by kernel, make sure log level above 4.\n",DRIVER_NAME);
}

static ssize_t
phase_tune_dev_store(struct device *dev, struct device_attribute *attr,
              const char *buf, size_t count)
{
    u32 w_phase,r_phase,c_phase,ds_tune,ds_mode;
    u32 reg1,reg2,reg3,reg4,reg5;
    u32 local_ctl = 0;
    long tmp_val = 0;
    bool phase_test = false;
    char *input = NULL;
    char *opt = NULL;
    struct mmc_host * host = dev_get_drvdata(dev);
    struct rtksd_host *sdport = mmc_priv(host);

    w_phase = r_phase = c_phase = ds_tune = ds_mode = 0xff;

    input = kmalloc(count, GFP_KERNEL);
    if (!input)
        return -ENOMEM;

    strcpy(input, buf);
    downspeed_test = false;

    while ((opt= strsep(&input, " ")) != NULL) {
        if (!*opt)
            continue;

        if((tuning_phase.ctl & PHASE_SAVED)
        || (tuning_phase.ctl & PHASE_FORCE)){
            if (!strncmp(opt, "w_phase=", 8)) {
                if (!kstrtol(opt + 8, 10, (long*)&tmp_val)) {
                    if (tmp_val < MAX_TUNING_STEP){
                        w_phase = (u32)tmp_val;
                        SET_FIX_WPHA400(w_phase,rtk_fix_wphase);
                        SET_FIX_WPHA200(w_phase,rtk_fix_wphase);
                        tuning_phase.ctl |= PHASE_CHANGED;
                        local_ctl |= PHASE_CHANGED;

                        EM_ALERT("w_phase=%d\n",w_phase);
                    }else{
                        EM_ALERT("unacceptable w_phase value!\n");
                    }
                }
            }

            if (!strncmp(opt, "r_phase=", 8)) {
                if(!kstrtol(opt + 8, 10, (long*)&tmp_val)) {
                    if (tmp_val < MAX_TUNING_STEP){
                        r_phase = (u32)tmp_val;
                        tuning_phase.ctl |= PHASE_CHANGED;
                        local_ctl |= PHASE_CHANGED;
                        EM_ALERT("r_phase=%u\n",r_phase);
                    }else{
                        EM_ALERT("unacceptable r_phase value!\n");
                    }
                }
            }

            if (!strncmp(opt, "c_phase=", 8)) {
                if (!kstrtol(opt + 8, 10, (long*)&tmp_val)) {
                    if (tmp_val < MAX_TUNING_STEP){
                        c_phase = (u32)tmp_val;
                        tuning_phase.ctl |= PHASE_CHANGED;
                        local_ctl |= PHASE_CHANGED;
                        SET_FIX_CPHA200(c_phase,rtk_fix_wphase);
                        EM_ALERT("c_phase=%u\n",c_phase);
                    }else{
                        EM_ALERT("unacceptable c_phase value!\n");
                    }
                }
            }

            if (!strncmp(opt, "ds_tune=", 8)) {
                if (!kstrtol(opt + 8, 10, (long*)&tmp_val)) {
                    if (tmp_val < MAX_TUNING_STEP){
                        ds_tune = (u32)tmp_val;
                        if(ds_mode == 0xff )
                            ds_mode = tuning_phase.ds_mode_old;
                        tuning_phase.ctl |= DSTUNE_CHANGED;
                        local_ctl |= DSTUNE_CHANGED;
                        EM_ALERT("ds_tune=%u\n",ds_tune);
                    }else{
                        EM_ALERT("unacceptable ds_tune value!\n");
                    }
                }
            }

            if (!strncmp(opt, "phase_test", 10)) {
                EM_ALERT("phase_test enabled\n");
                phase_test = true;
            }

            if (!strncmp(opt, "downspeed", 9)) {
                EM_ALERT("downspeed enabled\n");
                downspeed_test = true;
            }

            if (!strncmp(opt, "ds_mode=", 8)) {
                if (!kstrtol(opt + 8, 10, (long*)&tmp_val)) {
                    if (tmp_val < 4){
                        ds_mode = (u32)tmp_val;
                        EM_ALERT("ds_mode=%u\n",ds_mode);
                    }else{
                        EM_ALERT("unacceptable ds_mode value!\n");
                    }
                }
            }

            if (!strncmp(opt, "restore", 7)) {
                EM_ALERT("restore phase parameter\n");
                EM_ALERT("!!! release Phase scan mode!!! \n");

                tuning_phase.ctl &= ~(PHASE_CHANGED|DSTUNE_CHANGED|PHASE_SAVED);

                rtk_fix_wphase = tuning_phase.fix_wphase_old;
                dw_em_init_phase(sdport,
                    tuning_phase.wphase_old,
                    tuning_phase.rphase_old,
                    tuning_phase.cphase_old);

                ds_mode = tuning_phase.ds_mode_old;
                ds_tune = tuning_phase.ds_tune_old;
                sdport->mmc_dstune = ds_tune|(ds_mode << 16);
                rtk_emmc_test_ctl |= EMMC_FIX_DSTUNE;
                dw_em_set_ds_delay(sdport,0);

                rtk_emmc_test_ctl = tuning_phase.test_ctl_old;
                sdport->mmc_dstune = tuning_phase.fix_dstune_old;
                goto FINISH_OUT;
            }

            if (!strncmp(opt, "disforce=", 8)) {
                tuning_phase.ctl &= ~PHASE_FORCE;
            }

        }else{
            if (!strncmp(opt, "save", 4)) {
                EM_ALERT("save phase parameter\n");
                EM_ALERT("!!! Phase scan mode alert. It's going to skip retrying mechanism!!! \n");

                memset(&tuning_phase,0,sizeof(mmc_phase_t));

                reg1 = (u32)cr_readl(EMMC_WRAP_half_cycle_cal_en_reg);
                reg2 = (u32)cr_readl(EMMC_WRAP_half_cycle_cal_result_reg);
                reg3 = (u32)cr_readl(EMMC_WRAP_ds_tune_ctrl_reg);
                reg4 = (u32)cr_readl(EMMC_PLLPHASE_CTRL);
                reg5 = (u32)cr_readl(EMMC_PLL_PHASE_INTERPOLATION);
                tuning_phase.wphase_old     = (reg4&PHASE_W_MASK);
                tuning_phase.rphase_old     = ((reg4>>PHASE_R_SHT)&PHASE_R_MASK);
                tuning_phase.cphase_old     = ((reg5>>PHASE_C_SHT)&PHASE_C_MASK);
                tuning_phase.ds_mode_old    = ((reg1>>DS_CAL_MODE_SHIFT)&DS_CAL_MODE_MASK);
                tuning_phase.ds_tune_old    = ((reg1>>DS_CAL_THR_SHIFT)&DS_CAL_THR_MASK);
                tuning_phase.ds_result_old  = (reg2&DS_CAL_RES_MASK);

                tuning_phase.test_ctl_old   = rtk_emmc_test_ctl;
                tuning_phase.fix_wphase_old = rtk_fix_wphase;
                tuning_phase.fix_dstune_old = sdport->mmc_dstune;

                tuning_phase.ctl = PHASE_SAVED;

                EM_ALERT("store wphase=%u rwphase=%u cphase=%u ds_mode=%u ds_tune=%u ds_result=%u\n",
                    tuning_phase.wphase_old,tuning_phase.rphase_old,tuning_phase.cphase_old,
                    tuning_phase.ds_mode_old,tuning_phase.ds_tune_old,tuning_phase.ds_result_old);
            }else if(!strncmp(opt, "force", 5)){
                EM_ALERT("set force\n");
                tuning_phase.ctl = PHASE_FORCE;
                if(input)
                    kfree(input);
                return count;
            } else {
                EM_ALERT("Please save phase first!\n");
                if(input)
                    kfree(input);
                return count;
            }
        }
    }

    if (local_ctl & PHASE_CHANGED) {
        EM_ALERT("change w_phase=%u r_phase=%u c_phase=%u\n",
            w_phase,r_phase,c_phase);
        if (phase_test){
            EM_ALERT("phase_test, will not lock phase\n");
            rtk_emmc_test_ctl &= ~EMMC_FIX_WPHASE;
            rtk_emmc_test_ctl |= EMMC_TUNE_WPH;
        }
        else{
            rtk_emmc_test_ctl |= EMMC_FIX_WPHASE;
            rtk_emmc_test_ctl &= ~EMMC_TUNE_WPH;
        }
        dw_em_init_phase(sdport,w_phase,r_phase,c_phase);
    }

    if (local_ctl & DSTUNE_CHANGED) {
        EM_ALERT("change dstune=%u dsmode=%u\n",ds_tune,ds_mode);

        if (phase_test){
            EM_ALERT("phase_test, will not lock phase\n");
            rtk_emmc_test_ctl &= ~EMMC_FIX_DSTUNE;
            dw_em_set_ds_delay(sdport,ds_tune);
        }
        else{
            rtk_emmc_test_ctl |= EMMC_FIX_DSTUNE;
            sdport->mmc_dstune = ds_tune|(ds_mode << 16);
            ds_tune = sdport->mmc_dstune;    //backup mmc_dstune
            dw_em_set_ds_delay(sdport,0);
            sdport->mmc_dstune = ds_tune;    //restore mmc_dstune
        }
    }

FINISH_OUT:
    dw_em_show_phase();

    if(input)
    	kfree(input);

    return count;
}

DEVICE_ATTR(phase_tune, S_IRUGO|S_IWUSR|S_IWGRP,
            phase_tune_dev_show, phase_tune_dev_store);

static ssize_t
em_chk_mode_dev_show(struct device *dev, struct device_attribute *attr,
            char *buf)
{
    struct mmc_host * host = dev_get_drvdata(dev);
    struct mmc_card *card = NULL;
    int type;
    struct rtksd_host *sdport = NULL;
    int  n = 0;
    int  count = PAGE_SIZE;
    char *ptr = buf;

    static const char *const card_type_tbl[5] = {
        "MMC ",
        "SD ",
        "SDIO ",
        "SD_COMBO ",
        "Unknow "
    };

    rtkcr_display_version();
    dw_em_show_phase();
    EM_ALERT("0x%08x=0x%08x 0x%08x=0x%08x 0x%08x=0x%08x\n",
            RTK_IOPAD_CFG1,cr_readl(RTK_IOPAD_CFG1),
            RTK_IOPAD_CFG2,cr_readl(RTK_IOPAD_CFG2),
            RTK_IOPAD_CFG3,cr_readl(RTK_IOPAD_CFG3));
    EM_ALERT("0x%08x=0x%08x 0x%08x=0x%08x 0x%08x=0x%08x 0x%08x=0x%08x\n",
            RTK_IOPAD_SET1,cr_readl(RTK_IOPAD_SET1),
            RTK_IOPAD_SET2,cr_readl(RTK_IOPAD_SET2),
            RTK_IOPAD_SET3,cr_readl(RTK_IOPAD_SET3),
            RTK_IOPAD_SET4,cr_readl(RTK_IOPAD_SET4));

    if(host && host->card){
        sdport = mmc_priv(host);
        card = host->card;
        type = card->type;
        if(type >3)
            type = 4;
        n = snprintf(ptr, count,"%s: new %s%s%s%s card(%dHz)[%dbit] at address %04x\n",
            mmc_hostname(host),
            mmc_card_uhs(host->card) ? "ultra high speed " :
            (mmc_card_hs(host->card) ? "high speed " : ""),
            mmc_card_hs400(host->card) ? "HS400 " :
            (mmc_card_hs200(host->card) ? "HS200 " : ""),
            mmc_card_ddr52(host->card) ? "DDR " : "",
            card_type_tbl[type],
            sdport->cur_clock,
            sdport->cur_width,
            host->card->rca);
        ptr+=n; count-=n;

        if (sdport->int_mode){
            n = snprintf(ptr, count,"ISR mode enabled\n");
            ptr+=n; count-=n;
        }
        if (sdport->rtflags & RTKCR_FTIMER_CHECK){
            n = snprintf(ptr, count,"Max Timeout Report: cmd%d total_byte:0x%08x max_timeout = %lldus\n",
                cmd_record, total_byte_record,atomic64_read(&cmd_time_ns_old)/1000);
            ptr+=n; count-=n;
        }
    }

    return ptr - buf;;
}

static ssize_t
em_chk_mode_dev_store(struct device *dev,
            struct device_attribute *attr,
            const char *buf, size_t count)
{
    EM_ALERT("%s(%u)\n", __func__, __LINE__);
    EM_ALERT("%s\n", buf);
    EM_ALERT("count=%d\n", (int)count);

    /*
    return value must be equare or big then "count"
    to finish this attribute
    */
    return count;
}

DEVICE_ATTR(em_chk_mode, S_IRUGO|S_IWUSR|S_IWGRP,
        em_chk_mode_dev_show,em_chk_mode_dev_store);


/************************************************************************
 *
 ************************************************************************/
#ifdef RTK_CMD23_USE
static ssize_t
em_predefined_show(struct device *dev, struct device_attribute *attr,
            char *buf)
{
    return sprintf(buf, "Predefined Enable: %u\n", predefined_read);
}

static ssize_t
em_predefined_dev_store(struct device *dev,
            struct device_attribute *attr,
            const char *buf, size_t count)
{
    EM_ALERT("input (%s)\n", buf);
    if (buf[0] == '1')
        predefined_read = 1;
    else
        predefined_read = 0;
    return count;
}

DEVICE_ATTR(em_predefined_read, S_IRUGO|S_IWUSR|S_IWGRP,
              em_predefined_show,em_predefined_dev_store);
#endif

/************************************************************************
 *
 ************************************************************************/
static
int rtkemmc_boot_type_getting(void)
{
    int err = 0;
    unsigned char *cr_param = NULL;

    cr_param =
        (char *)rtkcr_parse_token(platform_info.system_parameters,
                      "boot_flash");

    if(cr_param == NULL){
        EM_INFO
            ("[Warning]Can't get boot type!eMMC driver init continue....\n");
    }else{
        if(memcmp(cr_param,"emmc",strlen(cr_param))!=0){
            EM_INFO("Boottype %s!eMMC flash driver init exit....\n",
                   cr_param);
            err = -1;
        }else{
            EM_INFO("Boottype %s!eMMC driver init continue....\n",
                   cr_param);
        }
    }

    if(cr_param)
        kfree(cr_param);

    return err;

}

/************************************************************************
 * Bootcode command sample:
 *
 * env set mmc_param_1 pl200=00000001,53,0d,ff,07,0052,011e,0bba3 pl190=00000000,50,0d,ff,07,0000,0000,00000
 * env set mmc_param_2 pl180=00000000,4d,0d,ff,07,0000,0000,00000 pl170=00000000,48,0d,ff,03,0000,0000,00000
 * env set mmc_param_3 pl110=00000000,2d,23,ff,03,0000,0000,00000 pl100=00000000,29,25,ff,03,0000,0000,00000
 * env set mmc_param_4 pl050=00000000,13,0c,ff,01,0000,0000,00000
 * env set mmc_param_5 dstune=09
 * env set mmc_param_6 mmc_drv_io=73e9e9e9,fbfbfbfb,fbfbfbfb,80000000
 * env set mmc_param_7 mmc_drv=00040100
 *
 * env set bootargs emmc_bus=003C0001 emmc_clock=160000000 androidboot.console=ttyS0 console=ttyS0,115200
 *
 * Note : max mmc_param_NUM NUM=12
 ************************************************************************/

static __attribute__((unused))
int rtkemmc_parse_pad_drv(struct rtksd_host * rtk_priv, char *drv_str)
{
    int err = -1;
    char *tmp;
    char *tmp_drv_str = drv_str;
    long driving = 0;
    int i=0;

    if (!rtk_priv)
        return err;

    rtk_priv->mmc_io_drv = kmalloc(sizeof(u32)*5, GFP_KERNEL); // 5 param at most
    if (rtk_priv->mmc_io_drv == NULL) {
        EM_ALERT("No mem can be allocated\n");
        return err;
    }

    for (i=0; i<5; i++) {
        tmp = strsep(&tmp_drv_str, ",");
        if (tmp == NULL || tmp[0] == '\0')
            continue;
        if (!kstrtol(tmp, 16, (long*)&driving))
            rtk_priv->mmc_io_drv[i] = (u32)driving;
    }

    EM_NOTICE("\n");
    for (i=0; i<5; i++)
        EM_NOTICE("mmc_io_drv[%d] = 0x%08x, ", i, rtk_priv->mmc_io_drv[i]);
    EM_NOTICE("\n");

    return 0;
}

static int rtkemmc_parse_mmc_param(struct rtksd_host *sdport)
{
    char *opt = NULL;
    char *platform_mmc_param = NULL;

    long dstune = 0;
    long tphase = 0;
    long card_drv = 0;
    long burst_blk = 0;
    char *str_tmp;

    if (strlen(platform_info.mmc_param) == 0) {
        EM_INFO("=== NO MMC PARAMETER in bootloader ===\n");
        return 0;
    }

    if (!sdport) {
        EM_ERR("%s rtkemmc initial error\n", __FUNCTION__);
        return -1;
    }

    /* log func stste */
    if(rtk_emmc_log_state){
        sdport->log_state = rtk_emmc_log_state;
        EM_ALERT("log_state is %u\n",sdport->log_state);
    }

    EM_NOTICE("=== mmc_param is \"%s\" ===\n", platform_info.mmc_param);

    platform_mmc_param = kmalloc(MMC_PARAM_LEN,GFP_KERNEL);
    if(platform_mmc_param == NULL)
        return 0;

    memcpy(platform_mmc_param,platform_info.mmc_param,MMC_PARAM_LEN);

    while ((opt= strsep(&platform_mmc_param, " ")) != NULL) {
        if (!*opt)
            continue;

        if (!strncmp(opt, "emmc_debug_log", 14)) {
            rtk_emmc_debug_log = 1;
        }
        else if (!strncmp(opt, "emmc_open_log", 13)) {
            sdport->rtflags |= RTKCR_FOPEN_LOG;
        }
        else if (!strncmp(opt, "timeout_chk", 11)) {
            sdport->rtflags |= RTKCR_FTIMER_CHECK;
        }
        else  if (!strncmp(opt, "emmc_crc", 8)) {
            rtk_emmc_test_ctl |= EMMC_SHOW_CRC;
        }
        else if (!strncmp(opt, "emmc_width=", 11)) {
            long bus_wid = 0;
            if (!kstrtol(opt + 11, 10, (long*)&bus_wid))
            {
                switch (bus_wid)
                {
                case 1:
                    sdport->mmc->caps &= ~MMC_CAP_8_BIT_DATA;
                    sdport->mmc->caps &= ~MMC_CAP_4_BIT_DATA;
                    break;
                case 4:
                    sdport->mmc->caps &= ~MMC_CAP_8_BIT_DATA;
                    sdport->mmc->caps |= MMC_CAP_4_BIT_DATA;
                    break;
                default:
                    sdport->mmc->caps |= MMC_CAP_8_BIT_DATA;
                    sdport->mmc->caps |= MMC_CAP_4_BIT_DATA;
                }
            }
        }
        else if (!strncmp(opt, "emmc_clock=", 11)) {
            long bus_clock = 0;
            if (!kstrtol(opt + 11, 10, (long*)&bus_clock)){

                EM_NOTICE("emmc_clock %ld\n", bus_clock);
                // check clock > 200MHz?
                if (bus_clock > 200000000)
                    bus_clock = 200000000;

                sdport->mmc->f_max = bus_clock;
            }
        }
        else if (!strncmp(opt, "dstune=", 7)) {
            if (kstrtol(opt + 7, 10, (long*)&dstune)){
                dstune = 0;
            } else {
                sdport->mmc_dstune = dstune;
                EM_NOTICE("dstune %ld\n", dstune);
                rtk_emmc_test_ctl |= EMMC_FIX_DSTUNE;
            }
        }
        else if (!strncmp(opt, "dsmode=", 7)) {
            if (!kstrtol(opt + 7, 10, (long*)&dstune)){
                sdport->mmc_dstune |= (dstune << 16);
                EM_NOTICE("dsmode=%ld\n", dstune);
            }
        }
        else if (!strncmp(opt, "pad_drv=", 8)) {
            str_tmp = strsep(&opt, "=");
            EM_NOTICE("\ndebug {str_tmp %s} {opt %s} \n", str_tmp, opt);
            if(!rtkemmc_parse_pad_drv(sdport, opt))
                rtk_emmc_test_ctl |= EMMC_EN_IO_DRV;
        }
        else if (!strncmp(opt, "card_drv=", 9)) {
            if (kstrtol(opt + 9, 10, (long*)&card_drv))
                card_drv = 0;
            else {
                if(card_drv <= 4){  // check card_drv 0x0 ~ 0x4
                    sdport->mmc_drv = card_drv;
                    rtk_emmc_test_ctl |= EMMC_EN_MMC_DRV;
                    EM_NOTICE("card_drv 0x%lx\n", sdport->mmc_drv);
                }else
                    EM_NOTICE("unacceptable card_drv 0x%lx\n", card_drv);
            }
        }
        else if (!strncmp(opt, "burst_blk=", 10)) {
            if (kstrtol(opt + 10, 10, (long*)&burst_blk))
                burst_blk = 0;
            else {
                EM_NOTICE("burst_blk %ld\n", burst_blk);
                sdport->mmc->max_blk_count = burst_blk;
            }
        }
        else if (!strncmp(opt, "emmc_dis_sscg", 13)) {
            rtk_emmc_test_ctl |= EMMC_DIS_SSCG;
        }
        else if (!strncmp(opt, "emmc_always_clk", 15)) {
            rtk_emmc_test_ctl |= EMMC_ALWAYS_CLK;
        }
        else if (!strncmp(opt, "en_tune_wphase", 10)) {
            if(rtk_emmc_test_ctl & EMMC_FIX_WPHASE)
                EM_NOTICE("fix write phase enable; skip phase tuning\n");
            else
                rtk_emmc_test_ctl |= EMMC_TUNE_WPH;
        }
        else if (!strncmp(opt, "wphase400=", 10)) {
            tphase = 0;
            if (!kstrtol(opt + 10, 10, (long*)&tphase)){
                rtk_emmc_test_ctl |= EMMC_FIX_WPHASE;
                rtk_emmc_test_ctl &= ~EMMC_TUNE_WPH;
                fix_phase_flag |= FIX_WPHASE400;
                SET_FIX_WPHA400(tphase,rtk_fix_wphase);
                EM_NOTICE("get fix HS400 write phase %ld.(0x%08x)\n",
                        tphase,rtk_fix_wphase);
            }
        }
        else if (!strncmp(opt, "wphase200=", 10)) {
            tphase = 0;
            if (!kstrtol(opt + 10, 10, (long*)&tphase)){
                rtk_emmc_test_ctl |= EMMC_FIX_WPHASE;
                rtk_emmc_test_ctl &= ~EMMC_TUNE_WPH;
                fix_phase_flag |= FIX_WPHASE200;
                SET_FIX_WPHA200(tphase,rtk_fix_wphase);
                EM_NOTICE("get fix HS200 write phase %ld.(0x%08x)\n",
                        tphase,rtk_fix_wphase);
            }
        }
        else if (!strncmp(opt, "rphase400=", 10)) {
            tphase = 0;
            if (!kstrtol(opt + 10, 10, (long*)&tphase)){
                fix_phase_flag |= FIX_RPHASE400;
                SET_FIX_RPHA400(tphase);
                EM_NOTICE("get fix HS400 cmd read phase %ld.(0x%08x/0x%08x)\n",
                        tphase,rtk_fix_rphase,GET_FIX_RPHA400());
            }
        }
        else if (!strncmp(opt, "rphase200=", 10)) {
            tphase = 0;
            if (!kstrtol(opt + 10, 10, (long*)&tphase)){
                fix_phase_flag |= FIX_RPHASE200;
                SET_FIX_RPHA200(tphase);
                EM_NOTICE("get fix HS200 read phase %ld.(0x%08x/0x%08x)\n",
                        tphase,rtk_fix_rphase,GET_FIX_RPHA200());

            }
        }
        else if (!strncmp(opt, "cphase200=", 10)) {
            tphase = 0;
            if (!kstrtol(opt + 10, 10, (long*)&tphase)){
                rtk_emmc_test_ctl |= EMMC_FIX_WPHASE;
                rtk_emmc_test_ctl &= ~EMMC_TUNE_WPH;
                fix_phase_flag |= FIX_CPHASE200;
                SET_FIX_CPHA200(tphase,rtk_fix_wphase);
                EM_NOTICE("get fix HS200 cmd phase %ld.(0x%08x)\n",
                        tphase,rtk_fix_wphase);
            }
        }
        else if (!strncmp(opt, "wphase50=", 9)) {
            tphase = 0;
            if (!kstrtol(opt + 9, 10, (long*)&tphase)){
                fix_phase_flag |= FIX_WPHASE50;
                SET_FIX_WPHA50(tphase,rtk_fix_50M_phase);
                EM_NOTICE("get fix HS50 wphase phase %ld.(0x%08x)\n",
                        tphase,rtk_fix_wphase);
            }
        }
        else if (!strncmp(opt, "rphase50=", 9)) {
            tphase = 0;
            if (!kstrtol(opt + 9, 10, (long*)&tphase)){
                fix_phase_flag |= FIX_RPHASE50;
                SET_FIX_RPHA50(tphase,rtk_fix_50M_phase);
                EM_NOTICE("get fix HS50 read phase %ld.(0x%08x)\n",
                        tphase,rtk_fix_wphase);
            }
        }
        else if (!strncmp(opt, "dis_50m_tune", 12)) {
            rtk_emmc_test_ctl |= EMMC_DIS_50M_TUNE;
        }
        else if (!strncmp(opt, "en_phase_tool", 13)) {
            rtk_emmc_test_ctl |= EMMC_EN_SCAN_TOOL;
        }
        else if (!strncmp(opt, "emmc_en_hs50", 13)){
            sdport->mmc->caps2 &= ~(MMC_CAP2_HS200_1_8V_SDR
                    | MMC_CAP2_HS400_1_8V);
            EM_ALERT("bootcode select HS50 mode\n");
        }
        else if (!strncmp(opt, "emmc_en_hs200", 13)){
            sdport->mmc->caps2 &= ~(MMC_CAP2_HS200_1_8V_SDR
                    | MMC_CAP2_HS400_1_8V);
            sdport->mmc->caps2 |= MMC_CAP2_HS200_1_8V_SDR;
            EM_ALERT("bootcode select HS200 mode\n");
        }
        else if (!strncmp(opt, "emmc_en_hs400", 13)) {
            sdport->mmc->caps2 &= ~(MMC_CAP2_HS200_1_8V_SDR
                    | MMC_CAP2_HS400_1_8V);
            sdport->mmc->caps2 |= (MMC_CAP2_HS200_1_8V_SDR
                        | MMC_CAP2_HS400_1_8V);
            EM_ALERT("bootcode select HS400 mode\n");
            HS400_fix_test = true;
        }
        else if (!strncmp(opt, "emmc_skip_tune", 14)) {
            rtk_emmc_test_ctl |= EMMC_SKIP_TUNE;
        }
        else if (!strncmp(opt, "emmc_chk_buf", 12)) {
            EM_NOTICE("%s(%d)\n",__func__,__LINE__);
            rtk_emmc_test_ctl |= EMMC_CHK_BUF;
        }
        else if (!strncmp(opt, "drv_strength=", sizeof("drv_strength=")-1)) {
            long dstrength = 0;
            if (!kstrtol(opt + sizeof("drv_strength=")-1, 10, (long*)&dstrength)){
                rtk_emmc_test_ctl |= EMMC_FIX_STRENGTH;
                SET_FIX_STRENGTH(dstrength,rtk_fix_wphase);
                EM_NOTICE("get drv_strength=%ld.\n",
                        dstrength);
            }
        }
        else if (!strncmp(opt, "emmclog=", 8)) {
            /*
            For customer request, valid value from 0~15(0xf).
            1. without emmclog or emmclog=0, disable message function.
            2. emmclog=1, enable message for this request.
            3. emmclog=other_value , for future request.
             */
            long temp = 0;
            if (kstrtol(opt + 8, 10, (long*)&temp))
                rtk_emmc_log_state = 0;
            else {
                sdport->log_state = rtk_emmc_log_state = (u32)temp;
                EM_ERR("log_state is %u\n",sdport->log_state);
            }
        }
    }

    if(platform_mmc_param)
        kfree(platform_mmc_param);

    EM_NOTICE("\n");
    return 0;

}

static
struct rtk_host_ops emmc_ops = {
    .func_irq       = NULL,
    .re_init_proc   = NULL,
    .card_det       = NULL,
    .card_power     = NULL,
    .chk_card_insert= rtkem_chk_card_insert,
    .set_crt_muxpad = NULL,
    .set_clk        = dw_em_set_clk,
    .set_bits       = dw_em_set_bits,
    .set_DDR        = dw_em_set_DDR,
    .reset_card     = dw_em_reset_card,
    .reset_host     = dw_em_reset_host,
    .set_IO_driving = dw_em_set_IO_driving,
    .bus_speed_down = dw_em_bus_speed_down,
    .get_cmdcode    = NULL,
    .get_r1_type    = NULL,
    .chk_cmdcode    = NULL,
    .chk_r1_type    = NULL,
    .hold_card      = rtkem_hold_card,
};

static
int rtkemmc_disable_int(struct rtksd_host * sdport)
{
    /* enable SCPU int */
    cr_writel(EMMC_SINT_MASK, EMMC_SCPU_INT);
    /* enable KCPU int */
    //cr_writel(EMMC_KINT_MASK, EMMC_KCPU_INT);
    /* clear mask */
    cr_writel( 0x0, EM_DWC_INTMASK);
    cr_writel( 0x0, EM_DWC_CTRL );
    /* clear pending bit */
    cr_writel(0xffffffff, EM_DWC_IDSTS);
    cr_writel( 0xffffffff,EM_DWC_RINTSTS);
    cr_writel( BIT(2) | BIT(1) | BIT(0),EM_INT_STATUS);
    udelay(1);

    return 0;
}

static
int rtkemmc_enable_int(struct rtksd_host * sdport)
{
    u32 reginfo = 0;
    /* int mask setting */
    if(sdport->int_mode){
        reginfo = ( SDMMC_INT_CMD_DONE |
                    SDMMC_INT_DATA_OVER |
                    //SDMMC_INT_TXDR |	/* use iDMAC skip this */
                    //SDMMC_INT_RXDR |	/* use iDMAC skip this */
                    SDMMC_INT_ERROR);
    }else{
        reginfo = 0;
    }
    /* clear pending bit */
    cr_writel( BIT(2) | BIT(1) | BIT(0),EM_INT_STATUS);
    cr_writel(0xffffffff, EM_DWC_IDSTS);
    cr_writel( 0xffffffff,EM_DWC_RINTSTS);
    /* Enable IP Interrupt*/
    cr_writel(reginfo, EM_DWC_INTMASK);
    if(sdport->int_mode)
        reginfo = SDMMC_CTRL_INT_ENABLE;	/* enable global int */
    cr_writel(reginfo, EM_DWC_CTRL);
    /* enable SCPU int */
    cr_writel( EMMC_SINT_MASK|BIT(0),EMMC_SCPU_INT);
    /* enable KCPU int */
    //cr_writel( EMMC_KINT_MASK|BIT(0),EMMC_KCPU_INT);
    udelay(1);

    return 0;
}

/************************************************************************
 *
 ************************************************************************/
static
int __init rtkemmc_probe(struct platform_device *pdev)
{
    struct mmc_host *mmc = NULL;
    struct rtksd_host *sdport = NULL;
    struct resource *r;
    int ret;
    int irq;
    int att_err;

    atomic64_set( &cmd_time_ns_old, 0 );
    memset(&tuning_phase,0,sizeof(mmc_phase_t));

    /* check if emmc BOOT */
    if(rtkemmc_boot_type_getting()){
        EM_NOTICE("%s: It's not eMMC boot type!\n", DRIVER_NAME);
        return -ENXIO;
    }
    att_err = device_create_file(&pdev->dev, &dev_attr_cr_fast_RW);
    att_err = device_create_file(&pdev->dev, &dev_attr_em_open_log);
    att_err = device_create_file(&pdev->dev, &dev_attr_em_chk_mode);
    att_err = device_create_file(&pdev->dev, &dev_attr_emmc_errcnt);
    att_err = device_create_file(&pdev->dev, &dev_attr_phase_tune);
#ifdef CONFIG_MMC_RTKEMMC_HK_ATTR
    att_err = device_create_file(&pdev->dev, &dev_attr_emmc_hacking);
#endif
#ifdef RTK_CMD23_USE
    att_err = device_create_file(&pdev->dev, &dev_attr_em_predefined_read);
#endif

    r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    irq = platform_get_irq(pdev, 0);
    EM_INFO("%s: IRQ %u\n", DRIVER_NAME, irq);

    if (!r || irq < 0)
        return -ENXIO;

    mmc = mmc_alloc_host(sizeof(struct rtksd_host), &pdev->dev);

    if (!mmc) {
        ret = -ENOMEM;
        goto out;
    }

    mmc_host_local = mmc;

    sdport = mmc_priv(mmc);
    memset(sdport, 0, sizeof(struct rtksd_host));

    sdport->mmc = mmc;
    sdport->dev = &pdev->dev;
    sdport->res = r;
    sdport->ops = &emmc_ops;
    /* Full polling or Trigger ISR */
    sdport->int_mode = 1;
    sdport->base = devm_ioremap_resource(&pdev->dev, sdport->res);
    if (IS_ERR(sdport->base)) {
        ret = PTR_ERR(sdport->base);
        goto out;
    }

    /* Need to check : DMA 64 or 32 bits*/
    /* Right now device-tree probed devices don't get dma_mask set.
         * we check here
         */
    if (!pdev->dev.dma_mask)
        pdev->dev.dma_mask = &pdev->dev.coherent_dma_mask;
    if (!pdev->dev.coherent_dma_mask)
        pdev->dev.coherent_dma_mask = DMA_BIT_MASK(32);

    mmc->ocr_avail = MMC_VDD_30_31 | MMC_VDD_31_32
        | MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;

    mmc->caps = MMC_CAP_HW_RESET
        | MMC_CAP_4_BIT_DATA
        | MMC_CAP_8_BIT_DATA
        | MMC_CAP_SD_HIGHSPEED
        | MMC_CAP_MMC_HIGHSPEED
        | MMC_CAP_NONREMOVABLE
        | MMC_CAP_CMD23
        | MMC_CAP_WAIT_WHILE_BUSY
            ;

    mmc->caps2 = MMC_CAP2_NO_SDIO
                | MMC_CAP2_NO_SD
                | MMC_CAP2_FULL_PWR_CYCLE

#ifdef EN_EMMC_HS200
        | MMC_CAP2_HS200_1_8V_SDR
#endif

#ifdef EN_EMMC_HS400
        | MMC_CAP2_HS400_1_8V
#endif
            ;
#if defined(CONFIG_RTKEMMC_HS200_MODULE) || defined(CONFIG_RTKEMMC_HS400_MODULE) \
       ||  defined(CONFIG_RTKEMMC_HS200) || defined(CONFIG_RTKEMMC_HS400)
        /* use config setting */
        mmc->f_max = (CONFIG_RTKEMMC_HS200_CLOCK*1000000);
#else
        mmc->f_max = CLK_50Mhz;    /* RTK max bus clk is 50Mhz */
#endif
    /* check mmc parameter from bootcode  */
    ret = rtkemmc_parse_mmc_param(sdport);
    if (ret) {
        EM_ERR("parse mmc parameter fail\n");
    }
    mmc->f_min = CLK_200Khz;        /* follow lowest clock 100KHz according spec, but we set to 200KHz*/


    mmc->max_segs = (EMMC_SCRIPT_BUF_SIZE) / 16;
    mmc->max_blk_size = 65535; // Supports block size of 1 to 65,535 bytes
    mmc->max_seg_size = DW_MCI_DESC_DATA_LENGTH;
    mmc->max_req_size = mmc->max_seg_size * mmc->max_segs;
    mmc->max_blk_count = mmc->max_req_size / 512;

    spin_lock_init(&sdport->lock);
#ifndef RTK_EMMC_ISR_DIRECT
    tasklet_init(&sdport->req_end_tasklet, rtkcr_req_end_tasklet,
                (unsigned long)sdport);
#endif
    tasklet_init(&sdport->polling_tasklet, rtkem_polling_tasklet,
                (unsigned long)sdport);

    if(sdport->int_mode){
        ret = request_irq(irq, dw_em_irq, IRQF_SHARED, DRIVER_NAME, sdport);
        if (ret) {
            EM_ERR( "%s: cannot assign irq %d\n", DRIVER_NAME, irq);
            goto out;
        } else{
            sdport->irq = irq;
        }
    }

    dw_em_chk_shoutdown_protect(sdport);
    timer_setup(&sdport->timer, rtksd_timeout_timer, 0);

    rtkem_clr_sta(sdport);
    sdport->ops->chk_card_insert(sdport);

    /*
     * The scripter are 16 bytes per line,
     * So we need Scripter POOL size is 4096(EMMC_MAX_SCRIPT_LINE*16)
     */
    sdport->scrip_pool = dma_pool_create(DRIVER_NAME, sdport->dev,
                         (EMMC_SCRIPT_BUF_SIZE), 4096,
                         4096);
    if(!(sdport->scrip_pool)){
        EM_INFO( "%s: request Scripter Pool faill!\n",
               DRIVER_NAME);
        ret = -ENOMEM;
        goto out;
    }

    sdport->scrip_buf = dma_pool_alloc(sdport->scrip_pool,GFP_KERNEL ,
                     &sdport->scrip_buf_phy);
    if(!(sdport->scrip_buf)){
        EM_INFO( "%s: request Scripter Buffer fail!\n",
               DRIVER_NAME);
        dma_pool_destroy(sdport->scrip_pool);
        ret = -ENOMEM;
        goto out;
    }
    if (dw_em_idmac_init(sdport)){
        EM_ERR("%s: Unable to initialize DMA Controller.\n",
                DRIVER_NAME);
        if(sdport->scrip_buf)
            dma_pool_free(sdport->scrip_pool, sdport->scrip_buf,
                         sdport->scrip_buf_phy);
        if(sdport->scrip_pool)
            dma_pool_destroy(sdport->scrip_pool);
        ret = -ENOMEM;
        goto out;
    }
    dw_em_set_scrip_base(sdport->scrip_buf_phy);

    platform_set_drvdata(pdev, mmc);

    ret = mmc_add_host(mmc);
    if (ret){
        goto out;
    }

#ifdef CONFIG_CUSTOMER_TV006
    device_enable_async_suspend(&pdev->dev);
#endif

    if(sdport->int_mode)
        rtkemmc_enable_int(sdport);

    EM_NOTICE( "%s: %s driver initialized\n",
               mmc_hostname(mmc), DRIVER_NAME);

    return 0;

out:
    if (sdport) {
        if (sdport->irq)
            free_irq(sdport->irq, sdport);

        if (sdport->base)
            devm_iounmap(&pdev->dev, sdport->base);

    }

    if (mmc)
        mmc_free_host(mmc);

    return ret;
}

/************************************************************************
 *
 ************************************************************************/
static
int __exit rtkem_remove(struct platform_device *pdev)
{
    struct mmc_host *mmc = platform_get_drvdata(pdev);

    device_remove_file(&pdev->dev, &dev_attr_cr_fast_RW);
    device_remove_file(&pdev->dev, &dev_attr_em_open_log);
    device_remove_file(&pdev->dev, &dev_attr_em_chk_mode);
    device_remove_file(&pdev->dev, &dev_attr_emmc_errcnt);
    device_remove_file(&pdev->dev, &dev_attr_phase_tune);
#ifdef CONFIG_MMC_RTKEMMC_HK_ATTR
    device_remove_file(&pdev->dev, &dev_attr_emmc_hacking);
#endif
#ifdef RTK_CMD23_USE
    device_remove_file(&pdev->dev, &dev_attr_em_predefined_read);
#endif

    if (mmc) {
        struct rtksd_host *sdport = mmc_priv(mmc);

        if (sdport->int_mode)
            rtkemmc_disable_int(sdport);

        flush_scheduled_work();
        tasklet_kill(&sdport->polling_tasklet);
#ifndef RTK_EMMC_ISR_DIRECT
        tasklet_kill(&sdport->req_end_tasklet);
#endif
        mmc_remove_host(mmc);
        if(!mmc){
            EM_NOTICE("eMMC host have removed.\n");
            mmc_host_local = NULL;
        }

        if(sdport->scrip_buf)
            dma_pool_free(sdport->scrip_pool, sdport->scrip_buf,
                         sdport->scrip_buf_phy);
        if(sdport->scrip_pool)
            dma_pool_destroy(sdport->scrip_pool);

        free_irq(sdport->irq, sdport);

        del_timer_sync(&sdport->timer);
        iounmap(sdport->base);

        release_resource(sdport->res);
        mmc_free_host(mmc);
    }
    platform_set_drvdata(pdev, NULL);
    return 0;
}

#ifdef CONFIG_PM
/************************************************************************
 *
 ************************************************************************/
static
int rtkem_suspend(struct platform_device *dev, pm_message_t state)
{
    struct mmc_host *mmc = platform_get_drvdata(dev);
    int ret = 0;

    EM_NOTICE( "%s: Prepare to suspend...\n", DRIVER_NAME);

#ifdef CONFIG_RTKEMMC_COWORK_WITH_KCPU
#ifndef CONFIG_RTK_KDRV_OPENBSD
    if(mmc_pm_sync_addr != 0){
        int log_flag = 0;
        EM_NOTICE("wait kcpu emmc suspend ,share memory(0x%x)=0x%x...\n", \
            mmc_pm_sync_addr,*((volatile unsigned int *)mmc_pm_sync_addr));
        // polling kcpu emmc suspend done
        while(*((volatile unsigned int *)mmc_pm_sync_addr) != KCPU_EMMC_DEVICE_SUSPEND_DONE){
            if (log_flag == 0){
                EM_NOTICE("wait kcpu emmc suspend done ...\n");
                log_flag++;
            }else{
                udelay(10);
            }
        }
        // clear share memory
        *((volatile unsigned int *)mmc_pm_sync_addr) = 0x0;
        EM_NOTICE("kcpu emmc suspend ...done\n");
    }
#endif
#endif

    if (mmc){
        struct rtksd_host *sdport = mmc_priv(mmc);

        if (sdport->int_mode)
            rtkemmc_disable_int(sdport);

        flush_scheduled_work();

        tasklet_kill(&sdport->polling_tasklet);
#ifndef RTK_EMMC_ISR_DIRECT
        tasklet_kill(&sdport->req_end_tasklet);
#endif
        /* should turn of plug timer */
        del_timer_sync(&sdport->timer);

        sdport->rtflags &= ~RTKCR_FCARD_DETECTED;

        EM_NOTICE( "%s: record:  r_phase50=%d r_phase200=%d ds_tune=%d\n",
            DRIVER_NAME,sdport->tud_r_pha50,sdport->tud_r_pha200 ,sdport->pre_ds_tune);

    }

    /* reset eMMC flow */
    EM_NOTICE( "%s: Holding eMMC reset pin...\n", DRIVER_NAME);
    if (mmc){
        struct rtksd_host *sdport = mmc_priv(mmc);
        rtkem_hold_card(sdport);
    }

    /* light sleep enable */
    cr_maskl(RTK_IP_MBIST, RTK_IP_MBIST_LS_MASK0, RTK_IP_MBIST_LS_SHIFT0, 1);
    cr_maskl(RTK_IP_MBIST, RTK_IP_MBIST_LS_MASK1, RTK_IP_MBIST_LS_SHIFT1, 1);
    cr_maskl(RTK_WRAP_MBIST, RTK_WRAP_MBIST_LS_MASK, RTK_WRAP_MBIST_LS_SHIFT, 1);
    device_suspend = 1;

    return ret;
}

/************************************************************************
 *
 ************************************************************************/
static
int rtkem_clr_sta(struct rtksd_host *sdport)
{
    sdport->cur_clock   = 0;
    sdport->cur_width   = 0;
    sdport->cur_PLL     = 0;
    sdport->cur_div     = 0;
    sdport->cur_w_phase = 0;
    sdport->cur_w_ph400 = 0;
    sdport->cur_r_phase = 0xff;
    sdport->cur_ds_tune = 0;

    return 0;
}

/************************************************************************
 *
 ************************************************************************/
static
int rtkem_resume(struct platform_device *dev)
{
    struct mmc_host *mmc = platform_get_drvdata(dev);
    unsigned long flags;
    int ret = 0;

    EM_NOTICE( "%s: wake up to resume...\n", DRIVER_NAME);

    /* light sleep disable */
    cr_maskl(RTK_IP_MBIST, RTK_IP_MBIST_LS_MASK0, RTK_IP_MBIST_LS_SHIFT0, 0);
    cr_maskl(RTK_IP_MBIST, RTK_IP_MBIST_LS_MASK1, RTK_IP_MBIST_LS_SHIFT1, 0);
    cr_maskl(RTK_WRAP_MBIST, RTK_WRAP_MBIST_LS_MASK, RTK_WRAP_MBIST_LS_SHIFT, 0);

    if (mmc){
        struct rtksd_host *sdport = mmc_priv(mmc);

#ifndef RTK_EMMC_ISR_DIRECT
        tasklet_init(&sdport->req_end_tasklet, rtkcr_req_end_tasklet,
                (unsigned long)sdport);
#endif
        tasklet_init(&sdport->polling_tasklet, rtkem_polling_tasklet,
                (unsigned long)sdport);

        rtkem_clr_sta(sdport);
        spin_lock_irqsave(&sdport->lock, flags);
        dw_em_crt_reset(sdport,0);
        spin_unlock_irqrestore(&sdport->lock, flags);

        /* enable PLL of eMMC */
        dw_em_set_PLL_clk(sdport,CLK_50Mhz,0,1);
        if (get_mmc_tuning_50_done(mmc))
            dw_em_init_phase(sdport,DEFAULT_W_PHASE,sdport->tud_r_pha50,DEFAULT_C_PHASE); /* w_phase;r_phase */
        else
            dw_em_init_phase(sdport,DEFAULT_W_PHASE,DEFAULT_R_PHASE,DEFAULT_C_PHASE); /* w_phase;r_phase */
        sdport->rtflags |= RTKCR_FCARD_DETECTED;

        dw_em_idmac_init(sdport);
        dw_em_set_scrip_base(sdport->scrip_buf_phy);

        spin_lock_irqsave(&sdport->lock, flags);
        sdport->ins_event = (EVENT_SKIP_PHASETUNE|EVENT_SKIP_DSTUNING);
        spin_unlock_irqrestore(&sdport->lock, flags);
        if(sdport->int_mode)
            rtkemmc_enable_int(sdport);

        EM_NOTICE( "%s: read back:  r_phase50=%d r_phase200=%d ds_tune=%d\n",
            DRIVER_NAME,sdport->tud_r_pha50,sdport->tud_r_pha200 ,sdport->pre_ds_tune);

    }

    device_suspend = 0;
    EM_NOTICE("%s: resume finish\n",DRIVER_NAME);
    return ret;
}
#else
#define rtkem_suspend   NULL
#define rtkem_resume    NULL
#endif

#ifdef CONFIG_OF
static const struct of_device_id of_rtkemmc_ids[] = {
    { .compatible = "realtek,rtk-emmc" },
    {}
};

MODULE_DEVICE_TABLE(of, of_rtkemmc_ids);
#endif
static
struct platform_driver rtkemmc_driver = {
    .driver     = {
        .name   = DRIVER_NAME,
        .owner  = THIS_MODULE,
#ifdef CONFIG_OF
        .of_match_table = of_rtkemmc_ids,
#endif
    },
    .remove     = __exit_p(rtkem_remove),
    .suspend    = rtkem_suspend,
    .resume     = rtkem_resume,

};

/************************************************************************
 *
 ************************************************************************/
static
void rtkcr_display_version (void)
{
#ifndef CONFIG_MMC_RTKEMMC_PLUS_MODULE
    const __u8 *revision;
    const __u8 *date;
    const __u8 *time;
    char *running;
    char tmp[256] = {0};

    memcpy(tmp,VERSION,strlen(VERSION));
    running = tmp;

    strsep(&running, " ");
    strsep(&running, " ");
    revision = strsep(&running, " ");
    date = strsep(&running, " ");
    time = strsep(&running, " ");
    EM_NOTICE(BANNER " Rev:%s (%s %s)\n", revision, date, time);

#ifdef CONFIG_MMC_BLOCK_BOUNCE
    EM_NOTICE("%s: CONFIG_MMC_BLOCK_BOUNCE enable\n", DRIVER_NAME);
#endif
#endif
}

#ifndef CONFIG_MMC_RTKEMMC_PLUS_MODULE
static int dw_em_log_func(char *buf)
{
    /*
     For customer request, valid value from 0~15(0xf).
      1. without emmclog or emmclog=0, disable message function.
      2. emmclog=1, enable message for this request.
      3. emmclog=other_value , for future request.
     */
    rtkcr_chk_param(&rtk_emmc_log_state,1,buf+1);
    EM_NOTICE("************************************************\n");
    EM_NOTICE("%s:Bootcode setting emmc_log func is %u\n",
                DRIVER_NAME,rtk_emmc_log_state);
    return 0;
}
__setup("emmc_log",dw_em_log_func);
#else //CONFIG_MMC_RTKEMMC_PLUS_MODULE
void dw_em_log_func_module(void)
{
    char strings[20];
    char *buf = strings;

    if(rtk_parse_commandline_equal("emmc_log", buf, sizeof(strings)) == 0){
        //can't get emmc_log from bootargs
        return;
    }else{
        EM_ERR("get emmc_log from bootargs\n");
    }

    /*
     For customer request, valid value from 0~15(0xf).
      1. without emmclog or emmclog=0, disable message function.
      2. emmclog=1, enable message for this request.
      3. emmclog=other_value , for future request.
     */
    rtkcr_chk_param(&rtk_emmc_log_state,1,buf+1);
    EM_NOTICE("************************************************\n");
    EM_NOTICE("%s:Bootcode setting emmc_log func is %u\n",
                DRIVER_NAME,rtk_emmc_log_state);
    return ;

}
#endif
static
int __init rtkemmc_init(void)
{
    int rc = 0;

    rtkcr_display_version();

#ifdef CONFIG_ANDROID
    EM_NOTICE( "%s: Android timming setting\n", DRIVER_NAME);
#endif

#ifdef MMC_ROOTFS_CHECK
    EM_NOTICE( "%s: enable MMC rootfs check.\n", DRIVER_NAME);
#endif

#ifdef CONFIG_MMC_RTKEMMC_PLUS_MODULE
    dw_em_log_func_module();
#endif

    rc = platform_driver_probe(&rtkemmc_driver, rtkemmc_probe);

    if (rc < 0){
        EM_NOTICE( "Realtek EMMC Controller Driver installation fails.\n\n");
        return -ENODEV;
    }else{
        EM_NOTICE( "Realtek EMMC Controller Driver is successfully installing.\n\n");
        return 0;
    }
}

static
void __exit rtkemmc_exit(void)
{
    platform_driver_unregister(&rtkemmc_driver);
}

/* allow emmc driver initialization earlier */
#ifdef CONFIG_MMC_RTKEMMC_PLUS_MODULE
module_init(rtkemmc_init);
#else
#ifdef CONFIG_RTKEMMC_COWORK_WITH_KCPU
/* if rpc driver initial early than mmc driver, we will cancel this modify*/
module_init(rtkemmc_init);
#else
subsys_initcall(rtkemmc_init);
#endif
#endif

module_exit(rtkemmc_exit);

/* maximum card clock frequency (default 50MHz) */
module_param(maxfreq, int, 0);

/* force PIO transfers all the time */
module_param(nodma, int, 0);

MODULE_AUTHOR("Elbereth");
MODULE_DESCRIPTION("Realtek EMMC Host Controller driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:rtkemmc");

