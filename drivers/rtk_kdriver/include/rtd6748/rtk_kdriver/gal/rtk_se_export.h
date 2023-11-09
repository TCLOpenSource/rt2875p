#ifndef SE_EXPORT_H
#define SE_EXPORT_H
#include <linux/types.h>
#include <rtk_kadp_se.h> /*for ioctl->kadp se*/

extern struct se_dev *se_devices;
void rtk_se_power(int state);
bool rtk_se_checkFinish(void);
bool rtk_se_checkFinishV(void);

#define se_usleep(usecs) \
(void)({ \
		  unsigned long us___ = (unsigned long)(usecs); \
		  if (us___ < 10) \
		  udelay(us___); \
		  else if (us___ >= 10 && us___ <= 20000) \
			  usleep_range(us___, us___); \
			  else \
				  msleep_interruptible(DIV_ROUND_UP(us___, 1000)); \
				  (void)us___; \
				})

#if SE_SE20 || SE_OLD
unsigned int se_get_mem(void);
void se_free_mem(void);
#endif
void se_cmdqueue_lock(long timeout);
void se_cmdqueue_unlock(void);
void _WriteCmd(struct se_dev *dev, uint8_t* pbyCommandBuffer, int32_t lCommandLength);
void _ExecuteCmd(struct se_dev *dev, uint8_t* pbyCommandBuffer, int32_t lCommandLength);
void WriteCmd(struct se_dev *dev, uint8_t* pbyCommandBuffer, int32_t lCommandLength);
void WriteCmdISR(struct se_dev *dev, uint8_t* pbyCommandBuffer, int32_t lCommandLength);

typedef struct _SECMD_EXT_FMT_CONV_SIM
{
	struct
	{
		uint32_t	OpCode: 4;
		uint32_t	RegisterAddress: 12;
		uint32_t	Clut: 1;
		uint32_t	Reserved: 15;
	} WordReg1;
	struct
	{
		uint32_t	RegisterValue;
	} WordReg1_1;
	struct
	{
		uint32_t	OpCode: 4;
		uint32_t	RegisterAddress: 12;
		uint32_t	Clut: 1;
		uint32_t	Reserved: 15;
	} WordAddr0;
	struct
	{
		uint32_t	RegisterValue;
	} WordAddr0_1;
	struct
	{
		uint32_t	OpCode: 4;
		uint32_t	RegisterAddress: 12;
		uint32_t	Clut: 1;
		uint32_t	Reserved: 15;
	} WordPitch0;
	struct
	{
		uint32_t	RegisterValue;
	} WordPitch0_1;
	struct
	{
		uint32_t	OpCode: 4;
		uint32_t	RegisterAddress: 12;
		uint32_t	Clut: 1;
		uint32_t	Reserved: 15;
	} WordAddr1;
	struct
	{
		uint32_t	RegisterValue;
	} WordAddr1_1;
	struct
	{
		uint32_t	OpCode: 4;
		uint32_t	RegisterAddress: 12;
		uint32_t	Clut: 1;
		uint32_t	Reserved: 15;
	} WordPitch1;
	struct
	{
		uint32_t	RegisterValue;
	} WordPitch1_1;
	struct
	{
		uint32_t	OpCode: 4;
		uint32_t	RegisterAddress: 12;
		uint32_t	Clut: 1;
		uint32_t	Reserved: 15;
	} WordAddr2;
	struct
	{
		uint32_t	RegisterValue;
	} WordAddr2_1;
	struct
	{
		uint32_t	OpCode: 4;
		uint32_t	RegisterAddress: 12;
		uint32_t	Clut: 1;
		uint32_t	Reserved: 15;
	} WordPitch2;
	struct
	{
		uint32_t	RegisterValue;
	} WordPitch2_1;
	struct
	{
		uint32_t	OpCode: 4;
		uint32_t	RegisterAddress: 12;
		uint32_t	Clut: 1;
		uint32_t	Reserved: 15;
	} WordAddr3;
	struct
	{
		uint32_t	RegisterValue;
	} WordAddr3_1;
	struct
	{
		uint32_t	OpCode: 4;
		uint32_t	RegisterAddress: 12;
		uint32_t	Clut: 1;
		uint32_t	Reserved: 15;
	} WordPitch3;
	struct
	{
		uint32_t	RegisterValue;
	} WordPitch3_1;
	struct
	{
		uint32_t	opcode: 4;
		uint32_t	rslt_fmt: 5;
		uint32_t	lay_ctl: 1;
		uint32_t	lay_num: 1;
		uint32_t	sync_vo: 1;
		uint32_t	rslt_swp_chr: 2;
		uint32_t	src_swp_chr: 2;
		uint32_t	itu_en: 1;
		uint32_t	clip_10b: 1;
		uint32_t	rslt_matx_en: 1;
		uint32_t	out_alpha: 1;
		uint32_t	rslt_argb_order: 5;
		uint32_t	reserved0: 1;
		uint32_t	src2_argb_order: 5;
		uint32_t	two_dc_mode: 1;
	} Word1;
	struct
	{
		uint32_t	rslt_out_x: 13;
		uint32_t	rslt_out_y: 13;
		uint32_t	reserved0: 6;
	} Word2;
	struct
	{
		uint32_t	rslt_chr2_addr_index: 7;
		uint32_t	reserved0: 1;
		uint32_t	rslt_chr1_addr_index: 7;
		uint32_t	reserved1: 1;
		uint32_t	rslt_addr_index: 7;
		uint32_t	rslt_blk_mode_interleave: 1;
		uint32_t	reserved2: 8;
	} Word3;
	struct
	{
		uint32_t	src1_inpt_w: 13;
		uint32_t	src1_inpt_h: 13;
		uint32_t	src1_fmt: 5;
		uint32_t	src1_mat_en: 1;
	} Word4;
	struct
	{
		uint32_t	src1_inpt_x: 13;
		uint32_t	src1_inpt_y: 13;
		uint32_t	src1_argb_order: 5;
		uint32_t	src1_fcv_stch: 1;
	} Word5;
	struct
	{
		uint32_t	src1_chr2_addr_index: 7;
		uint32_t	reserved0: 1;
		uint32_t	src1_chr1_addr_index: 7;
		uint32_t	reserved1: 1;
		uint32_t	src1_addr_index: 7;
		uint32_t	src1_blk_mode_interleave: 1;
		uint32_t	src1_inpt_sel: 1;
		uint32_t	reserved2: 7;
	} Word6;
} SECMD_EXT_FMT_CONV_SIM;

