#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/usb/g_hid.h>
#include <linux/hid.h>
#include <rtd_log/rtd_module_log.h>
#include <linux/uhid.h>
#include "rtk_composite_platform.h"
#include "rtk_hidg_dev_info.h"


static struct hid_driver dummy_hid_driver;

static void rtk_hidg_dev_release(struct device *dev)
{
        
}

/*********************USB Keyboard*********************************/
static struct hidg_func_descriptor rtk_gadget_hidg_func_desc0 =
{
        .subclass                       = 0, /* No subclass */
        .protocol                       = 1, /* Keyboard */
        .report_length          = 9,
        .report_desc_length     = 123,
        .report_desc            = {
                0x05, 0x01,     /* USAGE_PAGE (Generic Desktop)           */
                0x09, 0x06,     /* USAGE (Keyboard)                       */
                0xa1, 0x01,     /* COLLECTION (Application)               */
                0x85, 0x01,    /* REPORT_ID (1)               */
                0x05, 0x07,     /*   USAGE_PAGE (Keyboard)                */
                0x19, 0xe0,     /*   USAGE_MINIMUM (Keyboard LeftControl) */
                0x29, 0xe7,     /*   USAGE_MAXIMUM (Keyboard Right GUI)   */
                0x15, 0x00,     /*   LOGICAL_MINIMUM (0)                  */
                0x25, 0x01,     /*   LOGICAL_MAXIMUM (1)                  */
                0x75, 0x01,     /*   REPORT_SIZE (1)                      */
                0x95, 0x08,     /*   REPORT_COUNT (8)                     */
                0x81, 0x02,     /*   INPUT (Data,Var,Abs)                 */
                0x95, 0x01,     /*   REPORT_COUNT (1)                     */
                0x75, 0x08,     /*   REPORT_SIZE (8)                      */
                0x81, 0x03,     /*   INPUT (Cnst,Var,Abs)                 */
                0x95, 0x05,     /*   REPORT_COUNT (5)                     */
                0x75, 0x01,     /*   REPORT_SIZE (1)                      */
                0x05, 0x08,     /*   USAGE_PAGE (LEDs)                    */
                0x19, 0x01,     /*   USAGE_MINIMUM (Num Lock)             */
                0x29, 0x05,     /*   USAGE_MAXIMUM (Kana)                 */
                0x91, 0x02,     /*   OUTPUT (Data,Var,Abs)                */
                0x95, 0x01,     /*   REPORT_COUNT (1)                     */
                0x75, 0x03,     /*   REPORT_SIZE (3)                      */
                0x91, 0x03,     /*   OUTPUT (Cnst,Var,Abs)                */
                0x95, 0x06,     /*   REPORT_COUNT (6)                     */
                0x75, 0x08,     /*   REPORT_SIZE (8)                      */
                0x15, 0x00,     /*   LOGICAL_MINIMUM (0)                  */
                0x25, 0xA0,     /*   LOGICAL_MAXIMUM (101)                */
                0x05, 0x07,     /*   USAGE_PAGE (Keyboard)                */
                0x19, 0x00,     /*   USAGE_MINIMUM (Reserved)             */
                0x29, 0xA0,     /*   USAGE_MAXIMUM (Keyboard Application) */
                0x81, 0x00,     /*   INPUT (Data,Ary,Abs)                 */
                0xc0,            /* END_COLLECTION                         */

                0x05,0x0c,/*Consumer Control*/
                0x09,0x01,
                0xa1,0x01,
                0x85,0x02,
                0x15,0x00,
                0x25,0x01,
                0x75,0x01,
                0x95,0x10,
                0x0a,0x23,
                0x02,0x0a,
                0x21,0x02,
                0x0a,0x8a,
                0x01,0x0a,
                0xae,0x01,
                0x0a,0x96,
                0x01,0x0a,
                0x83,0x01,
                0x0a,0xae,
                0x01,0x09,
                0x30,0x09,
                0x70,0x09,
                0x6f,0x09,
                0xb6,0x09,
                0xcd,0x09,
                0xb5,0x09,
                0xe2,0x09,
                0xea,0x09,
                0xe9,0x81,
                0x02,0xc0,            
        }
};

struct rtk_hidg_report_desc_data rtk_gadget_hidg_func_desc_data0 =
{
        .id = 0,
        .nid = "KeyBoard0",
        .func_desc = &rtk_gadget_hidg_func_desc0,
};

