#include <linux/kernel.h>
#include <linux/usb/ch11.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <linux/uaccess.h>
#include <rtk_kdriver/io.h>
#include <mach/system.h>
#include <linux/usb/hcd.h>
#include <linux/device.h>
#include "../../usb/core/hub.h"
#include "rtk_usb_core.h"


#if IS_ENABLED(CONFIG_RTK_KDRV_EHCI_HCD_PLATFORM)
int rtk_ehci_port_test_mode(struct usb_device *udev, int port1, int mode);
#endif
#if IS_ENABLED(CONFIG_RTK_KDRV_XHCI_HCD_PLATFORM)	
int rtk_xhci_port_test_mode(struct usb_device *udev, int port1, int mode);
#endif

#define UsbTestModeNumber 6
#define UsbSuspendResume 1
#define UsbGetDescriptor 3


int rtk_usb_test_mode_suspend_flag = 0;
EXPORT_SYMBOL(rtk_usb_test_mode_suspend_flag);
int rtk_gUsbHubSuspendResume[4] = {-1, -1, -1, -1}; /*  0:Resume, 1:Suspend */
EXPORT_SYMBOL(rtk_gUsbHubSuspendResume);

static int gUsbHubPortSuspendResume = 0; /*  0:Resume, 1:Suspend */
static int gUsbHubPort = 1; /* default = 1 */
static int gUsbHubTestMode = 0;
//static int gUsbTestModeChanged = 0;

/*
*  Get the type of host controller: OHCI/EHCI/XHCI
*  Return value:  0:fail;1:OHCI;2:EHCI:3:XHCI
*/
static int get_usb_host_controller_type(struct usb_device *udev)
{
	struct usb_device *roothub;
	if(!udev)
		return 0;
	roothub = udev;
	while(roothub && roothub->parent) {
		roothub = roothub->parent;
	}
	if(roothub && roothub->product) {
		if(strstr(roothub->product, "EHCI Host")) {
			return 2;
		} else if(strstr(roothub->product, "xHCI Host")) {
			return 3;
		} else if(strstr(roothub->product, "OHCI Host")) {
			return 1;
		} 
	}
	return 0;
}


/*
 * USB 2.0 spec Section 11.24.2.2
 */
static int hub_clear_port_feature(struct usb_device *hdev, int port1, int feature)
{
	return usb_control_msg(hdev, usb_sndctrlpipe(hdev, 0),
			(u8)ClearPortFeature, USB_RT_PORT, feature, port1,
			NULL, 0, 1000);
}


/*
 * USB 2.0 spec Section 11.24.2.13
 */
static int hub_set_port_feature(struct usb_device *hdev, int port1, int feature)
{
	return usb_control_msg(hdev, usb_sndctrlpipe(hdev, 0),
			(u8)SetPortFeature, USB_RT_PORT, feature, port1,
			NULL, 0, 1000);
}


static unsigned int volatile regs_addr = 0;
static ssize_t show_regs_addr(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%x\n", regs_addr);
}
static ssize_t store_regs_addr(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned int value;

	if (kstrtouint(buf, 16, &value) != 0)
		return -EINVAL;

	regs_addr = value;
	RTK_USB_DBG("0x%x\n", regs_addr);
	return count;
}
static DEVICE_ATTR(rtk_regs_addr, S_IRUGO | S_IWUSR, show_regs_addr, store_regs_addr);

static ssize_t show_regs_value(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (regs_addr >= 0xb8000000)
		return snprintf(buf, PAGE_SIZE, "0x%x = %x\n", regs_addr, rtd_inl(regs_addr));
       return 0;
}
static ssize_t store_regs_value(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int value;

	if (kstrtouint(buf, 16, &value) != 0)
		return -EINVAL;

	rtd_outl(regs_addr, value);
	return count;
}
static DEVICE_ATTR(rtk_regs_value, S_IRUGO | S_IWUSR, show_regs_value, store_regs_value);

static ssize_t  show_bPortNumber (struct device *dev, struct device_attribute *attr, char *buf)
{
	struct usb_device *udev = to_usb_device (dev);

	if(udev->descriptor.bDeviceClass != USB_CLASS_HUB)
		return snprintf(buf, PAGE_SIZE, "Not a hub device\n");
	return snprintf(buf, PAGE_SIZE, "%d\n", gUsbHubPort);
}