typedef struct _SECMD_EXD_FMT_CONV_FILM
{
	struct
	{
		uint32_t	OpCode: 4;
		uint32_t	RegisterAddress: 12;
		uint32_t	Clut: 1;
		uint32_t	Reserved: 15;
	} WordAddr0;
	struct
	{
		uint32_t	RegisterValue;
	} WordAddr0_1;
	struct
	{
		uint32_t	OpCode: 4;
		uint32_t	RegisterAddress: 12;
		uint32_t	Clut: 1;
		uint32_t	Reserved: 15;
	} WordPitch0;
	struct
	{
		uint32_t	RegisterValue;
	} WordPitch0_1;
	struct
	{
		uint32_t	OpCode: 4;
		uint32_t	RegisterAddress: 12;
		uint32_t	Clut: 1;
		uint32_t	Reserved: 15;
	} WordAddr1;
	struct
	{
		uint32_t	RegisterValue;
	} WordAddr1_1;
	struct
	{
		uint32_t	OpCode: 4;
		uint32_t	RegisterAddress: 12;
		uint32_t	Clut: 1;
		uint32_t	Reserved: 15;
	} WordPitch1;
	struct
	{
		uint32_t	RegisterValue;
	} WordPitch1_1;
	struct
	{
		uint32_t	OpCode: 4;
		uint32_t	RegisterAddress: 12;
		uint32_t	Clut: 1;
		uint32_t	Reserved: 15;
	} WordAddr2;
	struct
	{
		uint32_t	RegisterValue;
	} WordAddr2_1;
	struct
	{
		uint32_t	OpCode: 4;
		uint32_t	RegisterAddress: 12;
		uint32_t	Clut: 1;
		uint32_t	Reserved: 15;
	} WordPitch2;
	struct
	{
		uint32_t	RegisterValue;
	} WordPitch2_1;
	struct
	{
		uint32_t	OpCode: 4;
		uint32_t	RegisterAddress: 12;
		uint32_t	Clut: 1;
		uint32_t	Reserved: 15;
	} WordAddr3;
	struct
	{
		uint32_t	RegisterValue;
	} WordAddr3_1;
	struct
	{
		uint32_t	OpCode: 4;
		uint32_t	RegisterAddress: 12;
		uint32_t	Clut: 1;
		uint32_t	Reserved: 15;
	} WordPitch3;
	struct
	{
		uint32_t	RegisterValue;
	} WordPitch3_1;
	struct
	{
		uint32_t	opcode: 4;
		uint32_t	rslt_fmt: 5;
		uint32_t	lay_ctl: 1;
		uint32_t	lay_num: 1;
		uint32_t	sync_vo: 1;
		uint32_t	rslt_swp_chr: 2;
		uint32_t	src_swp_chr: 2;
		uint32_t	itu_en: 1;
		uint32_t	clip_10b: 1;
		uint32_t	rslt_matx_en: 1;
		uint32_t	out_alpha: 1;
		uint32_t	rslt_argb_order: 5;
		uint32_t	reserved0: 1;
		uint32_t	src2_argb_order: 5;
		uint32_t	two_dc_mode: 1;
	} Word1;
	struct
	{
		uint32_t	rslt_out_x: 13;
		uint32_t	rslt_out_y: 13;
		uint32_t	reserved0: 6;
	} Word2;
	struct
	{
		uint32_t	rslt_chr2_addr_index: 7;
		uint32_t	reserved0: 1;
		uint32_t	rslt_chr1_addr_index: 7;
		uint32_t	reserved1: 1;
		uint32_t	rslt_addr_index: 7;
		uint32_t	rslt_blk_mode_interleave: 1;
		uint32_t	reserved2: 8;
	} Word3;
	struct
	{
		uint32_t	src1_inpt_w: 13;
		uint32_t	src1_inpt_h: 13;
		uint32_t	src1_fmt: 5;
		uint32_t	src1_mat_en: 1;
	} Word4;
	struct
	{
		uint32_t	src1_inpt_x: 13;
		uint32_t	src1_inpt_y: 13;
		uint32_t	src1_argb_order: 5;
		uint32_t	src1_fcv_stch: 1;
	} Word5;
	struct
	{
		uint32_t	src1_chr2_addr_index: 7;
		uint32_t	reserved0: 1;
		uint32_t	src1_chr1_addr_index: 7;
		uint32_t	reserved1: 1;
		uint32_t	src1_addr_index: 7;
		uint32_t	src1_blk_mode_interleave: 1;
		uint32_t	src1_inpt_sel: 1;
		uint32_t	reserved2: 7;
	} Word6;
	struct
	{
		uint32_t	src2_inpt_x: 13;
		uint32_t	src2_inpt_y: 13;
		uint32_t	src2_fmt: 5;
		uint32_t	src2_mat_en: 1;
	} Word7;
	struct
	{
		uint32_t	src2_c2_addr_index: 7;
		uint32_t	Reserved4: 1;
		uint32_t	src2_c1_addr_index: 7;
		uint32_t	Reserved3: 1;
		uint32_t	src2_addr_index: 7;
		uint32_t	Reserved2: 1;
		uint32_t	src2_inpt_sel: 1;
		uint32_t	src2_swp_chroma: 2;
		uint32_t	Reserved1:1;
		uint32_t	src2_linear_gradient:1;
		uint32_t	Reserved: 3;
	} Word8;
	struct
	{
		uint32_t	sel_s2_alpha_s1s2: 3;
		uint32_t	Reserved2: 1;
		uint32_t	sel_s1_alpha_s1s2: 3;
		uint32_t	Reserved1: 1;
		uint32_t	lay_num_ext: 3;
		uint32_t	Reserved0: 21;
	} Word9;
	struct
	{
		uint32_t	ROP_code_s1s2: 4;
		uint32_t	fb_sel_destclr_blend_s1s2: 4;
		uint32_t	fb_sel_srcclr_blend_s1s2: 4;
		uint32_t	fb_demultiply_s1s2: 1;
		uint32_t	fb_blend_alphachannel_s1s2: 1;
		uint32_t	fb_blend_coloralpha_s1s2: 1;
		uint32_t	fb_colorize_s1s2: 1;
		uint32_t	fb_xor_s1s2: 1;
		uint32_t	fb_src_premultiply_s1s2: 1;
		uint32_t	fb_src_premulcolor_s1s2: 1;
		uint32_t	fb_dst_premultiply_s1s2: 1;
		uint32_t	src_clr_transparency_s1s2: 1;//merlin3 no support
		uint32_t	src_clr_transform_s1s2: 2;//merlin3 no support
		uint32_t	reserved0: 2;
		uint32_t	color_key_s1s2: 2;
		uint32_t	blend_sel_s1s2: 4;
		uint32_t	reserved1: 1;
	} Word10;
	struct
	{
		uint32_t	gl_sel_destclr_blend_s1s2: 6;
		uint32_t	reserved0: 2;
		uint32_t	gl_sel_srcclr_blend_s1s2: 6;
		uint32_t	reserved1: 2;
		uint32_t	gl_sel_destalpha_s1s2: 6;
		uint32_t	reserved2: 2;
		uint32_t	gl_sel_src_alpha_s1s2: 6;
		uint32_t	reserved3: 2;
	} Word11;

	struct
	{
		uint32_t   Reserved: 32;
	} Word12;
} SECMD_EXD_FMT_CONV_FILM;

