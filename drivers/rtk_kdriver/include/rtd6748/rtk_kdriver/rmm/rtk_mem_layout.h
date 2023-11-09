
#ifndef _RTK_MEMORY_LAYOUT_H
#define _RTK_MEMORY_LAYOUT_H

#include <linux/version.h>

/**
 *  --- Default memory layout start ---
 */

#if defined (CONFIG_IMG_DEMOD_CARVED_OUT_ADDRESS)
#define DEMOD_CARVED_OUT CONFIG_IMG_DEMOD_CARVED_OUT_ADDRESS
#else
#define DEMOD_CARVED_OUT 0x02000000  //compatible for previous released version.
#endif  //#if defined (CONFIG_IMG_DEMOD)

#define AUDIO_BUFFER_START             (0x14400000)
#if IS_ENABLED(CONFIG_RTK_KDRV_QUICKSHOW) //FIXME can be always on after audio supporting reserved-memory
#define AUDIO_BUFFER_SIZE              (16*_MB_)
#else
#define AUDIO_BUFFER_SIZE              (0 *_MB_)
#endif


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

#define CPB_START_ADDRESS_KERNEL       0x3ec00000  //

#define VBM_START_ADDRESS_KERNEL       0x20800000  // + c800000 (+ 200M)
#define MEMC_START_ADDRESS_KERNEL      0x2d000000  // + 4800000 (+  72M)
#define MDOMAIN_START_ADDRESS_KERNEL   0x31800000  // + 0e00000 (+  14M)
#define DI_NR_START_ADDRESS_KERNEL     0x32600000  // + 0e00000 (+  14M)
#define VIP_START_ADDRESS_KERNEL       0x33400000  // + 0400000 (+   4M)
#define OD_START_ADDRESS_KERNEL        0x33800000  // + 0a00000 (+  10M)
#define NN_START_ADDRESS_KERNEL        0x34200000  // + 0200000 (+   2M)
#define TP_START_ADDRESS_KERNEL        0x34400000  //

#define SVP_CPB_START (CPB_START_ADDRESS_KERNEL)
#define SVP_CPB_SIZE (16*_MB_)

#if (CPB_START_ADDRESS_KERNEL != SVP_CPB_START)
#error "SVP_CPB size doesn't match for kernel & drivers!"
#endif

#define CMA_LOW_LIMIT_START      (0x03000000)
#define CMA_LOW_LIMIT_SIZE       24*_MB_              //
#if (CMA_LOW_LIMIT_START & 0x3FFFFF) || (CMA_LOW_LIMIT_SIZE & 0x3FFFFF)
#error "CMA_LOW_LIMIT no follow 4MB alignment"
#endif

#if 0
#define CMA_LOW_START      (0x20000000)
#define CMA_LOW_SIZE        104*_MB_              //
#if (CMA_LOW_START & 0x3FFFFF) || (CMA_LOW_SIZE & 0x3FFFFF)
#error "CMA_LOW no follow 4MB alignment"
#endif
#endif

#define VDEC_BUFFER_START (VBM_START_ADDRESS_KERNEL)
#define VDEC_BUFFER_SIZE  200*_MB_    // 

#if (VBM_START_ADDRESS_KERNEL != VDEC_BUFFER_START)
#error "VBM size doesn't match for kernel & drivers!"
#endif

#define SCALER_MEMC_START (VDEC_BUFFER_START + VDEC_BUFFER_SIZE)
#define SCALER_MEMC_SIZE (72*_MB_)

#if (MEMC_START_ADDRESS_KERNEL != SCALER_MEMC_START)
#error "Scaler M-domain size doesn't match for kernel & drivers!"
#endif

#define SCALER_MDOMAIN_START (SCALER_MEMC_START + SCALER_MEMC_SIZE)
#define SCALER_MDOMAIN_SIZE (14*_MB_) // 13MB for 2K FRC, 37MB if 4K FRC is needed, +1MB for 3DDRs 2GB

#if (MDOMAIN_START_ADDRESS_KERNEL != SCALER_MDOMAIN_START)
#error "Scaler M-domain size doesn't match for kernel & drivers!"
#endif

#define SCALER_DI_NR_START (SCALER_MDOMAIN_START + SCALER_MDOMAIN_SIZE)
#define SCALER_DI_NR_SIZE (14*_MB_) // 13MB, +1MB for 3DDRs 2GB

#if (DI_NR_START_ADDRESS_KERNEL != SCALER_DI_NR_START)
#error "Scaler DI_NR size doesn't match for kernel & drivers!"
#endif

#define SCALER_VIP_START (SCALER_DI_NR_START + SCALER_DI_NR_SIZE)
#define SCALER_VIP_SIZE  ((3+1)*_MB_)	// DMA(dmato3d table) 800 KB + Demura 5 layer/481*271/6bit/RGB mode 1.5 MB = 2.3 MB

#if (VIP_START_ADDRESS_KERNEL != SCALER_VIP_START)
#error "Scaler VIP size doesn't match for kernel & drivers!"
#endif

#define SCALER_OD_START (SCALER_VIP_START + SCALER_VIP_SIZE)
#define SCALER_OD_SIZE  ((9+1)*_MB_)	// RGB mode

#if (OD_START_ADDRESS_KERNEL != SCALER_OD_START)
#error "Scaler OD size doesn't match for kernel & drivers!"
#endif

#define SCALER_NN_START (SCALER_OD_START + SCALER_OD_SIZE)
#define SCALER_NN_SIZE ((1+1)*_MB_) // 1MB

#if (NN_START_ADDRESS_KERNEL != SCALER_NN_START)
#error "Scaler NN size doesn't match for kernel & drivers!"
#endif

#define SCLAER_MODULE_START SCALER_MEMC_START
#define SCALER_MODULE_SIZE_QUERY (SCALER_MEMC_SIZE + 	\
								  SCALER_MDOMAIN_SIZE + \
								  SCALER_DI_NR_SIZE + 	\
								  SCALER_NN_SIZE + 		\
								  SCALER_VIP_SIZE +	 	\
								  SCALER_OD_SIZE)
#define SCALER_MODULE_SIZE DO_ALIGNMENT(SCALER_MODULE_SIZE_QUERY, 4*_MB_)
#define SCALER_MODULE_BAND (SCALER_MODULE_SIZE - SCALER_MODULE_SIZE_QUERY)

#if (SCALER_MODULE_SIZE & 0x3FFFFF) // 4MB align
#error "Scaler module size didn't follow 4MB alignment"
#endif

#define DEMUX_TP_BUFFER_START (SCLAER_MODULE_START + SCALER_MODULE_SIZE)
#if 1//def CONFIG_RTK_KDRV_DEMUX
#define DEMUX_TP_BUFFER_SIZE 0*_MB_  // must be 4MB page block alignment
#else
#define DEMUX_TP_BUFFER_SIZE 0
#endif

#if (TP_START_ADDRESS_KERNEL != DEMUX_TP_BUFFER_START)
#error "TP buffer size doesn't match for kernel & drivers!"
#endif

/**
 *  --- Default memory layout end ---
 */


/**
 *  --- 1.5GB memory layout start ---
 */
