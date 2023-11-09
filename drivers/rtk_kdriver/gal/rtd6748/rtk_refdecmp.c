
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/irq.h>

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/uaccess.h>/*copy_from_user*/

#include <linux/delay.h>


#include <linux/pageremap.h>
#include <mach/rtk_platform.h>
#include <mach/platform.h>

//#include <linux/mod_devicetable.h> // for of_device_id
#include <linux/of_irq.h>

#include <linux/ion.h>


#include <gal/rtk_refdecmp.h>
#include "rtk_refdecmp_ver.h"


#if IS_ENABLED(CONFIG_DMABUF_HEAPS_DVR)


#endif//


// ch_cmp_sta_adr


#if !IS_ENABLED(CONFIG_ION_DVR_HEAP)

	#define DECMP_USE_DMA_HEAP

#else
	
#endif//


#if IS_ENABLED(DECMP_USE_DMA_HEAP) && !IS_ENABLED(CONFIG_DMABUF_HEAPS_DVR)
 #error "kernel config error for gdecmp compile" 

#endif//



//os device relates 
#define RTKREFDECMP_NAME			"refdecmp"
static struct class *refdecmp_class;
int refdecmp_major = 0; // dynamic
int refdecmp_minor = 0;


const unsigned int debug_regs[] = { GPU_decmp_ctrl_reg,  GPU_decmp_status_reg, GPU_REF_DECMP_WRAP_GPU_decmp_warp_status_reg, GPU_wrap_int_status_0, 
	GPU_wrap_int_status_1, GPU_wrap_int_status_2,  TVSB3_MUX_reg };

const unsigned int debug_regs_per_set[] = { GPU_decmp_region_reg, 
	GPU_decmp_lu_0, GPU_decmp_lu_VEdataStart_reg, GPU_decmp_ch_VEdataStart_reg,  GPU_decmp_lu_GPUdataStart_reg, GPU_decmp_ch_GPUdataStart_reg, 
	GPU_decmp_lu_2, GPU_decmp_lu_3, GPU_decmp_lu_4, GPU_decmp_lu_5, 
    GPU_decmp_ch_0,  GPU_decmp_ch_2, GPU_decmp_ch_3, GPU_decmp_ch_4, GPU_decmp_ch_5 };
	

//0x780 == 1920
#define GPU_decmp_1920_w     (0x780)
#define GPU_decmp_buf_8MB    (1920*1080*4)

#define GPU_decmp_buf_NV12_1080p    (1920*1080*1.5)


#define GPU_decmp_header_addr  (0x10000000)
#define GPU_decmp_data_addr    0x11000000

#define GPU_decmp_ch_header_addr  0x20000000
#define GPU_decmp_ch_data_addr    0x21000000



unsigned int g_decmpInit = 0;

typedef struct 
{
	int region_enable_by_ve;
	int region_disable_by_ve;
	int irq_no;

} GPUDecCONFIG;


/*
0: disable
1: fatal logs
..
10: print all logs

*/
int gDebugDecmp_loglevel = 1;


extern int gEnableDecmpError_DUMP ;

GPUDecCONFIG gDecConfig;

static unsigned int g_disableAllRegion_usec = 0;

static RTK_DECMP_REGION_TABLE_STATUS gRegionStatus;

// usleep_range(1 * 1000, 1 * 1000);
#define OUTSTANDING_SLEEP  \
    do { \
        udelay(10); \
    } while(0)

#define DECOMP_ERR_DELAY_USEC ( 10 )
#define DECOMP_CHECK_ERR_DELAY_USEC ( 10 )

static int g_gpu_switch_outstanding = 1;

#define MAX_TRAFFIC_STR (32)
unsigned int xDumpTraffic(const char* str, int show_err);



	


#ifdef CONFIG_OF

static int gdecmp_probe(struct platform_device *pdev);


#ifdef CONFIG_PM
static int gdecmp_suspend(struct device *p_dev)
{
   pr_debug(" %s %d\n", __FUNCTION__, __LINE__);
   
	return 0;
}

static int gdecmp_resume(struct device *p_dev)
{

	pr_debug(" %s %d\n", __FUNCTION__, __LINE__);
	

	return 0;
}

#ifdef CONFIG_HIBERNATION
static int gdecmp_suspend_std(struct device *p_dev)
{
	pr_debug(" %s %d\n", __FUNCTION__, __LINE__);


	return 0;
}

static int gdecmp_resume_std(struct device *p_dev)
{
	pr_debug(" %s %d\n", __FUNCTION__, __LINE__);

	return 0;
}
#endif//CONFIG_HIBERNATION

#endif//CONFIG_PM


static const struct dev_pm_ops gdecmp_pm_ops = {
	.suspend    = gdecmp_suspend,
	.resume     = gdecmp_resume,
#ifdef CONFIG_HIBERNATION
	.freeze     = gdecmp_suspend_std,
	.thaw       = gdecmp_resume_std,
	.poweroff   = gdecmp_suspend_std,
	.restore    = gdecmp_resume_std,
#endif
};


static const struct of_device_id gdecmp_of_match[] = {
	{
		.compatible = "realtek,gdecmp"
	},
	{},
};


MODULE_DEVICE_TABLE(of, gdecmp_of_match);



static struct platform_driver gdecmp_driver = {
	.probe			= gdecmp_probe,
	.driver = {
		.name         = RTKREFDECMP_NAME,
		.of_match_table = gdecmp_of_match,
		.bus          = &platform_bus_type,
#ifdef CONFIG_PM
		.pm           = &gdecmp_pm_ops,
#endif

	},
};

static struct platform_device *gdecmp_devs;





#endif // CONFIG_OF


#ifdef GDECMP_ENABLE_BRINGUP

inline u64 getVcpuClock(void)
{
	return  ktime_to_us( ktime_get() );
}

#else
inline u64 getVcpuClock(void)
{
	unsigned int hi=0 , low= 0 ;

	do
	{
		low = rtd_inl(TIMER_VCPU_CLK90K_LO_reg);
		hi  = rtd_inl(TIMER_VCPU_CLK90K_HI_reg);
	} while ( hi != rtd_inl(TIMER_VCPU_CLK90K_HI_reg) ) ;

	return (((u64)hi << 32) | low);
}


#endif//

void GpuDecUtil_dump_exportInfo(rtk_decmp_export* info)
{
	printk(KERN_ERR"info addr: 0x%"DECMPADDR_PRTYPE" 0x%"DECMPADDR_PRTYPE " 0x%" DECMPADDR_PRTYPE " 0x%"DECMPADDR_PRTYPE 
		" w/h/stride [%d %d %d %d] "
		" size: %d %d %d %d idx:%d\n",
		info->data_y_addr, info->data_uv_addr, info->header_y_addr, info->header_uv_addr, 
		info->width, info->height, info->pitch, info->cmp_pitch,
		info->data_y_size, info->data_uv_size, info->header_y_size, info->header_uv_size,
		info->region_idx);
		
}


int GpuDecUtil_get_region_by_dataAddr(DECMP_ADDR address, int* ret_idx)
{
	int index=0;
	//unsigned int region_reg;
	//unsigned int data;
	DECMP_ADDR data_addr = 0;
	int ret = -1;

	*ret_idx = -1;
	
	for( ; index< DECMP_NUM_REGIONS; index++)
	{
		data_addr = LOCAL_INL(GPU_decmp_lu_VEdataStart_reg+index*4 );

		if( data_addr == address) 
		{
			*ret_idx = index;
			ret = 0;
			break;
		}
	#if 0		
		region_reg = GPU_decmp_region_reg+ index*4;
		data = LOCAL_INL(region_reg);
		enable = GPU_REF_DECMP_WRAP_GPU_decmp_region_0_get_enable(data);
	#endif//
	
	}



	return ret;
}
	


/*
example:
  1920*1080 lossless

*/
void Gpu_Set_ch_Dec_Region(int index,unsigned int ch_hdr_addr,unsigned int ch_data_addr,
								DECMP_GADDR ch_gaddr, 
						unsigned int ch_data_size,unsigned int ch_data_pitch,
						unsigned int ch_cmp_pitch,unsigned int ch_mem_pitch)
{
	unsigned int gaddr = 0;
	pr_err("in %s %d\n", __FUNCTION__, __LINE__);

			DECMP_CHECK_VALUE(GPU_decmp_ch_0+index*4, ch_hdr_addr);
    //Compression header start address (unit: 16-byte)
	DECMP_OUTL(GPU_decmp_ch_0+index*4, ch_hdr_addr);
	
			DECMP_CHECK_VALUE(GPU_decmp_ch_VEdataStart_reg+index*4, ch_data_addr);
	//Uncompressed and compressed data start
	DECMP_OUTL(GPU_decmp_ch_VEdataStart_reg+index*4, ch_data_addr);
	
	 gaddr = (unsigned int)ch_gaddr;
	DECMP_OUTL( GPU_decmp_ch_GPUdataStart_reg+index*4, gaddr);
	
			DECMP_CHECK_VALUE(GPU_decmp_ch_2+index*4, ch_data_addr+ch_data_size);	
	//Uncompressed data end address (unit: 16-byte)
	DECMP_OUTL(GPU_decmp_ch_2+index*4, gaddr + ch_data_size);
	
			DECMP_CHECK_VALUE(GPU_decmp_ch_3+index*4, ch_data_pitch);
	//Uncompressed data pitch (unit: 16-byte) 
	DECMP_OUTL(GPU_decmp_ch_3+index*4, ch_data_pitch);
	
			DECMP_CHECK_VALUE(GPU_decmp_ch_4+index*4, ch_cmp_pitch);
	//Compressed data pitch 
	DECMP_OUTL(GPU_decmp_ch_4+index*4, ch_cmp_pitch);
	
			DECMP_CHECK_VALUE(GPU_decmp_ch_5+index*4, ch_mem_pitch);
	//Dram pitch of compressed data (unit: 16-byte)
	DECMP_OUTL(GPU_decmp_ch_5+index*4, ch_mem_pitch);

	pr_err("exit %s %d\n", __FUNCTION__, __LINE__);	
	return;
}

