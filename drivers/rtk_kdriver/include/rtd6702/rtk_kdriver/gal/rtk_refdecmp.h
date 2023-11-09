#ifndef _RTK_GPU_DECMP_H__
#define _RTK_GPU_DECMP_H__


#define RTK_ENABLE_DECMP_INTERRUPT

  //[FIXME] don't check other IP
#define GDECMP_ENABLE_BRINGUP


#define GDECMP_ENABLE_VERIFY

//#define GDECMP_ENABLE_VERIFY_DEBUG


//#define RTK_ENABLE_DECMP_RECORD_DEBUG

//#include <mach/io.h>
#include <rtk_kdriver/io.h>
#include <linux/ktime.h>



#if 0
//64 bit os
//don't use ulong
typedef unsigned int DECMP_ADDR;
#define DECMPADDR_PRTYPE "x"


typedef unsigned int DECMP_GADDR;
#define DECMP_GADDR_PRTYPE "x"

#else 
// > 4G ?
//don't use ulong

typedef unsigned int DECMP_ADDR;
#define DECMPADDR_PRTYPE "x"


typedef u64 DECMP_GADDR;
#define DECMP_GADDR_PRTYPE "llx"


#endif//


#if 1

#include "rbus/gpu_ref_decmp_wrap_reg.h"
#include "rbus/gw_reg.h"
#include "rbus/dc_sys_reg.h"


//DataAddr -> set to VEDataAddr

#define GPU_decmp_ctrl_reg   (GPU_REF_DECMP_WRAP_GPU_decmp_ctrl_reg)
#define GPU_decmp_region_reg ( GPU_REF_DECMP_WRAP_GPU_decmp_region_0_reg )
#define GPU_decmp_lu_0       (GPU_REF_DECMP_WRAP_GPU_decmp_lu_0_0_reg)
	#define GPU_decmp_lu_hdrStart_reg (GPU_decmp_lu_0)

#if 0
#define GPU_decmp_lu_1       (GPU_REF_DECMP_WRAP_GPU_decmp_lu_1_0_reg )
	#define GPU_decmp_lu_dataStart_reg (GPU_decmp_lu_1)
#else
  #define GPU_decmp_lu_GPUdataStart_reg (GPU_REF_DECMP_WRAP_GPU_decmp_lu_1_0_reg)

#endif//

#define GPU_decmp_lu_2       (GPU_REF_DECMP_WRAP_GPU_decmp_lu_2_0_reg )
	#define GPU_decmp_lu_dataEnd_reg (GPU_decmp_lu_2)
#define GPU_decmp_lu_3       (GPU_REF_DECMP_WRAP_GPU_decmp_lu_3_0_reg )
#define GPU_decmp_lu_4       (GPU_REF_DECMP_WRAP_GPU_decmp_lu_4_0_reg )
	#define GPU_decmp_lu_cmpPitch_reg       (GPU_REF_DECMP_WRAP_GPU_decmp_lu_4_0_reg )
	
#define GPU_decmp_lu_5       (GPU_REF_DECMP_WRAP_GPU_decmp_lu_5_0_reg )
	#define GPU_decmp_lu_memPitch_reg       (GPU_REF_DECMP_WRAP_GPU_decmp_lu_5_0_reg )


#define GPU_decmp_lu_VEdataStart_reg (GPU_REF_DECMP_WRAP_GPU_decmp_lu_6_0_reg)


#define GPU_decmp_status_reg (GPU_REF_DECMP_WRAP_GPU_decmp_status_reg)

#define GPU_decmp_ch_0       (GPU_REF_DECMP_WRAP_GPU_decmp_ch_0_0_reg)
	#define GPU_decmp_ch_hdrStart_reg (GPU_decmp_ch_0)

#if 0
#define GPU_decmp_ch_1       (GPU_REF_DECMP_WRAP_GPU_decmp_ch_1_0_reg)
	#define GPU_decmp_ch_dataStart_reg (GPU_decmp_ch_1)