#if IS_ENABLED(CONFIG_REALTEK_LOGBUF)
#define DDR_1G5_CONST_LOGBUF_MEM_ADDR_START (0x1ca00000)
#define DDR_1G5_CONST_LOGBUF_MEM_SIZE (512*1024)
#else
#define DDR_1G5_CONST_LOGBUF_MEM_ADDR_START 0
#define DDR_1G5_CONST_LOGBUF_MEM_SIZE 0
#endif

#ifdef CONFIG_CUSTOMER_TV006	//for GAL reserve mem, 4MB align
extern const int DDR_1G5_GAL_MEM_SIZE;
extern const int DDR_1G5_GAL_MEM_ADDR_START;
#else
#define DDR_1G5_GAL_MEM_SIZE 0
#define DDR_1G5_GAL_MEM_ADDR_START 0
#endif

#define DDR_1G5_VBM_START_ADDRESS_KERNEL       0x20000000  // + c800000 (+ 200M)
#define DDR_1G5_MEMC_START_ADDRESS_KERNEL      0x2c800000  // + 4700000 (+  71M)
#define DDR_1G5_MDOMAIN_START_ADDRESS_KERNEL   0x30f00000  // + 0d00000 (+  13M)
#define DDR_1G5_DI_NR_START_ADDRESS_KERNEL     0x31c00000  // + 0d00000 (+  13M)
#define DDR_1G5_VIP_START_ADDRESS_KERNEL       0x32900000  // + 0200000 (+   2M) -200K
#define DDR_1G5_OD_START_ADDRESS_KERNEL        0x32ace000  // + 0400000 (+   4M) +200K
#define DDR_1G5_NN_START_ADDRESS_KERNEL        0x32f00000  // + 0100000 (+   1M)
#define DDR_1G5_TP_START_ADDRESS_KERNEL        0x33000000  //

#define DDR_1G5_CPB_START_ADDRESS_KERNEL       0x3ec00000  // 

#define DDR_1G5_SVP_CPB_START (DDR_1G5_CPB_START_ADDRESS_KERNEL)
#define DDR_1G5_SVP_CPB_SIZE (16*_MB_)

#if (DDR_1G5_CPB_START_ADDRESS_KERNEL != DDR_1G5_SVP_CPB_START)
#error "1.5GB SVP_CPB size doesn't match for kernel & drivers!"
#endif

#define DDR_1G5_CMA_LOW_START      (0x03800000)
#define DDR_1G5_CMA_LOW_SIZE        128*_MB_              //
#if (DDR_1G5_CMA_LOW_START & 0x3FFFFF) || (DDR_1G5_CMA_LOW_SIZE & 0x3FFFFF)
#error "1G5 CMA_LOW no follow 4MB alignment"
#endif

#define DDR_1G5_VDEC_BUFFER_START (DDR_1G5_VBM_START_ADDRESS_KERNEL)
#define DDR_1G5_VDEC_BUFFER_SIZE  200*_MB_    // 

#if (DDR_1G5_VBM_START_ADDRESS_KERNEL != DDR_1G5_VDEC_BUFFER_START)
#error "1.5GB VBM size doesn't match for kernel & drivers!"
#endif

#define DDR_1G5_SCALER_MEMC_START (DDR_1G5_VDEC_BUFFER_START + DDR_1G5_VDEC_BUFFER_SIZE)
#define DDR_1G5_SCALER_MEMC_SIZE (71*_MB_)

#if (DDR_1G5_MEMC_START_ADDRESS_KERNEL != DDR_1G5_SCALER_MEMC_START)
#error "1.5GB MEMC size doesn't match for kernel & drivers!"
#endif

#define DDR_1G5_SCALER_MDOMAIN_START (DDR_1G5_SCALER_MEMC_START + DDR_1G5_SCALER_MEMC_SIZE)
#define DDR_1G5_SCALER_MDOMAIN_SIZE (13*_MB_) // 13MB for 2K FRC, 37MB if 4K FRC is needed

#if (DDR_1G5_MDOMAIN_START_ADDRESS_KERNEL != DDR_1G5_SCALER_MDOMAIN_START)
#error "1.5GB Scaler M-domain size doesn't match for kernel & drivers!"
#endif

#define DDR_1G5_SCALER_DI_NR_START (DDR_1G5_SCALER_MDOMAIN_START + DDR_1G5_SCALER_MDOMAIN_SIZE)
#define DDR_1G5_SCALER_DI_NR_SIZE (13*_MB_) // 13MB

#if (DDR_1G5_DI_NR_START_ADDRESS_KERNEL != DDR_1G5_SCALER_DI_NR_START)
#error "1.5GB Scaler DI_NR size doesn't match for kernel & drivers!"
#endif

#define DDR_1G5_SCALER_VIP_START (DDR_1G5_SCALER_DI_NR_START + DDR_1G5_SCALER_DI_NR_SIZE)
#define DDR_1G5_SCALER_VIP_SIZE  (1848*_KB_)

#if (DDR_1G5_VIP_START_ADDRESS_KERNEL != DDR_1G5_SCALER_VIP_START)
#error "1.5GB Scaler VIP size doesn't match for kernel & drivers!"
#endif

#define DDR_1G5_SCALER_OD_START (DDR_1G5_SCALER_VIP_START + DDR_1G5_SCALER_VIP_SIZE)
#define DDR_1G5_SCALER_OD_SIZE  (4296*_KB_)

//fixme: why can't enable this compile check in arm64?
#if (DDR_1G5_OD_START_ADDRESS_KERNEL != DDR_1G5_SCALER_OD_START)
#error "1.5GB Scaler OD size doesn't match for kernel & drivers!"
#endif

#define DDR_1G5_SCALER_NN_START (DDR_1G5_SCALER_OD_START + DDR_1G5_SCALER_OD_SIZE)
#define DDR_1G5_SCALER_NN_SIZE (1*_MB_) // 1MB

#if (DDR_1G5_NN_START_ADDRESS_KERNEL != DDR_1G5_SCALER_NN_START)
#error "1.5GB Scaler NN size doesn't match for kernel & drivers!"
#endif

#define DDR_1G5_SCLAER_MODULE_START DDR_1G5_SCALER_MEMC_START
#define DDR_1G5_SCALER_MODULE_SIZE_QUERY (DDR_1G5_SCALER_MEMC_SIZE +			\
										  DDR_1G5_SCALER_MDOMAIN_SIZE + 		\
										  DDR_1G5_SCALER_DI_NR_SIZE + 			\
										  DDR_1G5_SCALER_NN_SIZE + 				\
										  DDR_1G5_SCALER_VIP_SIZE + 			\
										  DDR_1G5_SCALER_OD_SIZE)
#define DDR_1G5_SCALER_MODULE_SIZE DO_ALIGNMENT(DDR_1G5_SCALER_MODULE_SIZE_QUERY, 4*_MB_)
#define DDR_1G5_SCALER_MODULE_BAND (DDR_1G5_SCALER_MODULE_SIZE - DDR_1G5_SCALER_MODULE_SIZE_QUERY)

#if (DDR_1G5_SCALER_MODULE_SIZE & 0x3FFFFF) // 4MB align
#error "1.5GB Scaler module size didn't follow 4MB alignment"
#endif