/*
enum YUV_FMT {
	FMT_YUYV    = 0,
	FMT_YUV420  = 1,
	FMT_YUV422P = 2,
};
typedef enum YUV_FMT YUV_FMT;
*/

typedef struct SE_PROC_INFO{
	uint32_t sAddr;
	int sX;
	int sY;
	int sWidth;
	int sHeight;
	int sPitch;
	uint32_t dAddr;
	int dX;
	int dY;
	int RotateType;
} SE_SCALE_T;

typedef struct SE_LOCK_INFO{
	int m_pid;
	int m_ppid;
	int m_tid;
} SE_LOCK_INFO_T;

typedef enum EXT_SWAP_T {
	EXT_YUYV    = 0, //or Cb first
	EXT_YVYU    = 1, //or Cr first
	EXT_UYVY    = 2,
	EXT_VYUY    = 3
}EXT_SWAP;

typedef enum EXT_FMT_T{
	EXT_FBC_4U8	= 0x3,
	EXT_ARGB8	= 0xc,
	EXT_YUV4	= 0x10,
	EXT_YV12_422	= 0x14,
	EXT_NV12_422	= 0x15,
	EXT_YUY2_422	= 0x16,
	EXT_YV12_420	= 0x18,
	EXT_NV12_420	= 0x19
}EXT_FMT;

