#include <linux/module.h>

#include <linux/kernel.h>

#include <linux/dma-mapping.h>
#include <linux/highmem.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/smp.h>
#include <linux/zacc.h>
#include <rbus/lzma_reg.h>
#include <rtk_kdriver/io.h>
#include <linux/delay.h>
#include <linux/smp.h>
#include "zacc.h"

#define ZACC_DEC_TIMEOUT_COUNT1		4000
#define ZACC_DEC_TIMEOUT_COUNT2		4000
#define ZACC_DEC_MAX_RETRY_COUNT	3
#define ZACC_ENABLE_RECOVERY		0
#define ZACC_USE_HW_RESET		1

static LIST_HEAD(zacc_dec_list);
static spinlock_t zacc_dec_lock;
static unsigned long zacc_dec_buf[ZACC_QUEUE_SIZE];

extern spinlock_t zacc_hw_lock;
extern bool zacc_recovery;
extern int zacc_cmd_state[ZACC_QUEUE_SIZE];
extern const unsigned int reg_offset[ZACC_HW_QUEUE_SIZE];

//extern int LzmaDecode2(void *dest, size_t *destLen, const void *src, size_t *srcLen, const void *probs);

int zacc_decoder_ready(void)
{
	return !list_empty(&zacc_dec_list);
}

static struct zacc_dev *zacc_decoder(void)
{
	return __zacc_device(&zacc_dec_list, &zacc_dec_lock);
}

static void zacc_dec_recovery(void)
{
#if ZACC_USE_HW_RESET
	unsigned long flags;
	int index = smp_processor_id();
	int i;

	spin_lock_irqsave(&zacc_hw_lock, flags);
	if (zacc_recovery) {
		spin_unlock_irqrestore(&zacc_hw_lock, flags);
		return;
	}
	else {
		zacc_recovery = true;
	}
	spin_unlock_irqrestore(&zacc_hw_lock, flags);

	zacc_cmd_state[index] = 0;
	while (1) {
		for (i = 0; i < ZACC_QUEUE_SIZE; i++) {
			if (zacc_cmd_state[i])
				break;
		}
		if (i == ZACC_QUEUE_SIZE)
			break;
		udelay(1);
	}

	rtd_outl(SYS_REG_SYS_CLKEN3_reg, BIT(28));
	udelay(10);
	rtd_outl(SYS_REG_SYS_SRST3_reg, BIT(28));
	udelay(10);
	rtd_outl(SYS_REG_SYS_SRST3_reg, BIT(28) | BIT(0));
	udelay(10);
	rtd_outl(SYS_REG_SYS_CLKEN3_reg, BIT(28) | BIT(0));
	udelay(10);
	rtd_maskl(LZMA_LZMA_DMA_RD_Ctrl_reg, 0xfff0fcfe, (0x2 << 18) | (0x3 << 16) | (0x3 << 8) | (0x1 << 0)); //bit0: force_all_rst=1, bit8: max_outstanding=3, bit16: cmd_burst_length=3, bit18: data_burst_length=2
	rtd_maskl(LZMA_LZMA_DMA_WR_Ctrl_reg, 0xfffcfcfe, (0x2 << 16) | (0x3 << 8) | (0x1 << 0)); //bit0: force_all_rst=1, bit8: max_outstanding=3, bit16: data_burst_length=2
	rtd_maskl(LZMA_INT_STATUS_reg, 0x3fffffdf, (ZACC_HW_DONE_EN << 31) | (0x0 << 30) | (0x1 << 5)); //bit31: hw_done_en=0, bit30: cmd_register_mode=0, bit5: cut_value1_en=1
	rtd_maskl(LZMA_LZMA_DUMMY_reg, 0xbfffffff, 0x1 << 30); //reg_lzma_4k_sram_ready_eco=1 (0:bypass, 1:valid)

	spin_lock_irqsave(&zacc_hw_lock, flags);
	zacc_recovery = false;
	spin_unlock_irqrestore(&zacc_hw_lock, flags);
#else
	if (!(rtd_inl(LZMA_CMD_GO_SWAP_reg) & (0x1 << 24))) {
		rtd_maskl(LZMA_CMD_GO_SWAP_reg, ~(0x1 << 24), 0x1 << 24);
		udelay(4);
		rtd_maskl(LZMA_CMD_GO_SWAP_reg, ~(0x1 << 24), 0x0 << 24);
	}
#endif
}