#define DDR_1G5_DEMUX_TP_BUFFER_START (DDR_1G5_SCLAER_MODULE_START + DDR_1G5_SCALER_MODULE_SIZE)
#if 1//def CONFIG_RTK_KDRV_DEMUX
#define DDR_1G5_DEMUX_TP_BUFFER_SIZE 24*_MB_  // must be 4MB page block alignment
#else
#define DDR_1G5_DEMUX_TP_BUFFER_SIZE 0
#endif

#if (DDR_1G5_TP_START_ADDRESS_KERNEL != DDR_1G5_DEMUX_TP_BUFFER_START)
#error "1.5GB TP buffer size doesn't match for kernel & drivers!"
#endif

/**
 *  --- 1.5GB memory layout end ---
 */

/**
 *  --- 1GB memory layout start ---
 */
#if IS_ENABLED(CONFIG_REALTEK_LOGBUF)
#define DDR_1GB_CONST_LOGBUF_MEM_ADDR_START (0x1ca00000)
#define DDR_1GB_CONST_LOGBUF_MEM_SIZE (512*1024)
#else
#define DDR_1GB_CONST_LOGBUF_MEM_ADDR_START 0
#define DDR_1GB_CONST_LOGBUF_MEM_SIZE 0
#endif

#ifdef CONFIG_CUSTOMER_TV006	//for GAL reserve mem, 4MB align
extern const int DDR_1GB_GAL_MEM_SIZE;
extern const int DDR_1GB_GAL_MEM_ADDR_START;
#else
#define DDR_1GB_GAL_MEM_SIZE 0
#define DDR_1GB_GAL_MEM_ADDR_START 0
#endif

#define DDR_1GB_VBM_START_ADDRESS_KERNEL       0x20000000  // + 6000000 (+  96M)
#define DDR_1GB_MEMC_START_ADDRESS_KERNEL      0x26000000  // + 0000000 (+   0M)
#define DDR_1GB_MDOMAIN_START_ADDRESS_KERNEL   0x26000000  // + 0d00000 (+  13M)
#define DDR_1GB_DI_NR_START_ADDRESS_KERNEL     0x26d00000  // + 0d00000 (+  13M)
#define DDR_1GB_VIP_START_ADDRESS_KERNEL       0x27a00000  // + 0200000 (+   2M) -200K
#define DDR_1GB_OD_START_ADDRESS_KERNEL        0x27bce000  // + 0400000 (+   4M) +200K
#define DDR_1GB_NN_START_ADDRESS_KERNEL        0x28000000  // + 0000000 (+   0M)
#define DDR_1GB_TP_START_ADDRESS_KERNEL        0x28000000

#define DDR_1GB_CPB_START_ADDRESS_KERNEL       0x3ec00000  // 

#define DDR_1GB_SVP_CPB_START (DDR_1GB_CPB_START_ADDRESS_KERNEL)
#define DDR_1GB_SVP_CPB_SIZE (16*_MB_)

#if (DDR_1GB_CPB_START_ADDRESS_KERNEL != DDR_1GB_SVP_CPB_START)
#error "1.0GB SVP_CPB size doesn't match for kernel & drivers!"
#endif

#define DDR_1GB_CMA_LOW_LIMIT_START      (0x03000000)
#define DDR_1GB_CMA_LOW_LIMIT_SIZE        0*_MB_              //
#if (DDR_1GB_CMA_LOW_LIMIT_START & 0x3FFFFF) || (DDR_1GB_CMA_LOW_LIMIT_SIZE & 0x3FFFFF)
#error "1GB CMA_LOW_LIMIT no follow 4MB alignment"
#endif

#if 0
#define DDR_1GB_CMA_LOW_START      (0x03000000)
#define DDR_1GB_CMA_LOW_SIZE         128*_MB_              //
#if (DDR_1GB_CMA_LOW_START & 0x3FFFFF) || (DDR_1GB_CMA_LOW_SIZE & 0x3FFFFF)
#error "1GB CMA_LOW no follow 4MB alignment"
#endif
#endif

#define DDR_1GB_VDEC_BUFFER_START (DDR_1GB_VBM_START_ADDRESS_KERNEL)
#define DDR_1GB_VDEC_BUFFER_SIZE  96*_MB_    // 

#ifdef DDR_1GB_VBM_START_ADDRESS_KERNEL
#if (DDR_1GB_VBM_START_ADDRESS_KERNEL != DDR_1GB_VDEC_BUFFER_START)
#error "1.0GB VBM size doesn't match for kernel & drivers!"
#endif
#else
#error "1GB No define VBM_START_ADDRESS_KERNEL"
#endif

#define DDR_1GB_SCALER_MEMC_START (DDR_1GB_VDEC_BUFFER_START + DDR_1GB_VDEC_BUFFER_SIZE)
#define DDR_1GB_SCALER_MEMC_SIZE (0*_MB_)

#if (DDR_1GB_MEMC_START_ADDRESS_KERNEL != DDR_1GB_SCALER_MEMC_START)
#error "1.0GB Scaler M-domain size doesn't match for kernel & drivers!"
#endif

#define DDR_1GB_SCALER_MDOMAIN_START (DDR_1GB_SCALER_MEMC_START + DDR_1GB_SCALER_MEMC_SIZE)
#define DDR_1GB_SCALER_MDOMAIN_SIZE (13*_MB_) // 13MB for 2K FRC, 37MB if 4K FRC is needed

#if (DDR_1GB_MDOMAIN_START_ADDRESS_KERNEL != DDR_1GB_SCALER_MDOMAIN_START)
#error "1.0GB Scaler M-domain size doesn't match for kernel & drivers!"
#endif

#define DDR_1GB_SCALER_DI_NR_START (DDR_1GB_SCALER_MDOMAIN_START + DDR_1GB_SCALER_MDOMAIN_SIZE)
#define DDR_1GB_SCALER_DI_NR_SIZE (13*_MB_) // 13MB(DI/RTNR)

#if (DDR_1GB_DI_NR_START_ADDRESS_KERNEL != DDR_1GB_SCALER_DI_NR_START)
#error "1.0GB Scaler DI_NR size doesn't match for kernel & drivers!"
#endif

#define DDR_1GB_SCALER_VIP_START (DDR_1GB_SCALER_DI_NR_START + DDR_1GB_SCALER_DI_NR_SIZE)
#define DDR_1GB_SCALER_VIP_SIZE  (1848*_KB_)

#if (DDR_1GB_VIP_START_ADDRESS_KERNEL != DDR_1GB_SCALER_VIP_START)
#error "1.0GB Scaler VIP size doesn't match for kernel & drivers!"
#endif

#define DDR_1GB_SCALER_OD_START (DDR_1GB_SCALER_VIP_START + DDR_1GB_SCALER_VIP_SIZE)
#define DDR_1GB_SCALER_OD_SIZE  (4296*_KB_)

#if (DDR_1GB_OD_START_ADDRESS_KERNEL != DDR_1GB_SCALER_OD_START)
#error "1.0GB Scaler OD size doesn't match for kernel & drivers!"
#endif

#define DDR_1GB_SCALER_NN_START (DDR_1GB_SCALER_OD_START + DDR_1GB_SCALER_OD_SIZE)
#define DDR_1GB_SCALER_NN_SIZE (0*_MB_) //

