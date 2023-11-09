#ifndef __SPI_RTK_H__
#define __SPI_RTK_H__

#include <linux/spi/spi.h>
#include <rbus/sb2_reg.h>
#include <rbus/md_reg.h>
#include <rtk_kdriver/io.h>

#define MD_PP_DATA_SIZE_SHIFT   8       /* 256 bytes */
#define MD_PP_DATA_SIZE             (1 << MD_PP_DATA_SIZE_SHIFT)

#define SPI_REG_NUM                 0x8
#define SPI_REG_BASE                SB2_SFC_OPCODE_reg

#define POS_LATCH_FALL              0
#define POS_LATCH_RISE              1

#define CMD_READ_ID                 0x9f
#define CMD_READ_STATUS1            0x05
#define CMD_READ_STATUS2            0x35
#define CMD_READ_STATUS3            0x15
#define CMD_WRITE_ENABLE            0x06
#define CMD_WRITE_STATUS1           0X01
#define CMD_WRITE_STATUS2           0X31
#define CMD_WRITE_STATUS3           0X11
#define CMD_PAGE_PROGRAM            0x02
#define CMD_PAGE_PROGRAM_QUAD       0x32
#define CMD_READ_NORMAL             0x03
#define CMD_READ_NORMAL_FAST        0x0b
#define CMD_READ_DUAL_FAST          0x3b
#define CMD_READ_QUAD_FAST          0x6b
#define CMD_ERASE_BLOCK_64K         0xd8
#define CMD_ERASE_BLOCK_32K         0x52
#define CMD_ERASE_SECTOR_4K         0x20
#define CMD_ENABLE_RESET            0x66
#define CMD_RESET                   0x99


/* SPI STATUS REGISTER */
#define WIP_OFFSET                  0
#define WEL_OFFSET                  1
#define STATUS_WIP                  0x01

/*MD define*/
#define FLASH_2_DDR                 0
#define DDR_2_FLASH                 1

#define SET_DMA_LEN_512             2

/*step auto tune param*/
#define DELAY_SEL_STEP_TOTAL_CNT     12
#define DELAY_SEL_STEP_GATE_CNT      8

/************************************************************************
 *  Define misc
 ************************************************************************/
#define SFC_DELAY                   100
#define SFC_DELAY_TW                5000           // write status register cycle time
#define SN_SYNC                     __asm__ __volatile__ ("DSB sy;")
#define SUSPEND_TIMEOUT             25000
#define ERASE_DATA_TIMEOUT          25000 // 2500ms
#define WRITE_DATA_TIMEOUT          100   // 5ms
#define SET_FEATURE_TIMEOUT         1000   // 50ms

#define SFC_WP_MASK_BIT2_TO_BIT7    0xfc
#define SFC_WP_MASK_BIT6            0x40
#define SFC_WP_MASK_BIT2            0x04

/***********************************************************************/

/***********************************************************************/
struct spi_master_clk_t
{
	unsigned int clock_div;
	unsigned int delay_sel;
};

typedef enum {
    SM_READY,
    SM_WORKING,
    SM_SUSPEND,
}spi_master_device_state_t;

/* runtime info for spi master */
struct rtk_spi_master {
	struct spi_master *master;     /* SPI framework hookup */
	
	unsigned long base_addr;       /*SPI base address*/
	
	unsigned short current_mode;   /*current mode*/
	unsigned int current_clock;    /*current clock*/
	unsigned char current_bits_per_word; /*current bits_per_word*/
	
	unsigned int max_clock;        /*support max clock*/
	unsigned char current_clock_div;
    unsigned char current_delay_sel;
	unsigned char min_clock_div;
	
	/* device get and release control */
	spinlock_t spi_lock;
	wait_queue_head_t wq;	
	spi_master_device_state_t	state;

	spinlock_t spi_lock_priv;
	unsigned long spi_lock_flags;

	/*register value store buffer for suspend and resume */
	unsigned int *reg_array;
	
	/*transfer datas*/
	int (*transfer)(struct rtk_spi_master *master, const unsigned char *tx_data, unsigned char *rx_data, unsigned int len);
	/*receive data*/
	int (*receive)(struct rtk_spi_master *master, const unsigned char *tx_data, unsigned char *rx_data, unsigned int len);
};

#endif
