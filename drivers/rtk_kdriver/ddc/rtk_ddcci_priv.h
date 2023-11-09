#ifndef __RTK_DDCCI_PRIV_H__
#define __RTK_DDCCI_PRIV_H__
#include <linux/wait.h>
#include <linux/timer.h>
#include <linux/platform_device.h>
#include <ioctrl/ddcci/ddcci_cmd_id.h>

#define RTK_DDCCI_NAME_MAX        24

typedef struct {
    unsigned char data[DDC_BUF_LENGTH];
    unsigned char len;
    unsigned char tx_en;
} rtk_ddcci_xmit;

typedef struct {
    unsigned char data[DDC_BUF_LENGTH];
    unsigned char len;
    unsigned char rx_en;
} rtk_ddcci_rcv;

typedef struct {
    rtk_ddcci_xmit *xmit;
    rtk_ddcci_rcv *rcv;
    int rx_num;
    int tx_num;
    spinlock_t lock;
} rtk_ddcci;

typedef enum
{
    DDCCI_VGA,
    DDCCI_HDMI,
    DDCCI_DP,
    DDCCI_TYPEC,
    MAX_DDCCI_TYPE
} ERTK_DDCCI_TYPE;

typedef struct srtk_ddcci_reg_map_t            srtk_ddcci_reg_map;
typedef struct srtk_ddcci_chip_t               srtk_ddcci_chip;


struct srtk_ddcci_reg_map_t
{
    unsigned long DDC_DDC_SFCR;
    unsigned long DDC_DDC_SFSAR;
    unsigned long DDC_DDC_SFDR;
    unsigned long DDC_DDC_SFBSR;
    unsigned long DDC_DDC_SFIBLR;
    unsigned long DDC_DDC_SFOBSR;
    unsigned long DDC_DDC_SFIR;
    unsigned long DDC_DDC_SFSR;
};


struct srtk_ddcci_chip_t
{
    const int                       id;
    const srtk_ddcci_reg_map        *reg_remap;
    int                             is_fs_registered;
    char                            name[RTK_DDCCI_NAME_MAX];
    ERTK_DDCCI_TYPE                 ddcci_type;
    struct class                    class;
};

static rtk_ddcci_xmit p_xmit;
static rtk_ddcci_rcv p_rcv;

extern struct task_struct *g_rtk_ddcci_fifo_thread;
extern struct task_struct *g_rtk_ddcci_dbg_thread;
extern unsigned char g_ddcci_connect_flag;

int rtk_ddcci_interrupt(int ddc_irq_t);
int rtk_ddcci_free_interrupt(int ddc_irq_t);
void rtk_ddcci_lib_init(unsigned char id);
int rtk_ddcci_xmit_message(unsigned char *tx_data, unsigned char len);
int rtk_ddcci_rcv_message(unsigned char *rx_data, unsigned char* len);

bool rtk_ddcci_pooling_t(void);
void rtk_ddcci_isr_tx_fifo_t(void);
int rtk_ddcci_fifo_thread(void* arg);

#endif  //__RTK_DDCCI_PRIV_H__