#if (DDR_1GB_NN_START_ADDRESS_KERNEL != DDR_1GB_SCALER_NN_START)
#error "1.0GB Scaler NN size doesn't match for kernel & drivers!"
#endif

#define DDR_1GB_SCLAER_MODULE_START DDR_1GB_SCALER_MEMC_START
#define DDR_1GB_SCALER_MODULE_SIZE_QUERY (DDR_1GB_SCALER_MEMC_SIZE + 	\
										  DDR_1GB_SCALER_MDOMAIN_SIZE + \
										  DDR_1GB_SCALER_DI_NR_SIZE + 	\
										  DDR_1GB_SCALER_NN_SIZE + 		\
										  DDR_1GB_SCALER_VIP_SIZE + 	\
										  DDR_1GB_SCALER_OD_SIZE)
#define DDR_1GB_SCALER_MODULE_SIZE DO_ALIGNMENT(DDR_1GB_SCALER_MODULE_SIZE_QUERY, 4*_MB_)
#define DDR_1GB_SCALER_MODULE_BAND (DDR_1GB_SCALER_MODULE_SIZE - DDR_1GB_SCALER_MODULE_SIZE_QUERY)

#if (DDR_1GB_SCALER_MODULE_SIZE & 0x3FFFFF) // 4MB align
#error "1.0GB Scaler module size didn't follow 4MB alignment"
#endif

#define DDR_1GB_DEMUX_TP_BUFFER_START (DDR_1GB_SCLAER_MODULE_START + DDR_1GB_SCALER_MODULE_SIZE)
#ifdef CONFIG_RTK_KDRV_DEMUX
#define DDR_1GB_DEMUX_TP_BUFFER_SIZE 8*_MB_ //32*_MB_  // must be 4MB page block alignment
#else
#define DDR_1GB_DEMUX_TP_BUFFER_SIZE 0
#endif

#if (DDR_1GB_TP_START_ADDRESS_KERNEL != DDR_1GB_DEMUX_TP_BUFFER_START)
#error "1GB TP buffer size doesn't match for kernel & drivers!"
#endif

/**
 *  --- 1GB memory layout end ---
 */

/**
 *  --- DISP_8K memory layout start ---
 */
#if IS_ENABLED(CONFIG_REALTEK_LOGBUF)
#define DISP_8K_CONST_LOGBUF_MEM_ADDR_START (0x1ca00000)
#define DISP_8K_CONST_LOGBUF_MEM_SIZE (512*1024)
#else
#define DISP_8K_CONST_LOGBUF_MEM_ADDR_START 0
#define DISP_8K_CONST_LOGBUF_MEM_SIZE 0
#endif

#ifdef CONFIG_CUSTOMER_TV006	//for GAL reserve mem, 4MB align
extern const int DISP_8K_GAL_MEM_SIZE;
extern const int DISP_8K_GAL_MEM_ADDR_START;
#else
#define DISP_8K_GAL_MEM_SIZE 0
#define DISP_8K_GAL_MEM_ADDR_START 0
#endif

#define DISP_8K_VBM_START_ADDRESS_KERNEL       0x20000000  // + 25400000 (+ 596 M)
#define DISP_8K_MEMC_START_ADDRESS_KERNEL      0x45400000  // + 15100000 (+ 337 M)
#define DISP_8K_MDOMAIN_START_ADDRESS_KERNEL   0x5a500000  // + 14300000 (+ 323 M)
#define DISP_8K_DI_NR_START_ADDRESS_KERNEL     0x6e800000  // +  3400000 (+  52 M)
#define DISP_8K_VIP_START_ADDRESS_KERNEL       0x71c00000  // +  0800000 (+   8 M)
#define DISP_8K_OD_START_ADDRESS_KERNEL        0x72400000  // +  1100000 (+  17 M)
#define DISP_8K_NN_START_ADDRESS_KERNEL        0x73500000  // +  0400000 (+   4 M)
#define DISP_8K_TP_START_ADDRESS_KERNEL        0x73c00000  // +  1800000 (+  24 M)

#define DISP_8K_CPB_START_ADDRESS_KERNEL       0x75000000  //

#define DISP_8K_SVP_CPB_START (DISP_8K_CPB_START_ADDRESS_KERNEL)
#define DISP_8K_SVP_CPB_SIZE (32*_MB_)

#if (DISP_8K_CPB_START_ADDRESS_KERNEL != DISP_8K_SVP_CPB_START)
#error "DISP_8K SVP_CPB size doesn't match for kernel & drivers!"
#endif

#define DISP_8K_PCIE_START_ADDRESS_KERNEL       0x78000000  // PCIE bus memory address should be 64MB aligned

#define DISP_8K_PCIE_START (DISP_8K_PCIE_START_ADDRESS_KERNEL)
#define DISP_8K_PCIE_SIZE (4*_MB_)  //4Mbytes only for CMA, 60M for device
#define DISP_8K_PCIE_DMA_SIZE	(64*_MB_)	//reserve 64M for PCIE always

#if (DISP_8K_PCIE_START_ADDRESS_KERNEL != DISP_8K_PCIE_START)
#error "DISP_8K PCIE size doesn't match for kernel & drivers!"
#endif

#define DISP_8K_CMA_LOW_START      (0x03800000)
#define DISP_8K_CMA_LOW_SIZE        128*_MB_              //
#if (DISP_8K_CMA_LOW_START & 0x3FFFFF) || (DISP_8K_CMA_LOW_SIZE & 0x3FFFFF)
#error "DISP_8K CMA_LOW no follow 4MB alignment"
#endif

#define DISP_8K_VDEC_BUFFER_START (DISP_8K_VBM_START_ADDRESS_KERNEL)
//#define DISP_8K_VDEC_BUFFER_SIZE 560*_MB_    // 8K decode 536 MB + 21 MB for compression header, and need 4MB aligned
#define DISP_8K_VDEC_BUFFER_SIZE 596*_MB_    // 8K decode (chroma/luma all 60%, 13 frame 498 MB + more 2 occupied frames 37.75x2 + 21 MB for compression header, and need 4MB aligned

#if (DISP_8K_VBM_START_ADDRESS_KERNEL != DISP_8K_VDEC_BUFFER_START)
#error "DISP_8K VBM size doesn't match for kernel & drivers!"
#endif

#define DISP_8K_SCALER_MEMC_START (DISP_8K_VDEC_BUFFER_START + DISP_8K_VDEC_BUFFER_SIZE)
#define DISP_8K_SCALER_MEMC_SIZE (337*_MB_)

#if (DISP_8K_MEMC_START_ADDRESS_KERNEL != DISP_8K_SCALER_MEMC_START)
#error "DISP_8K MEMC size doesn't match for kernel & drivers!"
#endif

#define DISP_8K_SCALER_MDOMAIN_START (DISP_8K_SCALER_MEMC_START + DISP_8K_SCALER_MEMC_SIZE)
#define DISP_8K_SCALER_MDOMAIN_SIZE (323*_MB_) // For M-Doamin, SUB, I3DDMA co-buffer

#if (DISP_8K_MDOMAIN_START_ADDRESS_KERNEL != DISP_8K_SCALER_MDOMAIN_START)
#error "DISP_8K Scaler M-domain size doesn't match for kernel & drivers!"
#endif

