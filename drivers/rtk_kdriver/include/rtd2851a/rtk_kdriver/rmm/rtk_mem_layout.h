
#ifndef _RTK_MEMORY_LAYOUT_H
#define _RTK_MEMORY_LAYOUT_H

/**
 *  --- Default memory layout start ---
 */

#if defined (CONFIG_IMG_DEMOD_CARVED_OUT_ADDRESS)
#define DEMOD_CARVED_OUT CONFIG_IMG_DEMOD_CARVED_OUT_ADDRESS
#else
#define DEMOD_CARVED_OUT 0x02000000  //compatible for previous released version.
#endif  //#if defined (CONFIG_IMG_DEMOD)


#define AUDIO_QS_BUFFER_START (0x14400000)
#define AUDIO_QS_BUFFER_SIZE  (16*_MB_)


#define VIDEO_RINGBUFFER_START         0x1b200000
#define VIDEO_RINGBUFFER_SIZE			0x1000000
#define VIDEO_RINGBUFFER_SIZE_K2L_ONLY 		 0x1800000
#define AUDIO_RINGBUFFER_START         (VIDEO_RINGBUFFER_START+VIDEO_RINGBUFFER_SIZE)
#define AUDIO_RINGBUFFER_SIZE                      0x00600000


#ifdef CONFIG_REALTEK_LOGBUF
#define CONST_LOGBUF_MEM_ADDR_START (0x1ca00000)
#define CONST_LOGBUF_MEM_SIZE (512*1024)
#endif

#ifdef CONFIG_CUSTOMER_TV006	//for GAL reserve mem, 4MB align
extern const int GAL_MEM_SIZE;
extern const int GAL_MEM_ADDR_START;
#else
#define GAL_MEM_SIZE 0
#define GAL_MEM_ADDR_START 0
#endif

#ifdef CONFIG_KASAN
#define VBM_START_ADDRESS_KERNEL       0x28000000  // + 8400000 (+ 132M)
#define MEMC_START_ADDRESS_KERNEL      0x30400000  // + 0000000 (+   0M)
#define MDOMAIN_START_ADDRESS_KERNEL   0x30400000  // + 1c00000 (+  28M)
#define DI_NR_START_ADDRESS_KERNEL     0x32000000  // + 0900000 (+   9M)
#define VIP_START_ADDRESS_KERNEL       0x32900000  // + 0200000 (+   2M)
#define OD_START_ADDRESS_KERNEL        0x32b00000  // +       0 (+   0M)
#define NN_START_ADDRESS_KERNEL        0x32b00000  // +       0 (+   0M)
#else
#define VBM_START_ADDRESS_KERNEL       0x20000000  // + 8400000 (+ 132M)
#define MEMC_START_ADDRESS_KERNEL      0x28400000  // + 0000000 (+   0M)
#define MDOMAIN_START_ADDRESS_KERNEL   0x28400000  // + 1c00000 (+  28M)
#define DI_NR_START_ADDRESS_KERNEL     0x2a000000  // + 0900000 (+   9M)
#define VIP_START_ADDRESS_KERNEL       0x2a900000  // + 0200000 (+   2M)
#define OD_START_ADDRESS_KERNEL        0x2ab00000  // +       0 (+   0M)
#define NN_START_ADDRESS_KERNEL        0x2ab00000  // +       0 (+   0M)
#endif

#define VDEC_BUFFER_START (VBM_START_ADDRESS_KERNEL)
#define VDEC_BUFFER_SIZE 132*_MB_    // 132 MB for single 4K 60% comp.

#ifdef VBM_START_ADDRESS_KERNEL
#if (VBM_START_ADDRESS_KERNEL != VDEC_BUFFER_START)
#error "VBM size doesn't match for kernel & drivers!"
#endif
#else
#error "No define VBM_START_ADDRESS_KERNEL"
#endif

#define SCALER_MEMC_START (VDEC_BUFFER_START + VDEC_BUFFER_SIZE)
#define SCALER_MEMC_SIZE (0*_MB_)  // No MEMC in Mac series

#if (MEMC_START_ADDRESS_KERNEL != SCALER_MEMC_START)
#error "Scaler M-domain size doesn't match for kernel & drivers!"
#endif

#define SCALER_MDOMAIN_START (SCALER_MEMC_START + SCALER_MEMC_SIZE)
#define SCALER_MDOMAIN_SIZE (28*_MB_) // 2K2K(2160x2160) x 12 bits x 4 frames