#else

#define GPU_decmp_ch_GPUdataStart_reg (GPU_REF_DECMP_WRAP_GPU_decmp_ch_1_0_reg)

#endif//
	
#define GPU_decmp_ch_2       (GPU_REF_DECMP_WRAP_GPU_decmp_ch_2_0_reg)
	#define GPU_decmp_ch_dataEnd_reg (GPU_decmp_ch_2)

#define GPU_decmp_ch_3       (GPU_REF_DECMP_WRAP_GPU_decmp_ch_3_0_reg)
#define GPU_decmp_ch_4       (GPU_REF_DECMP_WRAP_GPU_decmp_ch_4_0_reg)
	#define GPU_decmp_ch_cmpPitch_reg       (GPU_REF_DECMP_WRAP_GPU_decmp_ch_4_0_reg)
#define GPU_decmp_ch_5       (GPU_REF_DECMP_WRAP_GPU_decmp_ch_5_0_reg)
	#define GPU_decmp_ch_memPitch_reg       (GPU_REF_DECMP_WRAP_GPU_decmp_ch_5_0_reg)

#define GPU_decmp_ch_VEdataStart_reg (GPU_REF_DECMP_WRAP_GPU_decmp_ch_6_0_reg)


#define GPU_wrap_int_status_0	(GPU_REF_DECMP_WRAP_GPU_wrap_int_status_0_reg)
#define GPU_wrap_int_status_1	(GPU_REF_DECMP_WRAP_GPU_wrap_int_status_1_reg)
#define GPU_wrap_int_status_2	(GPU_REF_DECMP_WRAP_GPU_wrap_int_status_2_reg)
#define GPU_decmp_fifo_status	(0xB8069B00)
//#define GPU_WRAPPER_outstand_num_reg (0xB810E030)
#define GPU_WRAPPER_outstand_num_reg (GW_outstand_num_reg)


#define TVSB3_MUX_reg (0xB8007480)


#else



#define GPU_decmp_ctrl_reg   (0xB8069000)
#define GPU_decmp_region_reg (0xB8069100)
#define GPU_decmp_lu_0       (0xB8069200)
#define GPU_decmp_lu_1       (0xB8069240)
#define GPU_decmp_lu_2       (0xB8069280)
#define GPU_decmp_lu_3       (0xB80692C0)
#define GPU_decmp_lu_4       (0xB8069300)
#define GPU_decmp_lu_5       (0xB8069340)


#define GPU_decmp_status_reg (0xB8069800)

#define GPU_decmp_ch_0       (0xB8069400)
#define GPU_decmp_ch_1       (0xB8069440)
#define GPU_decmp_ch_2       (0xB8069480)
#define GPU_decmp_ch_3       (0xB80694C0)
#define GPU_decmp_ch_4       (0xB8069500)
#define GPU_decmp_ch_5       (0xB8069540)

#define GPU_wrap_int_status_0	(0xB8069A00)
#define GPU_wrap_int_status_1	(0xB8069A04)
#define GPU_wrap_int_status_2	(0xB8069A08)
#define GPU_decmp_fifo_status	(0xB8069B00)
#define GPU_WRAPPER_outstand_num_reg (0xB810E030)

#define TVSB3_MUX_reg (0xB8007480)

#endif//


#define GDECMP_CMD_TEST_DECMP_1  (0xff1)

#define GDECMP_CMD_INIT  (0x9f)
#define GDECMP_CMD_SET  (0xa0)
#define GDECMP_CMD_UNSET  (0xa1)

#define GDECMP_CMD_DISABLE  (0xa2)
#define GDECMP_CMD_SELECT_DISABLE  (0xa6)
#define GDECMP_CMD_SELECT_ENABLE  (0xa7)
#define GDECMP_CMD_SET_SIMPLE  (0xa8)
#define GDECMP_CMD_UNSET_SIMPLE  (0xa9)
#define GDECMP_CMD_DUMPSTATUS  (0xaa)
#define GDECMP_CMD_CLEAR_DUMPSTATUS  (0xab)
#define GDECMP_CMD_RECORD  (0xac)
#define GDECMP_CMD_CMP_SET  (0xb1)