#define DISP_8K_SCALER_DI_NR_START (DISP_8K_SCALER_MDOMAIN_START + DISP_8K_SCALER_MDOMAIN_SIZE)
#define DISP_8K_SCALER_DI_NR_SIZE (52*_MB_)

#if (DISP_8K_DI_NR_START_ADDRESS_KERNEL != DISP_8K_SCALER_DI_NR_START)
#error "DISP_8K Scaler DI_NR size doesn't match for kernel & drivers!"
#endif

#define DISP_8K_SCALER_VIP_START (DISP_8K_SCALER_DI_NR_START + DISP_8K_SCALER_DI_NR_SIZE)
#define DISP_8K_SCALER_VIP_SIZE  (8*_MB_)

#if (DISP_8K_VIP_START_ADDRESS_KERNEL != DISP_8K_SCALER_VIP_START)
#error "DISP_8K Scaler VIP size doesn't match for kernel & drivers!"
#endif

#define DISP_8K_SCALER_OD_START (DISP_8K_SCALER_VIP_START + DISP_8K_SCALER_VIP_SIZE)
#define DISP_8K_SCALER_OD_SIZE  17*_MB_

//fixme: why can't enable this compile check in arm64?
#if (DISP_8K_OD_START_ADDRESS_KERNEL != DISP_8K_SCALER_OD_START)
#error "DISP_8K Scaler OD size doesn't match for kernel & drivers!"
#endif

#define DISP_8K_SCALER_NN_START (DISP_8K_SCALER_OD_START + DISP_8K_SCALER_OD_SIZE)
#define DISP_8K_SCALER_NN_SIZE (4*_MB_) // temp setting

#if (DISP_8K_NN_START_ADDRESS_KERNEL != DISP_8K_SCALER_NN_START)
#error "DISP_8K Scaler NN size doesn't match for kernel & drivers!"
#endif

#define DISP_8K_SCLAER_MODULE_START DISP_8K_SCALER_MEMC_START
#define DISP_8K_SCALER_MODULE_SIZE_QUERY (DISP_8K_SCALER_MEMC_SIZE +			\
										  DISP_8K_SCALER_MDOMAIN_SIZE + 		\
										  DISP_8K_SCALER_DI_NR_SIZE + 			\
										  DISP_8K_SCALER_NN_SIZE + 				\
										  DISP_8K_SCALER_VIP_SIZE + 			\
										  DISP_8K_SCALER_OD_SIZE)
#define DISP_8K_SCALER_MODULE_SIZE DO_ALIGNMENT(DISP_8K_SCALER_MODULE_SIZE_QUERY, 4*_MB_)
#define DISP_8K_SCALER_MODULE_BAND (DISP_8K_SCALER_MODULE_SIZE - DISP_8K_SCALER_MODULE_SIZE_QUERY)

#if (DISP_8K_SCALER_MODULE_SIZE & 0x3FFFFF) // 4MB align
#error "DISP_8K Scaler module size didn't follow 4MB alignment"
#endif

#define DISP_8K_DEMUX_TP_BUFFER_START (DISP_8K_SCLAER_MODULE_START + DISP_8K_SCALER_MODULE_SIZE)
#if 1//def CONFIG_RTK_KDRV_DEMUX
#define DISP_8K_DEMUX_TP_BUFFER_SIZE 24*_MB_  // must be 4MB page block alignment
#else
#define DISP_8K_DEMUX_TP_BUFFER_SIZE 0
#endif

#if (DISP_8K_TP_START_ADDRESS_KERNEL != DISP_8K_DEMUX_TP_BUFFER_START)
#error "DISP_8K TP buffer size doesn't match for kernel & drivers!"
#endif

/**
 *  --- DISP_8K memory layout end ---
 */


/**
 *  --- DISP_4K memory layout start ---
 */
#if IS_ENABLED(CONFIG_REALTEK_LOGBUF)
#define DISP_4K_CONST_LOGBUF_MEM_ADDR_START (0x1ca00000)
#define DISP_4K_CONST_LOGBUF_MEM_SIZE (512*1024)
#else
#define DISP_4K_CONST_LOGBUF_MEM_ADDR_START 0
#define DISP_4K_CONST_LOGBUF_MEM_SIZE 0
#endif

#ifdef CONFIG_CUSTOMER_TV006	//for GAL reserve mem, 4MB align
extern const int DISP_4K_GAL_MEM_SIZE;
extern const int DISP_4K_GAL_MEM_ADDR_START;
#else
#define DISP_4K_GAL_MEM_SIZE 0
#define DISP_4K_GAL_MEM_ADDR_START 0
#endif

#define DISP_4K_VBM_START_ADDRESS_KERNEL       0x20000000  // + 26800000 (+ 616 M)
#define DISP_4K_MEMC_START_ADDRESS_KERNEL      0x46800000  // +  5700000 (+  87 M)
#define DISP_4K_MDOMAIN_START_ADDRESS_KERNEL   0x4bf00000  // +  0d00000 (+  13 M)
#define DISP_4K_DI_NR_START_ADDRESS_KERNEL     0x4cc00000  // +  0d00000 (+  13 M)
#define DISP_4K_VIP_START_ADDRESS_KERNEL       0x4d900000  // +  0200000 (+   2 M) -200K
#define DISP_4K_OD_START_ADDRESS_KERNEL        0x4dace000  // +  0400000 (+   4 M) +200K
#define DISP_4K_NN_START_ADDRESS_KERNEL        0x4df00000  // +  0100000 (+   1 M)
#define DISP_4K_TP_START_ADDRESS_KERNEL        0x4e000000  // +  1800000 (+  24 M)

#define DISP_4K_CPB_START_ADDRESS_KERNEL       0x75000000  //

#define DISP_4K_SVP_CPB_START (DISP_4K_CPB_START_ADDRESS_KERNEL)
#define DISP_4K_SVP_CPB_SIZE (32*_MB_)

#if (DISP_4K_CPB_START_ADDRESS_KERNEL != DISP_4K_SVP_CPB_START)
#error "DISP_4K SVP_CPB size doesn't match for kernel & drivers!"
#endif

#define DISP_4K_PCIE_START_ADDRESS_KERNEL       0x78000000  // PCIE bus memory address should be 64MB aligned

#define DISP_4K_PCIE_START (DISP_8K_PCIE_START_ADDRESS_KERNEL)
#define DISP_4K_PCIE_SIZE (64*_MB_)

#if (DISP_4K_PCIE_START_ADDRESS_KERNEL != DISP_4K_PCIE_START)
#error "DISP_4K PCIE size doesn't match for kernel & drivers!"
#endif

#define DISP_4K_CMA_LOW_START      (0x03800000)
#define DISP_4K_CMA_LOW_SIZE        128*_MB_              //
#if (DISP_4K_CMA_LOW_START & 0x3FFFFF) || (DISP_4K_CMA_LOW_SIZE & 0x3FFFFF)
#error "DISP_4K CMA_LOW no follow 4MB alignment"
#endif

