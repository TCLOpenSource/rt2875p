

#include "rtk_refdecmp_ver.h"



#include <rtk_kdriver/io.h>

#include <linux/version.h>
#include <linux/proc_fs.h> //debug fs 

//#include <linux/rwlock.h>
#include <linux/spinlock.h>
#include <linux/rwlock.h>
#include <linux/delay.h>
#include <linux/uaccess.h>



// default disable, to prevent dcmt callback can't pass compiling on new platform
#define DECOMP_DCMT_UNIT_TEST

// #define ENABLE_GDECMP_RECORD 

#if IS_ENABLED(CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER) && defined(DECOMP_DCMT_UNIT_TEST)
	
#include <rtk_dc_mt.h>

#endif//


#ifdef RTK_DUMP_VE_FILE
#include <linux/fs.h>
#include <asm/segment.h>
#include <linux/uaccess.h>
#include <linux/buffer_head.h>

#include "rbus/sb2_reg.h"


#endif//


int gEnableCheckRecord = 0;  //change to 0 when stable

int gEnableCheckRecord_DEBUGLOG = 1;


int gEnableDecmpError_DUMP = 1;  //change to 0 when stable


#if 0 //AAA
///
static const unsigned int regs_100_match[] = { GPU_decmp_ctrl_reg,  GPU_decmp_status_reg};

static const unsigned int regs_100_match_per_region[] = { GPU_decmp_lu_3,  GPU_decmp_lu_4, GPU_decmp_lu_5, GPU_decmp_ch_3, GPU_decmp_ch_4,
	GPU_decmp_ch_5};

static const unsigned int regs_100_diff_per_region[] = {
	GPU_decmp_lu_0 , GPU_decmp_lu_1,
	 GPU_decmp_lu_2, GPU_decmp_lu_1, 
	 GPU_decmp_ch_1, GPU_decmp_lu_1,
	 GPU_decmp_ch_2, GPU_decmp_lu_1 ,
	 GPU_decmp_ch_0, GPU_decmp_lu_1,

 	 GPU_decmp_lu_0, GPU_decmp_ch_1,
 	 GPU_decmp_lu_2, GPU_decmp_ch_1,
 	 GPU_decmp_ch_2, GPU_decmp_ch_1,
 	 GPU_decmp_ch_0, GPU_decmp_ch_1,

	 GPU_decmp_lu_2, GPU_decmp_lu_0,
	 GPU_decmp_ch_0, GPU_decmp_lu_0,
 	 GPU_decmp_ch_2, GPU_decmp_lu_0,
};


extern unsigned int debug_regs[] ;

extern unsigned int debug_regs_per_set[];


static unsigned int reg_set_2k[] = {
		   0xb8069000, 0x30000001,
			 0xb8069800, 0x0000ff00,
			 0xb8069a00, 0x00000000,
			 0xb8069a04, 0x00000000,
			 0xb8069a08, 0x00000000,
			 0xb8007480, 0xffffff1f,
	0xb8069100, 0x50078101,
	0xb8069200, 0x08b2a000,
	0xb8069240, 0x08800000,
	0xb8069280, 0x089fa400,
	0xb80692c0, 0x00000780,
	0xb8069300, 0x00000780,
	0xb8069340, 0x00000780,
	0xb8069400, 0x08b3c000,
	0xb8069440, 0x08a1c000,
	0xb8069480, 0x08b19200,
	0xb80694c0, 0x00000780,
	0xb8069500, 0x00000780,
	0xb8069540, 0x00000780,
	0xb8069104, 0x50078101,
	0xb8069204, 0x08f2a000,
	0xb8069244, 0x08c00000,
	0xb8069284, 0x08dfa400,
	0xb80692c4, 0x00000780,
	0xb8069304, 0x00000780,
	0xb8069344, 0x00000780,
	0xb8069404, 0x08f3c000,
	0xb8069444, 0x08e1c000,
	0xb8069484, 0x08f19200,
	0xb80694c4, 0x00000780,
	0xb8069504, 0x00000780,
	0xb8069544, 0x00000780,
	0xb8069108, 0x50078101,
	0xb8069208, 0x0932a000,
	0xb8069248, 0x09000000,
	0xb8069288, 0x091fa400,
	0xb80692c8, 0x00000780,
	0xb8069308, 0x00000780,
	0xb8069348, 0x00000780,
	0xb8069408, 0x0933c000,
	0xb8069448, 0x0921c000,
	0xb8069488, 0x09319200,
	0xb80694c8, 0x00000780,
	0xb8069508, 0x00000780,
	0xb8069548, 0x00000780,
	0xb806910c, 0x50078101,
	0xb806920c, 0x0972a000,
	0xb806924c, 0x09400000,
	0xb806928c, 0x095fa400,
	0xb80692cc, 0x00000780,
	0xb806930c, 0x00000780,
	0xb806934c, 0x00000780,
	0xb806940c, 0x0973c000,
	0xb806944c, 0x0961c000,
	0xb806948c, 0x09719200,
	0xb80694cc, 0x00000780,
	0xb806950c, 0x00000780,
	0xb806954c, 0x00000780,
	0xb8069110, 0x50078101,
	0xb8069210, 0x09b2a000,
	0xb8069250, 0x09800000,
	0xb8069290, 0x099fa400,
	0xb80692d0, 0x00000780,
	0xb8069310, 0x00000780,
	0xb8069350, 0x00000780,
	0xb8069410, 0x09b3c000,
	0xb8069450, 0x09a1c000,
	0xb8069490, 0x09b19200,
	0xb80694d0, 0x00000780,
	0xb8069510, 0x00000780,
	0xb8069550, 0x00000780,
	0xb8069114, 0x50078101,
	0xb8069214, 0x09f2a000,
	0xb8069254, 0x09c00000,
	0xb8069294, 0x09dfa400,
	0xb80692d4, 0x00000780,
	0xb8069314, 0x00000780,
	0xb8069354, 0x00000780,
	0xb8069414, 0x09f3c000,
	0xb8069454, 0x09e1c000,
	0xb8069494, 0x09f19200,
	0xb80694d4, 0x00000780,
	0xb8069514, 0x00000780,
	0xb8069554, 0x00000780,
	0xb8069118, 0x50078101,
	0xb8069218, 0x0a32a000,
	0xb8069258, 0x0a000000,
	0xb8069298, 0x0a1fa400,
	0xb80692d8, 0x00000780,
	0xb8069318, 0x00000780,
	0xb8069358, 0x00000780,
	0xb8069418, 0x0a33c000,
	0xb8069458, 0x0a21c000,
	0xb8069498, 0x0a319200,
	0xb80694d8, 0x00000780,
	0xb8069518, 0x00000780,
	0xb8069558, 0x00000780,
	0xb806911c, 0x50078101,
	0xb806921c, 0x0a72a000,
	0xb806925c, 0x0a400000,
	0xb806929c, 0x0a5fa400,
	0xb80692dc, 0x00000780,
	0xb806931c, 0x00000780,
	0xb806935c, 0x00000780,
	0xb806941c, 0x0a73c000,
	0xb806945c, 0x0a61c000,
	0xb806949c, 0x0a719200,
	0xb80694dc, 0x00000780,
	0xb806951c, 0x00000780,
	0xb806955c, 0x00000780,
	0xb8069120, 0x50078101,
	0xb8069220, 0x7eb2a000,
	0xb8069260, 0x7e800000,
	0xb80692a0, 0x7e9fa400,
	0xb80692e0, 0x00000780,
	0xb8069320, 0x00000780,
	0xb8069360, 0x00000780,
	0xb8069420, 0x7eb3c000,
	0xb8069460, 0x7ea1c000,
	0xb80694a0, 0x7eb19200,
	0xb80694e0, 0x00000780,
	0xb8069520, 0x00000780,
	0xb8069560, 0x00000780,
	0xb8069124, 0x50078101,
	0xb8069224, 0x7ef2a000,
	0xb8069264, 0x7ec00000,
	0xb80692a4, 0x7edfa400,
	0xb80692e4, 0x00000780,
	0xb8069324, 0x00000780,
	0xb8069364, 0x00000780,
	0xb8069424, 0x7ef3c000,
	0xb8069464, 0x7ee1c000,
	0xb80694a4, 0x7ef19200,
	0xb80694e4, 0x00000780,
	0xb8069524, 0x00000780,
	0xb8069564, 0x00000780,
	0xb8069128, 0x50078101,
	0xb8069228, 0x7f32a000,
	0xb8069268, 0x7f000000,
	0xb80692a8, 0x7f1fa400,
	0xb80692e8, 0x00000780,
	0xb8069328, 0x00000780,
	0xb8069368, 0x00000780,
	0xb8069428, 0x7f33c000,
	0xb8069468, 0x7f21c000,
	0xb80694a8, 0x7f319200,
	0xb80694e8, 0x00000780,
	0xb8069528, 0x00000780,
	0xb8069568, 0x00000780,
	0xb806912c, 0x50078101,
	0xb806922c, 0x7f72a000,
	0xb806926c, 0x7f400000,
	0xb80692ac, 0x7f5fa400,
	0xb80692ec, 0x00000780,
	0xb806932c, 0x00000780,
	0xb806936c, 0x00000780,
	0xb806942c, 0x7f73c000,
	0xb806946c, 0x7f61c000,
	0xb80694ac, 0x7f719200,
	0xb80694ec, 0x00000780,
	0xb806952c, 0x00000780,
	0xb806956c, 0x00000780,
};

