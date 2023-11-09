#ifndef __RTK_MD_HW_H__
#define __RTK_MD_HW_H__
#include <linux/semaphore.h>

/***************************MD error define*******************************/
#define MD_SUCCESS              (0)
#define MD_ERR_WRITE_FAIL       (-1)
#define MD_ERR_NO_SEM           (-2)

/*********************SOME GLOBAL DEFINES*******************************/

#ifdef CONFIG_ARM64
#define DSB                                     dsb(sy)
#else
#define DSB                                     dsb()
#endif

#define MD_ONE_CMD_LEN                         16
#define MD_WRITE_MAX_SIZE                    16384
#define MD_CMD_BUF                                   (4096*sizeof(uint32_t))
#define MD_SEQ_LEN_LIMIT                (0x80000000 - 16)
#define MD_SEQ_LEN_LIMIT_MASK           (0x7fffffff)


/*****************MD DESCRPT RELEATED DEFINES***************************/
#define MdClearWriteData                (0)
#define MD_CONST                        BIT(6)
#define MD_MOVE_DIR                     BIT(7)

/*Word 1 bit[3:0] : opcode */
#define MD_1B                           (0x05)
#define MD_2B                           (0x07)
#define MD_4B                           (0x0D)

/*Word 1 bit[5:4] : move mode */
#define MD_SS                           (0x00) /*seq -> seq */
#define MD_SB                           (0x10) /*seq -> blk */
#define MD_BS                           (0x20) /*blk -> seq */
#define MD_SS_PITCH                     (0x30) /*seq -> seq (pitch) */

#define MD_2B_SS                        (MD_SS|MD_2B)
#define MD_2B_SS_PITCH                  (MD_SS_PITCH|MD_2B)

#define MD_4B_SS                        (MD_SS|MD_4B)
#define MD_4B_SS_PITCH                  (MD_SS_PITCH|MD_4B)

#define MD_SRC_PITCH(x)         ((x & 0x1FFF)<<19)
#define MD_MAX_SRC_PITCH        (0x2000)
#define MD_DST_PITCH(x)         ((x & 0x1FFF)<<6)
#define MD_MAX_DST_PITCH        (0x2000)
#define MD_HEIGHT_BLK(x)        ((x & 0x7FF)<<21)
#define MD_MAX_HEIGHT_BLOCK     (0x800)
#define MD_WIDTH_BLK(x)         ((x & 0x3FFF)<<7)
#define MD_MAX_WIDTH_BLOCK      (0x4000)



/**********************SOME REGS RELATED DEFINES*************************/

typedef enum {
    MdWriteData     = BIT(0),
    MdGo            = BIT(1),
    MdEndianSwap    = BIT(2),
    MdIdle          = BIT(3)
} MD_CTRL_REG;


/*interrupt status and control bits */
typedef enum {
    MdIntCmdOpcodeError     = BIT(1),
    MdIntCmdLengthError     = BIT(2),
    MdIntCommandEmpty       = BIT(3),
    MdIntFlashCmdDone       = BIT(4),
    MdInt2B4BModeError      = BIT(5),
    MdIntTxThreshold        = BIT(6),
    MdIntRxThreshold        = BIT(7),
    MdIntRxTimeOut          = BIT(8),
    MdIntTxRxLengthError    = BIT(9),
    MdIntSCPUInt            = BIT(10)
} MD_INT_STATUS_REG, MD_INT_CTRL_REG;


struct md_dev {
    volatile void *CmdBuf;
    volatile void *CmdBase;
    volatile void *CmdLimit;
    volatile void *CmdWptr;
    int wrptr;
    int cmd_cnt;
    volatile int size;
    volatile uint64_t u64InstCnt;
    volatile uint64_t u64IssuedInstCnt;
    volatile int queue;
    struct semaphore sem;
    wait_queue_head_t  wait_queue __attribute__((__aligned__(4)));
    volatile int CmdDone;
    struct device *md_device;
};

/*********************SOME MD STATE RELEATED DEFINES***********************/
#define SMQ_STATUS                      BIT(0)
#define KMQ_STATUS                      BIT(1)
#define SMQ_SELF_CHECK          BIT(2)
#define STOP                            0
#define START                   1
#define RESET                   1
#define FINISH                  0

/*

DUMMY1
           +----------+----------+----------+----------+
bit 4 ~ 0  |  No Use  |  No Use  |  No Use  |SMQ Status|
           +----------+----------+----------+----------+

DUMMY2
           +----------+----------+----------+----------+
bit 4 ~ 0  |  No Use  |SELF_CHECK|KMQ Status|  No Use  |
           +----------+----------+----------+----------+
*/

#define GET_DUMMY1_REG(bit)      (rtd_inl(MD_DUMMY1_reg) & bit)
#define SET_DUMMY1_REG(bit, val) val==0?rtd_clearbits(MD_DUMMY1_reg, bit):rtd_setbits(MD_DUMMY1_reg, bit)
#define GET_DUMMY2_REG(bit)      (rtd_inl(MD_DUMMY2_reg) & bit)
#if defined(CONFIG_REALTEK_SECURE) && !defined(CONFIG_RTK_KDRV_OPENBSD)
#define GET_KMQ_STATUS()                (GET_DUMMY2_REG(KMQ_STATUS) >> 1)
#else
#define GET_KMQ_STATUS()                STOP
#endif
#define GET_SMQ_SELF_CHECK()    (GET_DUMMY2_REG(SMQ_SELF_CHECK) >> 2)
#define SET_DUMMY2_REG(bit, val) val==0?rtd_clearbits(MD_DUMMY2_reg, bit):rtd_setbits(MD_DUMMY2_reg, bit)



extern bool rtk_md_is_clk_on(void);
extern void rtk_md_dump_reg(char *buf);
extern void md_dump_cmd_ring(struct md_dev *pdev, unsigned int count, char *buf);
extern void rtk_md_init_kv_reg(void);
extern void rtk_md_store_kv_reg(void);
extern void rtk_md_init_reg(struct md_dev *pdev, uint32_t set_multi_req);
extern void rtk_md_reset_reg(struct md_dev *pdev, uint32_t set_multi_req);
extern void rtk_md_enable_mult_reg(uint32_t on_off);
extern bool rtk_md_check_hw_run_state(void);
extern bool rtk_md_check_empty_event(void);
extern void rtk_md_on_off_empty_int(uint32_t on_off);
extern void rtk_md_clear_empty_int_status(void);
extern uint16_t rtk_md_get_hw_instcnt(void);
extern void rtk_md_update_inst_count(struct md_dev *pdev);
extern void rtk_md_set_dummy_1_reg(uint32_t event, uint32_t value);
extern void rtk_md_set_dummy_2_reg(uint32_t event, uint32_t value);

extern void rtk_md_print_sk_md_status(const char *function);

extern int32_t rtk_write_cmd_wrapper(struct md_dev *dev,
                                uint8_t *pbyCommandBuffer, uint32_t lCommandLength);
extern int rtk_md_memfill(uint32_t addrdst, int32_t len,
            uint32_t bytemode, uint32_t movemode, uint32_t val);
extern int rtk_md_hw_power_down(void);
void rtk_md_hw_set_burst_len(int byte);
int  rtk_md_hw_get_burst_len(void);
int rtk_md_hw_fdma_copy(uint32_t flash_address, unsigned char *buf,
              uint32_t len, uint32_t fdma_max_xfer, uint32_t fdma_dir);
void * rtk_md_phys_to_virt(unsigned long phy_addr);
#endif