void Gpu_Set_Dec_Region(int index,unsigned int hdr_addr,unsigned int data_addr,
									DECMP_GADDR lu_gaddr, 
						unsigned int data_size,unsigned int pic_width,
						unsigned int data_pitch,unsigned int cmp_pitch,
						unsigned int mem_pitch, unsigned int compr_rate)
{
	unsigned int gaddr=0;

		
		gpu_ref_decmp_wrap_gpu_decmp_region_RBUS decmp_region_reg;
		decmp_region_reg.regValue = LOCAL_INL(GPU_decmp_region_reg+index*4);

		decmp_region_reg.pic_width_div_16 = (pic_width/16);

	// 100 -> lossy 100
	// 0 -> loseless
	if( compr_rate != 0) 
	{
			decmp_region_reg.lossy_en = 1;
	}

			decmp_region_reg.decomp_qlvl_y = 1;
			decmp_region_reg.decomp_qlvl_c = 1;
			
		DECMP_CHECK_VALUE(GPU_decmp_region_reg+index*4, decmp_region_reg.regValue );
	DECMP_OUTL(GPU_decmp_region_reg+index*4, decmp_region_reg.regValue);



			DECMP_CHECK_VALUE(GPU_decmp_lu_0+index*4, hdr_addr);
    //Compression header start address (unit: 16-byte)
	DECMP_OUTL(GPU_decmp_lu_0+index*4, hdr_addr);
	
			DECMP_CHECK_VALUE(GPU_decmp_lu_VEdataStart_reg+index*4, data_addr);
	//compressed data start
	DECMP_OUTL(GPU_decmp_lu_VEdataStart_reg+index*4, data_addr);


	gaddr = (unsigned int)lu_gaddr;
	DECMP_OUTL( GPU_decmp_lu_GPUdataStart_reg+index*4, gaddr);
	
			DECMP_CHECK_VALUE(GPU_decmp_lu_2+index*4, data_addr+data_size);	
	//Uncompressed data end address (unit: 16-byte)
	DECMP_OUTL(GPU_decmp_lu_2+index*4, gaddr + data_size);
	
			DECMP_CHECK_VALUE(GPU_decmp_lu_3+index*4, data_pitch);
	//Uncompressed data pitch (unit: 16-byte)
	DECMP_OUTL(GPU_decmp_lu_3+index*4, data_pitch);
	
			DECMP_CHECK_VALUE(GPU_decmp_lu_4+index*4, cmp_pitch);
	//Compressed data pitch 
	DECMP_OUTL(GPU_decmp_lu_4+index*4, cmp_pitch);
	
			DECMP_CHECK_VALUE(GPU_decmp_lu_5+index*4, mem_pitch);	
	//Dram pitch of compressed data (unit: 16-byte)
	DECMP_OUTL(GPU_decmp_lu_5+index*4, mem_pitch);
	
	return;
}


void Gpu_GpuDec_Select_enable(int flag)
{
	unsigned int data=0;
	unsigned int sel_reg = GPU_decmp_ctrl_reg;

	static unsigned int debug_outstanding_val = 0;
	unsigned int org_standing_val = LOCAL_INL(GPU_WRAPPER_outstand_num_reg);

	if( debug_outstanding_val == 0 ) {
		debug_outstanding_val = org_standing_val;
	}

	if( g_gpu_switch_outstanding ) {
		DECMP_OUTL(GPU_WRAPPER_outstand_num_reg, 0);
		mdelay(1);
	}

	data = LOCAL_INL(sel_reg);

	
	if(flag)
	{
		data |= (1<<0);
	}else
	{
		data &= ~(1<<0);
	}

	DECMP_OUTL(sel_reg, data);

	if( g_gpu_switch_outstanding ) {
	  DECMP_OUTL(GPU_WRAPPER_outstand_num_reg, org_standing_val);
	}




	
	return ;
}


void Gpu_Set_Dec_Region_enable(int index,int flag, int all_regions)
{
	unsigned int org_standing_val;
	unsigned int enabled = 0;
	int i=0;
	//char tmp_buf[MAX_TRAFFIC_STR];
	
	unsigned int data=0;
	unsigned int org_data=0;
	int is_write_reg = 0;
	//unsigned int mask_io;
	unsigned int region_reg = GPU_decmp_region_reg+index*4;

	unsigned int data_addr = rtd_inl(GPU_decmp_lu_VEdataStart_reg+index*4);
	unsigned int data_end_addr = get_dataEndGpuAddr_by_region(index);

	data = org_data = LOCAL_INL(region_reg);
	enabled = GPU_REF_DECMP_WRAP_GPU_decmp_region_0_get_enable(data);

	#ifdef DECMP_PRE_SET_VALUE
		//for pre set test, assume enabled=0(don't do disable->enable flow)
		enabled = 0;
	#endif//

if( g_gpu_switch_outstanding )
{
    org_standing_val = LOCAL_INL(GPU_WRAPPER_outstand_num_reg);
	DECMP_OUTL_2(GPU_WRAPPER_outstand_num_reg, 0);

	  //mdelay(1);  
	OUTSTANDING_SLEEP;

	//xDumpTraffic("after gpuOut to 0\n", 1);

}

	#if 0
	mask_io = LOCAL_INL(GPU_decmp_fifo_status);
	mask_io |= ( 0x00000004 );
	DECMP_OUTL_2(GPU_decmp_fifo_status, mask_io);
	#endif//

	if( all_regions )
	{
		for( i=0; i< DECMP_NUM_REGIONS; i++)
		{
			gpu_ref_decmp_wrap_gpu_decmp_region_RBUS data_reg;
			region_reg = GPU_decmp_region_reg+ i*4;
			//enable = GPU_REF_DECMP_WRAP_GPU_decmp_region_0_get_enable(data);
			data_reg.regValue = LOCAL_INL(region_reg);

			enabled = data_reg.enable;


			if( flag == 0 && enabled ) //disable
			{
				data_reg.enable = 0;

				//reset err
				data_reg.chg_err = 0;
				data_reg.bl_err = 0;
				data_reg.busy = 0;
				data_reg.cross_err = 0;
				
				
				DECMP_OUTL(region_reg, data_reg.regValue);
			}
			else if (flag == 1 && enabled == 0 )
			{
				//enable
				//data |= GPU_REF_DECMP_WRAP_GPU_decmp_region_0_enable_mask;
				data_reg.enable = 1;
				
				DECMP_OUTL(region_reg, data_reg.regValue);
			}
	

			
		}

		udelay(100);
	
	}
	else 
	{
		if(flag)
		{
			if( enabled ) 
			{

				#ifdef RTK_DECMP_REGIONE_STATUS_DISABLE_CHECK
					//unit test flow
					// disable first
				
					data &= ~(GPU_REF_DECMP_WRAP_GPU_decmp_region_0_enable_mask);
					DECMP_OUTL_2(region_reg, data);
					udelay(100);
				#else
					pr_err("[decmp] regionSet dup status(1->1):%d addr:0x%08x-0x%08x val:0x%08x \n", index, data_addr, data_end_addr, org_data);

				#endif//
				
			}
			else 
			{
				data |= GPU_REF_DECMP_WRAP_GPU_decmp_region_0_enable_mask;
				
				is_write_reg = 1;

				DECMP_PRINT(5, "[decmp] enableReg:%d addr:0x%08x-0x%08x t:%llx\n", index, data_addr, data_end_addr, getVcpuClock() );
			}
		
			
		}
		else
		{
			if( enabled ) {
				
				// 1 -> 0 
				data &= ~(GPU_REF_DECMP_WRAP_GPU_decmp_region_0_enable_mask);
				is_write_reg = 1;

				
				DECMP_PRINT(5,"[decmp] gr disableReg (1->0):%d addr:0x%08x-0x%08x t:%llx\n", index, data_addr, data_end_addr, getVcpuClock() );
				
			}
			else {
				// 0 -> 0
				printk(KERN_WARNING"[decmp] regionSet dup status(0->0):%d addr:0x%08x-0x%08x val:0x%08x \n", index, data_addr, data_end_addr, org_data);
			}
		

		}

		if( is_write_reg ) 
		{
				DECMP_CHECK_VALUE(region_reg, data);
			DECMP_OUTL_2(region_reg, data);

			decmp_region_log_settime(index);
		}
		
	}
	
	#if 0
	mask_io &= ~(0x00000004);
	DECMP_OUTL(GPU_decmp_fifo_status, mask_io);
	#endif//

	if( g_gpu_switch_outstanding ) {

	   //snprintf(tmp_buf, MAX_TRAFFIC_STR, "bef gpuOutSt:%d", org_standing_val );
	   
	   
	   DECMP_OUTL_2(GPU_WRAPPER_outstand_num_reg, org_standing_val);

	   OUTSTANDING_SLEEP;
	   
	}

	if( all_regions ) {
		g_disableAllRegion_usec = jiffies_to_usecs(jiffies);
		
		printk(KERN_WARNING"[decmp] log: disable all region :%u\n", g_disableAllRegion_usec);

	}
	

	
	return ;
}

int GpuDec_CheckRegionStatus(int in_isr )
{
	int i=0;
	int chg_err, cross_err, bl_err, busy;
	int enable = 0;
	int num_enabled = 0;
	//int wrap_status_err = 0;
	unsigned int region_reg;
	unsigned int data ;
	int have_error = 0;
	int decmp_status_err = 0;
	unsigned int region_enable_bitmask=0;
	
	

	data = LOCAL_INL(GPU_decmp_status_reg);
	decmp_status_err = GPU_REF_DECMP_WRAP_GPU_decmp_status_get_decomp_core_errflag_ro(data);

	if( decmp_status_err ) {
		
		pr_crit("decmp error 0x%x raw:0x%08x TT:%llx\n", decmp_status_err, data, getVcpuClock() );


		//test todo 
//		tasklet_schedule(&my_tasklet);
	}
	
	for(i=0; i< DECMP_NUM_REGIONS; i++)
	{
		region_reg = GPU_decmp_region_reg+ i*4;
		data = LOCAL_INL(region_reg);
		enable = GPU_REF_DECMP_WRAP_GPU_decmp_region_0_get_enable(data);

		#if 0
		if( ! enable )
		{
			continue;
		}
		#endif//

		if ( enable )
		{
			region_enable_bitmask |= ( 0x00000001<< i ) ;
			num_enabled++;
		}

		
		busy = GPU_REF_DECMP_WRAP_GPU_decmp_region_0_get_busy(data);
		chg_err =  GPU_REF_DECMP_WRAP_GPU_decmp_region_0_get_chg_err(data);
		cross_err =  GPU_REF_DECMP_WRAP_GPU_decmp_region_0_get_cross_err(data) ;
		bl_err =  GPU_REF_DECMP_WRAP_GPU_decmp_region_0_get_bl_err(data)  ;

		if( busy | chg_err | cross_err| bl_err )
		{
			pr_crit("regionErr %d, addr:0x%"DECMPADDR_PRTYPE " enable:%d  busy:%d chg_err:%d cross_err:%d bl_err:%d TT:%llx\n",
				i, get_VEdataAddr_by_region(i), enable ,busy, chg_err, cross_err, bl_err, getVcpuClock()  );

			//pr_err("regionReg 0x%08x idx:%d  err:0x%08x enable:%d  busy:%d chg_err:%d cross_err:%d bl_err:%d\n", 
			//	region_reg, i, 	data,  (0x00000001&(data)),busy, chg_err, cross_err, bl_err  );

			have_error = 1;
		}
		else if( enable )
		{
			pr_crit(" regoEnabled:%d addr:0x%"DECMPADDR_PRTYPE " val:0x%x\n",i, get_VEdataAddr_by_region(i), data );

		}

	}

	pr_crit(" \t enabled reg:0x%08x #:%d ",region_enable_bitmask, num_enabled);

	#ifdef GDECMP_ENABLE_VERIFY_DEBUG
	if( gEnableDecmpError_DUMP) 
	{
		static int dump_done = 0;
	
		if(dump_done ==0) {
			#ifdef RTK_ENABLE_DECMP_RECORD_DEBUG
			decmp_rec_dumpRecord();
			#endif// RTK_ENABLE_DECMP_RECORD_DEBUG
			dump_done = 1;	
		}
		
	}
	#endif//GDECMP_ENABLE_VERIFY_DEBUG
	
	#if 0
	{ //test
		static unsigned int aaa = 0;

		if( aaa == 60 )
		{	

			pr_crit(" decmp st:0x%x regions:0x%x ",decmp_status_err, region_enable_bitmask);
			aaa = 0;
		}
		aaa++;
	}
	#endif//0

	return have_error;
}


