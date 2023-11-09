// SPDX-License-Identifier: GPL-2.0
/*
 * DMABUF DVR heap exporter
 *
 * Copyright (C) 2021 Realtek, Inc.
 *
 */

#define pr_fmt(fmt) "dmabuf_dvr: " fmt

#include <linux/version.h>
#include <linux/cma.h>
#include <linux/dma-buf.h>
#include <linux/dma-heap.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
#include <linux/dma-map-ops.h>
#else
#include <linux/dma-contiguous.h>
#endif
#include <linux/err.h>
#include <linux/highmem.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/scatterlist.h>
#include <linux/sched/signal.h>
#include <linux/slab.h>

#include <linux/pageremap.h>
//#include <uapi/linux/dma-heap.h>
#include <rtk_kdriver/rmm/rmm_heap.h>
#include <rtd_log/rtd_module_log.h>

// #define DVR_HEAP_LOG rtd_pr_rmm_err
#define DVR_HEAP_LOG rtd_pr_rmm_crit


int gDEBUG_DMA_HEAP = 0;


DEFINE_SPINLOCK(gDVR_HEAP_LOCK);
uint64_t gDVR_HEAP_STAMP = 0;


#if 0
	//use 4M as a unit log_2(4M) = 22
const int  COUNT_SHIFT = 22;

#define DVR_CHUNKS_NUM (3)

#elif 0
// 2MB 
const int  COUNT_SHIFT = 21;

#define DVR_CHUNKS_NUM (8 )


#else
  // 8M 
static const int  DMAH_COUNT_SHIFT = 23;

#define DVRDMA_CHUNKS_NUM (3)


#endif//

unsigned long  DVRDMA_CHUNK_UNIT_SIZE = (1u << DMAH_COUNT_SHIFT);



enum {
  DVR_HEAP_ERR_ALLOC_SGTABLE_FAIL = 100,


};

struct dvr_heap {
	struct dma_heap *heap;
};

struct dvr_heap_buffer {
	struct dvr_heap *heap;
	struct list_head attachments;
	struct mutex lock;
	unsigned long len;
	struct sg_table sg_table;
	struct page *dvr_pages;
	//struct page **pages;
	pgoff_t pagecount;
	int vmap_cnt;
	void *vaddr;

	unsigned long phys_addr;
	unsigned long flags;
	bool uncached;

	unsigned long ve_addr;
	unsigned long ve_addr_2;


	uint64_t stamp_id;	//unique

	//unsigned long buf_size;
	unsigned int chunk_count ; 
	struct page* chunk_pages[DVRDMA_CHUNKS_NUM];	  //first dvr_malloc still store in dvr_page
	unsigned long chunk_size[DVRDMA_CHUNKS_NUM];
	
};

struct dvr_heap_attachment {
	struct device *dev;
	struct sg_table *table;
	struct list_head list;
	bool mapped;

	bool uncached;
};


//test
static int gDVR_HEAP_DEBUG = 0;



