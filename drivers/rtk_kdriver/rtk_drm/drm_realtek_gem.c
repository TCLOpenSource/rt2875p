#include <linux/version.h>

#include <drm/drm_drv.h>
#include <drm/drm_prime.h>
#include <linux/platform_device.h>
#include <drm/drm_gem.h>
#include <drm/drm_auth.h>

#include <linux/dma-buf.h>
#include <linux/pagemap.h>
#include <linux/pageremap.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/capability.h>

#include "drm_realtek_gem.h"
#include "rtk_drm.h"
#include "kernel_compatibility.h"

struct rtk_drm_gem_private {
	struct mutex			lock;
	uint64_t            size;
};

const struct vm_operations_struct rtk_drm_gem_vm_ops = {
  .fault  = rtk_drm_gem_object_vm_fault,
  .open   = rtk_drm_gem_vm_open,
  .close  = rtk_drm_gem_vm_close,
};

const struct drm_gem_object_funcs rtk_drm_gem_funcs = {
	.export = drm_gem_prime_export,
	.pin = rtk_drm_gem_pin,
	.unpin = rtk_drm_gem_unpin,
	.get_sg_table = rtk_drm_gem_get_sg_table,
	.vmap = rtk_drm_gem_vmap,
	.vunmap = rtk_drm_gem_vunmap,
	.free = rtk_drm_gem_object_free,
	.vm_ops = &rtk_drm_gem_vm_ops,
};

int rtk_drm_gem_object_get_pages(struct drm_gem_object *obj);


static struct rtk_drm_gem_object *
rtk_drm_gem_private_object_create(struct drm_device *dev,
			      size_t size,
			      struct dma_resv *resv, u32 flags)
{
	struct rtk_drm_gem_object *gem_obj;
    struct address_space *mapping;
    int err;

	WARN_ON(PAGE_ALIGN(size) != size);

	gem_obj = kzalloc(sizeof(*gem_obj), GFP_KERNEL);
	if (!gem_obj)
		return ERR_PTR(-ENOMEM);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 2, 0))
	if (!resv)
		dma_resv_init(&gem_obj->_resv);
#else
	gem_obj->base.resv = resv;
#endif

	gem_obj->base.funcs = &rtk_drm_gem_funcs;

    gem_obj->flags = flags;

    if(flags & (RTK_DRM_IS_MASTER | RTK_DRM_IS_CONTINUOUS | RTK_DRM_IS_IMPORT))
    {
        drm_gem_private_object_init(dev, &gem_obj->base, size);
    }else{
        err = drm_gem_object_init(dev, &gem_obj->base, size);
        if (err) {
            kfree(gem_obj);
            return ERR_PTR(err);
        }
        mapping = file_inode(gem_obj->base.filp)->i_mapping;
        mapping_set_gfp_mask(mapping, GFP_USER | __GFP_NOWARN | __GFP_NOMEMALLOC | __GFP_RECLAIMABLE| __GFP_ZERO);
    }

	return gem_obj;
}

struct drm_gem_object *rtk_drm_gem_object_create(struct drm_device *dev,
					     struct rtk_drm_gem_private *gem_priv,
					     size_t size,
					     u32 flags)
{
	struct rtk_drm_gem_object *gem_obj;
	int err = 0;

	gem_obj = rtk_drm_gem_private_object_create(dev, size, NULL, flags);
	if (!gem_obj) {
		err = -ENOMEM;
		goto err_exit;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 2, 0))
	gem_obj->resv = &gem_obj->_resv;
#else
	gem_obj->resv = gem_obj->base.resv;
#endif

	return &gem_obj->base;

err_exit:
	return ERR_PTR(err);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0))
 vm_fault_t rtk_drm_gem_object_vm_fault(struct vm_fault *vmf)
#else
 int rtk_drm_gem_object_vm_fault(struct vm_area_struct *vma,
				  struct vm_fault *vmf)
#endif
 {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0))
	 struct vm_area_struct *vma = vmf->vma;
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0))
	 unsigned long addr = vmf->address;
