#ifndef _RTK_VDEC_INTERNAL_H_
#define _RTK_VDEC_INTERNAL_H_

#include <rpc_common.h>

#ifdef CONFIG_ARM64
#define rtkvdec_memset(a,b,c) memset_io((void *)(a),(b),(c))
#define rtkvdec_memcpy(a,b,c) memcpy_toio((void *)(a),(b),(c))
#else
#define rtkvdec_memset(a,b,c) memset((void *)(a),(b),(c))
#define rtkvdec_memcpy(a,b,c) memcpy((void *)(a),(b),(c))
#endif

#define RTKVDEC_DUMMY_VBM_SIZE 0xB801A61C

#define RTKVDEC_MAX_VMM_MEM_NUM 3
#define S_OK    	0x10000000
#define S_FAIL  	0x10000000
#define FILE_NAME_SIZE 50

#if 0
typedef struct _tagRingBufferHeader {
	unsigned long magic;   /*Magic number*/
	unsigned long beginAddr;
	unsigned long size;
	unsigned long bufferID;  /* RINGBUFFER_TYPE, choose a type from RINGBUFFER_TYPE*/

	unsigned long  writePtr;
	unsigned long  numOfReadPtr;
	unsigned long  reserve2;  /*Reserve for Red Zone*/
	unsigned long  reserve3;  /*Reserve for Red Zone*/

	unsigned long readPtr[4];

	long          fileOffset ;
	long          requestedFileOffset ;
	long          fileSize ;

	long          bSeekable ;  /* Can't be sought if data is streamed by HTTP */

} RINGBUFFER_HEADER ;
#endif

typedef struct VDEC_DBG_STRUCT {
	unsigned char file_name[FILE_NAME_SIZE];    /* debug log file place & name */
	unsigned int  mem_size; 	/* debug memory size */
	unsigned char enable ;
} VDEC_DBG_STRUCT;

struct file *file_open(const char *path, int flags, int rights)
{
	struct file *filp = NULL;
	mm_segment_t oldfs;
	int err = 0;

	oldfs = get_fs();
	set_fs(KERNEL_DS);

	filp = filp_open(path, flags, rights);
	set_fs(oldfs);

	if (IS_ERR(filp)) {
		err = PTR_ERR(filp);
		return NULL;
	}

	return filp;
}

void file_close(struct file *file)
{
	filp_close(file, NULL);
}

int file_read(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size)
{
	mm_segment_t oldfs;
	int ret;

	oldfs = get_fs();
	set_fs(KERNEL_DS);

	ret = vfs_read(file, data, size, &offset);
	set_fs(oldfs);

	return ret;
}

int file_write(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size)
{
	mm_segment_t oldfs;
	int ret;

	oldfs = get_fs();
	set_fs(KERNEL_DS);
	ret = vfs_write(file, data, size, &offset);
	set_fs(oldfs);

	return ret;
}

int file_sync(struct file *file)
{
	vfs_fsync(file, 0);
	return 0;
}

//void rtkvdec_user_alloc_query(RINGBUFFER_HEADER *, unsigned int, unsigned int, unsigned long) ;

void rtkvdec_user_alloc_query(RINGBUFFER_HEADER *ringBuf_H, unsigned int pid, unsigned int size, unsigned long phy_addr );

#endif
