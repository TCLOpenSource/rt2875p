#include <linux/module.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/dma-direction.h>
#include <linux/dma-mapping.h>
#include <md/rtk_md.h>
#include <asm/cacheflush.h>
#include <mach/platform.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <rtd_log/rtd_module_log.h>

#include <rbus/sb2_reg.h>
#include <rbus/md_reg.h>

#include "rtk_spinor.h"

//#define CONFIG_RTK_SPI_WP_HW
//#define SUPPORT_SFC_MD
/************************************************************************
 *  Defines
 ************************************************************************/
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
#define CMD_ENTER_4BYTES_ADDR_MODE	0xB7
#define CMD_EXIT_4BYTES_ADDR_MODE	0xE9

#define CMD_SOFTWARE_DIE_SELECT		0xC2


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
struct clock_match_t
{
	unsigned int clock_div;
	unsigned int delay_sel;
};

#ifndef CONFIG_CUSTOMER_TV006
struct clock_match_t clock_match[] = 
{
	{0x20, 0x00}, // 15.1515MHz
	{0x21, 0x00}, // 14.71MHz
	{0x25, 0x00}, // 13.16MHz
	{0x27, 0x00}, // 12.50MHz
	{0x2B, 0x00}, // 11.36MHz
	{0x31, 0x00}, // 10MHz
};
#else
struct clock_match_t clock_match[] = 
{
	{0x20, 0x00}, // 15.1515MHz
	{0x21, 0x00}, // 14.71MHz
	{0x25, 0x00}, // 13.16MHz
	{0x27, 0x00}, // 12.50MHz
	{0x2B, 0x00}, // 11.36MHz
	{0x31, 0x00}, // 10MHz
};
#endif

void __iomem *spi_virt_base = NULL;
unsigned long SPI_BASE_ADDR  = 0;

static int sfc_get_device(struct spinor_chip_t *spinor_chip, int new_state)
{
	DECLARE_WAITQUEUE(wait, current);

	/*Grab the lock and see if the device is available*/
	while (1) {
		spin_lock(&spinor_chip->spinor_lock);
		if (spinor_chip->state == FL_READY) {
			spinor_chip->state = new_state;
			spin_unlock(&spinor_chip->spinor_lock);
			break;
		}

		if (new_state == FL_SUSPEND) {
			spin_unlock(&spinor_chip->spinor_lock);
			return (spinor_chip->state == FL_SUSPEND) ? 0 : -EAGAIN;
		}

		set_current_state(TASK_UNINTERRUPTIBLE);
		add_wait_queue(&spinor_chip->wq, &wait);
		spin_unlock(&spinor_chip->spinor_lock);
		schedule();
		remove_wait_queue(&spinor_chip->wq, &wait);
	}
	return 0;
}

static void sfc_release_device(struct spinor_chip_t *spinor_chip)
{
	/* Release the chip */
	spin_lock(&spinor_chip->spinor_lock);
	spinor_chip->state = FL_READY;
	wake_up(&spinor_chip->wq);
	spin_unlock(&spinor_chip->spinor_lock);
}

void sfc_debug(void)
{
	rtd_pr_spi_nor_err("\n%x: %x\n", SB2_SFC_OPCODE_reg, rtd_inl(SB2_SFC_OPCODE_reg));
	rtd_pr_spi_nor_err("%x: %x\n", SB2_SFC_CTL_reg, rtd_inl(SB2_SFC_CTL_reg));
	rtd_pr_spi_nor_err("%x: %x\n", SB2_SFC_SCK_reg, rtd_inl(SB2_SFC_SCK_reg));
	rtd_pr_spi_nor_err("%x: %x\n", SB2_SFC_CE_reg, rtd_inl(SB2_SFC_CE_reg));
	rtd_pr_spi_nor_err("%x: %x\n", SB2_SFC_WP_reg, rtd_inl(SB2_SFC_WP_reg));
	rtd_pr_spi_nor_err("%x: %x\n", SB2_SFC_POS_LATCH_reg, rtd_inl(SB2_SFC_POS_LATCH_reg));
	rtd_pr_spi_nor_err("%x: %x\n", SB2_SFC_WAIT_WR_reg, rtd_inl(SB2_SFC_WAIT_WR_reg));
	rtd_pr_spi_nor_err("%x: %x\n", SB2_SFC_EN_WR_reg, rtd_inl(SB2_SFC_EN_WR_reg));
}

static unsigned int swap_endian(unsigned int input)
{
	unsigned int output;

	output = (input & 0xff000000) >> 24 |
		(input & 0x00ff0000) >> 8 |
		(input & 0x0000ff00) << 8 |
		(input & 0x000000ff) << 24;

	return output;
}

static void sfc_enable_4bytes_addr_mode(void)
{
	unsigned char tmp = 0;

	/*enable spi device 4bytes address mode*/
	rtd_outl(SB2_SFC_OPCODE_reg, SB2_SFC_OPCODE_opc(CMD_ENTER_4BYTES_ADDR_MODE));
	rtd_outl(SB2_SFC_CTL_reg,  SB2_SFC_CTL_dataen(0) | SB2_SFC_CTL_adren(0) | SB2_SFC_CTL_dmycnt(0));
	SN_SYNC;
	udelay(SFC_DELAY);
	/*trigger cmd*/
	tmp = rtd_inb(SPI_BASE_ADDR);

	/*enable spi controler 4bytes address mode*/
	rtd_outl(SB2_SFC_OPCODE2_reg, SB2_SFC_OPCODE2_adr4byte_en(1));
}

static void sfc_disable_4bytes_addr_mode(void)
{
	unsigned char tmp = 0;

	/*disable spi device 4bytes address mode*/
	rtd_outl(SB2_SFC_OPCODE_reg, SB2_SFC_OPCODE_opc(CMD_EXIT_4BYTES_ADDR_MODE));
	rtd_outl(SB2_SFC_CTL_reg, SB2_SFC_CTL_dataen(0) | SB2_SFC_CTL_adren(0) | SB2_SFC_CTL_dmycnt(0));
	SN_SYNC;
	udelay(SFC_DELAY);
	/*trigger cmd*/
	tmp = rtd_inb(SPI_BASE_ADDR);

	/*disable spi controler 4bytes address mode*/
	rtd_outl(SB2_SFC_OPCODE2_reg, SB2_SFC_OPCODE2_adr4byte_en(0x0));
}

static void sfc_enable_write(void)
{
	unsigned char tmp;
	
	rtd_outl(SB2_SFC_OPCODE_reg, SB2_SFC_OPCODE_opc(CMD_WRITE_ENABLE));
	rtd_outl(SB2_SFC_CTL_reg, SB2_SFC_CTL_dataen(0) | SB2_SFC_CTL_adren(0) | SB2_SFC_CTL_dmycnt(0));  // data_en=0 addr_en=0 dmy=0
	
	SN_SYNC;
	udelay(SFC_DELAY);
	
	tmp = rtd_inb(SPI_BASE_ADDR);
	return;
}

static void sfc_get_feature(unsigned char rdsr, unsigned char *data)
{
	rtd_outl(SB2_SFC_OPCODE_reg, SB2_SFC_OPCODE_opc(rdsr));
	rtd_outl(SB2_SFC_CTL_reg, SB2_SFC_CTL_dataen(1) | SB2_SFC_CTL_adren(0) | SB2_SFC_CTL_dmycnt(0));
	
	SN_SYNC;
	udelay(SFC_DELAY);
	
	*data = rtd_inb(SPI_BASE_ADDR);
	SN_SYNC;
	return;
}

static int sfc_set_feature(unsigned char wrsr, unsigned char data)
{
	unsigned char tmp;
	int ret = 0, retry = 0;

	sfc_enable_write();

	rtd_outl(SB2_SFC_OPCODE_reg, SB2_SFC_OPCODE_opc(wrsr));
	rtd_outl(SB2_SFC_CTL_reg, SB2_SFC_CTL_dataen(1) | SB2_SFC_CTL_adren(0) | SB2_SFC_CTL_dmycnt(0));
	SN_SYNC;
	udelay(SFC_DELAY);
	rtd_outb(SPI_BASE_ADDR, data);

	do {
		udelay(SFC_DELAY);
		sfc_get_feature(CMD_READ_STATUS1, &tmp);
	} while ((tmp & STATUS_WIP) && (++retry < SET_FEATURE_TIMEOUT));

	if (retry == SET_FEATURE_TIMEOUT) {
		rtd_pr_spi_nor_err("%s wait timet out  0x%x\n", __FUNCTION__, tmp);
		ret = -1;
	}

	return ret;
}

static int sfc_set_feature_16bit(unsigned char wrsr, unsigned short data)
{
	unsigned char tmp;
	int ret = 0, retry = 0;

	sfc_enable_write();

	rtd_outl(SB2_SFC_OPCODE_reg, SB2_SFC_OPCODE_opc(wrsr));
	rtd_outl(SB2_SFC_CTL_reg, SB2_SFC_CTL_dataen(1) | SB2_SFC_CTL_adren(0) | SB2_SFC_CTL_dmycnt(0));
	SN_SYNC;
	udelay(SFC_DELAY);
	rtd_outw(SPI_BASE_ADDR, data);

	do {
		udelay(SFC_DELAY);
		sfc_get_feature(CMD_READ_STATUS1, &tmp);
	} while ((tmp & STATUS_WIP) && (++retry < SET_FEATURE_TIMEOUT));

	if (retry == SET_FEATURE_TIMEOUT) {
		rtd_pr_spi_nor_err("%s wait timet out  0x%x\n", __FUNCTION__, tmp);
		ret = -1;
	}
	
	return ret;
}

static int sfc_wait_erase_done(void)
{
	int ret = 0, retry = 0;
	unsigned char data = 0;

	do {
		udelay(SFC_DELAY);
		sfc_get_feature(CMD_READ_STATUS1, &data);
	} while ((data & STATUS_WIP) && (++retry < ERASE_DATA_TIMEOUT));

	if (retry == ERASE_DATA_TIMEOUT) {
		rtd_pr_spi_nor_err("%s wait timet out  0x%x\n", __FUNCTION__, data);
		ret = -1;
	}

	return ret;
}

static int sfc_wait_write_done(void)
{
	int ret = 0, retry = 0;
	unsigned char data = 0;

	do {
		udelay(SFC_DELAY);
		sfc_get_feature(CMD_READ_STATUS1, &data);
	} while ((data & STATUS_WIP) && (++retry < WRITE_DATA_TIMEOUT));

	if (retry == WRITE_DATA_TIMEOUT) {
		rtd_pr_spi_nor_err("%s wait timet out  0x%x\n", __FUNCTION__, data);
		ret = -1;
	}
	
	return ret;
}

static void sfc_die_select(unsigned char die_id)
{
    /*select die*/	
    rtd_outl(SB2_SFC_OPCODE_reg, SB2_SFC_OPCODE_opc(CMD_SOFTWARE_DIE_SELECT));
    rtd_outl(SB2_SFC_CTL_reg, SB2_SFC_CTL_dataen(1) | SB2_SFC_CTL_adren(0) | SB2_SFC_CTL_dmycnt(0));
    SN_SYNC;	
    udelay(SFC_DELAY);
    
    rtd_outb(SPI_BASE_ADDR, die_id);
}

#ifdef MARK_NOT_USE
static int sfc_wait_set_feature_done(void)
{
	int ret = 0, retry = 0;
	unsigned char data = 0;

	do {
		udelay(SFC_DELAY);
		sfc_get_feature(CMD_READ_STATUS1, &data);
	} while ((data & STATUS_WIP) && (++retry < SET_FEATURE_TIMEOUT));

	if (retry == SET_FEATURE_TIMEOUT) {
		rtd_pr_spi_nor_err("%s wait timet out  0x%x\n", __FUNCTION__, data);
		ret = -1;
	}
	
	return ret;
}
#endif