/*********************USB Mouse 1*********************************/
static struct hidg_func_descriptor rtk_gadget_hidg_func_desc1 =
{
        .subclass = 0,  /*NO SubClass*/
        .protocol = 2,  /*Mouse*/
        .report_length = 16,
        .report_desc_length = 68,
        .report_desc={
	 0x05,0x01,	/*Usage Page (Generic Desktop Controls)*/
                0x09,0x02,	/*Usage (Mouse)*/
                0xa1,0x01,	/*Collction (Application)*/
                0x09,0x01,	/*Usage (pointer)*/
                0xa1,0x00,	/*Collction (Physical)*/
                0x85, 0x01,            //REPORT 1
                0x05,0x09,	/*Usage Page (Button)*/
                0x19,0x01,	/*Usage Minimum(1)*/
                0x29,0x10,	/*Usage Maximum(3) */ 
                0x15,0x00,	/*Logical Minimum(0)*/
                0x25,0x01,	/*Logical Maximum(1)*/
                0x95,0x10,	/*Report Count(16)  */
                0x75,0x01,	/*Report Size(1)*/
                0x81,0x02,	/*Input(Data,Variable,Absolute,BitFiled)*/
                
                0x05,0x01,	/*Usage Page (Generic Desktop Controls)*/
                0x09,0x30,	/*Usage(x)*/
                0x09,0x31,	/*Usage(y)*/
                0x16,0x01,
                0xF8,	              /*Logical Minimum(-2047)*/
                0x26,0xff,
                0x07,                    /*Logical Maximum(2047)*/
                0x75,0x0c,	/*Report Size(12)*/
                0x95,0x02,	/*Report Count(2)  */
                0x81,0x06,	/*Input(Data,Variable,Relative,BitFiled)*/
               
                0x16,0x01,
                0xF8,	              /*Logical Minimum(-2047)*/
                0x26,0xff,
                0x07,                    /*Logical Maximum(2047)*/
                
                0x75,0x0c,            /*Report Size(12)*/
                0x95,0x01,            /*Report Count(1)  */   
	 0x09,0x38,	/*Usage(Wheel)*/
	 0x81,0x06,	/*Input(Data,Variable,Relative,BitFiled)*/
	         0x75,0x04,
                0x95,0x01,
                0x81,0x01,
                

                0xc0,	/*End Collection*/
                0xc0,	/*End Collection*/
        }
};

struct rtk_hidg_report_desc_data rtk_gadget_hidg_func_desc_data1 =
{
	 .id = 1,
        .nid = "Mouse1",
        .func_desc = &rtk_gadget_hidg_func_desc1,
};

/*********************USB Mouse 2*********************************/
static struct hidg_func_descriptor rtk_gadget_hidg_func_desc2 =
{
        .subclass = 0,  /*NO SubClass*/
        .protocol = 2,  /*Mouse*/
        .report_length = 16,
        .report_desc_length = 56,
        .report_desc={
	 0x05,0x01,	/*Usage Page (Generic Desktop Controls)*/
                0x09,0x02,	/*Usage (Mouse)*/
                0xa1,0x01,	/*Collction (Application)*/
                0x09,0x01,	/*Usage (pointer)*/
                0xa1,0x00,	/*Collction (Physical)*/

                0x85, 0x02,            //REPORT 2
                0x05,0x09,	/*Usage Page (Button)*/
                0x19,0x01,	/*Usage Minimum(1)*/
                0x29,0x10,	/*Usage Maximum(3) */ 
                0x15,0x00,	/*Logical Minimum(0)*/
                0x25,0x01,	/*Logical Maximum(1)*/
                0x95,0x10,	/*Report Count(16)  */
                0x75,0x01,	/*Report Size(1)*/
                0x81,0x02,	/*Input(Data,Variable,Absolute,BitFiled)*/
                
                0x05,0x01,	/*Usage Page (Generic Desktop Controls)*/
                0x09,0x30,	/*Usage(x)*/
                0x09,0x31,	/*Usage(y)*/
                0x09,0x38,  /*Usage(Wheel)*/
                0x16,0x01,
                0xF8,	              /*Logical Minimum(-2047)*/
                0x26,0xff,
                0x07,                    /*Logical Maximum(2047)*/
                0x75,0x0c,	/*Report Size(12)*/
                0x95,0x03,	/*Report Count(3)  */
                0x81,0x06,	/*Input(Data,Variable,Relative,BitFiled)*/
                0x75,0x04,
                0x95,0x01,
                0x81,0x01,
                0xc0,	/*End Collection*/
                0xc0,	/*End Collection*/
        }
};

struct rtk_hidg_report_desc_data rtk_gadget_hidg_func_desc_data2 =
{
	 .id = 2,
        .nid = "Mouse2",
        .func_desc = &rtk_gadget_hidg_func_desc2,
};

static struct platform_device rtk_gadget_hidg_plat_devices[] =
{
        {
                .name                   = PLATFORM_HIDG_NAME,
                .id                             = 0,
                .num_resources  = 0,
                .resource               = 0,
                .dev.release = rtk_hidg_dev_release,
                .dev.platform_data      = &rtk_gadget_hidg_func_desc_data0,
                
        },
        {
                .name                   = PLATFORM_HIDG_NAME,
                .id                             = 1,
                .num_resources  = 0,
                .resource               = 0,
                .dev.release = rtk_hidg_dev_release,
                .dev.platform_data      = &rtk_gadget_hidg_func_desc_data1,
        },
        {
                .name                   = PLATFORM_HIDG_NAME,
                .id                             = 2,
                .num_resources  = 0,
                .resource               = 0,
                .dev.release = rtk_hidg_dev_release,
                .dev.platform_data      = &rtk_gadget_hidg_func_desc_data2,
        },
};

