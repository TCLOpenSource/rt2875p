/*
 *  Copyright (C) 2010 Realtek Semiconductors, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __RTKSDIO_H
#define __RTKSDIO_H

#include <linux/version.h>

#ifdef CONFIG_RTK_KDRIVER_SUPPORT
#include <rtk_kdriver/rtk_crt.h>
#else
#include <mach/rtk_crt.h>
#endif

#include <rtk_kdriver/io.h>
#include "rtkemmc_dbg.h"
#include "rtksdio_dbg.h"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 13))
#include "../../../mmc/core/card.h"
#include "../../../mmc/core/host.h"
#include "../../../mmc/core/core.h"
#include "../../../mmc/core/mmc_ops.h"
#endif

#include <rbus/sys_reg_reg.h>
#include <rbus/pll27x_reg_reg.h>

typedef void (*set_gpio_func_t)(u32 gpio_num,u8 ctl);


/* all error bit define at "Device Status". */
#define RESP_ERR_BIT    ( BIT(31)|BIT(30)|BIT(29)|BIT(28)|   \
                BIT(27)|BIT(26)|BIT(24)|BIT(23)|   \
                BIT(22)|BIT(21)|BIT(20)|BIT(19)|   \
                BIT(16)|BIT(15)|BIT(13)|BIT(7)  )

/* fatal error, if these bits alert the command should skip retry. not figure out yet. */
#define RESP_ERR_FATAL  ( BIT(31)|BIT(30)|BIT(29)|BIT(28)|   \
                BIT(27)|BIT(26)|BIT(24)|BIT(23)|   \
                BIT(22)|BIT(21)|BIT(20)|BIT(19)|   \
                BIT(16)|BIT(15)|BIT(13)|BIT(7)  )

/*
 * Clock rates
 */
#define RTKSD_CLOCKRATE_MAX         33000000 //limited by HW
#define RTKSD_BASE_DIV_MAX          0x100

#define RBUS_READ_FIX
#ifdef RBUS_READ_FIX

#define MMC_ADDR_START      (0xb8010800)
#define MMC_ADDR_END        (0xb8010800+0x3FF)

