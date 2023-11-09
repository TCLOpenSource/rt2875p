#if 0
#ifndef __VSC_H
#define  __VSC_H

//#define _DISP_WID (Get_DISP_ACT_END_HPOS() - Get_DISP_ACT_STA_HPOS())
//#define _DISP_LEN  (Get_DISP_ACT_END_VPOS()  - Get_DISP_ACT_STA_VPOS() )

typedef enum {
	VODMA_COLOR_RGB    = 0x00,
	VODMA_COLOR_YUV422,
	VODMA_COLOR_YUV444,
	VODMA_COLOR_YUV411,
	VODMA_COLOR_UNKNOW
} VODMA_COLOR_SPACE_T;
 typedef enum
{
	INPUT_BASE_ON_TIMING = 0,
	INPUT_BASE_ON_DISP_WIN,
	INPUT_BASE_ON_HAL_ORIGINAL_INPUT,
} SCALER_VSC_INPUT_REGION_TYPE;
typedef enum _DISPLAY_PIXEL_MODE{
	PIXEL_MODE_1_PIXEL=0,
	PIXEL_MODE_2_PIXEL,
	PIXEL_MODE_4_PIXEL,
}DISPLAY_PIXEL_MODE;
typedef enum _DI_RTNR_Mode_By_AP{
    DI_RTNR_OFF = 0,
    DI_RTNR_422 = 1,
    DI_RTNR_420 = 2,
    DI_RTNR_400 = 3,

} DI_RTNR_Mode_By_AP;

typedef enum _SLR_DMA_mode{
	SLR_DMA_422_mode 			= 0,
	SLR_DMA_420_mode 			= 1,
	SLR_DMA_400_mode 			= 2,
} SLR_DMA_mode;


enum E_uzu_dering_h{
	e_uzu_dering_h_h4h12_blend_en,
	e_uzu_dering_h_h4h12_blend_range,
	e_uzu_dering_h_h4h12_blend_lowbound,
	e_uzu_dering_h_lpf_blend_en,
	e_uzu_dering_h_lpf_blend_mask_sel,
	e_uzu_dering_h_lpf_blend_range,
	e_uzu_dering_h_lpf_blend_lowbound,
	e_uzu_dering_h_level_flatness_coeff,
	e_uzu_dering_h_level_maxmin_coeff,
	e_uzu_dering_h_dering_h4h12_blend_type,// gap dering enable
	e_uzu_dering_h_dering_gap_sidemode,
	e_uzu_dering_h_dering_gap_offset,
	e_uzu_dering_h_dering_gap_center_gain,
	e_uzu_dering_h_dering_gap_side_gain,
	e_uzu_dering_h__count
};

//compression mode
typedef enum {
	COMPRESSION_LINE_MODE = 0,
	COMPRESSION_FRAME_MODE = 1,
	COMPRESSION_NONE_MODE = 2,
}MemCompression_Mode;
typedef enum
{
	MEMCAPTYPE_LINE = 0x00,
	MEMCAPTYPE_FRAME = 0x01
}eMemCapAccessType;

/*// Memory Index
typedef enum {
	MEMIDX_VDC=0x0,
	MEMIDX_TT,
	MEMIDX_JPEG,
	MEMIDX_DI,
	MEMIDX_MAIN,
	MEMIDX_MAIN_SEC,
	MEMIDX_MAIN_THIRD,
	MEMIDX_MAIN_FOURTH,
	MEMIDX_MAIN_FIFTH,
	MEMIDX_MAIN_SIXTH,
	MEMIDX_MAIN_SEVENTH,
	MEMIDX_MAIN_EIGHTH,
	MEMIDX_SUB,
	MEMIDX_SUB_SEC,
	MEMIDX_SUB_THIRD,
	MEMIDX_MAIN_2,
	MEMIDX_MAIN_2_SEC,
	MEMIDX_MAIN_2_THIRD,
	MEMIDX_DI_2,
	MEMIDX_ALL,
	MEMIDX_UNKNOW
}eMemIdx;
*/
// Structure of memory tag
typedef enum {
	UNDEFINED = 0,
	ALLOCATED_FROM_LOW = 1,
	ALLOCATED_FROM_HIGH = 2,
	ALLOCATED_FROM_PLI = 3,
} MemBlockStatus;
typedef struct _MemoryBlock
{
	unsigned int Allocate_StartAddr;
	unsigned long Allocate_VirAddr;
	MemBlockStatus Status;		// 0: undefined, 1: allocated from low, 2: allocated from high
	unsigned long StartAddr;

	unsigned int Size;
}StructMemBlock;

unsigned char scaler_quickshow_output_connect(void);

#endif
#endif