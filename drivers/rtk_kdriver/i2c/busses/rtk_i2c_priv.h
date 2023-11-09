#ifndef __RTK_I2C_PRIV_H__
#define __RTK_I2C_PRIV_H__

#ifndef BUILD_QUICK_SHOW
#include <asm/atomic.h>
#include <asm/delay.h>
#include <asm/irq.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <rtk_kdriver/i2c-algo-rtk.h>
#else
#include <no_os/wait.h>
#include <no_os/spinlock.h>
#include <no_os/slab.h>
#include <no_os/gfp.h>
#endif

#include <rtd_log/rtd_module_log.h>
#include <mach/platform.h>
#include <rtk_kdriver/io.h>

#ifdef CONFIG_RTK_KDRIVER_SUPPORT
#include <rtk_kdriver/rtk_gpio.h>
#else
#include <mach/rtk_gpio.h>
#endif

#ifndef BUILD_QUICK_SHOW
#define LockrtkI2CPhy(hHandle)               mutex_lock(&hHandle->mutex)
#define UnlockrtkI2CPhy(hHandle)             mutex_unlock(&hHandle->mutex)
#else
#define LockrtkI2CPhy(hHandle)
#define UnlockrtkI2CPhy(hHandle)

#define irqreturn_t             int
#define IRQ_NONE                -1
#define IRQ_HANDLED             0

#define mutex_init(x)
#define mutex_destroy(x)
#define request_irq(x, x1, x2, x3, x4)          0
#define free_irq(x, x1)
#endif


#define VERSION                "2.2"
#define MODLE_NAME             "Rtd_I2C"

#if (defined(CONFIG_ARCH_RTK2851A) || defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2851F))
#define I2C_PHY_CNT             (3)
#else
#define I2C_PHY_CNT             (6)
#endif

#define I2C_DEV_MAX_CNT         (32)

#define MINIMUM_DELAY_EN
#define SPIN_LOCK_PROTECT_EN
#define FIFO_THRESHOLD          31
#define I2C_TIMEOUT_INTERVAL    100

#define EVENT_START_XFER        4
#define EVENT_STOP_XFER         5
#define EVENT_ENTER_ISR         6
#define EVENT_EXIT_ISR          7
#define EVENT_EXIT_TIMEOUT      8

typedef enum
{
    CLKSEL_MODE_27MHZ = 0,
    CLKSEL_MODE_62_5MHZ = 1,
} CLKSEL_MODE;

typedef enum
{
    SPD_MODE_LS = 33,
    SPD_MODE_SS = 100,
    SPD_MODE_FS = 400,
    SPD_MODE_HS = 1000
} SPD_MODE;

typedef enum
{
    ADDR_MODE_7BITS = 7,
    ADDR_MODE_10BITS = 10
} ADDR_MODE;

enum
{
    ECMDSPLIT = 40,
    ETXABORT = 41,
    ETIMEOUT = 42,
    EILLEGALMSG = 43,
    EADDROVERRANGE = 44,
};

enum
{
    NON_STOP = 0,
    WAIT_STOP = 1,
};

enum
{
    I2C_MODE = 0,
    GPIO_MODE = 1
};

enum
{
    G2C_STATE_START = 0,
    G2C_STATE_ADDR,
    G2C_STATE_DATA,
    G2C_STATE_STOP,
    G2C_STATE_DONE,
};

#define G2C_ST(major, minor)    (((major & 0x7) <<5) + (minor & 0x1F))
#define G2C_MAJOR_STATE(x)      ((x >>5) & 0x07)
#define G2C_MINOR_STATE(x)      (x & 0x1F)

enum
{
    G2C_ST_START = G2C_ST(G2C_STATE_START, 0),

    G2C_ST_ADDR0 = G2C_ST(G2C_STATE_ADDR, 0),
    G2C_ST_ADDR1 = G2C_ST(G2C_STATE_ADDR, 1),
    G2C_ST_ADDR2 = G2C_ST(G2C_STATE_ADDR, 2),
    G2C_ST_ADDR3 = G2C_ST(G2C_STATE_ADDR, 3),
    G2C_ST_ADDR4 = G2C_ST(G2C_STATE_ADDR, 4),
    G2C_ST_ADDR5 = G2C_ST(G2C_STATE_ADDR, 5),
    G2C_ST_ADDR6 = G2C_ST(G2C_STATE_ADDR, 6),
    G2C_ST_ADDR7 = G2C_ST(G2C_STATE_ADDR, 7),
    G2C_ST_ADDR_ACK = G2C_ST(G2C_STATE_ADDR, 8),