#define DVR_HEAP_DEBUG_PRINT(level,fmt, arg...) do { \
       if( unlikely(gDVR_HEAP_DEBUG >= level) ) { rtd_pr_rmm_crit("TT:%lld "  fmt, ktime_to_us(ktime_get()), ##arg ); } \
   } while(0)



void dvr_heap_enable_debug(int loglevel)
{
	gDVR_HEAP_DEBUG = loglevel;
}

EXPORT_SYMBOL(dvr_heap_enable_debug);



//  v_addr, g_addr
typedef int (*RT_DVR_HEAP_FREE_CALLBACK)(unsigned long, uint64_t, void*);

static DEFINE_MUTEX(callback_mutex);
static RT_DVR_HEAP_FREE_CALLBACK gFreeNotifyDVR_HEAP = NULL;
int dvr_heap_free_register_callback(RT_DVR_HEAP_FREE_CALLBACK cb)
{
	mutex_lock(&callback_mutex);
	gFreeNotifyDVR_HEAP = cb;
	mutex_unlock(&callback_mutex);

	return 0;
}
EXPORT_SYMBOL(dvr_heap_free_register_callback);



int dvr_heap_free_ve_addr(void* in_buffer, unsigned long addr, unsigned long addr_2)
{

	struct dvr_heap_buffer *buffer = in_buffer;

	if( buffer == NULL)
	{
		if( addr != 0 ) {
			pli_free_mesg(addr);

			DVR_HEAP_DEBUG_PRINT(2, "dvrHFree 0 FM00 addr:0x%lx \n", addr );
		}

		if( addr_2 != 0 ) {
			pli_free_mesg(addr_2);

			DVR_HEAP_DEBUG_PRINT(2, "dvrHFree 1 FM00 addr:0x%lx \n", addr_2 );
		}
		
		return 0;
	}
	else
	{
		//double check consistent ?
		struct dvr_heap_buffer *info = buffer;


		if( info->ve_addr != addr ) {
			rtd_pr_rmm_crit( "dvrHFree err check addr fail\n", buffer, addr );
			return -5;

		}

		//pli_free_mesg(addr);
		if( info->ve_addr != 0) {
			pli_free_mesg(info->ve_addr);
			//DVR_HEAP_DEBUG_PRINT(3, " dvrHFree__ FM0 addr:0x%lx\n", info->ve_addr );
		}

		if( info->ve_addr_2 != 0 ) {
			pli_free_mesg(info->ve_addr_2);
			//DVR_HEAP_DEBUG_PRINT(3, " dvrHFree___ FM0 addr:0x%lx\n", info->ve_addr_2 );
		}

		DVR_HEAP_DEBUG_PRINT(2, "dvrHFree___ FM0 addr:0x%lx (0x%lx 0x%lx) %p %d  f:0x%x\n", info->phys_addr, info->ve_addr, info->ve_addr_2, buffer, 
			(uint32_t)buffer->len, buffer->flags );
		
		
		return 0;
	}
	

	return -1;
	
}
EXPORT_SYMBOL(dvr_heap_free_ve_addr);


unsigned long dvr_heap_alloc_ve_addr(int size)
{
	unsigned long phyaddr = 0;

	phyaddr = pli_malloc_mesg(size, GFP_DCU2_FIRST, "DMABUF_VE_R");
	
	if (phyaddr == INVALID_VAL)	{


		#if 0
			struct page *dvr_malloc_gpu_page(size_t size);

		#endif//

		rtd_pr_rmm_crit("alloc veAddr fail! %d\n", size);
		
		return 0;
	}

	DVR_HEAP_DEBUG_PRINT(2, "ionAlloc FMve addr:0x%lx 0x%x\n", phyaddr, size );
	
	return phyaddr;
}
EXPORT_SYMBOL( dvr_heap_alloc_ve_addr );


// note:  caller should validate consistent of 'buffer' pointer
int dvr_heap_set_ve_addr(void* in_buffer, unsigned long ve_addr, unsigned long ve_addr_2 )
{

	struct dvr_heap_buffer *buffer = in_buffer;
	
	#if 0
	if (buffer->heap->type != ION_HEAP_TYPE_CUSTOM) {
		rtd_pr_rmm_crit("%s() not sutable heap type(%d) fd(%d)\n", __func__, buffer->heap->type, fd);
		goto err;
	}

	if (buffer->sg_table == NULL) {
		rtd_pr_rmm_err("%s() no sg table,  \n", __func__, buffer);
		goto err;
	}
	#endif//

	if(buffer == NULL || buffer->phys_addr == 0 ) {
		rtd_pr_rmm_crit( "fail to set addr for p:%llx  0x%llx 0x%llx\n", buffer, ve_addr, ve_addr_2 );

		return -5;
	}


	buffer->ve_addr = ve_addr;
	buffer->ve_addr_2 = ve_addr_2;


	return 0;
	
}
EXPORT_SYMBOL( dvr_heap_set_ve_addr );



phys_addr_t dmaheap_to_phys(int fd);


static struct dma_heap *gDVR_CACHED = NULL;
static struct dma_heap *gDVR_FM = NULL;
static struct dma_heap *gDVR_GDECMP = NULL;
static struct dma_heap *gDVR_MULTICHUNK = NULL;




static int dvr_heap_get_sgtable(struct sg_table *sgt,
			       unsigned long phys_addr, size_t size, struct dvr_heap_buffer *info)
{
	int ret;

	int count = 0;
	struct page *page = phys_to_page(phys_addr);
		
	if( (info->flags & DMA_HEAP_FLAG_FOR_MULTI) == DMA_HEAP_FLAG_FOR_MULTI  ) 
	{
		count = 1 + info->chunk_count;


	}
	else
	{
		count = 1;

	}


	ret = sg_alloc_table(sgt, count, GFP_KERNEL);
	if (unlikely(ret)) {
		//note! need set this special flag to correct free sg_table 
		return DVR_HEAP_ERR_ALLOC_SGTABLE_FAIL;
	}


	if( (info->flags & DMA_HEAP_FLAG_FOR_MULTI) == DMA_HEAP_FLAG_FOR_MULTI  )
	{
		struct scatterlist *sg = sgt->sgl;
		int cc = 0;
	

		if( size <=  DVRDMA_CHUNK_UNIT_SIZE ) 
		{
			//not really multi ion
			sg_set_page( sg, page, (size), 0);

			DVR_HEAP_DEBUG_PRINT(7, "dvrMulti single entry 0x%lx! page_to_addr: 0x%lx %ld\n", phys_addr, page_to_phys(page), size );
		}
		else 
		{
			sg_set_page( sg, page, (DVRDMA_CHUNK_UNIT_SIZE), 0);
			
			DVR_HEAP_DEBUG_PRINT(7, "dvrMulti sgSet0  0x%lx! size:%ld \n",   page_to_phys(page), (DVRDMA_CHUNK_UNIT_SIZE) );

			sg =  sg_next(sg);
			
			for (cc= 0; cc < info->chunk_count; cc++) {

				page = info->chunk_pages[cc];

				if( page== NULL) {
					rtd_pr_rmm_crit( " dvr_get_sgtable fail %d %ld\n", cc, info->chunk_size[cc] );
					goto err_set_sgtable;
				}

				if( info->chunk_size[cc] != DVRDMA_CHUNK_UNIT_SIZE )
			 	{
					if( cc != info->chunk_count -1 )  {
						 //only last one can < DVRDMA_CHUNK_UNIT_SIZE
						 rtd_pr_rmm_emerg("invalid dvrMulti chunk algo %d %ld\n", cc,  info->chunk_size[cc] );
					}
			 	}
				
				//sg_set_page(sg, page, PAGE_ALIGN(info->chunk_size[cc]), 0);
				sg_set_page(sg, page, info->chunk_size[cc], 0);
				sg = sg_next(sg);

				DVR_HEAP_DEBUG_PRINT(7, "dvrMulti sgSet idx:%d 0x%lx! size:%ld \n", cc, page_to_phys(page), info->chunk_size[cc] );
			}


		}
		


				
	}
	else
	{
		//org
		sg_set_page(sgt->sgl, page, (size), 0);

		DVR_HEAP_DEBUG_PRINT(7, "dvr sgSet org 0x%lx! \n", page_to_phys(page) );
	}
	
	return 0;


	err_set_sgtable:
		

		return -1;



}


static struct dma_buf *dvr_heap_allocate(struct dma_heap *heap,
					 unsigned long len,
					 unsigned long fd_flags,
					 unsigned long heap_flags);

static struct sg_table *dup_sg_table(struct sg_table *table)
{
	struct sg_table *new_table;
	int ret, i;
	struct scatterlist *sg, *new_sg;

	new_table = kzalloc(sizeof(*new_table), GFP_KERNEL);
	if (!new_table)
		return ERR_PTR(-ENOMEM);

	ret = sg_alloc_table(new_table, table->orig_nents, GFP_KERNEL);
	if (ret) {
		kfree(new_table);
		return ERR_PTR(-ENOMEM);
	}

	new_sg = new_table->sgl;
	for_each_sgtable_sg(table, sg, i) {
		sg_set_page(new_sg, sg_page(sg), sg->length, sg->offset);
		new_sg = sg_next(new_sg);
	}

	return new_table;
}

static int dvr_heap_attach(struct dma_buf *dmabuf,
			   struct dma_buf_attachment *attachment)
{
	struct dvr_heap_buffer *buffer = dmabuf->priv;
	struct dvr_heap_attachment *a;
	struct sg_table *table;

	a = kzalloc(sizeof(*a), GFP_KERNEL);
	if (!a)
		return -ENOMEM;

	table = dup_sg_table(&buffer->sg_table);
	if (IS_ERR(table)) {
		kfree(a);
		return -ENOMEM;
	}

	a->table = table;
	a->dev = attachment->dev;
	INIT_LIST_HEAD(&a->list);
	a->mapped = false;
	a->uncached = buffer->uncached;

	attachment->priv = a;

	mutex_lock(&buffer->lock);
	list_add(&a->list, &buffer->attachments);
	mutex_unlock(&buffer->lock);

	return 0;
}

static void dvr_heap_detach(struct dma_buf *dmabuf,
			    struct dma_buf_attachment *attachment)
{
	struct dvr_heap_buffer *buffer = dmabuf->priv;
	struct dvr_heap_attachment *a = attachment->priv;

	mutex_lock(&buffer->lock);
	list_del(&a->list);
	mutex_unlock(&buffer->lock);

	sg_free_table(a->table);
	kfree(a->table);
	kfree(a);
}

static struct sg_table *dvr_heap_map_dma_buf(struct dma_buf_attachment *attachment,
					     enum dma_data_direction direction)
{
	struct dvr_heap_attachment *a = attachment->priv;
	struct sg_table *table = a->table;
	int attr = 0;
	int ret;

	if (a->uncached)
		attr = DMA_ATTR_SKIP_CPU_SYNC;

	ret = dma_map_sgtable(attachment->dev, table, direction, attr);
	if (ret)
		return ERR_PTR(-ENOMEM);

	a->mapped = true;
	return table;
}

static void dvr_heap_unmap_dma_buf(struct dma_buf_attachment *attachment,
				   struct sg_table *table,
				   enum dma_data_direction direction)
{
	struct dvr_heap_attachment *a = attachment->priv;
	int attr = 0;

	if (a->uncached)
		attr = DMA_ATTR_SKIP_CPU_SYNC;

	a->mapped = false;
	dma_unmap_sgtable(attachment->dev, table, direction, attr);
}

static int dvr_heap_dma_buf_begin_cpu_access(struct dma_buf *dmabuf,
					     enum dma_data_direction direction)
{
	struct dvr_heap_buffer *buffer = dmabuf->priv;
	struct dvr_heap_attachment *a;

	mutex_lock(&buffer->lock);

	if (buffer->vmap_cnt)
		invalidate_kernel_vmap_range(buffer->vaddr, buffer->len);

	if (!buffer->uncached) {
		list_for_each_entry(a, &buffer->attachments, list) {
			if (!a->mapped)
				continue;
			dma_sync_sgtable_for_cpu(a->dev, a->table, direction);
		}
	}
	mutex_unlock(&buffer->lock);

	return 0;
}

static int dvr_heap_dma_buf_end_cpu_access(struct dma_buf *dmabuf,
					   enum dma_data_direction direction)
{
	struct dvr_heap_buffer *buffer = dmabuf->priv;
	struct dvr_heap_attachment *a;

	mutex_lock(&buffer->lock);

	if (buffer->vmap_cnt)
		flush_kernel_vmap_range(buffer->vaddr, buffer->len);

	if (!buffer->uncached) {
		list_for_each_entry(a, &buffer->attachments, list) {
			if (!a->mapped)
				continue;
			dma_sync_sgtable_for_device(a->dev, a->table, direction);
		}
	}
	mutex_unlock(&buffer->lock);

	return 0;
}

static int dvr_heap_mmap(struct dma_buf *dmabuf, struct vm_area_struct *vma)
{
	struct dvr_heap_buffer *buffer = dmabuf->priv;
	struct sg_table *table = &buffer->sg_table;
	unsigned long addr = vma->vm_start;
	struct sg_page_iter piter;
	int ret = -ENXIO;

	if ((vma->vm_flags & (VM_SHARED | VM_MAYSHARE)) == 0) {
		DVR_HEAP_LOG("dmaHeap dvr map fail :0x%x\n", vma->vm_flags );
		return -EINVAL;
	}

	if (buffer->uncached) {
		vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
	}

	rtd_pr_rmm_debug("Map %lx to %lx size %lx uncached(%d)\n", 
					 ((buffer->phys_addr >> PAGE_SHIFT) + vma->vm_pgoff) << PAGE_SHIFT, 
					 vma->vm_start, 
					 ((vma->vm_end - vma->vm_start) >> PAGE_SHIFT) << PAGE_SHIFT, buffer->uncached);

	for_each_sgtable_page(table, &piter, vma->vm_pgoff) {
		struct page *page = sg_page_iter_page(&piter);

		ret = remap_pfn_range(vma, addr, page_to_pfn(page), PAGE_SIZE,
				      vma->vm_page_prot);
		if (ret) {
			goto remap_out;
		}
		addr += PAGE_SIZE;
		if (addr >= vma->vm_end) {
			ret = 0;
			goto remap_out;
		}
	}

remap_out:
	if( ret != 0 ) 
		DVR_HEAP_LOG(" fail %s %d %d\n", __func__, __LINE__, ret);
		
	return ret;
}

static void *dvr_heap_do_vmap(struct dvr_heap_buffer *buffer)
{
	struct sg_table *table = &buffer->sg_table;
	int npages = PAGE_ALIGN(buffer->len) / PAGE_SIZE;
	struct page **pages = vmalloc(sizeof(struct page *) * npages);
	struct page **tmp = pages;
	struct sg_page_iter piter;
	pgprot_t pgprot = PAGE_KERNEL;
	void *vaddr;

	if (!pages)
		return ERR_PTR(-ENOMEM);

	if (buffer->uncached)
		pgprot = pgprot_writecombine(PAGE_KERNEL);

	for_each_sgtable_page(table, &piter, 0) {
		WARN_ON(tmp - pages >= npages);
		*tmp++ = sg_page_iter_page(&piter);
	}

	vaddr = vmap(pages, npages, VM_MAP, pgprot);
	vfree(pages);

	if (!vaddr) {
		DVR_HEAP_LOG(" fail %s %d %p\n", __func__, __LINE__, vaddr);
		return ERR_PTR(-ENOMEM);
	}

	return vaddr;
}


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 11, 0))