#define HIG_PLAT_DEVICE_NUM (sizeof(rtk_gadget_hidg_plat_devices) / sizeof(rtk_gadget_hidg_plat_devices[0]))

#if 0
void rtk_hidg_print_hid_field(struct hid_field *field)
{
	int i = 0;
	RTK_COMPOSITE_ERR("SLL***********************start field**********************************\n");
	RTK_COMPOSITE_ERR("SLL physical=%x, logical=%x, application=%x, report_size=%x, report_count=%x\n",
		field->physical, field->logical, field->application, field->report_size, field->report_count);
	for(i = 0; i < field->maxusage; i++) 
		RTK_COMPOSITE_ERR("SLL usage[%d] = %x\n", i, field->usage[i].hid);
	RTK_COMPOSITE_ERR("SLL ***********************end field**********************************\n");
	
}
void rtk_hidg_print_hid_report(struct hid_device *hid_dev, unsigned char report_type)
{
	int n = 0;
	struct hid_report ** reports = NULL;
	if(!hid_dev || report_type >= HID_REPORT_TYPES)
		return;
	
	reports = hid_dev->report_enum[report_type].report_id_hash;
	if(reports) {
		for(n = 0; n < HID_MAX_IDS; n++) {
			int m = 0;
			if(!reports[n])
				continue;
			RTK_COMPOSITE_ERR("SLL report[%d][%d][id:%u, type:%u, application:%u, maxfield:%u, size:%u]\n", 
				report_type, n, reports[n]->id, reports[n]->type, reports[n]->application,
				reports[n]->maxfield, reports[n]->size);
			for(m = 0; m < reports[n]->maxfield; m++) {
				struct hid_field *field = reports[n]->field[m];
				if(!field)
					continue;
				rtk_hidg_print_hid_field(field);
			}
		}
	}	
}
#endif
static bool rtk_hidg_parse_hid_report_desc(struct rtk_hidg_report_desc_data *data)
{
        struct hid_device *hid_dev = NULL;
        if(!data)
	return false;
	
        hid_dev = hid_allocate_device();
        if(!hid_dev)
                goto EXIT;
                
        hid_dev->driver = &dummy_hid_driver;
        hid_dev->dev_rdesc = kmemdup(data->func_desc->report_desc, data->func_desc->report_desc_length, GFP_KERNEL);
        if(!hid_dev->dev_rdesc) {
                hid_destroy_device(hid_dev);
                goto EXIT;
        }
        hid_dev->dev_rsize = data->func_desc->report_desc_length;

        if(hid_open_report(hid_dev) != 0) {
                        hid_destroy_device(hid_dev);
                        goto EXIT;
        }
        data->hid_devices = hid_dev;
        return true;
EXIT:
        return false;        
}

static void rtk_hidg_free_hid_report_desc(struct rtk_hidg_report_desc_data *data)
{
        if(!data)
                return;
	
        hid_destroy_device(data->hid_devices);
        data->hid_devices = NULL;
}

bool rtk_hidg_plat_devices_add(void)
{
        
        int i;
        for(i = 0; i < HIG_PLAT_DEVICE_NUM; i++) {
                if(rtk_hidg_parse_hid_report_desc(
                        (struct rtk_hidg_report_desc_data *)(rtk_gadget_hidg_plat_devices[i].dev.platform_data)) == false)
                        goto EXIT;
                if(platform_device_register(&rtk_gadget_hidg_plat_devices[i]) != 0) {
                        rtk_hidg_free_hid_report_desc((struct rtk_hidg_report_desc_data *)(rtk_gadget_hidg_plat_devices[i].dev.platform_data));
                        goto EXIT;
                }
        }
        return true;
EXIT:
        i = i - 1;
        for(; i >= 0; i--) {
                platform_device_unregister(&rtk_gadget_hidg_plat_devices[i]);
                rtk_hidg_free_hid_report_desc((struct rtk_hidg_report_desc_data *)(rtk_gadget_hidg_plat_devices[i].dev.platform_data));
        }
        return false;
}

void rtk_hidg_plat_devices_remove(void)
{
        int i;
        for(i = 0; i < HIG_PLAT_DEVICE_NUM; i++) {
                platform_device_unregister(&rtk_gadget_hidg_plat_devices[i]);
                rtk_hidg_free_hid_report_desc((struct rtk_hidg_report_desc_data *)(rtk_gadget_hidg_plat_devices[i].dev.platform_data));
        }
}

static bool _rtk_hidg_usage_x_y_compatile(struct hid_field *src_field, 
											struct hid_field *dst_field)
{
	if(!(dst_field->report_count == 2 && src_field->report_count == 2
		&& src_field->report_size < 24))
		return false;
	if((src_field->usage[0].hid == 0x10030 && src_field->usage[1].hid == 0x10031))
		return true;
	return false;
}