#if 0
static int zacc_decode2(void *dst, void const *src, size_t size, void const *dec_buf)
{
	int err;
	size_t padding_size, src_len, dst_len;

	padding_size = 3 + ((unsigned char *)src)[size - 1];
	src_len = size - padding_size;
	dst_len = PAGE_SIZE;

	err = LzmaDecode2(dst, &dst_len, src, &src_len, dec_buf);

	if (err < 0) {
		printk(KERN_ERR "LzmaDecode2 return err = %d\n", err);
		return -EIO;
	}

	return 0;
}
#endif

ssize_t zacc_decode(void *dst, void const *src, size_t size)
{
	struct zacc_dev *zdev;
	struct zacc_desc *desc;
	struct zacc_decode_desc *decode_desc;
	struct zacc_decode_desc tmp_decode_desc;
	struct device *dev;
	struct page *dst_pg, *src_pg;
	dma_addr_t dst_pa, src_pa, _dst, _src, desc_pa;
	int err, index;
	unsigned int padding_size, desc_size, err_count, retry_count, timeout, cmd_status;
	bool do_retry = false, do_recovery = false;
	unsigned long flags;
	unsigned int cpu_id;
#if (ZACC_HW_DONE_EN == 1)
	unsigned int hw_done_count;
#endif

	if (!dst || !src)
		return -EINVAL;

	zdev = zacc_decoder();
	if (!zdev)
		return -ENODEV;
	dev = &zdev->pdev->dev;

	if (size == PAGE_SIZE) {
		memcpy(dst, src, PAGE_SIZE);
		return PAGE_SIZE;
	}

	preempt_disable();
	cpu_id = smp_processor_id();
	desc = zacc_get_desc(zdev);
	index = desc - zdev->desc;
	desc_size = sizeof(struct zacc_desc);
	desc_pa = index * desc_size + zdev->desc_pa;

	if (ZACC_QUEUE_SHIFT > 1)
		index = cpu_id;
#ifdef CONFIG_ARCH_RTK6702
	index = zacc_get_hw_resource();
#endif

	dst_pg = zdev->dst_pg[index];
	src_pg = zdev->src_pg[index];

	_dst = page_address(dst_pg);
	_src = page_address(src_pg);
	memcpy(_src, src, size);

	dst_pa = dma_map_page(dev, dst_pg, 0, PAGE_SIZE, DMA_FROM_DEVICE);
	src_pa = dma_map_page(dev, src_pg, 0, size, DMA_TO_DEVICE);

	retry_count = 0;

retry:
	do_retry = false;
	do_recovery = false;
	zacc_cmd_state[index] = 0;

	if (retry_count >= ZACC_DEC_MAX_RETRY_COUNT) {
		err = -1;//zacc_decode2(dst, src, size, zacc_dec_buf[index]);
		if (err < 0) {
			/* hex-dump source compressed data as debug information */
			print_hex_dump(KERN_ERR, "input data: ", DUMP_PREFIX_ADDRESS,
				16, 1, _src, size, true);
			panic("rtk-zacc decode2 error !! desc(0x%lx): 0x%x, 0x%x, 0x%x, 0x%x\n",
				(unsigned long)desc, desc->data[0], desc->data[1], desc->data[2], desc->data[3]);
		}
	}
	else {
#if ZACC_USE_HW_RESET
		while (zacc_recovery) {
			udelay(1);
		}
#else
		while (rtd_inl(LZMA_CMD_GO_SWAP_reg) & (0x1 << 24)) {
			udelay(1);
		}
#endif
		zacc_cmd_state[index] = 1;
		memset((void *)&tmp_decode_desc, 0, ZACC_DESC_SIZE);
		tmp_decode_desc.decode = 1;
		tmp_decode_desc.dst_addr = dst_pa >> 4;
		tmp_decode_desc.src_addr = src_pa >> 4;

		padding_size = 3 + ((unsigned char *)_src)[size - 1];
		tmp_decode_desc.comp_size = size - padding_size;
		tmp_decode_desc.decomp_size = PAGE_SIZE;
		tmp_decode_desc.crc_msb = ((unsigned char *)_src)[size - 2];
		tmp_decode_desc.crc_lsb = ((unsigned char *)_src)[size - 3];

		decode_desc = (struct zacc_decode_desc *)((unsigned long)desc);
		memcpy((void *)decode_desc, (void *)&tmp_decode_desc, ZACC_DESC_SIZE);

		rtd_outl(LZMA_CMD_0_BASE_reg + reg_offset[index], desc_pa);
		rtd_outl(LZMA_CMD_0_LIMIT_reg + reg_offset[index], (desc_pa + 0x20));
		rtd_outl(LZMA_CMD_0_RPTR_reg + reg_offset[index], desc_pa);
		rtd_outl(LZMA_CMD_0_WPTR_reg + reg_offset[index], (desc_pa + 0x10));
#if (ZACC_HW_DONE_EN == 1)
		hw_done_count = rtd_inl(LZMA_CMD_0_dec_reg + reg_offset[index]) & 0x0f;
#endif
		wmb();
#if ZACC_USE_HW_RESET
		if (zacc_recovery) {
			retry_count++;
			goto retry;
		}
#else
		if (rtd_inl(LZMA_CMD_GO_SWAP_reg) & (0x1 << 24)) {
			retry_count++;
			goto retry;
		}
#endif
		rtd_outl(LZMA_CMD_GO_SWAP_reg, 0x04000000 | (0xf << (0x6 * index)));
		wmb();

		err_count = 0;
		timeout = ZACC_DEC_TIMEOUT_COUNT1;
		while (1) {
			cmd_status = rtd_inl(LZMA_CMD_GO_SWAP_reg);
#if (ZACC_HW_DONE_EN == 1)
			if (((cmd_status & (0x1 << (0x6 * index))) == 0) &&
				(hw_done_count != (rtd_inl(LZMA_CMD_0_dec_reg + reg_offset[index]) & 0x0f))) {
				break;
			}
#else
			if ((cmd_status & (0x1 << (0x6 * index))) == 0) {
				break;
			}
#endif
			if (++err_count >= timeout) {
				do_recovery = true;
				//do_retry = true;
				break;
			}
#if ZACC_USE_HW_RESET
			if (zacc_recovery) {
				retry_count++;
				goto retry;
			}
#else
			if (cmd_status & (0x1 << 24)) {
				retry_count++;
				goto retry;
			}
#endif
			udelay(1);
		}

		err_count = 0;
		timeout = ZACC_DEC_TIMEOUT_COUNT2;
		while (1) {
			if ((decode_desc->hw_done) || (++err_count >= timeout)) {
				break;
			}
#if ZACC_USE_HW_RESET
			if (zacc_recovery) {
				retry_count++;
				goto retry;
			}
#else
			if (rtd_inl(LZMA_CMD_GO_SWAP_reg) & (0x1 << 24)) {
				retry_count++;
				goto retry;
			}
#endif
			udelay(1);
		}

		memcpy((void *)&tmp_decode_desc, (void *)decode_desc, ZACC_DESC_SIZE);
		if ((!tmp_decode_desc.hw_done) ||
			(tmp_decode_desc.comp_size != (size - padding_size)) ||
			(tmp_decode_desc.decode != 1) ||
			(tmp_decode_desc.decomp_size != PAGE_SIZE)||
			(tmp_decode_desc.dst_addr != (dst_pa >> 4)) ||
			(tmp_decode_desc.src_addr != (src_pa >> 4)) ||
			(tmp_decode_desc.unused_0 != 0)) {
			do_recovery = true;
		}

		if ((!tmp_decode_desc.hw_done) ||
			(tmp_decode_desc.cmd_err) ||
			(tmp_decode_desc.crc_err) ||
			(tmp_decode_desc.decode_err) ||
			(tmp_decode_desc.dist_err) ||
			(tmp_decode_desc.size_err)) {
			do_retry = true;
		}

		if ((do_recovery) || (do_retry))
			err = -EIO;
		else
			err = PAGE_SIZE;

		if (err < 0) {
			if (retry_count++ >= ZACC_DEC_MAX_RETRY_COUNT) {
				/* hex-dump source compressed data as debug information */
				print_hex_dump(KERN_ERR, "input data: ", DUMP_PREFIX_ADDRESS,
					16, 1, _src, size, true);
				panic("rtk-zacc decode error !! desc(0x%lx): 0x%x, 0x%x, 0x%x, 0x%x\ncmd_err:%d, crc_err:%d, decode_err:%d, dist_err:%d, size_err:%d, hw_done:%d",
					(unsigned long)desc, desc->data[0], desc->data[1], desc->data[2], desc->data[3],
					decode_desc->cmd_err, decode_desc->crc_err, decode_desc->decode_err,
					decode_desc->dist_err, decode_desc->size_err, decode_desc->hw_done);
			}
			else {
#if ZACC_ENABLE_RECOVERY
				if (do_recovery)
					zacc_dec_recovery();
#endif
				goto retry;
			}
		}
	}

#if (ZACC_HW_DONE_EN == 0)
	udelay(1);
#endif

	//memset(desc, 0, ZACC_DESC_SIZE << 2);
	dma_unmap_page(dev, dst_pa, PAGE_SIZE, DMA_FROM_DEVICE);
	dma_unmap_page(dev, src_pa, size, DMA_TO_DEVICE);

	rmb();

	if (err > 0)
		memcpy(dst, _dst, PAGE_SIZE);

	zacc_cmd_state[index] = 0;
#ifdef CONFIG_ARCH_RTK6702
	zacc_put_hw_resource(index);
#endif
	zacc_put_desc(zdev, desc);
	preempt_enable();

	return err;
}
EXPORT_SYMBOL(zacc_decode);