#else
	 unsigned long addr = (unsigned long)vmf->virtual_address;
#endif
	 struct drm_gem_object *obj = vma->vm_private_data;
	 struct rtk_drm_gem_object *rtk_gem_obj = to_rtk_drm_gem_obj(obj);
	 unsigned long pg_off;
	 unsigned long pfn;
	 struct page *page;
 
	 /*
	  * rtk_drm_gem_object_get_pages should have been called in
	  * rtk_drm_gem_mmap so there's no need to do it here.
	  */
	 if (WARN_ON(atomic_read(&rtk_gem_obj->pg_refcnt) == 0))
		 return VM_FAULT_SIGBUS;
	 pg_off = addr - vma->vm_start;
     if( rtk_gem_obj->flags & RTK_DRM_IS_CONTINUOUS ){
         pfn = (rtk_gem_obj->phy_addr + pg_off ) >> PAGE_SHIFT;
         page = pfn_to_page(pfn);
     }else{
        page = rtk_gem_obj->pages[pg_off>>PAGE_SHIFT];
     }
 
	 get_page(page);
	 vmf->page = page;
 
	 return 0;
 }


void rtk_drm_gem_vm_open(struct vm_area_struct *vma)
{
	struct drm_gem_object *obj = vma->vm_private_data;

	drm_gem_vm_open(vma);

	if (!obj->import_attach) {
		struct drm_device *dev = obj->dev;

		mutex_lock(&dev->struct_mutex);
		(void) rtk_drm_gem_object_get_pages(obj);
		mutex_unlock(&dev->struct_mutex);
	}
}

void rtk_drm_gem_vm_close(struct vm_area_struct *vma)
{
	struct drm_gem_object *obj = vma->vm_private_data;

	if (!obj->import_attach) {
		struct drm_device *dev = obj->dev;

		mutex_lock(&dev->struct_mutex);
		(void) rtk_drm_gem_object_put_pages(obj);
		mutex_unlock(&dev->struct_mutex);
	}

	drm_gem_vm_close(vma);
}

static void *rtk_drm_gem_vmap_priv(struct drm_gem_object *obj)
{
	struct rtk_drm_gem_object *gem_obj = to_rtk_drm_gem_obj(obj);
	if (WARN_ON(atomic_read(&gem_obj->pg_refcnt) == 0))
		return NULL;

	if(gem_obj->flags & (RTK_DRM_IS_MASTER | RTK_DRM_IS_CONTINUOUS)) {
		if(gem_obj->virt_addr)
			return gem_obj->virt_addr;
		return dvr_remap_cached_memory(gem_obj->phy_addr, obj->size, __builtin_return_address(0));
	}else{
		return vmap(gem_obj->pages, obj->size >> PAGE_SHIFT, 0, PAGE_KERNEL);
	}
}

static void rtk_drm_gem_vunmap_priv(struct drm_gem_object *obj, void *vaddr)
{
	struct rtk_drm_gem_object *gem_obj = to_rtk_drm_gem_obj(obj);

	if(gem_obj->flags & (RTK_DRM_IS_MASTER | RTK_DRM_IS_CONTINUOUS)) {
		dvr_unmap_memory(vaddr, obj->size);
	}else{
		vunmap(vaddr);
	}
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
void *rtk_drm_gem_vmap(struct drm_gem_object *obj)
{
	return rtk_drm_gem_vmap_priv(obj);
}

void rtk_drm_gem_vunmap(struct drm_gem_object *obj, void *vaddr)
{
	rtk_drm_gem_vunmap_priv(obj, vaddr);
}
#else
int rtk_drm_gem_vmap(struct drm_gem_object *obj, struct dma_buf_map *map)
{
	void *vaddr = rtk_drm_gem_vmap_priv(obj);

	dma_buf_map_set_vaddr(map, vaddr);
	return (vaddr == NULL) ? -ENOMEM : 0;
}

void rtk_drm_gem_vunmap(struct drm_gem_object *obj, struct dma_buf_map *map)
{
	rtk_drm_gem_vunmap_priv(obj, map->vaddr);
	dma_buf_map_clear(map);
}
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0) */

