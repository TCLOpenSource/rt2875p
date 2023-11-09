#ifndef _RTK_VDEC_SECURE_BUF_UPDATE_H_
#define _RTK_VDEC_SECURE_BUF_UPDATE_H_

#include <linux/file.h>

struct rtk_vdec_dmabuf_ringbuf_map {
	uint32_t buffer_addr;
	uint32_t buffer_size;
	uint32_t offset1;
	uint32_t length1;
	uint32_t offset2;
	uint32_t length2;
};

long rtk_vdec_dmabuf_ringbuf_start_update(
	void *headerAddr, unsigned long length, unsigned long offset,
	struct rtk_vdec_dmabuf_ringbuf_map *map, unsigned long baseaddr,
	unsigned long basesize);

long rtk_vdec_dmabuf_ringbuf_commit_update(void *headerAddr,
					   unsigned long length,
					   unsigned long baseaddr,
					   unsigned long basesize);

#endif