    G2C_ST_DATA0 = G2C_ST(G2C_STATE_DATA, 0),
    G2C_ST_DATA1 = G2C_ST(G2C_STATE_DATA, 1),
    G2C_ST_DATA2 = G2C_ST(G2C_STATE_DATA, 2),
    G2C_ST_DATA3 = G2C_ST(G2C_STATE_DATA, 3),
    G2C_ST_DATA4 = G2C_ST(G2C_STATE_DATA, 4),
    G2C_ST_DATA5 = G2C_ST(G2C_STATE_DATA, 5),
    G2C_ST_DATA6 = G2C_ST(G2C_STATE_DATA, 6),
    G2C_ST_DATA7 = G2C_ST(G2C_STATE_DATA, 7),
    G2C_ST_DATA_ACK = G2C_ST(G2C_STATE_DATA, 8),

    G2C_ST_STOP = G2C_ST(G2C_STATE_STOP, 0),
    G2C_ST_DONE = G2C_ST(G2C_STATE_DONE, 0)
};

typedef struct
{
    unsigned char mode;
#define I2C_IDEL               0
#define I2C_MASTER_READ        1
#define I2C_MASTER_WRITE       2
#define I2C_MASTER_RANDOM_READ 3

    unsigned char flags;
#define I2C_NO_STOP            0x01

    unsigned char *tx_buff;
    unsigned short tx_buff_len;
    unsigned short tx_len;
    unsigned char *rx_buff;
    unsigned short rx_buff_len;
    unsigned short rx_len;
    unsigned long except_time;
    unsigned long timeout;
    int ret;
    unsigned int tx_abort_source;

    unsigned int gpio_wait_time;
#define G2C_WAIT_TIMEOUT	3

    unsigned char gpio_xfer_state;
    unsigned char gpio_xfer_sub_state;
} rtk_i2c_xfer;

typedef struct rtk_i2c_reg_map_t rtk_i2c_reg_map;

struct rtk_i2c_reg_map_t
{
    unsigned long IC_ISR_EN;
    unsigned long IC_ISR_EN_MASK;
    unsigned long IC_ISR;
    unsigned long IC_INT;
    unsigned long IC_CLKSEL;
    unsigned long IC_CLKSEL_MASK;
    unsigned long IC_INTERVAL_MODE_CTRL;
    unsigned long IC_GUARD_STATUS;
    unsigned long IC_GPIO_MODE_CTRL;
    unsigned long IC_PH_CR;
    unsigned long IC_CON;
    unsigned long IC_TAR;
    unsigned long IC_SAR;
    unsigned long IC_HS_MADDR;
    unsigned long IC_DATA_CMD;
    unsigned long IC_SS_SCL_HCNT;
    unsigned long IC_SS_SCL_LCNT;
    unsigned long IC_FS_SCL_HCNT;
    unsigned long IC_FS_SCL_LCNT;
    unsigned long IC_INTR_STAT;
    unsigned long IC_INTR_MASK;
    unsigned long IC_RAW_INTR_STAT;
    unsigned long IC_RX_TL;
    unsigned long IC_TX_TL;
    unsigned long IC_CLR_INTR;
    unsigned long IC_CLR_RX_UNDER;
    unsigned long IC_CLR_RX_OVER;
    unsigned long IC_CLR_TX_OVER;
    unsigned long IC_CLR_RD_REQ;
    unsigned long IC_CLR_TX_ABRT;
    unsigned long IC_CLR_RX_DONE;
    unsigned long IC_CLR_ACTIVITY;
    unsigned long IC_CLR_STOP_DET;
    unsigned long IC_CLR_START_DET;
    unsigned long IC_CLR_GEN_CALL;
    unsigned long IC_ENABLE;
    unsigned long IC_STATUS;
    unsigned long IC_TXFLR;
    unsigned long IC_RXFLR;
    unsigned long IC_SDA_HOLD;
    unsigned long IC_TX_ABRT_SOURCE;
    unsigned long IC_SLV_DATA_NACK;
    unsigned long IC_SDA_SETUP;
    unsigned long IC_ACK_GEN_CALL;
    unsigned long IC_ENABLE_STATUS;
    unsigned long IC_COMP_PARAM_1;
    unsigned long IC_COMP_VERSION;
    unsigned long IC_COMP_TYPE;
    unsigned long IC_SDA_DEL;
};