// return 1 : quad enable  return 0: quad disable
static int sfc_check_quad(spinor_info_t *spinor_info)
{
	unsigned char data = 0;

	sfc_get_feature(spinor_info->cmd_quad_rdsr, &data);
	
	if(data & spinor_info->cmd_quad_bit)
		return 1;
	else
		return 0;
}

static int sfc_enable_quad(spinor_info_t *spinor_info)
{
	unsigned char data = 0;
	unsigned char tmp = 0;
	volatile unsigned short data_16bit = 0xffff;
	int ret = 0;

	if(spinor_info->id == WINBOND_W32Q64_64Mbit) {
		sfc_get_feature(CMD_READ_STATUS1, &data);
		data_16bit = data;
		sfc_get_feature(spinor_info->cmd_quad_rdsr, &data);
		data = data | spinor_info->cmd_quad_bit;
		data_16bit = ((data << 8) | data_16bit);
		
		ret = sfc_set_feature_16bit(spinor_info->cmd_quad_wrsr, data_16bit);
		if(ret < 0) {
			return ret;
		}
		
		sfc_get_feature(spinor_info->cmd_quad_rdsr, &tmp);
		if(tmp != data) {
			rtd_pr_spi_nor_err("%s 0x%x != 0x%x\n", __FUNCTION__, tmp, data);
			return -1;
		}
	} else {
		sfc_get_feature(spinor_info->cmd_quad_rdsr, &data);
		data = data | spinor_info->cmd_quad_bit;
		
		ret = sfc_set_feature(spinor_info->cmd_quad_wrsr, data);
		if(ret < 0) {
			return ret;
		}
		
		sfc_get_feature(spinor_info->cmd_quad_rdsr, &tmp);
		if(tmp != data) {
			rtd_pr_spi_nor_err("%s 0x%x != 0x%x\n", __FUNCTION__, tmp, data);
			return -1;
		}
	}
	return ret;
}

#ifdef MARK_NOT_USE
static int sfc_disable_quad(spinor_info_t *spinor_info)
{
	unsigned char data = 0;
	int ret = 0;

	sfc_get_feature(spinor_info->cmd_quad_rdsr, &data);
	data = data & (~spinor_info->cmd_quad_bit);
	ret = sfc_set_feature(spinor_info->cmd_quad_wrsr, data);

	return ret;
}
#endif

static void switch_read_mode(spinor_info_t *spinor_info)
{
	switch(spinor_info->read_mode) {
		case eMODE_READ_NORMAL:
			rtd_outl(SB2_SFC_OPCODE_reg, SB2_SFC_OPCODE_opc(CMD_READ_NORMAL) | SB2_SFC_OPCODE_dual_quad_mode(0)); //switch flash to read mode
			rtd_outl(SB2_SFC_CTL_reg, SB2_SFC_CTL_dataen(1) | SB2_SFC_CTL_adren(1) | SB2_SFC_CTL_dmycnt(0)); //data_en=1 addr_en=1 dmy_en=0
			break;
		case eMODE_READ_NORMAL_FAST:
			rtd_outl(SB2_SFC_OPCODE_reg, SB2_SFC_OPCODE_opc(CMD_READ_NORMAL_FAST) | SB2_SFC_OPCODE_dual_quad_mode(0)); //switch flash to read mode
			rtd_outl(SB2_SFC_CTL_reg, SB2_SFC_CTL_dataen(1) | SB2_SFC_CTL_adren(1) | SB2_SFC_CTL_dmycnt(1));  //data_en=1 addr_en=1 dmy_en=1
			break;
		case eMODE_READ_DUAL_FAST:
			rtd_outl(SB2_SFC_OPCODE_reg, SB2_SFC_OPCODE_opc(CMD_READ_DUAL_FAST) | SB2_SFC_OPCODE_dual_quad_mode(1));  //switch flash to read mode
			rtd_outl(SB2_SFC_CTL_reg, SB2_SFC_CTL_dataen(1) | SB2_SFC_CTL_adren(1) | SB2_SFC_CTL_dmycnt(1)); //data_en=1 addr_en=1 dmy_en=1
			break;
		case eMODE_READ_QUAD_FAST:
			rtd_outl(SB2_SFC_OPCODE_reg, SB2_SFC_OPCODE_opc(CMD_READ_QUAD_FAST) | SB2_SFC_OPCODE_dual_quad_mode(0x5));  //switch flash to read mode
			rtd_outl(SB2_SFC_CTL_reg, SB2_SFC_CTL_dataen(1) | SB2_SFC_CTL_adren(1) | SB2_SFC_CTL_dmycnt(1)); //data_en=1 addr_en=1 dmy_en=1
			break;
		default:
			rtd_outl(SB2_SFC_OPCODE_reg, SB2_SFC_OPCODE_opc(CMD_READ_NORMAL) | SB2_SFC_OPCODE_dual_quad_mode(0)); //switch flash to read mode
			rtd_outl(SB2_SFC_CTL_reg, SB2_SFC_CTL_dataen(1) | SB2_SFC_CTL_adren(1) | SB2_SFC_CTL_dmycnt(0)); //data_en=1 addr_en=1 dmy_en=0
			break;
	}

	SN_SYNC;
}
	
static void switch_write_mode(spinor_info_t *spinor_info)
{
	if(spinor_info->write_mode == eMODE_WRITE_NORMAL) {
		//issue write command
		rtd_outl(SB2_SFC_OPCODE_reg, SB2_SFC_OPCODE_opc(CMD_PAGE_PROGRAM) | SB2_SFC_OPCODE_dual_quad_mode(0)); //switch flash to normal mode
		rtd_outl(SB2_SFC_CTL_reg, SB2_SFC_CTL_dataen(1) | SB2_SFC_CTL_adren(1) | SB2_SFC_CTL_dmycnt(0)); //data_en=1 addr_en=1 dmy_en=0
		SN_SYNC;
	} else if(spinor_info->write_mode == eMODE_WRITE_QUAD_FAST) {
		rtd_pr_spi_nor_err("the spi dirver not surport write quad mode\n");
	}
}

static inline size_t sfc_memcpy(unsigned long dst, unsigned long src, size_t len)
{
	size_t i;
	for (i = 0; i < len; i++) {
		rtd_outb((dst + i), rtd_inb(src + i));
	}

	return len;
}

static void sfc_read_cpu(struct spinor_chip_t* spinor_chip, loff_t from, size_t len, char *buf)
{
	spinor_info_t* spinor_info = (spinor_info_t*)(spinor_chip->spinor_info);

	sfc_get_device(spinor_chip, FL_WORKING);
	if(spinor_info->address_4bytes_enable)
		sfc_enable_4bytes_addr_mode();
	switch_read_mode(spinor_info);
	sfc_memcpy((unsigned long)buf, (SPI_BASE_ADDR + (unsigned long)from), len);
	if(spinor_info->address_4bytes_enable)
		sfc_disable_4bytes_addr_mode();
	sfc_release_device(spinor_chip);

	return;
}

static __maybe_unused  void sfc_read_md(struct spinor_chip_t *spinor_chip, loff_t from, size_t len, unsigned char *buf)
{
	spinor_info_t* spinor_info = (spinor_info_t*)(spinor_chip->spinor_info);

	sfc_get_device(spinor_chip, FL_WORKING);
	if(spinor_info->address_4bytes_enable)
		sfc_enable_4bytes_addr_mode();
	switch_read_mode(spinor_info);
	smd_checkComplete();

	// set DDR and flash addr
	rtd_outl(MD_MD_FDMA_DDR_SADDR_reg, (unsigned int)virt_to_phys(buf));
	rtd_outl(MD_MD_FDMA_FL_SADDR_reg, MD_MD_FDMA_FL_SADDR_fdma_fl_saddr((unsigned int)from));
	rtd_outl(MD_MD_FDMA_CTRL2_reg, MD_MD_FDMA_CTRL2_fdma_length(len) | MD_MD_FDMA_CTRL2_fdma_dir(FLASH_2_DDR) | \
		MD_MD_FDMA_CTRL2_fdma_max_xfer(SET_DMA_LEN_512) | MD_MD_FDMA_CTRL2_fdma_swap(0) | \
		MD_MD_FDMA_CTRL2_fl_mapping_mode(1)); // forward map, no_swap, DMA LEN, FLASH to DDR

	dmac_flush_range(buf, buf + len);
	outer_flush_range(virt_to_phys(buf), virt_to_phys(buf + len));

	SN_SYNC;
	rtd_outl(MD_MD_FDMA_CTRL1_reg, MD_MD_FDMA_CTRL1_write_en1(1) | MD_MD_FDMA_CTRL1_fdma_go(1));

	while(rtd_inl(MD_MD_FDMA_CTRL1_reg) & MD_MD_FDMA_CTRL1_fdma_go_mask); // wait for MD done its operation
	
	rtd_outl(MD_MD_SMQ_INT_STATUS_reg, MD_MD_SMQ_INT_STATUS_fdma_done(1));  // clear flash done bit

	if(spinor_info->address_4bytes_enable)
		sfc_disable_4bytes_addr_mode();
	sfc_release_device(spinor_chip);

	return;
}

static int sfc_write_cpu(struct spinor_chip_t *spinor_chip, loff_t to, size_t len, unsigned char *buf)
{
	int ret = 0;
	spinor_info_t* spinor_info = (spinor_info_t*)(spinor_chip->spinor_info);

	sfc_get_device(spinor_chip, FL_WORKING);
	if(spinor_info->address_4bytes_enable)
		sfc_enable_4bytes_addr_mode();
	switch_write_mode(spinor_info);
	sfc_memcpy((SPI_BASE_ADDR + (unsigned long)to), (unsigned long)buf, len);
	SN_SYNC;
	ret = sfc_wait_write_done();
	if(spinor_info->address_4bytes_enable)
		sfc_disable_4bytes_addr_mode();
	sfc_release_device(spinor_chip);
	return ret;
}

/****************************************************************************
 * using MD to write data from DDR to FLASH which size is within a single page
 * pass-in address must be aligned on 4-bytes, and
 * length must be multiples of 4-bytes
 *****************************************************************************/
static __maybe_unused int sfc_write_md(struct spinor_chip_t *spinor_chip, loff_t to, size_t len, unsigned char *buf)
{
	spinor_info_t* spinor_info = (spinor_info_t*)(spinor_chip->spinor_info);
	int ret = 0;

	sfc_get_device(spinor_chip, FL_WORKING);
	if(spinor_info->address_4bytes_enable)
		sfc_enable_4bytes_addr_mode();
	switch_write_mode(spinor_info);

	// set DDR and flash addr
	rtd_outl(MD_MD_FDMA_DDR_SADDR_reg, (unsigned int)virt_to_phys(buf));
	rtd_outl(MD_MD_FDMA_FL_SADDR_reg, MD_MD_FDMA_FL_SADDR_fdma_fl_saddr((unsigned int)to));
	rtd_outl(MD_MD_FDMA_CTRL2_reg, MD_MD_FDMA_CTRL2_fdma_length(len) | MD_MD_FDMA_CTRL2_fdma_dir(DDR_2_FLASH) | \
		MD_MD_FDMA_CTRL2_fdma_max_xfer(SET_DMA_LEN_512) | MD_MD_FDMA_CTRL2_fdma_swap(0) | \
		MD_MD_FDMA_CTRL2_fl_mapping_mode(1)); // forward map, no_swap, DMA LEN, FLASH to DDR

	dmac_flush_range(buf, buf + len);
	outer_flush_range(virt_to_phys(buf), virt_to_phys(buf + len));

	SN_SYNC;
	rtd_outl(MD_MD_FDMA_CTRL1_reg, MD_MD_FDMA_CTRL1_write_en1(1) | MD_MD_FDMA_CTRL1_fdma_go(1));

	while(rtd_inl(MD_MD_FDMA_CTRL1_reg) & MD_MD_FDMA_CTRL1_fdma_go_mask); // wait for MD done its operation
	
	rtd_outl(MD_MD_SMQ_INT_STATUS_reg, MD_MD_SMQ_INT_STATUS_fdma_done(1));  // clear flash done bit

	SN_SYNC;
	//mdelay(3);
	ret = sfc_wait_write_done();
	if(spinor_info->address_4bytes_enable)
		sfc_disable_4bytes_addr_mode();
	sfc_release_device(spinor_chip);
	return ret;
}