#if (MDOMAIN_START_ADDRESS_KERNEL != SCALER_MDOMAIN_START)
#error "Scaler M-domain size doesn't match for kernel & drivers!"
#endif

#define SCALER_DI_NR_START (SCALER_MDOMAIN_START + SCALER_MDOMAIN_SIZE)
#define SCALER_DI_NR_SIZE (9*_MB_) // 13MB

#if (DI_NR_START_ADDRESS_KERNEL != SCALER_DI_NR_START)
#error "Scaler DI_NR size doesn't match for kernel & drivers!"
#endif

#define SCALER_VIP_START (SCALER_DI_NR_START + SCALER_DI_NR_SIZE)
#define SCALER_VIP_SIZE  (2*_MB_)

//fixme: why can't enable this compile check in arm64?
#ifndef CONFIG_ARM64
#ifdef VIP_START_ADDRESS_KERNEL
#if (VIP_START_ADDRESS_KERNEL != SCALER_VIP_START)
#error "Scaler VIP size doesn't match for kernel & drivers!"
#endif
#else
#error "No define VIP_START_ADDRESS_KERNEL"
#endif
#endif

#define SCALER_OD_START (SCALER_VIP_START + SCALER_VIP_SIZE)
#define SCALER_OD_SIZE  0*_MB_

//fixme: why can't enable this compile check in arm64?
#ifndef CONFIG_ARM64
#ifdef OD_START_ADDRESS_KERNEL
#if (OD_START_ADDRESS_KERNEL != SCALER_OD_START)
#error "Scaler OD size doesn't match for kernel & drivers!"
#endif
#else
#error "No define OD_START_ADDRESS_KERNEL"
#endif

#endif

#define SCALER_NN_START (SCALER_OD_START + SCALER_OD_SIZE)
#define SCALER_NN_SIZE (0*_MB_) // 1MB

#if (NN_START_ADDRESS_KERNEL != SCALER_NN_START)
#error "Scaler NN size doesn't match for kernel & drivers!"
#endif


#define SCLAER_MODULE_START SCALER_MEMC_START
#define SCALER_MODULE_SIZE_QUERY (SCALER_MEMC_SIZE +    \
                                  SCALER_MDOMAIN_SIZE + \
                                  SCALER_DI_NR_SIZE +   \
                                  SCALER_NN_SIZE +      \
                                  SCALER_VIP_SIZE +     \
                                  SCALER_OD_SIZE)
#define SCALER_MODULE_SIZE DO_ALIGNMENT(SCALER_MODULE_SIZE_QUERY, 4*_MB_)
#define SCALER_MODULE_BAND (SCALER_MODULE_SIZE - SCALER_MODULE_SIZE_QUERY)

#if (SCALER_MODULE_SIZE & 0x3FFFFF) // 4MB align
#error "Scaler module size didn't follow 4MB alignment"
#endif

#define DEMUX_TP_BUFFER_START (SCLAER_MODULE_START + SCALER_MODULE_SIZE)
#ifdef CONFIG_RTK_KDRV_DEMUX
#define DEMUX_TP_BUFFER_SIZE 0  // must be 4MB page block alignment
#else
#define DEMUX_TP_BUFFER_SIZE 0
#endif

#ifdef CONFIG_KASAN
#define KASAN_SHADOW_PHYS_DEFAULT (DEMUX_TP_BUFFER_START + DEMUX_TP_BUFFER_SIZE)
#endif

#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
#define BW_HIGH_CMA_SIZE        (88*_MB_)	// Better BW CMA size in HighMem
#endif

/**
 *  --- Default memory layout end ---
 */


/**
 *  --- 1.5GB  memory layout start ---
 */
#define DDR_1G5_VIDEO_RINGBUFFER_START         0x1b200000
#define DDR_1G5_VIDEO_RINGBUFFER_SIZE			0x1000000
#define DDR_1G5_VIDEO_RINGBUFFER_SIZE_K2L_ONLY 		 0x1800000
#define DDR_1G5_AUDIO_RINGBUFFER_START         (VIDEO_RINGBUFFER_START+VIDEO_RINGBUFFER_SIZE)
#define DDR_1G5_AUDIO_RINGBUFFER_SIZE                      0x00600000


#ifdef CONFIG_REALTEK_LOGBUF
#define DDR_1G5_CONST_LOGBUF_MEM_ADDR_START (0x1ca00000)
#define DDR_1G5_CONST_LOGBUF_MEM_SIZE (512*1024)
#endif