#define DISP_4K_VDEC_BUFFER_START (DISP_4K_VBM_START_ADDRESS_KERNEL)
#define DISP_4K_VDEC_BUFFER_SIZE 616*_MB_    // 8K decode 402 MB + 4K decimate 192 MB + 21 MB for compression header, and need 4MB aligned

#if (DISP_4K_VBM_START_ADDRESS_KERNEL != DISP_4K_VDEC_BUFFER_START)
#error "DISP_4K VBM size doesn't match for kernel & drivers!"
#endif

#define DISP_4K_SCALER_MEMC_START (DISP_4K_VDEC_BUFFER_START + DISP_4K_VDEC_BUFFER_SIZE)
#define DISP_4K_SCALER_MEMC_SIZE (87*_MB_)

#if (DISP_4K_MEMC_START_ADDRESS_KERNEL != DISP_4K_SCALER_MEMC_START)
#error "DISP_4K MEMC size doesn't match for kernel & drivers!"
#endif

#define DISP_4K_SCALER_MDOMAIN_START (DISP_4K_SCALER_MEMC_START + DISP_4K_SCALER_MEMC_SIZE)
#define DISP_4K_SCALER_MDOMAIN_SIZE (13*_MB_) // For M-Doamin 2K FRC, 4K would be data frame sync

#if (DISP_4K_MDOMAIN_START_ADDRESS_KERNEL != DISP_4K_SCALER_MDOMAIN_START)
#error "DISP_4K Scaler M-domain size doesn't match for kernel & drivers!"
#endif

#define DISP_4K_SCALER_DI_NR_START (DISP_4K_SCALER_MDOMAIN_START + DISP_4K_SCALER_MDOMAIN_SIZE)
#define DISP_4K_SCALER_DI_NR_SIZE (13*_MB_)

#if (DISP_4K_DI_NR_START_ADDRESS_KERNEL != DISP_4K_SCALER_DI_NR_START)
#error "DISP_4K Scaler DI_NR size doesn't match for kernel & drivers!"
#endif

#define DISP_4K_SCALER_VIP_START (DISP_4K_SCALER_DI_NR_START + DISP_4K_SCALER_DI_NR_SIZE)
#define DISP_4K_SCALER_VIP_SIZE  (1848*_KB_)

#if (DISP_4K_VIP_START_ADDRESS_KERNEL != DISP_4K_SCALER_VIP_START)
#error "DISP_4K Scaler VIP size doesn't match for kernel & drivers!"
#endif

#define DISP_4K_SCALER_OD_START (DISP_4K_SCALER_VIP_START + DISP_4K_SCALER_VIP_SIZE)
#define DISP_4K_SCALER_OD_SIZE  (4296*_KB_)

//fixme: why can't enable this compile check in arm64?
#if (DISP_4K_OD_START_ADDRESS_KERNEL != DISP_4K_SCALER_OD_START)
#error "DISP_4K Scaler OD size doesn't match for kernel & drivers!"
#endif

#define DISP_4K_SCALER_NN_START (DISP_4K_SCALER_OD_START + DISP_4K_SCALER_OD_SIZE)
#define DISP_4K_SCALER_NN_SIZE (1*_MB_) // temp setting

#if (DISP_4K_NN_START_ADDRESS_KERNEL != DISP_4K_SCALER_NN_START)
#error "DISP_4K Scaler NN size doesn't match for kernel & drivers!"
#endif

#define DISP_4K_SCLAER_MODULE_START DISP_4K_SCALER_MEMC_START
#define DISP_4K_SCALER_MODULE_SIZE_QUERY (DISP_4K_SCALER_MEMC_SIZE +			\
										  DISP_4K_SCALER_MDOMAIN_SIZE + 		\
										  DISP_4K_SCALER_DI_NR_SIZE + 			\
										  DISP_4K_SCALER_NN_SIZE + 				\
										  DISP_4K_SCALER_VIP_SIZE + 			\
										  DISP_4K_SCALER_OD_SIZE)
#define DISP_4K_SCALER_MODULE_SIZE DO_ALIGNMENT(DISP_4K_SCALER_MODULE_SIZE_QUERY, 4*_MB_)
#define DISP_4K_SCALER_MODULE_BAND (DISP_4K_SCALER_MODULE_SIZE - DISP_4K_SCALER_MODULE_SIZE_QUERY)

#if (DISP_4K_SCALER_MODULE_SIZE & 0x3FFFFF) // 4MB align
#error "DISP_4K Scaler module size didn't follow 4MB alignment"
#endif

#define DISP_4K_DEMUX_TP_BUFFER_START (DISP_4K_SCLAER_MODULE_START + DISP_4K_SCALER_MODULE_SIZE)
#if 1//def CONFIG_RTK_KDRV_DEMUX
#define DISP_4K_DEMUX_TP_BUFFER_SIZE 24*_MB_  // must be 4MB page block alignment
#else
#define DISP_4K_DEMUX_TP_BUFFER_SIZE 0
#endif

#if (DISP_4K_TP_START_ADDRESS_KERNEL != DISP_4K_DEMUX_TP_BUFFER_START)
#error "DISP_4K TP buffer size doesn't match for kernel & drivers!"
#endif

/**
 *  --- DISP_4K memory layout end ---
 */


/**
 *  --- M7_8K memory layout start ---
 */
#if IS_ENABLED(CONFIG_REALTEK_LOGBUF)
#define M7_8K_CONST_LOGBUF_MEM_ADDR_START (0x1ca00000)
#define M7_8K_CONST_LOGBUF_MEM_SIZE (512*1024)
#else
#define M7_8K_CONST_LOGBUF_MEM_ADDR_START 0
#define M7_8K_CONST_LOGBUF_MEM_SIZE 0
#endif

#ifdef CONFIG_CUSTOMER_TV006	//for GAL reserve mem, 4MB align
extern const int M7_8K_GAL_MEM_SIZE;
extern const int M7_8K_GAL_MEM_ADDR_START;
#else
#define M7_8K_GAL_MEM_SIZE 0
#define M7_8K_GAL_MEM_ADDR_START 0
#endif

#ifdef CONFIG_RTK_BUSINESS_DISPLAY_MODEL

#define M7_8K_VBM_START_ADDRESS_KERNEL       0x2c000000  // + 1c000000 (+ 448 M)
#define M7_8K_MEMC_START_ADDRESS_KERNEL      0x48000000  // +        0 (+   0 M)
#define M7_8K_MDOMAIN_START_ADDRESS_KERNEL   0x48000000  // +  1c00000 (+  28 M)
#define M7_8K_DI_NR_START_ADDRESS_KERNEL     0x49c00000  // +  0d00000 (+  13 M)
#define M7_8K_VIP_START_ADDRESS_KERNEL       0x4a900000  // +  0200000 (+   2 M) -200K
#define M7_8K_OD_START_ADDRESS_KERNEL        0x4aace000  // +  0400000 (+   4 M) +200K
#define M7_8K_NN_START_ADDRESS_KERNEL        0x4af00000  // +        0 (+   0 M)
#define M7_8K_TP_START_ADDRESS_KERNEL        0x4b000000  // +  1800000 (+  24 M)

#define M7_8K_CPB_START_ADDRESS_KERNEL       0x75000000  //

#define M7_8K_SVP_CPB_START (M7_8K_CPB_START_ADDRESS_KERNEL)
#define M7_8K_SVP_CPB_SIZE (32*_MB_)