static ssize_t set_bPortNumber (struct device *dev, struct device_attribute *attr, 
								const char *buf, size_t count)
{
	struct usb_device	*udev = to_usb_device (dev);
	int config;

	if (kstrtoint(buf, 10, &config) != 0 || config > udev->maxchild || config < 1)
		return -EINVAL;

	if(udev->descriptor.bDeviceClass != USB_CLASS_HUB)
		return -ENXIO;

	if(gUsbHubPort != config)
		gUsbHubPort = config;

	return count;
}

static DEVICE_ATTR(rtk_bPortNumber, S_IRUGO | S_IWUSR, show_bPortNumber, set_bPortNumber);

static ssize_t  show_bPortDescriptor (struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "\n");
}

static struct usb_hub *rtk_usb_hub_to_struct_hub(struct usb_device *hdev)
{
	if (!hdev || !hdev->actconfig || !hdev->maxchild)
		return NULL;
	return usb_get_intfdata(hdev->actconfig->interface[0]);
}

static int rtk_get_hub_descriptor_port(struct usb_device *hdev, void *data, int size, int port1)
{
	struct usb_hub *hub = NULL;
	struct usb_device *dev = NULL;
	hub = rtk_usb_hub_to_struct_hub(hdev);
	if(!hub)
		return 0;
	
	dev = hub->ports[port1 - 1]->child;
	if (dev) {
		memset(data, 0, size);
		return usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
				USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
				(USB_DT_DEVICE << 8), 0, data, size,
				USB_CTRL_GET_TIMEOUT);

	}
	return 0;
}

static ssize_t set_bPortDescriptor (struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct usb_device	*udev = to_usb_device (dev);
	int config;
	unsigned char *data = NULL;
	int	i, ret;
	unsigned int desc_size = 18;
	int port = gUsbHubPort;

	if (kstrtoint(buf, 10, &config) != 0 || config > UsbGetDescriptor)
		return -EINVAL;

	if(udev->descriptor.bDeviceClass != USB_CLASS_HUB || port < 1)
		return -ENXIO;

	data = (unsigned char *)kmalloc(desc_size, GFP_KERNEL);
	if(!data)
		return -ENOMEM;

	ret = rtk_get_hub_descriptor_port(udev, data, desc_size, port);
	if(ret == desc_size) {
		RTK_USB_INFO(" get device descriptor:\n");
		for( i = 0; i < desc_size; i++) {
			RTK_USB_INFO(" %.2x", data[i]);
			if((i % 15) == 0 && (i != 0))
				RTK_USB_INFO("\n<1>");
		}
		RTK_USB_INFO("\n");
	}
	kfree(data);
	return count;
}

static DEVICE_ATTR(rtk_bPortDescriptor, S_IRUGO | S_IWUSR,
		show_bPortDescriptor, set_bPortDescriptor);

/*
 * USB 2.0 spec Section 11.24.2.7
 */
static int hub_get_port_status(struct usb_device *hdev, int port1, unsigned char *buf, int length)
{
	return usb_control_msg(hdev, usb_rcvctrlpipe(hdev, 0),
			(u8)GetPortStatus, USB_RT_PORT | USB_DIR_IN, 0, port1,
			buf, length, 1000);
}

static int hub_check_port_suspend_resume(struct usb_device *hdev, int port1)
{
	unsigned char *data = NULL;
	unsigned int buf_size = 4;
	int ret = -1;

	data = (unsigned char *)kmalloc(buf_size, GFP_KERNEL);
	if(!data)
		return ret;
	
	hub_get_port_status(hdev, port1, data, buf_size);
	ret = (data[0] >> 2) & 0x1; /* bit 2 */
	
	kfree(data);
	return ret;
}


static ssize_t  show_bPortSuspendResume (struct device *dev, struct device_attribute *attr, char *buf)
{
	struct usb_device *udev = to_usb_device (dev);

	if(udev->descriptor.bDeviceClass != USB_CLASS_HUB)
		return snprintf(buf, PAGE_SIZE, "Not a hub device\n");

	if(udev->parent == NULL) { /*  root hub */
		return snprintf(buf, PAGE_SIZE, "%d %d %d %d\n", rtk_gUsbHubSuspendResume[0], \
				rtk_gUsbHubSuspendResume[1], rtk_gUsbHubSuspendResume[2], rtk_gUsbHubSuspendResume[3]);
	}
	else { /*  not root hub */
		gUsbHubPortSuspendResume = hub_check_port_suspend_resume(udev, gUsbHubPort);
		return snprintf(buf, PAGE_SIZE, "%d\n", gUsbHubPortSuspendResume);
	}

	return 0;
}