static __maybe_unused int sfc_verify_after_write(struct spinor_chip_t *spinor_chip, unsigned char * to_verify, size_t len_verify, unsigned char *buf_verify)
{
	spinor_info_t* spinor_info = (spinor_info_t*)(spinor_chip->spinor_info);
	size_t i = 0;
	int ret = 0;
	to_verify += SPI_BASE_ADDR;

	sfc_get_device(spinor_chip, FL_WORKING);
	if(spinor_info->address_4bytes_enable)
		sfc_enable_4bytes_addr_mode();
	switch_read_mode(spinor_info);
	// mdelay(10);

	do {
		if (*to_verify != *buf_verify) {
			rtd_pr_spi_nor_info("0x%08lx -> 0x%02x,  0x%08lx -> 0x%02x\n", (unsigned long)to_verify, *to_verify, (unsigned long)buf_verify, *buf_verify);
			ret = -1;
			goto final;
		}
		to_verify++;
		buf_verify++;
		SN_SYNC;
	} while (++i < len_verify);

final:
	if(spinor_info->address_4bytes_enable)
		sfc_disable_4bytes_addr_mode();
	sfc_release_device(spinor_chip);
	return ret;
}

static int sfc_erase_flash_operation(struct spinor_chip_t *spinor_chip, loff_t from, size_t len)
{
	spinor_info_t* spinor_info = (spinor_info_t*)(spinor_chip->spinor_info);
	loff_t erase_addr = SPI_BASE_ADDR + from;
	size_t erase_size = len;
	unsigned int cmd_erase = 0;
	unsigned char tmp;
	size_t size = 0;
	int ret = 0;

	if(spinor_info->min_erase_size == 0) {
		rtd_pr_spi_nor_err("the spi nor min_erase_size is not init\n");
		return -1;
	}

	if(len % spinor_info->min_erase_size != 0) {
		rtd_pr_spi_nor_err("the erase size is not align\n");
		return -1;
	}

	if((erase_addr & (spinor_info->min_erase_size - 1)) != 0) {
		rtd_pr_spi_nor_err("the erase addr is not align\n");
		return -1;
	}

	sfc_get_device(spinor_chip, FL_WORKING);
	if(spinor_info->address_4bytes_enable)
		sfc_enable_4bytes_addr_mode();
	while(erase_size > 0) {

		if ((erase_size >= ERASE_SZIE_64K) && (((unsigned int)erase_addr & (ERASE_SZIE_64K - 1)) == 0) && (spinor_info->erase_capacity & ERASE_64K)) {
			//rtd_pr_spi_nor_debug("(64k)");
			cmd_erase = CMD_ERASE_BLOCK_64K;
			size = ERASE_SZIE_64K;
		} else if ((erase_size >= ERASE_SZIE_32K) && (((unsigned int)erase_addr & (ERASE_SZIE_32K - 1)) == 0) && (spinor_info->erase_capacity & ERASE_32K)) {
			//rtd_pr_spi_nor_debug("(32k)");
			cmd_erase = CMD_ERASE_BLOCK_32K;
			size = ERASE_SZIE_32K;
		} else if ((erase_size >= ERASE_SZIE_4K) && (((unsigned int)erase_addr & (ERASE_SZIE_4K - 1)) == 0) && (spinor_info->erase_capacity & ERASE_4K)) {
			//rtd_pr_spi_nor_debug("(4k)");
			cmd_erase = CMD_ERASE_SECTOR_4K;
			size = ERASE_SZIE_4K;
		}

		if(cmd_erase) {
			rtd_outl(SB2_SFC_OPCODE_reg, SB2_SFC_OPCODE_opc(cmd_erase));
			rtd_outl(SB2_SFC_CTL_reg, SB2_SFC_CTL_dataen(0) | SB2_SFC_CTL_adren(1) | SB2_SFC_CTL_dmycnt(0));
			
			SN_SYNC;
			tmp = rtd_inb(erase_addr);
			udelay(SFC_DELAY);
			cmd_erase = 0;
		} else {
			rtd_pr_spi_nor_err("the erase size is not aligned,please use rtk_spi_nor_get_align()\n");
			ret = -1;
			goto end;
		}

		ret = sfc_wait_erase_done();
		if(ret < 0) {
			ret = -1;
			goto end;
		}

		size = (erase_size > size) ? size : erase_size ;
		erase_size -= size;
		erase_addr += size;
	}

end:
	if(spinor_info->address_4bytes_enable)
		sfc_disable_4bytes_addr_mode();
	sfc_release_device(spinor_chip);
	return ret;
}

#if 0
static int sfc_erase_flash(struct spinor_chip_t *spinor_chip, loff_t from, size_t len)
{
	spinor_info_t *spinor_info = (spinor_info_t*)(spinor_chip->spinor_info);

	if(spinor_info->multi_die_support){
		if(from >= spinor_info->per_die_size){
			sfc_die_select(0x1);

			from -= spinor_info->per_die_size;
		}else{
			sfc_die_select(0x0);

			if((from + len) > spinor_info->per_die_size){
				size_t erase_len = spinor_info->per_die_size - from;

				int ret = sfc_erase_flash_operation(spinor_chip, from, erase_len);
				if(ret < 0){
					rtd_pr_spi_nor_err("%s_%d: erase fail\n",__func__, __LINE__);
					return ret;
				}

				from = 0x0;
				len -= erase_len; 
				sfc_die_select(0x1);
			}
		}
	}

	return sfc_erase_flash_operation(spinor_chip, from, len);
}
#endif
static int sfc_get_status_register(spinor_info_t *spinor_info, STATUS_REGISTER_T *status_register)
{
	unsigned char status1 = 0xFF, status2 = 0xFF, status3 = 0xFF;
	// unsigned short status_16bit = 0xffff;

	switch(spinor_info->id) {
		case GD25Q128C_128Mbit:
		case WINBOND_W25Q128FV_128Mbit:
		case WINBOND_W25M512JV_512Mbit:
			sfc_get_feature(CMD_READ_STATUS1, &status1);
			sfc_get_feature(CMD_READ_STATUS2, &status2);
			sfc_get_feature(CMD_READ_STATUS3, &status3);
			rtd_pr_spi_nor_info("old wp status: 0x%x 0x%x 0x%x\n", status1, status2, status3);
			break;

		case GD25Q64C_64Mbit:
			sfc_get_feature(CMD_READ_STATUS1, &status1);
			sfc_get_feature(CMD_READ_STATUS2, &status2);
			sfc_get_feature(CMD_READ_STATUS3, &status3);
			rtd_pr_spi_nor_info("old wp status: 0x%x 0x%x 0x%x\n", status1, status2, status3);
			break;

		case WINBOND_W32Q64_64Mbit:
			sfc_get_feature(CMD_READ_STATUS1, &status1);
			sfc_get_feature(CMD_READ_STATUS2, &status2);
			rtd_pr_spi_nor_info("old wp status: 0x%x 0x%x\n", status1, status2);
			break;

		default:
			break;
	}

	status_register->status_register1 = status1;
	status_register->status_register2 = status2;
	status_register->status_register3 = status3;
	return 0;
}

static int sfc_lock(spinor_info_t *spinor_info)
{
	unsigned char status1 = 0xFF, status2 = 0xFF, status3 = 0xFF;
	unsigned short status_16bit = 0xffff;

	switch(spinor_info->id) {
		case GD25Q128C_128Mbit:
		case WINBOND_W25Q128FV_128Mbit:
		case WINBOND_W25M512JV_512Mbit:
			sfc_get_feature(CMD_READ_STATUS1, &status1);
			sfc_get_feature(CMD_READ_STATUS2, &status2);
			sfc_get_feature(CMD_READ_STATUS3, &status3);
			rtd_pr_spi_nor_info("%s: old wp status: 0x%x 0x%x 0x%x\n", __FUNCTION__, status1, status2, status3);

			if(((status1 & SFC_WP_MASK_BIT2_TO_BIT7) != 0x0c) \
				|| ((status2 & SFC_WP_MASK_BIT6) != SFC_WP_MASK_BIT6)) {
				status1 = (status1 & (~SFC_WP_MASK_BIT2_TO_BIT7)) | 0x0c;
				status2 = status2 | SFC_WP_MASK_BIT6;
				if((-1 == sfc_set_feature(CMD_WRITE_STATUS1, status1)) \
					|| (-1 == sfc_set_feature(CMD_WRITE_STATUS2, status2))) {
					rtd_pr_spi_nor_err("sfc_lock fail\n");
					return -1;
				}
				rtd_pr_spi_nor_info("%s: new wp status: 0x%x 0x%x\n", __FUNCTION__, status1, status2);
			}
			break;

		case GD25Q64C_64Mbit:
			sfc_get_feature(CMD_READ_STATUS1, &status1);
			sfc_get_feature(CMD_READ_STATUS2, &status2);
			sfc_get_feature(CMD_READ_STATUS3, &status3);
			rtd_pr_spi_nor_info("%s: old wp status: 0x%x 0x%x 0x%x\n", __FUNCTION__, status1, status2, status3);

			if(((status1 & SFC_WP_MASK_BIT2_TO_BIT7) != 0x10) \
				|| ((status2 & SFC_WP_MASK_BIT6) != SFC_WP_MASK_BIT6)) {
				status1 = (status1 & (~SFC_WP_MASK_BIT2_TO_BIT7)) | 0x10;
				status2 = status2 | SFC_WP_MASK_BIT6;
				if((-1 == sfc_set_feature(CMD_WRITE_STATUS1, status1)) \
					|| (-1 == sfc_set_feature(CMD_WRITE_STATUS2, status2))) {
					rtd_pr_spi_nor_err("sfc_lock fail\n");
					return -1;
				}
				rtd_pr_spi_nor_info("%s: new wp status: 0x%x 0x%x\n", __FUNCTION__, status1, status2);
			}
			break;

		case WINBOND_W32Q64_64Mbit:
			sfc_get_feature(CMD_READ_STATUS1, &status1);
			sfc_get_feature(CMD_READ_STATUS2, &status2);
			rtd_pr_spi_nor_info("%s: old wp status: 0x%x 0x%x\n", __FUNCTION__, status1, status2);
			if(((status1 & SFC_WP_MASK_BIT2_TO_BIT7) != 0x10) \
				|| ((status2 & SFC_WP_MASK_BIT6) != SFC_WP_MASK_BIT6)) {
				status1 = (status1 & (~SFC_WP_MASK_BIT2_TO_BIT7)) | 0x10;
				status2 = status2 | SFC_WP_MASK_BIT6;
				status_16bit = ((status2 << 8) | status1);
				if(-1 == sfc_set_feature_16bit(CMD_WRITE_STATUS1, status_16bit)) {
					rtd_pr_spi_nor_err("sfc_lock fail\n");
					return -1;
				}
				rtd_pr_spi_nor_info("%s: new wp status: 0x%x 0x%x\n", __FUNCTION__, status1, status2);
			}
			break;

		default:
			rtd_pr_spi_nor_info("lock not support\n");
			break;
	}

	return 0;
}