#ifdef CONFIG_CUSTOMER_TV006	//for GAL reserve mem, 4MB align
extern const int DDR_1G5_GAL_MEM_SIZE;
extern const int DDR_1G5_GAL_MEM_ADDR_START;
#else
#define DDR_1G5_GAL_MEM_SIZE 0
#define DDR_1G5_GAL_MEM_ADDR_START 0
#endif

#ifdef CONFIG_KASAN
#define DDR_1G5_VBM_START_ADDRESS_KERNEL       0x28000000  // + 7000000 (+ 112M)
#define DDR_1G5_MEMC_START_ADDRESS_KERNEL      0x2f000000  // + 0000000 (+   0M)
#define DDR_1G5_MDOMAIN_START_ADDRESS_KERNEL   0x2f000000  // + 0d00000 (+  13M)
#define DDR_1G5_DI_NR_START_ADDRESS_KERNEL     0x2fd00000  // + 0900000 (+   9M)
#define DDR_1G5_VIP_START_ADDRESS_KERNEL       0x30600000  // + 0200000 (+   2M)
#define DDR_1G5_OD_START_ADDRESS_KERNEL        0x30800000  // +       0 (+   0M)
#define DDR_1G5_NN_START_ADDRESS_KERNEL        0x30800000  // +       0 (+   0M)
#else
#define DDR_1G5_VBM_START_ADDRESS_KERNEL       0x20000000  // + 7000000 (+ 112M)
#define DDR_1G5_MEMC_START_ADDRESS_KERNEL      0x27000000  // + 0000000 (+   0M)
#define DDR_1G5_MDOMAIN_START_ADDRESS_KERNEL   0x27000000  // + 1c00000 (+  28M)
#define DDR_1G5_DI_NR_START_ADDRESS_KERNEL     0x28c00000  // + 0900000 (+   9M)
#define DDR_1G5_VIP_START_ADDRESS_KERNEL       0x29500000  // + 0200000 (+   2M)
#define DDR_1G5_OD_START_ADDRESS_KERNEL        0x29700000  // +       0 (+   0M)
#define DDR_1G5_NN_START_ADDRESS_KERNEL        0x29700000  // +       0 (+   0M)
#endif

#define DDR_1G5_VDEC_BUFFER_START (VBM_START_ADDRESS_KERNEL)
#define DDR_1G5_VDEC_BUFFER_SIZE 112*_MB_    // 112 MB for single 4K 50% comp.

#ifdef DDR_1G5_VBM_START_ADDRESS_KERNEL
#if (DDR_1G5_VBM_START_ADDRESS_KERNEL != DDR_1G5_VDEC_BUFFER_START)
#error "1.5GB VBM size doesn't match for kernel & drivers!"
#endif
#else
#error "1.5GB No define VBM_START_ADDRESS_KERNEL"
#endif

#define DDR_1G5_SCALER_MEMC_START (DDR_1G5_VDEC_BUFFER_START + DDR_1G5_VDEC_BUFFER_SIZE)
#define DDR_1G5_SCALER_MEMC_SIZE (0*_MB_)  // No MEMC in Mac series

#if (DDR_1G5_MEMC_START_ADDRESS_KERNEL != DDR_1G5_SCALER_MEMC_START)
#error "1.5GB Scaler M-domain size doesn't match for kernel & drivers!"
#endif

#define DDR_1G5_SCALER_MDOMAIN_START (DDR_1G5_SCALER_MEMC_START + DDR_1G5_SCALER_MEMC_SIZE)
#define DDR_1G5_SCALER_MDOMAIN_SIZE (28*_MB_) // 2K2K(2160x2160) x 12 bits x 4 frames

#if (DDR_1G5_MDOMAIN_START_ADDRESS_KERNEL != DDR_1G5_SCALER_MDOMAIN_START)
#error "1.5GB Scaler M-domain size doesn't match for kernel & drivers!"
#endif

#define DDR_1G5_SCALER_DI_NR_START (DDR_1G5_SCALER_MDOMAIN_START + DDR_1G5_SCALER_MDOMAIN_SIZE)
#define DDR_1G5_SCALER_DI_NR_SIZE (9*_MB_) // disabling MASNR, and RTNR uses Y only mode

#if (DDR_1G5_DI_NR_START_ADDRESS_KERNEL != DDR_1G5_SCALER_DI_NR_START)
#error "1.5GB Scaler DI_NR size doesn't match for kernel & drivers!"
#endif