static ssize_t set_bPortSuspendResume (struct device *dev, 
					struct device_attribute *attr, const char *buf, size_t count)
{
	struct usb_device	*udev = to_usb_device (dev);
	struct usb_hcd		*hcd = container_of(udev->bus, struct usb_hcd, self);
	int			config;
	unsigned int		port1;

	if (kstrtoint(buf, 10, &config) != 0 || config > UsbSuspendResume)
		return -EINVAL;

	if(udev->descriptor.bDeviceClass != USB_CLASS_HUB)
		return -ENXIO;

	if(udev->parent == NULL) {/*  root hub */
		port1 = (unsigned int) gUsbHubPort;
		if(port1 >= 1 && rtk_gUsbHubSuspendResume[port1 - 1] != config)
		{
			rtk_gUsbHubSuspendResume[port1 - 1] = config;
			RTK_USB_DBG("Root Hub port %d - %s\n", port1, (rtk_gUsbHubSuspendResume[port1 - 1] == 1) ? "Suspend" : "Resume");
			if(rtk_gUsbHubSuspendResume[port1 - 1] == 1) /* Suspend */
			{
				rtk_usb_test_mode_suspend_flag = 1;

				RTK_USB_DBG("call bus_suspend() to the root hub port %d...\n", gUsbHubPort);
				usb_lock_device (hcd->self.root_hub);

				if (hcd->driver->bus_suspend) {
					hcd->driver->bus_suspend(hcd);
				} else {
					RTK_USB_ERR("#@# %s(%d) hcd->driver->bus_suspend(hcd) is NULL!!! Need CONFIG_PM=y\n", __func__, __LINE__);
				}

				usb_unlock_device (hcd->self.root_hub);
				RTK_USB_DBG("call bus_suspend() OK !!!\n");

			}
			else /* Resume */
			{
				rtk_usb_test_mode_suspend_flag = 0;

				RTK_USB_DBG("call bus_resume() to the root hub port %d...\n", gUsbHubPort);
				usb_lock_device (hcd->self.root_hub);

				if (hcd->driver->bus_resume) {
					hcd->driver->bus_resume(hcd);
				} else {
					RTK_USB_DBG("#@# %s(%d) hcd->driver->bus_resume(hcd) is NULL!!! Need CONFIG_PM=y\n", __func__, __LINE__);
				}

				usb_unlock_device (hcd->self.root_hub);
				RTK_USB_DBG("call bus_resume() OK !!!\n");

			}
			msleep(1000);
		}
	}
	else /*  the hub which is not root hub */
	{
		gUsbHubPortSuspendResume = hub_check_port_suspend_resume(udev, gUsbHubPort);

		if(gUsbHubPortSuspendResume != config)
		{
			gUsbHubPortSuspendResume = config;
			if(gUsbHubPortSuspendResume == 1) /* Suspend */
			{
				RTK_USB_DBG("set USB_PORT_FEAT_SUSPEND to the port %d of the hub ...\n", gUsbHubPort);
				hub_set_port_feature(udev, gUsbHubPort, USB_PORT_FEAT_SUSPEND);
				RTK_USB_DBG("set OK !!!\n");
			}
			else /* Resume */
			{
				RTK_USB_DBG("clear USB_PORT_FEAT_SUSPEND to the port %d of the hub ...\n", gUsbHubPort);
				hub_clear_port_feature(udev, gUsbHubPort, USB_PORT_FEAT_SUSPEND);
				RTK_USB_DBG("clear OK !!!\n");
			}
			msleep(1000);
		}
	}

	return count;
}

static DEVICE_ATTR(rtk_bPortSuspendResume, S_IRUGO | S_IWUSR,
		show_bPortSuspendResume, set_bPortSuspendResume);


static ssize_t  show_bPortSuspendResume_ctrl (struct device *dev, struct device_attribute *attr, char *buf)
{
	struct usb_device *udev = to_usb_device (dev);

	if(udev->descriptor.bDeviceClass != USB_CLASS_HUB)
		return snprintf(buf, PAGE_SIZE, "%d\n", -1);

	if(udev->parent == NULL) /*  root hub */
	{
		return snprintf(buf, PAGE_SIZE, "%d %d %d %d\n", rtk_gUsbHubSuspendResume[0], \
				rtk_gUsbHubSuspendResume[1], rtk_gUsbHubSuspendResume[2], rtk_gUsbHubSuspendResume[3]);
	}
	else /*  not root hub */
	{
		gUsbHubPortSuspendResume = hub_check_port_suspend_resume(udev, gUsbHubPort);
		return snprintf(buf, PAGE_SIZE, "%d\n", gUsbHubPortSuspendResume);
	}

	return 0;
}