#ifdef CONFIG_RTK_FEATURE_FOR_GKI
#ifdef RTK_DUMP_VE_FILE
static struct file* file_open(const char* path, int flags, int rights)
{
        struct file* filp = NULL;
        mm_segment_t oldfs;
        int err = 0;

        oldfs = get_fs();
        set_fs(get_ds());
        filp = filp_open(path, flags, rights);
        set_fs(oldfs);
        if(IS_ERR(filp)) {
                err = PTR_ERR(filp);

			pr_err("dump fail when open:%s %d\n", path, err);
            return NULL;
        }
		
        return filp;
}

static void file_close(struct file* file)
{
        filp_close(file, NULL);
}

static int file_read(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size)
{
        mm_segment_t oldfs;
        int ret;

        oldfs = get_fs();
        set_fs(get_ds());

        ret = vfs_read(file, data, size, &offset);

        set_fs(oldfs);
        return ret;
}

static int file_write(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size)
{
        mm_segment_t oldfs;
        int ret;

        oldfs = get_fs();
        set_fs(get_ds());

        ret = vfs_write(file, data, size, &offset);

        set_fs(oldfs);
        return ret;
}

static int file_sync(struct file* file)
{
        vfs_fsync(file, 0);
        return 0;
}



extern void *dvr_remap_uncached_memory(unsigned long phys_addr, size_t size, const void *caller);
extern void dvr_unmap_memory(void *cpu_addr, size_t size);

/*
2k: d_y, data_uv_start,       h_y_start,    h_uv_start
      0, 2211840(1920*1152), 1920*1152*1.5, 1920*1152*1.5 + header_luma_size

*/

void decmp_dump_single_file(const char* path, int size, unsigned long phy_addr)
{
	
	char *buf_st = NULL;
	struct file * filep;


	buf_st = (char *) dvr_remap_uncached_memory(phy_addr, size, __builtin_return_address(0) );

	if( buf_st == NULL ) {
		rtd_pr_gdecmp_err("decmp can't map mem 0x%llx %d\n", phy_addr, size);
		return;
	}
	//filep = file_open(path, O_CREAT |  O_RDWR , S_IRWXU | S_IRWXG | S_IRWXO);	
	//filep = file_open(path, O_CREAT |  O_RDWR |O_LARGEFILE , 0);
	//filep = file_open(path, O_TRUNC | O_RDWR | O_CREAT, 0777);
	filep = file_open(path,  O_RDWR | O_CREAT, 0644);

	if( filep != NULL )
	{
	     char arr[100];

	     //char * str = "I just wrote something";
	     
	     file_write(filep, 0, buf_st, size  ); 

	     //memset(arr, '\0', 100);
	     //file_read(filep, 0, arr, 20);
	     rtd_pr_gdecmp_err("decmp output file %s  addr:0x%08x %d\n", path, phy_addr, size ); 

	     file_close(filep);

   }

	dvr_unmap_memory((void*)buf_st, size);
	


}


void decmp_dbg_dump_file(int file_size)
{
	char fname[64];
	int i=0;
	int data_addr = 0;
	int org_val = rtd_inl(SB2_SHADOW_2_reg);
	const int bit0 = (0x01 <<0);

	
	static int f_cnt_max = DECMP_DUMP_FILE_COUNTS;

	// 0xB801A618
	{
	#ifdef CONFIG_ARCH_RTK2851A
		// pause VE

		rtd_outl(SB2_SHADOW_2_reg, org_val | bit0);

	#else
		
	#endif//

	}		

	for ( ; i < DECMP_DUMP_FILE_COUNTS; i++)
	{
		data_addr = get_dataAddr_by_region(i);

		if(data_addr != NULL) 
		{
			//need to chmod 777 "everytime" at first
			snprintf(fname, 64, "/data/veDump/ve_%d.bin", i); //EACCES 

			decmp_dump_single_file(fname, file_size, data_addr);
		}
		
	}


	#ifdef CONFIG_ARCH_RTK2851A
		rtd_outl(SB2_SHADOW_2_reg, org_val & (~(bit0)) );
	
	#else
			
	#endif//
	
	//for pass compile 
	org_val = 0;
	
	
}
	


void decmp_dump_regions()
{
	


}


#endif// RTK_DUMP_VE_FILE
#endif // CONFIG_RTK_FEATURE_FOR_GKI

static int x_dbg_find_value(unsigned int addr, unsigned int* ret_value   )
{
	int i=0;

	for(i=0; i < sizeof(reg_set_2k)/sizeof(int);)
	{
		if( reg_set_2k[i] == addr)
		{
			* ret_value =  reg_set_2k[i+1];

			return 0;
		}

		i= i+2;
	}


	return -1;
}

// for 2K diff
void decmp_dbg_diff_register( )
{
	int i=0;
	int region_idx = 0;
	int reg_addr;
	int reg_value;
	int cur_reg_value;
	
	int diff;
		
	unsigned int num_regs = sizeof(reg_set_2k)/sizeof(int)/2;
	rtd_pr_gdecmp_err("diff #registers:%d\n", num_regs);

	//kjw TODO check if reg_set have dup entries
	

	// 100% need match
	
	for(i=0; i < sizeof(regs_100_match)/sizeof(int); i++)
	{
		reg_addr = regs_100_match[i];
		cur_reg_value = rtd_inl(reg_addr);
			
		if( x_dbg_find_value(reg_addr, &reg_value) == 0 )
		{
			if( cur_reg_value != reg_value ) 
			{
				rtd_pr_gdecmp_err("[decmpCmp] value not match addr:0x%08x  0x%08x<->0x%08x\n", reg_addr, cur_reg_value, reg_value);

				
			}
		}

	}

	/* within region rule:
		1. GPU_decmp_lu_hdrStart_reg > GPU_decmp_ch_hdr_start_reg >  GPU_decmp_ch_dataEnd_reg > GPU_decmp_ch_dataStart_reg > GPU_decmp_lu_dataEnd_reg > GPU_decmp_lu_data_start

		
	*/

	/* between region rule:
	2. offset to these should same
	
		 GPU_decmp_lu_0[0] - GPU_decmp_lu_1[0] ==  GPU_decmp_lu_0[x] - GPU_decmp_lu_1[x]
		 GPU_decmp_lu_2 - GPU_decmp_lu_1 
		 GPU_decmp_ch_1 - GPU_decmp_lu_1
		 GPU_decmp_ch_2 - GPU_decmp_lu_1 
		 GPU_decmp_ch_0 - GPU_decmp_lu_1
	
		 GPU_decmp_lu_0 - GPU_decmp_ch_1
		 GPU_decmp_lu_2 - GPU_decmp_ch_1
		 GPU_decmp_ch_2 - GPU_decmp_ch_1
		 GPU_decmp_ch_0 - GPU_decmp_ch_1
	
		 GPU_decmp_lu_2 - GPU_decmp_lu_0
		 GPU_decmp_ch_0 - GPU_decmp_lu_0
		 GPU_decmp_ch_2 - GPU_decmp_lu_0
			



	*/
	

	for(region_idx =0; region_idx<12; region_idx++)
	{
		int addr1;
		int addr2;
		int val1, val2;

		int cur_val1, cur_val2;
		
		for(i=0; i < sizeof(regs_100_diff_per_region)/sizeof(int);)
		{
			addr1 = regs_100_diff_per_region[i] + region_idx*4;
			addr2 = regs_100_diff_per_region[i+1] + region_idx*4;

			
			cur_val1 = rtd_inl(addr1);
			cur_val2 = rtd_inl(addr2);

			if( x_dbg_find_value(addr1, &val1) == 0 )
			{
				
			}
			else
			{
				rtd_pr_gdecmp_err("[decmpCmp] can't find reg 0x%08x! \n", addr1);
			}

			if( x_dbg_find_value(addr2, &val2) == 0 )
			{
				
			}
			else
			{
				rtd_pr_gdecmp_err("[decmpCmp] can't find reg 0x%08x! \n", addr2);
			}


			i= i+2;
		}


		
	}

	

}

RTK_DECMP_RECORD_TABLE gRecTable;
	