static int dvr_heap_vmap(struct dma_buf *dmabuf, struct dma_buf_map *map)
{
	struct dvr_heap_buffer *buffer = dmabuf->priv;
	void *vaddr;
	int ret = 0;

	mutex_lock(&buffer->lock);
	if (buffer->vmap_cnt) {
		buffer->vmap_cnt++;
		dma_buf_map_set_vaddr(map, buffer->vaddr);
		goto out;
	}

	vaddr = dvr_heap_do_vmap(buffer);
	if (IS_ERR(vaddr)) {
		ret = PTR_ERR(vaddr);
		goto out;
	}

	buffer->vaddr = vaddr;
	buffer->vmap_cnt++;
	dma_buf_map_set_vaddr(map, buffer->vaddr);
out:
	mutex_unlock(&buffer->lock);

	return ret;
}

static void dvr_heap_vunmap(struct dma_buf *dmabuf, struct dma_buf_map *map)
{
	struct dvr_heap_buffer *buffer = dmabuf->priv;

	mutex_lock(&buffer->lock);
	if (!--buffer->vmap_cnt) {
		vunmap(buffer->vaddr);
		buffer->vaddr = NULL;
	}
	mutex_unlock(&buffer->lock);
	dma_buf_map_clear(map);
}

#else

static void *dvr_heap_vmap(struct dma_buf *dmabuf)
{
	struct dvr_heap_buffer *buffer = dmabuf->priv;
	void *vaddr;

	mutex_lock(&buffer->lock);
	if (buffer->vmap_cnt) {
		buffer->vmap_cnt++;
		vaddr = buffer->vaddr;
		goto out;
	}

	vaddr = dvr_heap_do_vmap(buffer);
	if (IS_ERR(vaddr))
		goto out;

	buffer->vaddr = vaddr;
	buffer->vmap_cnt++;
out:
	mutex_unlock(&buffer->lock);

	return vaddr;
}

