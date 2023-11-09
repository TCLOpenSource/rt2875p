#include <linux/kernel.h>
#include <linux/usb/ch11.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <linux/uaccess.h>
#include <rtk_kdriver/io.h>
#include <mach/system.h>
#include <linux/usb/hcd.h>
#include <linux/device.h>
#include "rtk_usb_hub_smbus.h"
#include "rtk_usb_hub.h"
#include "rtk_usb_hub_wrapper.h"

static bool s_is_in_typec_source = false;
static struct bus_type *s_usb_bus_type = NULL;

struct usb_hub_port_poll_data
{
	unsigned char bus_num;   //typea port root_hub_number
	unsigned char port_num; //typea port root_port_number
	unsigned char port_sts_reg_high;  //hub port hub_port_status_reg
	unsigned char port_sts_reg_low;
	bool need_poll;
};

/*WARNING: Need to change according to different Project or IC*/
static struct usb_hub_port_poll_data g_hub_port_poll_data[] =
			CONFIG_HUB_PORT_POLL_DATA;

#define HUB_DSP_PORT_ENABLE 0x2
#define HUB_DSP_PORT_CONNECT 0x1

void rtk_usb_hub_check_port(void)
{
	int i = 0;
	int size = sizeof(g_hub_port_poll_data)/ sizeof(g_hub_port_poll_data[0]);
	for(i = 0; i < size; i++) {
		if(g_hub_port_poll_data[i].need_poll == true) {
			unsigned int reg_val = 0;
			if(rtk_smbus_read_data_transfer(g_hub_port_poll_data[i].port_sts_reg_high, 
					g_hub_port_poll_data[i].port_sts_reg_low, &reg_val) == true) {
				if((reg_val & 0x3) == 0x0) {
					g_hub_port_poll_data[i].need_poll = false;
					rtk_usb_hub_specail_dsp_enable(g_hub_port_poll_data[i].port_num - 1, false);
				}
			}
		}
	}
}


static struct usb_hub_port_poll_data * rtk_usb_hub_find_port_poll_data(unsigned char bus_num, unsigned int port_num)
{
	int i = 0;
	int size = sizeof(g_hub_port_poll_data)/ sizeof(g_hub_port_poll_data[0]);
	for(i = 0; i < size; i++) {
		if((g_hub_port_poll_data[i].bus_num == bus_num)
			&& (g_hub_port_poll_data[i].port_num == port_num))
			return &g_hub_port_poll_data[i];
	}
	return NULL;
}

static unsigned char get_usb_device_root_port_number(struct usb_device *usb_dev)
{
	while(usb_dev->parent && usb_dev->parent->parent) {
		usb_dev = usb_dev->parent;
	}
	return usb_dev->portnum;
}

static int rtk_usb_bus_notify(struct notifier_block *nb, unsigned long action,
		void *data)
{
	struct device *dev = data;

	if(!s_is_in_typec_source)
		return 0;

	switch (action) {
	case BUS_NOTIFY_ADD_DEVICE:
		if (dev->type && dev->type->name 
			&& (strcmp(dev->type->name, "usb_interface") == 0)) {
			struct usb_interface *intf = to_usb_interface(dev);
			if(intf->cur_altsetting && 
				intf->cur_altsetting->desc.bInterfaceClass == USB_CLASS_VIDEO) {
				struct usb_device *udev = interface_to_usbdev(intf);
				int portnum = get_usb_device_root_port_number(udev);
				struct usb_hub_port_poll_data *poll_data = rtk_usb_hub_find_port_poll_data(udev->bus->busnum, portnum);
				if(poll_data != NULL) {
					rtk_usb_hub_specail_dsp_enable(poll_data->port_num - 1, true);
					poll_data->need_poll = true;
				}	
			}
		}
		break;
	default:
		break;
	}
	return 0;
}

