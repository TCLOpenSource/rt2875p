
#ifndef __ZACC_H
#define __ZACC_H

#define ZACC_QUEUE_SHIFT	2
#define ZACC_QUEUE_SIZE		(1 << ZACC_QUEUE_SHIFT)
#define ZACC_DESC_SIZE		0x10
#define ZACC_HW_QUEUE_SIZE	4
#ifdef CONFIG_ARCH_RTK6702
  #define ZACC_HW_DONE_EN	0
#else
  #define ZACC_HW_DONE_EN	1
#endif

#pragma pack(push, 1)

struct zacc_encode_desc {
	u32 encode:		1;	/* 0      - Encode mode (SW) */
	u32 decode:		1;	/* 1      - Decode mode (SW) */
	u32 bypss:		1;	/* 2      - Bypass mode (SW) */
	u32 cmd_err:		1;	/* 3      - Cmd error (HW) */
	u32 src_addr:		28;	/* 31:4   - Data source address (SW) */
	u32 hw_done:		1;	/* 32     - HW done (HW) */
	u32 over_4k:		1;	/* 33     - Size over 4KB (HW) */
	u32 unused_2:		2;	/* 35:34  - Unused */
	u32 dst_addr:		28;	/* 63:36  - Data destination address (SW) */
	u32 comp_size:		13;	/* 76:64  - Compressed size (HW) */
	u32 unused_1:		3;	/* 79:77  - Unused */
	u32 crc:		16;	/* 95:80  - Crc result (HW) */
	u32 unused_0:		32;	/* 127:96 - Unused */
};

struct zacc_decode_desc {
	u32 encode:		1;	/*0       - Encode mode (SW) */
	u32 decode:		1;	/*1       - Decode mode (SW) */
	u32 bypass:		1;	/*2       - Bypass mode (SW) */
	u32 cmd_err:		1;	/*3       - Cmd error (HW) */
	u32 src_addr:		28;	/*31:4    - Data source address (SW) */
	u32 hw_done:		1;	/*32      - HW done (HW) */
	u32 size_err:		1;	/*33      - Size error (HW) */
	u32 decode_err:		1;	/*34      - Decode error (HW) */
	u32 dist_err:		1;	/*35      - Distance error (HW) */
	u32 dst_addr:		28;	/*63:36   - Data destination address (SW) */
	u32 comp_size:		23;	/*86:64   - Compressed size, 16bytes alignmen (SW) */
	u32 crc_err:		1;	/*87      - CRC error (HW) */
	u32 crc_msb:		8;	/*95:88   - Golden crc msb 8 bit (SW) */
	u32 decomp_size:	23;	/*118:96  - Decompressed size, byte (SW) */
	u32 unused_0:		1;	/*119     - Unused */
	u32 crc_lsb:		8;	/*127:120 - Golden crc lsb 8 bit (SW) */
};

struct zacc_desc {
	u32 data[64];
};

#pragma pack(pop)

struct zacc_dev {
	struct platform_device *pdev;

	struct list_head list;
	spinlock_t lock;

	struct zacc_desc *desc;
	dma_addr_t desc_pa;

	unsigned int head;
	unsigned int tail;
	size_t used;
	size_t size;

	/* DMA bounce buffers */
	struct page *dst_pg[ZACC_QUEUE_SIZE];
	struct page *src_pg[ZACC_QUEUE_SIZE];

//	struct page *tmp_dst_pg[ZACC_QUEUE_SIZE];
//	struct page *tmp_src_pg[ZACC_QUEUE_SIZE];
};

extern int zacc_decoder_ready(void);
extern int zacc_encoder_ready(void);

extern struct zacc_dev *__zacc_device(struct list_head *list, spinlock_t *lock);
extern unsigned int zacc_get_hw_resource(void);
extern void zacc_put_hw_resource(unsigned int index);
extern struct zacc_desc *zacc_get_desc(struct zacc_dev *zdev);
extern void zacc_put_desc(struct zacc_dev *zdev, struct zacc_desc *desc);
extern struct zacc_dev *__zacc_probe(struct platform_device *pdev);
extern void __zacc_remove(struct platform_device *pdev);
extern int zacc_suspend(struct device *dev);
extern int zacc_resume(struct device *dev);

extern struct dev_pm_ops const zacc_pm_ops;

#endif	/* __ZACC_H */
