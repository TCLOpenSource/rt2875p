#if !defined(__PVR_DRM_CORE_H__)
#define __PVR_DRM_CORE_H__

#if defined(__KERNEL__)
#include <drm/drm.h>
#else
#include <asm/ioctl.h>
#include <linux/types.h>

#define DRM_IOCTL_BASE			'd'
#define DRM_COMMAND_BASE                0x40

#define DRM_IOWR(nr, type)		_IOWR(DRM_IOCTL_BASE, nr, type)

struct drm_version {
	int version_major;
	int version_minor;
	int version_patchlevel;
	__kernel_size_t name_len;
	char *name;
	__kernel_size_t date_len;
	char *date;
	__kernel_size_t desc_len;
	char *desc;
};

struct drm_set_version {
	int drm_di_major;
	int drm_di_minor;
	int drm_dd_major;
	int drm_dd_minor;
};

#define DRM_IOCTL_VERSION		DRM_IOWR(0x00, struct drm_version)
#define DRM_IOCTL_SET_VERSION		DRM_IOWR(0x07, struct drm_set_version)
#endif

#endif