int GpuDec_CheckStatus(int in_isr )
{
	volatile unsigned int data = LOCAL_INL(GPU_decmp_status_reg);

	int decmp_status_err = 0;

	unsigned int ccc = 0;

#if 1
	int tmp_decmp_status_err = 0;
	int reset_core = 0;
	int i ;
	unsigned int region_reg;
	int is_region_enabled = 0;

	int is_region_disabled = 0;
	
//	int wrap_status_err = 0;
	int tmp_count = 0;
	unsigned int tmp_time = 0;
	unsigned int time_st, time_end;
#endif//0

	//wrap_status_err = LOCAL_INL(GPU_wrap_int_status_0) | LOCAL_INL(GPU_wrap_int_status_1) | LOCAL_INL(GPU_wrap_int_status_2);


	decmp_status_err = GPU_REF_DECMP_WRAP_GPU_decmp_status_get_decomp_core_errflag_ro(data);


#if 1
	for( i=0; i< DECMP_NUM_REGIONS; i++)
	{
		region_reg = GPU_decmp_region_reg+ i*4;
		//enable = GPU_REF_DECMP_WRAP_GPU_decmp_region_0_get_enable(data);
		data = LOCAL_INL(region_reg);
		
		if( GPU_REF_DECMP_WRAP_GPU_decmp_region_0_get_enable(data) )
		{
			is_region_enabled = 1;
			
			break;
		}
	
	}

	if( is_region_enabled == 0 )
	{
		tmp_time = jiffies_to_usecs(jiffies);
		
		//if ( (tmp_time - g_disableAllRegion_usec) > 1*1000 && g_disableAllRegion_usec != 0) 
		if ( g_disableAllRegion_usec != 0) 
		{ 
			is_region_disabled = 1;


			pr_err("decomp to clear %u %u %u %d\n", (tmp_time - g_disableAllRegion_usec), tmp_time, g_disableAllRegion_usec, 
				is_region_enabled );
		}

	}

	//if ( decmp_status_err && is_region_disabled  )
	if ( decmp_status_err )
	{
		
		//static unsigned int ccc = 0;
		data = LOCAL_INL(GPU_decmp_status_reg);
				
		//
		#if 0
		data |= GPU_REF_DECMP_WRAP_GPU_decmp_status_decomp_core_errflag_clr_mask;
		data &= ~(GPU_REF_DECMP_WRAP_GPU_decmp_status_decomp_core_errflag_ro_mask);
		DECMP_OUTL(GPU_decmp_status_reg, data);

		//data &= 0x7FFFFFFF;
		//DECMP_OUTL(GPU_decmp_status_reg, data);

		
		#endif//
		//u(1);

		data = LOCAL_INL(GPU_decmp_status_reg);
		tmp_decmp_status_err = GPU_REF_DECMP_WRAP_GPU_decmp_status_get_decomp_core_errflag_ro(data);

		if( tmp_decmp_status_err ) {

			gpu_ref_decmp_wrap_gpu_decmp_ctrl_RBUS decmp_ctrl_reg;


			data = LOCAL_INL(GPU_decmp_status_reg);

			//enable [31], force clear status
			data |= GPU_REF_DECMP_WRAP_GPU_decmp_status_decomp_core_errflag_clr_mask;
			data &= ~(GPU_REF_DECMP_WRAP_GPU_decmp_status_decomp_core_errflag_ro_mask);
			DECMP_OUTL(GPU_decmp_status_reg, data);
			udelay( DECOMP_ERR_DELAY_USEC );

			
			if( data != 0x8000ff00 ) {
				pr_err("invalid reg 0x%08x value:0x%08x\n", GPU_decmp_status_reg, data );
			}


			//check err again
			#if 1
			tmp_count = 0;
			time_st = jiffies_to_usecs(jiffies);
			while( 1 ) {
				data = LOCAL_INL(GPU_decmp_status_reg);

				time_end = jiffies_to_usecs(jiffies);

				if( GPU_REF_DECMP_WRAP_GPU_decmp_status_get_decomp_core_errflag_ro(data) == 0 ) {
					pr_err("decmp err gone. %d\n", time_end - time_st );
					break;
				}

				udelay( DECOMP_CHECK_ERR_DELAY_USEC );

				if( tmp_count++ > 1000) {
					time_end = jiffies_to_usecs(jiffies);
					pr_err("decmp still err 0x%08x . break %d\n", data, time_end -  time_st);
					break;
				}
			}
			#endif

			if( reset_core )
			{
				decmp_ctrl_reg.regValue = LOCAL_INL(GPU_decmp_ctrl_reg);
				
				decmp_ctrl_reg.decomp_core_rst = 1;
				//decmp_ctrl_reg.decomp_sel = 0; //
				DECMP_OUTL(GPU_decmp_ctrl_reg, decmp_ctrl_reg.regValue);
				udelay( DECOMP_ERR_DELAY_USEC *2 );


				decmp_ctrl_reg.decomp_core_rst = 0;
				DECMP_OUTL(GPU_decmp_ctrl_reg, decmp_ctrl_reg.regValue);
				udelay( DECOMP_ERR_DELAY_USEC );
			}

			tmp_count = 0;
			time_st = jiffies_to_usecs(jiffies);

			#if 0 //
			while( 1 ) {
				data = LOCAL_INL(GPU_decmp_status_reg);

				time_end = jiffies_to_usecs(jiffies);

				if( GPU_REF_DECMP_WRAP_GPU_decmp_status_get_decomp_core_errflag_ro(data) == 0 ) {
					pr_err("decmp2 err gone. %d\n", time_end - time_st );
					break;
				}

				udelay( DECOMP_CHECK_ERR_DELAY_USEC );

				if( tmp_count++ > 1000) {
					time_end = jiffies_to_usecs(jiffies);
					pr_err("decmp2 still err 0x%08x . break %d\n", data, time_end -	time_st);
					break;
				}
					
			}
			#endif//

			#if 0
			if( reset_core ) 
			{
					//decmp_ctrl_reg.decomp_core_rst = 1;
					decmp_ctrl_reg.decomp_core_rst = 0;
					decmp_ctrl_reg.decomp_sel = 0; //
					DECMP_OUTL(GPU_decmp_ctrl_reg, decmp_ctrl_reg.regValue);
			}
			#endif//
			


			#if 1 //disable (no clear) [31] 
			data = LOCAL_INL(GPU_decmp_status_reg);
			data = data & 0x7FFFFFFF & (~(GPU_REF_DECMP_WRAP_GPU_decmp_status_decomp_core_errflag_ro_mask));
			DECMP_OUTL(GPU_decmp_status_reg, data);
			udelay( DECOMP_ERR_DELAY_USEC );
			#endif//

			
			//read error status again.
			data = LOCAL_INL(GPU_decmp_status_reg);
			if( GPU_REF_DECMP_WRAP_GPU_decmp_status_get_decomp_core_errflag_ro(data) ) 
			{
				pr_err("A2 decmp still err 0x%08x \n", data );
			}
			else 
			{ //reset 
				g_disableAllRegion_usec = 0;
			}
			

			
		}

		

	}
#endif//

	if( ccc++ > 1000 ) {

		pr_err("decmp err 0x%08x \n", decmp_status_err );
		ccc = 0;
	}

	if( decmp_status_err )
		return 1;
	
	return 0;
}


int GpuDec_reset(void)
{
	gpu_ref_decmp_wrap_gpu_decmp_ctrl_RBUS decmp_ctrl_reg;
	decmp_ctrl_reg.regValue = LOCAL_INL(GPU_decmp_ctrl_reg);
				
	decmp_ctrl_reg.decomp_core_rst = 1;
	DECMP_OUTL(GPU_decmp_ctrl_reg, decmp_ctrl_reg.regValue);
	udelay( DECOMP_ERR_DELAY_USEC *2 );


	decmp_ctrl_reg.decomp_core_rst = 0;
	DECMP_OUTL(GPU_decmp_ctrl_reg, decmp_ctrl_reg.regValue);
	udelay( DECOMP_ERR_DELAY_USEC );



	#if 0
	//CLKEN = 0
		DECMP_OUTL(SYS_REG_SYS_CLKEN4_reg, SYS_REG_SYS_CLKEN4_clken_gdcmp_mask | 0x0);
		udelay(1000);

		//RSTN = 0
		DECMP_OUTL(SYS_REG_SYS_SRST4_reg,  SYS_REG_SYS_SRST4_rstn_gdcmp_mask | 0x0);
		udelay(1000);

		//RSTN = 1
		DECMP_OUTL(SYS_REG_SYS_SRST4_reg,  SYS_REG_SYS_SRST4_rstn_gdcmp_mask | 0x1);
		udelay(1000);

		//CLKEN = 1
		DECMP_OUTL(SYS_REG_SYS_CLKEN4_reg, SYS_REG_SYS_CLKEN4_clken_gdcmp_mask | 0x1);
		udelay(1000);
	#endif//
	return 0;	
}
				

inline unsigned int xDumpTraffic(const char* str, int show_err)
{

#if 0
	unsigned int read = 0;
	
	DECMP_OUTL(DC_SYS_DC_PC_SYSH_prog_ctrl_reg, 0xB8);
	//shoule become 0x38
	
	rtd_outl(DC_SYS_DC_PC_CTRL_reg, 0);
	udelay(10);
	rtd_outl(DC_SYS_DC_PC_CTRL_reg, 1);

	//usleep_range(5 * 1000, 5 * 1000);
	msleep(1000);

	read = rtd_inl(DC_SYS_DC_PC_SYSH_prog_0_ACK_NUM_reg);

	if(read != 0 && show_err ) {
		pr_err("jjjj DumpTraffic %s %u.\n", str, read);

	}

	return read;
#else
	return 0;

#endif//
}

