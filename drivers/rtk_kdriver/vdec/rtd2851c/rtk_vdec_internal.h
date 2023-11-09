#ifndef _RTK_VDEC_INTERNAL_H_
#define _RTK_VDEC_INTERNAL_H_

#include <rpc_common.h>
#include <linux/version.h>

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


#ifdef CONFIG_RTK_FEATURE_FOR_GKI

struct file *file_open(const char *path, int flags, int rights)
{
	struct file *filp = NULL;
	int err = 0;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	mm_segment_t oldfs;

	oldfs = get_fs();
	set_fs(KERNEL_DS);

	filp = filp_open(path, flags, rights);
	set_fs(oldfs);
#else
	filp = filp_open(path, flags, rights);
#endif

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
	int ret;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	mm_segment_t oldfs;

	oldfs = get_fs();
	set_fs(KERNEL_DS);

	ret = kernel_read(file, data, size, &offset);
	set_fs(oldfs);
#else
	ret = kernel_read(file, data, size, &offset);
#endif

	return ret;
}

int file_write(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size)
{
	int ret;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	mm_segment_t oldfs;

	oldfs = get_fs();
	set_fs(KERNEL_DS);
	ret = kernel_write(file, data, size, &offset);
	set_fs(oldfs);
#else
	ret = kernel_write(file, data, size, &offset);
#endif

	return ret;
}

int file_sync(struct file *file)
{
	vfs_fsync(file, 0);
	return 0;
}

int file_size(char *filename)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	int size;
	struct kstat *statbuf;
	mm_segment_t oldfs;

	oldfs = get_fs();
	set_fs(KERNEL_DS);

	statbuf = (struct kstat *)kmalloc(sizeof(struct kstat), GFP_KERNEL);
	vfs_stat(filename, statbuf);
	set_fs(oldfs);

	size = statbuf->size;
	kfree(statbuf);

	return size;
#else
	struct file *filePtr = 0 ;
	loff_t size;

	filePtr = file_open(filename, O_RDONLY, 0666) ;
	size = generic_file_llseek(filePtr, 0, SEEK_END);
	file_close(filePtr) ;

	return (int)size;
#endif
}

#else //#ifdef CONFIG_RTK_FEATURE_FOR_GKI

struct file *file_open(const char *path, int flags, int rights)
{
	return NULL;
}

void file_close(struct file *file)
{
}

int file_read(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size)
{
	return 0;
}

int file_write(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size)
{
	return 0;
}

int file_sync(struct file *file)
{
	return 0;
}

int file_size(char *filename)
{
	return 0;
}

#endif //#ifdef CONFIG_RTK_FEATURE_FOR_GKI

//void rtkvdec_user_alloc_query(RINGBUFFER_HEADER *, unsigned int, unsigned int, unsigned long) ;

void rtkvdec_user_alloc_query(RINGBUFFER_HEADER *ringBuf_H, unsigned int pid, unsigned int size, unsigned long phy_addr );

#endif