static bool _rtk_hidg_usage_x_y_wheel_compatile(struct hid_field *src_field, 
											struct hid_field *dst_field)
{
	if(!(dst_field->report_count == 3 && src_field->report_count == 3
		&& src_field->report_size < 24))
		return false;
	if((src_field->usage[0].hid == 0x10030 && src_field->usage[1].hid == 0x10031 
			&& src_field->usage[2].hid == 0x10038))
		return true;
	return false;
}

static HIDG_USAGE_TYPE rtk_hidg_hid_usage_compatible(unsigned int application,
				struct hid_field *src_field, struct hid_field *dst_field)
{
	HIDG_USAGE_TYPE usage_type = HIDG_USAGE_INVALID;
	
	if(application == HID_KEYBOARD_APPLICATION_ID) {
		if(dst_field->usage[0].hid == 0x700e0 && src_field->usage[0].hid == 0x700e0)
			usage_type = HIDG_USAGE_KEYBOARD_E0;
		else if(dst_field->usage[0].hid == 0x70000 && src_field->usage[0].hid == 0x70000)
			usage_type = HIDG_USAGE_KEYBOARD_00;
	} else if(application == HID_MOUSE_APPLICATION_ID){
		if(dst_field->usage[0].hid == 0x90001 && src_field->usage[0].hid == 0x90001)
			usage_type = HIDG_USAGE_MOUSE_BUTTON;
		else if(_rtk_hidg_usage_x_y_compatile(src_field, dst_field))
			usage_type = HIDG_USAGE_MOUSE_X_Y;
		else if(_rtk_hidg_usage_x_y_wheel_compatile(src_field, dst_field))
			usage_type = HIDG_USAGE_MOUSE_X_Y_WHEEL; 
		else if(dst_field->usage[0].hid == 0x10038 && src_field->usage[0].hid == 0x10038
			&& dst_field->report_count == 1 && src_field->report_count == 1
			&& src_field->report_size < 24)
			usage_type = HIDG_USAGE_MOUSE_WHEEL;
			
	} else if(application == HID_CONSUMER_CONTROL_APPLICATION_ID) {
		if(src_field->report_size == 1 && dst_field->report_size == 1)
			usage_type = HIDG_USAGE_CONSUMER_CONTROL;
	}
	return usage_type;
}

static bool rtk_hidg_hid_report_compatible(struct hid_report *src_report, 
			struct hid_report * dst_report, struct hid_report_mapping *report_mapping)
{
        int i = 0;
        int cur_index = 0;
        if(dst_report->maxfield > MAX_FIELD_MAPPINF_NUM)
                return false;
	 
        if(src_report->application != HID_MOUSE_APPLICATION_ID 
		&& src_report->application != HID_KEYBOARD_APPLICATION_ID
		&& src_report->application != HID_CONSUMER_CONTROL_APPLICATION_ID)
                return false;
	
        if(src_report->application != dst_report->application)
                return false;

        for(i = 0; i < dst_report->maxfield; i++) {
	bool field_matched = false;
	int j = 0;
	struct hid_field *dst_field = dst_report->field[i];
	if(!dst_field->maxusage)
		continue;
	for(j = 0; j < src_report->maxfield; j++) {
		struct hid_field *src_field = src_report->field[j];
		HIDG_USAGE_TYPE usage_type;
		if(!src_field->maxusage || src_field->flags != dst_field->flags
			|| (src_field->usage[0].hid & 0xFFFF0000) != (dst_field->usage[0].hid & 0xFFFF0000))
			continue;
		
		if((usage_type = rtk_hidg_hid_usage_compatible(dst_report->application, src_field, dst_field)) != HIDG_USAGE_INVALID) {
			field_matched = true;
			report_mapping->field_mappings[cur_index].dst_signed = (dst_field->logical_minimum < 0) ? true: false;
			report_mapping->field_mappings[cur_index].dst_report_offset= dst_field->report_offset;
			report_mapping->field_mappings[cur_index].dst_report_count= dst_field->report_count;
			report_mapping->field_mappings[cur_index].dst_report_size= dst_field->report_size;
			report_mapping->field_mappings[cur_index].dst_flags= dst_field->flags;
			report_mapping->field_mappings[cur_index].src_signed = (src_field->logical_minimum < 0) ? true: false;
			report_mapping->field_mappings[cur_index].src_report_offset= src_field->report_offset;
			report_mapping->field_mappings[cur_index].src_report_count= src_field->report_count;
			report_mapping->field_mappings[cur_index].src_report_size= src_field->report_size;
			report_mapping->field_mappings[cur_index].src_flags= src_field->flags;
			report_mapping->field_mappings[cur_index].usage_type= usage_type;
			if(usage_type == HIDG_USAGE_CONSUMER_CONTROL) {
				int m;
				int cnt = (src_field->maxusage > MAX_CONSUMER_USAGE_MAPPING_NUM) ? src_field->maxusage : MAX_CONSUMER_USAGE_MAPPING_NUM;
				memset(report_mapping->field_mappings[cur_index].consumer_usage_mapping, 0xFF, MAX_CONSUMER_USAGE_MAPPING_NUM);
				for(m = 0; m < cnt; m++) {
					int n;
					for(n = 0; n < dst_field->maxusage; n++) {
						if(src_field->usage[m].hid == dst_field->usage[n].hid) {
							report_mapping->field_mappings[cur_index].consumer_usage_mapping[m] = n;
							break;
						}
					}
				}
				report_mapping->field_mappings[cur_index].consumer_usage_mapping_cnt = cnt;
			}
			break;
		}
	}
	if(!field_matched)
		return false;
	cur_index++;
        }
        report_mapping->mapping_cnt = cur_index;
        return true;
}