#if 1
 #define DECMPREC_CHECK_VALUE(addr, idx, val, str, ret) \
    do { \
		unsigned int retXX = rtd_inl(addr+idx*4) ; \
		if( retXX != val ) { \
			rtd_pr_gdecmp_err("[decmprec] reg: 0x%08x region:%d %s notEqual  0x%08x(reg) != 0x%08x\n", addr+idx*4,idx, str, retXX , val); \
			ret = -1; \
		} else {ret = 0;} \
    } while(0)

#else

	#define DECMPREC_CHECK_VALUE(addr, val)

#endif//


DEFINE_MUTEX(DecmpRecInfo_mutex);


RTK_DECMP_RECORD_ITEM* decmp_rec_find( unsigned long addr, int * ret_idx)
{
	RTK_DECMP_RECORD_ITEM* item = NULL;
	int i =0;

	int target_idx = REGION_INVALID_INDEX;

	
	for( ; i< DECMP_REC_NUM_REGIONS; i++ )
	{
		item = &(gRecTable.regions[i]);

		if(item->data_y_addr == addr) {
			target_idx = i;
			break;
		}
	}

	if( target_idx == REGION_INVALID_INDEX )
		return NULL;

	if(ret_idx)
		*ret_idx = target_idx;
	
	return item;
}


int decmp_rec_resetRec(unsigned long addr, int region_idx)
{
	int i = 0;
	int cur_reg_idx = REGION_INVALID_INDEX;
	rtk_decmp_export* tmp=NULL;
	RTK_DECMP_RECORD_ITEM* item= NULL;

	for( ; i < DECMP_REC_NUM_REGIONS; i++ )
	{
		tmp = &(gRecTable.temp_rec[i]);

		if( tmp->data_y_addr == addr ) {
			//reset 
			tmp->region_idx = REGION_INVALID_INDEX;
		}


			
	}

	if( region_idx != REGION_INVALID_INDEX )
	{
		item = &(gRecTable.regions[region_idx]);
		if( item->item_region_idx == REGION_INVALID_INDEX ) {
			rtd_pr_gdecmp_err("[decmp rec] invalid region ? addr:0x%"PRIx64 " idx:%d tmp->region_idx:%d\n", addr, region_idx, tmp->region_idx);
		}

		if( item->data_y_addr != addr )
		{
			rtd_pr_gdecmp_err("[decmp rec] region/addr mismatch ? addr:0x%"PRIx64 " idx:%d tmp->region_idx:%d\n", addr, region_idx, tmp->region_idx);

			item = decmp_rec_find( addr, NULL);

			if(item == NULL) {
				rtd_pr_gdecmp_err("[decmprec] no item found for addr addr:0x%"PRIx64 "\n", addr);
				return -1;
			}
			
		}

		
		
		//[FIXME] need mutex protect ?
		item->item_region_idx = REGION_INVALID_INDEX;
		item->rec_wp = 0;
		item->rec_rp = 0; 
		
	}
	else {

		rtd_pr_gdecmp_err("[decmp rec] invalid when reset rec addr:0x%"PRIx64 " idx:%d tmp->region_idx:%d\n", addr, region_idx, tmp->region_idx);
	}
	
}


	

//called when ion alloc, so region index is unknown at this time.
int decmp_rec_initRec(rtk_decmp_export* info)
{
	int i = 0;
	rtk_decmp_export* tmp=NULL;

	
	for( ; i < DECMP_REC_NUM_REGIONS; i++ )
	{
		tmp = &(gRecTable.temp_rec[i]);
		
		if( tmp->region_idx == REGION_INVALID_INDEX ) {

			memcpy(tmp, info, sizeof( rtk_decmp_export) ) ;
			tmp->region_idx = REGION_USED_INDEX;

			if( gEnableCheckRecord_DEBUGLOG )  
			{
				rtd_pr_gdecmp_err("[decmprec] init temp_rec entry:%d addr:0x%"PRIx64 " \n", i , info->data_y_addr);
				GpuDecUtil_dump_exportInfo(info);
			}
			break;
		}
		
	}

	if( tmp == NULL ) {

		rtd_pr_gdecmp_err("[decmp] decmp_rec_initRec no free entry !\n");
		GpuDecUtil_dump_exportInfo(info);
	}
	
	#if 0	
	int reg_idx = 0;


	//data addr / reg idx match, update only 
	RTK_DECMP_RECORD_ITEM* item = decmp_rec_find(info->data_y_addr, &reg_idx);
	
	decmp_rec_updateItem(item, info, reg_idx);
	//buf size & crop infomation

	//all same, update last timestamp only
	#endif//0

	return 0;
}

//note: this func may have race condiction 
void decmp_rec_dumpTempRec( void )
{ 
	rtk_decmp_export* tmp=NULL;
	int idx=0;
	
	for( idx=0; idx < DECMP_REC_NUM_REGIONS; idx++ )
	{
		tmp = &(gRecTable.temp_rec[idx]);
		rtd_pr_gdecmp_err("tmpRec:[%d] region:%d addr:0x%"PRIx64 " \n", idx, tmp->region_idx, tmp->data_y_addr);
	}

}

void decmp_rec_dumpRecord( void )
{
	
	RTK_DECMP_TIMEINFO  *recInfo=NULL;
	rtk_decmp_export* tmp=NULL;
	rwlock_t xxx_lock =  __RW_LOCK_UNLOCKED(xxx_lock);
	int idx=0;
	
	//mutex_lock(DecmpRecInfo_mutex);
	unsigned long flags;
	read_lock_irqsave(&xxx_lock, flags);

	for( idx=0; idx < DECMP_REC_NUM_REGIONS; idx++ )
	{
		tmp = &(gRecTable.temp_rec[idx]);
		rtd_pr_gdecmp_err("tmpRec:%d region:%d addr:0x%"PRIx64 " \n", idx, tmp->region_idx, tmp->data_y_addr);
	}

	
	idx= gRecTable.recList_wp -1;
	 rtd_pr_gdecmp_err("\ndecmp_rec_dumpRecord st:%d\n", idx );
	 
	for( ; idx >= 0 ; idx-- ) 
	{

		recInfo = &(gRecTable.recInfo_list[idx]);

		rtd_pr_gdecmp_err("idx:%d addr:0x%"PRIx64 " TT:%llx\n", idx, recInfo->info.data_y_addr, recInfo->recTime.last_bind_time );
	}


	idx = DECMP_TIMELIST_COUNT -1;
	for( ; idx > gRecTable.recList_wp; idx-- ) {
		
		recInfo = &(gRecTable.recInfo_list[idx]);
		rtd_pr_gdecmp_err("idx:%d addr:0x%"PRIx64 " TT:%llx\n", idx, recInfo->info.data_y_addr, recInfo->recTime.last_bind_time );
		
	}
	
	//mutex_unlock(DecmpRecInfo_mutex);
	read_unlock_irqrestore(&xxx_lock, flags);

}



rtk_decmp_export* decmp_rec_getInfo(int region_idx)
{
	rtk_decmp_export* info = NULL;
	RTK_DECMP_RECORD_ITEM* item= NULL;

	item = &(gRecTable.regions[region_idx]);
	
	if( item->item_region_idx == REGION_INVALID_INDEX ) {
		rtd_pr_gdecmp_err("decmp region:%d is invalid\n", region_idx);
		return NULL;	
	}

	
	{
		//lastest one is wp -1
		int last_p = item->rec_wp-1;
		if( last_p < 0) last_p = 0;
		
		info = &item->rec[last_p];

		if( info->region_idx != region_idx ) {

			rtd_pr_gdecmp_err("decmp_rec_getInfo recordInvalid?? recRegion:%d  yStart:0x%"PRIx64 " last_p:%d.\n", 
				info->region_idx, info->data_y_addr,  last_p,region_idx );
		}
		
	}

	return info;
}