#define DDR_1G5_SCALER_VIP_START (DDR_1G5_SCALER_DI_NR_START + DDR_1G5_SCALER_DI_NR_SIZE)
#define DDR_1G5_SCALER_VIP_SIZE  (2*_MB_)

//fixme: why can't enable this compile check in arm64?
#ifndef CONFIG_ARM64
#ifdef DDR_1G5_VIP_START_ADDRESS_KERNEL
#if (DDR_1G5_VIP_START_ADDRESS_KERNEL != DDR_1G5_SCALER_VIP_START)
#error "1.5GB Scaler VIP size doesn't match for kernel & drivers!"
#endif
#else
#error "1.5GB No define VIP_START_ADDRESS_KERNEL"
#endif
#endif

#define DDR_1G5_SCALER_OD_START (DDR_1G5_SCALER_VIP_START + DDR_1G5_SCALER_VIP_SIZE)
#define DDR_1G5_SCALER_OD_SIZE  0*_MB_

//fixme: why can't enable this compile check in arm64?
#ifndef CONFIG_ARM64
#ifdef DDR_1G5_OD_START_ADDRESS_KERNEL
#if (DDR_1G5_OD_START_ADDRESS_KERNEL != DDR_1G5_SCALER_OD_START)
#error "1.5GB Scaler OD size doesn't match for kernel & drivers!"
#endif
#else
#error "1.5GB No define OD_START_ADDRESS_KERNEL"
#endif

#endif

#define DDR_1G5_SCALER_NN_START (DDR_1G5_SCALER_OD_START + DDR_1G5_SCALER_OD_SIZE)
#define DDR_1G5_SCALER_NN_SIZE (0*_MB_) // 1MB

#if (DDR_1G5_NN_START_ADDRESS_KERNEL != DDR_1G5_SCALER_NN_START)
#error "1.5GB Scaler NN size doesn't match for kernel & drivers!"
#endif

#define DDR_1G5_SCLAER_MODULE_START DDR_1G5_SCALER_MEMC_START
#define DDR_1G5_SCALER_MODULE_SIZE_QUERY (DDR_1G5_SCALER_MEMC_SIZE +    \
                                          DDR_1G5_SCALER_MDOMAIN_SIZE + \
                                          DDR_1G5_SCALER_DI_NR_SIZE +   \
                                          DDR_1G5_SCALER_NN_SIZE +      \
                                          DDR_1G5_SCALER_VIP_SIZE +     \
                                          DDR_1G5_SCALER_OD_SIZE)
#define DDR_1G5_SCALER_MODULE_SIZE DO_ALIGNMENT(DDR_1G5_SCALER_MODULE_SIZE_QUERY, 4*_MB_)
#define DDR_1G5_SCALER_MODULE_BAND (DDR_1G5_SCALER_MODULE_SIZE - DDR_1G5_SCALER_MODULE_SIZE_QUERY)

#if (DDR_1G5_SCALER_MODULE_SIZE & 0x3FFFFF) // 4MB align
#error "1.5GB Scaler module size didn't follow 4MB alignment"
#endif

#define DDR_1G5_DEMUX_TP_BUFFER_START (DDR_1G5_SCLAER_MODULE_START + DDR_1G5_SCALER_MODULE_SIZE)
#ifdef CONFIG_RTK_KDRV_DEMUX
#define DDR_1G5_DEMUX_TP_BUFFER_SIZE 0  // must be 4MB page block alignment
#else
#define DDR_1G5_DEMUX_TP_BUFFER_SIZE 0
#endif

#ifdef CONFIG_KASAN
#define DDR_1G5_KASAN_SHADOW_PHYS_DEFAULT (DEMUX_TP_BUFFER_START + DEMUX_TP_BUFFER_SIZE)
#endif

#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
#define DDR_1G5_BW_HIGH_CMA_SIZE        (88*_MB_)	// Better BW CMA size in HighMem
#endif

/**
 *  --- 1.5GB memory layout end ---
 */


/**
 *  --- 1GB memory layout start ---
 */
#define DDR_1GB_VIDEO_RINGBUFFER_START         0x1b200000
#define DDR_1GB_VIDEO_RINGBUFFER_SIZE			0x1000000
#define DDR_1GB_VIDEO_RINGBUFFER_SIZE_K2L_ONLY 		 0x1800000
#define DDR_1GB_AUDIO_RINGBUFFER_START         (DDR_1GB_VIDEO_RINGBUFFER_START+DDR_1GB_VIDEO_RINGBUFFER_SIZE)
#define DDR_1GB_AUDIO_RINGBUFFER_SIZE                      0x00600000