static void dvr_heap_vunmap(struct dma_buf *dmabuf, void *vaddr)
{
	struct dvr_heap_buffer *buffer = dmabuf->priv;

	mutex_lock(&buffer->lock);
	if (!--buffer->vmap_cnt) {
		vunmap(buffer->vaddr);
		buffer->vaddr = NULL;
	}
	mutex_unlock(&buffer->lock);
}

#endif

static void dvr_heap_dma_buf_release(struct dma_buf *dmabuf)
{
	struct dvr_heap_buffer *buffer = dmabuf->priv;
	struct sg_table *table;

	unsigned long ve_addr = buffer->ve_addr ;
	unsigned long ve_addr_2 = buffer->ve_addr_2 ;

	rtd_pr_rmm_debug("Release buffer %lx, phy(0x%lx)\n", (unsigned long)buffer, (unsigned long)buffer->phys_addr);

	if (buffer->vmap_cnt > 0) {
		WARN(1, "%s: buffer still mapped in the kernel\n", __func__);
		vunmap(buffer->vaddr);
	}

	if(gDEBUG_DMA_HEAP ) {
			rtd_pr_rmm_crit("dmaHeap dvr free 0x%lx %ld uncached(%d) \n", buffer->phys_addr, buffer->len, buffer->uncached);
	}


	{
		int flags = buffer->flags;
		unsigned long phy_addr = buffer->phys_addr;
		
	
		if (gFreeNotifyDVR_HEAP != NULL && ( (flags & DMA_HEAP_FLAG_FOR_VE) ||  (flags & DMA_HEAP_FLAG_FOR_FM)  )
			)
		{
			
			gFreeNotifyDVR_HEAP(ve_addr, phy_addr, NULL);
		}
			
	}



	if ( buffer->flags & DMA_HEAP_FLAG_FOR_FM ) {

		#ifdef CONFIG_FAKEMEMORY_GRAPHICS
		
			dvr_free_gpu_page(  buffer->dvr_pages );

			//if( ve_addr != 0 ) 
			{
				dvr_heap_free_ve_addr( buffer,  ve_addr, ve_addr_2 );
			}
			
			DVR_HEAP_DEBUG_PRINT(2, "dvrHFree FM0 addr:0x%lx (0x%lx 0x%lx) %p %d  f:0x%x\n", buffer->phys_addr, ve_addr, ve_addr_2, buffer, (uint32_t)buffer->len, buffer->flags );	
			
		#else
			dvr_free_page_mesg( buffer->dvr_pages );
		
			DVR_HEAP_DEBUG_PRINT(2, "dvrHFree FM1 addr:0x%lx (0x%lx 0x%lx) %p %d  f:0x%x\n", buffer->phys_addr, ve_addr, ve_addr_2, buffer, (uint32_t)buffer->len, buffer->flags );
			
		#endif//

		
		
	}
	else {
		dvr_free_page_mesg(buffer->dvr_pages);

		DVR_HEAP_DEBUG_PRINT(2, "dvrHFree norm addr:0x%lx (0x%lx) %p %d  f:0x%x\n", buffer->phys_addr, ve_addr, buffer, (uint32_t)buffer->len, buffer->flags );
	}


	if ( buffer->chunk_pages[0] != NULL )
	{
		int idd = 0;
		
		if(  (buffer->flags & DMA_HEAP_FLAG_FOR_MULTI) == 0  ) 
		{
			rtd_pr_rmm_alert("ERROR invalid ion_multi when free !  %p %ld 0x%x\n", buffer->phys_addr, buffer->len, buffer->flags );
		}


		for( idd=0; idd < DVRDMA_CHUNKS_NUM; ++idd )
		{
		
			if( buffer->chunk_pages[idd]  != NULL ) {
				
				if( buffer->chunk_size[idd] <= 0 ) {
					rtd_pr_rmm_crit("ERROR invalid ion_multi size. idx:%d when free %p %ld 0x%x %ld\n", idd, buffer->phys_addr, buffer->len, buffer->flags, buffer->chunk_size[idd] );
				}

				dvr_free_page_mesg( buffer->chunk_pages[idd] );
				
				DVR_HEAP_DEBUG_PRINT(2, "kkk2 dvrMulti free idx:%d ok  phy:0x%lx, size:%lu\n", idd, page_to_phys(buffer->chunk_pages[idd]), buffer->chunk_size[idd]);
			}


		}
	}


	/* free page list */

	table = &buffer->sg_table;
	sg_free_table(table);		
	kfree(buffer);
}