void rtk_drm_gem_object_free_priv(struct rtk_drm_gem_private *gem_priv,
			      struct drm_gem_object *obj)
{
	struct rtk_drm_gem_object *gem_obj = to_rtk_drm_gem_obj(obj);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 2, 0))
	if (&gem_obj->_resv == gem_obj->resv)
		dma_resv_fini(&gem_obj->_resv);
#endif
	if (obj->import_attach) {
        /* if pages and addrs arrary is not NULL, then free them, otherwise just return */
        vfree(gem_obj->pages);
        vfree(gem_obj->addrs);

		drm_gem_free_mmap_offset(obj);
		drm_prime_gem_destroy(obj, gem_obj->sgt);
	}else{
		if(atomic_read(&gem_obj->pg_refcnt) > 0){
			printk(KERN_EMERG"[rtk][drm] %s %d: free gem memory",__func__,__LINE__);
			rtk_drm_gem_object_put_pages(obj);
		}
	}

	drm_gem_object_release(&gem_obj->base);
	kfree(gem_obj);
}


int rtk_drm_gem_prime_mmap(struct drm_gem_object *obj,
		  struct vm_area_struct *vma)
{
	int err;

	mutex_lock(&obj->dev->struct_mutex);
	err = rtk_drm_gem_object_get_pages(obj);
	if (!err)
	  err = drm_gem_mmap_obj(obj, obj->size, vma);
	mutex_unlock(&obj->dev->struct_mutex);

	return err;
}

static int
rtk_drm_gem_lookup_our_object(struct drm_file *file, u32 handle,
			  struct drm_gem_object **objp)

{
	struct drm_gem_object *obj;

	obj = drm_gem_object_lookup(file, handle);
	if (!obj)
		return -ENOENT;

	if (obj->import_attach) {
		/*
		 * The dmabuf associated with the object is not one of ours.
		 * Our own buffers are handled differently on import.
		 */
		drm_gem_object_put(obj);
		return -EINVAL;
	}

	*objp = obj;
	return 0;
}

int rtk_drm_gem_object_get_pages(struct drm_gem_object *obj)
{
  struct drm_device *dev = obj->dev;
  struct rtk_drm_gem_object *gem_obj = to_rtk_drm_gem_obj(obj);
  struct rtk_drm_private *dev_priv = (struct rtk_drm_private *)dev->dev_private;
  struct rtk_drm_gem_private *drm_gem_priv = dev_priv->gem_priv;
  struct page ** pages;
  int err;

  if (WARN_ON(obj->import_attach))
	  return -EEXIST;

  WARN_ON(!mutex_is_locked(&dev->struct_mutex));

  if (atomic_inc_return(&gem_obj->pg_refcnt) == 1) {
      if (gem_obj->flags & (RTK_DRM_IS_MASTER | RTK_DRM_IS_CONTINUOUS)) {
          gem_obj->virt_addr = dvr_malloc_specific(obj->size, GFP_DCU2);
          if( !gem_obj->virt_addr ) {
              printk(KERN_EMERG"[rtk][drm] %s: dvr_malloc_specific failure ! size:0x%x\n",__func__, (unsigned int)obj->size);
              atomic_dec(&gem_obj->pg_refcnt);
              return -ENOMEM;
          }
		  memset(gem_obj->virt_addr, 0, obj->size);

          gem_obj->phy_addr = dvr_to_phys(gem_obj->virt_addr);
          printk(KERN_EMERG"[rtk][drm] %s: dvr_malloc_specific memory size:0x%x physical address:0x%llx\n", __func__, (unsigned int)obj->size, gem_obj->phy_addr);
      }else{
          unsigned int npages = obj->size >> PAGE_SHIFT;
          dma_addr_t *addrs;
          unsigned int i;

          pages = drm_gem_get_pages(obj);
          if (IS_ERR(pages)) {
              err = PTR_ERR(pages);
              goto dec_refcnt;
          }

          addrs = vmalloc(npages * sizeof(*addrs));
          if (!addrs) {
              err = -ENOMEM;
              goto free_pages;
          }

          for (i = 0; i < npages; i++) {
              addrs[i] = dma_map_page(dev->dev, pages[i], 0, PAGE_SIZE, DMA_BIDIRECTIONAL);
          }
		  printk(KERN_EMERG"[rtk][drm] %s: drm_gem_get_pages memory size:0x%x physical address(1st page):0x%llx\n", __func__, (unsigned int)obj->size, addrs[0]);

          gem_obj->pages = pages;
          gem_obj->addrs = addrs;
      }
	  mutex_lock(&drm_gem_priv->lock);
	  drm_gem_priv->size += obj->size;
	  mutex_unlock(&drm_gem_priv->lock);
  }

  return 0;
free_pages:
  drm_gem_put_pages(obj, pages, false, false);
dec_refcnt:
  atomic_dec(&gem_obj->pg_refcnt);
  return err;
}