static int sfc_unlock(struct spinor_chip_t *spinor_chip)
{
	spinor_info_t* spinor_info = (spinor_info_t*)(spinor_chip->spinor_info);

	unsigned char status1 = 0xFF, status2 = 0xFF, status3 = 0xFF;
	unsigned short status_16bit = 0xffff;

	switch(spinor_info->id) {
		case GD25Q128C_128Mbit:
		case GD25Q40C_4Mbit:
		case WINBOND_W25M512JV_512Mbit:
			sfc_get_feature(CMD_READ_STATUS1, &status1);
			sfc_get_feature(CMD_READ_STATUS2, &status2);
			sfc_get_feature(CMD_READ_STATUS3, &status3);
			rtd_pr_spi_nor_info("%s: old wp status: 0x%x 0x%x 0x%x\n", __FUNCTION__, status1, status2, status3);
			if(((status1 & SFC_WP_MASK_BIT2_TO_BIT7)) \
				|| (status2 & SFC_WP_MASK_BIT6)) {
				status1 = (status1 & (~SFC_WP_MASK_BIT2_TO_BIT7));
				status2 = status2 & (~SFC_WP_MASK_BIT6);
				sfc_set_feature(CMD_WRITE_STATUS1, status1);
				sfc_set_feature(CMD_WRITE_STATUS2, status2);
				rtd_pr_spi_nor_info("%s: new wp status: 0x%x 0x%x\n", __FUNCTION__, status1, status2);
			}
			break;

		case GD25Q64C_64Mbit:
			sfc_get_feature(CMD_READ_STATUS1, &status1);
			sfc_get_feature(CMD_READ_STATUS2, &status2);
			sfc_get_feature(CMD_READ_STATUS3, &status3);
			rtd_pr_spi_nor_info("%s: old wp status: 0x%x 0x%x 0x%x\n", __FUNCTION__, status1, status2, status3);
			if(((status1 & SFC_WP_MASK_BIT2_TO_BIT7)) \
				|| (status2 & SFC_WP_MASK_BIT6)) {
				status1 = (status1 & (~SFC_WP_MASK_BIT2_TO_BIT7));
				status2 = status2 & (~SFC_WP_MASK_BIT6);
				sfc_set_feature(CMD_WRITE_STATUS1, status1);
				sfc_set_feature(CMD_WRITE_STATUS2, status2);
				rtd_pr_spi_nor_info("%s: new wp status: 0x%x 0x%x\n", __FUNCTION__, status1, status2);
			}
			break;

		case WINBOND_W32Q64_64Mbit:
			sfc_get_feature(CMD_READ_STATUS1, &status1);
			sfc_get_feature(CMD_READ_STATUS2, &status2);
			rtd_pr_spi_nor_info("%s: old wp status: 0x%x 0x%x\n", __FUNCTION__, status1, status2);
			if(((status1 & SFC_WP_MASK_BIT2_TO_BIT7)) \
				|| (status2 & SFC_WP_MASK_BIT6)) {
				status1 = (status1 & (~SFC_WP_MASK_BIT2_TO_BIT7));
				status2 = status2 & (~SFC_WP_MASK_BIT6);
				status_16bit = ((status2 << 8) | status1);
				sfc_set_feature_16bit(CMD_WRITE_STATUS1, status_16bit);
				rtd_pr_spi_nor_info("%s: new wp status: 0x%x 0x%x\n", __FUNCTION__, status1, status2);
			}
			break;

		case WINBOND_W25Q128FV_128Mbit:
			sfc_get_feature(CMD_READ_STATUS1, &status1);
			sfc_get_feature(CMD_READ_STATUS2, &status2);
			sfc_get_feature(CMD_READ_STATUS3, &status3);
			rtd_pr_spi_nor_info("%s: old wp status: 0x%x 0x%x 0x%x\n", __FUNCTION__, status1, status2, status3);
			if(((status1 & SFC_WP_MASK_BIT2_TO_BIT7)) \
				|| (status2 & SFC_WP_MASK_BIT6)) {
				status1 = (status1 & (~SFC_WP_MASK_BIT2_TO_BIT7));
				status2 = status2 & (~SFC_WP_MASK_BIT6);
				sfc_set_feature(CMD_WRITE_STATUS1, status1);
				sfc_set_feature(CMD_WRITE_STATUS2, status2);
				rtd_pr_spi_nor_info("%s: new wp status: 0x%x 0x%x 0x%x\n", __FUNCTION__, status1, status2, status3);
			}
			break;

		case WINBOND_W25X40CL_4Mbit:
			sfc_get_feature(CMD_READ_STATUS1, &status1);
			rtd_pr_spi_nor_info("%s: old wp status: 0x%x\n", __FUNCTION__, status1);
			if((status1 & SFC_WP_MASK_BIT2_TO_BIT7)){
				status1 = (status1 & (~SFC_WP_MASK_BIT2_TO_BIT7));
				sfc_set_feature(CMD_WRITE_STATUS1, status1);
				rtd_pr_spi_nor_info("%s: new wp status: 0x%x\n", __FUNCTION__, status1);
			}
			break;

		default:
			break;
	}

	return 0;
}

static void sfc_read_id(unsigned int* id)
{
	rtd_outl(SB2_SFC_OPCODE_reg, SB2_SFC_OPCODE_opc(CMD_READ_ID));
	rtd_outl(SB2_SFC_CTL_reg, SB2_SFC_CTL_dataen(1) | SB2_SFC_CTL_adren(0) | SB2_SFC_CTL_dmycnt(0)); /* dataen = 1, adren = 0, dmycnt = 0 */
	SN_SYNC;
	udelay(SFC_DELAY);
	
	*id = rtd_inl(SPI_BASE_ADDR);
	SN_SYNC;
	*id = swap_endian(*id);
	(*id) = (*id) >> 8;
	
	return;
}

static int sfc_id_verify(unsigned int *id)
{
	unsigned int tmp_id = 0;
	unsigned int i  =0;
	unsigned int cmp_cnt = 0xA;

	sfc_read_id(id);

	for(i = 0; i < cmp_cnt; i++){
		sfc_read_id(&tmp_id);
		if(tmp_id != *id){
			rtd_pr_spi_nor_info("[SPINOR-INFO] ID verify fail, cmp_id = 0x%x id = 0x%x \r\n", tmp_id, *id);
			return -1;
		}
	}

	return 0;
}


#ifdef MARK_NOT_USE
static int sfc_reset(void)
{
	unsigned char tmp;
	int  retry = 0;

	rtd_outl(SB2_SFC_OPCODE_reg, SB2_SFC_OPCODE_opc(CMD_ENABLE_RESET));
	rtd_outl(SB2_SFC_CTL_reg, SB2_SFC_CTL_dataen(0) | SB2_SFC_CTL_adren(0) | SB2_SFC_CTL_dmycnt(0)); 

	SN_SYNC;
	udelay(SFC_DELAY);
	tmp = rtd_inb(SPI_BASE_ADDR);
	udelay(SFC_DELAY);

	do {
		udelay(SFC_DELAY);
		sfc_get_feature(CMD_READ_STATUS1, &tmp);
	} while ((tmp & STATUS_WIP) && (++retry < SET_FEATURE_TIMEOUT));

	if (retry == SET_FEATURE_TIMEOUT) {
		rtd_pr_spi_nor_err("%s reset spi fail 0x%x\n", __FUNCTION__, tmp);
		return -1;
	}

	return 0;
}
#endif

static int sfc_set_clk(struct spinor_chip_t *spinor_chip, unsigned int clk)
{
#if 1
	int ret = -1;
	unsigned int id = 0;

	unsigned int clk_div = 0;

	unsigned int clk_div_save = 0;
	unsigned int delay_sel_save = 0;

	unsigned int support_max_div = 0;
	unsigned int support_min_div = 0;
	unsigned int support_clk_cnt = 0;

	unsigned int delay_sel = 0, m = 0, n = 0;
	unsigned int delay_sel_flag = 0;
	unsigned int valid_loop = 0, valid_num = 0, delay_sel_std = 0, save_id[DELAY_SEL_STEP_TOTAL_CNT] = {0};

	unsigned int delay_sel_min = 0;
	unsigned int delay_sel_max = 0;
	unsigned int invalid_delay_sel_flag = 0;

	if((clk ==0) || (clk >100))
	{
		rtd_pr_spi_nor_err("set spi clock 0x%x fail\n", clk);
		return -1;
	}
	sfc_get_device(spinor_chip, FL_WORKING);

	clk_div  = 500 / clk - 1;

	clk_div_save = SB2_SFC_SCK_get_fdiv(rtd_inl(SB2_SFC_SCK_reg));
	delay_sel_save = SB2_SFC_POS_LATCH_get_delay_sel(rtd_inl(SB2_SFC_POS_LATCH_reg));

	support_clk_cnt = sizeof(clock_match)/sizeof(struct clock_match_t);
	//support_min_div = clock_match[0].clock_div;
	support_min_div = spinor_chip->min_clock_div;
	support_max_div = clock_match[support_clk_cnt - 1].clock_div;

	if(clk_div <= support_max_div && clk_div >= support_min_div) {
		// set clk_div
		rtd_maskl(SB2_SFC_SCK_reg, ~SB2_SFC_SCK_fdiv_mask, SB2_SFC_SCK_fdiv(clk_div));

		delay_sel_flag = 0;
		// scan the delay_sel
		for(delay_sel=0; delay_sel< DELAY_SEL_STEP_TOTAL_CNT; delay_sel++)
		{
			// set delay sel
			rtd_maskl(SB2_SFC_POS_LATCH_reg, ~SB2_SFC_POS_LATCH_delay_sel_mask, SB2_SFC_POS_LATCH_delay_sel(delay_sel));
			ret = sfc_id_verify(&id);
			if(ret < 0 ){
				save_id[delay_sel] = 0;
			}else{
				save_id[delay_sel] = id;
			}
			rtd_pr_spi_nor_info("[SPINOR-INFO] ID: 0x%x, clk_div:0x%x\n", save_id[delay_sel], clk_div); 
		}

		delay_sel_std = save_id[0] ;
		for(n = 0; n < DELAY_SEL_STEP_TOTAL_CNT; n++){
			if((save_id[n] == delay_sel_std) && (delay_sel_std != 0x0) && (delay_sel_std != 0xffffff)){
				valid_num++;
			}else{
				if((save_id[n] != 0x0) && (save_id[n] != 0xffffff)){
					valid_num = 1;
				}else{
					valid_num = 0;
				}
				delay_sel_std = save_id[n];		
			}
			if(valid_num >= DELAY_SEL_STEP_GATE_CNT){
				for(m = 0; m < valid_num; m++){
					delay_sel_flag |= (1<<(n -m));
				}
				delay_sel_std = save_id[n];
				valid_loop = n;
				break;
			}
		}

		for(n = valid_loop+1; n < DELAY_SEL_STEP_TOTAL_CNT; n++){
			if((save_id[n] == delay_sel_std) && (delay_sel_std != 0x0) && (delay_sel_std != 0xffffff)){
				delay_sel_flag |= (1<<n);
			}else{
				break;
			}
		}
		rtd_pr_spi_nor_info("[SPINOR-INFO] ID: 0x%x, clk_div:0x%x, delay_sel:0x%x !\n", \
			id, clk_div, delay_sel_flag); 

		// find the available delay_sel
		delay_sel_min = 0;
		delay_sel_max = 0;
		invalid_delay_sel_flag = 0;

		// find the min delay_sel
		for(delay_sel=0; delay_sel < DELAY_SEL_STEP_TOTAL_CNT; delay_sel++){
			if(delay_sel_flag & (1<<delay_sel)){
				delay_sel_min = delay_sel;
				break;
			}
		}

		// find the max delay sel
		for(delay_sel=(DELAY_SEL_STEP_TOTAL_CNT - 1); delay_sel < DELAY_SEL_STEP_TOTAL_CNT; delay_sel--){
			if(delay_sel_flag & (1<<delay_sel)){
				delay_sel_max = delay_sel;
				break;
			}
		}

		// sanity check
		for(delay_sel = delay_sel_min; delay_sel <= delay_sel_max; delay_sel++ ){
			if((delay_sel_flag & (1<<delay_sel)) == 0){
				rtd_pr_spi_nor_err("min delay_sel = %d, max delay_sel = %d , invalid delay_sel = %d\r\n", 
					delay_sel_min, delay_sel_max, delay_sel);
				invalid_delay_sel_flag = 1;
				break;
			}
		}

		if(invalid_delay_sel_flag == 0){
			if((delay_sel_max - delay_sel_min + 1) >= DELAY_SEL_STEP_GATE_CNT)
			{
				spinor_chip->delay_sel = (delay_sel_max + delay_sel_min)/2;
				spinor_chip->clock_div = clk_div;
			}else{
				invalid_delay_sel_flag = 1;
			}
		}

		if(invalid_delay_sel_flag == 0)
		{
			rtd_pr_spi_nor_info("[SPINOR-INFO]find available delay_sel(0x%x) for clock(0x%x), the delay_sel_flag is (0x%x)\n", spinor_chip->delay_sel, spinor_chip->clock_div, delay_sel_flag);
			rtd_maskl(SB2_SFC_SCK_reg, ~SB2_SFC_SCK_fdiv_mask, SB2_SFC_SCK_fdiv(spinor_chip->clock_div));
			rtd_maskl(SB2_SFC_POS_LATCH_reg, ~SB2_SFC_POS_LATCH_delay_sel_mask, SB2_SFC_POS_LATCH_delay_sel(spinor_chip->delay_sel));
			
			ret = 0;
		}
		else
		{
			rtd_pr_spi_nor_info("[SPINOR-INFO]cannot find available delay_sel_flag(0x%x) for clock(0x%x)\n", delay_sel_flag, clk_div);
			
			rtd_maskl(SB2_SFC_SCK_reg, ~SB2_SFC_SCK_fdiv_mask, SB2_SFC_SCK_fdiv(clk_div_save));
			rtd_maskl(SB2_SFC_POS_LATCH_reg, ~SB2_SFC_POS_LATCH_delay_sel_mask, SB2_SFC_POS_LATCH_delay_sel(delay_sel_save));
			ret = 0;
		}
	}
	else 
	{
		rtd_pr_spi_nor_info("[SPINOR-INFO] paramter clk(0x%x) is error\n", clk);
		ret = 0;
	}

	sfc_release_device(spinor_chip);
	return ret;
#else
	return 0;
#endif
}