void xDumpRegion(int index)
{
	
	pr_crit("Luma data_st:0x%08x(0x%08x) data_end:0x%08x gDataSize:%d h_st:0x%08x\n", 
		rtd_inl(GPU_decmp_lu_VEdataStart_reg+index*4), rtd_inl(GPU_decmp_lu_GPUdataStart_reg+index*4),
		rtd_inl(GPU_decmp_lu_2+index*4), 
		(int)(rtd_inl(GPU_decmp_lu_2+index*4) - rtd_inl(GPU_decmp_lu_GPUdataStart_reg+index*4) ),
		rtd_inl(GPU_decmp_lu_0+index*4) );

	pr_crit("Chroma data_st:0x%08x(0x%08x) data_end:0x%08x gDataSize:%d h_st:0x%08x\n", 
		rtd_inl(GPU_decmp_ch_VEdataStart_reg+index*4), rtd_inl(GPU_decmp_ch_GPUdataStart_reg+index*4),
		rtd_inl(GPU_decmp_ch_2+index*4 ),
		(int) ( rtd_inl(GPU_decmp_ch_2+index*4 ) - rtd_inl(GPU_decmp_ch_GPUdataStart_reg+index*4) ), 
		rtd_inl(GPU_decmp_ch_0+index*4) );

	pr_crit("Luma data_pitch:%d cmp_pitch:%d mem_pitch:%d \n", rtd_inl(GPU_decmp_lu_3+index*4),
		rtd_inl(GPU_decmp_lu_4+index*4 ), rtd_inl(GPU_decmp_lu_5+index*4 ) );


	pr_crit("Chroma data_pitch:%d cmp_pitch:%d mem_pitch:%d \n",  rtd_inl(GPU_decmp_ch_3+index*4), 
		rtd_inl(GPU_decmp_ch_4+index*4), rtd_inl(GPU_decmp_ch_5+index*4) );

	
}
int GpuDec_DumpStatus(unsigned long arg, int do_clear )
{
	int i=0;
	int chg_err, cross_err, bl_err, busy;
	int have_error = 0;
	int enable = 0;
	int wrap_status_err = 0;
	int decmp_status_err = 0;
	unsigned int region_reg;
	unsigned int region_enable_bitmask = 0;
	unsigned int reg_addr = 0;

	int dump_register = 1; // flag
	
	volatile unsigned int data = LOCAL_INL(GPU_decmp_status_reg);

	wrap_status_err = LOCAL_INL(GPU_wrap_int_status_0) | LOCAL_INL(GPU_wrap_int_status_1) | LOCAL_INL(GPU_wrap_int_status_2);

	decmp_status_err = 0xFF & data;
	

	//xDumpTraffic("dumpOnly", 1);
	
	pr_crit("DumpStatus begin:%d\n", do_clear);
	pr_crit("decomp_core_errflag_ro: 0x%x\n", decmp_status_err);
//	pr_err("decomp_core_flag2irq_en: 0x%x\n", (0xFF00 & data) >>8 );
//	pr_err("decomp_core_errflag_clr: 0x%x\n", (data) >> 31 );


	if( dump_register )
	{
		pr_crit("decomp dump register start:\n");
		for(i=0; i< sizeof(debug_regs)/sizeof(debug_regs[0])  ; i++)
		{
			data = 	LOCAL_INL(debug_regs[i] );
			pr_err(" rtd_inl(0x%08x) = 0x%08x\n", debug_regs[i], data);
		}
	}

	for(i=0; i< DECMP_NUM_REGIONS; i++)
	{
		region_reg = GPU_decmp_region_reg+ i*4;
		data = LOCAL_INL(region_reg);
			
		busy = GPU_REF_DECMP_WRAP_GPU_decmp_region_0_get_busy(data);
		chg_err =  GPU_REF_DECMP_WRAP_GPU_decmp_region_0_get_chg_err(data);
		cross_err =  GPU_REF_DECMP_WRAP_GPU_decmp_region_0_get_cross_err(data) ;
		bl_err =  GPU_REF_DECMP_WRAP_GPU_decmp_region_0_get_bl_err(data)  ;
		enable = GPU_REF_DECMP_WRAP_GPU_decmp_region_0_get_enable(data);
		
	
		if( busy | chg_err | cross_err| bl_err )
		{
		
//			pr_err("region i, enable:%d  busy:%d chg_err:%d cross_err:%d bl_err:%d\n",
	//			(0x00000001&(data)),busy, chg_err, cross_err, bl_err  );

			pr_crit("regionRegErr 0x%08x idx:%d  err:0x%08x enable:%d  busy:%d chg_err:%d cross_err:%d bl_err:%d\n", 
				region_reg, i, 
				data,  enable, busy, chg_err, cross_err, bl_err  );

			have_error = 1;
		}
		

		if( enable ) 
		{
			int xx = 0;

			region_enable_bitmask |= ( 0x00000001<< i ) ;

			if( dump_register )
			{
				pr_err("regionReg idx:%d: 0x%08x=0x%08x enable:%d\n", i, region_reg,  data,  (0x00000001&(data)) );

				for(xx=0; xx< sizeof(debug_regs_per_set)/sizeof(debug_regs_per_set[0]); xx++)
				{
					reg_addr = debug_regs_per_set[xx] + i*4;
					data = 	LOCAL_INL(reg_addr );
					pr_err("\trtd_inl(0x%08x) = 0x%08x\n", reg_addr, data);
				}

				
				xDumpRegion(i);
				
			}
			
		}
		
	}

	//clear
	if( do_clear || have_error  ) {
		data = LOCAL_INL(GPU_wrap_int_status_0);

		pr_crit("decmp to clear region status: have_error:%d do_clear:%d\n", have_error, do_clear);
		pr_crit(" value: 0x%08x, 0x%08x,0x%08x \n",	rtd_inl(GPU_wrap_int_status_0), rtd_inl(GPU_wrap_int_status_1),
			rtd_inl(GPU_wrap_int_status_2) );

		//clear cross_err,...
		data |= (1<<31);
		DECMP_OUTL(GPU_wrap_int_status_0, data);

		data = 0;
		DECMP_OUTL(GPU_wrap_int_status_0, data);
	}

	if( do_clear) {
		//GpuDec_reset();
	}

	#if 0
	if ( decmp_status_err ) {
		
		data = LOCAL_INL(GPU_decmp_status_reg);

		pr_err("decmp to clear decmp status: 0x%08x\n", data);
		
		//
		data |= (1<<31);
		DECMP_OUTL(GPU_decmp_status_reg, data);
		msleep(1);

		//data &= 0x7FFFFFFF;
		//DECMP_OUTL(GPU_decmp_status_reg, data);
	}
	#endif//

	pr_err("DumpStatus end. region enabled: 0x%08x\n", region_enable_bitmask );

	return 0;
}


int GpuDec_DisableRegion(unsigned long arg )
{
	unsigned int in_region_mask = 0;
	unsigned int cur_region_mask = 0;
	int idx = 0;
	
	if (copy_from_user(&in_region_mask, (unsigned int *)arg, sizeof(in_region_mask)) ) {
			
			return -EFAULT;
	}

	for( idx =0 ; idx < DECMP_NUM_REGIONS; idx++)
	{
		cur_region_mask = ( 0x00000001<< idx ) ;
	
		if( cur_region_mask && in_region_mask ) {
			
			Gpu_Set_Dec_Region_enable( idx, 0 , 0);
		}
	}

	
	
	return 0;
}

	
int GpuDec_Disable(unsigned long arg )
{

	//disable all region
	Gpu_Set_Dec_Region_enable(0, 0, 1);
	
		
	GpuDec_DumpStatus( arg, 0);
	

	
	return 0;
}


void decmp_region_log_settime(int idx)
{
	u64 tt;
	RTK_DECMP_REGION_RECORD  *rec;

	//tt= arch_timer_read_counter();
	tt = getVcpuClock();


	rec = &gRegionStatus.regions[idx].rec[gRegionStatus.regions[idx].rec_wp];


	rec->set_time = tt;
	
}


void decmp_region_log_unsettime(int idx)
{
	
	u64 tt;
	RTK_DECMP_REGION_RECORD  *rec;
	
	tt= getVcpuClock();
	
	rec = &gRegionStatus.regions[idx].rec[gRegionStatus.regions[idx].rec_wp];

	rec->unset_time = tt;

}

/*
	check region consistent
*/
void GpuDec_Region_check(int index,int flag)
{
	
	unsigned int enabled = 0;
	//int i=0;
	//char tmp_buf[MAX_TRAFFIC_STR];
	
	unsigned int data=0;
	unsigned int org_data=0;
	//unsigned int mask_io;
	unsigned int region_reg = GPU_decmp_region_reg+index*4;

	unsigned int ve_data_addr = get_VEdataAddr_by_region(index);
		
	DECMP_GADDR data_addr = get_GPUdataAddr_by_region(index);

	DECMP_GADDR data_end_addr = get_dataEndGpuAddr_by_region(index);

	data = org_data = LOCAL_INL(region_reg);
	enabled = GPU_REF_DECMP_WRAP_GPU_decmp_region_0_get_enable(data);


	#ifdef RTK_ENABLE_DECMP_RECORD_DEBUG

	decmp_rec_diff( index,  data_addr);

	#endif//
	
	
	if(flag)
	{
		if( enabled ) {
			// 1 -> 1

			{
				//don't need check when enabled by ve
				//pr_err("enableChk dup 1->1 :%d  addr:0x%08x-0x%08x t:%llx\n", index, data_addr, data_end_addr, getVcpuClock() );  
			}
		}
		else
		{ // 0 -> 1
		
			if( gDecConfig.region_enable_by_ve ) 
			{  //VE should set to 1.

				pr_err("[decmpReg] err status(0->1):%d  addr:0x%"DECMP_GADDR_PRTYPE "-0x%"DECMP_GADDR_PRTYPE " val:0x%08x t:%llx\n", index, data_addr, data_end_addr, org_data, getVcpuClock() );
			}
			else 
			{
				// 0 -> 1: target case
				DECMP_LOG_DEBUG("enableChk 0->1 :%d   addr:0x%"DECMP_GADDR_PRTYPE "-0x%"DECMP_GADDR_PRTYPE " t:%llx\n", 
				index, data_addr, data_end_addr, getVcpuClock() );  
			}
			
		}
		
	}
	else
	{ 
		//to disable
	
		if( enabled ) 
		{	// 1 -> 0
			DECMP_LOG_DEBUG("disableChk 1->0 :%d  addr:0x%"DECMP_GADDR_PRTYPE "-0x%"DECMP_GADDR_PRTYPE " 0x%x t:%llx\n", 
				index, data_addr, data_end_addr, ve_data_addr, getVcpuClock() ); 
		}
		else 
		{
			// 0 -> 0
			DECMP_LOG_DEBUG("disableChk dup 0->0 :%d  addr:0x%"DECMP_GADDR_PRTYPE "-0x%"DECMP_GADDR_PRTYPE " 0x%x t:%llx\n", 
				index, data_addr, data_end_addr, ve_data_addr, getVcpuClock() );
		}

	}

	//decmp_region_log_settime(index);
	
}

inline void GpuDec_UnSet_DataAddr(int index, unsigned int in_addr)
{
	
	unsigned int data_addr = rtd_inl(GPU_decmp_lu_VEdataStart_reg+index*4);

	if(data_addr != in_addr) 
	{
		pr_crit("decmpReg unset: invalid dataAddr! %d 0x%08x 0x%08x\n", index, data_addr, in_addr);

		return ;
	}

	DECMP_OUTL_2(GPU_decmp_lu_VEdataStart_reg+index*4, 0 );
	

}