void rtk_drm_gem_object_put_pages(struct drm_gem_object *obj)
{
	struct drm_device *dev = obj->dev;
	struct rtk_drm_gem_object *gem_obj = to_rtk_drm_gem_obj(obj);
	struct rtk_drm_private *dev_priv = (struct rtk_drm_private *)dev->dev_private;
    struct rtk_drm_gem_private *drm_gem_priv = dev_priv->gem_priv;

	if (WARN_ON(atomic_read(&gem_obj->pg_refcnt) == 0))
		return;

	if (atomic_dec_and_test(&gem_obj->pg_refcnt)) {
        if(gem_obj->flags & (RTK_DRM_IS_MASTER | RTK_DRM_IS_CONTINUOUS)) {
            printk(KERN_EMERG"[rtk][drm] %s: dvr_free memory, size:0x%x physical_addr:0x%llx\n",__func__, (unsigned int)obj->size, gem_obj->phy_addr);
            dvr_free(gem_obj->virt_addr);
            gem_obj->phy_addr = 0;
        }else{
            unsigned int npages = obj->size >> PAGE_SHIFT;
            unsigned int i;

            printk(KERN_EMERG"[rtk][drm] %s: drm_gem_put_pages, size:0x%x physical_addr (1st page):0x%llx\n",__func__, (unsigned int)obj->size, gem_obj->addrs[0]);

            for (i = 0; i < npages; i++) {
                dma_unmap_page(dev->dev, gem_obj->addrs[i], PAGE_SIZE, DMA_BIDIRECTIONAL); }

            vfree(gem_obj->addrs);
            gem_obj->addrs = NULL;

            drm_gem_put_pages(obj, gem_obj->pages, true, true);
            gem_obj->pages = NULL;
        }
		mutex_lock(&drm_gem_priv->lock);
		drm_gem_priv->size -= obj->size;
		mutex_unlock(&drm_gem_priv->lock);		
	}
}

int rtk_drm_gem_pin(struct drm_gem_object *obj)
{
	struct drm_device *dev = obj->dev;
	int err;

	mutex_lock(&dev->struct_mutex);
	err = rtk_drm_gem_object_get_pages(obj);
	mutex_unlock(&dev->struct_mutex);

	return err;
}

void rtk_drm_gem_unpin(struct drm_gem_object *obj)
{
	struct drm_device *dev = obj->dev;

	mutex_lock(&dev->struct_mutex);
	rtk_drm_gem_object_put_pages(obj);
	mutex_unlock(&dev->struct_mutex);
}

struct drm_gem_object *
rtk_drm_gem_prime_import(struct drm_device *dev,
		     struct dma_buf *dma_buf)
{
	struct drm_gem_object *obj = dma_buf->priv;

	if (obj->dev == dev) {
		drm_gem_object_get(obj);

		return obj;
	}

	return drm_gem_prime_import(dev, dma_buf);
}

struct sg_table * rtk_drm_gem_get_sg_table(struct drm_gem_object *obj)
{
	struct rtk_drm_gem_object *gem_obj = to_rtk_drm_gem_obj(obj);
	struct sg_table * sg;
	int ret;

