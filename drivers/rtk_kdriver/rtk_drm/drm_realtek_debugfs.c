
#include <linux/version.h>

#include <linux/debugfs.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 5, 0))
#include <drm/drm_debugfs.h>
#include <drm/drm_file.h>
#include <linux/uaccess.h>
#endif

#include "drm_realtek_drv.h"

#define RTK_DRM_DEBUGFS_DISPLAY_ENABLED "display_enabled"

static int display_enabled_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;

	return 0;
}

static ssize_t display_enabled_read(struct file *file,
				    char __user *user_buffer,
				    size_t count,
				    loff_t *position_ptr)
{
	struct drm_device *dev = file->private_data;
	struct rtk_drm_private *dev_priv = dev->dev_private;
	loff_t position = *position_ptr;
	char buffer[] = "N\n";
	size_t buffer_size = ARRAY_SIZE(buffer);
	int err;

	if (position < 0)
		return -EINVAL;
	else if (position >= buffer_size || count == 0)
		return 0;

	if (dev_priv->display_enabled)
		buffer[0] = 'Y';

	if (count > buffer_size - position)
		count = buffer_size - position;

	err = copy_to_user(user_buffer, &buffer[position], count);
	if (err)
		return -EFAULT;

	*position_ptr = position + count;

	return count;
}

static ssize_t display_enabled_write(struct file *file,
				     const char __user *user_buffer,
				     size_t count,
				     loff_t *position)
{
	struct drm_device *dev = file->private_data;
	struct rtk_drm_private *dev_priv = dev->dev_private;
	char buffer[3];
	int err;

	count = min(count, ARRAY_SIZE(buffer) - 1);

	err = copy_from_user(buffer, user_buffer, count);
	if (err)
		return -EFAULT;
	buffer[count] = '\0';

	if (!strtobool(buffer, &dev_priv->display_enabled) && dev_priv->crtc)
		rtk_drm_crtc_set_plane_enabled(dev_priv->crtc, dev_priv->display_enabled);

	return count;
}

static const struct file_operations rtk_drm_display_enabled_fops = {
	.owner = THIS_MODULE,
	.open = display_enabled_open,
	.read = display_enabled_read,
	.write = display_enabled_write,
	.llseek = default_llseek,
};

static int rtk_drm_debugfs_create(struct drm_minor *minor, const char *name,
			      umode_t mode, const struct file_operations *fops)
{
	struct drm_info_node *node;

	/*
	 * We can't get access to our driver private data when this function is
	 * called so we fake up a node so that we can clean up entries later on.
	 */
	node = kzalloc(sizeof(*node), GFP_KERNEL);
	if (!node)
		return -ENOMEM;

	node->dent = debugfs_create_file(name, mode, minor->debugfs_root,
					 minor->dev, fops);
	if (!node->dent) {
		kfree(node);
		return -ENOMEM;
	}

	node->minor = minor;
	node->info_ent = (void *) fops;

	mutex_lock(&minor->debugfs_lock);
	list_add(&node->list, &minor->debugfs_list);
	mutex_unlock(&minor->debugfs_lock);

	return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 8, 0))
int rtk_drm_debugfs_init(struct drm_minor *minor)
#else
void rtk_drm_debugfs_init(struct drm_minor *minor)
#endif
{
	int err;

	err = rtk_drm_debugfs_create(minor, RTK_DRM_DEBUGFS_DISPLAY_ENABLED,
				 0100644,
				 &rtk_drm_display_enabled_fops);
	if (err) {
		DRM_INFO("failed to create '%s' debugfs entry\n",
			 RTK_DRM_DEBUGFS_DISPLAY_ENABLED);
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 8, 0))
	return err;
#endif
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0))
void rtk_drm_debugfs_cleanup(struct drm_minor *minor)
{
	drm_debugfs_remove_files((struct drm_info_list *) &rtk_drm_display_enabled_fops,
				 1, minor);
}
#endif
