#ifndef __RTK_QOS_CONTROL_H__
#define __RTK_QOS_CONTROL_H__

#define ENABLE_IB_SUPPORT

typedef enum {
	RTK_QOS_BOOT_MODE = 0x0,
	RTK_QOS_NORMAL_MODE = 0x1,
	RTK_QOS_DECODER_4K_MODE = 0x2,
	RTK_QOS_DECODER_2K_MODE = 0x3, // special for 2K enable DeXC
	RTK_QOS_HDMI_4K_MODE = 0x4,
	RTK_QOS_HDMI_2K_MODE = 0x5,
	RTK_QOS_USER_A_MODE = 0x6,
	RTK_QOS_DECODER_4K_OPT1_MODE = 0x7,
	RTK_QOS_DECODER_2K_OPT1_MODE = 0x8,
	RTK_QOS_DECODER_BAD_EDIT_MODE = 0x9,
	RTK_QOS_DECODER_8K_AV1_MODE = 0xa,
	RTK_QOS_MODE_NUM
} RTK_QOS_MODE_T;

typedef enum {
	RTK_BW_LIMIT_NONE = 0x0, /* specific value */
	RTK_BW_LIMIT_GPU = 0x1, /* mask bit0 */
	RTK_BW_LIMIT_TVSB3 = 0x2, /* mask bit1 */
	RTK_BW_LIMIT_BAD_EDIT = 0xbad, /* specific value */
	RTK_BW_LIMIT_IGNORE = 0xFFFF, /* ignore setting */
	RTK_BW_LIMIT_NUM
} RTK_BW_LIMIT_T;

typedef enum {
	RTK_BW_LMMIT_HW_OFF = 0x0,
	RTK_BW_LMMIT_HW_REQ = 0x1,
	RTK_BW_LMMIT_HW_DAT = 0x2,
	RTK_BW_LMMIT_HW_PRE_TRA = 0x3
} RTK_BW_LIMIT_HW_MODE_T;

typedef enum {
	RTK_BRIG_VE = 0x5,
	RTK_BRIG_GPU = 0x7,
	RTK_BRIG_NNIP = 0x9,
	RTK_BRIG_TVSB3 = 0xA,
	RTK_BRIG_ERROR = 0xdeaddead
} RTK_BRIG_T;

typedef enum {
	RTK_SYS_PATH1 = 0x1,
	RTK_SYS_PATH3 = 0x3,
	RTK_SYS_ERROR = 0xdeaddead
} RTK_SYS_PATH_T;

#define DBUS_BUS_ID_NUM		16
#define DBUS_CLIENT_ID_NUM	16

typedef struct {
//	unsigned short idx[2]; // record_idx and curr_pos
	unsigned int r_count[DBUS_BUS_ID_NUM][DBUS_CLIENT_ID_NUM]; // read  count from sram record
	unsigned int w_count[DBUS_BUS_ID_NUM][DBUS_CLIENT_ID_NUM]; // write count from sram record
} SRAM_RECORD_t;

extern void rtk_qos_set_vdec_codec_type(int rtkvdec_codec_type);
extern int rtk_qos_get_vdec_codec_type(void);
extern int rtk_qos_set_mode(RTK_QOS_MODE_T qos_mode);
extern int rtk_qos_set_bad_edit(unsigned int is_bad_edit);
extern int rtk_qos_suspend(void);
extern int rtk_qos_resume(void);
extern int rtk_qos_mode_parsing(const char *buf);
extern RTK_SYS_PATH_T rtk_sys_brig_get_curr_path(RTK_BRIG_T brig);
extern int rtk_qos_lock_current_mode(void);
extern int rtk_qos_unlock_current_mode(void);
extern char * const rtk_qos_get_current_mode(void);

#ifdef ENABLE_IB_SUPPORT
// IB related
typedef enum {
	RTK_IB_REGION_0 = 0,
	RTK_IB_REGION_1,
	RTK_IB_REGION_2,
	RTK_IB_REGION_3,
	RTK_IB_REGION_TOTAL_NUM
}RTK_IB_REGION_INDEX;

typedef enum {
	RTK_IB_REGION_MODE_DC1 = 0,
	RTK_IB_REGION_MODE_DC2,
	RTK_IB_REGION_MODE_IB_1_1,
	RTK_IB_REGION_MODE_IB_1_1_SCRAMBLE,
	RTK_IB_REGION_MODE_IB_1_2,
	RTK_IB_REGION_MODE_IB_2_1,
}RTK_IB_REGION_MODE;

typedef struct {
	unsigned int start_addr;
	unsigned int end_addr;
	RTK_IB_REGION_MODE region_mode;
	unsigned int slice_size;
}RTK_IB_REGION;

#ifdef ALIGNED_UP
#undef ALIGNED_UP
#endif
#ifdef ALIGNED_DOWN
#undef ALIGNED_DOWN
#endif
#define ALIGNED_UP(x, y)                ((((x) + (y) - 1) / (y)) * (y))
#define ALIGNED_DOWN(x, y)              (((x) / (y)) * (y))


extern void rtk_ib_get_region_addr(RTK_IB_REGION_INDEX idx, unsigned int *start, unsigned int *end);
extern RTK_IB_REGION_MODE rtk_ib_get_region_mode(RTK_IB_REGION_INDEX idx, unsigned int *slice_size);
extern int rtk_ib_get_region(RTK_IB_REGION_INDEX idx, RTK_IB_REGION *ddr_region);
#endif

#endif /* __RTK_QOS_CONTROL_H__ */
