
#include <linux/kernel.h>

#include <linux/dma-mapping.h>
#include <linux/list.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <rbus/lzma_reg.h>
#include <rtk_kdriver/io.h>

#include "zacc.h"

spinlock_t zacc_hw_lock;
bool zacc_recovery = false;
unsigned int zacc_cmd_state[ZACC_QUEUE_SIZE];
const unsigned int reg_offset[ZACC_HW_QUEUE_SIZE] = {0, 0x24, 0x4c, 0x70};
unsigned int zacc_hw_resource[ZACC_HW_QUEUE_SIZE] = {0, 0, 0, 0};

#if (ZACC_QUEUE_SIZE > ZACC_HW_QUEUE_SIZE)
#error ZACC_QUEUE_SIZE must be equal to or less than ZACC_HW_QUEUE_SIZE
#endif

int zacc_ready(void)
{
	return zacc_decoder_ready() && zacc_encoder_ready();
}
EXPORT_SYMBOL(zacc_ready);

unsigned int zacc_get_hw_resource(void)
{
	unsigned long flags;
	unsigned int index = 0xffffffff;
	unsigned int i;

	do {
		spin_lock_irqsave(&zacc_hw_lock, flags);

		for (i = 0; i < ZACC_QUEUE_SIZE; i++) {
			if (zacc_hw_resource[i] == 0) {
				index = i;
				zacc_hw_resource[i] = 1;
				break;
			}
		}

		spin_unlock_irqrestore(&zacc_hw_lock, flags);
	} while (index == 0xffffffff);

	return index;
}

void zacc_put_hw_resource(unsigned int index)
{
	unsigned long flags;

	spin_lock_irqsave(&zacc_hw_lock, flags);

	if (zacc_hw_resource[index] == 1)
		zacc_hw_resource[index] = 0;

	spin_unlock_irqrestore(&zacc_hw_lock, flags);
}

struct zacc_dev *__zacc_device(struct list_head *list, spinlock_t *lock)
{
	struct zacc_dev *zdev;
	unsigned long flags;

	if (list_empty(list))
		return NULL;

	spin_lock_irqsave(lock, flags);

	zdev = list_first_entry(list, struct zacc_dev, list);
	if (!list_is_singular(list))
		list_rotate_left(list);

	spin_unlock_irqrestore(lock, flags);

	return zdev;
}

struct zacc_desc *zacc_get_desc(struct zacc_dev *zdev)
{
	struct zacc_desc *desc = NULL;
	unsigned long flags;

	do {
		spin_lock_irqsave(&zdev->lock, flags);

		if (zdev->used < zdev->size) {
			desc = &zdev->desc[zdev->head];
			zdev->head = (zdev->head + 1) % zdev->size;
			zdev->used++;
		}

		spin_unlock_irqrestore(&zdev->lock, flags);
	} while (!desc);

	return desc;
}

void zacc_put_desc(struct zacc_dev *zdev, struct zacc_desc *desc)
{
	unsigned long flags;
	bool done = false;

	do {
		spin_lock_irqsave(&zdev->lock, flags);

		if (desc - zdev->desc == zdev->tail) {
			zdev->tail = (zdev->tail + 1) % zdev->size;
			zdev->used--;

			done = true;
		}

		spin_unlock_irqrestore(&zdev->lock, flags);
	} while (!done);
}

static int zacc_alloc_desc(struct zacc_dev *zdev)
{
	struct device *dev = &zdev->pdev->dev;
	size_t sz = sizeof(struct zacc_desc) * zdev->size;
	zdev->desc = dma_alloc_coherent(dev, sz, &zdev->desc_pa, GFP_DMA32);
	printk("%s: desc=0x%lx, desc_pa=0x%lx\n", __FUNCTION__, (unsigned long)zdev->desc, (unsigned long)zdev->desc_pa);
	return zdev->desc ? 0 : -ENOMEM;
}

static void zacc_free_desc(struct zacc_dev *zdev)
{
	struct device *dev = &zdev->pdev->dev;
	size_t sz = sizeof(struct zacc_desc) * zdev->size;
	dma_free_coherent(dev, sz, zdev->desc, zdev->desc_pa);
}