static unsigned long sfc_get_clk(void)
{
	unsigned long value = 0;

	value = rtd_inl(SB2_SFC_SCK_reg);
	value = SB2_SFC_SCK_get_fdiv(value);

	return (500 / (value + 1));
}

static RHAL_CS_TYPE_E sfc_get_cs_type(void)
{
	unsigned long value = 0;

	value = rtd_inl(SB2_SFC_WP_reg) ;

	if(SB2_SFC_WP_get_ce_n(value))
		return RHAL_CS1_FCIC;
	else
		return RHAL_CS0_SPIFLASH ;
}

static int sfc_init_early(struct spinor_chip_t *spinor_chip, RHAL_SPI_INIT_PARAM_T  param)
{
	int ret = 0;

	if(param.cs_type == RHAL_CS1_FCIC)
		rtd_outl(SB2_SFC_WP_reg, SB2_SFC_WP_ce_n(1) | SB2_SFC_WP_write_en2(1));
	else
		rtd_outl(SB2_SFC_WP_reg, SB2_SFC_WP_ce_n(0) | SB2_SFC_WP_write_en2(1));

	/*HW SD provide the value :   Phcnt=0x60 / Plcnt=0xF(default) / tdt=0x39 */
	rtd_maskl(SB2_SFC_CE_reg, ~SB2_SFC_CE_phcnt_mask, SB2_SFC_CE_phcnt(0x60)); 
	rtd_maskl(SB2_SFC_CE_reg, ~SB2_SFC_CE_plcnt_mask, SB2_SFC_CE_plcnt(0xF));
	rtd_maskl(SB2_SFC_CE_reg, ~SB2_SFC_CE_tdt_mask, SB2_SFC_CE_tdt(0x39));

	// auto
	rtd_outl(SB2_SFC_EN_WR_reg, SB2_SFC_EN_WR_wt_prog_en(1) | SB2_SFC_EN_WR_wr_en_op(CMD_WRITE_ENABLE));
	rtd_outl(SB2_SFC_WAIT_WR_reg, SB2_SFC_WAIT_WR_wt_prog_done(1) | SB2_SFC_WAIT_WR_rdsr_op(CMD_READ_STATUS1));

	return ret;
}

static __maybe_unused int sfc_match_clock(struct spinor_chip_t *spinor_chip)
{
	__maybe_unused int ret = -1;
	__maybe_unused unsigned int id = 0;

	__maybe_unused unsigned int clock_index = 0;
	__maybe_unused unsigned int delay_sel = 0, m = 0, n = 0;
	__maybe_unused unsigned int i = 0, valid_loop = 0, valid_num = 0, delay_sel_std = 0, save_id[DELAY_SEL_STEP_TOTAL_CNT] = {0};

	__maybe_unused unsigned int delay_sel_min = 0;
	__maybe_unused unsigned int delay_sel_max = 0;
	__maybe_unused unsigned int invalid_delay_sel_flag = 0;
	__maybe_unused unsigned int valid_delay_sel_cnt_gate = DELAY_SEL_STEP_GATE_CNT;

retry_sacn:
	// scan the clock
	for(clock_index = 0; clock_index < sizeof(clock_match)/sizeof(struct clock_match_t); clock_index++)
	{
		// set current scan clock
		rtd_outl(SB2_SFC_SCK_reg, SB2_SFC_SCK_fdiv(clock_match[clock_index].clock_div) | SB2_SFC_SCK_clk_duty50_en(1)); // SCK  500M/10 = 50M
		
		clock_match[clock_index].delay_sel = 0;
		// scan the delay_sel
		for(delay_sel = 0; delay_sel < DELAY_SEL_STEP_TOTAL_CNT; delay_sel++)
		{
			rtd_outl(SB2_SFC_POS_LATCH_reg, SB2_SFC_POS_LATCH_delay_sel(delay_sel) | SB2_SFC_POS_LATCH_pos_latch(POS_LATCH_RISE));
			ret = sfc_id_verify(&id);
			if(ret < 0 ){
				save_id[delay_sel] = 0;
			}else{
				save_id[delay_sel] = id;
			}
			rtd_pr_spi_nor_info("[SPINOR-INFO] ID: 0x%x, clk_div:0x%x\n", save_id[delay_sel], clock_match[clock_index].clock_div); 
		}

		delay_sel_std = save_id[0] ;
		for(n = 0; n < DELAY_SEL_STEP_TOTAL_CNT; n++){
			if((save_id[n] == delay_sel_std) && (delay_sel_std != 0x0) && (delay_sel_std != 0xffffff)){
				valid_num++;
			}else{
				if((save_id[n] != 0x0) && (save_id[n] != 0xffffff)){
					valid_num = 1;
				}else{
					valid_num = 0;
				}
				delay_sel_std = save_id[n];		
			}
			if(valid_num >= valid_delay_sel_cnt_gate){
				for(m = 0; m < valid_num; m++){
					clock_match[clock_index].delay_sel |= (1<<(n -m));
				}
				delay_sel_std = save_id[n];
				valid_loop = n;
				break;
			}
		}

		for(n = valid_loop+1; n < DELAY_SEL_STEP_TOTAL_CNT; n++){
			if((save_id[n] == delay_sel_std) && (delay_sel_std != 0x0) && (delay_sel_std != 0xffffff)){
				clock_match[clock_index].delay_sel |= (1<<n);
			}else{
				break;
			}
		}
		rtd_pr_spi_nor_info("[SPINOR-INFO] ID: 0x%x, clk_div:0x%x, delay_sel:0x%x !\n", \
			id, clock_match[clock_index].clock_div, clock_match[clock_index].delay_sel); 

		delay_sel_min = 0;
		delay_sel_max = 0;
		invalid_delay_sel_flag = 0;

		// find the min delay_sel
		for(delay_sel = 0; delay_sel < DELAY_SEL_STEP_TOTAL_CNT; delay_sel++){
			if(clock_match[clock_index].delay_sel & (1<<delay_sel)){
				delay_sel_min = delay_sel;
				break;
			}
		}

		// find the max delay sel
		for(delay_sel = (DELAY_SEL_STEP_TOTAL_CNT - 1); delay_sel < DELAY_SEL_STEP_TOTAL_CNT; delay_sel--){
			if(clock_match[clock_index].delay_sel & (1<<delay_sel)){
				delay_sel_max = delay_sel;
				break;
			}
		}

		// sanity check
		for(delay_sel = delay_sel_min; delay_sel <= delay_sel_max; delay_sel++ ){
			if((clock_match[clock_index].delay_sel & (1<<delay_sel)) == 0){
				rtd_pr_spi_nor_err("min delay_sel = %d, max delay_sel = %d , invalid delay_sel = %d\r\n", 
						delay_sel_min, delay_sel_max, delay_sel);
				invalid_delay_sel_flag = 1;
				break;
			}
		}


		if(invalid_delay_sel_flag == 0){
			if((delay_sel_max - delay_sel_min + 1) >= valid_delay_sel_cnt_gate)
			{
				spinor_chip->delay_sel = (delay_sel_max + delay_sel_min)/2;
				spinor_chip->clock_div = clock_match[clock_index].clock_div;
				break;
			}
		}

		rtd_pr_spi_nor_info("[SPINOR-INFO]cannot find available clk_div and delay_sel from clock_match(0x%x, 0x%x)\n", 
			clock_match[clock_index].clock_div, clock_match[clock_index].delay_sel);

		clock_match[clock_index].delay_sel = 0;
	}

	if(clock_index  == sizeof(clock_match)/sizeof(struct clock_match_t))
	{
		if(valid_delay_sel_cnt_gate > (DELAY_SEL_STEP_TOTAL_CNT/2)){
			valid_delay_sel_cnt_gate = valid_delay_sel_cnt_gate -2;
			goto retry_sacn;
		}

		rtd_pr_spi_nor_err("[SPINOR-INFO]Cannot find available clk_div and delay_sel\n");
		goto end;
	}
	else
	{
		rtd_pr_spi_nor_info("[SPINOR-INFO]find available clk_div(0x%x) and delay_sel(0x%x) from clock_match(0x%x, 0x%x)\n", 
		spinor_chip->clock_div, spinor_chip->delay_sel, clock_match[clock_index].clock_div, clock_match[clock_index].delay_sel);
	}

	// set clock and delay_sel
	rtd_outl(SB2_SFC_SCK_reg, SB2_SFC_SCK_fdiv(spinor_chip->clock_div) | SB2_SFC_SCK_clk_duty50_en(1));
	rtd_outl(SB2_SFC_POS_LATCH_reg, SB2_SFC_POS_LATCH_delay_sel(spinor_chip->delay_sel) | SB2_SFC_POS_LATCH_pos_latch(POS_LATCH_RISE));

	sfc_read_id(&id);
	for (i = 0; i < DEV_SIZE_S; i++) {
		/* find the match flash brand */
		if (id != s_device[i].id)
			continue;
		memcpy(spinor_chip->spinor_info, &s_device[i], sizeof(spinor_info_t));
		break;
	}

	if (i == DEV_SIZE_S) {
		memcpy(spinor_chip->spinor_info, &default_device, sizeof(spinor_info_t));
		rtd_pr_spi_nor_info("[SPINOR-INFO]ID not in the ID table, use default value\n");
	}

	rtd_pr_spi_nor_info("[SPINOR-INFO] ID: 0x%x, clk_div:0x%x, delay_sel:0x%x\n", id, spinor_chip->clock_div, spinor_chip->delay_sel);

	// record max support clock
	spinor_chip->min_clock_div = spinor_chip->clock_div;

	return 0;

end:
	return -1;
}

