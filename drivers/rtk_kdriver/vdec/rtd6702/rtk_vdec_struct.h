#ifndef _RTK_VDEC_STRUCT_H_
#define _RTK_VDEC_STRUCT_H_

#include <linux/fs.h>
#include <linux/dma-buf.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/delay.h>
#include <linux/pageremap.h>

#include <mach/rtk_platform.h>

#include <rbus/sb2_reg.h>

#include <rtk_kdriver/RPCDriver.h>
#include <rtk_kdriver/rtk_vdec.h>
#include <rtk_kdriver/rtk_vdec_svp.h>
#include <rtk_kdriver/rtk_qos_export.h>
#include <rtk_kdriver/rtk_hw_monitor.h>

#include <rtd_log/rtd_module_log.h>

#include "rtk_vdec_def.h"

/* struct */
/* Record memory use*/
typedef struct buffer_info {
	struct list_head buffer_list;
	unsigned long    vir_addr;         //virtual address
	unsigned long    phy_addr;         //physical address
	unsigned long    request_size;     //request size
	unsigned long    malloc_size;      //kernel malloc size
	unsigned long    is_md_memory;     //is md memory
	pid_t            task_pid;
} buffer_info_t;

typedef struct memory_info {
	struct list_head memory_list;
	unsigned long    dcu;              //allocate from dcu
	unsigned long    phy_addr;         //physical address
	unsigned long    request_size;     //request size
	unsigned long    malloc_size;      //kernel malloc size
	unsigned int     protected;        //is protected
	pid_t            task_pid;
} memory_info_t;

typedef struct dmabuf_info {
	struct list_head dmabuf_list;
	struct dma_buf   *dma_buf_ptr;     //dma buffer ptr
	struct file      *filp;            //file ptr
	int              fd;               //dma buffer fd
	unsigned long    phy_addr;         //physical address
	int              ref_cnt;          //dma buffer ref cnt in vdec
	int              ref_status;       //dma buffer ref cnt in vdec
	pid_t            task_pid;
} dmabuf_info_t;

typedef struct frmbuf_info {
	struct list_head frmbuf_list;
	struct dma_buf   *frmbuf_ptr;      //dma buffer ptr
	unsigned long    phy_addr;         //physical address
	unsigned long    hdr_phy_addr;     //physical address
} frmbuf_info_t;

/* IOCTL return structure */
typedef struct ret_info {
	unsigned long private_info[16];
} ret_info_t;

typedef struct vbm_frm_mm {
	/* for frame base allocate memory use */
	unsigned int memType;
	unsigned int memLuSize;
	unsigned int memChSize;
	unsigned int memHdrSize;
	unsigned int memCnt;
	unsigned int memLuAddr[FRAME_ARRAY_SIZE];
	unsigned int memLuHdrAddr[FRAME_ARRAY_SIZE];
	unsigned int memChAddr[FRAME_ARRAY_SIZE];
	unsigned int memChHdrAddr[FRAME_ARRAY_SIZE];

	/* for frame base free memory use */
	unsigned int memFreeLuAddr[FRAME_ARRAY_SIZE];
	unsigned int memFreeLuHdrAddr[FRAME_ARRAY_SIZE];
	unsigned int memFreeChAddr[FRAME_ARRAY_SIZE];
	unsigned int memFreeChHdrAddr[FRAME_ARRAY_SIZE];

	/* for frame base check allocate/free status */
	unsigned int memCheck;
} vbm_frm_mm_t;

/* debug use*/
typedef struct VDEC_SHARE_MEM_STRUCT {
	unsigned int isr[10];
	unsigned int isr_done[10];
	unsigned int epc[10];
	unsigned int now ;
	unsigned int isr_vector[10];
	unsigned int isr_in_time[10];
	unsigned int isr_out_time[10];
	unsigned int show_ct_no_crash;
	unsigned int video_is_crash;
	unsigned int isr_in_sb2[10];
	unsigned int isr_out_sb2[10];

	/* for frame base use */
	struct vbm_frm_mm frm_mm[8];
} VDEC_SHARE_MEM_STRUCT;

enum vbm_by_frame_req_type {
	VBM_REQ_TYPE_4K,
	VBM_REQ_TYPE_2K
};