static __inline__ unsigned int cr_readl(unsigned int __addr){

    volatile unsigned int reg_info;
    unsigned int addr;

    addr = (unsigned int)__addr;
    reg_info = rtd_inl(addr);
    if((addr >= MMC_ADDR_START) && (addr < MMC_ADDR_START) ){
        reg_info = rtd_inl(addr);
        if(reg_info != rtd_inl(addr)){
            //CR_ALERT("rbus hit!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
            WARN(1,"rbus hit!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        }
    }
    return reg_info;
}
#else
    #define cr_readl(offset)        rtd_inl(offset)
#endif

#define cr_writel(val, offset)  rtd_outl(offset, val)

static inline void reg_maskl(const unsigned int mask,
                const unsigned int val, __u32 __iomem regs)
{
    cr_writel((cr_readl(regs) & ~mask) | val,regs);
}

#define CLEAR_CR_CARD_STATUS(reg_addr)      \
do {                                \
    cr_writel(0xffffffff, reg_addr);    \
} while (0)

#define CLEAR_ALL_CR_CARD_STATUS(io_base)        \
do {                                \
    int i = 0;  \
    for(i=0; i<5; i++)  \
        CLEAR_CR_CARD_STATUS(io_base+20+ (i*4));  \
} while (0)

/*  ===== macros and funcitons for Realtek CR ===== */
/* for SD/MMC usage */
#define ON                      0
#define OFF                     1

#define GPIO_H      (0x1<<0)
#define GPIO_L      (0)
#define DIR_OUT     (0x1<<1)
#define DIR_IN      (0)
#define DIR_SKIP    (0x1<<2)
#define MUX_SKIP    (0x1<<3)
#define ISO_FLAG    (0x800UL)

/* MMC response type */
#define SD_R0                   (RESP_TYPE_NON|CRC7_CHK_DIS)
#define SD_R1                   (RESP_TYPE_6B)
#define SD_R1b                  (RESP_TYPE_6B)
#define SD_R2                   (RESP_TYPE_17B)
#define SD_R3                   (RESP_TYPE_6B|CRC7_CHK_DIS)
#define SD_R4                   (RESP_TYPE_6B)
#define SD_R5                   (RESP_TYPE_6B)
#define SD_R6                   (RESP_TYPE_6B)
#define SD_R7                   (RESP_TYPE_6B)
#define SD_R_NO                 (0xFF)

/* rtflags */
#define RTKCR_FCARD_DETECTED    BIT(0)      /* Card is detected */
#define RTKCR_FOPEN_LOG         BIT(1)      /* open command log */
#define RTKCR_USER_PARTITION    BIT(2)      /* card is working on normal partition */
#define RTKCR_IDENT_STATE       BIT(3)      /* card is identification mode */
#define RTKCR_FCARD_POWER       BIT(4)      /* Card is power on */
#define RTKCR_FHOST_POWER       BIT(5)      /* Host is power on */
#define RTKCR_TUNING_STATE      BIT(6)      /* Host is processing phase tuning */
#define RTKCR_RECOVER_STATE     BIT(7)      /* Host is processing recover flow */
#define RTKCR_FOPEN_INT         BIT(8)      /* open interrupt */
#define RTKCR_CMD23_ALERT       BIT(9)      /* Host is processing cmd23 flow */

#define MMC_EN_SSCG             BIT(0)

typedef struct  {
    u32                 ctl;
    u8                  ncode_t;
    u8                  sdiv2;
    u8                  r_phase;
    u8                  w_phase;
    u8                  icp;
    u8                  pi_isel;
    u16                 ncode_ssc;
    u16                 fcode_ssc;
    u32                 range_ssc;
}mmc_param_t;


typedef struct  {
#define PHASE_SAVED     BIT(0)
#define PHASE_CHANGED   BIT(1)
#define DSTUNE_CHANGED  BIT(2)
#define PHASE_FORCE     BIT(3)
    u32 ctl;
    u32 test_ctl_old;

    u32 fix_wphase_old;
    u32 fix_dstune_old;

    u8  wphase;
    u8  rphase;
    u8  cphase;
    u8  ds_mode;
    u8  ds_tune;

    u8  wphase_old;
    u8  rphase_old;
    u8  cphase_old;
    u8  ds_mode_old;
    u8  ds_tune_old;
    u8  ds_result_old;
}mmc_phase_t;


#define STATE_IDLE          0
#define STATE_SENDING_CMD   1
#define STATE_SENDING_DATA  2
#define STATE_DATA_BUSY     3
#define STATE_POST_POLLING  4
#define STATE_SENDING_STOP  5
#define STATE_DATA_ERROR    6
#define STATE_DATA_COMPLETE 7
#define STATE_WAIT_STOP     8
#define STATE_REQUEST_END   9

struct rtk_host_status{
    u32 config1;
    u32 config2;
    u32 config3;
    u32 int_mask;
};

struct rtksd_host {
    struct mmc_host     *mmc;           /* MMC structure */
    struct mmc_command  *cmd;
    u32                 rtflags;        /* Driver states */
    u32                 log_state;      /* log_func state */
    u8                  ins_event;
    u8                  cmd_opcode;
    u8                  int_mode;

#define EVENT_NON               (0)
#define EVENT_INSER             BIT(0)
#define EVENT_REMOV             BIT(1)
#define EVENT_SKIP_PHASETUNE    BIT(4)
#define EVENT_SKIP_DSTUNING     BIT(5)
    u32                 cur_clock;
    u32                 cur_width;
    u32                 cur_PLL;
    u32                 cur_div;
    u8                  cur_w_phase;
    u8                  cur_w_ph400;
    u8                  cur_r_pha50;
    u8                  cur_r_phase;
    u8                  cur_c_phase;
    u8                  tud_r_pha50;
    u8                  tud_r_pha200;
    u32                 cur_ds_tune;
    u32                 pre_ds_tune;
    u8                  cur_timing;
    bool                hs50_tuned;
    bool                hs200_tuned;
    bool                hs400_tuned;
    struct rtk_host_status cur_host_status;
    u32                 mid;

    u8                  reset_event;
    struct mmc_request  *mrq;            /* Current request */
    u8                  wp;
    struct rtk_host_ops *ops;
    struct semaphore    sem;
    struct semaphore    sem_op_end;

    void __iomem        *base;
    u32 base_io;
    spinlock_t          lock;
    unsigned int        ns_per_clk;
    struct workqueue_struct *cr_workqueue;
    struct delayed_work cr_work;
    struct tasklet_struct req_end_tasklet;
    struct tasklet_struct polling_tasklet;
    struct sd_cmd_pkt   *cmd_info;
    struct sd_cmd_pkt   *stop_info;
    struct timer_list   timer;
    struct timer_list   plug_timer;
    struct completion   *int_waiting;
    struct device       *dev;
    struct resource     *res;
    int                 irq;
    u8                  *tmp_buf;
    dma_addr_t          tmp_buf_phy_addr;
    struct  dma_pool    *scrip_pool;
    void                *scrip_buf;
    dma_addr_t          scrip_buf_phy;

#ifdef MONI_MEM_TRASH
    u8                  *cmp_buf;
    dma_addr_t          phy_addr;
#endif

    u8                  pinmux_type;
    u32                 test_count;
    u32                 int_status;
    u32                 int_status_old;
    u32                 sd_status;
    u32                 sd_status2;
    u32                 sd_trans;
    u32                 wrap_sta;
    u32                 gpio_isr_info;
    u32                 task_state;
    u32                 tmout;

    /* for bootcode pre setting parameter *** */
    mmc_param_t *mmc_boot_param;
    u32 *mmc_io_drv;
    u32 mmc_drv;
    u32 mmc_dstune;
    u32 mmc_burst_blk;
    /* for bootcode pre setting parameter &&& */

    unsigned long       timeend;
};

struct rtk_host_ops {
    irqreturn_t (*func_irq)(int irq, void *dev);
    int (*re_init_proc)(struct mmc_card *card);
    int (*card_det)(struct rtksd_host *sdport);
    void (*card_power)(struct rtksd_host *sdport,u8 status);
    void (*chk_card_insert)(struct rtksd_host *rtkhost);
    void (*set_crt_muxpad)(struct rtksd_host *rtkhost);
    void (*set_clk)(struct rtksd_host *rtkhost,u32 mmc_clk);
    void (*reset_card)(struct rtksd_host *rtkhost);
    void (*reset_host)(struct rtksd_host *rtkhost,u8 save_en);
    void (*set_IO_driving)(struct rtksd_host *rtkhost,u32 type_sel);
    void (*bus_speed_down)(struct rtksd_host *sdport);
    u32 (*get_cmdcode)(u32 opcode );
    u32 (*get_r1_type)(u32 opcode );
    u32 (*chk_cmdcode)(struct mmc_command* cmd);
    u32 (*chk_r1_type)(struct mmc_command* cmd);
    void (*hold_card)(struct rtksd_host *sdport);
    int (*set_bits)(struct rtksd_host *sdport,u32 bus_wid);
    int (*set_DDR)(struct rtksd_host *sdport,u32 ddr_mode);
};

struct ms_cmd_pkt {
    struct rtksd_host   *sdport;
    struct ms_command   *mscmd;
    struct mmc_data     *data;

    unsigned char       *dma_buffer;    //data transfer address
    u16                 cmdcode;        //rtk ms operation code
    u16                 trans_bytes;    //register transfer bytes
    u16                 block_count;    //data block transfer count
};

struct sd_cmd_pkt {
    struct mmc_host     *mmc;       /* MMC structure */
    struct rtksd_host   *sdport;
    struct mmc_command  *sbc;    /* cmd->opcode; cmd->arg; cmd->resp; cmd->data */
    struct mmc_command  *cmd;
    struct mmc_command  *stop;
    struct mmc_data     *data;
    unsigned char       *dma_buffer;
    u16                 byte_count;
    u16                 block_count;
    u32                 total_byte_cnt;
    u32                 scrip_cnt;
    u32                 cmd_flag;
    u32                 cmdcode;
    u32                 flags;
    u8                  rsp_para;
    u8                  rsp_len;
    u32                 timeout;
    u8                  sbc_flag;
    u8                  predefined_read;
};

#define DRIVING_COMMON          0
#define DRIVING_HS200           1
#define DRIVING_HS400           2

/* CRT_SYS_CLKSEL setting bit *** */
#define EMMC_SOURCECLKSEL_SHT       (27)
#define MASK_EMMC_SOURCECLKSEL      (0x01UL<<EMMC_SOURCECLKSEL_SHT)
#define CR_SRC_CLK_250M             (0x00UL<<EMMC_SOURCECLKSEL_SHT)
#define CR_SRC_CLK_295M             (0x01UL<<EMMC_SOURCECLKSEL_SHT)

#define SD_CLKSEL_SHT               (20)

#define SD_CLKSEL_MASK              (0x07UL<<SD_CLKSEL_SHT)
#define SD_CLOCK_SPEED_GAP          (CARD_SWITCHCLOCK_25MHZ<<SD_CLKSEL_SHT)

#define CARD_SWITCHCLOCK_75MHZ      (0x00UL)
#define CARD_SWITCHCLOCK_50MHZ      (0x01UL)
#define CARD_SWITCHCLOCK_37MHZ      (0x02UL)
#define CARD_SWITCHCLOCK_30MHZ      (0x03UL)
#define CARD_SWITCHCLOCK_25MHZ      (0x04UL)
#define CARD_SWITCHCLOCK_18MHZ      (0x05UL)
#define CARD_SWITCHCLOCK_15MHZ      (0x06UL)

#define LOWEST_CLOCK    CARD_SWITCHCLOCK_15MHZ

#define CLK_200Khz      200000
#define CLK_400Khz      400000

#define CLK_10Mhz       10000000
#define CLK_15Mhz       15000000
#define CLK_20Mhz       20000000
#define CLK_25Mhz       25000000
#define CLK_30Mhz       30000000
#define CLK_40Mhz       40000000
#define CLK_50Mhz       50000000
#define CLK_60Mhz       60000000
#define CLK_70Mhz       70000000
#define CLK_80Mhz       80000000
#define CLK_90Mhz       90000000

#define CLK_100Mhz      100000000
#define CLK_110Mhz      110000000
#define CLK_120Mhz      120000000
#define CLK_130Mhz      130000000
#define CLK_140Mhz      140000000
#define CLK_150Mhz      150000000
#define CLK_160Mhz      160000000
#define CLK_170Mhz      170000000
#define CLK_180Mhz      180000000
#define CLK_190Mhz      190000000
#define CLK_200Mhz      200000000

#define RTK_CLK_STEP   7

static const
u32 rtk_clock_tbl[RTK_CLK_STEP] = {
    75000000,
    50000000,
    37000000,
    30000000,
    25000000,
    18000000,
    15000000
};

#define RTK_DIV_CLK_128  (rtk_clock_tbl[RTK_CLK_STEP-1]>>7) //clock divide with 128
#define RTK_DIV_CLK_256  (rtk_clock_tbl[RTK_CLK_STEP-1]>>8) //clock divide with 256

#define RCA_SHIFTER         16

#define BYTE_CNT            0x200
#define RTK_NORMAL_SPEED    0x00
#define RTK_HIGH_SPEED      0x01
#define RTK_1_BITS          0x00
#define RTK_4_BITS          0x10
#define RTK_BITS_MASK       0x30
#define RTK_SPEED_MASK      0x01
#define RTK_PHASE_MASK      0x06

/* send status event */
#define STATE_IDLE          0
#define STATE_READY         1
#define STATE_IDENT         2
#define STATE_STBY          3
#define STATE_TRAN          4
#define STATE_DATA          5
#define STATE_RCV           6
#define STATE_PRG           7
#define STATE_DIS           8

#define LVL_HI              0x01
#define LVL_LO              0x00

#define rtkcr_get_int_sta(io_base,status_addr) \
            *(u32 *)status_addr = cr_readl(io_base+CR_INT_STATUS)

#define rtkcr_get_sd_sta(io_base,status_addr) \
            *(u32 *)status_addr = cr_readl(io_base+SD_STATUS)

#define rtkcr_get_sd_sta2(io_base,status_addr) \
            *(u32 *)status_addr = cr_readl(io_base+SD_STATUS2)

#define rtkcr_get_sd_trans(io_base,status_addr) \
            *(u32 *)status_addr = cr_readl(io_base+SD_TRANSFER)

#define rtkcr_clr_int_dec_sta(io_base) \
            cr_writel( cr_readl(io_base+CR_INT_STATUS)| \
                       (RTKCR_INT_DEC_STA) , \
                       io_base+CR_INT_STATUS)


/* RTKCR_INT_CARD_STA : only bit0 can be write clear */
#define rtkcr_clr_int_card_sta(io_base) \
            cr_writel( cr_readl(io_base+CR_INT_STATUS)| \
                       (RTKCR_INT_STATUS_CHANGE  ) , \
                       io_base+CR_INT_STATUS)

#define rtkcr_clr_int_sta(io_base) \
            cr_writel( cr_readl(io_base+CR_INT_STATUS)| \
                       (RTKCR_INT_ALL_STA) , \
                       io_base+CR_INT_STATUS)

#define rtkcr_hold_int_dec(io_base)    \
            cr_writel( cr_readl(io_base+CR_INT_MASK) &  \
                       ~(RTKCR_INT_DEC_MASK), \
                       io_base+CR_INT_MASK)

#define rtkcr_hold_int_card(io_base)    \
            cr_writel( cr_readl(io_base+CR_INT_MASK) &  \
                       ~(RTKCR_INT_CARD_MASK), \
                       io_base+CR_INT_MASK)

#define rtkcr_hold_int_all(io_base)    \
            cr_writel( cr_readl(io_base+CR_INT_MASK) &  \
                       ~(RTKCR_INT_ALL_MASK), \
                       io_base+CR_INT_MASK)

#define rtkcr_en_int_dec(io_base)  \
            cr_writel( cr_readl(io_base+CR_INT_MASK) |  \
                       (RTKCR_INT_DEC_MASK), \
                       io_base+CR_INT_MASK)

#define rtkcr_en_int_card(io_base)  \
            cr_writel( cr_readl(io_base+CR_INT_MASK) |  \
                       (RTKCR_INT_CARD_MASK), \
                       io_base+CR_INT_MASK)

#define  rtkcr_mdelay(x)  \
            set_current_state(TASK_INTERRUPTIBLE); \
            schedule_timeout(msecs_to_jiffies(x))

#define INT_BLOCK_R_GAP 0x200
#define INT_BLOCK_W_GAP 5

static const char *const state_tlb[9] = {
    "STATE_IDLE",
    "STATE_READY",
    "STATE_IDENT",
    "STATE_STBY",
    "STATE_TRAN",
    "STATE_DATA",
    "STATE_RCV",
    "STATE_PRG",
    "STATE_DIS"
};

static const char *const bit_tlb[4] = {
    "1bit",
    "4bits",
    "8bits",
    "unknow"
};

static const char *const clk_tlb[7] = {
    "75MHz",
    "50MHz",
    "37MHz",
    "30MHz",
    "25MHz",
    "18MHz",
    "15MHz"
};

/* data read cmd */
static const u8  opcode_r_type[16] = {
    0,0,0,0,0,1,1,0,0,0,0,0,1,1,1,0
};

/* data write cmd */
static const u8  opcode_w_type[16] = {
    1,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0
};

/* data xfer cmd */
static const u8 opcode_d_type[16] = {
    1,1,1,0,0,1,1,0,0,1,1,0,1,1,1,0
};

static const char *const cmdcode_tlb[16] = {
    "N_W",      /* 0 */
    "AW3",      /* 1 */
    "AW4",      /* 2 */
    "UNK",      /* 3 */
    "UNK",      /* 4 */
    "AR3",      /* 5 */
    "AR4",      /* 6 */
    "UNK",      /* 7 */
    "SGR",      /* 8 */
    "AW1",      /* 9 */
    "AW2",      /* 10 */
    "UNK",      /* 11 */
    "N_R",      /* 12 */
    "AR1",      /* 13 */
    "AR2",      /* 14 */
    "UNK",      /* 15 */
};

#define CARD_STA_ERR_MASK ((1<<31)|(1<<30)|(1<<29)|(1<<28)|(1<<27)|(1<<26)|(1<<24)|(1<<23)|(1<<22)|(1<<21)|(1<<20)|(1<<19)|(1<<18)|(1<<17)|(1<<16)|(1<<15)|(1<<13)|(1<<7))

/* Only ADTC type cmd use */
static const unsigned char rtk_sd_cmdcode[60][2] = {
    {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, /* 0~3 */
    {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_NORMALREAD ,SD_R1 }, {SD_CMD_UNKNOW ,SD_R0 }, /* 4~7 */
    {SD_NORMALREAD ,SD_R1 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_AUTOREAD1  ,SD_R1 }, /* 8~11 */
    {SD_CMD_UNKNOW ,SD_R0 }, {SD_NORMALREAD ,SD_R1 }, {SD_NORMALREAD ,SD_R1 }, {SD_CMD_UNKNOW ,SD_R0 }, /* 12~15 */
    {SD_CMD_UNKNOW ,SD_R0 }, {SD_AUTOREAD2  ,SD_R1 }, {SD_AUTOREAD1  ,SD_R1 }, {SD_NORMALWRITE,SD_R1 }, /* 16~19 */
    {SD_AUTOWRITE1 ,SD_R1 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_NORMALREAD ,SD_R1 }, {SD_CMD_UNKNOW ,SD_R0 }, /* 20~23 */
    {SD_AUTOWRITE2 ,SD_R1 }, {SD_AUTOWRITE1 ,SD_R1 }, {SD_NORMALWRITE,SD_R1 }, {SD_NORMALWRITE,SD_R1 }, /* 24~27 */
    {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_NORMALREAD ,SD_R1 }, {SD_NORMALREAD ,SD_R1 }, /* 28~31 */
    {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, /* 32~35 */
    {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, /* 36~39 */
    {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_NORMALREAD ,SD_R1 }, {SD_CMD_UNKNOW ,SD_R0 }, /* 40~43 */
    {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, /* 44~47 */
    {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_NORMALREAD ,SD_R1 }, /* 48~51 */
    {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, /* 52~55 */
    {SD_AUTOREAD2  ,SD_R1 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }  /* 56~59 */
};

#define CLK_PIN_IDX     0
#define CMD_PIN_IDX     1
#define RST_PIN_IDX     2   // for eMMC
#define WP_PIN_IDX      RST_PIN_IDX //for CARD-Reader
#define CD_PIN_IDX      3
#define D0_PIN_IDX      4
#define D1_PIN_IDX      5
#define D2_PIN_IDX      6
#define D3_PIN_IDX      7
#define D4_PIN_IDX      8
#define D5_PIN_IDX      9
#define D6_PIN_IDX      10
#define D7_PIN_IDX      11

#define MAX_TUNING_STEP         (32)    //skip 28~31 step
#define MAX_DS_TUNE             (32)    //Only 0~27 are available even hardware has 32 step

/* One git use */
#if defined(CONFIG_ARCH_RTK6748) || defined(CONFIG_ARCH_RTK2885M)
#include "rtk_mmc_6748.h" /* M6 and M7 */
#elif defined(CONFIG_ARCH_RTK6702)
#include "rtk_mmc_6702.h" /* MK2 */
#elif defined(CONFIG_ARCH_RTK2851A)
#include "rtk_mmc_2851a.h" /* mac7 */
#elif defined(CONFIG_ARCH_RTK2851C)
#include "rtk_mmc_2851c.h" /* mac8 */
#endif

/*
Pin mux info :
    xxxx_pinmux_main.doc
    xxxx_pinmux_pmm.doc

GPIO info :
    xxxx_DesignSpec_MISC_GPIO.doc
    xxxx_DesignSpec_ISO_MISC_GPIO.doc
*/

/* max iso_gpio is 30 */
static const u8 iso_gpio_mux_tbl[MAX_ISO_GPIO_CNT+1][2] ={
    {0x00, 28}, {0x00, 20}, {0x00, 12}, {0x00, 4},  /*  0: 3 */
    {0x04, 28}, {0x04, 20}, {0x04, 12}, {0x04, 4},  /*  4: 7 */
    {0x08, 28}, {0x08, 20}, {0x08, 12}, {0x08, 4},  /*  8:11 */
    {0x0C, 28}, {0x0C, 20}, {0x0C, 12}, {0x0C, 4},  /* 12:15 */
    {0x10, 28}, {0x10, 20}, {0x10, 12}, {0x10, 4},  /* 16:19 */
    {0x14, 28}, {0x14, 20}, {0x14, 12}, {0x14, 4},  /* 20:23 */
    {0x18, 28}, {0x18, 20}, {0x18, 12}, {0x18, 4},  /* 24:27 */
    {0x1C, 28}, {0x1C, 20}, {0x1C, 12}              /* 28:30 */
};

/* max mis_gpio is 144 */
static const u8 mis_gpio_mux_tbl[MAX_MIS_GPIO_CNT][2] ={
    {0x00, 28}, {0x00, 20}, {0x00, 12}, {0x00, 4},  /*  0:  3 */
    {0x04, 28}, {0x04, 20}, {0x04, 12}, {0x04, 4},  /*  4:  7 */
    {0x08, 28}, {0x08, 20}, {0x08, 12}, {0x08, 4},  /*  8: 11 */
    {0x0C, 28}, {0x0C, 20}, {0x0C, 12}, {0x0C, 4},  /* 12: 15 */
    {0x10, 28}, {0x10, 20}, {0x10, 12}, {0x10, 4},  /* 16: 19 */
    {0x14, 28}, {0x14, 20}, {0x14, 12}, {0x14, 4},  /* 20: 23 */
    {0x18, 28}, {0x18, 20}, {0x18, 12}, {0x18, 4},  /* 24: 27 */
    {0x1C, 28}, {0x1C, 20}, {0x1C, 12}, {0x1C, 4},  /* 28: 31 */
    {0x20, 28}, {0x20, 20}, {0x20, 12}, {0x20, 4},  /* 32: 35 */
    {0x24, 28}, {0x24, 20}, {0x24, 12}, {0x24, 4},  /* 36: 39 */
    {0x28, 28}, {0x28, 20}, {0x28, 12}, {0x28, 4},  /* 40: 43 */
    {0x2C, 28}, {0x2C, 20}, {0x2C, 12}, {0x2C, 4},  /* 44: 47 */
    {0x30, 28}, {0x30, 20}, {0x30, 12}, {0x30, 4},  /* 48: 51 */
    {0x34, 28}, {0x34, 20}, {0x34, 12}, {0x34, 4},  /* 52: 55 */
    {0x38, 28}, {0x38, 20}, {0x38, 12}, {0x38, 4},  /* 56: 59 */
    {0x3C, 28}, {0x3C, 20}, {0x3C, 12}, {0x3C, 4},  /* 60: 63 */
    {0x40, 28}, {0x40, 20}, {0x40, 12}, {0x40, 4},  /* 64: 67 */
    {0x44, 28}, {0x44, 20}, {0x44, 12}, {0x44, 4},  /* 68: 71 */
    {0x48, 28}, {0x48, 20}, {0x48, 12}, {0x48, 4},  /* 72: 75 */
    {0x4C, 28}, {0x4C, 20}, {0x4C, 12}, {0x4C, 4},  /* 76: 79 */
    {0x50, 28}, {0x50, 20}, {0x50, 12}, {0x50, 4},  /* 80: 83 */
    {0x54, 28}, {0x54, 20}, {0x54, 12}, {0x54, 4},  /* 84: 87 */
    {0x58, 28}, {0x58, 20}, {0x58, 12}, {0x58, 4},  /* 88: 91 */
    {0x5C, 28}, {0x5C, 20}, {0x5C, 12}, {0x5C, 4},  /* 92: 95 */
    {0x60, 28}, {0x60, 20}, {0x60, 12}, {0x60, 4},  /* 96: 99 */
    {0x64, 28}, {0x64, 20}, {0x64, 12}, {0x64, 4},  /* 100:103*/
    {0x68, 28}, {0x68, 20}, {0x68, 12}, {0x68, 4},  /* 104:107*/
    {0x6C, 28}, {0x6C, 20}, {0x6C, 12}, {0x6C, 4},  /* 108:111*/
    {0x70, 28}, {0x70, 20}, {0x70, 12}, {0x70, 4},  /* 112:115*/
    {0x74, 28}, {0x74, 20}, {0x74, 12}, {0x74, 4},  /* 116:119*/
    {0x78, 28}, {0x78, 20}, {0x78, 12}, {0x78, 4},  /* 120:123*/
    {0x7C, 28}, {0x7C, 20}, {0x7C, 12}, {0x7C, 4},  /* 124:127*/
    {0x80, 28}, {0x80, 20}, {0x80, 12}, {0x80, 4},  /* 128:131*/
    {0x84, 28}, {0x84, 20}, {0x84, 12}, {0x84, 4},  /* 132:135*/
    {0x88, 28}, {0x88, 20}, {0x88, 12}, {0x88, 4},  /* 136:139*/
    {0x8C, 28}, {0x8C, 20}, {0x8C, 12}, {0x8C, 4},  /* 140:143*/
};

#define I_MUX(pin)          (iso_gpio_mux_tbl[pin][0]+ISO_PINMUX_BASE)
#define I_SHT(pin)          (iso_gpio_mux_tbl[pin][1])
#define M_MUX(pin)          (mis_gpio_mux_tbl[pin][0]+MIS_PINMUX_BASE)
#define M_SHT(pin)          (mis_gpio_mux_tbl[pin][1])
#define GPIO_CHK_ISO(pin)   ((pin &  ISO_FLAG)?1:0)
#define GPIO_GET_NUM(pin)   (pin & ~ISO_FLAG)

#define PIN_MUX_SEL_SD      (0x2UL)

//////////////////////////////////
/*
 //CI_CA_POWER_EN/SD_Card ST_GPIO40
rtd_part_outl(0x1806021c,23,20,0xF);  //ST_GPIO40 Pinmux
rtd_outl(0x18061104,0x00000401);  //set GPIO
rtd_outl(0x1806110c,0x00000401);  //output 1
*/
#define RT_CR_POW_MUX_REG       0xb806021c
#define RT_CR_POW_MUX_MASK      (0xFU<<20)
#define RT_CR_POW_GPDIR_REG     0xb8061104
#define RT_CR_POW_GPDATO_REG    0xb806110c
#define RT_CR_POW_MASK          (BIT(10))

#define ACTIVE_SRC_2    //default

#if defined(ACTIVE_SRC_0)
/* Pinmux information [Source 0]
rtd_part_outl(0xb80008e0,01,00,0x0);//src0:input mux
rtd_part_outl(0xb8000844,23,20,0x2);//   SD_CD_src0,<I>,<HS>            Ps_Reg:gpio_62_ps
rtd_part_outl(0xb8000844,15,12,0x2);//   SD_WP_src0,<I>,<HS>            Ps_Reg:gpio_63_ps
rtd_part_outl(0xb8000844,07,04,0x2);//   SD_Data1_pad_src0,<I/O>,<HS>   Ps_Reg:gpio_64_ps
rtd_part_outl(0xb8000848,31,28,0x2);//   SD_Data0_pad_src0,<I/O>,<HS>   Ps_Reg:gpio_65_ps
rtd_part_outl(0xb8000848,23,20,0x2);//   SD_CLK_pad_src0,<O>,<HS>       Ps_Reg:gpio_66_ps
rtd_part_outl(0xb8000848,15,12,0x2);//   SD_CMD_pad_src0,<I/O>,<HS>     Ps_Reg:gpio_67_ps
rtd_part_outl(0xb8000848,07,04,0x2);//   SD_Data3_pad_src0,<I/O>,<HS>   Ps_Reg:gpio_68_ps
rtd_part_outl(0xb800084c,31,28,0x2);//   SD_Data2_pad_src0,<I/O>,<HS>   Ps_Reg:gpio_69_ps
*/
#define ACTIVE_SRC 0
#define RT_CR_SRC_REG   0xb80008e0
#define RT_CR_MUX_REG1  0xb8000844
#define RT_CR_MUX_REG2  0xb8000848
#define RT_CR_MUX_REG3  0xb800084C

#define RT_CR_MUX_MASK1 ((0xfUL<<20)|(0xfUL<<12)|(0xfUL<<4))    //CD/WP/D1
#define RT_CR_MUX_MASK2 ((0xfUL<<28)|(0xfUL<<12)|(0xfUL<<4))    //D0/CMD/D3
#define RT_CR_MUX_MASK3 (0xfUL<<28)                             //D2

#define RT_CR_MUX_VAL1  ((0x2UL<<20)|(0x2UL<<12)|(0x2UL<<4))    //CD/WP/D1
#define RT_CR_MUX_VAL2  ((0x2UL<<28)|(0x2UL<<12)|(0x2UL<<4))    //D0/CMD/D3
#define RT_CR_MUX_VAL3  (0x2UL<<28)                             //D2

#define RT_CR_CLK_MUX   0xb8000848
#define RT_CR_CLK_MASK  (0xfUL<<20)
#define RT_CR_CLK_VAL   (0x2UL<<20)

/* GPIO setting */
#define RT_CR_GPDIR_REG1    (0xB801BD08)
#define RT_CR_GPDATO_REG1   (0xB801BD18)
#define RT_CR_GP_MASK1      (BIT(1)|BIT(2)|BIT(3)|BIT(4)|BIT(6)|BIT(7)|BIT(8))
#define RT_CR_GP_CLK        (BIT(5))

__attribute__((unused))
static u16 rtk_sd_pin_mux_tbl[8] = {
/* CLK,CMD, WP, CD, D0, D1, D2, D3 */
    66, 67, 63, 62, 65, 64, 69, 68
};

#elif defined(ACTIVE_SRC_1)
/* Pinmux information [Source 1]
rtd_part_outl(0xb80008e0,01,00,0x1);//src1:input mux
rtd_part_outl(0xb8000828,23,20,0x2);//SD_CD_src1,<I>,<HS>           MIS_GPIO_bit34,<I/O>
rtd_part_outl(0xb8000828,19,16,0x2);//SD_WP_src1,<I>,<HS>           MIS_GPIO_bit35,<I/O>
rtd_part_outl(0xb8000828,15,12,0x2);//SD_Data1_pad_src1,<I/O>,<HS>  MIS_GPIO_bit36,<I/O>
rtd_part_outl(0xb8000828,11,08,0x2);//SD_Data0_pad_src1,<I/O>,<HS>  MIS_GPIO_bit37,<I/O>
rtd_part_outl(0xb8000844,23,20,0x3);//SD_CLK_pad_src1,<O>,<HS>      MIS_GPIO_bit62,<I/O>
rtd_part_outl(0xb8000844,15,12,0x3);//SD_CMD_pad_src1,<I/O>,<HS>    MIS_GPIO_bit63,<I/O>
rtd_part_outl(0xb8000844,07,04,0x3);//SD_Data3_pad_src1,<I/O>,<HS>  MIS_GPIO_bit64,<I/O>
rtd_part_outl(0xb8000848,31,28,0x3);//SD_Data2_pad_src1,<I/O>,<HS>  MIS_GPIO_bit65,<I/O>
*/
#define ACTIVE_SRC 1
#define RT_CR_SRC_REG   0xb80008e0
#define RT_CR_MUX_REG1  0xb8000828
#define RT_CR_MUX_REG2  0xb8000844
#define RT_CR_MUX_REG3  0xb8000848

#define RT_CR_MUX_MASK1 ((0xfUL<<20)|(0xfUL<<16)|(0xfUL<<12)|(0xfUL<<4))    //CD/WP/D1/D0
#define RT_CR_MUX_MASK2 ((0xfUL<<12)|(0xfUL<<4))    //CMD/D3
#define RT_CR_MUX_MASK3 (0xfUL<<28)                             //D2

#define RT_CR_MUX_VAL1  ((0x2UL<<20)|(0x2UL<<16)|(0x2UL<<12)|(0x2UL<<4))    //CD/WP/D1/D0
#define RT_CR_MUX_VAL2  ((0x3UL<<12)|(0x3UL<<4))    //CMD/D3
#define RT_CR_MUX_VAL3  (0x3UL<<28)                             //D2

#define RT_CR_CLK_MUX   0xb8000844
#define RT_CR_CLK_MASK  (0xfUL<<20)
#define RT_CR_CLK_VAL   (0x3UL<<20)

/* GPIO setting */
#define RT_CR_GPDIR_REG1    (0xB801BD08)
#define RT_CR_GPDIR_REG2    (0xB801BD04)
#define RT_CR_GPDATO_REG1   (0xB801BD1C)
#define RT_CR_GPDATO_REG2   (0xB801BD18)
#define RT_CR_GP_MASK1      (BIT(2)|BIT(3)|BIT(4))
#define RT_CR_GP_MASK2      (BIT(4)|BIT(5)|BIT(6)|BIT(7))
#define RT_CR_GP_CLK        (BIT(1))

__attribute__((unused))
static u16 rtk_sd_pin_mux_tbl[8] = {
/* CLK,CMD, WP, CD, D0, D1, D2, D3 */
    62, 63, 35, 34, 37, 36, 65, 64
};

#else
/* Pinmux information [Source 2]
rtd_part_outl(0xb80008e0,01,00,0x2);//src2:input mux
rtd_part_outl(0xb80008b0,23,20,0x2);//SD_WP_src2,<I>,<HS>            Ps_Reg:gpio_144_ps
rtd_part_outl(0xb80008b0,15,12,0x2);//SD_CD_src2,<I>,<HS>            Ps_Reg:gpio_145_ps
rtd_part_outl(0xb80008b0,07,04,0x2);//SD_Data1_pad_src2,<I/O>,<HS>   Ps_Reg:gpio_146_ps
rtd_part_outl(0xb80008b4,31,28,0x2);//SD_Data0_pad_src2,<I/O>,<HS>   Ps_Reg:gpio_147_ps
rtd_part_outl(0xb80008b4,23,20,0x2);//SD_CLK_pad_src2,<O>,<HS>       Ps_Reg:gpio_148_ps
rtd_part_outl(0xb80008b4,15,12,0x2);//SD_CMD_pad_src2,<I/O>,<HS>     Ps_Reg:gpio_149_ps
rtd_part_outl(0xb80008b4,07,04,0x2);//SD_Data3_pad_src2,<I/O>,<HS>   Ps_Reg:gpio_150_ps
rtd_part_outl(0xb80008b8,31,28,0x2);//SD_Data2_pad_src2,<I/O>,<HS>   Ps_Reg:gpio_151_ps
 */
#define ACTIVE_SRC 2
#define RT_CR_SRC_REG   0xb80008e0
#define RT_CR_MUX_REG1  0xb80008b0
#define RT_CR_MUX_REG2  0xb80008b4
#define RT_CR_MUX_REG3  0xb80008b8

#define RT_CR_MUX_MASK1 ((0xfU<<20)|(0xfU<<12)|(0xfU<<4))               //WP/CD/D1
#define RT_CR_MUX_MASK2 ((0xfU<<28)|(0xfU<<20)|(0xfU<<12)|(0xfU<<4))    //D0/CLK/CMD/D3
#define RT_CR_MUX_MASK3 ((0xfU<<28))                                    //D2

#define RT_CR_MUX_VAL1  ((0x2U<<20)|(0x2U<<12)|(0x2U<<4))               //WP/CD/D1
#define RT_CR_MUX_VAL2  ((0x2U<<28)|(0x2U<<20)|(0x2U<<12)|(0x2U<<4))    //D0/CLK/CMD/D3
#define RT_CR_MUX_VAL3  ((0x2U<<28))                                    //D2

/* RT_CR_MUX_LOWX: switch cmd/d0/d1/d2/d3 to gpio and pull low */
#define RT_CR_MUX_LOW1 ((0xfU<<4))                          //D1
#define RT_CR_MUX_LOW2 ((0xfU<<28)|(0xfU<<12)|(0xfU<<4))    //D0/CMD/D3
#define RT_CR_MUX_LOW3 ((0xfU<<28))                         //D2

#define RT_CR_CLK_MUX   0xb80008b4
#define RT_CR_CLK_MASK  (0xfU<<20)
#define RT_CR_CLK_VAL   (0x2U<<20)

/* GPIO setting */
#define RT_CR_GPDIR_REG1    (0xB801BD10)
#define RT_CR_GPDIR_REG2    RT_CR_GPDIR_REG1    //dummy define
#define RT_CR_GPDATO_REG1   (0xB801BD24)
#define RT_CR_GPDATO_REG2   RT_CR_GPDATO_REG1   //dummy define
#define RT_CR_GP_MASK1      (BIT(23)|BIT(24)|BIT(26)|BIT(27)|BIT(28))   //D0/D1/CMD/D3/D2
#define RT_CR_GP_MASK2      RT_CR_GP_MASK1      //dummy define
#define RT_CR_GP_CLK        (BIT(25))

__attribute__((unused))
static u16 rtk_sd_pin_mux_tbl[8] = {
/* CLK,CMD, WP, CD, D0, D1, D2, D3 */
   148,149,144,145,107,146,151,150
};

#endif


#endif