const struct dma_buf_ops dvr_heap_buf_ops = {
	.attach = dvr_heap_attach,
	.detach = dvr_heap_detach,
	.map_dma_buf = dvr_heap_map_dma_buf,
	.unmap_dma_buf = dvr_heap_unmap_dma_buf,
	.begin_cpu_access = dvr_heap_dma_buf_begin_cpu_access,
	.end_cpu_access = dvr_heap_dma_buf_end_cpu_access,
	.mmap = dvr_heap_mmap,
	.vmap = dvr_heap_vmap,
	.vunmap = dvr_heap_vunmap,
	.release = dvr_heap_dma_buf_release,
};

EXPORT_SYMBOL( dvr_heap_buf_ops );


unsigned long dvr_heap_to_phys(struct dma_buf *dmabuf)
{
	struct dvr_heap_buffer *buffer;
	unsigned long ret = INVALID_VAL;

	if (IS_ERR(dmabuf)) {
		rtd_pr_rmm_err("%s invalid dmabuf(%lx)\n", __func__, (unsigned long)dmabuf);
		return INVALID_VAL;
	}

	if ((buffer = dmabuf->priv) == NULL) {
		rtd_pr_rmm_err("%s() no buffer, dmabuf(%lx)\n", __func__, (unsigned long)dmabuf);
		goto err_out;
	}

	if( dmabuf->ops != &dvr_heap_buf_ops ) {
		rtd_pr_rmm_crit("%s invalid dvr dmabuf(%lx)\n", __func__, (unsigned long)dmabuf);
	}

	ret = (unsigned long)buffer->phys_addr;

	if (ret == 0)
		return INVALID_VAL;
	else
		return ret;

err_out:
	return INVALID_VAL;
}

uint64_t dmaheap_get_stamp(int fd)
{
	struct dma_buf *dmabuf = NULL;
	struct dvr_heap_buffer *buffer;
	uint64_t ret = 0;
	
	dmabuf = dma_buf_get(fd);
	if (IS_ERR(dmabuf)) {
		rtd_pr_rmm_err("%s() invalid dmabuf, fd(%d)\n", __func__, fd);
		goto err_out;
	}

	if( dmabuf->ops != &dvr_heap_buf_ops ) {
		rtd_pr_rmm_crit("%s invalid dvr dmabuf(%lx)\n", __func__, (unsigned long)dmabuf);
		goto err_out;
	}

	
	if ((buffer = dmabuf->priv) == NULL) {
		rtd_pr_rmm_err("%s() no buffer, dmabuf(%lx)\n", __func__, (unsigned long)dmabuf);
		goto err_out;
	}


	ret = buffer->stamp_id;
	
	

  err_out:
	if (!IS_ERR(dmabuf))
		dma_buf_put(dmabuf);
	
	return ret;

}
EXPORT_SYMBOL( dmaheap_get_stamp );


phys_addr_t dmaheap_to_phys(int fd)
{
	struct dma_buf *dmabuf = NULL;
	//struct dvr_heap_buffer *buffer = NULL;
	unsigned long addr =  INVALID_VAL;
	// phys_addr_t ret = INVALID_VAL;

	dmabuf = dma_buf_get(fd);
	if (IS_ERR(dmabuf)) {
		rtd_pr_rmm_err("%s() fail, fd(%d)\n", __func__, fd);
		goto err;
	}

	addr = dvr_heap_to_phys( dmabuf );
	

  err:
	if (!IS_ERR(dmabuf))
		dma_buf_put(dmabuf);
	
	return (phys_addr_t)addr;

}
EXPORT_SYMBOL( dmaheap_to_phys );



//#define ION_FLAG_FOR_VE (0x80)  //bit 7
static struct dma_buf *dvr_heap_allocate_gdecmp(struct dma_heap *heap,
					 unsigned long len,
					 unsigned long fd_flags,
					 unsigned long heap_flags)
{

	struct dma_buf *dmabuf = NULL;

		
		dmabuf = dvr_heap_allocate(heap, len, fd_flags, heap_flags | DMA_HEAP_FLAG_FOR_VE );


	return dmabuf;
}


// #define ION_FLAG_FOR_FM (0x40)  //bit 6
static struct dma_buf *dvr_heap_allocate_fm(struct dma_heap *heap,
					 unsigned long len,
					 unsigned long fd_flags,
					 unsigned long heap_flags)
{

	struct dma_buf *dmabuf=NULL;
	dmabuf = dvr_heap_allocate(heap, len, fd_flags, heap_flags | DMA_HEAP_FLAG_FOR_FM );
	
	return dmabuf;
}