	if (WARN_ON(atomic_read(&gem_obj->pg_refcnt) == 0))
		return NULL;

    if (gem_obj->flags & (RTK_DRM_IS_MASTER | RTK_DRM_IS_CONTINUOUS)) {
        sg = kmalloc(sizeof(struct sg_table), GFP_KERNEL);
        ret = sg_alloc_table(sg, 1, GFP_KERNEL);
        if (ret) {
            kfree(sg);
            return NULL;
        }

        sg_dma_len(sg->sgl) = obj->size;
        sg_set_page(sg->sgl, phys_to_page(gem_obj->phy_addr), obj->size, 0);
    }else{
        int nr_pages = obj->size >> PAGE_SHIFT;
        sg = drm_prime_pages_to_sg(obj->dev, gem_obj->pages, nr_pages);
    }

	return sg;
}

struct drm_gem_object *
rtk_drm_gem_prime_import_sg_table(struct drm_device *dev,
			      struct dma_buf_attachment *attach,
			      struct sg_table *sgt)
{
	struct rtk_drm_gem_object *gem_obj = NULL;
	struct page *page;
	int err = 0;
    struct page **pages;
    dma_addr_t *addrs;
    unsigned int npages;
    u32 flags = RTK_DRM_IS_IMPORT;

    if (sgt->nents == 1) {
        flags |= RTK_DRM_IS_CONTINUOUS;
    } 

	gem_obj = rtk_drm_gem_private_object_create(dev,
						attach->dmabuf->size,
						attach->dmabuf->resv, flags);
	if (!gem_obj) {
		err = -ENOMEM;
		goto err_exit;
	}

	gem_obj->sgt = sgt;


    if (gem_obj->sgt->nents <= 1) {
        gem_obj->virt_addr = NULL;
        page = sg_page(sgt->sgl);
        gem_obj->phy_addr = page_to_phys(page);
        gem_obj->resv = attach->dmabuf->resv;
    }else{
        npages = gem_obj->base.size >> PAGE_SHIFT;
        pages = vmalloc(npages * sizeof(*pages));
        addrs = vmalloc(npages * sizeof(*addrs));
        if (!pages || !addrs)
            goto exit_free_arrays;
		
#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 15, 0))
		if (drm_prime_sg_to_page_array(sgt, pages, npages) || drm_prime_sg_to_dma_addr_array(sgt, addrs, npages))
			goto exit_free_arrays;
#else
        if (drm_prime_sg_to_page_addr_arrays(sgt, pages, addrs, npages))
            goto exit_free_arrays;
#endif
        gem_obj->pages = pages;
        gem_obj->addrs = addrs;
    }

	return &gem_obj->base;

exit_free_arrays:
    vfree(pages);
    vfree(addrs);
	drm_gem_object_put(&gem_obj->base);
err_exit:
	return ERR_PTR(err);
}

int rtk_drm_gem_dumb_create_priv(struct drm_file *file,
			     struct drm_device *dev,
			     struct rtk_drm_gem_private *gem_priv,
			     struct drm_mode_create_dumb *args)
{
	struct drm_gem_object *obj;
	u32 handle;
	u32 pitch;
	size_t size;
	int err;

	pitch = args->width * (ALIGN(args->bpp, 8) >> 3);
	size = PAGE_ALIGN(pitch * args->height);

	obj = rtk_drm_gem_object_create(dev, gem_priv, size, RTK_DRM_IS_CONTINUOUS);
	if (IS_ERR(obj))
		return PTR_ERR(obj);

	err = drm_gem_handle_create(file, obj, &handle);
	if (err)
		goto exit;

	args->handle = handle;
	args->pitch = pitch;
	args->size = size;

	mutex_lock(&dev->struct_mutex);
	err = rtk_drm_gem_object_get_pages(obj);
	mutex_unlock(&dev->struct_mutex);

	if (err)
		goto exit;
exit:
	drm_gem_object_put(obj);
	return err;
}