//prevent multiple unset 
DEFINE_MUTEX(DecmpUnSet_mutex);

int GpuDec_UnSet_Simple(rtk_decmp_export_simple* info, int* ret_region_idx)
{
	int target_idx = REGION_INVALID_INDEX;

	mutex_lock(&DecmpUnSet_mutex);
	
	if ( GpuDecUtil_get_region_by_dataAddr( info->address, &target_idx) != 0 ) {

		printk(KERN_WARNING"decmp UnSet, can't find addr 0x%08x fd:%d\n", info->address, info->fd); 

		mutex_unlock(&DecmpUnSet_mutex);
		return -1;
 	}

	//should atomic ?
	Gpu_Set_Dec_Region_enable(target_idx, 0, 0);
	GpuDec_UnSet_DataAddr(target_idx, info->address);

	//end atomic 
	
	//pr_err("[decmp] disable: :%d  addr:0x%08x\n", target_idx, info->address);
	
	mutex_unlock(&DecmpUnSet_mutex);

	if( ret_region_idx ) {
		*ret_region_idx = target_idx;
	}
	
	return 0;
}

int GpuDec_Set_Simple(rtk_decmp_export_simple* info)
{
	int target_idx = 0;

	static DECMP_ADDR glocal_last_target_addr = 0;
	static int num_region_enabled = 0;


	if( glocal_last_target_addr == info->address ) 
	{ //bind wtice, ignore ?

		return 0;
	}
	
	if ( GpuDecUtil_get_region_by_dataAddr( info->address, &target_idx) != 0 ) {

		pr_err("decmp can't find addr 0x%08x fd:%d\n", info->address, info->fd); 

		return -1;
 	}




	if( rtd_inl( 0xB802b0a0) == 0 ) {  //test only 
		//reset 
		num_region_enabled = 0;
	}

	if( num_region_enabled > 0 ) {
		//test one region only !
		
		return 0;
	}
	// check unbind

	//#ifdef RTK_DUMP_VE_FILE
	#if 0
	{
	
		#define VE_BUF_FILE_SIZE_2K (3428352)
		char fname[64];

		static int f_cnt_max = DECMP_DUMP_FILE_COUNTS;

		static int dump_count = 0;

		//big bunny 40
		static unsigned int call_cnt = 0;
		static unsigned int dump_frame_st = 30;

		//need to chmod 777 "everytime" at first
		//snprintf(fname, 64, "/data/ve_%d.bin", call_cnt); //EACCES 
		snprintf(fname, 64, "/data/ve_%d.bin", dump_count); //EACCES 


		if( f_cnt_max > 0 && call_cnt >= dump_frame_st ) {
			decmp_dump_file(fname, VE_BUF_FILE_SIZE_2K, info->address);
			f_cnt_max --;
			dump_count++;
		}
			

		call_cnt++;
	}
		

	#endif//RTK_DUMP_VE_FILE

	glocal_last_target_addr = info->address;



	if( gDecConfig.region_enable_by_ve )
	{
		GpuDec_Region_check(target_idx,  1);
	}
	else 
	{
		Gpu_Set_Dec_Region_enable(target_idx,  1, 0);
		//num_region_enabled = num_region_enabled+1;  //test only 
	}


	#if 0
	{
	static unsigned int aaa = 0;

		if( aaa++ == 15) {
			GpuDec_DumpStatus(0, 0);
		}
	}
	#endif//0
	
	//Gpu_Set_Dec_Region_enable(target_idx, 1, 0);
	//pr_err("[decmp] logOnly enable reg :%d  addr:0x%08x last:0x%08x\n", target_idx, info->address, glocal_last_target_addr );
	

	return 0;
}


int GpuDec_surface_free( unsigned long _addr, DECMP_GADDR g_addr, void* param)
{
	int ret = -1;
	int region_idx = REGION_INVALID_INDEX;

	DECMP_ADDR in_addr = (DECMP_ADDR)_addr;

	//static int sFreeCount = 0;
	
	rtk_decmp_export_simple info;
	memset(&info, 0, sizeof(info) );

	//DECMP_LOG_MEM("ionFree surface_free kern 0x%"DECMP_GADDR_PRTYPE " \n", in_addr);
	
	info.address = in_addr;
	
	//check if in dataAddr
	ret = GpuDec_UnSet_Simple(&info, &region_idx);


	#ifdef RTK_ENABLE_DECMP_RECORD_DEBUG
	decmp_rec_resetRec(in_addr, region_idx);

	#endif//
		
	#if 0
	sFreeCount++;
	if( sFreeCount >= 4 )
	{
		//assume play finish
		decmp_rec_reset();
		
		sFreeCount = 0;
	}
	#endif//
	
	return ret;
}

int GpuDec_Record(unsigned long arg )
{
	rtk_decmp_export decmp_export_info;
	
	//struct venusfb_info *fbi = (struct venusfb_info *) info;
//	pr_err("aa1\n");

	if (copy_from_user(&decmp_export_info, (struct rtk_decmp_export *)arg, sizeof(rtk_decmp_export))) {
		
		return -EFAULT;
	}

	#ifdef RTK_ENABLE_DECMP_RECORD_DEBUG
	decmp_rec_initRec( &decmp_export_info );

	#endif//
	

	return 0;
}

int Gpu_Set_Dec_Set(unsigned long arg )
{

//	unsigned int pic_pitch = 0;
	rtk_decmp_export decmp_export_info;
	
	unsigned int data_pitch = 0;
	unsigned int cmp_pitch = 0;
	unsigned int mem_pitch = 0;
	unsigned int compr_rate = 0;
	int index = 0;

	
	//struct venusfb_info *fbi = (struct venusfb_info *) info;
//	pr_err("aa1\n");

	if (copy_from_user(&decmp_export_info, (struct rtk_decmp_export *)arg, sizeof(rtk_decmp_export))) {
		
		return -EFAULT;
	}
	compr_rate = decmp_export_info.compr_ratio;
		
	data_pitch = decmp_export_info.pitch; //uncompressed
	
	cmp_pitch = decmp_export_info.cmp_pitch;  //compressed
	mem_pitch = decmp_export_info.mem_pitch;

	//for comp wrapper
	if( decmp_export_info.cmp_pitch != 0 ) {
		cmp_pitch = decmp_export_info.cmp_pitch;
	}

	#ifdef GPUDECMP_ENABLE_UNIT_TEST 
	//for VE unit test
	if( compr_rate == 50 )
	{
		if( data_pitch == 768 ) //720
		{
			cmp_pitch = mem_pitch = 384;
		}
		else if (data_pitch == 1920 )
		{
			cmp_pitch = mem_pitch = 960;
		}
		else if (data_pitch == 3840 )
		{
			cmp_pitch = mem_pitch = 1920;
		}
	}
	else if ( compr_rate == 75 )
	{
		if( data_pitch == 768 )
		{
			cmp_pitch = mem_pitch = 576;
		}
		else if (data_pitch == 1920 )
		{
			cmp_pitch = mem_pitch = 1440;
		}
		else if (data_pitch == 3840 )
		{
			cmp_pitch = mem_pitch = 2880;
		}

	}

	#endif//

	
	index = decmp_export_info.region_idx;

	//pr_err("aa4\n");

	// Constraint: lu_mem_pitch >= lu_cmp_pitch
	Gpu_Set_Dec_Region(index, decmp_export_info.header_y_addr, decmp_export_info.data_y_addr,
				decmp_export_info.data_y_addr_gpu, 
			    decmp_export_info.data_y_size ,
					  (decmp_export_info.width),
					  data_pitch,
					  cmp_pitch,
					  mem_pitch,
					  compr_rate
					 );
	


	Gpu_Set_ch_Dec_Region(index, decmp_export_info.header_uv_addr, decmp_export_info.data_uv_addr,
				   decmp_export_info.data_uv_addr_gpu,
				   decmp_export_info.data_uv_size ,
				   data_pitch,
				   cmp_pitch, mem_pitch);

	
	Gpu_Set_Dec_Region_enable(index, 1, 0);

	#if 0 //test 
	pr_err("decmpDrv enable %d, y:0x%08x uv:0x%08x 0x%08x 0x%08x data_pitch:%d cmp_pitch:%d rate:%d\n", 
		index, 	decmp_export_info.data_y_addr, decmp_export_info.data_uv_addr,
		decmp_export_info.header_y_addr, decmp_export_info.header_uv_addr, data_pitch, cmp_pitch, compr_rate );
	#endif//0

	return 0;
}

int Gpu_Set_Dec_UnSet(unsigned long arg )
{
	
	rtk_decmp_export decmp_export_info;
	
	
	if (copy_from_user(&decmp_export_info, (struct rtk_decmp_export *)arg, sizeof(rtk_decmp_export))) {
		
		return -EFAULT;
	}
	

	Gpu_Set_Dec_Region_enable(decmp_export_info.region_idx, 0, 0);

	printk(KERN_WARNING"Gpu_Set_Dec_UnSet idx:%d\n", decmp_export_info.region_idx);
	
	return 0;
}

int Gpu_Set_Dec_test_RGBA(unsigned long arg)
{
	rtk_decmp_export decmp_export_info;
	
	//struct venusfb_info *fbi = (struct venusfb_info *) info;

	if (copy_from_user(&decmp_export_info, (struct rtk_decmp_export *)arg, sizeof(rtk_decmp_export)))
		return -EFAULT;
		

	#if 0
	unsigned int data=0;

	
	data = LOCAL_INL(GPU_decmp_ctrl_reg);
    //disable Q1 FIFO in data cache    (1<<29)
	//don!|t change cmd order from GPU  (1<<28)
	//enable to decomp GPU command     (1<<1)
	data |= ((1<<29)|(1<<28)|(1<<1));
	DECMP_OUTL(GPU_decmp_ctrl_reg,data);
	
	Gpu_Set_Dec_Region(0, header_addr, data_addr,
					   GPU_decmp_buf_8MB,
					  (GPU_decmp_1920_w),
					  GPU_decmp_1920_w,
					  GPU_decmp_1920_w,
					  GPU_decmp_1920_w );
	
	
	Gpu_Set_Dec_Region_enable(0,1);
	#endif

	pr_err("decmpDrv %s %d  0x%x 0x%x\n", __FUNCTION__, __LINE__, decmp_export_info.data_y_addr, decmp_export_info.data_uv_addr);

	
	return 0;
}

#if IS_ENABLED(CONFIG_DMABUF_HEAPS_DVR) && IS_ENABLED(CONFIG_DMABUF_CARVEOUT_HEAP) && IS_ENABLED(CONFIG_ION_DVR_HEAP)


extern void dvr_dmabuf_free(struct dma_buf *dmabuf);
extern int dvr_dmabuf_to_list(struct dma_buf *dmabuf, unsigned long buf_array[], int array_num);
extern  struct dma_buf *dvr_dmabuf_frames_malloc(size_t frame_size, unsigned int frame_num, int flags, int frame_type, bool need_protect, unsigned long **first_frame_phys);