#ifdef CONFIG_REALTEK_LOGBUF
#define DDR_1GB_CONST_LOGBUF_MEM_ADDR_START (0x1ca00000)
#define DDR_1GB_CONST_LOGBUF_MEM_SIZE (512*1024)
#endif

#ifdef CONFIG_CUSTOMER_TV006	//for GAL reserve mem, 4MB align
extern const int DDR_1GB_GAL_MEM_SIZE;
extern const int DDR_1GB_GAL_MEM_ADDR_START;
#else
#define DDR_1GB_GAL_MEM_SIZE 0
#define DDR_1GB_GAL_MEM_ADDR_START 0
#endif

#define DDR_1GB_VBM_START_ADDRESS_KERNEL       0x20000000  // + 6800000 (+ 104M)
#define DDR_1GB_MEMC_START_ADDRESS_KERNEL      0x26800000  // + 0000000 (+   0M)
#define DDR_1GB_MDOMAIN_START_ADDRESS_KERNEL   0x26800000  // + 1c00000 (+  28M)
#define DDR_1GB_DI_NR_START_ADDRESS_KERNEL     0x28400000  // + 0900000 (+   9M)
#define DDR_1GB_VIP_START_ADDRESS_KERNEL       0x28d00000  // + 0100000 (+   1M)
#define DDR_1GB_OD_START_ADDRESS_KERNEL        0x28e00000  // + 0000000 (+   0M)
#define DDR_1GB_NN_START_ADDRESS_KERNEL        0x28e00000  // + 0000000 (+   0M)

#define DDR_1GB_VDEC_BUFFER_START (DDR_1GB_VBM_START_ADDRESS_KERNEL)
#define DDR_1GB_VDEC_BUFFER_SIZE 104*_MB_    // Support MiraCast and AirPlay only, worst case 4K H264 lossless 102 MB, +2 MB for 4 MB aligned

#ifdef DDR_1GB_VBM_START_ADDRESS_KERNEL
#if (DDR_1GB_VBM_START_ADDRESS_KERNEL != DDR_1GB_VDEC_BUFFER_START)
#error "1GB VBM size doesn't match for kernel & drivers!"
#endif
#else
#error "1GB No define VBM_START_ADDRESS_KERNEL"
#endif

#define DDR_1GB_SCALER_MEMC_START (DDR_1GB_VDEC_BUFFER_START + DDR_1GB_VDEC_BUFFER_SIZE)
#define DDR_1GB_SCALER_MEMC_SIZE (0*_MB_)

#if (DDR_1GB_MEMC_START_ADDRESS_KERNEL != DDR_1GB_SCALER_MEMC_START)
#error "1GB Scaler M-domain size doesn't match for kernel & drivers!"
#endif

#define DDR_1GB_SCALER_MDOMAIN_START (DDR_1GB_SCALER_MEMC_START + DDR_1GB_SCALER_MEMC_SIZE)
#define DDR_1GB_SCALER_MDOMAIN_SIZE (28*_MB_) // 2K2K(2160x2160) x 12 bits x 4 frames

#if (DDR_1GB_MDOMAIN_START_ADDRESS_KERNEL != DDR_1GB_SCALER_MDOMAIN_START)
#error "1GB Scaler M-domain size doesn't match for kernel & drivers!"
#endif

#define DDR_1GB_SCALER_DI_NR_START (DDR_1GB_SCALER_MDOMAIN_START + DDR_1GB_SCALER_MDOMAIN_SIZE)
#define DDR_1GB_SCALER_DI_NR_SIZE (9*_MB_) // disabling MASNR, and RTNR uses Y only mode

#if (DDR_1GB_DI_NR_START_ADDRESS_KERNEL != DDR_1GB_SCALER_DI_NR_START)
#error "1GB Scaler DI_NR size doesn't match for kernel & drivers!"
#endif

#define DDR_1GB_SCALER_VIP_START (DDR_1GB_SCALER_DI_NR_START + DDR_1GB_SCALER_DI_NR_SIZE)
#define DDR_1GB_SCALER_VIP_SIZE  (1*_MB_) // No need demura

//fixme: why can't enable this compile check in arm64?
#ifndef CONFIG_ARM64
#ifdef DDR_1GB_VIP_START_ADDRESS_KERNEL
#if (DDR_1GB_VIP_START_ADDRESS_KERNEL != DDR_1GB_SCALER_VIP_START)
#error "1GB Scaler VIP size doesn't match for kernel & drivers!"
#endif
#else
#error "No define DDR_1GB_VIP_START_ADDRESS_KERNEL"
#endif
#endif