//diff register vs. decmp_rec from DDK's view
int decmp_rec_diff(int region_idx, unsigned long addr)
{
	RTK_DECMP_RECORD_ITEM* item= NULL;
	int i=0;
	//first associate
	rtk_decmp_export* info = NULL;
	int to_diff = 0;
	int res = 0;
	int last_p = -1;

	//RTK_DECMP_RECORD_ITEM* item = decmp_rec_find(info->data_y_addr, &reg_idx);

	if( gEnableCheckRecord == 0 ) {

		return 0;
	}

	item = &(gRecTable.regions[region_idx]);

	FIND_TARGET_INFO:
	if( item->item_region_idx == REGION_INVALID_INDEX ) {
		//first update, source is from gRecTable.temp_rec

		for( ; i < DECMP_REC_NUM_REGIONS; i++ )
		{
			info = &(gRecTable.temp_rec[i]);
			
			if( info->data_y_addr == addr ) {

				break;
			}
		
		}

		if( i == DECMP_REC_NUM_REGIONS ) {
			rtd_pr_gdecmp_err("can't find from temp_rec. assumeed_region_idx:%d addr:0x%"PRIx64" \n", region_idx, addr );
			decmp_rec_dumpTempRec();
			
		}
		else {
			decmp_rec_updateItem(item, info, region_idx );
			
			rtd_pr_gdecmp_err("[decmp_rec] first update region:%d  addr:0x%"PRIx64 " from temp_rec:%d %d %d\n", region_idx, addr, i,  item->rec_wp, item->rec_rp );
		}
	}
	else if( item->data_y_addr != addr )
	{
		int regionIdx_in_table = 0; 
		//VE change region, reget corretn info (use correct item )
		//if( decmp_rec_find( addr, &regionIdx_in_table) != 0 )
		item =  decmp_rec_find( addr, &regionIdx_in_table);

		if(item) 
		{
			rtd_pr_gdecmp_err("[decmp_rec] addr:0x%"PRIx64 " table's regionIdx change from %d to %d.  \n", addr, region_idx, regionIdx_in_table);
			item = &(gRecTable.regions[regionIdx_in_table]);

			goto FIND_TARGET_INFO;
		}
		else {
			//invalid return !

			rtd_pr_gdecmp_err("[decmp_rec] Error VE change region,   addr:0x%"PRIx64 " org idxInRsgister:%d   \n", addr, region_idx);

			return -1;
		}


	}
	else {
		// > 1 update, source is from gRecTable.regions[last_p]
		
		//lastest one is wp -1
		last_p = item->rec_wp-1;
		if( last_p < 0) last_p = 0;
		
		info = &item->rec[last_p];

		if( info->data_y_addr != addr )
		{	
		  //should not happen ?
		  rtd_pr_gdecmp_err(" [decmp_rec] region addr mismatch? recRegion:%d  addr:0x%"PRIx64 " itemAddr:0x%"PRIx64 " infoAddr 0x%"PRIx64 "\n", region_idx, addr, 
		  item->data_y_addr, info->data_y_addr );

		}
		else if( info->region_idx != region_idx ) {

			rtd_pr_gdecmp_err("[decmp_rec] recordInvalid?? recRegion:%d  yStart:0x%"PRIx64 " last_p:%d  curBind:%d 0x%"PRIx64 " \n", info->region_idx, info->data_y_addr,  last_p,
				region_idx, addr );
		}
		
		
		to_diff = 1;

	}

	{
		unsigned int region_reg = GPU_decmp_region_reg+region_idx*4;
		unsigned int data = rtd_inl(region_reg);
		unsigned int enabled = GPU_REF_DECMP_WRAP_GPU_decmp_region_0_get_enable(data);

		if( enabled == 0 ) 
		{
			rtd_pr_gdecmp_crit("[decmp_rec] check err. region %d not Enable! \n", region_idx );
		}
		

		if( info == NULL) {
			rtd_pr_gdecmp_crit(" [decmp_rec] no info. something wrong in VE/decmp_rec flow for addr:0x%"PRIx64 "idx:%d", addr, region_idx);

			return -2;
		}
	
	//check info vs. register
	DECMPREC_CHECK_VALUE(GPU_decmp_lu_0, region_idx, info->header_y_addr, "header_yStart", res);
	
	DECMPREC_CHECK_VALUE(GPU_decmp_lu_1, region_idx, info->data_y_addr, "yStart", res);

	DECMPREC_CHECK_VALUE(GPU_REF_DECMP_WRAP_GPU_decmp_lu_2_0_reg, region_idx, info->data_y_addr+info->data_y_size, "yEnd", res);	

	
	DECMPREC_CHECK_VALUE(GPU_decmp_lu_3, region_idx, info->pitch, "y_gpuPitch", res);
	
	DECMPREC_CHECK_VALUE(GPU_decmp_lu_4, region_idx, info->cmp_pitch, "y_cmpPitch", res);
	if( res) {
		rtd_pr_gdecmp_err("**decmp Warn recUpdate y_cmpPitch from %d to %d %d %d\n", info->cmp_pitch, rtd_inl(GPU_decmp_lu_4+region_idx*4), last_p, to_diff );
		//only allow vePitch/uv_vePitch update dynamic (adaptive resolution change)
		info->cmp_pitch = rtd_inl(GPU_decmp_lu_4+region_idx*4);
	}
	
	//DECMPREC_CHECK_VALUE(GPU_decmp_lu_5, region_idx, mem_pitch);
	
	DECMPREC_CHECK_VALUE(GPU_decmp_lu_memPitch_reg, region_idx, info->mem_pitch, "y_memPitch", res);
	if( res) {
		rtd_pr_gdecmp_err("**decmp Warn update y_mem_pitch from %d to %d\n", info->mem_pitch, rtd_inl(GPU_decmp_lu_memPitch_reg+region_idx*4)  );
		//only allow mem/cmp pith update dynamic (adaptive resolution change)
		info->mem_pitch = rtd_inl(GPU_decmp_lu_memPitch_reg+region_idx*4);
	}

	//double confirm luma/chroma's pitch is same
	DECMPREC_CHECK_VALUE(GPU_decmp_lu_cmpPitch_reg, region_idx, rtd_inl(GPU_decmp_ch_cmpPitch_reg+region_idx*4), "y/uv cmpPitch", res);
	DECMPREC_CHECK_VALUE(GPU_decmp_lu_memPitch_reg, region_idx, rtd_inl(GPU_decmp_ch_memPitch_reg+region_idx*4), "y/uv memPitch", res);



	DECMPREC_CHECK_VALUE(GPU_decmp_ch_0, region_idx, info->header_uv_addr, "header_uvStart", res);
   
	
	DECMPREC_CHECK_VALUE(GPU_decmp_ch_1, region_idx, info->data_uv_addr, "uvStart", res);
	
	DECMPREC_CHECK_VALUE(GPU_decmp_ch_2, region_idx, info->data_uv_addr+info->data_uv_size, "uvEnd", res);	
	
	DECMPREC_CHECK_VALUE(GPU_decmp_ch_3, region_idx, info->pitch, "uv_gpuPitch", res);
	


	}
	
	//DECMP_CHECK_VALUE(GPU_decmp_ch_5+index*4, ch_mem_pitch);

	if( to_diff ) {
		
		decmp_rec_updateItem(item, info, region_idx );
	}
	
	return 0;
}

extern u64 getVcpuClock(void);


int decmp_rec_updateItem( RTK_DECMP_RECORD_ITEM* item, rtk_decmp_export* info, int idx)
{
	int is_different = 0;
	
	rtk_decmp_export *rec = &item->rec[item->rec_wp];
	
	if( item->item_region_idx == REGION_INVALID_INDEX) {

		item->item_region_idx = idx;
	}
	else if ( item->item_region_idx != idx)	{
		rtd_pr_gdecmp_err("[decmp_rec] index changed %d to %d\n", item->item_region_idx, idx);
		
	}

	item->data_y_addr = info->data_y_addr;
		
	memcpy(rec, info, sizeof(rtk_decmp_export) );

	if( info->region_idx == REGION_USED_INDEX) {
		rec->region_idx = idx;
	}

	item->rec_time[item->rec_wp].last_bind_time = getVcpuClock();
	
	item->rec_wp++;


	if( item->rec_wp >= DECMP_RECITEM_COUNT ) {
		item->rec_wp = 0;
	}

	if( item->rec_rp == item->rec_wp ) {
		//rtd_pr_gdecmp_err("[decmp_rec] overflow reg:%d  [%d %d]\n", idx, item->rec_rp, item->rec_wp );
	}

	{
		rwlock_t xxx_lock =  __RW_LOCK_UNLOCKED(xxx_lock);;
		unsigned long flags;

		write_lock_irqsave(&xxx_lock, flags);
		
			RTK_DECMP_TIMEINFO  *recInfo = &(gRecTable.recInfo_list[gRecTable.recList_wp]);
			recInfo->recTime.last_bind_time = -1; //for debug,
			gRecTable.recList_wp++;
			
			if( gRecTable.recList_wp >= DECMP_TIMELIST_COUNT ) {
				gRecTable.recList_wp = 0;
			}

		write_unlock_irqrestore(&xxx_lock, flags);

		recInfo->recTime.last_bind_time = getVcpuClock();
		memcpy(&recInfo->info, info, sizeof(rtk_decmp_export) );
				
	}

	
	return 0;
}



int decmp_rec_reset( void )
{
	int i =0;
	RTK_DECMP_RECORD_ITEM* item;;
	memset(&gRecTable, 0, sizeof(gRecTable) );

	for( ; i< DECMP_REC_NUM_REGIONS; i++ )
	{
		item = &(gRecTable.regions[i]);
		item->item_region_idx = REGION_INVALID_INDEX;

		gRecTable.temp_rec[i].region_idx = REGION_INVALID_INDEX;
	}




	return 0;
}

#endif// 0 AAA


struct proc_dir_entry *rtkdecmp_proc_dir=0;   // rtkdecmp_proc_dir
struct proc_dir_entry *rtkdecmp_proc_entry=0;  //rtkdecmp_proc_entry
#define RTKDECMP_PROC_DIR "rtkdecmp"
#define RTKDECMP_PROC_ENTRY "dbg"