/*SE_IOC_FMT */
typedef struct SE_FMT_INFO{
	uint32_t width;
	uint32_t height;
	uint32_t s1_CPUAddr;	
	uint32_t s1_Addr;
	EXT_FMT  sFmt;
	EXT_SWAP sSwap;
	uint32_t s2_CPUAddr;	
	uint32_t s2_Addr;
	EXT_FMT  dFmt;
	EXT_SWAP dSwap;
} SE_FMT_ST;

typedef struct KADP_SE_PALE_T {
	KGAL_PALETTE_SURFACE_INFO_T pale;
	SE_LOCK_INFO_T pinfo;
} KADP_SE_PALE_ST;

typedef struct KADP_SE_BLIT_T {
	KGAL_SURFACE_INFO_T tSrcSurface;
	KGAL_RECT_T tSrcRect;
	KGAL_SURFACE_INFO_T tDstSurface;
	uint16_t dx;
	uint16_t dy;
	KGAL_BLIT_FLAGS_T tBlitFlags;
	KGAL_BLIT_SETTINGS_T tBlitSettings;
	SE_LOCK_INFO_T pinfo;
} KADP_SE_BLIT_ST;

typedef struct KADP_SE_STCHBLIT_T {
	KGAL_SURFACE_INFO_T tSrcSurface;
	KGAL_RECT_T tSrcRect;
	KGAL_SURFACE_INFO_T tDstSurface;
	KGAL_RECT_T tDstRect;
	KGAL_BLIT_FLAGS_T tBlitFlags;
	KGAL_BLIT_SETTINGS_T tBlitSettings;
	SE_LOCK_INFO_T pinfo;
} KADP_SE_STCHBLIT_ST;