static int sfc_init_later(struct spinor_chip_t *spinor_chip)
{
	spinor_info_t* spinor_info = (spinor_info_t*)(spinor_chip->spinor_info);
	int ret = 0;

	switch(spinor_info->read_mode) {
		case eMODE_READ_NORMAL:
			rtd_pr_spi_nor_info("support eMODE_READ_NORMAL\n");
			break;
		case eMODE_READ_NORMAL_FAST:
			rtd_pr_spi_nor_info("support eMODE_READ_NORMAL_FAST\n");
			break;
		case eMODE_READ_DUAL_FAST:
			rtd_pr_spi_nor_info("support eMODE_READ_DUAL_FAST\n");
			break;
		case eMODE_READ_QUAD_FAST:
			rtd_pr_spi_nor_info("support eMODE_READ_QUAD_FAST\n");
			break;
		default:
			rtd_pr_spi_nor_info("please set spi read mode\n");
			break;
	}

	if(spinor_info->read_mode == eMODE_READ_QUAD_FAST || spinor_info->write_mode == eMODE_WRITE_QUAD_FAST) {
		if((spinor_info->cmd_quad_rdsr == EMPTY_VALUE) || (spinor_info->cmd_quad_wrsr == EMPTY_VALUE) || (spinor_info->cmd_quad_bit == EMPTY_VALUE)) {
			rtd_pr_spi_nor_info("quad mode set error, please check s_device[]\n");
			return -1;
		}

		if(sfc_check_quad(spinor_info) == 0) {
			rtd_pr_spi_nor_info("enabel quad mode\n");
			ret = sfc_enable_quad(spinor_info);
			if(ret == -1) {
			    rtd_pr_spi_nor_err("enabel quad mode fail\n");
			    return ret;
			}
		} else {
			rtd_pr_spi_nor_info("enabel quad mode\n");
		}
	}

#ifdef CONFIG_RTK_SPI_WP_HW
	if(sfc_check_quad(spinor_info) == 1) {
		rtd_pr_spi_nor_err("the WP_HW is conflict with quad mode, only chose the one\n");
		return -1;
	}
#endif
	return ret;
}

void hex_dump(char *string, char *buffer, size_t len)
{
	size_t i = 0;

	rtd_pr_spi_nor_info("hexdump: %s address: 0x%p, len:0x%zx\n", string, buffer, len);

	for(i = 0; i < len; i++) {
		if(i % 16 == 0)
			rtd_pr_spi_nor_info("\n");
		rtd_pr_spi_nor_info("0x%x ", buffer[i]);
	}
	rtd_pr_spi_nor_info("\n");
}

static int sfc_write (struct spinor_chip_t *spinor_chip, loff_t to, size_t len, char *buf)
{
	int ret = 0;
	__maybe_unused size_t write_len = 0;
	__maybe_unused size_t write_total_len = 0;
	__maybe_unused unsigned char *to_old = (unsigned char *)(unsigned long)to;
	__maybe_unused size_t len_old = len;
	__maybe_unused unsigned char *buf_old = buf;

	if((spinor_chip == NULL) || (buf == NULL))
		return -EINVAL;

	// 1st pass: write data which smaller than a single page (256 bytes) to make destination address within 256-bytes alignment
	if((len > 0) && (((to + MD_PP_DATA_SIZE - 1) & (~(MD_PP_DATA_SIZE - 1))) - to) > 0) {
		write_len = ((to + MD_PP_DATA_SIZE - 1) & (~(MD_PP_DATA_SIZE - 1))) - to;
		if(write_len > len)
			write_len = len;

		ret = sfc_write_cpu(spinor_chip, to, write_len, buf);
		if(ret < 0) {
			goto to_error;
		}

		to += write_len;
		buf += write_len;
		len -= write_len;
		write_total_len += write_len;
	}

	// 2rd pass: write data in page unit (256 bytes)
	while(len >= MD_PP_DATA_SIZE) {
		write_len = MD_PP_DATA_SIZE;
#ifndef SUPPORT_SFC_MD
		ret = sfc_write_cpu(spinor_chip, to, write_len, buf);
#else
		ret = sfc_write_md(spinor_chip, to, write_len, buf);               
#endif
		if(ret < 0) {
			goto to_error;
		}
		to += write_len;
		buf += write_len;
		len -= write_len;
		write_total_len += write_len;
	}

	// 3th pass: write data which smaller than a single page (256 bytes)
	if(len > 0) {
		write_len = len;
		ret = sfc_write_cpu(spinor_chip, to, write_len, buf);
		if(ret < 0) {
			goto to_error;
		}

		to += write_len;
		buf += write_len;
		len -= write_len;
		write_total_len += write_len;
	}

#if 0  // verify_after_write
	ret = sfc_verify_after_write(spinor_chip, to_old, len_old, buf_old);
	if(ret < 0) {
		rtd_pr_spi_nor_err("spi nor verify fail after write\n");
		return -EAGAIN;
	}
#endif

	to_error:
	return ret;
}

static int sfc_read (struct spinor_chip_t *spinor_chip, loff_t from, size_t len, char *buf)
{
#ifndef SUPPORT_SFC_MD
	sfc_read_cpu( spinor_chip, from, len, buf);
#else
	if(len < WRITE_BYTES_MASK)
	{
		sfc_read_cpu( spinor_chip, from, len, buf);
	}
	else
	{
		sfc_read_md( spinor_chip, from, len, buf);
	}
#endif
	return 0;
}

static int sfc_init(struct spinor_chip_t **pp_spinor_chip, RHAL_SPI_INIT_PARAM_T  param)
{
	int ret = 0;
	struct spinor_chip_t *tmp = NULL;
	__maybe_unused unsigned int id = 0, i =0;

	tmp  = kmalloc (sizeof(struct spinor_chip_t), GFP_KERNEL);
	if ( !tmp ) {
		rtd_pr_spi_nor_err("%s: Error, no enough memory for spinor_chip\n", __FUNCTION__);
		ret = -ENOMEM;
		goto failed;
	} else {
		memset ( (char *)tmp, 0, sizeof(struct spinor_chip_t));
		tmp-> clock_div = 0x1e;
	}

	tmp ->spinor_info =  kmalloc(sizeof(spinor_info_t), GFP_KERNEL);
	if(!tmp ->spinor_info) {
		rtd_pr_spi_nor_err("%s: Error, no enough memory for spinor_chip\n", __FUNCTION__);
		ret = -ENOMEM;
		goto failed;
	}

	spin_lock_init(&tmp ->spinor_lock);
	tmp->state = FL_READY;
	init_waitqueue_head(&tmp->wq);
	sfc_init_early(tmp, param);
       
	if(sfc_match_clock(tmp) < 0)
	{
		rtd_pr_spi_nor_err("%s: Error, can not find the available clock_div and delay_sel\n", __FUNCTION__);
		ret = -1;
		goto failed;
	}

	tmp ->read_id = sfc_read_id;
	tmp ->read = sfc_read;
	tmp ->write = sfc_write;
	tmp ->erase = sfc_erase_flash_operation;
	tmp ->unlock = sfc_unlock;

	ret = sfc_init_later(tmp );
	if(ret < 0) {
		ret = -1;
		goto failed;
	}

#ifndef SUPPORT_SFC_MD
	rtd_pr_spi_nor_info("%s : spi not support md read/write\n", __FUNCTION__);
#endif

	rtd_pr_spi_nor_info("%s : spi driver init done\n", __FUNCTION__);

	*pp_spinor_chip = tmp;
	return 0;

failed:
	if(tmp) {
		if(tmp->spinor_info)
		{
			kfree(tmp->spinor_info);
			tmp ->spinor_info = NULL;
		}
		kfree(tmp);
		tmp = NULL;
	}
	return -1;
}

static struct class *spi_class;
static dev_t spi_devt = 0;
struct cdev p_cdev;
struct spinor_chip_t *spinor_chip = NULL;

static ssize_t rtk_spi_read_operation(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	size_t read_len = 0;
	size_t read_total_len = 0;
	void * buffer = NULL;
	loff_t erase_size = 0;

	if((spinor_chip == NULL) || ((*f_pos+count)> (spinor_chip->spinor_info->size)))
	{
		rtd_pr_spi_nor_err("%s: parameter error, *f_pos(0x%llx) count(0x%zx)\n", __FUNCTION__, *f_pos, count);
		return -EINVAL;
	}

	erase_size = spinor_chip->spinor_info->min_erase_size;
	//rtd_pr_spi_nor_debug(" KMALLOC_MAX_SIZE: 0x%lx\n", KMALLOC_MAX_SIZE);

	buffer = kmalloc (erase_size, GFP_KERNEL);
	if(buffer == NULL)
		return -EINVAL;

	while(count) {
		//hex_dump(__FUNCTION__, buffer, read_len);
		memset(buffer, 0, erase_size);

		if(count >= erase_size)
			read_len = erase_size;
		else
			read_len = count;

		// SPI_ERR("%s_%d:  buffer:0x%lx, read_len:0x%x\n", __FUNCTION__, __LINE__, (unsigned long)buffer, read_len);

		spinor_chip->read(spinor_chip, *f_pos, read_len, buffer);

		// hex_dump(__FUNCTION__, buffer, read_len);
		if (copy_to_user(buf, buffer, read_len)) {
			kfree(buffer);
			return -EFAULT;
		} else {
			*f_pos += read_len;
			buf += read_len;
			read_total_len += read_len;
			count -= read_len;
		}

	}

	kfree(buffer);
	return read_total_len;
}

static ssize_t rtk_spi_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	spinor_info_t *spinor_info = NULL;

	if((spinor_chip == NULL) || ((*f_pos+count)> (spinor_chip->spinor_info->size))){
		rtd_pr_spi_nor_err("%s: parameter error, *f_pos(0x%llx) count(0x%zx)\n", __FUNCTION__, *f_pos, count);
		return -EINVAL;
	}

	spinor_info = (spinor_info_t*)(spinor_chip->spinor_info);

	if(spinor_info->multi_die_support){
		if(*f_pos >= spinor_info->per_die_size){
			sfc_die_select(0x1);

			*f_pos -= spinor_info->per_die_size;
		}else{
			sfc_die_select(0x0);

			if((*f_pos + count) > spinor_info->per_die_size){
				ssize_t read_len = spinor_info->per_die_size - *f_pos;

				ssize_t ret = rtk_spi_read_operation(filp, buf, read_len, f_pos);
				if(ret < 0){
					rtd_pr_spi_nor_err("%s_%d: read fail\n",__func__, __LINE__);
					return ret;
				}

				*f_pos = 0x0;
				count -= read_len;
				buf += read_len;
				sfc_die_select(0x1);
			}
		}
	}

	return rtk_spi_read_operation(filp, buf, count, f_pos);
}