static void bin_array_shift_right(unsigned char *buf, unsigned int len, unsigned int shift) 
{
	int i = 0;
	if(!buf|| !shift || shift >= 8 || !len)
		return;
	for (i = 0; i < (len - 1); i++) {
	                buf[i] = (buf[i] >> shift) | ((buf[i + 1] & ((1 << shift) - 1)) << (8 - shift));
	}
	buf[i] = (buf[i] >> shift);
}
/***make sure that the actual lengh of buf will be greater than len****/
static void bin_array_shift_left(unsigned char *buf, unsigned int len, unsigned int shift) 
{
	int i = 0;
	unsigned char tmp0 = 0;
	if(!buf|| !shift || shift >= 8 || !len)
		return;
	for (i = 0; i < len; i++) {
		unsigned char tmp = (buf[i] >> (8 - shift));
		buf[i] = (buf[i] << shift) | tmp0;
		tmp0 = tmp;
	}
	buf[i] = tmp0;
}

static bool rtk_hidg_convert_bits(unsigned char *src, unsigned int src_ofst, 
			unsigned char *dst, unsigned int dst_ofst, unsigned int bits_len)
{
	unsigned int src_byte_ofst = src_ofst / 8;
	unsigned int dst_byte_ofst = dst_ofst / 8;
	unsigned int tmp_len = 0;
	unsigned char tmp_buf[20];
	int i = 0;
	
	if(bits_len > 128)
		return false;
	
	memset(tmp_buf, 0, sizeof(tmp_buf));
	tmp_len = ((bits_len + (src_ofst % 8) -1)  / 8 + 1);
	memcpy(tmp_buf, src + src_byte_ofst, tmp_len);
	if(src_ofst % 8)
		bin_array_shift_right(tmp_buf, tmp_len, src_ofst % 8);
	tmp_len = (bits_len - 1)  / 8 + 1;
	if(bits_len % 8)
		tmp_buf[tmp_len - 1] = tmp_buf[tmp_len - 1] & ((1 << bits_len % 8) - 1);
	
	if (dst_ofst % 8) {
		bin_array_shift_left(tmp_buf, tmp_len, dst_ofst % 8);
		tmp_len++;

	}
	for(i = 0; i < tmp_len; i++)
		dst[dst_byte_ofst + i] |= tmp_buf[i];
	return true;
	
}