static struct dma_buf *dvr_heap_allocate_multi(struct dma_heap *heap,
					 unsigned long len,
					 unsigned long fd_flags,
					 unsigned long heap_flags)
{

	struct dma_buf *dmabuf=NULL;
	dmabuf = dvr_heap_allocate(heap, len, fd_flags, heap_flags | DMA_HEAP_FLAG_FOR_MULTI );
	
	return dmabuf;
}


static struct dma_buf *dvr_heap_allocate_cached(struct dma_heap *heap,
					 unsigned long len,
					 unsigned long fd_flags,
					 unsigned long heap_flags)
{
	
	struct dma_buf *dmabuf=NULL;
	
	dmabuf = dvr_heap_allocate(heap, len, fd_flags, heap_flags | DMA_HEAP_FLAG_CACHED );


	return dmabuf;	
}


static struct dma_buf *dvr_heap_allocate_uncached(struct dma_heap *heap,
					 unsigned long len,
					 unsigned long fd_flags,
					 unsigned long heap_flags)
{
	struct dma_buf *dmabuf=NULL;

	
	dmabuf = dvr_heap_allocate(heap, len, fd_flags, 0 );


	return dmabuf;
}



static struct dma_buf *dvr_heap_allocate(struct dma_heap *heap,
					 unsigned long len,
					 unsigned long fd_flags,
					 unsigned long heap_flags)
{
	struct dvr_heap *dvr_heap = dma_heap_get_drvdata(heap);
	struct dvr_heap_buffer *buffer;
	DEFINE_DMA_BUF_EXPORT_INFO(exp_info);
	size_t size = PAGE_ALIGN(len);
	const size_t size_input = len;
		
	pgoff_t pagecount = size >> PAGE_SHIFT;
	struct page *dvr_pages= NULL ;
	struct dma_buf *dmabuf=NULL;
	struct sg_table *table= NULL;
	int alloc_sg_table = 0;
	int ret = -ENOMEM;

	unsigned long flags;
//	pgoff_t pg;

	struct scatterlist *sg; 	int i;

	#define FLAG_STR_SIZE (32)
	char flags_str[FLAG_STR_SIZE];




	
	if (heap_flags & DMA_HEAP_FLAG_FOR_VE ) {
		snprintf(flags_str, FLAG_STR_SIZE, "DMABUF_VE:%lx_%ld", heap_flags, gDVR_HEAP_STAMP);
	}
	else if ( heap_flags & DMA_HEAP_FLAG_FOR_FM ) {
		snprintf(flags_str, FLAG_STR_SIZE, "DMABUF_FM:%lx_%ld", heap_flags, gDVR_HEAP_STAMP);
	}
	else if ( heap_flags & DMA_HEAP_FLAG_FOR_MULTI )
		snprintf(flags_str, FLAG_STR_SIZE, "GraMul:%lx_%ld", heap_flags, gDVR_HEAP_STAMP);
	else {
		snprintf(flags_str, FLAG_STR_SIZE, "DMABUF_%lx_%ld", heap_flags, gDVR_HEAP_STAMP);
	}

	if(gDEBUG_DMA_HEAP ) {
	
		rtd_pr_rmm_crit("dmaHeap dvr size:%d %x flag:0x%lx\n", len, fd_flags, heap_flags);
	}


	buffer = kzalloc(sizeof(*buffer), GFP_KERNEL);
	if (!buffer) {
		rtd_pr_rmm_err("Can't allocate buffer info\n");
		return ERR_PTR(-ENOMEM);
	}

	INIT_LIST_HEAD(&buffer->attachments);
	mutex_init(&buffer->lock);
	buffer->len = size;
	buffer->flags = heap_flags;
	buffer->uncached = (buffer->flags & DMA_HEAP_FLAG_CACHED) ? false : true;



	if (buffer->flags & DMA_HEAP_FLAG_FROM_DCU1) {
		dvr_pages = dvr_malloc_page_mesg(size, GFP_DCU1, "DMABUF:01:");
	} else if (buffer->flags & DMA_HEAP_FLAG_FROM_DCU2) {
		dvr_pages = dvr_malloc_page_mesg(size, GFP_DCU2, "DMABUF:02:");
	} 
	else if (buffer->flags & DMA_HEAP_FLAG_FOR_FM )
	{
		#ifdef CONFIG_FAKEMEMORY_GRAPHICS
			dvr_pages = dvr_malloc_gpu_page(len);

			//rtd_pr_rmm_crit("KKK ion FM addr: 0x%llx  size:%ld\n", page_to_phys(info->dvr_page), len );

			DVR_HEAP_DEBUG_PRINT(2, "dvrHAlloc FMmem0 %p %d phy(0x%lx)\n", buffer, (uint32_t)size_input, page_to_phys(dvr_pages) );

		#else
			dvr_pages = dvr_malloc_page_mesg(len, GFP_DCU2_FIRST, flags_str);

			DVR_HEAP_DEBUG_PRINT(2, "dvrHAlloc FMmem1 %p %d phy(0x%lx)\n", buffer, (uint32_t)size_input, page_to_phys(dvr_pages) );

		#endif//CONFIG_FAKEMEMORY_GRAPHICS
	}
	else if ( (buffer->flags & DMA_HEAP_FLAG_FOR_MULTI)  )
	{
		unsigned long cur_length = size;  //note: size has page aligned
		
		int ii = 0;
		unsigned int unit_size = DVRDMA_CHUNK_UNIT_SIZE;
		
		unsigned long count_unit = (cur_length + DVRDMA_CHUNK_UNIT_SIZE -1)  >> DMAH_COUNT_SHIFT;

		 // reset info->flags  ION_FLAG_FOR_MULTI ?

		if( cur_length <= DVRDMA_CHUNK_UNIT_SIZE ) {
			unit_size = cur_length;
		}
		
		dvr_pages = dvr_malloc_page_mesg(unit_size, GFP_DCU2_FIRST, flags_str);

		if( dvr_pages == NULL ) {
			rtd_pr_rmm_err("Can't allocate buffer for ION MULTI:%lu %lu\n", unit_size, cur_length);
			goto free_dvr;
		}

		DVR_HEAP_DEBUG_PRINT(3, "dvrMulti alloc org addr:0x%lx (0x%lx) %p %d  f:0x%x\n", page_to_phys( dvr_pages ),  buffer, (uint32_t)unit_size, buffer->flags );

		//--count; // first count is original dvr
		cur_length -= unit_size;

		if( (count_unit -1) > DVRDMA_CHUNKS_NUM ) {
			rtd_pr_rmm_crit(" dvrMulti can't allocate > %d  chunks  inSize:%lu\n", count_unit,  size_input);
			goto free_dvr;
		}

		for( ii=0; ii < count_unit -1 ; ++ii)
		{

			unit_size = DVRDMA_CHUNK_UNIT_SIZE;
			
			if( cur_length <= DVRDMA_CHUNK_UNIT_SIZE ) {
				unit_size = cur_length;
			}
			
			{
				// FLAG_STR_SIZE (32)
				//flags_str[10] = ii;
				buffer->chunk_pages[ii] = dvr_malloc_page_mesg(unit_size, GFP_DCU2_FIRST, flags_str);

				buffer->chunk_size[ii] = unit_size;
			}
			

			if( buffer->chunk_pages[ii] == NULL) {
				
				rtd_pr_rmm_crit("Can't allocate buffer for ION MULTI:%lu idx:%d %ld %ld \n", buffer->chunk_size[ii], ii, cur_length, unit_size);
				buffer->chunk_size[ii] = 0;
				
				goto free_dvr;
			}


			cur_length -= buffer->chunk_size[ii];

			
			DVR_HEAP_DEBUG_PRINT(5, "ionAlloc Multi idx:%d ok  phy:0x%lx, size:%lu rem:%ld total:%ld\n", ii, page_to_phys( buffer->chunk_pages[ii]), buffer->chunk_size[ii], 
				cur_length, len );
			
		}

		if( count_unit > 0)
			buffer->chunk_count = count_unit - 1; //skip original info->dvr_page
		
	}

	else {
		dvr_pages = dvr_malloc_page_mesg(size, GFP_DCU2_FIRST, flags_str);
	}
	
	if (!dvr_pages) {
		rtd_pr_rmm_err("Fail to allocate buffer\n");
		goto free_buffer;
	}


	buffer->dvr_pages = dvr_pages;
	buffer->heap = dvr_heap;
	buffer->pagecount = pagecount;
	buffer->phys_addr = page_to_phys(dvr_pages);


	table = &buffer->sg_table;  //note! this is always  non-NULL
	#if 0	
	if (sg_alloc_table(table, 1, GFP_KERNEL)) {
		rtd_pr_rmm_err("sg_alloc_table fail\n");
		ret = -ENOMEM;
		goto free_pages;
	}
	
	sg_set_page(table->sgl, pfn_to_page(PFN_DOWN(buffer->phys_addr)), buffer->len, 0);
	#else
		ret = dvr_heap_get_sgtable(table, buffer->phys_addr, buffer->len, buffer) ;
	
		if( ret != DVR_HEAP_ERR_ALLOC_SGTABLE_FAIL )
			alloc_sg_table = 1;
		
		if( ret )
			goto free_pages;

	#endif//

  for_each_sg( table->sgl, sg, table->nents, i) 
  {
	  struct page *page = sg_page(sg);
	  unsigned long sg_len = sg->length;
		
	if ( !(buffer->flags & DMA_HEAP_FLAG_FOR_FM ) )
	{
		if (PageHighMem( page )) {
			unsigned long nr_clear_pages = PAGE_ALIGN(sg_len) >> PAGE_SHIFT;
			

			while (nr_clear_pages > 0) {
				void *vaddr = kmap_atomic(page);

				memset(vaddr, 0, PAGE_SIZE);
				kunmap_atomic(vaddr);
				/*
				 * Avoid wasting time zeroing memory if the process
				 * has been killed by by SIGKILL
				 */
				if (fatal_signal_pending(current))
					goto free_pages;
				page++;
				nr_clear_pages--;
			}
		} else {
			//memset( page_address(page), 0, PAGE_ALIGN(sg_len) );
			memset( page_address(page), 0, sg_len );

		}

		//dma_sync_single_for_cpu( dma_heap_get_dev(heap), page_to_phys(dvr_pages), PAGE_ALIGN(len), DMA_BIDIRECTIONAL);
  		  dma_sync_single_for_cpu( dma_heap_get_dev(heap), page_to_phys(page), sg_len, DMA_BIDIRECTIONAL);
			
	}

	DVR_HEAP_DEBUG_PRINT(8, "dvrAlloc clear idx:%d phy:0x%x, size:%lu\n", i, page_to_phys( page ), sg_len);
	
  }
	

	/* create the dmabuf */
	#if ( LINUX_VERSION_CODE > KERNEL_VERSION(5, 15, 0))
	exp_info.exp_name = dma_heap_get_name(heap);
	#endif
	
	exp_info.ops = &dvr_heap_buf_ops;
	exp_info.size = buffer->len;
	exp_info.flags = fd_flags;
	exp_info.priv = buffer;
	dmabuf = dma_buf_export(&exp_info);
	if (IS_ERR(dmabuf)) {
		ret = PTR_ERR(dmabuf);
		goto free_pages;
	}

	spin_lock_irqsave(&gDVR_HEAP_LOCK, flags);
	buffer->stamp_id = gDVR_HEAP_STAMP;
	gDVR_HEAP_STAMP++;
	spin_unlock_irqrestore(&gDVR_HEAP_LOCK, flags);


	DVR_HEAP_DEBUG_PRINT(2, "dvr heap alloc 0x%lx %ld uncached(%d) %lx %ld \n", buffer->phys_addr, buffer->len, buffer->uncached, heap_flags, buffer->stamp_id);

	if(gDEBUG_DMA_HEAP ) {
	
		rtd_pr_rmm_crit("dmaHeap dvr alloc ok 0x%lx %ld uncached(%d) \n", buffer->phys_addr, buffer->len, buffer->uncached);
	}
	return dmabuf;

free_pages:



	if( alloc_sg_table ) {
		sg_free_table(table);
	}
	

free_dvr:
	
	
		
	if(dvr_pages != NULL) {
		
		if (buffer->flags & DMA_HEAP_FLAG_FOR_FM ) {

			#ifdef CONFIG_FAKEMEMORY_GRAPHICS
				
					dvr_free_gpu_page( dvr_pages );
			#else

					dvr_free_page_mesg( dvr_pages );
			#endif//

		}
		else {	
		dvr_free_page_mesg(dvr_pages);
		}
	}

	if ( buffer->chunk_pages[0] != NULL )
	{
		int idd = 0;
		
		if(  (buffer->flags & DMA_HEAP_FLAG_FOR_MULTI) == 0  ) 
		{
			rtd_pr_rmm_crit("invalid ion_multi case!  %p %ld 0x%x\n", buffer->phys_addr, buffer->len, buffer->flags );
		}


		for( idd=0; idd < DVRDMA_CHUNKS_NUM; ++idd )
		{
		
			if( buffer->chunk_pages[idd]  != NULL ) {
				
				if( buffer->chunk_size[idd] <= 0 ) {
					rtd_pr_rmm_crit("invalid ion_multi error %p %ld 0x%x %ld\n", buffer->phys_addr, buffer->len, buffer->flags, buffer->chunk_size[idd] );
				}

				dvr_free_page_mesg( buffer->chunk_pages[idd] );
				
			}


		}
	}

free_buffer:
	kfree(buffer);

	//if(gDEBUG_DMA_HEAP ) 
	{
		rtd_pr_rmm_crit("dmaHeap dvr fail:0x%d %d %d\n", ret, len, heap_flags);
	}

	return ERR_PTR(ret);
}