static ssize_t rtk_spi_write_operation(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	size_t write_total_len = 0;
	void * buffer = NULL;
	size_t count_old = count;
	loff_t offsetInSector, sectorOffset;
	loff_t erase_size = 0;
	loff_t copy_size = 0;

	if((spinor_chip == NULL) || ((*f_pos+count)> (spinor_chip->spinor_info->size)))
	{
		rtd_pr_spi_nor_err("%s: parameter error, *f_pos(0x%llx) count(0x%zx)\n", __FUNCTION__, *f_pos, count);
		return -EINVAL;
	}

	erase_size = spinor_chip->spinor_info->min_erase_size;
	offsetInSector = (*f_pos) & (erase_size - 1);
	sectorOffset = (*f_pos) - offsetInSector;
	buffer = kmalloc (erase_size, GFP_KERNEL);
	if(buffer == NULL)
		return -EINVAL;

	memset(buffer, 0x00, erase_size);
	// wrie 0xff0 - 0x1000
	if(offsetInSector) {
		spinor_chip->read(spinor_chip, sectorOffset, erase_size, buffer);

		copy_size = (count < (erase_size - offsetInSector)) ? count : (erase_size - offsetInSector);
		if (copy_from_user(buffer + offsetInSector, buf, copy_size)) {
			kfree(buffer);
			rtd_pr_spi_nor_err("%s line%d:copy_from_user fail\n", __FUNCTION__, __LINE__);
			return -EFAULT;
		}
		// hex_dump(__FUNCTION__, buffer + offsetInSector, copy_size);
		if(-1 == spinor_chip->erase(spinor_chip, sectorOffset, erase_size)) {
			kfree(buffer);
			rtd_pr_spi_nor_err("%s line %d:sfc_erase_flash fail\n", __FUNCTION__, __LINE__);
			return -EFAULT;
		}
		if(spinor_chip->write(spinor_chip, sectorOffset, erase_size, buffer) < 0)
		{
			rtd_pr_spi_nor_err("%s line %d:write flash fail\n", __FUNCTION__, __LINE__);
			kfree(buffer);
			return -EFAULT;
		}

		*f_pos += copy_size;
		buf += copy_size;
		write_total_len += copy_size;
		count -= copy_size;
	}

	// wrie 0x1000 - 0x3000
	while(count >= erase_size) {
		memset(buffer, 0x00, erase_size);
		if (copy_from_user(buffer, buf, erase_size)) {
			kfree(buffer);
			return -EFAULT;
		}

		// hex_dump(__FUNCTION__, buffer, 0x20);
		if(-1 == spinor_chip->erase(spinor_chip, *f_pos, erase_size)) {
			kfree(buffer);
			return -EFAULT;
		}
		if(spinor_chip->write(spinor_chip, *f_pos, erase_size, buffer) < 0)
		{
			rtd_pr_spi_nor_err("%s line %d:write flash fail\n", __FUNCTION__, __LINE__);
			kfree(buffer);
			return -EFAULT;
		}

		*f_pos += erase_size;
		buf += erase_size;
		write_total_len += erase_size;
		count -= erase_size;
	}

	// wrie 0x3000 - 0x3ff0
	if(count) {
		memset(buffer, 0x00, erase_size);
		spinor_chip->read(spinor_chip, *f_pos, erase_size, buffer);

		if (copy_from_user(buffer, buf, count)) {
			kfree(buffer);
			return -EFAULT;
		}
		// hex_dump(__FUNCTION__, buffer, count);

		if(-1 == spinor_chip->erase(spinor_chip, *f_pos, erase_size)) {
			kfree(buffer);
			return -EFAULT;
		}
		if(spinor_chip->write(spinor_chip, *f_pos, erase_size, buffer) < 0)
		{
			rtd_pr_spi_nor_err("%s line %d:write flash fail\n", __FUNCTION__, __LINE__);
			kfree(buffer);
			return -EFAULT;
		}

		*f_pos += count;
		buf += count;
		write_total_len += count;
		count -= count;
	}

	if(write_total_len != count_old) {
		kfree(buffer);
		rtd_pr_spi_nor_err("%s: write_total_len(0x%zx) != count_old(0x%zx)\n", __FUNCTION__, write_total_len, count_old);
		return -EINVAL;
	}

	kfree(buffer);
	return write_total_len;
}

static ssize_t rtk_spi_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	spinor_info_t* spinor_info = NULL;

	if((spinor_chip == NULL) || ((*f_pos+count)> (spinor_chip->spinor_info->size))){
		rtd_pr_spi_nor_err("%s: parameter error, *f_pos(0x%llx) count(0x%zx)\n", __FUNCTION__, *f_pos, count);
		return -EINVAL;
	}

	spinor_info = (spinor_info_t*)(spinor_chip->spinor_info);

	if(spinor_info->multi_die_support){
		if(*f_pos >= spinor_info->per_die_size){
			sfc_die_select(0x1);

			*f_pos -= spinor_info->per_die_size;
		}else{
			sfc_die_select(0x0);

			if((*f_pos + count) > spinor_info->per_die_size){
				size_t write_len = spinor_info->per_die_size - *f_pos;

				ssize_t ret = rtk_spi_write_operation(filp, buf,write_len, f_pos);
				if(ret < 0){
				    rtd_pr_spi_nor_err("%s_%d: write fail\n",__func__, __LINE__);
				    return ret;
				}

				*f_pos = 0x0;
				count -= write_len;
				buf += write_len;
				sfc_die_select(0x1);
			}
		}
	}

	return rtk_spi_write_operation(filp, buf, count, f_pos);
}

static long rtk_spi_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	long ret = 0;
	RHAL_SPI_INIT_PARAM_T  init_param;
	STATUS_REGISTER_T status_register;
	int set_clock = 0;

	switch(cmd) {
		case SPI_IOCR_INIT:
			rtd_pr_spi_nor_debug("%s_%d:cmd 0x%x\n", __FUNCTION__, __LINE__, cmd);
			if(spinor_chip == NULL)
			{
				if(0 != copy_from_user(&init_param, (void __user *)arg, sizeof(RHAL_SPI_INIT_PARAM_T)))
				{        
					rtd_pr_spi_nor_err("%s_%d fail\n", __FUNCTION__, __LINE__);
					return -EINVAL;
				}
				ret = sfc_init(&spinor_chip, init_param);
				if(ret == 0) 
				{
					init_param.cs_type = sfc_get_cs_type();
					init_param.clock = sfc_get_clk();
					init_param.flash_size = spinor_chip ->spinor_info->size;
					init_param.erase_size = spinor_chip ->spinor_info->min_erase_size;
					if(0 != copy_to_user((void __user *)arg, (void *)(&init_param), sizeof(init_param))) 
					{
						ret = -EINVAL;
					}
				} else {
					spinor_chip =  NULL;
					ret = -EINVAL;
				}
			}
			else
			{
				init_param.cs_type = sfc_get_cs_type();
				init_param.clock = sfc_get_clk();
				init_param.flash_size = spinor_chip ->spinor_info->size;
				init_param.erase_size = spinor_chip ->spinor_info->min_erase_size;
				if(0 != copy_to_user((void __user *)arg, (void *)(&init_param), sizeof(init_param))) 
				{
					ret = -EINVAL;
				}
			}
			if(-EINVAL == ret)
				rtd_pr_spi_nor_err("SPI_IOCR_INIT fail\n");
			break;
		case SPI_IOCR_GetStatusRegister:
			rtd_pr_spi_nor_debug("%s_%d:cmd 0x%x\n", __FUNCTION__, __LINE__, cmd);
			if(spinor_chip == NULL)
			{
				rtd_pr_spi_nor_err("spi don't init\n");
				return -EINVAL;
			}
			sfc_get_status_register(spinor_chip->spinor_info, &status_register);
			if(0 != copy_to_user((void __user *)arg, (void *)(&status_register), sizeof(STATUS_REGISTER_T)))
				ret = -EINVAL;
			break;
		case SPI_IOCR_LockFlash:
			rtd_pr_spi_nor_debug("%s_%d:cmd 0x%x\n", __FUNCTION__, __LINE__, cmd);
			if(spinor_chip == NULL)
			{
				rtd_pr_spi_nor_err("spi don't init\n");
				return -EINVAL;
			}
			if(sfc_lock(spinor_chip->spinor_info) == -1)
				ret = -EINVAL;
			else
				ret = 0;
			break;
		case SPI_IOCR_UnLockFlash:
			rtd_pr_spi_nor_debug("%s_%d:cmd 0x%x\n", __FUNCTION__, __LINE__, cmd);
			if(spinor_chip == NULL)
			{
				rtd_pr_spi_nor_err("spi don't init\n");
				return -EINVAL;
			}
			if(sfc_unlock(spinor_chip) == -1)
				ret = -EINVAL;
			else
				ret = 0;
			break;
		case SPI_IOCR_SET_CLK:
			rtd_pr_spi_nor_debug("%s_%d:cmd 0x%x\n", __FUNCTION__, __LINE__, cmd);
			if(spinor_chip == NULL)
			{
				rtd_pr_spi_nor_err("spi don't init\n");
				return -EINVAL;
			}
			if(0 != copy_from_user(&set_clock, (void __user *)arg, sizeof(set_clock)))
			{
				ret = -EINVAL;
			}
			if(sfc_set_clk(spinor_chip, set_clock) == -1)
				ret = -EINVAL;
			else
				ret = 0;
			break;

		case SPI_IOCR_GetParameter:
			rtd_pr_spi_nor_debug("%s_%d:cmd 0x%x\n", __FUNCTION__, __LINE__, cmd);
			if(spinor_chip == NULL)
			{
				rtd_pr_spi_nor_err("spi don't init\n");
				return -EINVAL;
			}
			init_param.cs_type = sfc_get_cs_type();
			init_param.clock = sfc_get_clk();
			init_param.flash_size = spinor_chip ->spinor_info->size;
			init_param.erase_size = spinor_chip ->spinor_info->min_erase_size;
			if(copy_to_user((void __user *)arg, &init_param, sizeof(init_param)))
				ret = -EINVAL;
			else
				ret = 0;
			break;        
		default:
			rtd_pr_spi_nor_err("%s_%d:cmd 0x%x\n", __FUNCTION__, __LINE__, cmd);
			ret = -EINVAL;
			break;
	}

	return ret;
}

static int rtk_spi_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static int rtk_spi_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static loff_t rtk_spi_lseek(struct file *file, loff_t offset, int orig)
{
	if(spinor_chip == NULL)
		return -EINVAL;
	switch (orig) {
		case SEEK_SET:
			break;
		case SEEK_CUR:
			offset += file->f_pos;
			break;
		case SEEK_END:
			offset = spinor_chip->spinor_info->size;
			break;
		default:
			return -EINVAL;
	}

	if (offset >= 0 && offset <= (spinor_chip->spinor_info->size))
		return file->f_pos = offset;

	return -EINVAL;
}

static struct file_operations rtk_spi_fops = {
	.owner = THIS_MODULE,
	.read = rtk_spi_read,
	.write = rtk_spi_write,
	.unlocked_ioctl = rtk_spi_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = rtk_spi_ioctl,
#endif
	.release = rtk_spi_release,
	.open = rtk_spi_open,
	.llseek = rtk_spi_lseek,
};

