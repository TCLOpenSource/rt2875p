#ifndef __RTK_HIDG_DEV_INFO_H__
#define __RTK_HIDG_DEV_INFO_H__
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/usb/g_hid.h>
#include <linux/platform_device.h>
#include <linux/version.h>

#define PLATFORM_HIDG_NAME "rtk_hidg"
#define MAX_NID_SIZE  32

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))

#define HID_MOUSE_APPLICATION_ID  0x10002
#define HID_KEYBOARD_APPLICATION_ID 0x10006
#define HID_CONSUMER_CONTROL_APPLICATION_ID 0xc0001
#define MAX_FIELD_MAPPINF_NUM 6
typedef enum {
	HIDG_USAGE_INVALID = 0x0,
	HIDG_USAGE_MOUSE_X_Y,
	HIDG_USAGE_MOUSE_WHEEL,
	HIDG_USAGE_MOUSE_BUTTON,
	HIDG_USAGE_MOUSE_X_Y_WHEEL,
	HIDG_USAGE_KEYBOARD_E0,//E0~E7
	HIDG_USAGE_KEYBOARD_00,//00~~~
	HIDG_USAGE_CONSUMER_CONTROL,
}HIDG_USAGE_TYPE;

#define MAX_CONSUMER_USAGE_MAPPING_NUM 64

struct hid_field_mapping
{
	unsigned  src_report_offset;
	unsigned  src_report_size;
	unsigned  src_report_count;
	unsigned  src_flags;
	unsigned  dst_report_offset;
	unsigned  dst_report_size;
	unsigned  dst_report_count;
	unsigned  dst_flags;
	HIDG_USAGE_TYPE usage_type;
	bool src_signed;
	bool dst_signed;
	unsigned char consumer_usage_mapping[MAX_CONSUMER_USAGE_MAPPING_NUM];
	unsigned char consumer_usage_mapping_cnt;

};

struct hid_report_mapping
{
	unsigned short dst_report_id; 
	unsigned short src_report_id;
	unsigned int src_size;
	unsigned int dst_size;
	unsigned char dst_device_id;
	unsigned char src_numbered;
	unsigned char dst_numbered;
	bool active;
	unsigned int application;
	struct hid_field_mapping field_mappings[MAX_FIELD_MAPPINF_NUM];
	unsigned char mapping_cnt;	
	
};

#define MAX_REPORT_MAPPING_CNT  32
struct hid_report_mapping_data
{
	struct hid_report_mapping input_report_mapping[MAX_REPORT_MAPPING_CNT];
};

#endif


struct rtk_hidg_report_desc_data
{
	 unsigned int id; //must equial to platorm device id
        char nid[MAX_NID_SIZE];
        struct hidg_func_descriptor*  func_desc;
        struct hid_device *hid_devices;
};

struct rtk_hidg_report_desc_data * rtk_hidg_get_hid_func_node_by_name(const char *name);
bool rtk_hidg_plat_devices_add(void);
void rtk_hidg_plat_devices_remove(void);

#endif