extern int ion_dvr_set_ve_addr(struct ion_buffer *buffer, unsigned long ve_addr, unsigned long ve_addr_2 );
extern int ion_dvr_free_ve_addr(struct ion_buffer *buffer, unsigned long addr,  unsigned long addr_2);
extern unsigned long ion_dvr_alloc_ve_addr(int size);


#define VE_VBM_MAX_COUNT (14)

typedef struct 
{
	struct dma_buf *dma_buf;
	unsigned long phy_addr;
	int in_use;


} VBM_DMABUF_ITEM;

static VBM_DMABUF_ITEM gVBM_DMABUF_Array[VE_VBM_MAX_COUNT] = {0};


DEFINE_MUTEX(VBM_DECMP_mutex);

unsigned long ion_VbmDMAHeap_get_freeAddr(void)
{
	unsigned long phy_addr = 0;
	int idx = 0;

	mutex_lock(&VBM_DECMP_mutex);

	for( ; idx < VE_VBM_MAX_COUNT; idx++)
	{
		if ( gVBM_DMABUF_Array[idx].in_use ==0 ) {
			
			phy_addr = gVBM_DMABUF_Array[idx].phy_addr; 
				
			gVBM_DMABUF_Array[idx].in_use = 1;
			break;
		}

	}


	mutex_unlock(&VBM_DECMP_mutex);


	if( idx == VE_VBM_MAX_COUNT ) {

		pr_crit("kkk no free VbmDMAbuf for VE\n");

	}

	return phy_addr;

}


int ion_VbmDMAHeap_free_ve_addr(struct ion_buffer *buffer, unsigned long addr, unsigned long addr_2)
{
	int err = -1;
	int idx = 0;
	//put back to pool


	mutex_lock(&VBM_DECMP_mutex);


	for( ; idx < VE_VBM_MAX_COUNT; idx++)
	{
		if ( gVBM_DMABUF_Array[idx].phy_addr == addr) {
			
			pr_crit(" kkk free gVBM_DMABUF_Array %d 0x%lx\n", idx, gVBM_DMABUF_Array[idx].phy_addr  );
				
			gVBM_DMABUF_Array[idx].in_use = 0;
			err = 0;
			break;
		}

	}

	mutex_unlock(&VBM_DECMP_mutex);	

	if( idx == VE_VBM_MAX_COUNT ) {

		pr_crit(" kkk free gVBM_DMABUF_Array  fail !!! :%d 0x%lx\n", idx, addr );
	}

	return err;
	
}
EXPORT_SYMBOL(ion_VbmDMAHeap_free_ve_addr);


int ion_VbmDMAHeap_init_pool( void )
{
	int err = 0;
	size_t  buf_size = 10948608;
	int size_1_cnt = 9;
	int size_2_cnt = VE_VBM_MAX_COUNT - size_1_cnt;
	int idx = 0;
	int cc = 0;


	struct dma_buf *dmabuf_1 = NULL;
	struct dma_buf *dmabuf_2 = NULL;
	int size_1_ret_cnt = 0;
	int size_2_ret_cnt = 0;
	
	unsigned long first_phy_addr = 0, sec_phy_addr = 0;
	unsigned long dmabuf_array_1[VE_VBM_MAX_COUNT] = {0};
	unsigned long dmabuf_array_2[VE_VBM_MAX_COUNT] = {0};

	#if 0
  //zone 0: 102M / 10948608 = 9.7
  //zone 1: change to 
	unsigned long VBM_HEAP_CHUNK_SIZE[CHUNK_ZONE_COUNT][MAX_CHUNKS_PER_ZONE] = {
		{36*__MB__, 36*__MB__, 12*__MB__, 18*__MB__},
		{48*__MB__, 18*__MB__, 0, 0},
		{24*__MB__, 6*__MB__, 0, 0},
		};


	#endif

	dmabuf_1 =  dvr_dmabuf_frames_malloc(buf_size, size_1_cnt , VBM_BUF_DECOMP, DMA_HEAP_USER_VIDEO_4K_LUMA, false, (unsigned long **)&first_phy_addr);
	if ( IS_ERR_OR_NULL(dmabuf_1) ) {
			pr_crit("[%s %d] alloc 1 dmabuf vbm fail :%d \n", __func__, __LINE__, size_1_cnt);
		err = -1;
		goto ALLOC_FAIL;
	}

	dmabuf_2 =  dvr_dmabuf_frames_malloc(buf_size, size_2_cnt , VBM_BUF_DECOMP, DMA_HEAP_USER_VIDEO_4K_CHROMA, false, (unsigned long **)&sec_phy_addr);
	if ( IS_ERR_OR_NULL(dmabuf_2) ) {
			pr_crit("[%s %d] alloc 2 dmabuf vbm fail :%d\n", __func__, __LINE__, size_2_cnt);

		err = -2;
		goto ALLOC_FAIL;
	}



	size_1_ret_cnt = dvr_dmabuf_to_list(dmabuf_1, dmabuf_array_1, size_1_cnt);
	size_2_ret_cnt = dvr_dmabuf_to_list(dmabuf_2, dmabuf_array_2, size_2_cnt);

	if ( size_1_ret_cnt != size_1_cnt || size_2_ret_cnt != size_2_cnt 
		) {
		
		pr_crit("[%s %d] cnt(%d) cnt not match %d %d %d %d\n", __func__, __LINE__,  size_1_ret_cnt, size_1_cnt, size_2_ret_cnt, size_2_cnt );
		
		err = -3;
		goto ALLOC_FAIL;
	}

	mutex_lock(&VBM_DECMP_mutex);

	for( cc =0 ;cc < size_1_ret_cnt; cc++)
	{
		gVBM_DMABUF_Array[idx].dma_buf =  (struct dma_buf *)dmabuf_array_1[cc];
		gVBM_DMABUF_Array[idx].in_use = 0;

		gVBM_DMABUF_Array[idx].phy_addr = dvr_dmabuf_to_phys( gVBM_DMABUF_Array[idx].dma_buf );

		pr_crit(" kkk get vbmBuf for decmp ve %d 0x%lx\n", idx, gVBM_DMABUF_Array[idx].phy_addr);
		++idx;
	}
	
	for( cc=0 ;cc < size_2_ret_cnt; cc++)
	{
		gVBM_DMABUF_Array[idx].dma_buf =  (struct dma_buf *)dmabuf_array_2[cc];
		gVBM_DMABUF_Array[idx].in_use = 0;

		gVBM_DMABUF_Array[idx].phy_addr = dvr_dmabuf_to_phys( gVBM_DMABUF_Array[idx].dma_buf );
		
		pr_crit(" kkk get vbmBuf for decmp ve %d 0x%lx\n", idx, gVBM_DMABUF_Array[idx].phy_addr);
		
		++idx;

	}
	mutex_unlock(&VBM_DECMP_mutex);


   ALLOC_FAIL:

		return err;
}


unsigned long ion_VbmDMAHeap_alloc_ve_addr(int size)
{
	static int first_call = 0;

	unsigned long ve_addr = 0;


	if( first_call == 0 ) {
		//init, create pool
		ion_VbmDMAHeap_init_pool();
		first_call = 1;
	}



	ve_addr = ion_VbmDMAHeap_get_freeAddr();

	return ve_addr;

}



// for ion version
int GpuDec_Alloc_Addr_VBM(unsigned long arg)
{
	struct dma_buf *dmabuf = NULL;
	rtk_decmp_atachAddr addr_info;

	struct ion_buffer *buffer=NULL;
	unsigned long ve_addr = 0;
	unsigned long ve_addr_2 = 0;
		
	int ion_fd = 0;
	int ret = -1;
	
	memset(&addr_info, 0, sizeof(addr_info) );
	
	if (copy_from_user(&addr_info, (void*)arg, sizeof(rtk_decmp_atachAddr)))	{
		DECMP_PRINT(1, " %s copy_from_user fail: %llx\n", __func__, arg );
		return -EFAULT;
	}

	ion_fd = addr_info.fd;
	

	dmabuf = dma_buf_get(ion_fd);
	
	if (IS_ERR(dmabuf)) {
		DECMP_PRINT(1, "%s get fd fail %d \n", __func__, ion_fd);
		goto ErrRet;
	}
	
	buffer = dmabuf->priv;
	if (buffer == NULL) {
		DECMP_PRINT(1, "%s() no buffer, dmabuf(%lx) fd(%d)\n", __func__, (unsigned long)dmabuf, ion_fd);
		goto ErrRet;
	}
	
	if (buffer->heap->type != ION_HEAP_TYPE_CUSTOM) {
		DECMP_PRINT(1, "%s() not sutable heap type(%d) fd(%d)\n", __func__, buffer->heap->type, ion_fd);
		goto ErrRet;
	}
	
	if (buffer->sg_table == NULL) {
		DECMP_PRINT(1, "%s() no sg table, fd(%d)\n", __func__, ion_fd);
		goto ErrRet;
	}

	
	ve_addr = ion_VbmDMAHeap_alloc_ve_addr(addr_info.size);
	
	if( ve_addr == 0 ) {
		DECMP_PRINT(1, " alloc_ve_addr fail %d  org:0x%llx\n", addr_info.size, buffer->phy_address);
		goto ErrRet;
	}

	if( addr_info.size_2 != 0 ) 
	{
		ve_addr_2 = ion_VbmDMAHeap_alloc_ve_addr( addr_info.size_2 );

		if( ve_addr_2 == 0 ) {
			DECMP_PRINT(1, " alloc_ve_addr2 fail %d  org:0x%llx\n", addr_info.size_2, buffer->phy_address);
			goto ErrRet;
		}

	}

	ret =  ion_dvr_set_ve_addr(buffer, ve_addr,  0xFFFFFFF0 ) ;


	if( ret ) {
		DECMP_PRINT(1, " alloc_ve_addr set fail 0x%llx 0x%llx 0x%llx\n", ve_addr, ve_addr_2, buffer->phy_address);
		goto ErrRet;
	}


	

	addr_info.ret_addr = ve_addr;
	addr_info.ret_addr_2 = ve_addr_2;
	
	if ( copy_to_user((void *)arg, &addr_info, sizeof(rtk_decmp_atachAddr) ) != 0)
	{
		DECMP_PRINT(1, " %s copy_to_user fail: %llx\n", __func__, ve_addr );
		goto ErrRet;
		return -EFAULT;
	}

	if (!IS_ERR(dmabuf))
		dma_buf_put(dmabuf);

	return 0;
	
  ErrRet:
  	if( ve_addr != 0 || ve_addr_2 != 0 ) {
//		ion_dvr_free_ve_addr( NULL, ve_addr, ve_addr_2 );
 	}
	
	if (!IS_ERR(dmabuf))
		dma_buf_put(dmabuf);


	return ret;
}

#endif/// defined(CONFIG_DMABUF_HEAPS_DVR)


//#if defined(DECMP_USE_DMA_HEAP) && defined(CONFIG_DMABUF_HEAPS_DVR)
#if IS_ENABLED(CONFIG_DMABUF_HEAPS_DVR)


