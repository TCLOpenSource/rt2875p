
#ifndef _RTK_MEMORY_LAYOUT_H
#define _RTK_MEMORY_LAYOUT_H

/**
 *  --- TV 4K memory layout start ---
 */

#if defined (CONFIG_IMG_DEMOD_CARVED_OUT_ADDRESS)
#define DEMOD_CARVED_OUT CONFIG_IMG_DEMOD_CARVED_OUT_ADDRESS  // 0x15400000
#else
#define DEMOD_CARVED_OUT 0x02000000  //compatible for previous released version.
#endif  //#if defined (CONFIG_IMG_DEMOD)


#define AUDIO_BUFFER_START (0x14400000)
#define AUDIO_BUFFER_SIZE  (16*_MB_)


#define VIDEO_RINGBUFFER_START         0x3f000000
#define VIDEO_RINGBUFFER_SIZE			0x1000000
//#define VIDEO_RINGBUFFER_SIZE_K2L_ONLY 		 0x1800000
//#define AUDIO_RINGBUFFER_START         (VIDEO_RINGBUFFER_START+VIDEO_RINGBUFFER_SIZE)
//#define AUDIO_RINGBUFFER_SIZE                      0 //0x00600000


#if IS_ENABLED(CONFIG_REALTEK_LOGBUF)
#define CONST_LOGBUF_MEM_ADDR_START (0x1ca00000)
#define CONST_LOGBUF_MEM_SIZE (512*1024)
#else
#define CONST_LOGBUF_MEM_ADDR_START 0
#define CONST_LOGBUF_MEM_SIZE 0
#endif

#ifdef CONFIG_CUSTOMER_TV006	//for GAL reserve mem, 4MB align
extern const int GAL_MEM_SIZE;
extern const int GAL_MEM_ADDR_START;
#else
#define GAL_MEM_SIZE 0
#define GAL_MEM_ADDR_START 0
#endif

#define VBM_START_ADDRESS_KERNEL       0x20000000  // + e000000 (+ 224M)
#define MEMC_START_ADDRESS_KERNEL      0x2e000000  // + 5c00000 (+  92M)
#define MDOMAIN_START_ADDRESS_KERNEL   0x33c00000  // + 1c00000 (+  28M)
#define DI_NR_START_ADDRESS_KERNEL     0x35800000  // + 0a00000 (+  10M)
#define VIP_START_ADDRESS_KERNEL       0x36200000  // + 0300000 (+   3M)
#define OD_START_ADDRESS_KERNEL        0x36500000  // + 0500000 (+   5M)
#define NN_START_ADDRESS_KERNEL        0x36a00000  // + 0100000 (+   1M)

#define VDEC_BUFFER_START (VBM_START_ADDRESS_KERNEL)
#define VDEC_BUFFER_SIZE 224*_MB_    // 4K lossy 60% luma 50% chroma, VVC/AVS3 luma not compressed, support dual decode 2K+4K+decimate

#ifdef VBM_START_ADDRESS_KERNEL
#if (VBM_START_ADDRESS_KERNEL != VDEC_BUFFER_START)
#error "VBM size doesn't match for kernel & drivers!"
#endif
#else
#error "No define VBM_START_ADDRESS_KERNEL"
#endif

#define SCALER_MEMC_START (VDEC_BUFFER_START + VDEC_BUFFER_SIZE)
#define SCALER_MEMC_SIZE (92*_MB_)  // No MEMC in Mac series

#if (MEMC_START_ADDRESS_KERNEL != SCALER_MEMC_START)
#error "Scaler M-domain size doesn't match for kernel & drivers!"
#endif

#define SCALER_MDOMAIN_START (SCALER_MEMC_START + SCALER_MEMC_SIZE)
#define SCALER_MDOMAIN_SIZE (28*_MB_) // 2K2K(2160x2160) x 12 bits x 4 frames

#if (MDOMAIN_START_ADDRESS_KERNEL != SCALER_MDOMAIN_START)
#error "Scaler M-domain size doesn't match for kernel & drivers!"
#endif

#define SCALER_DI_NR_START (SCALER_MDOMAIN_START + SCALER_MDOMAIN_SIZE)
#define SCALER_DI_NR_SIZE (10*_MB_) // 4K rolling mode w/o MASNR

#if (DI_NR_START_ADDRESS_KERNEL != SCALER_DI_NR_START)
#error "Scaler DI_NR size doesn't match for kernel & drivers!"
#endif

#define SCALER_VIP_START (SCALER_DI_NR_START + SCALER_DI_NR_SIZE)
#define SCALER_VIP_SIZE  (3*_MB_)

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
#define SCALER_OD_SIZE  5*_MB_

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
#define SCALER_NN_SIZE (1*_MB_)

#if (NN_START_ADDRESS_KERNEL != SCALER_NN_START)
#error "Scaler NN size doesn't match for kernel & drivers!"
#endif

