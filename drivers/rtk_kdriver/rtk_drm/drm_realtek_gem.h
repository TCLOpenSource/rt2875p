#if !defined(__DRM_PDP_GEM_H__)
#define __DRM_PDP_GEM_H__

#include <linux/dma-buf.h>
#include <linux/version.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 5, 0))
#include <drm/drm_device.h>
#include <drm/drm_file.h>
#else
#include <drm/drmP.h>
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
#include <drm/drm_gem.h>
#endif

#include "drm_realtek_drv.h"
#include "rtk_drm_dma_resv.h"

struct rtk_drm_gem_private;

struct rtk_drm_gem_object {
	struct drm_gem_object base;
	atomic_t pg_refcnt;
    uint32_t flags;

    /* members for non-continuous allocation */
    struct page **pages;
    dma_addr_t *addrs;

    /* members fir continuous allocation */
	void * virt_addr;
	uint64_t    phy_addr;

	/* Non-null if backing was imported */
	struct sg_table *sgt;
	//bool dma_map_export_host_addr;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 2, 0))
	struct dma_resv _resv;
#endif
	struct dma_resv *resv;
	bool cpu_prep;
};

#define to_rtk_drm_gem_obj(obj) container_of(obj, struct rtk_drm_gem_object, base)

struct rtk_drm_gem_private *rtk_drm_gem_init(struct drm_device *dev);

void rtk_drm_gem_cleanup(struct rtk_drm_gem_private *dev_priv);

/* ioctl functions */
int rtk_drm_gem_object_create_ioctl(struct drm_device *dev,
				     void *data,
				     struct drm_file *file);
int rtk_drm_gem_object_mmap_ioctl(struct drm_device *dev, void *data,
			      struct drm_file *file);
int rtk_drm_gem_object_cpu_prep_ioctl(struct drm_device *dev, void *data,
				  struct drm_file *file);
int rtk_drm_gem_object_cpu_fini_ioctl(struct drm_device *dev, void *data,
				  struct drm_file *file);

/* drm driver functions */
struct drm_gem_object *rtk_drm_gem_object_create(struct drm_device *dev,
					struct rtk_drm_gem_private *gem_priv, size_t size, u32 flags);

void rtk_drm_gem_object_free_priv(struct rtk_drm_gem_private *gem_priv,
			      struct drm_gem_object *obj);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0))
vm_fault_t rtk_drm_gem_object_vm_fault(struct vm_fault *vmf);
#else
int rtk_drm_gem_object_vm_fault(struct vm_area_struct *vma, struct vm_fault *vmf);
#endif

void rtk_drm_gem_vm_open(struct vm_area_struct *vma);
void rtk_drm_gem_vm_close(struct vm_area_struct *vma);


int rtk_drm_gem_object_get_pages(struct drm_gem_object *obj);
void rtk_drm_gem_object_put_pages(struct drm_gem_object *obj);


int rtk_drm_gem_pin(struct drm_gem_object *obj);
void rtk_drm_gem_unpin(struct drm_gem_object *obj);

struct sg_table * rtk_drm_gem_get_sg_table(struct drm_gem_object *obj);
int rtk_drm_gem_prime_mmap(struct drm_gem_object *obj,		  struct vm_area_struct *vma);


#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
void *rtk_drm_gem_vmap(struct drm_gem_object *obj);
void rtk_drm_gem_vunmap(struct drm_gem_object *obj, void *vaddr);
#else
int rtk_drm_gem_vmap(struct drm_gem_object *obj, struct dma_buf_map *map);
void rtk_drm_gem_vunmap(struct drm_gem_object *obj, struct dma_buf_map *map);
#endif
void rtk_drm_gem_object_free(struct drm_gem_object *obj);

struct drm_gem_object *rtk_drm_gem_prime_import(struct drm_device *dev,
					    struct dma_buf *dma_buf);

struct drm_gem_object *
rtk_drm_gem_prime_import_sg_table(struct drm_device *dev,
			      struct dma_buf_attachment *attach,
			      struct sg_table *sgt);

int rtk_drm_gem_dumb_create(struct drm_file *file,
				 struct drm_device *dev,
				 struct drm_mode_create_dumb *args);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
int rtk_drm_gem_dumb_destroy(struct drm_file *file,
			  struct drm_device *dev,
			  uint32_t handle);
#endif

int rtk_drm_gem_dumb_map_offset(struct drm_file *file, struct drm_device *dev,
			    uint32_t handle, uint64_t *offset);

/* vm operation functions */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0))
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 17, 0))
typedef int vm_fault_t;
#endif
vm_fault_t rtk_drm_gem_object_vm_fault(struct vm_fault *vmf);
#else
int rtk_drm_gem_object_vm_fault(struct vm_area_struct *vma, struct vm_fault *vmf);
#endif

/* internal interfaces */
struct dma_resv *rtk_drm_gem_get_resv(struct drm_gem_object *obj);
u64 rtk_drm_gem_get_phy_addr(struct drm_gem_object *obj);

#endif /* !defined(__DRM_PDP_GEM_H__) */