#define RTKDECMP_MAX_CMD_LENGTH (256)



ssize_t GpuDec_read   (struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
ssize_t GpuDec_write  (struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
loff_t  GpuDec_llseek (struct file *filp, loff_t off, int whence);
//int     GpuDec_ioctl  (struct file *filp, unsigned int cmd, unsigned long arg);
int     GpuDec_open   (struct inode *inode, struct file *filp);
int     GpuDec_release(struct inode *inode, struct file *filp);


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0))
struct proc_ops gpuDecmp_fops
= {
    .proc_lseek   =    GpuDec_llseek,
    .proc_read     =    GpuDec_read,
    .proc_write    =    GpuDec_write,
    .proc_ioctl    =    NULL,
    .proc_open     =    GpuDec_open,
    .proc_release  =    GpuDec_release,
#ifdef CONFIG_COMPAT
    .proc_compat_ioctl   = NULL,
#endif
};
#else
struct file_operations gpuDecmp_fops
	= {
	.owner    =    THIS_MODULE,
	.llseek   =    GpuDec_llseek,
	.read     =    GpuDec_read,
	.write    =    GpuDec_write,
	.unlocked_ioctl    =    NULL,
	.open     =    GpuDec_open,
	.release  =    GpuDec_release,
#ifdef CONFIG_COMPAT
	.compat_ioctl   = NULL,
#endif
};
#endif


static const char *decmp_cmd_str[] = {
	"dump",
	"check",
	"errDump",
	"dcmt",
	"log", // DECMP_DBG_MEM_LIFE  , log level for mem life-cycle
	"status",  //dump status
	"tt",
};

typedef enum {
	DECMP_DBG_DUMP = 0,
	DECMP_DBG_CHECK_RECORD,
	DECMP_DBG_ERROR_DUMP,
	DECMP_DBG_DCMT,
	DECMP_DBG_MEM_LIFE,
	DECMP_DBG_STATUS,
	DECMP_DBG_DUMP_NUMBER
} DECMP_DBG_CMD_INDEX_T;

static inline bool decmp_dbg_parse_value(char *cmd_pointer, long long *parsed_data)
{
	if (kstrtoll(cmd_pointer, 0, parsed_data) == 0) {
		return true;
	} else {
		return false;
	}
}


#define DECMP_CROSS_READ_SIZE (128*1024)


#if IS_ENABLED(CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER) && defined(DECOMP_DCMT_UNIT_TEST)

//static void decomp_dcmt_callback(void);
//extern int dump_dcmt_last_set(void);
extern int dcmt_set_monitor(unsigned int entry, DC_MT_DESC * pDesc);
//DCMT_DEBUG_INFO_DECLARE(gpu_decomp, decomp_dcmt_ballback);

//static unsigned long  g_yStart_for_dcmt = 0;
//static unsigned long  g_uvStart_for_dcmt = 0;

void set_dcmt_header(int index, 	int dcmt_idx )
{
	DC_MT_DESC mt_desc;

	int y_header_size = 81920;
	int uv_header_size = 40960;

			// ( uv_end, header_st -1 )
		memset(&mt_desc, 0, sizeof(mt_desc));
	    mt_desc.type = MEMORY_TYPE_SERIAL_BLOCK;
		mt_desc.start = rtd_inl(GPU_decmp_lu_0+index*4) ;
		mt_desc.end = mt_desc.start  + y_header_size - 16 ;

        mt_desc.mode = 1; //trap if module_ID and access_type match
        mt_desc.ip_entry[0].module = MODULE_ANY; //all module
        mt_desc.ip_entry[0].prot = PROT_MODE_RW; //access type write
	dcmt_set_monitor(dcmt_idx, &mt_desc);


		// ( header_end,  header_end + 2 MB )
		dcmt_idx++;
		memset(&mt_desc, 0, sizeof(mt_desc));
	    mt_desc.type = MEMORY_TYPE_SERIAL_BLOCK;
		mt_desc.start = rtd_inl(GPU_decmp_ch_0+index*4) ;
		mt_desc.end = mt_desc.start   + uv_header_size - 16;

        mt_desc.mode = 1; //trap if module_ID and access_type match
        mt_desc.ip_entry[0].module = MODULE_ANY; //all module
        mt_desc.ip_entry[0].prot = PROT_MODE_RW; //access type write
	dcmt_set_monitor(dcmt_idx, &mt_desc);

}


void set_dcmt_gpu_addr(int index, int dcmt_idx )
{
	DC_MT_DESC mt_desc;


			// ( uv_end, header_st -1 )
		memset(&mt_desc, 0, sizeof(mt_desc));
	    mt_desc.type = MEMORY_TYPE_SERIAL_BLOCK;
		mt_desc.start = rtd_inl(GPU_decmp_lu_GPUdataStart_reg+index*4) ;
		mt_desc.end = rtd_inl(GPU_decmp_lu_dataEnd_reg) ;

        mt_desc.mode = 1; //trap if module_ID and access_type match
        mt_desc.ip_entry[0].module = MODULE_ANY; //all module
        mt_desc.ip_entry[0].prot = PROT_MODE_RW; //access type write
	dcmt_set_monitor(dcmt_idx, &mt_desc);


		// ( header_end,  header_end + 2 MB )
		dcmt_idx++;
		memset(&mt_desc, 0, sizeof(mt_desc));
	    mt_desc.type = MEMORY_TYPE_SERIAL_BLOCK;
		mt_desc.start = rtd_inl(GPU_decmp_ch_GPUdataStart_reg+index*4) ;
		mt_desc.end = rtd_inl(GPU_decmp_ch_dataEnd_reg);

        mt_desc.mode = 1; //trap if module_ID and access_type match
        mt_desc.ip_entry[0].module = MODULE_ANY; //all module
        mt_desc.ip_entry[0].prot = PROT_MODE_RW; //access type write
	dcmt_set_monitor(dcmt_idx, &mt_desc);

}


void set_dcmt_ve_addr(int index, int dcmt_idx )
{
	DC_MT_DESC mt_desc;

	int y_size = 2457600;  // luma_1920x1280_lossless.data
	int uv_size = 1228800; // chroma_1920x640_lossless.data

			// ( uv_end, header_st -1 )
		memset(&mt_desc, 0, sizeof(mt_desc));
	    mt_desc.type = MEMORY_TYPE_SERIAL_BLOCK;
		mt_desc.start = rtd_inl(GPU_decmp_lu_VEdataStart_reg+index*4) ;
		mt_desc.end = mt_desc.start  + y_size - 16 ;

        mt_desc.mode = 1; //trap if module_ID and access_type match
        mt_desc.ip_entry[0].module = MODULE_ANY; //all module
        mt_desc.ip_entry[0].prot = PROT_MODE_RW; //access type write
	dcmt_set_monitor(dcmt_idx, &mt_desc);


		// ( header_end,  header_end + 2 MB )
		dcmt_idx++;
		memset(&mt_desc, 0, sizeof(mt_desc));
	    mt_desc.type = MEMORY_TYPE_SERIAL_BLOCK;
		mt_desc.start = rtd_inl(GPU_decmp_ch_VEdataStart_reg+index*4) ;
		mt_desc.end = mt_desc.start  + uv_size - 16 ;;

        mt_desc.mode = 1; //trap if module_ID and access_type match
        mt_desc.ip_entry[0].module = MODULE_ANY; //all module
        mt_desc.ip_entry[0].prot = PROT_MODE_RW; //access type write
	dcmt_set_monitor(dcmt_idx, &mt_desc);

}


#if 0

static void decomp_dcmt_callback(void)
{
	dcmt_trap_info trap_info;
	unsigned long trash_addr = 0;
	int x=0;
	int y = 0;
	int x1=0;
	int pitch = 3840;
	int chroma_height = 2160/2; 

	
#if defined(CONFIG_ARM64)
	volatile unsigned long wi_addr;
#else
	volatile unsigned int wi_addr;
#endif

	get_dcmt_trap_info(&trap_info);
	trash_addr = trap_info.trash_addr;

	rtd_pr_gdecmp_err(" %s get 0x%lx\n", __FUNCTION__, trap_info.trash_addr );
	if( trash_addr > g_uvStart_for_dcmt ) 
	{
		x = ( trash_addr - g_uvStart_for_dcmt) %pitch  ;
		y = (trash_addr -  g_uvStart_for_dcmt ) / pitch ;

		x1 = ( trash_addr - (g_uvStart_for_dcmt + y*pitch) ) ;

		rtd_pr_gdecmp_err(" \t trash cord: chroma [x y] = [%d %d] x1:%d pitch:%d\n", x,y, x1, pitch);

	}
	else if ( trash_addr >= g_yStart_for_dcmt && trash_addr <= g_uvStart_for_dcmt ) 
	{
		x = ( trash_addr - g_yStart_for_dcmt) %pitch  ;
		y = (trash_addr -  g_yStart_for_dcmt ) / pitch ;
		 
		x1 = ( trash_addr - (g_yStart_for_dcmt + y*pitch) ) ;

		rtd_pr_gdecmp_err(" \t trash cord:  luma [x y] = [%d %d] x1:%d pitch:%d\n", x,y, x1, pitch);
	}
	else 
	{
		rtd_pr_gdecmp_err("%s trash cord: invalid address?? 0x%"PRIx64 "  \n", __FUNCTION__, trash_addr);
	}

	dump_dcmt_last_set();
}

