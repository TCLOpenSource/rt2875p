
#include <gal/rtk_refdecmp.h>


#if 0
#ifdef CONFIG_ARM64
#define PRId64 "lld"
#define PRIx64 "llx"
#define PRIu64 "llu"
#define PRIo64 "llo"
#else

#define PRId64 "d"
#define PRIx64 "x"
#define PRIu64 "u"
#define PRIo64 "o"


#endif//

#endif//0

#define DECMP_REC_NUM_REGIONS (12)


#define DECMP_RECITEM_COUNT (10)


#define DECMP_TIMELIST_COUNT (256)


typedef struct _RTK_DECMP_RECORD_TIME
{

	//unsigned int set_time;
	long long last_bind_time;
	unsigned int unset_time;
	
} RTK_DECMP_RECORD_TIME;



typedef struct _RTK_DECMP_TIMEINFO
{

	rtk_decmp_export info;
	RTK_DECMP_RECORD_TIME recTime;
	
} RTK_DECMP_TIMEINFO;



typedef struct _RTK_DECMP_RECORD_ITEM
{

	int item_region_idx;
	unsigned long data_y_addr;
	int rec_wp;
	int rec_rp;
	//RTK_DECMP_REGION_RECORD rec[REGION_HISTORY_COUNT] ;
	rtk_decmp_export rec[DECMP_RECITEM_COUNT] ;
	RTK_DECMP_RECORD_TIME rec_time[DECMP_RECITEM_COUNT] ;
		
} RTK_DECMP_RECORD_ITEM ;


typedef struct _RTK_DECMP_RECORD_TABLE
{
	int max_region_idx; //soft constrain mode
	

	RTK_DECMP_RECORD_ITEM regions[DECMP_REC_NUM_REGIONS];

	rtk_decmp_export temp_rec[DECMP_REC_NUM_REGIONS];

	RTK_DECMP_TIMEINFO recInfo_list[DECMP_TIMELIST_COUNT];
	int recList_wp;
} RTK_DECMP_RECORD_TABLE; 


int decmp_rec_reset( void );
int decmp_rec_initRec(rtk_decmp_export* info);
int decmp_rec_resetRec(unsigned long addr, int region_idx);

RTK_DECMP_RECORD_ITEM* decmp_rec_find( unsigned long addr, int * ret_idx);
int decmp_rec_diff(int region_idx, unsigned long addr);
int decmp_rec_updateItem( RTK_DECMP_RECORD_ITEM* item, rtk_decmp_export* info, int idx);
void decmp_rec_dumpRecord( void );



//#define RTK_DUMP_VE_FILE
#define DECMP_DUMP_FILE_COUNTS (12)

void decmp_dump_file(const char* path, int size, unsigned long phy_addr);

void decmp_dbg_diff_register(void);
int decmp_dbg_dcmt(int type, int param_1);





void decmp_dump_regions( void );


int gdecmp_init_debug_proc(void);

void gdecmp_uninit_debug_proc(void);