typedef struct KADP_SE_DRAW_T {
	KGAL_SURFACE_INFO_T tDstSurface;
	KGAL_RECT_T tDstRect;
	uint32_t color;
	KGAL_LINE_T tDstLine;
	KGAL_DRAW_FLAGS_T tDrawFlags;
	KGAL_DRAW_SETTINGS_T tDrawSettings;
	SE_LOCK_INFO_T pinfo;
} KADP_SE_DRAW_ST;



#define SE_IOC_MAGIC			'k'
#define SE_IOCRESET			_IO(SE_IOC_MAGIC,   0)
#define SE_IOCSQUANTUM			_IOW(SE_IOC_MAGIC,  1, int)
#define SE_IOCSQSET			_IOW(SE_IOC_MAGIC,  2, int)
#define SE_IOCTQUANTUM			_IO(SE_IOC_MAGIC,   3)
#define SE_IOCTQSET			_IO(SE_IOC_MAGIC,   4)
#define SE_IOCGQUANTUM			_IOR(SE_IOC_MAGIC,  5, int)
#define SE_IOCGQSET			_IOR(SE_IOC_MAGIC,  6, int)
#define SE_IOCQQUANTUM			_IO(SE_IOC_MAGIC,   7)
#define SE_IOCQQSET			_IO(SE_IOC_MAGIC,   8)
#define SE_IOCXQUANTUM			_IOWR(SE_IOC_MAGIC, 9, int)
#define SE_IOCXQSET			_IOWR(SE_IOC_MAGIC,10, int)
#define SE_IOC_SETPALTE			_IOR(SE_IOC_MAGIC,  11, int)
#define SE_IOC_BLIT			_IOR(SE_IOC_MAGIC,  12, int)
#define SE_IOC_STCHBLIT			_IOR(SE_IOC_MAGIC,  13, int)
#define SE_IOC_FILLRECT			_IOR(SE_IOC_MAGIC,  14, int)
#define SE_IOC_DRAWRECT			_IOR(SE_IOC_MAGIC,  15, int)
#define SE_IOC_DRAWLINE			_IOR(SE_IOC_MAGIC,  16, int)
#define SE_IOC_FMTCONV			_IOR(SE_IOC_MAGIC,  17, int)
#define SE_IOC_WAITFINH			_IOR(SE_IOC_MAGIC,  18, int)
#define SE_IOC_SET_DCU_INFO		_IOW(SE_IOC_MAGIC, 19, int)
#define SE_IOC_READ_HW_CMD_COUNTER_LOW_WORD	_IOR(SE_IOC_MAGIC, 20, int)
#define SE_IOC_SET_VQUEUE		_IOR(SE_IOC_MAGIC, 21, int)
#define SE_IOC_READ_INST_COUNT		_IOR(SE_IOC_MAGIC, 22, int)
#define SE_IOC_WRITE_ISSUE_CMD		_IOR(SE_IOC_MAGIC, 23, int)
#define SE_IOC_WRITE_QUEUE_CMD		_IOR(SE_IOC_MAGIC, 24, int)
#define SE_IOC_VIDEO_RPC_YUYV_TO_RGB	_IO(SE_IOC_MAGIC, 25)
#define SE_IOC_WAIT_FOR_COMPLETE	_IO(SE_IOC_MAGIC, 26)
#define SE_IOC_SCALING_DOWN		_IOR(SE_IOC_MAGIC, 27, SE_SCALE_T)
#define SE_IOC_DUMP_REGISTER		_IOR(SE_IOC_MAGIC, 28, int)
#define SE_IOC_DISP_SIZE		_IOW(SE_IOC_MAGIC, 29, int)
#define SE_IOC_CMD_LOCK			_IOR(SE_IOC_MAGIC, 30, SE_LOCK_INFO_T)
#define SE_IOC_CMD_UNLOCK		_IO(SE_IOC_MAGIC, 31)
#define SE_IOC_HW_RESET			_IOR(SE_IOC_MAGIC, 32, SE_LOCK_INFO_T)
#define SE_IOC_PM_CTRL			_IOW(SE_IOC_MAGIC, 33, int)
#define SE_IOC_FMT			_IOW(SE_IOC_MAGIC, 34, SE_FMT_ST)
#define SE_IOC_MAXNR 35

#endif