static int check_usb_device_for_switch(struct usb_device *udev, void *unused)
{
	int i  = 0;
	int intf_num = 0;
	if(!udev->actconfig)
		return 0;
	intf_num = udev->actconfig->desc.bNumInterfaces;
	for(i = 0; i < intf_num; i++) {
		struct usb_interface *intf = udev->actconfig->interface[i];
		if(intf->cur_altsetting && 
				intf->cur_altsetting->desc.bInterfaceClass == USB_CLASS_VIDEO) {
				int portnum = get_usb_device_root_port_number(udev);
				struct usb_hub_port_poll_data *poll_data = rtk_usb_hub_find_port_poll_data(udev->bus->busnum, portnum);
				if(poll_data != NULL) {
					rtk_usb_hub_specail_dsp_enable(poll_data->port_num - 1, true);
					poll_data->need_poll = true;
					break;
				}
					
		}
	}
	return 0;
}

struct each_dev_arg {
	void *data;
	int (*fn)(struct usb_device *, void *);
};

static int __each_dev(struct device *dev, void *data)
{
	struct each_dev_arg *arg = (struct each_dev_arg *)data;

	/* There are struct usb_interface on the same bus, filter them out */
	if (!dev->type || !dev->type->name 
			|| (strcmp(dev->type->name, "usb_device") != 0))
		return 0;

	return arg->fn(to_usb_device(dev), arg->data);
}

int do_usb_for_each_dev(void *data, int (*fn)(struct usb_device *, void *))
{
	struct each_dev_arg arg = {data, fn};

	return bus_for_each_dev(s_usb_bus_type, NULL, &arg, __each_dev);
}

void do_check_usb_camera_on_enter_typec(void)
{
	if(s_usb_bus_type)
		do_usb_for_each_dev(NULL, check_usb_device_for_switch);
}

void do_check_usb_camera_on_exit_typec(void)
{
	int i = 0;
	int size = sizeof(g_hub_port_poll_data)/ sizeof(g_hub_port_poll_data[0]);
	for(i = 0; i < size; i++) {
		rtk_usb_hub_specail_dsp_enable(g_hub_port_poll_data[i].port_num - 1, false);
		g_hub_port_poll_data[i].need_poll = false;
	}
}

static struct notifier_block rtk_usb_bus_nb = {
	.notifier_call = rtk_usb_bus_notify,
};


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
	.name =		"dummy_usb_hub_driver",
	.probe =	dummy_usb_probe,
	.disconnect =	dummy_usb_disconnect,
	.id_table =	dummy_usb_ids,
	.supports_autosuspend = 0,
	.soft_unbind =	0,
};


int register_typec_source_switch_notifier(struct notifier_block *nb);
int unregister_typec_source_switch_notifier(struct notifier_block *nb);

static int typec_source_switch_notifier_fn(struct notifier_block *nb, unsigned long action, void *data)
{
	if(action == 1) {//enter typec source
		do_check_usb_camera_on_enter_typec();
		s_is_in_typec_source = true;
	} else {
		do_check_usb_camera_on_exit_typec();
		s_is_in_typec_source = false;
	}
	return 0;
}

static struct notifier_block typec_source_switch_notifier = {
	.notifier_call = typec_source_switch_notifier_fn,
};


int rtk_usb_hub_register_notifier(void)
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
	if(ret) {
		s_usb_bus_type = NULL;
		goto EXIT;
	}
	ret = register_typec_source_switch_notifier(&typec_source_switch_notifier);
	if(ret) {
		bus_unregister_notifier(s_usb_bus_type, &rtk_usb_bus_nb);
		s_usb_bus_type = NULL;
		goto EXIT;
	}
EXIT:
	return 0;
}

void rtk_usb_hub_unregister_notifier(void)
{
	if(s_usb_bus_type) {
		unregister_typec_source_switch_notifier(&typec_source_switch_notifier);	
		bus_unregister_notifier(s_usb_bus_type, &rtk_usb_bus_nb);
		s_usb_bus_type = NULL;
	}
}