#endif//


#endif// defined(CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER) && defined(DECOMP_DCMT_UNIT_TEST)

//#ifdef CONFIG_FAKEMEMORY_GRAPHICS
extern phys_addr_t memblock_end_of_DRAM(void);
//#endif//


void set_check_region_addr(int region_idx )
{
	DC_MT_DESC mt_desc;
	int dcmt_idx = 8;

	int ve_size = 0xA70FE0;  //1080p. 75%


  // A1:  ( fake_mem_start, g1 -4), all not allow
  
	memset(&mt_desc, 0, sizeof(mt_desc));
		mt_desc.type = MEMORY_TYPE_SERIAL_BLOCK;
		mt_desc.start = memblock_end_of_DRAM() ;
		mt_desc.end = rtd_inl(GPU_decmp_lu_GPUdataStart_reg + region_idx*4) - 4;

		mt_desc.mode = 1; //trap if module_ID and access_type match
		mt_desc.ip_entry[0].module = MODULE_ANY; //all module
		mt_desc.ip_entry[0].prot = PROT_MODE_RW; //access type write
	dcmt_set_monitor(dcmt_idx, &mt_desc);



		// A2: (g2, g2) allow gpu only 
	dcmt_idx++;
	memset(&mt_desc, 0, sizeof(mt_desc));
		mt_desc.type = MEMORY_TYPE_SERIAL_BLOCK;
		mt_desc.start = rtd_inl(GPU_decmp_lu_GPUdataStart_reg + region_idx*4) ;
		mt_desc.end = rtd_inl(GPU_decmp_lu_dataEnd_reg + region_idx*4) ;
	
		mt_desc.mode = 2; //trap if module_ID and access_type match
		mt_desc.ip_entry[0].module = MODULE_GPU; //all module
		mt_desc.ip_entry[0].prot = PROT_MODE_RW; //access type write
	dcmt_set_monitor(dcmt_idx, &mt_desc);


	// A3: ( > g2, end) not allow
	dcmt_idx++;
	memset(&mt_desc, 0, sizeof(mt_desc));
		mt_desc.type = MEMORY_TYPE_SERIAL_BLOCK;
		mt_desc.start = rtd_inl(GPU_decmp_lu_dataEnd_reg + region_idx*4) + 4;
		mt_desc.end = 
	
		mt_desc.mode = 2; //trap if module_ID and access_type match
		mt_desc.ip_entry[0].module = MODULE_GPU; //all module
		mt_desc.ip_entry[0].prot = PROT_MODE_RW; //access type write
	dcmt_set_monitor(dcmt_idx, &mt_desc);

	
dcmt_idx = 16;

	// B1: no one can read/ (maybe I frame ref by ve ? )
	memset(&mt_desc, 0, sizeof(mt_desc));
		mt_desc.type = MEMORY_TYPE_SERIAL_BLOCK;
		mt_desc.start =  rtd_inl(GPU_decmp_lu_VEdataStart_reg + region_idx*4) ;
		mt_desc.end = mt_desc.start + ve_size;
	
		mt_desc.mode = 1; //trap if module_ID and access_type match
		mt_desc.ip_entry[0].module = MODULE_ANY; //all module
		mt_desc.ip_entry[0].prot = PROT_MODE_RW; //access type write

	dcmt_set_monitor(dcmt_idx, &mt_desc);
	

}