extern const struct dma_buf_ops dvr_heap_buf_ops;


extern int dvr_heap_set_ve_addr(void* buffer, unsigned long ve_addr, unsigned long ve_addr_2 );
extern int dvr_heap_free_ve_addr(void* buffer, unsigned long addr,  unsigned long addr_2);
extern unsigned long dvr_heap_alloc_ve_addr(int size);


int GpuDec_Alloc_Addr_DMAHEAP(unsigned long arg)
{
	struct dma_buf *dmabuf = NULL;
	rtk_decmp_atachAddr addr_info;

	void *buffer=NULL;
	unsigned long ve_addr = 0;
	unsigned long ve_addr_2 = 0;
		
	int ion_fd = 0;
	int ret = -1;
	
	memset(&addr_info, 0, sizeof(addr_info) );
	
	if (copy_from_user(&addr_info, (void*)arg, sizeof(rtk_decmp_atachAddr)))	{
		DECMP_PRINT(1, " %s copy_from_user fail: %llx\n", __func__, arg );
		return -EFAULT;
	}

	ion_fd = addr_info.fd;
	

	dmabuf = dma_buf_get(ion_fd);
	
	if (IS_ERR(dmabuf)) {
		DECMP_PRINT(1, "%s get fd fail %d \n", __func__, ion_fd);
		goto ErrRet;
	}

	if( dmabuf->ops != &dvr_heap_buf_ops)
	{
		DECMP_PRINT(1, "%s() no dvr_heap, dmabuf(%lx) fd(%d)\n", __func__, (unsigned long)dmabuf, ion_fd);
		goto ErrRet;

	}

	
	buffer = dmabuf->priv;
	if (buffer == NULL) {
		DECMP_PRINT(1, "%s() no buffer, dmabuf(%lx) fd(%d)\n", __func__, (unsigned long)dmabuf, ion_fd);
		goto ErrRet;
	}


	
	ve_addr = dvr_heap_alloc_ve_addr(addr_info.size);
	
	if( ve_addr == 0 ) {
		//DECMP_PRINT(1, " alloc_ve_addr fail %d  org:0x%llx\n", addr_info.size, buffer->phy_address);
		DECMP_PRINT(1, " alloc_ve_addr fail %d  org:0x%llx\n", addr_info.size, buffer);
		goto ErrRet;
	}

	if( addr_info.size_2 != 0 ) 
	{
		ve_addr_2 = dvr_heap_alloc_ve_addr( addr_info.size_2 );

		if( ve_addr_2 == 0 ) {
			//DECMP_PRINT(1, " alloc_ve_addr2 fail %d  org:0x%llx\n", addr_info.size_2, buffer->phy_address);
			DECMP_PRINT(1, " alloc_ve_addr2 fail %d  org:0x%llx\n", addr_info.size_2, buffer);
			goto ErrRet;
		}

	}

	ret =  dvr_heap_set_ve_addr(buffer, ve_addr, ve_addr_2) ;


	if( ret ) {
		//DECMP_PRINT(1, " alloc_ve_addr set fail 0x%llx 0x%llx 0x%llx\n", ve_addr, ve_addr_2, buffer->phy_address);
		DECMP_PRINT(1, " alloc_ve_addr set fail 0x%llx 0x%llx 0x%llx\n", ve_addr, ve_addr_2, buffer);
		goto ErrRet;
	}


	

	addr_info.ret_addr = ve_addr;
	addr_info.ret_addr_2 = ve_addr_2;
	
	if ( copy_to_user((void *)arg, &addr_info, sizeof(rtk_decmp_atachAddr) ) != 0)
	{
		DECMP_PRINT(1, " %s copy_to_user fail: %llx\n", __func__, ve_addr );
		goto ErrRet;
		return -EFAULT;
	}

	if (!IS_ERR(dmabuf))
		dma_buf_put(dmabuf);

	return 0;
	
  ErrRet:
  	if( ve_addr != 0 || ve_addr_2 != 0 ) {
		dvr_heap_free_ve_addr( NULL, ve_addr, ve_addr_2 );
 	}
	
	if (!IS_ERR(dmabuf))
		dma_buf_put(dmabuf);


	return ret;
}
#else
int GpuDec_Alloc_Addr_DMAHEAP(unsigned long arg)
{

	pr_crit("invalid kernel config for GpuDec_Alloc_Addr_DMAHEAP \n");
}


#endif//


#if IS_ENABLED(CONFIG_ION_DVR_HEAP)
extern int ion_dvr_set_ve_addr(struct ion_buffer *buffer, unsigned long ve_addr, unsigned long ve_addr_2 );
extern int ion_dvr_free_ve_addr(struct ion_buffer *buffer, unsigned long addr,  unsigned long addr_2);
extern unsigned long ion_dvr_alloc_ve_addr(int size);


int GpuDec_Alloc_Addr(unsigned long arg)
{
	struct dma_buf *dmabuf = NULL;
	rtk_decmp_atachAddr addr_info;

	struct ion_buffer *buffer=NULL;
	unsigned long ve_addr = 0;
	unsigned long ve_addr_2 = 0;
		
	int ion_fd = 0;
	int ret = -1;
	
	memset(&addr_info, 0, sizeof(addr_info) );
	
	if (copy_from_user(&addr_info, (void*)arg, sizeof(rtk_decmp_atachAddr)))	{
		DECMP_PRINT(1, " %s copy_from_user fail: %llx\n", __func__, arg );
		return -EFAULT;
	}

	ion_fd = addr_info.fd;
	

	dmabuf = dma_buf_get(ion_fd);
	
	if (IS_ERR(dmabuf)) {
		DECMP_PRINT(1, "%s get fd fail %d \n", __func__, ion_fd);
		goto ErrRet;
	}
	
	buffer = dmabuf->priv;
	if (buffer == NULL) {
		DECMP_PRINT(1, "%s() no buffer, dmabuf(%lx) fd(%d)\n", __func__, (unsigned long)dmabuf, ion_fd);
		goto ErrRet;
	}
	
	if (buffer->heap->type != ION_HEAP_TYPE_CUSTOM) {
		DECMP_PRINT(1, "%s() not sutable heap type(%d) fd(%d)\n", __func__, buffer->heap->type, ion_fd);
		goto ErrRet;
	}
	
	if (buffer->sg_table == NULL) {
		DECMP_PRINT(1, "%s() no sg table, fd(%d)\n", __func__, ion_fd);
		goto ErrRet;
	}

	
	ve_addr = ion_dvr_alloc_ve_addr(addr_info.size);
	
	if( ve_addr == 0 ) {
		DECMP_PRINT(1, " alloc_ve_addr fail %d  org:0x%llx\n", addr_info.size, buffer->phy_address);
		goto ErrRet;
	}

	if( addr_info.size_2 != 0 ) 
	{
		ve_addr_2 = ion_dvr_alloc_ve_addr( addr_info.size_2 );

		if( ve_addr_2 == 0 ) {
			DECMP_PRINT(1, " alloc_ve_addr2 fail %d  org:0x%llx\n", addr_info.size_2, buffer->phy_address);
			goto ErrRet;
		}

	}

	ret =  ion_dvr_set_ve_addr(buffer, ve_addr, ve_addr_2) ;


	if( ret ) {
		DECMP_PRINT(1, " alloc_ve_addr set fail 0x%llx 0x%llx 0x%llx\n", ve_addr, ve_addr_2, buffer->phy_address);
		goto ErrRet;
	}


	

	addr_info.ret_addr = ve_addr;
	addr_info.ret_addr_2 = ve_addr_2;
	
	if ( copy_to_user((void *)arg, &addr_info, sizeof(rtk_decmp_atachAddr) ) != 0)
	{
		DECMP_PRINT(1, " %s copy_to_user fail: %llx\n", __func__, ve_addr );
		goto ErrRet;
		return -EFAULT;
	}

	if (!IS_ERR(dmabuf))
		dma_buf_put(dmabuf);

	return 0;
	
  ErrRet:
  	if( ve_addr != 0 || ve_addr_2 != 0 ) {
		ion_dvr_free_ve_addr( NULL, ve_addr, ve_addr_2 );
 	}
	
	if (!IS_ERR(dmabuf))
		dma_buf_put(dmabuf);


	return ret;
}

#endif/// defined(CONFIG_DMABUF_HEAPS_DVR)





irqreturn_t GPU_DECMP_irq_handler(int irq, void *dev_id)
{
  
	//gdma_dev *gdma = dev_id;
	int have_status = 0;


	GpuDec_CheckRegionStatus( 1); //print error only, don't reset error

	have_status = GpuDec_CheckStatus( 1) ;

	if( ! have_status ) {
		
		return IRQ_NONE;
	}
  
	


	return IRQ_HANDLED; 
}



int Gpu_Set_Dec_deInit(void* dev )
{

	if( gDecConfig.irq_no > 0 ) {
		free_irq( gDecConfig.irq_no, dev);
	}

	#ifdef GDECMP_ENABLE_VERIFY
		gdecmp_uninit_debug_proc( );
#endif//
	//memset(&gDecConfig, 0, sizeof(gDecConfig) );
		
	return 0;
}



typedef int (*RT_ION_FREE_CALLBACK)(unsigned long, DECMP_GADDR, void*);


extern int ion_free_register_callback( RT_ION_FREE_CALLBACK cb);


#if IS_ENABLED(CONFIG_DMABUF_HEAPS_DVR)
	extern int dvr_heap_free_register_callback( RT_ION_FREE_CALLBACK cb );

#endif//



void _init_gpu_decomp( void )
{
	unsigned int data=0;

	printk(KERN_WARNING"_init_gpu_decomp 1 %d\n", __LINE__ );

	#if 1 //
	    
		//CLKEN = 0
		rtd_outl( SYS_REG_SYS_CLKEN4_reg, SYS_REG_SYS_CLKEN4_clken_gdcmp_mask | 0x0);
		udelay(500);

		//RSTN = 0
		DECMP_OUTL( SYS_REG_SYS_SRST4_reg,  SYS_REG_SYS_SRST4_rstn_gdcmp_mask | 0x0);
		udelay(500);

		//RSTN = 1
		DECMP_OUTL( SYS_REG_SYS_SRST4_reg,  SYS_REG_SYS_SRST4_rstn_gdcmp_mask | 0x1);
		udelay(500);

		//CLKEN = 1
		DECMP_OUTL( SYS_REG_SYS_CLKEN4_reg, SYS_REG_SYS_CLKEN4_clken_gdcmp_mask | 0x1);
		udelay(500);
		
		
		
	#else
		//DECMP_OUTL( 0xB800010C, 0x00800001); // refcmp_rstn (cmp engine)
		// SYS_REG_SYS_SRST4_reg 0xB8000120
		rtd_outl(SYS_REG_SYS_SRST4_reg, SYS_REG_SYS_SRST4_rstn_gdcmp_mask | SYS_REG_SYS_SRST4_write_data_mask ); // decompress rstn   [FIXME]
	#endif//

	data = rtd_inl(GPU_REF_DECMP_WRAP_GPU_decmp_ctrl_reg);
	
    //disable Q1 FIFO in data cache    (1<<29)
	//don't change cmd order from GPU  (1<<28)
	//enable to decomp GPU command     (1<<0)
	#if 0
	    data |= ((GPU_REF_DECMP_WRAP_GPU_decmp_ctrl_cache_q1_dis_mask)|(GPU_REF_DECMP_WRAP_GPU_decmp_ctrl_cmd_inorder_mask)|(1<<0));
	#else
		data |= ((1<<0));
	#endif

	
	rtd_outl(GPU_REF_DECMP_WRAP_GPU_decmp_ctrl_reg, data);

}