#define SCLAER_MODULE_START SCALER_MEMC_START
#define SCALER_MODULE_SIZE_QUERY (SCALER_MEMC_SIZE + SCALER_MDOMAIN_SIZE + SCALER_DI_NR_SIZE + SCALER_VIP_SIZE + SCALER_OD_SIZE + SCALER_NN_SIZE)
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

#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
#define BW_HIGH_CMA_SIZE        (88*_MB_)	// Better BW CMA size in HighMem
#endif

/**
 *  --- TV 4K memory layout end ---
 */


/**
 *  --- DIAS 2GB  memory layout start ---
 */
#define DDR_DIAS_2G_VIDEO_RINGBUFFER_START         0x3f000000
#define DDR_DIAS_2G_VIDEO_RINGBUFFER_SIZE			0x1000000
//#define DDR_DIAS_2G_VIDEO_RINGBUFFER_SIZE_K2L_ONLY 		 0x1800000
//#define DDR_DIAS_2G_AUDIO_RINGBUFFER_START         (VIDEO_RINGBUFFER_START+VIDEO_RINGBUFFER_SIZE)
//#define DDR_DIAS_2G_AUDIO_RINGBUFFER_SIZE                      0 //0x00600000


#if IS_ENABLED(CONFIG_REALTEK_LOGBUF)
#define DDR_DIAS_2G_CONST_LOGBUF_MEM_ADDR_START (0x1ca00000)
#define DDR_DIAS_2G_CONST_LOGBUF_MEM_SIZE (512*1024)
#else
#define DDR_DIAS_2G_CONST_LOGBUF_MEM_ADDR_START 0
#define DDR_DIAS_2G_CONST_LOGBUF_MEM_SIZE 0
#endif

#ifdef CONFIG_CUSTOMER_TV006	//for GAL reserve mem, 4MB align
extern const int DDR_DIAS_2G_GAL_MEM_SIZE;
extern const int DDR_DIAS_2G_GAL_MEM_ADDR_START;
#else
#define DDR_DIAS_2G_GAL_MEM_SIZE 0
#define DDR_DIAS_2G_GAL_MEM_ADDR_START 0
#endif

#define DDR_DIAS_2G_VBM_START_ADDRESS_KERNEL       0x20000000  // + d000000 (+ 208M)
#define DDR_DIAS_2G_MEMC_START_ADDRESS_KERNEL      0x2d000000  // + 5400000 (+  84M)
#define DDR_DIAS_2G_MDOMAIN_START_ADDRESS_KERNEL   0x32400000  // + 1c00000 (+  28M)
#define DDR_DIAS_2G_DI_NR_START_ADDRESS_KERNEL     0x34000000  // + 0a00000 (+  10M)
#define DDR_DIAS_2G_VIP_START_ADDRESS_KERNEL       0x34a00000  // + 0300000 (+   3M)
#define DDR_DIAS_2G_OD_START_ADDRESS_KERNEL        0x34d00000  // + 0500000 (+   5M)
#define DDR_DIAS_2G_NN_START_ADDRESS_KERNEL        0x35200000  // + 0100000 (+   1M)

#define DDR_DIAS_2G_VDEC_BUFFER_START (DDR_DIAS_2G_VBM_START_ADDRESS_KERNEL)
#define DDR_DIAS_2G_VDEC_BUFFER_SIZE 208*_MB_    // 4K lossy 60% luma 50% chroma w/o VVC and AVS3, need support CP, support dual decode 2K+4K+decimate

#ifdef DDR_DIAS_2G_VBM_START_ADDRESS_KERNEL
#if (DDR_DIAS_2G_VBM_START_ADDRESS_KERNEL != DDR_DIAS_2G_VDEC_BUFFER_START)
#error "DIAS 2GB VBM size doesn't match for kernel & drivers!"
#endif
#else
#error "DIAS 2GB No define VBM_START_ADDRESS_KERNEL"
#endif

#define DDR_DIAS_2G_SCALER_MEMC_START (DDR_DIAS_2G_VDEC_BUFFER_START + DDR_DIAS_2G_VDEC_BUFFER_SIZE)
#define DDR_DIAS_2G_SCALER_MEMC_SIZE (84*_MB_)  // MC only mode, YUV422, frame mode, PQC 9.5, buffer 8 

#if (DDR_DIAS_2G_MEMC_START_ADDRESS_KERNEL != DDR_DIAS_2G_SCALER_MEMC_START)
#error "DIAS 2GB Scaler M-domain size doesn't match for kernel & drivers!"
#endif

#define DDR_DIAS_2G_SCALER_MDOMAIN_START (DDR_DIAS_2G_SCALER_MEMC_START + DDR_DIAS_2G_SCALER_MEMC_SIZE)
#define DDR_DIAS_2G_SCALER_MDOMAIN_SIZE (28*_MB_) // 2K2K(2160x2160) x 12 bits x 4 frames

#if (DDR_DIAS_2G_MDOMAIN_START_ADDRESS_KERNEL != DDR_DIAS_2G_SCALER_MDOMAIN_START)
#error "DIAS 2GB Scaler M-domain size doesn't match for kernel & drivers!"
#endif