static bool rtk_hidg_convert_integer(bool src_signed, unsigned char *src, unsigned int src_ofst, unsigned int src_bits_len,
			bool dst_signed, unsigned char *dst, unsigned int dst_ofst, unsigned int dst_bits_len)
{
	unsigned char buf[12] __attribute__((__aligned__(4)));
	unsigned int tmp_len;
	int i = 0;
	if(src_bits_len > 24 || dst_bits_len > 24)
		return false;
	
	memset(buf, 0, sizeof(buf));
	tmp_len = ((src_bits_len + (src_ofst % 8) - 1)  / 8 + 1);
	memcpy(buf, src + src_ofst / 8, tmp_len);
	if(src_ofst % 8)
		bin_array_shift_right(buf, tmp_len, src_ofst % 8);
	tmp_len = (src_bits_len - 1)  / 8 + 1;
	if(src_bits_len % 8)
		buf[tmp_len - 1] = buf[tmp_len - 1] & ((1 << src_bits_len % 8) - 1);
		
	if(!dst_signed) {
		unsigned int d_val = 0;
		unsigned int max = ((1 << dst_bits_len) - 1);
		if(!src_signed) {
			d_val = (buf[0] << 0) | (buf[1] <<8) | (buf[2] << 16) | (buf[3] << 24);
		} else {
			if(buf[(src_bits_len - 1) / 8] & (1 << ((src_bits_len - 1) % 8))) {
				unsigned int tmp1 = (buf[0] << 0) | (buf[1] <<8) | (buf[2] << 16) | (buf[3] << 24);
				unsigned int tmp2 = 0xFFFFFFFF << (src_bits_len - 1);
				d_val = (int)(tmp1 | tmp2);
				
			} else {
				d_val = (buf[0] << 0) | (buf[1] <<8) | (buf[2] << 16) | (buf[3] << 24);
			}
			d_val += ((1 << (src_bits_len - 1)) - 1);
		}
		if(d_val > max)
			d_val = max;
		memset(buf, 0, sizeof(buf));
		buf[0] = (d_val & 0xFF);
		buf[1] = ((d_val>> 8) & 0xFF);
		buf[2] = ((d_val>> 16) & 0xFF);
		buf[3] = ((d_val>> 24) & 0xFF);
	} else {
		int d_val = 0;
		int max = ((1 << (dst_bits_len - 1)) - 1);
		int min = -((1 << (dst_bits_len - 1)) - 1);
		if(!src_signed) {
			d_val = (buf[0] << 0) | (buf[1] <<8) | (buf[2] << 16) | (buf[3] << 24);
			d_val -= ((1 << (src_bits_len - 1)) - 1);
		} else {
			if(buf[(src_bits_len - 1) / 8] & (1 << ((src_bits_len - 1) % 8))) {
				unsigned int tmp1 = (buf[0] << 0) | (buf[1] <<8) | (buf[2] << 16) | (buf[3] << 24);
				unsigned int tmp2 = 0xFFFFFFFF << (src_bits_len - 1);
				d_val = (int)(tmp1 | tmp2);
			} else {
				d_val = (buf[0] << 0) | (buf[1] <<8) | (buf[2] << 16) | (buf[3] << 24);
			}
		}
		if(d_val > max)
			d_val = max;
		else if(d_val < min)
			d_val = min;
		
		memset(buf, 0, sizeof(buf));
		buf[0] = (d_val & 0xFF);
		buf[1] = ((d_val>> 8) & 0xFF);
		buf[2] = ((d_val>> 16) & 0xFF);
		buf[3] = ((d_val>> 24) & 0xFF);
	}
	
	tmp_len = (dst_bits_len - 1)  / 8 + 1;
	if(dst_bits_len % 8)
		buf[tmp_len - 1] = buf[tmp_len - 1] & ((1 << dst_bits_len % 8) - 1);
	if (dst_ofst % 8) {
		bin_array_shift_left(buf, tmp_len, dst_ofst % 8);
		tmp_len++;

	}
	for(i = 0; i < tmp_len; i++)
		dst[(dst_ofst / 8) + i] |= buf[i];	
	return true;
}