int rtk_drm_gem_dumb_map_offset(struct drm_file *file,
			    struct drm_device *dev,
			    uint32_t handle,
			    uint64_t *offset)
{
	struct drm_gem_object *obj;
	int err;

	mutex_lock(&dev->struct_mutex);

	err = rtk_drm_gem_lookup_our_object(file, handle, &obj);
	if (err)
		goto exit_unlock;

	err = drm_gem_create_mmap_offset(obj);
	if (err)
		goto exit_obj_unref;

	*offset = drm_vma_node_offset_addr(&obj->vma_node);
exit_obj_unref:
	drm_gem_object_put(obj);
exit_unlock:
	mutex_unlock(&dev->struct_mutex);
	return err;
}

struct rtk_drm_gem_private *rtk_drm_gem_init(struct drm_device *dev)
{
	struct rtk_drm_gem_private *gem_priv =
					kmalloc(sizeof(*gem_priv), GFP_KERNEL);

	if (!gem_priv)
		return NULL;

	mutex_init(&gem_priv->lock);
	gem_priv->size = 0;

	return gem_priv;
}

void rtk_drm_gem_cleanup(struct rtk_drm_gem_private *gem_priv)
{
	mutex_destroy(&gem_priv->lock);

	kfree(gem_priv);
}

struct dma_resv *rtk_drm_gem_get_resv(struct drm_gem_object *obj)
{
	return (to_rtk_drm_gem_obj(obj)->resv);
}

u64 rtk_drm_gem_get_phy_addr(struct drm_gem_object *obj)
{
	struct rtk_drm_gem_object *gem_obj = to_rtk_drm_gem_obj(obj);
	if(gem_obj->flags & (RTK_DRM_IS_CONTINUOUS)) {
		return gem_obj->phy_addr;
	}else {
		printk(KERN_EMERG"[rtk][drm] %s %d: return dma address of 1st page: 0x%llx",__func__,__LINE__, gem_obj->addrs[0]);
		return gem_obj->addrs[0];
	}
}


int rtk_drm_gem_object_mmap_ioctl(struct drm_device *dev, void *data,
			      struct drm_file *file)
{
	struct rtk_drm_gem_mmap *args = (struct rtk_drm_gem_mmap *)data;

	if (args->pad) {
		DRM_ERROR("invalid pad (this should always be 0)\n");
		return -EINVAL;
	}

	if (args->offset) {
		DRM_ERROR("invalid offset (this should always be 0)\n");
		return -EINVAL;
	}

	return rtk_drm_gem_dumb_map_offset(file, dev, args->handle, &args->offset);
}

int rtk_drm_gem_object_cpu_prep_ioctl(struct drm_device *dev, void *data,
				  struct drm_file *file)
{
	struct rtk_drm_gem_cpu_prep *args = (struct rtk_drm_gem_cpu_prep *)data;
	struct drm_gem_object *obj;
	struct rtk_drm_gem_object *gem_obj;
	bool write = !!(args->flags & RTK_DRM_GEM_CPU_PREP_WRITE);
	bool wait = !(args->flags & RTK_DRM_GEM_CPU_PREP_NOWAIT);
	int err = 0;

	if (args->flags & ~(RTK_DRM_GEM_CPU_PREP_READ |
			    RTK_DRM_GEM_CPU_PREP_WRITE |
			    RTK_DRM_GEM_CPU_PREP_NOWAIT)) {
		DRM_ERROR("invalid flags: %#08x\n", args->flags);
		return -EINVAL;
	}

	mutex_lock(&dev->struct_mutex);

	err = rtk_drm_gem_lookup_our_object(file, args->handle, &obj);
	if (err)
		goto exit_unlock;

	gem_obj = to_rtk_drm_gem_obj(obj);

	if (gem_obj->cpu_prep) {
		err = -EBUSY;
		goto exit_unref;
	}

	if (wait) {
		long lerr;
#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 15, 0))

		lerr = dma_resv_wait_timeout(gem_obj->resv,
						 write,
						 true,
						 30 * HZ);
#else
		lerr = dma_resv_wait_timeout_rcu(gem_obj->resv,
						 write,
						 true,
						 30 * HZ);
#endif
		if (!lerr)
			err = -EBUSY;
		else if (lerr < 0)
			err = lerr;
	} else {
#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 15, 0))
		if (!dma_resv_test_signaled(gem_obj->resv,
						write))
			err = -EBUSY;
#else
		if (!dma_resv_test_signaled_rcu(gem_obj->resv,
						write))
			err = -EBUSY;
#endif
	}

