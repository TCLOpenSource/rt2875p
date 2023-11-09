#ifndef _RMM_HEAP_H
#define _RMM_HEAP_H

#include <linux/genalloc.h>
/*
 * RTK dma-buf heaps use heap flags to deliver information to dma heaps
 * derived from legacy kernel uapi dma-heap.h
 *
 * Should be complied with
 * 1. All flags should be valid in DMA_HEAP_VALID_HEAP_FLAGS of uapi/dma-heap.h
 * 2. FLAG_USER flags should be consistent with pageremap.c and derived from enum dma_heap_user_type of pageremap.h
 *
 */


/*
 * The last 5 bits are used to set frames number for frame based allocation,
 *   i.e., max frames number can be 31
 */

#define DMA_HEAP_FLAG_FRAMES_NUM_MASK           (0x0000001f)

#define DMA_HEAP_FLAG_CACHED                    0x20u

#define DMA_HEAP_FLAG_FROM_DCU1                 0x40u           /* memory allocated from DCU1 only */
#define DMA_HEAP_FLAG_FROM_DCU2                 0x80u           /* memory allocated from DCU2 only */

#define DMA_HEAP_FLAG_ALLOC_CHUNKS              0x100u          /* request memory by chunks */
#define DMA_HEAP_FLAG_ALLOC_FRAMES              0x200u          /* request memory by frames */

#define DMA_HEAP_FLAG_NEED_SECURE_PROTECT       0x400u          /* request memory is need to be secure protected */


#define DMA_HEAP_FLAG_USER_SHIFT                (12)
#define DMA_HEAP_FLAG_USER_MASK                 (0x0001f000)
#define DMA_HEAP_FLAG_USER_VIDEO_4K_LUMA        (1 << (DMA_HEAP_USER_VIDEO_4K_LUMA   + DMA_HEAP_FLAG_USER_SHIFT)) /* request user is for video 4K luma */
#define DMA_HEAP_FLAG_USER_VIDEO_4K_CHROMA      (1 << (DMA_HEAP_USER_VIDEO_4K_CHROMA + DMA_HEAP_FLAG_USER_SHIFT)) /* request user is for video 2K chroma */
#define DMA_HEAP_FLAG_USER_VIDEO_2K             (1 << (DMA_HEAP_USER_VIDEO_2K        + DMA_HEAP_FLAG_USER_SHIFT)) /* request user is for video 2K */
#define DMA_HEAP_FLAG_USER_COBUFFER             (1 << (DMA_HEAP_USER_COBUFFER        + DMA_HEAP_FLAG_USER_SHIFT)) /* request user is for VBM co-buffer */
#define DMA_HEAP_FLAG_USER_VIDEO_SUB_2K         (1 << (DMA_HEAP_USER_VIDEO_SUB_2K    + DMA_HEAP_FLAG_USER_SHIFT)) /* request user is for video sub 2K */

  // start from bit [18]  (PS. note! DMA_HEAP_USER_VIDEO_SUB_2K is [16]  )
#define DMA_HEAP_FLAG_FOR_VE                    (0x00040000u)  //bit [18]
#define DMA_HEAP_FLAG_FOR_FM                    (0x00080000u)
#define DMA_HEAP_FLAG_FOR_MULTI                 (0x00100000u)


//#define DMA_HEAP_VALID_HEAP_FLAGS               (0x000fffff) //ref: include/uapi/linux/dma-heap.h

int __gen_pool_growup(struct gen_pool *pool, unsigned long addr, size_t size, int nid);


#endif /* _RMM_HEAP_H */