#define DDR_1GB_SCALER_OD_START (DDR_1GB_SCALER_VIP_START + DDR_1GB_SCALER_VIP_SIZE)
#define DDR_1GB_SCALER_OD_SIZE  0*_MB_

//fixme: why can't enable this compile check in arm64?
#ifndef CONFIG_ARM64
#ifdef DDR_1GB_OD_START_ADDRESS_KERNEL
#if (DDR_1GB_OD_START_ADDRESS_KERNEL != DDR_1GB_SCALER_OD_START)
#error "1GB Scaler OD size doesn't match for kernel & drivers!"
#endif
#else
#error "1GB No define OD_START_ADDRESS_KERNEL"
#endif

#endif

#define DDR_1GB_SCALER_NN_START (DDR_1GB_SCALER_OD_START + DDR_1GB_SCALER_OD_SIZE)
#define DDR_1GB_SCALER_NN_SIZE (0*_MB_) //

#if (DDR_1GB_NN_START_ADDRESS_KERNEL != DDR_1GB_SCALER_NN_START)
#error "1GB Scaler NN size doesn't match for kernel & drivers!"
#endif

#define DDR_1GB_SCLAER_MODULE_START DDR_1GB_SCALER_MEMC_START
#define DDR_1GB_SCALER_MODULE_SIZE_QUERY (DDR_1GB_SCALER_MEMC_SIZE + DDR_1GB_SCALER_MDOMAIN_SIZE + DDR_1GB_SCALER_DI_NR_SIZE + DDR_1GB_SCALER_NN_SIZE + DDR_1GB_SCALER_VIP_SIZE + DDR_1GB_SCALER_OD_SIZE)
#define DDR_1GB_SCALER_MODULE_SIZE DO_ALIGNMENT(DDR_1GB_SCALER_MODULE_SIZE_QUERY, 4*_MB_)
#define DDR_1GB_SCALER_MODULE_BAND (DDR_1GB_SCALER_MODULE_SIZE - DDR_1GB_SCALER_MODULE_SIZE_QUERY)

#if (DDR_1GB_SCALER_MODULE_SIZE & 0x3FFFFF) // 4MB align
#error "1GB Scaler module size didn't follow 4MB alignment"
#endif

#define DDR_1GB_DEMUX_TP_BUFFER_START (DDR_1GB_SCLAER_MODULE_START + DDR_1GB_SCALER_MODULE_SIZE)
#ifdef CONFIG_RTK_KDRV_DEMUX
#define DDR_1GB_DEMUX_TP_BUFFER_SIZE 0 //32*_MB_  // must be 4MB page block alignment
#else
#define DDR_1GB_DEMUX_TP_BUFFER_SIZE 0
#endif

#ifdef CONFIG_KASAN
#define DDR_1GB_KASAN_SHADOW_PHYS (DDR_1GB_DEMUX_TP_BUFFER_START + DDR_1GB_DEMUX_TP_BUFFER_SIZE)
#endif

#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
#define DDR_1GB_BW_HIGH_CMA_SIZE        (0*_MB_)	// Better BW CMA size in HighMem
#endif

/**
 *  --- 1GB memory layout end ---
 */


/**
 *  --- 2K_MODEL memory layout start ---
 */
#define DDR_2K_MODEL_VIDEO_RINGBUFFER_START         0x1b200000
#define DDR_2K_MODEL_VIDEO_RINGBUFFER_SIZE			0x800000
#define DDR_2K_MODEL_VIDEO_RINGBUFFER_SIZE_K2L_ONLY 		 0x1800000
#define DDR_2K_MODEL_AUDIO_RINGBUFFER_START         (DDR_2K_MODEL_VIDEO_RINGBUFFER_START+DDR_2K_MODEL_VIDEO_RINGBUFFER_SIZE)
#define DDR_2K_MODEL_AUDIO_RINGBUFFER_SIZE                      0x00600000

#ifdef CONFIG_REALTEK_LOGBUF
#define DDR_2K_MODEL_CONST_LOGBUF_MEM_ADDR_START (0x1ca00000)
#define DDR_2K_MODEL_CONST_LOGBUF_MEM_SIZE (512*1024)
#endif