static inline // temporarily supress unused warning
struct dma_buf *dvr_heap_not_initialized(struct dma_heap *heap,
							    unsigned long len,
							    unsigned long fd_flags,
							    unsigned long heap_flags)
{
	return ERR_PTR(-EBUSY);
}


static const struct dma_heap_ops dvr_heap_ops = {
	.allocate = dvr_heap_allocate_uncached,
};


static struct dma_heap_ops dvr_heap_ops_cached = {
	/* After system_heap_create is complete, we will swap this */
	.allocate = dvr_heap_allocate_cached,
};

static struct dma_heap_ops dvr_heap_ops_gdecmp = {
	/* After system_heap_create is complete, we will swap this */
	.allocate = dvr_heap_allocate_gdecmp,
};


static struct dma_heap_ops dvr_heap_ops_fm = {
	/* After system_heap_create is complete, we will swap this */
	.allocate = dvr_heap_allocate_fm,
};

static struct dma_heap_ops dvr_heap_ops_multi = {
	/* After system_heap_create is complete, we will swap this */
	.allocate = dvr_heap_allocate_multi,
};



static int dvr_heap_create(void)
{
	struct dvr_heap *dvr_heap;
	struct dma_heap_export_info exp_info;

	dvr_heap = kzalloc(sizeof(*dvr_heap), GFP_KERNEL);
	if (!dvr_heap)
		return -ENOMEM;

	exp_info.name = "dvr";   // uncached
	exp_info.ops = &dvr_heap_ops;
	exp_info.priv = dvr_heap;

	dvr_heap->heap = dma_heap_add(&exp_info);
	if (IS_ERR(dvr_heap->heap)) {
		int ret = PTR_ERR(dvr_heap->heap);

		kfree(dvr_heap);
		return ret;
	}


	exp_info.name = "dvr-cached";
	exp_info.ops = &dvr_heap_ops_cached;
	exp_info.priv = NULL   ;
	
	gDVR_CACHED = dma_heap_add(&exp_info);
	if (IS_ERR(gDVR_CACHED))
		return PTR_ERR(gDVR_CACHED);
	//dvr_heap_ops_cached.allocate = dvr_heap_allocate_cached;
		
	exp_info.name = "dvr-fm";
	exp_info.ops = &dvr_heap_ops_fm;
	exp_info.priv = NULL ;
	
	gDVR_FM = dma_heap_add(&exp_info);
	if (IS_ERR(gDVR_FM))
		return PTR_ERR(gDVR_FM);

	dvr_heap_ops_fm.allocate = dvr_heap_allocate_fm;
		

	exp_info.name = "dvr-gdecmp";
	exp_info.ops = &dvr_heap_ops_gdecmp;
	exp_info.priv = NULL ;
	
	gDVR_GDECMP = dma_heap_add(&exp_info);
	if (IS_ERR(gDVR_GDECMP))
		return PTR_ERR(gDVR_GDECMP);

	dvr_heap_ops_gdecmp.allocate = dvr_heap_allocate_gdecmp;
		


	exp_info.name = "dvr-multi";
	exp_info.ops = &dvr_heap_ops_multi;
	exp_info.priv = NULL ;
	
	gDVR_MULTICHUNK = dma_heap_add(&exp_info);
	if (IS_ERR(gDVR_MULTICHUNK))
		return PTR_ERR(gDVR_MULTICHUNK);

	dvr_heap_ops_multi.allocate = dvr_heap_allocate_multi;

	return 0;
}
module_init(dvr_heap_create);

MODULE_LICENSE("GPL v2");