static int zacc_probe(struct platform_device *pdev)
{
	struct zacc_dev *zdev;
	unsigned long flags;
	int i;

	zdev = __zacc_probe(pdev);
	if (!zdev)
		return -ENODEV;

	memset(zacc_dec_buf, 0, sizeof(unsigned long) * ZACC_QUEUE_SIZE);
	for (i = 0; i < ZACC_QUEUE_SIZE; i++) {
		zacc_dec_buf[i] = vmalloc(16 << 10); //alloc 16kb memory;
		if (!zacc_dec_buf[i])
			goto __failed;
	}

	spin_lock_irqsave(&zacc_dec_lock, flags);
	list_add_tail(&zdev->list, &zacc_dec_list);
	spin_unlock_irqrestore(&zacc_dec_lock, flags);

	pr_info("zacc_dec: %s initialized\n", pdev->name);

	return 0;

__failed:
	for (i = 0; i < ZACC_QUEUE_SIZE; i++) {
		if (zacc_dec_buf[i])
			vfree(zacc_dec_buf[i]);
	}

	return -1;
}

static int zacc_remove(struct platform_device *pdev)
{
	struct zacc_dev *zdev = platform_get_drvdata(pdev);
	unsigned long flags;
	int i;

	spin_lock_irqsave(&zacc_dec_lock, flags);
	list_del(&zdev->list);
	spin_unlock_irqrestore(&zacc_dec_lock, flags);

	for (i = 0; i < ZACC_QUEUE_SIZE; i++) {
		if (zacc_dec_buf[i])
			vfree(zacc_dec_buf[i]);
	}

	__zacc_remove(pdev);

	return 0;
}

#ifdef CONFIG_OF

static struct of_device_id const zacc_of_match[] = {
	{ .compatible = "realtek,zacc-dec", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, zacc_of_match);

#endif	/* CONFIG_OF */

static struct platform_driver zacc_dec_drv = {
	.probe	= zacc_probe,
	.remove	= zacc_remove,
	.driver	= {
		.name	= "zacc-dec",
		.owner	= THIS_MODULE,
		.pm	= &zacc_pm_ops,
		.of_match_table	= of_match_ptr(zacc_of_match),
	},
};
module_platform_driver(zacc_dec_drv);

MODULE_AUTHOR("Alex-KC Yuan <alexkc.yuan@realtek.com>");
MODULE_DESCRIPTION("Realtek ZACC HW decompressor support");
MODULE_LICENSE("GPL");