#define GDECMP_CMD_ALLOC_ADDR  (0x11)


// #define GPUDECMP_ENABLE_UNIT_TEST 
//#define DECMP_PRE_SET_VALUE 1

//#ifdef DECMP_PRE_SET_VALUE
#if 0
 #define DECMP_CHECK_VALUE(addr, val) \
    do { \
		pr_err("hope rtd_outl(0x%08x, 0x%08x);\n", addr-0xA0000000, val); \
		if( rtd_inl(addr) != val ) { \
			pr_err("kkk addr: 0x%08x value not same 0x%08x != 0x%08x\n", addr, rtd_inl(addr), val); \
		} \
    } while(0)

#else

	#define DECMP_CHECK_VALUE(addr, val)

#endif//


#if 0

#define DECMP_RTDOUTL_GPU rtd_outl

#elif 1
#define DECMP_RTDOUTL_GPU(addr, val) \
    do { \
        pr_err("rtd_outl(0x%08x, 0x%08x);\n", addr-0xA0000000, val); \
        rtd_outl(addr, val); \
        pr_err("\t addr: 0x%08x, got 0x%08x   %s %d\n", addr-0xA0000000, rtd_inl(addr), __FUNCTION__, __LINE__ ); \
    } while(0)



#endif//

#if 1

#define DECMP_OUTL_2 rtd_outl

#elif 1
#define DECMP_OUTL_2(addr, val) \
    do { \
        pr_err("rtd_outl(0x%08x, 0x%08x);\n", addr-0xA0000000, val); \
        rtd_outl(addr, val); \
        pr_err("\t addr: 0x%08x, got 0x%08x   %s %d\n", addr-0xA0000000, rtd_inl(addr), __FUNCTION__, __LINE__ ); \
    } while(0)

#else
  #define DECMP_OUTL_2 rtd_outl

#endif//


//#ifdef DECMP_DISABLE_ALL_LOG
#if 1

#define DECMP_OUTL rtd_outl


#elif defined(CONFIG_ARCH_RTK287O)

#define DECMP_OUTL(addr, val) \
    do { \
        pr_err("rtd_outl(0x%08x, 0x%08x);\n", addr-0xA0000000, val); \
    } while(0)



#elif 1
#define DECMP_OUTL(addr, val) \
    do { \
        pr_err("rtd_outl(0x%08x, 0x%08x);\n", addr-0xA0000000, val); \
        rtd_outl(addr, val); \
        pr_err("\t addr: 0x%08x, got 0x%08x   %s %d\n", addr-0xA0000000, rtd_inl(addr), __FUNCTION__, __LINE__ ); \
    } while(0)

#else
  #define DECMP_OUTL rtd_outl

#endif//

#define LOCAL_INL rtd_inl


#define DECMP_NUM_REGIONS (12)


#define REGION_HISTORY_COUNT (10)

#define REGION_INVALID_INDEX (-1)

#define REGION_USED_INDEX (0xff)