#define DDR_DIAS_2G_SCALER_DI_NR_START (DDR_DIAS_2G_SCALER_MDOMAIN_START + DDR_DIAS_2G_SCALER_MDOMAIN_SIZE)
#define DDR_DIAS_2G_SCALER_DI_NR_SIZE (10*_MB_) // 4K rolling mode w/o MASNR

#if (DDR_DIAS_2G_DI_NR_START_ADDRESS_KERNEL != DDR_DIAS_2G_SCALER_DI_NR_START)
#error "DIAS 2GB Scaler DI_NR size doesn't match for kernel & drivers!"
#endif

#define DDR_DIAS_2G_SCALER_VIP_START (DDR_DIAS_2G_SCALER_DI_NR_START + DDR_DIAS_2G_SCALER_DI_NR_SIZE)
#define DDR_DIAS_2G_SCALER_VIP_SIZE  (3*_MB_) // no demura

//fixme: why can't enable this compile check in arm64?
#ifndef CONFIG_ARM64
#ifdef DDR_DIAS_2G_VIP_START_ADDRESS_KERNEL
#if (DDR_DIAS_2G_VIP_START_ADDRESS_KERNEL != DDR_DIAS_2G_SCALER_VIP_START)
#error "DIAS 2GB Scaler VIP size doesn't match for kernel & drivers!"
#endif
#else
#error "DIAS 2GB No define VIP_START_ADDRESS_KERNEL"
#endif
#endif

#define DDR_DIAS_2G_SCALER_OD_START (DDR_DIAS_2G_SCALER_VIP_START + DDR_DIAS_2G_SCALER_VIP_SIZE)
#define DDR_DIAS_2G_SCALER_OD_SIZE  5*_MB_

//fixme: why can't enable this compile check in arm64?
#ifndef CONFIG_ARM64
#ifdef DDR_DIAS_2G_OD_START_ADDRESS_KERNEL
#if (DDR_DIAS_2G_OD_START_ADDRESS_KERNEL != DDR_DIAS_2G_SCALER_OD_START)
#error "DIAS 2GB Scaler OD size doesn't match for kernel & drivers!"
#endif
#else
#error "DIAS 2GB No define OD_START_ADDRESS_KERNEL"
#endif
#endif

#define DDR_DIAS_2G_SCALER_NN_START (DDR_DIAS_2G_SCALER_OD_START + DDR_DIAS_2G_SCALER_OD_SIZE)
#define DDR_DIAS_2G_SCALER_NN_SIZE (1*_MB_)

#if (DDR_DIAS_2G_NN_START_ADDRESS_KERNEL != DDR_DIAS_2G_SCALER_NN_START)
#error "DIAS 2GB Scaler NN size doesn't match for kernel & drivers!"
#endif

#define DDR_DIAS_2G_SCLAER_MODULE_START DDR_DIAS_2G_SCALER_MEMC_START
#define DDR_DIAS_2G_SCALER_MODULE_SIZE_QUERY (DDR_DIAS_2G_SCALER_MEMC_SIZE + DDR_DIAS_2G_SCALER_MDOMAIN_SIZE + DDR_DIAS_2G_SCALER_DI_NR_SIZE + DDR_DIAS_2G_SCALER_VIP_SIZE + DDR_DIAS_2G_SCALER_OD_SIZE + DDR_DIAS_2G_SCALER_NN_SIZE)
#define DDR_DIAS_2G_SCALER_MODULE_SIZE DO_ALIGNMENT(DDR_DIAS_2G_SCALER_MODULE_SIZE_QUERY, 4*_MB_)
#define DDR_DIAS_2G_SCALER_MODULE_BAND (DDR_DIAS_2G_SCALER_MODULE_SIZE - DDR_DIAS_2G_SCALER_MODULE_SIZE_QUERY)

#if (DDR_DIAS_2G_SCALER_MODULE_SIZE & 0x3FFFFF) // 4MB align
#error "DIAS 2GB Scaler module size didn't follow 4MB alignment"
#endif

#define DDR_DIAS_2G_DEMUX_TP_BUFFER_START (DDR_DIAS_2G_SCLAER_MODULE_START + DDR_DIAS_2G_SCALER_MODULE_SIZE)
#ifdef CONFIG_RTK_KDRV_DEMUX
#define DDR_DIAS_2G_DEMUX_TP_BUFFER_SIZE 0  // must be 4MB page block alignment
#else
#define DDR_DIAS_2G_DEMUX_TP_BUFFER_SIZE 0
#endif

#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
#define DDR_DIAS_2G_BW_HIGH_CMA_SIZE        (88*_MB_)	// Better BW CMA size in HighMem
#endif

/**
 *  --- DIAS DIAS 2GB memory layout end ---
 */


#define RTK_GPU_FW_SIZE 0 //(512*1024)  not use now

#define CMA_HIGHMEM_DIAS_2GB 684*1024*1024
#define CMA_HIGHMEM_TV_4K 692*1024*1024

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