bool rtk_hidg_convert_report(struct hid_report_mapping *report_mapping, 
				unsigned char *src_buf, unsigned int src_len, 
				unsigned char *p_dst_buf, unsigned int *p_dst_len)
{
        unsigned int i = 0;
        bool ret = false;
        unsigned char *dst_buf = p_dst_buf;
        unsigned char *new_report_buf = NULL;
        unsigned char *old_report_buf = NULL;
        unsigned int dst_len = 0;
	
        if(!report_mapping ||!src_buf || !src_len || !p_dst_buf || !p_dst_len)
                return ret;

        if(src_len < ((report_mapping->src_size -1) / 8 + 1 + (!!report_mapping->src_numbered))  || !report_mapping->dst_size)
	return ret;	

        dst_len = ((report_mapping->dst_size -1) / 8 + 1);
        if(report_mapping->dst_numbered)
		dst_len++;
        if(dst_len > *p_dst_len)
                return ret;
        memset(dst_buf, 0, dst_len);

        if(report_mapping->dst_numbered)
	dst_buf[0] = report_mapping->dst_report_id;
	
        new_report_buf = dst_buf + (!!report_mapping->dst_numbered);
        old_report_buf = src_buf + (!!report_mapping->src_numbered);
	
        for(i = 0; i < report_mapping->mapping_cnt; i++) {
	struct hid_field_mapping *field_mapping = &report_mapping->field_mappings[i];
	switch(field_mapping->usage_type) {
	case HIDG_USAGE_MOUSE_BUTTON:
	case HIDG_USAGE_KEYBOARD_E0: {
		ret = rtk_hidg_convert_bits(old_report_buf, field_mapping->src_report_offset, 
			new_report_buf, field_mapping->dst_report_offset, 
			(field_mapping->src_report_count > field_mapping->dst_report_count) ? field_mapping->dst_report_count : field_mapping->src_report_count);
		if(ret == false)
			goto EXIT;
		break;

	}
	case HIDG_USAGE_KEYBOARD_00: {
		unsigned int clen = min(field_mapping->src_report_count * field_mapping->src_report_size , field_mapping->dst_report_count * field_mapping->dst_report_size);
		ret = rtk_hidg_convert_bits(old_report_buf, field_mapping->src_report_offset, 
			new_report_buf, field_mapping->dst_report_offset, clen);
		if(ret == false)
			goto EXIT;
		break;
	}
	case HIDG_USAGE_MOUSE_WHEEL: {
		ret = rtk_hidg_convert_integer(field_mapping->src_signed, old_report_buf, field_mapping->src_report_offset, field_mapping->src_report_size,
			field_mapping->dst_signed, new_report_buf, field_mapping->dst_report_offset, field_mapping->dst_report_size);
		if(ret == false)
			goto EXIT;
		break;

	}
	case HIDG_USAGE_MOUSE_X_Y: {
		ret = rtk_hidg_convert_integer(field_mapping->src_signed, old_report_buf, field_mapping->src_report_offset, field_mapping->src_report_size,
			field_mapping->dst_signed, new_report_buf, field_mapping->dst_report_offset, field_mapping->dst_report_size);
		if(ret == false)
			goto EXIT;
		ret = rtk_hidg_convert_integer(field_mapping->src_signed, old_report_buf, field_mapping->src_report_offset + field_mapping->src_report_size, field_mapping->src_report_size,
			field_mapping->dst_signed, new_report_buf, field_mapping->dst_report_offset + field_mapping->dst_report_size, field_mapping->dst_report_size);
		if(ret == false)
			goto EXIT;
		break;

	}
	case HIDG_USAGE_MOUSE_X_Y_WHEEL:{
		ret = rtk_hidg_convert_integer(field_mapping->src_signed, old_report_buf, field_mapping->src_report_offset, field_mapping->src_report_size,
			field_mapping->dst_signed, new_report_buf, field_mapping->dst_report_offset, field_mapping->dst_report_size);
		if(ret == false)
			goto EXIT;
		ret = rtk_hidg_convert_integer(field_mapping->src_signed, old_report_buf, field_mapping->src_report_offset + field_mapping->src_report_size, field_mapping->src_report_size,
			field_mapping->dst_signed, new_report_buf, field_mapping->dst_report_offset + field_mapping->dst_report_size, field_mapping->dst_report_size);
		if(ret == false)
			goto EXIT;
		ret = rtk_hidg_convert_integer(field_mapping->src_signed, old_report_buf, field_mapping->src_report_offset + 2 * field_mapping->src_report_size, field_mapping->src_report_size,
			field_mapping->dst_signed, new_report_buf, field_mapping->dst_report_offset + 2 * field_mapping->dst_report_size, field_mapping->dst_report_size);
		if(ret == false)
			goto EXIT;
		break;

	}
		
	case HIDG_USAGE_CONSUMER_CONTROL: {
		int m = 0;
		for(m = 0; m < field_mapping->consumer_usage_mapping_cnt; m++) {
			unsigned int src_byte_ofst;
			unsigned int src_bit_ofst;
			if(field_mapping->consumer_usage_mapping[m] == 0xFF)
				continue;
			src_byte_ofst = (field_mapping->src_report_offset + m) / 8;
			src_bit_ofst = (field_mapping->src_report_offset + m) % 8;
			if(old_report_buf[src_byte_ofst] & (1 << src_bit_ofst)) {
				unsigned int dst_byte_ofst = (field_mapping->dst_report_offset + field_mapping->consumer_usage_mapping[m]) / 8;
				unsigned int dst_bit_ofst = (field_mapping->dst_report_offset + field_mapping->consumer_usage_mapping[m]) % 8;
				new_report_buf[dst_byte_ofst] = new_report_buf[dst_byte_ofst] | (1 << dst_bit_ofst);
			}
			
		}
		break;
	}	
	default:
		goto EXIT;
	};
        }
        *p_dst_len = dst_len;
        ret = true;
 EXIT:   
        return ret;
}


bool rtk_hidg_mapping_hid_report(struct hid_report * src_report, struct hid_report_mapping *report_mapping)
{
        int i = 0;
        bool ret = false;
        if(!src_report || !report_mapping)
                return false;
        memset(report_mapping, 0, sizeof(struct hid_report_mapping));

        for(i  = 0; i < HIG_PLAT_DEVICE_NUM; i++) {
                int j = 0;
                struct hid_device *hid_dev;
                struct rtk_hidg_report_desc_data *desc_data = (struct rtk_hidg_report_desc_data *)rtk_gadget_hidg_plat_devices[i].dev.platform_data;
                if(!desc_data || !desc_data->hid_devices)
                        continue;
                hid_dev = desc_data->hid_devices;
                for(j = 0; j < HID_MAX_IDS; j++) {
                        struct hid_report *dst_report = hid_dev->report_enum[HID_INPUT_REPORT].report_id_hash[j];
                        if(!dst_report)
                                continue;
	         memset(report_mapping, 0, sizeof(*report_mapping));
                        if(rtk_hidg_hid_report_compatible(src_report, dst_report, report_mapping) == true) {
                                report_mapping->dst_device_id = i;
                                report_mapping->dst_numbered = hid_dev->report_enum[HID_INPUT_REPORT].numbered;
                                report_mapping->dst_report_id = dst_report->id;
                                report_mapping->dst_size = dst_report->size;
                                report_mapping->src_size = src_report->size;
                                report_mapping->application = src_report->application;
                                ret =true;
                                break;
                        }
                }
                if(ret == true)
                        break;
        }
        
        return ret;
}

