#ifndef _RTK_VDEC_DEF_H_
#define _RTK_VDEC_DEF_H_

#ifdef CONFIG_ARM64
#define rtkvdec_memset(a,b,c) memset_io((void *)(a),(b),(c))
#define rtkvdec_memcpy(a,b,c) memcpy_toio((void *)(a),(b),(c))
#else
#define rtkvdec_memset(a,b,c) memset((void *)(a),(b),(c))
#define rtkvdec_memcpy(a,b,c) memcpy((void *)(a),(b),(c))
#endif

#define rtkvdec_align(size, unit)      (((size) + ((unit) - 1)) & ~((unit) - 1))

#define RTKVDEC_MAX_VMM_MEM_NUM 3

#define RTKVDEC_DUMMY_VBM_SIZE 0xB801A61C

#define IBBUF_DEBUG_SB2_ID 7
#define IBBUF_DEBUG_SB2_CNT 2

#define RTKVDEC_STECMD_TYPE_8KDEC 0x1

#define DMA_ARRAY_SIZE 512
#define DMA_REF_STATUS_USER_BIT 0x1
#define DMA_REF_STATUS_VDEC_BIT 0x2

#define FRAME_ARRAY_SIZE 20

#endif