#if 0
#define SPI_TEST_ADDR  0x00000000
void rtk_spi_test(void)
{
        unsigned char read_buf[100] = {0xff};
        unsigned char write_buf[100] = {0};
         struct erase_info instr;
        int i;
        
        rtd_pr_spi_nor_info("xun_yang: 11\n");
        
        sfc_read(spinor_chip, SPI_TEST_ADDR, sizeof(read_buf), read_buf);
        for(i = 0; i < 16; i++) {
                if(i % 16 == 0)
                        rtd_pr_spi_nor_info("\n");
                rtd_pr_spi_nor_info("0x%x\t", read_buf[i]);
        }
 
        rtd_pr_spi_nor_info("\nerase...\n");
        instr.addr = SPI_TEST_ADDR;
        instr.len = 0x1000;
    
        if(-1 == sfc_erase_flash(spinor_chip, instr.addr, instr.len))
       {
                rtd_pr_spi_nor_info("\nerase fail\n");
        }
        else
        {
                rtd_pr_spi_nor_info("\nerase OK\n");
         }
        
#if 1
        sfc_read(spinor_chip, SPI_TEST_ADDR, sizeof(read_buf), read_buf);

        for(i = 0; i < 16; i++) {
                if(i % 16 == 0)
                        rtd_pr_spi_nor_info("\n");
                rtd_pr_spi_nor_info("0x%x\t", read_buf[i]);
        }

        write_buf[0] = 0x99;
        write_buf[1] = 0x11;
        write_buf[2] = 0x11;
        write_buf[3] = 0x33;
        write_buf[4] = 0x44;
        write_buf[5] = 0x55;
        write_buf[6] = 0x99;
        write_buf[7] = 0x33;
        write_buf[8] = 0x22;
        write_buf[9] = 0x11;
        write_buf[10] = 0xbb;

        rtd_pr_spi_nor_info("\nread after write\n");
        sfc_write(spinor_chip, SPI_TEST_ADDR, sizeof(write_buf), write_buf);
        sfc_read(spinor_chip, SPI_TEST_ADDR, sizeof(read_buf), read_buf);

        for(i = 0; i < 16; i++) {
                if(i % 16 == 0)
                        rtd_pr_spi_nor_info("\n");
                rtd_pr_spi_nor_info("0x%x\t", read_buf[i]);
        }
#endif

#if 0
        rtd_pr_spi_nor_info("\nerase...\n");
        instr.addr = SPI_TEST_ADDR;
        instr.len = 0x1000;
        if(-1 == sfc_erase_flash(spinor_chip, instr.addr, instr.len))
                rtd_pr_spi_nor_err("\nerase fail\n");

        sfc_read(spinor_chip, SPI_TEST_ADDR, sizeof(read_buf), read_buf);


        for(i = 0; i < 16; i++) {
                if(i % 16 == 0)
                        rtd_pr_spi_nor_info("\n");
                rtd_pr_spi_nor_info("0x%x\t", read_buf[i]);
        }
#endif
}
#endif

static int rtk_spi_probe(struct platform_device *pdev)
{    
	spi_virt_base = of_iomap(pdev->dev.of_node, 0);
	rtd_pr_spi_nor_err("spi_virt_base = %p\n",spi_virt_base);
	WARN(!spi_virt_base, "unable to map rtk-spi area\n");

	if(IS_ERR(spi_virt_base))
		return -EIO;

	SPI_BASE_ADDR  = (unsigned long)spi_virt_base;

	return 0;
}

static int rtk_spi_remove(struct platform_device *pdev)
{
	return 0;
}

static int rtk_spi_suspend(struct device *dev)
{
	int ret = 0, retry = 0;
	unsigned char data = 0;
	int i = 0;

	if(spinor_chip != NULL) {
		if(sfc_get_device(spinor_chip, FL_SUSPEND) == -1) {
			ret = -1;
			goto end;
		}

		// wait FDMA DONE
		while(rtd_inl(MD_MD_FDMA_CTRL1_reg) & MD_MD_FDMA_CTRL1_fdma_go_mask); // wait for MD done its operation

		// wait WIP
		do {
			udelay(SFC_DELAY);
			sfc_get_feature(CMD_READ_STATUS1, &data);
		} while ((data & STATUS_WIP) && (++retry < SUSPEND_TIMEOUT));

		if (retry == SUSPEND_TIMEOUT) {
			rtd_pr_spi_nor_err("spi flash suspend fail: %s wait timet out  0x%x\n", __FUNCTION__, data);
			ret = -1;
		}

		if(spinor_chip->spi_reg_array == NULL) {
			spinor_chip->spi_reg_array = (void *)kmalloc(SPI_REG_NUM * sizeof(unsigned int), GFP_KERNEL);
		}

		if(spinor_chip->spi_reg_array == NULL) {
			rtd_pr_spi_nor_err("spi flash suspend fail\n");
			return -1;
		}

		for(i = 0; i < SPI_REG_NUM; i++) {
			spinor_chip->spi_reg_array[i] = rtd_inl(SPI_REG_BASE + i * sizeof(unsigned int));
		}

		// sfc_debug();
		sfc_release_device(spinor_chip);
	}

end:
	return ret;
}

static int rtk_spi_resume(struct device *dev)
{
	int ret = 0;
	int i = 0;

	if(spinor_chip != NULL) {
		if(sfc_get_device(spinor_chip, FL_SUSPEND) == -1) {
			ret = -1;
			if(spinor_chip->spi_reg_array) {
				kfree(spinor_chip->spi_reg_array);
				spinor_chip->spi_reg_array = NULL;
			}
			goto end;
		}

		if(spinor_chip->spi_reg_array != NULL) {
			// sfc_init_early(spinor_chip, spinor_chip->spinor_param);

			for(i = 0; i < SPI_REG_NUM; i++) {
				rtd_outl(SPI_REG_BASE + i * sizeof(unsigned int), spinor_chip->spi_reg_array[i]);
			}

			if(spinor_chip->spi_reg_array[4] & SB2_SFC_WP_ce_n_mask)
				rtd_outl(SB2_SFC_WP_reg, spinor_chip->spi_reg_array[4] |SB2_SFC_WP_ce_n(1) | SB2_SFC_WP_write_en2(1));

			kfree(spinor_chip->spi_reg_array);
			spinor_chip->spi_reg_array = NULL;
			// sfc_debug();
		} else {
			rtd_pr_spi_nor_err("%s line%d: spi flash resume fail\n", __FUNCTION__, __LINE__);
			ret = -1;
		}

		sfc_release_device(spinor_chip);
	}

	end:
	return ret;
}

static const struct dev_pm_ops rtk_spi_pm_ops = {
	.suspend = rtk_spi_suspend,
	.resume = rtk_spi_resume,
#ifdef CONFIG_HIBERNATION	
	.freeze = rtk_spi_suspend,
	.thaw = rtk_spi_resume,
	.poweroff = rtk_spi_suspend,
	.restore = rtk_spi_resume,
#endif	
};

static const struct of_device_id rtk_spi_of_match[] = {
	{
	.compatible = "realtek,rtk_spi",
	},
	{},
};

static struct platform_driver rtk_spi_driver = {
	.driver = {
		.name = "rtk_sfc",
		.pm = &rtk_spi_pm_ops,
		.of_match_table = rtk_spi_of_match,
	},

	.probe = rtk_spi_probe,
	.remove = rtk_spi_remove,
};

static int register_spi_cdev(void)
{
	int ret = 0;

	ret = alloc_chrdev_region(&spi_devt, 0, 1, "spi");
	if (ret) {
		rtd_pr_spi_nor_err("can't get dev_t\n");
		spi_devt = 0;
		return ret;
	}

#if 0
	p_cdev = cdev_alloc();
	if(p_cdev == NULL) {
		rtd_pr_spi_nor_err("can't alloc cdev\n");
		goto end;
	}
	p_cdev->owner = THIS_MODULE;
	p_cdev->ops = &rtk_spi_fops;
#else
	cdev_init(&p_cdev, &rtk_spi_fops);
	p_cdev.owner = THIS_MODULE;
#endif
	ret = cdev_add(&p_cdev, spi_devt, 1);
	if (ret < 0) {
		rtd_pr_spi_nor_err("add chr dev failed\n");
		goto end;
	}

	// auto generate the /dev/rtk_spi,
	spi_class = class_create(THIS_MODULE, "spi");
	if (IS_ERR(spi_class )) {
		rtd_pr_spi_nor_err("couldn't create spi class\n");
		return PTR_ERR(spi_class);
	}
	device_create(spi_class, NULL, spi_devt, NULL, "rtk_spi");  
	rtd_pr_spi_nor_info("add chr dev OK (%d, %d)\n", MAJOR(spi_devt), MINOR(spi_devt));
	return 0;

end:
	cdev_del(&p_cdev);
	if(spi_devt) {
		unregister_chrdev_region(spi_devt, 1);
		spi_devt = 0;
	}
	if(spi_class)
	{
		device_destroy(spi_class, MKDEV(0, 0));
		class_destroy(spi_class);
	}
	return -1;
}

static int unregister_spi_cdev(void)
{
	cdev_del(&p_cdev);
	if(spi_devt) {
		unregister_chrdev_region(spi_devt, 1);
		spi_devt = 0;
	}
	if(spi_class)
	{
		device_destroy(spi_class, MKDEV(0, 0));
		class_destroy(spi_class);
	}
	return -1;
}

/*****************************************************
type = 0	: spi flash
type = 1	: fcic
******************************************************/
int spi_read_internal(unsigned int type, unsigned long offset, unsigned int count, void *buf)
{

	int ret = 0;
	size_t read_len = 0;
	RHAL_SPI_INIT_PARAM_T  init_param;
	void * buffer = NULL;

	memset(&init_param, 0x00, sizeof(RHAL_SPI_INIT_PARAM_T));
	/*spi init*/
	if(spinor_chip == NULL){
		if(type==0){
			init_param.cs_type = RHAL_CS0_SPIFLASH;
		}else if(type == 1){
			init_param.cs_type = RHAL_CS1_FCIC;
		}else{
			return -EINVAL;
		}

		ret = sfc_init(&spinor_chip, init_param);
		if(ret == 0) 
		{
			init_param.cs_type = sfc_get_cs_type();
			init_param.clock = sfc_get_clk();
			init_param.flash_size = spinor_chip ->spinor_info->size;
			init_param.erase_size = spinor_chip ->spinor_info->min_erase_size;
		} else {
			spinor_chip =  NULL;
			return -EINVAL;
		}
	}else{
		init_param.cs_type = sfc_get_cs_type();
		init_param.clock = sfc_get_clk();
		init_param.flash_size = spinor_chip ->spinor_info->size;
		init_param.erase_size = spinor_chip ->spinor_info->min_erase_size;
	}
	if(-EINVAL == ret){
		rtd_pr_spi_nor_err("SPI_IOCR_INIT fail\n");
		return -EINVAL;
		}


	if(((offset+count)> (spinor_chip->spinor_info->size)) || buf == NULL)
	{
		rtd_pr_spi_nor_err("%s: parameter error, offset(0x%lx) count(0x%zx)\n", __FUNCTION__, offset, count);
		return -EINVAL;
	}
	
	/*spi read*/
	buffer = kmalloc (init_param.erase_size, GFP_KERNEL);
	if(buffer == NULL)
		return -EINVAL;
	
	while(count) {
		memset(buffer, 0, init_param.erase_size);

		if(count >= init_param.erase_size)
			read_len = init_param.erase_size;
		else
			read_len = count;
		
		spinor_chip->read(spinor_chip, (loff_t)offset, read_len, buffer);
		memcpy(buf, buffer, read_len);

		offset += read_len;
		buf += read_len;
		count -= read_len;
	}

	kfree(buffer);
	return 0;
	
}
EXPORT_SYMBOL(spi_read_internal);
static int __init rtk_spi_nor_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&rtk_spi_driver);
	if (ret) {
		rtd_pr_spi_nor_info("\nregister driver %s fail\n", rtk_spi_driver.driver.name);
		goto failed;
	}
	ret = register_spi_cdev();
	if(ret == -1) {
		rtd_pr_spi_nor_err("\nregister cdev %s fail\n", rtk_spi_driver.driver.name);
		goto failed;
	}
	return 0;

failed:
	platform_driver_unregister(&rtk_spi_driver);
	return -EINVAL;
}

static void __exit rtk_spi_nor_exit(void)
{
	unregister_spi_cdev();
	platform_driver_unregister(&rtk_spi_driver);
	rtd_pr_spi_nor_err("[SFC-INFO]rtk_spi_nor_exit!!!\n");
}

module_init(rtk_spi_nor_init);

module_exit(rtk_spi_nor_exit);

MODULE_LICENSE("GPL");

MODULE_AUTHOR("xun_yang <xun_yang@apowertec.com>");

MODULE_DESCRIPTION("MTD chip driver for Realtek Serial Nor Flash Controller");