#ifdef CONFIG_CUSTOMER_TV006	//for GAL reserve mem, 4MB align
extern const int DDR_2K_MODEL_GAL_MEM_SIZE;
extern const int DDR_2K_MODEL_GAL_MEM_ADDR_START;
#else
#define DDR_2K_MODEL_GAL_MEM_SIZE 0
#define DDR_2K_MODEL_GAL_MEM_ADDR_START 0
#endif

#define DDR_2K_MODEL_VBM_START_ADDRESS_KERNEL       0x20000000  // + 2400000 (+  36M)
#define DDR_2K_MODEL_MEMC_START_ADDRESS_KERNEL      0x22400000  // + 0000000 (+   0M)
#define DDR_2K_MODEL_MDOMAIN_START_ADDRESS_KERNEL   0x22400000  // + 0000000 (+   0M)
#define DDR_2K_MODEL_DI_NR_START_ADDRESS_KERNEL     0x22400000  // + 0800000 (+   8M)
#define DDR_2K_MODEL_VIP_START_ADDRESS_KERNEL       0x22c00000  // + 0200000 (+   2M)
#define DDR_2K_MODEL_OD_START_ADDRESS_KERNEL        0x22e00000  // + 0000000 (+   0M)
#define DDR_2K_MODEL_NN_START_ADDRESS_KERNEL        0x22e00000  // + 0000000 (+   0M)

#define DDR_2K_MODEL_VDEC_BUFFER_START (DDR_2K_MODEL_VBM_START_ADDRESS_KERNEL)
#define DDR_2K_MODEL_VDEC_BUFFER_SIZE 36*_MB_    // 50% lossy compression 23.5MB + 8.5MB, cmprs hdr 4MB (single decoding, 2nd decoding uses CMA)

#ifdef DDR_2K_MODEL_VBM_START_ADDRESS_KERNEL
#if (DDR_2K_MODEL_VBM_START_ADDRESS_KERNEL != DDR_2K_MODEL_VDEC_BUFFER_START)
#error "2K_MODEL VBM size doesn't match for kernel & drivers!"
#endif
#else
#error "2K_MODEL No define VBM_START_ADDRESS_KERNEL"
#endif

#define DDR_2K_MODEL_SCALER_MEMC_START (DDR_2K_MODEL_VDEC_BUFFER_START + DDR_2K_MODEL_VDEC_BUFFER_SIZE)
#define DDR_2K_MODEL_SCALER_MEMC_SIZE (0*_MB_)

#if (DDR_2K_MODEL_MEMC_START_ADDRESS_KERNEL != DDR_2K_MODEL_SCALER_MEMC_START)
#error "2K_MODEL Scaler M-domain size doesn't match for kernel & drivers!"
#endif

#define DDR_2K_MODEL_SCALER_MDOMAIN_START (DDR_2K_MODEL_SCALER_MEMC_START + DDR_2K_MODEL_SCALER_MEMC_SIZE)
#define DDR_2K_MODEL_SCALER_MDOMAIN_SIZE (0*_MB_) // 0MB(M-Domain)

#if (DDR_2K_MODEL_MDOMAIN_START_ADDRESS_KERNEL != DDR_2K_MODEL_SCALER_MDOMAIN_START)
#error "2K_MODEL Scaler M-domain size doesn't match for kernel & drivers!"
#endif

#define DDR_2K_MODEL_SCALER_DI_NR_START (DDR_2K_MODEL_SCALER_MDOMAIN_START + DDR_2K_MODEL_SCALER_MDOMAIN_SIZE)
#define DDR_2K_MODEL_SCALER_DI_NR_SIZE (8*_MB_) // 8MB(DI/RTNR)

#if (DDR_2K_MODEL_DI_NR_START_ADDRESS_KERNEL != DDR_2K_MODEL_SCALER_DI_NR_START)
#error "2K_MODEL Scaler DI_NR size doesn't match for kernel & drivers!"
#endif

#define DDR_2K_MODEL_SCALER_VIP_START (DDR_2K_MODEL_SCALER_DI_NR_START + DDR_2K_MODEL_SCALER_DI_NR_SIZE)
#define DDR_2K_MODEL_SCALER_VIP_SIZE  (2*_MB_)

//fixme: why can't enable this compile check in arm64?
#ifndef CONFIG_ARM64
#ifdef DDR_2K_MODEL_VIP_START_ADDRESS_KERNEL
#if (DDR_2K_MODEL_VIP_START_ADDRESS_KERNEL != DDR_2K_MODEL_SCALER_VIP_START)
#error "2K_MODEL Scaler VIP size doesn't match for kernel & drivers!"
#endif
#else
#error "No define DDR_2K_MODEL_VIP_START_ADDRESS_KERNEL"
#endif
#endif

