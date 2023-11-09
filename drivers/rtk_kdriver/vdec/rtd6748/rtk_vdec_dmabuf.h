#ifndef _RTK_VDEC_DMABUF_H_
#define _RTK_VDEC_DMABUF_H_

#include <rtk_kdriver/rtk_vdec.h>

#include "rtk_vdec_ringbuf_update.h"

/*
FLAG_PROTECTED | FLAG_CPB                           : secure bs buffer
FLAG_CPB                                            : clear bs buffer
FLAG_PROTECTED | FLAG_CPB | FLAG_RINGBUF_HEADER     : secure ring-buffer
*/
enum { FLAG_NONE = 0,
       FLAG_PROTECTED = 0x1,
       FLAG_CPB = 0x2,
       FLAG_RINGBUF_HEADER = 0x4,
       FLAG_IOCTL = 0x8,
};

struct rtk_vdec_dmabuf_allocation_data {
	uint32_t len;
	uint32_t flags;
	int handle;
};

struct rtk_vdec_dmabuf_info_data {
	uint32_t len;
	uint32_t flags;
	uint32_t phyaddr;
	int handle;
};

struct rtk_vdec_dmabuf_securebuf_update {
	uint32_t length;
	uint32_t offset;
	int handle;
	struct rtk_vdec_dmabuf_ringbuf_map map;
	uint32_t reserve[7];
};

#define VDEC_IOC_SVP_DMABUF_ALLOCATE _IOWR(VDEC_IOC_MAGIC, 0x10001, int)

#define VDEC_IOC_SVP_DMABUF_GET_INFO _IOWR(VDEC_IOC_MAGIC, 0x10002, int)

#define VDEC_IOC_SVP_DMABUF_START_RINGBUF_UPDATE                               \
	_IOWR(VDEC_IOC_MAGIC, 0x10003, int)

#define VDEC_IOC_SVP_DMABUF_COMMIT_RINGBUF_UPDATE                              \
	_IOWR(VDEC_IOC_MAGIC, 0x10004, int)

long rtk_vdec_dmabuf_ioctl(struct file *filp, unsigned int cmd,
			   unsigned long arg);

int rtk_vdec_dma_heap_create(void);

#endif /* _RTK_VDEC_DMABUF_H_ */