enum vbm_by_frame_check_type {
	VBM_CHECK_TYPE_EMPTY,
	VBM_CHECK_TYPE_ALLOC_START,
	VBM_CHECK_TYPE_ALLOC_DOING,
	VBM_CHECK_TYPE_ALLOC_FINISH,
	VBM_CHECK_TYPE_ALLOC_FAIL,
	VBM_CHECK_TYPE_FREE_START
};

typedef struct VDEC_SVPMEM_LIST {
	unsigned int addr ;
	unsigned int size ;
	unsigned int used ;
} VDEC_SVPMEM_LIST;

typedef struct VDEC_SVPMEM_USED_LIST {
	pid_t pid ;
	unsigned int index;
	unsigned int depth;
	unsigned int addr ;
	unsigned int size ;
	unsigned int prot ;
} VDEC_SVPMEM_USED_LIST;

typedef struct VDEC_IBBMEM_LIST {
	pid_t pid ;
	unsigned int used;
	unsigned int addr ;
	unsigned int size ;
} VDEC_IBBMEM_LIST;


/* SB2 monitor */
typedef struct sb2_dbg_param {
        int set_id; // which set to be use,range 1-8
        unsigned int cpu_flag[7]; // which cpu type to be set,0:all 1:a v:2 s:3 v2:4 k:5
        unsigned int start; // dbg start address,phy addr
        unsigned int end; // dbg end address,phy addr
        unsigned char operation; // the main operation of sb2 cmd,0:monitor 1:clear
        unsigned char rw;  // read & write monitor flag
        unsigned char str; // STR support flag
        unsigned int bit_mask; // bit mask param
} sb2_dbg_param;

/* mm function */
void rtkvdec_mm_init              (void);
void rtkvdec_svp_memory_init      (void);
void rtkvdec_InbandBuf_init       (void);
void rtkvdec_VMM_memory_init      (void);
void rtkvdec_VBM_memory_init      (void);
void rtkvdec_vbm_by_frame_init    (void);
void rtkvdec_show_svp_status      (void);
void rtkvdec_show_ibbuf_status    (void);
void rtkvdec_show_vmm_status      (void);
void rtkvdec_show_vbm_status      (void);
void rtkvdec_vbm_by_frame_summary (void);
bool rtkvdec_query_svp_protect    (void);
int  rtkvdec_alloc                (int size);
int  rtkvdec_free                 (unsigned int phy_addr);
int  rtkvdec_cobuffer_alloc       (VDEC_COBUF_TYPE type, int port);
int  rtkvdec_cobuffer_free        (VDEC_COBUF_TYPE type, unsigned int phy_addr);
int  rtkvdec_svp_alloc            (VDEC_SVPMEM_STRUCT *svpmem, unsigned int cmd);
int  rtkvdec_svp_free             (VDEC_SVPMEM_STRUCT *svpmem);
int  __rtkvdec_svp_free           (int index);
void rtkvdec_svp_free_by_pid      (unsigned long pid);
void rtkvdec_ibbuf_free_by_pid    (unsigned long pid);
unsigned long rtkvdec_vmm_alloc   (unsigned long size);
unsigned long rtkvdec_vmm_free    (unsigned long addr);
void rtkvdec_vmm_destroy          (void);
unsigned long rtkvdec_vbm_alloc   (unsigned long size);
unsigned long rtkvdec_vbm_free    (unsigned long addr);
#ifdef CONFIG_VBM_HEAP
void rtkvdec_vbm_free_by_chunk    (unsigned long phy_addr);
int  rtkvdec_vbm_free_by_frame    (int vcpu_id);
int  rtkvdec_vbm_alloc_by_chunk   (int size);
int  rtkvdec_vbm_alloc_by_frame   (int NumofUse);
unsigned long rtkvdec_get_decimate_addr (void);

void rtkvdec_vbm_by_frame_testcase1 (void);
void rtkvdec_vbm_by_frame_testcase2 (void);
#endif
unsigned long rtkvdec_remote_malloc (unsigned long para1, unsigned long para2);
unsigned long rtkvdec_remote_free   (unsigned long para1, unsigned long para2);

/* utils function */
void rtkvdec_send_sharedmemory       (void);
void rtkvdec_show_sharedmemory       (void);
void rtkvdec_show_calltrace          (void);
unsigned long rtkvdec_set_command    (unsigned long para1, unsigned long para2);
void memory_debug_protect            (unsigned int idx, unsigned int addr, unsigned int size);
void memory_debug_unprotect          (unsigned int idx);
int  rtkvdec_query_num_of_vcpu       (void);

#endif