#define DECMP_PRINT(level,fmt, arg...) do { \
       if( unlikely(gDebugDecmp_loglevel >= level) ) { pr_crit("gDecmp TT:%lld "  fmt, ktime_to_us(ktime_get()), ##arg ); } \
   } while(0)


//#define DECMP_LOG_DEBUG pr_debug

#define DECMP_LOG_DEBUG pr_err

#if 1
#define DECMP_LOG_MEM pr_err
#else

#define DECMP_LOG_MEM pr_debug

#endif


typedef struct _RTK_DECMP_REGION_RECORD
{

	unsigned int set_time;
	unsigned int unset_time;
	
} RTK_DECMP_REGION_RECORD;


typedef struct _RTK_DECMP_REGION_ITEM_STATUS
{

	unsigned int region_idx;
	int rec_wp;
	int rec_rp;
	RTK_DECMP_REGION_RECORD rec[REGION_HISTORY_COUNT] ;
	
} RTK_DECMP_REGION_ITEM_STATUS ;


typedef struct _RTK_DECMP_REGION_TABLE_STATUS
{
	int max_region_idx; //soft constrain mode
	

	RTK_DECMP_REGION_ITEM_STATUS regions[DECMP_NUM_REGIONS];
	
} RTK_DECMP_REGION_TABLE_STATUS ; 


void decmp_region_log_settime(int idx);
void decmp_region_log_unsettime(int idx);



//NEED sync with um
typedef struct _rtk_decmp_export
{
	//VE
	DECMP_ADDR data_y_addr;
	unsigned int data_y_size;
	DECMP_ADDR data_uv_addr;
	unsigned int data_uv_size;

	DECMP_ADDR header_y_addr;
	unsigned int header_y_size;
	DECMP_ADDR header_uv_addr;
	unsigned int header_uv_size;

	//gpu
	DECMP_GADDR data_y_addr_gpu;
	unsigned int    data_y_size_gpu;
	DECMP_GADDR data_uv_addr_gpu;
	unsigned int    data_uv_size_gpu;


	unsigned int width;
	unsigned int height;
	unsigned int pitch;
	unsigned int region_idx;

	unsigned int cmp_pitch;
	unsigned int mem_pitch;
	unsigned int refresh;

	// 0(no compress), 50, 70
	unsigned int compr_ratio;
} rtk_decmp_export;


typedef struct _rtk_decmp_export_simple
{
	int version;
	int fd;
	DECMP_ADDR address;
}  rtk_decmp_export_simple;


typedef struct _rtk_decmp_atachAddr
{
	int version;
	int fd;
	unsigned int size;
	DECMP_ADDR ret_addr;
	unsigned int size_2;
	DECMP_ADDR ret_addr_2;
}  rtk_decmp_atachAddr;


int Gpu_Set_Dec_init(int enable_intr, void* dev );
int Gpu_Set_Dec_deInit(void* dev );


int Gpu_Set_Dec_Set(unsigned long arg );
int Gpu_Set_Dec_UnSet(unsigned long arg );
int GpuDec_Record(unsigned long arg);


int GpuDec_Alloc_Addr(unsigned long arg);

int GpuDec_Set_Simple(rtk_decmp_export_simple* info);
int GpuDec_UnSet_Simple(rtk_decmp_export_simple* info,  int* ret_region_idx);


int GpuDec_surface_free( unsigned long _addr, DECMP_GADDR g_addr, void* param);



int GpuDec_Disable(unsigned long arg );
int GpuDec_DisableRegion(unsigned long arg_mask);


void Gpu_GpuDec_Select_enable(int flag);
	
int GpuDec_DumpStatus(unsigned long arg,int clear);

int  Gpu_Set_Dec_test_RGBA(unsigned long arg);


int GPU_CMP_Set(unsigned long arg);


int GpuDecUtil_get_region_by_dataAddr(DECMP_ADDR address, int* ret_idx);
void GpuDecUtil_dump_exportInfo(rtk_decmp_export* info);



//static inline DECMP_ADDR_TYPE get_dataAddr_by_region(int idx)
static inline DECMP_ADDR get_VEdataAddr_by_region(int idx)
{

	return rtd_inl(GPU_decmp_lu_VEdataStart_reg+idx*4);

}


static inline DECMP_GADDR get_GPUdataAddr_by_region(int idx)
{
	return rtd_inl(GPU_decmp_lu_GPUdataStart_reg+idx*4);
}


static inline DECMP_GADDR get_dataEndGpuAddr_by_region(int idx)
{

	return rtd_inl(GPU_decmp_lu_dataEnd_reg+idx*4);
}


#endif// _RTK_GPU_DECMP_H__