	if (!err)
		gem_obj->cpu_prep = true;

exit_unref:
	drm_gem_object_put(obj);
exit_unlock:
	mutex_unlock(&dev->struct_mutex);
	return err;
}

int rtk_drm_gem_object_cpu_fini_ioctl(struct drm_device *dev, void *data,
				  struct drm_file *file)
{
	struct rtk_drm_gem_cpu_fini *args = (struct rtk_drm_gem_cpu_fini *)data;
	struct drm_gem_object *obj;
	struct rtk_drm_gem_object *gem_obj;
	int err = 0;

	if (args->pad) {
		DRM_ERROR("invalid pad (this should always be 0)\n");
		return -EINVAL;
	}

	mutex_lock(&dev->struct_mutex);

	err = rtk_drm_gem_lookup_our_object(file, args->handle, &obj);
	if (err)
		goto exit_unlock;

	gem_obj = to_rtk_drm_gem_obj(obj);

	if (!gem_obj->cpu_prep) {
		err = -EINVAL;
		goto exit_unref;
	}

	gem_obj->cpu_prep = false;

exit_unref:
	drm_gem_object_put(obj);
exit_unlock:
	mutex_unlock(&dev->struct_mutex);
	return err;
}

int rtk_drm_gem_object_create_ioctl_priv(struct drm_device *dev,
			  struct rtk_drm_gem_private *gem_priv,
			  void *data,
			  struct drm_file *file)
{
  struct rtk_drm_gem_create *args = data;
  struct drm_gem_object *obj;
  int err;

  if (args->flags) {
	  DRM_ERROR("invalid flags: %#08x\n", args->flags);
	  return -EINVAL;
  }

  if (args->handle) {
	  DRM_ERROR("invalid handle (this should always be 0)\n");
	  return -EINVAL;
  }

  if (drm_is_current_master(file))
      args->flags |= (RTK_DRM_IS_MASTER | RTK_DRM_IS_CONTINUOUS);

  obj = rtk_drm_gem_object_create(dev,
				  gem_priv,
				  PAGE_ALIGN(args->size),
				  args->flags);
  if (IS_ERR(obj))
	  return PTR_ERR(obj);

  err = drm_gem_handle_create(file, obj, &args->handle);
  drm_gem_object_put(obj);

  return err;

}

int rtk_drm_gem_object_create_ioctl(struct drm_device *dev,
					 void *data,
					 struct drm_file *file)
{
  struct rtk_drm_private *dev_priv = dev->dev_private;

  return rtk_drm_gem_object_create_ioctl_priv(dev,
					  dev_priv->gem_priv,
					  data,
					  file);
}

int rtk_drm_gem_dumb_create(struct drm_file *file,
				 struct drm_device *dev,
				 struct drm_mode_create_dumb *args)
{
  struct rtk_drm_private *dev_priv = dev->dev_private;

  return rtk_drm_gem_dumb_create_priv(file,
				  dev,
				  dev_priv->gem_priv,
				  args);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
int rtk_drm_gem_dumb_destroy(struct drm_file *file,
		   struct drm_device *dev,
		   uint32_t handle)

{
	return drm_gem_handle_delete(file, handle);
}
#endif

void rtk_drm_gem_object_free(struct drm_gem_object *obj)
{
  struct rtk_drm_private *dev_priv = obj->dev->dev_private;

  rtk_drm_gem_object_free_priv(dev_priv->gem_priv, obj);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0))
void rtk_drm_gem_object_free_unlocked(struct drm_gem_object *obj)
{
  struct drm_device *dev = obj->dev;

  mutex_lock(&dev->struct_mutex);
  rtk_drm_gem_object_free(obj);
  mutex_unlock(&dev->struct_mutex);
}
#endif