static void zacc_init(struct zacc_dev *zdev)
{
	int i;
	static bool inited = false;

	zdev->head = 0;
	zdev->tail = 0;
	zdev->used = 0;

	memset(zdev->desc, 0, sizeof(struct zacc_desc) * zdev->size);

	if (!inited) {
		spin_lock_init(&zacc_hw_lock);
		for (i = 0; i < ZACC_QUEUE_SIZE; i++)
			zacc_cmd_state[i] = 0;
		inited = true;
	}
	rtd_maskl(LZMA_LZMA_DMA_RD_Ctrl_reg, 0xfff0fcfe, (0x2 << 18) | (0x3 << 16) | (0x3 << 8) | (0x1 << 0)); //bit0: force_all_rst=1, bit8: max_outstanding=3, bit16: cmd_burst_length=3, bit18: data_burst_length=2
	rtd_maskl(LZMA_LZMA_DMA_WR_Ctrl_reg, 0xfffcfcfe, (0x2 << 16) | (0x3 << 8) | (0x1 << 0)); //bit0: force_all_rst=1, bit8: max_outstanding=3, bit16: data_burst_length=2
	rtd_maskl(LZMA_INT_STATUS_reg, 0x3fffffdf, (ZACC_HW_DONE_EN << 31) | (0x0 << 30) | (0x1 << 5)); //bit31: hw_done_en=0, bit30: cmd_register_mode=0, bit5: cut_value1_en=1
	rtd_maskl(LZMA_LZMA_DUMMY_reg, 0xbfffffff, 0x1 << 30); //reg_lzma_4k_sram_ready_eco=1 (0:bypass, 1:valid)
}

static void zacc_stop(struct zacc_dev *zdev)
{

}

struct zacc_dev *__zacc_probe(struct platform_device *pdev)
{
	struct zacc_dev *zdev;
	int i;

	zdev = devm_kzalloc(&pdev->dev, sizeof(struct zacc_dev), GFP_KERNEL);
	if (!zdev)
		return NULL;

	platform_set_drvdata(pdev, zdev);
	zdev->pdev = pdev;

	INIT_LIST_HEAD(&zdev->list);
	spin_lock_init(&zdev->lock);

	zdev->size = ZACC_QUEUE_SIZE;

	if (zacc_alloc_desc(zdev) < 0)
		return NULL;

	for (i = 0; i < ZACC_QUEUE_SIZE; i++) {
		zdev->dst_pg[i] = alloc_pages(GFP_DMA32, 1);
		if (!zdev->dst_pg[i])
			goto __failed;

		zdev->src_pg[i] = alloc_pages(GFP_DMA32, 0);
		if (!zdev->src_pg[i])
			goto __failed;
	}

	zacc_init(zdev);

	return zdev;
__failed:
	for (i = 0; i < ZACC_QUEUE_SIZE; i++) {
		if (!zdev->dst_pg[i])
			break;
		__free_pages(zdev->dst_pg[i], 1);

		if (!zdev->src_pg[i])
			break;
		__free_pages(zdev->src_pg[i], 0);
	}

	return NULL;
}

void __zacc_remove(struct platform_device *pdev)
{
	struct zacc_dev *zdev = platform_get_drvdata(pdev);
	int i;

	zacc_stop(zdev);
	zacc_free_desc(zdev);

	for (i = 0; i < ZACC_QUEUE_SIZE; i++) {
		__free_pages(zdev->dst_pg[i], 1);
		__free_pages(zdev->src_pg[i], 0);
	}
}

int zacc_suspend(struct device *dev)
{
	struct zacc_dev *zdev = dev_get_drvdata(dev);
	zacc_stop(zdev);
	return 0;
}

int zacc_resume(struct device *dev)
{
	struct zacc_dev *zdev = dev_get_drvdata(dev);
	zacc_init(zdev);
	return 0;
}

SIMPLE_DEV_PM_OPS(zacc_pm_ops, zacc_suspend, zacc_resume);