int decmp_dbg_dcmt(int type, int param_1)
{
#if  IS_ENABLED(CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER) && defined(DECOMP_DCMT_UNIT_TEST)

//	DC_MT_DESC mt_desc;
//	int dcmt_idx = 0;

	#if 0
	static int g_register_dcmt = 0;
	if( g_register_dcmt == 0) {
		g_register_dcmt = 1;
		DCMT_DEBUG_INFO_REGISTER(gpu_decomp, decomp_dcmt_ballback );
	}
	#endif//
	
	if( param_1 >= DC_MT_ENTRY_COUNT )  //dcmt idx 
	{

		rtd_pr_gdecmp_crit("invalid idx :%d\n",param_1 );

		return -1;
	}

	if( type == 11 )
	{
		set_dcmt_gpu_addr(0, param_1);
		
		return 0;
	}
	else if ( type == 12 )
	{
		set_dcmt_ve_addr(0, param_1);

		return 0;
	}
	else if( type == 13 )
	{
		set_dcmt_header(0, param_1);

		return 0;
	}

	if( type == 21 )
	{  //video pause + check 1 region only ( +fake mem )

		set_check_region_addr( param_1 );

	}
	
 #if 0
	if( type == 1 )
	{ // test when  region bit enable
	
		rtk_decmp_export* info = NULL;
		int region_idx = param_1;

		info = decmp_rec_getInfo(region_idx);

		if (info == NULL) {
			rtd_pr_gdecmp_err("decmp_dbg_dcmt get info fail ! %d %d\n", type, param_1);
			return -1;
		}

		#if 0
		//check luma 
		memset(&mt_desc, 0, sizeof(mt_desc));
	    mt_desc.type = MEMORY_TYPE_SERIAL_BLOCK;
		mt_desc.start = info->data_y_addr - DECMP_CROSS_READ_SIZE;
		mt_desc.end = info->data_y_addr - 1;

        mt_desc.mode = 1; //trap if module_ID and access_type match
        mt_desc.ip_entry[0].module = MODULE_ANY; //all module
        mt_desc.ip_entry[0].prot = PROT_MODE_RW; //access type write
	dcmt_set_monitor(dcmt_idx, &mt_desc);

		dcmt_idx++;
		memset(&mt_desc, 0, sizeof(mt_desc));
	    mt_desc.type = MEMORY_TYPE_SERIAL_BLOCK;
		mt_desc.start = info->data_y_addr + info->data_y_size;
		mt_desc.end = mt_desc.start + DECMP_CROSS_READ_SIZE;

        mt_desc.mode = 1; //trap if module_ID and access_type match
        mt_desc.ip_entry[0].module = MODULE_ANY; //all module
        mt_desc.ip_entry[0].prot = PROT_MODE_RW; //access type write
	dcmt_set_monitor(dcmt_idx, &mt_desc);


		//check chroma
		dcmt_idx++;
		memset(&mt_desc, 0, sizeof(mt_desc));
	    mt_desc.type = MEMORY_TYPE_SERIAL_BLOCK;
		mt_desc.start = info->data_uv_addr - DECMP_CROSS_READ_SIZE;
		mt_desc.end = info->data_uv_addr - 1;

        mt_desc.mode = 1; //trap if module_ID and access_type match
        mt_desc.ip_entry[0].module = MODULE_ANY; //all module
        mt_desc.ip_entry[0].prot = PROT_MODE_RW; //access type write
	dcmt_set_monitor(dcmt_idx, &mt_desc);


		dcmt_idx++;
		memset(&mt_desc, 0, sizeof(mt_desc));
	    mt_desc.type = MEMORY_TYPE_SERIAL_BLOCK;
		mt_desc.start = info->data_uv_addr + info->data_uv_size;
		mt_desc.end = mt_desc.start + DECMP_CROSS_READ_SIZE;

        mt_desc.mode = 1; //trap if module_ID and access_type match
        mt_desc.ip_entry[0].module = MODULE_ANY; //all module
        mt_desc.ip_entry[0].prot = PROT_MODE_RW; //access type write
	dcmt_set_monitor(dcmt_idx, &mt_desc);
	
		//check header
		dcmt_idx++;
		memset(&mt_desc, 0, sizeof(mt_desc));
	    mt_desc.type = MEMORY_TYPE_SERIAL_BLOCK;
		mt_desc.start = info->header_y_addr - DECMP_CROSS_READ_SIZE;
		mt_desc.end = info->header_y_addr - 1;

        mt_desc.mode = 1; //trap if module_ID and access_type match
        mt_desc.ip_entry[0].module = MODULE_ANY; //all module
        mt_desc.ip_entry[0].prot = PROT_MODE_RW; //access type write
	dcmt_set_monitor(dcmt_idx, &mt_desc);


		dcmt_idx++;
		memset(&mt_desc, 0, sizeof(mt_desc));
	    mt_desc.type = MEMORY_TYPE_SERIAL_BLOCK;
		mt_desc.start = info->header_y_addr +  info->header_y_size + info->header_uv_size;
		mt_desc.end = mt_desc.start + DECMP_CROSS_READ_SIZE;

        mt_desc.mode = 1; //trap if module_ID and access_type match
        mt_desc.ip_entry[0].module = MODULE_ANY; //all module
        mt_desc.ip_entry[0].prot = PROT_MODE_RW; //access type write
	dcmt_set_monitor(dcmt_idx, &mt_desc);

		#else
			// ( - 2 MB, y_start -1 )
		memset(&mt_desc, 0, sizeof(mt_desc));
	    mt_desc.type = MEMORY_TYPE_SERIAL_BLOCK;
		mt_desc.start = info->data_y_addr - 10*1024*1024;
		mt_desc.end = info->data_y_addr - 1;

        mt_desc.mode = 1; //trap if module_ID and access_type match
        mt_desc.ip_entry[0].module = MODULE_ANY; //all module
        mt_desc.ip_entry[0].prot = PROT_MODE_RW; //access type write
	dcmt_set_monitor(dcmt_idx, &mt_desc);

			// (y end, chroma_start -1)
		dcmt_idx++;
		memset(&mt_desc, 0, sizeof(mt_desc));
	    mt_desc.type = MEMORY_TYPE_SERIAL_BLOCK;
		mt_desc.start = info->data_y_addr + info->data_y_size;
		mt_desc.end = info->data_uv_addr - 1;;


        mt_desc.mode = 1; //trap if module_ID and access_type match
        mt_desc.ip_entry[0].module = MODULE_ANY; //all module
        mt_desc.ip_entry[0].prot = PROT_MODE_RW; //access type write
	dcmt_set_monitor(dcmt_idx, &mt_desc);


		// ( uv_end, header_st -1 )
		dcmt_idx++;
		memset(&mt_desc, 0, sizeof(mt_desc));
	    mt_desc.type = MEMORY_TYPE_SERIAL_BLOCK;
		mt_desc.start = info->data_uv_addr + info->data_uv_size;
		mt_desc.end = info->header_y_addr - 1;

        mt_desc.mode = 1; //trap if module_ID and access_type match
        mt_desc.ip_entry[0].module = MODULE_ANY; //all module
        mt_desc.ip_entry[0].prot = PROT_MODE_RW; //access type write
	dcmt_set_monitor(dcmt_idx, &mt_desc);


		// ( header_end,  header_end + 2 MB )
		dcmt_idx++;
		memset(&mt_desc, 0, sizeof(mt_desc));
	    mt_desc.type = MEMORY_TYPE_SERIAL_BLOCK;
		mt_desc.start = info->header_y_addr +  info->header_y_size + info->header_uv_size;
		mt_desc.end = mt_desc.start + 10*1024*1024;

        mt_desc.mode = 1; //trap if module_ID and access_type match
        mt_desc.ip_entry[0].module = MODULE_ANY; //all module
        mt_desc.ip_entry[0].prot = PROT_MODE_RW; //access type write
	dcmt_set_monitor(dcmt_idx, &mt_desc);


	
		#endif// 0

		
		rtd_pr_gdecmp_err("\t dcmt y_start 0x%"PRIx64 " uv_start 0x%"PRIx64 " header_start 0x%"PRIx64 "\n",
			info->data_y_addr, info->data_uv_addr, info->header_y_addr );
	}
	else if( type == 2 )
	{ // test when  region is disabled, check bottom-halt 

		rtk_decmp_export* info = NULL;
		int region_idx = param_1;

		int x_start = 0;
		int y_start = 1080;
		int width = 3840;
		int height_to_cover = 1080;
		int luma_cover_size = width * height_to_cover;

		info = decmp_rec_getInfo(region_idx);

		if (info == NULL) {
			rtd_pr_gdecmp_err("decmp_dbg_dcmt get info fail ! %d %d\n", type, param_1);
			return -1;
		}

			//check luma bottom half
		memset(&mt_desc, 0, sizeof(mt_desc));
	    mt_desc.type = MEMORY_TYPE_SERIAL_BLOCK;
		mt_desc.start = info->data_y_addr + x_start*width + y_start*width;
		mt_desc.end = info->data_uv_addr - 1;  // or - N lines

        mt_desc.mode = 1; //trap if module_ID and access_type match
        mt_desc.ip_entry[0].module = MODULE_ANY; //all module
        mt_desc.ip_entry[0].prot = PROT_MODE_RW; //access type write
	dcmt_set_monitor(dcmt_idx, &mt_desc);


			//check chroma bottom half
		dcmt_idx++;
		memset(&mt_desc, 0, sizeof(mt_desc));
	    mt_desc.type = MEMORY_TYPE_SERIAL_BLOCK;
		//chroma height is height/2
		mt_desc.start = info->data_uv_addr + x_start*width + y_start/2*width;  
		mt_desc.end = info->header_y_addr - 1; // or - N lines

        mt_desc.mode = 1; //trap if module_ID and access_type match
        mt_desc.ip_entry[0].module = MODULE_ANY; //all module
        mt_desc.ip_entry[0].prot = PROT_MODE_RW; //access type write
	dcmt_set_monitor(dcmt_idx, &mt_desc);

		rtd_pr_gdecmp_err("\t dcmt gpuAddr y_start 0x%"PRIx64 " uv_start 0x%"PRIx64 " header_start 0x%"PRIx64 " y_start:%d\n",
			info->data_y_addr, info->data_uv_addr, info->header_y_addr, y_start );

	}
	else if( type == 3 )
	{ // scan top-right corner, co-work with type 2
		int cur_line = 0;	
		int j = 0;
		int dcmt_entry_count = 16 - 2;
		int dcmt_start_idx = 2;
		// 3 rules: luma line x + luma line x+1 + chroma line x  
		// 13 /3 = 4 -> each DCMT iteration can check 8 luma lines( & 4 chroma lines)
		int rule_per_set = dcmt_entry_count/3;	
		rtk_decmp_export* info = NULL;
		
		int region_idx = param_1;

		int total_line_count = 1080; 

		int x_start = 1920;
		int y_start = 0;
		int gpu_pitch = 3840;
		int video_width = 1920;

		//first 2 entry are used in type 2
		dcmt_idx = 2;

		info = decmp_rec_getInfo(region_idx);

		if (info == NULL) {
			rtd_pr_gdecmp_err("decmp_dbg_dcmt get info fail ! %d %d\n", type, param_1);
			return -1;
		}

		rtd_pr_gdecmp_err("\t dcmt check right-bottom y_start 0x%"PRIx64 " uv_start 0x%"PRIx64 " header_start 0x%"PRIx64 " \n",
			info->data_y_addr, info->data_uv_addr, info->header_y_addr );

		g_yStart_for_dcmt = info->data_y_addr;
		g_uvStart_for_dcmt =  info->data_uv_addr ;
	
		cur_line = y_start;
		//
	   	for( cur_line=0; cur_line < total_line_count; cur_line++ )
   		{
   			dcmt_idx = dcmt_start_idx;
			
			//chroma's height is half
			
   			for( j=0; j < rule_per_set; j++ )
			{
						//check luma cur_line 
					memset(&mt_desc, 0, sizeof(mt_desc));
				    mt_desc.type = MEMORY_TYPE_SERIAL_BLOCK;
					mt_desc.start = info->data_y_addr + x_start + cur_line*gpu_pitch;
					mt_desc.end = mt_desc.start  + gpu_pitch/2 -1 ;  // or - N lines

			        mt_desc.mode = 1; //trap if module_ID and access_type match
			        mt_desc.ip_entry[0].module = MODULE_ANY; //all module
			        mt_desc.ip_entry[0].prot = PROT_MODE_RW; //access type write

					//rtd_pr_gdecmp_err("\t aa1:%d cur_line:%d 0x%x 0x%x \n", dcmt_idx, cur_line, mt_desc.start , mt_desc.end );
					
				dcmt_set_monitor(dcmt_idx, &mt_desc);


						//check chroma  cur_line 
					dcmt_idx++;
					memset(&mt_desc, 0, sizeof(mt_desc));
				    mt_desc.type = MEMORY_TYPE_SERIAL_BLOCK;
					//chroma height is height/2
					mt_desc.start = info->data_uv_addr + x_start + ((int)(cur_line/2))*gpu_pitch;
					mt_desc.end = mt_desc.start + gpu_pitch/2 -1 ; //

			        mt_desc.mode = 1; //trap if module_ID and access_type match
			        mt_desc.ip_entry[0].module = MODULE_ANY; //all module
			        mt_desc.ip_entry[0].prot = PROT_MODE_RW; //access type write

					//rtd_pr_gdecmp_err("\t aaChroma:%d cur_line:%d 0x%x 0x%x \n", dcmt_idx, ((int)(cur_line/2)), mt_desc.start , mt_desc.end );
						
				dcmt_set_monitor(dcmt_idx, &mt_desc);

				cur_line ++;  

				//check luma  cur_line+1
					dcmt_idx++;
					memset(&mt_desc, 0, sizeof(mt_desc));
				    mt_desc.type = MEMORY_TYPE_SERIAL_BLOCK;
					mt_desc.start = info->data_y_addr + x_start + cur_line*gpu_pitch;
					mt_desc.end = mt_desc.start  + gpu_pitch/2 -1;  //

			        mt_desc.mode = 1; //trap if module_ID and access_type match
			        mt_desc.ip_entry[0].module = MODULE_ANY; //all module
			        mt_desc.ip_entry[0].prot = PROT_MODE_RW; //access type write
					//rtd_pr_gdecmp_err("\t aa2:%d cur_line:%d 0x%x 0x%x \n", dcmt_idx, cur_line, mt_desc.start , mt_desc.end );
				dcmt_set_monitor(dcmt_idx, &mt_desc);
				dcmt_idx++;

				cur_line ++;  
				
			}

			rtd_pr_gdecmp_err("\t dcmt line start:%d lastSt 0x%x lastEnd 0x%x y_start:%d dcmtidx:[%d %d]\n",
					cur_line, mt_desc.start, mt_desc.end, y_start, dcmt_start_idx, dcmt_idx);

			usleep_range(16 * 1000, 16 * 1000); //prevent race condiction ??
   		}


	}
  #endif//

  return -1;
  
#endif//
	return -1;
}