typedef struct rtk_i2c_phy_t       rtk_i2c_phy;
typedef struct rtk_input_mux_reg_t rtk_input_mux_reg;
typedef struct rtk_pin_mux_reg_t   rtk_pin_mux_reg;
typedef struct rtk_i2c_port_t      rtk_i2c_port;
typedef struct rtk_i2c_init_reg_t  rtk_i2c_init_reg;

struct rtk_i2c_init_reg_t
{
    unsigned long addr;
    unsigned long mask;
    unsigned long val;
};

struct rtk_input_mux_reg_t
{
    unsigned long addr;
    unsigned long mask;
    unsigned long val;
};

struct rtk_pin_mux_reg_t
{
    unsigned long addr;
    unsigned long mask;
    unsigned long i2c_val;
    unsigned long gpio_val;
};

struct rtk_i2c_port_t
{
    unsigned char gpio_mapped;
    RTK_GPIO_ID g2c_scl;
    RTK_GPIO_ID g2c_sda;
    rtk_input_mux_reg input_mux[2];
    rtk_pin_mux_reg pin_mux[2];
};

struct rtk_i2c_phy_t
{
    const rtk_i2c_reg_map *p_reg_map;
    const unsigned char n_port;
    const rtk_i2c_port *p_port;
    const unsigned int n_init_reg;
    const rtk_i2c_init_reg *p_init_reg;
};

typedef struct
{
    int           (*handle_command) (int id, unsigned char *cmd, unsigned char len, unsigned char stop);
    unsigned char (*read_data)      (int id);
    void          (*stop)           (int id);
} rtk_i2c_slave_ops;

typedef struct rtk_i2c_t rtk_i2c;

struct rtk_i2c_t
{
    unsigned int flags;
#ifndef BUILD_QUICK_SHOW
    atomic_t ref_cnt;
#endif

#define rtk_I2C_IRQ_RDY  0x01
#define rtk_I2C_SLAVE_ENABLE  0x80
    char *model_name;

    unsigned int irq;
    unsigned char logical_id;
    unsigned char id;
    unsigned int spd;
    CLKSEL_MODE clksel_mode;
    unsigned int tick;
    unsigned int guard_interval;
    unsigned short sar;
    ADDR_MODE sar_mode;
    unsigned char disable_switch_port;
    unsigned char i2c_function_disable;
    unsigned char i2c_GPIO_mode;
    unsigned char i2c_loglevel;
    unsigned char polling_mode;
    unsigned char disable_auto_recovery;
    unsigned char fixed_speed;
    unsigned short tar;
    ADDR_MODE tar_mode;

    rtk_i2c_reg_map reg_map;

    unsigned char n_port;

    rtk_i2c_port *p_port;

    rtk_i2c_port *current_port;

    unsigned int n_init_reg;

    const rtk_i2c_init_reg *p_init_reg;

    unsigned char rx_fifo_depth;
    unsigned char tx_fifo_depth;

    unsigned long time_stamp;

    rtk_i2c_xfer xfer;

    unsigned char slave_rx_buffer[64];
    unsigned int slave_rx_len;
    rtk_i2c_slave_ops slave_ops;
    unsigned long slave_id;

    wait_queue_head_t wq;

    spinlock_t lock;
#ifndef BUILD_QUICK_SHOW
    struct mutex mutex;
#endif

    int (*init) (rtk_i2c * p_this);
    int (*uninit) (rtk_i2c * p_this);
    int (*set_spd) (rtk_i2c * p_this, int KHz);
    int (*set_guard_interval) (rtk_i2c * p_this, unsigned long us);
    int (*set_hw_guard_interval) (rtk_i2c * p_this, unsigned long us);
    int (*set_tar) (rtk_i2c * p_this, unsigned short, ADDR_MODE mode);
    int (*set_port) (rtk_i2c * p_this, unsigned char port_id);
    int (*unset_port) (rtk_i2c * p_this, unsigned char port_id);

    int (*read) (rtk_i2c * p_this, unsigned char *tx_buf,
                 unsigned short tx_buf_len, unsigned char *rx_buff,
                 unsigned short rx_buf_len);

    int (*write) (rtk_i2c * p_this, unsigned char *tx_buf,
                  unsigned short tx_buf_len, unsigned char wait_stop);

    int (*gpio_read) (rtk_i2c * p_this, unsigned char *tx_buf,
                      unsigned short tx_buf_len, unsigned char *rx_buff,
                      unsigned short rx_buf_len);