static ssize_t
set_bPortSuspendResume_ctrl (struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct usb_device	*udev = to_usb_device (dev);
	//struct usb_hcd		*hcd = container_of(udev->bus, struct usb_hcd, self);
	int			config;
	unsigned int		port1;

	if (kstrtoint(buf, 10, &config) != 0 || config > UsbSuspendResume)
		return -EINVAL;

	if(udev->descriptor.bDeviceClass != USB_CLASS_HUB)
		return -ENXIO;

	if(udev->parent == NULL) /*  root hub */
	{
		port1 = (unsigned int) gUsbHubPort;
		if(port1 >= 1)
		{
			rtk_gUsbHubSuspendResume[port1 - 1] = config;
			RTK_USB_DBG("Root Hub port %d - %s\n", port1, (rtk_gUsbHubSuspendResume[port1 - 1] == 1) ? "Suspend" : "Resume");
			if(rtk_gUsbHubSuspendResume[port1 - 1] == 1) /* Suspend */
			{
				rtk_usb_test_mode_suspend_flag = 1;

				RTK_USB_DBG("call bus_suspend() to the root hub port %d...\n", gUsbHubPort);
				hub_set_port_feature(udev, gUsbHubPort, USB_PORT_FEAT_SUSPEND);
				RTK_USB_DBG("call bus_suspend() OK !!!\n");

			}
			else /* Resume */
			{
				rtk_usb_test_mode_suspend_flag = 0;

				RTK_USB_DBG("call bus_resume() to the root hub port %d...\n", gUsbHubPort);
				hub_clear_port_feature(udev, gUsbHubPort, USB_PORT_FEAT_SUSPEND);
				RTK_USB_DBG("call bus_resume() OK !!!\n");

			}
			msleep(1000);
		}
	}
	else /*  the hub which is not root hub */
	{
		gUsbHubPortSuspendResume = hub_check_port_suspend_resume(udev, gUsbHubPort);

		/* if(gUsbHubPortSuspendResume != config) */
		{
			gUsbHubPortSuspendResume = config;
			if(gUsbHubPortSuspendResume == 1) /* Suspend */
			{
				RTK_USB_DBG("set USB_PORT_FEAT_SUSPEND to the port %d of the hub ...\n", gUsbHubPort);
				hub_set_port_feature(udev, gUsbHubPort, USB_PORT_FEAT_SUSPEND);
				RTK_USB_DBG("set OK !!!\n");
			}
			else /* Resume */
			{
				RTK_USB_DBG("clear USB_PORT_FEAT_SUSPEND to the port %d of the hub ...\n", gUsbHubPort);
				hub_clear_port_feature(udev, gUsbHubPort, USB_PORT_FEAT_SUSPEND);
				RTK_USB_DBG("clear OK !!!\n");
			}
			msleep(1000);
		}
	}

	return count;
}

static DEVICE_ATTR(rtk_bPortSuspendResume_ctrl, S_IRUGO | S_IWUSR,
		show_bPortSuspendResume_ctrl, set_bPortSuspendResume_ctrl);


static ssize_t  show_bPortTestMode (struct device *dev, struct device_attribute *attr, char *buf)
{
	struct usb_device *udev = to_usb_device (dev);

	if(udev->descriptor.bDeviceClass != USB_CLASS_HUB)
		return snprintf(buf, PAGE_SIZE, "Not a hub device\n");

	return snprintf(buf, PAGE_SIZE, "%d\n", gUsbHubTestMode);
}

