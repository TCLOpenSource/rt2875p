#if !defined(__RTK_DRM_H__)
#define __RTK_DRM_H__

#if defined(__KERNEL__)
#include <drm/drm.h>
#else
#include <drm.h>
#endif

enum rtk_drm_version {
	RTK_DRM_VERSION_GDMA_ADAPTER,
};

enum rtk_drm_output_device {
	RTK_DRM_OUTPUT_GDMA_ADAPTER = 1,
};

#define RTK_DRM_IS_MASTER     (1<<0)
#define RTK_DRM_IS_CONTINUOUS (1<<1)
#define RTK_DRM_IS_IMPORT     (1<<2)

struct rtk_drm_gem_create {
	__u64 size;   /* in */
	__u32 flags;  /* in */
	__u32 handle; /* out */
};

struct rtk_drm_gem_mmap {
	__u32 handle; /* in */
	__u32 pad;
	__u64 offset; /* out */
};

#define RTK_DRM_GEM_CPU_PREP_READ   (1 << 0)
#define RTK_DRM_GEM_CPU_PREP_WRITE  (1 << 1)
#define RTK_DRM_GEM_CPU_PREP_NOWAIT (1 << 2)

struct rtk_drm_gem_cpu_prep {
	__u32 handle; /* in */
	__u32 flags;  /* in */
};

struct rtk_drm_gem_cpu_fini {
	__u32 handle; /* in */
	__u32 pad;
};

/*
 * DRM command numbers, relative to DRM_COMMAND_BASE.
 * These defines must be prefixed with "DRM_".
 */
#define RTK_DRM_GEM_CREATE   0x00
#define RTK_DRM_GEM_MMAP     0x01
#define RTK_DRM_GEM_CPU_PREP 0x02
#define RTK_DRM_GEM_CPU_FINI 0x03

/* These defines must be prefixed with "DRM_IOCTL_". */
#define DRM_IOCTL_RTK_DRM_GEM_CREATE \
	DRM_IOWR(DRM_COMMAND_BASE + RTK_DRM_GEM_CREATE, \
		 struct rtk_drm_gem_create)

#define DRM_IOCTL_RTK_DRM_GEM_MMAP \
	DRM_IOWR(DRM_COMMAND_BASE + RTK_DRM_GEM_MMAP, \
		 struct rtk_drm_gem_mmap)

#define DRM_IOCTL_RTK_DRM_GEM_CPU_PREP \
	DRM_IOW(DRM_COMMAND_BASE + RTK_DRM_GEM_CPU_PREP, \
		struct rtk_drm_gem_cpu_prep)

#define DRM_IOCTL_RTK_DRM_GEM_CPU_FINI \
	DRM_IOW(DRM_COMMAND_BASE + RTK_DRM_GEM_CPU_FINI, \
		struct rtk_drm_gem_cpu_fini)

#endif /* defined(__RTK_DRM_H__) */