#if (M7_8K_CPB_START_ADDRESS_KERNEL != M7_8K_SVP_CPB_START)
#error "M7_8K SVP_CPB size doesn't match for kernel & drivers!"
#endif

#define M7_8K_CMA_LOW_START      (0x03800000)
#define M7_8K_CMA_LOW_SIZE        128*_MB_              //
#if (M7_8K_CMA_LOW_START & 0x3FFFFF) || (M7_8K_CMA_LOW_SIZE & 0x3FFFFF)
#error "M7_8K CMA_LOW no follow 4MB alignment"
#endif

#define M7_8K_VDEC_BUFFER_START (M7_8K_VBM_START_ADDRESS_KERNEL)
#define M7_8K_VDEC_BUFFER_SIZE 448*_MB_    // HEVC 8K decode 306 MB + 4K decimate 128 MB + 13 MB for compression header, and need 4MB aligned

#if (M7_8K_VBM_START_ADDRESS_KERNEL != M7_8K_VDEC_BUFFER_START)
#error "M7_8K VBM size doesn't match for kernel & drivers!"
#endif

#define M7_8K_SCALER_MEMC_START (M7_8K_VDEC_BUFFER_START + M7_8K_VDEC_BUFFER_SIZE)
#define M7_8K_SCALER_MEMC_SIZE (0*_MB_)

#if (M7_8K_MEMC_START_ADDRESS_KERNEL != M7_8K_SCALER_MEMC_START)
#error "M7_8K MEMC size doesn't match for kernel & drivers!"
#endif

#define M7_8K_SCALER_MDOMAIN_START (M7_8K_SCALER_MEMC_START + M7_8K_SCALER_MEMC_SIZE)
#define M7_8K_SCALER_MDOMAIN_SIZE (28*_MB_) // For M-Doamin 4K FRC 90/270 degree rotation, others 4K would be data frame sync

#if (M7_8K_MDOMAIN_START_ADDRESS_KERNEL != M7_8K_SCALER_MDOMAIN_START)
#error "M7_8K Scaler M-domain size doesn't match for kernel & drivers!"
#endif

#define M7_8K_SCALER_DI_NR_START (M7_8K_SCALER_MDOMAIN_START + M7_8K_SCALER_MDOMAIN_SIZE)
#define M7_8K_SCALER_DI_NR_SIZE (13*_MB_)

#if (M7_8K_DI_NR_START_ADDRESS_KERNEL != M7_8K_SCALER_DI_NR_START)
#error "M7_8K Scaler DI_NR size doesn't match for kernel & drivers!"
#endif

#define M7_8K_SCALER_VIP_START (M7_8K_SCALER_DI_NR_START + M7_8K_SCALER_DI_NR_SIZE)
#define M7_8K_SCALER_VIP_SIZE  (1848*_KB_)

#if (M7_8K_VIP_START_ADDRESS_KERNEL != M7_8K_SCALER_VIP_START)
#error "M7_8K Scaler VIP size doesn't match for kernel & drivers!"
#endif

#define M7_8K_SCALER_OD_START (M7_8K_SCALER_VIP_START + M7_8K_SCALER_VIP_SIZE)
#define M7_8K_SCALER_OD_SIZE  (4296*_KB_)

//fixme: why can't enable this compile check in arm64?
#if (M7_8K_OD_START_ADDRESS_KERNEL != M7_8K_SCALER_OD_START)
#error "M7_8K Scaler OD size doesn't match for kernel & drivers!"
#endif

#define M7_8K_SCALER_NN_START (M7_8K_SCALER_OD_START + M7_8K_SCALER_OD_SIZE)
#define M7_8K_SCALER_NN_SIZE (0*_MB_) // temp setting

#if (M7_8K_NN_START_ADDRESS_KERNEL != M7_8K_SCALER_NN_START)
#error "M7_8K Scaler NN size doesn't match for kernel & drivers!"
#endif

#define M7_8K_SCLAER_MODULE_START M7_8K_SCALER_MEMC_START
#define M7_8K_SCALER_MODULE_SIZE_QUERY (M7_8K_SCALER_MEMC_SIZE +			\
										  M7_8K_SCALER_MDOMAIN_SIZE + 		\
										  M7_8K_SCALER_DI_NR_SIZE + 			\
										  M7_8K_SCALER_NN_SIZE + 				\
										  M7_8K_SCALER_VIP_SIZE + 			\
										  M7_8K_SCALER_OD_SIZE)
#define M7_8K_SCALER_MODULE_SIZE DO_ALIGNMENT(M7_8K_SCALER_MODULE_SIZE_QUERY, 4*_MB_)
#define M7_8K_SCALER_MODULE_BAND (M7_8K_SCALER_MODULE_SIZE - M7_8K_SCALER_MODULE_SIZE_QUERY)

#if (M7_8K_SCALER_MODULE_SIZE & 0x3FFFFF) // 4MB align
#error "M7_8K Scaler module size didn't follow 4MB alignment"
#endif

#define M7_8K_DEMUX_TP_BUFFER_START (M7_8K_SCLAER_MODULE_START + M7_8K_SCALER_MODULE_SIZE)
#if 1//def CONFIG_RTK_KDRV_DEMUX
#define M7_8K_DEMUX_TP_BUFFER_SIZE 24*_MB_  // must be 4MB page block alignment
#else
#define M7_8K_DEMUX_TP_BUFFER_SIZE 0
#endif

#if (M7_8K_TP_START_ADDRESS_KERNEL != M7_8K_DEMUX_TP_BUFFER_START)
#error "M7_8K TP buffer size doesn't match for kernel & drivers!"
#endif

#else  // #ifdef CONFIG_RTK_BUSINESS_DISPLAY_MODEL

#define M7_8K_VBM_START_ADDRESS_KERNEL       0x20000000  // + 1c000000 (+ 448 M)
#define M7_8K_MEMC_START_ADDRESS_KERNEL      0x3c000000  // +  5700000 (+  87 M)
#define M7_8K_MDOMAIN_START_ADDRESS_KERNEL   0x41700000  // +  0d00000 (+  13 M)
#define M7_8K_DI_NR_START_ADDRESS_KERNEL     0x42400000  // +  0d00000 (+  13 M)
#define M7_8K_VIP_START_ADDRESS_KERNEL       0x43100000  // +  0200000 (+   2 M) -200K
#define M7_8K_OD_START_ADDRESS_KERNEL        0x432ce000  // +  0400000 (+   4 M) +200K
#define M7_8K_NN_START_ADDRESS_KERNEL        0x43700000  // +  0100000 (+   1 M)
#define M7_8K_TP_START_ADDRESS_KERNEL        0x43800000  // +  1800000 (+  24 M)

#define M7_8K_CPB_START_ADDRESS_KERNEL       0x75000000  //

#define M7_8K_SVP_CPB_START (M7_8K_CPB_START_ADDRESS_KERNEL)
#define M7_8K_SVP_CPB_SIZE (32*_MB_)

#if (M7_8K_CPB_START_ADDRESS_KERNEL != M7_8K_SVP_CPB_START)
#error "M7_8K SVP_CPB size doesn't match for kernel & drivers!"
#endif