//need init before gpu init
int Gpu_Set_Dec_init( int enable_intr, void* dev )
{

	if( g_decmpInit ) {
		printk(KERN_WARNING"gpu Decmp reinit. do nothing !\n");
		return 0;
	}

	memset(&gDecConfig, 0, sizeof(gDecConfig) );

	gDecConfig.region_enable_by_ve = 1;  
	gDecConfig.region_disable_by_ve = 1;
	

    #if IS_ENABLED(CONFIG_ION_DVR_HEAP)
		ion_free_register_callback( GpuDec_surface_free );
	#endif//

	#if IS_ENABLED(CONFIG_DMABUF_HEAPS_DVR)
		dvr_heap_free_register_callback( GpuDec_surface_free );

	#endif//
	
	//gdecmp_rec_reset();

	_init_gpu_decomp(); // Note -> move to DDK  ?  [FIXME]
	
	g_decmpInit = 1;
	
	memset((void *)&gRegionStatus, 0, sizeof(gRegionStatus) );


	
	if( enable_intr )  
	//if (0)
	{
		gpu_ref_decmp_wrap_gpu_decmp_status_RBUS status_reg;

		
		msleep(1);

		//data = GPU_REF_DECMP_WRAP_GPU_decmp_status_decomp_core_flag2irq_en_mask;
		status_reg.regValue = 0;

		#if 1
		//default 
		status_reg.decomp_core_flag2irq_en = //( 1<< 0 ) |
											 ( 1<<1 ) |
											 (1 << 2 ) |
											 ( 1<< 3 )|
											 ( 1<< 4 ) |
										//	 ( 1 << 5 ) |
											 ( 1 << 6 ) |
											 ( 1 << 7 );
		#else
		status_reg.decomp_core_flag2irq_en = ( 1<< 0 ) |
											 ( 1<<1 ) |
											 (1 << 2 ) |
											 ( 1<< 3 )|
											 ( 1<< 4 ) |
											 ( 1 << 5 ) |
											 ( 1 << 6 ) |
											 ( 1 << 7 );

		#endif//		
											 
		DECMP_OUTL(GPU_decmp_status_reg, status_reg.regValue );
		
	}

	#ifdef GDECMP_ENABLE_VERIFY
		gdecmp_init_debug_proc();
	
	#endif//
	
	return 0;
}


static int open_refdecmp(struct inode * inode, struct file * filp)
{
	return 0;
}

static int release_refdecmp(struct inode * inode, struct file * filp)
{
	if (filp->private_data != NULL) {

		#if 0		
		struct mem_record_node *rec, *tmp;
		struct list_head *plist;

		plist = &((struct mem_record_head *)filp->private_data)->list;
		
		list_for_each_entry_safe(rec, tmp, plist, list) {
//			printk("*** remove2: %lx \n", rec->addr);
			dvr_free_page(pfn_to_page(rec->addr));
			list_del(&rec->list);
			kfree(rec);
		}

		kfree(filp->private_data);
		filp->private_data = NULL;

		#endif// 0
		
	}
	return 0;
}

static long ioctl_refdecmp(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int retval = 0;

	switch (cmd) 
	{
	
	case GDECMP_CMD_INIT:

	retval = Gpu_Set_Dec_init(0, NULL);
	break;
	
	case GDECMP_CMD_TEST_DECMP_1:
		retval = Gpu_Set_Dec_test_RGBA(arg);

		break;

	case GDECMP_CMD_SET:
		retval = Gpu_Set_Dec_Set(arg);
		break;
	case GDECMP_CMD_UNSET:
		retval = Gpu_Set_Dec_UnSet(arg);
		break;

	case GDECMP_CMD_DUMPSTATUS:
		retval = GpuDec_DumpStatus(arg, 0);
		break;
	
	case GDECMP_CMD_CLEAR_DUMPSTATUS:
		retval = GpuDec_DumpStatus(arg, 1);

		break;

	case GDECMP_CMD_DISABLE:
		retval = GpuDec_Disable(arg);
		
		break;

	case GDECMP_CMD_SELECT_DISABLE:
		Gpu_GpuDec_Select_enable(0);
		break;
	case GDECMP_CMD_SELECT_ENABLE:
		Gpu_GpuDec_Select_enable(1);

		break;
	case GDECMP_CMD_SET_SIMPLE:
		{
			rtk_decmp_export_simple decmp_export_simple;
			if (copy_from_user(&decmp_export_simple, (struct rtk_decmp_export *)arg, sizeof(decmp_export_simple))) {
			
				return -EFAULT;
			}

			retval = GpuDec_Set_Simple(&decmp_export_simple);
		}		
		break;

	case GDECMP_CMD_UNSET_SIMPLE:
		{
			rtk_decmp_export_simple decmp_export_simple;
			if (copy_from_user(&decmp_export_simple, (struct rtk_decmp_export *)arg, sizeof(decmp_export_simple))) {
			
				return -EFAULT;
			}

			retval = GpuDec_UnSet_Simple(&decmp_export_simple, NULL);
		}
		break;
	case GDECMP_CMD_RECORD:
		{
			retval = GpuDec_Record(arg);

		
		}
		break;
  #if IS_ENABLED(CONFIG_ION_DVR_HEAP)

	case GDECMP_CMD_ALLOC_ADDR:
		{
			retval = GpuDec_Alloc_Addr(arg);
		
			break;
		}
  #endif//

  #if IS_ENABLED(CONFIG_DMABUF_HEAPS_DVR) 
	case  GDECMP_CMD_ALLOC_ADDR_DMAHEAP:
		{
			retval = GpuDec_Alloc_Addr_DMAHEAP(arg);
		
			break;
		}

   #endif//

  #if IS_ENABLED(CONFIG_DMABUF_HEAPS_DVR) && IS_ENABLED(CONFIG_ION_DVR_HEAP) && IS_ENABLED(CONFIG_DMABUF_CARVEOUT_HEAP)  
	case GDECMP_CMD_ALLOC_ADDR_VBM:
		{
			retval = GpuDec_Alloc_Addr_VBM(arg);
		
			break;
		}
  #endif//
	
	case GDECMP_CMD_GET_HW_VERSION:
		{
			unsigned int ver = get_ic_version();

			if ( copy_to_user((void *)arg, &ver, sizeof(ver) ) != 0)
			{
				DECMP_PRINT(1, " %s copy_to_user fail: %u\n", __func__, ver );
				return -EFAULT;
			}
			

		}
		break;		
	default:
        retval = -ENOIOCTLCMD;
    }

	if( retval != 0) {
		printk(KERN_WARNING"gDecmp cmd:0x%x err:%d\n", cmd, retval);
	}
	
	
	return retval;
}

static char *refdecmp_devnode(struct device *dev, umode_t *mode)
{
	if (mode)
		*mode = 0666;
	return NULL;
}

static const struct file_operations refdecmp_fops = {
//	.llseek         = ,
//	.read           = ,
//	.write          = ,
	.open           = open_refdecmp,
	.release        = release_refdecmp,
	.unlocked_ioctl = ioctl_refdecmp,
	.compat_ioctl   = ioctl_refdecmp,
};


static int gdecmp_probe(struct platform_device *pdev)
{
	int result = 0, irq;

//	

	//pr_alert("kkk %s %d irq ok\n", __FUNCTION__, __LINE__);


	#if 0
	
	if ( request_irq(gic_irq_find_mapping(SPI_GPU_DECOMP), GPU_DECMP_irq_handler, IRQF_SHARED, "gpuDecmp", dev) ) {
			printk(KERN_EMERG" GpuDecmp: can't get assigned irq%i\n", SPI_GPU_DECOMP);
	
			return -1;
	}
	#endif//
	
	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		pr_err("gdecmp: can't get irq. ret=%d\n", irq);
	} else {
		result = request_irq(irq, GPU_DECMP_irq_handler, IRQF_SHARED, "se", (void *)&gDecConfig);
		if (result) {
			pr_err("gdecmp: can't get request_irq. ret=%d\n", result);
			/* fail, but we continue. */
			result = 0;
		}
		else {
			//ok

			gDecConfig.irq_no = irq;
			//pr_alert(" %s %d irq ok\n", __FUNCTION__, __LINE__);
			
		}
	}


	return result;
}


int __init refdecmp_init(void) {
	int result;

	result = register_chrdev(refdecmp_major, RTKREFDECMP_NAME, &refdecmp_fops);
	if (result < 0) {
		pr_err("refdecmp: can not register (%d %d)...\n", refdecmp_major, refdecmp_minor);
		return -ENODEV;
	}

	if (refdecmp_major == 0)
		refdecmp_major = result;  // dynamic

	refdecmp_class = class_create(THIS_MODULE, RTKREFDECMP_NAME);
	if (IS_ERR(refdecmp_class))
		return PTR_ERR(refdecmp_class);


	
	refdecmp_class->devnode = refdecmp_devnode;
	device_create(refdecmp_class, NULL, MKDEV(refdecmp_major, refdecmp_minor), NULL, RTKREFDECMP_NAME);

	pr_info("gdecmp major number: %d\n", refdecmp_major);

	
	#ifdef RTK_ENABLE_DECMP_INTERRUPT
		Gpu_Set_Dec_init( 1,  refdecmp_class);
	
	#else
	
		Gpu_Set_Dec_init( 0, refdecmp_class);
	#endif


	
#ifdef CONFIG_PM
		gdecmp_devs = platform_device_register_simple(RTKREFDECMP_NAME, -1, NULL, 0);
		result = platform_driver_register(&gdecmp_driver);
		if ((result) != 0) {
			pr_emerg("Can't register gdecmp device driver %d\n", result);
		} else {
			pr_debug("register gdecmp device driver...\n");
		}
	
#endif


	return 0;

	
}

void __exit refdecmp_exit(void) {


	device_destroy(refdecmp_class, MKDEV(refdecmp_major, refdecmp_minor));
	class_destroy(refdecmp_class);
	unregister_chrdev(refdecmp_major, RTKREFDECMP_NAME);

	
}

#ifdef MODULE
/* Register Macros */
module_init(refdecmp_init);
module_exit(refdecmp_exit);
#else
fs_initcall(refdecmp_init);
#endif

MODULE_AUTHOR("Realtek.com");
MODULE_LICENSE("Dual BSD/GPL");