static ssize_t
set_bPortTestMode (struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct usb_device *udev = to_usb_device(dev);
	struct usb_hcd *hcd;
	int host_controller_type;
	int value = -EINVAL, mode;
	int port = gUsbHubPort;

	if (kstrtoint(buf, 10, &mode) != 0)
		return -EINVAL;

	if(udev->descriptor.bDeviceClass != USB_CLASS_HUB || port < 1) {
		RTK_USB_ERR("device is not hub \n");
		return -ENXIO;
	}

	hcd = container_of(udev->bus, struct usb_hcd, self);
	if(!hcd)
		return -ENXIO;

	host_controller_type = get_usb_host_controller_type(udev);
	if(host_controller_type == 0)
		return -ENXIO;
	
	value = 0;
	if(host_controller_type == 2) { //EHCI
#if IS_ENABLED(CONFIG_RTK_KDRV_EHCI_HCD_PLATFORM)	
	RTK_USB_INFO("port_test_mode, port=%d mode=%d \n", port, mode);
	if (!rtk_ehci_port_test_mode(udev, port, mode)) {
		RTK_USB_ERR("port_test_mode success \n");
	}
#endif
	} else if(host_controller_type == 3) { //EHCI
#if IS_ENABLED(CONFIG_RTK_KDRV_XHCI_HCD_PLATFORM)	
	RTK_USB_INFO("port_test_mode, port=%d mode=%d \n", port, mode);
	if (!rtk_xhci_port_test_mode(udev, port, mode)) {
		RTK_USB_ERR("port_test_mode success \n");
	}
#endif
	}
	return count;
}


static DEVICE_ATTR(rtk_bPortTestMode, S_IRUGO | S_IWUSR,
		show_bPortTestMode, set_bPortTestMode);


static const struct attribute * const rtk_usb_dev_attrs[] = {

	&dev_attr_rtk_regs_addr.attr,
	&dev_attr_rtk_regs_value.attr,
	&dev_attr_rtk_bPortNumber.attr,
	&dev_attr_rtk_bPortDescriptor.attr,
	&dev_attr_rtk_bPortSuspendResume.attr,
	&dev_attr_rtk_bPortSuspendResume_ctrl.attr,
	&dev_attr_rtk_bPortTestMode.attr,
	NULL,
};

static int rtk_usb_bus_notify(struct notifier_block *nb, unsigned long action,
		void *data)
{
	int ret;
	struct device *dev = data;

	switch (action) {
	case BUS_NOTIFY_ADD_DEVICE:
		if (dev->type && dev->type->name 
			&& (strcmp(dev->type->name, "usb_device") == 0))
			ret = sysfs_create_files(&dev->kobj, rtk_usb_dev_attrs);
		break;
	case BUS_NOTIFY_BOUND_DRIVER:
		if (dev->type && dev->type->name 
			&& (strcmp(dev->type->name, "usb_interface") == 0)) {
			struct usb_interface *intf = to_usb_interface(dev);
			struct usb_device *udev = interface_to_usbdev(intf);
			if(intf->cur_altsetting && 
				intf->cur_altsetting->desc.bInterfaceClass == USB_CLASS_HUB) {
				if(udev->parent && udev->speed > USB_SPEED_HIGH)
					usb_disable_autosuspend(udev);
			}
		}
		break;
	case BUS_NOTIFY_DEL_DEVICE:
		if (dev->type && dev->type->name 
			&& (strcmp(dev->type->name, "usb_device") == 0))
			sysfs_remove_files(&dev->kobj, rtk_usb_dev_attrs);
		break;
	}
	return 0;
}

static struct notifier_block rtk_usb_bus_nb = {
	.notifier_call = rtk_usb_bus_notify,
};

static struct bus_type *s_usb_bus_type = NULL;

static struct usb_device_id dummy_usb_ids[] = {
	{ }
};

static int dummy_usb_probe(struct usb_interface *intf,
			 const struct usb_device_id *id)
{
	return -1;
}

static void dummy_usb_disconnect(struct usb_interface *intf)
{
	
}

static struct usb_driver dummy_usb_driver = {
	.name =		"dummy_usb_driver",
	.probe =	dummy_usb_probe,
	.disconnect =	dummy_usb_disconnect,
	.id_table =	dummy_usb_ids,
	.supports_autosuspend = 0,
	.soft_unbind =	0,
};

int rtk_usb_register_notifier(void)
{
	int ret;
	ret = usb_register(&dummy_usb_driver);
	if(ret)
		goto EXIT;
	s_usb_bus_type = dummy_usb_driver.drvwrap.driver.bus;
	usb_deregister(&dummy_usb_driver);
	if(!s_usb_bus_type)
		goto EXIT;
	ret = bus_register_notifier(s_usb_bus_type, &rtk_usb_bus_nb);
	if(ret)
		s_usb_bus_type = NULL;
EXIT:
	return 0;
}

void rtk_usb_unregister_notifier(void)
{
	if(s_usb_bus_type) {
		bus_unregister_notifier(s_usb_bus_type, &rtk_usb_bus_nb);
		s_usb_bus_type = NULL;
	}
}