extern bool g_hid_via_otg_enable;
extern bool g_hid_via_ddc_enable;
ssize_t f_hidg_send_data(int id, const unsigned char *buffer, unsigned int count, unsigned int f_flags);
ssize_t f_hidddc_send_data(int id, const unsigned char *buffer, unsigned int count, unsigned int f_flags);
bool rtk_hidg_send_hid_report(int id, const unsigned char *buffer, unsigned int count, unsigned int f_flags)
{
        if(count == 0 || count > 1024)
                return false;
	 if(g_hid_via_otg_enable)
        	f_hidg_send_data(id, buffer, count, f_flags);
	 if(g_hid_via_ddc_enable)
	 	f_hidddc_send_data(id, buffer, count, f_flags);
        return true;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
static struct bus_type *s_hid_bus_type = NULL;
int hid_input_report_callback(struct hid_device *hdev, struct hid_report *report, u8 *data, int size)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)) && defined(CONFIG_TRACEPOINTS) && defined(CONFIG_ANDROID_VENDOR_HOOKS)	
	bool res;
	struct hid_report_mapping_data *report_mapping_data = NULL;
	struct hid_report_mapping *report_mapping = NULL;
	unsigned char dst[32];
	unsigned int dst_len = 0;

	if(!hdev || !report || !data || !size || !hdev->android_kabi_reserved1)
		return 0;

	if((report->type != HID_INPUT_REPORT) ||  (report->id >= MAX_REPORT_MAPPING_CNT))
		return 0;

	report_mapping_data = (struct hid_report_mapping_data *)hdev->android_kabi_reserved1;
	report_mapping = &report_mapping_data->input_report_mapping[report->id];
	if(!report_mapping->active)
		return 0;

	dst_len = 32;
	res = rtk_hidg_convert_report(report_mapping, data, size, dst, &dst_len);
	if(!res)
		return 0;

	rtk_hidg_send_hid_report(report_mapping->dst_device_id, dst, dst_len, 0);
#endif
	return 0;
}

static int rtk_hidg_bus_notify(struct notifier_block *nb, unsigned long action,
		void *data)
{
	struct hid_device *hdev = NULL;
	struct device *dev = data;

	if(!dev)
		return 0;

	hdev = to_hid_device(dev);

	if(!hdev || (hdev->bus != BUS_USB && hdev->bus != BUS_BLUETOOTH))
		return 0;

	switch (action) {
	case BUS_NOTIFY_ADD_DEVICE:
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)) && defined(CONFIG_TRACEPOINTS) && defined(CONFIG_ANDROID_VENDOR_HOOKS)		
		if(!hdev->android_kabi_reserved1) {
			hdev->android_kabi_reserved1 = (unsigned long)kmalloc(sizeof(struct hid_report_mapping_data), GFP_KERNEL);
		}
#endif		
		
		break;
	case BUS_NOTIFY_BOUND_DRIVER:
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)) && defined(CONFIG_TRACEPOINTS) && defined(CONFIG_ANDROID_VENDOR_HOOKS)		
		if(hdev->driver)
			hdev->driver->raw_event = hid_input_report_callback;
		if(hdev->android_kabi_reserved1) {
				int i;
				struct  hid_report_mapping_data *report_mapping_data = (struct  hid_report_mapping_data *)hdev->android_kabi_reserved1;
				
				for(i = 0; i < MAX_REPORT_MAPPING_CNT; i++) {
					bool ret;
					struct hid_report *report = hdev->report_enum[HID_INPUT_REPORT].report_id_hash[i];
					if(!report || report->id >= MAX_REPORT_MAPPING_CNT)
						continue;
			               ret = rtk_hidg_mapping_hid_report(report, &report_mapping_data->input_report_mapping[report->id]);
			               if(ret == false) {
			               	continue;
			               } else {
						report_mapping_data->input_report_mapping[report->id].src_numbered = hdev->report_enum[HID_INPUT_REPORT].numbered;
						report_mapping_data->input_report_mapping[report->id].src_report_id = report->id;
						report_mapping_data->input_report_mapping[report->id].active = true;
			               }
				}
		}
#endif		
		break;
	case BUS_NOTIFY_DEL_DEVICE:
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)) && defined(CONFIG_TRACEPOINTS) && defined(CONFIG_ANDROID_VENDOR_HOOKS)		
		if(hdev->android_kabi_reserved1) {
			kfree((void *)hdev->android_kabi_reserved1);
			hdev->android_kabi_reserved1 = 0;
		}
#endif		
		break;
	}
	return 0;
}

static struct notifier_block rtk_hidg_bus_nb = {
	.notifier_call = rtk_hidg_bus_notify,
};


int __init rtk_hidg_register_hid_input_report_callback(void)
{
	int ret;
	struct hid_device *hid_dev = NULL;
	hid_dev = hid_allocate_device();
	if(!hid_dev)
		return -ENOMEM;
	s_hid_bus_type = hid_dev->dev.bus;
	hid_destroy_device(hid_dev);

	ret = bus_register_notifier(s_hid_bus_type, &rtk_hidg_bus_nb);
	if(ret != 0)
		return ret;
	return 0;
}

void __exit rtk_hidg_unregister_hid_input_report_callback(void)
{
	bus_unregister_notifier(s_hid_bus_type, &rtk_hidg_bus_nb);
}
#endif