    int (*gpio_write) (rtk_i2c * p_this, unsigned char *tx_buf,
                       unsigned short tx_buf_len, unsigned char wait_stop);

    int (*dump) (rtk_i2c * p_this);
    int (*suspend) (rtk_i2c * p_this);
    int (*resume) (rtk_i2c * p_this);

    unsigned int (*get_tx_abort_reason) (rtk_i2c * p_this);

    int (*get_bus_status) (rtk_i2c * p_this);
    void (*do_bus_jame_recover) (rtk_i2c * p_this);

    int (*set_sar) (rtk_i2c * p_this, unsigned short, ADDR_MODE mode);
    int (*register_slave_ops) (rtk_i2c * p_this,rtk_i2c_slave_ops * ops, unsigned long id);
    int (*slave_mode_enable) (rtk_i2c * p_this, unsigned char on);

    int (*get_hold_time)(rtk_i2c *p_this);
    int (*set_hold_time)(rtk_i2c * p_this, int val);
    int (*get_setup_time)(rtk_i2c * p_this);
    int (*set_setup_time)(rtk_i2c * p_this, int val);
};


#define wr_reg(x,y)			rtd_outl(x, y)
#define rd_reg(x)			rtd_inl(x)

#define SET_I2C_SCPU_INT_EN(x)          wr_reg(ISO_MISC_OFF_SYSTEM_reg,(rd_reg(ISO_MISC_OFF_SYSTEM_reg) | x))
#define SET_IC_GUARD_INTERVAL(adp,x)    wr_reg(adp->reg_map.IC_INTERVAL_MODE_CTRL, x)
#define GET_IC_GUARD_INTERVAL(adp)      rd_reg(adp->reg_map.IC_INTERVAL_MODE_CTRL)
#define GET_IC_GUARD_INTERVAL_ST(adp)   rd_reg(adp->reg_map.IC_GUARD_STATUS)
#define CLR_IC_GUARD_INTERVAL(adp)      wr_reg(adp->reg_map.IC_GUARD_STATUS,0x1)
#define SET_IC_ISR(adp,x)               wr_reg(adp->reg_map.IC_ISR, x)
#define GET_IC_ISR(adp)                 rd_reg(adp->reg_map.IC_ISR)
#define GET_IC_ISR_EN(adp)              rd_reg(adp->reg_map.IC_ISR_EN)
#define SET_IC_ENABLE(adp,x)            wr_reg(adp->reg_map.IC_ENABLE,x)
#define GET_IC_ENABLE(adp)              rd_reg(adp->reg_map.IC_ENABLE)
#define GET_IC_PH_CR(adp)               rd_reg(adp->reg_map.IC_PH_CR)
#define SET_IC_PH_CR(adp, x)            wr_reg(adp->reg_map.IC_PH_CR, x)
#define SET_IC_CON(adp,x)               wr_reg(adp->reg_map.IC_CON, x)
#define GET_IC_CON(adp)                 rd_reg(adp->reg_map.IC_CON)
#define SET_IC_SAR(adp,x)               wr_reg(adp->reg_map.IC_SAR, x)
#define GET_IC_SAR(adp)                 rd_reg(adp->reg_map.IC_SAR)
#define SET_IC_TAR(adp,x)               wr_reg(adp->reg_map.IC_TAR, x)
#define GET_IC_TAR(adp)                 rd_reg(adp->reg_map.IC_TAR)
#define SET_IC_DATA_CMD(adp, x)         wr_reg(adp->reg_map.IC_DATA_CMD, x)
#define GET_IC_DATA_CMD(adp)            rd_reg(adp->reg_map.IC_DATA_CMD)
#define SET_IC_SS_SCL_HCNT(adp,x)       wr_reg(adp->reg_map.IC_SS_SCL_HCNT, x)
#define SET_IC_SS_SCL_LCNT(adp,x)       wr_reg(adp->reg_map.IC_SS_SCL_LCNT, x)
#define SET_IC_FS_SCL_HCNT(adp,x)       wr_reg(adp->reg_map.IC_FS_SCL_HCNT, x)
#define SET_IC_FS_SCL_LCNT(adp,x)       wr_reg(adp->reg_map.IC_FS_SCL_LCNT, x)
#define GET_IC_STATUS(adp)              rd_reg(adp->reg_map.IC_STATUS)
#define SET_IC_INTR_MASK(adp,x)         wr_reg(adp->reg_map.IC_INTR_MASK, x)
#define GET_IC_INTR_MASK(adp)           rd_reg(adp->reg_map.IC_INTR_MASK)
#define GET_IC_INTR_STAT(adp)           rd_reg(adp->reg_map.IC_INTR_STAT)
#define GET_IC_RAW_INTR_STAT(adp)       rd_reg(adp->reg_map.IC_RAW_INTR_STAT)
#define CLR_IC_INTR(adp)                rd_reg(adp->reg_map.IC_CLR_INTR)
#define CLR_IC_RX_UNDER(adp)            rd_reg(adp->reg_map.IC_CLR_RX_UNDER)
#define CLR_IC_TX_OVER(adp)             rd_reg(adp->reg_map.IC_CLR_TX_OVER)
#define CLR_IC_RD_REQ(adp)              rd_reg(adp->reg_map.IC_CLR_RD_REQ)
#define CLR_IC_RX_DONE(adp)             rd_reg(adp->reg_map.IC_CLR_RX_DONE)
#define CLR_IC_ACTIVITY(adp)            rd_reg(adp->reg_map.IC_CLR_ACTIVITY)
#define CLR_IC_GEN_CALL(adp)            rd_reg(adp->reg_map.IC_CLR_GEN_CALL)
#define CLR_IC_TX_ABRT(adp)             rd_reg(adp->reg_map.IC_CLR_TX_ABRT)
#define CLR_IC_STOP_DET(adp)            rd_reg(adp->reg_map.IC_CLR_STOP_DET)
#define GET_IC_COMP_PARAM_1(adp)        rd_reg(adp->reg_map.IC_COMP_PARAM_1)
#define GET_IC_TXFLR(adp)               rd_reg(adp->reg_map.IC_TXFLR)
#define GET_IC_RXFLR(adp)               rd_reg(adp->reg_map.IC_RXFLR)
#define GET_IC_RX_TL(adp)               rd_reg(adp->reg_map.IC_RX_TL)
#define GET_IC_TX_TL(adp)               rd_reg(adp->reg_map.IC_TX_TL)
#define GET_IC_SDA_DEL(adp)             rd_reg(adp->reg_map.IC_SDA_DEL)
#define SET_IC_RX_TL(adp, x)            wr_reg(adp->reg_map.IC_RX_TL, x)
#define SET_IC_TX_TL(adp, x)            wr_reg(adp->reg_map.IC_TX_TL, x)
#define SET_IC_SDA_DEL(adp, x)          wr_reg(adp->reg_map.IC_SDA_DEL, x)
#define GET_IC_SDA_HOLD(adp)            rd_reg(adp->reg_map.IC_SDA_HOLD)
#define SET_IC_SDA_HOLD(adp, x)         wr_reg(adp->reg_map.IC_SDA_HOLD, x)
#define GET_IC_SDA_SETUP(adp)           rd_reg(adp->reg_map.IC_SDA_SETUP)
#define SET_IC_SDA_SETUP(adp, x)        wr_reg(adp->reg_map.IC_SDA_SETUP, x)
#define GET_IC_TX_ABRT_SOURCE(adp)      rd_reg(adp->reg_map.IC_TX_ABRT_SOURCE)
#define GET_IC_CLKSEL_MODE(adp)         ((rd_reg(adp->reg_map.IC_CLKSEL) & adp->reg_map.IC_CLKSEL_MASK) ? CLKSEL_MODE_62_5MHZ : CLKSEL_MODE_27MHZ)
#define NOT_TXFULL(adp)                 (GET_IC_STATUS(adp) & ST_TFNF_BIT)
#define NOT_RXEMPTY(adp)                (GET_IC_STATUS(adp) & ST_RFNE_BIT)
#define NOT_TXFULL_EX(adp)              (GET_IC_TXFLR(adp)  < ((adp)->tx_fifo_depth - 2))