#define M7_8K_CMA_LOW_START      (0x03800000)
#define M7_8K_CMA_LOW_SIZE        128*_MB_              //
#if (M7_8K_CMA_LOW_START & 0x3FFFFF) || (M7_8K_CMA_LOW_SIZE & 0x3FFFFF)
#error "M7_8K CMA_LOW no follow 4MB alignment"
#endif

#define M7_8K_VDEC_BUFFER_START (M7_8K_VBM_START_ADDRESS_KERNEL)
#define M7_8K_VDEC_BUFFER_SIZE 448*_MB_    // HEVC 8K decode 306 MB + 4K decimate 128 MB + 13 MB for compression header, and need 4MB aligned

#if (M7_8K_VBM_START_ADDRESS_KERNEL != M7_8K_VDEC_BUFFER_START)
#error "M7_8K VBM size doesn't match for kernel & drivers!"
#endif

#define M7_8K_SCALER_MEMC_START (M7_8K_VDEC_BUFFER_START + M7_8K_VDEC_BUFFER_SIZE)
#define M7_8K_SCALER_MEMC_SIZE (87*_MB_)

#if (M7_8K_MEMC_START_ADDRESS_KERNEL != M7_8K_SCALER_MEMC_START)
#error "M7_8K MEMC size doesn't match for kernel & drivers!"
#endif

#define M7_8K_SCALER_MDOMAIN_START (M7_8K_SCALER_MEMC_START + M7_8K_SCALER_MEMC_SIZE)
#define M7_8K_SCALER_MDOMAIN_SIZE (13*_MB_) // For M-Doamin 2K FRC, 4K would be data frame sync

#if (M7_8K_MDOMAIN_START_ADDRESS_KERNEL != M7_8K_SCALER_MDOMAIN_START)
#error "M7_8K Scaler M-domain size doesn't match for kernel & drivers!"
#endif

#define M7_8K_SCALER_DI_NR_START (M7_8K_SCALER_MDOMAIN_START + M7_8K_SCALER_MDOMAIN_SIZE)
#define M7_8K_SCALER_DI_NR_SIZE (13*_MB_)

#if (M7_8K_DI_NR_START_ADDRESS_KERNEL != M7_8K_SCALER_DI_NR_START)
#error "M7_8K Scaler DI_NR size doesn't match for kernel & drivers!"
#endif

#define M7_8K_SCALER_VIP_START (M7_8K_SCALER_DI_NR_START + M7_8K_SCALER_DI_NR_SIZE)
#define M7_8K_SCALER_VIP_SIZE  (1848*_KB_)

#if (M7_8K_VIP_START_ADDRESS_KERNEL != M7_8K_SCALER_VIP_START)
#error "M7_8K Scaler VIP size doesn't match for kernel & drivers!"
#endif

#define M7_8K_SCALER_OD_START (M7_8K_SCALER_VIP_START + M7_8K_SCALER_VIP_SIZE)
#define M7_8K_SCALER_OD_SIZE  (4296*_KB_)

//fixme: why can't enable this compile check in arm64?
#if (M7_8K_OD_START_ADDRESS_KERNEL != M7_8K_SCALER_OD_START)
#error "M7_8K Scaler OD size doesn't match for kernel & drivers!"
#endif

#define M7_8K_SCALER_NN_START (M7_8K_SCALER_OD_START + M7_8K_SCALER_OD_SIZE)
#define M7_8K_SCALER_NN_SIZE (1*_MB_) // temp setting

#if (M7_8K_NN_START_ADDRESS_KERNEL != M7_8K_SCALER_NN_START)
#error "M7_8K Scaler NN size doesn't match for kernel & drivers!"
#endif

#define M7_8K_SCLAER_MODULE_START M7_8K_SCALER_MEMC_START
#define M7_8K_SCALER_MODULE_SIZE_QUERY (M7_8K_SCALER_MEMC_SIZE +			\
										  M7_8K_SCALER_MDOMAIN_SIZE + 		\
										  M7_8K_SCALER_DI_NR_SIZE + 			\
										  M7_8K_SCALER_NN_SIZE + 				\
										  M7_8K_SCALER_VIP_SIZE + 			\
										  M7_8K_SCALER_OD_SIZE)
#define M7_8K_SCALER_MODULE_SIZE DO_ALIGNMENT(M7_8K_SCALER_MODULE_SIZE_QUERY, 4*_MB_)
#define M7_8K_SCALER_MODULE_BAND (M7_8K_SCALER_MODULE_SIZE - M7_8K_SCALER_MODULE_SIZE_QUERY)

#if (M7_8K_SCALER_MODULE_SIZE & 0x3FFFFF) // 4MB align
#error "M7_8K Scaler module size didn't follow 4MB alignment"
#endif

#define M7_8K_DEMUX_TP_BUFFER_START (M7_8K_SCLAER_MODULE_START + M7_8K_SCALER_MODULE_SIZE)
#if 1//def CONFIG_RTK_KDRV_DEMUX
#define M7_8K_DEMUX_TP_BUFFER_SIZE 24*_MB_  // must be 4MB page block alignment
#else
#define M7_8K_DEMUX_TP_BUFFER_SIZE 0
#endif

#if (M7_8K_TP_START_ADDRESS_KERNEL != M7_8K_DEMUX_TP_BUFFER_START)
#error "M7_8K TP buffer size doesn't match for kernel & drivers!"
#endif

#endif

/**
 *  --- M7_8K memory layout end ---
 */


#define RTK_GPU_FW_SIZE (512*1024)

#define CMA_HIGHMEM_SMALL 128*_MB_
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
#define CMA_HIGHMEM_LARGE 660*_MB_
#else
#define CMA_HIGHMEM_LARGE 712*_MB_
#endif
#if defined(CONFIG_KASAN) && defined(CONFIG_ARCH_RTK6702)
#define CMA_HIGHMEM_EXT_LARGE 1280*_MB_
#else
#define CMA_HIGHMEM_EXT_LARGE 1024*_MB_
#endif
#ifdef CONFIG_RTK_BUSINESS_DISPLAY_MODEL
#define CMA_HIGHMEM_M7_8K 1048*_MB_

#define CMA_GPU_4K_ION_SIZE (0x1fa4000)
#define CMA_GPU_4K_ION_NUMBER 6
#define CMA_GPU_4K_SIZE DO_ALIGNMENT(CMA_GPU_4K_ION_SIZE * CMA_GPU_4K_ION_NUMBER, 4*_MB_)
#else
#define CMA_HIGHMEM_M7_8K 1100*_MB_

#define CMA_GPU_4K_ION_SIZE (0x2023000)
#define CMA_GPU_4K_ION_NUMBER 9
#define CMA_GPU_4K_SIZE DO_ALIGNMENT(CMA_GPU_4K_ION_SIZE * CMA_GPU_4K_ION_NUMBER, 4*_MB_)
#endif

#ifdef RTK_MEM_LAYOUT_DEVICETREE
    // test if not used
#else
#define ZRAM_RESERVED_SIZE 128*_MB_
#define RTK_SPI_ADDR	0x80200000
#define RTK_SPI_SIZE	0x02000000
#endif


#endif //_RTK_MEMORY_LAYOUT_H