#define DDR_2K_MODEL_SCALER_OD_START (DDR_2K_MODEL_SCALER_VIP_START + DDR_2K_MODEL_SCALER_VIP_SIZE)
#define DDR_2K_MODEL_SCALER_OD_SIZE  0*_MB_

//fixme: why can't enable this compile check in arm64?
#ifndef CONFIG_ARM64
#ifdef DDR_2K_MODEL_OD_START_ADDRESS_KERNEL
#if (DDR_2K_MODEL_OD_START_ADDRESS_KERNEL != DDR_2K_MODEL_SCALER_OD_START)
#error "2K_MODEL Scaler OD size doesn't match for kernel & drivers!"
#endif
#else
#error "2K_MODEL No define OD_START_ADDRESS_KERNEL"
#endif

#endif

#define DDR_2K_MODEL_SCALER_NN_START (DDR_2K_MODEL_SCALER_OD_START + DDR_2K_MODEL_SCALER_OD_SIZE)
#define DDR_2K_MODEL_SCALER_NN_SIZE (0*_MB_) //

#if (DDR_2K_MODEL_NN_START_ADDRESS_KERNEL != DDR_2K_MODEL_SCALER_NN_START)
#error "2K_MODEL Scaler NN size doesn't match for kernel & drivers!"
#endif

#define DDR_2K_MODEL_SCLAER_MODULE_START DDR_2K_MODEL_SCALER_MEMC_START
#define DDR_2K_MODEL_SCALER_MODULE_SIZE_QUERY (DDR_2K_MODEL_SCALER_MEMC_SIZE +    \
                                               DDR_2K_MODEL_SCALER_MDOMAIN_SIZE + \
                                               DDR_2K_MODEL_SCALER_DI_NR_SIZE +   \
                                               DDR_2K_MODEL_SCALER_NN_SIZE +      \
                                               DDR_2K_MODEL_SCALER_VIP_SIZE +     \
                                               DDR_2K_MODEL_SCALER_OD_SIZE)
#define DDR_2K_MODEL_SCALER_MODULE_SIZE DO_ALIGNMENT(DDR_2K_MODEL_SCALER_MODULE_SIZE_QUERY, 4*_MB_)
#define DDR_2K_MODEL_SCALER_MODULE_BAND (DDR_2K_MODEL_SCALER_MODULE_SIZE - DDR_2K_MODEL_SCALER_MODULE_SIZE_QUERY)

#if (DDR_2K_MODEL_SCALER_MODULE_SIZE & 0x3FFFFF) // 4MB align
#error "2K_MODEL Scaler module size didn't follow 4MB alignment"
#endif

#define DDR_2K_MODEL_DEMUX_TP_BUFFER_START (DDR_2K_MODEL_SCLAER_MODULE_START + DDR_2K_MODEL_SCALER_MODULE_SIZE)
#ifdef CONFIG_RTK_KDRV_DEMUX
#define DDR_2K_MODEL_DEMUX_TP_BUFFER_SIZE 0 //32*_MB_  // must be 4MB page block alignment
#else
#define DDR_2K_MODEL_DEMUX_TP_BUFFER_SIZE 0
#endif

#ifdef CONFIG_KASAN
#define DDR_2K_MODEL_KASAN_SHADOW_PHYS (DDR_2K_MODEL_DEMUX_TP_BUFFER_START + DDR_2K_MODEL_DEMUX_TP_BUFFER_SIZE)
#endif

#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
#define DDR_2K_MODEL_BW_HIGH_CMA_SIZE        (0*_MB_)	// Better BW CMA size in HighMem
#endif

/**
 *  --- 2K_MODEL memory layout end ---
 */


#define RTK_GPU_FW_SIZE (512*1024)

#define CMA_HIGHMEM_SMALL 160*1024*1024
#define CMA_HIGHMEM_MEDIAN 368*1024*1024
#define CMA_HIGHMEM_LARGE 880*1024*1024

#define CMA_GPU_4K_SIZE 0

#ifdef RTK_MEM_LAYOUT_DEVICETREE
    // test if not used
#else
#define ZRAM_RESERVED_SIZE 128*_MB_

// TODO: shall be get from dts
#define RTK_SPI_ADDR    0x80200000
#define RTK_SPI_SIZE    0x02000000
#endif

#endif //_RTK_MEMORY_LAYOUT_H