#define i2c_print    rtd_printk
#define dbg_char(x)  wr_reg(0xb801b200, (unsigned long) (x))


#define I2C_INFO(fmt, args...)		rtd_pr_i2c_info("[Info]" fmt, ## args)
#define I2C_WARNING(fmt, args...)	rtd_pr_i2c_warn("[Warn]" fmt, ## args)
#define I2C_DEBUG(fmt, args...)		rtd_pr_i2c_debug("[Debug]" fmt, ## args)

extern unsigned char I2C_error_log_onoff;

#define RTK_I2C_INFO(fmt, args...)  \
{ \
    if (I2C_error_log_onoff)  \
    {   \
        rtd_pr_i2c_info("[Info]" fmt, ## args); \
    }   \
}

#define RTK_I2C_WARNING(fmt, args...)  \
{ \
    if (I2C_error_log_onoff)  \
    {   \
        rtd_pr_i2c_warn("[Warn]" fmt, ## args); \
    }   \
}

#define RTK_I2C_DEBUG(fmt, args...)  \
{ \
    if (I2C_error_log_onoff)  \
    {   \
        rtd_pr_i2c_debug("[Debug]" fmt, ## args); \
    }   \
}

#ifdef SPIN_LOCK_PROTECT_EN
#define LOCK_rtk_I2C(a,b)         spin_lock_irqsave(a, b)
#define UNLOCK_rtk_I2C(a, b)      spin_unlock_irqrestore(a, b)
#else
#define LOCK_rtk_I2C(a,b)         do { b = 1; }while(0)
#define UNLOCK_rtk_I2C(a, b)      do { b = 0; }while(0)
#endif

#ifdef I2C_PROFILEING_EN
#define LOG_EVENT(x)                    log_event(x)
#else
#define LOG_EVENT(x)
#endif


#define SA_SHIRQ IRQF_SHARED


#define rtk_i2c_has_mapped_gpio(p_this)   (p_this->current_port && p_this->current_port->gpio_mapped)
#define current_port_id(p_this)           ((p_this->current_port) ? (((unsigned long) p_this->current_port - (unsigned long) p_this->p_port)/sizeof(rtk_i2c_port)) : -1)



#ifdef CONFIG_RTK_KDRV_I2C_BUS_JAM_RECOVER

#define I2C_HW_GUARD

int rtk_i2c_bus_jam_detect(rtk_i2c * p_this);
void rtk_i2c_bus_jam_recover_proc(rtk_i2c * p_this);

#ifdef I2C_HW_GUARD
int rtk_i2c_bus_jam_detect_hw_guard(rtk_i2c * p_this);
void rtk_i2c_bus_jam_recover_proc_hw_guard(rtk_i2c * p_this);
#endif

#endif



void i2c_power(int);
int rtk_i2c_get_dev_info_by_pcbenum(void);
char *rtk_i2c_get_dev_name(unsigned char logiacl_id, unsigned char addr);
int rtk_i2c_dump_dev_info(unsigned char logical_id, char *buf);

rtk_i2c *create_rtk_i2c_handle(unsigned char logical_id,
                               unsigned char id,
                               unsigned short sar,
                               ADDR_MODE sar_mode,
                               SPD_MODE spd, unsigned int irq);

rtk_i2c *create_rtk_g2c_handle(unsigned char id, RTK_GPIO_ID sda, RTK_GPIO_ID scl);

void destroy_rtk_i2c_handle(rtk_i2c * hHandle);

unsigned char get_rtk_i2c_phy_count(void);


int rtk_i2c_start_xfer(rtk_i2c * p_this);

/* g2c section */

void i2c_power(int state);

int rtk_i2c_set_guard_interval(rtk_i2c * p_this, unsigned long us);

unsigned int rtk_i2c_get_tx_abort_reason(rtk_i2c * p_this);

void rtk_i2c_gpio_selection(rtk_i2c * p_this, unsigned char mode);

int rtk_i2c_get_mapped_gpio(rtk_i2c * p_this, RTK_GPIO_ID * p_sda, RTK_GPIO_ID * p_scl);

int rtk_i2c_write(rtk_i2c * p_this,
	unsigned char *tx_buf,
	unsigned short tx_buf_len, unsigned char wait_stop) ;

int rtk_i2c_read(rtk_i2c * p_this,
	unsigned char *tx_buf,
	unsigned short tx_buf_len,
	unsigned char *rx_buf, unsigned short rx_buf_len);

int rtk_i2c_load_message(rtk_i2c * p_this,
	unsigned char mode,
	unsigned char *tx_buf,
	unsigned short tx_buf_len,
	unsigned char *rx_buf,
	unsigned short rx_buf_len, unsigned char xfer_flags) ;

int rtk_i2c_register_slave_ops(rtk_i2c * p_this,
	rtk_i2c_slave_ops * ops, unsigned long id);

int rtk_g2c_read(rtk_i2c * p_this,
	unsigned char *tx_buf,
	unsigned short tx_buf_len,
	unsigned char *rx_buf, unsigned short rx_buf_len);

int rtk_g2c_write(rtk_i2c * p_this,
	unsigned char *tx_buf,
	unsigned short tx_buf_len, unsigned char wait_stop);

/* g2c section */

#endif