#define VE_BUF_FILE_SIZE_2K (3428352)


#if IS_ENABLED(CONFIG_ION_DVR_HEAP)
extern void ion_dvr_enable_debug(int loglevel);
#endif//

#if IS_ENABLED(CONFIG_DMABUF_HEAPS_DVR)

extern void dvr_heap_enable_debug(int loglevel);
#endif//


static inline void decmp_dbg_EXECUTE(const int cmd_index, char **cmd_pointer)
{
//	long long parsed_data = 0;
	//char tmpStr[128] = {0};
	
	if (cmd_pointer == NULL || *cmd_pointer == NULL) {
		rtd_pr_gdecmp_err("%s cmd-%d error. cmd_pointer invalid\n", __func__, cmd_index);
		return;
	}

	switch (cmd_index) {
		case DECMP_DBG_DUMP:
			{
				#ifdef RTK_DUMP_VE_FILE
				int w, h;
				decmp_dbg_parse_value(*cmd_pointer, &parsed_data);
				//g_osdshift_ctrl.h_shift_pixel = (int)parsed_data;
				w = (int)parsed_data;

			//rtd_pr_gdecmp_info("%s -- %d parsed_data=%d\n",__FUNCTION__,__LINE__,(int)parsed_data);

				rtd_pr_gdecmp_err("decmp to dump: %d\n", w);
				decmp_dbg_dump_file(VE_BUF_FILE_SIZE_2K);

				#endif//RTK_DUMP_VE_FILE
			
				
				return;
			}
		#ifdef ENABLE_GDECMP_RECORD
		case DECMP_DBG_CHECK_RECORD:
			{
				/*
					0: disable
					1: enable
					2: reset record

				*/
				int enable = 0;
				sscanf(*cmd_pointer, "%d\n", &enable);
				rtd_pr_gdecmp_err("[decmprec] enable:%d\n", enable);

				if( enable ==0 || enable == 1) {
					gEnableCheckRecord = enable;
				}
				else if (enable == 2) {
					decmp_rec_reset();
				}
				

			}
			break;
		#endif// ENABLE_GDECMP_RECORD
		case DECMP_DBG_ERROR_DUMP:
			{
				int enable = 0;
				sscanf(*cmd_pointer, "%d\n", &enable);
				rtd_pr_gdecmp_err("[decmprec] enable decmpErrDump:%d\n", enable);

				if( enable ==0 || enable == 1) {
					gEnableDecmpError_DUMP = enable;
				}
				
			}
		break;
		case DECMP_DBG_DCMT:
			{
				#if IS_ENABLED(CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER) && defined(DECOMP_DCMT_UNIT_TEST)
				int ret = -1;
				int type = 0;
				int val = 0;
				sscanf(*cmd_pointer, "%d %d\n", &type, &val );

				ret = decmp_dbg_dcmt(type, val);

				if( ret == 0 ) {
					rtd_pr_gdecmp_err("[decmpDBG]dcmt type:%d val:%d ok\n", type, val);
				}
				else 
					rtd_pr_gdecmp_err("[decmpDBG]dcmt type:%d val:%d fail %d\n", type, val, ret);

				#else
					rtd_pr_gdecmp_crit("decmp kernel doesn't enable CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER config\n");
				
				#endif//
			}	
			break;
		case DECMP_DBG_MEM_LIFE:
			{
				int decmp_cmd = 0;
				int ion_level= 0;
				sscanf(*cmd_pointer, "%d %d\n", &decmp_cmd, &ion_level );

				#if IS_ENABLED(CONFIG_ION_DVR_HEAP)
				ion_dvr_enable_debug(ion_level);

				#endif//

				#if IS_ENABLED(CONFIG_DMABUF_HEAPS_DVR)
					dvr_heap_enable_debug(ion_level);

				#endif//
			}
		

			break;
		case DECMP_DBG_STATUS:
			{
				GpuDec_DumpStatus(0, 0);

			}
			break;
		default:
			rtd_pr_gdecmp_err("decmp invalid dgb cmd:%d\n", cmd_index);

			return; 


	}
	
 	
}


const int DECMP_DBG_CMD_NUMBER = sizeof(decmp_cmd_str)/sizeof(char*);

	
loff_t GpuDec_llseek(struct file *filp, loff_t off, int whence)
{
	return -EINVAL;
}

ssize_t GpuDec_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	GpuDec_DumpStatus(0, 0 );
		
	return 0;
}

ssize_t GpuDec_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	
	char str[RTKDECMP_MAX_CMD_LENGTH] = {0};
	int cmd_index = 0, cmd_length = 0;
	char *cmd_pointer = NULL;

	if (buf == NULL) {
		return -EFAULT;
	}

	
	if (count > RTKDECMP_MAX_CMD_LENGTH - 1)
		count = RTKDECMP_MAX_CMD_LENGTH - 1;
	
	if (copy_from_user(str, buf, count)) {
		rtd_pr_gdecmp_err("decmp_debug:%d copy_from_user failed! (buf=%p, count=%d)\n",
			__LINE__, buf, (unsigned int)count);
		return -EFAULT;
	}

	if (count > 0)
		str[count-1] = '\0';

	rtd_pr_gdecmp_err("decmp cmd: %s\n", str);


	for (cmd_index = 0; cmd_index < DECMP_DBG_CMD_NUMBER; cmd_index++) {
		if (strncmp(str, decmp_cmd_str[cmd_index], strlen(decmp_cmd_str[cmd_index])) == 0)
			break;
	}
	
	

	if (cmd_index < DECMP_DBG_CMD_NUMBER ) 
	{
		//cmd_pointer = &str[strlen(rtkgdma_cmd_str[cmd_index])];
		cmd_length = strlen(decmp_cmd_str[cmd_index]);
		
		if (cmd_length <= RTKDECMP_MAX_CMD_LENGTH - 1)
			cmd_pointer = &str[cmd_length];
		else {
			rtd_pr_gdecmp_err("rtkgdma_debug:%d array index:%d invalid!\n", __LINE__, cmd_length);
			return -EFAULT;
		}

		if (cmd_pointer != NULL && ( cmd_pointer[0] != '\0') ) {
			decmp_dbg_EXECUTE(cmd_index, &cmd_pointer);
		}
			
	}





	return count;
}
int GpuDec_open(struct inode *inode, struct file *filp)
{
	return 0;
}

int GpuDec_release(struct inode *inode, struct file *filp)
{
	return 0;
}


int gdecmp_init_debug_proc(void)
{
	
	if ( rtkdecmp_proc_dir == NULL && rtkdecmp_proc_entry == NULL) {
		rtkdecmp_proc_dir = proc_mkdir(RTKDECMP_PROC_DIR , NULL);

		if (rtkdecmp_proc_dir != NULL) {
			rtkdecmp_proc_entry =
				proc_create(RTKDECMP_PROC_ENTRY, 0666,
							rtkdecmp_proc_dir, &gpuDecmp_fops);

			if (rtkdecmp_proc_entry == NULL) {
				proc_remove(rtkdecmp_proc_dir);
				rtkdecmp_proc_dir = NULL;

				return -1;
			}
		} else 	{
			rtd_pr_gdecmp_err("decmp create %s fail\n", RTKDECMP_PROC_DIR );

			return -2;
		}
	}

	return 0;
}

void gdecmp_uninit_debug_proc(void)
{
	if (rtkdecmp_proc_entry) {
		proc_remove(rtkdecmp_proc_entry);
		rtkdecmp_proc_entry = NULL;
	}

	if (rtkdecmp_proc_dir) {
		proc_remove(rtkdecmp_proc_dir);
		rtkdecmp_proc_dir = NULL;
	}
}

